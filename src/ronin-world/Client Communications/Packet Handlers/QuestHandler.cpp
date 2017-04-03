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
initialiseSingleton( QuestMgr );

void WorldSession::HandleQuestgiverStatusQueryOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_QUESTGIVER_STATUS_QUERY." );

    CHECK_INWORLD_RETURN();

    uint64 guid;
    Object* qst_giver = NULL;

    recv_data >> guid;
    uint32 guidtype = GUID_HIPART(guid);
    if(guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = _player->GetMapInstance()->GetCreature(guid);
        if(quest_giver)
            qst_giver = quest_giver;
        else
            return;

        if (!quest_giver->isQuestGiver())
        {
            sLog.outDebug("WORLD: Creature is not a questgiver.");
            return;
        }
    }
    else if(guidtype==HIGHGUID_TYPE_ITEM)
    {
        if(Item* quest_giver = GetPlayer()->GetInventory()->GetInventoryItem(guid))
            qst_giver = quest_giver;
        else return;
    }
    else if(guidtype==HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = _player->GetMapInstance()->GetGameObject(guid);
        if(quest_giver)
            qst_giver = quest_giver;
        else
            return;
    }

    if (!qst_giver)
    {
        sLog.outDebug("WORLD: Invalid questgiver GUID %ull.", guid);
        return;
    }

    WorldPacket data(SMSG_QUESTGIVER_STATUS, 12);
    data << guid << sQuestMgr.CalcStatus(qst_giver, GetPlayer());
    SendPacket( &data );
}

void WorldSession::HandleQuestgiverHelloOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_QUESTGIVER_HELLO." );
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    recv_data >> guid;

    Creature* qst_giver = _player->GetMapInstance()->GetCreature(guid);

    if (!qst_giver)
    {
        sLog.outDebug("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!qst_giver->isQuestGiver())
    {
        sLog.outDebug("WORLD: Creature is not a questgiver.");
        return;
    }

    sQuestMgr.OnActivateQuestGiver(qst_giver, GetPlayer());
}

void WorldSession::HandleQuestGiverQueryQuestOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_QUESTGIVER_QUERY_QUEST." );
    CHECK_INWORLD_RETURN();

    WorldPacket data;
    uint64 guid;
    uint32 quest_id;
    uint32 status = 0;

    recv_data >> guid;
    recv_data >> quest_id;

    Object* qst_giver = NULL;

    bool bValid = false;
    Quest* qst = sQuestMgr.GetQuestPointer(quest_id);
    if (!qst)
    {
        sLog.outDebug("WORLD: Invalid quest ID.");
        return;
    }

    uint32 guidtype = GUID_HIPART(guid);
    if(guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = _player->GetMapInstance()->GetCreature(guid);
        if(quest_giver)
            qst_giver = quest_giver;
        else
            return;
        bValid = quest_giver->isQuestGiver();
        if(bValid)
            status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id), false);
    }
    else if(guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = _player->GetMapInstance()->GetGameObject(guid);
        if(quest_giver)
            qst_giver = quest_giver;
        else
            return;
        bValid = quest_giver->isQuestGiver();
        if(bValid)
            status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id), false);
    }
    else if(guidtype==HIGHGUID_TYPE_ITEM)
    {
        if(Item* quest_giver = GetPlayer()->GetInventory()->GetInventoryItem(guid))
        {
            if(qst->id == quest_giver->GetProto()->QuestId)
            {
                bValid = true;
                qst_giver = quest_giver;
                status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, 1, false);
            }
        }
    }

    if (!qst_giver)
    {
        sLog.outDebug("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!bValid)
    {
        sLog.outDebug("WORLD: object is not a questgiver.");
        return;
    }

    if(status == QMGR_QUEST_NOT_FINISHED || status == QMGR_QUEST_FINISHED)
    {
        if(qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE)
            sQuestMgr.BuildOfferReward(&data, qst, qst_giver, 1, _player);
        else
            sQuestMgr.BuildRequestItems(&data, qst, qst_giver, status);
        SendPacket(&data);
        sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_REQUEST_ITEMS." );
    }
    else if (status == QMGR_QUEST_CHAT || status == QMGR_QUEST_AVAILABLE)
    {
        sQuestMgr.BuildQuestDetails(&data, qst, qst_giver, 1, _player);
        SendPacket(&data);
        sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS." );
    }
}

void WorldSession::HandleQuestgiverAcceptQuestOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_QUESTGIVER_ACCEPT_QUEST" );
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    uint32 quest_id;
    uint32 unk;

    recv_data >> guid;
    recv_data >> quest_id;
    recv_data >> unk;

    bool bValid = false;
    bool hasquest = true;
    bool bSkipLevelCheck = false;
    Quest *qst = NULL;
    Object* qst_giver = NULL;
    uint32 guidtype = guid.getHigh();

    if(guidtype == HIGHGUID_TYPE_UNIT)
    {
        if(Creature* quest_giver = _player->GetMapInstance()->GetCreature(guid))
        {
            qst_giver = quest_giver;
            bValid = quest_giver->isQuestGiver();
            hasquest = quest_giver->HasQuest(quest_id, 1);
            if(bValid)
                qst = sQuestMgr.GetQuestPointer(quest_id);
        }
        else return;
    }
    else if(guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        if(GameObject* quest_giver = _player->GetMapInstance()->GetGameObject(guid))
            qst_giver = quest_giver;
        else return;
        //bValid = quest_giver->isQuestGiver();
        //if(bValid)
        bValid = true;
        qst = sQuestMgr.GetQuestPointer(quest_id);
    }
    else if(guidtype == HIGHGUID_TYPE_ITEM)
    {
        if(Item* quest_giver = GetPlayer()->GetInventory()->GetItemByGUID(guid))
        {
            qst_giver = quest_giver;
            bValid = true;
            bSkipLevelCheck = true;
            qst = sQuestMgr.GetQuestPointer(quest_id);
            if( qst && qst->id != quest_giver->GetProto()->QuestId )
                return;
        }
        else return;
    }
    else if(guidtype == HIGHGUID_TYPE_PLAYER)
    {
        if(Player* quest_giver = _player->GetMapInstance()->GetPlayer(guid))
            qst_giver = quest_giver;
        else return;
        bValid = true;
        qst = sQuestMgr.GetQuestPointer(quest_id);
    }

    if (!qst_giver)
    {
        sLog.outDebug("WORLD: Invalid questgiver GUID.");
        return;
    }

    if( !bValid || qst == NULL )
    {
        sLog.outDebug("WORLD: Creature is not a questgiver.");
        return;
    }

    if( _player->GetQuestLogForEntry( qst->id ) )
        return;

    // Check the player hasn't already taken this quest, or
    // it isn't available.
    uint32 status = sQuestMgr.CalcQuestStatus(_player, qst, 3, bSkipLevelCheck);
    if((!sQuestMgr.IsQuestRepeatable(qst) && _player->HasFinishedQuest(qst->id)) || ( status != QMGR_QUEST_CHAT && status != QMGR_QUEST_AVAILABLE ) || !hasquest)
    {
        // We've got a hacker. Disconnect them.
        //sWorld.LogCheater(this, "tried to accept incompatible quest %u from %u.", qst->id, qst_giver->GetEntry());
        //Disconnect();
        SystemMessage("Internal quest error 3");
        return;
    }

    int32 log_slot = _player->GetOpenQuestSlot();
    if (log_slot == -1)
    {
        sQuestMgr.SendQuestLogFull(GetPlayer());
        return;
    }

    if(qst->count_receiveitems || qst->srcitem)
    {
        uint32 slots_required = qst->count_receiveitems;

        if(_player->GetInventory()->CalculateFreeSlots(NULL) < slots_required)
        {
            _player->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_BAG_FULL);
            sQuestMgr.SendQuestFailed(FAILED_REASON_INV_FULL, qst, _player);
            return;
        }
    }

    QuestLogEntry *qle = new QuestLogEntry();
    qle->Init(qst, _player, log_slot);
    qle->UpdatePlayerFields();

    if(qst->count_required_item || qst_giver->GetTypeId() == TYPEID_GAMEOBJECT) // gameobject quests deactivate
        _player->UpdateNearbyGameObjects();

    sQuestMgr.OnQuestAccepted(_player,qst,qst_giver);
}

