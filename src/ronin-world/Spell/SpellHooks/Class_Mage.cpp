/***
* Demonstrike Core
*/

#include "StdAfx.h"

bool MageArcaneBrillianceApplicator(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
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

void SpellManager::_RegisterMageFixes()
{
    // Arcane Brilliance applicator call
    _RegisterDummyEffect(1459, SP_EFF_INDEX_0, MageArcaneBrillianceApplicator);
}
