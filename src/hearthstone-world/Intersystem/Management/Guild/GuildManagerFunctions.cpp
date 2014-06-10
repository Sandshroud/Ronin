/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void SendGuildCommandResult(WorldSession* pClient, uint32 iCmd, const char* szMsg, uint32 iType)
{
    WorldPacket data(SMSG_GUILD_COMMAND_RESULT, (9 + strlen(szMsg)));
    data << iCmd << szMsg << iType;
    pClient->SendPacket(&data);
}

void SendTurnInPetitionResult(WorldSession* pClient, uint32 result)
{
    if(pClient == NULL)
        return;

    WorldPacket data(SMSG_TURN_IN_PETITION_RESULTS, 4);
    data << result;
    pClient->SendPacket(&data);
}


void GuildMgr::Packet_DisbandGuild(WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_QUIT_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_QUIT_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(gInfo->m_guildLeader != plr->GetLowGUID())
    {
        SendGuildCommandResult(m_session, GUILD_QUIT_S, "", GUILD_PERMISSIONS);
        return;
    }

    guildmgr.Disband(gInfo->m_guildId);
}

void GuildMgr::Packet_SetMotd(WorldSession* m_session, string motd)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_FOUNDER_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_FOUNDER_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(!HasGuildRights(plr, GR_RIGHT_SETMOTD))
    {
        SendGuildCommandResult(m_session, GUILD_FOUNDER_S, "", GUILD_PERMISSIONS);
        return;
    }

    gInfo->m_motd = motd;
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    SendMotd(NULL, gInfo->m_guildId);
}

void GuildMgr::Packet_GuildInviteAccept(WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if(plr == NULL)
        return;

    Player* inviter = objmgr.GetPlayer( plr->GetGuildInvitersGuid() );
    plr->UnSetGuildInvitersGuid();
    if(!inviter)
        return;

    uint32 GuildId = inviter->GetGuildId();
    GuildInfo *gInfo = GetGuildInfo(inviter->GetGuildId());
    if(gInfo == NULL)
        return;

    if(GetMemberCount(gInfo->m_guildId) >= MAX_GUILD_MEMBERS)
        return;

    AddGuildMember(gInfo, plr->getPlayerInfo(), inviter->GetSession());
}

void GuildMgr::Packet_GuildInviteDecline(WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if(plr == NULL)
        return;

    Player* inviter = objmgr.GetPlayer( plr->GetGuildInvitersGuid() );
    plr->UnSetGuildInvitersGuid();
    if(inviter == NULL)
        return;

    const char* PlrName = plr->GetName();
    WorldPacket data(SMSG_GUILD_DECLINE, strlen(PlrName));
    data << PlrName;
    inviter->SendPacket(&data);
}

