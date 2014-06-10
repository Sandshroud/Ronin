/***
 * Demonstrike Core
 */

#include "StdAfx.h"

///====================================================================
///  Desc: Updates Combat Status of m_Unit
///====================================================================
void AIInterface::_UpdateCombat(uint32 p_time)
{
    ASSERT(m_Unit != NULL);
    if(!m_Unit->isAlive())
        return;
    if( m_AIType != AITYPE_PET && disable_combat )
        return;

    if(m_nextTarget)
    {
        // Check if our target is attackable, if not, change to the most hated.
        if(!sFactionSystem.CanEitherUnitAttack(m_Unit, m_nextTarget, false))
        {
            SetNextTarget(GetMostHated());

            // Check if our new target is unattackable, or doesn't exist
            if(!sFactionSystem.CanEitherUnitAttack(m_Unit, m_nextTarget, false))
                SetNextTarget(FindTarget());
        }

        if( m_AIType != AITYPE_PET && (m_outOfCombatRange && m_Unit->GetDistanceSq(GetReturnPos()) > m_outOfCombatRange)
            && m_AIState != STATE_EVADE && m_AIState != STATE_SCRIPTMOVE && !m_fleeTimer && !m_is_in_instance)
        {
            HandleEvent( EVENT_LEAVECOMBAT, m_Unit, 0 );
            return;
        }
        else if( m_nextTarget == NULL && m_AIState != STATE_FOLLOWING && m_AIState != STATE_SCRIPTMOVE && !m_fleeTimer )
        {
            SetNextTarget(GetMostHated());
            if( m_nextTarget == NULL )
            {
                HandleEvent( EVENT_LEAVECOMBAT, m_Unit, 0 );
                return;
            }
        }
    }

    if(!disable_spell && m_spells.size() && m_AIState != STATE_EVADE && !m_Unit->isCasting())
    {
        if(m_CastTimer > p_time)
            m_CastTimer -= p_time;
        else
        {
            m_CastTimer = 0;
            m_AIState = STATE_ATTACKING;
            m_Unit->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT,0);
            m_Unit->SetUInt32Value(UNIT_CHANNEL_SPELL,0);
            uint32 currentTime = getMSTime();

            // DO OUR BARREL SPELLS
            if(m_CastNext == NULL)
            {
                //Try our chance at casting a spell (Will actually be cast on next ai update, so we just
                //schedule it. This is needed to avoid next dealt melee damage while we cast the spell.)
                AI_Spell* Spell = NULL;
                for( map<uint32, AI_Spell*>::iterator SpellIter = m_spells.begin(); SpellIter != m_spells.end(); ++SpellIter )
                {
                    Spell = SpellIter->second;
                    if( Spell->m_AI_Spell_disabled )
                        continue;
                    if( Spell->perctrigger == 0.0f )
                        continue;
                    if(!CanCastAISpell(Spell, currentTime))
                        continue;
                    // Check if spell won the roll
                    Unit* pTarget = GetTargetForSpell(Spell);
                    if(pTarget == NULL)
                        continue;

                    if(Spell->perctrigger != 100.0f)
                    {
                        float ChanceRoll = RandomFloat(100.0f);
                        if(Spell->perctrigger < ChanceRoll)
                            continue;
                    }

                    if(pTarget == m_Unit)
                        CastAISpell(m_Unit, Spell, currentTime);
                    else
                    {
                        unitBehavior = Behavior_Spell;
                        if(pTarget != GetNextTarget())
                            SetNextTarget(pTarget);
                        m_CastNext = Spell;
                        break;
                    }
                }
            }
        }
    }

    if(m_AIState == STATE_IDLE || m_AIState == STATE_FOLLOWING
        || m_AIState == STATE_FEAR || m_AIState == STATE_WANDER
        || m_AIState == STATE_SCRIPTMOVE)
        return;
    if(m_AIType == AITYPE_PET && m_Unit->IsPet())
    {
        Pet* pPet = TO_PET(m_Unit);
        if(pPet->GetPetAction() != PET_ACTION_ATTACK || pPet->GetPetState() == PET_STATE_PASSIVE)
            return;
    }
    if(!m_Unit->CombatStatus.IsInCombat())
        return;

    BehaviorType LastBehavior = unitBehavior;
    if( m_nextTarget != NULL && m_nextTarget->isAlive() && m_AIState != STATE_EVADE && !m_Unit->isCasting())
    {
        if( LastBehavior == Behavior_Default || ( m_AIType == AITYPE_PET && m_CastNext == NULL ) ) // allow pets autocast
        {
            if(m_Unit->IsPet() && !m_CastNext)
            {
                AI_Spell* PetSpell = TO_PET(m_Unit)->HandleAutoCastEvent();
                if(PetSpell && CanCastAISpell(PetSpell, getMSTime()))
                {
                    if(IsValidUnitTarget(m_nextTarget, PetSpell))
                    {
                        unitBehavior = Behavior_Spell;
                        m_CastNext = PetSpell;
                    }
                }
            }

            if(m_canFlee && !m_hasFled && ( float(m_Unit->GetHealthPct()) < (m_FleeHealth ? m_FleeHealth : 1)))
            {
                setMoveRunFlag(false);
                if(m_fleeTimer == 0)
                    m_fleeTimer = m_FleeDuration;

                _CalcDestinationAndMove(m_nextTarget, 10.0f);
                if(!m_hasFled)
                    CALL_SCRIPT_EVENT(m_Unit, OnFlee)(m_nextTarget);

                SetAIState(STATE_FLEEING);
                SetNextTarget(NULLUNIT);

                switch(sendflee_message)
                {
                case 1:
                    {
                        if(flee_message.size())
                            m_Unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, flee_message.c_str());
                    }break;
                case 2:
                    {
                        if(flee_message.size())
                            m_Unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, flee_message.c_str());
                    }break;
                default:
                    {
                        stringstream ss;
                        ss << TO_CREATURE( m_Unit )->GetCreatureInfo()->Name << " attempts to run away in fear!";
                        m_Unit->SendChatMessage(CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, ss.str().c_str());
                    }break;
                }

                m_hasFled = true;
                return;
            }
            else if(m_canCallForHelp && !m_hasCalledForHelp )
            {
                FindFriends( 50.0f /*7.0f*/ );
                m_hasCalledForHelp = true; // We only want to call for Help once in a Fight.
                if( m_Unit->GetTypeId() == TYPEID_UNIT )
                    objmgr.HandleMonsterSayEvent( TO_CREATURE( m_Unit ), MONSTER_SAY_EVENT_CALL_HELP );
                CALL_SCRIPT_EVENT( m_Unit, OnCallForHelp )();
                return;
            }
            else
            {
                if(m_CastNext != NULL)
                    LastBehavior = Behavior_Spell;
                else
                    LastBehavior = Behavior_Melee;
            }
        }

        //check if we can do range attacks
        if(LastBehavior == Behavior_Ranged || LastBehavior == Behavior_Melee)
        {
            if(m_canRangedAttack)
            {
                float dist = m_Unit->GetDistanceSq(m_nextTarget);
                if(m_nextTarget->IsPlayer())
                {
                    if( TO_PLAYER( m_nextTarget )->m_currentMovement == MOVE_ROOT || dist >= 32.0f )
                        LastBehavior = Behavior_Ranged;
                    else
                        LastBehavior = Behavior_Melee;
                }
                else if( m_nextTarget->m_canMove == false || dist >= 32.0f )
                    LastBehavior = Behavior_Ranged;
                else
                    LastBehavior = Behavior_Melee;
            }
            else
                LastBehavior = Behavior_Melee;
        }

        if( disable_melee && LastBehavior == Behavior_Melee )
            LastBehavior = Behavior_Default;
        if( disable_ranged && LastBehavior == Behavior_Ranged )
            LastBehavior = Behavior_Default;
        if( disable_spell && LastBehavior == Behavior_Spell )
            LastBehavior = Behavior_Default;

        float distance = m_Unit->CalcDistance(m_nextTarget);
        switch(LastBehavior)
        {
        case Behavior_Ranged:
            {
                if(distance <= 8.0f)
                    LastBehavior = Behavior_Melee;
            }break;

        case Behavior_Spell:
            {
                bool los = m_Unit->IsInLineOfSight(m_nextTarget);
                if(!los)
                {
                    m_CastNext = NULL;
                    LastBehavior = Behavior_Melee;
                    SetBehaviorType(Behavior_Default);
                }
                else if(m_CastNext != NULL)
                {
                    if(!IsValidUnitTarget(m_nextTarget, m_CastNext))
                    {
                        m_CastNext = NULL;
                        LastBehavior = Behavior_Melee;
                        SetBehaviorType(Behavior_Default);
                    }
                }
                else
                {
                    LastBehavior = Behavior_Melee;
                    SetBehaviorType(Behavior_Default);
                }
            }break;
        }

        switch(LastBehavior)
        {
        case Behavior_Melee:
            {
                if( m_Unit->GetTypeId() == TYPEID_UNIT )
                    TO_CREATURE(m_Unit)->SetSheatheForAttackType( 1 );
                float combatReach = _CalcCombatRange(m_nextTarget, false); // Calculate Combat Reach
                float distance = m_Unit->CalcDistance(m_nextTarget);

                if(distance <= combatReach + DISTANCE_TO_SMALL_TO_WALK) // Target is (alomst) in Range -> Attack
                {
                    if(getUnitToFollow() != NULL)
                        MovementHandler.ClearFollowInformation(getUnitToFollow());

                    if(m_Unit->isAttackReady(false) && !m_fleeTimer && m_AllowedToEnterCombat)
                    {
                        m_creatureState = ATTACKING;
                        bool infront = m_Unit->isTargetInFront(m_nextTarget);
                        if(!infront) // set InFront
                        {
                            // prevent mob from rotating while stunned
                            if(!m_Unit->IsStunned())
                            {
                                setInFront(m_nextTarget);
                                infront = true;
                            }
                        }
                        else
                        {
                            m_Unit->setAttackTimer(0, false);
                            if(m_nextTarget != NULL)
                            {
                                //we require to know if strike was succesfull. If there was no dmg then target cannot be dazed by it
                                uint32 health_before_strike = m_nextTarget->GetUInt32Value(UNIT_FIELD_HEALTH);

                                m_Unit->Strike( m_nextTarget, MELEE, NULL, 0, 0, 0, false, false, true );
                                //now if the target is facing his back to us then we could just cast dazed on him :P
                                //as far as i know dazed is casted by most of the creatures but feel free to remove this code if you think otherwise
                                if(m_nextTarget != NULL && !(m_Unit->m_faction->RepListId == -1 && m_Unit->m_factionTemplate->FriendlyMask==0 && m_Unit->m_factionTemplate->HostileMask==0) /* neutral creature */
                                        && m_nextTarget->IsPlayer() && !m_Unit->IsPet() && health_before_strike>m_nextTarget->GetUInt32Value(UNIT_FIELD_HEALTH)
                                        && Rand(m_Unit->CalculateDazeCastChance(m_nextTarget)))
                                {
                                    float our_facing = m_Unit->calcRadAngle(m_Unit->GetPositionX(),m_Unit->GetPositionY(),m_nextTarget->GetPositionX(),m_nextTarget->GetPositionY());
                                    float his_facing = m_nextTarget->GetOrientation();
                                    if(fabs(our_facing-his_facing) < CREATURE_DAZE_TRIGGER_ANGLE && !m_nextTarget->m_AuraInterface.HasNegativeAura(CREATURE_SPELL_TO_DAZE))
                                    {
                                        //This should have a chance to it. It's not always 100%.
                                        SpellEntry *info = dbcSpell.LookupEntry(CREATURE_SPELL_TO_DAZE);
                                        if(info == NULL)
                                            return;

                                        Spell* sp = new Spell(m_Unit, info, false, NULLAURA);
                                        SpellCastTargets targets;
                                        targets.m_unitTarget = m_nextTarget->GetGUID();
                                        sp->prepare(&targets);
                                    }
                                }
                            }
                        }
                    }
                }
                else // Target out of Range -> Run to it
                {
                    //Make sure target can reach us.
                    float dist = _CalcCombatRange(m_nextTarget, false);
                    if(dist < m_Unit->GetModelHalfSize())
                        dist = m_Unit->GetModelHalfSize(); //unbelievable how this could happen

                    setMoveRunFlag(true);
                    _CalcDestinationAndMove(m_nextTarget, dist);
                }
            }break;
        case Behavior_Ranged:
            {
                if( m_Unit->GetTypeId() == TYPEID_UNIT )
                {
                    TO_CREATURE(m_Unit)->SetSheatheForAttackType( 3 );
                    TO_CREATURE(m_Unit)->SetUInt32Value(UNIT_NPC_EMOTESTATE, GetWeaponEmoteType(true));
                }

                float combatReach[2]; // Used Shooting Ranges
                float distance = m_Unit->CalcDistance(m_nextTarget);

                combatReach[0] = 8.0f;
                combatReach[1] = 30.0f;

                if(distance >= combatReach[0] && distance <= combatReach[1]) // Target is in Range -> Shoot!!
                {
                    if(getUnitToFollow() != NULL)
                        MovementHandler.ClearFollowInformation(getUnitToFollow());

                    // FIXME: offhand shit
                    if(m_Unit->isAttackReady(false) && !m_fleeTimer && m_AllowedToEnterCombat)
                    {
                        m_creatureState = ATTACKING;
                        bool infront = m_Unit->isTargetInFront(m_nextTarget);

                        if(!infront) // set InFront
                        {
                            //prevent mob from rotating while stunned
                            if(!m_Unit->IsStunned())
                            {
                                setInFront(m_nextTarget);
                                infront = true;
                            }
                        }
                        else
                        {
                            m_Unit->setAttackTimer(0, false);
                            SpellEntry *info = dbcSpell.LookupEntry(3018);
                            if(info)
                            {
                                Spell* sp = NULLSPELL;
                                sp = new Spell(m_Unit, info, false, NULLAURA);
                                SpellCastTargets targets(m_nextTarget->GetGUID());
                                sp->prepare(&targets);
                            }
                        }
                    }
                }
                else // Target out of Range -> Run to/from it, depending on current distance
                {
                    float dist;
                    if(distance < combatReach[0])// Target is too near
                        dist = 9.0f;
                    else
                        dist = 20.0f;

                    setMoveRunFlag(true);
                    _CalcDestinationAndMove(m_nextTarget, dist);
                }
            }break;
        case Behavior_Spell:
            {
                if( m_CastNext != NULL && m_nextTarget != NULL )
                {
                    sLog.Debug("AiAgents","NextSpell %u by NPC %u", m_CastNext->info->Id, GetUnit()->GetGUID());

                    if( m_Unit->GetTypeId() == TYPEID_UNIT )
                        TO_CREATURE(m_Unit)->SetSheatheForAttackType( 0 );

                    uint32 currentTime = getMSTime();

                    // DO OUR BARREL SPELLS
                    if(IsValidUnitTarget(m_nextTarget, m_CastNext) && CanCastAISpell(m_CastNext, currentTime))
                        CastAISpell(m_nextTarget, m_CastNext, currentTime);
                }
            }break;
        }
    }
    else
    {
        if(m_nextTarget == NULL)
        {
            if(!m_Unit->isCasting())
            {
                // no more target
                SetNextTarget(NULLUNIT);
                HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
            }
        }
        else if(m_nextTarget->GetInstanceID() != m_Unit->GetInstanceID() || !m_nextTarget->isAlive() || !m_nextTarget->IsInWorld())
        {
            // no more target
            SetNextTarget(NULLUNIT);
            HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
        }
    }
}

