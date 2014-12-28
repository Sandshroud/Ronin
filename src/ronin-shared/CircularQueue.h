/***
 * Demonstrike Core
 */

#pragma once

template<class T, int ELEMENTCOUNT>
class CircularQueue
{
    T m_elements[ELEMENTCOUNT];
    int m_pos;
public:
    CircularQueue() : m_pos(0)
    {
        memset(m_elements, 0, sizeof(T) * ELEMENTCOUNT);
    }
    ~CircularQueue() {}

    void push(T& val)
    {
        m_elements[++m_pos] = val;
        if(m_pos >= ELEMENTCOUNT)
            m_pos = 0;
    }

    inline T * get()
    {
        return m_elements;
    }

    /* assumes we're int-based */
    void print()
    {
        printf("Elements of CircularQueue[%u]: ", ELEMENTCOUNT);
        for(int i = 0; i < ELEMENTCOUNT; ++i)
            printf("%u ", (int)m_elements[i]);
        printf("\n");
    }
};
