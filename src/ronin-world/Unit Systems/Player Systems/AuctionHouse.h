/***
 * Demonstrike Core
 */

#pragma once

enum AuctionRemoveType
{
    AUCTION_REMOVE_EXPIRED,
    AUCTION_REMOVE_WON,
    AUCTION_REMOVE_CANCELLED,
};
enum AUCTIONRESULT
{
    AUCTION_CREATE,
    AUCTION_CANCEL,
    AUCTION_BID,
    AUCTION_BUYOUT,
};
enum AUCTIONRESULTERROR
{
    AUCTION_ERROR_NONE,
    AUCTION_ERROR_UNK1,
    AUCTION_ERROR_INTERNAL,
    AUCTION_ERROR_MONEY,
    AUCTION_ERROR_ITEM,
};
enum AuctionMailResult
{
    AUCTION_OUTBID,
    AUCTION_WON,
    AUCTION_SOLD,
    AUCTION_EXPIRED,
    AUCTION_EXPIRED2,
    AUCTION_CANCELLED,
};

struct Auction
{
    uint32 Id;
    WoWGuid owner;
    WoWGuid highestBidder;
    uint64 highestBid;
    uint64 buyoutPrice;
    uint64 depositAmount;
    uint64 expirationTime;
    Item* m_item;

    void DeleteFromDB();
    void SaveToDB(uint32 AuctionHouseId);
    void UpdateInDB();
    void AddToPacket(WorldPacket & data);
    bool Deleted;
    uint32 DeletedReason;
};

class AuctionHouse
{
public:
    AuctionHouse(uint32 ID);
    ~AuctionHouse();

    RONIN_INLINE uint32 GetID() { return dbc->id; }
    void LoadAuctions();

    void UpdateAuctions();
    void UpdateDeletionQueue();

    void RemoveAuction(Auction * auct);
    void AddAuction(Auction * auct);
    Auction * GetAuction(uint32 Id);
    void QueueDeletion(Auction * auct, uint32 Reason);

    void SendAuctionHello(WoWGuid guid, Player *plr);
    void SendOwnerListPacket(Player* plr, WorldPacket * packet);
    void SendBidListPacket(Player* plr, WorldPacket * packet);
    void SendAuctionNotificationPacket(Player* plr, Auction * auct);
    void SendAuctionList(Player* plr, WorldPacket * packet);

    void UpdateItemOwnerships(WoWGuid oldGuid, WoWGuid newGuid);

private:
    RWLock itemLock;
    std::map<WoWGuid, Item* > auctionedItems;

    RWLock auctionLock;
    std::map<uint32, Auction*> auctions;

    Mutex removalLock;
    std::list<Auction*> removalList;

    AuctionHouseEntry * dbc;

public:
    float cut_percent;
    float deposit_percent;
};
