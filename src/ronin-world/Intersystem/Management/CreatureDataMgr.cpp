
#include "StdAfx.h"

initialiseSingleton( CreatureDataManager );

CreatureDataManager::~CreatureDataManager()
{
    for(std::map<uint32, CreatureData*>::iterator itr = m_creatureData.begin(); itr != m_creatureData.end(); itr++)
        delete itr->second;
    m_creatureData.clear();
}

static const char * tableColumns = "maleName, femaleName, subname, iconName, flags, flags2, type, family, rank, killCredit, killCreditSpellId, maleDisplayId, femaleDisplayId, "
    "maleDisplayId2, femaleDisplayId2, healthMod, powerMod, leader, questItem1, questItem2, questItem3, questItem4, questItem5, questItem6, movementId, expansionId, "
    "class, minlevel, maxlevel, faction, powertype, scale, lootType, npcflags, attacktime, attacktype, mindamage, maxdamage, "
    "rangedattacktime, rangedmindamage, rangedmaxdamage, item1, item2, item3, respawntime, armor, holyresist, fireresist, natureresist, frostresist, shadowresist, arcaneresist, "
    "combatReach, boundingRadius, money, invisibilityType, walkSpeed, runSpeed, flySpeed, auraimmune_flag, vehicle_entry, spellclickid, canmove, battlemastertype, auras";

void CreatureDataManager::LoadFromDB()
{
    QueryResult *result = WorldDatabase.Query("SELECT creature_names.entry, %s FROM creature_names INNER JOIN creature_proto ON creature_names.entry = creature_proto.entry", tableColumns);
    if(result == NULL)
        return;

    do
    {
        uint32 field_count = 0;
        Field *feilds = result->Fetch();

        CreatureData *ctrData = new CreatureData();
        ctrData->entry = feilds[field_count++].GetUInt32();
        ctrData->maleName = feilds[field_count++].GetString();
        ctrData->femaleName = feilds[field_count++].GetString();
        ctrData->subName = feilds[field_count++].GetString();
        ctrData->iconName = feilds[field_count++].GetString();
        ctrData->flags = feilds[field_count++].GetUInt32();
        ctrData->flags2 = feilds[field_count++].GetUInt32();
        ctrData->type = feilds[field_count++].GetUInt32();
        ctrData->family = feilds[field_count++].GetUInt32();
        ctrData->rank = feilds[field_count++].GetUInt32();
        ctrData->killCredit[0] = feilds[field_count++].GetUInt32();
        ctrData->killCredit[1] = feilds[field_count++].GetUInt32();
        for(uint8 i = 0; i < 4; i++)
            ctrData->displayInfo[i] = feilds[field_count++].GetUInt32();
        ctrData->healthMod = feilds[field_count++].GetFloat();
        ctrData->powerMod = feilds[field_count++].GetFloat();
        ctrData->leader = feilds[field_count++].GetUInt8();
        for(uint8 i = 0; i < 6; i++)
            ctrData->questItems[i] = feilds[field_count++].GetUInt32();
        ctrData->dbcMovementId = feilds[field_count++].GetUInt32();
        ctrData->expansionId = feilds[field_count++].GetUInt32();
        ctrData->Class = feilds[field_count++].GetUInt8();
        ctrData->minLevel = feilds[field_count++].GetUInt32();
        ctrData->maxLevel = feilds[field_count++].GetUInt32();
        ctrData->faction = feilds[field_count++].GetUInt32();
        ctrData->powerType = feilds[field_count++].GetUInt8();
        ctrData->scale = feilds[field_count++].GetFloat();
        ctrData->lootType = feilds[field_count++].GetUInt32();
        ctrData->NPCFLags = feilds[field_count++].GetUInt32();
        ctrData->attackTime = feilds[field_count++].GetUInt32();
        ctrData->attackType = std::min<uint32>(SCHOOL_ARCANE, feilds[field_count++].GetUInt32());
        ctrData->minDamage = feilds[field_count++].GetFloat();
        ctrData->maxDamage = feilds[field_count++].GetFloat();
        ctrData->rangedAttackTime = feilds[field_count++].GetUInt32();
        ctrData->rangedMinDamage = feilds[field_count++].GetFloat();
        ctrData->rangedMaxDamage = feilds[field_count++].GetFloat();
        ctrData->inventoryItem[0] = feilds[field_count++].GetUInt32();
        ctrData->inventoryItem[1] = feilds[field_count++].GetUInt32();
        ctrData->inventoryItem[2] = feilds[field_count++].GetUInt32();
        ctrData->respawnTime = feilds[field_count++].GetUInt32();
        for(uint8 i = 0; i < 7; i++)
            ctrData->resistances[i] = feilds[field_count++].GetUInt32();
        ctrData->combatReach = feilds[field_count++].GetFloat();
        ctrData->boundingRadius = feilds[field_count++].GetFloat();
        ctrData->money = feilds[field_count++].GetInt32();
        ctrData->invisType = feilds[field_count++].GetUInt32();
        ctrData->walkSpeed = feilds[field_count++].GetFloat();
        ctrData->runSpeed = feilds[field_count++].GetFloat();
        ctrData->flySpeed = feilds[field_count++].GetFloat();
        ctrData->auraMechanicImmunity = feilds[field_count++].GetUInt32();
        ctrData->vehicleEntry = feilds[field_count++].GetUInt32();
        ctrData->spellClickid = feilds[field_count++].GetUInt32();
        ctrData->movementMask = feilds[field_count++].GetUInt32();
        ctrData->battleMasterType = feilds[field_count++].GetUInt32();
        const char* aurastring = feilds[field_count++].GetString();

        if(aurastring && strlen(aurastring))
        {
            std::vector<std::string> aurs = RONIN_UTIL::StrSplit(std::string(aurastring), " ");
            for(std::vector<std::string>::iterator it = aurs.begin(); it != aurs.end(); it++)
                if(uint32 id = atol((*it).c_str()))
                    ctrData->Auras.insert( id );
        }

        m_creatureData.insert(std::make_pair(ctrData->entry, ctrData));
    }while(result->NextRow());
}

void CreatureDataManager::Reload()
{

}
