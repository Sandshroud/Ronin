/***
* Demonstrike Core
*/

#pragma once

class Creature;
class ItemPrototype;
class SpellEffectClass;
class SpellCastTargets;

typedef bool(*tSpellAmountModifier)(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount);
typedef bool(*tSpellDummyEffect)(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount);
typedef bool(*tSpellScriptedEffect)(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount);
typedef bool(*tSpellTeleportData)(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O);

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

    bool CanTriggerInstantKillEffect(Unit *caster, Unit *target, SpellEntry *sp);
    ItemPrototype *GetCreateItemForSpell(Player *target, SpellEntry *info, uint32 effIndex, int32 amount, uint32 &count);

    bool HandleTakePower(SpellEffectClass *spell, Unit *unitCaster, int32 powerField, int32 &cost, bool &result);
    // Modifiers for effect amounts and dummy effect handlers
    bool ModifyEffectAmount(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount);
    bool HandleDummyEffect(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount);
    bool TriggerScriptedEffect(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 modAmt);
    bool FetchSpellCoordinates(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 modAmt, uint32 &mapId, float &X, float &Y, float &Z, float &O);

    bool CanCastCreatureCombatSpell(SpellEntry *sp, Creature *ctr);
    bool GenerateCreatureCombatSpellTargets(SpellEntry *sp, Creature *ctr, SpellCastTargets *targets, WoWGuid attackGuid);

    // Area and shapeshift checks
    bool IsAuraApplicable(Unit *unit, SpellEntry *spell);

    // Skill line processing functions
    std::vector<uint32> *GetSkillLineEntries(uint32 skillLine)
    {
        if(m_skillLineEntriesBySkillLine.find(skillLine) != m_skillLineEntriesBySkillLine.end())
            return &m_skillLineEntriesBySkillLine.at(skillLine);
        return NULL;
    }

    // Get our cached language spell id by skill
    uint32 getSpellIdForLanguageSkill(uint16 skillId)
    {
        if(m_SpellIdForLanguage.find(skillId) != m_SpellIdForLanguage.end())
            return m_SpellIdForLanguage[skillId];
        return 0;
    }

    // Grab our class for supplied skill line
    uint8 GetClassForSkillLine(uint32 skillLine);
    // Calculate our skill max based on input
    void getSkillLineValues(Player *target, SkillLineEntry *skillLineEntry, int32 amount, uint16 &skillMax, uint16 &skillCurrent);
    // Validate on unit load
    void ValidateSkillLineValues(uint16 skillId, uint16 skillStep, uint16 &skillCurrent, uint16 &skillMax);

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

    void _RegisterHomeboundData();
    void _RegisterTeleportData();
    void _RegisterTranslocateData();

    void _RegisterTirisfalGladesScripts();
    void _RegisterSilvermoonCityScripts();
    void _RegisterDalaranScripts();

    // Register handler
    RONIN_INLINE void _RegisterAmountModifier(uint32 spellId, uint32 effIndex, tSpellAmountModifier amountHandler) { m_amountModifierHandlers.insert(std::make_pair(std::make_pair(spellId, effIndex), amountHandler)); }
    RONIN_INLINE void _RegisterDummyEffect(uint32 spellId, uint32 effIndex, tSpellDummyEffect dummyHandler) { m_dummyEffectHandlers.insert(std::make_pair(std::make_pair(spellId, effIndex), dummyHandler)); }
    RONIN_INLINE void _RegisterScriptedEffect(uint32 spellId, uint32 effIndex, tSpellScriptedEffect scriptedHandler) { m_scriptedEffectHandlers.insert(std::make_pair(std::make_pair(spellId, effIndex), scriptedHandler)); }
    RONIN_INLINE void _RegisterTeleportData(uint32 spellId, uint32 effIndex, tSpellTeleportData teleportData) { m_teleportEffectHandlers.insert(std::make_pair(std::make_pair(spellId, effIndex), teleportData)); }

    RONIN_INLINE void _RegisterCanCastCCS(uint32 spellId, tCanCastCCS canCastCCS) { m_canCastCCSTriggers.insert(std::make_pair(spellId, canCastCCS)); }
    RONIN_INLINE void _RegisterGenCCSTargets(uint32 spellId, tGenCCSTargets genCCSTargets) { m_genCCSTargetTriggers.insert(std::make_pair(spellId, genCCSTargets)); }

private:
    // Sets default values for custom fields
    void SetSingleSpellDefaults(SpellEntry *sp);

    // Creates a dummy spell entry pointer at ID
    SpellEntry *_CreateDummySpell(uint32 id);
    SpellEntry *_CreateScriptedEffectSpell(uint32 id);

    // Links spell target flags to spell target IDs
    void SetupSpellTargets();

    // Single spell fixing for ID and hashname based fixes
    void ApplySingleSpellFixes(SpellEntry *sp);

    // Coefficient spell fixes for spell scaling
    void ApplyCoeffSpellFixes(SpellEntry *sp);

    // Set proc flags based on spell data
    void SetProcFlags(SpellEntry *sp);

    // Set custom fields and process into groups
    void ProcessSpellInformation(SpellEntry *sp);

    // Dumy spell id storage
    std::set<uint32> m_dummySpells;

    // Dummy effect handler storage
    std::map<std::pair<uint32, uint32>, tSpellAmountModifier> m_amountModifierHandlers;
    std::map<std::pair<uint32, uint32>, tSpellDummyEffect> m_dummyEffectHandlers;
    std::map<std::pair<uint32, uint32>, tSpellScriptedEffect> m_scriptedEffectHandlers;
    std::map<std::pair<uint32, uint32>, tSpellTeleportData> m_teleportEffectHandlers;

    // Creature Combat spell limiters
    std::map<uint32, tCanCastCCS> m_canCastCCSTriggers;
    std::map<uint32, tGenCCSTargets> m_genCCSTargetTriggers;

    // Skill line parsed data
    std::map<uint8, std::vector<SkillLineEntry*>> m_skillLinesByCategory;
    std::map<std::string, SkillLineEntry*> m_skillLinesByName;
    std::map<uint32, uint8> m_skillLineClasses;
    // Language spells
    std::set<std::pair<char*, uint16>> m_languageSkillIds;
    // Skill line processors
    std::map<uint32, std::vector<uint32>> m_skillLineEntriesBySkillLine;
    // Skill lines for languages
    std::map<uint16, uint32> m_SpellIdForLanguage;
};

#define sSpellMgr SpellManager::getSingleton()
