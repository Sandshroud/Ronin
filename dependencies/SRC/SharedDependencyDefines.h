/*
 * This file exists as a header for all dependencies so that we can keep data alligned appropriately
 *
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

#pragma warning(disable:4091)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <assert.h>

#include <set>
#include <list>
#include <map>
#include <vector>
#include <queue>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <atomic>

// current platform and compiler
#define PLATFORM_WIN   0
#define PLATFORM_UNIX  1
#define PLATFORM_APPLE 2
#define PLATFORM_INTEL 3

#define __STORMLIB_SELF__

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined(_WIN64)
#  define PLATFORM PLATFORM_WIN
#elif defined( __INTEL_COMPILER )
#  define PLATFORM PLATFORM_INTEL
#elif defined( __APPLE_CC__ )
#  define PLATFORM PLATFORM_APPLE
#else
#  define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#define RONIN_INLINE __forceinline

#ifdef byte
#undef byte
#endif

#ifndef SCRIPTLIB
#define SERVER_DECL __declspec(dllexport)
#define SCRIPT_DECL __declspec(dllimport)
#else
#define SERVER_DECL __declspec(dllimport)
#define SCRIPT_DECL __declspec(dllexport)
#endif
#else
 #define SERVER_DECL
 #define SCRIPT_DECL
#define RONIN_INLINE inline
#endif

#include <string>
#ifndef MAX_PATH
#define MAX_PATH 255
#endif

/* Use correct types for x64 platforms, too */
typedef unsigned int uint;
typedef unsigned long ulong;

#ifdef __GNUC__

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef uint32_t DWORD;

#else

typedef signed __int64 int64;
typedef signed __int32 int32;
typedef signed __int16 int16;
typedef signed __int8 int8;

typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8 uint8;

#endif

#if _MSC_VER

#define snprintf _snprintf

#define num_isnan(x) _isnan(x)
#if _MSC_VER >= 1400
#define I64FMT "%016llX"
#define I64FMTD "%lld"
#define UI64FMTD "%llu"
#else
#define I64FMT "%016I64X"
#define I64FMTD "%I64d"
#define UI64FMTD "%I64u"
#endif

#else

#define num_isnan(x) std::isnan(x)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I64FMT "%016llX"
#define I64FMTD "%lld"
#define UI64FMTD "%llu"

#endif

RONIN_INLINE uint32 ronin_GetThreadId()
{
#if PLATFORM == PLATFORM_WIN
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif
}