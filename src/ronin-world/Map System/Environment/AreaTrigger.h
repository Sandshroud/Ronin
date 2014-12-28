/***
 * Demonstrike Core
 */

#pragma once

enum AreaTriggerType
{
    ATTYPE_NULL             = 0,
    ATTYPE_INSTANCE         = 1,
    ATTYPE_QUESTTRIGGER     = 2,
    ATTYPE_INN              = 3,
    ATTYPE_TELEPORT         = 4,
    ATTYPE_SPELL            = 5,
    ATTYPE_BATTLEGROUND     = 6,
};

#pragma pack(PRAGMA_PACK)

typedef struct AreaTrigger
{
    uint32 AreaTriggerID;
    uint8 Type;
    int32 RequiredTeam;
    uint32 Mapid;
    char * Name;
    float x; float y;
    float z; float o;
    uint32 required_honor_rank;
    uint32 required_level;
}AreaTrigger;

#pragma pack(PRAGMA_POP)
