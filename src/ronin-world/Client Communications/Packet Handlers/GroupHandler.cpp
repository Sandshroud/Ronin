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

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GROUP_INVITE
//////////////////////////////////////////////////////////////
void WorldSession::HandleGroupInviteOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    std::string realmName, memberName;
    recv_data.read_skip<uint32>(); // cross-realm party related
    recv_data.read_skip<uint32>(); // roles mask?
    recv_data.ReadGuidBitString(2, guid, 2, 7);
    size_t realmLength = recv_data.ReadBits(9);
    recv_data.ReadGuidBitString(1, guid, 3);
    size_t nameLength = recv_data.ReadBits(10);
    recv_data.ReadGuidBitString(5, guid, 5, 4, 6, 0, 1);
    recv_data.ReadGuidByteString(3, guid, 4, 7, 6);
    realmName = recv_data.ReadString(realmLength);
    memberName = recv_data.ReadString(nameLength);
    recv_data.ReadGuidByteString(5, guid, 0, 1, 2, 3, 5);
    if(_player->HasBeenInvited())
        return;

    Player *player = objmgr.GetPlayer(memberName.c_str(), false);
    if ( player == NULL)
    {
        SendPartyCommandResult(_player, 0, memberName, ERR_PARTY_CANNOT_FIND);
        return;
    }

    if (player == _player)
    {
        return;
    }

    if ( _player->InGroup() && (!_player->IsGroupLeader() || _player->GetGroup()->HasFlag(GROUP_FLAG_BATTLEGROUND_GROUP) ) )
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        return;
    }

    if ( Group *group = _player->GetGroup() )
    {
        if (group->IsFull())
        {
            SendPartyCommandResult(_player, 0, "", ERR_PARTY_IS_FULL);
            return;
        }
    }

    bool inviteAvailable = true;
    if ( player->InGroup() )
    {
        inviteAvailable = false;
        SendPartyCommandResult(_player, player->GetGroup()->GetGroupType(), memberName, ERR_PARTY_ALREADY_IN_GROUP);
    }
    else
    {
        if(player->GetTeam()!=_player->GetTeam() && _player->GetSession()->GetPermissionCount() == 0 && !sWorld.cross_faction_world)
        {
            SendPartyCommandResult(_player, 0, memberName, ERR_PARTY_WRONG_FACTION);
            return;
        }

        if ( player->HasBeenInvited() )
        {
            SendPartyCommandResult(_player, 0, memberName, ERR_PARTY_ALREADY_IN_GROUP);
            return;
        }

        if( player->Social_IsIgnoring( _player->GetLowGUID() ) )
        {
            SendPartyCommandResult(_player, 0, memberName, ERR_PARTY_IS_IGNORING_YOU);
            return;
        }

        if( player->hasGMTag() && !HasGMPermissions())
        {
            SendPartyCommandResult(_player, 0, memberName, ERR_PARTY_CANNOT_FIND);
            return;
        }

        SendPartyCommandResult(GetPlayer(), 0, memberName, ERR_PARTY_NO_ERROR);
        player->SetInviter(_player->GetGUID());
    }

    WorldPacket data(SMSG_GROUP_INVITE, 100);
    data.WriteBit(0);
    data.WriteGuidBitString(3, guid, 0, 3, 2);
    data.WriteBit(inviteAvailable);   // Invite available
    data.WriteGuidBitString(2, guid, 6, 5);
    data.WriteBits(0, 9);   // realm name length
    data.WriteBit(guid[4]);
    data.WriteBits(strlen(player->GetName()), 7);
    data.WriteBits(0, 24);  // count
    data.WriteBit(0);
    data.WriteSeqByteString(2, guid, 1, 7);

    data.WriteSeqByteString(2, guid, 1, 4);
    data << uint32(getMSTime());
    data << uint32(0) << uint32(0);
    data.WriteSeqByteString(4, guid, 6, 0, 2, 3);
    // for(int i = 0; i < count; ++i)
    //    data << uint32(0);
    data.WriteSeqByteString(2, guid, 5, 7);
    data.append(player->GetName(), strlen(player->GetName()));
    data << uint32(0);
    player->PushPacket(&data);
}

void WorldSession::HandleGroupInviteResponseOpcode(WorldPacket &recv_data)
{
    bool crossRealm = recv_data.ReadBit();
    bool accepted = recv_data.ReadBit();
    if (crossRealm) recv_data.read_skip<uint32>();
    if(accepted) HandleGroupAcceptOpcode(recv_data);
    else HandleGroupCancelOpcode(recv_data);
}

///////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_CANCEL:
///////////////////////////////////////////////////////////////
void WorldSession::HandleGroupCancelOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    sLog.Debug( "WORLD"," got CMSG_GROUP_CANCEL." );
}

////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_ACCEPT:
////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupAcceptOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    Player* player = objmgr.GetPlayer(_player->GetInviter());
    if(!player)
        return;

    player->ClearGroupInviter();
    _player->ClearGroupInviter();

    if(Group *grp = player->GetGroup())
    {
        if(grp->AddMember(_player->m_playerInfo))
        {
            if(grp->GetLeader()->m_loggedInPlayer)
            {
                _player->iInstanceType = grp->GetLeader()->m_loggedInPlayer->iInstanceType;
                _player->iRaidType = grp->GetLeader()->m_loggedInPlayer->iRaidType;
            }

            _player->PushData(MSG_SET_DUNGEON_DIFFICULTY, 4, &_player->iInstanceType);
            _player->PushData(MSG_SET_RAID_DIFFICULTY, 4, &_player->iRaidType);
        }
        return;
    }

    // If we're this far, it means we have no existing group, and have to make one.
    Group *grp = new Group(true);
    grp->AddMember(player->m_playerInfo);       // add the inviter first, therefore he is the leader
    grp->AddMember(_player->m_playerInfo);      // add us.
    grp->SetDifficulty(player->iInstanceType);  // Set our instance difficulty.
    grp->SetRaidDifficulty(player->iRaidType);  // Set our raid difficulty.
    _player->iInstanceType = player->iInstanceType;
    _player->iRaidType = player->iRaidType;
    _player->PushData(MSG_SET_DUNGEON_DIFFICULTY, 4, &player->iInstanceType);
    _player->PushData(MSG_SET_RAID_DIFFICULTY, 4, &player->iRaidType);
}

