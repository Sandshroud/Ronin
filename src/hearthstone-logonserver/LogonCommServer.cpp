/***
 * Demonstrike Core
 */

#include "LogonStdAfx.h"
#pragma pack(push, 1)
typedef struct
{
    uint16 opcode;
    uint32 size;
}logonpacket;
#pragma pack(pop)

LogonCommServerSocket::LogonCommServerSocket(SOCKET fd, const sockaddr_in * peer) : TcpSocket(fd, 65536, 524288, false, peer)
{
    // do nothing
    last_pong = last_ping = getMSTime();
    remaining = opcode = 0;
    removed = true;

    latency = 0;
    authenticated = 0;
}

LogonCommServerSocket::~LogonCommServerSocket()
{

}

void LogonCommServerSocket::OnConnect()
{
    if( !IsServerAllowed(GetRemoteAddress().s_addr) )
    {
        printf("Server connection from %u(%s:%u) DENIED, not an allowed IP.\n", GetRemoteAddress().s_addr, GetIP(), GetPort());
        Disconnect();
        return;
    }

    sInfoCore.AddServerSocket(this);
    removed = false;
}

void LogonCommServerSocket::OnDisconnect()
{
    // if we're registered -> de-register
    if(!removed)
    {
        sInfoCore.SetRealmOffline(realmID, this);
        sInfoCore.RemoveServerSocket(this);
    }
}

void LogonCommServerSocket::OnRecvData()
{
    while(true)
    {
        if(!remaining)
        {
            if(GetReadBuffer()->GetSize() < 6)
                return;  // no header

            // read header
            Read((uint8*)&opcode, 2);
            Read((uint8*)&remaining, 4);
            // decrypt the packet
            _recv.Process((unsigned char*)&opcode, (unsigned char*)&opcode, 2);
            _recv.Process((unsigned char*)&remaining, (unsigned char*)&remaining, 4);

            EndianConvert(opcode);
            /* reverse byte order */
            EndianConvertReverse(remaining);
        }

        // do we have a full packet?
        if(GetReadBuffer()->GetSize() < remaining)
            return;

        // create the buffer
        WorldPacket buff(opcode, remaining);
        if(remaining)
        {
            buff.resize(remaining);
            Read((uint8*)buff.contents(), remaining);
            _recv.Process((unsigned char*)buff.contents(), (unsigned char*)buff.contents(), remaining);
        }

        // handle the packet
        HandlePacket(buff);

        remaining = 0;
        opcode = 0;
    }
}

void LogonCommServerSocket::HandlePacket(WorldPacket & recvData)
{
    if(authenticated == 0 && recvData.GetOpcode() != RCMSG_AUTH_CHALLENGE)
    {
        // invalid
        Disconnect();
        return;
    }

    static logonpacket_handler Handlers[RMSG_COUNT] = {
        NULL,                                               // RMSG_NULL
        &LogonCommServerSocket::HandleRegister,             // RCMSG_REGISTER_REALM
        NULL,                                               // RSMSG_REALM_REGISTERED
        &LogonCommServerSocket::HandleSessionRequest,       // RCMSG_REQUEST_SESSION
        NULL,                                               // RSMSG_SESSION_RESULT
        NULL,                                               // RCMSG_PING
        &LogonCommServerSocket::HandlePong,                 // RSMSG_PONG
        NULL,                                               // RMSG_LATENCY
        NULL,/*Deprecated*/                                 // RCMSG_SQL_EXECUTE
        &LogonCommServerSocket::HandleReloadAccounts,       // RCMSG_RELOAD_ACCOUNTS
        &LogonCommServerSocket::HandleAuthChallenge,        // RCMSG_AUTH_CHALLENGE
        NULL,                                               // RSMSG_AUTH_RESPONSE
        NULL,                                               // RSMSG_REQUEST_ACCOUNT_CHARACTER_MAPPING
        &LogonCommServerSocket::HandleMappingReply,         // RCMSG_ACCOUNT_CHARACTER_MAPPING_REPLY
        &LogonCommServerSocket::HandleUpdateMapping,        // RCMSG_UPDATE_CHARACTER_MAPPING_COUNT
        NULL,                                               // RSMSG_DISCONNECT_ACCOUNT
        &LogonCommServerSocket::HandleTestConsoleLogin,     // RCMSG_TEST_CONSOLE_LOGIN
        NULL,                                               // RSMSG_CONSOLE_LOGIN_RESULT
        &LogonCommServerSocket::HandleDatabaseModify,       // RCMSG_MODIFY_DATABASE
    };

    if(recvData.GetOpcode() >= RMSG_COUNT || Handlers[recvData.GetOpcode()] == NULL)
    {
        printf("Got unknown packet from logoncomm: %u\n", recvData.GetOpcode());
        return;
    }

    (this->*(Handlers[recvData.GetOpcode()]))(recvData);
}

