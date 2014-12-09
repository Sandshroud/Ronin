/***
 * Demonstrike Core
 */

#pragma once

//! Other libs we depend on.
#include "../SharedDependencyDefines.h"

// Socket code required for db connections
#if PLATFORM == PLATFORM_WIN
#include <winsock2.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#endif

#include <mysql.h>

//! Our own includes.
#include "Field.h"
#include "DatabaseCallback.h"
#include "DirectDatabase.h"
#include "DatabaseEngine.h"
