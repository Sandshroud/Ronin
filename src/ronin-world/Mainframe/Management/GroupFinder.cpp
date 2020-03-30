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

GroupFinderMgr::GroupFinderMgr() : m_updateTimer(0), m_maxReqExpansion(0), updateTeamIndex(0), m_queueIdHigh(0x000000FF), m_propIdHigh(0x00000F0F)
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
            entry->mapEntry = entry->mapId >= 0 ? dbcMap.LookupEntry(entry->mapId) : NULL;

            m_maxReqExpansion = std::max<uint32>(m_maxReqExpansion, entry->reqExpansion);
            m_lfgDungeonsByExpansion.insert(std::make_pair(entry->reqExpansion, entry));
            m_lfgDungeonsByLFGType.insert(std::make_pair(entry->LFGType, entry));
            if(entry->LFGType != DBC_LFG_TYPE_RANDOM && entry->LFGFaction != -1)
                m_lfgDungeonsByLFGFaction.insert(std::make_pair(entry->LFGFaction, entry));
            if(entry->randomCategoryId)
                m_lfgDungeonsByRandomCategoryId.insert(std::make_pair(entry->randomCategoryId, entry));

            // Don't cache raids into our level grouping
            if(entry->LFGType == DBC_LFG_TYPE_RAIDLIST)
                continue;
            for(uint32 level = entry->minLevel; level <= entry->maxLevel; ++level)
            {
                m_lfgDungeonsByLevel.insert(std::make_pair(level, entry));
                if( level >= entry->recomMinLevel && level <= entry->recomMaxLevel )
                    m_lfgDungeonsByRecommended.insert(std::make_pair(level, entry));
            }
        }
    }

    for(uint32 i = 0; i < dbcLFGDungeonsGrouping.GetNumRows(); i++)
    {
        if(LFGDungeonsGroupingEntry *entry = dbcLFGDungeonsGrouping.LookupRow(i))
        {
            LFGDungeonsEntry *dungeon = dbcLFGDungeons.LookupEntry(entry->dungeonId);
            if(dungeon == NULL)
                continue;
            if(dungeon->randomCategoryId == entry->categoryId)
                continue;
            m_lfgDungeonsByRandomCategoryId.insert(std::make_pair(entry->categoryId, dungeon));
        }
    }
}

void GroupFinderMgr::LoadFromDB()
{
    uint32 count = 0;

    // Load group finder rewards
    if(QueryResult* result = WorldDatabase.Query("SELECT * FROM groupfinder_rewards ORDER BY dungeonId"))
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
        delete result;

        sLog.Notice("GroupFinder", "%u GroupFinder rewards loaded.", count);
    }

    // Load group finder dungeons
    if(QueryResult* result = WorldDatabase.Query("SELECT * FROM groupfinder_dungeons ORDER BY dungeonId"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 dungeonid = fields[0].GetUInt32();
            LFGDungeonsEntry *entry = NULL;
            if((entry = dbcLFGDungeons.LookupEntry(dungeonid)) == NULL)
                continue;
            if(m_dungeonData.find(dungeonid) != m_dungeonData.end())
                continue;

            GroupFinderDungeonData *dungeonData = new GroupFinderDungeonData();
            dungeonData->entry = entry;
            dungeonData->reqItemLevel = fields[2].GetUInt32();
            dungeonData->recomItemLevel = fields[3].GetUInt32();
            dungeonData->x = fields[4].GetFloat();
            dungeonData->y = fields[5].GetFloat();
            dungeonData->z = fields[6].GetFloat();
            dungeonData->o = fields[7].GetFloat();
            if(entry->LFGType != DBC_LFG_TYPE_RANDOM && dungeonData->x == 0.f && dungeonData->y == 0.f)
            {
                LocationVector entrance;
                if(!objmgr.GetDungeonEntrance(dungeonData->entry->mapId, &entrance))
                {
                    delete dungeonData;
                    continue;
                }

                dungeonData->x = entrance.x;
                dungeonData->y = entrance.y;
                dungeonData->z = entrance.z;
                dungeonData->o = entrance.o;
            }
            m_dungeonData.insert(std::make_pair(dungeonid, dungeonData));
            count++;
        }while(result->NextRow());
        delete result;

        sLog.Notice("GroupFinder", "%u GroupFinder rewards loaded.", count);
    }
}

bool RoleChoiceHelper(Player *plr, uint8 plrRoleMask, uint32 plrItemLevel, uint8 currentRole, WoWGuid curRole, uint32 curRoleIL, uint8 curRoleMask, WoWGuid otherRole, uint32 otherRoleIL)
{
    // TODO: Check if our target is their position mask only, and we have a secondary, if so push us to secondary based on IL
    // Check if we need a healer instead of a tank
    if(currentRole == ROLEMASK_TANK && !curRole.empty())
    {
        if(plrRoleMask & ROLEMASK_HEALER && (otherRole.empty() || otherRoleIL < plrItemLevel))
            return false;
        if(curRoleMask & ROLEMASK_HEALER && otherRole.empty())
            return true;
    }

    return (curRole.empty() || curRoleIL < plrItemLevel);
}

