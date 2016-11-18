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

class MapCellObjectStorage;

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
    bool HasPlayers(uint16 phaseMask = 0xFFFF);

    // Iterating through different phases of sets
    MapCell::CellObjectSet *GetNextObjectSet(uint16 &phaseMask, std::vector<uint32> &conditionAccess, std::vector<uint32> &eventAccess, bool &handledAllPhases);
    void FillObjectSets(std::set<WorldObject*> &set, uint16 phaseMask, std::vector<uint32> conditionAccess, std::vector<uint32> eventAccess, bool playersOnly);

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
    Map *_mapData;

    MapCell::CellObjectSet m_objectSet, m_playerSet;
    Loki::AssocVector<uint8, MapCellObjectStorage*> m_phaseStorage, m_eventStorage, m_conditionStorage;
};

class MapCellObjectStorage
{
    MapCell::CellObjectSet m_objectSet;
public:
    MapCellObjectStorage(uint32 identifier) : m_identifier(identifier) {}
    ~MapCellObjectStorage() { m_objectSet.clear(); }

    void AddObject(WorldObject *obj);
    void RemoveObject(WorldObject *obj);

    MapCell::CellObjectSet *GetObjectSet() { return &m_objectSet; }
    void UnloadCellData(bool pendingUnload, bool preDestruction);

    bool isEmpty() { return m_objectSet.empty(); }
    MapCell::CellObjectSet::iterator begin() { return m_objectSet.begin(); }
    MapCell::CellObjectSet::iterator end() { return m_objectSet.end(); }
    RONIN_INLINE uint32 GetObjectCount() { return m_objectSet.size(); }

    uint8 GetIdentifier() { return m_identifier; }

private:
    uint8 m_identifier;
};
