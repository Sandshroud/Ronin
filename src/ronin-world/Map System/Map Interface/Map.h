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

typedef struct
{
    uint32 id;//spawn ID
    uint32 entry;
    float  x, y, z, o;
    uint32 modelId;
    uint8 modelGender;

    uint32 flags;
    uint32 factionid;
    uint32 emote_state;
    uint8  stand_state;
    bool   vehicle;
    uint32 CanMove;
    int32  vendormask;
    uint16 death_state;
    uint32 MountedDisplayID;

    SpawnBytes *Bytes;
    SpawnChannelData *ChannelData;

    bool GetModelData(uint8 &gender, uint32 &model)
    {
        if(model = modelId)
        {
            gender = modelGender;
            return true;
        }
        return false;
    }
}CreatureSpawn;

typedef struct GOSpawn
{
    uint32  id;//spawn ID
    uint32  entry;
    float  x, y, z, o;
    uint32  state;
    uint32  flags;
    uint32  faction;
    float   scale;
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

    RONIN_INLINE std::string GetNameString() { return name; }
    RONIN_INLINE const char* GetName() { return name.c_str(); }
    RONIN_INLINE MapEntry* GetDBCEntry() { return me; }

    RONIN_INLINE CellSpawns *GetSpawnsList(uint32 cellx,uint32 celly)
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

    RONIN_INLINE CellSpawns *GetSpawnsListAndCreate(uint32 cellx, uint32 celly)
    {
        ASSERT(cellx < _sizeX);
        ASSERT(celly < _sizeY);
        SpawnsMap::iterator itr = m_spawns.find(cellx);
        if(itr == m_spawns.end())
        {
            m_spawns.insert(std::make_pair(cellx, new CellSpawnsMap()));
            itr = m_spawns.find(cellx);
        }
        if(itr->second == NULL)
            itr->second = new CellSpawnsMap();
        if(itr->second->find(celly) == itr->second->end())
            itr->second->insert(std::make_pair(celly, new CellSpawns()));
        return itr->second->at(celly);
    }

    void LoadSpawns(bool reload = false);//set to true to make clean up
    uint32 CreatureSpawnCount;
    TerrainMgr* GetMapTerrain() { return _terrain; };

    RONIN_INLINE void LoadAllTerrain() { _terrain->LoadAllTerrain(); }
    RONIN_INLINE void UnloadAllTerrain() { _terrain->UnloadAllTerrain(); }

    RONIN_INLINE size_t GetTerrainSize() { return _terrain->GetSize(); }
    RONIN_INLINE float GetLandHeight(float x, float y) { return _terrain->GetLandHeight(x, y); }
    RONIN_INLINE float GetWaterHeight(float x, float y, float z) { return _terrain->GetWaterHeight(x, y, z); }
    RONIN_INLINE uint8 GetWaterType(float x, float y) { return _terrain->GetWaterType(x, y); }
    RONIN_INLINE uint8 GetWalkableState(float x, float y) { return _terrain->GetWalkableState(x, y); }

    RONIN_INLINE uint16 GetAreaID(float x, float y, float z) { return _terrain->GetAreaID(x, y, z); }
    RONIN_INLINE void GetCellLimits(uint32 &StartX, uint32 &EndX, uint32 &StartY, uint32 &EndY) { _terrain->GetCellLimits(StartX, EndX, StartY, EndY); }
    RONIN_INLINE bool CellHasAreaID(uint32 x, uint32 y, uint16 &AreaID) { return _terrain->CellHasAreaID(x, y, AreaID); }

    RONIN_INLINE bool IsCollisionEnabled() { return Collision; }
    RONIN_INLINE void CellGoneActive(uint32 x, uint32 y) { _terrain->CellGoneActive(x,y); }
    RONIN_INLINE void CellGoneIdle(uint32 x,uint32 y) { _terrain->CellGoneIdle(x,y); }

private:
    bool Collision;
    TerrainMgr *_terrain;
    MapInfo *_mapInfo;
    uint32 _mapId;
    std::string name;
    MapEntry *me;

    SpawnsMap m_spawns;
};
