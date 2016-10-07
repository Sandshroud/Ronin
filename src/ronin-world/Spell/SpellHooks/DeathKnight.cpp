/***
* Demonstrike Core
*/

#include "StdAfx.h"

bool DeathKnightDeathCoilTrigger(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount)
{
    Unit *unitTarget, *unitCaster;
    if((unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL) && (unitCaster = caster->IsUnit() ? castPtr<Unit>(caster) : NULL))
    {
        if(sFactionSystem.CanEitherUnitAttack(unitTarget, unitCaster))
            unitCaster->CastSpell(unitTarget, 47632, true);
        else unitCaster->CastSpell(unitTarget, 47633, true);
    }

    return true;
}

bool DeathKnightFrostAmountModifier(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount)
{
    if(Unit *unitCaster = caster->IsUnit() ? castPtr<Unit>(caster) : NULL)
    {
        switch(sp->Id)
        {
            // Icy Touch has an AP modifier of 0.2
        case 45477: amount += float2int32(((float)unitCaster->GetAttackPower())*0.2f); break;
        }
    }
    return true;
}

bool DeathKnightUnholyAmountModifier(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount)
{
    if(Unit *unitCaster = caster->IsUnit() ? castPtr<Unit>(caster) : NULL)
    {
        switch(sp->Id)
        {
            // Death Coil has an AP modifier of 0.2
        case 47632: amount += float2int32(((float)unitCaster->GetAttackPower())*0.288f); break;
        }
    }
    return true;
}

void SpellManager::_RegisterDeathKnightFixes()
{
    // Modify the amount of damage our icy touch does
    _RegisterDummyEffect(47541, SP_EFF_INDEX_0, DeathKnightDeathCoilTrigger);

    // Modify the amount of damage our frost spells do
    _RegisterAmountModifier(45477, SP_EFF_INDEX_0, DeathKnightFrostAmountModifier); // Icy Touch

    // Modify the amount of damage our Unholy spells do
    _RegisterAmountModifier(47632, SP_EFF_INDEX_0, DeathKnightUnholyAmountModifier); // Death Coil
}
