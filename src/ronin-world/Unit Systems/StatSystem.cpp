/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

initialiseSingleton(StatSystem);

StatSystem::StatSystem()
{

}

StatSystem::~StatSystem()
{
    while(m_UnitBaseStats.size())
    {
        delete m_UnitBaseStats.begin()->second;
        m_UnitBaseStats.erase(m_UnitBaseStats.begin());
    }
}

bool StatSystem::Load()
{
    LoadClassBaseStats();
    LoadClassPowers();
    LoadClassTalentData();
    return LoadUnitStats();
}

void StatSystem::LoadClassBaseStats()
{
    for(uint8 _class = WARRIOR; _class < CLASS_MAX; ++_class)
    {
        // Only process classes that exist
        if(dbcCharClass.LookupEntry(_class) == NULL)
            continue;

        gtFloat *lastHP = dbcBaseHPForClass.LookupEntry((_class-1) * 100);
        if(lastHP == NULL)
            continue;
        m_baseHPByClassLevel.insert(std::make_pair(std::make_pair(_class, ((uint32)1)), lastHP));
        gtFloat *lastMP = dbcBaseMPForClass.LookupEntry((_class-1) * 100);
        // Classes are not required to have MP
        if(lastMP != NULL && lastMP->val > 0) m_baseMPByClassLevel.insert(std::make_pair(std::make_pair(_class, ((uint32)1)), lastHP));

        for(uint32 level = 2; level < MAXIMUM_ATTAINABLE_LEVEL; ++level)
        {
            gtFloat *currentHP = dbcBaseHPForClass.LookupEntry(((_class-1) * 100)+level-1);
            if(currentHP == NULL)
                currentHP = lastHP;
            else lastHP = currentHP;
            m_baseHPByClassLevel.insert(std::make_pair(std::make_pair(_class, level), currentHP));

            // Classes are not required to have MP
            gtFloat *currentMP = dbcBaseMPForClass.LookupEntry((_class-1) * 100+level-1);
            if(currentMP == NULL || currentMP->val <= 0)
                currentMP = lastMP;
            else lastMP = currentMP;
            if(currentMP == NULL || currentMP->val <= 0)
                continue;

            m_baseMPByClassLevel.insert(std::make_pair(std::make_pair(_class, level), currentMP));
        }
    }
}

void StatSystem::LoadClassPowers()
{
    uint32 PowersByClass[CLASS_MAX][POWER_TYPE_MAX];
    for (uint8 i = 0; i < CLASS_MAX; ++i)
        for (uint8 j = 0; j < POWER_TYPE_MAX; ++j)
            PowersByClass[i][j] = POWER_TYPE_MAX;

    for (uint32 i = 0; i < dbcCharPowerType.GetNumRows(); ++i)
    {
        if (CharPowerTypeEntry const* power = dbcCharPowerType.LookupEntry(i))
        {
            uint32 field = UNIT_FIELD_POWERS;
            for (uint8 j = 0; j < POWER_TYPE_MAX; ++j)
                if (PowersByClass[power->classId][j] != POWER_TYPE_MAX)
                    field++;

            PowersByClass[power->classId][power->power] = field;
        }
    }

    for (uint8 i = WARRIOR; i < CLASS_MAX; ++i)
    {
        for (uint8 j = POWER_TYPE_MANA; j < POWER_TYPE_MAX; ++j)
        {
            if(PowersByClass[i][j] == POWER_TYPE_MAX)
                continue;
            if(j == POWER_TYPE_ALTERNATE)
            {
                switch(i)
                {
                case DEATHKNIGHT:
                    m_unitPowersForClass[i].push_back(POWER_TYPE_RUNE);
                    m_unitPowersByClass.insert(std::make_pair(std::make_pair(i, POWER_TYPE_RUNE), EUnitFields(PowersByClass[i][j])));
                    break;
                };
                continue;
            }

            m_unitPowersForClass[i].push_back(j);
            m_unitPowersByClass.insert(std::make_pair(std::make_pair(i, j), EUnitFields(PowersByClass[i][j])));
        }
    }
}

