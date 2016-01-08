/***
 * Demonstrike Core
 */

//
// MapInstance->h
//

#pragma once

class MapCell;
class Map;
class WorldObject;
class WorldSession;
class GameObject;
class Creature;
class Player;
class Pet;
class Transporter;
class Corpse;
class CBattleground;
class Transporter;

enum MapInstanceTimers
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

#define MAX_VIEW_DISTANCE 38000
#define MAX_TRANSPORTERS_PER_MAP 25
#define RESERVE_EXPAND_SIZE 1024

#define TRIGGER_INSTANCE_EVENT( Mgr, Func )

class SERVER_DECL MapInstance : public CellHandler <MapCell>, public EventableObject
{
    friend class UpdateObjectThread;
    friend class ObjectUpdaterThread;
    friend class MapCell;
public:

    typedef RONIN_UNORDERED_SET<WorldObject*> ObjectSet;
    typedef RONIN_UNORDERED_SET<Player*> PlayerSet;
    typedef RONIN_UNORDERED_SET<Creature*> CreatureSet;
    typedef RONIN_UNORDERED_SET<GameObject*> GameObjectSet;
    typedef RONIN_UNORDERED_SET<uint64> CombatProgressMap;
    typedef RONIN_UNORDERED_MAP<uint32, WorldObject* > StorageMap;
    typedef RONIN_UNORDERED_MAP<uint32, Creature*> CreatureSqlIdMap;
    typedef RONIN_UNORDERED_MAP<uint32, GameObject* > GameObjectSqlIdMap;

    //This will be done in regular way soon
    Mutex m_objectinsertlock;
    ObjectSet m_objectinsertpool;
    void AddObject(WorldObject*);
    WorldObject* GetObjectClosestToCoords(uint32 entry, float x, float y, float z, float ClosestDist, int32 forcedtype = -1);

////////////////////////////////////////////////////////
// Local (MapInstance) storage/generation of GameObjects
/////////////////////////////////////////////
    typedef RONIN_UNORDERED_MAP<WoWGuid, GameObject* > GameObjectMap;
    GameObjectMap m_gameObjectStorage;
    uint32 m_GOHighGuid;
    GameObject* CreateGameObject(uint32 entry);

    RONIN_INLINE uint32 GenerateGameobjectGuid()
    {
        m_GOHighGuid &= 0x00FFFFFF;
        return ++m_GOHighGuid;
    }

    RONIN_INLINE GameObject* GetGameObject(WoWGuid guid)
    {
        ASSERT(guid.getHigh() == HIGHGUID_TYPE_GAMEOBJECT);
        GameObjectMap::iterator itr = m_gameObjectStorage.find(guid.getLow());
        return (itr != m_gameObjectStorage.end()) ? itr->second : NULL;
    }

/////////////////////////////////////////////////////////
// Local (MapInstance) storage/generation of Creatures
/////////////////////////////////////////////
    uint32 m_CreatureHighGuid;
    RONIN_UNORDERED_MAP<WoWGuid, Creature*> m_CreatureStorage;
    Creature* CreateCreature(uint32 entry);

    RONIN_INLINE Creature* GetCreature(WoWGuid guid)
    {
        ASSERT(guid.getHigh() == HIGHGUID_TYPE_UNIT || guid.getHigh() == HIGHGUID_TYPE_VEHICLE);
        RONIN_UNORDERED_MAP<WoWGuid, Creature*>::iterator itr = m_CreatureStorage.find(guid);
        return ((itr != m_CreatureStorage.end()) ? itr->second : NULL);
    }

    // Use a creature guid to create our summon.
    Summon* CreateSummon(uint32 entry);
//////////////////////////////////////////////////////////
// Local (MapInstance) storage/generation of DynamicObjects
////////////////////////////////////////////
    uint32 m_DynamicObjectHighGuid;
    typedef RONIN_UNORDERED_MAP<WoWGuid, DynamicObject*> DynamicObjectStorageMap;
    DynamicObjectStorageMap m_DynamicObjectStorage;
    DynamicObject* CreateDynamicObject();

