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

    // Nullify these counters here
    m_creatureGUIDCounter = m_gameObjectGUIDCounter = 0;

    // Assign a connection to this thread
    StateDatabase.AssignThreadConnection();
    // clear any instances that have expired.
    sLog.Notice("WorldManager", "Deleting Expired Instances...");
    StateDatabase.WaitExecute("DELETE FROM instance_data WHERE expiration <= %u", UNIXTIME);
    // Clear any states from deleted instances
    StateDatabase.WaitExecute("DELETE FROM instance_data_object_state WHERE instanceId NOT IN (SELECT id FROM instance_data)");
    // Clear any links from deleted instances
    StateDatabase.WaitExecute("DELETE FROM instance_links WHERE instanceId NOT IN (SELECT id FROM instance_data)");
    // Release our assigned thread connection
    StateDatabase.ReleaseThreadConnection();

    // Load our instance data counter
    if( QueryResult *result = StateDatabase.Query( "SELECT MAX(id) FROM instance_data" ) )
    {
        m_instanceCounter = result->Fetch()[0].GetUInt32();
        delete result;
    } else m_instanceCounter = 0x3FF;

    // Push our instance script assignment as well
    SetupInstanceScripts();
}

void InstanceManager::Prepare()
{
    SetupInstanceScripts();
}

void InstanceManager::_LoadInstances()
{
    std::map<uint32, MapScriptAllocator*>::iterator itr;
    for(itr = m_mapScriptAllocators.begin(); itr != m_mapScriptAllocators.end(); itr++)
        itr->second->CheckInstanceDataTables();

    std::map<uint32, std::vector<std::pair<WoWGuid, uint8>>> objectCache;
    // Preload and cache creature/gameobject state
    if(QueryResult *result = StateDatabase.Query("SELECT * FROM instance_data_object_state"))
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 instanceId = fields[0].GetUInt32();
            WoWGuid guid = fields[1].GetUInt64();
            uint8 state = fields[2].GetUInt8();
            objectCache[instanceId].push_back(std::make_pair(guid, state));
        }while(result->NextRow());
    }

    // load saved instances
    if(QueryResult *result = StateDatabase.Query("SELECT * FROM instance_data"))
    {
        uint32 count = 0;
        do
        {
            Field *fields = result->Fetch();
            uint32 instanceId = fields[0].GetUInt32();
            uint32 mapId = fields[1].GetUInt32();
            InstanceData *data = new InstanceData(mapId, instanceId);
            if(!data->LoadFromDB(fields, objectCache[instanceId]))
                continue;
            m_instanceData.insert(std::make_pair(instanceId, data));

            if((itr = m_mapScriptAllocators.find(mapId)) != m_mapScriptAllocators.end())
                itr->second->LoadExtraInstanceData(instanceId);

            count++;
        } while(result->NextRow());
        delete result;

        sLog.Success("WorldManager", "Loaded %u saved instance(s)." , count);
    } else sLog.Debug("WorldManager", "No saved instances found.");

    // load saved instance links
    if(QueryResult *result = StateDatabase.Query("SELECT * FROM instance_links"))
    {
        do
        {
            Field *fields = result->Fetch();
            WoWGuid guid = fields[0].GetUInt64();
            uint32 instanceId = fields[2].GetUInt32();

            // Just link the mapped section to our instanceId
            m_guidLinkedDungeons[guid][fields[1].GetUInt32()] = instanceId;
            // Link instance side
            m_dungeonLinkedGuids[instanceId].insert(guid);
        } while(result->NextRow());
    }

    objectCache.clear();
}

MapScript *InstanceManager::AllocateMapScript(MapInstance *instance)
{
    std::map<uint32, MapScriptAllocator*>::iterator itr;
    if((itr = m_mapScriptAllocators.find(instance->GetMapId())) != m_mapScriptAllocators.end())
        return itr->second->Allocate(instance);
    return NULL;
}

