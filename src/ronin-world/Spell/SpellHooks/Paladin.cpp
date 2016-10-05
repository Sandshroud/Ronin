/***
* Demonstrike Core
*/

#include "StdAfx.h"

bool PaladinBlessingApplicator(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount)
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

bool PaladinJudgementDummyHandler(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount)
{
    Unit *unitCaster = caster->IsUnit() ? castPtr<Unit>(caster) : NULL, *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitCaster && unitTarget)
    {
        SpellEntry *sealTrigger = NULL;
        // Insight and justice trigger the same seal release
        if(unitCaster->m_AuraInterface.FindActiveAuraWithNameHash(SPELL_HASH_SEAL_OF_INSIGHT) || unitCaster->m_AuraInterface.FindActiveAuraWithNameHash(SPELL_HASH_SEAL_OF_JUSTICE))
            sealTrigger = dbcSpell.LookupEntry(54158);
        else if(unitCaster->m_AuraInterface.FindActiveAuraWithNameHash(SPELL_HASH_SEAL_OF_RIGHTEOUSNESS))
            sealTrigger = dbcSpell.LookupEntry(20187);
        else if(unitCaster->m_AuraInterface.FindActiveAuraWithNameHash(SPELL_HASH_SEAL_OF_TRUTH))
            sealTrigger = dbcSpell.LookupEntry(31804);

        if(sealTrigger && sealTrigger != sp)
            unitCaster->CastSpell(unitTarget, sealTrigger, true);
    }
    return true;
}

bool PaladinJudgementTriggerAmountModifier(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount)
{
    if(Unit *unitCaster = caster->IsUnit() ? castPtr<Unit>(caster) : NULL)
    {
        switch(sp->Id)
        {
            // Righteousness is a 0.32 and 0.2 coefficient
        case 20187: amount += float2int32(((float)unitCaster->GetDamageDoneMod(SCHOOL_HOLY))*0.32f + ((float)unitCaster->GetAttackPower())*0.2f); break;
            // Truth is a 0.22 and 0.14 coefficient
        case 31804: amount += float2int32(((float)unitCaster->GetDamageDoneMod(SCHOOL_HOLY))*0.22f + ((float)unitCaster->GetAttackPower())*0.14f); break;
            // Raw judgement is a 0.25 and 0.16 coefficient
        case 54158: amount += float2int32(((float)unitCaster->GetDamageDoneMod(SCHOOL_HOLY))*0.25f + ((float)unitCaster->GetAttackPower())*0.16f); break;
        }
    }

    Aura *censure = NULL;
    if(sp->Id == 31804 && target->IsUnit() && (censure = castPtr<Unit>(target)->m_AuraInterface.FindActiveAuraWithNameHash(SPELL_HASH_CENSURE)))
        amount += ((float)amount) * 0.2f * censure->getStackSize();
    return true;
}

bool PaladinExorcismAmountModifier(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount)
{
    // Exorcism gets a raw attack power increase of 0.315-0.325%, not sure yet which
    if(Unit *unitCaster = caster->IsUnit() ? castPtr<Unit>(caster) : NULL)
        amount += float2int32(unitCaster->GetAttackPower() * 0.325f);
    return true;
}

bool PaladinTemplarsVerdictAmountModifier(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount)
{
    if(Unit *unitCaster = caster->IsUnit() ? castPtr<Unit>(caster) : NULL)
    {
        switch(unitCaster->GetPower(POWER_TYPE_HOLY_POWER))
        {   // Add wpn pct based on left over holy power after we remove 1
        case 1: amount += 60; break;
        case 2: amount += 205; break;
        default:break;
        }
        unitCaster->SetPower(POWER_TYPE_HOLY_POWER, 0);
    }
    return true;
}

void SpellManager::_RegisterPaladinFixes()
{
    // Register applicator for blessing of might and blessing of kings
    _RegisterDummyEffect(19740, SP_EFF_INDEX_0, PaladinBlessingApplicator);
    _RegisterDummyEffect(20217, SP_EFF_INDEX_0, PaladinBlessingApplicator);

    // Register the dummy handler for casting judgement spells
    _RegisterDummyEffect(20271, SP_EFF_INDEX_0, PaladinJudgementDummyHandler);

    // Register the damage modifier for judgement triggers
    _RegisterAmountModifier(20187, SP_EFF_INDEX_0, PaladinJudgementTriggerAmountModifier);
    _RegisterAmountModifier(31804, SP_EFF_INDEX_0, PaladinJudgementTriggerAmountModifier);
    _RegisterAmountModifier(54158, SP_EFF_INDEX_0, PaladinJudgementTriggerAmountModifier);

    // Register the damage modifier based off attack power for exorcism
    _RegisterAmountModifier(879, SP_EFF_INDEX_0, PaladinExorcismAmountModifier);

    // Register the damage increase for templar's verdict based off holy power available
    _RegisterAmountModifier(85256, SP_EFF_INDEX_0, PaladinTemplarsVerdictAmountModifier);
}