void WorldSession::HandleQuestlogRemoveQuestOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    sLog.Debug( "QuestHandler","Received CMSG_QUESTLOG_REMOVE_QUEST" );

    uint8 quest_slot;
    recvPacket >> quest_slot;
    if(quest_slot >= 25)
        return;

    QuestLogEntry *qEntry = _player->GetQuestLogInSlot(quest_slot);
    if (!qEntry)
    {
        sLog.Debug("QuestHandler","No quest in slot %d.", quest_slot);
        return;
    }
    Quest *qPtr = qEntry->GetQuest();

    if (!qPtr)
    {
        sLog.Debug("QuestHandler","Quest %u does not exist in database", qPtr->id);
        return;
    }

    TRIGGER_QUEST_EVENT(qPtr->id, OnQuestCancel)(_player);

    qEntry->Finish();

    // Remove all items given by the questgiver at the beginning
    uint32 srcItem = qPtr->srcitem;
    for(uint32 i = 0; i < 4; i++)
        if(qPtr->receive_items[i] && qPtr->receive_items[i] != srcItem)
            _player->GetInventory()->RemoveItemAmt( qPtr->receive_items[i], qPtr->receive_itemcount[i] );

    // Remove source item
    if(qPtr->srcitem)
        _player->GetInventory()->RemoveItemAmt( qPtr->srcitem, 1 );

    // Reset timed quests, remove timed event
    // always remove collected items (need to be recollectable again in case of repeatable).
    for( uint32 y = 0; y < 6; y++)
        if( qPtr->required_item[y] && qPtr->required_item[y] != srcItem )
            _player->GetInventory()->RemoveItemAmt(qPtr->required_item[y], qPtr->required_itemcount[y]);

    _player->ProcessVisibleQuestGiverStatus();
    _player->UpdateNearbyGameObjects();

    _player->SaveToDB(false);
}

