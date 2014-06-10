/***
 * Demonstrike Core
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
