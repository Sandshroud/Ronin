/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleSplitOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();

    int8 DstInvSlot=0, DstSlot=0, SrcInvSlot=0, SrcSlot=0;
    uint8 count=0;

    AddItemResult result;

    recv_data >> SrcInvSlot >> SrcSlot >> DstInvSlot >> DstSlot >> count;
    if(!GetPlayer())
        return;

    if(count >= 127) /* exploit fix */      
        return;

    // f*ck wpe
    if( !_player->GetInventory()->VerifyBagSlotsWithBank(SrcInvSlot, SrcSlot) )
        return;

    if( !_player->GetInventory()->VerifyBagSlotsWithBank(DstInvSlot, DstSlot) )
        return;

    int32 c = count;
    Item* i1 = _player->GetInventory()->GetInventoryItem(SrcInvSlot,SrcSlot);
    if(!i1)
        return;
    Item* i2=_player->GetInventory()->GetInventoryItem(DstInvSlot,DstSlot);

    if( (i1 && i1->IsWrapped()) || (i2 && i2->IsWrapped()) )
    {
        GetPlayer()->GetInventory()->BuildInventoryChangeError(i1, i2, INV_ERR_CANT_STACK);
        return;
    }

    if(i1 && (i1->GetProto()->MaxCount > 0 && i1->GetProto()->MaxCount < 2))
    {
        GetPlayer()->GetInventory()->BuildInventoryChangeError(i1, i2, INV_ERR_CANT_STACK);
        return;
    }

    if(i2 && (count < 1 || (i2->GetProto()->MaxCount > 0 && i2->GetProto()->MaxCount < 2)))
    {
        GetPlayer()->GetInventory()->BuildInventoryChangeError(i1, i2, INV_ERR_CANT_STACK);
        return;
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
                    i1->m_isDirty = true;
                    i2->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT, +c);
                    i2->m_isDirty = true;
                } else GetPlayer()->GetInventory()->BuildInventoryChangeError(i1, i2, INV_ERR_CANT_STACK);
            } else _player->GetInventory()->BuildInventoryChangeError(i1, i2, INV_ERR_SPLIT_FAILED); //error cant split item
        } else GetPlayer()->GetInventory()->BuildInventoryChangeError(i1, i2, INV_ERR_CANT_STACK);
    }
    else
    {
        if(c < (int32)i1->GetUInt32Value(ITEM_FIELD_STACK_COUNT))
        {
            i1->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT,-c);
            i1->m_isDirty = true;

            i2 = objmgr.CreateItem(i1->GetEntry(), _player, c);
            i2->m_isDirty = true;

            if(DstSlot == -1)
            {
                // Find a free slot
                SlotResult res = _player->GetInventory()->FindFreeInventorySlot(i2->GetProto());
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

            if(!(result = _player->GetInventory()->SafeAddItem(i2,DstInvSlot,DstSlot)))
            {
                printf("HandleBuyItemInSlot: Error while adding item to dstslot");
                //i2->DeleteFromDB();
                i2->Destruct();
                i2 = NULL;
            }
        } else _player->GetInventory()->BuildInventoryChangeError(i1, i2, INV_ERR_SPLIT_FAILED);
    }
}

