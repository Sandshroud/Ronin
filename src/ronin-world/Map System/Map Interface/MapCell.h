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

class ObjectProcessCallback
{
public:
    virtual void operator()(WorldObject *obj, WorldObject *curObj) = 0;

    void Lock() { processLock.Acquire(); }
    void Unlock() { processLock.Release(); }

private:
    Mutex processLock;
};

class SERVER_DECL MapCell
{
    friend class CellHandler<MapCell>;
public:
    MapCell();
    ~MapCell();

    typedef Loki::AssocVector<WoWGuid, WorldObject*> CellObjectMap;

    //Init
    void Init(uint32 x, uint32 y, uint32 mapid, MapInstance* instance);

    //WorldObject Managing
    void AddObject(WorldObject* obj);
    void RemoveObject(WorldObject* obj);

    void ReactivateObject(WorldObject *obj);
    void DeactivateObject(WorldObject *obj);

    WorldObject *FindObject(WoWGuid guid);

    bool HasPlayers() { return !m_playerSet.empty(); }

    // Iterating through different phases of sets
    void ProcessObjectSets(WorldObject *obj, ObjectProcessCallback *callback, uint32 objectMask = 0);

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

    void SetPermanentActivity(bool val) { _forcedActive = val; }
    bool IsForcedActive() { return _forcedActive; }

    uint16 GetPositionX() { return _x; }
    uint16 GetPositionY() { return _y; }

private:
    bool _forcedActive;
    uint16 _x,_y;

    bool _active, _loaded;
    bool _unloadpending;

    uint32 objectCount;
    MapInstance* _instance;
    Mutex cellLock;
    Map *_mapData;

    // Non player set and player set
    CellObjectMap m_nonPlayerSet, m_playerSet;
    // Object type sets
    CellObjectMap m_gameObjectSet, m_creatureSet;
    // Deactivated objects
    CellObjectMap m_deactivatedObjects;

    // Used for instance based guid recalculation
    Loki::AssocVector<WoWGuid, WoWGuid> m_sqlIdToGuid;
};
