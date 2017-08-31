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

//
// MapInstance->h
//

#pragma once

class MapCell;
class MapScript;
class WorldObject;
class InstanceData;
class WorldSession;
class GameObject;
class Creature;
class Player;
class Pet;
class Transporter;
class Corpse;
class CBattleground;
class Transporter;
class MapTargetCallback;
class DynamicObjectTargetCallback;
class SpellTargetClass;

// Keep in mind, squared size of a cell is around 4350, we only parse two cells around so max attainable view distance is around 18000([cell size * 2] ^ 2), so we scale to 2 cells away or 40K (([cell size * 3] ^ 2)) and cut down to inbetween, 28-35K
static const uint32 MaxViewDistance = 28000;

#define TRIGGER_INSTANCE_EVENT( Mgr, Func )
#define VECTOR_POOLS 1

template <class T> class StoragePoolTask : public ThreadManager::PoolTask
{
public:
#ifdef VECTOR_POOLS
    StoragePoolTask(std::vector<T*> *pool, uint32 msTime, uint32 diff) : targetPool(pool), _msTime(msTime), _diff(diff) { }
#else
    StoragePoolTask(std::set<T*> *pool, uint32 msTime, uint32 diff) : targetPool(pool), _msTime(msTime), _diff(diff) { }
#endif

    virtual int call()
    {
        std::for_each(targetPool->begin(), targetPool->end(), [this](T *elem)
        {
            if(elem->IsActiveObject() && !elem->IsActivated())
                elem->InactiveUpdate(_msTime, _diff);
            else elem->Update(_msTime, _diff);
        } );
        return 0;
    }

private:
    uint32 _msTime, _diff;
#ifdef VECTOR_POOLS
    std::vector<T*> *targetPool;
#else
    std::set<T*> *targetPool;
#endif
};

