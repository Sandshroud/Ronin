/***
 * Demonstrike Core
 */

#include "StdAfx.h"

QuestMgr::QuestMgr()
{

}

QuestMgr::~QuestMgr()
{
    QuestStorageMap::iterator MapQuestIterator;
    HM_NAMESPACE::hash_map<uint32, Quest*>::iterator itr1;
    HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* >::iterator itr2;
    list<QuestRelation*>::iterator itr3;
    HM_NAMESPACE::hash_map<uint32, list<QuestAssociation *>* >::iterator itr4;
    std::list<QuestAssociation*>::iterator itr5;
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

    for(itr4 = m_quest_associations.begin(); itr4 != m_quest_associations.end(); itr4++)
    {
        if(!itr4->second)
            continue;

        itr5 = itr4->second->begin();
        for(; itr5 != itr4->second->end(); itr5++)
        {
            delete (*itr5);
        }
        itr4->second->clear();
        delete itr4->second;
    }
    m_quest_associations.clear();

    for(itr6 = m_extraqueststuff_list.begin(); itr6 != m_extraqueststuff_list.end(); itr6++)
    {
        GameObjectInfo *inf = GameObjectNameStorage.LookupEntry(*itr6);
        if( inf == NULL )
            continue;
        objmgr.RemoveInvolvedQuestIds(inf->ID);
    }

    for(MapQuestIterator = QuestStorage.begin(); MapQuestIterator != QuestStorage.end(); MapQuestIterator++)
    {
        delete MapQuestIterator->second->qst_title;
        delete MapQuestIterator->second->qst_details;
        delete MapQuestIterator->second->qst_objectivetext;
        delete MapQuestIterator->second->qst_completiontext;
        delete MapQuestIterator->second->qst_endtext;
        delete MapQuestIterator->second->qst_incompletetext;
        for(uint8 i = 0; i < 4; i++)
            delete MapQuestIterator->second->qst_objectivetexts[i];
        delete MapQuestIterator->second;
    }
    QuestStorage.clear();
}

