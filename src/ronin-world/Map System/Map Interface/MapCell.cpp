/***
 * Demonstrike Core
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
    _objects.clear();
    _players.clear();
}

void MapCell::AddObject(WorldObject* obj)
{
    if(obj->IsPlayer())
        _players.push_back(obj);

    _objects.push_back(obj);
}

void MapCell::RemoveObject(WorldObject* obj)
{
    CellObjectSet::iterator itr;
    if(obj->IsPlayer() && (itr = std::find(_players.begin(), _players.end(), obj)) != _players.end())
        _players.erase(itr);

    if((itr = std::find(_objects.begin(), _objects.end(), obj)) != _objects.end())
        _objects.erase(itr);
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
            if(Creature *c = _instance->CreateCreature(spawn->entry))
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

    if(sp->GOSpawns.size())//got GOs
    {
        for(GOSpawnList::iterator i = sp->GOSpawns.begin(); i != sp->GOSpawns.end(); i++)
        {
            GOSpawn *spawn = *i;
            if(GameObject *go = _instance->CreateGameObject(spawn->entry))
            {
                if(!go->Load(mapId, spawn))
                {
                    go->Destruct();
                    continue;
                }

                go->PushToWorld(_instance);
                loadCount++;
                TRIGGER_GO_EVENT(go, OnSpawn);
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
    if(_objects.size())
    {
        //This time it's simpler! We just remove everything :)
        CellObjectSet set(_objects);
        for(CellObjectSet::iterator itr = set.begin(); itr != set.end(); itr++)
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

            obj->Destruct();
        }
        _objects.clear();
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
