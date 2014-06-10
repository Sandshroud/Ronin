/***
 * Demonstrike Core
 */

#pragma once

#if defined(WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES 1
#pragma warning(disable:4251)       // dll-interface bullshit
#endif

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
    MSTIME_DAY  = MSTIME_HOUR * 24,
};

#if defined(WIN32) || defined(_WIN64)
#define HEARTHSTONE_INLINE __forceinline
#else
#define HEARTHSTONE_INLINE inline
#endif

#include "hearthstoneConfig.h"
#include "format.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <errno.h>

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined(_WIN64)
#  define WIN32_LEAN_AND_MEAN
#  define _WIN32_WINNT 0x0500
#  define NOMINMAX
#  include <windows.h>
#else
#  include <string.h>
#  define MAX_PATH 1024
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef CONFIG_USE_SELECT
#undef FD_SETSIZE
#define FD_SETSIZE 2048
#endif

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
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

#if PLATFORM == PLATFORM_UNIX || PLATFORM == PLATFORM_APPLE
#ifdef HAVE_DARWIN
#define PLATFORM_TEXT "MacOSX"
#define UNIX_FLAVOUR UNIX_FLAVOUR_OSX
#else
#ifdef USE_KQUEUE
#define PLATFORM_TEXT "FreeBSD"
#define UNIX_FLAVOUR UNIX_FLAVOUR_BSD
#else
#ifdef USE_KQUEUE_DFLY
#define PLATFORM_TEXT "DragonFlyBSD"
#define UNIX_FLAVOUR UNIX_FLAVOUR_BSD
#else
#define PLATFORM_TEXT "Linux"
#define UNIX_FLAVOUR UNIX_FLAVOUR_LINUX
#endif
#endif
#endif
#endif

#if PLATFORM == PLATFORM_WIN
#define PLATFORM_TEXT "Win86"
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

/*#if COMPILER == COMPILER_MICROSOFT
#  pragma warning( disable : 4267 ) // conversion from 'size_t' to 'int', possible loss of data
#  pragma warning( disable : 4311 ) // 'type cast': pointer truncation from HMODULE to uint32
#  pragma warning( disable : 4786 ) // identifier was truncated to '255' characters in the debug information
#  pragma warning( disable : 4146 )
#  pragma warning( disable : 4800 )
#endif*/

#if PLATFORM == PLATFORM_WIN
#define STRCASECMP stricmp
#else
#define STRCASECMP strcasecmp
#endif

#if PLATFORM == PLATFORM_WIN
    #define ASYNC_NET
#endif

#ifdef USE_EPOLL
    #define CONFIG_USE_EPOLL
#endif
#ifdef USE_KQUEUE
    #define CONFIG_USE_KQUEUE
#endif
#ifdef USE_KQUEUE_DFLY
    #define CONFIG_USE_KQUEUE_DFLY
#endif
#ifdef USE_SELECT
    #define CONFIG_USE_SELECT
#endif
#ifdef USE_POLL
    #define CONFIG_USE_POLL
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <set>
#include <list>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>
//#include <iostream>

#if defined (__GNUC__)
#  define GCC_VERSION (__GNUC__ * 10000 \
                       + __GNUC_MINOR__ * 100 \
                       + __GNUC_PATCHLEVEL__)
#endif


#ifndef WIN32
#ifndef _WIN64
#  if defined (__GNUC__)
#   if GCC_VERSION >= 30400
#         ifdef HAVE_DARWIN
#         define __fastcall
#         else
#             define __fastcall __attribute__((__fastcall__))
#         endif
#   else
#     define __fastcall __attribute__((__regparm__(3)))
#   endif
#  else
#   define __fastcall __attribute__((__fastcall__))
#  endif
#else
#define __fastcall
#endif
#endif

#if COMPILER == COMPILER_GNU
// Packing allignment GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#define PRAGMA_PACK 1
#define PRAGMA_POP
#else
// Packing allignment
#define PRAGMA_PACK push,1
#define PRAGMA_POP pop
#endif

#if COMPILER == COMPILER_INTEL

#include <ext/hash_map>

#elif COMPILER == COMPILER_GNU && __GNUC__ >= 4

#include <tr1/memory>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

#elif COMPILER == COMPILER_GNU && __GNUC__ >= 3

#include <ext/hash_map>
#include <ext/hash_set>

#elif COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1700

#include <memory>
#include <unordered_map>
#include <unordered_set>

#elif COMPILER == COMPILER_MICROSOFT && (_MSC_VER < 1600 || !_HAS_TR1)

#pragma message ("FATAL ERROR: Please install a newer version of visual studio, VS2010 or above.")

