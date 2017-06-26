/*
 * Sandshroud Project Ronin
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
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

class MapInstanceContainer;
class InstanceData;

// Each instance has it's own instance data linked to unique IDs
typedef std::map<uint32, InstanceData*> InstanceDataMap;

class SERVER_DECL InstanceManager
{
    friend class WorldManager;
public:
    InstanceManager();
    ~InstanceManager();

    void Destruct();

    void Launch();
    void Prepare();

private:
    void _LoadInstances();

public:
    MapScript *AllocateMapScript(MapInstance *instance);
    void AssignMapScriptAllocator(uint32 mapId, MapScriptAllocator *allocator);

private:
    void SetupInstanceScripts();

    std::map<uint32, MapScriptAllocator*> m_mapScriptAllocators;

public:
    // Loading instances for LFD system
    void LaunchGroupFinderDungeon(uint32 mapId, GroupFinderMgr::GroupFinderDungeon *dungeon, Group *grp);

    // Quick storage of map data for checking instance allocation later
    void AddMapData(MapEntry *entry, Map *map) { mapDataLock.Acquire(); m_mapData.insert(std::make_pair(entry->MapID, map)); mapDataLock.Release(); }

    // Grab that instance stuff
    MapInstance *GetInstanceForObject(WorldObject *obj);

    void ResetInstanceLinks(Player *plr);
    bool GetLinkedInstanceID(Player *plr, MapEntry *map, uint32 difficulty, uint32 &instanceId, bool groupFinder);

    // Pre teleport check for instance creation 
    uint32 PreTeleportInstanceCheck(PlayerInfo *info, MapEntry *map, uint32 instanceId);

    uint32 AllocateCreatureGuid() { counterLock.Acquire(); uint32 ret = ++m_creatureGUIDCounter; counterLock.Release(); return ret; };
    uint32 AllocateGameObjectGuid() { counterLock.Acquire(); uint32 ret = ++m_gameObjectGUIDCounter; counterLock.Release(); return ret; };

    bool LinkGuidToInstance(MapInstance *instance, WoWGuid guid, bool groupFinder);

private:
    Map *GetMapData(uint32 mapId) { Map *ret = NULL; mapDataLock.Acquire(); ret = m_mapData.find(mapId) == m_mapData.end() ? NULL : m_mapData.at(mapId); mapDataLock.Release(); return ret; }
    void _AddInstance(uint32 instanceId, MapInstance *instance);
    MapInstance *_LoadInstance(uint32 mapId, uint32 instanceId);

    friend class InstanceManagerSlave;
    void HandleUpdateRequests(InstanceManagerSlave *slaveThis);

    // Processing queue lock
    Mutex instancePoolLock, instanceStorageLock;
    std::deque<MapInstanceContainer*> mInstancePool;

    // First is instance id, second is pointer
    std::map<uint32, std::pair<MapInstanceContainer*, MapInstance*>> mInstanceStorage;

    Mutex counterLock;
    // Counter
    uint32 m_instanceCounter;
    // Creature guids
    uint32 m_creatureGUIDCounter;
    // Gameobject guids
    uint32 m_gameObjectGUIDCounter;

    // instance data storage
    InstanceDataMap m_instanceData;

    // Instance player linking
    typedef std::map<WoWGuid, Loki::AssocVector<uint32, uint32>> LinkedGuidDungeons;
    typedef std::map<uint32, std::set<WoWGuid>> DungeonLinkedGuids;

    LinkedGuidDungeons m_guidLinkedDungeons;
    DungeonLinkedGuids m_dungeonLinkedGuids;

    // Map data storage
    Mutex mapDataLock;
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
    InstanceData() = delete; // Don't allow allocation outside initializing
    InstanceData(uint32 mapId, uint32 instanceId, WoWGuid guid = 0, WoWGuid guid2 = 0, time_t start = 0, time_t expire = 0, uint16 diff = 0)
        : m_mapId(mapId), m_instanceId(instanceId), m_creatorGuid(guid), m_creatorGroup(guid2), m_creation(start), m_expiration(expire), m_difficulty(diff), m_isUpdated(false), m_isBattleground(false) {}

    void Update(uint32 msTime);

    bool LoadFromDB(Field *fields, std::vector<std::pair<WoWGuid, uint8>> &spawnState);
    void SaveToDB();
    void DeleteFromDB();

    uint32 GetInstanceId() { return m_instanceId; }
    uint32 GetMapId() { return m_mapId; }

    WoWGuid getCreatorGuid() { return m_creatorGuid; }
    WoWGuid getCreatorGroupID() { return m_creatorGroup; }

    time_t getCreationTime() { return m_creation; }

    uint32 getDifficulty() { return m_difficulty; }

    time_t getExpirationTime() { return m_expiration; }

    bool isBattleground() { return m_isBattleground; }

    void AcquireSaveLock() { m_savedLock.Acquire(); }
    void ReleaseSaveLock() { m_savedLock.Release(); }

    bool GetObjectState(WoWGuid guid, uint8 &stateOut);
    void AddObjectState(WoWGuid guid, uint8 state);

    void SetUpdated() { m_isUpdated = true; }

private:
    uint32 m_instanceId;
    uint32 m_mapId;

    WoWGuid m_creatorGuid, m_creatorGroup;
    time_t m_creation, m_expiration;

    uint16 m_difficulty;

    bool m_isUpdated, m_isBattleground;

    Mutex m_savedLock;
    Loki::AssocVector<WoWGuid, uint8> m_objectState;
};
