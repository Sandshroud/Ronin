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

#define BANNER "Sandshroud Ronin %s::%s r%u/%s-%s-%s :: World Server\n"

createFileSingleton( Master );
std::string LogFileName;
bool crashed = false;

volatile bool Master::m_stopEvent;

// Database defines.
SERVER_DECL DirectDatabase* Database_World;
SERVER_DECL DirectDatabase* Database_Character;
SERVER_DECL DirectDatabase* Database_State;
SERVER_DECL DirectDatabase* Database_Log;

void Master::_OnSignal(int s)
{
    switch (s)
    {
#ifndef WIN32
    case SIGHUP:
        sWorld.Rehash(true);
        break;
#endif
    case SIGINT:
    case SIGTERM:
    case SIGABRT:
#ifdef _WIN32
    case SIGBREAK:
#endif
        remove( "ronin-world.pid" );
        Master::m_stopEvent = true;
        break;
    }

    signal(s, _OnSignal);
}

Master::Master()
{

}

Master::~Master()
{

}

struct Addr
{
    unsigned short sa_family;
    /* sa_data */
    unsigned short Port;
    unsigned long IP; // inet_addr
    unsigned long unusedA;
    unsigned long unusedB;
};

#define DEF_VALUE_NOT_SET 0xDEADBEEF

#ifdef WIN32
static const char* default_config_file = "./ronin-world.ini";
#else
static const char* default_config_file = (char*)CONFDIR "/ronin-world.ini";
#endif

bool bServerShutdown = false;
bool StartConsoleListener();
void CloseConsoleListener();
ThreadContext * GetConsoleListener();