void AIInterface::CheckNextTargetFlyingStatus()
{
    if(m_nextTarget == NULL || !m_nextTarget->canFly())
        return;

    bool LeaveCombat = false;
    if(!IS_INSTANCE(m_Unit->GetMapId()) && !MovementHandler.m_moveFly)
    {
        float target_land_z = m_nextTarget->GetCHeightForPosition();
        if(target_land_z+_CalcCombatRange(m_nextTarget, m_canRangedAttack) < m_nextTarget->GetPositionZ())
            HandleEvent( EVENT_LEAVECOMBAT, m_Unit, 0);
    }
}

void AIInterface::_UpdateTargets(uint32 p_time)
{
    if( m_Unit->IsPlayer() || disable_targeting )
        return;

    if(m_updateListTimer > p_time)
        m_updateListTimer -= p_time;
    else
    {
        m_updateListTimer = TARGET_UPDATE_INTERVAL - (m_updateListTimer - p_time);
        // Find new Assist Targets and remove old ones
        if(m_AIState == STATE_FLEEING)
            FindFriends(100.0f/*11.0*/);
        else if(m_AIState != STATE_IDLE && m_AIState != STATE_SCRIPTIDLE)
            FindFriends(16.0f/*4.0f*/);
    }

    CheckNextTargetFlyingStatus();

    if(m_updateTargetsTimer > p_time)
        m_updateTargetsTimer -= p_time;
    else
    {
        m_updateTargetsTimer = (TARGET_UPDATE_INTERVAL * 2) - (p_time-m_updateTargetsTimer);
        ai_TargetLock.Acquire();
        for(TargetMap::iterator itr = m_aiTargets.begin(), it2; itr != m_aiTargets.end();)
        {
            it2 = itr++;

            if( it2->first->event_GetCurrentInstanceId() != m_Unit->event_GetCurrentInstanceId() || !m_Unit->PhasedCanInteract(it2->first) ||
                !sFactionSystem.CanEitherUnitAttack(m_Unit, it2->first) || m_Unit->GetDistanceSq(it2->first) >= 6400.0f)
            {
                m_aiTargets.erase( it2 );
            }
        }
        ai_TargetLock.Release();

        if(m_aiTargets.size() == 0
            && m_AIState != STATE_IDLE && m_AIState != STATE_FOLLOWING
            && m_AIState != STATE_EVADE && m_AIState != STATE_FEAR
            && m_AIState != STATE_WANDER && m_AIState != STATE_SCRIPTIDLE)
        {
            if(firstLeaveCombat)
            {
                Unit* target = FindTarget();
                if(target)
                    AttackReaction(target, 1, 0);
                else
                    firstLeaveCombat = false;
            }
        }
        else if( m_aiTargets.size() == 0 && (m_AIType == AITYPE_PET && (m_Unit->IsPet() && TO_PET(m_Unit)->GetPetState() == PET_STATE_AGGRESSIVE) || (!m_Unit->IsPet() && disable_melee == false ) ) )
        {
            Unit* target = FindTarget();
            if( target )
                AttackReaction(target, 1, 0);
        }

        // Find new Targets when we are ooc
        if(m_AIState == STATE_IDLE || m_AIState == STATE_SCRIPTIDLE)
        {
            Unit* target = FindTarget();
            if(target)
                AttackReaction(target, 1, 0);
        }
    }
}

