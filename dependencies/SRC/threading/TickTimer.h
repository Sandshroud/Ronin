/***
 * Demonstrike Core
 */

#pragma once

#if PLATFORM == PLATFORM_WIN
// gettimeofday is in mmsystem
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")
#else
uint32 timeGetTime()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return uint32((now.tv_sec * 1000) + (now.tv_usec / 1000));
}
#endif

RONIN_INLINE uint32 getMSTime() { return timeGetTime(); }
RONIN_INLINE uint32 getMSTimeDiff(uint32 newVal, uint32 oldVal)
{
    if(oldVal>newVal)
    {
        // If our old timer was from previous overrun
        if(newVal <= 0x0000FFFF && oldVal > 0xF0000000)
            return (0xFFFFFFFF-oldVal)+newVal;
        else return oldVal-newVal;
    } else return newVal-oldVal;
}
