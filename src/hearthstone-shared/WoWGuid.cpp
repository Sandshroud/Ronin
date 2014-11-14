
#include "WoWGuid.h"

template <class T> WoWGuid& WoWGuid::operator =(T input)
{
    for(uint8 i = 0; i < sizeof(T); i++)
        blocks[i] = ((uint8*)&input)[i];
    return *this;
}

void WoWGuid::Clean()
{
    blocks[0]=blocks[1]=blocks[2]=blocks[3]=blocks[4]=blocks[5]=blocks[6]=blocks[7] = 0;
}

uint8 WoWGuid::GenMask()
{
    return ((blocks[0] ? 0x01 : 0) | (blocks[1] ? 0x02 : 0) | (blocks[2] ? 0x04 : 0) | (blocks[3] ? 0x08 : 0) | (blocks[4] ? 0x10 : 0) | (blocks[5] ? 0x20 : 0) | (blocks[6] ? 0x40 : 0) | (blocks[7] ? 0x80 : 0));
}
