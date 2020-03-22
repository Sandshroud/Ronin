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

void WorldSession::HandleGuildQuery(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WoWGuid guildId, playerGuid;
    recv_data >> guildId >> playerGuid;
    guildmgr.Packet_SendGuildQuery(this, guildId.getLow(), playerGuid);
}

void WorldSession::HandleGuildXP(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_SendGuildXP(this);
}

void WorldSession::HandleGuildMaxDailyXP(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    recv_data.ReadGuidBitString(8, guid, ByteBuffer::Filler, 0, 3, 5, 1, 4, 6, 7, 2);
    recv_data.ReadGuidByteString(8, guid, ByteBuffer::Filler, 7, 4, 3, 5, 1, 2, 6, 0);

    guildmgr.Packet_SendGuildMaxDailyXP(this, guid);
}

void WorldSession::HandleGuildNews(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_SendGuildNews(this);
}

void WorldSession::HandleGuildRanks(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WoWGuid guildId;
    recv_data.ReadGuidBitString(8, guildId, ByteBuffer::Filler, 2, 3, 0, 6, 4, 7, 5, 1);
    recv_data.ReadGuidByteString(8, guildId, ByteBuffer::Filler, 3, 4, 5, 7, 1, 0, 6, 2);
    guildmgr.Packet_SendGuildRankInfo(this);
}

void WorldSession::HandleGuildRoster(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid1, guid2;

    recv_data.ReadGuidBitString(2, guid2, ByteBuffer::Filler, 2, 3);
    recv_data.ReadGuidBitString(2, guid1, ByteBuffer::Filler, 6, 0);
    recv_data.ReadGuidBitString(1, guid2, ByteBuffer::Filler, 7);
    recv_data.ReadGuidBitString(1, guid1, ByteBuffer::Filler, 2);
    recv_data.ReadGuidBitString(2, guid2, ByteBuffer::Filler, 6, 4);
    recv_data.ReadGuidBitString(1, guid1, ByteBuffer::Filler, 1);
    recv_data.ReadGuidBitString(1, guid2, ByteBuffer::Filler, 5);
    recv_data.ReadGuidBitString(2, guid1, ByteBuffer::Filler, 4, 3);
    recv_data.ReadGuidBitString(1, guid2, ByteBuffer::Filler, 0);
    recv_data.ReadGuidBitString(1, guid1, ByteBuffer::Filler, 5);
    recv_data.ReadGuidBitString(1, guid2, ByteBuffer::Filler, 1);
    recv_data.ReadGuidBitString(1, guid1, ByteBuffer::Filler, 7);

    // Skip the rest of the packet
    recv_data.rpos(recv_data.size());
    // This is wrong, gotta find the right structure
    /*recv_data.ReadGuidByteString(1, guid1, ByteBuffer::Filler, 3);
    recv_data.ReadGuidByteString(1, guid2, ByteBuffer::Filler, 4);
    recv_data.ReadGuidByteString(4, guid1, ByteBuffer::Filler, 7, 2, 4, 0);
    recv_data.ReadGuidByteString(1, guid2, ByteBuffer::Filler, 5);
    recv_data.ReadGuidByteString(1, guid1, ByteBuffer::Filler, 1);
    recv_data.ReadGuidByteString(2, guid2, ByteBuffer::Filler, 0, 6);
    recv_data.ReadGuidByteString(1, guid1, ByteBuffer::Filler, 5);
    recv_data.ReadGuidByteString(4, guid2, ByteBuffer::Filler, 7, 2, 3, 1);
    recv_data.ReadGuidByteString(1, guid1, ByteBuffer::Filler, 6);*/

    guildmgr.Packet_SendGuildRoster(this);
}

void WorldSession::HandleGuildRewards(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    recv_data.read_skip<uint32>(); // Unk

    guildmgr.Packet_SendGuildRewards(this);
}

void WorldSession::HandleGuildPermissions(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_SendGuildPermissions(this);
}

void WorldSession::HandleGuildPartyState(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    if(_player->GetGroup() == NULL)
        return;

    guildmgr.Packet_SendGuildPartyState(this);
}

