/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"
#include <consolelog/consolelog.h>

class SERVER_DECL hLog : public basicLog, public Singleton<hLog>
{
public:
    bool isOutProcess() { return false; };
    bool isOutDevelopment() { return false; };

private:
    virtual time_t GetTime() { return UNIXTIME; };

    virtual void AcquireLock() { logLock.Acquire(); };
    virtual void ReleaseLock() { logLock.Release(); };
    Mutex logLock;

public:
    void InitializeUnderlayingLog() { SetBasicLog(this); }
};

#define sLog hLog::getSingleton()
