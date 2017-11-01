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

void WorldSession::HandleDuelAccepted(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    if( !_player->IsInDuel() )
        return;
    DuelStorage *storage = NULL;
    if( (storage = _player->GetDuelStorage())->duelState != DUEL_STATE_REQUESTED )
        return;
    // Only the person asked for a duel accepts so we should be using 0 but check anyway
    Player *target = _player->GetMapInstance()->GetPlayer(storage->duelists[_player->GetGUID() == storage->duelists[0] ? 1 : 0]);
    if(target == NULL)
        return;

    // Update duelstate
    _player->GetDuelStorage()->duelState = DUEL_STATE_STARTED;

    WorldPacket data( SMSG_DUEL_COUNTDOWN, 4 );
    data << uint32( 3000 );
    SendPacket( &data );
    target->PushPacket( &data );
}

void WorldSession::HandleDuelCancelled(WorldPacket & recv_data)
{
    if(!_player->IsInDuel())
        return;
    DuelStorage *storage = NULL;
    if( !(storage = _player->GetDuelStorage())->quitter.empty() )
        return;

    storage->quitter = _player->GetGUID();
}