void QuestMgr::LoadQuests()
{
    LoadLocks.Acquire();
    QueryResult* mainResult = WorldDatabase.Query("SELECT * FROM quests");
    if(mainResult == NULL)
    {
        sLog.Notice("QuestMgr", "No quests found in the quests table!");
        return;
    }

    Field *fields = NULL;
    uint32 f = 0, QuestId = 0;
    QueryResult* result = NULL;
    do
    {
        f = 0;
        fields = mainResult->Fetch();
        QuestId = fields[f++].GetUInt32();
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
        newQuest->qst_endtext = strdup(fields[f++].GetString());
        newQuest->qst_incompletetext = strdup(fields[f++].GetString());
        for(uint8 i = 0; i < 4; i++)
            newQuest->qst_objectivetexts[i] = strdup(fields[f++].GetString());
        newQuest->qst_zone_id = fields[f++].GetUInt32();
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
    mainResult = NULL;
    sLog.Notice("QuestMgr", "Loaded %u Quests from the Database! Starting data pooling...", QuestStorage.size());

    result = WorldDatabase.Query("SELECT * FROM quests_rewards");
    if(result != NULL)
    {
        if(result->GetFieldCount() != 55)
            sLog.Error("QuestMgr", "Incorrect column count in quests_rewards(%u/55)", result->GetFieldCount());
        else do
        {
            f = 0;
            fields = result->Fetch();
            QuestId = fields[f++].GetUInt32();
            if(QuestStorage.find(QuestId) == QuestStorage.end())
            {
                continue;
            }

            Quest *quest = QuestStorage.at(QuestId);
            quest->srcitem = fields[f++].GetUInt32();
            quest->srcitemcount = fields[f++].GetUInt16();
            for(uint8 i = 0; i < 6; i++)
            {
                quest->reward_choiceitem[i] = fields[f++].GetUInt32();
                if(quest->reward_choiceitem[i])
                    quest->count_reward_choiceitem++;
            }
            for(uint8 i = 0; i < 6; i++)
                quest->reward_choiceitemcount[i] = fields[f++].GetUInt16();

            for(uint8 i = 0; i < 4; i++)
            {
                quest->reward_item[i] = fields[f++].GetUInt32();
                if(quest->reward_item[i])
                    quest->count_reward_item++;
            }
            for(uint8 i = 0; i < 4; i++)
                quest->reward_itemcount[i] = fields[f++].GetUInt16();

            for(uint8 i = 0; i < 4; i++)
            {
                quest->receive_items[i] = fields[f++].GetUInt32();
                if(quest->receive_items[i])
                    quest->count_receiveitems++;
            }
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
        result = NULL;
    }

    result = WorldDatabase.Query("SELECT * FROM quests_objectives");
    if(result != NULL)
    {
        if(result->GetFieldCount() != 40)
            sLog.Error("QuestMgr", "Incorrect column count in quests_rewards(%u/40)", result->GetFieldCount());
        else do
        {
            f = 0;
            fields = result->Fetch();
            QuestId = fields[f++].GetUInt32();
            if(QuestStorage.find(QuestId) == QuestStorage.end())
            {
                continue;
            }

            Quest *quest = QuestStorage.at(QuestId);
            for(uint8 i = 0; i < 6; i++)
            {
                quest->required_item[i] = fields[f++].GetUInt32();
                if(quest->required_item[i])
                    quest->count_required_item++;
            }

            for(uint8 i = 0; i < 6; i++)
                quest->required_itemcount[i] = fields[f++].GetUInt16();

            for(uint8 i = 0; i < 4; i++)
            {
                quest->required_mob[i] = fields[f++].GetUInt32();
                if(quest->required_mob[i])
                    quest->count_required_mob++;
            }

            for(uint8 i = 0; i < 4; i++)
                quest->required_mobtype[i] = fields[f++].GetUInt8();
            for(uint8 i = 0; i < 4; i++)
                quest->required_mobcount[i] = fields[f++].GetUInt16();

            quest->required_spell = fields[f++].GetUInt32();
            f++; f++; f++;
            for(uint8 i = 0; i < 4; i++)
            {
                quest->required_areatriggers[i] = fields[f++].GetUInt32();
                if(quest->required_areatriggers[i])
                    quest->count_requiredareatriggers++;
            }

            quest->required_player_kills = fields[f++].GetUInt32();
            quest->required_timelimit = fields[f++].GetUInt32();
            quest->required_money = fields[f++].GetUInt32();
            quest->required_point_mapid = fields[f++].GetUInt32();
            quest->required_point_x = fields[f++].GetFloat();
            quest->required_point_y = fields[f++].GetFloat();
            quest->required_point_radius = fields[f++].GetUInt32();
        }while(result->NextRow());
        delete result;
        result = NULL;
    }

    result = WorldDatabase.Query("SELECT * FROM quests_requirements");
    if(result != NULL)
    {
        if(result->GetFieldCount() != 13)
            sLog.Error("QuestMgr", "Incorrect column count in quests_rewards(%u/13)", result->GetFieldCount());
        else do
        {
            f = 0;
            fields = result->Fetch();
            QuestId = fields[f++].GetUInt32();
            if(QuestStorage.find(QuestId) == QuestStorage.end())
            {
                continue;
            }

            Quest *quest = QuestStorage.at(QuestId);
            quest->required_team = fields[f++].GetInt8();
            quest->required_races = fields[f++].GetUInt16();
            quest->required_class = fields[f++].GetUInt16();
            quest->required_tradeskill = fields[f++].GetUInt16();
            quest->required_tradeskill_value = fields[f++].GetUInt16();
            quest->required_rep_faction = fields[f++].GetUInt16();
            quest->required_rep_value = fields[f++].GetUInt16();
            for(uint8 i = 0; i < 4; i++)
            {
                quest->required_quests[i] = fields[f++].GetUInt32();
                if(quest->required_quests[i])
                    quest->count_requiredquests = i+1;
            }
            quest->required_quest_one_or_all = fields[f++].GetBool();
        }while(result->NextRow());
        delete result;
        result = NULL;
    }

    LoadExtraQuestStuff();
    LoadLocks.Release();
}

uint32 QuestMgr::CalcQuestStatus(Player* plr, QuestRelation* qst)
{
    return CalcQuestStatus(plr, qst->qst, qst->type, false);
}

// Crow: ALL NOT AVAILABLES MUST GO FIRST!
uint32 QuestMgr::PlayerMeetsReqs(Player* plr, Quest* qst, bool skiplevelcheck)
{
    std::list<uint32>::iterator itr;
    uint32 status = QMGR_QUEST_AVAILABLE;

    if(qst->qst_previous_quest_id)
        if(!(plr->HasFinishedQuest(qst->qst_previous_quest_id)))
            return QMGR_QUEST_NOT_AVAILABLE;

    if(qst->required_team >= 0)
        if(qst->required_team != plr->GetTeam())
            return QMGR_QUEST_NOT_AVAILABLE;

    if(qst->required_class)
        if(!(qst->required_class & plr->getClassMask()))
            return QMGR_QUEST_NOT_AVAILABLE;

    if(qst->required_races)
        if(!(qst->required_races & plr->getRaceMask()))
            return QMGR_QUEST_NOT_AVAILABLE;

    if(qst->required_tradeskill)
    {
        if(!plr->_HasSkillLine(qst->required_tradeskill))
            return QMGR_QUEST_NOT_AVAILABLE;
        if (qst->required_tradeskill_value && plr->_GetSkillLineCurrent(qst->required_tradeskill) < qst->required_tradeskill_value)
            return QMGR_QUEST_NOT_AVAILABLE;
    }

    // Check reputation
    if(qst->required_rep_faction && qst->required_rep_value)
        if(plr->GetStanding(qst->required_rep_faction) < (int32)qst->required_rep_value)
            return QMGR_QUEST_NOT_AVAILABLE;

    //Do we just need to complete one, or all quest requisitions?
    if( !qst->required_quest_one_or_all )
    {
        for(uint32 i = 0; i < qst->count_requiredquests; i++)
        {
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

uint32 QuestMgr::CalcQuestStatus(Player* plr, Quest* qst, uint8 type, bool skiplevelcheck)
{
    QuestLogEntry* qle = plr->GetQuestLogForEntry(qst->id);
    if (!qle)
    {
        if (type & QUESTGIVER_QUEST_START)
            return PlayerMeetsReqs(plr, qst, skiplevelcheck);
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
        bValid = TO_GAMEOBJECT(quest_giver)->HasQuests();
        if(bValid)
        {
            q_begin = TO_GAMEOBJECT(quest_giver)->QuestsBegin();
            q_end = TO_GAMEOBJECT(quest_giver)->QuestsEnd();
        }
    }
    else if( quest_giver->GetTypeId() == TYPEID_UNIT )
    {
        bValid = TO_CREATURE( quest_giver )->HasQuests();
        if(bValid)
        {
            q_begin = TO_CREATURE(quest_giver)->QuestsBegin();
            q_end = TO_CREATURE(quest_giver)->QuestsEnd();
        }
    }
    else if( quest_giver->GetTypeId() == TYPEID_ITEM )
    {
        if( TO_ITEM( quest_giver )->GetProto()->QuestId )
            bValid = true;
    } //This will be handled at quest share so nothing important as status
    else if(quest_giver->IsPlayer())
        status = QMGR_QUEST_AVAILABLE;
    if(bValid == false)
        return status;

    if(quest_giver->GetTypeId() == TYPEID_ITEM)
    {
        Quest *pQuest = GetQuestPointer( TO_ITEM(quest_giver)->GetProto()->QuestId );
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
    map<uint32, uint8> tmp_map;
    uint32 questCount = 0;

    std::list<QuestRelation *>::const_iterator q_begin;
    std::list<QuestRelation *>::const_iterator q_end;
    bool bValid = false;

    if(quest_giver->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        bValid = TO_GAMEOBJECT(quest_giver)->HasQuests();
        if(bValid)
        {
            q_begin = TO_GAMEOBJECT(quest_giver)->QuestsBegin();
            q_end   = TO_GAMEOBJECT(quest_giver)->QuestsEnd();

        }
    }
    else if(quest_giver->GetTypeId() == TYPEID_UNIT)
    {
        bValid = TO_CREATURE(quest_giver)->HasQuests();
        if(bValid)
        {
            q_begin = TO_CREATURE(quest_giver)->QuestsBegin();
            q_end   = TO_CREATURE(quest_giver)->QuestsEnd();
        }
    }

    if(!bValid)
    {
        sLog.outDebug("QUESTS: Warning, invalid NPC "I64FMT" specified for ActiveQuestsCount. TypeId: %d.", quest_giver->GetGUID(), quest_giver->GetTypeId());
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
            it = ItemPrototypeStorage.LookupEntry(qst->reward_choiceitem[i]);
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
            it = ItemPrototypeStorage.LookupEntry(qst->reward_item[i]);
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
        it = (qst->reward_choiceitem[i] ? ItemPrototypeStorage.LookupEntry(qst->reward_choiceitem[i]) : NULL);
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
            it = ItemPrototypeStorage.LookupEntry(qst->reward_item[i]);
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
            it = ItemPrototypeStorage.LookupEntry(qst->required_item[i]);
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
        plr->GiveXP(xp, 0, false);
    }

    WorldPacket data(SMSG_QUESTGIVER_QUEST_COMPLETE, 20);
    data << uint8(0x80);
    data << uint32(0); // rewSkillLineId
    data << uint32(qst->id);
    data << uint32(GenerateRewardMoney(plr, qst));
    data << uint32(qst->reward_talents);
    data << uint32(0) << uint32(xp);
    plr->GetSession()->SendPacket(&data);
}

void QuestMgr::BuildQuestList(WorldPacket *data, Object* qst_giver, Player* plr)
{
    uint32 status;
    list<QuestRelation *>::iterator it,it2;
    list<QuestRelation *>::iterator st;
    list<QuestRelation *>::iterator ed;
    set<uint32> tmp_map;

    data->Initialize( SMSG_QUESTGIVER_QUEST_LIST );

    *data << qst_giver->GetGUID();
    *data << "How can I help you?"; //Hello line
    *data << uint32(1);//Emote Delay
    *data << uint32(1);//Emote

    bool bValid = false;
    if(qst_giver->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        bValid = TO_GAMEOBJECT(qst_giver)->HasQuests();
        if(bValid)
        {
            st = TO_GAMEOBJECT(qst_giver)->QuestsBegin();
            ed = TO_GAMEOBJECT(qst_giver)->QuestsEnd();
        }
    }
    else if(qst_giver->GetTypeId() == TYPEID_UNIT)
    {
        bValid = TO_CREATURE(qst_giver)->HasQuests();
        if(bValid)
        {
            st = TO_CREATURE(qst_giver)->QuestsBegin();
            ed = TO_CREATURE(qst_giver)->QuestsEnd();
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
    plr->GetSession()->SendPacket(&data);
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
    plr->GetSession()->SendPacket(&data);
}

bool QuestMgr::OnGameObjectActivate(Player* plr, GameObject* go)
{
    uint32 i, j;
    QuestLogEntry *qle;
    uint32 entry = go->GetEntry();

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
                if( qle->GetQuest()->required_mob[j] == entry &&
                    qle->GetQuest()->required_mobtype[j] == QUEST_MOB_TYPE_GAMEOBJECT &&
                    qle->m_mobcount[j] < qle->GetQuest()->required_mobcount[j] )
                {
                    // add another kill.
                    // (auto-dirtys it)
                    qle->SetMobCount( j, qle->m_mobcount[j] + 1 );
                    qle->SendUpdateAddKill( j );
                    CALL_QUESTSCRIPT_EVENT( qle->GetQuest()->id, OnGameObjectActivate )( entry, plr, qle );

                    if( qle->CanBeFinished() )
                        qle->SendQuestComplete();

                    qle->UpdatePlayerFields();
                    qle->SaveToDB(NULL);
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
                        qle->m_mobcount[j] < qle->GetQuest()->required_mobcount[j] )
                    {
                        // don't update killcount for these questlog entries
                        if ( SkippedKills( qle->GetQuest()->id) )
                            return;

                        // add another kill.(auto-dirtys it)
                        qle->SetMobCount( j, qle->m_mobcount[j] + 1 );
                        qle->SendUpdateAddKill( j );
                        CALL_QUESTSCRIPT_EVENT( qle->GetQuest()->id, OnCreatureKill)( creature_entry, plr, qle );
                        qle->UpdatePlayerFields();
                        qle->SaveToDB(NULL);
                        break;
                    }
                }
            }
        }
    }

    // Shared kills
    Player* gplr = NULLPLR;

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
                                        qle->m_mobcount[j] < qle->GetQuest()->required_mobcount[j] )
                                    {
                                        // don't update killcount for these quest log entries
                                        if ( SkippedKills( qle->GetQuest()->id) )
                                            return;

                                        // add another kill.
                                        // (auto-dirtys it)
                                        qle->SetMobCount(j, qle->m_mobcount[j] + 1);
                                        qle->SendUpdateAddKill( j );
                                        CALL_QUESTSCRIPT_EVENT( qle->GetQuest()->id, OnCreatureKill )( creature_entry, plr, qle );

                                        if( qle->CanBeFinished() )
                                            qle->SendQuestComplete();

                                        qle->UpdatePlayerFields();
                                        qle->SaveToDB(NULL);
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
    uint32 i;
    for(i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if((qle = plr->GetQuestLogInSlot(i)))
        {
            if(qle->m_quest->required_player_kills)
            {
                qle->SetPlayerSlainCount(qle->m_player_slain + 1);
                if(qle->CanBeFinished())
                    qle->SendQuestComplete();
                qle->UpdatePlayerFields();
                qle->SaveToDB(NULL);
            }
        }
    }

    // Shared kills
    Player* gplr = NULLPLR;

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
                        for( i = 0; i < QUEST_LOG_COUNT; i++ )
                        {
                            qle = gplr->GetQuestLogInSlot(i);
                            if( qle != NULL )
                            {
                                // dont waste time on quests without mobs
                                if( qle->GetQuest()->required_player_kills == 0 )
                                    continue;

                                qle->SetPlayerSlainCount(qle->m_player_slain + 1);
                                if(qle->CanBeFinished())
                                    qle->SendQuestComplete();
                                qle->UpdatePlayerFields();
                                qle->SaveToDB(NULL);
                            }
                        }
                    }
                }
            }
            pGroup->Unlock();
        }
    }
}

