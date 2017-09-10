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

Unit::Unit() : WorldObject(), m_AuraInterface(this), m_movementInterface(this), m_spellInterface(this), m_unitTeam(TEAM_NONE) { }

void Unit::Construct(WoWGuid guid, uint32 fieldCount)
{
    WorldObject::Construct(guid, fieldCount);

    SetTypeFlags(TYPEMASK_TYPE_UNIT);
    m_objType = TYPEID_UNIT;

    m_updateFlags |= UPDATEFLAG_LIVING;

    m_attackInterrupt = 0;
    memset(&m_attackTimer, 0, sizeof(uint16)*2);
    memset(&m_attackDelay, 0, sizeof(uint16)*2);
    m_dualWield = m_autoShot = false;

    baseStats = NULL;

    m_state = 0;
    m_deathState = ALIVE;

    m_silenced = 0;
    _stunStateCounter = 0;

    disarmed = false;
    disarmedShield = false;

    //DK:modifiers
    for( uint32 x = 0; x < 4; x++ )
        m_ObjectSlots[x] = 0;

    m_P_regenTimer = 0;
    m_H_regenTimer = 1000;
    m_interruptRegen = 0;
    m_powerRegenPCT = 0;

    m_triggerSpell = 0;
    m_triggerDamage = 0;
    m_canMove = 0;
    m_noInterrupt = 0;
    m_modelhalfsize = 1.0f; //worst case unit size. (Should be overwritten)

    m_invisFlag = INVIS_FLAG_NORMAL;

    for(int i = 0; i < INVIS_FLAG_TOTAL; i++)
        m_invisDetect[i] = 0;

    m_combatStopTimer = 0;
    m_attackUpdateTimer = 0;
    m_emoteState = 0;
    m_oldEmote = 0;

    m_p_DelayTimer = 0;

    m_onAuraRemoveSpells.clear();

    m_DummyAuras.clear();
    m_autoShotSpell = NULL;

    m_vehicleKitId = 0;
    m_aiAnimKitId = 0;
    m_movementAnimKitId = 0;
    m_meleeAnimKitId = 0;
}

Unit::~Unit()
{

}

void Unit::Init()
{
    WorldObject::Init();

    // Attach our movement interface to us
    m_movementInterface.AttachToOwner();

    // Trigger a stat update
    TriggerModUpdate(UF_UTYPE_STATS);
    TriggerModUpdate(UF_UTYPE_ATTACKTIME);
    TriggerModUpdate(UF_UTYPE_POWERCOST);

    // Required regeneration flag
    SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER );

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, M_PI );
    SetFloatValue(UNIT_FIELD_COMBATREACH, 5.f );
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.f);
    SetFloatValue(UNIT_MOD_CAST_HASTE, 1.f);
    SetFloatValue(UNIT_FIELD_HOVERHEIGHT, 0.001f);
}

void Unit::Destruct()
{
    if (IsInWorld())
        RemoveFromWorld();

    m_spellInterface.Cleanup();
    m_DummyAuras.clear();

    Loki::AssocVector<uint32, onAuraRemove*>::iterator itr;
    for ( itr = m_onAuraRemoveSpells.begin() ; itr != m_onAuraRemoveSpells.end() ; itr++)
        delete itr->second;
    m_onAuraRemoveSpells.clear();

    m_AuraInterface.Destruct();

    WorldObject::Destruct();
}

void Unit::Update(uint32 msTime, uint32 uiDiff)
{
    WorldObject::Update(msTime, uiDiff);
    // Update our delayed field values
    UpdateFieldValues();

    // Update spell interface(delayed, casting, channel etc)
    m_spellInterface.Update(msTime, uiDiff);

    // Update aura triggers and modifiers
    m_AuraInterface.Update(uiDiff);
    if(!isDead())
    {
        // Update attack timers
        for(uint8 i = 0; i < 2; i++)
        {
            if(m_attackDelay[i] == 0)
                continue;
            if(m_attackTimer[i] >= uiDiff)
                m_attackTimer[i] -= uiDiff;
            else m_attackTimer[i] = 0;
        }

        if(HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT))
        {
            if((m_combatStopTimer += uiDiff) > 1000)
            {
                if(IsInWorld() && !m_mapInstance->CheckCombatStatus(this))
                    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
                m_combatStopTimer = 0;
            }
        }

        /*-----------------------POWER & HP REGENERATION-----------------*/
        if(!isFullHealth())
        {
            if( m_H_regenTimer <= uiDiff )
            {
                m_H_regenTimer = 1000;//set next regen time
                RegenerateHealth(IsInCombat());
            } else m_H_regenTimer -= uiDiff;
        } else m_H_regenTimer = 1000;

        m_P_regenTimer += uiDiff;
        if(m_P_regenTimer >= 1000)
        {
            if(m_p_DelayTimer > m_P_regenTimer)
                m_p_DelayTimer -= m_P_regenTimer;
            else m_p_DelayTimer = 0;

            RegeneratePower( m_p_DelayTimer > 0 );
            m_P_regenTimer = 0;
        }

        m_movementInterface.Update(msTime, uiDiff);
    }
}

void Unit::OnAuraModChanged(uint32 modType)
{
    std::vector<uint8> pendingIndex;
    switch(modType)
    {
    case SPELL_AURA_MOD_STAT:
    case SPELL_AURA_MOD_PERCENT_STAT:
    case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
        pendingIndex.push_back(UF_UTYPE_STATS);
        break;
    case SPELL_AURA_MOD_BASE_HEALTH_PCT:
    case SPELL_AURA_MOD_INCREASE_HEALTH:
    case SPELL_AURA_MOD_INCREASE_MAX_HEALTH:
    case SPELL_AURA_MOD_INCREASE_HEALTH_2:
    case SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT:
        pendingIndex.push_back(UF_UTYPE_HEALTH);
        break;
    case SPELL_AURA_MOD_INCREASE_ENERGY:
    case SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT:
        pendingIndex.push_back(UF_UTYPE_POWER);
        break;
    case SPELL_AURA_MOD_POWER_REGEN:
    case SPELL_AURA_MOD_POWER_REGEN_PERCENT:
    case SPELL_AURA_MOD_MANA_REGEN_INTERRUPT:
        pendingIndex.push_back(UF_UTYPE_REGEN);
        break;
    case SPELL_AURA_MOD_ATTACKSPEED:
        pendingIndex.push_back(UF_UTYPE_ATTACKTIME);
        break;
    case SPELL_AURA_MOD_RESISTANCE:
    case SPELL_AURA_MOD_RESISTANCE_PCT:
    case SPELL_AURA_MOD_BASE_RESISTANCE:
    case SPELL_AURA_MOD_BASE_RESISTANCE_PCT:
    case SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE:
        pendingIndex.push_back(UF_UTYPE_RESISTANCE);
        break;
    case SPELL_AURA_MOD_ATTACK_POWER_PCT:
    case SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR:
        pendingIndex.push_back(UF_UTYPE_ATTACKPOWER);
        break;
    case SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT:
        pendingIndex.push_back(UF_UTYPE_RANGEDATTACKPOWER);
        break;
    case SPELL_AURA_MOD_DAMAGE_DONE:
    case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:
        pendingIndex.push_back(UF_UTYPE_ATTACKDAMAGE);
        if(IsPlayer()) pendingIndex.push_back(UF_UTYPE_PLAYERDAMAGEMODS);
        break;
    case SPELL_AURA_MOD_POWER_COST_SCHOOL:
    case SPELL_AURA_MOD_POWER_COST:
        pendingIndex.push_back(UF_UTYPE_POWERCOST);
        break;
    case SPELL_AURA_HOVER:
        pendingIndex.push_back(UF_UTYPE_HOVER);
        pendingIndex.push_back(UF_UTYPE_MOVEMENT);
        break;
        // Player opcode handling
    case SPELL_AURA_OVERRIDE_SPELL_POWER_BY_AP_PCT:
    case SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT:
    case SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER:
    case SPELL_AURA_MOD_HEALING_DONE:
    case SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT:
    case SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER:
        if(IsPlayer()) pendingIndex.push_back(UF_UTYPE_PLAYERDAMAGEMODS);
        break;
    case SPELL_AURA_MASTERY:
    case SPELL_AURA_MOD_RATING:
    case SPELL_AURA_MOD_RATING_FROM_STAT:
        if(IsPlayer()) pendingIndex.push_back(UF_UTYPE_PLAYERRATINGS);
        break;
        /// Movement handler opcodes
        // Enabler opcodes
    case SPELL_AURA_FLY:
        // State change opcodes
    case SPELL_AURA_MOD_ROOT:
    case SPELL_AURA_WATER_WALK:
    case SPELL_AURA_FEATHER_FALL:
        // Speed opcodes
    case SPELL_AURA_MOD_INCREASE_SPEED:
    case SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED:
    case SPELL_AURA_MOD_DECREASE_SPEED:
    case SPELL_AURA_MOD_INCREASE_SWIM_SPEED:
    case SPELL_AURA_MOD_SPEED_ALWAYS:
    case SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS:
    case SPELL_AURA_MOD_SPEED_NOT_STACK:
    case SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK:
    case SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED:
    case SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED:
    case SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED:
    case SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS:
    case SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK:
    case SPELL_AURA_MOD_MINIMUM_SPEED:
        pendingIndex.push_back(UF_UTYPE_MOVEMENT);
        break;
    case SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED:
    case SPELL_AURA_MOD_VEHICLE_SPEED_ALWAYS:
        pendingIndex.push_back(255);
        break;
    }
    if(pendingIndex.empty())
        return;

    queueModLock.Acquire();
    while(pendingIndex.size())
    {
        uint8 index = *pendingIndex.begin();
        pendingIndex.erase(pendingIndex.begin());
        m_modQueuedModUpdates[index].push_back(modType);
    }
    queueModLock.Release();
}

void Unit::UpdateFieldValues()
{
    if(m_modQueuedModUpdates.empty())
        return;

    queueModLock.Acquire();
    while(m_modQueuedModUpdates.size())
    {
        uint32 modType = m_modQueuedModUpdates.begin()->first;
        std::vector<uint32> vect(m_modQueuedModUpdates.begin()->second);
        m_modQueuedModUpdates.erase(m_modQueuedModUpdates.begin());
        queueModLock.Release();
        ProcessModUpdate(modType, vect);
        queueModLock.Acquire();
    }
    m_modQueuedModUpdates.clear();
    queueModLock.Release();
}

void Unit::ProcessModUpdate(uint8 modUpdateType, std::vector<uint32> modMap)
{
    switch(modUpdateType)
    {
    case UF_UTYPE_STATS: UpdateStatValues(); break;
    case UF_UTYPE_HEALTH: UpdateHealthValues(); break;
    case UF_UTYPE_POWER: UpdatePowerValues(); break;
    case UF_UTYPE_REGEN: UpdateRegenValues(); break;
    case UF_UTYPE_ATTACKTIME: UpdateAttackTimeValues(); break;
    case UF_UTYPE_RESISTANCE: UpdateResistanceValues(); break;
    case UF_UTYPE_ATTACKPOWER: UpdateAttackPowerValues(modMap); break;
    case UF_UTYPE_RANGEDATTACKPOWER: UpdateRangedAttackPowerValues(modMap); break;
    case UF_UTYPE_ATTACKDAMAGE: UpdateAttackDamageValues(); break;
    case UF_UTYPE_POWERCOST: UpdatePowerCostValues(modMap); break;
    case UF_UTYPE_HOVER: UpdateHoverValues(); break;
    case UF_UTYPE_PLAYERDAMAGEMODS: castPtr<Player>(this)->UpdatePlayerDamageDoneMods(); break;
    case UF_UTYPE_PLAYERRATINGS: castPtr<Player>(this)->UpdatePlayerRatings(); break;
    case UF_UTYPE_MOVEMENT: m_movementInterface.ProcessModUpdate(modUpdateType, modMap); break;
    }
}

class StatValueCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_STAT:
            {
                bool positive = mod->m_amount > 0;
                if(mod->m_miscValue[1] == -1)
                {
                    for(uint8 s = 0; s < MAX_STAT; ++s)
                    {
                        if(positive)
                            statPos[s] += mod->m_amount;
                        else statNeg[s] -= mod->m_amount;
                    }
                }
                else if(mod->m_miscValue[1] < MAX_STAT)
                {
                    if(positive)
                        statPos[mod->m_miscValue[1]] += mod->m_amount;
                    else statNeg[mod->m_miscValue[1]] -= mod->m_amount;
                }
            }break;
        case SPELL_AURA_MOD_PERCENT_STAT:
            {
                if(mod->m_miscValue[1] == -1)
                {
                    for(uint8 s = 0; s < MAX_STAT; ++s)
                    {
                        if(mod->m_amount > 0)
                            modPos[s] += mod->m_amount;
                        else modNeg[s] -= mod->m_amount;
                    }
                }
                else if(mod->m_miscValue[1] < MAX_STAT)
                {
                    if(mod->m_amount > 0)
                        modPos[mod->m_miscValue[1]] += mod->m_amount;
                    else modNeg[mod->m_miscValue[1]] -= mod->m_amount;
                }
            }break;
        case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
            {
                if(mod->m_miscValue[1] == -1)
                {
                    for(uint8 s = 0; s < MAX_STAT; ++s)
                    {
                        if(mod->m_amount > 0)
                            modTPos[s] += mod->m_amount;
                        else modTNeg[s] -= mod->m_amount;
                    }
                }
                else if(mod->m_miscValue[1] < MAX_STAT)
                {
                    if(mod->m_amount > 0)
                        modTPos[mod->m_miscValue[1]] += mod->m_amount;
                    else modTNeg[mod->m_miscValue[1]] -= mod->m_amount;
                }
            }break;
        }
    }

    void Init(Unit *unit, UnitBaseStats *stats, uint32 level, bool strClass, float ctrMod)
    {
        for(uint8 s = 0; s < MAX_STAT; s++)
        {
            statPos[s] = statNeg[s] = 0;
            modPos[s] = modNeg[s] = modTPos[s] = modTNeg[s] = 0.f;

            statBase[s] = stats ? stats->baseStat[s] : level*25;
            if(stats && stats->level < level)
                statBase[s] += (level-stats->level)*15;
            if(unit->IsCreature() && s == STAT_STRENGTH)
                statBase[s] *= float2int32(ctrMod * (strClass ? 1.f : 0.5f));
            else if(unit->IsCreature() && s == STAT_AGILITY)
                statBase[s] *= float2int32(ctrMod * (strClass ? 0.5f : 1.0f));
            statBase[s] += unit->GetBonusStat(s);
        }
    }

    int32 GetStatValue(uint8 stat, int32 &posOut, int32 &negOut)
    {
        int32 base = statBase[stat];
        int32 statMod = (posOut = statPos[stat]) - (negOut = statNeg[stat]);
        if(statMod <= 0)
            statMod = 0;
        else
        {
            posOut += float2int32(statMod*(modPos[stat]/100.f));
            negOut += float2int32(statMod*(modNeg[stat]/100.f));
        }

        int32 fullStat = base + (posOut - negOut);
        if(fullStat > 0)
        {
            posOut += float2int32(statMod*(modTPos[stat]/100.f));
            negOut += float2int32(statMod*(modTNeg[stat]/100.f));
        }

        return base + (posOut - negOut);
    }

    int32 statBase[MAX_STAT];
    int32 statPos[MAX_STAT], statNeg[MAX_STAT];
    float modPos[MAX_STAT], modNeg[MAX_STAT], modTPos[MAX_STAT], modTNeg[MAX_STAT];
};

void Unit::UpdateStatValues()
{
    uint32 _class = getClass(), Level = getLevel();
    float ctrMod = std::max<float>(1.f, ((float)Level)/fMaxLevelSqrt);
    bool strClass = _class == WARRIOR || _class == PALADIN || _class == DEATHKNIGHT;

    gtFloat *HPPerStam = NULL;
    if(IsCreature() && (HPPerStam = dbcHPPerStam.LookupEntry((getClass()-1)*MAXIMUM_ATTAINABLE_LEVEL+(getLevel()-1))))
    {
        ctrMod *= 1.f + std::max<float>(0.f, (HPPerStam->val-fMaxLevelSqrt));
        if(uint32 levelMod = sStatSystem.GetXPackModifierForLevel(getLevel(), castPtr<Creature>(this)->GetCreatureData()->rank > 0 ? 3 : 0))
            ctrMod *= ((float)(levelMod+1))/2.f;
    }

    StatValueCallback valueCallback;
    valueCallback.Init(this, baseStats, Level, strClass, ctrMod);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_STAT, &valueCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_PERCENT_STAT, &valueCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, &valueCallback);

    for(uint8 s = 0; s < MAX_STAT; s++)
    {
        int32 pos, neg;
        SetUInt32Value(UNIT_FIELD_STATS+s, valueCallback.GetStatValue(s, pos, neg));
        SetUInt32Value(UNIT_FIELD_POSSTATS+s, pos);
        SetUInt32Value(UNIT_FIELD_NEGSTATS+s, neg);
    }

    TriggerModUpdate(UF_UTYPE_HEALTH);
    TriggerModUpdate(UF_UTYPE_POWER);
    TriggerModUpdate(UF_UTYPE_ATTACKPOWER);
    TriggerModUpdate(UF_UTYPE_RANGEDATTACKPOWER);

    // Set stat values as updated to update affected auras
    if(m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT)) TriggerModUpdate(UF_UTYPE_RESISTANCE);
    if(IsPlayer()) TriggerModUpdate(UF_UTYPE_PLAYERDAMAGEMODS);
    if(IsPlayer()) TriggerModUpdate(UF_UTYPE_PLAYERRATINGS);
}

class HealthUpdateCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_BASE_HEALTH_PCT:
            bonusBaseHP += baseHp * (((float)abs(mod->m_amount))/100.f);
            break;
        case SPELL_AURA_MOD_INCREASE_HEALTH:
        case SPELL_AURA_MOD_INCREASE_MAX_HEALTH:
        case SPELL_AURA_MOD_INCREASE_HEALTH_2:
            bonusHp += mod->m_amount;
            break;
        case SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT:
            bonusHp += Hp * (((float)abs(mod->m_amount))/100.f);
            break;
        }
    }

    void Init(Unit *unit, UnitBaseStats *stats)
    {
        bonusBaseHP = bonusHp = 0;
        baseHp = stats ? stats->baseHP->val*unit->GetHealthMod() : 20;
        int32 stam = unit->GetStat(STAT_STAMINA), baseStam = stam < 20 ? stam : 20;
        if(unit->IsCreature() && (castPtr<Creature>(unit)->isCritter() || castPtr<Creature>(unit)->isTrainingDummy()))
            baseStam = 1;

        stam = (stam <= baseStam ? 0 : stam-baseStam);
        int32 stamHp = baseStam;
        if(gtFloat *HPPerStam = dbcHPPerStam.LookupEntry((unit->getClass()-1)*MAXIMUM_ATTAINABLE_LEVEL+(unit->getLevel()-1)))
            stamHp += stam*(HPPerStam->val*unit->GetHealthMod());
        // Set base
        Hp = baseHp + stamHp + unit->GetBonusHealth();
    }

    uint32 GetBaseHP() { return baseHp + bonusBaseHP; }
    uint32 GetHP() { return baseHp + bonusBaseHP + Hp + bonusHp; }

    uint32 Hp, baseHp, bonusBaseHP, bonusHp;
};

void Unit::UpdateHealthValues()
{
    HealthUpdateCallback healthCallback;
    healthCallback.Init(this, baseStats);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_BASE_HEALTH_PCT, &healthCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_INCREASE_HEALTH, &healthCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_INCREASE_MAX_HEALTH, &healthCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_INCREASE_HEALTH_2, &healthCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT, &healthCallback);

    // Set values from calculated results
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, healthCallback.GetBaseHP());
    if(GetUInt32Value(UNIT_FIELD_HEALTH) > healthCallback.GetHP())
        SetUInt32Value(UNIT_FIELD_HEALTH, healthCallback.GetHP());
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, healthCallback.GetHP());
}

class PowerUpdateCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        if(_basePowers.find(mod->m_miscValue[0]) == _basePowers.end())
            return;

        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_INCREASE_ENERGY:
            _bonusPowers[mod->m_miscValue[0]] += mod->m_amount;
            break;
        case SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT:
            _bonusPowers[mod->m_miscValue[0]] += _basePowers[mod->m_miscValue[0]] * (((float)abs(mod->m_amount))/100.f);
            break;
        }
    }

    bool Init(Unit *unit, UnitBaseStats *baseStats, std::vector<uint8> *classPower, uint32 *basePowers)
    {
        bool hasMana = false;
        for(std::vector<uint8>::iterator itr = classPower->begin(); itr != classPower->end(); itr++)
        {
            if((*itr) > POWER_TYPE_RUNIC)
                continue;

            uint32 basePower = basePowers[*itr];
            // Set mana base value
            if((*itr) == POWER_TYPE_MANA)
            {
                uint32 baseMana = baseStats ? baseStats->baseMP->val*unit->GetPowerMod() : 0;
                unit->SetUInt32Value(UNIT_FIELD_BASE_MANA, basePower+baseMana);

                int32 intellect = unit->GetStat(STAT_INTELLECT), baseIntellect = intellect < 20 ? intellect : 20;
                intellect = intellect <= baseIntellect ? 0 : intellect-baseIntellect;
                basePower += baseMana + unit->GetBonusMana() + baseIntellect + intellect*15.f*unit->GetPowerMod();
                hasMana = true;
            }

            _basePowers.insert(std::make_pair(*itr, basePower));
            _bonusPowers.insert(std::make_pair(*itr, 0));
        }
        return hasMana;
    }

    void IncrementBPower(uint8 powerType, uint32 baseIncr) { _basePowers[powerType] += baseIncr; }
    uint32 GetPower(uint8 powerType) { return _basePowers[powerType] + _bonusPowers[powerType]; }

    std::map<uint8, uint32> _basePowers, _bonusPowers;
};

static uint32 basePowerValues[POWER_TYPE_MAX] = { 0, 1000, 100, 100, 1050000, 0xFF, 1000, 3, 100, 3 };
void Unit::UpdatePowerValues()
{
    PowerUpdateCallback powerCallback;
    std::vector<uint8> *classPower = sStatSystem.GetUnitPowersForClass(getClass());
    if(classPower->size() == 1 && POWER_TYPE_MANA != *classPower->begin())
        SetPowerType(*classPower->begin());

    bool hasMana = powerCallback.Init(this, baseStats, classPower, basePowerValues);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_INCREASE_ENERGY, &powerCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT, &powerCallback);
    for(std::vector<uint8>::iterator itr = classPower->begin(); itr != classPower->end(); itr++)
    {
        uint8 powerType = *itr;
        uint32 power = powerCallback.GetPower(powerType);
        if(powerType == POWER_TYPE_RUNE)
            SetPower(powerType, power = basePowerValues[powerType]);

        if(GetPower(powerType) > power)
            SetPower(powerType, power);
        SetMaxPower(powerType, power);
    }

    if(hasMana) TriggerModUpdate(UF_UTYPE_REGEN);
}

class RegenUpdateCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_POWER_REGEN:
            if(mod->m_miscValue[0] != _powerType)
                return;
            _baseRegen += mod->m_amount;
            break;
        case SPELL_AURA_MOD_POWER_REGEN_PERCENT:
            if(mod->m_miscValue[0] != _powerType)
                return;
            _regenMod += _regenVal * ((float)mod->m_amount);
            break;
        case SPELL_AURA_MOD_MANA_REGEN_INTERRUPT:
            _interruptMod += mod->m_amount;
            if (_interruptMod > 100) _interruptMod = 100;
            break;
        }
    }

    void Init(uint8 powerType, float baseRegen, float regenVal, int32 interruptMod)
    {
        _powerType = powerType;
        _baseRegen = baseRegen;
        _regenVal = regenVal, _regenMod = 0.f;
        _interruptMod = interruptMod;
    }
    
    float getBaseRegen() { return _baseRegen; }
    float getRegenValue() { return _regenVal + _regenMod; }
    int32 getInterruptMod() { return _interruptMod; }

    uint8 _powerType;
    float _baseRegen, _regenVal, _regenMod;
    int32 _interruptMod;
};

static uint32 baseRegenValues[POWER_TYPE_MAX] = { 0, 10, 5, 5, 250, 10, 0, 0, 0, 0 };
void Unit::UpdateRegenValues()
{
    float base_regen = 0.01f;
    RegenUpdateCallback regenCallback;
    if(base_regen *= GetUInt32Value(UNIT_FIELD_BASE_MANA))
    {
        float spirit_regen = sqrt(GetStat(STAT_INTELLECT));

        uint32 level = std::min(getLevel(), uint32(100));
        if(gtFloat *ratio = dbcManaRegenBase.LookupEntry((getClass()-1)*100+level-1))
            spirit_regen *= GetStat(STAT_SPIRIT)*ratio->val;

        regenCallback.Init(POWER_TYPE_MANA, base_regen, spirit_regen, 1);
        m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_POWER_REGEN, &regenCallback);
        m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_POWER_REGEN_PERCENT, &regenCallback);
        m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT, &regenCallback);

        SetFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER, regenCallback.getBaseRegen() + 0.001f + regenCallback.getRegenValue());
        SetFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER, regenCallback.getBaseRegen() + (regenCallback.getRegenValue() * (regenCallback.getInterruptMod() / 100.0f)));
    }

    if(uint8 powerType = getPowerType())
    {
        if(base_regen = baseRegenValues[powerType])
        {
            regenCallback.Init(powerType, base_regen, 0.f, 1);
            m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_POWER_REGEN, &regenCallback);
            m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_POWER_REGEN_PERCENT, &regenCallback);
            m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT, &regenCallback);

            SetFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER+powerType, regenCallback.getRegenValue()+regenCallback.getBaseRegen() + 0.001f);
            SetFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER+powerType, regenCallback.getRegenValue()+float2int32(floor(regenCallback.getBaseRegen() * (regenCallback.getInterruptMod() / 100.f))));
        }
    }
}

class AttackTimeUpdateCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_ATTACKSPEED:
            for(uint8 i = 0; i < 3; ++i)
                attackSpeedMod[i] += ((float)abs(mod->m_amount))/100.f;
            break;
        case SPELL_AURA_MOD_MELEE_HASTE:
            attackSpeedMod[MELEE] += ((float)abs(mod->m_amount))/100.f;
            attackSpeedMod[OFFHAND] += ((float)abs(mod->m_amount))/100.f;
            break;
        case SPELL_AURA_MOD_RANGED_HASTE:
            attackSpeedMod[RANGED] += ((float)abs(mod->m_amount))/100.f;
            break;
        }
    }

    void Init(Unit *unit)
    {
        for(uint8 i = 0; i < 3; ++i)
            attackSpeedMod[i] = 1.f;
    }

    float getAttackSpeedMod(uint8 index) { return attackSpeedMod[index]; }

    float attackSpeedMod[3];
};

void Unit::UpdateAttackTimeValues()
{
    AttackTimeUpdateCallback attackTimeCallback;
    attackTimeCallback.Init(this);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_ATTACKSPEED, &attackTimeCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_MELEE_HASTE, &attackTimeCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_RANGED_HASTE, &attackTimeCallback);

    uint8 updateMask = 0x00;
    for(uint8 i = 0; i < 3; i++)
    {
        uint32 baseAttack = GetBaseAttackTime(i), attackTime = GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+i);
        if(baseAttack || i == 0) // Force an attack time for mainhand
        {
            float attackSpeedMod = attackTimeCallback.getAttackSpeedMod(i);
            if(IsPlayer() && i == RANGED) attackSpeedMod *= (1.f+(castPtr<Player>(this)->CalcRating(PLAYER_RATING_MODIFIER_RANGED_HASTE)/100.f));
            else if(IsPlayer()) attackSpeedMod *= (1.f+(castPtr<Player>(this)->CalcRating(PLAYER_RATING_MODIFIER_MELEE_HASTE)/100.f));

            baseAttack = std::min<uint32>(12000, std::max<uint32>(500, float2int32(floor(float(baseAttack)/attackSpeedMod))));
        }

        SetUInt32Value(UNIT_FIELD_BASEATTACKTIME+i, baseAttack);
        // Only update our attack timer if we've changed our attack time
        if(baseAttack == attackTime)
            continue;

        updateMask |= 1<<i;
    }

    if(updateMask > 0)
    {
        resetAttackDelay(updateMask);
        TriggerModUpdate(UF_UTYPE_ATTACKDAMAGE);
    }
}

class AttackDamageUpdateCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_DAMAGE_DONE:
            if((mod->m_miscValue[0] & 0x01) == 0)
                return;

            for(uint8 i = 0; i < 3; ++i)
                flatMod[i] += mod->m_amount;
            break;
        case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:
            {
                if(mod->m_spellInfo->EquippedItemClass != -1)
                {
                    for(uint8 i = 0; i < 3; ++i)
                        if(equippedWpnMask[i] == mod->m_spellInfo->EquippedItemClass)
                            pctMod[i] += mod->m_amount;
                    return;
                }

                if((mod->m_miscValue[0] & 0x01) == 0)
                    return;
                for(uint8 i = 0; i < 3; ++i)
                    pctMod[i] += mod->m_amount;
            }break;
        }
    }

    void Init(Unit *unit)
    {
        Item *item = NULL;
        for(uint32 i = 0; i < 3; ++i)
        {
            flatMod[i] = 0;
            pctMod[i] = 100.f;
            equippedWpnMask[i] = -1;
            if(unit->IsPlayer() && (item = castPtr<Player>(unit)->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+i)))
                equippedWpnMask[i] = item->GetProto()->SubClass;
        }
    }

    int32 getFlatMod(uint8 index) { return flatMod[index]; }
    float getPctMod(uint8 index) { return pctMod[index]; }

    int32 flatMod[3];
    float pctMod[3];
    int32 equippedWpnMask[3];
};

static uint32 minAttackDamages[3] = { UNIT_FIELD_MINDAMAGE, UNIT_FIELD_MINOFFHANDDAMAGE, UNIT_FIELD_MINRANGEDDAMAGE };
void Unit::UpdateAttackDamageValues()
{
    AttackDamageUpdateCallback attackDamageCallback;
    attackDamageCallback.Init(this);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_DAMAGE_DONE, &attackDamageCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, &attackDamageCallback);

    uint32 attackPower = CalculateAttackPower(), rangedAttackPower = CalculateRangedAttackPower();
    for(uint8 i = 0; i < 3; i++)
    {
        if(GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+i) == 0)
        {
            SetFloatValue(minAttackDamages[i], 0);
            SetFloatValue(minAttackDamages[i]+1, 0);
            continue;
        }

        float apBonus = ((float)GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+i))/1000.f * ((float)(i == RANGED ? rangedAttackPower/14.f : attackPower/14.f));
        float baseMinDamage = GetBaseMinDamage(i), baseMaxDamage = GetBaseMaxDamage(i);
        if(IsCreature())
        {   // Creature damage is AP times healthmod, with 1.5+rank times being the max damage
            CreatureData *data = castPtr<Creature>(this)->GetCreatureData();
            uint32 levelMod = sStatSystem.GetXPackModifierForLevel(getLevel(), data->rank > 0 ? 3 : 0);
            baseMaxDamage = std::max<float>(2.f, apBonus * (1.f + (((float)data->rank) * 0.5f)) * (levelMod ? ((float)(levelMod+1)) : 1.f));
            baseMaxDamage = ceil(baseMaxDamage * data->damageMod);
            baseMinDamage = std::max<float>(1.f, floor(baseMaxDamage * ((6.5f + ((float)data->rank+1) + data->damageRangeMod)/10.f)));
        }
        else if(IsInFeralForm())
        {
            Item *currentWeapon = NULL;
            if(IsPlayer() && !disarmed && (currentWeapon = castPtr<Player>(this)->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)) != NULL)
            {
                float modifier = ((float)currentWeapon->GetProto()->Delay)/1000.f;
                if(GetShapeShift() == FORM_BEAR || GetShapeShift() == FORM_DIREBEAR)
                {
                    baseMaxDamage = std::max<float>(2.f, baseMaxDamage / modifier + (baseMaxDamage/2.5f));
                    baseMinDamage = std::max<float>(1.f, baseMinDamage / modifier + (baseMinDamage/2.5f));
                }
                else
                {
                    baseMaxDamage = std::max<float>(2.f, baseMaxDamage / modifier);
                    baseMinDamage = std::max<float>(1.f, baseMinDamage / modifier);
                }
            }
            else 
            {
                baseMinDamage += apBonus;
                baseMaxDamage += apBonus;
            }
        }
        else // Add our AP bonus
        {
            baseMinDamage += apBonus;
            baseMaxDamage += apBonus;
        }

        // Add up our damage done modificiation auras
        baseMinDamage += attackDamageCallback.getFlatMod(i);
        baseMaxDamage += attackDamageCallback.getFlatMod(i);

        // Percentage based damage done auras
        baseMinDamage *= (attackDamageCallback.getPctMod(i)/100.f);
        baseMaxDamage *= (attackDamageCallback.getPctMod(i)/100.f);

        if(i == OFFHAND) // Offhand weapons do 50% of actual damage, dual wield spec they do 75%
        {
            float modifier = HasDummyAura(SPELL_HASH_DUAL_WIELD_SPECIALIZATION) ? 0.75f : 0.5f;
            baseMinDamage *= modifier;
            baseMaxDamage *= modifier;
        }

        SetFloatValue(minAttackDamages[i], baseMinDamage);
        SetFloatValue(minAttackDamages[i]+1, baseMaxDamage);
    }
}

class ResistanceUpdateCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        for(uint8 s = 0; s < MAX_RESISTANCE; ++s)
        {
            switch(mod->m_type)
            {
            case SPELL_AURA_MOD_RESISTANCE:
                {
                    if(mod->m_miscValue[0] & (1 << s))
                    {
                        if(mod->m_amount > 0)
                            resPos[s] += mod->m_amount;
                        else resNeg[s] -= mod->m_amount;
                    }
                }break;
            case SPELL_AURA_MOD_RESISTANCE_PCT:
                {
                    if(mod->m_miscValue[0] & (1 << s))
                    {
                        if(mod->m_amount >= 0)
                            bonusPos[s] += mod->m_amount;
                        else bonusNeg[s] -= mod->m_amount;
                    }
                }break;
            case SPELL_AURA_MOD_BASE_RESISTANCE:
                {
                    if(mod->m_miscValue[0] & (1 << s))
                    {
                        if(mod->m_amount > 0)
                            basePos[s] += mod->m_amount;
                        else baseNeg[s] -= mod->m_amount;
                    }
                }break;
            case SPELL_AURA_MOD_BASE_RESISTANCE_PCT:
                {
                    if(mod->m_miscValue[0] & (1 << s))
                    {
                        if(mod->m_amount >= 0)
                            baseBonusPos[s] += mod->m_amount;
                        else baseBonusNeg[s] -= mod->m_amount;
                    }
                }break;
            case SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE:
                {
                    if(mod->m_miscValue[0] & (1 << s))
                        excluseRes[s] = std::max<int32>(excluseRes[s], mod->m_amount);
                }break;
            case SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT:
                {
                    if(mod->m_miscValue[0] & (1 << s))
                        resFromStat[s][mod->m_miscValue[1]] += mod->m_amount;
                }break;
            }
        }
    }

    void Init(Unit *unit)
    {
        for(uint8 s = 0; s < MAX_STAT; ++s)
            statCache[s] = unit->GetStat(s);
        for(uint8 i = 0; i < MAX_RESISTANCE; ++i)
        {
            startRes[i] = unit->GetBonusResistance(i);
            basePos[i] = resPos[i] = resNeg[i] = baseNeg[i] = excluseRes[i] = 0;
            baseBonusPos[i] = baseBonusNeg[i] = bonusPos[i] = bonusNeg[i] = 0.f;
            for(uint8 s = 0; s < MAX_STAT; ++s)
                resFromStat[i][s] = 0;
        }
    }

    int32 CalcResistance(uint8 res, int32 &posOut, int32 &negOut)
    {
        // Get our base resist: pos + neg
        int32 baseRes = startRes[res]+basePos[res]-baseNeg[res];

        // Multiply base by % modifiers
        if(baseRes <= 0) // If base is zero or negative, just set it to zero
            baseRes = 0; // Else modify base by % values
        else
        {
            int32 resist = baseRes;
            baseRes += float2int32(resist*(baseBonusPos[res]/100.f));
            baseRes -= float2int32(resist*(baseBonusNeg[res]/100.f));
        }

        int32 modResist = (posOut = resPos[res]) + (negOut = resNeg[res]);
        if(modResist > 0)
        {
            posOut += float2int32(modResist*(bonusPos[res]/100.f));
            negOut += float2int32(modResist*(bonusNeg[res]/100.f));
        }

        for(uint8 i = 0; i < MAX_STAT; ++i)
            posOut += resFromStat[res][i] * statCache[i];
        if(posOut < excluseRes[res])
            posOut = excluseRes[res];
        return baseRes + std::max<int32>(0, posOut - negOut);
    }

    int32 startRes[MAX_RESISTANCE], basePos[MAX_RESISTANCE], baseNeg[MAX_RESISTANCE], resPos[MAX_RESISTANCE], resNeg[MAX_RESISTANCE], excluseRes[MAX_RESISTANCE];
    float baseBonusPos[MAX_RESISTANCE], baseBonusNeg[MAX_RESISTANCE], bonusPos[MAX_RESISTANCE], bonusNeg[MAX_RESISTANCE];
    int32 statCache[MAX_STAT], resFromStat[MAX_RESISTANCE][MAX_STAT];
};

void Unit::UpdateResistanceValues()
{
    ResistanceUpdateCallback resistanceCallback;
    resistanceCallback.Init(this);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_RESISTANCE, &resistanceCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_RESISTANCE_PCT, &resistanceCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_BASE_RESISTANCE, &resistanceCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_BASE_RESISTANCE_PCT, &resistanceCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE, &resistanceCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT, &resistanceCallback);

    for(uint8 s = 0; s < MAX_RESISTANCE; s++)
    {
        int32 pos, neg;
        SetUInt32Value(UNIT_FIELD_RESISTANCES+s, resistanceCallback.CalcResistance(s, pos, neg));
        SetUInt32Value(UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+s, pos);
        SetUInt32Value(UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+s, neg);
    }
}

class AttackPowerUpdateCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_ATTACK_POWER_PCT:
            attackPowerMod += ((float)mod->m_amount);
            break;
        case SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR:
            {
                float modAmt = mod->m_spellInfo->EffectBasePoints[mod->i+1], reqAmount;
                if((reqAmount = modAmt * mod->m_amount) > 0.f && resist_cache > reqAmount)
                    attackPowerBonus += float2int32(modAmt*floor(resist_cache/reqAmount));
            }break;
        }
    }

    bool Init(Unit *unit, std::vector<uint32> *modMap)
    {
        bool ret = false;
        attackPowerBonus = 0;
        resist_cache = unit->GetUInt32Value(UNIT_FIELD_RESISTANCES);
        attackPowerMod = unit->GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER) * 100.f;
        if(std::find(modMap->begin(), modMap->end(), SPELL_AURA_MOD_ATTACK_POWER_PCT) != modMap->end())
        {
            attackPowerMod = 100.f;
            ret = true;
        }

        return ret;
    }

    int32 GetAttackPowerBonus() { return attackPowerBonus; }
    float GetAttackPowerMod() { return attackPowerMod/100.f; }

    float resist_cache;
    int32 attackPowerBonus;
    float attackPowerMod;
};

void Unit::UpdateAttackPowerValues(std::vector<uint32> modMap)
{
    AttackPowerUpdateCallback attackPowerCallback;
    if(attackPowerCallback.Init(this, &modMap))
        m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_ATTACK_POWER_PCT, &attackPowerCallback);
    m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR, &attackPowerCallback);

    uint8 _class = getClass();
    int32 attackPower = GetBonusAttackPower();
    switch(_class)
    {
    case DRUID: { attackPower += std::max<uint32>(30, GetStrength() * 2) - 20; }break;
    case HUNTER: case ROGUE: case SHAMAN: { attackPower += std::max<uint32>(30, GetStrength()+GetAgility()+getLevel()*2)-20; }break;
    case WARRIOR: case DEATHKNIGHT: case PALADIN: { attackPower += std::max<uint32>(30, GetStrength()*2+getLevel()*3)-20; }break;
    default: { attackPower += std::max<uint32>(20, GetAgility()) - 10; }break;
    }

    attackPower += attackPowerCallback.GetAttackPowerBonus();

    SetUInt32Value(UNIT_FIELD_ATTACK_POWER, attackPower);
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_POS, 0);
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_NEG, 0);
    SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, attackPowerCallback.GetAttackPowerMod());

    TriggerModUpdate(UF_UTYPE_ATTACKDAMAGE);
    if(IsPlayer())
        TriggerModUpdate(UF_UTYPE_PLAYERDAMAGEMODS);
}

class RangedAttackPowerUpdateCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT:
            rangedAttackPowerMod += ((float)mod->m_amount);
            break;
        }
    }

    bool Init(Unit *unit, std::vector<uint32> *modMap)
    {
        bool ret = false;
        rangedAttackPowerMod = unit->GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER) * 100.f;
        if(std::find(modMap->begin(), modMap->end(), SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT) != modMap->end())
        {
            rangedAttackPowerMod = 100.f;
            ret = true;
        }

        return ret;
    }

    float GetRangedAttackPowerMod() { return rangedAttackPowerMod/100.f; }

    float rangedAttackPowerMod;
};

void Unit::UpdateRangedAttackPowerValues(std::vector<uint32> modMap)
{
    RangedAttackPowerUpdateCallback rangedAttackPowerCallback;
    if(rangedAttackPowerCallback.Init(this, &modMap))
        m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_ATTACK_POWER_PCT, &rangedAttackPowerCallback);

    int32 rangedAttackPower = GetBonusRangedAttackPower();
    switch (getClass())
    {
    case WARRIOR: case DEATHKNIGHT: case ROGUE: { rangedAttackPower += getLevel()+GetAgility()-10; }break;
    case HUNTER: { rangedAttackPower += getLevel()*2+GetAgility()-10; }break;
    default: { rangedAttackPower += GetAgility()-10; }break;
    }

    gtFloat *HPPerStam = NULL;
    if(IsCreature() && (HPPerStam = dbcHPPerStam.LookupEntry((getClass()-1)*MAXIMUM_ATTAINABLE_LEVEL+(getLevel()-1))))
        rangedAttackPower *= 1.f + std::max<float>(0.f, (HPPerStam->val-fMaxLevelSqrt));

    SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER, rangedAttackPower);
    SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS, 0);
    SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG, 0);
    SetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER, rangedAttackPowerCallback.GetRangedAttackPowerMod());

    TriggerModUpdate(UF_UTYPE_ATTACKDAMAGE);
    if(IsPlayer()) TriggerModUpdate(UF_UTYPE_PLAYERDAMAGEMODS);
}

class PowerCostUpdateCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_POWER_COST_SCHOOL:
            {
                for(uint8 s = 0; s < MAX_RESISTANCE; s++)
                {
                    if((mod->m_miscValue[0] & (1<<s)) == 0)
                        continue;
                    costModifier[s] += mod->m_amount;
                }
            }break;
        case SPELL_AURA_MOD_POWER_COST:
            {
                for(uint8 s = 0; s < MAX_RESISTANCE; s++)
                {
                    if((mod->m_miscValue[0] & (1<<s)) == 0)
                        continue;
                    costMultiplier[s] += mod->m_amount;
                }
            }break;
        }
    }

    void Init(Unit *unit, uint32 &updateFlag, std::vector<uint32> *modMap)
    {
        if(std::find(modMap->begin(), modMap->end(), SPELL_AURA_MOD_POWER_COST_SCHOOL) != modMap->end())
            updateFlag |= 0x01;

        if(std::find(modMap->begin(), modMap->end(), SPELL_AURA_MOD_POWER_COST) != modMap->end())
            updateFlag |= 0x02;

        for(uint8 s = 0; s < MAX_RESISTANCE; s++)
        {
            costModifier[s] = 0;
            costMultiplier[s] = 100.f;

            if((updateFlag & 0x01) == 0)
                costModifier[s] += unit->GetUInt32Value(UNIT_FIELD_POWER_COST_MODIFIER+s);
            if((updateFlag & 0x02) == 0)
                costMultiplier[s] *= unit->GetFloatValue(UNIT_FIELD_POWER_COST_MODIFIER+s);
        }
    }

    float GetCostModifier(uint8 school) { return costModifier[school]; }
    float GetCostMultipler(uint8 school) { return costMultiplier[school]/100.f; }

    uint32 costModifier[MAX_RESISTANCE];
    float costMultiplier[MAX_RESISTANCE];
};

void Unit::UpdatePowerCostValues(std::vector<uint32> modMap)
{
    uint32 updateFlag;
    PowerCostUpdateCallback powerCostCallback;
    powerCostCallback.Init(this, updateFlag, &modMap);

    if(updateFlag & 0x01) m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_POWER_COST_SCHOOL, &powerCostCallback);
    if(updateFlag & 0x02) m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_POWER_COST, &powerCostCallback);

    for(uint8 s = 0; s < MAX_RESISTANCE; s++)
    {
        SetUInt32Value(UNIT_FIELD_POWER_COST_MODIFIER+s,powerCostCallback.GetCostModifier(s));
        SetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+s,0.f);
    }
}

class HoverValueUpdateCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_HOVER: { height += mod->m_amount; }break;
        }
    }

    void Init() { height = 0.002f; }

    float GetHeight() { return height/2.f; }

    float height;
};

void Unit::UpdateHoverValues()
{
    HoverValueUpdateCallback hoverValueCallback;
    hoverValueCallback.Init();
    m_AuraInterface.TraverseModMap(SPELL_AURA_HOVER, &hoverValueCallback);

    SetFloatValue(UNIT_FIELD_HOVERHEIGHT, hoverValueCallback.GetHeight());
}

class DamageDoneModCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_DAMAGE_DONE:
            break;
        case SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT:
            break;
        case SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER:
            break;
        }
    }

    void Init(Unit *unit, uint8 school)
    {
        for(uint8 s = 0; s < MAX_STAT; ++s)
            statCache[s] = unit->GetStat(s);
    }

    uint32 statCache[MAX_STAT];

};

int32 Unit::GetDamageDoneMod(uint8 school, bool forceCalc, int32 *negativeOut)
{
    int32 res = 0;
    if(IsPlayer() && forceCalc == false)
    {
        res += GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+school);
        res -= GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG+school);
        return res;
    }
    else if(IsSummon())
    {
        WorldObject *summoner = castPtr<Summon>(this)->GetSummonOwner();
        /// To avoid processing loops, do not count pets of pets or summons of summons
        if(!summoner->IsSummon() && GetsDamageBonusFromOwner(school))
            res += castPtr<Unit>(summoner)->GetDamageDoneMod(school);
    }

    DamageDoneModCallback DamageDoneCallback;
    DamageDoneCallback.Init(this, school);
    if(AuraInterface::modifierMap *damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DAMAGE_DONE))
        for(AuraInterface::modifierMap::iterator itr = damageMod->begin(); itr != damageMod->end(); itr++)
            if(itr->second->m_miscValue[0] & (1<<school))
                res += itr->second->m_amount;

    if(school != SCHOOL_NORMAL)
    {
        if(AuraInterface::modifierMap *damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT))
        {
            float statMods[5] = {0,0,0,0,0};
            for(AuraInterface::modifierMap::iterator itr = damageMod->begin(); itr != damageMod->end(); itr++)
                if(itr->second->m_miscValue[0] & (1<<school))
                    statMods[itr->second->m_miscValue[1]] += float(itr->second->m_amount)/100.f;
            for(uint8 i = 0; i < 5; i++)
                if(statMods[i])
                    res += statMods[i]*GetStat(i);
        }

        if(AuraInterface::modifierMap *damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER))
        {
            float attackPowerMod = 0.0f;
            for(AuraInterface::modifierMap::iterator itr = damageMod->begin(); itr != damageMod->end(); itr++)
                if(itr->second->m_miscValue[0] & (1<<school))
                    attackPowerMod += float(itr->second->m_amount)/100.f;
            res += float2int32(float(CalculateAttackPower())*attackPowerMod);
        }
    }
    return res;
}

class HealingDoneModCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_HEALING_DONE:
            break;
        case SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT:
            break;
        case SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER:
            break;
        }
    }

    void Init(Unit *unit, uint8 school)
    {
        for(uint8 s = 0; s < MAX_STAT; ++s)
            statCache[s] = unit->GetStat(s);
    }

    uint32 statCache[MAX_STAT];

    uint32 statModPos;
};

int32 Unit::GetHealingDoneMod(bool forceCalc, int32 *negativeOut)
{
    // If we're a player, this is already precalculated
    if(IsPlayer() && forceCalc == false)
        return GetUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS);

    int32 result = 0;
    if(AuraInterface::modifierMap *healingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_HEALING_DONE))
        for(AuraInterface::modifierMap::iterator itr = healingMod->begin(); itr != healingMod->end(); itr++)
            result += itr->second->m_amount;

    if(AuraInterface::modifierMap *healingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT))
    {
        float statMods[5] = {0,0,0,0,0};
        for(AuraInterface::modifierMap::iterator itr = healingMod->begin(); itr != healingMod->end(); itr++)
            statMods[itr->second->m_miscValue[1]] += float(itr->second->m_amount)/100.f;
        for(uint8 i = 0; i < 5; i++)
            if(statMods[i])
                result += statMods[i]*GetStat(i);
    }

    if(AuraInterface::modifierMap *healingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER))
    {
        float attackPowerMod = 0.0f;
        for(AuraInterface::modifierMap::iterator itr = healingMod->begin(); itr != healingMod->end(); itr++)
            attackPowerMod += float(itr->second->m_amount)/100.f;
        result += float2int32(float(CalculateAttackPower())*attackPowerMod);
    }
    return result;
}

float Unit::GetDamageDonePctMod(uint8 school, bool forceCalc)
{
    // If we're a player, this is already precalculated
    if(IsPlayer() && forceCalc == false)
        return GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+school);

    float result = 1.f;
    if(AuraInterface::modifierMap *damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE))
        for(AuraInterface::modifierMap::iterator itr = damageMod->begin(); itr != damageMod->end(); itr++)
            if(itr->second->m_miscValue[0] & (1<<school))
                result += itr->second->m_amount;
    return result;
}

float Unit::GetHealingDonePctMod(bool forceCalc)
{
    // If we're a player, this is already precalculated
    if(IsPlayer() && forceCalc == false)
        return GetFloatValue(PLAYER_FIELD_MOD_HEALING_PCT);

    float result = 1.f;
    if(AuraInterface::modifierMap *healingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_HEALING_DONE_PERCENT))
        for(AuraInterface::modifierMap::iterator itr = healingMod->begin(); itr != healingMod->end(); itr++)
            result += itr->second->m_amount;
    return result;
}

uint32 Unit::GetMechanicDispels(uint8 mechanic)
{
    AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_ADD_CREATURE_IMMUNITY);
    uint32 count = modMap ? modMap->size() : 0;
    if( mechanic == 16 || mechanic == 19 || mechanic == 25 || mechanic == 31 )
        count = 0;
    if(modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MECHANIC_IMMUNITY))
    {
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if(itr->second->m_miscValue[0] == mechanic) count++;
    }

    if(mechanic == MECHANIC_POLYMORPHED && GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID))
        count++;
    return count;
}

float Unit::GetMechanicResistPCT(uint8 mechanic)
{
    float resist = 0.0f;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MECHANIC_RESISTANCE))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if(itr->second->m_miscValue[0] == mechanic)
                resist += itr->second->m_amount;
    return resist;
}

float Unit::GetDamageTakenByMechPCTMod(uint8 mechanic)
{
    float resist = 0.0f;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if(itr->second->m_miscValue[0] == mechanic)
                resist += float(itr->second->m_amount)/100.f;
    return resist;
}

