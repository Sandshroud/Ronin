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

void TransportMgr::ParseDBC()
{
    for(uint32 j = 0; j < dbcTaxiPathNode.GetNumRows(); j++)
    {
        /*DBCTaxiPathNode *pathnode = dbcTaxiPathNode.LookupRow(j);
        if(pathnode->path == PathID)
        {
            Path[i].mapid       = pathnode->mapid;
            Path[i].x           = pathnode->x;
            Path[i].y           = pathnode->y;
            Path[i].z           = pathnode->z;
            Path[i].actionFlag  = pathnode->flag;
            Path[i].delay       = pathnode->waittime;
            ++i;
        }*/
    }

}

void TransportMgr::ProcessTransports(uint32 msTime)
{

}

void TransportMgr::PreloadMapInstance(MapInstance *instance, uint32 mapId)
{

}

bool TransportMgr::RegisterTransport(GameObject *gobj, uint32 mapId)
{
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
