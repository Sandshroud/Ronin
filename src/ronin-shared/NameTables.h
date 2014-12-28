/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"

struct NameTableEntry
{
    uint32 id;
    const char *name;
};

static inline const char* LookupName(uint32 id, NameTableEntry *table)
{
    for(uint32 i = 0; table[i].name != 0; i++)
    {
        if (table[i].id == id)
            return table[i].name;
    }

    return "UNKNOWN";
}
