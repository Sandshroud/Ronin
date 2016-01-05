/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::SendTradeStatus(uint32 TradeStatus)
{
    OutPacket(SMSG_TRADE_STATUS, 4, &TradeStatus);
};

void WorldSession::HandleInitiateTrade(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint64 guid;
    recv_data >> guid;

    PlayerTradeStatus tradeStatus = TRADE_STATUS_BEGIN_TRADE;
    Player* pTarget = _player->GetMapInstance()->GetPlayer((uint32)guid);
    if(pTarget == NULL || !pTarget->IsInWorld())
        tradeStatus = TRADE_STATUS_NO_TARGET;
    else if(_player->isDead())
        tradeStatus = TRADE_STATUS_YOU_ARE_DEAD;
    else if(pTarget->isDead())
        tradeStatus = TRADE_STATUS_TARGET_IS_DEAD;
    else if(_player->IsStunned())
        tradeStatus = TRADE_STATUS_YOU_ARE_STUNNED;
    else if(pTarget->IsStunned())
        tradeStatus = TRADE_STATUS_TARGET_IS_STUNNED;
    else if(pTarget->m_tradeData)
        tradeStatus = TRADE_STATUS_TARGET_IS_BUSY;
    else if(pTarget->GetTeam() != _player->GetTeam() && GetPermissionCount() == 0)
        tradeStatus = TRADE_STATUS_TARGET_WRONG_FACTION;
    else if(pTarget->m_ignores.find(_player->GetGUID()) != pTarget->m_ignores.end())
        tradeStatus = TRADE_STATUS_TARGET_IGNORING_YOU;
    else if(_player->CalcDistance(pTarget) > 10.0f)     // This needs to be checked
        tradeStatus = TRADE_STATUS_TARGET_TOO_FAR;
    else if(pTarget->m_session == NULL || pTarget->m_session->GetSocket() == NULL)
        tradeStatus = TRADE_STATUS_TARGET_LOGGING_OUT;
    else
    {
        pTarget->CreateNewTrade(_player->GetGUID());
        _player->CreateNewTrade(pTarget->GetGUID());
        pTarget->SendTradeUpdate(false, tradeStatus);
        return;
    }

    _player->SendTradeUpdate(false, tradeStatus);
    _player->ResetTradeVariables(); // Clear our trade data
}

void WorldSession::HandleBeginTrade(WorldPacket & recv_data)
{

}

void WorldSession::HandleBusyTrade(WorldPacket & recv_data)
{

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
