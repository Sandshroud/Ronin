/***
 * Demonstrike Core
 */

//
// WorldRunnable.h
//

#pragma once

class WorldRunnable : public ThreadContext, public Singleton<WorldRunnable>
{
public:
    WorldRunnable(EventableObjectHolder* m_Holder);
    bool run();

    EventableObjectHolder* eventHolder;
};

#define sWorldRunnable WorldRunnable::getSingleton()
