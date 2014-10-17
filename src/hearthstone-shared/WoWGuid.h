/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"
#include "ByteBuffer.h"

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

class SERVER_DECL WoWGuid
{
public:
    WoWGuid() { guidData.fullGuid = 0; }
    WoWGuid(uint32 guid) { guidData.fullGuid = guid; }
    WoWGuid(uint64 guid) { guidData.fullGuid = guid; }
    WoWGuid(uint32 high, uint32 low) { guidData.fullGuid = MAKE_NEW_GUID(low, 0, high); }
    WoWGuid(uint32 high, uint32 entry, uint32 low) { guidData.fullGuid = MAKE_NEW_GUID(low, entry, high); }
    ~WoWGuid() { guidData.fullGuid = 0; }

    const uint64 GetOldGuid() const { return guidData.fullGuid; }
    const uint8* GetNewGuid() const { return guidData.byteGuid; }
    const uint8 GetNewGuidLen() const { return BitCount8(GetNewGuidMask()); }
    const uint8 GetNewGuidMask() const
    {
        uint8 mask = 0;
        for(uint8 i = 0; i < 8; i++)
            if(guidData.byteGuid[i])
                mask |= (1<<i);
        return mask;
    }

    // WoWGuid == someval bool
    const bool operator ==(int someval) const { if(someval < 0) return false; return (guidData.fullGuid == uint(someval)); }
    const bool operator ==(uint8 someval) const { return (guidData.fullGuid == someval); }
    const bool operator ==(uint32 someval) const { return (guidData.fullGuid == someval); }
    const bool operator ==(uint64 someval) const { return (guidData.fullGuid == someval); }
    // WoWGuid != someval bool
    const bool operator !=(int someval) const { if(someval < 0) return false; return (guidData.fullGuid != uint(someval)); }
    const bool operator !=(uint8 someval) const { return (guidData.fullGuid != someval); }
    const bool operator !=(uint32 someval) const { return (guidData.fullGuid != someval); }
    const bool operator !=(uint64 someval) const { return (guidData.fullGuid != someval); }
    // WoWGuid & someval contains
    const uint64 operator &(int someval) const { if(someval < 0) someval = 0; return (guidData.fullGuid & uint(someval)); }
    const uint64 operator &(uint8 someval) const { return (guidData.fullGuid & someval); }
    const uint64 operator &(uint32 someval) const { return (guidData.fullGuid & someval); }
    const uint64 operator &(uint64 someval) const { return (guidData.fullGuid & someval); }
    // WoWGuid = someval
    template <class T> void operator =(T someval) { guidData.fullGuid = someval; }
    // WoWGuid byte checks
    uint8& operator[](uint32 index) { ASSERT(index < sizeof(uint64)); return guidData.byteGuid[index]; }
    uint8 const& operator[](uint32 index) const { ASSERT(index < sizeof(uint64)); return guidData.byteGuid[index]; }
    // WoWGuid check
    bool operator !() { return (guidData.fullGuid == 0); }
    operator bool() { return (guidData.fullGuid != 0); }
    operator uint64() { return guidData.fullGuid; }
    operator uint32() { return GUID_LOPART(guidData.fullGuid); }

    uint32 GetHighGuid() { return GUID_HIPART(guidData.fullGuid); }
    uint32 GetLowGUID() { return GUID_LOPART(guidData.fullGuid); }
    uint32 GetEntry() { return GUID_ENPART(guidData.fullGuid); }
private:
    union
    {
        uint64 fullGuid;
        uint8 byteGuid[8];
    } guidData;

public:
    void ReadAsFull(ByteBuffer *buff) { guidData.fullGuid = buff->read<uint64>(); }
    void WriteAsFull(ByteBuffer *buff) { buff->append<uint64>(guidData.fullGuid); }
    void WriteAsPacked(ByteBuffer *buff)
    {
        if(uint8 mask = GetNewGuidMask())
        {
            buff->append(mask);
            for(int i = 0; i < BitCount8(mask); i++)
                buff->append<uint8>(guidData.byteGuid[i]);
        } else buff->append<uint8>(0);
    }
    void ReadAsPacked(ByteBuffer *buff)
    {
        if(uint8 mask = buff->read<uint8>())
            for(int i = 0; i < BitCount8(mask); i++)
                guidData.byteGuid[i] = buff->read<uint8>();
    }

    void WriteAsSequence(ByteBuffer *buff, ...)
    {

    }

    void ReadAsSequence(ByteBuffer *buff, ...)
    {

    }
};