void InstanceManager::AssignMapScriptAllocator(uint32 mapId, MapScriptAllocator *allocator)
{
    if(m_mapScriptAllocators.find(mapId) != m_mapScriptAllocators.end())
        return;
    m_mapScriptAllocators.insert(std::make_pair(mapId, allocator));
}

void InstanceManager::LaunchGroupFinderDungeon(uint32 mapId, GroupFinderMgr::GroupFinderDungeon *dungeon, Group *grp)
{
    Map *mapData = GetMapData(mapId);
    ASSERT(mapData != NULL && mapData->GetEntry()->IsDungeon());

    // Instance ID generation occurs inside mutex
    counterLock.Acquire();
    dungeon->instanceId = ++m_instanceCounter;
    counterLock.Release();

    instanceStorageLock.Acquire();
    InstanceData *data = new InstanceData(mapId, dungeon->instanceId, WoWGuid(), grp->GetGuid(), UNIXTIME, UNIXTIME+TIME_DAY, 0);//dungeon->difficulty);
    m_instanceData.insert(std::make_pair(dungeon->instanceId, data));
    MapInstance *instance = new MapInstance(mapData, mapId, dungeon->instanceId, data);
    _AddInstance(dungeon->instanceId, instance);
    instanceStorageLock.Release();

    LinkGuidToInstance(instance, grp->GetGuid(), true);
    LocationVector destination(dungeon->dataEntry->x, dungeon->dataEntry->y, dungeon->dataEntry->z, dungeon->dataEntry->o);
    for(uint32 i = 0; i < grp->GetSubGroupCount(); ++i)
    {
        SubGroup *sub = grp->GetSubGroup(i);
        if(sub == NULL)
            continue;

        for(auto itr = sub->GetGroupMembersBegin(); itr != sub->GetGroupMembersEnd(); ++itr)
        {
            if(Player *plr = (*itr)->m_loggedInPlayer)
                plr->SafeTeleport(instance, destination);
            LinkGuidToInstance(instance, (*itr)->charGuid, true);
        }
    }
}

MapInstance *InstanceManager::GetInstanceForObject(WorldObject *obj)
{
    Map *mapData = NULL;
    MapInstance *ret = NULL;
    uint32 mapId = obj->GetMapId(), instanceId = obj->GetInstanceID();
    // Instance manager only handles dungeon maps, return if we have no data
    if((mapData = GetMapData(mapId)) == NULL)
        return ret;
    // Check if we have a linked instance or if we're not able to create a new one
    if(obj->IsPlayer() && !GetLinkedInstanceID(castPtr<Player>(obj), mapData->GetEntry(), castPtr<Player>(obj)->GetDifficulty(mapData->GetEntry()), instanceId, false))
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
        // Instance ID generation occurs inside mutex
        counterLock.Acquire();
        uint32 instanceId = ++m_instanceCounter;
        counterLock.Release();
        InstanceData *data = new InstanceData(mapId, instanceId, obj->GetGUID(), castPtr<Player>(obj)->GetGroupGuid(), UNIXTIME, UNIXTIME+TIME_DAY, 0);//dungeon->difficulty);
        m_instanceData.insert(std::make_pair(instanceId, data));
        _AddInstance(instanceId, ret = new MapInstance(mapData, mapId, instanceId, data));
        LinkGuidToInstance(ret, obj->GetGUID(), false);
    }

    instanceStorageLock.Release();
    return ret;
}

void InstanceManager::ResetInstanceLinks(Player *plr)
{
    Group *grp = NULL;
    if((grp = plr->GetGroup()) && grp->GetLeader() == plr->getPlayerInfo())
    {

    }

}

