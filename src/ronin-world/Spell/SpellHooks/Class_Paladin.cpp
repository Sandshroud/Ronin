/***
* Demonstrike Core
*/

#include "StdAfx.h"

bool PaladinBlessingApplicator(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, SpellTarget *spTarget, int32 &amount)
{
    Group *grp = NULL;
    SpellEntry *triggerSpell = dbcSpell.LookupEntry(spell->GetSpellProto()->Id == 19740 ? 79101 : 79062);
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

bool PaladinJudgementDummyHandler(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, SpellTarget *spTarget, int32 &amount)
{
    SpellEntry *sp = spell->GetSpellProto();
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
            unitCaster->GetSpellInterface()->TriggerSpell(sealTrigger, unitTarget);
    }
    return true;
}

bool PaladinJudgementTriggerAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    SpellEntry *sp = spell->GetSpellProto();
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

bool PaladinExorcismAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    // Exorcism gets a raw attack power increase of 0.315-0.325%, not sure yet which
    if(Unit *unitCaster = caster->IsUnit() ? castPtr<Unit>(caster) : NULL)
        amount += float2int32(unitCaster->GetAttackPower() * 0.325f);
    return true;
}

bool PaladinTemplarsVerdictAmountModifier(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
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

class SealOfTruthProcData : public SpellProcData
{
public:
    SealOfTruthProcData(SpellEntry *sp) : SpellProcData(sp) { }
    ~SealOfTruthProcData() {}

    bool canProc(uint8 procIdentifier, Unit *caster, Unit *target, SpellEntry *sp, uint8 procType, uint16 procMods) { return false; }

    bool canProc(uint8 procIdentifier, Unit *caster, Unit *target, SpellEntry *sp, std::map<uint8, uint16> procPairs, uint8 weaponDamageType)
    {
        if(sp != NULL && sp->isSpellAreaOfEffect())
           return false;
        if(procIdentifier != PROCD_CASTER)
            return false;
        if(procPairs.find(PROC_ON_STRIKE) == procPairs.end() && procPairs.find(PROC_ON_SPELL_LAND) == procPairs.end())
            return false;
        return true;
    }

    bool endsDummycheck() { return true; }

    SpellEntry *GetProcSpellOverride(uint8 triggerIndex, Unit *target)
    {   // Seal of Truth will damage target when they have 5 stacks of censure
        if(triggerIndex == 0 && target && target->m_AuraInterface.checkStackSize(31803, 5))
            return dbcSpell.LookupEntry(42463);
        else if(triggerIndex == 2) // Trigger our censure stack second
            return dbcSpell.LookupEntry(31803);
        return NULL;
    }

    bool AlwaysOverrideProcSpell() { return true; }
};

class JudgementsOfTheBoldProcData : public SpellProcData
{
public:
    JudgementsOfTheBoldProcData(SpellEntry* sp) : SpellProcData(sp) { }
    ~JudgementsOfTheBoldProcData() {}

    bool canProc(uint8 procIdentifier, Unit* caster, Unit* target, SpellEntry* sp, uint8 procType, uint16 procMods) { return false; }

    bool canProc(uint8 procIdentifier, Unit* caster, Unit* target, SpellEntry* sp, std::map<uint8, uint16> procPairs, uint8 weaponDamageType)
    {
        if (sp != NULL && (sp->NameHash == SPELL_HASH_JUDGEMENT || sp->NameHash == SPELL_HASH_JUDGEMENT_OF_TRUTH || sp->NameHash == SPELL_HASH_JUDGEMENT_OF_RIGHTEOUSNESS))
            return true;
        return false;
    }

    bool endsDummycheck() { return true; }

    SpellEntry* GetProcSpellOverride(uint8 triggerIndex, Unit* target) { return dbcSpell.LookupEntry(89906); }
    bool AlwaysOverrideProcSpell() { return true; }
};

void SpellProcManager::_RegisterPaladinProcs()
{
    //
    static SpellEntry *sealOfTruth = dbcSpell.LookupEntry(31801);
    if(sealOfTruth) RegisterProcData(sealOfTruth, new SealOfTruthProcData(sealOfTruth));

    //
    static SpellEntry *judgementsOfTheBold = dbcSpell.LookupEntry(89901);
    if(judgementsOfTheBold) RegisterProcData(judgementsOfTheBold, new JudgementsOfTheBoldProcData(judgementsOfTheBold));
}
