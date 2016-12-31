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

MapInstance::MapInstance(Map *map, uint32 mapId, uint32 instanceid) : CellHandler<MapCell>(map), _mapId(mapId), m_instanceID(instanceid), pdbcMap(dbcMap.LookupEntry(mapId)), m_stateManager(new WorldStateManager(this)), _removalCallback(this)
{
    m_mapPreloading = false;
    iInstanceMode = 0;

    m_GOHighGuid = 0;
    m_CreatureHighGuid = 0;
    m_DynamicObjectHighGuid=0;
    m_battleground = NULL;

    InactiveMoveTime = 0;

    // buffers
    m_createBuffer.reserve(0x7FFF);
    m_updateBuffer.reserve(0x1FF);

    m_PlayerStorage.clear();
    m_DynamicObjectStorage.clear();

    _combatProgress.clear();
    _updates.clear();
    _processQueue.clear();
    MapSessions.clear();

    m_corpses.clear();

    mCreaturePool.Initialize(pdbcMap && pdbcMap->IsContinent() ? 8 : 4);
    mGameObjectPool.Initialize(pdbcMap && pdbcMap->IsContinent() ? 4 : 2);
    mDynamicObjectPool.Initialize(pdbcMap && pdbcMap->IsContinent() ? 2 : 1);
    mUnitPathPool.Initialize(pdbcMap && pdbcMap->IsContinent() ? 2 : 1);

    projectileSpellUpdateTime[0] = projectileSpellUpdateTime[1] = 0;
    projectileSpellIndex[0] = projectileSpellIndex[1] = 0;
}

void MapInstance::Preload()
{
    if(sWorld.ServerPreloading >= 2)
        UpdateAllCells(true);
}

void MapInstance::Init(uint32 msTime)
{
    mCreaturePool.ResetTime(msTime);
    mGameObjectPool.ResetTime(msTime);
    mDynamicObjectPool.ResetTime(msTime);
}

void MapInstance::Destruct()
{
    if( m_stateManager != NULL )
    {
        delete m_stateManager;
        m_stateManager = NULL;
    }

    std::vector<WorldObject*> m_delQueue;
    while(m_CreatureStorage.size())
    {
        Creature *ctr = m_CreatureStorage.begin()->second;
        m_delQueue.push_back(ctr);
        RemoveObject(ctr);
    }

    while(m_gameObjectStorage.size())
    {
        GameObject *gObj = m_gameObjectStorage.begin()->second;
        m_delQueue.push_back(gObj);
        RemoveObject(gObj);
    }

    while(m_DynamicObjectStorage.size())
    {
        DynamicObject *dynObj = m_DynamicObjectStorage.begin()->second;
        m_delQueue.push_back(dynObj);
        RemoveObject(dynObj);
    }

    while(m_delQueue.size())
    {
        WorldObject *obj = *m_delQueue.begin();
        m_delQueue.erase(m_delQueue.begin());
        obj->Destruct();
    }
    UnloadCells();

    Corpse* pCorpse;
    if(m_corpses.size())
    {
        for(std::vector<Corpse* >::iterator itr = m_corpses.begin(); itr != m_corpses.end();)
        {
            pCorpse = *itr;
            ++itr;

            if(pCorpse->IsInWorld())
                pCorpse->RemoveFromWorld();

            pCorpse->Destruct();
            pCorpse = NULL;
        }
        m_corpses.clear();
    }

    //Clear our remaining containers
    m_PlayerStorage.clear();
    m_DynamicObjectStorage.clear();
    m_CreatureStorage.clear();
    m_gameObjectStorage.clear();

    _combatProgress.clear();
    _updates.clear();
    _processQueue.clear();
    MapSessions.clear();
    pdbcMap = NULL;

    mCreaturePool.Cleanup();
    mGameObjectPool.Cleanup();
    mDynamicObjectPool.Cleanup();

    m_battleground = NULL;

    delete this;
}

MapInstance::~MapInstance()
{

}

void MapInstance::EventPushObjectToSelf(WorldObject *obj)
{
    obj->PushToWorld(this);
}

