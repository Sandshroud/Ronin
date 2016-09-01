
#include "StdAfx.h"

EventHandler::EventHandler(Object *obj) : _object(obj)
{

}

EventHandler::~EventHandler()
{
    while(!m_eventQueue.empty())
    {
        EventBaseClass *baseClass = *m_eventQueue.begin();
        m_eventQueue.erase(m_eventQueue.begin());
        delete baseClass;
    }
    while(!m_dynamicEvents.empty())
    {
        EventBaseClass *baseClass = m_dynamicEvents.begin()->second;
        m_dynamicEvents.erase(m_dynamicEvents.begin());
        delete baseClass;
    }
    while(!m_staticEvents.empty())
    {
        EventBaseClass *baseClass = *m_staticEvents.begin();
        m_staticEvents.erase(m_staticEvents.begin());
        delete baseClass;
    }
}

void EventHandler::Init()
{

}

bool EventHandler::_HasEvent(size_t hash)
{
    m_lock.Acquire();
    bool ret = m_dynamicEvents.find(hash) != m_dynamicEvents.end();
    m_lock.Release();
    return ret;
}

void EventHandler::_RemoveEvent(size_t hash)
{
    Loki::AssocVector<size_t, EventBaseClass*>::iterator itr;
    m_lock.Acquire();
    if((itr = m_dynamicEvents.find(hash)) != m_dynamicEvents.end())
    {
        EventBaseClass *bClass = itr->second;
        m_dynamicEvents.erase(itr);
        delete bClass;
    }
    m_lock.Release();
}

void EventHandler::_AddEvent(size_t hash, CallbackBase *eventCallback, time_t occurTimer)
{
    m_lock.Acquire();
    m_eventQueue.push_back(new TimedEvent(eventCallback, hash, occurTimer));
    m_lock.Release();
}

void EventHandler::_AddEvent(size_t hash, CallbackBase *eventCallback, uint32 occurTimer)
{
    m_lock.Acquire();
    m_eventQueue.push_back(new DelayedEvent(eventCallback, hash, occurTimer));
    m_lock.Release();
}

bool EventHandler::_HasStaticEvent(size_t hash)
{
    m_lock.Acquire();
    for(std::vector<EventBaseClass*>::iterator itr = m_staticEvents.begin(); itr != m_staticEvents.end(); itr++)
    {
        if(hash == (*itr)->getFuncHash())
        {
            m_lock.Release();
            return true;
        }
    }
    m_lock.Release();
    return false;
}

void EventHandler::_RemoveStaticEvent(size_t hash)
{
    Loki::AssocVector<size_t, EventBaseClass*>::iterator itr;
    m_lock.Acquire();
    for(std::vector<EventBaseClass*>::iterator itr2, itr = m_staticEvents.begin(); itr != m_staticEvents.end();)
    {
        itr2 = itr;
        itr++;
        if(hash == (*itr2)->getFuncHash())
        {
            EventBaseClass *bClass = (*itr2);
            m_staticEvents.erase(itr2);
            delete bClass;
        }
    }
    m_lock.Release();
}

void EventHandler::_AddStaticEvent(size_t hash, CallbackBase *eventCallback, uint32 occurTimer)
{
    m_lock.Acquire();
    for(std::vector<EventBaseClass*>::iterator itr = m_staticEvents.begin(); itr != m_staticEvents.end(); itr++)
    {
        if(hash == (*itr)->getFuncHash())
        {
            delete eventCallback;
            m_lock.Release();
            return;
        }
    }

    m_staticEvents.push_back(new DelayedEvent(eventCallback, hash, occurTimer));
    m_lock.Release();
}

void EventHandler::Update(uint32 msDiff, time_t curTime)
{
    m_lock.Acquire();
    if(!m_eventQueue.empty())
    {
        while(!m_eventQueue.empty())
        {
            EventBaseClass *baseClass = *m_eventQueue.begin();
            m_eventQueue.erase(m_eventQueue.begin());
            size_t functionHash = baseClass->getFuncHash();
            Loki::AssocVector<size_t, EventBaseClass*>::iterator itr;
            if((itr = m_dynamicEvents.find(functionHash)) != m_dynamicEvents.end())
            {
                delete itr->second;
                m_dynamicEvents.erase(itr);
            }
            m_dynamicEvents.insert(std::make_pair(functionHash, baseClass));
        }
    }

    if(!m_dynamicEvents.empty())
    {
        Loki::AssocVector<size_t, EventBaseClass*>::iterator itr;
        for(itr = m_dynamicEvents.begin(); itr != m_dynamicEvents.end(); itr++)
            if(itr->second->Update(msDiff, curTime))
                m_runningEvents.push_back(itr->first);

        if(!m_runningEvents.empty())
        {
            while(!m_runningEvents.empty())
            {
                size_t eventId = *m_runningEvents.begin();
                m_runningEvents.erase(m_runningEvents.begin());
                if((itr = m_dynamicEvents.find(eventId)) != m_dynamicEvents.end())
                {
                    EventBaseClass *baseClass = itr->second;
                    m_dynamicEvents.erase(itr);
                    baseClass->Activate();
                    delete baseClass;
                }
            }
        }
    }

    if(!m_staticEvents.empty())
    {
        for(std::vector<EventBaseClass*>::iterator itr = m_staticEvents.begin(); itr != m_staticEvents.end(); itr++)
        {
            if((*itr)->Update(msDiff, curTime))
            {
                (*itr)->Activate();
                (*itr)->Reset();
            }
        }
    }
    m_lock.Release();
}
