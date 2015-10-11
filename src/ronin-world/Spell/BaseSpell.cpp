/***
 * Demonstrike Core
 */

#include "StdAfx.h"

BaseSpell::BaseSpell(WorldObject* caster, SpellEntry *info, uint8 castNumber) : m_caster(caster), m_spellInfo(info), m_castNumber(castNumber)
{

}

BaseSpell::~BaseSpell()
{
    m_caster = NULL;
    m_spellInfo = NULL;
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
