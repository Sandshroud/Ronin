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

#define SPELL_CHANNEL_UPDATE_INTERVAL 1000

enum SpellTargetSpecification
{
    TARGET_SPECT_NONE       = 0,
    TARGET_SPEC_INVISIBLE   = 1,
    TARGET_SPEC_DEAD        = 2,
};

Spell::Spell(Unit* Caster, SpellEntry *info, uint8 castNumber, WoWGuid itemGuid, Aura* aur) : SpellEffectClass(Caster, info, castNumber, itemGuid)
{
    ASSERT( Caster != NULL && info != NULL );

    chaindamage = 0;
    m_pushbackCount = 0;

    duelSpell = (_unitCaster->IsPlayer() && castPtr<Player>(_unitCaster)->GetDuelState() == DUEL_STATE_STARTED);
    Caster->GetPosition(m_castPositionX, m_castPositionY, m_castPositionZ);
    m_AreaAura = false;

    m_triggeredByAura = aur;

    damageToHit = 0;
    castedItemId = 0;

    m_usesMana = false;

    m_canCastResult = SPELL_CANCAST_OK;
    damage = 0;
    m_Delayed = false;
    m_ForceConsumption = false;
    m_cancelled = false;
    m_reflectedParent = NULL;
    m_isCasting = false;
    m_magnetTarget = 0;
}

Spell::~Spell()
{

}

void Spell::Destruct()
{
    objTargetGuid.Clean();
    itemTargetGuid.Clean();
    m_magnetTarget.Clean();

    m_triggeredByAura = NULL;
    m_reflectedParent = NULL;
    SpellEffectClass::Destruct();
}

bool Spell::IsAuraApplyingSpell()
{
    if(m_spellInfo->HasEffect(SPELL_EFFECT_APPLY_AURA))
        return true;
    for(uint8 i = 0; i < 3; i++)
        if(m_spellInfo->EffectApplyAuraName[i])
            return true;
    return false;
}

//i might forget conditions here. Feel free to add them
bool Spell::IsStealthSpell()
{
    //check if aura name is some stealth aura
    if( m_spellInfo->EffectApplyAuraName[0] == 16 ||
        m_spellInfo->EffectApplyAuraName[1] == 16 ||
        m_spellInfo->EffectApplyAuraName[2] == 16 )
        return true;
    return false;
}

//i might forget conditions here. Feel free to add them
bool Spell::IsInvisibilitySpell()
{
    //check if aura name is some invisibility aura
    if( m_spellInfo->EffectApplyAuraName[0] == 18 ||
        m_spellInfo->EffectApplyAuraName[1] == 18 ||
        m_spellInfo->EffectApplyAuraName[2] == 18 )
        return true;
    return false;
}

bool Spell::CanEffectTargetGameObjects(uint32 i)
{
    switch(m_spellInfo->Effect[i])
    {
    case SPELL_EFFECT_DUMMY:
    case SPELL_EFFECT_OPEN_LOCK:
    case SPELL_EFFECT_OPEN_LOCK_ITEM:
    case SPELL_EFFECT_ACTIVATE_OBJECT:
    case SPELL_EFFECT_WMO_DAMAGE:
    case SPELL_EFFECT_WMO_REPAIR:
    case SPELL_EFFECT_WMO_CHANGE:
        return true;
    }
    return false;
}

uint8 Spell::_DidHit(Unit* target, float *resistOut, uint8 *reflectout)
{
    //note resistchance is vise versa, is full hit chance
    if( target == NULL )
        return SPELL_DID_HIT_MISS;

    /************************************************************************/
    /* Can't can't miss your own spells                                     */
    /************************************************************************/
    if(_unitCaster == target)
        return SPELL_DID_HIT_SUCCESS;

    /************************************************************************/
    /* Check if the unit is evading                                      */
    /************************************************************************/
    if(target->IsCreature() && castPtr<Creature>(target)->hasStateFlag(UF_EVADING))
        return SPELL_DID_HIT_EVADE;

    if(uint32 mechanic = m_spellInfo->MechanicsType)
    {
        /*************************************************************************/
        /* Check if the target is immune to this mechanic                       */
        /*************************************************************************/
        if(target->GetMechanicDispels(mechanic))
            return SPELL_DID_HIT_IMMUNE; // Moved here from Spell::CanCast

        // Creature Aura Immune Flag Check
        if (Creature* cTarget = target->IsCreature() ? castPtr<Creature>(target) : NULL)
            if(cTarget->GetCreatureData()->auraMechanicImmunity && (cTarget->GetCreatureData()->auraMechanicImmunity & (uint32(1)<<(mechanic-1))))
                return SPELL_DID_HIT_IMMUNE;

        /************************************************************************/
        /* Check if the target has a % resistance to this mechanic            */
        /************************************************************************/
        if( mechanic < MECHANIC_COUNT)
        {
            float res = target->GetMechanicResistPCT(Spell::GetMechanic(m_spellInfo));
            if( !m_spellInfo->isSpellResistanceIgnorant() && Rand(res))
                return SPELL_DID_HIT_RESIST;
        }
    }

    /************************************************************************/
    /* Check if the spell is a melee attack and if it was missed/parried    */
    /************************************************************************/
    uint32 meleeResult = 0;
    if( m_spellInfo->IsSpellWeaponSpell() && (meleeResult = _unitCaster->GetSpellDidHitResult(target, m_spellInfo->spellType, m_spellInfo)) )
        return meleeResult;
    return _unitCaster->GetSpellDidHitResult(target, this, resistOut, reflectout);
}

