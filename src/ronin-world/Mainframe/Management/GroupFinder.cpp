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

initialiseSingleton( GroupFinderMgr );

GroupFinderMgr::GroupFinderMgr() : m_maxReqExpansion(0)
{

}

GroupFinderMgr::~GroupFinderMgr()
{

}

void GroupFinderMgr::Initialize()
{
    for(uint32 i = 0; i < dbcLFGDungeons.GetNumRows(); i++)
    {
        if(LFGDungeonsEntry *entry = dbcLFGDungeons.LookupRow(i))
        {
            m_maxReqExpansion = std::max<uint32>(m_maxReqExpansion, entry->reqExpansion);
            m_lfgDungeonsByExpansion.insert(std::make_pair(entry->reqExpansion, entry));
            m_lfgDungeonsByLFGType.insert(std::make_pair(entry->LFGType, entry));
            for(uint32 level = entry->minLevel; level <= entry->maxLevel; ++level)
            {
                m_lfgDungeonsByLevel.insert(std::make_pair(level, entry));
                if( level >= entry->recomMinLevel && level <= entry->recomMaxLevel )
                    m_lfgDungeonsByRecommended.insert(std::make_pair(level, entry));
            }
        }
    }
}

void GroupFinderMgr::LoadRewards()
{
    uint32 count = 0;
    QueryResult* result = WorldDatabase.Query("SELECT * FROM groupfinder_rewards ORDER BY dungeonId");
    if(result != NULL)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 dungeonid = fields[0].GetUInt32();
            uint32 maxLevel = fields[1].GetUInt32();
            // Lookup check
            if(dbcLFGDungeons.LookupEntry(dungeonid) == NULL)
                continue;
            // Check if we already have a reward
            if(m_dungeonRewards[dungeonid].find(maxLevel) != m_dungeonRewards[dungeonid].end())
                continue;

            GroupFinderReward *reward = new GroupFinderReward();
            reward->dungeonId = dungeonid;
            reward->maxLevel = maxLevel;

            // First Reward
            reward->questId[0] = fields[2].GetUInt32();
            reward->moneyReward[0] = fields[3].GetUInt32();
            reward->xpReward[0] = fields[4].GetUInt32();

            // Second reward
            reward->questId[1] = fields[5].GetUInt32();
            reward->moneyReward[1] = fields[6].GetUInt32();
            reward->xpReward[1] = fields[7].GetUInt32();
            m_dungeonRewards[dungeonid].insert(std::make_pair(maxLevel, reward));
            count++;
        }while(result->NextRow());
    }

    sLog.Notice("LfgMgr", "%u LFD rewards loaded.", count);
}

void GroupFinderMgr::AddPlayer(Player *plr)
{

}

void GroupFinderMgr::RemovePlayer(Player *plr)
{

}

bool GroupFinderMgr::GetPlayerQueueStatus(Player *plr, uint8 &status, uint32 &queueId, uint32 &step, std::vector<uint32> *QueueIds, time_t joinTime)
{

    return false;
}

void GroupFinderMgr::BuildRandomDungeonData(Player *plr, WorldPacket *data)
{
    uint8 expansion = plr->GetSession()->GetHighestExpansion();
    size_t pos = data->wpos();
    uint8 count = 0;
    *data << count; // Random Dungeons count
    LFGDungeonMultiMap::iterator begin = m_lfgDungeonsByLFGType.lower_bound(DBC_LFG_TYPE_RANDOM), end = m_lfgDungeonsByLFGType.upper_bound(DBC_LFG_TYPE_RANDOM);
    for(auto itr = begin; itr != end; itr++)
    {
        LFGDungeonsEntry *entry = itr->second;
        if(entry->reqExpansion > expansion)
            continue;
        if(plr->getLevel() < entry->minLevel || plr->getLevel() > entry->maxLevel)
            continue;
        _BuildRandomDungeonData(plr, data, entry);
        ++count;
    }

    for(auto itr = m_currentSeasonDungeons.begin(); itr != m_currentSeasonDungeons.end(); itr++)
    {
        LFGDungeonsEntry *entry = *itr;
        if(entry->reqExpansion > expansion)
            continue;
        if(plr->getLevel() < entry->minLevel || plr->getLevel() > entry->maxLevel)
            continue;
        _BuildRandomDungeonData(plr, data, entry);
        ++count;
    }
    data->put<uint8>(pos, count);
}

