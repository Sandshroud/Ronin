/***
 * Demonstrike Core
 */

#include "StdAfx.h"

uint32 getConColor(uint16 AttackerLvl, uint16 VictimLvl)
{

#define PLAYER_LEVEL_CAP 85
    const uint32 grayLevel[PLAYER_LEVEL_CAP+1] = {0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,13,14,15,16,17,18,19,20,21,22,22,23,24,25,26,27,28,29,30,31,31,32,33,34,35,35,36,37,38,39,39,40,41,42,43,43,44,45,46,47,47,48,49,50,51,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,65,66,66,67,68,69};
    if(AttackerLvl + 5 <= VictimLvl)
    {
        if(AttackerLvl + 10 <= VictimLvl)
        {
            return 5;
        }
        return 4;
    }
    else
    {
        switch(VictimLvl - AttackerLvl)
        {
        case 4:
        case 3:
            return 3;
            break;
        case 2:
        case 1:
        case 0:
        case -1:
        case -2:
            return 2;
            break;
        default:
            // More adv formula for grey/green lvls:
            if(AttackerLvl <= 6)
            {
                return 1; //All others are green.
            }
            else
            {
                if(AttackerLvl > PLAYER_LEVEL_CAP)
                    return 1;//gm
                if(AttackerLvl<PLAYER_LEVEL_CAP && VictimLvl <= grayLevel[AttackerLvl])
                    return 0;
                else
                    return 1;
            }
        }
    }
#undef PLAYER_LEVEL_CAP
}

uint32 CalculateXpToGive(Unit* pVictim, Unit* pAttacker)
{
    if(pVictim->IsPlayer())
        return 0;

    if( TO_CREATURE(pVictim)->IsTotem())
        return 0;

    CreatureInfo *victimI;
    victimI = TO_CREATURE(pVictim)->GetCreatureInfo();

    if(victimI)
        if(victimI->Type == CRITTER)
            return 0;
    uint32 VictimLvl = pVictim->getLevel();
    uint32 AttackerLvl = pAttacker->getLevel();

    if( pAttacker->IsPet() && TO_PET(pAttacker)->GetPetOwner() )
    {
        // based on: http://www.wowwiki.com/Talk:Formulas:Mob_XP#Hunter.27s_pet_XP (2008/01/12)
        uint32 ownerLvl = TO_PET( pAttacker )->GetPetOwner()->getLevel();
        VictimLvl += ownerLvl - AttackerLvl;
        AttackerLvl = ownerLvl;
    }
    else if( (int32)VictimLvl - (int32)AttackerLvl > 10 ) //not wowwikilike but more balanced
        return 0;

    // Partha: this screws things up for pets and groups

    float zd = 5;
    float g = 5;

    // get zero diff
    // get grey diff

    if(AttackerLvl >= 100)
    {
        zd = 23;
        g = 19;
    }
    else if(AttackerLvl >= 90)
    {
        zd = 22;
        g = 18;
    }
    else if(AttackerLvl >= 80)
    {
        zd = 21;
        g = 17;
    }
    else if(AttackerLvl >= 75)
    {
        zd = 20;
        g = 16;
    }
    else if(AttackerLvl >= 70)
    {
        zd = 19;
        g = 15;
    }
    else if(AttackerLvl >= 65)
    {
        zd = 18;
        g = 14;
    }
    else if(AttackerLvl >= 60)
    {
        zd = 17;
        g = 13;
    }
    else if(AttackerLvl >= 55)
    {
        zd = 16;
        g = 12;
    }
    else if(AttackerLvl >= 50)
    {
        zd = 15;
        g = 11;
    }
    else if(AttackerLvl >= 45)
    {
        zd = 14;
        g = 10;
    }
    else if(AttackerLvl >= 40)
    {
        zd = 13;
        g = 9;
    }
    else if(AttackerLvl >= 30)
    {
        zd = 12;
        g = 8;
    }
    else if(AttackerLvl >= 20)
    {
        zd = 11;
        g = 7;
    }
    else if(AttackerLvl >= 16)
    {
        zd = 9;
        g = 6;
    }
    else if(AttackerLvl >= 12)
    {
        zd = 8;
        g = 6;
    }
    else if(AttackerLvl >= 10)
    {
        zd = 7;
        g = 6;
    }
    else if(AttackerLvl >= 8)
    {
        zd = 6;
        g = 5;
    }
    else
    {
        zd = 5;
        g = 5;
    }

    float xp = 0.0f;
    float fVictim = float(VictimLvl);
    float fAttacker = float(AttackerLvl);

    if(VictimLvl == AttackerLvl)
        xp = float( ((fVictim * 5.0f) + 45.0f) );
    else if(VictimLvl > AttackerLvl)
    {
        float j = 1.0f + (0.25f * (fVictim - fAttacker));
        xp = float( ((AttackerLvl * 5.0f) + 45.0f) * j );
    }
    else
    {
        if((AttackerLvl - VictimLvl) < g)
        {
            float j = (1.0f - float((fAttacker - fVictim) / zd));
            xp = (AttackerLvl * 5.0f + 45.0f) * j;
        }
    }

    // multiply by global XP rate
    if(xp == 0.0f)
        return 0;

    xp *= sWorld.getRate(RATE_XP);

    // elite boss multiplier
    if(victimI)
    {
        switch(victimI->Rank)
        {
        case 0: // normal mob
            break;
        case 1: // elite
            xp *= 2.0f;
            break;
        case 2: // rare elite
            xp *= 2.0f;
            break;
        case 3: // world boss
            xp *= 2.5f;
            break;
        default:    // rare or higher
            //          xp *= 7.0f;
            break;
        }
    }
    if( xp < 0 )//probably caused incredible wrong exp
        xp = 0;

    return (uint32)xp;
}

