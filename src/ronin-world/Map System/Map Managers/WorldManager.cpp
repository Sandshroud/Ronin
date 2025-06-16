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

#include "StdAfx.h"

initialiseSingleton( WorldManager );

static const uint32 MapInstanceUpdatePeriod = 50;

WorldManager::WorldManager() { }
WorldManager::~WorldManager() { }

void WorldManager::Destruct()
{
    m_loadedMaps.clear();
    for(std::map<uint32, Map*>::iterator itr = m_maps.begin(); itr != m_maps.end(); itr++)
        delete itr->second;
    m_maps.clear();
    for(std::map<uint32, CellSpawns>::iterator itr = m_SpawnStorageMap.begin(); itr != m_SpawnStorageMap.end(); itr++)
    {
        for(CreatureSpawnArray::iterator i = itr->second.CreatureSpawns.begin(); i != itr->second.CreatureSpawns.end(); i++)
        {
            for(WaypointStorage::iterator itr = (*i)->m_waypointData.begin(); itr != (*i)->m_waypointData.end(); itr++)
                delete itr->second;
            (*i)->m_waypointData.clear();
            delete (*i);
        }
        itr->second.CreatureSpawns.clear();
        for(GameObjectSpawnArray::iterator i = itr->second.GameObjectSpawns.begin(); i != itr->second.GameObjectSpawns.end(); i++)
            delete (*i);
        itr->second.GameObjectSpawns.clear();
    }
    m_SpawnStorageMap.clear();

    delete WorldStateTemplateManager::getSingletonPtr();
    sInstanceMgr.Destruct();
    delete this;
}

void WorldManager::ParseMapDBCFiles()
{
    sLog.Notice("WorldManager", "Processing %u map entries...", dbcMap.GetNumRows());
    for(uint32 i = 0; i < dbcMap.GetNumRows(); i++)
    {
        MapEntry *map = dbcMap.LookupRow(i);
        if(map == NULL)
            continue;
        if(m_loadedMaps.find(map->MapID) != m_loadedMaps.end())
            continue;
        if(!(map->IsContinent() || map->Instanceable()))
            continue;

        m_loadedMaps.insert(std::make_pair(map->MapID, map));
    }

    std::map<std::string, AreaTableEntry *> areaTableByName;
    sLog.Notice("WorldManager", "Processing %u area table entries...", dbcAreaTable.GetNumRows());
    for (uint32 i = 0; i < dbcAreaTable.GetNumRows(); i++)
    {
        AreaTableEntry *areaEntry = dbcAreaTable.LookupRow(i);

        if (!sWorld.IsSanctuaryArea(areaEntry->AreaId) && (areaEntry->category == AREAC_SANCTUARY || areaEntry->AreaFlags & AREA_SANCTUARY))
            sWorld.SetSanctuaryArea(areaEntry->AreaId);

        if (sWorld.GetRestedAreaInfo(areaEntry->AreaId) == NULL && (areaEntry->AreaFlags & AREA_CITY_AREA || areaEntry->AreaFlags & AREA_CITY || areaEntry->AreaFlags & AREA_CAPITAL_SUB || areaEntry->AreaFlags & AREA_CAPITAL))
        {
            int8 team = -1;
            if (areaEntry->category == AREAC_ALLIANCE_TERRITORY)
                team = TEAM_ALLIANCE;
            if (areaEntry->category == AREAC_HORDE_TERRITORY)
                team = TEAM_HORDE;
            sWorld.SetRestedArea(areaEntry->AreaId, team);
        }

        areaTableByName.insert(std::make_pair(areaEntry->name, areaEntry));

        uint32 mapId = areaEntry->mapId;
        if (m_mapAreaIds[mapId].find(areaEntry->AreaId) == m_mapAreaIds[mapId].end())
            m_mapAreaIds[mapId].insert(areaEntry->AreaId);
        if (areaEntry->AreaId && areaEntry->ZoneId == 0 && m_mapZoneIds[mapId].find(areaEntry->AreaId) == m_mapZoneIds[mapId].end())
            m_mapZoneIds[mapId].insert(areaEntry->AreaId);
    }
}

