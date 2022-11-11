/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
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

#define MAPMGR_INACTIVE_MOVE_TIME 10
extern bool bServerShutdown;

unsigned int MapInstance::ActiveCellRange = 2;

MapInstance::MapInstance(Map *map, uint32 mapId, uint32 instanceid, InstanceData *data) : CellHandler<MapCell>(map), _mapId(mapId), m_instanceID(instanceid), m_script(NULL), pdbcMap(dbcMap.LookupEntry(mapId)),
    m_stateManager(new WorldStateManager(this)), _processCallback(this), _removalCallback(this)
{
    m_mapPreloading = false;

    m_GOHighGuid = 0;
    m_CreatureHighGuid = 0;
    m_DynamicObjectHighGuid=0;
    m_battleground = NULL;

    InactiveMoveTime = 0;
    iInstanceMode = 0;

    m_forceCombatState = false;

    // buffers
    m_dataBuffer.reserve(0x7FFF);

    m_PlayerStorage.clear();
    m_DynamicObjectStorage.clear();

    _updates.clear();
    _processQueue.clear();
    MapSessions.clear();

    m_corpses.clear();

    uint32 threadCount = 0;
    uint64 coreAffinity = 0;
    if(pdbcMap && pdbcMap->IsContinent() && (coreAffinity = sWorld.GetCoreAffinity(_mapId, &threadCount)) != 0)
        sLog.Notice("MapInstance", "Map %u assigned %u cores, mask %llu\n", _mapId, threadCount, coreAffinity);

    if(_updatePool = threadCount ? sThreadManager.SpawnPool(threadCount, coreAffinity) : NULL)
        _updatePool->attach();

    // Objects and paths are updated in parallel threads, initialize pools for each update thread
    mCreaturePool.Initialize(pdbcMap && pdbcMap->IsContinent() ? 8 * std::max<uint32>(1, threadCount) : 4);
    mGameObjectPool.Initialize(pdbcMap && pdbcMap->IsContinent() ? 4 * std::max<uint32>(1, threadCount) : 2);
    mDynamicObjectPool.Initialize(pdbcMap && pdbcMap->IsContinent() ? 2 * std::max<uint32>(1, threadCount) : 1);
    mUnitPathPool.Initialize(pdbcMap && pdbcMap->IsContinent() ? 2 * std::max<uint32>(1, threadCount) : 1);
    mTransporterPool.Initialize(2);

    projectileSpellUpdateTime[0] = projectileSpellUpdateTime[1] = 0;
    projectileSpellIndex[0] = projectileSpellIndex[1] = 0;

    if(m_instanceData = (m_iData = data) ? new MapInstance::MapInstanceData() : NULL)
    {
        m_instanceData->difficulty = m_iData->getDifficulty();
        m_instanceData->linkedGroupId = 0;
    }

    m_script = sInstanceMgr.AllocateMapScript(this);
}

MapInstance::~MapInstance()
{

}

void MapInstance::Preload()
{
    if(sWorld.ServerPreloading >= 2)
        UpdateAllCells(true);
    else if(IsInstance())
    {

    }
}

void MapInstance::Init(uint32 msTime)
{
    sTransportMgr.PreloadMapInstance(msTime, this, _mapId);

    mCreaturePool.ResetTime(msTime);
    mGameObjectPool.ResetTime(msTime);
    mTransporterPool.ResetTime(msTime);
    mDynamicObjectPool.ResetTime(msTime);
    mUnitPathPool.ResetTime(msTime);
}

void MapInstance::Destruct()
{
    if( m_stateManager != NULL )
    {
        delete m_stateManager;
        m_stateManager = NULL;
    }

    // Clean up our pools ahead of time
    if(_updatePool)
    {
        sThreadManager.CleanPool(_updatePool->getPoolId());
        _updatePool = NULL;
    }

    mCreaturePool.Cleanup();
    mGameObjectPool.Cleanup();
    mTransporterPool.Cleanup();
    mDynamicObjectPool.Cleanup();
    mUnitPathPool.Cleanup();

    _inRangeTargetCBStack.cleanup();
    _broadcastMessageCBStack.cleanup();
    _broadcastMessageInRangeCBStack.cleanup();
    _broadcastChatPacketCBStack.cleanup();
    _broadcastObjectUpdateCBStack.cleanup();
    _dynamicObjectTargetMappingCBStack.cleanup();
    _spellTargetMappingCBStack.cleanup();

    UnloadCells();
    _PerformPendingActions();

    std::vector<WorldObject*> m_delQueue;
    while(m_CreatureStorage.size())
    {
        Creature *ctr = m_CreatureStorage.begin()->second;
        m_delQueue.push_back(ctr);
        ctr->RemoveFromWorld();
    }
    while(m_gameObjectStorage.size())
    {
        GameObject *gObj = m_gameObjectStorage.begin()->second;
        m_delQueue.push_back(gObj);
        gObj->RemoveFromWorld();
    }
    while(m_transporterStorage.size())
    {
        GameObject *gObj = m_transporterStorage.begin()->second;
        m_delQueue.push_back(gObj);
        gObj->RemoveFromWorld();
    }
    while(m_DynamicObjectStorage.size())
    {
        DynamicObject *dynObj = m_DynamicObjectStorage.begin()->second;
        m_delQueue.push_back(dynObj);
        dynObj->RemoveFromWorld();
    }

    _PerformPendingActions();
    while(m_delQueue.size())
    {
        WorldObject *obj = *m_delQueue.begin();
        m_delQueue.erase(m_delQueue.begin());
        obj->Destruct();
    }

    if(m_corpses.size())
    {
        for(std::vector<Corpse* >::iterator itr = m_corpses.begin(); itr != m_corpses.end();)
        {
            Corpse* pCorpse = *itr;
            ++itr;

            pCorpse->Cleanup();
        }
        m_corpses.clear();
    }

    _PerformPendingActions();

    //Clear our remaining containers
    m_PlayerStorage.clear();
    m_DynamicObjectStorage.clear();
    m_CreatureStorage.clear();
    m_gameObjectStorage.clear();
    m_transporterStorage.clear();

    _updates.clear();
    _processQueue.clear();
    MapSessions.clear();
    pdbcMap = NULL;

    m_battleground = NULL;

    CleanupCells();

    delete this;
}

WorldObject *MapInstance::GetInRangeObject(ObjectCellManager *manager, WoWGuid guid)
{
    if(m_objectCells.find(guid) == m_objectCells.end())
        return NULL;
    uint32 cellId = m_objectCells.at(guid);
    if(!manager->hasCell(cellId))
        return NULL;
    std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(cellId);
    if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
        return cell->FindObject(guid);
    return NULL;
}

void MapInstance::PrePushObject(WorldObject* obj)
{
    /////////////
    // Assertions
    /////////////
    ASSERT(obj);
    ASSERT(obj->GetMapId() == _mapId);
    ASSERT(obj->GetPositionZ() < _maxY && obj->GetPositionZ() > _minY);

    float mx = obj->GetPositionX(), my = obj->GetPositionY(), mz = obj->GetPositionZ();
    uint32 cx = GetPosX(mx), cy = GetPosY(my);

    MapCell* objCell = GetCell(cx, cy);
    if (objCell == NULL && (objCell = Create(cx, cy)) != NULL) // Should never fail to create but...
        objCell->Init(cx, cy, _mapId, this);
    ASSERT(objCell);

    // Push object cell activity notification
    UpdateCellActivity(cx, cy, MapInstance::ActiveCellRange, obj->IsPlayer());
}

void MapInstance::PushObject(WorldObject* obj)
{
    /////////////
    // Assertions
    /////////////
    ASSERT(obj);

    obj->GetCellManager()->ClearInRangeObjects(this);

    Player* plObj = NULL;
    if(obj->IsPlayer())
    {
        if((plObj = castPtr<Player>( obj )) == NULL)
        {
            sLog.Debug("MapInstance","Could not get a valid playerobject from object while trying to push to world");
            return;
        }

        WorldSession * plSession = plObj->GetSession();
        if(plSession == NULL)
        {
            sLog.Debug("MapInstance","Could not get a valid session for player while trying to push to world");
            return;
        }

        if(m_PlayerStorage.find(plObj->GetGUID()) != m_PlayerStorage.end())
        {
            sLog.Debug("MapInstance","Player being pushed when already stored in world");
            return;
        }
        plObj->ClearInRangeObjects();
    }

    ///////////////////////
    // Get cell coordinates
    ///////////////////////

    ASSERT(obj->GetMapId() == _mapId);
    ASSERT(obj->GetPositionZ() < _maxY && obj->GetPositionZ() > _minY);

    // Grab object position information
    float mx = obj->GetPositionX();
    float my = obj->GetPositionY();
    float mz = obj->GetPositionZ();
    uint32 cx = GetPosX(mx), cy = GetPosY(my);

    // Grab our object cell
    MapCell* objCell = GetCell(cx, cy);
    if( objCell == NULL ||
        mx > _maxX || my > _maxY ||
        mx < _minX || my < _minY ||
        cx >= _sizeX || cy >= _sizeY)
    {
        if( plObj != NULL )
        {
            if(plObj->GetBindMapId() != GetMapId())
            {
                plObj->SafeTeleport(plObj->GetBindMapId(),0,plObj->GetBindPositionX(),plObj->GetBindPositionY(),plObj->GetBindPositionZ(),0);
                plObj->GetSession()->SystemMessage("Teleported you to your hearthstone location as you ended up on the wrong map.");
                return;
            }
            else
            {
                obj->GetPositionV()->ChangeCoords(plObj->GetBindPositionX(),plObj->GetBindPositionY(),plObj->GetBindPositionZ(),0);
                plObj->GetSession()->SystemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
                WorldPacket * data = plObj->BuildTeleportAckMsg(plObj->GetPosition());
                plObj->PushPacket(data);
                delete data;
            }
        } else obj->GetPositionV()->ChangeCoords(0.f,0.f,0.f,0.f);

        cx = GetPosX(mx = obj->GetPositionX());
        cy = GetPosY(my = obj->GetPositionY());
        mz = obj->GetPositionY();

        if ((objCell = GetCell(cx, cy)) == NULL)
        {
            if (obj->IsPlayer())
                castPtr<Player>(obj)->SoftDisconnect();
            return;
        }
    }

    m_dataBufferLock.Acquire();
    uint32 count = 0, minX = cx ? cx-1 : 0, maxX = (cx < _sizeY-1 ? cx+1 : _sizeY-1), minY = cy ? cy-1 : 0, maxY = (cy < _sizeY-1 ? cy+1 : _sizeY-1);
    if(plObj && (count = plObj->BuildCreateUpdateBlockForPlayer(&m_dataBuffer, plObj)))
    {
        sLog.Debug("MapInstance","Creating player %llu for himself.", obj->GetGUID().raw());
        plObj->PushUpdateBlock(_mapId, &m_dataBuffer, count);
    }
    m_dataBuffer.clear();
    m_dataBufferLock.Release();

    // We must always have a cell at this point
    ASSERT(objCell);

    //Add to the cell's object list
    objCell->AddObject(obj);
    obj->SetMapCell(objCell);

    m_objectStorageLock.Acquire();
    //Add to the mapmanager's object list
    if(plObj)
    {
        m_PlayerStorage.insert(std::make_pair(plObj->GetGUID(), plObj));
        UpdateCellActivity(cx, cy, MapInstance::ActiveCellRange);
    }
    else
    {
        switch(obj->GetHighGUID())
        {
        case HIGHGUID_TYPE_CORPSE:
            m_corpses.push_back( castPtr<Corpse>(obj) );
            break;

        case HIGHGUID_TYPE_VEHICLE:
        case HIGHGUID_TYPE_UNIT:
            {
                Creature *creature = castPtr<Creature>(obj);
                m_CreatureStorage.insert(std::make_pair(obj->GetGUID(), creature));
                TRIGGER_INSTANCE_EVENT( this, OnCreaturePushToWorld )( creature );
                mUnitPathPool.Add(creature->GetMovementInterface()->GetPath());
            }break;

        case HIGHGUID_TYPE_GAMEOBJECT:
            {
                GameObject* go = castPtr<GameObject>(obj);
                m_gameObjectStorage.insert(std::make_pair(obj->GetGUID(), go));
                TRIGGER_INSTANCE_EVENT( this, OnGameObjectPushToWorld )( go );
                sVMapInterface.LoadGameobjectModel(obj->GetGUID(), _mapId, go->GetDisplayId(), go->GetFloatValue(OBJECT_FIELD_SCALE_X), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), go->GetOrientation(), go->GetInstanceID(), go->GetPhaseMask());
            }break;

        case HIGHGUID_TYPE_MO_TRANSPORT:
            {
                GameObject* go = castPtr<GameObject>(obj);
                mTransporterPool.Add(go);
                m_transporterStorage.insert(std::make_pair(obj->GetGUID(), go));
                TRIGGER_INSTANCE_EVENT( this, OnGameObjectPushToWorld )( go );

                //sVMapInterface.LoadGameobjectModel(obj->GetGUID(), _mapId, go->GetDisplayId(), go->GetFloatValue(OBJECT_FIELD_SCALE_X), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), go->GetOrientation(), go->GetInstanceID(), go->GetPhaseMask());
            }break;

        case HIGHGUID_TYPE_DYNAMICOBJECT:
            m_DynamicObjectStorage.insert(std::make_pair(obj->GetGUID(), castPtr<DynamicObject>(obj)));
            break;
        }
    }
    m_objectStorageLock.Release();

    // Handle activation of that object.
    if(objCell->IsActive() && (!obj->IsBulkSpawn() || obj->IsDynamicObj()) && !obj->IsTransport())
    {
        m_poolLock.Acquire();
        switch(obj->GetTypeId())
        {
        case TYPEID_UNIT:
            {
                Creature *cObj = castPtr<Creature>(obj);
                mCreaturePool.Add(cObj, GetPoolOverrideForZone(obj->GetZoneId()));
            }break;

        case TYPEID_GAMEOBJECT:
            {
                GameObject *gObj = castPtr<GameObject>(obj);
                mGameObjectPool.Add(gObj, GetPoolOverrideForZone(obj->GetZoneId()));
            }break;

        case TYPEID_DYNAMICOBJECT:
            {
                DynamicObject *dObj = castPtr<DynamicObject>(obj);
                mDynamicObjectPool.Add(dObj, GetPoolOverrideForZone(obj->GetZoneId()));
            }break;
        }
        m_poolLock.Release();
    }

    // Add the session to our set if it is a player.
    if(plObj)
    {
        m_poolLock.Acquire();
        MapSessions.insert(plObj->GetSession());
        m_poolLock.Release();

        if(Group *grp = plObj->GetGroup())
            OnGroupEnter(plObj, grp);

        // Change the instance ID, this will cause it to be removed from the world thread (return value 1)
        plObj->GetSession()->SetEventInstanceId(GetInstanceID());

        // Update our player's area and zone visibility
        UpdateAreaAndZoneVisibility(plObj);
    }

    if(MapScript *script = m_script)
        script->OnPushObject(obj);

    // Push to our update queue
    m_updateMutex.Acquire();
    if(_updates.find(obj) == _updates.end())
        _updates.insert(obj);
    m_updateMutex.Release();

    // Update our cell manager with current position data
    // Skip cell loading for preloading maps or inactive objects
    bool skipCellLoad = m_mapPreloading || (obj->IsActiveObject() && !obj->IsActivated());
    obj->GetCellManager()->SetCurrentCell(skipCellLoad ? NULL : this, mx, my, mz, ObjectCellManager::VisibleCellRange);
    CacheObjectCell(obj->GetGUID(), ObjectCellManager::_makeCell(cx, cy));
}

