/***
* Demonstrike Core
*/

#include "StdAfx.h"

bool MageArcaneBrillianceApplicator(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, SpellTarget *spTarget, int32 &amount)
{
    Group *grp = NULL;
    SpellEntry *triggerSpell = dbcSpell.LookupEntry(79058);
    if(triggerSpell && caster->IsPlayer() && target->IsPlayer() && (grp = castPtr<Player>(caster)->GetGroup()) && grp->HasMember(castPtr<Player>(target)))
    {
        grp->Lock();
        GroupMembersSet::iterator itr;
        for(uint8 i = 0; i < grp->GetSubGroupCount(); i++)
            for(itr = grp->GetSubGroup(i)->GetGroupMembersBegin(); itr != grp->GetSubGroup(i)->GetGroupMembersEnd(); itr++)
                if((*itr)->m_loggedInPlayer && ((caster == (*itr)->m_loggedInPlayer) || (*itr)->m_loggedInPlayer->IsVisible(caster)))
                    caster->GetSpellInterface()->TriggerSpell(triggerSpell, (*itr)->m_loggedInPlayer);
        grp->Unlock();
    } else if(triggerSpell && target->IsUnit())
        caster->GetSpellInterface()->TriggerSpell(triggerSpell, castPtr<Unit>(target));
    return true;
}

bool MageConjureRefreshmentDummyEffect(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, SpellTarget *spTarget, int32 &amount)
{
    static uint8 requiredLevels[7] = { 33, 44, 54, 64, 74, 80, 85 };
    static SpellEntry* rewardRefreshments[7] =
    {
        dbcSpell.LookupEntry(92739), // Mana cookie
        dbcSpell.LookupEntry(92799), // Mana Brownie
        dbcSpell.LookupEntry(92802), // Mana Cupcake
        dbcSpell.LookupEntry(92805), // Mana Lollipop
        dbcSpell.LookupEntry(74625), // Mana Pie
        dbcSpell.LookupEntry(92822), // Mana Strudel
        dbcSpell.LookupEntry(92727)  // Mana Cake
    };

    SpellEntry *sp = NULL;
    uint32 level = caster->getLevel();
    for(uint8 i = 0; i < 7; ++i)
        if(level >= requiredLevels[i] && rewardRefreshments[i])
            sp = rewardRefreshments[i];

    if(sp != NULL)
        caster->GetSpellInterface()->TriggerSpell(sp, caster);
    return true;
}

void SpellManager::_RegisterMageFixes()
{
    // Arcane Brilliance applicator call
    _RegisterDummyEffect(1459, SP_EFF_INDEX_0, MageArcaneBrillianceApplicator);

    // Trigger sub spells for conjuring refreshments based on level
    _RegisterDummyEffect(42955, SP_EFF_INDEX_0, MageConjureRefreshmentDummyEffect);
}

void SpellProcManager::_RegisterMageProcs()
{

}