/// Storage pool used to store and dynamically update objects in our map
template < class T > class SERVER_DECL StoragePool
{
public: // Defined type
#ifdef VECTOR_POOLS
    typedef std::vector<T*> PoolSet;
    #define POOL_FIND(itr, pool, val) itr = std::find(pool.begin(), pool.end(), val)
    #define POOL_ADD(pool, obj) pool.push_back(obj)
#else
    typedef std::set<T*> PoolSet;
    #define POOL_FIND(itr, pool, val) itr = pool.find(val)
    #define POOL_ADD(pool, obj) pool.insert(obj)
#endif

public:
    StoragePool() : m_updating(false), mPoolCounter(0), mFullPoolSize(0), mPoolSize(0) { mPoolStack = NULL; mPoolLastUpdateStack = NULL; }

    void Initialize(uint32 poolSize)
    {
        Guard guard(poolLocks);
        mFullPoolSize = mPoolSize = poolSize;
        mPoolCounter = 0;
        if(mFullPoolSize == 1) // Don't initialize the single stack
            return;
        mPoolLastUpdateStack = new uint32[mFullPoolSize];
        mPoolStack = new PoolSet[mFullPoolSize];
        for(uint32 i = 0; i < mFullPoolSize; ++i)
            m_poolSizes[i] = 0;
    }

    void PreservePools(uint32 preserveSize)
    {
        ASSERT(preserveSize < mPoolSize);
        mPoolSize -= preserveSize;
    }

    void Cleanup()
    {
        Guard guard(poolLocks);
        if(mPoolLastUpdateStack)
            delete [] mPoolLastUpdateStack;
        mPoolLastUpdateStack = NULL;
        if(mPoolStack)
            delete [] mPoolStack;
        mPoolStack = NULL;
        mPool.clear();
    }

    bool isUpdating() { return m_updating; }
    uint32 getCounter() { return mPoolCounter; }

    // Update our object stack, this includes inactivity timers
    void Update(uint32 msTime, uint32 pDiff, ThreadManager::TaskPool *taskPool)
    {
        poolLocks.Acquire();
        std::vector<std::pair<uint32, PoolSet*>> targetPools;
        if(mPoolStack == NULL) // No stack so use our main pool
            targetPools.push_back(std::make_pair(pDiff, &mPool));
        else
        {
            uint32 pushCount = taskPool ? std::min<uint32>(mFullPoolSize, taskPool->getThreadCount()) : 1;
            for(uint32 i = 0; i < pushCount; ++i)
            {
                // Select our next pool to update in the sequence
                if(++mPoolCounter == mFullPoolSize)
                    mPoolCounter = 0;
                // Recalculate the diff from the last time we updated this pool
                uint32 diff = msTime - mPoolLastUpdateStack[mPoolCounter];
                mPoolLastUpdateStack[mPoolCounter] = msTime;
                // Set the target pool pointer
                targetPools.push_back(std::make_pair(diff, &mPoolStack[mPoolCounter]));
            }
        }

        if(taskPool == NULL)
            poolLocks.Release();

        m_updating = true;
        while(!targetPools.empty())
        {
            uint32 updateDiff = (*targetPools.begin()).first;
            PoolSet *targetPool = (*targetPools.begin()).second;
            targetPools.erase(targetPools.begin());
            if(!targetPool->empty())
            {
                if(taskPool)
                    taskPool->AddTask(new StoragePoolTask<T>(targetPool, msTime, updateDiff));
                else
                {
                    std::for_each(targetPool->begin(), targetPool->end(), [this, msTime, updateDiff](T *elem)
                    {
                        if(elem->IsActiveObject() && !elem->IsActivated())
                            elem->InactiveUpdate(msTime, updateDiff);
                        else elem->Update(msTime, updateDiff);
                    } );
                }
            }
        }

        if(taskPool != NULL)
            poolLocks.Release();

        if(taskPool != NULL)
            taskPool->wait();
        m_updating = false;
    }

    // Resets the stack timers
    void ResetTime(uint32 msTime)
    {
        Guard guard(poolLocks);
        if(mPoolStack == NULL)
            return;

        // Times have to be reset for our pools so we don't have massive differences from currentms-0
        for(uint32 i = 0; i < mFullPoolSize; i++)
            mPoolLastUpdateStack[i] = msTime;
    }

    // Add our object to the stack
    void Add(T *obj, uint8 forcedPool = 0)
    {
        Guard guard(poolLocks);
        std::map<T*, uint8>::iterator checkItr;
        if((checkItr = _poolTracking.find(obj)) != _poolTracking.end())
            return;

        uint8 pool = 0xFF;
        POOL_ADD(mPool, obj);
        if(mPoolStack)
        {
            if((pool = forcedPool) == 0 || pool >= mFullPoolSize)
            {
                // Grab us from the front
                pool = 0;
                uint32 count = 0xFFFFFFFF;
                for(auto itr = m_poolSizes.begin(); itr != m_poolSizes.end(); ++itr)
                {
                    if(itr->second < count)
                    {
                        pool = itr->first;
                        count = itr->second;
                    }
                }
            }

            POOL_ADD(mPoolStack[pool], obj);
            m_poolSizes[pool] = mPoolStack[pool].size();
        }
        _poolTracking.insert(std::make_pair(obj, pool));
    }

#if STACKED_MEMORY_ALLOCATION == 1
    // Add a pool of stack allocated objects to the stack
    void AddPool(T *objStack, uint32 poolSize)
    {
        Guard guard(poolLocks);
        uint8 pool = 0xFF;
        if(mPoolStack)
        {
            // Grab us from the front
            pool = 0;
            uint32 count = 0xFFFFFFFF;
            for(auto itr = m_poolSizes.begin(); itr != m_poolSizes.end(); ++itr)
            {
                if(itr->second < count)
                {
                    pool = itr->first;
                    count = itr->second;
                    if(count == 0)
                        break;
                }
            }
        }

        for(uint32 i = 0; i < poolSize; ++i)
        {
            T *obj = &objStack[i];

            POOL_ADD(mPool, obj);
            if(mPoolStack)
            {
                POOL_ADD(mPoolStack[pool], obj);
                m_poolSizes[pool] = mPoolStack[pool].size();
            }
            _poolTracking.insert(std::make_pair(obj, pool));
        }
    }
#endif

    // Remove our object from the stack, poolID is needed to remove from the correct stack quickly
    void QueueRemoval(T *obj)
    {
        Guard guard(poolLocks);
        std::map<T*, uint8>::iterator checkItr;
        if((checkItr = _poolTracking.find(obj)) == _poolTracking.end())
            return;
        uint8 poolId = checkItr->second;
        _poolTracking.erase(checkItr);
        _pendingRemovals.push_back(std::make_pair(obj, poolId));
    }

    // Delayed removal of objects
    void ProcessRemovals()
    {
        std::for_each(_pendingRemovals.begin(), _pendingRemovals.end(), [this](std::pair<T*, uint8> pair)
        {
            T *obj = pair.first;
            uint8 poolId = pair.second;

            PoolSet::iterator itr;
            if((POOL_FIND(itr, mPool, obj)) != mPool.end())
                mPool.erase(itr);

            if(mPoolStack && poolId != 0xFF)
            {
                if((POOL_FIND(itr, mPoolStack[poolId], obj)) != mPoolStack[poolId].end())
                {
                    mPoolStack[poolId].erase(itr);
                    m_poolSizes[poolId] = mPoolStack[poolId].size();
                }
            }
        } );
        _pendingRemovals.clear();
    }

    typename PoolSet::iterator begin() { return mPool.begin(); };
    typename PoolSet::iterator end() { return mPool.end(); };

private:

    Mutex poolLocks;
    bool m_updating;
    std::map<T*, uint8> _poolTracking;
    std::vector<std::pair<T*, uint8>> _pendingRemovals;

    PoolSet mPool, *mPoolStack;
    std::map<uint32, uint32> m_poolSizes;
    uint32 mPoolCounter, mFullPoolSize, mPoolSize, *mPoolLastUpdateStack;
};

