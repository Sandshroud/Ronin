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

QuestMgr::QuestMgr()
{

}

QuestMgr::~QuestMgr()
{
    QuestStorageMap::iterator MapQuestIterator;
    QuestStorageMap::iterator itr1;
    QuestRelationListMap::iterator itr2;
    QuestRelationList::iterator itr3;
    QuestAssociationListMap::iterator itr4;
    QuestAssociationList::iterator itr5;
    std::list<uint32>::iterator itr6;

    // clear relations
    for(itr2 = m_obj_quests.begin(); itr2 != m_obj_quests.end(); itr2++)
    {
        if(!itr2->second)
            continue;

        itr3 = itr2->second->begin();
        for(; itr3 != itr2->second->end(); itr3++)
        {
            delete (*itr3);
        }
        itr2->second->clear();
        delete itr2->second;
    }
    m_obj_quests.clear();

    for(itr2 = m_npc_quests.begin(); itr2 != m_npc_quests.end(); itr2++)
    {
        if(!itr2->second)
            continue;

        itr3 = itr2->second->begin();
        for(; itr3 != itr2->second->end(); itr3++)
        {
            delete (*itr3);
        }
        itr2->second->clear();
        delete itr2->second;
    }
    m_npc_quests.clear();

    for(itr2 = m_itm_quests.begin(); itr2 != m_itm_quests.end(); itr2++)
    {
        if(!itr2->second)
            continue;

        itr3 = itr2->second->begin();
        for(; itr3 != itr2->second->end(); itr3++)
        {
            delete (*itr3);
        }
        itr2->second->clear();
        delete itr2->second;
    }
    m_itm_quests.clear();

    for(MapQuestIterator = QuestStorage.begin(); MapQuestIterator != QuestStorage.end(); MapQuestIterator++)
    {
        free(MapQuestIterator->second->qst_title);
        free(MapQuestIterator->second->qst_details);
        free(MapQuestIterator->second->qst_objectivetext);
        free(MapQuestIterator->second->qst_completiontext);
        free(MapQuestIterator->second->qst_finishedtext);
        free(MapQuestIterator->second->qst_endtext);
        free(MapQuestIterator->second->qst_incompletetext);
        for(uint8 i = 0; i < 4; i++)
            free(MapQuestIterator->second->qst_objectivetexts[i]);
        MapQuestIterator->second->quest_poi.clear();
        delete MapQuestIterator->second;
    }
    QuestStorage.clear();

    for (std::vector<QuestPOI*>::const_iterator itr = m_questPOI.begin(); itr != m_questPOI.end(); ++itr)
    {
        (*itr)->points.clear();
        delete *itr;
    }
    m_questPOI.clear();
}

