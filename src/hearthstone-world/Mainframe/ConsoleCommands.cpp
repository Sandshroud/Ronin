/***
 * Demonstrike Core
 */

#include "StdAfx.h"

bool HandleInfoCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    int gm = 0;
    int count = 0;
    int avg = 0;
    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        if(itr->second->GetSession())
        {
            count++;
            avg += itr->second->GetSession()->GetLatency();
            if(itr->second->GetSession()->GetPermissionCount())
                gm++;
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    pConsole->Write("======================================================================\r\n");
    pConsole->Write("Server Information: \r\n");
    pConsole->Write("======================================================================\r\n");
    pConsole->Write("Server Revision: SS Hearthstone(%s::%s) r%u/%s-%s-%s\r\n", BUILD_TAG, BUILD_HASH_STR, BUILD_REVISION, CONFIG, PLATFORM_TEXT, ARCH);
    pConsole->Write("Server Uptime: %s\r\n", sWorld.GetUptimeString().c_str());
    pConsole->Write("Useage(Win only): RAM:(%f), CPU:(%f)\r\n", sWorld.GetRAMUsage(), sWorld.GetCPUUsage());
    pConsole->Write("SQL Query Cache Size: (W: %u/C: %u) queries delayed\r\n", WorldDatabase.GetQueueSize(), CharacterDatabase.GetQueueSize());
    pConsole->Write("Active Thread Count: %u\r\n", ThreadPool.GetActiveThreadCount());
    pConsole->Write("Players Online: (%u Alliance/%u Horde/%u GMs)\r\n",sWorld.AlliancePlayers, sWorld.HordePlayers, gm);
    pConsole->Write("Average Latency: %.3fms\r\n", count ?  ((float)((float)avg / (float)count)) : 0.0f);
    pConsole->Write("Accepted Connections: %u\r\n", sWorld.mAcceptedConnections);
    pConsole->Write("Connection Peak: %u\r\n", sWorld.PeakSessionCount);
    pConsole->Write("Logonserver Latency: %u\r\n", sLogonCommHandler.GetLatency());
    pConsole->Write("Network Stress(In/Out): %fkb/%fkb\r\n", sWorld.NetworkStressIn, sWorld.NetworkStressOut);
    pConsole->Write("======================================================================\r\n\r\n");
    return true;
}

bool HandleSuicideCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    int32 i = 1;
    { i = 3/--i; }
    return true;
}

bool HandleGMsCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    WorldPacket data;
    //bool first = true;

    pConsole->Write("There are the following GM's online on this server: \r\n");
    pConsole->Write("======================================================\r\n");
    pConsole->Write("| %21s | %15s | %04s  |\r\n" , "Name", "Permissions", "Latency");
    pConsole->Write("======================================================\r\n");

    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        if(itr->second->GetSession()->GetPermissionCount())
        {
            pConsole->Write("| %21s | %15s | %04u ms |\r\n" , itr->second->GetName(), itr->second->GetSession()->GetPermissions(), itr->second->GetSession()->GetLatency());
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    pConsole->Write("======================================================\r\n\r\n");
    return true;
}


void ConcatArgs(string & outstr, int argc, int startoffset, const char * argv[])
{
    for(int i = startoffset + 1; i < argc; i++)
    {
        outstr += argv[i];
        if((i+1) != (argc))
            outstr += " ";
    }
}
bool HandleAnnounceCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    char pAnnounce[1024];
    string outstr;
    if(argc < 2)
        return false;

    ConcatArgs(outstr, argc, 0, argv);
    snprintf(pAnnounce, 1024, "%sConsole: |r%s", MSG_COLOR_LIGHTBLUE, outstr.c_str());
    sWorld.SendWorldText(pAnnounce); // send message
    pConsole->Write("Message sent.\r\n");
    return true;
}

bool HandleWAnnounceCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    char pAnnounce[1024];
    string outstr;
    if(argc < 2)
        return false;

    ConcatArgs(outstr, argc, 0, argv);
    snprintf(pAnnounce, 1024, "%sConsole: |r%s", MSG_COLOR_LIGHTBLUE, outstr.c_str());
    sWorld.SendWorldWideScreenText(pAnnounce); // send message
    pConsole->Write("Message sent.\r\n");
    return true;
}

