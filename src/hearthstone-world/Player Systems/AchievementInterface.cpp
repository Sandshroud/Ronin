/***
 * Demonstrike Core
 */

#include "StdAfx.h"

AchievementInterface::AchievementInterface(Player* plr)
{
    m_player = plr;
    m_achievementInspectPacket = NULL;
}

AchievementInterface::~AchievementInterface()
{
    m_player = NULLPLR;

    if( m_achivementDataMap.size() > 0 )
    {
        std::map<uint32,AchievementData*>::iterator itr = m_achivementDataMap.begin();
        for(; itr != m_achivementDataMap.end(); itr++)
            delete itr->second;
    }

    if( m_achievementInspectPacket )
        delete m_achievementInspectPacket;
}

void AchievementInterface::LoadFromDB( QueryResult * pResult )
{
    // don't allow GMs to complete achievements
    if( sWorld.m_blockgmachievements && m_player->GetSession()->HasGMPermissions() )
    {
        CharacterDatabase.Execute("DELETE FROM achievements WHERE player = %u;", m_player->GetGUID());
        return;
    }

    if( !pResult ) // We have no achievements yet. :)
        return;

    do
    {
        Field * fields = pResult->Fetch();
        uint32 achievementid = fields[1].GetUInt32();
        bool completed = (fields[3].GetUInt32() > 0);

        AchievementEntry * ae = dbcAchievement.LookupEntry( achievementid );
        if(ae != NULL)
        {
            AchievementData * ad = new AchievementData;
            memset(ad, 0, sizeof(AchievementData));
            std::string criteriaprogress = fields[2].GetString();

            ad->id = achievementid;
            ad->num_criterias = ae->AssociatedCriteriaCount;
            ad->completed = completed;
            ad->date = fields[3].GetUInt32();
            ad->groupid = fields[4].GetUInt64();

            vector<string> Delim = StrSplit( criteriaprogress, "," );
            for( uint32 i = 0; !completed && i < ae->AssociatedCriteriaCount; i++)
            {
                if( i >= Delim.size() )
                    continue;

                string posValue = Delim[i];
                if( !posValue.size() )
                    continue;

                uint32 r = atoi(posValue.c_str());
                ad->counter[i] = r;

                //printf("Loaded achievement: %u, %s\n", ae->ID, ad->completed ? "completed" : "incomplete" );
            }

            m_achivementDataMap.insert( make_pair( achievementid, ad) );
        }
    } while ( pResult->NextRow() );
}

void AchievementInterface::SaveToDB(QueryBuffer * buffer)
{
    // don't allow GMs to save achievements
    if( sWorld.m_blockgmachievements && m_player->GetSession()->HasGMPermissions() )
        return;

    uint32 count = 0;
    std::stringstream ss;
    if(m_achivementDataMap.size())
    {
        map<uint32,AchievementData*>::iterator itr = m_achivementDataMap.begin();
        ss << "REPLACE INTO achievements (player,achievementid,progress,completed,groupid) VALUES ";
        bool first = true;
        for(; itr != m_achivementDataMap.end(); itr++)
        {
            AchievementData* ad = itr->second;
            if( !ad->m_isDirty )
                continue;

            if(first)
                first = false;
            else
                ss << ",";

            ss << "(" << m_player->GetLowGUID() << ",";
            ss << ad->id << ",";
            ss << "'";
            for(uint32 i = 0; i < ad->num_criterias; i++)
                ss << ad->counter[i] << ",";

            ss << "',";
            ss << ad->date << ",";
            ss << ad->groupid << ")";
            ad->m_isDirty = false;
            count++;
        }
        if(count)
            ss << ";";
    }
    else // If we have no achievements, delete all of our DB data.
    {
        count = 1; // Bwahahahahaha...
        ss << "DELETE FROM achievements WHERE player = '" << m_player->GetLowGUID() << "';";
    }

    if(count)
    {
        if(buffer == NULL)
            CharacterDatabase.Execute(ss.str().c_str());
        else // Execute or add our bulk inserts
            buffer->AddQuery(ss.str().c_str());
    }
}