void QuestMgr::LoadQuests()
{
    LoadLocks.Acquire();
    QueryResult* mainResult = WorldDatabase.Query("SELECT * FROM quest_data");
    if(mainResult == NULL)
    {
        sLog.Notice("QuestMgr", "No quests found in the quests table!");
        return;
    }

    do
    {
        uint8 f = 0;
        Field *fields = mainResult->Fetch();
        uint32 QuestId = fields[f++].GetUInt32();
        if(QuestStorage.find(QuestId) != QuestStorage.end())
        {
            sLog.Error("QuestMgr", "Duplicate quest data found in DB for quest %u, skipping\n", QuestId);
            continue;
        }

        Quest *newQuest = new Quest();
        memset(newQuest, 0, sizeof(Quest));
        newQuest->id = QuestId;
        newQuest->qst_title = strdup(fields[f++].GetString());
        newQuest->qst_details = strdup(fields[f++].GetString());
        newQuest->qst_objectivetext = strdup(fields[f++].GetString());
        newQuest->qst_completiontext = strdup(fields[f++].GetString());
        newQuest->qst_finishedtext = strdup(fields[f++].GetString());
        newQuest->qst_endtext = strdup(fields[f++].GetString());
        newQuest->qst_incompletetext = strdup(fields[f++].GetString());
        for(uint8 i = 0; i < 4; i++)
            newQuest->qst_objectivetexts[i] = strdup(fields[f++].GetString());
        newQuest->qst_zone_id = fields[f++].GetUInt32();
        newQuest->qst_accept_type = 2; // Default for now
        newQuest->qst_sort = fields[f++].GetUInt32();
        newQuest->qst_type = fields[f++].GetUInt32();
        newQuest->qst_flags = fields[f++].GetUInt32();
        newQuest->qst_flags &= ~QUEST_FLAG_AUTO_ACCEPT;
        newQuest->qst_min_level = fields[f++].GetUInt32();
        newQuest->qst_max_level = fields[f++].GetUInt32();
        newQuest->qst_suggested_players = fields[f++].GetUInt32();
        newQuest->qst_previous_quest_id = fields[f++].GetUInt32();
        newQuest->qst_next_quest_id = fields[f++].GetUInt32();
        newQuest->qst_start_phase = fields[f++].GetInt32();
        newQuest->qst_complete_phase = fields[f++].GetInt32();
        newQuest->qst_is_repeatable = fields[f++].GetUInt8();
        QuestStorage.insert(std::make_pair(QuestId, newQuest));
    }while(mainResult->NextRow());
    delete mainResult;

    sLog.Notice("QuestMgr", "Loaded %u Quests from the Database! Starting data pooling...", QuestStorage.size());

    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM quest_objectives"))
    {
        if(result->GetFieldCount() != 40)
            sLog.Error("QuestMgr", "Incorrect column count in quest_rewards(%u/40)", result->GetFieldCount());
        else do
        {
            uint8 f = 0;
            Field *fields = result->Fetch();
            uint32 QuestId = fields[f++].GetUInt32();
            if(QuestStorage.find(QuestId) == QuestStorage.end())
                continue;

            Quest *quest = QuestStorage.at(QuestId);
            for(uint8 i = 0; i < 6; i++)
                if(quest->required_item[i] = fields[f++].GetUInt32())
                    quest->count_required_item++;

            for(uint8 i = 0; i < 6; i++)
                quest->required_itemcount[i] = fields[f++].GetUInt16();

            for(uint8 i = 0; i < 4; i++)
                if(quest->required_mob[i] = fields[f++].GetUInt32())
                    quest->count_required_mob++;

            for(uint8 i = 0; i < 4; i++)
                quest->required_mobtype[i] = fields[f++].GetUInt8();
            for(uint8 i = 0; i < 4; i++)
                quest->required_mobcount[i] = fields[f++].GetUInt16();
            for(uint8 i = 0; i < 4; i++)
                quest->required_spell[i] = fields[f++].GetUInt32();

            for(uint8 i = 0; i < 4; i++)
                if(quest->required_areatriggers[i] = fields[f++].GetUInt32())
                    quest->count_requiredareatriggers++;

            quest->required_player_kills = fields[f++].GetUInt32();
            quest->required_timelimit = fields[f++].GetUInt32();
            quest->required_money = fields[f++].GetUInt32();
            quest->required_point_mapid = fields[f++].GetUInt32();
            quest->required_point_x = fields[f++].GetFloat();
            quest->required_point_y = fields[f++].GetFloat();
            quest->required_point_radius = fields[f++].GetUInt32();
        }while(result->NextRow());
        delete result;
    }

    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM quest_requirements"))
    {
        if(result->GetFieldCount() != 13)
            sLog.Error("QuestMgr", "Incorrect column count in quests_rewards(%u/13)", result->GetFieldCount());
        else do
        {
            uint8 f = 0;
            Field *fields = result->Fetch();
            uint32 QuestId = fields[f++].GetUInt32();
            if(QuestStorage.find(QuestId) == QuestStorage.end())
                continue;

            Quest *quest = QuestStorage.at(QuestId);
            quest->required_team = fields[f++].GetInt8();
            quest->required_races = fields[f++].GetUInt16();
            quest->required_class = fields[f++].GetUInt16();
            quest->required_tradeskill = fields[f++].GetUInt16();
            quest->required_tradeskill_value = fields[f++].GetUInt16();
            quest->required_rep_faction = fields[f++].GetUInt16();
            quest->required_rep_value = fields[f++].GetUInt16();
            for(uint8 i = 0; i < 4; i++)
                if(quest->required_quests[i] = fields[f++].GetUInt32())
                    quest->count_requiredquests = i+1;
            quest->required_quest_one_or_all = fields[f++].GetBool();
        }while(result->NextRow());
        delete result;
    }

    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM quest_rewards"))
    {
        if(result->GetFieldCount() != 55)
            sLog.Error("QuestMgr", "Incorrect column count in quest_rewards(%u/55)", result->GetFieldCount());
        else do
        {
            uint8 f = 0;
            Field *fields = result->Fetch();
            uint32 QuestId = fields[f++].GetUInt32();
            if(QuestStorage.find(QuestId) == QuestStorage.end())
                continue;

            Quest *quest = QuestStorage.at(QuestId);
            quest->srcitem = fields[f++].GetUInt32();
            quest->srcitemcount = fields[f++].GetUInt16();
            for(uint8 i = 0; i < 6; i++)
                if(quest->reward_choiceitem[i] = fields[f++].GetUInt32())
                    quest->count_reward_choiceitem++;
            for(uint8 i = 0; i < 6; i++)
                quest->reward_choiceitemcount[i] = fields[f++].GetUInt16();

            for(uint8 i = 0; i < 4; i++)
                if(quest->reward_item[i] = fields[f++].GetUInt32())
                    quest->count_reward_item++;
            for(uint8 i = 0; i < 4; i++)
                quest->reward_itemcount[i] = fields[f++].GetUInt16();

            for(uint8 i = 0; i < 4; i++)
                if(quest->receive_items[i] = fields[f++].GetUInt32())
                    quest->count_receiveitems++;
            for(uint8 i = 0; i < 4; i++)
                quest->receive_itemcount[i] = fields[f++].GetUInt16();

            for(uint8 i = 0; i < 5; i++)
                quest->reward_repfaction[i] = fields[f++].GetUInt32();
            for(uint8 i = 0; i < 5; i++)
                quest->reward_repvalue[i] = fields[f++].GetInt16();
            for(uint8 i = 0; i < 5; i++)
                quest->reward_replimit[i] = fields[f++].GetInt16();

            quest->reward_title = fields[f++].GetUInt8();
            quest->reward_money = fields[f++].GetUInt32();
            quest->reward_maxlvlmoney = quest->reward_money;
            f++; f++;
            quest->reward_xp = fields[f++].GetUInt32();
            quest->reward_spell = fields[f++].GetUInt32();
            quest->reward_talents = fields[f++].GetUInt16();
            quest->reward_cast_on_player = fields[f++].GetUInt32();
            quest->reward_xp_index = fields[f++].GetUInt32();
        }while(result->NextRow());
        delete result;
    }

    if(QueryResult *pResult = WorldDatabase.Query("SELECT * FROM quest_starter_creature"))
    {
        do
        {
            Field *data = pResult->Fetch();
            if(Quest *qst = GetQuestPointer(data[1].GetUInt32()))
                _AddQuest<Creature>(data[0].GetUInt32(), qst, QUESTGIVER_QUEST_START);
        } while(pResult->NextRow());
        delete pResult;
    }

    if(QueryResult *pResult = WorldDatabase.Query("SELECT * FROM quest_starter_gameobject"))
    {
        do
        {
            Field *data = pResult->Fetch();
            if(Quest *qst = GetQuestPointer(data[1].GetUInt32()))
                _AddQuest<GameObject>(data[0].GetUInt32(), qst, QUESTGIVER_QUEST_START);
        } while(pResult->NextRow());
        delete pResult;
    }

    if(QueryResult *pResult = WorldDatabase.Query("SELECT * FROM quest_finisher_creature"))
    {
        do
        {
            Field *data = pResult->Fetch();
            if(Quest *qst = GetQuestPointer(data[1].GetUInt32()))
                _AddQuest<Creature>(data[0].GetUInt32(), qst, QUESTGIVER_QUEST_END);
        } while(pResult->NextRow());
        delete pResult;
    }

    if(QueryResult *pResult = WorldDatabase.Query("SELECT * FROM quest_finisher_gameobject"))
    {
        do
        {
            Field *data = pResult->Fetch();
            if(Quest *qst = GetQuestPointer(data[1].GetUInt32()))
                _AddQuest<GameObject>(data[0].GetUInt32(), qst, QUESTGIVER_QUEST_END);
        } while(pResult->NextRow());
        delete pResult;
    }

    std::map<std::pair<uint32, uint32>, QuestPOI*> m_orderedQuestPOI;
    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM quest_poi ORDER BY questId, id ASC"))
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 QuestId = fields[0].GetUInt32();
            if(QuestStorage.find(QuestId) == QuestStorage.end())
                continue;

            Quest *quest = QuestStorage.at(QuestId);
            QuestPOI *PoI = new QuestPOI();
            PoI->questId = QuestId;
            PoI->PoIID = fields[1].GetUInt32();
            PoI->questObjectIndex = fields[2].GetInt32();
            PoI->mapId = fields[3].GetUInt32();
            PoI->areaId = fields[4].GetUInt32();
            PoI->MapFloorId = fields[5].GetUInt32();
            m_questPOI.push_back(PoI);
            quest->quest_poi.push_back(PoI);
            m_orderedQuestPOI[std::make_pair(PoI->questId, PoI->PoIID)] = PoI;
        } while (result->NextRow());
        delete result;    
    }

    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM quest_poi_points ORDER BY questId, poiId, internalIndex ASC"))
    {
        do
        {
            Field *pointFields = result->Fetch();
            std::pair<uint32, uint32> questPOID = std::make_pair(pointFields[0].GetUInt32(), pointFields[1].GetUInt32());
            if(m_orderedQuestPOI.find(questPOID) != m_orderedQuestPOI.end())
                m_orderedQuestPOI.at(questPOID)->points.push_back(std::make_pair(pointFields[3].GetInt32(), pointFields[4].GetInt32()));
        }while (result->NextRow());
        delete result;
    }

    sLog.Notice("QuestMgr", "%u quest POI definitions", m_orderedQuestPOI.size());
    m_orderedQuestPOI.clear();
    LoadLocks.Release();
}

void QuestMgr::AppendQuestList(Object *obj, Player *plr, uint32 &count, WorldPacket *packet)
{
    QuestRelationListMap *map;
    switch(obj->GetHighGUID())
    {
    case HIGHGUID_TYPE_ITEM: map = &m_itm_quests; break;
    case HIGHGUID_TYPE_UNIT: map = &m_npc_quests; break;
    case HIGHGUID_TYPE_GAMEOBJECT: map = &m_obj_quests; break;
    }

    QuestRelationListMap::iterator itr;
    if(map && ((itr = map->find(obj->GetEntry())) != map->end()) && !itr->second->empty())
    {   // Process through our quest relation list and build our quest list packet and increment counter
        for(QuestRelationList::iterator listItr = itr->second->begin(); listItr != itr->second->end(); ++listItr)
        {
            uint32 status = CalcQuestStatus(plr, *listItr);
            if(status <= QMGR_QUEST_AVAILABLELOW_LEVEL)
                continue;

            BuildGossipQuest(packet, (*listItr)->qst, status, plr);
            ++count;
        }
    }

    // Map specific quests occur here so only allow world objects
    if(!(obj->IsObject() && obj->IsInWorld()))
        return;
    // Our map instances have their own guidlist so we have different map lists and event data affects lists as well
    castPtr<WorldObject>(obj)->GetMapInstance()->AppendQuestList(obj->GetGUID(), plr, count, packet);
}

uint32 QuestMgr::CalcQuestStatus(Player* plr, QuestRelation* qst)
{
    return CalcQuestStatus(plr, qst->qst, qst->type);
}

