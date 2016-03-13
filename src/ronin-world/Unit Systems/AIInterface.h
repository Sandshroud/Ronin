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

class SERVER_DECL AIInterface
{
public:
    AIInterface(Unit *unit, UnitPathSystem *unitPath, Unit *owner = NULL);
    ~AIInterface();

    // Update
    void Update(uint32 p_time);

    void OnDeath();

    bool FindTarget();
    void FindNextPoint();

    AI_State GetAIState() { return m_AIState; }

protected:
    AI_State m_AIState;

protected:
    uint32 m_waypointCounter;
    std::map<uint32, Position> *m_waypointMap;

private:
    Unit* m_Unit;
    UnitPathSystem *m_path;

    WoWGuid m_targetGuid;
    std::map<WoWGuid, uint32> m_threatMap;
};
