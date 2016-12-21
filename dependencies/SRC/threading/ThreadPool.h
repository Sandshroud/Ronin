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
    RONIN_INLINE uint32 GetActiveThreadCount() { return (uint32)m_activeThreads.size(); }

    // Creates exception which sets the thread name, do not call inside try block
    static void SetThreadName(const char* format);

    static void Suicide();
};

extern SERVER_DECL CThreadPool ThreadPool;
