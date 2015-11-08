/***
 * Demonstrike Core
 */

//
// mapMgr->cpp
//

#include "StdAfx.h"

#define MAP_MGR_UPDATE_PERIOD 50
#define MAPMGR_INACTIVE_MOVE_TIME 10
extern bool bServerShutdown;

MapMgr::MapMgr(Map *map, uint32 mapId, uint32 instanceid) : ThreadContext(), CellHandler<MapCell>(map), _mapId(mapId), eventHolder(instanceid)
{
    m_instanceID = instanceid;
    pMapInfo = WorldMapInfoStorage.LookupEntry(mapId);
    pdbcMap = dbcMap.LookupEntry(mapId);
    m_UpdateDistance = MAX_VIEW_DISTANCE;
    iInstanceMode = 0;

    m_GOHighGuid = 0;
    m_CreatureHighGuid = 0;
    m_VehicleHighGuid = 0;
    m_DynamicObjectHighGuid=0;
    lastUnitUpdate = getMSTime();
    lastGameobjectUpdate = getMSTime();
    m_battleground = NULL;

    m_holder = &eventHolder;
    m_event_Instanceid = eventHolder.GetInstanceID();
    forced_expire = false;
    InactiveMoveTime = 0;
    mLoopCounter = 0;
    pInstance = NULL;
    thread_kill_only = false;
    thread_running = false;

    // buffers
    m_updateBuffer.reserve(50000);
    m_createBuffer.reserve(20000);

    m_updateBuildBuffer.reserve(48000);
    m_compressionBuffer.reserve(54000); // uint32 destsize = size + size/10 + 16;

    m_PlayerStorage.clear();
    m_PetStorage.clear();
    m_DynamicObjectStorage.clear();

    _combatProgress.clear();
    _updates.clear();
    _processQueue.clear();
    MapSessions.clear();

    ActiveLock.Acquire();
    activeGameObjects.clear();
    activeCreatures.clear();
    activeVehicles.clear();
    ActiveLock.Release();

    m_corpses.clear();
    _sqlids_creatures.clear();
    _sqlids_gameobjects.clear();
    _reusable_guids_creature.clear();
    _reusable_guids_vehicle.clear();
}

void MapMgr::Init(bool Instance)
{
    m_stateManager = new WorldStateManager(this);
}

void MapMgr::Destruct()
{
    sEventMgr.RemoveEvents(this);
    sEventMgr.RemoveEventHolder(m_instanceID);

    if( m_stateManager != NULL )
    {
        delete m_stateManager;
        m_stateManager = NULL;
    }

    // Remove objects
    MapCell* cell = NULL;
    if(_cells)
    {
        for (uint32 i = 0; i < _sizeX; i++)
        {
            if(_cells[i] != NULL)
            {
                for (uint32 j = 0; j < _sizeY; j++)
                {
                    cell = _cells[i][j];
                    if(cell != NULL)
                    {
                        cell->_unloadpending = false;
                        cell->RemoveObjects();
                        cell = NULL;
                    }
                }
            }
        }
    }

    Corpse* pCorpse;
    if(m_corpses.size())
    {
        for(std::unordered_set<Corpse* >::iterator itr = m_corpses.begin(); itr != m_corpses.end();)
        {
            pCorpse = *itr;
            ++itr;

            if(pCorpse->IsInWorld())
                pCorpse->RemoveFromWorld(false);

            pCorpse->Destruct();
            pCorpse = NULL;
        }
        m_corpses.clear();
    }

    //Clear our remaining containers
    m_PlayerStorage.clear();
    m_PetStorage.clear();
    m_DynamicObjectStorage.clear();
    m_CreatureStorage.clear();
    m_gameObjectStorage.clear();

    _combatProgress.clear();
    _updates.clear();
    _processQueue.clear();
    MapSessions.clear();
    pMapInfo = NULL;
    pdbcMap = NULL;

    ActiveLock.Acquire();
    activeVehicles.clear();
    activeCreatures.clear();
    activeGameObjects.clear();
    ActiveLock.Release();

    _sqlids_creatures.clear();
    _sqlids_gameobjects.clear();
    _reusable_guids_vehicle.clear();
    _reusable_guids_creature.clear();

    m_battleground = NULL;

    sLog.Notice("MapMgr", "Instance %u shut down. (%s)" , m_instanceID, GetBaseMap()->GetName());
}

MapMgr::~MapMgr()
{

}

void MapMgr::EventPushObjectToSelf(WorldObject *obj)
{
    obj->PushToWorld(this);
}

