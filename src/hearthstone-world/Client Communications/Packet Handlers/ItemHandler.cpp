/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleSplitOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 5);
    int8 DstInvSlot=0, DstSlot=0, SrcInvSlot=0, SrcSlot=0;
    uint8 count=0;

    AddItemResult result;

    recv_data >> SrcInvSlot >> SrcSlot >> DstInvSlot >> DstSlot >> count;
    if(!GetPlayer())
        return;

    if(count >= 127) /* exploit fix */      
        return;

    // f*ck wpe
    if( !_player->GetItemInterface()->VerifyBagSlotsWithBank(SrcInvSlot, SrcSlot) )
        return;

    if( !_player->GetItemInterface()->VerifyBagSlotsWithBank(DstInvSlot, DstSlot) )
        return;

    int32 c = count;
    Item* i1 = _player->GetItemInterface()->GetInventoryItem(SrcInvSlot,SrcSlot);
    if(!i1)
        return;
    Item* i2=_player->GetItemInterface()->GetInventoryItem(DstInvSlot,DstSlot);

    if( (i1 && i1->wrapped_item_id) || (i2 && i2->wrapped_item_id) )
    {
        GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
        return;
    }

    if(i1)
    {
        if(i1->GetProto()->MaxCount > 0 && i1->GetProto()->MaxCount < 2)
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
            return;
        }
    }

    if(i2)
    {
        if(count < 1)
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
            return;
        }

        if(i2->GetProto()->MaxCount > 0 && i2->GetProto()->MaxCount < 2)
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
            return;
        }
    }

    if(i2)//smth already in this slot
    {
        if(i1->GetEntry()==i2->GetEntry() )
        {
            //check if player has the required stacks to avoid exploiting.
            //safe exploit check
            if(c < (int32)i1->GetUInt32Value(ITEM_FIELD_STACK_COUNT))
            {
                //check if there is room on the other item.
                if(((c + i2->GetUInt32Value(ITEM_FIELD_STACK_COUNT)) <= (uint32)i2->GetProto()->MaxCount || i2->GetProto()->MaxCount < 0))
                {
                    i1->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT, -c);
                    i2->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT, +c);
                    i1->m_isDirty = true;
                    i2->m_isDirty = true;
                }
                else
                     GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
            }
            else        
                _player->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_COULDNT_SPLIT_ITEMS); //error cant split item
        }
        else
             GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
    }
    else
    {
        if(c < (int32)i1->GetUInt32Value(ITEM_FIELD_STACK_COUNT))
        {
            i1->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT,-c);

            i2=objmgr.CreateItem(i1->GetEntry(),_player);
            i2->SetUInt32Value(ITEM_FIELD_STACK_COUNT,c);
            i1->m_isDirty = true;
            i2->m_isDirty = true;

            if(DstSlot == -1)
            {
                // Find a free slot
                SlotResult res = _player->GetItemInterface()->FindFreeInventorySlot(i2->GetProto());
                if(!res.Result)
                {
                    SendNotification("Internal Error");
                    return;
                }
                else
                {
                    DstSlot = res.Slot;
                    DstInvSlot = res.ContainerSlot;
                }
            }
            result = _player->GetItemInterface()->SafeAddItem(i2,DstInvSlot,DstSlot);
            if(!result)
            {
                printf("HandleBuyItemInSlot: Error while adding item to dstslot");
                //i2->DeleteFromDB();
                i2->DeleteMe();
                i2 = NULL;
            }
        }
        else
            _player->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_COULDNT_SPLIT_ITEMS);
    }
}

void WorldSession::HandleSwapItemOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 4);
    WorldPacket data;
    WorldPacket packet;
    Item* SrcItem = NULLITEM;
    Item* DstItem = NULLITEM;

    int8 DstInvSlot=0, DstSlot=0, SrcInvSlot=0, SrcSlot=0, error=0;

    recv_data >> DstInvSlot >> DstSlot >> SrcInvSlot >> SrcSlot;

    sLog.outDebug("ITEM: swap, DstInvSlot %i DstSlot %i SrcInvSlot %i SrcSlot %i", DstInvSlot, DstSlot, SrcInvSlot, SrcSlot);

    if(DstInvSlot == SrcSlot && SrcInvSlot == -1) // player trying to add self container to self container slots
    {
        GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_ITEMS_CANT_BE_SWAPPED);
        return;
    }

    if( ( DstInvSlot <= 0 && DstSlot < 0 ) || DstInvSlot < -1 )
        return;

    if( ( SrcInvSlot <= 0 && SrcSlot < 0 ) || SrcInvSlot < -1 )
        return;

    SrcItem=_player->GetItemInterface()->GetInventoryItem(SrcInvSlot,SrcSlot);
    if(!SrcItem)
        return;

    DstItem=_player->GetItemInterface()->GetInventoryItem(DstInvSlot,DstSlot);

    if(DstItem)
    {   //check if it will go to equipment slot
        if(SrcInvSlot==INVENTORY_SLOT_NOT_SET)//not bag
        {
            if(DstItem->IsContainer())
            {
                if(TO_CONTAINER(DstItem)->HasItems())
                {
                    if(SrcSlot < INVENTORY_SLOT_BAG_START || SrcSlot >= INVENTORY_SLOT_BAG_END || SrcSlot < BANK_SLOT_BAG_START || SrcSlot >= BANK_SLOT_BAG_END)
                    {
                        _player->GetItemInterface()->BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                        return;
                    }
                }
            }

            if(SrcSlot < MAX_INVENTORY_SLOT)
            {
                if((error = GetPlayer()->GetItemInterface()->CanEquipItemInSlot(SrcInvSlot, SrcSlot, DstItem->GetProto())))
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(SrcItem, DstItem, error);
                    return;
                }
            }
        }
        else
        {
            if(DstItem->IsContainer())
            {
                if(TO_CONTAINER(DstItem)->HasItems())
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                    return;
                }
            }

            if((error = GetPlayer()->GetItemInterface()->CanEquipItemInSlot(SrcInvSlot, SrcInvSlot, DstItem->GetProto())))
            {
                _player->GetItemInterface()->BuildInventoryChangeError(SrcItem, DstItem, error);
                return;
            }
        }
    }

    if(SrcItem)
    {   //check if it will go to equipment slot
        if(DstInvSlot == INVENTORY_SLOT_NOT_SET)//not bag
        {
            if(SrcItem->IsContainer())
            {
                if(TO_CONTAINER(SrcItem)->HasItems())
                {
                    if(DstSlot < INVENTORY_SLOT_BAG_START || DstSlot >= INVENTORY_SLOT_BAG_END || DstSlot < BANK_SLOT_BAG_START || DstSlot >= BANK_SLOT_BAG_END)
                    {
                        _player->GetItemInterface()->BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                        return;
                    }
                }
            }

            if(DstSlot < MAX_INVENTORY_SLOT)
            {
                if((error=GetPlayer()->GetItemInterface()->CanEquipItemInSlot(DstInvSlot, DstSlot, SrcItem->GetProto())))
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(SrcItem, DstItem, error);
                    return;
                }
            }
        }
        else
        {
            if(SrcItem->IsContainer())
            {
                if(TO_CONTAINER(SrcItem)->HasItems())
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                    return;
                }
            }

            if((error=GetPlayer()->GetItemInterface()->CanEquipItemInSlot(DstInvSlot, DstInvSlot, SrcItem->GetProto())))
            {
                _player->GetItemInterface()->BuildInventoryChangeError(SrcItem, DstItem, error);
                return;
            }
        }
    }

    if( DstSlot < INVENTORY_SLOT_BAG_START && DstInvSlot == INVENTORY_SLOT_NOT_SET ) //equip
        SrcItem->Bind(ITEM_BIND_ON_EQUIP);

    _player->GetItemInterface()->SwapItems(SrcInvSlot, DstInvSlot, SrcSlot, DstSlot);
}

