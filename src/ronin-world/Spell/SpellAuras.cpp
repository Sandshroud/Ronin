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

std::map<uint16, Aura::pSpellAura> Aura::m_auraHandlerMap;

void Aura::InitializeAuraHandlerClass()
{
    m_auraHandlerMap[SPELL_AURA_BIND_SIGHT] = &Aura::SpellAuraBindSight; // 1
    m_auraHandlerMap[SPELL_AURA_MOD_POSSESS] = &Aura::SpellAuraModPossess; // 2
    m_auraHandlerMap[SPELL_AURA_DUMMY] = &Aura::SpellAuraDummy; // 4
    m_auraHandlerMap[SPELL_AURA_MOD_CONFUSE] = &Aura::SpellAuraModConfuse; // 5
    m_auraHandlerMap[SPELL_AURA_MOD_CHARM] = &Aura::SpellAuraModCharm; // 6
    m_auraHandlerMap[SPELL_AURA_MOD_FEAR] = &Aura::SpellAuraModFear; // 7
    m_auraHandlerMap[SPELL_AURA_PERIODIC_HEAL] = &Aura::SpellAuraPeriodicHeal; // 8
    m_auraHandlerMap[SPELL_AURA_MOD_ATTACKSPEED] = &Aura::SpellAuraModAttackSpeed; // 9
    m_auraHandlerMap[SPELL_AURA_MOD_THREAT] = &Aura::SpellAuraModThreatGenerated; // 10
    m_auraHandlerMap[SPELL_AURA_MOD_TAUNT] = &Aura::SpellAuraModTaunt; // 11
    m_auraHandlerMap[SPELL_AURA_MOD_STUN] = &Aura::SpellAuraModStun; // 12
    m_auraHandlerMap[SPELL_AURA_MOD_DAMAGE_DONE] = &Aura::SpellAuraModDamageDone; // 13
    m_auraHandlerMap[SPELL_AURA_MOD_DAMAGE_TAKEN] = &Aura::SpellAuraModDamageTaken; // 14
    m_auraHandlerMap[SPELL_AURA_DAMAGE_SHIELD] = &Aura::SpellAuraDamageShield; // 15
    m_auraHandlerMap[SPELL_AURA_MOD_STEALTH] = &Aura::SpellAuraModStealth; // 16
    m_auraHandlerMap[SPELL_AURA_MOD_STEALTH_DETECT] = &Aura::SpellAuraModDetect; // 17
    m_auraHandlerMap[SPELL_AURA_MOD_INVISIBILITY] = &Aura::SpellAuraModInvisibility; // 18
    m_auraHandlerMap[SPELL_AURA_MOD_INVISIBILITY_DETECT] = &Aura::SpellAuraModInvisibilityDetection; // 19
    m_auraHandlerMap[SPELL_AURA_MOD_TOTAL_HEALTH_REGEN_PCT] = &Aura::SpellAuraModTotalHealthRegenPct; // 20
    m_auraHandlerMap[SPELL_AURA_MOD_TOTAL_MANA_REGEN_PCT] = &Aura::SpellAuraModTotalManaRegenPct; // 21
    m_auraHandlerMap[SPELL_AURA_PERIODIC_TRIGGER_SPELL] = &Aura::SpellAuraPeriodicTriggerSpell; // 23
    m_auraHandlerMap[SPELL_AURA_PERIODIC_ENERGIZE] = &Aura::SpellAuraPeriodicEnergize; // 24
    m_auraHandlerMap[SPELL_AURA_MOD_PACIFY] = &Aura::SpellAuraModPacify; // 25
    m_auraHandlerMap[SPELL_AURA_MOD_ROOT] = &Aura::SpellAuraModRoot; // 26
    m_auraHandlerMap[SPELL_AURA_MOD_SILENCE] = &Aura::SpellAuraModSilence; // 27
    m_auraHandlerMap[SPELL_AURA_REFLECT_SPELLS] = &Aura::SpellAuraReflectSpells; // 28
    m_auraHandlerMap[SPELL_AURA_MOD_STAT] = &Aura::SpellAuraModStat; // 29
    m_auraHandlerMap[SPELL_AURA_MOD_SKILL] = &Aura::SpellAuraModSkill; // 30
    m_auraHandlerMap[SPELL_AURA_MOD_INCREASE_SPEED] = &Aura::SpellAuraModIncreaseSpeed; // 31
    m_auraHandlerMap[SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED] = &Aura::SpellAuraModDecreaseSpeed; // 32
    m_auraHandlerMap[SPELL_AURA_MOD_DECREASE_SPEED] = &Aura::SpellAuraModIncreaseSpeed; // 33
    m_auraHandlerMap[SPELL_AURA_MOD_INCREASE_HEALTH] = &Aura::SpellAuraModIncreaseHealth; // 34
    m_auraHandlerMap[SPELL_AURA_MOD_INCREASE_ENERGY] = &Aura::SpellAuraModIncreaseEnergy; // 35
    m_auraHandlerMap[SPELL_AURA_MOD_SHAPESHIFT] = &Aura::SpellAuraModShapeshift; // 36
    m_auraHandlerMap[SPELL_AURA_EFFECT_IMMUNITY] = &Aura::SpellAuraModEffectImmunity; // 37
    m_auraHandlerMap[SPELL_AURA_STATE_IMMUNITY] = &Aura::SpellAuraModStateImmunity; // 38
    m_auraHandlerMap[SPELL_AURA_SCHOOL_IMMUNITY] = &Aura::SpellAuraModSchoolImmunity; // 39
    m_auraHandlerMap[SPELL_AURA_DAMAGE_IMMUNITY] = &Aura::SpellAuraModDmgImmunity; // 40
    m_auraHandlerMap[SPELL_AURA_DISPEL_IMMUNITY] = &Aura::SpellAuraModDispelImmunity; // 41
    m_auraHandlerMap[SPELL_AURA_PROC_TRIGGER_SPELL] = &Aura::SpellAuraProcTriggerSpell; // 42
    m_auraHandlerMap[SPELL_AURA_PROC_TRIGGER_DAMAGE] = &Aura::SpellAuraProcTriggerDamage; // 43
    m_auraHandlerMap[SPELL_AURA_TRACK_CREATURES] = &Aura::SpellAuraTrackCreatures; // 44
    m_auraHandlerMap[SPELL_AURA_TRACK_RESOURCES] = &Aura::SpellAuraTrackResources; // 45
    m_auraHandlerMap[SPELL_AURA_MOD_PARRY_PERCENT] = &Aura::SpellAuraModParryPerc; // 47
    m_auraHandlerMap[SPELL_AURA_MOD_DODGE_PERCENT] = &Aura::SpellAuraModDodgePerc; // 49
    m_auraHandlerMap[SPELL_AURA_MOD_BLOCK_PERCENT] = &Aura::SpellAuraModBlockPerc; // 51
    m_auraHandlerMap[SPELL_AURA_MOD_WEAPON_CRIT_PERCENT] = &Aura::SpellAuraModCritPerc; // 52
    m_auraHandlerMap[SPELL_AURA_PERIODIC_LEECH] = &Aura::SpellAuraPeriodicLeech; // 53
    m_auraHandlerMap[SPELL_AURA_MOD_HIT_CHANCE] = &Aura::SpellAuraModHitChance; // 54
    m_auraHandlerMap[SPELL_AURA_MOD_SPELL_HIT_CHANCE] = &Aura::SpellAuraModSpellHitChance; // 55
    m_auraHandlerMap[SPELL_AURA_TRANSFORM] = &Aura::SpellAuraTransform; // 56
    m_auraHandlerMap[SPELL_AURA_MOD_SPELL_CRIT_CHANCE] = &Aura::SpellAuraModSpellCritChance; // 57
    m_auraHandlerMap[SPELL_AURA_MOD_INCREASE_SWIM_SPEED] = &Aura::SpellAuraIncreaseSwimSpeed; // 58
    m_auraHandlerMap[SPELL_AURA_MOD_DAMAGE_DONE_CREATURE] = &Aura::SpellAuraModCratureDmgDone; // 59
    m_auraHandlerMap[SPELL_AURA_MOD_PACIFY_SILENCE] = &Aura::SpellAuraPacifySilence; // 60
    m_auraHandlerMap[SPELL_AURA_MOD_SCALE] = &Aura::SpellAuraModScale; // 61
    m_auraHandlerMap[SPELL_AURA_PERIODIC_HEALTH_FUNNEL] = &Aura::SpellAuraPeriodicHealthFunnel; // 62
    m_auraHandlerMap[SPELL_AURA_PERIODIC_MANA_LEECH] = &Aura::SpellAuraPeriodicManaLeech; // 64
    m_auraHandlerMap[SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK] = &Aura::SpellAuraModCastingSpeed; // 65
    m_auraHandlerMap[SPELL_AURA_FEIGN_DEATH] = &Aura::SpellAuraFeignDeath; // 66
    m_auraHandlerMap[SPELL_AURA_MOD_DISARM] = &Aura::SpellAuraModDisarm; // 67
    m_auraHandlerMap[SPELL_AURA_MOD_STALKED] = &Aura::SpellAuraModStalked; // 68
    m_auraHandlerMap[SPELL_AURA_SCHOOL_ABSORB] = &Aura::SpellAuraSchoolAbsorb; // 69
    m_auraHandlerMap[SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL] = &Aura::SpellAuraModSpellCritChanceSchool; // 71
    m_auraHandlerMap[SPELL_AURA_MOD_POWER_COST] = &Aura::SpellAuraModPowerCost; // 72
    m_auraHandlerMap[SPELL_AURA_MOD_POWER_COST_SCHOOL] = &Aura::SpellAuraModPowerCostSchool; // 73
    m_auraHandlerMap[SPELL_AURA_REFLECT_SPELLS_SCHOOL] = &Aura::SpellAuraReflectSpellsSchool; // 74
    m_auraHandlerMap[SPELL_AURA_MOD_LANGUAGE] = &Aura::SpellAuraModLanguage; // 75
    m_auraHandlerMap[SPELL_AURA_FAR_SIGHT] = &Aura::SpellAuraAddFarSight; // 76
    m_auraHandlerMap[SPELL_AURA_MECHANIC_IMMUNITY] = &Aura::SpellAuraMechanicImmunity; // 77
    m_auraHandlerMap[SPELL_AURA_MOUNTED] = &Aura::SpellAuraMounted; // 78
    m_auraHandlerMap[SPELL_AURA_MOD_DAMAGE_PERCENT_DONE] = &Aura::SpellAuraModDamagePercDone; // 79
    m_auraHandlerMap[SPELL_AURA_MOD_PERCENT_STAT] = &Aura::SpellAuraModPercStat; // 80
    m_auraHandlerMap[SPELL_AURA_SPLIT_DAMAGE_PCT] = &Aura::SpellAuraSplitDamage; // 81
    m_auraHandlerMap[SPELL_AURA_WATER_BREATHING] = &Aura::SpellAuraWaterBreathing; // 82
    m_auraHandlerMap[SPELL_AURA_MOD_REGEN] = &Aura::SpellAuraModRegen; // 84
    m_auraHandlerMap[SPELL_AURA_MOD_POWER_REGEN] = &Aura::SpellAuraModPowerRegen; // 85
    m_auraHandlerMap[SPELL_AURA_CHANNEL_DEATH_ITEM] = &Aura::SpellAuraChannelDeathItem; // 86
    m_auraHandlerMap[SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN] = &Aura::SpellAuraModDamagePercTaken; // 87
    m_auraHandlerMap[SPELL_AURA_MOD_HEALTH_REGEN_PERCENT] = &Aura::SpellAuraModRegenPercent; // 88
    m_auraHandlerMap[SPELL_AURA_MOD_DETECT_RANGE] = &Aura::SpellAuraModDetectRange; // 91
    m_auraHandlerMap[SPELL_AURA_PREVENTS_FLEEING] = &Aura::SpellAuraPreventsFleeing; // 92
    m_auraHandlerMap[SPELL_AURA_MOD_UNATTACKABLE] = &Aura::SpellAuraModUnattackable; // 93
    m_auraHandlerMap[SPELL_AURA_INTERRUPT_REGEN] = &Aura::SpellAuraInterruptRegen; // 94
    m_auraHandlerMap[SPELL_AURA_GHOST] = &Aura::SpellAuraGhost; // 95
    m_auraHandlerMap[SPELL_AURA_SPELL_MAGNET] = &Aura::SpellAuraMagnet; // 96
    m_auraHandlerMap[SPELL_AURA_MANA_SHIELD] = &Aura::SpellAuraManaShield; // 97
    m_auraHandlerMap[SPELL_AURA_MOD_SKILL_TALENT] = &Aura::SpellAuraSkillTalent; // 98
    m_auraHandlerMap[SPELL_AURA_MOD_ATTACK_POWER] = &Aura::SpellAuraModAttackPower; // 99
    m_auraHandlerMap[SPELL_AURA_AURAS_VISIBLE] = &Aura::SpellAuraVisible; // 100
    m_auraHandlerMap[SPELL_AURA_MOD_CREATURE_ATTACK_POWER] = &Aura::SpellAuraModCreatureAttackPower; // 102
    m_auraHandlerMap[SPELL_AURA_MOD_TOTAL_THREAT] = &Aura::SpellAuraModTotalThreat; // 103
    m_auraHandlerMap[SPELL_AURA_WATER_WALK] = &Aura::SpellAuraWaterWalk; // 104
    m_auraHandlerMap[SPELL_AURA_FEATHER_FALL] = &Aura::SpellAuraFeatherFall; // 105
    m_auraHandlerMap[SPELL_AURA_HOVER] = &Aura::SpellAuraHover; // 106
    m_auraHandlerMap[SPELL_AURA_ADD_FLAT_MODIFIER] = &Aura::SpellAuraAddFlatModifier; // 107
    m_auraHandlerMap[SPELL_AURA_ADD_PCT_MODIFIER] = &Aura::SpellAuraAddPctMod; // 108
    m_auraHandlerMap[SPELL_AURA_ADD_TARGET_TRIGGER] = &Aura::SpellAuraAddTargetTrigger; // 109
    m_auraHandlerMap[SPELL_AURA_MOD_POWER_REGEN_PERCENT] = &Aura::SpellAuraModPowerRegPerc; // 110
    m_auraHandlerMap[SPELL_AURA_ADD_CASTER_HIT_TRIGGER] = &Aura::SpellAuraAddTargetTrigger; // 111
    m_auraHandlerMap[SPELL_AURA_OVERRIDE_CLASS_SCRIPTS] = &Aura::SpellAuraOverrideClassScripts; // 112
    m_auraHandlerMap[SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN] = &Aura::SpellAuraModRangedDamageTaken; // 113
    m_auraHandlerMap[SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT] = &Aura::SpellAuraModRangedDamageTaken; // 114
    m_auraHandlerMap[SPELL_AURA_MOD_HEALING] = &Aura::SpellAuraModHealing; // 115
    m_auraHandlerMap[SPELL_AURA_MOD_REGEN_DURING_COMBAT] = &Aura::SpellAuraIgnoreRegenInterrupt; // 116
    m_auraHandlerMap[SPELL_AURA_MOD_MECHANIC_RESISTANCE] = &Aura::SpellAuraModMechanicResistance; // 117
    m_auraHandlerMap[SPELL_AURA_MOD_HEALING_PCT] = &Aura::SpellAuraModHealingPCT; // 118
    m_auraHandlerMap[SPELL_AURA_UNTRACKABLE] = &Aura::SpellAuraUntrackable; // 120
    m_auraHandlerMap[SPELL_AURA_EMPATHY] = &Aura::SpellAuraEmphaty; // 121
    m_auraHandlerMap[SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT] = &Aura::SpellAuraModOffhandDamagePCT; // 122
    m_auraHandlerMap[SPELL_AURA_MOD_TARGET_RESISTANCE] = &Aura::SpellAuraModPenetration; // 123
    m_auraHandlerMap[SPELL_AURA_MOD_RANGED_ATTACK_POWER] = &Aura::SpellAuraModRangedAttackPower; // 124
    m_auraHandlerMap[SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN] = &Aura::SpellAuraModMeleeDamageTaken; // 125
    m_auraHandlerMap[SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT] = &Aura::SpellAuraModMeleeDamageTakenPct; // 126
    m_auraHandlerMap[SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS] = &Aura::SpellAuraRAPAttackerBonus; // 127
    m_auraHandlerMap[SPELL_AURA_MOD_POSSESS_PET] = &Aura::SpellAuraModPossessPet; // 128
    m_auraHandlerMap[SPELL_AURA_MOD_SPEED_ALWAYS] = &Aura::SpellAuraModIncreaseSpeedAlways; // 129
    m_auraHandlerMap[SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS] = &Aura::SpellAuraModIncreaseMountedSpeed; // 130
    m_auraHandlerMap[SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER] = &Aura::SpellAuraModCreatureRangedAttackPower; // 131
    m_auraHandlerMap[SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT] = &Aura::SpellAuraModIncreaseEnergyPerc; // 132
    m_auraHandlerMap[SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT] = &Aura::SpellAuraModIncreaseHealthPerc; // 133
    m_auraHandlerMap[SPELL_AURA_MOD_MANA_REGEN_INTERRUPT] = &Aura::SpellAuraModManaRegInterrupt; // 134
    m_auraHandlerMap[SPELL_AURA_MOD_HEALING_DONE] = &Aura::SpellAuraModHealingDone; // 135
    m_auraHandlerMap[SPELL_AURA_MOD_HEALING_DONE_PERCENT] = &Aura::SpellAuraModHealingDonePct; // 136
    m_auraHandlerMap[SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE] = &Aura::SpellAuraModTotalStatPerc; // 137
    m_auraHandlerMap[SPELL_AURA_MOD_MELEE_HASTE] = &Aura::SpellAuraModHaste; // 138
    m_auraHandlerMap[SPELL_AURA_FORCE_REACTION] = &Aura::SpellAuraForceReaction; // 139
    m_auraHandlerMap[SPELL_AURA_MOD_RANGED_HASTE] = &Aura::SpellAuraModRangedHaste; // 140
    m_auraHandlerMap[SPELL_AURA_MOD_RANGED_AMMO_HASTE] = &Aura::SpellAuraModRangedAmmoHaste; // 141
    m_auraHandlerMap[SPELL_AURA_SAFE_FALL] = &Aura::SpellAuraSafeFall; // 144
    m_auraHandlerMap[SPELL_AURA_MOD_PET_TALENT_POINTS] = &Aura::SpellAuraModPetTalentPoints; // 145
    m_auraHandlerMap[SPELL_AURA_ALLOW_TAME_PET_TYPE] = &Aura::SpellAuraAllowTamePetType; // 146
    m_auraHandlerMap[SPELL_AURA_ADD_CREATURE_IMMUNITY] = &Aura::SpellAuraAddCreatureImmunity; // 147
    m_auraHandlerMap[SPELL_AURA_RETAIN_COMBO_POINTS] = &Aura::SpellAuraRetainComboPoints; // 148
    m_auraHandlerMap[SPELL_AURA_REDUCE_PUSHBACK] = &Aura::SpellAuraResistPushback; // 149
    m_auraHandlerMap[SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT] = &Aura::SpellAuraModShieldBlockPCT; // 150
    m_auraHandlerMap[SPELL_AURA_TRACK_STEALTHED] = &Aura::SpellAuraTrackStealthed; // 151
    m_auraHandlerMap[SPELL_AURA_MOD_DETECTED_RANGE] = &Aura::SpellAuraModDetectedRange; // 152
    m_auraHandlerMap[SPELL_AURA_SPLIT_DAMAGE_FLAT] = &Aura::SpellAuraSplitDamageFlat; // 153
    m_auraHandlerMap[SPELL_AURA_MOD_STEALTH_LEVEL] = &Aura::SpellAuraModStealthLevel; // 154
    m_auraHandlerMap[SPELL_AURA_MOD_WATER_BREATHING] = &Aura::SpellAuraModUnderwaterBreathing; // 155
    m_auraHandlerMap[SPELL_AURA_MOD_REPUTATION_GAIN] = &Aura::SpellAuraModReputationAdjust; // 156
    m_auraHandlerMap[SPELL_AURA_MOD_SHIELD_BLOCKVALUE] = &Aura::SpellAuraModBlockValue; // 158
    m_auraHandlerMap[SPELL_AURA_NO_PVP_CREDIT] = &Aura::SpellAuraNoPVPCredit; // 159
    m_auraHandlerMap[SPELL_AURA_MOD_AOE_AVOIDANCE] = &Aura::SpellAuraReduceAOEDamageTaken; // 160
    m_auraHandlerMap[SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT] = &Aura::SpellAuraModHealthRegInCombat; // 161
    m_auraHandlerMap[SPELL_AURA_POWER_BURN_MANA] = &Aura::SpellAuraPowerBurn; // 162
    m_auraHandlerMap[SPELL_AURA_SPIRIT_OF_REDEMPTION] = &Aura::SpellAuraSpiritOfRedemption; // 176
    m_auraHandlerMap[SPELL_AURA_MOD_DEBUFF_RESISTANCE] = &Aura::SpellAuraDispelDebuffResist; // 178
}

