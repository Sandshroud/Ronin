/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Mutex m_confSettingLock;
vector<string> m_bannedChannels;

void Channel::LoadConfSettings()
{
    string BannedChannels = mainIni->ReadString("Channels", "BannedChannels", "");
    m_confSettingLock.Acquire();
    m_bannedChannels = StrSplit(BannedChannels, ";");
    m_confSettingLock.Release();
}

bool Channel::HasMember(Player* pPlayer)
{
    m_lock.Acquire();
    if( m_members.find(pPlayer) == m_members.end() )
    {
        m_lock.Release();
        return false;
    }
    else
    {
        m_lock.Release();
        return true;
    }
}

Channel::Channel(const char * name, uint32 team, uint32 type_id, uint32 id)
{
    m_flags = 0;
    m_announce = true;
    m_muted = false;
    m_general = false;
    m_name = std::string(name);
    m_team = team;
    m_typeId = type_id;
    m_channelId = id;
    m_deleted = false;

    pDBC = dbcChatChannels.LookupEntry(type_id);
    if( pDBC != NULL )
    {
        m_general = true;
        m_announce = false;

        m_flags |= 0x10;            // general flag
        // flags (0x01 = custom?, 0x04 = trade?, 0x20 = city?, 0x40 = lfg?, , 0x80 = voice?,

        if( pDBC->flags & 0x08 )
            m_flags |= 0x08;        // trade

        if( pDBC->flags & 0x10 || pDBC->flags & 0x20 )
            m_flags |= 0x20;        // city flag

        if( pDBC->flags & 0x40000 )
            m_flags |= 0x40;        // lfg flag
    }
    else
        m_flags = 0x01;
}

Channel::~Channel()
{
    m_lock.Acquire();
    for(MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); itr++)
        itr->first->LeftChannel(this);
    m_lock.Release();
    m_deleted = true;
}

void Channel::UserListJoinNotify(Player* plr)
{
    WorldPacket data(SMSG_USERLIST_ADD, 8+1+1+4+m_name.size()+1);
    if(!m_general)
        data.SetOpcode(SMSG_USERLIST_UPDATE);

    data << plr->GetGUID();
    data << uint8(plr->GetChatTag());
    data << uint8(m_flags);
    data << uint32(GetNumMembers());
    data << m_name;
    SendToAll(&data);
}

void Channel::UserListLeaveNotify(Player* plr)
{
    WorldPacket data(SMSG_USERLIST_REMOVE, 8+1+4+m_name.size()+1);
    data << plr->GetGUID();
    data << uint8(m_flags);
    data << uint32(GetNumMembers());
    data << m_name;
    SendToAll(&data);
}

void Channel::AttemptJoin(Player* plr, const char * password)
{
    if(plr == NULL)
        return;
    Guard mGuard(m_lock);
    WorldPacket data(100);
    uint32 flags = CHANNEL_FLAG_NONE;

    if( !m_general && plr->GetSession()->CanUseCommand('c') )
        flags |= CHANNEL_FLAG_MODERATOR;

    if(!m_password.empty() && strcmp(m_password.c_str(), password) != 0)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_WRONGPASS);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(m_bannedMembers.find(plr->GetLowGUID()) != m_bannedMembers.end())
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_YOURBANNED);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(m_members.find(plr) != m_members.end())
    {
        if( !m_general )
        {
            MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_ALREADY_ON);
            plr->GetSession()->SendPacket(&data);
        }

        return;
    }

    if(m_members.empty() && !m_general)
        flags |= CHANNEL_FLAG_OWNER;

    plr->JoinedChannel(this);
    m_members.insert(make_pair(plr, flags));

    if(m_announce && !plr->bGMTagOn)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_JOINED);
        data << plr->GetGUID();
        SendToAll(&data, NULLPLR);
    }

    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_YOUJOINED);
    data << m_flags << m_typeId << uint32(0);
    plr->GetSession()->SendPacket(&data);

    UserListJoinNotify(plr);
}