void GroupFinderMgr::Update(uint32 msTime, uint32 uiDiff)
{
    m_updateTimer += uiDiff;
    if(m_updateTimer < 1000)
        return;

    _queueGroupLock.Acquire();
    // Process proposition timeouts
    std::vector<uint32> failedPropositions;
    for(auto itr = m_propositionTimeouts[updateTeamIndex].begin(); itr != m_propositionTimeouts[updateTeamIndex].end(); itr++)
        if((itr->second = std::max<int32>(0, itr->second - m_updateTimer)) == 0)
            failedPropositions.push_back(itr->first);

    // Time to process through our map of dungeons with queued player groups
    for(DungeonGroupStackMap::iterator itr = m_dungeonQueues[updateTeamIndex].begin(); itr != m_dungeonQueues[updateTeamIndex].end(); itr++)
    {
        // Only allow one queue pop per dungeon iteration
        bool success = false;
        DungeonDataMap::iterator dDItr;
        GroupFinderDungeonData *dungeonData = NULL;
        if((dDItr = m_dungeonData.find(itr->first)) != m_dungeonData.end())
            dungeonData = dDItr->second;

        LFGDungeonsEntry *entry = dbcLFGDungeons.LookupEntry(itr->first);
        QueueGroupStack *stack = itr->second;

        // We have 3 processing types, combining groups, combining a group with singles, or combining singles
        if(stack->m_groupQueues.size())
        {
            // First we're going to process our single queue to group up what we need... Why didn't we preprocess this
            std::set<uint32> talentedPool, tankGroups, tankDPSGroups, healGroups, healDPSGroups, tankHealGroups, dpsGroups;
            std::vector<std::set<uint32>*> setVector;
            // Fill our sets into our set vector for easier removal processing
            setVector.push_back(&talentedPool);
            setVector.push_back(&tankGroups);
            setVector.push_back(&tankDPSGroups);
            setVector.push_back(&healGroups);
            setVector.push_back(&healDPSGroups);
            setVector.push_back(&tankHealGroups);
            setVector.push_back(&dpsGroups);
            // Begin the single queue processing!
            for(std::vector<QueueGroupHolder*>::iterator sItr = stack->m_singleQueues.begin(); sItr != stack->m_singleQueues.end();)
            {
                // If we have a null group, we're queued for cleanup
                if((*sItr)->group == NULL)
                    sItr = stack->m_singleQueues.erase(sItr);
                else
                {   // Check the single group for the player in map
                    QueueGroup *group = (*sItr)->group;
                    ++sItr;

                    uint8 roleMask = (group->memberRoles.begin()->second & ROLEMASK_ROLE_TYPE);
                    // Push us into our individual roles based on mask
                    if(roleMask == (ROLEMASK_TANK|ROLEMASK_HEALER|ROLEMASK_DPS))
                        talentedPool.insert(group->queueId);
                    else if(roleMask == ROLEMASK_TANK)
                        tankGroups.insert(group->queueId);
                    else if(roleMask == (ROLEMASK_TANK|ROLEMASK_DPS))
                        tankDPSGroups.insert(group->queueId);
                    else if(roleMask == (ROLEMASK_TANK|ROLEMASK_HEALER))
                        tankHealGroups.insert(group->queueId);
                    else if(roleMask == ROLEMASK_HEALER)
                        healGroups.insert(group->queueId);
                    else if(roleMask == (ROLEMASK_HEALER|ROLEMASK_DPS))
                        healDPSGroups.insert(group->queueId);
                    else dpsGroups.insert(group->queueId);
                }
            }

            // Create calculated bools ahead of iteration to easily check if we can fill from single queue
            bool noSingleTanks = talentedPool.empty() && tankGroups.empty() && tankDPSGroups.empty();
            bool noSingleHeals = talentedPool.empty() && healGroups.empty() && tankHealGroups.empty();
            bool noSingleDPS = talentedPool.empty() && dpsGroups.empty() && tankDPSGroups.empty() && healDPSGroups.empty();

            // If we have more than one group, try and combine
            if(stack->m_groupQueues.size() > 1)
            {   // If we have more than one group, we're gonna process our queue list to see if we can combine any
                for(std::vector<QueueGroupHolder*>::iterator gItr = stack->m_groupQueues.begin(); gItr != stack->m_groupQueues.end();)
                {
                    if(success == true)
                        break;

                    // If we have a null group, we're queued for cleanup
                    if((*gItr)->group == NULL)
                        gItr = stack->m_groupQueues.erase(gItr);
                    else
                    {   // Check the single group for the player in map
                        QueueGroup *group = (*gItr)->group;
                        ++gItr;

                        std::vector<WoWGuid> gTanks, gTankOnly, gHeals, gHealOnly, gTankHeals, gDPS;
                        for(Loki::AssocVector<WoWGuid, uint8>::iterator mItr = group->memberRoles.begin(); mItr != group->memberRoles.end(); mItr++)
                        {
                            // If we're only tank or heals, insert us into tankHeals grouping
                            if(mItr->second == (ROLEMASK_TANK|ROLEMASK_HEALER))
                            {
                                gTankHeals.push_back(mItr->first);
                                continue;
                            }

                            // Push us into our individual roles based on mask
                            if(mItr->second == ROLEMASK_TANK)
                                gTankOnly.push_back(mItr->first);
                            else if(mItr->second & ROLEMASK_TANK)
                                gTanks.push_back(mItr->first);
                            if(mItr->second == ROLEMASK_HEALER)
                                gHealOnly.push_back(mItr->first);
                            else if(mItr->second & ROLEMASK_HEALER)
                                gHeals.push_back(mItr->first);
                            if(mItr->second & ROLEMASK_DPS)
                                gDPS.push_back(mItr->first);
                        }

                        // If we have 4 members just fill from singles, do that later
                        if(group->memberRoles.size() < 4)
                        {
                            // start at our current iterator to improve processing speed
                            std::vector<QueueGroupHolder*>::iterator tItr = gItr;
                            for(; tItr != stack->m_groupQueues.end(); ++tItr)
                            {
                                QueueGroup *tGroup = (*tItr)->group;
                                // Ignore us and any invalid groups for now, main iteration will clean those up
                                if(tGroup == group || tGroup == NULL)
                                    continue;
                                // Now we need to see if our target group is compatible with our main group
                                std::vector<WoWGuid> tTanks, tTankOnly, tHeals, tHealOnly, tTankHeals, tDPS;
                                for(Loki::AssocVector<WoWGuid, uint8>::iterator mItr = tGroup->memberRoles.begin(); mItr != tGroup->memberRoles.end(); mItr++)
                                {
                                    // If we're only tank or heals, insert us into tankHeals grouping
                                    if(mItr->second == (ROLEMASK_TANK|ROLEMASK_HEALER))
                                    {
                                        tTankHeals.push_back(mItr->first);
                                        continue;
                                    }

                                    // Push us into our individual roles based on mask
                                    if(mItr->second == ROLEMASK_TANK)
                                        tTankOnly.push_back(mItr->first);
                                    else if(mItr->second & ROLEMASK_TANK)
                                        tTanks.push_back(mItr->first);
                                    if(mItr->second == ROLEMASK_HEALER)
                                        tHealOnly.push_back(mItr->first);
                                    else if(mItr->second & ROLEMASK_HEALER)
                                        tHeals.push_back(mItr->first);
                                    if(mItr->second & ROLEMASK_DPS)
                                        tDPS.push_back(mItr->first);
                                }

                                // Check tank only count
                                if(gTankOnly.size() && tTankOnly.size())
                                    continue; // Can't have two tanks
                                if(gTankOnly.empty() && tTankOnly.empty() && noSingleTanks)
                                    continue; // We need a tank
                                // Check heal only count
                                if(gHealOnly.size() && tHealOnly.size())
                                    continue; // Can't have two heals
                                if(gHealOnly.empty() && tHealOnly.empty() && noSingleHeals)
                                    continue; // We need a healer
                                if((gDPS.size() + tDPS.size()) < 3 && noSingleDPS)
                                    continue; // We need 3 dps

                                // Check tankHeal only with tankOnly or healOnly
                                if((tTankHeals.size() || gTankHeals.size())
                                    && ((gTankOnly.size() && gHealOnly.size()) || (tTankOnly.size() && tHealOnly.size())
                                    || (gTankOnly.size() && tHealOnly.size()) || (tTankOnly.size() && gHealOnly.size())))
                                    continue; // Can't have two heals or two tanks because someone doesn't want to be a DPS(why tho)
                                // Check if we have too many tank and heal combos
                                if(tTankHeals.size() + gTankHeals.size() > 2)
                                    continue;

                                // Thank god for macros
#define SET_TARGET_IL_CHECK_ETC(t, tIL, iterator)\
                                uint32 itemLevel = 1;/*entry->minItemLevel;*/\
                                if(Player *plr = objmgr.GetPlayer(*iterator))\
                                    itemLevel = plr->GetTotalItemLevel();\
                                if(t.empty() || (tIL < itemLevel))\
                                    t = *mItr, tIL = itemLevel;
                                // End macro

                                std::vector<uint32> groupIds;
                                // Push our group queueIds into our vector
                                groupIds.push_back(group->queueId);
                                groupIds.push_back(tGroup->queueId);
                                // Now we need to assign our roles
                                WoWGuid tank(0), heal(0);
                                std::set<WoWGuid> unselectedDPS;
                                // See if we have a specified tank
                                if(gTankOnly.size())
                                    tank = *gTankOnly.begin();
                                else if(tTankOnly.size())
                                    tank = *tTankOnly.begin();
                                // See if we have a specified healer
                                if(gHealOnly.size())
                                    heal = *gHealOnly.begin();
                                else if(tHealOnly.size())
                                    heal = *tHealOnly.begin();
                                else if(gTankHeals.size() || tTankHeals.size())
                                {   // Check now if we can find a tankHeal healer
                                    uint32 healIL = 0;
                                    for(std::vector<WoWGuid>::iterator mItr = gTankHeals.begin(); mItr != gTankHeals.end(); ++mItr)
                                    { SET_TARGET_IL_CHECK_ETC(heal, healIL, mItr); }

                                    for(std::vector<WoWGuid>::iterator mItr = tTankHeals.begin(); mItr != tTankHeals.end(); ++mItr)
                                    { SET_TARGET_IL_CHECK_ETC(heal, healIL, mItr); }
                                }

                                // Check which tank we pick(the one who isn't a healer)
                                if(tank.empty() && (gTankHeals.size() || tTankHeals.size()))
                                {   // Check to see if we can find a tank that isn't our healer
                                    uint32 tankIL = 0;
                                    for(std::vector<WoWGuid>::iterator mItr = gTankHeals.begin(); mItr != gTankHeals.end(); ++mItr)
                                    {
                                        if((*mItr) == heal)
                                            continue;
                                        SET_TARGET_IL_CHECK_ETC(tank, tankIL, mItr);
                                    }

                                    for(std::vector<WoWGuid>::iterator mItr = tTankHeals.begin(); mItr != tTankHeals.end(); ++mItr)
                                    {
                                        if((*mItr) == heal)
                                            continue;
                                        SET_TARGET_IL_CHECK_ETC(tank, tankIL, mItr);
                                    }
                                }

                                if(heal.empty() && (gHeals.size() || tHeals.size()))
                                {   // Check to see if we can find a healer from either heal groups
                                    uint32 healIL = 0;
                                    for(std::vector<WoWGuid>::iterator mItr = gHeals.begin(); mItr != gHeals.end(); ++mItr)
                                    { SET_TARGET_IL_CHECK_ETC(heal, healIL, mItr) }

                                    for(std::vector<WoWGuid>::iterator mItr = tHeals.begin(); mItr != tHeals.end(); ++mItr)
                                    { SET_TARGET_IL_CHECK_ETC(heal, healIL, mItr) }
                                }

                                if(tank.empty() && (gTanks.size() || tTanks.size()))
                                {   // Check to see if we can find a tank from either tank groups
                                    uint32 tankIL = 0;
                                    for(std::vector<WoWGuid>::iterator mItr = gTanks.begin(); mItr != gTanks.end(); ++mItr)
                                    { SET_TARGET_IL_CHECK_ETC(tank, tankIL, mItr); }

                                    for(std::vector<WoWGuid>::iterator mItr = tTanks.begin(); mItr != tTanks.end(); ++mItr)
                                    { SET_TARGET_IL_CHECK_ETC(tank, tankIL, mItr); }
                                }
#undef SET_TARGET_IL_CHECK_ETC

                                // Fill the rest into our DPS pool
                                for(std::vector<WoWGuid>::iterator mItr = group->members.begin(); mItr != group->members.end(); mItr++)
                                {
                                    WoWGuid member = (*mItr);
                                    if(member == heal || member == tank)
                                        continue;
                                    unselectedDPS.insert(member);
                                }
                                for(std::vector<WoWGuid>::iterator mItr = tGroup->members.begin(); mItr != tGroup->members.end(); mItr++)
                                {
                                    WoWGuid member = (*mItr);
                                    if(member == heal || member == tank)
                                        continue;
                                    unselectedDPS.insert(member);
                                }

                                // More macros
#define SET_GROUP_CACHE_POOL(pool, iterator)\
                                if(m_queueGroupMap.find(*iterator) == m_queueGroupMap.end())\
                                    continue;\
                                group = m_queueGroupMap.at(*iterator);\
                                break;
                                // End macro

                                // See what we need
                                if(tank.empty())
                                {   // Find a tank in our pool
                                    QueueGroup *group = NULL;
                                    if(talentedPool.size())
                                    {
                                        for(std::set<uint32>::iterator mItr = talentedPool.begin(); mItr != talentedPool.end(); mItr++)
                                        { SET_GROUP_CACHE_POOL(talentedPool, mItr); }
                                    }
                                    else if(tankGroups.size())
                                    {
                                        for(std::set<uint32>::iterator mItr = tankGroups.begin(); mItr != tankGroups.end(); mItr++)
                                        { SET_GROUP_CACHE_POOL(tankGroups, mItr); }
                                    }
                                    else if(tankDPSGroups.size())
                                    {
                                        for(std::set<uint32>::iterator mItr = tankDPSGroups.begin(); mItr != tankDPSGroups.end(); mItr++)
                                        { SET_GROUP_CACHE_POOL(tankDPSGroups, mItr); }
                                    }

                                    if(group == NULL)
                                        continue; // we failed

                                    tank = *group->members.begin();
                                    // check to see if we pass now
                                    if(tank.empty() || heal.empty() || (unselectedDPS.size() < 3))
                                        continue;
                                    groupIds.push_back(group->queueId);
                                    // Remove us from all our pooled sets
                                    for(auto sVItr = setVector.begin(); sVItr != setVector.end(); ++sVItr)
                                        (*sVItr)->erase(group->queueId);
                                }
                                else if(heal.empty())
                                {   // Find a healer in our pool
                                    QueueGroup *group = NULL;
                                    std::set<uint32> *fromPool;
                                    if(talentedPool.size())
                                    {
                                        for(std::set<uint32>::iterator mItr = talentedPool.begin(); mItr != talentedPool.end(); mItr++)
                                        { SET_GROUP_CACHE_POOL(talentedPool, mItr); }
                                    }
                                    else if(healGroups.size())
                                    {
                                        for(std::set<uint32>::iterator mItr = healGroups.begin(); mItr != healGroups.end(); mItr++)
                                        { SET_GROUP_CACHE_POOL(healGroups, mItr); }
                                    }
                                    else if(tankHealGroups.size())
                                    {
                                        for(std::set<uint32>::iterator mItr = tankHealGroups.begin(); mItr != tankHealGroups.end(); mItr++)
                                        { SET_GROUP_CACHE_POOL(tankHealGroups, mItr); }
                                    }

                                    if(group == NULL)
                                        continue; // we failed

                                    heal = *group->members.begin();
                                    // check to see if we pass now
                                    if(tank.empty() || heal.empty() || (unselectedDPS.size() < 3))
                                        continue;
                                    groupIds.push_back(group->queueId);
                                    // Remove us from all our pooled sets
                                    for(auto sVItr = setVector.begin(); sVItr != setVector.end(); ++sVItr)
                                        (*sVItr)->erase(group->queueId);
                                }
                                else if(unselectedDPS.size() < 3)
                                {   // Find a dps in our pool
                                    QueueGroup *group = NULL;
                                    std::set<uint32> *fromPool;
                                    if(talentedPool.size())
                                    {
                                        for(std::set<uint32>::iterator mItr = talentedPool.begin(); mItr != talentedPool.end(); mItr++)
                                        { SET_GROUP_CACHE_POOL(talentedPool, mItr); }
                                    }
                                    else if(dpsGroups.size())
                                    {
                                        for(std::set<uint32>::iterator mItr = dpsGroups.begin(); mItr != dpsGroups.end(); mItr++)
                                        { SET_GROUP_CACHE_POOL(dpsGroups, mItr); }
                                    }
                                    else if(tankDPSGroups.size())
                                    {
                                        for(std::set<uint32>::iterator mItr = tankDPSGroups.begin(); mItr != tankDPSGroups.end(); mItr++)
                                        { SET_GROUP_CACHE_POOL(tankDPSGroups, mItr); }
                                    }
                                    else if(healDPSGroups.size())
                                    {
                                        for(std::set<uint32>::iterator mItr = healDPSGroups.begin(); mItr != healDPSGroups.end(); mItr++)
                                        { SET_GROUP_CACHE_POOL(healDPSGroups, mItr); }
                                    }

                                    if(group == NULL)
                                        continue; // we failed

                                    unselectedDPS.insert(*group->members.begin());
                                    // check to see if we pass now
                                    if(tank.empty() || heal.empty() || (unselectedDPS.size() < 3))
                                        continue;
                                    groupIds.push_back(group->queueId);
                                    // Remove us from all our pooled sets
                                    for(auto sVItr = setVector.begin(); sVItr != setVector.end(); ++sVItr)
                                        (*sVItr)->erase(group->queueId);
                                }
#undef SET_GROUP_CACHE_POOL     // Thank you based macro

                                // We win, see if we can properly create our group from these retards
                                if(tank.empty() || heal.empty() || (unselectedDPS.size() < 3))
                                    continue;

                                WoWGuid dps1, dps2, dps3;
                                // Grab our first DPS
                                dps1 = *unselectedDPS.begin();
                                unselectedDPS.erase(unselectedDPS.begin());
                                // Grab our second DPS
                                dps2 = *unselectedDPS.begin();
                                unselectedDPS.erase(unselectedDPS.begin());
                                // Grab our third DPS
                                dps3 = *unselectedDPS.begin();
                                unselectedDPS.erase(unselectedDPS.begin());

                                // Launch our dungeon proposition, this will handle random and clearing of players from other queues
                                _LaunchProposition(itr->first, updateTeamIndex, &groupIds, tank, heal, dps1, dps2, dps3);
                                success = true;
                                break; // End iteration when we succeed
                            }
                        }
                    }
                }
            }

            if(success == true)
                continue;

            // We don't need to process group queues against single groups if we have none
            if(stack->m_singleQueues.empty())
                continue;

            // Process our groups with single queued groups
            for(std::vector<QueueGroupHolder*>::iterator gItr = stack->m_groupQueues.begin(); gItr != stack->m_groupQueues.end();)
            {
                // If we have a null group, we're queued for cleanup
                if((*gItr)->group == NULL)
                    gItr = stack->m_groupQueues.erase(gItr);
                else
                {   // Check the single group for the player in map
                    QueueGroup *group = (*gItr)->group;
                    ++gItr;

                }
            }

            if(success == true)
                continue;
        }

        // Quick handle any single queue chances
        if(stack->m_singleQueues.size() < dungeonTeamSize)
            continue; // no use if we're below the required size of singles

        // We need to find a tank and healer, then push anyone who isn't either into DPS if they have the flag
        WoWGuid tank(0), heal(0);
        uint32 tankIL = 0, healIL = 0;
        uint32 tankRoles = 0, healRoles = 0;
        std::set<WoWGuid> unselectedDPS;
        // Start processing through our single queues
        for(std::vector<QueueGroupHolder*>::iterator gItr = stack->m_singleQueues.begin(); gItr != stack->m_singleQueues.end();)
        {
            // Stop iteration if we have a group ready to queue
            if(!(tank.empty() || heal.empty() || (unselectedDPS.size() < 3)))
                break;

            // If we have a null group, we're queued for cleanup
            if((*gItr)->group == NULL)
                gItr = stack->m_singleQueues.erase(gItr);
            else
            {   // Check the single group for the player in map
                QueueGroup *group = (*gItr)->group;
                ++gItr;

                Player *plr = objmgr.GetPlayer(*group->members.begin());
                if(plr == NULL) // That ends the group for us
                    continue;
                // Limit our player item level to the recommended item level for the dungeon
                // so that players over the cap don't get too much of an advantage
                uint32 plrIL = std::min<uint32>(plr->GetAverageItemLevel(), ((dungeonData && dungeonData->recomItemLevel) ? dungeonData->recomItemLevel : 0xFFFFFFFF));

                // Quick store our role for easier use later
                uint8 role = group->memberRoles.begin()->second;
                // First check is for tank, role choice helper is used to efficiently detect the need for a tank or healer based on existing selection
                if(role & ROLEMASK_TANK && RoleChoiceHelper(plr, role, plrIL, ROLEMASK_TANK, tank, tankIL, tankRoles, heal, healIL))
                {   // If we passed helper checks, see if our current tank can be pushed to a different queue spot
                    if(!tank.empty() && (tankRoles & ~ROLEMASK_TANK) != 0)
                    {   // If we are also queued a healer then try and fit us into healer spot
                        if(tankRoles & ROLEMASK_HEALER && RoleChoiceHelper(plr, role, plrIL, ROLEMASK_HEALER, heal, healIL, 0, 0, 0))
                        {
                            // Push our current healer into DPS role if we can, never back up to tank
                            if(!heal.empty() && (healRoles & ROLEMASK_DPS))
                                unselectedDPS.insert(heal);
                            // Set our current tank to our healer before we update our new tank's data
                            heal = tank, healIL = tankIL, healRoles = tankRoles;
                            // Make sure our healer isn't in our unselected DPS set
                            std::set<WoWGuid>::iterator hItr;
                            if((hItr = unselectedDPS.find(heal)) != unselectedDPS.end())
                                unselectedDPS.erase(hItr);
                        } else if(tankRoles & ROLEMASK_DPS) // If we aren't being pushed into healer
                            unselectedDPS.insert(tank);     // Check if we can fit as an unselected DPS
                    }

                    // Set our current tank data to this player's info, store IL, and role as well
                    tank = plr->GetGUID(), tankIL = plrIL, tankRoles = role;
                    std::set<WoWGuid>::iterator tItr; // Make sure we aren't in our DPS set
                    if((tItr = unselectedDPS.find(tank)) != unselectedDPS.end())
                        unselectedDPS.erase(tItr);
                }  // Oh boy, here's the healer checks, do a role helper and update if needed
                else if(role & ROLEMASK_HEALER && RoleChoiceHelper(plr, role, plrIL, ROLEMASK_HEALER, heal, healIL, healRoles, tank, tankIL))
                {
                    if(!heal.empty() && (healRoles & ~ROLEMASK_HEALER) != 0)
                    {   // Check if we're a better match for tank, but role helper isn't as nice to wannabe tanks that heal
                        if(healRoles & ROLEMASK_TANK && RoleChoiceHelper(plr, role, plrIL, ROLEMASK_TANK, tank, tankIL, 0, 0, 0))
                        {
                            if(!tank.empty() && (tankRoles & ROLEMASK_DPS))
                                unselectedDPS.insert(tank);
                            tank = heal, tankIL = healIL, tankRoles = healRoles;
                            std::set<WoWGuid>::iterator hItr;
                            if((hItr = unselectedDPS.find(tank)) != unselectedDPS.end())
                                unselectedDPS.erase(hItr);
                        } else if(healRoles & ROLEMASK_DPS)
                            unselectedDPS.insert(heal);
                    }

                    // Set our current healer data to current player's info, store IL and role as usual
                    heal = plr->GetGUID(), healIL = plrIL, healRoles = role;
                    std::set<WoWGuid>::iterator hItr; // MAke sure we aren't in our DPS set
                    if((hItr = unselectedDPS.find(heal)) != unselectedDPS.end())
                        unselectedDPS.erase(hItr);
                } else if(role & ROLEMASK_DPS) // Queue us up for DPS deployment
                    unselectedDPS.insert(plr->GetGUID());
            }
        }

        // Make sure we weren't resized to less than required size
        if(stack->m_singleQueues.size() < dungeonTeamSize)
            continue;

        // We win, see if we can properly create our group from these retards
        if(tank.empty() || heal.empty() || (unselectedDPS.size() < 3))
            continue;

        std::vector<uint32> groupIds;
        // Grab our tank
        QueueGroup *tankGroup = m_queueGroupPlayerMap.at(tank);
        groupIds.push_back(tankGroup->queueId);
        // Grab our healer
        QueueGroup *healGroup = m_queueGroupPlayerMap.at(heal);
        groupIds.push_back(healGroup->queueId);
        WoWGuid dps1, dps2, dps3;
        // Grab our first DPS
        QueueGroup *dpsGroup = m_queueGroupPlayerMap.at(dps1 = *unselectedDPS.begin());
        unselectedDPS.erase(unselectedDPS.begin());
        groupIds.push_back(dpsGroup->queueId);
        // Grab our second DPS
        dpsGroup = m_queueGroupPlayerMap.at(dps2 = *unselectedDPS.begin());
        unselectedDPS.erase(unselectedDPS.begin());
        groupIds.push_back(dpsGroup->queueId);
        // Grab our third DPS
        dpsGroup = m_queueGroupPlayerMap.at(dps3 = *unselectedDPS.begin());
        unselectedDPS.erase(unselectedDPS.begin());
        groupIds.push_back(dpsGroup->queueId);
        // Clear out our unselected dps
        unselectedDPS.clear();
        // Launch our dungeon proposition, this will handle random and clearing of players from other queues
        _LaunchProposition(itr->first, updateTeamIndex, &groupIds, tank, heal, dps1, dps2, dps3);
    }

    while(m_completedPropositions[updateTeamIndex].size())
    {
        QueueProposition *prop = *m_completedPropositions[updateTeamIndex].begin();
        m_completedPropositions[updateTeamIndex].erase(m_completedPropositions[updateTeamIndex].begin());

        int32 mapId = -1;
        GroupFinderDungeon *dungeon = NULL;
        DungeonDataMap::iterator dDataItr;
        GroupFinderDungeonData *dungeonData = NULL;

        uint32 dungeonId = prop->propDungeonId;
        if(LFGDungeonsEntry *entry = dbcLFGDungeons.LookupEntry(dungeonId))
        {
            if((dungeonId = GetRealDungeon(entry)) > 0 && prop->propState == LFG_PROP_STATE_SUCCESS && ((dDataItr = m_dungeonData.find(dungeonId)) != m_dungeonData.end()) != NULL)
            {
                dungeonData = dDataItr->second;
                mapId = dungeonData->entry->mapId;

                dungeon = new GroupFinderDungeon();
                dungeon->origDungeonId = prop->propDungeonId;
                dungeon->dungeonId = dungeonId;
                dungeon->instanceId = 0;
                dungeon->groupId = prop->targetGroupId;
                dungeon->dataEntry = dungeonData;
            }
        }

        for(std::vector<QueueGroup*>::iterator itr = prop->queueGroups.begin(); itr != prop->queueGroups.end(); itr++)
        {
            QueueGroup *group = *itr;
            if(group == NULL)
                continue;
            group->queueId = 0;
            group->dungeonIds.clear();
            group->queueStep = LFG_STEP_GROUP_FOUND;
            group->queueState = LFG_STATE_NONE;
            group->timeStamp = UNIXTIME;

            std::vector<Player*> toRemove;
            for(Loki::AssocVector<WoWGuid, uint8>::iterator itr = group->memberRoles.begin(); itr != group->memberRoles.end(); itr++)
            {
                if(dungeon != NULL)
                    dungeon->memberRoles.insert(std::make_pair(objmgr.GetPlayerInfo(itr->first), itr->second));

                if(Player *plr = objmgr.GetPlayer(itr->first))
                {
                    SendQueueCommandResult(plr, LFG_STEP_NONE, group->queueId, LFG_STATE_NONE, true, &group->dungeonIds, 0, "");
                    SendQueueCommandResult(plr, dungeon ? LFG_STEP_UPDATE_STATE : LFG_STEP_NONE, group->queueId, dungeon ? LFG_STATE_ACTIVE : LFG_STATE_NONE, false, &group->dungeonIds, 0, "");
                    SendProposalUpdate(NULL, plr);
                    toRemove.push_back(plr);
                }
            }
            for(std::vector<Player*>::iterator itr = toRemove.begin(); itr != toRemove.end(); itr++)
                RemovePlayer(*itr, dungeon != NULL);
            _CleanupQueueGroup(group, true);
        }
        prop->queueGroups.clear();
        delete prop;

        if(dungeon == NULL)
            continue;

        Group *grp = Group::Init(prop->targetGroupId, dungeonId, GROUP_TYPE_LFD|((dungeonData->entry->LFGType == 5) ? GROUP_TYPE_HEROIC : 0));
        grp->FillLFDMembers(&dungeon->memberRoles);
        grp->Update();

        sInstanceMgr.LaunchGroupFinderDungeon(mapId, dungeon, grp);
    }

    _queueGroupLock.Release();
    if(++updateTeamIndex == 2)
        updateTeamIndex = 0;
    m_updateTimer = 0;
}

