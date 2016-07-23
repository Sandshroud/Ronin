/***
 * Demonstrike Core
 */

#include "StdAfx.h"

#define SPELL_CHANNEL_UPDATE_INTERVAL 1000

enum SpellTargetSpecification
{
    TARGET_SPECT_NONE       = 0,
    TARGET_SPEC_INVISIBLE   = 1,
    TARGET_SPEC_DEAD        = 2,
};

Spell::Spell(WorldObject* Caster, SpellEntry *info, uint8 castNumber, Aura* aur) : SpellEffectClass(Caster, info, castNumber)
{
    ASSERT( Caster != NULL && info != NULL );

    chaindamage = 0;
    m_pushbackCount = 0;

	if (!(duelSpell = (m_caster->IsPlayer() && castPtr<Player>(m_caster)->GetDuelState() == DUEL_STATE_STARTED)))
		duelSpell = (m_caster->IsItem() && castPtr<Item>(m_caster)->GetOwner() && castPtr<Item>(m_caster)->GetOwner()->GetDuelState() == DUEL_STATE_STARTED);

    m_castPositionX = m_castPositionY = m_castPositionZ = 0;
    m_AreaAura = false;

    m_triggeredByAura = aur;

    damageToHit = 0;
    castedItemId = 0;

    m_usesMana = false;

    cancastresult = SPELL_CANCAST_OK;
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
    if( m_caster && m_caster->IsUnit() && castPtr<Unit>(m_caster)->GetCurrentSpell() == this )
        castPtr<Unit>(m_caster)->SetCurrentSpell(NULL);

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
        if(GetSpellProto()->EffectApplyAuraName[i])
            return true;
    return false;
}

//i might forget conditions here. Feel free to add them
bool Spell::IsStealthSpell()
{
    //check if aura name is some stealth aura
    if( GetSpellProto()->EffectApplyAuraName[0] == 16 ||
        GetSpellProto()->EffectApplyAuraName[1] == 16 ||
        GetSpellProto()->EffectApplyAuraName[2] == 16 )
        return true;
    return false;
}

//i might forget conditions here. Feel free to add them
bool Spell::IsInvisibilitySpell()
{
    //check if aura name is some invisibility aura
    if( GetSpellProto()->EffectApplyAuraName[0] == 18 ||
        GetSpellProto()->EffectApplyAuraName[1] == 18 ||
        GetSpellProto()->EffectApplyAuraName[2] == 18 )
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

void Spell::FillSpecifiedTargetsInArea( float srcx, float srcy, float srcz, uint32 ind, uint32 specification )
{
    FillSpecifiedTargetsInArea( ind, srcx, srcy, srcz, GetRadius(ind), specification );
}

// for the moment we do invisible targets
void Spell::FillSpecifiedTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float range, uint32 specification)
{
    float r = range * range;
    WorldObject *wObj = NULL;
    Unit * u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : (m_caster->IsGameObject() ? castPtr<GameObject>(m_caster)->GetSummoner() : nullptr);
    for(WorldObject::InRangeMap::iterator itr = m_caster->GetInRangeMapBegin(); itr != m_caster->GetInRangeMapEnd(); itr++ )
    {
        if((wObj = itr->second) == NULL)
            continue;

        if(wObj->IsUnit())
        {
            if(!castPtr<Unit>(wObj)->isAlive())
                continue;
            if(GetSpellProto()->TargetCreatureType)
            {
                Unit* Target = castPtr<Unit>(wObj);
                if(uint32 creatureType = Target->GetCreatureType())
                {
                    if(((1<<(creatureType-1)) & GetSpellProto()->TargetCreatureType) == 0)
                        continue;
                } else continue;
            }
        } else if(wObj->IsGameObject() && !CanEffectTargetGameObjects(i))
            continue;

        if(!IsInrange(srcx, srcy, srcz, wObj, r))
            continue;

        if(castPtr<Unit>(m_caster) && wObj->IsUnit() )
        {
            if( sFactionSystem.CanEitherUnitAttack(castPtr<Unit>(m_caster), castPtr<Unit>(wObj), !GetSpellProto()->isSpellStealthTargetCapable()) )
                _AddTarget(castPtr<Unit>(wObj), i);
        } else _AddTarget(wObj, i);

        if(GetSpellProto()->MaxTargets && m_hitTargetCount >= GetSpellProto()->MaxTargets)
            break;
    }
}

void Spell::FillAllTargetsInArea(LocationVector & location,uint32 ind)
{
    FillAllTargetsInArea(ind,location.x,location.y,location.z,GetRadius(ind));
}

void Spell::FillAllTargetsInArea(float srcx,float srcy,float srcz,uint32 ind)
{
    FillAllTargetsInArea(ind,srcx,srcy,srcz,GetRadius(ind));
}

/// We fill all the targets in the area, including the stealth ed one's
void Spell::FillAllTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float range, bool includegameobjects)
{
    float r = range*range;
    uint32 placeholder = 0;
    WorldObject *wObj = NULL;
    std::vector<WorldObject*> ChainTargetContainer;
    bool canAffectGameObjects = CanEffectTargetGameObjects(i);
    Unit * u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : (m_caster->IsGameObject() ? castPtr<GameObject>(m_caster)->GetSummoner() : nullptr);
    for(WorldObject::InRangeMap::iterator itr = m_caster->GetInRangeMapBegin(); itr != m_caster->GetInRangeMapEnd(); itr++ )
    {
        if((wObj = itr->second) == NULL)
            continue;

        if(wObj->IsUnit())
        {
            Unit *uTarget = castPtr<Unit>(wObj);
            if(!uTarget->isAlive())
                continue;
            if( GetSpellProto()->TargetCreatureType && !(GetSpellProto()->TargetCreatureType & (1<<(uTarget->GetCreatureType()-1))))
                continue;
        } else if(wObj->IsGameObject() && !canAffectGameObjects)
            continue;

        if(!IsInrange(srcx, srcy, srcz, wObj, r))
            continue;

        if(castPtr<Unit>(m_caster) && wObj->IsUnit() )
        {
            if( sFactionSystem.CanEitherUnitAttack(castPtr<Unit>(m_caster), castPtr<Unit>(wObj), !GetSpellProto()->isSpellStealthTargetCapable()) )
            {
                ChainTargetContainer.push_back(wObj);
                placeholder++;
            }
        } else ChainTargetContainer.push_back(wObj);
    }

    if(GetSpellProto()->MaxTargets)
    {
        WorldObject* chaintarget = NULL;
        uint32 targetCount = GetSpellProto()->MaxTargets;
        while(targetCount && ChainTargetContainer.size())
        {
            placeholder = (rand()%ChainTargetContainer.size());
            chaintarget = ChainTargetContainer.at(placeholder);
            if(chaintarget == NULL)
                continue;

            if(chaintarget->IsUnit())
                _AddTarget(castPtr<Unit>(chaintarget), i);
            else _AddTarget(chaintarget, i);
            ChainTargetContainer.erase(ChainTargetContainer.begin()+placeholder);
            targetCount--;
        }
    }
    else
    {
        for(std::vector<WorldObject*>::iterator itr = ChainTargetContainer.begin(); itr != ChainTargetContainer.end(); itr++)
        {
            if((*itr)->IsUnit())
                _AddTarget(castPtr<Unit>(*itr), i);
            else _AddTarget(*itr, i);
        }
    }
    ChainTargetContainer.clear();
}

// We fill all the targets in the area, including the stealthed one's
void Spell::FillAllFriendlyInArea( uint32 i, float srcx, float srcy, float srcz, float range )
{
    float r = range*range;
    WorldObject *wObj = NULL;
    bool canAffectGameObjects = CanEffectTargetGameObjects(i);
    Unit * u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : (m_caster->IsGameObject() ? castPtr<GameObject>(m_caster)->GetSummoner() : nullptr);
    for(WorldObject::InRangeMap::iterator itr = m_caster->GetInRangeMapBegin(); itr != m_caster->GetInRangeMapEnd(); itr++ )
    {
        if((wObj = itr->second) == NULL)
            continue;

        if(wObj->IsUnit())
        {
            Unit *uTarget = castPtr<Unit>(wObj);
            if( !uTarget->isAlive() || (uTarget->IsCreature() && castPtr<Creature>(uTarget)->IsTotem()))
                continue;
            if( GetSpellProto()->TargetCreatureType && !(GetSpellProto()->TargetCreatureType & (1<<(uTarget->GetCreatureType()-1))))
                continue;
        } else if(wObj->IsGameObject() && !canAffectGameObjects)
            continue;

        if( IsInrange( srcx, srcy, srcz, wObj, r ))
        {
            if(castPtr<Unit>(m_caster) && wObj->IsUnit() )
            {
                if( sFactionSystem.CanEitherUnitAttack(castPtr<Unit>(m_caster), castPtr<Unit>(wObj), !GetSpellProto()->isSpellStealthTargetCapable()) )
                    _AddTarget(castPtr<Unit>(wObj), i);
            }
            else _AddTarget(wObj, i);

            if( GetSpellProto()->MaxTargets && m_hitTargetCount >= GetSpellProto()->MaxTargets )
                break;
        }
    }
}

/// We fill all the gameobject targets in the area
void Spell::FillAllGameObjectTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float range)
{
    float r = range*range;

    for(WorldObject::InRangeSet::iterator itr = m_caster->GetInRangeGameObjectSetBegin(); itr != m_caster->GetInRangeGameObjectSetEnd(); itr++ )
    {
        if(GameObject *gObj = m_caster->GetInRangeObject<GameObject>(*itr))
        {
            if(!IsInrange( srcx, srcy, srcz, gObj, r ))
                continue;
            _AddTarget(gObj, i);
        }
    }
}