void QuestMgr::OnPlayerCast(Player* plr, uint32 spellid, uint64& victimguid)
{
    if(!plr || !plr->HasQuestSpell(spellid))
        return;

    Unit* victim = plr->GetMapMgr() ? plr->GetMapMgr()->GetUnit(victimguid) : NULLUNIT;
    if(victim == NULL)
        return;

    uint32 i, j;
    uint32 entry = victim->GetEntry();
    QuestLogEntry *qle;
    for(i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if((qle = plr->GetQuestLogInSlot(i)))
        {
            // dont waste time on quests without casts
            if(!qle->IsCastQuest())
                continue;

            for(j = 0; j < 4; ++j)
            {
                if(qle->GetQuest()->required_mob[j])
                {
                    if(qle->GetQuest()->required_mob[j] == entry &&
                        qle->GetRequiredSpell() == spellid &&
                        qle->m_mobcount[j] < qle->GetQuest()->required_mobcount[j] &&
                        !qle->IsUnitAffected(victim))
                    {
                        // add another kill.(auto-dirtys it)
                        qle->AddAffectedUnit(victim);
                        qle->SetMobCount(j, qle->m_mobcount[j] + 1);
                        qle->SendUpdateAddKill(j);
                        qle->UpdatePlayerFields();
                        qle->SaveToDB(NULL);
                        break;
                    }
                }
                else if( qle->GetRequiredSpell() == spellid )// Some quests don't require a target.
                {
                    qle->SendUpdateAddKill(j);
                    qle->UpdatePlayerFields();
                    qle->SaveToDB(NULL);
                    break;
                }
            }
        }
    }
}

