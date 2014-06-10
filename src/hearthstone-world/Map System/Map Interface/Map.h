/***
 * Demonstrike Core
 */

//
// Map.h
//

#pragma once

class MapMgr;
struct MapInfo;
class TerrainMgr;
class TemplateMgr;

struct Formation;

struct SpawnBytes
{
    SpawnBytes(uint32 _bytes = 0, uint32 _bytes1 = 0, uint32 _bytes2 = 0)
    {
        bytes = _bytes;
        bytes1 = _bytes1;
        bytes2 = _bytes2;
    }

    uint32 bytes;
    uint32 bytes1;
    uint32 bytes2;
};

struct SpawnChannelData
{
    SpawnChannelData(uint32 _channel_spell = 0, uint32 _channel_target_go = 0, uint32 _channel_target_creature = 0)
    {
        channel_spell = _channel_spell;
        channel_target_go = _channel_target_go;
        channel_target_creature = _channel_target_creature;
    }

    uint32 channel_spell;
    uint32 channel_target_go;
    uint32 channel_target_creature;
};

struct SpawnMountedDisplay
{
    SpawnMountedDisplay(uint32 _MountedDisplayID = 0)
    {
        MountedDisplayID = _MountedDisplayID;
    }

    uint32 MountedDisplayID;
};

typedef struct
{
    uint32  id;//spawn ID
    uint32  entry;
    float   x, y, z, o;

    uint32  flags;
    uint32  factionid;
    uint32  emote_state;
    uint8   stand_state;
    int32   phase;
    bool    vehicle;
    uint32  CanMove;
    int32   vendormask;
    uint16 death_state;

    SpawnBytes *Bytes;
    SpawnChannelData *ChannelData;
    SpawnMountedDisplay *MountedDisplay;
}CreatureSpawn;

typedef struct GOSpawn
{
    uint32  id;//spawn ID
    uint32  entry;
    float   x;
    float   y;
    float   z;
    float   facing;
    uint32  state;
    uint32  flags;
    uint32  faction;
    float   scale;
    int32   phase;
} GOSpawn;

typedef std::vector<CreatureSpawn*> CreatureSpawnList;
typedef std::vector<GOSpawn*> GOSpawnList;

typedef struct
{
    CreatureSpawnList CreatureSpawns;
    GOSpawnList GOSpawns;
}CellSpawns;

typedef std::map<uint32, CellSpawns*> CellSpawnsMap;
typedef std::map<uint32, CellSpawnsMap* > SpawnsMap;

class SERVER_DECL Map
{
public:
    Map(uint32 mapid, MapInfo * inf);
    ~Map();

    HEARTHSTONE_INLINE string GetNameString() { return name; }
    HEARTHSTONE_INLINE const char* GetName() { return name.c_str(); }
    HEARTHSTONE_INLINE MapEntry* GetDBCEntry() { return me; }

    HEARTHSTONE_INLINE CellSpawns *GetSpawnsList(uint32 cellx,uint32 celly)
    {
        ASSERT(cellx < _sizeX);
        ASSERT(celly < _sizeY);

        SpawnsMap::iterator itr = m_spawns.find(cellx);
        if(itr == m_spawns.end())
            return NULL;
        if(itr->second == NULL)
            return NULL;
        if(itr->second->find(celly) == itr->second->end())
            return NULL;
        return itr->second->at(celly);
    }

    HEARTHSTONE_INLINE CellSpawns *GetSpawnsListAndCreate(uint32 cellx, uint32 celly)
    {
        ASSERT(cellx < _sizeX);
        ASSERT(celly < _sizeY);
        SpawnsMap::iterator itr = m_spawns.find(cellx);
        if(itr == m_spawns.end())
        {
            m_spawns.insert(make_pair(cellx, new CellSpawnsMap()));
            itr = m_spawns.find(cellx);
        }
        if(itr->second == NULL)
            itr->second = new CellSpawnsMap();
        if(itr->second->find(celly) == itr->second->end())
            itr->second->insert(make_pair(celly, new CellSpawns()));
        return itr->second->at(celly);
    }

    void LoadSpawns(bool reload = false);//set to true to make clean up
    uint32 CreatureSpawnCount;
    TerrainMgr* GetMapTerrain() { return _terrain; };

    HEARTHSTONE_INLINE void LoadAllTerrain() { _terrain->LoadAllTerrain(); }
    HEARTHSTONE_INLINE void UnloadAllTerrain() { _terrain->UnloadAllTerrain(); }

    HEARTHSTONE_INLINE float GetLandHeight(float x, float y) { return _terrain->GetLandHeight(x, y); }
    HEARTHSTONE_INLINE float GetWaterHeight(float x, float y, float z) { return _terrain->GetWaterHeight(x, y, z); }
    HEARTHSTONE_INLINE uint16 GetWaterType(float x, float y) { return _terrain->GetWaterType(x, y); }
    HEARTHSTONE_INLINE uint8 GetWalkableState(float x, float y) { return _terrain->GetWalkableState(x, y); }

    HEARTHSTONE_INLINE uint16 GetAreaID(float x, float y, float z) { return _terrain->GetAreaID(x, y, z); }
    HEARTHSTONE_INLINE void GetCellLimits(uint32 &StartX, uint32 &EndX, uint32 &StartY, uint32 &EndY) { _terrain->GetCellLimits(StartX, EndX, StartY, EndY); }
    HEARTHSTONE_INLINE bool CellHasAreaID(uint32 x, uint32 y, uint16 &AreaID) { return _terrain->CellHasAreaID(x, y, AreaID); }

    HEARTHSTONE_INLINE bool IsCollisionEnabled() { return Collision; }
    HEARTHSTONE_INLINE void CellGoneActive(uint32 x, uint32 y) { _terrain->CellGoneActive(x,y); }
    HEARTHSTONE_INLINE void CellGoneIdle(uint32 x,uint32 y) { _terrain->CellGoneIdle(x,y); }

private:
    bool Collision;
    TerrainMgr *_terrain;
    MapInfo *_mapInfo;
    uint32 _mapId;
    string name;
    MapEntry *me;

    SpawnsMap m_spawns;
};