/*
Author: pionere

Calculate the stat increase. Using 3rd grade polynome.

Parameter level The level the character reached.
Parameter a3 The factor for x^3.
Parameter a2 The factor for x^2.
Parameter a1 The factor for x^1.
Parameter a0 The constant factor for the polynome.
Return stat gain
*/
uint32 CalculateStat(uint16 level,double a3, double a2, double a1, double a0)
{
    int result1;
    int result2;
    int diff;

    result1 =   (int)(a3*level*level*level +
        a2*level*level +
        a1*level +
        a0);

    result2 =   (int)(a3*(level-1)*(level-1)*(level-1) +
        a2*(level-1)*(level-1) +
        a1*(level-1) +
        a0);

    //get difference
    diff = result1-result2;
    return diff;
}

uint32 CalculateStat(uint16 level, float inc)
{
    float a = level * inc;

    return (uint32)a; // truncate: working as intended.
}

#define StatCalculationMacroAction(_stat, statfloat1, statfloat2, statfloat3, statfloat4, statfloat5)\
    switch(_stat)\
    {\
    case STAT_STRENGTH: { gain = CalculateStat(level, statfloat1); } break;\
    case STAT_AGILITY:  { gain = CalculateStat(level, statfloat2); } break;\
    case STAT_STAMINA:  { gain = CalculateStat(level, statfloat3); } break;\
    case STAT_INTELLECT:    { gain = CalculateStat(level, statfloat4); } break;\
    case STAT_SPIRIT:   { gain = CalculateStat(level, statfloat5); } break;\
    }