bool Master::Run(int argc, char ** argv)
{
    m_stopEvent = false;
    char * config_file = (char*)default_config_file;
    int screen_log_level = DEF_VALUE_NOT_SET;
    int do_check_conf = 0;
    int do_version = 0;
    int do_cheater_check = 0;
    int do_database_clean = 0;
    time_t curTime;

    struct startup_option longopts[] =
    {
        { "checkconf",          startup_no_argument,            &do_check_conf,         1       },
        { "screenloglevel",     startup_required_argument,      &screen_log_level,      1       },
        { "version",            startup_no_argument,            &do_version,            1       },
        { "cheater",            startup_no_argument,            &do_cheater_check,      1       },
        { "cleandb",            startup_no_argument,            &do_database_clean,     1       },
        { "conf",               startup_required_argument,      NULL,                   'c'     },
        { "realmconf",          startup_required_argument,      NULL,                   'r'     },
        { 0, 0, 0, 0 }
    };

    char c;
    while ((c = startup_getopt_long_only(argc, argv, ":f:", longopts, NULL)) != -1)
    {
        switch (c)
        {
        case 'c':
            config_file = new char[strlen(startup_optarg)];
            strcpy(config_file, startup_optarg);
            break;
        case 0:
            break;
        default:
            sLog.SetLoggingLevel(3);
            sLog.printf("Usage: %s [--checkconf] [--conf <filename>] [--realmconf <filename>] [--version]\n", argv[0]);
            return true;
        }
    }

    /* set new log levels if used as argument*/
    if( screen_log_level != (int)DEF_VALUE_NOT_SET )
        sLog.SetLoggingLevel(screen_log_level);

    // Startup banner
    UNIXTIME = time(NULL);
    g_localTime = *localtime(&UNIXTIME);

    sLog.printf(BANNER, BUILD_TAG, BUILD_HASH_STR, BUILD_REVISION, CONFIG, PLATFORM_TEXT, ARCH);
    sLog.Line();

    sLog.printf( "The key combination <Ctrl-C> will safely shut down the server at any time.\n" );
    sLog.Line();

#ifndef WIN32
    if(geteuid() == 0 || getegid() == 0)
        sLog.LargeErrorMessage( LARGERRORMESSAGE_WARNING, "You are running Ronin as root.", "This is not needed, and may be a possible security risk.", "It is advised to hit CTRL+C now and", "start as a non-privileged user.", NULL);
#endif

    InitRandomNumberGenerators();
    sLog.Success( "Rnd", "Initialized Random Number Generators." );

    mainIni = new CIniFile(config_file);
    uint32 LoadingTime = getMSTime();
    sLog.Notice( "Config", "Loading Config Files..." );
    if( !mainIni->ParseError() )
        sLog.Success( "Config", ">> ronin-world.ini" );
    else
    {
        sLog.Error( "Config", ">> ronin-world.ini" );
        return false;
    }

    int logLevel = 1;
    //use these log_level until we are fully started up.
    if(mainIni->ReadInteger("LogLevel", "Screen", 1) == -1)
    {
        sLog.Notice("Master", "Running silent mode...");
        logLevel = -1;
    }
#ifdef _DEBUG
    else logLevel = 3;
#endif // _DEBUG
    sLog.Init(logLevel);

    sDBEngine.Init(false);

    if(!_StartDB())
    {
        DirectDatabase::CleanupLibs();
        return false;
    }

    sLog.Line();
    sLog.outString("");

    new OpcodeManager();
    new World();

    /* load the config file */
    sWorld.Rehash(true);
    // Set our opcode data
    sOpcodeMgr.SetOpcodeListData();

    // Because of our log DB system, these have to be initialized different then rehash.
    sWorld.LogCheaters = mainIni->ReadBoolean("Log", "Cheaters", false);
    sWorld.LogCommands = mainIni->ReadBoolean("Log", "GMCommands", false);
    sWorld.LogPlayers = mainIni->ReadBoolean("Log", "Player", false);
    sWorld.bLogChat = mainIni->ReadBoolean("Log", "Chat", false);

    //Update log to obey config setting
    sLog.Init(mainIni->ReadInteger("LogLevel", "Screen", 1));

    // Initialize Opcode Table
    WorldSession::InitPacketHandlerTable();

    std::string host = mainIni->ReadString( "Listen", "Host", DEFAULT_HOST );
    int wsport = mainIni->ReadInteger( "RealmData", "WorldServerPort", DEFAULT_WORLDSERVER_PORT );

    if( !sWorld.SetInitialWorldSettings() )
    {
        sLog.Error( "Server", "SetInitialWorldSettings() failed. Something went wrong? Exiting." );
        return false;
    }

    sWorld.SetStartTime(uint32(UNIXTIME));

    // Initialize the new worldRunnable
    sThreadManager.ExecuteTask("WorldThread", World::getSingletonPtr());

    _HookSignals();

    ConsoleThread* console = new ConsoleThread();
    sThreadManager.ExecuteTask("ConsoleThread", console);

    // Start Network Subsystem
    sLog.Debug("Server","Starting network subsystem..." );
    CreateSocketEngine(std::min<int8>(16, mainIni->ReadInteger("ServerSettings", "NetworkThreads", 2)));
    sSocketEngine.SpawnThreads();

    if( StartConsoleListener() )
        sLog.Success("RemoteConsole", "Started and listening on port %i", mainIni->ReadInteger("RemoteConsole", "Port", 8092));
    else sLog.Debug("RemoteConsole", "Not enabled or failed listen.");

    LoadingTime = getMSTime() - LoadingTime;
    sLog.Success("Server","Ready for connections. Startup time: %ums\n", LoadingTime );

    /* write pid file */
    FILE * fPid = fopen( "ronin-world.pid", "w" );
    if( fPid )
    {
        uint32 pid;
#ifdef WIN32
        pid = GetCurrentProcessId();
#else
        pid = getpid();
#endif
        fprintf( fPid, "%u", uint(pid) );
        fclose( fPid );
    }
#ifdef WIN32
    HANDLE hThread = GetCurrentThread();
#endif

    if(mainIni->ReadInteger("LogLevel", "Screen", 1) == -1)
    {
        sLog.Init(1);
        sLog.Notice("Master", "Leaving Silent Mode...");
    }

    /* Connect to realmlist servers / logon servers */
    new LogonCommHandler();
    sLogonCommHandler.Startup();

    ListenSocket<WorldSocket> * ls = new ListenSocket<WorldSocket>();
    bool listnersockcreate = ls->Open(host.c_str(), wsport);

    sLog.SetDelayPrint(true);
    while( !m_stopEvent && listnersockcreate )
    {
        uint32 diff = sLog.Update(25);

        /* since time() is an expensive system call, we only update it once per server loop */
        curTime = time(NULL);
        if( UNIXTIME != curTime )
        {
            UNIXTIME = curTime;
            g_localTime = *localtime(&curTime);
            UpdateRandomNumberGenerators();
        }

        sLogonCommHandler.UpdateSockets(diff);
        sSocketDeleter.Update();
    }

    sLog.SetDelayPrint(false);
    sLog.Update(1);

    // begin server shutdown
    sLog.Notice( "Shutdown", "Initiated at %s", RONIN_UTIL::ConvertTimeStampToDataTime( (uint32)UNIXTIME).c_str() );
    bServerShutdown = true;

    if( lootmgr.is_loading )
    {
        sLog.Notice( "Shutdown", "Waiting for loot to finish loading..." );
        while( lootmgr.is_loading )
            Sleep( 100 );
    }

    while( sWorldMgr.HasLoadingMaps() )
        Sleep( 100 );

    sDBEngine.EndThreads();

    sLog.Notice( "Database", "Clearing all pending queries..." );

    // kill the database thread first so we don't lose any queries/data
    CharacterDatabase.EndThreads();
    WorldDatabase.EndThreads();

    if(Database_Log)
        LogDatabase.EndThreads();

    guildmgr.SaveAllGuilds();
    sWorld.LogoutPlayers(); //(Also saves players).

    sLog.Notice("Server", "Shutting down random generator.");
    CleanupRandomNumberGenerators();

    ls->Disconnect();

    sLog.Notice( "Network", "Shutting down network subsystem." );
    sSocketEngine.Shutdown();

    sAddonMgr.SaveToDB();
    sLog.Notice("AddonMgr", "~AddonMgr()");
    delete AddonMgr::getSingletonPtr();

    sLog.Notice("LootMgr", "~LootMgr()");
    delete LootMgr::getSingletonPtr();

    sLog.Notice("MailSystem", "~MailSystem()");
    delete MailSystem::getSingletonPtr();

    /* Shut down console system */
    CloseConsoleListener();
    console->terminate();
    delete console;

    sLog.Notice("Thread", "Terminating thread pool...");
    sThreadManager.Shutdown();
    DBCLoader::StartCleanup();

    sLog.Notice( "Network", "Deleting Network Subsystem..." );
    {
        /* delete the socket deleter */
        delete SocketDeleter::getSingletonPtr();

        /* delete the socket engine */
        delete SocketEngine::getSingletonPtr();

        /* WSA network cleanup */
        WSACleanup();
    }

    sLog.Notice("Config", "~Config()");
    delete mainIni;

    sLog.Notice("LogonComm", "~LogonCommHandler()");
    delete LogonCommHandler::getSingletonPtr();

    sLog.Notice( "World", "~World()" );
    sWorld.Destruct();

    sLog.Notice("OpcodeManager", "~OpcodeManager()");
    delete OpcodeManager::getSingletonPtr();

    // Wait for cleanup thread to exit
    sThreadManager.Shutdown();

    sLog.Notice( "Database", "Closing Connections..." );
    _StopDB();

    _UnhookSignals();

    // remove pid
    remove( "ronin-world.pid" );

    sLog.Notice( "Shutdown", "Shutdown complete." );
    return true;
}

