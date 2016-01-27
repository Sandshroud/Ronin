/***
 * Demonstrike Core
 */

#pragma once

class CreatureData;
class CreatureInfoExtra;

class SERVER_DECL AIInterface
{
public:
    friend class AI_Movement;
    typedef Loki::AssocVector<uint32, AI_Spell*> AISpellMap;

public:
    AIInterface();
    ~AIInterface();

    // Misc
    void Init(Unit* un, AIType at, Unit *owner = NULL);

    Unit* GetUnit() { return m_Unit; }
    Unit* GetPetOwner() { return m_PetOwner; }

    bool setInFront(Unit* target);

    // Type/State
    RONIN_INLINE AIType getAIType() { return m_AIType; }
    RONIN_INLINE void SetAIType(AIType newtype) { m_AIType = newtype; }

    RONIN_INLINE uint8 getAIState() { return m_AIState; }
    RONIN_INLINE void SetAIState(AI_State newstate) { m_AIState = newstate; }

    RONIN_INLINE CreatureState getCreatureState() { return m_creatureState; };
    RONIN_INLINE void setCreatureState(CreatureState state) { m_creatureState = state; }

    void WipeHateList();
    void ClearHateList();
    Unit* GetMostHated(AI_Spell* sp = NULL);
    Unit* GetSecondHated(AI_Spell* sp = NULL);

    bool GetIsTaunted();
    Unit* getTauntedBy();
    bool taunt(Unit* caster, bool apply = true);

    uint32 getThreat(WoWGuid guid);
    void RemoveThreat(WoWGuid guid);
    bool modThreat(WoWGuid guid, int32 mod, Unit* redirect = NULL, float redirectVal = 0.f);

    void HandlePetAction(uint32 action) {}
    void WipeTargetList();
    RONIN_INLINE TargetMap *GetAITargets() { return &m_aiTargets; }
    RONIN_INLINE size_t getAITargetsCount() { return m_aiTargets.size(); }

    RONIN_INLINE uint32 getOutOfCombatRange() { return m_outOfCombatRange; }
    void setOutOfCombatRange(uint32 val) { m_outOfCombatRange = val; }

    void InitalizeExtraInfo(CreatureData *, CreatureInfoExtra *, uint32);

    // Spell
    SpellEntry *getSpellEntry(uint32 spellId);
    void addSpellToList(AI_Spell *sp);
    void CancelSpellCast();

    // Event Handler
    void HandleEvent(uint32 event, Unit* pUnit, uint32 misc1);

    void OnDeath(WorldObject* pKiller);
    void OnRespawn(Unit* unit); // We don't really need the unit anymore.
    void AttackReaction( Unit* pUnit, uint32 damage_dealt, uint32 spellId = 0);
    bool HealReaction(Unit* caster, Unit* victim, uint32 amount, SpellEntry * sp);

    // Update
    void Update(uint32 p_time);

    // Calculation
    float _CalcDistanceFromHome();
    float _CalcAggroRange(Unit* target);
    float _CalcCombatRange(Unit* target, bool ranged);
    uint32 _CalcThreat(uint32 damage, SpellEntry * sp, Unit* Attacker);

    void SetAllowedToEnterCombat(bool val) { m_AllowedToEnterCombat = val; }
    RONIN_INLINE bool GetAllowedToEnterCombat(void) { return m_AllowedToEnterCombat; }

    void CheckTarget(Unit* target);

    void SetBehaviorType(BehaviorType pBehavior) { unitBehavior = pBehavior; };
    BehaviorType GetBehaviorType() { return unitBehavior; };

    uint32 GetWeaponEmoteType(bool ranged);
    RONIN_INLINE Unit* GetNextTarget() { return m_nextTarget; }
    RONIN_INLINE bool SetNextTarget(Unit* nextTarget);

    void WipeReferences();
    RONIN_INLINE void SetPetOwner(Unit* owner) { m_PetOwner = owner; }

    Unit *GetTargetForSpell(AI_Spell* pSpell);
    Unit *GetNearestTargetInSet(std::set<Unit*> pTargetSet);
    Unit *GetSecondMostHatedTargetInSet(std::set<Unit*> pTargetSet );
    Unit *ChooseBestTargetInSet(std::set<Unit*> pTargetSet, uint32 pTargetFilter);
    Unit *GetBestUnitTarget( SpellEntry *info, uint32 pTargetFilter, float pMinRange, float pMaxRange);
    Unit *GetBestPlayerTarget( SpellEntry *info, uint32 pTargetFilter, float pMinRange, float pMaxRange);

    bool CanCastAISpell(AI_Spell* toCast, uint32 currentTime);
    void CastAISpell(Unit* Target, AI_Spell* toCast, uint32 currentTime);
    bool IsValidUnitTarget( WorldObject *pObject, SpellEntry *info, uint32 pFilter, float pMinRange = 0.0f, float pMaxRange = 0.0f  );
    bool IsValidUnitTarget( WorldObject *pObject, AI_Spell* pSpell)
    {
        if(pObject == NULL || pSpell == NULL)
            return false;
        return IsValidUnitTarget(pObject, pSpell->info, pSpell->TargetFilter, pSpell->mindist2cast, pSpell->maxdist2cast);
    }

    void ResetProcCounts(bool all = false);
    void CheckNextTargetFlyingStatus();

protected:

    // Update
    void _UpdateTotem(uint32 p_time);
    void _UpdateTargets(uint32 p_time);
    void _UpdateCombat(uint32 p_time);

    // Misc
    Unit* FindTarget();
    bool FindFriends(float dist);

public:
    bool m_AllowedToEnterCombat;
    CreatureState m_creatureState;
    bool m_canCallForHelp;
    float m_CallForHelpHealth;
    bool m_canRangedAttack;
    bool m_canFlee;
    float m_FleeHealth;
    uint32 m_FleeDuration;
    int sendflee_message;
    std::string flee_message;

    uint32 m_totemSpellTimer, m_totemSpellTime;
    SpellEntry * m_totemSpell;

    AISpellMap m_spells;

    bool disable_combat;

    bool disable_melee;
    bool disable_ranged;
    bool disable_spell;
    bool disable_targeting;

    bool waiting_for_cooldown;

    uint32 m_CastTimer;

    AI_Spell* m_CastNext;

    bool firstLeaveCombat;
    uint32 m_updateListTimer;
    uint32 m_updateTargetsTimer;

    Unit* m_nextTarget;
    uint32 m_fleeTimer;
    bool m_hasFled;
    bool m_hasCalledForHelp;
    uint32 m_outOfCombatRange;

    Unit* m_Unit;
    Unit* m_PetOwner;

    Mutex ai_TargetLock;
    TargetMap m_aiTargets;

    AIType m_AIType;
    AI_State m_AIState;
    BehaviorType unitBehavior;

    Unit* tauntedBy; //This mob will hit only tauntedBy mob.
    bool isTaunted;
    Unit* soullinkedWith; //This mob can be hitten only by soullinked unit
    bool isSoulLinked;

    int32 m_currentHighestThreat;

public:
    bool m_is_in_instance;
    bool skip_reset_hp;

    void WipeCurrentTarget();
    bool CheckCurrentTarget();
    bool TargetUpdateCheck(Unit* ptr);

};
