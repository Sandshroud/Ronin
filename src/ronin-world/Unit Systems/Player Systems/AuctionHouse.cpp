/***
 * Demonstrike Core
 */


#include "StdAfx.h"

void Auction::DeleteFromDB()
{
    CharacterDatabase.WaitExecute("DELETE FROM auctions WHERE auctionId = %u", Id);
}

void Auction::SaveToDB(uint32 AuctionHouseId)
{
    CharacterDatabase.Execute("INSERT INTO auctions VALUES(%u, %u, "I64FMTD", "I64FMTD", "I64FMTD", "I64FMTD", "I64FMTD", "I64FMTD", "I64FMTD")",
        Id, AuctionHouseId, m_item->itemGuid.raw(), owner.raw(), buyoutPrice, expirationTime, highestBidder, highestBid, depositAmount);
}

void Auction::UpdateInDB()
{
    CharacterDatabase.Execute("UPDATE auctions SET bidder = "I64FMTD" WHERE auctionId = %u", highestBidder, Id);
    CharacterDatabase.Execute("UPDATE auctions SET bid = "I64FMTD" WHERE auctionId = %u", highestBid, Id);
}

AuctionHouse::AuctionHouse(uint32 ID)
{
    dbc = dbcAuctionHouse.LookupEntry(ID);
    assert(dbc);

    cut_percent = float( float(dbc->tax) / 100.0f );
    deposit_percent = float( float(dbc->fee ) / 100.0f );
}

AuctionHouse::~AuctionHouse()
{
    for(RONIN_UNORDERED_MAP<uint32, Auction*>::iterator itr = auctions.begin(); itr != auctions.end(); itr++)
    {
        itr->second->m_item = NULL;
        delete itr->second;
    }
}

void AuctionHouse::QueueDeletion(Auction * auct, uint32 Reason)
{
    if(auct->Deleted)
        return;

    auct->Deleted = true;
    auct->DeletedReason = Reason;
    removalLock.Acquire();
    removalList.push_back(auct);
    removalLock.Release();
}

void AuctionHouse::UpdateDeletionQueue()
{
    removalLock.Acquire();
    Auction * auct;

    std::list<Auction*>::iterator it = removalList.begin();
    for(; it != removalList.end(); it++)
    {
        auct = *it;
        assert(auct->Deleted);
        RemoveAuction(auct);
    }

    removalList.clear();
    removalLock.Release();
}

void AuctionHouse::UpdateAuctions()
{
    auctionLock.AcquireReadLock();
    removalLock.Acquire();

    time_t now = UNIXTIME;
    RONIN_UNORDERED_MAP<uint32, Auction*>::iterator itr = auctions.begin();
    Auction * auct;
    for(; itr != auctions.end();)
    {
        auct = itr->second;
        ++itr;
        if(auct->expirationTime > now)
            continue;

        if(auct->highestBidder.empty())
            auct->DeletedReason = AUCTION_REMOVE_EXPIRED;
        else auct->DeletedReason = AUCTION_REMOVE_WON;

        auct->Deleted = true;
        removalList.push_back(auct);
    }

    removalLock.Release();
    auctionLock.ReleaseReadLock();
}

void AuctionHouse::AddAuction(Auction * auct)
{
    // add to the map
    auctionLock.AcquireWriteLock();
    auctions.insert( RONIN_UNORDERED_MAP<uint32, Auction*>::value_type( auct->Id , auct ) );
    auctionLock.ReleaseWriteLock();

    // add the item
    itemLock.AcquireWriteLock();
    auctionedItems.insert( RONIN_UNORDERED_MAP<WoWGuid, ItemData* >::value_type( auct->m_item->itemGuid, auct->m_item ) );
    itemLock.ReleaseWriteLock();

    sLog.Debug("AuctionHouse", "%u: Add auction %u, expire@ %u.", dbc->id, auct->Id, auct->expirationTime);
}

Auction * AuctionHouse::GetAuction(uint32 Id)
{
    Auction * ret;
    RONIN_UNORDERED_MAP<uint32, Auction*>::iterator itr;
    auctionLock.AcquireReadLock();
    itr = auctions.find(Id);
    ret = (itr == auctions.end()) ? 0 : itr->second;
    auctionLock.ReleaseReadLock();
    return ret;
}