void WorldSession::HandleQuestgiverRequestRewardOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    sLog.Debug( "WORLD"," Received CMSG_QUESTGIVER_REQUESTREWARD_QUEST." );

    uint64 guid;
    uint32 quest_id;

    recv_data >> guid;
    recv_data >> quest_id;

    bool bValid = false;
    Quest *qst = NULL;
    WorldObject* qst_giver = NULL;
    uint32 status = 0;
    uint32 guidtype = GUID_HIPART(guid);

    if(guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = _player->GetMapInstance()->GetCreature(guid);
        if(quest_giver)
            qst_giver = quest_giver;
        else
            return;
        bValid = quest_giver->isQuestGiver();
        if(bValid)
        {
            qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
            if(!qst)
                qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_START);

            /*if(!qst)
                qst = sQuestMgr.GetQuestPointer(quest_id);*/
            if(!qst)
            {
                sLog.outDebug("WARNING: Cannot complete quest, as it doesnt exist.");
                return;
            }
            status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id),false);
        }
    }
    else if(guidtype==HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = _player->GetMapInstance()->GetGameObject(guid);
        if(quest_giver)
            qst_giver = quest_giver;
        else return; // oops..

        bValid = quest_giver->isQuestGiver();
        if(bValid)
        {
            qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
            /*if(!qst) sQuestMgr.FindQuest(quest_id);*/
            if(!qst)
            {
                sLog.outDebug("WARNING: Cannot complete quest, as it doesnt exist.");
                return;
            }
            status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id),false);
        }
    }

    if (!qst_giver)
    {
        sLog.outDebug("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!bValid || qst == NULL)
    {
        sLog.outDebug("WORLD: Creature is not a questgiver.");
        return;
    }

    if (status >= QMGR_QUEST_FINISHED_LOWLEVEL)
    {
        WorldPacket data;
        sQuestMgr.BuildOfferReward(&data, qst, qst_giver, 1, _player);
        SendPacket(&data);
        sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_REQUEST_ITEMS." );
    }

    // if we got here it means we're cheating
}

void WorldSession::HandleQuestgiverCompleteQuestOpcode( WorldPacket & recvPacket )
{
    CHECK_INWORLD_RETURN();
    sLog.Debug( "WORLD"," Received CMSG_QUESTGIVER_COMPLETE_QUEST." );

    uint64 guid;
    uint32 quest_id;

    recvPacket >> guid;
    recvPacket >> quest_id;

    bool bValid = false;
    Quest *qst = NULL;
    WorldObject* qst_giver = NULL;
    uint32 status = 0;
    uint32 guidtype = GUID_HIPART(guid);

    if(guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = _player->GetMapInstance()->GetCreature(guid);
        if(quest_giver)
            qst_giver = quest_giver;
        else
            return;
        bValid = quest_giver->isQuestGiver();
        if(bValid)
        {
            qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
            /*if(!qst)
                sQuestMgr.FindQuest(quest_id);*/
            if(!qst)
            {
                sLog.outDebug("WARNING: Cannot complete quest, as it doesnt exist.");
                return;
            }
            status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id),false);
        }
    }
    else if(guidtype==HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = _player->GetMapInstance()->GetGameObject(guid);
        if(quest_giver)
            qst_giver = quest_giver;
        else
            return; // oops..
        bValid = quest_giver->isQuestGiver();
        if(bValid)
        {
            qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
            /*if(!qst) sQuestMgr.FindQuest(quest_id);*/
            if(!qst)
            {
                sLog.outDebug("WARNING: Cannot complete quest, as it doesnt exist.");
                return;
            }
            status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id),false);
        }
    }

    if (!qst_giver)
    {
        sLog.outDebug("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!bValid || qst == NULL)
    {
        sLog.outDebug("WORLD: Creature is not a questgiver.");
        return;
    }

    if (status >= QMGR_QUEST_FINISHED_LOWLEVEL || qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE)
    {
        WorldPacket data;
        sQuestMgr.BuildOfferReward(&data, qst, qst_giver, 1, _player);
        SendPacket(&data);
        sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_REQUEST_ITEMS." );
    }
    else if (status == QMGR_QUEST_NOT_FINISHED || qst->qst_is_repeatable)
    {
        WorldPacket data;
        sQuestMgr.BuildRequestItems(&data, qst, qst_giver, status);
        SendPacket(&data);
        sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_REQUEST_ITEMS." );
    }
}

void WorldSession::HandleQuestgiverChooseRewardOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    sLog.Debug( "WORLD"," Received CMSG_QUESTGIVER_CHOOSE_REWARD." );

    uint64 guid;
    uint32 quest_id;
    uint32 reward_slot;

    recvPacket >> guid;
    recvPacket >> quest_id;
    recvPacket >> reward_slot;

    if( reward_slot >= 6 )
        return;

    bool bValid = false, bRegularGossip = true;
    Quest *qst = NULL;
    WorldObject* qst_giver = NULL;
    uint32 guidtype = GUID_HIPART(guid);

    if(guidtype == HIGHGUID_TYPE_UNIT)
    {
        Creature* quest_giver = _player->GetMapInstance()->GetCreature(guid);
        if(quest_giver)
            qst_giver = quest_giver;
        else
            return;

        bValid = quest_giver->isQuestGiver();
        if(bValid)
            qst = sQuestMgr.GetQuestPointer(quest_id);
    }
    else if(guidtype==HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* quest_giver = _player->GetMapInstance()->GetGameObject(guid);
        if(quest_giver)
            qst_giver = quest_giver;
        else
            return;
        //bValid = quest_giver->isQuestGiver();
        //if(bValid)
        bValid = true;
        qst = sQuestMgr.GetQuestPointer(quest_id);
    }

    if (!qst_giver)
    {
        sLog.outDebug("WORLD: Invalid questgiver GUID.");
        return;
    }

    if (!bValid || qst == NULL)
    {
        sLog.outDebug("WORLD: Creature is not a questgiver.");
        return;
    }

    //FIXME: Some Quest givers talk in the end of the quest.
    //   qst_giver->SendChatMessage(CHAT_MSG_MONSTER_SAY,LANG_UNIVERSAL,qst->GetQuestEndMessage().c_str());
    QuestLogEntry *qle = _player->GetQuestLogForEntry(quest_id);
    if (!qle && !qst->qst_is_repeatable && !(qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE))
    {
        sLog.outDebug("WORLD: QuestLogEntry not found.");
        return;
    }

    if (qle && !qle->CanBeFinished())
    {
        sLog.outDebug("WORLD: Quest not finished.");
        return;
    }

    //check for room in inventory for all items
    if(!sQuestMgr.CanStoreReward(_player,qst,reward_slot))
    {
        sQuestMgr.SendQuestFailed(FAILED_REASON_INV_FULL, qst, _player);
        return;
    }

    sQuestMgr.OnQuestFinished(_player, qst, qst_giver, reward_slot);

    if(qst->qst_next_quest_id)
    {
        WorldPacket data(CMSG_QUESTGIVER_QUERY_QUEST, 12);
        data << guid << qst->qst_next_quest_id;
        HandleQuestGiverQueryQuestOpcode(data);
    } else if(bRegularGossip == false)
        OutPacket(SMSG_GOSSIP_COMPLETE);
}