void WorldSession::HandleGuildChallengeUpdate(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_SendGuildChallengeUpdate(this);
}

void WorldSession::HandleInviteToGuild(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    std::string inviteeName = recv_data.ReadString(recv_data.ReadBits(7));
    guildmgr.Packet_HandleGuildInvite(this, inviteeName);
}

void WorldSession::HandleGuildAccept(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_GuildInviteAccept(this);
}

void WorldSession::HandleGuildDecline(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_GuildInviteDecline(this);
}

void WorldSession::HandleSetGuildInformation(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    std::string NewGuildInfo = recv_data.ReadString(recv_data.ReadBits(12));
    guildmgr.Packet_SetGuildInformation(this, NewGuildInfo);
}

void WorldSession::HandleGuildPromote(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WoWGuid targetGuid;
    recv_data.ReadGuidBitString(8, targetGuid, ByteBuffer::Filler, 7, 2, 5, 6, 1, 0, 3, 4);
    recv_data.ReadGuidByteString(8, targetGuid, ByteBuffer::Filler, 0, 5, 2, 3, 6, 4, 1, 7);

    if(PlayerInfo* Promoted = objmgr.GetPlayerInfo(targetGuid))
    {
        guildmgr.Packet_PromoteGuildMember(this, Promoted);
        return;
    }

    SendGuildCommandResult(this, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_FOUND);
    return;
}

void WorldSession::HandleGuildDemote(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WoWGuid targetGuid;
    recv_data.ReadGuidBitString(8, targetGuid, ByteBuffer::Filler, 7, 1, 5, 6, 2, 3, 0, 4);
    recv_data.ReadGuidByteString(8, targetGuid, ByteBuffer::Filler, 1, 2, 7, 5, 6, 0, 4, 3);

    if(PlayerInfo* Demoted = objmgr.GetPlayerInfo(targetGuid))
    {
        guildmgr.Packet_DemoteGuildMember(this, Demoted);
        return;
    }

    SendGuildCommandResult(this, GUILD_PROMOTE_S, "", GUILD_PLAYER_NOT_FOUND);
    return;
}

void WorldSession::HandleGuildLeave(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.RemoveMember(_player, _player->getPlayerInfo());
}

void WorldSession::HandleGuildRemove(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WoWGuid memberGuid;
    recv_data.ReadGuidBitString(8, memberGuid, ByteBuffer::Filler, 6, 5, 4, 0, 1, 3, 7, 2);
    recv_data.ReadGuidByteString(8, memberGuid, ByteBuffer::Filler, 2, 6, 5, 7, 1, 4, 3, 0);

    PlayerInfo * dstplr = objmgr.GetPlayerInfo(memberGuid);
    if( dstplr == NULL )
        return;

    guildmgr.RemoveMember(_player, dstplr);
}

void WorldSession::HandleGuildDisband(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_DisbandGuild(this);
}

void WorldSession::HandleGuildMotd(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    std::string motd = recv_data.ReadString(recv_data.ReadBits(11));

    guildmgr.Packet_SetMotd(this, motd);
}

void WorldSession::HandleGuildAddRank(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    std::string rankName;
    recv_data >> rankName;
    if(rankName.size() < 2)
        return;

    guildmgr.Packet_HandleAddRank(this, rankName);
}

void WorldSession::HandleGuildDelRank(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_HandleDeleteRank(this);
}

void WorldSession::HandleGuildSetNote(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WoWGuid targetGuid;
    recv_data.ReadGuidBitString(6, targetGuid, ByteBuffer::Filler, 1, 4, 5, 3, 0, 7);
    bool officer = !recv_data.ReadBit();
    recv_data.ReadGuidBitString(1, targetGuid, ByteBuffer::Filler, 6);
    size_t noteLen = recv_data.ReadBits(8);
    recv_data.ReadGuidBitString(1, targetGuid, ByteBuffer::Filler, 2);

    recv_data.ReadGuidByteString(7, targetGuid, ByteBuffer::Filler, 4, 5, 0, 3, 1, 6, 7);
    std::string newnote = recv_data.ReadString(noteLen);
    recv_data.ReadGuidByteString(1, targetGuid, ByteBuffer::Filler, 2);

    PlayerInfo *pInfo = objmgr.GetPlayerInfo(targetGuid);
    if(pInfo == NULL)
        return;

    guildmgr.Packet_SetMemberNote(this, pInfo, officer, newnote);;
}

