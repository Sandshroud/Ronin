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

//
// MapCell.cpp
//

#include "StdAfx.h"

MapCell::MapCell() : _pendingLock(), _objLock()
{
    _forcedActive = false;
#if STACKED_MEMORY_ALLOCATION == 1
    _creatureStack = NULL;
    _gameobjectStack = NULL;
#endif
}

MapCell::~MapCell()
{
    UnloadCellData(true);
}

void MapCell::Init(uint32 x, uint32 y, uint32 mapid, MapInstance* instance)
{
    _mapData = instance->GetBaseMap();
    _instance = instance;
    _active = false;
    _loaded = false;
    _x = x;
    _y = y;
    _unloadpending=false;
}

void MapCell::AddObject(WorldObject* obj)
{
    RWGuard guard(_objLock, true);
    if(obj->IsActiveObject() && !obj->IsActivated())
        m_deactivatedObjects[obj->GetGUID()] = obj;
    else if(obj->IsPlayer())
        m_activePlayerSet[obj->GetGUID()] = obj;
    else
    {
        m_activeNonPlayerSet[obj->GetGUID()] = obj;
        if(obj->IsCreature())
            m_creatureSet[obj->GetGUID()] = obj;
        else if(obj->IsGameObject())
            m_gameObjectSet[obj->GetGUID()] = obj;
    }
}

void MapCell::RemoveObject(WorldObject* obj)
{
    Guard guard(_pendingLock);
    m_pendingRemovals.insert(obj->GetGUID());
    _instance->CellActionPending(_x, _y);
}

void MapCell::ProcessPendingActions()
{
    Guard guard(_pendingLock);
    RWGuard objguard(_objLock, true);
    while(!m_pendingRemovals.empty())
    {
        WoWGuid guid = *m_pendingRemovals.begin();
        m_pendingRemovals.erase(m_pendingRemovals.begin());
        m_activePlayerSet.erase(guid);
        m_activeNonPlayerSet.erase(guid);
        m_creatureSet.erase(guid);
        m_gameObjectSet.erase(guid);
        m_deactivatedObjects.erase(guid);
    }

    MapCell::CellObjectMap::iterator itr;
    while(!m_pendingDeactivate.empty())
    {
        WoWGuid guid = *m_pendingDeactivate.begin();
        m_pendingDeactivate.erase(m_pendingDeactivate.begin());
        if((itr = m_activeNonPlayerSet.find(guid)) != m_activeNonPlayerSet.end())
        {
            WorldObject *obj = itr->second;
            m_activeNonPlayerSet.erase(itr);
            m_activePlayerSet.erase(guid);
            m_activeNonPlayerSet.erase(guid);
            m_creatureSet.erase(guid);
            m_gameObjectSet.erase(guid);
            m_deactivatedObjects[obj->GetGUID()] = obj;
        }
    }

    while(!m_pendingReactivate.empty())
    {
        WoWGuid guid = *m_pendingReactivate.begin();
        m_pendingReactivate.erase(m_pendingReactivate.begin());
        if((itr = m_deactivatedObjects.find(guid)) != m_deactivatedObjects.end())
        {
            WorldObject *obj = itr->second;
            m_deactivatedObjects.erase(itr);
            AddObject(obj);
        }
    }
}

void MapCell::ReactivateObject(WorldObject *obj)
{
    m_pendingReactivate.insert(obj->GetGUID());
}

void MapCell::DeactivateObject(WorldObject *obj)
{
    m_pendingDeactivate.insert(obj->GetGUID());
}

WorldObject *MapCell::FindObject(WoWGuid guid, bool searchDeactivated)
{
    RWGuard guard(_objLock, false);
    MapCell::CellObjectMap::iterator itr;
    if((itr = m_activePlayerSet.find(guid)) != m_activePlayerSet.end() || (itr = m_activeNonPlayerSet.find(guid)) != m_activeNonPlayerSet.end())
        return itr->second;
    if(searchDeactivated && (itr = m_deactivatedObjects.find(guid)) != m_deactivatedObjects.end())
        return itr->second;
    return NULL;
}

