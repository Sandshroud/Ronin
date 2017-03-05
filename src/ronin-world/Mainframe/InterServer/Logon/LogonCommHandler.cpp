/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
initialiseSingleton(LogonCommHandler);

extern bool bServerShutdown;

LogonCommHandler::LogonCommHandler()
{
    idhigh = 1;
    logon = NULL;
    realm = NULL;
    server = NULL;
    next_request = 1;
    ReConCounter = 0;
    pings = !mainIni->ReadBoolean("LogonServer", "DisablePings", false);
    std::string logon_pass = mainIni->ReadString("LogonServer", "RemotePassword", "r3m0t3");

    // sha1 hash it
    Sha1Hash hash;
    hash.UpdateData(logon_pass);
    hash.Finalize();
    memcpy(sql_passhash, hash.GetDigest(), 20);

#if PLATFORM == PLATFORM_WIN
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
#endif
}

LogonCommHandler::~LogonCommHandler()
{
    delete server;
    server = NULL;

    delete realm;
    realm = NULL;

#if PLATFORM == PLATFORM_WIN
    CloseHandle(hEvent);
#else
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
#endif
}

LogonCommClientSocket * LogonCommHandler::ConnectToLogon(std::string Address, uint32 Port)
{
    return ConnectTCPSocket<LogonCommClientSocket>(Address.c_str(), Port);
}

void LogonCommHandler::RequestAddition(LogonCommClientSocket * Socket)
{
    // Add realm to the packet
    WorldPacket data(RCMSG_REGISTER_REALM, 100);
    data << logon->realmID << logon->realmName;
    data << realm->Address;
    data << realm->Icon;
    data << realm->WorldRegion;
    data << uint32(sWorld.GetPlayerLimit());
    data << uint8(4) << uint8(3) << uint8(4);
    data << uint16(CL_BUILD_SUPPORT);
    Socket->SendPacket(&data);
}

void LogonCommHandler::Startup()
{
    // Try to connect to all logons.
    LoadRealmConfiguration();

    sLog.Notice("LogonCommClient", "Loading forced permission strings...");
    QueryResult * result = CharacterDatabase.Query("SELECT * FROM account_forced_permissions");
    if( result != NULL )
    {
        do
        {
            std::string acct = RONIN_UTIL::TOUPPER_RETURN(result->Fetch()[0].GetString());
            std::string perm = result->Fetch()[1].GetString();
            forced_permissions.insert(std::make_pair(acct,perm));

        } while (result->NextRow());
        delete result;
    }

    Connect();
}

const std::string* LogonCommHandler::GetForcedPermissions(std::string& username)
{
    ForcedPermissionMap::iterator itr = forced_permissions.find(username);
    if(itr == forced_permissions.end())
        return NULL;

    return &itr->second;
}

void LogonCommHandler::Connect()
{
    if(bServerShutdown)
        return;

    if(ReConCounter >= 10)
    { // Attempt to connect 5 times, if not able to, shut down.
        sWorld.QueueShutdown(5, SERVER_SHUTDOWN_TYPE_SHUTDOWN);
        return;
    }

    ++ReConCounter;

    sLog.Notice("LogonCommClient", "Connecting to logonserver on `%s:%u, attempt %u`", server->Address.c_str(), server->Port, ReConCounter );

    server->RetryTime = getMSTime()+10000;
    server->Registered = false;

    mapLock.Acquire();
    logon = ConnectToLogon(server->Address, server->Port);
    if(logon == NULL)
    {
        sLog.Notice("LogonCommClient", "Connection failed. Will try again in 10 seconds.");
        mapLock.Release();
        return;
    }

    sLog.Notice("LogonCommClient", "Authenticating...");
    uint32 tt = getMSTime()+10000;
    logon->SendChallenge(realm->Name);
    while(!logon->authenticated)
    {
        if(getMSTime() >= tt || logon->rejected || bServerShutdown)
        {
            if(logon->rejected)
                sLog.Error("LogonCommClient", "Authentication rejected.");
            else
                sLog.Notice("LogonCommClient", "Authentication timed out.");
            logon->Disconnect();
            logon = NULL;
            mapLock.Release();
            return;
        }

        Delay(10);
    }

    if(!logon->authenticated)
    {
        sLog.Notice("LogonCommClient","Authentication failed.");
        logon->Disconnect();
        logon = NULL;
        mapLock.Release();
        return;
    }
    else
        sLog.Success("LogonCommClient","Authentication succeeded.");

    sLog.Notice("LogonCommClient", "Registering Realms...");
    logon->_id = server->ID;

    RequestAddition(logon);

    uint32 st = getMSTime()+15000;

    // Wait for register ACK
    while(server->Registered == false)
    {   // Don't wait more than 15 seconds for a registration, thats our ping timeout
        if(getMSTime() >= st || bServerShutdown)
        {
            sLog.Notice("LogonCommClient", "Realm registration timed out.");
            logon->Disconnect();
            logon = NULL;
            break;
        }
        Delay(50);
    }

    if(logon == NULL || !server->Registered)
    {
        mapLock.Release();
        return;
    }

    // Wait for all realms to register
    Delay(200);

    sLog.Success("LogonCommClient", "Logonserver latency is %ums.", logon->latency);

    // We have connected, reset our attempt counter.
    ReConCounter = 0;
    mapLock.Release();
}

