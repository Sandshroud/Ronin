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

void SendGuildCommandResult(WorldSession* pClient, uint32 iCmd, const char* szMsg, uint32 iType)
{
    size_t len = strlen(szMsg);
    WorldPacket data(SMSG_GUILD_COMMAND_RESULT, (9 + len));
    data << iCmd << iType;
    data.WriteBits(len, 8);
    data.append(szMsg, len);
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

    if(gInfo->m_guildLeader != plr->GetGUID())
    {
        SendGuildCommandResult(m_session, GUILD_QUIT_S, "", GUILD_PERMISSIONS);
        return;
    }

    guildmgr.Disband(gInfo->m_guildId);
}

void GuildMgr::Packet_SetMotd(WorldSession* m_session, std::string motd)
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
    data << PlrName << uint8(0);
    inviter->PushPacket(&data);
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

    WoWGuid oldGuild(MAKE_NEW_GUID(thatplr->GetGuildId(), 0, HIGHGUID_TYPE_GUILD));
    WoWGuid newGuild(MAKE_NEW_GUID(gInfo->m_guildId, 0, HIGHGUID_TYPE_GUILD));
    std::string oldGuildName = oldGuild.getLow() ? "Old Guild Name" : "";
    std::string newGuildName = gInfo->m_guildName;
    std::string *thisPlrName = thisplr->GetNameString();

    WorldPacket data(SMSG_GUILD_INVITE, 100);
    data << uint32(gInfo->m_guildLevel);
    data << uint32(gInfo->m_borderStyle);
    data << uint32(gInfo->m_borderColor);
    data << uint32(gInfo->m_emblemStyle);
    data << uint32(gInfo->m_backgroundColor);
    data << uint32(gInfo->m_emblemColor);

    data.WriteGuidBitString(2, newGuild, 3, 2);
    data.WriteBits(oldGuildName.length(), 8);
    data.WriteGuidBitString(1, newGuild, 1);
    data.WriteGuidBitString(6, oldGuild, 6, 4, 1, 5, 7, 2);
    data.WriteGuidBitString(3, newGuild, 7, 0, 6);
    data.WriteBits(newGuildName.length(), 8);
    data.WriteGuidBitString(2, oldGuild, 3, 0);
    data.WriteGuidBitString(1, newGuild, 5);
    data.WriteBits(thisPlrName->length(), 7);
    data.WriteGuidBitString(1, newGuild, 4);
    data.FlushBits();

    data.WriteByteSeq(newGuild[1]);
    data.WriteByteSeq(oldGuild[3]);
    data.WriteByteSeq(newGuild[6]);
    data.WriteSeqByteString(2, oldGuild, 2, 1);
    data.WriteByteSeq(newGuild[0]);
    data.append(oldGuildName.c_str(), oldGuildName.length());
    data.WriteSeqByteString(2, newGuild, 7, 2);
    data.append(thisPlrName->c_str(), thisPlrName->length());
    data.WriteSeqByteString(4, oldGuild, 7, 6, 5, 0);
    data.WriteSeqByteString(1, newGuild, 4);
    data.append(newGuildName.c_str(), newGuildName.length());
    data.WriteSeqByteString(2, newGuild, 5, 3);
    data.WriteSeqByteString(1, oldGuild, 4);
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

    return;
    WorldPacket data(SMSG_GUILD_EVENT_LOG_QUERY_RESULT, 18 * LogStorage->m_logs.size() + 1);
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

    MemberMapStorage->MemberMapLock.Acquire();

    ByteBuffer buffer;
    uint32 weeklyRepCap = GetWeeklyRepCap();
    WorldPacket data(SMSG_GUILD_ROSTER, (4 + gInfo->m_motd.length() + 1 + gInfo->m_guildInfo.length() + 1 + 4 + MemberMapStorage->MemberMap.size() * 50));
    data.WriteBits(gInfo->m_motd.length(), 11);
    data.WriteBits(MemberMapStorage->MemberMap.size(), 18);
    for(GuildMemberMap::iterator itr = MemberMapStorage->MemberMap.begin(); itr != MemberMapStorage->MemberMap.end(); ++itr)
    {
        GuildMember *gMember = itr->second;
        PlayerInfo *pInfo = gMember->pPlayer;
        WoWGuid guid = gMember->PlrGuid;
        data.WriteGuidBitString(2, guid, 3, 4);
        data.WriteBit(false); // Has Authenticator
        data.WriteBit(false); // Can Scroll of Ressurect
        data.WriteBits(gMember->szPublicNote.length(), 8);
        data.WriteBits(gMember->szOfficerNote.length(), 8);
        data.WriteGuidBitString(1, guid, 0);
        data.WriteBits(pInfo->charName.length(), 7);
        data.WriteGuidBitString(5, guid, 1, 2, 6, 5, 7);

        uint8 flags = 0x00;
        Player *player = NULL;
        if (player = gMember->pPlayer->m_loggedInPlayer)
        {
            flags |= 0x01;
            if (player->GetChatTag() == 3)
                flags |= 0x04;
            else if (player->GetChatTag() == 1)
                flags |= 0x02;
        }

        buffer << uint8(pInfo->charClass);
        buffer << uint32(gMember->guildReputation);
        buffer.WriteByteSeq(guid[0]);
        buffer << uint64(gMember->weeklyActivity);
        buffer << uint32(gMember->pRank->iId);
        buffer << uint32(pInfo->achievementPoints); // achievement points
        for(uint8 i = 0; i < 2; i++) // professions: id, value, rank
            buffer << pInfo->professionId[i] << pInfo->professionSkill[i] << pInfo->professionRank[i];
        buffer.WriteByteSeq(guid[2]);
        buffer << uint8(flags);
        buffer << uint32(pInfo->lastZone);
        buffer << uint64(gMember->totalActivity);
        buffer.WriteByteSeq(guid[7]);
        buffer << uint32(std::max<int>(0, weeklyRepCap - gMember->weekReputation)); // Remaining guild week Rep
        buffer.WriteString(gMember->szPublicNote);
        buffer.WriteByteSeq(guid[3]);
        buffer << uint8(pInfo->lastLevel);
        buffer << int32(0); // unk
        buffer.WriteByteSeq(guid[5]);
        buffer.WriteByteSeq(guid[4]);
        buffer << uint8(0); // unk
        buffer.WriteByteSeq(guid[1]);
        buffer << float(player ? 0.f : (float(UNIXTIME - pInfo->lastOnline) / 86400.f));
        buffer.WriteString(gMember->szOfficerNote);
        buffer.WriteByteSeq(guid[6]);
        buffer.WriteString(pInfo->charName);
    }

    data.WriteBits(gInfo->m_guildInfo.length(), 12);
    data.FlushBits();
    data.append(buffer.contents(), buffer.size());

    data.WriteString(gInfo->m_guildInfo);
    data.WriteString(gInfo->m_motd);

    data << uint32(MemberMapStorage->MemberMap.size());
    data << uint32(weeklyRepCap);  // weekly rep cap
    data << uint32(RONIN_UTIL::secsToTimeBitFields(gInfo->m_creationTimeStamp));
    data << uint32(0);

    MemberMapStorage->MemberMapLock.Release();
    m_session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildXP(WorldSession *session)
{
    Player *plr = session->GetPlayer();
    if(plr == NULL || !plr->IsInGuild())
        return;

    GuildMember *gMember = GetGuildMember(plr->GetGUID());
    if(gMember == NULL)
        return;

    GuildInfo *gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
        return;

    gInfo->guildXPLock.Acquire();
    uint64 xpTillNextLevel = GetXPForNextGuildLevel(gInfo->m_guildLevel), guildXP = xpTillNextLevel ? gInfo->m_guildExperience : 0;
    if(xpTillNextLevel && guildXP)
        xpTillNextLevel -= guildXP;

    WorldPacket data(SMSG_GUILD_XP, 40);
    data << uint64(gInfo->m_xpGainedToday); // Member xp given today
    data << uint64(xpTillNextLevel);        // XP missing for next level
    data << uint64(gMember->guildXPToday);  // XP gained today
    data << uint64(gMember->guildWeekXP);   // Member xp given this week
    data << uint64(guildXP);                // Total XP
    gInfo->guildXPLock.Release();
    session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildMaxDailyXP(WorldSession *session, WoWGuid guid)
{
    if(GuildMember *gMember = GetGuildMember(session->GetPlayer()->GetGUID()))
    {
        if(gMember->guildId == guid.getLow())
        {
            WorldPacket data(SMSG_GUILD_MAX_DAILY_XP, 8);
            data << uint64(gMember->pRank->DailyXPCap*gMember->guildXPCapModifier);
            session->SendPacket(&data);
        }
    }
}

void GuildMgr::Packet_SendGuildNews(WorldSession *session)
{
    WorldPacket data(SMSG_GUILD_NEWS_UPDATE, (21 + 0 * (26 + 8)) / 8 + (8 + 6 * 4) * 0);
    data.WriteBits(0, 21); // Num logs

    // for(size) writeBits(0, 26); writeguidmaskbits(8, guid, 7, 0, 6, 5, 4, 3, 1, 2);

    data.FlushBits();

    /*for (GuildLog::const_iterator itr = logs->begin(); itr != logs->end(); ++itr)
    {
        NewsLogEntry* news = (NewsLogEntry*)(*itr);
        ObjectGuid guid = news->GetPlayerGuid();
        data.WriteByteSeq(guid[5]);

        data << uint32(news->GetFlags());   // 1 sticky
        data << uint32(news->GetValue());
        data << uint32(0);

        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[1]);

        data << uint32(news->GetGUID());
        data << uint32(news->GetType());
        data.AppendPackedTime(news->GetTimestamp());
    }*/
    session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildRankInfo(WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
        return;

    GuildRankStorage* RankStorage = GetGuildRankStorage(plr->GetGuildId());
    if(RankStorage == NULL)
        return;

    ByteBuffer buffer;
    WorldPacket data(SMSG_GUILD_RANK, 20);
    data.WriteBits(RankStorage->ssid, 18);
    for(uint8 i = 0; i < RankStorage->ssid; ++i)
    {
        GuildRank *rank = RankStorage->m_ranks[i];
        data.WriteBits(rank ? rank->szRankName.length() : 0, 7);

        buffer << uint32(i);
        for (uint8 x = 0; x < MAX_GUILD_BANK_TABS; x++)
        {
            buffer << uint32(rank ? rank->iTabPermissions[x].iStacksPerDay : 0);
            buffer << uint32(rank ? rank->iTabPermissions[x].iFlags : 0);
        }

        buffer << uint32(rank ? rank->iGoldLimitPerDay : 0);
        buffer << uint32(rank ? rank->iRights : 0);
        buffer.WriteString(rank ? rank->szRankName : "");
        buffer << uint32(i);
    }
    data.FlushBits();
    data.append(buffer.contents(), buffer.size());
    m_session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildRewards(WorldSession *session)
{
    WorldPacket data(SMSG_GUILD_REWARDS_LIST, 3 + 0 * (4 + 4 + 4 + 8 + 4 + 4));
    data.WriteBits(0, 21);
    data.FlushBits();
    /*for (uint32 i = 0; i < rewards.size(); i++)
    {
        data << uint32(rewards[i].Standing);
        data << int32(rewards[i].Racemask);
        data << uint32(rewards[i].Entry);
        data << uint64(rewards[i].Price);
        data << uint32(0); // Unused
        data << uint32(rewards[i].AchievementId);
    }*/
    data << uint32(UNIXTIME);
    session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildPermissions(WorldSession *session)
{
    Player* plr = session->GetPlayer();
    if(!plr->IsInGuild())
        return;

    GuildInfo* gInfo = GetGuildInfo(plr->GetGuildId());
    if(gInfo == NULL)
        return;

    GuildMember *gMember = GetGuildMember(plr->GetGUID());
    if(gMember == NULL || gMember->pRank == NULL)
        return;

    uint32 count = 0;
    if(GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(gInfo->m_guildId))
        count = BankTabStorage->ssid;

    WorldPacket data(SMSG_GUILD_PERMISSIONS_QUERY_RESULTS, 4 * 15 + 1);
    data << uint32(gMember->pRank->iId);
    data << uint32(count);
    data << uint32(gMember->pRank->iRights);
    data << uint32(CalculateAvailableAmount(gMember));
    data.WriteBits(MAX_GUILD_BANK_TABS, 23);
    data.FlushBits();
    for (uint8 tabId = 0; tabId < MAX_GUILD_BANK_TABS; ++tabId)
    {
        data << uint32(gMember->pRank->iTabPermissions[tabId].iFlags);
        data << uint32(CalculateAllowedItemWithdraws(gMember, tabId));
    }

    session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildPartyState(WorldSession *session)
{
    WorldPacket data(SMSG_GUILD_PARTY_STATE_RESPONSE, 13);
    data.WriteBit(false); // Is guild group
    data.FlushBits();
    data << float(0.f);                                                             // Guild XP multiplier
    data << uint32(0);                                                              // Current guild members
    data << uint32(0);                                                              // Needed guild members
    session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildChallengeUpdate(WorldSession *session)
{
    WorldPacket data(SMSG_GUILD_CHALLENGE_UPDATED, 4 * GUILD_CHALLENGES_TYPES * 5);
    for (uint8 i = 0; i < GUILD_CHALLENGES_TYPES; ++i)
        data << uint32(GuildChallengeXPReward[i]);
    for (uint8 i = 0; i < GUILD_CHALLENGES_TYPES; ++i)
        data << uint32(GuildChallengeGoldReward[i]);
    for (uint8 i = 0; i < GUILD_CHALLENGES_TYPES; ++i)
        data << uint32(GuildChallengesPerWeek[i]);
    for (uint8 i = 0; i < GUILD_CHALLENGES_TYPES; ++i)
        data << uint32(GuildChallengeMaxLevelGoldReward[i]);
    for (uint8 i = 0; i < GUILD_CHALLENGES_TYPES; ++i)
        data << uint32(0); /// @todo current count
    session->SendPacket(&data);
}

void GuildMgr::Packet_SendGuildQuery(WorldSession* m_session, uint32 GuildId,  WoWGuid targetPlayer)
{
    if(!targetPlayer.empty())
    {
        GuildMember *gMember = GetGuildMember(targetPlayer);
        if(gMember == NULL || gMember->guildId != GuildId)
            return;
    }

    GuildInfo* gInfo = GetGuildInfo(GuildId);
    if(gInfo == NULL)
        return;

    GuildRankStorage* RankStorage = GetGuildRankStorage(GuildId);
    if(RankStorage == NULL)
        return;

    WorldPacket data(SMSG_GUILD_QUERY_RESPONSE, 8+8*MAX_GUILD_RANKS+100);
    data << uint64(MAKE_NEW_GUID(gInfo->m_guildId, 0, HIGHGUID_TYPE_GUILD));
    data << gInfo->m_guildName;

    RankStorage->RankLock.Acquire();
    // Rank names
    for(uint8 i = 0; i < MAX_GUILD_RANKS; ++i)
    {
        if(GuildRank* r = RankStorage->m_ranks[i])
            data << r->szRankName;
        else data << uint8(0);
    }
    // Order by creation
    for(uint8 i = 0; i < MAX_GUILD_RANKS; ++i)
    {
        if(RankStorage->m_ranks[i])
            data << uint32(i);
        else data << uint32(0);
    }
    // Order by importance
    for(uint8 i = 0; i < MAX_GUILD_RANKS; ++i)
    {
        if(RankStorage->m_ranks[i])
            data << uint32(i);
        else data << uint32(0);
    }

    data << gInfo->m_emblemStyle;
    data << gInfo->m_emblemColor;
    data << gInfo->m_borderStyle;
    data << gInfo->m_borderColor;
    data << gInfo->m_backgroundColor;
    data << RankStorage->ssid;
    RankStorage->RankLock.Release();

    if(m_session != NULL)
        m_session->SendPacket(&data);
    else if(GuildMemberMapStorage* MemberListStorage = GetGuildMemberMapStorage(GuildId))
    {
        for(GuildMemberMap::iterator itr = MemberListStorage->MemberMap.begin(); itr != MemberListStorage->MemberMap.end(); ++itr)
            if(Player *player = itr->second->pPlayer->m_loggedInPlayer)
                player->PushPacket(&data);
    }
}

void GuildMgr::Packet_SetMemberNote(WorldSession* m_session, PlayerInfo* pInfo, bool officer, std::string note)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild() || plr->GetGuildId() != pInfo->GuildId)
        return;

    if(!HasGuildRights(plr, officer ? GR_RIGHT_EOFFNOTE : GR_RIGHT_EPNOTE))
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PERMISSIONS);
        return;
    }

    SetNote(pInfo, note, officer);

    //SendGuildCommandResult(m_session, GUILD_PUBLIC_NOTE_CHANGED_S, pInfo->charName.c_str(), 0);
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
    if(gInfo->m_guildLeader != plr->GetGUID())
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
    if(gInfo->m_guildLeader != plr->GetGUID())
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
    if(gInfo->m_guildLeader != plr->GetGUID())
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
    if(gInfo->m_guildLeader != plr->GetGUID())
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
    if(gInfo->m_guildLeader != plr->GetGUID())
    {
        SendGuildCommandResult(m_session, GUILD_CREATE_S, "", GUILD_PERMISSIONS);
        return;
    }

    GuildMember *GuildLeader = GetGuildMember(plr->GetLowGUID()), *NewGuildLeader = GetGuildMember(newLeader->charGuid);
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
    gInfo->m_guildLeader = newLeader->charGuid;
    NewGuildLeader->pRank = RankStorage->m_ranks[0];
    if(newLeader->m_loggedInPlayer)
        newLeader->m_loggedInPlayer->SetGuildRank(0);
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();

    LogGuildEvent(NULL, plr->GetGuildId(), GUILD_EVENT_LEADER_CHANGED, plr->GetName(), newLeader->charName.c_str());
}

void GuildMgr::Packet_DemoteGuildMember(WorldSession* m_session, PlayerInfo *demoteeInfo)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(plr->GetGuildId() != demoteeInfo->GuildId)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, demoteeInfo->charName.c_str(), GUILD_PLAYER_NOT_IN_GUILD_S);
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

    GuildMember* gMember = GetGuildMember(plr->GetGUID()), *DemotedMember = GetGuildMember(demoteeInfo->charGuid);
    if(gMember == NULL || DemotedMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, demoteeInfo->charName.c_str(), GUILD_PLAYER_NOT_IN_GUILD_S);
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
    LogGuildEvent(NULL, plr->GetGuildId(), GUILD_EVENT_DEMOTION, plr->GetName(), demoteeInfo->charName.c_str(), newRank->szRankName.c_str());
    AddGuildLogEntry(plr->GetGuildId(), GUILD_LOG_EVENT_DEMOTION, plr->GetLowGUID(), demoteeInfo->charGuid.getLow(), newRank->iId);

    // if the player is online, update his guildrank
    if(demoteeInfo->m_loggedInPlayer)
        demoteeInfo->m_loggedInPlayer->SetGuildRank(nh);
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();
}

