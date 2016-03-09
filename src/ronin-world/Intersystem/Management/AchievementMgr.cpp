
#include "stdafx.h"

initialiseSingleton( AchievementMgr );

AchievementMgr::AchievementMgr()
{

}

AchievementMgr::~AchievementMgr()
{

}

void AchievementMgr::ParseAchievements()
{

    for(uint32 i = 0; i < dbcAchievementCriteria.GetNumRows(); i++)
    {
        AchievementCriteriaEntry *entry = dbcAchievementCriteria.LookupRow(i);
        if(entry == NULL)
            continue;

        m_criteriaByType.insert(std::make_pair(entry->requiredType, entry));
        m_criteriaByAchievement.insert(std::make_pair(entry->referredAchievement, entry));
    }
}

void AchievementMgr::AllocatePlayerData(WoWGuid guid)
{
    if(m_playerAchieveData.find(guid) != m_playerAchieveData.end())
        return;
    m_playerAchieveData.insert(std::make_pair(guid, new AchieveDataContainer()));
}

void AchievementMgr::CleanupPlayerData(WoWGuid guid)
{
    if(m_playerAchieveData.find(guid) == m_playerAchieveData.end())
        return;
    AchieveDataContainer *container = m_playerAchieveData.at(guid);
    m_playerAchieveData.erase(guid);
    delete container;
}

void AchievementMgr::LoadAchievementData(WoWGuid guid, PlayerInfo *info, QueryResult *result)
{
    uint32 loadedPoints = 0;
    if(result)
    {
        AchieveDataContainer *container = m_playerAchieveData.at(guid);
        do
        {
            Field *fields = result->Fetch();
            uint32 achievement = fields[1].GetUInt32();
            AchievementEntry *entry = dbcAchievement.LookupEntry(achievement);
            if(entry == NULL)
                continue;

            time_t finishTime = fields[2].GetUInt64();
            container->m_completedAchievements.insert(std::make_pair(achievement, finishTime));
            loadedPoints += entry->points;
        }while(result->NextRow());
    }

    if(loadedPoints != info->achievementPoints)
        info->achievementPoints = loadedPoints;
}

void AchievementMgr::LoadCriteriaData(WoWGuid guid, QueryResult *result)
{
    if(result == NULL)
        return;

    AchieveDataContainer *container = m_playerAchieveData.at(guid);
    do
    {
        Field *fields = result->Fetch();
        uint32 criteria = fields[1].GetUInt32();
        AchievementCriteriaEntry *entry = dbcAchievementCriteria.LookupEntry(criteria);
        if(entry == NULL)
            continue;

        CriteriaData *data = new CriteriaData();
        data->flag = 0, data->dirty = data->failedTimedState = false;
        data->criteriaCounter = fields[2].GetUInt64();
        data->timerData[0] = fields[3].GetUInt64();
        data->timerData[1] = fields[4].GetUInt64();
        container->m_criteriaProgress.insert(std::make_pair(criteria, data));
    }while(result->NextRow());
}

void AchievementMgr::SaveAchievementData(WoWGuid guid, QueryBuffer *buff)
{
    // Clear out the table
    if(buff) buff->AddQuery("DELETE FROM character_achievements WHERE guid = %u;", guid.getLow());
    else CharacterDatabase.Execute("DELETE FROM character_achievements WHERE guid = %u;", guid.getLow());

    // Append anything we need to save
    AchieveDataContainer *container = m_playerAchieveData.at(guid);
    if(container == NULL)
        return;

    std::stringstream ss;
    for(auto it = container->m_completedAchievements.begin(); it != container->m_completedAchievements.end(); it++)
    {
        if(ss.str().length())
            ss << ", ";

        ss << "(" << guid.getLow()
        << ", " << uint32(it->first)
        << ", " << uint64(it->second);
        ss << ")";
    }

    if(ss.str().length())
    {
        if(buff) buff->AddQuery("REPLACE INTO character_achievements VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_achievements VALUES %s;", ss.str().c_str());
    }
}

