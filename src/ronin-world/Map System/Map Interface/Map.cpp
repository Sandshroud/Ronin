/***
 * Demonstrike Core
 */

// Class Map
// Holder for all instances of each mapmgr, handles transferring
// players between, and template holding.

#include "StdAfx.h"

Map::Map(MapEntry *map, char *name) : _mapId(map->MapID), mapName(name), _terrain(NULL), _entry(map)
{

}

Map::~Map()
{
    if(_terrain) delete _terrain, _terrain = NULL;

    for(SpawnsMap::iterator itr = m_spawns.begin(); itr != m_spawns.end(); itr++)
    {
        itr->second.CreatureSpawns.clear();
        itr->second.GOSpawns.clear();
    }
    m_spawns.clear();
}

void Map::_InitializeTerrain(bool continent)
{
    if(_terrain != NULL)
        return;

    _terrain = new TerrainMgr(sWorld.MapPath, _mapId);

    // Initialize the terrain header
    _terrain->LoadTerrainHeader();
    if(continent == false)
        return;

    // Load up the vmap terrain
    _terrain->LoadVMapTerrain();

    // Load all the terrain from this map
    if(sWorld.ServerPreloading == 0)
        _terrain->LoadAllTerrain();
}

void Map::Initialize(CellSpawns *mapSpawns, bool continent)
{
    //new stuff Load Spawns
    LoadSpawns(mapSpawns);

    // Initialize our terrain data
    _InitializeTerrain(continent);
}

void Map::LoadSpawns(CellSpawns *mapSpawns)
{
    for(SpawnsMap::iterator itr = m_spawns.begin(); itr != m_spawns.end(); itr++)
    {
        itr->second.CreatureSpawns.clear();
        itr->second.GOSpawns.clear();
    }
    m_spawns.clear();
    if(mapSpawns == NULL)
        return;

    for(CreatureSpawnList::iterator i = mapSpawns->CreatureSpawns.begin(); i != mapSpawns->CreatureSpawns.end(); i++)
    {
        uint32 cellx = CellHandler<MapInstance>::GetPosX((*i)->x), celly = CellHandler<MapInstance>::GetPosY((*i)->y);
        GetSpawnsListAndCreate(cellx, celly)->CreatureSpawns.push_back((*i));
    }

    for(GOSpawnList::iterator i = mapSpawns->GOSpawns.begin(); i != mapSpawns->GOSpawns.end(); i++)
    {
        uint32 cellx = CellHandler<MapInstance>::GetPosX((*i)->x), celly = CellHandler<MapInstance>::GetPosY((*i)->y);
        GetSpawnsListAndCreate(cellx, celly)->GOSpawns.push_back((*i));
    }
}