class MapInstanceObjectProcessCallback : public ObjectProcessCallback
{
public:
    MapInstanceObjectProcessCallback(MapInstance *instance) : _instance(instance) {}
    void operator()(WorldObject *obj, WorldObject *curObj);
    void SetCell(uint32 cellX, uint32 cellY) { _cellX = cellX; _cellY = cellY; }

protected:
    MapInstance *_instance;
    uint32 _cellX, _cellY;
};

class MapInstanceObjectRemovalCallback : public ObjectProcessCallback
{
public:
    MapInstanceObjectRemovalCallback(MapInstance *instance) : _instance(instance) {}
    void operator()(WorldObject *obj, WorldObject *curObj);
    void setForced(bool set) { _forced = set; }

protected:
    MapInstance *_instance;
    bool _forced;
};

class MapInstanceInRangeTargetCallback : public ObjectProcessCallback
{
public:
    MapInstanceInRangeTargetCallback(MapInstance *instance) : _instance(instance) {}
    void operator()(WorldObject *obj, WorldObject *curObj);
    void ResetData(float range) { _range = range; _result = NULL; _resultDist = 0.f; };
    Unit *GetResult() { return _result; }

protected:
    MapInstance *_instance;

    float _range;
    Unit *_result;
    float _resultDist;
};

class MapInstanceBroadcastMessageCallback : public ObjectProcessCallback
{
public:
    MapInstanceBroadcastMessageCallback(MapInstance *instance) : _instance(instance) {}

    void operator()(WorldObject *obj, WorldObject *curObj);
    void setPacketData(WorldPacket *data) { _packet = data; }

protected:
    MapInstance *_instance;
    WorldPacket *_packet;
};

class MapInstanceBroadcastMessageInrangeCallback : public MapInstanceBroadcastMessageCallback
{
public:
    MapInstanceBroadcastMessageInrangeCallback(MapInstance *instance) : MapInstanceBroadcastMessageCallback(instance) {}
    void operator()(WorldObject *obj, WorldObject *curObj);

    void ResetData(float range, WorldPacket *data, bool myTeam, uint32 teamId) { setPacketData(data); _range = range; _myTeam = myTeam; _teamId = teamId; _opcode = 0; _dataLen = 0; _dataStream = NULL; };
    void ResetData(float range, uint32 opcode, uint16 Len, const void *data, bool myTeam, uint32 teamId) { _range = range; _dataLen = Len; _dataStream = data; _myTeam = myTeam; _teamId = teamId; setPacketData(NULL); };

protected:
    bool _myTeam;
    uint32 _teamId;
    float _range;

    uint32 _opcode;
    uint16 _dataLen;
    const void *_dataStream;
};

class MapInstanceBroadcastObjectUpdateCallback : public ObjectProcessCallback
{
public:
    MapInstanceBroadcastObjectUpdateCallback(MapInstance *instance) : _instance(instance) {}
    void operator()(WorldObject *obj, WorldObject *curObj);

private:
    MapInstance *_instance;
};

class MapInstanceBroadcastChatPacketCallback : public ObjectProcessCallback
{
public:
    MapInstanceBroadcastChatPacketCallback(MapInstance *instance) : _instance(instance) {}

    void operator()(WorldObject *obj, WorldObject *curObj);
    void setPacketData(WorldPacket *data, int32 lang, uint32 langPos, uint32 guidPos)
    {
        _packet = data;
        _defaultLang = lang;
        _langPos = langPos;
        _guidPos = guidPos;
    }

private:
    MapInstance *_instance;
    WorldPacket *_packet;

    int32 _defaultLang;
    uint32 _langPos, _guidPos;
};