Unit* Aura::GetUnitCaster()
{
    if( m_target == NULL && m_casterGuid && m_casterGuid.getHigh() == HIGHGUID_TYPE_PLAYER)
        if(Player* punit = objmgr.GetPlayer(m_casterGuid))
            return punit;
    if( m_target == NULL )
        return NULL;

    if( m_casterGuid && m_casterGuid == m_target->GetGUID() )
        return m_target;
    if( m_target->GetMapInstance() != NULL )
        return m_target->GetMapInstance()->GetUnit( m_casterGuid );
    return NULL;
}

Aura::Aura(Unit *target, SpellEntry *proto, SpellEntry *spellParent, uint16 auraFlags, uint8 auraLevel, int16 auraStackCharge, time_t expirationTime, WoWGuid casterGuid)
    : m_target(target), m_spellProto(proto), m_spellParent(spellParent), m_auraFlags(auraFlags), m_auraLevel(auraLevel), m_duration(-1), m_expirationTime(0), m_casterGuid(casterGuid),
    m_auraSlot(0xFF), m_applied(false), m_deleted(false), m_dispelled(false), m_castInDuel(false), m_creatureAA(false), m_areaAura(false), m_interrupted(-1),
    m_positive(!proto->isNegativeSpell1())
{
    m_stackSizeorProcCharges = auraStackCharge;

    CalculateLifetime(expirationTime);

    mod = NULL;
    m_modcount = 0;
    memset(m_modList, 0, sizeof(Modifier)*3);

    m_castedItemId = 0;
    m_triggeredSpellId = 0;

    _periodicData = NULL;
}

