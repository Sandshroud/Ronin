/***
* Demonstrike Core
*/

#include "StdAfx.h"

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

void SpellManager::_RegisterDeathKnightFixes()
{
    _RegisterAmountModifier(45477, SP_EFF_INDEX_0, DeathKnightFrostAmountModifier);
}