void GuildMgr::Packet_HandleGuildInvite(WorldSession* m_session, std::string inviteeName)
{
    Player *thisplr = m_session->GetPlayer(), *thatplr = (objmgr.GetPlayer( inviteeName.c_str() , false));
    if(!thatplr)
    {
        SendGuildCommandResult(m_session, GUILD_INVITE_S, inviteeName.c_str(), GUILD_PLAYER_NOT_FOUND);
        return;
    }
    else if(!thisplr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_INVITE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(thisplr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_INVITE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if( thatplr->IsInGuild() )
    {
        SendGuildCommandResult(m_session, GUILD_INVITE_S, inviteeName.c_str(), ALREADY_IN_GUILD);
        return;
    }
    else if(thatplr->GetGuildInvitersGuid())
    {
        SendGuildCommandResult(m_session, GUILD_INVITE_S, inviteeName.c_str(), ALREADY_INVITED_TO_GUILD);
        return;
    }
    else if(!HasGuildRights(thisplr, GR_RIGHT_INVITE))
    {
        SendGuildCommandResult(m_session, GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        return;
    }
    else if(thatplr->GetTeam() != thisplr->GetTeam() && thisplr->GetSession()->GetPermissionCount() == 0 && !sWorld.cross_faction_world)
    {
        SendGuildCommandResult(m_session, GUILD_INVITE_S, "", GUILD_NOT_ALLIED);
        return;
    }
    else if(GetMemberCount(thisplr->GetGuildId()) >= MAX_GUILD_MEMBERS)
    {
        // We can't handle >= 500 members, or WoW will #132. I don't have the proper error code, so just throw the internal one.
        SendGuildCommandResult(m_session, GUILD_INVITE_S, "", GUILD_INTERNAL);
        return;
    }

    SendGuildCommandResult(m_session, GUILD_INVITE_S, inviteeName.c_str(), GUILD_U_HAVE_INVITED);

    WorldPacket data(SMSG_GUILD_INVITE, 100);
    data << thisplr->GetName();
    data << gInfo->m_guildName.c_str();
    thatplr->GetSession()->SendPacket(&data);

    thatplr->SetGuildInvitersGuid( thisplr->GetLowGUID() );
}

void GuildMgr::Packet_SendGuildLog(WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
        return;

    GuildLogStorage* LogStorage = GetGuildLogStorage(plr->GetGuildId());
    if(LogStorage == NULL)
        return;

    WorldPacket data(MSG_GUILD_EVENT_LOG_QUERY, 18 * LogStorage->m_logs.size() + 1);
    GuildLogList::iterator itr;
    uint32 count = 0;

    data << uint8(LogStorage->m_logs.size() >= 25 ? 25 : LogStorage->m_logs.size());
    LogStorage->Locks.Acquire();
    for(itr = LogStorage->m_logs.begin(); itr != LogStorage->m_logs.end(); ++itr)
    {
        data << uint8((*itr)->iEvent);
        switch((*itr)->iEvent)
        {
        case GUILD_LOG_EVENT_DEMOTION:
        case GUILD_LOG_EVENT_PROMOTION:
            {
                data << uint64((*itr)->iEventData[0]);
                data << uint64((*itr)->iEventData[1]);
                data << uint8((*itr)->iEventData[2]);
            }break;
        case GUILD_LOG_EVENT_INVITE:
        case GUILD_LOG_EVENT_REMOVAL:
            {
                data << uint64((*itr)->iEventData[0]);
                data << uint64((*itr)->iEventData[1]);
            }break;
        case GUILD_LOG_EVENT_JOIN:
        case GUILD_LOG_EVENT_LEFT:
            {
                data << uint64((*itr)->iEventData[0]);
            }break;
        }

        data << uint32(UNIXTIME - (*itr)->iTimeStamp);
        if((++count) >= 25)
            break;
    }
    LogStorage->Locks.Release();

    m_session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildRoster(WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
        return;

    GuildMemberMapStorage* MemberMapStorage = GetGuildMemberMapStorage(plr->GetGuildId());
    if(MemberMapStorage == NULL)
        return;

    GuildRankStorage* RankStorage = GetGuildRankStorage(plr->GetGuildId());
    if(RankStorage == NULL)
        return;

    WorldPacket data(SMSG_GUILD_ROSTER, (60 * 10) + (100 * MemberMapStorage->MemberMap.size()) + 100);
    RankStorage->RankLock.Acquire();
    data << uint32(MemberMapStorage->MemberMap.size());
    if(gInfo->m_motd.length())
        data << gInfo->m_motd.c_str();
    else
        data << uint8(0);

    if(gInfo->m_guildInfo.length())
        data << gInfo->m_guildInfo.c_str();
    else
        data << uint8(0);

    uint32 count = 0;
    GuildRank* r = NULL;
    size_t pos = data.wpos();
    data << uint32(0);
    for(uint8 i = 0; i < MAX_GUILD_RANKS; ++i)
    {
        r = RankStorage->m_ranks[i];
        if(r != NULL)
        {
            data << r->iRights;
            data << r->iGoldLimitPerDay;
            for (uint8 j = 0; j < 6; j++)
            {
                data << r->iTabPermissions[j].iFlags;
                data << r->iTabPermissions[j].iStacksPerDay;
            }

            ++count;
        }
    }
    RankStorage->RankLock.Release();
    data.put<uint32>(pos, count);

    PlayerInfo* pPlayer = NULL;
    MemberMapStorage->MemberMapLock.Acquire();
    for(GuildMemberMap::iterator itr = MemberMapStorage->MemberMap.begin(); itr != MemberMapStorage->MemberMap.end(); ++itr)
    {
        pPlayer = itr->second->pPlayer;

        data << itr->first;
        data << uint32(0);          // highguid
        data << uint8((pPlayer->m_loggedInPlayer != NULL) ? 1 : 0);
        data << pPlayer->name;
        data << pPlayer->GuildRank;
        data << uint8(pPlayer->lastLevel);
        data << uint8(pPlayer->_class);
        data << uint8(pPlayer->gender);
        data << pPlayer->lastZone;

        if(!pPlayer->m_loggedInPlayer)
            data << float((UNIXTIME - pPlayer->lastOnline) / 86400.0);

        if(itr->second->szPublicNote.length())
            data << itr->second->szPublicNote.c_str();
        else
            data << uint8(0);

        if(HasGuildRights(plr, GR_RIGHT_VIEWOFFNOTE) && itr->second->szOfficerNote.length())
            data << itr->second->szOfficerNote.c_str();
        else
            data << uint8(0);
    }
    MemberMapStorage->MemberMapLock.Release();
    m_session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildInformation(WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
        return;

    uint32 GuildSize = GetMemberCount(plr->GetGuildId());
    time_t ct = (time_t)gInfo->m_creationTimeStamp;
    tm* pTM = localtime(&ct);

    WorldPacket data(SMSG_GUILD_INFO, 4);
    data << gInfo->m_guildName.c_str();
    data << uint32(pTM->tm_year + 1900);
    data << uint32(pTM->tm_mon);
    data << uint32(pTM->tm_mday);
    data << uint32(GuildSize);
    data << uint32(GuildSize);
    m_session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildPermissions(WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    GuildMember* gMember = GetGuildMember(plr->GetLowGUID());
    if(gMember == NULL || gMember->pRank == NULL)
        return;

    WorldPacket data(MSG_GUILD_PERMISSIONS, 61);
    data << gMember->pRank->iId;
    data << gMember->pRank->iRights;
    data << gMember->pRank->iGoldLimitPerDay;
    data << uint8(GetGuildBankTabCount(plr->GetGuildId()));
    for( uint8 i = 0; i < MAX_GUILD_BANK_TABS; i++)
    {
        data << gMember->pRank->iTabPermissions[i].iFlags;
        data << gMember->pRank->iTabPermissions[i].iStacksPerDay;
    }

    m_session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildQuery(WorldSession* m_session, uint32 GuildId)
{
    GuildInfo* gInfo = GetGuildInfo(GuildId);
    if(gInfo == NULL)
        return;

    GuildRankStorage* RankStorage = GetGuildRankStorage(GuildId);
    if(RankStorage == NULL)
        return;

    uint32 i = 0;
    GuildRank* r = NULL;
    WorldPacket data(SMSG_GUILD_QUERY_RESPONSE, 300);
    data << gInfo->m_guildId;
    data << gInfo->m_guildName.c_str();

    RankStorage->RankLock.Acquire();
    for(i = 0; i < MAX_GUILD_RANKS; ++i)
    {
        r = RankStorage->m_ranks[i];
        if(r != NULL)
            data << r->szRankName.c_str();
        else
            data << uint8(0);
    }

    data << gInfo->m_emblemStyle;
    data << gInfo->m_emblemColor;
    data << gInfo->m_borderStyle;
    data << gInfo->m_borderColor;
    data << gInfo->m_backgroundColor;

    if(m_session != NULL)
        m_session->SendPacket(&data);
    else
    {
        GuildMemberMapStorage* MemberListStorage = GetGuildMemberMapStorage(GuildId);
        if(MemberListStorage != NULL)
        {
            for(GuildMemberMap::iterator itr = MemberListStorage->MemberMap.begin(); itr != MemberListStorage->MemberMap.end(); ++itr)
            {
                WorldPacket* packet2 = new WorldPacket(data);
                if(itr->second->pPlayer->m_loggedInPlayer)
                    itr->second->pPlayer->m_loggedInPlayer->SendPacket(packet2);
            }
        }
    }

    RankStorage->RankLock.Release();
}

void GuildMgr::Packet_SetPublicNote(WorldSession* m_session, std::string playername, std::string note)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    if(!HasGuildRights(plr, GR_RIGHT_EPNOTE))
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PERMISSIONS);
        return;
    }

    PlayerInfo* pInfo = objmgr.GetPlayerInfoByName(playername.c_str());
    if(pInfo == NULL)
        return;

    if(plr->GetGuildId() != pInfo->GuildId)
        return;

    SetNote(pInfo, note, false);

    SendGuildCommandResult(m_session, GUILD_PUBLIC_NOTE_CHANGED_S, playername.c_str(), 0);
}

void GuildMgr::Packet_SetOfficerNote(WorldSession* m_session, std::string playername, std::string note)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    if(!HasGuildRights(plr, GR_RIGHT_EOFFNOTE))
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PERMISSIONS);
        return;
    }

    PlayerInfo* pInfo = objmgr.GetPlayerInfoByName(playername.c_str());
    if(pInfo == NULL)
        return;

    if(plr->GetGuildId() != pInfo->GuildId)
        return;

    SetNote(pInfo, note, true);

    SendGuildCommandResult(m_session, GUILD_PUBLIC_NOTE_CHANGED_S, playername.c_str(), 0);
}

void GuildMgr::Packet_SaveGuildEmblem(WorldSession* m_session, uint32 emblemStyle, uint32 emblemColor, uint32 borderStyle, uint32 borderColor, uint32 backgroundColor)
{
    WorldPacket data(MSG_SAVE_GUILD_EMBLEM, 4);
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        data << uint32(ERR_GUILDEMBLEM_NOGUILD);
        m_session->SendPacket(&data);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo->m_guildLeader != plr->GetLowGUID())
    {
        data << uint32(ERR_GUILDEMBLEM_NOTGUILDMASTER);
        m_session->SendPacket(&data);
        return;
    }

    uint32 cost = MONEY_ONE_GOLD * 10;
    if(plr->GetUInt32Value(PLAYER_FIELD_COINAGE) < cost)
    {
        data << uint32(ERR_GUILDEMBLEM_NOTENOUGHMONEY);
        m_session->SendPacket(&data);
        return;
    }

    plr->ModUnsigned32Value(PLAYER_FIELD_COINAGE, (-(int32(cost))));

    gInfo->m_GuildLock.Acquire();
    gInfo->m_emblemStyle = emblemStyle;
    gInfo->m_emblemColor = emblemColor;
    gInfo->m_borderStyle = borderStyle;
    gInfo->m_borderColor = borderColor;
    gInfo->m_backgroundColor = backgroundColor;
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();
    Packet_SendGuildQuery(NULL, plr->GetGuildId());
}