void QuestMgr::OnPlayerItemPickup(Player* plr, Item* item, uint32 pickedupstacksize)
{
    uint32 i, j;
    uint32 pcount;
    uint32 entry = item->GetEntry();
    QuestLogEntry *qle;
    for( i = 0; i < QUEST_LOG_COUNT; i++ )
    {
        if( ( qle = plr->GetQuestLogInSlot( i ) ) )
        {
            if( qle->GetQuest()->count_required_item == 0 )
                continue;

            for( j = 0; j < 6; ++j )
            {
                if( qle->GetQuest()->required_item[j] == entry )
                {
                    pcount = plr->GetItemInterface()->GetItemCount(entry, true, item, pickedupstacksize);
                    CALL_QUESTSCRIPT_EVENT(qle->GetQuest()->id, OnPlayerItemPickup)(entry, pcount, plr, qle);
                    if(pcount < qle->GetQuest()->required_itemcount[j])
                    {
                        WorldPacket data(SMSG_QUESTUPDATE_ADD_ITEM, 8);
                        data << qle->GetQuest()->required_item[j] << uint32(1);
                        plr->GetSession()->SendPacket(&data);
                        if(qle->CanBeFinished())
                        {
                            plr->UpdateNearbyQuestGivers();
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
    uint32 i, j;
    QuestLogEntry *qle;
    for( i = 0; i < QUEST_LOG_COUNT; i++ )
    {
        if( ( qle = plr->GetQuestLogInSlot( i ) ) )
        {
            if( qle->GetQuest()->count_required_item == 0 )
                continue;

            for( j = 0; j < 6; ++j )
                if( qle->GetQuest()->required_item[j] == entry )
                    qle->UpdatePlayerFields();
        }
    }
}

void QuestMgr::OnPlayerExploreArea(Player* plr, uint32 areaId)
{
    uint32 i, j;
    QuestLogEntry *qle;
    for( i = 0; i < QUEST_LOG_COUNT; i++ )
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
                if(qle->GetQuest()->required_areatriggers[j] == areaTrigger
                    && !qle->GetCrossedAreaTrigger(j))
                {
                    qle->SetAreaTrigger(j);
                    CALL_QUESTSCRIPT_EVENT(qle->GetQuest()->id, OnCrossAreaTrigger)(areaTrigger, plr, qle);
                    if(qle->CanBeFinished())
                    {
                        plr->UpdateNearbyGameObjects();
                        qle->SendQuestComplete();
                    }
                    qle->UpdatePlayerFields();
                    qle->SaveToDB(NULL);
                    break;
                }
            }
        }
    }
}

void QuestMgr::GiveQuestRewardReputation(Player* plr, Quest* qst, Object* qst_giver)
{
    // Reputation reward
    for(int z = 0; z < 5; z++)
    {
        uint32 fact = 19;   // default to 19 if no factiondbc
        int32 amt = 0;   // guess
        if( qst->reward_repfaction[z] )
        {
            fact = qst->reward_repfaction[z];
            if(qst->reward_repvalue[z])
                amt = qst->reward_repvalue[z];
        }
        else
            continue;

        if(amt)
        {
            amt = float2int32( float( amt ) * sWorld.getRate( RATE_QUESTREPUTATION ) ); // reputation rewards
            if(qst->reward_replimit && plr->GetStanding(fact)+ amt >= (int32)qst->reward_replimit)
            {
                amt = (int32)qst->reward_replimit - plr->GetStanding(fact);
                //prevent substraction when current_rep > limit (this quest should not be available?)
                amt = amt<0 ? 0 : amt;
            }

            plr->ModStanding(fact, amt);
        }
    }
}
void QuestMgr::OnQuestAccepted(Player* plr, Quest* qst, Object* qst_giver)
{

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
    {
        BuildQuestComplete(plr, qst);
        plr->PushToRemovedQuests(qst->id);
    }
    else
    {
        QuestLogEntry *qle = plr->GetQuestLogForEntry(qst->id);
        if(!qle)
            return;
        BuildQuestComplete(plr, qst);
        CALL_QUESTSCRIPT_EVENT(qst->id, OnQuestComplete)(plr, qle);

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
                    qle->SetMobCount(x,0);
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

        for( uint32 y = 0; y < 6; y++)
        {
            //always remove collected items (need to be recollectable again in case of repeatable).
            if( qst->required_item[y] )
                plr->GetItemInterface()->RemoveItemAmt(qst->required_item[y], qst->required_itemcount[y]);
        }

        qle->Finish();
    }

    if(qst_giver->GetTypeId() == TYPEID_UNIT)
    {
        if(!TO_CREATURE(qst_giver)->HasQuest(qst->id, 2))
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

        // Static Item reward
        for(uint32 i = 0; i < 4; i++)
        {
            if(qst->reward_item[i])
            {
                ItemPrototype *proto = ItemPrototypeStorage.LookupEntry(qst->reward_item[i]);
                if(!proto)
                    sLog.outDebug("Invalid item prototype in quest reward! ID %d, quest %d", qst->reward_item[i], qst->id);
                else
                {
                    Item* add;
                    SlotResult slotresult;
                    add = plr->GetItemInterface()->FindItemLessMax(qst->reward_item[i], qst->reward_itemcount[i], false);
                    if (!add)
                    {
                        slotresult = plr->GetItemInterface()->FindFreeInventorySlot(proto);
                        if(!slotresult.Result)
                        {
                            plr->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            Item* itm = objmgr.CreateItem(qst->reward_item[i], plr);
                            itm->SetUInt32Value(ITEM_FIELD_STACK_COUNT, uint32(qst->reward_itemcount[i]));
                            if( !plr->GetItemInterface()->SafeAddItem(itm,slotresult.ContainerSlot, slotresult.Slot) )
                            {
                                itm->DeleteMe();
                                itm = NULLITEM;
                            }
                        }
                    }
                    else
                    {
                        add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + qst->reward_itemcount[i]);
                        add->m_isDirty = true;
                    }
                }
            }
        }

        // Choice Rewards
        if(qst->reward_choiceitem[reward_slot])
        {
            ItemPrototype *proto = ItemPrototypeStorage.LookupEntry(qst->reward_choiceitem[reward_slot]);
            if(!proto)
                sLog.outDebug("Invalid item prototype in quest reward! ID %d, quest %d", qst->reward_choiceitem[reward_slot], qst->id);
            else
            {
                Item* add;
                SlotResult slotresult;
                add = plr->GetItemInterface()->FindItemLessMax(qst->reward_choiceitem[reward_slot], qst->reward_choiceitemcount[reward_slot], false);
                if (!add)
                {
                    slotresult = plr->GetItemInterface()->FindFreeInventorySlot(proto);
                    if(!slotresult.Result)
                    {
                        plr->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_INVENTORY_FULL);
                    }
                    else
                    {
                        Item* itm = objmgr.CreateItem(qst->reward_choiceitem[reward_slot], plr);
                        itm->SetUInt32Value(ITEM_FIELD_STACK_COUNT, uint32(qst->reward_choiceitemcount[reward_slot]));
                        if( !plr->GetItemInterface()->SafeAddItem(itm,slotresult.ContainerSlot, slotresult.Slot) )
                        {
                            itm->DeleteMe();
                            itm = NULLITEM;
                        }

                    }
                }
                else
                {
                    add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + qst->reward_choiceitemcount[reward_slot]);
                    add->m_isDirty = true;
                }
            }
        }

        // cast Effect Spell
        if(qst->reward_cast_on_player)
        {
            SpellEntry  * inf =dbcSpell.LookupEntry(qst->reward_cast_on_player);
            if(inf)
            {
                Spell* spe(new Spell(qst_giver,inf,true,NULLAURA));
                SpellCastTargets tgt;
                tgt.m_unitTarget = plr->GetGUID();
                spe->prepare(&tgt);
            }
        }
    }
    else
    {
        // Reputation reward
        GiveQuestRewardReputation(plr, qst, qst_giver);
        GiveQuestTitleReward(plr, qst);

        plr->ModUnsigned32Value(PLAYER_FIELD_COINAGE, GenerateRewardMoney(plr, qst));

        // Static Item reward
        for(uint32 i = 0; i < 4; i++)
        {
            if(qst->reward_item[i])
            {
                ItemPrototype *proto = ItemPrototypeStorage.LookupEntry(qst->reward_item[i]);
                if(!proto)
                    sLog.outDebug("Invalid item prototype in quest reward! ID %d, quest %d", qst->reward_item[i], qst->id);
                else
                {
                    Item* add;
                    SlotResult slotresult;
                    add = plr->GetItemInterface()->FindItemLessMax(qst->reward_item[i], qst->reward_itemcount[i], false);
                    if (!add)
                    {
                        slotresult = plr->GetItemInterface()->FindFreeInventorySlot(proto);
                        if(!slotresult.Result)
                        {
                            plr->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            Item* itm = objmgr.CreateItem(qst->reward_item[i], plr);
                            itm->SetUInt32Value(ITEM_FIELD_STACK_COUNT, uint32(qst->reward_itemcount[i]));
                            if( !plr->GetItemInterface()->SafeAddItem(itm,slotresult.ContainerSlot, slotresult.Slot) )
                            {
                                itm->DeleteMe();
                                itm = NULLITEM;
                            }
                        }
                    }
                    else
                    {
                        add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + qst->reward_itemcount[i]);
                        add->m_isDirty = true;
                    }
                }
            }
        }

        // Choice Rewards
        if(qst->reward_choiceitem[reward_slot])
        {
            ItemPrototype *proto = ItemPrototypeStorage.LookupEntry(qst->reward_choiceitem[reward_slot]);
            if(!proto)
                sLog.outDebug("Invalid item prototype in quest reward! ID %d, quest %d", qst->reward_choiceitem[reward_slot], qst->id);
            else
            {
                Item* add;
                SlotResult slotresult;
                add = plr->GetItemInterface()->FindItemLessMax(qst->reward_choiceitem[reward_slot], qst->reward_choiceitemcount[reward_slot], false);
                if (!add)
                {
                    slotresult = plr->GetItemInterface()->FindFreeInventorySlot(proto);
                    if(!slotresult.Result)
                    {
                        plr->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_INVENTORY_FULL);
                    }
                    else
                    {
                        Item* itm = objmgr.CreateItem(qst->reward_choiceitem[reward_slot], plr);
                        itm->SetUInt32Value(ITEM_FIELD_STACK_COUNT, uint32(qst->reward_choiceitemcount[reward_slot]));
                        if( !plr->GetItemInterface()->SafeAddItem(itm,slotresult.ContainerSlot, slotresult.Slot) )
                        {
                            itm->DeleteMe();
                            itm = NULLITEM;
                        }
                    }
                }
                else
                {
                    add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + qst->reward_choiceitemcount[reward_slot]);
                    add->m_isDirty = true;
                }
            }
        }


        // cast learning spell
        if(qst->reward_spell)
        {
            if(!plr->HasSpell(qst->reward_spell))
            {
                // "Teaching" effect
                WorldPacket data(SMSG_SPELL_START, 42);
                data << qst_giver->GetNewGUID() << qst_giver->GetNewGUID();
                data << uint32(7763);
                data << uint8(0);
                data << uint16(0);
                data << uint32(0);
                data << uint16(2);
                data << plr->GetGUID();
                plr->GetSession()->SendPacket( &data );

                data.Initialize( SMSG_SPELL_GO );
                data << qst_giver->GetNewGUID() << qst_giver->GetNewGUID();
                data << uint32(7763);          // spellID
                data << uint8(0) << uint8(1);   // flags
                data << uint8(1);              // amount of targets
                data << plr->GetGUID();      // target
                data << uint8(0);
                data << uint16(2);
                data << plr->GetGUID();
                plr->GetSession()->SendPacket( &data );

                // Teach the spell
                plr->addSpell(qst->reward_spell);
            }
        }

        // cast Effect Spell
        if(qst->reward_cast_on_player)
        {
            SpellEntry  * inf =dbcSpell.LookupEntry(qst->reward_cast_on_player);
            if(inf)
            {
                Spell* spe(new Spell(qst_giver,inf,true,NULLAURA));
                SpellCastTargets tgt;
                tgt.m_unitTarget = plr->GetGUID();
                spe->prepare(&tgt);
            }
        }
    }

    //Add to finished quests
    if(qst->qst_is_repeatable == REPEATABLE_DAILY)
        plr->AddToFinishedDailyQuests(qst->id);
    else if(!IsQuestRepeatable(qst))
        plr->AddToFinishedQuests(qst->id);
    if(qst->qst_complete_phase != 0)
        plr->SetPhaseMask(qst->qst_complete_phase);

    //Remove any timed events
    if (sEventMgr.HasEvent(plr,EVENT_TIMED_QUEST_EXPIRE))
        sEventMgr.RemoveEvents(plr, EVENT_TIMED_QUEST_EXPIRE); 

    plr->UpdateNearbyQuestGivers();

    if(qst->qst_zone_id)
        plr->GetAchievementInterface()->HandleAchievementCriteriaCompleteQuestsInZone(qst->qst_zone_id);
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

