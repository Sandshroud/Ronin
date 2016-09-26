/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton(CBattlegroundManager);
typedef CBattleground*(*CreateBattlegroundFunc)( MapInstance* instance,uint32 iid,uint32 group, uint32 type);

const uint32 GetBGMapID(uint32 type)
{
    switch(type)
    {
    case BATTLEGROUND_ALTERAC_VALLEY:
        return 30;
    case BATTLEGROUND_WARSONG_GULCH:
        return 489;
    case BATTLEGROUND_ARATHI_BASIN:
        return 529;
    case BATTLEGROUND_EYE_OF_THE_STORM:
        return 566;
    case BATTLEGROUND_STRAND_OF_THE_ANCIENTS:
        return 607;
    case BATTLEGROUND_ISLE_OF_CONQUEST:
        return 628;
    }
    return 0;
};

const static uint32 arena_map_ids[5] = {
    559,
    562,
    572,
    617,
    618
};

const static CreateBattlegroundFunc GetBGCFunc(uint32 type)
{
    switch(type)
    {
    case BATTLEGROUND_ALTERAC_VALLEY:
        return &AlteracValley::Create;
    case BATTLEGROUND_WARSONG_GULCH:
        return &WarsongGulch::Create;
    case BATTLEGROUND_ARATHI_BASIN:
        return &ArathiBasin::Create;
    case BATTLEGROUND_EYE_OF_THE_STORM:
        return &EyeOfTheStorm::Create;
    case BATTLEGROUND_STRAND_OF_THE_ANCIENTS:
        return &StrandOfTheAncients::Create;
    case BATTLEGROUND_ISLE_OF_CONQUEST:
        return &IsleOfConquest::Create;
    }
    // Should never happen.
    return NULL;
};

const static uint32 GetBGMinPlayers(uint32 type)
{
    switch(type)
    {
    case BATTLEGROUND_ALTERAC_VALLEY:
        return sWorld.av_minplrs;
    case BATTLEGROUND_WARSONG_GULCH:
        return sWorld.wsg_minplrs;
    case BATTLEGROUND_ARATHI_BASIN:
        return sWorld.ab_minplrs;
    case BATTLEGROUND_EYE_OF_THE_STORM:
        return sWorld.eots_minplrs;
    case BATTLEGROUND_STRAND_OF_THE_ANCIENTS:
        return sWorld.sota_minplrs;
    case BATTLEGROUND_ISLE_OF_CONQUEST:
        return sWorld.ioc_minplrs;
    }
    return 0;
}

const static uint32 GetBGPvPDataCount(uint32 type)
{
    switch(type)
    {
    case BATTLEGROUND_ALTERAC_VALLEY:
        return 5;
    case BATTLEGROUND_WARSONG_GULCH:
    case BATTLEGROUND_ARATHI_BASIN:
        return 2;
    case BATTLEGROUND_EYE_OF_THE_STORM:
        return 1;
    }
    return 0;
}

#define IS_ARENA(x) ( (x) >= BATTLEGROUND_ARENA_2V2 && (x) <= BATTLEGROUND_ARENA_5V5 )

CBattlegroundManager::CBattlegroundManager()
{
    int i, j;

    m_maxBattlegroundId = 0;

    for(i = 0; i < BATTLEGROUND_NUM_TYPES; i++)
    {
        m_instances[i].clear();
        m_queuedGroups[i].clear();

        for (j = 0; j < MAX_LEVEL_GROUP; ++j)
        {
            m_queuedPlayers[i][j].clear();
        }
    }

    memset(m_averageQueueTimes, 0, sizeof(uint32)*USABLEBSGS*10);
}

void CBattlegroundManager::Init()
{

}

CBattlegroundManager::~CBattlegroundManager()
{
    // Moved from Master.cpp
    sLog.Notice("BattlegroundMgr", "~BattlegroundMgr()");
}

// Returns the average queue time for a bg type, using last 10 players queued
uint32 CBattlegroundManager::GetAverageQueueTime(uint32 BgType)
{
    uint32 avg;
    int i;

    assert(BgType < BATTLEGROUND_NUM_TYPES);

    avg = m_averageQueueTimes[BgType][0];
    for (i=1; i<10; i++)
    {
        if (m_averageQueueTimes[BgType][i] == 0) break;

        avg = (avg + m_averageQueueTimes[BgType][i])/2;
    }

    return avg;
}

void CBattlegroundManager::AddAverageQueueTime(uint32 BgType, uint32 queueTime)
{
    int i;

    assert(BgType < BATTLEGROUND_NUM_TYPES);

    // move the queue times one place in the array
    for (i=0; i<9; i++)
    {
        m_averageQueueTimes[BgType][i+1] = m_averageQueueTimes[BgType][i];
    }

    // add the new queueTime in the first slot
    m_averageQueueTimes[BgType][0] = queueTime;
}

void CBattlegroundManager::HandleBattlegroundListPacket(WorldSession * m_session, uint32 BattlegroundType, WoWGuid requestguid)
{
    if( !IsValidBG(BattlegroundType))
        return;

    uint32 Count = 0;
    bool random = (BattlegroundType == 32);
    uint32 ArenaP = m_session->GetPlayer()->randombgwinner ? m_session->GetPlayer()->GenerateRBGReward(m_session->GetPlayer()->getLevel(),15) : m_session->GetPlayer()->GenerateRBGReward(m_session->GetPlayer()->getLevel(),25);
    uint32 HonorP = m_session->GetPlayer()->randombgwinner ? m_session->GetPlayer()->GenerateRBGReward(m_session->GetPlayer()->getLevel(),15) : m_session->GetPlayer()->GenerateRBGReward(m_session->GetPlayer()->getLevel(),30);
    uint32 LoseHonorP = m_session->GetPlayer()->GenerateRBGReward(m_session->GetPlayer()->getLevel(),5);
    uint32 LevelGroup = GetLevelGrouping(m_session->GetPlayer()->getLevel());

    WorldPacket data(SMSG_BATTLEFIELD_LIST, 55);
    data << uint32(0); // 4.3.4 winC Weekend
    data << uint32(0); // 4.3.4 winC Random
    data << uint32(0); // 4.3.4 loss Weekend
    data << BattlegroundType; // BG ID
    data << uint32(0); // 4.3.4 loss Random
    data << uint32(0); // 4.3.4 win Random
    data << uint32(0); // 4.3.4 win Weekend
    data << uint8(0); // Maxlevel
    data << uint8(0); // Minlevel
    data.WriteGuidBitString(3, requestguid, 0, 1, 7);
    data.WriteBit(false); // holidayBG curBonus
    data.WriteBit(false); // randBG Curbonus

    ByteBuffer buff;
    uint32 count = 0;
    if(!IS_ARENA(BattlegroundType))
    {
        /* Append the battlegrounds */
        m_instanceLock.Acquire();
        for(std::map<uint32, CBattleground* >::iterator itr = m_instances[BattlegroundType].begin(); itr != m_instances[BattlegroundType].end(); itr++)
        {
            if( itr->second->GetLevelGroup() == LevelGroup  && !itr->second->HasEnded() )
            {
                buff << uint32(itr->first);
                ++count;
            }
        }
        m_instanceLock.Release();
    }
    data.WriteBits(count, 24);
    data.WriteGuidBitString(4, requestguid, 6, 4, 2, 3);
    data.WriteBit(false);                  // unk
    data.WriteBit(requestguid[5]);
    data.WriteBit(true);                   // signals EVENT_PVPQUEUE_ANYWHERE_SHOW if set
    data.FlushBits();

    data.WriteSeqByteString(4, requestguid, 6, 1, 7, 5);
    if (count)
        data.append(buff.contents(), buff.size());
    data.WriteSeqByteString(4, requestguid, 0, 2, 4, 3);
    m_session->SendPacket(&data);
}

