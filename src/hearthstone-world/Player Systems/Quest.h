/***
 * Demonstrike Core
 */

#pragma once

using namespace std;

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
#pragma pack(PRAGMA_PACK)

struct Quest
{
    uint32 id;

    char *qst_title;
    char *qst_details;
    char *qst_objectivetext;
    char *qst_completiontext;
    char *qst_incompletetext;
    char *qst_endtext;
    char *qst_objectivetexts[4];

    uint32 qst_zone_id;
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

    uint32 required_spell;
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
};

#pragma pack(PRAGMA_POP)

enum QUEST_MOB_TYPES
{
    QUEST_MOB_TYPE_CREATURE = 0,
    QUEST_MOB_TYPE_GAMEOBJECT = 1,
};

#define CALL_QUESTSCRIPT_EVENT(id, func) if(sScriptMgr.GetQuestScript(id) != NULL)\
    sScriptMgr.GetQuestScript(id)->func

enum QuestCompletionStatus
{
    QUEST_STATUS__INCOMPLETE    = 0,
    QUEST_STATUS__COMPLETE      = 1,
    QUEST_STATUS__FAILED        = 2
};

class SERVER_DECL QuestLogEntry : public EventableObject
{
    friend class QuestMgr;

public:
    QuestLogEntry();
    ~QuestLogEntry();

    HEARTHSTONE_INLINE Quest* GetQuest() { return m_quest; };
    void Init(Quest* quest, Player* plr, uint32 slot);

    bool CanBeFinished();
    void SubtractTime(uint32 value);
    void SaveToDB(QueryBuffer * buf);
    bool LoadFromDB(Field *fields);
    void UpdatePlayerFields();

    void SetAreaTrigger(uint32 i);
    void SetMobCount(uint32 i, uint32 count);
    void SetPlayerSlainCount(uint32 count);

    bool IsUnitAffected(Unit* target);
    HEARTHSTONE_INLINE bool IsCastQuest() { return iscastquest;}
    void AddAffectedUnit(Unit* target);
    void ClearAffectedUnits();

    void SetSlot(int32 i);
    void Finish();
    uint32 GetSlot() { return m_slot; };

    void SendQuestComplete();
    void SendUpdateAddKill(uint32 i);
    HEARTHSTONE_INLINE uint32 GetPlayerSlainCount() { return m_player_slain; }
    HEARTHSTONE_INLINE uint32 GetMobCount(uint32 i) { return m_mobcount[i]; }
    HEARTHSTONE_INLINE uint32 GetCrossedAreaTrigger(uint32 i) { return m_areatriggers[i]; }
    HEARTHSTONE_INLINE uint32 GetTimeLeft() { return m_time_left; }
    uint32 GetRequiredSpell();

    HEARTHSTONE_INLINE uint32 GetBaseField(uint32 slot)
    {
        return PLAYER_QUEST_LOG_1_1 + (slot * 5);
    }

    uint32 Quest_Status;

private:
    bool mInitialized;
    bool mDirty;

    Quest *m_quest;
    Player* m_plr;

    uint32 m_mobcount[4];
    uint32 m_areatriggers[4];

    uint32 m_player_slain;

    std::set<uint64> m_affected_units;
    bool iscastquest;

    uint32 m_time_left;
    int32 m_slot;
};
