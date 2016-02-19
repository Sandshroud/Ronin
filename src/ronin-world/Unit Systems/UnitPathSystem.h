/***
 * Demonstrike Core
 */

#pragma once

struct MovementPoint
{
    MovementPoint() : pos(0.f, 0.f, 0.f), timeStamp(0) {}
    MovementPoint(uint32 time, float x, float y, float z) : pos(x, y, z), timeStamp(time) {}
    Position pos;
    uint32 timeStamp;
};

class UnitPathSystem
{
public:
    UnitPathSystem(Unit *unit);
    ~UnitPathSystem();

    void Update(uint32 msTime, uint32 uiDiff);

private:
    void _CleanupPath();

public:
    void MoveToPoint(float x, float y, float z, float o);
    void ResumeOrStopMoving();

    void BroadcastMovementPacket();
    void SendMovementPacket(Player *plr);

private:
    Unit *m_Unit;

    uint32 m_pathCounter, m_pathStartTime, m_pathLength;

    float _srcX, _srcY, _srcZ;
    float _destX, _destY, _destZ, _destO;

    std::deque<MovementPoint*> m_movementPoints;
};
