/***
 * Demonstrike Core
 */

//
// mapMgr->h
//

#pragma once

class MapCell;
class Map;
class Object;
class WorldSession;
class GameObject;
class Creature;
class Player;
class Pet;
class Vehicle;
class Transporter;
class Corpse;
class CBattleground;
class Instance;
class Transporter;

enum MapMgrTimers
{
    MMUPDATE_OBJECTS = 0,
    MMUPDATE_SESSIONS = 1,
    MMUPDATE_FIELDS = 2,
    MMUPDATE_IDLE_OBJECTS = 3,
    MMUPDATE_ACTIVE_OBJECTS = 4,
    MMUPDATE_COUNT = 5
};

enum ObjectActiveState
{
    OBJECT_STATE_NONE    = 0,
    OBJECT_STATE_INACTIVE = 1,
    OBJECT_STATE_ACTIVE   = 2,
};

typedef unordered_set<Object* > ObjectSet;
typedef unordered_set<Object* > UpdateQueue;
typedef unordered_set<Player*  > PUpdateQueue;
typedef unordered_set<Player*  > PlayerSet;
typedef HM_NAMESPACE::hash_map<uint32, Object* > StorageMap;
typedef unordered_set<uint64> CombatProgressMap;
typedef unordered_set<Vehicle*> VehicleSet;
typedef unordered_set<Creature*> CreatureSet;
typedef unordered_set<GameObject* > GameObjectSet;
typedef HM_NAMESPACE::hash_map<uint32, Vehicle*> VehicleSqlIdMap;
typedef HM_NAMESPACE::hash_map<uint32, Creature*> CreatureSqlIdMap;
typedef HM_NAMESPACE::hash_map<uint32, GameObject* > GameObjectSqlIdMap;

#define MAX_VIEW_DISTANCE 38000
#define MAX_TRANSPORTERS_PER_MAP 25
#define RESERVE_EXPAND_SIZE 1024

#define FunctionCall(Mgr, Func) Mgr->GetMapScript()->Func
#define ManagerCheck(Mgr) Mgr != NULL && Mgr->GetMapScript() != NULL
#define CALL_INSTANCE_SCRIPT_EVENT( Mgr, Func ) if ( ManagerCheck(Mgr) ) FunctionCall(Mgr, Func)

class SERVER_DECL MapMgr : public CellHandler <MapCell>, public EventableObject, public ThreadContext
{
    friend class UpdateObjectThread;
    friend class ObjectUpdaterThread;
    friend class MapCell;
public:

    //This will be done in regular way soon
    Mutex m_objectinsertlock;
    ObjectSet m_objectinsertpool;
    void AddObject(Object*);
    Object* GetObjectClosestToCoords(uint32 entry, float x, float y, float z, float ClosestDist, int32 forcedtype = -1);

////////////////////////////////////////////////////////
// Local (mapmgr) storage/generation of GameObjects
/////////////////////////////////////////////
    typedef HM_NAMESPACE::hash_map<uint32, GameObject* > GameObjectMap;
    GameObjectMap m_gameObjectStorage;
    uint32 m_GOHighGuid;
    GameObject* CreateGameObject(uint32 entry);

    HEARTHSTONE_INLINE uint32 GenerateGameobjectGuid()
    {
        m_GOHighGuid &= 0x00FFFFFF;
        return ++m_GOHighGuid;
    }

    HEARTHSTONE_INLINE GameObject* GetGameObject(uint32 guid)
    {
        GameObjectMap::iterator itr = m_gameObjectStorage.find(guid);
        return (itr != m_gameObjectStorage.end()) ? m_gameObjectStorage[guid] : NULLGOB;
    }

/////////////////////////////////////////////////////////
// Local (mapmgr) storage/generation of Vehicles
/////////////////////////////////////////////
    uint32 m_VehicleArraySize;
    uint32 m_VehicleHighGuid;
    HM_NAMESPACE::hash_map<uint32,Vehicle*> m_VehicleStorage;
    Vehicle* CreateVehicle(uint32 entry);

    __inline Vehicle* GetVehicle(uint32 guid)
    {
        return guid <= m_VehicleHighGuid ? m_VehicleStorage[guid] : NULLVEHICLE;
    }
/////////////////////////////////////////////////////////
// Local (mapmgr) storage/generation of Creatures
/////////////////////////////////////////////
    uint32 m_CreatureArraySize;
    uint32 m_CreatureHighGuid;
    HM_NAMESPACE::hash_map<uint32,Creature*> m_CreatureStorage;
    Creature* CreateCreature(uint32 entry);

    HEARTHSTONE_INLINE Creature* GetCreature(uint32 guid)
    {
        return guid <= m_CreatureHighGuid ? m_CreatureStorage[guid] : NULLCREATURE;
    }

    // Use a creature guid to create our summon.
    Summon* CreateSummon(uint32 entry);
//////////////////////////////////////////////////////////
// Local (mapmgr) storage/generation of DynamicObjects
////////////////////////////////////////////
    uint32 m_DynamicObjectHighGuid;
    typedef HM_NAMESPACE::hash_map<uint32, DynamicObject*> DynamicObjectStorageMap;
    DynamicObjectStorageMap m_DynamicObjectStorage;
    DynamicObject* CreateDynamicObject();