void MapInstance::RemoveObject(WorldObject* obj)
{
    /////////////
    // Assertions
    /////////////

    ASSERT(obj);
    ASSERT(obj->GetMapId() == _mapId);

    m_poolLock.Acquire();
    RemoveCachedCell(obj->GetGUID());
    switch(obj->GetTypeId())
    {
    case TYPEID_UNIT:
        {
            Creature *cObj = castPtr<Creature>(obj);
            mCreaturePool.QueueRemoval(cObj);
        }break;

    case TYPEID_GAMEOBJECT:
        {
            GameObject *gObj = castPtr<GameObject>(obj);
            mGameObjectPool.QueueRemoval(gObj);
        }break;

    case TYPEID_DYNAMICOBJECT:
        {
            DynamicObject *dObj = castPtr<DynamicObject>(obj);
            mDynamicObjectPool.QueueRemoval(dObj);
        }break;
    }
    m_poolLock.Release();

    m_updateMutex.Acquire();
    _updates.erase(obj);
    m_updateMutex.Release();
    obj->ClearUpdateMask();

    ///////////////////////////////////////
    // Remove object from all needed places
    ///////////////////////////////////////
    if(Player *plObj = (obj->IsPlayer()) ? castPtr<Player>( obj ) : NULL)
    {
        m_updateMutex.Acquire();
        m_removeQueue.push(plObj);
        m_updateMutex.Release();
    }
    else
    {
        switch(obj->GetHighGUID())
        {
        case HIGHGUID_TYPE_VEHICLE:
        case HIGHGUID_TYPE_UNIT:
            {
                m_CreatureStorage.erase(obj->GetGUID());
                TRIGGER_INSTANCE_EVENT( this, OnCreatureRemoveFromWorld )( castPtr<Creature>(obj) );
                UnitPathSystem *path = castPtr<Creature>(obj)->GetMovementInterface()->GetPath();
                mUnitPathPool.QueueRemoval(path);
            }break;

        case HIGHGUID_TYPE_CORPSE:
            ClearCorpse(castPtr<Corpse>(obj));
            break;

        case HIGHGUID_TYPE_DYNAMICOBJECT:
            m_DynamicObjectStorage.erase(obj->GetGUID());
            break;

        case HIGHGUID_TYPE_GAMEOBJECT:
            {
                m_gameObjectStorage.erase(obj->GetGUID());
                TRIGGER_INSTANCE_EVENT( this, OnGameObjectRemoveFromWorld )( castPtr<GameObject>(obj) );
                sVMapInterface.UnLoadGameobjectModel(obj->GetGUID(), m_instanceID, _mapId);
            }break;

        case HIGHGUID_TYPE_MO_TRANSPORT:
            {
                GameObject *gObj = castPtr<GameObject>(obj);
                mTransporterPool.QueueRemoval(gObj);
                m_transporterStorage.erase(obj->GetGUID());
                TRIGGER_INSTANCE_EVENT( this, OnGameObjectRemoveFromWorld )( gObj );
                //sVMapInterface.UnLoadGameobjectModel(obj->GetGUID(), m_instanceID, _mapId);
            }break;
        }
    }

    if(MapScript *script = m_script)
        script->OnRemoveObject(obj);

    // Clear object's in-range set before we're removed from the cell
    obj->GetCellManager()->ClearInRangeObjects(this);

    // Now that we're not in the cell's objects range, we can clear out our data
    MapCell *currentCell = obj->GetMapCell();
    if(currentCell == NULL && (!(obj->GetPositionX() >= _maxX || obj->GetPositionX() <= _minY || obj->GetPositionY() >= _maxY || obj->GetPositionY() <= _minY)))
        currentCell = GetCellByCoords(obj->GetPositionX(), obj->GetPositionY());
    if(currentCell) // Remove object from cell
        currentCell->RemoveObject(obj);
    // Unset object's cell
    obj->SetMapCell(NULL);
}

void MapInstance::QueueRemoval(WorldObject *obj)
{
    m_setLock.Acquire();
    _pendingRemoval.insert(obj);
    m_setLock.Release();
}

void MapInstance::QueueCleanup(WorldObject *obj)
{
    m_setLock.Acquire();
    _pendingCleanup.insert(obj);
    obj->MarkForCleanup();
    m_setLock.Release();
}

void MapInstance::QueueSoftDisconnect(Player *plr)
{
    m_setLock.Acquire();
    _softDCPlayers.insert(plr);
    m_setLock.Release();
}

void MapInstance::ChangeObjectLocation( WorldObject* obj )
{
    if( obj->GetMapInstance() != this )
        return;

    // Grab our cell data from position
    float fposX = obj->GetPositionX(), fposY = obj->GetPositionY(), fposZ = obj->GetPositionZ();
    uint16 cellX = GetPosX(fposX), cellY = GetPosY(fposY);
    if(cellX >= _sizeX || cellY >= _sizeY)
        return; // No data outside parameters

    WorldObject* curObj = NULL;
    Player *plObj = obj->IsPlayer() ? castPtr<Player>(obj) : NULL;

    // Update our zone and area data
    obj->EventExploration(this);

    if(Group *grp = plObj ? plObj->GetGroup() : NULL)
        grp->HandlePartialChange( PARTY_UPDATE_FLAG_LOCATION, plObj );

    // Check if we're moving to a new map
    if(obj->GetMapInstance() != this)
    {
        // Clear our inrange objects on our maps cells
        obj->GetCellManager()->ClearInRangeObjects(this);
    }
    else
    {
        ///////////////////////////
        // Get new cell coordinates
        ///////////////////////////
        if(obj->GetPositionX() >= _maxX || obj->GetPositionX() <= _minX ||
            obj->GetPositionY() >= _maxY || obj->GetPositionY() <= _minY)
        {
            if(plObj)
            {
                if(plObj->GetBindMapId() != GetMapId())
                {
                    plObj->SafeTeleport(plObj->GetBindMapId(),0,plObj->GetBindPositionX(),plObj->GetBindPositionY(),plObj->GetBindPositionZ(),0);
                    plObj->GetSession()->SystemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
                    return;
                }
                else
                {
                    obj->GetPositionV()->ChangeCoords(plObj->GetBindPositionX(),plObj->GetBindPositionY(),plObj->GetBindPositionZ(),0);
                    plObj->GetSession()->SystemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
                    WorldPacket * data = plObj->BuildTeleportAckMsg(plObj->GetPosition());
                    plObj->PushPacket(data);
                    delete data;
                }
            }
            else
            {
                obj->GetCellManager()->ClearInRangeObjects(this);
                obj->GetPositionV()->ChangeCoords(0,0,0,0);
                return;
            }
        }

        // Update our cell activity real quick
        if(plObj)
        {
            UpdateCellActivity(cellX, cellY, MapInstance::ActiveCellRange);
            UpdateAreaAndZoneVisibility(plObj);
        }

        // Grab our new cell and store our old cell pointer
        MapCell *objCell = GetCellOrInit(cellX, cellY, obj->IsMapCellInitializer(), true), *pOldCell = obj->GetMapCell();
        if(objCell != pOldCell)
        {
            // Remove us from old cell
            if(pOldCell)
                pOldCell->RemoveObject(obj);

            obj->SetMapCell(objCell);
            // Process new cell
            if(objCell) // Add us to our cell
                objCell->AddObject(obj);

            // Non player objects need area info for current cell
            if(obj->IsPlayer() == false)
                obj->UpdateAreaInfo(this);

            // Set our cellId
            obj->GetCellManager()->SetCurrentCell(this, fposX, fposY, fposZ, ObjectCellManager::VisibleCellRange);
            CacheObjectCell(obj->GetGUID(), ObjectCellManager::_makeCell(cellX, cellY));
        }

        // Our full range objects need to have their visibility updated if our zone or area change
        if(IsFullRangeObject(obj))
        {
            Player *plr = NULL;
            std::set<WoWGuid> removeVisible;
            bool fullZone = m_zoneFullRangeObjects.find(obj) != m_zoneFullRangeObjects.end();
            bool fullArea = m_areaFullRangeObjects.find(obj) != m_areaFullRangeObjects.end();
            uint32 currZone = obj->GetZoneId(), currArea = obj->GetAreaId();
            for(std::set<WoWGuid>::iterator itr = obj->GetCellManager()->beginVisible(); itr != obj->GetCellManager()->endVisible(); ++itr)
            {
                Player *plr = GetPlayer(*itr);
                if(plr == NULL)
                {
                    removeVisible.insert(*itr);
                    continue;
                }

                if(fullZone && plr->GetZoneId() != currZone)
                    removeVisible.insert(*itr);
                else if(fullArea && plr->GetAreaId() != currArea)
                    removeVisible.insert(*itr);
            }

            if(fullZone && m_zoneFullRangeObjects[obj] != currZone)
            {   // Change our zone info
                uint32 oldZone = m_zoneFullRangeObjects[obj];
                // Erase from old zone vector our object
                std::vector<WorldObject*>::iterator itr;
                if((itr = std::find(m_fullRangeObjectsByZone[oldZone].begin(), m_fullRangeObjectsByZone[oldZone].end(), obj)) != m_fullRangeObjectsByZone[oldZone].end())
                    m_fullRangeObjectsByZone[oldZone].erase(itr);
                // Add to new vector our object and set new zone ID
                m_fullRangeObjectsByZone[currZone].push_back(obj);
                m_zoneFullRangeObjects[obj] = currZone;
            }

            if(fullArea && m_areaFullRangeObjects[obj] != currArea)
            {   // Change our area info
                uint32 oldArea = m_areaFullRangeObjects[obj];
                // Erase from old area vector our object
                std::vector<WorldObject*>::iterator itr;
                if((itr = std::find(m_fullRangeObjectsByArea[oldArea].begin(), m_fullRangeObjectsByArea[oldArea].end(), obj)) != m_fullRangeObjectsByArea[oldArea].end())
                    m_fullRangeObjectsByArea[oldArea].erase(itr);
                // Add to new vector our object and set new area ID
                m_fullRangeObjectsByArea[currArea].push_back(obj);
                m_areaFullRangeObjects[obj] = currArea;
            }

            for(auto itr = removeVisible.begin(); itr != removeVisible.end(); ++itr)
            {
                if(Player *plr = GetPlayer(*itr))
                    plr->RemoveIfVisible(_mapId, obj);

                obj->GetCellManager()->RemoveVisibleBy(*itr);
            }
        }
    }
}

void MapInstanceObjectProcessCallback::operator()(WorldObject *obj, WorldObject *curObj)
{
    Player *plObj = obj->IsPlayer() ? castPtr<Player>(obj) : NULL, *plObj2 = curObj->IsPlayer() ? castPtr<Player>(curObj) : NULL;
    if( plObj2 && _instance->canObjectsInteract(plObj2, obj) && plObj2->CanSee( obj ) && !plObj2->IsVisible( obj ) )
    {
        plObj2->AddVisibleObject(obj);
        obj->GetCellManager()->AddVisibleBy(plObj2->GetGUID());
        if(uint32 count = obj->BuildCreateUpdateBlockForPlayer(&m_callbackBuffer, plObj2))
        {
            plObj2->PushUpdateBlock(_instance->GetMapId(), &m_callbackBuffer, count);
            if(obj->IsUnit() && castPtr<Unit>(obj)->GetMovementInterface()->GetPath()->hasDestination())
                castPtr<Unit>(obj)->GetMovementInterface()->GetPath()->SendMovementPacket(plObj2, MOVEBCFLAG_DELAYED);
        }
        m_callbackBuffer.clear();
    }

    if( plObj != NULL && _instance->canObjectsInteract(plObj, curObj) && plObj->CanSee( curObj ) && !plObj->IsVisible( curObj ) )
    {
        plObj->AddVisibleObject( curObj );
        curObj->GetCellManager()->AddVisibleBy(plObj->GetGUID());
        if(uint32 count = curObj->BuildCreateUpdateBlockForPlayer( &m_callbackBuffer, plObj ))
        {
            plObj->PushUpdateBlock(_instance->GetMapId(), &m_callbackBuffer, count);
            if(curObj->IsUnit() && castPtr<Unit>(curObj)->GetMovementInterface()->GetPath()->hasDestination())
                castPtr<Unit>(curObj)->GetMovementInterface()->GetPath()->SendMovementPacket(plObj, MOVEBCFLAG_DELAYED);
        }
        m_callbackBuffer.clear();
    }
}

