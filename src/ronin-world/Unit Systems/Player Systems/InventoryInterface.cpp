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

//
//-------------------------------------------------------------------//
PlayerInventory::PlayerInventory( Player* pPlayer )
{
    m_pOwner = pPlayer;

    memset(m_pItems, 0, sizeof(Item*)*MAX_INVENTORY_SLOT);
    memset(m_pBuyBack, 0, sizeof(Item*)*MAX_BUYBACK_SLOT);
}

//-------------------------------------------------------------------//
PlayerInventory::~PlayerInventory()
{
    for(int i = 0; i < MAX_INVENTORY_SLOT; i++)
    {
        if( m_pItems[i] != NULL && m_pItems[i]->GetOwner() == m_pOwner )
        {
            m_pItems[i]->Destruct();
            m_pItems[i] = NULL;
        }
    }

    m_pOwner = NULL;
}

void PlayerInventory::AddToWorld()
{
    for(uint8 i = 0; i < MAX_INVENTORY_SLOT; i++)
    {
        if(m_pItems[i])
        {
            if(!m_pItems[i]->IsInWorld() && i < INVENTORY_SLOT_BAG_END) // only equipment slots get mods.
                m_pOwner->ApplyItemMods(m_pItems[i], i, true);

            m_pItems[i]->SetItemInWorld(true);
            if(Container *pContainer = m_pItems[i]->IsContainer() ? castPtr<Container>(m_pItems[i]) : NULL)
                for(uint8 e = 0; e < pContainer->GetSlotCount(); e++)
                    if(Item* pItem = pContainer->GetItem(e))
                        pItem->SetItemInWorld(true);
        }
    }
}

void PlayerInventory::RemoveFromWorld()
{
    for(uint8 i = 0; i < MAX_INVENTORY_SLOT; i++)
    {
        if(m_pItems[i])
        {
            if(m_pItems[i]->IsInWorld() && i < INVENTORY_SLOT_BAG_END) // only equipment slots get mods.
                m_pOwner->ApplyItemMods(m_pItems[i], i, false);
            m_pItems[i]->SetItemInWorld(false);

            if(Container *pContainer = m_pItems[i]->IsContainer() ? castPtr<Container>(m_pItems[i]) : NULL)
                for(uint8 e = 0; e < pContainer->GetSlotCount(); e++)
                    if(Item* pItem = pContainer->GetItem(e))
                        pItem->SetItemInWorld(false);
        }
    }
}

//-------------------------------------------------------------------// 100%
uint32 PlayerInventory::m_CreateForPlayer(ByteBuffer *data)
{
    ASSERT(m_pOwner != NULL);
    uint32 count = 0;

    for(uint8 i = 0; i < MAX_INVENTORY_SLOT; i++)
    {
        if(m_pItems[i])
        {
            count += m_pItems[i]->BuildCreateUpdateBlockForPlayer(data, m_pOwner);
            if(Container *pContainer = m_pItems[i]->IsContainer() ? castPtr<Container>(m_pItems[i]) : NULL)
            {
                for(uint8 e = 0; e < pContainer->GetSlotCount(); e++)
                {
                    if(Item* pItem = pContainer->GetItem(e))
                        count += pItem->BuildCreateUpdateBlockForPlayer( data, m_pOwner );
                }
            }
        }
    }
    return count;
}

//-------------------------------------------------------------------// 100%
void PlayerInventory::m_DestroyForPlayer(Player* plr)
{
    ASSERT(m_pOwner != NULL);

    for (uint8 i = 0; i < INVENTORY_SLOT_BAG_END; i++)
        if(m_pItems[i] != NULL)
            m_pItems[i]->DestroyForPlayer( plr );

    if(m_pOwner == plr)
    {
        for(uint16 i = INVENTORY_SLOT_ITEM_START; i < MAX_INVENTORY_SLOT; i++)
        {
            if(m_pItems[i] != NULL)
            {
                if(m_pItems[i]->IsContainer() && m_pItems[i]->GetProto())
                {
                    for(int32 e = 0; e < m_pItems[i]->GetProto()->ContainerSlots; e++)
                    {
                        if(Item* pItem = castPtr<Container>(m_pItems[i])->GetItem(e))
                            pItem->DestroyForPlayer( plr );
                    }
                }
                m_pItems[i]->DestroyForPlayer( plr );
            }
        }
    }
}

//-------------------------------------------------------------------//
//Description: Creates and adds a item that can be manipulated after
//-------------------------------------------------------------------//
Item* PlayerInventory::SafeAddItem(uint32 ItemId, int16 ContainerSlot, int16 slot)
{
    if(ItemPrototype *pProto = sItemMgr.LookupEntry(ItemId))
    {
        Item* pItem = objmgr.CreateItem(ItemId, m_pOwner);
        if(m_AddItem(pItem, ContainerSlot, slot))
            return pItem;
        pItem->Destruct();
    }
    return NULL;
}

//-------------------------------------------------------------------//
//Description: Creates and adds a item that can be manipulated after
//-------------------------------------------------------------------//
AddItemResult PlayerInventory::SafeAddItem( Item* pItem, int16 ContainerSlot, int16 slot)
{
    return m_AddItem( pItem, ContainerSlot, slot );
}

//-------------------------------------------------------------------//
//Description: adds items to player inventory, this includes all types of slots.
//-------------------------------------------------------------------//
AddItemResult PlayerInventory::m_AddItem( Item* item, int16 ContainerSlot, int16 slot)
{
    if ( slot >= MAX_INVENTORY_SLOT )
    {
        sLog.outString("%s: slot (%d) >= MAX_INVENTORY_SLOT (%d)", __FUNCTION__, slot, MAX_INVENTORY_SLOT);
        return ADD_ITEM_RESULT_ERROR;
    }

    if ( ContainerSlot >= MAX_INVENTORY_SLOT )
    {
        sLog.outString("%s: ContainerSlot (%d) >= MAX_INVENTORY_SLOT (%d)", __FUNCTION__, ContainerSlot, MAX_INVENTORY_SLOT);
        return ADD_ITEM_RESULT_ERROR;
    }

    SlotResult result;
    if( item == NULL || !item->GetProto() )
        return ADD_ITEM_RESULT_ERROR;

    if( slot < 0 )
    {
        result = FindFreeInventorySlot(item->GetProto());
        if( result.Result )
        {
            slot = result.Slot;
            ContainerSlot = result.ContainerSlot;
        } else return ADD_ITEM_RESULT_ERROR;
    }

    item->m_isDirty = true;

    // doublechecking
    int32 i, j, k;
    Item* tempitem;
    for(i = 0; i < MAX_INVENTORY_SLOT; i++)
    {
        tempitem = m_pItems[i];
        if( tempitem != NULL )
        {
            if( tempitem == item )
                return ADD_ITEM_RESULT_DUPLICATED;

            if( tempitem->IsContainer() )
            {
                k = tempitem->GetProto()->ContainerSlots;
                for(j = 0; j < k; ++j)
                {
                    if( castPtr<Container>(tempitem)->GetItem( j ) == item )
                        return ADD_ITEM_RESULT_DUPLICATED;
                }
            }
        }
    }

    if(item->GetProto() == NULL)
        return ADD_ITEM_RESULT_ERROR;

    //case 1, item is from backpack container
    if(ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        //ASSERT(m_pItems[slot] == NULL);
        if(GetInventoryItem(slot) != NULL || (slot == EQUIPMENT_SLOT_OFFHAND && !m_pOwner->HasSkillLine(118)))
        {
            result = FindFreeInventorySlot(item->GetProto());

            // send message to player
            sChatHandler.BlueSystemMessageToPlr(m_pOwner, "A duplicated item, `%s` was found in your inventory. We've attempted to add it to a free slot in your inventory, if there is none this will fail. It will be attempted again the next time you log on.", item->GetProto()->Name.c_str());
            if(result.Result == true)
            {
                // Found a new slot for that item.
                slot = result.Slot;
                ContainerSlot = result.ContainerSlot;
            } else return ADD_ITEM_RESULT_ERROR;
        }

        if(!GetInventoryItem(slot)) //slot is free, add item.
        {
            item->SetOwner( m_pOwner );
            item->SetUInt64Value(ITEM_FIELD_CONTAINED, m_pOwner->GetGUID());
            m_pItems[slot] = item;
            item->Bind(ITEM_BIND_ON_PICKUP);
            if( m_pOwner->IsInWorld() && !item->IsInWorld())
            {
                item->SetItemInWorld(true);
                ByteBuffer buf(2500);
                if(uint32 count = item->BuildCreateUpdateBlockForPlayer( &buf, m_pOwner ))
                    m_pOwner->PushUpdateBlock(m_pOwner->GetMapId(), &buf, count);
            }
            m_pOwner->SetUInt64Value(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2), item->GetGUID());
        } else return ADD_ITEM_RESULT_ERROR;
    }
    else //case 2: item is from a bag container
    {
        if( GetInventoryItem(ContainerSlot) && GetInventoryItem(ContainerSlot)->IsContainer() &&
            slot < (int32)GetInventoryItem(ContainerSlot)->GetUInt32Value(CONTAINER_FIELD_NUM_SLOTS)) //container exists
        {
            bool result = castPtr<Container>(m_pItems[ContainerSlot])->AddItem(slot, item);
            if( !result )
            {
                return ADD_ITEM_RESULT_ERROR;
            }
        }
        else
        {
            return ADD_ITEM_RESULT_ERROR;
        }
    }

    if ( slot < EQUIPMENT_SLOT_END && ContainerSlot == INVENTORY_SLOT_NOT_SET )
    {
        int VisibleBase = PLAYER_VISIBLE_ITEM + (slot * PLAYER_VISIBLE_ITEM_LENGTH);
        if( VisibleBase < PLAYER_CHOSEN_TITLE )
        {
            m_pOwner->SetUInt32Value( VisibleBase, item->GetUInt32Value( OBJECT_FIELD_ENTRY ) );
            m_pOwner->SetUInt16Value( VisibleBase + 1, 0, item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT));
            m_pOwner->SetUInt16Value( VisibleBase + 1, 1, item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT));
        }
    }

    if( m_pOwner->IsInWorld() && slot < INVENTORY_SLOT_BAG_END && ContainerSlot == INVENTORY_SLOT_NOT_SET )
        m_pOwner->ApplyItemMods( item, slot, true );

    return ADD_ITEM_RESULT_OK;
}

//-------------------------------------------------------------------//
//Description: Checks if the slot is a Bag slot
//-------------------------------------------------------------------//
bool PlayerInventory::IsBagSlot(int16 slot)
{
    if((slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END) || (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END))
    {
        return true;
    }
    return false;
}

//-------------------------------------------------------------------//
//Description: removes the item safely and returns it back for usage
//-------------------------------------------------------------------//
Item* PlayerInventory::SafeRemoveAndRetreiveItemFromSlot(int16 ContainerSlot, int16 slot, bool destroy)
{
    ASSERT(slot < MAX_INVENTORY_SLOT);
    ASSERT(ContainerSlot < MAX_INVENTORY_SLOT);
    Item* pItem = NULL;

    if(ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        if((pItem = GetInventoryItem(ContainerSlot,slot)) == NULL)
            return NULL;

        if(pItem->GetProto()->ContainerSlots > 0 && pItem->GetTypeId() == TYPEID_CONTAINER && castPtr<Container>(pItem)->HasItems())
        {
            /* sounds weird? no. this will trigger a callstack display due to my other debug code. */
            pItem->DeleteFromDB();
            return NULL;
        }

        m_pItems[slot] = NULL;
        if(pItem->GetOwner() == m_pOwner)
        {
            pItem->m_isDirty = true;

            m_pOwner->SetUInt64Value(PLAYER_FIELD_INV_SLOT_HEAD  + (slot*2), 0 );

            if ( slot < EQUIPMENT_SLOT_END )
            {
                m_pOwner->ApplyItemMods( pItem, slot, false );
                int VisibleBase = PLAYER_VISIBLE_ITEM + (slot * PLAYER_VISIBLE_ITEM_LENGTH);
                m_pOwner->SetUInt32Value( VisibleBase, 0 );
                m_pOwner->SetUInt32Value( VisibleBase + 1, 0 );
            } else if ( slot < INVENTORY_SLOT_BAG_END )
                m_pOwner->ApplyItemMods( pItem, slot, false );

            if(destroy)
            {
                if (pItem->IsInWorld())
                    pItem->RemoveFromWorld();
                pItem->DeleteFromDB();
            }
        } else pItem = NULL;
    }
    else
    {
        Item* pContainer = GetInventoryItem(ContainerSlot);
        if(pContainer && pContainer->IsContainer())
        {
            pItem = castPtr<Container>(pContainer)->SafeRemoveAndRetreiveItemFromSlot(slot, destroy);
            if (pItem == NULL) { return NULL; }
        }
    }

    return pItem;
}

