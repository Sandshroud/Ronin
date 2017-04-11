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

class GroupFinderMgr : public Singleton < GroupFinderMgr >
{
public:
    GroupFinderMgr();
    ~GroupFinderMgr();

    void Initialize(); // Initialize is all parsing, no database loading
    void LoadRewards();

    // Player Interaction
    void AddPlayer(Player *plr);
    void RemovePlayer(Player *plr);

    bool GetPlayerQueueStatus(Player *plr, uint8 &status, uint32 &queueId, uint32 &step, std::vector<uint32> *QueueIds, time_t joinTime);

    void BuildRandomDungeonData(Player *plr, WorldPacket *data);
    void BuildPlayerLockInfo(Player *plr, WorldPacket *data);

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

    uint32 m_maxReqExpansion;
    std::vector<LFGDungeonsEntry*> m_currentSeasonDungeons;

    std::map<uint32, Loki::AssocVector<uint32, GroupFinderReward*>> m_dungeonRewards;

    typedef std::multimap<uint32, LFGDungeonsEntry*> LFGDungeonMultiMap;
    LFGDungeonMultiMap m_lfgDungeonsByLevel;
    LFGDungeonMultiMap m_lfgDungeonsByRecommended;
    LFGDungeonMultiMap m_lfgDungeonsByExpansion;
    LFGDungeonMultiMap m_lfgDungeonsByLFGType;
};

#define sGroupFinder GroupFinderMgr::getSingleton()
