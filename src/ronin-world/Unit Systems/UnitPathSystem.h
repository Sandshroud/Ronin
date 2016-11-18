/***
 * Demonstrike Core
 */

#pragma once

enum MovementSpeedTypes : uint8;

struct MovementPoint
{
    MovementPoint() : pos(0.f, 0.f, 0.f), timeStamp(0) {}
    MovementPoint(uint32 time, float x, float y, float z) : pos(x, y, z), timeStamp(time) {}
    Position pos;
    uint32 timeStamp;
};

class UnitPathSystem
{
public: // Class defines
    static float fInfinite;

public: // Class functions
    UnitPathSystem(Unit *unit);
    ~UnitPathSystem();

    bool Update(uint32 msTime, uint32 uiDiff);

    bool hasDestination();
    bool closeToDestination(uint32 msTime);

    bool GetDestination(float &x, float &y, float *z = NULL);

    void SetSpeed(MovementSpeedTypes speedType);

private:
    void _CleanupPath();

public:
    void MoveToPoint(float x, float y, float z, float o = fInfinite, bool pointOverride = false);
    void StopMoving();

    void BroadcastMovementPacket();
    void SendMovementPacket(Player *plr);

private:
    Unit *m_Unit;

    MovementSpeedTypes _moveSpeed;

    uint32 m_pathCounter, m_pathStartTime, m_pathLength;

    MovementPoint srcPoint;
    float _destX, _destY, _destZ, _destO;

    std::deque<MovementPoint*> m_movementPoints;
};
