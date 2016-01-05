/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Unit::Unit(uint64 guid, uint32 fieldCount) : WorldObject(guid, fieldCount), m_AuraInterface(this), m_movementInterface(this)
{
    SetTypeFlags(TYPEMASK_TYPE_UNIT);
    m_updateFlags |= UPDATEFLAG_LIVING;

    m_attackInterrupt = 0;
    memset(&m_attackTimer, 0, sizeof(uint16)*3);
    memset(&m_attackDelay, 0, sizeof(uint16)*3);
    m_dualWield = m_autoShot = false;

    baseStats = NULL;
    m_statValuesChanged = false;
    m_needStatRecalculation = true;
    m_needRecalculateAllFields = true;

    m_state = 0;
    m_deathState = ALIVE;
    m_currentSpell = NULL;

    m_silenced = 0;
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

    m_instanceInCombat = false;
    m_combatStopTimer = 0;
    m_attackUpdateTimer = 0;
    m_AreaUpdateTimer = 0;
    m_lastAreaPosition.ChangeCoords(0.0f, 0.0f, 0.0f);
    m_emoteState = 0;
    m_oldEmote = 0;

    pLastSpell = 0;

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

    m_aiInterface.Init(castPtr<Unit>(this), AITYPE_AGRO, MOVEMENTTYPE_NONE);

    // Required regeneration flag
    SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER );

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, M_PI );
    SetFloatValue(UNIT_FIELD_COMBATREACH, IsPlayer() ? 5.f : 1.5f );
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.f);
    SetFloatValue(UNIT_MOD_CAST_HASTE, 1.f);
    SetFloatValue(UNIT_FIELD_HOVERHEIGHT, 0.001f);
}

void Unit::Destruct()
{
    sEventMgr.RemoveEvents(this);

    m_AuraInterface.RemoveAllAuras();

    if (IsInWorld())
        RemoveFromWorld(true);

    if(m_currentSpell)
        m_currentSpell->cancel();

    m_DummyAuras.clear();

    RONIN_MAP<uint32, onAuraRemove*>::iterator itr;
    for ( itr = m_onAuraRemoveSpells.begin() ; itr != m_onAuraRemoveSpells.end() ; itr++)
        delete itr->second;
    m_onAuraRemoveSpells.clear();

    WorldObject::Destruct();
}

void Unit::Update( uint32 p_time )
{
    WorldObject::Update(p_time);
    UpdateFieldValues();

    _UpdateSpells( p_time );
    m_AuraInterface.Update(p_time);

    if(isDead())
        return;

    if(!IsPlayer())
    {
        m_AreaUpdateTimer += p_time;
        if(m_AreaUpdateTimer >= 2000)
        {
            if(m_lastAreaPosition.Distance(GetPosition()) > sWorld.AreaUpdateDistance)
            {
                // Update our area id and position
                UpdateAreaInfo();
                m_lastAreaPosition = GetPosition();
            }
            m_AreaUpdateTimer = 0;
        }
    }

    if(!m_instanceInCombat && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT))
    {
        if(m_combatStopTimer <= p_time)
        {
            m_combatStopTimer = 0;
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
        } else m_combatStopTimer -= p_time;
    }

    if(!isCasting())
    {
        m_attackUpdateTimer += p_time;
        if(m_attackUpdateTimer >= 200)
        {
            m_attackUpdateTimer = 0;

            if(m_attackInterrupt)
            {
                if(m_attackInterrupt > p_time)
                    m_attackInterrupt -= p_time;
                else m_attackInterrupt = 0;
            }

            if(m_attackInterrupt == 0)
            {
                if(m_attackTarget.empty())
                {
                    if(m_attackDelay[0])
                        m_attackTimer[0] = std::min<uint32>(m_attackDelay[0], m_attackTimer[0]+p_time);
                    if(m_dualWield && m_attackDelay[1])
                        m_attackTimer[1] = std::min<uint32>(m_attackDelay[1], m_attackTimer[1]+p_time);
                    if(m_attackTimer[2] <= p_time)
                        m_attackTimer[2] = 0;
                    else m_attackTimer[2] -= p_time;
                }
                else
                {
                    Unit *target = GetInRangeObject<Unit>(m_attackTarget);
                    if(!validateAttackTarget(target))
                        EventAttackStop();
                    else
                    {
                        if(m_attackDelay[0])
                        {
                            m_attackTimer[0] += p_time;
                            if(m_attackTimer[0] >= m_attackDelay[0])
                            {
                                m_attackTimer[0] = m_attackDelay[0];
                                if(canReachWithAttack(MELEE, castPtr<Unit>(target)))
                                {
                                    EventAttack(target, MELEE);
                                    m_attackTimer[0] = 0;
                                    if(m_dualWield && m_attackTimer[1] > 800)
                                        m_attackTimer[1] -= 800;
                                    else m_attackTimer[1] = 0;
                                }
                            }
                        }

                        if(m_dualWield && m_attackDelay[1])
                        {
                            m_attackTimer[1] += p_time;
                            if(m_attackTimer[1] >= m_attackDelay[1])
                            {
                                m_attackTimer[1] = m_attackDelay[1];
                                if(canReachWithAttack(OFFHAND, castPtr<Unit>(target)))
                                {
                                    EventAttack(target, OFFHAND);
                                    m_attackTimer[1] = 0;
                                }
                            }
                        }

                        if(m_autoShotSpell && m_attackDelay[2])
                        {
                            if(m_attackTimer[2] <= p_time)
                                m_attackTimer[2] = 0;
                            else m_attackTimer[2] -= p_time;

                            if( m_autoShot && m_attackTimer[2] == 0 )
                            {
                                if(canReachWithAttack(RANGED_AUTOSHOT, castPtr<Unit>(target), m_autoShotSpell->Id))
                                {
                                    EventAttack(target, RANGED);
                                    m_attackTimer[2] = m_attackDelay[2];
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /*-----------------------POWER & HP REGENERATION-----------------*/
    if(!isFullHealth())
    {
        if( m_H_regenTimer <= p_time )
        {
            m_H_regenTimer = 1000;//set next regen time
            RegenerateHealth(IsInCombat());
        } else m_H_regenTimer -= p_time;
    }

    m_P_regenTimer += p_time;
    if(m_P_regenTimer >= 1000)
    {
        if(m_p_DelayTimer > m_P_regenTimer)
            m_p_DelayTimer -= m_P_regenTimer;
        else m_p_DelayTimer = 0;

        RegeneratePower( m_p_DelayTimer > 0 );
        m_P_regenTimer = 0;
    }

    m_aiInterface.Update(p_time);
    m_movementInterface.Update(p_time);
}

void Unit::UpdateFieldValues()
{
    // Update base stats first
    UpdateStatValues();
    UpdateHealthValues();
    UpdatePowerValues();
    UpdateRegenValues();
    UpdateAttackTimeValues();
    UpdateResistanceValues();
    UpdateAttackPowerValues();
    UpdateRangedAttackPowerValues();
    UpdateAttackDamageValues();
    UpdatePowerCostValues();
    UpdateHoverValues();
    ClearFieldUpdateValues();
}

void Unit::ClearFieldUpdateValues()
{
    m_needRecalculateAllFields = m_needStatRecalculation = m_statValuesChanged = false;
    m_AuraInterface.ClearModMaskBits();
}

bool Unit::StatUpdateRequired()
{
    return m_needRecalculateAllFields|m_needStatRecalculation;
    bool res = m_needRecalculateAllFields|m_needStatRecalculation;
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_STAT);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_PERCENT_STAT);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
    return res;
}

bool Unit::HealthUpdateRequired()
{
    return m_needRecalculateAllFields|m_statValuesChanged;
    bool res = m_needRecalculateAllFields|m_statValuesChanged;
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_BASE_HEALTH_PCT);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_INCREASE_HEALTH);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_INCREASE_MAX_HEALTH);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_INCREASE_HEALTH_2);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT);
    return res;
}

bool Unit::PowerUpdateRequired()
{
    return m_needRecalculateAllFields|m_statValuesChanged;
    bool res = m_needRecalculateAllFields|m_statValuesChanged;
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_INCREASE_ENERGY);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT);
    return res;
}

bool Unit::RegenUpdateRequired()
{
    return m_needRecalculateAllFields|m_statValuesChanged;
    bool res = m_needRecalculateAllFields|m_statValuesChanged;
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_POWER_REGEN);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
    return res;
}

bool Unit::AttackTimeUpdateRequired(uint8 weaponType)
{
    return m_needRecalculateAllFields;
    bool res = m_needRecalculateAllFields;
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_ATTACKSPEED);
    return res;
}

bool Unit::AttackDamageUpdateRequired(uint8 weaponType)
{
    return m_needRecalculateAllFields|m_statValuesChanged;
    bool res = m_needRecalculateAllFields|m_statValuesChanged|AttackTimeUpdateRequired(weaponType);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_DAMAGE_DONE);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);

    return res;
}

bool Unit::APUpdateRequired()
{
    return m_needRecalculateAllFields|m_statValuesChanged;
    bool res = m_needRecalculateAllFields|m_statValuesChanged;
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_ATTACK_POWER_PCT);
    return res;
}

bool Unit::RAPUpdateRequired()
{
    return m_needRecalculateAllFields|m_statValuesChanged;
    bool res = m_needRecalculateAllFields|m_statValuesChanged;
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT);
    return res;
}

bool Unit::ResUpdateRequired()
{
    return m_needRecalculateAllFields;
    bool res = m_needRecalculateAllFields;
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_RESISTANCE);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_RESISTANCE_PCT);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_BASE_RESISTANCE);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_BASE_RESISTANCE_PCT);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE);
    return res;
}

void Unit::UpdateStatValues()
{
    if(!StatUpdateRequired())
        return;

    int32 basepos,pos,neg;
    AuraInterface::modifierMap statMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_STAT),
        statPCTMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_PERCENT_STAT),
        totalStatMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
    for(uint8 s = 0; s < MAX_STAT; s++)
    {
        basepos = baseStats ? baseStats->baseStat[s] : getLevel()*15;

        pos=GetBonusStat(s), neg=0;
        for(AuraInterface::modifierMap::iterator itr = statMod.begin(); itr != statMod.end(); itr++)
        {
            if(itr->second->m_miscValue[0] == -1 || itr->second->m_miscValue[0] == s)
            {
                if(itr->second->m_amount > 0)
                    pos += itr->second->m_amount;
                else neg -= itr->second->m_amount;
            }
        }
        for(AuraInterface::modifierMap::iterator itr = statPCTMod.begin(); itr != statPCTMod.end(); itr++)
        {
            if(itr->second->m_miscValue[0] == -1 || itr->second->m_miscValue[0] == s)
            {
                if(itr->second->m_amount > 0)
                    pos = float(pos)*(float(itr->second->m_amount)/100.0f);
                else neg = float(neg)*(float(abs(itr->second->m_amount))/100.0f);
            }
        }

        int32 res = basepos+pos+neg;
        if(res > 0)
        {
            for(AuraInterface::modifierMap::iterator itr = totalStatMod.begin(); itr != totalStatMod.end(); itr++)
            {
                if(itr->second->m_miscValue[0] == -1 || itr->second->m_miscValue[0] == s)
                {
                    if(itr->second->m_amount > 0)
                        pos += float(res)*(float(itr->second->m_amount)/100.0f);
                    else neg -= float(res)*(float(abs(itr->second->m_amount))/100.0f);
                }
            }
            res = basepos+pos+neg;
        }

        SetUInt32Value(UNIT_FIELD_STATS+s, res > 0 ? res : 0);
        SetUInt32Value(UNIT_FIELD_POSSTATS+s, pos);
        SetUInt32Value(UNIT_FIELD_NEGSTATS+s, neg);
    }
    // Set stat values as updated to update affected auras
    m_statValuesChanged = true;
}

void Unit::UpdateHealthValues()
{
    if(!HealthUpdateRequired())
        return;

    uint32 HP = baseStats ? baseStats->baseHP : 20;
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, HP);
    AuraInterface::modifierMap increaseHPMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_BASE_HEALTH_PCT);
    for(AuraInterface::modifierMap::iterator itr = increaseHPMod.begin(); itr != increaseHPMod.end(); itr++)
        HP *= float(abs(itr->second->m_amount))/100.f;

    int32 stam = GetStat(STAT_STAMINA), baseStam = stam < 20 ? stam : 20;
    stam = (stam <= baseStam ? 0 : stam-baseStam);
    HP += GetBonusHealth() + baseStam;
    if(gtFloat *HPPerStam = dbcHPPerStam.LookupEntry((getClass()-1)*MAXIMUM_ATTAINABLE_LEVEL+(getLevel()-1)))
        HP += stam*HPPerStam->val;
    increaseHPMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_INCREASE_HEALTH);
    for(AuraInterface::modifierMap::iterator itr = increaseHPMod.begin(); itr != increaseHPMod.end(); itr++)
        HP += itr->second->m_amount;
    increaseHPMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_INCREASE_MAX_HEALTH);
    for(AuraInterface::modifierMap::iterator itr = increaseHPMod.begin(); itr != increaseHPMod.end(); itr++)
        HP += itr->second->m_amount;
    increaseHPMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_INCREASE_HEALTH_2);
    for(AuraInterface::modifierMap::iterator itr = increaseHPMod.begin(); itr != increaseHPMod.end(); itr++)
        HP += itr->second->m_amount;
    increaseHPMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT);
    for(AuraInterface::modifierMap::iterator itr = increaseHPMod.begin(); itr != increaseHPMod.end(); itr++)
        HP *= float(abs(itr->second->m_amount))/100.f;

    HP *= GetHealthMod();
    if(GetUInt32Value(UNIT_FIELD_HEALTH) > HP)
        SetUInt32Value(UNIT_FIELD_HEALTH, HP);
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, HP );
}