void MapMgr::PushObject(WorldObject* obj)
{
    /////////////
    // Assertions
    /////////////
    ASSERT(obj);

    if(obj->GetTypeId() == TYPEID_CORPSE)
        m_corpses.insert( castPtr<Corpse>(obj) );

    Player* plObj = NULL;
    if(obj->IsPlayer())
    {
        plObj = castPtr<Player>( obj );
        if(plObj == NULL)
        {
            sLog.Debug("MapMgr","Could not get a valid playerobject from object while trying to push to world");
            return;
        }
        plObj->ClearInRangeSet();

        WorldSession * plSession = plObj->GetSession();
        if(plSession == NULL)
        {
            sLog.Debug("MapMgr","Could not get a valid session for player while trying to push to world");
            return;
        }
    } else obj->ClearInRangeSet();

    ///////////////////////
    // Get cell coordinates
    ///////////////////////

    ASSERT(obj->GetMapId() == _mapId);

    ASSERT(obj->GetPositionZ() < _maxY && obj->GetPositionZ() > _minY);
    ASSERT(_cells);

    float mx = obj->GetPositionX();
    float my = obj->GetPositionY();
    uint32 cx = GetPosX(mx);
    uint32 cy = GetPosY(my);

    if( mx > _maxX || my > _maxY ||
        mx < _minX || my < _minY ||
        cx >= _sizeX || cy >= _sizeY)
    {
        if( plObj != NULL )
        {
            if(plObj->GetBindMapId() != GetMapId())
            {
                plObj->SafeTeleport(plObj->GetBindMapId(),0,plObj->GetBindPositionX(),plObj->GetBindPositionY(),plObj->GetBindPositionZ(),0);
                plObj->GetSession()->SystemMessage("Teleported you to your ronin location as you ended up on the wrong map.");
                return;
            }
            else
            {
                obj->GetPositionV()->ChangeCoords(plObj->GetBindPositionX(),plObj->GetBindPositionY(),plObj->GetBindPositionZ(),0);
                plObj->GetSession()->SystemMessage("Teleported you to your ronin location as you were out of the map boundaries.");
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

    uint32 count = 0, endX = (cx <= _sizeX) ? cx + 1 : (_sizeX-1), endY = (cy <= _sizeY) ? cy + 1 : (_sizeY-1);
    uint32 startX = cx > 0 ? cx - 1 : 0, startY = cy > 0 ? cy - 1 : 0;
    if(plObj)
    {
        sLog.Debug("MapMgr","Creating player "I64FMT" for himself.", obj->GetGUID());
        count = plObj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj);
        plObj->PushUpdateBlock(&m_createBuffer, count);
        m_createBuffer.clear();
    }

    //Add to the cell's object list
    objCell->AddObject(obj);

    obj->SetMapCell(objCell);
     //Add to the mapmanager's object list
    if(plObj)
    {
        m_PlayerStorage[plObj->GetLowGUID()] = plObj;
        UpdateCellActivity(cx, cy, 2);
    }
    else
    {
        switch(obj->GetHighGUID())
        {
        case HIGHGUID_TYPE_PET:
            m_PetStorage[obj->GetLowGUID()] = castPtr<Pet>( obj );
            break;

        case HIGHGUID_TYPE_UNIT:
            {
                Creature *creature = castPtr<Creature>(obj);
                ASSERT((obj->GetLowGUID()) <= m_CreatureHighGuid);
                m_CreatureStorage[obj->GetLowGUID()] = creature;
                if(creature->IsSpawn())
                    _sqlids_creatures.insert(std::make_pair( creature->GetSQL_id(), creature ) );
                TRIGGER_INSTANCE_EVENT( this, OnCreaturePushToWorld )( creature );
            }break;

        case HIGHGUID_TYPE_VEHICLE:
            {
                Vehicle *vehicle = castPtr<Vehicle>(obj);
                ASSERT((obj->GetLowGUID()) <= m_VehicleHighGuid);
                m_VehicleStorage[obj->GetLowGUID()] = vehicle;
                if(vehicle->IsSpawn())
                    _sqlids_creatures.insert(std::make_pair( vehicle->GetSQL_id(), vehicle ) );
                TRIGGER_INSTANCE_EVENT( this, OnCreaturePushToWorld )( vehicle );
            }break;
        case HIGHGUID_TYPE_GAMEOBJECT:
            {
                GameObject* go = castPtr<GameObject>(obj);
                m_gameObjectStorage.insert(std::make_pair(obj->GetLowGUID(), go));
                if( go->m_spawn != NULL)
                    _sqlids_gameobjects.insert(std::make_pair(go->m_spawn->id, go ) );
                TRIGGER_INSTANCE_EVENT( this, OnGameObjectPushToWorld )( go );
                sVMapInterface.LoadGameobjectModel(obj->GetGUID(), _mapId, go->GetDisplayId(), go->GetFloatValue(OBJECT_FIELD_SCALE_X), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), go->GetOrientation(), go->GetInstanceID(), go->GetPhaseMask());
            }break;

        case HIGHGUID_TYPE_DYNAMICOBJECT:
            m_DynamicObjectStorage[obj->GetLowGUID()] = castPtr<DynamicObject>(obj);
            break;
        }
    }

    // Handle activation of that object.
    if(objCell->IsActive() && obj->CanActivate())
        obj->Activate(this);

    // Add the session to our set if it is a player.
    if(plObj)
    {
        MapSessions.insert(plObj->GetSession());

        // Change the instance ID, this will cause it to be removed from the world thread (return value 1)
        plObj->GetSession()->SetInstance(GetInstanceID());

        // Update our player's zone
        plObj->UpdateAreaInfo(this);

        /* Add the zone wide objects */
        if(m_zoneRangelessObjects[plObj->GetZoneId()].size())
        {
            for(std::set<WorldObject* >::iterator itr = m_zoneRangelessObjects[plObj->GetZoneId()].begin(); itr != m_zoneRangelessObjects[plObj->GetZoneId()].end(); itr++)
            {
                count = (*itr)->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj);
                plObj->PushUpdateBlock(&m_createBuffer, count);
                m_createBuffer.clear();
            }
        }
    }

    if(plObj && InactiveMoveTime && !forced_expire)
        InactiveMoveTime = 0;

    //////////////////////
    // Build in-range data
    //////////////////////
    UpdateInrangeSetOnCells(obj->GetGUID(), startX, endX, startY, endY);
}

void MapMgr::RemoveObject(WorldObject* obj, bool free_guid)
{
    /////////////
    // Assertions
    /////////////

    ASSERT(obj);
    ASSERT(obj->GetMapId() == _mapId);
    ASSERT(_cells);

    if(obj->Active)
        obj->Deactivate(this);

    m_updateMutex.Acquire();
    _updates.erase(obj);
    m_updateMutex.Release();
    obj->ClearUpdateMask();
    Player* plObj = (obj->IsPlayer()) ? castPtr<Player>( obj ) : NULL;

    ///////////////////////////////////////
    // Remove object from all needed places
    ///////////////////////////////////////

    switch(obj->GetHighGUID())
    {
    case HIGHGUID_TYPE_VEHICLE:
        {
            ASSERT(obj->GetLowGUID() <= m_VehicleHighGuid);
            if(castPtr<Vehicle>(obj)->IsSpawn()) _sqlids_creatures.erase(castPtr<Vehicle>(obj)->GetSQL_id());
            if(free_guid) _reusable_guids_vehicle.push_back(obj->GetLowGUID());
            m_VehicleStorage.erase(obj->GetLowGUID());
            TRIGGER_INSTANCE_EVENT( this, OnCreatureRemoveFromWorld )( castPtr<Vehicle>(obj) );
        }break;

    case HIGHGUID_TYPE_UNIT:
        {
            ASSERT(obj->GetLowGUID() <= m_CreatureHighGuid);
            if(castPtr<Creature>(obj)->IsSpawn()) _sqlids_creatures.erase(castPtr<Creature>(obj)->GetSQL_id());
            if(free_guid) _reusable_guids_creature.push_back(obj->GetLowGUID());
            m_CreatureStorage.erase(obj->GetLowGUID());
            TRIGGER_INSTANCE_EVENT( this, OnCreatureRemoveFromWorld )( castPtr<Creature>(obj) );
        }break;

    case HIGHGUID_TYPE_PET:
        {
            // check iterator
            if( __pet_iterator != m_PetStorage.end() && __pet_iterator->second == castPtr<Pet>(obj) )
                ++__pet_iterator;
            m_PetStorage.erase(obj->GetLowGUID());
        }break;

    case HIGHGUID_TYPE_DYNAMICOBJECT:
        {
            m_DynamicObjectStorage.erase(obj->GetLowGUID());
        }break;

    case HIGHGUID_TYPE_GAMEOBJECT:
        {
            m_gameObjectStorage.erase(obj->GetLowGUID());
            if(castPtr<GameObject>(obj)->m_spawn != NULL) _sqlids_gameobjects.erase(castPtr<GameObject>(obj)->m_spawn->id);
            TRIGGER_INSTANCE_EVENT( this, OnGameObjectRemoveFromWorld )( castPtr<GameObject>(obj) );
            sVMapInterface.UnLoadGameobjectModel(obj->GetGUID(), m_instanceID, _mapId);
        }break;

    case HIGHGUID_TYPE_PLAYER:
        {
            // check iterator
            if( __player_iterator != m_PlayerStorage.end() && __player_iterator->second == castPtr<Player>(obj) )
                ++__player_iterator;
        }break;
    }

    // That object types are not map objects. TODO: add AI groups here?
    if(obj->GetTypeId() == TYPEID_ITEM || obj->GetTypeId() == TYPEID_CONTAINER)
        return;

    if(obj->GetTypeId() == TYPEID_CORPSE)
        m_corpses.erase( castPtr<Corpse>(obj) );

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

    // Clear any updates pending
    if(obj->IsPlayer())
    {
        _processQueue.erase(castPtr<Player>(obj));
        castPtr<Player>( obj )->ClearAllPendingUpdates();
    }

    // Remove object from all objects 'seeing' him
    for (WorldObject::InRangeMap::iterator iter = obj->GetInRangeMapBegin(); iter != obj->GetInRangeMapEnd(); iter++)
    {
        assert(iter->second);
        if(iter->second->IsPlayer())
        {
            Player *plr = castPtr<Player>(iter->second);
            if( plr->IsVisible( obj ) && plr->GetTransportGuid() != obj->GetGUID())
                plr->PushOutOfRange(obj->GetGUID());
            obj->DestroyForPlayer(plr, obj->IsGameObject());
        }
        iter->second->RemoveInRangeObject(obj);
    }

    // Clear object's in-range set
    obj->ClearInRangeSet();

    if(plObj)
    {
        // If it's a player and he's inside boundaries - update his nearby cells
        if(obj->GetPositionX() <= _maxX && obj->GetPositionX() >= _minX &&
            obj->GetPositionY() <= _maxY && obj->GetPositionY() >= _minY)
        {
            uint32 x = GetPosX(obj->GetPositionX());
            uint32 y = GetPosY(obj->GetPositionY());
            UpdateCellActivity(x, y, 2);
        }
        m_PlayerStorage.erase( castPtr<Player>( obj )->GetLowGUID() );

        // Setting an instance ID here will trigger the session to be removed
        // by MapMgr::run(). :)
        plObj->GetSession()->SetInstance(0);

        // Add it to the global session set (if it's not being deleted).
        if(!plObj->GetSession()->bDeleted)
            sWorld.AddGlobalSession(plObj->GetSession());
    }

    if(!HasPlayers() && !InactiveMoveTime && !forced_expire && GetMapInfo()->type != INSTANCE_NULL)
    {
        InactiveMoveTime = UNIXTIME + (MAPMGR_INACTIVE_MOVE_TIME * 60);    // 10 mins -> move to inactive
    }
}

