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

SERVER_DECL InstanceManager sInstanceMgr;

InstanceManager::InstanceManager()
{

}

InstanceManager::~InstanceManager()
{

}

void InstanceManager::Destruct()
{

}

void InstanceManager::Launch()
{
    InstanceManagerSlave *worker = new InstanceManagerSlave();
    // Only need one instance management thread to start with, dynamically allocate more if we need them.
    ThreadPool.ExecuteTask(format("InstanceManager - Worker %u", 0+1).c_str(), worker);
}

void InstanceManager::Prepare()
{
    // Nullify these counters here
    m_creatureGUIDCounter = m_gameObjectGUIDCounter = 0;

    // Create all non-instance type maps.
    if( QueryResult *result = CharacterDatabase.Query( "SELECT MAX(id) FROM instances" ) )
    {
        m_instanceCounter = result->Fetch()[0].GetUInt32();
        delete result;
    } else m_instanceCounter = 0x3FF;
}

void InstanceManager::LaunchGroupFinderDungeon(uint32 mapId, GroupFinderMgr::GroupFinderDungeon *dungeon, Group *grp)
{
    Map *mapData = GetMapData(mapId);
    ASSERT(mapData != NULL && mapData->GetEntry()->IsDungeon());

    // Instance ID generation occurs inside mutex
    counterLock.Acquire();
    dungeon->instanceId = ++m_instanceCounter;
    counterLock.Release();

    MapInstance *instance = new MapInstance(mapData, mapId, dungeon->instanceId);
    _AddInstance(dungeon->instanceId, instance);

    LocationVector destination(dungeon->dataEntry->x, dungeon->dataEntry->y, dungeon->dataEntry->z, dungeon->dataEntry->o);
    for(uint32 i = 0; i < grp->GetSubGroupCount(); ++i)
    {
        SubGroup *sub = grp->GetSubGroup(i);
        if(sub == NULL)
            continue;

        for(auto itr = sub->GetGroupMembersBegin(); itr != sub->GetGroupMembersEnd(); ++itr)
            (*itr)->m_loggedInPlayer->SafeTeleport(instance, destination);
    }
}

MapInstance *InstanceManager::GetInstanceForObject(WorldObject *obj)
{
    Map *mapData = NULL;
    MapInstance *ret = NULL;
    uint32 mapId = obj->GetMapId(), instanceId = obj->GetInstanceID();
    // If we're a player without a group, we can't load into raids so just return here to prevent loading an instance we won't use
    if((mapData = GetMapData(mapId)) && obj->IsPlayer() && mapData->GetEntry()->IsRaid() && castPtr<Player>(obj)->GetGroup() == NULL)
        return ret;

    instanceStorageLock.Acquire();
    if(instanceId != 0)
    {
        // Check to see if we have the instance ID in storage
        if(mInstanceStorage.find(instanceId) != mInstanceStorage.end())
            ret = mInstanceStorage.at(instanceId).second;
        else ret = _LoadInstance(mapId, instanceId);
        // If not, then we can return nothing
    }
    else if(obj->IsPlayer() && mapData)
    {
        Player *plr = castPtr<Player>(obj);
        if(instanceId = castPtr<Player>(obj)->GetLinkedInstanceID(mapData->GetEntry()))
        {
            if(mInstanceStorage.find(instanceId) != mInstanceStorage.end())
                ret = mInstanceStorage.at(instanceId).second;
            else ret = _LoadInstance(mapId, instanceId);
        }
        else if(plr->CanCreateNewDungeon(mapId))
        {
            // Instance ID generation occurs inside mutex
            counterLock.Acquire();
            uint32 instanceId = ++m_instanceCounter;
            counterLock.Release();

            ret = new MapInstance(mapData, mapId, instanceId);
            _AddInstance(instanceId, ret);
        }
    }

    instanceStorageLock.Release();
    return ret;
}

