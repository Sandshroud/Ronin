/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleSplitOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();

    uint16 srcSlot, dstSlot;
    recv_data >> srcSlot >> dstSlot;
    uint32 count = std::min(recv_data.read<uint8>(), uint8(100));

    // Packet editting
    if(!_player->GetItemInterface()->IsValidSrcSlot(srcSlot, false))
        return;
    Item *src = _player->GetItemInterface()->GetInventoryItem(srcSlot);
    if(src == NULL)
        return;
    if(!_player->GetItemInterface()->IsValidDstSlot(src, dstSlot, false))
        return;
    if(count == 0)
    {
        _player->GetItemInterface()->BuildInvError(INV_ERR_CANT_STACK, src, NULL);
        return;
    }
    if(count >= src->GetUInt32Value(ITEM_FIELD_STACK_COUNT))
    {
        _player->GetItemInterface()->BuildInvError(INV_ERR_TOO_FEW_TO_SPLIT, src, NULL);
        return;
    }

    Item *dst = _player->GetItemInterface()->GetInventoryItem(dstSlot);
    if(dst && dst->GetEntry() != src->GetEntry())
        return;

    if(src->isWrapped() || (dst && dst->isWrapped()) )
    {
        _player->GetItemInterface()->BuildInvError(INV_ERR_CANT_STACK, src, dst);
        return;
    }

    if((src->GetProto()->MaxCount == 1) || (dst && dst->GetProto()->MaxCount == 1))
    {
        _player->GetItemInterface()->BuildInvError(INV_ERR_CANT_STACK, src, dst);
        return;
    }

    if(dst)
    {
        uint32 newCount = count + dst->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
        if(newCount > dst->GetProto()->MaxCount)
        {
            _player->GetItemInterface()->BuildInvError(INV_ERR_CANT_STACK, src, dst);
            return;
        }

        src->SetStackSize(src->GetStackSize()-count);
        dst->SetStackSize(newCount);
    }
    else if(ItemData *newData = sItemMgr.CreateItemData(src->GetEntry()))
    {
        dst = new Item(newData);
        dst->Initialize(_player);
        dst->SetStackSize(count);

        if(INVSLOT_ITEM(dstSlot) == INVENTORY_SLOT_NONE)
        {
            if(!_player->GetItemInterface()->FindFreeSlot(dst, dstSlot))
            {
                _player->GetItemInterface()->BuildInvError(INV_ERR_SPLIT_FAILED, src, NULL);
                return;
            }
        }

        if(!_player->GetItemInterface()->AddInventoryItemToSlot(dst, dstSlot))
        {
            sItemMgr.DeleteItemData(dst->GetGUID());
            _player->GetItemInterface()->BuildInvError(INV_ERR_SPLIT_FAILED, src, NULL);
            delete dst;
            return;
        }
        src->SetStackSize(src->GetStackSize()-count);
    } else _player->GetItemInterface()->BuildInvError(INV_ERR_SPLIT_FAILED, src, NULL);
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
    uint16 itemSlot;
    recvPacket >> itemSlot;

    if(Item* item = _player->GetItemInterface()->GetInventoryItem(itemSlot))
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
