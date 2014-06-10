/***
 * Demonstrike Core
 */

#ifndef __LOGONCONSOLE_H
#define __LOGONCONSOLE_H

#include "Common.h"

class LogonConsoleThread : public ThreadContext
{
public:
    bool kill;
    LogonConsoleThread();
    ~LogonConsoleThread();
    bool run();
};

class LogonConsole :  public Singleton < LogonConsole >
{
    friend class LogonConsoleThread;

public:                     // Public methods:
    void Kill();

protected:                  // Protected methods:
    LogonConsoleThread *_thread;

    // Process one command
    void ProcessCmd(char *cmd);

    // quit | exit
    void TranslateQuit(char *str);
    void ProcessQuit(int delay);
    void CancelShutdown(char *str);

    // help | ?
    void TranslateHelp(char *str);
    void ProcessHelp(char *command);

    void ReloadAccts(char *str);
    void TranslateRehash(char* str);
    void ListRealms(char *str);
};

#define sLogonConsole LogonConsole::getSingleton()

#endif