void WorldSession::HandleSaveGuildEmblem(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint64 guid;
    uint32 emblemStyle, emblemColor, borderStyle, borderColor, backgroundColor;
    recv_data >> guid >> emblemStyle >> emblemColor >> borderStyle >> borderColor >> backgroundColor;

    guildmgr.Packet_SaveGuildEmblem(this, emblemStyle, emblemColor, borderStyle, borderColor, backgroundColor);
}

// Charter part
void WorldSession::HandleCharterBuy(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    std::string name;
    uint64 creature_guid;
    uint32 arena_index, PetitionSignerCount;

    recv_data >> creature_guid;
    recv_data.read_skip<uint32>();
    recv_data.read_skip<uint64>();
    recv_data >> name;
    recv_data.read_skip<std::string>();
    for(uint32 s = 0; s < 7; ++s)
        recv_data.read_skip<uint32>();
    recv_data.read_skip<uint16>();
    recv_data.read_skip<uint64>();
    recv_data >> PetitionSignerCount;
    for(uint32 s = 0; s < 10; ++s)
        recv_data.read_skip<std::string>();
    recv_data >> arena_index;

    guildmgr.CharterBuy(this, creature_guid, name, PetitionSignerCount, arena_index);
}

void WorldSession::HandleGuildLog(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_SendGuildLog(this);
}

void WorldSession::HandleGuildBankBuyTab(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;

    guildmgr.Packet_BuyBankTab(this, guid);
}

void WorldSession::HandleGuildBankGetAvailableAmount(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_SendAvailableBankFunds(this);
}

void WorldSession::HandleGuildBankModifyTab(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint8 slot;
    uint64 guid;
    std::string tabname, tabicon;

    recv_data >> guid;
    recv_data >> slot;
    recv_data >> tabname;
    recv_data >> tabicon;

    guildmgr.Packet_SetBankTabInfo(this, guid, slot, tabname, tabicon);
}

void WorldSession::HandleGuildBankWithdrawMoney(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    uint32 amount;

    recv_data >> guid;
    recv_data >> amount;
    int64 highamount = int64(amount);
    if(highamount >= 2147483647 || highamount < 0)
        amount = 0;

    guildmgr.Packet_WithdrawMoney(this, guid, amount);
}

void WorldSession::HandleGuildBankDepositMoney(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    uint32 amount;

    recv_data >> guid;
    recv_data >> amount;
    int64 highamount = int64(amount);
    if(highamount >= 2147483647 || highamount < 0)
        amount = 0;

    guildmgr.Packet_DepositMoney(this, guid, amount);
}

void WorldSession::HandleGuildBankSwapItem(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint64 guid;
    uint8 source_isfrombank;
    recv_data >> guid >> source_isfrombank;
    if(source_isfrombank)
    {
        uint8 autostore;
        uint8 dest_bank;
        uint8 dest_bankslot;
        uint8 source_bank;
        uint8 source_bankslot;
        int32 splitted_count;
        uint32 itementry;

        /* read packet */
        recv_data >> dest_bank;
        recv_data >> dest_bankslot;
        recv_data.read_skip<uint32>();
        recv_data >> source_bank;
        recv_data >> source_bankslot;

        recv_data >> itementry;
        recv_data >> autostore;
        recv_data >> splitted_count;

        guildmgr.Packet_WithdrawItem(this, dest_bank, dest_bankslot, source_bank, source_bankslot, itementry, autostore, splitted_count);
    }
    else
    {
        uint8 autostore;
        uint8 dest_bank;
        uint8 dest_bankslot;
        uint8 withdraw_stack = 0;
        uint8 deposit_stack = 0;
        uint8 tochar = 0;
        int8 source_slot;
        int8 source_bagslot;
        uint32 itementry;

        /* read packet */
        recv_data >> dest_bank;
        recv_data >> dest_bankslot;
        recv_data >> itementry;
        recv_data >> autostore;
        if( autostore )
            recv_data >> withdraw_stack;

        recv_data >> source_bagslot;
        recv_data >> source_slot;

        if(!(source_bagslot == 1 && source_slot == 0))
            recv_data >> tochar >> deposit_stack;

        guildmgr.Packet_DepositItem(this, dest_bank, dest_bankslot, itementry, autostore, withdraw_stack, source_bagslot, source_slot, tochar, deposit_stack);
    }
}

