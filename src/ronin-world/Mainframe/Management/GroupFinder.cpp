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

GroupFinderMgr::GroupFinderMgr() : m_updateTimer(0), m_maxReqExpansion(0), updateTeamIndex(0)
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
        delete result;
    }

    sLog.Notice("LfgMgr", "%u LFD rewards loaded.", count);
}

bool RoleChoiceHelper(Player *plr, uint8 plrRoleMask, uint32 plrItemLevel, uint8 currentRole, WoWGuid curRole, uint32 curRoleIL, uint8 curRoleMask, WoWGuid otherRole, uint32 otherRoleIL)
{
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
    // Time to process through our map of dungeons with queued player groups
    for(DungeonGroupStackMap::iterator itr = m_dungeonQueues[updateTeamIndex].begin(); itr != m_dungeonQueues[updateTeamIndex].end(); itr++)
    {
        LFGDungeonsEntry *entry = dbcLFGDungeons.LookupEntry(itr->first);
        QueueGroupStack *stack = itr->second;

        // Check if we only have singles groups, then we can cut down on processing
        if(stack->m_groupQueues.empty())
        { // Quick handle any single queue chances
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
                    uint32 plrIL = std::min<uint32>(plr->GetAverageItemLevel(), 0xFFFFFFFF);//entry->recomItemLevel);

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
            if(!(tank.empty() || heal.empty() || (unselectedDPS.size() < 3)))
            {
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
                // Launch our dungeon, this will handle random and clearing of players from other queues
                _LaunchDungeon(itr->first, &groupIds, tank, heal, dps1, dps2, dps3);
            }

            continue;
        }

        /*std::vector<QueueGroup*> validQueues, validGroups;
        for(std::vector<QueueGroupHolder*>::iterator itr = stack->m_singleQueues.begin(); itr != stack->m_singleQueues.end();)
        {
            if((*itr)->group == NULL)
                itr = stack->m_singleQueues.erase(itr);
            else validQueues.push_back((*itr++)->group);
        }

        for(std::vector<QueueGroupHolder*>::iterator itr = stack->m_groupQueues.begin(); itr != stack->m_groupQueues.end();)
        {
            if((*itr)->group == NULL)
                itr = stack->m_groupQueues.erase(itr);
            else validGroups.push_back((*itr++)->group);
        }

        for(std::vector<QueueGroup*>::iterator itr = validGroups.begin(); itr != validGroups.end(); itr++)
        {
            QueueGroup *group = *itr;
            bool isValidGroup = true;
            uint32 highestGroupTankIL = 0;
            WoWGuid tankOnlyMember, healOnlyMember, tankHealOnlyMember, nonFullTank;
            for(Loki::AssocVector<WoWGuid, uint8>::iterator mItr = group->memberRoles.begin(); mItr != group->memberRoles.end(); mItr++)
            {
                Player *plr = objmgr.GetPlayer((*mItr).first);
                if(plr == NULL) // That ends the group for us
                {
                    isValidGroup = false;
                    break;
                }

                if(((*mItr).second & ROLEMASK_ROLE_TYPE) == (ROLEMASK_TANK|ROLEMASK_HEALER))
                    tankHealOnlyMember = (*mItr).first; // Member must be either tank or healer
                else if(((*mItr).second & ROLEMASK_ROLE_TYPE) == ROLEMASK_TANK)
                    tankOnlyMember = (*mItr).first; // Member must be tank
                else if(((*mItr).second & ROLEMASK_ROLE_TYPE) == ROLEMASK_HEALER)
                    healOnlyMember = (*mItr).first; // Member must be healer

                if((*mItr).second & ROLEMASK_TANK)
                {
                    if((*mItr).first != tankOnlyMember)
                    {
                        nonFullTank = (*mItr).first;
                        if(plr->GetAverageItemLevel() > highestGroupTankIL)
                            highestGroupTankIL = plr->GetAverageItemLevel();
                    }
                }
            }
            if(!isValidGroup)
                continue;

            WoWGuid tank(nonFullTank.empty() ? tankOnlyMember.empty() ? tankHealOnlyMember : tankOnlyMember : nonFullTank), heal(healOnlyMember.empty() ? (tank != tankHealOnlyMember ? tankHealOnlyMember : 0) : healOnlyMember);

            std::vector<std::vector<WoWGuid>> unselectedDPS;
            uint32 tankIL = 0, healIL = 0, lowestDPSIL = 0xFFFFFFFF;
            for(std::vector<QueueGroup*>::iterator vItr = validQueues.begin(); vItr != validQueues.end(); vItr++)
            {
                uint32 groupDPS;
                std::vector<std::pair<WoWGuid, uint32>> availableDPS;
                for(Loki::AssocVector<WoWGuid, uint8>::iterator mItr = (*vItr)->memberRoles.begin(); mItr != (*vItr)->memberRoles.end(); mItr++)
                {
                    Player *plr = objmgr.GetPlayer((*mItr).first);
                    if(plr == NULL) // That ends the group for us
                        break;

                    if((*mItr).second & ROLEMASK_TANK && tankOnlyMember.empty() && RoleChoiceHelper(plr, (*mItr).second, ROLEMASK_TANK, tank, tankIL, (*vItr)->memberRoles[tank], heal, healIL))
                    {
                        if(!tank.empty() && ((*vItr)->memberRoles[tank] & ROLEMASK_DPS))
                            availableDPS.push_back(std::make_pair((*mItr).first, (*vItr)->queueId));

                        tank = (*mItr).first, tankIL = plr->GetAverageItemLevel();
                    }
                    else if((*mItr).second & ROLEMASK_HEALER && healOnlyMember.empty() && RoleChoiceHelper(plr, (*mItr).second, ROLEMASK_HEALER, heal, healIL, (*vItr)->memberRoles[tank], tank, tankIL))
                    {
                        if(!heal.empty() && ((*vItr)->memberRoles[heal] & ROLEMASK_DPS))
                            availableDPS.push_back(std::make_pair((*mItr).first, (*vItr)->queueId));

                        heal = (*mItr).first, healIL = plr->GetAverageItemLevel();
                    }
                    else if((*mItr).second & ROLEMASK_DPS)
                        availableDPS.push_back(std::make_pair((*mItr).first, (*vItr)->queueId));
                }
            }

            if(!(tank.empty() || heal.empty() || (unselectedDPS.size() < 3)))
            {
                // Queue successful, start dungeon
                break;
            } // Add our group to the back to test against other groups
            else validQueues.push_back(group);
        }*/
    }

    _queueGroupLock.Release();
    if(++updateTeamIndex == 2)
        updateTeamIndex = 0;
    m_updateTimer = 0;
}

