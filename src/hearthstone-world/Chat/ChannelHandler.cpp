/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton( ChannelMgr );

void WorldSession::HandleChannelJoin(WorldPacket& recvPacket)
{
    string channelname,pass;
    uint32 dbc_id = 0;
    uint16 crap;        // crap = some sort of channel type?
    uint32 i;
    Channel * chn;

    recvPacket >> dbc_id >> crap;
    recvPacket >> channelname;
    recvPacket >> pass;

    if(dbc_id)
		_player->EventDBCChatUpdate(dbc_id);
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
    string channelname;

    recvPacket >> code;
    recvPacket >> channelname;
    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(), _player))
        chn->Part(_player, false);
}

void WorldSession::HandleChannelList(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;
    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(), _player))
        chn->List(_player);
}

void WorldSession::HandleChannelPassword(WorldPacket& recvPacket)
{
    string channelname,pass;
    Channel * chn;

    recvPacket >> channelname;
    recvPacket >> pass;
    chn = channelmgr.GetChannel(channelname.c_str(),_player);
    if( chn != NULL )
        chn->Password(_player, pass.c_str());
}

void WorldSession::HandleChannelSetOwner(WorldPacket& recvPacket)
{
    string channelname,newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->SetOwner(_player, plr);
}

void WorldSession::HandleChannelOwner(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;
    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(),_player))
        chn->GetOwner(_player);
}

void WorldSession::HandleChannelModerator(WorldPacket& recvPacket)
{
    string channelname,newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->GiveModerator(_player, plr);
}

void WorldSession::HandleChannelUnmoderator(WorldPacket& recvPacket)
{
    string channelname,newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->TakeModerator(_player, plr);
}

void WorldSession::HandleChannelMute(WorldPacket& recvPacket)
{
    string channelname,newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->Mute(_player, plr);
}

void WorldSession::HandleChannelUnmute(WorldPacket& recvPacket)
{
    string channelname,newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->Unmute(_player, plr);
}

void WorldSession::HandleChannelInvite(WorldPacket& recvPacket)
{
    string channelname,newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->Invite(_player, plr);
}
void WorldSession::HandleChannelKick(WorldPacket& recvPacket)
{
    string channelname,newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->Kick(_player, plr, false);
}

void WorldSession::HandleChannelBan(WorldPacket& recvPacket)
{
    string channelname,newp;
    Channel * chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if( chn != NULL && plr != NULL )
        chn->Kick(_player, plr, true);
}

void WorldSession::HandleChannelUnban(WorldPacket& recvPacket)
{
    string channelname,newp;
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
    string channelname;
    recvPacket >> channelname;

    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(), _player))
        chn->Announce(_player);
}

void WorldSession::HandleChannelModerate(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;

    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(), _player))
        chn->Moderate(_player);
}

void WorldSession::HandleChannelRosterQuery(WorldPacket & recvPacket)
{
    string channelname;
    recvPacket >> channelname;

    if(Channel *chn = channelmgr.GetChannel(channelname.c_str(), _player))
        chn->List(_player);
}

void WorldSession::HandleChannelNumMembersQuery(WorldPacket & recvPacket)
{
    string channel_name;
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