Unit* AIInterface::FindTarget()
{
    ASSERT(m_Unit != NULL);
    // Faction check
    if(m_Unit->m_faction == NULL)
        return NULLUNIT;
    // find nearest hostile Target to attack
    if( !m_AllowedToEnterCombat || m_fleeTimer || m_Unit->isDead() )
        return NULLUNIT;

    Unit *target = NULLUNIT, *critterTarget = NULLUNIT, *pUnit = NULLUNIT;
    float distance = 999999.0f, crange = 0.0f, z_diff = 0.0f, dist = 0.0f; // that should do it.. :p

    //target is immune to all form of attacks, cant attack either.
    if(m_Unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
        return NULLUNIT;

    for( unordered_set<Unit* >::iterator itr = m_Unit->GetInRangeUnitSetBegin(); itr != m_Unit->GetInRangeUnitSetEnd(); )
    {
        pUnit = (*itr++);
        if( pUnit->isDead() || pUnit->m_invisible ) // skip invisible units
            continue;
        // Check the aggro range
        dist = m_Unit->GetDistanceSq(pUnit);
        if(dist > _CalcAggroRange(pUnit))  // we want to find the CLOSEST target
            continue;
        // Get the closest target available
        if(distance < dist)
            continue;
        // Check the z height difference
        crange = _CalcCombatRange(pUnit, false);
        if(m_isGuard)
            crange *= 4;
        z_diff = fabs(m_Unit->GetPositionZ() - pUnit->GetPositionZ());
        if(z_diff > crange)
            continue;
        //do not agro units that are faking death. Should this be based on chance ?
        if( pUnit->HasFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH ) )
            continue;
        // Check if units can engage in combat
        if(!sFactionSystem.CanEitherUnitAttack(m_Unit, pUnit, true))
            continue;
        // Check LOS if we're in range
        if( !m_Unit->IsInLineOfSight(pUnit) )
            continue;
        // If it's a critter, set the critter target.
        if(pUnit->m_factionTemplate->Faction == 28)
        {
            if(critterTarget && m_Unit->GetDistanceSq(critterTarget) < m_Unit->GetDistanceSq(pUnit))
                continue;
            critterTarget = pUnit;
            continue;
        }

        distance = dist;
        target = pUnit;
    }

    if(!target && critterTarget)
        target = critterTarget;

    if( target )
    {
        AttackReaction(target, 1, 0);

        WorldPacket data(SMSG_AI_REACTION, 12);
        data << m_Unit->GetGUID() << uint32(2);     // Aggro sound
        m_Unit->SendMessageToSet(&data, false);
        if(target->IsPet())
            AttackReaction(TO_PET(target)->GetOwner(), 1, 0);
        else if(target->IsSummon() && TO_SUMMON(target)->GetSummonOwner()->IsUnit())
            AttackReaction(TO_UNIT(TO_SUMMON(target)->GetSummonOwner()), 1, 0);
    }
    return target;
}

