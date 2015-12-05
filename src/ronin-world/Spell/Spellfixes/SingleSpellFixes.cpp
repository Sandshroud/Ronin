/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void ApplySingleSpellFixes(SpellEntry *sp)
{
    if( sp->EquippedItemClass == 2 && sp->EquippedItemSubClassMask & (0x10|0x100|0x40000) ) // 4 + 8 + 262144 ( becomes item classes 2, 3 and 18 which correspond to bow, gun and crossbow respectively)
        sp->CustomAttributes[1] |= 0x10;

/*    if( IsTargetingStealthed( sp ) )
        sp->c_is_flags |= SPELL_FLAG_IS_TARGETINGSTEALTHED;
    if( IsCastedOnFriends(sp) )
        sp->c_is_flags |= SPELL_FLAG_CASTED_ON_FRIENDS;
    if( IsCastedOnEnemies(sp) )
        sp->c_is_flags |= SPELL_FLAG_CASTED_ON_ENEMIES;

    if( IsDamagingSpell( sp ) )
        sp->c_is_flags |= SPELL_FLAG_IS_DAMAGING;
    if( IsHealingSpell( sp ) )
        sp->c_is_flags |= SPELL_FLAG_IS_HEALING;
    if( IsFlyingSpell(sp) )
        sp->c_is_flags |= SPELL_FLAG_IS_FLYING;*/
}