class MapInstanceDynamicObjectTargetMappingCallback : public ObjectProcessCallback
{
public:
    MapInstanceDynamicObjectTargetMappingCallback(MapInstance *instance) : _instance(instance) {}
    void operator()(WorldObject *obj, WorldObject *curObj);
    void SetData(DynamicObjectTargetCallback *callback, DynamicObject *dynObj, Unit *caster, float minRange, float maxRange)
    {
        _callback = callback;
        _dynObject = dynObj;
        _caster = caster;

        _minRange = minRange;
        _maxRange = maxRange;
    }

private:
    MapInstance *_instance;
    DynamicObjectTargetCallback *_callback;
    DynamicObject *_dynObject;
    Unit *_caster;

    float _minRange, _maxRange;
};

class MapInstanceSpellTargetMappingCallback : public ObjectProcessCallback
{
public:
    MapInstanceSpellTargetMappingCallback(MapInstance *instance) : _instance(instance) {}
    void operator()(WorldObject *obj, WorldObject *curObj);
    void SetData(MapTargetCallback *callback, SpellTargetClass *spell, uint32 i, uint32 targetType, float x, float y, float z, float minRange, float maxRange)
    {
        _callback = callback;
        _spell = spell;
        _effIndex = i;
        _targetType = targetType;
        _x = x; _y = y; _z = z;
        _minRange = minRange;
        _maxRange = maxRange;
    }

private:
    MapInstance *_instance;
    MapTargetCallback *_callback;
    SpellTargetClass *_spell;

    uint32 _effIndex, _targetType;
    float _x, _y, _z, _minRange, _maxRange;
};

template <class T> class CallbackStack
{
public:
    class callbackStorage
    {
    public:
        callbackStorage(MapInstance *instance) : callback(instance) {}

        T callback;
        std::vector<uint32> cellvector;
    };

    callbackStorage *getOrAllocateCallback(uint32 threadId, MapInstance *instance)
    {
        allocationLock.Acquire();
        callbackStorage *ret = NULL;
        std::map<uint32, callbackStorage*>::iterator itr;
        if((itr = m_callbackStorages.find(threadId)) != m_callbackStorages.end())
            ret = itr->second;
        else m_callbackStorages.insert(std::make_pair(threadId, ret = new callbackStorage(instance)));
        allocationLock.Release();
        return ret;
    }

    void cleanup()
    {
        for(std::map<uint32, callbackStorage*>::iterator itr = m_callbackStorages.begin(); itr != m_callbackStorages.end(); ++itr)
            delete itr->second;
        m_callbackStorages.clear();
    }

private:
    Mutex allocationLock;
    std::map<uint32, callbackStorage*> m_callbackStorages;
};

/// Map instance class for processing different map instances(duh)
class SERVER_DECL MapInstance : public CellHandler <MapCell>
{
    friend class UpdateObjectThread;
    friend class ObjectUpdaterThread;
    friend class MapCell;
public:

    typedef std::set<WorldObject*> ObjectSet;
    typedef std::set<Player*> PlayerSet;
    typedef std::set<Creature*> CreatureSet;
    typedef std::set<GameObject*> GameObjectSet;
    typedef std::set<WoWGuid> CombatProgressSet;
    typedef Loki::AssocVector<uint32, Creature*> CreatureSqlIdMap;
    typedef Loki::AssocVector<uint32, GameObject* > GameObjectSqlIdMap;

    typedef Loki::AssocVector<WoWGuid, Creature*> CreatureStorageMap;
    typedef Loki::AssocVector<WoWGuid, GameObject* > GameObjectStorageMap;
    typedef Loki::AssocVector<WoWGuid, DynamicObject*> DynamicObjectStorageMap;

//////////////////////////////////////////////////////////
// Map initializers and functions
///////////////////////////////////
public:
    MapInstance(Map *map, uint32 mapid, uint32 instanceid, InstanceData *data = NULL);
    ~MapInstance();

    void Preload();
    void Init(uint32 msTime);
    void Destruct();

    WorldObject *GetInRangeObject(ObjectCellManager *manager, WoWGuid guid);

    void EventPushObjectToSelf(WorldObject *obj);

    void PushObject(WorldObject* obj);
    void RemoveObject(WorldObject* obj);

    void QueueRemoval(WorldObject *obj);
    void QueueCleanup(WorldObject *obj);

    void QueueSoftDisconnect(Player *plr);

    virtual void ChangeObjectLocation(WorldObject* obj); // update inrange lists
    void ChangeFarsightLocation(Player* plr, Unit* farsight, bool apply);
    void ChangeFarsightLocation(Player* plr, float X, float Y, bool apply);

    static bool canObjectsInteract(WorldObject *obj, WorldObject *curObj);
    static bool IsInRange(float fRange, WorldObject* obj, WorldObject* currentobj, float &distOut);
    static bool InZRange(float fRange, WorldObject* obj, WorldObject* currentobj);

    void OnGroupEnter(Player *plr, Group *grp);

