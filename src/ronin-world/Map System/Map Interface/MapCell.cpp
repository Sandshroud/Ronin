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
}

void MapCell::AddObject(WorldObject* obj)
{
    if(obj->IsPlayer())
    {
        m_objectSet.push_back(obj);
        m_playerSet.push_back(obj);
        return;
    }

    MapCellObjectStorage *objectStorage;
    Loki::AssocVector<uint8, MapCellObjectStorage*>::iterator iter;
    if(obj->GetPhaseMask() & 0x8000)
        m_objectSet.push_back(obj);
    else
    {
        uint8 phaseBit;
        uint16 phaseMask = obj->GetPhaseMask();
        while((phaseBit = RONIN_UTIL::FirstBitValue<uint16>(phaseMask)) != 0xFF)
        {
            phaseMask &= ~(1<<phaseBit);
            if((iter = m_phaseStorage.find(phaseBit)) == m_phaseStorage.end())
            {
                objectStorage = new MapCellObjectStorage(phaseBit);
                m_phaseStorage.insert(std::make_pair(phaseBit, objectStorage));
            } else objectStorage = iter->second;

            objectStorage->AddObject(obj);
        }
    }

    if(uint32 eventId = obj->getEventID())
    {
        if((iter = m_eventStorage.find(eventId)) == m_eventStorage.end())
        {
            objectStorage = new MapCellObjectStorage(eventId);
            m_eventStorage.insert(std::make_pair(eventId, objectStorage));
        } else objectStorage = iter->second;

        objectStorage->AddObject(obj);
    }
}

void MapCellObjectStorage::AddObject(WorldObject *obj)
{
    m_objectSet.push_back(obj);
}

void MapCell::RemoveObject(WorldObject* obj)
{
    MapCell::CellObjectSet::iterator itr;
    if(obj->IsPlayer())
    {
        if((itr = std::find(m_objectSet.begin(), m_objectSet.end(), obj)) != m_objectSet.end())
            m_objectSet.erase(itr);
        if((itr = std::find(m_playerSet.begin(), m_playerSet.end(), obj)) != m_playerSet.end())
            m_playerSet.erase(itr);
        return;
    }

    Loki::AssocVector<uint8, MapCellObjectStorage*>::iterator iter;
    if(obj->GetPhaseMask() & 0x8000)
    {
        if((itr = std::find(m_objectSet.begin(), m_objectSet.end(), obj)) != m_objectSet.end())
            m_objectSet.erase(itr);
    }
    else
    {
        uint8 phaseBit;
        uint16 phaseMask = obj->GetPhaseMask();
        while((phaseBit = RONIN_UTIL::FirstBitValue<uint16>(phaseMask)) != 0xFF)
        {
            phaseMask &= ~(1<<phaseBit);
            if((iter = m_phaseStorage.find(phaseBit)) != m_phaseStorage.end())
                iter->second->RemoveObject(obj);
        }
    }

    if(uint32 eventId = obj->getEventID())
        if((iter = m_eventStorage.find(eventId)) != m_eventStorage.end())
            iter->second->RemoveObject(obj);
}

bool MapCell::HasPlayers(uint16 phaseMask)
{
    return !m_playerSet.empty();
}

void MapCellObjectStorage::RemoveObject(WorldObject *obj)
{
    MapCell::CellObjectSet::iterator itr;
    if((itr = std::find(m_objectSet.begin(), m_objectSet.end(), obj)) != m_objectSet.end())
        m_objectSet.erase(itr);
}

MapCell::CellObjectSet *MapCell::GetNextObjectSet(uint16 &phaseMask, std::vector<uint32> &eventAccess, bool &handledAllPhases)
{
    // Check the full phase mask to see what's in it
    if(handledAllPhases == false && !m_objectSet.empty())
    {
        handledAllPhases = true;
        return &m_objectSet;
    }

    Loki::AssocVector<uint8, MapCellObjectStorage*>::iterator iter;
    // Check active events or event access to see what we have here
    while(!m_eventStorage.empty() && !eventAccess.empty())
    {
        uint32 eventId = *eventAccess.begin();
        eventAccess.erase(eventAccess.begin());
        if((iter = m_eventStorage.find(eventId)) != m_eventStorage.end())
            return iter->second->GetObjectSet();
    }

    // We're parsing based on bits directly, not on masked 32bit values, if we're capped return false here
    uint8 phaseBit; MapCellObjectStorage *phaseStorage = NULL;
    while(true)
    {
        phaseBit = RONIN_UTIL::FirstBitValue<uint16>(phaseMask);
        if(phaseBit == 0xFF)
            return false;

        phaseMask &= ~(1<<phaseBit);
        if((iter = m_phaseStorage.find(phaseBit)) == m_phaseStorage.end() || iter->second->isEmpty())
            continue;
        return iter->second->GetObjectSet();
    }
    return NULL;
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
    for(Loki::AssocVector<uint8, MapCellObjectStorage*>::iterator itr = m_phaseStorage.begin(); itr != m_phaseStorage.end(); itr++)
    {
        itr->second->UnloadCellData(_unloadpending, preDestruction);
        if( !preDestruction && _unloadpending )
            continue;
        delete itr->second;
        itr->second = NULL;
    }

    if( preDestruction || !_unloadpending )
        m_phaseStorage.clear();

    if(m_objectSet.size())
    {
        //This time it's simpler! We just remove everything :)
        MapCell::CellObjectSet set(m_objectSet);
        for(MapCell::CellObjectSet::iterator itr = set.begin(); itr != set.end(); itr++)
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
        m_objectSet.clear();
    }

    // Start calldown for cell map unloading
    _mapData->CellUnloaded(_x, _y);
}

void MapCellObjectStorage::UnloadCellData(bool pendingUnload, bool preDestruction)
{
    if(m_objectSet.size())
    {
        //This time it's simpler! We just remove everything :)
        MapCell::CellObjectSet set(m_objectSet);
        for(MapCell::CellObjectSet::iterator itr = set.begin(); itr != set.end(); itr++)
        {
            WorldObject *obj = (*itr);
            if(obj == NULL)
                continue;

            if( !preDestruction && pendingUnload )
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
        m_objectSet.clear();
    }
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