void LogonCommHandler::AdditionAck(uint32 ServID)
{
    server->ServerID = ServID;
    server->Registered = true;
}

void LogonCommHandler::UpdateSockets(uint32 diff)
{
    if(bServerShutdown)
        return;

    mapLock.Acquire();
    uint32 t = getMSTime();
    if(logon != NULL)
    {
        if(logon->IsDeleted() || !logon->IsConnected())
        {
            sLog.Error("LogonCommClient","Realm id %u lost connection.", (unsigned int)server->ID);
            logon->_id = 0;
            if(logon->IsConnected())
                logon->Disconnect();
            logon = NULL;
            mapLock.Release();
            return;
        }

        if(pings)
        {
            if(t - logon->last_ping > (15000+logon->latency))
            {
                // no ping for 15 seconds -> remove the socket
                sLog.Error("LogonCommClient","Realm id %u connection dropped due to pong timeout.", (unsigned int)server->ID);
                logon->_id = 0;
                logon->Disconnect();
                logon = NULL;
                mapLock.Release();
                return;
            }
        }

        mapLock.Release();
        return;
    }

    mapLock.Release();

    // Try to reconnect
    if(t >= server->RetryTime && !bServerShutdown)
        Connect();
}

void LogonCommHandler::ConnectionDropped()
{
    if(bServerShutdown)
        return;

    mapLock.Acquire();
    if(logon != NULL)
    {
        if(!bServerShutdown)
            sLog.Error("LogonCommHandler"," Realm connection was dropped unexpectedly. reconnecting next loop.");
        logon->_id = 0;
        logon->Disconnect();
        logon = NULL;
    }
    mapLock.Release();
}

uint32 LogonCommHandler::ClientConnected(std::string AccountName, WorldSocket * Socket)
{
    uint32 request_id = next_request++;
    size_t i = 0;
    const char * acct = AccountName.c_str();
    sLog.Debug( "LogonCommHandler","Sending request for account information: `%s` (request %u).", AccountName.c_str(), request_id);

    // Send request packet to server.
    if(logon == NULL)
    {
        // No valid logonserver is connected.
        return (uint32)-1;
    }

    WorldPacket data(RCMSG_REQUEST_SESSION, 100);
    data << int32(-42);
    data << server->ID;
    data << request_id;
    data << std::string(acct);
    logon->SendPacket(&data);

    pendingLock.Acquire();
    pending_logons[request_id] = Socket;
    pendingLock.Release();

    return request_id;
}

void LogonCommHandler::UnauthedSocketClose(uint32 id)
{
    pendingLock.Acquire();
    pending_logons.erase(id);
    pendingLock.Release();
}

void LogonCommHandler::RemoveUnauthedSocket(uint32 id)
{
    pending_logons.erase(id);
}

