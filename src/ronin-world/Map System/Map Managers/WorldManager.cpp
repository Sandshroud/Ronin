/***
 * Demonstrike Core
 */

//
// WorldCreator.cpp
//

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
        for(CreatureSpawnList::iterator i = itr->second.CreatureSpawns.begin(); i != itr->second.CreatureSpawns.end(); i++)
            delete (*i);
        itr->second.CreatureSpawns.clear();
        for(GameObjectSpawnList::iterator i = itr->second.GameObjectSpawns.begin(); i != itr->second.GameObjectSpawns.end(); i++)
            delete (*i);
        itr->second.GameObjectSpawns.clear();
    }
    m_SpawnStorageMap.clear();

    delete WorldStateTemplateManager::getSingletonPtr();
    sInstanceMgr.Destruct();
    delete this;
}

void WorldManager::ParseMapDBC()
{
    for(uint32 i = 0; i < dbcMap.GetNumRows(); i++)
    {
        MapEntry *map = dbcMap.LookupRow(i);
        if(map == NULL)
            continue;
        if(m_loadedMaps.find(map->MapID) != m_loadedMaps.end())
            continue;
        if(!map->IsContinent() && !map->Instanceable())
            continue;

        m_loadedMaps.insert(std::make_pair(map->MapID, map));
    }
}

void WorldManager::LoadSpawnData()
{
    ProcessPreSpawnLoadTables();

    for(std::map<uint32, MapEntry*>::iterator itr = m_loadedMaps.begin(); itr != m_loadedMaps.end(); itr++)
    {
        if(!itr->second->IsContinent())
            continue;

        uint32 count = 0;
        std::map<uint32, CreatureSpawn*> m_creatureDataShortcut;
        if(QueryResult *checkRes = WorldDatabase.Query("SHOW TABLES LIKE 'world_data_%03u_creatures'", itr->second->MapID))
        {
            delete checkRes;
            if(QueryResult *result = WorldDatabase.Query("SELECT id, entry, position_x, position_y, position_z, orientation, modelId, phaseMask, eventId, conditionId, vendorMask FROM world_data_%03u_creatures", itr->second->MapID))
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
                }while(result->NextRow());
                delete result;
            }
        } else sLog.Error("WorldManager", "Continent %s is missing creature spawn table!", itr->second->name);

        std::set<CreatureSpawn*> m_handledSpawns;
        if(QueryResult *checkRes = WorldDatabase.Query("SHOW TABLES LIKE 'world_data_%03u_creatures'", itr->second->MapID))
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
                    waypoint->actionId = fields[8].GetUInt32();
                    cspawn->m_waypointData.insert(std::make_pair(point, waypoint));
                    if(m_handledSpawns.find(cspawn) == m_handledSpawns.end())
                        m_handledSpawns.insert(cspawn);
                }while(result->NextRow());
                delete result;
            }
        } else sLog.Error("WorldManager", "Continent %s is missing creature waypoint table!", itr->second->MapID);
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
            if(QueryResult *result = WorldDatabase.Query("SELECT id, entry, position_x, position_y, position_z, rotationX, rotationY, rotationZ, rotationAngle, state, flags, faction, scale, phaseMask, eventId, conditionId FROM world_data_%03u_gameobjects", itr->second->MapID))
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
    }

}

void WorldManager::LoadMapTileData(TaskList & tl)
{
    if(sWorld.ServerPreloading == 0)
        return;

    sLog.Notice("WorldManager", "Preloading map tile data");
    for(std::map<uint32, MapEntry*>::iterator itr = m_loadedMaps.begin(); itr != m_loadedMaps.end(); itr++)
    {
        if(!itr->second->IsContinent())
        {
            if(true == true)
                continue;
        }

        tl.AddTask(new Task(new CallbackP1<WorldManager, MapEntry*>(this, &WorldManager::_LoadTileData, itr->second)));
    }
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
    }

    return (m_maps.find(mapId) == m_maps.end()) ? 1 : 0;
}

void WorldManager::Load(TaskList * l)
{
    new WorldStateTemplateManager();
    sWorldStateTemplateManager.LoadFromDB();

    sInstanceMgr.Prepare();

    // create maps for any we don't have yet.
    for(std::map<uint32, MapEntry*>::iterator itr = m_loadedMaps.begin(); itr != m_loadedMaps.end(); itr++)
        l->AddTask(new Task(new CallbackP1<WorldManager, MapEntry*>(this, &WorldManager::_CreateMap, itr->second)));

    l->wait();

    for(auto itr = m_continentManagement.begin(); itr != m_continentManagement.end(); itr++)
        itr->second->SetThreadState(THREADSTATE_AWAITING);

    // load saved instances
    _LoadInstances();

    sInstanceMgr.Launch();
}

void WorldManager::Shutdown()
{
    // Map manager threads are self cleanup
    m_continentManagement.clear();
}