//should return a valid target
Unit* AIInterface::GetMostHated(AI_Spell* sp)
{
    ASSERT(m_Unit != NULL);

    // Fleeing means we shit our pants and hate no one
    if(m_fleeTimer || !m_AllowedToEnterCombat)
        return NULLUNIT;

    //override mosthated with taunted target. Basic combat checks are made for it.
    //What happens if we can't see tauntedby unit ?
    Unit* ResultUnit = getTauntedBy();
    if(ResultUnit)
        return ResultUnit;

    int32 CurrentThreat = -1;
    ai_TargetLock.Acquire();
    TargetMap::iterator it2 = m_aiTargets.begin(), itr;
    for(; it2 != m_aiTargets.end();)
    {
        itr = it2;
        ++it2;

        /* check the target is valid */
        if(itr->first->event_GetCurrentInstanceId() != m_Unit->event_GetCurrentInstanceId() || !itr->first->isAlive() || !sFactionSystem.CanEitherUnitAttack(m_Unit, itr->first))
        {
            m_aiTargets.erase(itr);
            continue;
        }

        if(itr->second == 0)
            continue; // Ignore non combat targets

        if(!IsValidUnitTarget(itr->first, (sp == NULL ? NULL : sp->info), (sp == NULL ? TargetFilter_None : sp->TargetFilter), (sp == NULL ? 0.0f : sp->mindist2cast), (sp == NULL ? m_outOfCombatRange : sp->maxdist2cast)))
            continue;

        if((itr->second + itr->first->GetThreatModifier()) > CurrentThreat)
        {
            /* new target */
            ResultUnit = itr->first;
            CurrentThreat = itr->second + itr->first->GetThreatModifier();
            m_currentHighestThreat = CurrentThreat;
        }

        /* there are no more checks needed here... the needed checks are done by CheckTarget() */
    }
    ai_TargetLock.Release();
    return ResultUnit;
}