void WorldSession::HandleSwapInvItemOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 2);
    WorldPacket data;
    int8 srcslot=0, dstslot=0;
    int8 error=0;

    recv_data >> dstslot >> srcslot;

    if(!GetPlayer())
        return;

    sLog.outDebug("ITEM: swap, src slot: %u dst slot: %u", (uint32)srcslot, (uint32)dstslot);

    if(dstslot == srcslot) // player trying to add item to the same slot
    {
        GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_ITEMS_CANT_BE_SWAPPED);
        return;
    }

    Item* dstitem = _player->GetItemInterface()->GetInventoryItem(dstslot);
    Item* srcitem = _player->GetItemInterface()->GetInventoryItem(srcslot);

    // allow weapon switching in combat
    bool skip_combat = false;
    if( srcslot < EQUIPMENT_SLOT_END || dstslot < EQUIPMENT_SLOT_END )    // We're doing an equip swap.
    {
        if( _player->CombatStatus.IsInCombat() )
        {
            if( srcslot < EQUIPMENT_SLOT_MAINHAND || dstslot < EQUIPMENT_SLOT_MAINHAND )    // These can't be swapped
            {
                _player->GetItemInterface()->BuildInventoryChangeError(srcitem, dstitem, INV_ERR_CANT_DO_IN_COMBAT);
                return;
            }
            skip_combat= true;
        }
    }

    if( !srcitem )
    {
        _player->GetItemInterface()->BuildInventoryChangeError( srcitem, dstitem, INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM );
        return;
    }

    if( srcslot == dstslot )
    {
        _player->GetItemInterface()->BuildInventoryChangeError( srcitem, dstitem, INV_ERR_ITEM_DOESNT_GO_TO_SLOT );
        return;
    }

    if( ( error = _player->GetItemInterface()->CanEquipItemInSlot( INVENTORY_SLOT_NOT_SET, dstslot, srcitem->GetProto(), skip_combat ) ) )
    {
        if( dstslot < MAX_INVENTORY_SLOT )
        {
            _player->GetItemInterface()->BuildInventoryChangeError( srcitem, dstitem, error );
            return;
        }
    }

    if(dstitem)
    {
        if((error=_player->GetItemInterface()->CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET, srcslot, dstitem->GetProto(), skip_combat)))
        {
            if(srcslot < MAX_INVENTORY_SLOT)
            {
                data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
                data << error;
                if(error == 1)
                {
                    data << dstitem->GetProto()->RequiredLevel;
                }
                data << (srcitem ? srcitem->GetGUID() : uint64(0));
                data << (dstitem ? dstitem->GetGUID() : uint64(0));
                data << uint8(0);

                SendPacket( &data );
                return;
            }
        }
    }

    if(srcitem->IsContainer())
    {
        //source has items and dst is a backpack or bank
        if(TO_CONTAINER(srcitem)->HasItems())
            if(!_player->GetItemInterface()->IsBagSlot(dstslot))
            {
                _player->GetItemInterface()->BuildInventoryChangeError(srcitem,dstitem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                return;
            }

        if(dstitem)
        {
            //source is a bag and dst slot is a bag inventory and has items
            if(dstitem->IsContainer())
            {
                if(TO_CONTAINER(dstitem)->HasItems() && !_player->GetItemInterface()->IsBagSlot(srcslot))
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(srcitem,dstitem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                    return;
                }
            }
            else
            {
                //dst item is not a bag, swap impossible
                _player->GetItemInterface()->BuildInventoryChangeError(srcitem,dstitem,INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                return;
            }
        }

        //dst is bag inventory
        if(dstslot < INVENTORY_SLOT_BAG_END)
            srcitem->Bind(ITEM_BIND_ON_EQUIP);
    }

    // swap items
    _player->GetItemInterface()->SwapItemSlots(srcslot, dstslot);
}

void WorldSession::HandleDestroyItemOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 2);

    int8 SrcInvSlot, SrcSlot;
    uint32 data;

    recv_data >> SrcInvSlot >> SrcSlot >> data;

    sLog.outDebug( "ITEM: destroy, SrcInv Slot: %i Src slot: %i", SrcInvSlot, SrcSlot );
    Item* it = _player->GetItemInterface()->GetInventoryItem(SrcInvSlot,SrcSlot);

    if(it)
    {
        if(it->IsContainer())
        {
            if(TO_CONTAINER(it)->HasItems())
            {
                _player->GetItemInterface()->BuildInventoryChangeError( it, NULLITEM, INV_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS);
                return;
            }
        }

        if(it->GetProto()->ItemId == ITEM_ENTRY_GUILD_CHARTER)
        {
            if( _player->m_playerInfo->charterId[CHARTER_TYPE_GUILD] != 0 )
            {
                Charter *gc = guildmgr.GetCharter(_player->m_playerInfo->charterId[CHARTER_TYPE_GUILD], CHARTER_TYPE_GUILD);
                if(gc != NULL)
                    gc->Destroy();

                _player->m_playerInfo->charterId[CHARTER_TYPE_GUILD] = 0;
            }
        }

        if(it->GetProto()->ItemId == ARENA_TEAM_CHARTER_2v2)
        {
            if( _player->m_playerInfo->charterId[CHARTER_TYPE_ARENA_2V2] != 0 )
            {
                Charter *gc = guildmgr.GetCharter(_player->m_playerInfo->charterId[CHARTER_TYPE_ARENA_2V2], CHARTER_TYPE_ARENA_2V2);
                if(gc != NULL)
                    gc->Destroy();

                _player->m_playerInfo->charterId[CHARTER_TYPE_ARENA_2V2] = 0;
            }
        }

        if(it->GetProto()->ItemId == ARENA_TEAM_CHARTER_5v5)
        {
            if( _player->m_playerInfo->charterId[CHARTER_TYPE_ARENA_5V5] != 0 )
            {
                Charter *gc = guildmgr.GetCharter(_player->m_playerInfo->charterId[CHARTER_TYPE_ARENA_5V5], CHARTER_TYPE_ARENA_5V5);
                if(gc != NULL)
                    gc->Destroy();

                _player->m_playerInfo->charterId[CHARTER_TYPE_ARENA_5V5] = 0;
            }
        }

        if(it->GetProto()->ItemId == ARENA_TEAM_CHARTER_3v3)
        {
            if( _player->m_playerInfo->charterId[CHARTER_TYPE_ARENA_3V3] != 0 )
            {
                Charter *gc = guildmgr.GetCharter(_player->m_playerInfo->charterId[CHARTER_TYPE_ARENA_3V3], CHARTER_TYPE_ARENA_3V3);
                if(gc != NULL)
                    gc->Destroy();

                _player->m_playerInfo->charterId[CHARTER_TYPE_ARENA_3V3] = 0;
            }
        }

        uint32 mail_id = it->GetTextID();
        if(mail_id)
            _player->m_mailBox->OnMessageCopyDeleted(mail_id);

        Item* pItem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot,SrcSlot,false);
        if(!pItem)
            return;

        sQuestMgr.OnPlayerDropItem(_player, pItem->GetEntry());
        if(_player->GetCurrentSpell() && _player->GetCurrentSpell()->i_caster==pItem)
        {
            _player->GetCurrentSpell()->i_caster=NULLITEM;
            _player->GetCurrentSpell()->cancel();
        }

        pItem->DeleteFromDB();
        pItem->DeleteMe();
        pItem = NULL;
    }
}