bool MapInstance::UpdateCellData(WorldObject *obj, uint32 cellX, uint32 cellY, bool playerObj, bool priority)
{
    // Check our cell status
    MapCell *objCell = GetCellOrInit(cellX, cellY, obj->IsMapCellInitializer(), priority);
    if(objCell == NULL)
        return false;

    _processCallback.SetCell(cellX, cellY);
    return objCell->ProcessObjectSets(obj, &_processCallback, NULL, playerObj ? 0x00 : TYPEMASK_TYPE_PLAYER);
}

void MapInstanceObjectRemovalCallback::operator()(WorldObject *obj, WorldObject *curObj)
{
    if(obj == curObj || (!_forced && _instance->IsFullRangeObject(curObj)))
        return;

    if( obj->IsPlayer() )
        castPtr<Player>(obj)->RemoveIfVisible(_instance->GetMapId(), curObj);
    if( curObj->IsPlayer() )
        castPtr<Player>( curObj )->RemoveIfVisible(_instance->GetMapId(), obj);
}

void MapInstance::RemoveCellData(WorldObject *Obj, std::set<uint32> &set, bool forced)
{
    if(!forced && IsFullRangeObject(Obj))
        return;

    _removalCallback.setForced(forced);
    for(auto itr = set.begin(); itr != set.end(); itr++)
    {
        std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(*itr);
        if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
            cell->ProcessObjectSets(Obj, &_removalCallback, NULL, Obj->IsPlayer() ? 0x00 : TYPEMASK_TYPE_PLAYER);
    }
}

bool MapInstance::HasActivatedCondition(uint32 conditionId, WorldObject *obj)
{
    // Check if condition is active
    if(m_activeConditions.find(conditionId) == m_activeConditions.end())
        return false;

    // TODO: check condition requirements

    // Passed condition checks, set us up
    return true;
}

uint8 MapInstance::GetPoolOverrideForZone(uint32 zoneId)
{
    switch(zoneId)
    {
        // Map cities
    case 1637: // Orgrimmar
    case 1638: // Thunderbluff
    case 1657: // Darnassus
    case 1497: // Undercity
    case 1519: // Stormwind City
    case 1537: // Ironforge
        // Don't need these yet
    case 3430: // Eversong Woods
    case 4080: // Isle of Quel'Danas
        //return 6;
    case 3524: // Azuremyst Isle
    case 3703: // Shattrath
        //return 7;
    default:
        return 0;
    }
}

class CellUpdateTask : public ThreadManager::PoolTask
{
public:
    CellUpdateTask(MapInstance *instance, MapCell *cellInfo, CellSpawns *sp) : _instance(instance), _cellInfo(cellInfo), _spawns(sp) { }

    virtual int call()
    {
        _cellInfo->SetActivity(true);
        _cellInfo->LoadCellData(_spawns);
        _instance->AddForcedCell(_cellInfo, 0);
        return 0;
    }

private:
    MapInstance *_instance;
    MapCell *_cellInfo;
    CellSpawns *_spawns;
};

void MapInstance::UpdateAllCells(bool apply, uint32 areamask)
{
    if(apply && areamask == 0)
    {
        m_mapPreloading = true;
        uint32 startTime = getMSTime();
        if(m_instanceID == 0) sLog.Info("MapInstance", "Updating all cells for map %03u, server might lag.", _mapId);
        for(SpawnsMap::iterator itr = _map->GetSpawnsMapBegin(); itr != _map->GetSpawnsMapEnd(); itr++)
        {
            MapCell *cellInfo = GetCell(itr->first.first, itr->first.second);
            if( cellInfo == NULL )
            {   // Cell doesn't exist, create it.
                cellInfo = Create( itr->first.first, itr->first.second );
                cellInfo->Init( itr->first.first, itr->first.second, _mapId, this );
            } else if(cellInfo && cellInfo->IsLoaded())
                continue;

            if(_updatePool)
            {
                _updatePool->AddTask(new CellUpdateTask(this, cellInfo, &itr->second));
                continue;
            }

            cellInfo->SetActivity(true);
            cellInfo->LoadCellData(&itr->second);
            AddForcedCell(cellInfo, 0);
        }

        if(_updatePool)
            _updatePool->wait();

        if(m_instanceID == 0) sLog.Success("MapInstance", "Cell preload for map %03u finished in %ums", _mapId, getMSTimeDiff(getMSTime(), startTime));
        m_mapPreloading = false;
    }
    else
    {
        uint32 loadCount = 0;
        for( uint32 x = 0; x < _sizeX; x++ )
        {
            for( uint32 y = 0; y < _sizeY; y++ )
            {
                if(areamask)
                {
                    uint16 areaId;
                    if(!GetBaseMap()->CellHasAreaID(x, y, areaId))
                        continue;

                    AreaTableEntry* at = dbcAreaTable.LookupEntry( areaId );
                    if(at == NULL || (at->ZoneId != areamask && at->AreaId != areamask))
                        continue;
                }

                MapCell *cellInfo = GetCell( x , y );
                if(apply)
                {
                    CellSpawns *spawns = _map->GetSpawnsList( x , y );
                    if(spawns == NULL)
                        continue;

                    if( cellInfo == NULL )
                    {   // Cell doesn't exist, create it.
                        cellInfo = Create( x , y );
                        cellInfo->Init( x , y , _mapId , this );
                        sLog.Debug("MapInstance","Created cell [%u,%u] on map %u (instance %u)." , x , y , _mapId , m_instanceID );
                    }

                    if (cellInfo->IsLoaded())
                        continue;

                    cellInfo->SetActivity(true);
                    loadCount += cellInfo->LoadCellData( spawns );
                    AddForcedCell(cellInfo, 0);
                } else if(cellInfo != NULL)
                    RemoveForcedCell(cellInfo);
            }
        }

        sLog.Success("MapInstance", "Cell update for map %03u finished with %u objLoad calls", _mapId, loadCount);
    }
}

void MapInstance::UpdateCellActivity(uint32 x, uint32 y, int radius, bool preloadPlayer)
{
    bool isActiveCellSet = _CellActive(x, y, radius);
    uint32 endX = (x + radius) <= _sizeX ? x + radius : (_sizeX-1);
    uint32 endY = (y + radius) <= _sizeY ? y + radius : (_sizeY-1);
    uint32 startX = x - radius > 0 ? x - radius : 0;
    uint32 startY = y - radius > 0 ? y - radius : 0;
    uint32 posX, posY;

    CellSpawns *sp = NULL;
    MapCell *objCell = NULL;
    for (posX = startX; posX <= endX; posX++ )
    {
        for (posY = startY; posY <= endY; posY++ )
        {
            if( posX >= _sizeX ||  posY >= _sizeY )
                continue;

            // Grab cell pointer and spawn list
            objCell = GetCell(posX, posY);
            sp = _map->GetSpawnsList(posX, posY);
            bool targetCell = posX == x && posY == y;
            // Check cell activity for spawn loading
            if (objCell == NULL && (isActiveCellSet || (preloadPlayer && targetCell)))
            {
                ASSERT(objCell = Create(posX, posY));
                objCell->Init(posX, posY, _mapId, this);
                objCell->SetActivity(true);
                ASSERT(!objCell->IsLoaded());
                objCell->LoadCellData(sp);
            }
            else if(objCell)
            {
                //Cell is now active
                if (!objCell->IsActive() && (isActiveCellSet || (preloadPlayer && targetCell)))
                {
                    objCell->SetActivity(true);
                    objCell->LoadCellData(sp);
                } else if (!_CellActive(posX, posY, MapInstance::ActiveCellRange) && objCell->IsActive()) // Cell is no longer active
                    objCell->SetActivity(false);
            }
        }
    }
}

float MapInstance::GetWalkableHeight(WorldObject *obj, float x, float y, float z, float destZ)
{
    uint32 wmoID;
    float groundHeight = NO_WMO_HEIGHT, liquidHeight = NO_WATER_HEIGHT;
    // Holes are used for WMO placement
    bool isHole = false;//mgr->GetADTIsHole(x, y);
    // Grab our ADT ground height before WMO checks
    float ADTHeight = GetADTLandHeight(x, y);
    uint16 adtLiqType; // Grab our ADT liquid height before WMO checks
    float ADTLiquid = GetADTWaterHeight(x, y, adtLiqType);
    // Grab our wmo height values
    sVMapInterface.GetWalkableHeight(this, _mapId, x, y, z, wmoID, groundHeight, liquidHeight);
    // Ground height, works pretty well
    if(groundHeight == NO_WMO_HEIGHT)
    {
        groundHeight = ADTHeight - 5.f;
        if(groundHeight > z && groundHeight > destZ)
            groundHeight = std::max<float>(z, destZ);
        else groundHeight += 5.f;
    }
    else if(groundHeight > destZ && ADTHeight < groundHeight && ADTHeight < destZ)
        groundHeight = ADTHeight;

    // Liquid heights, needs more work | Don't use ADT height at holes or when under ADT height | TODO: Buildings underwater that cut off ADT liquid
    if(liquidHeight == NO_WMO_HEIGHT && (isHole || (groundHeight != ADTHeight && z < ADTHeight)))
        liquidHeight = NO_WATER_HEIGHT;
    else if(liquidHeight == NO_WMO_HEIGHT || (groundHeight == ADTHeight))
        liquidHeight = ADTLiquid;

    groundHeight += 0.75f;
    if(liquidHeight != NO_WATER_HEIGHT)
    {   // We have liquid height
        if(obj->IsUnit() && castPtr<Unit>(obj)->canSwim())
        {
            if(z >= groundHeight)
            {
                if(z < liquidHeight)
                    return z; // We can return our z height
                return liquidHeight + 0.35f;
            }
            return groundHeight;
        }
    }

    return groundHeight;
}

uint16 MapInstance::GetADTAreaId(float x, float y)
{
    return GetBaseMap()->GetAreaID(x, y);
}

float MapInstance::GetADTLandHeight(float x, float y)
{
    return GetBaseMap()->GetLandHeight(x, y);
}

float MapInstance::GetADTWaterHeight(float x, float y, uint16 &outType)
{
    outType = GetBaseMap()->GetWaterType(x, y);
    return GetBaseMap()->GetWaterHeight(x, y);
}

void MapInstance::AddForcedCell(MapCell * c, uint32 range)
{
    c->SetPermanentActivity(true);
    UpdateCellActivity(c->GetPositionX(), c->GetPositionY(), range);
}

void MapInstance::RemoveForcedCell(MapCell * c, uint32 range)
{
    c->SetPermanentActivity(false);
    UpdateCellActivity(c->GetPositionX(), c->GetPositionY(), range);
}

bool MapInstance::IsFullRangeObject(WorldObject *obj)
{
    if(m_zoneFullRangeObjects.find(obj) != m_zoneFullRangeObjects.end())
        return true;
    if(m_areaFullRangeObjects.find(obj) != m_areaFullRangeObjects.end())
        return true;
    return false;
}

MapCell *MapInstance::GetCellOrInit(uint32 x, uint32 y, bool shouldInit, bool priority)
{
    MapCell *mapCell = GetCell(x, y);
    if(mapCell != NULL || shouldInit == false)
        return mapCell;

    CellSpawns *spawns = _map->GetSpawnsList( x , y );
    if(spawns == NULL && priority == false)
        return NULL;

    if((mapCell = Create(x, y)) == NULL)
        return NULL;

    mapCell->Init(x, y, _mapId, this);
    mapCell->SetActivity(true);
    if(spawns) mapCell->LoadCellData(spawns);
    return mapCell;
}

bool MapInstance::_CellActive(uint32 x, uint32 y, int radius)
{
    uint32 endX = ((x+radius) <= _sizeX) ? x + radius : (_sizeX-1);
    uint32 endY = ((y+radius) <= _sizeY) ? y + radius : (_sizeY-1);
    uint32 startX = x > radius ? x - radius : 0;
    uint32 startY = y > radius ? y - radius : 0;
    uint32 posX, posY;

    MapCell *objCell;
    for (posX = startX; posX <= endX; posX++ )
    {
        for (posY = startY; posY <= endY; posY++ )
        {
            objCell = GetCell(posX, posY);
            if (objCell && (objCell->HasPlayers() || objCell->IsForcedActive()))
                return true;
        }
    }
    return false;
}

void MapInstanceInRangeTargetCallback::operator()(WorldObject *obj, WorldObject *curObj)
{   // Should work here
    Unit *unitTarget = curObj->IsUnit() ? castPtr<Unit>(curObj) : NULL;
    if(unitTarget == NULL || unitTarget->isDead()) // Cut down on checks by skipping dead creatures
        return;

    float distance = obj->GetDistanceSq(unitTarget);
    if(_result && _resultDist <= distance)
        return;
    // Can't aggro on flying targets
    if(unitTarget->CheckFlightStatus(4.5f) && !castPtr<Creature>(obj)->canFly())
        return;

    uint32 latency = 0;
    // Visibility and interaction checking
    if(unitTarget->IsPlayer())
    {
        if(!castPtr<Player>(unitTarget)->IsVisible(obj))
            return;
        // Pull latency for bound box calcs while we're here
        if(WorldSession *session = castPtr<Player>(unitTarget)->GetSession())
            latency = session->GetLatency();
    } else if(!_instance->canObjectsInteract(obj, curObj))
        return;
    // Interaction limitation
    if(sFactionSystem.IsInteractionLocked(obj, curObj))
        return;
    // Check our aggro range against our saved range
    float aggroRange = unitTarget->ModDetectedRange(castPtr<Unit>(obj), _range);
    aggroRange *= aggroRange; // Distance is squared so square our range
    if(distance >= aggroRange)
        return;
    if(!obj->GetBoundBox()->IsInRange(_msTime, obj->GetPosition(), aggroRange, latency))
        return;

    if(!sFactionSystem.isHostile(castPtr<Unit>(obj), unitTarget))
        return;
    // LOS is a big system hit so do it last
    if(!obj->IsInLineOfSight(unitTarget))
        return;

    _result = unitTarget;
    _resultDist = distance;
}

