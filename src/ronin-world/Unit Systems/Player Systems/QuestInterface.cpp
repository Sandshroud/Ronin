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

//Packet Building
/////////////////

WorldPacket* WorldSession::BuildQuestQueryResponse(Quest *qst)
{
    // 2048 bytes should be more than enough. The fields cost ~200 bytes.
    // better to allocate more at startup than have to realloc the buffer later on.

    uint32 i = 0, total = 0;

    WorldPacket* data = new WorldPacket(SMSG_QUEST_QUERY_RESPONSE, 248);
    *data << uint32(qst->id);                           // Quest ID
    *data << uint32(qst->qst_accept_type);              // Quest Method
    *data << int32(qst->qst_max_level);                 // Quest level
    *data << uint32(qst->qst_min_level);                // minlevel !!!
    if(qst->qst_sort > 0)
        *data << int32(-(int32)qst->qst_sort);          // Negative if pointing to a sort.
    else *data << uint32(qst->qst_zone_id);             // Positive if pointing to a zone.

    *data << uint32(qst->qst_type);                     // Info ID / Type
    *data << uint32(qst->qst_suggested_players);        // suggested players
    *data << uint32(qst->required_rep_faction);         // Faction ID
    *data << uint32(qst->required_rep_value);           // Faction Amount
    *data << uint32(0);                                 // 3.3.3 // Opposite Faction ID
    *data << uint32(0);                                 // 3.3.3 // Opposite Faction Amount
    *data << uint32(qst->qst_next_quest_id);            // Next Quest ID
    *data << uint32(qst->reward_xp_index);              // 3.3.0

    if(qst->required_money)
        *data << int32(-qst->required_money) << uint32(0);
    else
    {
        *data << uint32(qst->reward_money*sWorld.getRate(RATE_QUESTMONEY));
        *data << uint32(qst->reward_maxlvlmoney*sWorld.getRate(RATE_QUESTMONEY));
    }
    *data << uint32(qst->reward_spell);                 // Spell added to spellbook upon completion
    *data << uint32(qst->reward_cast_on_player);        // Spell casted on player upon completion

    // Honor fields, now currency
    *data << uint32(0);
    *data << float(0);

    *data << uint32(qst->srcitem);                      // Item given at the start of a quest (srcitem)
    *data << uint32(qst->qst_flags);                    // Quest Flags
    *data << uint32(0);                                 // Quest target mark type

    *data << uint32(qst->reward_title);                 // Reward Title Id - Player is givn this title upon completion
    *data << uint32(qst->required_player_kills);        // Required Kill Player
    *data << uint32(qst->reward_talents);

    *data << uint32(0); // Arena currency?
    *data << uint32(0); // Skill Line Id
    *data << uint32(0); // Skill Line Points
    *data << uint32(0); // Reward rep mask
    *data << uint32(0); // Giver portrait
    *data << uint32(0); // Turn in portrait

    // (loop 4 times)
    for(i = 0; i < 4; i++)
    {
        *data << uint32(qst->reward_item[i]);           // Forced Reward Item [i]
        *data << uint32(qst->reward_itemcount[i]);      // Forced Reward Item Count [i]
    }

    // (loop 6 times)
    for(uint32 i = 0; i < 6; i++)
    {
        *data << uint32(qst->reward_choiceitem[i]);     // Choice Reward Item [i]
        *data << uint32(qst->reward_choiceitemcount[i]);// Choice Reward Item Count [i]
    }

    // Faction Reward Stuff.
    for(i = 0; i < 5; i++)
        *data << uint32(qst->reward_repfaction[i]);

    for(i = 0; i < 5; i++)
        *data << int32(qst->reward_repvalue[i]);

    for(i = 0; i < 5; i++)
        *data << int32(qst->reward_replimit[i]);
    // end

    *data << uint32(qst->required_point_mapid);     // Unknown
    *data << float(qst->required_point_x);          // Unknown
    *data << float(qst->required_point_y);          // Unknown
    *data << uint32(qst->required_point_radius);    // Unknown

    *data << qst->qst_title;            // Title / name of quest
    *data << qst->qst_objectivetext;    // Objectives / description
    *data << qst->qst_details;          // Details
    *data << qst->qst_endtext;          // Subdescription
    *data << qst->qst_finishedtext;     // Finished text

    // (loop 4 times)
    for(i = 0; i < 4; i++)
    {
        *data << uint32(qst->required_mob[i]);      // Kill mob entry ID [i]
        *data << uint32(qst->required_mobcount[i]); // Kill mob count [i]
        *data << int32(0) << int32(0);              // 3.0.2
    }

    for(i = 0; i < 6; i++)
    {
        *data << uint32(qst->required_item[i]);     // Collect item [i]
        *data << uint32(qst->required_itemcount[i]);// Collect item count [i]
    }

    *data << uint32(0);//qst->required_spell;

    for(i = 0; i < 4; i++)
        *data << qst->qst_objectivetexts[i];    // Objective 1 - Used as text if mob not set

    // Currency start
    for(i = 0; i < 4; i++)
        *data << uint32(0) << uint32(0);
    for(i = 0; i < 4; i++)
        *data << uint32(0) << uint32(0);
    // Currency end

    // Giver/Turnin strings
    *data << uint8(0) << uint8(0) << uint8(0) << uint8(0);

    // Accept/Turnin sounds
    *data << uint32(0) << uint32(0);
    return data;
}


