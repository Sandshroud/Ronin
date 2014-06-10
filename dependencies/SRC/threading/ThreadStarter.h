/***
 * Demonstrike Core
 */

#pragma once

enum CThreadState
{
    THREADSTATE_TERMINATE = 0,
    THREADSTATE_PAUSED = 1,
    THREADSTATE_SLEEPING = 2,
    THREADSTATE_BUSY = 3,
    THREADSTATE_AWAITING = 4,
    THREADSTATE_SELF_TERMINATE = 5,
};

class SERVER_DECL ThreadContext
{
public:
    ThreadContext()
    {
#if PLATFORM == PLATFORM_WIN
        hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
#endif

        ThreadState = THREADSTATE_AWAITING;
        start_time  = 0;
    }

    virtual ~ThreadContext()
    {
#if PLATFORM == PLATFORM_WIN
        CloseHandle(hEvent);
#else
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
#endif
    }

    HEARTHSTONE_INLINE bool SetThreadState(CThreadState thread_state)
    {
        if(ThreadState == thread_state)
            return true;

        if(ThreadState == THREADSTATE_TERMINATE)
            if(thread_state != THREADSTATE_SELF_TERMINATE)
                return false;
        if(ThreadState == THREADSTATE_SELF_TERMINATE)
            if(thread_state != THREADSTATE_TERMINATE)
                return false;

        ThreadState = thread_state;
        return true;
    }

    HEARTHSTONE_INLINE CThreadState GetThreadState() {  return ThreadState; }

    int GetThreadId() { return ThreadId; }
    time_t GetStartTime() { return start_time; }

    virtual bool run() { return false; };
    virtual void OnShutdown()
    {
        SetThreadState(THREADSTATE_TERMINATE);
#if PLATFORM == PLATFORM_WIN
        SetEvent(hEvent);
#else
        pthread_cond_signal(&cond);
#endif
    }

    void SelfTerminate()
    {
        SetThreadState(THREADSTATE_SELF_TERMINATE);
#if PLATFORM == PLATFORM_WIN
        SetEvent(hEvent);
#else
        pthread_cond_signal(&cond);
#endif
    }

    void Delay(uint32 timems)
    {
        if(timems == 0)
            return;

#if PLATFORM == PLATFORM_WIN
        WaitForSingleObject(hEvent, timems);
#else
        unsigned long times = timems / 1000;
        timems = timems - times * 1000;

        timeval now;
        timespec tv;

        gettimeofday(&now, NULL);

        tv.tv_sec = now.tv_sec;
        tv.tv_nsec = now.tv_usec * 1000;
        tv.tv_sec += times;
        tv.tv_nsec += (timems * 1000 * 1000);

        pthread_mutex_lock(&mutex);
        pthread_cond_timedwait(&cond, &mutex, &tv);
        pthread_mutex_unlock(&mutex);
#endif
    }

protected:
    CThreadState ThreadState;
    time_t start_time;
    int ThreadId;

#if PLATFORM == PLATFORM_WIN
    HANDLE hEvent;
#else
    pthread_cond_t cond;
    pthread_mutex_t mutex;
#endif
};

