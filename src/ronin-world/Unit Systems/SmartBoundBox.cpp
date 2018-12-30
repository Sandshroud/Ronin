
#include "StdAfx.h"

SmartBounding::SmartBounding()
{

}

SmartBounding::~SmartBounding()
{

}

void SmartBounding::UpdatePosition(uint32 startTime, uint32 currentTime, float sX, float sY, float sZ, uint32 timeDest, float eX, float eY, float eZ)
{
    msStartTime = startTime;
    moveTime = currentTime;
    lastPointX = sX;
    lastPointY = sY;
    lastPointZ = sZ;
    timeToTarget = timeDest;
    endX = eX;
    endY = eY;
    endZ = eZ;
}

void SmartBounding::UpdateMovementPoint(uint32 msTime, UnitPathSystem *path, MovementPoint *output)
{
    uint32 moveOffset = msTime - (msStartTime + moveTime);
    uint32 totalMove = (timeToTarget - moveTime);
    float p = float(moveOffset) / float(totalMove);

    output->timeStamp = moveTime + moveOffset;
    output->pos.x = lastPointX - ((lastPointX - endX)*p);
    output->pos.y = lastPointY - ((lastPointY - endY)*p);
    output->pos.z = lastPointZ - ((lastPointZ - endZ)*p);
    output->orientation = (WorldObject::calcAngle(lastPointX, lastPointY, endX, endY) * M_PI / 180.f);
}

void SmartBounding::GetPosition(uint32 msTime, LocationVector *output)
{

}