void GuildMgr::Packet_HandleDeleteRank(WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo->m_guildLeader != plr->GetLowGUID())
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_PERMISSIONS);
        return;
    }

    uint32 error = RemoveGuildRank(plr->GetGuildId());
    if(error)
    {
        switch(error)
        {
        case 1:
        case 2:
            {
                m_session->SystemMessage("Cannot find a rank to delete.");
            }break;
        case 3:
            {
                m_session->SystemMessage("There are still members using this rank. You cannot delete it yet!");
            }break;
        }
        return;
    }

    // there is probably a command result for this. need to find it.
    Packet_SendGuildQuery(NULL, plr->GetGuildId());
    Packet_SendGuildRoster(m_session);
}

void GuildMgr::Packet_HandleAddRank(WorldSession* m_session, std::string name)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo->m_guildLeader != plr->GetLowGUID())
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_PERMISSIONS);
        return;
    }

    GuildRankStorage* RankStorage = GetGuildRankStorage(plr->GetGuildId());
    if(RankStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    CreateGuildRank(RankStorage, name.c_str(), GR_RIGHT_DEFAULT, false);

    // there is probably a command result for this. need to find it.
    Packet_SendGuildQuery(NULL, plr->GetGuildId());
    Packet_SendGuildRoster(m_session);
}

void GuildMgr::Packet_HandleEditRank(WorldSession* m_session, std::string name, uint32 rankid, uint32 rankrights, int32 DailyGoldLimit, uint32* iflags, int32* istacksperday)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo->m_guildLeader != plr->GetLowGUID())
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_PERMISSIONS);
        return;
    }

    GuildRankStorage* RankStorage = GetGuildRankStorage(plr->GetGuildId());
    if(RankStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    RankStorage->RankLock.Acquire();
    if(rankid > MAX_GUILD_RANKS || RankStorage->m_ranks[rankid] == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        RankStorage->RankLock.Release();
        return;
    }

    GuildRank* rank = RankStorage->m_ranks[rankid];
    if(name.length())
        rank->szRankName = name;
    rank->iRights = rankrights;
    rank->iGoldLimitPerDay = DailyGoldLimit;
    for(uint8 i = 0; i < MAX_GUILD_BANK_TABS; i++)
    {
        rank->iTabPermissions[i].iFlags = iflags[i];
        rank->iTabPermissions[i].iStacksPerDay = istacksperday[i];
    }
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    RankStorage->RankLock.Release();
    Packet_SendGuildRoster(m_session);
}

void GuildMgr::Packet_ChangeGuildLeader(WorldSession* m_session, PlayerInfo* newLeader)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild() || plr->GetGuildId() != newLeader->GuildId)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo->m_guildLeader != plr->GetLowGUID())
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_PERMISSIONS);
        return;
    }

    GuildMember *GuildLeader = GetGuildMember(plr->GetLowGUID()), *NewGuildLeader = GetGuildMember(newLeader->guid);
    if(GuildLeader == NULL || NewGuildLeader == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildRankStorage* RankStorage = GetGuildRankStorage(plr->GetGuildId());
    if(RankStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    gInfo->m_GuildLock.Acquire();
    GuildRank* newRank = FindLowestRank(RankStorage);
    GuildLeader->pRank = newRank;
    plr->SetGuildRank(newRank->iId);
    plr->getPlayerInfo()->GuildRank = newRank->iId;

    newLeader->GuildRank = 0;
    gInfo->m_guildLeader = newLeader->guid;
    NewGuildLeader->pRank = RankStorage->m_ranks[0];
    if(newLeader->m_loggedInPlayer)
        newLeader->m_loggedInPlayer->SetGuildRank(0);
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();

    LogGuildEvent(NULL, plr->GetGuildId(), GUILD_EVENT_LEADER_CHANGED, plr->GetName(), newLeader->name);
}

void GuildMgr::Packet_DemoteGuildMember(WorldSession* m_session, std::string demoteeName)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildRankStorage* RankStorage = GetGuildRankStorage(plr->GetGuildId());
    if(RankStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    PlayerInfo* Demoted = objmgr.GetPlayerInfoByName(demoteeName.c_str());
    if(Demoted == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_FOUND);
        return;
    }

    if(plr->GetGuildId() != Demoted->GuildId)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, demoteeName.c_str(), GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    GuildMember* gMember = GetGuildMember(plr->GetLowGUID()), *DemotedMember = GetGuildMember(Demoted->guid);
    if(gMember == NULL || DemotedMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, demoteeName.c_str(), GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    if(DemotedMember->pRank->iId == MAX_GUILD_RANKS-1)
    {
        m_session->SystemMessage("Could not find a rank to demote this member to.");
        return;
    }

    RankStorage->RankLock.Acquire();
    if(!HasGuildRights(plr, GR_RIGHT_DEMOTE) || DemotedMember->pRank->iId == 0
        || gMember->pRank->iId >= DemotedMember->pRank->iId)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PERMISSIONS);
        RankStorage->RankLock.Release();
        return;
    }

    // find the next highest rank
    uint32 nh = DemotedMember->pRank->iId + 1;
    GuildRank* newRank = RankStorage->m_ranks[nh];
    while(nh < 10 && newRank == NULL)
    {
        newRank = RankStorage->m_ranks[nh];
        if(newRank == NULL)
            ++nh;
    }
    RankStorage->RankLock.Release();

    if(newRank == NULL)
    {
        m_session->SystemMessage("Could not find a rank to demote this member to.");
        return;
    }

    gInfo->m_GuildLock.Acquire();
    DemotedMember->pRank = newRank;
    DemotedMember->pPlayer->GuildRank = nh;

    // log it
    LogGuildEvent(NULL, plr->GetGuildId(), GUILD_EVENT_DEMOTION, plr->GetName(), demoteeName.c_str(), newRank->szRankName.c_str());
    AddGuildLogEntry(plr->GetGuildId(), GUILD_LOG_EVENT_DEMOTION, plr->GetLowGUID(), Demoted->guid, newRank->iId);

    // if the player is online, update his guildrank
    if(Demoted->m_loggedInPlayer)
        Demoted->m_loggedInPlayer->SetGuildRank(nh);
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();
}

void GuildMgr::Packet_PromoteGuildMember(WorldSession* m_session, std::string promoteeName)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildRankStorage* RankStorage = GetGuildRankStorage(plr->GetGuildId());
    if(RankStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    PlayerInfo* Promoted = objmgr.GetPlayerInfoByName(promoteeName.c_str());
    if(Promoted == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_FOUND);
        return;
    }

    if(plr->GetGuildId() != Promoted->GuildId)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, string(string("1") + promoteeName).c_str(), GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    GuildMember* gMember = GetGuildMember(plr->GetLowGUID()), *PromotedMember = GetGuildMember(Promoted->guid);
    if(gMember == NULL || PromotedMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, string(string("2 ") + promoteeName).c_str(), GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    RankStorage->RankLock.Acquire();
    if(!HasGuildRights(plr, GR_RIGHT_PROMOTE) || PromotedMember->pRank->iId == 0
        || gMember->pRank->iId >= PromotedMember->pRank->iId)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PERMISSIONS);
        RankStorage->RankLock.Release();
        return;
    }

    // find the lowest rank that isn't his rank
    int32 nh = PromotedMember->pRank->iId - 1;
    GuildRank* newRank = RankStorage->m_ranks[nh];

    while(nh > 0 && newRank == NULL)
    {
        newRank = RankStorage->m_ranks[nh];
        nh--;
    }
    RankStorage->RankLock.Release();

    if(nh == 0 || newRank == NULL)
    {
        m_session->SystemMessage("Could not find a rank to promote this member to.");
        return;
    }


    gInfo->m_GuildLock.Acquire();
    PromotedMember->pRank = newRank;
    PromotedMember->pPlayer->GuildRank = nh;

    // log it
    LogGuildEvent(NULL, plr->GetGuildId(), GUILD_EVENT_PROMOTION, plr->GetName(), promoteeName.c_str(), newRank->szRankName.c_str());
    AddGuildLogEntry(plr->GetGuildId(), GUILD_LOG_EVENT_PROMOTION, plr->GetLowGUID(), Promoted->guid, newRank->iId);

    // if the player is online, update his guildrank
    if(Promoted->m_loggedInPlayer)
        Promoted->m_loggedInPlayer->SetGuildRank(nh);
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();
}

