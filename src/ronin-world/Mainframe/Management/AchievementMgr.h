
#pragma once

struct CriteriaData
{
    CriteriaData() : flag(0), criteriaCounter(0), failedTimedState(false) { timerData[0] = timerData[1] = UNIXTIME; }

    uint8 flag;
    time_t timerData[2];
    uint64 criteriaCounter;
    bool failedTimedState;

};

enum CriteriaCounterModifier : uint8
{
    CCM_HIGHEST,
    CCM_CURRENT,
    CCM_TOTAL
};

struct AchieveDataContainer
{
    AchieveDataContainer(uint64 guid) : playerguid(guid), _loading(true) {}

    bool _loading;
    WoWGuid playerguid;
    std::map<uint32, time_t> m_completedAchievements;
    std::map<uint32, CriteriaData*> m_criteriaProgress;
};

class SERVER_DECL AchievementMgr : public Singleton < AchievementMgr >
{
    typedef bool (AchievementMgr::*pAchievementHandler)(AchievementEntry *entry, Player *plr, uint32 mod, uint32 misc1, uint32 misc2);
    // Criteria storage
    typedef std::multimap<uint32, AchievementCriteriaEntry*> CriteriaStorage;
    typedef std::pair<CriteriaStorage::iterator, CriteriaStorage::iterator> CriteriaStorageBounds;
    // Achievement storage
    typedef std::multimap<uint32, AchievementEntry*> AchievementStorage;
    typedef std::pair<AchievementStorage::iterator, AchievementStorage::iterator> AchievementStorageBounds;

public:
    AchievementMgr();
    ~AchievementMgr();

    // DBC parsing for criteria and data
    void ParseAchievements();

    // Pre load allocation and post logout deallocation
    void AllocatePlayerData(WoWGuid guid);
    void CleanupPlayerData(WoWGuid guid);
    void PlayerFinishedLoading(Player *plr);

    // Player loading of achievement and criteria
    void LoadAchievementData(WoWGuid guid, PlayerInfo *info, QueryResult *result);
    void LoadCriteriaData(WoWGuid guid, QueryResult *result);

    // Player saving of achievement and criteria data
    void SaveAchievementData(WoWGuid guid, QueryBuffer *buff);
    void SaveCriteriaData(WoWGuid guid, QueryBuffer *buff);

    // Post loading packet builder
    void BuildAchievementData(WoWGuid guid, WorldPacket *packet, bool buildEmpty = false);

    // Check if player can earn achievement
    bool IsValidAchievement(Player *plr, AchievementEntry *entry);

    // Trigger criteria update and achievement earning
    void UpdateCriteriaValue(Player *plr, uint32 criteriaType, uint32 mod, uint32 misc1 = 0, uint32 misc2 = 0, bool onLoad = false);

    // Achievement earn function for scripts
    void EarnAchievement(Player *plr, uint32 achievementId);

    // Remove achievements from players
    void RemoveAchievement(Player *plr, uint32 achievementId);

protected:
    // Check if criteria matches the info supplied
    bool _ValidateCriteriaRequirements(Player *plr, AchievementCriteriaEntry *entry, CriteriaCounterModifier &modType, uint32 &mod, uint32 &maxCounter, uint32 misc1, uint32 misc2);
    bool _FinishedCriteria(AchieveDataContainer *container, AchievementCriteriaEntry *criteria, AchievementEntry *achievement);
    bool _CheckCriteriaForAchievement(AchievementCriteriaEntry *criteria, AchievementEntry *achievement);
    bool _CheckAchievementRequirements(AchieveDataContainer *container, AchievementEntry *achievement);

private:
    Mutex achieveDataLock;
    std::map<WoWGuid, AchieveDataContainer*> m_playerAchieveData;

    std::set<uint32> m_realmFirstAchievements, m_realmFirstCompleted;

    AchievementStorage m_achievementsByReferrence;
    CriteriaStorage m_criteriaByType, m_criteriaByAchievement;
};

#define AchieveMgr AchievementMgr::getSingleton()