Unit* AIInterface::GetSecondHated(AI_Spell* sp)
{
    ASSERT(m_Unit != NULL);

    // Fleeing means we shit our pants and hate no one
    if( m_fleeTimer || !m_AllowedToEnterCombat )
        return NULLUNIT;

    Unit* ResultUnit = GetMostHated();
    pair< Unit*, int32> currentTarget;
    currentTarget.first = NULLUNIT;
    currentTarget.second = -1;

    ai_TargetLock.Acquire();
    TargetMap::iterator it2 = m_aiTargets.begin(), itr;
    for(; it2 != m_aiTargets.end();)
    {
        itr = it2;
        ++it2;

        /* check the target is valid */
        if(itr->first->GetInstanceID() != m_Unit->GetInstanceID() || !itr->first->isAlive() || !sFactionSystem.isAttackable(m_Unit, itr->first))
        {
            m_aiTargets.erase(itr);
            continue;
        }

        if(itr->second == 0)
            continue; // Ignore non combat targets

        if(!IsValidUnitTarget(itr->first, (sp == NULL ? NULL : sp->info), (sp == NULL ? TargetFilter_None : sp->TargetFilter), (sp == NULL ? 0.0f : sp->mindist2cast), (sp == NULL ? m_outOfCombatRange : sp->maxdist2cast)))
            continue;

        if((itr->second + itr->first->GetThreatModifier()) > currentTarget.second &&
            itr->first != ResultUnit)
        {
            /* new target */
            currentTarget.first = itr->first;
            currentTarget.second = itr->second + itr->first->GetThreatModifier();
            m_currentHighestThreat = currentTarget.second;
        }
    }
    ai_TargetLock.Release();

    return currentTarget.first;
}