bool HandleKickCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    if(argc < 3)
        return false;

    char pAnnounce[1024];
    Player* pPlayer;

    pPlayer = objmgr.GetPlayer(argv[1]);
    if( pPlayer == NULL )
    {
        pConsole->Write("Could not find player, %s.\r\n", argv[1]);
        return true;
    }
    string outstr;
    ConcatArgs(outstr, argc, 1, argv);
    snprintf(pAnnounce, 1024, "%sConsole:|r %s was kicked from the server for: %s.", MSG_COLOR_LIGHTBLUE, pPlayer->GetName(), argv[2]);
    pPlayer->BroadcastMessage("You were kicked by the console for: %s", argv[2]);
    sWorld.SendWorldText(pAnnounce, NULL);
    pPlayer->Kick(5000);
    pConsole->Write("Kicked player %s.\r\n", pPlayer->GetName());
    return true;
}

bool HandleRestartCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    uint32 delay = 5;
    if(argc >= 2)
        delay = atoi(argv[1]);

    pConsole->Write("Shutdown initiated.\r\n");
    sWorld.QueueShutdown(delay, SERVER_SHUTDOWN_TYPE_RESTART);
    return true;
}

bool HandleQuitCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    uint32 delay = 5;
    if(argc >= 2)
        delay = atoi(argv[1]);

    pConsole->Write("Shutdown initiated.\r\n");
    sWorld.QueueShutdown(delay, SERVER_SHUTDOWN_TYPE_SHUTDOWN);
    return true;
}

bool HandleCancelCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    pConsole->Write("Shutdown canceled.\r\n");
    sWorld.CancelShutdown();
    return true;
}

bool HandleUptimeCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    string up = sWorld.GetUptimeString();
    pConsole->Write("Server Uptime: %s\r\n", up.c_str());
    return true;
}

bool HandleBanAccountCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    if(argc < 4)
        return false;

    int32 timeperiod = GetTimePeriodFromString(argv[2]);
    if(timeperiod < 0)
        return false;

    uint32 banned = (timeperiod ? (uint32)UNIXTIME+timeperiod : 1);

    /// apply instantly in db
    sLogonCommHandler.Account_SetBanned(argv[1], banned, argv[3]);

    pConsole->Write("Account '%s' has been banned %s%s. The change will be effective with the next reload cycle.\r\n", argv[1],
        timeperiod ? "until " : "forever", timeperiod ? ConvertTimeStampToDataTime(timeperiod+(uint32)UNIXTIME).c_str() : "");

    return true;
}

bool HandleUnbanAccountCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    if(argc < 2)
        return false;

    sLogonCommHandler.Account_SetBanned(argv[1], 0, "");

    pConsole->Write("Account '%s' has been unbanned.\r\n", argv[1]);
    return true;
}

bool HandleCreateAccountCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    if(argc < 5)
        return false;

    //<name> <pass> <email> <flags>
    const char * username = argv[1];
    const char * password = argv[2];
    const char * email = argv[3];
    const char * flags = argv[4];

    if(strlen(username) == 0 || strlen(password) == 0 || strlen(email) == 0 || strlen(flags) == 0)
        return false;

    sLogonCommHandler.SendCreateAccountRequest(username, password, email, atol(flags));
    return true;
}

bool HandleMOTDCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    if(argc < 2)
    {
        pConsole->Write( "The current motd is: '%s'.\r\n", sWorld.GetMotd() );
    }else
    {
        char set_motd[1024];
        string outstr;
        ConcatArgs( outstr, argc, 0, argv );
        snprintf( set_motd, 1024, "%s", outstr.c_str() );

        sWorld.SetMotd( set_motd );
        pConsole->Write( "The motd has been set to: '%s'.\r\n", sWorld.GetMotd() );
    }
    return true;
}

bool HandlePlayerInfoCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    if(argc < 2)
        return false;

    Player* plr = objmgr.GetPlayer(argv[1]);
    if( plr == NULL )
    {
        pConsole->Write("Player not found.\r\n");
        return true;
    }

    pConsole->Write("Player: %s\r\n", plr->GetName());
    pConsole->Write("Race: %s\r\n", plr->myRace->Name);
    pConsole->Write("Class: %s\r\n", plr->myClass->name);
    pConsole->Write("IP: %s\r\n", plr->GetSession()->GetSocket() ? plr->GetSession()->GetSocket()->GetIP() : "disconnected");
    pConsole->Write("Level: %u\r\n", plr->getLevel());
    pConsole->Write("Account: %s\r\n", plr->GetSession()->GetAccountNameS());
    return true;
}

