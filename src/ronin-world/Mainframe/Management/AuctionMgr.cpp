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

initialiseSingleton( AuctionMgr );

void AuctionMgr::LoadAuctionHouses()
{
    sLog.Notice("AuctionMgr", "Loading Auction Houses...");

    QueryResult* res = CharacterDatabase.Query("SELECT MAX(auctionId) FROM auctions");
    if(res)
    {
        auctionHighGuid = res->Fetch()[0].GetUInt32();
        delete res;
    }

    for(uint32 i = 0; i < dbcAuctionHouse.GetNumRows(); i++)
        if(AuctionHouseEntry *entry = dbcAuctionHouse.LookupRow(i))
            auctionHouseMap.insert(std::make_pair(entry->id, new AuctionHouse(entry->id)));

    res = WorldDatabase.Query("SELECT * FROM auctionhouse");
    if(res)
    {
        uint32 entry = 0;
        do
        {
            if(CreatureAuctionTypes.find(res->Fetch()[0].GetUInt32()) != CreatureAuctionTypes.end())
                continue;

            CreatureAuctionTypes.insert(std::make_pair(res->Fetch()[0].GetUInt32(), res->Fetch()[1].GetUInt8()));
        }while(res->NextRow());
        delete res;
    }
}

AuctionHouse * AuctionMgr::GetAuctionHouse(uint32 Entry)
{
    if(CreatureAuctionTypes.find(Entry) == CreatureAuctionTypes.end())
        return NULL;
    uint8 AHType = CreatureAuctionTypes.at(Entry);
    if(auctionHouseMap.find(AHType) != auctionHouseMap.end())
        return auctionHouseMap.at(AHType);
    return NULL;
}

void AuctionMgr::Update()
{
    if((++loopcount % 100))
        return;

    std::map<uint32, AuctionHouse*>::iterator itr = auctionHouseMap.begin();
    for(; itr != auctionHouseMap.end(); itr++)
    {
        itr->second->UpdateDeletionQueue();

        // Actual auction loop is on a seperate timer.
        if(!(loopcount % 1200))
            itr->second->UpdateAuctions();
    }
}
