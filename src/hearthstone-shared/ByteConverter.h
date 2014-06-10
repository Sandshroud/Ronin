/***
 * Demonstrike Core
 */

#pragma once

/** ByteConverter reverse your byte order. This is use
    for cross platform where they have different endians.
 */

#include "Common.h"
#include <algorithm>

namespace ByteConverter
{
    template<size_t T>
    inline void convert(char *val)
    {
        std::swap(*val, *(val + T - 1));
        convert<T - 2>(val + 1);
    }

    template<> inline void convert<0>(char *) {}
    template<> inline void convert<1>(char *) {} // ignore central byte
    template<typename T> inline void apply(T *val)
    {
        convert<sizeof(T)>((char *)(val));
    }
}

#ifdef USING_BIG_ENDIAN

template<typename T> inline void EndianConvert(T& val) { ByteConverter::apply<T>(&val); }
template<typename T> inline void EndianConvertReverse(T&) { }

template<typename T> void EndianConvert(T*val) { ByteConverter::apply<T>(val); }
template<typename T> void EndianConvertReverse(T*) { };

#else

template<typename T> inline void EndianConvert(T&) { }
template<typename T> inline void EndianConvertReverse(T& val) { ByteConverter::apply<T>(&val); }

template<typename T> void EndianConvert(T*) { };
template<typename T> void EndianConvertReverse(T*val) { ByteConverter::apply<T>(val); };

#endif

inline void EndianConvert(uint8&) { }
inline void EndianConvert( int8&) { }
inline void EndianConvertReverse(uint8&) { }
inline void EndianConvertReverse( int8&) { }