/*****************
* QuestLogEntry *
*****************/
QuestLogEntry::QuestLogEntry()
{
    m_slot = -1;
    m_quest = NULL;
    m_players_slain = 0;
    m_questStatus = QUEST_STATUS__INCOMPLETE;
}

QuestLogEntry::~QuestLogEntry()
{
    m_Player = NULL;
    m_quest = NULL;
}

void QuestLogEntry::Init(Quest* quest, Player* plr, uint32 slot)
{
    ASSERT(quest);
    ASSERT(plr);

    m_quest = quest;
    m_Player = plr;
    m_slot = slot;

    m_isCastQuest = false;
    uint32 requiredSpell = GetRequiredSpell();
    if(requiredSpell)
    {
        m_isCastQuest = true;
        if (!plr->HasQuestSpell(requiredSpell))
            plr->quest_spells.insert(requiredSpell);
    }

    for (uint32 i = 0; i < 4; i++)
    {
        if (quest->required_mob[i] != 0)
        {
            if (!plr->HasQuestMob(quest->required_mob[i]))
                plr->quest_mobs.insert(quest->required_mob[i]);
        }
    }

    m_expirationTime = m_quest->required_timelimit ? (UNIXTIME+(m_quest->required_timelimit/1000)) : 0;

    // update slot
    plr->SetQuestLogSlot(this, slot);

    memset(m_objectiveCount, 0, sizeof(uint32)*4);
    m_areaTriggerFlags = 0;
    m_players_slain = 0;
}

void QuestLogEntry::Load(Field *fields)
{
    //guid,slot,questid,timestamp,count1,count2,count3,count4,playerkillcount,explorationflags
    m_expirationTime = fields[3].GetUInt64();
    m_objectiveCount[0] = fields[4].GetUInt16();
    m_objectiveCount[1] = fields[5].GetUInt16();
    m_objectiveCount[2] = fields[6].GetUInt16();
    m_objectiveCount[3] = fields[7].GetUInt16();
    m_areaTriggerFlags = fields[8].GetUInt8();
    m_players_slain = fields[9].GetUInt32();

    if(CanBeFinished())
        SetQuestStatus(QUEST_STATUS__COMPLETE);
}

void QuestLogEntry::AddAffectedUnit(WoWGuid guid)
{
    if(m_affected_units.find(guid) == m_affected_units.end())
        m_affected_units.insert(guid);
}