//-------------------------------------------------------------------//
//Description: removes the item safely by guid and returns it back for usage, supports full inventory
//-------------------------------------------------------------------//
Item* PlayerInventory::SafeRemoveAndRetreiveItemByGuid(uint64 guid, bool destroy)
{
    int16 i = 0;

    for(i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->GetGUID() == guid)
        {
            return SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for(i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->GetGUID() == guid)
        {
            return SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for(i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if(item && item->GetGUID() == guid)
        {
            return SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
        else
        {
            if(item && item->IsContainer() && item->GetProto())
            {
                for (int32 j =0; j < item->GetProto()->ContainerSlots; j++)
                {
                    Item* item2 = castPtr<Container>(item)->GetItem(j);
                    if (item2 && item2->GetGUID() == guid)
                    {
                        return castPtr<Container>(item)->SafeRemoveAndRetreiveItemFromSlot(j, destroy);
                    }
                }
            }
        }
    }

    for(i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->GetGUID() == guid)
        {
            return SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for(i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->GetGUID() == guid)
        {
            return SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
        else
        {
            if(item && item->IsContainer() && item->GetProto())
            {
                for (int32 j =0; j < item->GetProto()->ContainerSlots; j++)
                {
                    Item* item2 = castPtr<Container>(item)->GetItem(j);
                    if (item2 && item2->GetGUID() == guid)
                    {
                        return castPtr<Container>(item)->SafeRemoveAndRetreiveItemFromSlot(j, destroy);
                    }
                }
            }
        }
    }

    return NULL;
}

Item* PlayerInventory::SafeRemoveAndRetreiveItemByGuidRemoveStats(uint64 guid, bool destroy)
{
    int16 i = 0;

    for(i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->GetGUID() == guid)
        {
            m_pOwner->ApplyItemMods(item, i, false);
            return SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for(i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if(item && item->GetGUID() == guid)
        {
            m_pOwner->ApplyItemMods(item, i, false);
            return SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
        else
        {
            if(item && item->IsContainer() && item->GetProto())
            {
                for (int32 j = 0; j < item->GetProto()->ContainerSlots; j++)
                {
                    Item* item2 = castPtr<Container>(item)->GetItem(j);
                    if (item2 && item2->GetGUID() == guid)
                    {
                        return castPtr<Container>(item)->SafeRemoveAndRetreiveItemFromSlot(j, destroy);
                    }
                }
            }
        }
    }

    for(i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->GetGUID() == guid)
        {
            return SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for(i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->GetGUID() == guid)
        {
            return SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for(i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->GetGUID() == guid)
        {
            return SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
        else
        {
            if(item && item->IsContainer() && item->GetProto())
            {
                for (int32 j =0; j < item->GetProto()->ContainerSlots; j++)
                {
                    Item* item2 = castPtr<Container>(item)->GetItem(j);
                    if (item2 && item2->GetGUID() == guid)
                    {
                        return castPtr<Container>(item)->SafeRemoveAndRetreiveItemFromSlot(j, destroy);
                    }
                }
            }
        }
    }

    return NULL;
}

//-------------------------------------------------------------------//
//Description: completely removes item from player
//Result: true if item removal was succefull
//-------------------------------------------------------------------//
bool PlayerInventory::SafeFullRemoveItemFromSlot(int16 ContainerSlot, int16 slot)
{
    ASSERT(slot < MAX_INVENTORY_SLOT);
    ASSERT(ContainerSlot < MAX_INVENTORY_SLOT);

    if(ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        Item* pItem = GetInventoryItem(slot);

        if (pItem == NULL) { return false; }

        if(pItem->GetProto()->ContainerSlots > 0 && pItem->GetTypeId() == TYPEID_CONTAINER && castPtr<Container>(pItem)->HasItems())
        {
            /* sounds weird? no. this will trigger a callstack display due to my other debug code. */
            pItem->DeleteFromDB();
            return false;
        }

        m_pItems[slot] = NULL;
        // hacky crashfix
        if( pItem->GetOwner() == m_pOwner )
        {
            pItem->m_isDirty = true;

            m_pOwner->SetUInt64Value(PLAYER_FIELD_INV_SLOT_HEAD  + (slot*2), 0 );

            if ( slot < EQUIPMENT_SLOT_END )
            {
                m_pOwner->ApplyItemMods(pItem, slot, false );
                int VisibleBase = PLAYER_VISIBLE_ITEM + (slot * PLAYER_VISIBLE_ITEM_LENGTH);
                m_pOwner->SetUInt32Value( VisibleBase, 0 );
                m_pOwner->SetUInt32Value( VisibleBase + 1, 0 );
            }
            else if( slot < INVENTORY_SLOT_BAG_END )
                m_pOwner->ApplyItemMods(pItem, slot, false ); //watch containers that give attackspeed and stuff ;)

            if (pItem->IsInWorld())
                pItem->RemoveFromWorld();

            pItem->DeleteFromDB();
            pItem->Destruct(); // Please note; possible crash here
            pItem = NULL;
        }
    }
    else
    {
        Item* pContainer = GetInventoryItem(ContainerSlot);
        if(pContainer && pContainer->IsContainer())
        {
            castPtr<Container>(pContainer)->SafeFullRemoveItemFromSlot(slot);
        }
    }
    return true;
}

//-------------------------------------------------------------------//
//Description: removes the item safely by guid, supports full inventory
//-------------------------------------------------------------------//
bool PlayerInventory::SafeFullRemoveItemByGuid(uint64 guid)
{
    int16 i = 0;

    for(i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->GetGUID() == guid)
        {
            return SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
    }

    for(i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->GetGUID() == guid)
        {
            return SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
    }

    for(i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if(item && item->GetGUID() == guid)
        {
            return SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
        else
        {
            if(item && item->IsContainer() && item->GetProto())
            {
                for (int32 j =0; j < item->GetProto()->ContainerSlots; j++)
                {
                    Item* item2 = castPtr<Container>(item)->GetItem(j);
                    if (item2 && item2->GetGUID() == guid)
                    {
                        return castPtr<Container>(item)->SafeFullRemoveItemFromSlot(j);
                    }
                }
            }
        }
    }

    for(i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);

        if (item && item->GetGUID() == guid)
        {
            return SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
    }

    for(i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if(item && item->GetGUID() == guid)
        {
            return SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
        else
        {
            if(item && item->IsContainer() && item->GetProto())
            {
                for (int32 j =0; j < item->GetProto()->ContainerSlots; j++)
                {
                    Item* item2 = castPtr<Container>(item)->GetItem(j);
                    if (item2 && item2->GetGUID() == guid)
                    {
                        return castPtr<Container>(item)->SafeFullRemoveItemFromSlot(j);
                    }
                }
            }
        }
    }
    return false;
}

Item* PlayerInventory::GetInventoryItem(WoWGuid guid)
{
    for(uint32 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
        if( Item *item = m_pItems[i] )
            if(item->GetGUID() == guid)
                return item;

    for(uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        if(Container *container = castPtr<Container>(m_pItems[i]))
            if(Item *item = container->GetItem(guid))
                return item;
    return NULL; //was changed from 0 cuz 0 is the slot for head
}

//-------------------------------------------------------------------//
//Description: Gets a item from Inventory
//-------------------------------------------------------------------//
Item* PlayerInventory::GetInventoryItem(int16 slot)
{
    if(slot < 0 || slot > MAX_INVENTORY_SLOT)
        return NULL;

    return m_pItems[slot];
}

//-------------------------------------------------------------------//
//Description: Gets a Item from inventory or container
//-------------------------------------------------------------------//
Item* PlayerInventory::GetInventoryItem(int16 ContainerSlot, int16 slot)
{
    if(ContainerSlot <= INVENTORY_SLOT_NOT_SET)
    {
        if(slot < 0 || slot > MAX_INVENTORY_SLOT)
            return NULL;

        return m_pItems[slot];
    }
    else
    {
        if(IsBagSlot(ContainerSlot))
        {
            if(m_pItems[ContainerSlot])
            {
                return castPtr<Container>(m_pItems[ContainerSlot])->GetItem(slot);
            }
        }
    }
    return NULL;
}

//-------------------------------------------------------------------//
//Description: checks for stacks that didnt reached max capacity
//-------------------------------------------------------------------//
Item* PlayerInventory::FindItemLessMax(uint32 itemid, uint32 cnt, bool IncBank)
{
    uint32 i = 0;
    for(i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item)
        {
            if((item->GetEntry() == itemid) && (item->GetProto()->MaxCount < 0 || ((uint32)item->GetProto()->MaxCount >= (item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + cnt))))
            {
                return item;
            }
        }
    }

    for(i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if(item && item->IsContainer())
        {
            for (int32 j = 0; j < item->GetProto()->ContainerSlots; j++)
            {
                Item* item2 = castPtr<Container>(item)->GetItem(j);
                if(item2)
                {
                    if((item2->GetProto()->ItemId == itemid) && (item2->GetProto()->MaxCount < 0 || ((uint32)item2->GetProto()->MaxCount >= (item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + cnt))))
                    {
                        return item2;
                    }
                }
            }
        }
    }

    if(IncBank)
    {
        for(i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            Item* item = GetInventoryItem(i);
            if (item)
            {
                if((item->GetEntry() == itemid) && (item->GetProto()->MaxCount < 0 || ((uint32)item->GetProto()->MaxCount >= (item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + cnt))))
                {
                    return item;
                }
            }
        }

        for(i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            Item* item = GetInventoryItem(i);
            if(item && item->IsContainer())
            {

                    for (int32 j =0; j < item->GetProto()->ContainerSlots; j++)
                    {
                        Item* item2 = castPtr<Container>(item)->GetItem(j);
                        if (item2)
                        {
                            if((item2->GetProto()->ItemId == itemid) && (item->GetProto()->MaxCount < 0 || ((uint32)item2->GetProto()->MaxCount >= (item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + cnt))))
                            {
                                return item2;
                            }
                        }
                    }

            }
        }
    }

    return NULL;
}

//-------------------------------------------------------------------//
//Description: finds item ammount on inventory, banks not included
//-------------------------------------------------------------------//
uint32 PlayerInventory::GetItemCount(uint32 itemid, bool IncBank, Item* exclude, uint32 counttoexclude)
{
    Item* ToExcludeTemp = exclude;
    int64 cnt = 0, i = 0, excludec = 0;
    for(i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item)
        {
            if(item == ToExcludeTemp)
            {
                if(item->GetStackCount() == counttoexclude)
                    continue;
                cnt -= counttoexclude;
                ToExcludeTemp = 0;
            }

            if(item->GetEntry() == itemid)
            {
                cnt += item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) ? item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) : 1;
            }
        }
    }

    for(i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if(item && item->IsContainer())
        {
            for (int32 j = 0; j < item->GetProto()->ContainerSlots; j++)
            {
                Item* item2 = castPtr<Container>(item)->GetItem(j);
                if (item2)
                {
                    if(item2 == ToExcludeTemp)
                    {
                        if(item2->GetStackCount() == counttoexclude)
                            continue;
                        cnt -= counttoexclude;
                        ToExcludeTemp = 0;
                    }

                    if (item2->GetEntry() == itemid)
                    {
                        cnt += item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT) ? item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT) : 1;
                    }
                }
            }
        }
    }

    if(IncBank)
    {
        for(i = BANK_SLOT_ITEM_START; i < BANK_SLOT_BAG_END; i++)
        {
            Item* item = GetInventoryItem(i);
            if (item)
            {
                if(item == ToExcludeTemp)
                {
                    if(item->GetStackCount() == counttoexclude)
                        continue;
                    cnt -= counttoexclude;
                    ToExcludeTemp = 0;
                }

                if(item->GetProto()->ItemId == itemid)
                {
                    cnt += item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) ? item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) : 1;
                }
            }
        }

        for(i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            Item* item = GetInventoryItem(i);
            if(item)
            {
                if(item->IsContainer())
                {
                    for (int32 j =0; j < item->GetProto()->ContainerSlots; j++)
                    {
                        Item* item2 = castPtr<Container>(item)->GetItem(j);
                        if (item2)
                        {
                            if(item2 == ToExcludeTemp)
                            {
                                if(item2->GetStackCount() == counttoexclude)
                                    continue;
                                cnt -= counttoexclude;
                                ToExcludeTemp = 0;
                            }

                            if(item2->GetProto()->ItemId == itemid)
                            {
                                cnt += item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT) ? item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT) : 1;
                            }
                        }
                    }
                }
            }
        }
    }

    if(cnt < 0)
        cnt = 0;
    return (uint32)cnt;
}

//-------------------------------------------------------------------//
//Description: Removes a ammount of items from inventory
//-------------------------------------------------------------------//
uint32 PlayerInventory::RemoveItemAmt(uint32 id, uint32 amt)
{
    //this code returns shit return value is fucked
    if (GetItemCount(id) < amt)
    {
        return 0;
    }
    uint32 i;

    for(i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item)
        {
            if(item->GetEntry() == id)
            {
                if(item->GetProto()->ContainerSlots > 0 && item->IsContainer() && castPtr<Container>(item)->HasItems())
                {
                    /* sounds weird? no. this will trigger a callstack display due to my other debug code. */
                    item->DeleteFromDB();
                    continue;
                }

                if (item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > amt)
                {
                    item->SetCount(item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetUInt32Value(ITEM_FIELD_STACK_COUNT)== amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
                    if(result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    amt -= item->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
                }
            }
        }
    }

    for(i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if(item && item->IsContainer())
        {
            for (int32 j =0; j < item->GetProto()->ContainerSlots;j++)
            {
                Item* item2 = castPtr<Container>(item)->GetItem(j);
                if (item2)
                {
                    if (item2->GetProto()->ItemId == id)
                    {
                        if (item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > amt)
                        {
                            item2->SetCount(item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT) - amt);
                            item2->m_isDirty = true;
                            return amt;
                        }
                        else if (item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT)== amt)
                        {
                            bool result = SafeFullRemoveItemFromSlot(i, j);
                            if(result)
                            {
                                return amt;
                            }
                            else
                            {
                                return 0;
                            }
                        }
                        else
                        {
                            amt -= item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
                            SafeFullRemoveItemFromSlot(i, j);

                        }
                    }
                }
            }
        }
    }

    return 0;
}

uint32 PlayerInventory::RemoveItemAmt_ProtectPointer(uint32 id, uint32 amt, Item** pointer)
{
    //this code returns shit return value is fucked
    if (GetItemCount(id) < amt)
    {
        return 0;
    }
    uint32 i;

    for(i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item)
        {
            if(item->GetEntry() == id)
            {
                if(item->GetProto()->ContainerSlots > 0 && item->IsContainer() && castPtr<Container>(item)->HasItems())
                {
                    /* sounds weird? no. this will trigger a callstack display due to my other debug code. */
                    item->DeleteFromDB();
                    continue;
                }

                if (item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > amt)
                {
                    item->SetCount(item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetUInt32Value(ITEM_FIELD_STACK_COUNT)== amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
                    if( pointer != NULL && *pointer != NULL && *pointer == item )
                        *pointer = NULL;

                    if(result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    amt -= item->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);

                    if( pointer != NULL && *pointer != NULL && *pointer == item )
                        *pointer = NULL;
                }
            }
        }
    }

    for(i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if(item && item->IsContainer())
        {
            for (int32 j =0; j < item->GetProto()->ContainerSlots;j++)
            {
                Item* item2 = castPtr<Container>(item)->GetItem(j);
                if (item2)
                {
                    if (item2->GetProto()->ItemId == id)
                    {
                        if (item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > amt)
                        {
                            item2->SetCount(item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT) - amt);
                            item2->m_isDirty = true;
                            return amt;
                        }
                        else if (item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT)== amt)
                        {
                            bool result = SafeFullRemoveItemFromSlot(i, j);
                            if( pointer != NULL && *pointer != NULL && *pointer == item2 )
                                *pointer = NULL;

                            if(result)
                            {
                                return amt;
                            }
                            else
                            {
                                return 0;
                            }
                        }
                        else
                        {
                            amt -= item2->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
                            SafeFullRemoveItemFromSlot(i, j);

                            if( pointer != NULL && *pointer != NULL && *pointer == item2 )
                                *pointer = NULL;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

void PlayerInventory::RemoveAllConjured()
{
    for(uint32 x = INVENTORY_SLOT_BAG_START; x < INVENTORY_SLOT_ITEM_END; ++x)
    {
        if (m_pItems[x]!= NULL)
        {
            if(IsBagSlot(x) && m_pItems[x]->IsContainer())
            {
                Container* bag = castPtr<Container>(m_pItems[x]);

                for(int32 i = 0; i < bag->GetProto()->ContainerSlots; i++)
                {
                    if (bag->GetItem(i) != NULL && bag->GetItem(i)->GetProto() && (bag->GetItem(i)->GetProto()->Flags)& 2)
                        bag->SafeFullRemoveItemFromSlot(i);
                }
            }
            else
            {
                if(m_pItems[x]->GetProto() && (m_pItems[x]->GetProto()->Flags)& 2 )
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, x);
            }
        }
    }
}


//-------------------------------------------------------------------//
//Description: Gets slot number by itemid, banks not included
//-------------------------------------------------------------------//
int16 PlayerInventory::GetInventorySlotById(uint32 ID)
{
    for(uint32 i=0;i<INVENTORY_SLOT_ITEM_END;++i)
    {
        if(m_pItems[i])
        {
            if(m_pItems[i]->GetProto()->ItemId == ID)
            {
                return i;
            }
        }
    }

    return ITEM_NO_SLOT_AVAILABLE;
}

//-------------------------------------------------------------------//
//Description: Gets slot number by item guid, banks not included
//-------------------------------------------------------------------//
int16 PlayerInventory::GetInventorySlotByGuid(uint64 guid)
{
    for(uint32 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        if(m_pItems[i])
        {
            if(m_pItems[i]->GetGUID() == guid)
            {
                return i;
            }
        }
    }

    return ITEM_NO_SLOT_AVAILABLE; //was changed from 0 cuz 0 is the slot for head
}

int16 PlayerInventory::GetBagSlotByGuid(uint64 guid, uint8 &slotOut)
{
    for(uint32 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        if(m_pItems[i])
        {
            if(m_pItems[i]->GetGUID() == guid)
            {
                slotOut = i;
                return INVENTORY_SLOT_NOT_SET;
            }
        }
    }

    for(uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_pItems[i] && m_pItems[i]->GetTypeId() == TYPEID_CONTAINER)
        {
            for(int32 j = 0; j < m_pItems[i]->GetProto()->ContainerSlots; ++j)
            {
                Item* inneritem = castPtr<Container>(m_pItems[i])->GetItem(j);
                if(inneritem && inneritem->GetGUID() == guid)
                {
                    slotOut = j;
                    return i;
                }
            }
        }
    }

    return (slotOut = ITEM_NO_SLOT_AVAILABLE); //was changed from 0 cuz 0 is the slot for head
}

//-------------------------------------------------------------------//
//Description: Adds a Item to a free slot
//-------------------------------------------------------------------//
AddItemResult PlayerInventory::AddItemToFreeSlot(Item* item)
{
    if( item == NULL )
        return ADD_ITEM_RESULT_ERROR;

    if( item->GetProto() == NULL )
        return ADD_ITEM_RESULT_ERROR;

    uint32 i = 0;
    bool result2;
    AddItemResult result3;

    //detect special bag item
    if( item->GetProto()->BagFamily )
    {
        for(i=INVENTORY_SLOT_BAG_START;i<INVENTORY_SLOT_BAG_END;++i)
        {
            if(m_pItems[i])
            {
                if (m_pItems[i]->GetProto()->BagFamily == item->GetProto()->BagFamily)
                {
                    if(m_pItems[i]->IsContainer())
                    {
                        uint8 r_slot;
                        result2 = castPtr<Container>(m_pItems[i])->AddItemToFreeSlot(item, &r_slot);
                        if(result2)
                        {
                            result.ContainerSlot = i;
                            result.Slot = r_slot;
                            result.Result = true;
                            return ADD_ITEM_RESULT_OK;
                        }
                    }

                }
            }
        }
    }

    //INVENTORY
    for(i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;++i)
    {
        if(m_pItems[i] == NULL)
        {
            if(result3 = SafeAddItem(item, INVENTORY_SLOT_NOT_SET, i))
            {
                result.ContainerSlot = INVENTORY_SLOT_NOT_SET;
                result.Slot = i;
                result.Result = true;
                return ADD_ITEM_RESULT_OK;
            }
        }
    }

    //INVENTORY BAGS
    for(i=INVENTORY_SLOT_BAG_START;i<INVENTORY_SLOT_BAG_END;++i)
    {
        if(m_pItems[i] != NULL && m_pItems[i]->GetProto()->BagFamily == 0 && m_pItems[i]->IsContainer()) //special bags ignored
        {
            for (int32 j =0; j < m_pItems[i]->GetProto()->ContainerSlots;j++)
            {
                Item* item2 = castPtr<Container>(m_pItems[i])->GetItem(j);
                if (item2 == NULL)
                {
                    if(result3 = SafeAddItem(item, i, j))
                    {
                        result.ContainerSlot = i;
                        result.Slot = j;
                        result.Result = true;
                        return ADD_ITEM_RESULT_OK;
                    }
                }
            }
        }
    }
    return ADD_ITEM_RESULT_ERROR;
}

bool PlayerInventory::countDownFreeSlots(ItemPrototype *proto, uint32 requiredSpace)
{
    uint32 count = requiredSpace;

    if(proto && proto->BagFamily)
    {
        if(proto->BagFamily & ITEM_TYPE_CURRENCY )
        {
            if(--count == 0)
                return true;
        }
        else
        {
            for(uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END;++i)
            {
                if(m_pItems[i] && m_pItems[i]->IsContainer())
                {
                    if (m_pItems[i]->GetProto()->BagFamily == proto->BagFamily)
                    {
                        int8 slot = castPtr<Container>(m_pItems[i])->FindFreeSlot();
                        if(slot != ITEM_NO_SLOT_AVAILABLE)
                        {
                            if(--count == 0)
                                return true;
                        }
                    }
                }
            }
        }
    }

    for(uint32 i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;++i)
        if(m_pItems[i] == NULL)
            if(--count == 0)
                return true;

    for(uint32 i=INVENTORY_SLOT_BAG_START;i<INVENTORY_SLOT_BAG_END;++i)
    {
        if(m_pItems[i] != NULL )
        {
            if(m_pItems[i]->IsContainer() && !m_pItems[i]->GetProto()->BagFamily)
            {
                for (int32 j =0; j < m_pItems[i]->GetProto()->ContainerSlots;j++)
                {
                    Item* item2 = castPtr<Container>(m_pItems[i])->GetItem(j);
                    if (item2 == NULL)
                    {
                        if(--count == 0)
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

//-------------------------------------------------------------------//
//Description: Calculates inventory free slots, bag inventory slots not included
//-------------------------------------------------------------------//
uint32 PlayerInventory::CalculateFreeSlots(ItemPrototype *proto)
{
    uint32 count = 0;
    uint32 i;

    if(proto && proto->BagFamily)
    {
        if(proto->BagFamily & ITEM_TYPE_CURRENCY )
        {
            count++;
        }
        else
        {
            for(uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END;++i)
            {
                if(m_pItems[i] && m_pItems[i]->IsContainer())
                {
                    if (m_pItems[i]->GetProto()->BagFamily == proto->BagFamily)
                    {
                        int8 slot = castPtr<Container>(m_pItems[i])->FindFreeSlot();
                        if(slot != ITEM_NO_SLOT_AVAILABLE)
                        {
                            count++;
                        }
                    }
                }
            }
        }
    }

    for(i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;++i)
    {
        if(m_pItems[i] == NULL)
        {
            count++;
        }
    }

    for(i=INVENTORY_SLOT_BAG_START;i<INVENTORY_SLOT_BAG_END;++i)
    {
        if(m_pItems[i] != NULL )
        {
            if(m_pItems[i]->IsContainer() && !m_pItems[i]->GetProto()->BagFamily)
            {
                for (int32 j =0; j < m_pItems[i]->GetProto()->ContainerSlots;j++)
                {
                    Item* item2 = castPtr<Container>(m_pItems[i])->GetItem(j);
                    if (item2 == NULL)
                    {
                        count++;
                    }
                }
            }
        }
    }
    return count;
}

//-------------------------------------------------------------------//
//Description: finds a free slot on the backpack
//-------------------------------------------------------------------//
int16 PlayerInventory::FindFreeBackPackSlot()
{
    //search for backpack slots
    for(int8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if(!item)
        {
            return i;
        }
    }

    return ITEM_NO_SLOT_AVAILABLE; //no slots available
}

//-------------------------------------------------------------------//
//Description: converts bank bags slot ids into player bank byte slots(0-5)
//-------------------------------------------------------------------//
int16 PlayerInventory::GetInternalBankSlotFromPlayer(int16 islot)
{
    switch(islot)
    {
    case BANK_SLOT_BAG_1:
        {
            return 1;
        }
    case BANK_SLOT_BAG_2:
        {
            return 2;
        }
    case BANK_SLOT_BAG_3:
        {
            return 3;
        }
    case BANK_SLOT_BAG_4:
        {
            return 4;
        }
    case BANK_SLOT_BAG_5:
        {
            return 5;
        }
    case BANK_SLOT_BAG_6:
        {
            return 6;
        }
    case BANK_SLOT_BAG_7:
        {
            return 7;
        }
    default:
        return 8;
    }
}

uint32 PlayerInventory::GetEquippedCountByItemID(uint32 itemID)
{
    uint32 count = 0;
    for(uint32 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
        if(m_pItems[x] && m_pItems[x]->GetEntry() == itemID)
            count++;
    return count;
}

uint32 PlayerInventory::GetEquippedCountByItemLimit(uint32 LimitId)
{
    uint32 count = 0;
    for(uint32 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
    {
        if(Item* it = m_pItems[x])
        {
            for(uint32 s = 0; s < 3; s++)
            {
                EnchantmentInstance* ei = it->GetEnchantment(SOCK_ENCHANTMENT_SLOT1 + s);
                if(ei && ei->Enchantment && ei->Enchantment->GemEntry)
                {
                    ItemPrototype* ip = sItemMgr.LookupEntry(ei->Enchantment->GemEntry);
                    if(ip && ip->ItemLimitCategory == LimitId)
                        count++;
                }
            }
        }
    }
    return count;
}

//-------------------------------------------------------------------//
//Description: checks if the item can be equipped on a specific slot
//             this will check unique-equipped gems as well
//-------------------------------------------------------------------//
int16 PlayerInventory::CanEquipItemInSlot2(int16 SrcSlot, int16 DstInvSlot, int16 slot, Item* item, bool ignore_combat /* = false */, bool skip_2h_check /* = false */)
{
    ItemPrototype* proto = item->GetProto();

    if(int8 ret = CanEquipItemInSlot(SrcSlot, DstInvSlot, slot, proto, ignore_combat, skip_2h_check))
        return ret;

    if((slot < INVENTORY_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET) || (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET))
    {
        for(uint32 count = 0; count < 3; count++)
        {
            EnchantmentInstance* ei = item->GetEnchantment(SOCK_ENCHANTMENT_SLOT1 + count);
            if(ei && ei->Enchantment && ei->Enchantment->GemEntry ) //huh ? Gem without entry ?
            {
                ItemPrototype* ip = sItemMgr.LookupEntry(ei->Enchantment->GemEntry);

                if( ip ) //maybe gem got removed from db due to update ?
                {
                    if(ip->Flags & DBC_ITEMFLAG_UNIQUE_EQUIPPED && IsEquipped(ip->ItemId))
                        return INV_ERR_ITEM_MAX_COUNT;

                    if(ip->ItemLimitCategory > 0)
                    {
                        uint32 LimitId = ip->ItemLimitCategory;
                        ItemLimitCategoryEntry* ile = dbcItemLimitCategory.LookupEntry(LimitId);
                        if(ile)
                        {
                            uint32 gemCount = 0;
                            if((ile->EquippedFlag & 1  && slot < EQUIPMENT_SLOT_END)
                                    || (!(ile->EquippedFlag & 1) && slot > EQUIPMENT_SLOT_END))
                                gemCount = item->CountGemsWithLimitId(ile->Id);

                            uint32 gCount = GetEquippedCountByItemLimit(ile->Id);
                            if((gCount + gemCount) > ile->MaxAmount)
                                return INV_ERR_ITEM_MAX_COUNT;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

//-------------------------------------------------------------------//
//Description: checks if the item can be equiped on a specific slot
//-------------------------------------------------------------------//
int16 PlayerInventory::CanEquipItemInSlot(int16 SrcSlot, int16 DstInvSlot, int16 slot, ItemPrototype* proto, bool ignore_combat /* = false */, bool skip_2h_check /* = false */)
{
    uint32 type = proto->InventoryType;
    if(slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END && DstInvSlot == -1)
        if(proto->ContainerSlots == 0)
            return INV_ERR_CANT_SWAP;

    if((slot < INVENTORY_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET) || (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET))
    {
        if (!ignore_combat && m_pOwner->IsInCombat() && (slot < EQUIPMENT_SLOT_MAINHAND || slot > EQUIPMENT_SLOT_RANGED))
            return INV_ERR_NOT_IN_COMBAT;
    
        if( proto->ItemLimitCategory )
        {
            if(ItemLimitCategoryEntry * il = dbcItemLimitCategory.LookupEntry( proto->ItemLimitCategory ))
                if( GetEquippedItemCountWithLimitId( proto->ItemLimitCategory ) >= il->MaxAmount )
                    return INV_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED_IS;
        }

        if(IsEquipped(proto->ItemId, SrcSlot))
        {
            if( proto->Unique == 1 ) //how did we end up here? We shouldn't have 2 of these...
                return INV_ERR_ITEM_MAX_COUNT;

            if( proto->Flags & DBC_ITEMFLAG_UNIQUE_EQUIPPED)
                return INV_ERR_ITEM_MAX_COUNT;
        }

        // Check to see if we have the correct race
        if(proto->AllowableRace > 0 && !(proto->AllowableRace & m_pOwner->getRaceMask()))
            return INV_ERR_CANT_EQUIP_EVER;

        // Check to see if we have the correct class
        if(proto->AllowableClass > 0 && !(proto->AllowableClass & m_pOwner->getClassMask()))
            return INV_ERR_CANT_EQUIP_EVER_2;

        // Check to see if we have the reqs for that reputation
        if(proto->RequiredFaction)
        {
            Standing current_standing = m_pOwner->GetFactionInterface()->GetStandingRank(proto->RequiredFaction);
            if(current_standing < (Standing)proto->RequiredFactionStanding)    // Not enough rep rankage..
                return INV_ERR_CANT_EQUIP_REPUTATION;
        }

        // Check to see if we have the correct level.
        if(proto->RequiredLevel > 0 && proto->RequiredLevel > m_pOwner->GetUInt32Value(UNIT_FIELD_LEVEL))
            return INV_ERR_PURCHASE_LEVEL_TOO_LOW;

        // It's kind of hacky, but only check proficiency when we're inworld
        if(m_pOwner->IsInWorld())
        {
            if(proto->Class == ITEM_CLASS_ARMOR && !(m_pOwner->GetArmorProficiency()&(((uint32)(1))<<proto->SubClass)))
                return INV_ERR_PROFICIENCY_NEEDED;
            else if(proto->Class == ITEM_CLASS_WEAPON && !(m_pOwner->GetWeaponProficiency()&((uint32(1))<<proto->SubClass)))
                return INV_ERR_PROFICIENCY_NEEDED;
        }

        if (proto->RequiredSkill > 0 && !((uint32)proto->RequiredSkillRank > m_pOwner->getSkillLineVal(proto->RequiredSkill,true)))
            return INV_ERR_CANT_EQUIP_SKILL;

        if (proto->RequiredSpell && !m_pOwner->HasSpell(proto->RequiredSpell))
            return INV_ERR_PROFICIENCY_NEEDED;

                // You are dead !
        if(m_pOwner->getDeathState() != ALIVE)
            return INV_ERR_PLAYER_DEAD;
    }

    switch(slot)
    {
    case EQUIPMENT_SLOT_HEAD: return type == INVTYPE_HEAD ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_NECK: return type == INVTYPE_NECK ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_SHOULDERS: return type == INVTYPE_SHOULDERS ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_BODY: return type == INVTYPE_BODY ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_CHEST: return (type == INVTYPE_CHEST || type == INVTYPE_ROBE) ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_WAIST: return type == INVTYPE_WAIST ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_LEGS: return type == INVTYPE_LEGS ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_FEET: return type == INVTYPE_FEET ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_WRISTS: return type == INVTYPE_WRISTS ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_HANDS: return type == INVTYPE_HANDS ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_FINGER1:
    case EQUIPMENT_SLOT_FINGER2: return type == INVTYPE_FINGER ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_TRINKET1:
    case EQUIPMENT_SLOT_TRINKET2: return type == INVTYPE_TRINKET ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_BACK: return type == INVTYPE_CLOAK ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_MAINHAND:
        {
            if(proto->Class == ITEM_CLASS_WEAPON && proto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM)
                return 0;
            if(type == INVTYPE_WEAPON || type == INVTYPE_WEAPONMAINHAND || (type == INVTYPE_2HWEAPON && (!GetInventoryItem(EQUIPMENT_SLOT_OFFHAND) || skip_2h_check || m_pOwner->HasSpellWithEffect(SPELL_EFFECT_TITAN_GRIP))))
                return 0;
            return INV_ERR_WRONG_SLOT;
        }break;
    case EQUIPMENT_SLOT_OFFHAND:
        {
            if(proto->Class == ITEM_CLASS_WEAPON && proto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM)
                return INV_ERR_WRONG_SLOT;
            if( m_pOwner->HasSpellWithEffect(SPELL_EFFECT_TITAN_GRIP) && (type == INVTYPE_2HWEAPON || type == INVTYPE_WEAPON || type == INVTYPE_WEAPONOFFHAND || type == INVTYPE_SHIELD))
                return 0;   // Titan's Grip

            if(type == INVTYPE_WEAPON || type == INVTYPE_WEAPONOFFHAND)
            {
                if(Item* mainweapon = GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)) //item exists
                {
                    if(mainweapon->GetProto() && mainweapon->GetProto()->InventoryType != INVTYPE_2HWEAPON)
                        return m_pOwner->HasSkillLine(SKILL_DUAL_WIELD) ? 0 : INV_ERR_2HSKILLNOTFOUND;
                    return skip_2h_check ? 0 : INV_ERR_2HANDED_EQUIPPED;
                } else if(m_pOwner->HasSkillLine(SKILL_DUAL_WIELD))
                    return 0;
                return INV_ERR_2HSKILLNOTFOUND;
            }
            else if(type == INVTYPE_SHIELD || type == INVTYPE_HOLDABLE)
            {
                if(Item* mainweapon = GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)) //item exists
                {
                    if(mainweapon->GetProto() && mainweapon->GetProto()->InventoryType != INVTYPE_2HWEAPON)\
                        return 0;
                    return skip_2h_check ? 0 : INV_ERR_2HANDED_EQUIPPED;
                }
                return 0;
            }
            return INV_ERR_WRONG_SLOT;
        }break;
    case EQUIPMENT_SLOT_RANGED: return (type == INVTYPE_RANGED || type == INVTYPE_THROWN || type == INVTYPE_RANGEDRIGHT || type == INVTYPE_RELIC) ? 0 : INV_ERR_WRONG_SLOT;
    case EQUIPMENT_SLOT_TABARD: return type == INVTYPE_TABARD ? 0 : INV_ERR_WRONG_SLOT;
    case INVENTORY_SLOT_BAG_1:
    case INVENTORY_SLOT_BAG_2:
    case INVENTORY_SLOT_BAG_3:
    case INVENTORY_SLOT_BAG_4:
        {
            //this chunk of code will limit you to equip only 1 Ammo Bag. Later i found out that this is not blizzlike so i will remove it when it's blizzlike
            //we are trying to equip an Ammo Bag
            if(proto->Class == ITEM_CLASS_QUIVER)
            {
                //check if we already have an AB equiped
                FindAmmoBag();

                //we do have amo bag but we are not swaping them then we send error
                if(result.Slot != ITEM_NO_SLOT_AVAILABLE && result.Slot != slot)
                    return INV_ERR_ONLY_ONE_AMMO;
            }

            if(Item *item = GetInventoryItem(INVENTORY_SLOT_NOT_SET, slot))
            {
                if(item->GetProto()->BagFamily)
                {
                    if((IsBagSlot(slot) && DstInvSlot == INVENTORY_SLOT_NOT_SET))
                        if(proto->InventoryType == INVTYPE_BAG )
                            return 0;

                    if(proto->BagFamily == GetInventoryItem(INVENTORY_SLOT_NOT_SET,slot)->GetProto()->BagFamily)
                        return 0;
                    return INV_ERR_WRONG_BAG_TYPE;
                }
                return 0;
            } else if(type == INVTYPE_BAG)
                return 0;
            return INV_ERR_NOT_A_BAG;
        }break;
    case BANK_SLOT_BAG_1:
    case BANK_SLOT_BAG_2:
    case BANK_SLOT_BAG_3:
    case BANK_SLOT_BAG_4:
    case BANK_SLOT_BAG_5:
    case BANK_SLOT_BAG_6:
    case BANK_SLOT_BAG_7:
        {
            int32 bytes,slots;
            int8 islot;

            if(!GetInventoryItem(INVENTORY_SLOT_NOT_SET, slot))
            {
                //check if player got that slot.
                bytes = m_pOwner->GetUInt32Value(PLAYER_BYTES_2);
                slots =(uint8) (bytes >> 16);
                islot = GetInternalBankSlotFromPlayer(slot);
                if(slots < islot)
                    return INV_ERR_NO_BANK_SLOT;

                //in case bank slot exists, check if player can put the item there
                if(type == INVTYPE_BAG)
                    return 0;
                return INV_ERR_NOT_A_BAG;
            }

            if(GetInventoryItem(INVENTORY_SLOT_NOT_SET, slot)->GetProto()->BagFamily)
            {
                if((IsBagSlot(slot) && DstInvSlot == INVENTORY_SLOT_NOT_SET))
                    if(proto->InventoryType == INVTYPE_BAG )
                        return 0;

                if(proto->BagFamily == GetInventoryItem(INVENTORY_SLOT_NOT_SET,slot)->GetProto()->BagFamily)
                    return 0;
                return INV_ERR_WRONG_BAG_TYPE;
            }
            return 0;
        }break;
    }
    return 0;
}

//-------------------------------------------------------------------//
//Description: Checks if player can receive the item
//-------------------------------------------------------------------//
int8 PlayerInventory::CanReceiveItem(ItemPrototype * item, uint32 amount, ItemExtendedCostEntry *ec)
{
    if(item == NULL)
        return 0;

    if(item->Unique > 0)
    {
        int32 count = GetItemCount(item->ItemId, true);
        if(count == item->Unique || ((count + amount) > (uint32)item->Unique))
            return INV_ERR_ITEM_MAX_COUNT;
    }

    if(amount > item->MaxCount)
    { // Check if we have enough slots
        uint32 requiredStackSpace = ceil(((float)amount)/((float)item->MaxCount));
        if(!countDownFreeSlots(item, requiredStackSpace))
            return INV_ERR_BAG_FULL;
    }
    else
    {
        SlotResult result = FindFreeInventorySlot(item);
        if(result.Result == false)
            return INV_ERR_BAG_FULL;
    }

    return 0;
}

void PlayerInventory::BuyItem(ItemPrototype *item, uint32 total_amount, Creature* pVendor, ItemExtendedCostEntry *ec)
{
    if(item->BuyPrice)
    {
        if(uint64 itemprice = sItemMgr.CalculateBuyPrice(item->ItemId, total_amount, m_pOwner, pVendor, ec))
        {
            uint64 coinage = m_pOwner->GetUInt64Value(PLAYER_FIELD_COINAGE);
            if(itemprice >= coinage)
                coinage = 0;
            else coinage -= itemprice;
            m_pOwner->SetUInt64Value(PLAYER_FIELD_COINAGE, coinage);
        }
    }

    if( ec != NULL )
    {
        for(uint8 i = 0;i<5;++i)
        {
            if(ec->reqItem[i])
                m_pOwner->GetInventory()->RemoveItemAmt( ec->reqItem[i], total_amount * ec->reqItemCount[i] );
            if(ec->reqCurrency[i])
                m_pOwner->GetCurrency()->RemoveCurrency(ec->reqCurrency[i], ec->reqCurrencyCount[i]);
        }
    }
}

enum CanAffordItem
{
    CAN_AFFORD_ITEM_ERROR_OK                        = 0,
    CAN_AFFORD_ITEM_ERROR_CURRENTLY_SOLD_OUT        = 1,
    CAN_AFFORD_ITEM_ERROR_DONT_HAVE_ENOUGH_MONEY    = 2,
    CAN_AFFORD_ITEM_ERROR_NOT_FOUND                 = 3,
    CAN_AFFORD_ITEM_ERROR_DOESNT_LIKE_YOU           = 4,
    CAN_AFFORD_ITEM_ERROR_TOO_FAR_AWAY              = 5,
    CAN_AFFORD_ITEM_ERROR_NO_MESSAGE                = 6,
    CAN_AFFORD_ITEM_ERROR_CANT_CARRY_ANY_MORE       = 8,
    CAN_AFFORD_ITEM_ERROR_NOT_REQUIRED_RANK         = 11,
    CAN_AFFORD_ITEM_ERROR_REPUTATION                = 12,
};

int8 PlayerInventory::CanAffordItem(ItemPrototype * item,uint32 amount, Creature* pVendor, ItemExtendedCostEntry *ec)
{
    if(item->RequiredFaction)
    {
        FactionEntry *faction = dbcFaction.LookupEntry(item->RequiredFaction);
        if(!faction || faction->RepListIndex < 0)
            return CAN_AFFORD_ITEM_ERROR_NO_MESSAGE;

        if( Player::GetReputationRankFromStanding( m_pOwner->GetFactionInterface()->GetStanding( item->RequiredFaction )) < item->RequiredFactionStanding )
            return CAN_AFFORD_ITEM_ERROR_REPUTATION;
    }

    if(ec && ec->reqPersonalRating && m_pOwner->GetMaxPersonalRating(ec->reqArenaSlot > 0) < ec->reqPersonalRating)
        return CAN_AFFORD_ITEM_ERROR_NOT_REQUIRED_RANK;

    if( ec != NULL )
    {
        for(uint8 i = 0; i < 5; ++i)
        {
            if(ec->reqItem[i] && (m_pOwner->GetInventory()->GetItemCount(ec->reqItem[i], false) < (ec->reqItemCount[i]*amount)))
                return CAN_AFFORD_ITEM_ERROR_DONT_HAVE_ENOUGH_MONEY;
            if(ec->reqCurrency[i])
            {
                if(i < 4 && ec->reqCurrency[i] == ec->reqCurrency[i+1])
                {
                    if(!m_pOwner->GetCurrency()->HasTotalCurrency(ec->reqCurrency[i], ec->reqCurrencyCount[i+1]))
                        return CAN_AFFORD_ITEM_ERROR_NOT_REQUIRED_RANK;
                    else
                    {
                        if(!m_pOwner->GetCurrency()->HasCurrency(ec->reqCurrency[i], ec->reqCurrencyCount[i]))
                            return CAN_AFFORD_ITEM_ERROR_DONT_HAVE_ENOUGH_MONEY;
                        // Iterate index after we check if we have enough currency
                        ++i;
                    }
                } else if(!m_pOwner->GetCurrency()->HasCurrency(ec->reqCurrency[i], ec->reqCurrencyCount[i]))
                    return CAN_AFFORD_ITEM_ERROR_DONT_HAVE_ENOUGH_MONEY;
            }
        }
    }

    if(item->BuyPrice)
    {
        if(uint64 price = sItemMgr.CalculateBuyPrice(item->ItemId, amount, m_pOwner, pVendor, ec))
            if(m_pOwner->GetUInt64Value(PLAYER_FIELD_COINAGE) < price)
                return CAN_AFFORD_ITEM_ERROR_DONT_HAVE_ENOUGH_MONEY;
    }
    return 0;
}

//-------------------------------------------------------------------//
//Description: Gets the Item slot by item type
//-------------------------------------------------------------------//
int8 PlayerInventory::GetItemSlotByType(ItemPrototype* proto)
{
    switch(proto->InventoryType)
    {
    case INVTYPE_NON_EQUIP:
        return ITEM_NO_SLOT_AVAILABLE;
    case INVTYPE_HEAD:
            return EQUIPMENT_SLOT_HEAD;
    case INVTYPE_NECK:
            return EQUIPMENT_SLOT_NECK;
    case INVTYPE_SHOULDERS:
            return EQUIPMENT_SLOT_SHOULDERS;
    case INVTYPE_BODY:
            return EQUIPMENT_SLOT_BODY;
    case INVTYPE_CHEST:
            return EQUIPMENT_SLOT_CHEST;
    case INVTYPE_ROBE: // ???
            return EQUIPMENT_SLOT_CHEST;
    case INVTYPE_WAIST:
            return EQUIPMENT_SLOT_WAIST;
    case INVTYPE_LEGS:
            return EQUIPMENT_SLOT_LEGS;
    case INVTYPE_FEET:
            return EQUIPMENT_SLOT_FEET;
    case INVTYPE_WRISTS:
            return EQUIPMENT_SLOT_WRISTS;
    case INVTYPE_HANDS:
            return EQUIPMENT_SLOT_HANDS;
    case INVTYPE_FINGER:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_FINGER1))
                return EQUIPMENT_SLOT_FINGER1;
            else if (!GetInventoryItem(EQUIPMENT_SLOT_FINGER2))
                return EQUIPMENT_SLOT_FINGER2;
            return EQUIPMENT_SLOT_FINGER1; //auto equips always in finger 1
        }
    case INVTYPE_TRINKET:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_TRINKET1))
                return EQUIPMENT_SLOT_TRINKET1;
            else if (!GetInventoryItem(EQUIPMENT_SLOT_TRINKET2))
                return EQUIPMENT_SLOT_TRINKET2;
            return EQUIPMENT_SLOT_TRINKET1; //auto equips always on trinket 1
        }
    case INVTYPE_CLOAK:
            return EQUIPMENT_SLOT_BACK;
    case INVTYPE_WEAPON:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_MAINHAND) )
                return EQUIPMENT_SLOT_MAINHAND;
            else if(!GetInventoryItem(EQUIPMENT_SLOT_OFFHAND))
                return EQUIPMENT_SLOT_OFFHAND;
            return EQUIPMENT_SLOT_MAINHAND;
        }
    case INVTYPE_SHIELD:
            return EQUIPMENT_SLOT_OFFHAND;
    case INVTYPE_RANGED:
            return EQUIPMENT_SLOT_RANGED;
    case INVTYPE_2HWEAPON:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
                return EQUIPMENT_SLOT_MAINHAND;
            else if(!GetInventoryItem(EQUIPMENT_SLOT_OFFHAND))
                return EQUIPMENT_SLOT_OFFHAND;
            return EQUIPMENT_SLOT_MAINHAND;
        }
    case INVTYPE_TABARD:
            return EQUIPMENT_SLOT_TABARD;
    case INVTYPE_WEAPONMAINHAND:
            return EQUIPMENT_SLOT_MAINHAND;
    case INVTYPE_WEAPONOFFHAND:
            return EQUIPMENT_SLOT_OFFHAND;
    case INVTYPE_HOLDABLE:
            return EQUIPMENT_SLOT_OFFHAND;
    case INVTYPE_THROWN:
        return EQUIPMENT_SLOT_RANGED;
    case INVTYPE_RANGEDRIGHT:
        return EQUIPMENT_SLOT_RANGED;
    case INVTYPE_RELIC:
        return EQUIPMENT_SLOT_RANGED;
    case INVTYPE_BAG:
        {
            for (int8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            {
                if (!GetInventoryItem(i))
                    return i;
            }
            return ITEM_NO_SLOT_AVAILABLE; //bags are not suposed to be auto-equiped when slots are not free
        }
    default:
        return ITEM_NO_SLOT_AVAILABLE;
    }
}

//-------------------------------------------------------------------//
//Description: Gets a Item by guid
//-------------------------------------------------------------------//
Item* PlayerInventory::GetItemByGUID(uint64 Guid)
{
    uint32 i ;

    //EQUIPMENT
    for(i=EQUIPMENT_SLOT_START;i<EQUIPMENT_SLOT_END;++i)
    {
        if(m_pItems[i] != 0)
        {
            if( m_pItems[i]->GetGUID() == Guid)
            {
                result.ContainerSlot = INVALID_BACKPACK_SLOT;//not a containerslot. In 1.8 client marked wrong slot like this
                result.Slot = i;
                return m_pItems[i];
            }
        }
    }

    //INVENTORY BAGS
    for(i=INVENTORY_SLOT_BAG_START;i<INVENTORY_SLOT_BAG_END;++i)
    {
        if(m_pItems[i] != NULL && m_pItems[i]->IsContainer())
        {
            if(m_pItems[i]->GetGUID()==Guid)
            {
                result.ContainerSlot = INVALID_BACKPACK_SLOT;
                result.Slot = i;
                return m_pItems[i];
            }

            for (int32 j =0; j < m_pItems[i]->GetProto()->ContainerSlots;j++)
            {
                Item* item2 = castPtr<Container>(m_pItems[i])->GetItem(j);
                if (item2)
                {
                    if (item2->GetGUID() == Guid)
                    {
                        result.ContainerSlot = i;
                        result.Slot = j;
                        return item2;
                    }
                }
            }
        }
    }

    //INVENTORY
    for(i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;++i)
    {
        if(m_pItems[i] != 0)
        {
            if(m_pItems[i]->GetGUID() == Guid)
            {
                result.ContainerSlot = INVALID_BACKPACK_SLOT;
                result.Slot = i;
                return m_pItems[i];
            }
        }
    }

    return NULL;
}

//-------------------------------------------------------------------//
//Description: Inventory Error report
//-------------------------------------------------------------------//
void PlayerInventory::BuildInventoryChangeError(Item* SrcItem, Item* DstItem, uint8 Error, WoWGuid srcGuidRep, WoWGuid dstGuidRep)
{
    WorldPacket data(SMSG_INVENTORY_CHANGE_FAILURE, 22);
    data << Error;
    if (Error != INV_ERR_OK)
    {
        data << uint64(SrcItem ? SrcItem->GetGUID() : srcGuidRep);
        data << uint64(DstItem ? DstItem->GetGUID() : dstGuidRep);
        data << uint8(0);
        switch(Error)
        {
        case INV_ERR_PURCHASE_LEVEL_TOO_LOW:
            data << uint32(SrcItem ? SrcItem->GetProto()->RequiredLevel : 0);
            break;
        case INV_ERR_NO_OUTPUT:
            data << uint64(0) << uint32(0) << uint64(0);
            break;
        case INV_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED_IS:
        case INV_ERR_ITEM_MAX_LIMIT_CATEGORY_SOCKETED_EXCEEDED_IS:
        case INV_ERR_ITEM_MAX_LIMIT_CATEGORY_EQUIPPED_EXCEEDED_IS:
            data << uint32(SrcItem ? SrcItem->GetProto()->ItemLimitCategory : 0);
            break;
        }
    }

    m_pOwner->PushPacket( &data );
}

void PlayerInventory::EmptyBuyBack()
{
    for (uint32 j = 0;j < 12;++j)
    {
        if (m_pBuyBack[j] != NULL)
        {
             m_pBuyBack[j]->DestroyForPlayer(m_pOwner);
             m_pBuyBack[j]->DeleteFromDB();

            if(m_pBuyBack[j]->IsContainer())
            {
                if (castPtr<Container>(m_pBuyBack[j])->IsInWorld())
                    castPtr<Container>(m_pBuyBack[j])->RemoveFromWorld();

                castPtr<Container>(m_pBuyBack[j])->Destruct();
            }
            else
            {
                if (m_pBuyBack[j]->IsInWorld())
                    m_pBuyBack[j]->RemoveFromWorld();

                m_pBuyBack[j]->Destruct();
            }

            m_pOwner->SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2*j),0);
            m_pOwner->SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + j,0);
            m_pOwner->SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j,0);
            m_pBuyBack[j] = NULL;
        }
        else
        {
            break;
        }
    }
}

void PlayerInventory::AddBuyBackItem(Item* it,uint32 price)
{
    int i;
    if ((m_pBuyBack[11] != NULL) && (m_pOwner->GetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + 22) != 0))
    {
        if(m_pBuyBack[0] != NULL)
        {
             m_pBuyBack[0]->DestroyForPlayer(m_pOwner);
             m_pBuyBack[0]->DeleteFromDB();

             if(m_pBuyBack[0]->IsContainer())
             {
                if (castPtr<Container>(m_pBuyBack[0])->IsInWorld())
                    castPtr<Container>(m_pBuyBack[0])->RemoveFromWorld();

                castPtr<Container>(m_pBuyBack[0])->Destruct();
                m_pBuyBack[0] = NULL;
             }
             else
             {
                if (m_pBuyBack[0]->IsInWorld())
                    m_pBuyBack[0]->RemoveFromWorld();

                m_pBuyBack[0]->Destruct();
                m_pBuyBack[0] = NULL;
             }
        }

        for (int j = 0;j < 11;j++)
        {
            //SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2*j),buyback[j+1]->GetGUID());
            m_pOwner->SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2*j),m_pOwner->GetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ((j+1)*2) ) );
            m_pOwner->SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + j,m_pOwner->GetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + j+1));
            m_pOwner->SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j,m_pOwner->GetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j+1));
            m_pBuyBack[j] = m_pBuyBack[j+1];
        }
        m_pBuyBack[11] = it;

        m_pOwner->SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2*(11)),m_pBuyBack[11]->GetGUID());
        m_pOwner->SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + 11,price);
        m_pOwner->SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + 11,(uint32)UNIXTIME);
        return;
    }

    for(i=0; i < 24;i+=2)
    {
        if((m_pOwner->GetUInt32Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + i) == 0) || (m_pBuyBack[i/2] == NULL))
        {
            sLog.outDebug("setting buybackslot %u\n",i/2);
            m_pBuyBack[i >> 1] = it;

            m_pOwner->SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + i,m_pBuyBack[i >> 1]->GetGUID());
            //SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + i,it->GetGUID());
            m_pOwner->SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + (i >> 1),price);
            m_pOwner->SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + (i >> 1),(uint32)UNIXTIME);
            return;
        }
    }
}

