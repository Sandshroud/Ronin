
#include "StdAfx.h"

float UnitPathSystem::fInfinite = std::numeric_limits<float>::infinity();

UnitPathSystem::UnitPathSystem(Unit *unit) : m_Unit(unit), m_pathCounter(0), m_pathStartTime(0), m_pathLength(0), srcPoint(), _destX(fInfinite), _destY(fInfinite), _destZ(0.f), _destO(0.f)
{

}

UnitPathSystem::~UnitPathSystem()
{
    _CleanupPath();
}

bool UnitPathSystem::Update(uint32 msTime, uint32 uiDiff)
{
    if(_destX == fInfinite && _destY == fInfinite)
        return true;

    if(uint32 timeWalked = msTime-m_pathStartTime)
    {
        if(timeWalked >= m_pathLength || m_movementPoints.empty())
        {
            m_Unit->GetMovementInterface()->MoveClientPosition(_destX,_destY,_destZ,_destO);
            _CleanupPath();
            return true;
        }

        MovementPoint *lastPoint = &srcPoint, *nextPoint = m_movementPoints.front();
        if(timeWalked >= nextPoint->timeStamp && m_movementPoints.size() != 1)
        {
            lastPoint = nextPoint;
            while(m_movementPoints.size() >= 2)
            {
                if((nextPoint = m_movementPoints.at(1))->timeStamp >= timeWalked)
                {
                    if(nextPoint->timeStamp == timeWalked)
                    {   // 
                        m_Unit->GetMovementInterface()->MoveClientPosition(nextPoint->pos.x, nextPoint->pos.y, nextPoint->pos.z, m_Unit->GetOrientation());
                        return false;
                    }
                    break;
                }

                // Remove the last point since we don't need it
                m_movementPoints.pop_front();
                delete lastPoint;
                // Get the new last point
                lastPoint = nextPoint;
                nextPoint = NULL;
            }
        }

        if(nextPoint == NULL) // No next point means we've cleared up our movement path
        {
            m_Unit->GetMovementInterface()->MoveClientPosition(_destX,_destY,_destZ,_destO);
            _CleanupPath();
            return true;
        }

        // Calculate the time percentage of movement between our two points that we've moved so far
        uint32 moveDiff = nextPoint->timeStamp-lastPoint->timeStamp, moveDiff2 = timeWalked-lastPoint->timeStamp, timeLeft = moveDiff-moveDiff2;
        float x = lastPoint->pos.x, y = lastPoint->pos.y, z = lastPoint->pos.z, x2 = nextPoint->pos.x, y2 = nextPoint->pos.y, z2 = nextPoint->pos.z;
        float p = float(timeLeft)/float(moveDiff), px = x2-((x2-x)*p), py = y2-((y2-y)*p), pz = z2-((z2-z)*p);
        // Update unit client position, post update heartbeat will reset unit position for us
        m_Unit->GetMovementInterface()->MoveClientPosition(px, py, pz, m_Unit->calcAngle(px, py, x2, y2));
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

bool UnitPathSystem::closeToDestination(uint32 msTime)
{   // Creatures update every 400ms, should be changed to be within the 500ms block creation
    if(((msTime-m_pathStartTime) + 400) >= m_pathLength)
        return true;
    return false;
}

void UnitPathSystem::SetSpeed(MovementSpeedTypes speedType)
{
    _moveSpeed = speedType;
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

void UnitPathSystem::MoveToPoint(float x, float y, float z, float o, bool pointOverride)
{
    if((_destX == x && _destY == y) || (m_Unit->GetPositionX() == x && m_Unit->GetPositionY() == y))
        return;

    uint32 timeNeededToFinishBackups = 0;
    std::vector<MovementPoint> m_backupPoints;
    if(pointOverride == false && !m_movementPoints.empty())
    {
        uint32 timeWalked = getMSTime()-m_pathStartTime;
        if(timeWalked < m_pathLength)
        {   // TODO: We only really need the target point and the time it takes to get there
            for(size_t i = 0; i < m_movementPoints.size(); i++)
            {
                if(timeWalked >= m_movementPoints[i]->timeStamp)
                    continue;

                // Create a new temporary point for reinsertion
                MovementPoint point(*m_movementPoints[i]);
                // Save our position
                point.pos = m_movementPoints[i]->pos;
                // New timestamp
                timeNeededToFinishBackups = (point.timeStamp = m_movementPoints[i]->timeStamp-timeWalked);
                // store
                m_backupPoints.push_back(point);
            }
        }
    }

    // Clean up any existing paths
    _CleanupPath();

    m_pathCounter++;
    m_pathStartTime = getMSTime();
    m_Unit->GetPosition(srcPoint.pos.x, srcPoint.pos.y, srcPoint.pos.z);

    _destX = x, _destY = y, _destZ = z, _destO = o;
    float speed = m_Unit->GetMoveSpeed(_moveSpeed), dist = sqrtf(m_Unit->GetDistanceSq(x, y, z));

    if(sNavMeshInterface.IsNavmeshLoadedAtPosition(m_Unit->GetMapId(), x, y) && sNavMeshInterface.IsNavmeshLoadedAtPosition(m_Unit->GetMapId(), srcPoint.pos.x, srcPoint.pos.y))
        sNavMeshInterface.BuildFullPath(m_Unit, m_Unit->GetMapId(), srcPoint.pos.x, srcPoint.pos.y, srcPoint.pos.z, x, y, z, true);
    else
    {
        MovementPoint *lastPoint = &srcPoint; // Store our starting position
        m_pathLength = (dist/speed)*1000.f;

        if(m_backupPoints.size())
        {
            m_pathLength += timeNeededToFinishBackups;
            for(std::vector<MovementPoint>::iterator itr = m_backupPoints.begin(); itr != m_backupPoints.end(); itr++)
                m_movementPoints.push_back(new MovementPoint((*itr).timeStamp, (*itr).pos.x, (*itr).pos.y, (*itr).pos.z));
            m_backupPoints.clear();
        }

        bool ignoreTerrainHeight = m_Unit->canFly();
        float terrainHeight = m_Unit->GetMapInstance()->GetLandHeight(srcPoint.pos.x, srcPoint.pos.y), targetTHeight = m_Unit->GetMapInstance()->GetLandHeight(_destX, _destY), posToAdd = 0.f;
        if(ignoreTerrainHeight == false)
            posToAdd = ((z-srcPoint.pos.z)/(((float)m_pathLength)/500.f));
        else posToAdd = ((targetTHeight-terrainHeight)/(((float)m_pathLength)/500.f));

        float lastCalcPoint = lastPoint->pos.z;// Path calculation
        uint32 timeToMove = timeNeededToFinishBackups;
        while((m_pathLength-timeToMove) > 500)
        {
            timeToMove += 500;
            lastCalcPoint += posToAdd;

            float targetZ = lastCalcPoint;
            float p = float(timeToMove)/float(m_pathLength), px = srcPoint.pos.x-((srcPoint.pos.x-_destX)*p), py = srcPoint.pos.y-((srcPoint.pos.y-_destY)*p);
            if(ignoreTerrainHeight == false)
            {
                terrainHeight = m_Unit->GetMapHeight(px, py, std::max<float>(srcPoint.pos.z, z));
                if(targetZ < terrainHeight)
                    targetZ = terrainHeight;
            }

            m_movementPoints.push_back(lastPoint = new MovementPoint(timeToMove, px, py, targetZ));
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
    if(!m_Unit->HasInRangePlayers())
        return;

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
    uint32 moveTime = getMSTime()-m_pathStartTime;
    if((m_Unit->GetPositionX() == _destX && m_Unit->GetPositionY() == _destY) || (_destX == fInfinite && _destY == fInfinite) || (moveTime >= m_pathLength))
        return;

    WorldPacket data(SMSG_MONSTER_MOVE, 100);
    data << m_Unit->GetGUID().asPacked();
    data << uint8(0);
    data.appendvector(m_Unit->GetPosition(), false);
    data << uint32(m_pathCounter);
    if(_destO == fInfinite) data << uint8(0);
    else data << uint8(4) << float( _destO );
    data << uint32(0x00400000);
    data << uint32(m_pathLength - moveTime);

    uint32 counter = 0;
    size_t counterPos = data.wpos();
    data << uint32(0); // movement point counter
    for(uint32 i = 1; i < m_movementPoints.size(); i++)
    {
        if(MovementPoint *path = m_movementPoints[i])
        {
            if(path->timeStamp <= moveTime)
                continue;

            data << path->pos.x << path->pos.y << path->pos.z;
            counter++;
        }
    }

    if(counter == 0)
        return;

    data.put<uint32>(counterPos, counter);
    plr->PushPacket(&data);
}
