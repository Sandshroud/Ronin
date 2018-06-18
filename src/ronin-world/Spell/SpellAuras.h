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

#pragma once

class SERVER_DECL Aura
{
public:
    Aura(Unit *target, SpellEntry *proto, uint16 auraFlags, uint8 auraLevel, int16 auraStackCharge, time_t expirationTime, WoWGuid casterGuid);
    ~Aura();

    static void InitializeAuraHandlerClass();
    typedef void(Aura::*pSpellAura)(bool apply);
    static std::map<uint16, pSpellAura> m_auraHandlerMap;

    void Remove();
    void Update(uint32 diff);
    void UpdatePreApplication();
    void OnTargetChangeLevel(uint32 newLevel, uint64 targetGuid);
    void AddMod(uint32 i, uint32 t, int32 a, uint32 b = 0, int32 f = 0, float ff = 0);

    RONIN_INLINE SpellEntry* GetSpellProto() const { return m_spellProto; }
    RONIN_INLINE uint32 GetSpellId() const { return m_spellProto->Id; }
    RONIN_INLINE bool IsPassive() { return (m_spellProto->isPassiveSpell() || m_spellProto->isHiddenSpell()) && !m_areaAura; }

    RONIN_INLINE uint32 GetModCount() const { return m_modcount; }
    RONIN_INLINE Modifier *GetMod(uint32 x) { return &m_modList[x]; }

    void ResetExpirationTime();
    time_t GetExpirationTime() { return m_expirationTime; }
    RONIN_INLINE int32 GetDuration() const { return m_duration; }
    RONIN_INLINE int32 GetMSTimeLeft()
    {
        if(m_expirationTime == 0)
            return -1;
        if(m_expirationTime <= UNIXTIME)
            return 0;
        return (m_expirationTime-UNIXTIME)*1000;
    }

    RONIN_INLINE uint32 GetTriggeredSpellId() { return m_triggeredSpellId; }
    void SetTriggerSpellId(uint32 spellid) { m_triggeredSpellId = spellid; }
    RONIN_INLINE uint8 GetAuraSlot() const { return m_auraSlot; }
    void SetAuraSlot(uint8 slot) { m_auraSlot = slot; }
    RONIN_INLINE uint16 GetAuraFlags() const { return m_auraFlags; }
    void SetAuraFlags(uint16 flags) { m_auraFlags = flags; }
    void SetAuraFlag(uint16 flag) { m_auraFlags |= flag; };
    RONIN_INLINE uint8 GetAuraLevel() const { return m_auraLevel; }
    void SetAuraLevel(uint8 level) { m_auraLevel = level; }

    RONIN_INLINE bool IsAreaAura() { return m_areaAura; }
    RONIN_INLINE bool IsApplied() { return m_applied; }
    RONIN_INLINE bool IsDeleted() { return m_deleted; }
    RONIN_INLINE bool IsPositive() { return m_positive; }
    void SetNegative() { m_positive = false; }
    void SetPositive() { m_positive = true; }

    Unit* GetUnitCaster();
    RONIN_INLINE WoWGuid GetCasterGUID() { return m_casterGuid; }

    RONIN_INLINE Unit* GetUnitTarget() { return m_target; }
    RONIN_INLINE uint64 GetTargetGUID() { return m_target->GetGUID(); }

    void RemoveIfNecessary();

    void ApplyModifiers(bool apply);
    void UpdateModifiers();
    bool AddAuraVisual();
    void BuildAuraUpdate();
    void BuildAuraUpdatePacket(WorldPacket *data);
    void EventRelocateRandomTarget();
    void AttemptDispel(Unit* pCaster, bool canResist = true);

    uint8 getStackSize()
    {
        if(m_stackSizeorProcCharges <= 0)
            return 0;
        return (m_stackSizeorProcCharges&0xFF);
    }
    uint8 getProcCharges()
    {
        if(m_stackSizeorProcCharges >= 0)
            return 0;
        return (m_stackSizeorProcCharges&0xFF);
    }
    int16 getStackSizeOrProcCharges() { return m_stackSizeorProcCharges; };

    void AddStackSize(uint8 mod);
    void RemoveStackSize(uint8 mod);
    void SetProcCharges(uint8 mod);
    void RemoveProcCharges(uint8 mod);

