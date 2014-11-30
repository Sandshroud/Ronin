
#include "StdAfx.h"

initialiseSingleton( CreatureDataManager );

CreatureDataManager::~CreatureDataManager()
{
    for(std::map<uint32, CreatureData*>::iterator itr = m_creatureData.begin(); itr != m_creatureData.end(); itr++)
    {
        free(itr->second->Name);
        free(itr->second->SubName);
        free(itr->second->InfoStr);
        delete itr->second;
    }
    m_creatureData.clear();
}

static const char * tableColumns = "name, subname, info_str, flags1, type, loot_skill_type, family, rank, unk4, spelldataid, "
    "male_displayid, female_displayid, male_displayid2, female_displayid2, unknown_float1, unknown_float2, civilian, leader, minlevel, maxlevel, "
    "faction, minhealth, maxhealth, powertype, minpower, maxpower, scale, npcflags, attacktime, attacktype, mindamage, maxdamage, "
    "rangedattacktime, rangedmindamage, rangedmaxdamage, item1, item2, item3, respawntime, armor, holyresist, fireresist, natureresist, "
    "frostresist, shadowresist, arcaneresist, combat_reach, bounding_radius, auras, boss, money, invisibility_type, walk_speed, run_speed, fly_speed, "
    "extra_a9_flags, auraimmune_flag, vehicle_entry, battlemastertype, spellclickid, canmove";
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
        ctrData->Entry = feilds[field_count++].GetUInt32();
        // Parse name data
        ctrData->Name = strdup(feilds[field_count++].GetString());
        ctrData->SubName = strdup(feilds[field_count++].GetString());
        ctrData->InfoStr = strdup(feilds[field_count++].GetString());
        ctrData->Flags = feilds[field_count++].GetUInt32();
        ctrData->Type = feilds[field_count++].GetUInt32();
        ctrData->TypeFlags = feilds[field_count++].GetUInt32();
        ctrData->Family = feilds[field_count++].GetUInt32();
        ctrData->Rank = feilds[field_count++].GetUInt32();
        ctrData->KillCredit[0] = feilds[field_count++].GetUInt32();
        ctrData->KillCredit[1] = feilds[field_count++].GetUInt32();
        for(uint8 i = 0; i < 4; i++)
            ctrData->DisplayInfo[i] = feilds[field_count++].GetUInt32();
        ctrData->HealthMod = feilds[field_count++].GetFloat();
        ctrData->PowerMod = feilds[field_count++].GetFloat();
        ctrData->Civilian = feilds[field_count++].GetUInt8();
        ctrData->Leader = feilds[field_count++].GetUInt8();
        // Parse proto data
        ctrData->MinLevel = feilds[field_count++].GetUInt32();
        ctrData->MaxLevel = feilds[field_count++].GetUInt32();
        ctrData->Faction = feilds[field_count++].GetUInt32();
        ctrData->MinHealth = feilds[field_count++].GetUInt32();
        ctrData->MaxHealth = feilds[field_count++].GetUInt32();
        ctrData->Powertype = feilds[field_count++].GetUInt8();
        ctrData->MinPower = feilds[field_count++].GetUInt32();
        ctrData->MaxPower = feilds[field_count++].GetUInt32();
        ctrData->Scale = feilds[field_count++].GetFloat();
        ctrData->NPCFLags = feilds[field_count++].GetUInt32();
        ctrData->AttackTime = feilds[field_count++].GetUInt32();
        ctrData->AttackType = feilds[field_count++].GetUInt32();
        ctrData->MinDamage = feilds[field_count++].GetFloat();
        ctrData->MaxDamage = feilds[field_count++].GetFloat();
        ctrData->RangedAttackTime = feilds[field_count++].GetUInt32();
        ctrData->RangedMinDamage = feilds[field_count++].GetFloat();
        ctrData->RangedMaxDamage = feilds[field_count++].GetFloat();
        ctrData->Item1 = feilds[field_count++].GetUInt32();
        ctrData->Item2 = feilds[field_count++].GetUInt32();
        ctrData->Item3 = feilds[field_count++].GetUInt32();
        ctrData->RespawnTime = feilds[field_count++].GetUInt32();
        for(uint8 i = 0; i < 7; i++)
            ctrData->Resistances[i] = feilds[field_count++].GetUInt32();
        ctrData->CombatReach = feilds[field_count++].GetFloat();
        ctrData->BoundingRadius = feilds[field_count++].GetFloat();
        const char* aurastring = feilds[field_count++].GetString();
        ctrData->Boss = feilds[field_count++].GetUInt32();
        ctrData->Money = feilds[field_count++].GetInt32();
        ctrData->Invisibility_type = feilds[field_count++].GetUInt32();
        ctrData->Walk_speed = feilds[field_count++].GetFloat();
        ctrData->Run_speed = feilds[field_count++].GetFloat();
        ctrData->Fly_speed = feilds[field_count++].GetFloat();
        ctrData->Extra_a9_flags = feilds[field_count++].GetUInt32();
        ctrData->AuraMechanicImmunity = feilds[field_count++].GetUInt32();
        ctrData->Vehicle_entry = feilds[field_count++].GetInt32();
        ctrData->BattleMasterType = feilds[field_count++].GetUInt32();
        ctrData->SpellClickid = feilds[field_count++].GetUInt32();
        ctrData->CanMove = feilds[field_count++].GetUInt32();
        ctrData->lowercase_name.append(ctrData->Name);
        RONIN_UTIL::TOLOWER(ctrData->lowercase_name);

        if(aurastring && strlen(aurastring))
        {
            std::vector<std::string> aurs = RONIN_UTIL::StrSplit(std::string(aurastring), " ");
            for(std::vector<std::string>::iterator it = aurs.begin(); it != aurs.end(); it++)
                if(uint32 id = atol((*it).c_str()))
                    ctrData->Auras.insert( id );
        }

        if (ctrData->AttackType > SCHOOL_ARCANE)
            ctrData->AttackType = SCHOOL_NORMAL;

        m_creatureData.insert(std::make_pair(ctrData->Entry, ctrData));
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

            SkillLineSpell * _spell = objmgr.GetSpellSkill(spellID);
            if(_spell)
            {
                SkillLineEntry * _skill = dbcSkillLine.LookupEntry(_spell->skilline);
                if(_skill)
                {
                    if(_skill->type == SKILL_TYPE_PROFESSION)
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