    // Generally an expansion indicator
    uint32 GetZoneModifier(uint32 zoneId);

    // Append quests based on guid and scripts
    void AppendQuestList(WoWGuid guid, Player *plr, uint32 &count, WorldPacket *packet);

    void SetUnitState(WoWGuid guid, uint8 state);
    void SetGameObjState(WoWGuid guid, uint8 state);

public:
    // Combat functions
    bool CheckCombatStatus(Unit *unit = NULL);
    void ClearCombatTimers(WoWGuid guid, WoWGuid guid2 = WoWGuid());
    void TriggerCombatTimer(WoWGuid guid, WoWGuid guid2, uint32 timer);

private:
    Mutex m_combatLock;
    bool m_forceCombatState;
    std::map<WoWGuid, std::set<WoWGuid>> m_combatPartners;
    std::map<std::pair<WoWGuid, WoWGuid>, uint32> m_combatTimers;

public:
    // Cell walking functions
    Unit *FindInRangeTarget(Creature *ctr, float range, uint32 typeMask);

    void MessageToCells(WorldObject *obj, uint16 opcodeId, uint16 Len, const void *data, float range);
    void MessageToCells(WorldObject *obj, WorldPacket *data, float range, bool myTeam, uint32 teamId);

    void BroadcastObjectUpdate(WorldObject *obj);

    void UpdateObjectCellVisibility(WorldObject *obj, std::vector<uint32> *cellVector);

    // Dynamic object target mapping
    void HandleDynamicObjectRangeMapping(DynamicObjectTargetCallback *callback, DynamicObject *object, Unit *caster, float minRange, float maxRange, uint32 typeMask = 0);

    // Spell targetting functions: note range is squared in the function, do not input squared range
    void HandleSpellTargetMapping(MapTargetCallback *callback, SpellTargetClass *spell, uint32 i, uint32 targetType, float x, float y, float z, float minRange, float maxRange, uint32 typeMask = 0);

protected:
    // These are stored in MapInstance.cpp with functions
    friend class MapInstanceObjectProcessCallback;
    MapInstanceObjectProcessCallback _processCallback;

    friend class MapInstanceObjectRemovalCallback;
    MapInstanceObjectRemovalCallback _removalCallback;

    typedef CallbackStack<MapInstanceInRangeTargetCallback> InrangeTargetCallbackStack;
    friend class MapInstanceInRangeTargetCallback;
    InrangeTargetCallbackStack _inRangeTargetCBStack;

    typedef CallbackStack<MapInstanceBroadcastMessageCallback> BroadcastMessageCallbackStack;
    friend class MapInstanceBroadcastMessageCallback;
    BroadcastMessageCallbackStack _broadcastMessageCBStack;

    typedef CallbackStack<MapInstanceBroadcastMessageInrangeCallback> BroadcastMessageInRangeCallbackStack;
    friend class MapInstanceBroadcastMessageInrangeCallback;
    BroadcastMessageInRangeCallbackStack _broadcastMessageInRangeCBStack;

    typedef CallbackStack<MapInstanceBroadcastChatPacketCallback> BroadcastChatPacketCallbackStack;
    friend class MapInstanceBroadcastChatPacketCallback;
    BroadcastChatPacketCallbackStack _broadcastChatPacketCBStack;

    typedef CallbackStack<MapInstanceBroadcastObjectUpdateCallback> BroadcastObjectUpdateCallbackStack;
    friend class MapInstanceBroadcastObjectUpdateCallback;
    BroadcastObjectUpdateCallbackStack _broadcastObjectUpdateCBStack;

    typedef CallbackStack<MapInstanceDynamicObjectTargetMappingCallback> DynamicObjectTargetMappingCallbackStack;
    friend class MapInstanceDynamicObjectTargetMappingCallback;
    DynamicObjectTargetMappingCallbackStack _dynamicObjectTargetMappingCBStack;

    typedef CallbackStack<MapInstanceSpellTargetMappingCallback> SpellTargetMappingCallbackStack;
    friend class MapInstanceSpellTargetMappingCallback;
    SpellTargetMappingCallbackStack _spellTargetMappingCBStack;

    // These are stored in SpellTargets.cpp with functions
public:

    //! Mark object as updated
    bool UpdateQueued(WorldObject *obj);
    void ObjectUpdated(WorldObject *obj);
    void ObjectLocationChange(WorldObject *obj);
    void UpdateCellActivity(uint32 x, uint32 y, int radius);

    // Calc functions
    float GetWalkableHeight(WorldObject *obj, float x, float y, float z);

    // Terrain Functions
    uint16 GetADTAreaId(float x, float y);
    float GetADTLandHeight(float x, float y);
    float GetADTWaterHeight(float x, float y, uint16 &outType);

