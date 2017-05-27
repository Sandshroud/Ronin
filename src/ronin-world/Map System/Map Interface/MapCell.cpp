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

MapCell::MapCell()
{
    _forcedActive = false;
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
    if(obj->IsPlayer())
        m_playerSet[obj->GetGUID()] = obj;
    else
    {
        m_nonPlayerSet[obj->GetGUID()] = obj;
        if(obj->IsCreature())
            m_creatureSet[obj->GetGUID()] = obj;
        else if(obj->IsGameObject())
            m_gameObjectSet[obj->GetGUID()] = obj;
    }
}

void MapCell::RemoveObject(WorldObject* obj)
{
    m_playerSet.erase(obj->GetGUID());
    m_nonPlayerSet.erase(obj->GetGUID());
    m_creatureSet.erase(obj->GetGUID());
    m_gameObjectSet.erase(obj->GetGUID());
}

WorldObject *MapCell::FindObject(WoWGuid guid)
{
    MapCell::CellObjectMap::iterator itr;
    if((itr = m_playerSet.find(guid)) != m_playerSet.end() || (itr = m_nonPlayerSet.find(guid)) != m_nonPlayerSet.end())
        return itr->second;
    return NULL;
}

void MapCell::ProcessObjectSets(WorldObject *obj, ObjectProcessCallback *callback, uint32 objectMask)
{
    WorldObject *curObj;
    if(objectMask == 0)
    {
        for(MapCell::CellObjectMap::iterator itr = m_nonPlayerSet.begin(); itr != m_nonPlayerSet.end(); itr++)
            if((curObj = itr->second) && obj != curObj)
                (*callback)(obj, curObj);
        for(MapCell::CellObjectMap::iterator itr = m_playerSet.begin(); itr != m_playerSet.end(); itr++)
            if((curObj = itr->second) && obj != curObj)
                (*callback)(obj, curObj);
    }
    else
    {
        if(objectMask & TYPEMASK_TYPE_UNIT)
        {
            for(MapCell::CellObjectMap::iterator itr = m_creatureSet.begin(); itr != m_creatureSet.end(); itr++)
                if((curObj = itr->second) && obj != curObj)
                    (*callback)(obj, curObj);
        }

        if(objectMask & TYPEMASK_TYPE_PLAYER)
        {
            for(MapCell::CellObjectMap::iterator itr = m_playerSet.begin(); itr != m_playerSet.end(); itr++)
                if((curObj = itr->second) && obj != curObj)
                    (*callback)(obj, curObj);
        }

        if(objectMask & TYPEMASK_TYPE_GAMEOBJECT)
        {
            for(MapCell::CellObjectMap::iterator itr = m_gameObjectSet.begin(); itr != m_gameObjectSet.end(); itr++)
                if((curObj = itr->second) && obj != curObj)
                    (*callback)(obj, curObj);
        }
    }
}

void MapCell::SetActivity(bool state)
{
    uint32 x = _x/8, y = _y/8;
    if(state && _unloadpending)
        CancelPendingUnload();
    else if(!state && !_unloadpending)
        QueueUnloadPending();
    _active = state;
}

uint32 MapCell::LoadCellData(CellSpawns * sp)
{
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
    if(sp->CreatureSpawns.size())//got creatures
    {
        for(CreatureSpawnArray::iterator i=sp->CreatureSpawns.begin();i!=sp->CreatureSpawns.end();++i)
        {
            CreatureSpawn *spawn = *i;
            if(data && data->GetObjectState(spawn->guid))
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
            GameObjectSpawn *spawn = *i;
            uint8 objState = data ? data->GetObjectState(spawn->guid) : 0x00;

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
                go->SetState(objState|spawn->state);

                if(_instance->IsGameObjectPoolUpdating())
                    _instance->AddObject(go);
                else go->PushToWorld(_instance);
                loadCount++;
            }
        }
    }
    return loadCount;
}

void MapCell::UnloadCellData(bool preDestruction)
{
    if(_loaded == false)
        return;

    _loaded = false;
    std::set<WorldObject*> deletionSet;
    //This time it's simpler! We just remove everything :)
    for(MapCell::CellObjectMap::iterator itr = m_nonPlayerSet.begin(); itr != m_nonPlayerSet.end(); itr++)
    {
        WorldObject *obj = itr->second;
        if(obj == NULL)
            continue;

        if( !preDestruction && _unloadpending )
        {
            if(obj->GetHighGUID() == HIGHGUID_TYPE_TRANSPORTER)
                continue;

            if(obj->GetTypeId() == TYPEID_CORPSE && obj->GetUInt32Value(CORPSE_FIELD_OWNER) != 0)
                continue;
        }

        deletionSet.insert(obj);
    }
    m_nonPlayerSet.clear();

    while(deletionSet.size())
    {
        WorldObject *obj = *deletionSet.begin();
        deletionSet.erase(deletionSet.begin());
        obj->Cleanup();
    }

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
    sLog.Debug("MapCell", "Cancelling pending unload of cell %u %u", _x, _y);
    if(!_unloadpending)
        return;
}

void MapCell::Unload()
{
    if(_active)
        return;

    sLog.Debug("MapCell", "Unloading cell %u %u", _x, _y);
    UnloadCellData(false);
    _unloadpending = false;
}
