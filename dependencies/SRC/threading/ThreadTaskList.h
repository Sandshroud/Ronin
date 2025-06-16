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

class ThreadTask;

class ThreadTaskList
{
    friend ThreadTaskListExecutor;

public:
    ThreadTaskList(uint32 spawnCountMax);
    ~ThreadTaskList();

    ThreadTask * GetTask();
    void AddTask(CallbackBase * cb);
    void RemoveTask(ThreadTask * task)
    {
        queueLock.Acquire();
        tasks.erase(task);
        queueLock.Release();
    }

    // Returns spawned thread count
    uint32 spawn(char *source, char *startupMessage, bool threaded);
    void kill();

    void wait(time_t &input_systime, tm &input_localTime);
    void waitForThreadsToExit();
    void incrementThreadCount()
    {
        tcMutex.Acquire();
        ++thread_count;
        tcMutex.Release();
    }

    void decrementThreadCount()
    {
        tcMutex.Acquire();
        --thread_count;
        tcMutex.Release();
    }

    static std::string format_name(const char* fmt,...);

protected:
    std::set<ThreadTask*> tasks;
    Mutex queueLock, tcMutex;

    uint32 max_thread_count;
    uint32 thread_count;
    bool running;
};