static uint32 basePowerValues[POWER_TYPE_MAX] = { 0, 1000, 100, 100, 1050000, 1000, 6, 3, 100, 3 };
void Unit::UpdatePowerValues()
{
    if(!PowerUpdateRequired())
        return;

    uint32 power = baseStats ? baseStats->basePower : 0;
    if(power)
    {
        SetUInt32Value(UNIT_FIELD_BASE_MANA, power);

        int32 intellect = GetStat(STAT_INTELLECT), baseIntellect = intellect < 20 ? intellect : 20;
        intellect = intellect <= baseIntellect ? 0 : intellect-baseIntellect;
        power += GetBonusMana() + baseIntellect + intellect*15.0f;
        AuraInterface::modifierMap increaseEnergyMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_INCREASE_ENERGY);
        for(AuraInterface::modifierMap::iterator itr = increaseEnergyMod.begin(); itr != increaseEnergyMod.end(); itr++)
            if(itr->second->m_miscValue[0] == 0) power += itr->second->m_amount;
        increaseEnergyMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT);
        for(AuraInterface::modifierMap::iterator itr = increaseEnergyMod.begin(); itr != increaseEnergyMod.end(); itr++)
            if(itr->second->m_miscValue[0] == 0) power *= float(abs(itr->second->m_amount))/100.f;

        power *= GetPowerMod();
        if(GetPower(POWER_TYPE_MANA) > power)
            SetPower(POWER_TYPE_MANA, power);
        SetMaxPower(POWER_TYPE_MANA, power);
    }

    if(uint8 powerType = getPowerType())
    {
        power = basePowerValues[powerType];
        if(powerType <= POWER_TYPE_RUNIC)
        {
            AuraInterface::modifierMap increaseEnergyMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_INCREASE_ENERGY);
            for(AuraInterface::modifierMap::iterator itr = increaseEnergyMod.begin(); itr != increaseEnergyMod.end(); itr++)
                if(itr->second->m_miscValue[0] == powerType) power += itr->second->m_amount;
            increaseEnergyMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT);
            for(AuraInterface::modifierMap::iterator itr = increaseEnergyMod.begin(); itr != increaseEnergyMod.end(); itr++)
                if(itr->second->m_miscValue[0] == powerType) power *= float(abs(itr->second->m_amount))/100.f;
        }

        if(GetPower(powerType) > power)
            SetPower(powerType, power);
        SetMaxPower(powerType, power);
    }
}

static uint32 baseRegenValues[POWER_TYPE_MAX] = { 0, 10, 5, 5, 250, 10, 0, 0, 0, 0 };
void Unit::UpdateRegenValues()
{
    if(!RegenUpdateRequired())
        return;

    float base_regen = GetUInt32Value(UNIT_FIELD_BASE_MANA) * 0.01f;
    if(base_regen)
    {
        float spirit_regen = sqrt(GetStat(STAT_INTELLECT));

        uint32 level = std::min(getLevel(), uint32(100));
        if(gtFloat *ratio = dbcManaRegenBase.LookupEntry((getClass()-1)*100+level-1))
            spirit_regen *= GetStat(STAT_SPIRIT)*ratio->val;

        AuraInterface::modifierMap regenMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_POWER_REGEN);
        for(AuraInterface::modifierMap::iterator itr = regenMod.begin(); itr != regenMod.end(); itr++)
            if(itr->second->m_miscValue[0] == POWER_TYPE_MANA)
                base_regen += itr->second->m_amount;
        regenMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
        for(AuraInterface::modifierMap::iterator itr = regenMod.begin(); itr != regenMod.end(); itr++)
            if(itr->second->m_miscValue[0] == POWER_TYPE_MANA)
                spirit_regen *= itr->second->m_amount;

        int32 interruptMod = 1;
        regenMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
        for(AuraInterface::modifierMap::iterator itr = regenMod.begin(); itr != regenMod.end(); itr++)
            interruptMod += itr->second->m_amount;
        if (interruptMod > 100) interruptMod = 100;
        SetFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER, base_regen + 0.001f + spirit_regen);
        SetFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER, base_regen + (spirit_regen * (interruptMod / 100.0f)));
    }

    if(uint8 powerType = getPowerType())
    {
        if(base_regen = baseRegenValues[powerType])
        {
            int32 interruptMod = 1, regen_value = 0;
            AuraInterface::modifierMap regenMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_POWER_REGEN);
            for(AuraInterface::modifierMap::iterator itr = regenMod.begin(); itr != regenMod.end(); itr++)
                if(itr->second->m_miscValue[0] == powerType)
                    regen_value += itr->second->m_amount;
            regenMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
            for(AuraInterface::modifierMap::iterator itr = regenMod.begin(); itr != regenMod.end(); itr++)
                if(itr->second->m_miscValue[0] == powerType)
                    base_regen *= itr->second->m_amount;

            regenMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
            for(AuraInterface::modifierMap::iterator itr = regenMod.begin(); itr != regenMod.end(); itr++)
                interruptMod += itr->second->m_amount;
            if (interruptMod > 100)
                interruptMod = 100;

            SetFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER+powerType, regen_value+base_regen + 0.001f);
            SetFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER+powerType, regen_value+float2int32(floor(base_regen * (interruptMod / 100.f))));
        }
    }
}

void Unit::UpdateAttackTimeValues()
{
    uint8 updateMask = 0x00;
    for(uint8 i = 0; i < 3; i++)
    {
        if(!AttackTimeUpdateRequired(i))
            continue;

        uint32 baseAttack = GetBaseAttackTime(i);
        if(baseAttack || i == 0) // Force an attack time for mainhand
        {
            float attackSpeedMod = 1.0f;
            AuraInterface::modifierMap attackTimeMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_ATTACKSPEED);
            for(AuraInterface::modifierMap::iterator itr = attackTimeMod.begin(); itr != attackTimeMod.end(); itr++)
                attackSpeedMod += float(abs(itr->second->m_amount))/100.f;

            if(i == RANGED) attackTimeMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RANGED_HASTE);
            else attackTimeMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MELEE_HASTE);
            for(AuraInterface::modifierMap::iterator itr = attackTimeMod.begin(); itr != attackTimeMod.end(); itr++)
                attackSpeedMod += float(abs(itr->second->m_amount))/100.f;

            if(IsPlayer() && i == RANGED) attackSpeedMod *= (1.f+(castPtr<Player>(this)->CalcRating(PLAYER_RATING_MODIFIER_RANGED_HASTE)/100.f));
            else if(IsPlayer()) attackSpeedMod *= (1.f+(castPtr<Player>(this)->CalcRating(PLAYER_RATING_MODIFIER_MELEE_HASTE)/100.f));

            baseAttack = float2int32(floor(float(baseAttack)/attackSpeedMod));
            if(baseAttack < 500)
                baseAttack = 500;
            else if(baseAttack > 12000)
                baseAttack = 12000;
        }
        updateMask |= 1<<i;
        SetUInt32Value(UNIT_FIELD_BASEATTACKTIME+i, baseAttack);
    }

    if(updateMask > 0)
        resetAttackDelay(updateMask);
}

static uint32 minAttackPowers[3] = { UNIT_FIELD_MINDAMAGE, UNIT_FIELD_MINOFFHANDDAMAGE, UNIT_FIELD_MINRANGEDDAMAGE };
void Unit::UpdateAttackDamageValues()
{
    uint32 attackPower = CalculateAttackPower(), rangedAttackPower = CalculateRangedAttackPower();
    for(uint8 i = 0; i < 3; i++)
    {
        if(!AttackDamageUpdateRequired(i))
            continue;

        if(GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+i) == 0)
        {
            SetFloatValue(minAttackPowers[i], 0);
            SetFloatValue(minAttackPowers[i]+1, 0);
            continue;
        }

        float apBonus = float(GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+i))/1000.f;
        float baseMinDamage = GetBaseMinDamage(i), baseMaxDamage = GetBaseMaxDamage(i);
        if(IsInFeralForm())
        {
            uint32 level = std::min(getLevel(), uint32(60));
            baseMinDamage = level*0.85f*apBonus;
            baseMaxDamage = level*1.25f*apBonus;
        }
        if(i != RANGED) apBonus *= attackPower/14.f;
        else apBonus *= rangedAttackPower/14.f;
        AuraInterface::modifierMap damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DAMAGE_DONE);
        for(AuraInterface::modifierMap::iterator itr = damageMod.begin(); itr != damageMod.end(); itr++)
            if(itr->second->m_miscValue[0] & 0x01)
                baseMinDamage += itr->second->m_amount, baseMaxDamage += itr->second->m_amount;
        baseMinDamage += apBonus, baseMaxDamage += apBonus;

        damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        for(AuraInterface::modifierMap::iterator itr = damageMod.begin(); itr != damageMod.end(); itr++)
        {
            if(IsPlayer() && itr->second->m_spellInfo->EquippedItemClass != -1)
            {
                if(Item *item = castPtr<Player>(this)->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+i))
                    if(itr->second->m_spellInfo->EquippedItemClass == item->GetProto()->SubClass)
                        baseMinDamage *= itr->second->m_amount, baseMaxDamage *= itr->second->m_amount;
                continue;
            }
            if(itr->second->m_miscValue[0] & 0x01)
                baseMinDamage *= itr->second->m_amount, baseMaxDamage *= itr->second->m_amount;
        }

        // Offhand weapons do 50% of actual damage
        if(i == OFFHAND)
        {
            baseMinDamage *= 0.5f;
            baseMaxDamage *= 0.5f;
        }

        SetFloatValue(minAttackPowers[i], baseMinDamage);
        SetFloatValue(minAttackPowers[i]+1, baseMaxDamage);
    }
}

void Unit::UpdateResistanceValues()
{
    if(!ResUpdateRequired())
        return;

    int32 basepos,baseneg,pos,neg;
    AuraInterface::modifierMap resistMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RESISTANCE),
        ResistPCTMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RESISTANCE_PCT),
        baseResistMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_BASE_RESISTANCE),
        baseResistPCTMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_BASE_RESISTANCE_PCT),
        exclusiveResistMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE),
        statbasedResistMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT);
    for(uint8 s = 0; s < MAX_RESISTANCE; s++)
    {
        basepos=baseneg=pos=neg=0;
        for(AuraInterface::modifierMap::iterator itr = baseResistMod.begin(); itr != baseResistMod.end(); itr++)
        {
            if(itr->second->m_miscValue[0] & (1 << s))
            {
                if(itr->second->m_amount > 0)
                    basepos += itr->second->m_amount;
                else baseneg -= itr->second->m_amount;
            }
        }
        for(AuraInterface::modifierMap::iterator itr = baseResistPCTMod.begin(); itr != baseResistPCTMod.end(); itr++)
        {
            if(itr->second->m_miscValue[0] & (1 << s))
            {
                if(itr->second->m_amount > 0)
                    basepos = float(basepos)*(float(itr->second->m_amount)/100.0f);
                else baseneg = float(baseneg)*(float(abs(itr->second->m_amount))/100.0f);
            }
        }
        pos += GetBonusResistance(s);
        for(AuraInterface::modifierMap::iterator itr = resistMod.begin(); itr != resistMod.end(); itr++)
        {
            if(itr->second->m_miscValue[0] & (1 << s))
            {
                if(itr->second->m_amount > 0)
                    pos += itr->second->m_amount;
                else neg -= itr->second->m_amount;
            }
        }
        for(AuraInterface::modifierMap::iterator itr = ResistPCTMod.begin(); itr != ResistPCTMod.end(); itr++)
        {
            if(itr->second->m_miscValue[0] & (1 << s))
            {
                if(itr->second->m_amount > 0)
                    pos = float(pos)*(float(itr->second->m_amount)/100.0f);
                else neg = float(neg)*(float(abs(itr->second->m_amount))/100.0f);
            }
        }
        if(statbasedResistMod.size())
        {
            float statMod[5] = {0,0,0,0,0};
            for(AuraInterface::modifierMap::iterator itr = statbasedResistMod.begin(); itr != statbasedResistMod.end(); itr++)
                if(itr->second->m_miscValue[0] & (1 << s))
                    statMod[itr->second->m_miscValue[1]] += float(itr->second->m_amount)/100.f;
            for(uint8 i = 0; i < 5; i++)
                pos += GetStat(i)*statMod[i];
        }
        for(AuraInterface::modifierMap::iterator itr = exclusiveResistMod.begin(); itr != exclusiveResistMod.end(); itr++)
        {
            if(itr->second->m_miscValue[0] & (1 << s))
            {
                if(itr->second->m_amount >= 0)
                    pos = ((pos < itr->second->m_amount) ? itr->second->m_amount : pos);
                else neg = ((neg > itr->second->m_amount) ? itr->second->m_amount : neg);
            }
        }

        int32 res = basepos+baseneg+pos+neg;
        SetUInt32Value(UNIT_FIELD_RESISTANCES+s, res > 0 ? res : 0);
        SetUInt32Value(UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+s, pos);
        SetUInt32Value(UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+s, neg);
    }
}