    RONIN_INLINE uint32 GetMapId() { return _mapId; }
    void AddForcedCell(MapCell * c, uint32 range = 1);
    void RemoveForcedCell(MapCell * c, uint32 range = 1);

    void PushToProcessed(Player* plr);

    RONIN_INLINE bool HasPlayers() { return (m_PlayerStorage.size() > 0); }
    void TeleportPlayers();

    bool CanLootPlayers(Player *plr) { return false; }
    RONIN_INLINE virtual bool IsInstance() { return pdbcMap && (pdbcMap->IsDungeon() || pdbcMap->IsRaid()); }
    RONIN_INLINE uint32 GetInstanceID() { return m_instanceID; }
    RONIN_INLINE MapEntry *GetdbcMap() { return pdbcMap; }
    bool CanUseCollision(WorldObject* obj);

    virtual int32 event_GetMapID() { return _mapId; }

    void UpdateAllCells(bool apply, uint32 areamask = 0);
    RONIN_INLINE size_t GetPlayerCount() { return m_PlayerStorage.size(); }

    void _ProcessInputQueue();
    void _PerformScriptUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformCombatUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformPlayerUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformCreatureUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformObjectUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformDynamicObjectUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformDelayedSpellUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformUnitPathUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformMovementUpdates(bool includePlayers);
    void _PerformSessionUpdates();
    void _PerformPendingUpdates();
    void _PerformPendingActions();

    void EventCorpseDespawn(uint64 guid);

    time_t InactiveMoveTime;
    uint32 iInstanceMode;

    RONIN_INLINE void AddSpawn(uint32 x, uint32 y, CreatureSpawn * sp)
    {
        GetBaseMap()->GetSpawnsListAndCreate(x, y)->CreatureSpawns.push_back(sp);
    }

    RONIN_INLINE void AddGoSpawn(uint32 x, uint32 y, GameObjectSpawn * gs)
    {
        GetBaseMap()->GetSpawnsListAndCreate(x, y)->GameObjectSpawns.push_back(gs);
    }

    void UnloadCell(uint32 x,uint32 y);
    void SendMessageToCellPlayers(WorldObject* obj, WorldPacket * packet, uint32 cell_radius = 2);
    void SendChatMessageToCellPlayers(WorldObject* obj, WorldPacket * packet, uint32 cell_radius, int32 lang, uint32 langpos, uint32 guidPos);
    void BeginInstanceExpireCountdown();
    void HookOnAreaTrigger(Player* plr, uint32 id);

    RONIN_INLINE void AddProjectile(Spell *spell)
    {
        m_poolLock.Acquire();
        uint8 index = projectileSpellIndex[1]++;
        if(projectileSpellIndex[1] == 2)
            projectileSpellIndex[1] = 0;
        m_projectileSpells[index].insert(spell);
        m_poolLock.Release();
    }

protected: ///! Instance identification data
    //! Our instance's map ID
    uint32 _mapId;
    //! Our instance's Instance ID, continent = 0
    uint32 m_instanceID;
    //! Script tied to map by MapID
    MapScript *m_script;
    //! Active conditions
    std::set<uint32> m_activeConditions;

protected: ///! Objects that exist on map
    // In this zone, we always show these objects
    Loki::AssocVector<WorldObject*, uint32> m_zoneFullRangeObjects, m_areaFullRangeObjects;
    Loki::AssocVector<uint32, std::vector<WorldObject*>> m_fullRangeObjectsByZone, m_fullRangeObjectsByArea;

    bool IsFullRangeObject(WorldObject *obj);

    MapCell *GetCellOrInit(uint32 x, uint32 y, bool shouldInit, bool priority);
    bool _CellActive(uint32 x, uint32 y);

    void UpdateObjectVisibility(Player *plObj, WorldObject *curObj);

    friend class ObjectCellManager;
    friend class PlayerCellManager;
    bool UpdateCellData(WorldObject *Obj, uint32 cellX, uint32 cellY, bool playerObj, bool priority);
    void RemoveCellData(WorldObject *Obj, std::set<uint32> &set, bool forced);

public:
    bool IsPreloading() { return m_mapPreloading; }
    bool IsRaid() { return pdbcMap ? pdbcMap->IsRaid() : false; }
    bool IsContinent() { return pdbcMap ? pdbcMap->IsContinent() : true; }

    uint8 GetPoolOverrideForZone(uint32 zoneId);
    bool IsCreaturePoolUpdating() { return mCreaturePool.isUpdating(); }
    bool IsGameObjectPoolUpdating() { return mGameObjectPool.isUpdating(); }

