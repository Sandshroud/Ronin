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

#pragma once

extern const uint32 MapInstanceUpdatePeriod;

enum INSTANCE_TYPE
{
    INSTANCE_NULL,
    INSTANCE_RAID,
    INSTANCE_NONRAID,
    INSTANCE_PVP,
    INSTANCE_MULTIMODE,
};

enum MAPENTRY_TYPE
{
    BATTLEGROUND    = 0,
    INSTANCE        = 1,
    RAID            = 2,
    ARENA           = 4,
};

enum INSTANCE_MODE
{
    MODE_5PLAYER_NORMAL = 0,    // DUNGEON_DIFFICULTY_5PLAYER
    MODE_5PLAYER_HEROIC = 1     // DUNGEON_DIFFICULTY_5PLAYER_HEROIC
};

enum RAID_MODE
{
    MODE_10PLAYER_NORMAL    =   0,  // RAID_DIFFICULTY_10PLAYER
    MODE_25PLAYER_NORMAL    =   1,  // RAID_DIFFICULTY_25PLAYER
    MODE_10PLAYER_HEROIC    =   2,  // RAID_DIFFICULTY_10PLAYER_HEROIC
    MODE_25PLAYER_HEROIC    =   3   // RAID_DIFFICULTY_25PLAYER_HEROIC
};

enum INSTANCE_ABORT_ERROR
{
    INSTANCE_ABORT_CREATE_NEW_INSTANCE          = 0,
    INSTANCE_ABORT_ERROR_ERROR                  = 1,
    INSTANCE_ABORT_FULL                         = 2,
    INSTANCE_ABORT_NOT_FOUND                    = 3,
    INSTANCE_ABORT_TOO_MANY                     = 4,
    INSTANCE_ABORT_ENCOUNTER                    = 6,
    INSTANCE_ABORT_NON_CLIENT_TYPE              = 7,
    INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE    = 8,
    INSTANCE_ABORT_CANNOT_LAUNCH_MORE           = 10,
    INSTANCE_ABORT_NOT_IN_RAID_GROUP            = 11,
    INSTANCE_ABORT_MAX_CLIENT_IDS               = 11,

    INSTANCE_ABORT_INSTANCE_CLOSING             = 18,
    INSTANCE_ABORT_RESET_POS                    = 19,
    INSTANCE_OK                                 = 0xFF
};

enum OWNER_CHECK
{
    OWNER_CHECK_ERROR       = 0,
    OWNER_CHECK_EXPIRED     = 1,
    OWNER_CHECK_NOT_EXIST   = 2,
    OWNER_CHECK_NO_GROUP    = 3,
    OWNER_CHECK_DIFFICULT   = 4,
    OWNER_CHECK_MAX_LIMIT   = 5,
    OWNER_CHECK_MIN_LEVEL   = 6,
    OWNER_CHECK_WRONG_GROUP = 7,
    OWNER_CHECK_OK          = 10,
    OWNER_CHECK_GROUP_OK    = 11,
    OWNER_CHECK_SAVED_OK    = 12,
    OWNER_CHECK_RESET_LOC   = 13
};

extern const char * InstanceAbortMessages[];

class Map;
class MapInstance;

class WorldObject;
class Group;
class Player;
class MapUpdaterThread;
class Battleground;

class ContinentManager;

class SERVER_DECL WorldManager :  public Singleton < WorldManager >
{
    friend class MapInstance;
public:
    WorldManager();
    ~WorldManager();

    void Destruct();

    RONIN_INLINE Map* GetMap(uint32 mapid)
    {
        std::map<uint32, Map*>::iterator itr;
        if (mapid <= NUM_MAPS && ((itr = m_maps.find(mapid)) != m_maps.end()))
            return itr->second;
        return NULL;
    }