void MapCell::ProcessObjectSets(WorldObject *obj, ObjectProcessCallback *callback, uint32 objectMask)
{
    RWGuard guard(_objLock, false);

    WorldObject *curObj;
    if(objectMask == 0)
    {
        for(MapCell::CellObjectMap::iterator itr = m_activeNonPlayerSet.begin(); itr != m_activeNonPlayerSet.end(); itr++)
        {
            if((curObj = itr->second) == NULL || obj == curObj)
                continue;
            (*callback)(obj, curObj);
        }

        for(MapCell::CellObjectMap::iterator itr = m_activePlayerSet.begin(); itr != m_activePlayerSet.end(); itr++)
        {
            if((curObj = itr->second) == NULL || obj == curObj)
                continue;
            (*callback)(obj, curObj);
        }
    }
    else
    {
        if(objectMask & TYPEMASK_TYPE_UNIT)
        {
            for(MapCell::CellObjectMap::iterator itr = m_creatureSet.begin(); itr != m_creatureSet.end(); itr++)
            {
                if((curObj = itr->second) == NULL || obj == curObj)
                    continue;
                (*callback)(obj, curObj);
            }
        }

        if(objectMask & TYPEMASK_TYPE_PLAYER)
        {
            for(MapCell::CellObjectMap::iterator itr = m_activePlayerSet.begin(); itr != m_activePlayerSet.end(); itr++)
            {
                if((curObj = itr->second) == NULL || obj == curObj)
                    continue;
                (*callback)(obj, curObj);
            }
        }

        if(objectMask & TYPEMASK_TYPE_GAMEOBJECT)
        {
            for(MapCell::CellObjectMap::iterator itr = m_gameObjectSet.begin(); itr != m_gameObjectSet.end(); itr++)
            {
                if((curObj = itr->second) == NULL || obj == curObj)
                    continue;
                (*callback)(obj, curObj);
            }
        }

        if(objectMask & TYPEMASK_TYPE_DEACTIVATED)
        {
            for(MapCell::CellObjectMap::iterator itr = m_deactivatedObjects.begin(); itr != m_deactivatedObjects.end(); itr++)
            {
                if((curObj = itr->second) == NULL || obj == curObj)
                    continue;
                (*callback)(obj, curObj);
            }
        }
    }
}

void MapCell::SetActivity(bool state)
{
    uint32 x = _x/CellsPerTile, y = _y/CellsPerTile;
    if(state && _unloadpending)
        CancelPendingUnload();
    else if(!state && !_unloadpending)
        QueueUnloadPending();
    _active = state;
}

