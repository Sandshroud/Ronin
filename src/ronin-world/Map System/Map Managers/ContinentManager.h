/***
 * Demonstrike Core
 */

//
// MapManager.h
//

#pragma once

class SERVER_DECL ContinentManager : public ThreadContext
{
    friend class MapInstance;
public:
    ContinentManager(MapEntry *mapEntry, Map *map);
    ~ContinentManager();

    bool Initialize();

    bool run();

    Map *GetMapData() { return m_mapData; }
    MapInstance *GetContinent() { return m_continent; }

private:
    uint32 m_mapId;

    // Map data for terrain, datacells etc
    Map *m_mapData;

    // Map Entry data pointer
    MapEntry *m_mapEntry;

    // Our continent instance
    MapInstance *m_continent;
};
