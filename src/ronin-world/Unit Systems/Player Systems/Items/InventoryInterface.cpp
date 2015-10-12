/***
 * Demonstrike Core
 */

#include "StdAfx.h"

//
//-------------------------------------------------------------------//
PlayerInventory::PlayerInventory( Player* pPlayer ) : m_pOwner(pPlayer)
{

}

//-------------------------------------------------------------------//
PlayerInventory::~PlayerInventory()
{
    while(!m_itemPtrs.empty())
    {
        Item *item = m_itemPtrs.begin()->second;
        m_itemPtrs.erase(m_itemPtrs.begin());
        item->Destruct();
    }

    m_pOwner = NULL;
}

SlotResult PlayerInventory::_addItem( Item* item, uint16 slot)
{
    SlotResult result;
    result.result = ADD_ITEM_RESULT_ERROR;
    if( item == NULL || !item->GetProto() )
        return result;
    if(m_itemSlots.find(item->GetGUID()) != m_itemSlots.end())
    {
        result.result = ADD_ITEM_RESULT_DUPLICATED;
        return result;
    }

    if(INVSLOT_ITEM(slot) == INVENTORY_SLOT_NONE && !FindFreeSlot(item, slot))
        return result;
    if(!IsValidSlot(item, slot))
        return result;

    if(INVSLOT_BAG(slot) != INVENTORY_SLOT_NONE)
    {
        if(Item *container = m_itemPtrs.at(MAKE_INVSLOT(0xFF, INVSLOT_BAG(slot))))
        {
            if(container->AddItem(INVSLOT_ITEM(slot), item->GetGUID()))
                item->SetContainerData(container->GetGUID(), slot);
            else return result;
        }else return result;

        // This has to happen post adding to container
        item->SetOwner( m_pOwner );
        item->Bind(ITEM_BIND_ON_PICKUP);
    }
    else
    {
        // This has to happen pre adding to player slots
        item->SetOwner( m_pOwner );
        item->Bind(ITEM_BIND_ON_PICKUP);

        item->SetUInt64Value(ITEM_FIELD_CONTAINED, m_pOwner->GetGUID());
        if(INVSLOT_ITEM(slot) < EQUIPMENT_SLOT_END)
        {
            uint32 VisibleBase = PLAYER_VISIBLE_ITEM + (INVSLOT_ITEM(slot) * PLAYER_VISIBLE_ITEM_LENGTH);
            m_pOwner->SetUInt32Value( VisibleBase, item->GetUInt32Value( OBJECT_FIELD_ENTRY ) );
            m_pOwner->SetUInt32Value( VisibleBase+1, item->GetUInt32Value( ITEM_FIELD_ENCHANTMENT_DATA ) );
            if(INVSLOT_ITEM(slot) == EQUIPMENT_SLOT_OFFHAND && item->GetProto()->Class == ITEM_CLASS_WEAPON)
                m_pOwner->SetDualWield(true);
            if(m_pOwner->IsInWorld())
                m_pOwner->ApplyItemMods( item, INVSLOT_ITEM(slot), true );
            if(uint32 itemSet = item->GetProto()->ItemSet)
            {
                if(ItemSetEntry* set = dbcItemSet.LookupEntry(itemSet))
                {
                    std::set<WoWGuid> *m_set = &m_itemSets[itemSet];
                    m_set->insert(item->GetGUID());

                    bool applicable = true;
                    if(set->requiredSkill[0])
                    {
                        m_skillItemSets.insert(itemSet);
                        applicable = (m_pOwner->_GetSkillLineCurrent(set->requiredSkill[0], true) >= set->requiredSkill[1]);
                    }

                    if(applicable)
                    {
                        for( uint8 x = 0; x < 8; x++)
                        {
                            if(set->setBonusSpellIds[x] == 0)
                                continue;

                            if(SpellEntry *info = dbcSpell.LookupEntry(set->setBonusSpellIds[x]))
                            {
                                if( m_set->size() == set->spellRequiredItemCount[x])
                                {   //cast new spell
                                    SpellCastTargets targets(m_pOwner->GetGUID());
                                    if(Spell* spell = new Spell( m_pOwner, info ))
                                        spell->prepare( &targets, true );
                                }
                            }
                        }
                    }
                }
            }
        }
        m_pOwner->SetUInt64Value(PLAYER_FIELD_INV_SLOT_HEAD + (INVSLOT_ITEM(slot)*2), item->GetGUID());
    }

    m_itemPtrs.insert(std::make_pair(slot, item));
    m_itemSlots.insert(std::make_pair(item->GetGUID(), slot));
    if(item->GetProto()->Flags & DBC_ITEMFLAG_CONJURED)
        m_conjuredItems.insert(item->GetGUID());
    m_itemsByEntry[item->GetEntry()].insert(item->GetGUID());

    if(!item->IsInWorld() && m_pOwner->IsInWorld())
        item->AddToWorld();
    result.result = ADD_ITEM_RESULT_OK;
    return result;
}

