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

void WorldSession::HandleAttackSwingOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    recv_data >> guid;
    if(guid.empty() || _player->isDead())
    {
        _player->EventAttackStop();
        return;
    }

    // AttackSwing
    sLog.Debug( "WORLD"," Recvd CMSG_ATTACKSWING Message" );
    if(GetPlayer()->IsPacified() || GetPlayer()->IsStunned() || GetPlayer()->IsFeared())
        return;

    Unit* pEnemy = _player->GetMapInstance()->GetUnit(guid);
    if(pEnemy == NULL || pEnemy->isDead())
    {
        sLog.outDebug("WORLD: %016llX does not exist.", guid);
        _player->EventAttackStop();
        return;
    }

    // Implemented Hackfix for quest 1640
    if(!(pEnemy->GetEntry() == 6090 && _player->HasQuest(1640)) && sFactionSystem.AC_GetAttackableStatus(GetPlayer(), pEnemy))
    {
        sWorld.LogCheater(this, "Faction exploit detected. Damagetype: Melee.");
        GetPlayer()->BroadcastMessage("Faction exploit detected. You will be disconnected in 5 seconds.");
        GetPlayer()->Kick(5000);
        return;
    }

    _player->EventAttackStart(guid);
}

void WorldSession::HandleAttackStopOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    GetPlayer()->EventAttackStop();
}

