
#include "StdAfx.h"

SmartBounding::SmartBounding() : msStartTime(0), moveTime(0), timeToTarget(0), lastUpdateTime(0), lastPointX(UnitPathSystem::fInfinite), lastPointY(UnitPathSystem::fInfinite), lastPointZ(UnitPathSystem::fInfinite), endX(UnitPathSystem::fInfinite), endY(UnitPathSystem::fInfinite), endZ(UnitPathSystem::fInfinite)
{

}

SmartBounding::~SmartBounding()
{

}

void SmartBounding::UpdatePosition(uint32 startTime, uint32 currentTime, float sX, float sY, float sZ, uint32 timeDest, float eX, float eY, float eZ)
{
    lastUpdateTime = msStartTime = startTime;
    moveTime = currentTime;
    lastPointX = sX;
    lastPointY = sY;
    lastPointZ = sZ;
    timeToTarget = timeDest;
    endX = eX;
    endY = eY;
    endZ = eZ;
}

bool SmartBounding::HasTravelData()
{   // Check if we have travel time data
    if(msStartTime == 0 || timeToTarget == 0)
        return false;
    return true;
}

bool SmartBounding::UpdateMovementPoint(uint32 msTime, UnitPathSystem *path, MovementPoint *output)
{
    if(!HasTravelData())
        return false;
    // We've recently updated so do not change our point
    if(lastUpdateTime >= msTime)
        return false;

    uint32 moveOffset = msTime - (msStartTime + moveTime);
    uint32 totalMove = (timeToTarget - moveTime);
    float p = float(moveOffset) / float(totalMove);

    lastUpdateTime = msTime;
    output->timeStamp = moveTime + moveOffset;
    output->pos.x = lastPointX - ((lastPointX - endX)*p);
    output->pos.y = lastPointY - ((lastPointY - endY)*p);
    output->pos.z = lastPointZ - ((lastPointZ - endZ)*p);
    output->orientation = (WorldObject::calcAngle(lastPointX, lastPointY, endX, endY) * M_PI / 180.f);
    return true;
}

void SmartBounding::GetPosition(uint32 msTime, LocationVector *output)
{

}
