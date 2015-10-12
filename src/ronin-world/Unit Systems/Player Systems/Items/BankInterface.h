/***
 * Demonstrike Core
 */

#pragma once

class SERVER_DECL PlayerBank
{
public:
    typedef std::map<uint16, ItemData*> ItemDataStorage;

private:
    Player* m_pOwner;
    ItemDataStorage m_itemData;
    std::map<WoWGuid, uint16> m_itemSlots;
    std::map<uint32, std::set<WoWGuid> > m_itemsByEntry;

public:
    PlayerBank( Player* pPlayer );
    ~PlayerBank();

    void LoadFromDB(QueryResult *result);
    void SaveToDB(QueryBuffer *buf);

    uint32 BuildCreateUpdateBlocks(ByteBuffer *data);
    void DestroyForPlayer(Player* plr);

private:
    RONIN_INLINE bool IsValidBankSlot(Item *item, uint16 slot)
    {
        uint8 cBagSlot = INVSLOT_BAG(slot), cSlot = INVSLOT_ITEM(slot);
        if(cBagSlot == INVENTORY_SLOT_NONE)
        {
            if(m_itemData.find(slot) != m_itemData.end())
                return false;
            if(cSlot < BANK_SLOT_ITEM_START || cSlot >= BANK_SLOT_BAG_END)
                return false;
            if(cSlot > BANK_SLOT_BAG_START && !item->IsContainer())
                return false;
        }
        else
        {
            if(cBagSlot < BANK_SLOT_BAG_START || cBagSlot >= BANK_SLOT_BAG_END)
                return false;
            uint16 iBagSlot = MAKE_INVSLOT(0xFF, cBagSlot);
            if(m_itemData.find(iBagSlot) == m_itemData.end())
                return false;
            ItemData *item = m_itemData.at(iBagSlot);
            if(item->containerData == NULL)
                return false;
            if(cSlot > item->proto->ContainerSlots)
                return false;
            ItemData::ContainerData *cData = item->containerData;
            if(cData->m_items.find(cSlot) != cData->m_items.end())
                return false;
        }
        return true;
    }
};
