
#pragma once

class Object;
class EventBaseClass;

#define GEN_METH_ID(m) size_t hash; char buff[16+2+1]; sprintf_s(buff, "0x%p", m); hash = std::stoull(buff, nullptr, 16); ASSERT(hash != 0);

class EventHandler
{
public:
    EventHandler(Object *obj);
    ~EventHandler();

    void Init();
    void Update(uint32 msDiff, time_t curTime);

protected:
    // Event internal handlers
    bool _HasEvent(size_t hash);
    void _RemoveEvent(size_t hash);
    void _AddEvent(size_t hash, CallbackBase *eventCallback, time_t occurTimer);
    void _AddEvent(size_t hash, CallbackBase *eventCallback, uint32 occurTimer);

    // Static event internal handlers
    bool _HasStaticEvent(size_t hash);
    void _RemoveStaticEvent(size_t hash);
    void _AddStaticEvent(size_t hash, CallbackBase *eventCallback, uint32 occurTimer);

private:
    Mutex m_lock;
    std::deque<EventBaseClass*> m_eventQueue;
    Loki::AssocVector<size_t, EventBaseClass*> m_dynamicEvents;

    std::vector<EventBaseClass*> m_staticEvents;
    std::vector<size_t> m_runningEvents;
    Object *_object;

public:
    template <class Class> bool HasEvent(Class *obj, void (Class::*method)(void)) { GEN_METH_ID(method); return _HasEvent(hash); }
    template <class Class, typename P1> bool HasEvent(Class *obj, void (Class::*method)(P1)) { GEN_METH_ID(method); return _HasEvent(hash); }
    template <class Class, typename P1, typename P2> bool HasEvent(Class *obj, void (Class::*method)(P1,P2)) { GEN_METH_ID(method); return _HasEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3> bool HasEvent(Class *obj, void (Class::*method)(P1,P2,P3)) { GEN_METH_ID(method); return _HasEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4> bool HasEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4)) { GEN_METH_ID(method); return _HasEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5> bool HasEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4,P5)) { GEN_METH_ID(method); return _HasEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> bool HasEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4,P5,P6)) { GEN_METH_ID(method); return _HasEvent(hash); }

    template <class Class> void RemoveEvent(Class *obj, void (Class::*method)(void)) { GEN_METH_ID(method); _RemoveEvent(hash); }
    template <class Class, typename P1> void RemoveEvent(Class *obj, void (Class::*method)(P1)) { GEN_METH_ID(method); _RemoveEvent(hash); }
    template <class Class, typename P1, typename P2> void RemoveEvent(Class *obj, void (Class::*method)(P1,P2)) { GEN_METH_ID(method); _RemoveEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3> void RemoveEvent(Class *obj, void (Class::*method)(P1,P2,P3)) { GEN_METH_ID(method); _RemoveEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4> void RemoveEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4)) { GEN_METH_ID(method); _RemoveEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5> void RemoveEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4,P5)) { GEN_METH_ID(method); _RemoveEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> void RemoveEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4,P5,P6)) { GEN_METH_ID(method); _RemoveEvent(hash); }

    template <class Class> void AddEvent(Class* obj, void (Class::*method)(void), time_t occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP0<Class>(obj, method), occurTimer);
    }

    template <class Class, typename P1> void AddEvent(Class* obj, void (Class::*method)(P1), P1 p1, time_t occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP1<Class, P1>(obj, method, p1), occurTimer);
    }

    template <class Class, typename P1, typename P2> void AddEvent(Class* obj, void (Class::*method)(P1,P2), P1 p1, P2 p2, time_t occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP2<Class, P1, P2>(obj, method, p1, p2), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3> void AddEvent(Class* obj, void (Class::*method)(P1,P2,P3), P1 p1, P2 p2, P3 p3, time_t occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP3<Class, P1, P2, P3>(obj, method, p1, p2, p3), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3, typename P4> void AddEvent(Class* obj, void (Class::*method)(P1,P2,P3,P4), P1 p1, P2 p2, P3 p3, P4 p4, time_t occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP4<Class, P1, P2, P3, P4>(obj, method, p1, p2, p3, p4), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5> void AddEvent(Class* obj, void (Class::*method)(P1,P2,P3,P4,P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, time_t occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP5<Class, P1, P2, P3, P4, P5>(obj, method, p1, p2, p3, p4, p5), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> void AddEvent(Class* obj, void (Class::*method)(P1,P2,P3,P4,P5,P6), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, time_t occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP6<Class, P1, P2, P3, P4, P5, P6>(obj, method, p1, p2, p3, p4, p5, p6), occurTimer);
    }

    template <class Class> void AddEvent(Class* obj, void (Class::*method)(void), uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP0<Class>(obj, method), occurTimer);
    }

    template <class Class, typename P1> void AddEvent(Class* obj, void (Class::*method)(P1), P1 p1, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP1<Class, P1>(obj, method, p1), occurTimer);
    }

    template <class Class, typename P1, typename P2> void AddEvent(Class* obj, void (Class::*method)(P1,P2), P1 p1, P2 p2, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP2<Class, P1, P2>(obj, method, p1, p2), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3> void AddEvent(Class* obj, void (Class::*method)(P1,P2,P3), P1 p1, P2 p2, P3 p3, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP3<Class, P1, P2, P3>(obj, method, p1, p2, p3), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3, typename P4> void AddEvent(Class* obj, void (Class::*method)(P1,P2,P3,P4), P1 p1, P2 p2, P3 p3, P4 p4, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP4<Class, P1, P2, P3, P4>(obj, method, p1, p2, p3, p4), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5> void AddEvent(Class* obj, void (Class::*method)(P1,P2,P3,P4,P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP5<Class, P1, P2, P3, P4, P5>(obj, method, p1, p2, p3, p4, p5), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> void AddEvent(Class* obj, void (Class::*method)(P1,P2,P3,P4,P5,P6), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push dynamic event to be handled once when timer ends
        _AddEvent(hash, new CallbackP6<Class, P1, P2, P3, P4, P5, P6>(obj, method, p1, p2, p3, p4, p5, p6), occurTimer);
    }

    template <class Class> bool HasStaticEvent(Class *obj, void (Class::*method)(void)) { GEN_METH_ID(method); return _HasStaticEvent(hash); }
    template <class Class, typename P1> bool HasStaticEvent(Class *obj, void (Class::*method)(P1)) { GEN_METH_ID(method); return _HasStaticEvent(hash); }
    template <class Class, typename P1, typename P2> bool HasStaticEvent(Class *obj, void (Class::*method)(P1,P2)) { GEN_METH_ID(method); return _HasStaticEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3> bool HasStaticEvent(Class *obj, void (Class::*method)(P1,P2,P3)) { GEN_METH_ID(method); return _HasStaticEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4> bool HasStaticEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4)) { GEN_METH_ID(method); return _HasStaticEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5> bool HasStaticEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4,P5)) { GEN_METH_ID(method); return _HasStaticEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> bool HasStaticEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4,P5,P6)) { GEN_METH_ID(method); return _HasStaticEvent(hash); }

    template <class Class> void RemoveStaticEvent(Class *obj, void (Class::*method)(void)) { GEN_METH_ID(method); _RemoveStaticEvent(hash); }
    template <class Class, typename P1> void RemoveStaticEvent(Class *obj, void (Class::*method)(P1)) { GEN_METH_ID(method); _RemoveStaticEvent(hash); }
    template <class Class, typename P1, typename P2> void RemoveStaticEvent(Class *obj, void (Class::*method)(P1,P2)) { GEN_METH_ID(method); _RemoveStaticEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3> void RemoveStaticEvent(Class *obj, void (Class::*method)(P1,P2,P3)) { GEN_METH_ID(method); _RemoveStaticEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4> void RemoveStaticEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4)) { GEN_METH_ID(method); _RemoveStaticEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5> void RemoveStaticEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4,P5)) { GEN_METH_ID(method); _RemoveStaticEvent(hash); }
    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> void RemoveStaticEvent(Class *obj, void (Class::*method)(P1,P2,P3,P4,P5,P6)) { GEN_METH_ID(method); _RemoveStaticEvent(hash); }

    template <class Class> void AddStaticEvent(Class* obj, void (Class::*method)(void), uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push static event to be handled repeatedly when timer ends
        _AddStaticEvent(hash, new CallbackP0<Class>(obj, method), occurTimer);
    }

    template <class Class, typename P1> void AddStaticEvent(Class* obj, void (Class::*method)(P1), P1 p1, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push static event to be handled repeatedly when timer ends
        _AddStaticEvent(hash, new CallbackP1<Class, P1>(obj, method, p1), occurTimer);
    }

    template <class Class, typename P1, typename P2> void AddStaticEvent(Class* obj, void (Class::*method)(P1,P2), P1 p1, P2 p2, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push static event to be handled repeatedly when timer ends
        _AddStaticEvent(hash, new CallbackP2<Class, P1, P2>(obj, method, p1, p2), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3> void AddStaticEvent(Class* obj, void (Class::*method)(P1,P2,P3), P1 p1, P2 p2, P3 p3, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push static event to be handled repeatedly when timer ends
        _AddStaticEvent(hash, new CallbackP3<Class, P1, P2, P3>(obj, method, p1, p2, p3), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3, typename P4> void AddStaticEvent(Class* obj, void (Class::*method)(P1,P2,P3,P4), P1 p1, P2 p2, P3 p3, P4 p4, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push static event to be handled repeatedly when timer ends
        _AddStaticEvent(hash, new CallbackP4<Class, P1, P2, P3, P4>(obj, method, p1, p2, p3, p4), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5> void AddStaticEvent(Class* obj, void (Class::*method)(P1,P2,P3,P4,P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push static event to be handled repeatedly when timer ends
        _AddStaticEvent(hash, new CallbackP5<Class, P1, P2, P3, P4, P5>(obj, method, p1, p2, p3, p4, p5), occurTimer);
    }

    template <class Class, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> void AddStaticEvent(Class* obj, void (Class::*method)(P1,P2,P3,P4,P5,P6), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, uint32 occurTimer)
    {
        GEN_METH_ID(method); // We need to generate a static identifier for our method
        // Now push static event to be handled repeatedly when timer ends
        _AddStaticEvent(hash, new CallbackP6<Class, P1, P2, P3, P4, P5, P6>(obj, method, p1, p2, p3, p4, p5, p6), occurTimer);
    }
};

class EventBaseClass
{
public:
    EventBaseClass(size_t hash, CallbackBase *callback) : funcHash(hash), cb(callback) {}
    ~EventBaseClass() { delete cb; }

    void Activate() { cb->execute(); }
    size_t getFuncHash() { return funcHash; }

    virtual bool Update(uint32 msDiff, time_t curTime) = 0;
    virtual void Reset() {};

private:
    size_t funcHash;
    CallbackBase *cb;
};

class TimedEvent : public EventBaseClass
{
public:
    TimedEvent(CallbackBase* callback, size_t hash, time_t occurTimer) : EventBaseClass(hash, callback), eventTimer(occurTimer) {}
    ~TimedEvent() {}

    bool Update(uint32 msDiff, time_t curTime) { return curTime >= eventTimer; }
private:
    time_t eventTimer;
};

class DelayedEvent : public EventBaseClass
{
public:
    DelayedEvent(CallbackBase* callback, size_t hash, uint32 occurTimer) : EventBaseClass(hash, callback), eventTimer(occurTimer), curTimer(occurTimer) {}
    ~DelayedEvent() {}

    bool Update(uint32 msDiff, time_t curTime)
    {
        if(curTimer > msDiff)
            curTimer -= msDiff;
        else curTimer = 0;
        return curTimer == 0;
    }

    void Reset() { curTimer = eventTimer; };
private:
    uint32 eventTimer, curTimer;
};
