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
    _RegisterWarriorProcs();
    _RegisterPaladinProcs();
    _RegisterHunterProcs();
    _RegisterRogueProcs();
    _RegisterPriestProcs();
    _RegisterDeathKnightProcs();
    _RegisterShamanProcs();
    _RegisterMageProcs();
    _RegisterWarlockProcs();
    _RegisterDruidProcs();

    InitializeBaseSpellProcData(this);
}

void SpellProcManager::QuickProcessProcs(Unit *caster, uint8 procType, uint16 procMods)
{
    time_t current = UNIXTIME;
    uint32 msTime = RONIN_UTIL::ThreadTimer::getThreadTime();
    for(std::set<SpellProcData*>::iterator itr = caster->GetSpellInterface()->beginProcData(); itr != caster->GetSpellInterface()->endProcData(); ++itr)
        if((*itr)->canProc(PROCD_CASTER, caster, caster, NULL, procType, procMods) && caster->GetSpellInterface()->CanTriggerProc(*itr, NULL, current, msTime))
            caster->GetSpellInterface()->TriggerProc(*itr, NULL);
}

uint32 SpellProcManager::ProcessProcFlags(Unit *caster, Unit *target, std::map<uint8, uint16> procPairs, std::map<uint8, uint16> vProcPairs, SpellEntry *fromAbility, int32 &realDamage, uint32 &absoluteDamage, uint8 weaponDamageType)
{
    time_t current = UNIXTIME;
    uint32 msTime = RONIN_UTIL::ThreadTimer::getThreadTime();
    std::vector<SpellProcData*> casterProcs, victimProcs;
    // Process procs for our caster that effect our target
    for(std::set<SpellProcData*>::iterator itr = caster->GetSpellInterface()->beginProcData(); itr != caster->GetSpellInterface()->endProcData(); ++itr)
        if((*itr)->canProc(PROCD_CASTER, caster, target, fromAbility, procPairs, weaponDamageType) && caster->GetSpellInterface()->CanTriggerProc(*itr, fromAbility, current, msTime))
            casterProcs.push_back(*itr);
    // Now we process our targets affects: TODO should be procD victim but needs more testing
    for(std::set<SpellProcData*>::iterator itr = target->GetSpellInterface()->beginProcData(); itr != target->GetSpellInterface()->endProcData(); ++itr)
        if((*itr)->canProc(PROCD_VICTIM, target, caster, fromAbility, vProcPairs, weaponDamageType) && target->GetSpellInterface()->CanTriggerProc(*itr, fromAbility, current, msTime))
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

    if(apply)
        target->GetSpellInterface()->AddProcData(data);
    else target->GetSpellInterface()->RemoveProcData(data);
    return data->endsDummycheck();
}

void SpellProcManager::HandleAuraProcTriggerSpell(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply)
{
    SpellProcData *data = NULL;
    if((data = GetSpellProcData(spellProto)) == NULL)
        return;
    if(apply)
        target->GetSpellInterface()->AddProcData(data);
    else target->GetSpellInterface()->RemoveProcData(data);
}