void CBattlegroundManager::HandleBattlegroundJoin(WorldSession * m_session, WorldPacket & pck)
{
    uint64 guid;
    uint32 bgtype;
    uint32 instance;
    uint8 joinasgroup; //0x01 = Group and 0x00 = Player
    Player *plr = m_session->GetPlayer();
    uint32 lgroup = GetLevelGrouping(plr->getLevel());

    pck >> guid >> bgtype >> instance >> joinasgroup;
    if(bgtype == 32)
    {
        bgtype = GenerateRandomBGType();
        plr->fromrandombg = true;
    }

    if(bgtype >= BATTLEGROUND_NUM_TYPES || GetBGMapID(bgtype) == 0)
    {
        m_session->Disconnect();
        return;     // cheater!
    }

    if ( m_session->GetPlayer()->m_AuraInterface.HasActiveAura(BG_DESERTER) && !m_session->HasGMPermissions() )
    {
        m_session->SendNotification("You have been marked as a Deserter.");
        return;
    }

    if( joinasgroup && m_session->GetPlayer()->GetGroup() == NULL )
    {
        //Correct? is there any message here at blizz?
        sChatHandler.RedSystemMessage( m_session, "You are not in a Group." );
        return;
    }

    /* Check the instance id */
    if(instance)
    {
        /* We haven't picked the first instance. This means we've specified an instance to join. */
        m_instanceLock.Acquire();
        std::map<uint32, CBattleground* >::iterator itr = m_instances[bgtype].find(instance);

        if(itr == m_instances[bgtype].end())
        {
            sChatHandler.SystemMessage(m_session, "You have tried to join an invalid instance id.");
            m_instanceLock.Release();
            return;
        }

        m_instanceLock.Release();
    }

    /* Queue him! */
    m_queueLock.Acquire();
    if( !plr->HasBattlegroundQueueSlot() )
    {
        plr->BroadcastMessage("You are in too many queues already.");
        m_queueLock.Release();
        return;
    }

    if( joinasgroup && m_session->GetPlayer()->GetGroup())
    {
        m_session->GetPlayer()->GetGroup()->m_isqueued = true;
        sLog.Success("BattlegroundManager", "Group %u is now in battleground queue for instance %u", m_session->GetPlayer()->GetGroupID(), instance );
    } else sLog.Success("BattlegroundManager", "Player %u is now in battleground queue for instance %u", m_session->GetPlayer()->GetLowGUID(), instance );

    /* send the battleground status packet */
    uint32 queueSlot = plr->GetBGQueueSlot();
    if(queueSlot == 0xFFFFFFFF)
    {
        sLog.outDebug("WARNING: queueSlot is 0xFFFFFFFF in %s\n", __FUNCTION__);
        m_queueLock.Release();
        return;
    }

    if( plr->HasBGQueueSlotOfType( bgtype ) != 4)
        queueSlot = plr->HasBGQueueSlotOfType( bgtype );

    if( queueSlot >= 2 )
    {
        m_queueLock.Release();
        return;
    }

    if( plr->m_pendingBattleground[ queueSlot ] )
        plr->m_pendingBattleground[ queueSlot ]->RemovePendingPlayer(plr);

    if( plr->m_bg && plr->m_bg->GetType() == bgtype )
    {
        sLog.outDebug("Player has queued from already in a BG for itself.");
        m_queueLock.Release();
        return;
    }

    //sLog.Notice("BattlegroundManager", "Chose queue slot %u for HandleBattlegroundJoin", queueSlot);
    plr->m_bgIsQueued[queueSlot] = true;
    plr->m_bgQueueInstanceId[queueSlot] = instance;
    plr->m_bgQueueType[queueSlot] = bgtype;
    plr->m_bgQueueTime[queueSlot] = (uint32)UNIXTIME;

    /* Set battleground entry point */
    plr->m_bgEntryPointX = plr->GetPositionX();
    plr->m_bgEntryPointY = plr->GetPositionY();
    plr->m_bgEntryPointZ = plr->GetPositionZ();
    plr->m_bgEntryPointMap = plr->GetMapId();
    plr->m_bgEntryPointInstance = plr->GetInstanceID();

    m_queuedPlayers[bgtype][lgroup].push_back(plr->GetGUID());

    m_queueLock.Release();

    SendBattlegroundQueueStatus( plr, queueSlot);

    /* We will get updated next few seconds =) */
}

void ErasePlayerFromList(WoWGuid guid, std::list<WoWGuid>* l)
{
    for(std::list<WoWGuid>::iterator itr = l->begin(); itr != l->end(); itr++)
    {
        if((*itr) == guid)
        {
            l->erase(itr);
            return;
        }
    }
}

uint32 CBattlegroundManager::GetArenaGroupQInfo(Group * group, int type, uint32 *avgRating)
{
    ArenaTeam *team;
    ArenaTeamMember *atm;
    Player* plr;
    uint32 count=0;
    uint32 rating=0;

    if (group == NULL || group->GetLeader() == NULL) return 0;

    plr = group->GetLeader()->m_loggedInPlayer;
    if (plr == NULL || plr->m_playerInfo == NULL) return 0;

    team = plr->m_playerInfo->arenaTeam[type-BATTLEGROUND_ARENA_2V2];
    if (team == NULL) return 0;

    GroupMembersSet::iterator itx;
    for(itx = group->GetSubGroup(0)->GetGroupMembersBegin(); itx != group->GetSubGroup(0)->GetGroupMembersEnd(); itx++)
    {
        plr = (*itx)->m_loggedInPlayer;
        if(plr && plr->m_playerInfo)
        {
            if (team == plr->m_playerInfo->arenaTeam[type-BATTLEGROUND_ARENA_2V2])
            {
                atm = team->GetMemberByGuid(plr->GetGUID());
                if (atm)
                {
                    rating+= atm->PersonalRating;
                    count++;
                }
            }
        }
    }

    *avgRating = count > 0 ? rating/count : 0;

    return team->m_id;
}

void CBattlegroundManager::AddGroupToArena(CBattleground* bg, Group * group, int nteam)
{
    ArenaTeam *team;
    Player* plr;

    if (group == NULL || group->GetLeader() == NULL) return;

    plr = group->GetLeader()->m_loggedInPlayer;
    if (plr == NULL || plr->m_playerInfo == NULL) return;

    team = plr->m_playerInfo->arenaTeam[bg->GetType()-BATTLEGROUND_ARENA_2V2];
    if (team == NULL) return;

    GroupMembersSet::iterator itx;
    for(itx = group->GetSubGroup(0)->GetGroupMembersBegin(); itx != group->GetSubGroup(0)->GetGroupMembersEnd(); itx++)
    {
        plr = (*itx)->m_loggedInPlayer;
        if(plr && plr->m_playerInfo && team == plr->m_playerInfo->arenaTeam[bg->GetType()-BATTLEGROUND_ARENA_2V2])
        {
            if( bg->HasFreeSlots(nteam) )
            {
                bg->AddPlayer(plr, nteam);
                plr->SetTeam(nteam);
                ArenaTeamMember * atm = team->GetMember(plr->m_playerInfo);
                if(atm)
                {
                    atm->Played_ThisSeason++;
                    atm->Played_ThisWeek++;
                }
            }
        }
    }

    team->m_stat_gamesplayedseason++;
    team->m_stat_gamesplayedweek++;

    team->SaveToDB();
}

int CBattlegroundManager::CreateArenaType(int type, Group * group1, Group * group2)
{
    if(!CanCreateInstance(type, LEVEL_GROUP_RATED_ARENA))
        return -1;

    Arena* ar = castPtr<Arena>(CreateInstance(type, LEVEL_GROUP_RATED_ARENA));
    if (ar == NULL)
    {
        sLog.Error("BattlegroundMgr", "%s (%u): Couldn't create Arena Instance", __FILE__, __LINE__);
        m_queueLock.Release();
        m_instanceLock.Release();
        return -1;
    }
    ar->rated_match=true;

    AddGroupToArena(ar, group1, 0);
    AddGroupToArena(ar, group2, 1);

    return 0;
}

void CBattlegroundManager::AddPlayerToBg(CBattleground* bg, std::deque<Player*  > *playerVec, uint32 i, uint32 j)
{
    Player* plr = *playerVec->begin();
    playerVec->pop_front();
    if(bg->CanPlayerJoin(plr))
    {
        bg->AddPlayer(plr, plr->GetTeam());
        ErasePlayerFromList(plr->GetGUID(), &m_queuedPlayers[i][j]);
    }
    else
    {
        // Put again the player in the queue
        playerVec->push_back(plr);
    }
}

void CBattlegroundManager::AddPlayerToBgTeam(CBattleground* bg, std::deque<Player*  > *playerVec, uint32 i, uint32 j, int Team)
{
    if (bg->HasFreeSlots(Team))
    {
        Player* plr = *playerVec->begin();
        playerVec->pop_front();
        plr->SetBGTeam(Team);
        bg->AddPlayer(plr, Team);
        ErasePlayerFromList(plr->GetGUID(), &m_queuedPlayers[i][j]);
    }
}