Aura::~Aura()
{

}

void Aura::Update(uint32 diff)
{
    if(m_expirationTime == 0)
        return;

    if(m_expirationTime <= UNIXTIME)
        Remove();

    if(m_deleted || _periodicData == NULL)
        return;

    bool forceInitialTick = false;
    if(m_spellProto->isPeriodicAtApplication() && _periodicData->m_lifeTimer == 0)
        forceInitialTick = true;

    _periodicData->m_lifeTimer += diff;
    for(uint32 i = 0; i < 3; i++)
    {
        PeriodicAura::PeriodicModifier *pMod = NULL;
        if((pMod = _periodicData->periodicMod[i]) == NULL)
            continue;

        // Update our tick timer
        if(forceInitialTick == true)
            pMod->timer += diff;
        else
        {
            if((pMod->timer += diff) < pMod->rate)
                continue;
            pMod->timer -= pMod->rate;
            ++pMod->tickCounter;
        }

        TriggerPeriodic(i);
    }

    bool empty = true;
    for(uint32 i = 0; i < 3; i++)
        if(_periodicData->periodicMod[i])
            empty = false;

    if(empty == false)
        return;
    
    delete _periodicData;
    _periodicData = NULL;
}

void Aura::UpdatePreApplication()
{
    CalculateLifetime();
    if (m_spellProto->NameHash == SPELL_HASH_DRINK)
        EventPeriodicDrinkDummy(1, m_modList[1].m_amount);
}

void Aura::TriggerPeriodic(uint32 i)
{
    switch(m_modList[i].m_type)
    {
    case SPELL_AURA_PERIODIC_DAMAGE:
        EventPeriodicDamage(m_modList[i].m_amount);
        break;
    case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        EventPeriodicDamagePercent(m_modList[i].m_amount);
        break;
    case SPELL_AURA_PERIODIC_HEAL:
        EventPeriodicHeal(m_modList[i].m_amount);
        break;
    case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        if(SpellEntry *sp = dbcSpell.LookupEntry(m_spellProto->EffectTriggerSpell[i]))
            EventPeriodicTriggerSpell(sp, false, 0);
        break;
    case SPELL_AURA_PERIODIC_ENERGIZE:
        EventPeriodicEnergize(m_modList[i].m_amount, m_modList[i].m_miscValue[0]);
        break;
    case SPELL_AURA_PERIODIC_LEECH:
        EventPeriodicLeech(m_modList[i].m_amount, m_spellProto);
        break;
    case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        EventPeriodicHealthFunnel(m_modList[i].m_amount);
        break;
    case SPELL_AURA_PERIODIC_MANA_LEECH:
        EventPeriodicManaLeech(m_modList[i].m_amount);
        break;
    case SPELL_AURA_PERIODIC_DUMMY:
        if(m_spellProto->NameHash == SPELL_HASH_DRINK)
            EventPeriodicDrinkDummy(i, m_modList[i].m_amount);
        else if(m_spellProto->NameHash == SPELL_HASH_DEATH_AND_DECAY)
            EventPeriodicDamage(m_modList[0].m_amount);
        break;
    case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
        if(SpellEntry *sp = dbcSpell.LookupEntry(m_spellProto->EffectTriggerSpell[i]))
            EventPeriodicTriggerSpell(sp, false, 0);
        break;
    case SPELL_AURA_ABILITY_PERIODIC_CRIT:
        break;
    case SPELL_AURA_PERIODIC_HASTE:
        break;
    }
}

void Aura::CalculateLifetime(time_t expectedExpire)
{
    if(IsPassive())
        return;

    m_creationTime = UNIXTIME;
    int32 forcedDuration = expectedExpire ? 1000*(expectedExpire - UNIXTIME) : 0;

    int32 Duration = m_spellProto->CalculateSpellDuration(m_auraLevel, 0);
    if(!m_positive && !m_spellProto->isPassiveSpell())
        ::ApplyDiminishingReturnTimer(&Duration, m_target, GetSpellProto());
    uint32 mechanic = GetMechanic();
    if( m_target->IsPlayer() && mechanic < NUM_MECHANIC && Duration > 0 )
        Duration *= m_target->GetMechanicDurationPctMod(mechanic);
    if((m_duration = Duration) > 0)
    {
        m_auraFlags |= AFLAG_HAS_DURATION;
        m_expirationTime = UNIXTIME+(m_duration/1000);
    } else m_expirationTime = 0;
}

void Aura::Refresh(bool freshStack)
{
    // Recalc duration instead of just resetting
    CalculateLifetime();
    // If we're a fresh stack then we have a stacksize
    if(freshStack)
        AddStackSize(1);
    else if(m_spellProto->procCharges)
        SetProcCharges(GetMaxProcCharges(GetUnitCaster()));
    BuildAuraUpdate();
}

void Aura::Remove()
{
    if( m_deleted )
        return;
    m_deleted = true;

    if( !IsPassive() )
        BuildAuraUpdate();

    m_target->m_AuraInterface.OnAuraRemove(this, m_auraSlot);

    ApplyModifiers( false );

    // reset diminishing return timer if needed
    ::UnapplyDiminishingReturnTimer( m_target, m_spellProto );

    Unit * m_caster = GetUnitCaster();
    if (m_caster != NULL)
    {
        m_caster->OnAuraRemove(m_spellProto, m_target);
        if(m_spellProto->IsSpellChannelSpell() && m_caster->isCasting() && m_caster->GetSpellInterface()->GetCurrentSpellProto()->Id == m_spellProto->Id)
            m_caster->GetSpellInterface()->CleanupCurrentSpell();
    }

    for( uint32 x = 0; x < 3; x++ )
    {
        if( !m_spellProto->Effect[x] )
            continue;

        if( m_spellProto->Effect[x] == SPELL_EFFECT_TRIGGER_SPELL && !m_spellProto->always_apply )
            m_target->RemoveAura(m_spellProto->EffectTriggerSpell[x]);
    }

    if( m_spellProto->MechanicsType == MECHANIC_ENRAGED )
        m_target->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_ENRAGE );
    else if( m_spellProto->Id == 642 )
    {
        m_target->RemoveAura( 53523 );
        m_target->RemoveAura( 53524 );
    }

    m_target = NULL;
    m_casterGuid = 0;
    m_modcount = 0;

    delete this;
}

void Aura::OnTargetChangeLevel(uint32 newLevel, uint64 targetGuid)
{
    // Get our unit target so we can test against the given guid
    if(m_target->GetGUID() == targetGuid)
        RecalculateModBaseAmounts();
}

void Aura::AddMod(uint32 i, uint32 t, int32 a, uint32 b, int32 f, float ff )
{
    if( m_modcount >= 3 || m_target == NULL || m_target->GetMechanicDispels(GetMechanicOfEffect(i)))
        return;

    uint8 modIndex = m_modcount++;

    if(i == 0) m_auraFlags |= AFLAG_EFF_INDEX_0;
    else if(i == 1) m_auraFlags |= AFLAG_EFF_INDEX_1;
    else if(i == 2) m_auraFlags |= AFLAG_EFF_INDEX_2;
    int32 amount = a;
    if(m_stackSizeorProcCharges >= 0)
        amount *= m_stackSizeorProcCharges;
    amount += b;

    m_modList[modIndex].i = i;
    m_modList[modIndex].m_type = t;
    m_modList[modIndex].m_amount = amount;
    m_modList[modIndex].m_baseAmount = a;
    m_modList[modIndex].m_miscValue[0] = m_spellProto->EffectMiscValue[i];
    m_modList[modIndex].m_miscValue[1] = m_spellProto->EffectMiscValueB[i];
    m_modList[modIndex].m_bonusAmount = b;
    m_modList[modIndex].fixed_amount = f;
    m_modList[modIndex].fixed_float_amount = ff;
    m_modList[modIndex].m_spellInfo = GetSpellProto();
    CalculateBonusAmount(GetUnitCaster(), modIndex);

    // Periodic effect adding
    switch(t)
    {
    case SPELL_AURA_PERIODIC_DAMAGE:
    case SPELL_AURA_PERIODIC_HEAL:
    case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
    case SPELL_AURA_PERIODIC_ENERGIZE:
    case SPELL_AURA_PERIODIC_LEECH:
    case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
    case SPELL_AURA_PERIODIC_MANA_LEECH:
    case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
    case SPELL_AURA_PERIODIC_DUMMY:
    case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
    case SPELL_AURA_ABILITY_PERIODIC_CRIT:
    case SPELL_AURA_PERIODIC_HASTE:
        {
            if(_periodicData == NULL)
            {   // Construct our new periodic data holder
                _periodicData = new PeriodicAura();
                for(uint32 x = 0; x < 3; x++)
                    _periodicData->periodicMod[x] = NULL;
                _periodicData->m_lifeTimer = 0;
            }

            _periodicData->periodicMod[modIndex] = new PeriodicAura::PeriodicModifier();
            _periodicData->periodicMod[modIndex]->rate = m_spellProto->EffectAmplitude[i] > 0 ? m_spellProto->EffectAmplitude[i] : 3000;
            _periodicData->periodicMod[modIndex]->timer = 0;
            _periodicData->periodicMod[modIndex]->tickCounter = 0;
        }break;
    }
}

void Aura::ResetExpirationTime()
{
    if(m_expirationTime == 0 || m_duration == -1)
        return;
    m_expirationTime = UNIXTIME+(m_duration/1000);
}

