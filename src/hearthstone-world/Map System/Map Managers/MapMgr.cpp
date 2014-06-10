/***
 * Demonstrike Core
 */

//
// mapMgr->cpp
//

#include "StdAfx.h"

#define MAP_MGR_UPDATE_PERIOD 100
#define MAPMGR_INACTIVE_MOVE_TIME 10
extern bool bServerShutdown;

MapMgr::MapMgr(Map *map, uint32 mapId, uint32 instanceid) : ThreadContext(), CellHandler<MapCell>(map), _mapId(mapId), eventHolder(instanceid)
{
    m_instanceID = instanceid;
    pMapInfo = WorldMapInfoStorage.LookupEntry(mapId);
    pdbcMap = dbcMap.LookupEntry(mapId);
    m_UpdateDistance = MAX_VIEW_DISTANCE;
    iInstanceMode = 0;

    // Set up storage arrays
    m_CreatureArraySize = map->CreatureSpawnCount;
    m_VehicleArraySize = 0;

    m_GOHighGuid = 0;
    m_CreatureHighGuid = 0;
    m_VehicleHighGuid = 0;
    m_DynamicObjectHighGuid=0;
    lastUnitUpdate = getMSTime();
    lastDynamicUpdate = getMSTime();
    lastGameobjectUpdate = getMSTime();
    m_battleground = NULLBATTLEGROUND;

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
    _mapWideStaticObjects.clear();
    _updates.clear();
    _processQueue.clear();
    MapSessions.clear();

    ActiveLock.Acquire();
    activeGameObjects.clear();
    activeCreatures.clear();
    activeVehicles.clear();
    ActiveLock.Release();

    m_corpses.clear();
    _sqlids_vehicles.clear();
    _sqlids_creatures.clear();
    _sqlids_gameobjects.clear();
    _reusable_guids_creature.clear();
    _reusable_guids_vehicle.clear();
}

void MapMgr::Init(bool Instance)
{
    m_stateManager = new WorldStateManager(this);

    // Create our script
    _script = sScriptMgr.CreateMapManagerScriptForMapManager(_mapId, this, Instance);

    sHookInterface.OnContinentCreate(this);
}

