/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void ApplySingleSpellFixes(SpellEntry *sp)
{
    if( sp->EquippedItemClass == 2 && sp->EquippedItemSubClass & (0x10|0x100|0x40000) ) // 4 + 8 + 262144 ( becomes item classes 2, 3 and 18 which correspond to bow, gun and crossbow respectively)
        sp->is_ranged_spell = true;

    //////////////////////////////////////////////////////////////////
    //AREA AURA TARGETS - END
    //////////////////////////////////////////////////////////////////
    if( IsDamagingSpell( sp ) )
        sp->c_is_flags |= SPELL_FLAG_IS_DAMAGING;
    if( IsHealingSpell( sp ) )
        sp->c_is_flags |= SPELL_FLAG_IS_HEALING;
    if( IsTargetingStealthed( sp ) )
        sp->c_is_flags |= SPELL_FLAG_IS_TARGETINGSTEALTHED;
    if( IsFlyingSpell(sp) )
        sp->c_is_flags |= SPELL_FLAG_IS_FLYING;
    if( IsCastedOnFriends(sp) )
        sp->c_is_flags |= SPELL_FLAG_CASTED_ON_FRIENDS;
    if( IsCastedOnEnemies(sp) )
        sp->c_is_flags |= SPELL_FLAG_CASTED_ON_ENEMIES;
    if(sp->Flags3 & FLAGS3_CANT_CRIT) //I can haz hacky? :O
        sp->spell_can_crit = false;
}