const static float baseAR[17] = {19.0f, 18.5f, 18.0f, 17.5f, 17.0f, 16.5f, 16.0f, 15.5f, 15.0f, 14.5f, 12.0f, 10.5f, 8.5f,  7.5f,  6.5f,  6.5f, 5.0f};

float AIInterface::_CalcAggroRange(Unit* target)
{
    ASSERT(m_Unit != NULL);
    if(!TO_CREATURE(m_Unit)->CanSee(target))
        return 0;

    int8 lvlDiff = target->getLevel() - m_Unit->getLevel();
    uint8 realLvlDiff = lvlDiff;
    if(lvlDiff > 8)
        lvlDiff = 8;
    if(lvlDiff < -8)
        lvlDiff = -8;

    float AggroRange = baseAR[lvlDiff + 8];

    // Check to see if the target is a player mining a node
    bool isMining = false;
    if(target->IsPlayer())
    {
        if(target->isCasting())
        {
            // If nearby miners weren't spotted already we'll give them a little surprise.
            Spell* sp = target->GetCurrentSpell();
            if(sp->GetSpellProto()->Effect[0] == SPELL_EFFECT_OPEN_LOCK && sp->GetSpellProto()->EffectMiscValue[0] == LOCKTYPE_MINING)
            {
                isMining = true;
            }
        }
    }

    // If the target is of a much higher level the aggro range must be scaled down, unless the target is mining a nearby resource node
    if(realLvlDiff > 8 && !isMining)
        AggroRange += AggroRange * ((lvlDiff - 8) * 5 / 100);

    // Multiply by elite value
    if(m_Unit->IsCreature() && TO_CREATURE(m_Unit)->GetCreatureInfo()->Rank > 0)
        AggroRange *= (TO_CREATURE(m_Unit)->GetCreatureInfo()->Rank) * 1.50f;

    if(AggroRange > 40.0f) // cap at 40.0f
        AggroRange = 40.0f;

    // SPELL_AURA_MOD_DETECT_RANGE
    int32 modDetectRange = target->getDetectRangeMod(m_Unit->GetGUID());
    AggroRange += modDetectRange;
    if(target->IsPlayer())
        AggroRange += TO_PLAYER( target )->DetectedRange;
    if(AggroRange < 5.0f)
        AggroRange = 5.0f;
    AggroRange *= 1.5f;
    if(AggroRange > 40.0f) // cap at 40.0f
        AggroRange = 40.0f;
    return (AggroRange*AggroRange);
}

float AIInterface::_CalcCombatRange(Unit* target, bool ranged)
{
    ASSERT(m_Unit != NULL);
    if(target == NULL)
        return 0.0f;

    float rang = ranged ? 5.0f : 0.0f;
    float selfreach = m_Unit->GetCombatReach();
    float targetradius = target->GetModelHalfSize();
    float selfradius = m_Unit->GetModelHalfSize();
    return (targetradius + selfreach + selfradius + rang);
}

uint32 AIInterface::getThreatByGUID(uint64 guid)
{
    ASSERT(m_Unit != NULL);

    Unit* obj = m_Unit->GetMapMgr()->GetUnit(guid);
    if(obj)
        return getThreatByPtr(obj);

    return 0;
}

uint32 AIInterface::getThreatByPtr(Unit* obj)
{
    ai_TargetLock.Acquire();
    TargetMap::iterator it = m_aiTargets.find(obj);
    if(it != m_aiTargets.end())
    {
        ai_TargetLock.Release();
        return it->second;
    }
    ai_TargetLock.Release();
    return 0;
}