void WorldSession::HandleSwapItemOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    WorldPacket data;
    WorldPacket packet;
    Item* SrcItem = NULL;
    Item* DstItem = NULL;

    int8 DstInvSlot=0, DstSlot=0, SrcInvSlot=0, SrcSlot=0, error=0;
    recv_data >> DstInvSlot >> DstSlot >> SrcInvSlot >> SrcSlot;

    sLog.outDebug("ITEM: swap, DstInvSlot %i DstSlot %i SrcInvSlot %i SrcSlot %i", DstInvSlot, DstSlot, SrcInvSlot, SrcSlot);

    if(DstInvSlot == SrcSlot && SrcInvSlot == -1) // player trying to add self container to self container slots
    {
        GetPlayer()->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_CANT_SWAP);
        return;
    }

    if( ( DstInvSlot <= 0 && DstSlot < 0 ) || DstInvSlot < -1 )
        return;

    if( ( SrcInvSlot <= 0 && SrcSlot < 0 ) || SrcInvSlot < -1 )
        return;

    SrcItem=_player->GetInventory()->GetInventoryItem(SrcInvSlot,SrcSlot);
    if(!SrcItem)
        return;

    DstItem=_player->GetInventory()->GetInventoryItem(DstInvSlot,DstSlot);

    if(DstItem)
    {   //check if it will go to equipment slot
        if(SrcInvSlot==INVENTORY_SLOT_NOT_SET)//not bag
        {
            if(DstItem->IsContainer())
            {
                if(castPtr<Container>(DstItem)->HasItems())
                {
                    if(SrcSlot < INVENTORY_SLOT_BAG_START || SrcSlot >= INVENTORY_SLOT_BAG_END || SrcSlot < BANK_SLOT_BAG_START || SrcSlot >= BANK_SLOT_BAG_END)
                    {
                        _player->GetInventory()->BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_BAG_IN_BAG);
                        return;
                    }
                }
            }

            if(SrcSlot < MAX_INVENTORY_SLOT)
            {
                if((error = GetPlayer()->GetInventory()->CanEquipItemInSlot(DstSlot, SrcInvSlot, SrcSlot, DstItem->GetProto())))
                {
                    _player->GetInventory()->BuildInventoryChangeError(SrcItem, DstItem, error);
                    return;
                }
            }
        }
        else
        {
            if(DstItem->IsContainer())
            {
                if(castPtr<Container>(DstItem)->HasItems())
                {
                    _player->GetInventory()->BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_BAG_IN_BAG);
                    return;
                }
            }

            if((error = GetPlayer()->GetInventory()->CanEquipItemInSlot(DstSlot, SrcInvSlot, SrcInvSlot, DstItem->GetProto())))
            {
                _player->GetInventory()->BuildInventoryChangeError(SrcItem, DstItem, error);
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
                if(castPtr<Container>(SrcItem)->HasItems())
                {
                    if(DstSlot < INVENTORY_SLOT_BAG_START || DstSlot >= INVENTORY_SLOT_BAG_END || DstSlot < BANK_SLOT_BAG_START || DstSlot >= BANK_SLOT_BAG_END)
                    {
                        _player->GetInventory()->BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_BAG_IN_BAG);
                        return;
                    }
                }
            }

            if(DstSlot < MAX_INVENTORY_SLOT)
            {
                if((error=GetPlayer()->GetInventory()->CanEquipItemInSlot(SrcSlot, DstInvSlot, DstSlot, SrcItem->GetProto())))
                {
                    _player->GetInventory()->BuildInventoryChangeError(SrcItem, DstItem, error);
                    return;
                }
            }
        }
        else
        {
            if(SrcItem->IsContainer())
            {
                if(castPtr<Container>(SrcItem)->HasItems())
                {
                    _player->GetInventory()->BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_BAG_IN_BAG);
                    return;
                }
            }

            if((error=GetPlayer()->GetInventory()->CanEquipItemInSlot(SrcSlot, DstInvSlot, DstInvSlot, SrcItem->GetProto())))
            {
                _player->GetInventory()->BuildInventoryChangeError(SrcItem, DstItem, error);
                return;
            }
        }
    }

    if( DstSlot < INVENTORY_SLOT_BAG_START && DstInvSlot == INVENTORY_SLOT_NOT_SET ) //equip
        SrcItem->Bind(ITEM_BIND_ON_EQUIP);

    _player->GetInventory()->SwapItems(SrcInvSlot, DstInvSlot, SrcSlot, DstSlot);
}

void WorldSession::HandleSwapInvItemOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    WorldPacket data;
    int8 srcslot=0, dstslot=0;
    int8 error=0;

    recv_data >> dstslot >> srcslot;

    if(!GetPlayer())
        return;

    sLog.outDebug("ITEM: swap, src slot: %u dst slot: %u", (uint32)srcslot, (uint32)dstslot);

    if(dstslot == srcslot) // player trying to add item to the same slot
    {
        GetPlayer()->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_CANT_SWAP);
        return;
    }

    Item* dstitem = _player->GetInventory()->GetInventoryItem(dstslot);
    Item* srcitem = _player->GetInventory()->GetInventoryItem(srcslot);

    // allow weapon switching in combat
    bool skip_combat = false;
    if( srcslot < EQUIPMENT_SLOT_END || dstslot < EQUIPMENT_SLOT_END )    // We're doing an equip swap.
    {
        if( _player->IsInCombat() )
        {
            if( srcslot < EQUIPMENT_SLOT_MAINHAND || dstslot < EQUIPMENT_SLOT_MAINHAND )    // These can't be swapped
            {
                _player->GetInventory()->BuildInventoryChangeError(srcitem, dstitem, INV_ERR_NOT_IN_COMBAT);
                return;
            }
            skip_combat= true;
        }
    }

    if( !srcitem )
    {
        _player->GetInventory()->BuildInventoryChangeError( srcitem, dstitem, INV_ERR_CANT_EQUIP_EVER );
        return;
    }

    if( srcslot == dstslot )
    {
        _player->GetInventory()->BuildInventoryChangeError( srcitem, dstitem, INV_ERR_WRONG_SLOT );
        return;
    }

    if( ( error = _player->GetInventory()->CanEquipItemInSlot( srcslot, INVENTORY_SLOT_NOT_SET, dstslot, srcitem->GetProto(), skip_combat ) ) )
    {
        if( dstslot < MAX_INVENTORY_SLOT )
        {
            _player->GetInventory()->BuildInventoryChangeError( srcitem, dstitem, error );
            return;
        }
    }

    if(dstitem)
    {
        if((error=_player->GetInventory()->CanEquipItemInSlot(srcslot, INVENTORY_SLOT_NOT_SET, srcslot, dstitem->GetProto(), skip_combat)))
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
        if(castPtr<Container>(srcitem)->HasItems())
        {
            if(!_player->GetInventory()->IsBagSlot(dstslot))
            {
                _player->GetInventory()->BuildInventoryChangeError(srcitem,dstitem, INV_ERR_BAG_IN_BAG);
                return;
            }
        }

        if(dstitem)
        {
            //source is a bag and dst slot is a bag inventory and has items
            if(dstitem->IsContainer())
            {
                if(castPtr<Container>(dstitem)->HasItems() && !_player->GetInventory()->IsBagSlot(srcslot))
                {
                    _player->GetInventory()->BuildInventoryChangeError(srcitem,dstitem, INV_ERR_BAG_IN_BAG);
                    return;
                }
            }
            else
            {
                //dst item is not a bag, swap impossible
                _player->GetInventory()->BuildInventoryChangeError(srcitem,dstitem,INV_ERR_BAG_IN_BAG);
                return;
            }
        }

        //dst is bag inventory
        if(dstslot < INVENTORY_SLOT_BAG_END)
            srcitem->Bind(ITEM_BIND_ON_EQUIP);
    }

    // swap items
    _player->GetInventory()->SwapItemSlots(srcslot, dstslot);
}