Item *PlayerInventory::_removeItemBySlot(uint16 slot)
{
    if(m_itemPtrs.find(slot) == m_itemPtrs.end())
        return NULL;

    Item *item = m_itemPtrs.at(slot);
    item->SetOwner(NULL);
    item->SetUInt64Value(ITEM_FIELD_CONTAINED, 0);

    if(INVSLOT_BAG(slot) != INVENTORY_SLOT_MAX)
    {
        if(Item *container = m_itemPtrs.at(MAKE_INVSLOT(0xFF, INVSLOT_BAG(slot))))
        {
            container->RemoveItem(INVSLOT_ITEM(slot));
            item->SetContainerData(0, 0);
        } else ASSERT(false);
    }
    else
    {
        item->SetContainerData(0, 0);
        if(INVSLOT_ITEM(slot) < EQUIPMENT_SLOT_END)
        {
            m_pOwner->SetUInt64Value(PLAYER_VISIBLE_ITEM + (INVSLOT_ITEM(slot) * PLAYER_VISIBLE_ITEM_LENGTH), 0);
            if(INVSLOT_ITEM(slot) == EQUIPMENT_SLOT_OFFHAND && item->GetProto()->Class == ITEM_CLASS_WEAPON)
                m_pOwner->SetDualWield(false);

            if(m_pOwner->IsInWorld())
                m_pOwner->ApplyItemMods( item, INVSLOT_ITEM(slot), false );

            if(uint32 itemSet = item->GetProto()->ItemSet)
            {
                if(ItemSetEntry* set = dbcItemSet.LookupEntry(itemSet))
                {
                    std::set<WoWGuid> *m_set = &m_itemSets[itemSet];
                    m_set->erase(item->GetGUID());

                    for( uint8 x = 0; x < 8; x++)
                    {
                        if(set->setBonusSpellIds[x] == 0)
                            continue;
                        if( m_set->size() == set->spellRequiredItemCount[x])
                            m_pOwner->RemoveAura(set->setBonusSpellIds[x]);
                    }

                    if(m_set->size() == 0)
                    {
                        m_itemSets.erase(itemSet);
                        m_skillItemSets.erase(itemSet);
                    }
                }
            }
        }
        m_pOwner->SetUInt64Value(PLAYER_FIELD_INV_SLOT_HEAD + (INVSLOT_ITEM(slot)*2), item->GetGUID());
    }

    if(item->IsInWorld())
    {
        if(m_pOwner->IsInWorld())
            item->DestroyForPlayer(m_pOwner);
        item->RemoveFromWorld(true);
    }
    m_itemPtrs.erase(slot);
    m_conjuredItems.erase(item->GetGUID());

    // Items by entry cleanup
    std::set<WoWGuid> *itemsByEntry = &m_itemsByEntry[item->GetEntry()];
    itemsByEntry->erase(item->GetGUID());
    if(itemsByEntry->size() == 0) m_itemsByEntry.erase(item->GetEntry());
    return item;
}

bool PlayerInventory::_findFreeSlot(ItemPrototype *proto, uint16 &slot)
{
    uint16 bagSlot = 0xFF00, fallbackSpot = 0xFFFF;
    for(uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        INVSLOT_SET_ITEMSLOT(bagSlot, i);
        if(m_itemPtrs.find(bagSlot) == m_itemPtrs.end())
            continue;
        Item *bag = m_itemPtrs.at(bagSlot);
        if(bag->GetProto()->BagFamily && bag->GetProto()->BagFamily != proto->BagFamily)
            continue;

        if(proto->BagFamily && bag->GetProto()->BagFamily == proto->BagFamily)
        {
            for(uint32 s = 0; s < bag->GetNumSlots(); s++)
            {
                if(bag->HasItem(s))
                    continue;
                slot = MAKE_INVSLOT(i, s);
                return true;
            }
        }
        else
        {
            for(uint32 s = 0; s < bag->GetNumSlots(); s++)
            {
                if(bag->HasItem(s))
                    continue;
                fallbackSpot = MAKE_INVSLOT(i, s);
                break;
            }

            if(fallbackSpot != 0xFFFF)
                break;
        }
    }

    for(uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        if(m_itemPtrs.find(i) == m_itemPtrs.end())
        {
            slot = MAKE_INVSLOT(INVENTORY_SLOT_NONE, i);
            return true;
        }
    }

    if(fallbackSpot != 0xFFFF)
    {
        slot = fallbackSpot;
        return true;
    }

    return false;
}

