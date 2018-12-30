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

#define TRIGGER_QUEST_EVENT(id, func)

class Quest;
class QuestMgr;

class SERVER_DECL QuestLogEntry
{
    friend class QuestMgr;
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