void WorldSession::HandleDestroyItemOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    int8 SrcInvSlot, SrcSlot;
    uint32 data;

    recv_data >> SrcInvSlot >> SrcSlot >> data;

    sLog.outDebug( "ITEM: destroy, SrcInv Slot: %i Src slot: %i", SrcInvSlot, SrcSlot );

    if(Item* it = _player->GetInventory()->GetInventoryItem(SrcInvSlot,SrcSlot))
    {
        if(it->IsContainer())
        {
            if(castPtr<Container>(it)->HasItems())
            {
                _player->GetInventory()->BuildInventoryChangeError( it, NULL, INV_ERR_DESTROY_NONEMPTY_BAG);
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

        Item* pItem = _player->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot,SrcSlot,false);
        if(!pItem)
            return;

        sQuestMgr.OnPlayerDropItem(_player, pItem->GetEntry());
        pItem->DeleteFromDB();
        pItem->Destruct();
    }
}

void WorldSession::HandleAutoEquipItemOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    AddItemResult result;
    int8 SrcInvSlot, SrcSlot, error = 0;
    recv_data >> SrcInvSlot >> SrcSlot;

    sLog.outDebug("ITEM: autoequip, Inventory slot: %i Source Slot: %i", SrcInvSlot, SrcSlot);

    Item *eitem=_player->GetInventory()->GetInventoryItem(SrcInvSlot,SrcSlot), *oitem = NULL;
    if(eitem == NULL)
    {
        _player->GetInventory()->BuildInventoryChangeError(eitem, NULL, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    int8 Slot = _player->GetInventory()->GetItemSlotByType(eitem->GetProto());
    if(Slot == ITEM_NO_SLOT_AVAILABLE)
    {
        _player->GetInventory()->BuildInventoryChangeError(eitem,NULL,INV_ERR_NOT_EQUIPPABLE);
        return;
    }

    if((Slot == EQUIPMENT_SLOT_MAINHAND || Slot == EQUIPMENT_SLOT_OFFHAND) && !_player->HasSpellWithEffect(SPELL_EFFECT_TITAN_GRIP))
    {
        Item* mainhandweapon = _player->GetInventory()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
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

        if((error = _player->GetInventory()->CanEquipItemInSlot(SrcSlot, INVENTORY_SLOT_NOT_SET, Slot, eitem->GetProto(), true, true)))
        {
            _player->GetInventory()->BuildInventoryChangeError(eitem,NULL, error);
            return;
        }

        if(eitem->GetProto()->InventoryType == INVTYPE_2HWEAPON)
        {
            // see if we have a weapon equipped in the offhand, if so we need to remove it
            Item* offhandweapon = _player->GetInventory()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND);
            if( offhandweapon != NULL )
            {
                // we need to de-equip this
                SlotResult result = _player->GetInventory()->FindFreeInventorySlot(offhandweapon->GetProto());
                if( !result.Result )
                {
                    // no free slots for this item
                    _player->GetInventory()->BuildInventoryChangeError(eitem,NULL, INV_ERR_BAG_FULL);
                    return;
                }

                offhandweapon = _player->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND, false);
                if( offhandweapon == NULL )
                    return;     // should never happen

                if(!_player->GetInventory()->SafeAddItem(offhandweapon, result.ContainerSlot, result.Slot) )
                {
                    if( !_player->GetInventory()->AddItemToFreeSlot(offhandweapon) )        // shouldn't happen either.
                    {
                        offhandweapon->Destruct();
                        offhandweapon = NULL;
                    }
                }
            }
        }
        else
        {
            // can't equip a non-two-handed weapon with a two-handed weapon
            mainhandweapon = _player->GetInventory()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
            if( mainhandweapon != NULL && mainhandweapon->GetProto()->InventoryType == INVTYPE_2HWEAPON )
            {
                // we need to de-equip this
                SlotResult result = _player->GetInventory()->FindFreeInventorySlot(mainhandweapon->GetProto());
                if( !result.Result )
                {
                    // no free slots for this item
                    _player->GetInventory()->BuildInventoryChangeError(eitem,NULL, INV_ERR_BAG_FULL);
                    return;
                }

                mainhandweapon = _player->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND, false);
                if( mainhandweapon == NULL )
                    return;     // should never happen

                if( !_player->GetInventory()->SafeAddItem(mainhandweapon, result.ContainerSlot, result.Slot) )
                {
                    if( !_player->GetInventory()->AddItemToFreeSlot(mainhandweapon) )       // shouldn't happen either.
                    {
                        mainhandweapon->Destruct();
                        mainhandweapon = NULL;
                    }
                }
            }
        }
    }
    else
    {
        if((error = _player->GetInventory()->CanEquipItemInSlot(SrcSlot, INVENTORY_SLOT_NOT_SET, Slot, eitem->GetProto())))
        {
            _player->GetInventory()->BuildInventoryChangeError(eitem,NULL, error);
            return;
        }
    }

    if( Slot <= INVENTORY_SLOT_BAG_END )
    {
        if((error = _player->GetInventory()->CanEquipItemInSlot(SrcSlot, INVENTORY_SLOT_NOT_SET, Slot, eitem->GetProto(), false, false)))
        {
            _player->GetInventory()->BuildInventoryChangeError(eitem,NULL, error);
            return;
        }
    }

    if( SrcInvSlot == INVENTORY_SLOT_NOT_SET )
    {
        _player->GetInventory()->SwapItemSlots( SrcSlot, Slot );
    }
    else if(eitem = _player->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot,SrcSlot, false))
    {
        if(oitem = _player->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, Slot, false))
        {
            result = _player->GetInventory()->SafeAddItem(oitem,SrcInvSlot,SrcSlot);
            if(!result)
            {
                printf("HandleAutoEquip: Error while adding item to SrcSlot");
                oitem->Destruct();
                oitem = NULL;
            }
        }

        result = _player->GetInventory()->SafeAddItem(eitem, INVENTORY_SLOT_NOT_SET, Slot);
        if(!result)
        {
            printf("HandleAutoEquip: Error while adding item to Slot");
            eitem->Destruct();
            eitem = NULL;
        }

    }

    if(eitem)
        eitem->Bind(ITEM_BIND_ON_EQUIP);
}

