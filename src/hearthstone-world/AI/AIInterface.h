/***
 * Demonstrike Core
 */

#pragma once

class SERVER_DECL AIInterface
{
    friend class AI_Movement;
public:
    AIInterface();
    ~AIInterface();

    // Misc
    void Init(Unit* un, AIType at, MovementType mt);
    void Init(Unit* un, AIType at, MovementType mt, Unit* owner); // used for pets

    Unit* GetUnit() { return m_Unit; }
    Unit* GetPetOwner() { return m_PetOwner; }

    bool setInFront(Unit* target);

    // Type/State
    HEARTHSTONE_INLINE AIType getAIType() { return m_AIType; }
    HEARTHSTONE_INLINE void SetAIType(AIType newtype) { m_AIType = newtype; }

    HEARTHSTONE_INLINE uint8 getAIState() { return m_AIState; }
    HEARTHSTONE_INLINE void SetAIState(AI_State newstate) { m_AIState = newstate; }

    HEARTHSTONE_INLINE CreatureState getCreatureState() { return m_creatureState; };
    HEARTHSTONE_INLINE void setCreatureState(CreatureState state) { m_creatureState = state; }

    void WipeHateList();
    void ClearHateList();
    Unit* GetMostHated(AI_Spell* sp = NULL);
    Unit* GetSecondHated(AI_Spell* sp = NULL);

    bool GetIsTaunted();
    Unit* getTauntedBy();
    bool taunt(Unit* caster, bool apply = true);

    void RemoveThreatByPtr(Unit* obj);
    uint32 getThreatByPtr(Unit* obj);
    uint32 getThreatByGUID(uint64 guid);
    bool modThreatByPtr(Unit* obj, int32 mod);
    bool modThreatByGUID(uint64 guid, int32 mod);

    void WipeTargetList();
    HEARTHSTONE_INLINE TargetMap *GetAITargets() { return &m_aiTargets; }
    HEARTHSTONE_INLINE size_t getAITargetsCount() { return m_aiTargets.size(); }

    HEARTHSTONE_INLINE uint32 getOutOfCombatRange() { return m_outOfCombatRange; }
    void setOutOfCombatRange(uint32 val) { m_outOfCombatRange = val; }

    // Spell
    SpellEntry *getSpellEntry(uint32 spellId);
    void addSpellToList(AI_Spell *sp);
    void CancelSpellCast();

    // Event Handler
    void HandleEvent(uint32 event, Unit* pUnit, uint32 misc1);

    void EventForceRedirected(Unit* pUnit, uint32 misc1);
    void EventHostileAction(Unit* pUnit, uint32 misc1);
    void EventUnitDied(Unit* pUnit, uint32 misc1);
    void EventUnwander(Unit* pUnit, uint32 misc1);
    void EventWander(Unit* pUnit, uint32 misc1);
    void EventUnfear(Unit* pUnit, uint32 misc1);
    void EventFear(Unit* pUnit, uint32 misc1);
    void EventUnitRespawn(Unit* pUnit, uint32 misc1);
    void EventFollowOwner(Unit* pUnit, uint32 misc1);
    void EventDamageTaken(Unit* pUnit, uint32 misc1);
    void EventLeaveCombat(Unit* pUnit, uint32 misc1);
    void EventEnterCombat(Unit* pUnit, uint32 misc1);

    void OnDeath(Object* pKiller);
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
    HEARTHSTONE_INLINE bool GetAllowedToEnterCombat(void) { return m_AllowedToEnterCombat; }

    void CheckTarget(Unit* target);

    void SetBehaviorType(BehaviorType pBehavior) { unitBehavior = pBehavior; };
    BehaviorType GetBehaviorType() { return unitBehavior; };

    CreatureState m_creatureState;

    uint32 GetWeaponEmoteType(bool ranged);
    bool m_canCallForHelp;
    float m_CallForHelpHealth;
    bool m_canRangedAttack;
    bool m_canFlee;
    float m_FleeHealth;
    uint32 m_FleeDuration;
    int sendflee_message;
    string flee_message;

    uint32 m_totemspelltimer;
    uint32 m_totemspelltime;
    SpellEntry * totemspell;

    HEARTHSTONE_INLINE Unit* GetNextTarget() { return m_nextTarget; }
    HEARTHSTONE_INLINE void SetNextTarget(Unit* nextTarget)
    {
        m_nextTarget = nextTarget;
        if(nextTarget)
            m_Unit->SetUInt64Value(UNIT_FIELD_TARGET, m_nextTarget->GetGUID());
        else
            m_Unit->SetUInt64Value(UNIT_FIELD_TARGET, 0);
    }

