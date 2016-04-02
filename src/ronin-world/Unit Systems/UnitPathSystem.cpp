
#include "StdAfx.h"

float UnitPathSystem::fInfinite = std::numeric_limits<float>::infinity();

UnitPathSystem::UnitPathSystem(Unit *unit) : m_Unit(unit), m_pathCounter(0), m_pathStartTime(0), m_pathLength(0), _srcX(0.f), _srcY(0.f), _srcZ(0.f), _destX(fInfinite), _destY(fInfinite), _destZ(0.f), _destO(0.f)
{

}

UnitPathSystem::~UnitPathSystem()
{

}

bool UnitPathSystem::Update(uint32 msTime, uint32 uiDiff)
{
    if(_destX == fInfinite && _destY == fInfinite)
        return true;

    if(uint32 timeWalked = msTime-m_pathStartTime)
    {
        if(timeWalked >= m_pathLength || m_movementPoints.empty())
        {
            m_Unit->SetPosition(_destX,_destY,_destZ,_destO);
            _CleanupPath();
            return true;
        }

        MovementPoint *lastPoint = m_movementPoints[0], *nextPoint;
        for(size_t i = 1; i < m_movementPoints.size(); i++)
        {
            if((nextPoint = m_movementPoints[i])->timeStamp >= timeWalked)
            {
                if(nextPoint->timeStamp == timeWalked)
                {   // 
                    m_Unit->SetPosition(nextPoint->pos.x, nextPoint->pos.y, nextPoint->pos.z, m_Unit->GetOrientation());
                    return false;
                }
                break;
            }
            lastPoint = m_movementPoints[i];
        }

        uint32 moveDiff = nextPoint->timeStamp-lastPoint->timeStamp, moveDiff2 = timeWalked-lastPoint->timeStamp, timeLeft = moveDiff-moveDiff2;
        float x = lastPoint->pos.x, y = lastPoint->pos.y, z = lastPoint->pos.z, x2 = nextPoint->pos.x, y2 = nextPoint->pos.y, z2 = nextPoint->pos.z;
        float p = float(timeLeft)/float(moveDiff), px = x2-((x2-x)*p), py = y2-((y2-y)*p), pz = z2-((z2-z)*p);
        m_Unit->SetPosition(px, py, pz, m_Unit->calcAngle(px, py, x2, y2));
    }
    return false;
}

bool UnitPathSystem::hasDestination() { return !(_destX == fInfinite && _destY == fInfinite); }
bool UnitPathSystem::GetDestination(float &x, float &y, float *z)
{
    if(!hasDestination())
        return false;
    x = _destX;
    y = _destY;
    if(z) *z = _destZ;
    return true;
}

void UnitPathSystem::_CleanupPath()
{
    _destX = _destY = fInfinite;
    while(!m_movementPoints.empty())
    {
        MovementPoint *point = m_movementPoints.front();
        m_movementPoints.pop_front();
        delete point;
    }
}

void UnitPathSystem::MoveToPoint(float x, float y, float z, float o)
{
    if((_destX == x && _destY == y) || (m_Unit->GetPositionX() == x && m_Unit->GetPositionY() == y))
        return;

    // Clean up any existing paths
    _CleanupPath();

    m_pathCounter++;
    m_pathStartTime = getMSTime();
    _destX = x, _destY = y, _destZ = z, _destO = o;
    float speed = m_Unit->GetMoveSpeed(MOVE_SPEED_RUN), dist = sqrtf(m_Unit->GetDistanceSq(x, y, z));

    LocationVector currPos = m_Unit->GetPosition();
    if(sNavMeshInterface.IsNavmeshLoadedAtPosition(m_Unit->GetMapId(), x, y) && sNavMeshInterface.IsNavmeshLoadedAtPosition(m_Unit->GetMapId(), currPos.x, currPos.y))
        sNavMeshInterface.BuildFullPath(m_Unit, m_Unit->GetMapId(), currPos.x, currPos.y, currPos.z, x, y, z, true);
    else
    {
        // Store our starting position
        m_movementPoints.push_back(new MovementPoint(0, currPos.x, currPos.y, currPos.z));

        m_pathLength = (dist/speed)*1000.f;

        // Path calculation
        uint32 timeToMove = 0;
        while((m_pathLength-timeToMove) > 500)
        {
            timeToMove += 500;

            float p = float(timeToMove)/float(m_pathLength), px = currPos.x-((currPos.x-_destX)*p), py = currPos.y-((currPos.y-_destY)*p), pz = (m_Unit->canFly() ? std::max<float>(currPos.z, z) : m_Unit->GetMapHeight(px, py, std::max<float>(currPos.z, z)));
            m_movementPoints.push_back(new MovementPoint(timeToMove, px, py, pz));
        }

        m_movementPoints.push_back(new MovementPoint(m_pathLength, _destX, _destY, _destZ));
    }

    BroadcastMovementPacket();
}

void UnitPathSystem::StopMoving()
{
    _CleanupPath();
    // Set destX/Y to infinite, zero out destZ and Orientation
    _destX = _destY = fInfinite; _destZ = _destO  = 0.f;
    BroadcastMovementPacket();
}

void UnitPathSystem::BroadcastMovementPacket()
{
    WorldPacket data(SMSG_MONSTER_MOVE, 100);
    data << m_Unit->GetGUID().asPacked();
    data << uint8(0);
    data.appendvector(m_Unit->GetPosition(), false);
    // If we are at our destination, or have no destination, broadcast a stop packet
    if((m_Unit->GetPositionX() == _destX && m_Unit->GetPositionY() == _destY) || (_destX == fInfinite && _destY == fInfinite))
        data << uint32(0) << uint8(1);
    else
    {
        data << uint32(m_pathCounter);
        if(_destO == fInfinite) data << uint8(0);
        else data << uint8(4) << float( _destO );
        data << uint32(0x00400000);
        data << uint32(m_pathLength);

        uint32 counter = 0;
        size_t counterPos = data.wpos();
        data << uint32(0); // movement point counter
        for(uint32 i = 1; i < m_movementPoints.size(); i++)
        {
            if(MovementPoint *path = m_movementPoints[i])
            {
                data << path->pos.x << path->pos.y << path->pos.z;
                counter++;
            }
        }
        data.put<uint32>(counterPos, counter);
    }

    m_Unit->SendMessageToSet( &data, false );
}

void UnitPathSystem::SendMovementPacket(Player *plr)
{

}
