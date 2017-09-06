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
// MapCell.h
//

#pragma once

class Map;

#define MAKE_CELL_EVENT(x,y) ( ((x) * 1000) + 200 + y )
#define DECODE_CELL_EVENT(dest_x, dest_y, ev) (dest_x) = ((ev-200)/1000); (dest_y) = ((ev-200)%1000);

class MapCellObjectStorage;

class ObjectProcessCallback { public: virtual void operator()(WorldObject *obj, WorldObject *curObj) = 0; };

typedef Loki::AssocVector<WoWGuid, WorldObject*> CellObjectMap;

struct PhaseObjectStack
{
    // Object type sets
    CellObjectMap gameObjectSet, creatureSet;
};

class SERVER_DECL MapCell
{
    friend class CellHandler<MapCell>;
public:
    MapCell();
    ~MapCell();

    //Init
    void Init(uint32 x, uint32 y, uint32 mapid, MapInstance* instance);

    //WorldObject Managing
    void AddObject(WorldObject* obj);
    void RemoveObject(WorldObject* obj);
    void ProcessPendingActions();

    void ReactivateObject(WorldObject *obj);
    void DeactivateObject(WorldObject *obj);

    WorldObject *FindObject(WoWGuid guid, bool searchDeactivated = false);

    RONIN_INLINE bool HasPlayers() { return !m_activePlayerSet.empty(); }

    // Iterating through different phases of sets
    void ProcessObjectSets(WorldObject *obj, ObjectProcessCallback *callback, const std::vector<uint16> *phaseSet, uint32 objectMask = 0);

    //State Related
    void SetActivity(bool state);

    RONIN_INLINE bool IsActive() { return _active; }
    RONIN_INLINE bool IsLoaded() { return _loaded; }

    //WorldObject Loading Managing
    uint32 LoadCellData(CellSpawns * sp);
    void UnloadCellData(bool preDestruction);

    RONIN_INLINE bool IsUnloadPending() { return _unloadpending; }
    RONIN_INLINE void SetUnloadPending(bool up) { _unloadpending = up; }
    void QueueUnloadPending();
    void CancelPendingUnload();
    void Unload();

    RONIN_INLINE void SetPermanentActivity(bool val) { _forcedActive = val; }
    RONIN_INLINE bool IsForcedActive() { return _forcedActive; }

    RONIN_INLINE uint16 GetPositionX() { return _x; }
    RONIN_INLINE uint16 GetPositionY() { return _y; }

private:
    bool _forcedActive;
    uint16 _x,_y;

    bool _loaded;
    std::atomic<bool> _active, _unloadpending;

    MapInstance* _instance;
    Map *_mapData;

    // Deferred object removal
    Mutex _pendingLock;
    std::set<WoWGuid> m_pendingRemovals, m_pendingReactivate, m_pendingDeactivate;

    // Object storage/locks
    RWMutex _objLock;
    // Non player set and player set
    CellObjectMap m_activeNonPlayerSet, m_activePlayerSet;
    // Deactivated objects
    CellObjectMap m_deactivatedObjects;
    // Phase mask maps
    Loki::AssocVector<uint16, PhaseObjectStack> m_objectStacks;

    // Used for instance based guid recalculation
    Loki::AssocVector<WoWGuid, WoWGuid> m_sqlIdToGuid;

    std::vector<uint16> m_objAddPhaseVect;

#if STACKED_MEMORY_ALLOCATION == 1
    // Allocation stacks
    Creature *_creatureStack;
    GameObject *_gameobjectStack;
    std::map<uint32, WoWGuid> m_ctrIndex, m_gobjIndex;
#else
    // Spawned objects
    std::vector<Creature*> m_spawnedCreatures;
    std::vector<GameObject*> m_spawnedGameObjects;
#endif
};
