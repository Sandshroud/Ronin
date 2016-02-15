
#include "StdAfx.h"

static float fInfinite = std::numeric_limits<float>::infinity();

UnitPathSystem::UnitPathSystem(Unit *unit) : m_pathStartTime(0), m_pathLength(0), _srcX(0.f), _srcY(0.f), _srcZ(0.f), _destX(fInfinite), _destY(fInfinite), _destZ(0.f), _destO(0.f)
{

}

UnitPathSystem::~UnitPathSystem()
{

}

void UnitPathSystem::Update(uint32 msTime, uint32 uiDiff)
{
    if(_destX == fInfinite && _destY == fInfinite)
        return;

    uint32 timeWalked = msTime-m_pathStartTime;
    if(timeWalked == 0)
        return;

    if(timeWalked >= m_pathLength || m_movementPoints.empty())
    {
        m_Unit->SetPosition(_destX,_destY,_destZ,_destO);
        _destX = _destY = fInfinite;
        return;
    }

    MovementPoint *lastPoint = m_movementPoints[0], *nextPoint;
    for(size_t i = 1; i < m_movementPoints.size(); i++)
    {
        if((nextPoint = m_movementPoints[i])->timeStamp >= timeWalked)
        {
            if(nextPoint->timeStamp == timeWalked)
            {   // 
                m_Unit->SetPosition(nextPoint->pos.x, nextPoint->pos.y, nextPoint->pos.z, m_Unit->GetOrientation());
                return;
            }
            break;
        }
        lastPoint = m_movementPoints[i];
    }

    uint32 moveDiff = nextPoint->timeStamp-lastPoint->timeStamp, moveDiff2 = timeWalked-lastPoint->timeStamp, timeLeft = moveDiff-moveDiff2;
    float x = lastPoint->pos.x, y = lastPoint->pos.y, z = lastPoint->pos.z, x2 = nextPoint->pos.x, y2 = nextPoint->pos.y, z2 = nextPoint->pos.z;
    float p = float(timeLeft)/float(moveDiff), px = x2-((x2-x)*p), py = y2-((y2-y)*p), pz = m_Unit->GetMapHeight(px, py, std::max<float>(z, z2));
    m_Unit->SetPosition(px, py, pz, m_Unit->calcAngle(px, py, x2, y2));
}

void UnitPathSystem::MoveToPoint(float x, float y, float z, float o)
{
    sNavMeshInterface.BuildFullPath(m_Unit, m_Unit->GetMapId(), m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ(), x, y, z, true);
}

void UnitPathSystem::ResumeOrStopMoving()
{

}