    void WaitForTaskPool() { if(_updatePool) _updatePool->wait(); }

    bool HasActivatedCondition(uint32 conditionId, WorldObject *obj);

    void CellActionPending(uint16 x, uint16 y) { m_pendingCellActions.insert(std::make_pair(x, y)); }

protected:
    /* Map Information */
    MapEntry* pdbcMap;

    /* Update System */
    Mutex m_updateMutex, m_setLock;
    std::set<std::pair<uint16, uint16>> m_pendingCellActions;
    ObjectSet _updates, _movedObjects, _pendingRemoval, _pendingCleanup;
    PlayerSet _processQueue, _movedPlayers, _softDCPlayers;

    /* Sessions */
    SessionSet MapSessions;

    // Map preloading to push back updating inrange objects
    bool m_mapPreloading;

public:
    Mutex m_poolLock;
    ThreadManager::TaskPool *_updatePool;
    StoragePool<Creature> mCreaturePool;
    StoragePool<GameObject> mGameObjectPool;
    StoragePool<DynamicObject> mDynamicObjectPool;
    StoragePool<UnitPathSystem> mUnitPathPool;

    // Storage pool setup for projectile spells
    uint32 projectileSpellUpdateTime[2];
    uint8 projectileSpellIndex[2];
    std::set<Spell*> m_projectileSpells[2];
    // End storage pool setup for projectile spells

    CBattleground* m_battleground;
    std::vector<Corpse* > m_corpses;

    // world state manager stuff
    WorldStateManager* m_stateManager;

    // bytebuffer caching
    ByteBuffer m_createBuffer, m_updateBuffer;

    ByteBuffer *GetCreateBuffer() { return &m_createBuffer; }
    ByteBuffer *GetUpdateBuffer() { return &m_updateBuffer; }

    // Object cell stacking
    Mutex objectCellCacheLock;
    std::map<WoWGuid, uint32> m_objectCells;

    void CacheObjectCell(WoWGuid guid, uint32 cellId) { objectCellCacheLock.Acquire(); m_objectCells[guid] = cellId; objectCellCacheLock.Release(); }
    void RemoveCachedCell(WoWGuid guid) { objectCellCacheLock.Acquire(); m_objectCells.erase(guid); objectCellCacheLock.Release(); }

public:
    void ClearCorpse(Corpse* remove) { std::vector<Corpse* >::iterator itr; if((itr = std::find(m_corpses.begin(), m_corpses.end(), remove)) != m_corpses.end()) m_corpses.erase(itr); };

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

    //This will be done in regular way soon
    Mutex m_objectinsertlock;
    ObjectSet m_objectinsertpool;
    Mutex m_objectCreationLock;
    Mutex m_objectStorageLock;

    void AddObject(WorldObject*);
    WorldObject* GetObjectClosestToCoords(uint32 entry, float x, float y, float z, float ClosestDist, int32 forcedtype = -1);

    bool IsClosing() { return false; }
    uint32 IsFull(PlayerInfo *info);

    void AddZoneVisibleSpawn(uint32 zoneId, WorldObject *obj);
    void RemoveZoneVisibleSpawn(uint32 zoneId, WorldObject *obj);
    void AddAreaVisibleSpawn(uint32 areaId, WorldObject *obj);
    void RemoveAreaVisibleSpawn(uint32 areaId, WorldObject *obj);

    /////////////////////////////////////////////////////////
    // Local (MapInstance) storage/generation of Creatures
    /////////////////////////////////////////////
    uint32 m_CreatureHighGuid;
    CreatureStorageMap m_CreatureStorage;
    Creature *CreateCreature(WoWGuid guid, uint32 entry = 0);
#if STACKED_MEMORY_ALLOCATION == 1
    void ConstructCreature(WoWGuid &guid, Creature *allocation);
#endif

    RONIN_INLINE Creature* GetCreature(WoWGuid guid)
    {
        ASSERT(guid.getHigh() == HIGHGUID_TYPE_UNIT || guid.getHigh() == HIGHGUID_TYPE_VEHICLE);
        CreatureStorageMap::iterator itr = m_CreatureStorage.find(guid);
        return ((itr != m_CreatureStorage.end()) ? itr->second : NULL);
    }

    // Use a creature guid to create our summon.
    Summon* CreateSummon(uint32 entry, int32 duration);

////////////////////////////////////////////////////////
// Local (MapInstance) storage/generation of GameObjects
/////////////////////////////////////////////
    uint32 m_GOHighGuid;
    GameObjectStorageMap m_gameObjectStorage;
    GameObject *CreateGameObject(WoWGuid guid, uint32 entry = 0);
#if STACKED_MEMORY_ALLOCATION == 1
    void ConstructGameObject(WoWGuid &guid, GameObject *allocation);
#endif