void WorldManager::LoadSpawnData()
{
    uint32 count = 0;
    std::map<uint32, CreatureSpawn*> m_creatureDataShortcut;
    ProcessPreSpawnLoadTables();

    for(std::map<uint32, MapEntry*>::iterator itr = m_loadedMaps.begin(); itr != m_loadedMaps.end(); itr++)
    {
        if(!itr->second->IsContinent())
            continue;

        if(QueryResult *checkRes = WorldDatabase.Query("SHOW TABLES LIKE 'world_data_%03u_creatures'", itr->second->MapID))
        {
            delete checkRes;
            if(QueryResult *result = WorldDatabase.Query("SELECT guid, entry, position_x, position_y, position_z, orientation, modelId, phaseMask, eventId, conditionId, vendorMask FROM world_data_%03u_creatures", itr->second->MapID))
            {
                do
                {
                    count++;
                    Field * fields = result->Fetch();
                    uint32 entry = fields[1].GetUInt32();
                    CreatureData *data;
                    if((data = sCreatureDataMgr.GetCreatureData(entry)) == NULL)
                        continue;

                    CreatureSpawn *cspawn = new CreatureSpawn();
                    cspawn->guid = MAKE_NEW_GUID(fields[0].GetUInt32(), entry, (data->vehicleEntry > 0 ? HIGHGUID_TYPE_VEHICLE : HIGHGUID_TYPE_UNIT));
                    cspawn->x = fields[2].GetFloat();
                    cspawn->y = fields[3].GetFloat();
                    cspawn->z = fields[4].GetFloat();
                    cspawn->o = NormAngle(fields[5].GetFloat());
                    cspawn->modelId = fields[6].GetUInt32();
                    cspawn->phaseMask = fields[7].GetUInt16();
                    cspawn->eventId = fields[8].GetUInt32();
                    cspawn->conditionId = fields[9].GetUInt32();
                    cspawn->vendormask = fields[10].GetInt32();
                    m_SpawnStorageMap[itr->second->MapID].CreatureSpawns.push_back(cspawn);
                    m_creatureDataShortcut.insert(std::make_pair(cspawn->guid.getLow(), cspawn));
                    // Push our spirit healer into our map
                    if(sCreatureDataMgr.IsSpiritHealer(data))
                        m_SpawnStorageMap[itr->second->MapID].SpiritHealers.insert(m_creatureDataShortcut.size()-1);
                }while(result->NextRow());
                delete result;
            }
        } else sLog.Error("WorldManager", "Continent %s is missing creature spawn table!", itr->second->name);

        std::set<CreatureSpawn*> m_handledSpawns;
        if(QueryResult *checkRes = WorldDatabase.Query("SHOW TABLES LIKE 'world_data_%03u_waypoints'", itr->second->MapID))
        {
            delete checkRes;
            if(QueryResult *result = WorldDatabase.Query("SELECT guid, point, movetype, position_x, position_y, position_z, orientation, delay, actionid FROM world_data_%03u_waypoints", itr->second->MapID))
            {
                do
                {
                    Field * fields = result->Fetch();
                    uint32 guid = fields[0].GetUInt32();
                    uint32 point = fields[1].GetUInt32();
                    if(m_creatureDataShortcut.find(guid) == m_creatureDataShortcut.end())
                    {
                        sLog.outDebug("");
                        continue;
                    }
                    CreatureSpawn *cspawn = m_creatureDataShortcut.at(guid);
                    // Create our waypoint to push to our spawn
                    CreatureWaypoint *waypoint = new CreatureWaypoint();
                    waypoint->moveType = fields[2].GetUInt32();
                    waypoint->x = fields[3].GetFloat();
                    waypoint->y = fields[4].GetFloat();
                    waypoint->z = fields[5].GetFloat();
                    waypoint->o = NormAngle(fields[6].GetFloat());
                    waypoint->delay = fields[7].GetUInt32();
                    waypoint->actionId = fields[8].GetUInt32();
                    cspawn->m_waypointData.insert(std::make_pair(point, waypoint));
                    if(m_handledSpawns.find(cspawn) == m_handledSpawns.end())
                        m_handledSpawns.insert(cspawn);
                }while(result->NextRow());
                delete result;
            }
        } else sLog.Error("WorldManager", "Continent %s is missing creature waypoint table!", itr->second->name);
        m_creatureDataShortcut.clear();

        // Push our spawn point to the beginning of our waypoint map
        for(std::set<CreatureSpawn*>::iterator itr = m_handledSpawns.begin(); itr != m_handledSpawns.end(); itr++)
        {
            CreatureWaypoint *waypoint = new CreatureWaypoint();
            waypoint->moveType = waypoint->delay = waypoint->actionId = 0;
            waypoint->x = (*itr)->x;
            waypoint->y = (*itr)->y;
            waypoint->z = (*itr)->z;
            waypoint->o = (*itr)->o;
            (*itr)->m_waypointData.insert(std::make_pair(0, waypoint));
        }
        m_handledSpawns.clear();

        if(QueryResult *checkRes = WorldDatabase.Query("SHOW TABLES LIKE 'world_data_%03u_gameobjects'", itr->second->MapID))
        {
            delete checkRes;
            if(QueryResult *result = WorldDatabase.Query("SELECT guid, entry, position_x, position_y, position_z, rotationX, rotationY, rotationZ, rotationAngle, state, flags, faction, scale, phaseMask, eventId, conditionId FROM world_data_%03u_gameobjects", itr->second->MapID))
            {
                do
                {
                    count++;
                    Field * fields = result->Fetch();
                    uint32 entry = fields[1].GetUInt32();
                    GameObjectInfo *info;
                    if((info = GameObjectNameStorage.LookupEntry(entry)) == NULL)
                        continue;

                    GameObjectSpawn *gspawn = new GameObjectSpawn();
                    gspawn->guid = MAKE_NEW_GUID(fields[0].GetUInt32(), entry, HIGHGUID_TYPE_GAMEOBJECT);
                    gspawn->x = fields[2].GetFloat();
                    gspawn->y = fields[3].GetFloat();
                    gspawn->z = fields[4].GetFloat();
                    gspawn->rX = fields[5].GetFloat();
                    gspawn->rY = fields[6].GetFloat();
                    gspawn->rZ = fields[7].GetFloat();
                    gspawn->rAngle = fields[8].GetFloat();
                    gspawn->state = fields[9].GetUInt32();
                    gspawn->flags = fields[10].GetUInt32();
                    gspawn->faction = fields[11].GetUInt32();
                    gspawn->scale = std::min<float>(255.f, fields[12].GetFloat());
                    gspawn->phaseMask = fields[13].GetUInt16();
                    gspawn->eventId = fields[14].GetUInt32();
                    gspawn->conditionId = fields[15].GetUInt32();
                    m_SpawnStorageMap[itr->second->MapID].GameObjectSpawns.push_back(gspawn);
                }while(result->NextRow());
                delete result;
            }
        } else sLog.Error("WorldManager", "Continent %s is missing gameobject spawn table!", itr->second->name);

        if(count == 0)
            continue;

        sLog.Notice("WorldManager", "%u spawns for %s loaded into storage.", count, itr->second->name);
        count = 0;
    }

    if(QueryResult *checkRes = WorldDatabase.Query("SHOW TABLES LIKE 'instance_data_creatures'"))
    {
        delete checkRes;
        if(QueryResult *result = WorldDatabase.Query("SELECT id, map, entry, position_x, position_y, position_z, orientation, modelId, phaseMask, eventId, conditionId, vendorMask FROM instance_data_creatures"))
        {
            do
            {
                count++;
                Field * fields = result->Fetch();
                uint32 mapId = fields[1].GetUInt32();
                if(m_loadedMaps.find(mapId) == m_loadedMaps.end())
                    continue;

                uint32 entry = fields[2].GetUInt32();
                CreatureData *data;
                if((data = sCreatureDataMgr.GetCreatureData(entry)) == NULL)
                    continue;

                CreatureSpawn *cspawn = new CreatureSpawn();
                cspawn->guid = MAKE_NEW_GUID(fields[0].GetUInt32(), entry, (data->vehicleEntry > 0 ? HIGHGUID_TYPE_VEHICLE : HIGHGUID_TYPE_UNIT));
                cspawn->x = fields[3].GetFloat();
                cspawn->y = fields[4].GetFloat();
                cspawn->z = fields[5].GetFloat();
                cspawn->o = NormAngle(fields[6].GetFloat());
                cspawn->modelId = fields[7].GetUInt32();
                cspawn->phaseMask = fields[8].GetUInt16();
                cspawn->eventId = fields[9].GetUInt32();
                cspawn->conditionId = fields[10].GetUInt32();
                cspawn->vendormask = fields[11].GetInt32();
                m_SpawnStorageMap[mapId].CreatureSpawns.push_back(cspawn);
                m_creatureDataShortcut.insert(std::make_pair(cspawn->guid.getLow(), cspawn));
            }while(result->NextRow());
            delete result;
        }
    } else sLog.Error("WorldManager", "Instance data creature spawn table is missing!");

    std::set<CreatureSpawn*> m_handledSpawns;
    if(QueryResult *checkRes = WorldDatabase.Query("SHOW TABLES LIKE 'instance_data_waypoints'"))
    {
        delete checkRes;
        if(QueryResult *result = WorldDatabase.Query("SELECT id, map, point, movetype, position_x, position_y, position_z, orientation, delay, actionid FROM instance_data_waypoints"))
        {
            do
            {
                Field * fields = result->Fetch();
                uint32 guid = fields[0].GetUInt32();
                uint32 mapId = fields[1].GetUInt32();
                if(m_loadedMaps.find(mapId) == m_loadedMaps.end())
                    continue;
                uint32 point = fields[2].GetUInt32();
                if(m_creatureDataShortcut.find(guid) == m_creatureDataShortcut.end())
                {
                    sLog.outDebug("");
                    continue;
                }

                CreatureSpawn *cspawn = m_creatureDataShortcut.at(guid);
                // Create our waypoint to push to our spawn
                CreatureWaypoint *waypoint = new CreatureWaypoint();
                waypoint->moveType = fields[3].GetUInt32();
                waypoint->x = fields[4].GetFloat();
                waypoint->y = fields[5].GetFloat();
                waypoint->z = fields[6].GetFloat();
                waypoint->o = NormAngle(fields[7].GetFloat());
                waypoint->delay = fields[8].GetUInt32();
                waypoint->actionId = fields[9].GetUInt32();
                cspawn->m_waypointData.insert(std::make_pair(point, waypoint));
                if(m_handledSpawns.find(cspawn) == m_handledSpawns.end())
                    m_handledSpawns.insert(cspawn);
            }while(result->NextRow());
            delete result;
        }
    } else sLog.Error("WorldManager", "Instance data creature waypoint table is missing!");
    m_creatureDataShortcut.clear();

    // Push our spawn point to the beginning of our waypoint map
    for(std::set<CreatureSpawn*>::iterator itr = m_handledSpawns.begin(); itr != m_handledSpawns.end(); itr++)
    {
        CreatureWaypoint *waypoint = new CreatureWaypoint();
        waypoint->moveType = waypoint->delay = waypoint->actionId = 0;
        waypoint->x = (*itr)->x;
        waypoint->y = (*itr)->y;
        waypoint->z = (*itr)->z;
        waypoint->o = (*itr)->o;
        (*itr)->m_waypointData.insert(std::make_pair(0, waypoint));
    }
    m_handledSpawns.clear();

    if(QueryResult *checkRes = WorldDatabase.Query("SHOW TABLES LIKE 'instance_data_gameobjects'"))
    {
        delete checkRes;
        if(QueryResult *result = WorldDatabase.Query("SELECT id, map, entry, position_x, position_y, position_z, rotationX, rotationY, rotationZ, rotationAngle, state, flags, faction, scale, phaseMask, eventId, conditionId FROM instance_data_gameobjects"))
        {
            do
            {
                count++;
                Field * fields = result->Fetch();
                uint32 mapId = fields[1].GetUInt32();
                if(m_loadedMaps.find(mapId) == m_loadedMaps.end())
                    continue;
                uint32 entry = fields[2].GetUInt32();

                GameObjectInfo *info;
                if((info = GameObjectNameStorage.LookupEntry(entry)) == NULL)
                    continue;

                GameObjectSpawn *gspawn = new GameObjectSpawn();
                gspawn->guid = MAKE_NEW_GUID(fields[0].GetUInt32(), entry, HIGHGUID_TYPE_GAMEOBJECT);
                gspawn->x = fields[3].GetFloat();
                gspawn->y = fields[4].GetFloat();
                gspawn->z = fields[5].GetFloat();
                gspawn->rX = fields[6].GetFloat();
                gspawn->rY = fields[7].GetFloat();
                gspawn->rZ = fields[8].GetFloat();
                gspawn->rAngle = fields[9].GetFloat();
                gspawn->state = fields[10].GetUInt32();
                gspawn->flags = fields[11].GetUInt32();
                gspawn->faction = fields[12].GetUInt32();
                gspawn->scale = std::min<float>(255.f, fields[13].GetFloat());
                gspawn->phaseMask = fields[14].GetUInt16();
                gspawn->eventId = fields[15].GetUInt32();
                gspawn->conditionId = fields[16].GetUInt32();
                m_SpawnStorageMap[mapId].GameObjectSpawns.push_back(gspawn);
            }while(result->NextRow());
            delete result;
        }
    } else sLog.Error("WorldManager", "Instance data gameobject data table is missing!");

    sLog.Notice("WorldManager", "%u spawns for instances loaded.", count);
}