void StatSystem::LoadClassTalentData()
{
    SpellEntry *sp = NULL;
    for(uint32 i = 0; i < dbcTalentTab.GetNumRows(); i++)
    {
        if(TalentTabEntry *entry = dbcTalentTab.LookupRow(i))
        {
            entry->affectedClass = 0;
            for(uint8 i = WARRIOR; i < CLASS_MAX; i++)
            {
                if(entry->ClassMask & (1<<(i-1)))
                {
                    entry->affectedClass = i;
                    break;
                }
            }

            if(entry->affectedClass == 0)
                continue;

            for(uint8 i = 0; i < 2; i++)
                if(entry->masterySpells[i] && (sp = dbcSpell.LookupEntry(entry->masterySpells[i])))
                    m_talentMasterySpells.insert(std::make_pair(std::make_pair(entry->affectedClass, entry->TabPage), sp->Id));

            m_talentRoleMasks.insert(std::make_pair(std::make_pair(entry->affectedClass, entry->TabPage), entry->roleMask));
        }
    }

    TalentTreePrimarySpellsEntry *entry = NULL;
    for(uint32 i = 0; i < dbcTreePrimarySpells.GetNumRows(); i++)
        if((entry = dbcTreePrimarySpells.LookupRow(i)) && (sp = dbcSpell.LookupEntry(entry->SpellID)))
            if(TalentTabEntry *talentTab = dbcTalentTab.LookupEntry(entry->TalentTabID))
                m_talentPrimarySpells.insert(std::make_pair(std::make_pair(talentTab->affectedClass, talentTab->TabPage), sp->Id));
}

bool StatSystem::LoadUnitStats()
{
    QueryResult *result = WorldDatabase.Query("SELECT * FROM unit_basestats");
    if(result == NULL)
    {
        sLog.Error("StatMgr", "No unit stats loaded, server will not function without proper unit stat data!");
        return false;
    }

    do
    {
        Field *fields = result->Fetch();
        uint8 race = fields[0].GetUInt8();
        uint8 _class = fields[1].GetUInt8();
        uint16 level = fields[2].GetUInt16();
        uint32 index = uint32(level) | uint32(uint32(race)<<16) | uint32(uint32(_class)<<24);
        UnitBaseStats *baseStats = new UnitBaseStats();
        baseStats->baseHP = baseStats->baseMP = NULL;
        baseStats->level = level;
        for(uint8 i = 0; i < 5; ++i)
            baseStats->baseStat[i] = fields[3+i].GetUInt32();
        m_UnitBaseStats.insert(std::make_pair(index, baseStats));

        baseStats->baseHP = m_baseHPByClassLevel.at(std::make_pair(_class, ((uint32)level)));
        std::map<std::pair<uint8, uint32>, gtFloat*>::iterator itr;
        if((itr = m_baseMPByClassLevel.find(std::make_pair(_class, ((uint32)level)))) != m_baseMPByClassLevel.end())
            baseStats->baseMP = itr->second;

        std::pair<uint8, uint8> pair = std::make_pair(race, _class);
        if(m_maxBaseStats.find(pair) == m_maxBaseStats.end())
            m_maxBaseStats.insert(std::make_pair(pair, baseStats));
        else if(m_maxBaseStats.at(pair)->level < level)
            m_maxBaseStats[pair] = baseStats;
    }while(result->NextRow());
    return true;
}

UnitBaseStats *StatSystem::GetMaxUnitBaseStats(uint8 race, uint8 _class)
{
    std::pair<uint8, uint8> pair = std::make_pair(race, _class);
    if(m_maxBaseStats.find(pair) == m_maxBaseStats.end())
        return NULL;
    return m_maxBaseStats.at(pair);
}

UnitBaseStats *StatSystem::GetUnitBaseStats(uint8 race, uint8 _class, uint16 level)
{
    uint32 index = uint32(level) | uint32(uint32(race)<<16) | uint32(uint32(_class)<<24);
    if(m_UnitBaseStats.find(index) == m_UnitBaseStats.end())
        return NULL;
    return m_UnitBaseStats.at(index);
}

EUnitFields StatSystem::GetPowerFieldForClassAndType(uint8 _class, uint8 powerType)
{
    std::pair<uint8, uint8> pair = std::make_pair(_class, powerType);
    if(m_unitPowersByClass.find(pair) == m_unitPowersByClass.end())
        return UNIT_END;
    return m_unitPowersByClass.at(pair);
}