void MapInstance::PushObject(WorldObject* obj)
{
    /////////////
    // Assertions
    /////////////
    ASSERT(obj);

    obj->ClearInRangeObjects();

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
    }

    ///////////////////////
    // Get cell coordinates
    ///////////////////////

    ASSERT(obj->GetMapId() == _mapId);
    ASSERT(obj->GetPositionZ() < _maxY && obj->GetPositionZ() > _minY);

    float mx = obj->GetPositionX();
    float my = obj->GetPositionY();
    uint32 cx = GetPosX(mx), cy = GetPosY(my);

    if( mx > _maxX || my > _maxY ||
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
                plObj->GetSession()->SendPacket(data);
                delete data;
            }
        }
        else
        {
            obj->GetPositionV()->ChangeCoords(0.f,0.f,0.f,0.f);
        }

        cx = GetPosX(obj->GetPositionX());
        cy = GetPosY(obj->GetPositionY());
    }

    MapCell *objCell = GetCell(cx,cy);
    if (objCell == NULL && (objCell = Create(cx,cy)) != NULL) // Should never fail to create but...
        objCell->Init(cx, cy, _mapId, this);
    ASSERT(objCell);

    uint32 count = 0, minX = cx ? cx-1 : 0, maxX = (cx < _sizeY-1 ? cx+1 : _sizeY-1), minY = cy ? cy-1 : 0, maxY = (cy < _sizeY-1 ? cy+1 : _sizeY-1);
    if(plObj && (count = plObj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj)))
    {
        sLog.Debug("MapInstance","Creating player %llu for himself.", obj->GetGUID().raw());
        plObj->PushUpdateBlock(&m_createBuffer, count);
    }
    m_createBuffer.clear();

    //Add to the cell's object list
    objCell->AddObject(obj);

    obj->SetMapCell(objCell);

    //Add to the mapmanager's object list
    if(plObj)
    {
        UnitPathSystem *path = plObj->GetMovementInterface()->GetPath();
        path->setPathPool(mUnitPathPool.Add(path));
        m_PlayerStorage.insert(std::make_pair(plObj->GetGUID(), plObj));
        UpdateCellActivity(cx, cy, 2);
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
                UnitPathSystem *path = creature->GetMovementInterface()->GetPath();
                path->setPathPool(mUnitPathPool.Add(path));
            }break;

        case HIGHGUID_TYPE_GAMEOBJECT:
            {
                GameObject* go = castPtr<GameObject>(obj);
                m_gameObjectStorage.insert(std::make_pair(obj->GetGUID(), go));
                TRIGGER_INSTANCE_EVENT( this, OnGameObjectPushToWorld )( go );
                sVMapInterface.LoadGameobjectModel(obj->GetGUID(), _mapId, go->GetDisplayId(), go->GetFloatValue(OBJECT_FIELD_SCALE_X), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), go->GetOrientation(), go->GetInstanceID(), go->GetPhaseMask());
            }break;

        case HIGHGUID_TYPE_DYNAMICOBJECT:
            m_DynamicObjectStorage.insert(std::make_pair(obj->GetGUID(), castPtr<DynamicObject>(obj)));
            break;
        }
    }

    // Handle activation of that object.
    if(objCell->IsActive())
    {
        m_poolLock.Acquire();
        switch(obj->GetTypeId())
        {
        case TYPEID_UNIT:
            {
                Creature *cObj = castPtr<Creature>(obj);
                uint8 poolID = mCreaturePool.Add(cObj);
                cObj->AssignCreaturePool(poolID);
            }break;

        case TYPEID_GAMEOBJECT:
            {
                GameObject *gObj = castPtr<GameObject>(obj);
                uint8 poolID = mGameObjectPool.Add(gObj);
                gObj->AssignGameObjectPool(poolID);
            }break;

        case TYPEID_DYNAMICOBJECT:
            {
                DynamicObject *dObj = castPtr<DynamicObject>(obj);
                uint8 poolID = mDynamicObjectPool.Add(dObj);
                dObj->AssignDynamicObjectPool(poolID);
            }break;
        }
        m_poolLock.Release();
    }

    // Add the session to our set if it is a player.
    if(plObj)
    {
        MapSessions.insert(plObj->GetSession());

        // Change the instance ID, this will cause it to be removed from the world thread (return value 1)
        plObj->GetSession()->SetEventInstanceId(GetInstanceID());

        // Update our player's zone
        plObj->UpdateAreaInfo(this);

        /* Add the zone wide objects */
        if(m_fullRangeObjectsByZone[plObj->GetZoneId()].size())
        {
            for(std::vector<WorldObject* >::iterator itr = m_fullRangeObjectsByZone[plObj->GetZoneId()].begin(); itr != m_fullRangeObjectsByZone[plObj->GetZoneId()].end(); itr++)
            {
                if(count = (*itr)->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj))
                    plObj->PushUpdateBlock(&m_createBuffer, count);
                m_createBuffer.clear();
            }
        }

        /* Add the area wide objects */
        if(m_fullRangeObjectsByArea[plObj->GetAreaId()].size())
        {
            for(std::vector<WorldObject* >::iterator itr = m_fullRangeObjectsByArea[plObj->GetZoneId()].begin(); itr != m_fullRangeObjectsByArea[plObj->GetZoneId()].end(); itr++)
            {
                if(count = (*itr)->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj))
                    plObj->PushUpdateBlock(&m_createBuffer, count);
                m_createBuffer.clear();
            }
        }
    }

    // Push to our update queue
    m_updateMutex.Acquire();
    if(_updates.find(obj) == _updates.end())
        _updates.insert(obj);
    m_updateMutex.Release();

    //////////////////////
    // Build in-range data
    //////////////////////
    // Preloading maps will do inrange update after cell loading
    if(m_mapPreloading)
        return;

    // We don't want to build inrange maps for deactivated objects
    if(obj->IsActiveObject() && !obj->IsActivated())
        return;

    if(!obj->IsUnit()) // Call data for us alone instead of using cell manager
        UpdateCellData(obj, objCell->GetPositionX(), objCell->GetPositionY(), true);
    else castPtr<Unit>(obj)->GetCellManager()->SetCurrentCell(this, objCell->GetPositionX(), objCell->GetPositionY(), 2);
}