void PlayerInventory::RemoveBuyBackItem(uint32 index)
{
    int32 j = 0;
    for( j = index;j < 11; j++ )
    {
        if (m_pOwner->GetUInt64Value( PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ( j * 2 ) ) != 0 )
        {
            m_pOwner->SetUInt64Value( PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ( 2 * j ), m_pOwner->GetUInt64Value( PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ( ( j + 1 ) * 2 ) ) );
            m_pOwner->SetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + j, m_pOwner->GetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + j+1));
            m_pOwner->SetUInt32Value( PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j, m_pOwner->GetUInt32Value( PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j + 1 ) );

            if( m_pBuyBack[j+1] != NULL && ( m_pOwner->GetUInt64Value( PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ( ( j + 1 ) * 2 ) ) != 0 ) )
            {
                m_pBuyBack[j] = m_pBuyBack[j+1];
            }
            else
            {
                if( m_pBuyBack[j] != NULL )
                {
                        m_pBuyBack[j] = NULL;
                }

                sLog.outDebug( "nulling %u\n", j );
            }
        }
        else
            return;
    }
    j = 11;
    m_pOwner->SetUInt64Value( PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ( 2 * j ), m_pOwner->GetUInt64Value( PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ( ( j + 1 ) * 2 ) ) );
    m_pOwner->SetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + j, m_pOwner->GetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + j + 1 ) );
    m_pOwner->SetUInt32Value( PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j, m_pOwner->GetUInt32Value( PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j + 1 ) );
    if( m_pBuyBack[11] )
    {
        m_pBuyBack[11] = NULL;
    }

}

