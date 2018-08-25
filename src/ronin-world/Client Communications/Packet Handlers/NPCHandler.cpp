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
/// This function handles MSG_TABARDVENDOR_ACTIVATE:
//////////////////////////////////////////////////////////////
void WorldSession::HandleTabardVendorActivateOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;
    Creature* pCreature = _player->GetMapInstance()->GetCreature(guid);
    if(!pCreature) return;

    SendTabardHelp(pCreature);
}

void WorldSession::SendTabardHelp(Creature* pCreature)
{
    CHECK_INWORLD_RETURN();
    WorldPacket data(8);
    data.Initialize( MSG_TABARDVENDOR_ACTIVATE );
    data << pCreature->GetGUID();
    SendPacket( &data );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_BANKER_ACTIVATE:
//////////////////////////////////////////////////////////////
void WorldSession::HandleBankerActivateOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapInstance()->GetCreature(guid);
    if(!pCreature) return;

    SendBankerList(pCreature);
}

void WorldSession::SendBankerList(Creature* pCreature)
{
    CHECK_INWORLD_RETURN();
    WorldPacket data(8);
    data.Initialize( SMSG_SHOW_BANK );
    data << pCreature->GetGUID();
    SendPacket( &data );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_TRAINER_LIST
//////////////////////////////////////////////////////////////
//NOTE: we select prerequirements for spell that TEACHES you
//not by spell that you learn!
void WorldSession::HandleTrainerListOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    // Inits, grab creature, check.
    uint64 guid;
    recv_data >> guid;
    if(Creature* train = GetPlayer()->GetMapInstance()->GetCreature(guid))
    {
        if(FactionEntry *faction = train->GetFaction())
            _player->Reputation_OnTalk(faction);

        WorldPacket data(SMSG_TRAINER_LIST, 5000);
        train->BuildTrainerData(&data, _player);
        SendPacket(&data);
    }
}

void WorldSession::HandleTrainerBuySpellOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();
    WoWGuid Guid;
    uint32 spellId, subCategory;
    recvPacket >> Guid >> subCategory >> spellId;
    if(Guid.getHigh() != HIGHGUID_TYPE_UNIT)
        return;
    Creature* pCreature = _player->GetInRangeObject<Creature>(Guid);
    if(pCreature == NULL || !pCreature->CanTrainPlayer(_player))
        return;
    if(pCreature->GetTrainerSubCategory() != subCategory)
        return;
    uint8 trainerCategory = pCreature->GetTrainerCategory();
    ObjectMgr::TrainerSpellMap *map = objmgr.GetTrainerSpells(trainerCategory, subCategory);
    if(map->find(spellId) == map->end())
        return;
    TrainerSpell *tspell = &map->at(spellId);
    if(_player->GetTrainerSpellStatus(tspell) != TRAINER_SPELL_AVAILABLE)
        return;
    _player->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -(int32)tspell->spellCost);

    pCreature->SendPlaySpellVisualKit(179, 0);  // 53 SpellCastDirected
    _player->SendPlaySpellVisualKit(362, 1);    // 113 EmoteSalute

    _player->addSpell(spellId);

    WorldPacket data(SMSG_TRAINER_BUY_SUCCEEDED, 12);
    data << uint64(Guid);
    data << uint32(spellId);
    SendPacket(&data);
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_PETITION_SHOWLIST:
//////////////////////////////////////////////////////////////
void WorldSession::HandleCharterShowListOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapInstance()->GetCreature(guid);
    if(!pCreature) return;

    SendCharterRequest(pCreature);
}

