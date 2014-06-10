/*
 * Sandshroud Hearthstone
 * Copyright (C) 2010 - 2011 Sandshroud <http://www.sandshroud.org/>
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
 * Crow: Credits to Egari from Feathermoon, you kick ass dude. Seriously.
 *
 */

#include "StdAfx.h"

void WarnSystem::Punishment_Kick(Player * Warner, Player * Warned, uint32 NewWarnCnt, std::string reason)
{
    BroadcastToWorld("User will now be kicked from the server.");
    sEventMgr.AddEvent(Warned, &Player::SoftDisconnect, EVENT_PLAYER_SOFT_DISCONNECT, 5000, 1, 0);
}

void WarnSystem::Punishment_PermBanAccount(Player * Warner, Player * Warned, uint32 NewWarnCnt, std::string reason)
{
    if( Warner && Warner->GetSession() )
    {
        sChatHandler.SystemMessage(Warner->GetSession(), "[WarnSystem] Banning ingame account of '%s' for '%s'.", Warned->GetName(), reason.c_str());
        sChatHandler.SystemMessage(Warner->GetSession(), "[WarnSystem] This ban is due to expire when hell freezes over.");
    }

    sChatHandler.SystemMessage(Warned->GetSession(), "[WarnSystem] Your account has been banned ingame for '%s'.", reason.c_str());
    sChatHandler.SystemMessage(Warned->GetSession(), "[WarnSystem] This ban is due to expire when hell freezes over.");

    std::string reasonTagged = "[WarnSystem]" + reason;
    Warned->SetBanned( 1, reasonTagged );
    Warned->SaveToDB(false);

    const char * pAccount = Warned->GetSession()->GetAccountName().c_str();
    sLogonCommHandler.Account_SetBanned(pAccount, 1, reasonTagged.c_str());
    sWorld.DisconnectUsersWithAccount(pAccount, Warner->GetSession());

    BroadcastToWorld("%s's account will now be locked until the day hell freezes over.", Warned->GetName() );
}

void WarnSystem::Punishment_BanCharacter(Player * Warner, Player * Warned, uint32 NewWarnCnt, std::string reason)
{
    int32 BanTime = ((NewWarnCnt % 4) * 4) * TIME_HOUR;
    if( Warner && Warner->GetSession() )
    {
        sChatHandler.SystemMessage(Warner->GetSession(), "[WarnSystem] Banning character '%s' ingame for '%s'.", Warned->GetName(), reason.c_str());
        sChatHandler.SystemMessage(Warner->GetSession(), "[WarnSystem] This ban is due to expire %s%s.", BanTime ? "on " : "", BanTime ? ConvertTimeStampToDataTime(BanTime+(uint32)UNIXTIME).c_str() : "when hell freezes over");
    }
    sChatHandler.SystemMessage(Warned->GetSession(), "[WarnSystem] This ban is due to expire %s%s.", BanTime ? "on " : "", BanTime ? ConvertTimeStampToDataTime(BanTime+(uint32)UNIXTIME).c_str() : "when hell freezes over");
    sChatHandler.SystemMessage(Warned->GetSession(), "[WarnSystem] Your character has been banned ingame for '%s'.", reason.c_str());

    uint32 uBanTime = BanTime ? BanTime + (uint32)UNIXTIME : 1;

    BroadcastToWorld("Character %s will now be locked until %s.", Warned->GetName(), BanTime ? ConvertTimeStampToDataTime(uBanTime).c_str() : "the day hell freezes over" );

    string reasonTagged = "[WarnSystem] " + reason;
    Warned->SetBanned(uBanTime, reasonTagged);
    Warned->SaveToDB(false);

    sEventMgr.AddEvent(Warned, &Player::SoftDisconnect, EVENT_PLAYER_SOFT_DISCONNECT, 5000, 1, 0);
}

void WarnSystem::Punishment_None()
{

}
