/***
* Demonstrike Core
*/

#include <stdafx.h>

initialiseSingleton( SpellProcManager );

SpellProcManager::SpellProcManager()
{

}

SpellProcManager::~SpellProcManager()
{

}

void InitializeBaseSpellProcData(SpellProcManager *manager);

void SpellProcManager::InitProcData()
{
    InitializeBaseSpellProcData(this);

}

void SpellProcManager::QuickProcessProcs(Unit *caster, uint8 procType, uint16 procMods)
{
    for(std::set<SpellProcData*>::iterator itr = caster->GetSpellInterface()->beginProcData(); itr != caster->GetSpellInterface()->endProcData(); ++itr)
        if((*itr)->canProc(PROCD_CASTER, caster, NULL, procType, procMods) && caster->GetSpellInterface()->CanTriggerProc(*itr, 0, 0))
            caster->GetSpellInterface()->TriggerProc(*itr, NULL);
}

uint32 SpellProcManager::ProcessProcFlags(Unit *caster, Unit *target, std::map<uint8, uint16> procPairs, std::map<uint8, uint16> vProcPairs, SpellEntry *fromAbility, int32 &realDamage, uint32 &absoluteDamage, uint8 weaponDamageType)
{
    std::vector<SpellProcData*> casterProcs, victimProcs;
    for(std::set<SpellProcData*>::iterator itr = caster->GetSpellInterface()->beginProcData(); itr != caster->GetSpellInterface()->endProcData(); ++itr)
        if((*itr)->canProc(PROCD_CASTER, caster, fromAbility, procPairs) && caster->GetSpellInterface()->CanTriggerProc(*itr, 0, 0))
            casterProcs.push_back(*itr);
    for(std::set<SpellProcData*>::iterator itr = target->GetSpellInterface()->beginProcData(); itr != target->GetSpellInterface()->endProcData(); ++itr)
        if((*itr)->canProc(PROCD_CASTER, target, fromAbility, vProcPairs) && target->GetSpellInterface()->CanTriggerProc(*itr, 0, 0))
            victimProcs.push_back(*itr);

    for(std::vector<SpellProcData*>::iterator itr = casterProcs.begin(); itr != casterProcs.end(); ++itr)
        caster->GetSpellInterface()->TriggerProc(*itr, target);
    for(std::vector<SpellProcData*>::iterator itr = victimProcs.begin(); itr != victimProcs.end(); ++itr)
        target->GetSpellInterface()->TriggerProc(*itr, caster);

    casterProcs.clear();
    victimProcs.clear();
    return 0;
}

bool SpellProcManager::HandleAuraProcTriggerDummy(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply)
{
    SpellProcData *data = NULL;
    if((data = GetSpellProcData(spellProto)) == NULL)
        return false;

    target->GetSpellInterface()->AddProcData(data);
    return data->endsDummycheck();
}

void SpellProcManager::HandleAuraProcTriggerSpell(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply)
{
    SpellProcData *data = NULL;
    if((data = GetSpellProcData(spellProto)) == NULL)
        return;
    target->GetSpellInterface()->AddProcData(data);
}

void SpellProcManager::HandleAuraProcTriggerDamage(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply)
{
    SpellProcData *data = NULL;
    if((data = GetSpellProcData(spellProto)) == NULL)
        return;
    target->GetSpellInterface()->AddProcData(data);
}

void SpellProcManager::RegisterProcData(SpellEntry *sp, SpellProcData *procData)
{
    if(m_spellProcData.find(sp->Id) != m_spellProcData.end())
        return;
    m_spellProcData.insert(std::make_pair(sp->Id, procData));
}

bool SpellProcManager::ProcDataMatches(SpellEntry *sp, uint8 inputType, uint8 inputModifier, uint16 expectedModifier)
{
    switch(inputType)
    {
    case PROC_ON_KILL:
        {   // Proc on kills have modifiers for requiring XP
            if(expectedModifier & PROC_ON_KILL_GRANTS_XP && (inputModifier & PROC_ON_KILL_GRANTS_XP) == 0)
                return false;
        }break;
    }

    return true;
}

SpellProcData *SpellProcManager::GetSpellProcData(SpellEntry *sp)
{
    std::map<uint32, SpellProcData*>::iterator itr;
    if((itr = m_spellProcData.find(sp->Id)) != m_spellProcData.end())
        return itr->second;
    return NULL;
}

enum DBCProcFlags
{
    PROC_FLAG_NONE                          = 0x00000000,

    PROC_FLAG_KILLED                        = 0x00000001,   // 00 Killed by aggressor
    PROC_FLAG_KILL                          = 0x00000002,   // 01 Kill target (in most cases need XP/Honor reward, see Unit::IsTriggeredAtSpellProcEvent for additinoal check)

    PROC_FLAG_SUCCESSFUL_MELEE_HIT          = 0x00000004,   // 02 Successful melee auto attack
    PROC_FLAG_TAKEN_MELEE_HIT               = 0x00000008,   // 03 Taken damage from melee auto attack hit

    PROC_FLAG_SUCCESSFUL_MELEE_SPELL_HIT    = 0x00000010,   // 04 Successful attack by Spell that use melee weapon
    PROC_FLAG_TAKEN_MELEE_SPELL_HIT         = 0x00000020,   // 05 Taken damage by Spell that use melee weapon

