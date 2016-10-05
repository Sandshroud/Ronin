/***
* Demonstrike Core
*/

#pragma once

class SpellEffectClass;

typedef bool(*tSpellAmountModifier)(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount);
typedef bool(*tSpellDummyEffect)(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount);

class SERVER_DECL SpellManager : public Singleton<SpellManager>
{
public:
    SpellManager();
    ~SpellManager();

    void Initialize();
    void ParseSpellDBC();

    void LoadSpellFixes();
    void PoolSpellData();

    bool HandleTakePower(SpellEffectClass *spell, Unit *unitCaster, int32 powerField, int32 &cost, bool &result);
    // Modifiers for effect amounts and dummy effect handlers
    bool ModifyEffectAmount(SpellEffectClass *spell, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount);
    bool HandleDummyEffect(SpellEffectClass *spell, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount);

private:    // Spell fixes, start with class then continue to zones, items, quests
    void _RegisterWarriorFixes();
    void _RegisterPaladinFixes();
    void _RegisterHunterFixes();
    void _RegisterRogueFixes();
    void _RegisterPriestFixes();
    void _RegisterDeathKnightFixes();
    void _RegisterShamanFixes();
    void _RegisterMageFixes();
    void _RegisterWarlockFixes();
    void _RegisterDruidFixes();

    // Register handler
    RONIN_INLINE void _RegisterAmountModifier(uint32 spellId, uint32 effIndex, tSpellAmountModifier amountHandler) { m_amountModifierHandlers.insert(std::make_pair(std::make_pair(spellId, effIndex), amountHandler)); }
    RONIN_INLINE void _RegisterDummyEffect(uint32 spellId, uint32 effIndex, tSpellDummyEffect dummyHandler) { m_dummyEffectHandlers.insert(std::make_pair(std::make_pair(spellId, effIndex), dummyHandler)); }

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

    // Dummy effect handler storage
    std::map<std::pair<uint32, uint32>, tSpellAmountModifier> m_amountModifierHandlers;
    std::map<std::pair<uint32, uint32>, tSpellDummyEffect> m_dummyEffectHandlers;
};

#define sSpellMgr SpellManager::getSingleton()