void GroupFinderMgr::RemovePlayer(Player *plr, bool silent)
{
    _queueGroupLock.Acquire();
    // Check if we're linked to a group
    QueueGroupGuidStorage::iterator itr;
    if((itr = m_queueGroupPlayerMap.find(plr->GetGUID())) == m_queueGroupPlayerMap.end())
    {
        _queueGroupLock.Release();
        return;
    }

    QueueGroup *group = itr->second;
    std::vector<WoWGuid>::iterator vItr;
    if(group->queueState <= LFG_STATE_INQUEUE)
        _CleanupQueueGroup(group, silent);
    else if((vItr = std::find(group->members.begin(), group->members.end(), plr->GetGUID())) != group->members.end())
    {
        group->members.erase(vItr);
        group->memberRoles.erase(plr->GetGUID());
        m_queueGroupPlayerMap.erase(plr->GetGUID());
        if(silent == false) SendQueueCommandResult(plr, LFG_STEP_REMOVED_FROM_QUEUE, group->queueId, group->queueState, group->groupType >= 2, NULL, UNIXTIME, "");
    }
    _queueGroupLock.Release();
}

void GroupFinderMgr::TeleportPlayer(Player *plr, bool toDungeon)
{

}

bool GroupFinderMgr::GetPlayerQueueStatus(Player *plr, uint8 &status, uint32 &queueId, uint32 &step, std::vector<uint32> **QueueIds, time_t &joinTime)
{
    _queueGroupLock.Acquire();
    QueueGroupGuidStorage::iterator itr;
    if((itr = m_queueGroupPlayerMap.find(plr->GetGUID())) == m_queueGroupPlayerMap.end())
    {
        _queueGroupLock.Release();
        return false;
    }

    status = itr->second->queueState;
    queueId = itr->second->queueId;
    step = itr->second->queueStep;
    *QueueIds = &itr->second->dungeonIds;
    joinTime = itr->second->timeStamp;
    _queueGroupLock.Release();
    return true;
}

