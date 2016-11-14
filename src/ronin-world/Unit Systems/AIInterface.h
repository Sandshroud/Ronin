/***
 * Demonstrike Core
 */

#pragma once

class CreatureData;
class CreatureInfoExtra;
class UnitPathSystem;

#define MAX_RANDOM_MOVEMENT_DIST 25.f
#define MAX_COMBAT_MOVEMENT_DIST 14400.f

enum AI_State : uint8
{
    AI_STATE_DEAD   = 0,
    AI_STATE_IDLE   = 1,
    AI_STATE_COMBAT = 2,
    AI_STATE_SCRIPT = 0xFF
};

enum AI_Flags : uint16
{
    AI_FLAG_NONE        = 0x0000,
    AI_FLAG_DISABLED    = 0x8000
};

class SERVER_DECL AIInterface
{
public:
    AIInterface(Creature *creature, UnitPathSystem *unitPath, Unit *owner = NULL);
    ~AIInterface();

    // Post creation initialization call
    void Init();
    void SetWaypoints(WaypointStorage *waypoints);

    // Update
    void Update(uint32 msTime, uint32 p_time);

    void OnAttackStop();
    void OnDeath();
    void OnRespawn();
    void OnPathChange();
    void OnTakeDamage(Unit *attacker, uint32 damage);

    bool FindTarget();
    void FindNextPoint();

    AI_State GetAIState() { return m_AIState; }

    uint32 m_pendingWaitTimer;

protected:
    AI_State m_AIState;
    uint32 m_AISeed;
    uint32 m_AIFlags;

    uint32 m_waypointWaitTimer;
    void _HandleCombatAI();

protected:
    WaypointStorage *m_waypointMap;
    WaypointStorage::iterator waypointIterator;

private:
    Creature* m_Creature;
    UnitPathSystem *m_path;

    WoWGuid m_targetGuid;
    std::map<WoWGuid, uint32> m_threatMap;
};
