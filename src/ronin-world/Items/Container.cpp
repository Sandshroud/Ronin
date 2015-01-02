/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Container::Container(ItemData *data, uint32 fieldCount) : Item(data, fieldCount)
{
    SetTypeFlags(TYPEMASK_TYPE_CONTAINER);
    ASSERT(data->containerData);
}

Container::~Container( )
{

}

void Container::Initialize(Player *owner)
{
    Item::Initialize(owner);
    SetUInt32Value(CONTAINER_FIELD_NUM_SLOTS, _itemData->containerData->numSlots);
    for(std::map<uint8, WoWGuid>::iterator itr = _itemData->containerData->m_items.begin(); itr != _itemData->containerData->m_items.end(); itr++)
        SetUInt64Value(CONTAINER_FIELD_SLOT_1+(itr->first*2), itr->second);
}

void Container::Destruct()
{
    for(auto itr = m_items.begin(); itr != m_items.end(); itr++)
        itr->second->Destruct();
    Item::Destruct();
}

bool Container::AddItem(Item* item, uint8 slot)
{
    if(slot >= GetSlotCount())
        return false;
    if(slot == 255 && (slot = FindFreeSlot()) == 255)
        return false;
    if(m_items.find(slot) != m_items.end())
        return false;

    m_items.insert(std::make_pair(slot, item));
    item->SetContainerData(GetGUID(), slot);
    item->SetOwner(m_owner);

    Bind(ITEM_BIND_ON_PICKUP);
    SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (slot*2), item->GetGUID());
    return true;
}

uint8 Container::FindFreeSlot()
{
    uint8 TotalSlots;
    if((TotalSlots = GetSlotCount()) != m_items.size())
    {
        for (uint8 i = 0; i < TotalSlots; i++)
        {
            if(m_items.find(i) != m_items.end())
                continue;
            return i;
        }
    }
    return 0xFF;
}

bool Container::RemoveItem(uint8 slot)
{
    if(m_items.find(slot) == m_items.end())
        return NULL;

    Item *pItem = m_items.at(slot);
    m_items.erase(slot);

    SetUInt64Value(CONTAINER_FIELD_SLOT_1  + slot*2, 0 );
    pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, 0);
    pItem->RemoveFromWorld(true);
    return true;
}

Item *Container::RetreiveItem(uint8 slot)
{
    if(m_items.find(slot) == m_items.end())
        return NULL;

    Item *pItem = m_items.at(slot);
    m_items.erase(slot);

    SetUInt64Value(CONTAINER_FIELD_SLOT_1 + slot*2, 0 );
    if(GetGUID() == pItem->GetContainerGuid())
        pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, 0);
    pItem->RemoveFromWorld(false);
    return pItem;
}

void Container::SwapItems(uint8 SrcSlot, uint8 DstSlot)
{
    if(m_items.find(SrcSlot) == m_items.end() && m_items.find(DstSlot) == m_items.end())
        return;
    uint8 slotCount = GetSlotCount();
    if(SrcSlot >= slotCount || DstSlot >= slotCount)
        return;

    Item *src = m_items.find(SrcSlot) != m_items.end() ? m_items.at(SrcSlot) : NULL,
        *dst = m_items.find(DstSlot) != m_items.end() ? m_items.at(DstSlot) : NULL;
    if(src && dst && src->GetEntry() == dst->GetEntry() && !src->IsWrapped() && !dst->IsWrapped())
    {
        if(dst->GetProto()->MaxCount > dst->GetStackCount())
        {
            uint32 delta = dst->GetProto()->MaxCount-dst->GetStackCount();
            if(delta >= src->GetStackCount())
            {
                dst->ModStackCount(src->GetStackCount());
                sItemMgr.QueueItemDeletion(src);
                RemoveItem(SrcSlot);
            }
            else
            {
                dst->ModStackCount(delta);
                src->ModStackCount(-int32(delta));
                sItemMgr.QueueItemSave(src);
            }
            sItemMgr.QueueItemSave(dst);
            return;
        }
    }

    m_items.erase(SrcSlot);
    m_items.erase(DstSlot);
    if(dst)
    {
        SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (SrcSlot*2), dst->GetGUID());
        m_items.insert(std::make_pair(SrcSlot, dst));
        sItemMgr.ContainerSlotChange(dst, SrcSlot);
    }
    if(src)
    {
        SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (DstSlot*2), src->GetGUID());
        m_items.insert(std::make_pair(DstSlot, src));
        sItemMgr.ContainerSlotChange(src, DstSlot);
    }
}

uint32 Container::BuildCreateUpdateBlockForPlayer(ByteBuffer *data, Player *target)
{
    uint32 count = Object::BuildCreateUpdateBlockForPlayer(data, target);
    for(std::map<uint8, Item*>::iterator itr = m_items.begin(); itr != m_items.end(); itr++)
        count += itr->second->BuildCreateUpdateBlockForPlayer(data, target);
    return 1;
}