void AuctionHouse::RemoveAuction(Auction * auct)
{
    sLog.Debug("AuctionHouse", "%u: Removing auction %u, reason %u.", dbc->id, auct->Id, auct->DeletedReason);

    char subject[100];
    char body[200];
    switch(auct->DeletedReason)
    {
    case AUCTION_REMOVE_EXPIRED:
        {
            // ItemEntry:0:3
            snprintf(subject, 100, "%u:0:3", (unsigned int)auct->m_item->itemGuid.getEntry());

            // Auction expired, resend item, no money to owner.
            sMailSystem.DeliverMessage(MAILTYPE_AUCTION, dbc->id, auct->owner, subject, "", 0, 0, auct->m_item->itemGuid, STATIONERY_AUCTION, true);
        }break;

    case AUCTION_REMOVE_WON:
        {
            // ItemEntry:0:1
            snprintf(subject, 100, "%u:0:1", (unsigned int)auct->m_item->itemGuid.getEntry());

            // <owner player guid>:bid:buyout
            snprintf(body, 200, "%X:%u:%u", auct->owner.getLow(), auct->highestBid, auct->buyoutPrice);

            // Auction won by highest bidder. He gets the item.
            sMailSystem.DeliverMessage(MAILTYPE_AUCTION, dbc->id, auct->highestBidder, subject, body, 0, 0, auct->m_item->itemGuid, STATIONERY_AUCTION, true);

            // Send a mail to the owner with his cut of the price.
            uint32 auction_cut = float2int32(float(cut_percent * float(auct->highestBid)));
            int32 amount = auct->highestBid - auction_cut + auct->depositAmount;
            if(amount < 0)
                amount = 0;

            // ItemEntry:0:2
            snprintf(subject, 100, "%u:0:2", (unsigned int)auct->m_item->itemGuid.getEntry());

            // <hex player guid>:bid:0:deposit:cut
            if(auct->highestBid == auct->buyoutPrice)      // Buyout
                snprintf(body, 200, "%X:%u:%u:%u:%u", (unsigned int)auct->highestBidder.getLow(), (unsigned int)auct->highestBid, (unsigned int)auct->buyoutPrice, (unsigned int)auct->depositAmount, (unsigned int)auction_cut);
            else snprintf(body, 200, "%X:%u:0:%u:%u", (unsigned int)auct->highestBidder.getLow(), (unsigned int)auct->highestBid, (unsigned int)auct->depositAmount, (unsigned int)auction_cut);

            // send message away.
            sMailSystem.DeliverMessage(MAILTYPE_AUCTION, dbc->id, auct->owner, subject, body, amount, 0, 0, STATIONERY_AUCTION, true);
        }break;
    case AUCTION_REMOVE_CANCELLED:
        {
            snprintf(subject, 100, "%u:0:5", (unsigned int)auct->m_item->itemGuid.getEntry());
            uint32 cut = uint32(float(cut_percent * auct->highestBid));
            Player* plr = objmgr.GetPlayer(auct->owner);
            if(cut && plr && plr->GetUInt32Value(PLAYER_FIELD_COINAGE) >= cut)
                plr->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -((int32)cut));

            sMailSystem.DeliverMessage(MAILTYPE_AUCTION, GetID(), auct->owner, subject, "", 0, 0, auct->m_item->itemGuid, STATIONERY_AUCTION, true);

            // return bidders money
            if(auct->highestBidder)
            {
                sMailSystem.DeliverMessage(MAILTYPE_AUCTION, GetID(), auct->highestBidder, subject, "", auct->highestBid, 0, 0, STATIONERY_AUCTION, true);
            }

        }break;
    }

    // Remove the auction from the hashmap.
    auctionLock.AcquireWriteLock();
    itemLock.AcquireWriteLock();

    auctions.erase(auct->Id);
    auctionedItems.erase(auct->m_item->itemGuid);

    auctionLock.ReleaseWriteLock();
    itemLock.ReleaseWriteLock();

    auct->m_item = NULL;

    // Finally destroy the auction instance.
    auct->DeleteFromDB();
    delete auct;
}

void WorldSession::HandleAuctionListBidderItems( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    uint32 unk1, unk2;
    recv_data >> guid >> unk1 >> unk2;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(guid);
    if(!pCreature || !pCreature->auctionHouse)
        return;

    pCreature->auctionHouse->SendBidListPacket(_player, &recv_data);
}