void LogonCommServerSocket::HandleRegister(WorldPacket & recvData)
{
    uint32 realmid;
    std::string realmName, address;

    recvData >> realmid >> realmName >> address;
    sLog.Notice("LogonCommServer","Registering realm `%s` under ID %u.", realmName.c_str(), realmid);
    Realm* oldrealm = sInfoCore.GetRealm(realmid);
    if(oldrealm || sInfoCore.FindRealmWithAdress(address))
    {
        // We already have a realm here, and it's not offline, this may be dangerous, but meh.
        WorldPacket data(RSMSG_REALM_REGISTERED, 4);
        data << uint32(1); // Error
        SendPacket(&data);
        if(oldrealm)
            sLog.Notice("LogonCommServer", "Realm(%s) addition denied, realm already connected.", realmName.c_str());
        else
        {
            sLog.Notice("LogonCommServer", "Realm(%s) addition denied, adress already used.", realmName.c_str());
            sLog.Line();
        }
        return;
    }

    Realm * realm = new Realm();
    ZeroMemory(realm, sizeof(Realm*));
    realm->Name = realmName;
    realm->Address = address;
    realm->Flag = REALM_FLAG_RECOMMENDED;
    realm->Population = REALM_POP_NEW_PLAYERS;
    realm->ServerSocket = this;
    recvData >> realm->Icon >> realm->WorldRegion >> realm->RealmCap >> realm->RequiredCV[0] >> realm->RequiredCV[1] >> realm->RequiredCV[2] >> realm->RequiredBuild;

    // Add to the main realm list
    sInfoCore.AddRealm(realmid, realm);

    // Send back response packet.
    WorldPacket data(RSMSG_REALM_REGISTERED, 4);
    data << uint32(0);          // Error
    SendPacket(&data);
    realmID = realmid;

    /* request character mapping for this realm */
    data.Initialize(RSMSG_REQUEST_ACCOUNT_CHARACTER_MAPPING);
    data << realmid;
    SendPacket(&data);

    SendDataPing();

    sLog.Notice("LogonCommServer", "Realm(%s) successfully added.", realm->Name.c_str());
}

void LogonCommServerSocket::HandleSessionRequest(WorldPacket & recvData)
{
    std::string account_name;
    int32 identifier = 0;
    uint32 serverid = 0, request_id = 0;
    recvData >> identifier;
    if(identifier != -42)
        request_id = identifier;
    else
    {
        recvData >> serverid;
        recvData >> request_id;
    }
    recvData >> account_name;

    // get sessionkey!
    uint32 error = 0;
    Account * acct = sAccountMgr.GetAccount(account_name);
    if(acct == NULL || acct->SessionKey == NULL || acct == NULL )
        error = 1;      // Unauthorized user.

    if(serverid)
    {
        Realm* realm = sInfoCore.GetRealm(serverid);
        if(realm != NULL && realm->Flag & REALM_FLAG_FULL)
            error = 1; // Unauthorized user.
    }

    // build response packet
    WorldPacket data(RSMSG_SESSION_RESULT, 150);
    data << request_id;
    data << error;
    if(!error)
    {
        // Append account information.
        data << acct->AccountId;
        data << acct->UsernamePtr->c_str();
        if(!acct->GMFlags)
            data << uint8(0);
        else
            data << acct->GMFlags;

        data << acct->AccountFlags;
        data.append(acct->SessionKey, 40);
        data.append(acct->Locale, 4);
        data << acct->Muted;
    }

    SendPacket(&data);
}

