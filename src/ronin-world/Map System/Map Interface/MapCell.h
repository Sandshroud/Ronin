/***
 * Demonstrike Core
 */

//
// MapCell.h
//

#pragma once

class Map;

#define MAKE_CELL_EVENT(x,y) ( ((x) * 1000) + 200 + y )
#define DECODE_CELL_EVENT(dest_x, dest_y, ev) (dest_x) = ((ev-200)/1000); (dest_y) = ((ev-200)%1000);

class SERVER_DECL MapCell
{
    friend class CellHandler<MapCell>;
public:
    MapCell();
    ~MapCell();

    typedef std::vector<WorldObject*> CellObjectSet;

    //Init
    void Init(uint32 x, uint32 y, uint32 mapid, MapInstance* mapmgr);

    //WorldObject Managing
    void AddObject(WorldObject* obj);
    void RemoveObject(WorldObject* obj);
    void RemoveObjects(bool preDestruction = false);
    bool HasObject(WorldObject* obj) { return std::find(_objects.begin(), _objects.end(), obj) != _objects.end(); }
    bool HasPlayers() { return ((_playerCount > 0) ? true : false); }
    RONIN_INLINE size_t GetObjectCount() { return _objects.size(); }
    RONIN_INLINE CellObjectSet::iterator Begin() { return _objects.begin(); }
    RONIN_INLINE CellObjectSet::iterator End() { return _objects.end(); }

    void AddRespawn(WorldObject* obj);
    void RemoveRespawn(WorldObject* obj);
    bool EventRespawn(WorldObject *obj);

    //State Related
    void SetActivity(bool state);

    RONIN_INLINE bool IsActive() { return _active; }
    RONIN_INLINE bool IsLoaded() { return _loaded; }

    //WorldObject Loading Managing
    uint32 LoadObjects(CellSpawns * sp);
    RONIN_INLINE uint32 GetPlayerCount() { return _playerCount; }

    //ING Events
    void RemoveEventIdObjects(uint8 eventToRemove);
    void ModifyEventIdSetting(bool active, uint8 eventId);
    void LoadEventIdObjects(CellSpawns * sp, uint8 eventId);

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
    CellObjectSet _objects, _respawnObjects;
    bool _active, _loaded;
    bool _unloadpending;

    uint16 _playerCount;
    MapInstance* _mapmgr;
};
