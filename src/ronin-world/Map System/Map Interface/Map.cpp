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
    // Load up the vmap terrain
    _terrain->LoadVMapTerrain();

    // Load all the terrain from this map
    if(continent && sWorld.ServerPreloading >= 1)
        _terrain->LoadAllTerrain();
}

void Map::Initialize(CellSpawns *mapSpawns, bool continent)
{
    //new stuff Load Spawns
    LoadSpawns(mapSpawns);

    // Initialize our terrain data
    _InitializeTerrain(continent);
}

void Map::ProcessInputData()
{
    if(!_safeLoc.empty() && !m_spiritHealer.empty() && _safeLocGraveyards.empty())
    {
        // Scan our creature spawns for spirit healer npcs
        std::map<WorldSafeLocation*, CreatureSpawn*> m_pairedGraveyards;
        for (auto itr = m_spiritHealer.begin(); itr != m_spiritHealer.end(); ++itr)
        {
            float dist = 99999999.f;
            WorldSafeLocation *closest = NULL;
            for (auto loc = _safeLoc.begin(); loc != _safeLoc.end(); ++loc)
            {
                WorldSafeLocation *location = loc->second;
                float delta_x = fabs(location->x - (*itr)->x), delta_y = fabs(location->y - (*itr)->y), delta_z = fabs(location->z - (*itr)->z), locDist = (delta_x*delta_x + delta_y * delta_y + delta_z * delta_z);
                if (closest == NULL || locDist < dist)
                {
                    closest = loc->second;
                    dist = locDist;
                }
            }

            std::map<WorldSafeLocation*, CreatureSpawn*>::iterator eItr;
            if (closest && (eItr = m_pairedGraveyards.find(closest)) != m_pairedGraveyards.end())
            {
                float delta_x = fabs(closest->x - eItr->second->x), delta_y = fabs(closest->y - eItr->second->y), delta_z = fabs(closest->z - eItr->second->z);
                float dist2 = (delta_x*delta_x + delta_y * delta_y + delta_z * delta_z);
                if (dist < dist2) // If we're closer to the graveyard, we become the pair
                    m_pairedGraveyards[closest] = (*itr);
            } else m_pairedGraveyards.insert(std::make_pair(closest, (*itr)));
        }

        for (auto itr = m_pairedGraveyards.begin(); itr != m_pairedGraveyards.end(); ++itr)
        {
            float delta_x = fabs(itr->first->x - itr->second->x), delta_y = fabs(itr->first->y - itr->second->y);
            if ((delta_x*delta_x + delta_y * delta_y) > 1250.f)
                continue;
            itr->first->flags |= SAFE_LOCFLAG_GRAVEYARD;
            itr->first->o = (WorldObject::calcAngle(itr->first->x, itr->first->y, itr->second->x, itr->second->y) * M_PI / 180.f);
            _safeLocGraveyards.insert(std::make_pair(itr->first->dbcId, itr->first));
        }

        sLog.Notice("WorldManager", "%u safe locations loaded for map %u(%s).", _safeLocGraveyards.size(), _mapId, mapName.c_str());
    }
}

void Map::LoadSpawns(CellSpawns *mapSpawns)
{
    for(SpawnsMap::iterator itr = m_spawns.begin(); itr != m_spawns.end(); itr++)
    {
        itr->second.CreatureSpawns.clear();
        itr->second.GameObjectSpawns.clear();
    }
    m_spiritHealer.clear();
    m_spawns.clear();
    if(mapSpawns == NULL)
        return;

    for(CreatureSpawnArray::iterator i = mapSpawns->CreatureSpawns.begin(); i != mapSpawns->CreatureSpawns.end(); i++)
    {
        uint32 cellx = CellHandler<MapInstance>::GetPosX((*i)->x), celly = CellHandler<MapInstance>::GetPosY((*i)->y);
        GetSpawnsListAndCreate(cellx, celly)->CreatureSpawns.push_back((*i));
        if (CreatureData *data = sCreatureDataMgr.GetCreatureData((*i)->guid.getEntry()))
            if (sCreatureDataMgr.IsSpiritHealer(data))
                m_spiritHealer.insert(*i);
    }

    for(GameObjectSpawnArray::iterator i = mapSpawns->GameObjectSpawns.begin(); i != mapSpawns->GameObjectSpawns.end(); i++)
    {
        uint32 cellx = CellHandler<MapInstance>::GetPosX((*i)->x), celly = CellHandler<MapInstance>::GetPosY((*i)->y);
        GetSpawnsListAndCreate(cellx, celly)->GameObjectSpawns.push_back((*i));
    }
}

void Map::GetClosestGraveyard(uint8 team, uint32 mapId, float x, float y, float z, uint32 zoneId, LocationVector &output)
{
    float graveDist = 99999999.f;
    WorldSafeLocation *graveLoc = NULL;
    for (auto itr = _safeLocGraveyards.begin(); itr != _safeLocGraveyards.end(); ++itr)
    {
        WorldSafeLocation *safeLoc = itr->second;
        if (team == TEAM_ALLIANCE && safeLoc->flags & SAFE_LOCFLAG_HORDE)
            continue;
        if (team == TEAM_HORDE && safeLoc->flags & SAFE_LOCFLAG_ALLIANCE)
            continue;

        float delta_x = fabs(safeLoc->x - x), delta_y = fabs(safeLoc->y - y), delta_z = fabs(safeLoc->z - z);
        float dist = (delta_x*delta_x + delta_y * delta_y + delta_z * delta_z);
        if (graveLoc == NULL || dist < graveDist)
        {
            graveLoc = safeLoc;
            graveDist = dist;
        }
    }

    if (graveLoc != NULL)
        output.ChangeCoords(graveLoc->x, graveLoc->y, graveLoc->z, graveLoc->o);
}

void Map::AddSafeLocation(WorldSafeLocsEntry *safeLocation)
{
    WorldSafeLocation *safeLoc = new WorldSafeLocation();
    safeLoc->dbcId = safeLocation->ID;
    safeLoc->flags = SAFE_LOCFLAG_NONE;
    safeLoc->x = safeLocation->x;
    safeLoc->y = safeLocation->y;
    safeLoc->z = safeLocation->z;
    safeLoc->o = 0.f;
    safeLoc->name = safeLocation->locationName;
    safeLoc->zoneId = 0xFFFF;
    // Push new loc to storage to be processed later
    _safeLoc.insert(std::make_pair(safeLoc->dbcId, safeLoc));
}