void MapMgr::ChangeObjectLocation( WorldObject* obj )
{
    if( obj->GetMapMgr() != this )
        return;

    WorldObject* curObj = NULL;
    Player* plObj = obj->IsPlayer() ? castPtr<Player>(obj) : NULL;
    ///////////////////////////////////////
    // Update in-range data for old objects
    ///////////////////////////////////////
    uint32 lastZone = obj->GetLastMovementZone(), currZone = obj->GetZoneId();
    if(lastZone != obj->GetZoneId())
    {
        if(m_zoneRangelessObjects[lastZone].size())
        {
            for(std::set<WorldObject*>::iterator itr = m_zoneRangelessObjects[lastZone].begin(); itr != m_zoneRangelessObjects[lastZone].end(); itr++)
                if(!(*itr)->IsTransport() || (!obj->IsUnit() || castPtr<Unit>(obj)->GetTransportGuid() != (*itr)->GetGUID()))
                    obj->RemoveInRangeObject(*itr);
        }

        if(m_zoneRangelessObjects[currZone].size())
        {
            for(std::set<WorldObject*>::iterator itr = m_zoneRangelessObjects[currZone].begin(); itr != m_zoneRangelessObjects[currZone].end(); itr++)
                obj->AddInRangeObject(*itr);
        }
    }
    obj->SetLastMovementZone(currZone);

    if(obj->GetMapMgr() != this)
    {
        if(!obj->HasInRangeObjects())
            return;

        for (WorldObject::InRangeMap::iterator iter = obj->GetInRangeMapBegin(); iter != obj->GetInRangeMapEnd(); iter++)
        {
            curObj = iter->second;
            if(curObj == obj)
                continue;

            if(curObj->GetMapMgr() == this)
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
            if(obj->IsPlayer())
            {
                Player* plr = castPtr<Player>( obj );
                if(plr->GetBindMapId() != GetMapId())
                {
                    plr->SafeTeleport(plr->GetBindMapId(),0,plr->GetBindPositionX(),plr->GetBindPositionY(),plr->GetBindPositionZ(),0);
                    plr->GetSession()->SystemMessage("Teleported you to your ronin location as you were out of the map boundaries.");
                    return;
                }
                else
                {
                    obj->GetPositionV()->ChangeCoords(plr->GetBindPositionX(),plr->GetBindPositionY(),plr->GetBindPositionZ(),0);
                    plr->GetSession()->SystemMessage("Teleported you to your ronin location as you were out of the map boundaries.");
                    WorldPacket * data = plr->BuildTeleportAckMsg(plr->GetPosition());
                    plr->GetSession()->SendPacket(data);
                    delete data;
                }
            }
            else
            {
                obj->GetPositionV()->ChangeCoords(0,0,0,0);
            }
        }

        MapCell *pOldCell = obj->GetMapCell(), *objCell = NULL;
        uint32 cellX = GetPosX(obj->GetPositionX()), cellY = GetPosY(obj->GetPositionY());
        if(cellX < _sizeX && cellY < _sizeY)
        {
            objCell = GetCell(cellX, cellY);
            if (!objCell)
            {
                objCell = Create(cellX,cellY);
                objCell->Init(cellX, cellY, _mapId, this);
            }
        }

        ObjectMovingCells(obj, pOldCell, objCell);

        //////////////////////////////////////
        // Update in-range set for new objects
        //////////////////////////////////////
        uint32 startX = cellX > 0 ? cellX - 1 : 0;
        uint32 startY = cellY > 0 ? cellY - 1 : 0;
        uint32 endX = cellX <= _sizeX ? cellX + 1 : (_sizeX-1);
        uint32 endY = cellY <= _sizeY ? cellY + 1 : (_sizeY-1);

        ///////////////////////////////////////
        // Update in-range data for old objects
        ///////////////////////////////////////
        if(!obj->HasInRangeObjects())
            UpdateInrangeSetOnCells(obj->GetGUID(), startX, endX, startY, endY);
        else if(m_rangelessObjects.find(obj) == m_rangelessObjects.end())
        {
            // Construct set of inrange objs
            WorldObject::InRangeMap m_inRange(*obj->GetInRangeMap());

            MapCell* cell;
            Player* plObj2;
            uint32 posX, posY;
            bool cansee, isvisible;
            for (posX = startX; posX <= endX; posX++ )
            {
                for (posY = startY; posY <= endY; posY++ )
                {
                    if (cell = GetCell(posX, posY))
                    {
                        ObjectSet::iterator iter = cell->Begin();
                        for(ObjectSet::iterator iter = cell->Begin(); iter != cell->End(); iter++)
                        {
                            curObj = *iter;
                            if( curObj == NULL || curObj == obj )
                                continue;

                            // We only need to parse objects that are in range
                            if (!IsInRange(m_UpdateDistance, obj, curObj))
                                continue;
                            // We've parsed the object here, erase from our other map
                            m_inRange.erase(curObj->GetGUID());

                            if(obj->IsInRangeMap(curObj))
                            {
                                // Check visiblility
                                if( curObj->IsPlayer() )
                                    UpdateObjectVisibility(castPtr<Player>(curObj), obj);

                                if( plObj ) UpdateObjectVisibility(plObj, curObj);
                            }
                            else
                            {
                                // WorldObject in range, add to set
                                obj->AddInRangeObject( curObj );
                                curObj->AddInRangeObject( obj );

                                if( curObj->IsPlayer() )
                                {
                                    plObj2 = castPtr<Player>( curObj );
                                    if( plObj2->CanSee( obj ) && !plObj2->IsVisible( obj ) )
                                    {
                                        plObj2->AddVisibleObject(obj);
                                        if(int count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2))
                                        {
                                            plObj2->PushUpdateBlock(&m_createBuffer, count);
                                            m_createBuffer.clear();
                                        }
                                    }
                                }

                                if( plObj != NULL )
                                {
                                    if( plObj->CanSee( curObj ) && !plObj->IsVisible( curObj ) )
                                    {
                                        plObj->AddVisibleObject( curObj );
                                        if(int count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj ))
                                        {
                                            plObj->PushUpdateBlock( &m_createBuffer, count );
                                            m_createBuffer.clear();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            for(WorldObject::InRangeMap::iterator itr = m_inRange.begin(); itr != m_inRange.end(); itr++)
            {
                curObj = itr->second;
                if(m_rangelessObjects.find(curObj) != m_rangelessObjects.end())
                    continue;
                // We cannot remove our current transport from inrange set
                if(obj->IsUnit() && castPtr<Unit>(obj)->GetTransportGuid())
                    if(curObj->GetGUID() == castPtr<Unit>(obj)->GetTransportGuid())
                        continue;

                if( obj->IsPlayer() )
                    castPtr<Player>(obj)->RemoveIfVisible(curObj);

                if( curObj->IsPlayer() )
                    castPtr<Player>( curObj )->RemoveIfVisible(obj);

                curObj->RemoveInRangeObject(obj);
                obj->RemoveInRangeObject(curObj);
            }
        }

        if(Unit *uObj = (obj->IsUnit() ? castPtr<Unit>(obj) : NULL))
            uObj->OnPositionChange();
    }

}

void MapMgr::UpdateInrangeSetOnCells(uint64 guid, uint32 startX, uint32 endX, uint32 startY, uint32 endY)
{
    MapCell* cell;
    uint32 posX, posY;
    for (posX = startX; posX <= endX; posX++ )
    {
        for (posY = startY; posY <= endY; posY++ )
        {
            if (cell = GetCell(posX, posY))
                UpdateInRangeSet(guid, cell);
        }
    }
}

void MapMgr::ObjectMovingCells(WorldObject *obj, MapCell *oldCell, MapCell *newCell)
{
    if(newCell == NULL && oldCell == NULL)
        return;

    // If object moved cell
    if (newCell != oldCell)
    {
        // THIS IS A HACK!
        // Current code, if a creature on a long waypoint path moves from an active
        // cell into an inactive one, it will disable itself and will never return.
        // This is to prevent cpu leaks. I will think of a better solution very soon :P
        if(!newCell->IsActive() && !obj->IsPlayer() && obj->Active)
            obj->Deactivate(this);

        if(oldCell) oldCell->RemoveObject(obj);
        if(newCell) newCell->AddObject(obj);
        obj->SetMapCell(newCell);

        // if player we need to update cell activity
        // radius = 2 is used in order to update both
        // old and new cells
        if(newCell && obj->IsPlayer())
        {
            uint32 cellX = newCell->GetPositionX(), cellY = newCell->GetPositionY();
            // have to unlock/lock here to avoid a deadlock situation.
            UpdateCellActivity(cellX, cellY, 2);
            // only do the second check if theres -/+ 2 difference
            if( oldCell != NULL && ((abs((int)cellX - (int)oldCell->GetPositionX()) > 2) || (abs((int)cellY - (int)oldCell->GetPositionY()) > 2)))
                UpdateCellActivity( oldCell->GetPositionX(), oldCell->GetPositionY(), 2 );
        }
    }
}

void MapMgr::UpdateObjectVisibility(Player *plObj, WorldObject *curObj)
{
    ASSERT(plObj && curObj);

    WorldObject::InRangeWorldObjectSet::iterator itr;
    bool cansee = plObj->CanSee(curObj), isvisible = plObj->GetVisibility(curObj, &itr);
    if(!cansee && isvisible)
    {
        plObj->PushOutOfRange(curObj->GetGUID());
        plObj->RemoveVisibleObject(itr);
    }
    else if(cansee && !isvisible)
    {
        plObj->AddVisibleObject(curObj);
        if(int count = curObj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj))
        {
            plObj->PushUpdateBlock(&m_createBuffer, count);
            m_createBuffer.clear();
        }
    }
}

void MapMgr::UpdateInRangeSet( WorldObject* obj, Player* plObj, MapCell* cell )
{
    if( cell == NULL )
        return;

    WorldObject* curObj;
    Player* plObj2;
    int count;
    ObjectSet::iterator iter = cell->Begin(), itr;
    bool cansee, isvisible;

    while( iter != cell->End() )
    {
        curObj = *iter;
        ++iter;

        if( curObj == NULL || curObj == obj )
            continue;

        // Add if we are not ourself and range == 0 or distance is withing range.
        if (IsInRange(m_UpdateDistance, obj, curObj))
        {
            if( !obj->IsInRangeMap( curObj ) )
            {
                // WorldObject in range, add to set
                obj->AddInRangeObject( curObj );
                curObj->AddInRangeObject( obj );

                if( curObj->IsPlayer() )
                {
                    plObj2 = castPtr<Player>( curObj );

                    if( plObj2->CanSee( obj ) && !plObj2->IsVisible( obj ) )
                    {
                        count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2);
                        plObj2->PushUpdateBlock(&m_createBuffer, count);
                        plObj2->AddVisibleObject(obj);
                        m_createBuffer.clear();
                    }
                }

                if( plObj != NULL )
                {
                    if( plObj->CanSee( curObj ) && !plObj->IsVisible( curObj ) )
                    {
                        count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj );
                        plObj->PushUpdateBlock( &m_createBuffer, count );
                        plObj->AddVisibleObject( curObj );
                        m_createBuffer.clear();
                    }
                }
            }
            else
            {
                // Check visiblility
                if( curObj->IsPlayer() )
                {
                    plObj2 = castPtr<Player>( curObj );
                    cansee = plObj2->CanSee(obj);
                    isvisible = plObj2->GetVisibility(obj, &itr);
                    if(!cansee && isvisible)
                    {
                        plObj2->PushOutOfRange(obj->GetGUID());
                        plObj2->RemoveVisibleObject(itr);
                    }
                    else if(cansee && !isvisible)
                    {
                        count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2);
                        plObj2->PushUpdateBlock(&m_createBuffer, count);
                        plObj2->AddVisibleObject(obj);
                        m_createBuffer.clear();
                    }
                }

                if( plObj )
                {
                    cansee = plObj->CanSee( curObj );
                    isvisible = plObj->GetVisibility( curObj, &itr );
                    if(!cansee && isvisible)
                    {
                        plObj->PushOutOfRange( curObj->GetGUID() );
                        plObj->RemoveVisibleObject( itr );
                    }
                    else if(cansee && !isvisible)
                    {
                        count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj );
                        plObj->PushUpdateBlock( &m_createBuffer, count );
                        plObj->AddVisibleObject( curObj );
                        m_createBuffer.clear();
                    }
                }
            }
        }
    }
}

void MapMgr::UpdateInRangeSet(uint64 guid, MapCell* cell )
{
    if( cell == NULL )
        return;

    int count;
    WorldObject* obj = NULL;
    Player* plObj = NULL;
    WorldObject* curObj = NULL;
    Player* plObj2 = NULL;
    bool cansee, isvisible;
    ObjectSet::iterator itr;
    ObjectSet::iterator iter = cell->Begin();
    obj = _GetObject(guid);
    if(obj == NULL)
        return;
    if(obj->IsPlayer())
        plObj = castPtr<Player>(obj);

    while( iter != cell->End() )
    {
        curObj = *iter;
        ++iter;

        if( curObj == NULL )
            continue;

        // Add if we are not ourself and range == 0 or distance is withing range.
        if ( curObj != obj && IsInRange(m_UpdateDistance, obj, curObj))
        {
            if( !obj->IsInRangeMap( curObj ) )
            {
                // WorldObject in range, add to set
                obj->AddInRangeObject( curObj );
                curObj->AddInRangeObject( obj );

                if( curObj->IsPlayer() )
                {
                    plObj2 = castPtr<Player>( curObj );

                    if( plObj2->CanSee( obj ) && !plObj2->IsVisible( obj ) )
                    {
                        if(count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2))
                        {
                            plObj2->PushUpdateBlock(&m_createBuffer, count);
                            plObj2->AddVisibleObject(obj);
                            m_createBuffer.clear();
                        }
                    }
                }

                if( plObj != NULL )
                {
                    if( plObj->CanSee( curObj ) && !plObj->IsVisible( curObj ) )
                    {
                        if(count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj ))
                        {
                            plObj->PushUpdateBlock( &m_createBuffer, count );
                            plObj->AddVisibleObject( curObj );
                            m_createBuffer.clear();
                        }
                    }
                }
            }
            else
            {
                // Check visiblility
                if( curObj->IsPlayer() )
                {
                    plObj2 = castPtr<Player>( curObj );
                    cansee = plObj2->CanSee(obj);
                    isvisible = plObj2->GetVisibility(obj, &itr);
                    if(!cansee && isvisible)
                    {
                        plObj2->PushOutOfRange(obj->GetGUID());
                        plObj2->RemoveVisibleObject(itr);
                    }
                    else if(cansee && !isvisible)
                    {
                        if(count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2))
                        {
                            plObj2->PushUpdateBlock(&m_createBuffer, count);
                            plObj2->AddVisibleObject(obj);
                            m_createBuffer.clear();
                        }
                    }
                }

                if( plObj )
                {
                    cansee = plObj->CanSee( curObj );
                    isvisible = plObj->GetVisibility( curObj, &itr );
                    if(!cansee && isvisible)
                    {
                        plObj->PushOutOfRange( curObj->GetGUID() );
                        plObj->RemoveVisibleObject( itr );
                    }
                    else if(cansee && !isvisible)
                    {
                        if(count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj ))
                        {
                            plObj->PushUpdateBlock( &m_createBuffer, count );
                            plObj->AddVisibleObject( curObj );
                            m_createBuffer.clear();
                        }
                    }
                }
            }
        }
    }
}