uint8 Spell::prepare(SpellCastTargets *targets, bool triggered)
{
    uint8 ccr = SPELL_CANCAST_OK;
    if( _unitCaster->IsPlayer() && (m_spellInfo->Id == 51514 || m_spellInfo->NameHash == SPELL_HASH_ARCANE_SHOT || m_spellInfo->NameHash == SPELL_HASH_MIND_FLAY))
    {
        targets->m_unitTarget = 0;
        GenerateTargets( targets );
    }

    m_targets = *targets;
    m_triggeredSpell = triggered;

    // Call base spell preparations
    _Prepare();

    // Handle triggered spells here that aren't channeled spells
    if( m_triggeredSpell && !m_spellInfo->IsSpellChannelSpell())
    {
        cast( false );
        return ccr;
    }

    SendSpellStart();

    // Non triggered spell cast checks
    if(m_triggeredSpell == false && !m_spellInfo->isCastableWhileMounted())
        _unitCaster->Dismount();

    // Double check our dismounting here
    if(!m_spellInfo->isCastableWhileMounted())
        _unitCaster->Dismount();

    // Triggered channel spells can ignore power checks
    if( m_triggeredSpell == false && !HasPower() )
    {
        SendCastResult(SPELL_FAILED_NO_POWER);
        finish();
        return SPELL_FAILED_NO_POWER;
    }

    // start cooldown handler
    if( _unitCaster->IsPlayer() )
        AddStartCooldown();

    if( m_timer > 0 )
        _unitCaster->interruptAttackTimer(m_timer+1000);

    // aura state removal
    if( m_spellInfo->CasterAuraState && m_spellInfo->CasterAuraState != AURASTATE_FLAG_JUDGEMENT )
        _unitCaster->RemoveFlag( UNIT_FIELD_AURASTATE, m_spellInfo->CasterAuraState );

    m_spellState = SPELL_STATE_PREPARING;

    // instant cast(or triggered) and not channeling
    if( ( m_castTime > 0 || m_spellInfo->IsSpellChannelSpell() ) && !m_triggeredSpell  )
        _unitCaster->GetSpellInterface()->ProcessSpell( this );
    else cast( false );

    return ccr;
}

void Spell::cancel()
{
    if(m_spellState == SPELL_STATE_FINISHED)
        return;

    SendInterrupted(0);
    SendCastResult(SPELL_FAILED_INTERRUPTED);

    if(m_spellState == SPELL_STATE_CASTING)
    {
        if(m_timer > 0 || m_Delayed)
        {
            if(_unitCaster->IsPlayer())
            {
                Unit* pTarget = _unitCaster->GetMapInstance()->GetUnit(_unitCaster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT));
                if(!pTarget)
                    pTarget = _unitCaster->GetMapInstance()->GetUnit(castPtr<Player>(_unitCaster)->GetSelection());

                if(pTarget)
                    pTarget->RemoveAura(m_spellInfo->Id, _unitCaster->GetGUID());

                if(m_AreaAura)//remove of blizz and shit like this
                {
                    DynamicObject* dynObj = _unitCaster->GetMapInstance()->GetDynamicObject(_unitCaster->GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT));
                    if(dynObj)
                    {
                        dynObj->RemoveFromWorld();
                        dynObj->Destruct();
                        dynObj = NULL;
                    }
                }

                if(GameObject *summon = castPtr<Player>(_unitCaster)->GetSummonedObject())
                {
                    if(summon->IsInWorld())
                        summon->RemoveFromWorld();
                    // for now..
                    ASSERT(summon->GetTypeId() == TYPEID_GAMEOBJECT);
                    summon->Destruct();
                    castPtr<Player>(_unitCaster)->SetSummonedObject(NULL);
                }

                if(m_timer > 0)
                {
                    castPtr<Player>(_unitCaster)->interruptAttackTimer(-m_timer);
                    RemoveItems();
                }
             }
        }
    }

    SendChannelUpdate(0);

    // Ensure the item gets consumed once the channel has started
    if (m_timer > 0)
        m_ForceConsumption = true;

    if( !m_isCasting )
        finish();
}

void Spell::AddCooldown()
{
    if( _unitCaster->IsPlayer() && !castPtr<Player>(_unitCaster)->hasCooldownCheat())
        castPtr<Player>(_unitCaster)->Cooldown_Add( m_spellInfo, NULL );
}

void Spell::AddStartCooldown()
{
    if( _unitCaster->IsPlayer() && !castPtr<Player>(_unitCaster)->hasCooldownCheat())
        castPtr<Player>(_unitCaster)->Cooldown_AddStart( m_spellInfo );
}

