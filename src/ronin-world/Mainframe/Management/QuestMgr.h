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

#pragma once

#define QUEST_LOG_COUNT 50

enum QUEST_STATUS
{
    QMGR_QUEST_NOT_AVAILABLE        = 0x00,  // There aren't quests avaiable.                | "No Mark"
    QMGR_QUEST_AVAILABLELOW_LEVEL   = 0x02,  // Quest available, and your level isnt enough. | "Gray Exclamation Mark !"
    QMGR_QUEST_CHAT                 = 0x04,  // Quest available it shows a talk baloon.      | "Yellow ! Mark"
    QMGR_QUEST_NOT_FINISHED         = 0x20,  // Quest isnt finished yet.                     | "Gray Question ? Mark"
    QMGR_QUEST_AVAILABLE            = 0x100, // Quest available, show the ! Mark.            | "Yellow ! Mark"
    QMGR_QUEST_FINISHED_LOWLEVEL    = 0x200, // Quest has been finished.                     | "No Minimap Mark"
    QMGR_QUEST_FINISHED             = 0x400, // Quest has been finished.                     | "Yellow Question Minimap Mark"
};

enum QUESTGIVER_QUEST_TYPE
{
    QUESTGIVER_QUEST_START  = 0x01,
    QUESTGIVER_QUEST_END    = 0x02,
};

enum QUEST_TYPE
{
    QUEST_GATHER    = 0x01,
    QUEST_SLAY      = 0x02,
};

enum QUEST_FLAG
{
    QUEST_FLAG_NONE             = 0,
    QUEST_FLAG_DELIVER          = 1,
    QUEST_FLAG_KILL             = 2,
    QUEST_FLAG_SPEAKTO          = 4,
    QUEST_FLAG_REPEATABLE       = 8,
    QUEST_FLAG_EXPLORATION      = 16,
    QUEST_FLAG_TIMED            = 32,
    QUEST_FLAG_REPUTATION       = 128,
    QUEST_FLAG_DAILY            = 4096,
    QUEST_FLAG_ACTIVATES_PVP    = 8192,
    QUEST_FLAG_AUTOCOMPLETE     = 65536,
    QUEST_FLAG_AUTO_ACCEPT      = 524288
};

enum FAILED_REASON
{
    FAILED_REASON_FAILED            = 0,
    FAILED_REASON_INV_FULL          = 4,
    FAILED_REASON_DUPE_ITEM_FOUND   = 17,
};

enum QUEST_SHARE
{
    QUEST_SHARE_MSG_SHARING_QUEST           = 0,
    QUEST_SHARE_MSG_CANT_TAKE_QUEST         = 1,
    QUEST_SHARE_MSG_ACCEPT_QUEST            = 2,
    QUEST_SHARE_MSG_REFUSE_QUEST            = 3,
    QUEST_SHARE_MSG_BUSY                    = 4,
    QUEST_SHARE_MSG_LOG_FULL                = 5,
    QUEST_SHARE_MSG_HAVE_QUEST              = 6,
    QUEST_SHARE_MSG_FINISH_QUEST            = 7,
    QUEST_SHARE_MSG_CANT_SHARE_TODAY        = 8,
    QUEST_SHARE_MSG_QUEST_TIMER_FINISHED    = 9,
    QUEST_SHARE_MSG_NOT_IN_PARTY            = 10,
};

enum QUEST_MOB_TYPES
{
    QUEST_MOB_TYPE_CREATURE = 0,
    QUEST_MOB_TYPE_GAMEOBJECT = 1,
};

enum QuestCompletionStatus : uint8
{
    QUEST_STATUS__INCOMPLETE    = 0,
    QUEST_STATUS__COMPLETE      = 1,
    QUEST_STATUS__FAILED        = 2
};

enum RepeatableTypes
{
    UNREPEATABLE_QUEST  = 0,
    REPEATABLE_QUEST    = 1,
    REPEATABLE_DAILY    = 2,
    REPEATABLE_WEEKLY   = 3,
};

#pragma pack(PRAGMA_PACK)

struct QuestPOI
{
    QuestPOI() : questId(0), PoIID(0), questObjectIndex(0), mapId(0), areaId(0), MapFloorId(0) {}

    uint32 questId, PoIID;
    int32 questObjectIndex;
    uint32 mapId;
    uint32 areaId;
    uint32 MapFloorId;
    std::vector<std::pair<int32, int32>> points;
};

struct Quest
{
    uint32 id;

    char *qst_title;
    char *qst_details;
    char *qst_objectivetext;
    char *qst_completiontext;
    char *qst_incompletetext;
    char *qst_endtext;
    char *qst_finishedtext;
    char *qst_objectivetexts[4];

    uint32 qst_zone_id;
    uint32 qst_accept_type;
    uint32 qst_sort;
    uint32 qst_type;
    uint32 qst_flags;
    uint32 qst_min_level;
    uint32 qst_max_level;
    uint32 qst_suggested_players;

