/***
 * Demonstrike Core
 */

#pragma once

#define INVENTORY_SLOT_NOT_SET -1
#define MAX_CONTAINER_SLOTS 36

class SERVER_DECL Container : public Item
{
public:
    Container(ItemPrototype *proto, uint32 counter);
    ~Container();

    virtual void Init();
    virtual void Destruct();
    virtual bool IsContainer() { return true; }

    void LoadFromDB( Field*fields);

    bool AddItem(uint8 slot, Item* item);
    bool AddItemToFreeSlot(Item* pItem, uint8 *r_slot);

    Item* GetItem(uint8 slot) { ASSERT(slot < GetSlotCount()); return m_itemSlots[slot]; }

    uint8 FindFreeSlot();
    bool HasItems();

    void SwapItems(uint8 SrcSlot, uint8 DstSlot);
    Item* SafeRemoveAndRetreiveItemFromSlot(uint8 slot, bool destroy); //doesnt destroy item from memory
    bool SafeFullRemoveItemFromSlot(uint8 slot); //destroys item fully

    void SaveBagToDB(uint8 slot, bool first, QueryBuffer * buf);

    uint8 GetSlotCount() { return (m_uint32Values[CONTAINER_FIELD_NUM_SLOTS]&0xFF); }
    void SetSlotCount(uint32 count) { SetUInt32Value(CONTAINER_FIELD_NUM_SLOTS, std::min<uint32>(MAX_CONTAINER_SLOTS, count)); }

protected:
    Item **m_itemSlots;

};