void MapInstance::RemoveObject(WorldObject* obj)
{
    /////////////
    // Assertions
    /////////////

    ASSERT(obj);
    ASSERT(obj->GetMapId() == _mapId);

    m_poolLock.Acquire();
    switch(obj->GetTypeId())
    {
    case TYPEID_UNIT:
        {
            Creature *cObj = castPtr<Creature>(obj);
            mCreaturePool.Remove(cObj, cObj->GetCreaturePool());
            cObj->AssignCreaturePool(0xFF);
        }break;

    case TYPEID_GAMEOBJECT:
        {
            GameObject *gObj = castPtr<GameObject>(obj);
            mGameObjectPool.Remove(gObj, gObj->GetGameObjectPool());
            gObj->AssignGameObjectPool(0xFF);
        }break;

    case TYPEID_DYNAMICOBJECT:
        {
            DynamicObject *dObj = castPtr<DynamicObject>(obj);
            mDynamicObjectPool.Remove(dObj, dObj->GetDynamicObjectPool());
            dObj->AssignDynamicObjectPool(0xFF);
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
        UnitPathSystem *path = plObj->GetMovementInterface()->GetPath();
        mUnitPathPool.Remove(path, path->getPathPool());
        path->setPathPool(0xFF);
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
                mUnitPathPool.Remove(path, path->getPathPool());
                path->setPathPool(0xFF);
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
        }
    }

    // That object types are not map objects. TODO: add AI groups here?
    if(obj->GetTypeId() == TYPEID_ITEM || obj->GetTypeId() == TYPEID_CONTAINER)
        return;

    if(!obj->GetMapCell())
    {
        /* set the map cell correctly */
        if(!(obj->GetPositionX() >= _maxX || obj->GetPositionX() <= _minY || obj->GetPositionY() >= _maxY || obj->GetPositionY() <= _minY))
            obj->SetMapCell(GetCellByCoords(obj->GetPositionX(), obj->GetPositionY()));
    }

    if(obj->GetMapCell())
    {
        ASSERT(obj->GetMapCell());

        // Remove object from cell
        obj->GetMapCell()->RemoveObject(obj);

        // Unset object's cell
        obj->SetMapCell(NULL);
    }

    if(!bServerShutdown)
    {
        // Remove object from all objects 'seeing' him
        WorldObject::InRangeHashMap inrangeObjects(*obj->GetInRangeMap());
        for (WorldObject::InRangeHashMap::iterator iter = inrangeObjects.begin(); iter != inrangeObjects.end(); iter++)
        {
            if(WorldObject *wObj = iter->second)
            {
                if(wObj->IsPlayer())
                {
                    if(Player *plr = castPtr<Player>(wObj))
                    {
                        if( plr->IsVisible( obj ) && plr->GetTransportGuid() != obj->GetGUID())
                            plr->PushOutOfRange(obj->GetGUID());
                        obj->DestroyForPlayer(plr, obj->IsGameObject());
                    }
                }
                wObj->RemoveInRangeObject(obj);
            }
        }
    }

    // Clear object's in-range set
    obj->ClearInRangeObjects();
}

