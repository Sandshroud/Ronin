/***
 * Demonstrike Core
 */

#include "Database.h"

createFileSingleton( DBEngine );

bool QThread::run()
{
    threadRunning = true;
    int32 curr = 0, start = 0, diff = 0;
    while(GetThreadState() != THREADSTATE_SELF_TERMINATE)
    {
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        start = GetTickCount();
        OrdinanceLock.Acquire();
        for(std::set<QueryThread*>::iterator itr = m_Ordinances.begin(); itr != m_Ordinances.end(); itr++)
            (*itr)->Update();
        OrdinanceLock.Release();
        curr = GetTickCount();
        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;
        diff = curr-start;
        if(diff <= 0)
            Delay(5);
        else if(diff < 5)
            Delay(5-(curr-start));

        while(!m_Ordinances.size() && SetThreadState(THREADSTATE_SLEEPING))
            Delay(10);
    }

    return true;
}

void QThread::AddQueryThread(QueryThread* m_Thread)
{
    OrdinanceLock.Acquire();
    m_Ordinances.insert(m_Thread);
    OrdinanceLock.Release();
}

bool DBThread::run()
{
    threadRunning = true;
    int32 curr = 0, start = 0, diff = 0;
    while(GetThreadState() != THREADSTATE_SELF_TERMINATE)
    {
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        start = GetTickCount();
        OrdinanceLock.Acquire();
        for(std::set<DirectDatabase*>::iterator itr = m_Ordinances.begin(); itr != m_Ordinances.end(); itr++)
            (*itr)->Update();
        OrdinanceLock.Release();
        curr = GetTickCount();
        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;
        diff = curr-start;
        if(diff <= 0)
            Delay(5);
        else if(diff < 5)
            Delay(5-(curr-start));

        while(!m_Ordinances.size() && SetThreadState(THREADSTATE_SLEEPING))
            Delay(10);
    }

    return true;
}

void DBThread::AddDatabase(DirectDatabase* m_DB)
{
    OrdinanceLock.Acquire();
    m_Ordinances.insert(m_DB);
    OrdinanceLock.Release();
}

DBEngine::DBEngine()
{
    ThreadCount = 0;
    m_MThreaded = false;
    m_QueryThread = NULL;
    m_DatabaseThread = NULL;
}

DBEngine::~DBEngine()
{

}

void DBEngine::Init(bool MultiThreaded)
{
    m_MThreaded = MultiThreaded;
    if(m_MThreaded)
        StartThreads();
    else
    {
        bLog.Notice("DBEngine", "Starting engine with parallel threads...");
        ThreadPool.ExecuteTask("QueryThread", m_QueryThread = new QThread());
        ThreadPool.ExecuteTask("Database Execute Thread", m_DatabaseThread = new DBThread());
    }
}

void DBEngine::StartThreads()
{
    uint32 number_of_cpus = 2;
#if PLATFORM == PLATFORM_WIN
    SYSTEM_INFO si;
    GetSystemInfo( &si );
    number_of_cpus = si.dwNumberOfProcessors/2;
#endif // WIN32

    bLog.Notice("DBEngine", "Starting engine with %u threads...", number_of_cpus*2);
    m_QueryThreads = new DBThreadHolder< QThread >*[number_of_cpus];
    m_DatabaseThreads = new DBThreadHolder< DBThread >*[number_of_cpus];

    for(ThreadCount = 0; ThreadCount < number_of_cpus; ThreadCount++)
    {
        m_QueryThreads[ThreadCount] = (new DBThreadHolder< QThread >(new QThread()));
        m_DatabaseThreads[ThreadCount] = (new DBThreadHolder< DBThread >(new DBThread()));
        char qThreadName[255], eThreadName[255];
        sprintf(qThreadName, "QueryThread %u", ThreadCount);
        ThreadPool.ExecuteTask(qThreadName, m_QueryThreads[ThreadCount]->m_Thread);
        sprintf(eThreadName, "Database Execute Thread %u", ThreadCount);
        ThreadPool.ExecuteTask(eThreadName, m_DatabaseThreads[ThreadCount]->m_Thread);
    }
}

void DBEngine::AddDatabase(DirectDatabase* m_Database)
{
    if(m_MThreaded)
    {
        uint32 threadId = 0;
        uint32 minStress = 100;
        for(uint32 i = 0; i < ThreadCount; i++)
        {
            if(m_QueryThreads[i]->StressCounter < minStress)
            {
                minStress = m_DatabaseThreads[i]->StressCounter;
                threadId = i;
            }
        }

        m_DatabaseThreads[threadId]->StressCounter++;
        m_DatabaseThreads[threadId]->m_Thread->AddDatabase(m_Database);
    }
    else
        m_DatabaseThread->AddDatabase(m_Database);
}

void DBEngine::AddQueryThread(QueryThread* m_QThread)
{
    if(m_MThreaded)
    {
        uint32 threadId = 0;
        uint32 minStress = 100;
        for(uint32 i = 0; i < ThreadCount; i++)
        {
            if(m_QueryThreads[i]->StressCounter < minStress)
            {
                minStress = m_QueryThreads[i]->StressCounter;
                threadId = i;
            }
        }

        m_QueryThreads[threadId]->StressCounter++;
        m_QueryThreads[threadId]->m_Thread->AddQueryThread(m_QThread);
    }
    else
        m_QueryThread->AddQueryThread(m_QThread);
}

void DBEngine::EndThreads()
{
    bLog.Notice("DBEngine", "Closing ordinances...");
    if(m_MThreaded)
    {
        for(uint32 i = 0; i < ThreadCount; i++)
        {
            m_QueryThreads[i]->m_Thread->ClearOrdinances();
            m_DatabaseThreads[i]->m_Thread->ClearOrdinances();

            m_QueryThreads[i]->m_Thread->SelfTerminate();
            m_DatabaseThreads[i]->m_Thread->SelfTerminate();
        }
    }
    else
    {
        m_QueryThread->ClearOrdinances();
        m_DatabaseThread->ClearOrdinances();

        m_QueryThread->SelfTerminate();
        m_DatabaseThread->SelfTerminate();
    }
}
