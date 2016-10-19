/***
 * Demonstrike Core
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

void WorldSession::HandleUnlearnTalents( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint32 playerGold = GetPlayer()->GetUInt32Value( PLAYER_FIELD_COINAGE );
    uint32 price = GetPlayer()->CalcTalentResetCost(GetPlayer()->m_talentInterface.GetTalentResets());

    if( playerGold < price ) return;

    GetPlayer()->ResetSpec(_player->m_talentInterface.GetActiveSpec());
    GetPlayer()->SetUInt32Value( PLAYER_FIELD_COINAGE, playerGold - price );
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
