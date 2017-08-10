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

#if PLATFORM == PLATFORM_WIN && _MSC_VER > 1700
#include <mutex>
#include <shared_mutex>

// A standard mutex wrapper class
class SERVER_DECL StandardMutex : protected std::recursive_mutex
{
public:
    StandardMutex() : std::recursive_mutex() {}
    ~StandardMutex() {}

    RONIN_INLINE void Acquire() { lock(); }
    RONIN_INLINE void Release() { unlock(); }
    RONIN_INLINE bool AttemptAcquire() { return try_lock(); }
};

class SERVER_DECL PriorityMutex : protected std::shared_mutex
{
public:
    PriorityMutex() : std::shared_mutex(), count(0), owner() {}
    ~PriorityMutex() {}

    // Shared access here

    // Acquire sharedlock
    RONIN_INLINE void LowAcquire()
    {
        if(owner == std::this_thread::get_id())
            return;

        lock_shared();
    }

    // Release shared lock
    RONIN_INLINE void LowRelease()
    {
        if(owner == std::this_thread::get_id())
            return;

        unlock_shared();
    }

    // Note: shared mutex is not recursive, so we set our own recursive system here

    // Acquire full lock
    RONIN_INLINE void HighAcquire()
    {
        std::thread::id this_id = std::this_thread::get_id();
        if(owner == this_id)
            ++count;
        else
        {
            lock();
            owner = this_id;
            ++count;
        }
    }

    // Release full lock
    RONIN_INLINE void HighRelease()
    {
        if(count > 1)
            --count;
        else
        {
            owner = std::thread::id();
            --count;
            unlock();
        }
    }

private:
    std::atomic<std::thread::id> owner;
    int count;
};

#define Mutex StandardMutex
#define RWMutex PriorityMutex

#else

#if PLATFORM == PLATFORM_WIN
#include <Windows.h>
#elif defined(__DragonFly__)
#include <pthread.h>
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
    RONIN_INLINE void Acquire()
    {
#if PLATFORM != PLATFORM_WIN
        pthread_mutex_lock(&mutex);
#else
        EnterCriticalSection(&cs);
#endif
    }

    /** Releases this mutex. No error checking performed
     */
    RONIN_INLINE void Release()
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
    RONIN_INLINE bool AttemptAcquire()
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

class SERVER_DECL PriorityMutex
{
public:
    PriorityMutex();
    ~PriorityMutex();

    // Enter atomic state and wait for acquired count to equal zero
    RONIN_INLINE void HighAcquire()
    {
#if PLATFORM != PLATFORM_WIN
        pthread_mutex_lock(&mutex);
#else
        EnterCriticalSection(&csHigh);
#endif
        while(acquiredCount);
    }

    // Release atomic state
    RONIN_INLINE void HighRelease()
    {
#if PLATFORM != PLATFORM_WIN
        pthread_mutex_unlock(&mutex);
#else
        LeaveCriticalSection(&csHigh);
#endif
    }

    // Enter critical state to set our atomic counter higher, do not use highacquire when entering atomic state
    RONIN_INLINE void LowAcquire()
    {
#if PLATFORM != PLATFORM_WIN
        pthread_mutex_lock(&mutex);
#else
        EnterCriticalSection(&csHigh);
#endif
        ++acquiredCount;
#if PLATFORM != PLATFORM_WIN
        pthread_mutex_unlock(&mutex);
#else
        LeaveCriticalSection(&csHigh);
#endif
    }

    // Note, acquireCount is atomic so we don't need to lock to decrement it
    RONIN_INLINE void LowRelease() { --acquiredCount; }

private:
    std::atomic<int> acquiredCount;

#if PLATFORM == PLATFORM_WIN
    /** Critical section used for system calls
     */
    CRITICAL_SECTION csHigh;

#else
    /** Static mutex attribute
     */
    static bool prio_attr_initalized;
    static pthread_mutexattr_t prio_attr;

    /** pthread struct used in system calls
     */
    pthread_mutex_t mutex;
#endif
};

#define Mutex EasyMutex
#define RWMutex PriorityMutex

#endif