void Unit::UpdateAttackPowerValues()
{
    if(!APUpdateRequired())
        return;

    if(m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_ATTACK_POWER_PCT))
    {
        float val = 100.0f;
        AuraInterface::modifierMap hoverMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_ATTACK_POWER_PCT);
        for(AuraInterface::modifierMap::iterator itr = hoverMod.begin(); itr != hoverMod.end(); itr++)
            val += float(itr->second->m_amount);
        SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, val/100.0f);
    }

    int32 attackPower = GetBonusAttackPower();
    switch(getClass())
    {
    case DRUID: { attackPower += GetStrength() * 2 - 20; }break;
    case HUNTER: case ROGUE: case SHAMAN: { attackPower += GetStrength()+GetAgility()+getLevel()*2-20; }break;
    case WARRIOR: case DEATHKNIGHT: case PALADIN: { attackPower += GetStrength()*2+getLevel()*3-20; }break;
    default: { attackPower += GetAgility() - 10; }break;
    }

    SetUInt32Value(UNIT_FIELD_ATTACK_POWER, attackPower);
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_POS, 0);
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_NEG, 0);
}

void Unit::UpdateRangedAttackPowerValues()
{
    if(!RAPUpdateRequired())
        return;

    if(m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT))
    {
        float val = 100.0f;
        AuraInterface::modifierMap hoverMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT);
        for(AuraInterface::modifierMap::iterator itr = hoverMod.begin(); itr != hoverMod.end(); itr++)
            val += float(itr->second->m_amount);
        SetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER, val/100.0f);
    }

    int32 rangedAttackPower = GetBonusRangedAttackPower();
    switch (getClass())
    {
    case WARRIOR: case DEATHKNIGHT: case ROGUE: { rangedAttackPower += getLevel()+GetAgility()-10; }break;
    case HUNTER: { rangedAttackPower += getLevel()*2+GetAgility()-10; }break;
    default: { rangedAttackPower += GetAgility()-10; }break;
    }

    SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER, rangedAttackPower);
    SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS, rangedAttackPower);
    SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG, 0);
}

void Unit::UpdatePowerCostValues()
{
    if(m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_POWER_COST_SCHOOL))
    {
        AuraInterface::modifierMap powerCostMods = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_POWER_COST_SCHOOL);
        for(uint8 s = 0; s < MAX_RESISTANCE; s++)
        {
            int32 val = 0;
            for(AuraInterface::modifierMap::iterator itr = powerCostMods.begin(); itr != powerCostMods.end(); itr++)
            {
                if(itr->second->m_miscValue[0] & (1 << s))
                    val += itr->second->m_amount;
            }
            SetFloatValue(UNIT_FIELD_POWER_COST_MODIFIER+s,val);
        }
    }

    if(m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_POWER_COST))
    {
        AuraInterface::modifierMap powerCostMods = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_POWER_COST);
        for(uint8 s = 0; s < MAX_RESISTANCE; s++)
        {
            float val = 0.f;
            for(AuraInterface::modifierMap::iterator itr = powerCostMods.begin(); itr != powerCostMods.end(); itr++)
            {
                if(itr->second->m_miscValue[0] & (1 << s))
                    val += itr->second->m_amount;
            }
            SetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+s,val/100.0f);
        }
    }
}

void Unit::UpdateHoverValues()
{
    if(!m_AuraInterface.GetModMaskBit(SPELL_AURA_HOVER))
        return;

    float val = 0.001f;
    AuraInterface::modifierMap hoverMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_HOVER);
    for(AuraInterface::modifierMap::iterator itr = hoverMod.begin(); itr != hoverMod.end(); itr++)
        val += float(itr->second->m_amount)/2.0f;
    SetFloatValue(UNIT_FIELD_HOVERHEIGHT, val);
}

int32 Unit::GetDamageDoneMod(uint8 school)
{
    int32 res = 0;
    if(IsPlayer())
    {
        res += GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+school);
        res -= GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG+school);
        return res;
    }
    else if(IsSummon() || IsPet())
    {
        WorldObject *summoner = NULL;
        if(IsPet()) summoner = castPtr<Pet>(this)->GetOwner();
        else summoner = castPtr<Summon>(this)->GetSummonOwner();
        /// To avoid processing loops, do not count pets of pets or summons of summons
        if(!(summoner->IsSummon() || summoner->IsPet()) && GetsDamageBonusFromOwner(school))
            res += castPtr<Unit>(summoner)->GetDamageDoneMod(school);
    }

    AuraInterface::modifierMap damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DAMAGE_DONE);
    for(AuraInterface::modifierMap::iterator itr = damageMod.begin(); itr != damageMod.end(); itr++)
        if(itr->second->m_miscValue[0] & (1<<school))
            res += itr->second->m_amount;
    if(school != SCHOOL_NORMAL)
    {
        if((damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT)).size())
        {
            float statMods[5] = {0,0,0,0,0};
            for(AuraInterface::modifierMap::iterator itr = damageMod.begin(); itr != damageMod.end(); itr++)
                if(itr->second->m_miscValue[0] & (1<<school))
                    statMods[itr->second->m_miscValue[1]] += float(itr->second->m_amount)/100.f;
            for(uint8 i = 0; i < 5; i++)
                if(statMods[i])
                    res += statMods[i]*GetStat(i);
        }

        if((damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER)).size())
        {
            float attackPowerMod = 0.0f;
            for(AuraInterface::modifierMap::iterator itr = damageMod.begin(); itr != damageMod.end(); itr++)
                if(itr->second->m_miscValue[0] & (1<<school))
                    attackPowerMod += float(itr->second->m_amount)/100.f;
            res += float2int32(float(CalculateAttackPower())*attackPowerMod);
        }
    }
    return res;
}

float Unit::GetDamageDonePctMod(uint8 school)
{
    // If we're a player, this is already precalculated
    if(IsPlayer()) return GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+school);

    float result = 1.f;
    AuraInterface::modifierMap damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for(AuraInterface::modifierMap::iterator itr = damageMod.begin(); itr != damageMod.end(); itr++)
        if(itr->second->m_miscValue[0] & (1<<school))
            result += itr->second->m_amount;
    return result;
}

int32 Unit::GetHealingDoneMod()
{
    // If we're a player, this is already precalculated
    if(IsPlayer()) return GetUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS);

    int32 result = 0;
    AuraInterface::modifierMap healingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_HEALING_DONE);
    for(AuraInterface::modifierMap::iterator itr = healingMod.begin(); itr != healingMod.end(); itr++)
        result += itr->second->m_amount;

    if((healingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT)).size())
    {
        float statMods[5] = {0,0,0,0,0};
        for(AuraInterface::modifierMap::iterator itr = healingMod.begin(); itr != healingMod.end(); itr++)
            statMods[itr->second->m_miscValue[1]] += float(itr->second->m_amount)/100.f;
        for(uint8 i = 0; i < 5; i++)
            if(statMods[i])
                result += statMods[i]*GetStat(i);
    }

    if((healingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER)).size())
    {
        float attackPowerMod = 0.0f;
        for(AuraInterface::modifierMap::iterator itr = healingMod.begin(); itr != healingMod.end(); itr++)
            attackPowerMod += float(itr->second->m_amount)/100.f;
        result += float2int32(float(CalculateAttackPower())*attackPowerMod);
    }
    return result;
}

float Unit::GetHealingDonePctMod()
{
    // If we're a player, this is already precalculated
    if(IsPlayer()) return GetFloatValue(PLAYER_FIELD_MOD_HEALING_PCT);

    float result = 1.f;
    AuraInterface::modifierMap healingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
    for(AuraInterface::modifierMap::iterator itr = healingMod.begin(); itr != healingMod.end(); itr++)
        result += itr->second->m_amount;
    return result;
}

uint32 Unit::GetMechanicDispels(uint8 mechanic)
{
    uint32 count = m_AuraInterface.GetModMapByModType(SPELL_AURA_ADD_CREATURE_IMMUNITY).size();
    if( mechanic == 16 || mechanic == 19 || mechanic == 25 || mechanic == 31 ) count = 0;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MECHANIC_IMMUNITY);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if(itr->second->m_miscValue[0] == mechanic) count++;
    if(mechanic == MECHANIC_POLYMORPHED && GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID))
        count++;
    return count;
}

float Unit::GetMechanicResistPCT(uint8 mechanic)
{
    float resist = 0.0f;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MECHANIC_RESISTANCE);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if(itr->second->m_miscValue[0] == mechanic)
            resist += itr->second->m_amount;
    return resist;
}

float Unit::GetDamageTakenByMechPCTMod(uint8 mechanic)
{
    float resist = 0.0f;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if(itr->second->m_miscValue[0] == mechanic)
            resist += float(itr->second->m_amount)/100.f;
    return resist;
}

float Unit::GetMechanicDurationPctMod(uint8 mechanic)
{
    float resist = 1.f;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MECHANIC_DURATION_MOD);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if(itr->second->m_miscValue[0] == mechanic) resist *= float(itr->second->m_amount)/100.f;
    modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
    {
        if(itr->second->m_miscValue[0] == mechanic)
        {
            float val = float(itr->second->m_amount)/100.f;
            if(resist < val)
                resist = val;
        }
    }
    modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL_NOT_STACK);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
    {
        if(itr->second->m_miscValue[0] == mechanic)
        {
            float val = float(itr->second->m_amount)/100.f;
            if(resist < val)
                resist = val;
        }
    }
    return resist;
}

uint32 Unit::GetDispelImmunity(uint8 dispel)
{
    uint32 count = 0;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_DISPEL_IMMUNITY);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if(itr->second->m_miscValue[0] == dispel) count++;
    return count;
}

float Unit::GetDispelResistancesPCT(uint8 dispel)
{
    float resist = 0.0f;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DEBUFF_RESISTANCE);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if(itr->second->m_miscValue[0] == dispel)
            resist += itr->second->m_amount;
    return resist;
}

int32 Unit::GetCreatureRangedAttackPowerMod(uint32 creatureType)
{
    if(creatureType == 0)
        return 0;

    int32 mod = 0;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if(itr->second->m_miscValue[0] & (1<<creatureType-1))
            mod += itr->second->m_amount;
    return mod;
}

int32 Unit::GetCreatureAttackPowerMod(uint32 creatureType)
{
    if(creatureType == 0)
        return 0;

    int32 mod = 0;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_CREATURE_ATTACK_POWER);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if(itr->second->m_miscValue[0] & (1<<creatureType-1))
            mod += itr->second->m_amount;
    return mod;
}

int32 Unit::GetRangedDamageTakenMod()
{
    int32 mod = 0;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        mod += itr->second->m_amount;
    modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        mod *= itr->second->m_amount;
    return mod;
}

float Unit::GetCritMeleeDamageTakenModPct(uint32 school)
{
    float mod = 0.f;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if (itr->second->m_miscValue[0] & (1<<school))
            mod += itr->second->m_amount;
    return mod;
}

float Unit::GetCritRangedDamageTakenModPct(uint32 school)
{
    float mod = 0.f;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if (itr->second->m_miscValue[0] & (1<<school))
            mod += itr->second->m_amount;
    return mod;
}

int32 Unit::GetDamageTakenMod(uint32 school)
{
    int32 mod = 0;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if (itr->second->m_miscValue[0] & (1<<school))
            mod += itr->second->m_amount;
    if(school == 0)
    {
        modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN);
        for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
            mod += itr->second->m_amount;
    }
    return mod;
}

float Unit::GetDamageTakenModPct(uint32 school)
{
    float mod = 1.f;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
        if (itr->second->m_miscValue[0] & (1<<school))
            mod *= float(itr->second->m_amount)/100.f;
    if(school == 0)
    {
        modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT);
        for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
            mod += itr->second->m_amount;
    }
    return mod;
}

float Unit::GetAreaOfEffectDamageMod()
{
    float mod = 1.f;
    AuraInterface::modifierMap modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE);
    for(AuraInterface::modifierMap::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
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
    else if(IsPlayer())
    {
        Player* plr = castPtr<Player>(this);
        if(plr->m_FlyingAura)
            return true;
    }

    return false;
}