bool InstanceManager::GetLinkedInstanceID(Player *plr, MapEntry *map, uint32 difficulty, uint32 &instanceId, bool groupFinder)
{
    WoWGuid &guid = plr->GetGUID(), grpGuid;
    if(Group *grp = plr->GetGroup())
        grpGuid = grp->GetGuid();

    uint32 uniqueMapId = (difficulty<<28)|map->MapID;
    if(instanceId != 0 && m_dungeonLinkedGuids.find(instanceId) != m_dungeonLinkedGuids.end())
    {
        if(m_dungeonLinkedGuids[instanceId].find(guid) != m_dungeonLinkedGuids[instanceId].end())
            return true;
        if(!grpGuid.empty() && m_dungeonLinkedGuids[instanceId].find(grpGuid) != m_dungeonLinkedGuids[instanceId].end())
            return true;
    }

    // See if the group we're in is locked to any instanceIds matching this unique mapId
    if(!grpGuid.empty() && (m_guidLinkedDungeons.find(grpGuid) != m_guidLinkedDungeons.end() && m_guidLinkedDungeons[grpGuid].find(uniqueMapId) != m_guidLinkedDungeons[grpGuid].end()))
    {
        instanceId = m_guidLinkedDungeons[grpGuid][uniqueMapId];
        return true;
    }

    // Since we're either not looking for an instance or not eligble for the instance we're looking for, check if we have an eligible instance
    if(m_guidLinkedDungeons.find(guid) != m_guidLinkedDungeons.end() && m_guidLinkedDungeons[guid].find(uniqueMapId) != m_guidLinkedDungeons[guid].end())
    {
        instanceId = m_guidLinkedDungeons[guid][uniqueMapId];
        return true;
    }

    if(true) // Check if we can load new instance
    {
        instanceId = 0;
        return true;
    }
    return false;
}

uint32 InstanceManager::PreTeleportInstanceCheck(PlayerInfo *info, MapEntry *map, uint32 instanceId)
{
    Map *mapData = GetMapData(map->MapID);
    if(mapData == NULL)
        return INSTANCE_ABORT_NOT_FOUND;

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
            else if((ret = instance->IsFull(info)) != INSTANCE_OK)
            {
                instanceStorageLock.Release();
                return ret;
            }
            else if(instance->CheckCombatStatus())
                ret = INSTANCE_ABORT_ENCOUNTER;
            // else ret = instance_ok and we can enter our existing instance
        }   // Check instance data for load preparation, if no data abort
        else if(m_instanceData.find(instanceId) == m_instanceData.end())
            ret = INSTANCE_ABORT_NOT_FOUND;
        else // We have instance data, run checks on if we can access it
        {
            InstanceData *data = m_instanceData.at(instanceId);
            if(data->GetMapId() != map->MapID) // Check if we're loading into a different instance to prevent abuse
                ret = INSTANCE_ABORT_NOT_FOUND;
            // Check if we're in the list of allowed players to load in(we need to have entered normally)
            /*if(data->PlayerBlocked(plr) || data->IsExpired())
                ret = INSTANCE_ABORT_NOT_FOUND;*/
        }
    }
    instanceStorageLock.Release();
    return ret;
}

