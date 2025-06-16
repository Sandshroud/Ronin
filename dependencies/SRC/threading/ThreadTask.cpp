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

ThreadTaskExecutor::ThreadTaskExecutor(CallbackBase * Callback, uint32 Priority) : cb(Callback), priority(Priority)
{

}

ThreadTaskExecutor::~ThreadTaskExecutor()
{
    delete cb;
}

bool ThreadTaskExecutor::run()
{
    /* Set thread priority, this is a bitch for multiplatform :P */
#ifdef WIN32
    switch(priority)
    {
    case TTE_PRIORITY_LOW:
        ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_LOWEST );
        break;

    case TTE_PRIORITY_HIGH:
        ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
        break;

    default: // TTE_PRIORITY_MED
        ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_NORMAL );
        break;
    }
#else
    struct sched_param param;
    switch(priority)
    {
    case BTE_PRIORITY_LOW:
        param.sched_priority = 0;
        break;

    case BTW_PRIORITY_HIGH:
        param.sched_priority = 10;
        break;

    default:        // BTW_PRIORITY_MED
        param.sched_priority = 5;
        break;
    }
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif

    // Execute the task in our new context.
    cb->execute();
#ifdef WIN32
    ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#else
    param.sched_priority = 5;
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif

    return true;
}

ThreadTaskListExecutor::ThreadTaskListExecutor(ThreadTaskList * l) : ThreadContext(), starter(l)
{
    l->incrementThreadCount();
}

ThreadTaskListExecutor::~ThreadTaskListExecutor()
{
    starter->decrementThreadCount();
}

bool ThreadTaskListExecutor::run()
{
    ThreadTask * t;
    while(starter->running)
    {
        if(t = starter->GetTask())
        {
            t->execute();
            t->completed = true;
            starter->RemoveTask(t);
            delete t;
        } else Delay(20);
    }
    return true;
}

ThreadTask::ThreadTask(CallbackBase * cb) : _cb(cb), completed(false), in_progress(false)
{

}

ThreadTask::~ThreadTask()
{
    delete _cb;
}

void ThreadTask::execute()
{
    _cb->execute();
}