void SpellProcManager::HandleAuraProcTriggerDamage(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply)
{
    SpellProcData *data = NULL;
    if((data = GetSpellProcData(spellProto)) == NULL)
        return;
    if(apply)
        target->GetSpellInterface()->AddProcData(data);
    else target->GetSpellInterface()->RemoveProcData(data);
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
    case PROC_ON_STRIKE:
        {
            if((expectedModifier&PROC_ON_STRIKE_CRITICAL_HIT) && (inputModifier&PROC_ON_STRIKE_CRITICAL_HIT) == 0)
                return false;

            if(expectedModifier & (PROC_ON_STRIKE_CAT_FORM|PROC_ON_STRIKE_BEAR_FORM))
            {
                if((expectedModifier&(PROC_ON_STRIKE_CAT_FORM|PROC_ON_STRIKE_BEAR_FORM)) == (PROC_ON_STRIKE_CAT_FORM|PROC_ON_STRIKE_BEAR_FORM))
                {
                    if((inputModifier&(PROC_ON_STRIKE_CAT_FORM|PROC_ON_STRIKE_BEAR_FORM)) == 0)
                        return false;
                }
                else if(expectedModifier & PROC_ON_STRIKE_CAT_FORM && (inputModifier & PROC_ON_STRIKE_CAT_FORM) == 0)
                    return false;
                else if(expectedModifier & PROC_ON_STRIKE_BEAR_FORM && (inputModifier & PROC_ON_STRIKE_BEAR_FORM) == 0)
                    return false;
            }

        }break;
    case PROC_ON_STRIKE_VICTIM:
        {

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

bool SpellProcManager::MatchTriggerSpell(SpellEntry *target, SpellEntry *trigger)
{
    if(target->NameHash == trigger->NameHash)
        return true;
    // Process through our indexes
    for(uint8 i = 0; i < 3; ++i)
        if(GetSpellProcFromSpellEntry(target, i) == trigger)
            return true;
    return false;
}

SpellEntry *SpellProcManager::GetSpellProcFromSpellEntry(SpellEntry *sp, uint8 index, Unit *target)
{
    SpellEntry *triggerSpell = NULL;
    switch(sp->EffectApplyAuraName[index])
    {
        {
        case SPELL_AURA_PROC_TRIGGER_SPELL:
        case SPELL_AURA_PROC_TRIGGER_DAMAGE:
        case SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE:
        case SPELL_AURA_PROC_TRIGGER_SPELL_2:
            {
                triggerSpell = dbcSpell.LookupEntry(sp->EffectTriggerSpell[index]);
            }break;
        }
    }

    SpellProcData * data = NULL;
    if((data = GetSpellProcData(sp)) && (triggerSpell == NULL || data->AlwaysOverrideProcSpell()))
        triggerSpell = data->GetProcSpellOverride(index, target);
    return triggerSpell;
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
    DefaultSpellProcData(SpellEntry *sp, uint8 index) : SpellProcData(sp) {}
    ~DefaultSpellProcData() {};

    // Can proc processing
    bool canProc(uint8 procIdentifier, Unit *caster, Unit *target, SpellEntry *sp, uint8 procType, uint16 procMods)
    {
        if(expectedTypes[procIdentifier].find(procType) != expectedTypes[procIdentifier].end())
            if(sSpellProcMgr.ProcDataMatches(GetSpellProto(), procType, procMods, expectedTypes[procIdentifier][procType]))
                return true;

        return false;
    }

    // Can proc processing
    bool canProc(uint8 procIdentifier, Unit *caster, Unit *target, SpellEntry *sp, std::map<uint8, uint16> procPairs, uint8 weaponDamageType)
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
        char desc[4096];
        strcpy(desc, GetSpellProto()->Description);
        uint32 len = (uint32)strlen(desc);
        for(uint i = 0; i < len; i++)
            desc[i] = tolower(desc[i]);

        uint32 procFlags = GetSpellProto()->procFlags;

        if((procFlags & PROC_FLAG_SUCCESSFUL_MELEE_HIT) || (strstr( desc,"chance on hit") || strstr( desc,"your auto attacks") || strstr( desc,"character strikes an enemy")
            || strstr( desc,"when it hits") || strstr( desc,"when successfully hit") || strstr( desc,"an enemy on hit")
            || strstr( desc,"when the caster is using melee attacks") || strstr( desc,"successful melee attack")
            || strstr( desc,"chance per hit") || strstr( desc,"you deal melee damage") || strstr( desc,"your melee attacks")
            || strstr( desc,"chance per attack") || strstr( desc,"damage with your sword") || strstr( desc,"on a successful hit")
            || strstr( desc,"takes damage") || strstr( desc,"when damaging an enemy in melee") || strstr( desc,"on a hit")
            || strstr( desc,"on successful melee or ranged attack") ||  strstr( desc,"when ranged or melee damage is dealt")
            || strstr( desc,"damaging melee attacks") || strstr( desc,"attackers when hit") || strstr( desc,"on a melee swing")
            || strstr( desc,"on melee or ranged attack") || strstr( desc,"chance on melee") ||  strstr( desc,"melee attacks has")
            || strstr( desc,"each melee attack a chance") || strstr( desc, "a chance to deal additional")
            || strstr( desc,"chance to get an extra attack") || strstr( desc,"giving each melee attack")
            || strstr( desc,"each strike has") || strstr( desc,"chance on hit") || strstr( desc,"with a melee weapon")
            || strstr( desc,"damage to melee attackers") || strstr( desc,"into flame, causing an additional")
            || strstr( desc,"damage on every attack") || strstr( desc,"your melee and ranged attacks")
            || strstr( desc, "gives your melee") || strstr( desc, "granting each melee")))
        {
            // Determine strike type
            uint8 procMod = PROC_ON_STRIKE_MELEE_HIT;
            if(strstr( desc, "critical"))
                procMod |= PROC_ON_STRIKE_CRITICAL_HIT;
            // Any additive flags
            if(strstr( desc, "cat form"))
                procMod |= PROC_ON_STRIKE_CAT_FORM;
            if(strstr( desc, "bear form"))
                procMod |= PROC_ON_STRIKE_BEAR_FORM;
            // Done processing strike checks
            expectedTypes[PROCD_CASTER].insert(std::make_pair(PROC_ON_STRIKE, procMod));
        }

        if((procFlags & PROC_FLAG_TAKEN_MELEE_HIT) || (strstr( desc,"attackers when hit") || strstr( desc,"strike you with a melee attack")
            || strstr( desc,"enemy strikes the caster") || strstr( desc,"strikes you with a melee attack")
            || strstr( desc,"enemy that strikes you in melee") || strstr( desc,"the next melee attack on the caster")
            || strstr( desc,"when struck in melee combat") ||  strstr( desc,"the next melee attack against the caster")
            || strstr( desc,"damage to attacker on hit") || strstr( desc,"melee and ranged attacks against you")
            || strstr( desc,"when struck in combat") || strstr( desc,"striking melee or ranged attackers")
            || strstr( desc,"strikes the caster") || strstr( desc,"each melee or ranged damage hit against the priest")
            || strstr( desc,"hit by a melee or ranged attack") || strstr( desc,"when struck in combat")
            || strstr( desc,"that strikes a party member") || strstr( desc,"when hit by a melee attack")
            || strstr( desc,"ranged and melee attacks to deal") || strstr( desc,"striking melee or ranged attackers")
            || strstr( desc,"damage to attackers when hit") || strstr( desc,"striking melee attackers")
            || strstr( desc,"striking melee attackers")))
        {
            // Determine strike type
            uint8 procMod = PROC_ON_STRIKEVICTIM_HIT;
            if(strstr( desc, "critical"))
                procMod |= PROC_ON_STRIKEVICTIM_CRITICAL;
            // Any additive flags
            if(strstr( desc, "dodge"))
                procMod |= PROC_ON_STRIKEVICTIM_DODGE;
            if(strstr( desc, "parry"))
                procMod |= PROC_ON_STRIKEVICTIM_PARRY;
            if(strstr( desc, "block"))
                procMod |= PROC_ON_STRIKEVICTIM_BLOCK;
            // Done processing strike checks
            expectedTypes[PROCD_VICTIM].insert(std::make_pair(PROC_ON_STRIKE_VICTIM, procMod));
        }

        /*if((procFlags & PROC_ON_CAST_SPELL) == 0 && (strstr( desc,"target casts a spell") || strstr( desc,"your harmful spells land")
            || strstr( desc, "any damage spell hits a target") || strstr( desc,"gives your finishing moves")
            || strstr( desc,"gives your sinister strike, backstab, gouge and shiv") || strstr( desc,"chance on spell hit")
            || strstr( desc,"your shadow word: pain, mind flay and vampiric touch spells also cause the target")
            || strstr( desc,"corruption, curse of agony, siphon life and seed of corruption spells also cause")
            || strstr( desc,"chance on spell hit") || strstr( desc,"your spell casts have") || strstr( desc,"chance on spell cast")
            || strstr( desc,"your spell casts have") || strstr( desc,"your Fire damage spell hits")
            || strstr( desc,"pain, mind flay and vampiric touch spells also cause")
            || strstr( desc,"next offensive ability") || strstr( desc,"on successful spellcast")
            || strstr( desc,"shadow damage spells have") || strstr( desc,"your next offensive ability")))
            procFlags |= PROC_ON_CAST_SPELL;

        if((procFlags & PROC_ON_ANY_DAMAGE_VICTIM) == 0 && (strstr( desc,"any damage caused") || strstr( desc,"when caster takes damage") || strstr( desc,"damage on hit")
            || strstr( desc,"after being hit by any damaging attack") || strstr( desc,"whenever the caster takes damage")
            || strstr( desc, "damaging attack is taken") || strstr( desc,"a spell, melee or ranged attack hits the caster")
            || strstr( desc,"whenever damage is dealt to you") || strstr( desc, "damage when hit")))
            procFlags |= PROC_ON_ANY_DAMAGE_VICTIM;

        if((procFlags & PROC_ON_RANGED_ATTACK_VICTIM) == 0 && (strstr( desc,"each melee or ranged damage hit against the priest")
            || strstr( desc,"melee and ranged attacks against you") || strstr( desc,"striking melee or ranged attackers")
            || strstr( desc,"hit by a melee or ranged attack") || strstr( desc,"striking melee or ranged attackers")
            || strstr( desc,"ranged and melee attacks to deal")))
            procFlags |= PROC_ON_RANGED_ATTACK_VICTIM;

        if((procFlags & PROC_ON_CRIT_ATTACK) == 0 && (strstr( desc,"landing a melee critical strike") || strstr( desc,"your critical strikes") || strstr( desc,"critical hit")
            || strstr( desc, "melee critical strike") || strstr( desc,"after dealing a critical strike")
            || strstr( desc,"dealing a critical strike from a weapon swing, spell, or ability")
            || strstr( desc,"after getting a critical strike")))
            procFlags |= PROC_ON_CRIT_ATTACK;

        if((procFlags & PROC_ON_RANGED_ATTACK) == 0 && (strstr( desc,"on successful melee or ranged attack") ||  strstr( desc,"when ranged or melee damage is dealt")
            || strstr( desc,"on melee or ranged attack") || strstr( desc,"damage on every attack")
            || strstr( desc,"your melee and ranged attacks") || strstr( desc,"whenever you deal ranged damage")))
            procFlags |= PROC_ON_RANGED_ATTACK;

        if((procFlags & PROC_ON_SPELL_HIT_VICTIM) == 0 && (strstr( desc,"any successful spell cast against the priest") || strstr( desc,"chance to reflect Fire spells")
            || strstr( desc, "struck by a Stun or Immobilize")))
            procFlags |= PROC_ON_SPELL_HIT_VICTIM;

        if((procFlags & PROC_ON_SPELL_CRIT_HIT_VICTIM) == 0 && (strstr( desc,"your spell criticals have") || strstr( desc, "getting a critical effect from")
            || strstr( desc,"spell criticals against you")))
            procFlags |= PROC_ON_SPELL_CRIT_HIT_VICTIM;

        if(strstr( desc,"dealing a critical strike from a weapon swing, spell, or ability")
            || strstr( desc,"your spell criticals have"))
            procFlags |= PROC_ON_SPELL_CRIT_HIT;

        if( strstr( desc,"hunter takes on the aspects of a hawk") || strstr( desc,"hunter takes on the aspects of a dragonhawk"))
        {
            if((procFlags & PROC_ON_RANGED_ATTACK) == 0)
                procFlags |= PROC_ON_RANGED_ATTACK;
            procFlags |= PROC_TARGET_SELF;
        }

        if((procFlags & PROC_ON_CRIT_HIT_VICTIM) == 0 && (strstr( desc,"victim of a critical strike") || strstr( desc,"after being struck by a melee or ranged critical hit")
            || strstr( desc, "victim of a melee or ranged critical strike") || strstr( desc,"victim of a critical melee strike")))
            procFlags |= PROC_ON_CRIT_HIT_VICTIM;

        if((procFlags & PROC_ON_RANGED_CRIT_ATTACK) == 0 && (strstr( desc,"your ranged criticals")))
            procFlags |= PROC_ON_RANGED_CRIT_ATTACK;

        if((procFlags & PROC_ON_GAIN_EXPIERIENCE) == 0 && (strstr( desc, "experience or honor")))
            procFlags |= PROC_ON_GAIN_EXPIERIENCE;

        if((procFlags & PROC_ON_SPELL_LAND_VICTIM) == 0 && (strstr( desc,"after being hit with a shadow or fire spell")))
            procFlags |= PROC_ON_SPELL_LAND_VICTIM;

        if((procFlags & PROC_ON_AUTO_SHOT_HIT) == 0 && (strstr( desc,"successful auto shot attacks")))
            procFlags |= PROC_ON_AUTO_SHOT_HIT;

        if( strstr( desc, "gives your"))
        {
            if(strstr( desc,"chance to daze the target") && (procFlags & PROC_ON_CAST_SPELL) == 0)
                procFlags |= PROC_ON_CAST_SPELL;
            else // We should find that specific spell (or group) on what we will trigger
                if((procFlags & PROC_ON_CAST_SPECIFIC_SPELL) == 0)
                    procFlags |= PROC_ON_CAST_SPECIFIC_SPELL;
        }

        if((procFlags & PROC_ON_CRIT_ATTACK) == 0 && (strstr( desc, "chance to add an additional combo") && strstr(desc, "critical")))
            procFlags |= PROC_ON_CRIT_ATTACK;
        else if((procFlags & PROC_ON_CAST_SPELL) == 0 && (strstr( desc, "chance to add an additional combo")))
            procFlags |= PROC_ON_CAST_SPELL;*/

        if(procFlags & PROC_FLAG_KILLED)
        {

        }

        if(procFlags & PROC_FLAG_KILL)
        {
            uint8 procMod = PROC_ON_KILL_MODIFIER_NONE;
            if(strlen(desc))
            {
                procMod |= PROC_ON_KILL_GRANTS_XP;
                // Prevent creature based procs from occuring when killing players
                if(strstr(desc, "killing creatures") || strstr(desc, "killing a creature") || (strstr(desc, "experience") && !strstr(desc, "honor")))
                    procMod |= PROC_ON_KILL_CREATURE;
                // Not used, but implemented
                if(strstr(desc, "killing players") || strstr(desc, "killing a player") || (strstr(desc, "honor") && !strstr(desc, "experience")))
                    procMod |= PROC_ON_KILL_PLAYER;
                // Remove XP requirement if we're not requiring experience or honor
                if(!(strstr(desc, "experience") || strstr(desc, "honor")))
                    procMod &= ~PROC_ON_KILL_GRANTS_XP;
            }

            expectedTypes[PROCD_CASTER].insert(std::make_pair(PROC_ON_KILL, procMod));
        }
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
            if(manager->GetSpellProcData(sp))
                continue;

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
                            manager->RegisterProcData(sp, procData = new DefaultSpellProcData(sp, j));
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