void GroupFinderMgr::UpdateRoles(Player *plr, uint8 roleMask)
{
    _queueGroupLock.Acquire();
    QueueGroupGuidStorage::iterator itr;
    if((itr = m_queueGroupPlayerMap.find(plr->GetGUID())) == m_queueGroupPlayerMap.end())
    {
        _queueGroupLock.Release();
        return;
    }

    Loki::AssocVector<WoWGuid, uint8>::iterator rItr;
    if((rItr = itr->second->memberRoles.find(plr->GetGUID())) != itr->second->memberRoles.end())
        rItr->second = roleMask;
    else itr->second->memberRoles.insert(std::make_pair(plr->GetGUID(), roleMask));

    if(itr->second->memberRoles.size() == itr->second->members.size())
    {

    }
    _queueGroupLock.Release();
}

void GroupFinderMgr::HandleDungeonJoin(Player *plr, uint32 roleMask, std::vector<uint32> *dungeonSet, std::string comment)
{
    uint8 error = LFG_ERROR_NONE;
    QueueGroup *queueGroup = NULL;

    Group *grp = plr->GetGroup();

    if(grp && grp->GetLeader() != plr->getPlayerInfo())
        error = LFG_ERROR_ROLECHECK_FAILED;
    else if(m_queueGroupPlayerMap.find(plr->GetGUID()) != m_queueGroupPlayerMap.end())
        error = LFG_ERROR_INTERNAL;
    else
    {
        uint8 teamId = plr->GetTeam();
        queueGroup = new QueueGroup();
        queueGroup->groupType = grp ? 2 : 0;
        queueGroup->groupTeam = teamId;
        queueGroup->queueState = LFG_STATE_NONE;
        queueGroup->queueId = _GenerateQueueId();
        queueGroup->queueStep = LFG_STEP_NONE;
        queueGroup->timeStamp = UNIXTIME;
        queueGroup->comment.append(comment.c_str());
        // Push our selected dungeons into our queuegroup's list
        for(std::vector<uint32>::iterator itr = dungeonSet->begin(); itr != dungeonSet->end(); itr++)
        {
            bool needCheck = true;
            if(m_dungeonQueues[teamId].find(*itr) == m_dungeonQueues[teamId].end())
            {
                needCheck = false;
                QueueGroupStack *stack = new QueueGroupStack();
                stack->roleMask = 0;
                m_dungeonQueues[teamId].insert(std::make_pair(*itr, stack));
            }

            // Add our new dungeon Id
            queueGroup->dungeonIds.push_back(*itr);
            // Create our group holder
            QueueGroupHolder *holder = new QueueGroupHolder();
            holder->group = queueGroup;
            // Store the group holder locally
            queueGroup->groupHolders.push_back(holder);
            // Push our group holder into the appropriate dungeon queue map
            if(queueGroup->groupType >= 2)
                m_dungeonQueues[teamId][*itr]->m_groupQueues.push_back(holder);
            else
            {
                m_dungeonQueues[teamId][*itr]->m_singleQueues.push_back(holder);
                // Also add our rolemask into the queue's rolemask for easy check
                m_dungeonQueues[teamId][*itr]->roleMask |= roleMask;
            }
            // Update our needCheck state
            m_dungeonQueues[teamId][*itr]->needCheck = needCheck;
        }

        if(grp)
        {   // We have a group so process through all the players
            grp->Lock();
            for(uint8 i = 0; i < 8; i++)
            {
                if(SubGroup *sub = grp->GetSubGroup(i))
                {
                    for(GroupMembersSet::iterator itr = sub->GetGroupMembersBegin(); itr != sub->GetGroupMembersEnd(); itr++)
                    {   // Queue our target player into the group and stash it by his guid
                        queueGroup->members.push_back((*itr)->charGuid);
                        m_queueGroupPlayerMap[(*itr)->charGuid] = queueGroup;
                    }
                }
            }
            grp->Unlock();
            // Set our group state to rolecheck
            queueGroup->queueState = LFG_STATE_ROLECHECK;
        }
        else
        {   // Queue our player into the group and stash it by his guid
            queueGroup->members.push_back(plr->GetGUID());
            queueGroup->memberRoles.insert(std::make_pair(plr->GetGUID(), uint8(roleMask&0xFF)));
            m_queueGroupPlayerMap[plr->GetGUID()] = queueGroup;
            // Set our group state to in Queue
            queueGroup->queueState = LFG_STATE_INQUEUE;
            // Set us to join Queue step
            queueGroup->queueStep = LFG_STEP_JOIN_QUEUE;
        }

        // Stash our queue group by it's ID
        m_queueGroupMap[queueGroup->queueId] = queueGroup;
    }

    SendLFGJoinResult(plr, error, queueGroup);
    if(error == LFG_ERROR_NONE && queueGroup)
    {
        for(auto itr = queueGroup->members.begin(); itr != queueGroup->members.end(); ++itr)
        {
            if(Player *member = objmgr.GetPlayer(*itr))
                SendQueueCommandResult(member, queueGroup->queueStep, queueGroup->queueId, queueGroup->queueState, grp != NULL, &queueGroup->dungeonIds, queueGroup->timeStamp, "");
        }
    }
}

