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
    size_t len;

public:

    FastQueue()
    {
        last = 0;
        first = 0;
        len = 0;
        m_lock = new LOCK();
    }

    ~FastQueue()
    {
        Clear();

        if(m_lock != NULL)
            delete m_lock;
        m_lock = NULL;
    }

    void Clear()
    {
        // clear any elements
        Guard guard(*m_lock);
        while(last != 0)
            Pop();
        len = 0;
    }

    void Push(T elem)
    {
        Guard guard(*m_lock);
        node * n = new node;
        if(last)
            last->next = n;
        else
            first = n;

        last = n;
        n->next = 0;
        n->element = elem;
        ++len;
    }

    T Pop()
    {
        Guard guard(*m_lock);
        if(first == 0)
            return nullptr;

        T ret = first->element;
        node * td = first;
        first = td->next;
        if(!first)
            last = 0;

        delete td;
        return ret;
    }

    T front()
    {
        Guard guard(*m_lock);
        if(first == 0)
            return nullptr;

        T ret = first->element;
        return ret;
    }

    T at(size_t pos)
    {
        Guard guard(*m_lock);
        node * td = first;
        while(pos > 0)
        {
            if(td->next == NULL)
                return nullptr;
            td = td->next;
            --pos;
        }

        return td->element;
    }

    size_t size() { return len; }

    void pop_front()
    {
        Guard guard(*m_lock);
        if(first == 0)
            return;

        node * td = first;
        first = td->next;
        if(!first)
            last = 0;
        delete td;
        --len;
    }

    bool HasItems()
    {
        Guard guard(*m_lock);
        return (first != 0);
    }
};