// Charter Buy
void GuildMgr::CharterBuy(WorldSession* m_session, uint64 SellerGuid, std::string name, uint32 petitionCount, uint32 ArenaIndex)
{
    uint8 error = 0;
    Creature* crt = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(SellerGuid));
    if(!crt)
    {
        m_session->Disconnect();
        return;
    }

    if( ArenaIndex >= NUM_CHARTER_TYPES )
        return;

    //All arena organizers should be allowed to create arena charter's
    if( !crt->ArenaOrganizersFlags() )
    {
        uint32 arena_type = ArenaIndex - 1;
        if(arena_type > 2)
            return;

        if(m_session->GetPlayer()->m_playerInfo->arenaTeam[arena_type])
        {
            m_session->SendNotification("You are already in an arena team.");
            return;
        }

        if(m_session->GetPlayer()->m_playerInfo->charterId[ArenaIndex] != 0)
        {
            m_session->SendNotification("You already have an arena charter of this type.");
            return;
        }

        if(!sWorld.VerifyName(name.c_str(), name.length()))
        {
            m_session->SendNotification("That name is invalid or contains invalid characters.");
            return;
        }

        ArenaTeam * t = objmgr.GetArenaTeamByName(name, arena_type);
        if(t != NULL)
        {
            sChatHandler.SystemMessage(m_session,"That name is already in use.");
            return;
        }

        if(guildmgr.GetCharterByName(name, (CharterTypes)ArenaIndex))
        {
            sChatHandler.SystemMessage(m_session, "That name is already in use.");
            return;
        }

        static uint32 item_ids[] = {ARENA_TEAM_CHARTER_2v2, ARENA_TEAM_CHARTER_3v3, ARENA_TEAM_CHARTER_5v5};
        static uint32 costs[] = {ARENA_TEAM_CHARTER_2v2_COST,ARENA_TEAM_CHARTER_3v3_COST,ARENA_TEAM_CHARTER_5v5_COST};

        if(m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE) < costs[arena_type])
        {
            sChatHandler.SystemMessage(m_session, "You don't have enough money!");
            return;         // error message needed here
        }

        ItemPrototype * ip = ItemPrototypeStorage.LookupEntry(item_ids[arena_type]);
        ASSERT(ip);
        SlotResult res = m_session->GetPlayer()->GetItemInterface()->FindFreeInventorySlot(ip);
        if(res.Result == 0)
        {
            m_session->GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_INVENTORY_FULL);
            return;
        }

        error = m_session->GetPlayer()->GetItemInterface()->CanReceiveItem(ip,1, NULL);
        if(error)
        {
            m_session->GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM,error);
        }
        else
        {
            // Create the item and charter
            Item* i = objmgr.CreateItem(item_ids[arena_type], m_session->GetPlayer());
            Charter * c = guildmgr.CreateCharter(m_session->GetPlayer()->GetLowGUID(), (CharterTypes)ArenaIndex);
            c->GuildName = name;
            c->ItemGuid = i->GetLowGUID();

            i->Bind(ITEM_BIND_ON_PICKUP);
            i->SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);
            i->SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1, c->GetID());
            i->SetUInt32Value(ITEM_FIELD_PROPERTY_SEED, 57813883);
            if( !m_session->GetPlayer()->GetItemInterface()->AddItemToFreeSlot(i) )
            {
                c->Destroy();
                c = NULL;
                i->DeleteMe();
                i = NULL;
                return;
            }

            c->SaveToDB();
            m_session->SendItemPushResult(i, false, true, false, true, m_session->GetPlayer()->GetItemInterface()->LastSearchItemBagSlot(), m_session->GetPlayer()->GetItemInterface()->LastSearchItemSlot(), 1);
            m_session->GetPlayer()->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -(int32)costs[arena_type]);
            m_session->GetPlayer()->m_playerInfo->charterId[ArenaIndex] = c->GetID();
            m_session->GetPlayer()->SaveToDB(false);
        }
    }
    else
    {
        if( m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE) < 1000)
        {
            m_session->SendNotification("You don't have enough money.");
            return;
        }

        if(m_session->GetPlayer()->m_playerInfo->charterId[CHARTER_TYPE_GUILD] != 0)
        {
            m_session->SendNotification("You already have a guild charter.");
            return;
        }

        if(!sWorld.VerifyName(name.c_str(), name.length()))
        {
            m_session->SendNotification("That name is invalid or contains invalid characters.");
            return;
        }

        GuildInfo * g = guildmgr.GetGuildByGuildName(name);
        Charter * c = guildmgr.GetCharterByName(name, CHARTER_TYPE_GUILD);
        if(g != 0 || c != 0)
        {
            m_session->SendNotification("A guild with that name already exists.");
            return;
        }

        ItemPrototype * ip = ItemPrototypeStorage.LookupEntry(ITEM_ENTRY_GUILD_CHARTER);
        assert(ip);
        SlotResult res = m_session->GetPlayer()->GetItemInterface()->FindFreeInventorySlot(ip);
        if(res.Result == 0)
        {
            m_session->GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_INVENTORY_FULL);
            return;
        }

        error = m_session->GetPlayer()->GetItemInterface()->CanReceiveItem(ItemPrototypeStorage.LookupEntry(ITEM_ENTRY_GUILD_CHARTER),1, NULL);
        if(error)
            m_session->GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM,error);
        else
        {
            // Meh...
            WorldPacket data(SMSG_PLAY_OBJECT_SOUND, 12);
            data << uint32(0x000019C2);
            data << SellerGuid;
            m_session->SendPacket(&data);

            // Create the item and charter
            Item * i = objmgr.CreateItem(ITEM_ENTRY_GUILD_CHARTER, m_session->GetPlayer());
            c = guildmgr.CreateCharter(m_session->GetPlayer()->GetLowGUID(), CHARTER_TYPE_GUILD);
            c->GuildName = name;
            c->ItemGuid = i->GetLowGUID();

            i->Bind(ITEM_BIND_ON_PICKUP);
            i->SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);
            i->SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1, c->GetID());
            i->SetUInt32Value(ITEM_FIELD_PROPERTY_SEED, 57813883);
            if( !m_session->GetPlayer()->GetItemInterface()->AddItemToFreeSlot(i) )
            {
                c->Destroy();
                c = NULL;
                i->DeleteMe();
                i = NULL;
                return;
            }

            c->SaveToDB();

            m_session->SendItemPushResult(i, false, true, false, true, m_session->GetPlayer()->GetItemInterface()->LastSearchItemBagSlot(), m_session->GetPlayer()->GetItemInterface()->LastSearchItemSlot(), 1);

            m_session->GetPlayer()->m_playerInfo->charterId[CHARTER_TYPE_GUILD] = c->GetID();

            // 10 silver
            m_session->GetPlayer()->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -1000);
            m_session->GetPlayer()->SaveToDB(false);
        }
    }
}