float Unit::GetMechanicDurationPctMod(uint8 mechanic)
{
    float resist = 1.f;
    AuraInterface::modifierMap *modMap = NULL;
    if(modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MECHANIC_DURATION_MOD))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if(itr->second->m_miscValue[0] == mechanic) resist *= float(itr->second->m_amount)/100.f;

    if(modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK))
    {
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
        {
            if(itr->second->m_miscValue[0] == mechanic)
            {
                float val = float(itr->second->m_amount)/100.f;
                if(resist < val)
                    resist = val;
            }
        }
    }

    if(modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL_NOT_STACK))
    {
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
        {
            if(itr->second->m_miscValue[0] == mechanic)
            {
                float val = float(itr->second->m_amount)/100.f;
                if(resist < val)
                    resist = val;
            }
        }
    }
    return resist;
}

uint32 Unit::GetDispelImmunity(uint8 dispel)
{
    uint32 count = 0;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_DISPEL_IMMUNITY))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if(itr->second->m_miscValue[0] == dispel) count++;
    return count;
}

float Unit::GetDispelResistancesPCT(uint8 dispel)
{
    float resist = 0.0f;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DEBUFF_RESISTANCE))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if(itr->second->m_miscValue[0] == dispel)
                resist += itr->second->m_amount;
    return resist;
}

int32 Unit::GetCreatureRangedAttackPowerMod(uint32 creatureType)
{
    if(creatureType == 0)
        return 0;

    int32 mod = 0;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if(itr->second->m_miscValue[0] & (1<<creatureType-1))
                mod += itr->second->m_amount;
    return mod;
}

int32 Unit::GetCreatureAttackPowerMod(uint32 creatureType)
{
    if(creatureType == 0)
        return 0;

    int32 mod = 0;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_CREATURE_ATTACK_POWER))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if(itr->second->m_miscValue[0] & (1<<creatureType-1))
                mod += itr->second->m_amount;
    return mod;
}

int32 Unit::GetRangedDamageTakenMod()
{
    int32 mod = 0;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            mod += itr->second->m_amount;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            mod *= itr->second->m_amount;
    return mod;
}

float Unit::GetCritMeleeDamageTakenModPct(uint32 school)
{
    float mod = 0.f;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if (itr->second->m_miscValue[0] & (1<<school))
                mod += itr->second->m_amount;
    return mod;
}

float Unit::GetCritRangedDamageTakenModPct(uint32 school)
{
    float mod = 0.f;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if (itr->second->m_miscValue[0] & (1<<school))
                mod += itr->second->m_amount;
    return mod;
}

int32 Unit::GetDamageTakenMod(uint32 school)
{
    int32 mod = 0;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DAMAGE_TAKEN))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if (itr->second->m_miscValue[0] & (1<<school))
                mod += itr->second->m_amount;
    if(school == 0)
    {
        if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN))
            for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
                mod += itr->second->m_amount;
    }
    return mod;
}

float Unit::GetDamageTakenModPct(uint32 school)
{
    float mod = 1.f;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            if (itr->second->m_miscValue[0] & (1<<school))
                mod += float(itr->second->m_amount)/100.f;
    if(school == 0)
    {
        if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT))
            for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
                mod += itr->second->m_amount;
    }
    return mod;
}

float Unit::GetAreaOfEffectDamageMod()
{
    float mod = 1.f;
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            mod *= float(itr->second->m_amount)/100.f;
    return mod;
}

bool Unit::canWalk()
{
    if(IsCreature())
    {
        Creature* ctr = castPtr<Creature>(this);
        if(ctr->GetCanMove() & LIMIT_ANYWHERE)
            return true;
        if(ctr->GetCanMove() & LIMIT_GROUND)
            return true;
    } else if(IsPlayer())
        return true;
    return false;
}

bool Unit::canSwim()
{
    if(IsCreature())
    {
        Creature* ctr = castPtr<Creature>(this);
        if(ctr->GetCanMove() & LIMIT_ANYWHERE)
            return true;
        if(ctr->GetCanMove() & LIMIT_WATER)
            return true;
    } else if(IsPlayer())
        return true;
    return false;
}

bool Unit::canFly()
{
    if(IsVehicle())
        return false;
    else if(IsCreature())
    {
        Creature* ctr = castPtr<Creature>(this);
        if(ctr->GetCanMove() & LIMIT_ANYWHERE)
            return true;
        if(ctr->GetCanMove() & LIMIT_AIR)
            return true;
    }

    if(m_AuraInterface.HasAurasWithModType(SPELL_AURA_FLY))
        return true;
    if(m_AuraInterface.HasFlightAura())
        return true;

    return false;
}

WeaponDamageType Unit::GetPreferredAttackType(SpellEntry **sp)
{
    if(false)
    {
        if(sp) *sp = NULL;
        return RANGED;
    }

    return MELEE;
}

bool Unit::validateAttackTarget(WorldObject *target)
{
    if(target == nullptr || !target->IsUnit())
        return false;
    else if(!sFactionSystem.CanEitherUnitAttack(this, castPtr<Unit>(target), false))
        return false;

    return true;
}

bool Unit::calculateAttackRange(WeaponDamageType type, float &minRange, float &maxRange, SpellEntry *sp)
{
    float selfReach = GetFloatValue(UNIT_FIELD_COMBATREACH);
    minRange = 0.f, maxRange = selfReach + GetModelHalfSize();
    if((type == RANGED || type == RANGED_AUTOSHOT) && sp) // Sanity check
    {
        // Min range for our ranged attack spell
        minRange = sp->minRange[0];
        minRange *= minRange;
        // Calculate max range
        maxRange -= (type == RANGED_AUTOSHOT ? 0.f : selfReach);
        float spellRange = sp->maxRange[0];
        if( sp->SpellGroupType )
        {
            SM_FFValue(SMT_RANGE, &spellRange, sp->SpellGroupType );
            SM_PFValue(SMT_RANGE, &spellRange, sp->SpellGroupType );
        }
        maxRange += spellRange;
    }

    return true;
}

bool Unit::canReachWithAttack(WeaponDamageType attackType, Unit* pVictim, uint32 spellId)
{
    if(GetMapId() != pVictim->GetMapId())
        return false;

    SpellEntry *sp = NULL;
    if((spellId || attackType == RANGED || attackType == RANGED_AUTOSHOT) && (sp = dbcSpell.LookupEntry(spellId)) == NULL)
        return false;

    float minRange = 0.f, maxRange = pVictim->GetModelHalfSize(), distance = GetDistanceSq(pVictim);
    if(!calculateAttackRange(attackType, minRange, maxRange, sp))
        return false;

    //formula adjustment for player side.
    if(IsPlayer())
    {
        // latency compensation!!
        // figure out how much extra distance we need to allow for based on our movespeed and latency.
        MovementInterface *move;
        if((move = pVictim->GetMovementInterface()) && move->isMoving())
        {
            // this only applies to PvP.
            uint32 lat = pVictim->IsPlayer() ? castPtr<Player>(pVictim)->GetSession()->GetLatency() : 0;

            // if we're over 500 get fucked anyway.. your gonna lag! and this stops cheaters too
            lat = (lat > 500) ? 500 : lat;

            // calculate the added distance
            maxRange += move->GetMoveSpeed(MOVE_SPEED_RUN) * 0.001f * lat;
        }

        if((move = GetMovementInterface()) && move->isMoving())
        {
            // this only applies to PvP.
            uint32 lat = castPtr<Player>(this)->GetSession() ? castPtr<Player>(this)->GetSession()->GetLatency() : 0;

            // if we're over 500 get fucked anyway.. your gonna lag! and this stops cheaters too
            lat = (lat > 500) ? 500 : lat;

            // calculate the added distance
            maxRange += move->GetMoveSpeed(MOVE_SPEED_RUN) * 0.001f * lat;
        }
    }
    return (distance > (minRange*minRange)) && (distance <= (maxRange*maxRange));
}

void Unit::resetAttackTimer(uint8 attackType)
{
    m_attackTimer[attackType] = m_attackDelay[attackType];
}

void Unit::resetAttackDelay(uint8 typeMask)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if((typeMask & (1<<i)) == 0)
            continue;

        uint32 baseAttackTime = GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+i);
        if(baseAttackTime == 0)
        {
            if(i == OFFHAND)
                m_dualWield = false;
            m_attackDelay[i] = 0;
            continue;
        }

        m_attackDelay[i] = std::max<uint32>(1000, std::min<uint32>(0x7FFF, std::ceil(float(baseAttackTime)*GetCastSpeedMod())));
        if(i == OFFHAND && m_attackDelay[i] > 0)
            m_dualWield = true;
    }
}

void Unit::QueueExtraAttacks(uint32 spellId, uint32 amount)
{
    for(uint32 i = 0; i < amount; i++)
        m_extraAttacks.push_back(spellId);
}

float Unit::ModDetectedRange(Unit *detector, float base)
{
    // "The maximum Aggro Radius has a cap of 25 levels under. Example: A level 35 char has the same Aggro Radius of a level 5 char on a level 60 mob."
    uint32 lvl = getLevel(), dlvl = detector->getLevel(), lvlDiff = lvl < dlvl ? dlvl-lvl : lvl-dlvl;
    // "Aggro Radius varies with level difference at a rate of roughly 1 yard/level"
    // radius grow if playlevel < creaturelevel
    if(lvl < dlvl)
        base += float(std::min<int32>(25, dlvl-lvl))*1.115f;
    else base -= float(std::min<int32>(25, lvl-dlvl))*0.985f;
    // Check detected modifiers auras that exist on us
    if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DETECTED_RANGE))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            base += itr->second->m_amount;
    // "Minimum Aggro Radius for a mob seems to be combat range (5 yards)"
    return std::max<float>(5.f, base);
}

void Unit::SetDiminishTimer(uint32 index)
{
    assert(index < DIMINISH_GROUPS);

}

void Unit::setLevel(uint32 level)
{
    TriggerModUpdate(UF_UTYPE_STATS);
    m_AuraInterface.OnChangeLevel(level);
    SetUInt32Value(UNIT_FIELD_LEVEL, level);
    if((baseStats = sStatSystem.GetUnitBaseStats(getRace(), getClass(), level)) == NULL)
        if(baseStats = sStatSystem.GetMaxUnitBaseStats(getRace(), getClass()))
            if(baseStats->level > level)
                baseStats = NULL;
}

void Unit::GiveGroupXP(Unit* pVictim, Player* PlayerInGroup)
{
    if(!PlayerInGroup)
        return;
    if(!pVictim)
        return;
    if(!PlayerInGroup->InGroup())
        return;
    Group *pGroup = PlayerInGroup->GetGroup();
    uint32 xp = 0;
    if(!pGroup)
        return;

    bool givesGuildXP = pVictim->IsCreature() ? pGroup->QualifiesForGuildXP(castPtr<Creature>(pVictim)) : NULL;

    //Get Highest Level Player, Calc Xp and give it to each group member
    Player *pHighLvlPlayer = NULL, *pGroupGuy = NULL, *active_player_list[MAX_GROUP_SIZE_RAID];//since group is small we can afford to do this ratehr then recheck again the whole active player set
    int active_player_count=0, total_level=0;
    float xp_mod = 1.0f;

    //change on 2007 04 22 by Zack
    //we only take into count players that are near us, on same map
    GroupMembersSet::iterator itr;
    pGroup->Lock();
    for(uint32 i = 0; i < pGroup->GetSubGroupCount(); i++)
    {
        for(itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); itr++)
        {
            pGroupGuy = (*itr)->m_loggedInPlayer;
            if( pGroupGuy && pGroupGuy->isAlive()
                && pVictim->GetMapInstance() == pGroupGuy->GetMapInstance()
                && pGroupGuy->GetDistanceSq(pVictim)<100*100 )
            {
                active_player_list[active_player_count] = pGroupGuy;
                active_player_count++;
                total_level += pGroupGuy->getLevel();
                if(pHighLvlPlayer)
                {
                    if(pGroupGuy->getLevel() > pHighLvlPlayer->getLevel())
                        pHighLvlPlayer = pGroupGuy;
                } else pHighLvlPlayer = pGroupGuy;
            }
        }
    }
    pGroup->Unlock();
    if(active_player_count<1) //killer is always close to the victim. This should never execute
    {
        if(PlayerInGroup == 0)
        {
            PlayerInfo * pleaderinfo = pGroup->GetLeader();
            if(!pleaderinfo->m_loggedInPlayer)
                return;

            PlayerInGroup = pleaderinfo->m_loggedInPlayer;
        }

        xp = CalculateXpToGive(pVictim, PlayerInGroup, pVictim->GetMapInstance()->GetZoneModifier(pVictim->GetZoneId()));
        PlayerInGroup->GiveXP(xp, pVictim->GetGUID(), true, givesGuildXP);
    }
    else
    {
        if( pGroup->GetGroupType() == GROUP_TYPE_PARTY)
        {
            if(active_player_count==3)
                xp_mod=1.1666f;
            else if(active_player_count==4)
                xp_mod=1.3f;
            else if(active_player_count==5)
                xp_mod=1.4f;
            else xp_mod=1;//in case we have only 2 members ;)
        }
        else if(pGroup->GetGroupType() == GROUP_TYPE_RAID)
            xp_mod=0.5f;

        if(pHighLvlPlayer == 0)
        {
            PlayerInfo * pleaderinfo = pGroup->GetLeader();
            if(!pleaderinfo->m_loggedInPlayer)
                return;

            pHighLvlPlayer = pleaderinfo->m_loggedInPlayer;
        }

        xp = CalculateXpToGive(pVictim, pHighLvlPlayer, pVictim->GetMapInstance()->GetZoneModifier(pVictim->GetZoneId()));
        //i'm not sure about this formula is correct or not. Maybe some brackets are wrong placed ?
        for(int i=0;i<active_player_count;++i)
            active_player_list[i]->GiveXP( float2int32(((xp*active_player_list[i]->getLevel()) / total_level)*xp_mod), pVictim->GetGUID(), true, givesGuildXP );
    }
}

bool Unit::IsInInstance()
{
    if(m_mapInstance && m_mapInstance->IsInstance())
        return true;
    return false;
}

uint32 roll_results[5] =
{
    SPELL_DID_HIT_MISS,
    SPELL_DID_HIT_DODGE,
    SPELL_DID_HIT_PARRY,
    SPELL_DID_HIT_BLOCK,
    SPELL_DID_HIT_SUCCESS
};