Unit *MapInstance::FindInRangeTarget(uint32 msTime, Creature *ctr, float range, uint32 typeMask)
{
    // Acquire our storage for this thread
    InrangeTargetCallbackStack::callbackStorage *storage = _inRangeTargetCBStack.getOrAllocateCallback(RONIN_UTIL::GetThreadId(), this);
    ASSERT(storage != NULL);

    std::vector<uint16> phaseSet;
    ctr->BuildPhaseSet(&phaseSet);
    storage->callback.ResetData(msTime, range);
    ctr->GetCellManager()->CreateCellRange(&storage->cellvector, range);
    std::for_each(storage->cellvector.begin(), storage->cellvector.end(), [this, ctr, typeMask, storage, phaseSet](uint32 cellId)
    {
        std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(cellId);
        if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
            cell->ProcessObjectSets(ctr, &storage->callback, &phaseSet, typeMask);
    });

    Unit *Result = storage->callback.GetResult();
    storage->cellvector.clear();
    return Result;
}

void MapInstanceBroadcastMessageCallback::operator()(WorldObject *obj, WorldObject *curObj)
{
    if(!curObj->IsPlayer())
        return;
    Player *curPlr = castPtr<Player>(curObj);
    if(!curPlr->IsVisible(obj))
        return;
    curPlr->PushPacket(_packet, false);
}

void MapInstance::SendMessageToCellPlayers(WorldObject* obj, WorldPacket * packet, uint32 cell_radius /* = 2 */)
{
    // Acquire our storage for this thread
    BroadcastMessageCallbackStack::callbackStorage *storage = _broadcastMessageCBStack.getOrAllocateCallback(RONIN_UTIL::GetThreadId(), this);
    ASSERT(storage != NULL);

    std::vector<uint16> phaseSet;
    obj->BuildPhaseSet(&phaseSet);
    storage->callback.setPacketData(packet);
    obj->GetCellManager()->CreateCellRange(&storage->cellvector, cell_radius);
    std::for_each(storage->cellvector.begin(), storage->cellvector.end(), [this, obj, storage, phaseSet](uint32 cellId)
    {
        std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(cellId);
        if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
            cell->ProcessObjectSets(obj, &storage->callback, &phaseSet, TYPEMASK_TYPE_PLAYER);
    });

    storage->cellvector.clear();
}

void MapInstanceBroadcastMessageInrangeCallback::operator()(WorldObject *obj, WorldObject *curObj)
{
    float range = 0.f;
    Player *playerObj = castPtr<Player>(curObj);
    if(!playerObj->IsVisible(obj))
        return;
    if(_range > 1.f && !_instance->IsInRange(_range, obj, curObj, range))
        return;
    if(_myTeam && playerObj->GetTeam() != _teamId)
        return;

    if(_packet != NULL)
        playerObj->PushPacket(_packet, true);
    else if(WorldSession *session = playerObj->GetSession())
        session->OutPacket(_opcode, _dataLen, _dataStream);
}

void MapInstance::MessageToCells(WorldObject *obj, uint16 opcodeId, uint16 Len, const void *data, float range)
{
    // Acquire our storage for this thread
    BroadcastMessageInRangeCallbackStack::callbackStorage *storage = _broadcastMessageInRangeCBStack.getOrAllocateCallback(RONIN_UTIL::GetThreadId(), this);
    ASSERT(storage != NULL);

    std::vector<uint16> phaseSet;
    obj->BuildPhaseSet(&phaseSet);
    storage->callback.ResetData(range, opcodeId, Len, data, false, 0);
    obj->GetCellManager()->CreateCellRange(&storage->cellvector, range);
    std::for_each(storage->cellvector.begin(), storage->cellvector.end(), [this, obj, storage, phaseSet](uint32 cellId)
    {
        std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(cellId);
        if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
            cell->ProcessObjectSets(obj, &storage->callback, &phaseSet, TYPEMASK_TYPE_PLAYER);
    });

    storage->cellvector.clear();
}

void MapInstance::MessageToCells(WorldObject *obj, WorldPacket *data, float range, bool myTeam, uint32 teamId)
{
    // Acquire our storage for this thread
    BroadcastMessageInRangeCallbackStack::callbackStorage *storage = _broadcastMessageInRangeCBStack.getOrAllocateCallback(RONIN_UTIL::GetThreadId(), this);
    ASSERT(storage != NULL);

    std::vector<uint16> phaseSet;
    obj->BuildPhaseSet(&phaseSet);
    storage->callback.ResetData(range, data, myTeam, teamId);
    obj->GetCellManager()->CreateCellRange(&storage->cellvector, range);
    std::for_each(storage->cellvector.begin(), storage->cellvector.end(), [this, obj, storage, phaseSet](uint32 cellId)
    {
        std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(cellId);
        if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
            cell->ProcessObjectSets(obj, &storage->callback, &phaseSet, TYPEMASK_TYPE_PLAYER);
    });

    storage->cellvector.clear();
}

void MapInstanceBroadcastChatPacketCallback::operator()(WorldObject *obj, WorldObject *curObj)
{
    if(!curObj->IsPlayer())
        return;
    Player *curPlr = castPtr<Player>(curObj);
    if(!curPlr->IsVisible(obj))
        return;

    if(WorldSession *session = curPlr->GetSession())
        session->SendChatPacket(_packet, _defaultLang, _langPos, _guidPos);
}

void MapInstance::SendChatMessageToCellPlayers(WorldObject* obj, WorldPacket *packet, uint32 cell_radius, int32 lang, uint32 langpos, uint32 guidPos)
{
    // Acquire our storage for this thread
    BroadcastChatPacketCallbackStack::callbackStorage *storage = _broadcastChatPacketCBStack.getOrAllocateCallback(RONIN_UTIL::GetThreadId(), this);
    ASSERT(storage != NULL);

    std::vector<uint16> phaseSet;
    obj->BuildPhaseSet(&phaseSet);
    storage->callback.setPacketData(packet, lang, langpos, guidPos);
    obj->GetCellManager()->CreateCellRange(&storage->cellvector, cell_radius);
    std::for_each(storage->cellvector.begin(), storage->cellvector.end(), [this, obj, storage, phaseSet](uint32 cellId)
    {
        std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(cellId);
        if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
            cell->ProcessObjectSets(obj, &storage->callback, &phaseSet, TYPEMASK_TYPE_PLAYER);
    });

    storage->cellvector.clear();
}

void MapInstanceBroadcastObjectUpdateCallback::operator()(WorldObject *obj, WorldObject *curObj)
{
    Player *plrTarget = curObj->IsPlayer() ? castPtr<Player>(curObj) : NULL;
    if(plrTarget == NULL || !plrTarget->IsVisible(obj))
        return; // Make sure that the target player can see us.

    uint32 targetFlag = obj->GetUpdateFlag(plrTarget);
    if(targetFlag & UF_FLAG_PARTY_MEMBER)
    {
        if(uint32 count = obj->BuildValuesUpdateBlockForPlayer(&m_callbackBuffer, plrTarget, UF_FLAGMASK_PARTY_MEMBER))
        {
            plrTarget->PushUpdateBlock(_instance->GetMapId(), &m_callbackBuffer, count);
            m_callbackBuffer.clear();
        }
    }
    else if(targetFlag & UF_FLAG_OWNER)
    {
        if(uint32 count = obj->BuildValuesUpdateBlockForPlayer(&m_callbackBuffer, plrTarget, UF_FLAGMASK_OWN_PET))
        {
            plrTarget->PushUpdateBlock(_instance->GetMapId(), &m_callbackBuffer, count);
            m_callbackBuffer.clear();
        }
    }
    else if(uint32 count = obj->BuildValuesUpdateBlockForPlayer(&m_callbackBuffer, plrTarget, UF_FLAGMASK_PUBLIC))
    {
        plrTarget->PushUpdateBlock(_instance->GetMapId(), &m_callbackBuffer, count);
        m_callbackBuffer.clear();
    }
}

void MapInstance::BroadcastObjectUpdate(WorldObject *obj)
{
    // Acquire our storage for this thread
    BroadcastObjectUpdateCallbackStack::callbackStorage *storage = _broadcastObjectUpdateCBStack.getOrAllocateCallback(RONIN_UTIL::GetThreadId(), this);
    ASSERT(storage != NULL);

    std::vector<uint16> phaseSet;
    obj->BuildPhaseSet(&phaseSet);
    obj->GetCellManager()->FillCellRange(&storage->cellvector);
    std::for_each(storage->cellvector.begin(), storage->cellvector.end(), [this, obj, storage, phaseSet](uint32 cellId)
    {
        std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(cellId);
        if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
            cell->ProcessObjectSets(obj, &storage->callback, &phaseSet, TYPEMASK_TYPE_PLAYER);
    });

    storage->cellvector.clear();
}

void MapInstance::UpdateObjectCellVisibility(WorldObject *obj, std::vector<uint32> *cellVector)
{
    for(auto itr = cellVector->begin(); itr != cellVector->end(); itr++)
    {
        std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(*itr);
        // We should skip the cell even if we're a player since a loading cell will lack data, but it is unlikely to happen anyway
        if(!obj->IsPlayer() && IsCellLoading(cellPair.first, cellPair.second))
            continue;

        UpdateCellData(obj, cellPair.first, cellPair.second, obj->IsPlayer(), true);
    }

    // Zone wide visibility
    if(!obj->IsPlayer())
        return;
    UpdateAreaAndZoneVisibility(castPtr<Player>(obj));
}

void MapInstance::UpdateAreaAndZoneVisibility(Player *plr)
{
    uint32 areaId = plr->GetAreaId(), zoneId = plr->GetZoneId();
    if(m_fullRangeObjectsByZone.find(zoneId) != m_fullRangeObjectsByZone.end())
    {
        for(auto itr = m_fullRangeObjectsByZone[zoneId].begin(); itr != m_fullRangeObjectsByZone[zoneId].end(); ++itr)
        {
            if(!(*itr)->IsActivated())
                continue;

            if( canObjectsInteract(plr, *itr) && plr->CanSee(*itr) && !plr->IsVisible(*itr) )
            {
                plr->AddVisibleObject(*itr);
                (*itr)->GetCellManager()->AddVisibleBy(plr->GetGUID());

                m_dataBufferLock.Acquire();
                if(uint32 count = (*itr)->BuildCreateUpdateBlockForPlayer( &m_dataBuffer, plr ))
                {
                    plr->PushUpdateBlock(GetMapId(), &m_dataBuffer, count);
                    if((*itr)->IsUnit() && castPtr<Unit>(*itr)->GetMovementInterface()->GetPath()->hasDestination())
                        castPtr<Unit>(*itr)->GetMovementInterface()->GetPath()->SendMovementPacket(plr, MOVEBCFLAG_DELAYED);
                }
                m_dataBuffer.clear();
                m_dataBufferLock.Release();
            }
        }
    }

    if(m_fullRangeObjectsByArea.find(zoneId) != m_fullRangeObjectsByArea.end())
    {
        for(auto itr = m_fullRangeObjectsByArea[areaId].begin(); itr != m_fullRangeObjectsByArea[areaId].end(); ++itr)
        {
            if(!(*itr)->IsActivated())
                continue;

            if( canObjectsInteract(plr, *itr) && plr->CanSee(*itr) && !plr->IsVisible(*itr) )
            {
                plr->AddVisibleObject(*itr);
                (*itr)->GetCellManager()->AddVisibleBy(plr->GetGUID());

                m_dataBufferLock.Acquire();
                if(uint32 count = (*itr)->BuildCreateUpdateBlockForPlayer( &m_dataBuffer, plr ))
                {
                    plr->PushUpdateBlock(GetMapId(), &m_dataBuffer, count);
                    if((*itr)->IsUnit() && castPtr<Unit>(*itr)->GetMovementInterface()->GetPath()->hasDestination())
                        castPtr<Unit>(*itr)->GetMovementInterface()->GetPath()->SendMovementPacket(plr, MOVEBCFLAG_DELAYED);
                }
                m_dataBuffer.clear();
                m_dataBufferLock.Release();
            }
        }
    }
}

void MapInstanceDynamicObjectTargetMappingCallback::operator()(WorldObject *obj, WorldObject *curObj)
{
    if((obj == curObj) || (curObj->IsUnit() && !castPtr<Unit>(curObj)->isAlive()))
        return;
    float dist = _dynObject->GetDistanceSq(curObj);
    if(dist < _minRange || dist > _maxRange)
        return;
    if(!_instance->canObjectsInteract(obj, curObj))
        return;
    if(sFactionSystem.IsInteractionLocked(obj, curObj, true))
        return;

    (*_callback)(_dynObject, castPtr<Unit>(obj), castPtr<Unit>(curObj), dist);
}