    void ParseMapDBCFiles();
    void LoadSpawnData();
    void LoadMapTileData(TaskList & tl);
    void _CreateMap(MapEntry *map);
    void _ProcessMapData(Map *map);
    void FillMapSafeLocations();

    void ContinentUnloaded(uint32 mapId);
    bool ContinentManagerExists(uint32 mapId) { return m_continentManagement.find(mapId) != m_continentManagement.end(); }
    ContinentManager *GetContinentManager(uint32 mapId)
    {
        if(ContinentManagerExists(mapId))
            return m_continentManagement.at(mapId);
        return NULL;
    }

    uint8 ValidateMapId(uint32 mapId);
    uint32 PreTeleport(uint32 mapid, Player* plr, uint32 &instanceid, bool groupFinderDungeon);

    bool PushToWorldQueue(WorldObject *obj);
    MapInstance *GetInstance(WorldObject* obj);

    void BuildXMLStats(char * m_file);
    void Load(TaskList * l);

    // player left a group, boot him out of any instances he's not supposed to be in.
    void PlayerLeftGroup(Group * pGroup, Player* pPlayer);

    // Has instance expired? Can player join?
    RONIN_INLINE uint8 PlayerOwnsInstance(MapInstance * pInstance, Player* pPlayer)
    {
        // expired?
        if( HasInstanceExpired( pInstance) )
        {
            _DeleteInstance(pInstance, false, false);
            return OWNER_CHECK_EXPIRED;
        }

        /* //Reached player limit?
        if( pInstance->GetMapInfo()->playerlimit < uint32(pInstance->GetPlayerCount()))
            return OWNER_CHECK_MAX_LIMIT;

        if(!pInstance->m_isBattleground)
        {
            // Matching the requested mode?
            if( pInstance->iInstanceMode != (pInstance->GetdbcMap()->IsRaid() ? pPlayer->iRaidType : pPlayer->iInstanceType) )
                return OWNER_CHECK_DIFFICULT;
        }

        //Meet level requirements?
        if(pPlayer->getLevel() < pInstance->GetMapInfo()->minlevel && !pPlayer->triggerpass_cheat )
            return OWNER_CHECK_MIN_LEVEL;

        //Need to be in group?
        if(!pPlayer->GetGroup() && pInstance->GetdbcMap()->IsRaid() && !pPlayer->triggerpass_cheat)
            return OWNER_CHECK_NO_GROUP;

        // Are we on the saved list?
        pInstance->m_SavedLock.Acquire();
        if( pInstance->m_SavedPlayers.find(pPlayer->GetLowGUID()) != pInstance->m_SavedPlayers.end() )
        {
            pInstance->m_SavedLock.Release();
            if(pInstance->m_EnteredPlayers.find(pPlayer->GetLowGUID()) == pInstance->m_EnteredPlayers.end())
            {
                pInstance->m_EnteredPlayers.insert(pPlayer->GetLowGUID());
                return OWNER_CHECK_RESET_LOC;
            }
            return OWNER_CHECK_SAVED_OK;
        }
        pInstance->m_SavedLock.Release();

        if(!pInstance->m_isBattleground)
        {
            // Active raid?
            if( pInstance->HasPlayers() )
            {
                //we have ensured the groupid is valid when it was created.
                if( pPlayer->GetGroup() )
                {
                    if(!pInstance->m_creatorGuid.empty() && pPlayer->GetGroup()->HasMember(objmgr.GetPlayerInfo(pInstance->m_creatorGuid)))
                    {
                        if(pInstance->m_EnteredPlayers.find(pPlayer->GetLowGUID()) == pInstance->m_EnteredPlayers.end())
                        {
                            pInstance->m_EnteredPlayers.insert(pPlayer->GetLowGUID());
                            return OWNER_CHECK_RESET_LOC;
                        }
                        return OWNER_CHECK_OK;
                    }
                    else if(pPlayer->GetGroupID() != pInstance->m_creatorGroup)
                        return OWNER_CHECK_WRONG_GROUP;
                }
            }

            // if we are not the creator, check if we are in same creator group.
            // First player in should have set the correct instance_id.
            if( !pPlayer->triggerpass_cheat )
            {
                if( pInstance->m_creatorGuid != pPlayer->GetGUID() )
                {
                    if(pInstance->m_creatorGroup)
                    {
                         if( pPlayer->GetGroupID() != pInstance->m_creatorGroup)
                            return OWNER_CHECK_WRONG_GROUP;
                    } else return OWNER_CHECK_WRONG_GROUP;
                }
            }
        }

        //nothing left to check, should be OK then
        if(pInstance->m_EnteredPlayers.find(pPlayer->GetLowGUID()) == pInstance->m_EnteredPlayers.end())
        {
            pInstance->m_EnteredPlayers.insert(pPlayer->GetLowGUID());
            return OWNER_CHECK_RESET_LOC;
        }*/
        return OWNER_CHECK_OK;
    }