void Aura::RemoveIfNecessary()
{
    if( !m_applied )
        return; // already removed

    if( m_spellProto->CasterAuraState && m_target && !(m_target->GetUInt32Value(UNIT_FIELD_AURASTATE) & (uint32(1) << (m_spellProto->CasterAuraState-1)) ) )
    {
        ApplyModifiers(false);
        return;
    }
    if( m_spellProto->CasterAuraStateNot && m_target && m_target->GetUInt32Value(UNIT_FIELD_AURASTATE) & (uint32(1) << (m_spellProto->CasterAuraStateNot-1)) )
    {
        ApplyModifiers(false);
        return;
    }
}

void Aura::ApplyModifiers( bool apply )
{
    if(!m_applied && !apply)    // Don't want to unapply modifiers if they haven't been applied
        return;
    if(m_deleted && apply == true)
        return;

    m_applied = apply;
    if( apply && m_spellProto->CasterAuraState && m_target && !(m_target->GetUInt32Value(UNIT_FIELD_AURASTATE) & (uint32(1) << (m_spellProto->CasterAuraState - 1) ) ) )
    {
        m_applied = false;
        return;
    }
    if( apply && m_spellProto->CasterAuraStateNot && m_target && m_target->GetUInt32Value(UNIT_FIELD_AURASTATE) & (uint32(1) << (m_spellProto->CasterAuraStateNot - 1) ) )
    {
        m_applied = false;
        return;
    }

    if(m_spellProto->buffIndex > BUFF_PALADIN_SEAL_START && m_spellProto->buffIndex < BUFF_PALADIN_HAND_START)
    {
        if(apply && !m_target->HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_JUDGEMENT))
            m_target->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_JUDGEMENT);
        else if(apply == false)
            m_target->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_JUDGEMENT);
    }

    for( uint32 x = 0; x < m_modcount; x++ )
    {
        mod = &m_modList[x];
        if(mod->m_type >= SPELL_AURA_TOTAL)
        {
            sLog.Debug( "Aura","Unknown Aura id %d in spell %u", uint32(mod->m_type), GetSpellId());
            continue;
        }

        m_target->m_AuraInterface.UpdateModifier(GetAuraSlot(), x, mod, apply);
        sLog.Debug( "Aura","Known Aura id %d, value %d in spell %u", uint32(mod->m_type), uint32(mod->m_amount), GetSpellId());

        std::map<uint16, pSpellAura>::iterator itr;
        if((itr = m_auraHandlerMap.find(mod->m_type)) != m_auraHandlerMap.end() && itr->second != NULL)
            (*this.*(itr->second))(apply);
    }
}

void Aura::UpdateModifiers( )
{
    for( uint32 x = 0; x < m_modcount; x++ )
    {
        mod = &m_modList[x];
        if(mod->m_type >= SPELL_AURA_TOTAL)
        {
            sLog.Debug( "Aura","Unknown Aura id %d", (uint32)mod->m_type);
            continue;
        }

        m_target->m_AuraInterface.UpdateModifier(GetAuraSlot(), x, mod, m_applied);
        sLog.Debug( "Aura","Updating Aura modifiers target = %u, slot = %u, Spell Aura id = %u, SpellId  = %u, i = %u, duration = %i, damage = %d",
            m_target->GetLowGUID(), m_auraSlot, mod->m_type, m_spellProto->Id, mod->i, GetDuration(),mod->m_amount);
    }
}

bool Aura::AddAuraVisual()
{
    uint8 slot = m_target->m_AuraInterface.GetFreeSlot(IsPositive());
    if (slot == 0xFF)
        return false;

    m_auraSlot = slot;
    BuildAuraUpdate();
    return true;
}

void Aura::BuildAuraUpdate()
{
    if( m_target == NULL || IsPassive() )
        return;

    WorldPacket data(SMSG_AURA_UPDATE, 50);
    FastGUIDPack(data, m_target->GetGUID());
    BuildAuraUpdatePacket(&data);
    m_target->SendMessageToSet(&data, true);
}

void Aura::BuildAuraUpdatePacket(WorldPacket *data)
{
    *data << uint8(m_auraSlot);
    uint16 stackCount = abs(m_stackSizeorProcCharges);
    if(m_deleted || stackCount == 0)
    {
        *data << uint32(0);
        return;
    }

    uint16 flags = GetAuraFlags();
    *data << uint32(GetSpellProto()->Id) << uint16(flags);
    *data << uint8(GetAuraLevel());
    *data << uint8(stackCount > 0xFF ? 0xFF : (stackCount & 0xFF));

    if(!(flags & AFLAG_NOT_GUID))
        *data << GetCasterGUID().asPacked();
    if( flags & AFLAG_HAS_DURATION )
    {
        *data << GetDuration();
        *data << GetMSTimeLeft();
    }

    if (flags & AFLAG_EFF_AMOUNT_SEND)
    {
        for (uint8 i = 0; i < 3; ++i)
        {
            if (flags & 1<<i)
            {
                Modifier *mod = GetMod(i);
                *data << uint32(mod ? mod->m_amount : 0);
            }
        }
    }
}

void Aura::EventPeriodicDamage(uint32 amount)
{
    //DOT
    if(!m_target->isAlive())
        return;

    if(Unit *m_caster = GetUnitCaster())
        m_caster->SpellNonMeleeDamageLog(m_target, m_spellProto->Id, amount, 0.f);
    else m_target->SpellNonMeleeDamageLog(m_target, m_spellProto->Id, amount, 0.f);
}

void Aura::EventPeriodicDamagePercent(uint32 amount)
{
    //DOT
    if(!m_target->isAlive())
        return;

    uint32 damage = m_target->GetModPUInt32Value(UNIT_FIELD_MAXHEALTH, amount);
    Unit * m_caster = GetUnitCaster();
    if(m_caster!=NULL)
        m_caster->SpellNonMeleeDamageLog(m_target, m_spellProto->Id, damage, 0.f);
    else m_target->SpellNonMeleeDamageLog(m_target, m_spellProto->Id, damage, 0.f);
}

void Aura::EventPeriodicHeal( uint32 amount )
{
    int32 add = amount; // IMPORTANT: target heals himself, but the packet says the caster does it. This is important, to allow for casters to log out and players still get healed.
    uint32 overheal = m_target->Heal(m_target, add, GetSpellId(), true);
    SendPeriodicAuraLog( m_casterGuid, m_target, GetSpellProto(), add, 0, overheal, FLAG_PERIODIC_HEAL );

    if( m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP )
        m_target->Emote( EMOTE_ONESHOT_EAT );
}

void Aura::EventPeriodicHealPct(float RegenPct)
{
    Unit* m_caster = GetUnitCaster();
    if(m_caster == NULL || !m_target->isAlive())
        return;

    uint32 add = float2int32(m_target->GetUInt32Value(UNIT_FIELD_MAXHEALTH) * (RegenPct / 100.0f));
    uint32 overheal = m_caster->Heal(m_target, add, GetSpellId(), true);
    SendPeriodicAuraLog( m_casterGuid, m_target, GetSpellProto(), add, 0, overheal, FLAG_PERIODIC_HEAL );

    if(m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP)
        m_target->Emote(EMOTE_ONESHOT_EAT);
}

void Aura::EventPeriodicTriggerSpell(SpellEntry* spellInfo, bool overridevalues, int32 overridevalue)
{
    m_target->GetSpellInterface()->TriggerSpell(spellInfo, m_target);
}

void Aura::EventPeriodicEnergize(uint32 amount,uint32 type)
{
    m_target->Energize(m_target, m_spellProto->Id, amount, type );
    if((m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP) && type == 0)
        m_target->Emote(EMOTE_ONESHOT_EAT);

    m_target->SendPowerUpdate();
}

void Aura::EventPeriodicHeal1(uint32 amount)
{
    if(m_target == NULL )
        return;

    if(!m_target->isAlive())
        return;

    uint32 ch = m_target->GetUInt32Value(UNIT_FIELD_HEALTH);
    ch+=amount;
    uint32 mh = m_target->GetUInt32Value(UNIT_FIELD_MAXHEALTH);

    if(ch>mh)
        m_target->SetUInt32Value(UNIT_FIELD_HEALTH,mh);
    else m_target->SetUInt32Value(UNIT_FIELD_HEALTH,ch);

    if(m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP)
        m_target->Emote(EMOTE_ONESHOT_EAT);
    else if(m_spellProto->buffIndex == 0)
        SendPeriodicAuraLog(amount, FLAG_PERIODIC_HEAL);
}

void Aura::EventPeriodicLeech(uint32 amount, SpellEntry* sp)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL || m_target == NULL || !m_target->isAlive() || !m_caster->isAlive() )
        return;

    if( sp->NameHash == SPELL_HASH_DRAIN_LIFE && m_caster->HasDummyAura(SPELL_HASH_DEATH_S_EMBRACE) && m_caster->GetHealthPct() <= 20 )
        amount *= 1.3f;

    amount = m_caster->GetSpellBonusDamage(m_target, sp, mod->i, amount, false);

    uint32 Amount = std::min(amount, m_target->GetUInt32Value( UNIT_FIELD_HEALTH ));

    SendPeriodicAuraLog(m_casterGuid, m_target, sp, Amount, -1, 0, (uint32)FLAG_PERIODIC_DAMAGE);

    //deal damage before we add healing bonus to damage
    m_caster->DealDamage(m_target, Amount, 0, 0, sp->Id, true);
    if(sp)
    {
        float coef = sp->EffectValueMultiplier[mod->i]; // how much health is restored per damage dealt
        m_caster->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, sp->SpellGroupType);
        m_caster->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, sp->SpellGroupType);
        Amount = float2int32((float)Amount * coef);
    }

    uint32 newHealth = float2int32(m_caster->GetUInt32Value(UNIT_FIELD_HEALTH) + Amount);

    uint32 mh = m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(newHealth <= mh)
        m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, newHealth);
    else
        m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, mh);

    SendPeriodicAuraLog(m_casterGuid, m_caster, sp, Amount, -1, 0, (uint32)FLAG_PERIODIC_HEAL);
}

void Aura::EventPeriodicBurn(uint32 amount, uint32 misc)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL)
        return;

    if(m_target->isAlive() && m_caster->isAlive())
    {
        uint32 Amount = std::min( amount, m_target->GetPower(misc) );
        SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, Amount, 0, 0, FLAG_PERIODIC_DAMAGE);
        m_target->DealDamage(m_target, Amount, 0, 0, m_spellProto->Id);
    }
}

void Aura::EventPeriodicHealthFunnel(uint32 amount)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL  || m_target == NULL || !m_target->isAlive() || !m_caster->isAlive())
        return;
    if(m_target->isAlive() && m_caster->isAlive())
    {

        m_caster->DealDamage(m_target, amount, 0, 0, GetSpellId(),true);
        uint32 newHealth = m_caster->GetUInt32Value(UNIT_FIELD_HEALTH) + 1000;

        uint32 mh = m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
        if(newHealth <= mh)
            m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, newHealth);
        else
            m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, mh);

        SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, amount, -1, 0, (uint32)FLAG_PERIODIC_LEECH);
    }
}