void MapInstance::HandleDynamicObjectRangeMapping(DynamicObjectTargetCallback *callback, DynamicObject *object, Unit *caster, float minRange, float maxRange, uint32 typeMask)
{
    // Acquire our storage for this thread
    DynamicObjectTargetMappingCallbackStack::callbackStorage *storage = _dynamicObjectTargetMappingCBStack.getOrAllocateCallback(RONIN_UTIL::GetThreadId(), this);
    ASSERT(storage != NULL);

    std::vector<uint16> phaseSet;
    object->BuildPhaseSet(&phaseSet);
    storage->callback.SetData(callback, object, caster, minRange*minRange, maxRange*maxRange);
    ObjectCellManager::ConstructCellData(object->GetPositionX(), object->GetPositionY(), maxRange, &storage->cellvector);
    std::for_each(storage->cellvector.begin(), storage->cellvector.end(), [this, object, typeMask, storage, phaseSet](uint32 cellId)
    {
        std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(cellId);
        if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
            cell->ProcessObjectSets(object, &storage->callback, &phaseSet, typeMask);
    });

    storage->cellvector.clear();
}

void MapInstanceSpellTargetMappingCallback::operator()(WorldObject *obj, WorldObject *curObj)
{
    if(curObj->IsUnit() && !castPtr<Unit>(curObj)->isAlive())
        return;
    float dist = curObj->GetDistanceSq(_x, _y, _z);
    if(dist < _minRange || dist > _maxRange)
        return;
    if(!_instance->canObjectsInteract(obj, curObj))
        return;
    if(sFactionSystem.IsInteractionLocked(obj, curObj, true))
        return;

    (*_callback)(_spell, _effIndex, _targetType, curObj);
}

void MapInstance::HandleSpellTargetMapping(MapTargetCallback *callback, SpellTargetClass *spell, uint32 i, uint32 targetType, float x, float y, float z, float minRange, float maxRange, uint32 typeMask)
{
    // Acquire our storage for this thread
    SpellTargetMappingCallbackStack::callbackStorage *storage = _spellTargetMappingCBStack.getOrAllocateCallback(RONIN_UTIL::GetThreadId(), this);
    ASSERT(storage != NULL);
    Unit *caster = spell->GetCaster();
    if(caster == NULL)
        return;

    std::vector<uint16> phaseSet;
    caster->BuildPhaseSet(&phaseSet);
    storage->callback.SetData(callback, spell, i, targetType, x, y, z, minRange*minRange, maxRange*maxRange);
    ObjectCellManager::ConstructCellData(x, y, maxRange, &storage->cellvector);
    std::for_each(storage->cellvector.begin(), storage->cellvector.end(), [this, caster, spell, typeMask, storage, phaseSet](uint32 cellId)
    {
        std::pair<uint16, uint16> cellPair = ObjectCellManager::unPack(cellId);
        if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
            cell->ProcessObjectSets(caster, &storage->callback, &phaseSet, typeMask);
    });

    storage->cellvector.clear();
}

void MapInstanceProcessItemUpdateCallback::Process(Player *plr, Item *item)
{
    if(uint32 count = item->BuildValuesUpdateBlockForPlayer(&m_callbackBuffer, plr, 0xFFFF))
        plr->PushUpdateBlock(plr->GetMapId(), &m_callbackBuffer, count);
    m_callbackBuffer.clear();
}

void MapInstance::HandleItemUpdateRequest(Player *plr, Item *item)
{
    if(ProcessItemUpdateCallbackStack::callbackStorage *storage = _processItemUpdateCBStack.getOrAllocateCallback(RONIN_UTIL::GetThreadId(), this))
        ((MapInstanceProcessItemUpdateCallback*)storage)->Process(plr, item);
}

bool MapInstance::UpdateQueued(WorldObject *obj)
{
    bool res = false;
    m_updateMutex.Acquire();
    if(_updates.find(obj) != _updates.end())
        res = true;
    m_updateMutex.Release();
    return res;
}

void MapInstance::ObjectUpdated(WorldObject *obj)
{
    m_updateMutex.Acquire();
    if(_updates.find(obj) == _updates.end())
        _updates.insert(obj);
    m_updateMutex.Release();
}

void MapInstance::ObjectLocationChange(WorldObject *obj)
{
    m_updateMutex.Acquire();
    if(obj->IsPlayer())
    {
        Player *plr = castPtr<Player>(obj);
        if(_movedPlayers.find(plr) == _movedPlayers.end())
            _movedPlayers.insert(plr);
    } else if(_movedObjects.find(obj) == _movedObjects.end())
        _movedObjects.insert(obj);
    m_updateMutex.Release();
}

void MapInstance::PushToProcessed(Player* plr)
{
    _processQueue.insert(plr);
}

void MapInstance::ChangeFarsightLocation(Player* plr, Unit* farsight, bool apply)
{

}

void MapInstance::ChangeFarsightLocation(Player* plr, float X, float Y, bool apply)
{

}

void MapInstance::BeginInstanceExpireCountdown()
{
    WorldPacket data(SMSG_RAID_GROUP_ONLY, 8);

    // send our sexy packet
    data << uint32(60000) << uint32(1);

    Player* ptr; // Update all players on map.
    __player_iterator = m_PlayerStorage.begin();
    for(; __player_iterator != m_PlayerStorage.end();)
    {
        ptr = __player_iterator->second;;
        ++__player_iterator;

        if(!ptr->raidgrouponlysent)
            ptr->PushPacket(&data);
    }

    // set our expire time to 60 seconds.
    InactiveMoveTime = UNIXTIME + 60;
}

void MapInstance::AddObject(WorldObject* obj)
{
    m_objectinsertlock.Acquire();
    m_objectinsertpool.insert(obj);
    m_objectinsertlock.Release();
}

uint32 MapInstance::IsFull(PlayerInfo *info)
{
    if(!m_PlayerStorage.empty())
    {
        if(m_PlayerStorage.size() >= GetdbcMap()->maxPlayers)
            return INSTANCE_ABORT_FULL;
        if(m_instanceData && m_instanceData->linkedGroupId)
            if(info->m_Group == NULL || info->m_Group->GetID() != m_instanceData->linkedGroupId)
                return INSTANCE_ABORT_CREATE_NEW_INSTANCE;
    }

    return INSTANCE_OK;
}

void MapInstance::AddZoneVisibleSpawn(uint32 zoneId, WorldObject *obj)
{
    m_fullRangeObjectsByZone[zoneId].push_back(obj);
    m_zoneFullRangeObjects.insert(std::make_pair(obj, zoneId));
}

void MapInstance::RemoveZoneVisibleSpawn(uint32 zoneId, WorldObject *obj)
{
    std::vector<WorldObject*>::iterator itr;
    if((itr = std::find(m_fullRangeObjectsByZone[zoneId].begin(), m_fullRangeObjectsByZone[zoneId].end(), obj)) != m_fullRangeObjectsByZone[zoneId].end())
        m_fullRangeObjectsByZone[zoneId].erase(itr);
    m_zoneFullRangeObjects.erase(obj);
}

void MapInstance::AddAreaVisibleSpawn(uint32 areaId, WorldObject *obj)
{
    m_fullRangeObjectsByArea[areaId].push_back(obj);
    m_areaFullRangeObjects.insert(std::make_pair(obj, areaId));
}

void MapInstance::RemoveAreaVisibleSpawn(uint32 areaId, WorldObject *obj)
{
    std::vector<WorldObject*>::iterator itr;
    if((itr = std::find(m_fullRangeObjectsByArea[areaId].begin(), m_fullRangeObjectsByArea[areaId].end(), obj)) != m_fullRangeObjectsByArea[areaId].end())
        m_fullRangeObjectsByArea[areaId].erase(itr);
    m_areaFullRangeObjects.erase(obj);
}

Unit* MapInstance::GetUnit(WoWGuid guid)
{
    switch(guid.getHigh())
    {
    case HIGHGUID_TYPE_PLAYER: return GetPlayer(guid);
    case HIGHGUID_TYPE_VEHICLE:
    case HIGHGUID_TYPE_UNIT:
        return GetCreature(guid);
    }

    return NULL;
}

WorldObject* MapInstance::_GetObject(WoWGuid guid)
{
    switch(guid.getHigh())
    {
    case HIGHGUID_TYPE_GAMEOBJECT: return GetGameObject(guid);
    case HIGHGUID_TYPE_DYNAMICOBJECT: return GetDynamicObject(guid);
    case HIGHGUID_TYPE_MO_TRANSPORT: return GetTransporter(guid);
    case HIGHGUID_TYPE_CORPSE: return objmgr.GetCorpse(guid.getLow());
    }
    return GetUnit(guid);
}

WorldObject* MapInstance::GetObjectClosestToCoords(uint32 entry, float x, float y, float z, float ClosestDist, int32 forcedtype)
{
    MapCell * pCell = GetCell(GetPosX(x), GetPosY(y));
    if(pCell == NULL)
        return NULL;

    WorldObject* ClosestObject = NULL;
    /*float CurrentDist = 0;
    MapCell::CellObjectSet::const_iterator iter;
    for(iter = pCell->Begin(); iter != pCell->End(); iter++)
    {
        CurrentDist = (*iter)->GetDistanceSq(x, y, (z != 0.0f ? z : (*iter)->GetPositionZ()));
        if(CurrentDist < ClosestDist)
        {
            if(forcedtype > -1 && forcedtype != (*iter)->GetTypeId())
                continue;

            if((entry && (*iter)->GetEntry() == entry) || !entry)
            {
                ClosestDist = CurrentDist;
                ClosestObject = (*iter);
            }
        }
    }*/

    return ClosestObject;
}

bool MapInstance::CanUseCollision(WorldObject* obj)
{
    if(GetBaseMap()->IsCollisionEnabled())
    {
        uint32 tileX = (GetPosX(obj->GetPositionX())/CellsPerTile);
        uint32 tileY = (GetPosY(obj->GetPositionY())/CellsPerTile);
        if(sVMapInterface.IsActiveTile(_mapId, tileX, tileY))
            return true;
    }

    return false;
}

void MapInstance::_ProcessInputQueue(uint32 msTime)
{
    if(!m_objectinsertlock.AttemptAcquire())
        return;
    for(ObjectSet::iterator i = m_objectinsertpool.begin(); i != m_objectinsertpool.end(); i++)
        (*i)->PushToWorld(this, msTime);
    m_objectinsertpool.clear();
    m_objectinsertlock.Release();
}

void MapInstance::_PerformScriptUpdates(uint32 msTime, uint32 uiDiff)
{
    if(InstanceData *data = m_iData)
        data->Update(msTime);

    // UPDATE THE SCRIPT, DO IT.
    if(MapScript *script = m_script)
        script->Update(msTime, uiDiff);

    // Update map conditions
    //std::vector<uint32> finishedConditions;
    //for(auto itr = m_conditionTimers.begin(); itr != m_conditionTimers.end(); itr++)
    // if(itr->second->update(uiDiff))
    //   finishedConditions.push_back(itr->first);
    // Now clean up conditions based on filled vector

    // 
}

void MapInstance::_PerformCombatUpdates(uint32 msTime, uint32 uiDiff)
{
    Guard guard(m_combatLock);
    if(m_combatTimers.empty())
        return;

    std::vector<std::pair<WoWGuid, WoWGuid>> deletionQueue;
    // Find combats that have cleared up
    for(std::map<std::pair<WoWGuid, WoWGuid>, uint32>::iterator itr = m_combatTimers.begin(); itr != m_combatTimers.end(); itr++)
    {
        bool toDelete = false;
        itr->second -= std::min<uint32>(itr->second, uiDiff);
        Unit *unit1 = GetUnit(itr->first.first), *unit2 = GetUnit(itr->first.second);
        if(unit1 == NULL || unit2 == NULL)
            toDelete = true;
        else if(!(unit1->checkAttackTarget(itr->first.second) || unit2->checkAttackTarget(itr->first.first)))
            toDelete = true;
        else if(itr->second == 0)
            toDelete = true;
        if(toDelete == false)
            continue;
        // Queue our combat into deletion queue
        deletionQueue.push_back(itr->first);
    }

    // Cleaning up finished combat
    std::for_each(deletionQueue.begin(), deletionQueue.end(), [this](std::pair<WoWGuid, WoWGuid> pair)
    {
        // Remove our combatants from our partner lists
        m_combatPartners[pair.first].erase(pair.second);
        m_combatPartners[pair.second].erase(pair.first);
        // Clean up our combat partner list
        if(m_combatPartners[pair.first].empty())
            m_combatPartners.erase(pair.first);
        if(m_combatPartners[pair.second].empty())
            m_combatPartners.erase(pair.second);
        m_combatTimers.erase(pair);
    });

    deletionQueue.clear();
}

void MapInstance::_PerformPlayerUpdates(uint32 msTime, uint32 uiDiff)
{
    // Update player trades
    for(auto itr = m_activeTradeData.begin(); itr != m_activeTradeData.end(); ++itr)
        UpdateTrade(*itr);

    for(auto itr = m_tradeDeleteQueue.begin(); itr != m_tradeDeleteQueue.end(); ++itr)
        delete *itr;
    m_tradeDeleteQueue.clear();

    // Now we process player updates, there is no pool as all players are constantly in our update set
    if(!m_PlayerStorage.empty())
    {
        Player* ptr; // Update players.
        __player_iterator = m_PlayerStorage.begin();
        while( __player_iterator != m_PlayerStorage.end())
        {
            ptr = __player_iterator->second;
            ++__player_iterator;
            ptr->Update( msTime, uiDiff );
        }
    }
}

void MapInstance::_PerformCreatureUpdates(uint32 msTime, uint32 uiDiff)
{
    mCreaturePool.Update(msTime, uiDiff, _updatePool);
    mCreaturePool.ProcessRemovals();
}

