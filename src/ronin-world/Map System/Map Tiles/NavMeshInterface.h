/***
 * Demonstrike Core
 */

#pragma once

typedef MMapManagerExt*(*mmap_manager_construction)(std::string dataDir, unsigned int mapId);

class SERVER_DECL CNavMeshInterface : public Singleton<CNavMeshInterface>
{
private:
    HMODULE hModule;
    mmap_manager_construction allocator;

public:
    void Init();
    void DeInit();
    MMapManagerExt* GetOrCreateMMapManager(uint32 mapid);

public: // Navmesh settings
    bool LoadNavMesh(uint32 mapid, uint32 x, uint32 y);
    void UnloadNavMesh(uint32 mapid, uint32 x, uint32 y);
    bool IsNavmeshLoaded(uint32 mapid, uint32 x, uint32 y);
    bool IsNavmeshLoadedAtPosition(uint32 mapid, float x, float y) { if(!AreCoordinatesValid(x, y)) return false; return IsNavmeshLoaded(mapid, (GetPosX(x)/8), (GetPosY(y)/8)); };

    float GetWalkingHeight(uint32 mapid, float positionx, float positiony, float positionz, float positionz2);
    bool BuildPath(uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, Position& out);
    Position BuildPath(uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, bool best = false);
    PositionMapContainer* BuildFullPath(Unit* m_Unit, uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, bool straight = true);

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

    MMapManagerExt* GetMMap(uint32 mapid)
    {
        if(m_maps.find(mapid) != m_maps.end())
            return m_maps.at(mapid);
        return NULL;
    };

    std::map<uint32, MMapManagerExt*> m_maps;
};

#define sNavMeshInterface CNavMeshInterface::getSingleton()