void WorldSession::HandleAutoEquipItemOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 2);

    AddItemResult result;
    int8 SrcInvSlot, SrcSlot, error = 0;
    recv_data >> SrcInvSlot >> SrcSlot;

    sLog.outDebug("ITEM: autoequip, Inventory slot: %i Source Slot: %i", SrcInvSlot, SrcSlot);

    Item* eitem=_player->GetItemInterface()->GetInventoryItem(SrcInvSlot,SrcSlot);

    if(!eitem)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, NULLITEM, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    int8 Slot = _player->GetItemInterface()->GetItemSlotByType(eitem->GetProto());
    if(Slot == ITEM_NO_SLOT_AVAILABLE)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem,NULLITEM,INV_ERR_ITEM_CANT_BE_EQUIPPED);
        return;
    }

    if((Slot == EQUIPMENT_SLOT_MAINHAND || Slot == EQUIPMENT_SLOT_OFFHAND) && !(_player->titanGrip || _player->ignoreitemreq_cheat))
    {
        Item* mainhandweapon = _player->GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
        if(mainhandweapon != NULL && mainhandweapon->GetProto()->InventoryType == INVTYPE_2HWEAPON)
        {
            if(Slot == EQUIPMENT_SLOT_OFFHAND && (eitem->GetProto()->InventoryType == INVTYPE_WEAPON
                || eitem->GetProto()->InventoryType == INVTYPE_2HWEAPON))
                Slot = EQUIPMENT_SLOT_MAINHAND;
        }
        else
        {
            if(Slot == EQUIPMENT_SLOT_OFFHAND && eitem->GetProto()->InventoryType == INVTYPE_2HWEAPON)
                Slot = EQUIPMENT_SLOT_MAINHAND;
        }

        if((error = _player->GetItemInterface()->CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET, Slot, eitem->GetProto(), true, true)))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(eitem,NULLITEM, error);
            return;
        }

        if(eitem->GetProto()->InventoryType == INVTYPE_2HWEAPON)
        {
            // see if we have a weapon equipped in the offhand, if so we need to remove it
            Item* offhandweapon = _player->GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND);
            if( offhandweapon != NULL )
            {
                // we need to de-equip this
                SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(offhandweapon->GetProto());
                if( !result.Result )
                {
                    // no free slots for this item
                    _player->GetItemInterface()->BuildInventoryChangeError(eitem,NULLITEM, INV_ERR_BAG_FULL);
                    return;
                }

                offhandweapon = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND, false);
                if( offhandweapon == NULL )
                    return;     // should never happen

                if(!_player->GetItemInterface()->SafeAddItem(offhandweapon, result.ContainerSlot, result.Slot) )
                {
                    if( !_player->GetItemInterface()->AddItemToFreeSlot(offhandweapon) )        // shouldn't happen either.
                    {
                        offhandweapon->DeleteMe();
                        offhandweapon = NULL;
                    }
                }
            }
        }
        else
        {
            // can't equip a non-two-handed weapon with a two-handed weapon
            mainhandweapon = _player->GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
            if( mainhandweapon != NULL && mainhandweapon->GetProto()->InventoryType == INVTYPE_2HWEAPON )
            {
                // we need to de-equip this
                SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(mainhandweapon->GetProto());
                if( !result.Result )
                {
                    // no free slots for this item
                    _player->GetItemInterface()->BuildInventoryChangeError(eitem,NULLITEM, INV_ERR_BAG_FULL);
                    return;
                }

                mainhandweapon = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND, false);
                if( mainhandweapon == NULL )
                    return;     // should never happen

                if( !_player->GetItemInterface()->SafeAddItem(mainhandweapon, result.ContainerSlot, result.Slot) )
                {
                    if( !_player->GetItemInterface()->AddItemToFreeSlot(mainhandweapon) )       // shouldn't happen either.
                    {
                        mainhandweapon->DeleteMe();
                        mainhandweapon = NULL;
                    }
                }
            }
        }
    }
    else
    {
        if((error = _player->GetItemInterface()->CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET, Slot, eitem->GetProto())))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(eitem,NULLITEM, error);
            return;
        }
    }

    if( Slot <= INVENTORY_SLOT_BAG_END )
    {
        if((error = _player->GetItemInterface()->CanEquipItemInSlot(INVENTORY_SLOT_NOT_SET, Slot, eitem->GetProto(), false, false)))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(eitem,NULLITEM, error);
            return;
        }
    }

    Item* oitem = NULLITEM;

    if( SrcInvSlot == INVENTORY_SLOT_NOT_SET )
    {
        _player->GetItemInterface()->SwapItemSlots( SrcSlot, Slot );
    }
    else
    {
        eitem=_player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot,SrcSlot, false);
        oitem=_player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, Slot, false);
        if(oitem)
        {
            result = _player->GetItemInterface()->SafeAddItem(oitem,SrcInvSlot,SrcSlot);
            if(!result)
            {
                printf("HandleAutoEquip: Error while adding item to SrcSlot");
                oitem->DeleteMe();
                oitem = NULL;
            }
        }
        result = _player->GetItemInterface()->SafeAddItem(eitem, INVENTORY_SLOT_NOT_SET, Slot);
        if(!result)
        {
            printf("HandleAutoEquip: Error while adding item to Slot");
            eitem->DeleteMe();
            eitem = NULL;
        }

    }

    if(eitem)
        eitem->Bind(ITEM_BIND_ON_EQUIP);
}

void WorldSession::HandleBuyBackOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 8);
    WorldPacket data(16);
    uint64 guid;
    int32 stuff;
    Item* add ;
    AddItemResult result;
    uint8 error;

    sLog.Debug( "WORLD"," Received CMSG_BUYBACK_ITEM" );

    recv_data >> guid >> stuff;
    stuff -= 74;

    // prevent crashes
    if( stuff > MAX_BUYBACK_SLOT)
        return;

    //what a magical number 69???
    Item* it = _player->GetItemInterface()->GetBuyBack(stuff);
    if (it)
    {
        // Find free slot and break if inv full
        uint32 amount = it->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
        uint32 itemid = it->GetUInt32Value(OBJECT_FIELD_ENTRY);

        add = _player->GetItemInterface()->FindItemLessMax(itemid,amount, false);

        uint32 FreeSlots = _player->GetItemInterface()->CalculateFreeSlots(it->GetProto());
        if ((FreeSlots == 0) && (!add))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_INVENTORY_FULL);
            return;
        }

        // Check for gold
        int32 cost =_player->GetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + stuff);
        if((int32)_player->GetUInt32Value(PLAYER_FIELD_COINAGE) < cost )
        {
            WorldPacket data(SMSG_BUY_FAILED, 12);
            data << uint64(guid);
            data << uint32(itemid);
            data << uint8(2); //not enough money
            SendPacket( &data );
            return;
        }
        // Check for item uniqueness
        if ((error = _player->GetItemInterface()->CanReceiveItem(it->GetProto(), amount, NULL)))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, error);
            return;
        }
        _player->ModUnsigned32Value( PLAYER_FIELD_COINAGE , -cost);
        _player->GetItemInterface()->RemoveBuyBackItem(stuff);

        if (!add)
        {
            it->m_isDirty = true;           // save the item again on logout
            result = _player->GetItemInterface()->AddItemToFreeSlot(it);
            if(!result)
            {
                printf("HandleBuyBack: Error while adding item to free slot");
                it->DeleteMe();
                it = NULL;
            }
        }
        else
        {
            add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + amount);
            add->m_isDirty = true;

            // delete the item
            it->DeleteFromDB();

            // free the pointer
            it->DestroyForPlayer( _player );
            it->DeleteMe();
            it = NULL;
        }

        data.Initialize( SMSG_BUY_ITEM );
        data << uint64(guid);
        data << getMSTime();
        data << uint32(itemid) << uint32(amount);
        SendPacket( &data );
    }
    _player->SaveToDB(false);
}