void Channel::Part(Player* plr, bool silent, bool keepData)
{
    if(plr == NULL)
        return;

    m_lock.Acquire();
    WorldPacket data;
    uint32 flags;
    MemberMap::iterator itr = m_members.find(plr);
    if(itr == m_members.end())
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        m_lock.Release();
        return;
    }

    flags = itr->second;
    m_members.erase(itr);

    plr->LeftChannel(this);

    if(flags & CHANNEL_FLAG_OWNER)
    {
        // we need to find a new owner
        SetOwner(NULLPLR, NULLPLR);
    }

    if(!silent)
    {
        if(!(plr->GetSession() && (plr->GetSession()->IsLoggingOut() || plr->m_TeleportState == 1)))
        {
            MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_YOULEFT);
            data << m_typeId << uint8(keepData ? 0x1 : 0x0);
            plr->GetSession()->SendPacket(&data);
        }
    }

    if(m_announce && !plr->bGMTagOn)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_LEFT);
        data << plr->GetGUID();
        SendToAll(&data);
    }

    UserListLeaveNotify(plr);
    m_lock.Release();
}

void Channel::SetOwner(Player* oldpl, Player* plr)
{
    Guard mGuard(m_lock);
    Player* pOwner = NULLPLR;
    uint32 oldflags = 0;
    uint32 oldflags2 = 0;
    WorldPacket data;
    if(oldpl != NULL)
    {
        MemberMap::iterator itr = m_members.find(oldpl);
        if(m_members.end() == itr)
        {
            MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
            plr->GetSession()->SendPacket(&data);
            return;
        }

        if(!(itr->second & CHANNEL_FLAG_OWNER))
        {
            MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOT_OWNER);
            plr->GetSession()->SendPacket(&data);
            return;
        }
    }

    if(plr == NULL)
    {
        for(MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); itr++)
        {
            if(itr->second & CHANNEL_FLAG_OWNER)
            {
                // remove the old owner
                oldflags2 = itr->second;
                itr->second &= ~CHANNEL_FLAG_OWNER;
                MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_MODE_CHG);
                data << itr->first->GetGUID() << uint8(oldflags2) << uint8(itr->second);
                SendToAll(&data);
            }
            else
            {
                if(pOwner == NULL)
                {
                    pOwner = itr->first;
                    oldflags = itr->second;
                    itr->second |= CHANNEL_FLAG_OWNER;
                }
            }
        }
    }
    else
    {
        for(MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); itr++)
        {
            if(itr->second & CHANNEL_FLAG_OWNER)
            {
                // remove the old owner
                oldflags2 = itr->second;
                itr->second &= ~CHANNEL_FLAG_OWNER;
                MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_MODE_CHG);
                data << itr->first->GetGUID() << uint8(oldflags2) << uint8(itr->second);
                SendToAll(&data);
            }
            else
            {
                if(plr == itr->first)
                {
                    pOwner = itr->first;
                    oldflags = itr->second;
                    itr->second |= CHANNEL_FLAG_OWNER;
                }
            }
        }
    }

    if(pOwner == NULL)
        return;     // obviously no members

    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_CHGOWNER);
    data << pOwner->GetGUID();
    SendToAll(&data);

    // send the mode changes
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << pOwner->GetGUID() << uint8(oldflags) << uint8(oldflags | CHANNEL_FLAG_OWNER);
    SendToAll(&data);
}

void Channel::Invite(Player* plr, Player* new_player)
{
    Guard mGuard(m_lock);
    if(m_members.find(plr) == m_members.end())
    {
        SendNotOn(plr);
        return;
    }

    if(m_members.find(new_player) != m_members.end())
    {
        SendAlreadyOn(plr, new_player);
        return;
    }

    WorldPacket data(100);
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_INVITED);
    data << plr->GetGUID();
    new_player->GetSession()->SendPacket(&data);

    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_YOU_INVITED);
    data << new_player->GetGUID();
    plr->GetSession()->SendPacket(&data);
}