void GroupFinderMgr::HandleDungeonLeave(Player *plr, WoWGuid guid, uint32 queueId)
{
    _queueGroupLock.Acquire();
    QueueGroupGuidStorage::iterator itr;
    if((itr = m_queueGroupPlayerMap.find(plr->GetGUID())) == m_queueGroupPlayerMap.end())
    {
        _queueGroupLock.Release();
        return;
    }

    if(itr->second->groupType < 2)
    {
        _queueGroupLock.Release();
        RemovePlayer(plr);
        return;
    }

    switch(itr->second->queueState)
    {
    case LFG_STATE_ROLECHECK:
        break;
    case LFG_STATE_INQUEUE:
        break;
    case LFG_STATE_PROPOSAL:
        break;
    default:
        break;
    }
    _queueGroupLock.Release();
    RemovePlayer(plr);
}

void GroupFinderMgr::UpdateProposal(Player *plr, uint32 proposalId, bool result, WoWGuid guid, WoWGuid guid2)
{
    _queueGroupLock.Acquire();
    QueueGroupGuidStorage::iterator itr;
    if((itr = m_queueGroupPlayerMap.find(plr->GetGUID())) == m_queueGroupPlayerMap.end())
    {
        _queueGroupLock.Release();
        return;
    }

    DungeonPropositionMap::iterator propItr;
    if((propItr = m_dungeonPropositionsByPropId.find(proposalId)) == m_dungeonPropositionsByPropId.end())
    {
        _queueGroupLock.Release();
        return;
    }

    QueueProposition *prop = propItr->second;
    if(std::find(prop->queueGroups.begin(), prop->queueGroups.end(), itr->second) == prop->queueGroups.end())
    {
        plr->Kick(5, false);
        _queueGroupLock.Release();
        return;
    }

    if(result)
        prop->acceptedMembers.insert(plr->GetGUID());
    else
    {
        prop->rejectedMembers.insert(plr->GetGUID());
        prop->members.erase(plr->GetGUID());
        RemovePlayer(plr);
    }

    if((prop->acceptedMembers.size() + prop->rejectedMembers.size()) == prop->memberCount)
    {
        prop->propState = prop->rejectedMembers.empty() ? LFG_PROP_STATE_SUCCESS : LFG_PROP_STATE_FAILED;
        // Queue us for finished prop handling
        m_completedPropositions[prop->propTeam].push_back(prop);
        // Remove our timeout, not necessary anymore
        m_propositionTimeouts[prop->propTeam].erase(prop->propId);
        // Remove us from our proposition map
        m_dungeonPropositionsByPropId.erase(prop->propId);
        // Remove us from assigned queue groups
        for(std::vector<QueueGroup*>::iterator itr = prop->queueGroups.begin(); itr != prop->queueGroups.end(); itr++)
            m_currentQueueGroupProposals.erase((*itr)->queueId);
    }

    SendProposalUpdate(prop, NULL);
    if(prop->propState == LFG_PROP_STATE_SUCCESS)
    {
        for(std::vector<QueueGroup*>::iterator itr = prop->queueGroups.begin(); itr != prop->queueGroups.end(); itr++)
        {
            QueueGroup *group = *itr;
            if(group == NULL)
                continue;

            group->queueStep = LFG_STEP_GROUP_FOUND;
            group->queueState = LFG_STATE_PROPOSAL;
            group->groupType = 0;

            group->timeStamp = UNIXTIME;
            for(std::vector<WoWGuid>::iterator itr = group->members.begin(); itr != group->members.end(); itr++)
            {
                if(Player *plr = objmgr.GetPlayer(*itr))
                {
                    SendQueueCommandResult(plr, group->queueStep, group->queueId, group->queueState, group->groupType >= 2, &group->dungeonIds, group->timeStamp, "");
                }
            }

            group->queueState = LFG_STATE_ACTIVE;
        }
    }

    _queueGroupLock.Release();
}