uint64 Spell::GetSinglePossibleEnemy(uint32 i,float prange)
{
    float rMin = GetSpellProto()->minRange[0], rMax = prange;
    if(rMax == 0.f)
    {
        rMax = GetSpellProto()->maxRange[0];
        if( GetSpellProto()->SpellGroupType && m_caster->IsUnit())
        {
            castPtr<Unit>(m_caster)->SM_FFValue(SMT_RADIUS,&rMax,GetSpellProto()->SpellGroupType);
            castPtr<Unit>(m_caster)->SM_PFValue(SMT_RADIUS,&rMax,GetSpellProto()->SpellGroupType);
        }
    }

    float srcx = m_caster->GetPositionX(), srcy = m_caster->GetPositionY(), srcz = m_caster->GetPositionZ();
    for( WorldObject::InRangeSet::iterator itr = m_caster->GetInRangeUnitSetBegin(); itr != m_caster->GetInRangeUnitSetEnd(); itr++ )
    {
        Unit *unit = m_caster->GetInRangeObject<Unit>(*itr);
        if(!unit->isAlive())
            continue;
        if( GetSpellProto()->TargetCreatureType && (!(1<<(unit->GetCreatureType()-1) & GetSpellProto()->TargetCreatureType)))
            continue;
        if(!IsInrange(srcx,srcy,srcz, unit, rMax, rMin))
            continue;
        if(!sFactionSystem.isAttackable(m_caster, unit,!GetSpellProto()->isSpellStealthTargetCapable()))
            continue;
        if(_DidHit(i, unit) != SPELL_DID_HIT_SUCCESS)
            continue;
        return unit->GetGUID();
    }
    return 0;
}

uint64 Spell::GetSinglePossibleFriend(uint32 i,float prange)
{
    float rMin = GetSpellProto()->minRange[1], rMax = prange;
    if(rMax == 0.f)
    {
        rMax = GetSpellProto()->maxRange[1];
        if( GetSpellProto()->SpellGroupType && m_caster->IsUnit())
        {
            castPtr<Unit>(m_caster)->SM_FFValue(SMT_RADIUS,&rMax,GetSpellProto()->SpellGroupType);
            castPtr<Unit>(m_caster)->SM_PFValue(SMT_RADIUS,&rMax,GetSpellProto()->SpellGroupType);
        }
    }

    float srcx = m_caster->GetPositionX(), srcy = m_caster->GetPositionY(), srcz = m_caster->GetPositionZ();
    for(WorldObject::InRangeSet::iterator itr = m_caster->GetInRangeUnitSetBegin(); itr != m_caster->GetInRangeUnitSetEnd(); itr++ )
    {
        Unit *unit = m_caster->GetInRangeObject<Unit>(*itr);
        if(!unit->isAlive())
            continue;
        if( GetSpellProto()->TargetCreatureType && (!(1<<(unit->GetCreatureType()-1) & GetSpellProto()->TargetCreatureType)))
            continue;
        if(!IsInrange(srcx,srcy,srcz, unit, rMax, rMin))
            continue;
        if(!sFactionSystem.isFriendly(m_caster, unit))
            continue;
        if(_DidHit(i, unit) != SPELL_DID_HIT_SUCCESS)
            continue;
        return unit->GetGUID();
    }
    return 0;
}

uint8 Spell::_DidHit(uint32 index, Unit* target, uint8 *reflectout)
{
    //note resistchance is vise versa, is full hit chance
    if( target == NULL )
        return SPELL_DID_HIT_MISS;

    /************************************************************************/
    /* Can't resist non-unit                                                */
    /************************************************************************/
    if(!m_caster->IsUnit())
        return SPELL_DID_HIT_SUCCESS;

    /************************************************************************/
    /* Can't resist non-unit and can't miss your own spells                 */
    /************************************************************************/
    if(!m_caster->IsUnit() || m_caster == target)
        return SPELL_DID_HIT_SUCCESS;

    /************************************************************************/
    /* Check if the unit is evading                                      */
    /************************************************************************/
    /*if(target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->GetAIInterface()->getAIState() == STATE_EVADE)
        return SPELL_DID_HIT_EVADE;*/

    /*************************************************************************/
    /* Check if the target is immune to this mechanic                       */
    /*************************************************************************/
    if(target->GetMechanicDispels(GetSpellProto()->MechanicsType))
        return SPELL_DID_HIT_IMMUNE; // Moved here from Spell::CanCast

    /************************************************************************/
    /* Check if the target has a % resistance to this mechanic            */
    /************************************************************************/
    if( GetSpellProto()->MechanicsType < MECHANIC_COUNT)
    {
        float res = target->GetMechanicResistPCT(Spell::GetMechanic(m_spellInfo));
        if( !GetSpellProto()->isSpellResistanceIgnorant() && Rand(res))
            return SPELL_DID_HIT_RESIST;
    }

    /************************************************************************/
    /* Check if the spell is a melee attack and if it was missed/parried    */
    /************************************************************************/
    if( GetSpellProto()->IsSpellWeaponSpell() )
    {
        uint32 _type = MELEE;
        if( GetType() == SPELL_DMG_TYPE_RANGED )
            _type = RANGED;
        else if (GetSpellProto()->reqOffHandWeapon())
            _type = OFFHAND;

        if(uint32 melee_test_result = castPtr<Unit>(m_caster)->GetSpellDidHitResult( (Unit*)target, _type, m_spellInfo ))
            return uint8(melee_test_result & 0xFF);
    }

    return castPtr<Unit>(m_caster)->GetSpellDidHitResult(index, (Unit*)target, this, reflectout);
}

bool Spell::GenerateTargets(SpellCastTargets *t)
{
    if(!m_caster->IsInWorld() || !m_caster->IsUnit())
        return false;

    bool result = false;

    for(uint32 i = 0; i < 3; ++i)
    {
        if(m_spellInfo->Effect[i] == 0)
            continue;
        uint32 TargetType = GetTargetType(m_spellInfo->EffectImplicitTargetA[i], i);

        //never get info from B if it is 0 :P
        if(m_spellInfo->EffectImplicitTargetB[i] != 0)
            TargetType |= GetTargetType(m_spellInfo->EffectImplicitTargetB[i], i);

        if(TargetType & (SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID))
        {
            t->m_targetMask |= TARGET_FLAG_UNIT;
            t->m_unitTarget = m_caster->GetGUID();
            result = true;
        }

        if(TargetType & SPELL_TARGET_NO_OBJECT)
        {
            t->m_targetMask = TARGET_FLAG_SELF;
            t->m_unitTarget = m_caster->GetGUID();
            result = true;
        }

        if(!(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE)))
        {
            if(TargetType & SPELL_TARGET_ANY_OBJECT)
            {
                if(m_caster->GetUInt64Value(UNIT_FIELD_TARGET))
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    if(WorldObject* target = m_caster->GetInRangeObject<WorldObject>(m_caster->GetUInt64Value(UNIT_FIELD_TARGET)))
                    {
                        if(target->IsUnit())
                        {
                            t->m_targetMask |= TARGET_FLAG_UNIT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                        else if(target->IsGameObject())
                        {
                            t->m_targetMask |= TARGET_FLAG_OBJECT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                    }
                    result = true;
                }
            }

            if(TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE)
            {
                if(m_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    if(WorldObject* target = m_caster->GetInRangeObject<WorldObject>(m_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)))
                    {
                        if(target->IsUnit())
                        {
                            t->m_targetMask |= TARGET_FLAG_UNIT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                        else if(target->IsGameObject())
                        {
                            t->m_targetMask |= TARGET_FLAG_OBJECT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                    }
                }
                else if(m_caster->GetUInt64Value(UNIT_FIELD_TARGET))
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    if(WorldObject* target = m_caster->GetInRangeObject<WorldObject>(m_caster->GetUInt64Value(UNIT_FIELD_TARGET)))
                    {
                        if(target->IsUnit())
                        {
                            t->m_targetMask |= TARGET_FLAG_UNIT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                        else if(target->IsGameObject())
                        {
                            t->m_targetMask |= TARGET_FLAG_OBJECT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                    }
                    result = true;
                }
                else if(m_caster->IsCreature() && m_caster->IsTotem())
                {
                    if(Unit* target = m_caster->GetInRangeObject<Unit>(GetSinglePossibleEnemy(i)))
                    {
                        t->m_targetMask |= TARGET_FLAG_UNIT;
                        t->m_unitTarget = target->GetGUID();
                    }
                }
            }

            if(TargetType & SPELL_TARGET_REQUIRE_FRIENDLY)
            {
                if(Unit* target = m_caster->GetInRangeObject<Unit>(GetSinglePossibleFriend(i)))
                {
                    t->m_targetMask |= TARGET_FLAG_UNIT;
                    t->m_unitTarget = target->GetGUID();
                    result = true;
                }
                else
                {
                    t->m_targetMask |= TARGET_FLAG_UNIT;
                    t->m_unitTarget = m_caster->GetGUID();
                    result = true;
                }
            }
        }

        if(TargetType & SPELL_TARGET_AREA_RANDOM)
        {
            //we always use radius(0) for some reason
            uint8 attempts = 0;
            do
            {
                //prevent deadlock
                ++attempts;
                if(attempts > 10)
                    return false;

                float r = RandomFloat(GetRadius(0));
                float ang = RandomFloat(M_PI * 2);
                t->m_dest.x = m_caster->GetPositionX() + (cosf(ang) * r);
                t->m_dest.y = m_caster->GetPositionY() + (sinf(ang) * r);
                t->m_dest.z = m_caster->GetMapHeight(t->m_dest.x, t->m_dest.y, m_caster->GetPositionZ() + 2.0f);
                t->m_targetMask = TARGET_FLAG_DEST_LOCATION;
            }
            while(sWorld.Collision && !sVMapInterface.CheckLOS(m_caster->GetMapId(), m_caster->GetInstanceID(), m_caster->GetPhaseMask(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), t->m_dest.x, t->m_dest.y, t->m_dest.z));
            result = true;
        }
        else if(TargetType & SPELL_TARGET_AREA)  //targetted aoe
        {
            if(TargetType & SPELL_TARGET_REQUIRE_FRIENDLY)
            {
                t->m_targetMask |= TARGET_FLAG_DEST_LOCATION;
                t->m_dest = m_caster->GetPosition();
                result = true;
            }
            else if(m_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)) //spells like blizzard, rain of fire
            {
                if(WorldObject* target = m_caster->GetInRangeObject<WorldObject>(m_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)))
                {
                    t->m_targetMask |= TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_UNIT;
                    t->m_unitTarget = target->GetGUID();
                    t->m_dest = target->GetPosition();
                }
                result = true;
            }
        }
        else if(TargetType & SPELL_TARGET_AREA_SELF)
        {
            t->m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
            t->m_dest = t->m_src = m_caster->GetPosition();
            result = true;
        }

        if(TargetType & SPELL_TARGET_AREA_CHAIN)
        {
            if(!(TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE))
            {
                t->m_targetMask |= TARGET_FLAG_UNIT;
                t->m_unitTarget = m_caster->GetGUID();
                result = true;
            }
        }
    }
    return result;
}