void MapInstance::ChangeObjectLocation( WorldObject* obj )
{
    if( obj->GetMapInstance() != this )
        return;

    WorldObject* curObj = NULL;
    Player *plObj = obj->IsPlayer() ? castPtr<Player>(obj) : NULL;

    // Update our zone and area data
    obj->EventExploration(this);

    if(Group *grp = plObj ? plObj->GetGroup() : NULL)
        grp->HandlePartialChange( PARTY_UPDATE_FLAG_POSITION, plObj );

    ////////////////////////////////////////
    // Update in-range data for zone objects
    ////////////////////////////////////////
    uint32 lastZone = obj->GetLastMovementZone(), currZone;
    if(lastZone != (currZone = obj->GetZoneId()))
    {
        if(lastZone && m_fullRangeObjectsByZone[lastZone].size())
        {
            for(std::vector<WorldObject*>::iterator itr = m_fullRangeObjectsByZone[lastZone].begin(); itr != m_fullRangeObjectsByZone[lastZone].end(); itr++)
                if(!(*itr)->IsTransport() || (!obj->IsUnit() || castPtr<Unit>(obj)->GetTransportGuid() != (*itr)->GetGUID()))
                    obj->RemoveInRangeObject(*itr);
        }

        if(currZone && m_fullRangeObjectsByZone[currZone].size())
        {
            for(std::vector<WorldObject*>::iterator itr = m_fullRangeObjectsByZone[currZone].begin(); itr != m_fullRangeObjectsByZone[currZone].end(); itr++)
            {
                if(obj->IsInRangeSet(*itr) || !(*itr)->IsActivated())
                    continue;

                obj->AddInRangeObject(*itr);
                if(plObj && plObj->CanSee( (*itr) ) && !plObj->IsVisible( (*itr) ) )
                {
                    plObj->AddVisibleObject( (*itr) );
                    if(uint32 count = (*itr)->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj ))
                        plObj->PushUpdateBlock(&m_createBuffer, count);
                    m_createBuffer.clear();
                }
            }
        }

        if(Group *grp = plObj ? plObj->GetGroup() : NULL)
            grp->HandlePartialChange( PARTY_UPDATE_FLAG_ZONEID, plObj );
    }
    obj->SetLastMovementZone(currZone);

    ////////////////////////////////////////
    // Update in-range data for area objects
    ////////////////////////////////////////
    uint32 lastArea = obj->GetLastMovementArea(), currArea;
    if(lastArea != (currArea = obj->GetAreaId()))
    {
        if(lastArea && m_fullRangeObjectsByArea[lastArea].size())
        {
            for(std::vector<WorldObject*>::iterator itr = m_fullRangeObjectsByArea[lastArea].begin(); itr != m_fullRangeObjectsByArea[lastArea].end(); itr++)
                if(!(*itr)->IsTransport() || (!obj->IsUnit() || castPtr<Unit>(obj)->GetTransportGuid() != (*itr)->GetGUID()))
                    obj->RemoveInRangeObject(*itr);
        }

        if(currArea && m_fullRangeObjectsByArea[currArea].size())
        {
            for(std::vector<WorldObject*>::iterator itr = m_fullRangeObjectsByArea[currArea].begin(); itr != m_fullRangeObjectsByArea[currArea].end(); itr++)
            {
                if(obj->IsInRangeSet(*itr) || !(*itr)->IsActivated())
                    continue;

                obj->AddInRangeObject(*itr);
                if(plObj && plObj->CanSee( (*itr) ) && !plObj->IsVisible( (*itr) ) )
                {
                    plObj->AddVisibleObject( (*itr) );
                    if(uint32 count = (*itr)->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj ))
                        plObj->PushUpdateBlock(&m_createBuffer, count);
                    m_createBuffer.clear();
                }
            }
        }
    }
    obj->SetLastMovementArea(currArea);

    // Check if we're moving to a new map
    if(obj->GetMapInstance() != this)
    {
        if(!obj->HasInRangeObjects())
            return;

        for (WorldObject::InRangeHashMap::iterator iter = obj->GetInRangeMapBegin(); iter != obj->GetInRangeMapEnd(); iter++)
        {
            if((curObj = iter->second) == NULL || curObj == obj)
                continue;

            if(curObj->GetMapInstance() == this)
            {
                if( obj->IsPlayer() )
                    castPtr<Player>(obj)->RemoveIfVisible(curObj);

                if( curObj->IsPlayer() )
                    castPtr<Player>( curObj )->RemoveIfVisible(obj);

                curObj->RemoveInRangeObject(obj);
                obj->RemoveInRangeObject(curObj);
            }
        }
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
                    plObj->GetSession()->SendPacket(data);
                    delete data;
                }
            }
            else
            {
                // Remove object from all objects 'seeing' it
                WorldObject::InRangeHashMap inrangeObjects(*obj->GetInRangeMap());
                for (WorldObject::InRangeHashMap::iterator iter = inrangeObjects.begin(); iter != inrangeObjects.end(); iter++)
                {
                    if(WorldObject *wObj = iter->second)
                    {
                        if(wObj->IsPlayer())
                        {
                            if(Player *plr = castPtr<Player>(wObj))
                            {
                                if( plr->IsVisible( obj ) && plr->GetTransportGuid() != obj->GetGUID())
                                    plr->PushOutOfRange(obj->GetGUID());
                                obj->DestroyForPlayer(plr, obj->IsGameObject());
                            }
                        }
                        wObj->RemoveInRangeObject(obj);
                    }
                }

                // Clear object's in-range set
                obj->ClearInRangeObjects();

                obj->GetPositionV()->ChangeCoords(0,0,0,0);
                return;
            }
        }

        // Grab our cell data from position
        float fposX = obj->GetPositionX(), fposY = obj->GetPositionY();
        uint16 cellX = GetPosX(fposX), cellY = GetPosY(fposY);
        if(cellX >= _sizeX || cellY >= _sizeY)
            return; // No data outside parameters

        // Update our cell activity real quick
        if(plObj) UpdateCellActivity(cellX, cellY, 2);

        // Grab our new cell and store our old cell pointer
        MapCell *objCell = GetCell(cellX, cellY), *pOldCell = obj->GetMapCell();
        if (objCell == NULL && (objCell = Create(cellX,cellY)))
            objCell->Init(cellX, cellY, _mapId, this);

        if(objCell != pOldCell)
        {
            // Remove us from old cell
            if(pOldCell)
                pOldCell->RemoveObject(obj);

            obj->SetMapCell(objCell);
            // Process new cell
            if(objCell)
            {
                // Add us to our cell
                objCell->AddObject(obj);

                // Non player objects need area info for current cell
                if(!obj->IsPlayer())
                    obj->UpdateAreaInfo(this);

                if(!obj->IsUnit()) // Call data for us alone instead of using cell manager
                    UpdateCellData(obj, cellX, cellY, true);
                else castPtr<Unit>(obj)->GetCellManager()->SetCurrentCell(this, cellX, cellY, 2);
            }
        }
    }
}

