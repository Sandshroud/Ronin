/***
 * Demonstrike Core
 */

//
// Map.h
//

#pragma once

class MapInstance;
class TerrainMgr;
class TemplateMgr;
struct Formation;

typedef struct
{
    uint32 id;//spawn ID
    uint32 entry;
    float x, y, z, o;
    uint32 modelId;
    int32 vendormask;
}CreatureSpawn;
typedef std::vector<CreatureSpawn*> CreatureSpawnList;

typedef struct
{
    uint32  id;//spawn ID
    uint32  entry;
    float  x, y, z, o;
    uint32  state;
    uint32  flags;
    uint32  faction;
    float   scale;
} GOSpawn;
typedef std::vector<GOSpawn*> GOSpawnList;

typedef struct
{
    CreatureSpawnList CreatureSpawns;
    GOSpawnList GOSpawns;
}CellSpawns;

typedef std::map<std::pair<uint32, uint32>, CellSpawns > SpawnsMap;

class SERVER_DECL Map
{
public:
    Map(uint32 mapid, char *name);
    ~Map();

    RONIN_INLINE std::string GetNameString() { return mapName; }
    RONIN_INLINE const char* GetName() { return mapName.c_str(); }

    RONIN_INLINE CellSpawns *GetSpawnsListAndCreate(uint32 cellx, uint32 celly)
    {
        ASSERT(cellx < _sizeX && celly < _sizeY);
        return &m_spawns[std::make_pair(cellx, celly)];
    }

    RONIN_INLINE CellSpawns *GetSpawnsList(uint32 cellx,uint32 celly)
    {
        ASSERT(cellx < _sizeX && celly < _sizeY);
        std::pair<uint32, uint32> cellPair = std::make_pair(cellx, celly);
        if(m_spawns.find(cellPair) == m_spawns.end())
            return NULL;
        return &m_spawns.at(cellPair);
    }

    void LoadSpawns(bool reload = false);//set to true to make clean up
    TerrainMgr* GetMapTerrain() { return _terrain; };

    RONIN_INLINE void LoadAllTerrain() { _terrain->LoadAllTerrain(); }
    RONIN_INLINE void UnloadAllTerrain(bool forced = false) { _terrain->UnloadAllTerrain(forced); }

    RONIN_INLINE float GetLandHeight(float x, float y) { return _terrain->GetLandHeight(x, y); }
    RONIN_INLINE float GetWaterHeight(float x, float y, float z) { return _terrain->GetWaterHeight(x, y, z); }
    RONIN_INLINE uint8 GetWaterType(float x, float y) { return _terrain->GetWaterType(x, y); }
    RONIN_INLINE uint8 GetWalkableState(float x, float y) { return _terrain->GetWalkableState(x, y); }

    RONIN_INLINE uint16 GetAreaID(float x, float y, float z) { return _terrain->GetAreaID(x, y, z); }
    RONIN_INLINE bool CellHasAreaID(uint32 x, uint32 y, uint16 &AreaID) { return _terrain->CellHasAreaID(x, y, AreaID); }

    RONIN_INLINE bool IsCollisionEnabled() { return false; }
    RONIN_INLINE void CellGoneActive(uint32 x, uint32 y) { _terrain->CellGoneActive(x,y); }
    RONIN_INLINE void CellGoneIdle(uint32 x,uint32 y) { _terrain->CellGoneIdle(x,y); }

private:
    uint32 _mapId;
    std::string mapName;

    TerrainMgr *_terrain;
    SpawnsMap m_spawns;
};
