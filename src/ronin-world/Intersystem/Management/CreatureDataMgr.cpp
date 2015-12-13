
#include "StdAfx.h"

initialiseSingleton( CreatureDataManager );

CreatureDataManager::~CreatureDataManager()
{
    for(std::map<uint32, CreatureData*>::iterator itr = m_creatureData.begin(); itr != m_creatureData.end(); itr++)
    {
        free(itr->second->maleName);
        free(itr->second->femaleName);
        free(itr->second->subName);
        free(itr->second->iconName);
        delete itr->second;
    }
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
        ctrData->maleName = strdup(feilds[field_count++].GetString());
        ctrData->femaleName = strdup(feilds[field_count++].GetString());
        ctrData->subName = strdup(feilds[field_count++].GetString());
        ctrData->iconName = strdup(feilds[field_count++].GetString());
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

    if( result = WorldDatabase.Query( "SELECT * FROM ai_agents" ) )
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            CreatureData *data = GetCreatureData(entry);
            if( data == NULL )
            {
                sLog.Warning("AIAgent", "Agent skipped, NPC %u does not exist.", entry);
                continue;
            }

            uint32 spellID = fields[1].GetUInt32();
            SpellEntry* spe = dbcSpell.LookupEntry(spellID);
            if( spe == NULL )
            {
                if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                    WorldDatabase.Execute("DELETE FROM ai_agents where entry = '%u' AND spell = '%u'", entry, spellID);
                sLog.Warning("AIAgent", "Agent skipped, NPC %u tried to add non-existing Spell %u.", entry, spellID);
                continue;
            }

            SkillLineAbilityEntry * _spell = objmgr.GetSpellSkill(spellID);
            if(_spell)
            {
                SkillLineEntry * _skill = dbcSkillLine.LookupEntry(_spell->skilline);
                if(_skill)
                {
                    if(_skill->categoryId == SKILL_TYPE_PROFESSION)
                    {
                        sLog.outError("Creature %u has SpellID %u, which is a profession.", entry, spellID);
                        continue;
                    }
                }
            }

            AI_Spell *sp = new AI_Spell();
            const char* CharTargetType = fields[2].GetString();
            if(strcmp(CharTargetType, "ManaClass") == 0)
                sp->TargetType = TargetGen_ManaClass;
            else if(strcmp(CharTargetType, "RandomPlayerApplyAura") == 0)
                sp->TargetType = TargetGen_RandomPlayerApplyAura;
            else if(strcmp(CharTargetType, "RandomPlayerDestination") == 0)
                sp->TargetType = TargetGen_RandomPlayerDestination;
            else if(strcmp(CharTargetType, "RandomPlayer") == 0)
                sp->TargetType = TargetGen_RandomPlayer;
            else if(strcmp(CharTargetType, "RandomUnitApplyAura") == 0)
                sp->TargetType = TargetGen_RandomUnitApplyAura;
            else if(strcmp(CharTargetType, "RandomUnitDestination") == 0)
                sp->TargetType = TargetGen_RandomUnitDestination;
            else if(strcmp(CharTargetType, "RandomUnit") == 0)
                sp->TargetType = TargetGen_RandomUnit;
            else if(strcmp(CharTargetType, "Predefined") == 0)
                sp->TargetType = TargetGen_Predefined;
            else if(strcmp(CharTargetType, "SecondMostHated") == 0)
                sp->TargetType = TargetGen_SecondMostHated;
            else if(strcmp(CharTargetType, "SummonOwner") == 0)
                sp->TargetType = TargetGen_SummonOwner;
            else if(strcmp(CharTargetType, "Destination") == 0)
                sp->TargetType = TargetGen_Destination;
            else if(strcmp(CharTargetType, "Current") == 0)
                sp->TargetType = TargetGen_Current;
            else if(strcmp(CharTargetType, "Self") == 0)
                sp->TargetType = TargetGen_Self;

            const char* CharTargetFilter = fields[3].GetString();
            if(strcmp(CharTargetFilter, "ClosestFriendlyCorpse") == 0)
                sp->TargetFilter = TargetFilter_ClosestFriendlyCorpse;
            else if(strcmp(CharTargetFilter, "FriendlyCorpse") == 0)
                sp->TargetFilter = TargetFilter_FriendlyCorpse;
            else if(strcmp(CharTargetFilter, "WoundedFriendly") == 0)
                sp->TargetFilter = TargetFilter_WoundedFriendly;
            else if(strcmp(CharTargetFilter, "ClosestNotCurrent") == 0)
                sp->TargetFilter = TargetFilter_ClosestNotCurrent;
            else if(strcmp(CharTargetFilter, "ClosestFriendly") == 0)
                sp->TargetFilter = TargetFilter_ClosestFriendly;
            else if(strcmp(CharTargetFilter, "ManaClass") == 0)
                sp->TargetFilter = TargetFilter_ManaClass;
            else if(strcmp(CharTargetFilter, "IgnoreLineOfSight") == 0)
                sp->TargetFilter = TargetFilter_IgnoreLineOfSight;
            else if(strcmp(CharTargetFilter, "IgnoreSpecialStates") == 0)
                sp->TargetFilter = TargetFilter_IgnoreSpecialStates;
            else if(strcmp(CharTargetFilter, "InRangeOnly") == 0)
                sp->TargetFilter = TargetFilter_InRangeOnly;
            else if(strcmp(CharTargetFilter, "InMeleeRange") == 0)
                sp->TargetFilter = TargetFilter_InMeleeRange;
            else if(strcmp(CharTargetFilter, "Corpse") == 0)
                sp->TargetFilter = TargetFilter_Corpse;
            else if(strcmp(CharTargetFilter, "Aggroed") == 0)
                sp->TargetFilter = TargetFilter_Aggroed;
            else if(strcmp(CharTargetFilter, "SecondMostHated") == 0)
                sp->TargetFilter = TargetFilter_SecondMostHated;
            else if(strcmp(CharTargetFilter, "Wounded") == 0)
                sp->TargetFilter = TargetFilter_Wounded;
            else if(strcmp(CharTargetFilter, "NotCurrent") == 0)
                sp->TargetFilter = TargetFilter_NotCurrent;
            else if(strcmp(CharTargetFilter, "Friendly") == 0)
                sp->TargetFilter = TargetFilter_Friendly;
            else if(strcmp(CharTargetFilter, "Closest") == 0)
                sp->TargetFilter = TargetFilter_Closest;
            else if(strcmp(CharTargetFilter, "None") == 0)
                sp->TargetFilter = TargetFilter_None;

            sp->info = spe;
            sp->m_AI_Spell_disabled = false;
            sp->perctrigger = fields[4].GetFloat();
            sp->cooldown = fields[5].GetUInt32();
            sp->attackstoptimer = fields[6].GetUInt32();
            sp->casttime = fields[7].GetUInt32();
            sp->soundid = fields[8].GetUInt32();
            sp->ProcLimit = fields[9].GetUInt32();
            sp->ProcResetDelay = fields[10].GetUInt32();
            sp->mindist2cast = fields[11].GetUInt32();
            sp->maxdist2cast = fields[12].GetUInt32();
            sp->minHPPercentReq = fields[13].GetUInt32();
            sp->FloatData[0] = fields[14].GetFloat();
            sp->FloatData[1] = fields[15].GetFloat();
            sp->FloatData[2] = fields[16].GetFloat();
            sp->difficulty_mask = fields[17].GetInt32();
            data->spells.push_back(sp);
            count++;
        }while( result->NextRow() );
        delete result;
        result = NULL;
        sLog.Notice("Storage", "%u AI agents loaded from the database.", count);
    }
}

void CreatureDataManager::Reload()
{

}