WorldPacket* AchievementInterface::BuildAchievementData(bool forInspect)
{
    if(forInspect && m_achievementInspectPacket)
        return m_achievementInspectPacket;

    WorldPacket * data = new WorldPacket(forInspect ? SMSG_RESPOND_INSPECT_ACHIEVEMENTS : SMSG_ALL_ACHIEVEMENT_DATA, 400);
    *data << uint32(0);
    *data << m_player->GetNewGUID();
    *data << uint32(0);
    if(forInspect)
        m_achievementInspectPacket = data;
    return data;
}

void AchievementInterface::GiveRewardsForAchievement(AchievementEntry * ae)
{
    AchievementReward * ar = AchievementRewardStorage.LookupEntry( ae->ID );
    if(!ar)
        return;

    // Define: Alliance Title
    if(m_player->GetTeam() == ALLIANCE)
        if( ar->AllianceTitle )
            m_player->SetKnownTitle(ar->AllianceTitle, true);

    // Define: Horde Title
    if(m_player->GetTeam() == HORDE)
        if( ar->HordeTitle )
            m_player->SetKnownTitle(ar->HordeTitle, true);

    // Reward: Item
    if( ar->ItemID && !ar->MailMessage)
    {
        m_player->GetGUID();
        Item* pItem = objmgr.CreateItem(ar->ItemID, m_player);
        if( !m_player->GetItemInterface()->AddItemToFreeSlot(pItem) )
        {
            // Inventory full? Send it by mail.
            pItem->SaveToDB(-1, -1, true, NULL);
            m_player->GetSession()->SendNotification("No free slots were found in your inventory, item has been mailed.");
            sMailSystem.DeliverMessage(MAILTYPE_NORMAL, m_player->GetGUID(), m_player->GetGUID(), "Achievement Reward", "Here is your reward.", 0, 0, pItem->GetGUID(), 1, true);
            pItem->DeleteMe();
            pItem = NULL;
        }
    }
    else
    {
        uint32 Sender = ar->SenderEntry;
        char* messageheader = ar->MessageHeader;
        char* messagebody = ar->MessageBody;
        // Crow: TODO: Localization
        if(ar->MailMessage || messagebody)
        {
            if(ar->ItemID)
            {
                Item* pItem = objmgr.CreateItem(ar->ItemID, m_player);
                if(pItem != NULL)
                {
                    pItem->SaveToDB(-1, -1, true, NULL);
                    sMailSystem.DeliverMessage(MAILTYPE_CREATURE, Sender, m_player->GetGUID(),
                        messageheader, messagebody, 0, 0, pItem->GetGUID(), 1, true);
                    pItem->DeleteMe();
                    pItem = NULL;
                }
                else
                {
                    printf("MISSING ITEM PROTO FOR AN ACHIEVEMENT REWARD!!!(%u)\n", ar->ItemID);
                }
            }
            else
            {
                sMailSystem.DeliverMessage(MAILTYPE_CREATURE, Sender, m_player->GetGUID(),
                    messageheader, messagebody, 0, 0, 0, 1, true);
            }
        }
    }
}

void AchievementInterface::EventAchievementEarned(AchievementData * pData)
{
    pData->completed = true;
    pData->date = (uint32)time(NULL);
    AchievementEntry * ae = dbcAchievement.LookupEntry(pData->id);
    if(ae == NULL)
        return;

    GiveRewardsForAchievement(ae);

    if(ae->Previous_achievement)
        if(!HasAchievement(ae->Previous_achievement))
            EventAchievementEarned(GetAchievementDataByAchievementID(ae->Previous_achievement));

    WorldPacket * data = BuildAchievementEarned(pData);

    if( m_player->IsInWorld() )
        m_player->GetSession()->SendPacket( data );
    else
        m_player->CopyAndSendDelayedPacket( data );

    delete data;

    HandleAchievementCriteriaRequiresAchievement(pData->id);

    // Realm First Achievements
    if( string(ae->name).find("Realm First") != string::npos  ) // flags are wrong lol
    {
        // Send to my team
        WorldPacket data(SMSG_SERVER_FIRST_ACHIEVEMENT, 60);
        data << m_player->GetName();
        data << m_player->GetGUID();
        data << ae->ID;
        data << uint32(1);
        sWorld.SendFactionMessage(&data, m_player->GetTeam());

        // Send to the other team (no clickable link)
        WorldPacket data2(SMSG_SERVER_FIRST_ACHIEVEMENT, 60);
        data2 << m_player->GetName();
        data2 << m_player->GetGUID();
        data2 << ae->ID;
        data2 << uint32(0);
        sWorld.SendFactionMessage(&data2, m_player->GetTeam() ? 0 : 1);
    }
}