void Aura::EventPeriodicManaLeech(uint32 amount)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL  || m_target == NULL || !m_target->isAlive() || !m_caster->isAlive())
        return;

    int32 amt = amount;

    // Drained amount should be reduced by resilence
    if(m_target->IsPlayer())
    {
        float amt_reduction_pct = 2.2f * castPtr<Player>(m_target)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE ) / 100.0f;
        if( amt_reduction_pct > 0.33f ) amt_reduction_pct = 0.33f; // 3.0.3
        amt = float2int32( amt - (amt * amt_reduction_pct) );
    }

    float coef = m_spellProto->EffectValueMultiplier[mod->i] > 0 ? m_spellProto->EffectValueMultiplier[mod->i] : 1; // how much mana is restored per mana leeched
    m_caster->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, m_spellProto->SpellGroupType);
    m_caster->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, m_spellProto->SpellGroupType);
    amt = float2int32((float)amt * coef);

    uint32 cm = m_caster->GetPower(POWER_TYPE_MANA) + amt;
    uint32 mm = m_caster->GetMaxPower(POWER_TYPE_MANA);
    if(cm <= mm)
    {
        m_caster->SetPower(POWER_TYPE_MANA, cm);
        SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, amt, 0, 0, FLAG_PERIODIC_LEECH);
    }
    else
    {
        m_caster->SetPower(POWER_TYPE_MANA, mm);
        SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, mm - cm, 0, 0, FLAG_PERIODIC_LEECH);
    }

    m_caster->SendPowerUpdate();
}

void Aura::EventPeriodicManaPct(float RegenPct)
{
    if(!m_target->isAlive())
        return;
    uint32 manaMax = m_target->GetMaxPower(POWER_TYPE_MANA);
    if(manaMax == 0)
        return;

    uint32 add = float2int32(manaMax * (RegenPct / 100.0f));
    uint32 newMana = m_target->GetPower(POWER_TYPE_MANA) + add;

    m_target->SetPower(POWER_TYPE_MANA, newMana <= manaMax ? newMana : manaMax);
    SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, add, 0, 0, FLAG_PERIODIC_ENERGIZE);

    if(m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP)
        m_target->Emote(EMOTE_ONESHOT_EAT);
    m_target->SendPowerUpdate();
}

void Aura::EventPeriodicRegenManaStatPct(uint32 perc, uint32 stat)
{
    if(m_target->isDead())
        return;

    uint32 spellId = m_triggeredSpellId ? m_triggeredSpellId : (m_spellProto ? m_spellProto->Id : 0);

    m_target->Energize(m_target, spellId, (m_target->GetUInt32Value(UNIT_FIELD_STATS + stat) * perc)/100, POWER_TYPE_MANA);
}

void Aura::EventPeriodicSpeedModify(int32 modifier)
{

}

void Aura::EventPeriodicDrinkDummy(int32 index, uint32 amount)
{
    int32 newAmt = amount;
    if(m_target->isAlive() && m_target->IsPlayer() && m_target->IsInWorld() && m_target->GetMapInstance()->IsBattleArena())
    {
        switch(_periodicData->periodicMod[index]->tickCounter)
        {
            // Start and first tick skip
        case 0: case 1:
            return;
        case 2:
            newAmt = (amount*5)/3;
            break;
        case 3:
            newAmt = (amount*4)/3;
            break;
        }
    }

    // Amount is divided by 5 for regen, dunno why it's not interval based
    m_modList[0].m_amount = ceil(((float)newAmt)/5.f);

    // Tell our unit that the mod has updated
    m_target->OnAuraModChanged(m_modList[0].m_type);

    // check if we're setting to basic
    if(newAmt != amount)
        return;

    // Clean up our periodic tick since we don't need it
    delete _periodicData->periodicMod[index];
    _periodicData->periodicMod[index] = NULL;
}

void Aura::EventRelocateRandomTarget()
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL || !m_caster->IsPlayer() || m_caster->isDead() )
        return;

    // Ok, let's do it. :D
    std::set<Unit* > enemies;

    // Can't do anything w/o a target
    if( !enemies.size() )
        return;

    uint32 random = RandomUInt(uint32(enemies.size()) - 1);
    std::set<Unit* >::iterator it2 = enemies.begin();
    while( random-- )
        it2++;

    Unit* pTarget = (*it2);
    if(pTarget == NULL)
        return; // In case I did something horribly wrong.

    float ang = pTarget->GetOrientation();

    // avoid teleporting into the model on scaled models
    const static float killingspree_distance = 1.6f * GetDBCScale( dbcCreatureDisplayInfo.LookupEntry( pTarget->GetUInt32Value(UNIT_FIELD_DISPLAYID)));
    float new_x = pTarget->GetPositionX() - (killingspree_distance * cosf(ang));
    float new_y = pTarget->GetPositionY() - (killingspree_distance * sinf(ang));
    float new_z = pTarget->GetMapInstance()->GetWalkableHeight(pTarget, new_x, new_y, pTarget->GetPositionZ(), pTarget->GetPositionZ());
    castPtr<Player>(m_caster)->SafeTeleport( pTarget->GetMapId(), pTarget->GetInstanceID(), new_x, new_y, new_z, pTarget->GetOrientation() );
    // void Unit::Strike( Unit pVictim, uint32 weapon_damage_type, SpellEntry* ability, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool proc_extrahit = false )
    castPtr<Player>(m_caster)->Strike( pTarget, MELEE, NULL, 0, 0, false, false, true );
    castPtr<Player>(m_caster)->Strike( pTarget, OFFHAND, NULL, 0, 0, false, false, true );
}

//------------------------- Aura Effects -----------------------------

void Aura::SpellAuraBindSight(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if(m_caster != NULL || !m_caster->IsPlayer())
        return;
    if(apply)
        m_caster->SetUInt64Value(PLAYER_FARSIGHT, m_target->GetGUID());
    else m_caster->SetUInt64Value(PLAYER_FARSIGHT, 0 );
}

void Aura::SpellAuraModPossess(bool apply)
{

}

void Aura::SpellAuraDummy(bool apply)
{
    if(sSpellProcMgr.HandleAuraProcTriggerDummy(m_target, m_spellProto, mod, apply))
        return;
    if(sSpellMgr.HandleDummyAuraEffect(m_target, this, m_spellProto, mod->i, m_modList, apply))
        return;

    switch(m_spellProto->NameHash)
    {
    case SPELL_HASH_DEATH_AND_DECAY:

        break;
    }
}

void Aura::SpellAuraModConfuse(bool apply)
{

}

void Aura::SpellAuraModCharm(bool apply)
{

}

void Aura::SpellAuraModFear(bool apply)
{

}

void Aura::SpellAuraPeriodicHeal( bool apply )
{
    Unit * m_caster = GetUnitCaster();
    if(m_caster == NULL)
        return;

}

void Aura::SpellAuraModAttackSpeed(bool apply)
{

}

void Aura::SpellAuraModThreatGenerated(bool apply)
{

}

void Aura::SpellAuraModTaunt(bool apply)
{

}

void Aura::SpellAuraModStun(bool apply)
{
    m_target->SetUnitStunned(apply);
}

void Aura::SpellAuraModDamageDone(bool apply)
{

}

void Aura::SpellAuraModDamageTaken(bool apply)
{

}

void Aura::SpellAuraDamageShield(bool apply)
{
    // Todo:PROC
}

void Aura::SpellAuraModStealth(bool apply)
{
    if(apply)
        m_target->SetByteFlag(UNIT_FIELD_BYTES_1, 2, 0x02);
    else m_target->RemoveByteFlag(UNIT_FIELD_BYTES_1, 2, 0x02);

    //0x20000000 is see stealthed
    if( apply )
        m_target->SetByteFlag(PLAYER_FIELD_BYTES2, 3, 0x20);
    else m_target->RemoveByteFlag(PLAYER_FIELD_BYTES2, 3, 0x20);
}

void Aura::SpellAuraModDetect(bool apply)
{

}

void Aura::SpellAuraModInvisibility(bool apply)
{
    m_target->UpdateVisibility();
}

void Aura::SpellAuraModInvisibilityDetection(bool apply)
{
    //Always Positive

    assert(mod->m_miscValue[0] < INVIS_FLAG_TOTAL);
    if(apply)
        m_target->m_invisDetect[mod->m_miscValue[0]] += mod->m_amount;
    else m_target->m_invisDetect[mod->m_miscValue[0]] -= mod->m_amount;

    if(m_target->IsPlayer())
        castPtr<Player>( m_target )->UpdateVisibility();
}

void Aura::SpellAuraModTotalHealthRegenPct(bool apply)
{

}

void Aura::SpellAuraModTotalManaRegenPct(bool apply)
{

}

void Aura::SpellAuraPeriodicTriggerSpell(bool apply)
{

}

void Aura::SpellAuraPeriodicEnergize(bool apply)
{

}

void Aura::SpellAuraModPacify(bool apply)
{

}

void Aura::SpellAuraModRoot(bool apply)
{

}

void Aura::SpellAuraModSilence(bool apply)
{
    if(apply)
    {
        m_target->m_silenced++;
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);

        // remove the current spell (for channelers)
        /*if(m_target->GetCurrentSpell() && m_target->GetGUID() != m_casterGuid &&
            m_target->GetCurrentSpell()->getState() == SPELL_STATE_CASTING )
        {
            m_target->GetCurrentSpell()->cancel();
            m_target->SetCurrentSpell(NULL);
        }*/
    }
    else
    {
        m_target->m_silenced--;

        if(m_target->m_silenced == 0)
        {
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
        }
    }
}

void Aura::SpellAuraReflectSpells(bool apply)
{
    // Todo:PROC
}

void Aura::SpellAuraModStat(bool apply)
{

}

void Aura::SpellAuraModSkill(bool apply)
{

}

void Aura::SpellAuraModIncreaseSpeed(bool apply)
{

}

void Aura::SpellAuraModIncreaseMountedSpeed(bool apply)
{

}

void Aura::SpellAuraModCreatureRangedAttackPower(bool apply)
{

}

void Aura::SpellAuraModDecreaseSpeed(bool apply)
{

}

void Aura::UpdateAuraModDecreaseSpeed()
{

}

void Aura::SpellAuraModIncreaseHealth(bool apply)
{

}

void Aura::SpellAuraModIncreaseEnergy(bool apply)
{

}

void Aura::SpellAuraModShapeshift(bool apply)
{
    if( !m_target->IsPlayer())
        return;
    uint8 shapeShift = m_target->GetShapeShift();
    if(apply == false && shapeShift == mod->m_miscValue[0])
        shapeShift = 0;
    else shapeShift = mod->m_miscValue[0];

    castPtr<Player>(m_target)->SetShapeShift( shapeShift );
}

