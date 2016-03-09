
#pragma once

struct CriteriaData
{
    uint8 flag;
    time_t timerData[2];
    uint64 criteriaCounter;
    bool dirty, failedTimedState;
};

struct AchieveDataContainer
{
    WoWGuid playerguid;
    std::map<uint32, time_t> m_completedAchievements;
    std::map<uint32, CriteriaData*> m_criteriaProgress;
};

class SERVER_DECL AchievementMgr : public Singleton < AchievementMgr >
{
public:
    AchievementMgr();
    ~AchievementMgr();

    // DBC parsing for criteria and data
    void ParseAchievements();

    // Pre load allocation and post logout deallocation
    void AllocatePlayerData(WoWGuid guid);
    void CleanupPlayerData(WoWGuid guid);

    // Player loading of achievement and criteria
    void LoadAchievementData(WoWGuid guid, PlayerInfo *info, QueryResult *result);
    void LoadCriteriaData(WoWGuid guid, QueryResult *result);

    // Player saving of achievement and criteria data
    void SaveAchievementData(WoWGuid guid, QueryBuffer *buff);
    void SaveCriteriaData(WoWGuid guid, QueryBuffer *buff);

    // Post loading packet builder
    void BuildAchievementData(WoWGuid guid, WorldPacket *packet);

    // Achievement earn function for scripts
    void EarnAchievement(Player *plr, uint32 achievementId);

private:
    Mutex achieveDataLock;
    std::map<WoWGuid, AchieveDataContainer*> m_playerAchieveData;
    std::multimap<uint32, AchievementCriteriaEntry*> m_criteriaByType, m_criteriaByAchievement;
};

#define AchieveMgr AchievementMgr::getSingleton()