void LogonCommHandler::LoadRealmConfiguration()
{
    server = new LogonServer();
    server->ID = idhigh++;
    server->Name = mainIni->ReadString("LogonServer", "Name", "UnkLogon");
    server->Address = mainIni->ReadString("LogonServer", "Address", "127.0.0.1");
    server->Port = mainIni->ReadInteger("LogonServer", "Port", 8093);

    realm = new Realm();
    ZeroMemory(realm, sizeof(Realm*));

    char* port = new char[10];
    itoa(mainIni->ReadInteger( "RealmData", "WorldServerPort", 8129), port, 10);
    std::string address = mainIni->ReadString( "RealmData", "Address", "127.0.0.1" );
    address.append(":");
    address.append(port);
    delete[] port;

    realm->Address = address;
    realm->WorldRegion = mainIni->ReadInteger("RealmData", "WorldRegion", 1);
    realm->Name = mainIni->ReadString("RealmData", "RealmName", "SomeRealm");

    // Realm type is based on icon, in world pvp is default true
    switch(realm->Icon = mainIni->ReadInteger("RealmData", "RealmType", 0))
    {
    case REALMTYPE_NORMAL:
    case REALMTYPE_RP:
        // For normal and RP set pvp type to false
        sWorld.IsPvPRealm = false;
    case REALMTYPE_RPPVP:
        break; // RP PVP is fine
    default: // If we're not an RP PvP, then we're pvp
        realm->Icon = REALMTYPE_PVP;
        break;
    }
}

void LogonCommHandler::UpdateAccountCount(uint32 account_id, int8 add)
{
    if(logon == NULL) // No valid logonserver is connected.
        return;

    logon->UpdateAccountCount(account_id, add);
}

void LogonCommHandler::TestConsoleLogon(std::string& username, std::string& password, uint32 requestnum)
{
    if(logon == NULL) // No valid logonserver is connected.
        return;

    std::string newuser = username;
    std::string newpass = password;
    std::string srpstr;

    RONIN_UTIL::TOUPPER(newuser);
    RONIN_UTIL::TOUPPER(newpass);

    srpstr = newuser + ":" + newpass;

    Sha1Hash hash;
    hash.UpdateData(srpstr);
    hash.Finalize();

    WorldPacket data(RCMSG_TEST_CONSOLE_LOGIN, 100);
    data << requestnum;
    data << newuser;
    data.append(hash.GetDigest(), 20);

    logon->SendPacket(&data);
}

// db funcs
void LogonCommHandler::Account_SetBanned(const char * account, uint32 banned, const char* reason)
{
    if(logon == NULL) // No valid logonserver is connected.
        return;

    WorldPacket data(RCMSG_MODIFY_DATABASE, 50);
    data << uint32(1);      // 1 = ban
    data << std::string(account);
    data << uint32(banned);
    data << std::string(reason);
    logon->SendPacket(&data);
}

void LogonCommHandler::Account_SetGM(const char * account, const char * flags)
{
    if(logon == NULL) // No valid logonserver is connected.
        return;

    WorldPacket data(RCMSG_MODIFY_DATABASE, 50);
    data << uint32(2);      // 2 = set gm
    data << std::string(account);
    data << std::string(flags);
    logon->SendPacket(&data);
}

void LogonCommHandler::Account_SetMute(const char * account, uint32 muted)
{
    if(logon == NULL) // No valid logonserver is connected.
        return;

    WorldPacket data(RCMSG_MODIFY_DATABASE, 50);
    data << uint32(3);      // 3 = mute
    data << std::string(account);
    data << uint32(muted);
    logon->SendPacket(&data);
}

void LogonCommHandler::IPBan_Add(const char * ip, uint32 duration, const char* reason)
{
    if(logon == NULL) // No valid logonserver is connected.
        return;

    WorldPacket data(RCMSG_MODIFY_DATABASE, 50);
    data << uint32(4);      // 4 = ipban add
    data << std::string(ip);
    data << uint32(duration);
    data << std::string(reason);
    logon->SendPacket(&data);
}

void LogonCommHandler::IPBan_Remove(const char * ip)
{
    if(logon == NULL) // No valid logonserver is connected.
        return;

    WorldPacket data(RCMSG_MODIFY_DATABASE, 50);
    data << uint32(5);      // 5 = ipban remove
    data << std::string(ip);
    logon->SendPacket(&data);
}

void LogonCommHandler::SendCreateAccountRequest(const char *accountname, const char *password, const char *email, uint32 accountFlags)
{
    if(logon == NULL) // No valid logonserver is connected.
        return;

    WorldPacket data(RCMSG_MODIFY_DATABASE, 50);
    data << uint32(6);      // 6 = create account
    data << std::string(email);
    data << std::string(password);
    data << std::string(accountname);
    data << accountFlags;
    logon->SendPacket(&data);
}