void Aura::SpellAuraModEffectImmunity(bool apply)
{

}

void Aura::SpellAuraModStateImmunity(bool apply)
{
    //%50 chance to dispel 1 magic effect on target
    //23922
}

void Aura::SpellAuraModSchoolImmunity(bool apply)
{

}

void Aura::SpellAuraModDmgImmunity(bool apply)
{

}

void Aura::SpellAuraModDispelImmunity(bool apply)
{
    assert(mod->m_miscValue[0] < 10);

    if(apply)
        m_target->m_AuraInterface.RemoveAllAurasWithDispelType((uint32)mod->m_miscValue[0]);
}

void Aura::SpellAuraProcTriggerSpell(bool apply)
{
    sSpellProcMgr.HandleAuraProcTriggerSpell(m_target, m_spellProto, mod, apply);
}

void Aura::SpellAuraProcTriggerDamage(bool apply)
{
    sSpellProcMgr.HandleAuraProcTriggerDamage(m_target, m_spellProto, mod, apply);
}

void Aura::SpellAuraTrackCreatures(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
        {
            if(castPtr<Player>( m_target )->TrackingSpell)
                m_target->RemoveAura( castPtr<Player>( m_target )->TrackingSpell);

            m_target->SetUInt32Value(PLAYER_TRACK_CREATURES,(uint32)1<< (mod->m_miscValue[0]-1));
            castPtr<Player>( m_target )->TrackingSpell = GetSpellId();
        }
        else
        {
            castPtr<Player>( m_target )->TrackingSpell = 0;
            m_target->SetUInt32Value(PLAYER_TRACK_CREATURES,0);
        }
    }
}

void Aura::SpellAuraTrackResources(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
        {
            if(castPtr<Player>( m_target )->TrackingSpell)
                m_target->RemoveAura(castPtr<Player>( m_target )->TrackingSpell);

        m_target->SetUInt32Value(PLAYER_TRACK_RESOURCES,(uint32)1<< (mod->m_miscValue[0]-1));
        castPtr<Player>( m_target )->TrackingSpell = GetSpellId();
        }
        else
        {
            castPtr<Player>( m_target )->TrackingSpell = 0;
                m_target->SetUInt32Value(PLAYER_TRACK_RESOURCES,0);
        }
    }
}

void Aura::SpellAuraModParryPerc(bool apply)
{

}

void Aura::SpellAuraModDodgePerc(bool apply)
{

}

void Aura::SpellAuraModBlockPerc(bool apply)
{

}

void Aura::SpellAuraModCritPerc(bool apply)
{

}

void Aura::SpellAuraPeriodicLeech(bool apply)
{

}

void Aura::SpellAuraModHitChance(bool apply)
{

}

void Aura::SpellAuraModSpellHitChance(bool apply)
{

}

void Aura::SpellAuraTransform(bool apply)
{
    uint32 displayId = 0;
    CreatureData* data = sCreatureDataMgr.GetCreatureData(mod->m_miscValue[0]);

    if(data == NULL)
        sLog.Debug("Aura","SpellAuraTransform cannot find CreatureData for id %d",mod->m_miscValue[0]);
    else displayId = data->displayInfo[0];

    Unit * m_caster = GetUnitCaster();
    switch( m_spellProto->Id )
    {
        case 47585: // Dispersion
        {
            if( apply && m_caster != NULL )
            {
                SpellEntry *spellInfo = dbcSpell.LookupEntry( 60069 );
                if(!spellInfo)
                    return;

                SpellCastTargets targets(m_target->GetGUID());
                if(Spell* spell = new Spell(m_target, spellInfo))
                    spell->prepare(&targets, true);
            }
        }break;
        case 20584://wisp
        {
            m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, apply ? 10045:m_target->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
        }break;

        case 30167: // Red Ogre Costume
        {
            if( apply )
                m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, 11549 );
            else
                m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
        }break;

        case 41301: // Time-Lost Figurine
        {
            if( apply )
                m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, 18628 );
            else
                m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
        }break;

        case 16739: // Orb of Deception
        {
            if( apply )
            {
                switch(m_target->getRace())
                {
                    case RACE_ORC:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10139);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10140);
                    }break;
                    case RACE_TAUREN:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10136);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10147);
                    }break;
                    case RACE_TROLL:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10135);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10134);
                    }break;
                    case RACE_UNDEAD:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10146);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10145);
                    }break;
                    case RACE_BLOODELF:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17829);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17830);
                    }break;
                    case RACE_GNOME:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10148);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10149);
                    }break;
                    case RACE_DWARF:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10141);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10142);
                    }break;
                    case RACE_HUMAN:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10137);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10138);
                    }break;
                    case RACE_NIGHTELF:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10143);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10144);
                    }break;
                    case RACE_DRAENEI:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17827);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17828);
                    }break;
                    default:
                        m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
                }
            }
        }break;

        case 42365: // murloc costume
            m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, apply ? 21723 : m_target->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
            break;

        case 19937:
            {
                if (apply)
                {
                    // TODO: Sniff the spell / item, we need to know the real displayID
                    // guessed this may not be correct
                    // human = 7820
                    // dwarf = 7819
                    // halfling = 7818
                    // maybe 7842 as its from a lesser npc
                    m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, 7842);
                }
                else
                {
                    m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
                }
            }break;

        default:
        {
            if(!displayId) return;

            if (apply)
                {
                    m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, displayId);
                }
                else
                {
                    m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
                }
        }break;
    };
}

void Aura::SpellAuraModSpellCritChance(bool apply)
{

}

void Aura::SpellAuraIncreaseSwimSpeed(bool apply)
{

}

void Aura::SpellAuraModCratureDmgDone(bool apply)
{

}

void Aura::SpellAuraPacifySilence(bool apply)
{

}

void Aura::SpellAuraModScale(bool apply)
{
    float current = m_target->GetFloatValue(OBJECT_FIELD_SCALE_X);
    float delta = mod->m_amount/100.0f;

    m_target->SetFloatValue(OBJECT_FIELD_SCALE_X, apply ? (current+current*delta) : current/(1.0f+delta));
}

void Aura::SpellAuraPeriodicHealthFunnel(bool apply)
{

}

void Aura::SpellAuraPeriodicManaLeech(bool apply)
{

}

void Aura::SpellAuraModCastingSpeed(bool apply)
{

}

void Aura::SpellAuraFeignDeath(bool apply)
{
    if( m_target->IsPlayer() )
    {
        Player* pTarget = castPtr<Player>( m_target );
        WorldPacket data(50);
        if( apply )
        {
            pTarget->EventAttackStop();
            pTarget->SetFlag( UNIT_FIELD_FLAGS_2, 1 );
            pTarget->SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH );
            //pTarget->SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_DEAD );
            pTarget->SetFlag( UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_DEAD );
            //pTarget->SetUInt32Value( UNIT_NPC_EMOTESTATE, EMOTE_STATE_DEAD );

            data.SetOpcode( SMSG_START_MIRROR_TIMER );
            data << uint32( 2 );        // type
            data << int32( GetDuration() );
            data << int32( GetDuration() );
            data << uint32( 0xFFFFFFFF );
            data << uint8( 0 );
            data << uint32( m_spellProto->Id );     // ???
            pTarget->PushPacket( &data );

            data.Initialize(SMSG_CLEAR_TARGET);
            data << pTarget->GetGUID();

            //now get rid of mobs agro. pTarget->CombatStatus.AttackersForgetHate() - this works only for already attacking mobs
            /*WorldObject::InRangeArray::iterator itr, itr2;
            for(itr = pTarget->GetInRangeUnitSetBegin(); itr != pTarget->GetInRangeUnitSetEnd();)
            {
                itr2 = itr++;
                Unit* pObject = pTarget->GetInRangeObject<Unit>(*itr2);
                if(pObject->isAlive())
                {
                    //if this is player and targeting us then we interrupt cast
                    if(pObject->IsPlayer())
                    {   //if player has selection on us
                        if( castPtr<Player>(pObject)->GetSelection() == pTarget->GetGUID())
                        {
                            castPtr<Player>(pObject)->SetSelection(0); //lose selection
                            castPtr<Player>(pObject)->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                        }

                        castPtr<Player>( pObject )->PushPacket( &data );
                    }
                }
            }*/
            pTarget->SetDeathState(ALIVE);
        }
        else
        {
            pTarget->RemoveFlag(UNIT_FIELD_FLAGS_2, 1);
            pTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH);
            pTarget->RemoveFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_DEAD);
            //pTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DEAD);
            //pTarget->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
            data.SetOpcode(SMSG_STOP_MIRROR_TIMER);
            data << uint32(2);
            pTarget->PushPacket(&data);
        }
    }
}

void Aura::SpellAuraModDisarm(bool apply)
{
    if(apply)
    {
        if( m_target->IsPlayer() && castPtr<Player>(m_target)->IsInFeralForm())
            return;

        m_target->disarmed = true;
        m_target->disarmedShield = (m_spellProto->NameHash == SPELL_HASH_DISMANTLE);
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
    }
    else
    {
        m_target->disarmed = false;
        if( m_spellProto->NameHash == SPELL_HASH_DISMANTLE )
            m_target->disarmedShield = false;
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
    }
}

void Aura::SpellAuraModStalked(bool apply)
{

}

void Aura::SpellAuraSchoolAbsorb(bool apply)
{
    if(apply)
        mod->fixed_amount = mod->m_amount;
}

void Aura::SpellAuraModSpellCritChanceSchool(bool apply)
{

}

void Aura::SpellAuraModPowerCost(bool apply)
{

}

void Aura::SpellAuraModPowerCostSchool(bool apply)
{

}

void Aura::SpellAuraReflectSpellsSchool(bool apply)
{
    // Todo:PROC
}

void Aura::SpellAuraModLanguage(bool apply)
{

}

void Aura::SpellAuraAddFarSight(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if(m_caster == NULL || !m_caster->IsPlayer())
        return;

    if(apply)
    {
        float sightX = m_caster->GetPositionX() + 100.0f;
        float sightY = m_caster->GetPositionY() + 100.0f;
        m_caster->SetUInt64Value(PLAYER_FARSIGHT, mod->m_miscValue[0]);
        m_caster->GetMapInstance()->ChangeFarsightLocation(castPtr<Player>(m_caster), sightX, sightY, true);

    }
    else
    {
        m_caster->SetUInt64Value(PLAYER_FARSIGHT, 0);
        m_caster->GetMapInstance()->ChangeFarsightLocation(castPtr<Player>(m_caster), 0, 0, false);
    }
}