uint32 MapCell::LoadCellData(CellSpawns * sp)
{
    RWGuard guard(_objLock, true);
    if(_loaded == true)
        return 0;

    // start calldown for cell map loading
    _mapData->CellLoaded(_x, _y);
    _loaded = true;

    // check if we have a spawn map, otherwise no use continuing
    if(sp == NULL)
        return 0;

    uint32 loadCount = 0, mapId = _instance->GetMapId();
    InstanceData *data = _instance->m_iData;
#if STACKED_MEMORY_ALLOCATION == 1
    if(uint32 stackSize = sp->CreatureSpawns.size())//got creatures
    {
        uint32 index = 0;
        _creatureStack = new Creature[stackSize];
        for(CreatureSpawnArray::iterator i=sp->CreatureSpawns.begin();i!=sp->CreatureSpawns.end();++i)
        {
            CreatureSpawn *spawn = *i;
            // Grab an allocation from our stack
            uint32 creatureIndex = index++;
            Creature *allocation = &_creatureStack[creatureIndex];

            uint8 creatureState = 0;
            if(data) data->GetObjectState(spawn->guid, creatureState);

            WoWGuid guid = spawn->guid;
            if(_instance->IsInstance())
            {
                Loki::AssocVector<WoWGuid, WoWGuid>::iterator itr;
                if((itr = m_sqlIdToGuid.find(guid)) != m_sqlIdToGuid.end())
                    guid = itr->second;
                else m_sqlIdToGuid.insert(std::make_pair(spawn->guid, (guid = MAKE_NEW_GUID(sInstanceMgr.AllocateCreatureGuid(), spawn->guid.getEntry(), HIGHGUID_TYPE_UNIT))));
            }

            // Mark our index
            _instance->ConstructCreature(guid, allocation);
            m_ctrIndex[creatureIndex] = guid;

            // Finish loading our allocation
            allocation->Load(mapId, spawn->x, spawn->y, spawn->z, spawn->o, _instance->iInstanceMode, spawn);
            allocation->SetInstanceID(_instance->GetInstanceID());
            if(!allocation->CanAddToWorld())
                continue;

            if(_instance->IsCreaturePoolUpdating())
                _instance->AddObject(allocation);
            else allocation->PushToWorld(_instance);
            loadCount++;
        }
    }

    if(uint32 stackSize = sp->GameObjectSpawns.size())//got GOs
    {
        uint32 index = 0;
        _gameobjectStack = new GameObject[stackSize];
        for(GameObjectSpawnArray::iterator i = sp->GameObjectSpawns.begin(); i != sp->GameObjectSpawns.end(); i++)
        {
            GameObjectSpawn *spawn = *i;
            // Grab an allocation from our stack
            uint32 gobjIndex = index++;
            GameObject *allocation = &_gameobjectStack[gobjIndex];

            uint8 gameObjState = 0x00;
            if(data == NULL || !data->GetObjectState(spawn->guid, gameObjState))
                gameObjState = spawn->state;

            WoWGuid guid = spawn->guid;
            if(_instance->IsInstance())
            {
                Loki::AssocVector<WoWGuid, WoWGuid>::iterator itr;
                if((itr = m_sqlIdToGuid.find(guid)) != m_sqlIdToGuid.end())
                    guid = itr->second;
                else m_sqlIdToGuid.insert(std::make_pair(spawn->guid, (guid = MAKE_NEW_GUID(sInstanceMgr.AllocateCreatureGuid(), spawn->guid.getEntry(), HIGHGUID_TYPE_GAMEOBJECT))));
            }

            // Mark our index
            _instance->ConstructGameObject(guid, allocation);
            m_gobjIndex[gobjIndex] = spawn->guid;

            // Finish loading our allocation
            allocation->Load(mapId, spawn->x, spawn->y, spawn->z, 0.f, spawn->rX, spawn->rY, spawn->rZ, spawn->rAngle, spawn);
            allocation->SetInstanceID(_instance->GetInstanceID());
            allocation->SetState(gameObjState);

            if(_instance->IsGameObjectPoolUpdating())
                _instance->AddObject(allocation);
            else allocation->PushToWorld(_instance);
            loadCount++;
        }
    }    
#else
    if(sp->CreatureSpawns.size())//got creatures
    {
        for(CreatureSpawnArray::iterator i=sp->CreatureSpawns.begin();i!=sp->CreatureSpawns.end();++i)
        {
            uint8 creatureState = 0;
            CreatureSpawn *spawn = *i;
            if(data && data->GetObjectState(spawn->guid, creatureState) && creatureState > 0)
                continue;

            WoWGuid guid = spawn->guid;
            if(_instance->IsInstance())
            {
                Loki::AssocVector<WoWGuid, WoWGuid>::iterator itr;
                if((itr = m_sqlIdToGuid.find(guid)) != m_sqlIdToGuid.end())
                    guid = itr->second;
                else m_sqlIdToGuid.insert(std::make_pair(spawn->guid, (guid = MAKE_NEW_GUID(sInstanceMgr.AllocateCreatureGuid(), spawn->guid.getEntry(), HIGHGUID_TYPE_UNIT))));
            }

            if(Creature *c = _instance->CreateCreature(guid))
            {
                c->Load(mapId, spawn->x, spawn->y, spawn->z, spawn->o, _instance->iInstanceMode, spawn);
                c->SetInstanceID(_instance->GetInstanceID());
                if(!c->CanAddToWorld())
                {
                    c->Destruct();
                    continue;
                }

                if(_instance->IsCreaturePoolUpdating())
                    _instance->AddObject(c);
                else c->PushToWorld(_instance);
                loadCount++;
            }
        }
    }

    if(sp->GameObjectSpawns.size())//got GOs
    {
        for(GameObjectSpawnArray::iterator i = sp->GameObjectSpawns.begin(); i != sp->GameObjectSpawns.end(); i++)
        {
            uint8 gameObjState = 0x00;
            GameObjectSpawn *spawn = *i;
            if(data == NULL || !data->GetObjectState(spawn->guid, gameObjState))
                gameObjState = spawn->state;

            WoWGuid guid = spawn->guid;
            if(_instance->IsInstance())
            {
                Loki::AssocVector<WoWGuid, WoWGuid>::iterator itr;
                if((itr = m_sqlIdToGuid.find(guid)) != m_sqlIdToGuid.end())
                    guid = itr->second;
                else m_sqlIdToGuid.insert(std::make_pair(spawn->guid, (guid = MAKE_NEW_GUID(sInstanceMgr.AllocateCreatureGuid(), spawn->guid.getEntry(), HIGHGUID_TYPE_GAMEOBJECT))));
            }

            if(GameObject *go = _instance->CreateGameObject(guid))
            {
                go->Load(mapId, spawn->x, spawn->y, spawn->z, 0.f, spawn->rX, spawn->rY, spawn->rZ, spawn->rAngle, spawn);
                go->SetInstanceID(_instance->GetInstanceID());
                go->SetState(gameObjState);

                if(_instance->IsGameObjectPoolUpdating())
                    _instance->AddObject(go);
                else go->PushToWorld(_instance);
                loadCount++;
            }
        }
    }
#endif
    return loadCount;
}