    RONIN_INLINE GameObject* GetGameObject(WoWGuid guid)
    {
        ASSERT(guid.getHigh() == HIGHGUID_TYPE_GAMEOBJECT);
        GameObjectStorageMap::iterator itr = m_gameObjectStorage.find(guid);
        return (itr != m_gameObjectStorage.end()) ? itr->second : NULL;
    }

//////////////////////////////////////////////////////////
// Local (MapInstance) storage/generation of DynamicObjects
////////////////////////////////////////////
    uint32 m_DynamicObjectHighGuid;
    DynamicObjectStorageMap m_DynamicObjectStorage;
    DynamicObject* AllocateDynamicObject(WoWGuid source);

    RONIN_INLINE DynamicObject* GetDynamicObject(WoWGuid guid)
    {
        DynamicObjectStorageMap::iterator itr = m_DynamicObjectStorage.find(guid);
        return ((itr != m_DynamicObjectStorage.end()) ? itr->second : NULL);
    }

//////////////////////////////////////////////////////////
// Local (MapInstance) storage of players for faster lookup
////////////////////////////////
    typedef Loki::AssocVector<WoWGuid, Player*> PlayerStorageMap;
    PlayerStorageMap m_PlayerStorage;
    RONIN_INLINE Player* GetPlayer(WoWGuid guid)
    {
        ASSERT(guid.getHigh() == HIGHGUID_TYPE_PLAYER);
        PlayerStorageMap::iterator itr = m_PlayerStorage.find(guid);
        return (itr != m_PlayerStorage.end()) ? m_PlayerStorage[guid] : NULL;
    }
    std::queue<Player*> m_removeQueue;

    // stored iterators for safe checking
    PlayerStorageMap::iterator __player_iterator;

//////////////////////////////////////////////////////////
// Lookup Wrappers
///////////////////////////////////
    Unit* GetUnit(WoWGuid guid);
    WorldObject* _GetObject(WoWGuid guid);

private:
    InstanceData *m_iData;

    struct MapInstanceData
    {
        uint32 difficulty;
        uint32 linkedGroupId;
        uint32 encounterMask;
    } *m_instanceData;

public:
    enum PlayerTradeStatus : uint8
    {
        TRADE_STATUS_OPEN_WINDOW = 0,
        // For trading boss loot
        TRADE_STATUS_NOT_ON_TAPLIST = 2,
        TRADE_STATUS_YOU_LOGOUT = 3,
        TRADE_STATUS_IGNORE_YOU = 4,
        TRADE_STATUS_TARGET_DEAD = 5,
        TRADE_STATUS_TRADE_ACCEPT = 6,
        TRADE_STATUS_TARGET_LOGOUT = 7,
        TRADE_STATUS_TRADE_COMPLETE = 9,
        TRADE_STATUS_TRIAL_ACCOUNT = 10,
        TRADE_STATUS_BEGIN_TRADE = 12,
        TRADE_STATUS_YOU_DEAD = 13,
        TRADE_STATUS_TARGET_TO_FAR = 16,
        TRADE_STATUS_NO_TARGET = 17,
        TRADE_STATUS_BUSY_2 = 18,
        TRADE_STATUS_CURRENCY_NOT_TRADABLE = 19,
        TRADE_STATUS_WRONG_FACTION = 20,
        TRADE_STATUS_BUSY = 21,
        TRADE_STATUS_TRADE_CANCELED = 23,
        TRADE_STATUS_CURRENCY = 24,
        TRADE_STATUS_BACK_TO_TRADE = 25,
        // Can only trade conjured items
        TRADE_STATUS_WRONG_REALM = 26,
        TRADE_STATUS_YOU_STUNNED = 27,
        TRADE_STATUS_TARGET_STUNNED = 29,
        TRADE_STATUS_CLOSE_WINDOW = 31,
    };

    uint8 StartTrade(WoWGuid owner, WoWGuid target);
    void SetTradeStatus(WoWGuid trader, uint8 status);
    void SetTradeAccepted(WoWGuid trader, bool set);

private:
    struct TradeData
    {
        WoWGuid traders[2];
        bool accepted[2];

        uint64 gold[2];
        WoWGuid items[14];
        uint32 enchantId[2];

        uint8 getIndex(WoWGuid guid)
        {
            if(guid == traders[0])
                return 0x00;
            if(guid == traders[1])
                return 0x01;
            return 0xFF;
        }
    };

    std::map<WoWGuid, TradeData*> m_tradeData;
};
