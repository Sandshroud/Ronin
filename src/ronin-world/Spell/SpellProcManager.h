/***
* Demonstrike Core
*/

#pragma once

enum ProcDataStackIdentifier
{
    PROCD_CASTER,
    PROCD_VICTIM,
    PROCD_COUNT
};

class SpellProcData
{
public:
    SpellProcData(SpellEntry *sp) : spellProto(sp) {}
    ~SpellProcData() {}

    SpellEntry *GetSpellProto() { return spellProto; }

    virtual bool canProc(uint8 procIdentifier, Unit *caster, Unit *target, SpellEntry *sp, uint8 procType, uint16 procMods) = 0;
    virtual bool canProc(uint8 procIdentifier, Unit *caster, Unit *target, SpellEntry *sp, std::map<uint8, uint16> procPairs, uint8 weaponDamageType) = 0;
    virtual bool endsDummycheck() { return false; }

private:
    SpellEntry *spellProto;
};

enum SpellProcType : uint8
{
    PROC_ON_DEATH                           = 1,
    PROC_ON_KILL                            = 2,
    // Use weapon type to determine melee/offhand or ranged
    PROC_ON_STRIKE                          = 3,
    PROC_ON_STRIKE_VICTIM                   = 4,
    // Spell hit
    PROC_ON_SPELL_LAND                      = 5,
    PROC_ON_SPELL_LAND_VICTIM               = 6,
    PROC_ON_SPELL_REFLECT                   = 7,
    // 
    // Periodic Tick
    PROC_ON_PERIODIC_TICK                   = 10,
    PROC_ON_PERIODIC_TICK_VICTIM            = 11,
};

enum SpellProcMods : uint16
{
    // On kill modifiers
    PROC_ON_KILL_MODIFIER_NONE              = 0x0000,
    PROC_ON_KILL_PLAYER                     = 0x0001,
    PROC_ON_KILL_CREATURE                   = 0x0002,
    PROC_ON_KILL_GRANTS_XP                  = 0x0004,
    // Strike modifiers
    PROC_ON_STRIKE_NONE                     = 0x0000,
    PROC_ON_STRIKE_MELEE_HIT                = 0x0001,
    PROC_ON_STRIKE_CRITICAL_HIT             = 0x0002,
    PROC_ON_STRIKE_MELEE_SPELL_HIT          = 0x0004,
    PROC_ON_STRIKE_BLOCKED                  = 0x0008,
    PROC_ON_STRIKE_MISS                     = 0x0010,
    PROC_ON_STRIKE_CAT_FORM                 = 0x0040,
    PROC_ON_STRIKE_BEAR_FORM                = 0x0080,
    // Strike Victim Modifiers
    PROC_ON_STRIKEVICTIM_NONE               = 0x0000,
    PROC_ON_STRIKEVICTIM_DODGE              = 0x0001,
    PROC_ON_STRIKEVICTIM_PARRY              = 0x0002,
    PROC_ON_STRIKEVICTIM_HIT                = 0x0004,
    PROC_ON_STRIKEVICTIM_BLOCK              = 0x0008,
    PROC_ON_STRIKEVICTIM_CRITICAL           = 0x0010,
    PROC_ON_STRIKEVICTIM_SPELL_HIT          = 0x0020,
    // Spell hit
    PROC_ON_SPELL_LAND_RESIST               = 0x0001,
    PROC_ON_SPELL_LAND_RESIST_PARTIAL       = 0x0002,
};

class SpellProcManager : public Singleton<SpellProcManager>
{
public:
    SpellProcManager();
    ~SpellProcManager();

    // Initialize has to happen after spell data has been pooled!
    void InitProcData();

    // 
    void QuickProcessProcs(Unit *caster, uint8 procType, uint16 procMods);
    uint32 ProcessProcFlags(Unit *caster, Unit *target, std::map<uint8, uint16> procPairs, std::map<uint8, uint16> vProcPairs, SpellEntry *fromAbility, int32 &realDamage, uint32 &absoluteDamage, uint8 weaponDamageType);

    // Proc trigger checks
    bool HandleAuraProcTriggerDummy(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply);
    void HandleAuraProcTriggerSpell(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply);
    void HandleAuraProcTriggerDamage(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply);

public:

    // Register a proc data function
    void RegisterProcData(SpellEntry *sp, SpellProcData *procData);

    // Checks proc data
    bool ProcDataMatches(SpellEntry *sp, uint8 inputType, uint8 inputModifier, uint16 expectedModifier);

    // Returns proc data for spell entry pointer
    SpellProcData *GetSpellProcData(SpellEntry *sp);

    // Only use inside Player::SpellInterface::CanTriggerProc
    bool CanTriggerProc(SpellProcData *procData, Unit *target);

    // Checks if the spells that are triggered by our target match us
    bool MatchTriggerSpell(SpellEntry *target, SpellEntry *trigger);

    // Returns the triggered spell
    SpellEntry *GetSpellProcFromSpellEntry(SpellEntry *sp, uint8 index, Unit *target = NULL);

private:    // Spell proc overrides for different classes
    void _RegisterWarriorProcs();
    void _RegisterPaladinProcs();
    void _RegisterHunterProcs();
    void _RegisterRogueProcs();
    void _RegisterPriestProcs();
    void _RegisterDeathKnightProcs();
    void _RegisterShamanProcs();
    void _RegisterMageProcs();
    void _RegisterWarlockProcs();
    void _RegisterDruidProcs();

private:

    std::map<uint32, SpellProcData*> m_spellProcData;
};

#define sSpellProcMgr SpellProcManager::getSingleton()