void MapInstance::_PerformObjectUpdates(uint32 msTime, uint32 uiDiff)
{
    mGameObjectPool.Update(msTime, uiDiff, _updatePool);
    mGameObjectPool.ProcessRemovals();
}

void MapInstance::_PerformTransportUpdates(uint32 msTime, uint32 uiDiff)
{
    mTransporterPool.Update(msTime, uiDiff, NULL);
    mTransporterPool.ProcessRemovals();
}

void MapInstance::_PerformDynamicObjectUpdates(uint32 msTime, uint32 uiDiff)
{
    mDynamicObjectPool.Update(msTime, uiDiff, _updatePool);
    mDynamicObjectPool.ProcessRemovals();
}

void MapInstance::_PerformDelayedSpellUpdates(uint32 msTime, uint32 uiDiff)
{
    projectileSpellUpdateTime[0] += uiDiff;
    projectileSpellUpdateTime[1] += uiDiff;
    uint8 index = projectileSpellIndex[0]++;
    if(projectileSpellIndex[0] == 2)
        projectileSpellIndex[0] = 0;

    std::vector<Spell*> cleanupSet;
    for(std::set<Spell*>::iterator itr = m_projectileSpells[index].begin(); itr != m_projectileSpells[index].end(); itr++)
        if((*itr)->UpdateDelayedTargetEffects(this, projectileSpellUpdateTime[index]))
            cleanupSet.push_back(*itr);

    while(!cleanupSet.empty())
    {
        Spell *spell = *cleanupSet.begin();
        cleanupSet.erase(cleanupSet.begin());
        m_projectileSpells[index].erase(spell);
        spell->Destruct();
    }
    projectileSpellUpdateTime[index] = 0;
}

void MapInstance::_PerformUnitPathUpdates(uint32 msTime, uint32 uiDiff)
{
    mUnitPathPool.Update(msTime, uiDiff, _updatePool);
}

void MapInstance::_PerformMovementUpdates(bool includePlayers)
{
    m_updateMutex.Acquire();
    if(includePlayers && !_movedPlayers.empty())
    {
        for(auto itr = _movedPlayers.begin(); itr != _movedPlayers.end(); ++itr)
            ChangeObjectLocation(*itr);
        _movedPlayers.clear();
    }

    for(auto itr = _movedObjects.begin(); itr != _movedObjects.end(); ++itr)
        ChangeObjectLocation(*itr);
    _movedObjects.clear();
    m_updateMutex.Release();
}

void MapInstance::_PerformSessionUpdates()
{
    // Sessions are updated every loop.
    for(SessionSet::iterator itr = MapSessions.begin(), it2; itr != MapSessions.end();)
    {
        WorldSession *MapSession = (*itr);
        it2 = itr++;

        //we have teleported to another map, remove us here.
        if(MapSession->GetEventInstanceId() != m_instanceID)
        {
            MapSessions.erase(it2);
            continue;
        }

        // Session is deserted, just do garbage collection
        if(MapSession->GetPlayer() == NULL)
        {
            MapSessions.erase(it2);
            sWorld.DeleteSession(MapSession);
            continue;
        }

        // Don't update players not on our map.
        // If we abort in the handler, it means we will "lose" packets, or not process this.
        // .. and that could be diasterous to our client :P
        if( MapSession->GetPlayer()->GetMapInstance() == NULL ||
            MapSession->GetPlayer()->GetMapInstance() != this)
            continue;

        if(int result = MapSession->Update(m_instanceID))//session or socket deleted?
        {
            MapSessions.erase(it2);
            if(result == 1)//socket don't exist anymore, delete from both world- and map-sessions.
                sWorld.DeleteSession(MapSession);
        }
    }
}

void MapInstance::_PerformPendingUpdates()
{
    m_updateMutex.Acquire();
    if(!_updates.size() && !_processQueue.size())
    {
        m_updateMutex.Release();
        return;
    }

    uint32 count = 0;
    WorldObject *wObj;
    PlayerSet m_partyTargets, m_petTargets;
    for(ObjectSet::iterator iter = _updates.begin(); iter != _updates.end();)
    {
        wObj = *iter;
        ++iter;
        if(wObj == NULL || !wObj->IsInWorld())
            continue;

        // players have to receive their own updates ;)
        if( wObj->IsPlayer() )
        {
            m_dataBufferLock.Acquire(); // need to be different! ;)
            if( count = wObj->BuildValuesUpdateBlockForPlayer(&m_dataBuffer, castPtr<Player>( wObj ), UF_FLAGMASK_SELF) )
                castPtr<Player>( wObj )->PushUpdateBlock(_mapId, &m_dataBuffer, count );
            m_dataBuffer.clear();
            m_dataBufferLock.Release();
        }

        wObj->OnUpdateProcess();
        BroadcastObjectUpdate(wObj);
        wObj->ClearUpdateMask();
    }
    _updates.clear();

    Player* plyr;
    // generate pending a9packets and send to clients.
    while(!_processQueue.empty())
    {
        plyr = *_processQueue.begin();
        if(plyr && plyr->GetMapInstance() == this)
            plyr->PopPendingUpdates(_mapId, true);
        _processQueue.erase(_processQueue.begin());
    }
    m_updateMutex.Release();
}

void MapInstance::_PerformPendingActions()
{
    m_updateMutex.Acquire();
    while(!m_pendingCellActions.empty())
    {
        std::pair<uint16, uint16> pair = *m_pendingCellActions.begin();
        m_pendingCellActions.erase(m_pendingCellActions.begin());
        if(MapCell *cell = GetCell(pair.first, pair.second))
            cell->ProcessPendingActions();
    }

    mCreaturePool.ProcessRemovals();
    mGameObjectPool.ProcessRemovals();
    mTransporterPool.ProcessRemovals();
    mDynamicObjectPool.ProcessRemovals();
    mUnitPathPool.ProcessRemovals();

    while(!_pendingRemoval.empty())
    {
        WorldObject *obj = *_pendingRemoval.begin();
        _pendingRemoval.erase(_pendingRemoval.begin());
        obj->RemoveFromWorld();
    }

    while(!_pendingCleanup.empty())
    {
        WorldObject *obj = *_pendingCleanup.begin();
        _pendingCleanup.erase(_pendingCleanup.begin());
        if(obj->IsInWorld())
            obj->RemoveFromWorld();
        obj->Destruct();
    }

    while(!_softDCPlayers.empty())
    {
        Player *plObj = *_softDCPlayers.begin();
        _softDCPlayers.erase(_softDCPlayers.begin());
        plObj->ForceLogout();
    }

    while(!m_removeQueue.empty())
    {
        Player *plObj = m_removeQueue.front();
        m_removeQueue.pop();

        // Clear any updates pending
        _processQueue.erase(plObj);
        _movedObjects.erase(plObj);
        _softDCPlayers.erase(plObj);
        _updates.erase(plObj);

        plObj->PopPendingUpdates(_mapId, true);

        // If it's a player and he's inside boundaries - update his nearby cells
        if(plObj->GetPositionX() <= _maxX && plObj->GetPositionX() >= _minX &&
            plObj->GetPositionY() <= _maxY && plObj->GetPositionY() >= _minY)
        {
            uint32 x = GetPosX(plObj->GetPositionX());
            uint32 y = GetPosY(plObj->GetPositionY());
            UpdateCellActivity(x, y, MapInstance::ActiveCellRange);
        }
        m_PlayerStorage.erase( plObj->GetGUID() );

        // If we have no session, then clean up the player pointer
        if(plObj->GetSession() == NULL)
            plObj->Destruct();
        else
        {
            // Setting an instance ID here will trigger the session to be removed
            // by MapInstance::run(). :)
            plObj->GetSession()->SetEventInstanceId(-1);

            // Add it to the global session set (if it's not being deleted).
            if(!plObj->GetSession()->bDeleted)
                sWorld.AddGlobalSession(plObj->GetSession());
        }
    }
    m_updateMutex.Release();
}

void MapInstance::EventCorpseDespawn(uint64 guid)
{
    objmgr.DespawnCorpse(guid);
}

void MapInstance::TeleportPlayers()
{
    if(!bServerShutdown)
    {
        // Update all players on map.
        __player_iterator = m_PlayerStorage.begin();
        Player* ptr;
        for(; __player_iterator != m_PlayerStorage.end();)
        {
            ptr = __player_iterator->second;;
            ++__player_iterator;

            if(ptr->GetSession() && !ptr->EjectFromInstance())
                ptr->TeleportToHomebind();
        }
    }
    else
    {
        // Update all players on map.
        __player_iterator = m_PlayerStorage.begin();
        Player* ptr;
        for(; __player_iterator != m_PlayerStorage.end();)
        {
            ptr = __player_iterator->second;;
            ++__player_iterator;

            if(ptr->GetSession())
                ptr->GetSession()->LogoutPlayer();
            else
            {
                ptr->Destruct();
                ptr = NULL;
                m_PlayerStorage.erase(__player_iterator);
            }
        }
    }
}

void MapInstance::UnloadCell(uint32 x, uint32 y)
{
    MapCell * c = GetCell(x,y);
    if(c == NULL || c->HasPlayers() || _CellActive(x,y,MapInstance::ActiveCellRange) || !c->IsUnloadPending())
        return;

    sLog.Debug("MapInstance","Unloading Cell [%d][%d] on map %d (instance %d)...", x, y, _mapId, m_instanceID);
    c->Unload();
}

bool MapInstance::canObjectsInteract(WorldObject *obj, WorldObject *curObj)
{
    if((obj->GetPhaseMask() & curObj->GetPhaseMask()) == 0)
        return false;
    // Cull inactive objects
    if(obj->IsActiveObject() && !obj->IsActivated())
        return false;
    if(curObj->IsActiveObject() && !curObj->IsActivated())
        return false;
    // Cull world triggers
    if(obj->IsCreature() && castPtr<Creature>(obj)->isWorldTrigger())
        return false;
    if(curObj->IsCreature() && castPtr<Creature>(curObj)->isWorldTrigger())
        return false;

    return true;
}

bool MapInstance::IsInRange(float fRange, WorldObject* obj, WorldObject *curObj, float &distOut)
{   // First distance check, are we in range?
    if((distOut = curObj->GetDistance2dSq(obj)) > (fRange*fRange))
        return false;
    return true;
}

bool MapInstance::InZRange(float fRange, WorldObject* obj, WorldObject *curObj)
{   // Check our z height diff
    float heightDifference = RONIN_UTIL::Diff(obj->GetPositionZ(), curObj->GetPositionZ());
    if((heightDifference*heightDifference) >= fRange)
        return false;
    return true;
}

void MapInstance::OnGroupEnter(Player *plr, Group *grp)
{
    if(m_instanceData == NULL || m_instanceData->linkedGroupId)
        return;

    if(sInstanceMgr.LinkGuidToInstance(this, grp->GetGuid(), false))
        m_instanceData->linkedGroupId = grp->GetID();
}

uint32 MapInstance::GetZoneModifier(uint32 zoneId)
{
    if(pdbcMap == 0)
        return 0;
    uint32 mapId = _mapId;
    if(pdbcMap->rootPhaseMap != -1)
        mapId = pdbcMap->rootPhaseMap;
    // Starting zones should be marked as 0 modifier
    if(mapId == 648 || mapId == 654)
        return 0; // Gilneas and The Lost Islands
    if(AreaTableEntry *areaTable = dbcAreaTable.LookupEntry(zoneId))
    {
        switch(areaTable->ZoneId)
        {   // Cataclysm zones in kalim/Ek
        case 616: // Mount Hyjal
        case 4922: // Twilight Highlands
        case 5034: // Uldum
        case 5042: // Deepholm
            return 3;
        }
    }
    return pdbcMap->addon;
}

void MapInstance::AppendQuestList(WoWGuid guid, Player *plr, uint32 &count, WorldPacket *packet)
{
    // Nothing yet
}

void MapInstance::SetUnitState(WoWGuid guid, uint8 state)
{
    if(InstanceData *data = m_iData) // Save raw death state for units
    {
        data->AddObjectState(guid, state);
        data->SetUpdated();
    }
}

void MapInstance::SetGameObjState(WoWGuid guid, uint8 state)
{
    if(InstanceData *data = m_iData) // Only save certain gameobject states to instance data
    {
        data->AddObjectState(guid, state&0xFF);
        data->SetUpdated();
    }
}

void MapInstance::GetClosestGraveyard(uint8 team, uint32 mapId, float x, float y, float z, uint32 zoneId, LocationVector &output)
{
    _map->GetClosestGraveyard(team, mapId, x, y, z, zoneId, output);
}

bool MapInstance::CheckCombatStatus(Unit *unit)
{
    Guard guard(m_combatLock);
    // If we're forcing combat ignore mapping states
    if(m_forceCombatState)
        return true;
    if(unit == NULL)
        return !m_combatPartners.empty();
    // We have no combat history
    if(m_combatPartners.find(unit->GetGUID()) == m_combatPartners.end())
        return false;
    // We have no combatants
    if(m_combatPartners.at(unit->GetGUID()).empty())
        return false;

    return true;
}

void MapInstance::ClearCombatTimers(WoWGuid guid, WoWGuid guid2)
{
    Guard guard(m_combatLock);
    if(guid2.empty())
    {   // Remove all guids tied to guid1'
        for(std::set<WoWGuid>::iterator itr = m_combatPartners[guid].begin(); itr != m_combatPartners[guid].end(); itr++)
        {
            // Remove our combatant from our partner lists
            m_combatPartners[*itr].erase(guid);
            // Clean up our combat partner list
            if(m_combatPartners[*itr].empty())
                m_combatPartners.erase(*itr);
            // Remove combat timer from processing list
            m_combatTimers.erase(std::make_pair(guid, *itr));
        }
        // Clean up our combat partner list
        m_combatPartners.erase(guid);
        return;
    }

    // Remove our combatants from our partner lists
    m_combatPartners[guid].erase(guid2);
    m_combatPartners[guid2].erase(guid);
    // Clean up our combat partner list
    if(m_combatPartners[guid].empty())
        m_combatPartners.erase(guid);
    if(m_combatPartners[guid2].empty())
        m_combatPartners.erase(guid2);
    // Remove combat timer from processing list
    m_combatTimers.erase(std::make_pair(guid, guid2));
}