//-------------------------------------------------------------------//
//Description: swap inventory slots
//-------------------------------------------------------------------//
bool PlayerInventory::SwapItemSlots(int16 srcslot, int16 dstslot)
{
    // srcslot and dstslot are int... NULL might not be an int depending on arch where it is compiled
    if( srcslot >= MAX_INVENTORY_SLOT || srcslot < 0 )
        return false;

    if( dstslot >= MAX_INVENTORY_SLOT || dstslot < 0 )
        return false;

    Item* SrcItem = GetInventoryItem( srcslot );
    Item* DstItem = GetInventoryItem( dstslot );

    sLog.Debug( "PlayerInventory","SwapItemSlots(%u, %u);" , srcslot , dstslot );

    // Force GM robes on all GM's execpt 'az' status, if set in world config
    if(m_pOwner->GetSession()->HasGMPermissions() && sWorld.gm_force_robes )
    {
        if( strchr(m_pOwner->GetSession()->GetPermissions(), 'z') == NULL)
        {
            //don't allow anything else then these items
            if( dstslot < INVENTORY_SLOT_BAG_END )
            {
                if( !(SrcItem->GetUInt32Value(OBJECT_FIELD_ENTRY) == 12064 ||
                    SrcItem->GetUInt32Value(OBJECT_FIELD_ENTRY) == 2586 ||
                    SrcItem->GetUInt32Value(OBJECT_FIELD_ENTRY) == 11508 ))
                {
                    AddItemToFreeSlot(SrcItem);
                    return true;
                }
            }
        }
    }

    if( SrcItem != NULL && DstItem != NULL && SrcItem->GetEntry()==DstItem->GetEntry() && (SrcItem->GetProto()->MaxCount < 0 || SrcItem->GetProto()->MaxCount > 1) )
    {
        uint32 total = SrcItem->GetUInt32Value( ITEM_FIELD_STACK_COUNT ) + DstItem->GetUInt32Value( ITEM_FIELD_STACK_COUNT );
        if( total <= (uint32)DstItem->GetProto()->MaxCount || DstItem->GetProto()->MaxCount < 0 )
        {
            DstItem->ModUnsigned32Value( ITEM_FIELD_STACK_COUNT, SrcItem->GetUInt32Value( ITEM_FIELD_STACK_COUNT ) );
            SafeFullRemoveItemFromSlot( INVENTORY_SLOT_NOT_SET, srcslot );
            DstItem->m_isDirty = true;
            return true;
        }
        else
        {
            if(!(DstItem->GetUInt32Value( ITEM_FIELD_STACK_COUNT ) == DstItem->GetProto()->MaxCount))
            {
                int32 delta=DstItem->GetProto()->MaxCount-DstItem->GetUInt32Value( ITEM_FIELD_STACK_COUNT );
                DstItem->SetUInt32Value( ITEM_FIELD_STACK_COUNT, DstItem->GetProto()->MaxCount );
                SrcItem->ModUnsigned32Value( ITEM_FIELD_STACK_COUNT, -delta );
                SrcItem->m_isDirty = true;
                DstItem->m_isDirty = true;
                return true;
            }
        }
    }

    //src item was equiped previously
    if( srcslot < INVENTORY_SLOT_BAG_END )
    {
        if( m_pItems[srcslot] != NULL )
            m_pOwner->ApplyItemMods( m_pItems[srcslot], srcslot, false );
    }

    //dst item was equiped previously
    if( dstslot < INVENTORY_SLOT_BAG_END )
    {
        if( m_pItems[dstslot] != NULL )
            m_pOwner->ApplyItemMods( m_pItems[dstslot], dstslot, false );
    }

    //sLog.outDebug( "Putting items into slots..." );



    m_pItems[dstslot] = SrcItem;

    // Moving a bag with items to a empty bagslot
    if ( (DstItem == NULL) && (SrcItem->IsContainer()) )
    {
        Item* tSrcItem = NULL;

        for ( int32 Slot = 0; Slot < SrcItem->GetProto()->ContainerSlots; Slot++ )
        {
            tSrcItem = (castPtr<Container>(m_pItems[srcslot]))->GetItem(Slot);

            m_pOwner->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(srcslot, Slot, false);

            if ( tSrcItem != NULL )
            {
                m_pOwner->GetInventory()->SafeAddItem(tSrcItem, dstslot, Slot);
            }
        }
    }

    m_pItems[srcslot] = DstItem;

    // swapping 2 bags filled with items
    if ( DstItem && SrcItem->IsContainer() && DstItem->IsContainer() )
    {
        Item* tDstItem = NULL;
        Item* tSrcItem = NULL;
        uint32 TotalSlots = 0;

        // Determine the max amount of slots to swap
        if ( SrcItem->GetProto()->ContainerSlots <= DstItem->GetProto()->ContainerSlots )
            TotalSlots = SrcItem->GetProto()->ContainerSlots;
        else TotalSlots = DstItem->GetProto()->ContainerSlots;

        // swap items in the bags
        for( uint32 Slot = 0; Slot < TotalSlots; Slot++ )
        {
            tSrcItem = (castPtr<Container>(m_pItems[srcslot]))->GetItem(Slot);
            tDstItem = (castPtr<Container>(m_pItems[dstslot]))->GetItem(Slot);

            if( tSrcItem != NULL )
                m_pOwner->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(srcslot, Slot, false);
            if( tDstItem != NULL )
                m_pOwner->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(dstslot, Slot, false);

            if( tSrcItem != NULL )
                (castPtr<Container>(DstItem))->AddItem(Slot, tSrcItem);
            if( tDstItem != NULL )
                (castPtr<Container>(SrcItem))->AddItem(Slot, tDstItem);
        }
    }

    if( DstItem != NULL )
        DstItem->m_isDirty = true;
    if( SrcItem != NULL )
        SrcItem->m_isDirty = true;

    if( m_pItems[dstslot] != NULL )
    {
        //sLog.outDebug( "(SrcItem) PLAYER_FIELD_INV_SLOT_HEAD + %u is now %u" , dstslot * 2 , m_pItems[dstslot]->GetGUID() );
        m_pOwner->SetUInt64Value( PLAYER_FIELD_INV_SLOT_HEAD + (dstslot*2),  m_pItems[dstslot]->GetGUID() );
    }
    else
    {
        //sLog.outDebug( "(SrcItem) PLAYER_FIELD_INV_SLOT_HEAD + %u is now 0" , dstslot * 2 );
        m_pOwner->SetUInt64Value( PLAYER_FIELD_INV_SLOT_HEAD + (dstslot*2), 0 );
    }

    if( m_pItems[srcslot] != NULL )
    {
        //sLog.outDebug( "(DstItem) PLAYER_FIELD_INV_SLOT_HEAD + %u is now %u" , dstslot * 2 , m_pItems[srcslot]->GetGUID() );
        m_pOwner->SetUInt64Value( PLAYER_FIELD_INV_SLOT_HEAD + (srcslot*2), m_pItems[srcslot]->GetGUID() );
    }
    else
    {
        //sLog.outDebug( "(DstItem) PLAYER_FIELD_INV_SLOT_HEAD + %u is now 0" , dstslot * 2 );
        m_pOwner->SetUInt64Value( PLAYER_FIELD_INV_SLOT_HEAD + (srcslot*2), 0 );
    }

    if( srcslot < INVENTORY_SLOT_BAG_END && m_pItems[srcslot] )  // source item is equiped
        m_pItems[srcslot]->Bind(ITEM_BIND_ON_EQUIP);

    if( dstslot < INVENTORY_SLOT_BAG_END && m_pItems[dstslot] )   // source item is inside inventory
        m_pItems[dstslot]->Bind(ITEM_BIND_ON_EQUIP);

    //src item is equiped now
    if( srcslot < INVENTORY_SLOT_BAG_END && m_pItems[srcslot] != NULL )
    {
        m_pOwner->ApplyItemMods( m_pItems[srcslot], srcslot, true );
        AchieveMgr.UpdateCriteriaValue(m_pOwner, ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, GetEquippedCountByItemID(m_pItems[srcslot]->GetEntry()), m_pItems[srcslot]->GetEntry());
    }

    //dst item is equiped now
    if( dstslot < INVENTORY_SLOT_BAG_END && m_pItems[dstslot] != NULL )
    {
        m_pOwner->ApplyItemMods( m_pItems[dstslot], dstslot, true );
        AchieveMgr.UpdateCriteriaValue(m_pOwner, ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, GetEquippedCountByItemID(m_pItems[dstslot]->GetEntry()), m_pItems[dstslot]->GetEntry());
    }
    return true;
}