uint32 CalcStatForLevel(uint16 level, uint8 playerrace, uint8 playerclass, uint8 Stat)
{
    uint32 gain = 0;
    switch(playerrace)
    {
    case RACE_HUMAN:
        {
            switch(playerclass)
            {
            case WARRIOR:
                {
                    StatCalculationMacroAction(Stat, 2.3142f, 1.4125f, 2.1013f, 0.4514f, 0.7523f);
                }break;
            case PALADIN:
                {
                    StatCalculationMacroAction(Stat, 2.1625f, 1.1251f, 2.0000f, 1.2254f, 1.3517f);
                }break;
            case ROGUE:
                {
                    StatCalculationMacroAction(Stat, 1.4125f, 2.3625f, 1.3625f, 0.5375f, 0.8625f);
                }break;
            case PRIEST:
                {
                    StatCalculationMacroAction(Stat, 0.5375f, 0.6375f, 0.8375f, 2.1753f, 2.3252f);
                }break;
            case DEATHKNIGHT:
                {
                    StatCalculationMacroAction(Stat, 2.2516f, 1.4125f, 2.0013f, 0.4375f, 0.7518f);
                }break;
            case MAGE:
                {
                    StatCalculationMacroAction(Stat, 0.4527f, 0.5375f, 0.7375f, 2.2625f, 2.2375f);
                }break;
            case WARLOCK:
                {
                    StatCalculationMacroAction(Stat, 0.7375f, 0.8375f, 1.2125f, 1.9875f, 2.1257f);
                }break;
            }
        }break; // Human End
    case RACE_ORC:
        {
            switch(playerclass)
            {
            case WARRIOR:
                {
                    StatCalculationMacroAction(Stat, 2.3375f, 1.3750f, 2.1250f, 0.4125f, 0.7751f);
                }break;
            case HUNTER:
                {
                    StatCalculationMacroAction(Stat, 0.9625f, 2.2250f, 1.6250f, 1.0875f, 1.2512f);
                }break;
            case ROGUE:
                {
                    StatCalculationMacroAction(Stat, 1.4516f, 2.3250f, 1.3375f, 0.5174f, 0.8753f);
                }break;
            case DEATHKNIGHT:
                {
                    StatCalculationMacroAction(Stat, 2.4250f, 1.3625f, 2.1375f, 0.4133f, 0.7753f);
                }break;
            case SHAMAN:
                {
                    StatCalculationMacroAction(Stat, 1.5375f, 0.8875f, 1.7250f, 1.7125f, 1.8250f);
                }break;
            case WARLOCK:
                {
                    StatCalculationMacroAction(Stat, 0.7751f, 0.8012f, 1.2375f, 1.9516f, 2.1125f);
                }break;
            }
        }break; // Orc End

    case RACE_DWARF:
        {
            switch(playerclass)
            {
            case WARRIOR:
                {
                    StatCalculationMacroAction(Stat, 2.3253f, 1.3625f, 2.1375f, 0.4375f, 0.7250f);
                }break;
            case PALADIN:
                {
                    StatCalculationMacroAction(Stat, 2.1875f, 1.0750f, 1.8250f, 1.2125f, 1.3000f);
                }break;
            case HUNTER:
                {
                    StatCalculationMacroAction(Stat, 0.9516f, 2.2125f, 1.6375f, 1.1125f, 1.2163f);
                }break;
            case ROGUE:
                {
                    StatCalculationMacroAction(Stat, 1.4375f, 2.3125f, 1.3516f, 0.5251f, 0.8254f);
                }break;
            case PRIEST:
                {
                    StatCalculationMacroAction(Stat, 0.5625f, 0.5875f, 0.8752f, 2.1625f, 2.3625f);
                }break;
            case DEATHKNIGHT:
                {
                    StatCalculationMacroAction(Stat, 2.2750f, 1.3512f, 2.0375f, 0.4250f, 0.7250f);
                }break;
            }
        }break; // Dwarf End

    case RACE_NIGHTELF:
        {
            switch(playerclass)
            {
            case WARRIOR:
                {
                    StatCalculationMacroAction(Stat, 2.2625f, 1.4750f, 2.0875f, 0.4502f, 0.7375f);
                }break;
            case HUNTER:
                {
                    StatCalculationMacroAction(Stat, 0.8875f, 2.4125f, 1.5875f, 1.1625f, 1.2125f);
                }break;
            case ROGUE:
                {
                    StatCalculationMacroAction(Stat, 1.3750f, 2.4250f, 1.3016f, 0.5375f, 0.8375f);
                }break;
            case PRIEST:
                {
                    StatCalculationMacroAction(Stat, 0.5023f, 0.7027f, 0.8250f, 2.5016f, 2.3875f);
                }break;
            case DEATHKNIGHT:
                {
                    StatCalculationMacroAction(Stat, 2.2125f, 1.4625f, 1.9875f, 0.4375f, 0.7375f);
                }break;
            case DRUID:
                {
                    StatCalculationMacroAction(Stat, 1.0750f, 1.0875f, 1.2125f, 1.7875f, 1.9875f);
                }break;
            }
        }break; // Nightelf End

    case RACE_UNDEAD:
        {
            switch(playerclass)
            {
            case WARRIOR:
                {
                    StatCalculationMacroAction(Stat, 2.1625f, 1.3875f, 2.1362f, 0.4251f, 0.8016f);
                }break;
            case ROGUE:
                {
                    StatCalculationMacroAction(Stat, 1.4006f, 2.3375f, 1.3250f, 0.5125f, 0.9012f);
                }break;
            case PRIEST:
                {
                    StatCalculationMacroAction(Stat, 0.5251f, 0.6125f, 0.8501f, 2.1514f, 2.3250f);
                }break;
            case DEATHKNIGHT:
                {
                    StatCalculationMacroAction(Stat, 2.1750f, 1.3750f, 2.0125f, 0.4125f, 0.8111f);
                }break;
            case MAGE:
                {
                    StatCalculationMacroAction(Stat, 0.4375f, 0.5125f, 0.7512f, 2.2375f, 2.2375f);
                }break;
            case WARLOCK:
                {
                    StatCalculationMacroAction(Stat, 0.7251f, 0.8125f, 1.2375f, 1.9625f, 2.1375f);
                }break;
            }
        }break; // Undead End

    case RACE_TAUREN:
        {
            switch(playerclass)
            {
            case WARRIOR:
                {
                    StatCalculationMacroAction(Stat, 2.2375f, 1.3512f, 2.1250f, 0.3875f, 0.7625f);
                }break;
            case HUNTER:
                {
                    StatCalculationMacroAction(Stat, 0.9875f, 2.2875f, 1.6250f, 1.1103f, 1.2375f);
                }break;
            case DEATHKNIGHT:
                {
                    StatCalculationMacroAction(Stat, 2.3125f, 1.3375f, 2.0250f, 0.3751f, 0.7625f);
                }break;
            case SHAMAN:
                {
                    StatCalculationMacroAction(Stat, 1.5625f, 0.8625f, 1.7250f, 1.6875f, 1.8125f);
                }break;
            case DRUID:
                {
                    StatCalculationMacroAction(Stat, 1.1750f, 0.9625f, 1.2512f, 1.7250f, 2.3125f);
                }break;
            }
        }break; // Tauren End

    case RACE_GNOME:
        {
            switch(playerclass)
            {
            case WARRIOR:
                {
                    StatCalculationMacroAction(Stat, 2.1875f, 1.4516f, 2.0512f, 0.5163f, 0.7375f);
                }break;
            case ROGUE:
                {
                    StatCalculationMacroAction(Stat, 1.3504f, 2.4132f, 1.3516f, 0.6163f, 0.8375f);
                }break;
            case DEATHKNIGHT:
                {
                    StatCalculationMacroAction(Stat, 2.1250f, 1.4375f, 2.1163f, 0.4875f, 0.7375f);
                }break;
            case MAGE:
                {
                    StatCalculationMacroAction(Stat, 0.3875f, 0.5751f, 0.7254f, 2.4125f, 2.1750f);
                }break;
            case WARLOCK:
                {
                    StatCalculationMacroAction(Stat, 0.6753f, 0.8750f, 1.2123f, 2.1250f, 2.0750f);
                }break;
            }
        }break; // Gnome End

    case RACE_TROLL:
        {
            switch(playerclass)
            {
            case WARRIOR:
                {
                    StatCalculationMacroAction(Stat, 2.3125f, 1.4375f, 2.1125f, 0.4163f, 0.7516f);
                }break;
            case HUNTER:
                {
                    StatCalculationMacroAction(Stat, 0.9375f, 2.3750f, 1.6125f, 1.1125f, 1.2250f);
                }break;
            case ROGUE:
                {
                    StatCalculationMacroAction(Stat, 1.4250f, 2.3875f, 1.3750f, 0.4875f, 0.8516f);
                }break;
            case PRIEST:
                {
                    StatCalculationMacroAction(Stat, 0.5516f, 0.6625f, 0.8516f, 2.1250f, 2.2750f);
                }break;
            case DEATHKNIGHT:
                {
                    StatCalculationMacroAction(Stat, 2.2625f, 1.4250f, 2.0125f, 0.3875f, 0.7516f);
                }break;
            case SHAMAN:
                {
                    StatCalculationMacroAction(Stat, 1.5125f, 0.9542f, 1.7125f, 1.7000f, 1.8012f);
                }break;
            case MAGE:
                {
                    StatCalculationMacroAction(Stat, 0.4625f, 0.5625f, 0.7512f, 2.2125f, 2.1875f);
                }break;
            }
        }break; // Troll End

    case RACE_BLOODELF:
        {
            switch(playerclass)
            {
            case PALADIN:
                {
                    StatCalculationMacroAction(Stat, 1.8512f, 1.1512f, 1.7625f, 1.2750f, 1.3153f);
                }break;
            case HUNTER:
                {
                    StatCalculationMacroAction(Stat, 0.8875f, 2.2875f, 1.5750f, 1.1750f, 1.2163f);
                }break;
            case ROGUE:
                {
                    StatCalculationMacroAction(Stat, 1.3750f, 2.3875f, 1.3375f, 0.5875f, 0.8251f);
                }break;
            case PRIEST:
                {
                    StatCalculationMacroAction(Stat, 0.5163f, 0.6625f, 0.8125f, 2.2250f, 2.2512f);
                }break;
            case DEATHKNIGHT:
                {
                    StatCalculationMacroAction(Stat, 2.1516f, 1.4250f, 1.9750f, 0.4875f, 0.7251f);
                }break;
            case MAGE:
                {
                    StatCalculationMacroAction(Stat, 0.4125f, 0.5625f, 0.7125f, 2.3125f, 2.1625f);
                }break;
            case WARLOCK:
                {
                    StatCalculationMacroAction(Stat, 0.7135f, 0.8625f, 1.1875f, 2.0375f, 2.0625f);
                }break;
            }
        }break; // Bloodelf End

    case RACE_DRAENEI:
        {
            switch(playerclass)
            {
            case WARRIOR:
                {
                    StatCalculationMacroAction(Stat, 2.3125f, 1.3750f, 2.0875f, 0.4625f, 0.7625f);
                }break;
            case PALADIN:
                {
                    StatCalculationMacroAction(Stat, 1.9153f, 1.0875f, 1.7750f, 1.4125f, 1.3375f);
                }break;
            case HUNTER:
                {
                    StatCalculationMacroAction(Stat, 0.9375f, 2.2250f, 1.5875f, 1.1375f, 1.2375f);
                }break;
            case PRIEST:
                {
                    StatCalculationMacroAction(Stat, 0.5515f, 0.6000f, 0.8251f, 2.1875f, 2.2875f);
                }break;
            case DEATHKNIGHT:
                {
                    StatCalculationMacroAction(Stat, 2.2123f, 1.3625f, 1.9875f, 0.4512f, 0.7625f);
                }break;
            case SHAMAN:
                {
                    StatCalculationMacroAction(Stat, 1.5125f, 0.8875f, 1.6875f, 1.7625f, 1.8125f);
                }break;
            case MAGE:
                {
                    StatCalculationMacroAction(Stat, 0.4625f, 0.5126f, 0.7251f, 2.2750f, 2.2132f);
                }break;
            }
        }break; // Draenei End
    }

    return gain;
}

