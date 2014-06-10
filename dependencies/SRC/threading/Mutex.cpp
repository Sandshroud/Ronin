/***
 * Demonstrike Core
 */

#include "Threading.h"

#ifdef __DragonFly__                                                            
#include <pthread.h>                                                            
#endif         
#if PLATFORM == PLATFORM_WIN

/* Windows Critical Section Implementation */
EasyMutex::EasyMutex() { InitializeCriticalSection(&cs); }
EasyMutex::~EasyMutex() { DeleteCriticalSection(&cs); }

SmartMutex::SmartMutex() { InitializeCriticalSection(&cs); m_activeThread = m_ThreadCalls = 0; }
SmartMutex::~SmartMutex() { DeleteCriticalSection(&cs); }

#else

/* this is done slightly differently on bsd-variants */
#if defined(__FreeBSD__) ||  defined(__APPLE_CC__) || defined(__OpenBSD__) || defined(__DragonFly__)
#define recursive_mutex_flag PTHREAD_MUTEX_RECURSIVE
#else
#define recursive_mutex_flag PTHREAD_MUTEX_RECURSIVE_NP
#endif

/* Linux mutex implementation */
bool EasyMutex::attr_initalized = false;
pthread_mutexattr_t EasyMutex::attr;

EasyMutex::EasyMutex()
{
    if(!attr_initalized)
    {
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, recursive_mutex_flag);
        attr_initalized = true;
    }

    pthread_mutex_init(&mutex, &attr);
}

EasyMutex::~EasyMutex() { pthread_mutex_destroy(&mutex); }

// Smart locking mutex
bool SmartMutex::attr_initalized = false;
pthread_mutexattr_t SmartMutex::attr;

SmartMutex::SmartMutex()
{
    m_activeThread = m_ThreadCalls = 0;
    if(!attr_initalized)
    {
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, recursive_mutex_flag);
        attr_initalized = true;
    }

    pthread_mutex_init(&mutex, &attr);
}

SmartMutex::~SmartMutex() { pthread_mutex_destroy(&mutex); }

#endif
