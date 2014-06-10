
#pragma once

class QueryResult;
struct AsyncQueryResult;
typedef std::vector<AsyncQueryResult> QueryResultVector;

class SQLCallbackBase
{
public:
    virtual ~SQLCallbackBase();
    virtual void run(QueryResultVector & result) = 0;
};

template<class T>
class SQLClassCallbackP0 : public SQLCallbackBase
{
    typedef void (T::*SCMethod)(QueryResultVector & p);
    T * base;
    SCMethod method;
public:
    SQLClassCallbackP0(T* instance, SCMethod imethod) : SQLCallbackBase(), base(instance), method(imethod) {}
    ~SQLClassCallbackP0() {}
    void run(QueryResultVector & data) { (base->*method)(data); }
};

template<class T, typename P1>
class SQLClassCallbackP1 : public SQLCallbackBase
{
    typedef void (T::*SCMethod)(QueryResultVector & p, P1 p1);
    T * base;
    SCMethod method;
    P1 par1;
public:
    SQLClassCallbackP1(T* instance, SCMethod imethod, P1 p1) : SQLCallbackBase(), base(instance), method(imethod), par1(p1) {}
    ~SQLClassCallbackP1() {}
    void run(QueryResultVector & data) { (base->*method)(data, par1); }
};

template<class T, typename P1, typename P2>
class SQLClassCallbackP2 : public SQLCallbackBase
{
    typedef void (T::*SCMethod)(QueryResultVector & p, P1 p1, P2 p2);
    T * base;
    SCMethod method;
    P1 par1;
    P2 par2;
public:
    SQLClassCallbackP2(T* instance, SCMethod imethod, P1 p1, P2 p2) : SQLCallbackBase(), base(instance), method(imethod), par1(p1), par2(p2) {}
    ~SQLClassCallbackP2() {}
    void run(QueryResultVector & data) { (base->*method)(data, par1, par2); }
};

template<class T, typename P1, typename P2, typename P3>
class SQLClassCallbackP3 : public SQLCallbackBase
{
    typedef void (T::*SCMethod)(QueryResultVector & p, P1 p1, P2 p2, P3 p3);
    T * base;
    SCMethod method;
    P1 par1;
    P2 par2;
    P3 par3;
public:
    SQLClassCallbackP3(T* instance, SCMethod imethod, P1 p1, P2 p2, P3 p3) : SQLCallbackBase(), base(instance), method(imethod), par1(p1), par2(p2), par3(p3) {}
    ~SQLClassCallbackP3();
    void run(QueryResultVector & data) { (base->*method)(data, par1, par2, par3); }
};

template<class T, typename P1, typename P2, typename P3, typename P4>
class SQLClassCallbackP4 : public SQLCallbackBase
{
    typedef void (T::*SCMethod)(QueryResultVector & p, P1 p1, P2 p2, P3 p3, P4 p4);
    T * base;
    SCMethod method;
    P1 par1;
    P2 par2;
    P3 par3;
    P4 par4;
public:
    SQLClassCallbackP4(T* instance, SCMethod imethod, P1 p1, P2 p2, P3 p3, P4 p4) : SQLCallbackBase(), base(instance), method(imethod), par1(p1), par2(p2), par3(p3), par4(p4) {}
    ~SQLClassCallbackP4() {}
    void run(QueryResultVector & data) { (base->*method)(data, par1, par2, par3, par4); }
};

class SQLFunctionCallbackP0 : public SQLCallbackBase
{
    typedef void(*SCMethod)(QueryResult*);
    SCMethod method;
public:
    SQLFunctionCallbackP0(SCMethod m) : SQLCallbackBase(), method(m) {}
    ~SQLFunctionCallbackP0();
    void run(QueryResult * data) { method(data); }
};

template<typename T1>
class SQLFunctionCallbackP1 : public SQLCallbackBase
{
    typedef void(*SCMethod)(QueryResult*, T1 p1);
    SCMethod method;
    T1 p1;
public:
    SQLFunctionCallbackP1(SCMethod m, T1 par1) : SQLCallbackBase(), method(m), p1(par1) {}
    ~SQLFunctionCallbackP1();
    void run(QueryResult * data) { method(data, p1); }
};