void WorldSession::HandleSellItemOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 17);
    sLog.Debug( "WORLD"," Received CMSG_SELL_ITEM" );
    if(!GetPlayer())
        return;

    uint64 vendorguid=0, itemguid=0;
    int32 amount=0;

    recv_data >> vendorguid;
    recv_data >> itemguid;
    recv_data >> amount;

    if(_player->isCasting())
        _player->InterruptCurrentSpell();

    // Check if item exists
    if(!itemguid)
    {
        SendSellItem(vendorguid, itemguid, 1);
        return;
    }

    Creature* unit = _player->GetMapMgr()->GetCreature(GUID_LOPART(vendorguid));
    // Check if Vendor exists
    if (unit == NULL)
    {
        SendSellItem(vendorguid, itemguid, 3);
        return;
    }

    Item* item = _player->GetItemInterface()->GetItemByGUID(itemguid);
    if(!item)
    {
        SendSellItem(vendorguid, itemguid, 1);
        return; //our player doesn't have this item
    }

    ItemPrototype *it = item->GetProto();
    if(!it)
    {
        SendSellItem(vendorguid, itemguid, 2);
        return; //our player doesn't have this item
    }

    if(item->IsContainer() && TO_CONTAINER(item)->HasItems())
    {
        SendSellItem(vendorguid, itemguid, 6);
        return;
    }

    // Check if item can be sold
    if( it->SellPrice == 0 || item->wrapped_item_id != 0 )
    {
        SendSellItem(vendorguid, itemguid, 2);
        return;
    }

    uint32 stackcount = item->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
    uint32 quantity = 0;

    if (amount != 0)
        quantity = amount;
    else
        quantity = stackcount; //allitems

    if(quantity > stackcount)
        quantity = stackcount; //make sure we don't over do it

    uint32 price = Item::GetSellPriceForItem(it, quantity);

    _player->ModUnsigned32Value(PLAYER_FIELD_COINAGE,price);

    if(quantity < stackcount)
    {
        item->SetCount(stackcount - quantity);
        item->m_isDirty = true;
    }
    else
    {
        //removing the item from the char's inventory
        item = _player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(itemguid, false); //again to remove item from slot
        if(item)
        {
            sQuestMgr.OnPlayerDropItem(_player, item->GetEntry());
            _player->GetItemInterface()->AddBuyBackItem(item, (it->SellPrice) * quantity);
            item->DeleteFromDB();
        }
    }

    WorldPacket data(SMSG_SELL_ITEM, 12);
    data << vendorguid << itemguid << uint8(0);
    SendPacket( &data );

    sLog.Debug( "WORLD"," Sent SMSG_SELL_ITEM" );

    _player->SaveToDB(false);
}

void WorldSession::HandleBuyItemOpcode( WorldPacket & recv_data ) // right-click on item
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 14);
    sLog.Debug( "WORLD"," Received CMSG_BUY_ITEM" );

    if(!GetPlayer())
        return;

    uint64 vendorGuid;
    uint32 itemid, slot, count;
    uint8 error = 0;

    recv_data >> vendorGuid;
    recv_data.read_skip<uint8>();
    recv_data >> itemid >> slot >> count;
    if(count < 1) count = 1;
    recv_data.read_skip<uint64>();
    recv_data.read_skip<uint8>();

    Creature* unit = _player->GetMapMgr()->GetCreature(GUID_LOPART(vendorGuid));
    if (unit == NULL || !unit->HasItems())
        return;

    CreatureItem item;
    unit->GetSellItemByItemId(itemid, item);
    if(item.itemid == 0)
    {
        // vendor does not sell this item.. bitch about cheaters?
        _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_DONT_OWN_THAT_ITEM);
        return;
    }

    if (item.max_amount > 0 && item.available_amount < count)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_ITEM_IS_CURRENTLY_SOLD_OUT);
        return;
    }

    ItemPrototype *it = ItemPrototypeStorage.LookupEntry(itemid);
    if(!it)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_DONT_OWN_THAT_ITEM);
        return;
    }

    if( it->MaxCount > 0 && count > it->MaxCount )
    {
        _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_ITEM_CANT_STACK);
        return;
    }

    if((error = _player->GetItemInterface()->CanReceiveItem(it, count*item.amount, item.extended_cost)))
    {
        _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, error);
        return;
    }

    if((error = _player->GetItemInterface()->CanAffordItem(it, count, unit, item.extended_cost)))
    {
        SendBuyFailed(vendorGuid, itemid, error);
        return;
    }

    SlotResult slotresult;
    // Find free slot and break if inv full
    Item *add = _player->GetItemInterface()->FindItemLessMax(itemid, count*item.amount, false);
    if (!add) slotresult = _player->GetItemInterface()->FindFreeInventorySlot(it);
    if ((!slotresult.Result) && (!add))
    {
        //Our User doesn't have a free Slot in there bag
        _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_INVENTORY_FULL);
        return;
    }

    if(!add)
    {
        Item* itm = objmgr.CreateItem(item.itemid, _player);
        if(!itm)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_DONT_OWN_THAT_ITEM);
            return;
        }

        itm->m_isDirty = true;
        int32 amt = count*item.amount;
        if(itm->GetProto()->MaxCount > 0)
            if(amt > itm->GetProto()->MaxCount)
                amt = itm->GetProto()->MaxCount;
        itm->SetUInt32Value(ITEM_FIELD_STACK_COUNT, amt);

        if(slotresult.ContainerSlot == ITEM_NO_SLOT_AVAILABLE)
        {
            AddItemResult result = _player->GetItemInterface()->SafeAddItem(itm, INVENTORY_SLOT_NOT_SET, slotresult.Slot);
            if(!result)
            {
                itm->DeleteMe();
                itm = NULL;
            }
            else
                SendItemPushResult(itm, false, true, false, true,(uint8)INVENTORY_SLOT_NOT_SET, slotresult.Result, count*item.amount);
        }
        else
        {
            if( Item* bag = _player->GetItemInterface()->GetInventoryItem(slotresult.ContainerSlot))
            {
                if( !TO_CONTAINER(bag)->AddItem(slotresult.Slot, itm) )
                {
                    itm->DeleteMe();
                    itm = NULL;
                }
                else
                    SendItemPushResult(itm, false, true, false, true, slotresult.ContainerSlot, slotresult.Result, 1);
            }
        }
    }
    else
    {
        add->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT, count*item.amount);
        add->m_isDirty = true;
        SendItemPushResult(add, false, true, false, false, _player->GetItemInterface()->GetBagSlotByGuid(add->GetGUID()), 1, count*item.amount);
    }

    WorldPacket data(SMSG_BUY_ITEM, 45);
    data << uint64(vendorGuid);
    data << getMSTime();
    data << uint32(itemid) << uint32(count*item.amount);
    SendPacket( &data );

    _player->GetItemInterface()->BuyItem(it,count,unit, item.extended_cost);
    if(int32(item.max_amount) > 0)
    {
        unit->ModAvItemAmount(item.itemid,item.amount*count);

        // there is probably a proper opcode for this. - burlex
        SendInventoryList(unit);
    }

    _player->SaveToDB(false);
}

void WorldSession::HandleListInventoryOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    CHECK_PACKET_SIZE(recv_data, 8);
    sLog.Debug( "WORLD"," Recvd CMSG_LIST_INVENTORY" );
    uint64 guid;

    recv_data >> guid;

    Creature* unit = _player->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if (unit == NULL)
        return;

    if(unit->GetAIInterface())
        unit->GetAIInterface()->StopMovement(180000);

    _player->Reputation_OnTalk(unit->m_faction);
    SendInventoryList(unit);
}

