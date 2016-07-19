
#pragma once

extern const uint32 MapInstanceUpdatePeriod;

class MapInstanceContainer;
class InstanceData;

// Each instance has it's own instance data linked to unique IDs
typedef std::map<uint32, InstanceData*> InstanceDataMap;

class SERVER_DECL InstanceManager
{
public:
    InstanceManager();
    ~InstanceManager();

    void Destruct();

    void Launch();
    void Prepare();

    // Quick storage of map data for checking instance allocation later
    void AddMapData(MapEntry *entry, Map *map) { m_mapData.insert(std::make_pair(entry->MapID, map)); }

    // Grab that instance stuff
    MapInstance *GetInstanceForObject(WorldObject *obj);
    // Pre teleport check for instance creation 
    uint32 PreTeleportInstanceCheck(uint64 guid, uint32 mapId, uint32 instanceId, bool canCreate = true);

private:
    Map *GetMapData(uint32 mapId) { return m_mapData.find(mapId) == m_mapData.end() ? NULL : m_mapData.at(mapId); }
    void _AddInstance(uint32 instanceId, MapInstance *instance);
    MapInstance *_LoadInstance(uint32 mapId, uint32 instanceId);

    friend class InstanceManagerSlave;
    void HandleUpdateRequests(InstanceManagerSlave *slaveThis);

    // Processing queue lock
    Mutex instancePoolLock, instanceStorageLock;
    std::deque<MapInstanceContainer*> mInstancePool;

    // First is instance id, second is pointer
    std::map<uint32, std::pair<MapInstanceContainer*, MapInstance*>> mInstanceStorage;

    // Counter
    uint32 m_instanceCounter;

    // instance data storage
    InstanceDataMap m_instanceData;

    // Map data storage
    std::map<uint32, Map*> m_mapData;
};

extern SERVER_DECL InstanceManager sInstanceMgr;

class MapInstanceContainer
{
public:
    MapInstanceContainer(MapInstance *instance) : _lastUpdateTimer(0), _instance(instance) {}

    void Invalidate() { _instance = NULL; _lastUpdateTimer = 0; }
    MapInstance *Get() { return _instance; }
    bool Validate(uint32 msTime, uint32 &diff)
    {
        // Calculate diff between last update and now
        diff = getMSTimeDiff(msTime, _lastUpdateTimer);
        // Profile diff and update timout if needed
        if(diff > MapInstanceUpdatePeriod)
            return true;
        return false;
    }

    void ResetTimer(uint32 msTime) { _lastUpdateTimer = msTime; }

private:
    uint32 _lastUpdateTimer;
    MapInstance *_instance;
};

class InstanceManagerSlave : public ThreadContext
{
public: // Just make calls into instance management, then return true for auto deletion.
    bool run() { sInstanceMgr.HandleUpdateRequests(this); return true; }
};

class InstanceData
{
public:
    void LoadFromDB(Field * fields);
    void SaveToDB();
    void DeleteFromDB();

    uint32 GetInstanceId() { return m_instanceId; }
    uint32 GetMapId() { return m_mapId; }

    WoWGuid getCreatorGuid() { return m_creatorGuid; }
    uint32 getCreatorGroupID() { return m_creatorGroup; }

    time_t getCreationTime() { return m_creation; }

    uint32 getDifficulty() { return m_difficulty; }

    time_t getExpirationTime() { return m_expiration; }

    bool isBattleground() { return m_isBattleground; }

    void AcquireSaveLock() { m_savedLock.Acquire(); }
    void ReleaseSaveLock() { m_savedLock.Release(); }

    void AddKilledNPC(uint32 counter) { m_killedNpcs.insert(counter); }
    void AddSavedPlayer(uint32 counter) { m_SavedPlayers.insert(counter); }
    void AddEnteredPlayer(uint32 counter) { m_EnteredPlayers.insert(counter); }

    bool HasKilledNPC(uint32 counter) { return m_killedNpcs.find(counter) != m_killedNpcs.end(); }
    bool HasSavedPlayer(uint32 counter) { return m_SavedPlayers.find(counter) != m_SavedPlayers.end(); }
    bool HasEnteredPlayer(uint32 counter) { return m_EnteredPlayers.find(counter) != m_EnteredPlayers.end(); }

private:
    uint32 m_instanceId;
    uint32 m_mapId;

    WoWGuid m_creatorGuid;
    uint32 m_creatorGroup;
    time_t m_creation;

    uint32 m_difficulty;

    time_t m_expiration;
    bool m_isBattleground;

    Mutex m_savedLock;
    std::set<uint32> m_killedNpcs;
    std::set<uint32> m_SavedPlayers;
    std::set<uint32> m_EnteredPlayers;
};