void MapMgr::Destruct()
{
    sEventMgr.RemoveEvents(this);
    sEventMgr.RemoveEventHolder(m_instanceID);

    _script->Destruct();
    _script = NULL;

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

    Object* pObject;
    for(set<Object* >::iterator itr = _mapWideStaticObjects.begin(); itr != _mapWideStaticObjects.end(); itr++)
    {
        pObject = *itr;
        if(!pObject)
            continue;

        if(pObject->IsInWorld())
            pObject->RemoveFromWorld(false);

        switch(pObject->GetTypeFromGUID())
        {
        case HIGHGUID_TYPE_VEHICLE:
            TO_VEHICLE(pObject)->Destruct();
            break;
        case HIGHGUID_TYPE_CREATURE:
            TO_CREATURE(pObject)->Destruct();
            break;
        case HIGHGUID_TYPE_GAMEOBJECT:
            TO_GAMEOBJECT(pObject)->Destruct();
            break;
        default:
            pObject->Destruct();
        }
        pObject = NULL;
    }
    _mapWideStaticObjects.clear();

    Corpse* pCorpse;
    if(m_corpses.size())
    {
        for(unordered_set<Corpse* >::iterator itr = m_corpses.begin(); itr != m_corpses.end();)
        {
            pCorpse = *itr;
            ++itr;

            if(pCorpse->IsInWorld())
                pCorpse->RemoveFromWorld(false);

            pCorpse->Destruct();
            pCorpse = NULLCORPSE;
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

    _sqlids_vehicles.clear();
    _sqlids_creatures.clear();
    _sqlids_gameobjects.clear();
    _reusable_guids_vehicle.clear();
    _reusable_guids_creature.clear();

    m_battleground = NULLBATTLEGROUND;

    sLog.Notice("MapMgr", "Instance %u shut down. (%s)" , m_instanceID, GetBaseMap()->GetName());
}

MapMgr::~MapMgr()
{

}

void MapMgr::EventPushObjectToSelf(Object *obj)
{
    obj->PushToWorld(this);
}

void MapMgr::PushObject(Object* obj)
{
    /////////////
    // Assertions
    /////////////
    ASSERT(obj);

    // That object types are not map objects. TODO: add AI groups here?
    if(obj->GetTypeId() == TYPEID_ITEM || obj->GetTypeId() == TYPEID_CONTAINER)
        return;

    if(obj->GetTypeId() == TYPEID_CORPSE)
        m_corpses.insert( TO_CORPSE(obj) );

    Player* plObj = NULLPLR;
    if(obj->IsPlayer())
    {
        plObj = TO_PLAYER( obj );
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
    }
    else obj->ClearInRangeSet();

    ///////////////////////
    // Get cell coordinates
    ///////////////////////

    ASSERT(obj->GetMapId() == _mapId);

    ASSERT(obj->GetPositionY() < _maxY && obj->GetPositionY() > _minY);
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
            obj->GetPositionV()->ChangeCoords(0,0,0,0);
        }

        cx = GetPosX(obj->GetPositionX());
        cy = GetPosY(obj->GetPositionY());
    }

    MapCell *objCell = GetCell(cx,cy);
    if (!objCell) // Should never fail to create but...
        if((objCell = Create(cx,cy)) != NULL)
            objCell->Init(cx, cy, _mapId, this);
    ASSERT(objCell);

    uint32 count;
    uint32 endX = (cx <= _sizeX) ? cx + 1 : (_sizeX-1);
    uint32 endY = (cy <= _sizeY) ? cy + 1 : (_sizeY-1);
    uint32 startX = cx > 0 ? cx - 1 : 0;
    uint32 startY = cy > 0 ? cy - 1 : 0;

    if(plObj)
    {
        sLog.Debug("MapMgr","Creating player "I64FMT" for himself.", obj->GetGUID());
        count = plObj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj);
        plObj->PushUpdateData(&m_createBuffer, count);
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
        switch(obj->GetTypeFromGUID())
        {
        case HIGHGUID_TYPE_PET:
            m_PetStorage[obj->GetUIdFromGUID()] = TO_PET( obj );
            break;

        case HIGHGUID_TYPE_CREATURE:
            {
                ASSERT((obj->GetUIdFromGUID()) <= m_CreatureHighGuid);
                m_CreatureStorage[obj->GetUIdFromGUID()] = TO_CREATURE(obj);
                if(TO_CREATURE(obj)->m_spawn != NULL)
                {
                    _sqlids_creatures.insert(make_pair( TO_CREATURE(obj)-> GetSQL_id(), TO_CREATURE(obj) ) );
                }
                CALL_INSTANCE_SCRIPT_EVENT( this, OnCreaturePushToWorld )( TO_CREATURE(obj) );
            }break;

        case HIGHGUID_TYPE_VEHICLE:
            {
                ASSERT((obj->GetUIdFromGUID()) <= m_VehicleHighGuid);
                m_VehicleStorage[obj->GetUIdFromGUID()] = TO_VEHICLE(obj);
                if(TO_VEHICLE(obj)->m_spawn != NULL)
                {
                    _sqlids_vehicles.insert(make_pair( TO_VEHICLE(obj)->m_spawn->id, TO_VEHICLE(obj) ) );
                }
                CALL_INSTANCE_SCRIPT_EVENT( this, OnCreaturePushToWorld )( TO_CREATURE(obj) );
            }break;
        case HIGHGUID_TYPE_GAMEOBJECT:
            {
                GameObject* go = TO_GAMEOBJECT(obj);
                m_gameObjectStorage.insert(make_pair(obj->GetUIdFromGUID(), go));
                if( go->m_spawn != NULL)
                {
                    _sqlids_gameobjects.insert(make_pair(go->m_spawn->id, go ) );
                }
                CALL_INSTANCE_SCRIPT_EVENT( this, OnGameObjectPushToWorld )( go );
                sVMapInterface.LoadGameobjectModel(obj->GetGUID(), _mapId, go->GetDisplayId(), go->GetFloatValue(OBJECT_FIELD_SCALE_X), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), go->GetOrientation(), go->GetInstanceID(), go->GetPhaseMask());
            }break;

        case HIGHGUID_TYPE_DYNAMICOBJECT:
            m_DynamicObjectStorage[obj->GetLowGUID()] = TO_DYNAMICOBJECT(obj);
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

        /* Add the map wide objects */
        if(_mapWideStaticObjects.size())
        {
            for(set<Object* >::iterator itr = _mapWideStaticObjects.begin(); itr != _mapWideStaticObjects.end(); itr++)
            {
                count = (*itr)->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj);
                plObj->PushUpdateData(&m_createBuffer, count);
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

void MapMgr::PushStaticObject(Object* obj)
{
    _mapWideStaticObjects.insert(obj);

    switch(obj->GetTypeFromGUID())
    {
    case HIGHGUID_TYPE_VEHICLE:
        m_VehicleStorage[obj->GetUIdFromGUID()] = TO_VEHICLE(obj);
        break;

    case HIGHGUID_TYPE_CREATURE:
        m_CreatureStorage[obj->GetUIdFromGUID()] = TO_CREATURE(obj);
        break;

    case HIGHGUID_TYPE_GAMEOBJECT:
        m_gameObjectStorage.insert(make_pair(obj->GetUIdFromGUID(), TO_GAMEOBJECT(obj)));
        break;

    default:
        // maybe add a warning, shouldnt be needed
        break;
    }
}

void MapMgr::RemoveObject(Object* obj, bool free_guid)
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
    Player* plObj = (obj->IsPlayer()) ? TO_PLAYER( obj ) : NULLPLR;

    ///////////////////////////////////////
    // Remove object from all needed places
    ///////////////////////////////////////

    switch(obj->GetTypeFromGUID())
    {
    case HIGHGUID_TYPE_VEHICLE:
        {
            ASSERT(obj->GetUIdFromGUID() <= m_VehicleHighGuid);
            if(TO_VEHICLE(obj)->m_spawn != NULL)
                _sqlids_vehicles.erase(TO_VEHICLE(obj)->m_spawn->id);
            if(free_guid)
                _reusable_guids_vehicle.push_back(obj->GetUIdFromGUID());
            m_CreatureStorage.erase(obj->GetUIdFromGUID());
            CALL_INSTANCE_SCRIPT_EVENT( this, OnCreatureRemoveFromWorld )( TO_CREATURE(obj) );
        }break;

    case HIGHGUID_TYPE_CREATURE:
        {
            ASSERT(obj->GetUIdFromGUID() <= m_CreatureHighGuid);
            if(TO_CREATURE(obj)->m_spawn != NULL)
                _sqlids_creatures.erase(TO_CREATURE(obj)->GetSQL_id());
            if(free_guid)
                _reusable_guids_creature.push_back(obj->GetUIdFromGUID());
            m_CreatureStorage.erase(obj->GetUIdFromGUID());
            CALL_INSTANCE_SCRIPT_EVENT( this, OnCreatureRemoveFromWorld )( TO_CREATURE(obj) );
        }break;

    case HIGHGUID_TYPE_PET:
        {
            // check iterator
            if( __pet_iterator != m_PetStorage.end() && __pet_iterator->second == TO_PET(obj) )
                ++__pet_iterator;
            m_PetStorage.erase(obj->GetUIdFromGUID());
        }break;

    case HIGHGUID_TYPE_DYNAMICOBJECT:
        {
            m_DynamicObjectStorage.erase(obj->GetLowGUID());
        }break;

    case HIGHGUID_TYPE_GAMEOBJECT:
        {
            m_gameObjectStorage.erase(obj->GetUIdFromGUID());
            if(TO_GAMEOBJECT(obj)->m_spawn != NULL)
                _sqlids_gameobjects.erase(TO_GAMEOBJECT(obj)->m_spawn->id);
            CALL_INSTANCE_SCRIPT_EVENT( this, OnGameObjectRemoveFromWorld )( TO_GAMEOBJECT(obj) );
            sVMapInterface.UnLoadGameobjectModel(obj->GetGUID(), m_instanceID, _mapId);
        }break;

    case HIGHGUID_TYPE_PLAYER:
        {
            // check iterator
            if( __player_iterator != m_PlayerStorage.end() && __player_iterator->second == TO_PLAYER(obj) )
                ++__player_iterator;
        }break;
    }

    // That object types are not map objects. TODO: add AI groups here?
    if(obj->GetTypeId() == TYPEID_ITEM || obj->GetTypeId() == TYPEID_CONTAINER || obj->GetTypeId() == TYPEID_UNUSED)
        return;

    if(obj->GetTypeId() == TYPEID_CORPSE)
        m_corpses.erase( TO_CORPSE(obj) );

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
        _processQueue.erase(TO_PLAYER(obj));
        TO_PLAYER( obj )->ClearAllPendingUpdates();
    }

    // Remove object from all objects 'seeing' him
    for (Object::InRangeSet::iterator iter = obj->GetInRangeSetBegin(); iter != obj->GetInRangeSetEnd(); iter++)
    {
        if( (*iter) )
        {
            if( (*iter)->IsPlayer() )
            {
                if( TO_PLAYER( *iter )->IsVisible( obj ) && TO_PLAYER( *iter )->GetTransportGuid() != obj->GetGUID())
                    TO_PLAYER( *iter )->PushOutOfRange(obj->GetNewGUID());
                obj->DestroyForPlayer(TO_PLAYER( *iter ), obj->IsGameObject());
            }
            (*iter)->RemoveInRangeObject(obj);
        }
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
        m_PlayerStorage.erase( TO_PLAYER( obj )->GetLowGUID() );

        // Remove the session from our set if it is a player.
        for(set<Object* >::iterator itr = _mapWideStaticObjects.begin(); itr != _mapWideStaticObjects.end(); itr++)
        {
            plObj->PushOutOfRange((*itr)->GetNewGUID());
        }

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

void MapMgr::ChangeObjectLocation( Object* obj )
{
    // Items and containers are of no interest for us
    if( obj->GetTypeId() == TYPEID_ITEM || obj->GetTypeId() == TYPEID_CONTAINER || obj->GetMapMgr() != this )
        return;

    Player* plObj = NULLPLR;

    if( obj->IsPlayer() )
        plObj = TO_PLAYER( obj );
    Object* curObj = NULL;
    float fRange;

    ///////////////////////////////////////
    // Update in-range data for old objects
    ///////////////////////////////////////

    if(obj->HasInRangeObjects())
    {
        for (Object::InRangeSet::iterator iter = obj->GetInRangeSetBegin(), iter2; iter != obj->GetInRangeSetEnd();)
        {
            curObj = *iter;
            iter2 = iter++;
            if( curObj->IsPlayer() && obj->IsPlayer() && plObj && plObj->GetTransportGuid() && plObj->GetTransportGuid() == TO_PLAYER( curObj )->GetTransportGuid() )
                fRange = 0.0f; // unlimited distance for people on same boat
            else if( curObj->IsPlayer() && obj->IsPlayer() && plObj && plObj->GetVehicle() && plObj->GetVehicle() == TO_PLAYER( curObj )->GetVehicle() )
                fRange = 0.0f; // unlimited distance for people on same vehicle
            else if( curObj->GetTypeFromGUID() == HIGHGUID_TYPE_TRANSPORTER)
                fRange = 0.0f; // unlimited distance for transporters (only up to 2 cells +/- anyway.)
            else if( curObj->IsGameObject() && TO_GAMEOBJECT(curObj)->GetInfo() )
            {   // Crow: Arc, previous changes were only supporting Destructible.
                uint32 type = TO_GAMEOBJECT(curObj)->GetInfo()->Type;
                if( type == GAMEOBJECT_TYPE_TRANSPORT || type == GAMEOBJECT_TYPE_MAP_OBJECT || type == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING )
                    fRange = 0.0f;
                else
                    fRange = m_UpdateDistance; // normal distance
            }
            else
                fRange = m_UpdateDistance; // normal distance

            // If we have a update_distance, check if we are in range.
            if ( curObj != obj && fRange > 0.0f && !IsInRange(fRange, obj, curObj))
            {
                if( plObj )
                    plObj->RemoveIfVisible(curObj);

                if( curObj->IsPlayer() )
                    TO_PLAYER( curObj )->RemoveIfVisible(obj);

                curObj->RemoveInRangeObject(obj);

                if(obj->GetMapMgr() != this)
                    return; /* Something removed us. */

                obj->RemoveInRangeObject(iter2);
            }
        }
    }

    ///////////////////////////
    // Get new cell coordinates
    ///////////////////////////
    if(obj->GetMapMgr() != this)
        return; /* Something removed us. */

    if(obj->GetPositionX() >= _maxX || obj->GetPositionX() <= _minX ||
        obj->GetPositionY() >= _maxY || obj->GetPositionY() <= _minY)
    {
        if(obj->IsPlayer())
        {
            Player* plr = TO_PLAYER( obj );
            if(plr->GetBindMapId() != GetMapId())
            {
                plr->SafeTeleport(plr->GetBindMapId(),0,plr->GetBindPositionX(),plr->GetBindPositionY(),plr->GetBindPositionZ(),0);
                plr->GetSession()->SystemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
                return;
            }
            else
            {
                obj->GetPositionV()->ChangeCoords(plr->GetBindPositionX(),plr->GetBindPositionY(),plr->GetBindPositionZ(),0);
                plr->GetSession()->SystemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
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

    uint32 cellX = GetPosX(obj->GetPositionX());
    uint32 cellY = GetPosY(obj->GetPositionY());

    if(cellX >= _sizeX || cellY >= _sizeY)
        return;

    MapCell *objCell = GetCell(cellX, cellY);
    MapCell * pOldCell = obj->GetMapCell();
    if (!objCell)
    {
        objCell = Create(cellX,cellY);
        objCell->Init(cellX, cellY, _mapId, this);
    }

    // If object moved cell
    if (objCell != obj->GetMapCell())
    {
        // THIS IS A HACK!
        // Current code, if a creature on a long waypoint path moves from an active
        // cell into an inactive one, it will disable itself and will never return.
        // This is to prevent cpu leaks. I will think of a better solution very soon :P

        if(!objCell->IsActive() && !plObj && obj->Active)
            obj->Deactivate(this);

        if(obj->GetMapCell())
            obj->GetMapCell()->RemoveObject(obj);

        objCell->AddObject(obj);
        obj->SetMapCell(objCell);

        // if player we need to update cell activity
        // radius = 2 is used in order to update both
        // old and new cells
        if(obj->IsPlayer())
        {
            // have to unlock/lock here to avoid a deadlock situation.
            UpdateCellActivity(cellX, cellY, 2);
            if( pOldCell != NULL )
            {
                // only do the second check if theres -/+ 2 difference
                if( abs( (int)cellX - (int)pOldCell->_x ) > 2 ||
                    abs( (int)cellY - (int)pOldCell->_y ) > 2 )
                {
                    UpdateCellActivity( pOldCell->_x, pOldCell->_y, 2 );
                }
            }
        }
    }


    //////////////////////////////////////
    // Update in-range set for new objects
    //////////////////////////////////////
    uint32 startX = cellX > 0 ? cellX - 1 : 0;
    uint32 startY = cellY > 0 ? cellY - 1 : 0;
    uint32 endX = cellX <= _sizeX ? cellX + 1 : (_sizeX-1);
    uint32 endY = cellY <= _sizeY ? cellY + 1 : (_sizeY-1);

    UpdateInrangeSetOnCells(obj->GetGUID(), startX, endX, startY, endY);
    if(obj->IsUnit())
    {
        Unit* pobj = TO_UNIT(obj);
        pobj->OnPositionChange();
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
            cell = GetCell(posX, posY);
            if (cell)
                UpdateInRangeSet(guid, cell);
        }
    }
}

void MapMgr::UpdateInRangeSet( Object* obj, Player* plObj, MapCell* cell )
{
    if( cell == NULL )
        return;

    Object* curObj;
    Player* plObj2;
    int count;
    ObjectSet::iterator iter = cell->Begin();
    ObjectSet::iterator itr;
    float fRange;
    bool cansee, isvisible;

    while( iter != cell->End() )
    {
        curObj = *iter;
        ++iter;

        if( curObj == NULL )
            continue;

        if( curObj->IsPlayer() && obj->IsPlayer() && plObj && plObj->GetTransportGuid() && plObj->GetTransportGuid() == TO_PLAYER( curObj )->GetTransportGuid() )
            fRange = 0.0f; // unlimited distance for people on same boat
        else if( curObj->IsPlayer() && obj->IsPlayer() && plObj && plObj->GetVehicle() && plObj->GetVehicle() == TO_PLAYER( curObj )->GetVehicle() )
            fRange = 0.0f; // unlimited distance for people on same vehicle
        else if( curObj->GetTypeFromGUID() == HIGHGUID_TYPE_TRANSPORTER)
            fRange = 0.0f; // unlimited distance for transporters (only up to 2 cells +/- anyway.)
        else if( curObj->IsGameObject() && TO_GAMEOBJECT(curObj)->GetInfo() )
        {   // Crow: Arc, previous changes were only supporting Destructible.
            uint32 type = TO_GAMEOBJECT(curObj)->GetInfo()->Type;
            if( type == GAMEOBJECT_TYPE_TRANSPORT || type == GAMEOBJECT_TYPE_MAP_OBJECT || type == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING )
                fRange = 0.0f;
            else
                fRange = m_UpdateDistance; // normal distance
        }
        else
            fRange = m_UpdateDistance; // normal distance

        // Add if we are not ourself and range == 0 or distance is withing range.
        if ( curObj != obj && (fRange == 0.0f || IsInRange(fRange, obj, curObj)))
        {
            if( !obj->IsInRangeSet( curObj ) )
            {
                // Object in range, add to set
                obj->AddInRangeObject( curObj );
                curObj->AddInRangeObject( obj );

                if( curObj->IsPlayer() )
                {
                    plObj2 = TO_PLAYER( curObj );

                    if( plObj2->CanSee( obj ) && !plObj2->IsVisible( obj ) )
                    {
                        count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2);
                        plObj2->PushUpdateData(&m_createBuffer, count);
                        plObj2->AddVisibleObject(obj);
                        m_createBuffer.clear();
                    }
                }

                if( plObj != NULL )
                {
                    if( plObj->CanSee( curObj ) && !plObj->IsVisible( curObj ) )
                    {
                        count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj );
                        plObj->PushUpdateData( &m_createBuffer, count );
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
                    plObj2 = TO_PLAYER( curObj );
                    cansee = plObj2->CanSee(obj);
                    isvisible = plObj2->GetVisibility(obj, &itr);
                    if(!cansee && isvisible)
                    {
                        plObj2->PushOutOfRange(obj->GetNewGUID());
                        plObj2->RemoveVisibleObject(itr);
                    }
                    else if(cansee && !isvisible)
                    {
                        count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2);
                        plObj2->PushUpdateData(&m_createBuffer, count);
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
                        plObj->PushOutOfRange( curObj->GetNewGUID() );
                        plObj->RemoveVisibleObject( itr );
                    }
                    else if(cansee && !isvisible)
                    {
                        count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj );
                        plObj->PushUpdateData( &m_createBuffer, count );
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
    float fRange;
    Object* obj = NULL;
    Player* plObj = NULL;
    Object* curObj = NULL;
    Player* plObj2 = NULL;
    bool cansee, isvisible;
    ObjectSet::iterator itr;
    ObjectSet::iterator iter = cell->Begin();
    obj = _GetObject(guid);
    if(obj == NULL)
        return;
    if(obj->IsPlayer())
        plObj = TO_PLAYER(obj);

    while( iter != cell->End() )
    {
        curObj = *iter;
        ++iter;

        if( curObj == NULL )
            continue;

        if( curObj->IsPlayer() && obj->IsPlayer() && plObj && plObj->GetTransportGuid() && plObj->GetTransportGuid() == TO_PLAYER( curObj )->GetTransportGuid() )
            fRange = 0.0f; // unlimited distance for people on same boat
        else if( curObj->IsPlayer() && obj->IsPlayer() && plObj && plObj->GetVehicle() && plObj->GetVehicle() == TO_PLAYER( curObj )->GetVehicle() )
            fRange = 0.0f; // unlimited distance for people on same vehicle
        else if( curObj->GetTypeFromGUID() == HIGHGUID_TYPE_TRANSPORTER)
            fRange = 0.0f; // unlimited distance for transporters (only up to 2 cells +/- anyway.)
        else if( curObj->IsGameObject() && TO_GAMEOBJECT(curObj)->GetInfo() )
        {   // Crow: Arc, previous changes were only supporting Destructible.
            uint32 type = TO_GAMEOBJECT(curObj)->GetInfo()->Type;
            if( type == GAMEOBJECT_TYPE_TRANSPORT || type == GAMEOBJECT_TYPE_MAP_OBJECT || type == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING )
                fRange = 0.0f;
            else
                fRange = m_UpdateDistance; // normal distance
        }
        else
            fRange = m_UpdateDistance; // normal distance

        // Add if we are not ourself and range == 0 or distance is withing range.
        if ( curObj != obj && (fRange == 0.0f || IsInRange(fRange, obj, curObj)))
        {
            if( !obj->IsInRangeSet( curObj ) )
            {
                // Object in range, add to set
                obj->AddInRangeObject( curObj );
                curObj->AddInRangeObject( obj );

                if( curObj->IsPlayer() )
                {
                    plObj2 = TO_PLAYER( curObj );

                    if( plObj2->CanSee( obj ) && !plObj2->IsVisible( obj ) )
                    {
                        count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2);
                        plObj2->PushUpdateData(&m_createBuffer, count);
                        plObj2->AddVisibleObject(obj);
                        m_createBuffer.clear();
                    }
                }

                if( plObj != NULL )
                {
                    if( plObj->CanSee( curObj ) && !plObj->IsVisible( curObj ) )
                    {
                        count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj );
                        plObj->PushUpdateData( &m_createBuffer, count );
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
                    plObj2 = TO_PLAYER( curObj );
                    cansee = plObj2->CanSee(obj);
                    isvisible = plObj2->GetVisibility(obj, &itr);
                    if(!cansee && isvisible)
                    {
                        plObj2->PushOutOfRange(obj->GetNewGUID());
                        plObj2->RemoveVisibleObject(itr);
                    }
                    else if(cansee && !isvisible)
                    {
                        count = obj->BuildCreateUpdateBlockForPlayer(&m_createBuffer, plObj2);
                        plObj2->PushUpdateData(&m_createBuffer, count);
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
                        plObj->PushOutOfRange( curObj->GetNewGUID() );
                        plObj->RemoveVisibleObject( itr );
                    }
                    else if(cansee && !isvisible)
                    {
                        count = curObj->BuildCreateUpdateBlockForPlayer( &m_createBuffer, plObj );
                        plObj->PushUpdateData( &m_createBuffer, count );
                        plObj->AddVisibleObject( curObj );
                        m_createBuffer.clear();
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

    Object* pObj;
    Player* pOwner;
    unordered_set<Player*  >::iterator it_start, it_end, itr;
    Player* lplr;
    uint32 count = 0;

    UpdateQueue::iterator iter = _updates.begin();
    PUpdateQueue::iterator it, eit;

    for(; iter != _updates.end();)
    {
        pObj = *iter;
        ++iter;
        if(!pObj)
            continue;

        if(pObj->IsItem() || pObj->IsContainer())
        {
            // our update is only sent to the owner here.
            pOwner = TO_ITEM(pObj)->GetOwner();
            if( pOwner != NULL )
            {
                count = TO_ITEM( pObj )->BuildValuesUpdateBlockForPlayer( &m_updateBuffer, pOwner );
                // send update to owner
                if( count )
                {
                    pOwner->PushUpdateData( &m_updateBuffer, count );
                    m_updateBuffer.clear();
                }
            }
        }
        else
        {
            if( pObj->IsInWorld() )
            {
                // players have to receive their own updates ;)
                if( pObj->IsPlayer() )
                {
                    // need to be different! ;)
                    count = pObj->BuildValuesUpdateBlockForPlayer( &m_updateBuffer, TO_PLAYER( pObj ) );
                    if( count )
                    {
                        TO_PLAYER( pObj )->PushUpdateData( &m_updateBuffer, count );
                        m_updateBuffer.clear();
                    }
                }

                if( pObj->IsUnit() && pObj->HasUpdateField( UNIT_FIELD_HEALTH ) )
                    TO_UNIT( pObj )->EventHealthChangeSinceLastUpdate();

                // build the update
                count = pObj->BuildValuesUpdateBlockForPlayer( &m_updateBuffer, TO_PLAYER(NULLPLR) );
                if( count )
                {
                    it_start = pObj->GetInRangePlayerSetBegin();
                    it_end = pObj->GetInRangePlayerSetEnd();
                    for(itr = it_start; itr != it_end;)
                    {
                        lplr = *itr;
                        ++itr;
                        // Make sure that the target player can see us.
                        if( lplr->IsVisible( pObj ) )
                            lplr->PushUpdateData( &m_updateBuffer, count );
                    }
                    m_updateBuffer.clear();
                }
            }
        }
        pObj->ClearUpdateMask();
    }
    _updates.clear();

    // generate pending a9packets and send to clients.
    Player* plyr;
    for(it = _processQueue.begin(); it != _processQueue.end();)
    {
        plyr = *it;
        eit = it;
        ++it;
        _processQueue.erase(eit);
        if(plyr->GetMapMgr() == this)
            plyr->ProcessPendingUpdates(&m_updateBuildBuffer, &m_compressionBuffer);
    }
    m_updateMutex.Release();
}

void MapMgr::UpdateAllCells(bool apply, uint32 areamask)
{
    // eek
    uint16 AreaID = 0;
    MapCell * cellInfo;
    CellSpawns * spawns;
    uint32 StartX = 0, EndX = 0, StartY = 0, EndY = 0;
    GetBaseMap()->GetCellLimits(StartX, EndX, StartY, EndY);
    if(!areamask)
        sLog.Info("MapMgr", "Updating all cells for map %03u, server might lag.", _mapId);
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
                    // There is no spoon. Err... cell.
                    cellInfo = Create( x , y );
                    cellInfo->Init( x , y , _mapId , this );
                    sLog.Debug("MapMgr","Created cell [%u,%u] on map %u (instance %u)." , x , y , _mapId , m_instanceID );
                }

                AddForcedCell(cellInfo, 0);
                if (!cellInfo->IsLoaded())
                {
                    spawns = _map->GetSpawnsList( x , y );
                    if( spawns )
                        cellInfo->LoadObjects( spawns );
                }
            }
            else
            {
                if(!cellInfo)
                    continue;

                RemoveForcedCell(cellInfo);
            }
        }
    }
    if(!areamask)
        sLog.Success("MapMgr", "Cell updating success for map %03u", _mapId);
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
    uint16 vwaterType = 0;
    uint16 mapWaterType = GetBaseMap()->GetWaterType(x, y);
    float mapWaterheight = GetBaseMap()->GetWaterHeight(x, y, z);
    float vmapWaterHeight = sVMapInterface.GetWaterHeight(GetMapId(), x, y, z, vwaterType);
    if(!(mapWaterType & 0x10) && vwaterType && vmapWaterHeight != NO_WMO_HEIGHT)
    { outHeight = vmapWaterHeight; outType = vwaterType; }
    else
    { outHeight = mapWaterheight; outType = mapWaterType; }
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

void MapMgr::ObjectUpdated(Object* obj)
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
    if(!apply)
    {
        // We're clearing.
        for(ObjectSet::iterator itr = plr->m_visibleFarsightObjects.begin(); itr != plr->m_visibleFarsightObjects.end();
            itr++)
        {
            // Send destroy
            plr->PushOutOfRange((*itr)->GetNewGUID());
        }
        plr->m_visibleFarsightObjects.clear();
    }
    else
    {
        plr->m_visibleFarsightObjects.clear();
        uint32 cellX = GetPosX(farsight->GetPositionX());
        uint32 cellY = GetPosY(farsight->GetPositionY());
        uint32 endX = (cellX <= _sizeX) ? cellX + 1 : (_sizeX-1);
        uint32 endY = (cellY <= _sizeY) ? cellY + 1 : (_sizeY-1);
        uint32 startX = cellX > 0 ? cellX - 1 : 0;
        uint32 startY = cellY > 0 ? cellY - 1 : 0;
        uint32 posX, posY;
        MapCell *cell;
        Object* obj;
        MapCell::ObjectSet::iterator iter, iend;
        uint32 count;
        for (posX = startX; posX <= endX; ++posX )
        {
            for (posY = startY; posY <= endY; ++posY )
            {
                cell = GetCell(posX, posY);
                if (cell)
                {
                    iter = cell->Begin();
                    iend = cell->End();
                    for(; iter != iend; iter++)
                    {
                        obj = (*iter);
                        if(!plr->IsVisible(obj) && plr->CanSee(obj) && farsight->GetDistance2dSq(obj) <= m_UpdateDistance)
                        {
                            ByteBuffer buf;
                            count = obj->BuildCreateUpdateBlockForPlayer(&buf, plr);
                            plr->PushUpdateData(&buf, count);
                            plr->m_visibleFarsightObjects.insert(obj);
                        }
                    }

                }
            }
        }
    }
}

void MapMgr::ChangeFarsightLocation(Player* plr, float X, float Y, bool apply)
{
    if(!apply)
    {
        // We're clearing.
        for(ObjectSet::iterator itr = plr->m_visibleFarsightObjects.begin(); itr != plr->m_visibleFarsightObjects.end();
            itr++)
        {
            // Send destroy
            plr->PushOutOfRange((*itr)->GetNewGUID());
        }
        plr->m_visibleFarsightObjects.clear();
    }
    else
    {
        uint32 cellX = GetPosX(X);
        uint32 cellY = GetPosY(Y);
        uint32 endX = (cellX <= _sizeX) ? cellX + 1 : (_sizeX-1);
        uint32 endY = (cellY <= _sizeY) ? cellY + 1 : (_sizeY-1);
        uint32 startX = cellX > 0 ? cellX - 1 : 0;
        uint32 startY = cellY > 0 ? cellY - 1 : 0;
        uint32 posX, posY;
        MapCell *cell;
        Object* obj;
        MapCell::ObjectSet::iterator iter, iend;
        uint32 count;
        for (posX = startX; posX <= endX; ++posX )
        {
            for (posY = startY; posY <= endY; ++posY )
            {
                cell = GetCell(posX, posY);
                if (cell)
                {
                    iter = cell->Begin();
                    iend = cell->End();
                    for(; iter != iend; iter++)
                    {
                        obj = (*iter);
                        if(!plr->IsVisible(obj) && plr->CanSee(obj) && obj->GetDistance2dSq(X, Y) <= m_UpdateDistance)
                        {
                            ByteBuffer buf;
                            count = obj->BuildCreateUpdateBlockForPlayer(&buf, plr);
                            plr->PushUpdateData(&buf, count);
                            plr->m_visibleFarsightObjects.insert(obj);
                        }
                    }

                }
            }
        }
    }
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

    /* add static objects */
    for(set<Object* >::iterator itr = _mapWideStaticObjects.begin(); itr != _mapWideStaticObjects.end(); itr++)
        PushStaticObject(*itr);

    /* load corpses */
    objmgr.LoadCorpses(this);

    // initialize worldstates
    sWorldStateTemplateManager.ApplyMapTemplate(this);

    // Call script OnLoad virtual procedure
    CALL_INSTANCE_SCRIPT_EVENT( this, OnLoad )();

    if( GetMapInfo()->type == INSTANCE_NULL )
    {
        sHookInterface.OnContinentCreate(this);

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
#ifdef WIN32
    HANDLE hThread = GetCurrentThread();
#endif
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
        // check for a non-raid instance, these expire after 10 minutes.
        if(GetMapInfo()->type == INSTANCE_NONRAID || pInstance->m_isBattleground)
        {
            pInstance->m_mapMgr = NULLMAPMGR;
            sInstanceMgr._DeleteInstance(pInstance, true);
        }
        else
            pInstance->m_mapMgr = NULLMAPMGR;
    }
    else if(GetMapInfo()->type == INSTANCE_NULL)
        sInstanceMgr.m_singleMaps[GetMapId()] = NULLMAPMGR;

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

void MapMgr::AddObject(Object* obj)
{
    m_objectinsertlock.Acquire();
    m_objectinsertpool.insert(obj);
    m_objectinsertlock.Release();
}


Unit* MapMgr::GetUnit(const uint64 & guid)
{
    switch(GUID_HIPART(guid))
    {
    case HIGHGUID_TYPE_CREATURE:
        return GetCreature( GUID_LOPART(guid) );
        break;

    case HIGHGUID_TYPE_PLAYER:
        return GetPlayer( (uint32)guid );
        break;

    case HIGHGUID_TYPE_PET:
        return GetPet( GUID_LOPART(guid) );
        break;

    case HIGHGUID_TYPE_VEHICLE:
        return GetVehicle( GUID_LOPART(guid) );
        break;
    }

    return NULLUNIT;
}

Object* MapMgr::_GetObject(const uint64 & guid)
{
    switch(GUID_HIPART(guid))
    {
    case    HIGHGUID_TYPE_VEHICLE:
        return GetVehicle(GUID_LOPART(guid));
        break;
    case    HIGHGUID_TYPE_GAMEOBJECT:
        return GetGameObject(GUID_LOPART(guid));
        break;
    case    HIGHGUID_TYPE_CREATURE:
        return GetCreature(GUID_LOPART(guid));
        break;
    case    HIGHGUID_TYPE_DYNAMICOBJECT:
        return GetDynamicObject((uint32)guid);
        break;
    case    HIGHGUID_TYPE_TRANSPORTER:
        return objmgr.GetTransporter(GUID_LOPART(guid));
        break;
    case HIGHGUID_TYPE_CORPSE:
        return objmgr.GetCorpse(GUID_LOPART(guid));
        break;
    default:
        return GetUnit(guid);
        break;
    }
}

Object* MapMgr::GetObjectClosestToCoords(uint32 entry, float x, float y, float z, float ClosestDist, int32 forcedtype)
{
    MapCell * pCell = GetCell(GetPosX(x), GetPosY(y));
    if(pCell == NULL)
        return NULL;

    Object* ClosestObject = NULLOBJ;
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

bool MapMgr::CanUseCollision(Object* obj)
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
    ++mLoopCounter;
    uint32 mstime = getMSTime();
    uint32 difftime = mstime - lastUnitUpdate;
    if(difftime > 500)
        difftime = 500;

    // Update our objects.
    {
        ActiveLock.Acquire();
        if(activeCreatures.size())
        {
            Creature* ptr;
            __creature_iterator = activeCreatures.begin();
            for(; __creature_iterator != activeCreatures.end();)
            {
                ptr = *__creature_iterator;
                ++__creature_iterator;

                ptr->Update(difftime);
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

                ptr3->Update(difftime);
            }
        }

        if(!SetThreadState(THREADSTATE_AWAITING))
            return;

        if(m_PetStorage.size())
        {
            Pet* ptr2;
            __pet_iterator = m_PetStorage.begin();
            for(; __pet_iterator != m_PetStorage.end();)
            {
                ptr2 = __pet_iterator->second;
                ++__pet_iterator;

                ptr2->Update(difftime);
            }
        }
        ActiveLock.Release();
    }

    // Update any events.
    eventHolder.Update(difftime);
    if(!SetThreadState(THREADSTATE_AWAITING))
        return;

    // Update our collision system via singular map system
    sVMapInterface.UpdateSingleMap(_mapId, m_instanceID, difftime);
    if(!SetThreadState(THREADSTATE_AWAITING))
        return;

    // Call our script's update function.
    _script->Update(difftime);
    if(!SetThreadState(THREADSTATE_AWAITING))
        return;

    // Update players.
    Player* ptr4;
    __player_iterator = m_PlayerStorage.begin();
    for(; __player_iterator != m_PlayerStorage.end();)
    {
        ptr4 = __player_iterator->second;
        ++__player_iterator;
        if(GetThreadState() == THREADSTATE_TERMINATE)
            return;

        ptr4->Update( difftime );
    }

    lastUnitUpdate = mstime;
    if(!SetThreadState(THREADSTATE_AWAITING))
        return;

    // Update gameobjects (every 2nd tick only).
    if( mLoopCounter % 2 )
    {
        difftime = mstime - lastGameobjectUpdate;

        GameObject* ptr5;
        __gameobject_iterator = activeGameObjects.begin();
        for(; __gameobject_iterator != activeGameObjects.end(); )
        {
            ptr5 = *__gameobject_iterator;
            ++__gameobject_iterator;

            ptr5->Update( difftime );
        }
        lastGameobjectUpdate = mstime;

        if(!SetThreadState(THREADSTATE_AWAITING))
            return;

        difftime = mstime - lastDynamicUpdate;
        DynamicObject* ptr6;
        DynamicObjectStorageMap::iterator itr = m_DynamicObjectStorage.begin();
        for(; itr != m_DynamicObjectStorage.end(); )
        {
            ptr6 = itr->second;
            ++itr;

            ptr6->UpdateTargets( difftime );
        }
        lastDynamicUpdate = mstime;
    }

    if(!SetThreadState(THREADSTATE_AWAITING))
        return;

    // Sessions are updated every loop.
    {
        int result = 0;
        WorldSession * MapSession;
        SessionSet::iterator itr = MapSessions.begin();
        SessionSet::iterator it2;

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
                ptr = NULLPLR;
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

bool MapMgr::IsInRange(float fRange, Object* obj, Object* currentobj)
{
    // First distance check, are we in range?
    if(currentobj->GetDistance2dSq( obj ) > fRange )
        return false;
    return true;
}

void MapMgr::SendMessageToCellPlayers(Object* obj, WorldPacket * packet, uint32 cell_radius /* = 2 */)
{
    uint32 cellX = GetPosX(obj->GetPositionX());
    uint32 cellY = GetPosY(obj->GetPositionY());
    uint32 endX = ((cellX+cell_radius) <= _sizeX) ? cellX + cell_radius : (_sizeX-1);
    uint32 endY = ((cellY+cell_radius) <= _sizeY) ? cellY + cell_radius : (_sizeY-1);
    uint32 startX = (cellX-cell_radius) > 0 ? cellX - cell_radius : 0;
    uint32 startY = (cellY-cell_radius) > 0 ? cellY - cell_radius : 0;

    MapCell *cell;
    uint32 posX, posY;
    MapCell::ObjectSet::iterator iter, iend;
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
                        TO_PLAYER(*iter)->GetSession()->SendPacket(packet);
                    }
                }
            }
        }
    }
}

void MapMgr::SendChatMessageToCellPlayers(Object* obj, WorldPacket * packet, uint32 cell_radius, uint32 langpos, int32 lang, WorldSession * originator)
{
    uint32 cellX = GetPosX(obj->GetPositionX());
    uint32 cellY = GetPosY(obj->GetPositionY());
    uint32 endX = ((cellX+cell_radius) <= _sizeX) ? cellX + cell_radius : (_sizeX-1);
    uint32 endY = ((cellY+cell_radius) <= _sizeY) ? cellY + cell_radius : (_sizeY-1);
    uint32 startX = (cellX-cell_radius) > 0 ? cellX - cell_radius : 0;
    uint32 startY = (cellY-cell_radius) > 0 ? cellY - cell_radius : 0;

    uint32 posX, posY;
    MapCell *cell;
    MapCell::ObjectSet::iterator iter, iend;
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
                            TO_PLAYER(*iter)->GetSession()->SendChatPacket(packet, langpos, lang, originator);
                    }
                }
            }
        }
    }
}

