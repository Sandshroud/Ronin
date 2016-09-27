/***
* Demonstrike Core
*/

#include "StdAfx.h"

bool PaladinBlessingApplicator(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 amount)
{
    Group *grp = NULL;
    uint32 triggerSpell = (sp->Id == 19740 ? 79101 : 79062);
    if(caster->IsPlayer() && target->IsPlayer() && (grp = castPtr<Player>(caster)->GetGroup()) && grp->HasMember(castPtr<Player>(target)))
    {
        grp->Lock();
        GroupMembersSet::iterator itr;
        for(uint8 i = 0; i < grp->GetSubGroupCount(); i++)
            for(itr = grp->GetSubGroup(i)->GetGroupMembersBegin(); itr != grp->GetSubGroup(i)->GetGroupMembersEnd(); itr++)
                if((*itr)->m_loggedInPlayer && caster->IsInRangeSet((*itr)->m_loggedInPlayer))
                    caster->CastSpell((*itr)->m_loggedInPlayer, triggerSpell, true);
        grp->Unlock();
    } else if(target->IsUnit())
        caster->CastSpell(target, triggerSpell, true);
    return true;
}

void SpellManager::_RegisterPaladinFixes()
{
    // Register applicator for blessing of might and blessing of kings
    _RegisterDummyEffect(19740, SP_EFF_INDEX_0, PaladinBlessingApplicator);
    _RegisterDummyEffect(20217, SP_EFF_INDEX_0, PaladinBlessingApplicator);
}