void Spell::cast(bool check)
{
    if( duelSpell && ( _unitCaster->IsPlayer() && castPtr<Player>(_unitCaster)->GetDuelState() != DUEL_STATE_STARTED ) )
    {
        // Can't cast that!
        SendInterrupted( SPELL_FAILED_TARGET_FRIENDLY );
        finish();
        return;
    }

    sLog.Debug("Spell","Cast %u, Unit: %u", m_spellInfo->Id, _unitCaster->GetLowGUID());

    bool isNextMeleeAttack1 = m_spellInfo->isNextMeleeAttack1();
    if (_unitCaster->IsPlayer() && !m_triggeredSpell && _unitCaster->IsInWorld() && GUID_HIPART(m_targets.m_unitTarget) == HIGHGUID_TYPE_UNIT)
        sQuestMgr.OnPlayerCast(castPtr<Player>(_unitCaster), m_spellInfo->Id, m_targets.m_unitTarget);

    // trigger on next attack spells only check power
    if( isNextMeleeAttack1 && m_triggeredSpell == false )
    {
        // check power
        if(!HasPower())
        {
            SendInterrupted(SPELL_FAILED_NO_POWER);
            SendCastResult(SPELL_FAILED_NO_POWER);
            finish();
            return;
        }

        // Trigger our spell cooldown here, and not at trigger
        AddCooldown();

        // we're much better to remove this here, because otherwise spells that change powers etc,
        // don't get applied.
        if(!m_triggeredByAura) _unitCaster->m_AuraInterface.RemoveAllAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, m_spellInfo->Id);

        m_isCasting = false;
        SendCastResult(m_canCastResult);
        finish();
        return;
    }

    // Only take power if we're not a triggered spell, or we're a triggered next melee attack
    if(isNextMeleeAttack1 || m_triggeredSpell == false)
    {
        if(!TakePower())
        {
            SendInterrupted(SPELL_FAILED_NO_POWER);
            SendCastResult(SPELL_FAILED_NO_POWER);
            finish();
            return;
        }
    }

    FillTargetMap(false);

    m_isCasting = true;

    if(m_triggeredSpell == false)
        AddCooldown();

    SendSpellGo();

    if( m_spellInfo->IsSpellChannelSpell() && !m_triggeredSpell )
    {
        m_spellState = SPELL_STATE_CASTING;
        SendChannelStart(GetDuration());
        // Set our channel spell to this
        _unitCaster->SetChannelSpellId(m_spellInfo->Id);
        // If we have a single spell target and it's not us, set our channel focus to that
        if(m_fullTargetMap.size() == 1 && m_spellInfo->isChannelTrackTarget())
            _unitCaster->SetChannelSpellTargetGUID(m_fullTargetMap.begin()->first);
        _unitCaster->GetSpellInterface()->ProcessSpell(this);
        return;
    }
    else if(m_missileSpeed > 0.f)
    {
        if(!m_delayTargets.empty() || m_isDelayedAOEMissile)
            _unitCaster->GetMapInstance()->AddProjectile(this);
        finish();
        return;
    }

    std::set<WoWGuid> unitTargets;
    for(uint8 i = 0; i < 3; i++)
    {
        if(m_effectTargetMaps[i].empty())
            continue;

        for(SpellTargetStorage::iterator itr = m_effectTargetMaps[i].begin(); itr != m_effectTargetMaps[i].end(); itr++)
        {
            if(WorldObject *target = _unitCaster->GetInRangeObject(itr->first))
            {
                HandleEffects(i, itr->second, target);
                if(!target->IsUnit() || unitTargets.find(itr->first) != unitTargets.end())
                    continue;
                unitTargets.insert(itr->first);
            }
        }
    }

    for(auto itr = unitTargets.begin(); itr != unitTargets.end(); itr++)
    {
        SpellTargetStorage::iterator tgtItr;
        ASSERT((tgtItr = m_fullTargetMap.find(*itr)) != m_fullTargetMap.end());
        if(Unit *target = _unitCaster->GetInRangeObject<Unit>(*itr))
        {
            HandleDelayedEffects(target, tgtItr->second);
            if(m_spellInfo->TargetAuraState)
                target->RemoveFlag(UNIT_FIELD_AURASTATE, uint32(1) << (m_spellInfo->TargetAuraState - 1) );
        }
    }

    SendSpellMisses();

    // we're much better to remove this here, because otherwise spells that change powers etc, don't get applied.
    _unitCaster->m_AuraInterface.RemoveAllAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, m_spellInfo->Id);

    m_isCasting = false;
    finish();
}

void Spell::AddTime(uint32 type)
{
    if(_unitCaster->IsPlayer())
    {
        Player *p_caster = castPtr<Player>(_unitCaster);
        if( m_spellInfo->InterruptFlags & CAST_INTERRUPT_ON_DAMAGE_TAKEN)
        {
            cancel();
            return;
        }

        if( m_spellInfo->SpellGroupType)
        {
            float ch = 0;
            p_caster->SM_FFValue(SMT_NONINTERRUPT, &ch, m_spellInfo->SpellGroupType);
            if(Rand(ch))
                return;
        }

        if(m_pushbackCount > 1)
            return;

        m_pushbackCount++;
        if(m_spellState == SPELL_STATE_PREPARING)
        {
            int32 delay = 500;
            m_timer += delay;
            if(m_timer>m_castTime)
            {
                delay -= (m_timer - m_castTime);
                m_timer = m_castTime;
                if(delay < 0)
                    delay = 1;
            }

            WorldPacket data(SMSG_SPELL_DELAYED, 13);
            data << p_caster->GetGUID().asPacked();
            data << uint32(delay);
            p_caster->SendMessageToSet(&data, true);
            p_caster->interruptAttackTimer(delay);
        }
        else if(m_spellInfo->IsSpellChannelSpell())
        {
            int32 delay = GetDuration()/4;
            m_timer -= delay;
            if(m_timer < 0)
                m_timer = 0;
            else p_caster->interruptAttackTimer(-delay);

            m_Delayed = true;
            if(m_timer > 0)
                SendChannelUpdate(m_timer);
        }
    }
}

void Spell::Update(uint32 difftime)
{
    updatePosition(_unitCaster->GetPositionX(), _unitCaster->GetPositionY(), _unitCaster->GetPositionZ());

    if(m_cancelled)
    {
        cancel();
        return;
    }

    switch(m_spellState)
    {
    case SPELL_STATE_PREPARING:
        {
            if((int32)difftime >= m_timer)
            {
                m_timer = 0;
                cast(true);
            } else m_timer -= difftime;
        }break;
    case SPELL_STATE_CASTING:
        {
            if(m_timer > 0)
            {
                if((int32)difftime >= m_timer)
                    m_timer = 0;
                else m_timer -= difftime;
            }

            if(m_timer <= 0)
            {
                _UpdateChanneledSpell(0);
                finish();
            }
        }break;
    }
}

void Spell::updatePosition(float x, float y, float z)
{
    if(m_spellInfo->isSpellInterruptOnMovement() && ( m_castPositionX != x || m_castPositionY != y || m_castPositionZ != z))
    {
        if(m_spellInfo->IsSpellChannelSpell() || (_unitCaster->HasNoInterrupt() == 0 && m_spellInfo->EffectMechanic[1] != 14))
        {
            cancel();
            return;
        }
    }
}

void Spell::_UpdateChanneledSpell(uint32 difftime)
{

}