QuestRelationList* QuestMgr::GetGOQuestList(uint32 entryid)
{
    HM_NAMESPACE::hash_map<uint32, QuestRelationList* > &olist = _GetList<GameObject>();
    HM_NAMESPACE::hash_map<uint32, QuestRelationList* >::iterator itr = olist.find(entryid);
    return (itr == olist.end()) ? 0 : itr->second;
}

QuestRelationList* QuestMgr::GetCreatureQuestList(uint32 entryid)
{
    HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* > &olist = _GetList<Creature>();
    HM_NAMESPACE::hash_map<uint32, QuestRelationList* >::iterator itr = olist.find(entryid);
    return (itr == olist.end()) ? 0 : itr->second;
}

QuestAssociationList* QuestMgr::GetQuestAssociationListForItemId (uint32 itemId)
{
    HM_NAMESPACE::hash_map<uint32, QuestAssociationList* > &associationList = GetQuestAssociationList();
    HM_NAMESPACE::hash_map<uint32, QuestAssociationList* >::iterator itr = associationList.find( itemId );
    return (itr != associationList.end()) ? itr->second : 0;
}

void QuestMgr::AddItemQuestAssociation( uint32 itemId, Quest *qst, uint8 item_count)
{
    HM_NAMESPACE::hash_map<uint32, list<QuestAssociation *>* > &associationList = GetQuestAssociationList();
    std::list<QuestAssociation *>* tempList;
    QuestAssociation *ptr = NULL;

    // look for the item in the associationList
    if (associationList.find( itemId ) == associationList.end() )
    {
        // not found. Create a new entry and QuestAssociationList
        tempList = new std::list<QuestAssociation *>;

        associationList.insert(HM_NAMESPACE::hash_map<uint32, list<QuestAssociation *>* >::value_type(itemId, tempList));
    }
    else
    {
        // item found, now we'll search through its QuestAssociationList
        tempList = associationList.find( itemId )->second;
    }

    // look through this item's QuestAssociationList for a matching quest entry
    list<QuestAssociation *>::iterator it;
    for (it = tempList->begin(); it != tempList->end(); it++)
    {
        if ((*it)->qst == qst)
        {
            // matching quest found
            ptr = (*it);
            break;
        }
    }

    // did we find a matching quest?
    if (ptr == NULL)
    {
        // nope, create a new QuestAssociation for this item and quest
        ptr = new QuestAssociation;
        ptr->qst = qst;
        ptr->item_count = item_count;

        tempList->push_back( ptr );
    }
    else
    {
        // yep, update the QuestAssociation with the new item_count information
        ptr->item_count = item_count;
        sLog.outDebug( "WARNING: Duplicate entries found in item_quest_association, updating item #%d with new item_count: %d.", itemId, item_count );
    }
}

