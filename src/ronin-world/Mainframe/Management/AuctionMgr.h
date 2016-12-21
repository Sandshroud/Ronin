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
        std::map<uint32, AuctionHouse*>::iterator itr = auctionHouseMap.begin();
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
    std::map<uint32, uint8> CreatureAuctionTypes;
    std::map<uint32, AuctionHouse*> auctionHouseMap;
    uint32 auctionHighGuid;
    Mutex auctionGuidlock;
    uint32 loopcount;
};

#define sAuctionMgr AuctionMgr::getSingleton()