bool Spell::UpdateDelayedTargetEffects(MapInstance *instance, uint32 difftime)
{
    m_delayedTimer += difftime;
    float distanceTraveled = m_delayedTimer * m_missileSpeed;
    // Distance is squared, so square our travel distance
    distanceTraveled *= distanceTraveled;

    if(m_isDelayedAOEMissile)
    {
        float delta_x = RONIN_UTIL::Diff(m_castPositionX, m_targets.m_dest.x);
        float delta_y = RONIN_UTIL::Diff(m_castPositionY, m_targets.m_dest.y);
        float delta_z = RONIN_UTIL::Diff(m_castPositionZ, m_targets.m_dest.z);

        // Wait until we've reached our destination to trigger
        if(distanceTraveled > (delta_x*delta_x + delta_y*delta_y + delta_z*delta_z))
            return false;

        // Refill our target map since we're at our destination
        FillTargetMap(true);

        std::set<WoWGuid> unitTargets;
        // Handle all effects for our new targets
        for(uint8 i = 0; i < 3; i++)
        {
            if(m_effectTargetMaps[i].empty())
                continue;

            for(SpellTargetStorage::iterator itr = m_effectTargetMaps[i].begin(); itr != m_effectTargetMaps[i].end(); itr++)
            {
                WorldObject *target = NULL;
                if(_unitCaster == NULL || (target = _unitCaster->GetInRangeObject<WorldObject>(itr->first)) == NULL)
                {
                    if(itr->first.getHigh() == HIGHGUID_TYPE_GAMEOBJECT)
                        target = instance->GetGameObject(itr->first);
                    else target = instance->GetUnit(itr->first);
                }
                if(target == NULL)
                    continue;

                HandleEffects(i, itr->second, target);
                if(!target->IsUnit() || unitTargets.find(itr->first) != unitTargets.end())
                    continue;
                unitTargets.insert(itr->first);
            }
        }

        // Handle any delayed effects
        for(auto itr = unitTargets.begin(); itr != unitTargets.end(); itr++)
        {
            SpellTargetStorage::iterator tgtItr;
            ASSERT((tgtItr = m_fullTargetMap.find(*itr)) != m_fullTargetMap.end());
            if(Unit *target = _unitCaster->GetInRangeObject<Unit>(*itr))
            {
                HandleDelayedEffects(target, tgtItr->second);
                if(m_spellInfo->TargetAuraState)
                    target->RemoveFlag(UNIT_FIELD_AURASTATE, uint32(1) << (m_spellInfo->TargetAuraState - 1) );
            }
        }

        // Send any spell misses
        SendSpellMisses();
        // Now we're done so trigger our cleanup via mapinstance
        return true;
    }

    // Check our delayed target map
    SpellDelayTargets targets(m_delayTargets);
    for(SpellDelayTargets::iterator itr = targets.begin(); itr != targets.end(); itr++)
    {
        WoWGuid guid = *itr;
        WorldObject *target = NULL;
        SpellTarget *spTarget = GetSpellTarget(guid);
        if(spTarget != NULL)
        {
            if(_unitCaster == NULL || (target = _unitCaster->GetInRangeObject<WorldObject>(guid)) == NULL)
            {
                if(guid.getHigh() == HIGHGUID_TYPE_GAMEOBJECT)
                    target = instance->GetGameObject(guid);
                else target = instance->GetUnit(guid);
            }

            if(target != NULL)
            {
                if(distanceTraveled < target->GetDistanceSq(m_castPositionX, m_castPositionY, m_castPositionZ))
                    continue;

                if(spTarget->HitResult == SPELL_DID_HIT_SUCCESS)
                {
                    for(uint8 i = 0; i < 3; i++)
                        HandleEffects(i, spTarget, target);
                    if(target->IsUnit())
                    {                    
                        HandleDelayedEffects(castPtr<Unit>(target), spTarget);
                        if(m_spellInfo->TargetAuraState)
                            target->RemoveFlag(UNIT_FIELD_AURASTATE, uint32(1) << (m_spellInfo->TargetAuraState - 1) );
                    }
                } else SendSpellMisses(spTarget);
            }
        }

        m_delayTargets.erase(guid);
    }

    targets.clear();
    if(m_delayTargets.empty())
        return true;
    return false;
}

void Spell::finish()
{
    if(m_spellState == SPELL_STATE_FINISHED)
        return;

    m_spellState = SPELL_STATE_FINISHED;

    //enable pvp when attacking another player with spells
    if( _unitCaster->IsPlayer() )
    {
        if(m_Delayed && castPtr<Player>(_unitCaster)->IsInWorld())
        {
            Unit* pTarget = NULL;
            if((pTarget = _unitCaster->GetMapInstance()->GetUnit(_unitCaster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))) == NULL)
                pTarget = _unitCaster->GetMapInstance()->GetUnit(castPtr<Player>(_unitCaster)->GetSelection());
            if(pTarget) pTarget->RemoveAura(m_spellInfo->Id, _unitCaster->GetGUID());
        }

        if( m_spellInfo->HasEffect(SPELL_EFFECT_SUMMON_OBJECT) )
            castPtr<Player>(_unitCaster)->SetSummonedObject(NULL);

    }

    _unitCaster->GetSpellInterface()->FinishSpell(this);

    if(_unitCaster->IsPlayer())
    {
        if(castPtr<Player>(_unitCaster)->hasCooldownCheat() && m_spellInfo)
            castPtr<Player>(_unitCaster)->ClearCooldownForSpell(m_spellInfo->Id);

        if( m_ForceConsumption || ( m_canCastResult == SPELL_CANCAST_OK ) )
            RemoveItems();
    }

    if(m_delayTargets.empty() && !m_isDelayedAOEMissile)
        Destruct();
}

bool Spell::HasPower()
{
    int32 powerField = 0, cost = CalculateCost(powerField);
    if(powerField <= 0)
        return powerField == 0;
    if (cost <= 0)
    {
        m_usesMana = false; // no mana regen interruption for free spells
        return true;
    }

    if(m_spellInfo->powerType == POWER_TYPE_RUNE)
    {
        uint8 runeMask = _unitCaster->GetUInt32Value(powerField);
        return (runeMask & cost) == cost;
    }

    // Unit has enough power (needed for creatures)
    return (cost <= _unitCaster->GetUInt32Value(powerField));
}