void WorldSession::HandlePushQuestToPartyOpcode(WorldPacket &recv_data)
{
    CHECK_INWORLD_RETURN();
    uint32 questid, status;
    recv_data >> questid;

    sLog.Debug( "WORLD"," Received CMSG_PUSHQUESTTOPARTY quest = %u", questid );

    Quest *pQuest = sQuestMgr.GetQuestPointer(questid);
    if(pQuest)
    {
        Group *pGroup = _player->GetGroup();
        if(pGroup)
        {
            uint32 pguid = _player->GetLowGUID();
            SubGroup * sgr = _player->GetGroup() ?  _player->GetGroup()->GetSubGroup(_player->GetSubGroup()) : 0;

            if(sgr)
            {
                _player->GetGroup()->Lock();
                GroupMembersSet::iterator itr;
                for(itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); itr++)
                {
                    Player* pPlayer = (*itr)->m_loggedInPlayer;
                    if(pPlayer && pPlayer->GetLowGUID() !=  pguid)
                    {
                        WorldPacket data( MSG_QUEST_PUSH_RESULT, 13 );
                        data << pPlayer->GetGUID();
                        data << uint32(QUEST_SHARE_MSG_SHARING_QUEST);
                        data << uint8(0);
                        _player->PushPacket(&data);

                        uint32 response = 0;
                        //CHECKS IF CAN TAKE THE QUEST
                        status = sQuestMgr.PlayerMeetsReqs(pPlayer, pQuest, false);
                        if(status != QMGR_QUEST_CHAT && status != QMGR_QUEST_AVAILABLE)
                            response = QUEST_SHARE_MSG_CANT_TAKE_QUEST;

                        //CHECKS IF QUESTLOG ISN'T FULL
                        if(pPlayer->GetOpenQuestSlot() == -1)
                            response = QUEST_SHARE_MSG_LOG_FULL;

                        //CHEQUEA SI TIENE LA QUEST EN EL QUESTLOG | CHEKS IF HAVE ALREADY THE QUEST IN QUESTLOG
                        QuestLogEntry *qst = pPlayer->GetQuestLogForEntry(questid);
                        if(qst)
                            response = QUEST_SHARE_MSG_HAVE_QUEST;

                        //CHECKS IF ALREADY HAVE COMPLETED THE QUEST
                        if(pPlayer->HasFinishedQuest(questid))
                            response = QUEST_SHARE_MSG_FINISH_QUEST;

                        //CHECKS IF  ALREADY HAVE COMPLETED THE DAILY QUEST
                        if(pPlayer->HasFinishedDailyQuest(questid))
                            response = QUEST_SHARE_MSG_CANT_SHARE_TODAY;

                        //CHECKS IF IS IN THE PARTY
                        if(!pGroup)
                            response = QUEST_SHARE_MSG_NOT_IN_PARTY;

                        //CHECKS IF IS BUSY
                        if(pPlayer->DuelingWith != NULL)
                            response = QUEST_SHARE_MSG_BUSY;

                        if(response > 0)
                        {
                            sQuestMgr.SendPushToPartyResponse(_player, pPlayer, response);
                            continue;
                        }

                        data.clear();
                        sQuestMgr.BuildQuestDetails(&data, pQuest, pPlayer, 1, _player);
                        pPlayer->PushPacket(&data);
                        pPlayer->SetQuestSharer(pguid);
                    }
                }
                _player->GetGroup()->Unlock();
            }
        }
    }
}