void GroupFinderMgr::RemovePlayer(Player *plr)
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
    if(group->queueState == LFG_STATE_INQUEUE)
        _CleanupQueueGroup(group);
    else if((vItr = std::find(group->members.begin(), group->members.end(), plr->GetGUID())) != group->members.end())
    {
        group->members.erase(vItr);
        group->memberRoles.erase(plr->GetGUID());
        m_queueGroupPlayerMap.erase(plr->GetGUID());
        SendQueueCommandResult(plr, LFG_STEP_REMOVED_FROM_QUEUE, group->queueId, group->queueState, group->groupType >= 2, NULL, UNIXTIME, "");
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

    _SendLFGJoinResult(plr, error, queueGroup);
    if(error == LFG_ERROR_NONE && queueGroup)
        SendQueueCommandResult(plr, queueGroup->queueStep, queueGroup->queueId, queueGroup->queueState, grp != NULL, &queueGroup->dungeonIds, queueGroup->timeStamp, "");
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

void GroupFinderMgr::HandleRaidJoin(Player *plr, uint32 roleMask, std::vector<uint32> *raidSet, std::string comment)
{

}

void GroupFinderMgr::HandleRaidLeave(Player *plr, WoWGuid guid, uint32 queueId)
{

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
    data.WriteSeqByteString(6, guid, 1, 2, 4, 3, 5, 0);
    data << uint32(42);
    data.WriteByteSeq(guid[7]);
    if(dungeonSet != NULL && !dungeonSet->empty())
        for(auto itr = dungeonSet->begin(); itr != dungeonSet->end(); itr++)
            data << uint32(*itr);
    plr->PushPacket(&data);
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

void GroupFinderMgr::_CleanupQueueGroup(QueueGroup *group)
{
    while(!group->members.empty())
    {
        WoWGuid guid = *group->members.begin();
        group->members.erase(group->members.begin());
        if(Player *plr = objmgr.GetPlayer(guid))
            SendQueueCommandResult(plr, LFG_STEP_REMOVED_FROM_QUEUE, group->queueId, LFG_STATE_INQUEUE, group->groupType >= 2, NULL, UNIXTIME, "");
        m_queueGroupPlayerMap.erase(guid);
    }
    m_queueGroupMap.erase(group->queueId);
    for(std::vector<QueueGroupHolder*>::iterator itr = group->groupHolders.begin(); itr != group->groupHolders.end(); itr++)
        (*itr)->group = NULL;
    m_queueGroupDeletionQueue.push_back(group);
}

void GroupFinderMgr::_SendLFGJoinResult(Player *plr, uint8 result, QueueGroup *group)
{
    WoWGuid guid = plr->GetGUID();

    WorldPacket data(SMSG_LFG_JOIN_RESULT, 2000);
    data << uint32(3);
    data << uint8(result);
    data << uint32(group ? group->queueId : 0);
    data << uint8(group ? group->queueState : 0);
    data << uint32(group ? group->timeStamp : 0);
    data.WriteGuidBitString(4, guid, 2, 7, 3, 0);
    if(group == NULL)
    {
        data.WriteBits<uint32>(0, 24);
        data.WriteGuidBitString(4, guid, 4, 5, 1, 6);
    }
    else
    {
        data.WriteBits(group->members.size(), 24);
        for(std::vector<WoWGuid>::iterator itr = group->members.begin(); itr != group->members.end(); itr++)
        {
            WoWGuid memberGuid = (*itr);
            data.WriteGuidBitString(8, memberGuid, 7, 5, 3, 6, 0, 2, 4, 1);
            data.WriteBits<uint32>(0, 22);
        }

        data.WriteGuidBitString(4, guid, 4, 5, 1, 6);

        for(std::vector<WoWGuid>::iterator itr = group->members.begin(); itr != group->members.end(); itr++)
        {
            WoWGuid memberGuid = (*itr);
            for(uint32 i = 0; i < 0; i++)
            {
                data << uint32(0); // Lock state
                data << uint32(0); // ItemLevel
                data << uint32(0); // ReqItemLvl
                data << uint32(0); // dungeonId
            }

            data.WriteSeqByteString(8, memberGuid, 2, 5, 1, 0, 4, 3, 6, 7);
        }
    }

    data.WriteSeqByteString(8, guid, 1, 4, 3, 5, 0, 7, 2, 6);
    plr->PushPacket(&data);
}

void GroupFinderMgr::_LaunchDungeon(uint32 dungeonId, std::vector<uint32> *groupIds, WoWGuid tank, WoWGuid heal, WoWGuid dps1, WoWGuid dps2, WoWGuid dps3)
{
    bool forcedCleanup = true;//false;
    if(LFGDungeonsEntry *entry = dbcLFGDungeons.LookupEntry(dungeonId))
    {
        // Launch our dungeon entry

    } else forcedCleanup = true;

    for(std::vector<uint32>::iterator itr = groupIds->begin(); itr != groupIds->end(); itr++)
    {
        QueueGroup *group = m_queueGroupMap.at(*itr);
        if(group == NULL)
            continue;
        group->dungeonIds.clear();
        group->dungeonIds.push_back(dungeonId);
        if(forcedCleanup)
        {
            group->queueStep = LFG_STEP_REMOVED_FROM_QUEUE;
            group->queueState = LFG_STATE_INQUEUE;
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