bool AIInterface::modThreatByGUID(uint64 guid, int32 mod)
{
    ASSERT(m_Unit != NULL);

    if (!m_aiTargets.size())
        return false;

    Unit* obj = m_Unit->GetMapMgr()->GetUnit(guid);
    if(obj != NULL)
        return modThreatByPtr(obj, mod);
    return false;
}

bool AIInterface::modThreatByPtr(Unit* obj, int32 mod)
{
    ASSERT(m_Unit != NULL);
    if(obj == NULL)
        return false;

    int32 tempthreat;
    if( obj->mThreatRTarget && mod > 0)
    {
        int32 partmod = float2int32(mod * obj->mThreatRAmount);
        mod -= partmod;
        Unit* robj = obj->mThreatRTarget;
        if( partmod && robj && robj->isAlive() && obj->GetDistanceSq(robj) < 1600 )
        {
            ai_TargetLock.Acquire();
            TargetMap::iterator it = m_aiTargets.find(robj);
            if(it != m_aiTargets.end())
            {
                ai_TargetLock.Release();
                it->second += partmod;
                tempthreat = it->second + robj->GetThreatModifier();
                if(tempthreat < 1)
                    tempthreat = 1;
                if(tempthreat > m_currentHighestThreat)
                {
                    // new target!
                    if(!isTaunted)
                    {
                        m_currentHighestThreat = tempthreat;
                        SetNextTarget(robj);
                    }
                }
            }
            else
            {
                m_aiTargets.insert( make_pair( robj, partmod ) );
                ai_TargetLock.Release();
                tempthreat = partmod + robj->GetThreatModifier();
                if(tempthreat < 1)
                    tempthreat = 1;
                if(tempthreat > m_currentHighestThreat)
                {
                    if(!isTaunted)
                    {
                        m_currentHighestThreat = tempthreat;
                        SetNextTarget(robj);
                    }
                }
            }
        }
    }

    ai_TargetLock.Acquire();
    TargetMap::iterator it = m_aiTargets.find(obj);
    if( it != m_aiTargets.end() )
    {
        it->second += mod;
        if(it->second < 1)
            it->second = 1;
        tempthreat = it->second + obj->GetThreatModifier();
        ai_TargetLock.Release();

        if(tempthreat < 1)
            tempthreat = 1;
        if( tempthreat > m_currentHighestThreat )
        {
            // new target!
            if( !isTaunted )
            {
                m_currentHighestThreat = tempthreat;
                SetNextTarget(obj);
            }
        }
    }
    else
    {
        m_aiTargets.insert( make_pair( obj, mod ) );
        ai_TargetLock.Release();

        tempthreat = mod + obj->GetThreatModifier();
        if(tempthreat < 1)
            tempthreat = 1;
        if( tempthreat > m_currentHighestThreat )
        {
            if( !isTaunted )
            {
                m_currentHighestThreat = tempthreat;
                SetNextTarget(obj);
            }
        }
    }

    if(obj == m_nextTarget)
    {
        // check for a possible decrease in threat.
        if(mod < 0)
        {
            SetNextTarget(GetMostHated());
            //if there is no more new targets then we can walk back home ?
            if(!m_nextTarget)
                HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
        }
    }
    return true;
}

void AIInterface::RemoveThreatByPtr(Unit* obj)
{
    ASSERT(m_Unit != NULL);

    if(!obj)
        return;

    ai_TargetLock.Acquire();
    TargetMap::iterator it = m_aiTargets.find(obj);
    if(it != m_aiTargets.end())
    {
        m_aiTargets.erase(it);
        ai_TargetLock.Release();
        //check if we are in combat and need a new target
        if(obj == m_nextTarget)
        {
            SetNextTarget(GetMostHated());
            //if there is no more new targets then we can walk back home ?
            if( !m_nextTarget )
                HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
        }
    }
    else
        ai_TargetLock.Release();
}

void AIInterface::WipeHateList()
{
    ai_TargetLock.Acquire();
    for(TargetMap::iterator itr = m_aiTargets.begin(); itr != m_aiTargets.end(); itr++)
        itr->second = 0;
    ai_TargetLock.Release();
    m_currentHighestThreat = 0;
}

void AIInterface::ClearHateList() //without leaving combat
{
    ai_TargetLock.Acquire();
    for(TargetMap::iterator itr = m_aiTargets.begin(); itr != m_aiTargets.end(); itr++)
        itr->second = 1;
    ai_TargetLock.Release();
    m_currentHighestThreat = 1;
}

void AIInterface::WipeTargetList()
{
    ASSERT(m_Unit != NULL);
    SetNextTarget(NULLUNIT);
    m_CastNext = NULL;
    m_currentHighestThreat = 0;
    ai_TargetLock.Acquire();
    m_aiTargets.clear();
    ai_TargetLock.Release();
    m_Unit->CombatStatus.Vanished();
}

