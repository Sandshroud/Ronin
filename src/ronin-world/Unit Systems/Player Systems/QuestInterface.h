/***
 * Demonstrike Core
 */

#pragma once

#define TRIGGER_QUEST_EVENT(id, func)

class SERVER_DECL QuestLogEntry
{
public:
    QuestLogEntry();
    ~QuestLogEntry();

    RONIN_INLINE Quest* GetQuest() { return m_quest; };
    void Init(Quest* quest, Player* plr, uint32 slot);
    void Load(Field *fields);

    bool CanBeFinished();
    void SubtractTime(uint32 value);
    void UpdatePlayerFields();

    void SetAreaTrigger(uint8 i);
    bool HasAreaTrigger(uint8 i) { return (m_areaTriggerFlags & (1<<i)); };
    void SetObjectiveCount(uint8 i, uint16 count);
    void SetPlayerSlainCount(uint32 count);

    bool IsUnitAffected(WoWGuid guid);
    void AddAffectedUnit(WoWGuid guid);
    void ClearAffectedUnits();

    RONIN_INLINE bool IsCastQuest() { return m_isCastQuest; }

    void SetSlot(int32 i);
    void Finish();
    uint32 GetSlot() { return m_slot; };

    void SendQuestComplete();
    void SendUpdateAddKill(uint8 i);
    RONIN_INLINE uint16 GetObjectiveCount(uint8 i) { return m_objectiveCount[i]; }
    RONIN_INLINE uint8 GetExplorationFlag() { return m_areaTriggerFlags; }
    RONIN_INLINE uint32 GetPlayerSlainCount() { return m_players_slain; }
    uint32 GetRequiredSpell();

    static uint32 GetBaseField(uint32 slot) { return PLAYER_QUEST_LOG + (slot * 5); }

    void SetQuestStatus(uint8 status) { m_questStatus = status; }

    RONIN_INLINE time_t getExpirationTime() { return m_expirationTime; }
    RONIN_INLINE bool isExpired() { return m_expirationTime <= UNIXTIME; }
private:
    int32 m_slot;
    Quest *m_quest;
    Player* m_Player;

    uint16 m_objectiveCount[4];
    uint8 m_areaTriggerFlags;
    uint32 m_players_slain;

    std::set<WoWGuid> m_affected_units;
    bool m_isExpired, m_isCastQuest;

    uint8 m_questStatus;
    time_t m_expirationTime;
};