void Auction::AddToPacket(WorldPacket & data)
{
    data << Id;
    data << m_item->itemGuid.getEntry();

    for (uint8 i = 0; i < 10; i++)
    {
        data << m_item->enchantData[i]->enchantId;
        data << m_item->enchantData[i]->CalcTimeLeft();
        data << m_item->enchantData[i]->enchantCharges;
    }

    data << m_item->itemRandomProperty;
    data << m_item->itemRandomSeed;
    data << m_item->itemStackCount;  // Amount
    data << m_item->itemSpellCharges;
    data << uint32(0);              // Unknown
    data << owner;                  // Owner guid
    data << highestBid;             // Current prize
    data << uint64(50);             // Next bid value modifier, like current bid + this value
    data << buyoutPrice;            // Buyout
    data << uint32((expirationTime - UNIXTIME) * 1000); // Time left
    data << highestBidder;          // Last bidder
    data << uint64(highestBidder.empty() ? 0 : highestBid);
}

void AuctionHouse::SendBidListPacket(Player* plr, WorldPacket * packet)
{
    uint32 count = 0;

    WorldPacket data(SMSG_AUCTION_BIDDER_LIST_RESULT, 1024);
    data << uint32(0);                                        // Placeholder

    Auction * auct;
    auctionLock.AcquireReadLock();
    RONIN_UNORDERED_MAP<uint32, Auction*>::iterator itr = auctions.begin();
    for(; itr != auctions.end(); itr++)
    {
        auct = itr->second;
        if(plr->GetGUID() == auct->highestBidder)
        {
            if(auct->Deleted) continue;

            auct->AddToPacket(data);
            (*(uint32*)&data.contents()[0])++;
            ++count;
        }
    }
    data << count;
    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

void AuctionHouse::UpdateItemOwnerships(WoWGuid oldGuid, WoWGuid newGuid)
{

    Auction * auct;
    auctionLock.AcquireWriteLock();
    RONIN_UNORDERED_MAP<uint32, Auction*>::iterator itr = auctions.begin();
    for(; itr != auctions.end(); itr++)
    {
        auct = itr->second;
        if(auct->owner == oldGuid)
        {
            auct->owner = newGuid;
            // Don't save, we take care of this in char rename all at once. Less queries.
        }

        if(auct->highestBidder == oldGuid)
        {
            auct->highestBidder = newGuid;
            auct->UpdateInDB();
        }
    }
    auctionLock.ReleaseWriteLock();
}

void AuctionHouse::SendOwnerListPacket(Player* plr, WorldPacket * packet)
{
    uint32 count = 0;

    WorldPacket data(SMSG_AUCTION_OWNER_LIST_RESULT, 1024);
    data << uint32(0);                                        // Placeholder

    Auction * auct;
    auctionLock.AcquireReadLock();
    RONIN_UNORDERED_MAP<uint32, Auction*>::iterator itr = auctions.begin();
    for(; itr != auctions.end(); itr++)
    {
        auct = itr->second;
        if(auct->owner == plr->GetGUID())
        {
            if(auct->Deleted) continue;

            auct->AddToPacket(data);
            ++count;
        }
    }
    data.put<uint32>(0, count);
    data << count;
    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

void AuctionHouse::SendAuctionNotificationPacket(Player* plr, Auction * auct)
{
    WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 32);
    data << GetID();
    data << auct->Id;
    data << uint64(auct->highestBidder);
    data << uint32(0);
    data << uint32(0);
    data << auct->m_item->itemGuid.getEntry();
    data << uint32(0);

    plr->GetSession()->SendPacket(&data);
}

void WorldSession::HandleAuctionPlaceBid( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    uint64 guid;
    recv_data >> guid;

    uint32 auction_id, price;
    recv_data >> auction_id >> price;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(guid);
    if(!pCreature || !pCreature->auctionHouse || price == 0)
        return;

    // Find Item
    AuctionHouse * ah = pCreature->auctionHouse;
    Auction * auct = ah->GetAuction(auction_id);
    if(auct == 0 || !auct->owner || !_player || auct->owner == _player->GetGUID())
        return;

    if(auct->highestBid > price && price != auct->buyoutPrice)
        return;

    if(_player->GetUInt32Value(PLAYER_FIELD_COINAGE) < price)
        return;

    _player->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -((int32)price));
    if(!auct->highestBidder.empty())
    {
        // Return the money to the last highest bidder.
        char subject[100];
        snprintf(subject, 100, "%u:0:0", (int)auct->m_item->itemGuid.getEntry());
        sMailSystem.DeliverMessage(MAILTYPE_AUCTION, ah->GetID(), auct->highestBidder, subject, "", auct->highestBid, 0, 0, STATIONERY_AUCTION, true);

    }

    if(auct->buyoutPrice == price)
    {
        auct->highestBidder = _player->GetLowGUID();
        auct->highestBid = price;

        // we used buyout on the item.
        ah->QueueDeletion(auct, AUCTION_REMOVE_WON);

        // send response packet
        WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 12);
        data << auct->Id << uint32(AUCTION_BID) << uint32(0);
        SendPacket(&data);
    }
    else
    {
        // update most recent bid
        auct->highestBidder = _player->GetLowGUID();
        auct->highestBid = price;
        auct->UpdateInDB();

        // send response packet
        WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 12);
        data << auct->Id << uint32(AUCTION_BID) << uint32(0);
        SendPacket(&data);
    }
}

