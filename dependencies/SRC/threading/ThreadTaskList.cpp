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

ThreadTaskList::ThreadTaskList(uint32 spawnCountMax) : max_thread_count(spawnCountMax), thread_count(0), running(true)
{

}

ThreadTaskList::~ThreadTaskList()
{

}

void ThreadTaskList::AddTask(CallbackBase * cb)
{
    queueLock.Acquire();
    tasks.insert(new ThreadTask(cb));
    queueLock.Release();
}

ThreadTask *ThreadTaskList::GetTask()
{
    queueLock.Acquire();

    ThreadTask* t = 0;
    for(std::set<ThreadTask*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
    {
        if(!(*itr)->in_progress)
        {
            t = (*itr);
            t->in_progress = true;
            break;
        }
    }
    queueLock.Release();
    return t;
}

uint32 ThreadTaskList::spawn(char *source, char *startupMessage, bool threaded)
{
    uint32 threadcount = 1;
    if(threaded)
    {
        // get processor count
#ifndef WIN32
#if UNIX_FLAVOUR == UNIX_FLAVOUR_LINUX
#ifdef X64
        threadcount = 2;
#else
        long affmask;
        sched_getaffinity(0, 4, (cpu_set_t*)&affmask);
        threadcount = (BitCount8(affmask)) * 2;
#endif
#else
        threadcount = 2;
#endif
#else
        SYSTEM_INFO s;
        GetSystemInfo(&s);
        threadcount = s.dwNumberOfProcessors * 2;
#endif
    }

    if(threadcount > max_thread_count)
        threadcount = max_thread_count;
    else if(threadcount <= 1)
        threadcount = 1;

    sLog.Notice(source, "%s with %u thread(s).", startupMessage, threadcount);
    for(uint32 x = 0; x < threadcount; ++x)
        sThreadManager.ExecuteTask(format_name("TaskExecutor|%u", x).c_str(), new ThreadTaskListExecutor(this));

    return threadcount;
}

void ThreadTaskList::wait(time_t &input_systime, tm &input_localTime)
{
    bool has_tasks = true;
    time_t t;
    while(has_tasks)
    {
        queueLock.Acquire();
        has_tasks = false;
        for(std::set<ThreadTask*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
        {
            if(!(*itr)->completed)
            {
                has_tasks = true;
                break;
            }
        }
        queueLock.Release();

        // keep updating time lol
        t = time(NULL);
        if( input_systime != t )
        {
            input_systime = t;
            input_localTime = *localtime(&t);
        }

        Sleep(20);
    }
}

void ThreadTaskList::kill()
{
    running = false;
}

void ThreadTaskList::waitForThreadsToExit()
{
    while(thread_count)
        Sleep(20);
}

std::string ThreadTaskList::format_name(const char* fmt,...)
{
    bool append = false;
    va_list argList;
    va_start(argList,fmt);
    std::string result;
    // We draw the line at a 1MB string.
    const int maxSize = 1000000;

    // If the string is less than 161 characters,
    // allocate it on the stack because this saves
    // the malloc/free time.
    const int bufSize = 161;
    char stackBuffer[bufSize];

    // MSVC does not support va_copy
    int actualSize = _vscprintf(fmt, argList) + 1;
    if (actualSize > bufSize)
    {
        int fullSize = actualSize >= maxSize ? maxSize + 1 : actualSize;
        if(fullSize)
        {
            // Now use the heap.
            if(char* heapBuffer = (char*)malloc(fullSize))
            {
                if(_vsnprintf(heapBuffer, fullSize, fmt, argList))
                    result.append(heapBuffer);
                free(heapBuffer);
                append = true;
            }
        }
    }

    if(append == false)
    {
        vsprintf(stackBuffer, fmt, argList);
        result.append(stackBuffer);
    }
    va_end(argList);

    return result;
}
