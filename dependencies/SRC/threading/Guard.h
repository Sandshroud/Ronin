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

#include "Mutex.h"

/************************************************************************/
/* Guard class, unlocks mutex on destroy                                */
/************************************************************************/
// crossplatform :)

class SERVER_DECL Guard
{
public:
    Guard(Mutex& mutex) : target(mutex) { target.Acquire(); }

    ~Guard() { target.Release(); }

    // Disable = operator
    Guard& operator=(Guard& src) = delete;

protected:
    Mutex& target;
};

class SERVER_DECL RWGuard
{
public:
    RWGuard(RWMutex& mutex, bool high) : target(mutex), highLock(high)
    {
        if(highLock)
            target.HighAcquire();
        else target.LowAcquire();
    }

    ~RWGuard()
    {
        if(highLock)
            target.HighRelease();
        else target.LowRelease();
    }

    // Disable = operator
    RWGuard& operator=(Guard& src) = delete;

protected:
    bool highLock;
    RWMutex& target;
};