Vehicle* MapMgr::GetSqlIdVehicle(uint32 sqlid)
{
    VehicleSqlIdMap::iterator itr = _sqlids_vehicles.find(sqlid);
    return (itr == _sqlids_vehicles.end()) ? NULLVEHICLE : itr->second;
}

Creature* MapMgr::GetSqlIdCreature(uint32 sqlid)
{
    CreatureSqlIdMap::iterator itr = _sqlids_creatures.find(sqlid);
    return (itr == _sqlids_creatures.end()) ? NULLCREATURE : itr->second;
}

GameObject* MapMgr::GetSqlIdGameObject(uint32 sqlid)
{
    GameObjectSqlIdMap::iterator itr = _sqlids_gameobjects.find(sqlid);
    return (itr == _sqlids_gameobjects.end()) ? NULLGOB : itr->second;
}

void MapMgr::HookOnAreaTrigger(Player* plr, uint32 id)
{
    switch (id)
    {
    case 4591:
        //Only opens when the first one steps in, if 669 if you find a way, put it in :P (else was used to increase the time the door stays opened when another one steps on it)
        GameObject* door = TO_GAMEOBJECT(GetObjectClosestToCoords(184212, 803.827f, 6869.38f, -38.5434f, 99999.0f, TYPEID_GAMEOBJECT));
        if (door && (door->GetState() == 1))
            door->SetState(0);
        break;
    }
}