void WorldSession::HandleBuyBackOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
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
    Item* it = _player->GetInventory()->GetBuyBack(stuff);
    if (it)
    {
        // Find free slot and break if inv full
        uint32 amount = it->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
        uint32 itemid = it->GetUInt32Value(OBJECT_FIELD_ENTRY);

        add = _player->GetInventory()->FindItemLessMax(itemid,amount, false);

        uint32 FreeSlots = _player->GetInventory()->CalculateFreeSlots(it->GetProto());
        if ((FreeSlots == 0) && (!add))
        {
            _player->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INV_FULL);
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
        if ((error = _player->GetInventory()->CanReceiveItem(it->GetProto(), amount, NULL)))
        {
            _player->GetInventory()->BuildInventoryChangeError(NULL, NULL, error);
            return;
        }
        _player->ModUnsigned32Value( PLAYER_FIELD_COINAGE , -cost);
        _player->GetInventory()->RemoveBuyBackItem(stuff);

        if (!add)
        {
            it->m_isDirty = true;           // save the item again on logout
            result = _player->GetInventory()->AddItemToFreeSlot(it);
            if(!result)
            {
                printf("HandleBuyBack: Error while adding item to free slot");
                it->Destruct();
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
            it->Destruct();
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
    sLog.Debug( "WORLD"," Received CMSG_SELL_ITEM" );

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

    Creature* unit = _player->GetMapInstance()->GetCreature(vendorguid);
    // Check if Vendor exists
    if (unit == NULL)
    {
        SendSellItem(vendorguid, itemguid, 3);
        return;
    }

    Item* item = _player->GetInventory()->GetItemByGUID(itemguid);
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

    if(item->IsContainer() && castPtr<Container>(item)->HasItems())
    {
        SendSellItem(vendorguid, itemguid, 6);
        return;
    }

    // Check if item can be sold
    if( it->SellPrice == 0 )
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

    uint32 price = sItemMgr.CalculateSellPrice(it->ItemId, quantity);

    _player->ModUnsigned32Value(PLAYER_FIELD_COINAGE,price);

    if(quantity < stackcount)
    {
        item->SetCount(stackcount - quantity);
        item->m_isDirty = true;
    }
    else
    {
        //removing the item from the char's inventory
        item = _player->GetInventory()->SafeRemoveAndRetreiveItemByGuid(itemguid, false); //again to remove item from slot
        if(item)
        {
            sQuestMgr.OnPlayerDropItem(_player, item->GetEntry());
            _player->GetInventory()->AddBuyBackItem(item, (it->SellPrice) * quantity);
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
    sLog.Debug( "WORLD"," Received CMSG_BUY_ITEM" );

    WoWGuid vendorGuid, bagGuid;
    uint32 itemid, slot, count;
    uint8 type, batslot, error = 0;
    recv_data >> vendorGuid >> type >> itemid >> slot >> count >> bagGuid >> batslot;

    uint32 index = _player->GetVendorIndex(slot);
    if(index == 0xFFFFFFFF)
        return;
    Creature* unit = _player->GetInRangeObject<Creature>(vendorGuid);
    if (unit == NULL || !unit->HasItems())
        return;

    AvailableCreatureItem *availItem = unit->GetSellItemBySlot(index);
    if(availItem == NULL || availItem->proto->ItemId != itemid)
    {
        // vendor does not sell this item. bitch about cheaters?
        _player->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_NOT_OWNER);
        return;
    }

    int32 availableAmount = -1;
    if (availItem->max_amount > 0 && (availableAmount = unit->GetAvailableAmount(slot, availItem->max_amount)) < count)
    {
        _player->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_VENDOR_SOLD_OUT);
        return;
    }

    ItemPrototype *it = sItemMgr.LookupEntry(itemid);
    if(!it)
    {
        _player->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_NOT_OWNER);
        return;
    }

    if( it->MaxCount > 0 && count > it->MaxCount )
    {
        _player->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_CANT_STACK);
        return;
    }

    if((error = _player->GetInventory()->CanReceiveItem(it, count, availItem->extended_cost)))
    {
        _player->GetInventory()->BuildInventoryChangeError(NULL, NULL, error);
        return;
    }

    if((error = _player->GetInventory()->CanAffordItem(it, count, unit, availItem->extended_cost)))
    {
        SendBuyFailed(vendorGuid, itemid, error);
        return;
    }

    SlotResult slotresult;
    // Find free slot and break if inv full
    Item *add = NULL;
    if ((add = _player->GetInventory()->FindItemLessMax(itemid, count, false)) == NULL)
        slotresult = _player->GetInventory()->FindFreeInventorySlot(it);
    if (!slotresult.Result && add == NULL)
    {
        //Our User doesn't have a free Slot in there bag
        _player->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INV_FULL);
        return;
    }

    if(add == NULL)
    {
        Item* itm = objmgr.CreateItem(availItem->proto->ItemId, _player, count);
        if(itm == NULL)
        {
            _player->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_NOT_OWNER);
            return;
        }

        itm->m_isDirty = true;
        if(slotresult.ContainerSlot == ITEM_NO_SLOT_AVAILABLE)
        {
            AddItemResult result = _player->GetInventory()->SafeAddItem(itm, INVENTORY_SLOT_NOT_SET, slotresult.Slot);
            if(result == ADD_ITEM_RESULT_ERROR)
                itm->Destruct();
        } else if( Item* bag = _player->GetInventory()->GetInventoryItem(slotresult.ContainerSlot))
            if( !castPtr<Container>(bag)->AddItem(slotresult.Slot, itm) )
                itm->Destruct();
    }
    else
    {
        add->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT, count);
        add->m_isDirty = true;
    }

    if(int32(availItem->max_amount) > 0)
        availableAmount = unit->ModAvItemAmount(slot, count);

    WorldPacket data(SMSG_BUY_ITEM, 20);
    data << vendorGuid << uint32(slot) << availableAmount << uint32(count);
    SendPacket( &data );

    _player->GetInventory()->BuyItem(it,count,unit, availItem->extended_cost);

    _player->SaveToDB(false);
}

void WorldSession::HandleAutoStoreBagItemOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Recvd CMSG_AUTO_STORE_BAG_ITEM" );

    if(!GetPlayer())
        return;

    //WorldPacket data;
    WorldPacket packet;
    int8 SrcInv=0, Slot=0, DstInv=0;
    Item* srcitem = NULL, *dstitem= NULL;
    int8 NewSlot = 0;
    int8 error;
    AddItemResult result;

    recv_data >> SrcInv >> Slot >> DstInv;

    srcitem = _player->GetInventory()->GetInventoryItem(SrcInv, Slot);

    //source item exists
    if(srcitem)
    {
        //src containers cant be moved if they have items inside
        if(srcitem->IsContainer() && castPtr<Container>(srcitem)->HasItems())
        {
            _player->GetInventory()->BuildInventoryChangeError(srcitem, NULL, INV_ERR_DESTROY_NONEMPTY_BAG);
            return;
        }
        //check for destination now before swaping.
        //destination is backpack
        if(DstInv == INVENTORY_SLOT_NOT_SET)
        {
            //check for space
            NewSlot = _player->GetInventory()->FindFreeBackPackSlot();
            if(NewSlot == ITEM_NO_SLOT_AVAILABLE)
            {
                _player->GetInventory()->BuildInventoryChangeError(srcitem, NULL, INV_ERR_BAG_FULL);
                return;
            }
            else
            {
                //free space found, remove item and add it to the destination
                srcitem = _player->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(SrcInv, Slot, false);
                if( srcitem )
                {
                    result = _player->GetInventory()->SafeAddItem(srcitem, INVENTORY_SLOT_NOT_SET, NewSlot);
                    if(!result)
                    {
                        printf("HandleAutoStoreBagItem: Error while adding item to newslot");
                        srcitem->Destruct();
                        srcitem = NULL;
                        return;
                    }
                }
            }
        }
        else
        {
            if((error=_player->GetInventory()->CanEquipItemInSlot(Slot, DstInv,  DstInv, srcitem->GetProto())))
            {
                if(DstInv < MAX_INVENTORY_SLOT)
                {
                    _player->GetInventory()->BuildInventoryChangeError(srcitem,dstitem, error);
                    return;
                }
            }

            //destination is a bag
            dstitem = _player->GetInventory()->GetInventoryItem(DstInv);
            if(dstitem)
            {
                //dstitem exists, detect if its a container
                if(dstitem->IsContainer())
                {
                    NewSlot = castPtr<Container>(dstitem)->FindFreeSlot();
                    if(NewSlot == ITEM_NO_SLOT_AVAILABLE)
                    {
                        _player->GetInventory()->BuildInventoryChangeError(srcitem, NULL, INV_ERR_BAG_FULL);
                        return;
                    }
                    else
                    {
                        srcitem = _player->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(SrcInv, Slot, false);
                        if( srcitem != NULL )
                        {
                            result = _player->GetInventory()->SafeAddItem(srcitem, DstInv, NewSlot);
                            if(!result)
                            {
                                printf("HandleBuyItemInSlot: Error while adding item to newslot");
                                srcitem->Destruct();
                                srcitem = NULL;
                                return;
                            }
                        }
                    }
                }
                else
                {
                    _player->GetInventory()->BuildInventoryChangeError(srcitem, NULL,  INV_ERR_WRONG_SLOT);
                    return;
                }
            }
            else
            {
                _player->GetInventory()->BuildInventoryChangeError(srcitem, NULL, INV_ERR_WRONG_SLOT);
                return;
            }
        }
    }
    else
    {
        _player->GetInventory()->BuildInventoryChangeError(srcitem, NULL, INV_ERR_ITEM_NOT_FOUND);
        return;
    }
}

