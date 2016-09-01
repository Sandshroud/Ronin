/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"

#define BitCount1(x) ((x) & 1)
#define BitCount2(x) ( BitCount1(x) + BitCount1((x)>>1) )
#define BitCount4(x) ( BitCount2(x) + BitCount2((x)>>2) )
#define BitCount8(x) ( BitCount4(x) + BitCount4((x)>>4) )

#define GUID_HIPART(x) uint32((uint64(x) >> 48) == 0x1FF7 ? uint32(0x1FF7) : uint32(uint64(x) >> 52))
#define GUID_ENPART(x) uint32((uint64(x) & 0x0007FFFF00000000)>>32)
#define GUID_LOPART(x) uint32(uint64(x) & 0x00000000FFFFFFFF)
#define MAKE_NEW_GUID(l, e, h)   uint64(uint64(l) | (uint64(e) << 32) | (uint64(h) << (h == 0x1FF7 ? 48 : 52)))

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

    uint8 len() { return sizeof(uint64); }
    uint8 pLen() { uint8 len = 0; for(uint8 i = 0; i < 8; i++) { if(blocks[i]) len++; } return len; }
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