void Channel::Moderate(Player* plr)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data;
    if(m_members.end() == itr)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(itr->second & CHANNEL_FLAG_OWNER || itr->second & CHANNEL_FLAG_MODERATOR) && !plr->GetSession()->CanUseCommand('c'))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    m_muted = !m_muted;
    MakeNotifyPacket(&data, (m_muted ? CHANNEL_NOTIFY_FLAG_MODERATED : CHANNEL_NOTIFY_FLAG_UNMODERATED));
    data << plr->GetGUID();
    SendToAll(&data);
}

void Channel::Say(Player* plr, const char * message, Player* for_gm_client, bool forced)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(strlen(message)+100);
    if(!forced)
    {
        if(m_members.end() == itr)
        {
            MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
            plr->GetSession()->SendPacket(&data);
            return;
        }

        if(itr->second & CHANNEL_FLAG_MUTED)
        {
            MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK);
            plr->GetSession()->SendPacket(&data);
            return;
        }

        if(m_muted && !(itr->second & CHANNEL_FLAG_VOICED) && !(itr->second & CHANNEL_FLAG_MODERATOR) && !(itr->second & CHANNEL_FLAG_OWNER))
        {
            MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK);
            plr->GetSession()->SendPacket(&data);
            return;
        }
    }

    data.SetOpcode(SMSG_MESSAGECHAT);
    data << uint8(CHAT_MSG_CHANNEL);
    data << uint32(0);      // language
    data << plr->GetGUID(); // guid
    data << uint32(0);      // rank?
    data << m_name;         // channel name
    data << plr->GetGUID(); // guid again?
    data << uint32(strlen(message)+1);
    data << message;
    data << (uint8)(plr->GetChatTag());
    if(for_gm_client != NULL)
        for_gm_client->GetSession()->SendPacket(&data);
    else
        SendToAll(&data);
}

void Channel::SendNotOn(Player* plr)
{
    WorldPacket data(100);
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
    plr->GetSession()->SendPacket(&data);
}

void Channel::SendAlreadyOn(Player* plr, Player* plr2)
{
    WorldPacket data(100);
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_ALREADY_ON);
    data << plr2->GetGUID();
    plr->GetSession()->SendPacket(&data);
}

void Channel::Kick(Player* plr, Player* die_player, bool ban)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(die_player);
    MemberMap::iterator me_itr = m_members.find(plr);
    WorldPacket data(100);
    uint32 flags;

    if(me_itr == m_members.end())
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(itr == m_members.end())
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << die_player->GetGUID();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(me_itr->second & CHANNEL_FLAG_OWNER || me_itr->second & CHANNEL_FLAG_MODERATOR))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    flags = itr->second;
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_KICKED);
    data << die_player->GetGUID();
    SendToAll(&data);

    if(ban)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_BANNED);
        data << die_player->GetGUID();
        SendToAll(&data);
    }

    m_members.erase(itr);

    if(flags & CHANNEL_FLAG_OWNER)
        SetOwner(NULLPLR, NULLPLR);

    if(ban)
        m_bannedMembers.insert(die_player->GetLowGUID());

    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_YOULEFT);
    data << m_typeId << uint32(0) << uint8(0);
    die_player->GetSession()->SendPacket(&data);
}

void Channel::Unban(Player* plr, PlayerInfo * bplr)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(100);
    if(m_members.end() == itr)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(itr->second & CHANNEL_FLAG_OWNER || itr->second & CHANNEL_FLAG_MODERATOR))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    set<uint32>::iterator it2 = m_bannedMembers.find(bplr->guid);
    if(it2 == m_bannedMembers.end())
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << uint64(bplr->guid);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_UNBANNED);
    data << uint64(bplr->guid);
    SendToAll(&data);
    m_bannedMembers.erase(it2);
}

