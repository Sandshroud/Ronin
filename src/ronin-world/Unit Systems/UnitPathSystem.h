/***
 * Demonstrike Core
 */

#pragma once

struct MovementPoint
{
    Position pos;
    uint32 timeStamp;
};

class UnitPathSystem
{
public:
    UnitPathSystem(Unit *unit);
    ~UnitPathSystem();

    void Update(uint32 msTime, uint32 uiDiff);

    void MoveToPoint(float x, float y, float z, float o);
    void ResumeOrStopMoving();

private:
    Unit *m_Unit;

    uint32 m_pathStartTime, m_pathLength;

    float _srcX, _srcY, _srcZ;
    float _destX, _destY, _destZ, _destO;

    std::vector<MovementPoint*> m_movementPoints;
};
