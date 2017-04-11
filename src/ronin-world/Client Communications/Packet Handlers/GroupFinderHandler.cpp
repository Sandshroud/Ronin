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

void SendQueueCommandResult(Player *plr, uint8 type, uint32 queueId, bool groupUnk, bool joinPending, bool inQueue, std::vector<uint32> dungeonSet, time_t unkTime, std::string unkComment)
{
    WoWGuid guid = plr->GetGUID();

    WorldPacket data(SMSG_LFG_UPDATE_STATUS, 200);
    data.WriteBit(guid[1]);
    data.WriteBit(groupUnk);
    data.WriteBits<uint32>(dungeonSet.size(), 24);
    data.WriteBit(guid[6]);
    data.WriteBit(!dungeonSet.empty());
    data.WriteBits(unkComment.length(), 9);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[2]);
    data.WriteBit(joinPending);
    data.WriteBit(guid[0]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[5]);
    data.WriteBit(inQueue);
    data.FlushBits();
    data << uint8(type);
    data.WriteString(unkComment);
    data << uint32(queueId);
    data << uint32(unkTime);
    data.WriteByteSeq(guid[6]);
    for(uint8 i = 0; i < 3; i++)
        data << uint8(0);
    data.WriteSeqByteString(6, guid, 1, 2, 4, 3, 5, 0);
    data << uint32(42);
    data.WriteByteSeq(guid[7]);
    for(auto itr = dungeonSet.begin(); itr != dungeonSet.end(); itr++)
        data << uint32(*itr);
    plr->PushPacket(&data);
}

void WorldSession::HandleLFGGetStatusOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    uint8 queueStatus;
    uint32 queueId, queueStep = 0;
    time_t dataTimer = UNIXTIME;
    std::vector<uint32> dungeonSet;
    if(!sGroupFinder.GetPlayerQueueStatus(_player, queueStatus, queueId, queueStep, &dungeonSet, dataTimer))
        return;

    // Status sending is interesting, we need to send two status packets, one with current data
    // And the other to clear out the opposite data
    SendQueueCommandResult(_player, queueStatus, queueId, _player->InGroup(), false, false, dungeonSet, dataTimer, "");
    SendQueueCommandResult(_player, queueStatus, queueId, !_player->InGroup(), false, false, dungeonSet, dataTimer, "");
}

void WorldSession::HandleLFGLockInfoRequestOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    bool party = !recvPacket.ReadBit();
    if(party)
    {
        if(Group *grp = _player->GetGroup())
            grp->SendLFGLockInfo(_player);
        return;
    }

    WorldPacket data(SMSG_LFG_PLAYER_INFO, 200);
    sGroupFinder.BuildRandomDungeonData(_player, &data);
    sGroupFinder.BuildPlayerLockInfo(_player, &data);
    _player->PushPacket(&data);
}

void WorldSession::HandleLFGSetRolesOpcode(WorldPacket& recvPacket)
{

}

void WorldSession::HandleLFGTeleportOpcode(WorldPacket& recvPacket)
{

}

void WorldSession::HandleLFGJoinOpcode(WorldPacket& recvPacket)
{

}

void WorldSession::HandleLFGLeaveOpcode(WorldPacket& recvPacket)
{

}

void WorldSession::HandleLFGRaidJoinOpcode(WorldPacket& recvPacket)
{

}

void WorldSession::HandleLFGRaidLeaveOpcode(WorldPacket& recvPacket)
{

}

void WorldSession::HandleLFGProposalResultOpcode(WorldPacket& recvPacket)
{

}

void WorldSession::HandleLFGSetBootVoteOpcode(WorldPacket& recvPacket)
{

}

void WorldSession::HandleLFGSetCommentOpcode(WorldPacket& recvPacket)
{

}