    void WipeReferences();
    HEARTHSTONE_INLINE void SetPetOwner(Unit* owner) { m_PetOwner = owner; }

    map<uint32, AI_Spell*> m_spells;

    bool disable_combat;

    bool disable_melee;
    bool disable_ranged;
    bool disable_spell;
    bool disable_targeting;

    bool waiting_for_cooldown;

    uint32 m_CastTimer;

    AI_Spell* m_CastNext;

    Unit *GetTargetForSpell(AI_Spell* pSpell);
    Unit *GetNearestTargetInSet(set<Unit*> pTargetSet);
    Unit *GetSecondMostHatedTargetInSet(set<Unit*> pTargetSet );
    Unit *ChooseBestTargetInSet(set<Unit*> pTargetSet, uint32 pTargetFilter);
    Unit *GetBestUnitTarget( SpellEntry *info, uint32 pTargetFilter, float pMinRange, float pMaxRange);
    Unit *GetBestPlayerTarget( SpellEntry *info, uint32 pTargetFilter, float pMinRange, float pMaxRange);

    bool CanCastAISpell(AI_Spell* toCast, uint32 currentTime);
    void CastAISpell(Unit* Target, AI_Spell* toCast, uint32 currentTime);
    bool IsValidUnitTarget( Object *pObject, SpellEntry *info, uint32 pFilter, float pMinRange = 0.0f, float pMaxRange = 0.0f  );
    bool IsValidUnitTarget( Object *pObject, AI_Spell* pSpell)
    {
        if(pObject == NULL || pSpell == NULL)
            return false;
        return IsValidUnitTarget(pObject, pSpell->info, pSpell->TargetFilter, pSpell->mindist2cast, pSpell->maxdist2cast);
    }

    void ResetProcCounts(bool all = false);

    bool m_isGuard;
    void CallGuards();
    void setGuardTimer(uint32 timer) { m_guardTimer = timer; }
    uint32 m_guardCallTimer;
    void _UpdateCombat(uint32 p_time);
    void CheckNextTargetFlyingStatus();

protected:
    bool m_AllowedToEnterCombat;

    // Update
    void _UpdateTargets(uint32 p_time);

    uint32 m_updateListTimer;
    uint32 m_updateTargetsTimer;

    // Misc
    bool firstLeaveCombat;
    Unit* FindTarget();
    bool FindFriends(float dist);

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

    uint32 m_guardTimer;
    int32 m_currentHighestThreat;

public:
    bool m_is_in_instance;
    bool skip_reset_hp;

    void WipeCurrentTarget();
    bool CheckCurrentTarget();
    bool TargetUpdateCheck(Unit* ptr);

    /****************************************************
    ***** MOVEMENT HANDLING
    ********************************/
private: // Our internal handler
    AI_Movement MovementHandler;

public:
    HEARTHSTONE_INLINE void JumpToTargetLocation() { MovementHandler.m_moveJump = true; };
    HEARTHSTONE_INLINE void MoveTo(float x, float y, float z, float o = 0.0f, bool IgnorePathMap = false) { MovementHandler.MoveTo(x, y, z, o, IgnorePathMap); };
    HEARTHSTONE_INLINE void _CalcDestinationAndMove( Unit* target, float dist) { MovementHandler._CalcDestinationAndMove(target, dist); };

    HEARTHSTONE_INLINE void SendCurrentMove(Player* plyr)
    { MovementHandler.SendCurrentMove(plyr); };
    HEARTHSTONE_INLINE void SendMoveToPacket(Player* playerTarget = NULL)
    { MovementHandler.SendMoveToPacket(playerTarget); };
    HEARTHSTONE_INLINE void SendMoveToPacket(float toX, float toY, float toZ, float toO, uint32 time, uint32 MoveFlags, Player* playerTarget = NULL)
    { MovementHandler.SendMoveToPacket(toX, toY, toZ, toO, time, MoveFlags, playerTarget); };
    HEARTHSTONE_INLINE void SendJumpTo(float toX, float toY, float toZ, uint32 moveTime, float arc, uint32 unk = 0)
    { MovementHandler.SendJumpTo(toX, toY, toZ, moveTime, arc, unk); };