void WorldSession::HandleCancelAuction( WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint64 guid;
    recv_data >> guid;

    uint32 auction_id;
    recv_data >> auction_id;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(guid);
    if(!pCreature || !pCreature->auctionHouse)
        return;

    // Find Item
    Auction * auct = pCreature->auctionHouse->GetAuction(auction_id);
    if(auct == 0) return;

    pCreature->auctionHouse->QueueDeletion(auct, AUCTION_REMOVE_CANCELLED);

    // Send response packet.
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 8);
    data << auction_id << uint32(AUCTION_CANCEL) << uint32(0);
    SendPacket(&data);

    // Re-send the owner list.
    pCreature->auctionHouse->SendOwnerListPacket(_player, 0);
}

void WorldSession::HandleAuctionSellItem( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    uint64 guid, item;
    uint32 unk, count, bid, buyout, etime;  // etime is in minutes

    recv_data >> guid >> unk >> item >> count;
    recv_data >> bid >> buyout >> etime;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(guid);
    if(  !pCreature || !pCreature->auctionHouse )
        return;     // NPC doesnt exist or isnt an auctioneer

    // Get item
    Item* pItem = _player->GetInventory()->GetInventoryItem(item);
    if( !pItem || pItem->isSoulBound() || pItem->isAccountBound() || pItem->HasFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_CONJURED) )
    {
        WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 8);
        data << uint32(0);
        data << uint32(AUCTION_CREATE);
        data << uint32(AUCTION_ERROR_ITEM);
        SendPacket(&data);
        return;
    }

    AuctionHouse * ah = pCreature->auctionHouse;

    uint32 item_worth = pItem->GetProto()->SellPrice * pItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
    uint32 item_deposit = (uint32)(item_worth * ah->deposit_percent) * (uint32)(etime / 240.0f); // deposit is per 4 hours

    if (_player->GetUInt32Value(PLAYER_FIELD_COINAGE) < item_deposit)   // player cannot afford deposit
    {
        WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 8);
        data << uint32(0);
        data << uint32(AUCTION_CREATE);
        data << uint32(AUCTION_ERROR_MONEY);
        SendPacket(&data);
        return;
    }

    // TODO: AUCTION ITEMS
    {
        WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 8);
        data << uint32(0);
        data << uint32(AUCTION_CREATE);
        data << uint32(AUCTION_ERROR_ITEM);
        SendPacket(&data);
        return;
    }

    // Create auction
    Auction * auct = new Auction;
    auct->buyoutPrice = buyout;
    auct->expirationTime = (uint32)UNIXTIME + (etime * 60);
    auct->highestBid = bid;
    auct->highestBidder = 0;    // hm
    auct->Id = sAuctionMgr.GenerateAuctionId();
    auct->owner = _player->GetLowGUID();
    auct->m_item = sItemMgr.GetItemData(pItem->GetGUID());
    auct->Deleted = false;
    auct->DeletedReason = 0;
    auct->depositAmount = item_deposit;

    // remove deposit
    _player->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -(int32)item_deposit);

    // Add and save auction to DB
    ah->AddAuction(auct);
    auct->SaveToDB(ah->GetID());

    // Send result packet
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 8);
    data << auct->Id;
    data << uint32(AUCTION_CREATE);
    data << uint32(AUCTION_ERROR_NONE);
    SendPacket(&data);

    // Re-send the owner list.
    pCreature->auctionHouse->SendOwnerListPacket(_player, 0);
}

void WorldSession::HandleAuctionListOwnerItems( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    uint64 guid;
    uint32 unk;
    recv_data >> guid >> unk;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(guid);
    if(!pCreature || !pCreature->auctionHouse)
        return;

    pCreature->auctionHouse->SendOwnerListPacket(_player, &recv_data);
}