bool Spell::TakePower()
{
    Unit *u_caster = _unitCaster;
    int32 powerField = 0, cost = CalculateCost(powerField);
    if(powerField == -1)
        return false;

    bool result = false;
    if(sSpellMgr.HandleTakePower(this, u_caster, m_spellInfo->powerType, cost, result))
        return result;
    if (cost <= 0)
    {
        m_usesMana = false; // no mana regen interruption for free spells
        return true;
    }

    if(m_spellInfo->powerType == POWER_TYPE_RUNE)
    {
        if(uint32 runicGain = m_spellInfo->runicGain)
            u_caster->ModPower(POWER_TYPE_RUNIC, runicGain);
        return true;
    }

    int32 currentPower = _unitCaster->GetUInt32Value(powerField);
    if(powerField == UNIT_FIELD_HEALTH)
    {
        if(cost <= currentPower) // Unit has enough power (needed for creatures)
        {
            _unitCaster->DealDamage(u_caster, cost, 0, 0, 0,true);
            return true;
        }
    }
    else if(cost <= currentPower) // Unit has enough power (needed for creatures)
    {
        if( m_spellInfo->powerType == POWER_TYPE_MANA )
            if(m_spellInfo->IsSpellChannelSpell()) // Client only accepts channels
                u_caster->DelayPowerRegeneration(GetDuration());

        u_caster->SetPower(m_spellInfo->powerType, currentPower - cost);
        return true;
    }
    return false;
}

int32 Spell::CalculateCost(int32 &powerField)
{
    // Initialize powerfield
    powerField = -1;
    // Trainers can always cast, same with players with powercheat
    if(_unitCaster->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER))
        return (powerField = 0);
    uint32 powerType = m_spellInfo->powerType;
    if((powerField = _unitCaster->GetPowerFieldForType(powerType)) == UNIT_END)
        return -1;
    else if(m_spellInfo->powerType == POWER_TYPE_MANA)
        m_usesMana = true;

    int32 cost = _unitCaster->GetSpellBaseCost(m_spellInfo);
    if(powerType == POWER_TYPE_RUNE) // Rune cost is a mask
        return cost;

    int32 currentPower = _unitCaster->GetUInt32Value(powerField);
    if( m_spellInfo->isExhaustingPowerSpell() ) // Uses %100 mana
        return _unitCaster->GetUInt32Value(powerField);

    cost += float2int32(float(cost)* _unitCaster->GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + m_spellInfo->School));

    //apply modifiers
    if( m_spellInfo->SpellGroupType && _unitCaster)
    {
        _unitCaster->SM_FIValue(SMT_COST,&cost,m_spellInfo->SpellGroupType);
        _unitCaster->SM_PIValue(SMT_COST,&cost,m_spellInfo->SpellGroupType);
    }
    return cost;
}

bool Spell::IsBinary(SpellEntry * sp)
{
    // Normally, damage spells are only binary if they have an additional non-damage effect
    // DoTs used to be binary spells, but this was changed. (WoWwiki)
    return !(sp->Effect[0] == SPELL_EFFECT_SCHOOL_DAMAGE || sp->EffectApplyAuraName[0] == SPELL_AURA_PERIODIC_DAMAGE);
}

uint8 Spell::CanCast(bool tolerate)
{
    // no problems found, so we must be ok
    return SPELL_CANCAST_OK;
}

void Spell::RemoveItems()
{

}

void Spell::CreateItem(uint32 itemId)
{

}

bool Spell::Reflect(Unit* refunit)
{
    uint32 refspellid = 0;
    bool canreflect = false;
//  bool remove = false;

    if( m_reflectedParent != NULL || _unitCaster == refunit )
        return false;

    // if the spell to reflect is a reflect spell, do nothing.
    for(int i=0; i<3; i++)
    {
        if( m_spellInfo->Effect[i] == 6 && (m_spellInfo->EffectApplyAuraName[i] == 74 || m_spellInfo->EffectApplyAuraName[i] == 28))
            return false;
    }

    if( !refspellid || !canreflect )
        return false;

    SpellCastTargets targets(_unitCaster->GetGUID());
    if(Spell* spell = new Spell(refunit, m_spellInfo))
    {
        spell->m_reflectedParent = this;
        if(spell->prepare(&targets, true) == SPELL_CANCAST_OK)
            return true;
    }
    return false;
}

void ApplyDiminishingReturnTimer(int32 * Duration, Unit* Target, SpellEntry * spell)
{
    /*uint32 status = GetDiminishingGroup(spell->NameHash);
    uint32 Grp = status & 0xFFFF;   // other bytes are if apply to pvp
    uint32 PvE = (status >> 16) & 0xFFFF;

    // Make sure we have a group
    if(Grp == 0xFFFF) return;

    // Check if we don't apply to pve
    if(!PvE && Target->GetTypeId() != TYPEID_PLAYER && !Target->IsPet())
        return;

    assert(Grp < DIMINISH_GROUPS);

    // TODO: check for spells that should do this
    float Qduration = float(*Duration);
    switch(Target->m_diminishCount[Grp])
    {
    case 0: // Full effect
        if (Target->IsPlayer() && Qduration > 10000)
        {
            Qduration = 10000;
        }
        break;

    case 1: // Reduced by 50%
        Qduration *= 0.5f;
        if (Target->IsPlayer() && Qduration > 5000)
        {
            Qduration = 5000;
        }
        break;

    case 2: // Reduced by 75%
        Qduration *= 0.25f;
        if (Target->IsPlayer() && Qduration > 2500)
        {
            Qduration = 2500;
        }
        break;

    default:// Target immune to spell
        {
            *Duration = 0;
            return;
        }break;
    }

    // Convert back
    *Duration = float2int32(Qduration);

    // Reset the diminishing return counter, and add to the aura count (we don't decrease the timer till we
    // have no auras of this type left.
    ++Target->m_diminishAuraCount[Grp];
    ++Target->m_diminishCount[Grp];*/
}

void UnapplyDiminishingReturnTimer(Unit* Target, SpellEntry * spell)
{
    /*uint32 status = GetDiminishingGroup(spell->NameHash);
    uint32 Grp = status & 0xFFFF;   // other bytes are if apply to pvp
    uint32 PvE = (status >> 16) & 0xFFFF;

    // Make sure we have a group
    if(Grp == 0xFFFF) return;

    // Check if we don't apply to pve
    if(!PvE && Target->GetTypeId() != TYPEID_PLAYER && !Target->IsPet())
        return;

    assert(Grp < DIMINISH_GROUPS);

    Target->m_diminishAuraCount[Grp]--;

    // start timer decrease
    if(!Target->m_diminishAuraCount[Grp])
        Target->SetDiminishTimer(Grp);*/
}

