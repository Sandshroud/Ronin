/***
 * Demonstrike Core
 */

//
// WorldCreator.cpp
//

#include "StdAfx.h"

#define MAP_MGR_UPDATE_PERIOD 50

ContinentManager::ContinentManager(MapEntry *mapEntry, Map *map) : ThreadContext(), m_mapEntry(mapEntry), m_mapId(mapEntry->MapID), m_mapData(map), eventHolder(m_mapId), m_continent(NULL)
{
    SetThreadState(THREADSTATE_PAUSED);
}

ContinentManager::~ContinentManager()
{

}

bool ContinentManager::Initialize()
{
    sLog.Notice("ContinentManager", "Creating continent %u(%s).", m_mapId, m_mapEntry->name);
    if(m_continent = new MapInstance(m_mapData, m_mapId, 0))
    {
        if(sWorld.ServerPreloading >= 2)
            m_continent->UpdateAllCells(true);
        return true;
    }
    return false;
}

bool ContinentManager::run()
{
    while(GetThreadState() == THREADSTATE_PAUSED)
        Delay(50);

    m_continent->Init();

    uint32 counter = 0, mstime = getMSTime(), lastUpdate = mstime; // Get our ms time
    do
    {
        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        mstime = getMSTime();
        int32 diff = std::min<uint32>(500, mstime - lastUpdate);
        lastUpdate = mstime;

        // Update our collision system via singular map system
        sVMapInterface.UpdateSingleMap(m_mapId, diff);

        // Update any events.
        eventHolder.Update(diff);

        // Process all pending inputs in sequence
        m_continent->_ProcessInputQueue();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        // Perform all player updates in sequence
        m_continent->_PerformPlayerUpdates(mstime, diff);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        // Perform all creature updates in sequence
        m_continent->_PerformCreatureUpdates(mstime);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        // Perform all object updates in sequence
        m_continent->_PerformObjectUpdates(mstime);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        // Perform all session updates in sequence
        m_continent->_PerformSessionUpdates();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
        // Perform all pending object updates in sequence
        m_continent->_PerformPendingUpdates();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        // Set the thread to sleep to prevent thread overrun and wasted cycles
        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;

        diff = MAP_MGR_UPDATE_PERIOD-(getMSTime()-lastUpdate);
        if(diff < -50) printf("Map %03u overrunning by %i\n", m_mapId, abs(diff));
        Delay(std::max<int32>(5, diff));
        counter++;
    }while(true);

    sLog.Notice("ContinentManager", "Cleaning up continent %u (%s)", m_mapId, m_mapData->GetName());
    m_continent->Destruct();
    m_continent = NULL;

    // Unload all terrain
    if(sWorld.ServerPreloading)
        m_mapData->UnloadAllTerrain(true);
    delete m_mapData;

    sLog.Debug("MapInstance", "Map %u shut down. (%s)", m_mapId, m_mapData->GetName());
    return true;
}
