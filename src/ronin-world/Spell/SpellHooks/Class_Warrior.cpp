/***
* Demonstrike Core
*/

#include "StdAfx.h"

bool HeroicStrikeDummyEffect(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, SpellTarget *spTarget, int32 &amount)
{
    ASSERT(target->IsUnit());
    // Damage calc is spell effect index's amount as a percentage of attack power
    spTarget->accumAmount += float2int32(((float)spTarget->effectAmount[effIndex])*((float)caster->CalculateAttackPower())/100.f);
    return true;
}

bool HeroicStrikeAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{   // Description says 60/100
    amount = 60;
    return true;
}

bool ChargeDummyEffect(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, SpellTarget *spTarget, int32 &amount)
{
    // Trigger our energize spell
    static SpellEntry *chargeEnergize = dbcSpell.LookupEntry(34846);
    if(chargeEnergize != NULL)
        caster->GetSpellInterface()->LaunchSpellFromSpell(chargeEnergize, caster, spell->GetSpellProto());

    static SpellEntry *JuggernaughtTalent = dbcSpell.LookupEntry(64976);
    static SpellEntry *JuggernaughtEffect = dbcSpell.LookupEntry(65156);
    if(JuggernaughtEffect && JuggernaughtTalent && caster->HasAura(JuggernaughtTalent->Id))
        caster->GetSpellInterface()->TriggerSpell(JuggernaughtEffect, caster);
    return true;
}

bool ChargeEnergizeAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{   // Our parent spell is the charge that triggered us
    if(SpellEntry *parent = spell->GetSpellParent())
        amount = parent->EffectBasePoints[1];
    else amount = 150; // Hard code
    return true;
}

bool ThunderClapAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    amount += float2int32(12.f*((float)caster->CalculateAttackPower())/100.f);
    return true;
}

bool RevengeAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    amount += float2int32(31.f*((float)caster->CalculateAttackPower())/100.f);
    return true;
}

bool ConcussionBlowDummyEffect(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, SpellTarget *spTarget, int32 &amount)
{
    ASSERT(target->IsUnit());
    // Damage calc is spell effect index's amount as a percentage of attack power
    int32 damage = spTarget->effectAmount[effIndex];
    damage = float2int32(((float)damage)*((float)caster->CalculateAttackPower())/100.f);
    // Bonus damage check
    damage = caster->GetSpellBonusDamage(castPtr<Unit>(target), spell->GetSpellProto(), effIndex, damage, false);
    spTarget->accumAmount = damage;
    return true;
}

bool DeepWoundsTriggerDummyEffect(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, SpellTarget *spTarget, int32 &amount)
{
    ASSERT(target->IsUnit());

    static SpellEntry *deepWounds = dbcSpell.LookupEntry(12721);
    if(deepWounds != NULL)
        caster->GetSpellInterface()->LaunchSpellFromSpell(deepWounds, caster, spell->GetSpellProto());
    return true;
}

bool DeepWoundsAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    float modifier = 0.16f;
    if(SpellEntry *parent = spell->GetSpellParent())
        modifier *= parent->RankNumber;

    amount = float2int32(((float)amount) * modifier);
    return true;
}

bool InterceptAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    amount += float2int32(12.f*((float)caster->CalculateAttackPower())/100.f);
    return true;
}

bool BloodthirstDummyEffect(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, SpellTarget *spTarget, int32 &amount)
{
    ASSERT(target->IsUnit());
    // Damage calc is spell effect index 0's amount as a percentage of attack power
    int32 damage = spTarget->effectAmount[SP_EFF_INDEX_0];
    damage = float2int32(((float)damage)*((float)caster->CalculateAttackPower())/100.f);
    // Bonus damage check
    damage = caster->GetSpellBonusDamage(castPtr<Unit>(target), spell->GetSpellProto(), effIndex, damage, false);
    spTarget->accumAmount = damage;

    // Trigger our healing dummy spell
    static SpellEntry *bloodthirstTrigger = dbcSpell.LookupEntry(23885);
    if(bloodthirstTrigger != NULL)
        caster->GetSpellInterface()->TriggerSpell(bloodthirstTrigger, caster);
    return true;
}

bool BloodthirstHealAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{   // 0.5% is equal to amount/200
    amount = float2int32(((float)caster->GetMaxHealth())/200.f);
    return true;
}

bool HeroicLeapAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    amount += float2int32(50.f*((float)caster->CalculateAttackPower())/100.f);
    return true;
}

