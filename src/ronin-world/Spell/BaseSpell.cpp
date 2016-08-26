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
            case 2:                         // Keystones
                data.read_skip<uint32>();   // entry
                data.read_skip<uint32>();   // count
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
    m_triggeredSpell = m_projectileWait = m_AreaAura = b_durSet = b_radSet[0] = b_radSet[1] = b_radSet[2] = false;
    m_spellState = SPELL_STATE_NULL;
    m_hitTargetCount = m_missTargetCount = 0;
    m_triggeredByAura = NULL;
    m_missilePitch = 0.f;
    m_missileTravelTime = m_MSTimeToAddToTravel = 0;
    m_timer = m_castTime = 0;
}

BaseSpell::~BaseSpell()
{

}

void BaseSpell::_Prepare()
{
    m_missilePitch = m_targets.missilepitch;
    m_missileTravelTime = floor(m_targets.traveltime);

    if(m_missileTravelTime || m_spellInfo->speed > 0.0f && !m_spellInfo->IsSpellChannelSpell() || m_spellInfo->Id == 14157)
        m_projectileWait = true;

    if((m_spellInfo->SpellScalingId || m_spellInfo->CastingTimeIndex) && !(m_triggeredSpell || (m_caster->IsPlayer() && castPtr<Player>(m_caster)->CastTimeCheat)))
    {
        uint32 level = m_caster->getLevel();
        if(m_spellInfo->SpellScalingId)
        {
            m_castTime = m_spellInfo->castTimeMin;
            if(level > 1)
            {
                if(level < m_spellInfo->castScalingMaxLevel)
                    m_castTime += ((level-1) * (m_spellInfo->castTimeMax-m_spellInfo->castTimeMin))/m_spellInfo->castScalingMaxLevel;
                else m_castTime = m_spellInfo->castTimeMax;
            }
        }
        else // Via cast time index
        {
            m_castTime = m_spellInfo->castTime;
            if(m_spellInfo->SpellLevelsId)
            {
                if (m_spellInfo->spellLevelMaxLevel && level > m_spellInfo->spellLevelMaxLevel)
                    level = m_spellInfo->spellLevelMaxLevel;
                if(level < m_spellInfo->spellLevelBaseLevel)
                    level = m_spellInfo->spellLevelBaseLevel;
                level -= m_spellInfo->spellLevelBaseLevel;
            }

            // currently only profession spells have CastTimePerLevel data filled, always negative
            m_castTime += m_spellInfo->castTimePerLevel * level;
            if(m_castTime < m_spellInfo->baseCastTime)
                m_castTime = m_spellInfo->baseCastTime;
        }

        if(m_castTime && m_caster->IsUnit())
        {
            if(m_spellInfo->SpellGroupType)
            {
                castPtr<Unit>(m_caster)->SM_FIValue( SMT_CAST_TIME, (int32*)&m_castTime, m_spellInfo->SpellGroupType );
                castPtr<Unit>(m_caster)->SM_PIValue( SMT_CAST_TIME, (int32*)&m_castTime, m_spellInfo->SpellGroupType );
            }

            if (!(m_spellInfo->isAbilitySpell() || m_spellInfo->isTradeSpell()))
                m_castTime *= castPtr<Unit>(m_caster)->GetFloatValue(UNIT_MOD_CAST_SPEED);
            /*else if(m_spellInfo->isSpellRangedSpell() && !m_spellInfo->isAutoRepeatSpell())
            m_castTime *= castPtr<Unit>(m_caster)->getattacks[RANGED_ATTACK]);*/
        }

        //let us make sure cast_time is within decent range
        //this is a hax but there is no spell that has more then 10 minutes cast time
        m_timer = m_castTime = std::min<uint32>(10*60*1000, std::max<int32>(0, m_castTime));
    }
}

void BaseSpell::Destruct()
{
    m_effectTargetMaps[0].clear();
    m_effectTargetMaps[1].clear();
    m_effectTargetMaps[2].clear();
    while(!m_fullTargetMap.empty())
    {
        SpellTarget *tgt = m_fullTargetMap.begin()->second;
        m_fullTargetMap.erase(m_fullTargetMap.begin());
        delete tgt;
    }
    m_fullTargetMap.clear();

    m_caster = NULL;
    m_spellInfo = NULL;

    delete this;
}