void WorldSession::HandleGuildBankOpenVault(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;
    recv_data.read_skip<uint8>();

    guildmgr.Packet_SendGuildBankInfo(this, guid);
}

void WorldSession::HandleGuildBankViewTab(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    WoWGuid guid;
    uint8 tabid, fullTabs;
    recv_data >> guid >> tabid >> fullTabs;

    guildmgr.Packet_SendGuildBankTab(this, tabid, false);
}

void WorldSession::HandleGuildBankViewLog(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    /* slot 6 = I'm requesting money log */

    uint8 slotid;
    recv_data >> slotid;
    guildmgr.Packet_SendGuildBankLog(this, slotid);
}

void WorldSession::HandleGuildBankQueryText( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    uint8 tabid;
    recv_data >> tabid;
    guildmgr.Packet_SendGuildBankText(this, tabid);
}

void WorldSession::HandleSetGuildBankText( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    uint8 tabid;
    std::string tabtext;
    recv_data >> tabid >> tabtext;
    guildmgr.Packet_SetGuildBankText(this, tabid, tabtext);
}

/******************
** CHARTER HANDLERS
*******/

void SendShowSignatures(Charter * c, uint64 i, Player * p)
{
    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, 100);
    data << i;
    data << uint64(c->GetLeader());
    data << c->GetID();
    data << uint8(c->SignatureCount);
    for(uint32 j = 0; j < c->Slots; ++j)
    {
        if(c->Signatures[j].empty())
            continue;

        data << uint64(c->Signatures[j]) << uint32(1);
    }
    data << uint8(0);
    p->PushPacket(&data);
}

void WorldSession::HandleCharterShowSignatures(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    Charter * pCharter;
    uint64 item_guid;
    recv_data >> item_guid;
    pCharter = guildmgr.GetCharterByItemGuid(item_guid);

    if(pCharter)
        SendShowSignatures(pCharter, item_guid, _player);
}

void WorldSession::HandleCharterQuery(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint32 charter_id;
    uint64 item_guid;
    recv_data >> charter_id;
    recv_data >> item_guid;

    Charter * c = guildmgr.GetCharterByItemGuid(item_guid);
    if(c == NULL)
        return;

    WorldPacket data(SMSG_PETITION_QUERY_RESPONSE, 100);
    data << charter_id;
    data << (uint64)c->LeaderGuid;
    data << c->GuildName << uint8(0);
    if(c->CharterType == CHARTER_TYPE_GUILD)
        data << uint32(9) << uint32(9);
    else
        data << uint32(c->Slots) << uint32(c->Slots);

    data << uint32(0);                                      // 4
    data << uint32(0);                                      // 5
    data << uint32(0);                                      // 6
    data << uint32(0);                                      // 7
    data << uint32(0);                                      // 8
    data << uint16(0);                                      // 9 2 bytes field

    uint32 maxlevel = std::max<uint32>(80, _player->GetUInt32Value(PLAYER_FIELD_MAX_LEVEL));
    if( c->CharterType == CHARTER_TYPE_GUILD )
        data << uint32(1) << uint32(maxlevel);              // 10 minlevel
    else data << uint32(80) << uint32(maxlevel);            // 10

    data << uint32(0);                                      // 12
    data << uint32(0);                                      // 13 count of next std::strings?
    data << uint32(0);                                      // 14
    data << uint32(0);                                      // 15
    data << uint16(0);                                      // 16

    if (c->CharterType == CHARTER_TYPE_GUILD)
        data << uint32(0);
    else data << uint32(1);

    SendPacket(&data);
}

