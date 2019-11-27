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

bool MageIgniteDOTDamageCalculation(Unit *target, Aura *aur, SpellEntry *proto, uint32 modIndex, Modifier *modList, bool apply)
{
    ASSERT(modList);
    Unit *caster = aur->GetUnitCaster();
    if(apply == true && caster)
    {
        SpellEntry *sp;
        uint8 index = 0xFF;
        int32 effectAmount = 10;
        if(Aura *aur = caster->m_AuraInterface.FindPassiveAuraByNamehash(SPELL_HASH_IGNITE))
            effectAmount = aur->GetMod(SP_EFF_INDEX_0)->m_amount;

        if((sp = aur->GetParentSpell()) && sp->GetEffectIndex(SPELL_EFFECT_SCHOOL_DAMAGE, index))
            modList[0].m_amount = (caster->GetSpellBonusDamage(target, sp, index, sp->CalculateSpellPoints(index, aur->GetAuraLevel()), false) * effectAmount)/200;
    }
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

    // Mage Ignite damage percentage
    _RegisterDummyAuraEffect(12654, SP_EFF_INDEX_1, MageIgniteDOTDamageCalculation);

    // Trigger sub spells for conjuring refreshments based on level
    _RegisterDummyEffect(42955, SP_EFF_INDEX_0, MageConjureRefreshmentDummyEffect);
}

class MageIgniteProcData : public SpellProcData
{
public:
    MageIgniteProcData(SpellEntry *sp) : SpellProcData(sp) { }
    ~MageIgniteProcData() {}

    bool canProc(uint8 procIdentifier, Unit *caster, Unit *target, SpellEntry *sp, uint8 procType, uint16 procMods) { return false; }

    bool canProc(uint8 procIdentifier, Unit *caster, Unit *target, SpellEntry *sp, std::map<uint8, uint16> procPairs, uint8 weaponDamageType)
    {
        if(procIdentifier != PROCD_CASTER)
            return false;
        if(procPairs.find(PROC_ON_SPELL_LAND) == procPairs.end())
            return false;
        if((procPairs[PROC_ON_SPELL_LAND] & PROC_ON_SPELL_LAND_CRITICAL) == 0)
            return false;
        if(sp == NULL || sp->School != SCHOOL_FIRE)
           return false;
        return true;
    }

    bool endsDummycheck() { return true; }

    SpellEntry *GetProcSpellOverride(uint8 triggerIndex, Unit *target)
    {
        return dbcSpell.LookupEntry(12654);
    }

    bool AlwaysOverrideProcSpell() { return true; }
};

void SpellProcManager::_RegisterMageProcs()
{
    static SpellEntry *igniteRank1 = dbcSpell.LookupEntry(11119);
    if(igniteRank1) RegisterProcData(igniteRank1, new MageIgniteProcData(igniteRank1));
    static SpellEntry *igniteRank2 = dbcSpell.LookupEntry(11120);
    if(igniteRank2) RegisterProcData(igniteRank2, new MageIgniteProcData(igniteRank2));
    static SpellEntry *igniteRank3 = dbcSpell.LookupEntry(12846);
    if(igniteRank3) RegisterProcData(igniteRank3, new MageIgniteProcData(igniteRank3));
}