    uint8 GetMaxProcCharges(Unit* caster);
    void RecalculateModBaseAmounts();
    void UpdateModAmounts();
    void CalculateBonusAmount(Unit *caster, uint8 index);

    void UpdateAuraModDecreaseSpeed();

    // Events
    void EventPeriodicDamage(uint32);
    void EventPeriodicDamagePercent(uint32);
    void EventPeriodicHeal(uint32);
    void EventPeriodicHealPct(float);
    void EventPeriodicTriggerSpell(SpellEntry* spellInfo, bool overridevalues, int32 overridevalue);
    void EventPeriodicEnergize(uint32,uint32);
    void EventPeriodicHeal1(uint32);
    void EventPeriodicLeech(uint32, SpellEntry*);
    void EventPeriodicBurn(uint32,uint32);
    void EventPeriodicHealthFunnel(uint32);
    void EventPeriodicManaLeech(uint32);
    void EventPeriodicManaPct(float);
    void EventPeriodicRegenManaStatPct(uint32 perc,uint32 stat);
    void EventPeriodicSpeedModify(int32 mod);
    void EventPeriodicDrink(uint32 amount);

    // log message's
    void SendPeriodicAuraLog(uint32 amt, uint32 Flags);
    void SendPeriodicAuraLog(uint64 casterGuid, Unit* Target, SpellEntry *sp, uint32 Amount, int32 abs_dmg, uint32 resisted_damage, uint32 Flags, uint32 pSpellId = 0, bool crit = false);

    bool WasCastInDuel() { return m_castInDuel; }

    // this stuff can be cached in spellproto.
    RONIN_INLINE bool IsCombatStateAffecting()
    {
        for(uint32 x = 0; x < 3; ++x) {
            if(m_spellProto->EffectApplyAuraName[x] == SPELL_AURA_PERIODIC_DAMAGE ||
                m_spellProto->EffectApplyAuraName[x] == SPELL_AURA_PERIODIC_DAMAGE_PERCENT ||
                m_spellProto->EffectApplyAuraName[x] == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                m_spellProto->EffectApplyAuraName[x] == SPELL_AURA_PERIODIC_LEECH ||
                m_spellProto->EffectApplyAuraName[x] == SPELL_AURA_PERIODIC_MANA_LEECH)
            {
                return true;
            }
        }

        return false;
    }

    RONIN_INLINE bool HasMechanic(uint32 MechanicsType)
    {
        return Spell::HasMechanic(m_spellProto, MechanicsType);
    }
    RONIN_INLINE uint32 GetMechanic()
    {
        return Spell::GetMechanic(m_spellProto);
    }

    RONIN_INLINE uint32 GetMechanicOfEffect(uint32 i)
    {
        return Spell::GetMechanicOfEffect(m_spellProto, i);
    }

private:
    void TriggerPeriodic(uint32 i);
    void CalculateDuration();

    RONIN_INLINE bool IsInrange(float x1, float y1, float z1, WorldObject* obj, float square_r)
    {
        float t = x1-obj->GetPositionX();
        float r = t*t;
        t=y1-obj->GetPositionY();
        r+=t*t;
        t=z1-obj->GetPositionZ();
        r+=t*t;
        return (r <= square_r);
    }

    Unit *m_target;
    WoWGuid m_casterGuid;

    time_t m_expirationTime;
    int32 m_duration;
    bool m_castInDuel;

    uint32 m_modcount;
    Modifier m_modList[3], *mod;

    struct PeriodicAura
    {
        struct PeriodicModifier
        {
            uint32 rate, timer;

        } *periodicMod[3];
    } *_periodicData;

protected:
    SpellEntry * m_spellProto;
    bool m_dispelled;
    bool m_applied, m_positive, m_deleted;
    bool m_areaAura, m_creatureAA;        // Area aura stuff -> never passive.
    int16 m_stackSizeorProcCharges;

    uint8 m_auraSlot;

    uint32 m_castedItemId;
    uint16 m_auraFlags;
    uint8 m_auraLevel;
    uint32 m_triggeredSpellId; //this represents the triggering spell id
    int16 m_interrupted;