uint32 Unit::GetSpellDidHitResult( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability )
{
    Item* it = NULL;
    bool backAttack = pVictim->isTargetInBack(this);
    float hitchance = 100.f, dodge = 0.f, parry = 0.f, block = 0.f;
    float advantage = CalculateAdvantage(pVictim, hitchance, dodge, parry, block, NULL, NULL, NULL);

    //--------------------------------by victim state-------------------------------------------
    if(pVictim->IsPlayer()&&pVictim->GetStandState()) //every not standing state is >0
        hitchance = 100.0f;

    if( ability && ability->SpellGroupType )
    {
        SM_FFValue( SMT_HITCHANCE, &hitchance, ability->SpellGroupType );
        SM_PFValue( SMT_HITCHANCE, &hitchance, ability->SpellGroupType );
    }

    // overpower nana
    if( ability != NULL && ability->isUnstoppableForce() )
        dodge = 0.0f, parry = 0.0f, block = 0.0f;

    //==========================================================================================
    //==============================One Roll Processing=========================================
    //==========================================================================================
    //--------------------------------cummulative chances generation----------------------------
    uint32 r = 0;
    float chances[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    chances[0] = std::max(0.0f, std::max(0.0f, 100.0f-std::min<float>(100.f, hitchance)));
    if(!backAttack)
    {
        chances[1] = chances[0]+dodge;
        chances[2] = chances[1]+parry;
        chances[3] = chances[2]+block;
    } else if(pVictim->IsCreature())
        chances[1] = chances[0]+dodge;

    //--------------------------------roll------------------------------------------------------
    float Roll = RandomFloat(100.0f);
    while (r < 4 && Roll > chances[r])
        r++;
    return roll_results[r];
}

uint32 Unit::GetSpellDidHitResult( Unit* pVictim, BaseSpell* pSpell, float *resistOut, uint8 *reflectout )
{
    SpellEntry* m_spellEntry = pSpell->GetSpellProto();
    int32 levelDiff = pVictim->getLevel() - getLevel();
    if(resistOut) *resistOut = 0.f;
    if(reflectout) *reflectout = SPELL_DID_HIT_MISS;

    // Melee spells ignore hit checks because of previously called melee hit checks
    if(!m_spellEntry->IsSpellMeleeSpell())
    {
        // Calculate our spell miss chance
        float hitChance = 100.f, baseMiss[3] = { 4.0f, 5.0f, 6.0f };
        if(levelDiff >= 3)
            hitChance -= baseMiss[2] + ((levelDiff-2) * 11.f);
        else if(levelDiff >= 0.f)
            hitChance -= baseMiss[levelDiff];

        if(m_spellEntry->SpellGroupType)
        {
            SM_FFValue(SMT_HITCHANCE, &hitChance, m_spellEntry->SpellGroupType);
            SM_PFValue(SMT_HITCHANCE, &hitChance, m_spellEntry->SpellGroupType);
        }

        //rating bonus
        if( IsPlayer() )
            hitChance += castPtr<Player>(this)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_HIT );

        // 160: Mod AOE avoidance implementation needed.

        // Roll our hit chance
        if(Rand(hitChance) == false)
            return SPELL_DID_HIT_MISS;
    }

    /************************************************************************/
    /* Check if the spell is resisted.                                    */
    /************************************************************************/
    // Holy and physical spells cannot be resisted, ranged spells are always considered physical
    if( m_spellEntry->School == SCHOOL_HOLY || m_spellEntry->School == SCHOOL_NORMAL || m_spellEntry->isSpellRangedSpell() )
        return SPELL_DID_HIT_SUCCESS;
    if( m_spellEntry->isIgnorantOfHitResult() )
        return SPELL_DID_HIT_SUCCESS;
    // Spell reflect chance occurs before full and partial resist
    if(pSpell->Reflect(pVictim))
    {
        if(reflectout) *reflectout = SPELL_DID_HIT_SUCCESS;
        return SPELL_DID_HIT_REFLECT;
    }

    float resistChance = 0.f;
    if(uint32 targetResist = pVictim->GetResistance(m_spellEntry->School))
    {
        // targetResist -= std::min<uint32>(targetResist, GetSpellPenetration());
        // SPELL_AURA_MOD_TARGET_RESISTANCE
        resistChance += (100.f * ((float)targetResist)/(std::max<float>(20.f, getLevel()) * 5.f) * 0.75f);
    }

    // We get a resist chance to get our partial resist off, and if we pass that we also have a small chance to 100% resist depending on cap
    bool resisted = Rand(std::min<float>(75.f, resistChance)); // You have about a 5% chance to resist 100% of damage at resist cap
    if(resisted && (Spell::IsBinary(m_spellEntry) || Rand(5.f * std::min<float>(75.f, resistChance)/75.f)) && !(m_spellEntry->isUnstoppableForce2() || m_spellEntry->isSpellResistanceIgnorant()))
        return SPELL_DID_HIT_RESIST;

    // In cataclysm, partial resists no longer occur against bosses
    if(resisted && resistOut && !(IsCreature() && castPtr<Creature>(this)->isBoss()))
    {
        // Resist chance is from a pool of 4 values, so calculate our chance to get one of these values leaning heavier towards the cap above our 100%
        int stack = 0, chanceStack[4];
        chanceStack[3] = floor(resistChance/18.f);
        if(resistChance > 100.f) stack += chanceStack[3];
        chanceStack[2] = floor(resistChance/(12.f + stack));
        if(resistChance > 100.f) stack += chanceStack[2];
        chanceStack[1] = floor(resistChance/(7.f + stack));
        chanceStack[0] = std::max<int>(2, 2*(75.f/resistChance));
        int chanceCount = chanceStack[3] + chanceStack[2] + chanceStack[1] + chanceStack[0];
        // We have our slots for each roll allocated, but convert into chance blocks to reduce calculation overhead
        float chances[4];
        chances[0] = 100.f*((float)chanceStack[0])/((float)chanceCount);
        chances[1]=chances[0]+100.f*((float)chanceStack[1])/((float)chanceCount);
        chances[2]=chances[1]+100.f*((float)chanceStack[2])/((float)chanceCount);
        chances[3]=chances[2]+100.f*((float)chanceStack[3])/((float)chanceCount);
        // Roll against our chance blocks to see what we got
        float Roll = RandomFloat(100.0f);
        uint32 r = 3;
        while (r > 0 && Roll < chances[r-1])
            r--;

        // Since we have our results, now we calculate what partial resist % we got
        float result[4] = { 0.f, 0.f, 0.f, 0.f }; // { 0.f, 25.f, 50.f, 75.f }; 
        if(true) // Wotlk+ chances
        {
            float cap = std::min<float>(75.f, resistChance);
            float mitStep = ceil(cap/10.f), mitModifier = ceil(std::max(1.f, cap-5.f)/10.f);
            if(mitStep == mitModifier)
                mitStep += 1.f;
            result[3] = 10.f * (std::min<float>(10, mitStep));
            result[2] = std::max(0.f, result[3] - 10.f);
            result[1] = std::max(0.f, result[2] - 10.f);
            result[0] = std::max(0.f, result[1] - 10.f);
        } // Burning Crusade chances
        else result[1] = 25.f, result[2] = 50.f, result[3] = 75.f;
        *resistOut = result[r];
    }

    return SPELL_DID_HIT_SUCCESS;
}

static float hitModifierPerLevelDiff[21] = { /*-10 to -6*/-40.f, -35.f, -30.f, -25.f, -20.f, /*-5 to -1*/ -10.f, -5.f, -3.f, -1.f, -0.5f, /*0 and 1*/0.f, 0.f, /*2 to 5*/0.5f, 1.f, 2.5f, 5.f, /*5 to 10*/7.5f, 10.f, 12.5f, 15.f, 20.f };
float Unit::CalculateAdvantage(Unit *pVictim, float &hitchance, float &dodgechance, float &parrychance, float &blockchance, float *critChance, float *crushingBlow, float *glancingBlow)
{
    float level = getLevel(), vLevel = pVictim->getLevel(), rawDiff = level-vLevel, advantage = vLevel/level;
    rawDiff = floor(std::max<float>(-10.f, std::min<float>(10.f, rawDiff)));
    uint32 defense = pVictim->IsPlayer() ? castPtr<Player>(pVictim)->getSkillLineVal(SKILL_DEFENSE) : vLevel*5;

    hitchance -= pVictim->GetDodgeChance();
    hitchance += hitModifierPerLevelDiff[10+uint8(rawDiff)];
    hitchance += GetHitChance();

    if(critChance)
        *critChance += (-rawDiff/2.f);
    if(crushingBlow)
        *crushingBlow = std::max<float>(0.f, rawDiff >= 4 ? (-15.f+advantage*(1.f+RandomFloat(1.f))) : 0.f);
    if(glancingBlow)
        *glancingBlow = std::max<float>(0.f, 10.f*advantage);
    return std::min<float>(advantage, 5.f);
/*
    //--------------------------------crushing blow chance--------------------------------------
    if(pVictim->IsPlayer()&&!IsPlayer()&&!ability && !dmg.school_type)
    {
        int32 LevelDiff = GetUInt32Value(UNIT_FIELD_LEVEL) - pVictim->GetUInt32Value(UNIT_FIELD_LEVEL);
        if(diffVcapped>=15.0f && LevelDiff >= 4)
            crush = -15.0f+2.0f*diffVcapped;
        else
            crush = 0.0f;
    }
    //--------------------------------glancing blow chance--------------------------------------
    if(IsPlayer()&&!pVictim->IsPlayer()&&!ability)
    {
        glanc = 10.0f + diffAcapped;
        if(glanc<0)
            glanc = 0.0f;
    }
    //==========================================================================================
    //==============================Advanced Chances Modifications==============================
    //==========================================================================================
    //--------------------------------by talents------------------------------------------------
    if(pVictim->IsPlayer())
    {
        if( weapon_damage_type != RANGED )
        {
            crit += TO_PLAYER(pVictim)->res_M_crit_get();
            hitmodifier += TO_PLAYER(pVictim)->m_resist_hit[0];
        }
        else
        {
            crit += TO_PLAYER(pVictim)->res_R_crit_get(); //this could be ability but in that case we overwrite the value
            hitmodifier += TO_PLAYER(pVictim)->m_resist_hit[1];
        }
    }
    crit += (float)(pVictim->AttackerCritChanceMod[0]);
    //--------------------------------by skill difference---------------------------------------
    float vsk = (float)self_skill - (float)victim_skill;
    dodge = std::max( 0.0f, dodge - vsk * 0.04f );
    if( parry )
        parry = std::max( 0.0f, parry - vsk * 0.04f );
    if( block )
        block = std::max( 0.0f, block - vsk * 0.04f );

    crit += pVictim->IsPlayer() ? vsk * 0.04f : min( vsk * 0.2f, 0.0f );

    if( pVictim->IsPlayer() )
    {
        if( vsk > 0 )
            hitchance = std::max( hitchance, 95.0f + vsk * 0.02f);
        else
            hitchance = std::max( hitchance, 95.0f + vsk * 0.04f);
    } else
    {
        if(vsk >= -10 && vsk <= 10)
            hitchance = std::max( hitchance, 95.0f + vsk * 0.1f);
        else
            hitchance = std::max( hitchance, 93.0f + (vsk - 10.0f) * 0.4f);
    }
    //--------------------------------by ratings------------------------------------------------
    crit -= pVictim->IsPlayer() ? TO_PLAYER(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE ) : 0.0f;
    if(crit < 0.0f)
        crit = 0.0f;

    if (IsPlayer())
    {
        if(weapon_damage_type == RANGED)
            hitmodifier += TO_PLAYER(this)->CalcRating( PLAYER_RATING_MODIFIER_RANGED_HIT );
        else
        {
            hitmodifier += TO_PLAYER(this)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_HIT );

            float expertise_bonus = 0.0f;
            if(weapon_damage_type == MELEE)
                expertise_bonus += GetUInt32Value(PLAYER_EXPERTISE)*0.25f;
            else if(weapon_damage_type == OFFHAND)
                expertise_bonus += GetUInt32Value(PLAYER_OFFHAND_EXPERTISE)*0.25f;

            if(dodge < expertise_bonus)
                dodge = 0.0f;
            else dodge -= expertise_bonus;

            if(parry < expertise_bonus)
                parry = 0.0f;
            else parry -= expertise_bonus;
        }
    }

    //--------------------------------by damage type and by weapon type-------------------------
    if( weapon_damage_type == RANGED )
    {
        dodge = 0.0f;
        parry = 0.0f;
        glanc = 0.0f;
    }
    else if(IsPlayer())
    {
        it = TO_PLAYER(this)->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
        if( it != NULL && (it->GetProto()->InventoryType == INVTYPE_WEAPON ||
            it->GetProto()->InventoryType == INVTYPE_2HWEAPON) && !ability )//dualwield to-hit penalty
        {
            hitmodifier -= 19.0f;
        }
    }

    hitchance += hitmodifier;*/
}

void Unit::Strike( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability, uint8 effIndex, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool proc_extrastrike )
{
//==========================================================================================
//==============================Unacceptable Cases Processing===============================
//==========================================================================================
    if(!pVictim->isAlive() || !isAlive()  || IsStunned() || IsPacified() || IsFeared())
        return;

    // Strike requires that our target be in front of us, but some spells like Divine storm use strike in AOE form
    if(!isTargetInFront(pVictim) && (ability == NULL || !SpellTargetClass::isSpellAOEStrikeable(ability, effIndex)))
    {
        if(IsPlayer())
            castPtr<Player>(this)->PushData(SMSG_ATTACKSWING_BADFACING);
        return;
    }

    if(pVictim->hasStateFlag(UF_EVADING))
    {
        SendAttackerStateUpdate(pVictim, NULL, 0, 0, 0, HITSTATUS_SWINGNOHITSOUND, EVADE);
        return;
    }
//==========================================================================================
//==============================Variables Initialization====================================
//==========================================================================================
    dealdamage dmg            = {0,0,0};

    Item* it = NULL;

    float armorreducepct    = 0.f;

    float hitchance         = 100.f;
    float dodge             = 0.0f;
    float parry             = 0.0f;
    float glanc             = 0.0f;
    float block             = 0.0f;
    float crit              = 5.f;
    float crush             = 0.0f;

    uint32 targetEvent      = 0;
    uint32 hit_status       = 0;

    uint32 blocked_damage   = 0;
    int32  realdamage       = 0;

    uint32 vstate           = 1;

    float hitmodifier       = 0;
    uint32 SubClassSkill    = SKILL_UNARMED;

    bool backAttack         = pVictim->isTargetInBack(this);
    bool disable_dR         = false;

    dmg.school_type = SCHOOL_NORMAL;
    dmg.resisted_damage = 0;
    if(ability)
        dmg.school_type = ability->School;
    else if (GetTypeId() == TYPEID_UNIT)
        dmg.school_type = castPtr<Creature>(this)->GetCreatureData()->attackType;

    if( weapon_damage_type != RANGED )
    {
        //--------------------------------block chance----------------------------------------------
        if (!pVictim->disarmedShield) // If we have no shield, block percentage is 0%, even with aura mods
            block = pVictim->IsPlayer() ? pVictim->GetFloatValue(PLAYER_BLOCK_PERCENTAGE) : 0.f;
        //--------------------------------dodge chance----------------------------------------------
        if (!pVictim->IsStunned())
            dodge = pVictim->IsPlayer() ? pVictim->GetFloatValue(PLAYER_DODGE_PERCENTAGE) : 0.f;
        //--------------------------------parry chance----------------------------------------------
        if (!disarmed)
            parry = pVictim->IsPlayer() ? pVictim->GetFloatValue(PLAYER_PARRY_PERCENTAGE) : 0.f;
        crit = IsPlayer() ? GetFloatValue(PLAYER_CRIT_PERCENTAGE) : (pVictim->GetUInt32Value(UNIT_FIELD_AGILITY)/std::max<float>(50.f, (pVictim->getLevel()*1.5f)));
    } else crit = IsPlayer() ? GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE) : (pVictim->GetUInt32Value(UNIT_FIELD_AGILITY)/std::max<float>(50.f, (pVictim->getLevel()*1.5f)));

    float advantage = CalculateAdvantage(pVictim, hitchance, dodge, parry, block, &crit, &crush, &glanc);
    // Crushing blows only from creatures and physical attacks
    if(IsPlayer() || ability || dmg.school_type)
        crush = 0.f;
    // Glancing blows can only happen from players to creatures and on physicals
    if(!IsPlayer() || pVictim->IsPlayer() || ability || dmg.school_type)
        glanc = 0.f;

//-----------------------Advantage modifier code-------------------------------
    if(ability && ability->SpellGroupType)
    {
        SM_FFValue(SMT_CRITICAL,&crit,ability->SpellGroupType);
        SM_PFValue(SMT_CRITICAL,&crit,ability->SpellGroupType);
        SM_FFValue(SMT_HITCHANCE,&hitchance,ability->SpellGroupType);
        SM_PFValue(SMT_HITCHANCE,&hitchance,ability->SpellGroupType);
    }

    //Hackfix for Surprise Attacks
    if( IsPlayer() && ability && castPtr<Player>(this)->m_AuraInterface.HasAurasWithModType(SPELL_AURA_IGNORE_COMBAT_RESULT) && ability->isSpellFinishingMove())
        dodge = 0.0f;

    if( skip_hit_check )
        hitchance = 100.0f, dodge = parry = block = 0.0f;

    if( ability != NULL )
    {
        if( ability->NameHash == SPELL_HASH_FEROCIOUS_BITE && HasDummyAura( SPELL_HASH_REND_AND_TEAR ) && pVictim->m_AuraInterface.HasNegAuraWithMechanic(MECHANIC_BLEEDING) )
            crit += 5.0f * GetDummyAura(SPELL_HASH_REND_AND_TEAR )->RankNumber;
        else if( pVictim->IsStunned() && ability->Id == 20467 )
            crit = 100.0f;
        else if( ability->isUnstoppableForce() )
            dodge = parry = block = 0.0f;
    }

//--------------------------------by victim state-------------------------------------------
    if(pVictim->IsPlayer() && pVictim->GetStandState()) //every not standing state is >0
    {
        hitchance = crit = 100.0f;
        dodge = parry = block = crush = 0.f;
    }
    else if(backAttack)
    {
        if(pVictim->IsPlayer())
            dodge = 0.0f;
        parry = block = 0.0f;
    }

//==========================================================================================
//==============================One Roll Processing=========================================
//==========================================================================================
//--------------------------------cummulative chances generation----------------------------
    float chances[7];
    chances[0]=std::max(0.0f, 100.0f-std::min<float>(100.f, hitchance));
    chances[1]=chances[0]+dodge;
    chances[2]=chances[1]+parry;
    chances[3]=chances[2]+glanc;
    chances[4]=chances[3]+block;
    chances[5]=chances[4]+crit;
    chances[6]=chances[5]+crush;
//--------------------------------roll------------------------------------------------------
    float Roll = RandomFloat(100.0f);
    uint32 r = 0;
    while (r < 7 && Roll > chances[r])
        r++;