    PROC_FLAG_SUCCESSFUL_RANGED_HIT         = 0x00000040,   // 06 Successful Ranged auto attack
    PROC_FLAG_TAKEN_RANGED_HIT              = 0x00000080,   // 07 Taken damage from ranged auto attack

    PROC_FLAG_SUCCESSFUL_RANGED_SPELL_HIT   = 0x00000100,   // 08 Successful Ranged attack by Spell that use ranged weapon
    PROC_FLAG_TAKEN_RANGED_SPELL_HIT        = 0x00000200,   // 09 Taken damage by Spell that use ranged weapon

    PROC_FLAG_SUCCESSFUL_POSITIVE_AOE_HIT   = 0x00000400,   // 10 Successful AoE (not 100% sure, unused)
    PROC_FLAG_TAKEN_POSITIVE_AOE            = 0x00000800,   // 11 Taken AoE      (not 100% sure, unused)

    PROC_FLAG_SUCCESSFUL_AOE_SPELL_HIT      = 0x00001000,   // 12 Successful AoE damage spell hit (not 100% sure, unused)
    PROC_FLAG_TAKEN_AOE_SPELL_HIT           = 0x00002000,   // 13 Taken AoE damage spell hit      (not 100% sure, unused)

    PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL     = 0x00004000,   // 14 Successful cast positive spell (by default only on healing)
    PROC_FLAG_TAKEN_POSITIVE_SPELL          = 0x00008000,   // 15 Taken positive spell hit (by default only on healing)

    PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT = 0x00010000,   // 16 Successful negative spell cast (by default only on damage)
    PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT      = 0x00020000,   // 17 Taken negative spell (by default only on damage)

    PROC_FLAG_ON_DO_PERIODIC                = 0x00040000,   // 18 Successful do periodic (damage / healing, determined by PROC_EX_PERIODIC_POSITIVE or negative if no procEx)
    PROC_FLAG_ON_TAKE_PERIODIC              = 0x00080000,   // 19 Taken spell periodic (damage / healing, determined by PROC_EX_PERIODIC_POSITIVE or negative if no procEx)

    PROC_FLAG_TAKEN_ANY_DAMAGE              = 0x00100000,   // 20 Taken any damage
    PROC_FLAG_ON_TRAP_ACTIVATION            = 0x00200000,   // 21 On trap activation

    PROC_FLAG_TAKEN_OFFHAND_HIT             = 0x00400000,   // 22 Taken off-hand melee attacks(not used)
    PROC_FLAG_SUCCESSFUL_OFFHAND_HIT        = 0x00800000,   // 23 Successful off-hand melee attacks

    PROC_FLAG_ON_DEATH                      = 0x01000000,   // 24 On death
    PROC_FLAG_ON_JUMP                       = 0x02000000,   // 25 On jump
};

class DefaultSpellProcData : public SpellProcData
{
public:
    DefaultSpellProcData(SpellEntry *sp) : SpellProcData(sp) {}
    ~DefaultSpellProcData() {};

    // Can proc processing
    bool canProc(uint8 procIdentifier, Unit *target, SpellEntry *sp, uint8 procType, uint16 procMods)
    {
        if(expectedTypes[procIdentifier].find(procType) != expectedTypes[procIdentifier].end())
            if(sSpellProcMgr.ProcDataMatches(GetSpellProto(), procType, procMods, expectedTypes[procIdentifier][procType]))
                return true;

        return false;
    }

    // Can proc processing
    bool canProc(uint8 procIdentifier, Unit *target, SpellEntry *sp, std::map<uint8, uint16> procPairs)
    {
        for(auto itr = expectedTypes[procIdentifier].begin(); itr != expectedTypes[procIdentifier].end(); ++itr)
            if(procPairs.find(itr->first) != procPairs.end() && sSpellProcMgr.ProcDataMatches(GetSpellProto(), itr->first, procPairs[itr->first], itr->second))
                return true;
        return false;
    }

    void AddModifier(uint8 index)
    {

    }

    void Initialize()
    {
        if(GetSpellProto()->procFlags & PROC_FLAG_KILL)
            expectedTypes[PROCD_CASTER].insert(std::make_pair(PROC_ON_KILL, PROC_ON_KILL_GRANTS_XP));

    }

private:
    std::map<uint8, uint16> expectedTypes[PROCD_COUNT];
};

void InitializeBaseSpellProcData(SpellProcManager *manager)
{
    for(uint32 i = 0; i < dbcSpell.GetNumRows(); ++i)
    {
        if(SpellEntry *sp = dbcSpell.LookupRow(i))
        {
            DefaultSpellProcData *procData = NULL;
            for(uint8 j = 0; j < 3; ++j)
            {
                if(procData != NULL)
                    break;

                switch(sp->EffectApplyAuraName[j])
                {
                case SPELL_AURA_PROC_TRIGGER_SPELL:
                case SPELL_AURA_PROC_TRIGGER_DAMAGE:
                case SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE:
                case SPELL_AURA_PROC_TRIGGER_SPELL_2:
                    {
                        if(procData == NULL)
                            manager->RegisterProcData(sp, procData = new DefaultSpellProcData(sp));
                        else procData->AddModifier(j);
                    }break;
                }
            }

            if(procData == NULL)
                continue;
            procData->Initialize();
        }
    }
}