bool MapInstance::UpdateCellData(WorldObject *obj, uint32 cellX, uint32 cellY, bool priority)
{
    // Check our cell status
    MapCell *objCell = GetCell(cellX, cellY);
    if(objCell == NULL && priority == false)
        return false;
    else if(objCell == NULL && (objCell = Create(cellX, cellY)))
        objCell->Init(cellX, cellY, _mapId, this);

    uint32 count = 0;
    float distSq = 0.f;
    bool cansee, isvisible;
    WorldObject *curObj = NULL;
    uint16 phaseMask = obj->GetPhaseMask();
    std::vector<uint32> conditionAccess, eventAccess;
    Player *plObj = obj->IsPlayer() ? castPtr<Player>(obj) : NULL, *plObj2 = NULL;

    // Pushing through active events, todo: conditions
    sWorld.GetActiveEvents(eventAccess);
    if(plObj && plObj->hasGMTag() && plObj->getGMSight() == 1)
        eventAccess.push_back(plObj->getGMEventSight());

    // Process through our cell list
    objCell->FillObjectSets(obj, m_inRangeProcessed, m_inRangeStorage, phaseMask, conditionAccess, eventAccess);

    // Check our already processed objects to see if we need to remove anything
    for(auto itr = m_inRangeProcessed.begin(); itr != m_inRangeProcessed.end(); itr++)
    {
        if((curObj = obj->GetInRangeObject(*itr)) == NULL)
            continue;

        // Check if we're in range and get our distance while we do
        if(!IsInRange(MaxViewDistance, obj, curObj, distSq))
        {   // Remove us if we're not in range
            if( obj->IsPlayer() )
                castPtr<Player>(obj)->RemoveIfVisible(curObj);

            obj->RemoveInRangeObject(curObj);
            continue;
        }

        // Check our virtual callups for inrange shenanigans
        obj->UpdateInRangeObject(curObj);

        // Check unit trigger range mapping
        if(curObj->IsUnit())
            obj->CheckTriggerRange(castPtr<Unit>(obj), distSq);

        if( plObj )
            UpdateObjectVisibility(plObj, curObj);
    }
    // Clear our processed set here
    m_inRangeProcessed.clear();

    // Update inrange mapping based on storage
    for(WorldObject::InRangeObjSet::iterator itr = m_inRangeStorage.begin(); itr != m_inRangeStorage.end(); itr++)
    {
        curObj = (*itr);
        if(!IsInRange(MaxViewDistance, obj, *itr, distSq))
            continue;

        // WorldObject in range, add to set
        obj->AddInRangeObject( curObj );
        curObj->AddInRangeObject( obj );

        if( (plObj2 = curObj->IsPlayer() ? castPtr<Player>(curObj) : NULL ) && plObj2->CanSee( obj ) && !plObj2->IsVisible( obj ) )
        {
            plObj2->AddVisibleObject(obj);
            if(count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2))
                plObj2->PushUpdateBlock(&m_createBuffer, count);
            m_createBuffer.clear();
        }

        if( plObj != NULL && plObj->CanSee( curObj ) && !plObj->IsVisible( curObj ) )
        {
            plObj->AddVisibleObject( curObj );
            if(count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj ))
                plObj->PushUpdateBlock(&m_createBuffer, count);
            m_createBuffer.clear();
        }

        // Check unit trigger range mapping
        if(obj->IsUnit())
            curObj->CheckTriggerRange(castPtr<Unit>(obj), distSq);
        if(curObj->IsUnit())
            obj->CheckTriggerRange(castPtr<Unit>(obj), distSq);
    }
    m_inRangeStorage.clear();
    return true;
}

void MapInstanceObjectRemovalCallback::operator()(WorldObject *obj, WoWGuid guid)
{
    WorldObject *curObj = NULL;
    if((curObj = obj->GetInRangeObject(guid)) == NULL)
        return;
    if(_instance->IsFullRangeObject(curObj))
        return;

    if( obj->IsPlayer() )
        castPtr<Player>(obj)->RemoveIfVisible(curObj);
    if( curObj->IsPlayer() )
        castPtr<Player>( curObj )->RemoveIfVisible(obj);

    curObj->RemoveInRangeObject(obj);
    obj->RemoveInRangeObject(curObj);
}

void MapInstance::RemoveCellData(WorldObject *Obj, std::set<uint32> &set)
{
    if(IsFullRangeObject(Obj))
        return;

    for(auto itr = set.begin(); itr != set.end(); itr++)
    {
        std::pair<uint16, uint16> cellPair = UnitCellManager::unPack(*itr);
        if(MapCell *cell = GetCell(cellPair.first, cellPair.second))
            cell->ProcessSetRemovals(Obj, &_removalCallback);
    }
}

void MapInstance::UpdateInrangeSetOnCells(WorldObject *obj, uint32 startX, uint32 endX, uint32 startY, uint32 endY)
{
    Player *plObj = obj->IsPlayer() ? castPtr<Player>(obj) : NULL;
    std::vector<uint32> conditionAccess, eventAccess;
    sWorld.GetActiveEvents(eventAccess);
    if(plObj && plObj->hasGMTag() && plObj->getGMSight() == 1)
        eventAccess.push_back(plObj->getGMEventSight());

    uint16 phaseMask = obj->GetPhaseMask(), count = 0;
    float distSq = 0.f;

    for (uint32 posX = startX; posX <= endX; posX++ )
        for (uint32 posY = startY; posY <= endY; posY++ )
            if (MapCell *cell = GetCell(posX, posY))
                cell->FillObjectSets(obj, m_inRangeProcessed, m_inRangeStorage, phaseMask, conditionAccess, eventAccess);
    m_inRangeProcessed.clear();

    for(WorldObject::InRangeObjSet::iterator itr = m_inRangeStorage.begin(); itr != m_inRangeStorage.end(); itr++)
    {
        WorldObject *curObj = *itr;
        if( curObj == NULL || curObj == obj )
            continue;

        // Add if we are not ourself and range == 0 or distance is withing range.
        if (!IsInRange(MaxViewDistance, obj, curObj, distSq))
            continue;

        if( obj->IsInRangeSet( curObj ) )
            continue;
        // WorldObject in range, add to set
        obj->AddInRangeObject( curObj );
        curObj->AddInRangeObject( obj );

        if( curObj->IsPlayer() )
        {
            Player *plObj2 = castPtr<Player>( curObj );
            if( plObj2->CanSee( obj ) && !plObj2->IsVisible( obj ) )
            {
                plObj2->AddVisibleObject(obj);
                if(count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2))
                    plObj2->PushUpdateBlock(&m_createBuffer, count);
                m_createBuffer.clear();
            }
        }

        if( plObj && plObj->CanSee( curObj ) && !plObj->IsVisible( curObj ) )
        {
            plObj->AddVisibleObject( curObj );
            if(count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj ))
                plObj->PushUpdateBlock(&m_createBuffer, count);
            m_createBuffer.clear();
        }
    }
    m_inRangeStorage.clear();
}

