/***
 * Demonstrike Core
 */

#pragma once

class AuctionHouse;
class AuctionMgr : public Singleton <AuctionMgr>
{
public:
    AuctionMgr()
    {
        loopcount = 0;
        auctionHighGuid = 1;
    }

    ~AuctionMgr()
    {
        map<uint32, AuctionHouse*>::iterator itr = auctionHouseMap.begin();
        for(; itr != auctionHouseMap.end(); itr++)
            delete itr->second;
    }

    void LoadAuctionHouses();
    void Update();

    AuctionHouse *GetAuctionHouse(uint32 Entry);

    uint32 GenerateAuctionId()
    {
        auctionGuidlock.Acquire();
        uint32 id = ++auctionHighGuid;
        auctionGuidlock.Release();
        return id;
    }

private:
    map<uint32, uint8> CreatureAuctionTypes;
    map<uint32, AuctionHouse*> auctionHouseMap;
    uint32 auctionHighGuid;
    Mutex auctionGuidlock;
    uint32 loopcount;
};

#define sAuctionMgr AuctionMgr::getSingleton()
