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

initialiseSingleton( TransportMgr );

TransportMgr::TransportMgr()
{

}

TransportMgr::~TransportMgr()
{

}

void TransportMgr::LoadTransportData()
{
    QueryResult *result = WorldDatabase.Query("SELECT entry, data0 FROM gameobject_names WHERE type = 15");
    if(result == NULL)
        return; // No usable transport data in gameobject names

    uint32 creationCount = 0;
    do
    {
        GameObjectInfo *info = NULL;
        uint32 objectInfoEntry = result->Fetch()[0].GetUInt32();
        if((info = GameObjectNameStorage.LookupEntry(objectInfoEntry)) == NULL)
            continue; // Not possible but check anyway

        TaxiPath *transportPath = NULL;
        uint32 taxiPath = result->Fetch()[1].GetUInt32();
        if((transportPath = sTaxiMgr.GetTaxiPath(taxiPath)) == NULL)
            continue; // Possible as we might have deprecated transports

        // Create our transport data
        _CreateTransportData(info, transportPath);
        ++creationCount;
    }while(result->NextRow());
    delete result;
    result = NULL;

    sLog.Notice("TransportMgr", "%u transports loaded from database", creationCount);
}

void TransportMgr::ProcessTransports(uint32 msTime)
{

}

void TransportMgr::PreloadMapInstance(uint32 msTime, MapInstance *instance, uint32 mapId)
{
    for(auto itr = m_transportDataStorage.begin(); itr != m_transportDataStorage.end(); ++itr)
    {
        TransportData *transData = itr->second;
        if(mapId != transData->mapIds[0] && mapId != transData->mapIds[1])
            continue;

        TaxiPathNodeEntry* taxiPoint = transData->transportPath->GetPathNode(transData->transportPath->GetStartNode(mapId));
        if(taxiPoint == NULL)
            continue;

        GameObject *gobj = new GameObject();
        gobj->Construct(transData->transportTemplate, MAKE_NEW_GUID(transData->transportTemplate->ID, transData->transportTemplate->ID, HIGHGUID_TYPE_TRANSPORTER));

        // Finish loading our allocation
        gobj->Load(mapId, taxiPoint->LocX, taxiPoint->LocY, taxiPoint->LocZ);
        gobj->SetInstanceID(instance->GetInstanceID());

        // Push to our map instance
        if(instance->IsGameObjectPoolUpdating())
            instance->AddObject(gobj);
        else gobj->PushToWorld(instance, msTime);

        // Second map, deactivate till we're in timer
        if(mapId != transData->mapIds[0])
            gobj->Deactivate(0);
    }
}

bool TransportMgr::RegisterTransport(GameObject *gobj, uint32 mapId)
{
    if(gobj->GetGUID().getHigh() == HIGHGUID_TYPE_TRANSPORTER)
        return true; // YOLOLOLOLO
    return false;
}

void TransportMgr::ProcessingPendingEvents(MapInstance *instance)
{

}

bool TransportMgr::CheckTransportPosition(WoWGuid transport, uint32 mapId)
{
    return false;
}

void TransportMgr::UpdateTransportData(Player *plr)
{

}

void TransportMgr::ClearPlayerData(Player *plr)
{

}

void TransportMgr::_CreateTransportData(GameObjectInfo *info, TaxiPath *path)
{
    TransportData *data = new TransportData();
    data->transportTemplate = info;
    data->transportPath = path;
    for(uint8 i = 0; i < 2; ++i)
        data->mapIds[i] = path->mapData[i].mapId;

    m_transportDataStorage.insert(std::make_pair(info->ID, data));
}