    uint32 qst_previous_quest_id;
    uint32 qst_next_quest_id;

    int32 qst_start_phase;
    int32 qst_complete_phase;

    uint8 qst_is_repeatable;

    // Quest rewards
    uint32 srcitem;
    uint16 srcitemcount;

    uint32 reward_choiceitem[6];
    uint16 reward_choiceitemcount[6];

    uint32 reward_item[4];
    uint16 reward_itemcount[4];

    uint32 receive_items[4];
    uint16 receive_itemcount[4];

    uint32 reward_repfaction[5];
    int16 reward_repvalue[5];
    int16 reward_replimit[5];

    uint8 reward_title;
    uint32 reward_money;
    uint32 reward_maxlvlmoney;
    uint32 reward_xp;
    uint32 reward_spell;
    uint16 reward_talents;
    uint32 reward_cast_on_player;
    uint32 reward_xp_index;
    // End quest rewards

    // Quest Objectives
    uint32 required_item[6];
    uint16 required_itemcount[6];

    uint32 required_mob[4];
    uint8 required_mobtype[4];
    uint16 required_mobcount[4];
    uint32 required_areatriggers[4];

    uint32 required_spell[4];
    uint32 required_player_kills;

    uint32 required_timelimit;
    uint32 required_money;
    float required_point_x;
    float required_point_y;
    uint32 required_point_mapid;
    uint32 required_point_radius;
    // End quest objectives

    // Quest Requirements
    int8 required_team;
    uint16 required_races;
    uint16 required_class;
    uint16 required_tradeskill;
    uint16 required_tradeskill_value;
    uint16 required_rep_faction;
    uint16 required_rep_value;

    uint32 required_quests[4];
    bool required_quest_one_or_all;
    // End quest requirements

    //Calculated variables (not in db)
    uint8 count_receiveitems;
    uint8 count_reward_choiceitem;
    uint8 count_reward_item;
    uint8 count_required_item;
    uint8 count_required_mob;
    uint8 count_requiredareatriggers;
    uint8 count_requiredquests;

    std::vector<QuestPOI*> quest_poi;
};

#pragma pack(PRAGMA_POP)

struct QuestRelation
{
    Quest *qst;
    uint8 type;
};

struct QuestAssociation
{
    Quest *qst;
    uint8 item_count;
};

class Item;

typedef std::map<uint32, Quest*> QuestStorageMap;
typedef std::list<QuestRelation *> QuestRelationList;
typedef std::list<QuestAssociation *> QuestAssociationList;
typedef std::map<uint32, std::vector<QuestPOI*> > QuestPOIStorageMap;
typedef std::map<uint32, QuestRelationList* > QuestRelationListMap;
typedef std::map<uint32, QuestAssociationList* > QuestAssociationListMap;

class SERVER_DECL QuestMgr :  public Singleton < QuestMgr >
{
public:
    QuestMgr();
    ~QuestMgr();

    Mutex LoadLocks;
    void LoadQuests(); // Actually load our quests, do not call outside startup.
    void AppendQuestList(Object *obj, Player *plr, uint32 &count, WorldPacket *packet);

    bool QuestExists(uint32 entry) { if(QuestStorage.find(entry) != QuestStorage.end()) return true; return false; };
    Quest* GetQuestPointer(uint32 entry) { if(QuestStorage.find(entry) != QuestStorage.end()) return QuestStorage.at(entry); return NULL; };

    QuestStorageMap::iterator GetQuestStorageBegin() { return QuestStorage.begin(); };
    QuestStorageMap::iterator GetQuestStorageEnd() { return QuestStorage.end(); };

    uint32 PlayerMeetsReqs(Player* plr, Quest* qst, bool skiplevelcheck, bool skipprevquestcheck);
    static bool PlayerCanComplete(Player *plr, Quest *qst, QuestLogEntry *questLog);

    uint32 CalcStatus(Object* quest_giver, Player* plr);
    uint32 CalcQuestStatus(Player* plr, QuestRelation* qst);
    uint32 CalcQuestStatus(Player* plr, Quest* qst, uint8 type, bool interaction, bool skiplevelcheck = false, bool skipPrevQuestCheck = false);
    uint32 ActiveQuestsCount(Object* quest_giver, Player* plr);

    //Packet Forging...
    void BuildGossipQuest(WorldPacket *data, Quest *qst, uint32 qst_status, Player *plr);
    void BuildOfferReward(WorldPacket* data,Quest* qst, Object* qst_giver, uint32 menutype, Player* plr);
    void BuildQuestDetails(WorldPacket* data, Quest* qst, Object* qst_giver, uint32 menutype, Player* plr);
    void BuildRequestItems(WorldPacket* data, Quest* qst, Object* qst_giver, uint32 status, Player *plr);
    void BuildQuestComplete(Player* , Quest* qst);
    void BuildQuestList(WorldPacket* data, Object* qst_giver, Player* plr);
    bool OnActivateQuestGiver(Object* qst_giver, Player* plr);

