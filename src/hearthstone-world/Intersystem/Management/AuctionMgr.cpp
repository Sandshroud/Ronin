/***
 * Demonstrike Core
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

    for(ConstructDBCStorageIterator(AuctionHouseDBC) itr = dbcAuctionHouse.begin(); itr != dbcAuctionHouse.end(); ++itr)
        auctionHouseMap.insert(make_pair((*itr)->id, new AuctionHouse((*itr)->id)));

    res = WorldDatabase.Query("SELECT * FROM auctionhouse");
    if(res)
    {
        uint32 entry = 0;
        do
        {
            if(CreatureAuctionTypes.find(res->Fetch()[0].GetUInt32()) != CreatureAuctionTypes.end())
                continue;

            CreatureAuctionTypes.insert(make_pair(res->Fetch()[0].GetUInt32(), res->Fetch()[1].GetUInt8()));
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

    map<uint32, AuctionHouse*>::iterator itr = auctionHouseMap.begin();
    for(; itr != auctionHouseMap.end(); itr++)
    {
        itr->second->UpdateDeletionQueue();

        // Actual auction loop is on a seperate timer.
        if(!(loopcount % 1200))
            itr->second->UpdateAuctions();
    }
}
