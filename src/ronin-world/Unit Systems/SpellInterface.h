
#pragma once

class Unit;
class Spell;

class SERVER_DECL SpellInterface
{
public:
    SpellInterface(Unit *unit);
    ~SpellInterface();

    void Update(uint32 msTime, uint32 uiDiff);

    void Cleanup();
    void OnRemoveFromWorld();
    void CleanupCurrentSpell();
    bool CleanupSpecificSpell(SpellEntry *sp);

    void OnChangeSelection(WoWGuid guid);

    SpellEntry *GetCurrentSpellProto();
    bool IsCasting() { return (m_currentSpell != NULL); }

    void ProcessSpell(Spell *castingSpell);
    void FinishSpell(Spell *finishedSpell);
    void InterruptCast(Spell *interruptSpell, uint32 interruptTime = 0);
    void ProcessNextMeleeSpell(Spell *nextMeleeSpell);

    // Spell casting
    void LaunchSpell(SpellEntry *info, uint8 castNumber, SpellCastTargets &targets);
    void LaunchSpell(SpellEntry *info, float posX, float posY, float posZ);
    void LaunchSpell(SpellEntry *info, Unit *target = NULL);
    void TriggerSpell(SpellEntry *info, float posX, float posY, float posZ);
    void TriggerSpell(SpellEntry *info, Unit *target);

    void LaunchSpellFromSpell(SpellEntry *info, Unit *target, SpellEntry *parent);

    // Next melee spells
    uint32 getNextMeleeSpell();
    void TriggerNextMeleeSpell(Unit *target);
    void ClearNextMeleeSpell();

    // School affecting
    void PushbackCast(uint32 school);
    void PreventSchoolCast(uint32 school, uint32 duration);

protected:
    friend class WorldSession;
    bool checkCast(SpellEntry *sp, SpellCastTargets &targets, uint8 &errorOut);

private:
    Unit *m_Unit;

    Mutex _spellLock;
    Spell* m_currentSpell, *m_nextMeleeSpell;

    SpellEntry *m_lastSpell;

protected: // Proc related functions
    friend class SpellProcManager;

    void AddProcData(SpellProcData *procData);
    void RemoveProcData(SpellProcData *procData);

    bool CanTriggerProc(SpellProcData *procData, SpellEntry *fromAbility, time_t triggerTime, uint32 msTime);
    void TriggerProc(SpellProcData *procData, Unit *target);

    std::set<SpellProcData*>::iterator beginProcData() { return m_spellProcData.begin(); }
    std::set<SpellProcData*>::iterator endProcData() { return m_spellProcData.end(); }

private:
    std::set<SpellProcData*> m_spellProcData;
};
