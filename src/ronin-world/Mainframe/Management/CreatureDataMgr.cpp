
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
    "class, minlevel, maxlevel, faction, powertype, scale, lootType, npcflags, attacktype, damagemod, damagerangemod, meleeattacktime, "
    "rangedattacktime, item1, item2, item3, respawntime, armor, holyresist, fireresist, natureresist, frostresist, shadowresist, arcaneresist, "
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
        // Limit attack type to arcane or lower
        ctrData->attackType = std::min<uint32>(SCHOOL_ARCANE, feilds[field_count++].GetUInt32());
        ctrData->damageMod = feilds[field_count++].GetFloat();
        // Limit or damage range modifier to prevent bad code from crashing server
        ctrData->damageRangeMod = std::max<float>(-5.f, std::min<float>(2.f, feilds[field_count++].GetFloat()));
        ctrData->meleeAttackTime = feilds[field_count++].GetUInt32();
        ctrData->rangedAttackTime = feilds[field_count++].GetUInt32();
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

        ctrData->extraFlags = CREATURE_DATA_EX_FLAGS_NONE;
        if(RONIN_UTIL::FindXinYString("Training Dummy", ctrData->GetFullName()))
            ctrData->extraFlags = CREATURE_DATA_EX_FLAG_TRAINING_DUMMY;
        if(RONIN_UTIL::FindXinYString("World", ctrData->GetFullName()) && RONIN_UTIL::FindXinYString("Trigger", ctrData->GetFullName()))
            ctrData->extraFlags = CREATURE_DATA_EX_FLAG_WORLD_TRIGGER;

        m_creatureData.insert(std::make_pair(ctrData->entry, ctrData));
    }while(result->NextRow());
}

void CreatureDataManager::LoadCreatureSpells()
{
    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM creature_spells_combat"))
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            if(m_creatureData.find(entry) == m_creatureData.end())
                continue;
            CreatureData *data = m_creatureData.at(entry);
            for(uint8 i = 1; i < 11; i++)
                if(uint32 spellId = fields[i].GetUInt32())
                    data->combatSpells.push_back(spellId);
        }while(result->NextRow());
        delete result;
    }

    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM creature_spells_support"))
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            if(m_creatureData.find(entry) == m_creatureData.end())
                continue;
            CreatureData *data = m_creatureData.at(entry);
            if(uint32 spellId = fields[1].GetUInt32())
                data->supportSpells.push_back(spellId);
            if(uint32 spellId = fields[2].GetUInt32())
                data->supportSpells.push_back(spellId);
            if(uint32 spellId = fields[3].GetUInt32())
                data->supportSpells.push_back(spellId);
        }while(result->NextRow());
        delete result;
    }
}

void CreatureDataManager::Reload()
{

}
