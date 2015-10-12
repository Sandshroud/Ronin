/***
 * Demonstrike Core
 */

#pragma once

// sanity checking
enum AddItemResult : uint8
{
    ADD_ITEM_RESULT_ERROR           = 0x00,
    ADD_ITEM_RESULT_OK              = 0x01,
    ADD_ITEM_RESULT_SPLIT           = 0x02,
    ADD_ITEM_RESULT_ADDED_TO        = 0x04,
    ADD_ITEM_RESULT_DUPLICATED      = 0x08
};

struct SlotResult
{
    uint16 slot;
    AddItemResult result;

    void Init(AddItemResult res)
    {
        slot = 0xFFFF;
        result = res;
    }
    std::set<uint16> splitSlot;
};

#define MAKE_INVSLOT(bag, item) uint16((uint16(bag)<<8)|uint16(item))
#define INVSLOT_SET_ITEMSLOT(slot, itemslot) (slot&=~0xFF) |= itemslot
#define INVSLOT_BAG(slot) uint8(((slot&0xFF00)>>8))
#define INVSLOT_ITEM(slot) uint8(slot&0x00FF)

class Item;
class Container;
class Player;
class UpdateData;
class ByteBuffer;

class SERVER_DECL PlayerInventory
{
private:
    Player* m_pOwner;
    std::map<uint16, Item*> m_itemPtrs;
    std::map<WoWGuid, uint16> m_itemSlots;
    std::map<uint32, std::set<WoWGuid> > m_itemsByEntry;
    std::set<WoWGuid> m_conjuredItems;

private: // Item set handling
    std::set<uint32> m_skillItemSets;
    std::map<uint32, std::set<WoWGuid>> m_itemSets;

protected:
    SlotResult _addItem(Item* item, uint16 slot);
    Item *_removeItemBySlot(uint16 slot);
    bool _findFreeSlot(ItemPrototype *proto, uint16 &slot);
    bool _findBestSlot(ItemPrototype *proto, uint16 &slot);

public:
    PlayerInventory( Player* pPlayer );
    ~PlayerInventory();

    void LoadFromDB(QueryResult *result);
    void SaveToDB(bool newChar, QueryBuffer *buf);

    Player *GetOwner() { return m_pOwner; }

    void AddToWorld();
    void RemoveFromWorld(bool destroy);

    uint32 BuildCreateUpdateBlocks(ByteBuffer *data);
    void DestroyForPlayer(Player* plr);

    static bool IsBagSlot(uint8 slot);

    Item* GetInventoryItem(uint16 slot);
    Item* GetInventoryItem(WoWGuid guid);
    Item* GetInventoryItem(ItemData *data);
    uint16 GetInventorySlotByEntry(uint32 itemId);
    uint16 GetInventorySlotByGuid(WoWGuid guid);

    // Item deletion
    void DeleteItem(WoWGuid guid);
    void DeleteItem(Item *item);
    void DeleteItem(uint16 slot);

    // Interface hooks
    void CheckAreaItems();
    void RemoveConjuredItems();

    void EmptyBuyBack();

    // Find slot only needs proto but require an item pointer to avoid nasty calls
    bool FindFreeSlot(Item *item, uint16 &slot) { ASSERT(item); return _findFreeSlot(item->GetProto(), slot); }
    uint32 GetItemCount(uint32 entry) { if(m_itemsByEntry.find(entry) == m_itemsByEntry.end()) return 0; return m_itemsByEntry[entry].size(); }

    bool CreateQuestItems(Quest *qst);
    bool CreateQuestRewards(Quest *qst, uint8 reward_slot);
    bool CreateInventoryStacks(ItemPrototype *proto, uint32 count = 1, WoWGuid creatorGuid = 0, bool fromNPC = false);
    bool AddInventoryItemToSlot(Item *item, uint16 slot);
    bool DestroyInventoryItem(uint16 slot, ItemDeletionReason reason = ITEM_DELETION_USED);
    Item *RemoveInventoryItem(uint16 slot);
    Item *RemoveInventoryItem(WoWGuid guid);
    bool RemoveInventoryStacks(uint32 entry, uint32 count, bool force = true);

