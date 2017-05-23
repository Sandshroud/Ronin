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

enum ObjectActiveState
{
    OBJECT_STATE_NONE    = 0,
    OBJECT_STATE_INACTIVE = 1,
    OBJECT_STATE_ACTIVE   = 2,
};

// Keep in mind, squared size of a cell is around 4350, we only parse two cells around so max attainable view distance is around 18000([cell size * 2] ^ 2), so we scale to 2 cells away or 40K (([cell size * 3] ^ 2)) and cut down to inbetween, 28-35K
static const uint32 MaxViewDistance = 28000;

#define MAX_TRANSPORTERS_PER_MAP 25
#define RESERVE_EXPAND_SIZE 1024

#define TRIGGER_INSTANCE_EVENT( Mgr, Func )
#define VECTOR_POOLS 1

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
    StoragePool() : m_updating(false), mPoolCounter(0), mPoolAddCounter(0), mFullPoolSize(0), mPoolSize(0) { mPoolStack = NULL; mPoolLastUpdateStack = NULL; }

    void Initialize(uint32 poolSize)
    {
        mFullPoolSize = mPoolSize = poolSize;
        poolIterator = mPool.end();
        mPoolCounter = mPoolAddCounter = 0;
        if(mFullPoolSize == 1) // Don't initialize the single stack
            return;
        mPoolLastUpdateStack = new uint32[mFullPoolSize];
        mPoolStack = new PoolSet[mFullPoolSize];
    }

    void PreservePools(uint32 preserveSize)
    {
        ASSERT(preserveSize < mPoolSize);
        mPoolSize -= preserveSize;
    }

    void Cleanup()
    {
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
    void Update(uint32 msTime, uint32 pDiff)
    {
        uint32 diff = pDiff;
        PoolSet *targetPool;
        if(mPoolStack == NULL) // No stack so use our main pool
            targetPool = &mPool;
        else
        {
            // Select our next pool to update in the sequence
            if(++mPoolCounter == mFullPoolSize)
                mPoolCounter = 0;
            // Recalculate the diff from the last time we updated this pool
            diff = msTime - mPoolLastUpdateStack[mPoolCounter];
            mPoolLastUpdateStack[mPoolCounter] = msTime;
            // Set the target pool pointer
            targetPool = &mPoolStack[mPoolCounter];
        }

        if(!targetPool->empty())
        {
            m_updating = true;
            std::for_each(targetPool->begin(), targetPool->end(), [msTime, diff](T *elem)
            {
                if(elem->IsActiveObject() && !elem->IsActivated())
                    elem->InactiveUpdate(msTime, diff);
                else elem->Update(msTime, diff);
            } );
            m_updating = false;
        }
    }

    // Resets the stack timers
    void ResetTime(uint32 msTime)
    {
        if(mPoolStack == NULL)
            return;

        // Times have to be reset for our pools so we don't have massive differences from currentms-0
        for(uint32 i = 0; i < mFullPoolSize; i++)
            mPoolLastUpdateStack[i] = msTime;
    }

    // Add our object to the stack, return value is a pool identifier
    uint8 Add(T *obj, uint8 forcedPool = 0)
    {
        uint8 pool = 0xFF;
        POOL_ADD(mPool, obj);
        if(mPoolStack)
        {
            if((pool = forcedPool) == 0 || pool >= mFullPoolSize)
                pool = mPoolAddCounter++;
            if(mPoolAddCounter == mPoolSize)
                mPoolAddCounter = 0;

            POOL_ADD(mPoolStack[pool], obj);
        }
        return pool;
    }

    // Remove our object from the stack, poolID is needed to remove from the correct stack quickly
    void Remove(T *obj, uint8 poolId)
    {
        PoolSet::iterator itr;
        if((POOL_FIND(itr, mPool, obj)) != mPool.end())
        {
            // check iterator
            if( m_updating && poolIterator == itr )
                poolIterator = mPool.erase(itr);
            else mPool.erase(itr);
        }

        if(mPoolStack && poolId != 0xFF)
        {
            if((POOL_FIND(itr, mPoolStack[poolId], obj)) != mPoolStack[poolId].end())
            {
                // check iterator
                if( m_updating && poolIterator == itr )
                    poolIterator = mPoolStack[poolId].erase(itr);
                else mPoolStack[poolId].erase(itr);
            }
        }
    }

    typename PoolSet::iterator begin() { return mPool.begin(); };
    typename PoolSet::iterator end() { return mPool.end(); };

private:

    bool m_updating;
    PoolSet mPool, *mPoolStack;
    typename PoolSet::iterator poolIterator;
    uint32 mPoolCounter, mPoolAddCounter, mFullPoolSize, mPoolSize, *mPoolLastUpdateStack;
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

public:
    // Combat functions
    bool CheckCombatStatus(Unit *unit = NULL);
    void ClearCombatTimers(WoWGuid guid, WoWGuid guid2 = WoWGuid());
    void TriggerCombatTimer(WoWGuid guid, WoWGuid guid2, uint32 timer);

private:
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

    // Spell targetting functions
    void HandleSpellTargetMapping(MapTargetCallback *callback, SpellTargetClass *spell, uint32 i, uint32 targetType, float x, float y, float z, float minRange, float maxRange, uint32 typeMask = 0);

protected:
    // These are stored in MapInstance.cpp with functions
    friend class MapInstanceObjectProcessCallback;
    MapInstanceObjectProcessCallback _processCallback;

    friend class MapInstanceObjectRemovalCallback;
    MapInstanceObjectRemovalCallback _removalCallback;

    std::vector<uint32> _InRangeTargetCellVector;
    friend class MapInstanceInRangeTargetCallback;
    MapInstanceInRangeTargetCallback _inRangeTargetCallback;

    std::vector<uint32> _BroadcastMessageCellVector;
    friend class MapInstanceBroadcastMessageCallback;
    MapInstanceBroadcastMessageCallback _broadcastMessageCallback;

    std::vector<uint32> _BroadcastMessageInRangeCellVector;
    friend class MapInstanceBroadcastMessageInrangeCallback;
    MapInstanceBroadcastMessageInrangeCallback _broadcastMessageInRangeCallback;

    std::vector<uint32> _BroadcastChatPacketCellVector;
    friend class MapInstanceBroadcastChatPacketCallback;
    MapInstanceBroadcastChatPacketCallback _broadcastChatPacketCallback;

    std::vector<uint32> _BroadcastObjectUpdateCellVector;
    friend class MapInstanceBroadcastObjectUpdateCallback;
    MapInstanceBroadcastObjectUpdateCallback _broadcastObjectUpdateCallback;

    std::vector<uint32> _DynamicObjectTargetMappingCellVector;
    friend class MapInstanceDynamicObjectTargetMappingCallback;
    MapInstanceDynamicObjectTargetMappingCallback _DynamicObjectTargetMappingCallback;

    std::vector<uint32> _SpellTargetMappingCellVector;
    friend class MapInstanceSpellTargetMappingCallback;
    MapInstanceSpellTargetMappingCallback _SpellTargetMappingCallback;

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
    void _PerformPendingRemovals();

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

    bool HasActivatedCondition(uint32 conditionId, WorldObject *obj);

protected:
    /* Map Information */
    MapEntry* pdbcMap;

    /* Update System */
    Mutex m_updateMutex;
    ObjectSet _updates, _movedObjects, _pendingRemoval, _pendingCleanup;
    PlayerSet _processQueue, _movedPlayers, _softDCPlayers;

    /* Sessions */
    SessionSet MapSessions;

    // Map preloading to push back updating inrange objects
    bool m_mapPreloading;

public:
    Mutex m_poolLock;
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
    std::map<WoWGuid, uint32> m_objectCells;

    void CacheObjectCell(WoWGuid guid, uint32 cellId) { m_objectCells[guid] = cellId; }
    void RemoveCachedCell(WoWGuid guid) { m_objectCells.erase(guid); }

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
    struct MapInstanceData
    {
        uint32 difficulty;
        uint32 linkedGroupId;
        uint32 encounterMask;
    } *m_instanceData;
};