//--------------------------------postroll processing---------------------------------------
    uint32 abs = 0;
    switch(r)
    {
//--------------------------------miss------------------------------------------------------
    case 0:
        hit_status |= HITSTATUS_MISS;
        break;
//--------------------------------dodge-----------------------------------------------------
    case 1:
        TRIGGER_AI_EVENT(pVictim, OnTargetDodged)(castPtr<Unit>(this));
        TRIGGER_AI_EVENT(castPtr<Unit>(this), OnDodged)(castPtr<Unit>(this));
        targetEvent = 1;
        vstate = DODGE;
        pVictim->Emote(EMOTE_ONESHOT_PARRY_UNARMED);         // Animation
        break;
//--------------------------------parry-----------------------------------------------------
    case 2:
        TRIGGER_AI_EVENT(pVictim, OnTargetParried)(castPtr<Unit>(this));
        TRIGGER_AI_EVENT(castPtr<Unit>(this), OnParried)(castPtr<Unit>(this));
        targetEvent = 3;
        vstate = PARRY;
        pVictim->Emote(EMOTE_ONESHOT_PARRY_UNARMED);         // Animation
        break;
//--------------------------------not miss,dodge or parry-----------------------------------
    default:
        hit_status |= HITSTATUS_HITANIMATION;//hit animation on victim
        {
//--------------------------------base damage calculation-----------------------------------
            if(exclusive_damage)
                dmg.full_damage = exclusive_damage;
            else dmg.full_damage = sStatSystem.CalculateDamage( this, pVictim, weapon_damage_type, ability );

            if( weapon_damage_type == RANGED )
                dmg.full_damage += pVictim->GetRangedDamageTakenMod();

            // Bonus damage
            if( ability == NULL )
            {   // Bonus 
                dmg.full_damage += GetDamageDoneMod(SCHOOL_NORMAL);
                dmg.full_damage *= pVictim->GetDamageTakenModPct(SCHOOL_NORMAL);
            }

            if( ability && ability->MechanicsType == MECHANIC_BLEEDING )
                disable_dR = true;
            else if(ability == NULL || (ability->SchoolMask & spellMaskArray[SCHOOL_NORMAL]))
            {
                float armor = ((float)pVictim->GetResistance(SCHOOL_NORMAL));
                // Armor pen and ignore modifiers
                ////TODO

                if(IsPlayer())
                {
                    float maxArmorPen = 0;
                    if (pVictim->getLevel() < 60)
                        maxArmorPen = float(400 + 85 * pVictim->getLevel());
                    else maxArmorPen = 400 + 85 * pVictim->getLevel() + 4.5f * 85 * (pVictim->getLevel() - 59);

                    // Cap armor penetration to this number
                    maxArmorPen = std::min((armor + maxArmorPen) / 3, armor);
                    armor -= std::min(maxArmorPen, ((maxArmorPen * ((float)castPtr<Player>(this)->GetUInt32Value(PLAYER_RATING_MODIFIER_ARMOR_PENETRATION_RATING)))/100.f));
                }

                float levelModifier = getLevel();
                if (levelModifier > 59)
                    levelModifier = levelModifier + 4.5f * (levelModifier - 59);

                float modifiedArmor = 0.1f * armor / (8.5f * levelModifier + 40);
                dmg.full_damage *= (1.0f - std::max<float>(0.f, std::min<float>(0.75f, modifiedArmor / (1.0f + modifiedArmor))));
            }

            if( HasDummyAura(SPELL_HASH_REND_AND_TEAR) && ability &&
                ( ability->NameHash == SPELL_HASH_MAUL || ability->NameHash == SPELL_HASH_SHRED) && pVictim->m_AuraInterface.HasNegAuraWithMechanic(MECHANIC_BLEEDING) )
            {
                dmg.full_damage += float2int32(dmg.full_damage * ( ( GetDummyAura(SPELL_HASH_REND_AND_TEAR)->RankNumber * 4 ) / 100.f ) );
            }

            if(dmg.full_damage < 0)
                dmg.full_damage = 0;
//--------------------------------check for special hits------------------------------------
            switch(r)
            {
//--------------------------------glancing blow---------------------------------------------
            case 3:
                {
                    float low_dmg_mod = 1.5f - (0.5f * advantage);
                    if(getClass() == MAGE || getClass() == PRIEST || getClass() == WARLOCK) //casters = additional penalty.
                        low_dmg_mod -= 0.7f;
                    if(low_dmg_mod<0.01)
                        low_dmg_mod = 0.01f;
                    if(low_dmg_mod>0.91)
                        low_dmg_mod = 0.91f;
                    float high_dmg_mod = 1.2f - (0.3f * advantage);
                    if(getClass() == MAGE || getClass() == PRIEST || getClass() == WARLOCK) //casters = additional penalty.
                        high_dmg_mod -= 0.3f;
                    if(high_dmg_mod>0.99)
                        high_dmg_mod = 0.99f;
                    if(high_dmg_mod<0.2)
                        high_dmg_mod = 0.2f;

                    float damage_reduction = (high_dmg_mod + low_dmg_mod) / 2.0f;
                    if(damage_reduction > 0)
                        dmg.full_damage = float2int32(damage_reduction * float(dmg.full_damage));
                    hit_status |= HITSTATUS_GLANCING;
                }
                break;
//--------------------------------block-----------------------------------------------------
            case 4:
                {
                    Item* shield = castPtr<Player>( pVictim )->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if( shield != NULL && !pVictim->disarmedShield )
                    {
                        targetEvent = 2;
                        blocked_damage = 0;
                        pVictim->Emote(EMOTE_ONESHOT_PARRY_SHIELD);// Animation

                        if( shield->GetProto()->InventoryType == INVTYPE_SHIELD )
                        {
                            float block_multiplier = 1.f;
                            if( block_multiplier < 1.f )
                                block_multiplier = 1.f;

                            blocked_damage = pVictim->GetUInt32Value(PLAYER_SHIELD_BLOCK);
                            hit_status |= HITSTATUS_BLOCK;
                        }

                        if(dmg.full_damage <= (int32)blocked_damage)
                        {
                            TRIGGER_AI_EVENT(pVictim, OnTargetBlocked)(castPtr<Unit>(this), blocked_damage);
                            TRIGGER_AI_EVENT(castPtr<Unit>(this), OnBlocked)(pVictim, blocked_damage);
                            vstate = BLOCK;
                        }

                        if( pVictim->IsPlayer() )//not necessary now but we'll have blocking mobs in future
                            pVictim->SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_DODGE_BLOCK);  //SB@L: Enables spells requiring dodge
                    }
                }
                break;
//--------------------------------critical hit----------------------------------------------
            case 5:
                {
                    hit_status |= HITSTATUS_CRICTICAL;
                    float dmg_bonus_pct = 100.0f;
                    if(ability && ability->SpellGroupType)
                        SM_PFValue(SMT_CRITICAL_DAMAGE,&dmg_bonus_pct,ability->SpellGroupType);

                    if( IsPlayer() )
                        SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_CRITICAL);  //SB@L: Enables spells requiring critical strike

                    // SpellAuraReduceCritRangedAttackDmg
                    if( weapon_damage_type == RANGED )
                        dmg_bonus_pct -= GetCritRangedDamageTakenModPct(dmg.school_type);
                    else dmg_bonus_pct -= GetCritMeleeDamageTakenModPct(dmg.school_type);

                    // actual crit damage?
                    if( dmg_bonus_pct > 0 )
                        dmg.full_damage += float2int32( float(dmg.full_damage) * (dmg_bonus_pct / 100.0f));

                    if(IsPlayer() && pVictim->IsPlayer())
                    {
                        //Resilience is a special new rating which was created to reduce the effects of critical hits against your character.
                        float dmg_reduction_pct = 2.2f * castPtr<Player>(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE ) / 100.0f;
                        if( dmg_reduction_pct > 0.33f )
                            dmg_reduction_pct = 0.33f; // 3.0.3
                        dmg.full_damage = float2int32( dmg.full_damage - dmg.full_damage*dmg_reduction_pct );
                    }

                    //emote
                    pVictim->Emote(EMOTE_ONESHOT_WOUND_CRITICAL);

                    TRIGGER_AI_EVENT(pVictim, OnTargetCritHit)(castPtr<Unit>(this), float(dmg.full_damage));
                    TRIGGER_AI_EVENT(castPtr<Unit>(this), OnCritHit)(pVictim, float(dmg.full_damage));
                } break;
//--------------------------------crushing blow---------------------------------------------
            case 6:
                hit_status |= HITSTATUS_CRUSHINGBLOW;
                dmg.full_damage = (dmg.full_damage * 3) >> 1;
                break;
//--------------------------------regular hit-----------------------------------------------
            default:
                break;
            }

//==========================================================================================
//==============================Post Roll Damage Processing=================================
//==========================================================================================
//--------------------------absorption------------------------------------------------------
            abs = pVictim->AbsorbDamage(this, dmg.school_type, dmg.full_damage, ability);
//--------------------------armor reducing--------------------------------------------------
            if(dmg.full_damage > (int32)blocked_damage && !disable_dR);// CalculateResistanceReduction(pVictim,&dmg, ability,armorreducepct);

            if (dmg.school_type == SCHOOL_NORMAL)
            {
                abs+=dmg.resisted_damage;
                dmg.resisted_damage=0;
            }

            realdamage = dmg.full_damage-abs-dmg.resisted_damage-blocked_damage;
            if(realdamage < 0)
            {
                realdamage = 0;
                vstate = IMMUNE;
                if (!(hit_status & HITSTATUS_BLOCK))
                    hit_status |= HITSTATUS_ABSORBED;
            }
        }
        break;
    }

//==========================================================================================
//==============================Post Roll Special Cases Processing==========================
//==========================================================================================
//--------------------------special states processing---------------------------------------

    /*if(pVictim->bInvincible == true) //godmode
    {
        abs = dmg.resisted_damage = dmg.full_damage;
        dmg.full_damage = realdamage = 0;
        hit_status |= HITSTATUS_ABSORBED;
    }*/

//--------------------------split damage-----------------------------------------------

//--------------------------spells triggering-----------------------------------------------

//==========================================================================================
//==============================Data Sending================================================
//==========================================================================================
    if( !ability )
    {
        if( dmg.full_damage > 0 )
        {
            if( abs > 0 )
                hit_status |= HITSTATUS_ABSORBED;
            else if (dmg.resisted_damage > 0)
                hit_status |= HITSTATUS_RESIST;
        }

        if( dmg.full_damage < 0 )
            dmg.full_damage = 0;

        if( realdamage < 0 )
            realdamage = 0;

        SendAttackerStateUpdate(pVictim, &dmg, realdamage, abs, blocked_damage, hit_status, vstate);
    }
    else
    {
        if( realdamage > 0 )//FIXME: add log for miss,block etc for ability and ranged
        {
            // here we send "dmg.resisted_damage" for "AbsorbedDamage", "0" for "ResistedDamage", and "false" for "PhysicalDamage" even though "School" is "SCHOOL_NORMAL"   o_O
            SendSpellNonMeleeDamageLog( castPtr<Unit>(this), pVictim, ability->Id, realdamage, dmg.school_type, dmg.resisted_damage, 0, false, blocked_damage, ( ( hit_status & HITSTATUS_CRICTICAL ) != 0 ), true );
        }
        //FIXME: add log for miss,block etc for ability and ranged
        //example how it works
        //SendSpellLog(this,pVictim,ability->Id,SPELL_LOG_MISS);
    }

    if(ability && realdamage == 0)
    {
        SendSpellLog(this,pVictim,ability->Id,SPELL_LOG_RESIST);
    }
//==========================================================================================
//==============================Damage Dealing==============================================
//==========================================================================================
    if(realdamage)
    {
        DealDamage(pVictim, realdamage, 0, targetEvent, 0);
        pVictim->GetSpellInterface()->PushbackCast(0);
    }
    else
    {
        // have to set attack target here otherwise it wont be set
        // because dealdamage is not called.
        //setAttackTarget(pVictim);
    }
//==========================================================================================
//==============================Post Damage Dealing Processing==============================
//==========================================================================================
//--------------------------durability processing-------------------------------------------

//--------------------------rage processing-------------------------------------------------
//http://www.wowwiki.com/Formulas:Rage_generation

    if( dmg.full_damage && getPowerType() == POWER_TYPE_RAGE && !ability)
    {
        float level = (float)getLevel();

        // Conversion Value
        float c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;

        // Hit Factor
        float f = ( weapon_damage_type == OFFHAND ) ? 1.75f : 3.5f;

        if( hit_status & HITSTATUS_CRICTICAL )
            f *= 2.0f;

        float s = 1.0f;
        if( weapon_damage_type == OFFHAND )
            s = GetUInt32Value( UNIT_FIELD_BASEATTACKTIME + 1 ) / 1000.0f;
        else s = GetUInt32Value( UNIT_FIELD_BASEATTACKTIME ) / 1000.0f;

        float val = ( 7.5f * dmg.full_damage / c + f * s ) / 2.0f;;
        val *= ( 1 + ( 100.f / 100.0f ) );
        val *= 10;

        ModPower(POWER_TYPE_RAGE, val);
        SendPowerUpdate();
    }

    // I am receiving damage!
    if( dmg.full_damage && pVictim->getPowerType() == POWER_TYPE_RAGE && pVictim->IsInCombat() )
    {
        float val;
        float level = (float)getLevel();

        // Conversion Value
        float c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;

        val = 2.5f * dmg.full_damage / c;
        val *= 10;

        pVictim->ModPower(POWER_TYPE_RAGE, (int32)val );
    }

    m_AuraInterface.RemoveAllAurasByInterruptFlag(AURA_INTERRUPT_ON_START_ATTACK);

//--------------------------extra strikes processing----------------------------------------
    if(!m_extraAttacks.empty())
    {
        std::vector<uint32> attacks(m_extraAttacks.begin(), m_extraAttacks.end());
        m_extraAttacks.clear();

        while(!attacks.empty())
        {
            if(SpellEntry *sp = dbcSpell.LookupEntry(*attacks.begin()))
            {
                Strike(pVictim, weapon_damage_type, NULL, 0, 0, true, false);
            }
            attacks.erase(attacks.begin());
        }
    }
}

bool Unit::UpdateAutoAttackState()
{
    if(m_attackTarget.empty())
        return false;

    Unit* victim = GetInRangeObject<Unit>(m_attackTarget);
    if (victim == NULL)
        return false;

    if(isCasting() || (m_attackTimer[MELEE] != 0 && !(m_dualWield && m_attackTimer[OFFHAND] == 0)))
        return false;

    uint8 swingError = 0;
    if (!canReachWithAttack(MELEE, victim))
    {
		if (m_attackTimer[MELEE] < 100) m_attackTimer[MELEE] = 100;
		if (m_attackTimer[OFFHAND] < 100) m_attackTimer[OFFHAND] = 100;
        swingError = 1;
    } // 120 degrees of radiant range
    else if (!isTargetInFront(victim))
	{
		if (m_attackTimer[MELEE] < 100) m_attackTimer[MELEE] = 100;
		if (m_attackTimer[OFFHAND] < 100) m_attackTimer[OFFHAND] = 100;
        swingError = 2;
    }
    else
    {
        if (m_attackTimer[0] == 0)
        {
            // prevent base and off attack in same time, delay attack at 0.2 sec
            if (m_dualWield && m_attackTimer[OFFHAND] < 200)
                m_attackTimer[OFFHAND] = 200;
            EventAttack(victim, MELEE);
            resetAttackTimer(MELEE);
        }

        if (m_dualWield && m_attackTimer[1] == 0)
        {
            // prevent base and off attack in same time, delay attack at 0.2 sec
            if (m_attackTimer[MELEE] < 200) m_attackTimer[MELEE] = 200;
            // do attack
            EventAttack(victim, OFFHAND);
            resetAttackTimer(OFFHAND);
        }

        // Trigger a combat update timer on attack
        SetInCombat(victim);
    }

    Player *plr = NULL;
    if(swingError && (plr = IsPlayer() ? castPtr<Player>(this) : NULL) != NULL && plr->GetLastSwingError() != swingError)
    {
        WorldPacket data;
        if (swingError == 1)
            plr->PushData(SMSG_ATTACKSWING_BADFACING);
        else if (swingError == 2)
            plr->PushData(SMSG_ATTACKSWING_NOTINRANGE);
        plr->SetLastSwingError(swingError);
    }

    return swingError == 0;
}

void Unit::EventAttack( Unit *target, WeaponDamageType attackType )
{
    if (!m_spellInterface.getNextMeleeSpell() || attackType == OFFHAND)
        Strike( target, attackType, NULL, 0, 0, false, false, true);
    else m_spellInterface.TriggerNextMeleeSpell(target);
}

void Unit::EventAttackStart(WoWGuid guid)
{
    m_attackTarget = guid;
    addStateFlag(UF_ATTACKING);
    smsg_AttackStart(m_attackTarget);
    Dismount();

    SetUInt64Value(UNIT_FIELD_TARGET, guid);
    m_spellInterface.OnChangeSelection(guid);
}

void Unit::EventAttackStop()
{
    clearStateFlag(UF_ATTACKING);
    smsg_AttackStop(m_attackTarget);
    m_attackTarget.Clean();
    m_spellInterface.OnChangeSelection(0);
}

void Unit::smsg_AttackStart(WoWGuid victimGuid)
{
    // Send out ATTACKSTART
    WorldPacket data(SMSG_ATTACKSTART, 16);
    data << GetGUID() << victimGuid;
    SendMessageToSet(&data, IsPlayer());
}

void Unit::smsg_AttackStop(WoWGuid victimGuid)
{
    WorldPacket data(SMSG_ATTACKSTOP, 20);
    data << GetGUID().asPacked();
    data << victimGuid.asPacked();
    data << uint32(0);
    SendMessageToSet(&data, IsPlayer());
}