bool Unit::validateAttackTarget(WorldObject *target)
{
    if(target == nullptr)
        return false;
    else if(target->IsGameObject())
        return false;
    else if(!target->IsUnit())
        return false;
    else if(!sFactionSystem.isAttackable(this, target, false))
        return false;

    return true;
}

bool Unit::canReachWithAttack(WeaponDamageType attackType, Unit* pVictim, uint32 spellId)
{
    if(GetMapId() != pVictim->GetMapId())
        return false;

    // minimum melee range, UNIT_FIELD_COMBATREACH is too small and used eg. in melee spells
    float selfreach = GetFloatValue(UNIT_FIELD_COMBATREACH), selfradius = GetModelHalfSize();
    float targetradius = pVictim->GetModelHalfSize(), distance = CalcDistance(pVictim);
    float minRange = 0.f, maxRange = targetradius + selfreach + selfradius;
    if(attackType == RANGED || attackType == RANGED_AUTOSHOT)
    {
        maxRange -= (attackType == RANGED_AUTOSHOT ? 0.f : selfreach);
        if(SpellEntry *sp = dbcSpell.LookupEntry(spellId))
        {
            minRange = sp->minRange[0];
            minRange *= minRange;
            float spellRange = sp->maxRange[0];
            if( sp->SpellGroupType )
            {
                SM_FFValue(SMT_RANGE, &spellRange, sp->SpellGroupType );
                SM_PFValue(SMT_RANGE, &spellRange, sp->SpellGroupType );
            }
            maxRange += spellRange;
        } else return false;
    }

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
    return (distance > minRange) && (distance <= maxRange);
}

void Unit::resetAttackTimer(uint8 typeMask)
{
    for(uint8 i = 0; i < 3; i++)
    {
        if((typeMask & 1<<i) == 0)
            continue;

        m_attackTimer[i] = 0;
    }
}

void Unit::resetAttackDelay(uint8 typeMask)
{
    for(uint8 i = 0; i < 3; i++)
    {
        if((typeMask & 1<<i) == 0)
            continue;

        uint32 baseAttackTime = GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+i);
        if(baseAttackTime == 0)
        {
            m_attackDelay[i] = 0;
            continue;
        }

        m_attackDelay[i] = std::max<uint32>(1000, std::min<uint32>(0x7FFF, std::ceil(float(baseAttackTime)*GetCastSpeedMod())));
        if(i == OFFHAND && m_attackDelay[i] > 0)
            m_dualWield = true;
    }
}

void Unit::SetDiminishTimer(uint32 index)
{
    assert(index < DIMINISH_GROUPS);

}

void Unit::setLevel(uint32 level)
{
    m_needStatRecalculation = true;
    m_AuraInterface.OnChangeLevel(level);
    SetUInt32Value(UNIT_FIELD_LEVEL, level);
    baseStats = sStatSystem.GetUnitBaseStats(getRace(), getClass(), level);
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

    //Get Highest Level Player, Calc Xp and give it to each group member
    Player* pHighLvlPlayer = NULL;
    Player* pGroupGuy = NULL;
      int active_player_count=0;
    Player* active_player_list[MAX_GROUP_SIZE_RAID];//since group is small we can afford to do this ratehr then recheck again the whole active player set
    int total_level=0;
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
                }
                else
                    pHighLvlPlayer = pGroupGuy;
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

        xp = CalculateXpToGive(pVictim, PlayerInGroup);
        PlayerInGroup->GiveXP(xp, pVictim->GetGUID(), true);
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

        xp = CalculateXpToGive(pVictim, pHighLvlPlayer);
        //i'm not sure about this formula is correct or not. Maybe some brackets are wrong placed ?
        for(int i=0;i<active_player_count;++i)
            active_player_list[i]->GiveXP( float2int32(((xp*active_player_list[i]->getLevel()) / total_level)*xp_mod), pVictim->GetGUID(), true );
    }
}

bool Unit::isCasting()
{
    return (m_currentSpell != NULL);
}

bool Unit::IsInInstance()
{
    return false;
}

void Unit::RegenerateEnergy()
{
    if( m_interruptRegen > 0 )
        return;

    uint32 cur = GetPower(POWER_TYPE_ENERGY), mp = GetMaxPower(POWER_TYPE_ENERGY);
    if( cur >= mp )
        return;

    cur += float2int32(floor(float(0.01f * m_P_regenTimer * 1.f)));
    SetPower(POWER_TYPE_ENERGY, (cur >= mp) ? mp : cur);
}

void Unit::RegenerateFocus()
{
    if (m_interruptRegen)
        return;

    uint32 cur = GetPower(POWER_TYPE_FOCUS), mp = GetMaxPower(POWER_TYPE_FOCUS);
    if( cur >= mp )
        return;

    cur += float2int32(floor(float(1.0f)));
    SetPower(POWER_TYPE_FOCUS, (cur >= mp)? mp : cur);
}

double Unit::GetResistanceReducion(Unit* pVictim, uint32 school, float armorReducePct)
{
    double reduction = 0.0;
    if(school == 0) // physical
    {
        float armor = pVictim->GetResistance(0);
        if(IsPlayer()) // apply armor penetration
        {
            float maxArmorPen = 0;
            if (pVictim->getLevel() < 60)
                maxArmorPen = float(400 + 85 * pVictim->getLevel());
            else
                maxArmorPen = 400 + 85 * pVictim->getLevel() + 4.5f * 85 * (pVictim->getLevel() - 59);

            // Cap armor penetration to this number
            maxArmorPen = std::min((armor + maxArmorPen) / 3, armor);

            // Figure out how much armor we ignore
            float armorPen = armorReducePct+castPtr<Player>(this)->CalcRating(PLAYER_RATING_MODIFIER_ARMOR_PENETRATION_RATING);

            // Apply armor pen cap to our calculated armor penetration
            armor -= std::min(armorPen, maxArmorPen);
        }

        if (armor < 0.0f)
            armor = 0.0f;

        float levelModifier = getLevel();
        if (levelModifier > 59)
            levelModifier = levelModifier + (4.5f * (levelModifier - 59));
        reduction = 0.1f * armor / (8.5f * levelModifier + 40);
        reduction = reduction / (1.0f + reduction);
    }
    else 
    {   // non-physical
        float resistance = (float) pVictim->GetResistance(school);
        double RResist = resistance + float((pVictim->getLevel() > getLevel()) ? (pVictim->getLevel() - getLevel()) * 5 : 0);
        reduction = RResist / (double)(getLevel() * 5) * 0.75f;
    }

    if(reduction > 0.75)
        reduction = 0.75;
    else if(reduction < 0)
        reduction = 0;
    return reduction;
}

void Unit::CalculateResistanceReduction(Unit* pVictim,dealdamage * dmg, SpellEntry* ability, float armorReducePct)
{
    if( (dmg->school_type && ability && Spell::IsBinary(ability)) || dmg->school_type == SCHOOL_HOLY )  // damage isn't reduced for binary spells
    {
        (*dmg).resisted_damage = 0;
        return;
    }

    double reduction = GetResistanceReducion(pVictim, dmg->school_type, armorReducePct);

    // only for physical or non binary spells
    if(reduction > 0)
        (*dmg).resisted_damage = (uint32)(((*dmg).full_damage)*reduction);
    else
        (*dmg).resisted_damage = 0;
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

    float hitchance         = 0.0f;
    float dodge             = 0.0f;
    float parry             = 0.0f;
    float block             = 0.0f;

    float hitmodifier       = 0;
    int32 self_skill        = 0;
    int32 victim_skill      = 0;
    uint32 SubClassSkill    = SKILL_UNARMED;

    bool backAttack         = pVictim->isTargetInBack(this);
    uint32 vskill           = 0;

    //==========================================================================================
    //==============================Victim Skill Base Calculation===============================
    //==========================================================================================
    if( pVictim->IsPlayer() )
    {
        vskill = castPtr<Player>( pVictim )->_GetSkillLineCurrent( SKILL_DEFENSE );
        if( weapon_damage_type != RANGED )
        {
            if( !backAttack )
            {
//--------------------------------block chance----------------------------------------------
                if( !pVictim->disarmedShield )
                    block = pVictim->GetFloatValue(PLAYER_BLOCK_PERCENTAGE); //shield check already done in Update chances
//--------------------------------dodge chance----------------------------------------------
                if(!pVictim->IsStunned())
                    dodge = pVictim->GetFloatValue( PLAYER_DODGE_PERCENTAGE );
//--------------------------------parry chance----------------------------------------------
                if(true && !disarmed)
                    parry = pVictim->GetFloatValue( PLAYER_PARRY_PERCENTAGE );
            }
        }
        victim_skill = float2int32( vskill + castPtr<Player>( pVictim )->CalcRating( PLAYER_RATING_MODIFIER_DEFENCE ) );
    }
    //--------------------------------mob defensive chances-------------------------------------
    else if(pVictim->IsCreature())
    {
        Creature* c = castPtr<Creature>(pVictim);

        if( weapon_damage_type != RANGED && !pVictim->IsStunned())
            dodge = pVictim->GetUInt32Value(UNIT_FIELD_AGILITY) / 14.5f; // what is this value?

        victim_skill = pVictim->getLevel() * 5;
        if(c && c->GetCreatureData() && (c->GetCreatureData()->rank == ELITE_WORLDBOSS || c->GetCreatureData()->flags & CREATURE_FLAGS1_BOSS))
        {
            victim_skill = std::max(victim_skill,((int32)getLevel()+3)*5); //used max to avoid situation when lowlvl hits boss.
        }

        if( !backAttack )
        {
            if( c->b_has_shield && !pVictim->disarmedShield && pVictim->GetUInt32Value(UNIT_FIELD_STRENGTH))
                block = 5.0f+(victim_skill*0.04f)+(pVictim->GetUInt32Value(UNIT_FIELD_STRENGTH)/2);
            if(true && !disarmed) // VictimSkill*0.04 per point
                parry = (victim_skill*0.04f);
        }
    }
    //==========================================================================================
    //==============================Attacker Skill Base Calculation=============================
    //==========================================================================================
    if(IsPlayer())
    {
        self_skill = 0;
        Player* pr = castPtr<Player>(this);
        hitmodifier = 0.f;

        switch( weapon_damage_type )
        {
        case MELEE:   // melee main hand weapon
            it = disarmed ? NULL : pr->GetInventory()->GetInventoryItem( EQUIPMENT_SLOT_MAINHAND );
            hitmodifier += pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_HIT );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_MAIN_HAND_SKILL ) );
            break;
        case OFFHAND: // melee offhand weapon (dualwield)
            it = disarmed ? NULL : pr->GetInventory()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
            hitmodifier += pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_HIT );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_OFF_HAND_SKILL ) );
            break;
        case RANGED:  // ranged weapon
            it = disarmed ? NULL : pr->GetInventory()->GetInventoryItem( EQUIPMENT_SLOT_RANGED );
            hitmodifier += pr->CalcRating( PLAYER_RATING_MODIFIER_RANGED_HIT );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_WEAPON_SKILL ) );
            break;
        }

        // erm. some spells don't use ranged weapon skill but are still a ranged spell and use melee stats instead
        // i.e. hammer of wrath
        if( ability && (ability->NameHash == SPELL_HASH_HAMMER_OF_WRATH || ability->NameHash == SPELL_HASH_AVENGER_S_SHIELD) )
        {
            it = pr->GetInventory()->GetInventoryItem( EQUIPMENT_SLOT_MAINHAND );
            hitmodifier += pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_HIT );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_MAIN_HAND_SKILL ) );
        }

        SubClassSkill = it ? sItemMgr.GetSkillForItem(it->GetProto()) : SKILL_UNARMED;

        //chances in feral form don't depend on weapon skill
        if(castPtr<Player>(this)->IsInFeralForm())
        {
            uint8 form = castPtr<Player>(this)->GetShapeShift();
            if(form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR)
            {
                SubClassSkill = SKILL_FERAL_COMBAT;
                // Adjust skill for Level * 5 for Feral Combat
                self_skill += pr->getLevel() * 5;
            }
        }

        self_skill += pr->_GetSkillLineCurrent(SubClassSkill);
    }
    else
    {
        self_skill = getLevel() * 5;
        if(GetTypeId() == TYPEID_UNIT)
        {
            Creature* c = castPtr<Creature>(this);
            if(c && c->GetCreatureData() && (c->GetCreatureData()->rank == ELITE_WORLDBOSS || c->GetCreatureData()->flags & CREATURE_FLAGS1_BOSS))
                self_skill = std::max(self_skill,((int32)pVictim->getLevel()+3)*5);//used max to avoid situation when lowlvl hits boss.
        }
    }
    //==========================================================================================
    //==============================Special Chances Base Calculation============================
    //==========================================================================================
    //<THE SHIT> to avoid Linux bug.
    float diffVcapped = (float)self_skill;
    if(int32(pVictim->getLevel()*5)>victim_skill)
        diffVcapped -=(float)victim_skill;
    else diffVcapped -=(float)(pVictim->getLevel()*5);

    float diffAcapped = (float)victim_skill;
    if(int32(getLevel()*5)>self_skill)
        diffAcapped -=(float)self_skill;
    else diffAcapped -=(float)(getLevel()*5);
    //<SHIT END>

    //--------------------------------by victim state-------------------------------------------
    if(pVictim->IsPlayer()&&pVictim->GetStandState()) //every not standing state is >0
        hitchance = 100.0f;

    //--------------------------------by damage type and by weapon type-------------------------
    if( weapon_damage_type == RANGED )
    {
        dodge = 0.0f;
        parry = 0.0f;
    }
    else if(IsPlayer())
    {
        it = castPtr<Player>(this)->GetInventory()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
        if( it != NULL && (it->GetProto()->InventoryType == INVTYPE_WEAPON ||
            it->GetProto()->InventoryType == INVTYPE_2HWEAPON) && !ability )//dualwield to-hit penalty
        {
            hitmodifier -= 19.0f;
        }
    }

    //--------------------------------by skill difference---------------------------------------
    float vsk = (float)self_skill - (float)victim_skill;
    if( dodge )
        dodge = std::max( 0.0f, dodge - vsk * 0.04f );
    if( parry )
        parry = std::max( 0.0f, parry - vsk * 0.04f );
    if( block )
        block = std::max( 0.0f, block - vsk * 0.04f );

    if( pVictim->IsPlayer() )
    {
        if( vsk > 0 )
            hitchance = std::max( hitchance, 95.0f + vsk * 0.02f);
        else
            hitchance = std::max( hitchance, 95.0f + vsk * 0.04f);
    }
    else
    {
        if(vsk >= -10 && vsk <= 10)
            hitchance = std::max( hitchance, 95.0f + vsk * 0.1f);
        else
            hitchance = std::max( hitchance, 93.0f + (vsk - 10.0f) * 0.4f);
    }
    hitchance += hitmodifier;

    if( ability && ability->SpellGroupType )
    {
        SM_FFValue( SMT_HITCHANCE, &hitchance, ability->SpellGroupType );
        SM_PFValue( SMT_HITCHANCE, &hitchance, ability->SpellGroupType );
    }

    // overpower nana
    if( ability != NULL && ability->isUnstoppableForce() )
    {
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }

    //==========================================================================================
    //==============================One Roll Processing=========================================
    //==========================================================================================
    //--------------------------------cummulative chances generation----------------------------
    uint32 r = 0;
    float chances[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    chances[0] = std::max(0.0f, 100.0f-hitchance);
    if(!backAttack)
    {
        chances[1] = chances[0]+dodge;
        chances[2] = chances[1]+parry;
        chances[3] = chances[2]+block;
    }
    else if(pVictim->IsCreature())
        chances[1] = chances[0]+dodge;

    //--------------------------------roll------------------------------------------------------
    float Roll = RandomFloat(100.0f);
    while (r < 4 && Roll > chances[r])
        r++;
    return roll_results[r];
}