void AchievementMgr::SaveCriteriaData(WoWGuid guid, QueryBuffer *buff)
{
    // Clear out the table
    if(buff) buff->AddQuery("DELETE FROM character_criteria_data WHERE guid = %u;", guid.getLow());
    else CharacterDatabase.Execute("DELETE FROM character_criteria_data WHERE guid = %u;", guid.getLow());

    // Append anything we need to save
    AchieveDataContainer *container = m_playerAchieveData.at(guid);
    if(container == NULL)
        return;

    std::stringstream ss;
    for(auto it = container->m_criteriaProgress.begin(); it != container->m_criteriaProgress.end(); it++)
    {
        if(ss.str().length())
            ss << ", ";

        ss << "(" << guid.getLow()
        << ", " << uint32(it->first)
        << ", " << uint64(it->second->criteriaCounter)
        << ", " << uint64(it->second->timerData[0])
        << ", " << uint64(it->second->timerData[1]);
        ss << ")";
    }

    if(ss.str().length())
    {
        if(buff) buff->AddQuery("REPLACE INTO character_criteria_data VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_criteria_data VALUES %s;", ss.str().c_str());
    }
}

void AchievementMgr::BuildAchievementData(WoWGuid guid, WorldPacket *data)
{
    AchieveDataContainer *container = m_playerAchieveData.at(guid);
    ByteBuffer criteriaData;
    data->WriteBits(container->m_criteriaProgress.size(), 21);
    for(auto it = container->m_criteriaProgress.begin(); it != container->m_criteriaProgress.end(); it++)
    {
        WoWGuid counter(it->second->criteriaCounter);
        // Append our bit data
        data->WriteBit(guid[4]);
        data->WriteBit(counter[3]);
        data->WriteBit(guid[5]);
        data->WriteGuidBitString(2, counter, 0, 6);
        data->WriteGuidBitString(2, guid, 3, 0);
        data->WriteBit(counter[4]);
        data->WriteBit(guid[2]);
        data->WriteBit(counter[7]);
        data->WriteBit(guid[7]);
        data->WriteBits(it->second->flag, 2);
        data->WriteBit(guid[6]);
        data->WriteGuidBitString(3, counter, 2, 1, 5);
        data->WriteBit(guid[1]);

        // Append byte data
        criteriaData.WriteByteSeq(guid[3]);
        criteriaData.WriteSeqByteString(2, counter, 5, 6);
        criteriaData.WriteSeqByteString(2, guid, 4, 6);
        criteriaData.WriteByteSeq(counter[2]);

        criteriaData << uint32(UNIXTIME - it->second->timerData[1]); // Timer 2
        criteriaData.WriteByteSeq(guid[2]);
        criteriaData << uint32(it->first);

        criteriaData.WriteByteSeq(guid[5]);
        criteriaData.WriteSeqByteString(4, counter, 0, 3, 1, 4);
        criteriaData.WriteSeqByteString(2, guid, 0, 7);
        criteriaData.WriteByteSeq(counter[7]);

        criteriaData << uint32(UNIXTIME - it->second->timerData[0]); // Timer 1
        criteriaData << RONIN_UTIL::secsToTimeBitFields(UNIXTIME);

        criteriaData.WriteByteSeq(guid[1]);
    }

    data->WriteBits(container->m_completedAchievements.size(), 23);
    data->FlushBits();
    // Append our criteria bytes
    data->append(criteriaData.contents(), criteriaData.size());
    // Append achievements earned and completion dates
    for(auto it = container->m_completedAchievements.begin(); it != container->m_completedAchievements.end(); it++)
    {
        *data << uint32(it->first);
        *data << RONIN_UTIL::secsToTimeBitFields(it->second);
    }
}

void AchievementMgr::EarnAchievement(Player *plr, uint32 achievementId)
{
    AchievementEntry *entry = dbcAchievement.LookupEntry(achievementId);
    if(entry == NULL || (entry->factionFlag != -1 && entry->factionFlag == plr->GetTeam()))
        return;
    if(m_playerAchieveData.find(plr->GetGUID()) == m_playerAchieveData.end())
        return;
    AchieveDataContainer *container = m_playerAchieveData.at(plr->GetGUID());
    std::map<uint32, time_t> &achievements = container->m_completedAchievements;
    if(achievements.find(achievementId) != achievements.end())
        return;
    achievements.insert(std::make_pair(achievementId, time_t(UNIXTIME)));

    // Send our achievement earned packet
    WorldPacket data(SMSG_ACHIEVEMENT_EARNED, 8 + 4 + 4 + 4);
    data << plr->GetGUID().asPacked();
    data << uint32(achievementId);
    data << RONIN_UTIL::secsToTimeBitFields(UNIXTIME);
    data << uint32(0);
    plr->SendMessageToSet(&data, true, true, 50.f);
    // Increment cached achievement points
    plr->m_playerInfo->achievementPoints += entry->points;
}