void LogonCommServerSocket::SendDataPing()
{
    WorldPacket data(RCMSG_PING, 4);
    data << realmID;
    SendPacket(&data);

    last_ping = getMSTime();
}

void LogonCommServerSocket::HandlePong(WorldPacket & recvData)
{
    uint32 realmId, population;
    recvData >> realmId >> population;
    sInfoCore.UpdateRealmPop(realmId, population);

    last_pong = getMSTime();
    latency = last_pong-last_ping;

    WorldPacket data(RMSG_LATENCY, 0);
    data << latency;
    SendPacket(&data);
}

void LogonCommServerSocket::SendPacket(WorldPacket * data)
{
    bool rv;
    LockWriteBuffer();

    logonpacket header;
    header.opcode = data->GetOpcode();
    EndianConvert(header.opcode);
    header.size = (uint32)data->size();
    EndianConvertReverse(header.size);
    _send.Process((unsigned char*)&header, (unsigned char*)&header, 6);
    rv = WriteButHold((uint8*)&header, 6);

    if(data->size() > 0 && rv)
    {
        _send.Process((unsigned char*)data->contents(), (unsigned char*)data->contents(), (uint32)data->size());
        rv = Write(data->contents(), (uint32)data->size());
    }
    else if(rv)
        ForceSend();

    UnlockWriteBuffer();
}

void LogonCommServerSocket::HandleSQLExecute(WorldPacket & recvData)
{
    /*string Query;
    recvData >> Query;
    sLogonSQL->Execute(Query.c_str());*/
    printf("!! WORLD SERVER IS REQUESTING US TO EXECUTE SQL. THIS IS DEPRECATED AND IS BEING IGNORED. THE SERVER WAS: %s, PLEASE UPDATE IT.\n", GetIP());
}

void LogonCommServerSocket::HandleReloadAccounts(WorldPacket & recvData)
{
    if( !IsServerAllowedMod( GetRemoteAddress().s_addr ) )
    {
        sLog.Notice("WORLD", "We received a reload request from %s, but access was denied.", GetIP());
        return;
    }

    uint32 num1;
    recvData >> num1; //uint8(42);

    if( num1 == 3 )
    {
        sLog.Notice("WORLD", "World Server at %s is forcing us to reload accounts.", GetIP());
        sAccountMgr.ReloadAccounts(false);
    }
    else
    {
        sLog.Notice("WORLD", "We received a reload request from %s, but bad packet received.", GetIP());
    }
}

void LogonCommServerSocket::HandleAuthChallenge(WorldPacket & recvData)
{
    int32 realmId = -1;
    std::string challenger;
    unsigned char key[20];
    uint32 result = 1;
    recvData.read(key, 20);
    recvData >> challenger;

    // check if we have the correct password
    if(memcmp(key, LogonServer::getSingleton().sql_hash, 20))
        result = 0;

    sLog.Notice("LogonCommServer","Authentication request from %s, result %s.", GetIP(), result ? "OK" : "FAIL");

    /* send the response packet */
    WorldPacket data(RSMSG_AUTH_RESPONSE, 8+challenger.length());
    if(result)
    {
        realmId = sInfoCore.GetRealmIdByName(challenger);
        if(realmId == -1)
            realmId = sInfoCore.GenerateRealmID();
        else
        {
            sInfoCore.TimeoutSockets();
            Realm* oldrealm = sInfoCore.GetRealm(realmId);
            if(oldrealm->Flag == REALM_FLAG_OFFLINE)
            {
                sInfoCore.RemoveRealm(realmId);
                sLog.Notice("LogonCommServer", "Challenge from %s to take place of %u.", challenger.c_str(), realmId);
            } // Set the result to failed
            else result = 2;
        }
    }

    /* packets are encrypted from now on */
    authenticated = (result==1);
    data << result;
    data << realmId;
    data << challenger;
    SendPacket(&data);

    if(authenticated)
    {
        printf("Key: ");
        for(int i = 0; i < 20; ++i)
            printf("%.2X", key[i]);
        printf("\n");

        _recv.Setup(key, 20);
        _send.Setup(key, 20);
    }
}