void CBattlegroundManager::EventQueueUpdate(bool forceStart)
{
    std::deque<Player*  > tempPlayerVec[2];
    uint32 i,j,k;
    Player* plr;
    CBattleground* bg;
    std::list<WoWGuid>::iterator it3, it4;
    //std::vector<Player*  >::iterator it6;
    std::map<uint32, CBattleground* >::iterator iitr;
    Arena* arena;
    int32 team;
    m_queueLock.Acquire();
    m_instanceLock.Acquire();

    for(i = 0; i < BATTLEGROUND_NUM_TYPES; i++)
    {
        for(j = 0; j < MAX_LEVEL_GROUP; ++j)
        {
            if(!m_queuedPlayers[i][j].size())
                continue;

            tempPlayerVec[0].clear();
            tempPlayerVec[1].clear();

            for(it3 = m_queuedPlayers[i][j].begin(); it3 != m_queuedPlayers[i][j].end();)
            {
                it4 = it3++;
                plr = objmgr.GetPlayer(*it4);

                if(!plr || GetLevelGrouping(plr->getLevel()) != j)
                {
                    m_queuedPlayers[i][j].erase(it4);
                    continue;
                }

                uint32 queueSlot = plr->GetBGQueueSlotByBGType(i);
                if(queueSlot >= 3)
                    continue;

                if( !plr->m_bgIsQueued[queueSlot] )
                {
                    // We've since cancelled our queue
                    m_queuedPlayers[i][j].erase(it4);
                    continue;
                }

                // queued to a specific instance id?
                if(plr->m_bgQueueInstanceId[queueSlot] != 0 && plr->m_bgIsQueued[queueSlot])
                {
                    if( m_instances[i].empty() )
                        continue;

                    iitr = m_instances[i].find( plr->m_bgQueueInstanceId[queueSlot] );
                    if(iitr == m_instances[i].end())
                    {
                        // queue no longer valid
                        //plr->GetSession()->SystemMessage("Your queue on battleground instance id %u is no longer valid. Reason: Instance Deleted.", plr->m_bgQueueInstanceId[queueSlot]);
                        plr->RemoveFromBattlegroundQueue(queueSlot);
                        //m_queuedPlayers[i][j].erase(it4);
                        SendBattlegroundQueueStatus(plr, queueSlot);
                        continue;
                    }

                    // can we join?
                    bg = (*iitr).second;
                    if(bg->CanPlayerJoin(plr))
                    {
                        bg->AddPlayer(plr, plr->GetTeam());
                        m_queuedPlayers[i][j].erase(it4);
                    }
                }
                else
                {
                    if(IS_ARENA(i))
                        tempPlayerVec[0].push_back(plr);
                    else
                        tempPlayerVec[plr->GetTeam()].push_back(plr);
                }
            }

            // try to join existing instances
            for(iitr = m_instances[i].begin(); iitr != m_instances[i].end(); iitr++)
            {
                if( iitr->second->HasEnded() || iitr->second->GetLevelGroup() != j)
                    continue;

                if(IS_ARENA(i))
                {
                    arena = castPtr<Arena>(iitr->second);
                    if(arena->Rated())
                        continue;

                    team = arena->GetFreeTeam();
                    while(team >= 0 && tempPlayerVec[0].size())
                    {
                        plr = *tempPlayerVec[0].begin();
                        tempPlayerVec[0].pop_front();
                        plr->SetBGTeam(team);
                        arena->AddPlayer(plr, team);
                        ErasePlayerFromList(plr->GetGUID(), &m_queuedPlayers[i][j]);
                        team = arena->GetFreeTeam();
                    }
                }
                else
                {
                    bg = iitr->second;
                    if(bg)
                    {
                        int size = (int)std::min(tempPlayerVec[0].size(),tempPlayerVec[1].size());
                        for(int counter = 0; (counter < size) && (bg->IsFull() == false); counter++)
                        {
                            AddPlayerToBgTeam(bg, &tempPlayerVec[0], i, j, 0);
                            AddPlayerToBgTeam(bg, &tempPlayerVec[1], i, j, 1);
                        }

                        while (tempPlayerVec[0].size() > 0 && bg->HasFreeSlots(0))
                        {
                            AddPlayerToBgTeam(bg, &tempPlayerVec[0], i, j, 0);
                        }
                        while (tempPlayerVec[1].size() > 0 && bg->HasFreeSlots(1))
                        {
                            AddPlayerToBgTeam(bg, &tempPlayerVec[1], i, j, 1);
                        }
                    }
                }
            }

            if(IS_ARENA(i))
            {
                // enough players to start a round?
                if((forceStart == true && tempPlayerVec[0].size() >= 1) ||
                   (tempPlayerVec[0].size() >= GetBGMinPlayers(i)))
                {
                    if(CanCreateInstance(i,j))
                    {
                        arena = castPtr<Arena>(CreateInstance(i, j));
                        team = arena->GetFreeTeam();
                        while(!arena->IsFull() && tempPlayerVec[0].size() && team >= 0)
                        {
                            plr = *tempPlayerVec[0].begin();
                            tempPlayerVec[0].pop_front();

                            plr->SetBGTeam(team);
                            arena->AddPlayer(plr, team);
                            team = arena->GetFreeTeam();

                            // remove from the main queue (painful!)
                            ErasePlayerFromList(plr->GetGUID(), &m_queuedPlayers[i][j]);
                        }
                    }
                }
            }
            else
            {
                if( (forceStart == true && (tempPlayerVec[0].size() >= 1 || tempPlayerVec[1].size() >= 1)) ||
                    (tempPlayerVec[0].size() >= GetBGMinPlayers(i) && tempPlayerVec[1].size() >= GetBGMinPlayers(i)) )
                {
                    if(CanCreateInstance(i,j))
                    {
                        bg = CreateInstance(i,j);
                        if( bg == NULL )
                        {
                            // creation failed
                            for(k = 0; k < 2; ++k)
                            {
                                while(tempPlayerVec[k].size())
                                {
                                    plr = *tempPlayerVec[k].begin();
                                    tempPlayerVec[k].pop_front();
                                    ErasePlayerFromList(plr->GetGUID(), &m_queuedPlayers[i][j]);
                                }
                            }
                        }
                        else
                        {
                            // push as many as possible in
                            if (forceStart)
                            {
                                for(k = 0; k < 2; ++k)
                                {
                                    while(tempPlayerVec[k].size() && bg->HasFreeSlots(k))
                                    {
                                        AddPlayerToBgTeam(bg, &tempPlayerVec[k], i, j, k);
                                    }
                                }
                            }
                            else
                            {
                                int size = (int)std::min(tempPlayerVec[0].size(),tempPlayerVec[1].size());
                                for(int counter = 0; (counter < size) && (bg->IsFull() == false); counter++)
                                {
                                    AddPlayerToBgTeam(bg, &tempPlayerVec[0], i, j, 0);
                                    AddPlayerToBgTeam(bg, &tempPlayerVec[1], i, j, 1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* Handle paired arena team joining */
    Group *group1, *group2;
    std::list<uint32>::iterator itz;
    uint32 teamids[2] = {0,0}, avgRating[2] = {0,0}, n = 0;
    for(i = BATTLEGROUND_ARENA_2V2; i <= BATTLEGROUND_ARENA_5V5; i++)
    {
        if(!forceStart && m_queuedGroups[i].size() < 2)      /* got enough to have an arena battle ;P */
            continue;

        for (j=0; j<(uint32)m_queuedGroups[i].size(); j++)
        {
            group1 = group2 = NULL;
            n = RandomUInt((uint32)m_queuedGroups[i].size()) - 1;
            for(itz = m_queuedGroups[i].begin(); itz != m_queuedGroups[i].end() && n>0; itz++)
                --n;

            if(itz == m_queuedGroups[i].end())
                itz=m_queuedGroups[i].begin();

            if(itz == m_queuedGroups[i].end())
            {
                sLog.Error("BattlegroundMgr", "Internal error at %s:%u", __FILE__, __LINE__);
                m_queueLock.Release();
                m_instanceLock.Release();
                return;
            }

            group1 = objmgr.GetGroupById(*itz);
            if (group1 == NULL)
            {
                continue;
            }

            if (forceStart && m_queuedGroups[i].size() == 1)
            {
                if (CreateArenaType(i, group1, NULL) == -1) return;
                m_queuedGroups[i].remove(group1->GetID());
                group1->m_isqueued = false;
                continue;
            }

            teamids[0] = GetArenaGroupQInfo(group1, i, &avgRating[0]);

            std::list<uint32> possibleGroups;

            possibleGroups.clear();
            for(itz = m_queuedGroups[i].begin(); itz != m_queuedGroups[i].end(); itz++)
            {
                group2 = objmgr.GetGroupById(*itz);
                if (group2)
                {
                    teamids[1] = GetArenaGroupQInfo(group2, i, &avgRating[1]);
                    uint32 delta = abs((int32)avgRating[0] - (int32)avgRating[1]);
                    if (teamids[0] != teamids[1] && delta <= 150)
                    {
                        possibleGroups.push_back(group2->GetID());
                    }
                }
            }

            if (possibleGroups.size() > 0)
            {
                n = RandomUInt((uint32)possibleGroups.size()) - 1;
                for(itz = possibleGroups.begin(); itz != possibleGroups.end() && n>0; itz++)
                    --n;

                if(itz == possibleGroups.end())
                    itz=possibleGroups.begin();

                if(itz == possibleGroups.end())
                {
                    sLog.Error("BattlegroundMgr", "Internal error at %s:%u", __FILE__, __LINE__);
                    m_queueLock.Release();
                    m_instanceLock.Release();
                    return;
                }

                group2 = objmgr.GetGroupById(*itz);
                if (group2)
                {
                    if (CreateArenaType(i, group1, group2) == -1) return;
                    m_queuedGroups[i].remove(group1->GetID());
                    group1->m_isqueued = false;
                    m_queuedGroups[i].remove(group2->GetID());
                    group2->m_isqueued = false;
                }
            }
        }
    }

    m_queueLock.Release();
    m_instanceLock.Release();
}

void CBattlegroundManager::RemovePlayerFromQueues(Player* plr)
{
    m_queueLock.Acquire();

    uint32 lgroup = GetLevelGrouping(plr->getLevel());

    uint32 i;
    for(i = 0; i < 2; i++)
    {
        std::list<WoWGuid>::iterator itr2, itr = m_queuedPlayers[plr->m_bgQueueType[i]][lgroup].begin();
        while(itr != m_queuedPlayers[plr->m_bgQueueType[i]][lgroup].end())
        {
            itr2 = itr;
            itr2++;
            if((*itr) == plr->GetGUID())
            {
                sLog.outDebug("BattlegroundManager", "Removing player %u from queue instance %u type %u", plr->GetLowGUID(), plr->m_bgQueueInstanceId[i], plr->m_bgQueueType[i]);
                m_queuedPlayers[plr->m_bgQueueType[i]][lgroup].erase(itr);
            }

            itr = itr2;
        }
        plr->m_bgIsQueued[i] = false;
        plr->SetBGTeam(plr->GetTeam());
        plr->m_pendingBattleground[i]=NULL;
        plr->m_bgQueueType[i] = 0;
        plr->m_bgQueueInstanceId[i] = 0;
        plr->m_bgQueueTime[i] = 0;
    }
    m_queueLock.Release();
}

void CBattlegroundManager::RemoveGroupFromQueues(Group * grp)
{
    m_queueLock.Acquire();
    for(uint32 i = BATTLEGROUND_ARENA_2V2; i < BATTLEGROUND_ARENA_5V5+1; i++)
    {
        for(std::list<uint32>::iterator itr = m_queuedGroups[i].begin(); itr != m_queuedGroups[i].end(); )
        {
            if((*itr) == grp->GetID())
                itr = m_queuedGroups[i].erase(itr);
            else
                ++itr;
        }
    }

    for(GroupMembersSet::iterator itr = grp->GetSubGroup(0)->GetGroupMembersBegin(); itr != grp->GetSubGroup(0)->GetGroupMembersEnd(); itr++)
        if((*itr)->m_loggedInPlayer)
        {
            for(uint32 i = 0; i < 2; i++)
            {
                (*itr)->m_loggedInPlayer->m_bgIsQueued[i] = false;
                SendBattlegroundQueueStatus((*itr)->m_loggedInPlayer, 0);
            }
        }

    grp->m_isqueued = false;
    m_queueLock.Release();
}


bool CBattlegroundManager::CanCreateInstance(uint32 Type, uint32 LevelGroup)
{
    switch(Type)
    {
    case BATTLEGROUND_ALTERAC_VALLEY:
        {
            if(!sWorld.av_enabled)
                return false;
        }break;

    case BATTLEGROUND_WARSONG_GULCH:
        {
            if(!sWorld.wsg_enabled)
                return false;
        }break;

    case BATTLEGROUND_ARATHI_BASIN:
        {
            if(!sWorld.ab_enabled)
                return false;
        }break;

    case BATTLEGROUND_EYE_OF_THE_STORM:
        {
            if(!sWorld.eots_enabled)
                return false;
        }break;

    case BATTLEGROUND_STRAND_OF_THE_ANCIENTS:
        {
            if(!sWorld.sota_enabled)
                return false;
        }break;

    case BATTLEGROUND_ISLE_OF_CONQUEST:
        {
            if(!sWorld.ioc_enabled)
                return false;
        }break;

    case BATTLEGROUND_RANDOM:
        {
            return false;
        }break;
    }
    return true;
}

CBattleground::CBattleground( MapInstance* instance, uint32 id, uint32 levelgroup, uint32 type) : m_mapInstance(instance), m_id(id), m_type(type), m_levelGroup(levelgroup)
{
    m_nextPvPUpdateTime = 0;
    m_countdownStage = 0;
    m_ended = false;
    m_started = false;
    m_losingteam = 0xff;
    m_startTime = (uint32)UNIXTIME;
    m_lastResurrect = (uint32)UNIXTIME;

    /* create raid groups */
    for(uint32 i = 0; i < 2; i++)
    {
        m_groups[i] = new Group(true);
        m_groups[i]->SetFlag(GROUP_FLAG_DONT_DISBAND_WITH_NO_MEMBERS | GROUP_FLAG_BATTLEGROUND_GROUP);
        m_groups[i]->ExpandToRaid();
    }
}

void CBattleground::Init()
{

}

CBattleground::~CBattleground()
{
    for(uint32 i = 0; i < 2; i++)
    {
        PlayerInfo *inf;
        for(uint32 j = 0; j < m_groups[i]->GetSubGroupCount(); ++j) {
            for(GroupMembersSet::iterator itr = m_groups[i]->GetSubGroup(j)->GetGroupMembersBegin(); itr != m_groups[i]->GetSubGroup(j)->GetGroupMembersEnd();) {
                inf = (*itr);
                ++itr;
                m_groups[i]->RemovePlayer(inf);
            }
        }
        delete m_groups[i];
    }
}

void CBattleground::UpdatePvPData()
{
    if(m_type >= BATTLEGROUND_ARENA_2V2 && m_type <= BATTLEGROUND_ARENA_5V5)
    {
        if(!m_ended)
        {
            return;
        }
    }

    if(UNIXTIME >= m_nextPvPUpdateTime)
    {
        m_mainLock.Acquire();
        WorldPacket data(10*(m_players[0].size()+m_players[1].size())+50);
        BuildPvPUpdateDataPacket(&data);
        DistributePacketToAll(&data);
        m_mainLock.Release();

        m_nextPvPUpdateTime = UNIXTIME + 2;
    }
}

void CBattleground::BuildPvPUpdateDataPacket(WorldPacket * data)
{
    if(IsArena() && !m_ended)
        return;

    data->Initialize(SMSG_PVP_LOG_DATA);
    data->reserve(1+1+4+40*(m_players[0].size()+m_players[1].size()));

    BGScore * bs;
    *data << uint8(IsArena());
    if(IsArena())
    {   // send arena teams info
        Arena* arena= castPtr<Arena>(this);
        ArenaTeam * teams[2] = {NULL,NULL};
        uint32 ratingNegativeChange[2] = {0,0}, ratingPositiveChange[2] = {0,0};    // Value in ratingNegativeChange is displayed with minus sign in the client
        if(Rated())
        {
            teams[0] = objmgr.GetArenaTeamById(arena->m_teams[0]);
            teams[1] = objmgr.GetArenaTeamById(arena->m_teams[1]);
        }

        for(uint32 i=0; i<2; i++)
        {
            if(m_deltaRating[i] >= 0)
                ratingPositiveChange[i] = m_deltaRating[i];
            else
                ratingNegativeChange[i] = -m_deltaRating[i];
        }
        *data << ratingNegativeChange[0];
        *data << ratingPositiveChange[0];
        *data << ratingNegativeChange[1];
        *data << ratingPositiveChange[1];

        if(teams[0])
            *data << teams[0]->m_name;
        else
            *data << uint8(0);

        if(teams[1])
            *data << teams[1]->m_name;
        else
            *data << uint8(0);

        *data << uint64(0);
    }

    *data << uint8(m_ended);
    if(m_ended)
    {
        if(IsArena())   // Looks like for arena it should be winning team and not losing
            *data << uint8(m_losingteam > 0 ? 0 : 1);
        else
            *data << uint8(m_losingteam);
    }

    size_t pos = data->wpos();
    *data << uint32(0); //will be set to correct number later //uint32(m_players[0].size() + m_players[1].size());

    uint32 count = 0;
    uint32 fcount = GetBGPvPDataCount(GetType());
    for(uint32 i = 0; i < 2; i++)
    {
        for(std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); itr++)
        {
            bs = &(*itr)->m_bgScore;
            *data << (*itr)->GetGUID();
            *data << bs->KillingBlows;
            if(IsArena())
                *data << uint8((*itr)->GetBGTeam());
            else
            {
                *data << bs->HonorableKills;
                *data << bs->Deaths;
                *data << bs->BonusHonor;
            }

            *data << bs->DamageDone;
            *data << bs->HealingDone;
            *data << fcount;    // count of values after this
            for(uint32 j = 0; j < fcount; ++j)
                *data << bs->MiscData[j];

            count++;
        }
    }

    // Have to set correct number of players sent in log since we skip invisible GMs
    *(uint32*)&data->contents()[pos] = count;
    *data << uint32(0); // unknown
}

void CBattleground::AddPlayer(Player* plr, uint32 team)
{
    m_mainLock.Acquire();

    /* This is called when the player is added, not when they port. So, they're essentially still queued, but not inside the bg yet */
    m_pendPlayers[team].insert(plr->GetGUID());

    uint32 queueSlot = plr->GetBGQueueSlotByBGType(GetType());
    if(queueSlot >= 3)
    {
        m_mainLock.Release();
        return;
    }

    plr->m_pendingBattleground[queueSlot] = this;
    plr->m_bgIsQueued[queueSlot] = false;

    /* Send a packet telling them that they can enter */
    BattlegroundManager.SendBattlegroundQueueStatus(plr, queueSlot);

    /* Add the queue time */
    BattlegroundManager.AddAverageQueueTime(GetType(), (uint32)(UNIXTIME-plr->m_bgQueueTime[queueSlot]));

    m_mainLock.Release();
}

void CBattleground::RemovePendingPlayer(Player* plr)
{
    m_mainLock.Acquire();
    m_pendPlayers[0].erase(plr->GetGUID());
    m_pendPlayers[1].erase(plr->GetGUID());

    /* send a null bg update (so they don't join) */
    for(uint32 i = 0; i < 2; i++)
    {
        if( plr->m_pendingBattleground[i] &&
            plr->m_pendingBattleground[i] == this )
        {
            if( plr->m_pendingBattleground[i]->IsArena() )
                plr->m_bgRatedQueue = false;

            plr->m_pendingBattleground[i] = NULL;
            BattlegroundManager.SendBattlegroundQueueStatus(plr, i);
            break;
        }
    }
    plr->SetBGTeam(plr->GetTeam());

    m_mainLock.Release();
}

void CBattleground::OnPlayerPushed(Player* plr)
{
    if( plr->GetGroup() && !Rated() )
        plr->GetGroup()->RemovePlayer(plr->m_playerInfo);

    plr->PopPendingUpdates();

    if( plr->GetGroup() == NULL )
        m_groups[plr->GetBGTeam()]->AddMember( plr->m_playerInfo );
}

void CBattleground::PortPlayer(Player* plr, bool skip_teleport /* = false*/)
{
    m_mainLock.Acquire();
    if(m_ended)
    {
        for(uint32 i = 0; i < 2; i++)
        {
            if( plr->m_pendingBattleground[i] == this )
            {
                plr->m_pendingBattleground[i] = NULL;
                plr->m_bgIsQueued[i] = false;
            }
        }
        sChatHandler.SystemMessage(plr->GetSession(), "You cannot join this battleground as it has already ended.");
        BattlegroundManager.SendBattlegroundQueueStatus(plr, 0);
        BattlegroundManager.SendBattlegroundQueueStatus(plr, 1);
        m_mainLock.Release();
        return;
    }

    m_pendPlayers[0].erase(plr->GetGUID());
    m_pendPlayers[1].erase(plr->GetGUID());
    if(m_players[plr->GetBGTeam()].find(plr) != m_players[plr->GetBGTeam()].end())
    {
        m_mainLock.Release();
        return;
    }

    if( plr->m_bg != NULL )
    {
        plr->m_bg->RemovePlayer(plr, true);     // don't bother porting them out, we're about to port them anyway
        plr->m_bg = NULL;
    }

    plr->FullHPMP();
    plr->SetTeam(plr->GetBGTeam());

    WorldPacket data(SMSG_BATTLEGROUND_PLAYER_JOINED, 8);
    data << plr->GetGUID();
    DistributePacketToTeam(&data, plr->GetBGTeam());

    m_players[plr->GetBGTeam()].insert(plr);

    if( !skip_teleport && plr->IsInWorld() )
        plr->RemoveFromWorld();

    for(uint32 i = 0; i < 2; i++)
    {
        if( plr->m_pendingBattleground[i] == this )
        {
            plr->m_pendingBattleground[i] = NULL;
            plr->m_bgSlot = i;
            BattlegroundManager.SendBattlegroundQueueStatus(plr, i);
        }
    }

    /* remove from any auto queue remove events */

    if(!plr->IsPvPFlagged())
        plr->SetPvPFlag();

    // Remove auras that disappear on PvP Enter.
    plr->m_AuraInterface.RemoveAllAurasByInterruptFlag( AURA_INTERRUPT_ON_PVP_ENTER );

    /* Reset the score */
    memset(&plr->m_bgScore, 0, sizeof(BGScore));

    /* update pvp data */
    UpdatePvPData();

    /* add the player to the group */
    if(plr->GetGroup() && !Rated())
    {
        // remove them from their group
        plr->GetGroup()->RemovePlayer( plr->m_playerInfo );
    }

    if(!m_countdownStage)
    {
        m_countdownStage = 1;
    }

    plr->m_bg = this;
    if(!skip_teleport)
    {
        /* This is where we actually teleport the player to the battleground. */
        plr->SafeTeleport(m_mapInstance, GetStartingCoords(plr->GetBGTeam()));
    }

    m_mainLock.Release();
}

CBattleground* CBattlegroundManager::CreateInstance(uint32 Type, uint32 LevelGroup)
{
    // Crow: Just in case.
    if(!CanCreateInstance(Type, LevelGroup))
        return NULL;

    CreateBattlegroundFunc cfunc = GetBGCFunc(Type);
    MapInstance* mgr = NULL;
    CBattleground* bg = NULL;
    bool isWeekend = false;
    struct tm tm;
    time_t t;
    uint32 iid;
    int n;

    if(Type == BATTLEGROUND_ARENA_2V2 || Type == BATTLEGROUND_ARENA_3V3 || Type == BATTLEGROUND_ARENA_5V5)
    {
        /* arenas follow a different procedure. */
        uint32 mapid = arena_map_ids[RandomUInt(4)];
        mgr = sWorldMgr.CreateBattlegroundInstance(mapid);
        if(mgr == NULL)
        {
            sLog.Error("BattlegroundManager", "Arena CreateInstance() call failed for map %u, type %u, level group %u", mapid, Type, LevelGroup);
            return NULL;        // Shouldn't happen
        }

        uint32 players_per_side;
        switch(Type)
        {
        case BATTLEGROUND_ARENA_2V2:
            players_per_side = 2;
            break;

        case BATTLEGROUND_ARENA_3V3:
            players_per_side = 3;
            break;

        case BATTLEGROUND_ARENA_5V5:
            players_per_side = 5;
            break;
        default:
            players_per_side = 0;
            break;
        }

        iid = ++m_maxBattlegroundId;
        bg = new Arena(mgr, iid, LevelGroup, Type, players_per_side);
        bg->Init();
        mgr->m_battleground = bg;
        sLog.Success("BattlegroundManager", "Created arena battleground type %u for level group %u on map %u.", Type, LevelGroup, mapid);
        m_instanceLock.Acquire();
        m_instances[Type].insert( std::make_pair(iid, bg) );
        m_instanceLock.Release();
        return bg;
    }

    if(cfunc == NULL)
    {
        sLog.Error("BattlegroundManager", "Could not find CreateBattlegroundFunc pointer for type %u level group %u", Type, LevelGroup);
        return NULL;
    }

    /* Create Map Manager */
    mgr = sWorldMgr.CreateBattlegroundInstance(GetBGMapID(Type));
    if(mgr == NULL)
    {
        sLog.Error("BattlegroundManager", "CreateInstance() call failed for map %u, type %u, level group %u", GetBGMapID(Type), Type, LevelGroup);
        return NULL;        // Shouldn't happen
    }

    t = UNIXTIME;
#ifdef WIN32
    tm = *localtime(&t);
#else
    localtime_r(&t, &tm);
#endif

    switch (Type)
    {
        case BATTLEGROUND_WARSONG_GULCH: n = 0; break;
        case BATTLEGROUND_ARATHI_BASIN: n = 1; break;
        case BATTLEGROUND_EYE_OF_THE_STORM: n = 2; break;
        case BATTLEGROUND_ALTERAC_VALLEY: n = 3; break;
        case BATTLEGROUND_STRAND_OF_THE_ANCIENTS: n = 4; break;
        default: n = 0; break;
    }
    if (((tm.tm_yday / 7) % 4) == n)
    {
        /* Set weekend from thursday night at midnight until tuesday morning */
        isWeekend = tm.tm_wday >= 5 || tm.tm_wday < 2;
    }

    /* Call the create function */
    iid = ++m_maxBattlegroundId;
    bg = cfunc(mgr, iid, LevelGroup, Type);
    bg->Init();
    bg->SetIsWeekend(isWeekend);
    mgr->m_battleground = bg;
    sLog.Success("BattlegroundManager", "Created battleground type %u for level group %u.", Type, LevelGroup);

    m_instanceLock.Acquire();
    m_instances[Type].insert( std::make_pair(iid, bg) );
    m_instanceLock.Release();

    return bg;
}

void CBattlegroundManager::DeleteBattleground(CBattleground* bg)
{
    uint32 i = bg->GetType();
    uint32 j = bg->GetLevelGroup();
    Player* plr;

    m_instanceLock.Acquire();
    m_queueLock.Acquire();
    m_instances[i].erase(bg->GetId());

    /* erase any queued players */
    std::list<WoWGuid>::iterator itr = m_queuedPlayers[i][j].begin(), it2;
    for(; itr != m_queuedPlayers[i][j].end();)
    {
        it2 = itr++;
        plr = objmgr.GetPlayer(*it2);
        if(!plr)
        {
            m_queuedPlayers[i][j].erase(it2);
            continue;
        }

        for(uint32 z= 0; z < 2; ++z)
        {
            if(plr && plr->m_bgQueueInstanceId[z] == bg->GetId())
            {
                sChatHandler.SystemMessageToPlr(plr, "Your queue on battleground instance %u is no longer valid, the instance no longer exists.", bg->GetId());
                SendBattlegroundQueueStatus(plr, z);
                plr->m_bgIsQueued[z] = false;
                m_queuedPlayers[i][j].erase(it2);
            }
        }
    }

    m_queueLock.Release();
    m_instanceLock.Release();

    bg = NULL;
    delete bg;
}

GameObject* CBattleground::SpawnGameObject(uint32 entry,float x, float y, float z, float o, uint32 flags, uint32 faction, float scale)
{
    GameObject* go = m_mapInstance->CreateGameObject(entry);
    if(go == NULL || !go->CreateFromProto(entry, m_mapInstance->GetMapId(), x, y, z, o))
        return NULL;

    go->SetUInt32Value(GAMEOBJECT_FACTION,faction);
    go->SetFloatValue(OBJECT_FIELD_SCALE_X,scale);
    go->SetUInt32Value(GAMEOBJECT_FLAGS, flags);
    go->SetInstanceID(m_mapInstance->GetInstanceID());
    go->m_battleground = this;

    return go;
}

Creature* CBattleground::SpawnCreature(uint32 entry,float x, float y, float z, float o)
{
    if (Creature *c = m_mapInstance->CreateCreature(entry))
    {
        c->Load(m_mapInstance->GetMapId(), x, y, z, o, m_mapInstance->iInstanceMode);
        c->PushToWorld(m_mapInstance);
		return c;
    }
    return NULL;
}

void CBattleground::SendChatMessage(uint32 Type, uint64 Guid, const char * Format, ...)
{
    char msg[500];
    va_list ap;
    va_start(ap, Format);
    vsnprintf(msg, 500, Format, ap);
    va_end(ap);
    WorldPacket data;
    sChatHandler.FillMessageData(&data, false, Type, LANG_UNIVERSAL, 0, 0, "", msg, "", 0);
    DistributePacketToAll(&data);
}

void CBattleground::DistributePacketToAll(WorldPacket * packet)
{
    m_mainLock.Acquire();
    for(int8 i = 0; i < 2; i++)
    {
        if( !m_players[i].size() )
            continue;

        for(std::set<Player*  >::iterator itr = m_players[i].begin(); itr != m_players[i].end(); itr++)
            if( (*itr) && (*itr)->GetSession() )
                (*itr)->GetSession()->SendPacket(packet);
    }
    m_mainLock.Release();
}

void CBattleground::DistributePacketToTeam(WorldPacket * packet, uint32 Team)
{
    m_mainLock.Acquire();
    if( !m_players[Team].size() )
    {
        m_mainLock.Release();
        return;
    }
    for(std::set<Player*  >::iterator itr = m_players[Team].begin(); itr != m_players[Team].end(); itr++)
        if( (*itr) && (*itr)->GetSession() )
            (*itr)->GetSession()->SendPacket(packet);
    m_mainLock.Release();
}

void CBattleground::PlaySoundToAll(uint32 Sound)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << Sound;
    DistributePacketToAll(&data);
}

void CBattleground::PlaySoundToTeam(uint32 Team, uint32 Sound)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << Sound;
    DistributePacketToTeam(&data, Team);
}

void CBattlegroundManager::SendBattlegroundQueueStatus(Player* plr, uint32 queueSlot)
{
    //if( queueSlot > 3 )
        return;

    WorldPacket data(SMSG_BATTLEFIELD_STATUS, 40);
    data << uint32(queueSlot);
    plr->GetSession()->SendPacket(&data);
}

void CBattleground::RemovePlayer(Player* plr, bool logout)
{
    WorldPacket data(SMSG_BATTLEGROUND_PLAYER_LEFT, 30);
    data << plr->GetGUID();

    m_mainLock.Acquire();
    m_players[0].erase(plr);
    m_players[1].erase(plr);

    DistributePacketToAll(&data);

    memset(&plr->m_bgScore, 0, sizeof(BGScore));
    OnRemovePlayer(plr);

    plr->m_bg = NULL;
    plr->FullHPMP();

    /* are we in the group? */
    if(plr->GetGroup() == m_groups[plr->GetBGTeam()])
        plr->GetGroup()->RemovePlayer( plr->m_playerInfo );

    // reset team
    plr->SetBGTeam(TEAM_NONE);

    /* revive the player if he is dead */
    if(!logout && !plr->isAlive())
    {
        plr->SetUInt32Value(UNIT_FIELD_HEALTH, plr->GetUInt32Value(UNIT_FIELD_MAXHEALTH));
        plr->ResurrectPlayer();
    }

    /* teleport out */
    if(!logout)
    {
        if ( !m_ended )
        {
            SpellEntry *spellInfo = dbcSpell.LookupEntry( BG_DESERTER );// escape from BG
            if ( spellInfo )
            {
                SpellCastTargets targets;
                targets.m_unitTarget = plr->GetGUID();
                if(Spell* sp = new Spell(plr,spellInfo))
                    sp->prepare(&targets, true);
            }
        }

        if(!IS_INSTANCE(plr->m_bgEntryPointMap))
        {
            LocationVector vec(plr->m_bgEntryPointX, plr->m_bgEntryPointY, plr->m_bgEntryPointZ, plr->m_bgEntryPointO);
            plr->SafeTeleport(plr->m_bgEntryPointMap, plr->m_bgEntryPointInstance, vec);
        }
        else
        {
            LocationVector vec(plr->GetBindPositionX(), plr->GetBindPositionY(), plr->GetBindPositionZ());
            plr->SafeTeleport(plr->GetBindMapId(), 0, vec);
        }
        BattlegroundManager.SendBattlegroundQueueStatus(plr, 0);
        BattlegroundManager.SendBattlegroundQueueStatus(plr, 1);

        /* send some null world states */
        data.Initialize(SMSG_INIT_WORLD_STATES);
        data << uint32(plr->GetMapId()) << uint32(0) << uint32(0) << uint16(0);
        plr->GetSession()->SendPacket(&data);
    }

    if(!m_ended && m_players[0].size() == 0 && m_players[1].size() == 0)
    {
        /* create an inactive event */
    }

    plr->SetBGTeam(plr->GetTeam());
    m_mainLock.Release();
}

void CBattleground::SendPVPData(Player* plr)
{
    m_mainLock.Acquire();
    if( m_players[0].size() == 0 && m_players[1].size() == 0 )
    {
        m_mainLock.Release();
        return;
    }

    WorldPacket data(10*(m_players[0].size()+m_players[1].size())+50);
    BuildPvPUpdateDataPacket(&data);
    plr->GetSession()->SendPacket(&data);

    m_mainLock.Release();
}

void CBattleground::EventCreate()
{
    OnCreate();
}

int32 CBattleground::event_GetInstanceID()
{
    return m_mapInstance->GetInstanceID();
}

void CBattleground::EventCountdown()
{
    if(m_countdownStage == 1)
    {
        m_countdownStage = 2;
        SendChatMessage( CHAT_MSG_BG_SYSTEM_NEUTRAL, 0, "The battle for %s begins in 2 minutes.", GetName() );
    }
    else if(m_countdownStage == 2)
    {
        m_countdownStage = 3;
        SendChatMessage( CHAT_MSG_BG_SYSTEM_NEUTRAL, 0, "The battle for %s begins in 1 minute.", GetName() );
    }
    else if(m_countdownStage == 3)
    {
        m_countdownStage = 4;
        SendChatMessage( CHAT_MSG_BG_SYSTEM_NEUTRAL, 0, "The battle for %s begins in 30 seconds. Prepare yourselves!", GetName() );
    }
    else
    {
        SendChatMessage( CHAT_MSG_BG_SYSTEM_NEUTRAL, 0, "Let the battle for %s begin!", GetName() );
        Start();
    }
}

void CBattleground::Start()
{
    OnStart();
    m_startTime = (uint32)UNIXTIME;
}

void CBattleground::Close()
{
    /* remove all players from the battleground */
    m_mainLock.Acquire();
    m_ended = true;
    for(uint32 i = 0; i < 2; i++)
    {
        for(std::set<Player*>::iterator itr = m_players[i].begin(), itr2; itr != m_players[i].end();)
        {
            itr2 = itr;
            ++itr;
            RemovePlayer(*itr2, false);
        }

        for(std::set<WoWGuid>::iterator itr = m_pendPlayers[i].begin(); itr != m_pendPlayers[i].end();)
        {
            if(Player *plr = objmgr.GetPlayer(*itr))
            {
                itr++;
                RemovePendingPlayer(plr);
            }
            else itr = m_pendPlayers[i].erase(itr);
        }
    }

    /* call the virtual onclose for cleanup etc */
    OnClose();

    m_mainLock.Release();
}

Creature* CBattleground::SpawnSpiritGuide(float x, float y, float z, float o, bool horde)
{
    Creature* pCreature = m_mapInstance->CreateCreature(13116 + horde);
    if (pCreature == NULL)
        return NULL;

    pCreature->Load(m_mapInstance->GetMapId(), x, y, z, o, 0);
    pCreature->SetInstanceID(m_mapInstance->GetInstanceID());
    pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, 13116 + horde);
    pCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);

    pCreature->SetUInt32Value(UNIT_FIELD_HEALTH, 100000);
    pCreature->SetUInt32Value(UNIT_FIELD_MAXHEALTH, 10000);

    pCreature->setLevel(60);
    pCreature->SetFactionTemplate(84 - horde);
    pCreature->SetUInt32Value(UNIT_FIELD_BYTES_0, 0 | (2 << 8) | (1 << 16));

    pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, 22802);

    pCreature->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PLUS_MOB | UNIT_FLAG_IGNORE_PC | UNIT_FLAG_IGNORE_NPC); // 4928

    pCreature->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, 2000);
    pCreature->SetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME, 2000);
    pCreature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0.208f);
    pCreature->SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f);

    pCreature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 13337 + horde);
    pCreature->SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, 13337 + horde);

    pCreature->SetUInt32Value(UNIT_CHANNEL_SPELL, 22011);
    pCreature->SetUInt32Value(UNIT_MOD_CAST_SPEED, 1065353216);

    pCreature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER);
    pCreature->SetUInt32Value(UNIT_FIELD_BYTES_2, 1 | (0x10 << 8));

    pCreature->SetPvPFlag();

    pCreature->PushToWorld(m_mapInstance);
    return pCreature;
}

