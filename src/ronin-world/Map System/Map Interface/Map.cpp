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
        itr->second.GameObjectSpawns.clear();
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
    if(sWorld.ServerPreloading >= 1)
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
        itr->second.GameObjectSpawns.clear();
    }
    m_spawns.clear();
    if(mapSpawns == NULL)
        return;

    for(CreatureSpawnList::iterator i = mapSpawns->CreatureSpawns.begin(); i != mapSpawns->CreatureSpawns.end(); i++)
    {
        uint32 cellx = CellHandler<MapInstance>::GetPosX((*i)->x), celly = CellHandler<MapInstance>::GetPosY((*i)->y);
        GetSpawnsListAndCreate(cellx, celly)->CreatureSpawns.push_back((*i));
    }

    for(GameObjectSpawnList::iterator i = mapSpawns->GameObjectSpawns.begin(); i != mapSpawns->GameObjectSpawns.end(); i++)
    {
        uint32 cellx = CellHandler<MapInstance>::GetPosX((*i)->x), celly = CellHandler<MapInstance>::GetPosY((*i)->y);
        GetSpawnsListAndCreate(cellx, celly)->GameObjectSpawns.push_back((*i));
    }
}

