/***
 * Demonstrike Core
 */

//
// WorldCreator.cpp
//

#include "StdAfx.h"

#define MAP_MGR_UPDATE_PERIOD 50

MapManager::MapManager(MapEntry *mapEntry, Map *map) : ThreadContext(), m_mapEntry(mapEntry), m_mapId(mapEntry->MapID), m_mapData(map), eventHolder(m_mapId), m_continent(NULL)
{
    SetThreadState(THREADSTATE_PAUSED);
}

MapManager::~MapManager()
{

}

bool MapManager::Initialize()
{
    if(m_mapEntry->IsContinent())
    {
        sLog.Notice("MapManager", "Creating continent %u(%s).", m_mapId, m_mapEntry->name);
        m_continent = new MapInstance(m_mapData, m_mapId, m_mapId);
        if(sWorld.ServerPreloading >= 2)
            m_continent->UpdateAllCells(true);
    }

    return true;
}

bool MapManager::run()
{
    while(GetThreadState() == THREADSTATE_PAUSED)
        Delay(50);

    uint32 counter = 0, mstime = getMSTime(), lastUpdate = mstime; // Get our ms time
    do
    {
        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        mstime = getMSTime();
        uint32 diff = std::min<uint32>(500, mstime - lastUpdate);
        lastUpdate = mstime;

        // Update our collision system via singular map system
        sVMapInterface.UpdateSingleMap(m_mapId, diff);

        // Update any events.
        eventHolder.Update(diff);

        std::vector<MapInstance*> m_mapsToUpdate;
        // Push our continent(if we have one) to the processing map
        if(MapInstance *continent = m_continent)
            m_mapsToUpdate.push_back(continent);
        // Push all our instances to the processing map
        for(std::map<uint32, MapInstance*>::iterator itr = m_mapInstances.begin(); itr != m_mapInstances.end(); itr++)
            m_mapsToUpdate.push_back(itr->second);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        // Process all pending inputs in sequence
        for(std::vector<MapInstance*>::iterator itr = m_mapsToUpdate.begin(); itr != m_mapsToUpdate.end(); itr++)
            (*itr)->_ProcessInputQueue();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        uint32 diff1 = getMSTime()-mstime;
        // Perform all player updates in sequence
        for(std::vector<MapInstance*>::iterator itr = m_mapsToUpdate.begin(); itr != m_mapsToUpdate.end(); itr++)
            (*itr)->_PerformPlayerUpdates(diff);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        uint32 diff2 = getMSTime()-mstime;
        // Perform all creature updates in sequence
        for(std::vector<MapInstance*>::iterator itr = m_mapsToUpdate.begin(); itr != m_mapsToUpdate.end(); itr++)
            (*itr)->_PerformCreatureUpdates(mstime);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        uint32 diff3 = getMSTime()-mstime;
        // Perform all object updates in sequence
        for(std::vector<MapInstance*>::iterator itr = m_mapsToUpdate.begin(); itr != m_mapsToUpdate.end(); itr++)
            (*itr)->_PerformObjectUpdates(mstime);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        uint32 diff4 = getMSTime()-mstime;
        // Perform all session updates in sequence
        for(std::vector<MapInstance*>::iterator itr = m_mapsToUpdate.begin(); itr != m_mapsToUpdate.end(); itr++)
            (*itr)->_PerformSessionUpdates();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        uint32 diff5 = getMSTime()-mstime;
        // Perform all pending object updates in sequence
        for(std::vector<MapInstance*>::iterator itr = m_mapsToUpdate.begin(); itr != m_mapsToUpdate.end(); itr++)
            (*itr)->_PerformPendingUpdates();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        uint32 diff6 = getMSTime()-mstime;

        mstime = getMSTime();
        // If we have no maps(only instances can have no maps) to update then we just idle till we do
        if(m_mapsToUpdate.empty())
        {
            while(m_mapInstances.empty() == true)
            {
                if(!SetThreadState(THREADSTATE_SLEEPING))
                    break;
                Delay(MAP_MGR_UPDATE_PERIOD*10);
            }
        }
        else
        {
            // Normal processing, we set the thread to sleep to prevent thread overrun and wasted cycles
            if(!SetThreadState(THREADSTATE_SLEEPING))
                break;

            diff = mstime-lastUpdate;
            //if(diff > 5) printf("Map %u Update: %u | %u %u %u %u %u %u %u\n", m_mapId, counter, diff, diff1, diff2, diff3, diff4, diff5, diff6);
            if(diff<MAP_MGR_UPDATE_PERIOD)
                Delay(MAP_MGR_UPDATE_PERIOD-diff);
            else sLog.outDebug("Map %u running outside of update period", m_mapId);
        }
        counter++;
    }while(true);

    if(m_continent)
    {
        sLog.Notice("MapManager", "Cleaning up continent %u (%s)", m_mapId, m_mapData->GetName());
        m_continent->Destruct();
        m_continent = NULL;
    }
    // Push all our instances to the processing map
    for(std::map<uint32, MapInstance*>::iterator itr = m_mapInstances.begin(); itr != m_mapInstances.end(); itr++)
        itr->second->Destruct();
    m_mapInstances.clear();

    sLog.Debug("MapInstance", "Map %u shut down. (%s)", m_mapId, m_mapData->GetName());
    return true;
}

MapInstance *MapManager::CreateMapInstance(uint32 instanceId)
{
    MapInstance* ret = new MapInstance(m_mapData, m_mapId, instanceId);
    m_instanceLocks.Acquire();
    m_mapInstances.insert(std::make_pair(instanceId, ret));
    m_instanceLocks.Release();
    return ret;
}