void MapMgr::_UpdateObjects()
{
    m_updateMutex.Acquire();
    if(!_updates.size() && !_processQueue.size())
    {
        m_updateMutex.Release();
        return;
    }

    uint32 count = 0;
    WorldObject *wObj;
    ObjectSet::iterator iter = _updates.begin();
    for(; iter != _updates.end();)
    {
        wObj = *iter;
        ++iter;
        if(wObj == NULL)
            continue;

        if( wObj->IsInWorld() )
        {
            // players have to receive their own updates ;)
            if( wObj->IsPlayer() )
            {
                // need to be different! ;)
                if( count = wObj->BuildValuesUpdateBlockForPlayer(&m_updateBuffer, UF_FLAGMASK_SELF) )
                {
                    castPtr<Player>( wObj )->PushUpdateBlock( &m_updateBuffer, count );
                    m_updateBuffer.clear();
                }
            }

            if( wObj->IsUnit() && wObj->HasUpdateField( UNIT_FIELD_HEALTH ) )
                castPtr<Unit>( wObj )->EventHealthChangeSinceLastUpdate();

            PlayerSet m_partyTargets, m_petTargets;
            // build the update
            if(count = wObj->BuildValuesUpdateBlockForPlayer(&m_updateBuffer, UF_FLAGMASK_PUBLIC))
            {
                for(WorldObject::InRangeSet::iterator itr = wObj->GetInRangePlayerSetBegin(); itr != wObj->GetInRangePlayerSetEnd();)
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
                    else plrTarget->PushUpdateBlock( &m_updateBuffer, count );
                }
                m_updateBuffer.clear();
            }

            if(m_partyTargets.size() && (count = wObj->BuildValuesUpdateBlockForPlayer(&m_updateBuffer, UF_FLAGMASK_PARTY_MEMBER)))
            {
                for(PlayerSet::iterator itr = m_partyTargets.begin(); itr != m_partyTargets.end(); itr++)
                    (*itr)->PushUpdateBlock( &m_updateBuffer, count );
                m_updateBuffer.clear();
                m_partyTargets.clear();
            }
            if(m_petTargets.size() && (count = wObj->BuildValuesUpdateBlockForPlayer(&m_updateBuffer, UF_FLAGMASK_OWN_PET)))
            {
                for(PlayerSet::iterator itr = m_petTargets.begin(); itr != m_petTargets.end(); itr++)
                    (*itr)->PushUpdateBlock( &m_updateBuffer, count );
                m_updateBuffer.clear();
                m_petTargets.clear();
            }
        }
        wObj->ClearUpdateMask();
    }
    _updates.clear();

    // generate pending a9packets and send to clients.
    Player* plyr;
    for(PlayerSet::iterator itr = _processQueue.begin(); itr != _processQueue.end();)
    {
        plyr = *itr;
        itr = _processQueue.erase(itr);
        if(plyr->GetMapMgr() == this)
            plyr->PopPendingUpdates();
    }
    m_updateMutex.Release();
}

