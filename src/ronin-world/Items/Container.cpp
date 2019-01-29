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

#include "StdAfx.h"

Container::Container(ItemPrototype *proto, uint32 counter) : Item(proto, counter, CONTAINER_END)
{
    SetTypeFlags(TYPEMASK_TYPE_CONTAINER);
    m_objType = TYPEID_CONTAINER;

    SetSlotCount(proto->ContainerSlots);
    uint8 TotalSlots = GetSlotCount();
    m_itemSlots = new Item*[TotalSlots];
    for (uint8 i = 0; i < TotalSlots; i++)
        m_itemSlots[i] = NULL;
}

Container::~Container( )
{
    uint8 TotalSlots = GetSlotCount();
    for (uint8 i = 0; i < TotalSlots; i++)
        m_itemSlots[i] = NULL;
    delete [] m_itemSlots;
}

void Container::LoadFromDB( Field *fields )
{
    Item::LoadFromDB(fields);
}

int16 Container::FindFreeSlot(int16 ourSlot, std::set<std::pair<int16, int16>> *ignoreSlots)
{
    int16 TotalSlots = GetSlotCount();
    for (int16 i = 0; i < TotalSlots; i++)
    {
        std::pair<int16, int16> slotPair = std::make_pair(ourSlot, i);
        if(ignoreSlots && ignoreSlots->find(slotPair) != ignoreSlots->end())
            continue;

        if(m_itemSlots[i])
            return i;
    }

    sLog.Debug( "Container","FindFreeSlot: no slot available" );
    return ITEM_NO_SLOT_AVAILABLE;
}

bool Container::HasItems()
{
    uint8 TotalSlots = GetSlotCount();
    for (uint8 i = 0; i < TotalSlots; i++)
        if(m_itemSlots[i])
            return true;
    return false;
}

bool Container::AddItem(uint8 slot, Item* item)
{
    if (m_owner == NULL || slot >= GetSlotCount())
        return false;
    if(m_itemSlots[slot] != NULL)
        return false;

    m_itemSlots[slot] = item;
    item->m_isDirty = true;

    item->SetUInt64Value(ITEM_FIELD_CONTAINED, GetGUID());
    item->SetOwner(m_owner);

    Bind(ITEM_BIND_ON_PICKUP);

    SetUInt64Value(CONTAINER_FIELD_SLOT_1 + (slot*2), item->GetGUID());

    //new version to fix bag issues
    if(m_owner->IsInWorld() && !item->IsInWorld())
    {
        item->SetItemInWorld(true);

        ByteBuffer buf(2500);
        if(uint32 count = item->BuildCreateUpdateBlockForPlayer(&buf, m_owner))
            m_owner->PushUpdateBlock(m_owner->GetMapId(), &buf, count);
    }
    return true;
}

void Container::SwapItems(uint8 SrcSlot, uint8 DstSlot)
{
    Item* temp;
    if( SrcSlot >= GetUInt32Value(CONTAINER_FIELD_NUM_SLOTS))
        return;
    if( DstSlot >= GetUInt32Value(CONTAINER_FIELD_NUM_SLOTS))
        return;

    if(m_itemSlots[DstSlot] &&  m_itemSlots[SrcSlot] && m_itemSlots[DstSlot]->GetEntry()==m_itemSlots[SrcSlot]->GetEntry() && (m_itemSlots[DstSlot]->GetProto()->MaxCount < 0 || m_itemSlots[DstSlot]->GetProto()->MaxCount > 1))
    {
        uint32 total = m_itemSlots[SrcSlot]->GetUInt32Value(ITEM_FIELD_STACK_COUNT)+m_itemSlots[DstSlot]->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
        m_itemSlots[DstSlot]->m_isDirty = m_itemSlots[SrcSlot]->m_isDirty = true;
        if(total <= (uint32)m_itemSlots[DstSlot]->GetProto()->MaxCount || m_itemSlots[DstSlot]->GetProto()->MaxCount < 0)
        {
            m_itemSlots[DstSlot]->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT,m_itemSlots[SrcSlot]->GetUInt32Value(ITEM_FIELD_STACK_COUNT));
            SafeFullRemoveItemFromSlot(SrcSlot);
            return;
        }
        else
        {
            if(!(m_itemSlots[DstSlot]->GetUInt32Value(ITEM_FIELD_STACK_COUNT) == m_itemSlots[DstSlot]->GetProto()->MaxCount))
            {
                int32 delta = m_itemSlots[DstSlot]->GetProto()->MaxCount-m_itemSlots[DstSlot]->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
                m_itemSlots[DstSlot]->SetUInt32Value(ITEM_FIELD_STACK_COUNT,m_itemSlots[DstSlot]->GetProto()->MaxCount);
                m_itemSlots[SrcSlot]->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT,-delta);
                return;
            }
        }
    }

    temp = m_itemSlots[SrcSlot];
    m_itemSlots[SrcSlot] = m_itemSlots[DstSlot];
    m_itemSlots[DstSlot] = temp;

    if( m_itemSlots[DstSlot])
    {
        SetUInt64Value(CONTAINER_FIELD_SLOT_1  + (DstSlot*2),  m_itemSlots[DstSlot]->GetGUID()  );
        m_itemSlots[DstSlot]->m_isDirty = true;
    } else SetUInt64Value(CONTAINER_FIELD_SLOT_1  + (DstSlot*2), 0 );

    if( m_itemSlots[SrcSlot])
    {
        SetUInt64Value(CONTAINER_FIELD_SLOT_1  + (SrcSlot*2), m_itemSlots[SrcSlot]->GetGUID() );
        m_itemSlots[SrcSlot]->m_isDirty = true;
    } else SetUInt64Value(CONTAINER_FIELD_SLOT_1  + (SrcSlot*2), 0 );
}