bool PlayerInventory::_findBestSlot(ItemPrototype *proto, uint16 &slot)
{
    return false;
}

void PlayerInventory::AddToWorld()
{

}

void PlayerInventory::RemoveFromWorld(bool destroy)
{

}

uint32 PlayerInventory::BuildCreateUpdateBlocks(ByteBuffer *data)
{
    uint32 count = 0;
    for(std::map<uint16, Item*>::iterator itr = m_itemPtrs.begin(); itr != m_itemPtrs.end(); itr++)
        count += itr->second->BuildCreateUpdateBlockForPlayer(data, m_pOwner);
    return count;
}

void PlayerInventory::DestroyForPlayer(Player* plr)
{
    ASSERT(m_pOwner != NULL);

    if(plr == m_pOwner)
    {
        for(auto itr = m_itemPtrs.begin(); itr != m_itemPtrs.end(); itr++)
            itr->second->DestroyForPlayer(plr);
    }
    else
    {
        uint16 slot = 0xFF00;
        for(uint8 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
        {
            (slot &= ~0xFF) |= i;
            if(m_itemPtrs.find(slot) == m_itemPtrs.end())
                continue;
            m_itemPtrs.at(slot)->DestroyForPlayer( plr );
        }
    }
}

bool PlayerInventory::IsBagSlot(uint8 slot)
{
    if((slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END) || (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END))
    {
        return true;
    }
    return false;
}

Item *PlayerInventory::GetInventoryItem(uint16 slot)
{
    if(m_itemPtrs.find(slot) == m_itemPtrs.end())
        return NULL;
    return m_itemPtrs.at(slot);
}

Item *PlayerInventory::GetInventoryItem(WoWGuid guid)
{
    if(m_itemSlots.find(guid) == m_itemSlots.end())
        return NULL;
    return GetInventoryItem(m_itemSlots.at(guid));
}

Item *PlayerInventory::GetInventoryItem(ItemData *data)
{
    if(m_itemSlots.find(data->itemGuid) == m_itemSlots.end())
        return NULL;
    return GetInventoryItem(m_itemSlots.at(data->itemGuid));
}

uint16 PlayerInventory::GetInventorySlotByEntry(uint32 itemId)
{
    for(auto itr = m_itemSlots.begin(); itr != m_itemSlots.end(); itr++)
    {
        WoWGuid guid = itr->first;
        if(guid.getEntry() == itemId)
            return itr->second;
    }

    return INVENTORY_SLOT_NONE;
}

uint16 PlayerInventory::GetInventorySlotByGuid(WoWGuid guid)
{
    if(m_itemSlots.find(guid) == m_itemSlots.end())
        return INVENTORY_SLOT_NONE;
    return m_itemSlots.at(guid);
}

void PlayerInventory::CheckAreaItems()
{

}

void PlayerInventory::RemoveConjuredItems()
{
    std::set<WoWGuid> m_conjured(m_conjuredItems);
    do
    {
        WoWGuid itemGuid = *m_conjured.begin();
        m_conjured.erase(m_conjured.begin());
        if(m_itemSlots.find(itemGuid) == m_itemSlots.end())
            continue;
        if(Item *item = _removeItemBySlot(m_itemSlots.at(itemGuid)))
        {
            sItemMgr.DeleteItemData(itemGuid, true);
            item->Destruct();
        }
    }while(m_conjured.size());
}

void PlayerInventory::EmptyBuyBack()
{

}

bool PlayerInventory::CreateQuestItems(Quest *qst)
{
    if(qst->count_receiveitems == 0 && qst->srcitemcount == 0)
        return true;

    bool result = true, receivedSrc = false;
    // If the quest should give any items on begin, give them the items.
    for(uint8 i = 0; i < 4; i++)
    {
        if(qst->receive_items[i] == 0)
            continue;
        if(qst->srcitem == qst->receive_items[i])
            receivedSrc = true;
        if(ItemPrototype *proto = sItemMgr.LookupEntry(qst->receive_items[i]))
            if((result = CreateInventoryStacks(proto, qst->receive_itemcount[i])) == false)
                break;
    }

    if(result && qst->srcitem && !receivedSrc)
    {
        if(ItemPrototype *proto = sItemMgr.LookupEntry(qst->srcitem))
            result = CreateInventoryStacks(proto, std::max(uint16(1), qst->srcitemcount));
    }

    if(result == false)
    {
        return false;
    }
    return true;
}

bool PlayerInventory::CreateQuestRewards(Quest *qst, uint8 reward_slot)
{
    // Static Item reward
    /*for(uint32 i = 0; i < 4; i++)
    {
        if(qst->reward_item[i])
        {
            ItemPrototype *proto = sItemMgr.LookupEntry(qst->reward_item[i]);
            if(!proto)
                sLog.outDebug("Invalid item prototype in quest reward! ID %d, quest %d", qst->reward_item[i], qst->id);
            else
            {
                Item* add;
                SlotResult slotresult;
                add = FindItemLessMax(qst->reward_item[i], qst->reward_itemcount[i], false);
                if (!add)
                {
                    slotresult = plr->GetPlayerInventory()->FindFreeInventorySlot(proto);
                    if(!slotresult.Result)
                    {
                        BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                    }
                    else
                    {
                        Item* itm = objmgr.CreateItem(qst->reward_item[i], plr);
                        itm->SetUInt32Value(ITEM_FIELD_STACK_COUNT, uint32(qst->reward_itemcount[i]));
                        if( !SafeAddItem(itm,slotresult.ContainerSlot, slotresult.Slot) )
                        {
                            itm->Destruct();
                            itm = NULL;
                        }
                    }
                }
                else
                {
                    add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + qst->reward_itemcount[i]);
                    add->m_isDirty = true;
                }
            }
        }
    }

    // Choice Rewards
    if(qst->reward_choiceitem[reward_slot])
    {
        ItemPrototype *proto = sItemMgr.LookupEntry(qst->reward_choiceitem[reward_slot]);
        if(!proto)
            sLog.outDebug("Invalid item prototype in quest reward! ID %d, quest %d", qst->reward_choiceitem[reward_slot], qst->id);
        else
        {
            Item* add;
            SlotResult slotresult;
            add = FindItemLessMax(qst->reward_choiceitem[reward_slot], qst->reward_choiceitemcount[reward_slot], false);
            if (!add)
            {
                slotresult = FindFreeInventorySlot(proto);
                if(!slotresult.Result)
                {
                    BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                }
                else
                {
                    Item* itm = objmgr.CreateItem(qst->reward_choiceitem[reward_slot], plr);
                    itm->SetUInt32Value(ITEM_FIELD_STACK_COUNT, uint32(qst->reward_choiceitemcount[reward_slot]));
                    if( !SafeAddItem(itm,slotresult.ContainerSlot, slotresult.Slot) )
                    {
                        itm->Destruct();
                        itm = NULL;
                    }

                }
            }
            else
            {
                add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + qst->reward_choiceitemcount[reward_slot]);
                add->m_isDirty = true;
            }
        }
    }*/
    return true;
}

bool PlayerInventory::CreateInventoryStacks(ItemPrototype *proto, uint32 count, WoWGuid creatorGuid, bool fromNPC)
{
    uint8 pushFlags = fromNPC ? (creatorGuid.empty() ? 0x01 : 0x02) : 0x10;
    if(proto->MaxCount > 1)
    {
        if(m_itemsByEntry.find(proto->ItemId) != m_itemsByEntry.end())
        {
            std::set<WoWGuid> *items = &m_itemsByEntry[proto->ItemId];
            for(std::set<WoWGuid>::iterator itr = items->begin(); itr != items->end(); itr++)
            {
                if(m_itemSlots.find(*itr) == m_itemSlots.end())
                    continue; // Shouldn't happen
                uint16 itemSlot = m_itemSlots.at(*itr);
                if(m_itemPtrs.find(itemSlot) == m_itemPtrs.end())
                    continue; // Also shouldn't happen
                Item *item = m_itemPtrs.at(itemSlot);
                item->ModStackSize(count);
                SendItemPushResult(item, itemSlot, 0x00, count);
                if(count == 0x00000000)
                    return true;
            }
        }
    }

    while(count > 0)
    {
        uint16 targetSlot = 0xFFFF;
        if(!_findFreeSlot(proto, targetSlot))
            return false;

        ItemData *data = sItemMgr.CreateItemData(proto->ItemId);
        if(data == NULL)
            return false;
        Item *item = new Item(data);
        item->SetOwner(m_pOwner);
        item->ModStackSize(count);
        if(creatorGuid.count())
            item->SetCreatorGuid(creatorGuid);

        if(!AddInventoryItemToSlot(item, targetSlot))
        {
            sItemMgr.DeleteItemData(item->GetGUID(), true);
            item->Destruct();
            return false;
        }
        SendItemPushResult(item, targetSlot, pushFlags, count);
    }
    return true;
}

bool PlayerInventory::AddInventoryItemToSlot(Item *item, uint16 slot)
{
    return _addItem(item, slot).result == ADD_ITEM_RESULT_OK;
}

bool PlayerInventory::DestroyInventoryItem(uint16 slot, ItemDeletionReason reason)
{
    Item *item = _removeItemBySlot(slot);
    if(item == NULL)
        return false;

    sItemMgr.DeleteItemFromDatabase(item->GetGUID(), reason);
    sItemMgr.DeleteItemData(item->GetGUID(), reason == ITEM_DELETION_USED);
    item->Destruct();
}

Item *PlayerInventory::RemoveInventoryItem(uint16 slot)
{
    return _removeItemBySlot(slot);
}

Item *PlayerInventory::RemoveInventoryItem(WoWGuid guid)
{
    if(m_itemSlots.find(guid) == m_itemSlots.end())
        return NULL;

    return _removeItemBySlot(m_itemSlots.at(guid));
}

bool PlayerInventory::RemoveInventoryStacks(uint32 entry, uint32 count, bool force)
{
    if(m_itemsByEntry.find(entry) == m_itemsByEntry.end())
        return false;
    if(m_itemsByEntry.at(entry).size() == 0)
        return false;

    int32 itemCount = -int32(count);
    std::set<uint16> itemsToDelete;
    std::set<WoWGuid> *items = &m_itemsByEntry[entry];
    for(std::set<WoWGuid>::iterator itr = items->begin(); itr != items->end(); itr++)
    {
        if(itemCount >= 0)
            break;

        if(Item *item = GetInventoryItem(*itr))
        {
            if(itemCount >= item->GetStackSize())
            {
                itemsToDelete.insert(item->GetInventorySlot());
                itemCount += item->GetStackSize();
            } else item->ModStackSize(itemCount);
        }
    }

    if(force || itemCount >= 0)
    {
        for(auto itr = itemsToDelete.begin(); itr != itemsToDelete.end(); itr++)
        {
            if(Item *item = _removeItemBySlot(*itr))
            {
                sItemMgr.DeleteItemData(item->GetGUID(), true);
                item->Destruct();
            }
        }
    }
    return itemCount == 0;
}

void PlayerInventory::BuildInvError(uint8 error, Item *src, Item *dst, uint32 misc)
{
    if(m_pOwner == NULL || !m_pOwner->IsInWorld())
        return;

    WorldPacket data(SMSG_INVENTORY_CHANGE_FAILURE, 22);
    data << error;
    data << uint64(src ? src->GetGUID() : 0);
    data << uint64(dst ? dst->GetGUID() : 0);
    data << uint8(0);
    switch(error)
    {
    case INV_ERR_NO_OUTPUT:
        data << uint64(0); // item guid
        data << uint32(0); // slot
        data << uint64(0); // container
        break;
    case INV_ERR_CANT_EQUIP_LEVEL_N:
    case INV_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED_IS:
    case INV_ERR_ITEM_MAX_LIMIT_CATEGORY_SOCKETED_EXCEEDED_IS:
    case INV_ERR_ITEM_MAX_LIMIT_CATEGORY_EQUIPPED_EXCEEDED_IS:
        data << misc;
        break;
    }

    m_pOwner->GetSession()->SendPacket( &data );
}

void PlayerInventory::SendItemPushResult(Item *item, uint16 invSlot, uint8 flags, uint32 totalCount)
{
    WorldPacket data(SMSG_ITEM_PUSH_RESULT, 25);
    data << item->GetGUID();
    data << uint32((flags & 0x01) ? 1 : 0); // Received
    data << uint32((flags & 0x02) ? 1 : 0); // Created
    data << uint32(1);
    data << uint8(INVSLOT_BAG(invSlot));
    data << uint32((flags & 0x04) ? INVSLOT_ITEM(invSlot) : 0xFFFFFFFF);
    data << item->GetEntry();
    data << item->GetRandomSeed();
    data << item->GetRandomProperty();
    data << totalCount;
    data << item->GetStackSize();
    Group * grp = m_pOwner->GetGroup();
    if(flags & 0x10 && grp != NULL)
        grp->SendPacketToAll(&data);
    else m_pOwner->SendPacket(&data);
}