    void SendInterrupted(uint8 result, WorldObject* m_caster);
    void SendChannelUpdate(uint32 time, WorldObject* m_caster);
    void SpecialCases();

public:
    RONIN_INLINE bool IsInterrupted() { return ( m_interrupted >= 0 ); }

public:
    // Ignore that shit.
    void SpellAuraIgnore(bool apply) {};

    // Aura Handlers
    void SpellAuraBindSight(bool apply);
    void SpellAuraModPossess(bool apply);
    void SpellAuraDummy(bool apply);
    void SpellAuraModConfuse(bool apply);
    void SpellAuraModCharm(bool apply);
    void SpellAuraModFear(bool apply);
    void SpellAuraPeriodicHeal(bool apply);
    void SpellAuraModAttackSpeed(bool apply);
    void SpellAuraModThreatGenerated(bool apply);
    void SpellAuraModTaunt(bool apply);
    void SpellAuraModStun(bool apply);
    void SpellAuraModDamageDone(bool apply);
    void SpellAuraModDamageTaken(bool apply);
    void SpellAuraDamageShield(bool apply);
    void SpellAuraModStealth(bool apply);
    void SpellAuraModDetect(bool apply);
    void SpellAuraModInvisibility(bool apply);
    void SpellAuraModInvisibilityDetection(bool apply);
    void SpellAuraModTotalHealthRegenPct(bool apply);
    void SpellAuraModTotalManaRegenPct(bool apply);
    void SpellAuraPeriodicTriggerSpell(bool apply);
    void SpellAuraPeriodicEnergize(bool apply);
    void SpellAuraModPacify(bool apply);
    void SpellAuraModRoot(bool apply);
    void SpellAuraModSilence(bool apply);
    void SpellAuraReflectSpells(bool apply);
    void SpellAuraModStat(bool apply);
    void SpellAuraModSkill(bool apply);
    void SpellAuraModIncreaseSpeed(bool apply);
    void SpellAuraModDecreaseSpeed(bool apply);
    void SpellAuraModIncreaseHealth(bool apply);
    void SpellAuraModIncreaseEnergy(bool apply);
    void SpellAuraModShapeshift(bool apply);
    void SpellAuraModEffectImmunity(bool apply);
    void SpellAuraModStateImmunity(bool apply);
    void SpellAuraModSchoolImmunity(bool apply);
    void SpellAuraModDmgImmunity(bool apply);
    void SpellAuraModDispelImmunity(bool apply);
    void SpellAuraProcTriggerSpell(bool apply);
    void SpellAuraProcTriggerDamage(bool apply);
    void SpellAuraTrackCreatures(bool apply);
    void SpellAuraTrackResources(bool apply);
    void SpellAuraModParryPerc(bool apply);
    void SpellAuraModDodgePerc(bool apply);
    void SpellAuraModBlockPerc(bool apply);
    void SpellAuraModCritPerc(bool apply);
    void SpellAuraPeriodicLeech(bool apply);
    void SpellAuraModHitChance(bool apply);
    void SpellAuraModSpellHitChance(bool apply);
    void SpellAuraTransform(bool apply);
    void SpellAuraModSpellCritChance(bool apply);
    void SpellAuraIncreaseSwimSpeed(bool apply);
    void SpellAuraModCratureDmgDone(bool apply);
    void SpellAuraPacifySilence(bool apply);
    void SpellAuraModScale(bool apply);
    void SpellAuraPeriodicHealthFunnel(bool apply);
    void SpellAuraPeriodicManaLeech(bool apply);
    void SpellAuraModCastingSpeed(bool apply);
    void SpellAuraFeignDeath(bool apply);
    void SpellAuraModDisarm(bool apply);
    void SpellAuraModStalked(bool apply);
    void SpellAuraSchoolAbsorb(bool apply);
    void SpellAuraModSpellCritChanceSchool(bool apply);
    void SpellAuraModPowerCost(bool apply);
    void SpellAuraModPowerCostSchool(bool apply);
    void SpellAuraReflectSpellsSchool(bool apply);
    void SpellAuraModLanguage(bool apply);
    void SpellAuraAddFarSight(bool apply);
    void SpellAuraMechanicImmunity(bool apply);
    void SpellAuraMounted(bool apply);
    void SpellAuraModDamagePercDone(bool apply);
    void SpellAuraModPercStat(bool apply);
    void SpellAuraSplitDamage(bool apply);
    void SpellAuraWaterBreathing(bool apply);
    void SpellAuraModRegen(bool apply);
    void SpellAuraModPowerRegen(bool apply);
    void SpellAuraChannelDeathItem(bool apply);
    void SpellAuraModDamagePercTaken(bool apply);
    void SpellAuraModRegenPercent(bool apply);
    void SpellAuraModDetectRange(bool apply);
    void SpellAuraPreventsFleeing(bool apply);
    void SpellAuraModUnattackable(bool apply);
    void SpellAuraInterruptRegen(bool apply);
    void SpellAuraGhost(bool apply);
    void SpellAuraMagnet(bool apply);
    void SpellAuraManaShield(bool apply);
    void SpellAuraSkillTalent(bool apply);
    void SpellAuraModAttackPower(bool apply);
    void SpellAuraVisible(bool apply);
    void SpellAuraModCreatureAttackPower(bool apply);
    void SpellAuraModTotalThreat(bool apply);
    void SpellAuraWaterWalk(bool apply);
    void SpellAuraFeatherFall(bool apply);
    void SpellAuraHover(bool apply);
    void SpellAuraAddFlatModifier(bool apply);
    void SpellAuraAddPctMod(bool apply);
    void SpellAuraAddTargetTrigger(bool apply);
    void SpellAuraModPowerRegPerc(bool apply);
    void SpellAuraOverrideClassScripts(bool apply);
    void SpellAuraModRangedDamageTaken(bool apply);
    void SpellAuraModHealing(bool apply);
    void SpellAuraIgnoreRegenInterrupt(bool apply);
    void SpellAuraModMechanicResistance(bool apply);
    void SpellAuraModHealingPCT(bool apply);
    void SpellAuraUntrackable(bool apply);
    void SpellAuraEmphaty(bool apply);
    void SpellAuraModOffhandDamagePCT(bool apply);
    void SpellAuraModPenetration(bool apply);
    void SpellAuraModRangedAttackPower(bool apply);
    void SpellAuraModMeleeDamageTaken(bool apply);
    void SpellAuraModMeleeDamageTakenPct(bool apply);
    void SpellAuraRAPAttackerBonus(bool apply);
    void SpellAuraModPossessPet(bool apply);
    void SpellAuraModIncreaseSpeedAlways(bool apply);
    void SpellAuraModIncreaseMountedSpeed(bool apply);
    void SpellAuraModCreatureRangedAttackPower(bool apply);
    void SpellAuraModIncreaseEnergyPerc(bool apply);
    void SpellAuraModIncreaseHealthPerc(bool apply);
    void SpellAuraModManaRegInterrupt(bool apply);
    void SpellAuraModHealingDone(bool apply);
    void SpellAuraModHealingDonePct(bool apply);
    void SpellAuraModTotalStatPerc(bool apply);
    void SpellAuraModHaste(bool apply);
    void SpellAuraForceReaction(bool apply);
    void SpellAuraModRangedHaste(bool apply);
    void SpellAuraModRangedAmmoHaste(bool apply);
    void SpellAuraSafeFall(bool apply);
    void SpellAuraModPetTalentPoints(bool apply);
    void SpellAuraAllowTamePetType(bool apply);
    void SpellAuraAddCreatureImmunity(bool apply);
    void SpellAuraRetainComboPoints(bool apply);
    void SpellAuraResistPushback(bool apply);
    void SpellAuraModShieldBlockPCT(bool apply);
    void SpellAuraTrackStealthed(bool apply);
    void SpellAuraModDetectedRange(bool apply);
    void SpellAuraSplitDamageFlat(bool apply);
    void SpellAuraModStealthLevel(bool apply);
    void SpellAuraModUnderwaterBreathing(bool apply);
    void SpellAuraModReputationAdjust(bool apply);
    void SpellAuraModBlockValue(bool apply);
    void SpellAuraNoPVPCredit(bool apply);
    void SpellAuraReduceAOEDamageTaken(bool apply);
    void SpellAuraModHealthRegInCombat(bool apply);
    void SpellAuraPowerBurn(bool apply);
    void SpellAuraSpiritOfRedemption(bool apply);
    void SpellAuraDispelDebuffResist(bool apply);

};