void WorldManager::LoadMapTileData(ThreadTaskList & tl)
{
    if(sWorld.ServerPreloading == 0)
        return;

    sLog.Notice("WorldManager", "Preloading map tile data");
    for(std::map<uint32, MapEntry*>::iterator itr = m_loadedMaps.begin(); itr != m_loadedMaps.end(); itr++)
    {
        if(itr->second->IsBattleArena() || itr->second->IsBattleGround())
            ;//sInstanceMgr.LoadTileData(itr->second);
        else if(itr->second->IsRaid())
            ;//sRaidMgr.LoadTileData(itr->second);
        else if(itr->second->IsDungeon())
            ;//sInstanceMgr.LoadTileData(itr->second);
        else tl.AddTask(new CallbackP1<WorldManager, MapEntry*>(this, &WorldManager::_LoadTileData, itr->second));
    }
}

void WorldManager::_CreateMap(MapEntry *mapEntry)
{
    m_mapLock.Acquire();
    Map *map = NULL;
    if(m_maps.find(mapEntry->MapID) == m_maps.end())
        m_maps.insert(std::make_pair(mapEntry->MapID, map = new Map(mapEntry, mapEntry->name)));
    else map = m_maps.at(mapEntry->MapID);
    m_mapLock.Release();

    if(mapEntry->IsContinent())
        _InitializeContinent(mapEntry, map);
    else if(mapEntry->IsBattleGround() || mapEntry->IsBattleArena())
        _InitializeBattleGround(mapEntry, map);
    else if(mapEntry->IsDungeon())
        _InitializeInstance(mapEntry, map);
    else if(mapEntry->IsRaid())
        _InitializeRaid(mapEntry, map);
    else { m_mapLock.Acquire(); m_maps.erase(mapEntry->MapID); delete map; m_mapLock.Release(); }
}

