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

#pragma once

class SERVER_DECL WarnSystem :  public Singleton < WarnSystem >
{
    friend class ChatHandler;

public:
    void AddWarn(Player * Warner, Player * Warned, const char  *reason);
    void AddWarn_Internal(std::string Warner, Player * Warned, const char  *reason);
    void ReachedPunishmentLevel(Player * Warner, Player * Warned, uint32 NewWarnCnt, const char * reason);
    void DelWarn(Player * Warner, Player * Warned, uint32 WarnID);
    void ListWarns(Player * Warner, Player * Warned);
    void ClearWarns(Player * Warner, Player * Warned);

    void BroadcastToWorld(const char* Format, ...);
    void BroadcastToGMs(const char* Format, ...);

private:
    uint32 GetWarnCountForAccount(std::string WarnedAcct);
    void Punishment_PermBanAccount(Player * Warner, Player * Warned, uint32 NewWarnCnt, std::string);
    void Punishment_BanCharacter(Player * Warner, Player * Warned, uint32 NewWarnCnt, std::string);
    void Punishment_Kick(Player * Warner, Player * Warned, uint32 NewWarnCnt, std::string);
    bool Punishment_TakeGold(Player * Warned, uint32 NewWarnCnt);
    void Punishment_None();
};

#define sWarnSystem WarnSystem::getSingleton()
