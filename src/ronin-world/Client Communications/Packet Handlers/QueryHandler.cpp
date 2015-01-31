/***
 * Demonstrike Core
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

    // We query our own name on player create so check to send MOTD
    if(!_player->sentMOTD)
    {
        _player->sendMOTD();
        _player->sentMOTD = true;
    }

    sLog.Debug("WorldSession","Received CMSG_NAME_QUERY for: %s", pn->name );
    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, 15+strlen(pn->name));
    data << guid.asPacked();
    data << uint8(0);
    data << pn->name;
    data << uint8(0);
    data << uint8(pn->race);
    data << uint8(pn->gender);
    data << uint8(pn->_class);
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
    uint32 entry;
    uint64 guid;

    recv_data >> entry;
    recv_data >> guid;

    WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 150);
    if(entry == 300000)
    {
        data << (uint32)entry;
        data << "WayPoint" << uint8(0) << uint8(0) << uint8(0);
        data << "Level is WayPoint ID";
        data << uint8(0);
        for(uint32 i = 0; i < 10; i++)
            data << uint32(0);
        data << float(0.0f);
        data << float(0.0f);
        data << uint8(0);
        for(uint32 i = 0; i < 8; i++)
            data << uint32(0);
        SendPacket( &data );
        return;
    }

    if(CreatureData* ctrData = sCreatureDataMgr.GetCreatureData(entry))
    {
        data << entry;
        data << ctrData->Name << uint8(0) << uint8(0) << uint8(0);
        data << ctrData->Name << uint8(0) << uint8(0) << uint8(0);
        data << ctrData->SubName;
        data << ctrData->InfoStr;
        data << ctrData->Flags;
        data << uint32(0);
        data << ctrData->Type;
        data << ctrData->Family;
        data << ctrData->Rank;
        data << ctrData->KillCredit[0];
        data << ctrData->KillCredit[1];
        data << ctrData->DisplayInfo[0];
        data << ctrData->DisplayInfo[1];
        data << ctrData->DisplayInfo[2];
        data << ctrData->DisplayInfo[3];
        data << ctrData->HealthMod;
        data << ctrData->PowerMod;
        data << ctrData->Leader;
        uint8 index = 0;
        std::vector<uint32>* objQuestLoot = lootmgr.GetCreatureQuestLoot(entry);
        for(auto itr = objQuestLoot->begin(); itr != objQuestLoot->end(); itr++)
        {
            data << (*itr);
            if(++index == 6)
                break;
        }
        for(uint8 i = index; i < 6; i++)
            data << uint32(0);
        data << uint32(0);  // CreatureMovementInfo.dbc
        data << uint32(0);
        SendPacket( &data );
    }
    else
    {
        data << uint32(entry | 0x80000000);
        SendPacket(&data);
    }
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GAMEOBJECT_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleGameObjectQueryOpcode( WorldPacket & recv_data )
{
    uint32 entryID;
    uint64 guid;
    recv_data >> entryID;
    recv_data >> guid;

    sLog.Debug("WORLD","HandleGameObjectQueryOpcode CMSG_GAMEOBJECT_QUERY '%u'", entryID);

    GameObjectInfo* goinfo = GameObjectNameStorage.LookupEntry(entryID);
    if(goinfo == NULL)
        return;

    WorldPacket data(SMSG_GAMEOBJECT_QUERY_RESPONSE, 150);
    data << entryID;
    data << goinfo->Type;
    data << goinfo->DisplayID;
    data << goinfo->Name << uint8(0) << uint8(0) << uint8(0);
    data << goinfo->Icon;
    data << goinfo->CastBarText;
    data << uint8(0);
    for(uint32 d = 0; d < 32; d++)
        data << goinfo->RawData.ListedData[d];
    data << float(1.f);
    uint8 index = 0;
    std::vector<uint32>* objQuestLoot = lootmgr.GetGameObjectQuestLoot(entryID);
    for(auto itr = objQuestLoot->begin(); itr != objQuestLoot->end(); itr++)
    {
        data << (*itr);
        if(++index == 6)
            break;
    }
    for(uint8 i = index; i < 6; i++)
        data << uint32(0);
    data << uint32(0);
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
        if(MapInfo *pMapinfo = WorldMapInfoStorage.LookupEntry(corpse->GetMapId()))
        {
            if(pMapinfo->type != INSTANCE_NULL && pMapinfo->type != INSTANCE_PVP)
            {
                mapId = pMapinfo->repopmapid;
                x = pMapinfo->repopx;
                y = pMapinfo->repopy;
                z = pMapinfo->repopz;
            }
        }
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
        data.clear();
        ItemPage * page = ItemPageStorage.LookupEntry(pageid);
        if(page == NULL)
            return;

        char* text = page->text;
        if(text == NULL || *text == NULL)
            return;

        data << pageid;
        data << text;
        data << page->next_page;
        pageid = page->next_page;
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
    uint32 count, type;
    recvPacket >> count >> type;
    if (type != 0x50238EC2 && type != 0x919BE54E)
    {
        sLog.outString("Client tried to request update item data from non-handled update type");
        return;
    }

    for (uint32 i = 0; i < count; ++i)
    {
        uint32 item;
        recvPacket >> item;
        recvPacket.read_skip(8);

        WorldPacket data2(SMSG_DB_REPLY, 700);
        data2 << uint32(type); // Needed?
        data2 << uint32(item);
        ItemPrototype* proto = sItemMgr.LookupEntry(item);
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

                data << uint32(0);
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

                std::string name = proto->Name1; // Item name
                data << uint16(name.length());
                if (name.length())
                    data << name;

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
                data << uint32(proto->MaxDurability);

                data << uint32(proto->ZoneNameID);
                data << uint32(proto->MapID);
                data << uint32(proto->BagFamily);
                data << uint32(0);

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
                data << uint32(0);
                data << uint32(0);
            }

            data2 << uint32(data.size());
            data2 << data;
        }

        data2 << uint32(type);
        SendPacket(&data2);
    }
}

void WorldSession::HandleInrangeQuestgiverQuery(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    WorldPacket data(SMSG_QUESTGIVER_STATUS_MULTIPLE, 1000);
    uint32 count = 0;
    data << count;
    for(WorldObject::InRangeUnitSet::iterator itr = _player->GetInRangeUnitSetBegin(); itr != _player->GetInRangeUnitSetEnd(); itr++ )
    {
        if( (*itr)->GetTypeId() != TYPEID_UNIT )
            continue;

        Creature* pCreature = castPtr<Creature>(*itr);
        if( pCreature->isQuestGiver() )
        {
            data << pCreature->GetGUID();
            data << sQuestMgr.CalcStatus(pCreature, GetPlayer());
            ++count;
        }
    }

    data.put<uint32>(0, count);
    SendPacket(&data);
}