    HEARTHSTONE_INLINE float GetMovementTime(float distance) { return MovementHandler.GetMovementTime(distance); };

    HEARTHSTONE_INLINE bool HasDestination() { return MovementHandler.HasDestination(); }
    HEARTHSTONE_INLINE LocationVector GetDestPos() { return MovementHandler.GetDestPos(); }
    HEARTHSTONE_INLINE LocationVector GetNextPos() { return MovementHandler.GetNextPos(); }
    HEARTHSTONE_INLINE LocationVector GetReturnPos() { return MovementHandler.GetReturnPos(); }

    HEARTHSTONE_INLINE void SetSourcePos(float x, float y, float z) { MovementHandler.SetSourcePos(x, y, z); };
    HEARTHSTONE_INLINE void SetReturnPos(float x, float y, float z, float o = 0.0f) { MovementHandler.SetReturnPos(x, y, z, o); };
    HEARTHSTONE_INLINE void SetNextPos(float x, float y, float z, float o = 0.0f) { MovementHandler.SetNextPos(x, y, z, o); };
    HEARTHSTONE_INLINE void SetDestPos(float x, float y, float z, float o = 0.0f) { MovementHandler.SetDestPos(x, y, z, o); };

    HEARTHSTONE_INLINE void ResetLastFollowPos() { MovementHandler.ResetLastFollowPos(); }
    HEARTHSTONE_INLINE uint32 getMoveFlags(bool ignorejump = false) { return MovementHandler.getMoveFlags(ignorejump); };

    HEARTHSTONE_INLINE void AddStopTime(uint32 Time) { MovementHandler.AddStopTime(Time); };
    HEARTHSTONE_INLINE void StopMovement(uint32 time, bool stopatcurrent = true) { MovementHandler.StopMovement(time, stopatcurrent); };

    HEARTHSTONE_INLINE void setMoveRunFlag(bool f) { MovementHandler.setMoveRunFlag(f); }
    HEARTHSTONE_INLINE bool getMoveRunFlag() { return MovementHandler.getMoveRunFlag(); }

    HEARTHSTONE_INLINE void setMoveFlyFlag(bool f) { MovementHandler.setMoveFlyFlag(f); }
    HEARTHSTONE_INLINE bool getMoveFlyFlag() { return MovementHandler.getMoveFlyFlag(); }

    HEARTHSTONE_INLINE void setMoveSprintFlag(bool f) { MovementHandler.setMoveSprintFlag(f); }
    HEARTHSTONE_INLINE bool getMoveSprintFlag() { return MovementHandler.getMoveSprintFlag(); }

    HEARTHSTONE_INLINE void setMoveType(uint32 m) { MovementHandler.setMoveType(m); }
    HEARTHSTONE_INLINE uint32 getMoveType() { return MovementHandler.getMoveType(); }

    HEARTHSTONE_INLINE void setCanMove(bool c) { MovementHandler.setCanMove(c); }
    HEARTHSTONE_INLINE bool getCanMove() { return MovementHandler.getCanMove(); }

    HEARTHSTONE_INLINE bool IsFlying() { return MovementHandler.IsFlying(); };

    HEARTHSTONE_INLINE void setFearTimer(uint32 time) { MovementHandler.setFearTimer(time); }
    HEARTHSTONE_INLINE uint32 getFearTimer() { return MovementHandler.getFearTimer(); }

    HEARTHSTONE_INLINE void setWanderTimer(uint32 time) { MovementHandler.setWanderTimer(time); }
    HEARTHSTONE_INLINE uint32 getWanderTimer() { return MovementHandler.getWanderTimer(); }

    HEARTHSTONE_INLINE void RestoreFollowInformation() { MovementHandler.RestoreFollowInformation(); }
    HEARTHSTONE_INLINE void BackupFollowInformation() { MovementHandler.BackupFollowInformation(); }

    // Follow Code
    HEARTHSTONE_INLINE void ClearFollowInformation(Unit* u = NULL)  { MovementHandler.ClearFollowInformation(u); };

    HEARTHSTONE_INLINE Unit* getUnitToFear() { return MovementHandler.getUnitToFear(); };
    HEARTHSTONE_INLINE void SetUnitToFear(Unit* un)  { MovementHandler.SetUnitToFear(un); };

    HEARTHSTONE_INLINE Unit* getUnitToFollow() { return MovementHandler.getUnitToFollow(); };
    HEARTHSTONE_INLINE void SetUnitToFollow(Unit* un) { MovementHandler.SetUnitToFollow(un); };