// Crow: ALL NOT AVAILABLES MUST GO FIRST!
uint32 QuestMgr::PlayerMeetsReqs(Player* plr, Quest* qst, bool skiplevelcheck, bool skipprevquestcheck)
{
    std::list<uint32>::iterator itr;
    uint32 status = QMGR_QUEST_AVAILABLE;

    // We can skip previous quest check if we're gaining our status beforehand
    if(skipprevquestcheck == false && qst->qst_previous_quest_id && !(plr->HasFinishedQuest(qst->qst_previous_quest_id)))
        return QMGR_QUEST_NOT_AVAILABLE;

    if(qst->required_team >= 0 && qst->required_team != plr->GetTeam())
        return QMGR_QUEST_NOT_AVAILABLE;

    if(qst->required_class && !(qst->required_class & plr->getClassMask()))
        return QMGR_QUEST_NOT_AVAILABLE;

    if(qst->required_races && !(qst->required_races & plr->getRaceMask()))
        return QMGR_QUEST_NOT_AVAILABLE;

    if(qst->required_tradeskill)
    {
        if(!plr->HasSkillLine(qst->required_tradeskill))
            return QMGR_QUEST_NOT_AVAILABLE;
        if (qst->required_tradeskill_value && plr->getSkillLineVal(qst->required_tradeskill) < qst->required_tradeskill_value)
            return QMGR_QUEST_NOT_AVAILABLE;
    }

    // Check reputation
    if(qst->required_rep_faction && qst->required_rep_value)
        if(plr->GetFactionInterface()->GetStanding(qst->required_rep_faction) < (int32)qst->required_rep_value)
            return QMGR_QUEST_NOT_AVAILABLE;

    //Do we just need to complete one, or all quest requisitions?
    if( !qst->required_quest_one_or_all )
    {
        for(uint32 i = 0; i < qst->count_requiredquests; i++)
        {
            // Skip ourselves or previous quest since it's already been looked at.
            if(qst->required_quests[i] == qst->id || qst->required_quests[i] == qst->qst_previous_quest_id)
                continue;

            if (qst->required_quests[i] > 0 && !( plr->HasFinishedQuest(qst->required_quests[i]) || plr->HasFinishedDailyQuest(qst->required_quests[i])))
                return QMGR_QUEST_NOT_AVAILABLE;
        }
    }
    else
    {
        bool check_req = false;
        for(uint32 i = 0; i < qst->count_requiredquests; i++)
            if(plr->HasFinishedQuest(qst->required_quests[i]) || plr->HasFinishedDailyQuest(qst->required_quests[i]))
                check_req = true;

        if(!check_req)
            return QMGR_QUEST_NOT_AVAILABLE;
    }

    if (plr->HasFinishedQuest(qst->id) && !qst->qst_is_repeatable)
        return QMGR_QUEST_NOT_AVAILABLE;

    if (plr->HasFinishedDailyQuest(qst->id))
        return QMGR_QUEST_NOT_AVAILABLE;

    if (plr->getLevel() < qst->qst_min_level && !skiplevelcheck)
        return QMGR_QUEST_AVAILABLELOW_LEVEL;

    if(qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE)
        return QMGR_QUEST_FINISHED;

    // check quest level
    if( plr->getLevel() >= ( qst->qst_max_level + 5 ) )
        return QMGR_QUEST_CHAT;
    return status;
}

uint32 QuestMgr::CalcQuestStatus(Player* plr, Quest* qst, uint8 type, bool skiplevelcheck, bool skipPrevQuestCheck)
{
    QuestLogEntry* qle = plr->GetQuestLogForEntry(qst->id);
    if (!qle)
    {
        if (type & QUESTGIVER_QUEST_START)
            return PlayerMeetsReqs(plr, qst, skiplevelcheck, skipPrevQuestCheck);
    }
    else
    {
//      if( qle->HasFailed() )
//          return QMGR_QUEST_NOT_FINISHED;

        if(type & QUESTGIVER_QUEST_END)
        {
            if(!qle->CanBeFinished())
                return QMGR_QUEST_NOT_FINISHED;
            if( plr->getLevel() >= ( qst->qst_max_level + 5 ) )
                return QMGR_QUEST_FINISHED_LOWLEVEL;
            return QMGR_QUEST_FINISHED;
        }
    }

    return QMGR_QUEST_NOT_AVAILABLE;
}

uint32 QuestMgr::CalcStatus(Object* quest_giver, Player* plr)
{
    bool bValid = false;
    uint32 status = QMGR_QUEST_NOT_AVAILABLE;
    std::list<QuestRelation *>::const_iterator itr, q_begin, q_end;
    if( quest_giver->GetTypeId() == TYPEID_GAMEOBJECT )
    {
        if(bValid = castPtr<GameObject>(quest_giver)->HasQuests())
        {
            q_begin = castPtr<GameObject>(quest_giver)->QuestsBegin();
            q_end = castPtr<GameObject>(quest_giver)->QuestsEnd();
        }
    }
    else if( quest_giver->GetTypeId() == TYPEID_UNIT )
    {
        if(bValid = castPtr<Creature>( quest_giver )->HasQuests())
        {
            q_begin = castPtr<Creature>(quest_giver)->QuestsBegin();
            q_end = castPtr<Creature>(quest_giver)->QuestsEnd();
        }
    }
    else if( quest_giver->GetTypeId() == TYPEID_ITEM )
    {
        if( castPtr<Item>( quest_giver )->GetProto()->QuestId )
            bValid = true;
    } //This will be handled at quest share so nothing important as status
    else if(quest_giver->IsPlayer())
        status = QMGR_QUEST_AVAILABLE;
    if(bValid == false)
        return status;

    if(quest_giver->GetTypeId() == TYPEID_ITEM)
    {
        Quest *pQuest = GetQuestPointer( castPtr<Item>(quest_giver)->GetProto()->QuestId );
        QuestRelation qr;
        qr.qst = pQuest;
        qr.type = 1;

        uint32 tmp_status = CalcQuestStatus(plr, &qr);
        if(tmp_status > QMGR_QUEST_CHAT && pQuest->qst_flags & QUEST_FLAG_AUTOCOMPLETE)
            tmp_status = QMGR_QUEST_AVAILABLE;
        if(tmp_status > status)
            status = tmp_status;
    }

    for(itr = q_begin; itr != q_end; itr++)
    {
        uint32 tmp_status = CalcQuestStatus(plr, *itr); // save a call
        if(tmp_status > QMGR_QUEST_CHAT && (*itr)->qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE)
            tmp_status = QMGR_QUEST_AVAILABLE;
        if (tmp_status > status)
            status = tmp_status;
    }

    return status;
}

uint32 QuestMgr::ActiveQuestsCount(Object* quest_giver, Player* plr)
{
    std::list<QuestRelation *>::const_iterator itr;
    std::map<uint32, uint8> tmp_map;
    uint32 questCount = 0;

    std::list<QuestRelation *>::const_iterator q_begin, q_end;
    bool bValid = false;

    if(quest_giver->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        bValid = castPtr<GameObject>(quest_giver)->HasQuests();
        if(bValid)
        {
            q_begin = castPtr<GameObject>(quest_giver)->QuestsBegin();
            q_end   = castPtr<GameObject>(quest_giver)->QuestsEnd();

        }
    }
    else if(quest_giver->GetTypeId() == TYPEID_UNIT)
    {
        bValid = castPtr<Creature>(quest_giver)->HasQuests();
        if(bValid)
        {
            q_begin = castPtr<Creature>(quest_giver)->QuestsBegin();
            q_end   = castPtr<Creature>(quest_giver)->QuestsEnd();
        }
    }

    if(!bValid)
    {
        sLog.outDebug("QUESTS: Warning, invalid NPC %llu specified for ActiveQuestsCount. TypeId: %d.", quest_giver->GetGUID(), quest_giver->GetTypeId());
        return 0;
    }

    for(itr = q_begin; itr != q_end; itr++)
    {
        if (CalcQuestStatus(plr, *itr) >= QMGR_QUEST_CHAT)
        {
            if (tmp_map.find((*itr)->qst->id) == tmp_map.end())
            {
                tmp_map.insert(std::map<uint32,uint8>::value_type((*itr)->qst->id, 1));
                questCount++;
            }
        }
    }

    return questCount;
}

void QuestMgr::BuildGossipQuest(WorldPacket *data, Quest *qst, uint32 quest_status, Player *plr)
{
    *data << uint32(qst->id);
    *data << uint32((quest_status == QMGR_QUEST_NOT_FINISHED || quest_status >= QMGR_QUEST_FINISHED_LOWLEVEL) ? 0x4 : 0x2);
    *data << int32(qst->qst_max_level);
    *data << uint32(qst->qst_flags);
    *data << uint8(qst->qst_is_repeatable ? 1 : 0);
    *data << qst->qst_title;
}

