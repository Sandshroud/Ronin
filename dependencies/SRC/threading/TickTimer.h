/*
 * Sandshroud Project Ronin
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
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