void MapInstance::UpdateObjectVisibility(Player *plObj, WorldObject *curObj)
{
    ASSERT(plObj && curObj);

    WorldObject::InRangeObjSet::iterator itr;
    bool cansee = plObj->CanSee(curObj), isvisible = plObj->GetVisibility(curObj, &itr);
    if(!cansee && isvisible)
    {
        plObj->PushOutOfRange(curObj->GetGUID());
        plObj->RemoveVisibleObject(itr);
    }
    else if(cansee && !isvisible)
    {
        plObj->AddVisibleObject(curObj);
        if(uint32 count = curObj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj))
            plObj->PushUpdateBlock(&m_createBuffer, count);
        m_createBuffer.clear();
    }
}

void MapInstance::UpdateAllCells(bool apply, uint32 areamask)
{
    if(m_mapPreloading = (areamask == 0))
        sLog.Info("MapInstance", "Updating all cells for map %03u, server might lag.", _mapId);

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

    if(m_mapPreloading == false)
        return;
    sLog.Success("MapInstance", "Cell update for map %03u finished with %u objLoad calls", _mapId, loadCount);
    sLog.Info("MapInstance", "Object Inrange update started for map %03u", _mapId);

    m_poolLock.Acquire();
    for(StoragePool<Creature>::PoolSet::iterator itr = mCreaturePool.begin(); itr != mCreaturePool.end();)
    {
        Creature* ptr = *itr;
        ++itr;

        // We don't want to build inrange maps for deactivated objects
        if(ptr->IsActiveObject() && !ptr->IsActivated())
            continue;

        uint32 startX = GetPosX(ptr->GetPositionX()), startY = GetPosY(ptr->GetPositionY());
        uint32 minX = startX ? startX-1 : 0, maxX = (startX < _sizeY-1 ? startX+1 : _sizeY-1), minY = startY ? startY-1 : 0, maxY = (startY < _sizeY-1 ? startY+1 : _sizeY-1);
        UpdateInrangeSetOnCells(ptr, minX, maxX, minY, maxY);
    }

    for(StoragePool<GameObject>::PoolSet::iterator itr = mGameObjectPool.begin(); itr != mGameObjectPool.end();)
    {
        GameObject* ptr = *itr;
        ++itr;

        // We don't want to build inrange maps for deactivated objects
        if(ptr->IsActiveObject() && !ptr->IsActivated())
            continue;

        uint32 startX = GetPosX(ptr->GetPositionX()), startY = GetPosY(ptr->GetPositionY());
        uint32 minX = startX ? startX-1 : 0, maxX = (startX < _sizeY-1 ? startX+1 : _sizeY-1), minY = startY ? startY-1 : 0, maxY = (startY < _sizeY-1 ? startY+1 : _sizeY-1);
        UpdateInrangeSetOnCells(ptr, minX, maxX, minY, maxY);
    }
    m_poolLock.Release();

    sLog.Success("MapInstance", "Object Inrange update for map %03u completed", _mapId);
    m_mapPreloading = false;
}

void MapInstance::UpdateCellActivity(uint32 x, uint32 y, int radius)
{
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
            // Check cell activity for spawn loading
            if (objCell == NULL && _CellActive(posX, posY))
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
                if (_CellActive(posX, posY) && !objCell->IsActive())
                {
                    objCell->SetActivity(true);
                    objCell->LoadCellData(sp);
                } else if (!_CellActive(posX, posY) && objCell->IsActive()) // Cell is no longer active
                    objCell->SetActivity(false);
            }
        }
    }
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

bool MapInstance::_CellActive(uint32 x, uint32 y)
{
    uint32 endX = ((x+1) <= _sizeX) ? x + 1 : (_sizeX-1);
    uint32 endY = ((y+1) <= _sizeY) ? y + 1 : (_sizeY-1);
    uint32 startX = x > 0 ? x - 1 : 0;
    uint32 startY = y > 0 ? y - 1 : 0;
    uint32 posX, posY;

    MapCell *objCell;
    for (posX = startX; posX <= endX; posX++ )
    {
        for (posY = startY; posY <= endY; posY++ )
        {
            objCell = GetCell(posX, posY);
            if (objCell)
            {
                if (objCell->HasPlayers() || objCell->IsForcedActive() )
                    return true;
            }
        }
    }
    return false;
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
    // Only process object update calls if we're a player or have players inrange
    if(!(obj->IsPlayer() || obj->HasInRangePlayers()))
        return;

    m_updateMutex.Acquire();
    if(_updates.find(obj) == _updates.end())
        _updates.insert(obj);
    m_updateMutex.Release();
}

void MapInstance::ObjectLocationChange(WorldObject *obj)
{
    m_updateMutex.Acquire();
    if(_movedObjects.find(obj) == _movedObjects.end())
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

        if(!ptr->raidgrouponlysent && ptr->GetSession())
            ptr->GetSession()->SendPacket(&data);
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
    case HIGHGUID_TYPE_TRANSPORTER: return objmgr.GetTransporter(guid.getLow());
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
        uint32 tileX = (GetPosX(obj->GetPositionX())/8);
        uint32 tileY = (GetPosY(obj->GetPositionY())/8);
        if(sVMapInterface.IsActiveTile(_mapId, tileX, tileY))
            return true;
    }

    return false;
}

void MapInstance::_ProcessInputQueue()
{
    if(!m_objectinsertlock.AttemptAcquire())
        return;
    for(ObjectSet::iterator i = m_objectinsertpool.begin(); i != m_objectinsertpool.end(); i++)
        (*i)->PushToWorld(this);
    m_objectinsertpool.clear();
    m_objectinsertlock.Release();
}

