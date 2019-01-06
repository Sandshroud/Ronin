
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

bool checkFodderStatus(uint32 entry);

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
        if(dbcFactionTemplate.LookupEntry(ctrData->faction) == NULL)
        {
            sLog.Error("CreatureData", "Invalid faction entry %u in creature data %u", ctrData->faction, ctrData->entry);
            delete ctrData;
            continue;
        }

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
        if(checkFodderStatus(ctrData->entry))
            ctrData->extraFlags |= CREATURE_DATA_EX_FLAG_FODDER_SPAWN;
        if(RONIN_UTIL::FindXinYString("Training Dummy", ctrData->GetFullName()))
            ctrData->extraFlags |= CREATURE_DATA_EX_FLAG_TRAINING_DUMMY;
        if(RONIN_UTIL::FindXinYString("World", ctrData->GetFullName()) && RONIN_UTIL::FindXinYString("Trigger", ctrData->GetFullName()))
            ctrData->extraFlags |= CREATURE_DATA_EX_FLAG_WORLD_TRIGGER;

        m_creatureData.insert(std::make_pair(ctrData->entry, ctrData));
    }while(result->NextRow());
}

void CreatureDataManager::LoadCreatureSpells()
{
    uint32 q_size = 0;
    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM creature_spells_combat"))
    {
        q_size = result->GetRowCount();
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
        sLog.Notice("ObjectMgr", "%u creature combat spells loaded.", q_size);
    }

    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM creature_spells_support"))
    {
        q_size = result->GetRowCount();
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
        sLog.Notice("ObjectMgr", "%u creature support spells loaded.", q_size);
    }
}

void CreatureDataManager::Reload()
{

}

bool CreatureDataManager::IsSpiritHealer(CreatureData *data)
{
    if (data->NPCFLags & UNIT_NPC_FLAG_SPIRITHEALER)
        return true;
    switch (data->entry)
    {
    case 6491:
    case 32537:
        return true;
    }

    return false;
}

void CreatureDataManager::CalculateMinMaxDamage(CreatureData *data, float &minDamage, float &maxDamage, uint32 level, float apBonus)
{
    maxDamage = std::max(2.f, ceil(apBonus * (1.f + (((float)sStatSystem.GetXPackModifierForLevel(level, (data->rank > 0) ? 3 : 0))*0.1f)) * data->damageMod));

    minDamage = std::max<float>(1.f, floor(maxDamage * ((6.5f + ((float)data->rank+1) + data->damageRangeMod)/10.f)));
}

// Helps with culling down inrange processing
bool checkFodderStatus(uint32 entry)
{
    switch(entry)
    {
        // Spawned for an event, meant to be destroyed by cannon fire
    case 46711: // Stonevault Ruffian
    case 46712: // Stonevault Goon
        // Quest: The Final Voyage of the Brashtide
    case 43717: // Brashtide Crewman
        // Quest: The Endless Flow
    case 44329: // Stickbone Berserker
        // Quest: Assault on Dreadmaul Rock
    case 48414: // Blackrock Invader
    case 48432: // Blackrock Invader
        // Quest: Rundown
    case 40174: // Fleeing Zin'jatar Fathom-Stalker
        // Quest: First Degree Mortar
    case 6196: // Spitelash Myrmidon
        // Quest: Rider on the Storm
    case 35506: // Nijel's Point Defender
        return true;
        // Disabled spawns that exist due to required range data
    case 32782: // Noblegarden Bunny Waypoint
    case 40708: // Falling Boulder Animation
    case 45485: // Necropolis Flowers
    case 45365: // Drakgor Fire Bunny
    case 46849: // Firebeard's Patrol Fire Bunny
        return true;
        // True fodder to disable non player processing
    case 40280: // Swarming Serpent
    case 41249: // Azsh'ir Soldier
    case 41916: // Underlight Nibbler
    case 41357: // Rockpool Murloc
    case 42057: // Kvaldir Bonesnapper
    case 44331: // Andorhal Deathguard
    case 44332: // Andorhal Defender
    case 46384: // Crazed Gnome
    case 47203: // Creeper Egg
    case 48629: // Schnottz Infantryman
    case 48713: // Schnottz Infantryman
    case 49124: // Highbank Marksman
    case 50540: // Obsidian Hatchling
        return true;
    default: return false;
    }
}
