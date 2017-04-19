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

void WorldSession::HandleLFGGetStatusOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    uint8 queueStatus;
    uint32 queueId, queueStep = 0;
    time_t dataTimer = UNIXTIME;
    std::vector<uint32> *dungeonSet;
    if(!sGroupFinder.GetPlayerQueueStatus(_player, queueStatus, queueId, queueStep, &dungeonSet, dataTimer))
        return;

    // Status sending is interesting, we need to send two status packets, one with current data
    // And the other to clear out the opposite data
    sGroupFinder.SendQueueCommandResult(_player, queueStep, queueId, _player->InGroup(), queueStatus, dungeonSet, dataTimer, "");
    sGroupFinder.SendQueueCommandResult(_player, queueStep, queueId, !_player->InGroup(), queueStatus, dungeonSet, dataTimer, "");
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
    CHECK_INWORLD_RETURN();

    uint8 roleMask;
    recvPacket >> roleMask;

    sGroupFinder.UpdateRoles(_player, roleMask);
    sLog.outDebug("PLAYER %u ROLE UPDATED: %u | Leader: %s", _player->GetLowGUID(), ((roleMask & ROLEMASK_ROLE_TYPE) >> 1), (roleMask&0x01) ? "Yes" : "No");
}

void WorldSession::HandleLFGTeleportOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    // Teleport us to our dungeon or out of it
    sGroupFinder.TeleportPlayer(_player, recvPacket.read<uint8>() == 0);
}

void WorldSession::HandleLFGJoinOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    uint32 roleMask = recvPacket.read<uint32>();
    recvPacket.read_skip<uint32>();
    recvPacket.read_skip<uint64>();
    uint32 commLength = recvPacket.ReadBits(9);
    uint32 count = recvPacket.ReadBits(24);
    if(count == 0 || count >= 155)
        return;

    bool containsDungeon = false, containsRaid = false, containsRaidList = false;
    std::string comment = recvPacket.ReadString(commLength);
    std::vector<uint32> dungeonSet;
    for(uint32 i = 0; i < count; i++)
    {
        uint32 dungeonId = (recvPacket.read<uint32>() & 0x00FFFFFF);
        if(dungeonId == 0) // Pushing a type only
            continue;
        LFGDungeonsEntry *entry = dbcLFGDungeons.LookupEntry(dungeonId);
        if(entry == NULL) // Cut here, and return an error if we have an invalid list
        {
            sGroupFinder.SendLFGJoinResult(_player, LFG_ERROR_FINDER_LIST_ERR, NULL);
            return;
        }

        if(entry->LFGType == DBC_LFG_TYPE_RAIDLIST)
            containsRaidList = true;
        else if(entry->mapEntry && entry->mapEntry->IsRaid())
            containsRaid = true;
        else containsDungeon = true;

        dungeonSet.push_back(dungeonId);
    }

    // Contains raid and raidlist set to 2 as currently we don't support raid setups through GroupFinder
    uint8 typeCount = (containsRaidList ? 2 : 0) + (containsRaid ? 2 : 0) + (containsDungeon ? 1 : 0);
    if(typeCount != 1) // Only allow single queue types at once
    {
        sGroupFinder.SendLFGJoinResult(_player, LFG_ERROR_FINDER_LIST_ERROR2, NULL);
        return;
    }

    sGroupFinder.HandleDungeonJoin(_player, roleMask, &dungeonSet, comment);
}

void WorldSession::HandleLFGLeaveOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    recvPacket.read_skip<uint64>();
    recvPacket.read_skip<uint32>();
    uint32 queueId = recvPacket.read<uint32>();

    WoWGuid guid;
    recvPacket.ReadGuidBitString(8, guid, 4, 5, 0, 6, 2, 7, 1, 3);
    recvPacket.ReadGuidByteString(8, guid, 7, 4, 3, 2, 6, 0, 1, 5);

    sGroupFinder.HandleDungeonLeave(_player, guid, queueId);
}

void WorldSession::HandleLFGRaidListQueryOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    uint32 raidDungeonId = (recvPacket.read<uint32>() & 0x00FFFFFF);
    if(raidDungeonId == 0)
        return; // We mask out type, so make sure we aren't just sending type

}

void WorldSession::HandleLFGRaidListLeaveOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    uint32 raidDungeonId = (recvPacket.read<uint32>() & 0x00FFFFFF);
    if(raidDungeonId == 0)
        return; // We mask out type, so make sure we aren't just sending type

}

void WorldSession::HandleLFGProposalResultOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    // Our counterId
    uint32 proposalCounter = recvPacket.read<uint32>();
    recvPacket.read_skip<uint64>();
    recvPacket.read_skip<uint32>();

    WoWGuid guid, guid2;
    recvPacket.ReadGuidBitString(8, guid, 4, 5, 0, 6, 2, 7, 1, 3);
    recvPacket.ReadGuidByteString(8, guid, 7, 4, 3, 2, 6, 0, 1, 5);

    guid2[7] = recvPacket.ReadBit();
    bool result = recvPacket.ReadBit();
    recvPacket.ReadGuidBitString(7, guid2, 1, 3, 0, 5, 4, 6, 2);
    recvPacket.ReadGuidByteString(8, guid2, 7, 1, 5, 6, 3, 4, 0, 2);

    sGroupFinder.UpdateProposal(_player, proposalCounter, result, guid, guid2);
}

void WorldSession::HandleLFGSetBootVoteOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    sGroupFinder.UpdateBootVote(_player, recvPacket.read<uint8>() > 0);
}

void WorldSession::HandleLFGSetCommentOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    uint32 length = recvPacket.ReadBits(9);
    std::string comment = recvPacket.ReadString(length);

    sGroupFinder.UpdateComment(_player, comment);
}