///////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_DECLINE:
//////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupDeclineOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    if(Player* player = objmgr.GetPlayer(_player->GetInviter()))
    {
        WorldPacket data(SMSG_GROUP_DECLINE, 100);
        data << GetPlayer()->GetName();
        player->PushPacket( &data );

        player->ClearGroupInviter();
        _player->ClearGroupInviter();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_UNINVITE:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupUninviteOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    std::string membername;
    recv_data >> membername;
    if(PlayerInfo *info = objmgr.GetPlayerInfoByName(membername.c_str()))
        if(Player *player = objmgr.GetPlayer(membername.c_str(), false))
            _player->GroupUninvite(player, info);
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_UNINVITE_GUID:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupUninviteGUIDOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    recv_data >> guid;
    if(PlayerInfo *info = objmgr.GetPlayerInfo(guid))
        if(Player *player = objmgr.GetPlayer(guid))
            _player->GroupUninvite(player, info);
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_SET_LEADER:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupSetLeaderOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    WoWGuid MemberGuid;
    recv_data >> MemberGuid;

    Player* player = objmgr.GetPlayer(MemberGuid);
    if ( player == NULL )
    {
        SendPartyCommandResult(_player, 0, _player->GetName(), ERR_PARTY_CANNOT_FIND);
        return;
    }

    if(!_player->IsGroupLeader())
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        return;
    }

    if(player->GetGroup() != _player->GetGroup())
    {
        SendPartyCommandResult(_player, 0, _player->GetName(), ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
        return;
    }

    Group *pGroup = _player->GetGroup();
    if(pGroup) pGroup->SetLeader(player,false);
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_DISBAND:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupDisbandOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    Group* pGroup = _player->GetGroup();
    if(!pGroup)
        return;

    if(pGroup->HasFlag(GROUP_FLAG_BATTLEGROUND_GROUP))
        return;

    //Update the group list
    pGroup->SendNullUpdate( _player );

    pGroup->RemovePlayer(_player->m_playerInfo);
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_LOOT_METHOD:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleLootMethodOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    WoWGuid lootMaster;
    uint32 lootMethod, threshold;
    recv_data >> lootMethod >> lootMaster >>threshold;

    if(!_player->IsGroupLeader())
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        return;
    }

    Group* pGroup = _player->GetGroup();
    if( pGroup != NULL)
    {
        Player* pLooter = objmgr.GetPlayer(lootMaster) ? objmgr.GetPlayer(lootMaster) : _player;
        pGroup->SetLooter(pLooter , lootMethod, threshold );
    }
}

void WorldSession::HandleMinimapPingOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    float x, y;
    recv_data >> x >> y;
    if(Group *party = _player->GetGroup())
    {
        WorldPacket data(MSG_MINIMAP_PING, 16);
        data << _player->GetGUID();
        data << x << y;
        party->SendPacketToAllButOne(&data, _player);
    }
}

void WorldSession::HandleSetPlayerIconOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();

    Group * pGroup = _player->GetGroup();
    if(pGroup == NULL)
        return;

    uint64 guid;
    uint8 icon;

    recv_data >> icon;
    if(icon == 0xFF)
    {
        // client request
        WorldPacket data(MSG_RAID_TARGET_UPDATE, 73);
        data << uint8(1);
        for(uint8 i = 0; i < 8; i++)
            data << i << pGroup->m_targetIcons[i];

        SendPacket(&data);
    }
    else if(_player->IsGroupLeader())
    {
        recv_data >> guid;
        if(icon > 7)
            icon = 7;   // whoops, buffer overflow :p

        // Null last icon
        for(uint8 i = 0; i < 8; i++)
        {
            if( pGroup->m_targetIcons[i] == guid )
            {
                WorldPacket data(MSG_RAID_TARGET_UPDATE, 10);
                data << uint8(0);
                data << uint64(0);
                data << uint8(i);
                data << uint64(0);
                pGroup->SendPacketToAll(&data);

                pGroup->m_targetIcons[i] = 0;
                break;
            }
        }

        // setting icon
        WorldPacket data(MSG_RAID_TARGET_UPDATE, 10);
        data << uint8(0) << uint64(GetPlayer()->GetGUID()) << icon << guid;
        pGroup->SendPacketToAll(&data);

        pGroup->m_targetIcons[icon] = guid;
    }
}

void WorldSession::SendPartyCommandResult(Player* pPlayer, uint32 p1, std::string name, uint32 err)
{
    CHECK_INWORLD_RETURN();
    // if error message do not work, please sniff it and leave me a message
    if(pPlayer)
    {
        WorldPacket data(SMSG_PARTY_COMMAND_RESULT, name.length()+4+4+4+8);
        data << uint32(p1);
        if(!name.length())
            data << uint8(0);
        else data << name.c_str();
        data << uint32(err);
        data << uint32(0);
        data << uint64(0);
        pPlayer->PushPacket(&data);
    }
}
