/***
 * Demonstrike Core
 */

#include "LogonStdAfx.h"
#include "LogonConsole.h"
#include "hearthstone_log.h"

initialiseSingleton(LogonConsole);
bool Rehash();

void LogonConsole::TranslateRehash(char* str)
{
    sLog.outString("rehashing config file...");
    Rehash();
}

void LogonConsole::Kill()
{
#ifdef WIN32
    /* write the return keydown/keyup event */
    DWORD dwTmp;
    INPUT_RECORD ir[2];
    ir[0].EventType = KEY_EVENT;
    ir[0].Event.KeyEvent.bKeyDown = TRUE;
    ir[0].Event.KeyEvent.dwControlKeyState = 288;
    ir[0].Event.KeyEvent.uChar.AsciiChar = 13;
    ir[0].Event.KeyEvent.wRepeatCount = 1;
    ir[0].Event.KeyEvent.wVirtualKeyCode = 13;
    ir[0].Event.KeyEvent.wVirtualScanCode = 28;
    ir[1].EventType = KEY_EVENT;
    ir[1].Event.KeyEvent.bKeyDown = FALSE;
    ir[1].Event.KeyEvent.dwControlKeyState = 288;
    ir[1].Event.KeyEvent.uChar.AsciiChar = 13;
    ir[1].Event.KeyEvent.wRepeatCount = 1;
    ir[1].Event.KeyEvent.wVirtualKeyCode = 13;
    ir[1].Event.KeyEvent.wVirtualScanCode = 28;
    _thread->kill=true;
    WriteConsoleInput (GetStdHandle(STD_INPUT_HANDLE), ir, 2, & dwTmp);
    printf("Waiting for console thread to terminate....\n");
    while(_thread != NULL)
    {
        Sleep(100);
    }
    printf("Console shut down.\n");
#endif
}

bool LogonConsoleThread::run()
{
    new LogonConsole();

    sLogonConsole._thread = this;
    int i = 0;
    char cmd[96];

    while (!kill)
    {
        
        // Make sure our buffer is clean to avoid Array bounds overflow
        memset(cmd,0,sizeof(cmd)); 
        // Read in single line from "stdin"
        fgets(cmd, 80, stdin);

        if(kill)
            break;

        for( i = 0 ; i < 80 || cmd[i] != '\0' ; i++ )
        {
            if( cmd[i] =='\n' )
            {
                cmd[i]='\0';
                sLogonConsole.ProcessCmd(cmd);
                fflush(stdin);
                break;
            }
        }
    }

    sLogonConsole._thread=NULL;
    return true;
}

//------------------------------------------------------------------------------
// Protected methods:
//------------------------------------------------------------------------------
// Process one command
void LogonConsole::ProcessCmd(char *cmd)
{
    typedef void (LogonConsole::*PTranslater)(char *str);
    struct SCmd
    {
        const char *name;
        PTranslater tr;
    };

    SCmd cmds[] =
    {

        {"?",        &LogonConsole::TranslateHelp},
        {"help",     &LogonConsole::TranslateHelp},
        {"reload",   &LogonConsole::ReloadAccts},
        {"rehash",   &LogonConsole::TranslateRehash},
        {"list",     &LogonConsole::ListRealms},
        {"shutdown", &LogonConsole::TranslateQuit}, 
        {"quit",     &LogonConsole::TranslateQuit}, 
        {"exit",     &LogonConsole::TranslateQuit}, 
    };

    char cmd2[80];
    strcpy(cmd2, cmd);
    for(size_t i = 0; i < strlen(cmd); ++i)
        cmd2[i] = tolower(cmd[i]);

    for (size_t i = 0; i < sizeof(cmds)/sizeof(SCmd); i++)
        if (strncmp(cmd2, cmds[i].name, strlen(cmds[i].name)) == 0)
        {
            (this->*(cmds[i].tr)) (cmd + strlen(cmds[i].name));
            return;
        }

        printf("Console:Unknown console command (use \"help\" for help).\n");
}

void LogonConsole::ReloadAccts(char *str)
{
    AccountMgr::getSingleton().ReloadAccounts(false);
    IPBanner::getSingleton().Reload();
}

void LogonConsole::ListRealms(char *str)
{
    Realm* rlm = NULL;
    sLog.outString("\nConsole:------Listing Realms--------");
    sInfoCore.getRealmLock().Acquire();
    std::map<uint32, Realm*> realms = sInfoCore.GetRealmMap();
    std::map<uint32, Realm*>::iterator itr = realms.begin();
    for(; itr != realms.end(); ++itr)
    {
        uint32 id = itr->first;
        rlm = itr->second;
        if( rlm == NULL )
            continue;

        sLog.outString("   Realm ID:   %u", id);
        sLog.outString("   Name:       %s", rlm->Name.c_str());
        sLog.outString("   Address:    %s", rlm->Address.c_str());
        sLog.outString("   Icon:       %u", rlm->Icon);
        sLog.outString("   Flag:       %u", rlm->Flag);
        sLog.outString("   Plr Count:  %u", rlm->RealPopulation);
        sLog.outString("   Plr Cap:    %u", rlm->RealmCap);
        sLog.outString("   TimeZone:   %u", rlm->WorldRegion);
        sLog.outString("   Latency:    %ums", (rlm->ServerSocket != NULL ? rlm->ServerSocket->latency : 0));
        sLog.outString("------------------------------------");
    }
    sInfoCore.getRealmLock().Release();
}

// quit | exit
void LogonConsole::TranslateQuit(char *str)
{
    int delay = str != NULL ? atoi(str) : 5000;
    if(!delay)
        delay = 5000;
    else
        delay *= 1000;

    ProcessQuit(delay);
}
void LogonConsole::ProcessQuit(int delay)
{
    mrunning = false;
}
//------------------------------------------------------------------------------
// help | ?
void LogonConsole::TranslateHelp(char *str)
{
    ProcessHelp(NULL);
}
void LogonConsole::ProcessHelp(char *command)
{
    if (command == NULL)
    {
        sLog.outString("Console:--------help--------");
        sLog.outString("   help, ?: print this text");
        sLog.outString("   reload, reloads accounts");
        sLog.outString("   rehash, rehashes config file");
        sLog.outString("   list, lists all cached realm information");
        sLog.outString("   quit, shutdown, exit: close program");
    }
}
//------------------------------------------------------------------------------

LogonConsoleThread::LogonConsoleThread() : ThreadContext()
{
    kill=false;
}

LogonConsoleThread::~LogonConsoleThread()
{

}
