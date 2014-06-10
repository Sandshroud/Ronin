/***
 * Demonstrike Core
 */

#pragma once

enum LFGTypes
{
    LFG_NONE                    = 0,
    LFG_DUNGEON                 = 1,
    LFG_RAID                    = 2,
    LFG_QUEST                   = 3,
    LFG_ZONE                    = 4,
    LFG_HEROIC_DUNGEON          = 5,
    LFG_RANDOM                  = 6,
    LFG_DAILY_DUNGEON           = 7,
    LFG_DAILY_HEROIC_DUNGEON    = 8,
    LFG_MAX_TYPES               = 9
};

enum LFGLevelGroups
{
    LFG_LEVELGROUP_NONE         = 0,
    LFG_LEVELGROUP_10_UP        = 1,
    LFG_LEVELGROUP_20_UP        = 2,
    LFG_LEVELGROUP_30_UP        = 3,
    LFG_LEVELGROUP_40_UP        = 4,
    LFG_LEVELGROUP_50_UP        = 5,
    LFG_LEVELGROUP_60_UP        = 6,
    LFG_LEVELGROUP_70_UP        = 7,
    LFG_LEVELGROUP_80           = 8,
    LFG_LEVELGROUP_80_UP        = 9,
    NUM_LEVELGROUP              = 10
};

struct LfgRewardInternal
{
    uint32 QuestId;
    uint32 MoneyReward;
    uint32 XPReward;
};

/// Reward info
struct LfgReward
{
    LfgReward(uint32 firstQuest = 0, uint32 firstVarMoney = 0, uint32 firstVarXp = 0, uint32 otherQuest = 0, uint32 otherVarMoney = 0, uint32 otherVarXp = 0)
    {
        reward[0].QuestId = firstQuest;
        reward[0].MoneyReward = firstVarMoney;
        reward[0].XPReward = firstVarXp;
        reward[1].QuestId = otherQuest;
        reward[1].MoneyReward = otherVarMoney;
        reward[1].XPReward = otherVarXp;
    }

    LfgRewardInternal reward[2];
};

#define MAX_LFG_QUEUE_ID 3
#define LFG_MATCH_TIMEOUT 30        // in seconds

typedef std::set<uint32> DungeonSet;

class LfgMatch;
class LfgMgr : public Singleton < LfgMgr >, EventableObject
{
public:
    typedef list<Player*  > LfgPlayerList;

    LfgMgr();
    ~LfgMgr();

    void LoadRandomDungeonRewards();
    LfgReward* GetLFGReward(uint32 dungeon) { return DungeonRewards[dungeon]; };
    bool AttemptLfgJoin(Player* pl, uint32 LfgDungeonId);
    void SetPlayerInLFGqueue(Player* pl,uint32 LfgDungeonId);
    void SetPlayerInLfmList(Player* pl, uint32 LfgDungeonId);
    void RemovePlayerFromLfgQueue(Player* pl,uint32 LfgDungeonId);
    void RemovePlayerFromLfgQueues(Player* pl);
    void RemovePlayerFromLfmList(Player* pl, uint32 LfmDungeonId);
    void UpdateLfgQueue(uint32 LfgDungeonId);
    void SendLfgList(Player* plr, uint32 Dungeon);
    void EventMatchTimeout(LfgMatch * pMatch);
    uint32 GetPlayerLevelGroup(uint32 level);
    DungeonSet GetLevelSet(uint32 level) { return DungeonsByLevel[GetPlayerLevelGroup(level)]; };

    int32 event_GetInstanceId() { return -1; }

protected:
    uint32 MaxDungeonID;
    DungeonSet DungeonsByLevel[NUM_LEVELGROUP];
    std::map< uint32, LfgReward* > DungeonRewards;
    std::map< uint32, LfgPlayerList > m_lookingForGroup;
    std::map< uint32, LfgPlayerList > m_lookingForMore;
    Mutex m_lock;
};

class LfgMatch
{
public:
    set<Player* > PendingPlayers;
    set<Player* > AcceptedPlayers;
    Mutex lock;
    uint32 DungeonId;
    Group * pGroup;

    LfgMatch(uint32 did) : DungeonId(did),pGroup(NULL) { }
};

#define sLfgMgr LfgMgr::getSingleton()
