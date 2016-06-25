
#include "StdAfx.h"

SERVER_DECL InstanceManager sInstanceMgr;

InstanceManager::InstanceManager()
{

}

InstanceManager::~InstanceManager()
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
    // Create all non-instance type maps.
    if( QueryResult *result = CharacterDatabase.Query( "SELECT MAX(id) FROM instances" ) )
    {
        m_instanceCounter = result->Fetch()[0].GetUInt32();
        delete result;
    } else m_instanceCounter = 0x3FF;

}

MapInstance *InstanceManager::GetInstanceForObject(WorldObject *obj)
{
    Map *mapData = NULL;
    MapInstance *ret = NULL;
    instanceStorageLock.Acquire();
    uint32 mapId = obj->GetMapId(), instanceId = obj->GetInstanceID();
    if(instanceId != 0)
    {
        // Check to see if we have the instance ID in storage
        if(mInstanceStorage.find(instanceId) != mInstanceStorage.end())
            ret = mInstanceStorage.at(instanceId).second;
        // If not, then we can return nothing
    }
    else if(obj->IsPlayer() && (mapData = GetMapData(mapId)))
    {
        Player *plr = castPtr<Player>(obj);
        if(false)//instanceId = castPtr<Player>(obj)->GetLinkedInstanceID(mapId))
        {

        }
        else if(plr->CanCreateNewDungeon())
        {
            // Instance ID generation occurs inside mutex
            uint32 instanceId = ++m_instanceCounter;
            ret = new MapInstance(mapData, mapId, instanceId);
            _AddInstance(instanceId, ret);
        }
    }

    instanceStorageLock.Release();
    return ret;
}

MapInstance *InstanceManager::GetOrCreateInstance(uint32 mapId, uint32 instanceId, bool canCreate)
{
    Map *mapData = NULL;
    MapInstance *ret = NULL;
    instanceStorageLock.Acquire();
    if(instanceId != 0)
    {
        // Check to see if we have the instance ID in storage
        if(mInstanceStorage.find(instanceId) != mInstanceStorage.end())
            ret = mInstanceStorage.at(instanceId).second;
        // If not, then we can return nothing
    }
    else if(canCreate && (mapData = GetMapData(mapId)))
    {
        // Instance ID generation occurs inside mutex
        uint32 instanceId = ++m_instanceCounter;
        ret = new MapInstance(mapData, mapId, instanceId);
        _AddInstance(instanceId, ret);
    }

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
                // Process all pending inputs in sequence
                instance->_ProcessInputQueue();
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
                // Perform all session updates in sequence
                instance->_PerformSessionUpdates();
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