void GuildMgr::Packet_SetGuildInformation(WorldSession* m_session, std::string guildInfo)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    gInfo->m_GuildLock.Acquire();
    if(!HasGuildRights(plr, GR_RIGHT_EGUILDINFO))
    {
        SendGuildCommandResult(m_session, GUILD_INVITE_S, "", GUILD_PERMISSIONS);
        gInfo->m_GuildLock.Release();
        return;
    }

    if(guildInfo.length())
        gInfo->m_guildInfo = strdup(guildInfo.c_str());
    else
        gInfo->m_guildInfo = "";
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();
}

void GuildMgr::Packet_SendGuildBankLog(WorldSession* m_session, uint32 slotid)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(plr->GetGuildId());
    if(BankTabStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    uint8 count = 0;
    gInfo->m_GuildLock.Acquire();
    WorldPacket data(MSG_GUILD_BANK_LOG_QUERY);
    if(slotid == 6)
    {
        // sending the money log
        data.resize((17*BankTabStorage->m_money_logs.size()) + 2);
        uint32 lt = (uint32)UNIXTIME;
        data << uint8(0x06);
        data << uint8((BankTabStorage->m_money_logs.size() < 25) ? BankTabStorage->m_money_logs.size() : 25);
        list<GuildBankEvent*>::iterator itr = BankTabStorage->m_money_logs.begin();
        for(; itr != BankTabStorage->m_money_logs.end(); itr++)
        {
            data << (*itr)->iAction;
            data << (*itr)->uPlayer;
            data << uint32(0);      // highguid
            data << (*itr)->uEntry;
            data << uint32(lt - (*itr)->uTimeStamp);

            if( (++count) >= 25 )
                break;
        }
    }
    else
    {
        for(uint8 i = 0; i < 6; i++)
        {
            if(BankTabStorage->m_TabLogs[i] != NULL)
                count++;
        }

        if(slotid > count || slotid > MAX_GUILD_BANK_TABS)
        {
            gInfo->m_GuildLock.Release();
            return;
        }

        BankLogInternalStorage* InternalLog = BankTabStorage->m_TabLogs[slotid];
        if(InternalLog == NULL)
        {
            gInfo->m_GuildLock.Release();
            return;
        }

        uint32 lt = (uint32)UNIXTIME;
        data.resize(InternalLog->m_logs.size() * (4 * 4 + 2) + 1 + 1);
        data << uint8(slotid);
        data << uint8(InternalLog->m_logs.size());
        for (std::list<GuildBankEvent*>::iterator itr = InternalLog->m_logs.begin(); itr != InternalLog->m_logs.end(); ++itr)
        {
            data << uint8((*itr)->iAction);
            data << uint32((*itr)->uPlayer);
            data << uint32(0);
            data << uint32((*itr)->uEntry);
            data << uint8((*itr)->iStack);
            data << uint32(lt - (*itr)->uTimeStamp);
        }
    }

    m_session->SendPacket(&data);
    sLog.outDebug("WORLD: Sent (MSG_GUILD_BANK_LOG_QUERY)");
    gInfo->m_GuildLock.Release();
}

void GuildMgr::Packet_SendGuildBankText(WorldSession* m_session, uint8 TabSlot)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(plr->GetGuildId());
    if(BankTabStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    gInfo->m_GuildLock.Acquire();
    GuildBankTab* Tab = BankTabStorage->m_Tabs[TabSlot];
    if(Tab == NULL)
    {
        gInfo->m_GuildLock.Release();
        return;
    }

    WorldPacket data( MSG_QUERY_GUILD_BANK_TEXT, 1 + Tab->szTabInfo.length() );
    data << TabSlot;
    if( Tab->szTabInfo.size() )
        data << Tab->szTabInfo.c_str();
    else
        data << uint8(0);

    m_session->SendPacket(&data);
    gInfo->m_GuildLock.Release();
}

void GuildMgr::Packet_SetGuildBankText(WorldSession* m_session, uint32 tabid, std::string tabtext)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildMember* gMember = GetGuildMember(plr->GetLowGUID());
    if(gMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    if(!HasGuildBankRights(plr, tabid, GR_RIGHT_GUILD_BANK_VIEW_TAB))
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PERMISSIONS);
        return;
    }

    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(plr->GetGuildId());
    if(BankTabStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    gInfo->m_GuildLock.Acquire();
    BankTabStorage->m_Tabs[tabid]->szTabInfo = tabtext.c_str();
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();

    WorldPacket data(SMSG_GUILD_EVENT, 4);
    data << uint8(GUILD_EVENT_TABINFO);
    data << uint8(1);
    data << uint16(0x30 + tabid);
    m_session->SendPacket(&data);
}

void GuildMgr::Packet_WithdrawItem(WorldSession* m_session, uint8 dest_bank, uint8 dest_bankslot, uint8 source_bank, uint8 source_bankslot, uint32 itementry, uint8 autostore, int32 splitted_count)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildMember* gMember = GetGuildMember(plr->GetLowGUID());
    if(gMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    /* sanity checks to avoid overflows */
    if(source_bankslot >= MAX_GUILD_BANK_SLOTS ||
        dest_bankslot >= MAX_GUILD_BANK_SLOTS ||
        source_bank >= MAX_GUILD_BANK_TABS ||
        dest_bank >= MAX_GUILD_BANK_TABS)
    {
        return;
    }

    /* make sure we have permissions */
    if( !HasGuildBankRights(plr, dest_bank, GR_RIGHT_GUILD_BANK_DEPOSIT_ITEMS)
        || !HasGuildBankRights(plr, source_bank, GR_RIGHT_GUILD_BANK_DEPOSIT_ITEMS) )
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PERMISSIONS);
        return;
    }

    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(plr->GetGuildId());
    if(BankTabStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    gInfo->m_GuildLock.Acquire();
    /* locate the tabs */
    GuildBankTab *pSourceTab = BankTabStorage->m_Tabs[source_bank], *pDestTab = BankTabStorage->m_Tabs[dest_bank];
    if(pSourceTab == NULL || pDestTab == NULL)
    {
        gInfo->m_GuildLock.Release();
        return;
    }

    if(pSourceTab == pDestTab && source_bankslot == dest_bankslot)
    {
        gInfo->m_GuildLock.Release();
        return;
    }

    Item *pSourceItem = pSourceTab->pSlots[source_bankslot], *pDestItem = pDestTab->pSlots[dest_bankslot];
    if(pSourceItem == NULL && pDestItem == NULL)
    {
        gInfo->m_GuildLock.Release();
        return;
    }

    if(splitted_count > 0)
    {
        uint32 source_count = pSourceItem->GetUInt32Value( ITEM_FIELD_STACK_COUNT );
        if(pDestItem == NULL)
        {
            if(source_count == (uint32)splitted_count)
            {
                // swap
                pSourceTab->pSlots[source_bankslot] = pDestItem;
                pDestTab->pSlots[dest_bankslot] = pSourceItem;
            }
            else
            {
                pSourceItem->ModUnsigned32Value( ITEM_FIELD_STACK_COUNT, -splitted_count );

                pDestItem = objmgr.CreateItem(pSourceItem->GetEntry(), NULL);
                pDestItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT, splitted_count);
                pDestItem->SetUInt32Value(ITEM_FIELD_CREATOR, pSourceItem->GetUInt32Value(ITEM_FIELD_CREATOR));
                pDestTab->pSlots[dest_bankslot] = pDestItem;
            }
        }
        else
        {
            pDestItem->ModUnsigned32Value( ITEM_FIELD_STACK_COUNT, splitted_count );

            if((uint32)splitted_count != source_count)
                pSourceItem->ModUnsigned32Value( ITEM_FIELD_STACK_COUNT, -splitted_count );
            else
            {
                pSourceItem->DeleteMe();
                pSourceItem = NULLITEM;
                pSourceTab->pSlots[source_bankslot] = NULLITEM;
            }
        }
    }
    else
    {
        // swap
        pSourceTab->pSlots[source_bankslot] = pDestItem;
        pDestTab->pSlots[dest_bankslot] = pSourceItem;
    }

    /* update the client */
    if(pSourceTab == pDestTab)
    {
        /* send both slots in the packet */
        Packet_SendGuildBankTab(m_session, source_bank, source_bankslot, dest_bankslot);
    }
    else
    {
        /* send a packet for each different bag */
        Packet_SendGuildBankTab(m_session, source_bank, source_bankslot, -1);
        Packet_SendGuildBankTab(m_session, dest_bank, dest_bankslot, -1);
    }

    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();
}

