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