template <class T> void QuestMgr::_AddQuest(uint32 entryid, Quest *qst, uint8 type)
{
    HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* > &olist = _GetList<T>();
    std::list<QuestRelation *>* nlist;
    QuestRelation *ptr = NULL;

    if (olist.find(entryid) == olist.end())
    {
        nlist = new std::list<QuestRelation *>;

        olist.insert(HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* >::value_type(entryid, nlist));
    }
    else
    {
        nlist = olist.find(entryid)->second;
    }

    list<QuestRelation *>::iterator it;
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
    string::size_type pos = str->find(c,0);

    while (pos != string::npos)
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
        QuestXPEntry *xpentry = dbcQuestXP.LookupEntry(quest_level);
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
    plyr->GetSession()->SendPacket(&data);
    sLog.outDebug("WORLD:Sent SMSG_QUESTGIVER_QUEST_FAILED");
}

void QuestMgr::SendQuestUpdateFailedTimer(Quest *pQuest, Player* plyr)
{
    if(!plyr)
        return;

    plyr->GetSession()->OutPacket(SMSG_QUESTUPDATE_FAILEDTIMER, 4, &pQuest->id);
    sLog.outDebug("WORLD:Sent SMSG_QUESTUPDATE_FAILEDTIMER");
}

void QuestMgr::SendQuestUpdateFailed(Quest *pQuest, Player* plyr)
{
    if(!plyr)
        return;

    plyr->GetSession()->OutPacket(SMSG_QUESTUPDATE_FAILED, 4, &pQuest->id);
    sLog.outDebug("WORLD:Sent SMSG_QUESTUPDATE_FAILED");
}

