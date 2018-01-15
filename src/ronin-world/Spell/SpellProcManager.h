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

    virtual bool canProc(uint8 procIdentifier, Unit *target, SpellEntry *sp, uint32 procInputFlags) = 0;
    virtual bool endsDummycheck() { return false; }

private:
    SpellEntry *spellProto;
};

enum SpellProcFlags
{
    PROC_NULL                               = 0x00000000,
    PROC_ON_DEATH                           = 0x00000001,
    PROC_ON_KILL                            = 0x00000002,
    // Use weapon type to determine melee/offhand or ranged
    PROC_ON_STRIKE                          = 0x00000004,
    PROC_ON_STRIKE_VICTIM                   = 0x00000008,
    // Strike modifiers
    PROC_ON_STRIKE_NON_DIRECT_HIT           = 0x00000010,
    PROC_ON_STRIKE_CRITICAL_HIT             = 0x00000020,
    // Spell hit
    PROC_ON_SPELL_LAND                      = 0x00000040,
    PROC_ON_SPELL_LAND_VICTIM               = 0x00000080,

};

class SpellProcManager : public Singleton<SpellProcManager>
{
public:
    SpellProcManager();
    ~SpellProcManager();

    void Initialize();

    // 
    void QuickProcessProcs(Unit *caster, uint32 procFlags);
    uint32 ProcessProcFlags(Unit *caster, Unit *target, uint32 procFlags, uint32 victimFlags, SpellEntry *fromAbility, int32 &realDamage, uint32 &absoluteDamage, uint8 weaponDamageType);

    // Proc trigger checks
    bool HandleAuraProcTriggerDummy(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply);
    void HandleAuraProcTriggerSpell(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply);
    void HandleAuraProcTriggerDamage(Unit *target, SpellEntry *spellProto, Modifier *auraMod, bool apply);

public:

    // Register a proc data function
    void RegisterProcData(SpellEntry *sp, SpellProcData *procData);

    // Returns proc data for spell entry pointer
    SpellProcData *GetSpellProcData(SpellEntry *sp);

private:

    std::map<uint32, SpellProcData*> m_spellProcData;
};

#define sSpellProcMgr SpellProcManager::getSingleton()