    void SendQuestUpdateAddKill(Player* plr, uint32 questid, uint32 entry, uint32 count, uint32 tcount, uint64 guid);
    void BuildQuestUpdateAddItem(WorldPacket* data, uint32 itemid, uint32 count);
    void BuildQuestUpdateComplete(WorldPacket* data, Quest* qst);
    void BuildQuestFailed(WorldPacket* data, uint32 questid);
    void SendPushToPartyResponse(Player* plr, Player* pTarget, uint32 response);

    bool OnGameObjectActivate(Player* plr, GameObject* go);
    void _OnPlayerKill(Player* plr, uint32 creature_entry);
    void OnPlayerKill(Player* plr, Creature* victim);
    void OnPlayerCast(Player* plr, uint32 spellid, WoWGuid& victimguid);
    void OnPlayerItemPickup(Player* plr, Item* item, uint32 pickedupstacksize);
    void OnPlayerDropItem(Player* plr, uint32 entry);
    void OnPlayerExploreArea(Player* plr, uint32 AreaID);
    void OnPlayerAreaTrigger(Player* plr, uint32 areaTrigger);
    void OnPlayerSlain(Player* plr, Player* victim);
    void OnQuestAccepted(Player* plr, Quest* qst, Object* qst_giver);
    void OnQuestFinished(Player* plr, Quest* qst, Object* qst_giver, uint32 reward_slot);
    bool SkippedKills( uint32 QuestID );

    void GiveQuestRewardReputation(Player* plr, Quest* qst, Object* qst_giver);
    void GiveQuestTitleReward(Player* plr, Quest* qst);

    uint32 GenerateQuestXP(Player* pl, Quest *qst);
    uint32 GenerateRewardMoney(Player* pl, Quest *qst);
    uint32 GenerateMaxLvlRewardMoney(Player* pl, Quest *qst);

    void SendQuestFailed(FAILED_REASON failed, Quest *qst, Player* plyr);
    void SendQuestUpdateFailed(Quest *pQuest, Player* plyr);
    void SendQuestUpdateFailedTimer(Quest *pQuest, Player* plyr);
    void SendQuestLogFull(Player* plyr);

    void LoadNPCQuests(Creature* qst_giver);
    void LoadGOQuests(GameObject* go);

    bool hasQuests(WorldObject *curObj);
    QuestRelationList* GetCreatureQuestList(uint32 entryid);
    QuestRelationList* GetGOQuestList(uint32 entryid);

    RONIN_INLINE bool IsQuestRepeatable(Quest *qst) { return (qst->qst_is_repeatable != 0 ? true : false); }
    RONIN_INLINE bool IsQuestDaily(Quest *qst) { return (qst->qst_is_repeatable == 2 ? true : false); }

    bool CanStoreReward(Player* plyr, Quest *qst, uint32 reward_slot);

    RONIN_INLINE int32 QuestHasMob(Quest* qst, uint32 mob)
    {
        for(uint32 i = 0; i < 4; i++)
            if(qst->required_mob[i] == mob)
                return qst->required_mobcount[i];
        return -1;
    }

    RONIN_INLINE int32 GetOffsetForMob(Quest *qst, uint32 mob)
    {
        for(uint32 i = 0; i < 4; i++)
            if(qst->required_mob[i] == mob)
                return i;
        return -1;
    }

    RONIN_INLINE int32 GetOffsetForItem(Quest *qst, uint32 itm)
    {
        for(uint32 i = 0; i < 6; i++)
            if(qst->required_item[i] == itm)
                return i;
        return -1;
    }

private:
    QuestStorageMap QuestStorage;

    QuestRelationListMap m_npc_quests;
    QuestRelationListMap m_obj_quests;
    QuestRelationListMap m_itm_quests;
    std::vector<QuestPOI*> m_questPOI;

    std::map<uint32, uint32> m_ObjectLootQuestList;

    template <class T> void _AddQuest(uint32 entryid, Quest *qst, uint8 type);

    template <class T> QuestRelationListMap& _GetList();

    // Quest Loading
    void _RemoveChar(char* c, std::string *str);
    void _CleanLine(std::string *str);
};

template<> RONIN_INLINE QuestRelationListMap& QuestMgr::_GetList<Creature>()
    {return m_npc_quests;}
template<> RONIN_INLINE QuestRelationListMap& QuestMgr::_GetList<GameObject>()
    {return m_obj_quests;}
template<> RONIN_INLINE QuestRelationListMap& QuestMgr::_GetList<Item>()
    {return m_itm_quests;}


#define sQuestMgr QuestMgr::getSingleton()
