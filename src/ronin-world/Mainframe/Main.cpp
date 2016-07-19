/***
 * Demonstrike Core
 */

#include "StdAfx.h"

uint8 loglevel = DEFAULT_LOG_LEVEL;

#ifndef WIN32
int unix_main(int argc, char ** argv)
{
    rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1)
        printf("getrlimit failed. This could be problem.\n");
    else
    {
        rl.rlim_cur = rl.rlim_max;
        if (setrlimit(RLIMIT_CORE, &rl) == -1)
            printf("setrlimit failed. Server may not save core.dump files.\n");
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

const char* __file__ = "unknown";
size_t __line__ = 0;

void record_alloc(void *ptr, const char *file, size_t line)
{
    if(strcmp(__file__, "unknown") == 0)
        return;
    if(trackAllocation == false)
        return;
    trackAllocation = false;
    allocationRecord.insert(std::make_pair(ptr, std::make_pair(file, line)));
    trackAllocation = true;
}

void unrecord_alloc(void *ptr)
{
    allocationRecord.erase(ptr);
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

Mutex *lock = NULL;
bool trackAllocation = false;
std::map<void *, std::pair<const char*, size_t> > allocationRecord;
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
