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

enum AddItemFlags
{
    ADDITEM_FLAG_LOOTED = 0x01,
    ADDITEM_FLAG_GIFTED = 0x02,
    ADDITEM_FLAG_CREATED = 0x04,
    ADDITEM_FLAG_SILENT = 0x08,
    ADDITEM_FLAG_QUICKSAVE = 0x10
};

#define ITEM_NO_SLOT_AVAILABLE -1 //works for all kind of slots now
#define INVENTORY_SLOT_NOT_SET -1

#define MAX_BUYBACK_SLOT 13
#define MAX_INVENTORY_SLOT 118

class SERVER_DECL PlayerInventory
{
private:
    SlotResult result;
    Player* m_pOwner;
    Item* m_pItems[MAX_INVENTORY_SLOT];
    Item* m_pBuyBack[MAX_BUYBACK_SLOT];
    std::set<WoWGuid> m_lockedItems;

    AddItemResult m_AddItem(Item* item, int16 ContainerSlot, int16 slot);

public:
    friend class ItemIterator;
    PlayerInventory( Player* pPlayer );
    ~PlayerInventory();

    void AddToWorld();
    void RemoveFromWorld();

    Player* GetOwner() { return m_pOwner; }
    bool IsBagSlot(int16 slot);

    uint32 m_CreateForPlayer(ByteBuffer *data);
    void m_DestroyForPlayer(Player* plr);

    void mLoadItemsFromDatabase(QueryResult * inventory, QueryResult *enchants);
    void mSaveItemsToDatabase(bool first, QueryBuffer * buf);
    void mAddItemToBestSlot(ItemPrototype *proto, uint32 count, bool fromDB);

    void ModifyLevelBasedItemBonuses(bool apply);
    bool FillItemLevelData(uint32 &itemCount, uint32 &totalItemLevel, bool inventoryOnly);

    Item* GetInventoryItem(WoWGuid guid);
    Item* GetInventoryItem(int16 slot);
    Item* GetInventoryItem(int16 ContainerSlot, int16 slot);
    int16 GetInventorySlotById(uint32 ID);
    int16 GetInventorySlotByGuid(uint64 guid);
    int16 GetInventorySlotByGuid2(uint64 guid);
    int16 GetBagSlotByGuid(uint64 guid, uint8 &slotOut);

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

    bool countDownFreeSlots(ItemPrototype *proto, uint32 requiredSpace);
    uint32 CalculateFreeSlots(ItemPrototype *proto);
    void ReduceItemDurability();

    uint16 LastSearchItemBagSlot(){return result.ContainerSlot;}
    uint16 LastSearchItemSlot(){return result.Slot;}
    SlotResult *LastSearchResult(){return &result;}

    //Searching functions
    SlotResult FindFreeInventorySlot(ItemPrototype *proto, std::set<std::pair<int16, int16>> *ignoreSlots = NULL);
    SlotResult FindFreeBankSlot(ItemPrototype *proto, std::set<std::pair<int16, int16>> *ignoreSlots = NULL);
    SlotResult FindAmmoBag();

    int16 FindFreeBackPackSlot(std::set<std::pair<int16, int16>> *ignoreSlots = NULL);
    int16 FindSpecialBag(Item* item);

    uint32 GetEquippedCountByItemID(uint32 itemID);
    uint32 GetEquippedCountByItemLimit(uint32 LimitId);
    int16 CanEquipItemInSlot(int16 SrcInvSlot, int16 DstInvSlot, int16 slot, ItemPrototype* item, bool ignore_combat = false, bool skip_2h_check = false);
    int16 CanEquipItemInSlot2(int16 SrcInvSlot, int16 DstInvSlot, int16 slot, Item* item, bool ignore_combat = false, bool skip_2h_check = false);
    int8 CanReceiveItem(ItemPrototype * item, uint32 amount, ItemExtendedCostEntry *ec);
    int8 CanAffordItem(ItemPrototype * item,uint32 amount, Creature* pVendor, ItemExtendedCostEntry *ec);
    int8 GetItemSlotByType(ItemPrototype* proto);
    Item* GetItemByGUID(uint64 itemGuid);


    void BuildInventoryChangeError(Item* SrcItem, Item* DstItem, uint8 Error, WoWGuid srcGuidRep = 0, WoWGuid dstGuidRep = 0);
    bool SwapItemSlots(int16 srcslot, int16 dstslot);

    int16 GetInternalBankSlotFromPlayer(int16 islot); //converts inventory slots into 0-x numbers

    //buyback stuff
    RONIN_INLINE Item* GetBuyBack(int32 slot)
    {
        if(slot >= 0 && slot <= 12)
            return m_pBuyBack[slot];
        return NULL;
    }

    static bool CanTradeItem(Player *owner, Item *item);
    void AddBuyBackItem(Item* it, uint32 price);
    void RemoveBuyBackItem(uint32 index);
    void EmptyBuyBack();
    bool IsEquipped(uint32 itemid, int16 slotToSkip = -1);
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