void WorldSession::SendInventoryList(Creature* unit)
{
    if(!_player || !_player->IsInWorld())
        return;

    uint32 counter = 0;
    if(!unit->HasItems())
    {
        WorldPacket data(SMSG_LIST_INVENTORY, 10);
        data << uint64(unit->GetGUID());
        data << uint8(0) << uint8(0);
        SendPacket(&data);
        return;
    }

    ItemPrototype * curItem;
    WorldPacket data(SMSG_LIST_INVENTORY, ((unit->GetSellItemCount() * 28) + 9));      // allocate
    data << unit->GetGUID();
    data << uint8( 0 ); // placeholder for item count
    for(std::map<uint32, CreatureItem>::iterator itr = unit->GetSellItemBegin(); itr != unit->GetSellItemEnd(); itr++)
    {
        if(counter >= 150)
        {
            sLog.Error("VendorListing", "Creature %u contains too many items, Displaying (150/%u) items.",
                unit->GetEntry(), uint32(unit->GetSellItemCount()));
            break;
        }

        if(itr->second.itemid && (itr->second.max_amount == 0 || (itr->second.max_amount > 0 && itr->second.available_amount > 0)))
        {
            if((curItem = ItemPrototypeStorage.LookupEntry(itr->second.itemid)))
            {
                if(!(itr->second.vendormask & unit->VendorMask))
                    continue;

                if(!_player->ignoreitemreq_cheat)
                {
                    if(itr->second.IsDependent)
                    {
                        if(curItem->AllowableClass && !(_player->getClassMask() & curItem->AllowableClass))
                            continue;
                        if(curItem->AllowableRace && !(_player->getRaceMask() & curItem->AllowableRace))
                            continue;

                        if(curItem->Class == ITEM_CLASS_ARMOR && curItem->SubClass >= ITEM_SUBCLASS_ARMOR_LIBRAM && curItem->SubClass <= ITEM_SUBCLASS_ARMOR_SIGIL)
                            if(!(_player->GetArmorProficiency() & (uint32(1) << curItem->SubClass)))
                                continue; // Do not show relics to classes that can't use them.
                    }

                    if(itr->second.extended_cost == NULL && curItem->SellPrice > curItem->BuyPrice )
                        continue;
                }

                uint32 extendedCostId = itr->second.extended_cost != NULL ? itr->second.extended_cost->Id : 0;
                int32 av_am = (itr->second.max_amount > 0) ? itr->second.available_amount : -1;
                data << (++counter);
                data << uint32(1);
                data << curItem->ItemId;
                data << curItem->DisplayInfoID;
                data << av_am;
                data << Item::GetBuyPriceForItem(curItem, 1, _player, unit);
                data << curItem->MaxDurability;
                data << itr->second.amount;
                data << extendedCostId;
                data << uint8(0);
            }
        }
    }

    if(counter == 0) data << uint8(0);
    else data.put<uint8>(8, counter); // set count

    SendPacket( &data );
    sLog.Debug( "WORLD"," Sent SMSG_LIST_INVENTORY" );
}

void WorldSession::HandleAutoStoreBagItemOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 3);
    sLog.Debug( "WORLD"," Recvd CMSG_AUTO_STORE_BAG_ITEM" );

    if(!GetPlayer())
        return;

    //WorldPacket data;
    WorldPacket packet;
    int8 SrcInv=0, Slot=0, DstInv=0;
    Item* srcitem = NULLITEM;
    Item* dstitem= NULLITEM;
    int8 NewSlot = 0;
    int8 error;
    AddItemResult result;

    recv_data >> SrcInv >> Slot >> DstInv;

    srcitem = _player->GetItemInterface()->GetInventoryItem(SrcInv, Slot);

    //source item exists
    if(srcitem)
    {
        //src containers cant be moved if they have items inside
        if(srcitem->IsContainer() && TO_CONTAINER(srcitem)->HasItems())
        {
            _player->GetItemInterface()->BuildInventoryChangeError(srcitem, NULLITEM, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
            return;
        }
        //check for destination now before swaping.
        //destination is backpack
        if(DstInv == INVENTORY_SLOT_NOT_SET)
        {
            //check for space
            NewSlot = _player->GetItemInterface()->FindFreeBackPackSlot();
            if(NewSlot == ITEM_NO_SLOT_AVAILABLE)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(srcitem, NULLITEM, INV_ERR_BAG_FULL);
                return;
            }
            else
            {
                //free space found, remove item and add it to the destination
                srcitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInv, Slot, false);
                if( srcitem )
                {
                    result = _player->GetItemInterface()->SafeAddItem(srcitem, INVENTORY_SLOT_NOT_SET, NewSlot);
                    if(!result)
                    {
                        printf("HandleAutoStoreBagItem: Error while adding item to newslot");
                        srcitem->DeleteMe();
                        srcitem = NULL;
                        return;
                    }
                }
            }
        }
        else
        {
            if((error=_player->GetItemInterface()->CanEquipItemInSlot(DstInv,  DstInv, srcitem->GetProto())))
            {
                if(DstInv < MAX_INVENTORY_SLOT)
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(srcitem,dstitem, error);
                    return;
                }
            }

            //destination is a bag
            dstitem = _player->GetItemInterface()->GetInventoryItem(DstInv);
            if(dstitem)
            {
                //dstitem exists, detect if its a container
                if(dstitem->IsContainer())
                {
                    NewSlot = TO_CONTAINER(dstitem)->FindFreeSlot();
                    if(NewSlot == ITEM_NO_SLOT_AVAILABLE)
                    {
                        _player->GetItemInterface()->BuildInventoryChangeError(srcitem, NULLITEM, INV_ERR_BAG_FULL);
                        return;
                    }
                    else
                    {
                        srcitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInv, Slot, false);
                        if( srcitem != NULL )
                        {
                            result = _player->GetItemInterface()->SafeAddItem(srcitem, DstInv, NewSlot);
                            if(!result)
                            {
                                printf("HandleBuyItemInSlot: Error while adding item to newslot");
                                srcitem->DeleteMe();
                                srcitem = NULL;
                                return;
                            }
                        }
                    }
                }
                else
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(srcitem, NULLITEM,  INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
                    return;
                }
            }
            else
            {
                _player->GetItemInterface()->BuildInventoryChangeError(srcitem, NULLITEM, INV_ERR_ITEM_DOESNT_GO_TO_SLOT);
                return;
            }
        }
    }
    else
    {
        _player->GetItemInterface()->BuildInventoryChangeError(srcitem, NULLITEM, INV_ERR_ITEM_NOT_FOUND);
        return;
    }
}

void WorldSession::HandleReadItemOpcode(WorldPacket &recvPacket)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recvPacket, 2);
    int8 uslot=0, slot=0;
    recvPacket >> uslot >> slot;

    if(!GetPlayer())
        return;

    Item* item = _player->GetItemInterface()->GetInventoryItem(uslot, slot);
    sLog.Debug("WorldSession","Received CMSG_READ_ITEM %d", slot);

    if(item)
    {
        if(item->GetProto()->PageId)
        {
            WorldPacket data(SMSG_READ_ITEM_OK, 8);
            data << item->GetGUID();
            SendPacket(&data);
            sLog.Debug("WorldSession","Sent SMSG_READ_OK %d", item->GetGUID());
        }
    }
}

HEARTHSTONE_INLINE uint32 RepairItemCost(Player* pPlayer, Item* pItem)
{
    DurabilityCostsEntry * dcosts = dbcDurabilityCosts.LookupEntry(pItem->GetProto()->ItemLevel);
    if(!dcosts)
    {
        sLog.outDebug("Repair: Unknown item level (%u)", dcosts);
        return 1;
    }

    DurabilityQualityEntry * dquality = dbcDurabilityQuality.LookupEntry((pItem->GetProto()->Quality + 1) * 2);
    if(!dquality)
    {
        sLog.outDebug("Repair: Unknown item quality (%u)", pItem->GetProto()->Quality);
        return 1;
    }

    uint32 dmodifier = dcosts->modifier[pItem->GetProto()->Class == ITEM_CLASS_WEAPON ? pItem->GetProto()->SubClass : pItem->GetProto()->SubClass + 21];
    uint32 cost = long2int32((pItem->GetDurabilityMax() - pItem->GetDurability()) * dmodifier * double(dquality->quality_modifier));
    return cost;
}