void CBattleground::QueuePlayerForResurrect(Player* plr, Creature* spirit_healer)
{
    m_mainLock.Acquire();
    std::map<Creature*, std::set<WoWGuid> >::iterator itr = m_resurrectMap.find(spirit_healer);
    if(itr != m_resurrectMap.end())
    {
        itr->second.insert(plr->GetGUID());
        plr->m_areaSpiritHealer_guid=spirit_healer->GetGUID();
        plr->CastSpell(plr,2584,true);
    }
    m_mainLock.Release();
}

void CBattleground::RemovePlayerFromResurrect(Player* plr, Creature* spirit_healer)
{
    m_mainLock.Acquire();
    std::map<Creature*, std::set<WoWGuid> >::iterator itr = m_resurrectMap.find(spirit_healer);
    if(itr != m_resurrectMap.end())
        itr->second.erase(plr->GetGUID());
    plr->m_areaSpiritHealer_guid=0;
    m_mainLock.Release();
}

void CBattleground::AddSpiritGuide(Creature* pCreature)
{
    if (pCreature == NULL)
        return;
    m_mainLock.Acquire();
    std::map<Creature*, std::set<WoWGuid> >::iterator itr = m_resurrectMap.find(pCreature);
    if(itr == m_resurrectMap.end())
    {
        std::set<WoWGuid> ti;
        m_resurrectMap.insert(std::make_pair(pCreature, ti));
    }
    m_mainLock.Release();
}