void MapInstance::TriggerCombatTimer(WoWGuid guid, WoWGuid guid2, uint32 timer)
{
    Guard guard(m_combatLock);
    m_combatPartners[guid].insert(guid2);
    m_combatPartners[guid2].insert(guid);
    m_combatTimers[std::make_pair(guid, guid2)] = timer;
}

void MapInstance::HookOnAreaTrigger(Player* plr, uint32 id)
{
    switch (id)
    {
    case 4591:
        //Only opens when the first one steps in, if 669 if you find a way, put it in :P (else was used to increase the time the door stays opened when another one steps on it)
        GameObject* door = castPtr<GameObject>(GetObjectClosestToCoords(184212, 803.827f, 6869.38f, -38.5434f, 99999.0f, TYPEID_GAMEOBJECT));
        if (door && (door->GetState() == 1))
            door->SetState(0);
        break;
    }
}

Creature* MapInstance::CreateCreature(uint32 entry)
{
    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(entry);
    if(ctrData == NULL || !ctrData->HasValidModelData())
    {
        sLog.Warning("MapInstance", "Skipping CreateCreature for entry %u due to incomplete database.", entry);
        return NULL;
    }

    Creature *cr = new Creature();
    m_objectCreationLock.Acquire();
    cr->Construct(ctrData, MAKE_NEW_GUID((IsInstance() ? sInstanceMgr.AllocateCreatureGuid() : ++m_CreatureHighGuid), entry, (ctrData->vehicleEntry > 0 ? HIGHGUID_TYPE_VEHICLE : HIGHGUID_TYPE_UNIT)));
    m_objectCreationLock.Release();
    cr->Init();
    cr->MarkNonBulk();
    return cr;
}

void MapInstance::ConstructCreature(WoWGuid &guid, Creature *allocation)
{
    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(guid.getEntry());
    m_objectCreationLock.Acquire();
    uint16 highGuid = (ctrData->vehicleEntry > 0 ? HIGHGUID_TYPE_VEHICLE : HIGHGUID_TYPE_UNIT);
    ASSERT( guid.getHigh() == highGuid );
    allocation->Construct(ctrData, guid);
    m_objectCreationLock.Release();
    allocation->Init();
}

Summon* MapInstance::CreateSummon(uint32 entry, int32 duration)
{
    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(entry);
    if(ctrData == NULL)
    {
        sLog.Warning("MapInstance", "Skipping CreateSummon for entry %u due to incomplete database.", entry);
        return NULL;
    }

    m_objectCreationLock.Acquire();
    Summon *sum = new Summon(ctrData, MAKE_NEW_GUID(++m_CreatureHighGuid, entry, HIGHGUID_TYPE_UNIT), duration);
    m_objectCreationLock.Release();

    sum->Init();
    sum->MarkNonBulk();
    return sum;
}

GameObject* MapInstance::CreateGameObject(uint32 entry)
{
    GameObjectInfo *goi = GameObjectNameStorage.LookupEntry( entry );
    if( goi == NULL )
    {
        sLog.Warning("MapInstance", "Skipping CreateGameObject for entry %u due to incomplete database.", entry);
        return NULL;
    }

    GameObject *go = new GameObject();
    m_objectCreationLock.Acquire();
    go->Construct(goi, MAKE_NEW_GUID(++m_GOHighGuid, entry, HIGHGUID_TYPE_GAMEOBJECT));
    m_objectCreationLock.Release();

    go->Init();
    go->MarkNonBulk();
    return go;
}

void MapInstance::ConstructGameObject(WoWGuid &guid, GameObject *allocation)
{
    GameObjectInfo *goi = GameObjectNameStorage.LookupEntry( guid.getEntry() );
    ASSERT( guid.getHigh() == HIGHGUID_TYPE_GAMEOBJECT );

    m_objectCreationLock.Acquire();
    allocation->Construct(goi, guid);
    m_objectCreationLock.Release();
    allocation->Init();
}

DynamicObject* MapInstance::AllocateDynamicObject(WoWGuid source)
{
    m_objectCreationLock.Acquire();
    DynamicObject* dyn = new DynamicObject(HIGHGUID_TYPE_DYNAMICOBJECT, (++m_DynamicObjectHighGuid));
    m_objectCreationLock.Release();

    dyn->Init();
    dyn->MarkNonBulk();
    return dyn;
}

void MapInstance::SendPacketToPlayers(int32 iZoneMask, int32 iFactionMask, WorldPacket *pData)
{
    Player * ptr = NULL; // Update all players on map.
    __player_iterator = m_PlayerStorage.begin();
    while(__player_iterator != m_PlayerStorage.end())
    {
        ptr = __player_iterator->second;
        ++__player_iterator;

        if(ptr->GetSession())
        {
            //Are we in the right zone?
            if( iZoneMask != ZONE_MASK_ALL && ptr->GetZoneId() != (uint32)iZoneMask )
                continue;

            //Are we the right faction?
            if( iFactionMask != FACTION_MASK_ALL && ptr->GetTeam() != (uint32)iFactionMask )
                continue;

            ptr->PushPacket(pData);
        }
    }
}

void MapInstance::RemoveAuraFromPlayers(int32 iFactionMask, uint32 uAuraId)
{
    Player* ptr; // Update all players on map.
    __player_iterator = m_PlayerStorage.begin();
    while(__player_iterator != m_PlayerStorage.end())
    {
        ptr = __player_iterator->second;
        ++__player_iterator;

        if(ptr->GetSession())
        {
            if( iFactionMask != FACTION_MASK_ALL && ptr->GetTeam() != (uint32)iFactionMask )
                continue;

            ptr->RemoveAura(uAuraId);
        }
    }
}

void MapInstance::RemovePositiveAuraFromPlayers(int32 iFactionMask, uint32 uAuraId)
{
    Player* ptr; // Update all players on map.
    __player_iterator = m_PlayerStorage.begin();
    while(__player_iterator != m_PlayerStorage.end())
    {
        ptr = __player_iterator->second;
        ++__player_iterator;

        if(ptr->GetSession())
        {
            if( iFactionMask != FACTION_MASK_ALL && ptr->GetTeam() != (uint32)iFactionMask )
                continue;
            ptr->m_AuraInterface.RemovePositiveAura(uAuraId);
        }
    }
}

void MapInstance::CastSpellOnPlayers(int32 iFactionMask, uint32 uSpellId)
{
    SpellEntry * sp = dbcSpell.LookupEntry(uSpellId);
    if( !sp )
        return;

    Player* ptr; // Update all players on map.
    __player_iterator = m_PlayerStorage.begin();
    while(__player_iterator != m_PlayerStorage.end())
    {
        ptr = __player_iterator->second;
        ++__player_iterator;

        if(ptr->GetSession())
        {
            if( iFactionMask != FACTION_MASK_ALL && ptr->GetTeam() != (uint32)iFactionMask )
                continue;
        }
    }
}

void MapInstance::SendPvPCaptureMessage(int32 iZoneMask, uint32 ZoneId, const char * Format, ...)
{
    va_list ap;
    va_start(ap,Format);

    WorldPacket data(SMSG_DEFENSE_MESSAGE, 208);
    char msgbuf[200];
    vsnprintf(msgbuf, 200, Format, ap);
    va_end(ap);

    data << ZoneId;
    data << uint32(strlen(msgbuf)+1);
    data << msgbuf;

    Player* ptr; // Update all players on map.
    __player_iterator = m_PlayerStorage.begin();
    while(__player_iterator != m_PlayerStorage.end())
    {
        ptr = __player_iterator->second;
        ++__player_iterator;

        if(ptr->GetSession())
        {
            if( ( iZoneMask != ZONE_MASK_ALL && ptr->GetZoneId() != (uint32)iZoneMask) )
                continue;

            ptr->PushPacket(&data);
        }
    }
}

uint8 MapInstance::StartTrade(WoWGuid ownerGuid, WoWGuid targetGuid, Player **targetPlr)
{
    if(m_tradeData.find(ownerGuid) != m_tradeData.end())
        return 0x00; // No error, just exit

    Player *owner = GetPlayer(ownerGuid), *target = GetPlayer(targetGuid);
    if(target == NULL || !target->IsInWorld())
        return TRADE_STATUS_NO_TARGET;
    else if(owner->isDead())
        return TRADE_STATUS_YOU_DEAD;
    else if(target->isDead())
        return TRADE_STATUS_TARGET_DEAD;
    else if(owner->IsStunned())
        return TRADE_STATUS_YOU_STUNNED;
    else if(target->IsStunned())
        return TRADE_STATUS_TARGET_STUNNED;
    else if(m_tradeData.find(targetGuid) != m_tradeData.end())
        return TRADE_STATUS_BUSY;
    else if(target->GetTeam() != owner->GetTeam() && !owner->hasGMTag())
        return TRADE_STATUS_WRONG_FACTION;
    else if(target->hasIgnored(owner->GetGUID()))
        return TRADE_STATUS_IGNORE_YOU;
    else if(owner->GetDistanceSq(target) > 225.f)     // This needs to be checked
        return TRADE_STATUS_TARGET_TO_FAR;
    else if(target->GetSession() == NULL || target->GetSession()->GetSocket() == NULL)
        return TRADE_STATUS_TARGET_LOGOUT;

    *targetPlr = target;

    TradeData *tradeData = new TradeData();
    // Set our guids for the trade, we're the owner since we initiated
    tradeData->traders[0] = ownerGuid;
    tradeData->traders[1] = targetGuid;
    // Set accepted to false
    tradeData->accepted[0] = tradeData->accepted[1] = false;
    // Set trade information to 0
    tradeData->gold[0] = tradeData->gold[1] = 0;
    for(uint8 i = 0; i < 7; ++i)
        tradeData->items[i] = tradeData->items[7+i] = 0;
    tradeData->enchantId[0] = tradeData->enchantId[1] = 0;

    m_activeTradeData.insert(tradeData);
    m_tradeData.insert(std::make_pair(ownerGuid, tradeData));
    m_tradeData.insert(std::make_pair(targetGuid, tradeData));
    return TRADE_STATUS_BEGIN_TRADE;
}

Player *MapInstance::BeginTrade(WoWGuid acceptor)
{
    if(m_tradeData.find(acceptor) == m_tradeData.end())
        return NULL;
    TradeData *currentTrade = m_tradeData.at(acceptor);
    if(currentTrade->getIndex(acceptor) == 0xFF)
        return NULL;

    Player *owner = GetPlayer(currentTrade->traders[0]), *target = GetPlayer(currentTrade->traders[1]);
    if((owner == NULL || !owner->IsInWorld()) || (target == NULL || !target->IsInWorld()))
    {
        CleanupTrade(acceptor);
        return NULL;
    }

    // Trader statuses
    if(owner->isDead() || target->isDead() || owner->IsStunned() || target->IsStunned())
    {
        CleanupTrade(acceptor);
        return NULL;
    }

    return owner->GetGUID() == acceptor ? target : owner;
}

Player *MapInstance::GetTradeTarget(WoWGuid trader)
{
    if(m_tradeData.find(trader) == m_tradeData.end())
        return NULL;
    uint8 traderIndex;
    TradeData *currentTrade = m_tradeData.at(trader);
    if((traderIndex = currentTrade->getIndex(trader)) == 0xFF)
        return NULL;

    return GetPlayer(currentTrade->traders[traderIndex ? 0 : 1]);
}

void MapInstance::SetTradeStatus(WoWGuid trader, uint8 status)
{
    std::map<WoWGuid, TradeData*>::iterator itr;
    if((itr = m_tradeData.find(trader)) == m_tradeData.end())
        return; // No error, just exit

    TradeData *tradeData = itr->second;
    Player *owner = GetPlayer(tradeData->traders[0]), *target = GetPlayer(tradeData->traders[1]);
    if((owner == NULL || !owner->IsInWorld()) || (target == NULL || !target->IsInWorld()) || status == TRADE_STATUS_TRADE_CANCELED)
    {
        CleanupTrade(trader);
        return;
    }

    uint8 index = tradeData->getIndex(trader);
    switch(status)
    {
    case TRADE_STATUS_TRADE_ACCEPT:
        bool res = false;
        if(tradeData->accepted[index])
            tradeData->accepted[index] = false;
        else tradeData->accepted[index] = res = true;
        if(index == 0)
            SendTradeStatus(target, TRADE_STATUS_TRADE_ACCEPT, tradeData->traders[0], tradeData->traders[1], false);
        else SendTradeStatus(owner, TRADE_STATUS_TRADE_ACCEPT, tradeData->traders[1], tradeData->traders[0], false);

        if(res && tradeData->accepted[index ? 0 : 1])
            CompleteTrade(tradeData);
        break;
    }
}

void MapInstance::SetTradeValue(Player *plr, uint8 step, uint64 value, WoWGuid option)
{
    if(m_tradeData.find(plr->GetGUID()) == m_tradeData.end())
        return;
    uint8 ourIndex;
    TradeData *currentTrade = m_tradeData.at(plr->GetGUID());
    if((ourIndex = currentTrade->getIndex(plr->GetGUID())) == 0xFF)
        return;
    uint8 tIndex = ourIndex ? 0 : 1;

    Player *target = GetPlayer(currentTrade->traders[tIndex]);
    if(target == NULL || !target->IsInWorld())
    {
        CleanupTrade(plr->GetGUID());
        return;
    }

    switch(step)
    {
    case 0: // Set gold
        currentTrade->gold[ourIndex] = value;
        SendTradeData(target, false, currentTrade);
        break;

    case 1: // Set an item
        currentTrade->items[value+(ourIndex*7)] = option;
        SendTradeData(target, false, currentTrade);
        break;

    case 2: // Cast an enchantment
        currentTrade->enchantId[ourIndex] = value;
        SendTradeData(target, false, currentTrade);
        break;
    }
}

