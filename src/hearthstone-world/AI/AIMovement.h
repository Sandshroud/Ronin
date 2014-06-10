/***
 * Demonstrike Core
 */

#pragma once

class SERVER_DECL AI_Movement
{
    friend class AIInterface;
public:
    AI_Movement();

    void Nullify();
    void Initialize(AIInterface*, Unit*, MovementType);
    void DeInitialize();

    // Update from Interface
    void Update(uint32 p_time);

    // Movement
    void SendMoveToPacket(Player* playerTarget = NULL);
    void SendJumpTo(float toX, float toY, float toZ, uint32 moveTime, float arc , uint32 unk = 0);
    void SendMoveToPacket(float toX, float toY, float toZ, float toO, uint32 time, uint32 MoveFlags, Player* playerTarget = NULL);

    uint32 getMoveFlags(bool ignorejump = false);
    void MoveTo(float x, float y, float z, float o = 0.0f, bool IgnorePathMap = false);

    void UpdateMove();
    void SendCurrentMove(Player* plyr/*uint64 guid*/);
    void StopMovement(uint32 time, bool stopatcurrent = true);

    void changeWayPointID(uint32 oldwpid, uint32 newwpid);

    HEARTHSTONE_INLINE void setWaypointToMove(uint32 id) { m_CurrentWayPoint = id; }
    HEARTHSTONE_INLINE uint32 getCurrentWaypoint() { return m_CurrentWayPoint; }

    bool saveWayPoints();
    bool addWayPoint(WayPoint* wp);
    bool hideWayPoints(Player* pPlayer);
    bool showWayPoints(Player* pPlayer, bool Backwards);

    HEARTHSTONE_INLINE void SetWaypointMap(WayPointMap * m) { m_waypoints = m; }

    WayPoint* getWayPoint(uint32 wpid);

    void deleteAllWaypoints();
    void deleteWayPoint(uint32 wpid);

    void HandleEvade();
    void EventEnterCombat(uint32 misc1);
    void _CalcDestinationAndMove( Unit* target, float dist);

    HEARTHSTONE_INLINE void ResetLastFollowPos() { m_lastFollowX = m_lastFollowY = 0.0f; };

    HEARTHSTONE_INLINE void SetFormationLinkTarget(Creature* cr)  { m_FormationLinkTarget = cr; };
    HEARTHSTONE_INLINE Creature* GetFormationLinkTarget() { return m_FormationLinkTarget; }

    HEARTHSTONE_INLINE uint32 GetFormationSQLId() { return m_FormationLinkSqlId; };
    HEARTHSTONE_INLINE void SetFormationSQLId(uint32 Id) { m_FormationLinkSqlId = Id; }

    HEARTHSTONE_INLINE float GetFormationFollowAngle() { return m_FormationFollowAngle; };
    HEARTHSTONE_INLINE void SetFormationFollowAngle(float Angle) { m_FormationFollowAngle = Angle; }

    HEARTHSTONE_INLINE float GetFormationFollowDistance() { return m_FormationFollowDistance; };
    HEARTHSTONE_INLINE void SetFormationFollowDistance(float Dist) { m_FormationFollowDistance = Dist; }

    HEARTHSTONE_INLINE float GetFollowDistance() { return FollowDistance; };
    HEARTHSTONE_INLINE void SetFollowDistance(float dist) { FollowDistance = dist; };

    HEARTHSTONE_INLINE float GetUnitToFollowAngle() { return m_FollowAngle; }
    HEARTHSTONE_INLINE void SetUnitToFollowAngle(float angle) { m_FollowAngle = angle; }

    HEARTHSTONE_INLINE void SetUnitToFear(Unit* un)  { UnitToFear = un; };
    HEARTHSTONE_INLINE Unit* getUnitToFear() { return UnitToFear; }

    HEARTHSTONE_INLINE void SetUnitToFollow(Unit* un) { UnitToFollow = un; };
    HEARTHSTONE_INLINE Unit* getUnitToFollow() { return UnitToFollow; }

    HEARTHSTONE_INLINE void SetBackupUnitToFollow(Unit* un) { UnitToFollow_backup = un; };
    HEARTHSTONE_INLINE Unit* getBackupUnitToFollow() { return UnitToFollow_backup; }

    HEARTHSTONE_INLINE void AddStopTime(uint32 Time) { m_moveTimer += Time; }
    HEARTHSTONE_INLINE bool hasWaypoints() { return m_waypoints != NULL; }

    HEARTHSTONE_INLINE void setMoveType(uint32 m) { m_moveType = m; }
    HEARTHSTONE_INLINE uint32 getMoveType() { return m_moveType; }

    HEARTHSTONE_INLINE void setMoveRunFlag(bool f) { m_moveRun = f; }
    HEARTHSTONE_INLINE bool getMoveRunFlag() { return m_moveRun; }

    HEARTHSTONE_INLINE void setMoveFlyFlag(bool f) { m_moveFly = f; }
    HEARTHSTONE_INLINE bool getMoveFlyFlag() { return m_moveFly; }

    HEARTHSTONE_INLINE void setMoveSprintFlag(bool f) { m_moveSprint = f; }
    HEARTHSTONE_INLINE bool getMoveSprintFlag() { return m_moveSprint; }

    HEARTHSTONE_INLINE void setCanMove(bool c) { m_canMove = c; }
    HEARTHSTONE_INLINE bool getCanMove() { return m_canMove; }

    HEARTHSTONE_INLINE void setFearTimer(uint32 time) { m_FearTimer = time; }
    HEARTHSTONE_INLINE uint32 getFearTimer() { return m_FearTimer; }