void AchievementInterface::BuildAllAchievementDataPacket(WorldPacket *data)
{
    // Achievement count
    *data << uint32(0);
    // Criteria count
    *data << uint32(0);

//  for (CompletedAchievementMap::const_iterator iter = m_achivementDataMap.begin(); iter != m_achivementDataMap.end(); ++iter)
//      *data << uint32();
//  for (CompletedAchievementMap::const_iterator iter = m_achivementDataMap.begin(); iter != m_achivementDataMap.end(); ++iter)
//      *data << uint32(secsToTimeBitFields(iter->second.date));

    // Criteria mask
    *data << uint8(0);
}

WorldPacket* AchievementInterface::BuildAchievementEarned(AchievementData * pData)
{
    pData->m_isDirty = true;
    WorldPacket * data = new WorldPacket(SMSG_ACHIEVEMENT_EARNED, 40);
    *data << m_player->GetNewGUID();
    *data << pData->id;
    *data << uint32( unixTimeToTimeBitfields(time(NULL)) );
    *data << uint32(0);

    if( m_achievementInspectPacket )
    {
        delete m_achievementInspectPacket;
        m_achievementInspectPacket = NULL;
    }

    return data;
}

AchievementData* AchievementInterface::CreateAchievementDataEntryForAchievement(AchievementEntry * ae)
{
    AchievementData* ad = new AchievementData;
    memset(ad, 0, sizeof(AchievementData));
    ad->id = ae->ID;
    ad->num_criterias = ae->AssociatedCriteriaCount;
    m_achivementDataMap.insert( make_pair( ad->id, ad ) );
    return ad;
}

bool AchievementInterface::CanCompleteAchievement(AchievementData * ad)
{
    if(!m_player) // o.O bastard.
        return false;

    // don't allow GMs to complete achievements
    if( sWorld.m_blockgmachievements && m_player->GetSession()->HasGMPermissions() )
        return false;

    if( ad->completed )
        return false;

    bool hasCompleted = false;
    AchievementEntry * ach = dbcAchievement.LookupEntry(ad->id);

    if( ach->categoryId == 1 || ach->flags & ACHIEVEMENT_FLAG_COUNTER ) // We cannot complete statistics
        return false;

    /* Crow: Needs work :|
    This needs to be in a check that allows updates, but does not allow the
    player to actually get the achievement, also need to add it so that based on how
    many we need and how many we have, but that seems to be a small amount of achievements,
    maybe just do those by hand.
    */
    for(uint32 i = 0; i < ad->num_criterias; i++)
        if(ad->counter[i] == 0)
            return false;

    bool failedOne = false;
    for(uint32 i = 0; i < ad->num_criterias; i++)
    {
        bool thisFail = false;
        AchievementCriteriaEntry * ace = dbcAchievementCriteria.LookupEntry(ach->AssociatedCriteria[i]);
        if(ace == NULL)
            continue;

        uint32 ReqCount = ace->raw.field4 ? ace->raw.field4 : 1;

        if( ace->timedData[2] && ace->timedData[2] < ad->completionTimeLast )
            thisFail = true;

        if( ad->counter[i] < ReqCount )
            thisFail = true;

        if( ach->factionFlag == ACHIEVEMENT_FACTION_ALLIANCE && m_player->GetTeam() == 1 )
            thisFail = true;

        if( ach->factionFlag == ACHIEVEMENT_FACTION_HORDE && m_player->GetTeam() == 0 )
            thisFail = true;

        if( thisFail && ace->completionFlag & ACHIEVEMENT_CRITERIA_COMPLETE_ONE_FLAG )
            failedOne = true;
        else if(thisFail)
            return false;

        if( !thisFail && ace->completionFlag & ACHIEVEMENT_CRITERIA_COMPLETE_ONE_FLAG )
            hasCompleted = true;
    }

    if( failedOne && !hasCompleted )
        return false;

    return true;
}