uint8 Spell::prepare(SpellCastTargets *targets, bool triggered)
{
    uint8 ccr;
    if( m_caster->IsPlayer() && (GetSpellProto()->Id == 51514 || GetSpellProto()->NameHash == SPELL_HASH_ARCANE_SHOT || GetSpellProto()->NameHash == SPELL_HASH_MIND_FLAY))
    {
        targets->m_unitTarget = 0;
        GenerateTargets( targets );
    }

    m_targets = *targets;
    m_triggeredSpell = triggered;

    // Call base spell preparations
    BaseSpell::_Prepare();

    if( m_triggeredSpell )
        cancastresult = SPELL_CANCAST_OK;
    else cancastresult = CanCast(false);
    ccr = cancastresult;

    if( cancastresult != SPELL_CANCAST_OK )
    {
        SendCastResult( cancastresult );

        if( m_triggeredByAura )
            SendChannelUpdate( 0 );

        finish();
        return ccr;
    }

    // Handle triggered spells here that aren't channeled spells
    if( m_triggeredSpell && !GetSpellProto()->IsSpellChannelSpell())
    {
        cast( false );
        return ccr;
    }

    if( !HasPower() )
    {
        SendCastResult(SPELL_FAILED_NO_POWER);
        return SPELL_FAILED_NO_POWER;
    }

    SendSpellStart();

    // start cooldown handler
    if( m_caster->IsPlayer() )
        AddStartCooldown();

    if( m_caster->IsUnit() && m_timer > 0 )
        castPtr<Unit>(m_caster)->interruptAttackTimer(m_timer+1000);

    // aura state removal
    if( m_caster->IsUnit() && GetSpellProto()->CasterAuraState && GetSpellProto()->CasterAuraState != AURASTATE_FLAG_JUDGEMENT )
        castPtr<Unit>(m_caster)->RemoveFlag( UNIT_FIELD_AURASTATE, GetSpellProto()->CasterAuraState );

    m_spellState = SPELL_STATE_PREPARING;

    // instant cast(or triggered) and not channeling
    if( m_caster->IsUnit() && ( m_castTime > 0 || GetSpellProto()->IsSpellChannelSpell() ) && !m_triggeredSpell  )
    {
        m_castPositionX = m_caster->GetPositionX();
        m_castPositionY = m_caster->GetPositionY();
        m_castPositionZ = m_caster->GetPositionZ();
        castPtr<Unit>(m_caster)->CastSpell( this );
    } else cast( false );

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
            if(m_caster->IsPlayer())
            {
                Unit* pTarget = castPtr<Player>(m_caster)->GetMapInstance()->GetUnit(m_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT));
                if(!pTarget)
                    pTarget = castPtr<Player>(m_caster)->GetMapInstance()->GetUnit(castPtr<Player>(m_caster)->GetSelection());

                if(pTarget)
                    pTarget->RemoveAura(GetSpellProto()->Id, m_caster->GetGUID());

                if(m_AreaAura)//remove of blizz and shit like this
                {
                    DynamicObject* dynObj = m_caster->GetMapInstance()->GetDynamicObject(m_caster->GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT));
                    if(dynObj)
                    {
                        dynObj->RemoveFromWorld();
                        dynObj->Destruct();
                        dynObj = NULL;
                    }
                }

                if(castPtr<Player>(m_caster)->GetSummonedObject())
                {
                    if(castPtr<Player>(m_caster)->GetSummonedObject()->IsInWorld())
                        castPtr<Player>(m_caster)->GetSummonedObject()->RemoveFromWorld();
                    // for now..
                    ASSERT(castPtr<Player>(m_caster)->GetSummonedObject()->GetTypeId() == TYPEID_GAMEOBJECT);
                    castPtr<GameObject>(castPtr<Player>(m_caster)->GetSummonedObject())->Destruct();
                    castPtr<Player>(m_caster)->SetSummonedObject(NULL);
                }

                if(m_timer > 0)
                {
                    castPtr<Player>(m_caster)->interruptAttackTimer(-m_timer);
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
    if( m_caster->IsPlayer() && !castPtr<Player>(m_caster)->CooldownCheat)
        castPtr<Player>(m_caster)->Cooldown_Add( m_spellInfo, NULL );
}

void Spell::AddStartCooldown()
{
    if( m_caster->IsPlayer() && !castPtr<Player>(m_caster)->CastTimeCheat)
        castPtr<Player>(m_caster)->Cooldown_AddStart( m_spellInfo );
}

