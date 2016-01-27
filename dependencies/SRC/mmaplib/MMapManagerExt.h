
#pragma once

#define MMAP_UNAVAILABLE 424242.42f // 'MMAP'

struct Position
{
    Position(float _x, float _y, float _z) { x = _x; y = _y; z = _z; };
    float x, y, z;
};

struct PositionMapContainer
{
    std::map<uint32, Position> InternalMap;
};

class MMapManagerExt
{
public:
    virtual bool LoadNavMesh(uint32 x, uint32 y) = 0;
    virtual void UnloadNavMesh(uint32 x, uint32 y) = 0;
    virtual bool IsNavmeshLoaded(uint32 x, uint32 y) = 0;

    virtual Position getNextPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz) = 0;
    virtual Position getBestPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz) = 0;
    virtual PositionMapContainer* BuildFullPath(unsigned short moveFlags, float startx, float starty, float startz, float endx, float endy, float endz, bool straight) = 0;

    virtual bool GetWalkingHeightInternal(float startx, float starty, float startz, float endz, Position& out) = 0;
    virtual bool getNextPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz, Position& out) = 0;
};