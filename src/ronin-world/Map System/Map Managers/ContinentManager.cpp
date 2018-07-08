/*
 * Sandshroud Project Ronin
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
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

#include "StdAfx.h"

ContinentManager::ContinentManager(MapEntry *mapEntry, Map *map) : ThreadContext(), m_mapEntry(mapEntry), m_mapId(mapEntry->MapID), m_mapData(map)
{
    SetThreadState(THREADSTATE_PAUSED);
}

ContinentManager::~ContinentManager()
{
    m_mapData = NULL;
}

bool ContinentManager::Initialize()
{
    sLog.Notice("ContinentManager", "Creating continent %u(%s).", m_mapId, m_mapEntry->name);
    if(m_continent = new MapInstance(m_mapData, m_mapId, 0))
        return true;
    return false;
}

bool ContinentManager::run()
{
    DWORD affinityMask = sWorld.GetCoreAffinity(m_mapId, NULL);
    if(affinityMask && SetThreadAffinityMask(sThreadManager.GetSecurityHandle(RONIN_UTIL::GetThreadId()), affinityMask) == 0)
        sLog.Error("ThreadManager", "Failed to assign continent manager %u to affinity mask %llu", m_mapId, affinityMask);

    // Preload all needed spawns etc
    m_continent->Preload();
    // Wait for our thread to be activated
    DelayIfPaused();

    sWorldMgr.MapLoaded(m_mapId);
    FILE *file = NULL;

    // Pull any characters in pending login state
    sWorld.ProcessPendingCharacters(m_mapId);

    // Initialize the base continent timers
    uint32 mstime = getMSTime();
    m_continent->Init(mstime);
    // Initialize our counter at 0 and our last update time for diff calculations
    uint32 counter = 0, lastUpdate = mstime;
    do
    {
        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        mstime = getMSTime();
        RONIN_UTIL::ThreadTimer::SetThreadTime(mstime);
        int32 diff = std::min<uint32>(500, mstime - lastUpdate);
        lastUpdate = mstime;

        // Update our collision system via singular map system
        sVMapInterface.UpdateSingleMap(m_mapId, diff);

        // Push our instance to be updated by the world management system
        if(!sWorld.ProcessMapInstanceUpdate(this, m_continent, mstime, diff))
            break;

        // Set the thread to sleep to prevent thread overrun and wasted cycles
        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;

        Delay(std::max<int32>(5, MapInstanceUpdatePeriod-(getMSTime()-lastUpdate)));
        counter++;
    }while(true);
    sLog.Notice("ContinentManager", "Cleaning up continent %u (%s)", m_mapId, m_mapData->GetName());

    // Wait for all task lists to exit
    m_continent->WaitForTaskPool();

    // Remove us from content map
    sWorldMgr.ContinentUnloaded(m_mapId);

    // Clean up continent map instance
    m_continent->Destruct();
    m_continent = NULL;

    // Unload all terrain
    if(sWorld.ServerPreloading >= 1)
        m_mapData->UnloadAllTerrain(true);
    sLog.Debug("MapInstance", "Map %u shut down. (%s)", m_mapId, m_mapData->GetName());
    return true;
}
