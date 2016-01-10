/***
 * Demonstrike Core
 */

#pragma once

/*
MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
MSVC++ 7.0  _MSC_VER == 1300
MSVC++ 6.0  _MSC_VER == 1200
MSVC++ 5.0  _MSC_VER == 1100
*/

enum TimeVariables
{
    TIME_SECOND = 1,
    TIME_MINUTE = TIME_SECOND * 60,
    TIME_HOUR   = TIME_MINUTE * 60,
    TIME_DAY    = TIME_HOUR * 24,
    TIME_MONTH  = TIME_DAY * 30,
    TIME_YEAR   = TIME_MONTH * 12,
};

enum MsTimeVariables
{
    MSTIME_SECOND = 1000,
    MSTIME_MINUTE = MSTIME_SECOND * 60,
    MSTIME_HOUR   = MSTIME_MINUTE * 60,
    MSTIME_DAY    = MSTIME_HOUR * 24,
};

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <errno.h>

#ifdef CONFIG_USE_SELECT
#undef FD_SETSIZE
#define FD_SETSIZE 2048
#endif

// current platform and compiler
#define PLATFORM_WIN   0
#define PLATFORM_UNIX  1
#define PLATFORM_APPLE 2

#define UNIX_FLAVOUR_LINUX 1
#define UNIX_FLAVOUR_BSD   2
#define UNIX_FLAVOUR_OTHER 3
#define UNIX_FLAVOUR_OSX   4

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined(_WIN64)
#  define PLATFORM PLATFORM_WIN
#elif defined( __INTEL_COMPILER )
#  define PLATFORM PLATFORM_INTEL
#elif defined( __APPLE_CC__ )
#  define PLATFORM PLATFORM_APPLE
#else
#  define PLATFORM PLATFORM_UNIX
#endif

#define COMPILER_MICROSOFT 0
#define COMPILER_GNU       1
#define COMPILER_BORLAND   2
#define COMPILER_INTEL     3

#ifdef _MSC_VER
#  define COMPILER COMPILER_MICROSOFT
#elif defined( __INTEL_COMPILER )
#  define COMPILER COMPILER_INTEL
#elif defined( __BORLANDC__ )
#  define COMPILER COMPILER_BORLAND
#elif defined( __GNUC__ )
#  define COMPILER COMPILER_GNU
#else
#  pragma error "FATAL ERROR: Unknown compiler."
#endif

#if PLATFORM == PLATFORM_WIN
# define PLATFORM_TEXT "Win86"
# define WIN32_LEAN_AND_MEAN
# define _WIN32_WINNT 0x0500
# define NOMINMAX
# include <windows.h>
#else
# include <string.h>
# define MAX_PATH 1024
# if PLATFORM == PLATFORM_UNIX || PLATFORM == PLATFORM_APPLE
#  ifdef HAVE_DARWIN
#   define PLATFORM_TEXT "MacOSX"
#   define UNIX_FLAVOUR UNIX_FLAVOUR_OSX
#  else
#   ifdef USE_KQUEUE
#    define PLATFORM_TEXT "FreeBSD"
#    define UNIX_FLAVOUR UNIX_FLAVOUR_BSD
#   else
#    ifdef USE_KQUEUE_DFLY
#     define PLATFORM_TEXT "DragonFlyBSD"
#     define UNIX_FLAVOUR UNIX_FLAVOUR_BSD
#    else
#     define PLATFORM_TEXT "Linux"
#     define UNIX_FLAVOUR UNIX_FLAVOUR_LINUX
#    endif
#   endif
#  endif
# endif
#endif

#ifdef _DEBUG
#define CONFIG "Debug"
#else
#define CONFIG "Release"
#endif

#ifdef _WIN64
#define ARCH "x64"
#else
#define ARCH "x86"
#endif

#if PLATFORM == PLATFORM_WIN
# include <winsock2.h>
# include <ws2tcpip.h>
# define ASYNC_NET
#else
# include <sys/time.h>
# include <sys/types.h>
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <signal.h>
# include <netdb.h>
# define STRCASECMP strcasecmp
#  ifdef USE_EPOLL
# define CONFIG_USE_EPOLL
# elif USE_KQUEUE
#  define CONFIG_USE_KQUEUE
# elif USE_KQUEUE_DFLY
# define CONFIG_USE_KQUEUE_DFLY
#  elif USE_SELECT
# define CONFIG_USE_SELECT
#  elif USE_POLL
# define CONFIG_USE_POLL
# endif
#endif