void MapInstance::SendTradeStatus(Player *target, uint32 TradeStatus, WoWGuid owner, WoWGuid trader, bool linkedbNetAccounts, uint8 slotError, bool hasInvError, uint32 invError, uint32 limitCategoryId)
{
    WorldPacket data(SMSG_TRADE_STATUS, 13);
    data.WriteBit(linkedbNetAccounts ? 1 : 0);
    data.WriteBits(TradeStatus, 5);
    switch (TradeStatus)
    {
    case MapInstance::TRADE_STATUS_BEGIN_TRADE:
        data.WriteGuidBitString(8, trader, ByteBuffer::Filler, 2, 4, 6, 0, 1, 3, 7, 5);
        data.WriteSeqByteString(8, trader, ByteBuffer::Filler, 4, 1, 2, 3, 0, 7, 6, 5);
        break;
    case MapInstance::TRADE_STATUS_OPEN_WINDOW: data << uint32(0); break;
    case MapInstance::TRADE_STATUS_CLOSE_WINDOW:
        data.WriteBit(hasInvError);
        data << uint32(invError) << uint32(limitCategoryId);
        break;
    case MapInstance::TRADE_STATUS_WRONG_REALM:
    case MapInstance::TRADE_STATUS_NOT_ON_TAPLIST:
        data << uint8(slotError);
        break;
    case MapInstance::TRADE_STATUS_CURRENCY:
    case MapInstance::TRADE_STATUS_CURRENCY_NOT_TRADABLE:
        data << uint32(0); // Trading Currency Id
        data << uint32(0); // Trading Currency Amount
    default:
        data.FlushBits();
        break;
    }

    target->PushPacket(&data);
}

void MapInstance::SendTradeData(Player *plr, bool ourData, MapInstance::TradeData *tradeData)
{
    uint8 cIndex = ourData ? tradeData->getIndex(plr->GetGUID()) : tradeData->getIndex(plr->GetGUID()) ? 0 : 1;
    Player *target = GetPlayer(tradeData->traders[cIndex]);

    uint8 start = cIndex ? 7 : 0;
    uint32 itemCount = 0;
    for(uint8 i = 0; i < 7; ++i)
        if(tradeData->items[start+i])
            ++itemCount;

    WorldPacket data(SMSG_TRADE_STATUS_EXTENDED, 400);
    data << uint32(0) << uint32(0);
    data << uint64(tradeData->gold[cIndex]);
    data << uint32(tradeData->enchantId[cIndex]);
    data << uint32(7);
    data << uint32(0);
    data << uint8(ourData ? 0 : 1);
    data << uint32(7);
    data.WriteBits(itemCount, 22);

    ByteBuffer itemData;
    for(uint8 i = 0; i < 7; ++i)
    {
        if(tradeData->items[start+i].empty())
            continue;

        Item *item = target->GetInventory()->GetItemByGUID(tradeData->items[start+i]);
        if(item == NULL)
            continue;

        WoWGuid creatorGuid = item->GetCreatorGUID(), giftCreatorGuid = item->GetGiftCreatorGUID();
        data.WriteGuidBitString(2, giftCreatorGuid, ByteBuffer::Filler, 7, 1);
        data.WriteBit(!item->IsWrapped());
        data.WriteBit(giftCreatorGuid[3]);
        if(!item->IsWrapped())
        {   // Append bit data
            data.WriteGuidBitString(7, creatorGuid, ByteBuffer::Filler, 7, 1, 4, 6, 2, 3, 5);
            data.WriteBit(item->GetProto()->LockId);
            data.WriteBit(creatorGuid[0]);
            // Append item data to byte buffer, since we're not wrapped
            itemData.WriteByteSeq(creatorGuid[1]);
            itemData << uint32(item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT));
            itemData << uint32(item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT1));
            itemData << uint32(item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT2));
            itemData << uint32(item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT3));
            itemData << uint32(item->GetDurabilityMax());
            itemData.WriteSeqByteString(4, creatorGuid, ByteBuffer::Filler, 6, 2, 7, 4);
            itemData << uint32(item->GetEnchantmentId(REFORGE_ENCHANTMENT_SLOT));
            itemData << uint32(item->GetDurability());
            itemData << uint32(item->GetItemRandomPropertyId());
            itemData.WriteByteSeq(creatorGuid[3]);
            itemData << uint32(0);
            itemData.WriteByteSeq(creatorGuid[0]);
            itemData << uint32(item->GetChargesLeft());
            itemData << uint32(item->GetItemPropertySeed());
            itemData.WriteByteSeq(creatorGuid[5]);
        }

        // Write some bits
        data.WriteGuidBitString(5, giftCreatorGuid, ByteBuffer::Filler, 6, 4, 2, 0, 5);
        // Append item data to byte buffer
        itemData.WriteSeqByteString(4, giftCreatorGuid, ByteBuffer::Filler, 6, 1, 7, 4);
        itemData << uint32(item->GetEntry());
        itemData.WriteByteSeq(giftCreatorGuid[0]);
        itemData << uint32(item->GetStackCount());
        itemData.WriteByteSeq(giftCreatorGuid[5]);
        itemData << uint8(i);
        itemData.WriteByteSeq(giftCreatorGuid[2]);
        itemData.WriteByteSeq(giftCreatorGuid[3]);
    }
    data.FlushBits();
    data.append(itemData.contents(), itemData.size());
    plr->PushPacket(&data);
}

void MapInstance::UpdateTrade(MapInstance::TradeData *tradeData)
{
    bool cleanupTrade = false;
    Player *owner = GetPlayer(tradeData->traders[0]), *target = GetPlayer(tradeData->traders[1]);
    if((owner == NULL || !owner->IsInWorld()) || (target == NULL || !target->IsInWorld()))
        cleanupTrade = true;
    else if(owner->isDead() || target->isDead())
        cleanupTrade = true;
    else if(owner->IsStunned() || target->IsStunned())
        cleanupTrade = true;
    else if(owner->GetDistanceSq(target) > 225.f)     // This needs to be checked
        cleanupTrade = true;
    else if(owner->GetSession() == NULL || target->GetSession() == NULL)
        cleanupTrade = true;
    if(cleanupTrade == false)
        return;
    CleanupTrade(tradeData->traders[0]);
}

void MapInstance::CleanupTrade(WoWGuid eitherGuid, bool silent)
{
    if(m_tradeData.find(eitherGuid) == m_tradeData.end())
        return;
    TradeData *tradeData = m_tradeData.at(eitherGuid);
    if(silent == false)
    {
        if(Player *owner = GetPlayer(tradeData->traders[0]))
            SendTradeStatus(owner, TRADE_STATUS_TRADE_CANCELED, 0, 0, false);
        if(Player *target = GetPlayer(tradeData->traders[1]))
            SendTradeStatus(target, TRADE_STATUS_TRADE_CANCELED, 0, 0, false);
    }

    m_tradeData.erase(tradeData->traders[0]);
    m_tradeData.erase(tradeData->traders[1]);
    m_activeTradeData.erase(tradeData);
    m_tradeDeleteQueue.insert(tradeData);
}

void MapInstance::CompleteTrade(MapInstance::TradeData *tradeData)
{
    Player *owner = GetPlayer(tradeData->traders[0]), *target = GetPlayer(tradeData->traders[1]);
    // Run validation and send errors
    uint8 tradeError = TRADE_STATUS_TRADE_COMPLETE;
    if(owner == NULL || !owner->IsInWorld() || owner->GetSession() == NULL || target == NULL || !target->IsInWorld() || target->GetSession() == NULL)
        tradeError = TRADE_STATUS_TRADE_CANCELED;
    else if(owner->isDead() || target->isDead() || owner->IsStunned() || target->IsStunned())
        tradeError = TRADE_STATUS_TRADE_CANCELED;
    else if(owner->GetDistanceSq(target) > 225.f)
        tradeError = TRADE_STATUS_TRADE_CANCELED;

    Item *itemsToPush[2][6];
    std::map<Item*, std::pair<int16, uint8>> previousSlots[2], assignedItemSlots[2];
    Player *traders[2] = {owner, target};
    for(uint8 i = 0; i < 2; ++i)
    {
        if(uint64 tradeAmount = tradeData->gold[i])
            if(traders[i]->GetUInt64Value(PLAYER_FIELD_COINAGE) < tradeAmount)
                tradeError = TRADE_STATUS_TRADE_CANCELED;

        PlayerInventory *inventory = traders[i]->GetInventory();
        for(uint8 j = 0; j < 6; ++j)
        {
            itemsToPush[i][j] = NULL;
            if(tradeData->items[(i*7)+j].empty())
                continue;

            Item *tradeItem = inventory->GetItemByGUID(tradeData->items[(i*7)+j]);
            if(tradeItem == NULL || !traders[i]->GetInventory()->CanTradeItem(traders[i], tradeItem))
                tradeError = TRADE_STATUS_TRADE_CANCELED;
            else
            {
                uint8 invSlot; // Grab our bag and inventory slot from item before removal
                uint16 bagSlot = inventory->GetBagSlotByGuid(tradeData->items[(i*7)+j], invSlot);
                // Remove item from slot but do not mark for deletion, we can remove and destroy for player later
                inventory->SafeRemoveAndRetreiveItemByGuidRemoveStats(tradeData->items[(i*7)+j], false);
                // Save previous slot so that we can return it if we fail to find new slots for our new items
                previousSlots[i].insert(std::make_pair(tradeItem, std::make_pair(bagSlot, invSlot)));
                itemsToPush[i][j] = tradeItem;
            }
        }

        if(tradeData->enchantId[i] && !tradeData->items[(i*7)+6].empty())
        {
            Item *tradeItem = inventory->GetItemByGUID(tradeData->items[(i*7)+6]);
            if(tradeItem == NULL)// || !traders[i]->GetSpellInterface()->CanApplyEnchant(tradeItem, tradeData->enchantId[i]))
                tradeError = TRADE_STATUS_TRADE_CANCELED;
        }
    }

    // Check to see if we can trade our items into our targets inventory
    if(tradeError == TRADE_STATUS_TRADE_COMPLETE)
    {
        for(uint8 i = 0; i < 2; ++i)
        {
            uint8 tI = i ? 0 : 1;
            std::set<std::pair<int16, int16>> usedSlots;
            PlayerInventory *tInv = traders[tI]->GetInventory();
            for(uint8 j = 0; j < 6; ++j)
            {
                if(itemsToPush[i][j] == NULL)
                    continue;

                // Find a free slot for our item, we've already removed our items as well so the slots count from that
                SlotResult result = tInv->FindFreeInventorySlot(itemsToPush[i][j]->GetProto(), &usedSlots);
                if(result.Result == false)
                    tradeError = TRADE_STATUS_TRADE_CANCELED;
                else
                {   // Store our assigned slot and push the slot to the vector so it is skipped in next item transfer check
                    assignedItemSlots[i].insert(std::make_pair(itemsToPush[i][j], std::make_pair(result.ContainerSlot, (uint8)result.Slot)));
                    usedSlots.insert(std::make_pair(result.ContainerSlot, result.Slot));
                }
                // Null out the pointer
                itemsToPush[i][j] = NULL;
            }
        }
    }

    // If we've failed here, return all items
    if(tradeError != TRADE_STATUS_TRADE_COMPLETE)
    {
        for(uint8 i = 0; i < 2; ++i)
        {
            // Add all items back to their previous slots
            for(auto itr = previousSlots[i].begin(); itr != previousSlots[i].end(); ++itr)
            {
                ASSERT(traders[i]->GetInventory()->SafeAddItem(itr->first, itr->second.first, itr->second.second));
            }
        }
    }
    else // Validation complete, if we've passed our checks then we can start trading now
    {
        for(uint8 i = 0; i < 2; ++i)
        {
            uint8 tI = i ? 0 : 1;
            if(uint64 tradeAmount = tradeData->gold[i])
            {   // Just remove gold from us and add to target
                traders[i]->SetUInt64Value(PLAYER_FIELD_COINAGE, traders[i]->GetUInt64Value(PLAYER_FIELD_COINAGE)-tradeAmount);
                traders[tI]->SetUInt64Value(PLAYER_FIELD_COINAGE, traders[tI]->GetUInt64Value(PLAYER_FIELD_COINAGE)+tradeAmount);
            }

            // Now that we've removed all items we can add the new ones in
            for(auto itr = assignedItemSlots[i].begin(); itr != assignedItemSlots[i].end(); ++itr)
            {
                bool result;
                // Unassign from previous owner
                if(itr->first->IsInWorld())
                    itr->first->RemoveFromWorld();
                itr->first->DeleteFromDB();
                // Prep for transfer then add to target slot
                itr->first->PrepPostTransfer();
                ASSERT(result = traders[tI]->GetInventory()->SafeAddItem(itr->first, itr->second.first, itr->second.second));
            }

            // Now process enchantments
            if(tradeData->enchantId[i])
            {
                // Todo
            }
        }
    }

    // Push result packet, either complete or error
    if(owner && owner->GetSession())
        SendTradeStatus(owner, tradeError, 0, 0, false);
    if(target && target->GetSession())
        SendTradeStatus(target, tradeError, 0, 0, false);
    // cleanup after ourselves
    CleanupTrade(tradeData->traders[0], true);
}