void GroupFinderMgr::UpdateBootVote(Player *plr, bool vote)
{
    _queueGroupLock.Acquire();
    QueueGroupGuidStorage::iterator itr;
    if((itr = m_queueGroupPlayerMap.find(plr->GetGUID())) == m_queueGroupPlayerMap.end())
    {
        _queueGroupLock.Release();
        return;
    }

    _queueGroupLock.Release();
}

void GroupFinderMgr::UpdateComment(Player *plr, std::string comment)
{
    _queueGroupLock.Acquire();
    QueueGroupGuidStorage::iterator itr;
    if((itr = m_queueGroupPlayerMap.find(plr->GetGUID())) == m_queueGroupPlayerMap.end())
    {
        _queueGroupLock.Release();
        return;
    }

    _queueGroupLock.Release();
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
        if(LFGDungeonsEntry *entry = dbcLFGDungeons.LookupEntry(*itr))
        {
            if(entry->reqExpansion > expansion)
                continue;
            if(plr->getLevel() < entry->minLevel || plr->getLevel() > entry->maxLevel)
                continue;
            _BuildRandomDungeonData(plr, data, entry);
            ++count;
        }
    }
    data->put<uint8>(pos, count);
}

void GroupFinderMgr::BuildPlayerLockInfo(Player *plr, ByteBuffer *data, bool writeCount, bool joinResult)
{
    LFGDungeonMultiMap::iterator lower, upper;
    if((lower = m_lfgDungeonsByLevel.lower_bound(plr->getLevel())) != (upper = m_lfgDungeonsByLevel.upper_bound(plr->getLevel())))
    {
        size_t pos = data->wpos();
        if(writeCount) *data << uint32(0);
        uint32 level = plr->getLevel();
        uint32 count = 0, avgItemLvl = plr->GetAverageItemLevel();
        uint8 expansion = plr->GetSession()->GetHighestExpansion();
        for(LFGDungeonMultiMap::iterator itr = lower; itr != upper; itr++)
        {
            DungeonDataMap::iterator dDataItr;
            LFGDungeonsEntry *dungeon = itr->second;
            GroupFinderDungeonData *dungeonData = NULL;

            uint32 lockStatus = 0;
            if (dungeon->reqExpansion > expansion)
                lockStatus = 1;
            else if (dungeon->mapDifficulty > 0 && false)//sInstanceMgr.IsPlayerLockedToHeroic(plr, dungeon->mapId)
                lockStatus = 6;
            else if (dungeon->minLevel > level)
                lockStatus = 2;
            else if (dungeon->maxLevel < level)
                lockStatus = 3;
            else if ((dungeon->LFGFlags&0x04) && m_currentSeasonDungeons.find(dungeon->Id) == m_currentSeasonDungeons.end())
                lockStatus = 1031;
            else if((dDataItr = m_dungeonData.find(itr->second->Id)) == m_dungeonData.end())
                lockStatus = 1031;
            else if ((dungeonData = dDataItr->second)->reqItemLevel > avgItemLvl)
                lockStatus = 4;
            else if(dungeon->LFGType == DBC_LFG_TYPE_RANDOM && GetRealDungeon(dungeon) == 0)
                lockStatus = 1031;

            if(lockStatus)
            {
                if(joinResult)
                {
                    *data << uint32(lockStatus); // Lock status
                    *data << uint32(avgItemLvl); // Player current average item level
                    *data << uint32(dungeonData ? dungeonData->reqItemLevel : 0); // Item Level req
                    *data << uint32((dungeon->LFGType<<24)|(dungeon->Id&0x00FFFFFF)); // Dungeon entry
                }
                else
                {
                    *data << uint32((dungeon->LFGType<<24)|(dungeon->Id&0x00FFFFFF)); // Dungeon entry
                    *data << uint32(lockStatus); // Lock status
                    *data << uint32(dungeonData ? dungeonData->reqItemLevel : 0); // Item Level req
                    *data << uint32(avgItemLvl); // Player current average item level
                }
                ++count;
            }
        }
        if(writeCount) data->put<uint32>(pos, count);
    } else if(writeCount) *data << uint32(0);
}

