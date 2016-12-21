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

/** dummy lock to use a non-locked queue.
 */
class DummyLock
{
public:
    RONIN_INLINE void Acquire() { }
    RONIN_INLINE void Release() { }
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
