/***
 * Demonstrike Core
 */

#pragma once

#define MMAP_UNAVAILABLE 424242.42f // 'MMAP'
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

typedef map<uint32, TileReferenceC*> ReferenceMap;
typedef map<dtTileRef, uint32> ReverseReferenceMap;

class MMapManager
{
    uint32 GetPosX(float x)
    {
        ASSERT((x >= _minX) && (x <= _maxX));
        return (uint32)((_maxX-x)/_cellSize);
    };

    uint32 GetPosY(float y)
    {
        ASSERT((y >= _minY) && (y <= _maxY));
        return (uint32)((_maxY-y)/_cellSize);
    };
public:
    MMapManager(uint32 mapid);
    ~MMapManager();

private:
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

    LocationVector getNextPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz);
    LocationVector getBestPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz);
    LocationVectorMapContainer* BuildFullPath(Unit* m_Unit, float startx, float starty, float startz, float endx, float endy, float endz, bool straight);

    // Poly locating
    dtPolyRef GetPathPolyByPosition(dtPolyRef const* polyPath, uint32 polyPathSize, float const* Point, float* Distance = NULL) const;
    dtPolyRef GetPolyByLocation(dtQueryFilter* m_filter, dtPolyRef const* polyPath, uint32 polyPathSize, float const* Point, float* Distance) const;

    // Smooth pathing
    uint32 fixupCorridor(dtPolyRef* path, uint32 npath, uint32 maxPath, dtPolyRef* visited, uint32 nvisited);
    bool getSteerTarget(float* startPos, float* endPos, float minTargetDist, dtPolyRef* path, uint32 pathSize, float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef);
    dtStatus findSmoothPath(dtQueryFilter* m_filter, float* startPos, float* endPos, dtPolyRef* polyPath, uint32 polyPathSize, float* smoothPath, int* smoothPathSize, uint32 smoothPathMaxSize);

    bool GetWalkingHeightInternal(float startx, float starty, float startz, float endz, LocationVector& out);
    bool getNextPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz, LocationVector& out);

    static float calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y );
private:
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

class SERVER_DECL CNavMeshInterface
{
public:
    void Init();
    void DeInit();
    MMapManager* GetOrCreateMMapManager(uint32 mapid);

public: // Navmesh settings
    bool LoadNavMesh(uint32 mapid, uint32 x, uint32 y);
    void UnloadNavMesh(uint32 mapid, uint32 x, uint32 y);
    bool IsNavmeshLoaded(uint32 mapid, uint32 x, uint32 y);
    bool IsNavmeshLoadedAtPosition(uint32 mapid, float x, float y) { if(!AreCoordinatesValid(x, y)) return false; return IsNavmeshLoaded(mapid, (GetPosX(x)/8), (GetPosY(y)/8)); };

    float GetWalkingHeight(uint32 mapid, float positionx, float positiony, float positionz, float positionz2);
    bool BuildPath(uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, LocationVector& out);
    LocationVector BuildPath(uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, bool best = false);
    LocationVectorMapContainer* BuildFullPath(Unit* m_Unit, uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, bool straight = true);

private:
    uint32 GetPosX(float x)
    {
        ASSERT((x >= _minX) && (x <= _maxX));
        return (uint32)((_maxX-x)/_cellSize);
    };

    uint32 GetPosY(float y)
    {
        ASSERT((y >= _minY) && (y <= _maxY));
        return (uint32)((_maxY-y)/_cellSize);
    };

    bool AreCoordinatesValid(float x, float y)
    {
        if((x >= _minX) && (x <= _maxX))
            if((y >= _minY) && (y <= _maxY))
                return true;
        return false;
    };

    MMapManager* GetMMap(uint32 mapid)
    {
        MMapManager* mmapreturn = NULL;
        if(mapid < NUM_MAPS)
            mmapreturn = MMaps[mapid];
        return mmapreturn;
    };

    MMapManager* MMaps[NUM_MAPS];
};

extern SERVER_DECL CNavMeshInterface NavMeshInterface;