void WorldSession::HandleQuestPushResult(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    uint8 msg;
    recvPacket >> guid >> msg;

    sLog.Debug( "WORLD"," Received MSG_QUEST_PUSH_RESULT " );

    if(GetPlayer()->GetQuestSharer())
    {
        Player* pPlayer = objmgr.GetPlayer(GetPlayer()->GetQuestSharer());
        if(pPlayer)
        {
            WorldPacket data(MSG_QUEST_PUSH_RESULT, 13);
            data << guid;
            data << uint32(msg);
            data << uint8(0);
            pPlayer->PushPacket(&data);
            GetPlayer()->SetQuestSharer(0);
        }
    }
}

void WorldSession::HandleQuestPOI(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();
    uint32 count;
    recvPacket >> count;

    if (count >= 50)
        return;

    sLog.Debug( "WORLD"," Received MSG_QUEST_PUSH_RESULT " );

    WorldPacket data(SMSG_QUEST_POI_QUERY_RESPONSE, 4+(4+4)*count);
    data << uint32(count);

    for (uint i = 0; i < count; ++i)
    {
        uint32 questId;
        recvPacket >> questId;

        bool questOk = false;

        uint16 questSlot = _player->FindQuestSlot(questId);

        if (questSlot != 50)
            questOk =_player->GetQuestSlotId(questSlot) == questId;

        if (questOk)
        {
            if (Quest *quest = sQuestMgr.GetQuestPointer(questId))
            {
                data << uint32(questId);
                data << uint32(quest->quest_poi.size());

                for (std::vector<QuestPOI*>::const_iterator itr = quest->quest_poi.begin(); itr != quest->quest_poi.end(); ++itr)
                {
                    data << uint32((*itr)->PoIID);
                    data << int32((*itr)->questObjectIndex);
                    data << uint32((*itr)->mapId);
                    data << uint32((*itr)->areaId);
                    data << uint32((*itr)->MapFloorId);
                    data << uint32(0) << uint32(1);
                    data << uint32((*itr)->points.size());

                    for (std::vector<std::pair<int32, int32>>::const_iterator itr2 = (*itr)->points.begin(); itr2 != (*itr)->points.end(); ++itr2)
                    {
                        data << int32(itr2->first);
                        data << int32(itr2->second);
                    }
                }
            }
            else
            {
                data << uint32(questId);
                data << uint32(0);
            }
        }
        else
        {
            data << uint32(questId);
            data << uint32(0);
        }
    }
    SendPacket(&data);
}