uint32 Unit::GetSpellDidHitResult( uint32 index, Unit* pVictim, Spell* pSpell, uint8 *reflectout )
{
    if(reflectout) *reflectout = SPELL_DID_HIT_MISS;
    //
    float baseresist[3] = { 4.0f, 5.0f, 6.0f };
    int32 lvldiff           = 0;
    float hitchance         = 0.0f;
    float resistchance      = 0.0f;
    SpellEntry* m_spellEntry = pSpell->GetSpellProto();
    lvldiff = pVictim->getLevel() - getLevel();
    if(lvldiff > 5) // Crow: This has to be capped, in retail level 50's hit level 80's in dungeons with spells a lot.
        lvldiff = 5;
    if (lvldiff < 3)
        hitchance = 96 - lvldiff;
    else
        hitchance = 94 - (lvldiff - 2) * (pVictim->IsPlayer() ? 7 : 11);

    if(m_spellEntry->SpellGroupType)
    {
        SM_FFValue(SMT_HITCHANCE, &hitchance, m_spellEntry->SpellGroupType);
        SM_PFValue(SMT_HITCHANCE, &hitchance, m_spellEntry->SpellGroupType);
    }

    //rating bonus
    if( IsPlayer() )
        hitchance += castPtr<Player>(this)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_HIT );

    // 160: Mod AOE avoidance implementation needed.

    if(!Rand(hitchance))
        return SPELL_DID_HIT_MISS;

    /************************************************************************/
    /* Check if the spell is resisted.                                    */
    /************************************************************************/
    if( m_spellEntry->School == SCHOOL_NORMAL  || m_spellEntry->isSpellRangedSpell() ) // all ranged spells are physical too...
        return SPELL_DID_HIT_SUCCESS;
    if( m_spellEntry->isIgnorantOfHitResult() )
        return SPELL_DID_HIT_SUCCESS;

    resistchance = 100.0f-hitchance;
    if (m_spellEntry->DispelType < NUM_DISPELS)
        resistchance += pVictim->GetDispelResistancesPCT(m_spellEntry->DispelType);

    // Our resist to dispel
    if( m_spellEntry->Effect[index] == SPELL_EFFECT_DISPEL && m_spellEntry->SpellGroupType)
    {
        pVictim->SM_FFValue(SMT_RESIST_DISPEL, &resistchance, m_spellEntry->SpellGroupType);
        pVictim->SM_PFValue(SMT_RESIST_DISPEL, &resistchance, m_spellEntry->SpellGroupType);
    }

    if(Spell::IsBinary(m_spellEntry))
    { // need to apply resistance mitigation
        float mitigation = 1.0f - float (GetResistanceReducion(pVictim, m_spellEntry->School, 0.0f));
        resistchance = 100 - (100 - resistchance) * mitigation; // meaning hitchance * mitigation
    }

    if(resistchance < 1.0f)
        resistchance = 1.0f;

    if(resistchance > 99.0f)
        resistchance = 99.0f;

    if (m_spellEntry->isUnstoppableForce2())
        resistchance = 0.0f;

    if( m_spellEntry->isSpellResistanceIgnorant() )
        resistchance = 0.0f;

    uint32 res = Rand(resistchance) ? SPELL_DID_HIT_RESIST : SPELL_DID_HIT_SUCCESS;
    if(res == SPELL_DID_HIT_SUCCESS && pSpell->Reflect(pVictim))
    {
        res = SPELL_DID_HIT_REFLECT;
        if(reflectout) *reflectout = SPELL_DID_HIT_SUCCESS;
    }

    return res;
}

void Unit::Strike( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability, uint8 abEffindex, int32 add_damage, int32 pct_dmg_mod, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool proc_extrastrike )
{
//==========================================================================================
//==============================Unacceptable Cases Processing===============================
//==========================================================================================
    if(!pVictim->isAlive() || !isAlive()  || IsStunned() || IsPacified() || IsFeared())
        return;

    if(!isTargetInFront(pVictim))
    {
        if(IsPlayer())
        {
            if( !(ability && ability->isSpellBackAttackCapable()) )
            {
                castPtr<Player>(this)->GetSession()->OutPacket(SMSG_ATTACKSWING_BADFACING);
                return;
            }
        }
    }
//==========================================================================================
//==============================Variables Initialization====================================
//==========================================================================================
    dealdamage dmg            = {0,0,0};

    Item* it = NULL;

    float armorreducepct    = 0.f;

    float hitchance         = 0.0f;
    float dodge             = 0.0f;
    float parry             = 0.0f;
    float glanc             = 0.0f;
    float block             = 0.0f;
    float crit              = 0.0f;
    float crush             = 0.0f;

    uint32 targetEvent      = 0;
    uint32 hit_status       = 0;

    uint32 blocked_damage   = 0;
    int32  realdamage       = 0;

    uint32 vstate           = 1;

    float hitmodifier       = 0;
    int32 self_skill;
    int32 victim_skill;
    uint32 SubClassSkill    = SKILL_UNARMED;

    bool backAttack         = pVictim->isTargetInBack(this);
    uint32 vskill           = 0;
    bool disable_dR         = false;

    dmg.school_type = SCHOOL_NORMAL;
    if(ability)
        dmg.school_type = ability->School;
    else if (GetTypeId() == TYPEID_UNIT)
        dmg.school_type = castPtr<Creature>(this)->GetCreatureData()->attackType;

//==========================================================================================
//==============================Victim Skill Base Calculation===============================
//==========================================================================================
    if(pVictim->IsPlayer())
    {
        vskill = castPtr<Player>( pVictim )->_GetSkillLineCurrent( SKILL_DEFENSE );
        if( !backAttack )
        {
            if( weapon_damage_type != RANGED )
            {
//--------------------------------block chance----------------------------------------------
                if( !pVictim->disarmedShield )
                    block = pVictim->GetFloatValue(PLAYER_BLOCK_PERCENTAGE); //shield check already done in Update chances
//--------------------------------dodge chance----------------------------------------------
                if(!pVictim->IsStunned())
                    dodge = pVictim->GetFloatValue( PLAYER_DODGE_PERCENTAGE );
//--------------------------------parry chance----------------------------------------------
                if(true && !disarmed)
                    parry = pVictim->GetFloatValue( PLAYER_PARRY_PERCENTAGE );
            }
        }
        victim_skill = float2int32( vskill + castPtr<Player>( pVictim )->CalcRating( 1 ) );
    }
//--------------------------------mob defensive chances-------------------------------------
    else
    {
        if( weapon_damage_type != RANGED )
            dodge = pVictim->GetUInt32Value( UNIT_FIELD_AGILITY ) / 14.5f; // what is this value? (Agility)

        victim_skill = pVictim->getLevel() * 5;
        if( pVictim->GetTypeId() == TYPEID_UNIT )
        {
            Creature* c = castPtr<Creature>( pVictim );
            if( c != NULL && c->GetCreatureData() && (c->GetCreatureData()->rank == ELITE_WORLDBOSS || c->GetCreatureData()->flags & CREATURE_FLAGS1_BOSS) )
            {
                victim_skill = std::max( victim_skill, ( (int32)getLevel() + 3 ) * 5 ); //used max to avoid situation when lowlvl hits boss.
            }
        }
    }
//==========================================================================================
//==============================Attacker Skill Base Calculation=============================
//==========================================================================================
    if( IsPlayer() )
    {
        self_skill = 0;
        Player* pr = castPtr<Player>(this);
        hitmodifier = 0;

        switch( weapon_damage_type )
        {
        case MELEE:   // melee main hand weapon
            it = disarmed ? NULL : pr->GetInventory()->GetInventoryItem( EQUIPMENT_SLOT_MAINHAND );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_MAIN_HAND_SKILL ) );
            if (it && it->GetProto())
                dmg.school_type = it->GetProto()->DamageType;
            break;
        case OFFHAND: // melee offhand weapon (dualwield)
            it = disarmed ? NULL : pr->GetInventory()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_OFF_HAND_SKILL ) );
            hit_status |= HITSTATUS_DUALWIELD;//animation
            if (it && it->GetProto())
                dmg.school_type = it->GetProto()->DamageType;
            break;
        case RANGED:  // ranged weapon
            it = disarmed ? NULL : pr->GetInventory()->GetInventoryItem( EQUIPMENT_SLOT_RANGED );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_WEAPON_SKILL ) );
            if (it && it->GetProto())
                dmg.school_type = it->GetProto()->DamageType;
            break;
        }

        SubClassSkill = it ? sItemMgr.GetSkillForItem(it->GetProto()) : SKILL_UNARMED;

        //chances in feral form don't depend on weapon skill
        if(pr->IsInFeralForm())
        {
            uint8 form = pr->GetShapeShift();
            if(form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR)
            {
                SubClassSkill = SKILL_FERAL_COMBAT;
                self_skill += pr->getLevel() * 5;
            }
        }

        self_skill += pr->_GetSkillLineCurrent(SubClassSkill);
        crit = GetFloatValue(PLAYER_CRIT_PERCENTAGE);
    }
    else
    {
        self_skill = getLevel() * 5;
        if(GetTypeId() == TYPEID_UNIT)
        {
            Creature* c = castPtr<Creature>(this);
            if(c && c->GetCreatureData() && (c->GetCreatureData()->rank == ELITE_WORLDBOSS || c->GetCreatureData()->flags & CREATURE_FLAGS1_BOSS))
                self_skill = std::max(self_skill,((int32)pVictim->getLevel()+3)*5);//used max to avoid situation when lowlvl hits boss.
        }
        crit = 5.0f; //will be modified later
    }