#elif COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1600

#include <memory>
#include <unordered_map>
#include <unordered_set>

#else

#include <memory>
#include <hash_map>
#include <hash_set>
#endif

#ifdef _STLPORT_VERSION

#define HM_NAMESPACE std
using std::hash_map;
using std::hash_set;


#elif COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1700

using namespace std::tr1;
using std::tr1::shared_ptr;
#undef HM_NAMESPACE
#define HM_NAMESPACE std::tr1
#define hash_map unordered_map
#define TRHAX 1

#elif COMPILER == COMPILER_MICROSOFT && (_MSC_VER < 1600 || !_HAS_TR1)

using namespace std::tr1;
using std::tr1::shared_ptr;
#undef HM_NAMESPACE
#define HM_NAMESPACE std::tr1
#define hash_map unordered_map
#define ENABLE_SHITTY_STL_HACKS 1

// hacky stuff for vc++
#define snprintf _snprintf
#define vsnprintf _vsnprintf

#elif COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1600 && _HAS_TR1

using namespace std::tr1;
using std::tr1::shared_ptr;
#undef HM_NAMESPACE
#define HM_NAMESPACE std::tr1
#define hash_map unordered_map
#define TRHAX 1

#elif COMPILER == COMPILER_INTEL

#define HM_NAMESPACE std
using std::hash_map;
using std::hash_set;

#elif COMPILER == COMPILER_GNU && __GNUC__ >= 4

using namespace std::tr1;
using std::tr1::shared_ptr;
#undef HM_NAMESPACE
#define HM_NAMESPACE std::tr1
#define shared_ptr std::tr1::shared_ptr
#define hash_map unordered_map
#define TRHAX 1
namespace std
{
    namespace tr1
    {
        template<> struct hash<const long long unsigned int> : public std::unary_function<const long long unsigned int, std::size_t>
        {
            std::size_t operator()(const long long unsigned int val) const
            {
                return static_cast<std::size_t>(val);
            }
        };
        template<> struct hash<const unsigned int> : public std::unary_function<const unsigned int, std::size_t>
        {
            std::size_t operator()(const unsigned int val) const
            {
                return static_cast<std::size_t>(val);
            }
        };
    }
}

#elif COMPILER == COMPILER_GNU && __GNUC__ >= 3

#define HM_NAMESPACE __gnu_cxx
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;

namespace __gnu_cxx
{
    template<> struct hash<unsigned long long>
    {
        size_t operator()(const unsigned long long &__x) const { return (size_t)__x; }
    };
    template<typename T> struct hash<T *>
    {
        size_t operator()(T * const &__x) const { return (size_t)__x; }
    };

};
#else

#define HM_NAMESPACE std
using std::hash_map;

#endif

#if COMPILER == COMPILER_GNU && __GNUC__ >=4 && __GNUC_MINOR__ == 1 && __GNUC_PATCHLEVEL__ == 2
//GCC I HATE YOU!
namespace std
{
    namespace tr1
    {
        template<> struct hash<long long unsigned int> : public std::unary_function<long long unsigned int, std::size_t>
        {
            std::size_t operator()(const long long unsigned int val) const
            {
                return static_cast<std::size_t>(val);
            }
        };
    }
}

#endif

#if COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1700

#else

namespace std
{
    namespace tr1
    {
        template<typename T> struct hash<shared_ptr<T> >
        {
            size_t operator()(shared_ptr<T> const &__x) const { return (size_t)__x.get(); }
        };
    }
}

#endif

/* Use correct types for x64 platforms, too */
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long signed int LLSI;
typedef long long unsigned int LLUI;
typedef char const* const* PackedString;
#define UNORDERED_MAP std::tr1::unordered_map

#if COMPILER != COMPILER_GNU

typedef signed __int64 int64;
typedef signed __int32 int32;
typedef signed __int16 int16;
typedef signed __int8 int8;

typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8 uint8;

#else

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef uint32_t DWORD;

#endif

/*
Scripting system exports/imports
*/

#ifdef WIN32
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
#include "Threading/Threading.h"

#if defined(SHARED_LIB_BUILDER) || defined(_LOGON) || defined(_REALM) || defined(_GAME) // Game the lost just you?
#include "MersenneTwister.h"
#endif

#if COMPILER == COMPILER_MICROSOFT

#define I64FMT "%016I64X"
#define I64FMTD "%I64u"
#define SI64FMTD "%I64d"
#define snprintf _snprintf
#define atoll __atoi64

#else

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I64FMT "%016llX"
#define I64FMTD "%llu"
#define SI64FMTD "%lld"

#endif

