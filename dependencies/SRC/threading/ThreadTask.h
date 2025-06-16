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

class ThreadTaskList;
class CallbackBase;

enum ThreadTaskExecutorPriorities
{
    TTE_PRIORITY_LOW        = 0,
    TTE_PRIORITY_MED        = 1,
    TTE_PRIORITY_HIGH       = 2
};

class ThreadTaskExecutor : public ThreadContext
{
public:
    ThreadTaskExecutor(CallbackBase * Callback, uint32 Priority);
    ~ThreadTaskExecutor();
    bool run();

private:
    CallbackBase *cb;
    uint32 priority;
};

class ThreadTaskListExecutor : public ThreadContext
{
    ThreadTaskList * starter;
public:
    ThreadTaskListExecutor(ThreadTaskList * l);
    ~ThreadTaskListExecutor();

    bool run();
};

class ThreadTask
{
    friend ThreadTaskList;
    friend ThreadTaskListExecutor;

protected:
    ThreadTask(CallbackBase * cb);
    ~ThreadTask();

    void execute();

private:
    CallbackBase *_cb;
    bool completed, in_progress;
};