void Channel::Voice(Player* plr, Player* v_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(v_player);
    WorldPacket data(100);
    if(m_members.end() == itr)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(m_members.end() == itr2)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << v_player->GetGUID();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(itr->second & CHANNEL_FLAG_OWNER || itr->second & CHANNEL_FLAG_MODERATOR))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second |= CHANNEL_FLAG_VOICED;
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << v_player->GetGUID() << uint8(oldflags) << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::Devoice(Player* plr, Player* v_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(v_player);
    WorldPacket data(100);
    if(m_members.end() == itr)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(m_members.end() == itr2)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << v_player->GetGUID();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(itr->second & CHANNEL_FLAG_OWNER || itr->second & CHANNEL_FLAG_MODERATOR))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second &= ~CHANNEL_FLAG_VOICED;
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << v_player->GetGUID() << uint8(oldflags) << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::Mute(Player* plr, Player* die_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(die_player);
    WorldPacket data(100);
    if(m_members.end() == itr)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(m_members.end() == itr2)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << die_player->GetGUID();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(itr->second & CHANNEL_FLAG_OWNER || itr->second & CHANNEL_FLAG_MODERATOR))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second |= CHANNEL_FLAG_MUTED;
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << die_player->GetGUID() << uint8(oldflags) << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::Unmute(Player* plr, Player* die_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(die_player);
    WorldPacket data(100);
    if(m_members.end() == itr)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(m_members.end() == itr2)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << die_player->GetGUID();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(itr->second & CHANNEL_FLAG_OWNER || itr->second & CHANNEL_FLAG_MODERATOR))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second &= ~CHANNEL_FLAG_MUTED;
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << die_player->GetGUID() << uint8(oldflags) << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::GiveModerator(Player* plr, Player* new_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(new_player);
    WorldPacket data(100);
    if(m_members.end() == itr)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(m_members.end() == itr2)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << new_player->GetGUID();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(itr->second & CHANNEL_FLAG_OWNER || itr->second & CHANNEL_FLAG_MODERATOR))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second |= CHANNEL_FLAG_MODERATOR;
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << new_player->GetGUID() << uint8(oldflags) << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::TakeModerator(Player* plr, Player* new_player)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    MemberMap::iterator itr2 = m_members.find(new_player);
    WorldPacket data(100);
    if(m_members.end() == itr)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(m_members.end() == itr2)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOT_ON_2);
        data << new_player->GetGUID();
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(itr->second & CHANNEL_FLAG_OWNER || itr->second & CHANNEL_FLAG_MODERATOR))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint32 oldflags = itr2->second;
    itr2->second &= ~CHANNEL_FLAG_MODERATOR;
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_MODE_CHG);
    data << new_player->GetGUID() << uint8(oldflags) << uint8(itr2->second);
    SendToAll(&data);
}

void Channel::Announce(Player* plr)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(100);
    if(m_members.end() == itr)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(itr->second & CHANNEL_FLAG_OWNER || itr->second & CHANNEL_FLAG_MODERATOR))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    m_announce = !m_announce;
    MakeNotifyPacket(&data, (m_announce ? CHANNEL_NOTIFY_FLAG_ENABLE_ANN : CHANNEL_NOTIFY_FLAG_DISABLE_ANN));
    data << plr->GetGUID();
    SendToAll(&data);
}

void Channel::Password(Player* plr, const char * pass)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(100);
    if(m_members.end() == itr)
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    if(!(itr->second & CHANNEL_FLAG_OWNER || itr->second & CHANNEL_FLAG_MODERATOR))
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTMOD);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    m_password = string(pass);
    MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_SETPASS);
    data << plr->GetGUID();
    SendToAll(&data);
}

