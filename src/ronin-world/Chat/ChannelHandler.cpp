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

initialiseSingleton( ChannelMgr );

void WorldSession::HandleChannelJoin(WorldPacket& recvPacket)
{
    uint32 dbc_id;
    recvPacket >> dbc_id;
    uint8 unk1 = recvPacket.ReadBit(), unk2 = recvPacket.ReadBit();
    uint8 len = recvPacket.ReadBits(8), len2 = recvPacket.ReadBits(8);
    std::string channelname = recvPacket.ReadString(len), pass = recvPacket.ReadString(len2);

    if(dbc_id)
		_player->EventDBCChatUpdate(_player->GetMapInstance(), dbc_id);
    else
    {
        if( sWorld.GmClientChannel.size() && !stricmp(sWorld.GmClientChannel.c_str(), channelname.c_str()) && !GetPermissionCount())
            return;

        if(Channel *chn = channelmgr.GetCreateChannel(channelname.c_str(), _player))
			chn->AttemptJoin(_player, pass.c_str());
    }
}

void WorldSession::HandleChannelLeave(WorldPacket& recvPacket)
{
    uint32 code = 0;
    std::string channelname;

    recvPacket >> code;
    recvPacket >> channelname;
    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(), _player))
        chn->Part(_player, false);
}

void WorldSession::HandleChannelList(WorldPacket& recvPacket)
{
    std::string channelname;
    recvPacket >> channelname;
    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(), _player))
        chn->List(_player);
}

void WorldSession::HandleChannelPassword(WorldPacket& recvPacket)
{
    std::string channelname, pass;
    Channel * chn;

    recvPacket >> channelname;
    recvPacket >> pass;
    chn = channelmgr.GetChannel(channelname.c_str(),_player);
    if( chn != NULL )
        chn->Password(_player, pass.c_str());
}

void WorldSession::HandleChannelSetOwner(WorldPacket& recvPacket)
{
    std::string channelname, newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->SetOwner(_player, plr);
}

void WorldSession::HandleChannelOwner(WorldPacket& recvPacket)
{
    std::string channelname;
    recvPacket >> channelname;
    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(),_player))
        chn->GetOwner(_player);
}

void WorldSession::HandleChannelModerator(WorldPacket& recvPacket)
{
    std::string channelname, newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->GiveModerator(_player, plr);
}

void WorldSession::HandleChannelUnmoderator(WorldPacket& recvPacket)
{
    std::string channelname, newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->TakeModerator(_player, plr);
}

void WorldSession::HandleChannelMute(WorldPacket& recvPacket)
{
    std::string channelname, newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->Mute(_player, plr);
}

void WorldSession::HandleChannelUnmute(WorldPacket& recvPacket)
{
    std::string channelname, newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->Unmute(_player, plr);
}

void WorldSession::HandleChannelInvite(WorldPacket& recvPacket)
{
    std::string channelname, newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->Invite(_player, plr);
}
void WorldSession::HandleChannelKick(WorldPacket& recvPacket)
{
    std::string channelname, newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->Kick(_player, plr, false);
}

void WorldSession::HandleChannelBan(WorldPacket& recvPacket)
{
    std::string channelname, newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->Kick(_player, plr, true);
}

void WorldSession::HandleChannelUnban(WorldPacket& recvPacket)
{
    std::string channelname, newp;
    Channel * chn;
    PlayerInfo * plr;

    recvPacket >> channelname >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayerInfoByName(newp.c_str());
    if( chn != NULL && plr != NULL )
        chn->Unban(_player, plr);
}

void WorldSession::HandleChannelAnnounce(WorldPacket& recvPacket)
{
    std::string channelname;
    recvPacket >> channelname;

    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(), _player))
        chn->Announce(_player);
}

void WorldSession::HandleChannelModerate(WorldPacket& recvPacket)
{
    std::string channelname;
    recvPacket >> channelname;

    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(), _player))
        chn->Moderate(_player);
}

void WorldSession::HandleChannelRosterQuery(WorldPacket & recvPacket)
{
    std::string channelname;
    recvPacket >> channelname;

    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(), _player))
        chn->List(_player);
}

void WorldSession::HandleChannelNumMembersQuery(WorldPacket & recvPacket)
{
    std::string channel_name;
    recvPacket >> channel_name;
    if(Channel *chn = channelmgr.GetChannel(channel_name.c_str(), _player, true))
    {
        WorldPacket data(SMSG_CHANNEL_MEMBER_COUNT, recvPacket.size() + 4);
        data << channel_name;
        data << uint8(chn->m_flags);
        data << uint32(chn->GetNumMembers());
        SendPacket(&data);
    }
}
