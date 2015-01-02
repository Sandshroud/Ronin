/***
 * Demonstrike Core
 */

#pragma once

class SERVER_DECL Container : public Item
{
public:
    Container(ItemData *data, uint32 fieldCount = CONTAINER_END);
    ~Container();

    virtual void Initialize(Player *owner);
    virtual void Destruct();

    bool AddItem(Item* item, uint8 slot = 255);
    RONIN_INLINE Item *GetItem(uint8 slot)
    {
        if(m_items.find(slot) != m_items.end())
            return m_items.at(slot);
        return NULL;
    }

    uint8 FindFreeSlot();
    bool RemoveItem(uint8 slot);
    Item *RetreiveItem(uint8 slot);
    void SwapItems(uint8 SrcSlot, uint8 DstSlot);
    RONIN_INLINE bool HasItems() { return m_items.size(); };

    uint32 GetSlotCount() { return GetUInt32Value(CONTAINER_FIELD_NUM_SLOTS); }

    uint32 __fastcall BuildCreateUpdateBlockForPlayer( ByteBuffer *data, Player* target );

protected:
    std::map<uint8, Item*> m_items;
};