void GuildMgr::Packet_PromoteGuildMember(WorldSession* m_session, PlayerInfo *promoteeInfo)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(promoteeInfo->GuildId != plr->GetGuildId())
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, promoteeInfo->charName.c_str(), GUILD_PLAYER_NOT_IN_GUILD_S);
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

    GuildMember* gMember = GetGuildMember(plr->GetGUID()), *PromotedMember = GetGuildMember(promoteeInfo->charGuid);
    if(gMember == NULL || PromotedMember == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_PROMOTE_S, promoteeInfo->charName.c_str(), GUILD_PLAYER_NOT_IN_GUILD_S);
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
    LogGuildEvent(NULL, plr->GetGuildId(), GUILD_EVENT_PROMOTION, plr->GetName(), promoteeInfo->charName.c_str(), newRank->szRankName.c_str());
    AddGuildLogEntry(plr->GetGuildId(), GUILD_LOG_EVENT_PROMOTION, plr->GetLowGUID(), promoteeInfo->charGuid.getLow(), newRank->iId);

    // if the player is online, update his guildrank
    if(promoteeInfo->m_loggedInPlayer)
        promoteeInfo->m_loggedInPlayer->SetGuildRank(nh);
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    gInfo->m_GuildLock.Release();
}

// Charter Buy
void GuildMgr::CharterBuy(WorldSession* m_session, uint64 SellerGuid, std::string name, uint32 petitionCount, uint32 ArenaIndex)
{
    uint8 error = 0;
    Creature* crt = m_session->GetPlayer()->GetMapInstance()->GetCreature(SellerGuid);
    if(!crt)
    {
        m_session->Disconnect();
        return;
    }

    if( ArenaIndex >= NUM_CHARTER_TYPES )
        return;

    //All arena organizers should be allowed to create arena charter's
    if( crt->ArenaOrganizersFlags() )
    {

    }
    else
    {
        if( m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE) < 1000)
        {
            m_session->SendNotification("You don't have enough money.");
            return;
        }

        if(m_session->GetPlayer()->getPlayerInfo()->charterId[CHARTER_TYPE_GUILD] != 0)
        {
            m_session->SendNotification("You already have a guild charter.");
            return;
        }

        if(!sWorld.VerifyName(name.c_str(), name.length()))
        {
            m_session->SendNotification("That name is invalid or contains invalid characters.");
            return;
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

    return;
    uint8 count = 0;
    gInfo->m_GuildLock.Acquire();
    WorldPacket data(SMSG_GUILD_BANK_LOG_QUERY_RESULT);
    if(slotid == 6)
    {
        // sending the money log
        data.resize((17*BankTabStorage->m_money_logs.size()) + 2);
        uint32 lt = (uint32)UNIXTIME;
        data << uint8(0x06);
        data << uint8((BankTabStorage->m_money_logs.size() < 25) ? BankTabStorage->m_money_logs.size() : 25);
        std::list<GuildBankEvent*>::iterator itr = BankTabStorage->m_money_logs.begin();
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

    WorldPacket data( SMSG_GUILD_BANK_QUERY_TEXT_RESULT, 1 + Tab->szTabInfo.length() );
    data.WriteBits(Tab->szTabInfo.length(), 14);
    data.FlushBits();
    data << uint32(TabSlot);
    data.append(Tab->szTabInfo.c_str(), Tab->szTabInfo.length());
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

    uint64 money = gInfo->m_bankBalance, avail = CalculateAvailableAmount(gMember);

    /* pls gm mi hero poor give 1 gold coin pl0x */
    WorldPacket data(SMSG_GUILD_BANK_MONEY_WITHDRAWN, 8);
    if(avail == 0xFFFFFFFFFFFFFFFF)
        data << uint64(0xFFFFFFFFFFFFFFFF);
    else data << uint64(money > avail ? avail : money);
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

    if(gInfo->m_guildLeader != plr->GetGUID())
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

void GuildMgr::Packet_SendGuildBankTab(WorldSession* m_session, uint8 TabSlot, bool withTabInfo)
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
    GuildBankTab *pTab = BankTabStorage->m_Tabs[TabSlot];

    WorldPacket data(SMSG_GUILD_BANK_LIST, 1300);
    data.WriteBit(0);
    data.WriteBits(0, 20); // Iteminfo
    data.WriteBits(0, 22); // tabinfo
    data.FlushBits();
    data << uint64(gInfo->m_bankBalance); // amount you have deposited
    // Append byte buffer here
    data << uint32(pTab->iTabId);
    data << uint32(CalculateAllowedItemWithdraws(gMember, pTab->iTabId));
    gInfo->m_GuildLock.Release();
    m_session->SendPacket(&data);
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

    WorldPacket data(SMSG_GUILD_BANK_LIST, 1300);
    data.WriteBit(0);
    data.WriteBits(0, 20); // Iteminfo
    data.WriteBits(0, 22); // tabinfo
    data.FlushBits();
    data << uint64(gInfo->m_bankBalance); // amount you have deposited
    // Append byte buffer here
    data << uint32(0);
    data << uint32(CalculateAllowedItemWithdraws(gMember, 0));
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

    if(gInfo->m_guildLeader != plr->GetGUID())
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
    Packet_SendGuildBankTab(m_session, TabSlot, true);
}