void GroupFinderMgr::SendLFGJoinResult(Player *plr, uint8 result, QueueGroup *group)
{
    WoWGuid guid = plr->GetGUID();

    WorldPacket data(SMSG_LFG_JOIN_RESULT, 2000);
    data << uint32(3);
    data << uint8(result);
    data << uint32(group ? group->queueId : 0);
    data << uint8(group ? group->queueState : 0);
    data << uint32(group ? group->timeStamp : 0);
    data.WriteGuidBitString(4, guid, ByteBuffer::Filler, 2, 7, 3, 0);
    if(group == NULL)
    {
        data.WriteBits<uint32>(0, 24);
        data.WriteGuidBitString(4, guid, ByteBuffer::Filler, 4, 5, 1, 6);
    }
    else
    {
        uint8 index = 0;
        std::vector<ByteBuffer> lockVector(group->members.size());
        data.WriteBits(group->members.size(), 24);
        for(std::vector<WoWGuid>::iterator itr = group->members.begin(); itr != group->members.end(); itr++)
        {
            WoWGuid memberGuid = (*itr);
            data.WriteGuidBitString(8, memberGuid, ByteBuffer::Filler, 7, 5, 3, 6, 0, 2, 4, 1);
            if(Player *plr = objmgr.GetPlayer(memberGuid))
            {
                ByteBuffer &buff = lockVector[index++];
                BuildPlayerLockInfo(plr, &buff, false, true);
                data.WriteBits<uint32>((buff.size()/4), 22);
            } else data.WriteBits(0, 22);
        }

        data.WriteGuidBitString(4, guid, ByteBuffer::Filler, 4, 5, 1, 6);

        index = 0;
        for(std::vector<WoWGuid>::iterator itr = group->members.begin(); itr != group->members.end(); itr++)
        {
            WoWGuid memberGuid = (*itr);
            if(Player *plr = objmgr.GetPlayer(memberGuid))
            {
                ByteBuffer &buff = lockVector[index++];
                data.append(buff.contents(), buff.size());
            }

            data.WriteSeqByteString(8, memberGuid, ByteBuffer::Filler, 2, 5, 1, 0, 4, 3, 6, 7);
        }
    }

    data.WriteSeqByteString(8, guid, ByteBuffer::Filler, 1, 4, 3, 5, 0, 7, 2, 6);
    plr->PushPacket(&data);
}

void GroupFinderMgr::SendQueueCommandResult(Player *plr, uint8 type, uint32 queueId, uint32 queueStatus, bool groupUnk, std::vector<uint32> *dungeonSet, time_t unkTime, std::string unkComment)
{
    bool joining = false, queued = false;
    switch(type)
    {
    case LFG_STEP_JOIN_QUEUE:
    case LFG_STEP_ADDED_TO_QUEUE:
        queued = true;
    case LFG_STEP_PROPOSAL_BEGIN:
    case LFG_STEP_GROUP_JOIN_QUEUE:
        joining = true;
        break;
    case LFG_STEP_UPDATE_STATE:
        joining = queueStatus >= LFG_STATE_INQUEUE;
        queued = queueStatus == LFG_STATE_INQUEUE;
        break;
    }

    WoWGuid guid = plr->GetGUID();

    WorldPacket data(SMSG_LFG_UPDATE_STATUS, 200);
    data.WriteBit(guid[1]);
    data.WriteBit(groupUnk);
    data.WriteBits<uint32>(dungeonSet ? dungeonSet->size() : 0, 24);
    data.WriteBit(guid[6]);
    data.WriteBit(dungeonSet && !dungeonSet->empty());
    data.WriteBits(unkComment.length(), 9);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[2]);
    data.WriteBit(joining);
    data.WriteBit(guid[0]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[5]);
    data.WriteBit(queued);
    data.FlushBits();
    data << uint8(type);
    data.WriteString(unkComment);
    data << uint32(queueId);
    data << uint32(unkTime);
    data.WriteByteSeq(guid[6]);
    for(uint8 i = 0; i < 3; i++)
        data << uint8(0);
    data.WriteSeqByteString(6, guid, ByteBuffer::Filler, 1, 2, 4, 3, 5, 0);
    data << uint32(42);
    data.WriteByteSeq(guid[7]);
    if(dungeonSet != NULL && !dungeonSet->empty())
        for(auto itr = dungeonSet->begin(); itr != dungeonSet->end(); itr++)
            data << uint32(*itr);
    plr->PushPacket(&data);
}