bool AIInterface::taunt(Unit* caster, bool apply)
{
    if(apply)
    {
        //wowwiki says that we cannot owerride this spell
        if(GetIsTaunted())
            return false;

        if(!caster)
        {
            isTaunted = false;
            return false;
        }

        //check if we can attack taunter. Maybe it's a hack or a bug if we fail this test
        if(sFactionSystem.isHostile(m_Unit, caster))
        {
            //check if we have to add him to our agro list
            //GetMostHated(); //update our most hated list/ Note that at this point we do not have a taunter yet. If we would have then this funtion will not give real mosthated
            int32 oldthreat = getThreatByPtr(caster);
            //make sure we rush the target anyway. Since we are not tauted yet, this will also set our target
            modThreatByPtr(caster,abs(m_currentHighestThreat-oldthreat)+1); //we need to be the most hated at this moment
//          SetNextTarget(caster);
        }
        isTaunted = true;
        tauntedBy = caster;
    }
    else
    {
        isTaunted = false;
        tauntedBy = NULLUNIT;
        //taunt is over, we should get a new target based on most hated list
        SetNextTarget(GetMostHated());
    }

    return true;
}

Unit* AIInterface::getTauntedBy()
{
    if(GetIsTaunted())
    {
        return tauntedBy;
    }
    else
    {
        return NULLUNIT;
    }
}

bool AIInterface::GetIsTaunted()
{
    if(isTaunted)
    {
        if(!tauntedBy || !tauntedBy->isAlive())
        {
            isTaunted = false;
            tauntedBy = NULLUNIT;
        }
    }
    return isTaunted;
}

void AIInterface::CheckTarget(Unit* target)
{
    ASSERT(m_Unit != NULL);

    if( target == NULL )
        return;

    if(target == getUnitToFollow())
        ClearFollowInformation(target);
    else if(target == getBackupUnitToFollow())
        ClearFollowInformation(target);

    ai_TargetLock.Acquire();
    TargetMap::iterator it2 = m_aiTargets.find( target );
    if( it2 != m_aiTargets.end() || target == m_nextTarget )
    {
        if(it2 != m_aiTargets.end())
            m_aiTargets.erase(it2);
        ai_TargetLock.Release();

        if (target == m_nextTarget)  // no need to cast on these.. mem addresses are still the same
        {
            SetNextTarget(NULLUNIT);
            m_CastNext = NULL;

            // find the one with the next highest threat
            GetMostHated();
        }
    }
    else
        ai_TargetLock.Release();

    if( target->GetTypeId() == TYPEID_UNIT )
    {
        target->GetAIInterface()->ai_TargetLock.Acquire();
        it2 = target->GetAIInterface()->m_aiTargets.find( m_Unit );
        if( it2 != target->GetAIInterface()->m_aiTargets.end() )
            target->GetAIInterface()->m_aiTargets.erase( it2 );
        target->GetAIInterface()->ai_TargetLock.Release();

        if( target->GetAIInterface()->m_nextTarget == m_Unit )
        {
            target->GetAIInterface()->m_nextTarget = NULLUNIT;
            target->GetAIInterface()->m_CastNext = NULL;
            target->GetAIInterface()->GetMostHated();
        }

        if(target->GetAIInterface()->getUnitToFollow() == m_Unit)
            target->GetAIInterface()->ClearFollowInformation(m_Unit);
    }

    if(target == getUnitToFear())
        SetUnitToFear(NULLUNIT);

    if(tauntedBy == target)
        tauntedBy = NULLUNIT;
}

uint32 AIInterface::_CalcThreat(uint32 damage, SpellEntry * sp, Unit* Attacker)
{
    ASSERT(m_Unit != NULL);

    if (sFactionSystem.isSameFaction(m_Unit,Attacker))
        return 0;

    int32 mod = 0;
    if( sp != NULL && sp->ThreatForSpell != 0 )
    {
        mod = sp->ThreatForSpell;
    }
    else
    {
        mod = damage;
    }

    if (sp != NULL && sp->SpellGroupType && Attacker)
    {
        SM_FIValue(Attacker->SM[SMT_THREAT_REDUCED][0],&mod,sp->SpellGroupType);
        SM_PIValue(Attacker->SM[SMT_THREAT_REDUCED][1],&mod,sp->SpellGroupType);
    }

    // modify mod by Affects
    mod += (mod * Attacker->GetGeneratedThreatModifier() / 100);

    return mod;
}

void AIInterface::WipeReferences()
{
    m_CastNext = 0;
    m_currentHighestThreat = 0;
    ai_TargetLock.Acquire();
    m_aiTargets.clear();
    ai_TargetLock.Release();
    SetNextTarget(NULLUNIT);
    SetUnitToFear(NULLUNIT);
    ClearFollowInformation();
    tauntedBy = NULLUNIT;
}