void LogonCommServerSocket::HandleMappingReply(WorldPacket & recvData)
{
    /* this packet is gzipped, whee! :D */
    uint32 real_size;
    recvData >> real_size;
    uLongf rsize = real_size;

    ByteBuffer buf(real_size);
    buf.resize(real_size);

    if(uncompress((uint8*)buf.contents(), &rsize, recvData.contents() + 4, (u_long)recvData.size() - 4) != Z_OK)
    {
        printf("Uncompress of mapping failed.\n");
        return;
    }

    uint32 account_id;
    uint8 number_of_characters;
    uint32 count;
    uint32 realm_id;
    buf >> realm_id;
    Realm * realm = sInfoCore.GetRealm(realm_id);
    if(!realm)
        return;

    sInfoCore.getRealmLock().Acquire();

    HM_NAMESPACE::hash_map<uint32, uint8>::iterator itr;
    buf >> count;
    sLog.Notice("LogonCommServer","Got mapping packet for realm %u, total of %u entries.\n", (unsigned int)realm_id, (unsigned int)count);
    for(uint32 i = 0; i < count; ++i)
    {
        buf >> account_id >> number_of_characters;
        itr = realm->CharacterMap.find(account_id);
        if(itr != realm->CharacterMap.end())
            itr->second = number_of_characters;
        else
            realm->CharacterMap.insert( std::make_pair( account_id, number_of_characters ) );
    }

    sInfoCore.getRealmLock().Release();
}

void LogonCommServerSocket::HandleUpdateMapping(WorldPacket & recvData)
{
    uint32 realm_id;
    uint32 account_id;
    int8 toadd;
    recvData >> realm_id;

    Realm * realm = sInfoCore.GetRealm(realm_id);
    if(!realm)
        return;

    sInfoCore.getRealmLock().Acquire();
    recvData >> account_id >> toadd;

    HM_NAMESPACE::hash_map<uint32, uint8>::iterator itr = realm->CharacterMap.find(account_id);
    if(itr != realm->CharacterMap.end())
    {
        if(itr->second > 0 || toadd > 0)
            itr->second += toadd; // Crow: Bwahahahahaha....
    }
    else
    {
        if(toadd < 0)
            toadd = 0;
        realm->CharacterMap.insert( std::make_pair( account_id, toadd ) );
    }

    sInfoCore.getRealmLock().Release();
}

void LogonCommServerSocket::HandleTestConsoleLogin(WorldPacket & recvData)
{
    WorldPacket data(RSMSG_CONSOLE_LOGIN_RESULT, 8);
    uint32 request;
    std::string accountname;
    uint8 key[20];

    recvData >> request;
    recvData >> accountname;
    recvData.read(key, 20);
    sLog.Debug("LogonCommServerSocket","Testing console login: %s\n", accountname.c_str());

    data << request;

    Account * pAccount = sAccountMgr.GetAccount(accountname);
    if(pAccount == NULL)
    {
        data << uint32(0);
        SendPacket(&data);
        return;
    }

    if(pAccount->GMFlags == NULL || strchr(pAccount->GMFlags, 'z') == NULL)
    {
        data << uint32(0);
        SendPacket(&data);
        return;
    }

    if(memcmp(pAccount->SrpHash, key, 20) != 0)
    {
        data << uint32(0);
        SendPacket(&data);
        return;
    }

    data << uint32(1);
    SendPacket(&data);
}