bool QuestLogEntry::IsUnitAffected(WoWGuid guid)
{
    if (m_affected_units.find(guid) != m_affected_units.end())
        return true;
    return false;
}

void QuestLogEntry::ClearAffectedUnits()
{
    m_affected_units.clear();
}

bool QuestLogEntry::CanBeFinished()
{
    return QuestMgr::PlayerCanComplete(m_Player, m_quest, this);
}

void QuestLogEntry::SetPlayerSlainCount(uint32 count)
{
    m_players_slain = count;
}

void QuestLogEntry::SetAreaTrigger(uint8 i)
{
    ASSERT(i<8);
    m_areaTriggerFlags |= 1<<i;
}

void QuestLogEntry::SetObjectiveCount(uint8 i, uint16 count)
{
    ASSERT(i<4);
    m_objectiveCount[i] = count;
}

void QuestLogEntry::SetSlot(int32 i)
{
    ASSERT(i!=-1);
    m_slot = i;
}

void QuestLogEntry::Finish()
{
    uint32 base = GetBaseField(m_slot);
    m_Player->SetUInt32Value(base + 0, 0);
    m_Player->SetUInt32Value(base + 1, 0);
    m_Player->SetUInt64Value(base + 2, 0);
    m_Player->SetUInt32Value(base + 4, 0);

    // clear from player log
    m_Player->SetQuestLogSlot(NULL, m_slot);

    // delete ourselves
    delete this;
}

void QuestLogEntry::UpdatePlayerFields()
{
    if(m_Player == NULL)
        return;

    m_Player->SetUInt32Value(PLAYER_QUEST_LOG + m_slot*5 + 0, m_quest->id);
    m_Player->SetUInt32Value(PLAYER_QUEST_LOG + m_slot*5 + 1, m_questStatus == QUEST_STATUS__FAILED ? 0x02 : ((CanBeFinished() || m_questStatus == QUEST_STATUS__COMPLETE) ? 0x01 : 0x00));
    for(uint8 i = 0; i < 4; i++)
    {
        uint8 index = i/2, offset = i%2;
        if(m_quest->required_mob[i])
        {
            if(m_objectiveCount[i] == 0)
                continue;
            m_Player->SetUInt16Value(PLAYER_QUEST_LOG + m_slot*5 + 2 + index, offset, m_objectiveCount[i]);
        } else if(m_quest->required_areatriggers[i])
            m_Player->SetUInt16Value(PLAYER_QUEST_LOG + m_slot*5 + 2 + index, offset, (m_areaTriggerFlags&(1<<i)) ? 1 : 0);
    }

    m_Player->SetUInt32Value(PLAYER_QUEST_LOG + m_slot*5 + 4, m_expirationTime);
}

void QuestLogEntry::SendQuestComplete()
{
    WorldPacket data(SMSG_QUESTUPDATE_COMPLETE, 4);
    data << m_quest->id;
    m_Player->PushPacket(&data);
    TRIGGER_QUEST_EVENT(m_quest->id, OnQuestComplete)(m_Player, this);
}

void QuestLogEntry::SendUpdateAddKill(uint8 i)
{
    sQuestMgr.SendQuestUpdateAddKill(m_Player, m_quest->id, m_quest->required_mob[i], m_objectiveCount[i], m_quest->required_mobcount[i], 0);
}

uint32 QuestLogEntry::GetRequiredSpell()
{
    if(m_quest->required_spell[0])
    {
        if(!m_Player->HasSpell(m_quest->required_spell[0]))
        {
            SpellEntry* reqspell = dbcSpell.LookupEntry(m_quest->required_spell[0]);
             // Spell has a power type, so we check if player has spells with the same namehash, and replace it with that.
            if(reqspell && (reqspell->powerType != m_Player->getPowerType()))
            {
                if(uint32 newspell = m_Player->FindSpellWithNamehash(reqspell->NameHash))
                    return newspell;
            }
        }

        return m_quest->required_spell[0];
    }
    return 0;
}
