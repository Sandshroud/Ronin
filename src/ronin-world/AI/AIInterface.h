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

    void WipeTargetList();
    RONIN_INLINE TargetMap *GetAITargets() { return &m_aiTargets; }
    RONIN_INLINE size_t getAITargetsCount() { return m_aiTargets.size(); }

    RONIN_INLINE uint32 getOutOfCombatRange() { return m_outOfCombatRange; }
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

    CreatureState m_creatureState;

    uint32 GetWeaponEmoteType(bool ranged);
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

    RONIN_INLINE Unit* GetNextTarget() { return m_nextTarget; }
    RONIN_INLINE bool SetNextTarget(Unit* nextTarget);

    void WipeReferences();
    RONIN_INLINE void SetPetOwner(Unit* owner) { m_PetOwner = owner; }

    std::map<uint32, AI_Spell*> m_spells;

    bool disable_combat;

    bool disable_melee;
    bool disable_ranged;
    bool disable_spell;
    bool disable_targeting;

    bool waiting_for_cooldown;

    uint32 m_CastTimer;

    AI_Spell* m_CastNext;

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

    bool m_isGuard;
    void CallGuards();
    void setGuardTimer(uint32 timer) { m_guardTimer = timer; }
    uint32 m_guardCallTimer;
    void CheckNextTargetFlyingStatus();

