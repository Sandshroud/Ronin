/***
 * Demonstrike Core
 */

#pragma once

class SpellEffectClass : public BaseSpell
{
public:
    SpellEffectClass(WorldObject* caster, SpellEntry *info, uint8 castNumber);
    ~SpellEffectClass();
    virtual void Destruct();

    // Calculates the i'th effect value
    int32 CalculateEffect(uint32 effIndex, WorldObject* target);

    // Handle the Effects of the Spell
    void HandleEffects(uint32 i, WorldObject *target);

    // Add auras after handling effects
    void HandleAddAura(Unit *target);

    static void InitializeSpellEffectClass();
    typedef void (SpellEffectClass::*pSpellEffect)(uint32 i, WorldObject *target, int32 amount);

    static std::map<uint8, pSpellEffect> m_spellEffectMap;

    virtual void GetSpellDestination(LocationVector &dest) { m_caster->GetPosition(dest); }

private:
    std::map<uint64, Aura*> m_tempAuras;

protected: // Effect Handlers
    void SpellEffectNULL(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectInstantKill(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSchoolDMG(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDummy(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectRestoreHealthPct(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectRestoreManaPct(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectTeleportUnits(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectApplyAura(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectPowerDrain(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectHealthLeech(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectHeal(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectBind(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectQuestComplete(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectWeaponDamageNoSchool(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectResurrect(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectAddExtraAttacks(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDodge(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectBlock(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectParry(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectCreateItem(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectPersistentAA(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSummon(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectLeap(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectEnergize(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectWeaponDmgPerc(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectTriggerMissile(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectOpenLock(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectApplyAA(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectLearnSpell(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSpellDefense(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDispel(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSkillStep(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDetect(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSummonObject(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectEnchantItem(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectEnchantItemTemporary(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectTameCreature(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSummonPet(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectWeaponDamage(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectPowerBurn(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectThreat(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectTriggerSpell(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectHealthFunnel(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectPowerFunnel(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectHealMaxHealth(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectInterruptCast(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDistract(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectPickpocket(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectAddFarsight(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectResetTalents(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectUseGlyph(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectHealMechanical(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSummonObjectWild(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectScriptEffect(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSanctuary(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectAddComboPoints(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectCreateHouse(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDuel(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectStuck(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSummonPlayer(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectActivateObject(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectWMODamage(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectWMORepair(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectChangeWMOState(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectProficiency(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSendEvent(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSkinning(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectCharge(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectPlaceTotemsOnBar(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectKnockBack(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectInebriate(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectFeedPet(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDismissPet(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectReputation(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSummonObjectSlot(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDispelMechanic(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSummonDeadPet(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDestroyAllTotems(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSummonDemonOld(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectAttackMe(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSkill(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectApplyPetAura(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDummyMelee(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectPull(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSpellSteal(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectProspecting(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectOpenLockItem(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSelfResurrect(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDisenchant(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectWeapon(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDefense(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectLanguage(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDualWield(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSkinPlayerCorpse(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectResurrectNew(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectTranformItem(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectEnvironmentalDamage(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectLearnPetSpell(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectEnchantHeldItem(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectAddHonor(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSpawn(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectRedirectThreat(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectPlayMusic(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectTriggerSpellWithValue(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectJump(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectTeleportToCaster(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectMilling(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectAllowPetRename(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectAddPrismaticSocket(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectActivateRune(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectFailQuest(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectStartQuest(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectCreatePet(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectTitanGrip(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectCreateRandomItem(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSendTaxi(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectKillCredit(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectForceCast(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectSetTalentSpecsCount(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectActivateTalentSpec(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectDisengage(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectClearFinishedQuest(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectApplyDemonAura(uint32 i, WorldObject *target, int32 amount);
    void SpellEffectRemoveAura(uint32 i, WorldObject *target, int32 amount);

    // Summons
    void SummonWild(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonTotem(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonGuardian(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonTemporaryPet(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonPossessed(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonCompanion(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry* spe, CreatureData* proto, LocationVector & v);
    void SummonVehicle(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry *spe, CreatureData *proto, LocationVector &v);
};
