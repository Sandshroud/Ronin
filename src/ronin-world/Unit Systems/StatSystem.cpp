/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton(StatSystem);

StatSystem::StatSystem()
{

}

StatSystem::~StatSystem()
{

}

bool StatSystem::Load()
{
    LoadClassPowers();
    return LoadUnitStats();
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

    for (uint8 i = 0; i < CLASS_MAX; ++i)
    {
        for (uint8 j = 0; j < POWER_TYPE_MAX; ++j)
        {
            if(PowersByClass[i][j] == POWER_TYPE_MAX)
                continue;
            m_unitPowersByClass.insert(std::make_pair(std::make_pair(i, j), EUnitFields(PowersByClass[i][j])));
        }
    }
}

bool StatSystem::LoadUnitStats()
{
    QueryResult *result = WorldDatabase.Query("SELECT * FROM unit_base_stats");
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
        baseStats->baseHP = fields[3].GetUInt32();
        baseStats->basePower = fields[4].GetUInt32();
        baseStats->baseStat[0] = fields[5].GetUInt32();
        baseStats->baseStat[1] = fields[6].GetUInt32();
        baseStats->baseStat[2] = fields[7].GetUInt32();
        baseStats->baseStat[3] = fields[8].GetUInt32();
        baseStats->baseStat[4] = fields[9].GetUInt32();
        m_UnitBaseStats.insert(std::make_pair(index, baseStats));
    }while(result->NextRow());
    return true;
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
        it = castPtr<Player>(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
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
        BonusItem = castPtr<Player>(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    }
    else if(pAttacker->IsPlayer() && weapon_damage_type == OFFHAND )
    {
        BonusItem = castPtr<Player>(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    }
    else if(pAttacker->IsPlayer() && weapon_damage_type == RANGED )
    {
        BonusItem = castPtr<Player>(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
    }

    if(offset == UNIT_FIELD_MINRANGEDDAMAGE)
    {
        //starting from base attack power then we apply mods on it
        //ap += pAttacker->GetRAP();
        ap += pVictim->RAPvModifier;

        if(!pVictim->IsPlayer() && castPtr<Creature>(pVictim)->GetCreatureData())
        {
            uint32 creatType = castPtr<Creature>(pVictim)->GetCreatureData()->Type;
            ap += (float)pAttacker->GetCreatureRangedAttackPowerMod(creatType);

            if(pAttacker->IsPlayer())
            {
                min_damage = (min_damage + castPtr<Player>(pAttacker)->IncreaseDamageByType[creatType]) * (1 + castPtr<Player>(pAttacker)->IncreaseDamageByTypePCT[creatType]);
                max_damage = (max_damage + castPtr<Player>(pAttacker)->IncreaseDamageByType[creatType]) * (1 + castPtr<Player>(pAttacker)->IncreaseDamageByTypePCT[creatType]);
            }
        }

        if(pAttacker->IsPlayer())
        {
            if(!pAttacker->disarmed)
            {
                Item* it = castPtr<Player>(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                if(it)
                {
                    wspeed = (float)it->GetProto()->Delay;
                }
                else
                    wspeed = 2000;
            }
            else
                wspeed = (float)pAttacker->GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME);
        }
        else
        {
            wspeed = (float)pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);
        }

        //ranged weapon normalization.
        if(pAttacker->IsPlayer() && ability)
        {
            if(ability->Effect[0] == SPELL_EFFECT_DUMMYMELEE || ability->Effect[1] == SPELL_EFFECT_DUMMYMELEE || ability->Effect[2] == SPELL_EFFECT_DUMMYMELEE)
            {
                wspeed = 2800;
            }
        }

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
        //ap += pAttacker->GetAP();
        ap += pVictim->APvModifier;

        if(!pVictim->IsPlayer() && castPtr<Creature>(pVictim)->GetCreatureData())
        {
            uint32 creatType = castPtr<Creature>(pVictim)->GetCreatureData()->Type;
            ap += (float)pAttacker->GetCreatureAttackPowerMod(creatType);

            if(pAttacker->IsPlayer())
            {
                min_damage = (min_damage + castPtr<Player>(pAttacker)->IncreaseDamageByType[creatType]) * (1 + castPtr<Player>(pAttacker)->IncreaseDamageByTypePCT[creatType]);
                max_damage = (max_damage + castPtr<Player>(pAttacker)->IncreaseDamageByType[creatType]) * (1 + castPtr<Player>(pAttacker)->IncreaseDamageByTypePCT[creatType]);
            }
        }

        if(pAttacker->IsPlayer())
        {
            if(!pAttacker->disarmed)
            {
                Item* it = castPtr<Player>(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

                if(it)
                    wspeed = (float)it->GetProto()->Delay;
                else
                    wspeed = 2000;
            }
            else
                wspeed = (float)pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);

            if(ability && ability->SpellGroupType)
            {
                int32 apall = pAttacker->CalculateAttackPower();
                /* this spell modifier doesn't exist. also need to clear up how the AP is used here
                int32 apb=0;
                SM_FIValue(pAttacker->SM[SMT_ATTACK_POWER_BONUS][1],&apb,ability->SpellGroupType);

                if(apb)
                    ap += apall*((float)apb/100);
                else*/
                    ap = float(apall);
            }
        }
        else
        {
            wspeed = (float)pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);
        }

        //Normalized weapon damage checks.
        if(pAttacker->IsPlayer() && ability)
        {
            if(ability->Effect[0] == SPELL_EFFECT_DUMMYMELEE || ability->Effect[1] == SPELL_EFFECT_DUMMYMELEE || ability->Effect[2] == SPELL_EFFECT_DUMMYMELEE)
            {
                it = castPtr<Player>(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

                if(it)
                {
                    if(it->GetProto()->Class == ITEM_CLASS_WEAPON) //weapon
                    {
                        if(it->GetProto()->InventoryType == INVTYPE_2HWEAPON) wspeed = 3300;
                        else if(it->GetProto()->SubClass == 15) wspeed = 1700;
                        else wspeed = 2400;
                    }
                }
            }
        }

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

        if (offset == UNIT_FIELD_MINDAMAGE)
            bonus = (wspeed-pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME))/14000.0f*ap;
        else if( offset == UNIT_FIELD_MINOFFHANDDAMAGE )
            bonus = (wspeed-pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1)) / 14000.0f*ap;
        else
            bonus = (wspeed-pAttacker->GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME))/14000.0f*ap;
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
    uint8 diff=0;
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
    }
    return diff;
}

uint32 CalculateXpToGive(Unit* pVictim, Unit* pAttacker)
{
    uint32 baseGain = 0, nBaseExp = 45+(611*3); // 3 is zone expansion

    if (pVictim->getLevel() >= pAttacker->getLevel())
    {
        uint8 nLevelDiff = pVictim->getLevel() - pAttacker->getLevel();
        if (nLevelDiff > 4) nLevelDiff = 4;
        baseGain = ((pAttacker->getLevel() * 5 + nBaseExp) * (20 + nLevelDiff) / 10 + 1) / 2;
    }
    else
    {
        uint8 gray_level = getGrayLevel(pAttacker->getLevel());
        if (pVictim->getLevel() > gray_level)
        {
            uint8 ZD = GetZeroDifference(pAttacker->getLevel());
            baseGain = (pAttacker->getLevel() * 5 + nBaseExp) * (ZD + pVictim->getLevel() - pAttacker->getLevel()) / ZD;
        } else baseGain = 0;
    }

    return baseGain;
}

bool isEven (int num)
{
    if ((num%2)==0)
        return true;
    return false;
}
