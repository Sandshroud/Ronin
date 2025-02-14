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

/************************************************************************/
/* Talent Anti-Cheat                                                    */
/************************************************************************/
void WorldSession::HandleLearnTalentOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
 
    uint32 talent_id, requested_rank;
    recv_data >> talent_id >> requested_rank;

    _player->LearnTalent(talent_id, requested_rank);
}

void WorldSession::HandleLearnPreviewTalents( WorldPacket & recv_data )
{
    int32 tabPage, count;
    recv_data >> tabPage >> count;
    if(tabPage >= 0)
    {
        uint8 activeTab =  _player->m_talentInterface.GetActiveTalentTab();
        if(tabPage != activeTab && activeTab != 0xFF)
            return;

        _player->m_talentInterface.SetActiveTalentTab(tabPage);
    }

    uint32 talent_id, requested_rank;
    for(uint32 i = 0; i < count; i++)
    {
        recv_data >> talent_id >> requested_rank;
        if(!_player->LearnTalent(talent_id, requested_rank))
            break;
    }
    _player->m_talentInterface.SendTalentInfo();
}

void WorldSession::HandleUnlearnSkillOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    uint32 skill_line;
    recv_data >> skill_line;

    // Cheater detection
    // if(!_player->HasSkillLine(skill_line)) return;

    // Remove any spells within that line that the player has
    _player->RemoveSpellsFromLine(skill_line);

    // Remove any quests finished and active for this skill
    _player->RemoveQuestsFromLine(skill_line);

    // Finally, remove the skill line.
    _player->RemoveSkillLine(skill_line);
}