    HEARTHSTONE_INLINE Unit* getBackupUnitToFollow() { return MovementHandler.getBackupUnitToFollow(); };
    HEARTHSTONE_INLINE void SetBackupUnitToFollow(Unit* un) { MovementHandler.SetBackupUnitToFollow(un); };

    HEARTHSTONE_INLINE float GetFollowDistance() { return MovementHandler.GetFollowDistance(); };
    HEARTHSTONE_INLINE void SetFollowDistance(float dist) { MovementHandler.SetFollowDistance(dist); };

    HEARTHSTONE_INLINE float GetUnitToFollowAngle() { return MovementHandler.GetUnitToFollowAngle(); }
    HEARTHSTONE_INLINE void SetUnitToFollowAngle(float angle) { MovementHandler.SetUnitToFollowAngle(angle); }

    HEARTHSTONE_INLINE void SetFormationLinkTarget(Creature* cr)  { MovementHandler.SetFormationLinkTarget(cr); };
    HEARTHSTONE_INLINE Creature* GetFormationLinkTarget() { return MovementHandler.GetFormationLinkTarget(); }

    HEARTHSTONE_INLINE uint32 GetFormationSQLId() { return MovementHandler.GetFormationSQLId(); };
    HEARTHSTONE_INLINE void SetFormationSQLId(uint32 Id) { MovementHandler.SetFormationSQLId(Id); }

    HEARTHSTONE_INLINE uint32 GetFormationFollowAngle() { return MovementHandler.GetFormationFollowAngle(); };
    HEARTHSTONE_INLINE void SetFormationFollowAngle(float Angle) { MovementHandler.SetFormationFollowAngle(Angle); }

    HEARTHSTONE_INLINE uint32 GetFormationFollowDistance() { return MovementHandler.GetFormationFollowDistance(); };
    HEARTHSTONE_INLINE void SetFormationFollowDistance(float Dist) { MovementHandler.SetFormationFollowDistance(Dist); }

    HEARTHSTONE_INLINE uint32 getCurrentWaypoint() { return MovementHandler.getCurrentWaypoint(); }
    HEARTHSTONE_INLINE void setWaypointToMove(uint32 id) { return MovementHandler.setWaypointToMove(id); }

    HEARTHSTONE_INLINE uint32 GetWayPointsCount() { return MovementHandler.GetWayPointsCount(); };
    HEARTHSTONE_INLINE void SetWaypointMap(WayPointMap * m) { MovementHandler.SetWaypointMap(m); };

    HEARTHSTONE_INLINE bool addWayPoint(WayPoint* wp) { return MovementHandler.addWayPoint(wp); };
    HEARTHSTONE_INLINE WayPoint* getWayPoint(uint32 wpid) { return MovementHandler.getWayPoint(wpid); };

    HEARTHSTONE_INLINE void deleteAllWaypoints() { MovementHandler.deleteAllWaypoints(); };
    HEARTHSTONE_INLINE void deleteWayPoint(uint32 id) { MovementHandler.deleteWayPoint(id); };

    HEARTHSTONE_INLINE void changeWayPointID(uint32 oldwpid, uint32 newwpid) { MovementHandler.changeWayPointID(oldwpid, newwpid); };

    HEARTHSTONE_INLINE bool WayPointsShowing() { return MovementHandler.WayPointsShowing(); };
    HEARTHSTONE_INLINE void SetWayPointsShowing(bool s) { MovementHandler.SetWayPointsShowing(s); };
    HEARTHSTONE_INLINE bool WayPointsShowingBackwards() { return MovementHandler.WayPointsShowingBackwards(); };
    HEARTHSTONE_INLINE void SetWayPointsShowingBackwards(bool bs) { MovementHandler.SetWayPointsShowingBackwards(bs); };

    HEARTHSTONE_INLINE bool hasWaypoints() { return MovementHandler.hasWaypoints(); };
    HEARTHSTONE_INLINE bool saveWayPoints() { return MovementHandler.saveWayPoints(); };
    HEARTHSTONE_INLINE bool hideWayPoints(Player* pPlayer) { return MovementHandler.hideWayPoints(pPlayer); };
    HEARTHSTONE_INLINE bool showWayPoints(Player* pPlayer, bool Backwards) { return MovementHandler.showWayPoints(pPlayer, Backwards); };
};