HEARTHSTONE_INLINE void RepairItem(Player* pPlayer, Item* pItem, bool guild = false)
{
    int32 cost = RepairItemCost(pPlayer, pItem);
    if( cost < 0 )
        return;

    if( cost > (int32)pPlayer->GetUInt32Value( PLAYER_FIELD_COINAGE ) )
        return;

/*  if(guild)
    {
        uint32 amountavailable = pPlayer->GetGuild()->GetBankBalance();
        uint32 amountallowed = pPlayer->GetGuildMember()->CalculateAvailableAmount();
        uint32 available = (amountallowed == 0xFFFFFFFF ? amountavailable : amountallowed);
        uint64 totalamount = (pPlayer->GetUInt32Value(PLAYER_FIELD_COINAGE) + available);
        if(totalamount < pPlayer->GuildBankCost+cost)
            return;

        pPlayer->GuildBankCost += cost;
    }
    else*/ if(cost)
        pPlayer->ModUnsigned32Value( PLAYER_FIELD_COINAGE, -cost );

    pItem->SetDurabilityToMax();
    pItem->m_isDirty = true;
}

void WorldSession::HandleRepairItemOpcode(WorldPacket &recvPacket)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recvPacket, 12);

    uint64 npcguid, itemguid;
    bool guildmoney;
    Item* pItem;
    Container* pContainer;
    int32 j, i;

    recvPacket >> npcguid >> itemguid >> guildmoney;
    Creature* pCreature = _player->GetMapMgr()->GetCreature( GUID_LOPART(npcguid) );
    if( pCreature == NULL )
        return;

    if(guildmoney)
        return; // Fucking bastards

    if( !pCreature->HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_ARMORER ) )
        return;

    if( !itemguid )
    {
        for( i = 0; i < INVENTORY_SLOT_BAG_END; i++ )
        {
            pItem = _player->GetItemInterface()->GetInventoryItem( i );
            if( pItem != NULL )
            {
                if( pItem->IsContainer() )
                {
                    pContainer = TO_CONTAINER( pItem );
                    for( j = 0; j < pContainer->GetProto()->ContainerSlots; ++j )
                    {
                        pItem = pContainer->GetItem( j );
                        if( pItem != NULL )
                            RepairItem( _player, pItem, guildmoney );
                    }
                }
                else
                {
                    if( pItem->GetProto()->MaxDurability > 0 && i < INVENTORY_SLOT_BAG_END && pItem->GetDurability() <= 0 )
                    {
                        RepairItem( _player, pItem, guildmoney );
                        _player->ApplyItemMods( pItem, i, true );
                    }
                    else
                        RepairItem( _player, pItem, guildmoney );
                }
            }
        }

/*      if(guildmoney)
        {   // Just grab the money.
            uint32 amountavailable = _player->GetGuild()->GetBankBalance();
            uint32 amountallowed = _player->GetGuildMember()->CalculateAvailableAmount();
            uint32 available = (amountallowed == 0xFFFFFFFF ? amountavailable : amountallowed); // If we have an unlimited amount, take the max.
            if(available)
            {
                if(_player->GuildBankCost > available)
                    _player->GetGuild()->WithdrawMoney(this, available);
                else
                    _player->GetGuild()->WithdrawMoney(this, _player->GuildBankCost);
            }

            _player->ModUnsigned32Value( PLAYER_FIELD_COINAGE , (-(int32)(_player->GuildBankCost)) );
            _player->GuildBankCost = 0; // Reset our guild cost.
        }*/
    }
    else
    {
        Item* item = _player->GetItemInterface()->GetItemByGUID(itemguid);
        if(item)
        {
            SlotResult *searchres=_player->GetItemInterface()->LastSearchResult();//this never gets null since we get a pointer to the inteface internal var
            uint32 dDurability = item->GetDurabilityMax() - item->GetDurability();

            if (dDurability)
            {
                // the amount of durability that is needed to be added is the amount of money to be payed
                if (dDurability <= _player->GetUInt32Value(PLAYER_FIELD_COINAGE))
                {
                    int32 cDurability = item->GetDurability();
                    /*if(guildmoney) // Just grab the money.
                    {
                        uint32 amountavailable = _player->GetGuild()->GetBankBalance();
                        uint32 amountallowed = _player->GetGuildMember()->CalculateAvailableAmount();
                        uint32 available = (amountallowed == 0xFFFFFFFF ? amountavailable : amountallowed); // If we have an unlimited amount, take the max.
                        _player->GetGuild()->WithdrawMoney(this, available);
                    }*/

                    _player->ModUnsigned32Value( PLAYER_FIELD_COINAGE , -(int32)dDurability );
                    item->SetDurabilityToMax();
                    item->m_isDirty = true;

                    //only apply item mods if they are on char equiped
                    if(cDurability <= 0 && searchres->ContainerSlot==INVALID_BACKPACK_SLOT && searchres->Slot<INVENTORY_SLOT_BAG_END)
                        _player->ApplyItemMods(item, searchres->Slot, true);
                }
                else
                {
                    /*if(guildmoney)
                    {
                        int32 cDurability = item->GetDurability();
                        uint32 amountavailable = _player->GetGuild()->GetBankBalance();
                        uint32 amountallowed = _player->GetGuildMember()->CalculateAvailableAmount();
                        uint32 available = (amountallowed == 0xFFFFFFFF ? amountavailable : amountallowed); // If we have an unlimited amount, take the max.
                        if(cDurability < int64(_player->GetUInt32Value(PLAYER_FIELD_COINAGE) + available))
                        {
                            _player->GetGuild()->WithdrawMoney(this, available);
                            _player->ModUnsigned32Value( PLAYER_FIELD_COINAGE , -(int32)dDurability );
                            item->SetDurabilityToMax();
                            item->m_isDirty = true;

                            //only apply item mods if they are on char equiped
                            if(cDurability <= 0 && searchres->ContainerSlot == INVALID_BACKPACK_SLOT && searchres->Slot < INVENTORY_SLOT_BAG_END)
                                _player->ApplyItemMods(item, searchres->Slot, true);
                        }
                    }*/
                }
            }
        }
    }
    sLog.Debug("WorldSession","Received CMSG_REPAIR_ITEM %d, %s", itemguid, guildmoney ? "From Guild" : "From Player");
}

void WorldSession::HandleBuyBankSlotOpcode(WorldPacket& recvPacket)
{
    sLog.Debug("WorldSession","Received CMSG_BUY_bytes_SLOT");
    CHECK_INWORLD_RETURN();

    uint8 currentSlot = _player->GetByte(PLAYER_BYTES_2, 3);
    BankSlotPriceEntry* bsp = dbcBankSlotPrices.LookupEntry(currentSlot+1);
    if(bsp == NULL)
        return;

    sLog.Debug("WorldSession","HandleBuyBankSlotOpcode: slot number = %d", currentSlot);
    if (_player->GetUInt32Value(PLAYER_FIELD_COINAGE) >= bsp->Price)
    {
        _player->SetByte(PLAYER_BYTES_2, 3, currentSlot+1);
        _player->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -((int32)bsp->Price));
        _player->GetAchievementInterface()->HandleAchievementCriteriaBuyBankSlot();
    }
}