void WorldSession::HandleCharterOffer( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint32 shit;
    uint64 item_guid, target_guid;
    Charter * pCharter;
    recv_data >> shit >> item_guid >> target_guid;

    CHECK_INWORLD_RETURN();
    Player* pTarget = _player->GetMapInstance()->GetPlayer((uint32)target_guid);
    pCharter = guildmgr.GetCharterByItemGuid(item_guid);

    if( pCharter == NULL )
    {
        SendNotification("Charter cannot be found.");
        return;
    }

    if(pTarget == 0 || pTarget->GetTeam() != _player->GetTeam() || pTarget == _player)
    {
        SendNotification("Target is of the wrong faction.");
        return;
    }

    if(!pTarget->CanSignCharter(pCharter, _player))
    {
        SendNotification("Target player cannot sign your charter for one or more reasons.");
        return;
    }

    SendShowSignatures(pCharter, item_guid, pTarget);
}

void WorldSession::HandleCharterSign( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    uint64 item_guid;
    recv_data >> item_guid;

    Charter * c = guildmgr.GetCharterByItemGuid(item_guid);
    if( c == NULL )
        return;

    if( _player->m_playerInfo->charterId[c->CharterType] != 0 )
    {
        SendNotification("You cannot sign two charters of the same type.");
        return;
    }

    for(uint32 i = 0; i < 9; i++)
    {
        if(c->Signatures[i] == _player->GetGUID())
        {
            SendNotification("You have already signed that charter.");
            return;
        }
    }

    if(c->IsFull())
        return;

    c->AddSignature(_player->GetLowGUID());
    c->SaveToDB();
    _player->m_playerInfo->charterId[c->CharterType] = c->GetID();
    _player->SaveToDB(false);

    Player * l = _player->GetMapInstance()->GetPlayer(c->GetLeader());
    if(l == 0)
        return;

    WorldPacket data(SMSG_PETITION_SIGN_RESULTS, 100);
    data << item_guid << _player->GetGUID() << uint32(0);
    l->PushPacket(&data);
    data.clear();
    data << item_guid << (uint64)c->GetLeader() << uint32(0);
    SendPacket(&data);
}

void WorldSession::HandleCharterTurnInCharter(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 charterGuid;
    recv_data >> charterGuid;
    Charter * pCharter = guildmgr.GetCharterByItemGuid(charterGuid);
    if(!pCharter)
        return;

    if(pCharter->CharterType == CHARTER_TYPE_GUILD)
    {
        Charter * gc = pCharter;
        if(gc == NULL)
            return;

        if( gc->GetLeader() != _player->GetLowGUID() )
            return;

        if(!_player->hasGMTag())
        {
            if(gc->SignatureCount < 9)
            {
                SendTurnInPetitionResult( this, ERR_PETITION_NOT_ENOUGH_SIGNATURES );
                return;
            }
        }

        // don't know hacky or not but only solution for now
        // If everything is fine create guild
        guildmgr.CreateGuildFromCharter(gc);

        // Destroy the charter
        _player->m_playerInfo->charterId[CHARTER_TYPE_GUILD] = 0;
        gc->Destroy();

        _player->GetInventory()->RemoveItemAmt(ITEM_ENTRY_GUILD_CHARTER, 1);
    }
    else
    {

    }

    SendTurnInPetitionResult( this, ERR_PETITION_OK );
}

void WorldSession::HandleCharterRename(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    std::string name;
    recv_data >> guid >> name;

    Charter * pCharter = guildmgr.GetCharterByItemGuid(guid);
    if(pCharter == 0)
        return;

    GuildInfo* gInfo = guildmgr.GetGuildByGuildName(name);
    Charter * charter = guildmgr.GetCharterByName(name, (CharterTypes)pCharter->CharterType);
    if(charter || gInfo)
    {
        SendNotification("That name is in use by another guild.");
        return;
    }

    charter = pCharter;
    charter->GuildName = name;
    charter->SaveToDB();

    WorldPacket data(MSG_PETITION_RENAME, 100);
    data << guid << name;
    SendPacket(&data);
}