void MapMgr::UpdateAllCells(bool apply, uint32 areamask)
{
    uint16 AreaID = 0, loadCount = 0;
    MapCell * cellInfo;
    CellSpawns * spawns;
    uint32 StartX = 0, EndX = 0, StartY = 0, EndY = 0;
    GetBaseMap()->GetCellLimits(StartX, EndX, StartY, EndY);
    if(!areamask) sLog.Info("MapMgr", "Updating all cells for map %03u, server might lag.", _mapId);
    for( uint32 x = StartX ; x < EndX ; x ++ )
    {
        for( uint32 y = StartY ; y < EndY ; y ++ )
        {
            if(!SetThreadState(THREADSTATE_AWAITING))
                break;

            if(areamask)
            {
                if(!GetBaseMap()->CellHasAreaID(x, y, AreaID))
                    continue;

                AreaTableEntry* at = dbcAreaTable.LookupEntry( AreaID );
                if(at == NULL)
                    continue;
                if(at->ZoneId != areamask)
                    if(at->AreaId != areamask)
                        continue;
                AreaID = 0;
            }

            cellInfo = GetCell( x , y );
            if(apply)
            {
                if( !cellInfo )
                {   // Cell doesn't exist, create it.
                    cellInfo = Create( x , y );
                    cellInfo->Init( x , y , _mapId , this );
                    cellInfo->SetActivity(true);
                    _map->CellGoneActive(x, y);
                    sLog.Debug("MapMgr","Created cell [%u,%u] on map %u (instance %u)." , x , y , _mapId , m_instanceID );
                }

                if (!cellInfo->IsLoaded())
                {
                    if( spawns = _map->GetSpawnsList( x , y ) )
                    {
                        cellInfo->LoadObjects( spawns );
                        loadCount++;
                    }
                }
                AddForcedCell(cellInfo, 0);
            } else if(cellInfo != NULL)
                RemoveForcedCell(cellInfo);
        }
    }

    if(!areamask) sLog.Success("MapMgr", "Cell update for map %03u completed with %u objLoad calls", _mapId, loadCount);
}

void MapMgr::UpdateCellActivity(uint32 x, uint32 y, int radius)
{
    CellSpawns * sp;
    uint32 endX = (x + radius) <= _sizeX ? x + radius : (_sizeX-1);
    uint32 endY = (y + radius) <= _sizeY ? y + radius : (_sizeY-1);
    uint32 startX = x - radius > 0 ? x - radius : 0;
    uint32 startY = y - radius > 0 ? y - radius : 0;
    uint32 posX, posY;

    MapCell *objCell;

    for (posX = startX; posX <= endX; posX++ )
    {
        for (posY = startY; posY <= endY; posY++ )
        {
            if( posX >= _sizeX ||  posY >= _sizeY )
                continue;

            objCell = GetCell(posX, posY);
            if (!objCell)
            {
                if (_CellActive(posX, posY))
                {
                    objCell = Create(posX, posY);
                    objCell->Init(posX, posY, _mapId, this);
                    objCell->SetActivity(true);
                    _map->CellGoneActive(posX, posY);
                    ASSERT(!objCell->IsLoaded());
                    sp = _map->GetSpawnsList(posX, posY);
                    if(sp) objCell->LoadObjects(sp);
                }
            }
            else
            {
                //Cell is now active
                if (_CellActive(posX, posY) && !objCell->IsActive())
                {
                    _map->CellGoneActive(posX, posY);
                    objCell->SetActivity(true);

                    if (!objCell->IsLoaded())
                    {
                        sp = _map->GetSpawnsList(posX, posY);
                        if(sp) objCell->LoadObjects(sp);
                    }
                }
                else if (!_CellActive(posX, posY) && objCell->IsActive()) // Cell is no longer active
                {
                    _map->CellGoneIdle(posX, posY);
                    objCell->SetActivity(false);
                }
            }
        }
    }
}