//==========================================================================================
//==============================Special Chances Base Calculation============================
//==========================================================================================
    //<THE SHIT> to avoid Linux bug.
    float diffVcapped = (float)self_skill;
    if(int32(pVictim->getLevel()*5)>victim_skill)
        diffVcapped -=(float)victim_skill;
    else
        diffVcapped -=(float)(pVictim->getLevel()*5);

    float diffAcapped = (float)victim_skill;
    if(int32(getLevel()*5)>self_skill)
        diffAcapped -=(float)self_skill;
    else
        diffAcapped -=(float)(getLevel()*5);
    //<SHIT END>

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
//--------------------------------by skill difference---------------------------------------
    float vsk = (float)self_skill - (float)victim_skill;
    dodge = std::max( 0.0f, dodge - vsk * 0.04f );
    if( parry ) parry = std::max( 0.0f, parry - vsk * 0.04f );
    if( block ) block = std::max( 0.0f, block - vsk * 0.04f );

    crit += pVictim->IsPlayer() ? vsk * 0.04f : std::min( vsk * 0.2f, 0.0f );

    if( pVictim->IsPlayer() )
    {
        if( vsk > 0 )
            hitchance = std::max( hitchance, 95.0f + vsk * 0.02f);
        else hitchance = std::max( hitchance, 95.0f + vsk * 0.04f);
    }
    else
    {
        if(vsk >= -10 && vsk <= 10)
            hitchance = std::max( hitchance, 95.0f + vsk * 0.1f);
        else hitchance = std::max( hitchance, 93.0f + (vsk - 10.0f) * 0.4f);
    }
//--------------------------------by ratings------------------------------------------------
    crit -= pVictim->IsPlayer() ? castPtr<Player>(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE ) : 0.0f;
    if(crit < 0.0f)
        crit = 0.0f;

    if (IsPlayer())
    {
        if(weapon_damage_type == RANGED)
            hitmodifier += castPtr<Player>(this)->CalcRating( PLAYER_RATING_MODIFIER_RANGED_HIT );
        else
        {
            hitmodifier += castPtr<Player>(this)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_HIT );

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
        dodge = parry = glanc = 0.0f;
    else if(IsPlayer())
    {
        it = castPtr<Player>(this)->GetInventory()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
        if( it != NULL && (it->GetProto()->InventoryType == INVTYPE_WEAPON ||
            it->GetProto()->InventoryType == INVTYPE_2HWEAPON) && !ability )//dualwield to-hit penalty
        {
            hitmodifier -= 19.0f;
        }
    }

    hitchance += hitmodifier;

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
    {
        hitchance = 100.0f;
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }

    if( ability != NULL )
    {
        if( ability->NameHash == SPELL_HASH_FEROCIOUS_BITE && HasDummyAura( SPELL_HASH_REND_AND_TEAR ) && pVictim->m_AuraInterface.HasNegAuraWithMechanic(MECHANIC_BLEEDING) )
            crit += 5.0f * GetDummyAura(SPELL_HASH_REND_AND_TEAR )->RankNumber;
        else if( pVictim->IsStunned() && ability->Id == 20467 )
            crit = 100.0f;
        else if( ability->isUnstoppableForce() )
        {
            dodge = 0.0f;
            parry = 0.0f;
            block = 0.0f;
        }
    }

//--------------------------------by victim state-------------------------------------------
    if(pVictim->IsPlayer() && pVictim->GetStandState()) //every not standing state is >0
    {
        hitchance = 100.0f;
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
        crush = 0.0f;
        crit = 100.0f;
    }
    else if(backAttack)
    {
        if(pVictim->IsPlayer())
            dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }
//==========================================================================================
//==============================One Roll Processing=========================================
//==========================================================================================
//--------------------------------cummulative chances generation----------------------------
    float chances[7];
    chances[0]=std::max(0.0f,100.0f-hitchance);
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
    {
        r++;
    }
//--------------------------------postroll processing---------------------------------------
    uint32 abs = 0;

    switch(r)
    {
//--------------------------------miss------------------------------------------------------
    case 0:
        hit_status |= HITSTATUS_MISS;
        // dirty ai agro fix
        if(pVictim->GetTypeId() == TYPEID_UNIT && pVictim->GetAIInterface()->GetNextTarget() == NULL)
            pVictim->GetAIInterface()->AttackReaction(castPtr<Unit>(this), 1, 0);
        break;
//--------------------------------dodge-----------------------------------------------------
    case 1:
        // dirty ai agro fix
        if(pVictim->GetTypeId() == TYPEID_UNIT && pVictim->GetAIInterface()->GetNextTarget() == NULL)
            pVictim->GetAIInterface()->AttackReaction(castPtr<Unit>(this), 1, 0);

        TRIGGER_AI_EVENT(pVictim, OnTargetDodged)(castPtr<Unit>(this));
        TRIGGER_AI_EVENT(castPtr<Unit>(this), OnDodged)(castPtr<Unit>(this));
        targetEvent = 1;
        vstate = DODGE;
        pVictim->Emote(EMOTE_ONESHOT_PARRY_UNARMED);         // Animation
        //allmighty warrior overpower

        if( IsPlayer() )
        {
            Player* plr = castPtr<Player>( this );
            if( plr->getClass() == DEATHKNIGHT )
            {
                CastSpell(GetGUID(), 56817, true);  // client side aura enabling Rune Strike
            }
        }
        if( pVictim->IsPlayer() )
        {
            Player* vplr = castPtr<Player>( pVictim );
            if( vplr->getClass() == DRUID )
            {
                if( (vplr->GetShapeShift() == FORM_BEAR ||
                     vplr->GetShapeShift() == FORM_DIREBEAR) &&
                     vplr->HasDummyAura(SPELL_HASH_NATURAL_REACTION) )
                {
                    switch(vplr->GetDummyAura(SPELL_HASH_NATURAL_REACTION)->RankNumber)
                    {
                    case 1: vplr->CastSpell(vplr, 57893, true); break;
                    case 2: vplr->CastSpell(vplr, 59071, true); break;
                    case 3: vplr->CastSpell(vplr, 59072, true); break;
                    }
                }
            }
            else if( vplr->getClass() == ROGUE )
            {
                if( vplr->HasDummyAura(SPELL_HASH_SETUP) )
                {
                    if( Rand((vplr->GetDummyAura(SPELL_HASH_SETUP)->RankNumber * 33) + 1) )
                        vplr->CastSpell(vplr, 15250, true);
                }
            }
        }

        pVictim->SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_DODGE_BLOCK);  //SB@L: Enables spells requiring dodge
        if(!sEventMgr.HasEvent(pVictim,EVENT_DODGE_BLOCK_FLAG_EXPIRE))
            sEventMgr.AddEvent(pVictim,&Unit::EventAurastateExpire,(uint32)AURASTATE_FLAG_DODGE_BLOCK,EVENT_DODGE_BLOCK_FLAG_EXPIRE,5000,1,0);
        else sEventMgr.ModifyEventTimeLeft(pVictim,EVENT_DODGE_BLOCK_FLAG_EXPIRE,5000,0);
        break;
//--------------------------------parry-----------------------------------------------------
    case 2:
        // dirty ai agro fix
        if(pVictim->GetTypeId() == TYPEID_UNIT && pVictim->GetAIInterface()->GetNextTarget() == NULL)
            pVictim->GetAIInterface()->AttackReaction(castPtr<Unit>(this), 1, 0);

        TRIGGER_AI_EVENT(pVictim, OnTargetParried)(castPtr<Unit>(this));
        TRIGGER_AI_EVENT(castPtr<Unit>(this), OnParried)(castPtr<Unit>(this));
        targetEvent = 3;
        vstate = PARRY;
        pVictim->Emote(EMOTE_ONESHOT_PARRY_UNARMED);         // Animation
        if(pVictim->IsPlayer())
        {
            if( castPtr<Player>( pVictim )->getClass() == 1 || castPtr<Player>( pVictim )->getClass() == 4 )//warriors for 'revenge' and rogues for 'riposte'
            {
                //pVictim->SetFlag( UNIT_FIELD_AURASTATE,AURASTATE_FLAG_DODGE_BLOCK );  //SB@L: Enables spells requiring dodge
                if(!sEventMgr.HasEvent( pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE ) )
                    sEventMgr.AddEvent( pVictim, &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_DODGE_BLOCK, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0 );
                else
                    sEventMgr.ModifyEventTimeLeft( pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000 );
            }

            pVictim->SetFlag( UNIT_FIELD_AURASTATE,AURASTATE_FLAG_PARRY );  //SB@L: Enables spells requiring parry
            if(!sEventMgr.HasEvent( pVictim, EVENT_PARRY_FLAG_EXPIRE ) )
                sEventMgr.AddEvent( pVictim, &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_PARRY,EVENT_PARRY_FLAG_EXPIRE, 5000, 1, 0 );
            else
                sEventMgr.ModifyEventTimeLeft( pVictim, EVENT_PARRY_FLAG_EXPIRE, 5000 );
        }
        break;
//--------------------------------not miss,dodge or parry-----------------------------------
    default:
        hit_status |= HITSTATUS_HITANIMATION;//hit animation on victim
        {
//--------------------------------base damage calculation-----------------------------------
            if(exclusive_damage)
                dmg.full_damage = exclusive_damage;
            else dmg.full_damage = sStatSystem.CalculateDamage( castPtr<Unit>(this), pVictim, weapon_damage_type, ability );

            if( weapon_damage_type == RANGED )
                dmg.full_damage += pVictim->GetRangedDamageTakenMod();

            if( ability && ability->MechanicsType == MECHANIC_BLEEDING )
                disable_dR = true;

            if( pct_dmg_mod > 0 )
                dmg.full_damage = float2int32( dmg.full_damage *  ( float( pct_dmg_mod) / 100.0f ) );

            dmg.full_damage += add_damage;

            // Bonus damage
            if( ability )
                dmg.full_damage = GetSpellBonusDamage(pVictim, ability, abEffindex, dmg.full_damage, false);
            else
            {
                dmg.full_damage += GetDamageDoneMod(SCHOOL_NORMAL);
                dmg.full_damage *= pVictim->GetDamageTakenModPct(SCHOOL_NORMAL);
            }

            //pet happiness state dmg modifier
            if( IsPet() && !castPtr<Pet>(this)->IsSummonedPet() )
                dmg.full_damage = ( dmg.full_damage <= 0 ) ? 0 : float2int32( dmg.full_damage * castPtr<Pet>(this)->GetHappinessDmgMod() );

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
                    float low_dmg_mod = 1.5f - (0.05f * diffAcapped);
                    if(getClass() == MAGE || getClass() == PRIEST || getClass() == WARLOCK) //casters = additional penalty.
                    {
                        low_dmg_mod -= 0.7f;
                    }
                    if(low_dmg_mod<0.01)
                        low_dmg_mod = 0.01f;
                    if(low_dmg_mod>0.91)
                        low_dmg_mod = 0.91f;
                    float high_dmg_mod = 1.2f - (0.03f * diffAcapped);
                    if(getClass() == MAGE || getClass() == PRIEST || getClass() == WARLOCK) //casters = additional penalty.
                    {
                        high_dmg_mod -= 0.3f;
                    }
                    if(high_dmg_mod>0.99)
                        high_dmg_mod = 0.99f;
                    if(high_dmg_mod<0.2)
                        high_dmg_mod = 0.2f;

                    float damage_reduction = (high_dmg_mod + low_dmg_mod) / 2.0f;
                    if(damage_reduction > 0)
                    {
                            dmg.full_damage = float2int32(damage_reduction * float(dmg.full_damage));
                    }
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
                        pVictim->Emote(EMOTE_ONESHOT_PARRY_SHIELD);// Animation

                        if( shield->GetProto()->InventoryType == INVTYPE_SHIELD )
                        {
                            float block_multiplier = 1.f;
                            if( block_multiplier < 1.f )
                                block_multiplier = 1.f;

                            blocked_damage = pVictim->GetUInt32Value(PLAYER_SHIELD_BLOCK);
                            hit_status |= HITSTATUS_BLOCK;
                        }
                        else
                        {
                            blocked_damage = 0;
                        }

                        if(dmg.full_damage <= (int32)blocked_damage)
                        {
                            TRIGGER_AI_EVENT(pVictim, OnTargetBlocked)(castPtr<Unit>(this), blocked_damage);
                            TRIGGER_AI_EVENT(castPtr<Unit>(this), OnBlocked)(pVictim, blocked_damage);
                            vstate = BLOCK;
                        }
                        if( pVictim->IsPlayer() )//not necessary now but we'll have blocking mobs in future
                        {
                            pVictim->SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_DODGE_BLOCK);  //SB@L: Enables spells requiring dodge
                            if(!sEventMgr.HasEvent(pVictim,EVENT_DODGE_BLOCK_FLAG_EXPIRE))
                                sEventMgr.AddEvent(pVictim,&Unit::EventAurastateExpire,(uint32)AURASTATE_FLAG_DODGE_BLOCK,EVENT_DODGE_BLOCK_FLAG_EXPIRE,5000,1,0);
                            else sEventMgr.ModifyEventTimeLeft(pVictim,EVENT_DODGE_BLOCK_FLAG_EXPIRE,5000);
                        }
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
                    {
                        SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_CRITICAL);  //SB@L: Enables spells requiring critical strike
                        if(!sEventMgr.HasEvent( this ,EVENT_CRIT_FLAG_EXPIRE))
                            sEventMgr.AddEvent( castPtr<Unit>( this ),&Unit::EventAurastateExpire,(uint32)AURASTATE_FLAG_CRITICAL,EVENT_CRIT_FLAG_EXPIRE,5000,1,0);
                        else
                            sEventMgr.ModifyEventTimeLeft( this ,EVENT_CRIT_FLAG_EXPIRE,5000);
                    }

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
            if(dmg.full_damage > (int32)blocked_damage && !disable_dR)
            {
//--------------------------armor reducing--------------------------------------------------
                CalculateResistanceReduction(pVictim,&dmg, ability,armorreducepct);
            }

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
    if(realdamage > 0 && ability == 0)
    {
        if( IsPlayer() && castPtr<Player>(this)->m_onStrikeSpells.size() )
        {
            SpellCastTargets targets;
            targets.m_unitTarget = pVictim->GetGUID();
            targets.m_targetMask = 0x2;

            // Loop on hit spells, and strike with those.
            for( std::map< SpellEntry*, std::pair< uint32, uint32 > >::iterator itr = castPtr<Player>(this)->m_onStrikeSpells.begin();
                itr != castPtr<Player>(this)->m_onStrikeSpells.end(); itr++ )
            {
                if( itr->second.first )
                {
                    // We have a *periodic* delayed spell.
                    uint32 t = getMSTime();
                    if( t > itr->second.second )  // Time expired
                    {
                        // Set new time
                        itr->second.second = t + itr->second.first;
                    }

                }

                // Cast.
                if(Spell *cspell = new Spell(this, itr->first))
                    cspell->prepare(&targets, true);
            }
        }

        if( IsPlayer() && castPtr<Player>(this)->m_onStrikeSpellDmg.size() )
        {
            std::map< uint32, OnHitSpell >::iterator it2 = castPtr<Player>(this)->m_onStrikeSpellDmg.begin();
            std::map< uint32, OnHitSpell >::iterator itr;
            uint32 min_dmg, max_dmg, range, dmg;
            for(; it2 != castPtr<Player>(this)->m_onStrikeSpellDmg.end(); )
            {
                itr = it2;
                ++it2;

                min_dmg = itr->second.mindmg;
                max_dmg = itr->second.maxdmg;
                range = min_dmg - max_dmg;
                dmg = min_dmg;
                if(range) range += RandomUInt(range);

                SpellNonMeleeDamageLog(pVictim, itr->second.spellid, dmg, true);
            }
        }
    }

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

        if(pVictim->GetCurrentSpell())
            pVictim->GetCurrentSpell()->AddTime(0);
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
        val *= ( 1 + ( castPtr<Player>(this)->rageFromDamageDealt / 100.0f ) );
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
}