    void BuildInvError(uint8 error, Item *src, Item *dst, uint32 misc = 0);

    RONIN_INLINE bool IsValidSrcSlot(uint16 slot, bool bank)
    {
        if(m_itemPtrs.find(slot) == m_itemPtrs.end())
            return false;
        if(bank == false && slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_BAG_END)
            return false;
        if(INVSLOT_BAG(slot) == INVENTORY_SLOT_NONE)
        {

        }
        else
        {
            uint16 bagSlot = uint16(INVSLOT_BAG(slot));
            if(bagSlot < INVENTORY_SLOT_BAG_START || (bagSlot >= INVENTORY_SLOT_BAG_END && bagSlot < BANK_SLOT_BAG_START) || bagSlot >= BANK_SLOT_BAG_END)
                return false;
            bagSlot |= 0xFF00;
            if(m_itemPtrs.find(bagSlot) == m_itemPtrs.end())
                return false;
            Item *bag = m_itemPtrs.at(bagSlot);
            if(INVSLOT_ITEM(slot) >= bag->GetNumSlots())
                return false;
        }

        return true;
    }

    RONIN_INLINE bool IsValidDstSlot(Item *item, uint16 slot, bool bank)
    {
        if(bank == false && slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_BAG_END)
            return false;
        if(INVSLOT_BAG(slot) == INVENTORY_SLOT_NONE)
        {

        }
        else
        {
            uint16 bagSlot = uint16(INVSLOT_BAG(slot));
            if(bagSlot < INVENTORY_SLOT_BAG_START || (bagSlot >= INVENTORY_SLOT_BAG_END && bagSlot < BANK_SLOT_BAG_START) || bagSlot >= BANK_SLOT_BAG_END)
                return false;
            bagSlot |= 0xFF00; // Set the hipart to player inv
            if(m_itemPtrs.find(bagSlot) == m_itemPtrs.end())
                return false;
            Item *bag = m_itemPtrs.at(bagSlot);
            if(INVSLOT_ITEM(slot) >= bag->GetNumSlots())
                return false;
            if(bag->GetProto()->BagFamily && bag->GetProto()->BagFamily != item->GetProto()->BagFamily)
                return false;
        }

        return true;
    }

private:
    void SendItemPushResult(Item *item, uint16 invSlot, uint8 flags, uint32 totalCount);
    bool IsSlotValidForItem(ItemPrototype *proto, uint16 slot) { return true; }

    RONIN_INLINE bool IsValidSlot(Item *item, uint16 slot)
    {
        uint8 cBagSlot = INVSLOT_BAG(slot), cSlot = INVSLOT_ITEM(slot);
        if(cBagSlot == INVENTORY_SLOT_NONE)
        {
            if(cSlot >= INVENTORY_SLOT_MAX)
                return false;
            if(m_itemPtrs.find(slot) != m_itemPtrs.end())
                return false;
        }
        else
        {
            if(cBagSlot < INVENTORY_SLOT_BAG_START || cBagSlot >= INVENTORY_SLOT_BAG_END)
                return false;
            uint16 iBagSlot = MAKE_INVSLOT(0xFF, cBagSlot);
            if(m_itemPtrs.find(iBagSlot) == m_itemPtrs.end())
                return false;
            Item *item = m_itemPtrs.at(iBagSlot);
            if(cSlot >= item->GetNumSlots())
                return false;
            if(item->HasItem(cSlot))
                return false;
        }

        if(cBagSlot == INVENTORY_SLOT_NONE)
        {
            if(cSlot >= BANK_SLOT_ITEM_START && cSlot < BANK_SLOT_BAG_END)
                return false;
        } else if(cBagSlot >= BANK_SLOT_BAG_START && cBagSlot < BANK_SLOT_BAG_END)
            return false;
        return IsSlotValidForItem(item->GetProto(), slot);
    }
};