void LogonCommServerSocket::HandleDatabaseModify(WorldPacket& recvData)
{
    uint32 method;
    recvData >> method;

    if( !IsServerAllowedMod(GetRemoteAddress().s_addr) )
    {
        sLog.Error("LogonCommServerSocket","Database modify request %u denied for %s.\n", method, GetIP());
        return;
    }

    switch(method)
    {
    case 1:         // set account ban
        {
            std::string account;
            uint32 duration;
            std::string reason;
            recvData >> account >> duration >> reason;
            // remember we expect this in uppercase
            HEARTHSTONE_TOUPPER(account);
            Account * pAccount = sAccountMgr.GetAccount(account);
            if( pAccount == NULL )
                return;

            pAccount->Banned = duration;
            // update it in the sql (duh)
            sLogonSQL->Execute("UPDATE accounts SET banned = %u, banReason = \"%s\" WHERE login = \"%s\"", duration, sLogonSQL->EscapeString(reason).c_str(), 
                sLogonSQL->EscapeString(account).c_str());
        }break;
    case 2:     // set gm
        {
            std::string account;
            std::string gm;
            recvData >> account >> gm;
            // remember we expect this in uppercase
            HEARTHSTONE_TOUPPER(account);
            Account * pAccount = sAccountMgr.GetAccount(account);
            if( pAccount == NULL )
                return;

            pAccount->SetGMFlags( account.c_str() );
            // update it in the sql (duh)
            sLogonSQL->Execute("UPDATE accounts SET gm = \"%s\" WHERE login = \"%s\"", sLogonSQL->EscapeString(gm).c_str(), sLogonSQL->EscapeString(account).c_str());
        }break;
    case 3:     // set mute
        {
            std::string account;
            uint32 duration;
            recvData >> account >> duration;
            // remember we expect this in uppercase
            HEARTHSTONE_TOUPPER(account);
            Account * pAccount = sAccountMgr.GetAccount(account);
            if( pAccount == NULL )
                return;

            pAccount->Muted = duration;
            // update it in the sql (duh)
            sLogonSQL->Execute("UPDATE accounts SET muted = %u WHERE login = \"%s\"", duration, sLogonSQL->EscapeString(account).c_str());
        }break;
    case 4:     // ip ban add
        {
            std::string ip;
            std::string reason;
            uint32 duration;
            recvData >> ip >> duration >> reason;
            if( sIPBanner.Add( ip.c_str(), duration ) )
                sLogonSQL->Execute("INSERT INTO ipbans (ip, expire, banreason) VALUES(\"%s\", %u, \"%s\")", sLogonSQL->EscapeString(ip).c_str(), duration, sLogonSQL->EscapeString(reason).c_str() );
        }break;
    case 5:     // ip ban reomve
        {
            std::string ip;
            recvData >> ip;
            if( sIPBanner.Remove( ip.c_str() ) )
                sLogonSQL->Execute("DELETE FROM ipbans WHERE ip = \"%s\"", sLogonSQL->EscapeString(ip).c_str());
        }break;
    case 6:
        {
            uint32 accountFlags;
            std::string accountName, password, email;
            recvData >> email >> password >> accountName >> accountFlags;
            std::string safeAccountName = sLogonSQL->EscapeString(accountName);
            // remember we expect this in uppercase
            HEARTHSTONE_TOUPPER(safeAccountName);
            Account * pAccount = sAccountMgr.GetAccount(safeAccountName);
            if( pAccount != NULL )
            {
                sLog.outError("Request to create account %s, name already taken!");
                return;
            }
            HEARTHSTONE_TOLOWER(safeAccountName);
            safeAccountName[0] = (char)toupper(safeAccountName[0]);

            sLog.outString("Account %s created by request of realm %u", safeAccountName.c_str(), realmID);
            sLogonSQL->Execute("INSERT INTO accounts(login,password,flags,email) VALUES('%s', '%s', '%u', '%s')",
                safeAccountName.c_str(), sLogonSQL->EscapeString(password).c_str(), accountFlags, sLogonSQL->EscapeString(email).c_str());
        }break;
    }
}