void WorldManager::_ProcessMapData(Map *map)
{
    map->ProcessInputData();
}

void WorldManager::FillMapSafeLocations()
{
    Map* map;
    WorldSafeLocsEntry *safeLocation;
    for (uint32 i = 0; i < dbcWorldSafeLocs.GetNumRows(); ++i)
        if ((safeLocation = dbcWorldSafeLocs.LookupRow(i)) && (map = GetMap(safeLocation->map_id)))
            map->AddSafeLocation(safeLocation);

    sLog.Notice("WorldManager", "Finished processing %u world locations...", dbcWorldSafeLocs.GetNumRows());
}

void WorldManager::ContinentUnloaded(uint32 mapId)
{
    mapLoadLock.Acquire();
    // Make sure we're not marked as loading
    m_loadingMaps.erase(mapId);
    mapLoadLock.Release();
    // Store the manager in the worldManager thread
    m_continentManagement.erase(mapId);
}

uint8 WorldManager::ValidateMapId(uint32 mapId)
{
    if(m_loadedMaps.find(mapId) == m_loadedMaps.end())
        return 1;

    if(ContinentManagerExists(mapId))
    {
        ContinentManager *mgr = GetContinentManager(mapId);
        if(mgr->GetContinent()->IsPreloading())
            return 2;
        return 0;
    } else if(m_loadedMaps.at(mapId)->IsContinent())
        return 1;

    return (m_maps.find(mapId) == m_maps.end()) ? 1 : 0;
}