//-------------------------------------------------------------------//
//Description: Item Loading
//-------------------------------------------------------------------//
void PlayerInventory::mLoadItemsFromDatabase(QueryResult * inventory, QueryResult *enchants)
{
    if( inventory )
    {
        do
        {
            Field* fields = inventory->Fetch();
            int8 containerslot = fields[15].GetInt8();
            uint8 slot = fields[16].GetUInt8();
            if(ItemPrototype *proto = sItemMgr.LookupEntry(fields[2].GetUInt32()))
            {
                Item* item = (proto->InventoryType == INVTYPE_BAG ? new Container(proto, fields[1].GetUInt32()) : new Item(proto, fields[1].GetUInt32()));
                item->Init();
                item->LoadFromDB( fields );
                if( SafeAddItem( item, containerslot, slot ) )
                    item->m_isDirty = false;
                else item->Destruct();
            }
        } while( inventory->NextRow() );
    }

    if( enchants )
    {
        do
        {
            Field* fields = enchants->Fetch();
            WoWGuid itemGuid = MAKE_NEW_GUID(fields[0].GetUInt32(), fields[1].GetUInt32(), HIGHGUID_TYPE_ITEM);
            if(Item *item = GetItemByGUID(itemGuid))
                item->LoadEnchantment(fields[2].GetUInt8(), fields[3].GetUInt32(), fields[4].GetUInt32(), fields[5].GetUInt32(), fields[6].GetUInt32());
        } while( enchants->NextRow() );
    }
}

