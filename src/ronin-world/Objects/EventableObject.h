/***
 * Demonstrike Core
 */

#pragma once

class EventableObjectHolder;

/**
  * @class EventableObject
  * EventableObject means that the class inheriting this is able to take
  * events. This 'base' class will store and update these events upon
  * receiving the call from the instance thread / WorldRunnable thread.
  */

typedef std::list<TimedEvent*> EventList;
typedef std::multimap<uint32, TimedEvent*> EventMap;

#define EVENT_REMOVAL_FLAG_ALL -1
#define WORLD_INSTANCE -1

class SERVER_DECL EventableObject
{
    friend class EventMgr;
    friend class EventableObjectHolder;

protected:
    void event_RemoveEvents();
    void event_RemoveEvents(int32 EventType);
    void event_ModifyTimeLeft(uint32 EventType, uint32 TimeLeft,bool unconditioned=false);
    void event_ModifyTime(uint32 EventType, uint32 Time);
    void event_ModifyTimeAndTimeLeft(uint32 EventType, uint32 Time);
    void event_ModifyAuraTimeLeft(uint32 Time, uint32 Auraid);
    bool event_HasEvent(uint32 EventType);
    void event_RemoveByPointer(TimedEvent * ev);
    RONIN_INLINE int32 event_GetCurrentMapId() { return m_event_MapId; }
    bool event_GetTimeLeft(uint32 EventType, uint32 * Time);

public:
    uint32 event_GetEventPeriod(uint32 EventType);
    // Public methods
    EventableObject();
    virtual ~EventableObject();
    virtual void Destruct();

    RONIN_INLINE bool event_HasEvents() { return m_events.size() > 0 ? true : false; }
    void event_AddEvent(TimedEvent * ptr);
    void event_Relocate();

    // this func needs to be implemented by all eventable classes. use it to retreive the map
    // id that it needs to attach itself to.

    virtual int32 event_GetMapID() { return -1; }

protected:

    int32 m_event_MapId;
    SmartMutex m_lock; // Smart mutex, in case some function calls internal event
    EventMap m_events;
    EventableObjectHolder * m_holder;
};

/**
  * @class EventableObjectHolder
  * EventableObjectHolder will store eventable objects, and remove/add them when they change
  * from one holder to another (changing maps).
  *
  * EventableObjectHolder also updates all the timed events in all of its objects when its
  * update function is called.
  *
  */

class EventableObjectHolder
{
public:
    EventableObjectHolder(int32 mapId);
    ~EventableObjectHolder();

    void Update(uint32 time_difference);

    void AddEvent(TimedEvent * ev);
    void AddObject(EventableObject * obj);

    RONIN_INLINE uint32 GetMapID() { return mMapId; }

protected:
    int32 mMapId;
    Mutex m_lock;
    EventList m_events;

    Mutex m_insertPoolLock;
    typedef std::list<TimedEvent*> InsertableQueue;
    InsertableQueue m_insertPool;
};
