/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleGuildQuery(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 4);

    uint32 guildId;
    recv_data >> guildId;

    guildmgr.Packet_SendGuildQuery(this, guildId);
}

void WorldSession::HandleInviteToGuild(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 1);

    std::string inviteeName;
    recv_data >> inviteeName;

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
    CHECK_PACKET_SIZE(recv_data, 1);
    std::string NewGuildInfo;
    recv_data >> NewGuildInfo;

    guildmgr.Packet_SetGuildInformation(this, NewGuildInfo);
}

void WorldSession::HandleGuildInfo(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_SendGuildInformation(this);
}

void WorldSession::HandleGuildRoster(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_SendGuildRoster(this);
}

void WorldSession::HandleGuildPromote(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 1);

    std::string promoteeName;
    recv_data >> promoteeName;

    guildmgr.Packet_PromoteGuildMember(this, promoteeName);
}

void WorldSession::HandleGuildDemote(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 1);

    std::string demoteeName;
    recv_data >> demoteeName;

    guildmgr.Packet_DemoteGuildMember(this, demoteeName);
}

void WorldSession::HandleGuildLeave(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.RemoveMember(_player, _player->getPlayerInfo());
}

void WorldSession::HandleGuildRemove(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 1);

    std::string name;
    recv_data >> name;

    PlayerInfo * dstplr = objmgr.GetPlayerInfoByName(name.c_str());
    if( dstplr == NULL )
        return;

    guildmgr.RemoveMember(_player, dstplr);
}

void WorldSession::HandleGuildDisband(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_DisbandGuild(this);
}

void WorldSession::HandleGuildLeader(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 1);

    std::string name;
    recv_data >> name;
    PlayerInfo * dstplr = objmgr.GetPlayerInfoByName(name.c_str());
    if( dstplr == NULL )
        return;

    guildmgr.Packet_ChangeGuildLeader(this, dstplr);
}

void WorldSession::HandleGuildMotd(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    std::string motd = "";
    if(recv_data.size())
        recv_data >> motd;

    guildmgr.Packet_SetMotd(this, motd);
}

void WorldSession::HandleGuildEditRank(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 9);

    string newName;
    uint32 rankId, RankRights, iFlags[MAX_GUILD_BANK_TABS];
    int32 DailyGoldLimit, iStacksPerDay[MAX_GUILD_BANK_TABS];

    recv_data >> rankId >> RankRights >> newName;
    recv_data >> DailyGoldLimit;

    for(uint8 i = 0; i < MAX_GUILD_BANK_TABS; i++)
    {
        recv_data >> iFlags[i];
        recv_data >> iStacksPerDay[i];
    }

    guildmgr.Packet_HandleEditRank(this, newName, rankId, RankRights, DailyGoldLimit, iFlags, iStacksPerDay);
}

void WorldSession::HandleGuildAddRank(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    string rankName;
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

void WorldSession::HandleGuildSetPublicNote(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    string target, newnote;
    recv_data >> target >> newnote;

    guildmgr.Packet_SetPublicNote(this, target, newnote);
}

void WorldSession::HandleGuildSetOfficerNote(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    string target, newnote;
    recv_data >> target >> newnote;

    guildmgr.Packet_SetOfficerNote(this, target, newnote);
}

void WorldSession::HandleSaveGuildEmblem(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 28);
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

    string name;
    uint64 creature_guid;
    uint32 arena_index, PetitionSignerCount;

    recv_data >> creature_guid;
    recv_data.read_skip<uint32>();
    recv_data.read_skip<uint64>();
    recv_data >> name;
    recv_data.read_skip<string>();
    for(uint32 s = 0; s < 7; ++s)
        recv_data.read_skip<uint32>();
    recv_data.read_skip<uint16>();
    recv_data.read_skip<uint64>();
    recv_data >> PetitionSignerCount;
    for(uint32 s = 0; s < 10; ++s)
        recv_data.read_skip<string>();
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
    string tabname, tabicon;

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

void WorldSession::HandleGuildGetFullPermissions(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    guildmgr.Packet_SendGuildPermissions(this);
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
    uint64 guid;
    uint8 tabid;
    recv_data >> guid;
    recv_data >> tabid;
    recv_data.read_skip<uint8>();

    guildmgr.Packet_SendGuildBankTab(this, tabid);
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
    string tabtext;
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
        if(c->Signatures[j] == 0)
            continue;

        data << uint64(c->Signatures[j]) << uint32(1);
    }
    data << uint8(0);
    p->GetSession()->SendPacket(&data);
}

