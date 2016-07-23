/***
* Demonstrike Core
*/

#pragma once

class SERVER_DECL SpellManager : public Singleton<SpellManager>
{
public:
    SpellManager();
    ~SpellManager();

    void Initialize();
    void ParseSpellDBC();

    void LoadSpellFixes();
    void PoolSpellData();

private:
    // Sets default values for custom fields
    void SetSingleSpellDefaults(SpellEntry *sp);

    // Creates a dummy spell entry pointer at ID
    SpellEntry *_CreateDummySpell(uint32 id);

    // Links spell target flags to spell target IDs
    void SetupSpellTargets();

    // Single spell fixing for ID and hashname based fixes
    void ApplySingleSpellFixes(SpellEntry *sp);

    // Coefficient spell fixes for spell scaling
    void ApplyCoeffSpellFixes(SpellEntry *sp);

    // Set proc flags based on spell data
    void SetProcFlags(SpellEntry *sp);

    // Dumy spell id storage
    std::set<uint32> m_dummySpells;
};

#define sSpellMgr SpellManager::getSingleton()
