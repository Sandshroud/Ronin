/***
 * Demonstrike Core
 */

#pragma once

// Include our basic definitions
#include "../SharedDependencyDefines.h"

// Include our tick timer
#include "TickTimer.h"

// Include log header from dependencies
#include "../consolelog/consolelog.h"

// We need to be able to anticipate crashes
#include "StackWalker.h"
#include "CrashHandler.h"

// Asyncronous callback
#include "CallBack.h"

// We need assertions.
#include "Errors.h"

// For singular existence items
#include "Singleton.h"

// Platform Specific Lock Implementation
#include "Mutex.h"

// Platform Independant Guard
#include "Guard.h"

// Condition locking
#include "Condition.h"

// For data queuing
#include "Queue.h"

// Read and write condition locking
#include "RWLock.h"

// Platform Specific Thread Starter
#include "ThreadStarter.h"

// Platform independant locked queue
#include "LockedQueue.h"

// Thread Pool
#include "ThreadPool.h"
