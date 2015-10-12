/***
 * Demonstrike Core
 */

#include "StdAfx.h"

BaseSpell::BaseSpell(WorldObject* caster, SpellEntry *info, uint8 castNumber) : m_caster(caster), m_spellInfo(info), m_castNumber(castNumber)
{
    m_duration = -1;
    m_radius[0][0] = m_radius[0][1] = m_radius[0][2] = 0.f;
    m_radius[1][0] = m_radius[1][1] = m_radius[1][2] = 0.f;
    b_durSet = b_radSet[0] = b_radSet[1] = b_radSet[2] = false;
    m_spellState = SPELL_STATE_NULL;
}

BaseSpell::~BaseSpell()
{
}

void BaseSpell::Destruct()
{
    m_caster = NULL;
    m_spellInfo = NULL;
    delete this;
}

void BaseSpell::SendCastResult(uint8 result)
{
    if(result == SPELL_CANCAST_OK)
        return;

    if(!m_caster->IsInWorld())
        return;

    Player* plr = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
    if( plr == NULL)
        return;

    // reset cooldowns
    if( m_spellState == SPELL_STATE_PREPARING )
        plr->Cooldown_OnCancel(m_spellInfo);

    uint32 Extra = 0;
    switch( result )
    {
    case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
        Extra = GetSpellProto()->RequiresSpellFocus;
        break;

    case SPELL_FAILED_REQUIRES_AREA:
        {
            if( GetSpellProto()->AreaGroupId > 0 )
            {
                uint16 area_id = plr->GetAreaId();
                AreaGroupEntry *GroupEntry = dbcAreaGroup.LookupEntry( GetSpellProto()->AreaGroupId );

                for( uint8 i = 0; i < 7; i++ )
                {
                    if( GroupEntry->AreaId[i] != 0 && GroupEntry->AreaId[i] != area_id )
                    {
                        Extra = GroupEntry->AreaId[i];
                        break;
                    }
                }
            }
        }break;

    case SPELL_FAILED_TOTEMS:
        Extra = GetSpellProto()->Totem[1] ? GetSpellProto()->Totem[1] : GetSpellProto()->Totem[0];
        break;
    }

    plr->SendCastResult(m_spellInfo->Id, result, m_castNumber, Extra);
}

void BaseSpell::SendInterrupted(uint8 result)
{
    if(!m_caster->IsUnit() || !m_caster->IsInWorld()) 
        return;

    WorldPacket data(SMSG_SPELL_FAILURE, 13);
    data << m_caster->GetGUID();
    data << uint8(m_castNumber);
    data << uint32(m_spellInfo->Id);
    data << uint8(result);
    m_caster->SendMessageToSet(&data, true);

    data.Initialize(SMSG_SPELL_FAILED_OTHER);
    data << m_caster->GetGUID();
    data << uint8(m_castNumber);
    data << uint32(m_spellInfo->Id);
    data << uint8(result);
    m_caster->SendMessageToSet(&data, false);
}

void BaseSpell::SendChannelStart(int32 duration)
{
    if(!m_caster->IsUnit() || !m_caster->IsInWorld()) 
        return;

    WorldPacket data(MSG_CHANNEL_START, 16);
    data << m_caster->GetGUID();
    data << m_spellInfo->Id;
    data << duration;
    m_caster->SendMessageToSet(&data, true);
}

void BaseSpell::SendChannelUpdate(uint32 time)
{
    if(!m_caster->IsUnit() || !m_caster->IsInWorld()) 
        return;

    WorldPacket data(MSG_CHANNEL_UPDATE, 12);
    data << m_caster->GetGUID();
    data << time;
    m_caster->SendMessageToSet(&data, true);
}
