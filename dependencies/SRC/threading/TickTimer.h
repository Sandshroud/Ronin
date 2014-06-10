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

HEARTHSTONE_INLINE uint32 getMSTime() { return timeGetTime(); }
