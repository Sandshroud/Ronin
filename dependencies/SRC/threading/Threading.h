/*
 * Sandshroud Project Ronin
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

// Include our basic definitions
#include "../SharedDependencyDefines.h"

// Include our tick timer
#include "TickTimer.h"

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

// Implementation of an atomic rotator
#include "AtomicRotator.h"

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
#include "ThreadManagement.h"

// Thread safe console log
#include "ConsoleLog.h"

// Thread Task
#include "ThreadTask.h"

// Thread Task List
#include "ThreadTaskList.h"