void Unit::EventAttack( Unit *target, WeaponDamageType attackType )
{
    if (!GetOnMeleeSpell() || attackType == OFFHAND)
        Strike( target, attackType, NULL, 0, 0, 0, false, false, true);
    else if(SpellEntry *spellInfo = dbcSpell.LookupEntry( GetOnMeleeSpell() ))
    {
        SpellCastTargets targets(target->GetGUID());
        if(Spell *spell = new Spell(this, spellInfo, GetOnMeleeSpellCN() ))
            spell->prepare( &targets, true);
    }
    ClearNextMeleeSpell();
}

void Unit::EventAttackStart(WoWGuid guid)
{
    m_attackTarget = guid;
    smsg_AttackStart(m_attackTarget);
    Dismount();
}

void Unit::EventAttackStop()
{
    smsg_AttackStop(m_attackTarget);
    m_attackTarget.Clean();
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

void Unit::smsg_AttackStop(Unit* pVictim)
{
    if(pVictim == NULL)
        return;

    WorldPacket data(SMSG_ATTACKSTOP, 20);
    data << GetGUID().asPacked();
    data << pVictim->GetGUID().asPacked();
    data << uint32(0);
    SendMessageToSet(&data, IsPlayer());
}

void Unit::_UpdateSpells( uint32 time )
{
    if(m_currentSpell != NULL)
        m_currentSpell->Update(time);
}

void Unit::CastSpell( Spell* pSpell )
{
    // check if we have a spell already casting etc
    if(m_currentSpell && pSpell != m_currentSpell)
        sEventMgr.AddEvent(castPtr<Unit>(this), &Unit::EventCancelSpell, m_currentSpell, EVENT_UNK, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    m_currentSpell = pSpell;
    pLastSpell = pSpell->GetSpellProto();
}

int32 Unit::GetSpellBonusDamage(Unit* pVictim, SpellEntry *spellInfo, uint8 effIndex, int32 base_dmg, bool healing)
{
    int32 bonus_damage = base_dmg;
    Unit* caster = castPtr<Unit>(this);
    uint32 school = spellInfo->School;
    float summaryPCTmod = 0.0f;
    float levelPenalty = CalculateLevelPenalty(spellInfo);

    if( caster->IsPet() )
        caster = castPtr<Unit>(castPtr<Pet>(caster)->GetPetOwner());
    else if( caster->IsSummon() && castPtr<Summon>(caster)->GetSummonOwner() )
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
    float coefficient = IsCreature() ? 1.0f : 0.0f;
    if(spellInfo->School) coefficient += spellInfo->EffectBonusCoefficient[effIndex];

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
    if(spellInfo->School == SCHOOL_NORMAL)
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

    int32 res = float2int32((bonus_damage) * summaryPCTmod) + bonus_damage;
    if( res < 0 )
        res = 0;
    return res;
}

float Unit::CalculateLevelPenalty(SpellEntry* sp)
{
    if (sp->spellLevel <= 0 || sp->spellLevel >= sp->maxLevel || sp->maxLevel >= MAXIMUM_CEXPANSION_LEVEL)
        return 1.0f; // For custom content purposes, do not reduce damage done by the highest available teir of spells.

    float LvlPenalty = 0.0f;
    if (sp->spellLevel < 20)
        LvlPenalty = 20.0f - sp->spellLevel * 3.75f;
    float LvlFactor = (float(sp->spellLevel) + 6.0f) / float(getLevel());
    if (LvlFactor > 1.0f)
        LvlFactor = 1.0f;

    return float((100.0f - LvlPenalty) * LvlFactor / 100.0f);
}

void Unit::InterruptCurrentSpell()
{
    if(m_currentSpell)
        m_currentSpell->cancel();
}

void Unit::DeMorph()
{
    // hope it solves it :)
    uint32 displayid = GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    SetUInt32Value(UNIT_FIELD_DISPLAYID, displayid);
    EventModelChange();
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
    plr->GetSession()->SendPacket(&data);
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

void Unit::OnRemoveInRangeObject(WorldObject* pObj)
{
    if(pObj->IsUnit())
    {
        Unit* pUnit = castPtr<Unit>(pObj);
        if(GetAIInterface())
            GetAIInterface()->CheckTarget(pUnit);

        if(pObj->GetGUID() == GetUInt64Value(UNIT_FIELD_CHARM))
            if(m_currentSpell) m_currentSpell->cancel();
    }
    WorldObject::OnRemoveInRangeObject(pObj);
}

//Events
void Unit::EventAddEmote(EmoteType emote, uint32 time)
{
    m_oldEmote = GetUInt32Value(UNIT_NPC_EMOTESTATE);
    SetUInt32Value(UNIT_NPC_EMOTESTATE,emote);
    sEventMgr.AddEvent(castPtr<Unit>(this), &Unit::EmoteExpire, EVENT_UNIT_EMOTE, time, 1,0);
}

void Unit::EventAllowCombat(bool allow)
{
    m_aiInterface.SetAllowedToEnterCombat(allow);
    m_aiInterface.setCanMove(allow);
}

void Unit::EmoteExpire()
{
    SetUInt32Value(UNIT_NPC_EMOTESTATE, m_oldEmote);
    sEventMgr.RemoveEvents(this, EVENT_UNIT_EMOTE);
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
    AuraInterface::modifierMap absorbMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_SCHOOL_ABSORB);
    for(AuraInterface::modifierMap::iterator itr = absorbMap.begin(); itr != absorbMap.end(); itr++)
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
        castPtr<Player>(this)->GetSession()->OutPacket( SMSG_STANDSTATE_UPDATE, 1, &standstate );
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
            summonPet->RemoveFromWorld(false, true);
            summonPet->Destruct();
            summonPet = NULL;
        }
    }*/
    sEventMgr.RemoveEvents(this, EVENT_SUMMON_PET_EXPIRE);
}

void Unit::CastSpell(Unit* Target, SpellEntry* Sp, bool triggered, uint32 forcedCastTime)
{
    if( Sp == NULL )
        return;

    Spell* newSpell = new Spell(this, Sp);
    SpellCastTargets targets(Target ? Target->GetGUID() : 0);
    if(Target == NULL) newSpell->GenerateTargets(&targets);
    newSpell->prepare(&targets, triggered);
}

void Unit::CastSpell(Unit* Target, uint32 SpellID, bool triggered, uint32 forcedCastTime)
{
    SpellEntry * ent = dbcSpell.LookupEntry(SpellID);
    if(ent == 0)
        return;

    CastSpell(Target, ent, triggered, forcedCastTime);
}

void Unit::CastSpell(uint64 targetGuid, SpellEntry* Sp, bool triggered, uint32 forcedCastTime)
{
    if( Sp == NULL )
        return;

    SpellCastTargets targets(targetGuid);
    if(Spell* newSpell = new Spell(this, Sp))
        newSpell->prepare(&targets, triggered);
}

void Unit::CastSpell(uint64 targetGuid, uint32 SpellID, bool triggered, uint32 forcedCastTime)
{
    SpellEntry * ent = dbcSpell.LookupEntry(SpellID);
    if(ent == 0)
        return;

    CastSpell(targetGuid, ent, triggered, forcedCastTime);
}

uint8 Unit::CastSpellAoF(float x,float y,float z,SpellEntry* Sp, bool triggered, uint32 forcedCastTime)
{
    if( Sp == NULL )
        return SPELL_FAILED_ERROR;

    /*creature will not cast spells while moving, just interrupts itself all the time*/
    if(!IsPlayer() && (IsStunned() || IsPacified() || !isAlive() || m_silenced))
        return SPELL_FAILED_INTERRUPTED;

    SpellCastTargets targets;
    targets.m_dest.ChangeCoords(x, y, z);
    targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
    Spell* newSpell = new Spell(this, Sp);
    return newSpell->prepare(&targets, triggered);
}

void Unit::PlaySpellVisual(uint64 target, uint32 spellVisual)
{
    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 12);
    data << target << spellVisual;
    SendMessageToSet(&data, true);
}

void Unit::OnPushToWorld()
{
    m_AuraInterface.BuildAllAuraUpdates();
}

void Unit::RemoveFromWorld(bool free_guid)
{
    SummonExpireAll(false);

    if(GetInRangePlayerCount())
    {
        for(WorldObject::InRangeSet::iterator itr = GetInRangePlayerSetBegin(); itr != GetInRangePlayerSetEnd(); itr++)
        {
            Player *plr = GetInRangeObject<Player>(*itr);
            if(plr->GetSelection() == GetGUID())
            {
                plr->smsg_AttackStop(this);
                plr->SetSelection(0);
                plr->SetUInt64Value(UNIT_FIELD_TARGET, 0);
            }
        }
    }

    // Delete AAura's from our targets (must be done before object is removed from world)
    m_AuraInterface.RemoveAllAurasWithSpEffect(SPELL_EFFECT_APPLY_AREA_AURA);
    m_AuraInterface.RemoveAllAurasWithSpEffect(SPELL_EFFECT_APPLY_AREA_AURA_FRIEND);
    m_AuraInterface.RemoveAllAurasWithSpEffect(SPELL_EFFECT_APPLY_AREA_AURA_ENEMY);

    if(m_currentSpell != NULL)
    {
        m_currentSpell->cancel();
        m_currentSpell = NULL;
    }

    WorldObject::RemoveFromWorld(free_guid);
    m_aiInterface.WipeReferences();
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
    uint32 count;
    ByteBuffer buffer(2500);
    bool can_see, is_visible;
    WorldObject::InRangeWorldObjectSet::iterator itr, it3;
    if( GetTypeId() == TYPEID_PLAYER )
    {
        WorldObject* pObj;
        Player *plr = castPtr<Player>(this), *pl = NULL;
        for( WorldObject::InRangeMap::iterator itr = m_inRangeObjects.begin(); itr != m_inRangeObjects.end();)
        {
            pObj = itr->second;
            ++itr;

            can_see = plr->CanSee(pObj), is_visible = plr->GetVisibility(pObj, &it3);
            if(can_see && !is_visible)
            {
                plr->AddVisibleObject(pObj);
                if(count = pObj->BuildCreateUpdateBlockForPlayer( &buffer, plr ))
                {
                    plr->PushUpdateBlock(&buffer, count);
                    buffer.clear();
                }
            }
            else if(!can_see && is_visible)
            {
                pObj->DestroyForPlayer(plr);
                plr->RemoveVisibleObject(it3);
            }

            if( pObj->IsPlayer() )
            {
                pl = castPtr<Player>( pObj );
                can_see = pl->CanSee( plr ), is_visible = pl->GetVisibility( plr, &it3 );
                if( can_see && !is_visible )
                {
                    pl->AddVisibleObject(plr);
                    if(count = plr->BuildCreateUpdateBlockForPlayer( &buffer, pl ))
                    {
                        pl->PushUpdateBlock(&buffer, count);
                        buffer.clear();
                    }
                }
                else if(!can_see && is_visible)
                {
                    plr->DestroyForPlayer(pl);
                    pl->RemoveVisibleObject(it3);
                }
            }
        }
    }
    else            // For units we can save a lot of work
    {
        for(WorldObject::InRangeSet::iterator it2 = GetInRangePlayerSetBegin(); it2 != GetInRangePlayerSetEnd(); it2++)
        {
            Player *plr = GetInRangeObject<Player>(*it2);
            can_see = plr->CanSee(this), is_visible = plr->GetVisibility(this, &itr);
            if(!can_see && is_visible)
            {
                DestroyForPlayer(plr);
                plr->RemoveVisibleObject(itr);
            }
            else if(can_see && !is_visible)
            {
                plr->AddVisibleObject(this);
                if(count = BuildCreateUpdateBlockForPlayer(&buffer, plr))
                {
                    plr->PushUpdateBlock(&buffer, count);
                    buffer.clear();
                }
            }
        }
    }
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
    if( pct <= 35 && !HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH35) )
        toSet |= AURASTATE_FLAG_HEALTH35;
    else if( pct > 35 && HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH35) )
        toRemove |= AURASTATE_FLAG_HEALTH35;

    if( pct <= 20 && !HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH20) )
        toSet |= AURASTATE_FLAG_HEALTH20;
    else if(pct > 20 && HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH20))
        toRemove |= AURASTATE_FLAG_HEALTH20;

    if( pct >= 75 && !HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTHABOVE75) )
        toSet |= AURASTATE_FLAG_HEALTHABOVE75;
    else if(pct < 75 && HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTHABOVE75))
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