void Spell::cast(bool check)
{
    if( duelSpell && ( m_caster->IsPlayer() && castPtr<Player>(m_caster)->GetDuelState() != DUEL_STATE_STARTED ) )
    {
        // Can't cast that!
        SendInterrupted( SPELL_FAILED_TARGET_FRIENDLY );
        finish();
        return;
    }

    sLog.Debug("Spell","Cast %u, Unit: %u", GetSpellProto()->Id, m_caster->GetLowGUID());

    // Set the base ms time to now
    m_MSTimeToAddToTravel = getMSTime();

    cancastresult = check ? CanCast(true) : SPELL_CANCAST_OK;

    if(cancastresult == SPELL_CANCAST_OK)
    {
        if (m_caster->IsPlayer() && !m_triggeredSpell && m_caster->IsInWorld() && GUID_HIPART(m_targets.m_unitTarget) == HIGHGUID_TYPE_UNIT)
            sQuestMgr.OnPlayerCast(castPtr<Player>(m_caster), GetSpellProto()->Id, m_targets.m_unitTarget);

        // trigger on next attack
        if( GetSpellProto()->isNextMeleeAttack1() )
        {
            if(!m_triggeredSpell)
            {
                // check power
                if(!HasPower())
                {
                    SendInterrupted(SPELL_FAILED_NO_POWER);
                    SendCastResult(SPELL_FAILED_NO_POWER);
                    finish();
                    return;
                }
            }
            else if(!TakePower()) // this is the actual spell cast
            {
                SendInterrupted(SPELL_FAILED_NO_POWER);
                SendCastResult(SPELL_FAILED_NO_POWER);
                finish();
                return;
            }
        }
        else if(!m_triggeredSpell)
        {
            if(!TakePower())
            {
                SendInterrupted(SPELL_FAILED_NO_POWER);
                SendCastResult(SPELL_FAILED_NO_POWER);
                finish();
                return;
            }
        }

        for(uint32 i = 0; i < 3; i++)
        {
            uint32 TargetType = 0;

            // Fill from A regardless
            TargetType |= GetTargetType(m_spellInfo->EffectImplicitTargetA[i], i);

            //never get info from B if it is 0 :P
            if(m_spellInfo->EffectImplicitTargetB[i] != 0)
                TargetType |= GetTargetType(m_spellInfo->EffectImplicitTargetB[i], i);

            if(TargetType & SPELL_TARGET_AREA_CURTARGET)
            {
                //this just forces dest as the targets location :P
                if(WorldObject* target = m_caster->GetInRangeObject(m_targets.m_unitTarget))
                {
                    m_targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
                    m_targets.m_dest = target->GetPosition();
                    break;
                }
            }
        }

        SendCastResult(cancastresult);
        if(cancastresult != SPELL_CANCAST_OK)
        {
            finish();
            return;
        }

        m_isCasting = true;

        if(!m_triggeredSpell)
            AddCooldown();

        if( m_caster->IsPlayer() )
        {
            // Arathi Basin opening spell, remove stealth, invisibility, etc.
            // hacky but haven't found a better way that works
            // Note: Same stuff but for picking flags is over AddAura
            if (castPtr<Player>(m_caster)->m_bg)
            {
                // SOTA Gameobject spells
                if(castPtr<Player>(m_caster)->m_bg->GetType() == BATTLEGROUND_STRAND_OF_THE_ANCIENTS)
                {
                    StrandOfTheAncients* sota = (StrandOfTheAncients*)castPtr<Player>(m_caster)->m_bg;
                    // Transporter platforms
                    if(m_spellInfo->Id == 54640)
                        sota->OnPlatformTeleport(castPtr<Player>(m_caster));
                }

                // warsong gulch & eye of the storm flag pickup check
                // also includes check for trying to cast stealth/etc while you have the flag
                switch(m_spellInfo->Id)
                {
                case 21651: // Arathi Basin opening spell, remove stealth, invisibility, etc.
                case 23333: // if we're picking up the flag remove the buffs
                case 23335:
                case 34976:
                    /*castPtr<Player>(m_caster)->m_AuraInterface.RemoveAllAurasWithModType(SPELL_AURA_MOD_STEALTH);
                    castPtr<Player>(m_caster)->m_AuraInterface.RemoveAllAurasWithModType(SPELL_AURA_MOD_INVISIBILITY);*/
                    castPtr<Player>(m_caster)->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_DIVINE_SHIELD, false);
                    castPtr<Player>(m_caster)->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_DIVINE_PROTECTION, false);
                    castPtr<Player>(m_caster)->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_BLESSING_OF_PROTECTION, false);
                    break;
                    // cases for stealth - etc
                    // we can cast the spell, but we drop the flag (if we have it)
                case 1784:      // Stealth rank 1
                case 1785:      // Stealth rank 2
                case 1786:      // Stealth rank 3
                case 1787:      // Stealth rank 4
                case 5215:      // Prowl rank 1
                case 6783:      // Prowl rank 2
                case 9913:      // Prowl rank 3
                case 498:       // Divine protection
                case 5573:      // Unknown spell
                case 642:       // Divine shield
                case 1020:      // Unknown spell
                case 1022:      // Hand of Protection rank 1 (ex blessing of protection)
                case 5599:      // Hand of Protection rank 2 (ex blessing of protection)
                case 10278:     // Hand of Protection rank 3 (ex blessing of protection)
                case 1856:      // Vanish rank 1
                case 1857:      // Vanish rank 2
                case 26889:     // Vanish rank 3
                case 45438:     // Ice block
                case 20580:     // Unknown spell
                case 58984:     // Shadowmeld
                case 17624:     // Petrification-> http://www.wowhead.com/?spell=17624
                case 66:        // Invisibility
                    {
                        if(castPtr<Player>(m_caster)->m_bg->GetType() == BATTLEGROUND_WARSONG_GULCH)
                        {
                            if(castPtr<Player>(m_caster)->GetTeam() == 0)
                                castPtr<Player>(m_caster)->RemoveAura(23333);    // ally player drop horde flag if they have it
                            else castPtr<Player>(m_caster)->RemoveAura(23335);    // horde player drop ally flag if they have it
                        }
                        if(castPtr<Player>(m_caster)->m_bg->GetType() == BATTLEGROUND_EYE_OF_THE_STORM)
                            castPtr<Player>(m_caster)->RemoveAura(34976);    // drop the flag
                    }break;
                }
            }
        }

        bool isNextMeleeAttack1 = m_spellInfo->isNextMeleeAttack1();
        if(!isNextMeleeAttack1 || m_triggeredSpell)  //on next attack
        {
            SendSpellGo();

            //******************** SHOOT SPELLS ***********************
            if(GetSpellProto()->isSpellRangedSpell() && (m_caster->IsPlayer()) && m_caster->IsInWorld())
            {
                /// Part of this function contains a hack fix
                /// hack fix for shoot spells, should be some other resource for it
                //p_caster->SendSpellCoolDown(GetSpellProto()->Id, GetSpellProto()->RecoveryTime ? GetSpellProto()->RecoveryTime : 2300);
                WorldPacket data(SMSG_SPELL_COOLDOWN, 14);
                data << castPtr<Player>(m_caster)->GetGUID();
                data << uint8(0);
                data << GetSpellProto()->Id;
                data << uint32(GetSpellProto()->RecoveryTime ? GetSpellProto()->RecoveryTime : 2300);
                castPtr<Player>(m_caster)->GetSession()->SendPacket(&data);
            }
            else if( GetSpellProto()->IsSpellChannelSpell() && !m_triggeredSpell && GetSpellProto()->Id != 50589 ) // Note: [Warlock] Immolation Aura somehow has these flags, but it's not channeled
            {
                /*
                Channeled spells are handled a little differently. The five second rule starts when the spell's channeling starts; i.e. when you pay the mana for it.
                The rule continues for at least five seconds, and longer if the spell is channeled for more than five seconds. For example,
                Mind Flay channels for 3 seconds and interrupts your regeneration for 5 seconds, while Tranquility channels for 10 seconds
                and interrupts your regeneration for the full 10 seconds.
                */
                m_spellState = SPELL_STATE_CASTING;
                SendChannelStart(GetDuration());
                if( m_caster->IsPlayer() && castPtr<Player>(m_caster)->GetSelection() )
                {
                    //Use channel interrupt flags here
                    if(m_targets.m_targetMask == TARGET_FLAG_DEST_LOCATION || m_targets.m_targetMask == TARGET_FLAG_SOURCE_LOCATION)
                        castPtr<Player>(m_caster)->SetChannelSpellTargetGUID(castPtr<Player>(m_caster)->GetSelection());
                    else if(castPtr<Player>(m_caster)->GetSelection() == m_caster->GetGUID())
                    {
                        if(m_targets.m_unitTarget)
                            castPtr<Player>(m_caster)->SetChannelSpellTargetGUID(m_targets.m_unitTarget);
                        else castPtr<Player>(m_caster)->SetChannelSpellTargetGUID(castPtr<Player>(m_caster)->GetSelection());
                    }
                    else
                    {
                        if(castPtr<Player>(m_caster)->GetSelection())
                            castPtr<Player>(m_caster)->SetChannelSpellTargetGUID(castPtr<Player>(m_caster)->GetSelection());
                        else if(m_targets.m_unitTarget)
                            castPtr<Player>(m_caster)->SetChannelSpellTargetGUID(m_targets.m_unitTarget);
                        else
                        {
                            m_isCasting = false;
                            cancel();
                            return;
                        }
                    }
                }
            }

            //if(!m_projectileWait)
            {
                std::pair<WorldObject*, bool> casterTarget = std::make_pair<WorldObject*, bool>(NULL, false);
                // if the spell is not reflected
                for(uint8 i = 0; i < 3; i++)
                {
                    if(GetSpellProto()->Effect[i] == 0)
                        continue;

                    FillTargetMap(i);
                    if(!m_effectTargetMaps[i].empty())
                        continue;
                    // If for some reason we have a unitTarget but we aren't the target and map is empty, skip processing
                    if(!m_targets.m_unitTarget.empty() && m_targets.m_unitTarget != m_caster->GetGUID())
                        continue;
                    if(!CanHandleSpellEffect(i))
                        continue;
                    if(GetSpellProto()->Effect[i] == SPELL_EFFECT_SUMMON
                        || GetSpellProto()->Effect[i] == SPELL_EFFECT_TRIGGER_SPELL
                        || GetSpellProto()->Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                        continue;
                    HandleEffects(i, m_caster);
                    casterTarget.first = m_caster;
                    casterTarget.second = m_spellInfo->EffectApplyAuraName[i] > 0;
                }

                // Specific spells need to have effect 0 on all targets first, then the same for 1 and 2
                bool inlineEffects = m_spellInfo->inline_effects;
                if(inlineEffects == true)
                {
                    for(uint8 i = 0; i < 3; i++)
                    {
                        if(!CanHandleSpellEffect(i))
                            continue;
                        if(GetSpellProto()->Effect[i] == SPELL_EFFECT_SUMMON
                            || GetSpellProto()->Effect[i] == SPELL_EFFECT_TRIGGER_SPELL
                            || GetSpellProto()->Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                            continue;
                        for(SpellTargetMap::iterator itr = m_effectTargetMaps[i].begin(); itr != m_effectTargetMaps[i].end(); itr++)
                        {
                            if(itr->second.HitResult != SPELL_DID_HIT_SUCCESS)
                                continue;

                            if(WorldObject *target = m_caster->GetInRangeObject(itr->first))
                                HandleEffects(i, target);
                        }
                    }
                }

                for(SpellTargetMap::iterator itr = m_fullTargetMap.begin(); itr != m_fullTargetMap.end(); itr++)
                {
                    uint8 hitResult = itr->second.HitResult;
                    if(hitResult != SPELL_DID_HIT_SUCCESS)
                        continue;

                    if(WorldObject *target = m_caster->GetInRangeObject(itr->first))
                    {
                        // If we're not inline effect handling, do the effects here
                        if(inlineEffects == false)
                        {
                            for(uint8 i = 0; i < 3; i++)
                            {
                                if((itr->second.EffectMask & uint8(1<<i)) == 0)
                                    continue;
                                if(!CanHandleSpellEffect(i))
                                    continue;
                                if(GetSpellProto()->Effect[i] == SPELL_EFFECT_SUMMON
                                    || GetSpellProto()->Effect[i] == SPELL_EFFECT_TRIGGER_SPELL
                                    || GetSpellProto()->Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                                    continue;
                                HandleEffects(i, target);
                            }
                        }

                        if(!target->IsUnit())
                            continue;
                        Unit *unitTarget = castPtr<Unit>(target);
                        if(GetSpellProto()->TargetAuraState)
                            unitTarget->RemoveFlag(UNIT_FIELD_AURASTATE, uint32(1) << (GetSpellProto()->TargetAuraState - 1) );

                        // Add our aura after handling the effects
                        for(uint8 i = 0; i < 3; i++)
                        {
                            if(m_spellInfo->EffectApplyAuraName[i] && (itr->second.EffectMask & uint8(1<<i)) > 0)
                            {
                                HandleAddAura(unitTarget);
                                break;
                            }
                        }
                    }
                }

                if(casterTarget.first && casterTarget.first->IsUnit())
                {
                    Unit *unitTarget = castPtr<Unit>(casterTarget.first);
                    if(GetSpellProto()->TargetAuraState)
                        unitTarget->RemoveFlag(UNIT_FIELD_AURASTATE, uint32(1) << (GetSpellProto()->TargetAuraState - 1) );

                    // Add our aura after handling the effects
                    if(casterTarget.second)
                        HandleAddAura(unitTarget);
                }

                //Handle remaining effects for which we did not find targets.
                for( uint8 x = 0; x < 3; ++x )
                {
                    if(GetSpellProto()->Effect[x] == 0)
                        continue;
                    if(!CanHandleSpellEffect(x))
                        continue;

                    switch (GetSpellProto()->Effect[x])
                    {   // Target ourself for these effects
                    case SPELL_EFFECT_TRIGGER_SPELL:
                    case SPELL_EFFECT_SUMMON:
                    case SPELL_EFFECT_PERSISTENT_AREA_AURA:
                        {
                            HandleEffects(x, m_caster);
                        }break;
                    }
                }
            }// else CalcDestLocationHit();

            // we're much better to remove this here, because otherwise spells that change powers etc,
            // don't get applied.
            if( m_caster->IsUnit() )
                castPtr<Unit>(m_caster)->m_AuraInterface.RemoveAllAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, GetSpellProto()->Id);

            m_isCasting = false;

            if(m_spellState != SPELL_STATE_CASTING)
                finish();
        }
        else //this shit has nothing to do with instant, this only means it will be on NEXT melee hit
        {
            // we're much better to remove this here, because otherwise spells that change powers etc,
            // don't get applied.
            if(m_caster->IsUnit() && !m_triggeredSpell && !m_triggeredByAura)
                castPtr<Unit>(m_caster)->m_AuraInterface.RemoveAllAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, GetSpellProto()->Id);

            m_isCasting = false;
            SendCastResult(cancastresult);
            /*if(m_caster->IsPlayer())
                castPtr<Player>(m_caster)->SetOnMeleeSpell(GetSpellProto()->Id, m_castNumber);*/
            finish();
            return;
        }
    }
    else
    {
        // cancast failed
        SendCastResult(cancastresult);
        SendInterrupted(cancastresult);
        finish();
    }
}

