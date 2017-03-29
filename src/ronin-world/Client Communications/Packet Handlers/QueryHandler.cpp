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
/// This function handles CMSG_NAME_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleNameQueryOpcode( WorldPacket & recv_data )
{
    WoWGuid guid;
    recv_data >> guid;
    if(guid.getHigh() != HIGHGUID_TYPE_PLAYER)
        return;
    PlayerInfo *pn = objmgr.GetPlayerInfo(guid);
    if(pn == NULL)
        return;

    WorldPacket data;
    // We query our own name on player create so check to send MOTD
    if(guid == _player->GetGUID() && !_player->m_hasSentMoTD)
    {
        data.Initialize(SMSG_MOTD, 10);
        if(sWorld.BuildMoTDPacket(this, &data))
            SendPacket(&data);

        //Issue a message telling all guild members that this player has signed on
        guildmgr.PlayerLoggedIn(pn);

        _player->m_hasSentMoTD = true;
    }

    sLog.Debug("WorldSession","Received CMSG_NAME_QUERY for: %s", pn->charName.c_str() );
    data.Initialize(SMSG_NAME_QUERY_RESPONSE, 15+pn->charName.length());
    data << guid.asPacked();
    data << uint8(0);
    data << pn->charName;
    data << uint8(0);
    data << uint8(pn->charRace);
    data << uint8(pn->charAppearance3&0xFF);
    data << uint8(pn->charClass);
    data << uint8(0);
    SendPacket( &data );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_QUERY_TIME:
//////////////////////////////////////////////////////////////
void WorldSession::HandleQueryTimeOpcode( WorldPacket & recv_data )
{
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 8);
    data << uint32(UNIXTIME);
    data << uint32(14440);
    SendPacket(&data);
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_CREATURE_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleCreatureQueryOpcode( WorldPacket & recv_data )
{
    uint32 entry; WoWGuid guid;
    recv_data >> entry >> guid;

    WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 100);
    if(entry == 300000)
    {
        data << entry;
        data << "WayPoint" << uint8(0) << uint8(0) << uint8(0);
        data << "WayPoint" << uint8(0) << uint8(0) << uint8(0);
        data << "Level is WayPoint ID";
        data << uint8(0);
        for(uint32 i = 0; i < 11; i++)
            data << uint32(0);
        data << float(0.0f);
        data << float(0.0f);
        data << uint8(0);
        for(uint32 i = 0; i < 8; i++)
            data << uint32(0);
    }
    else if(CreatureData* ctrData = sCreatureDataMgr.GetCreatureData(entry))
    {
        data << entry;
        data << ctrData->maleName << uint8(0) << uint8(0) << uint8(0);
        data << ctrData->femaleName << uint8(0) << uint8(0) << uint8(0);
        data << ctrData->subName;
        data << ctrData->iconName;
        data << ctrData->flags;
        data << ctrData->flags2;
        data << ctrData->type;
        data << ctrData->family;
        data << ctrData->rank;
        data << ctrData->killCredit[0];
        data << ctrData->killCredit[1];
        data << ctrData->displayInfo[0];
        data << ctrData->displayInfo[1];
        data << ctrData->displayInfo[2];
        data << ctrData->displayInfo[3];
        data << ctrData->healthMod;
        data << ctrData->powerMod;
        data << ctrData->leader;
        for(uint8 i = 0; i < 6; i++)
            data << ctrData->questItems[i];
        data << ctrData->dbcMovementId;
        data << ctrData->expansionId;
        SendPacket( &data );
    } else data << uint32(entry | 0x80000000);
    SendPacket(&data);
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GAMEOBJECT_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleGameObjectQueryOpcode( WorldPacket & recv_data )
{
    uint32 entry; WoWGuid guid;
    recv_data >> entry >> guid;

    WorldPacket data(SMSG_GAMEOBJECT_QUERY_RESPONSE, 100);
    if(GameObjectInfo* goinfo = GameObjectNameStorage.LookupEntry(entry))
    {
        data << entry;
        data << goinfo->Type;
        data << goinfo->DisplayID;
        data << goinfo->Name << uint8(0) << uint8(0) << uint8(0);
        data << goinfo->Icon;
        data << goinfo->CastBarText;
        data << uint8(0);
        data.append(goinfo->data.raw.data, 32);
        data << float(goinfo->sizeMod);
        for(uint8 i = 0; i < 6; i++)
            data << uint32(goinfo->questItems[i]);
        data << uint32(0);
    } else data << uint32(entry | 0x80000000);
    SendPacket( &data );
}

void BuildCorpseInfo(WorldPacket* data, Corpse* corpse)
{
    *data << uint8(corpse == NULL ? 0 : 1);
    if(corpse != NULL)
    {
        float x, y, z;
        uint32 mapId = corpse->GetMapId(), mapId2 = mapId;
        corpse->GetPosition(x, y, z);

        *data << mapId;
        *data << x;
        *data << y;
        *data << z;
        *data << mapId2;
        *data << uint32(0);
    }
}

//////////////////////////////////////////////////////////////
/// This function handles MSG_CORPSE_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleCorpseQueryOpcode(WorldPacket &recv_data)
{
    sLog.outDebug("WORLD: Received MSG_CORPSE_QUERY");

    WorldPacket data(MSG_CORPSE_QUERY, 21);
    BuildCorpseInfo(&data, objmgr.GetCorpseByOwner(GetPlayer()->GetLowGUID()));
    SendPacket(&data);
}

void WorldSession::HandlePageTextQueryOpcode( WorldPacket & recv_data )
{
    uint32 pageid = 0;
    uint64 itemguid;
    recv_data >> pageid;
    recv_data >> itemguid;

    WorldPacket data(SMSG_PAGE_TEXT_QUERY_RESPONSE, 300);
    while(pageid)
    {
        ItemPage *page = ItemPageStorage.LookupEntry(pageid);
        data.clear();
        data << pageid;
        data << (page ? page->text : "Item page missing.");
        pageid = page ? page->next_page : 0;
        data << uint32(pageid);
        SendPacket(&data);
    }
}

void WorldSession::HandleQuestQueryOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    sLog.Debug( "WORLD"," Received CMSG_QUEST_QUERY" );

    uint32 quest_id;
    recv_data >> quest_id;

    Quest *qst = sQuestMgr.GetQuestPointer(quest_id);
    if (!qst)
    {
        sLog.outDebug("WORLD: Invalid quest ID.");
        return;
    }

    WorldPacket *pkt = BuildQuestQueryResponse(qst);
    SendPacket(pkt);
    delete pkt;
    sLog.Debug( "WORLD"," Sent SMSG_QUEST_QUERY_RESPONSE." );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_REQUEST_HOTFIX:
//////////////////////////////////////////////////////////////
void WorldSession::HandleItemHotfixQueryOpcode(WorldPacket & recvPacket)
{
    uint32 type, count;
    recvPacket >> type;
    if (type != 0x50238EC2 && type != 0x919BE54E)
    {
        sLog.outString("Client tried to request update item data from non-handled update type");
        return;
    }

    std::vector<uint8> masks;
    count = recvPacket.ReadBits(23);
    WoWGuid *guids = new WoWGuid[count];
    for (uint32 i = 0; i < count; ++i)
    {
        guids[i][0] = recvPacket.ReadBit();
        guids[i][4] = recvPacket.ReadBit();
        guids[i][7] = recvPacket.ReadBit();
        guids[i][2] = recvPacket.ReadBit();
        guids[i][5] = recvPacket.ReadBit();
        guids[i][3] = recvPacket.ReadBit();
        guids[i][6] = recvPacket.ReadBit();
        guids[i][1] = recvPacket.ReadBit();
    }

    uint32 item;
    for (uint32 c = 0; c < count; ++c)
    {
        recvPacket.ReadByteSeq(guids[c][5]);
        recvPacket.ReadByteSeq(guids[c][6]);
        recvPacket.ReadByteSeq(guids[c][7]);
        recvPacket.ReadByteSeq(guids[c][0]);
        recvPacket.ReadByteSeq(guids[c][1]);
        recvPacket.ReadByteSeq(guids[c][3]);
        recvPacket.ReadByteSeq(guids[c][4]);
        recvPacket >> item;
        recvPacket.ReadByteSeq(guids[c][2]);

        ItemPrototype* proto = sItemMgr.LookupEntry(item);
        WorldPacket data2(SMSG_DB_REPLY, 700);
        data2 << int32(proto ? item : -int32(item));
        data2 << uint32(type); // Needed?
        data2 << uint32(sWorld.GetStartTime());
        if (!proto) // Item does not exist
        {
            data2 << uint32(4); // sizeof(uint32)
            data2 << uint32(item | 0x80000000);
            SendPacket(&data2);
            continue;
        }
        else
        {
            ByteBuffer data;
            data << uint32(item);
            if (type == 0x50238EC2) // Update the base item shit
            {
                data << uint32(proto->Class);
                data << uint32(proto->SubClass);
                data << int32(proto->subClassSound);
                data << uint32(proto->LockMaterial);
                data << uint32(proto->DisplayInfoID);
                data << uint32(proto->InventoryType);
                data << uint32(proto->SheathID);
            }
            else if (type == 0x919BE54E) // Send more advanced shit
            {
                data << uint32(proto->Quality);
                data << uint32(proto->Flags);
                data << uint32(proto->FlagsExtra);
                data << float(0.f) << float(0.f);
                data << int32(proto->BuyCount);
                data << int32(proto->BuyPrice);
                data << uint32(proto->SellPrice);
                data << uint32(proto->InventoryType);
                data << int32(proto->AllowableClass);
                data << int32(proto->AllowableRace);
                data << uint32(proto->ItemLevel);
                data << uint32(proto->RequiredLevel);
                data << uint32(proto->RequiredSkill);
                data << uint32(proto->RequiredSkillRank);
                data << uint32(proto->RequiredSpell);
                data << uint32(proto->RequiredPlayerRank1);
                data << uint32(proto->RequiredPlayerRank2);
                data << uint32(proto->RequiredFaction);
                data << uint32(proto->RequiredFactionStanding);
                data << int32(proto->Unique);
                data << int32(proto->MaxCount);
                data << uint32(proto->ContainerSlots);

                for (uint32 x = 0; x < 10; ++x)
                    data << uint32(proto->Stats[x].Type);

                for (uint32 x = 0; x < 10; ++x)
                    data << int32(proto->Stats[x].Value);

                // Till here we are going good, now we start with the unk shit
                for (uint32 x = 0; x < 20; ++x) // 20 unk fields
                    data << uint32(0);

                data << uint32(proto->ScalingStatDistribution);
                data << uint32(proto->DamageType);
                data << uint32(proto->Delay);
                data << float(proto->Range);

                for (uint32 x = 0; x < 5; ++x)
                    data << int32(proto->Spells[x].Id);

                for (uint32 x = 0; x < 5; ++x)
                    data << uint32(proto->Spells[x].Trigger);

                for (uint32 x = 0; x < 5; ++x)
                    data << int32(proto->Spells[x].Charges);

                for (uint32 x = 0; x < 5; ++x)
                    data << int32(proto->Spells[x].Cooldown);

                for (uint32 x = 0; x < 5; ++x)
                    data << uint32(proto->Spells[x].Category);

                for (uint32 x = 0; x < 5; ++x)
                    data << int32(proto->Spells[x].CategoryCooldown);

                data << uint32(proto->Bonding);
                data << uint16(proto->Name.length());
                if (proto->Name.length())
                    data << proto->Name.c_str();

                for (uint32 i = 0; i < 3; ++i) // Other 3 names
                    data << uint16(0);

                std::string desc = proto->Description;
                data << uint16(desc.length());
                if (desc.length())
                    data << desc;

                data << uint32(proto->PageId);
                data << uint32(proto->PageLanguage);
                data << uint32(proto->PageMaterial);
                data << uint32(proto->QuestId);
                data << uint32(proto->LockId);
                data << int32(proto->LockMaterial);
                data << uint32(proto->SheathID);
                data << int32(proto->RandomPropId);
                data << int32(proto->RandomSuffixId);
                data << uint32(proto->ItemSet);

                data << uint32(proto->ZoneNameID);
                data << uint32(proto->MapID);
                data << uint32(proto->BagFamily);
                data << uint32(proto->TotemCategory);

                for (uint32 x = 0; x < 3; ++x)
                    data << uint32(proto->ItemSocket[x]);

                for (uint32 x = 0; x < 3; ++x)
                    data << uint32(proto->ItemContent[x]);

                data << uint32(proto->SocketBonus);
                data << uint32(proto->GemProperties);
                data << float(proto->ArmorDamageModifier);
                data << int32(proto->Duration);
                data << uint32(proto->ItemLimitCategory);
                data << uint32(proto->HolidayId);
                data << float(proto->StatScalingFactor); // StatScalingFactor
                data << uint32(0) << uint32(0);
            }

            data2 << uint32(data.size());
            data2.append(data.contents(), data.size());
        }

        data2 << uint32(type);
        SendPacket(&data2);
    }

    delete [] guids;
}

void WorldSession::HandleInrangeQuestgiverQuery(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    _player->ProcessVisibleQuestGiverStatus();
}