int32 Unit::GetSpellBonusDamage(Unit* pVictim, SpellEntry *spellInfo, uint8 effIndex, int32 base_dmg, bool healing)
{
    int32 bonus_damage = base_dmg;
    Unit* caster = castPtr<Unit>(this);
    uint32 school = spellInfo->School;
    float summaryPCTmod = 0.0f;
    float levelPenalty = CalculateLevelPenalty(spellInfo);

	if( caster->IsSummon() && castPtr<Summon>(caster)->GetSummonOwner() )
        caster = castPtr<Summon>(caster)->GetSummonOwner()->IsUnit() ? castPtr<Unit>(castPtr<Summon>(caster)->GetSummonOwner()) : NULL;
    else if( caster->GetTypeId() == TYPEID_GAMEOBJECT && caster->GetMapInstance() && caster->GetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY) )
        caster = castPtr<Unit>(caster->GetMapInstance()->GetUnit(caster->GetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY)));
    if( caster == NULL || pVictim == NULL)
        return bonus_damage;

    //---------------------------------------------------------
    // victim type
    //---------------------------------------------------------

    //---------------------------------------------------------
    // coefficient
    //---------------------------------------------------------
    float coefficient = 0.0f;
    if(spellInfo->School)
        coefficient += spellInfo->EffectBonusCoefficient[effIndex];

    //---------------------------------------------------------
    // modifiers (increase spell dmg by spell power)
    //---------------------------------------------------------
    if( spellInfo->SpellGroupType )
    {
        float modifier = 0;
        caster->SM_FFValue( SMT_SP_BONUS, &modifier, spellInfo->SpellGroupType );
        caster->SM_FFValue( SMT_SP_BONUS, &modifier, spellInfo->SpellGroupType );
        coefficient += modifier / 100.0f;
    }

    //---------------------------------------------------------
    // Apply coefficient
    //---------------------------------------------------------
    if( !healing )
    {
        if(coefficient) // Saves us some time.
            bonus_damage += caster->GetDamageDoneMod(school) * coefficient * levelPenalty;
        bonus_damage += pVictim->GetDamageTakenMod(school) * levelPenalty;
    }
    else
    {
        if(coefficient) // Saves us some time.
            bonus_damage += caster->GetHealingDoneMod() * coefficient * levelPenalty;
    }

    //---------------------------------------------------------
    // AP coefficient
    //---------------------------------------------------------

    //---------------------------------------------------------
    // modifiers (damage done by x)
    //---------------------------------------------------------
    if( spellInfo->SpellGroupType )
    {
        float dmg_bonus_pct = 0;
        caster->SM_FFValue(SMT_SPELL_VALUE_PCT, &dmg_bonus_pct, spellInfo->SpellGroupType);
        caster->SM_FFValue(SMT_DAMAGE_DONE, &dmg_bonus_pct, spellInfo->SpellGroupType);
        caster->SM_FIValue(SMT_DAMAGE_DONE, &bonus_damage, spellInfo->SpellGroupType);

        dmg_bonus_pct /= 100;
        bonus_damage += float2int32(bonus_damage * dmg_bonus_pct);
    }

    //---------------------------------------------------------
    // MISC BONUSDAMAGE
    //---------------------------------------------------------
    if( spellInfo->isSpellDamagingEffect() && spellInfo->isSpellAreaOfEffect() )
        bonus_damage *= pVictim->GetAreaOfEffectDamageMod();

    bool ranged = spellInfo->isSpellRangedSpell();
    float ap_coeff = 0.0f;
    if(spellInfo->spellType != NON_WEAPON)
        ap_coeff += spellInfo->EffectBonusCoefficient[effIndex];
    if(ap_coeff) bonus_damage += ap_coeff*(ranged ? caster->GetRangedAttackPower() : caster->GetAttackPower());

    //---------------------------------------------------------
    // PCT mods
    //---------------------------------------------------------
    if( !healing )
    {
        summaryPCTmod += caster->GetDamageDonePctMod(school)-1.f; //value is initialized with 1
        summaryPCTmod += pVictim->GetDamageTakenModPct(school)-1.f;//value is initialized with 1
        summaryPCTmod += pVictim->GetDamageTakenByMechPCTMod(Spell::GetMechanic(spellInfo));
    }

    int32 res = float2int32(((float)bonus_damage) * summaryPCTmod) + bonus_damage;
    if( res < 0 )
        res = 0;
    return res;
}

float Unit::CalculateLevelPenalty(SpellEntry* sp)
{
    if (sp->spellLevelSpellLevel <= 0 || sp->spellLevelSpellLevel >= sp->spellLevelMaxLevel || sp->spellLevelMaxLevel >= MAXIMUM_CEXPANSION_LEVEL)
        return 1.0f; // For custom content purposes, do not reduce damage done by the highest available teir of spells.

    float LvlPenalty = 0.0f;
    if (sp->spellLevelSpellLevel < 20)
        LvlPenalty = 20.0f - sp->spellLevelSpellLevel * 3.75f;
    float LvlFactor = (float(sp->spellLevelSpellLevel) + 6.0f) / float(getLevel());
    if (LvlFactor > 1.0f)
        LvlFactor = 1.0f;

    return float((100.0f - LvlPenalty) * LvlFactor / 100.0f);
}

void Unit::DeMorph()
{
    // hope it solves it :)
    uint32 displayid = GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    SetUInt32Value(UNIT_FIELD_DISPLAYID, displayid);
}

void Unit::Emote(EmoteType emote)
{
    if(emote == 0)
        return;

    WorldPacket data(SMSG_EMOTE, 12);
    data << uint32(emote);
    data << GetGUID();
    SendMessageToSet (&data, true);
}

void Unit::SendChatMessageToPlayer(uint8 type, uint32 lang, const char *msg, Player* plr)
{
    ASSERT(plr);

    const char* name = IsPlayer() ? castPtr<Player>(this)->GetName() : "";
    if(IsCreature()) name = castPtr<Creature>(this)->GetName();

    WorldPacket data;
    sChatHandler.FillMessageData(&data, false, type, lang, GetGUID(), 0, name, msg, "", 0);
    plr->PushPacket(&data);
}

void Unit::SendChatMessageAlternateEntry(uint32 entry, uint8 type, uint32 lang, const char * msg)
{
    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(entry);
    if(ctrData == NULL)
        return;

    WorldPacket data;
    sChatHandler.FillMessageData(&data, false, type, lang, GetGUID(), 0, ctrData->maleName, msg, "", 0);
    SendMessageToSet(&data, true);
}

void Unit::SendChatMessage(uint8 type, uint32 lang, const char *msg)
{
    const char* name = IsPlayer() ? castPtr<Player>(this)->GetName() : "";
    if(IsCreature()) name = castPtr<Creature>(this)->GetName();

    WorldPacket data;
    sChatHandler.FillMessageData(&data, false, type, lang, GetGUID(), 0, name, msg, "", 0);
    SendMessageToSet(&data, true);
}

//Events
void Unit::EventAddEmote(EmoteType emote, uint32 time)
{
    m_oldEmote = GetUInt32Value(UNIT_NPC_EMOTESTATE);
    SetUInt32Value(UNIT_NPC_EMOTESTATE,emote);
}

void Unit::EventAllowCombat(bool allow)
{

}

void Unit::EmoteExpire()
{
    SetUInt32Value(UNIT_NPC_EMOTESTATE, m_oldEmote);
}

// grep: Remove any AA spells that aren't owned by this player.
//       Otherwise, if he logs out and in and joins another group,
//       he'll apply it to them.
/*
void Unit::RemoveAllAreaAuras()
{
    AuraList::iterator itr,it1;
    for(itr = m_auras.begin();itr!=m_auras.end();)
    {
        it1 = itr;
        ++itr;
        if(((*it1)->m_spellProto->Effect[0] == SPELL_EFFECT_APPLY_AREA_AURA ||
            (*it1)->m_spellProto->Effect[1] == SPELL_EFFECT_APPLY_AREA_AURA ||
            (*it1)->m_spellProto->Effect[2] == SPELL_EFFECT_APPLY_AREA_AURA) && (*it1)->GetCaster() != this)
        {
            (*it1)->Remove();
            m_auras.erase(it1);
        }
    }
}
*/
uint32 Unit::AbsorbDamage( WorldObject* Attacker, uint32 School, int32 dmg, SpellEntry * pSpell )
{
    if( dmg == 0 || Attacker == NULL  || School > 6 )
        return 0;

    int32 abs = 0;
    float reflect_pct = 0.f;

    std::set<Aura*> m_removeAuras;
    if(AuraInterface::modifierMap *absorbMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_SCHOOL_ABSORB))
    {
        for(AuraInterface::modifierMap::iterator itr = absorbMap->begin(); itr != absorbMap->end(); itr++)
        {
            if(itr->second->m_miscValue[0] & (1<<School))
            {
                if(itr->second->fixed_amount == 0)
                    continue;

                int32 absDiff = dmg-abs;
                if( absDiff >= itr->second->fixed_amount)//remove this absorb
                {
                    abs += itr->second->fixed_amount;
                    if(abs == dmg)
                        break;
                }
                else
                {
                    abs = dmg;
                    itr->second->fixed_amount -= absDiff;
                    reflect_pct += itr->second->fixed_float_amount;
                    break;
                }
            }
        }
    }

    if( abs > 0 )
    {
        if(reflect_pct > 0.f && Attacker && Attacker->IsUnit() )
        {
            int32 amt = float2int32(abs * reflect_pct);
            DealDamage( castPtr<Unit>( Attacker ), amt, 0, 0, 0 );
        }
    }

    return abs;
}

void Unit::SetStandState(uint8 standstate)
{
    SetByte( UNIT_FIELD_BYTES_1, 0, standstate );
    if( standstate == STANDSTATE_STAND )//standup
        m_AuraInterface.RemoveAllAurasByInterruptFlag(AURA_INTERRUPT_ON_STAND_UP);

    if( GetTypeId() == TYPEID_PLAYER )
        castPtr<Player>(this)->PushData( SMSG_STANDSTATE_UPDATE, 1, &standstate );
}

void Unit::EventSummonPetExpire()
{
    /*if(summonPet)
    {
        if(summonPet->GetEntry() == 7915)//Goblin Bomb
        {
            SpellEntry *spInfo = dbcSpell.LookupEntry(13259);
            if(!spInfo)
                return;

            Spell* sp = NULL;
            sp = (new Spell(summonPet,spInfo,true,NULL));
            SpellCastTargets tgt;
            tgt.m_unitTarget=summonPet->GetGUID();
            sp->prepare(&tgt);
        }
        else
        {
            summonPet->RemoveFromWorld(, true);
            summonPet->Destruct();
            summonPet = NULL;
        }
    }*/
}

void Unit::PlaySpellVisual(uint64 target, uint32 spellVisual)
{
    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 12);
    data << target << spellVisual;
    SendMessageToSet(&data, true);
}

void Unit::SendPlaySpellVisualKit(uint32 id, uint32 unkParam)
{
    WorldPacket data(SMSG_PLAY_SPELL_VISUAL_KIT, 4 + 4+ 4 + 8);
    data << uint32(0);
    data << uint32(id); // SpellVisualKit.dbc index
    data << uint32(unkParam);
    data.WriteBit(m_objGuid[4]);
    data.WriteBit(m_objGuid[7]);
    data.WriteBit(m_objGuid[5]);
    data.WriteBit(m_objGuid[3]);
    data.WriteBit(m_objGuid[1]);
    data.WriteBit(m_objGuid[2]);
    data.WriteBit(m_objGuid[0]);
    data.WriteBit(m_objGuid[6]);
    data.FlushBits();
    data.WriteByteSeq(m_objGuid[0]);
    data.WriteByteSeq(m_objGuid[4]);
    data.WriteByteSeq(m_objGuid[1]);
    data.WriteByteSeq(m_objGuid[6]);
    data.WriteByteSeq(m_objGuid[7]);
    data.WriteByteSeq(m_objGuid[2]);
    data.WriteByteSeq(m_objGuid[3]);
    data.WriteByteSeq(m_objGuid[5]);
    SendMessageToSet(&data, true);
}

void Unit::OnPreSetInWorld()
{
    m_movementInterface.OnPreSetInWorld();
}

void Unit::OnPrePushToWorld()
{
    // Notify our movement info that we're being pushed
    m_movementInterface.OnPrePushToWorld();
}

void Unit::OnPushToWorld()
{
    WorldObject::OnPushToWorld();

    m_movementInterface.OnPushToWorld();
}

void Unit::RemoveFromWorld()
{
    SummonExpireAll(false);

    // Trigger our movement interface's removal signal
    m_movementInterface.OnRemoveFromWorld();

    // Delete AAura's from our targets (must be done before object is removed from world)
    m_AuraInterface.RemoveAllAurasWithSpEffect(SPELL_EFFECT_APPLY_AREA_AURA);
    m_AuraInterface.RemoveAllAurasWithSpEffect(SPELL_EFFECT_APPLY_AREA_AURA_FRIEND);
    m_AuraInterface.RemoveAllAurasWithSpEffect(SPELL_EFFECT_APPLY_AREA_AURA_ENEMY);

    m_spellInterface.OnRemoveFromWorld();

    WorldObject::RemoveFromWorld();
}

void Unit::EventExploration(MapInstance *instance)
{
    WorldObject::EventExploration(instance);
}

void Unit::SetPosition( float newX, float newY, float newZ, float newOrientation )
{
    WorldObject::SetPosition(newX, newY, newZ, newOrientation);
}

bool Unit::IsPoisoned()
{
    return m_AuraInterface.IsPoisoned();
}

bool Unit::IsDazed()
{
    return m_AuraInterface.IsDazed();
}

void Unit::UpdateVisibility()
{
    if(!IsInWorld())
        return;

    GetCellManager()->UpdateVisibility(m_mapInstance);
}

void Unit::InitVehicleKit(uint32 vehicleKitId)
{
    if (m_vehicleKitId == vehicleKitId)
        return;

    m_vehicleKitId = vehicleKitId;
    m_updateFlags |= UPDATEFLAG_VEHICLE;

    if(!IsInWorld())
        return;

    WorldPacket data(SMSG_PLAYER_VEHICLE_DATA, 8 + 4);
    data << m_objGuid.asPacked();
    data << uint32(vehicleKitId);
    SendMessageToSet(&data, true);
}

void Unit::SetAIAnimKitId(uint16 animKitId)
{
    if (m_aiAnimKitId == animKitId)
        return;

    m_aiAnimKitId = animKitId;

    if(!IsInWorld())
        return;

    WorldPacket data(SMSG_SET_AI_ANIM_KIT, 8 + 2);
    data << m_objGuid.asPacked();
    data << uint16(animKitId);
    SendMessageToSet(&data, true);
}

void Unit::SetMovementAnimKitId(uint16 animKitId)
{
    if (m_movementAnimKitId == animKitId)
        return;

    m_movementAnimKitId = animKitId;

    if(!IsInWorld())
        return;

    WorldPacket data(SMSG_SET_MOVEMENT_ANIM_KIT, 8 + 2);
    data << m_objGuid.asPacked();
    data << uint16(animKitId);
    SendMessageToSet(&data, true);
}

void Unit::SetMeleeAnimKitId(uint16 animKitId)
{
    if (m_meleeAnimKitId == animKitId)
        return;

    m_meleeAnimKitId = animKitId;

    WorldPacket data(SMSG_SET_MELEE_ANIM_KIT, 8 + 2);
    data << m_objGuid.asPacked();
    data << uint16(animKitId);
    SendMessageToSet(&data, true);
}

void Unit::EventHealthChangeSinceLastUpdate()
{
    uint8 pct = GetHealthPct();
    uint32 toSet = 0, toRemove = 0;
    if( isAlive() && pct <= 20 && !HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH20) )
        toSet |= AURASTATE_FLAG_HEALTH20;
    else if((isDead() || pct > 20) && HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH20))
        toRemove |= AURASTATE_FLAG_HEALTH20;

    if( isAlive() && pct <= 35 && !HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH35) )
        toSet |= AURASTATE_FLAG_HEALTH35;
    else if((isDead() || pct > 35) && HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH35) )
        toRemove |= AURASTATE_FLAG_HEALTH35;

    if( isAlive() && pct >= 75 && !HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTHABOVE75) )
        toSet |= AURASTATE_FLAG_HEALTHABOVE75;
    else if((isDead() || pct < 75) && HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTHABOVE75))
        toRemove |= AURASTATE_FLAG_HEALTHABOVE75;

    if(toSet)
        SetFlag(UNIT_FIELD_AURASTATE, toSet);

    if(toRemove)
        RemoveFlag(UNIT_FIELD_AURASTATE, toRemove);
}

int32 Unit::CalculateAttackPower()
{
    int32 baseap = GetUInt32Value(UNIT_FIELD_ATTACK_POWER) + GetAttackPowerPositiveMods() - GetAttackPowerNegativeMods();
    float totalap = float(baseap)*(GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER)+1);
    if(totalap >= 0)
        return float2int32(totalap);
    return 0;
}

int32 Unit::CalculateRangedAttackPower()
{
    int32 baseap = GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER) + GetRangedAttackPowerPositiveMods() - GetRangedAttackPowerNegativeMods();
    float totalap = float(baseap)*(GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER)+1);
    if(totalap >= 0)
        return float2int32(totalap);
    return 0;
}

//guardians are temporary spawn that will inherit master faction and will folow them. Apart from that they have their own mind
Unit* Unit::CreateTemporaryGuardian(uint32 guardian_entry,uint32 duration,float angle, uint32 lvl, uint8 Slot)
{
    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(guardian_entry);
    if(ctrData == NULL)
    {
        sLog.outDebug("Warning : Missing summon creature template %u !",guardian_entry);
        return NULL;
    }

    LocationVector v = GetPositionNC();
    float m_followAngle = angle + v.o;
    float x = v.x +(3*(cosf(m_followAngle)));
    float y = v.y +(3*(sinf(m_followAngle)));

    if(Creature* p = GetMapInstance()->CreateCreature(WoWGuid(), guardian_entry))
    {
        p->Load(GetMapId(), x, y, v.z, angle, GetMapInstance()->iInstanceMode);
        p->SetInstanceID(GetMapInstance()->GetInstanceID());

        if (lvl != 0)
        {
            /* MANA */
            if(uint32 maxMana = p->GetMaxPower(POWER_TYPE_MANA)+28+10*lvl)
            {
                p->SetPowerType(POWER_TYPE_MANA);
                p->SetMaxPower(POWER_TYPE_MANA, maxMana);
                p->SetPower(POWER_TYPE_MANA, maxMana);
            }
            /* HEALTH */
            p->SetUInt32Value(UNIT_FIELD_MAXHEALTH,p->GetUInt32Value(UNIT_FIELD_MAXHEALTH)+28+30*lvl);
            p->SetUInt32Value(UNIT_FIELD_HEALTH,p->GetUInt32Value(UNIT_FIELD_HEALTH)+28+30*lvl);
            /* LEVEL */
            p->SetUInt32Value(UNIT_FIELD_LEVEL, lvl);
        }

        p->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, GetGUID());
        p->SetUInt64Value(UNIT_FIELD_CREATEDBY, GetGUID());
        p->SetFactionTemplate(GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
        p->PushToWorld(GetMapInstance());
        return p;
    }
    return NULL;
}