void Unit::EventCastSpell(Unit* Target, SpellEntry * Sp)
{
    SpellCastTargets targets(Target->GetGUID());
    if(Spell* pSpell = new Spell(this, Sp))
        pSpell->prepare(&targets, true);
}

void Unit::SetFacing(float newo)
{
    SetOrientation(newo);
    m_aiInterface.SendMoveToPacket(m_position.x,m_position.y,m_position.z,m_position.o,1,MONSTER_MOVE_FLAG_WALK);
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

    if(Creature* p = GetMapInstance()->CreateCreature(guardian_entry))
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
        p->SetZoneId(GetZoneId());
        p->SetFactionTemplate(GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));

        p->GetAIInterface()->Init(p,AITYPE_PET,MOVEMENTTYPE_NONE,castPtr<Unit>(this));
        p->GetAIInterface()->SetUnitToFollow(castPtr<Unit>(this));
        p->GetAIInterface()->SetUnitToFollowAngle(angle);
        p->GetAIInterface()->SetFollowDistance(3.0f);
        p->PushToWorld(GetMapInstance());

        if(duration)
            sEventMgr.AddEvent(this, &Unit::SummonExpireSlot, Slot, EVENT_SUMMON_EXPIRE_0+Slot, duration, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );

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
                obj->ExpireAndDelete();
        m_ObjectSlots[x] = 0;
    }
}

void Unit::FillSummonList(std::vector<Creature*> &summonList, uint8 summonType)
{
    for(std::map< uint32, std::set<Creature*> >::iterator itr = m_Summons.begin(); itr != m_Summons.end(); itr++)
    {
        if(!itr->second.size())
            continue;
        for(std::set<Creature*>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++)
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
    for(std::map< uint32, std::set<Creature*> >::iterator itrMain = m_Summons.begin(); itrMain != m_Summons.end(); itrMain++)
    {
        if(itrMain->second.size())
        {
            for(std::set<Creature*>::iterator itr = itrMain->second.begin(); itr != itrMain->second.end(); itr++)
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
    if(Slot > 7)
        Slot = 0;

    //remove summons
    if(m_Summons[Slot].size())
    {
        Creature* mSum = NULL;
        for(std::set<Creature*>::iterator itr = m_Summons[Slot].begin(); itr != m_Summons[Slot].end(); itr++)
        {
            mSum = *itr;
            if(mSum->IsPet())
            {
                if(castPtr<Pet>(mSum)->GetUInt32Value(UNIT_CREATED_BY_SPELL) > 0)
                    castPtr<Pet>(mSum)->Dismiss(false);               // warlock summon -> dismiss
                else
                    castPtr<Pet>(mSum)->Remove(false, true, true);    // hunter pet -> just remove for later re-call
            }
            else
            {
                mSum->m_AuraInterface.RemoveAllAuras();
                if(mSum->IsInWorld())
                    mSum->Unit::RemoveFromWorld(true);
                mSum->DeleteMe();
            }
        }
        m_Summons[Slot].clear();
    }
    sEventMgr.RemoveEvents(this, EVENT_SUMMON_EXPIRE_0+Slot);
}

float Unit::CalculateDazeCastChance(Unit* target)
{
    float attack_skill = float( getLevel() ) * 5.0f;
    float defense_skill;
    if( target->IsPlayer() )
        defense_skill = float( castPtr<Player>( target )->_GetSkillLineCurrent( SKILL_DEFENSE, false ) );
    else defense_skill = float( target->getLevel() * 5 );
    if( !defense_skill )
        defense_skill = 1;
    float chance_to_daze = attack_skill * 20 / defense_skill;//if level is equal then we get a 20% chance to daze
    chance_to_daze = chance_to_daze * std::min(target->getLevel() / 30.0f, 1.0f );//for targets below level 30 the chance decreses
    if( chance_to_daze > 40 )
        return 40.0f;
    else
        return chance_to_daze;
}

uint32 Unit::Heal(Unit* target, uint32 SpellId, uint32 amount, bool silent)
{//Static heal
    if(!target || !SpellId || !amount || !target->isAlive() )
        return amount;

    uint32 overheal = 0;
    uint32 th = target->GetUInt32Value(UNIT_FIELD_HEALTH) + amount;
    uint32 mh = target->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(th > mh)
    {
        target->SetUInt32Value(UNIT_FIELD_HEALTH, mh);
        overheal = th - mh;
    } else target->SetUInt32Value(UNIT_FIELD_HEALTH, th);

    if(!silent)
        Spell::SendHealSpellOnPlayer(this, target, amount, false, overheal, SpellId);
    return overheal;
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
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
    m_combatStopTimer = timerOverride;

    // Trigger back combat status
    if(unit == NULL)
        return;
    // Only trigger combat calldown once
    unit->SetInCombat(NULL, timerOverride);
}

void Unit::EventCancelSpell(Spell* ptr)
{
    if(ptr != NULL)
        ptr->cancel();
    if(ptr == m_currentSpell)
        m_currentSpell = NULL;
}

void Unit::EventModelChange()
{
    //TODO: if has mount, grab mount model and add the z value of attachment 0
    if(CreatureBoundDataEntry *boundData = dbcCreatureBoundData.LookupEntry(GetUInt32Value(UNIT_FIELD_DISPLAYID)))
        m_modelhalfsize = boundData->High[2]/2.f;
    else m_modelhalfsize = 1.0f;
}

void Creature::UpdateLootAnimation(Player* Looter)
{

}

void Creature::ClearTag()
{
    if( isAlive() )
    {
        ClearLoot();
        m_taggingGroup = m_taggingPlayer = 0;
        m_lootMethod = -1;

        // if we are alive, means that we left combat
        if( IsInWorld() )
            UpdateLootAnimation(NULL);
    }
    // dead, don't clear tag
}

void Creature::Tag(Player* plr)
{
    // Tagging
    if( m_taggingPlayer != 0 )
        return;

    if(GetCreatureData() && GetCreatureData()->type == CRITTER || IsPet())
        return;

    m_taggingPlayer = plr->GetLowGUID();
    m_taggingGroup = plr->m_playerInfo->m_Group ? plr->m_playerInfo->m_Group->GetID() : 0;

    /* set loot method */
    if( plr->GetGroup() != NULL )
        m_lootMethod = plr->GetGroup()->GetMethod();

    // update tag visual
    UpdateLootAnimation(plr);
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
    RONIN_MAP<uint32, onAuraRemove*>::iterator itr;
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
    RONIN_MAP<uint32, onAuraRemove*>::iterator itr;
    if((itr = m_onAuraRemoveSpells.find(NameHash)) != m_onAuraRemoveSpells.end())
        itr->second->deleted = true;
}

// Aura by NameHash has been removed
void Unit::OnAuraRemove(uint32 NameHash, Unit* m_target)
{
    RONIN_MAP<uint32, onAuraRemove*>::iterator itr;
    if((itr = m_onAuraRemoveSpells.find(NameHash)) != m_onAuraRemoveSpells.end())
    {
        bool apply = true;
        if (itr->second->deleted == true)
            return;

        if (itr->second->chance != 100)
            apply = RandomUInt(100) < itr->second->chance;
        if (apply)
        {
            if (itr->second->self)
                CastSpell(castPtr<Unit>(this), itr->second->spell, true);
            else if (m_target)
                m_target->CastSpell(m_target, itr->second->spell, true);
        }
    }
}

//! Is PVP flagged?
bool Unit::IsPvPFlagged()
{
    return HasByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
}

void Unit::SetPvPFlag()
{
    // reset the timer as well..
    if(IsPlayer())
        castPtr<Player>(this)->StopPvPTimer();

    SetByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
}

//! Removal
void Unit::RemovePvPFlag()
{
    if(IsPlayer())
        castPtr<Player>(this)->StopPvPTimer();
    RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
}

void Unit::SetPowerType(uint8 type)
{
    SetByte(UNIT_FIELD_BYTES_0, 3, type);

    if(type == POWER_TYPE_RUNIC && IsPlayer())
    {
        SetFloatValue(PLAYER_RUNE_REGEN_1, 0.100000f);
        SetFloatValue(PLAYER_RUNE_REGEN_1+1, 0.100000f);
        SetFloatValue(PLAYER_RUNE_REGEN_1+2, 0.100000f);
        SetFloatValue(PLAYER_RUNE_REGEN_1+3, 0.100000f);
    }
}

int32 Unit::GetPowerPct(uint8 type)
{
    EUnitFields field = GetPowerFieldForType(type), maxField = EUnitFields(field+(UNIT_FIELD_MAXPOWERS-UNIT_FIELD_POWERS));
    if(field == UNIT_END || maxField == UNIT_END)
        return 0;

    uint32 power = GetPower(field), maxPower = GetMaxPower(maxField);
    if(maxPower <= 0) maxPower = 1;
    return int32(ceil(float(power)*100.f/float(maxPower)));
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
    uint32 _class = IsPet() ? HUNTER : getClass();
    return sStatSystem.GetPowerFieldForClassAndType(_class, type);
}

EUnitFields Unit::GetMaxPowerFieldForType(uint8 type)
{
    if(type == POWER_TYPE_HEALTH)
        return UNIT_FIELD_MAXHEALTH;
    uint32 _class = IsPet() ? HUNTER : getClass();
    EUnitFields field = sStatSystem.GetPowerFieldForClassAndType(_class, type);
    if(field == UNIT_END)
        return UNIT_END;
    return EUnitFields(field+(UNIT_FIELD_MAXPOWERS-UNIT_FIELD_POWERS));
}

void Unit::SetDeathState(DeathState s)
{
    m_deathState = s;
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
    if(IsPlayer())
    {
        Player* plr = castPtr<Player>(this);
        if( plr->m_MountSpellId )
        {
            m_AuraInterface.RemoveAllAuras( plr->m_MountSpellId);
            plr->m_MountSpellId = 0;
        }

        if( plr->m_FlyingAura )
        {
            m_AuraInterface.RemoveAllAuras( plr->m_FlyingAura);
            plr->m_FlyingAura = 0;
            plr->SetUInt32Value( UNIT_FIELD_DISPLAYID, plr->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
        }
    }

    m_movementInterface.OnDismount();

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI );
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER );
    EventModelChange();
}

void Unit::SetFaction(uint32 faction, bool save)
{
    SetFactionTemplate(faction);
    if(save && IsCreature() && castPtr<Creature>(this)->IsSpawn())
        castPtr<Creature>(this)->SaveToDB();
}

void Unit::ResetFaction()
{
    uint32 faction = 35;
    if(IsPlayer())
        faction = castPtr<Player>(this)->GetInfo()->factiontemplate;
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
        data << GetGUID();
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
    printf("TODO: ThreatRedirect\n");
}

void Unit::EventResetRedirectThreat()
{
    printf("TODO: ThreatRedirect\n");
}

uint32 Unit::GetCreatureType()
{
    if(IsCreature())
    {
        if(CreatureData * ci = castPtr<Creature>(this)->GetCreatureData())
            return ci->type;
    }
    else if(IsPlayer())
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
