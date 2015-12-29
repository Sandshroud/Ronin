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

class WorldObject;
class Group;
class Player;
class MapUpdaterThread;
class Battleground;

class SERVER_DECL FormationMgr : public Singleton < FormationMgr >
{
    std::map<uint32, Formation*> m_formations;
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

class InstanceMgr : public MapMgr
{
public:
    InstanceMgr(Map *map, uint32 mapid, uint32 instanceid) : MapMgr(map, mapid, instanceid) {}

    void LoadFromDB(Field * fields);
    void SaveToDB();
    void DeleteFromDB();

    RONIN_INLINE virtual bool IsInstance() { return true; }

    WoWGuid m_creatorGuid;
    uint32 m_creatorGroup;
    time_t m_creation;
    time_t m_expiration;
    bool m_isBattleground;

    Mutex m_SavedLock;
    std::unordered_set<uint32> m_killedNpcs;
    std::unordered_set<uint32> m_SavedPlayers;
    std::unordered_set<uint32> m_EnteredPlayers;
};

typedef RONIN_UNORDERED_MAP<uint32, InstanceMgr*> InstanceMap;

class SERVER_DECL WorldManager
{
    friend class MapMgr;
public:
    WorldManager();
    ~WorldManager();

    RONIN_INLINE Map* GetMap(uint32 mapid)
    {
        if(mapid > NUM_MAPS)
            return NULL;
        return m_maps[mapid];
    }

    uint32 PreTeleport(uint32 mapid, Player* plr, uint32 instanceid);

    bool PushToWorldQueue(WorldObject *obj);
    MapMgr* GetInstance(WorldObject* obj);
    MapMgr* GetInstance(uint32 MapId, uint32 InstanceId);

    uint32 GenerateInstanceID();
    void BuildXMLStats(char * m_file);
    void Load(TaskList * l);
    void Load(uint32 mapid);

    // deletes all instances owned by this player.
    void ResetSavedInstances(Player* plr);

    // player left a group, boot him out of any instances he's not supposed to be in.
    void PlayerLeftGroup(Group * pGroup, Player* pPlayer);

    // Has instance expired? Can player join?
    RONIN_INLINE uint8 PlayerOwnsInstance(InstanceMgr * pInstance, Player* pPlayer)
    {
        // expired?
        if( HasInstanceExpired( pInstance) )
        {
            _DeleteInstance(pInstance, false, false);
            return OWNER_CHECK_EXPIRED;
        }

        //Reached player limit?
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
        }
        return OWNER_CHECK_OK;
    }

    // has an instance expired?
    RONIN_INLINE bool HasInstanceExpired(InstanceMgr * pInstance)
    {
        MapEntry* map = dbcMap.LookupEntry(pInstance->GetMapId());
        if(map && map->IsRaid())
            return false;

        // expired? (heroic instances never expire, they are reset every day at 05:00).
        if( pInstance->iInstanceMode == 0 && pInstance->m_expiration && (UNIXTIME+20) >= pInstance->m_expiration)
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
    InstanceMgr* GetSavedInstance(uint32 map_id, uint32 guid, uint32 difficulty);
    InstanceMap * GetInstancesForMap(uint32 map_id) { return m_instances[map_id]; }
    InstanceMgr* GetInstanceByIds(uint32 mapid, uint32 instanceId)
    {
        if(mapid > NUM_MAPS)
            return NULL;
        if(mapid == NUM_MAPS)
        {
            InstanceMgr *in;
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
    bool _DeleteInstance(InstanceMgr* in, bool ForcePlayersOut, bool atSelfEnd);
    MapMgr *_CreateMapMgr(uint32 mapId, uint32 instanceId, bool instance = false);

private:
    uint32 m_InstanceHigh;

    Mutex m_mapLock;
    Map* m_maps[NUM_MAPS];

    InstanceMap* m_instances[NUM_MAPS];
    MapMgr* m_singleMaps[NUM_MAPS];
};

extern SERVER_DECL WorldManager sWorldMgr;