//////////////////////////////
#define GUID_HIPART(x) (uint32)( ( uint64(x) >> 48 ) & 0x0000FFFF )

#define _GUID_ENPART_2(x) (uint32)0
#define _GUID_ENPART_3(x) (uint32)((uint64(x) >> 24) & 0x0000000000FFFFFF)
#define _GUID_LOPART_2(x) (uint32)(uint64(x)         & 0x00000000FFFFFFFF)
#define _GUID_LOPART_3(x) (uint32)(uint64(x)         & 0x0000000000FFFFFF)

inline bool IsGuidHaveEnPart(uint64 guid)
{
    switch (GUID_HIPART(guid))
    {
    case 0xF110:
    case 0xF120:
    case 0xF130:
    case 0xF140:
    case 0xF150:
    case 0x1FC0:
        return true;
    default:
        return false;
    }
}

#define GUID_ENPART(x) (IsGuidHaveEnPart(x) ? _GUID_ENPART_3(x) : _GUID_ENPART_2(x))
#define GUID_LOPART(x) (IsGuidHaveEnPart(x) ? _GUID_LOPART_3(x) : _GUID_LOPART_2(x))

#define MAKE_NEW_GUID(l, e, h)   uint64(uint64(l) | (uint64(IsGuidHaveEnPart(h) ? e : 0) << 24) | (uint64(h) << 48))

//////////////////////////////

#define atol(a) strtoul( a, NULL, 10)

#define STRINGIZE(a) #a

// fix buggy MSVC's for variable scoping to be reliable =S
#define for if(true) for

#if COMPILER == COMPILER_MICROSOFT

#pragma float_control(push)
#pragma float_control(precise, on)

#endif

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
static inline int long2int32(const double value)
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

#if COMPILER == COMPILER_MICROSOFT

#pragma float_control(pop)
#endif

#ifndef WIN32
#include <sys/timeb.h>

#endif

#ifndef WIN32
#define FALSE   0
#define TRUE    1
#endif

#ifndef WIN32
#define Sleep(ms) usleep(1000*ms)
#endif

#if defined(SHARED_LIB_BUILDER) || defined(_LOGON) || defined(_REALM) || defined(_GAME) // Game the lost just you?

struct ConditionalData
{
    ConditionalData(bool _EmoteOneShot = false, uint32 _EmoteID = 0, uint32 _SkinID = 0, uint32 _StandState = 0, uint32 _SpellToCast = 0, const char* _SayText = "")
    {
        EmoteOneShot = _EmoteOneShot;
        EmoteID = _EmoteID;
        SkinID = _SkinID;
        StandState = _StandState;
        SpellToCast = _SpellToCast;
        SayText = _SayText;
    }

    bool EmoteOneShot;
    uint32 EmoteID;
    uint32 SkinID;
    uint32 StandState;
    uint32 SpellToCast;
    std::string SayText;
};

#include "Util.h"

struct WayPoint
{
    uint32 id;
    float x, y, z;
    float orientation;
    uint32 waittime; //ms
    uint32 flags;
    uint16 count;

    ConditionalData* forwardInfo;
    ConditionalData* backwardInfo;
};

HEARTHSTONE_INLINE void reverse_array(uint8 * pointer, size_t count)
{
    size_t x;
    uint8 * temp = (uint8*)malloc(count);
    memcpy(temp, pointer, count);
    for(x = 0; x < count; ++x)
        pointer[x] = temp[count-x-1];
    free(temp);
}

typedef std::vector<WayPoint*> WayPointMap;

int32 GetTimePeriodFromString(const char * str);
std::string ConvertTimeStampToString(uint32 timestamp);
std::string ConvertTimeStampToDataTime(uint32 timestamp);

HEARTHSTONE_INLINE void HEARTHSTONE_TOLOWER(std::string& str)
{
    for(size_t i = 0; i < str.length(); ++i)
        str[i] = (char)tolower(str[i]);
};

HEARTHSTONE_INLINE void HEARTHSTONE_TOUPPER(std::string& str)
{
    for(size_t i = 0; i < str.length(); ++i)
        str[i] = (char)toupper(str[i]);
};

HEARTHSTONE_INLINE std::string HEARTHSTONE_TOLOWER_RETURN(std::string str)
{
    std::string newname = str;
    for(size_t i = 0; i < str.length(); ++i)
        newname[i] = (char)tolower(str[i]);

    return newname;
};

// returns true if the ip hits the mask, otherwise false
bool ParseCIDRBan(unsigned int IP, unsigned int Mask, unsigned int MaskBits);
unsigned int MakeIP(const char * str);

#include "hearthstone_log.h"
#include "ByteConverter.h"
#include "Console/CConsole.h"

#endif
