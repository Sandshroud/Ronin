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
        m_playerSet.push_back(obj);
    else m_objectSet.push_back(obj);
}

void MapCell::RemoveObject(WorldObject* obj)
{
    MapCell::CellObjectSet::iterator itr;
    if(obj->IsPlayer() && (itr = std::find(m_playerSet.begin(), m_playerSet.end(), obj)) != m_playerSet.end())
        m_playerSet.erase(itr);
    else if((itr = std::find(m_objectSet.begin(), m_objectSet.end(), obj)) != m_objectSet.end())
        m_objectSet.erase(itr);
}

bool MapCell::HasPlayers(uint16 phaseMask)
{
    return !m_playerSet.empty();
}

void MapCell::ProcessObjectSets(WorldObject *obj, ObjectProcessCallback *callback)
{
    WorldObject *curObj;
    for(MapCell::CellObjectSet::iterator itr = m_objectSet.begin(); itr != m_objectSet.end(); itr++)
        if((curObj = *itr) && obj != curObj)
            (*callback)(obj, curObj);
    for(MapCell::CellObjectSet::iterator itr = m_playerSet.begin(); itr != m_playerSet.end(); itr++)
        if((curObj = *itr) && obj != curObj)
            (*callback)(obj, curObj);
}

void MapCell::ProcessSetRemovals(WorldObject *obj, ObjectRemovalCallback *callback, bool forced)
{
    WorldObject *curObj;
    for(MapCell::CellObjectSet::iterator itr = m_objectSet.begin(); itr != m_objectSet.end(); itr++)
        if((curObj = *itr) && obj != curObj)
            (*callback)(obj, curObj->GetGUID(), forced);
    for(MapCell::CellObjectSet::iterator itr = m_playerSet.begin(); itr != m_playerSet.end(); itr++)
        if((curObj = *itr) && obj != curObj)
            (*callback)(obj, curObj->GetGUID(), forced);
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
    //MapInstance *pInstance = NULL;//_instance->IsInstance() ? castPtr<InstanceMgr>(_instance) : NULL;
    if(sp->CreatureSpawns.size())//got creatures
    {
        for(CreatureSpawnList::iterator i=sp->CreatureSpawns.begin();i!=sp->CreatureSpawns.end();++i)
        {
            CreatureSpawn *spawn = *i;
            if(Creature *c = _instance->CreateCreature(spawn->guid))
            {
                c->Load(mapId, spawn->x, spawn->y, spawn->z, spawn->o, _instance->iInstanceMode, spawn);
                c->SetInstanceID(_instance->GetInstanceID());
                if(!c->CanAddToWorld())
                {
                    c->Destruct();
                    continue;
                }

                c->PushToWorld(_instance);
                loadCount++;
            }
        }
    }

    if(sp->GameObjectSpawns.size())//got GOs
    {
        for(GameObjectSpawnList::iterator i = sp->GameObjectSpawns.begin(); i != sp->GameObjectSpawns.end(); i++)
        {
            GameObjectSpawn *spawn = *i;
            if(GameObject *go = _instance->CreateGameObject(spawn->guid))
            {
                go->Load(mapId, spawn->x, spawn->y, spawn->z, 0.f, spawn->rX, spawn->rY, spawn->rZ, spawn->rAngle, spawn);
                go->SetInstanceID(_instance->GetInstanceID());

                go->PushToWorld(_instance);
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
    for(MapCell::CellObjectSet::iterator itr = m_objectSet.begin(); itr != m_objectSet.end(); itr++)
    {
        WorldObject *obj = (*itr);
        if(obj == NULL)
            continue;

        if( !preDestruction && _unloadpending )
        {
            if(obj->GetHighGUID() == HIGHGUID_TYPE_TRANSPORTER)
                continue;

            if(obj->GetTypeId() == TYPEID_CORPSE && obj->GetUInt32Value(CORPSE_FIELD_OWNER) != 0)
                continue;
        }

        if( obj->IsInWorld())
            obj->RemoveFromWorld();

        deletionSet.insert(obj);
    }
    m_objectSet.clear();

    while(deletionSet.size())
    {
        WorldObject *obj = *deletionSet.begin();
        deletionSet.erase(deletionSet.begin());
        obj->Destruct();
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
