/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"
#include "BaseConsole.h"

class ConsoleThread : public ThreadContext
{
protected:
    bool m_isRunning;

public:
    ConsoleThread();
    ~ConsoleThread() {};

    void terminate();
    bool run();
};