    RONIN_INLINE DynamicObject* GetDynamicObject(WoWGuid guid)
    {
        DynamicObjectStorageMap::iterator itr = m_DynamicObjectStorage.find(guid);
        return ((itr != m_DynamicObjectStorage.end()) ? itr->second : NULL);
    }

//////////////////////////////////////////////////////////
// Local (MapInstance) storage of pets
///////////////////////////////////////////
    typedef RONIN_UNORDERED_MAP<WoWGuid, Pet*> PetStorageMap;
    PetStorageMap m_PetStorage;
    RONIN_INLINE Pet* GetPet(WoWGuid guid)
    {
        PetStorageMap::iterator itr = m_PetStorage.find(guid);
        return (itr != m_PetStorage.end()) ? m_PetStorage[guid] : NULL;
    }

//////////////////////////////////////////////////////////
// Local (MapInstance) storage of players for faster lookup
////////////////////////////////
    typedef RONIN_UNORDERED_MAP<WoWGuid, Player*> PlayerStorageMap;
    PlayerStorageMap m_PlayerStorage;
    RONIN_INLINE Player* GetPlayer(WoWGuid guid)
    {
        ASSERT(guid.getHigh() == HIGHGUID_TYPE_PLAYER);
        PlayerStorageMap::iterator itr = m_PlayerStorage.find(guid);
        return (itr != m_PlayerStorage.end()) ? m_PlayerStorage[guid] : NULL;
    }

//////////////////////////////////////////////////////////
// Local (MapInstance) storage of combats in progress
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
    RONIN_INLINE bool IsCombatInProgress()
    {
        //if all players are out, list should be empty.
        if(!HasPlayers())
            _combatProgress.clear();
        return (_combatProgress.size() > 0);
    }

//////////////////////////////////////////////////////////
// Lookup Wrappers
///////////////////////////////////
    Unit* GetUnit(WoWGuid guid);
    WorldObject* _GetObject(WoWGuid guid);

    MapInstance(Map *map, uint32 mapid, uint32 instanceid);
    ~MapInstance();

    void Destruct();

    void EventPushObjectToSelf(WorldObject *obj);
    void PushObject(WorldObject* obj);
    void RemoveObject(WorldObject* obj, bool free_guid);
    void ChangeObjectLocation(WorldObject* obj); // update inrange lists
    void ChangeFarsightLocation(Player* plr, Unit* farsight, bool apply);
    void ChangeFarsightLocation(Player* plr, float X, float Y, bool apply);
    bool IsInRange(float fRange, WorldObject* obj, WorldObject* currentobj);

    //! Mark object as updated
    void ObjectUpdated(WorldObject* obj);
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

    RONIN_INLINE uint32 GetMapId() { return _mapId; }
    void AddForcedCell(MapCell * c, uint32 range = 1);
    void RemoveForcedCell(MapCell * c, uint32 range = 1);

    void PushToProcessed(Player* plr);

    RONIN_INLINE bool HasPlayers() { return (m_PlayerStorage.size() > 0); }
    void TeleportPlayers();

    RONIN_INLINE virtual bool IsInstance() { return false; }
    RONIN_INLINE uint32 GetInstanceID() { return m_instanceID; }
    RONIN_INLINE MapEntry *GetdbcMap() { return pdbcMap; }
    bool CanUseCollision(WorldObject* obj);

    virtual int32 event_GetMapID() { return _mapId; }

    void UpdateAllCells(bool apply, uint32 areamask = 0);
    RONIN_INLINE size_t GetPlayerCount() { return m_PlayerStorage.size(); }

    void _ProcessInputQueue();
    void _PerformPlayerUpdates(uint32 diff);
    void _PerformCreatureUpdates(uint32 msTime);
    void _PerformObjectUpdates(uint32 msTime);
    void _PerformSessionUpdates();
    void _PerformPendingUpdates();