void BaseSpell::writeSpellGoTargets( WorldPacket * data )
{
    SpellTargetMap::iterator itr;
    uint32 counter;

    // Make sure we don't hit over 100 targets.
    // It's fine internally, but sending it to the client will REALLY cause it to freak.

    *data << uint8(std::min<uint32>(100, m_hitTargetCount));
    if( m_hitTargetCount > 0 )
    {
        counter = 0;
        for( itr = m_fullTargetMap.begin(); itr != m_fullTargetMap.end() && counter < 100; itr++ )
        {
            if( itr->second->HitResult == SPELL_DID_HIT_SUCCESS )
            {
                *data << itr->first;
                ++counter;
            }
        }
    }

    *data << uint8(std::min<uint32>(100, m_missTargetCount));
    if( m_missTargetCount > 0 )
    {
        counter = 0;
        for( itr = m_fullTargetMap.begin(); itr != m_fullTargetMap.end() && counter < 100; itr++ )
        {
            if( itr->second->HitResult != SPELL_DID_HIT_SUCCESS )
            {
                *data << itr->first;
                *data << uint8(itr->second->HitResult);
                if (itr->second->HitResult == SPELL_DID_HIT_REFLECT)
                    *data << uint8(itr->second->ReflectResult);
                ++counter;
            }
        }
    }
}

void BaseSpell::writeSpellCastFlagData(WorldPacket *data, uint32 cast_flags)
{
    if (cast_flags & SPELL_CASTFLAG_POWER_UPDATE) //send new power
        *data << uint32(castPtr<Unit>(m_caster)->GetPower(GetSpellProto()->powerType));

    if( cast_flags & SPELL_CASTFLAG_RUNE_UPDATE ) //send new runes
    {
        uint8 runeMask = m_caster->IsPlayer() ? castPtr<Player>(m_caster)->GetRuneMask() : 0x3F, theoretical = m_caster->IsPlayer() ? castPtr<Player>(m_caster)->TheoreticalUseRunes(m_spellInfo->runeCost) : 0;
        *data << runeMask << theoretical;
        for (uint8 i = 0; i < 6; i++)
        {
            uint8 mask = (1 << i);
            if (mask & runeMask && !(mask & theoretical))
                *data << uint8(0);
        }
    }

    if (cast_flags & SPELL_CASTFLAG_MISSILE_INFO)
        *data << m_missilePitch << m_missileTravelTime;

    if(cast_flags & SPELL_CASTFLAG_VISUAL_CHAIN)
        *data << uint32(0) << uint32(0);

    if(cast_flags & SPELL_CASTFLAG_PROJECTILE)
        *data << uint32(0) << uint32(0);

    if(cast_flags & SPELL_CASTFLAG_EXTRA_MESSAGE)
        *data << uint32(0) << uint32(0);
    
    if(cast_flags & SPELL_CASTFLAG_HEAL_UPDATE)
    {
        uint32 amount = 0, type = 0;
        if(m_spellInfo->HasEffect(SPELL_EFFECT_HEAL))
            amount = m_spellInfo->CalculateSpellPoints(m_spellInfo->GetEffectIndex(SPELL_EFFECT_HEAL), m_caster->getLevel(), 0);
        else if(m_spellInfo->HasEffect(SPELL_EFFECT_HEAL_PCT))
            type = 1, amount = m_spellInfo->CalculateSpellPoints(m_spellInfo->GetEffectIndex(SPELL_EFFECT_HEAL_PCT), m_caster->getLevel(), 0);
        //else if(m_spellInfo->AppliesAura(SPELL_AURA_PERIODIC_HEAL)) {}// TODO

        *data << uint32(amount) << uint8(type);
        if(type == 2) *data << m_caster->GetGUID().asPacked();
    }

    if( m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION )
        *data << uint8(0);

    if( m_targets.m_targetMask & TARGET_FLAG_EXTRA_TARGETS )
        *data << uint32(0);
}