void AuctionHouse::SendAuctionList(Player* plr, WorldPacket * packet)
{
    uint32 start_index, current_index = 0;
    uint32 counted_items = 0;
    std::string auctionstring;
    uint8 levelRange1, levelRange2, usableCheck;
    int32 inventory_type, itemclass, itemsubclass, rarityCheck;

    *packet >> start_index;
    *packet >> auctionstring;
    *packet >> levelRange1 >> levelRange2;
    *packet >> inventory_type >> itemclass >> itemsubclass;
    *packet >> rarityCheck >> usableCheck;

    // convert auction string to lowercase for faster parsing.
	RONIN_UTIL::TOLOWER(auctionstring);

    WorldPacket data(SMSG_AUCTION_LIST_RESULT, 7000);
    data << uint32(0);

    auctionLock.AcquireReadLock();
    RONIN_UNORDERED_MAP<uint32, Auction*>::iterator itr = auctions.begin();
    for(; itr != auctions.end(); itr++)
    {
        if(itr->second->Deleted)
            continue;
        ItemPrototype *proto = itr->second->m_item->proto;

        // Check the auction for parameters

        // inventory type
        if(inventory_type != -1 && inventory_type != (int32)proto->InventoryType)
            continue;

        // class
        if(itemclass != -1 && itemclass != (int32)proto->Class)
            continue;

        // subclass
        if(itemsubclass != -1 && itemsubclass != (int32)proto->SubClass)
            continue;

        // this is going to hurt. - name
        if(auctionstring.length() > 0 && !RONIN_UTIL::FindXinYString(auctionstring, RONIN_UTIL::TOLOWER_RETURN(proto->Name1)))
            continue;

        // rarity
        if(rarityCheck != -1 && rarityCheck > (int32)proto->Quality)
            continue;

        // level range check - lower boundary
        if(levelRange1 && proto->RequiredLevel < levelRange1)
            continue;

        // level range check - high boundary
        if(levelRange2 && proto->RequiredLevel > levelRange2)
            continue;

        // usable check - this will hurt too :(
        if(usableCheck)
        {
            // allowed class
            if(proto->AllowableClass > 0 && !(plr->getClassMask() & (uint32)proto->AllowableClass))
                continue;

            if(proto->RequiredLevel > 0 && (uint32)proto->RequiredLevel > plr->getLevel())
                continue;

            if(proto->AllowableRace > 0 && !(plr->getRaceMask() & (uint32)proto->AllowableRace))
                continue;

            if(proto->Class == 4 && proto->SubClass && !(plr->GetArmorProficiency()&(((uint32)(1))<<proto->SubClass)))
                continue;

            if(proto->Class == 2 && proto->SubClass && !(plr->GetWeaponProficiency()&(((uint32)(1))<<proto->SubClass)))
                continue;

            if(proto->RequiredSkill > 0 && (!plr->_HasSkillLine(proto->RequiredSkill) || (uint32)proto->RequiredSkillRank > plr->_GetSkillLineCurrent(proto->RequiredSkill, true)))
                continue;
        }

        // Page system.
        ++counted_items;
        if(counted_items >= start_index + 50)
            continue;
        current_index++;
        if(start_index && current_index < start_index) continue;

        // all checks passed -> add to packet.
        itr->second->AddToPacket(data);
        (*(uint32*)&data.contents()[0])++;
    }

    // total count
    data << uint32(1 + counted_items);
    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

void WorldSession::HandleAuctionListItems( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(guid);
    if(!pCreature || !pCreature->auctionHouse)
        return;

    pCreature->auctionHouse->SendAuctionList(_player, &recv_data);
}

void AuctionHouse::LoadAuctions()
{
    QueryResult *result = CharacterDatabase.Query("SELECT * FROM auctions WHERE auctionhouse =%u", GetID());

    if( !result )
        return;

    Auction * auct;
    Field * fields;

    do
    {
        fields = result->Fetch();
        auct = new Auction;
        auct->Id = fields[0].GetUInt32();

        ItemData *pData = sItemMgr.GetItemData(fields[2].GetUInt64());
        if(pData == NULL)
        {
            printf("Deleting auction for invalid item %u (%u)\n", auct->Id, fields[2].GetUInt32());
            CharacterDatabase.Execute("DELETE FROM auctions WHERE auctionId=%u", auct->Id);
            delete auct;
            continue;
        }

        auct->m_item = pData;
        auct->owner = fields[3].GetUInt64();
        auct->buyoutPrice = fields[4].GetUInt64();
        auct->expirationTime = fields[5].GetUInt64();
        auct->highestBidder = fields[6].GetUInt64();
        auct->highestBid = fields[7].GetUInt64();
        auct->depositAmount = fields[8].GetUInt64();
        auct->DeletedReason = 0;
        auct->Deleted = false;

        auctions.insert( std::make_pair( auct->Id, auct ) );
    } while (result->NextRow());
    delete result;
}
