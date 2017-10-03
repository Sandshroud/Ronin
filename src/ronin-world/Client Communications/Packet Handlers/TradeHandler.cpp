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

void WorldSession::HandleInitiateTrade(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    Player *targetPlr = NULL;
    recv_data.ReadGuidBitString(8, guid, 0, 3, 5, 1, 4, 6, 7, 2);
    recv_data.ReadGuidByteString(8, guid, 7, 4, 3, 5, 1, 2, 6, 0);
    if(uint8 result = _player->GetMapInstance()->StartTrade(_player->GetGUID(), guid, &targetPlr))
    {
        switch(result)
        {
        case MapInstance::TRADE_STATUS_BEGIN_TRADE:
            MapInstance::SendTradeStatus(targetPlr, result, WoWGuid(0), _player->GetGUID(), false);
            break;
        default:
            MapInstance::SendTradeStatus(_player, result, WoWGuid(0), WoWGuid(0), false);
            break;
        }
    }
}

void WorldSession::HandleBeginTrade(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    if(Player *targetPlr = _player->GetMapInstance()->BeginTrade(_player->GetGUID()))
    {
        MapInstance::SendTradeStatus(_player, MapInstance::TRADE_STATUS_OPEN_WINDOW, WoWGuid(0), targetPlr->GetGUID(), false);
        MapInstance::SendTradeStatus(targetPlr, MapInstance::TRADE_STATUS_OPEN_WINDOW, WoWGuid(0), _player->GetGUID(), false);
    }
}

void WorldSession::HandleBusyTrade(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    //_player->GetMapInstance()->SetTradeStatus(_player->GetGUID(), MapInstance::TRADE_STATUS_BUSY_2);
}

void WorldSession::HandleIgnoreTrade(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    //_player->GetMapInstance()->SetTradeStatus(_player->GetGUID(), MapInstance::TRADE_STATUS_IGNORE_YOU);
}

void WorldSession::HandleCancelTrade(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    _player->GetMapInstance()->SetTradeStatus(_player->GetGUID(), MapInstance::TRADE_STATUS_TRADE_CANCELED);
}

void WorldSession::HandleTradeAcceptToggle(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    // We kind of just toggle
    _player->GetMapInstance()->SetTradeStatus(_player->GetGUID(), MapInstance::TRADE_STATUS_TRADE_ACCEPT);
}

void WorldSession::HandleSetTradeItem(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint8 tradeSlot, containerSlot, itemSlot;
    recv_data >> itemSlot >> tradeSlot >> containerSlot;
    if(tradeSlot >= 7)
    {
        _player->GetMapInstance()->CleanupTrade(_player->GetGUID());
        return;
    }

    Player *target = _player->GetMapInstance()->GetTradeTarget(_player->GetGUID());
    if(target == NULL)
    {
        _player->GetMapInstance()->CleanupTrade(_player->GetGUID());
        return;
    }

    Item *tradeItem = _player->GetInventory()->GetInventoryItem((int8)containerSlot, (int8)itemSlot);
    if(tradeItem == NULL)// || (tradeSlot != 7 && !_player->GetInventory()->CanTradeItem(_player, tradeItem)))
    {
        _player->GetMapInstance()->CleanupTrade(_player->GetGUID());
        return;
    }

    _player->GetMapInstance()->SetTradeValue(_player, 1, tradeSlot, tradeItem->GetGUID());
}

void WorldSession::HandleSetTradeGold(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint64 value = recv_data.read<uint64>();
    _player->GetMapInstance()->SetTradeValue(_player, 0, value, WoWGuid(0));
}

void WorldSession::HandleClearTradeItem(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint8 tradeSlot = recv_data.read<uint8>();
    if(tradeSlot >= 7)
    {
        _player->GetMapInstance()->CleanupTrade(_player->GetGUID());
        return;
    }

    _player->GetMapInstance()->SetTradeValue(_player, 1, tradeSlot, WoWGuid(0));
}
