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

#pragma once

class QThread : public ThreadContext
{
private:
    Mutex OrdinanceLock;
    std::set<QueryThread*> m_Ordinances;
public:
    QThread() {};
    ~QThread() {};

    void AddQueryThread(QueryThread* m_Thread);
    bool threadRunning;
    bool run();

    void OnShutdown()
    {
#if PLATFORM == PLATFORM_WIN
        SetEvent(hEvent);
#else
        pthread_cond_signal(&cond);
#endif
    }

    void ClearOrdinances()
    {
        OrdinanceLock.Acquire();
        for(std::set<QueryThread*>::iterator itr = m_Ordinances.begin(), itr2; itr != m_Ordinances.end();)
        {
            itr2 = itr++;
            delete (*itr2);
        }
        m_Ordinances.clear();
        OrdinanceLock.Release();
    };
};

class DBThread : public ThreadContext
{
private:
    Mutex OrdinanceLock;
    std::set<DirectDatabase*> m_Ordinances;
public:
    DBThread() {};
    ~DBThread() {};

    void AddDatabase(DirectDatabase* m_DB);
    bool threadRunning;
    bool run();

    void OnShutdown()
    {
#if PLATFORM == PLATFORM_WIN
        SetEvent(hEvent);
#else
        pthread_cond_signal(&cond);
#endif
    }

    void ClearOrdinances()
    {
        OrdinanceLock.Acquire();
        for(std::set<DirectDatabase*>::iterator itr = m_Ordinances.begin(); itr != m_Ordinances.end(); itr++)
            (*itr)->EndThreads();
        m_Ordinances.clear();
        OrdinanceLock.Release();
    };
};

template < class Class > struct DBThreadHolder
{
    DBThreadHolder<Class>(Class* m) { m_Thread = m; };

    uint32 StressCounter;
    Class* m_Thread;
};

class SERVER_DECL DBEngine : public Singleton<DBEngine>
{
private:
    bool m_MThreaded;

    DBThread* m_DatabaseThread;
    QThread* m_QueryThread;

    uint32 ThreadCount;
    DBThreadHolder< QThread >** m_QueryThreads;
    DBThreadHolder< DBThread >** m_DatabaseThreads;

    // Not to be called from outside.
    void StartThreads();

public:
    DBEngine();
    ~DBEngine();

    void Init(bool MultiThreaded);

    void AddDatabase(DirectDatabase* m_Database);
    void AddQueryThread(QueryThread* m_QThread);

    void EndThreads();
};

#define sDBEngine DBEngine::getSingleton()