void MapMgr::GetWaterData(float x, float y, float z, float &outHeight, uint16 &outType)
{
    uint16 vwaterType = 0, mapWaterType = GetBaseMap()->GetWaterType(x, y);
    float mapWaterheight = GetBaseMap()->GetWaterHeight(x, y, z);
    float vmapWaterHeight = sVMapInterface.GetWaterHeight(GetMapId(), x, y, z, vwaterType);
    if(!(mapWaterType & 0x10) && vwaterType && vmapWaterHeight != NO_WMO_HEIGHT)
    { outHeight = vmapWaterHeight; outType = vwaterType; }
    else { outHeight = mapWaterheight; outType = mapWaterType; }
}

float MapMgr::GetLandHeight(float x, float y)
{
    return GetBaseMap()->GetLandHeight(x, y);
}

uint8 MapMgr::GetWalkableState(float x, float y)
{
    return GetBaseMap()->GetWalkableState(x, y);
}

uint16 MapMgr::GetAreaID(float x, float y, float z)
{
    uint16 areaId = GetBaseMap()->GetAreaID(x, y, z), wmoAID = 0;
    uint32 wmoFlags = 0; int32 adtId = 0, rootId = 0, groupId = 0;
    sVMapInterface.GetAreaInfo(GetMapId(), x, y, z, wmoAID, wmoFlags, adtId, rootId, groupId);
    return wmoAID ? wmoAID : areaId;
}

void MapMgr::AddForcedCell(MapCell * c, uint32 range)
{
    c->SetPermanentActivity(true);
    UpdateCellActivity(c->GetPositionX(), c->GetPositionY(), range);
}

void MapMgr::RemoveForcedCell(MapCell * c, uint32 range)
{
    c->SetPermanentActivity(false);
    UpdateCellActivity(c->GetPositionX(), c->GetPositionY(), range);
}

bool MapMgr::_CellActive(uint32 x, uint32 y)
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

void MapMgr::ObjectUpdated(WorldObject* obj)
{
    // set our fields to dirty
    // stupid fucked up code in places.. i hate doing this but i've got to :<
    // - burlex
    m_updateMutex.Acquire();
    _updates.insert(obj);
    m_updateMutex.Release();
}

void MapMgr::PushToProcessed(Player* plr)
{
    _processQueue.insert(plr);
}

void MapMgr::ChangeFarsightLocation(Player* plr, Unit* farsight, bool apply)
{

}

void MapMgr::ChangeFarsightLocation(Player* plr, float X, float Y, bool apply)
{

}

/* new stuff
*/

bool MapMgr::run()
{
    return Do();
}

bool MapMgr::Do()
{
#ifdef WIN32
    threadid=GetCurrentThreadId();
#endif
    thread_running = true;
    ObjectSet::iterator i;
    uint32 last_exec=getMSTime();

    /* load corpses */
    objmgr.LoadCorpses(this);

    // initialize worldstates
    sWorldStateTemplateManager.ApplyMapTemplate(this);

    // Call script OnLoad virtual procedure
    TRIGGER_INSTANCE_EVENT( this, OnLoad );

    if( GetMapInfo()->type == INSTANCE_NULL )
    {
        if(sWorld.ServerPreloading == 2)
        {
#ifdef _WIN64
            UpdateAllCells(true);
#else
            sLog.Error("MapMgr", "Server cell preloading defined but version is %s, preloading is not supported.", ARCH);
#endif
        }
    }

    // always declare local variables outside of the loop!
    // otherwise theres a lot of sub esp; going on.

    uint32 exec_time, exec_start;
    while(true)
    {
        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        exec_start = getMSTime();
        //first push to world new objects
        m_objectinsertlock.Acquire();
        if(m_objectinsertpool.size())
        {
            for(i = m_objectinsertpool.begin(); i != m_objectinsertpool.end(); i++)
                (*i)->PushToWorld(this);

            m_objectinsertpool.clear();
        }
        m_objectinsertlock.Release();
        if(!SetThreadState(THREADSTATE_AWAITING))
            break;

        //Now update sessions of this map + objects
        _PerformObjectDuties();
        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;

        last_exec=getMSTime();
        exec_time=last_exec-exec_start;
        if(exec_time<MAP_MGR_UPDATE_PERIOD)
            Delay(MAP_MGR_UPDATE_PERIOD-exec_time);

        //////////////////////////////////////////////////////////////////////////
        // Check if we have to die :P
        //////////////////////////////////////////////////////////////////////////
        if(InactiveMoveTime && UNIXTIME >= InactiveMoveTime)
            break;
    }

    // Clear the instance's reference to us.
    if(m_battleground)
    {
        BattlegroundManager.DeleteBattleground(m_battleground);
        sInstanceMgr.DeleteBattlegroundInstance( GetMapId(), GetInstanceID() );
    }

    if(pInstance)
    {
        pInstance->m_mapMgr = NULL;
        // check for a non-raid instance, these expire after 10 minutes.
        if(GetMapInfo()->type == INSTANCE_NONRAID || pInstance->m_isBattleground)
            sInstanceMgr._DeleteInstance(pInstance, true);
    } else if(GetMapInfo()->type == INSTANCE_NULL)
        sInstanceMgr.m_singleMaps[GetMapId()] = NULL;

    // Teleport any left-over players out.
    TeleportPlayers();

    thread_running = false;
    if(thread_kill_only)
        return false;

    Destruct();

    // Sign us up for deletion
    return true;
}

void MapMgr::BeginInstanceExpireCountdown()
{
    WorldPacket data(SMSG_RAID_GROUP_ONLY, 8);

    // so players getting removed don't overwrite us
    forced_expire = true;

    // send our sexy packet
    data << uint32(60000) << uint32(1);

    // Update all players on map.
    __player_iterator = m_PlayerStorage.begin();
    Player* ptr;
    for(; __player_iterator != m_PlayerStorage.end();)
    {
        ptr = __player_iterator->second;;
        ++__player_iterator;

        if(ptr->GetSession())
        {
            if(!ptr->raidgrouponlysent)
                ptr->GetSession()->SendPacket(&data);
        }
    }

    // set our expire time to 60 seconds.
    InactiveMoveTime = UNIXTIME + 60;
}

void MapMgr::AddObject(WorldObject* obj)
{
    m_objectinsertlock.Acquire();
    m_objectinsertpool.insert(obj);
    m_objectinsertlock.Release();
}

Unit* MapMgr::GetUnit(WoWGuid guid)
{
    switch(guid.getHigh())
    {
    case HIGHGUID_TYPE_UNIT: return GetCreature(guid);
    case HIGHGUID_TYPE_PLAYER: return GetPlayer(guid);
    case HIGHGUID_TYPE_PET: return GetPet(guid);
    case HIGHGUID_TYPE_VEHICLE: return GetVehicle(guid);
    }

    return NULL;
}