void QuestMgr::BuildOfferReward(WorldPacket *data, Quest* qst, Object* qst_giver, uint32 menutype, Player* plr)
{
    uint32 i = 0;
    size_t packetPos;
    ItemPrototype *it;
    data->SetOpcode(SMSG_QUESTGIVER_OFFER_REWARD);
    *data << uint64(qst_giver->GetGUID());
    *data << uint32(qst->id);
    *data << qst->qst_title;
    *data << qst->qst_completiontext;
    // portrait strings
    *data << uint8(0) << uint8(0) << uint8(0) << uint8(0);
    // Portrait displays
    *data << uint32(0) << uint32(0);

    //uint32 a = 0, b = 0, c = 1, d = 0, e = 1;

    *data << (qst->qst_next_quest_id ? uint8(1) : uint8(0));    // next quest shit
    *data << qst->qst_flags;
    *data << qst->qst_suggested_players;                        // "Suggested players"
    *data << uint32(1);                                         // emotes count
    *data << uint32(0);                                         // emote delay
    *data << uint32(1);                                         // emote type
    *data << uint32(qst->count_reward_choiceitem);
    for(i = 0; i < 6; i++)
        *data << uint32(qst->reward_choiceitem[i]);
    for(i = 0; i < 6; i++)
        *data << uint32(qst->reward_choiceitemcount[i]);
    for(i = 0; i < 6; i++)
    {
        it = NULL;
        if(qst->reward_choiceitem[i])
            it = sItemMgr.LookupEntry(qst->reward_choiceitem[i]);
        *data << uint32(it ? it->DisplayInfoID : 0);
    }

    *data << uint32(qst->count_reward_item);
    for(i = 0; i < 6; i++)
        *data << uint32(qst->reward_item[i]);
    for(i = 0; i < 6; i++)
        *data << uint32(qst->reward_itemcount[i]);
    for(i = 0; i < 6; i++)
    {
        it = NULL;
        if(qst->reward_item[i])
            it = sItemMgr.LookupEntry(qst->reward_item[i]);
        *data << uint32(it ? it->DisplayInfoID : 0);
    }

    *data << GenerateRewardMoney(plr, qst);
    *data << uint32(float2int32(GenerateQuestXP(plr, qst) * sWorld.getRate(RATE_QUESTXP)));
    *data << uint32(qst->reward_title);
    *data << uint32(0) << uint32(0); // unk as of 4.0.6a
    *data << uint32(qst->reward_talents);
    *data << uint32(0) << uint32(0); // unk as of 4.0.6a

    for(i = 0; i < 5; i++)
        *data << uint32(qst->reward_repfaction[i]);
    for(i = 0; i < 5; i++)
        *data << int32(qst->reward_repvalue[i]);
    for(i = 0; i < 5; i++)
        *data << int32(qst->reward_replimit[i]);

    *data << uint32(qst->reward_cast_on_player);
    *data << uint32(0); // unk

    for(i = 0; i < 4; i++)
        *data << uint32(0); // CurrencyID
    for(i = 0; i < 4; i++)
        *data << uint32(0); // CurrencyCount

    *data << uint32(0);
    *data << uint32(0);
}

void QuestMgr::BuildQuestDetails(WorldPacket *data, Quest* qst, Object* qst_giver, uint32 menutype, Player* plr)
{
    uint32 i;
    size_t packetPos;
    ItemPrototype *it;
    std::map<uint32, uint8>::const_iterator itr;

    data->SetOpcode( SMSG_QUESTGIVER_QUEST_DETAILS );

    *data << qst_giver->GetGUID();
    *data << uint64(0);                             // Shared Quest from who?
    *data << qst->id;
    *data << qst->qst_title;
    *data << qst->qst_details;
    *data << qst->qst_objectivetext;
    // portrait strings
    *data << uint8(0) << uint8(0) << uint8(0) << uint8(0);
    // Portrait displays
    *data << uint32(0) << uint32(0);

    *data << uint8(1);                              // Is Acceptable
    *data << uint32(qst->qst_flags);
    *data << uint32(qst->qst_suggested_players);    // "Suggested players"
    *data << uint8(0) << uint8(0) << uint32(0);

    *data << uint32(qst->count_reward_choiceitem);
    for(i = 0; i < 6; i++)
        *data << uint32(qst->reward_choiceitem[i]);
    for(i = 0; i < 6; i++)
        *data << uint32(qst->reward_choiceitemcount[i]);
    for(i = 0; i < 6; i++)
    {
        it = (qst->reward_choiceitem[i] ? sItemMgr.LookupEntry(qst->reward_choiceitem[i]) : NULL);
        *data << uint32(it ? it->DisplayInfoID : 0);
    }

    *data << uint32(qst->count_reward_item);
    for(i = 0; i < 6; i++)
        *data << uint32(qst->reward_item[i]);
    for(i = 0; i < 6; i++)
        *data << uint32(qst->reward_itemcount[i]);
    for(i = 0; i < 6; i++)
    {
        it = NULL;
        if(qst->reward_item[i])
            it = sItemMgr.LookupEntry(qst->reward_item[i]);
        *data << uint32(it ? it->DisplayInfoID : 0);
    }

    *data << GenerateRewardMoney(plr, qst);
    *data << uint32(float2int32(GenerateQuestXP(plr, qst) * sWorld.getRate(RATE_QUESTXP)));
    *data << uint32(qst->reward_title);
    *data << uint32(0) << float(0); // unk as of 4.0.6a
    *data << uint32(qst->reward_talents);
    *data << uint32(0) << float(0); // unk as of 4.0.6a

    for(i = 0; i < 5; i++)
        *data << uint32(qst->reward_repfaction[i]);
    for(i = 0; i < 5; i++)
        *data << int32(qst->reward_repvalue[i]);
    for(i = 0; i < 5; i++)
        *data << int32(qst->reward_replimit[i]);

    *data << uint32(qst->reward_cast_on_player);
    *data << uint32(0); // unk

    for(i = 0; i < 4; i++)
        *data << uint32(0); // CurrencyID
    for(i = 0; i < 4; i++)
        *data << uint32(0); // CurrencyCount

    // Reward skill
    *data << uint32(0);
    *data << uint32(0);

    // Emote
    *data << uint32(4);                         // Quantity of emotes, always four
    *data << uint32(1);                         // Emote id 1
    *data << uint32(0);                         // Emote delay/player emote
    *data << uint32(1);                         // Emote id 2
    *data << uint32(0);                         // Emote delay/player emote
    *data << uint32(0);                         // Emote id 3
    *data << uint32(0);                         // Emote delay/player emote
    *data << uint32(0);                         // Emote id 4
    *data << uint32(0);                         // Emote delay/player emote
}

void QuestMgr::BuildRequestItems(WorldPacket *data, Quest* qst, Object* qst_giver, uint32 status)
{
    ItemPrototype * it;
    data->SetOpcode( SMSG_QUESTGIVER_REQUEST_ITEMS );

    *data << uint64(qst_giver->GetGUID());
    *data << uint32(qst->id);
    *data << qst->qst_title;
    *data << (strlen(qst->qst_incompletetext) ? qst->qst_incompletetext : qst->qst_details);
    *data << uint32(0);

    *data << uint32(1);
    *data << uint32(0); // Close on cancel?

    *data << uint32(qst->qst_flags);
    *data << uint32(qst->qst_suggested_players);

    *data << uint32(qst->required_money);       // Required Money
    *data << uint32(qst->count_required_item);  // item count

    // (loop for each item)
    for(uint32 i = 0; i < 6; i++)
    {
        if(qst->required_item[i])
        {
            it = sItemMgr.LookupEntry(qst->required_item[i]);
            *data << uint32(qst->required_item[i]);
            *data << uint32(qst->required_itemcount[i]);
            *data << uint32(it ? it->DisplayInfoID : 0);
        }
    }

    *data << uint32(0); // counter, uint64 for each

    uint32 canFinish = 0;
    if(qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE || status == QMGR_QUEST_FINISHED)
        canFinish = 0x02;

    *data << canFinish; //incomplete button
    *data << uint32(0x04);
    *data << uint32(0x08);
    *data << uint32(0x10);
    *data << uint32(0x40);
}