bool AchievementInterface::HandleBeforeChecks(AchievementData * ad)
{
    AchievementEntry * ach = dbcAchievement.LookupEntry(ad->id);
    if(ach == NULL) // Doh?
        return false;

    // Difficulty checks
    if(string(ach->description).find("25-player heroic mode") != string::npos)
        if(m_player->iRaidType < MODE_25PLAYER_HEROIC)
            return false;
    if(string(ach->description).find("10-player heroic mode") != string::npos)
        if(m_player->iRaidType < MODE_10PLAYER_HEROIC)
            return false;
    if(string(ach->description).find("25-player mode") != string::npos)
        if(m_player->iRaidType < MODE_25PLAYER_NORMAL)
            return false;
    if((string(ach->description).find("Heroic Difficulty") != string::npos) || ach->ID == 4526)
        if(m_player->iInstanceType < MODE_5PLAYER_HEROIC)
            return false;
    if(sWorld.m_blockgmachievements && m_player->GetSession()->HasGMPermissions())
        return false;
    if(IsHardCoded(ach))
        return false;
    return true;
}

bool AchievementInterface::IsHardCoded(AchievementEntry * ae)
{
    if((ae->flags & ACHIEVEMENT_FLAG_REALM_FIRST_OBTAIN) || (ae->flags & ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
        return true;

    switch(ae->ID)
    {
    case 16: // Skill shit, it's broken, so disable it.
    case 2398: // 4th Anniversary
    case 2716: // Dual Talent Specialization
    case 2772: // Tilted!
    case 4400: // 5th Anniversary
    case 4402: // More Dots! (10 player)
    case 4403: // Many Whelps! Handle It! (10 player)
    case 4404: // She Deep Breaths More (10 player)
    case 4405: // More Dots! (25 player)
    case 4406: // Many Whelps! Handle It! (25 player)
    case 4407: // She Deep Breaths More (25 player)
    case 5512: // 6th Anniversary
        return true;
    }return false;
}

bool AchievementInterface::HasAchievement(uint32 ID)
{
    AchievementData* acc = GetAchievementDataByAchievementID(ID);
    if(acc)
        return acc->completed;

    return false;
}

AchievementData* AchievementInterface::GetAchievementDataByAchievementID(uint32 ID)
{
    map<uint32,AchievementData*>::iterator itr = m_achivementDataMap.find( ID );
    if( itr != m_achivementDataMap.end() )
        return itr->second;
    else
    {
        AchievementEntry * ae = dbcAchievement.LookupEntry(ID);
        if(ae == NULL)
            return NULL;
        return CreateAchievementDataEntryForAchievement(ae);
    }
}

void AchievementInterface::SendCriteriaUpdate(AchievementData * ad, uint32 idx)
{
    ad->m_isDirty = true;
    ad->date = (uint32)time(NULL);
    ad->groupid = m_player->GetGroupID();
    AchievementEntry * ae = dbcAchievement.LookupEntry(ad->id);
    if(ae == NULL)
        return;

    WorldPacket data(SMSG_CRITERIA_UPDATE, 50);
    data << uint32(ae->AssociatedCriteria[idx]);
    FastGUIDPack( data, (uint64)ad->counter[idx] );
    data << m_player->GetNewGUID();
    data << uint32(0);
    data << uint32(unixTimeToTimeBitfields(time(NULL)));
    data << uint32(0);
    data << uint32(0);

    if( !m_player->IsInWorld() )
        m_player->CopyAndSendDelayedPacket(&data);
    else
        m_player->GetSession()->SendPacket(&data);

    if( m_achievementInspectPacket )
    {
        delete m_achievementInspectPacket;
        m_achievementInspectPacket = NULL;
    }
}

void AchievementInterface::HandleAchievementCriteriaConditionDeath()
{
    // We died, so reset all our achievements that have ACHIEVEMENT_CRITERIA_CONDITION_NO_DEATH
    if( m_achivementDataMap.empty() )
        return;

    map<uint32,AchievementData*>::iterator itr = m_achivementDataMap.begin();
    for(; itr != m_achivementDataMap.end(); itr++)
    {
        AchievementData * ad = itr->second;
        if(ad->completed)
            continue;
        AchievementEntry * ae = dbcAchievement.LookupEntry( ad->id );
        if(ae == NULL)
        {
            m_achivementDataMap.erase(itr);
            continue;
        }

        for(uint32 i = 0; i < ad->num_criterias; i++)
        {
            uint32 CriteriaID = ae->AssociatedCriteria[i];
            AchievementCriteriaEntry * ace = dbcAchievementCriteria.LookupEntry( CriteriaID );
            if( ad->counter[i] && ace->raw.additionalRequirement1_type & ACHIEVEMENT_CRITERIA_CONDITION_NO_DEATH )
            {
                ad->counter[i] = 0;
                SendCriteriaUpdate(ad, i); break;
            }
        }
    }
}

void AchievementInterface::HandleAchievementCriteriaKillCreature(uint32 killedMonster)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqKill = ace->kill_creature.creatureID;
        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;

        // Wrong monster, continue on, kids.
        if( ReqKill != killedMonster )
            continue;
        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i);
                break;
            }
        }
        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaWinBattleground(uint32 bgMapId, uint32 scoreMargin, uint32 time, CBattleground* bg)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_WIN_BG );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqBGMap = ace->win_bg.bgMapID;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        // Wrong BG, continue on, kids.
        if( ReqBGMap != bgMapId )
            continue;
        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        ad->completionTimeLast = time;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                if( compareCriteria->raw.additionalRequirement1_type && scoreMargin < compareCriteria->raw.additionalRequirement1_type ) // BG Score Requirement.
                    continue;
                // AV stuff :P
                if( bg->GetType() == BATTLEGROUND_ALTERAC_VALLEY )
                {
                    AlteracValley* pAV(TO_ALTERACVALLEY(bg));
                    if( pAchievementEntry->ID == 225 ||  pAchievementEntry->ID == 1164) // AV: Everything Counts
                    {
                        continue; // We do not support mines yet in AV
                    }
                    if( pAchievementEntry->ID == 220 ) // AV: Stormpike Perfection
                    {
                        bool failure = false;
                        // We must control all Alliance nodes and Horde nodes (towers only)
                        for(uint32 i = 0; i < AV_NUM_CONTROL_POINTS; i++)
                        {
                            if( pAV->GetNode(i)->IsGraveyard() )
                                continue;
                            if( pAV->GetNode(i)->GetState() != AV_NODE_STATE_ALLIANCE_CONTROLLED )
                                failure = true;
                        }
                        if( failure ) continue;
                    }
                    if( pAchievementEntry->ID == 873 ) // AV: Frostwolf Perfection
                    {
                        bool failure = false;
                        // We must control all Alliance nodes and Horde nodes (towers only)
                        for(uint32 i = 0; i < AV_NUM_CONTROL_POINTS; i++)
                        {
                            if( pAV->GetNode(i)->IsGraveyard() )
                                continue;

                            if( pAV->GetNode(i)->GetState() != AV_NODE_STATE_HORDE_CONTROLLED )
                                failure = true;
                        }
                        if( failure ) continue;
                    }
                }
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }
        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaRequiresAchievement(uint32 achievementId)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqAchievement = ace->complete_achievement.linkedAchievement;
        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        if( ReqAchievement != achievementId )
            continue;
        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }
        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaLevelUp(uint32 level)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqLevel = ace->reach_level.level;
        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        if( level < ReqLevel )
            continue;
        // Realm first to 80 stuff has race and class requirements. Let the hacking begin.
        if( string(pAchievementEntry->name).find("Realm First!") != string::npos )
        {
            static const char* classNames[] = { "", "Warrior", "Paladin", "Hunter", "Rogue", "Priest", "Death Knight", "Shaman", "Mage", "Warlock", "", "Druid" };
            static const char* raceNames[] = { "", "Human", "Orc", "Dwarf", "Night Elf", "Forsaken", "Tauren", "Gnome", "Troll", "", "Blood Elf", "Draenei" };
            uint32 ReqClass = 0;
            uint32 ReqRace = 0;
            for(uint32 i = 0; i < 12; i++)
            {
                if(strlen(classNames[i]) > 0 && string(pAchievementEntry->name).find(classNames[i]) != string::npos )
                {
                    // We require this class
                    ReqClass = i;
                    break;
                }
            }
            for(uint32 i = 0; i < 12; i++)
            {
                if(strlen(raceNames[i]) > 0 && string(pAchievementEntry->name).find(raceNames[i]) != string::npos )
                {
                    // We require this race
                    ReqRace = i;
                    break;
                }
            }

            if( ReqClass && m_player->getClass() != ReqClass )
                continue;

            if( ReqRace && m_player->getRace() != ReqRace )
                continue;
        }

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = m_player->getLevel() > ReqLevel ? ReqLevel : m_player->getLevel();
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaOwnItem(uint32 itemId, uint32 stack)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqItemId = ace->own_item.itemID;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        if( itemId != ReqItemId )
            continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if( ad->completed )
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + stack;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }

    HandleAchievementCriteriaLootItem(itemId, stack);
}