    struct AddItemDestination
    {
        struct DestinationSlot
        {
            int16 invSlot, slot;
            uint32 itemId, count;
            int32 randomProp;
            uint8 addFlag;
            WoWGuid creatorGuid;
        };

        std::set<std::pair<int16, int16>> usedSlots;
        std::vector<DestinationSlot*> m_destinationSlots;
    };

    bool BuildItemDestinations(AddItemDestination *concurrentAdds, uint32 itemId, uint32 count, int32 randomProp, uint8 addFlag, Player *creator = NULL);
    void ProcessItemDestination(AddItemDestination *dest);

    uint32 AddItemById(uint32 itemid, uint32 count, int32 randomprop, uint8 addFlag, Player* creator = NULL);
    void SwapItems(int16 SrcInvSlot, int16 DstInvSlot, int16 SrcSlot, int16 DstSlot);

    RONIN_INLINE bool VerifyBagSlotsWithBank(int16 ContainerSlot, int16 Slot)
    {
        if( ContainerSlot < -1 || Slot < 0 )
            return false;

        if( ContainerSlot > 0 && (ContainerSlot < INVENTORY_SLOT_BAG_START || ContainerSlot >= INVENTORY_SLOT_BAG_END) )
            return false;

        if( ContainerSlot == -1 && (Slot >= MAX_INVENTORY_SLOT || Slot <= EQUIPMENT_SLOT_END) )
            return false;

        return true;
    }

    RONIN_INLINE bool VerifyBagSlotsWithInv(int16 ContainerSlot, int16 Slot)
    {
        if( ContainerSlot < -1 || Slot < 0 )
            return false;

        if( ContainerSlot > 0 && (ContainerSlot < INVENTORY_SLOT_BAG_START || ContainerSlot >= INVENTORY_SLOT_BAG_END) )
            return false;

        if( ContainerSlot == -1 && Slot >= MAX_INVENTORY_SLOT )
            return false;

        return true;
    }
    void RemoveItemsWithHolidayId(uint32 IgnoreHolidayId = 0);

    void _sendPushResult(Item *item, int8 bagSlot, uint8 slot, uint32 count, uint8 addItemFlags);
};

class ItemIterator
{
    bool m_atEnd;
    bool m_searchInProgress;
    uint32 m_slot;
    uint32 m_containerSlot;
    Container* m_container;
    Item* m_currentItem;
    PlayerInventory* m_target;
public:
    ItemIterator(PlayerInventory* target) : m_atEnd(false),m_searchInProgress(false),m_slot(0),m_containerSlot(0),m_container(NULL),m_target(target) {}
    ~ItemIterator() { if(m_searchInProgress) { EndSearch(); } }

    void BeginSearch()
    {
        // iteminterface doesn't use mutexes, maybe it should :P
        ASSERT(!m_searchInProgress);
        m_atEnd=false;
        m_searchInProgress=true;
        m_container=NULL;
        m_currentItem=NULL;
        m_slot=0;
        Increment();
    }

    void EndSearch()
    {
        // nothing here either
        ASSERT(m_searchInProgress);
        m_atEnd=true;
        m_searchInProgress=false;
    }

    Item* operator*() const
    {
        return m_currentItem;
    }

    Item* operator->() const
    {
        return m_currentItem;
    }

    void Increment()
    {
        if(!m_searchInProgress)
            BeginSearch();

        // are we currently inside a container?
        if(m_container != NULL)
        {
            // loop the container.
            for(; m_containerSlot < (uint32)m_container->GetProto()->ContainerSlots; ++m_containerSlot)
            {
                m_currentItem = m_container->GetItem(m_containerSlot);
                if(m_currentItem != NULL)
                {
                    // increment the counter so we don't get the same item again
                    ++m_containerSlot;

                    // exit
                    return;
                }
            }

            // unset this
            m_container=NULL;
        }

        for(; m_slot < MAX_INVENTORY_SLOT; ++m_slot)
        {
            if(m_target->m_pItems[m_slot])
            {
                if(m_target->m_pItems[m_slot]->IsContainer())
                {
                    // we are a container :O lets look inside the box!
                    m_container = castPtr<Container>(m_target->m_pItems[m_slot]);
                    m_containerSlot = 0;

                    // clear the pointer up. so we can tell if we found an item or not
                    m_currentItem = NULL;

                    // increment m_slot so we don't search this container again
                    ++m_slot;

                    // call increment() recursively. this will search the container.
                    Increment();

                    // jump out so we're not wasting cycles and skipping items
                    return;
                }

                // we're not a container, just a regular item
                // set the pointer
                m_currentItem = m_target->m_pItems[m_slot];

                // increment the slot counter so we don't do the same item again
                ++m_slot;

                // jump out
                return;
            }
        }

        // if we're here we've searched all items.
        m_atEnd=true;
        m_currentItem=NULL;
    }

    RONIN_INLINE Item* Grab() { return m_currentItem; }
    RONIN_INLINE bool End() { return m_atEnd; }
};
