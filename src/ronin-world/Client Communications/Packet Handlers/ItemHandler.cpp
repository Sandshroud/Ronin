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
    if( !_player->GetItemInterface()->VerifyBagSlotsWithBank(SrcInvSlot, SrcSlot) )
        return;

    if( !_player->GetItemInterface()->VerifyBagSlotsWithBank(DstInvSlot, DstSlot) )
        return;

    int32 c = count;
    Item* i1 = _player->GetItemInterface()->GetInventoryItem(SrcInvSlot,SrcSlot);
    if(!i1)
        return;
    Item* i2=_player->GetItemInterface()->GetInventoryItem(DstInvSlot,DstSlot);

    if( (i1 && i1->isWrapped()) || (i2 && i2->isWrapped()) )
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
                } else GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
            } else _player->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_COULDNT_SPLIT_ITEMS); //error cant split item
        } else GetPlayer()->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_ITEM_CANT_STACK);
    }
    else
    {
        if(c < (int32)i1->GetUInt32Value(ITEM_FIELD_STACK_COUNT))
        {
            i1->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT,-c);

            i2=objmgr.CreateItem(i1->GetEntry(),_player);
            i2->SetUInt32Value(ITEM_FIELD_STACK_COUNT,c);

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
                i2->Destruct();
                i2 = NULL;
            }
        }
        else
            _player->GetItemInterface()->BuildInventoryChangeError(i1, i2, INV_ERR_COULDNT_SPLIT_ITEMS);
    }
}

void WorldSession::HandleSwapItemOpcode(WorldPacket& recv_data)
{

}

void WorldSession::HandleSwapInvItemOpcode( WorldPacket & recv_data )
{

}

void WorldSession::HandleDestroyItemOpcode( WorldPacket & recv_data )
{

}

void WorldSession::HandleAutoEquipItemOpcode( WorldPacket & recv_data )
{

}

void WorldSession::HandleBuyBackOpcode( WorldPacket & recv_data )
{

}

void WorldSession::HandleSellItemOpcode( WorldPacket & recv_data )
{

}

void WorldSession::HandleBuyItemOpcode( WorldPacket & recv_data ) // right-click on item
{

}

void WorldSession::HandleAutoStoreBagItemOpcode( WorldPacket & recv_data )
{

}

void WorldSession::HandleReadItemOpcode(WorldPacket &recvPacket)
{
    CHECK_INWORLD_RETURN();
    int8 uslot=0, slot=0;
    recvPacket >> uslot >> slot;

    if(Item* item = _player->GetItemInterface()->GetInventoryItem(uslot, slot))
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

void WorldSession::HandleRepairItemOpcode(WorldPacket &recvPacket)
{

}

void WorldSession::HandleAutoBankItemOpcode(WorldPacket &recvPacket)
{

}

void WorldSession::HandleAutoStoreBankItemOpcode(WorldPacket &recvPacket)
{

}

void WorldSession::HandleCancelTemporaryEnchantmentOpcode(WorldPacket &recvPacket)
{

}

void WorldSession::HandleInsertGemOpcode(WorldPacket &recvPacket)
{

}

void WorldSession::HandleWrapItemOpcode( WorldPacket& recv_data )
{

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