void WorldManager::Load(ThreadTaskList * l)
{
    new WorldStateTemplateManager();
    sWorldStateTemplateManager.LoadFromDB();

    sInstanceMgr.Prepare();

    // create maps for any we don't have yet.
    for(std::map<uint32, MapEntry*>::iterator itr = m_loadedMaps.begin(); itr != m_loadedMaps.end(); itr++)
        l->AddTask(new CallbackP1<WorldManager, MapEntry*>(this, &WorldManager::_CreateMap, itr->second));

    l->wait(UNIXTIME, g_localTime);

    // Push our world safe locations to our map storage
    FillMapSafeLocations();

    // Queue our maps for processing
    for(std::map<uint32, Map*>::iterator itr = m_maps.begin(); itr != m_maps.end(); itr++)
        l->AddTask(new CallbackP1<WorldManager, Map*>(this, &WorldManager::_ProcessMapData, itr->second));

    l->wait(UNIXTIME, g_localTime);

    for(auto itr = m_continentManagement.begin(); itr != m_continentManagement.end(); itr++)
        itr->second->SetThreadState(THREADSTATE_AWAITING);

    // load saved instances
    sInstanceMgr._LoadInstances();

    sInstanceMgr.Launch();
}

void WorldManager::Shutdown()
{
    // Map manager threads are self cleanup
    m_continentManagement.clear();
}