void CBattleground::RemoveSpiritGuide(Creature* pCreature)
{
    m_mainLock.Acquire();
    m_resurrectMap.erase(pCreature);
    m_mainLock.Release();
}

void CBattleground::EventResurrectPlayers()
{
    m_mainLock.Acquire();
    Player* plr;
    std::set<WoWGuid>::iterator itr;
    std::map<Creature*, std::set<WoWGuid> >::iterator i;
    WorldPacket data(50);
    for(i = m_resurrectMap.begin(); i != m_resurrectMap.end(); i++)
    {
        for(itr = i->second.begin(); itr != i->second.end(); itr++)
        {
            plr = m_mapInstance->GetPlayer(*itr);
            if(plr && plr->isDead())
            {
                data.Initialize(SMSG_SPELL_GO);
                data << plr->GetGUID().asPacked() << plr->GetGUID().asPacked() << uint8(0) << uint32(RESURRECT_SPELL) << uint32(256) << uint32(0) << uint32(getMSTime());
                data << uint8(1) << plr->GetGUID() << uint8(0) << uint32(2) << plr->GetGUID().asPacked();
                plr->SendMessageToSet(&data, true);

                plr->ResurrectPlayer();
                plr->SpawnCorpseBones();
                plr->SetUInt32Value(UNIT_FIELD_HEALTH, plr->GetUInt32Value(UNIT_FIELD_MAXHEALTH));
                plr->CastSpell(plr, BG_REVIVE_PREPARATION, true);
            }
        }
        i->second.clear();
    }
    m_lastResurrect = (uint32)UNIXTIME;
    m_mainLock.Release();
}