    HEARTHSTONE_INLINE DynamicObject* GetDynamicObject(uint32 guid)
    {
        DynamicObjectStorageMap::iterator itr = m_DynamicObjectStorage.find(guid);
        return (itr != m_DynamicObjectStorage.end()) ? m_DynamicObjectStorage[guid] : NULLDYN;
    }

//////////////////////////////////////////////////////////
// Local (mapmgr) storage of pets
///////////////////////////////////////////
    typedef HM_NAMESPACE::hash_map<uint32, Pet*> PetStorageMap;
    PetStorageMap m_PetStorage;
    __inline Pet* GetPet(uint32 guid)
    {
        PetStorageMap::iterator itr = m_PetStorage.find(guid);
        return (itr != m_PetStorage.end()) ? m_PetStorage[guid] : NULLPET;
    }

//////////////////////////////////////////////////////////
// Local (mapmgr) storage of players for faster lookup
////////////////////////////////
    typedef HM_NAMESPACE::hash_map<uint32, Player*> PlayerStorageMap;
    PlayerStorageMap m_PlayerStorage;
    __inline Player* GetPlayer(uint32 guid)
    {
        PlayerStorageMap::iterator itr = m_PlayerStorage.find(guid);
        return (itr != m_PlayerStorage.end()) ? m_PlayerStorage[guid] : NULLPLR;
    }

//////////////////////////////////////////////////////////
// Local (mapmgr) storage of combats in progress
////////////////////////////////
    CombatProgressMap _combatProgress;
    void AddCombatInProgress(uint64 guid)
    {
        _combatProgress.insert(guid);
    }
    void RemoveCombatInProgress(uint64 guid)
    {
        _combatProgress.erase(guid);
    }
    HEARTHSTONE_INLINE bool IsCombatInProgress()
    {
        //if all players are out, list should be empty.
        if(!HasPlayers())
            _combatProgress.clear();
        return (_combatProgress.size() > 0);
    }

//////////////////////////////////////////////////////////
// Lookup Wrappers
///////////////////////////////////
    Unit* GetUnit(const uint64 & guid);
    Object* _GetObject(const uint64 & guid);

    bool run();
    bool Do();

    MapMgr(Map *map, uint32 mapid, uint32 instanceid);
    ~MapMgr();
    void Init(bool Instance);
    void Destruct();

    void EventPushObjectToSelf(Object *obj);
    void PushObject(Object* obj);
    void PushStaticObject(Object* obj);
    void RemoveObject(Object* obj, bool free_guid);
    void ChangeObjectLocation(Object* obj); // update inrange lists
    void ChangeFarsightLocation(Player* plr, Unit* farsight, bool apply);
    void ChangeFarsightLocation(Player* plr, float X, float Y, bool apply);
    bool IsInRange(float fRange, Object* obj, Object* currentobj);

    //! Mark object as updated
    void ObjectUpdated(Object* obj);
    void UpdateCellActivity(uint32 x, uint32 y, int radius);

    // Terrain Functions
    void GetWaterData(float x, float y, float z, float &outHeight, uint16 &outType);
    float GetLandHeight(float x, float y);
    uint8 GetWalkableState(float x, float y);
    uint16 GetAreaID(float x, float y, float z = 0.0f);
    float GetWaterHeight(float x, float y, float z)
    {
        uint16 waterType = 0;
        float res = NO_WATER_HEIGHT;
        GetWaterData(x, y, z, res, waterType);
        return res;
    }

    HEARTHSTONE_INLINE uint32 GetMapId() { return _mapId; }
    void AddForcedCell(MapCell * c, uint32 range = 1);
    void RemoveForcedCell(MapCell * c, uint32 range = 1);

    void PushToProcessed(Player* plr);

    HEARTHSTONE_INLINE bool HasPlayers() { return (m_PlayerStorage.size() > 0); }
    void TeleportPlayers();

    HEARTHSTONE_INLINE uint32 GetInstanceID() { return m_instanceID; }
    HEARTHSTONE_INLINE MapInfo *GetMapInfo() { return pMapInfo; }
    HEARTHSTONE_INLINE MapEntry *GetdbcMap() { return pdbcMap; }
    bool CanUseCollision(Object* obj);

    HEARTHSTONE_INLINE MapManagerScript *GetMapScript() { return _script; }

    virtual int32 event_GetInstanceID() { return m_instanceID; }

    void UpdateAllCells(bool apply, uint32 areamask = 0);
    HEARTHSTONE_INLINE size_t GetPlayerCount() { return m_PlayerStorage.size(); }

    void _PerformObjectDuties();
    uint32 mLoopCounter;
    uint32 lastGameobjectUpdate;
    uint32 lastDynamicUpdate;
    uint32 lastUnitUpdate;
    void EventCorpseDespawn(uint64 guid);

    time_t InactiveMoveTime;
    uint32 iInstanceMode;

