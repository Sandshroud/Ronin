/***
 * Demonstrike Core
 */

#pragma once

#ifdef __DragonFly__
#include <pthread.h>
#endif
#ifdef _WIN32_WINNT
#include <Windows.h>
#endif

class SERVER_DECL EasyMutex
{
public:
    friend class Condition;

    /** Initializes a mutex class, with InitializeCriticalSection / pthread_mutex_init
     */
    EasyMutex();

    /** Deletes the associated critical section / mutex
     */
    ~EasyMutex();

    /** Acquires this mutex. If it cannot be acquired immediately, it will block.
     */
    HEARTHSTONE_INLINE void Acquire()
    {
#if PLATFORM != PLATFORM_WIN
        pthread_mutex_lock(&mutex);
#else
        EnterCriticalSection(&cs);
#endif
    }

    /** Releases this mutex. No error checking performed
     */
    HEARTHSTONE_INLINE void Release()
    {
#if PLATFORM != PLATFORM_WIN
        pthread_mutex_unlock(&mutex);
#else
        LeaveCriticalSection(&cs);
#endif
    }

    /** Attempts to acquire this mutex. If it cannot be acquired (held by another thread)
     * it will return false.
     * @return false if cannot be acquired, true if it was acquired.
     */
    HEARTHSTONE_INLINE bool AttemptAcquire()
    {
#if PLATFORM != PLATFORM_WIN
        return (pthread_mutex_trylock(&mutex) == 0);
#else
        return (TryEnterCriticalSection(&cs) == TRUE ? true : false);
#endif
    }

protected:
#if PLATFORM == PLATFORM_WIN
    /** Critical section used for system calls
     */
    CRITICAL_SECTION cs;

#else
    /** Static mutex attribute
     */
    static bool attr_initalized;
    static pthread_mutexattr_t attr;

    /** pthread struct used in system calls
     */
    pthread_mutex_t mutex;
#endif
};

class SERVER_DECL SmartMutex
{
public:
    friend class Condition;

    /** Initializes a mutex class, with InitializeCriticalSection / pthread_mutex_init
     */
    SmartMutex();

    /** Deletes the associated critical section / mutex
     */
    ~SmartMutex();

    /** Acquires this mutex. If it cannot be acquired immediately, it will block.
     */
    HEARTHSTONE_INLINE void Acquire(bool RestrictUnlocking = false)
    {
        if(hearthstone_GetThreadId() == m_activeThread)
        {
            m_ThreadCalls++;
            return;
        }

#if PLATFORM != PLATFORM_WIN
        pthread_mutex_lock(&mutex);
#else
        EnterCriticalSection(&cs);
#endif
        m_ThreadCalls++;
        m_activeThread = hearthstone_GetThreadId();
        LockReleaseToThread = RestrictUnlocking;
    }

    /** Releases this mutex. No error checking performed
     */
    HEARTHSTONE_INLINE void Release()
    {
        // If we're locked to this specific thread, and we're unlocked from another thread, error
        if(LockReleaseToThread && hearthstone_GetThreadId() != m_activeThread)
            ASSERT(false && "MUTEX RELEASE CALLED FROM NONACTIVE THREAD!");

        m_ThreadCalls--;
        if(m_ThreadCalls == 0)
        {
            m_activeThread = 0;
#if PLATFORM != PLATFORM_WIN
            pthread_mutex_unlock(&mutex);
#else
            LeaveCriticalSection(&cs);
#endif
        }
    }

    /** Attempts to acquire this mutex. If it cannot be acquired (held by another thread)
     * it will return false.
     * @return false if cannot be acquired, true if it was acquired.
     */
    HEARTHSTONE_INLINE bool AttemptAcquire(bool RestrictUnlocking = false)
    {
        if(hearthstone_GetThreadId() == m_activeThread)
        {
            m_ThreadCalls++;
            return true;
        }

#if PLATFORM != PLATFORM_WIN
        if(pthread_mutex_trylock(&mutex) == 0)
#else
        if(TryEnterCriticalSection(&cs) == TRUE)
#endif
        {
            m_ThreadCalls++;
            m_activeThread = hearthstone_GetThreadId();
            LockReleaseToThread = RestrictUnlocking;
            return true;
        }
        return false;
    }

protected:
    bool LockReleaseToThread;
    uint32 m_activeThread;
    uint32 m_ThreadCalls;

#if PLATFORM == PLATFORM_WIN
    /** Critical section used for system calls
     */
    CRITICAL_SECTION cs;

#else
    /** Static mutex attribute
     */
    static bool attr_initalized;
    static pthread_mutexattr_t attr;

    /** pthread struct used in system calls
     */
    pthread_mutex_t mutex;
#endif
};

#define Mutex EasyMutex