void MapInstance::_PerformPlayerUpdates(uint32 msTime, uint32 uiDiff)
{
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

    m_updateMutex.Acquire();
    if(!m_removeQueue.empty())
    {
        Player *plObj = m_removeQueue.front();
        m_removeQueue.pop();

        // Clear any updates pending
        _processQueue.erase(plObj);
        plObj->PopPendingUpdates();

        // If it's a player and he's inside boundaries - update his nearby cells
        if(plObj->GetPositionX() <= _maxX && plObj->GetPositionX() >= _minX &&
            plObj->GetPositionY() <= _maxY && plObj->GetPositionY() >= _minY)
        {
            uint32 x = GetPosX(plObj->GetPositionX());
            uint32 y = GetPosY(plObj->GetPositionY());
            UpdateCellActivity(x, y, 2);
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

void MapInstance::_PerformCreatureUpdates(uint32 msTime, uint32 uiDiff)
{
    m_poolLock.Acquire();
    mCreaturePool.Update(msTime, uiDiff);
    m_poolLock.Release();
}

void MapInstance::_PerformObjectUpdates(uint32 msTime, uint32 uiDiff)
{
    m_poolLock.Acquire();
    mGameObjectPool.Update(msTime, uiDiff);
    m_poolLock.Release();
}

void MapInstance::_PerformDynamicObjectUpdates(uint32 msTime, uint32 uiDiff)
{
    m_poolLock.Acquire();
    mDynamicObjectPool.Update(msTime, uiDiff);
    m_poolLock.Release();
}

void MapInstance::_PerformDelayedSpellUpdates(uint32 msTime, uint32 uiDiff)
{
    m_poolLock.Acquire();
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
    m_poolLock.Release();
}

void MapInstance::_PerformUnitPathUpdates(uint32 msTime, uint32 uiDiff)
{
    m_poolLock.Acquire();
    mUnitPathPool.Update(msTime, uiDiff);
    m_poolLock.Release();
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
    if(!_updates.size() && !_processQueue.size() && !_movedObjects.size())
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
            // need to be different! ;)
            if( count = wObj->BuildValuesUpdateBlockForPlayer(&m_updateBuffer, UF_FLAGMASK_SELF) )
                castPtr<Player>( wObj )->PushUpdateBlock( &m_updateBuffer, count );
            m_updateBuffer.clear();
        }

        wObj->OnUpdateProcess();
        if(wObj->HasInRangePlayers())
        {
            // build the update
            count = wObj->BuildValuesUpdateBlockForPlayer(&m_updateBuffer, UF_FLAGMASK_PUBLIC);
            for(WorldObject::InRangeArray::iterator itr = wObj->GetInRangePlayerSetBegin(); itr != wObj->GetInRangePlayerSetEnd();)
            {
                Player *plrTarget = wObj->GetInRangeObject<Player>(*itr);
                ++itr;
                if(plrTarget == NULL || !plrTarget->IsVisible(wObj))
                    continue; // Make sure that the target player can see us.
                uint32 targetFlag = wObj->GetUpdateFlag(plrTarget);
                if(targetFlag & UF_FLAG_PARTY_MEMBER)
                    m_partyTargets.insert(plrTarget);
                else if(targetFlag & UF_FLAG_OWNER)
                    m_petTargets.insert(plrTarget);
                else if(count > 0 && m_updateBuffer.size())
                    plrTarget->PushUpdateBlock(&m_updateBuffer, count);
            }
            m_updateBuffer.clear();

            if(m_partyTargets.size() && (count = wObj->BuildValuesUpdateBlockForPlayer(&m_updateBuffer, UF_FLAGMASK_PARTY_MEMBER)))
                for(PlayerSet::iterator itr = m_partyTargets.begin(); itr != m_partyTargets.end(); itr++)
                    (*itr)->PushUpdateBlock( &m_updateBuffer, count );
            m_updateBuffer.clear();
            m_partyTargets.clear();

            if(m_petTargets.size() && (count = wObj->BuildValuesUpdateBlockForPlayer(&m_updateBuffer, UF_FLAGMASK_OWN_PET)))
                for(PlayerSet::iterator itr = m_petTargets.begin(); itr != m_petTargets.end(); itr++)
                    (*itr)->PushUpdateBlock( &m_updateBuffer, count );
            m_updateBuffer.clear();
            m_petTargets.clear();
        }
        wObj->ClearUpdateMask();
    }
    _updates.clear();

    while(!_movedObjects.empty())
    {
        WorldObject *obj = *_movedObjects.begin();
        _movedObjects.erase(_movedObjects.begin());
        ChangeObjectLocation(obj);
    }
    _movedObjects.clear();

    Player* plyr;
    // generate pending a9packets and send to clients.
    while(!_processQueue.empty())
    {
        plyr = *_processQueue.begin();
        if(plyr && plyr->GetMapInstance() == this)
            plyr->PopPendingUpdates();
        _processQueue.erase(_processQueue.begin());
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
    if(c == NULL || c->HasPlayers() || _CellActive(x,y) || !c->IsUnloadPending())
        return;

    sLog.Debug("MapInstance","Unloading Cell [%d][%d] on map %d (instance %d)...", x, y, _mapId, m_instanceID);
    c->Unload();
}

bool MapInstance::IsInRange(float fRange, WorldObject* obj, WorldObject* currentobj, float &distOut)
{
    // First distance check, are we in range?
    if((distOut = currentobj->GetDistance2dSq(obj)) > fRange)
        return false;
    float heightDifference = RONIN_UTIL::Diff(obj->GetPositionZ(), currentobj->GetPositionZ());
    if((heightDifference*heightDifference) > fRange)
        return false;
    return true;
}

void MapInstance::SendMessageToCellPlayers(WorldObject* obj, WorldPacket * packet, uint32 cell_radius /* = 2 */)
{
    /*uint32 cellX = GetPosX(obj->GetPositionX());
    uint32 cellY = GetPosY(obj->GetPositionY());
    uint32 endX = ((cellX+cell_radius) <= _sizeX) ? cellX + cell_radius : (_sizeX-1);
    uint32 endY = ((cellY+cell_radius) <= _sizeY) ? cellY + cell_radius : (_sizeY-1);
    uint32 startX = (cellX-cell_radius) > 0 ? cellX - cell_radius : 0;
    uint32 startY = (cellY-cell_radius) > 0 ? cellY - cell_radius : 0;

    MapCell *cell;
    uint32 posX, posY;
    MapCell::CellObjectSet::iterator iter, iend;
    for (posX = startX; posX <= endX; ++posX )
    {
        for (posY = startY; posY <= endY; ++posY )
        {
            cell = GetCell(posX, posY);
            if (cell && cell->HasPlayers() )
            {
                iter = cell->Begin();
                iend = cell->End();
                for(; iter != iend; iter++)
                {
                    if((*iter)->IsPlayer())
                    {
                        castPtr<Player>(*iter)->GetSession()->SendPacket(packet);
                    }
                }
            }
        }
    }*/
}

void MapInstance::SendChatMessageToCellPlayers(WorldObject* obj, WorldPacket * packet, uint32 cell_radius, uint32 langpos, uint32 guidPos, int32 lang, WorldSession * originator)
{
    /*uint32 cellX = GetPosX(obj->GetPositionX());
    uint32 cellY = GetPosY(obj->GetPositionY());
    uint32 endX = ((cellX+cell_radius) <= _sizeX) ? cellX + cell_radius : (_sizeX-1);
    uint32 endY = ((cellY+cell_radius) <= _sizeY) ? cellY + cell_radius : (_sizeY-1);
    uint32 startX = (cellX-cell_radius) > 0 ? cellX - cell_radius : 0;
    uint32 startY = (cellY-cell_radius) > 0 ? cellY - cell_radius : 0;

    uint32 posX, posY;
    MapCell *cell;
    MapCell::CellObjectSet::iterator iter, iend;
    for (posX = startX; posX <= endX; ++posX )
    {
        for (posY = startY; posY <= endY; ++posY )
        {
            cell = GetCell(posX, posY);
            if (cell && cell->HasPlayers() )
            {
                iter = cell->Begin();
                iend = cell->End();
                for(; iter != iend; iter++)
                {
                    if((*iter)->IsPlayer() && originator->GetPlayer()->PhasedCanInteract((*iter))) // Matching phases.
                        castPtr<Player>(*iter)->GetSession()->SendChatPacket(packet, langpos, guidPos, lang, originator);
                }
            }
        }
    }*/
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

Creature* MapInstance::CreateCreature(WoWGuid guid, uint32 entry)
{
    if(!guid.empty())
        entry = guid.getEntry();
    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(entry);
    if(ctrData == NULL || !ctrData->HasValidModelData())
    {
        sLog.Warning("MapInstance", "Skipping CreateCreature for entry %u due to incomplete database.", entry);
        return NULL;
    }

    uint16 highGuid = (ctrData->vehicleEntry > 0 ? HIGHGUID_TYPE_VEHICLE : HIGHGUID_TYPE_UNIT);
    if(guid.empty() && IsInstance())
        guid = MAKE_NEW_GUID(sInstanceMgr.AllocateCreatureGuid(), entry, highGuid);
    else if(guid.empty())
        guid = MAKE_NEW_GUID(++m_CreatureHighGuid, entry, highGuid);
    ASSERT( guid.getHigh() == highGuid );

    Creature *cr = new Creature(ctrData, guid);
    cr->Init();
    return cr;
}

Summon* MapInstance::CreateSummon(uint32 entry)
{
    return NULL;

    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(entry);
    if(ctrData == NULL)
    {
        sLog.Warning("MapInstance", "Skipping CreateSummon for entry %u due to incomplete database.", entry);
        return NULL;
    }

    Summon *sum = new Summon(ctrData, MAKE_NEW_GUID(++m_CreatureHighGuid, entry, HIGHGUID_TYPE_UNIT));
    sum->Init();
    ASSERT( sum->GetHighGUID() == HIGHGUID_TYPE_UNIT );
    return sum;
}

GameObject* MapInstance::CreateGameObject(WoWGuid guid, uint32 entry)
{
    if(!guid.empty())
        entry = guid.getEntry();
    GameObjectInfo *goi = GameObjectNameStorage.LookupEntry( entry );
    if( goi == NULL )
    {
        sLog.Warning("MapInstance", "Skipping CreateGameObject for entry %u due to incomplete database.", entry);
        return NULL;
    }
    ASSERT( guid.getHigh() == HIGHGUID_TYPE_GAMEOBJECT );

    GameObject *go = new GameObject(goi, MAKE_NEW_GUID(++m_GOHighGuid, entry, HIGHGUID_TYPE_GAMEOBJECT));
    go->Init();
    return go;
}

DynamicObject* MapInstance::CreateDynamicObject()
{
    DynamicObject* dyn = new DynamicObject(HIGHGUID_TYPE_DYNAMICOBJECT, (++m_DynamicObjectHighGuid));
    dyn->Init();
    ASSERT( dyn->GetHighGUID() == HIGHGUID_TYPE_DYNAMICOBJECT );
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

            ptr->GetSession()->SendPacket(pData);
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

            ptr->GetSession()->SendPacket(&data);
        }
    }
}