void MapCell::UnloadCellData(bool preDestruction)
{
    if(_loaded == false)
        return;

    _loaded = false;
    RWGuard guard(_objLock, true);
    m_activeNonPlayerSet.clear();
    m_deactivatedObjects.clear();
    m_gameObjectSet.clear();
    m_creatureSet.clear();
    m_sqlIdToGuid.clear();

#if STACKED_MEMORY_ALLOCATION == 1
    for(auto it = m_ctrIndex.begin(); it != m_ctrIndex.end(); ++it)
        _instance->RemoveObject(&_creatureStack[it->first]);
    for(auto it = m_gobjIndex.begin(); it != m_gobjIndex.end(); ++it)
        _instance->RemoveObject(&_gameobjectStack[it->first]);

    if(_creatureStack)
        delete [] _creatureStack;
    _creatureStack = NULL;
    if(_gameobjectStack)
        delete [] _gameobjectStack;
    _gameobjectStack = NULL;
    m_ctrIndex.clear();
    m_gobjIndex.clear();
#else
    std::for_each(m_activeNonPlayerSet.begin(), m_activeNonPlayerSet.end(), [this, preDestruction](std::pair<WoWGuid, WorldObject *> pair)
    {
        bool cleanupObj = true;
        if( !preDestruction && _unloadpending )
        {
            if(pair.second->GetHighGUID() == HIGHGUID_TYPE_TRANSPORTER)
                cleanupObj = false;

            if(pair.second->GetTypeId() == TYPEID_CORPSE && pair.second->GetUInt32Value(CORPSE_FIELD_OWNER) != 0)
                cleanupObj = false;
        }

        if(cleanupObj)
            pair.second->Cleanup();
    });

    std::for_each(m_deactivatedObjects.begin(), m_deactivatedObjects.end(), [this, preDestruction](std::pair<WoWGuid, WorldObject *> pair)
    {
        bool cleanupObj = true;
        if( !preDestruction && _unloadpending )
        {
            if(pair.second->GetHighGUID() == HIGHGUID_TYPE_TRANSPORTER)
                cleanupObj = false;

            if(pair.second->GetTypeId() == TYPEID_CORPSE && pair.second->GetUInt32Value(CORPSE_FIELD_OWNER) != 0)
                cleanupObj = false;
        }

        if(cleanupObj)
            pair.second->Cleanup();
    });

#endif

    // Start calldown for cell map unloading
    _mapData->CellUnloaded(_x, _y);
}

void MapCell::QueueUnloadPending()
{
    if(_unloadpending)
        return;

    _unloadpending = true;
    sLog.Debug("MapCell", "Queueing pending unload of cell %u %u", _x, _y);
}

void MapCell::CancelPendingUnload()
{
    if(!_unloadpending)
        return;

    _unloadpending = false;
    sLog.Debug("MapCell", "Cancelling pending unload of cell %u %u", _x, _y);
}

void MapCell::Unload()
{
    if(_active)
        return;

    sLog.Debug("MapCell", "Unloading cell %u %u", _x, _y);
    UnloadCellData(false);
    _unloadpending = false;
}
