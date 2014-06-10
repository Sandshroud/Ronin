/***
 * Demonstrike Core
 */

//
// WorldRunnable.cpp
//

#include "StdAfx.h"

#define WORLD_UPDATE_DELAY 50

initialiseSingleton( WorldRunnable );

WorldRunnable::WorldRunnable(EventableObjectHolder* m_Holder) : ThreadContext()
{
    eventHolder = m_Holder;
}

bool WorldRunnable::run()
{
    uint32 LastWorldUpdate = getMSTime();
    uint32 LastSessionsUpdate = getMSTime();
    uint32 LastEventHolderUpdate = getMSTime();

    while(GetThreadState() != THREADSTATE_TERMINATE)
    {
        // Provision for pausing this thread.
        if(GetThreadState() == THREADSTATE_PAUSED)
            while(GetThreadState() == THREADSTATE_PAUSED)
                Delay(200);

        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        uint32 diff;
        //calce time passed
        uint32 now, execution_start;
        now = getMSTime();
        execution_start = now;

        if( now < LastWorldUpdate)//overrun
            diff = WORLD_UPDATE_DELAY;
        else
            diff = now - LastWorldUpdate;

        LastWorldUpdate = now;
        sWorld.Update( diff );
        if(GetThreadState() == THREADSTATE_TERMINATE)
            break;

        now = getMSTime();

        if( now < LastEventHolderUpdate)//overrun
            diff = WORLD_UPDATE_DELAY;
        else
            diff = now - LastEventHolderUpdate;
        LastEventHolderUpdate = now;

        // Update any events.
        eventHolder->Update(diff);
        if(GetThreadState() == THREADSTATE_TERMINATE)
            break;

        now = getMSTime();

        if( now < LastSessionsUpdate)//overrun
            diff = WORLD_UPDATE_DELAY;
        else
            diff = now - LastSessionsUpdate;

        LastSessionsUpdate = now;
        sWorld.UpdateSessions( diff );

        now = getMSTime();
        //we have to wait now

        if(execution_start > now)//overrun
            diff = WORLD_UPDATE_DELAY - now;
        else
            diff = now - execution_start;//time used for updating

        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;
        /* This is execution time compensating system
        if execution took more than default delay
        no need to make this sleep*/
        int32 sleepTime = WORLD_UPDATE_DELAY-diff;
        if(sleepTime > 0)
            Delay(sleepTime);
    }

    return true;
}