#ifdef min
# undef min
#endif

#ifdef max
# undef max
#endif

#include <set>
#include <map>
#include <list>
#include <string>
#include <queue>
#include <sstream>
#include <algorithm>
#include <memory>

#if PLATFORM == PLATFORM_WIN
# pragma warning(disable:4996)
# undef _CRT_SECURE_NO_DEPRECATE
# undef _CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES
# define _CRT_SECURE_NO_DEPRECATE 1
# define _CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES 1
# pragma warning(disable:4251)       // dll-interface bullshit
# define RONIN_INLINE __forceinline
#else
# define RONIN_INLINE inline
# if defined (__GNUC__)
#  define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#  ifndef X64
#   if GCC_VERSION >= 30400
#    ifdef HAVE_DARWIN
#     define __fastcall
#    else
#     define __fastcall __attribute__((__fastcall__))
#    endif
#   else
#    define __fastcall __attribute__((__regparm__(3)))
#   endif
#  else
#   define __fastcall  
#  endif
# else
#  define __fastcall __attribute__((__fastcall__))
# endif
#endif

/* Use correct types for x64 platforms, too */
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long signed int LLSI;
typedef long long unsigned int LLUI;
typedef char const* const* PackedString;

#if COMPILER == COMPILER_GNU
typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef uint32_t DWORD;

// Packing allignment GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#define PRAGMA_PACK 1
#define PRAGMA_POP
#else
typedef signed __int64 int64;
typedef signed __int32 int32;
typedef signed __int16 int16;
typedef signed __int8 int8;

typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8 uint8;

// Packing allignment
#define PRAGMA_PACK push,1
#define PRAGMA_POP pop
#endif

/*
Scripting system exports/imports
*/

#if PLATFORM == PLATFORM_WIN
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
#endif

// Include all threading files
#include <assert.h>

#include "format.h"
#include "Threading/Threading.h"
#include "MersenneTwister.h"

#if COMPILER == COMPILER_MICROSOFT

#define snprintf _snprintf

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

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I64FMT "%016llX"
#define I64FMTD "%lld"
#define UI64FMTD "%llu"

#endif

#define atol(a) strtoul( a, NULL, 10)
#define atoll(a) _strtoui64(a, NULL, 10)

// fast int abs
static inline int int32abs( const int value )
{
    return (value ^ (value >> 31)) - (value >> 31);
}

// fast int abs and recast to unsigned
static inline uint32 int32abs2uint32( const int value )
{
    return (uint32)(value ^ (value >> 31)) - (value >> 31);
}

#if COMPILER == COMPILER_MICROSOFT

#pragma float_control(push)
#pragma float_control(precise, on)

#endif

/// Fastest Method of float2int32
static inline int float2int32(const float value)
{
#if !defined(_WIN64) && COMPILER == COMPILER_MICROSOFT
    int i;
    __asm {
        fld value
        frndint
        fistp i
    }
    return i;
#else
    union { int asInt[2]; double asDouble; } n;
    n.asDouble = value + 6755399441055744.0;

    return n.asInt [0];
#endif
}

/// Fastest Method of long2int32
static inline int double2int32(const double value)
{
#if !defined(_WIN64) && COMPILER == COMPILER_MICROSOFT
    int i;
    __asm {
        fld value
        frndint
        fistp i
    }
    return i;
#else
  union { int asInt[2]; double asDouble; } n;
  n.asDouble = value + 6755399441055744.0;
  return n.asInt [0];
#endif
}

/// Fastest Method of double2int64
static inline long long int double2int64(const double value)
{
  union { long long int asInt; double asDouble; } n;
  n.asDouble = value + 6755399441055744.0;
  return n.asInt;
}

// modulos a radian orientation to the range of 0..2PI
static inline float NormAngle(float o)
{
    // fmod only supports positive numbers. Thus we have
    // to emulate negative numbers
    if (o < 0)
    {
        float mod = o * -1;
        mod = fmodf(mod, 6.2832f);
        mod = -mod + 6.2832f;
        return mod;
    } else if(o < 6.2832f)
        return o;
    return fmod(o, 6.2832f);
}

#if COMPILER == COMPILER_MICROSOFT
#pragma float_control(pop)
#endif

#if PLATFORM != PLATFORM_WIN
#include <sys/timeb.h>
#define FALSE   0
#define TRUE    1
#define Sleep(ms) usleep(1000*ms)
#endif

#include "Util.h"
#include "ByteConverter.h"
#include "Console/CConsole.h"