bool HeroicThrowAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    amount += float2int32(50.f*((float)caster->CalculateAttackPower())/100.f);
    return true;
}

bool ShatteringThrowAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    amount += float2int32(50.f*((float)caster->CalculateAttackPower())/100.f);
    return true;
}

void SpellManager::_RegisterWarriorFixes()
{
    // Scale heroic strike on weapon damage
    _RegisterDummyEffect(78, SP_EFF_INDEX_1, HeroicStrikeDummyEffect);
    _RegisterAmountModifier(78, SP_EFF_INDEX_1, HeroicStrikeAmountModifier);

    // Need to trigger rage increase
    _RegisterDummyEffect(100, SP_EFF_INDEX_1, ChargeDummyEffect);
    _RegisterAmountModifier(34846, SP_EFF_INDEX_0, ChargeEnergizeAmountModifier);

    // Scale thunderclap damage based on AP
    _RegisterAmountModifier(6343, SP_EFF_INDEX_0, ThunderClapAmountModifier);

    // Scale Revenge Damage based on AP
    _RegisterAmountModifier(6572, SP_EFF_INDEX_0, RevengeAmountModifier);

    // Need to scale concussion blow to AP
    _RegisterDummyEffect(12809, SP_EFF_INDEX_2, ConcussionBlowDummyEffect);

    // Deep wounds triggers periodic damage, need to link to our triggered spell
    _RegisterDummyEffect(12162, SP_EFF_INDEX_1, DeepWoundsTriggerDummyEffect);
    _RegisterDummyEffect(12850, SP_EFF_INDEX_1, DeepWoundsTriggerDummyEffect);
    _RegisterDummyEffect(12868, SP_EFF_INDEX_1, DeepWoundsTriggerDummyEffect);
    _RegisterAmountModifier(12721, SP_EFF_INDEX_0, DeepWoundsAmountModifier);

    // Intercept attack power scaling
    _RegisterAmountModifier(20253, SP_EFF_INDEX_1, InterceptAmountModifier);

    // Blood thirst triggers a healing spell that regens every successful melee
    _RegisterDummyEffect(23881, SP_EFF_INDEX_1, BloodthirstDummyEffect);
    _RegisterAmountModifier(23880, SP_EFF_INDEX_0, BloodthirstHealAmountModifier);

    // Heroic Leap AP power scaling
    _RegisterAmountModifier(52174, SP_EFF_INDEX_0, HeroicLeapAmountModifier);
    // Heroic Throw AP power scaling
    _RegisterAmountModifier(57755, SP_EFF_INDEX_0, HeroicThrowAmountModifier);
    // Shattering Throw AP power scaling
    _RegisterAmountModifier(64382, SP_EFF_INDEX_1, ShatteringThrowAmountModifier);

}

class FuriousAttacksProcData : public SpellProcData
{
public:
    FuriousAttacksProcData(SpellEntry *sp) : SpellProcData(sp), procPerSecond(0.f)
    {   // Proc chance is 100%! This is incorrect so assume that there is a 100% chance of uptime and calculate a rate to use based on attack time
        procPerSecond = sp->procChance;
        if(SpellEntry *aura = dbcSpell.LookupEntry(sp->EffectTriggerSpell[0]))
            procPerSecond /= (((float)aura->Duration[0])/1000.f);
        // This works as a PPM(proc-per-minute) system
    }

    ~FuriousAttacksProcData() {}

    bool canProc(uint8 procIdentifier, Unit *caster, Unit *target, SpellEntry *sp, uint8 procType, uint16 procMods) { return false; }

    bool canProc(uint8 procIdentifier, Unit *caster, Unit *target, SpellEntry *sp, std::map<uint8, uint16> procPairs, uint8 weaponDamageType)
    {
        if(procIdentifier != PROCD_CASTER || procPairs.find(PROC_ON_STRIKE) == procPairs.end())
            return false;
        float attackTime = ((float)caster->GetBaseAttackTime(weaponDamageType))/1000.f;
        if(Rand(attackTime * procPerSecond))
            return true;
        return false;
    }

    bool endsDummycheck() { return true; }

private:
    float procPerSecond;
};

void SpellProcManager::_RegisterWarriorProcs()
{
    static SpellEntry *furiousAttacks = dbcSpell.LookupEntry(46910);
    if(furiousAttacks) RegisterProcData(furiousAttacks, new FuriousAttacksProcData(furiousAttacks));
}