uint32 WorldManager::PreTeleport(uint32 mapId, Player* plr, uint32 &instanceid, bool groupFinderDungeon)
{
    // preteleport is where all the magic happens :P instance creation, etc.
    MapEntry *map = dbcMap.LookupEntry(mapId);
    if(map == NULL) //is the map vaild?
        return INSTANCE_ABORT_NOT_FOUND;

    uint32 instanceDiff = plr->GetDifficulty(map);

    // main continent check.
    if(map->IsContinent())
    {
        // we can check if the destination world server is online or not and then cancel them before they load.
        if(ContinentManagerExists(mapId))
            return INSTANCE_OK;
        return INSTANCE_ABORT_NOT_FOUND;
    }

    // shouldn't happen
    if(map->IsBattleGround() || map->IsBattleArena())
        return INSTANCE_ABORT_NOT_FOUND;

    if(map->IsRaid() && instanceDiff > 1 && (map->mapFlags & 0x100) == 0)
        return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;
    if(instanceDiff && (map->mapFlags & 0x100) == 0)
        return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;

    //do we need addition raid/heroic checks?
    Group * pGroup = plr->GetGroup();

    // players without groups cannot enter raid instances (no soloing them:P)
    if( pGroup == NULL && (map->IsRaid() || (map->mapFlags & 0x100)))
        return INSTANCE_ABORT_NOT_IN_RAID_GROUP;

    //and has the required level
    if(plr->getLevel() < MAXIMUM_CEXPANSION_LEVEL && instanceDiff)
    {
        if(map->addon == 3 && plr->getLevel() < 85)
            return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;
        else if(map->addon == 2 && plr->getLevel() < 80)
            return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;
        else if(map->addon == 1 && plr->getLevel() < 70)
            return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;
    }

    // if we are here, it means:
    // 1) we're a non-raid instance
    // 2) we're a raid instance, and the person is in a group.
    // so, first we have to check if they have an instance on this map already, if so, allow them to teleport to that.
    // next we check if there is a saved instance belonging to him.
    // otherwise, we can create them a new one.
    if(!sInstanceMgr.GetLinkedInstanceID(plr, map, instanceDiff, instanceid, groupFinderDungeon))
        return INSTANCE_ABORT_TOO_MANY;

    uint32 res = sInstanceMgr.PreTeleportInstanceCheck(plr->getPlayerInfo(), map, instanceid);
    if(res == INSTANCE_ABORT_CREATE_NEW_INSTANCE)
    {
        instanceid = 0;
        return INSTANCE_OK;
    }

    return res;
}

bool WorldManager::PushToWorldQueue(WorldObject *obj)
{
    if(MapInstance* mapInstance = GetInstance(obj))
    {
        if(Player* p = obj->IsPlayer() ? castPtr<Player>(obj) : NULL)
        {
            // players who's group disbanded cannot remain in a raid instances alone(no soloing them:P)
            if( p->GetGroup() == NULL && mapInstance->IsRaid())
                return false;

            p->m_beingPushed = true;
            if(WorldSession *sess = p->GetSession())
                sess->SetEventInstanceId(mapInstance->GetInstanceID());

            /*if(!mapInstance->IsRaid())
                p->LinkToInstance(mapInstance);*/
        }

        mapInstance->AddObject(obj);
        return true;
    }
    return false;
}

MapInstance *WorldManager::GetInstance(WorldObject* obj)
{
    if(ContinentManager *manager = GetContinentManager(obj->GetMapId()))
        return manager->GetContinent();
    else
    {
        /*if(BattleGroundManager *manager = GetBattleGroundManager(obj->GetMapId()))
            return manager->GetBattleground(obj->GetBGInstanceID());*/
        if(MapInstance *instance = sRaidMgr.GetInstanceForObject(obj))
            return instance;
        else if(MapInstance *instance = sInstanceMgr.GetInstanceForObject(obj))
            return instance;
    }
    return NULL;
}