void AchievementInterface::HandleAchievementCriteriaLootItem(uint32 itemId, uint32 stack)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqItemId = ace->loot_item.itemID;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        if( itemId != ReqItemId )
            continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + stack;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaQuestCount(uint32 questCount)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;

        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = questCount;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaHonorableKillClass(uint32 classId)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqClass = ace->hk_class.classID;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        if( ReqClass != classId )
            continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaHonorableKillRace(uint32 raceId)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_HK_RACE );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqRace = ace->hk_race.raceID;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        if( ReqRace != raceId )
            continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaTalentResetCostTotal(uint32 cost)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + cost;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaTalentResetCount()
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaBuyBankSlot(bool retroactive)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqSlots = ace->buy_bank_slot.numberOfSlots; // We don't actually use this. :P

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                if( retroactive )
                {
                    uint32 bytes = m_player->GetUInt32Value(PLAYER_BYTES_2);
                    uint32 slots = (uint8)(bytes >> 16);
                    ad->counter[i] = slots > ReqSlots ? ReqSlots : slots;
                }
                else
                    ad->counter[i] = ad->counter[i] + 1;

                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaFlightPathsTaken()
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaExploreArea(uint32 areaId, uint32 explorationFlags)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqFlags = ace->explore_area.areaReference;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        WorldMapOverlayEntry * wmoe = dbcWorldMapOverlay.LookupEntry( ReqFlags );
        if(!wmoe) continue;

        for(uint8 i = 0; i < 4; i++)
        {
            if(!wmoe->AreaTableID[i])
                continue;
            AreaTableEntry * at = dbcAreaTable.LookupEntry(wmoe->AreaTableID[i]);
            if(!at || !(ReqFlags & at->explorationFlag) )
                continue;

            uint32 offset = at->explorationFlag / 32;
            offset += PLAYER_EXPLORED_ZONES_1;

            uint32 val = (uint32)(1 << (at->explorationFlag % 32));
            uint32 currFields = m_player->GetUInt32Value(offset);

            // Not explored /sadface
            if( !(currFields & val) )
                continue;

            AchievementCriteriaEntry * compareCriteria = NULL;
            AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
            if(ad == NULL)
                continue;
            if(ad->completed)
                continue;
            if(!HandleBeforeChecks(ad))
                continue;

            // Figure out our associative ID.
            for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
            {
                compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
                if( compareCriteria == ace )
                {
                    ad->counter[i] = 1;
                    SendCriteriaUpdate(ad, i); break;
                }
            }

            if( CanCompleteAchievement(ad) )
                EventAchievementEarned(ad);
        }
    }
}

