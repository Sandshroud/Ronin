/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton( EventMgr );

TimedEvent * TimedEvent::Allocate(void* object, CallbackBase* callback, uint32 flags, time_t time, uint32 repeat)
{
    return new TimedEvent(object, callback, flags, time, repeat, 0, 0);
}