uint32 CalculateDamage( Unit* pAttacker, Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability ) // spellid is used only for 2-3 spells, that have AP bonus
{
    //TODO: Some awesome formula to determine how much damage to deal
    //consider this is melee damage
    // weapon_damage_type: 0 = melee, 1 = offhand(dualwield), 2 = ranged

    // Attack Power increases your base damage-per-second (DPS) by 1 for every 14 attack power.
    // (c) wowwiki

    //type of this UNIT_FIELD_ATTACK_POWER_MODS is unknown, not even uint32 disabled for now.

    uint32 offset;
    Item* it = NULLITEM;

    if(pAttacker->disarmed && pAttacker->IsPlayer())
    {
        offset=UNIT_FIELD_MINDAMAGE;
        it = TO_PLAYER(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
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
    Item* BonusItem = NULLITEM;
    if( pAttacker->IsPlayer() && weapon_damage_type == MELEE )
    {
        BonusItem = TO_PLAYER(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    }
    else if(pAttacker->IsPlayer() && weapon_damage_type == OFFHAND )
    {
        BonusItem = TO_PLAYER(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    }
    else if(pAttacker->IsPlayer() && weapon_damage_type == RANGED )
    {
        BonusItem = TO_PLAYER(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
    }

    if( pAttacker->IsPlayer() && BonusItem )
    {
        appbonuspct = TO_PLAYER(pAttacker)->m_WeaponSubClassDamagePct[ BonusItem->GetProto()->SubClass  ];
    }

    if(offset == UNIT_FIELD_MINRANGEDDAMAGE)
    {
        //starting from base attack power then we apply mods on it
        //ap += pAttacker->GetRAP();
        ap += pVictim->RAPvModifier;

        if(!pVictim->IsPlayer() && TO_CREATURE(pVictim)->GetCreatureInfo())
        {
            uint32 creatType = TO_CREATURE(pVictim)->GetCreatureInfo()->Type;
            ap += (float)pAttacker->CreatureRangedAttackPowerMod[creatType];

            if(pAttacker->IsPlayer())
            {
                min_damage = (min_damage + TO_PLAYER(pAttacker)->IncreaseDamageByType[creatType]) * (1 + TO_PLAYER(pAttacker)->IncreaseDamageByTypePCT[creatType]);
                max_damage = (max_damage + TO_PLAYER(pAttacker)->IncreaseDamageByType[creatType]) * (1 + TO_PLAYER(pAttacker)->IncreaseDamageByTypePCT[creatType]);
            }
        }

        if(pAttacker->IsPlayer())
        {
            if(!pAttacker->disarmed)
            {
                Item* it = TO_PLAYER(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
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
            if(TO_PLAYER(pAttacker)->IsInFeralForm())
            {
                uint8 ss = TO_PLAYER(pAttacker)->GetShapeShift();

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

        if(!pVictim->IsPlayer() && TO_CREATURE(pVictim)->GetCreatureInfo())
        {
            uint32 creatType = TO_CREATURE(pVictim)->GetCreatureInfo()->Type;
            ap += (float)pAttacker->CreatureAttackPowerMod[creatType];

            if(pAttacker->IsPlayer())
            {
                min_damage = (min_damage + TO_PLAYER(pAttacker)->IncreaseDamageByType[creatType]) * (1 + TO_PLAYER(pAttacker)->IncreaseDamageByTypePCT[creatType]);
                max_damage = (max_damage + TO_PLAYER(pAttacker)->IncreaseDamageByType[creatType]) * (1 + TO_PLAYER(pAttacker)->IncreaseDamageByTypePCT[creatType]);
            }
        }

        if(pAttacker->IsPlayer())
        {
            if(!pAttacker->disarmed)
            {
                Item* it = TO_PLAYER(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

                if(it)
                    wspeed = (float)it->GetProto()->Delay;
                else
                    wspeed = 2000;
            }
            else
                wspeed = (float)pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);

            if(ability && ability->SpellGroupType)
            {
                int32 apall = pAttacker->GetAP();
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
                it = TO_PLAYER(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

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
            if(TO_PLAYER(pAttacker)->IsInFeralForm())
            {
                uint8 ss = TO_PLAYER(pAttacker)->GetShapeShift();

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
    {
        return FL2UINT(result * appbonuspct);
    }

    return 0;
}

bool isEven (int num)
{
    if ((num%2)==0)
    {
        return true;
    }

    return false;
}
