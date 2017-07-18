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

#include "Threading.h"

#ifdef WIN32
#include <process.h>
#else

volatile int threadid_count = 0;
Mutex m_threadIdLock;
int GenerateThreadId()
{
    m_threadIdLock.Acquire();
    int i = ++threadid_count;
    m_threadIdLock.Release();
    return i;
}

#endif

SERVER_DECL ThreadManager sThreadManager;

ThreadManager::ThreadManager() : taskCounter(0)
{

}

void ThreadManager::ThreadExit(Thread * t)
{
    // we're definitely no longer active
    _mutex.Acquire();
    m_activeThreads.erase(t);
    _mutex.Release();
    
    // kill us.
    delete t;
}

void ThreadManager::ExecuteTask(const char* ThreadName, ThreadContext * ExecutionTarget)
{
    _mutex.Acquire();

    // creating a new thread...
    Thread* t = StartThread(ExecutionTarget);
    t->name = strdup(ThreadName);

    // add the thread to the active set
#ifdef WIN32
    sLog.Debug("ThreadManager", "Thread %s(%u) is now executing task at 0x%p.", t->name, t->ThreadId, ExecutionTarget);
#else
    sLog.Debug("ThreadManager", "Thread %s(%u) is now executing task at %p.", t->name, t->ThreadId, ExecutionTarget);
#endif
    m_activeThreads.insert(t);
    _mutex.Release();
}

void ThreadManager::Shutdown()
{
    _mutex.Acquire();
    sLog.Debug("ThreadManager", "Shutting down %u threads.", uint32(m_activeThreads.size()));
    for(std::map<uint32, TaskPool*>::iterator itr = m_taskPools.begin(); itr != m_taskPools.end(); ++itr)
        itr->second->kill();
    m_taskPools.clear();

    for(ThreadSet::iterator itr = m_activeThreads.begin(), itr2; itr != m_activeThreads.end();)
    {
        itr2 = itr++;
        if((*itr2)->ExecutionTarget)
            (*itr2)->ExecutionTarget->OnShutdown();
        else m_activeThreads.erase(itr2);
    }
    _mutex.Release();

    for(;;)
    {
        _mutex.Acquire();
        if(m_activeThreads.size())
        {
            sLog.Debug("ThreadManager", "%u active threads remaining...", m_activeThreads.size() );
            _mutex.Release();
            Sleep(1000);
            continue;
        }

        m_activeThreads.clear();
        _mutex.Release();
        break;
    }
}

bool RunThread(ThreadContext * target)
{
    bool res = false;
    THREAD_TRY_EXECUTION {
    res = target->run();
    } THREAD_HANDLE_CRASH;
    return res;
}

/* this is the only platform-specific code. neat, huh! */
#ifdef WIN32

static unsigned long WINAPI thread_proc(void* param)
{
    Thread * t = (Thread*)param;

    char* tName = t->name;
    uint32 tid = t->ThreadId;
    ThreadManager::SetThreadName(tName);

    if(t->ExecutionTarget != NULL)
    {
        if( RunThread( t->ExecutionTarget ) )
            delete t->ExecutionTarget;

        t->ExecutionTarget = NULL;
    }

    sThreadManager.ThreadExit(t);
    if(strlen(tName))
        sLog.Debug("ThreadManager", "Thread %s(%u) exiting.", tName, tid);
    else sLog.Debug("ThreadManager", "Thread %u exiting.", tid);

    // at this point the t pointer has already been freed, so we can just cleanly exit.
    ExitThread(0);

    // not reached
    return 0;
}

Thread * ThreadManager::StartThread(ThreadContext * ExecutionTarget)
{
    Thread * t = new Thread("ThreadStarter");
    t->ExecutionTarget = ExecutionTarget;
    CreateThread(NULL, 0, &thread_proc, (LPVOID)t, 0, (LPDWORD)&t->ThreadId);
    return t;
}

#else

static void * thread_proc(void * param)
{
    Thread * t = (Thread*)param;
    char* tName = t->name;
    uint32 tid = t->ThreadId;

    if(t->ExecutionTarget != NULL)
    {
        if(t->ExecutionTarget->run())
            delete t->ExecutionTarget;

        t->ExecutionTarget = NULL;
    }

    sThreadManager.ThreadExit(t);
    if(strlen(tName))
        sLog.Debug("ThreadManager", "Thread %s(%u) exiting.", tName, tid);
    else
        sLog.Debug("ThreadManager", "Thread %u exiting.", tid);

    //pthread_exit(0);
    return NULL;
}

Thread * ThreadManager::StartThread(ThreadContext * ExecutionTarget)
{
    Thread * t = new Thread("ThreadStarter");
    t->ExecutionTarget = ExecutionTarget;

    // lock the main mutex, to make sure id generation doesn't get messed up
    pthread_create(NULL, NULL, &thread_proc, (void*)t);
    return t;
}

#endif

void ThreadManager::SetThreadName(const char* thread_name)
{
    char buffer[255];
    strcpy(buffer, thread_name);

    DWORD threadid = GetCurrentThreadId();

    // This isn't supported on nix?
#if PLATFORM == PLATFORM_WIN
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.dwThreadID = GetCurrentThreadId();
    info.dwFlags = 0;
    info.szName = buffer;

    __try
    {
#ifdef _WIN64
        RaiseException(0x406D1388, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR*)&info);
#else
        RaiseException(0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info);
#endif
    }
    __except(EXCEPTION_CONTINUE_EXECUTION)
    {

    }
#endif
}

void ThreadManager::Suicide()
{
    uint i = 3;
    uint t = 0;
    for(uint b = 0; b < 6; b++)
        t = b/--i;
}

ThreadManager::TaskPool *ThreadManager::SpawnPool(uint32 thread_count)
{
    _mutex.Acquire();
    TaskPool *pool = new TaskPool(++taskCounter, thread_count);
    m_taskPools.insert(std::make_pair(taskCounter, pool));
    _mutex.Release();
    pool->spawn();
    return pool;
}

void ThreadManager::CleanPool(uint32 poolId)
{
    _mutex.Acquire();
    std::map<uint32, TaskPool*>::iterator itr;
    if((itr = m_taskPools.find(poolId)) != m_taskPools.end())
    {
        TaskPool *pool = itr->second;
        m_taskPools.erase(itr);
        // Pool will self terminate when all threads exit
        pool->kill();
    }
    _mutex.Release();
}

void ThreadManager::TaskPool::spawn()
{
    _processLock.Acquire();
    char buffer[MAX_PATH];
    for(uint32 i = 0; i < _threadCount; ++i)
    {
        sprintf(&buffer[0], "Pool%uTask%u", _poolId, i);
        sThreadManager.ExecuteTask(buffer, new TaskPoolSlave(this));
    }
    _processLock.Release();
}
