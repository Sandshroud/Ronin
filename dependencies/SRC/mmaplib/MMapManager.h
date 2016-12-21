/*
 * Copyright (C) 2014-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "SharedDependencyDefines.h"
#include "MMapManagerExt.h"

#define MMAP_MAGIC 0x4d4d4150   // 'MMAP'
#define MMAP_VERSION 4

struct MmapTileHeader
{
    uint32 mmapMagic;
    uint32 dtVersion;
    uint32 mmapVersion;
    uint32 size;
    bool usesLiquids : 1;

    MmapTileHeader() : mmapMagic(MMAP_MAGIC), dtVersion(DT_NAVMESH_VERSION),
        mmapVersion(MMAP_VERSION), size(0), usesLiquids(true) {}
};

struct TileReferenceC
{
    TileReferenceC(dtTileRef refid) { ID = refid; };
    ~TileReferenceC() {};

    dtTileRef ID;
};

typedef std::map<uint32, TileReferenceC*> ReferenceMap;
typedef std::map<dtTileRef, uint32> ReverseReferenceMap;

class MMapManager : public MMapManagerExt
{
public:
    MMapManager(const char* dataPath, uint32 mapid);
    ~MMapManager();

private:
    std::string m_dataPath;
//  Mutex m_Lock; // One day we'll need this, but for now, it's our silent knight, always watching...
    uint32 ManagerMapId;
    dtNavMesh* m_navMesh;
    dtNavMeshQuery* m_navMeshQuery;
    dtTileRef lastTileRef;
    ReferenceMap TileReferences;
    ReverseReferenceMap TileLoadCount;
    uint32 packTileID(int32 x, int32 y) { return uint32(x << 16 | y); };

public:
    bool LoadNavMesh(uint32 x, uint32 y);
    void UnloadNavMesh(uint32 x, uint32 y);
    bool IsNavmeshLoaded(uint32 x, uint32 y);

    Position getNextPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz);
    Position getBestPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz);
    PositionMapContainer* BuildFullPath(unsigned short moveFlags, float startx, float starty, float startz, float endx, float endy, float endz, bool straight);

    // Poly locating
    dtPolyRef GetPathPolyByPosition(dtPolyRef const* polyPath, uint32 polyPathSize, float const* Point, float* Distance = NULL) const;
    dtPolyRef GetPolyByLocation(dtQueryFilter* m_filter, dtPolyRef const* polyPath, uint32 polyPathSize, float const* Point, float* Distance) const;

    // Smooth pathing
    uint32 fixupCorridor(dtPolyRef* path, uint32 npath, uint32 maxPath, dtPolyRef* visited, uint32 nvisited);
    bool getSteerTarget(float* startPos, float* endPos, float minTargetDist, dtPolyRef* path, uint32 pathSize, float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef);
    dtStatus findSmoothPath(dtQueryFilter* m_filter, float* startPos, float* endPos, dtPolyRef* polyPath, uint32 polyPathSize, float* smoothPath, int* smoothPathSize, uint32 smoothPathMaxSize);

    bool GetWalkingHeightInternal(float startx, float starty, float startz, float endz, Position& out);
    bool getNextPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz, Position& out);

private:
    float calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y );

    bool inRangeYZX(float* v1, float* v2, float r, float h)
    {
        float dx = v2[0] - v1[0];
        float dy = v2[1] - v1[1]; // elevation
        float dz = v2[2] - v1[2];
        return (dx*dx + dz*dz) < r*r && fabsf(dy) < h;
    }

    void dtcopy(float* dest, float* a)
    {
        dest[0] = a[0];
        dest[1] = a[1];
        dest[2] = a[2];
    }

    void dtsub(float* dest, const float* v1, const float* v2)
    {
        dest[0] = v1[0]-v2[0];
        dest[1] = v1[1]-v2[1];
        dest[2] = v1[2]-v2[2];
    }

    float dtdot(float* v1, float* v2)
    {
        return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
    }

    void dtmad(float* dest, float* v1, float* v2, float s)
    {
        dest[0] = v1[0]+v2[0]*s;
        dest[1] = v1[1]+v2[1]*s;
        dest[2] = v1[2]+v2[2]*s;
    }
};
