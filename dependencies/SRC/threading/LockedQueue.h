/***
 * Demonstrike Core
 */

#pragma once

#include "Mutex.h"
#include <deque>

template<class TYPE>
class LockedQueue
{
public:
    ~LockedQueue()
    {

    }

    RONIN_INLINE void add(const TYPE& element)
    {
        mutex.Acquire();
        queue.push_back(element);
        mutex.Release();
    }

    RONIN_INLINE TYPE next()
    {
        mutex.Acquire();
        assert(queue.size() > 0);
        TYPE t = queue.front();
        queue.pop_front();
        mutex.Release();
        return t;
    }

    RONIN_INLINE size_t size()
    {
        mutex.Acquire();
        size_t c = queue.size();
        mutex.Release();
        return c;
    }

    RONIN_INLINE bool empty()
    {   // return true only if sequence is empty
        mutex.Acquire();
        bool isEmpty = queue.empty();
        mutex.Release();
        return isEmpty;
    }

    RONIN_INLINE TYPE get_first_element()
    {
        mutex.Acquire();
        TYPE t; 
        if(queue.size() == 0)
            t = reinterpret_cast<TYPE>(0);
        else
            t = queue.front();
        mutex.Release();
        return t;           
    }

    RONIN_INLINE void pop()
    {
        mutex.Acquire();
        ASSERT(queue.size() > 0);
        queue.pop_front();
        mutex.Release();
    }

    RONIN_INLINE void clear()
    {
        mutex.Acquire();
        queue.clear();
        mutex.Release();
    }

protected:
    std::deque<TYPE> queue;
    Mutex mutex;
};