void GuildMgr::Packet_DepositItem(WorldSession* m_session, uint8 dest_bank, uint8 dest_bankslot, uint32 itementry, uint8 autostore, uint8 withdraw_stack, int8 source_bagslot, int8 source_slot, uint8 tochar, uint8 deposit_stack)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildMember* gMember = GetGuildMember(plr->GetLowGUID());
    if(gMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    /* sanity checks to avoid overflows */
    if(dest_bank >= MAX_GUILD_BANK_TABS)
        return;

    /* make sure we have permissions */
    if(!HasGuildBankRights(plr, dest_bank, GR_RIGHT_GUILD_BANK_DEPOSIT_ITEMS))
        return;

    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(plr->GetGuildId());
    if(BankTabStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    gInfo->m_GuildLock.Acquire();
    /* locate the tabs */
    GuildBankTab *pDestTab = BankTabStorage->m_Tabs[dest_bank];
    if(pDestTab == NULL)
    {
        gInfo->m_GuildLock.Release();
        return;
    }

    /* check if we are auto assigning */
    if(dest_bankslot == 0xff)
    {
        for(uint8 i = 0; i < MAX_GUILD_BANK_SLOTS; i++)
        {
            if(pDestTab->pSlots[i] == NULL)
            {
                dest_bankslot = (uint8)i;
                break;
            }
        }

        if(dest_bankslot == 0xff)
        {
            plr->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_BAG_FULL);
            gInfo->m_GuildLock.Release();
            return;
        }
    }

    /* another check here */
    if(dest_bankslot >= MAX_GUILD_BANK_SLOTS)
    {
        gInfo->m_GuildLock.Release();
        return;
    }

    /* check if we're pulling an item from the bank, make sure we're not cheating. */
    Item *pDestItem = pDestTab->pSlots[dest_bankslot];

    /* grab the source/destination Item* */
    if(source_bagslot == 1 && source_slot == 0)
    {
        // find a free bag slot
        if(pDestItem == NULL)
        {
            // dis is fucked up mate
            gInfo->m_GuildLock.Release();
            return;
        }

        SlotResult sr = plr->GetItemInterface()->FindFreeInventorySlot(pDestItem->GetProto());
        if(!sr.Result)
        {
            plr->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_BAG_FULL);
            gInfo->m_GuildLock.Release();
            return;
        }

        source_bagslot = sr.ContainerSlot;
        source_slot = sr.Slot;
    }

    if( source_bagslot == 0xff && source_slot < INVENTORY_SLOT_ITEM_START && pDestItem != NULL)
    {
        sWorld.LogCheater(m_session,"Tried to equip an item from the guild bank (WPE HACK)");
        m_session->SystemMessage("You don't have permission to do that.");
        gInfo->m_GuildLock.Release();
        return;
    }

    if(pDestItem != NULL)
    {
        if(gMember->pRank->iTabPermissions[dest_bank].iStacksPerDay == 0)
        {
            m_session->SystemMessage("You don't have permission to do that.");
            gInfo->m_GuildLock.Release();
            return;
        }

        if(gMember->pRank->iTabPermissions[dest_bank].iStacksPerDay > 0)
        {
            if(CalculateAllowedItemWithdraws(gMember, dest_bank) == 0)
            {
                // a "no permissions" notice would probably be better here
                m_session->SystemMessage("You have withdrawn the maximum amount for today.");
                gInfo->m_GuildLock.Release();
                return;
            }

            /* reduce his count by one */
            OnItemWithdraw(gMember, dest_bank);
        }
    }

    if( source_bagslot == 0xff && source_slot < INVENTORY_SLOT_ITEM_START || source_slot == 0xff )
    {
        plr->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_CANT_DROP_SOULBOUND);
        gInfo->m_GuildLock.Release();
        return;
    }

    Item *pSourceItem = plr->GetItemInterface()->GetInventoryItem(source_bagslot, source_slot);

    /* make sure that both aren't null - wtf ? */
    if(pSourceItem == NULL && pDestItem == NULL)
    {
        gInfo->m_GuildLock.Release();
        return;
    }

    if(pDestItem != NULL && pSourceItem != NULL)
    {
        if(pDestItem->GetEntry() == pSourceItem->GetEntry() && pDestItem->GetProto() != NULL
            && pDestItem->GetStackCount() == pDestItem->GetProto()->MaxCount)
        {
            plr->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_ITEM_CANT_STACK);
            gInfo->m_GuildLock.Release();
            return;
        }
    }

    if(pSourceItem != NULL)
    {
        // make sure its not a soulbound item
        if(pSourceItem->IsSoulbound() || pSourceItem->GetProto()->Class == ITEM_CLASS_QUEST)
        {
            plr->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_CANT_DROP_SOULBOUND);
            gInfo->m_GuildLock.Release();
            return;
        }

        // pull the item from the slot
        if(deposit_stack && pSourceItem->GetStackCount() > deposit_stack)
        {
            Item *pSourceItem2 = pSourceItem;
            pSourceItem = objmgr.CreateItem(pSourceItem2->GetEntry(), plr);
            pSourceItem->SetStackCount(deposit_stack);
            pSourceItem->SetCreatorGUID(pSourceItem2->GetCreatorGUID());
            pSourceItem2->SetStackCount(-(int32)deposit_stack);
            pSourceItem2->m_isDirty = true;
        }
        else
        {
            if(!plr->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(source_bagslot, source_slot, false))
            {
                gInfo->m_GuildLock.Release();
                return;
            }

            pSourceItem->RemoveFromWorld();
        }
    }

    /* perform the swap. */
    uint8 HandleStack = autostore ? withdraw_stack : deposit_stack;

    /* pSourceItem = Source item from players backpack coming into guild bank */
    if(pSourceItem == NULL)
    {
        /* splitting */
        if(pDestItem != NULL && HandleStack > 0 && pDestItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > HandleStack)
        {
            Item *pSourceItem2 = pDestItem;

            pSourceItem2->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT, -(int32)HandleStack);
            pSourceItem2->SaveToDB(0,0,true, NULL);

            pDestItem = objmgr.CreateItem(pSourceItem2->GetEntry(), plr);
            pDestItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT, HandleStack);
            pDestItem->SetUInt32Value(ITEM_FIELD_CREATOR, pSourceItem2->GetUInt32Value(ITEM_FIELD_CREATOR));
        }
        else
        {
            /* that slot in the bank is now empty. */
            pDestTab->pSlots[dest_bankslot] = NULLITEM;
        }
    }
    else
    {
        /* there is a new item in that slot. */
        pDestTab->pSlots[dest_bankslot] = pSourceItem;

        /* remove the item's association with the player* */
        pSourceItem->SetOwner(NULLPLR);
        pSourceItem->SetUInt32Value(ITEM_FIELD_OWNER, 0);
        pSourceItem->SaveToDB(0, 0, true, NULL);

        /* log it */
        LogGuildBankAction(gInfo->m_guildId, GUILD_BANK_LOG_EVENT_DEPOSIT_ITEM, plr->GetLowGUID(), pSourceItem->GetEntry(), (uint8)pSourceItem->GetStackCount(), dest_bank);
    }

    /* pDestItem = Item from bank coming into players backpack */
    if(pDestItem == NULL)
    {
        /*  the item has already been removed from the players backpack at this stage,
            there isn't really much to do at this point. */
    }
    else
    {
        /* the guild was robbed by some n00b! :O */
        pDestItem->SetOwner(plr);
        if(HandleStack > 0)
            pDestItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT, HandleStack);
        pDestItem->SetUInt32Value(ITEM_FIELD_OWNER, plr->GetLowGUID());
        pDestItem->SaveToDB(source_bagslot, source_slot, true, NULL);

        /* add it to him in game */
        if(!plr->GetItemInterface()->SafeAddItem(pDestItem, source_bagslot, source_slot))
        {
            /* this *really* shouldn't happen. */
            if(!plr->GetItemInterface()->AddItemToFreeSlot(pDestItem))
            {
                //pDestItem->DeleteFromDB();
                pDestItem->DeleteMe();
                pDestItem = NULL;
            }
        }
        else
        {
            /* log it */
            LogGuildBankAction(gInfo->m_guildId, GUILD_BANK_LOG_EVENT_WITHDRAW_ITEM, plr->GetLowGUID(), pDestItem->GetEntry(), (uint8)pDestItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT), dest_bank);
        }
    }

    /* update the clients view of the bank tab */
    Packet_SendGuildBankTab(m_session, dest_bank, dest_bankslot);
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();
}

