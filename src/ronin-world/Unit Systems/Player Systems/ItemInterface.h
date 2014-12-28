/***
 * Demonstrike Core
 */

#pragma once

#define INVALID_BACKPACK_SLOT ((int8)(0xFF)) //In 1.8 client marked wrong slot like this

struct SlotResult
{
    SlotResult() { ContainerSlot = -1, Slot = -1, Result = false; }
    int16 ContainerSlot;
    int16 Slot;
    bool Result;
};

class Item;
class Container;
class Player;
class UpdateData;
class ByteBuffer;

// sanity checking
enum AddItemResult
{
    ADD_ITEM_RESULT_ERROR           = 0,
    ADD_ITEM_RESULT_OK              = 1,
    ADD_ITEM_RESULT_DUPLICATED      = 2,
};

class SERVER_DECL ItemInterface
{
private:
    Player* m_pOwner;
    std::map<uint8, Item*> m_pItems;

    AddItemResult m_AddItem(Item* item, int16 ContainerSlot, int16 slot);

public:
    friend class ItemIterator;
    ItemInterface( Player* pPlayer );
    ~ItemInterface();

    Player* GetOwner() { return m_pOwner; }
    bool IsBagSlot(int16 slot);

    uint32 CreateForPlayer(ByteBuffer *data);
    void DestroyForPlayer(Player* plr);

    void LoadPlayerItems(QueryResult * result);
    void SavePlayerItems(bool first, QueryBuffer * buf);

    Item* GetInventoryItem(int16 slot);
    Item* GetInventoryItem(int16 ContainerSlot, int16 slot);
    int16 GetInventorySlotById(uint32 ID);
    int16 GetInventorySlotByGuid(uint64 guid);
    int16 GetInventorySlotByGuid2(uint64 guid);
    int16 GetBagSlotByGuid(uint64 guid);

    Item* SafeAddItem(uint32 ItemId, int16 ContainerSlot, int16 slot);
    AddItemResult SafeAddItem(Item* pItem, int16 ContainerSlot, int16 slot);
    Item* SafeRemoveAndRetreiveItemFromSlot(int16 ContainerSlot, int16 slot, bool destroy); //doesnt destroy item from memory
    Item* SafeRemoveAndRetreiveItemByGuid(uint64 guid, bool destroy);
    Item* SafeRemoveAndRetreiveItemByGuidRemoveStats(uint64 guid, bool destroy);
    bool SafeFullRemoveItemFromSlot(int16 ContainerSlot, int16 slot); //destroys item fully
    bool SafeFullRemoveItemByGuid(uint64 guid); //destroys item fully
    AddItemResult AddItemToFreeSlot(Item* item);
    AddItemResult AddItemToFreeBankSlot(Item* item);
    uint32 GetEquippedItemCountWithLimitId(uint32 Id);
    uint32 GetSocketedGemCountWithLimitId(uint32 Id);

    /** Finds a stack that didn't reach max capacity
    \param itemid The entry of the item to search for
    \param cnt The item count you wish to add to the stack
    \param IncBank Should this search the player's bank as well?
    \return An Item* to a stack of itemid which can contain cnt more items
    */
    Item* FindItemLessMax(uint32 itemid, uint32 cnt, bool IncBank);
    uint32 GetItemCount(uint32 itemid, bool IncBank = false, Item* exclude = NULL, uint32 counttoexclude = 0);
    uint32 RemoveItemAmt(uint32 id, uint32 amt);
    uint32 RemoveItemAmt_ProtectPointer(uint32 id, uint32 amt, Item** pointer);
    uint32 RemoveItemAmtByGuid(uint64 guid, uint32 amt);
    void RemoveAllConjured();
    void BuyItem(ItemPrototype *item, uint32 total_amount, Creature* pVendor, ItemExtendedCostEntry *ec);

    uint32 CalculateFreeSlots(ItemPrototype *proto);
    void ReduceItemDurability();

    //Searching functions
    SlotResult FindFreeInventorySlot(ItemPrototype *proto);
    SlotResult FindFreeBankSlot(ItemPrototype *proto);
    SlotResult FindAmmoBag();
    int16 FindFreeBackPackSlot();
    int16 FindFreeKeyringSlot();
    int16 FindSpecialBag(Item* item);

    uint32 GetEquippedCountByItemLimit(uint32 LimitId);
    int16 CanEquipItemInSlot(int16 DstInvSlot, int16 slot, ItemPrototype* item, bool ignore_combat = false, bool skip_2h_check = false);
    int16 CanEquipItemInSlot2(int8 DstInvSlot, int8 slot, Item* item, bool ignore_combat = false, bool skip_2h_check = false);
    int8 CanReceiveItem(ItemPrototype * item, uint32 amount, ItemExtendedCostEntry *ec);
    int8 CanAffordItem(ItemPrototype * item,uint32 amount, Creature* pVendor, ItemExtendedCostEntry *ec);
    int8 GetItemSlotByType(ItemPrototype* proto);
    Item* GetItemByGUID(uint64 itemGuid);


    void BuildInventoryChangeError(Item* SrcItem, Item* DstItem, uint8 Error);
    bool SwapItemSlots(int16 srcslot, int16 dstslot);

    int16 GetInternalBankSlotFromPlayer(int16 islot); //converts inventory slots into 0-x numbers

    void AddBuyBackItem(Item* it, uint32 price);
    void RemoveBuyBackItem(uint32 index);
    void EmptyBuyBack();
    bool IsEquipped(uint32 itemid);
    void CheckAreaItems();

public:
    RONIN_INLINE bool VerifyBagSlots(int16 ContainerSlot, int16 Slot)
    {
        if( ContainerSlot < -1 || Slot < 0 )
            return false;

        if( ContainerSlot > 0 && (ContainerSlot < INVENTORY_SLOT_BAG_START || ContainerSlot >= INVENTORY_SLOT_BAG_END) )
            return false;

        if( ContainerSlot == -1 && (Slot >= INVENTORY_SLOT_ITEM_END  || Slot <= EQUIPMENT_SLOT_END) )
            return false;

        return true;
    }

    bool AddItemById(uint32 itemid, uint32 count, int32 randomprop, bool created, Player* creator = NULL);
    void SwapItems(int16 SrcInvSlot, int16 DstInvSlot, int16 SrcSlot, int16 DstSlot);

    RONIN_INLINE bool VerifyBagSlotsWithBank(int16 ContainerSlot, int16 Slot)
    {
        if( ContainerSlot < -1 || Slot < 0 )
            return false;

        if( ContainerSlot > 0 && (ContainerSlot < INVENTORY_SLOT_BAG_START || ContainerSlot >= INVENTORY_SLOT_BAG_END) )
            return false;

        if( ContainerSlot == -1 && (Slot == INVENTORY_SLOT_NONE || Slot <= EQUIPMENT_SLOT_END) )
            return false;

        return true;
    }

    RONIN_INLINE bool VerifyBagSlotsWithInv(int16 ContainerSlot, int16 Slot)
    {
        if( ContainerSlot < -1 || Slot < 0 )
            return false;

        if( ContainerSlot > 0 && (ContainerSlot < INVENTORY_SLOT_BAG_START || ContainerSlot >= INVENTORY_SLOT_BAG_END) )
            return false;

        if( ContainerSlot == -1 && Slot == INVENTORY_SLOT_NONE )
            return false;

        return true;
    }
    void RemoveItemsWithHolidayId(uint32 IgnoreHolidayId = 0);
};
