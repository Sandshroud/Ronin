/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"

#define BitCount1(x) ((x) & 1)
#define BitCount2(x) ( BitCount1(x) + BitCount1((x)>>1) )
#define BitCount4(x) ( BitCount2(x) + BitCount2((x)>>2) )
#define BitCount8(x) ( BitCount4(x) + BitCount4((x)>>4) )

inline bool IsGuidHaveEnPart(uint64 guid);
#define _GUID_ENPART_2(x) (uint32)0
#define _GUID_ENPART_3(x) (uint32)((uint64(x) >> 24) & 0x0000000000FFFFFF)
#define _GUID_LOPART_2(x) (uint32)(uint64(x)         & 0x00000000FFFFFFFF)
#define _GUID_LOPART_3(x) (uint32)(uint64(x)         & 0x0000000000FFFFFF)

#define GUID_HIPART(x) (uint32)( ( uint64(x) >> 48 ) & 0x0000FFFF )
#define GUID_ENPART(x) (IsGuidHaveEnPart(x) ? _GUID_ENPART_3(x) : _GUID_ENPART_2(x))
#define GUID_LOPART(x) (IsGuidHaveEnPart(x) ? _GUID_LOPART_3(x) : _GUID_LOPART_2(x))
#define MAKE_NEW_GUID(l, e, h)   uint64(uint64(l) | (uint64(IsGuidHaveEnPart(h) ? e : 0) << 24) | (uint64(h) << 48))

inline bool IsGuidHaveEnPart(uint64 guid)
{
    switch (GUID_HIPART(guid))
    {
    case 0x1000:
    case 0xF110:
    case 0xF120:
    case 0xF130:
    case 0xF140:
    case 0xF150:
    case 0x1FC0:
        return true;
    } return false;
}

class WoWGuid;

// Byte Buffer implementation classes
struct WGuidPacked
{
    friend class ByteBuffer;
    WGuidPacked(WoWGuid *guid) : m_guid(guid) {}
private: WoWGuid *m_guid;
};

class SERVER_DECL WoWGuid
{
protected:
    uint8 blocks[8];

public:
    operator uint64() const { return ((uint64*)&blocks[0])[0]; }
    operator double() const { return (double)this->operator uint64(); }

    template <class T> WoWGuid& operator =(T input);
    void Clean();

    uint8 GenMask();
    uint8& operator[](int index) { ASSERT(index < sizeof(uint64)); return blocks[index]; }
    uint8 const& operator[](int index) const { ASSERT(index < sizeof(uint64)); return blocks[index]; }
    uint64 operator +( const WoWGuid& val ) const { return ((uint64)*this + (uint64)val); }
    uint64 operator -( const WoWGuid& val ) const { return ((uint64)*this - (uint64)val); }
    uint64 operator *( const WoWGuid& val ) const { return ((uint64)*this * (uint64)val); }
    uint64 operator /( const WoWGuid& val ) const { return ((uint64)*this / (uint64)val); }
    uint64 operator |( const WoWGuid& val ) const { return ((uint64)*this | (uint64)val); }
    uint64 operator +( const uint64 val ) const { return ((uint64)*this + val); }
    uint64 operator -( const uint64 val ) const { return ((uint64)*this - val); }
    uint64 operator *( const uint64 val ) const { return ((uint64)*this * val); }
    uint64 operator /( const uint64 val ) const { return ((uint64)*this / val); }
    uint64 operator |( const uint64 val ) const { return ((uint64)*this | val); }
    uint64 operator >>( const uint64 val ) const { return ((uint64)*this >> val); }
    uint64 operator <<( const uint64 val ) const { return ((uint64)*this << val); }
    WoWGuid& operator +=( const WoWGuid& val ) { *this = *this + val; return *this; }
    WoWGuid& operator -=( const WoWGuid& val ) { *this = *this - val; return *this; }
    WoWGuid& operator *=( const WoWGuid& val ) { *this = *this * val; return *this; }
    WoWGuid& operator /=( const WoWGuid& val ) { *this = *this / val; return *this; }
    WoWGuid& operator |=( const WoWGuid& val ) { *this = *this | val; return *this; }
    WoWGuid& operator +=( const uint64 val ) { *this = *this + val; return *this; }
    WoWGuid& operator -=( const uint64 val ) { *this = *this - val; return *this; }
    WoWGuid& operator *=( const uint64 val ) { *this = *this * val; return *this; }
    WoWGuid& operator /=( const uint64 val ) { *this = *this / val; return *this; }
    WoWGuid& operator |=( const uint64 val ) { *this = *this | val; return *this; }
    WoWGuid& operator >>=( const uint64 val ) { *this = *this >> val; return *this; }
    WoWGuid& operator <<=( const uint64 val ) { *this = *this << val; return *this; }
    operator bool() const { return (uint64)*this != 0; }
    bool operator !() const { return !((uint64)*this); }
    bool operator ==( const WoWGuid& val ) const { return (uint64)*this == (uint64)val; }
    bool operator !=( const WoWGuid& val ) const { return (uint64)*this != (uint64)val; }
    bool operator >=( const WoWGuid& val ) const { return (uint64)*this >= (uint64)val; }
    bool operator <=( const WoWGuid& val ) const { return (uint64)*this <= (uint64)val; }
    bool operator >( const WoWGuid& val ) const { return (uint64)*this > (uint64)val; }
    bool operator <( const WoWGuid& val ) const { return (uint64)*this < (uint64)val; }
    bool operator ==( const uint64 val ) const { return (uint64)*this == val; }
    bool operator !=( const uint64 val ) const { return (uint64)*this != val; }
    bool operator >=( const uint64 val ) const { return (uint64)*this >= val; }
    bool operator <=( const uint64 val ) const { return (uint64)*this <= val; }
    bool operator >( const uint64 val ) const { return ((uint64)*this) > val; }
    bool operator <( const uint64 val ) const { return (uint64)*this < val; }

    bool empty() { return operator !(); };
    WGuidPacked *asPacked() { return new WGuidPacked(this); }
    uint32 getLow() { return GUID_LOPART(this->operator uint64()); }
    uint32 getEntry() { return GUID_ENPART(this->operator uint64()); }
    uint32 getHigh() { return GUID_HIPART(this->operator uint64()); }
    uint64 raw() { return this->operator uint64(); }

    // THESE ARE NOT CONSTRUCTORS
    static WoWGuid From64(uint64 guid) { WoWGuid wGuid; wGuid = guid; return wGuid; }
    // THESE ARE NOT CONSTRUCTORS
    static WoWGuid From32(uint32 high, uint32 entry, uint32 low) { WoWGuid wGuid; wGuid = MAKE_NEW_GUID(low, entry, high); return wGuid; }
};

namespace std
{
    template<> struct hash<WoWGuid>
    {
    public:
        size_t operator()(WoWGuid const& key) const
        {
            return hash<uint64>()(key);
        }
    };
}