void WorldSession::HandleAutoBankItemOpcode(WorldPacket &recvPacket)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recvPacket, 2);
    sLog.Debug("WorldSession","Received CMSG_AUTO_BANK_ITEM");

    //WorldPacket data;

    SlotResult slotresult;
    int8 SrcInvSlot, SrcSlot;//, error=0;

    if(!GetPlayer())
        return;

    recvPacket >> SrcInvSlot >> SrcSlot;

    sLog.Debug("WorldSession","HandleAutoBankItemOpcode: Inventory slot: %u Source Slot: %u", (uint32)SrcInvSlot, (uint32)SrcSlot);

    Item* eitem=_player->GetItemInterface()->GetInventoryItem(SrcInvSlot,SrcSlot);

    if(!eitem)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, NULLITEM, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    slotresult =  _player->GetItemInterface()->FindFreeBankSlot(eitem->GetProto());

    if(!slotresult.Result)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, NULLITEM, INV_ERR_BANK_FULL);
        return;
    }
    else
    {
        eitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot,SrcSlot, false);
        if(!_player->GetItemInterface()->SafeAddItem(eitem, slotresult.ContainerSlot, slotresult.Slot))
        {
            sLog.outDebug("[ERROR]AutoBankItem: Error while adding item to bank bag!\n");
            if( !_player->GetItemInterface()->SafeAddItem(eitem, SrcInvSlot, SrcSlot) )
            {
                eitem->DeleteMe();
                eitem = NULL;
            }
        }
    }
}

void WorldSession::HandleAutoStoreBankItemOpcode(WorldPacket &recvPacket)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recvPacket, 2);
    sLog.outDebug("WORLD: CMSG_AUTOSTORE_BANK_ITEM");

    //WorldPacket data;

    int8 SrcInvSlot, SrcSlot;//, error=0, slot=-1, specialbagslot=-1;

    if(!GetPlayer())
        return;

    recvPacket >> SrcInvSlot >> SrcSlot;

    sLog.outDebug("ITEM: AutoStore Bank Item, Inventory slot: %i Source Slot: %i", SrcInvSlot, SrcSlot);

    Item* eitem = _player->GetItemInterface()->GetInventoryItem(SrcInvSlot,SrcSlot);

    if(!eitem)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, NULLITEM, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    SlotResult slotresult = _player->GetItemInterface()->FindFreeInventorySlot(eitem->GetProto());

    if(!slotresult.Result)
    {
        _player->GetItemInterface()->BuildInventoryChangeError(eitem, NULLITEM, INV_ERR_INVENTORY_FULL);
        return;
    }
    else
    {
        eitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot, SrcSlot, false);
        if (!_player->GetItemInterface()->AddItemToFreeSlot(eitem))
        {
            sLog.outDebug("[ERROR]AutoStoreBankItem: Error while adding item from one of the bank bags to the player bag!\n");
            if( !_player->GetItemInterface()->SafeAddItem(eitem, SrcInvSlot, SrcSlot) )
            {
                eitem->Destruct();
                eitem = NULL;
            }
        }
        _player->SaveToDB(false);
    }
}

void WorldSession::HandleCancelTemporaryEnchantmentOpcode(WorldPacket &recvPacket)
{
    CHECK_INWORLD_RETURN();
    uint32 inventory_slot;
    recvPacket >> inventory_slot;

    Item* item = _player->GetItemInterface()->GetInventoryItem(inventory_slot);
    if(!item) return;

    item->RemoveAllEnchantments(true);
}

int32 ConvertDB2DBCGemType(uint32 DBGemType)
{
    uint32 DBCGemType = -1;
    switch(DBGemType)
    {
    case ITEM_SUBCLASS_GEM_RED: DBCGemType = 2; break;
    case ITEM_SUBCLASS_GEM_BLUE: DBCGemType = 8; break;
    case ITEM_SUBCLASS_GEM_YELLOW: DBCGemType = 4; break;
    case ITEM_SUBCLASS_GEM_PURPLE: DBCGemType = 10; break;
    case ITEM_SUBCLASS_GEM_GREEN: DBCGemType = 12; break;
    case ITEM_SUBCLASS_GEM_ORANGE: DBCGemType = 6; break;
    case ITEM_SUBCLASS_GEM_META: DBCGemType = 1; break;
    case ITEM_SUBCLASS_GEM_SIMPLE: DBCGemType = -1; break;
    case ITEM_SUBCLASS_GEM_PRISMATIC: DBCGemType = 14; break;
    }
    return DBCGemType;
}

void WorldSession::HandleInsertGemOpcode(WorldPacket &recvPacket)
{
    uint64 itemguid;
    uint64 gemguid[3];
    GemPropertyEntry * gp = NULL;
    EnchantEntry * Enchantment;
    recvPacket >> itemguid;

    CHECK_INWORLD_RETURN();

    Item* TargetItem =_player->GetItemInterface()->GetItemByGUID(itemguid);
    if(TargetItem == NULL)
        return;

    int slot = _player->GetItemInterface()->GetInventorySlotByGuid(itemguid);
    bool apply = (slot >= 0 && slot < 19);
    uint32 FilledSlots = 0;

    /* The following is a hack check to make sure player's aren't socketing more than they have,
        while still allowing socketing of items with prismatic sockets. */
    bool sockenchgloves = (TargetItem->HasEnchantment(3723) && TargetItem->GetProto()->InventoryType == 10);
    bool sockenchbracer = (TargetItem->HasEnchantment(3717) && TargetItem->GetProto()->InventoryType == 9);
    bool sockenchbelt = (TargetItem->HasEnchantment(3729) && TargetItem->GetProto()->InventoryType == 6);

    bool ColorMatch[3];
    for(uint32 i = 0; i < 3; i++)
    {
        recvPacket >> gemguid[i];

        if(i > ((sockenchgloves || sockenchbracer || sockenchbelt) ? TargetItem->GetMaxSocketsCount() + 1 : TargetItem->GetMaxSocketsCount()))
            continue;

        ColorMatch[i] = false;

        EnchantmentInstance * EI = TargetItem->GetEnchantment(2+i);
        if(EI)
        {
            gp = NULL;
            FilledSlots++;
            ItemPrototype * ip = ItemPrototypeStorage.LookupEntry(EI->Enchantment->GemEntry);
            if(ip != NULL)
                gp = dbcGemProperty.LookupEntry(ip->GemProperties);

            if(gp && !(gp->SocketMask & TargetItem->GetProto()->ItemSocket[i]) && TargetItem->GetProto()->ItemSocket[i] != 0)
                ColorMatch[i] = false;
        }

        if(gemguid[i])//add or replace gem
        {
            ItemInterface *itemi = _player->GetItemInterface();
            Item *it = itemi ? itemi->GetItemByGUID(gemguid[i]) : NULL;
            ItemPrototype * ip = it ? it->GetProto() : NULL;
            if( it == NULL || ip == NULL)
                continue;

            if (apply)
            {
                if(ip->GemProperties == 0) // Incomplete DB, but we have cached.
                    continue;

                if( ip->Flags & ITEM_FLAG_UNIQUE_EQUIP && itemi->IsEquipped( ip->ItemId ) )
                {
                    itemi->BuildInventoryChangeError( it, TargetItem, INV_ERR_CANT_CARRY_MORE_OF_THIS );
                    continue;
                }

                // Skill requirement
                if( ip->RequiredSkill > 0 )
                {
                    if( (uint32)ip->RequiredSkillRank > _player->_GetSkillLineCurrent( ip->RequiredSkill, true ) )
                    {
                        itemi->BuildInventoryChangeError( it, TargetItem, INV_ERR_SKILL_ISNT_HIGH_ENOUGH );
                        continue;
                    }
                }

                if( ip->ItemLimitCategory )
                {
                    ItemLimitCategoryEntry * il = dbcItemLimitCategory.LookupEntry( ip->ItemLimitCategory );
                    if( il != NULL && itemi->GetSocketedGemCountWithLimitId( ip->ItemLimitCategory ) >= il->MaxAmount )
                    {
                        itemi->BuildInventoryChangeError(it, TargetItem, INV_ERR_ITEM_MAX_COUNT_EQUIPPED_SOCKETED);
                        continue;
                    }
                }
            }

            it = _player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(gemguid[i], true);
            if(it == NULL)
            {
                itemi->BuildInventoryChangeError( it, TargetItem, INV_ERR_OBJECT_IS_BUSY );
                continue;
            }

            ip = it->GetProto();
            if(ip == NULL)
            {
                itemi->BuildInventoryChangeError( it, TargetItem, INV_ERR_OBJECT_IS_BUSY );
                continue;
            }
            sQuestMgr.OnPlayerDropItem(_player, ip->ItemId);
            it->DeleteMe();
            it = NULL;

            if(EI)//replace gem
                TargetItem->RemoveEnchantment(2+i);//remove previous
            else//add gem
                FilledSlots++;

            uint32 EnchantID = 0;
            gp = dbcGemProperty.LookupEntry(ip->GemProperties);
            if(gp != NULL)
            {
                if(!(gp->SocketMask & TargetItem->GetProto()->ItemSocket[i]))
                    ColorMatch[i] = false;
                Enchantment = dbcEnchant.LookupEntry(gp->EnchantmentID);
                if(gp->EnchantmentID && Enchantment != NULL)
                    TargetItem->AddEnchantment(Enchantment, 0, true,apply,false,2+i);
            }
            else
            {   // Lacking DBC data, pull from proto.
                uint32 gemmask = ConvertDB2DBCGemType(ip->SubClass);
                if(gemmask == -1 || !(gemmask & TargetItem->GetProto()->ItemSocket[i]))
                    ColorMatch[i] = false;

                if(ip->GemProperties < 0)
                {   // If we're negative, its a dummy gem.
                    Enchantment = dbcEnchant.LookupEntry(-ip->GemProperties);
                    if(Enchantment != NULL)
                        TargetItem->AddEnchantment(Enchantment, 0, true,apply,false,2+i,0,true);
                }
            }

            TargetItem->Gems[i] = ip->ItemId;
        }
    }

    bool truecolormatch = false;

    for(uint32 i = 0; i < 3; i++)
    {
        if(TargetItem->GetProto()->ItemSocket[i])
        {
            if(i <= TargetItem->GetMaxSocketsCount())
            {
                if(ColorMatch[i] == true)
                    truecolormatch = true;
                else
                    truecolormatch = false;
            }
        }
    }

    //Add color match bonus
    if(TargetItem->GetProto()->SocketBonus)
    {
        if(truecolormatch && (FilledSlots >= TargetItem->GetMaxSocketsCount()))
        {
            if(TargetItem->HasEnchantment(TargetItem->GetProto()->SocketBonus) > 0)
                return;

            Enchantment = dbcEnchant.LookupEntry(TargetItem->GetProto()->SocketBonus);
            if(Enchantment)
            {
                uint32 Slot = TargetItem->FindFreeEnchantSlot(Enchantment,0);
                TargetItem->AddEnchantment(Enchantment, 0, true,apply,false, Slot);
            }
        }
        else //remove
            TargetItem->RemoveSocketBonusEnchant();
    }

    TargetItem->m_isDirty = true;
}