uint32 StatSystem::GetXPackModifierForLevel(uint32 level, uint32 mod)
{
    if(mod >= level)
        return 0;
    uint32 tgtLvl = level - mod;
    if(tgtLvl > 80)
        return 3;
    if(tgtLvl > 70)
        return 2;
    if(tgtLvl > 60)
        return 1;
    return 0;
}

uint32 StatSystem::CalculateDamage( Unit* pAttacker, Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability ) // spellid is used only for 2-3 spells, that have AP bonus
{
    //TODO: Some awesome formula to determine how much damage to deal
    //consider this is melee damage
    // weapon_damage_type: 0 = melee, 1 = offhand(dualwield), 2 = ranged

    // Attack Power increases your base damage-per-second (DPS) by 1 for every 14 attack power.
    // (c) wowwiki

    //type of this UNIT_FIELD_ATTACK_POWER_MODS is unknown, not even uint32 disabled for now.

    uint32 offset;
    Item* it = NULL;

    if(pAttacker->disarmed && pAttacker->IsPlayer())
    {
        offset=UNIT_FIELD_MINDAMAGE;
        it = castPtr<Player>(pAttacker)->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    }
    else if( weapon_damage_type == MELEE )
        offset = UNIT_FIELD_MINDAMAGE;
    else if( weapon_damage_type == OFFHAND )
        offset = UNIT_FIELD_MINOFFHANDDAMAGE;
    else  // weapon_damage_type == RANGED
        offset = UNIT_FIELD_MINRANGEDDAMAGE;

    float min_damage = pAttacker->GetFloatValue(offset);
    float max_damage = pAttacker->GetFloatValue(offset+1);
    if(it)
    {
        min_damage -= it->GetProto()->minDamage;
        max_damage -= it->GetProto()->maxDamage;
    }

    float ap = 0;
    float bonus;
    float wspeed;
    float appbonuspct = 1.0f;
    Item* BonusItem = NULL;
    if( pAttacker->IsPlayer() && weapon_damage_type == MELEE )
    {
        BonusItem = castPtr<Player>(pAttacker)->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    }
    else if(pAttacker->IsPlayer() && weapon_damage_type == OFFHAND )
    {
        BonusItem = castPtr<Player>(pAttacker)->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    }
    else if(pAttacker->IsPlayer() && weapon_damage_type == RANGED )
    {
        BonusItem = castPtr<Player>(pAttacker)->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
    }

    if(offset == UNIT_FIELD_MINRANGEDDAMAGE)
    {
        //starting from base attack power then we apply mods on it
        if(pAttacker->IsPlayer())
        {
            if(!pAttacker->disarmed)
            {
                if(Item* it = castPtr<Player>(pAttacker)->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED))
                    wspeed = (float)it->GetProto()->Delay;
                else wspeed = 2000;
            } else wspeed = (float)pAttacker->GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME);
        } else wspeed = (float)pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);

        //ranged weapon normalization.
        if(pAttacker->IsPlayer() && ability && (ability->Effect[0] == SPELL_EFFECT_DUMMYMELEE || ability->Effect[1] == SPELL_EFFECT_DUMMYMELEE || ability->Effect[2] == SPELL_EFFECT_DUMMYMELEE))
            wspeed = 2800;

        //Weapon speed constant in feral forms
        if(pAttacker->IsPlayer())
        {
            if(castPtr<Player>(pAttacker)->IsInFeralForm())
            {
                uint8 ss = castPtr<Player>(pAttacker)->GetShapeShift();

                if(ss == FORM_CAT)
                    wspeed = 1000.0;
                else if(ss == FORM_DIREBEAR || ss == FORM_BEAR)
                    wspeed = 2500.0;
            }
        }

        bonus = (wspeed-pAttacker->GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME))/14000.0f*ap;
        min_damage += bonus;
        max_damage += bonus;
    }
    else
    {
        //MinD = AP(28AS-(WS/7))-MaxD
        //starting from base attack power then we apply mods on it
        if(pAttacker->IsPlayer())
        {
            if(!pAttacker->disarmed)
            {
                if(Item* it = castPtr<Player>(pAttacker)->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
                    wspeed = (float)it->GetProto()->Delay;
                else wspeed = 2000;
            } else wspeed = (float)pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);

            if(ability && ability->SpellGroupType)
            {
                int32 apall = pAttacker->CalculateAttackPower();
                ap = float(apall);
            }

            //Weapon speed constant in feral forms
            if(castPtr<Player>(pAttacker)->IsInFeralForm())
            {
                uint8 ss = castPtr<Player>(pAttacker)->GetShapeShift();

                if(ss == FORM_CAT)
                    wspeed = 1000.0;
                else if(ss == FORM_DIREBEAR || ss == FORM_BEAR)
                    wspeed = 2500.0;
            }
        } else wspeed = (float)pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);

        if (offset == UNIT_FIELD_MINDAMAGE)
            bonus = (wspeed-pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME))/14000.0f*ap;
        else if( offset == UNIT_FIELD_MINOFFHANDDAMAGE )
            bonus = (wspeed-pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1)) / 14000.0f*ap;
        else bonus = (wspeed-pAttacker->GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME))/14000.0f*ap;
        min_damage += bonus;
        max_damage += bonus;
    }

    float diff = fabs(max_damage - min_damage);
    float result = min_damage;

    if(diff >= 1)
        result += float(RandomDouble(diff));

    if(result >= 0)
        return float2int32(result * appbonuspct);
    return 0;
}