void Channel::List(Player* plr)
{
    Guard mGuard(m_lock);
    WorldPacket data(SMSG_CHANNEL_LIST, 50 + (m_members.size()*9));
    MemberMap::iterator itr = m_members.find(plr);
    if(itr == m_members.end())
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint8 flags;
    data << uint8(1) << m_name;
    data << uint8(m_flags);
    data << uint32(m_members.size());
    for(MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); itr++)
    {
        data << itr->first->GetGUID();
        flags = 0;
        if(!(itr->second & CHANNEL_FLAG_MUTED))
            flags |= 0x04;      // voice flag

        if(itr->second & CHANNEL_FLAG_OWNER)
            flags |= 0x01;      // owner flag

        if(itr->second & CHANNEL_FLAG_MODERATOR)
            flags |= 0x02;      // moderator flag

        if(!m_general)
            flags |= 0x10;

        data << flags;
    }

    plr->GetSession()->SendPacket(&data);
}

void Channel::GetOwner(Player* plr)
{
    Guard mGuard(m_lock);
    MemberMap::iterator itr = m_members.find(plr);
    WorldPacket data(100);
    if(itr == m_members.end())
    {
        MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_NOTON);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    for(itr = m_members.begin(); itr != m_members.end(); itr++)
    {
        if(itr->second & CHANNEL_FLAG_OWNER)
        {
            MakeNotifyPacket(&data, CHANNEL_NOTIFY_FLAG_WHO_OWNER);
            data << itr->first->GetGUID();
            plr->GetSession()->SendPacket(&data);
            return;
        }
    }
}

void Channel::SendToAll(WorldPacket * data, Player* plr)
{
    Guard guard(m_lock);
    for(MemberMap::iterator itr = m_members.begin(); itr != m_members.end(); itr++)
    {
        if(plr != NULL && itr->first == plr)
            continue;

        if ( itr->first->GetSession() )
            itr->first->GetSession()->SendPacket(data);
    }
}

ChannelMgr::ChannelMgr()
{
    m_idHigh = 0;
}

ChannelMgr::~ChannelMgr()
{
    for(int i = 0; i < 2; i++)
    {
        ChannelList::iterator itr = Channels[i].begin();
        for(; itr != Channels[i].end(); itr++)
        {
            delete itr->second;
        }
        Channels[i].clear();
    }
}

Channel * ChannelMgr::GetCreateChannel(const char *name, Player* p)
{
    uint32 rTeam = seperatechannels && p != NULL ? p->GetTeam() : 0;
    ChannelList::iterator itr;
    ChannelList * cl = &Channels[rTeam];
    Channel * chn;
    uint32 cid;

    lock.Acquire();
    for(itr = cl->begin(); itr != cl->end(); itr++)
    {
        if(!stricmp(name, itr->first.c_str()))
        {
            lock.Release();
            return itr->second;
        }
    }

    // make sure the name isn't banned
    m_confSettingLock.Acquire();
    for(vector<string>::iterator itr = m_bannedChannels.begin(); itr != m_bannedChannels.end(); itr++)
    {
        if(!strnicmp( name, itr->c_str(), itr->size() ) )
        {
            lock.Release();
            m_confSettingLock.Release();
            return NULL;
        }
    }
    m_confSettingLock.Release();

    cid = ++m_idHigh;
    chn = new Channel(name, rTeam, 0, cid);
    cl->insert(make_pair(chn->m_name, chn));
    m_idToChannel.insert(make_pair(cid, chn));
    lock.Release();
    return chn;
}

Channel * ChannelMgr::GetCreateDBCChannel(const char *name, Player* p, uint32 type_id)
{
    uint32 rTeam = seperatechannels && p != NULL ? p->GetTeam() : 0;
    uint32 cid;
    Channel *chn;
    DBCChannelMap * dbcChannel = &DBCChannels[rTeam];
    DBCChannelBounds channelBounds = dbcChannel->equal_range(type_id);

    lock.Acquire();
    for(DBCChannelMap::iterator itr = channelBounds.first; itr != channelBounds.second; itr++)
    {
        if(!stricmp(name, itr->second.first.c_str()))
        {
            lock.Release();
            return itr->second.second;
        }
    }

    cid = ++m_idHigh;
    chn = new Channel(name, rTeam, type_id, cid);
    dbcChannel->insert(std::make_pair(type_id, std::make_pair(chn->m_name, chn)));
    m_idToChannel.insert(std::make_pair(cid, chn));
    lock.Release();
    return chn;
}