    uint32 mLoopCounter;
    uint32 lastGameobjectUpdate;
    uint32 lastUnitUpdate;
    void EventCorpseDespawn(uint64 guid);

    time_t InactiveMoveTime;
    uint32 iInstanceMode;

    RONIN_INLINE void AddSpawn(uint32 x, uint32 y, CreatureSpawn * sp)
    {
        GetBaseMap()->GetSpawnsListAndCreate(x, y)->CreatureSpawns.push_back(sp);
    }

    RONIN_INLINE void AddGoSpawn(uint32 x, uint32 y, GOSpawn * gs)
    {
        GetBaseMap()->GetSpawnsListAndCreate(x, y)->GOSpawns.push_back(gs);
    }

    void UnloadCell(uint32 x,uint32 y);
    void EventRespawnCreature(Creature* c, MapCell * p);
    void EventRespawnGameObject(GameObject* o, MapCell * c);
    void SendMessageToCellPlayers(WorldObject* obj, WorldPacket * packet, uint32 cell_radius = 2);
    void SendChatMessageToCellPlayers(WorldObject* obj, WorldPacket * packet, uint32 cell_radius, uint32 langpos, uint32 guidPos, int32 lang, WorldSession * originator);
    void BeginInstanceExpireCountdown();
    void HookOnAreaTrigger(Player* plr, uint32 id);

protected:
    //! Collect and send updates to clients
    void _UpdateObjects();

    //! Objects that exist on map
    uint32 _mapId;

    // In this zone, we always show these objects
    std::map<WorldObject*, uint32> m_rangelessObjects;
    std::map<uint32, std::set<WorldObject*>> m_zoneRangelessObjects;

    bool _CellActive(uint32 x, uint32 y);
    void UpdateInRangeSet(WorldObject* obj, Player* plObj, MapCell* cell);

    void ObjectMovingCells(WorldObject *obj, MapCell *oldCell, MapCell *newCell);
    void UpdateObjectVisibility(Player *plObj, WorldObject *curObj);

public:
    void UpdateInrangeSetOnCells(WorldObject* obj, uint32 startX, uint32 endX, uint32 startY, uint32 endY);

    // Distance a Player can "see" other objects and receive updates from them (!! ALREADY dist*dist !!)
    float m_UpdateDistance;
    bool m_mapPreloading;

    bool IsRaid() { return pdbcMap ? pdbcMap->IsRaid() : false; }
    bool IsContinent() { return pdbcMap ? pdbcMap->IsContinent() : true; }
protected:
    /* Map Information */
    MapEntry* pdbcMap;
    uint32 m_instanceID;

    /* Update System */
    Mutex m_updateMutex;
    ObjectSet _updates;
    PlayerSet _processQueue;

    /* Sessions */
    SessionSet MapSessions;

public:
    Mutex m_activeLock;
    GameObjectSet activeGameObjects;
    CreatureSet activeCreatures;

    CBattleground* m_battleground;
    std::unordered_set<Corpse* > m_corpses;
    CreatureSqlIdMap _sqlids_creatures;
    GameObjectSqlIdMap _sqlids_gameobjects;

    Creature* GetSqlIdCreature(uint32 sqlid);
    GameObject* GetSqlIdGameObject(uint32 sqlid);
    std::deque<uint32> _reusable_guids_creature;

    // world state manager stuff
    WorldStateManager* m_stateManager;

    // bytebuffer caching
    ByteBuffer m_createBuffer, m_updateBuffer;

public:
    void ClearCorpse(Corpse* remove) { std::unordered_set<Corpse* >::iterator itr; if((itr = m_corpses.find(remove)) != m_corpses.end()) m_corpses.erase(itr); };

    // get!
    RONIN_INLINE WorldStateManager& GetStateManager() { return *m_stateManager; }

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

    CreatureSet::iterator __creature_iterator;
    GameObjectSet::iterator __gameobject_iterator;

    SessionSet::iterator __session_iterator_1;
    SessionSet::iterator __session_iterator_2;
};