bool Master::_StartDB()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo( &sysinfo );

    std::string error;
    // Configure World Database...
    std::string hostname = mainIni->ReadString("WorldDatabase", "Hostname", "ERROR");
    std::string username = mainIni->ReadString("WorldDatabase", "Username", "ERROR");
    std::string password = mainIni->ReadString("WorldDatabase", "Password", "ERROR");
    std::string database = mainIni->ReadString("WorldDatabase", "Name", "ERROR");
    int port = mainIni->ReadInteger("WorldDatabase", "Port", 0);
    int type = mainIni->ReadInteger("WorldDatabase", "Type", 0);
    if(strcmp(hostname.c_str(), "ERROR") == 0)
        error.append("Hostname");
    else if(strcmp(username.c_str(), "ERROR") == 0)
        error.append("Username");
    else if(strcmp(password.c_str(), "ERROR") == 0)
        error.append("Password");
    else if(strcmp(database.c_str(), "ERROR") == 0)
        error.append("DatabaseName");
    else if(port == 0 || type == 0)
        error.append("Port/Type");
    if(error.length())
    {
        sLog.outError("sql: World database parameter not found for %s.", error.c_str());
        return false;
    }

    Database_World = DirectDatabase::Create();
    // Initialize it
    if( !WorldDatabase.Initialize(hostname.c_str(), uint(port), username.c_str(),
        password.c_str(), database.c_str(), mainIni->ReadInteger("WorldDatabase", "ConnectionCount", sysinfo.dwNumberOfProcessors), 16384 ) )
    {
        sLog.outDebug( "sql: Main database initialization failed. Exiting." );
        return false;
    }

    hostname = mainIni->ReadString("CharacterDatabase", "Hostname", "ERROR");
    username = mainIni->ReadString("CharacterDatabase", "Username", "ERROR");
    password = mainIni->ReadString("CharacterDatabase", "Password", "ERROR");
    database = mainIni->ReadString("CharacterDatabase", "Name", "ERROR");
    port = mainIni->ReadInteger("CharacterDatabase", "Port", 0);
    type = mainIni->ReadInteger("CharacterDatabase", "Type", 0);
    if(strcmp(hostname.c_str(), "ERROR") == 0)
        error.append("Hostname");
    else if(strcmp(username.c_str(), "ERROR") == 0)
        error.append("Username");
    else if(strcmp(password.c_str(), "ERROR") == 0)
        error.append("Password");
    else if(strcmp(database.c_str(), "ERROR") == 0)
        error.append("DatabaseName");
    else if(port == 0 || type == 0)
        error.append("Port/Type");
    if(error.length())
    {
        sLog.outError("sql: Character database parameter not found for %s.", error.c_str());
        return false;
    }

    Database_Character = DirectDatabase::Create();
    // Initialize it
    if( !CharacterDatabase.Initialize( hostname.c_str(), uint(port), username.c_str(),
        password.c_str(), database.c_str(), mainIni->ReadInteger( "CharacterDatabase", "ConnectionCount", sysinfo.dwNumberOfProcessors), 16384 ) )
    {
        sLog.outDebug( "sql: Main database initialization failed. Exiting." );
        return false;
    }

    hostname = mainIni->ReadString("StateDatabase", "Hostname", "ERROR");
    username = mainIni->ReadString("StateDatabase", "Username", "ERROR");
    password = mainIni->ReadString("StateDatabase", "Password", "ERROR");
    database = mainIni->ReadString("StateDatabase", "Name", "ERROR");
    port = mainIni->ReadInteger("StateDatabase", "Port", 0);
    type = mainIni->ReadInteger("StateDatabase", "Type", 0);
    if(strcmp(hostname.c_str(), "ERROR") == 0)
        error.append("Hostname");
    else if(strcmp(username.c_str(), "ERROR") == 0)
        error.append("Username");
    else if(strcmp(password.c_str(), "ERROR") == 0)
        error.append("Password");
    else if(strcmp(database.c_str(), "ERROR") == 0)
        error.append("DatabaseName");
    else if(port == 0 || type == 0)
        error.append("Port/Type");
    if(error.length())
    {
        sLog.outError("sql: Instance database parameter not found for %s.", error.c_str());
        return false;
    }

    Database_State = DirectDatabase::Create();
    // Initialize it
    if( !StateDatabase.Initialize( hostname.c_str(), uint(port), username.c_str(),
        password.c_str(), database.c_str(), mainIni->ReadInteger( "StateDatabase", "ConnectionCount", sysinfo.dwNumberOfProcessors), 16384 ) )
    {
        sLog.outDebug( "sql: Main database initialization failed. Exiting." );
        return false;
    }

    Database_Log = NULL;
    if(mainIni->ReadBoolean("Log", "Cheaters", false) || mainIni->ReadBoolean("Log", "GMCommands", false)
        || mainIni->ReadBoolean("Log", "Player", false) || mainIni->ReadBoolean("Log", "Chat", false))
    {
        hostname = mainIni->ReadString("LogDatabase", "Hostname", "ERROR");
        username = mainIni->ReadString("LogDatabase", "Username", "ERROR");
        password = mainIni->ReadString("LogDatabase", "Password", "ERROR");
        database = mainIni->ReadString("LogDatabase", "Name", "ERROR");
        port = mainIni->ReadInteger("LogDatabase", "Port", 0);
        type = mainIni->ReadInteger("LogDatabase", "Type", 0);
        if(strcmp(hostname.c_str(), "ERROR") == 0)
            error.append("Hostname");
        else if(strcmp(username.c_str(), "ERROR") == 0)
            error.append("Username");
        else if(strcmp(password.c_str(), "ERROR") == 0)
            error.append("Password");
        else if(strcmp(database.c_str(), "ERROR") == 0)
            error.append("DatabaseName");
        else if(port == 0 || type == 0)
            error.append("Port/Type");
        if(error.length())
        {
            sLog.outError("sql: Log database parameter not found for %s.", error.c_str());
            return false;
        }

        Database_Log = DirectDatabase::Create();
        // Initialize it
        if( !(LogDatabase.Initialize( hostname.c_str(), uint(port), username.c_str(),
            password.c_str(), database.c_str(), mainIni->ReadInteger( "LogDatabase", "ConnectionCount", 2 ), 16384 )) )
        {
            sLog.outDebug( "sql: Log database initialization failed. Exiting." );
            return false;
        }
    }

    return true;
}

