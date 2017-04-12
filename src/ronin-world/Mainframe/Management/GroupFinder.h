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

struct GroupFinderStatus
{

};

struct GroupFinderReward
{
    uint32 dungeonId;
    uint32 maxLevel;
    uint32 questId[2];
    uint32 moneyReward[2];
    uint32 xpReward[2];
};

/// Determines the type of instance
enum LfgType
{
    DBC_LFG_TYPE_NONE   = 0,
    DBC_LFG_TYPE_DUNGEON= 1,
    DBC_LFG_TYPE_RAID   = 2,
    DBC_LFG_TYPE_HEROIC = 5,
    DBC_LFG_TYPE_RANDOM = 6
};

enum
{
    ROLEMASK_NONE   = 0,
    ROLEMASK_LEADER = 0x01,
    ROLEMASK_TANK   = 0x02,
    ROLEMASK_HEALER = 0x04,
    ROLEMASK_DPS    = 0x08,

    ROLEMASK_ROLE_TYPE = ROLEMASK_TANK|ROLEMASK_HEALER|ROLEMASK_DPS,
};

enum ClientLFGErrors
{
    LFG_ERROR_NONE                      = 0x00,
    LFG_ERROR_ROLECHECK_FAILED          = 0x1B,
    LFG_ERROR_GROUP_FULL                = 0x1C,
    LFG_ERROR_INTERNAL                  = 0x1E,
    LFG_ERROR_REQ_NOT_MET               = 0x1F,
    LFG_ERROR_FINDER_LIST_ERR           = 0x20,
    LFG_ERROR_CROSSREALM_LOCKED         = 0x21,
    LFG_ERROR_MEMBER_DISCONNECTED       = 0x22,
    LFG_ERROR_MEMBER_INFO_LOST          = 0x23,
    LFG_ERROR_FINDER_LIST_ERROR2        = 0x24,
    LFG_ERROR_LEADER_DESERTER           = 0x25,
    LFG_ERROR_MEMBER_DESERTER           = 0x26,
    LFG_ERROR_GF_COOLDOWN               = 0x27,
    LFG_ERROR_MEMBER_GF_COOLDOWN        = 0x28,
    LFG_ERROR_GROUP_TOO_LARGE           = 0x29,
    LFG_ERROR_IN_BG_QUEUE               = 0x2A,
    LFG_ERROR_ROLE_CHECK_FAILED_EXT     = 0x2B,
};

enum ClientLFGState
{
    LFG_STATE_NONE,
    LFG_STATE_ROLECHECK,
    LFG_STATE_INQUEUE,
    LFG_STATE_PROPOSAL,
    LFG_STATE_VOTE,
    LFG_STATE_ACTIVE,
    LFG_STATE_COMPLETE,
    LFG_STATE_RAIDFINDER
};

enum ClientLFGUpdateStep
{
    LFG_STEP_NONE                   = 0,
    LFG_STEP_LEADER_ASSIGN          = 1,
    LFG_STEP_ROLECHECK_CANCELLED    = 4,
    LFG_STEP_JOIN_QUEUE             = 6,
    LFG_STEP_ROLECHECK_FAILED       = 7,
    LFG_STEP_REMOVED_FROM_QUEUE     = 8,
    LFG_STEP_PROPOSAL_FAILED        = 9,
    LFG_STEP_PROPOSAL_REFUSED       = 10,
    LFG_STEP_GROUP_FOUND            = 11,
    LFG_STEP_ADDED_TO_QUEUE         = 13,
    LFG_STEP_PROPOSAL_BEGIN         = 14,
    LFG_STEP_UPDATE_STATE           = 15,
    LFG_STEP_GROUP_MEMBER_OFFLINE   = 16,
    LFG_STEP_GROUP_DISBAND_PENDING  = 17,
    LFG_STEP_GROUP_JOIN_QUEUE       = 24,
    LFG_STEP_DUNGEON_COMPLETE       = 25,
    LFG_STEP_PARTY_ROLE_FAILED      = 43,
    LFG_STEP_LFG_OBJECTIVE_FAILED   = 45
};

static const uint32 dungeonTeamSize = 5;

class GroupFinderMgr : public Singleton < GroupFinderMgr >
{
public:
    GroupFinderMgr();
    ~GroupFinderMgr();

    void Initialize(); // Initialize is all parsing, no database loading
    void LoadRewards();

    void Update(uint32 msTime, uint32 uiDiff);

    // Player Interaction
    void AddPlayer(Player *plr);
    void RemovePlayer(Player *plr);

    void TeleportPlayer(Player *plr, bool toDungeon);

    // Status functions
    bool GetPlayerQueueStatus(Player *plr, uint8 &status, uint32 &queueId, uint32 &step, std::vector<uint32> **QueueIds, time_t &joinTime);

    // Interaction functions
    void UpdateRoles(Player *plr, uint8 roleMask);

    void HandleDungeonJoin(Player *plr, uint32 roleMask, std::vector<uint32> *dungeonSet, std::string comment);
    void HandleDungeonLeave(Player *plr, WoWGuid guid, uint32 queueId);