void CBattlegroundManager::HandleArenaJoin(WorldSession * m_session, uint32 BattlegroundType, uint8 as_group, uint8 rated_match)
{
    Player* plr = m_session->GetPlayer();
    WoWGuid pguid = plr->GetGUID();
    uint32 lgroup = GetLevelGrouping(plr->getLevel());
    if(as_group && plr->GetGroup() == NULL)
        return;

    // Remove from all existing queues
    BattlegroundManager.RemovePlayerFromQueues(plr);

    Group * pGroup = plr->GetGroup();
    if(as_group)
    {
        if(pGroup->GetSubGroupCount() != 1)
        {
            m_session->SystemMessage("Sorry, raid groups joining battlegrounds are currently unsupported.");
            return;
        }
        if(pGroup->GetLeader() != plr->m_playerInfo)
        {
            m_session->SystemMessage("You must be the party leader to add a group to an arena.");
            return;
        }

        bool isQueued = false;
        GroupMembersSet::iterator itx;
        if(!rated_match)
        {
            /* add all players normally.. bleh ;P */
            pGroup->Lock();
            for(itx = pGroup->GetSubGroup(0)->GetGroupMembersBegin(); itx != pGroup->GetSubGroup(0)->GetGroupMembersEnd(); itx++)
            {
                if((*itx)->m_loggedInPlayer && ((*itx)->m_loggedInPlayer->m_bgIsQueued[0] ||(*itx)->m_loggedInPlayer->m_bgIsQueued[1] || (*itx)->m_loggedInPlayer->m_bgIsQueued[2]))
                    isQueued = true;
                if((*itx)->m_loggedInPlayer && !isQueued && !(*itx)->m_loggedInPlayer->m_bg)
                    HandleArenaJoin((*itx)->m_loggedInPlayer->GetSession(), BattlegroundType, 0, 0);
            }
            pGroup->Unlock();
            return;
        }
        else
        {
            ArenaTeam * pTargetTeam = NULL;
            uint32 teamType = 0;
            /* make sure all players are 70 */
            uint32 maxplayers = 0;
            switch(BattlegroundType)
            {
                case BATTLEGROUND_ARENA_3V3:
                    maxplayers=3;
                    teamType = ARENA_TEAM_TYPE_3V3;
                    break;

                case BATTLEGROUND_ARENA_5V5:
                    maxplayers=5;
                    teamType = ARENA_TEAM_TYPE_5V5;
                    break;

                case BATTLEGROUND_ARENA_2V2:
                    maxplayers=2;
                    teamType = ARENA_TEAM_TYPE_2V2;
                    break;
                default:
                    maxplayers=2;
                    teamType = ARENA_TEAM_TYPE_2V2;
                    break;
            }

            pTargetTeam = plr->m_playerInfo->arenaTeam[teamType];
            if(!pTargetTeam)
            {
                m_session->SystemMessage("You must be a member of an arena team to do that.");
                return;
            }

            pGroup->Lock();
            for(itx = pGroup->GetSubGroup(0)->GetGroupMembersBegin(); itx != pGroup->GetSubGroup(0)->GetGroupMembersEnd(); itx++)
            {
                if(maxplayers==0)
                {
                    m_session->SystemMessage("You have too many players in your party to join this type of arena.");
                    pGroup->Unlock();
                    return;
                }

                if((*itx)->lastLevel < 80)
                {
                    m_session->SystemMessage("Sorry, some of your party members are not level 80.");
                    pGroup->Unlock();
                    return;
                }

                if((*itx)->m_loggedInPlayer)
                {
                    if( !(*itx)->m_loggedInPlayer->IsInWorld() )
                    {
                        m_session->SystemMessage("One or more of your party members are offline.");
                        pGroup->Unlock();
                        return;
                    }

                    bool isQueued = false;
                    if( (*itx)->m_loggedInPlayer && ((*itx)->m_loggedInPlayer->m_bgIsQueued[0] || (*itx)->m_loggedInPlayer->m_bgIsQueued[1] || (*itx)->m_loggedInPlayer->m_bgIsQueued[2]))
                        isQueued = true;
                    if((*itx)->m_loggedInPlayer->m_bg || isQueued)
                    {
                        m_session->SystemMessage("One or more of your party members are already queued or inside a battleground.");
                        pGroup->Unlock();
                        return;
                    }

                    --maxplayers;
                }

                if(pTargetTeam && (*itx)->arenaTeam[teamType] != pTargetTeam)
                {
                    m_session->SystemMessage("One or more of your group members is not in your arena team.");
                    pGroup->Unlock();
                    return;
                }
            }

            for(itx = pGroup->GetSubGroup(0)->GetGroupMembersBegin(); itx != pGroup->GetSubGroup(0)->GetGroupMembersEnd(); itx++)
            {
                if((*itx)->m_loggedInPlayer)
                {
                    // FIXME: Force queueslot 0
                    (*itx)->m_loggedInPlayer->m_bgIsQueued[0] = true;
                    (*itx)->m_loggedInPlayer->m_bgQueueInstanceId[0] = 0;
                    (*itx)->m_loggedInPlayer->m_bgQueueType[0] = BattlegroundType;
                    (*itx)->m_loggedInPlayer->m_bgQueueTime[0] = (uint32)UNIXTIME;
                    (*itx)->m_loggedInPlayer->m_bgEntryPointX=(*itx)->m_loggedInPlayer->GetPositionX();
                    (*itx)->m_loggedInPlayer->m_bgEntryPointY=(*itx)->m_loggedInPlayer->GetPositionY();
                    (*itx)->m_loggedInPlayer->m_bgEntryPointZ=(*itx)->m_loggedInPlayer->GetPositionZ();
                    (*itx)->m_loggedInPlayer->m_bgEntryPointMap=(*itx)->m_loggedInPlayer->GetMapId();
                    (*itx)->m_loggedInPlayer->m_bgRatedQueue = rated_match ? true : false;
                    SendBattlegroundQueueStatus( (*itx)->m_loggedInPlayer, 0);
                }
            }

            pGroup->Unlock();
            pGroup->m_isqueued = true;

            m_queueLock.Acquire();
            m_queuedGroups[BattlegroundType].push_back(pGroup->GetID());
            m_queueLock.Release();
            sLog.Success("BattlegroundMgr", "Group %u is now in battleground queue for arena type %u", pGroup->GetID(), BattlegroundType);

            /* send the battleground status packet */

            return;
        }
    }


    /* Queue him! */
    m_queueLock.Acquire();
    m_queuedPlayers[BattlegroundType][lgroup].push_back(pguid);
    sLog.Success("BattlegroundMgr", "Player %u is now in battleground queue for {Arena %u}", plr->GetLowGUID(), BattlegroundType );

    /* send the battleground status packet */
    // use queueslot 0 since we can only join 1 arena :P
    plr->m_bgIsQueued[0] = true;
    plr->m_bgQueueInstanceId[0] = 0;
    plr->m_bgQueueType[0] = BattlegroundType;
    plr->m_bgRatedQueue = false;
    plr->m_bgQueueTime[0] = (uint32)UNIXTIME;
    SendBattlegroundQueueStatus( plr, 0);

    /* Set battleground entry point */
    plr->m_bgEntryPointX = plr->GetPositionX();
    plr->m_bgEntryPointY = plr->GetPositionY();
    plr->m_bgEntryPointZ = plr->GetPositionZ();
    plr->m_bgEntryPointMap = plr->GetMapId();
    plr->m_bgEntryPointInstance = plr->GetInstanceID();

    m_queueLock.Release();
}

