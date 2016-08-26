
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
    void _AddEvent(size_t hash, CallbackBase *eventCallback, time_t occurTimer);
    void _AddEvent(size_t hash, CallbackBase *eventCallback, uint32 occurTimer);
    void _AddStaticEvent(size_t hash, CallbackBase *eventCallback, uint32 occurTimer);

private:
    Mutex m_lock;
    std::deque<EventBaseClass*> m_eventQueue;
    Loki::AssocVector<size_t, EventBaseClass*> m_dynamicEvents;

    std::vector<EventBaseClass*> m_staticEvents;
    std::vector<size_t> m_runningEvents;
    Object *_object;

public:
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