WorldObject* MapMgr::_GetObject(WoWGuid guid)
{
    switch(guid.getHigh())
    {
    case HIGHGUID_TYPE_VEHICLE: return GetVehicle(guid);
    case HIGHGUID_TYPE_GAMEOBJECT: return GetGameObject(guid);
    case HIGHGUID_TYPE_UNIT: return GetCreature(guid);
    case HIGHGUID_TYPE_DYNAMICOBJECT: return GetDynamicObject(guid);
    case HIGHGUID_TYPE_TRANSPORTER: return objmgr.GetTransporter(guid.getLow());
    case HIGHGUID_TYPE_CORPSE: return objmgr.GetCorpse(guid.getLow());
    }
    return GetUnit(guid);
}

WorldObject* MapMgr::GetObjectClosestToCoords(uint32 entry, float x, float y, float z, float ClosestDist, int32 forcedtype)
{
    MapCell * pCell = GetCell(GetPosX(x), GetPosY(y));
    if(pCell == NULL)
        return NULL;

    WorldObject* ClosestObject = NULL;
    float CurrentDist = 0;
    ObjectSet::const_iterator iter;
    for(iter = pCell->Begin(); iter != pCell->End(); iter++)
    {
        CurrentDist = (*iter)->CalcDistance(x, y, (z != 0.0f ? z : (*iter)->GetPositionZ()));
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
    }

    return ClosestObject;
}

bool MapMgr::CanUseCollision(WorldObject* obj)
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

void MapMgr::_PerformObjectDuties()
{
    uint32 mstime = getMSTime(); // Get our ms time
    ++mLoopCounter; // Inc loop counter
    // Calculate loop diff
    uint32 diff = std::min((uint32)500, mstime - lastDutyUpdate);
    // Update duty timer
    lastDutyUpdate = mstime;

    // Update any events.
    eventHolder.Update(diff);

    // Update our collision system via singular map system
    sVMapInterface.UpdateSingleMap(_mapId, m_instanceID, diff);

    // Update players.
    Player* ptr4;
    __player_iterator = m_PlayerStorage.begin();
    for(; __player_iterator != m_PlayerStorage.end();)
    {
        ptr4 = __player_iterator->second;
        ++__player_iterator;

        ptr4->Update( diff );
    }

    if(!SetThreadState(THREADSTATE_AWAITING))
        return;

    // Update our objects every 2 ticks(planned 200ms)
    if( (mLoopCounter % 2) == 0 )
    {
        diff = mstime - lastUnitUpdate;
        ActiveLock.Acquire();
        if(activeCreatures.size())
        {
            Creature* ptr;
            __creature_iterator = activeCreatures.begin();
            for(; __creature_iterator != activeCreatures.end();)
            {
                ptr = *__creature_iterator;
                ++__creature_iterator;

                ptr->Update(diff);
            }
        }

        if(!SetThreadState(THREADSTATE_AWAITING))
            return;

        if(activeVehicles.size())
        {
            Vehicle* ptr3;
            __vehicle_iterator = activeVehicles.begin();
            for(; __vehicle_iterator != activeVehicles.end();)
            {
                ptr3 = *__vehicle_iterator;
                ++__vehicle_iterator;

                ptr3->Update(diff);
            }
        }

        ActiveLock.Release();
        lastUnitUpdate = mstime;
    }

    if(!SetThreadState(THREADSTATE_AWAITING))
        return;

    // Update gameobjects every 4 ticks(planned 400ms)
    if( (mLoopCounter % 4) == 0 )
    {
        diff = mstime - lastGameobjectUpdate;
        ActiveLock.Acquire();

        GameObject* ptr5;
        __gameobject_iterator = activeGameObjects.begin();
        for(; __gameobject_iterator != activeGameObjects.end(); )
        {
            ptr5 = *__gameobject_iterator;
            ++__gameobject_iterator;

            ptr5->Update( diff );
        }
        ActiveLock.Release();

        DynamicObject* ptr6;
        DynamicObjectStorageMap::iterator itr = m_DynamicObjectStorage.begin();
        for(; itr != m_DynamicObjectStorage.end(); )
        {
            ptr6 = itr->second;
            ++itr;

            ptr6->UpdateTargets( diff );
        }
        lastGameobjectUpdate = mstime;
    }

    if(!SetThreadState(THREADSTATE_AWAITING))
        return;

    // Sessions are updated every loop.
    {
        int result = 0;
        WorldSession * MapSession;
        SessionSet::iterator itr = MapSessions.begin(), it2;
        for(; itr != MapSessions.end();)
        {
            MapSession = (*itr);
            it2 = itr++;

            //we have teleported to another map, remove us here.
            if(MapSession->GetInstance() != m_instanceID)
            {
                MapSessions.erase(it2);
                continue;
            }

            // Don't update players not on our map.
            // If we abort in the handler, it means we will "lose" packets, or not process this.
            // .. and that could be diasterous to our client :P
            if( MapSession->GetPlayer()->GetMapMgr() == NULL ||
                MapSession->GetPlayer()->GetMapMgr() != this)
                continue;

            result = MapSession->Update(m_instanceID);
            if(result)//session or socket deleted?
            {
                if(result == 1)//socket don't exist anymore, delete from both world- and map-sessions.
                    sWorld.DeleteGlobalSession(MapSession);
                MapSessions.erase(it2);
            }
        }
    }

    if(!SetThreadState(THREADSTATE_AWAITING))
        return;

    // Finally, A9 Building/Distribution
    _UpdateObjects();
}

void MapMgr::EventCorpseDespawn(uint64 guid)
{
    objmgr.DespawnCorpse(guid);
}

void MapMgr::TeleportPlayers()
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

            if(ptr->GetSession())
                ptr->EjectFromInstance();
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
                ptr->GetSession()->LogoutPlayer(false);
            else
            {
                ptr->Destruct();
                ptr = NULL;
                m_PlayerStorage.erase(__player_iterator);
            }
        }
    }
}

void MapMgr::UnloadCell(uint32 x, uint32 y)
{
    MapCell * c = GetCell(x,y);
    if(c == NULL || c->HasPlayers() || _CellActive(x,y) || !c->IsUnloadPending())
        return;

    sLog.Debug("MapMgr","Unloading Cell [%d][%d] on map %d (instance %d)...", x, y, _mapId, m_instanceID);
    c->Unload();
}

void MapMgr::EventRespawnVehicle(Vehicle* v, MapCell * p)
{
    ObjectSet::iterator itr = p->_respawnObjects.find( v );
    if(itr != p->_respawnObjects.end())
    {
        v->m_respawnCell=NULL;
        p->_respawnObjects.erase(itr);
        v->OnRespawn(this);
    }
}

void MapMgr::EventRespawnCreature(Creature* c, MapCell * p)
{
    ObjectSet::iterator itr = p->_respawnObjects.find( c );
    if(itr != p->_respawnObjects.end())
    {
        c->m_respawnCell=NULL;
        p->_respawnObjects.erase(itr);
        c->OnRespawn(this);
        if(c->GetAIInterface())
            c->GetAIInterface()->OnRespawn(c);
    }
}

void MapMgr::EventRespawnGameObject(GameObject* o, MapCell * c)
{
    ObjectSet::iterator itr = c->_respawnObjects.find( o);
    if(itr != c->_respawnObjects.end())
    {
        o->m_respawnCell=NULL;
        c->_respawnObjects.erase(itr);
        o->Spawn(this);
    }
}

bool MapMgr::IsInRange(float fRange, WorldObject* obj, WorldObject* currentobj)
{
    // First distance check, are we in range?
    if(currentobj->GetDistance2dSq( obj ) > fRange )
        return false;
    return true;
}

void MapMgr::SendMessageToCellPlayers(WorldObject* obj, WorldPacket * packet, uint32 cell_radius /* = 2 */)
{
    uint32 cellX = GetPosX(obj->GetPositionX());
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
    }
}

