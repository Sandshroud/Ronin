/***
* Demonstrike Core
*/

#pragma once

class Creature;
class SpellEffectClass;
class SpellCastTargets;

typedef bool(*tSpellAmountModifier)(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount);
typedef bool(*tSpellDummyEffect)(SpellEntry *sp, uint32 effIndex, WorldObject *caster, WorldObject *target, int32 &amount);

typedef bool(*tCanCastCCS)(SpellEntry *sp, Creature *ctr);
typedef bool(*tGenCCSTargets)(SpellEntry *sp, Creature *ctr, SpellCastTargets *targets, WoWGuid attackGuid);

#define SKILL_TYPE_ATTRIBUTES           5
#define SKILL_TYPE_WEAPON               6
#define SKILL_TYPE_CLASS                7
#define SKILL_TYPE_ARMOR                8
#define SKILL_TYPE_SECONDARY            9
#define SKILL_TYPE_LANGUAGE             10
#define SKILL_TYPE_PROFESSION           11
#define SKILL_TYPE_NA                   12

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

    bool CanCastCreatureCombatSpell(SpellEntry *sp, Creature *ctr);
    bool GenerateCreatureCombatSpellTargets(SpellEntry *sp, Creature *ctr, SpellCastTargets *targets, WoWGuid attackGuid);

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


    RONIN_INLINE void _RegisterCanCastCCS(uint32 spellId, tCanCastCCS canCastCCS) { m_canCastCCSTriggers.insert(std::make_pair(spellId, canCastCCS)); }
    RONIN_INLINE void _RegisterGenCCSTargets(uint32 spellId, tGenCCSTargets genCCSTargets) { m_genCCSTargetTriggers.insert(std::make_pair(spellId, genCCSTargets)); }

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

    // Creature Combat spell limiters
    std::map<uint32, tCanCastCCS> m_canCastCCSTriggers;
    std::map<uint32, tGenCCSTargets> m_genCCSTargetTriggers;

    // Skill line parsed data
    std::map<uint8, std::vector<SkillLineEntry*>> m_skillLinesByCategory;
    std::map<std::string, SkillLineEntry*> m_skillLinesByName;
    // Language spells
    std::set<std::pair<char*, uint16>> m_languageSkillIds;
};

#define sSpellMgr SpellManager::getSingleton()