void AchievementInterface::HandleAchievementCriteriaHonorableKill()
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqKills = ace->honorable_kill.killCount;

        if( m_player->m_killsLifetime < ReqKills )
            continue;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

#define SCRIPTOK_FALSE { scriptOk = false; break; }
void AchievementInterface::HandleAchievementCriteriaDoEmote(uint32 emoteId, Unit* pTarget)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqEmote = ace->do_emote.emoteID;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        if(ReqEmote != emoteId)
            continue; // Wrong emote, newb!

        // Target information is not stored, so we'll have to do this one by one...
        // --unless the target's name is the description of the criteria! Bahahaha
        bool scriptOk = false;
        if( pTarget && pTarget->IsCreature() )
        {
            Creature* pCreature = TO_CREATURE(pTarget);
            if( !(!ace->name || strlen(ace->name) == 0 || !pCreature->GetCreatureInfo() || stricmp(pCreature->GetCreatureInfo()->Name, ace->name) != 0) )
            {
                scriptOk = true;
            }
        }

        string name = string(pAchievementEntry->name);
        if( name.find("Total") != string::npos )
        {
            // It's a statistic, like: "Total Times /Lol'd"
            scriptOk = true;
        }

        // Script individual ones here...
        if( ace->ID == 2379 ) // Make Love, Not Warcraft
        {
            if( !pTarget || !pTarget->IsPlayer() || !pTarget->isDead() || !sFactionSystem.isHostile(pTarget, m_player) )
                SCRIPTOK_FALSE

            scriptOk = true;
        }
        else if( ace->ID == 6261 ) // Winter Veil: A Frosty Shake
        {
            if( m_player->GetZoneId() != 4395 ) // Not in Dalaran
                SCRIPTOK_FALSE

            if( !pTarget || !pTarget->HasAura(21848) ) // Not a Snowman
                SCRIPTOK_FALSE

            scriptOk = true;
        }

        if( !scriptOk ) continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaCompleteQuestsInZone(uint32 zoneId)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqZone = ace->complete_quests_in_zone.zoneID;

        if( ReqZone != zoneId )
            continue;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL) continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;

        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaReachSkillLevel(uint32 skillId, uint32 skillLevel)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqSkill = ace->reach_skill_level.skillID;

        if( ReqSkill != skillId )
            continue;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;

        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = skillLevel;
                SendCriteriaUpdate(ad, i);
                break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaWinDuel()
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        //uint32 ReqDuels = ace->win_duel.duelCount;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaLoseDuel()
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        //uint32 ReqDuels = ace->win_duel.duelCount;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i] = ad->counter[i] + 1;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaKilledByCreature(uint32 killedMonster)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE );
    if( itr == objmgr.m_achievementCriteriaMap.end() )
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 ReqCreature = ace->killed_by_creature.creatureEntry;

        if( ReqCreature != killedMonster )
            continue;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i]++;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaKilledByPlayer()
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i]++;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

