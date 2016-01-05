/***
 * Demonstrike Core
 */

//
// MapManager.h
//

#pragma once

class SERVER_DECL MapManager : public ThreadContext
{
    friend class MapInstance;
public:
    MapManager(MapEntry *mapEntry, Map *map);
    ~MapManager();

    bool Initialize();

    bool run();

    MapInstance *CreateMapInstance(uint32 instanceId);

    MapInstance *GetContinent() { return m_continent; }
    MapInstance *GetMapInstance(uint32 instanceId)
    {
        MapInstance *instance = NULL;
        m_instanceLocks.Acquire();
        if(m_mapInstances.find(instanceId) != m_mapInstances.end())
            instance = m_mapInstances.at(instanceId);
        m_instanceLocks.Release();
        return instance;
    }

    // Release without destroying instance
    void ReleaseInstance(uint32 instanceId)
    {
        m_instanceLocks.Acquire();
        m_mapInstances.erase(instanceId);
        m_instanceLocks.Release();
    }

private:
    uint32 m_mapId;
    MapEntry *m_mapEntry;
    Map *m_mapData;

    MapInstance *m_continent;

    Mutex m_instanceLocks;
    std::map<uint32, MapInstance*> m_mapInstances;

    // Process events in the manager not individual maps
    EventableObjectHolder eventHolder;
};