void Spell::AddTime(uint32 type)
{
    if(m_caster->IsPlayer())
    {
        Player *p_caster = castPtr<Player>(m_caster);
        if( GetSpellProto()->InterruptFlags & CAST_INTERRUPT_ON_DAMAGE_TAKEN)
        {
            cancel();
            return;
        }

        if( GetSpellProto()->SpellGroupType)
        {
            float ch = 0;
            p_caster->SM_FFValue(SMT_NONINTERRUPT, &ch, GetSpellProto()->SpellGroupType);
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
            data << p_caster->GetGUID();
            data << uint32(delay);
            p_caster->SendMessageToSet(&data, true);
            p_caster->interruptAttackTimer(delay);
        }
        else if(GetSpellProto()->IsSpellChannelSpell())
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
    updatePosition(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ());

    if(m_cancelled)
    {
        cancel();
        return;
    }

//  printf("spell::update m_timer %u, difftime %d, newtime %d\n", m_timer, difftime, m_timer-difftime);
    switch(m_spellState)
    {
    case SPELL_STATE_PREPARING:
        {
            if((int32)difftime >= m_timer)
            {
                m_timer = 0;
                cast(true);
            }
            else
                m_timer -= difftime;
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

void Spell::_UpdateChanneledSpell(uint32 difftime)
{

}

void Spell::updatePosition(float x, float y, float z)
{
    if((GetSpellProto()->InterruptFlags & CAST_INTERRUPT_ON_MOVEMENT) &&
        ( m_castPositionX != x || m_castPositionY != y || m_castPositionZ != z))
    {
        if(m_caster->IsUnit())
        {
            if(castPtr<Unit>(m_caster)->HasNoInterrupt() == 0 && GetSpellProto()->EffectMechanic[1] != 14)
            {
                cancel();
                return;
            }
        }
    }

    if((m_spellInfo->IsSpellChannelSpell() && m_spellInfo->ChannelInterruptFlags & CHANNEL_INTERRUPT_ON_MOVEMENT) &&
        ( m_castPositionX != x || m_castPositionY != y || m_castPositionZ != z))
    {
        if(m_caster->IsUnit())
        {
            cancel();
            return;
        }
    }
}

void Spell::finish()
{
    if(m_spellState == SPELL_STATE_FINISHED)
        return;

    m_spellState = SPELL_STATE_FINISHED;

    //enable pvp when attacking another player with spells
    if( m_caster->IsPlayer() )
    {
        if(m_Delayed && castPtr<Player>(m_caster)->IsInWorld())
        {
            Unit* pTarget = NULL;
            if((pTarget = castPtr<Player>(m_caster)->GetMapInstance()->GetUnit(m_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))) == NULL)
                pTarget = castPtr<Player>(m_caster)->GetMapInstance()->GetUnit(castPtr<Player>(m_caster)->GetSelection());
            if(pTarget) pTarget->RemoveAura(GetSpellProto()->Id, m_caster->GetGUID());
        }
    }

    if( m_caster->IsPlayer() && GetSpellProto()->HasEffect(SPELL_EFFECT_SUMMON_OBJECT) )
        castPtr<Player>(m_caster)->SetSummonedObject(NULL);

    /*
    We set current spell only if this spell has cast time or is channeling spell
    otherwise it's instant spell and we delete it right after completion
    */
    if( m_caster->IsUnit() )
    {
        if(!m_triggeredSpell && (GetSpellProto()->IsSpellChannelSpell() || m_castTime > 0))
            castPtr<Unit>(m_caster)->SetCurrentSpell(NULL);
    }

    if(m_caster->IsPlayer())
    {
        if(castPtr<Player>(m_caster)->CooldownCheat && GetSpellProto())
            castPtr<Player>(m_caster)->ClearCooldownForSpell(GetSpellProto()->Id);

        if( m_ForceConsumption || ( cancastresult == SPELL_CANCAST_OK ) )
            RemoveItems();
    }

    if(!m_projectileWait)
        Destruct();
}

bool Spell::HasPower()
{
    int32 powerField = 0, cost = CalculateCost(powerField);
    if(powerField == -1)
        return false;
    if (cost <= 0)
    {
        m_usesMana = false; // no mana regen interruption for free spells
        return true;
    }
    // Unit has enough power (needed for creatures)
    return (cost <= m_caster->GetUInt32Value(powerField));
}

bool Spell::TakePower()
{
    if(!m_caster->IsUnit())
        return true;

    int32 powerField = 0, cost = CalculateCost(powerField);
    if(powerField == -1)
        return false;
    if (cost <= 0)
    {
        m_usesMana = false; // no mana regen interruption for free spells
        return true;
    }

    Unit *u_caster = castPtr<Unit>(m_caster);
    if(GetSpellProto()->powerType == POWER_TYPE_RUNIC)
    {
        if(u_caster->IsPlayer())
            castPtr<Player>(u_caster)->UseRunes(m_spellInfo->runeCost[0], m_spellInfo->runeCost[1], m_spellInfo->runeCost[2]);
        if(uint32 runePowerGain = m_spellInfo->runeGain)
            u_caster->SetPower(POWER_TYPE_RUNIC, u_caster->GetPower(POWER_TYPE_RUNIC) + runePowerGain);
    }
    else
    {
        int32 currentPower = m_caster->GetUInt32Value(powerField);
        if(powerField == UNIT_FIELD_HEALTH)
        {
            if(cost <= currentPower) // Unit has enough power (needed for creatures)
            {
                m_caster->DealDamage(u_caster, cost, 0, 0, 0,true);
                return true;
            }
        }
        else
        {
            if(cost <= currentPower) // Unit has enough power (needed for creatures)
            {
                if( GetSpellProto()->powerType == POWER_TYPE_MANA )
                    if(m_spellInfo->IsSpellChannelSpell()) // Client only accepts channels
                        u_caster->DelayPowerRegeneration(GetDuration());

                u_caster->SetPower(GetSpellProto()->powerType, currentPower - cost);
                return true;
            }
        }
    }
    return false;
}

int32 Spell::CalculateCost(int32 &powerField)
{
    // Initialize powerfield
    powerField = -1;
    // Only units use power, items do not
    if(!m_caster->IsUnit())
        return (powerField = 0);
    // Trainers can always cast, same with players with powercheat
    if(castPtr<Unit>(m_caster)->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER))
        return (powerField = 0);
    if((powerField = castPtr<Unit>(m_caster)->GetPowerFieldForType(GetSpellProto()->powerType)) == UNIT_END)
        return -1;
    else if(GetSpellProto()->powerType == POWER_TYPE_MANA)
        m_usesMana = true;

    int32 cost = m_caster->GetSpellBaseCost(m_spellInfo);
    int32 currentPower = m_caster->GetUInt32Value(powerField);
    if( m_caster->IsUnit() )
    {
        if( GetSpellProto()->isExhaustingPowerSpell() ) // Uses %100 mana
            return m_caster->GetUInt32Value(powerField);

        cost += float2int32(float(cost)* castPtr<Unit>(m_caster)->GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + GetSpellProto()->School));
    }

    //apply modifiers
    if( GetSpellProto()->SpellGroupType && castPtr<Unit>(m_caster))
    {
        castPtr<Unit>(m_caster)->SM_FIValue(SMT_COST,&cost,GetSpellProto()->SpellGroupType);
        castPtr<Unit>(m_caster)->SM_PIValue(SMT_COST,&cost,GetSpellProto()->SpellGroupType);
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
    if( castPtr<Unit>(m_caster) && castPtr<Unit>(m_caster)->GetCurrentSpell() != NULL && castPtr<Unit>(m_caster)->GetCurrentSpell() != this )
        return SPELL_FAILED_SPELL_IN_PROGRESS;

    /* Spells for the zombie event */
    if( m_caster->IsUnit() && castPtr<Unit>(m_caster)->GetShapeShift() == FORM_ZOMBIE && !( ((uint32)1 << (castPtr<Unit>(m_caster)->GetShapeShift()-1)) & GetSpellProto()->RequiredShapeShift  ))
    {
        sLog.outDebug("Invalid shapeshift: %u", GetSpellProto()->RequiredShapeShift);
        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    if((m_castTime || m_spellInfo->IsSpellChannelSpell()) && m_caster->IsUnit() && castPtr<Unit>(m_caster)->GetMovementInterface()->isMoving())
        if((GetSpellProto()->InterruptFlags & CAST_INTERRUPT_ON_MOVEMENT) || m_spellInfo->ChannelInterruptFlags & CHANNEL_INTERRUPT_ON_MOVEMENT)
            return SPELL_FAILED_MOVING;

    if(m_caster->IsInWorld())
    {
        if( Unit *target = m_caster->GetMapInstance()->GetUnit( m_targets.m_unitTarget ) )
        {
            if( GetSpellProto()->Id == 48788 && target->GetHealthPct() == 100)
                return SPELL_FAILED_ALREADY_AT_FULL_HEALTH;

            // GM flagged players should be immune to other players' casts, but not their own.
            if(target->IsPlayer() && (m_caster->GetTypeId() == TYPEID_ITEM ? (castPtr<Item>(m_caster)->GetOwner() != target) : (m_caster != target)) && castPtr<Player>(target)->bGMTagOn)
                return SPELL_FAILED_BM_OR_INVISGOD;

            //you can't mind control someone already mind controlled
            if (GetSpellProto()->NameHash == SPELL_HASH_MIND_CONTROL && target->m_AuraInterface.GetAuraSpellIDWithNameHash(SPELL_HASH_MIND_CONTROL))
                return SPELL_FAILED_CANT_BE_CHARMED;

            //these spells can be cast only on certain objects. Otherwise cool exploit
            //Most of this things goes to spell_forced_target table
            switch (GetSpellProto()->Id)
            {
                case 27907:// Disciplinary Rod
                {
                    if( target->IsPlayer() )
                        return SPELL_FAILED_BAD_TARGETS;
                }break;
            }
        }
    }

    if( m_caster->IsPlayer() )
    {
        Player *p_caster = castPtr<Player>(m_caster);
        if( GetSpellProto()->Id == 51721 )
        {
            if(p_caster->GetAreaId() != 4281)
                return SPELL_FAILED_NOT_HERE;
        }

        if( GetSpellProto()->NameHash == SPELL_HASH_LIFE_TAP )
        {
            if(p_caster->GetPowerPct(POWER_TYPE_MANA) == 100)
                return SPELL_FAILED_ALREADY_AT_FULL_POWER;
        }

        if(GetSpellProto()->isSpellCastableOnlyInOutlands() && m_caster->GetMapId() != 530)
            return SPELL_FAILED_NOT_HERE;

        // flying auras
        if( GetSpellProto()->isSpellFlyingSpell() )
        {
            for(uint8 i = 0; i < 3; i++)
            {
                if(m_spellInfo->EffectApplyAuraName[i] != SPELL_AURA_MOUNTED)
                    continue;

                if(p_caster->GetMountCapability(m_spellInfo->EffectMiscValueB[i]) == NULL)
                    return SPELL_FAILED_SPELL_UNAVAILABLE;
                break;
            }
        }

        if( GetSpellProto()->Id == 53822 && p_caster->getClass()!=DEATHKNIGHT)          // DeathGate
            return SPELL_FAILED_SPELL_UNAVAILABLE;

        uint32 self_rez = p_caster->GetUInt32Value(PLAYER_SELF_RES_SPELL);
        // if theres any spells that should be cast while dead let me know
        if( !p_caster->isAlive() && self_rez != GetSpellProto()->Id)
        {
            if( (m_targets.m_targetMask & TARGET_FLAG_SELF  || m_targets.m_unitTarget == p_caster->GetGUID() || !IsHealingSpell(m_spellInfo)) && p_caster->GetShapeShift() == FORM_SPIRITOFREDEMPTION)      // not a holy spell
                    return SPELL_FAILED_SPELL_UNAVAILABLE;

            if(!GetSpellProto()->isCastableWhileDead())
                return SPELL_FAILED_NOT_WHILE_GHOST;
        }

        if(m_targets.m_unitTarget && m_targets.m_unitTarget != m_caster->GetGUID())
        {
            if(Unit *unitTarget = p_caster->GetInRangeObject<Unit>(m_targets.m_unitTarget))
            {
                if(sFactionSystem.GetFactionsInteractStatus(p_caster, unitTarget) >= FI_STATUS_NEUTRAL)
                {
                    if(!sFactionSystem.CanEitherUnitAttack(p_caster, unitTarget, false))
                        return SPELL_FAILED_BAD_TARGETS;
                } else if(sFactionSystem.CanEitherUnitAttack(p_caster, unitTarget, false))
                    return SPELL_FAILED_BAD_TARGETS;
            } else return SPELL_FAILED_BAD_TARGETS;
        }

        if (p_caster->GetMapInstance() && p_caster->GetMapInstance()->CanUseCollision(p_caster))
        {
            if (GetSpellProto()->MechanicsType == MECHANIC_MOUNTED)
            {
                // Qiraj battletanks work everywhere on map 531
                if ( p_caster->GetMapId() == 531 && ( GetSpellProto()->Id == 25953 || GetSpellProto()->Id == 26054 || GetSpellProto()->Id == 26055 || GetSpellProto()->Id == 26056 ) )
                    return SPELL_CANCAST_OK;

                if (sVMapInterface.IsIndoor( p_caster->GetMapId(), p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ() + 2.0f ))
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;
            }
            else if( GetSpellProto()->isOutdoorOnly() )
            {
                if(sVMapInterface.IsIndoor( p_caster->GetMapId(),p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ() + 2.0f ) )
                    return SPELL_FAILED_ONLY_OUTDOORS;
            }
        }
        //are we in an arena and the spell cooldown is longer then 15mins?
        if ( p_caster->m_bg && ( p_caster->m_bg->GetType() >= BATTLEGROUND_ARENA_2V2 && p_caster->m_bg->GetType() <= BATTLEGROUND_ARENA_5V5 ) )
        {
            if( GetSpellProto()->isSpellNotAvailableInArena() )
                return SPELL_FAILED_NOT_IN_ARENA;

            if( !p_caster->m_bg->HasStarted() )
            {
                // cannot cast spells in an arena if it hasn't started (blinking through gates, lalala)
                if( GetSpellProto()->NameHash == SPELL_HASH_BLINK )
                    return SPELL_FAILED_SPELL_UNAVAILABLE;
            }
        }

        if(p_caster->m_bg == NULL && GetSpellProto()->reqInBattleground())
            return SPELL_FAILED_ONLY_BATTLEGROUNDS;

        // Requires ShapeShift (stealth only atm, need more work)
        if( GetSpellProto()->RequiredShapeShift )
        {
            if( GetSpellProto()->RequiredShapeShift == (uint32)1 << (FORM_STEALTH-1) )
            {
                if( !(((uint32)1 << (p_caster->GetShapeShift()-1)) & GetSpellProto()->RequiredShapeShift) && !p_caster->HasDummyAura(SPELL_HASH_SHADOW_DANCE) )
                    return SPELL_FAILED_ONLY_STEALTHED;
            }
        }

        if(!castPtr<Unit>(m_caster)->IsInCombat() && GetSpellProto()->NameHash == SPELL_HASH_DISENGAGE)
            return SPELL_FAILED_SPELL_UNAVAILABLE;

        // Disarm
        if( castPtr<Unit>(m_caster)!= NULL )
        {
            if (GetSpellProto()->isUnavailableInCombat() && castPtr<Unit>(m_caster)->IsInCombat())
            {
                // Charge In Combat, it's broke since 3.3.5, ??? maybe an aura state that needs to be set now
                //if ((GetSpellProto()->Id !=  100 && GetSpellProto()->Id != 6178 && GetSpellProto()->Id != 11578 ) )
                return SPELL_FAILED_TARGET_IN_COMBAT;
            }


            if( GetSpellProto()->IsSpellWeaponSpell() && castPtr<Unit>(m_caster)->disarmed )
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
            if( castPtr<Unit>(m_caster)->disarmedShield && GetSpellProto()->EquippedItemInventoryTypeMask && (GetSpellProto()->EquippedItemInventoryTypeMask & (1 << INVTYPE_SHIELD)) )
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        }

        // check for cooldowns
        if(!tolerate && !p_caster->Cooldown_CanCast(m_spellInfo))
            return SPELL_FAILED_NOT_READY;

        if(p_caster->GetDuelState() == DUEL_STATE_REQUESTED)
        {
            for(uint8 i = 0; i < 3; i++)
            {
                if( GetSpellProto()->Effect[i] && GetSpellProto()->Effect[i] != SPELL_EFFECT_APPLY_AURA
                    && GetSpellProto()->Effect[i] != SPELL_EFFECT_APPLY_PET_AURA
                    && GetSpellProto()->Effect[i] != SPELL_EFFECT_APPLY_AREA_AURA)
                {
                    return SPELL_FAILED_TARGET_DUELING;
                }
            }
        }

        // check for duel areas
        if( GetSpellProto()->Id == 7266 )
        {
            if(p_caster->GetMapInstance()->CanUseCollision(p_caster))
            {
                if(sVMapInterface.IsIncity(p_caster->GetMapId(), p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ()))
                    return SPELL_FAILED_NO_DUELING;
            }
            else
            {
                AreaTableEntry* at = dbcAreaTable.LookupEntry( p_caster->GetAreaId() );
                if(at == NULL)
                    at = dbcAreaTable.LookupEntry( p_caster->GetZoneId() );

                if(at != NULL && at->AreaFlags & AREA_CITY_AREA)
                    return SPELL_FAILED_NO_DUELING;
            }

            if( p_caster->m_bg )
                return SPELL_FAILED_NO_DUELING;
        }

        // check if spell is allowed while player is on a taxi
        if(p_caster->GetTaxiState())
        {
            // This uses the same flag as ordinary mounts
            if(!GetSpellProto()->isCastableWhileMounted())
                return SPELL_FAILED_NOT_ON_TAXI;
        }

        // check if spell is allowed while not mounted
        if(!p_caster->IsMounted())
        {
            if( GetSpellProto()->Id == 25860) // Reindeer Transformation
                return SPELL_FAILED_ONLY_MOUNTED;
        }
        else
        {
            if (!GetSpellProto()->isCastableWhileMounted())
                return SPELL_FAILED_NOT_MOUNTED;
        }

        for(uint8 i = 0; i < 3; i++)
        {
            if( GetSpellProto()->Effect[i] == SPELL_EFFECT_OPEN_LOCK && GetSpellProto()->EffectMiscValue[i] == LOCKTYPE_SLOW_OPEN )
            {
                if( p_caster->m_MountSpellId )
                    p_caster->RemoveAura( p_caster->m_MountSpellId );

                //p_caster->RemoveStealth();
                break;
            }
        }

        // check if spell is allowed while we have a battleground flag
        if(p_caster->m_bgHasFlag)
        {
            if( (GetSpellProto()->NameHash == SPELL_HASH_DIVINE_SHIELD || GetSpellProto()->NameHash == SPELL_HASH_ICE_BLOCK) )
            {
                if(p_caster->m_bg && p_caster->m_bg->GetType() == BATTLEGROUND_WARSONG_GULCH)
                    castPtr<WarsongGulch>(p_caster->m_bg)->DropFlag( p_caster );
                else if(p_caster->m_bg && p_caster->m_bg->GetType() == BATTLEGROUND_EYE_OF_THE_STORM)
                    castPtr<EyeOfTheStorm>(p_caster->m_bg)->DropFlag( p_caster );
            }
        }

        // check if we have the required tools, totems, etc
        if( GetSpellProto()->Totem[0] != 0)
        {
            if(!p_caster->GetInventory()->GetItemCount(GetSpellProto()->Totem[0]))
                return SPELL_FAILED_TOTEMS;
        }

        if( GetSpellProto()->Totem[1] != 0)
        {
            if(!p_caster->GetInventory()->GetItemCount(GetSpellProto()->Totem[1]))
                return SPELL_FAILED_TOTEMS;
        }

        // stealth check
        if( (GetSpellProto()->NameHash == SPELL_HASH_STEALTH || GetSpellProto()->NameHash == SPELL_HASH_PROWL) && p_caster->IsInCombat() )
            return SPELL_FAILED_TARGET_IN_COMBAT;

        if( ( GetSpellProto()->NameHash == SPELL_HASH_CANNIBALIZE || GetSpellProto()->Id == 46584 ))
        {
            bool check = false;
            for(WorldObject::InRangeSet::iterator i = p_caster->GetInRangeUnitSetBegin(); i != p_caster->GetInRangeUnitSetEnd(); i++)
            {
                Unit *target = p_caster->GetInRangeObject<Unit>(*i);
                if(p_caster->GetDistance2dSq(target) <= 25)
                    if( target->isDead() )
                        check = true;
            }

            if(check == false)
                return SPELL_FAILED_NO_EDIBLE_CORPSES;
        }

        // check if we have the required gameobject focus
        if( GetSpellProto()->RequiresSpellFocus)
        {
            float focusRange;
            bool found = false;
            for(WorldObject::InRangeSet::iterator itr = m_caster->GetInRangeGameObjectSetBegin(); itr != m_caster->GetInRangeGameObjectSetEnd(); itr++ )
            {
                GameObject *target = p_caster->GetInRangeObject<GameObject>(*itr);
                if(target == NULL || target->GetType() != GAMEOBJECT_TYPE_SPELL_FOCUS)
                    continue;

                GameObjectInfo *info = target->GetInfo();
                if(!info)
                {
                    sLog.Debug("Spell","Warning: could not find info about game object %u", target->GetEntry());
                    continue;
                }

                // lets read the distance from database
                focusRange = (float)info->data.spellFocus.dist;

                // is that possible?
                if( !focusRange )
                    focusRange = 5.0f;

                if(!IsInrange(p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ(), target, (focusRange * focusRange)))
                    continue;

                if(info->data.spellFocus.focusId == GetSpellProto()->RequiresSpellFocus)
                {
                    found = true;
                    break;
                }
            }

            if(!found)
                return SPELL_FAILED_REQUIRES_SPELL_FOCUS;
        }

        if( GetSpellProto()->AreaGroupId > 0)
        {
            bool found = false;
            uint16 area_id = p_caster->GetAreaId();
            uint32 zone_id = p_caster->GetZoneId();

            AreaGroupEntry *groupEntry = dbcAreaGroup.LookupEntry( GetSpellProto()->AreaGroupId );
            if( groupEntry )
            {
                for ( uint8 i=0; i<7; i++ )
                {
                    if( groupEntry->AreaId[i] == zone_id || groupEntry->AreaId[i] == area_id )
                    {
                        found = true;
                        break;
                    }
                }
            }

            if(!found)
                return SPELL_FAILED_REQUIRES_AREA;
        }

        // aurastate check
        if( GetSpellProto()->CasterAuraState)
        {
            if( !p_caster->HasFlag( UNIT_FIELD_AURASTATE, 1 << (GetSpellProto()->CasterAuraState-1) ) )
                return SPELL_FAILED_CASTER_AURASTATE;
        }
    }

    // set up our max Range
    float maxRange = GetSpellProto()->maxRange[0];
    if(m_targets.m_unitTarget && m_caster && m_caster->IsInWorld())
        if(sFactionSystem.isCombatSupport(castPtr<Unit>(m_caster), m_caster->GetMapInstance()->GetUnit(m_targets.m_unitTarget)))
            maxRange = GetSpellProto()->maxRange[1];

    if( GetSpellProto()->SpellGroupType && m_caster->IsUnit() )
    {
        castPtr<Unit>(m_caster)->SM_FFValue(SMT_RANGE, &maxRange, GetSpellProto()->SpellGroupType );
        castPtr<Unit>(m_caster)->SM_PFValue(SMT_RANGE, &maxRange, GetSpellProto()->SpellGroupType );
    }

    // Targeted Location Checks (AoE spells)
    if( m_targets.m_targetMask == TARGET_FLAG_DEST_LOCATION )
    {
        if( !IsInrange( m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, m_caster, ( maxRange * maxRange ) ) )
            return SPELL_FAILED_OUT_OF_RANGE;
    }

    // Collision 2 broken for this :|
    //if(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION && !m_caster->IsInLineOfSight(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ))
        //return SPELL_FAILED_LINE_OF_SIGHT;

    Unit* target = NULL;
    if( m_targets.m_targetMask == TARGET_FLAG_SELF )
        target = castPtr<Unit>(m_caster);

    // Targeted Unit Checks
    if(m_targets.m_unitTarget)
    {
        if( m_targets.m_unitTarget == m_caster->GetGUID() && m_caster->IsUnit() )
            target = castPtr<Unit>(m_caster);
        else
            target = (m_caster->IsInWorld()) ? m_caster->GetMapInstance()->GetUnit(m_targets.m_unitTarget) : NULL;

        if(target != NULL)
        {
            if( target != m_caster )
            {
                // Partha: +2.52yds to max range, this matches the range the client is calculating.
                // see extra/supalosa_range_research.txt for more info

                if( tolerate ) // add an extra 33% to range on final check (squared = 1.78x)
                {
                    if( !IsInrange( m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), target, ( ( maxRange + 2.52f ) * ( maxRange + 2.52f ) * 1.78f ) ) )
                        return SPELL_FAILED_OUT_OF_RANGE;
                }
                else
                {
                    // Added +2 because there's always someone who forgot to put CombatReach into the DB and latency compensation
                    float targetRange = maxRange + target->GetSize() + (castPtr<Unit>(m_caster) ? castPtr<Unit>(m_caster)->GetSize() : 0 ) + 2;
                    if( !IsInrange(m_caster, target, targetRange * targetRange ) )
                        return SPELL_FAILED_OUT_OF_RANGE;
                }
            }

            if( m_caster->IsPlayer() )
            {
                if ( target != m_caster && !m_caster->IsInLineOfSight(target) )
                    return SPELL_FAILED_LINE_OF_SIGHT;

                // check aurastate
                if( GetSpellProto()->TargetAuraState && !target->HasFlag( UNIT_FIELD_AURASTATE, 1<<(GetSpellProto()->TargetAuraState-1) ) )
                    return SPELL_FAILED_TARGET_AURASTATE;

                if(target->IsPlayer())
                {
                    // disallow spell casting in sanctuary zones
                    // allow attacks in duels
                    if( castPtr<Player>(m_caster)->DuelingWith != target && !sFactionSystem.isFriendly( m_caster, target ) )
                    {
                        AreaTableEntry* atCaster = dbcAreaTable.LookupEntry( m_caster->GetAreaId() );
                        AreaTableEntry* atTarget = dbcAreaTable.LookupEntry( target->GetAreaId() );
                        if( atCaster != NULL && atTarget != NULL )
                            if( atCaster->AreaFlags & 0x800 || atTarget->AreaFlags & 0x800 )
                                return SPELL_FAILED_NOT_HERE;
                    }
                }

                if( GetSpellProto()->EffectApplyAuraName[0] == 2)//mind control
                {
                    if( GetSpellProto()->EffectBasePoints[0])//got level req;
                    {
                        if((int32)target->getLevel() > GetSpellProto()->EffectBasePoints[0]+1 + int32(castPtr<Player>(m_caster)->getLevel() - GetSpellProto()->spellLevelSpellLevel))
                            return SPELL_FAILED_HIGHLEVEL;
                        else if(target->GetTypeId() == TYPEID_UNIT)
                        {
                            Creature* c =  castPtr<Creature>(target);
                            if (c&&c->GetCreatureData()&&c->GetCreatureData()->rank >ELITE_ELITE)
                                return SPELL_FAILED_HIGHLEVEL;
                        }
                    }
                }
            }

            // scripted spell stuff
            switch(GetSpellProto()->Id)
            {
            case 603: //curse of doom, can't be casted on players
            case 30910:
            case 47867:
                {
                    if(target->IsPlayer())
                        return SPELL_FAILED_TARGET_IS_PLAYER;
                }break;

            case 13907:
                {
                    if (!target || target->IsPlayer() || target->GetCreatureType()!=TARGET_TYPE_DEMON )
                        return SPELL_FAILED_SPELL_UNAVAILABLE;
                }break;

                // disable spell
            case 25997: // Eye for an Eye
            case 38554: //Absorb Eye of Grillok
                {
                    // do not allow spell to be cast
                    return SPELL_FAILED_SPELL_UNAVAILABLE;
                }break;

                //These spells are NPC only.
            case 25166: //Call Glyphs of Warding
            case 38892: //Shadow Bolt
            case 40536: //Chain Lightning
            case 41078: //Shadow Blast
                {
                    if(castPtr<Unit>(m_caster)->IsPlayer())
                        return SPELL_FAILED_BAD_TARGETS;
                }break;
            }

            // if the target is not the unit caster and not the masters pet
            if(target != castPtr<Unit>(m_caster))
            {

                /***********************************************************
                * Inface checks, these are checked in 2 ways
                * 1e way is check for damage type, as 3 is always ranged
                * 2e way is trough the data in the extraspell db
                *
                **********************************************************/

                /* burlex: units are always facing the target! */
                if(castPtr<Player>(m_caster) && GetSpellProto()->FacingCasterFlags)
                {
                    if(!castPtr<Player>(m_caster)->isTargetInFront(target))
                        return SPELL_FAILED_UNIT_NOT_INFRONT;
                    if(GetSpellProto()->reqCasterIsBehindTarget() && (GetSpellProto()->Id != SPELL_RANGED_THROW) &&
                        !castPtr<Player>(m_caster)->isInBackOfTarget(target))
                        return SPELL_FAILED_NOT_BEHIND;
                }
            }

            // if target is already skinned, don't let it be skinned again
            if( GetSpellProto()->Effect[0] == SPELL_EFFECT_SKINNING) // skinning
                if(target->IsUnit() && (castPtr<Creature>(target)->m_skinned) )
                    return SPELL_FAILED_TARGET_UNSKINNABLE;

            // target 39 is fishing, all fishing spells are handled
            if( GetSpellProto()->EffectImplicitTargetA[0] == 39 )
            {
                uint32 entry = GetSpellProto()->EffectMiscValue[0];
                if(entry == GO_FISHING_BOBBER)
                {
                    float px=castPtr<Unit>(m_caster)->GetPositionX();
                    float py=castPtr<Unit>(m_caster)->GetPositionY();
                    float orient = m_caster->GetOrientation();
                    float posx = 0,posy = 0,posz = 0;
                    float co = cos(orient);
                    float si = sin(orient);
                    MapInstance* map = m_caster->GetMapInstance();

                    float r;
                    for(r=20; r>10; r--)
                    {
                        posx = px + r * co;
                        posy = py + r * si;
                        posz = map->GetWaterHeight(posx,posy, NO_WATER_HEIGHT);
                        if(posz > map->GetLandHeight(posx,posy))//water
                            break;
                    }
                    if(r<=10)
                        return SPELL_FAILED_NOT_FISHABLE;

                    // if we are already fishing, dont cast it again
                    if(castPtr<Player>(m_caster)->GetSummonedObject())
                        if(castPtr<Player>(m_caster)->GetSummonedObject()->GetEntry() == GO_FISHING_BOBBER)
                            return SPELL_FAILED_SPELL_IN_PROGRESS;
                }
            }

            if( m_caster->IsPlayer() )
            {
                if( GetSpellProto()->NameHash == SPELL_HASH_GOUGE )// Gouge
                    if(!target->isTargetInFront(castPtr<Player>(m_caster)))
                        return SPELL_FAILED_NOT_INFRONT;

                if( GetSpellProto()->Category==1131)//Hammer of wrath, requires target to have 20- % of hp
                {
                    if(target->GetUInt32Value(UNIT_FIELD_HEALTH) == 0)
                        return SPELL_FAILED_BAD_TARGETS;

                    if(target->GetUInt32Value(UNIT_FIELD_MAXHEALTH)/target->GetUInt32Value(UNIT_FIELD_HEALTH)<5)
                         return SPELL_FAILED_BAD_TARGETS;
                }
                else if( GetSpellProto()->NameHash == SPELL_HASH_CONFLAGRATE)//Conflagrate, requires immolation spell on victim
                {
                    if(!target->HasAurasOfNameHashWithCaster(SPELL_HASH_IMMOLATION, NULL))
                        return SPELL_FAILED_BAD_TARGETS;
                }

                if( GetSpellProto()->NameHash == SPELL_HASH_ENVENOM )
                {
                    if( !target->m_AuraInterface.HasAuraVisual(5100) )
                        return SPELL_FAILED_BAD_TARGETS ;
                }

                if(target->GetDispelImmunity(GetSpellProto()->DispelType))
                    return SPELL_FAILED_PREVENTED_BY_MECHANIC-1;            // hackfix - burlex
            }

            // if we're replacing a higher rank, deny it
            if(IsAuraApplyingSpell())
            {
                AuraCheckResponse acr = target->m_AuraInterface.AuraCheck(m_spellInfo, m_caster->GetGUID());
                if( acr.Error == AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT )
                    return SPELL_FAILED_AURA_BOUNCED;
            }

            //check if we are trying to stealth or turn invisible but it is not allowed right now
            if( IsStealthSpell() || IsInvisibilitySpell() )
            {
                //if we have Faerie Fire, we cannot stealth or turn invisible
                if( castPtr<Unit>(m_caster)->m_AuraInterface.HasNegativeAuraWithNameHash( SPELL_HASH_FAERIE_FIRE ) || castPtr<Unit>(m_caster)->m_AuraInterface.HasNegativeAuraWithNameHash( SPELL_HASH_FAERIE_FIRE__FERAL_ ) )
                    return SPELL_FAILED_SPELL_UNAVAILABLE;
            }

            if( target->IsPlayer() )
            {
                switch( GetSpellProto()->NameHash )
                {
                case SPELL_HASH_DIVINE_PROTECTION:
                case SPELL_HASH_DIVINE_SHIELD:
                case SPELL_HASH_HAND_OF_PROTECTION:
                    {
                        if( castPtr<Player>(target)->HasAura(25771) ) // Forbearance
                            return SPELL_FAILED_DAMAGE_IMMUNE;

                        if( !castPtr<Player>(target)->HasDummyAura(SPELL_HASH_AVENGING_WRATH) )
                            return SPELL_FAILED_DAMAGE_IMMUNE;
                    }break;

                case SPELL_HASH_AVENGING_WRATH:
                    {
                        if( !castPtr<Player>(target)->HasDummyAura(SPELL_HASH_AVENGING_WRATH) )
                            return SPELL_FAILED_DAMAGE_IMMUNE;
                    }break;

                case SPELL_HASH_ICE_BLOCK:
                    {
                        if( castPtr<Player>(target)->HasAura(41425) ) // Hypothermia
                            return SPELL_FAILED_DAMAGE_IMMUNE;

                    }break;

                case SPELL_HASH_POWER_WORD__SHIELD:
                    {
                        if( castPtr<Player>(target)->HasAura(6788) ) // Weakened Soul
                            return SPELL_FAILED_DAMAGE_IMMUNE;
                    }break;

                case SPELL_HASH_FIRST_AID:
                    {
                        if( castPtr<Player>(target)->HasAura(11196) ) // Recently Bandaged
                            return SPELL_FAILED_DAMAGE_IMMUNE;
                    }break;
                case SPELL_HASH_BLOODLUST:
                    {
                        if( castPtr<Player>(target)->HasAurasOfNameHashWithCaster(SPELL_HASH_SATED, NULL) )
                            return SPELL_FAILED_DAMAGE_IMMUNE;
                    }break;
                }
            }

            if (GetSpellProto()->MechanicsType == 16 && target->HasAura(11196))
                return SPELL_FAILED_DAMAGE_IMMUNE;
        }
    }

    // Special State Checks (for creatures & players)
    if( castPtr<Unit>(m_caster) )
    {
        if(target != NULL) /* -Supalosa- Shouldn't this be handled on Spell Apply? */
        {
            for( int8 i = 0; i < 3; i++ ) // if is going to cast a spell that breaks stun remove stun auras, looks a bit hacky but is the best way i can find
            {
                if( GetSpellProto()->EffectApplyAuraName[i] == SPELL_AURA_MECHANIC_IMMUNITY )
                    target->m_AuraInterface.RemoveAllAurasByMechanic( GetSpellProto()->EffectMiscValue[i] , -1 , true );
            }
        }

        if(castPtr<Unit>(m_caster)->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) > 0)
        {
            SpellEntry * t_spellInfo = (castPtr<Unit>(m_caster)->GetCurrentSpell() ? castPtr<Unit>(m_caster)->GetCurrentSpell()->m_spellInfo : NULL);

            if(!t_spellInfo || !m_triggeredSpell)
                return SPELL_FAILED_SPELL_IN_PROGRESS;
            else if (t_spellInfo)
            {
                if( t_spellInfo->EffectTriggerSpell[0] != GetSpellProto()->Id &&
                    t_spellInfo->EffectTriggerSpell[1] != GetSpellProto()->Id &&
                    t_spellInfo->EffectTriggerSpell[2] != GetSpellProto()->Id)
                {
                    return SPELL_FAILED_SPELL_IN_PROGRESS;
                }
            }
        }
    }

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

    if( m_reflectedParent != NULL || m_caster == refunit )
        return false;

    // if the spell to reflect is a reflect spell, do nothing.
    for(int i=0; i<3; i++)
    {
        if( GetSpellProto()->Effect[i] == 6 && (GetSpellProto()->EffectApplyAuraName[i] == 74 || GetSpellProto()->EffectApplyAuraName[i] == 28))
            return false;
    }

    if( !refspellid || !canreflect )
        return false;

    SpellCastTargets targets(m_caster->GetGUID());
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

void Spell::_AddTarget(WorldObject* target, const uint32 effIndex)
{
    // Check if we're in the current list already, and if so, don't readd us.
    if(m_effectTargetMaps[effIndex].find(target->GetGUID()) != m_effectTargetMaps[effIndex].end())
        return;

    bool found = false;
    // look for the target in the list already
    SpellTargetMap::iterator itr = m_fullTargetMap.find(target->GetGUID());
    if(found = (itr != m_fullTargetMap.end()))
        itr->second.EffectMask |= (1 << effIndex);

    // setup struct
    SpellTarget tgt;
    tgt.Guid = target->GetGUID();
    tgt.EffectMask = (1<<effIndex);

    if(m_spellInfo->speed > 0.0f)
    {
        // calculate spell incoming interval
        float dist = sqrtf(m_caster->GetDistanceSq(target));
        tgt.DestinationTime = uint32(floor(dist / m_spellInfo->speed*1000.0f));
        if(tgt.DestinationTime+m_MSTimeToAddToTravel < 200)
            tgt.DestinationTime = 0;
        if (m_missileTravelTime == 0 || tgt.DestinationTime > m_missileTravelTime)
            m_missileTravelTime = tgt.DestinationTime;
        tgt.DestinationTime += m_MSTimeToAddToTravel;
    } else tgt.DestinationTime = 0;

    // add to the list
    m_effectTargetMaps[effIndex].insert(std::make_pair(target->GetGUID(), tgt));
    if(!found)
    {
        // work out hit result (always true if we are a GO)
        tgt.HitResult = target->IsUnit() ? _DidHit(effIndex, castPtr<Unit>(target), &tgt.ReflectResult) : SPELL_DID_HIT_SUCCESS;

        // Store in our full targetting map
        m_fullTargetMap.insert(std::make_pair(target->GetGUID(), tgt));
    }

    // add counter
    if( tgt.HitResult == SPELL_DID_HIT_SUCCESS )
        ++m_hitTargetCount;
    else ++m_missTargetCount;
}

void Spell::DamageGosAround(uint32 i)
{
    uint32 spell_id = GetSpellProto()->Id;
    float r = GetRadius(i);
    r *= r;
    LocationVector target = ((m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION) ? m_targets.m_dest : ((m_targets.m_targetMask & TARGET_FLAG_SOURCE_LOCATION) ? m_targets.m_src : m_caster->GetPosition()));
    for(WorldObject::InRangeSet::iterator itr = m_caster->GetInRangeGameObjectSetBegin(); itr != m_caster->GetInRangeGameObjectSetEnd(); ++itr )
    {
        if(GameObject *gObj = m_caster->GetInRangeObject<GameObject>(*itr))
        {
            if(gObj->GetDistance2dSq(target.x, target.y) > r)
                continue;
            gObj->TakeDamage(damage,m_caster,m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL,spell_id);
        }
    }
}

bool Spell::CanHandleSpellEffect(uint32 i)
{
    if(m_caster->IsUnit())
    {
        switch(i)
        {
        case 1:
            {
                switch(m_spellInfo->NameHash)
                {
                    case SPELL_HASH_FROSTBOLT:
                    {
                        if(castPtr<Unit>(m_caster)->HasDummyAura( SPELL_HASH_GLYPH_OF_FROSTBOLT ))
                            return false;
                    }break;
                }break;
            }break;
        case 2:
            {
                switch(m_spellInfo->NameHash)
                {
                    case SPELL_HASH_FIREBALL:
                    {
                        if(castPtr<Unit>(m_caster)->HasDummyAura( SPELL_HASH_GLYPH_OF_FIREBALL ))
                            return false;
                    }break;
                }break;
            }break;
        case 3:
            {
                switch(m_spellInfo->NameHash)
                {
                    case SPELL_HASH_BLAST_WAVE:
                    {
                        if(castPtr<Unit>(m_caster)->HasAura(62126))
                            return false;
                    }break;     
                    case SPELL_HASH_THUNDERSTORM:
                    {
                        if(castPtr<Unit>(m_caster)->HasAura(62132))
                            return false;
                    }break;
                }break;
            }break;
        }
    }
    return true;
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
        case SPELL_EFFECT_HEALTH_FUNNEL:
        case SPELL_EFFECT_HEAL_MAX_HEALTH:
            return true;
            break;

        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA:
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