void QuestMgr::SendQuestLogFull(Player* plyr)
{
    if(!plyr)
        return;

    plyr->GetSession()->OutPacket(SMSG_QUESTLOG_FULL);
    sLog.outDebug("WORLD:Sent QUEST_LOG_FULL_MESSAGE");
}

void QuestMgr::BuildQuestFailed(WorldPacket* data, uint32 questid)
{
    data->Initialize(SMSG_QUESTUPDATE_FAILEDTIMER);
    *data << questid;
}

bool QuestMgr::OnActivateQuestGiver(Object* qst_giver, Player* plr)
{
    if(qst_giver->GetTypeId() == TYPEID_GAMEOBJECT && !TO_GAMEOBJECT(qst_giver)->HasQuests())
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
            bValid = TO_GAMEOBJECT(qst_giver)->HasQuests();
            if(bValid)
            {
                q_begin = TO_GAMEOBJECT(qst_giver)->QuestsBegin();
                q_end   = TO_GAMEOBJECT(qst_giver)->QuestsEnd();
            }
        }
        else if(qst_giver->GetTypeId() == TYPEID_UNIT)
        {
            bValid = TO_CREATURE(qst_giver)->HasQuests();
            if(bValid)
            {
                q_begin = TO_CREATURE(qst_giver)->QuestsBegin();
                q_end   = TO_CREATURE(qst_giver)->QuestsEnd();
            }
        }

        if(!bValid)
        {
            sLog.outDebug("QUESTS: Warning, invalid NPC "I64FMT" specified for OnActivateQuestGiver. TypeId: %d.", qst_giver->GetGUID(), qst_giver->GetTypeId());
            return false;
        }

        for(itr = q_begin; itr != q_end; itr++)
            if (sQuestMgr.CalcQuestStatus(plr, *itr) >= QMGR_QUEST_CHAT)
                break;

        if (sQuestMgr.CalcStatus(qst_giver, plr) < QMGR_QUEST_CHAT)
            return false;

        ASSERT(itr != q_end);

        uint32 status = CalcStatus(qst_giver, plr);
        if (status == QMGR_QUEST_FINISHED || ((*itr)->qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE))
        {
            sQuestMgr.BuildOfferReward(&data, (*itr)->qst, qst_giver, 1, plr);
            plr->GetSession()->SendPacket(&data);
            //ss
            sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_OFFER_REWARD." );
        }
        else if (status == QMGR_QUEST_CHAT || status == QMGR_QUEST_AVAILABLE)
        {
            sQuestMgr.BuildQuestDetails(&data, (*itr)->qst, qst_giver, 1, plr);     // 1 because we have 1 quest, and we want goodbye to function
            plr->GetSession()->SendPacket(&data);
            sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_QUEST_DETAILS." );
        }
        else if (status == QMGR_QUEST_NOT_FINISHED)
        {
            sQuestMgr.BuildRequestItems(&data, (*itr)->qst, qst_giver, status);
            plr->GetSession()->SendPacket(&data);
            sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_REQUEST_ITEMS." );
        }
    }
    else
    {
        sQuestMgr.BuildQuestList(&data, qst_giver ,plr);
        plr->GetSession()->SendPacket(&data);
        sLog.Debug( "WORLD"," Sent SMSG_QUESTGIVER_QUEST_LIST." );
    }
    return true;
}

bool QuestMgr::CanStoreReward(Player* plyr, Quest *qst, uint32 reward_slot)
{
    uint32 slotsrequired = 0;
    uint32 available_slots = plyr->GetItemInterface()->CalculateFreeSlots(NULL);

    // Static Item reward
    for(uint32 i = 0; i < 4; i++)
    {
        if(qst->reward_item[i])
        {
            slotsrequired++;
            ItemPrototype *proto = ItemPrototypeStorage.LookupEntry(qst->reward_item[i]);
            if(!proto)
                sLog.outDebug("Invalid item prototype in quest reward! ID %d, quest %d", qst->reward_item[i], qst->id);
            else if(plyr->GetItemInterface()->CanReceiveItem(proto, qst->reward_itemcount[i], NULL))
                return false;
        }
    }

    // Choice Rewards
    if(qst->reward_choiceitem[reward_slot])
    {
        slotsrequired++;
        ItemPrototype *proto = ItemPrototypeStorage.LookupEntry(qst->reward_choiceitem[reward_slot]);
        if(!proto)
            sLog.outDebug("Invalid item prototype in quest reward! ID %d, quest %d", qst->reward_choiceitem[reward_slot], qst->id);
        else if(plyr->GetItemInterface()->CanReceiveItem(proto, qst->reward_choiceitemcount[reward_slot], NULL))
            return false;
    }

    if(available_slots < slotsrequired)
        return false;
    return true;
}