void Aura::SpellAuraMechanicImmunity(bool apply)
{
    if( m_target->IsPlayer())
    {
        switch(m_spellProto->Id)
        {
        case 49039:
            {
                if(apply && !m_target->HasAura(50397))
                    GetUnitCaster()->GetSpellInterface()->TriggerSpell(dbcSpell.LookupEntry(50397), m_target);
            }
        }
    }

    if(apply)
    {
        if(mod->m_miscValue[0] != MECHANIC_HEALING && mod->m_miscValue[0] != MECHANIC_INVULNARABLE && mod->m_miscValue[0] != MECHANIC_SHIELDED) // dont remove bandages, Power Word and protection effect
        {
            /* Supa's test run of Unit::RemoveAllAurasByMechanic */
            if( m_target ) // just to be sure?
            {
                m_target->m_AuraInterface.RemoveAllAurasByMechanic( (uint32)mod->m_miscValue[0] , -1 , false );
            }

            if(m_spellProto->Id == 42292 || m_spellProto->Id == 59752)  // PvP Trinket
            {
                // insignia of the A/H
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_CHARMED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_DISORIENTED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_FLEEING, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_ROOTED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_PACIFIED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_ASLEEP, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_STUNNED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_INCAPACIPATED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_POLYMORPHED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_SEDUCED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_FROZEN, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_ENSNARED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_BANISHED, -1, false);
            }
        }
    }
}

void Aura::SpellAuraMounted(bool apply)
{
    if(!m_target->IsPlayer())
        return;

    Player* pPlayer = castPtr<Player>(m_target);

    if(apply)
    {
        TRIGGER_INSTANCE_EVENT( pPlayer->GetMapInstance(), OnPlayerMount )( pPlayer );

        pPlayer->m_AuraInterface.RemoveAllAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_MOUNT, GetSpellId());

        CreatureData* ctrData = sCreatureDataMgr.GetCreatureData(mod->m_miscValue[0]);
        if(ctrData == NULL || ctrData->displayInfo[0] == 0)
            return;

        pPlayer->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT);
        pPlayer->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID, ctrData->displayInfo[0]);

        if( pPlayer->GetShapeShift() && !(pPlayer->GetShapeShift() & FORM_BATTLESTANCE | FORM_DEFENSIVESTANCE | FORM_BERSERKERSTANCE ))
            pPlayer->m_AuraInterface.RemoveAllAurasOfType(SPELL_AURA_MOD_SHAPESHIFT, m_spellProto);

        // If we already have a fixed amount, then this is a reapplication of the modifier
        if(mod->fixed_amount)
            return;

        // Grab our mount capability spell
        SpellEntry *mountCapability = pPlayer->GetMountCapability(mod->m_miscValue[1]);
        if(mountCapability == NULL)
            return;

        SpellCastTargets targets(pPlayer->GetGUID());
        if(Spell *mountAbility = new Spell(pPlayer, mountCapability))
        {
            mod->fixed_amount = mountCapability->Id;
            mountAbility->prepare(&targets, true);
        }
    }
    else
    {
        if(mod->fixed_amount)
            pPlayer->RemoveAura(mod->fixed_amount);

        pPlayer->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT);
        pPlayer->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
        pPlayer->m_AuraInterface.RemoveAllAurasByInterruptFlagButSkip( AURA_INTERRUPT_ON_DISMOUNT, GetSpellId() );
    }
}

void Aura::SpellAuraModDamagePercDone(bool apply)
{

}

void Aura::SpellAuraModPercStat(bool apply)
{

}

void Aura::SpellAuraSplitDamage(bool apply)
{

}

void Aura::SpellAuraModRegen(bool apply)
{

}

void Aura::SpellAuraModPowerRegen(bool apply)
{

}

void Aura::SpellAuraChannelDeathItem(bool apply)
{

}

void Aura::SpellAuraModDamagePercTaken(bool apply)
{

}

void Aura::SpellAuraModRegenPercent(bool apply)
{

}

void Aura::SpellAuraModDetectRange(bool apply)
{

}

void Aura::SpellAuraPreventsFleeing(bool apply)
{
    // Curse of Recklessness
}

void Aura::SpellAuraModUnattackable(bool apply)
{
/*
        Also known as Apply Aura: Mod Uninteractible
        Used by: Spirit of Redemption, Divine Intervention, Phase Shift, Flask of Petrification
        It uses one of the UNIT_FIELD_FLAGS, either UNIT_FLAG_NOT_SELECTABLE or UNIT_FLAG_NOT_ATTACKABLE_2
*/
}

void Aura::SpellAuraInterruptRegen(bool apply)
{
    if(apply)
        m_target->m_interruptRegen++;
    else
    {
        m_target->m_interruptRegen--;
        if(m_target->m_interruptRegen < 0)
            m_target->m_interruptRegen = 0;
    }
}

void Aura::SpellAuraGhost(bool apply)
{
    if(m_target->IsPlayer())
    {
        SpellAuraWaterWalk( apply );
    }
    //m_target->SendPowerUpdate();
}

void Aura::SpellAuraMagnet(bool apply)
{

}

void Aura::SpellAuraManaShield(bool apply)
{

}

void Aura::SpellAuraSkillTalent(bool apply)
{

}

void Aura::SpellAuraModAttackPower(bool apply)
{

}

void Aura::SpellAuraVisible(bool apply)
{

}

void Aura::SpellAuraModCreatureAttackPower(bool apply)
{

}

void Aura::SpellAuraModTotalThreat( bool apply )
{

}

void Aura::SpellAuraWaterWalk( bool apply )
{

}

void Aura::SpellAuraFeatherFall( bool apply )
{

}

void Aura::SpellAuraHover( bool apply )
{

}

void Aura::SpellAuraAddFlatModifier(bool apply)
{

}

void Aura::SpellAuraAddPctMod( bool apply )
{

}

void Aura::SpellAuraAddTargetTrigger(bool apply)
{

}

void Aura::SpellAuraModPowerRegPerc(bool apply)
{

}

void Aura::SpellAuraOverrideClassScripts(bool apply)
{

}

void Aura::SpellAuraModRangedDamageTaken(bool apply)
{

}

void Aura::SpellAuraModHealing(bool apply)
{

}

void Aura::SpellAuraIgnoreRegenInterrupt(bool apply)
{

}

void Aura::SpellAuraModMechanicResistance(bool apply)
{

}

void Aura::SpellAuraModHealingPCT(bool apply)
{

}

void Aura::SpellAuraModRangedAttackPower(bool apply)
{
    if(apply)
        m_target->ModUnsigned32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS,mod->m_amount);
    else m_target->ModUnsigned32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS,-mod->m_amount);
}

void Aura::SpellAuraModMeleeDamageTaken(bool apply)
{

}

void Aura::SpellAuraModMeleeDamageTakenPct(bool apply)
{

}

void Aura::SpellAuraRAPAttackerBonus(bool apply)
{

}

void Aura::SpellAuraModPossessPet(bool apply)
{

}

void Aura::SpellAuraModIncreaseSpeedAlways(bool apply)
{

}

void Aura::SpellAuraModIncreaseEnergyPerc( bool apply )
{

}

void Aura::SpellAuraModIncreaseHealthPerc( bool apply )
{

}

void Aura::SpellAuraModManaRegInterrupt( bool apply )
{

}

void Aura::SpellAuraModTotalStatPerc(bool apply)
{

}

void Aura::SpellAuraModHaste( bool apply )
{

}

void Aura::SpellAuraForceReaction( bool apply )
{
    if( !m_target->IsPlayer() )
        return;

    std::map<uint32,uint32>::iterator itr;
    Player* p_target = castPtr<Player>(m_target);

    if( apply )
    {
        itr = p_target->m_forcedReactions.find( mod->m_miscValue[0] );
        if( itr != p_target->m_forcedReactions.end() )
            itr->second = mod->m_amount;
        else
            p_target->m_forcedReactions.insert( std::make_pair( mod->m_miscValue[0], mod->m_amount ) );
    }
    else
        p_target->m_forcedReactions.erase( mod->m_miscValue[0] );

    WorldPacket data( SMSG_SET_FORCED_REACTIONS, ( 8 * p_target->m_forcedReactions.size() ) + 4 );
    data << uint32(p_target->m_forcedReactions.size());
    for( itr = p_target->m_forcedReactions.begin(); itr != p_target->m_forcedReactions.end(); itr++ )
    {
        data << itr->first;
        data << itr->second;
    }

    p_target->PushPacket( &data );
}

void Aura::SpellAuraModRangedHaste(bool apply)
{

}

void Aura::SpellAuraModRangedAmmoHaste(bool apply)
{

}

void Aura::SpellAuraRetainComboPoints(bool apply)
{

}

void Aura::SpellAuraResistPushback(bool apply)
{

}

void Aura::SpellAuraModShieldBlockPCT( bool apply )
{

}

void Aura::SpellAuraTrackStealthed(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster== NULL || !m_caster->IsPlayer() )
        return;

    //0x00000002 is track stealthed
    if( apply )
        m_caster->SetFlag(PLAYER_FIELD_BYTES, 0x00000002);
    else m_caster->RemoveFlag(PLAYER_FIELD_BYTES, 0x00000002);
}

void Aura::SpellAuraModDetectedRange(bool apply)
{

}

void Aura::SpellAuraSplitDamageFlat(bool apply)
{

}

void Aura::SpellAuraModStealthLevel(bool apply)
{

}

void Aura::SpellAuraModUnderwaterBreathing(bool apply)
{

}

void Aura::SpellAuraSafeFall(bool apply)
{

}

void Aura::SpellAuraModReputationAdjust(bool apply)
{

}

void Aura::SpellAuraModBlockValue(bool apply)
{

}

void Aura::SpellAuraNoPVPCredit(bool apply)
{

}

void Aura::SpellAuraModHealthRegInCombat(bool apply)
{

}

void Aura::SpellAuraPowerBurn(bool apply)
{

}

void Aura::SpellAuraWaterBreathing( bool apply )
{

}

void Aura::SpellAuraModHealingDone(bool apply)
{

}

void Aura::SpellAuraModHealingDonePct(bool apply)
{

}

void Aura::SpellAuraEmphaty(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if(m_caster == NULL || m_target == NULL || !m_caster->IsPlayer())
        return;

    m_target->SetUInt32Value(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_PLAYER_INFO);
}

void Aura::SpellAuraUntrackable(bool apply)
{
    if(apply)
        m_target->SetFlag(UNIT_FIELD_BYTES_1, 0x00040000);
    else
        m_target->RemoveFlag(UNIT_FIELD_BYTES_1, 0x00040000);
}

void Aura::SpellAuraModOffhandDamagePCT(bool apply)
{

}

void Aura::SpellAuraModPenetration(bool apply)
{

}

void Aura::SpellAuraSpiritOfRedemption(bool apply)
{
    if(!m_target->IsPlayer())
        return;

    if(apply)
    {
        //m_target->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);
        m_target->SetUInt32Value(UNIT_FIELD_HEALTH, 1);
        SpellEntry *sorInfo = dbcSpell.LookupEntry(27792);
        if(sorInfo == NULL)
            return;

        SpellCastTargets targets(m_target->GetGUID());
        if(Spell* sor = new Spell(m_target, sorInfo))
            sor->prepare(&targets, true);
    }
    else
    {
        //m_target->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
        m_target->RemoveAura(27792);
        m_target->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
    }
}