void BaseSpell::SendSpellStart()
{
    if(!IsNeedSendToClient())
        return;

    uint32 cast_flags = SPELL_CASTFLAG_HAS_TRAJECTORY;
    if((m_triggeredSpell || m_triggeredByAura) && !m_spellInfo->isAutoRepeatSpell())
        cast_flags |= SPELL_CASTFLAG_NO_VISUAL;
    if(GetSpellProto()->powerType > 0 && GetSpellProto()->powerType != POWER_TYPE_HEALTH)
        cast_flags |= SPELL_CASTFLAG_POWER_UPDATE;
    if (m_spellInfo->RuneCostID && m_spellInfo->powerType == POWER_TYPE_RUNE)
        cast_flags |= SPELL_CASTFLAG_NO_GCD;
    if(m_castTime && (m_spellInfo->HasEffect(SPELL_EFFECT_HEAL) || m_spellInfo->HasEffect(SPELL_EFFECT_HEAL_PCT) || m_spellInfo->AppliesAura(SPELL_AURA_PERIODIC_HEAL)))
        cast_flags |= SPELL_CASTFLAG_HEAL_UPDATE;

    WorldPacket data(SMSG_SPELL_START, 150);
    data << m_caster->GetGUID().asPacked();
    data << m_caster->GetGUID().asPacked();
    data << uint8(m_castNumber);
    data << uint32(GetSpellProto()->Id);
    data << uint32(cast_flags);
    data << uint32(m_timer);
    data << uint32(m_castTime);

    m_targets.write( data );

    writeSpellCastFlagData(&data, cast_flags);

    m_caster->SendMessageToSet( &data, m_caster->IsPlayer() );
}

void BaseSpell::SendSpellGo()
{
    if(!IsNeedSendToClient())
        return;

    uint32 cast_flags = SPELL_CASTFLAG_NONE;
    if((m_triggeredSpell || m_triggeredByAura) && !m_spellInfo->isAutoRepeatSpell())
        cast_flags |= SPELL_CASTFLAG_NO_VISUAL;
    if(GetSpellProto()->powerType > 0 && GetSpellProto()->powerType != POWER_TYPE_HEALTH)
        cast_flags |= SPELL_CASTFLAG_POWER_UPDATE;
    if(m_caster->IsUnit() && castPtr<Unit>(m_caster)->getClass() == DEATHKNIGHT && (GetSpellProto()->RuneCostID || m_spellInfo->HasEffect(SPELL_EFFECT_ACTIVATE_RUNE)))
        cast_flags |= (SPELL_CASTFLAG_NO_GCD | SPELL_CASTFLAG_RUNE_UPDATE);
    else if(m_spellInfo->StartRecoveryTime == 0)
        cast_flags |= SPELL_CASTFLAG_NO_GCD;
    if(m_targets.missilespeed)
        cast_flags |= SPELL_CASTFLAG_MISSILE_INFO;

    WorldPacket data(SMSG_SPELL_GO, 200);
    data << m_caster->GetGUID().asPacked();
    data << m_caster->GetGUID().asPacked();
    data << uint8(m_castNumber);
    data << uint32(m_spellInfo->Id);
    data << uint32(cast_flags);
    data << uint32(m_timer);
    data << uint32(getMSTime());

    writeSpellGoTargets(&data);

    m_targets.write( data ); // this write is included the target flag

    writeSpellCastFlagData(&data, cast_flags);

    m_caster->SendMessageToSet( &data, m_caster->IsPlayer() );
}

bool BaseSpell::IsNeedSendToClient()
{
    if(!m_caster->IsUnit() || !m_caster->IsInWorld() || m_spellInfo->isPassiveSpell())
        return false;
    if(m_spellInfo->SpellVisual[0] || m_spellInfo->SpellVisual[1])
        return true;
    if(m_spellInfo->isChanneledSpell() || m_spellInfo->isChanneledSpell2())
        return true;
    if(m_spellInfo->speed > 0.0f)
        return true;
    if(!m_triggeredSpell)
        return true;
    return false;
}

void BaseSpell::SendProjectileUpdate()
{
    WorldPacket data(SMSG_SET_PROJECTILE_POSITION, 40);
    data << m_caster->GetGUID();
    data << m_castNumber;
    data << float(0.0f) << float(0.0f) << float(0.0f);
    m_caster->SendMessageToSet(&data, true);
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