    // has an instance expired?
    RONIN_INLINE bool HasInstanceExpired(MapInstance * pInstance)
    {
        MapEntry* map = dbcMap.LookupEntry(pInstance->GetMapId());
        if(map && map->IsRaid())
            return false;

        // expired? (heroic instances never expire, they are reset every day at 05:00).
        /*if( pInstance->iInstanceMode == 0 && pInstance->m_expiration && (UNIXTIME+20) >= pInstance->m_expiration)
            return true;*/

        return false;
    }

    void SendHeroicResetWarning();
    void ResetHeroicInstances();

    // check for expired instances
    void CheckForExpiredInstances();

    // delete all instances
    void Shutdown();

    // packets, w000t! we all love packets!
    void BuildSavedRaidInstancesForPlayer(Player* plr);
    void BuildSavedInstancesForPlayer(Player* plr);
    MapInstance *CreateBattlegroundInstance(uint32 mapid);

    // this only frees the instance pointer, not the mapmgr itself
    void DeleteBattlegroundInstance(uint32 mapid, uint32 instanceid);

    bool _DeleteInstance(MapInstance* in, bool ForcePlayersOut, bool atSelfEnd);

    bool HasLoadingMaps() { mapLoadLock.Acquire(); bool ret = !m_loadingMaps.empty(); mapLoadLock.Release(); return ret; }
    void MapLoaded(uint32 mapId)
    {
        mapLoadLock.Acquire();
        if(m_loadingMaps.find(mapId) == m_loadingMaps.end() || (--m_loadingMaps[mapId]) >= 1)
        {
            mapLoadLock.Release();
            return;
        }
        m_loadingMaps.erase(mapId);
        mapLoadLock.Release();
    }

private:
    void _LoadTileData(MapEntry *mapEntry);
    void _InitializeContinent(MapEntry *mapEntry, Map *map);
    void _InitializeBattleGround(MapEntry *mapEntry, Map *map);
    void _InitializeInstance(MapEntry *mapEntry, Map *map);
    void _InitializeRaid(MapEntry *mapEntry, Map *map);

    void ProcessPreSpawnLoadTables();

    Mutex mapLoadLock;
    std::map<uint32, MapEntry*> m_loadedMaps;

    Mutex m_mapLock;
    std::map<uint32, Map*> m_maps;
    std::map<uint32, ContinentManager*> m_continentManagement;

    CellSpawns *GetSpawn(uint32 mapId) { if(m_SpawnStorageMap.find(mapId) == m_SpawnStorageMap.end()) return NULL; return &m_SpawnStorageMap[mapId]; };
    std::map<uint32, CellSpawns> m_SpawnStorageMap;
    CellSpawns m_spawnStorage_Kalimdor;

    std::map<uint32, uint32> m_loadingMaps;

    std::map<uint32, std::set<uint32>> m_mapAreaIds, m_mapZoneIds;
};

#define sWorldMgr WorldManager::getSingleton()