bool CBattleground::CanPlayerJoin(Player* plr)
{
    return ( plr->bGMTagOn || HasFreeSlots(plr->GetBGTeam())&&(GetLevelGrouping(plr->getLevel())==GetLevelGroup())&&(!plr->HasAura(BG_DESERTER)));
}

void CBattleground::QueueAtNearestSpiritGuide(Player* plr, Creature* old)
{
    float dd;
    float dist = 999999.0f;
    Creature* cl = NULL;
    std::set<WoWGuid> *closest = NULL;
    std::map<Creature*, std::set<WoWGuid> >::iterator itr = m_resurrectMap.begin();
    for(; itr != m_resurrectMap.end(); itr++)
    {
        if( itr->first == old )
            continue;

        dd = plr->GetDistance2dSq(itr->first) < dist;
        if( dd < dist )
        {
            cl = itr->first;
            closest = &itr->second;
            dist = dd;
        }
    }

    if( closest != NULL )
    {
        closest->insert(plr->GetGUID());
        plr->m_areaSpiritHealer_guid=cl->GetGUID();
        plr->CastSpell(plr,2584,true);
    }
}

void CBattleground::GiveHonorToTeam(uint32 team, uint32 amt)
{
    for(std::set<Player*  >::iterator itx = m_players[team].begin(); itx != m_players[team].end(); itx++)
    {
        Player* plr = (*itx);
        if(!plr) continue;

        plr->m_bgScore.BonusHonor += amt;
        HonorHandler::AddHonorPointsToPlayer( plr, amt);
    }
}


bool CBattleground::HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* pSpell)
{
    /*if( pPlayer->m_stealth )
        pPlayer->RemoveAura( pPlayer->m_stealth );*/

    return false;
}

bool CBattlegroundManager::IsValidBG(uint32 BgType)
{
    if(BgType && (BgType < 12 || BgType == 31 || BgType == 32))
       return true;
    return false;
}

uint32 CBattlegroundManager::GenerateRandomBGType()
{
    switch(RandomUInt(6))
    {
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    case 4: return 7;
    case 5: return 9;
    case 6: return 30;
    }
    return 0;
}