void WorldSession::HandleReadItemOpcode(WorldPacket &recvPacket)
{
    CHECK_INWORLD_RETURN();
    int8 uslot=0, slot=0;
    recvPacket >> uslot >> slot;

    if(!GetPlayer())
        return;

    Item* item = _player->GetInventory()->GetInventoryItem(uslot, slot);
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

uint32 RepairItemCost(Player* pPlayer, Item* pItem)
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

    uint32 dmodifier = dcosts->modifier[pItem->GetProto()->Class == ITEM_CLASS_WEAPON ? pItem->GetProto()->SubClass : (pItem->GetProto()->SubClass + 21)];
    uint32 cost = double2int32((pItem->GetDurabilityMax() - pItem->GetDurability()) * dmodifier * double(dquality->quality_modifier));
    return cost;
}

void RepairItem(Player* pPlayer, Item* pItem, bool guild = false)
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

    uint64 npcguid, itemguid;
    bool guildmoney;
    Item* pItem;
    Container* pContainer;
    int32 j, i;

    recvPacket >> npcguid >> itemguid >> guildmoney;
    Creature* pCreature = _player->GetMapInstance()->GetCreature(npcguid);
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
            pItem = _player->GetInventory()->GetInventoryItem( i );
            if( pItem != NULL )
            {
                if( pItem->IsContainer() )
                {
                    pContainer = castPtr<Container>( pItem );
                    for( j = 0; j < pContainer->GetProto()->ContainerSlots; ++j )
                    {
                        pItem = pContainer->GetItem( j );
                        if( pItem != NULL )
                            RepairItem( _player, pItem, guildmoney );
                    }
                }
                else
                {
                    if( pItem->GetProto()->Durability > 0 && i < INVENTORY_SLOT_BAG_END && pItem->GetDurability() <= 0 )
                    {
                        RepairItem( _player, pItem, guildmoney );
                        _player->ApplyItemMods( pItem, i, true );
                    } else RepairItem( _player, pItem, guildmoney );
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
    else if(Item* item = _player->GetInventory()->GetItemByGUID(itemguid))
    {
        SlotResult *searchres=_player->GetInventory()->LastSearchResult();//this never gets null since we get a pointer to the inteface internal var
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
    }
}

void WorldSession::HandleAutoBankItemOpcode(WorldPacket &recvPacket)
{
    CHECK_INWORLD_RETURN();
    sLog.Debug("WorldSession","Received CMSG_AUTO_BANK_ITEM");

    //WorldPacket data;

    SlotResult slotresult;
    int8 SrcInvSlot, SrcSlot;//, error=0;

    if(!GetPlayer())
        return;

    recvPacket >> SrcInvSlot >> SrcSlot;

    sLog.Debug("WorldSession","HandleAutoBankItemOpcode: Inventory slot: %u Source Slot: %u", (uint32)SrcInvSlot, (uint32)SrcSlot);

    Item* eitem=_player->GetInventory()->GetInventoryItem(SrcInvSlot,SrcSlot);

    if(!eitem)
    {
        _player->GetInventory()->BuildInventoryChangeError(eitem, NULL, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    slotresult =  _player->GetInventory()->FindFreeBankSlot(eitem->GetProto());

    if(!slotresult.Result)
    {
        _player->GetInventory()->BuildInventoryChangeError(eitem, NULL, INV_ERR_BANK_FULL);
        return;
    }
    else
    {
        eitem = _player->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot,SrcSlot, false);
        if(!_player->GetInventory()->SafeAddItem(eitem, slotresult.ContainerSlot, slotresult.Slot))
        {
            sLog.outDebug("[ERROR]AutoBankItem: Error while adding item to bank bag!\n");
            if( !_player->GetInventory()->SafeAddItem(eitem, SrcInvSlot, SrcSlot) )
            {
                eitem->Destruct();
                eitem = NULL;
            }
        }
    }
}

void WorldSession::HandleAutoStoreBankItemOpcode(WorldPacket &recvPacket)
{
    CHECK_INWORLD_RETURN();
    sLog.outDebug("WORLD: CMSG_AUTOSTORE_BANK_ITEM");

    //WorldPacket data;

    int8 SrcInvSlot, SrcSlot;//, error=0, slot=-1, specialbagslot=-1;

    if(!GetPlayer())
        return;

    recvPacket >> SrcInvSlot >> SrcSlot;

    sLog.outDebug("ITEM: AutoStore Bank Item, Inventory slot: %i Source Slot: %i", SrcInvSlot, SrcSlot);

    Item* eitem = _player->GetInventory()->GetInventoryItem(SrcInvSlot,SrcSlot);

    if(!eitem)
    {
        _player->GetInventory()->BuildInventoryChangeError(eitem, NULL, INV_ERR_ITEM_NOT_FOUND);
        return;
    }

    SlotResult slotresult = _player->GetInventory()->FindFreeInventorySlot(eitem->GetProto());

    if(!slotresult.Result)
    {
        _player->GetInventory()->BuildInventoryChangeError(eitem, NULL, INV_ERR_INV_FULL);
        return;
    }
    else
    {
        eitem = _player->GetInventory()->SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot, SrcSlot, false);
        if (!_player->GetInventory()->AddItemToFreeSlot(eitem))
        {
            sLog.outDebug("[ERROR]AutoStoreBankItem: Error while adding item from one of the bank bags to the player bag!\n");
            if( !_player->GetInventory()->SafeAddItem(eitem, SrcInvSlot, SrcSlot) )
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

    Item* item = _player->GetInventory()->GetInventoryItem(inventory_slot);
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
    CHECK_INWORLD_RETURN();

    WoWGuid itemGuid, gemGuid[3];
    SpellItemEnchantEntry * Enchantment;
    recvPacket >> itemGuid;
    for(uint32 i = 0; i < 3; i++)
        recvPacket >> gemGuid[i];

    PlayerInventory *itemi = _player->GetInventory();
    Item* TargetItem = itemi->GetItemByGUID(itemGuid);
    if(TargetItem == NULL)
        return;

    WorldPacket data(SMSG_SOCKET_GEMS_RESULT, 16);
    data << itemGuid;

    int slot = itemi->GetInventorySlotByGuid(itemGuid);
    bool apply = (slot >= 0 && slot < 19);
    uint32 filledSlots = 0;

    /* The following is a hack check to make sure player's aren't socketing more than they have,
        while still allowing socketing of items with prismatic sockets. */
    bool extraSocket = (TargetItem->HasEnchantment(3723) && TargetItem->GetProto()->InventoryType == 10)
        || (TargetItem->HasEnchantment(3717) && TargetItem->GetProto()->InventoryType == 9)
        || (TargetItem->HasEnchantment(3729) && TargetItem->GetProto()->InventoryType == 6);

    bool colorMatch[3] = { true, true, true };
    for(uint32 i = 0; i < 3; i++)
    {
        uint32 socketType = TargetItem->GetProto()->ItemSocket[i];
        if(socketType == 0 && !(extraSocket && (i >= 1) && TargetItem->GetProto()->ItemSocket[i-1] != 0))
            continue;

        GemPropertyEntry *gp = NULL;
        EnchantmentInstance *EI = NULL;
        if(EI = TargetItem->GetEnchantment(2+i))
        {
            filledSlots |= (1<<i);
            if(ItemPrototype *ip = sItemMgr.LookupEntry(EI->Enchantment->GemEntry))
                gp = dbcGemProperty.LookupEntry(ip->GemProperties);

            if(gp && socketType && !(gp->SocketMask & socketType))
                colorMatch[i] = false;
        } else if(gemGuid[i].empty())
            colorMatch[i] = false;

        if(gemGuid[i])//add or replace gem
        {
            Item *it = itemi ? itemi->GetItemByGUID(gemGuid[i]) : NULL;
            ItemPrototype * ip = it ? it->GetProto() : NULL;
            if( it == NULL || ip == NULL)
            {
                itemi->BuildInventoryChangeError( NULL, TargetItem, INV_ERR_OBJECT_IS_BUSY, gemGuid[i] );
                continue;
            }

            if (apply)
            {
                if(ip->GemProperties == 0) // Incomplete DB, but we have cached.
                    continue;

                if( ip->Flags & DBC_ITEMFLAG_UNIQUE_EQUIPPED && itemi->IsEquipped( ip->ItemId ) )
                {
                    itemi->BuildInventoryChangeError( it, TargetItem, INV_ERR_ITEM_MAX_COUNT );
                    continue;
                }

                // Skill requirement
                if( ip->RequiredSkill > 0 && ((uint32)ip->RequiredSkillRank > _player->_GetSkillLineCurrent( ip->RequiredSkill, true )) )
                {
                    itemi->BuildInventoryChangeError( it, TargetItem, INV_ERR_CANT_EQUIP_SKILL );
                    continue;
                }

                ItemLimitCategoryEntry *il = NULL;
                if( ip->ItemLimitCategory && (il = dbcItemLimitCategory.LookupEntry( ip->ItemLimitCategory )) && itemi->GetSocketedGemCountWithLimitId( ip->ItemLimitCategory ) >= il->MaxAmount )
                {
                    itemi->BuildInventoryChangeError(it, TargetItem, INV_ERR_ITEM_MAX_COUNT_EQUIPPED_SOCKETED);
                    continue;
                }
            }

            uint16 bagSlot = itemi->GetBagSlotByGuid(gemGuid[i]);
            if(bagSlot == ITEM_NO_SLOT_AVAILABLE)
            {
                itemi->BuildInventoryChangeError( it, TargetItem, INV_ERR_OBJECT_IS_BUSY );
                continue;
            }

            bool applied = false;
            if(EI)//replace gem
            {
                TargetItem->RemoveEnchantment(2+i);//remove previous
                filledSlots &= ~(1<<i);
            }

            if((gp = dbcGemProperty.LookupEntry(ip->GemProperties)) && (Enchantment = dbcSpellItemEnchant.LookupEntry(gp->EnchantmentID)))
            {
                if(!(gp->SocketMask & TargetItem->GetProto()->ItemSocket[i]))
                    colorMatch[i] = false;
                TargetItem->AddEnchantment(Enchantment, 0, true, apply, false, SOCK_ENCHANTMENT_SLOT1+i);
                applied = true;
            }
            else
            {   // Lacking DBC data, pull from proto.
                uint32 gemmask = ConvertDB2DBCGemType(ip->SubClass);
                if(gemmask == -1 || !(gemmask & TargetItem->GetProto()->ItemSocket[i]))
                    colorMatch[i] = false;

                // If we're negative, its a dummy gem.
                if(ip->GemProperties < 0 && (Enchantment = dbcSpellItemEnchant.LookupEntry(-ip->GemProperties)))
                {
                    TargetItem->AddEnchantment(Enchantment, 0, true, apply, false, SOCK_ENCHANTMENT_SLOT1+i, 0, true);
                    applied = true;
                }
            }

            if(applied)
            {
                filledSlots |= (1<<i);
                uint32 stackCount = 0;
                if((stackCount = it->GetStackCount()) > 1)
                {
                    it->SetStackCount(stackCount-1);
                    it->m_isDirty = true;
                }
                else if(Item *cleanup = itemi->SafeRemoveAndRetreiveItemByGuid(gemGuid[i], true))
                {
                    sQuestMgr.OnPlayerDropItem(_player, ip->ItemId);
                    cleanup->Destruct();
                }
            }
        }
    }

    bool truecolormatch = true;
    for(uint32 i = 0; i < 3; i++)
        if(TargetItem->GetProto()->ItemSocket[i])
            if(colorMatch[i] == false)
                truecolormatch = false;

    //Add color match bonus
    if(TargetItem->GetProto()->SocketBonus)
    {
        if(truecolormatch)
        {
            if(TargetItem->HasEnchantment(TargetItem->GetProto()->SocketBonus) == 0 && (Enchantment = dbcSpellItemEnchant.LookupEntry(TargetItem->GetProto()->SocketBonus)))
            {
                filledSlots |= (1<<3);
                TargetItem->AddEnchantment(Enchantment, 0, true, apply, false, BONUS_ENCHANTMENT_SLOT);
            }
        } else TargetItem->RemoveSocketBonusEnchant();
    }

    for(uint32 i = 0; i < 4; i++)
    {
        EnchantmentInstance *enchant = NULL;
        if(filledSlots & (1<<i))
            enchant = TargetItem->GetEnchantment(SOCK_ENCHANTMENT_SLOT1+i);
        data << uint32(enchant ? enchant->Enchantment->Id : NULL);
    }

    SendPacket(&data);
    TargetItem->m_isDirty = true;
}

void WorldSession::HandleWrapItemOpcode( WorldPacket& recv_data )
{

}

void WorldSession::HandleItemRefundInfoOpcode( WorldPacket& recv_data )
{

}

void WorldSession::HandleItemRefundRequestOpcode( WorldPacket& recv_data )
{

}
