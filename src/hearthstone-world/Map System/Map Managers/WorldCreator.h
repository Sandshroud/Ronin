/***
 * Demonstrike Core
 */

//
// WorldCreator.h
//

#pragma once

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
    INSTANCE_ABORT_ERROR_ERROR                  = 1,
    INSTANCE_ABORT_FULL                         = 2,
    INSTANCE_ABORT_NOT_FOUND                    = 3,
    INSTANCE_ABORT_TOO_MANY                     = 4,
    INSTANCE_ABORT_ENCOUNTER                    = 5,
    INSTANCE_ABORT_NON_CLIENT_TYPE              = 6,
    INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE    = 7,
    INSTANCE_ABORT_NOT_IN_RAID_GROUP            = 8,
    INSTANCE_OK_RESET_POS                       = 9,
    INSTANCE_OK                                 = 10
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
class MapMgr;

class Object;
class Group;
class Player;
class MapUpdaterThread;
class Battleground;

class SERVER_DECL FormationMgr : public Singleton < FormationMgr >
{
    map<uint32, Formation*> m_formations;
public:
    typedef std::map<uint32, Formation*> FormationMap;
    FormationMgr();
    ~FormationMgr();

    Formation * GetFormation(uint32 sqlid)
    {
        FormationMap::iterator itr = m_formations.find(sqlid);
        return (itr == m_formations.end()) ? 0 : itr->second;
    }
};

#define sFormationMgr FormationMgr::getSingleton()

class Instance
{
public:
    uint32 m_instanceId;
    uint32 m_mapId;
    MapMgr* m_mapMgr;
    uint32 m_creatorGuid;
    uint32 m_creatorGroup;
    uint32 m_difficulty;
    unordered_set<uint32> m_killedNpcs;
    unordered_set<uint32> m_SavedPlayers;
    unordered_set<uint32> m_EnteredPlayers;
    Mutex m_SavedLock;
    time_t m_creation;
    time_t m_expiration;
    MapInfo * m_mapInfo;
    MapEntry * m_dbcMap;
    bool m_isBattleground;

    void LoadFromDB(Field * fields);
    void SaveToDB();
    void DeleteFromDB();
};

typedef HM_NAMESPACE::hash_map<uint32, Instance*> InstanceMap;

class SERVER_DECL InstanceMgr
{
    friend class MapMgr;
public:
    InstanceMgr();
    ~InstanceMgr();

    HEARTHSTONE_INLINE Map* GetMap(uint32 mapid)
    {
        if(mapid > NUM_MAPS)
            return NULL;
        else
            return m_maps[mapid];
    }

    uint32 PreTeleport(uint32 mapid, Player* plr, uint32 instanceid);
    MapMgr* GetInstance(Object* obj);
    MapMgr* GetInstance(uint32 MapId, uint32 InstanceId);
    MapMgr* ClusterCreateInstance(uint32 mapid, uint32 instanceid);

    uint32 GenerateInstanceID();
    void BuildXMLStats(char * m_file);
    void Load(TaskList * l);
    void Load(uint32 mapid);

    // deletes all instances owned by this player.
    void ResetSavedInstances(Player* plr);

    // player left a group, boot him out of any instances he's not supposed to be in.
    void PlayerLeftGroup(Group * pGroup, Player* pPlayer);