uint8 StatSystem::GetViableOptionForRace(uint8 race)
{
    switch(race)
    {
    case RACE_FELORC:
        return RACE_ORC;
    case RACE_NAGA:
        return RACE_NIGHTELF;
    case RACE_BROKEN:
        return RACE_DRAENEI;
    case RACE_VRYKUL:
        return RACE_HUMAN;
    case RACE_SKELETON:
    case RACE_NORTHREND_SKELETON:
        return RACE_UNDEAD;
    case RACE_FORESTTROLL:
    case RACE_ICE_TROLL:
        return RACE_TROLL;
    case RACE_TAUNKA:
        return RACE_TAUREN;
    case RACE_SUBHUMAN:
        return RACE_WORGEN;
    }

    return race;
}

uint32 getGrayLevel(uint16 unitLevel)
{
    if (unitLevel > 5 && unitLevel <= 39)
        return unitLevel - 5 - unitLevel / 10;
    else if (unitLevel <= 59)
        return unitLevel - 1 - unitLevel / 5;
    return unitLevel - 9;
}

inline uint8 GetZeroDifference(uint8 pl_level)
{
    uint8 diff = 0;
    if(pl_level >= 10)
    {
        if(pl_level <= 20)
            diff += float2int32(floor((float(pl_level)-8.f)/2.f));
        else
        {
            for(uint8 i = 2; i <= 5; i++)
            {
                diff += 4;
                if(pl_level <= (20*i))
                {
                    diff += float2int32(floor((float(pl_level)-float((20*(i-1))-2))/2.f));
                    break;
                }
                if(i == 5) diff *= 2;
            }
        }
    } else diff = 5;
    return diff;
}

uint32 CalculateXpToGive(Unit* pVictim, Unit* pAttacker, uint32 mapZoneModifier)
{
    uint32 baseGain = 0, nBaseExp = 45, modifier = mapZoneModifier;
    while(modifier > 0)
        nBaseExp *= 1.8f + (--modifier);
    if (pVictim->getLevel() >= pAttacker->getLevel())
    {
        uint8 nLevelDiff = pVictim->getLevel() - pAttacker->getLevel();
        if (nLevelDiff > 4) nLevelDiff = 4;
        baseGain = ((pAttacker->getLevel() * 5 + nBaseExp) * (20 + nLevelDiff) / 10 + 1) / 2;
    }
    else
    {
        uint8 gray_level = getGrayLevel(pAttacker->getLevel()), ZD = 0;
        if (pVictim->getLevel() > gray_level && (ZD = GetZeroDifference(pAttacker->getLevel())))
            baseGain = (pAttacker->getLevel() * 5 + nBaseExp) * (ZD + pVictim->getLevel() - pAttacker->getLevel()) / ZD;
        else baseGain = 0;
    }

    return baseGain * sWorld.getRate(RATE_XP);
}

bool isEven (int num)
{
    if ((num%2)==0)
        return true;
    return false;
}
