/*
 * This file exists as a header for all dependencies so that we can keep data alligned appropriately
*/

#pragma once

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

// current platform and compiler
#define PLATFORM_WIN   0
#define PLATFORM_UNIX  1
#define PLATFORM_APPLE 2
#define PLATFORM_INTEL 3

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
#define _WIN32_WINNT 0x0500
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#define HEARTHSTONE_INLINE __forceinline

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
#define HEARTHSTONE_INLINE inline
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

#ifdef _MSC_VER

#define isnan(x) _isnan(x)
#define I64FMT "%016I64X"
#define I64FMTD "%I64u"
#define SI64FMTD "%I64d"
#define snprintf _snprintf
#define atoll __atoi64

#else

#define isnan(x) std::isnan(x)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I64FMT "%016llX"
#define I64FMTD "%llu"
#define SI64FMTD "%lld"

#endif

HEARTHSTONE_INLINE uint32 hearthstone_GetThreadId()
{
#if PLATFORM == PLATFORM_WIN
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif
}