    HEARTHSTONE_INLINE void setWanderTimer(uint32 time) { m_WanderTimer = time; }
    HEARTHSTONE_INLINE uint32 getWanderTimer() { return m_WanderTimer; }

    HEARTHSTONE_INLINE bool WayPointsShowing() { return m_WayPointsShowing; };
    HEARTHSTONE_INLINE void SetWayPointsShowing(bool s) { m_WayPointsShowing = s; };
    HEARTHSTONE_INLINE bool WayPointsShowingBackwards() { return m_WayPointsShowBackwards; };
    HEARTHSTONE_INLINE void SetWayPointsShowingBackwards(bool bs) { m_WayPointsShowBackwards = bs; };

    HEARTHSTONE_INLINE void ClearFollowInformation(Unit* m_un = NULL)
    {
        if(m_un == NULL)
        {
            UnitToFollow_backup = UnitToFollow = NULL;
            FollowDistance_backup = FollowDistance = 0.0f;
            ResetLastFollowPos();
        }
        else
        {
            if(m_un == UnitToFollow_backup)
            {
                UnitToFollow_backup = NULL;
                FollowDistance_backup = 0.0f;
            }

            if(m_un == UnitToFollow)
            {
                UnitToFollow = NULL;
                FollowDistance = 0.0f;
                ResetLastFollowPos();
            }
        }
    };

    HEARTHSTONE_INLINE void RestoreFollowInformation()
    {
        UnitToFollow = UnitToFollow_backup;
        FollowDistance = FollowDistance_backup;
    };

    HEARTHSTONE_INLINE void BackupFollowInformation()
    {
        UnitToFollow_backup = UnitToFollow;
        FollowDistance_backup = FollowDistance;
    };

    bool IsFlying();

    HEARTHSTONE_INLINE bool HasDestination()
    {
        if((!m_ignorePathMap && PathMap != NULL) || m_pathfinding)
            return ((m_nextPosX != 0.0f || m_nextPosY != 0.0f) && (m_nextPosX != m_sourceX && m_nextPosY != m_sourceY));
        return ((m_destinationX != 0.0f || m_destinationY != 0.0f) && (m_destinationX != m_sourceX && m_destinationY != m_sourceY));
    }

    HEARTHSTONE_INLINE LocationVector GetDestPos()
    {
        if((!m_ignorePathMap && PathMap != NULL) || m_pathfinding)
            return LocationVector(m_nextPosX, m_nextPosY, m_nextPosZ);
        return LocationVector(m_destinationX, m_destinationY, m_destinationZ);
    }

    HEARTHSTONE_INLINE LocationVector GetReturnPos() { return LocationVector(m_returnX, m_returnY, m_returnZ, m_returnO); }
    HEARTHSTONE_INLINE LocationVector GetNextPos() { return LocationVector(m_nextPosX, m_nextPosY, m_nextPosZ, m_nextPosO); }

    float GetMovementTime(float distance);
    HEARTHSTONE_INLINE void SetSourcePos(float x, float y, float z) { m_sourceX = x; m_sourceY = y; m_sourceZ = z; };
    HEARTHSTONE_INLINE void SetReturnPos(float x, float y, float z, float o = 0.0f) { m_returnX = x; m_returnY = y; m_returnZ = z; m_returnO = o; };
    HEARTHSTONE_INLINE void SetNextPos(float x, float y, float z, float o = 0.0f) { m_nextPosX = x; m_nextPosY = y; m_nextPosZ = z; m_nextPosO = o; };
    HEARTHSTONE_INLINE void SetDestPos(float x, float y, float z, float o = 0.0f) { m_destinationX = x; m_destinationY = y; m_destinationZ = z; m_destinationO = o; };

private:
    std::map<uint32, uint32> CurrentMoveRequired;

    float m_walkSpeed, m_runSpeed, m_flySpeed;

    float m_sourceX, m_sourceY, m_sourceZ;
    float m_returnX, m_returnY, m_returnZ, m_returnO; //Return position after attacking a mob
    float m_nextPosX, m_nextPosY, m_nextPosZ, m_nextPosO;
    float m_destinationX, m_destinationY, m_destinationZ, m_destinationO;

    bool m_pathfinding;
    bool m_ignorePathMap;
    LocationVectorMapContainer* PathMap;

    // Movement
    uint32 m_moveType;
    float m_lastFollowX, m_lastFollowY;

    Unit *UnitToFear;
    Unit *UnitToFollow, *UnitToFollow_backup;

    uint32 m_FearTimer, m_WanderTimer;
    uint32 m_timeToMove, m_timeMoved, m_moveTimer, m_totalMoveTime;

    MovementType m_MovementType;
    MovementState m_MovementState;

    bool m_canMove;
    bool m_moveRun;
    bool m_moveFly;
    bool m_moveJump;
    bool m_moveSprint;
    bool m_moveBackward;
    bool m_WayPointsShowing;
    bool m_WayPointsShowBackwards;

    float FollowDistance, FollowDistance_backup, m_FollowAngle;

    // Waypoint
    uint32 m_CurrentWayPoint;
    WayPointMap *m_waypoints;
    uint32 GetWayPointsCount()
    {
        if(m_waypoints)
            return uint32(m_waypoints->size());
        else
            return 0;
    }

    // Formation
    uint32 m_FormationLinkSqlId;
    Creature *m_FormationLinkTarget;
    float m_FormationFollowAngle, m_FormationFollowDistance;

protected:
    Unit* m_Unit;
    AIInterface* Interface;

public:
    AIType getAIType();
    void SetAIType(AIType newtype);

    uint8 getAIState();
    void SetAIState(AI_State newstate);

    CreatureState getCreatureState();
    void setCreatureState(CreatureState state);
};