void GroupFinderMgr::BuildPlayerLockInfo(Player *plr, WorldPacket *data)
{
    *data << uint32(0);
    if(0 > 0)
    {
        for(uint32 i = 0; i < 0; i++)
        {
            *data << uint32(0); // Dungeon entry
            *data << uint32(0); // Lock status
            *data << uint32(0); // Item Level req
            *data << uint32(0); // dumb player max item level
        }
    }
}

void GroupFinderMgr::_BuildRandomDungeonData(Player *plr, WorldPacket *data, LFGDungeonsEntry *entry)
{
    bool isDone = false;
    if(GroupFinderReward *reward = GetReward(entry->Id, plr->getLevel()))
        if(Quest *qst = sQuestMgr.GetQuestPointer(reward->questId[0]))
            isDone = plr->HasCompletedQuest(qst);

    *data << uint32(entry->Id); // Dungeon ID
    *data << uint8(isDone); // isDone
    *data << uint32(0); // Currency Cap
    *data << uint32(0); // Currency Reward
    for(uint32 i = 0; i < 4; i++)
        *data << uint32(0) << uint32(0); // Reward count / Reward Limit
    *data << uint32(0); // Prize item
    *data << uint32(0); // Completion mask
    *data << uint8(0); // Is bonus available
    // Append our bonus role quest info
    for(uint32 i = 0; i < 3; i++)
        if(!_BuildDungeonQuestData(plr, data, entry->Id, i, isDone))
            *data << uint32(0);

    // Append either linked quest or gold/xp reward
    if(!_BuildDungeonQuestData(plr, data, entry->Id, 0xFF, isDone))
    {
        *data << uint32(0); // Money reward
        *data << uint32(0); // XP reward
        *data << uint8(0); // Reward Count
    }
}

bool GroupFinderMgr::_BuildDungeonQuestData(Player *plr, WorldPacket *data, uint32 dungeonId, uint8 roleIndex, bool isDone)
{
    Quest *qst = NULL;
    if(roleIndex == 0xFF && ((qst = GetDungeonQuest(dungeonId, plr->getLevel(), isDone)) == NULL))
        return false;
    else if(roleIndex != 0xFF)
    {
        uint32 roleMask;
        if((qst = GetCallToArmsRole(dungeonId, roleIndex, roleMask)) == NULL)
            return false;
        *data << roleMask;
    }

    *data << sQuestMgr.GenerateRewardMoney(plr, qst);
    *data << uint32(float2int32(sQuestMgr.GenerateQuestXP(plr, qst) * sWorld.getRate(RATE_QUESTXP)));
    if(uint8 count = qst->count_reward_item + 0/*Currency reward count*/)
    {
        *data << uint8(count);
        for(uint8 i = 0; i < 4; i++)
        {
            if(uint32 reward = 0)//qst->reward_currency[i])
            {
                *data << uint32(reward) << uint32(0);
                *data << uint32(0);//qst->reward_currencycount[i]);
                *data << uint8(1);
            }
        }

        for(uint8 i = 0; i < 4; i++)
        {
            if(uint32 reward = qst->reward_item[i])
            {
                ItemPrototype *proto = sItemMgr.LookupEntry(reward);
                *data << uint32(reward) << uint32(proto ? proto->DisplayInfoID : NULL);
                *data << uint32(qst->reward_itemcount[i]) << uint8(0);
            }
        }

    } else *data << uint8(0);
    return true;
}

Quest *GroupFinderMgr::GetDungeonQuest(uint32 dungeonId, uint32 level, bool secondary)
{
    Quest *ret = NULL;
    if(GroupFinderReward *reward = GetReward(dungeonId, level))
        if(secondary || (ret = sQuestMgr.GetQuestPointer(reward->questId[0])) == NULL)
            ret = sQuestMgr.GetQuestPointer(reward->questId[1]);
    return ret;
}

Quest *GroupFinderMgr::GetCallToArmsRole(uint32 dungeonId, uint8 roleIndex, uint32 &roleMask)
{
    return NULL;
}
