/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void SpellCastTargets::read( WorldPacket & data, uint64 caster )
{
    data >> m_targetIndex >> m_castFlags >> m_targetMask;
    if( m_targetMask == TARGET_FLAG_SELF || m_targetMask & TARGET_FLAG_GLYPH )
        m_unitTarget = caster;

    if( m_targetMask & (TARGET_FLAG_OBJECT | TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 ) )
        data >> m_unitTarget.asPacked();
    else if( m_targetMask & ( TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM ) )
        data >> m_unitTarget.asPacked();

    if( m_targetMask & TARGET_FLAG_SOURCE_LOCATION )
    {
        data >> m_src_transGuid.asPacked() >> m_src.x >> m_src.y >> m_src.z;
        if( !( m_targetMask & TARGET_FLAG_DEST_LOCATION ) )
        {
            m_dest_transGuid = m_src_transGuid;
            m_dest = m_src;
        }
    }

    if( m_targetMask & TARGET_FLAG_DEST_LOCATION )
    {
        data >> m_dest_transGuid.asPacked() >> m_dest.x >> m_dest.y >> m_dest.z;
        if( !( m_targetMask & TARGET_FLAG_SOURCE_LOCATION ) )
        {
            m_src_transGuid = m_dest_transGuid;
            m_src = m_dest;
        }
    }

    if( m_targetMask & TARGET_FLAG_STRING )
        data >> m_strTarget;

    if(m_castFlags & 0x2)
    {
        data >> missilepitch >> missilespeed;

        float dx = m_dest.x - m_src.x;
        float dy = m_dest.y - m_src.y;
        if((missilepitch != (M_PI / 4)) && (missilepitch != -M_PI / 4))
            traveltime = (sqrtf(dx * dx + dy * dy) / (cosf(missilepitch) * missilespeed)) * 1000;
    }
    else if (m_castFlags & 0x08)         // has archaeology weight
    {
        uint32 count = data.read<uint32>();
        for (uint32 i = 0; i < count; ++i)
        {
            switch (data.read<uint8>()) // Type
            {
            case 1:                         // Fragments
                data.read_skip<uint32>();   // Currency entry
                data.read_skip<uint32>();   // Currency count
                break;
            case 2:                         // Keystones
                data.read_skip<uint32>();   // Item entry
                data.read_skip<uint32>();   // Item count
                break;
            }
        }
    }
}

void SpellCastTargets::write( WorldPacket& data )
{
    data << m_targetMask;

    if( m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_OBJECT | TARGET_FLAG_GLYPH) )
        data << m_unitTarget.asPacked();

    if( m_targetMask & ( TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM ) )
        data << m_itemTarget.asPacked();

    if( m_targetMask & TARGET_FLAG_SOURCE_LOCATION )
        data << m_src_transGuid.asPacked() << m_src.x << m_src.y << m_src.z;

    if( m_targetMask & TARGET_FLAG_DEST_LOCATION )
        data << m_dest_transGuid.asPacked() << m_dest.x << m_dest.y << m_dest.z;

    if (m_targetMask & TARGET_FLAG_STRING)
        data << m_strTarget;
}

BaseSpell::BaseSpell(WorldObject* caster, SpellEntry *info, uint8 castNumber) : m_caster(caster), m_spellInfo(info), m_castNumber(castNumber)
{
    m_duration = -1;
    m_radius[0][0] = m_radius[0][1] = m_radius[0][2] = 0.f;
    m_radius[1][0] = m_radius[1][1] = m_radius[1][2] = 0.f;
    m_AreaAura = b_durSet = b_radSet[0] = b_radSet[1] = b_radSet[2] = false;
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

void BaseSpell::SendHealSpellOnPlayer( WorldObject* caster, WorldObject* target, uint32 dmg, bool critical, uint32 overheal, uint32 spellid)
{
    if( caster == NULL || target == NULL || !target->IsPlayer())
        return;

    WorldPacket data(SMSG_SPELLHEALLOG, 34);
    data << target->GetGUID();
    data << caster->GetGUID();
    data << uint32(spellid);
    data << uint32(dmg);
    data << uint32(overheal);
    data << uint32(0);
    data << uint8(critical ? 1 : 0);
    data << uint8(0);
    caster->SendMessageToSet(&data, true);
}

void BaseSpell::SendHealManaSpellOnPlayer(WorldObject* caster, WorldObject* target, uint32 dmg, uint32 powertype, uint32 spellid)
{
    if( caster == NULL || target == NULL)
        return;

    WorldPacket data(SMSG_SPELLENERGIZELOG, 29);
    data << target->GetGUID();
    data << caster->GetGUID();
    data << uint32(spellid);
    data << uint32(powertype);
    data << uint32(dmg);
    caster->SendMessageToSet(&data, true);
}

void BaseSpell::SendResurrectRequest(Player* target)
{
    const char* name = m_caster->IsCreature() ? castPtr<Creature>(m_caster)->GetName() : "";
    WorldPacket data(SMSG_RESURRECT_REQUEST, 12+strlen(name)+3);
    data << m_caster->GetGUID();
    data << uint32(strlen(name) + 1);
    data << name;
    data << uint8(0);
    data << uint8(m_caster->IsCreature() ? 1 : 0);
    if (m_spellInfo->isResurrectionTimerIgnorant())
        data << uint32(0);
    target->GetSession()->SendPacket(&data);
}