Channel * ChannelMgr::GetChannel(const char *name, Player* p, bool requiresIn)
{
    uint32 rteam = seperatechannels && p != NULL ? p->GetTeam() : 0;
    ChannelList::iterator itr;
    ChannelList * cl = &Channels[rteam];

    lock.Acquire();
    for(itr = cl->begin(); itr != cl->end(); itr++)
    {
        if(!stricmp(name, itr->first.c_str()))
        {
            if(requiresIn && !itr->second->HasMember(p))
                continue;

            lock.Release();
            return itr->second;
        }
    }

    DBCChannelMap::iterator itr2;
    DBCChannelMap *dbcchannel = &DBCChannels[rteam];
    for(itr2 = dbcchannel->begin(); itr2 != dbcchannel->end(); itr2++)
    {
        if(!stricmp(name, itr2->second.first.c_str()))
        {
            if(requiresIn && !itr2->second.second->HasMember(p))
                continue;

            lock.Release();
            return itr2->second.second;
        }
    }

    lock.Release();
    return NULL;
}

Channel * ChannelMgr::GetChannel(uint32 id)
{
    lock.Acquire();
    Channel *ret = NULL;
    ChannelMap::iterator itr = m_idToChannel.find(id);
    if( itr != m_idToChannel.end() )
        ret = itr->second;

    lock.Release();
    return ret;
}

Channel * ChannelMgr::GetChannel(const char *name, uint32 team)
{
    uint32 rteam = seperatechannels ? team : 0;

    lock.Acquire();
    ChannelList * cl = &Channels[rteam];
    for(ChannelList::iterator itr = cl->begin(); itr != cl->end(); itr++)
    {
        if(!stricmp(name, itr->first.c_str()))
        {
            lock.Release();
            return itr->second;
        }
    }

    DBCChannelMap::iterator itr2;
    DBCChannelMap *dbcchannel = &DBCChannels[rteam];
    for(itr2 = dbcchannel->begin(); itr2 != dbcchannel->end(); itr2++)
    {
        if(!stricmp(name, itr2->second.first.c_str()))
        {
            lock.Release();
            return itr2->second.second;
        }
    }

    lock.Release();
    return NULL;
}

void ChannelMgr::RemoveChannel(Channel * chn)
{
    ChannelList::iterator itr;
    ChannelList * cl = &Channels[chn->m_team];
    DBCChannelMap::iterator itr2;
    DBCChannelMap *dbcchannel = &DBCChannels[chn->m_team];

    lock.Acquire();
    m_idToChannel.erase(chn->m_channelId);
    for(itr = cl->begin(); itr != cl->end(); itr++)
    {
        if(itr->second == chn)
        {
            cl->erase(itr);
            chn->m_lock.Release();
            delete chn;
            lock.Release();
            return;
        }
    }

    for(itr2 = dbcchannel->begin(); itr2 != dbcchannel->end(); itr2++)
    {
        if(itr2->second.second == chn)
        {
            dbcchannel->erase(itr2);
            chn->m_lock.Release();
            delete chn;
            lock.Release();
            return;
        }
    }

    lock.Release();
}

void ChannelMgr::BroadcastToDBCChannels(uint32 dbc_id, Player* plr, const char * message)
{
    uint32 team = seperatechannels && plr != NULL ? plr->GetTeam() : 0;
    DBCChannelMap * dbcChannel = &DBCChannels[team];
    DBCChannelBounds channelBounds = dbcChannel->equal_range(dbc_id);
    lock.Acquire();
    for(DBCChannelMap::iterator itr = channelBounds.first; itr != channelBounds.second; itr++)
        itr->second.second->Say(plr, message, NULL, true);
    lock.Release();
}