void TestConsoleLogin(string& username, string& password, uint32 requestno)
{
    sLogonCommHandler.TestConsoleLogon(username, password, requestno);
}

bool HandleRehashCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    pConsole->Write("Config file re-parsed.\n");
    sWorld.Rehash(false);
    return true;
}

bool HandleBackupDBCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    sWorld.BackupDB();
    return true;
}

bool HandleSaveAllCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    PlayerStorageMap::const_iterator itr;
    uint32 stime = getMSTime(), count = 0;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        if(itr->second->GetSession())
        {
            itr->second->SaveToDB(false);
            count++;
        }
    }
    objmgr._playerslock.ReleaseReadLock();
    char msg[100];
    snprintf(msg, 100, "Saved %u online players in %ums.", count, getMSTime() - stime);
    sWorld.SendWorldText(msg);
    sWorld.SendWorldWideScreenText(msg);

    return true;
}
bool HandleWhisperCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    char pAnnounce[1024];
    Player* pPlayer;
    string outstr;

    if(argc < 3)
        return false;

    pPlayer = objmgr.GetPlayer(argv[1],false);

    if( pPlayer == NULL )
    {
        pConsole->Write("Could not find player, %s.\r\n", argv[1]);
        return true;
    }

    ConcatArgs(outstr, argc, 1, argv);
    snprintf(pAnnounce, 1024, "%sConsole whisper: |r%s", MSG_COLOR_MAGENTA, outstr.c_str());

    pPlayer->BroadcastMessage(pAnnounce);
    pConsole->Write("Message sent to %s.\r\n", pPlayer->GetName());
    return true;
}
bool HandleNameHashCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    if( !argc )
        return false;
    string spstring;
        ConcatArgs(spstring, argc, 0, argv);
    uint32 spellid = (int)atoi((char*)spstring.c_str());
    SpellEntry * sp = dbcSpell.LookupEntry(spellid);
    if ( !sp )
    {
        pConsole->Write( "Spell %u could not be found in spell.dbc\n", spellid );
        return true;
    }
    else
    {
        pConsole->Write( "Name Hash for %u [%s] is 0x%X\n" ,sp->Id, sp->Name , uLong(sp->NameHash));
        return true;
    }
}
bool HandleOnlinePlayersCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    WorldPacket data;
    //bool first = true;

    pConsole->Write("The following players are online on this server: \r\n");
    pConsole->Write("======================================================\r\n");
    pConsole->Write("| %21s | %15s | % 04s  |\r\n" , "Name", "Level", "Latency");
    pConsole->Write("======================================================\r\n");

    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        pConsole->Write("| %21s | %15u | %04u ms |\r\n" , itr->second->GetName(), itr->second->GetSession()->GetPlayer()->getLevel(), itr->second->GetSession()->GetLatency());
    }
    objmgr._playerslock.ReleaseReadLock();

    pConsole->Write("======================================================\r\n\r\n");
    return true;
}

bool HandleReloadAllScriptsCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
//  sScriptMgr.ReloadScripts();
    pConsole->Write("DLL Scripts reloaded.\n");
    return true;
}

bool HandleUnloadSingleScriptCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    if(argc < 1)
        return false;

    // TODO
    const char* pie = argv[1];
    pConsole->Write("DLL Script %s unloaded.\n", pie);
    return true;
}

bool HandleUnloadAllScriptsCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    sScriptMgr.UnloadScripts();
    pConsole->Write("DLL Scripts unloaded.\n");
    return true;
}

bool HandleLoadNewScriptsCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    uint32 count = 0;
    // TODO
    pConsole->Write("%u DLL scripts loaded.\n", count);
    return true;
}

bool HandleLoadSingleScriptCommand(BaseConsole * pConsole, int argc, const char * argv[])
{
    if(argc < 1)
        return false;

    // TODO
    const char* pie = argv[1];
    pConsole->Write("DLL Script %s loaded.\n", pie);
    return true;
}
