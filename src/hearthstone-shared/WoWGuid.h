/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"

#define BitCount1(x) ((x) & 1)
#define BitCount2(x) ( BitCount1(x) + BitCount1((x)>>1) )
#define BitCount4(x) ( BitCount2(x) + BitCount2((x)>>2) )
#define BitCount8(x) ( BitCount4(x) + BitCount4((x)>>4) )

class SERVER_DECL WoWGuid
{
public:

    WoWGuid()
    {
        Clear();
    }

    WoWGuid(uint64 guid)
    {
        Clear();
        Init(uint64(guid));
    }

    WoWGuid(uint32 guid)
    {
        Clear();
        Init(uint64(guid));
    }

    WoWGuid(uint8 mask)
    {
        Clear();
        Init(uint8(mask));
    }

    WoWGuid(uint8 mask, uint8 *fields)
    {
        Clear();
        Init(uint8(mask), fields);
    }

    ~WoWGuid()
    {
        Clear();
    }

    void Clear()
    {
        oldguid = 0;
        guidmask = 0;

        *((uint32*)guidfields) = 0;
        *((uint32*)&guidfields[4]) = 0;
        compiled = false;
        fieldcount = 0;
    }

    void Init(uint64 guid)
    {
        Clear();

        oldguid = guid;

        _CompileByOld();
    }

    void Init(uint8 mask)
    {
        Clear();

        guidmask = mask;

        if(!guidmask)
            _CompileByNew();
    }

    void Init(uint8 mask, uint8 *fields)
    {
        Clear();

        guidmask = mask;

        if (!BitCount8(guidmask))
            return;

        for(int i = 0; i < BitCount8(guidmask); i++)
            guidfields[i] = (fields[i]);

        fieldcount = BitCount8(guidmask);

        _CompileByNew();
    }

    const uint64 GetOldGuid() const { return oldguid; }
    const uint8* GetNewGuid() const { return guidfields; }
    const uint8 GetNewGuidLen() const { return BitCount8(guidmask); }
    const uint8 GetNewGuidMask() const { return guidmask; }
    const bool operator !() const { return (!oldguid); }
    // WoWGuid == someval bool
    const bool operator ==(int someval) const { if(someval < 0) return false; return (oldguid == uint(someval)); }
    const bool operator ==(uint8 someval) const { return (oldguid == someval); }
    const bool operator ==(uint32 someval) const { return (oldguid == someval); }
    const bool operator ==(uint64 someval) const { return (oldguid == someval); }
    // WoWGuid != someval bool
    const bool operator !=(int someval) const { if(someval < 0) return false; return (oldguid != uint(someval)); }
    const bool operator !=(uint8 someval) const { return (oldguid != someval); }
    const bool operator !=(uint32 someval) const { return (oldguid != someval); }
    const bool operator !=(uint64 someval) const { return (oldguid != someval); }
    // WoWGuid & someval contains
    const uint64 operator &(int someval) const { if(someval < 0) someval = 0; return (oldguid & uint(someval)); }
    const uint64 operator &(uint8 someval) const { return (oldguid & someval); }
    const uint64 operator &(uint32 someval) const { return (oldguid & someval); }
    const uint64 operator &(uint64 someval) const { return (oldguid & someval); }
    // WoWGuid = someval
    void operator =(int someval) { if(someval < 0) return; Clear(); Init(uint64(someval)); }
    void operator =(uint8 someval) { Clear(); Init(uint64(someval)); }
    void operator =(uint32 someval) { Clear(); Init(uint64(someval)); }
    void operator =(uint64 someval) { Clear(); Init(uint64(someval)); }
    // WoWGuid check
    operator bool() { return (oldguid > 0); }
    operator uint64() { return oldguid; }

    void AppendField(uint8 field)
    {
        ASSERT(!compiled);
        ASSERT(fieldcount < BitCount8(guidmask));

        guidfields[fieldcount++] = field;

        if (fieldcount == BitCount8(guidmask))
            _CompileByNew();
    }

private:

    uint64 oldguid;
    uint8 guidmask;

    uint8 guidfields[8];

    uint8 fieldcount;
    bool compiled;

    void _CompileByOld()
    {
        ASSERT(!compiled);

        fieldcount = 0;

        for(uint32 x = 0; x < 8; ++x)
        {
            uint8 p =((uint8*)&oldguid)[x];
            if(p)
            {
                guidfields[fieldcount++] = p;
                guidmask |= 1 << x;
            }
        }
        compiled = true;
    }
    void _CompileByNew()
    {
        ASSERT(!compiled || fieldcount == BitCount8(guidmask));

        int j = 0;

        if (guidmask & 0x01) //1
        {
            oldguid |= ((uint64)guidfields[j]);
            ++j;
        }
        if (guidmask & 0x02) //2
        {
            oldguid |= ((uint64)guidfields[j] << 8);
            ++j;
        }
        if (guidmask & 0x04)//4
        {
            oldguid |= ((uint64)guidfields[j] << 16);
            ++j;
        }
        if (guidmask & 0x08) //8
        {
            oldguid |= ((uint64)guidfields[j] << 24);
            ++j;
        }
        if (guidmask & 0x10)//16
        {
            oldguid |= ((uint64)guidfields[j] << 32);
            ++j;
        }
        if (guidmask & 0x20)//32
        {
            oldguid |= ((uint64)guidfields[j] << 40);
            ++j;
        }
        if (guidmask & 0x40)//64
        {
            oldguid |= ((uint64)guidfields[j] << 48);
            ++j;
        }
        if (guidmask & 0x80) //128
        {
            oldguid |= ((uint64)guidfields[j] << 56);
            ++j;
        }
        compiled = true;
    }
};
