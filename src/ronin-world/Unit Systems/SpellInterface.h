
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

    SpellEntry *GetCurrentSpellProto();
    bool IsCasting() { return (m_currentSpell != NULL); }

    void ProcessSpell(Spell *castingSpell);
    void FinishSpell(Spell *finishedSpell);
    void InterruptCast(Spell *interruptSpell, uint32 interruptTime = 0);

    void LaunchSpell(SpellEntry *info, Unit *target = NULL, uint8 castNumber = 0);
    void TriggerSpell(SpellEntry *info, Unit *target, uint8 castNumber = 0);

    void PushbackCast(uint32 school);
    void PreventSchoolCast(uint32 school, uint32 duration);

protected:
    bool checkCast(SpellEntry *sp, uint8 &errorOut);

private:
    Unit *m_Unit;

    Spell* m_currentSpell;
    SpellEntry *m_lastSpell;
};
