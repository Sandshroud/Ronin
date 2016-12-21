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

uint8 loglevel = DEFAULT_LOG_LEVEL;

#ifndef WIN32
int unix_main(int argc, char ** argv)
{
    rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1)
        sLog.printf("getrlimit failed. This could be problem.\n");
    else
    {
        rl.rlim_cur = rl.rlim_max;
        if (setrlimit(RLIMIT_CORE, &rl) == -1)
            sLog.printf("setrlimit failed. Server may not save core.dump files.\n");
    }

    if(!sMaster.Run(argc, argv))
        return -1;
    return 0;// shouldn't be reached
}

#else

int win32_main( int argc, char ** argv )
{
    int32 result = 0;
    CThreadPool::SetThreadName( "Main Thread" );
#if defined(CRASHHANDLER_ENABLED)
    StartCrashHandler();
#endif

    THREAD_TRY_EXECUTION
    {
        result = sMaster.Run( argc, argv );
    }
    __except( HandleCrash( GetExceptionInformation() ) )
    {
        result = -1;
        exit(result);
    }

    return 0;
}

#endif

#ifdef NEW_ALLOCATION_TRACKING
#undef new

Mutex *lock = NULL;
size_t identifier = 0;
bool trackAllocation = false, controlledDestruct = false;
std::map<size_t, std::pair<const char*, size_t> > allocationRecord;

const char* __file__ = "unknown";
size_t __line__ = 0;

char ptr_buff[16+2+1];
void getPointerAddress(void *ptr)
{
    sprintf_s(ptr_buff, "0x%p", ptr);
    identifier = std::stoull(ptr_buff, nullptr, 16);
}

void record_alloc(void *ptr, const char *file, size_t line)
{
    if(strcmp(__file__, "unknown") == 0)
        return;
    if(trackAllocation == false)
        return;
    trackAllocation = false;
    getPointerAddress(ptr);
    allocationRecord.insert(std::make_pair(identifier, std::make_pair(file, line)));
    trackAllocation = true;
}

void unrecord_alloc(void *ptr)
{
    if(ptr == NULL || trackAllocation == false)
        return;

    bool res = trackAllocation;
    trackAllocation = false;
    getPointerAddress(ptr);
    trackAllocation = res;
    allocationRecord.erase(identifier);
}

void* operator new(size_t size)
{
    if(lock)
        lock->Acquire();
    void *ptr = std::malloc(size);
    record_alloc(ptr,__file__,__line__);
    __file__ = "unknown";
    __line__ = 0;
    if(lock)
        lock->Release();
    return ptr;
}

void operator delete(void *ptr)
{
    if(lock)
        lock->Acquire();
    unrecord_alloc(ptr);
    if(lock)
        lock->Release();
    std::free(ptr);
}

#endif

int main( int argc, char ** argv )
{
#ifdef NEW_ALLOCATION_TRACKING
    lock = new Mutex();
    trackAllocation = true;
#endif

    int result = 0;
#ifdef WIN32
    result = win32_main( argc, argv );
#else
    result = unix_main( argc, argv );
#endif

#ifdef NEW_ALLOCATION_TRACKING
    if(!allocationRecord.empty())
        system("PAUSE");
#endif
    return result;
}