/// Calculate the Diminishing Group. This is based on a name hash.
/// this off course is very hacky, but as its made done in a proper way
/// I leave it here.
uint32 GetDiminishingGroup(uint32 NameHash)
{
    uint32 ret = 0;
    return ret;
}

AuraApplicationResult Spell::CheckAuraApplication(Unit *target)
{
    // If it's passive and we already have it, reject, should happen earlier than this though...
    if(m_spellInfo->isPassiveSpell() && target->HasAura(m_spellInfo->Id))
        return AURA_APPL_REJECTED;

    // Check if we have the aura to be updated
    if(Aura *aur = target->m_AuraInterface.FindActiveAuraWithNameHash(m_spellInfo->NameHash))
    {
        SpellEntry *targetEntry = aur->GetSpellProto();
        // Check to see if we can just stack up our aura
        if(targetEntry->maxstack > 1)
            return AURA_APPL_STACKED;
        // Just refresh the aura, part of cata changes is it doesn't matter what rank
        if(targetEntry->procCharges || (m_spellInfo->RankNumber && targetEntry->RankNumber >= m_spellInfo->RankNumber))
            return AURA_APPL_REFRESH;
    }

    // No aura or complications found, pass through
    return AURA_APPL_SUCCESS;
}

void Spell::_AddTarget(WorldObject* target, const uint32 effIndex)
{
    // Check if we're in the current list already, and if so, don't readd us.
    if(m_effectTargetMaps[effIndex].find(target->GetGUID()) != m_effectTargetMaps[effIndex].end())
        return;

    SpellTarget *tgt = NULL;
    // look for the target in the list already
    SpellTargetStorage::iterator itr = m_fullTargetMap.find(target->GetGUID());
    if(itr != m_fullTargetMap.end())
        tgt = itr->second;
    else
    {
        tgt = new SpellTarget(target->GetGUID());
        tgt->HitResult = target->IsUnit() ? _DidHit(castPtr<Unit>(target), &tgt->resistMod, &tgt->ReflectResult) : SPELL_DID_HIT_SUCCESS;

        if( tgt->HitResult != SPELL_DID_HIT_SUCCESS )
            m_spellMisses.push_back(std::make_pair(tgt->Guid, tgt->HitResult));

        // Add us to the full target map
        m_fullTargetMap.insert(std::make_pair(target->GetGUID(), tgt));
        // If we're a delayed spell push us into our delayed vector
        if(m_missileSpeed > 0.f) m_delayTargets.insert(tgt->Guid);
    }

    if(tgt->HitResult != SPELL_DID_HIT_SUCCESS)
        return;

    // Effect mask used for storage
    uint32 effectMask = (1<<effIndex);
    // Add effect mask
    tgt->EffectMask |= effectMask;
    // Calculate effect amount
    tgt->effectAmount[effIndex] = CalculateEffect(effIndex, target);
    // Call to spell manager to modify the spell amount
    tgt->moddedAmount[effIndex] = sSpellMgr.ModifyEffectAmount(this, effIndex, _unitCaster, target, tgt->effectAmount[effIndex]);
    // Build any modifier data here
    if(m_spellInfo->isSpellAuraApplicator() && target->IsUnit())
    {
        Unit *unitTarget = castPtr<Unit>(target);
        if(tgt->resistMod)
        {
            tgt->AuraAddResult = AURA_APPL_RESISTED;
            ASSERT(tgt->aura == NULL);
        }
        else
        {
            if(tgt->aura == NULL && tgt->AuraAddResult == AURA_APPL_NOT_RUN)
            {
                if((tgt->AuraAddResult = CheckAuraApplication(unitTarget)) == AURA_APPL_SUCCESS)
                {
                    uint16 auraFlags = m_spellInfo->isPassiveSpell() ? 0 : (AFLAG_EFF_AMOUNT_SEND | (m_spellInfo->isNegativeSpell1() ? AFLAG_NEGATIVE : AFLAG_POSITIVE));
                    int16 stackSize = 1;
                    if(m_spellInfo->procCharges && m_spellInfo->SpellGroupType)
                    {
                        stackSize = (m_spellInfo->procCharges&0xFF);
                        _unitCaster->SM_FIValue(SMT_CHARGES, (int32*)&stackSize, m_spellInfo->SpellGroupType);
                        _unitCaster->SM_PIValue(SMT_CHARGES, (int32*)&stackSize, m_spellInfo->SpellGroupType);
                        stackSize *= -1;
                    }
                    tgt->aura = new Aura(unitTarget, m_spellInfo, auraFlags, _unitCaster->getLevel(), stackSize, UNIXTIME, _unitCaster->GetGUID());
                }
            }

            if(tgt->AuraAddResult == AURA_APPL_SUCCESS)
                tgt->aura->AddMod(effIndex, m_spellInfo->EffectApplyAuraName[effIndex], tgt->effectAmount[effIndex]);
        }
    }

    // add to the effect target map
    m_effectTargetMaps[effIndex].insert(std::make_pair(target->GetGUID(), tgt));
}

void Spell::DamageGosAround(uint32 i)
{
    uint32 spell_id = m_spellInfo->Id;
    float r = GetRadius(i);
    r *= r;
    LocationVector target = ((m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION) ? m_targets.m_dest : ((m_targets.m_targetMask & TARGET_FLAG_SOURCE_LOCATION) ? m_targets.m_src : _unitCaster->GetPosition()));
    /*for(WorldObject::InRangeArray::iterator itr = m_caster->GetInRangeGameObjectSetBegin(); itr != m_caster->GetInRangeGameObjectSetEnd(); ++itr )
    {
        if(GameObject *gObj = m_caster->GetInRangeObject<GameObject>(*itr))
        {
            if(gObj->GetDistance2dSq(target.x, target.y) > r)
                continue;
            gObj->TakeDamage(damage,m_caster,m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL,spell_id);
        }
    }*/
}