void Master::_StopDB()
{
    if(Database_World)
        delete Database_World;
    Database_World = NULL;
    if(Database_Character)
        delete Database_Character;
    Database_Character = NULL;
    if(Database_State)
        delete Database_State;
    Database_State = NULL;
    if(Database_Log)
        delete Database_Log;
    Database_Log = NULL;
}

#ifndef WIN32
// Unix crash handler :oOoOoOoOoOo
volatile bool m_crashed = false;
void segfault_handler(int c)
{
    if( m_crashed )
    {
        abort();
        return;     // not reached
    }

    m_crashed = true;

    sLog.printf ("Segfault handler entered...\n");
    try
    {
        if( World::getSingletonPtr() != 0 )
        {
            sLog.outString( "Waiting for all database queries to finish..." );
            if(Database_Log)
                LogDatabase.EndThreads();
            WorldDatabase.EndThreads();

            sLog.outString( "All pending database operations cleared.\n" );
            sWorld.SaveAllPlayers();
            guildmgr.SaveAllGuilds();
            objmgr.CorpseCollectorUnload(true);
            CharacterDatabase.EndThreads();
            sLog.outString( "Data saved." );
        }
    }
    catch(...)
    {
        sLog.outString( "Threw an exception while attempting to save all data." );
    }

    sLog.printf("Writing coredump...\n");
    abort();
}
#endif