//-------------------------------------------------------------------//
//Description: Item saving
//-------------------------------------------------------------------//
void PlayerInventory::mSaveItemsToDatabase(bool first, QueryBuffer * buf)
{
    for( uint32 x = EQUIPMENT_SLOT_START; x < MAX_INVENTORY_SLOT; ++x )
    {
        if( GetInventoryItem( x ) != NULL )
        {
            if( GetInventoryItem( x )->GetProto() != NULL )
            {
                if( !( (GetInventoryItem( x )->GetProto()->Flags) & DBC_ITEMFLAG_CONJURED ) ) // skip conjured item on save
                {
                    if( IsBagSlot( x ) && GetInventoryItem( x )->IsContainer() )
                    {
                        castPtr<Container>( GetInventoryItem( x ) )->SaveBagToDB( x, first, buf );
                    }
                    else
                    {
                        GetInventoryItem( x )->SaveToDB( INVENTORY_SLOT_NOT_SET, x, first, buf );
                    }
                }
            }
        }
    }
}

// Only called at character create
void PlayerInventory::mAddItemToBestSlot(ItemPrototype *proto, uint32 count, bool fromDB)
{
    if(proto == NULL)
        return;

    Item *item = NULL;
    switch(proto->Class)
    {
    case ITEM_CLASS_CONTAINER:
        {
            if(proto->BagFamily)
            {
                for(uint32 slot = INVENTORY_SLOT_BAG_4; slot >= INVENTORY_SLOT_BAG_1; slot--)
                {
                    if(GetInventoryItem(slot) != NULL)
                        continue;
                    if(CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET, INVENTORY_SLOT_NOT_SET, slot, proto, true, false) != INV_ERR_OK)
                        continue;
                    if(SafeAddItem((item ? item : (item=objmgr.CreateItem(proto->ItemId, m_pOwner, count))), INVENTORY_SLOT_NOT_SET, slot ) == ADD_ITEM_RESULT_OK)
                        return;
                }
            }
            else
            {
                for(uint32 slot = INVENTORY_SLOT_BAG_1; slot < INVENTORY_SLOT_BAG_END; slot++)
                {
                    if(GetInventoryItem(slot) != NULL)
                        continue;
                    if(CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET, INVENTORY_SLOT_NOT_SET, slot, proto, true, false) != INV_ERR_OK)
                        continue;
                    if(SafeAddItem((item ? item : (item=objmgr.CreateItem(proto->ItemId, m_pOwner, count))), INVENTORY_SLOT_NOT_SET, slot ) == ADD_ITEM_RESULT_OK)
                        return;
                }
            }
        } break;
    case ITEM_CLASS_WEAPON:
        if(GetInventoryItem(EQUIPMENT_SLOT_MAINHAND) == NULL && SafeAddItem((item ? item : (item=objmgr.CreateItem(proto->ItemId, m_pOwner, count))), INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND ) == ADD_ITEM_RESULT_OK)
            return;
        if(GetInventoryItem(EQUIPMENT_SLOT_OFFHAND) == NULL && SafeAddItem((item ? item : (item=objmgr.CreateItem(proto->ItemId, m_pOwner, count))), INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND ) == ADD_ITEM_RESULT_OK)
            return;
        if(GetInventoryItem(EQUIPMENT_SLOT_RANGED) == NULL && SafeAddItem((item ? item : (item=objmgr.CreateItem(proto->ItemId, m_pOwner, count))), INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_RANGED ) == ADD_ITEM_RESULT_OK)
            return;
        break;
    case ITEM_CLASS_ARMOR:
        {
            for(uint32 slot = EQUIPMENT_SLOT_START; slot <= EQUIPMENT_SLOT_BACK; slot++)
            {
                if(GetInventoryItem(slot) != NULL)
                    continue;
                if(CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET, INVENTORY_SLOT_NOT_SET, slot, proto, true, false) != INV_ERR_OK)
                    continue;
                if(SafeAddItem((item ? item : (item=objmgr.CreateItem(proto->ItemId, m_pOwner, count))), INVENTORY_SLOT_NOT_SET, slot ) == ADD_ITEM_RESULT_OK)
                    return;
            }
        }break;
    }

    // Check for bagfamily first
    if(proto->BagFamily)
    {
        for(uint8 slot = INVENTORY_SLOT_BAG_1; slot < INVENTORY_SLOT_BAG_END; slot++)
        {
            if(Item *bagItem = GetInventoryItem(slot))
            {
                if(!bagItem->IsContainer())
                    continue;
                if(bagItem->GetProto()->BagFamily != proto->BagFamily)
                    continue;

                if(castPtr<Container>(bagItem)->AddItemToFreeSlot((item ? item : (item=objmgr.CreateItem(proto->ItemId, m_pOwner, count))), NULL))
                    return;
            }
        }
    }

    for(uint32 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
    {
        if(GetInventoryItem(slot) != NULL)
            continue;
        if(SafeAddItem((item ? item : (item=objmgr.CreateItem(proto->ItemId, m_pOwner, count))), INVENTORY_SLOT_NOT_SET, slot ) == ADD_ITEM_RESULT_OK)
            return;
    }

    for(uint8 slot = INVENTORY_SLOT_BAG_1; slot < INVENTORY_SLOT_BAG_END; slot++)
    {
        if(Item *bagItem = GetInventoryItem(slot))
        {
            if(!bagItem->IsContainer())
                continue;
            if(bagItem->GetProto()->BagFamily != proto->BagFamily)
                continue;
            if(castPtr<Container>(bagItem)->AddItemToFreeSlot((item ? item : (item=objmgr.CreateItem(proto->ItemId, m_pOwner, count))), NULL))
                return;
        }
    }

    if(item != NULL)
        delete item;
}