bool Spell::UseMissileDelay()
{
    if(HasSpellEffect(SPELL_EFFECT_CHARGE) || HasSpellEffect(SPELL_EFFECT_JUMP_TO_TARGET) ||
        HasSpellEffect(SPELL_EFFECT_JUMP_TO_DESTIONATION) || HasSpellEffect(SPELL_EFFECT_TRACTOR_BEAM_FROM_DEST) || 
        HasSpellEffect(SPELL_EFFECT_PLAYER_PULL))
        return false;
    return true;
}


bool CanAgroHash(uint32 spellhashname)
{
    if (spellhashname == SPELL_HASH_HUNTER_S_MARK || spellhashname == SPELL_HASH_SAP || spellhashname == SPELL_HASH_EAGLE_EYE || spellhashname == SPELL_HASH_FAR_SIGHT )    //hunter's mark
        return false;
    return true;
}

bool IsCastedOnFriends(SpellEntry *sp)
{
    for( int frloop = 0; frloop < 3; frloop++ )
    {
        switch (sp->EffectImplicitTargetA[frloop])
        {
            case 1:     //EFF_TARGET_SELF
            case 4:
            case 5:     //EFF_TARGET_PET
            case 20:    //EFF_TARGET_ALL_PARTY_AROUND_CASTER
            case 21:    //EFF_TARGET_SINGLE_FRIEND
            case 27:    //EFF_TARGET_PET_MASTER
            case 30:    //EFF_TARGET_ALL_FRIENDLY_IN_AREA
            case 31:    //EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME
            case 32:    //EFF_TARGET_MINION
            case 33:    //EFF_TARGET_ALL_PARTY_IN_AREA
            case 35:    //EFF_TARGET_SINGLE_PARTY
            case 37:    //EFF_TARGET_ALL_PARTY
            case 41:    //EFF_TARGET_TOTEM_EARTH
            case 42:    //EFF_TARGET_TOTEM_WATER
            case 43:    //EFF_TARGET_TOTEM_AIR
            case 44:    //EFF_TARGET_TOTEM_FIRE
            case 45:    //EFF_TARGET_CHAIN
            case 56:
            case 57:    //EFF_TARGET_PARTY_MEMBER
            case 61:    //EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS
                return true;
        }

        switch (sp->EffectImplicitTargetB[frloop])
        {
            case 1:     //EFF_TARGET_SELF
            case 4:
            case 5:     //EFF_TARGET_PET
            case 20:    //EFF_TARGET_ALL_PARTY_AROUND_CASTER
            case 21:    //EFF_TARGET_SINGLE_FRIEND
            case 27:    //EFF_TARGET_PET_MASTER
            case 30:    //EFF_TARGET_ALL_FRIENDLY_IN_AREA
            case 31:    //EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME
            case 32:    //EFF_TARGET_MINION
            case 33:    //EFF_TARGET_ALL_PARTY_IN_AREA
            case 35:    //EFF_TARGET_SINGLE_PARTY
            case 37:    //EFF_TARGET_ALL_PARTY
            case 41:    //EFF_TARGET_TOTEM_EARTH
            case 42:    //EFF_TARGET_TOTEM_WATER
            case 43:    //EFF_TARGET_TOTEM_AIR
            case 44:    //EFF_TARGET_TOTEM_FIRE
            case 45:    //EFF_TARGET_CHAIN
            case 56:
            case 57:    //EFF_TARGET_PARTY_MEMBER
            case 61:    //EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS
                return true;
        }
    }
    return false;
}

bool IsCastedOnEnemies(SpellEntry *sp)
{
    for( int frloop = 0; frloop < 3; frloop++ )
    {
        switch (sp->EffectImplicitTargetA[frloop])
        {
            case 6:     //EFF_TARGET_SINGLE_ENEMY
            case 8:     //EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS
            case 15:    //EFF_TARGET_ALL_ENEMY_IN_AREA
            case 16:    //EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT
            case 22:    //EFF_TARGET_ALL_ENEMIES_AROUND_CASTER
            case 24:    //EFF_TARGET_IN_FRONT_OF_CASTER
            case 28:    //EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED
            case 54:    //EFF_TARGET_TARGET_AT_ORIENTATION_TO_CASTER
            case 77:    //EFF_TARGET_SELECTED_ENEMY_CHANNELED
                return true;
        }

        switch (sp->EffectImplicitTargetB[frloop])
        {
            case 6:     //EFF_TARGET_SINGLE_ENEMY
            case 8:     //EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS
            case 15:    //EFF_TARGET_ALL_ENEMY_IN_AREA
            case 16:    //EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT
            case 22:    //EFF_TARGET_ALL_ENEMIES_AROUND_CASTER
            case 24:    //EFF_TARGET_IN_FRONT_OF_CASTER
            case 28:    //EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED
            case 54:    //EFF_TARGET_TARGET_AT_ORIENTATION_TO_CASTER
            case 77:    //EFF_TARGET_SELECTED_ENEMY_CHANNELED
                return true;
        }
    }
    return false;
}

/************************************************************************/
/* IsDamagingSpell, this function seems slow, its only used rarely      */
/************************************************************************/
bool IsDamagingSpell(SpellEntry *sp)
{
    if( sp->NameHash == SPELL_HASH_MUTILATE )
        return true;

    for (uint32 i = 0; i < 3; i++)
    {
        switch (sp->Effect[i])
        {
        case SPELL_EFFECT_SCHOOL_DAMAGE:
        case SPELL_EFFECT_ENVIRONMENTAL_DAMAGE:
        case SPELL_EFFECT_HEALTH_LEECH:
        case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
        case SPELL_EFFECT_ADD_EXTRA_ATTACKS:
        case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE:
        case SPELL_EFFECT_POWER_BURN:
        case SPELL_EFFECT_ATTACK:
        case SPELL_EFFECT_DUMMYMELEE:
            return true;

        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA:
            {
                switch (sp->EffectApplyAuraName[i])
                {
                case SPELL_AURA_PERIODIC_DAMAGE://SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PROC_TRIGGER_DAMAGE://SPELL_AURA_PROC_TRIGGER_DAMAGE:
                case SPELL_AURA_PERIODIC_LEECH://SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT://SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                case SPELL_AURA_POWER_BURN_MANA://SPELL_AURA_POWER_BURN:
                    return true;
                }
            }
        }
    }
    return false;
}