void AchievementInterface::HandleAchievementCriteriaDeath()
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_DEATH );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i]++;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }

    HandleAchievementCriteriaDeathAtMap(m_player->GetMapId());
}

void AchievementInterface::HandleAchievementCriteriaDeathAtMap(uint32 mapId)
{
    AchievementCriteriaMap::iterator itr = objmgr.m_achievementCriteriaMap.find( ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP );
    if(itr == objmgr.m_achievementCriteriaMap.end())
        return;

    AchievementCriteriaSet * acs = itr->second;
    if( !acs ) // We have no achievements for this criteria :(
        return;

    AchievementCriteriaSet::iterator citr = acs->begin();
    for(; citr != acs->end(); ++citr)
    {
        AchievementCriteriaEntry * ace = (*citr);
        uint32 AchievementID = ace->referredAchievement;
        uint32 MapID = ace->death_at_map.mapID;

        if( mapId != MapID )
            continue;

        AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(AchievementID);
        if(pAchievementEntry == NULL)
            continue;
        AchievementCriteriaEntry * compareCriteria = NULL;
        AchievementData * ad = GetAchievementDataByAchievementID(AchievementID);
        if(ad == NULL)
            continue;
        if(ad->completed)
            continue;
        if(!HandleBeforeChecks(ad))
            continue;
        // Figure out our associative ID.
        for(uint32 i = 0; i < pAchievementEntry->AssociatedCriteriaCount; i++)
        {
            compareCriteria = dbcAchievementCriteria.LookupEntry( pAchievementEntry->AssociatedCriteria[i] );
            if( compareCriteria == ace )
            {
                ad->counter[i]++;
                SendCriteriaUpdate(ad, i); break;
            }
        }

        if( CanCompleteAchievement(ad) )
            EventAchievementEarned(ad);
    }
}

/*  Crow: Force earning achievements for scripts and possibly commands.
    For this to work, removed before checks and cancomplete's
    I have kept GM checks though, bastards o.o */
void AchievementInterface::ForceEarnedAchievement(uint32 achievementId)
{
    if(sWorld.m_blockgmachievements && m_player->GetSession()->HasGMPermissions())
        return;
    AchievementEntry * pAchievementEntry = dbcAchievement.LookupEntry(achievementId);
    if(pAchievementEntry == NULL)
        return;
    AchievementData * ad = GetAchievementDataByAchievementID(achievementId);
    if(ad == NULL)
        return;
    if(ad->completed)
        return;

    EventAchievementEarned(ad);
}
