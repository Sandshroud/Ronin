/***
 * Demonstrike Core
 */

#pragma once

/** dummy lock to use a non-locked queue.
 */
class DummyLock
{
public:
    HEARTHSTONE_INLINE void Acquire() { }
    HEARTHSTONE_INLINE void Release() { }
};

/** linked-list style queue
 */
template<class T, class LOCK>
class FastQueue
{
    struct node
    {
        T element;
        node * next;
    };

    node * last;
    node * first;
    LOCK * m_lock;

public:

    FastQueue()
    {
        last = 0;
        first = 0;
        m_lock = new LOCK();
    }

    ~FastQueue()
    {
        Clear();
    }

    void Clear()
    {
        // clear any elements
        while(last != 0)
            Pop();
        if(m_lock != NULL)
        {
            delete m_lock;
            m_lock = NULL;
        }
    }

    void Push(T elem)
    {
        if(m_lock == NULL)
            return;

        m_lock->Acquire();
        node * n = new node;
        if(last)
            last->next = n;
        else
            first = n;

        last = n;
        n->next = 0;
        n->element = elem;
        m_lock->Release();
    }

    T Pop()
    {
        if(m_lock == NULL)
            return reinterpret_cast<T>(0);

        m_lock->Acquire();
        if(first == 0)
        {
            m_lock->Release();
            return reinterpret_cast<T>(0);
        }

        T ret = first->element;
        node * td = first;
        first = td->next;
        if(!first)
            last = 0;

        delete td;
        m_lock->Release();
        return ret;
    }

    T front()
    {
        if(m_lock == NULL)
            return reinterpret_cast<T>(0);

        m_lock->Acquire();
        if(first == 0)
        {
            m_lock->Release();
            return reinterpret_cast<T>(0);
        }

        T ret = first->element;
        m_lock->Release();
        return ret;
    }

    void pop_front()
    {
        if(m_lock == NULL)
            return;

        m_lock->Acquire();
        if(first == 0)
        {
            m_lock->Release();
            return;
        }

        node * td = first;
        first = td->next;
        if(!first)
            last = 0;

        delete td;
        m_lock->Release();
    }

    bool HasItems()
    {
        if(m_lock == NULL)
            return false;

        bool ret;
        m_lock->Acquire();
        ret = (first != 0);
        m_lock->Release();
        return ret;
    }
};