void PlayerInventory::ModifyLevelBasedItemBonuses(bool apply)
{
    for( uint32 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x )
    {
        if(ItemPrototype *proto = GetInventoryItem(x) ? GetInventoryItem(x)->GetProto() : NULL)
        {
            if(proto->ScalingStatDistribution == 0)
                continue;
            m_pOwner->ApplyItemMods(GetInventoryItem(x), x, apply);
        }
    }
}

bool PlayerInventory::FillItemLevelData(uint32 &itemCount, uint32 &totalItemLevel, bool inventoryOnly)
{
    ItemPrototype *proto = NULL;
    std::map<uint32, uint32> inventoryItemLevels;
    for( uint32 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x )
    {
        if(x == EQUIPMENT_SLOT_BODY)
            continue;
        if(x == EQUIPMENT_SLOT_OFFHAND)
            continue;
        if(x == EQUIPMENT_SLOT_RANGED)
            continue;
        if(x == EQUIPMENT_SLOT_TABARD)
            continue;

        if( m_pItems[x] && (proto = m_pItems[x]->GetProto()) )
        {
            inventoryItemLevels[x] = proto->ItemLevel;
        }
    }

    if(inventoryOnly || true)
    {
        if(!inventoryItemLevels.empty())
        {
            itemCount = inventoryItemLevels.size();
            while(!inventoryItemLevels.empty())
            {
                totalItemLevel += inventoryItemLevels.begin()->second;
                inventoryItemLevels.erase(inventoryItemLevels.begin());
            }
            return true;
        }
        return false;
    }

}

AddItemResult PlayerInventory::AddItemToFreeBankSlot(Item* item)
{
    //special items first
    for( uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++ )
    {
        if( m_pItems[i] != NULL )
        {
            if( m_pItems[i]->GetProto()->BagFamily == item->GetProto()->BagFamily )
            {
                if( m_pItems[i]->IsContainer() )
                {
                    bool result = castPtr<Container>( m_pItems[i] )->AddItemToFreeSlot( item, NULL );
                    if( result )
                        return ADD_ITEM_RESULT_OK;
                }
            }
        }
    }

    for( uint32 i= BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++ )
    {
        if( m_pItems[i] == NULL )
        {
            return SafeAddItem( item, INVENTORY_SLOT_NOT_SET, i );
        }
    }

    for( uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++ )
    {
        if( m_pItems[i] != NULL && m_pItems[i]->GetProto()->BagFamily == 0 && m_pItems[i]->IsContainer() ) //special bags ignored
        {
            for( int32 j =0; j < m_pItems[i]->GetProto()->ContainerSlots; j++ )
            {
                Item* item2 = castPtr<Container>( m_pItems[i] )->GetItem(j);
                if( item2 == NULL )
                {
                    return SafeAddItem( item, i, j );
                }
            }
        }
    }
    return ADD_ITEM_RESULT_ERROR;
}

int16 PlayerInventory::FindSpecialBag(Item* item)
{
    for( uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++ )
    {
        if( m_pItems[i] != NULL )
        {
            if( m_pItems[i]->GetProto()->BagFamily == item->GetProto()->BagFamily )
            {
                return i;
            }
        }
    }
    return ITEM_NO_SLOT_AVAILABLE;
}

SlotResult PlayerInventory::FindFreeInventorySlot(ItemPrototype *proto)
{
    //special item
    //special slots will be ignored of item is not set
    if( proto != NULL )
    {
        //sLog.outDebug( "PlayerInventory::FindFreeInventorySlot called for item %s" , proto->Name1 );
        if( proto->BagFamily)
        {
            for( uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++ )
            {
                if( m_pItems[i] != NULL && m_pItems[i]->IsContainer() )
                {
                    if( m_pItems[i]->GetProto()->BagFamily == proto->BagFamily )
                    {
                        int32 slot = castPtr<Container>( m_pItems[i] )->FindFreeSlot();
                        if( slot != ITEM_NO_SLOT_AVAILABLE )
                        {
                            result.ContainerSlot = i;
                            result.Slot = slot;
                            result.Result = true;
                            return result;
                        }
                    }
                }
            }
        }
    }

    //backpack
    for( uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++ )
    {
        Item* item = GetInventoryItem( i );
        if( item == NULL )
        {
            result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
            result.Slot = i;
            result.Result = true;
            return result;
        }
    }

    //bags
    for( uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++ )
    {
        Item* item = GetInventoryItem(i);
        if( item != NULL )
        {
            if( item->IsContainer() && !item->GetProto()->BagFamily )
            {
                int32 slot = castPtr<Container>( m_pItems[i] )->FindFreeSlot();
                if( slot != ITEM_NO_SLOT_AVAILABLE )
                {
                    result.ContainerSlot = i;
                    result.Slot = slot;
                    result.Result = true;
                    return result;
                }
            }
        }
    }

    result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
    result.Slot = ITEM_NO_SLOT_AVAILABLE;
    result.Result = false;

    return result;
}

