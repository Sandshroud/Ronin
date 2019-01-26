/***
* Demonstrike Core
*/

#include "StdAfx.h"

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

void SpellManager::_RegisterWarriorFixes()
{
    _RegisterDummyEffect(23881, SP_EFF_INDEX_1, BloodthirstDummyEffect);
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