protected:
    bool m_AllowedToEnterCombat;

    // Update
    void _UpdateTotem(uint32 p_time);
    void _UpdateTargets(uint32 p_time);
    void _UpdateCombat(uint32 p_time);

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
    RONIN_INLINE void JumpToTargetLocation() { MovementHandler.m_moveJump = true; };
    RONIN_INLINE void MoveTo(float x, float y, float z, float o = 0.0f, bool IgnorePathMap = false) { MovementHandler.MoveTo(x, y, z, o, IgnorePathMap); };
    RONIN_INLINE void _CalcDestinationAndMove( Unit* target, float dist) { MovementHandler._CalcDestinationAndMove(target, dist); };

    RONIN_INLINE void SendCurrentMove(Player* plyr)
    { MovementHandler.SendCurrentMove(plyr); };
    RONIN_INLINE void SendMoveToPacket(Player* playerTarget = NULL)
    { MovementHandler.SendMoveToPacket(playerTarget); };
    RONIN_INLINE void SendMoveToPacket(float toX, float toY, float toZ, float toO, uint32 time, uint32 MoveFlags, Player* playerTarget = NULL)
    { MovementHandler.SendMoveToPacket(toX, toY, toZ, toO, time, MoveFlags, playerTarget); };
    RONIN_INLINE void SendJumpTo(float toX, float toY, float toZ, uint32 moveTime, float arc, uint32 unk = 0)
    { MovementHandler.SendJumpTo(toX, toY, toZ, moveTime, arc, unk); };

    RONIN_INLINE float GetMovementTime(float distance) { return MovementHandler.GetMovementTime(distance); };

    RONIN_INLINE bool HasDestination() { return MovementHandler.HasDestination(); }
    RONIN_INLINE LocationVector GetDestPos() { return MovementHandler.GetDestPos(); }
    RONIN_INLINE LocationVector GetNextPos() { return MovementHandler.GetNextPos(); }
    RONIN_INLINE LocationVector GetReturnPos() { return MovementHandler.GetReturnPos(); }

    RONIN_INLINE void SetSourcePos(float x, float y, float z) { MovementHandler.SetSourcePos(x, y, z); };
    RONIN_INLINE void SetReturnPos(float x, float y, float z, float o = 0.0f) { MovementHandler.SetReturnPos(x, y, z, o); };
    RONIN_INLINE void SetNextPos(float x, float y, float z, float o = 0.0f) { MovementHandler.SetNextPos(x, y, z, o); };
    RONIN_INLINE void SetDestPos(float x, float y, float z, float o = 0.0f) { MovementHandler.SetDestPos(x, y, z, o); };

    RONIN_INLINE void ResetLastFollowPos() { MovementHandler.ResetLastFollowPos(); }
    RONIN_INLINE uint32 getMoveFlags(bool ignorejump = false) { return MovementHandler.getMoveFlags(ignorejump); };

    RONIN_INLINE void AddStopTime(uint32 Time) { MovementHandler.AddStopTime(Time); };
    RONIN_INLINE void StopMovement(uint32 time, bool stopatcurrent = true) { MovementHandler.StopMovement(time, stopatcurrent); };

    RONIN_INLINE void setMoveRunFlag(bool f) { MovementHandler.setMoveRunFlag(f); }
    RONIN_INLINE bool getMoveRunFlag() { return MovementHandler.getMoveRunFlag(); }

    RONIN_INLINE void setMoveFlyFlag(bool f) { MovementHandler.setMoveFlyFlag(f); }
    RONIN_INLINE bool getMoveFlyFlag() { return MovementHandler.getMoveFlyFlag(); }

    RONIN_INLINE void setMoveSprintFlag(bool f) { MovementHandler.setMoveSprintFlag(f); }
    RONIN_INLINE bool getMoveSprintFlag() { return MovementHandler.getMoveSprintFlag(); }

    RONIN_INLINE void setMoveType(uint32 m) { MovementHandler.setMoveType(m); }
    RONIN_INLINE uint32 getMoveType() { return MovementHandler.getMoveType(); }

    RONIN_INLINE void setCanMove(bool c) { MovementHandler.setCanMove(c); }
    RONIN_INLINE bool getCanMove() { return MovementHandler.getCanMove(); }

    RONIN_INLINE void setFearTimer(uint32 time) { MovementHandler.setFearTimer(time); }
    RONIN_INLINE uint32 getFearTimer() { return MovementHandler.getFearTimer(); }

    RONIN_INLINE void setWanderTimer(uint32 time) { MovementHandler.setWanderTimer(time); }
    RONIN_INLINE uint32 getWanderTimer() { return MovementHandler.getWanderTimer(); }

    RONIN_INLINE void RestoreFollowInformation() { MovementHandler.RestoreFollowInformation(); }
    RONIN_INLINE void BackupFollowInformation() { MovementHandler.BackupFollowInformation(); }

    // Follow Code
    RONIN_INLINE void ClearFollowInformation(Unit* u = NULL)  { MovementHandler.ClearFollowInformation(u); };

    RONIN_INLINE Unit* getUnitToFear() { return MovementHandler.getUnitToFear(); };
    RONIN_INLINE void SetUnitToFear(Unit* un)  { MovementHandler.SetUnitToFear(un); };

    RONIN_INLINE Unit* getUnitToFollow() { return MovementHandler.getUnitToFollow(); };
    RONIN_INLINE void SetUnitToFollow(Unit* un) { MovementHandler.SetUnitToFollow(un); };

    RONIN_INLINE Unit* getBackupUnitToFollow() { return MovementHandler.getBackupUnitToFollow(); };
    RONIN_INLINE void SetBackupUnitToFollow(Unit* un) { MovementHandler.SetBackupUnitToFollow(un); };

    RONIN_INLINE float GetFollowDistance() { return MovementHandler.GetFollowDistance(); };
    RONIN_INLINE void SetFollowDistance(float dist) { MovementHandler.SetFollowDistance(dist); };

    RONIN_INLINE float GetUnitToFollowAngle() { return MovementHandler.GetUnitToFollowAngle(); }
    RONIN_INLINE void SetUnitToFollowAngle(float angle) { MovementHandler.SetUnitToFollowAngle(angle); }

    RONIN_INLINE void SetFormationLinkTarget(Creature* cr)  { MovementHandler.SetFormationLinkTarget(cr); };
    RONIN_INLINE Creature* GetFormationLinkTarget() { return MovementHandler.GetFormationLinkTarget(); }

    RONIN_INLINE uint32 GetFormationSQLId() { return MovementHandler.GetFormationSQLId(); };
    RONIN_INLINE void SetFormationSQLId(uint32 Id) { MovementHandler.SetFormationSQLId(Id); }

    RONIN_INLINE uint32 GetFormationFollowAngle() { return MovementHandler.GetFormationFollowAngle(); };
    RONIN_INLINE void SetFormationFollowAngle(float Angle) { MovementHandler.SetFormationFollowAngle(Angle); }

    RONIN_INLINE uint32 GetFormationFollowDistance() { return MovementHandler.GetFormationFollowDistance(); };
    RONIN_INLINE void SetFormationFollowDistance(float Dist) { MovementHandler.SetFormationFollowDistance(Dist); }

    RONIN_INLINE uint32 getCurrentWaypoint() { return MovementHandler.getCurrentWaypoint(); }
    RONIN_INLINE void setWaypointToMove(uint32 id) { return MovementHandler.setWaypointToMove(id); }

    RONIN_INLINE uint32 GetWayPointsCount() { return MovementHandler.GetWayPointsCount(); };
    RONIN_INLINE void SetWaypointMap(WayPointMap * m) { MovementHandler.SetWaypointMap(m); };

    RONIN_INLINE bool addWayPoint(WayPoint* wp) { return MovementHandler.addWayPoint(wp); };
    RONIN_INLINE WayPoint* getWayPoint(uint32 wpid) { return MovementHandler.getWayPoint(wpid); };

    RONIN_INLINE void deleteAllWaypoints() { MovementHandler.deleteAllWaypoints(); };
    RONIN_INLINE void deleteWayPoint(uint32 id) { MovementHandler.deleteWayPoint(id); };

    RONIN_INLINE void changeWayPointID(uint32 oldwpid, uint32 newwpid) { MovementHandler.changeWayPointID(oldwpid, newwpid); };

    RONIN_INLINE bool WayPointsShowing() { return MovementHandler.WayPointsShowing(); };
    RONIN_INLINE void SetWayPointsShowing(bool s) { MovementHandler.SetWayPointsShowing(s); };
    RONIN_INLINE bool WayPointsShowingBackwards() { return MovementHandler.WayPointsShowingBackwards(); };
    RONIN_INLINE void SetWayPointsShowingBackwards(bool bs) { MovementHandler.SetWayPointsShowingBackwards(bs); };

    RONIN_INLINE bool hasWaypoints() { return MovementHandler.hasWaypoints(); };
    RONIN_INLINE bool saveWayPoints() { return MovementHandler.saveWayPoints(); };
    RONIN_INLINE bool hideWayPoints(Player* pPlayer) { return MovementHandler.hideWayPoints(pPlayer); };
    RONIN_INLINE bool showWayPoints(Player* pPlayer, bool Backwards) { return MovementHandler.showWayPoints(pPlayer, Backwards); };
};