uint32 InstanceManager::PreTeleportInstanceCheck(uint64 guid, uint32 mapId, uint32 instanceId, bool canCreate)
{
    Map *mapData = NULL;
    uint32 ret = INSTANCE_OK;
    instanceStorageLock.Acquire();
    if(instanceId != 0)
    {
        // Instance exists, run checks before we return the OK
        if(mInstanceStorage.find(instanceId) != mInstanceStorage.end())
        {
            MapInstance *instance = mInstanceStorage.at(instanceId).second;
            if(instance->IsClosing())
                ret = INSTANCE_ABORT_INSTANCE_CLOSING;
            else if(instance->IsFull())
                ret = INSTANCE_ABORT_FULL;
            else if(instance->CheckCombatStatus())
                ret = INSTANCE_ABORT_ENCOUNTER;
            // else ret = instance_ok and we can enter our existing instance
        }   // Check instance data for load preparation, if no data abort
        else if(m_instanceData.find(instanceId) == m_instanceData.end())
            ret = INSTANCE_ABORT_NOT_FOUND;
        else // We have instance data, run checks on if we can access it
        {
            InstanceData *data = m_instanceData.at(instanceId);
            if(data->GetMapId() != mapId) // Check if we're loading into a different instance to prevent abuse
                ret = INSTANCE_ABORT_NOT_FOUND;
            // Check if we're in the list of allowed players to load in(we need to have entered normally)
            /*if(data->PlayerBlocked(plr) || data->IsExpired())
                ret = INSTANCE_ABORT_NOT_FOUND;*/
        }
    } else if(canCreate && (mapData = GetMapData(mapId)))
        ret = INSTANCE_ABORT_CREATE_NEW_INSTANCE;
    else ret = canCreate ? INSTANCE_ABORT_NOT_FOUND : INSTANCE_ABORT_TOO_MANY;
    instanceStorageLock.Release();
    return ret;
}

void InstanceManager::_AddInstance(uint32 instanceId, MapInstance *instance)
{
    MapInstanceContainer *container = new MapInstanceContainer(instance);
    // Quick store our container and instance since we've allocated them to prevent double allocation
    mInstanceStorage.insert(std::make_pair(instanceId, std::make_pair(container, instance)));
    // Preload instance data before mapping
    instance->Preload();
    // Time for push to pool
    instancePoolLock.Acquire();
    // Get our timer inside the lock so that everything occurs in the same timeframe
    uint32 msTime = getMSTime();
    instance->Init(msTime);
    container->ResetTimer(msTime);
    mInstancePool.push_back(container);
    instancePoolLock.Release();
}

MapInstance *InstanceManager::_LoadInstance(uint32 mapId, uint32 instanceId)
{
    MapInstance *ret = NULL;
    if(Map *mapData = GetMapData(mapId))
    {
        ret = new MapInstance(mapData, mapId, instanceId);
        //ret->LoadInstanceData();
        _AddInstance(instanceId, ret);
    }
    return ret;
}

void InstanceManager::HandleUpdateRequests(InstanceManagerSlave *slaveThis)
{
    uint32 diff = 0, msTimer = 0;
    MapInstanceContainer *container = NULL;
    while(slaveThis->SetThreadState(THREADSTATE_BUSY))
    {
        msTimer = getMSTime();
        instancePoolLock.Acquire();
        if(!mInstancePool.empty())
        {
            container = mInstancePool.front();
            // Test if the update timeout has passed and grab our map difference
            if(container->Validate(msTimer, diff))
                mInstancePool.pop_front();
            else container = NULL;
        }
        instancePoolLock.Release();

        if(container != NULL)
        {
            if(MapInstance *instance = container->Get())
            {
                // Update our collision system via instanced map system
                sVMapInterface.UpdateSingleMap(instance->GetMapId(), diff, instance->GetInstanceID());

                // Process all pending removals in sequence
                instance->_PerformPendingRemovals();
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Process all pending inputs in sequence
                instance->_ProcessInputQueue();
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all combat state updates before any unit updates
                instance->_PerformCombatUpdates(msTimer, diff);
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all delayed spell updates before object updates
                instance->_PerformDelayedSpellUpdates(msTimer, diff);
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all unit path updates in sequence
                instance->_PerformUnitPathUpdates(msTimer, diff);
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all player updates in sequence
                instance->_PerformPlayerUpdates(msTimer, diff);
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all dynamic object updates in sequence
                instance->_PerformDynamicObjectUpdates(msTimer, diff);
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all creature updates in sequence
                instance->_PerformCreatureUpdates(msTimer, diff);
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all object updates in sequence
                instance->_PerformObjectUpdates(msTimer, diff);
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all movement updates in sequence without player data
                instance->_PerformMovementUpdates(false);
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all session updates in sequence
                instance->_PerformSessionUpdates();
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all movement updates in sequence with player data
                instance->_PerformMovementUpdates(true);
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Process secondary pending removals in sequence
                instance->_PerformPendingRemovals();
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Perform all pending object updates in sequence
                instance->_PerformPendingUpdates();
                if(!slaveThis->SetThreadState(THREADSTATE_BUSY))
                    break;
                // Reset the last update timer for next update processing
                container->ResetTimer(msTimer);
                // Readd the instance to the update pool
                instancePoolLock.Acquire();
                mInstancePool.push_back(container);
                instancePoolLock.Release();
            } else delete container; // Clean up the empty container
        }
        if(!slaveThis->SetThreadState(THREADSTATE_SLEEPING))
            break;
        Sleep(25);
    }
}
