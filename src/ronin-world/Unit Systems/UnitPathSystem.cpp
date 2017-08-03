
#include "StdAfx.h"

float UnitPathSystem::fInfinite = std::numeric_limits<float>::infinity();

UnitPathSystem::UnitPathSystem(Unit *unit) : m_Unit(unit), m_autoPath(false), _waypointPath(NULL), m_autoPathDelay(0),
m_pendingAutoPathDelay(0), m_pathCounter(0), m_pathStartTime(0), m_pathLength(0), m_lastMSTimeUpdate(0), m_lastPositionUpdate(0),
srcPoint(), _destX(fInfinite), _destY(fInfinite), _destZ(0.f), _destO(0.f)
{

}

UnitPathSystem::~UnitPathSystem()
{
    _CleanupPath();
}

bool UnitPathSystem::IsActiveObject()
{
    return m_Unit->IsActiveObject();
}

bool UnitPathSystem::IsActivated()
{
    return m_Unit->IsActivated();
}

void UnitPathSystem::InactiveUpdate(uint32 msTime, uint32 uiDiff)
{
    // Dunno, we don't need to process this atm
}

bool UnitPathSystem::Update(uint32 msTime, uint32 uiDiff, bool fromMovement)
{
    // If it's the same mstime(same world tick), return
    if(m_lastMSTimeUpdate >= msTime)
    {
        if(fromMovement && hasDestination()) // Update our current position to our previously calculated update point
        {   // Note that only movement cares what our return value is
            m_Unit->GetMovementInterface()->MoveClientPosition(lastUpdatePoint.pos.x, lastUpdatePoint.pos.y, lastUpdatePoint.pos.z, lastUpdatePoint.orientationOverride);
            return false;
        }
        return true;
    }

    // Update ms timer
    m_lastMSTimeUpdate = msTime;

    if(m_autoPathDelay > uiDiff)
        m_autoPathDelay -= uiDiff;
    else m_autoPathDelay = 0;

    // Calculate our new position if we have a destination
    if(hasDestination())
    {
        if(msTime <= m_pathStartTime)
            return false;
        uint32 timeWalked = msTime-m_pathStartTime;
        if(timeWalked >= m_pathLength || !m_movementPoints.HasItems())
        {
            m_Unit->GetMovementInterface()->MoveClientPosition(_destX,_destY,_destZ,_destO);
            _CleanupPath();
        }
        else
        {
            // Move towards destination
            MovementPoint *lastPoint = &srcPoint, *nextPoint = m_movementPoints.front().get();
            if(timeWalked >= nextPoint->timeStamp && m_movementPoints.size() != 1)
            {
                lastPoint = nextPoint;
                while(m_movementPoints.size() >= 2)
                {
                    if((nextPoint = m_movementPoints.at(1).get())->timeStamp >= timeWalked)
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
            float p = float(timeLeft)/float(moveDiff), x = lastPoint->pos.x, y = lastPoint->pos.y, z = lastPoint->pos.z, x2 = nextPoint->pos.x, y2 = nextPoint->pos.y, z2 = nextPoint->pos.z;
            // Update our last update point
            lastUpdatePoint.timeStamp = timeWalked;
            lastUpdatePoint.pos.x = x2-((x2-x)*p);
            lastUpdatePoint.pos.y = y2-((y2-y)*p);
            lastUpdatePoint.pos.z = z2-((z2-z)*p);
            lastUpdatePoint.orientationOverride = m_Unit->calcRadAngle(lastUpdatePoint.pos.x, lastUpdatePoint.pos.y, x2, y2);
            // If we're from our movement interface, then pop to our new position
            if(fromMovement) // Update unit client position, post update heartbeat will reset unit position for us
                m_Unit->GetMovementInterface()->MoveClientPosition(lastUpdatePoint.pos.x, lastUpdatePoint.pos.y, lastUpdatePoint.pos.z, lastUpdatePoint.orientationOverride);
            return false;
        }
    }

    if(m_autoPath)
    {   //Process next steps of path
        if(m_autoPathDelay)
            return false;
        else if(m_autoPathDelay = m_pendingAutoPathDelay)
        {
            m_pendingAutoPathDelay = 0;
            return false;
        }

        if(_waypointPath && !_waypointPath->empty())
        {
            if(pathIterator == _waypointPath->end() || ++pathIterator == _waypointPath->end())
                pathIterator = _waypointPath->begin();

            CreatureWaypoint *point = pathIterator->second;
            switch(point->moveType)
            {
            case 0: SetSpeed(MOVE_SPEED_WALK); break;
            case 1: SetSpeed(MOVE_SPEED_RUN); break;
            case 2: SetSpeed(MOVE_SPEED_FLIGHT); break;
            }

            float x = point->x, y = point->y, z = point->z, o = point->o;
            uint32 delay = point->delay;

            // Grab next point
            WaypointStorage::iterator itr = pathIterator;
            if(itr == _waypointPath->end() || ++itr == _waypointPath->end())
                itr = _waypointPath->begin();
            point = itr->second;
            if(delay <= 500) // Calculate a new angle
                o = m_Unit->calcRadAngle(x, y, point->x, point->y);

            MoveToPoint(x, y, z, o);
            m_pendingAutoPathDelay = delay;
        }
    }

    return true;
}

void UnitPathSystem::EnterEvade()
{
    m_autoPath = false;
    if(_waypointPath == NULL)
    {
        MoveToPoint(m_Unit->GetSpawnX(), m_Unit->GetSpawnY(), m_Unit->GetSpawnZ(), m_Unit->GetSpawnO());
        return;
    }

}

void UnitPathSystem::SetAutoPath(WaypointStorage *storage)
{
    if(storage == NULL || storage->empty())
        return;

    m_autoPath = true;
    pathIterator = (_waypointPath = storage)->begin();
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
    m_movementPoints.Clear();

    lastUpdatePoint.timeStamp = 0; // Clean up our last update point
    lastUpdatePoint.pos.x = lastUpdatePoint.pos.y = lastUpdatePoint.pos.z = fInfinite;
}

uint32 UnitPathSystem::buildMonsterMoveFlags(uint8 packetSendFlags)
{
    if(packetSendFlags & MOVEBCFLAG_UNCOMP)
        return 0x00400000;

    // No monster flags required
    return 0;
}

void UnitPathSystem::SetFollowTarget(Unit *target, float distance)
{

}

void UnitPathSystem::MoveToPoint(float x, float y, float z, float o)
{
    if((_destX == x && _destY == y) || (m_Unit->GetPositionX() == x && m_Unit->GetPositionY() == y))
        return;

    // Clean up any existing paths
    _CleanupPath();

    m_pathCounter++;
    m_lastMSTimeUpdate = m_pathStartTime = getMSTime();
    m_Unit->GetPosition(srcPoint.pos.x, srcPoint.pos.y, srcPoint.pos.z);

    // Set our last update point data to our source with no move time
    lastUpdatePoint.timeStamp = 0;
    m_Unit->GetPosition(lastUpdatePoint.pos.x, lastUpdatePoint.pos.y, lastUpdatePoint.pos.z);

    _destX = x, _destY = y, _destZ = z, _destO = o;

    if(sNavMeshInterface.IsNavmeshLoadedAtPosition(m_Unit->GetMapId(), x, y) && sNavMeshInterface.IsNavmeshLoadedAtPosition(m_Unit->GetMapId(), srcPoint.pos.x, srcPoint.pos.y))
        sNavMeshInterface.BuildFullPath(m_Unit, m_Unit->GetMapId(), srcPoint.pos.x, srcPoint.pos.y, srcPoint.pos.z, x, y, z, true);
    else
    {
        MapInstance *instance = m_Unit->GetMapInstance();
        float speed = m_Unit->GetMoveSpeed(_moveSpeed), dist = sqrtf(m_Unit->GetDistanceSq(x, y, z));

        m_pathLength = (dist/speed)*1000.f;
        if(m_pathLength > 800)
        {
            bool ignoreTerrainHeight = m_Unit->canFly();
            float maxZ = std::max<float>(srcPoint.pos.z, _destZ);
            float terrainHeight = m_Unit->GetGroundHeight(), targetTHeight = instance->GetWalkableHeight(m_Unit, _destX, _destY, _destZ), posToAdd = 0.f;
            if(ignoreTerrainHeight)
                posToAdd = ((_destZ-srcPoint.pos.z)/(((float)m_pathLength)/500.f));
            else posToAdd = ((targetTHeight-terrainHeight)/(((float)m_pathLength)/500.f));

            float lastCalcPoint = srcPoint.pos.z;// Path calculation
            uint32 timeToMove = 500;
            while((m_pathLength-timeToMove) > 500)
            {
                timeToMove += 500;
                lastCalcPoint += posToAdd;

                float p = float(timeToMove)/float(m_pathLength), px = srcPoint.pos.x-((srcPoint.pos.x-_destX)*p), py = srcPoint.pos.y-((srcPoint.pos.y-_destY)*p);
                float targetZ = instance->GetWalkableHeight(m_Unit, px, py, maxZ);
                if(ignoreTerrainHeight && lastCalcPoint > targetZ)
                    targetZ = lastCalcPoint;

                m_movementPoints.Push(std::move(std::shared_ptr<MovementPoint>(new MovementPoint(timeToMove, px, py, targetZ))));
            }
        }

        m_movementPoints.Push(std::move(std::shared_ptr<MovementPoint>(new MovementPoint(m_pathLength, _destX, _destY, _destZ))));
    }

    BroadcastMovementPacket();
}

void UnitPathSystem::UpdateOrientation(Unit *unitTarget)
{
    float angle = NormAngle(m_Unit->GetAngle(unitTarget));
    if((m_Unit->GetPositionX() == _destX && m_Unit->GetPositionY() == _destY) || (_destX == fInfinite && _destY == fInfinite) || (lastUpdatePoint.timeStamp >= m_pathLength))
    {
        SetOrientation(angle);
        return;
    }

    _destO = angle;

    // Broadcast a movement change
    WorldPacket data(SMSG_MONSTER_MOVE, 100);
    data << m_Unit->GetGUID().asPacked();
    data << uint8(0);
    data.appendvector(LocationVector(lastUpdatePoint.pos.x, lastUpdatePoint.pos.y, lastUpdatePoint.pos.z), false);
    // If we are at our destination, or have no destination, broadcast a stop packet
    if((m_Unit->GetPositionX() == _destX && m_Unit->GetPositionY() == _destY) || (_destX == fInfinite && _destY == fInfinite))
        data << uint32(0) << uint8(5) << float( _destO );
    else
    {
        data << uint32(m_pathCounter);
        data << uint8(4) << float( _destO );
        data << uint32(buildMonsterMoveFlags(MOVEBCFLAG_UNCOMP));
        data << uint32(m_pathLength - lastUpdatePoint.timeStamp);

        uint32 counter = 0;
        size_t counterPos = data.wpos();
        data << uint32(0); // movement point counter
        for(uint32 i = 0; i < m_movementPoints.size(); i++)
        {
            if(MovementPoint *path = m_movementPoints.at(i).get())
            {
                if(path->timeStamp <= lastUpdatePoint.timeStamp)
                    continue;

                data << path->pos.x << path->pos.y << path->pos.z;
                counter++;
            }
        }
        data.put<uint32>(counterPos, counter);
    }

    m_Unit->SendMessageToSet( &data, false );
}

void UnitPathSystem::SetOrientation(float orientation)
{
    // Only update if we need to
    if(RONIN_UTIL::fuzzyEq(orientation, m_Unit->GetOrientation()))
        return;

    m_pathCounter++;
    m_Unit->SetOrientation(orientation);

    LocationVector *pos = m_Unit->GetPositionV();
    WorldPacket data(SMSG_MONSTER_MOVE, 100);
    data << m_Unit->GetGUID().asPacked();
    data << uint8(0);
    data.appendvector(*pos);
    data << uint32(m_pathCounter);
    data << uint8(4) << float( m_Unit->GetOrientation() );
    data << uint32(buildMonsterMoveFlags(MOVEBCFLAG_UNCOMP)) << uint32(0) << uint32(1);
    data.appendvector(*pos);
    m_Unit->SendMessageToSet( &data, false );
}

void UnitPathSystem::StopMoving()
{
    _CleanupPath();
    // Set destX/Y to infinite, zero out destZ and Orientation
    _destX = _destY = fInfinite; _destZ = _destO  = 0.f;
    BroadcastMovementPacket();
}

void UnitPathSystem::BroadcastMovementPacket(uint8 packetSendFlags)
{ 
    // Grab our destination point data
    MovementPoint *lastPoint = m_movementPoints.HasItems() ? m_movementPoints.at(m_movementPoints.size()-1).get() : NULL;
    if(lastPoint == NULL)
        return;
    // Grab our start point data
    LocationVector startPoint(lastUpdatePoint.pos.x, lastUpdatePoint.pos.y, lastUpdatePoint.pos.z);
    // If we have no path data but are broadcasting, check validity of last update point
    if(lastUpdatePoint.pos.x == fInfinite || lastUpdatePoint.pos.y == fInfinite)
        startPoint = *m_Unit->GetPositionV();

    WorldPacket data(SMSG_MONSTER_MOVE, 100);
    data << m_Unit->GetGUID().asPacked();
    data << uint8(0);
    // We need to append our start vector here, but need to use it later for compressed movement
    data.appendvector(startPoint, false);
    // If we are at our destination, or have no destination, broadcast a stop packet
    if((lastUpdatePoint.pos.x == _destX && lastUpdatePoint.pos.y == _destY) || (_destX == fInfinite && _destY == fInfinite))
        data << uint32(0) << uint8(1);
    else
    {
        data << uint32(m_pathCounter);
        if(_destO == fInfinite) data << uint8(0);
        else data << uint8(4) << float( _destO );
        data << uint32(buildMonsterMoveFlags(packetSendFlags));
        data << uint32(m_pathLength);

        uint32 counter = 1;
        size_t counterPos = data.wpos();
        data << uint32(counter);
        // Append uncompressed buffer
        if(packetSendFlags & MOVEBCFLAG_UNCOMP)
        {
            for(uint32 i = 0; i < m_movementPoints.size()-1; i++)
            {
                if(MovementPoint *path = m_movementPoints.at(i).get())
                {
                    if(path->timeStamp <= lastUpdatePoint.timeStamp)
                        continue;

                    data << path->pos.x << path->pos.y << path->pos.z;
                    counter++;
                }
            }
        }

        // Append our last point, could use _dest if we wanted to
        data << lastPoint->pos.x << lastPoint->pos.y << lastPoint->pos.z;
        // Append compressed buffer here
        if((packetSendFlags & MOVEBCFLAG_UNCOMP) == 0)
        {
            LocationVector middle(lastUpdatePoint.pos.x, lastUpdatePoint.pos.y, lastUpdatePoint.pos.z);
            middle.x = (middle.x+lastPoint->pos.x)/2.f;
            middle.y = (middle.y+lastPoint->pos.y)/2.f;
            middle.z = (middle.z+lastPoint->pos.z)/2.f;
            for(uint32 i = 0; i < m_movementPoints.size()-1; i++)
            {
                if(MovementPoint *path = m_movementPoints.at(i).get())
                {
                    if(path->timeStamp <= lastUpdatePoint.timeStamp)
                        continue;

                    data << RONIN_UTIL::CompressMovementPoint(middle.x - path->pos.x, middle.y - path->pos.y, middle.z - path->pos.z);
                    counter++;
                }
            }
        }
        data.put<uint32>(counterPos, counter);
    }

    m_Unit->SendMessageToSet( &data, false );
}

void UnitPathSystem::SendMovementPacket(Player *plr, uint8 packetSendFlags)
{
    if((m_Unit->GetPositionX() == _destX && m_Unit->GetPositionY() == _destY) || (_destX == fInfinite && _destY == fInfinite) || (lastUpdatePoint.timeStamp >= m_pathLength))
        return;
    MovementPoint *lastPoint = m_movementPoints.HasItems() ? m_movementPoints.at(m_movementPoints.size()-1).get() : NULL;
    if(lastPoint == NULL)
        return;

    WorldPacket data(SMSG_MONSTER_MOVE, 100);
    data << m_Unit->GetGUID().asPacked();
    data << uint8(0);
    data.appendvector(LocationVector(lastUpdatePoint.pos.x, lastUpdatePoint.pos.y, lastUpdatePoint.pos.z), false);
    data << uint32(m_pathCounter);
    if(_destO == fInfinite) data << uint8(0);
    else data << uint8(4) << float( _destO );
    data << uint32(buildMonsterMoveFlags(packetSendFlags));
    data << uint32(m_pathLength - lastUpdatePoint.timeStamp);

    uint32 counter = 1;
    size_t counterPos = data.wpos();
    data << uint32(counter);
    // Append uncompressed buffer
    if(packetSendFlags & MOVEBCFLAG_UNCOMP)
    {
        for(uint32 i = 0; i < m_movementPoints.size()-1; i++)
        {
            if(MovementPoint *path = m_movementPoints.at(i).get())
            {
                if(path->timeStamp <= lastUpdatePoint.timeStamp)
                    continue;

                data << path->pos.x << path->pos.y << path->pos.z;
                counter++;
            }
        }
    }

    // Append our last point, could use _dest if we wanted to
    data << lastPoint->pos.x << lastPoint->pos.y << lastPoint->pos.z;
    // Append compressed buffer here
    if((packetSendFlags & MOVEBCFLAG_UNCOMP) == 0)
    {
        LocationVector middle(lastUpdatePoint.pos.x, lastUpdatePoint.pos.y, lastUpdatePoint.pos.z);
        middle.x = (middle.x+lastPoint->pos.x)/2.f;
        middle.y = (middle.y+lastPoint->pos.y)/2.f;
        middle.z = (middle.z+lastPoint->pos.z)/2.f;
        for(uint32 i = 0; i < m_movementPoints.size()-1; i++)
        {
            if(MovementPoint *path = m_movementPoints.at(i).get())
            {
                if(path->timeStamp <= lastUpdatePoint.timeStamp)
                    continue;

                data << RONIN_UTIL::CompressMovementPoint(middle.x - path->pos.x, middle.y - path->pos.y, middle.z - path->pos.z);
                counter++;
            }
        }
    }
    data.put<uint32>(counterPos, counter);

    if(packetSendFlags & MOVEBCFLAG_DELAYED)
        plr->PushPacketToQueue(&data);
    else plr->PushPacket(&data);
}

void UnitPathSystem::AppendMoveBits(ByteBuffer *buffer, uint32 msTime, std::vector<MovementPoint*> *pointStorage)
{
    bool finalized = false;
    // Grab our start point data
    LocationVector startPoint(lastUpdatePoint.pos.x, lastUpdatePoint.pos.y, lastUpdatePoint.pos.z);
    // If we have no path data but are broadcasting, check validity of last update point
    if(lastUpdatePoint.pos.x == fInfinite || lastUpdatePoint.pos.y == fInfinite)
        startPoint = *m_Unit->GetPositionV();
    // Check if our starting point is also our destination or if we're just too far gone
    if((startPoint.x == _destX && startPoint.y == _destY) || (_destX == fInfinite && _destY == fInfinite) || (lastUpdatePoint.timeStamp >= m_pathLength))
        finalized = true;
    else if(closeToDestination(msTime))
        finalized = true;
    MovementPoint *lastPoint = m_movementPoints.HasItems() ? m_movementPoints.at(m_movementPoints.size()-1).get() : NULL;
    if(lastPoint == NULL)
        finalized = true;
    if(!buffer->WriteBit(!finalized))
        return;

    for(uint32 i = 0; i < m_movementPoints.size()-1; i++)
        if(MovementPoint *path = m_movementPoints.at(i).get())
            if(path->timeStamp > lastUpdatePoint.timeStamp)
                pointStorage->push_back(path);

    buffer->WriteBits<uint8>(0x00, 2); // Linear mode works
    buffer->WriteBit(false); // We don't need to handle any flags here
    buffer->WriteBits(pointStorage->size(), 22);
    buffer->WriteBits<uint8>(0xFF, 2); // Target destination arrival flags
    if(false); // Target destination flags, leave empty for now
    buffer->WriteBit(false); // Parabolic pathing effect
    buffer->WriteBits(buildMonsterMoveFlags(MOVEBCFLAG_UNCOMP), 25); // Spline movement flags different than normal?
}

void UnitPathSystem::AppendMoveBytes(ByteBuffer *buffer, uint32 msTime, std::vector<MovementPoint*> *pointStorage)
{
    LocationVector dest(_destZ, _destX, _destY);
    if(!pointStorage->empty())
    {
        if(false); // Parabolic pathing effect
        buffer->append<int32>(std::max<int32>(0, msTime - lastUpdatePoint.timeStamp));
        for(std::vector<MovementPoint*>::iterator itr = pointStorage->begin(); itr != pointStorage->end(); ++itr)
        {
            buffer->append<float>((*itr)->pos.z);
            buffer->append<float>((*itr)->pos.x);
            buffer->append<float>((*itr)->pos.y);
        }

        if(false); // Final point target

        // Next point speed modification
        buffer->append<float>(1.f);
        buffer->append<int32>(m_pathLength-lastUpdatePoint.timeStamp);
        if(false); // Parabolic pathing effect
        // current point speed modification
        buffer->append<float>(1.f);
    }

    if(false) dest.ChangeCoords(0.f, 0.f, 0.f);
    buffer->appendvector(dest);
    buffer->append<uint32>(m_pathCounter);
}
