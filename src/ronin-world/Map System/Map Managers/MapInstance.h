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
class MapManager;
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

enum ObjectActiveState
{
    OBJECT_STATE_NONE    = 0,
    OBJECT_STATE_INACTIVE = 1,
    OBJECT_STATE_ACTIVE   = 2,
};

static const uint32 MaxViewDistance = 38000;

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
    StoragePool() : mPoolCounter(0), mPoolAddCounter(0), mPoolSize(0) { mPoolStack = NULL; mPoolLastUpdateStack = NULL; }

    void Initialize(uint32 poolSize)
    {
        mPoolSize = poolSize;
        poolIterator = mPool.end();
        mPoolCounter = mPoolAddCounter = 0;
        if(mPoolSize == 1) // Don't initialize the single stack
            return;
        mPoolLastUpdateStack = new uint32[mPoolSize];
        mPoolStack = new PoolSet[mPoolSize];
    }

    void Cleanup()
    {
        if(mPoolLastUpdateStack)
            delete [] mPoolLastUpdateStack;
        mPoolLastUpdateStack = NULL;
        if(mPoolStack)
            delete [] mPoolStack;
        mPoolStack = NULL;
    }

    // Update our object stack, this includes inactivity timers
    void Update(uint32 msTime, uint32 pDiff)
    {
        T* ptr = NULL;
        uint32 diff = pDiff;
        PoolSet *targetPool;
        if(mPoolStack == NULL) // No stack so use our main pool
            targetPool = &mPool;
        else
        {
            // Select our next pool to update in the sequence
            if(++mPoolCounter == mPoolSize)
                mPoolCounter = 0;
            // Recalculate the diff from the last time we updated this pool
            diff = msTime - mPoolLastUpdateStack[mPoolCounter];
            mPoolLastUpdateStack[mPoolCounter] = msTime;
            // Set the target pool pointer
            targetPool = &mPoolStack[mPoolCounter];
        }

        if(!targetPool->empty())
        {
            poolIterator = targetPool->begin();
            while(poolIterator != targetPool->end())
            {
                ptr = *poolIterator;
                ++poolIterator;

                if(ptr->IsActiveObject() && !ptr->IsActivated())
                    ptr->InactiveUpdate(msTime, diff);
                else ptr->Update(msTime, diff);
            }
        }
    }

    // Resets the stack timers
    void ResetTime(uint32 msTime)
    {
        if(mPoolStack == NULL)
            return;

        // Times have to be reset for our pools so we don't have massive differences from currentms-0
        for(uint32 i = 0; i < mPoolSize; i++)
            mPoolLastUpdateStack[i] = msTime;
    }

    // Add our object to the stack, return value is a pool identifier
    uint8 Add(T *obj)
    {
        uint8 pool = 0xFF;
        POOL_ADD(mPool, obj);
        if(mPoolStack)
        {
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
            if( poolIterator == itr )
                poolIterator = mPool.erase(itr);
            else mPool.erase(itr);
        }

        if(mPoolStack && poolId != 0xFF)
        {
            if((POOL_FIND(itr, mPool, obj)) != mPoolStack[poolId].end())
            {
                // check iterator
                if( poolIterator == itr )
                    poolIterator = mPoolStack[poolId].erase(itr);
                else mPoolStack[poolId].erase(itr);
            }
        }
    }

    typename PoolSet::iterator begin() { return mPool.begin(); };
    typename PoolSet::iterator end() { return mPool.end(); };

private:

    PoolSet mPool, *mPoolStack;
    typename PoolSet::iterator poolIterator;
    uint32 mPoolCounter, mPoolAddCounter, mPoolSize, *mPoolLastUpdateStack;
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

    //This will be done in regular way soon
    Mutex m_objectinsertlock;
    ObjectSet m_objectinsertpool;
    void AddObject(WorldObject*);
    WorldObject* GetObjectClosestToCoords(uint32 entry, float x, float y, float z, float ClosestDist, int32 forcedtype = -1);

    bool IsClosing() { return false; }
    bool IsFull() { return false; }

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
    Summon* CreateSummon(uint32 entry);

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
    DynamicObject* CreateDynamicObject();

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

//////////////////////////////////////////////////////////
// Local (MapInstance) storage of combats in progress
////////////////////////////////
    CombatProgressSet _combatProgress;
    RONIN_INLINE void AddCombatInProgress(WoWGuid guid) { _combatProgress.insert(guid); }
    RONIN_INLINE void RemoveCombatInProgress(WoWGuid guid) { _combatProgress.erase(guid); }
    RONIN_INLINE bool IsCombatInProgress()
    {
        // if all players are out, list should be empty.
        if(HasPlayers() == false)
            return false;
        return !_combatProgress.empty();
    }