bool IsHealingSpell(SpellEntry *sp)
{
    for(uint32 i = 0; i < 3; i++)
    {
        switch( sp->Effect[i] )
        {
        case SPELL_EFFECT_SCHOOL_DAMAGE:
            return false;
            break;

        case SPELL_EFFECT_HEAL:
        case SPELL_EFFECT_HEAL_MAX_HEALTH:
            return true;
            break;

        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA:
        case SPELL_EFFECT_APPLY_RAID_AURA:
            {
                switch( sp->EffectApplyAuraName[i] )
                {
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
                    return true;
                    break;

                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                    {
                        SpellEntry * triggered = dbcSpell.LookupEntry(sp->EffectTriggerSpell[i]);
                        if(triggered && triggered != sp && IsHealingSpell(triggered))
                            return true;
                    }break;
                }
            }break;
        }
    }

    //flash of light, holy light uses a scripted effect which is not necessarily a heal spell
    if( sp->NameHash == SPELL_HASH_HOLY_LIGHT || sp->NameHash == SPELL_HASH_FLASH_OF_LIGHT )
        return true;

    return false;
}

bool IsInrange(LocationVector & location, WorldObject* o, float square_r, float square_min)
{
    float r = o->GetDistanceSq(location);
    if(square_min && r < square_min)
        return false;
    return ( r<=square_r);
}

bool IsInrange(float x1,float y1, float z1, WorldObject* o,float square_r, float square_min)
{
    float r = o->GetDistanceSq(x1, y1, z1);
    if(square_min && r < square_min)
        return false;
    return ( r<=square_r);
}

bool IsInrange(float x1,float y1, float z1,float x2,float y2, float z2,float square_r, float square_min)
{
    float t;
    float r;
    t=x1-x2;
    r=t*t;
    t=y1-y2;
    r+=t*t;
    t=z1-z2;
    r+=t*t;
    if(square_min && r < square_min)
        return false;
    return ( r<=square_r);
}

bool IsInrange(WorldObject* o1,WorldObject* o2,float square_r, float square_min)
{
    return IsInrange(o1->GetPositionX(),o1->GetPositionY(),o1->GetPositionZ(),
        o2->GetPositionX(),o2->GetPositionY(),o2->GetPositionZ(),square_r,square_min);
}

bool TargetTypeCheck(WorldObject* obj,uint32 ReqCreatureTypeMask)
{
    if( !ReqCreatureTypeMask )
        return true;

    if( obj->GetTypeId() == TYPEID_UNIT )
    {
        Creature* cr = castPtr<Creature>(obj);
        CreatureData* inf = cr->GetCreatureData();
        if( inf == NULL || !( 1 << ( inf->type - 1 ) & ReqCreatureTypeMask ) )
            return false;
    } else if(obj->IsPlayer() && !(UT_MASK_HUMANOID & ReqCreatureTypeMask))
        return false;
    return true;
}

bool IsFlyingSpell(SpellEntry *sp)
{
    if( sp->EffectApplyAuraName[0] == 206 ||
        sp->EffectApplyAuraName[1] == 206 ||
        sp->EffectApplyAuraName[1] == 206 ||

        sp->EffectApplyAuraName[0] == 207 ||
        sp->EffectApplyAuraName[1] == 207 ||
        sp->EffectApplyAuraName[1] == 207 ||

        sp->EffectApplyAuraName[0] == 208 ||
        sp->EffectApplyAuraName[1] == 208 ||
        sp->EffectApplyAuraName[1] == 208 ||

        sp->NameHash == SPELL_HASH_SWIFT_FLIGHT_FORM ||
        sp->NameHash == SPELL_HASH_FLIGHT_FORM ||
        sp->NameHash == SPELL_HASH_MAGNIFICENT_FLYING_CARPET ||
        sp->NameHash == SPELL_HASH_FLYING_CARPET )
    {
        return true;
    }

    return false;
}

bool IsTargetingStealthed(SpellEntry *sp)
{
    if( sp->Id == 3600 )
        return false;

    if(     sp->EffectImplicitTargetA[0]==EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS ||
            sp->EffectImplicitTargetA[1]==EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS ||
            sp->EffectImplicitTargetA[2]==EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS ||
            sp->EffectImplicitTargetB[0]==EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS ||
            sp->EffectImplicitTargetB[1]==EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS ||
            sp->EffectImplicitTargetB[2]==EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS ||

            sp->EffectImplicitTargetA[0]==EFF_TARGET_ALL_ENEMIES_AROUND_CASTER ||
            sp->EffectImplicitTargetA[1]==EFF_TARGET_ALL_ENEMIES_AROUND_CASTER ||
            sp->EffectImplicitTargetA[2]==EFF_TARGET_ALL_ENEMIES_AROUND_CASTER ||
            sp->EffectImplicitTargetB[0]==EFF_TARGET_ALL_ENEMIES_AROUND_CASTER ||
            sp->EffectImplicitTargetB[1]==EFF_TARGET_ALL_ENEMIES_AROUND_CASTER ||
            sp->EffectImplicitTargetB[2]==EFF_TARGET_ALL_ENEMIES_AROUND_CASTER ||

            sp->EffectImplicitTargetA[0]==EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED ||
            sp->EffectImplicitTargetA[1]==EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED ||
            sp->EffectImplicitTargetA[2]==EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED ||
            sp->EffectImplicitTargetB[0]==EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED ||
            sp->EffectImplicitTargetB[1]==EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED ||
            sp->EffectImplicitTargetB[2]==EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED ||

            sp->EffectImplicitTargetA[0]==EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
            sp->EffectImplicitTargetA[1]==EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
            sp->EffectImplicitTargetA[2]==EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
            sp->EffectImplicitTargetB[0]==EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
            sp->EffectImplicitTargetB[1]==EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
            sp->EffectImplicitTargetB[2]==EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT
        )
    {
        return true;
    }

    return false;
}