SlotResult PlayerInventory::FindFreeBankSlot(ItemPrototype *proto)
{
    //special item
    //special slots will be ignored of item is not set
    if( proto != NULL )
    {
        if( proto->BagFamily )
        {
            for( uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++ )
            {
                if( m_pItems[i] != NULL && m_pItems[i]->IsContainer() )
                {
                    if( m_pItems[i]->GetProto()->BagFamily == proto->BagFamily )
                    {
                        int32 slot = castPtr<Container>( m_pItems[i] )->FindFreeSlot();
                        if( slot != ITEM_NO_SLOT_AVAILABLE )
                        {
                            result.ContainerSlot = i;
                            result.Slot = slot;
                            result.Result = true;
                            return result;
                        }
                    }
                }
            }
        }
    }

    //backpack
    for( uint32 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++ )
    {
        Item* item = GetInventoryItem( i );
        if( item == NULL )
        {
            result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
            result.Slot = i;
            result.Result = true;
            return result;
        }
    }

    //bags
    for( uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++ )
    {
        Item* item = GetInventoryItem(i);
        if( item != NULL )
        {
            if( item->IsContainer() && !item->GetProto()->BagFamily )
            {
                int32 slot = castPtr<Container>( m_pItems[i] )->FindFreeSlot();
                if( slot != ITEM_NO_SLOT_AVAILABLE)
                {
                    result.ContainerSlot = i;
                    result.Slot = slot;
                    result.Result = true;
                    return result;
                }
            }
        }
    }

    result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
    result.Slot = ITEM_NO_SLOT_AVAILABLE;
    result.Result = false;

    return result;
}

SlotResult PlayerInventory::FindAmmoBag()
{
    for( uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++ )
    {
        if( m_pItems[i] != NULL && m_pItems[i]->IsAmmoBag())
        {
            result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
            result.Slot = i;
            result.Result = true;
            return result;
        }
    }

    result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
    result.Slot = ITEM_NO_SLOT_AVAILABLE;
    result.Result = false;

    return result;
}

void PlayerInventory::ReduceItemDurability()
{
    uint32 f = RandomUInt(100);
    if( f <= 10 ) //10% chance to loose 1 dur from a random valid item.
    {
        int32 slot = RandomUInt( EQUIPMENT_SLOT_END );
        Item* pItem = GetInventoryItem( INVENTORY_SLOT_NOT_SET, slot );
        if( pItem != NULL )
        {
            if( pItem->GetUInt32Value( ITEM_FIELD_DURABILITY) && pItem->GetUInt32Value( ITEM_FIELD_MAXDURABILITY ) )
            {
                pItem->SetUInt32Value( ITEM_FIELD_DURABILITY, ( pItem->GetUInt32Value( ITEM_FIELD_DURABILITY ) - 1 ) );
                pItem->m_isDirty = true;
                //check final durabiity
                if(pItem->GetUInt32Value( ITEM_FIELD_DURABILITY ) == 0 ) //no dur left
                    GetOwner()->ApplyItemMods( pItem, slot, false );
            }
        }
    }
}

bool PlayerInventory::IsEquipped(uint32 itemid, int16 slotToSkip)
{
    for( uint32 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x )
    {
        if( m_pItems[x] != NULL && x != slotToSkip)
            if( m_pItems[x]->GetProto()->ItemId == itemid )
                return true;
    }
    return false;
}

void PlayerInventory::CheckAreaItems()
{
    for( uint32 x = EQUIPMENT_SLOT_START; x < INVENTORY_SLOT_ITEM_END; ++x )
    {
        if( m_pItems[x] != NULL )
        {
            if( IsBagSlot(x) && m_pItems[x]->IsContainer() )
            {
                Container* bag = castPtr<Container>(m_pItems[x]);

                for( int32 i = 0; i < bag->GetProto()->ContainerSlots; i++ )
                {
                    if( bag->GetItem(i) != NULL && bag->GetItem(i)->GetProto() && bag->GetItem(i)->GetProto()->MapID && bag->GetItem(i)->GetProto()->MapID != GetOwner()->GetMapId() )
                        bag->SafeFullRemoveItemFromSlot( i );
                }
            }
            else
            {
                if( m_pItems[x]->GetProto() != NULL && m_pItems[x]->GetProto()->MapID && m_pItems[x]->GetProto()->MapID != GetOwner()->GetMapId() )
                    SafeFullRemoveItemFromSlot( INVENTORY_SLOT_NOT_SET, x );
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Crow: Adds an item by id, allowing for count, random prop, and if created, will send item push created, else, recieved.
// This was supposed to be given to Arc :|
uint32 PlayerInventory::AddItemById( uint32 itemid, uint32 count, int32 randomprop, uint8 addItemFlags, Player* creator /* = NULL*/ )
{
    if( count < 1 )
        count = 1;

    Player *chr = GetOwner();

    // checking if the PlayerInventory has owner, impossible to not have one
    if( chr == NULL )
        return false;

    ItemPrototype* it = sItemMgr.LookupEntry(itemid);
    if(it == NULL )
        return false;

    uint32 maxStack = (it->MaxCount > 0 ? it->MaxCount : 1 ); // Our database is lacking :|
    uint32 added = count, toadd = 0;
    bool freeslots = true;

    while( added > 0 && freeslots )
    {
        if( added < maxStack )
        {
            // find existing item with free stack
            Item* free_stack_item = FindItemLessMax( itemid, added, false );
            if( free_stack_item != NULL )
            {
                // increase stack by new amount
                uint8 inventorySlot = 0xFF;
                uint16 bagSlot = GetBagSlotByGuid(free_stack_item->GetGUID(), inventorySlot);
                _sendPushResult(free_stack_item, bagSlot, inventorySlot, added, addItemFlags);
                free_stack_item->SetUInt32Value( ITEM_FIELD_STACK_COUNT, free_stack_item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + added );
                free_stack_item->m_isDirty = true;
                return 0; // None left
            }
        }

        // create new item
        Item *item = objmgr.CreateItem( itemid, chr );
        if( item == NULL )
            return false;

        if(creator != NULL)
            item->SetUInt64Value(ITEM_FIELD_CREATOR, creator->GetGUID());

        item->Bind(ITEM_BIND_ON_PICKUP);
        if(randomprop || it->RandomPropId || it->RandomSuffixId)
            item->SetItemRandomPropertyData(randomprop ? randomprop : it->RandomPropId ? it->RandomPropId : it->RandomSuffixId, RandomUInt());
        item->LoadRandomProperties();

        toadd = added > maxStack ? maxStack : added;

        item->SetUInt32Value( ITEM_FIELD_STACK_COUNT, toadd );
        if( AddItemToFreeSlot( item ) )
        {
            SlotResult *lr = LastSearchResult();
            _sendPushResult(item, lr->ContainerSlot, lr->Slot, toadd, addItemFlags);
            sQuestMgr.OnPlayerItemPickup(chr, item, toadd);
            added -= toadd;
        }
        else
        {
            freeslots = false;
            chr->GetSession()->SendNotification("No free slots were found in your inventory!");
            item->Destruct();
            item = NULL;
            if(creator != NULL && creator != chr) // If someone else is creating the item, its mainly for GM command though.
                creator->GetSession()->SendNotification("No free slots were found in target's inventory!");
        }
    }
    return added;
}

void PlayerInventory::SwapItems(int16 SrcInvSlot, int16 DstInvSlot, int16 SrcSlot, int16 DstSlot)
{
    if(SrcInvSlot == DstInvSlot)
    {
        if( SrcInvSlot == INVENTORY_SLOT_NOT_SET ) //in backpack
            SwapItemSlots( SrcSlot, DstSlot );
        else//in bag
            castPtr<Container>( GetInventoryItem( SrcInvSlot ) )->SwapItems( SrcSlot, DstSlot );
    }
    else
    {
        Item *SrcItem = GetInventoryItem(SrcInvSlot, SrcSlot), *DstItem = GetInventoryItem(DstInvSlot, DstSlot);

        //Check for stacking
        if(DstItem && SrcItem->GetEntry()==DstItem->GetEntry() && (SrcItem->GetProto()->MaxCount < 0 || SrcItem->GetProto()->MaxCount > 1))
        {
            uint32 total = SrcItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT)+DstItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
            if(total <= (uint32)DstItem->GetProto()->MaxCount || DstItem->GetProto()->MaxCount < 0)
            {
                DstItem->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT,SrcItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT));
                DstItem->m_isDirty = true;
                if(SafeFullRemoveItemFromSlot(SrcInvSlot, SrcSlot) == false)
                    BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_CANT_STACK);
                return;
            }
            else
            {
                if(DstItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT) != DstItem->GetProto()->MaxCount)
                {
                    int32 delta=DstItem->GetProto()->MaxCount-DstItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
                    DstItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT,DstItem->GetProto()->MaxCount);
                    SrcItem->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT,-delta);
                    SrcItem->m_isDirty = true;
                    DstItem->m_isDirty = true;
                    return;
                }
            }
        }

        if(SrcItem)
            SrcItem = SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot,SrcSlot, false);

        if(DstItem)
            DstItem = SafeRemoveAndRetreiveItemFromSlot(DstInvSlot,DstSlot, false);

        if(SrcItem)
        {
            AddItemResult result = SafeAddItem(SrcItem,DstInvSlot,DstSlot);
            if(!result)
            {
                if (!SafeAddItem(SrcItem, SrcInvSlot, SrcSlot))
                {
                    SrcItem->Destruct();
                    SrcItem = NULL;
                }
                if (DstItem && !SafeAddItem(DstItem, DstInvSlot, DstSlot))
                {
                    DstItem->Destruct();
                    DstItem = NULL;
                }
            }
        }

        if(DstItem)
        {
            AddItemResult result = SafeAddItem(DstItem,SrcInvSlot,SrcSlot);
            if(!result)
            {
                if (SrcItem && !SafeAddItem(SrcItem, SrcInvSlot, SrcSlot))
                {
                    SrcItem->Destruct();
                    SrcItem = NULL;
                }
                if (!SafeAddItem(DstItem, DstInvSlot, DstSlot))
                {
                    DstItem->Destruct();
                    DstItem = NULL;
                }
            }
        }
    }
}

uint32 PlayerInventory::GetEquippedItemCountWithLimitId(uint32 Id)
{
    uint32 count = 0;
    for( uint32 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x )
    {
        Item* it = m_pItems[x];
        if( it != NULL )
        {
            ItemPrototype * ip = it->GetProto();
            if( ip && ip->ItemLimitCategory == Id )
                count++;
        }
    }
    return count;
}

uint32 PlayerInventory::GetSocketedGemCountWithLimitId(uint32 Id)
{
    uint32 count = 0;
    for( uint32 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x )
    {
        if( Item* it = m_pItems[x] )
        {
            for( uint32 socketcount = 0; socketcount < 3; socketcount++ )
            {
                EnchantmentInstance *e = it->GetEnchantment( 2 + socketcount );
                if (e && e->Enchantment && e->Enchantment->GemEntry)
                {
                    ItemPrototype * ip = sItemMgr.LookupEntry(e->Enchantment->GemEntry);
                    if( ip && ip->ItemLimitCategory == Id )
                        count++;
                }
            }
        }
    }
    return count;
}

void PlayerInventory::RemoveItemsWithHolidayId(uint32 IgnoreHolidayId)
{
    for(uint32 i = EQUIPMENT_SLOT_START; i < MAX_INVENTORY_SLOT; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item)
        {
            if(item->GetProto()->HolidayId && item->GetProto()->HolidayId != IgnoreHolidayId)
                SafeFullRemoveItemByGuid(item->GetGUID());
        }
    }
}

void PlayerInventory::_sendPushResult(Item *item, int8 bagSlot, uint8 slot, uint32 count, uint8 addItemFlags)
{
    WorldPacket data(SMSG_ITEM_PUSH_RESULT, 50);
    data << m_pOwner->GetGUID();
    data << uint32((addItemFlags & ADDITEM_FLAG_LOOTED) == 0 ? 1 : 0);
    data << uint32((addItemFlags & ADDITEM_FLAG_CREATED) == 0 ? 0 : 1);
    data << uint32((addItemFlags & ADDITEM_FLAG_SILENT) == 0 ? 1 : 0);
    data << uint8(bagSlot) << uint32(item->GetStackCount() == count ? slot : 0xFFFFFFFF);
    data << uint32(item->GetEntry());
    data << uint32(item->GetItemPropertySeed());
    data << uint32(item->GetItemRandomPropertyId());
    data << uint32(count);
    data << uint32(1);
    if((addItemFlags & ADDITEM_FLAG_CREATED) && (addItemFlags & ADDITEM_FLAG_GIFTED) == 0)
        m_pOwner->SendMessageToSet(&data, true, true, 25.f);
    else m_pOwner->PushPacket(&data);

    if(m_pOwner->GetGroup() && (addItemFlags & ADDITEM_FLAG_LOOTED) > 0)
        m_pOwner->GetGroup()->SendPacketToAllButOne(&data, m_pOwner);
}