void QuestMgr::BuildQuestComplete(Player* plr, Quest* qst)
{
    uint32 xp = 0;
    if(plr->getLevel() < plr->GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
    {
        xp = float2int32(GenerateQuestXP(plr,qst) * sWorld.getRate(RATE_QUESTXP));
        plr->GiveXP(xp, 0, false, true);
    }

    WorldPacket data(SMSG_QUESTGIVER_QUEST_COMPLETE, 20);
    data << uint32(qst->id) << uint32(xp);
    data << uint32(GenerateRewardMoney(plr, qst));
    data << uint32(0); // Honor
    data << uint32(qst->reward_talents);
    data << uint32(0); // Arena
    plr->PushPacket(&data);
}

void QuestMgr::BuildQuestList(WorldPacket *data, Object* qst_giver, Player* plr)
{
    uint32 status;
    std::list<QuestRelation *>::iterator it, it2, st, ed;
    std::set<uint32> tmp_map;

    data->Initialize( SMSG_QUESTGIVER_QUEST_LIST );

    *data << qst_giver->GetGUID();
    *data << "How can I help you?"; //Hello line
    *data << uint32(1);//Emote Delay
    *data << uint32(1);//Emote

    bool bValid = false;
    if(qst_giver->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        bValid = castPtr<GameObject>(qst_giver)->HasQuests();
        if(bValid)
        {
            st = castPtr<GameObject>(qst_giver)->QuestsBegin();
            ed = castPtr<GameObject>(qst_giver)->QuestsEnd();
        }
    }
    else if(qst_giver->GetTypeId() == TYPEID_UNIT)
    {
        bValid = castPtr<Creature>(qst_giver)->HasQuests();
        if(bValid)
        {
            st = castPtr<Creature>(qst_giver)->QuestsBegin();
            ed = castPtr<Creature>(qst_giver)->QuestsEnd();
        }
    }

    if(!bValid)
    {
        *data << uint8(0);
        return;
    }

    *data << uint8(sQuestMgr.ActiveQuestsCount(qst_giver, plr));

    for (it = st; it != ed; it++)
    {
        status = sQuestMgr.CalcQuestStatus(plr, *it);
        if (status >= QMGR_QUEST_CHAT)
        {
            if((*it)->qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE)
                status = 2;

            if (tmp_map.find((*it)->qst->id) == tmp_map.end())
            {
                tmp_map.insert((*it)->qst->id);

                *data << (*it)->qst->id;
                switch(status)
                {
                case QMGR_QUEST_FINISHED:
                    *data << uint32(4);
                    break;
                case QMGR_QUEST_AVAILABLE:
                case QMGR_QUEST_CHAT:
                    {
                        if((*it)->qst->qst_is_repeatable)
                            *data << uint32(7);
                        else
                            *data << uint32(8);
                    }break;
                default:
                    *data << status;
                    break;
                }

                *data << uint32((*it)->qst->qst_max_level) << uint32((*it)->qst->qst_flags);
                // Repeatable only on 7?
                *data << uint8((*it)->qst->qst_is_repeatable ? 1 : 0) << (*it)->qst->qst_title;
            }
        }
    }
    tmp_map.clear();
}

void QuestMgr::BuildQuestUpdateAddItem(WorldPacket* data, uint32 itemid, uint32 count)
{
    data->Initialize(SMSG_QUESTUPDATE_ADD_ITEM);
    *data << itemid << count;
}

void QuestMgr::SendQuestUpdateAddKill(Player* plr, uint32 questid, uint32 entry, uint32 count, uint32 tcount, uint64 guid)
{
    WorldPacket data(SMSG_QUESTUPDATE_ADD_KILL,24);
    data << questid;
    data << entry;
    data << count;
    data << tcount;
    data << guid;
    plr->PushPacket(&data);
}

void QuestMgr::BuildQuestUpdateComplete(WorldPacket* data, Quest* qst)
{
    data->Initialize(SMSG_QUESTUPDATE_COMPLETE);

    *data << qst->id;
}

void QuestMgr::SendPushToPartyResponse(Player* plr, Player* pTarget, uint32 response)
{
    WorldPacket data(MSG_QUEST_PUSH_RESULT, 13);
    data << pTarget->GetGUID();
    data << response;
    data << uint8(0);
    plr->PushPacket(&data);
}

bool QuestMgr::OnGameObjectActivate(Player* plr, GameObject* go)
{
    QuestLogEntry *qle;
    uint32 entry = go->GetEntry();
    for(uint8 i = 0; i < QUEST_LOG_COUNT; i++)
    {
        qle = plr->GetQuestLogInSlot( i );
        if( qle != NULL )
        {
            // dont waste time on quests without mobs
            if( qle->GetQuest()->count_required_mob == 0 )
                continue;

            for( uint8 j = 0; j < 4; ++j )
            {
                if( qle->GetQuest()->required_mob[j] == entry &&
                    qle->GetQuest()->required_mobtype[j] == QUEST_MOB_TYPE_GAMEOBJECT &&
                    qle->GetObjectiveCount(j) < qle->GetQuest()->required_mobcount[j] )
                {
                    // add another kill.
                    // (auto-dirtys it)
                    qle->SetObjectiveCount( j, qle->GetObjectiveCount(j) + 1 );
                    qle->SendUpdateAddKill( j );
                    TRIGGER_QUEST_EVENT( qle->GetQuest()->id, OnGameObjectActivate )( entry, plr, qle );

                    if( qle->CanBeFinished() )
                        qle->SendQuestComplete();

                    qle->UpdatePlayerFields();
                    return true;
                }
            }
        }
    }
    return false;
}

void QuestMgr::OnPlayerKill(Player* plr, Creature* victim) //acctually only needed for external script support
{
    uint32 entry = victim->GetEntry();
    _OnPlayerKill( plr, entry);
}

void QuestMgr::_OnPlayerKill(Player* plr, uint32 creature_entry)
{
    if(!plr)
        return;

    uint32 i, j;
    QuestLogEntry *qle;

    if (plr->HasQuestMob(creature_entry))
    {
        for(i = 0; i < QUEST_LOG_COUNT; i++)
        {
            qle = plr->GetQuestLogInSlot( i );
            if( qle != NULL )
            {
                // dont waste time on quests without mobs
                if( qle->GetQuest()->count_required_mob == 0 )
                    continue;

                for( j = 0; j < 4; ++j )
                {
                    if( qle->GetQuest()->required_mob[j] == creature_entry &&
                        qle->GetQuest()->required_mobtype[j] == QUEST_MOB_TYPE_CREATURE &&
                        qle->GetObjectiveCount(j) < qle->GetQuest()->required_mobcount[j] )
                    {
                        // don't update killcount for these questlog entries
                        if ( SkippedKills( qle->GetQuest()->id) )
                            return;

                        // add another kill.(auto-dirtys it)
                        qle->SetObjectiveCount( j, qle->GetObjectiveCount(j) + 1 );
                        qle->SendUpdateAddKill( j );
                        TRIGGER_QUEST_EVENT( qle->GetQuest()->id, OnCreatureKill)( creature_entry, plr, qle );
                        qle->UpdatePlayerFields();
                        break;
                    }
                }
            }
        }
    }

    // Shared kills
    Player* gplr = NULL;

    if(plr->InGroup())
    {
        if(Group* pGroup = plr->GetGroup())
        {
            GroupMembersSet::iterator gitr;
            pGroup->Lock();
            for(uint32 k = 0; k < pGroup->GetSubGroupCount(); k++)
            {
                for(gitr = pGroup->GetSubGroup(k)->GetGroupMembersBegin(); gitr != pGroup->GetSubGroup(k)->GetGroupMembersEnd(); ++gitr)
                {
                    gplr = (*gitr)->m_loggedInPlayer;
                    if(gplr && gplr != plr && plr->isInRange(gplr,300) && gplr->HasQuestMob(creature_entry)) // dont double kills also dont give kills to party members at another side of the world
                    {
                        for( i = 0; i < QUEST_LOG_COUNT; i++ )
                        {
                            qle = gplr->GetQuestLogInSlot(i);
                            if( qle != NULL )
                            {
                                // dont waste time on quests without mobs
                                if( qle->GetQuest()->count_required_mob == 0 )
                                    continue;

                                for( j = 0; j < 4; ++j )
                                {
                                    if( qle->GetQuest()->required_mob[j] == creature_entry &&
                                        qle->GetQuest()->required_mobtype[j] == QUEST_MOB_TYPE_CREATURE &&
                                        qle->GetObjectiveCount(j) < qle->GetQuest()->required_mobcount[j] )
                                    {
                                        // don't update killcount for these quest log entries
                                        if ( SkippedKills( qle->GetQuest()->id) )
                                            return;

                                        // add another kill.
                                        // (auto-dirtys it)
                                        qle->SetObjectiveCount(j, qle->GetObjectiveCount(j) + 1);
                                        qle->SendUpdateAddKill( j );
                                        TRIGGER_QUEST_EVENT( qle->GetQuest()->id, OnCreatureKill )( creature_entry, plr, qle );

                                        if( qle->CanBeFinished() )
                                            qle->SendQuestComplete();

                                        qle->UpdatePlayerFields();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            pGroup->Unlock();
        }
    }
}

void QuestMgr::OnPlayerSlain(Player* plr, Player* victim)
{
    if(!plr || !victim)
        return;

    QuestLogEntry *qle;
    for(uint8 i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if((qle = plr->GetQuestLogInSlot(i)))
        {
            if(qle->GetQuest()->required_player_kills)
            {
                qle->SetPlayerSlainCount(qle->GetPlayerSlainCount() + 1);
                if(qle->CanBeFinished())
                    qle->SendQuestComplete();
                qle->UpdatePlayerFields();
            }
        }
    }

    // Shared kills
    Player* gplr = NULL;

    if(plr->InGroup())
    {
        if(Group* pGroup = plr->GetGroup())
        {
            GroupMembersSet::iterator gitr;
            pGroup->Lock();
            for(uint32 k = 0; k < pGroup->GetSubGroupCount(); k++)
            {
                for(gitr = pGroup->GetSubGroup(k)->GetGroupMembersBegin(); gitr != pGroup->GetSubGroup(k)->GetGroupMembersEnd(); ++gitr)
                {
                    gplr = (*gitr)->m_loggedInPlayer;
                    if(gplr && gplr != plr && plr->isInRange(gplr,300)) // dont double kills also dont give kills to party members at another side of the world
                    {
                        for( uint8 i = 0; i < QUEST_LOG_COUNT; i++ )
                        {
                            qle = gplr->GetQuestLogInSlot(i);
                            if( qle != NULL )
                            {
                                // dont waste time on quests without mobs
                                if( qle->GetQuest()->required_player_kills == 0 )
                                    continue;

                                qle->SetPlayerSlainCount(qle->GetPlayerSlainCount() + 1);
                                if(qle->CanBeFinished())
                                    qle->SendQuestComplete();
                                qle->UpdatePlayerFields();
                            }
                        }
                    }
                }
            }
            pGroup->Unlock();
        }
    }
}

void QuestMgr::OnPlayerCast(Player* plr, uint32 spellid, WoWGuid& victimguid)
{
    if(!plr || !plr->HasQuestSpell(spellid))
        return;

    Unit* victim = plr->GetMapInstance() ? plr->GetMapInstance()->GetUnit(victimguid) : NULL;
    if(victim == NULL)
        return;

    uint32 entry = victim->GetEntry();
    QuestLogEntry *qle;
    for(uint8 i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if((qle = plr->GetQuestLogInSlot(i)))
        {
            // dont waste time on quests without casts
            if(!qle->IsCastQuest())
                continue;

            for(uint8 j = 0; j < 4; ++j)
            {
                if(qle->GetQuest()->required_mob[j])
                {
                    if(qle->GetQuest()->required_mob[j] == entry &&
                        qle->GetRequiredSpell() == spellid &&
                        qle->GetObjectiveCount(j) < qle->GetQuest()->required_mobcount[j] &&
                        !qle->IsUnitAffected(victim->GetGUID()))
                    {
                        // add another kill.(auto-dirtys it)
                        qle->AddAffectedUnit(victim->GetGUID());
                        qle->SetObjectiveCount(j, qle->GetObjectiveCount(j) + 1);
                        qle->SendUpdateAddKill(j);
                        qle->UpdatePlayerFields();
                        break;
                    }
                }
                else if( qle->GetRequiredSpell() == spellid )// Some quests don't require a target.
                {
                    qle->SendUpdateAddKill(j);
                    qle->UpdatePlayerFields();
                    break;
                }
            }
        }
    }
}

void QuestMgr::OnPlayerItemPickup(Player* plr, Item* item, uint32 pickedupstacksize)
{
    QuestLogEntry *qle;
    uint32 pcount, entry = item->GetEntry();
    for( uint8 i = 0; i < QUEST_LOG_COUNT; i++ )
    {
        if( ( qle = plr->GetQuestLogInSlot( i ) ) )
        {
            if( qle->GetQuest()->count_required_item == 0 )
                continue;

            for( uint8 j = 0; j < 6; ++j )
            {
                if( qle->GetQuest()->required_item[j] == entry )
                {
                    pcount = plr->GetInventory()->GetItemCount(entry);
                    TRIGGER_QUEST_EVENT(qle->GetQuest()->id, OnPlayerItemPickup)(entry, pcount, plr, qle);
                    if(pcount < qle->GetQuest()->required_itemcount[j])
                    {
                        WorldPacket data;
                        sQuestMgr.BuildQuestUpdateAddItem(&data, entry, pcount);
                        plr->PushPacket(&data);

                        if(qle->CanBeFinished())
                        {
                            plr->ProcessVisibleQuestGiverStatus();
                            plr->UpdateNearbyGameObjects();
                            qle->SendQuestComplete();
                        }
                        qle->UpdatePlayerFields();
                        break;
                    }
                }
            }
        }
    }
}

void QuestMgr::OnPlayerDropItem(Player* plr, uint32 entry)
{
    QuestLogEntry *qle;
    for( uint8 i = 0; i < QUEST_LOG_COUNT; i++ )
    {
        if( ( qle = plr->GetQuestLogInSlot( i ) ) )
        {
            if( qle->GetQuest()->count_required_item == 0 )
                continue;

            for( uint8 j = 0; j < 6; ++j )
                if( qle->GetQuest()->required_item[j] == entry )
                    qle->UpdatePlayerFields();
        }
    }
}

void QuestMgr::OnPlayerExploreArea(Player* plr, uint32 areaId)
{
    return;// Not implemented

    QuestLogEntry *qle;
    for( uint8 i = 0; i < QUEST_LOG_COUNT; i++ )
    {
        if((qle = plr->GetQuestLogInSlot(i)))
        {

        }
    }
}

void QuestMgr::OnPlayerAreaTrigger(Player* plr, uint32 areaTrigger)
{
    uint32 i, j;
    QuestLogEntry *qle;
    for( i = 0; i < QUEST_LOG_COUNT; i++ )
    {
        if((qle = plr->GetQuestLogInSlot(i)))
        {
            // dont waste time on quests without triggers
            if( qle->GetQuest()->count_requiredareatriggers == 0 )
                continue;

            for( j = 0; j < 4; ++j )
            {
                if(qle->GetQuest()->required_areatriggers[j] == areaTrigger && !qle->HasAreaTrigger(j))
                {
                    qle->SetAreaTrigger(j);
                    TRIGGER_QUEST_EVENT(qle->GetQuest()->id, OnCrossAreaTrigger)(areaTrigger, plr, qle);
                    if(qle->CanBeFinished())
                    {
                        plr->UpdateNearbyGameObjects();
                        qle->SendQuestComplete();
                    }
                    qle->UpdatePlayerFields();
                    break;
                }
            }
        }
    }
}

void QuestMgr::GiveQuestRewardReputation(Player* plr, Quest* qst, Object* qst_giver)
{
    // Reputation reward
    for(uint8 z = 0; z < 5; z++)
    {
        if(qst->reward_repfaction[z] == NULL)
            continue;
        if(qst->reward_repvalue[z] == NULL)
            continue;

        int32 val = float2int32( float( qst->reward_repvalue[z] ) * sWorld.getRate( RATE_QUESTREPUTATION ) ); // reputation rewards
        if(qst->reward_replimit[z] && plr->GetFactionInterface()->GetStanding(qst->reward_repfaction[z])+val >= (int32)qst->reward_replimit[z])
        {
            if((val = (int32)qst->reward_replimit[z] - plr->GetFactionInterface()->GetStanding(qst->reward_repfaction[z])) < 0)
                val = 0; //prevent substraction when current_rep > limit (this quest should not be available?)
        }

        plr->GetFactionInterface()->ModStanding(qst->reward_repfaction[z], val);
    }
}

void QuestMgr::OnQuestAccepted(Player* plr, Quest* qst, Object* qst_giver)
{
    // If the quest should give any items on begin, give them the items.
    for(uint32 i = 0; i < 4; ++i)
    {
        if(qst->receive_items[i] && qst->receive_items[i] != qst->srcitem)
        {
            plr->GetInventory()->AddItemById(qst->receive_items[i], qst->receive_itemcount[i], 0, ADDITEM_FLAG_GIFTED);
        }
    }

    if(qst->srcitem && plr->GetInventory()->GetItemCount(qst->srcitem) < qst->srcitemcount)
        plr->GetInventory()->AddItemById(qst->srcitem, qst->srcitemcount, 0, ADDITEM_FLAG_GIFTED);

    plr->ProcessVisibleQuestGiverStatus();
}

void QuestMgr::GiveQuestTitleReward(Player* plr, Quest* qst)
{
    if(!qst->reward_title)
        return;

    plr->SetKnownTitle(qst->reward_title, true);
}

void QuestMgr::OnQuestFinished(Player* plr, Quest* qst, Object* qst_giver, uint32 reward_slot)
{
    if(qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE)
        BuildQuestComplete(plr, qst);
    else
    {
        QuestLogEntry *qle = plr->GetQuestLogForEntry(qst->id);
        if(!qle)
            return;
        BuildQuestComplete(plr, qst);
        TRIGGER_QUEST_EVENT(qst->id, OnQuestComplete)(plr, qle);

        if (plr->HasQuestSpell(qle->GetRequiredSpell()))
            plr->RemoveQuestSpell(qle->GetRequiredSpell());
        for (uint32 x=0;x<4;x++)
        {
            if (qst->required_mob[x]!=0)
            {
                if (plr->HasQuestMob(qst->required_mob[x]))
                    plr->RemoveQuestMob(qst->required_mob[x]);
            }
        }
        qle->ClearAffectedUnits();

        // Cleanup aquired/required spells, items and kills.
        for( uint32 x=0;x<4;x++)
        {
            if( IsQuestRepeatable(qst) || IsQuestDaily(qst) ) //reset kill-counter in case of repeatable's
            {
                if( qst->required_mob[x] && plr->HasQuestMob(qst->required_mob[x]) )
                    qle->SetObjectiveCount(x,0);
            }
            else
            {
                //Remove Killed npc's
                if( qst->required_mob[x] && plr->HasQuestMob(qst->required_mob[x]) )
                    plr->RemoveQuestMob(qst->required_mob[x]);
            }
        }

        //Remove aquired spells
        if( qst->required_spell && plr->HasQuestSpell(qle->GetRequiredSpell()) )
            plr->RemoveQuestSpell(qle->GetRequiredSpell());

        for (uint32 x=0;x<4;x++)
        {
            if(qst->receive_itemcount[x] == 0)
                continue;
            plr->GetInventory()->RemoveItemAmt(qst->receive_items[x], qst->receive_itemcount[x]);
        }

        for (uint32 x=0;x<6;x++)
        {
            if(qst->required_itemcount[x] == 0)
                continue;
            plr->GetInventory()->RemoveItemAmt(qst->required_item[x], qst->required_itemcount[x]);
        }

        qle->Finish();
    }

    if(qst_giver->GetTypeId() == TYPEID_UNIT)
    {
        if(!castPtr<Creature>(qst_giver)->HasQuest(qst->id, 2))
        {
            //sWorld.LogCheater(plr->GetSession(), "tried to finish quest from invalid npc.");
            plr->GetSession()->Disconnect();
            return;
        }
    }

    //details: hmm as i can remember, repeatable quests give faction rep still after first completation
    if(IsQuestRepeatable(qst))
    {
        // Reputation reward
        GiveQuestRewardReputation(plr, qst, qst_giver);
        GiveQuestTitleReward(plr, qst);

        plr->ModUnsigned32Value(PLAYER_FIELD_COINAGE, GenerateRewardMoney(plr, qst));

        // cast Effect Spell
        if(qst->reward_cast_on_player)
        {
            if(SpellEntry *inf = dbcSpell.LookupEntry(qst->reward_cast_on_player))
            {
                if(Unit *uCaster = (qst_giver->IsUnit() ? castPtr<Unit>(qst_giver) : (qst_giver->IsItem() ? castPtr<Item>(qst_giver)->GetOwner() : NULL)))
                    uCaster->GetSpellInterface()->TriggerSpell(inf, plr);
            }
        }
    }
    else
    {
        // Reputation reward
        GiveQuestRewardReputation(plr, qst, qst_giver);
        GiveQuestTitleReward(plr, qst);

        plr->ModUnsigned32Value(PLAYER_FIELD_COINAGE, GenerateRewardMoney(plr, qst));

        // cast learning spell
        if(qst->reward_spell && !plr->HasSpell(qst->reward_spell))
        {
            // "Teaching" effect
            WorldPacket data(SMSG_SPELL_GO, 42);
            data << qst_giver->GetGUID().asPacked() << qst_giver->GetGUID().asPacked();
            data << uint8(0);
            data << uint32(7763);   // spellID
            data << uint32(256);    // flags
            data << uint32(0) << uint32(0);
            data << uint8(1);       // amount of targets
            data << plr->GetGUID(); // target
            data << uint8(0);
            data << uint16(2);
            data << plr->GetGUID().asPacked();
            plr->PushPacket( &data );

            // Teach the spell
            plr->addSpell(qst->reward_spell);
        }

        // cast Effect Spell
        if(qst->reward_cast_on_player)
        {
            if(SpellEntry *inf = dbcSpell.LookupEntry(qst->reward_cast_on_player))
            {
                if(Unit *uCaster = (qst_giver->IsUnit() ? castPtr<Unit>(qst_giver) : (qst_giver->IsItem() ? castPtr<Item>(qst_giver)->GetOwner() : NULL)))
                    uCaster->GetSpellInterface()->TriggerSpell(inf, plr);
            }
        }

        if(qst->count_reward_item)
        {
            for(uint8 i = 0; i < 4; i++)
            {
                if(qst->reward_itemcount[i] == 0)
                    continue;
                plr->GetInventory()->AddItemById(qst->reward_item[i], qst->reward_itemcount[i], 0, ADDITEM_FLAG_GIFTED|ADDITEM_FLAG_QUICKSAVE);
            }
        }

        if(qst->count_reward_choiceitem && qst->reward_choiceitem[reward_slot])
            plr->GetInventory()->AddItemById(qst->reward_choiceitem[reward_slot], qst->reward_choiceitemcount[reward_slot], 0, ADDITEM_FLAG_GIFTED|ADDITEM_FLAG_QUICKSAVE);
    }

    //Add to finished quests
    plr->AddToCompletedQuests(qst->id, true);
    if(qst->qst_is_repeatable == UNREPEATABLE_QUEST)
    {
        if(qst->qst_zone_id > 0)
            AchieveMgr.UpdateCriteriaValue(plr, ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE, 1, qst->qst_zone_id);

        AchieveMgr.UpdateCriteriaValue(plr, ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT, 1);
        AchieveMgr.UpdateCriteriaValue(plr, ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST, 1, qst->id);
    }

    plr->ProcessVisibleQuestGiverStatus();
}

/////////////////////////////////////
//      Quest Management         //
/////////////////////////////////////

void QuestMgr::LoadNPCQuests(Creature* qst_giver)
{
    qst_giver->SetQuestList(GetCreatureQuestList(qst_giver->GetEntry()));
}

void QuestMgr::LoadGOQuests(GameObject* go)
{
    go->SetQuestList(GetGOQuestList(go->GetEntry()));
}

bool QuestMgr::hasQuests(WorldObject *curObj)
{
    switch(curObj->GetHighGUID())
    {
    case HIGHGUID_TYPE_UNIT:
    case HIGHGUID_TYPE_VEHICLE:
        if(m_npc_quests.find(curObj->GetEntry()) != m_npc_quests.end())
            return true;
        break;
    case HIGHGUID_TYPE_GAMEOBJECT:
        if(m_obj_quests.find(curObj->GetEntry()) != m_obj_quests.end())
            return true;
        break;
    }

    return false;
}

QuestRelationList* QuestMgr::GetGOQuestList(uint32 entryid)
{
    QuestRelationListMap &olist = _GetList<GameObject>();
    QuestRelationListMap::iterator itr = olist.find(entryid);
    return (itr == olist.end()) ? 0 : itr->second;
}

QuestRelationList* QuestMgr::GetCreatureQuestList(uint32 entryid)
{
    QuestRelationListMap &olist = _GetList<Creature>();
    QuestRelationListMap::iterator itr = olist.find(entryid);
    return (itr == olist.end()) ? 0 : itr->second;
}

template <class T> void QuestMgr::_AddQuest(uint32 entryid, Quest *qst, uint8 type)
{
    QuestRelationListMap &olist = _GetList<T>();
    std::list<QuestRelation *>* nlist;
    QuestRelation *ptr = NULL;

    if (olist.find(entryid) == olist.end())
    {
        nlist = new std::list<QuestRelation *>;

        olist.insert(QuestRelationListMap::value_type(entryid, nlist));
    }
    else
    {
        nlist = olist.find(entryid)->second;
    }

    std::list<QuestRelation *>::iterator it;
    for (it = nlist->begin(); it != nlist->end(); it++)
    {
        if ((*it)->qst == qst)
        {
            ptr = (*it);
            break;
        }
    }

    if (ptr == NULL)
    {
        ptr = new QuestRelation;
        ptr->qst = qst;
        ptr->type = type;

        nlist->push_back(ptr);
    }
    else
    {
        ptr->type |= type;
    }
}

void QuestMgr::_CleanLine(std::string *str)
{
    _RemoveChar((char*)"\r", str);
    _RemoveChar((char*)"\n", str);

    while (str->c_str()[0] == 32)
    {
        str->erase(0,1);
    }
}

void QuestMgr::_RemoveChar(char *c, std::string *str)
{
    std::string::size_type pos = str->find(c,0);

    while (pos != std::string::npos)
    {
        str->erase(pos, 1);
        pos = str->find(c, 0);
    }
}

uint32 QuestMgr::GenerateRewardMoney( Player* pl, Quest * qst )
{
    if ( (int32)qst->reward_money < 0 )
        return 0;

    if ( pl && pl->getLevel() >= pl->GetUInt32Value(PLAYER_FIELD_MAX_LEVEL) && qst->qst_is_repeatable == 0 )
        return float2int32(qst->reward_maxlvlmoney*sWorld.getRate(RATE_QUESTMONEY));
    else
        return float2int32(qst->reward_money*sWorld.getRate(RATE_QUESTMONEY));
}

uint32 QuestMgr::GenerateQuestXP(Player* plr, Quest *qst)
{
    if(plr->getLevel() == plr->GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
        return 0;

    int32 quest_level = (qst->qst_max_level == -1 ? plr->getLevel() : qst->qst_max_level);
    int32 diffFactor = 2 * (quest_level - plr->getLevel()) + 20;
    if (diffFactor < 1)
        diffFactor = 1;
    else if (diffFactor > 10)
        diffFactor = 10;
    uint32 xp = qst->reward_xp;
    if(xp == NULL)
    {
        QuestXPLevelEntry *xpentry = dbcQuestXP.LookupEntry(quest_level);
        if(xpentry == NULL)
            return 0;
        xp = diffFactor * xpentry->xpIndex[qst->reward_xp_index] / 10;
    }
    if (xp <= 100) xp = 5 * ((xp + 2) / 5);
    else if (xp <= 500) xp = 10 * ((xp + 5) / 10);
    else if (xp <= 1000) xp = 25 * ((xp + 12) / 25);
    else xp = 50 * ((xp + 25) / 50);
    return xp;
}

void QuestMgr::SendQuestFailed(FAILED_REASON failed, Quest * qst, Player* plyr)
{
    if(!plyr)
        return;

    WorldPacket data(SMSG_QUESTGIVER_QUEST_FAILED, 8);
    data << uint32(qst->id);
    data << uint32(failed);
    plyr->PushPacket(&data);
    sLog.outDebug("WORLD:Sent SMSG_QUESTGIVER_QUEST_FAILED");
}

void QuestMgr::SendQuestUpdateFailedTimer(Quest *pQuest, Player* plyr)
{
    if(!plyr)
        return;

    plyr->PushData(SMSG_QUESTUPDATE_FAILEDTIMER, 4, &pQuest->id);
    sLog.outDebug("WORLD:Sent SMSG_QUESTUPDATE_FAILEDTIMER");
}

void QuestMgr::SendQuestUpdateFailed(Quest *pQuest, Player* plyr)
{
    if(!plyr)
        return;

    plyr->PushData(SMSG_QUESTUPDATE_FAILED, 4, &pQuest->id);
    sLog.outDebug("WORLD:Sent SMSG_QUESTUPDATE_FAILED");
}

void QuestMgr::SendQuestLogFull(Player* plyr)
{
    if(!plyr)
        return;

    plyr->PushData(SMSG_QUESTLOG_FULL);
    sLog.outDebug("WORLD:Sent QUEST_LOG_FULL_MESSAGE");
}

void QuestMgr::BuildQuestFailed(WorldPacket* data, uint32 questid)
{
    data->Initialize(SMSG_QUESTUPDATE_FAILEDTIMER);
    *data << questid;
}

bool QuestMgr::OnActivateQuestGiver(Object* qst_giver, Player* plr)
{
    if(qst_giver->GetTypeId() == TYPEID_GAMEOBJECT && !castPtr<GameObject>(qst_giver)->HasQuests())
        return false;

    WorldPacket data(1000);
    uint32 questCount = sQuestMgr.ActiveQuestsCount(qst_giver, plr);
    if (questCount == 0)
    {
        sLog.outDebug("WORLD: Invalid NPC for CMSG_QUESTGIVER_HELLO.");
        return false;
    }
    else if (questCount == 1)
    {
        std::list<QuestRelation *>::const_iterator itr;
        std::list<QuestRelation *>::const_iterator q_begin;
        std::list<QuestRelation *>::const_iterator q_end;

        bool bValid = false;

        if(qst_giver->GetTypeId() == TYPEID_GAMEOBJECT)
        {
            bValid = castPtr<GameObject>(qst_giver)->HasQuests();
            if(bValid)
            {
                q_begin = castPtr<GameObject>(qst_giver)->QuestsBegin();
                q_end   = castPtr<GameObject>(qst_giver)->QuestsEnd();
            }
        }
        else if(qst_giver->GetTypeId() == TYPEID_UNIT)
        {
            bValid = castPtr<Creature>(qst_giver)->HasQuests();
            if(bValid)
            {
                q_begin = castPtr<Creature>(qst_giver)->QuestsBegin();
                q_end   = castPtr<Creature>(qst_giver)->QuestsEnd();
            }
        }

        if(!bValid)
        {
            sLog.outDebug("QUESTS: Warning, invalid NPC %llu specified for OnActivateQuestGiver. TypeId: %d.", qst_giver->GetGUID(), qst_giver->GetTypeId());
            return false;
        }

        for(itr = q_begin; itr != q_end; itr++)
            if (sQuestMgr.CalcQuestStatus(plr, *itr) >= QMGR_QUEST_CHAT)
                break;

        if (sQuestMgr.CalcStatus(qst_giver, plr) < QMGR_QUEST_CHAT)
            return false;

        ASSERT(itr != q_end);

        uint32 status = CalcStatus(qst_giver, plr);
        if (status >= QMGR_QUEST_FINISHED_LOWLEVEL || ((*itr)->qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE))
        {
            sQuestMgr.BuildOfferReward(&data, (*itr)->qst, qst_giver, 1, plr);
            plr->PushPacket(&data);
            //ss
            sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_OFFER_REWARD." );
        }
        else if (status == QMGR_QUEST_CHAT || status == QMGR_QUEST_AVAILABLE)
        {
            sQuestMgr.BuildQuestDetails(&data, (*itr)->qst, qst_giver, 1, plr);     // 1 because we have 1 quest, and we want goodbye to function
            plr->PushPacket(&data);
            sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_QUEST_DETAILS." );
        }
        else if (status == QMGR_QUEST_NOT_FINISHED)
        {
            sQuestMgr.BuildRequestItems(&data, (*itr)->qst, qst_giver, status);
            plr->PushPacket(&data);
            sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_REQUEST_ITEMS." );
        }
    }
    else
    {
        sQuestMgr.BuildQuestList(&data, qst_giver ,plr);
        plr->PushPacket(&data);
        sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_QUEST_LIST." );
    }
    return true;
}

bool QuestMgr::CanStoreReward(Player* plyr, Quest *qst, uint32 reward_slot)
{

    return true;
}

bool QuestMgr::SkippedKills( uint32 QuestID )
{
    switch(QuestID)
    {
    case 6061:
    case 6062:
    case 6063:
    case 6064:
    case 6082:
    case 6083:
    case 6084:
    case 6085:
    case 6087:
    case 6088:
    case 6101:
    case 6102:
    case 9484:
    case 9485:
    case 9486:
    case 9591:
    case 9592:
    case 9593:
        return true;
    }
    return false;
}