void WorldSession::SendCharterRequest(Creature* pCreature)
{
    CHECK_INWORLD_RETURN();
    if( !pCreature->ArenaOrganizersFlags() )
    {
        WorldPacket data(SMSG_PETITION_SHOWLIST, 81);
        uint8 tdata[73];
        static const uint8 temp[73] = { 0x03, 0x01, 0x00, 0x00, 0x00, 0x08, 0x5C, 0x00, 0x00, 0x21, 0x3F, 0x00, 0x00, 0x00, 0x35, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x09, 0x5C, 0x00, 0x00, 0x21, 0x3F, 0x00, 0x00, 0x80, 0x4F, 0x12, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0A, 0x5C, 0x00, 0x00, 0x21, 0x3F, 0x00, 0x00, 0x80, 0x84, 0x1E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00 };
        memcpy(tdata, temp, sizeof(temp));
        data << pCreature->GetGUID();
        data.append(tdata,73);
        SendPacket(&data);
    }
    else
    {
        WorldPacket data(SMSG_PETITION_SHOWLIST, 29);
        data << pCreature->GetGUID();
        data << uint8(1);          // BOOL SHOW_COST = 1
        data << uint32(1);        // unknown
        if(pCreature && pCreature->GetEntry()==19861 ||
            pCreature->GetEntry()==18897 || pCreature->GetEntry()==19856)
        {
            data << uint16(ARENA_TEAM_CHARTER_2v2);  // ItemId of the guild charter
        } else data << uint16(0x16E7);  // ItemId of the guild charter

        data << float(0.62890625);  // strange floating point
        data << uint16(0);          // unknown
        data << uint32(1000);       // charter price
        data << uint32(0);          // unknown, maybe charter type
        data << uint32(9);          // amount of unique players needed to sign the charter
        SendPacket( &data );
    }
}

//////////////////////////////////////////////////////////////
/// This function handles MSG_AUCTION_HELLO:
//////////////////////////////////////////////////////////////
void WorldSession::HandleAuctionHelloOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;
    Creature* auctioneer = _player->GetMapInstance()->GetCreature(guid);
    if(!auctioneer)
        return;

    SendAuctionList(auctioneer);
}

void WorldSession::SendAuctionList(Creature* auctioneer)
{
    AuctionHouse* AH = sAuctionMgr.GetAuctionHouse(auctioneer->GetEntry());
    if(!AH)
    {
        sChatHandler.BlueSystemMessage(this, "Report to devs: Unbound auction house npc %u.", auctioneer->GetEntry());
        return;
    }

    WorldPacket data(MSG_AUCTION_HELLO, 12);
    data << auctioneer->GetGUID();
    data << uint32(AH->GetID());
    data << uint8(sWorld.AHEnabled ? 1 : 0);

    SendPacket( &data );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GOSSIP_HELLO:
//////////////////////////////////////////////////////////////
void WorldSession::HandleGossipHelloOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    WoWGuid guid;
    recv_data >> guid;

    Object* obj = NULL;
    if(guid.getHigh() == HIGHGUID_TYPE_ITEM) // Crow: Could possibly do GetObject because I don't think we need items...
        obj = _player->GetInventory()->GetInventoryItem(guid);
    else obj = _player->GetInRangeObject(guid);
    if(obj == NULL)
        return;

    WorldPacket data(SMSG_GOSSIP_MESSAGE, 500);
    sGossipMgr.BuildGossipMessage(&data, _player, obj);
    SendPacket(&data);
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GOSSIP_SELECT_OPTION:
//////////////////////////////////////////////////////////////
void WorldSession::HandleGossipSelectOptionOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    WoWGuid guid;
    recv_data >> guid;

    Object* obj = NULL;
    if(guid.getHigh() == HIGHGUID_TYPE_ITEM) // Crow: Could possibly do GetObject because I don't think we need items...
        obj = _player->GetInventory()->GetInventoryItem(guid);
    else obj = _player->GetInRangeObject(guid);
    if(obj == NULL)
        return;
    uint32 menuId, gossipOption;
    recv_data >> menuId >> gossipOption;
    std::string coded;
    if(recv_data.rpos() != recv_data.size())
        recv_data >> coded;

    sGossipMgr.HandleGossipOptionSelect(menuId, gossipOption, _player, obj, coded.c_str());
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_SPIRIT_HEALER_ACTIVATE:
//////////////////////////////////////////////////////////////
void WorldSession::HandleSpiritHealerActivateOpcode( WorldPacket & recv_data )
{
    if(!_player->IsInWorld() ||!_player->isDead())
        return;

    //No nonsense for GM's
    if(GetPermissionCount() == 0)
    {
        //25% duralbility loss
        _player->DeathDurabilityLoss(0.25f);

        //When revived by spirit healer, set health/mana at 50%
        _player->m_resurrectHealth = _player->GetUInt32Value(UNIT_FIELD_MAXHEALTH)/2;

        _player->ResurrectPlayer();

        if(_player->getLevel() > 10)
        {
            if(Aura* aur = _player->m_AuraInterface.FindAura(15007)) // If the player already have the aura, just extend it.
                aur->ResetExpirationTime();
            else
            {
                SpellCastTargets targets;
                targets.m_unitTarget = _player->GetGUID();
                if(Spell* sp = new Spell(_player, dbcSpell.LookupEntry(15007)))
                    sp->prepare(&targets, true);
            }
        }
    } else _player->ResurrectPlayer();
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_NPC_TEXT_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleNpcTextQueryOpcode( WorldPacket & recv_data )
{
    uint32 textID;
    WoWGuid targetGuid;
    recv_data >> textID >> targetGuid;
    sLog.Debug("WORLD","CMSG_NPC_TEXT_QUERY ID '%u'", textID );

    GetPlayer()->SetUInt64Value(UNIT_FIELD_TARGET, targetGuid);

    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 50000);
    data << textID;
    if(GossipText *pGossip = (textID == 68 ? NULL : NpcTextStorage.LookupEntry(textID)))
    {
        for(uint8 i = 0; i < 8; i++)
        {
            data << float(pGossip->Infos[i].Prob);
            if(strlen(pGossip->Texts[i].Text[0]) == 0)
                data << pGossip->Texts[i].Text[1];
            else
                data << pGossip->Texts[i].Text[0];

            if(strlen(pGossip->Texts[i].Text[1]) == 0)
                data << pGossip->Texts[i].Text[0];
            else
                data << pGossip->Texts[i].Text[1];
            data << pGossip->Infos[i].Lang;

            for(uint8 e = 0; e < 3; e++)
            {
                data << uint32(pGossip->Infos[i].Delay[e]);
                data << uint32(pGossip->Infos[i].Emote[e]);
            }
        }
    }
    else
    {
        data << float(1.0f);        // Prob
        data << (textID != 68 ? "Hello, $N. What can I do for you?" : "Hi there, how can I help you $N"); // Team
        data << (textID != 68 ? "Hello, $N. What can I do for you?" : "Greetings, $N"); // Team
        data << uint32(0x00);       // Language

        for(uint8 e = 0; e < 3; e++)
        {
            data << uint32(0x00);       // Emote delay
            data << uint32(0x00);       // Emote
        }

        for(uint8 i = 1; i < 8; i++)
        {
            data << float(1.0f);        // Prob
            data << uint8(0x00) << uint8(0x00); // Team
            data << uint32(0x00);       // Language

            for(uint8 e = 0; e < 3; e++)
            {
                data << uint32(0x00);       // Emote delay
                data << uint32(0x00);       // Emote
            }
        }
    }

    SendPacket(&data);
}

void WorldSession::HandleBinderActivateOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;

    Creature* pC = _player->GetMapInstance()->GetCreature(guid);
    if(!pC)
        return;

    SendInnkeeperBind(pC);
    _player->bHasBindDialogOpen = false;
}

