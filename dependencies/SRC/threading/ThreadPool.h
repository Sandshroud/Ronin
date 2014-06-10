/***
 * Demonstrike Core
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


class SERVER_DECL CThreadPool
{
    Mutex _mutex;

    typedef std::set<Thread*> ThreadSet;
    ThreadSet m_activeThreads;

public:
    CThreadPool();

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
    HEARTHSTONE_INLINE uint32 GetActiveThreadCount() { return (uint32)m_activeThreads.size(); }

    // Creates exception which sets the thread name, do not call inside try block
    static void SetThreadName(const char* format);

    static void Suicide();
};

extern SERVER_DECL CThreadPool ThreadPool;