void WorldSession::HandleWrapItemOpcode( WorldPacket& recv_data )
{
    int8 sourceitem_bagslot, sourceitem_slot;
    int8 destitem_bagslot, destitem_slot;
    uint32 source_entry;
    uint32 itemid;
    Item* src;
    Item* dst;

    recv_data >> sourceitem_bagslot >> sourceitem_slot;
    recv_data >> destitem_bagslot >> destitem_slot;

    CHECK_INWORLD_RETURN();

    src = _player->GetItemInterface()->GetInventoryItem( sourceitem_bagslot, sourceitem_slot );
    dst = _player->GetItemInterface()->GetInventoryItem( destitem_bagslot, destitem_slot );

    if( !src || !dst )
        return;

    if(src == dst || !(src->GetProto()->Class == ITEM_CLASS_CONSUMABLE && src->GetProto()->SubClass == ITEM_SUBCLASS_CONSUMABLE_OTHER))
    {
        _player->GetItemInterface()->BuildInventoryChangeError( src, dst, INV_ERR_WRAPPED_CANT_BE_WRAPPED );
        return;
    }

    if( dst->GetUInt32Value( ITEM_FIELD_STACK_COUNT ) > 1 )
    {
        _player->GetItemInterface()->BuildInventoryChangeError( src, dst, INV_ERR_STACKABLE_CANT_BE_WRAPPED );
        return;
    }

    if( dst->GetProto()->MaxCount > 1 )
    {
        _player->GetItemInterface()->BuildInventoryChangeError( src, dst, INV_ERR_STACKABLE_CANT_BE_WRAPPED );
        return;
    }

    if( dst->IsAccountbound() || dst->IsSoulbound() || dst->GetProto()->Bonding != 0)
    {
        _player->GetItemInterface()->BuildInventoryChangeError( src, dst, INV_ERR_BOUND_CANT_BE_WRAPPED );
        return;
    }

    if( dst->wrapped_item_id || src->wrapped_item_id )
    {
        _player->GetItemInterface()->BuildInventoryChangeError( src, dst, INV_ERR_WRAPPED_CANT_BE_WRAPPED );
        return;
    }

    if( dst->GetProto()->Unique )
    {
        _player->GetItemInterface()->BuildInventoryChangeError( src, dst, INV_ERR_UNIQUE_CANT_BE_WRAPPED );
        return;
    }

    if( dst->IsContainer() )
    {
        _player->GetItemInterface()->BuildInventoryChangeError( src, dst, INV_ERR_BAGS_CANT_BE_WRAPPED );
        return;
    }

    if( dst->HasEnchantments() )
    {
        _player->GetItemInterface()->BuildInventoryChangeError( src, dst, INV_ERR_ITEM_LOCKED );
        return;
    }
    if( destitem_bagslot == (int8)0xFF && ( destitem_slot >= EQUIPMENT_SLOT_START && destitem_slot <= INVENTORY_SLOT_BAG_END ) )
    {
        _player->GetItemInterface()->BuildInventoryChangeError( src, dst, INV_ERR_EQUIPPED_CANT_BE_WRAPPED );
        return;
    }

    // all checks passed ok
    source_entry = src->GetEntry();
    itemid = source_entry;
    switch( source_entry )
    {
    case 5042:
        itemid = 5043;
        break;

    case 5048:
        itemid = 5044;
        break;

    case 17303:
        itemid = 17302;
        break;

    case 17304:
        itemid = 17305;
        break;

    case 17307:
        itemid = 17308;
        break;

    case 21830:
        itemid = 21831;
        break;

    default:
        _player->GetItemInterface()->BuildInventoryChangeError( src, dst, INV_ERR_WRAPPED_CANT_BE_WRAPPED );
        return;
        break;
    }

    dst->SetProto( src->GetProto() );

    if( src->GetUInt32Value( ITEM_FIELD_STACK_COUNT ) <= 1 )
    {
        // destroy the source item
        _player->GetItemInterface()->SafeFullRemoveItemByGuid( src->GetGUID() );
    }
    else
    {
        // reduce stack count by one
        src->ModUnsigned32Value( ITEM_FIELD_STACK_COUNT, -1 );
        src->m_isDirty = true;
    }

    // change the dest item's entry
    dst->wrapped_item_id = dst->GetEntry();
    dst->SetUInt32Value( OBJECT_FIELD_ENTRY, itemid );

    // set the giftwrapper fields
    dst->SetUInt32Value( ITEM_FIELD_GIFTCREATOR, _player->GetLowGUID() );
    dst->SetUInt32Value( ITEM_FIELD_DURABILITY, 0 );
    dst->SetUInt32Value( ITEM_FIELD_MAXDURABILITY, 0 );
    dst->SetUInt32Value( ITEM_FIELD_FLAGS, 0x8008 );

    // save it
    dst->m_isDirty = true;
    dst->SaveToDB( destitem_bagslot, destitem_slot, false, NULL );
}