void WorldManager::BuildXMLStats(char * m_file)
{
    for(auto itr = m_continentManagement.begin(); itr != m_continentManagement.end(); itr++)
        ;//itr->second->BuildStats(m_file);

    sLog.Debug("WorldManager", "Dumping XML stats...");
}

void WorldManager::SendHeroicResetWarning()
{

}

void WorldManager::ResetHeroicInstances()
{

}

bool WorldManager::_DeleteInstance(MapInstance * in, bool ForcePlayersOut, bool atSelfEnd)
{
    m_mapLock.Acquire();

    m_mapLock.Release();
    return true;
}

void WorldManager::CheckForExpiredInstances()
{

}

void WorldManager::BuildSavedInstancesForPlayer(Player* plr)
{
    std::set<uint32> mapIds;

    WorldPacket data(SMSG_UPDATE_INSTANCE_OWNERSHIP, 4);
    data << uint32(mapIds.size() ? 0x01 : 0x00);
    plr->PushPacket(&data);

    for(std::set<uint32>::iterator itr = mapIds.begin(); itr != mapIds.end(); itr++)
        plr->PushData(SMSG_UPDATE_LAST_INSTANCE, 4, ((uint8*)&(*itr)));
}

void WorldManager::BuildSavedRaidInstancesForPlayer(Player* plr)
{
    uint32 counter = 0;
    WorldPacket data(SMSG_RAID_INSTANCE_INFO, 200);
    data << counter;
    data.put<uint32>(0, counter);
    plr->PushPacket(&data);
}

void WorldManager::PlayerLeftGroup(Group * pGroup, Player* pPlayer)
{

}

MapInstance* WorldManager::CreateBattlegroundInstance(uint32 mapid)
{
    return NULL;
}

void WorldManager::DeleteBattlegroundInstance(uint32 mapid, uint32 instanceid)
{
    m_mapLock.Acquire();
    sLog.printf("Could not delete battleground instance!\n");
    m_mapLock.Release();
}

void WorldManager::_LoadTileData(MapEntry *mapEntry)
{
    Map *map = NULL;
    m_mapLock.Acquire();
    if(m_maps.find(mapEntry->MapID) == m_maps.end())
    {
        map = new Map(mapEntry, mapEntry->name);
        m_maps.insert(std::make_pair(mapEntry->MapID, map));
    }
    m_mapLock.Release();
    if(map == NULL)
        return;

    map->PreloadTerrain(mapEntry->IsContinent());
    sLog.Notice("WorldManager", "Finished loading tiles for map %u...", mapEntry->MapID);
}

void WorldManager::_InitializeContinent(MapEntry *mapEntry, Map *map)
{
    ContinentManager *mgr = new ContinentManager(mapEntry, map);
    if(!mgr->Initialize()) // Initialize the manager before starting the thread
    {
        delete mgr;
        return;
    }

    mapLoadLock.Acquire(); // Mark as loading 
    if(m_loadingMaps.find(mapEntry->MapID) == m_loadingMaps.end())
        m_loadingMaps.insert(std::make_pair(mapEntry->MapID, 1));
    else m_loadingMaps[mapEntry->MapID] += 1;
    mapLoadLock.Release();

    // Initialize the map data first
    map->Initialize(GetSpawn(mapEntry->MapID), true);
    // Store the manager in the worldManager thread
    m_continentManagement.insert(std::make_pair(mapEntry->MapID, mgr));
    // Set manager to start it's internal thread
    sThreadManager.ExecuteTask(format("ContinentMgr - %s", mapEntry->internalname).c_str(), mgr);
}

void WorldManager::_InitializeBattleGround(MapEntry *mapEntry, Map *map)
{
    // Temp: clean up the map for now
    m_mapLock.Acquire();
    m_maps.erase(mapEntry->MapID); delete map;
    m_mapLock.Release();
}

void WorldManager::_InitializeInstance(MapEntry *mapEntry, Map *map)
{
    // Initialize the map data first
    map->Initialize(GetSpawn(mapEntry->MapID), false);

    // Push map data to instance manager
    sInstanceMgr.AddMapData(mapEntry, map);
}