    // Has instance expired? Can player join?
    HEARTHSTONE_INLINE uint8 PlayerOwnsInstance(Instance * pInstance, Player* pPlayer)
    {
        // expired?
        if( HasInstanceExpired( pInstance) )
        {
            _DeleteInstance(pInstance, false);
            return OWNER_CHECK_EXPIRED;
        }

        // Valid map?
        if( !pInstance->m_mapInfo || !pInstance->m_dbcMap) // ITS A TARP!
            return OWNER_CHECK_NOT_EXIST;

        //Reached player limit?
        if( pInstance->m_mapMgr && pInstance->m_mapInfo->playerlimit < uint32(pInstance->m_mapMgr->GetPlayerCount()))
            return OWNER_CHECK_MAX_LIMIT;

        if(!pInstance->m_isBattleground)
        {
            // Matching the requested mode?
            if( pInstance->m_difficulty != (pInstance->m_dbcMap->IsRaid() ? pPlayer->iRaidType : pPlayer->iInstanceType) )
                return OWNER_CHECK_DIFFICULT;
        }

        //Meet level requirements?
        if( pInstance->m_mapInfo && pPlayer->getLevel() < pInstance->m_mapInfo->minlevel && !pPlayer->triggerpass_cheat )
            return OWNER_CHECK_MIN_LEVEL;

        //Need to be in group?
        if(!pPlayer->GetGroup() && pInstance->m_dbcMap->IsRaid() && !pPlayer->triggerpass_cheat)
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
            if( pInstance->m_mapMgr && pInstance->m_mapMgr->HasPlayers() )
            {
                //we have ensured the groupid is valid when it was created.
                if( pPlayer->GetGroup() )
                {
                    if(pInstance->m_creatorGuid && pPlayer->GetGroup()->HasMember(objmgr.GetPlayerInfo(pInstance->m_creatorGuid)))
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
                if( pInstance->m_creatorGuid != pPlayer->GetLowGUID() )
                {
                    if(pInstance->m_creatorGroup)
                    {
                         if( pPlayer->GetGroupID() != pInstance->m_creatorGroup)
                            return OWNER_CHECK_WRONG_GROUP;
                    }
                    else // There is no group, so group checks will be wrong, check by guid.
                    {   // Since there is no group, if creator guid is wrong, fuck it.
                        if(pPlayer->GetLowGUID() != pInstance->m_creatorGuid)
                            return OWNER_CHECK_WRONG_GROUP;
                    }
                }
            }
        }

        //nothing left to check, should be OK then
        if(pInstance->m_EnteredPlayers.find(pPlayer->GetLowGUID()) == pInstance->m_EnteredPlayers.end())
        {
            pInstance->m_EnteredPlayers.insert(pPlayer->GetLowGUID());
            return OWNER_CHECK_RESET_LOC;
        }
        return OWNER_CHECK_OK;
    }

    // has an instance expired?
    HEARTHSTONE_INLINE bool HasInstanceExpired(Instance * pInstance)
    {
        MapEntry* map = dbcMap.LookupEntry(pInstance->m_mapId);
        if(map && map->IsRaid())
            return false;

        // expired? (heroic instances never expire, they are reset every day at 05:00).
        if( pInstance->m_difficulty == 0 && pInstance->m_expiration && (UNIXTIME+20) >= pInstance->m_expiration)
            return true;

        return false;
    }

    void ResetHeroicInstances();

    // check for expired instances
    void CheckForExpiredInstances();

    // delete all instances
    void Shutdown();

    // packets, w000t! we all love packets!
    void BuildSavedRaidInstancesForPlayer(Player* plr);
    void BuildSavedInstancesForPlayer(Player* plr);
    MapMgr* CreateBattlegroundInstance(uint32 mapid);

    // this only frees the instance pointer, not the mapmgr itself
    void DeleteBattlegroundInstance(uint32 mapid, uint32 instanceid);
    MapMgr* GetMapMgr(uint32 mapId);

    //Find saved instance for player at given mapid
    Instance* GetSavedInstance(uint32 map_id, uint32 guid, uint32 difficulty);
    InstanceMap * GetInstancesForMap(uint32 map_id) { return m_instances[map_id]; }
    Instance* GetInstanceByIds(uint32 mapid, uint32 instanceId)
    {
        if(mapid > NUM_MAPS)
            return NULL;
        if(mapid == NUM_MAPS)
        {
            Instance *in;
            for(uint32 i=0; i<NUM_MAPS; ++i)
            {
                in = GetInstanceByIds(i, instanceId);
                if(in != NULL)
                    return in;
            }
            return NULL;
        }
        InstanceMap *map = m_instances[mapid];
        if(map == NULL)
            return NULL;
        InstanceMap::iterator instance = map->find(instanceId);
        return instance == map->end() ? NULL : instance->second;
    }

    void _LoadInstances();
    void _CreateMap(uint32 mapid);
    MapMgr* _CreateInstance(Instance* in);
    MapMgr* _CreateInstance(uint32 mapid, uint32 instanceid);       // only used on main maps!
    bool _DeleteInstance(Instance* in, bool ForcePlayersOut);

private:
    uint32 m_InstanceHigh;

    Mutex m_mapLock;
    Map* m_maps[NUM_MAPS];
    InstanceMap* m_instances[NUM_MAPS];
    MapMgr* m_singleMaps[NUM_MAPS];
};

extern SERVER_DECL InstanceMgr sInstanceMgr;
