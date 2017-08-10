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

void SendTradeStatus(Player *target, uint32 TradeStatus, WoWGuid owner, WoWGuid trader, bool linkedbNetAccounts, uint8 slotError = -1, bool hasInvError = false, uint32 invError = 0, uint32 limitCategoryId = 0)
{
    WorldPacket data(SMSG_TRADE_STATUS, 13);
    data.WriteBit(linkedbNetAccounts ? 1 : 0);
    data.WriteBits(TradeStatus, 5);
    switch (TradeStatus)
    {
    case MapInstance::TRADE_STATUS_BEGIN_TRADE:
        data.WriteGuidBitString(8, trader, 2, 4, 6, 0, 1, 3, 7, 5);
        data.WriteSeqByteString(8, trader, 4, 1, 2, 3, 0, 7, 6, 5);
        break;
    case MapInstance::TRADE_STATUS_OPEN_WINDOW: data << uint32(0); break;
    case MapInstance::TRADE_STATUS_CLOSE_WINDOW:
        data.WriteBit(hasInvError);
        data << uint32(invError) << uint32(limitCategoryId);
        break;
    case MapInstance::TRADE_STATUS_WRONG_REALM:
    case MapInstance::TRADE_STATUS_NOT_ON_TAPLIST:
        data << uint8(slotError);
        break;
    case MapInstance::TRADE_STATUS_CURRENCY:
    case MapInstance::TRADE_STATUS_CURRENCY_NOT_TRADABLE:
        data << uint32(0); // Trading Currency Id
        data << uint32(0); // Trading Currency Amount
    default:
        data.FlushBits();
        break;
    }

    target->PushPacket(&data);
};

void WorldSession::HandleInitiateTrade(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    recv_data.ReadGuidBitString(8, guid, 0, 3, 5, 1, 4, 6, 7, 2);
    recv_data.ReadGuidByteString(8, guid, 7, 4, 3, 5, 1, 2, 6, 0);
    if(uint8 result = _player->GetMapInstance()->StartTrade(_player->GetGUID(), guid))
        SendTradeStatus(_player, result, WoWGuid(0), WoWGuid(0), false);
}

void WorldSession::HandleBeginTrade(WorldPacket & recv_data)
{

}

void WorldSession::HandleBusyTrade(WorldPacket & recv_data)
{
    _player->GetMapInstance()->SetTradeStatus(_player->GetGUID(), MapInstance::TRADE_STATUS_BUSY_2);
}

void WorldSession::HandleIgnoreTrade(WorldPacket & recv_data)
{

}

void WorldSession::HandleCancelTrade(WorldPacket & recv_data)
{

}

void WorldSession::HandleUnacceptTrade(WorldPacket & recv_data)
{

}

void WorldSession::HandleSetTradeItem(WorldPacket & recv_data)
{

}

void WorldSession::HandleSetTradeGold(WorldPacket & recv_data)
{

}

void WorldSession::HandleClearTradeItem(WorldPacket & recv_data)
{

}

void WorldSession::HandleAcceptTrade(WorldPacket & recv_data)
{

}