void QuestMgr::LoadExtraQuestStuff()
{
    QuestStorageMap::iterator it = QuestStorage.begin();
    Quest * qst;
    map<uint32, set<uint32> > tmp_map;
    map<uint32, vector<uint32> > loot_map;

    lootmgr.LoadLoot();
    lootmgr.FillObjectLootMap(&loot_map);
    sLog.Debug("QuestMgr","Creating gameobject involved quest map...");

    while(it != QuestStorage.end())
    {
        qst = it->second;

        for( uint32 j = 0; j < 6; j++)
        {
            if(qst->required_item[j])
            {
                map<uint32, vector<uint32> >::iterator tt = loot_map.find(qst->required_item[j]);
                if( tt != loot_map.end() )
                {
                    vector<uint32>::iterator tt2 = tt->second.begin();
                    for( ; tt2 != tt->second.end(); ++tt2 )
                    {
                        // this only applies if the only items under the loot template are quest items
                        LootStore::iterator itr = lootmgr.GOLoot.find((*tt2));
                        bool has_other = false;
                        bool has_quest = true;
                        if( itr != lootmgr.GOLoot.end() )
                        {
                            for(uint32 xx = 0; xx < itr->second.count; ++xx )
                            {
                                if( itr->second.items[xx].item.itemproto == NULL )
                                    continue;

                                if( itr->second.items[xx].item.itemproto->Class != ITEM_CLASS_QUEST )
                                {
                                    has_other = true;
                                    break;
                                }
                                else
                                    has_quest = true;
                            }
                        }

                        if( !has_other && has_quest )
                            tmp_map[(*tt2)].insert(qst->id);
                    }
                }
            }
        }
        it++;
    }

    for(map<uint32, set<uint32> >::iterator itr = tmp_map.begin(); itr != tmp_map.end(); itr++)
    {
        GameObjectInfo *inf = GameObjectNameStorage.LookupEntry(itr->first);
        if( inf == NULL )
            continue;

        m_extraqueststuff_list.push_back(itr->first);
        objmgr.AddInvolvedQuestIds(itr->first, itr->second);
    }

    // load creature starters
    uint32 creature, quest;

    QueryResult * pResult = WorldDatabase.Query("SELECT * FROM creature_quest_starter");
    uint32 pos = 0;
    uint32 total = 0;
    if(pResult)
    {
        total = pResult->GetRowCount();
        do
        {
            Field *data = pResult->Fetch();
            creature = data[0].GetUInt32();
            quest = data[1].GetUInt32();

            qst = GetQuestPointer(quest);
            if(!qst)
            {
                sLog.Warning("QuestMgr","Tried to add starter to npc %u for non-existant quest %u.", creature, quest);
                if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                {
                    WorldDatabase.Execute("DELETE FROM creature_quest_starter where quest = '%u'", quest);
                }
                total--;
            }
            else
                _AddQuest<Creature>(creature, qst, 1);  // 1 = starter

        } while(pResult->NextRow());
        delete pResult;
    }
    sLog.Notice("QuestMgr","Marked %u creatures as quest starter", total);

    pResult = WorldDatabase.Query("SELECT * FROM creature_quest_finisher");
    pos = total = 0;
    if(pResult)
    {
        total = pResult->GetRowCount();
        do
        {
            Field *data = pResult->Fetch();
            creature = data[0].GetUInt32();
            quest = data[1].GetUInt32();

            qst = GetQuestPointer(quest);
            if(!qst)
            {
                sLog.Warning("QuestMgr","Tried to add finisher to npc %d for non-existant quest %d.", creature, quest);
                if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                {
                    WorldDatabase.Execute("DELETE FROM creature_quest_finisher where quest = '%u'", quest);
                }
                total--;
            }
            else
                _AddQuest<Creature>(creature, qst, 2);  // 2 = starter

        } while(pResult->NextRow());
        delete pResult;
    }
    sLog.Notice("QuestMgr","Marked %u creatures as quest finisher", total);

    pResult = WorldDatabase.Query("SELECT * FROM gameobject_quest_starter");
    pos = total = 0;
    if(pResult)
    {
        total = pResult->GetRowCount();
        do
        {
            Field *data = pResult->Fetch();
            creature = data[0].GetUInt32();
            quest = data[1].GetUInt32();

            qst = GetQuestPointer(quest);
            if(!qst)
            {
                sLog.Warning("QuestMgr","Tried to add starter to go %d for non-existant quest %d.", creature, quest);
                if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                {
                    WorldDatabase.Execute("DELETE FROM gameobject_quest_starter where quest = '%u'", quest);
                }
                total--;
            }
            else
                _AddQuest<GameObject>(creature, qst, 1);  // 1 = starter

        } while(pResult->NextRow());
        delete pResult;
        sLog.Notice("QuestMgr","Marked %u gameobjects as quest starter", total);
    }

    pResult = WorldDatabase.Query("SELECT * FROM gameobject_quest_finisher");
    pos = total = 0;
    if(pResult)
    {
        total = pResult->GetRowCount();
        do
        {
            Field *data = pResult->Fetch();
            creature = data[0].GetUInt32();
            quest = data[1].GetUInt32();

            qst = GetQuestPointer(quest);
            if(!qst)
            {
                sLog.Warning("QuestMgr","Tried to add finisher to go %d for non-existant quest %d.\n", creature, quest);
                if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                {
                    WorldDatabase.Execute("DELETE FROM gameobject_quest_finisher where quest = '%u'", quest);
                }
                total--;
            }
            else
                _AddQuest<GameObject>(creature, qst, 2);  // 2 = finish

        } while(pResult->NextRow());
        delete pResult;
        sLog.Notice("QuestMgr","Marked %u gameobjects as quest finisher", total);
    }

    //load item quest associations
    pResult = WorldDatabase.Query("SELECT * FROM item_quest_association");
    pos = total = 0;
    if( pResult != NULL)
    {
        uint32 item = 0;
        uint8 item_count = 0;
        total = pResult->GetRowCount();
        do
        {
            Field *data = pResult->Fetch();
            item = data[0].GetUInt32();
            quest = data[1].GetUInt32();
            item_count = data[2].GetUInt8();

            qst = GetQuestPointer(quest);
            if(!qst)
            {
                sLog.Warning("QuestMgr","Tried to add association to item %d for non-existant quest %d.", item, quest);
                if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                {
                    WorldDatabase.Execute("DELETE FROM item_quest_association where quest = '%u'", quest);
                }
                total--;
            }
            else
                AddItemQuestAssociation( item, qst, item_count );

        } while( pResult->NextRow() );
        delete pResult;
        sLog.Notice("QuestMgr","Loaded %u item-quest associations", total);
    }

    //Proccess the stuff
    objmgr.ProcessGameobjectQuests();
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
        break;

    default:
        return false;
        break;
    }
}