    void HandleRaidJoin(Player *plr, uint32 roleMask, std::vector<uint32> *raidSet, std::string comment);
    void HandleRaidLeave(Player *plr, WoWGuid guid, uint32 queueId);

    void UpdateProposal(Player *plr, uint32 proposalId, bool result, WoWGuid guid, WoWGuid guid2);
    void UpdateBootVote(Player *plr, bool vote);
    void UpdateComment(Player *plr, std::string comment);

    // Packet building functions
    void BuildRandomDungeonData(Player *plr, WorldPacket *data);
    void BuildPlayerLockInfo(Player *plr, WorldPacket *data);

    void SendQueueCommandResult(Player *plr, uint8 type, uint32 queueId, uint32 queueStatus, bool groupUnk, std::vector<uint32> *dungeonSet, time_t unkTime, std::string unkComment);

protected:
    void _BuildRandomDungeonData(Player *plr, WorldPacket *data, LFGDungeonsEntry *entry);
    bool _BuildDungeonQuestData(Player *plr, WorldPacket *data, uint32 dungeonId, uint8 roleIndex, bool isDone);

    GroupFinderReward *GetReward(uint32 dungeonId, uint32 level)
    {
        std::map<uint32, Loki::AssocVector<uint32, GroupFinderReward*>>::iterator itr;
        if((itr = m_dungeonRewards.find(dungeonId)) != m_dungeonRewards.end())
        {
            GroupFinderReward *ret = NULL;
            Loki::AssocVector<uint32, GroupFinderReward*>::iterator itr2 = itr->second.begin();
            while(itr2 != itr->second.end())
            {   // First is max level, make sure it is equal or higher, then find the lowest one we fit in
                if((*itr2).first >= level && (ret == NULL || ret->maxLevel > (*itr2).first))
                    ret = (*itr2).second;
                ++itr2;
            }
            return ret;
        }
        return NULL;
    }

private:
    Quest *GetDungeonQuest(uint32 dungeonId, uint32 level, bool secondary);
    Quest *GetCallToArmsRole(uint32 dungeonId, uint8 roleIndex, uint32 &roleMask);

    uint32 m_updateTimer;
    uint32 m_maxReqExpansion;
    std::vector<LFGDungeonsEntry*> m_currentSeasonDungeons;

    std::map<uint32, Loki::AssocVector<uint32, GroupFinderReward*>> m_dungeonRewards;

    typedef std::multimap<uint32, LFGDungeonsEntry*> LFGDungeonMultiMap;
    LFGDungeonMultiMap m_lfgDungeonsByLevel;
    LFGDungeonMultiMap m_lfgDungeonsByRecommended;
    LFGDungeonMultiMap m_lfgDungeonsByExpansion;
    LFGDungeonMultiMap m_lfgDungeonsByLFGType;

    ////////////////////////////////
    /// Dungeon Queue functionality
    ////////////////////////////////
private: // None of this needs to be public
    struct QueueGroup;
    struct QueueGroupHolder { QueueGroup *group; };

    struct QueueGroup
    {
        uint8 groupType;
        uint8 groupTeam;
        uint8 queueState;
        uint32 queueId;
        uint32 queueStep;
        time_t timeStamp;
        std::string comment;
        std::vector<uint32> dungeonIds;

        std::vector<WoWGuid> members;
        Loki::AssocVector<WoWGuid, uint8> memberRoles;
        std::vector<QueueGroupHolder*> groupHolders;
    };
    typedef std::map<uint32, QueueGroup*> QueueGroupIdStorage;
    typedef std::map<WoWGuid, QueueGroup*> QueueGroupGuidStorage;

    Mutex queueIdLock;
    uint32 m_queueIdHigh;
    RONIN_INLINE uint32 _GenerateQueueId() { queueIdLock.Acquire(); uint32 lockId = ++m_queueIdHigh; queueIdLock.Release(); return lockId; }

    void _CleanupQueueGroup(QueueGroup *group);
    void _SendLFGJoinResult(Player *plr, uint8 result, QueueGroup *group);

    Mutex _queueGroupLock;
    QueueGroupIdStorage m_queueGroupMap;
    QueueGroupGuidStorage m_queueGroupPlayerMap;
    std::vector<QueueGroup*> m_queueGroupDeletionQueue;

    struct QueueGroupStack
    {
        bool needCheck;
        uint32 roleMask;
        std::vector<QueueGroupHolder*> m_groupQueues;
        std::vector<QueueGroupHolder*> m_singleQueues;
    };
    typedef std::map<uint32, QueueGroupStack*> DungeonGroupStackMap;

    uint8 updateTeamIndex;
    DungeonGroupStackMap m_dungeonQueues[2];

protected:
    void _LaunchDungeon(uint32 dungeonId, std::vector<uint32> *groupIds, WoWGuid tank, WoWGuid heal, WoWGuid dps1, WoWGuid dps2, WoWGuid dps3);

};

#define sGroupFinder GroupFinderMgr::getSingleton()