void Master::_HookSignals()
{
    signal( SIGINT, _OnSignal );
    signal( SIGTERM, _OnSignal );
    signal( SIGABRT, _OnSignal );
#ifdef _WIN32
    signal( SIGBREAK, _OnSignal );
#else
    signal( SIGHUP, _OnSignal );
    signal(SIGUSR1, _OnSignal);

    // crash handler
    signal(SIGSEGV, segfault_handler);
    signal(SIGFPE, segfault_handler);
    signal(SIGILL, segfault_handler);
    signal(SIGBUS, segfault_handler);
#endif
}

void Master::_UnhookSignals()
{
    signal( SIGINT, 0 );
    signal( SIGTERM, 0 );
    signal( SIGABRT, 0 );
#ifdef _WIN32
    signal( SIGBREAK, 0 );
#else
    signal( SIGHUP, 0 );
#endif
}

#ifdef WIN32

Mutex m_crashedMutex;

// Crash Handler
void OnCrash( bool Terminate )
{
    sLog.outString( "Advanced crash handler initialized." );

    if( !m_crashedMutex.AttemptAcquire() )
        TerminateThread( GetCurrentThread(), 0 );

    try
    {
        if( World::getSingletonPtr() != 0 )
        {
            sLog.outString( "Waiting for all database queries to finish..." );
            if(Database_Log)
                LogDatabase.EndThreads();

            WorldDatabase.EndThreads();
            sWorld.SaveAllPlayers();
            guildmgr.SaveAllGuilds();
            objmgr.CorpseCollectorUnload(true);
            CharacterDatabase.EndThreads();

            sLog.outString( "All pending database operations cleared.\n" );
            sLog.outString( "Data saved." );
        }
    }
    catch(...)
    {
        sLog.outString( "Threw an exception while attempting to save all data." );
    }

    sLog.outString( "Closing." );

    // Terminate Entire Application
    if( Terminate )
    {
        HANDLE pH = OpenProcess( PROCESS_TERMINATE, TRUE, GetCurrentProcessId() );
        TerminateProcess( pH, 1 );
        CloseHandle( pH );
    }
}

#endif
