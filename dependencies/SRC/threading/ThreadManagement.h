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

typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // must be 0x1000
    LPCSTR szName; // pointer to name (in user addr space)
    DWORD dwThreadID; // thread ID (-1=caller thread)
    DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

struct SERVER_DECL Thread
{
    char* name;
    uint32 ThreadId;
    Thread(char* tname) { name = tname; }

    ThreadContext * ExecutionTarget;
};


class SERVER_DECL ThreadManager
{
    Mutex _mutex;

    typedef std::set<Thread*> ThreadSet;
    ThreadSet m_activeThreads;

public:
    ThreadManager();

    // shutdown all threads
    void Shutdown();

    // return true - suspend ourselves, and wait for a future task.
    // return false - exit, we're shutting down or no longer needed.
    void ThreadExit(Thread * t);

    // creates a thread, returns a handle to it.
    Thread * StartThread(ThreadContext * ExecutionTarget);

    // grabs/spawns a thread, and tells it to execute a task.
    void ExecuteTask(const char* ThreadName, ThreadContext * ExecutionTarget);

    // gets active thread count
    RONIN_INLINE uint32 GetActiveThreadCount() { return (uint32)m_activeThreads.size(); }

    // Creates exception which sets the thread name, do not call inside try block
    static void SetThreadName(const char* format);

    static void Suicide();

public:
    class PoolTask { public: virtual int call() { return 0; } };

    class TaskPool
    {
    public:
        TaskPool(uint32 poolId, uint32 thread_count) : _dead(false), taskCount(0), _poolId(poolId), _threadCount(thread_count)
        {
#if PLATFORM == PLATFORM_WIN
            inputEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            endEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
        }
        ~TaskPool() {}

        void attach()
        {
            _processLock.Acquire();
            ++_threadCount; // Increment thread counter since this thread is waiting inside the pointer
            _processLock.Release();
        }

        void AddTask(PoolTask* task) 
        {
            Guard guard(_processLock);
            if(_dead == true)
            {
                delete task;
                return;
            }

            ++taskCount;
            _processQueue.push_back(task);
#if PLATFORM == PLATFORM_WIN
            SetEvent(inputEvent);
#else
            pthread_cond_signal(&cond);
#endif
        }

        void wait()
        {
            while(!_IsTasksEmpty())
            {
#if PLATFORM == PLATFORM_WIN
                WaitForSingleObject(endEvent, 100);
#else
                timeval now;
                timespec tv;
                gettimeofday(&now, NULL);
                tv.tv_sec = now.tv_sec;
                tv.tv_nsec = (now.tv_usec * 1000) + 100;

                pthread_mutex_lock(&mutex);
                pthread_cond_timedwait(&cond, &mutex, &tv);
                pthread_mutex_unlock(&mutex);
#endif
            }
#if PLATFORM == PLATFORM_WIN
            ResetEvent(inputEvent);
            ResetEvent(endEvent);
#endif
        }

        void kill()
        {
            _processLock.Acquire();
            _dead = true;
            uint32 threadCount = (--_threadCount);
            _processLock.Release();
            if(threadCount == 0)
                delete this;
        }

        uint32 getPoolId() { return _poolId; }
        uint32 getThreadCount() { return _threadCount; }

    private:
        friend class ThreadManager;
        class TaskPoolSlave : public ThreadContext
        {
        public:
            TaskPoolSlave(TaskPool *pool) : ThreadContext(), _pool(pool) {}
            virtual bool run() { _pool->slave_run(); return true; }
        private:
            TaskPool *_pool;
        };
        friend class TaskPoolSlave;

        void slave_run()
        {
            for(;;)
            {
                _processLock.Acquire();
                bool isDead = _dead;
                PoolTask *task = NULL;
                if(!_processQueue.empty() && (task = *_processQueue.begin()) != NULL)
                    _processQueue.erase(_processQueue.begin());
                _processLock.Release();
                if(task == NULL)
                {
                    if(isDead == true)
                        break;
#if PLATFORM == PLATFORM_WIN
                    WaitForSingleObject(inputEvent, 1000);
#else
                    timeval now;
                    timespec tv;
                    gettimeofday(&now, NULL);
                    tv.tv_sec = now.tv_sec;
                    tv.tv_nsec = (now.tv_usec * 1000) + 1000;

                    pthread_mutex_lock(&mutex);
                    pthread_cond_timedwait(&cond, &mutex, &tv);
                    pthread_mutex_unlock(&mutex);
#endif
                    continue;
                }

                if(task->call() == 0)
                    delete task;
                _processLock.Acquire();
                if((--taskCount) == 0); // Decrement task count after task finishes
                {
#if PLATFORM == PLATFORM_WIN
                    SetEvent(endEvent);
#else
                    pthread_cond_signal(&cond);
#endif
                }
                _processLock.Release();
            }

            _processLock.Acquire();
            uint32 threadCount = (--_threadCount);
            _processLock.Release();
            if(threadCount == 0)
                delete this;
        }

        void spawn();

        bool _dead;
        uint32 _poolId, _threadCount;

        Mutex _processLock;
        uint32 taskCount;
        std::vector<PoolTask*> _processQueue;
        bool _IsTasksEmpty() { _processLock.Acquire(); bool ret = (taskCount == 0); _processLock.Release(); return ret; }

#if PLATFORM == PLATFORM_WIN
        HANDLE inputEvent, endEvent;
#else
        pthread_cond_t cond;
        pthread_mutex_t mutex;
#endif
    };

    TaskPool *SpawnPool(uint32 thread_count);
    void CleanPool(uint32 poolId);

private:
    uint32 taskCounter;
    std::map<uint32, TaskPool*> m_taskPools;
};

extern SERVER_DECL ThreadManager sThreadManager;
