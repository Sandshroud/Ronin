/***
 * Demonstrike Core
 */

#pragma once

class CallbackBase {
public:
    virtual void execute() = 0;
    virtual ~CallbackBase() {};
};

class CallbackFP {
    typedef void(*callback)();
    callback mycallback;
public:
    CallbackFP(callback cb) : mycallback(cb) {}
    void operator()();
    void execute();
    void set(callback cb) { mycallback = cb; }
    CallbackFP* Create();
};

template < class Class >
class CallbackP0 : public CallbackBase
{
public:

    typedef void (Class::*Method)();
    CallbackP0(Class* _class_instance, Method _method)
    {
       _obj = _class_instance;
       _func = _method;
    };
    void operator()() { return (_obj->*_func)(); }
    void execute() { return operator()(); }

private:

    Class*  _obj;
    Method  _func;
};

template < class Class, typename P1 >
class CallbackP1 : public CallbackBase
{
public:

    typedef void (Class::*Method)(P1);
    CallbackP1(Class* _class_instance, Method _method, P1 p1)
    {
       _obj = _class_instance;
       _func = _method;
       _p1 = p1;
    };

    void operator()() { return (_obj->*_func)(_p1); }
    void execute() { return operator()(); }

private:

    Class*  _obj;
    Method  _func;
    P1 _p1;
};

template < class Class, typename P1, typename P2 >
class CallbackP2 : public CallbackBase
{
public:

    typedef void (Class::*Method)(P1, P2);
    CallbackP2(Class* _class_instance, Method _method, P1 p1, P2 p2)
    {
       _obj = _class_instance;
       _func = _method;
       _p1 = p1;
       _p2 = p2;
    };

    void operator()() { return (_obj->*_func)(_p1, _p2); }
    void execute() { return operator()(); }

private:

    Class*  _obj;
    Method  _func;
    P1 _p1;
    P2 _p2;
};

template < class Class, typename P1, typename P2, typename P3 >
class CallbackP3 : public CallbackBase
{
public:

    typedef void (Class::*Method)(P1, P2, P3);
    CallbackP3(Class* _class_instance, Method _method, P1 p1, P2 p2, P3 p3)
    {
       _obj = _class_instance;
       _func = _method;
       _p1 = p1;
       _p2 = p2;
       _p3 = p3;
    };

    void operator()() { return (_obj->*_func)(_p1, _p2, _p3); }
    void execute() { return operator()(); }

private:

    Class*  _obj;
    Method  _func;
    P1 _p1;
    P2 _p2;
    P3 _p3;
};

template < class Class, typename P1, typename P2, typename P3, typename P4 >
class CallbackP4 : public CallbackBase
{
public:

    typedef void (Class::*Method)(P1, P2, P3, P4);
    CallbackP4(Class* _class_instance, Method _method, P1 p1, P2 p2, P3 p3, P4 p4)
    {
       _obj = _class_instance;
       _func = _method;
       _p1 = p1;
       _p2 = p2;
       _p3 = p3;
       _p4 = p4;
    };

    void operator()() { return (_obj->*_func)(_p1, _p2, _p3, _p4); }
    void execute() { return operator()(); }

private:

    Class*  _obj;
    Method  _func;
    P1 _p1;
    P2 _p2;
    P3 _p3;
    P4 _p4;
};

template < class Class, typename P1, typename P2, typename P3, typename P4, typename P5 >
class CallbackP5 : public CallbackBase
{
public:

    typedef void (Class::*Method)(P1, P2, P3, P4, P5);
    CallbackP5(Class* _class_instance, Method _method, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
       _obj = _class_instance;
       _func = _method;
       _p1 = p1;
       _p2 = p2;
       _p3 = p3;
       _p4 = p4;
       _p5 = p5;
    };

    void operator()() { return (_obj->*_func)(_p1, _p2, _p3, _p4, _p5); }
    void execute() { return operator()(); }

private:

    Class*  _obj;
    Method  _func;
    P1 _p1;
    P2 _p2;
    P3 _p3;
    P4 _p4;
    P5 _p5;
};

template < class Class, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
class CallbackP6 : public CallbackBase
{
public:

    typedef void (Class::*Method)(P1, P2, P3, P4, P5, P6);
    CallbackP6(Class* _class_instance, Method _method, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
       _obj = _class_instance;
       _func = _method;
       _p1 = p1;
       _p2 = p2;
       _p3 = p3;
       _p4 = p4;
       _p5 = p5;
       _p6 = p6;
    };

    void operator()() { return (_obj->*_func)(_p1, _p2, _p3, _p4, _p5, _p6); }
    void execute() { return operator()(); }

private:

    Class*  _obj;
    Method  _func;
    P1 _p1;
    P2 _p2;
    P3 _p3;
    P4 _p4;
    P5 _p5;
    P6 _p6;
};