//////////////////////////////////////////////////////////
// Lookup Wrappers
///////////////////////////////////
    Unit* GetUnit(WoWGuid guid);
    WorldObject* _GetObject(WoWGuid guid);

//////////////////////////////////////////////////////////
// Map initializers and functions
///////////////////////////////////
    MapInstance(Map *map, uint32 mapid, uint32 instanceid);
    ~MapInstance();

    void Preload();
    void Init(uint32 msTime);
    void Destruct();

    void EventPushObjectToSelf(WorldObject *obj);

    void PushObject(WorldObject* obj);
    void RemoveObject(WorldObject* obj);

    virtual void ChangeObjectLocation(WorldObject* obj); // update inrange lists
    void ChangeFarsightLocation(Player* plr, Unit* farsight, bool apply);
    void ChangeFarsightLocation(Player* plr, float X, float Y, bool apply);
    bool IsInRange(float fRange, WorldObject* obj, WorldObject* currentobj, float &distOut);

    //! Mark object as updated
    bool UpdateQueued(WorldObject *obj);
    void ObjectUpdated(WorldObject *obj);
    void ObjectLocationChange(WorldObject *obj);
    void UpdateCellActivity(uint32 x, uint32 y, int radius);

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

    RONIN_INLINE virtual bool IsInstance() { return pdbcMap && (pdbcMap->IsDungeon() || pdbcMap->IsRaid()); }
    RONIN_INLINE uint32 GetInstanceID() { return m_instanceID; }
    RONIN_INLINE MapEntry *GetdbcMap() { return pdbcMap; }
    bool CanUseCollision(WorldObject* obj);

    virtual int32 event_GetMapID() { return _mapId; }

    void UpdateAllCells(bool apply, uint32 areamask = 0);
    RONIN_INLINE size_t GetPlayerCount() { return m_PlayerStorage.size(); }

    void _ProcessInputQueue();
    void _PerformPlayerUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformCreatureUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformObjectUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformDynamicObjectUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformDelayedSpellUpdates(uint32 msTime, uint32 uiDiff);
    void _PerformSessionUpdates();
    void _PerformPendingUpdates();

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
    void SendChatMessageToCellPlayers(WorldObject* obj, WorldPacket * packet, uint32 cell_radius, uint32 langpos, uint32 guidPos, int32 lang, WorldSession * originator);
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

protected:
    //! Collect and send updates to clients
    void _UpdateObjects();

    //! Objects that exist on map
    uint32 _mapId;

    // Storage for processing inrange updates
    WorldObject::InRangeObjSet m_inRangeStorage;

    // In this zone, we always show these objects
    Loki::AssocVector<WorldObject*, uint32> m_zoneFullRangeObjects, m_areaFullRangeObjects;
    Loki::AssocVector<uint32, std::vector<WorldObject*>> m_fullRangeObjectsByZone, m_fullRangeObjectsByArea;

    bool _CellActive(uint32 x, uint32 y);
    void UpdateInRangeSet(WorldObject* obj, Player* plObj, MapCell* cell, bool playerOnly);

    bool ObjectMovingCells(WorldObject *obj, MapCell *oldCell, MapCell *newCell);
    void UpdateObjectVisibility(Player *plObj, WorldObject *curObj);

public:
    void UpdateInrangeSetOnCells(WorldObject* obj, uint32 startX, uint32 endX, uint32 startY, uint32 endY, uint32 minX, uint32 maxX, uint32 minY, uint32 maxY);

    bool IsPreloading() { return m_mapPreloading; }
    bool IsRaid() { return pdbcMap ? pdbcMap->IsRaid() : false; }
    bool IsContinent() { return pdbcMap ? pdbcMap->IsContinent() : true; }
protected:
    /* Map Information */
    MapEntry* pdbcMap;
    uint32 m_instanceID;

    /* Update System */
    Mutex m_updateMutex;
    ObjectSet _updates, _movedObjects;
    PlayerSet _processQueue;

    /* Sessions */
    SessionSet MapSessions;

    // Map preloading to push back updating inrange objects
    bool m_mapPreloading;

public:
    Mutex m_poolLock;
    StoragePool<Creature> mCreaturePool;
    StoragePool<GameObject> mGameObjectPool;
    StoragePool<DynamicObject> mDynamicObjectPool;

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

    // stored iterators for safe checking
    PlayerStorageMap::iterator __player_iterator;
};