uint32 WorldManager::PreTeleport(uint32 mapId, Player* plr, uint32 &instanceid)
{
    // preteleport is where all the magic happens :P instance creation, etc.
    MapEntry *map = dbcMap.LookupEntry(mapId);
    if(map == NULL) //is the map vaild?
        return INSTANCE_ABORT_NOT_FOUND;

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

    if(map->IsRaid()) // check that heroic mode is available if the player has requested it.
    {
        if(plr->iRaidType > 1 && (map->mapFlags & 0x100) == 0)
            return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;
    }
    else if(plr->iInstanceType && (map->mapFlags & 0x100) == 0)
        return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;

    //do we need addition raid/heroic checks?
    Group * pGroup = plr->GetGroup();

    // players without groups cannot enter raid instances (no soloing them:P)
    if( pGroup == NULL && (map->IsRaid() || (map->mapFlags & 0x100)))
        return INSTANCE_ABORT_NOT_IN_RAID_GROUP;

    //and has the required level
    if( plr->getLevel() < 80)
    {
        if(uint32 instanceType = map->IsRaid() ? plr->iRaidType : plr->iInstanceType)
        {
            if(map->addon == 3 && plr->getLevel() < 90)
                return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;
            else if(map->addon == 2 && plr->getLevel() < 80)
                return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;
            else if(map->addon == 1 && plr->getLevel() < 70)
                return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;
            else if(map->addon)
                return INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE;
        }

        //Instance keys
    }

    // if we are here, it means:
    // 1) we're a non-raid instance
    // 2) we're a raid instance, and the person is in a group.
    // so, first we have to check if they have an instance on this map already, if so, allow them to teleport to that.
    // next we check if there is a saved instance belonging to him.
    // otherwise, we can create them a new one.
    if(instanceid == 0)
        instanceid = plr->GetLinkedInstanceID(map);

    uint32 res = sInstanceMgr.PreTeleportInstanceCheck(plr->GetGUID(), mapId, instanceid, plr->CanCreateNewDungeon(mapId));
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
            if( p->GetGroup()== NULL && (mapInstance->IsRaid() || mapInstance->GetdbcMap()->IsMultiDifficulty()))
                return false;

            p->m_beingPushed = true;
            if(WorldSession *sess = p->GetSession())
                sess->SetEventInstanceId(mapInstance->GetInstanceID());

            if(!mapInstance->IsRaid()) p->LinkToInstance(mapInstance);
        } else if(Creature *c = obj->IsCreature() ? castPtr<Creature>(obj) : NULL)
            if(!c->CanAddToWorld())
                return false;

        mapInstance->AddObject(obj);
        return true;
    }
    return false;
}

MapInstance *WorldManager::GetInstance(WorldObject* obj)
{
    if(ContinentManager *manager = GetContinentManager(obj->GetMapId()))
        return manager->GetContinent();
    /*else if(BattleGroundManager *manager = GetBattleGroundManager(obj->GetMapId()))
        return manager->GetBattleground(obj->GetBGInstanceID());*/
    else if(MapInstance *instance = sInstanceMgr.GetInstanceForObject(obj))
        return instance;
    return NULL;
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
    else if(mapEntry->IsRaid() || mapEntry->IsDungeon())
        _InitializeInstance(mapEntry, map);
    else { m_mapLock.Acquire(); m_maps.erase(mapEntry->MapID); delete map; m_mapLock.Release(); }
}

void WorldManager::BuildXMLStats(char * m_file)
{
    for(auto itr = m_continentManagement.begin(); itr != m_continentManagement.end(); itr++)
        ;//itr->second->BuildStats(m_file);

    sLog.Debug("WorldManager", "Dumping XML stats...");
}

void WorldManager::_LoadInstances()
{
    // clear any instances that have expired.
    sLog.Notice("WorldManager", "Deleting Expired Instances...");
    CharacterDatabase.WaitExecute("DELETE FROM instances WHERE expiration <= %u", UNIXTIME);

    // load saved instances
    if(QueryResult *result = CharacterDatabase.Query("SELECT * FROM instances"))
    {
        uint32 count = 0;
        do
        {
            //if(LoadInstance(result->Fetch()))
                count++;
        } while(result->NextRow());
        delete result;

        sLog.Success("WorldManager", "Loaded %u saved instance(s)." , count);
    } else sLog.Debug("WorldManager", "No saved instances found.");
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
    plr->GetSession()->SendPacket(&data);

    for(std::set<uint32>::iterator itr = mapIds.begin(); itr != mapIds.end(); itr++)
        plr->GetSession()->OutPacket(SMSG_UPDATE_LAST_INSTANCE, 4, ((uint8*)&(*itr)));
}

void WorldManager::BuildSavedRaidInstancesForPlayer(Player* plr)
{
    uint32 counter = 0;
    WorldPacket data(SMSG_RAID_INSTANCE_INFO, 200);
    data << counter;
    *(uint32*)&data.contents()[0] = counter;
    plr->GetSession()->SendPacket(&data);
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
    ThreadPool.ExecuteTask(format("ContinentMgr - M%u", mapEntry->MapID).c_str(), mgr);
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

void WorldManager::ProcessPreSpawnLoadTables()
{
    // These are largely uncessesary, but we'll store them here for the future
    //////////////////// Creature Spawns //////////////////////
    static const char *creatureDataTables[][2] = 
    {
        // Event data is held seperately, so combine our two tables
        { "world_data_creature_events", "UPDATE %s AS T INNER JOIN world_data_creature_events AS S ON (T.id = S.guid) SET T.eventId = S.event;" },
        // Condition data is held as multiple tables by different teams, so we have a list to go through
        { "world_data_creature_pools", "UPDATE %s AS T INNER JOIN world_data_creature_pools AS S ON (T.id = S.guid) SET T.conditionId = S.pool_entry;" },
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
        { "world_data_gameobject_orientation", "UPDATE %s AS T INNER JOIN world_data_gameobject_orientation AS S ON (T.id = S.guid) SET T.rotationZ = SIN(CAST(S.orientation AS float)/2.0), T.rotationAngle = COS(CAST(S.orientation AS float)/2.0);" },
        // Event data is held seperately, so combine our two tables
        { "world_data_gameobject_events", "UPDATE %s AS T INNER JOIN world_data_gameobject_events AS S ON (T.id = S.guid) SET T.eventId = S.eventEntry;" },
        // Condition data is held as multiple tables by different teams, so we have a list to go through
        { "world_data_gameobject_pools", "UPDATE %s AS T INNER JOIN world_data_gameobject_pools AS S ON (T.id = S.guid) SET T.conditionId = S.pool_entry;" },
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