void WorldSession::SendInnkeeperBind(Creature* pCreature)
{
    CHECK_INWORLD_RETURN();
    WorldPacket data(SMSG_GOSSIP_COMPLETE, 0);
    SendPacket(&data);

    static SpellEntry *updateBind = dbcSpell.LookupEntry(3286);
    if(updateBind == NULL)
        return;

    if(!_player->bHasBindDialogOpen)
    {
        data.Initialize(SMSG_BINDER_CONFIRM);
        data << pCreature->GetGUID() << pCreature->GetZoneId();
        SendPacket(&data);

        _player->bHasBindDialogOpen = true;
        return;
    }

    pCreature->GetSpellInterface()->TriggerSpell(updateBind, _player);

    data.Initialize(SMSG_BINDPOINTUPDATE);
    data << _player->GetBindPositionX() << _player->GetBindPositionY() << _player->GetBindPositionZ() << _player->GetBindMapId() << _player->GetBindZoneId();
    SendPacket( &data );

    data.Initialize(SMSG_PLAYERBOUND);
    data << pCreature->GetGUID() << _player->GetBindZoneId();
    SendPacket(&data);
}

void WorldSession::SendSpiritHealerRequest(Creature* pCreature)
{
    WorldPacket data(SMSG_SPIRIT_HEALER_CONFIRM, 8);
    data << pCreature->GetGUID();
    SendPacket(&data);
}

void WorldSession::HandleListInventoryOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    sLog.Debug( "WORLD"," Recvd CMSG_LIST_INVENTORY" );
    uint64 guid;
    recv_data >> guid;

    Creature* unit = _player->GetMapInstance()->GetCreature(guid);
    if (unit == NULL)
        return;

    if(FactionEntry *faction = unit->GetFaction())
        _player->Reputation_OnTalk(faction);
    unit->SendInventoryList(_player);
}