bool InstanceManager::LinkGuidToInstance(MapInstance *instance, WoWGuid guid, bool groupFinder)
{
    uint32 mapId = instance->GetMapId(), difficulty = 0/*instance->GetDifficulty()*/, instanceId = instance->GetInstanceID(), uniqueMapId = (difficulty << 28) | mapId;
    if(m_guidLinkedDungeons[guid].find(uniqueMapId) != m_guidLinkedDungeons[guid].end())
        return false; // We're already linked, don't link us to something else
    if(m_dungeonLinkedGuids[instanceId].size() && GUID_HIPART(*m_dungeonLinkedGuids[instanceId].begin()) == HIGHGUID_TYPE_GROUP)
        return false; // We're already linked to a group, don't link any players

    if(guid.getHigh() == HIGHGUID_TYPE_GROUP)
    {
        StateDatabase.Execute("DELETE FROM instance_links WHERE instanceId = '%u';", instanceId);
        while(m_dungeonLinkedGuids[instanceId].size())
        {
            WoWGuid guid2 = *m_dungeonLinkedGuids[instanceId].begin();
            m_dungeonLinkedGuids[instanceId].erase(m_dungeonLinkedGuids[instanceId].begin());
            // Clear out our last validation for this guid against our dungeon
            if(m_guidLinkedDungeons.find(guid2) != m_guidLinkedDungeons.end()
                && m_guidLinkedDungeons[guid2].find(uniqueMapId) != m_guidLinkedDungeons[guid2].end()
                && m_guidLinkedDungeons[guid2][uniqueMapId] == instanceId)
            {
                m_guidLinkedDungeons[guid2].erase(uniqueMapId);
            }
        }
    }

    StateDatabase.Execute("REPLACE INTO instance_links VALUES('%llu', '%u', '%u');", guid.raw(), uniqueMapId, instanceId);

    // Just link the mapped section to our instanceId
    m_guidLinkedDungeons[guid][uniqueMapId] = instanceId;
    // Link instance side
    m_dungeonLinkedGuids[instanceId].insert(guid);
    return true;
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
    Map *mapData = GetMapData(mapId);
    if(mapData == NULL)
        return NULL;

    MapInstance *ret = NULL;
    InstanceDataMap::iterator itr;
    if((itr = m_instanceData.find(instanceId)) != m_instanceData.end())
        _AddInstance(instanceId, ret = new MapInstance(mapData, mapId, instanceId, m_instanceData.at(instanceId)));

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
                // Process all script updates before object updates
                instance->_PerformScriptUpdates(msTimer, diff);
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

void InstanceData::Update(uint32 msTime)
{
    if(!m_isUpdated)
        return;
    m_isUpdated = false;

    // TODO: Add a timer
    SaveToDB();
}

bool InstanceData::LoadFromDB(Field *fields, std::vector<std::pair<WoWGuid, uint8>> &spawnState)
{
    m_creation = fields[2].GetUInt64();
    m_expiration = fields[3].GetUInt64();
    m_difficulty = fields[4].GetUInt16();
    m_creatorGroup = fields[5].GetUInt64();
    m_creatorGuid = fields[6].GetUInt64();

    for(auto itr = spawnState.begin(); itr != spawnState.end(); ++itr)
        m_objectState.insert(std::make_pair(itr->first, itr->second));
    return true;
}

void InstanceData::SaveToDB()
{
    std::stringstream ss;
    for(auto itr = m_objectState.begin(); itr != m_objectState.end(); ++itr)
    {
        if(ss.str().length())
            ss << ", ";

        ss << "('" << m_instanceId << "', '";
        ss << itr->first.raw() << "', '";
        ss << ((uint32)itr->second) << "')";
    }

    StateDatabase.Execute("DELETE FROM instance_data_object_state WHERE instanceId = %u", m_instanceId);
    StateDatabase.Execute("REPLACE INTO instance_data VALUES('%u', '%u', '%llu', '%llu', '%u', '%llu', '%llu', '');", m_instanceId, m_mapId, m_creation, m_expiration, m_difficulty, m_creatorGroup.raw(), m_creatorGuid.raw());
    if(ss.str().length())
        StateDatabase.Execute("REPLACE INTO instance_data_object_state VALUES %s;", ss.str().c_str());
}

void InstanceData::DeleteFromDB()
{
    // Delete all links
    StateDatabase.Execute("DELETE FROM instance_links WHERE instanceId = '%u';", m_instanceId);
}

uint8 InstanceData::GetObjectState(WoWGuid guid)
{
    Loki::AssocVector<WoWGuid, uint8>::iterator itr;
    if((itr = m_objectState.find(guid)) != m_objectState.end())
        return itr->second;
    return 0;
}

void InstanceData::AddObjectState(WoWGuid guid, uint8 state)
{
    Loki::AssocVector<WoWGuid, uint8>::iterator itr;
    if((itr = m_objectState.find(guid)) != m_objectState.end() && state == 0)
        m_objectState.erase(itr);
    else if(itr != m_objectState.end())
        itr->second = state;
    else m_objectState.insert(std::make_pair(guid, state));
}
