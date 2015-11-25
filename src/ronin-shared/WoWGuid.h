/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"

#define BitCount1(x) ((x) & 1)
#define BitCount2(x) ( BitCount1(x) + BitCount1((x)>>1) )
#define BitCount4(x) ( BitCount2(x) + BitCount2((x)>>2) )
#define BitCount8(x) ( BitCount4(x) + BitCount4((x)>>4) )

inline bool IsGuidHaveEnPart(uint32 highguid);
#define _GUID_ENPART_2(x) (uint32)0
#define _GUID_ENPART_3(x) (uint32)((uint64(x) >> 24) & 0x0000000000FFFFFF)
#define _GUID_LOPART_2(x) (uint32)(uint64(x)         & 0x00000000FFFFFFFF)
#define _GUID_LOPART_3(x) (uint32)(uint64(x)         & 0x0000000000FFFFFF)

#define GUID_HIPART(x) (uint32)( ( uint64(x) >> 48 ) & 0x0000FFFF )
#define GUID_ENPART(x) (IsGuidHaveEnPart(GUID_HIPART(x)) ? _GUID_ENPART_3(x) : _GUID_ENPART_2(x))
#define GUID_LOPART(x) (IsGuidHaveEnPart(GUID_HIPART(x)) ? _GUID_LOPART_3(x) : _GUID_LOPART_2(x))
#define MAKE_NEW_GUID(l, e, h)   uint64(uint64(l) | (uint64(IsGuidHaveEnPart(h) ? e : 0) << 24) | (uint64(h) << 48))

inline bool IsGuidHaveEnPart(uint32 highguid)
{
    switch (highguid)
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
    friend class WoWGuid;
    friend class ByteBuffer;
private: 
    WGuidPacked(WoWGuid *guid) : m_guid(guid) {}
    WoWGuid *m_guid;
};

class SERVER_DECL WoWGuid
{
protected:
    uint8 blocks[8];

public:
    WoWGuid() { Clean(); }
    WoWGuid(uint64 val) { ((uint64*)blocks)[0] = val; }

    operator uint64() const { return ((uint64*)blocks)[0]; }
    operator double() const { return (double)this->operator uint64(); }

    void Clean() { ((uint64*)blocks)[0] = uint64(0); }
    void Set(uint8 i, uint8 val) { blocks[i] = val; };
    uint8& operator[](int index) { ASSERT(index < sizeof(uint64)); return blocks[index]; }
    uint8 const& operator[](int index) const { ASSERT(index < sizeof(uint64)); return blocks[index]; }
    uint64 operator |( const WoWGuid& val ) const { return ((uint64)*this | (uint64)val); }
    uint64 operator |( const uint64 val ) const { return ((uint64)*this | val); }
    uint64 operator >>( const uint64 val ) const { return ((uint64)*this >> val); }
    uint64 operator <<( const uint64 val ) const { return ((uint64)*this << val); }
    WoWGuid& operator |=( const WoWGuid& val ) { *this = *this | val; return *this; }
    WoWGuid& operator |=( const uint64 val ) { *this = *this | val; return *this; }
    WoWGuid& operator >>=( const uint64 val ) { *this = *this >> val; return *this; }
    WoWGuid& operator <<=( const uint64 val ) { *this = *this << val; return *this; }
    WoWGuid operator =( const uint64 val ) { ((uint64*)blocks)[0] = val; return *this; }
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
    bool count() { return !empty(); }
    WGuidPacked *asPacked() { return new WGuidPacked(this); }
    uint64 raw() { return this->operator uint64(); }
    uint32 getLow() { return GUID_LOPART(this->operator uint64()); }
    uint32 getEntry() { return GUID_ENPART(this->operator uint64()); }
    uint32 getHigh() { return GUID_HIPART(this->operator uint64()); }
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