void GuildMgr::Packet_SendAvailableBankFunds(WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildMember* gMember = GetGuildMember(plr->GetLowGUID());
    if(gMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    uint64 money = gInfo->m_bankBalance;
    uint32 avail = CalculateAvailableAmount(gMember);

    /* pls gm mi hero poor give 1 gold coin pl0x */
    WorldPacket data(MSG_GUILD_BANK_MONEY_WITHDRAWN, 4);
    if(avail == 0xFFFFFFFF)
        data << uint32(avail);
    else
        data << uint32(money > avail ? avail : money);
    m_session->SendPacket(&data);
}

//                                      tab1, tab2, tab3, tab4, tab5, tab6
const static int32 GuildBankPrices[6] = { 100, 250,  500,  1000, 2500, 5000 };
void GuildMgr::Packet_BuyBankTab(WorldSession* m_session, uint64 BankGuid)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildMember* gMember = GetGuildMember(plr->GetLowGUID());
    if(gMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    if(gInfo->m_guildLeader != plr->GetLowGUID())
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PERMISSIONS);
        return;
    }

    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(plr->GetGuildId());
    if(BankTabStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    uint8 i = 0;
    for(; i < MAX_GUILD_BANK_TABS; i++)
    {
        if(BankTabStorage->m_Tabs[i] == NULL)
            break;
    }

    if(i == MAX_GUILD_BANK_TABS)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    int32 cost = MONEY_ONE_GOLD * GuildBankPrices[i];
    if(plr->GetUInt32Value(PLAYER_FIELD_COINAGE) < (unsigned)cost)
    {
        m_session->SystemMessage("You do not have enough funds to purchase a new bank tab.");
        return;
    }

    gInfo->m_GuildLock.Acquire();
    plr->ModUnsigned32Value( PLAYER_FIELD_COINAGE, -cost );
    BankTabStorage->m_Tabs[i] = new GuildBankTab(i);
    BankTabStorage->m_TabLogs[i] = new BankLogInternalStorage();
    BankTabStorage->m_TabLogs[i]->log_high_guid = 0;
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;

    gInfo->m_GuildLock.Release();
    LogGuildEvent(plr, gInfo->m_guildId, GUILD_EVENT_BANKTABBOUGHT, "");
}

void GuildMgr::Packet_SendGuildBankTab(WorldSession* m_session, uint8 TabSlot, int32 updated_slot1, int32 updated_slot2)
{
    if(TabSlot > 6)
        return;

    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildMember* gMember = GetGuildMember(plr->GetLowGUID());
    if(gMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(plr->GetGuildId());
    if(BankTabStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    if(BankTabStorage->m_Tabs[TabSlot] == NULL)
        return;

    if(!HasGuildBankRights(plr, TabSlot, GR_RIGHT_GUILD_BANK_VIEW_TAB))
        return;

    gInfo->m_GuildLock.Acquire();
    uint8 tabid = BankTabStorage->m_Tabs[TabSlot]->iTabId;
    WorldPacket data( SMSG_GUILD_BANK_LIST, 1300 );
    data << uint64(gInfo->m_bankBalance); // amount you have deposited
    data << uint8(tabid);
    data << uint32(CalculateAllowedItemWithdraws(gMember, tabid)); // remaining stacks for this day
    data << uint8(0); // some sort of view flag?

    uint8 count = 0;
    size_t pos = data.wpos();
    data << count;

    GuildBankTab* pTab = NULL;
    for(int32 j = 0; j < MAX_GUILD_BANK_SLOTS; ++j)
    {
        pTab = BankTabStorage->m_Tabs[TabSlot];
        if(pTab->pSlots[j] != NULL)
        {
            if(updated_slot1 >= 0 && j == updated_slot1)
                updated_slot1 = -1;

            if(updated_slot2 >= 0 && j == updated_slot2)
                updated_slot2 = -1;

            ++count;

            data << uint8(j);           // slot
            data << pTab->pSlots[j]->GetEntry();
            data << uint32(0);          // 3.3.0 (0x8000, 0x8020) from MaNGOS

            // random props
            if( pTab->pSlots[j]->GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID) )
            {
                data << pTab->pSlots[j]->GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID);
                if( (int32)pTab->pSlots[j]->GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID) < 0 )
                    data << pTab->pSlots[j]->GetItemRandomSuffixFactor();
                else
                    data << uint32(0);
            }
            else
                data << uint32(0);

            // stack
            data << uint32(pTab->pSlots[j]->GetUInt32Value(ITEM_FIELD_STACK_COUNT));
            data << uint32(0);                          // unknown value
            data << uint8(0);                           // unknown 2.4.2
            EnchantmentInstance * ei = pTab->pSlots[j]->GetEnchantment(0);
            if(ei != NULL)
            {
                data << uint8(1);                       // number of enchants
                data << uint8(0);                       // enchantment slot
                data << uint32(ei->Enchantment->Id);    // enchantment id
            }
            else
                data << uint8(0);                       // no enchantment
        }
    }

    // send the forced update slots
    if(updated_slot1 >= 0)
    {
        // this should only be hit if the items null though..
        if(pTab->pSlots[updated_slot1] == NULL)
        {
            ++count;
            data << uint8(updated_slot1);
            data << uint32(0);
        }
    }

    if(updated_slot2 >= 0)
    {
        // this should only be hit if the items null though..
        if(pTab->pSlots[updated_slot2] == NULL)
        {
            ++count;
            data << uint8(updated_slot2);
            data << uint32(0);
        }
    }

    *(uint8*)&data.contents()[pos] = (uint8)count;
    m_session->SendPacket(&data);
    gInfo->m_GuildLock.Release();
}