void WorldSession::HandleCharterShowSignatures(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    Charter * pCharter;
    uint64 item_guid;
    recv_data >> item_guid;
    pCharter = guildmgr.GetCharterByItemGuid(GUID_LOPART(item_guid));

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

    Charter * c = guildmgr.GetCharterByItemGuid(GUID_LOPART(item_guid));
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

    uint32 maxlevel = sWorld.GetMaxLevel(_player);
    if(maxlevel < 80)
        maxlevel = 80;
    if( c->CharterType == CHARTER_TYPE_GUILD )
        data << uint32(1) << uint32(maxlevel);              // 10 minlevel
    else
        data << uint32(80) << uint32(maxlevel);             // 10

    data << uint32(0);                                      // 12
    data << uint32(0);                                      // 13 count of next strings?
    data << uint32(0);                                      // 14
    data << uint32(0);                                      // 15
    data << uint16(0);                                      // 16

    if (c->CharterType == CHARTER_TYPE_GUILD)
        data << uint32(0);
    else
        data << uint32(1);

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
    Player* pTarget = _player->GetMapMgr()->GetPlayer((uint32)target_guid);
    pCharter = guildmgr.GetCharterByItemGuid(GUID_LOPART(item_guid));

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

    Charter * c = guildmgr.GetCharterByItemGuid(GUID_LOPART(item_guid));
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

    Player * l = _player->GetMapMgr()->GetPlayer(c->GetLeader());
    if(l == 0)
        return;

    WorldPacket data(SMSG_PETITION_SIGN_RESULTS, 100);
    data << item_guid << _player->GetGUID() << uint32(0);
    l->GetSession()->SendPacket(&data);
    data.clear();
    data << item_guid << (uint64)c->GetLeader() << uint32(0);
    SendPacket(&data);
}

void WorldSession::HandleCharterTurnInCharter(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 charterGuid;
    recv_data >> charterGuid;
    Charter * pCharter = guildmgr.GetCharterByItemGuid(GUID_LOPART(charterGuid));
    if(!pCharter)
        return;

    if(pCharter->CharterType == CHARTER_TYPE_GUILD)
    {
        Charter * gc = pCharter;
        if(gc == NULL)
            return;

        if( gc->GetLeader() != _player->GetLowGUID() )
            return;

        if(!_player->bGMTagOn)
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

        _player->GetItemInterface()->RemoveItemAmt(ITEM_ENTRY_GUILD_CHARTER, 1);
    }
    else
    {
        /* Arena charter - TODO: Replace with correct messages */
        ArenaTeam * team;
        uint32 type;
        uint32 i;
        uint32 icon, iconcolor, bordercolor, border, background;
        recv_data >> iconcolor >>icon >> bordercolor >> border >> background;

        switch(pCharter->CharterType)
        {
        case CHARTER_TYPE_ARENA_2V2:
            type = ARENA_TEAM_TYPE_2V2;
            break;

        case CHARTER_TYPE_ARENA_3V3:
            type = ARENA_TEAM_TYPE_3V3;
            break;

        case CHARTER_TYPE_ARENA_5V5:
            type = ARENA_TEAM_TYPE_5V5;
            break;

        default:
            SendNotification("Internal Error");
            return;
        }

        if( pCharter->GetLeader() != _player->GetLowGUID() )
            return;

        if(_player->m_playerInfo->arenaTeam[pCharter->CharterType-1] != NULL)
        {
            sChatHandler.SystemMessage(this, "You are already in an arena team.");
            return;
        }

        if(pCharter->SignatureCount < pCharter->GetNumberOfSlotsByType())
        {
            SendTurnInPetitionResult( this, ERR_PETITION_NOT_ENOUGH_SIGNATURES );
            return;
        }

        team = new ArenaTeam(type, objmgr.GenerateArenaTeamId());
        team->m_name = pCharter->GuildName;
        team->m_emblemColour = iconcolor;
        team->m_emblemStyle = icon;
        team->m_borderColour = bordercolor;
        team->m_borderStyle = border;
        team->m_backgroundColour = background;
        team->m_leader=_player->GetLowGUID();
        team->m_stat_rating=1500;

        objmgr.AddArenaTeam(team);
        objmgr.UpdateArenaTeamRankings();
        team->AddMember(_player->m_playerInfo);


        /* Add the members */
        for(i = 0; i < pCharter->SignatureCount; i++)
        {
            PlayerInfo * info = objmgr.GetPlayerInfo(pCharter->Signatures[i]);
            if(info)
            {
                team->AddMember(info);
            }
        }

        _player->GetItemInterface()->SafeFullRemoveItemByGuid(charterGuid);
        _player->m_playerInfo->charterId[pCharter->CharterType] = NULL;
        pCharter->Destroy();
    }

    SendTurnInPetitionResult( this, ERR_PETITION_OK );
}

void WorldSession::HandleCharterRename(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    string name;
    recv_data >> guid >> name;

    Charter * pCharter = guildmgr.GetCharterByItemGuid(GUID_LOPART(guid));
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