    HEARTHSTONE_INLINE void AddSpawn(uint32 x, uint32 y, CreatureSpawn * sp)
    {
        GetBaseMap()->GetSpawnsListAndCreate(x, y)->CreatureSpawns.push_back(sp);
        GetBaseMap()->CreatureSpawnCount++;
        m_CreatureArraySize++;
    }

    HEARTHSTONE_INLINE void AddGoSpawn(uint32 x, uint32 y, GOSpawn * gs)
    {
        GetBaseMap()->GetSpawnsListAndCreate(x, y)->GOSpawns.push_back(gs);
    }

    void UnloadCell(uint32 x,uint32 y);
    void EventRespawnVehicle(Vehicle* v, MapCell * p);
    void EventRespawnCreature(Creature* c, MapCell * p);
    void EventRespawnGameObject(GameObject* o, MapCell * c);
    void SendMessageToCellPlayers(Object* obj, WorldPacket * packet, uint32 cell_radius = 2);
    void SendChatMessageToCellPlayers(Object* obj, WorldPacket * packet, uint32 cell_radius, uint32 langpos, int32 lang, WorldSession * originator);

    Instance * pInstance;
    void BeginInstanceExpireCountdown();
    void HookOnAreaTrigger(Player* plr, uint32 id);

    // better hope to clear any references to us when calling this :P
    void InstanceShutdown()
    {
        pInstance = NULL;
        OnShutdown();
    }

    // kill the worker thread only
    void KillThread()
    {
        pInstance = NULL;
        thread_kill_only = true;
        OnShutdown();
        while(thread_running)
            Sleep(100);
    }

protected:
    //! Collect and send updates to clients
    void _UpdateObjects();

private:
    //! Objects that exist on map

    uint32 _mapId;
    set<Object* > _mapWideStaticObjects;

    bool _CellActive(uint32 x, uint32 y);
    void UpdateInRangeSet(Object* obj, Player* plObj, MapCell* cell);
    void UpdateInRangeSet(uint64 guid, MapCell* cell);

public:
    void UpdateInrangeSetOnCells(uint64 guid, uint32 startX, uint32 endX, uint32 startY, uint32 endY);

    // Distance a Player can "see" other objects and receive updates from them (!! ALREADY dist*dist !!)
    float m_UpdateDistance;

private:
    /* Map Information */
    MapInfo *pMapInfo;
    MapEntry* pdbcMap;
    uint32 m_instanceID;

    MapManagerScript *_script;

    /* Update System */
    Mutex m_updateMutex;
    UpdateQueue _updates;
    PUpdateQueue _processQueue;

    /* Sessions */
    SessionSet MapSessions;

public:
#ifdef WIN32
    DWORD threadid;
#endif

    Mutex ActiveLock;
    GameObjectSet activeGameObjects;
    CreatureSet activeCreatures;
    VehicleSet activeVehicles;

    EventableObjectHolder eventHolder;
    CBattleground* m_battleground;
    unordered_set<Corpse* > m_corpses;
    VehicleSqlIdMap _sqlids_vehicles;
    CreatureSqlIdMap _sqlids_creatures;
    GameObjectSqlIdMap _sqlids_gameobjects;

    Vehicle* GetSqlIdVehicle(uint32 sqlid);
    Creature* GetSqlIdCreature(uint32 sqlid);
    GameObject* GetSqlIdGameObject(uint32 sqlid);
    deque<uint32> _reusable_guids_creature;
    deque<uint32> _reusable_guids_vehicle;

    bool forced_expire;
    bool thread_kill_only;
    bool thread_running;

    // world state manager stuff
    WorldStateManager* m_stateManager;

    // bytebuffer caching
    ByteBuffer m_updateBuffer;
    ByteBuffer m_createBuffer;
    ByteBuffer m_updateBuildBuffer;
    ByteBuffer m_compressionBuffer;

public:
    void ClearCorpse(Corpse* remove) { unordered_set<Corpse* >::iterator itr; if((itr = m_corpses.find(remove)) != m_corpses.end()) m_corpses.erase(itr); };

    // get!
    HEARTHSTONE_INLINE WorldStateManager& GetStateManager() { return *m_stateManager; }

    // send packet functions for state manager
    void SendPacketToPlayers(int32 iZoneMask, int32 iFactionMask, WorldPacket *pData);
    void SendPvPCaptureMessage(int32 iZoneMask, uint32 ZoneId, const char * Format, ...);

    // auras :< (world pvp)
    void RemoveAuraFromPlayers(int32 iFactionMask, uint32 uAuraId);
    void RemovePositiveAuraFromPlayers(int32 iFactionMask, uint32 uAuraId);
    void CastSpellOnPlayers(int32 iFactionMask, uint32 uSpellId);

public:

    // stored iterators for safe checking
    PetStorageMap::iterator __pet_iterator;
    PlayerStorageMap::iterator __player_iterator;

    VehicleSet::iterator __vehicle_iterator;
    CreatureSet::iterator __creature_iterator;
    GameObjectSet::iterator __gameobject_iterator;

    SessionSet::iterator __session_iterator_1;
    SessionSet::iterator __session_iterator_2;
};
