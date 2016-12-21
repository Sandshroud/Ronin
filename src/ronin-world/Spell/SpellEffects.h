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

enum SpellEffectIndex
{
    SP_EFF_INDEX_0 = 0,
    SP_EFF_INDEX_1 = 1,
    SP_EFF_INDEX_2 = 2,
    SP_EFF_INDEX_MAX
};

class SpellEffectClass : public BaseSpell
{
public:
    SpellEffectClass(WorldObject* caster, SpellEntry *info, uint8 castNumber, WoWGuid itemGuid);
    ~SpellEffectClass();
    virtual void Destruct();

    // Calculates the i'th effect value
    int32 CalculateEffect(uint32 effIndex, WorldObject* target);

    // Handle the Effects of the Spell
    void HandleEffects(uint32 i, SpellTarget *spTarget, WorldObject *target);
    void HandleDelayedEffects(Unit *unitTarget, SpellTarget *spTarget);
    // Handles Teleport function
    void HandleTeleport(uint32 id, Unit* Target);
    // Handles healing function
    void Heal(Unit *target, uint8 effIndex, int32 amount);
    // Handles skill up
    void DetermineSkillUp(Player *target, uint32 skillid,uint32 targetlevel, uint32 multiplicator = 1);
    void DetermineSkillUp(Player *target, uint32 skillid);

    static void InitializeSpellEffectClass();
    typedef void (SpellEffectClass::*pSpellEffect)(uint32 i, WorldObject *target, int32 amount, bool rawAmt);

    static std::map<uint8, pSpellEffect> m_spellEffectMap;

    virtual void GetSpellDestination(LocationVector &dest) { m_caster->GetPosition(dest); }

private:
    typedef Loki::AssocVector<WoWGuid, Aura*> AuraTargetMap;
    AuraTargetMap m_tempAuras;

protected: // Effect Handlers
    void SpellEffectNULL(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    // Delayed effect can be left empty
    RONIN_INLINE void SpellEffectDelayed(uint32 i, WorldObject *target, int32 amount, bool rawAmt) {}
    // regular spell effect handlers
    void SpellEffectInstantKill(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSchoolDMG(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDummy(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectRestoreHealthPct(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectRestoreManaPct(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectTeleportUnits(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectApplyAura(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectPowerDrain(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectHealthLeech(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectHeal(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectBind(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectQuestComplete(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectWeaponDamageNoSchool(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectResurrect(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectAddExtraAttacks(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectCreateItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectPersistentAA(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSummon(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectLeap(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectEnergize(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectWeaponDmgPerc(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectTriggerMissile(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectOpenLock(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectApplyAA(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectLearnSpell(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDispel(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSkillStep(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDetect(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSummonObject(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectEnchantItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectEnchantItemTemporary(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectTameCreature(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSummonPet(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectWeaponDamage(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectPowerBurn(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectThreat(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectTriggerSpell(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectPowerFunnel(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectHealMaxHealth(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectInterruptCast(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDistract(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectPickpocket(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectAddFarsight(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectResetTalents(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectUseGlyph(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectHealMechanical(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectScriptEffect(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSanctuary(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectAddComboPoints(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDuel(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectStuck(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSummonPlayer(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectActivateObject(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectWMODamage(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectWMORepair(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectChangeWMOState(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectProficiency(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSendEvent(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSkinning(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectCharge(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectPlaceTotemsOnBar(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectKnockBack(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectInebriate(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectFeedPet(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDismissPet(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectReputation(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSummonObjectSlot(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDispelMechanic(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSummonDeadPet(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDestroyAllTotems(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSummonDemonOld(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectAttackMe(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSkill(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDummyMelee(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectPull(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSpellSteal(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectProspecting(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectOpenLockItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSelfResurrect(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDisenchant(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSkinPlayerCorpse(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectResurrectNew(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectTranformItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectEnvironmentalDamage(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectLearnPetSpell(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectEnchantHeldItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectAddHonor(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSpawn(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectRedirectThreat(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectPlayMusic(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectTriggerSpellWithValue(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectJump(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectTeleportToCaster(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectMilling(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectAllowPetRename(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectAddPrismaticSocket(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectActivateRune(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectFailQuest(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectStartQuest(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectCreatePet(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectTitanGrip(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectCreateRandomItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSendTaxi(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectKillCredit(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectForceCast(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectSetTalentSpecsCount(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectActivateTalentSpec(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectDisengage(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectClearFinishedQuest(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectApplyDemonAura(uint32 i, WorldObject *target, int32 amount, bool rawAmt);
    void SpellEffectRemoveAura(uint32 i, WorldObject *target, int32 amount, bool rawAmt);

    // Summons
    void SummonWild(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonTotem(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonGuardian(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonTemporaryPet(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonPossessed(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonCompanion(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonVehicle(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry *spe, CreatureData *proto, LocationVector &v);
};
