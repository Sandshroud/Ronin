/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
    uint32 moveType;
    float x, y, z, o;
    uint32 delay, actionId;
}CreatureWaypoint;

typedef std::map<uint32, CreatureWaypoint*> WaypointStorage;

// World
typedef struct
{
    WoWGuid guid;
    float x, y, z, o;
    uint32 spawnFlags;
    uint32 modelId;
    uint16 phaseMask;
    uint32 eventId;
    uint32 conditionId;
    int32 vendormask;

    WaypointStorage m_waypointData;
}CreatureSpawn;

typedef struct
{
    WoWGuid guid;
    float  x, y, z;
    float  rX, rY, rZ, rAngle;
    uint32 state;
    uint32 flags;
    uint32 faction;
    float  scale;
    uint16 phaseMask;
    uint32 eventId;
    uint32 conditionId;
}GameObjectSpawn;

typedef std::vector<CreatureSpawn*>     CreatureSpawnArray;
typedef std::vector<GameObjectSpawn*>   GameObjectSpawnArray;

typedef struct
{
    uint32 dbcId;
    uint8 flags;
    float x, y, z, o;
    uint32 zoneId;
    std::string name;
}WorldSafeLocation;

enum SafeLocationFlags : uint8
{
    SAFE_LOCFLAG_NONE       = 0x00,
    SAFE_LOCFLAG_ALLIANCE   = 0x01,
    SAFE_LOCFLAG_HORDE      = 0x02,
    SAFE_LOCFLAG_GRAVEYARD  = 0x04
};

typedef std::map<uint32, WorldSafeLocation*> SafeLocationStorage;

typedef struct
{
    CreatureSpawnArray CreatureSpawns;
    GameObjectSpawnArray GameObjectSpawns;
    std::set<size_t> SpiritHealers;
}CellSpawns;

typedef std::map<std::pair<uint32, uint32>, CellSpawns > SpawnsMap;

class SERVER_DECL Map
{
public:
    Map(MapEntry *map, char *name);
    ~Map();

    void PreloadTerrain(bool continent) { _InitializeTerrain(continent); }
    void Initialize(CellSpawns *mapSpawns, bool continent);
    void ProcessInputData();

protected:
    void _InitializeTerrain(bool continent);

public:
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

    SpawnsMap::iterator GetSpawnsMapBegin() { return m_spawns.begin(); }
    SpawnsMap::iterator GetSpawnsMapEnd() { return m_spawns.end(); }

    void LoadSpawns(CellSpawns *mapSpawns);
    RONIN_INLINE TerrainMgr* GetMapTerrain() { return _terrain; }
    RONIN_INLINE MapEntry *GetEntry() { return _entry; }

    RONIN_INLINE void UnloadAllTerrain(bool forced = false) { _terrain->UnloadAllTerrain(forced); }

    RONIN_INLINE float GetLandHeight(float x, float y) { return _terrain->GetLandHeight(x, y); }
    RONIN_INLINE float GetWaterHeight(float x, float y) { return _terrain->GetWaterHeight(x, y); }
    RONIN_INLINE uint16 GetWaterType(float x, float y) { return _terrain->GetWaterType(x, y); }
    RONIN_INLINE uint8 GetWalkableState(float x, float y) { return _terrain->GetWalkableState(x, y); }

    RONIN_INLINE uint16 GetAreaID(float x, float y) { return _terrain->GetAreaID(x, y); }
    RONIN_INLINE bool CellHasAreaID(uint32 x, uint32 y, uint16 &AreaID) { return _terrain->CellHasAreaID(x, y, AreaID); }

    RONIN_INLINE bool IsCollisionEnabled() { return true; }

    RONIN_INLINE void CellLoaded(uint32 x, uint32 y) { _terrain->CellGoneActive(x,y); }
    RONIN_INLINE void CellUnloaded(uint32 x,uint32 y) { _terrain->CellGoneIdle(x,y); }

    // Grab our 
    void GetClosestGraveyard(uint8 team, uint32 mapId, float x, float y, float z, uint32 zoneId, LocationVector &output);

    // Safe location data
    void AddSafeLocation(WorldSafeLocsEntry *safeLocation);

private:
    uint32 _mapId;
    std::string mapName;

    TerrainMgr *_terrain;
    SpawnsMap m_spawns;
    MapEntry *_entry;

    std::set<CreatureSpawn*> m_spiritHealer;
    SafeLocationStorage _safeLoc, _safeLocGraveyards;
};