void WorldManager::_InitializeRaid(MapEntry *mapEntry, Map *map)
{
    // Initialize the map data first
    map->Initialize(GetSpawn(mapEntry->MapID), false);

    // Push map data to instance manager
    sRaidMgr.AddMapData(mapEntry, map);
}

void WorldManager::ProcessPreSpawnLoadTables()
{
    // These are largely uncessesary, but we'll store them here for the future
    //////////////////// Creature Spawns //////////////////////
    static const char *creatureDataTables[][2] = 
    {
        // Event data is held seperately, so combine our two tables
        { "world_data_creature_events", "UPDATE %s AS T INNER JOIN world_data_creature_events AS S ON (T.guid = S.guid) SET T.eventId = S.event;" },
        // Condition data is held as multiple tables by different teams, so we have a list to go through
        { "world_data_creature_pools", "UPDATE %s AS T INNER JOIN world_data_creature_pools AS S ON (T.guid = S.guid) SET T.conditionId = S.pool_entry;" },
        // Force disable specific creatures if our disable table exists, helps clean up heavily spawned garbage
        { "world_data_creature_disable", "UPDATE %s SET conditionId = '2147483647' WHERE entry IN(SELECT entry FROM world_data_creature_disable);" },
        // End
        { NULL, NULL }
    };

    for(uint8 i = 0; creatureDataTables[i][0] != NULL; i++)
    {
        if(QueryResult *checkRes = WorldDatabase.Query("SHOW TABLES LIKE '%s'", creatureDataTables[i][0]))
        {
            delete checkRes;

            for(std::map<uint32, MapEntry*>::iterator itr = m_loadedMaps.begin(); itr != m_loadedMaps.end(); itr++)
            {
                if(!itr->second->IsContinent())
                    continue;
                char buff[100];
                sprintf(buff, "world_data_%03u_creatures", itr->first);
                if(checkRes = WorldDatabase.Query("SHOW TABLES LIKE '%s'", buff))
                {
                    delete checkRes;
                    WorldDatabase.Execute(creatureDataTables[i][1], buff);
                }
            }

            WorldDatabase.Execute("DROP TABLE `%s`", creatureDataTables[i][0]);
        }
    }

    /////////////////// Gameobject Spawns /////////////////////
    static const char *gameobjectDataTables[][2] = 
    {
        // Gameobject packed rotation is sometimes stored incorrectly by different teams, here's our orientation override
        { "world_data_gameobject_orientation", "UPDATE %s AS T INNER JOIN world_data_gameobject_orientation AS S ON (T.guid = S.guid) SET T.rotationZ = SIN(CAST(S.orientation AS float)/2.0), T.rotationAngle = COS(CAST(S.orientation AS float)/2.0);" },
        // Though it works fine, most projects store their scale by gameobject entry instead of spawn but we want better customizability
        { "world_data_gameobject_scale", "UPDATE %s AS T INNER JOIN world_data_gameobject_scale AS S ON (T.entry = S.entry) SET T.scale = S.scale;" },
        // Event data is held seperately, so combine our two tables
        { "world_data_gameobject_events", "UPDATE %s AS T INNER JOIN world_data_gameobject_events AS S ON (T.guid = S.guid) SET T.eventId = S.event;" },
        // Condition data is held as multiple tables by different teams, so we have a list to go through
        { "world_data_gameobject_pools", "UPDATE %s AS T INNER JOIN world_data_gameobject_pools AS S ON (T.guid = S.guid) SET T.conditionId = S.pool_entry;" },
        // End
        { NULL, NULL }
    };

    for(uint8 i = 0; gameobjectDataTables[i][0] != NULL; i++)
    {
        if(QueryResult *checkRes = WorldDatabase.Query("SHOW TABLES LIKE '%s'", gameobjectDataTables[i][0]))
        {
            delete checkRes;

            for(std::map<uint32, MapEntry*>::iterator itr = m_loadedMaps.begin(); itr != m_loadedMaps.end(); itr++)
            {
                if(!itr->second->IsContinent())
                    continue;
                char buff[100];
                sprintf(buff, "world_data_%03u_gameobjects", itr->first);
                if(checkRes = WorldDatabase.Query("SHOW TABLES LIKE '%s'", buff))
                {
                    delete checkRes;
                    WorldDatabase.Execute(gameobjectDataTables[i][1], buff);
                }
            }

            WorldDatabase.Execute("DROP TABLE `%s`", gameobjectDataTables[i][0]);
        }
    }
}