Item* Container::SafeRemoveAndRetreiveItemFromSlot(uint8 slot, bool destroy)
{
    if (slot >= GetSlotCount())
        return NULL;

    Item* pItem = m_itemSlots[slot];
    if (pItem == NULL || pItem == this)
        return NULL;

    m_itemSlots[slot] = NULL;
    if( pItem->GetOwner() == m_owner )
    {
        SetUInt64Value(CONTAINER_FIELD_SLOT_1  + slot*2, 0 );
        pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, 0);

        if(destroy)
        {
            if(pItem->IsInWorld())
                pItem->RemoveFromWorld();
            pItem->DeleteFromDB();
        }
    } else pItem = NULL;

    return pItem;
}

bool Container::SafeFullRemoveItemFromSlot(uint8 slot)
{
    if (slot >= GetSlotCount())
        return false;

    Item* pItem = m_itemSlots[slot];

    if(pItem == NULL || pItem == this)
        return false;

    m_itemSlots[slot] = NULL;
    SetUInt64Value(CONTAINER_FIELD_SLOT_1 + slot*2, 0 );
    pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, 0);

    if(pItem->IsInWorld())
        pItem->RemoveFromWorld();
    pItem->DeleteFromDB();
    pItem->Destruct();
    return true;
}

bool Container::AddItemToFreeSlot(Item* pItem, uint8 *r_slot)
{
    uint8 slotCount = GetSlotCount();
    for(uint8 slot = 0; slot < slotCount; slot++)
    {
        if(!m_itemSlots[slot])
        {
            m_itemSlots[slot] = pItem;
            pItem->m_isDirty = true;

            pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, GetGUID());
            pItem->SetOwner(m_owner);

            SetUInt64Value(CONTAINER_FIELD_SLOT_1  + (slot*2), pItem->GetGUID());

            if(m_owner->IsInWorld() && !pItem->IsInWorld())
            {
                pItem->SetItemInWorld(true);

                ByteBuffer buf(2500);
                if(uint32 count = pItem->BuildCreateUpdateBlockForPlayer( &buf, m_owner ))
                    m_owner->PushUpdateBlock(m_owner->GetMapId(), &buf, count);
            }

            if(r_slot) *r_slot = slot;
            return true;
        }
    }
    return false;
}

Item *Container::GetItem(WoWGuid guid, uint8 *slotOut)
{
    uint8 slotCount = GetSlotCount();
    for(uint8 i = 0; i < slotCount; i++)
    {
        if(m_itemSlots[i] && m_itemSlots[i]->GetGUID() == guid)
        {
            if(slotOut)
                *slotOut = i;
            return m_itemSlots[i];
        }
    }
    return NULL;
}

void Container::SaveBagToDB(uint8 slot, bool first, QueryBuffer * buf)
{
    SaveToDB(INVENTORY_SLOT_NOT_SET, slot, first, buf);
    uint8 slotCount = GetSlotCount();
    for(uint8 i = 0; i < slotCount; i++)
        if (m_itemSlots[i] && !(m_itemSlots[i]->GetProto()->Flags & DBC_ITEMFLAG_CONJURED) )
            m_itemSlots[i]->SaveToDB(slot, i, first, buf);
}