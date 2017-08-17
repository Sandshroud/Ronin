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

#ifndef DEBUG_CONTINENT_PERF
#define DEBUG_CONTINENT_PERF 0
#endif

#if DEBUG_CONTINENT_PERF == 1
#include <chrono>
namespace cClock = std::chrono;
#endif

bool ContinentManager::run()
{
    DWORD affinityMask = sWorld.GetCoreAffinity(m_mapId, NULL);
    if(affinityMask && SetThreadAffinityMask(sThreadManager.GetSecurityHandle(RONIN_UTIL::GetThreadId()), affinityMask) == 0)
        sLog.Error("ThreadManager", "Failed to assign continent manager %u to affinity mask %llu", m_mapId, affinityMask);

    // Preload all needed spawns etc
    m_continent->Preload();
    // Wait for our thread to be activated
    while(GetThreadState() == THREADSTATE_PAUSED)
        Delay(50);

    sWorldMgr.MapLoaded(m_mapId);
    FILE *file = NULL;
#if DEBUG_CONTINENT_PERF == 1
    if(fopen_s(&file, format("MAP_%03u_Perf.log", m_mapId).c_str(), "w") == 0)
        fclose(file);
#endif

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
        int32 diff = std::min<uint32>(500, mstime - lastUpdate);
        lastUpdate = mstime;

        // Update our collision system via singular map system
        sVMapInterface.UpdateSingleMap(m_mapId, diff);

#if DEBUG_CONTINENT_PERF == 1
        auto currentTime = cClock::system_clock::now();
#endif
        // Process all pending actions in sequence
        m_continent->_PerformPendingActions();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performPendingRemovals = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Process all pending inputs in sequence
        m_continent->_ProcessInputQueue();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 processInputQueue = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Process all script updates before object updates
        m_continent->_PerformScriptUpdates(mstime, diff);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performScriptUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all combat state updates before any unit updates
        m_continent->_PerformCombatUpdates(mstime, diff);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performCombatUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all delayed spell updates before object updates
        m_continent->_PerformDelayedSpellUpdates(mstime, diff);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performDelayedSpellUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all unit path updates in sequence
        m_continent->_PerformUnitPathUpdates(mstime, diff);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performUnitPathUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all player updates in sequence
        m_continent->_PerformPlayerUpdates(mstime, diff);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performPlayerUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all dynamic object updates in sequence
        m_continent->_PerformDynamicObjectUpdates(mstime, diff);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performDynamicObjectUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all creature updates in sequence
        m_continent->_PerformCreatureUpdates(mstime, diff);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performCreatureUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all object updates in sequence
        m_continent->_PerformObjectUpdates(mstime, diff);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performObjectUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all movement updates in sequence without player data
        m_continent->_PerformMovementUpdates(false);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performMovementUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all session updates in sequence
        m_continent->_PerformSessionUpdates();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performSessionUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all movement updates in sequence with player data
        m_continent->_PerformMovementUpdates(true);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performPlayerMovementUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Process secondary pending actions in sequence
        m_continent->_PerformPendingActions();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performLatePendingRemovals = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();
        currentTime = cClock::system_clock::now();
#endif
        // Perform all pending object updates in sequence
        m_continent->_PerformPendingUpdates();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;
#if DEBUG_CONTINENT_PERF == 1
        uint32 performPendingUpdates = cClock::duration_cast<cClock::milliseconds>(cClock::system_clock::now() - currentTime).count();

        if(fopen_s(&file, format("MAP_%03u_Perf.log", m_mapId).c_str(), "a") == 0)
        {
            fprintf(file, "[%04u] %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n", (performPendingRemovals + processInputQueue + performScriptUpdates
                + performCombatUpdates + performDelayedSpellUpdates + performUnitPathUpdates + performPlayerUpdates + performDynamicObjectUpdates
                + performCreatureUpdates + performObjectUpdates + performMovementUpdates + performSessionUpdates + performPlayerMovementUpdates + performLatePendingRemovals + performPendingUpdates),
                performPendingRemovals, processInputQueue, performScriptUpdates, performCombatUpdates, performDelayedSpellUpdates, performUnitPathUpdates, performPlayerUpdates, performDynamicObjectUpdates,
                performCreatureUpdates, performObjectUpdates, performMovementUpdates, performSessionUpdates, performPlayerMovementUpdates, performLatePendingRemovals, performPendingUpdates);
            fclose(file);
        }
#endif

        // Set the thread to sleep to prevent thread overrun and wasted cycles
        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;

        Delay(std::max<int32>(5, MapInstanceUpdatePeriod-(getMSTime()-lastUpdate)));
        counter++;
    }while(true);
    sLog.Notice("ContinentManager", "Cleaning up continent %u (%s)", m_mapId, m_mapData->GetName());

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