void MapMgr::SendChatMessageToCellPlayers(WorldObject* obj, WorldPacket * packet, uint32 cell_radius, uint32 langpos, uint32 guidPos, int32 lang, WorldSession * originator)
{
    uint32 cellX = GetPosX(obj->GetPositionX());
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
                    if((*iter)->IsPlayer())
                    {
                        if(originator->GetPlayer()->PhasedCanInteract((*iter))) // Matching phases.
                            castPtr<Player>(*iter)->GetSession()->SendChatPacket(packet, langpos, guidPos, lang, originator);
                    }
                }
            }
        }
    }
}

Creature* MapMgr::GetSqlIdCreature(uint32 sqlid)
{
    CreatureSqlIdMap::iterator itr = _sqlids_creatures.find(sqlid);
    return (itr == _sqlids_creatures.end()) ? NULL : itr->second;
}

GameObject* MapMgr::GetSqlIdGameObject(uint32 sqlid)
{
    GameObjectSqlIdMap::iterator itr = _sqlids_gameobjects.find(sqlid);
    return (itr == _sqlids_gameobjects.end()) ? NULL : itr->second;
}

void MapMgr::HookOnAreaTrigger(Player* plr, uint32 id)
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

Vehicle* MapMgr::CreateVehicle(uint32 entry)
{
    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(entry);
    if(ctrData == NULL || ctrData->Vehicle_entry == 0)
    {
        sLog.Warning("MapMgr", "Skipping CreateCreature for entry %u due to incomplete database.", entry);
        return NULL;
    }

    uint32 low_guid = 0;
    if(_reusable_guids_vehicle.size())
    {
        low_guid = _reusable_guids_vehicle.front();
        _reusable_guids_vehicle.pop_front();
    } else low_guid = ++m_VehicleHighGuid;

    Vehicle *v = new Vehicle(ctrData, MAKE_NEW_GUID(low_guid, entry, HIGHGUID_TYPE_VEHICLE));
    v->Init();
    ASSERT( v->GetHighGUID() == HIGHGUID_TYPE_VEHICLE );
    return v;
}

Creature* MapMgr::CreateCreature(uint32 entry)
{
    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(entry);
    if(ctrData == NULL)
    {
        sLog.Warning("MapMgr", "Skipping CreateCreature for entry %u due to incomplete database.", entry);
        return NULL;
    }

    uint32 low_guid = 0;
    if(_reusable_guids_creature.size())
    {
        low_guid = _reusable_guids_creature.front();
        _reusable_guids_creature.pop_front();
    } else low_guid = ++m_CreatureHighGuid;

    Creature *cr = new Creature(ctrData, MAKE_NEW_GUID(low_guid, entry, HIGHGUID_TYPE_UNIT));
    cr->Init();
    ASSERT( cr->GetHighGUID() == HIGHGUID_TYPE_UNIT );
    return cr;
}

Summon* MapMgr::CreateSummon(uint32 entry)
{
    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(entry);
    if(ctrData == NULL)
    {
        sLog.Warning("MapMgr", "Skipping CreateSummon for entry %u due to incomplete database.", entry);
        return NULL;
    }

    uint32 low_guid = 0;
    if(_reusable_guids_creature.size())
    {
        low_guid = _reusable_guids_creature.front();
        _reusable_guids_creature.pop_front();
    } else low_guid = ++m_CreatureHighGuid;

    Summon *sum = new Summon(ctrData, MAKE_NEW_GUID(low_guid, entry, HIGHGUID_TYPE_UNIT));
    sum->Init();
    ASSERT( sum->GetHighGUID() == HIGHGUID_TYPE_UNIT );
    return sum;
}

GameObject* MapMgr::CreateGameObject(uint32 entry)
{
    // Validate the entry
    GameObjectInfo *goi = GameObjectNameStorage.LookupEntry( entry );
    if( goi == NULL )
    {
        sLog.Warning("MapMgr", "Skipping CreateGameObject for entry %u due to incomplete database.", entry);
        return NULL;
    }

    GameObject *go = new GameObject(MAKE_NEW_GUID(++m_GOHighGuid, entry, HIGHGUID_TYPE_GAMEOBJECT));
    go->Init();
    ASSERT( go->GetHighGUID() == HIGHGUID_TYPE_GAMEOBJECT );
    return go;
}

DynamicObject* MapMgr::CreateDynamicObject()
{
    DynamicObject* dyn = new DynamicObject(HIGHGUID_TYPE_DYNAMICOBJECT, (++m_DynamicObjectHighGuid));
    dyn->Init();
    ASSERT( dyn->GetHighGUID() == HIGHGUID_TYPE_DYNAMICOBJECT );
    return dyn;
}

void MapMgr::SendPacketToPlayers(int32 iZoneMask, int32 iFactionMask, WorldPacket *pData)
{
    // Update all players on map.
    Player * ptr = NULL;
    PlayerStorageMap::iterator itr1 = m_PlayerStorage.begin();
    for(itr1 = m_PlayerStorage.begin(); itr1 != m_PlayerStorage.end();)
    {
        ptr = itr1->second;
        itr1++;
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

void MapMgr::RemoveAuraFromPlayers(int32 iFactionMask, uint32 uAuraId)
{
    // Update all players on map.
    Player* ptr;
    for(__player_iterator = m_PlayerStorage.begin(); __player_iterator != m_PlayerStorage.end();__player_iterator++)
    {
        ptr = __player_iterator->second;;

        if(ptr->GetSession())
        {
            if( iFactionMask != FACTION_MASK_ALL && ptr->GetTeam() != (uint32)iFactionMask )
                continue;

            ptr->RemoveAura(uAuraId);
        }
    }
}

void MapMgr::RemovePositiveAuraFromPlayers(int32 iFactionMask, uint32 uAuraId)
{
    // Update all players on map.
    Player* ptr;
    for(__player_iterator = m_PlayerStorage.begin(); __player_iterator != m_PlayerStorage.end();__player_iterator++)
    {
        ptr = __player_iterator->second;;

        if(ptr->GetSession())
        {
            if( iFactionMask != FACTION_MASK_ALL && ptr->GetTeam() != (uint32)iFactionMask )
                continue;
            ptr->m_AuraInterface.RemovePositiveAura(uAuraId);
        }
    }
}

void MapMgr::CastSpellOnPlayers(int32 iFactionMask, uint32 uSpellId)
{
    SpellEntry * sp = dbcSpell.LookupEntry(uSpellId);
    if( !sp )
        return;

    // Update all players on map.
    Player* ptr;
    for(__player_iterator = m_PlayerStorage.begin(); __player_iterator != m_PlayerStorage.end();__player_iterator++)
    {
        ptr = __player_iterator->second;;

        if(ptr->GetSession())
        {
            if( iFactionMask != FACTION_MASK_ALL && ptr->GetTeam() != (uint32)iFactionMask )
                continue;

            if(sp != NULL)
                sEventMgr.AddEvent(castPtr<Unit>(ptr), &Unit::EventCastSpell, castPtr<Unit>(__player_iterator->second), sp, EVENT_AURA_APPLY, 250, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
    }
}

void MapMgr::SendPvPCaptureMessage(int32 iZoneMask, uint32 ZoneId, const char * Format, ...)
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

    // Update all players on map.
    Player* ptr;
    for(__player_iterator = m_PlayerStorage.begin(); __player_iterator != m_PlayerStorage.end();__player_iterator++)
    {
        ptr = __player_iterator->second;;

        if(ptr->GetSession())
        {
            if( ( iZoneMask != ZONE_MASK_ALL && ptr->GetZoneId() != (uint32)iZoneMask) )
                continue;

            ptr->GetSession()->SendPacket(&data);
        }
    }
}