Vehicle* MapMgr::CreateVehicle(uint32 entry)
{
    uint32 low_guid = 0;
    if(_reusable_guids_vehicle.size())
    {
        low_guid = _reusable_guids_vehicle.front();
        _reusable_guids_vehicle.pop_front();
    } else low_guid = ++m_VehicleHighGuid;

    Vehicle *v = new Vehicle(MAKE_NEW_GUID(low_guid, entry, HIGHGUID_TYPE_VEHICLE));
    v->Init();

    ASSERT( v->GetTypeFromGUID() == HIGHGUID_TYPE_VEHICLE );
    m_VehicleStorage.insert( make_pair(v->GetUIdFromGUID(), v));
    return v;
}

Creature* MapMgr::CreateCreature(uint32 entry)
{
    uint32 low_guid = 0;
    if(_reusable_guids_creature.size())
    {
        low_guid = _reusable_guids_creature.front();
        _reusable_guids_creature.pop_front();
    } else low_guid = ++m_CreatureHighGuid;

    Creature *cr = new Creature(MAKE_NEW_GUID(low_guid, entry, HIGHGUID_TYPE_CREATURE));
    cr->Init();
    ASSERT( cr->GetTypeFromGUID() == HIGHGUID_TYPE_CREATURE );
    return cr;
}