void Unit::SummonExpireAll(bool clearowner)
{
    //Should be done allready, but better check if all summoned Creatures/Totems/GO's are removed;
    //empty our owners summonslot first (if possible).
    if(clearowner && IsCreature() && castPtr<Creature>(this)->IsSummon())
    {
        Unit * Owner = NULL;
        if(Owner != NULL)
        {
            uint8 slot = castPtr<Summon>(this)->GetSummonSlot();
            Owner->SummonExpireSlot(slot);
        }
    }

    //remove summoned npc's (7 slots)
    for(uint8 x = 0; x < 7; ++x)
        SummonExpireSlot(x);

    //remove summoned go's (4 slots)
    for(uint32 x = 0; x < 4; ++x)
    {
        if(IsInWorld() && !m_ObjectSlots[x].empty())
            if(GameObject* obj = m_mapInstance->GetGameObject(m_ObjectSlots[x]))
                obj->Deactivate(0);
        m_ObjectSlots[x] = 0;
    }
}

void Unit::FillSummonList(std::vector<Creature*> &summonList, uint8 summonType)
{
    for(Loki::AssocVector< uint32, std::vector<Creature*> >::iterator itr = m_Summons.begin(); itr != m_Summons.end(); itr++)
    {
        if(!itr->second.size())
            continue;
        for(std::vector<Creature*>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++)
        {
            // Should never happen
            if(!(*itr2)->IsSummon())
                continue;
            Summon* summon = castPtr<Summon>(*itr2);
            if(summonType == 0xFF || summon->GetSummonType() == summonType)
                summonList.push_back(*itr2);
        }
    }
}

void Unit::RemoveSummon(Creature* summon)
{
    for(Loki::AssocVector< uint32, std::vector<Creature*> >::iterator itrMain = m_Summons.begin(); itrMain != m_Summons.end(); itrMain++)
    {
        if(itrMain->second.size())
        {
            for(std::vector<Creature*>::iterator itr = itrMain->second.begin(); itr != itrMain->second.end(); itr++)
            {
                if(summon->GetGUID() == summon->GetGUID())
                {
                    itrMain->second.erase(itr);
                    break;
                }
            }
        }
    }
}

void Unit::SummonExpireSlot(uint8 Slot)
{
    if(!m_Summons.empty())
    {
        if(Slot > 7)
            Slot = 0;

        //remove summons
        if(m_Summons.find(Slot) != m_Summons.end() && m_Summons[Slot].size())
        {
            Creature* mSum = NULL;
            for(std::vector<Creature*>::iterator itr = m_Summons[Slot].begin(); itr != m_Summons[Slot].end(); itr++)
            {
                mSum = *itr;
                mSum->m_AuraInterface.RemoveAllAuras();
                mSum->DetatchFromSummoner();
            }

            m_Summons[Slot].clear();
            m_Summons.erase(Slot);
        }
    }
}

float Unit::CalculateDazeCastChance(Unit* target)
{
    float attack_skill = float( getLevel() ) * 5.0f, defense_skill = 1.f;
    if( target->IsPlayer() )
        defense_skill = float( castPtr<Player>( target )->getSkillLineVal( SKILL_DEFENSE, false ) );
    else defense_skill = float( target->getLevel() * 5 );

    float chance_to_daze = (attack_skill * 20 / std::max<float>(1.f, defense_skill)) //if level is equal then we get a 20% chance to daze
        * std::min(target->getLevel() / 30.0f, 1.0f );//for targets below level 30 the chance decreses
    return std::min<float>(40.f, chance_to_daze);
}

uint32 Unit::Heal(Unit* target, uint32 amount, uint32 SpellId, bool silent)
{//Static heal
    if(target == NULL || !target->isAlive() || amount == 0)
        return amount;

    int32 healAmount;
    if((healAmount = std::min<uint32>(amount, target->GetUInt32Value(UNIT_FIELD_MAXHEALTH)-target->GetUInt32Value(UNIT_FIELD_HEALTH))) > 0)
        target->ModSignedInt32Value(UNIT_FIELD_HEALTH, healAmount);
    else healAmount = 0;

    uint32 overHeal = amount-healAmount;
    if(silent == false && SpellId)
        Spell::SendHealSpellOnPlayer(this, target, amount, false, overHeal, SpellId);
    return overHeal;
}

void Unit::Energize(Unit* target, uint32 SpellId, uint32 amount, uint32 type)
{//Static energize
    if( !target || !SpellId )
        return;

    target->ModPower(type, amount);
    Spell::SendHealManaSpellOnPlayer(this,target, amount, type, SpellId);
    target->SendPowerUpdate();
}

void Unit::SetInCombat(Unit *unit, uint32 timerOverride)
{
    ASSERT(unit);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
    // Set our target's combat flag too
    unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
    m_mapInstance->TriggerCombatTimer(GetGUID(), unit->GetGUID(), timerOverride);
}

void Unit::EventModelChange()
{
    //TODO: if has mount, grab mount model and add the z value of attachment 0
    if(CreatureBoundDataEntry *boundData = dbcCreatureBoundData.LookupEntry(GetUInt32Value(UNIT_FIELD_DISPLAYID)))
        m_modelhalfsize = boundData->High[2]/2.f;
    else m_modelhalfsize = 1.0f;
}

//what is an Immobilize spell ? Have to add it later to spell effect handler
void Unit::EventStunOrImmobilize()
{

}

void Unit::SetTriggerStunOrImmobilize(uint32 newtrigger,uint32 new_chance)
{

}

void Unit::SendPowerUpdate(EUnitFields powerField)
{
    WorldPacket data(SMSG_POWER_UPDATE, 20);
    data << GetGUID().asPacked();
    data << uint32(1);
    if(powerField == UNIT_END)
    {
        uint32 count = 0;
        size_t pos = data.wpos()-4;
        for(uint8 i = POWER_TYPE_MANA; i < POWER_TYPE_MAX; i++)
        {
            EUnitFields field = GetPowerFieldForType(i);
            if(field == UNIT_END)
                continue;

            data << uint8(i);
            data << GetUInt32Value(field);
            count++;
        }
        if(count == 0)
            return;

        data.put<uint32>(pos, count);
    }
    else
    {
        data << uint8(powerField-UNIT_FIELD_POWERS);
        data << GetUInt32Value(powerField);
    }

    SendMessageToSet(&data, true);
}

void Unit::AddOnAuraRemoveSpell(uint32 NameHash, uint32 procSpell, uint32 procChance, bool procSelf)
{
    Loki::AssocVector<uint32, onAuraRemove*>::iterator itr;
    if((itr = m_onAuraRemoveSpells.find(NameHash)) != m_onAuraRemoveSpells.end())
    {
        itr->second->spell = procSpell;
        itr->second->chance = procChance;
        itr->second->self = procSelf;
        itr->second->deleted = false;
    }
    else
    {
        onAuraRemove *proc = new onAuraRemove;
        proc->spell = procSpell;
        proc->chance = procChance;
        proc->self = procSelf;
        proc->deleted = false;

        m_onAuraRemoveSpells[NameHash] = proc;
    }

}

void Unit::RemoveOnAuraRemoveSpell(uint32 NameHash)
{
    Loki::AssocVector<uint32, onAuraRemove*>::iterator itr;
    if((itr = m_onAuraRemoveSpells.find(NameHash)) != m_onAuraRemoveSpells.end())
        itr->second->deleted = true;
}

// Aura by NameHash has been removed
void Unit::OnAuraRemove(uint32 NameHash, Unit* m_target)
{
    Loki::AssocVector<uint32, onAuraRemove*>::iterator itr;
    if((itr = m_onAuraRemoveSpells.find(NameHash)) != m_onAuraRemoveSpells.end())
    {
        bool apply = true;
        SpellEntry *sp = dbcSpell.LookupEntry(itr->second->spell);
        if (itr->second->deleted == true || sp == NULL || itr->second->spell == NULL)
            return;
        if(itr->second->self == false && m_target == NULL)
            return;

        if (itr->second->chance != 100)
            apply = RandomUInt(100) < itr->second->chance;
        if (apply == false)
            return;

        // Trigger the aura spell
        GetSpellInterface()->TriggerSpell(sp, itr->second->self ? this : m_target);
    }
}

void Unit::SetUnitStunned(bool state)
{
    if(state)
    {
        if(_stunStateCounter++)
            return;
        m_movementInterface.setStunned(true);
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        return;
    }

    if(_stunStateCounter == 0 || (--_stunStateCounter > 0))
        return;
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    m_movementInterface.setStunned(false);
}

//! Is PVP flagged?
bool Unit::IsPvPFlagged()
{
    return HasByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
}

void Unit::SetPvPFlag()
{
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
    SetByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
}

//! Removal
void Unit::RemovePvPFlag()
{
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
    RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
}

uint16 Unit::GetFieldPctLg(EUnitFields field, EUnitFields maxField)
{
    uint32 val = GetPower(field), maxVal = GetMaxPower(maxField);
    if(maxVal == 0)
        return 0;
    return uint16(ceil(float(val)*10000.f/float(maxVal)));
}

int32 Unit::GetPowerPct(EUnitFields field, EUnitFields maxField)
{
    uint32 power = GetPower(field), maxPower = GetMaxPower(maxField);
    if(maxPower == 0)
        return 0;
    return int32(ceil(float(power)*100.f/float(maxPower)));
}

void Unit::SetPowerType(uint8 type)
{
    SetByte(UNIT_FIELD_BYTES_0, 3, type);

    if(type == POWER_TYPE_RUNIC && IsPlayer())
        for(uint8 i = 0; i < 4; i++)
            SetFloatValue(PLAYER_RUNE_REGEN_1 + i, 0.1f);
}

int32 Unit::GetPowerPct(uint8 type)
{
    EUnitFields field = GetPowerFieldForType(type), maxField = EUnitFields(field+(UNIT_FIELD_MAXPOWERS-UNIT_FIELD_POWERS));
    if(field == UNIT_END || maxField == UNIT_END)
        return 0;
    return GetPowerPct(field, maxField);
}

void Unit::LosePower(uint8 type, int32 decayValue)
{
    if( m_interruptRegen > 0 )
        return;

    ModPower(type, -decayValue);
}

uint32 Unit::GetPower(uint8 type)
{
    EUnitFields field = GetPowerFieldForType(type);
    if(field == UNIT_END)
        return 0;
    return GetUInt32Value(field);
}

uint32 Unit::GetMaxPower(uint8 type)
{
    EUnitFields field = GetMaxPowerFieldForType(type);
    if(field == UNIT_END)
        return 0;
    return GetUInt32Value(field);
}

void Unit::ModPower(uint8 type, int32 value)
{
    EUnitFields field = GetPowerFieldForType(type);
    if(field == UNIT_END)
        return;
    ModPower(field, value);
}

void Unit::SetPower(uint8 type, uint32 value)
{
    EUnitFields field = GetPowerFieldForType(type);
    if(field == UNIT_END)
        return;
    SetPower(field, value);
}

void Unit::SetMaxPower(uint8 type, uint32 value)
{
    EUnitFields field = GetMaxPowerFieldForType(type);
    if(field == UNIT_END)
        return;
    SetMaxPower(field, value);
}

uint32 Unit::GetPower(EUnitFields field)
{
    return GetUInt32Value(field);
}

uint32 Unit::GetMaxPower(EUnitFields field)
{
    return GetUInt32Value(field);
}

void Unit::ModPower(EUnitFields field, int32 value)
{
    EUnitFields maxfield = EUnitFields(field+(UNIT_FIELD_MAXPOWERS-UNIT_FIELD_POWERS));
    int32 val = GetUInt32Value(field)+value;
    if(val < 0.0f) val = 0.0f;
    if(val > GetUInt32Value(maxfield))
        val = GetUInt32Value(maxfield);
    SetUInt32Value(field, val);
}

void Unit::SetPower(EUnitFields field, uint32 value)
{
    if(GetPower(field) == value)
        return;

    SetUInt32Value(field, value);
    SendPowerUpdate(field);
}

void Unit::SetMaxPower(EUnitFields field, uint32 value)
{
    if(GetMaxPower(field) == value)
        return;

    SetUInt32Value(field, value);
}

EUnitFields Unit::GetPowerFieldForType(uint8 type)
{
    if(type == POWER_TYPE_HEALTH)
        return UNIT_FIELD_HEALTH;
	return sStatSystem.GetPowerFieldForClassAndType(getClass(), type);
}

EUnitFields Unit::GetMaxPowerFieldForType(uint8 type)
{
    if(type == POWER_TYPE_HEALTH)
        return UNIT_FIELD_MAXHEALTH;
	EUnitFields field = sStatSystem.GetPowerFieldForClassAndType(getClass(), type);
    if(field == UNIT_END)
        return UNIT_END;
    return EUnitFields(field+(UNIT_FIELD_MAXPOWERS-UNIT_FIELD_POWERS));
}

void Unit::SetDeathState(DeathState s)
{
    m_deathState = s;
    if(s == DEAD && !hasStateFlag(UF_CORPSE))
    {
        addStateFlag(UF_CORPSE);
        // Trigger cell manager death state
        GetCellManager()->OnUnitDeath(m_mapInstance);
        // Remove combat flag at death
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
    } else if(s != DEAD)
        clearStateFlag(UF_CORPSE);
}

//  custom functions for scripting
void Unit::SetWeaponDisplayId(uint8 slot, uint32 ItemId)
{
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+slot, ItemId);
}

bool Unit::IsFFAPvPFlagged()
{
    return HasByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_FFA_PVP);
}

void Unit::SetFFAPvPFlag()
{
    if(IsFFAPvPFlagged())
        return;

    SetByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_FFA_PVP);
}

void Unit::RemoveFFAPvPFlag()
{
    if(!IsFFAPvPFlagged())
        return;

    RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_FFA_PVP);
}

void Unit::Dismount()
{
    m_AuraInterface.OnDismount();
    m_movementInterface.OnDismount();

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI );
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER );
}

bool Unit::IsFactionNonHostile()
{
    if(m_factionTemplate == NULL)
        return true;
    return m_factionTemplate->nonHostileFaction;
}

bool Unit::IsFactionNPCHostile()
{
    if(IsCreature() && castPtr<Creature>(this)->IsScriptedNPCHostile())
        return true;
    return false;
}

void Unit::SetFaction(uint32 faction, bool save)
{
    SetFactionTemplate(faction);
    if(IsCreature() && m_factionTemplate)
    {
        if(save && castPtr<Creature>(this)->IsSpawn())
            castPtr<Creature>(this)->SaveToDB();

        m_unitTeam = m_factionTemplate->FactionTeam;
        // Creature faction templates show what should be flagged for pvp
        if(m_factionTemplate->FactionFlags & 0x800)
            SetPvPFlag();
        else RemovePvPFlag();
    }
}

void Unit::ResetFaction()
{
    uint32 faction = 35;
    if(IsPlayer())
        faction = castPtr<Player>(this)->getPlayerCreateInfo()->factiontemplate;
    else if(IsCreature()) faction = castPtr<Creature>(this)->GetCreatureData()->faction;

    SetFaction(faction);
}

void Unit::knockback(int32 basepoint, uint32 miscvalue, bool disengage )
{

}

void Unit::Teleport(float x, float y, float z, float o)
{
    if(IsPlayer())
        castPtr<Player>(this)->SafeTeleport(GetMapId(), GetInstanceID(), x, y, z, o);
    else
    {
        WorldPacket data(SMSG_MONSTER_MOVE, 50);
        data << GetGUID().asPacked();
        data << uint8(0);
        data << GetPositionX();
        data << GetPositionY();
        data << GetPositionZ();
        data << getMSTime();
        data << uint8(0x0);
        data << uint32(256);
        data << uint32(1);
        data << uint32(1);
        data << x << y << z;
        SendMessageToSet(&data, true);
        SetPosition( x, y, z, o );
    }
}

void Unit::SetRedirectThreat(Unit * target, float amount, uint32 Duration)
{
    sLog.printf("TODO: ThreatRedirect\n");
}

void Unit::EventResetRedirectThreat()
{
    sLog.printf("TODO: ThreatRedirect\n");
}

uint32 Unit::GetCreatureType()
{
    if(IsPlayer())
    {
        Player *plr = castPtr<Player>(this);
        if(plr->GetShapeShift())
        {
            if(SpellShapeshiftFormEntry* ssf = dbcSpellShapeshiftForm.LookupEntry(plr->GetShapeShift()))
                return ssf->creatureType;
            return 0;
        }
    }
    return 0;
}

bool Unit::IsSitting()
{
    uint8 s = getStandState();
    if(s == STANDSTATE_SIT)
        return true;
    else if(s == STANDSTATE_SIT_CHAIR)
        return true;
    else if(s == STANDSTATE_SIT_LOW_CHAIR)
        return true;
    else if(s == STANDSTATE_SIT_MEDIUM_CHAIR)
        return true;
    else if(s == STANDSTATE_SIT_HIGH_CHAIR)
        return true;
    return false;
}