void Aura::SpellAuraDispelDebuffResist(bool apply)
{

}

void Aura::SendInterrupted(uint8 result, WorldObject* m_ocaster)
{
    if( !m_ocaster->IsInWorld() )
        return;

    WorldPacket data( SMSG_SPELL_FAILURE, 20 );
    if( m_ocaster->IsPlayer() )
    {
        data << m_ocaster->GetGUID();
        data << uint8(0); //extra_cast_number
        data << uint32(m_spellProto->Id);
        data << uint8( result );
        castPtr<Player>( m_ocaster )->PushPacket( &data );
    }

    data.Initialize( SMSG_SPELL_FAILED_OTHER );
    data << m_ocaster->GetGUID();
    data << uint8(0); //extra_cast_number
    data << uint32(m_spellProto->Id);
    data << uint8( result );
    m_ocaster->SendMessageToSet( &data, false );

    m_interrupted = (int16)result;
}

void Aura::SendChannelUpdate(uint32 time, WorldObject* m_ocaster)
{
    WorldPacket data(MSG_CHANNEL_UPDATE, 18);
    data << m_ocaster->GetGUID();
    data << time;

    m_ocaster->SendMessageToSet(&data, true);
}

void Aura::SendPeriodicAuraLog(uint32 amt, uint32 Flags)
{
    SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, amt, 0, 0, Flags, m_triggeredSpellId);
}

void Aura::SendPeriodicAuraLog(uint64 CasterGuid, Unit* Target, SpellEntry *sp, uint32 Amount, int32 abs_dmg, uint32 resisted_damage, uint32 Flags, uint32 pSpellId, bool crit)
{
    if(Target == NULL || !Target->IsInWorld())
        return;

    uint32 spellId = pSpellId ? pSpellId : sp->Id;
    uint8 isCritical = crit ? 1 : 0;
    if(abs_dmg == -1)
        abs_dmg = Amount;

    WorldPacket data(SMSG_PERIODICAURALOG, 46);
    data << Target->GetGUID().asPacked();   // target guid
    data << WoWGuid(CasterGuid).asPacked(); // caster guid
    data << uint32(spellId);            // spellid
    data << uint32(1);                  // count of logs going next
    data << uint32(Flags);              // Log type
    switch(Flags)
    {
    case FLAG_PERIODIC_DAMAGE:
        {
            data << uint32(Amount);
            data << uint32(0);
            data << uint32(SchoolMask(sp->School));
            data << uint32(abs_dmg);
            data << uint32(resisted_damage);
            data << uint8(isCritical);
        }break;
    case FLAG_PERIODIC_HEAL:
        {
            data << uint32(Amount);
            data << uint32(resisted_damage);
            data << uint32(abs_dmg);
            data << uint8(isCritical);
        }break;
    case FLAG_PERIODIC_ENERGIZE:
    case FLAG_PERIODIC_LEECH:
        {
            data << uint32(sp->EffectMiscValue[mod->i]);
            data << uint32(Amount);
            if(Flags == FLAG_PERIODIC_LEECH)
                data << float(sp->EffectValueMultiplier[mod->i]);
        }break;
    default:
        {
            sLog.printf("Unknown type!");
            return;
        }break;
    }
    Target->SendMessageToSet(&data, true);
}

void Aura::AttemptDispel(Unit* pCaster, bool canResist)
{
    m_dispelled = true;
    Remove();
}

uint8 Aura::GetMaxProcCharges(Unit* caster)
{
    uint32 charges = m_spellProto->procCharges;
    if(caster)
    {
        caster->SM_FIValue(SMT_CHARGES, (int32*)&charges, m_spellProto->SpellGroupType);
        caster->SM_PIValue(SMT_CHARGES, (int32*)&charges, m_spellProto->SpellGroupType);
    }
    return uint8(charges & 0xFF);
}

void Aura::RecalculateModBaseAmounts()
{
    Unit *unitCaster = GetUnitCaster();
    Player *playerCaster = unitCaster ? unitCaster->IsPlayer() ? castPtr<Player>(unitCaster) : NULL : NULL;
    for(uint32 i = 0; i < m_modcount; i++)
    {
        int32 value = m_spellProto->CalculateSpellPoints(m_modList[i].i, m_auraLevel);
        if( unitCaster != NULL )
        {
            int32 spell_flat_modifers = 0, spell_pct_modifers = 0;
            unitCaster->SM_FIValue(SMT_MISC_EFFECT, &spell_flat_modifers,GetSpellProto()->SpellGroupType);
            unitCaster->SM_FIValue(SMT_MISC_EFFECT, &spell_pct_modifers, GetSpellProto()->SpellGroupType);

            if( m_modList[i].i == 0 )
            {
                unitCaster->SM_FIValue(SMT_FIRST_EFFECT_BONUS,&spell_flat_modifers,GetSpellProto()->SpellGroupType);
                unitCaster->SM_FIValue(SMT_FIRST_EFFECT_BONUS,&spell_pct_modifers,GetSpellProto()->SpellGroupType);
            }
            else if( m_modList[i].i == 1 )
            {
                unitCaster->SM_FIValue(SMT_SECOND_EFFECT_BONUS,&spell_flat_modifers,GetSpellProto()->SpellGroupType);
                unitCaster->SM_FIValue(SMT_SECOND_EFFECT_BONUS,&spell_pct_modifers,GetSpellProto()->SpellGroupType);
            }

            if( ( m_modList[i].i == 2 ) || ( m_modList[i].i == 1 && GetSpellProto()->Effect[2] == 0 ) || ( m_modList[i].i == 0 && GetSpellProto()->Effect[1] == 0 && GetSpellProto()->Effect[2] == 0 ) )
            {
                unitCaster->SM_FIValue(SMT_LAST_EFFECT_BONUS,&spell_flat_modifers,GetSpellProto()->SpellGroupType);
                unitCaster->SM_FIValue(SMT_LAST_EFFECT_BONUS,&spell_pct_modifers,GetSpellProto()->SpellGroupType);
            }
            value += float2int32(value * (float)(spell_pct_modifers / 100.0f)) + spell_flat_modifers;
        }

        m_modList[i].m_baseAmount = value;
    }
    UpdateModAmounts();
}

void Aura::UpdateModAmounts()
{
    Unit * m_caster = GetUnitCaster();
    for(uint8 i = 0; i < m_modcount; i++)
    {
        if( m_modList[i].m_bonusAmount == 0) CalculateBonusAmount(m_caster, i);
        if(m_modList[i].m_baseAmount >= 0)
            m_modList[i].m_amount = m_modList[i].m_baseAmount+m_modList[i].m_bonusAmount;
        else m_modList[i].m_amount = m_modList[i].m_baseAmount-m_modList[i].m_bonusAmount;
        if(m_stackSizeorProcCharges >= 0) m_modList[i].m_amount *= m_stackSizeorProcCharges;
        if(m_target) m_target->OnAuraModChanged(m_modList[i].m_type);//m_AuraInterface.SetModMaskBit(m_modList[i].m_type);
    }
}

void Aura::CalculateBonusAmount(Unit *caster, uint8 index)
{
    if(index >= m_modcount || index >= 3)
        return;
    if(caster == NULL || m_target == NULL)
        return;

    m_modList[index].m_bonusAmount = caster->GetSpellBonusDamage(m_target, m_spellProto, m_modList[index].i, m_modList[index].m_baseAmount, m_spellProto->isSpellHealingEffect());
    if(m_modList[index].m_bonusAmount > m_modList[index].m_baseAmount)
        m_modList[index].m_bonusAmount -= m_modList[index].m_baseAmount;
    else m_modList[index].m_bonusAmount = 0;
}

void Aura::AddStackSize(uint8 mod)
{
    if(mod == 0 || m_stackSizeorProcCharges < 0)
        return;

    uint16 maxStack = (m_spellProto->maxstack&0xFFFF);
    if(maxStack && m_stackSizeorProcCharges == maxStack)
        return;

    int16 newStack = m_stackSizeorProcCharges + mod;
    if(maxStack && newStack > maxStack)
        newStack = maxStack;

    m_stackSizeorProcCharges = newStack;
    BuildAuraUpdate();

    // now need to update amount and reapply modifiers
    ApplyModifiers(false);
    UpdateModAmounts();
    ApplyModifiers(true);
}

void Aura::RemoveStackSize(uint8 mod)
{
    if(mod == 0 || m_stackSizeorProcCharges < 0)
        return;

    if(m_stackSizeorProcCharges > mod)
    {
        m_stackSizeorProcCharges -= mod;
        BuildAuraUpdate();
        return;
    }

    m_stackSizeorProcCharges = 0;
    Remove();
}

void Aura::SetProcCharges(uint8 mod)
{
    if(m_stackSizeorProcCharges > 0)
        return;

    if(mod == 0)
        m_stackSizeorProcCharges = 0;
    else
    {
        m_stackSizeorProcCharges = -(mod&0xFF);
        BuildAuraUpdate();
    }
}

void Aura::RemoveProcCharges(uint8 mod)
{
    if(mod == 0 || m_stackSizeorProcCharges > 0)
        return;

    if(m_stackSizeorProcCharges < mod)
    {
        m_stackSizeorProcCharges += mod;
        BuildAuraUpdate();
        return;
    }

    m_stackSizeorProcCharges = 0;
    Remove();
}

void Aura::SpellAuraAllowTamePetType(bool apply)
{

}

void Aura::SpellAuraAddCreatureImmunity(bool apply)
{

}

void Aura::SpellAuraReduceAOEDamageTaken(bool apply)
{

}

void Aura::SpecialCases()
{
    //We put all the special cases here, so we keep the code clean.
    switch(m_spellProto->Id)
    {
    case 12976:// Last Stand
    case 50322:// Survival Instincts
        {
            mod->m_amount = (uint32)(m_target->GetUInt32Value(UNIT_FIELD_MAXHEALTH) * 0.3);
        }break;
    case 23782:// Gift of Life
        {
            mod->m_amount = 1500;
        }break;
    case 48418:// Master Shapeshifter Physical Damage
    case 48420:// Master Shapeshifter CritChance
    case 48421:// Master Shapeshifter SpellDamage
    case 48422:// Master Shapeshifter Healing
        {
            if(castPtr<Player>(m_target)->HasSpell(48411))
                mod->m_amount =  2;
            if(castPtr<Player>(m_target)->HasSpell(48412))
                mod->m_amount =  4;
        }break;
    }
}

void Aura::SpellAuraModPetTalentPoints(bool apply)
{
    if( !m_target->IsPlayer() )
        return;

}