Summon* MapMgr::CreateSummon(uint32 entry)
{
    uint32 low_guid = 0;
    if(_reusable_guids_creature.size())
    {
        low_guid = _reusable_guids_creature.front();
        _reusable_guids_creature.pop_front();
    } else low_guid = ++m_CreatureHighGuid;

    Summon *sum = new Summon(MAKE_NEW_GUID(low_guid, entry, HIGHGUID_TYPE_CREATURE));
    sum->Init();
    ASSERT( sum->GetTypeFromGUID() == HIGHGUID_TYPE_CREATURE );
    return sum;
}

GameObject* MapMgr::CreateGameObject(uint32 entry)
{
    // Validate the entry
    GameObjectInfo *goi = GameObjectNameStorage.LookupEntry( entry );
    if( goi == NULL )
    {
        sLog.Warning("MapMgr", "Skipping CreateGameObject for entry %u due to incomplete database.", entry);
        if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
            WorldDatabase.Execute("DELETE FROM gameobject_spawns WHERE entry = '%u';", entry);
        return NULLGOB;
    }

    GameObject *go = new GameObject(MAKE_NEW_GUID(++m_GOHighGuid, entry, HIGHGUID_TYPE_GAMEOBJECT));
    go->Init();
    ASSERT( go->GetTypeFromGUID() == HIGHGUID_TYPE_GAMEOBJECT );
    return go;
}

DynamicObject* MapMgr::CreateDynamicObject()
{
    DynamicObject* dyn = new DynamicObject(HIGHGUID_TYPE_DYNAMICOBJECT, (++m_DynamicObjectHighGuid));
    dyn->Init();
    ASSERT( dyn->GetTypeFromGUID() == HIGHGUID_TYPE_DYNAMICOBJECT );
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
                sEventMgr.AddEvent(TO_UNIT(ptr), &Unit::EventCastSpell, TO_UNIT(__player_iterator->second), sp, EVENT_AURA_APPLY, 250, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
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