void GuildMgr::Packet_SendGuildBankInfo(WorldSession* m_session, uint64 BankGuid)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildMember* gMember = GetGuildMember(plr->GetLowGUID());
    if(gMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(plr->GetGuildId());
    if(BankTabStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    gInfo->m_GuildLock.Acquire();
    uint8 count = 0;
    for(uint8 i = 0; i < 6; i++)
        if(BankTabStorage->m_Tabs[i] != NULL)
            count++;

    WorldPacket data(SMSG_GUILD_BANK_LIST, 500);
    data << uint64(gInfo->m_bankBalance);
    data << uint8(0); // Tab 0
    data << uint32(CalculateAllowedItemWithdraws(gMember, 0)); // Send tab 0 allowed items
    data << uint8(1);
    data << uint8(count);
    for(uint8 i = 0; i < count; i++)
    {
        GuildBankTab* Tab = BankTabStorage->m_Tabs[i];
        if(Tab == NULL || !HasGuildBankRights(plr, i, GR_RIGHT_GUILD_BANK_VIEW_TAB))
        {
            data << uint16(0);      // shouldn't happen
            continue;
        }

        if(Tab->szTabName.size())
            data << Tab->szTabName.c_str();
        else
            data << uint8(0);

        if(Tab->szTabIcon.size())
            data << Tab->szTabIcon.c_str();
        else
            data << uint8(0);
    }

    data << uint8(0);
    gInfo->m_GuildLock.Release();
    m_session->SendPacket(&data);
}

void GuildMgr::Packet_DepositMoney(WorldSession* m_session, uint64 BankGuid, uint32 Amount)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    if(plr->GetUInt32Value(PLAYER_FIELD_COINAGE) < Amount)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    if(Amount == 0)
    {
        m_session->SystemMessage("No value given.");
        return;
    }

    gInfo->m_GuildLock.Acquire();
    // add to the bank balance
    gInfo->m_bankBalance += Amount;
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;

    // take the money, oh noes gm pls gief gold mi hero poor
    plr->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -(int32)Amount);

    char buf[20];
    snprintf(buf, 20, I64FMT, ((LLUI)(gInfo->m_bankBalance)));
    gInfo->m_GuildLock.Release();

    // broadcast guild event telling everyone the new balance
    LogGuildEvent(plr, plr->GetGuildId(), GUILD_EVENT_SETNEWBALANCE, buf);

    // log it!
    AddGuildLogEntry(plr->GetGuildId(), GUILD_BANK_LOG_EVENT_DEPOSIT_MONEY, plr->GetLowGUID(), Amount);
    LogGuildBankAction(plr->GetGuildId(), GUILD_BANK_LOG_EVENT_DEPOSIT_MONEY, plr->GetLowGUID(), Amount, 0, 6);
}

void GuildMgr::Packet_WithdrawMoney(WorldSession* m_session, uint64 BankGuid, uint32 Amount)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    uint64 plrAmount = plr->GetUInt32Value(PLAYER_FIELD_COINAGE);
    if(plrAmount == 2147483647)
    {
        m_session->SystemMessage("Gold cap reached.");
        return;
    }

    if(plrAmount+Amount > 2147483647)
    {
        m_session->SystemMessage("Amount exceeds gold cap!");
        return;
    }

    if(Amount == 0)
    {
        m_session->SystemMessage("No value given.");
        return;
    }

    gInfo->m_GuildLock.Acquire();
    if(gInfo->m_bankBalance < Amount)
    {
        gInfo->m_GuildLock.Release();
        m_session->SystemMessage("You cannot withdraw more money then the account holds.");
        return;
    }

    // sanity checks (execpt for guildmasters)
    GuildMember* gMember = GetGuildMember(plr->GetLowGUID());
    if(gMember == NULL || gMember->pRank == NULL)
    {
        gInfo->m_GuildLock.Release();
        return;
    }

    if( gMember->pRank->iId !=0 )
    {
        if(gMember->pRank->iGoldLimitPerDay > 0 && CalculateAvailableAmount(gMember) < Amount )
        {
            gInfo->m_GuildLock.Release();
            m_session->SystemMessage("You have already withdrawn too much today.");
            return;
        }

        if(gMember->pRank->iGoldLimitPerDay == 0 )
        {
            gInfo->m_GuildLock.Release();
            m_session->SystemMessage("You don't have permission to withdraw money.");
            return;
        }
    }

    // update his bank state
    OnMoneyWithdraw(gMember, Amount);

    // give the gold! GM PLS GOLD PLS 1 COIN
    plr->ModUnsigned32Value(PLAYER_FIELD_COINAGE, (uint32)Amount);

    // subtract the balance
    gInfo->m_bankBalance -= Amount;
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;

    char buf[20];
    snprintf(buf, 20, I64FMT, ((LLUI)(gInfo->m_bankBalance)));
    gInfo->m_GuildLock.Release();

    // notify everyone with the new balance
    LogGuildEvent(plr, plr->GetGuildId(), GUILD_EVENT_SETNEWBALANCE, buf);

    // log it!
    AddGuildLogEntry(plr->GetGuildId(), GUILD_BANK_LOG_EVENT_WITHDRAW_MONEY, plr->GetLowGUID(), Amount);
    LogGuildBankAction(plr->GetGuildId(), GUILD_BANK_LOG_EVENT_WITHDRAW_MONEY, plr->GetLowGUID(), Amount, 0, 6);
}

void GuildMgr::Packet_SetBankTabInfo(WorldSession* m_session, uint64 BankGuid, uint8 TabSlot, std::string TabName, std::string TabIcon)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_INTERNAL);
        return;
    }

    if(gInfo->m_guildLeader != plr->GetLowGUID())
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PERMISSIONS);
        return;
    }

    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(plr->GetGuildId());
    if(BankTabStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    GuildBankTab* BankTab = BankTabStorage->m_Tabs[TabSlot];
    if(BankTab == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_INTERNAL);
        return;
    }

    gInfo->m_GuildLock.Acquire();
    if(TabName.size())
    {
        if(!(BankTab->szTabName.size() && strcmp(BankTab->szTabName.c_str(), TabName.c_str()) == 0))
            BankTab->szTabName = TabName.c_str();
    }
    else
    {
        if(BankTab->szTabName.size())
            BankTab->szTabName = "";
    }

    if(TabIcon.size())
    {
        if(!(BankTab->szTabIcon.size() && strcmp(BankTab->szTabIcon.c_str(), TabIcon.c_str()) == 0))
            BankTab->szTabIcon = TabIcon.c_str();
    }
    else
    {
        if(BankTab->szTabIcon.size())
            BankTab->szTabIcon = "";
    }
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();
    Packet_SendGuildBankInfo(m_session, BankGuid);
    Packet_SendGuildBankTab(m_session, TabSlot);
}