void GroupFinderMgr::SendProposalUpdate(QueueProposition *proposition, Player *plr)
{
    std::vector<Player*> toSend;
    if(plr == NULL && proposition == NULL)
        return;
    else if(plr == NULL)
    {
        for(std::set<WoWGuid>::iterator itr = proposition->members.begin(); itr != proposition->members.end(); itr++)
            toSend.push_back(objmgr.GetPlayer(*itr));
    } else toSend.push_back(plr);

    while(!toSend.empty())
    {
        Player *player = *toSend.begin();
        toSend.erase(toSend.begin());
        if(player == NULL)
            continue;

        QueueGroup *plrGroup = NULL;
        WoWGuid guid = player->GetGUID();
        WoWGuid groupId = (player->GetGroup() ? MAKE_NEW_GUID(player->GetGroup()->GetID(), 0, HIGHGUID_TYPE_GROUP) : 0);
        if(m_queueGroupPlayerMap.find(guid) != m_queueGroupPlayerMap.end())
            plrGroup = m_queueGroupPlayerMap.at(guid);

        WorldPacket data(SMSG_LFG_PROPOSAL_UPDATE, 350);
        data << uint32(plrGroup ? plrGroup->timeStamp : UNIXTIME);
        data << uint32(proposition ? proposition->encounterMask : 0);
        data << uint32(plrGroup ? plrGroup->queueId : 0);
        data << uint32(3);
        data << uint32(proposition ? proposition->propDungeonId : 0);
        data << uint32(proposition ? proposition->propId : 0);
        data << uint8(proposition ? proposition->propState : 0);
        data.WriteBit(groupId[4]);
        data.WriteGuidBitString(3, guid, ByteBuffer::Filler, 3, 7, 0);
        data.WriteBit(groupId[1]);
        data.WriteBit(false);
        data.WriteGuidBitString(2, guid, ByteBuffer::Filler, 4, 5);
        data.WriteBit(groupId[3]);
        data.WriteBits<uint32>(proposition ? proposition->memberRoles.size() : 0, 23);
        data.WriteBit(groupId[7]);
        if(proposition)
        {
            for(Loki::AssocVector<WoWGuid, uint32>::iterator itr = proposition->memberRoles.begin(); itr != proposition->memberRoles.end(); itr++)
            {
                data.WriteBit(player->GetGroup() && player->GetGroupGuid().getLow() == proposition->targetGroupId);
                data.WriteBit(plrGroup && plrGroup->memberRoles.find(itr->first) != plrGroup->memberRoles.end());
                if(proposition->acceptedMembers.find(itr->first) != proposition->acceptedMembers.end())
                    data.WriteBits(0xFF, 2);
                else if(proposition->rejectedMembers.find(itr->first) != proposition->rejectedMembers.end())
                    data.WriteBits(0x02, 2);
                else data.WriteBits(0x00, 2);
                data.WriteBit(player->GetGUID() == itr->first);
            }
        }
        data.WriteBit(groupId[5]);
        data.WriteBit(guid[6]);
        data.WriteBit(groupId[2]);
        data.WriteBit(groupId[6]);
        data.WriteBit(guid[2]);
        data.WriteBit(guid[1]);
        data.WriteBit(groupId[0]);

        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(groupId[3]);
        data.WriteByteSeq(groupId[6]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(groupId[5]);
        data.WriteByteSeq(guid[1]);
        if(proposition)
        {
            for(Loki::AssocVector<WoWGuid, uint32>::iterator itr = proposition->memberRoles.begin(); itr != proposition->memberRoles.end(); itr++)
                data << uint32(itr->second);
        }
        data.WriteByteSeq(groupId[7]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(groupId[0]);
        data.WriteByteSeq(groupId[1]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(groupId[2]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(groupId[4]);

        player->PushPacket(&data);
    }
}

void GroupFinderMgr::_BuildRandomDungeonData(Player *plr, WorldPacket *data, LFGDungeonsEntry *entry)
{
    bool isDone = false;
    if(GroupFinderReward *reward = GetReward(entry->Id, plr->getLevel()))
        if(Quest *qst = sQuestMgr.GetQuestPointer(reward->questId[0]))
            isDone = plr->HasCompletedQuest(qst);

    *data << uint32((entry->LFGType<<24)|(entry->Id&0x00FFFFFF)); // Dungeon entry
    *data << uint8(isDone); // isDone
    *data << uint32(0); // Currency Cap
    *data << uint32(0); // Currency Reward
    *data << uint32(0); // Currency ID
    for(uint32 i = 0; i < 4; i++)
        *data << uint32(0) << uint32(0); // Reward count / Reward Limit
    *data << uint32(0); // Prize item
    *data << uint32(0); // Completion mask
    *data << uint8(1); // Is bonus available
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

uint32 GroupFinderMgr::GetRealDungeon(LFGDungeonsEntry *entry)
{
    if(entry->LFGType != 6)
        return entry->Id;

    uint32 ret = 0;
    LFGDungeonMultiMap::iterator lower, upper;
    if(entry->LFGFaction != -1)
    {
        lower = m_lfgDungeonsByLFGFaction.lower_bound(entry->LFGFaction);
        upper = m_lfgDungeonsByLFGFaction.upper_bound(entry->LFGFaction);
    }
    else if((lower = m_lfgDungeonsByRandomCategoryId.lower_bound(entry->Id)) == (upper = m_lfgDungeonsByRandomCategoryId.upper_bound(entry->Id)))
        return ret;

    if(lower != upper)
    {
        std::vector<uint32> set;
        for(LFGDungeonMultiMap::iterator itr = lower; itr != upper; itr++)
        {
            LFGDungeonsEntry *current = itr->second;
            if(entry == current)
                continue;
            if(entry->mapDifficulty != current->mapDifficulty)
                continue;
            if(m_dungeonData.find(current->Id) == m_dungeonData.end())
                continue;
            MapEntry *map = dbcMap.LookupEntry(current->mapId);
            if(map == NULL || !map->IsDungeon())
                continue; // Skip anything not a dungeon

            set.push_back(ret = current->Id);
        }

        if(set.size() > 1)
            ret = set[RandomUInt(set.size()-1)];
    }
    return ret;
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

void GroupFinderMgr::_CleanupQueueGroup(QueueGroup *group, bool silent)
{
    Player *plr;
    while(!group->members.empty())
    {
        WoWGuid guid = *group->members.begin();
        group->members.erase(group->members.begin());
        if(silent == false && (plr = objmgr.GetPlayer(guid)))
            SendQueueCommandResult(plr, LFG_STEP_REMOVED_FROM_QUEUE, group->queueId, LFG_STATE_NONE, group->groupType >= 2, NULL, UNIXTIME, "");
        m_queueGroupPlayerMap.erase(guid);
    }
    m_queueGroupMap.erase(group->queueId);
    for(std::vector<QueueGroupHolder*>::iterator itr = group->groupHolders.begin(); itr != group->groupHolders.end(); itr++)
        (*itr)->group = NULL;
    m_queueGroupDeletionQueue.push_back(group);
}

void GroupFinderMgr::_LaunchProposition(uint32 dungeonId, uint8 propTeam, std::vector<uint32> *groupIds, WoWGuid tank, WoWGuid heal, WoWGuid dps1, WoWGuid dps2, WoWGuid dps3)
{
    QueueProposition *proposition = NULL;
    if(LFGDungeonsEntry *entry = dbcLFGDungeons.LookupEntry(dungeonId))
    {
        // Launch our dungeon entry
        proposition = new QueueProposition();
        proposition->propDungeonId = dungeonId;
        proposition->propId = _GeneratePropositionId();
        proposition->propState = LFG_PROP_STATE_START;
        proposition->propTeam = propTeam;
        proposition->encounterMask = 0;
        proposition->targetGroupId = objmgr.GenerateGroupId();
        for(std::vector<uint32>::iterator itr = groupIds->begin(); itr != groupIds->end(); itr++)
        {
            if(m_queueGroupMap.find(*itr) == m_queueGroupMap.end())
            {
                delete proposition;
                proposition = NULL;
                break;
            }

            QueueGroup *group = m_queueGroupMap.at(*itr);
            proposition->memberCount += group->members.size();
            for(std::vector<WoWGuid>::iterator itr = group->members.begin(); itr != group->members.end(); itr++)
            {
                if((*itr) == tank)
                    proposition->memberRoles.insert(std::make_pair(*itr, ROLEMASK_TANK));
                else if((*itr) == heal)
                    proposition->memberRoles.insert(std::make_pair(*itr, ROLEMASK_HEALER));
                else if((*itr) == dps1)
                    proposition->memberRoles.insert(std::make_pair(*itr, ROLEMASK_DPS));
                else if((*itr) == dps2)
                    proposition->memberRoles.insert(std::make_pair(*itr, ROLEMASK_DPS));
                else if((*itr) == dps3)
                    proposition->memberRoles.insert(std::make_pair(*itr, ROLEMASK_DPS));

                proposition->members.insert(*itr);
            }

            // Push our group into our proposition vector
            proposition->queueGroups.push_back(group);
        }

        if(proposition)
        {
            for(std::vector<uint32>::iterator itr = groupIds->begin(); itr != groupIds->end(); itr++)
                m_currentQueueGroupProposals.insert(std::make_pair(*itr, proposition->propId));
            m_dungeonPropositionsByPropId.insert(std::make_pair(proposition->propId, proposition));

            SendProposalUpdate(proposition, NULL);
        }
    }

    for(std::vector<uint32>::iterator itr = groupIds->begin(); itr != groupIds->end(); itr++)
    {
        QueueGroup *group = m_queueGroupMap.at(*itr);
        if(group == NULL)
            continue;

        group->dungeonIds.clear();
        group->dungeonIds.push_back(dungeonId);
        if(proposition)
        {   // We did it, set our queue state
            group->queueStep = LFG_STEP_PROPOSAL_BEGIN;
            group->queueState = LFG_STATE_PROPOSAL;
        }
        else
        {   // We failed to initialize, just remove us all from queue
            group->queueStep = LFG_STEP_REMOVED_FROM_QUEUE;
            group->queueState = LFG_STATE_NONE;
        }

        group->timeStamp = UNIXTIME;
        for(std::vector<WoWGuid>::iterator itr = group->members.begin(); itr != group->members.end(); itr++)
        {
            if(Player *plr = objmgr.GetPlayer(*itr))
            {
                SendQueueCommandResult(plr, group->queueStep, group->queueId, group->queueState, group->groupType >= 2, &group->dungeonIds, group->timeStamp, "");
            }
        }

        // We're in random with players, remove our group holders
        for(std::vector<QueueGroupHolder*>::iterator itr = group->groupHolders.begin(); itr != group->groupHolders.end(); itr++)
            (*itr)->group = NULL;
        group->groupHolders.clear();
    }
}
