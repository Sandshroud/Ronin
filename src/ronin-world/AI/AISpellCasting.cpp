/***
 * Demonstrike Core
 */

#include "StdAfx.h"

SpellEntry *AIInterface::getSpellEntry(uint32 spellId)
{
    SpellEntry *spellInfo = dbcSpell.LookupEntry(spellId );
    if(!spellInfo)
    {
        sLog.outDebug("WORLD: unknown spell id %i\n", spellId);
        return NULL;
    }

    return spellInfo;
}

void AIInterface::CancelSpellCast()
{
    ASSERT(m_Unit != NULL);

    m_CastNext = NULL;
    if( m_Unit && m_Unit->m_currentSpell)
        m_Unit->m_currentSpell->cancel();
}

void AIInterface::addSpellToList(AI_Spell *sp)
{
    ASSERT(m_Unit != NULL);
    if(sp->info == NULL || sp->info->Id == 0)
        return;

    AI_Spell* nSP = new AI_Spell(sp, sp->cooldown, sp->procCounter);
    if(nSP->casttime == 0) nSP->casttime = GetDBCCastTime(dbcSpellCastTime.LookupEntry( nSP->info->CastingTimeIndex ));
    if(nSP->maxdist2cast == 0.0f) nSP->maxdist2cast = GetDBCMaxRange( dbcSpellRange.LookupEntry( nSP->info->rangeIndex ) );
    if(nSP->mindist2cast == 0.0f) nSP->mindist2cast = GetDBCMinRange( dbcSpellRange.LookupEntry( nSP->info->rangeIndex ) );
    if(nSP->cooldown == 0) nSP->cooldown = nSP->info->StartRecoveryTime; //avoid spell spamming
    if(nSP->cooldown == 0) nSP->cooldown = nSP->info->StartRecoveryCategory; //still 0 ?
    if(nSP->cooldown == 0) nSP->cooldown = 4000; //omg, avoid spamming at least
    m_spells.insert(std::make_pair(nSP->info->Id, nSP));
}

bool AIInterface::CanCastAISpell(AI_Spell* toCast, uint32 currentTime)
{
    if(toCast->cooldown)
    {
        if((toCast->lastcast+toCast->cooldown) > currentTime)
            return false;
    }

    if(toCast->ProcLimit)
    {
        if(toCast->ProcResetDelay)
        {
            if(toCast->ProcResetTimer <= currentTime)
            {
                toCast->procCounter = 0;
                toCast->ProcResetTimer = currentTime+toCast->ProcResetDelay;
            }
        }

        if(toCast->procCounter >= toCast->ProcLimit)
            return false;
    }

    if(toCast->info->powerType == POWER_TYPE_MANA)
    {
        int32 currentPower = m_Unit->GetPower(POWER_TYPE_MANA);

        int32 cost;
        if( toCast->info->ManaCostPercentage)//Percentage spells cost % of !!!BASE!!! mana
            cost = (m_Unit->GetUInt32Value(UNIT_FIELD_BASE_MANA)*toCast->info->ManaCostPercentage)/100;
        else 
            cost = toCast->info->ManaCost;
        cost += m_Unit->PowerCostMod[toCast->info->School];//this is not percent!
        cost += float2int32(cost*m_Unit->GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+toCast->info->School));

        //apply modifiers
        if( toCast->info->SpellGroupType )
        {
            m_Unit->SM_FIValue(SMT_COST, &cost, toCast->info->SpellGroupType);
            m_Unit->SM_PIValue(SMT_COST, &cost, toCast->info->SpellGroupType);
        }

        if(cost > currentPower)
            return false;
    }

    return true;
}

void AIInterface::CastAISpell(Unit* Target, AI_Spell* toCast, uint32 currentTime)
{
    if(m_CastNext == toCast)
    {
        m_CastNext = NULL;
        SetBehaviorType(Behavior_Default);
    }

    if(Target == NULL)
        return;

    if(toCast->ProcLimit)
        toCast->procCounter++;

    if(toCast->info->IsSpellChannelSpell())
    {
        m_CastTimer = 1000;
        int32 duration = GetSpellInfoDuration(toCast->info, m_Unit, Target);
        if(duration > 0)
            m_CastTimer += duration;
    } else m_CastTimer = 1000+toCast->casttime;

    toCast->lastcast = currentTime;
    if(toCast->attackstoptimer)
        m_Unit->setAttackTimer(m_CastTimer+toCast->attackstoptimer, false);
    StopMovement(m_CastTimer);
    m_AIState = STATE_CASTING;

    toCast->mPredefinedTarget = NULL;

    switch(toCast->TargetType)
    {
    case TargetGen_Self:
    case TargetGen_Current:
    case TargetGen_Predefined:
    case TargetGen_RandomUnit:
    case TargetGen_RandomPlayer:
    case TargetGen_ManaClass:
        m_Unit->CastSpell(Target, toCast->info, false, toCast->casttime);
        break;

    case TargetGen_RandomUnitApplyAura:
    case TargetGen_RandomPlayerApplyAura:
        Target->CastSpell(Target, toCast->info, false, toCast->casttime);
        break;

    case TargetGen_Destination:
    case TargetGen_RandomUnitDestination:
    case TargetGen_RandomPlayerDestination:
        m_Unit->CastSpellAoF(Target->GetPositionX(), Target->GetPositionY(), Target->GetPositionZ(), toCast->info, false, toCast->casttime);
        break;
    }
}

bool AIInterface::IsValidUnitTarget( WorldObject *pObject, SpellEntry *info, uint32 pFilter, float pMinRange, float pMaxRange )
{
    // Make sure its a valid unit
    if (!pObject->IsUnit() )
        return false;

    Unit *UnitTarget = castPtr<Unit>( pObject );
    if ( UnitTarget->GetInstanceID() != m_Unit->GetInstanceID() )
        return false;

    float dist = 0.0f;
    if(UnitTarget != m_Unit)
    {
        dist = m_Unit->CalcDistance( UnitTarget );
        if ( m_outOfCombatRange && UnitTarget->GetDistanceSq( GetReturnPos() ) > m_outOfCombatRange )
            return false;
    }

    if ( pFilter & TargetFilter_Corpse )
    {   //Skip dead ( if required ), feign death or invisible targets
        if ( UnitTarget->isAlive() || !UnitTarget->IsCreature() || castPtr<Creature>( UnitTarget )->GetCreatureData()->Rank == ELITE_WORLDBOSS )
            return false;
    }
    else if ( !UnitTarget->isAlive() )
        return false;

    if ( UnitTarget->IsPlayer() && castPtr<Player>( UnitTarget )->m_isGmInvisible )
        return false;
    if ( UnitTarget->HasFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH ) )
        return false;
    // If it's a damaging spell, we can cast again, otherwise, do not double cast
    if(info != NULL && !IsDamagingSpell(info) && UnitTarget->HasAura(info->Id))
        return false;

    //Check if we apply target filtering
    if ( pFilter != TargetFilter_None )
    {
        //Skip units not on threat list
        if ( ( pFilter & TargetFilter_Aggroed ) && getThreat( UnitTarget->GetGUID() ) == 0 )
            return false;

        //Skip current attacking target if requested
        if ( ( pFilter & TargetFilter_NotCurrent ) && UnitTarget == GetNextTarget() )
            return false;

        //Keep only wounded targets if requested
        if ( ( pFilter & TargetFilter_Wounded ) && UnitTarget->GetHealthPct() >= 99 )
            return false;

        //Skip targets not in melee range if requested
        if ( ( pFilter & TargetFilter_InMeleeRange ) && dist > _CalcCombatRange( UnitTarget, false ) )
            return false;

        //Skip targets not in strict range if requested
        if ( ( pFilter & TargetFilter_InRangeOnly ) && ( pMinRange > 0 || pMaxRange > 0 ) )
        {
            if ( pMinRange > 0 && dist < pMinRange )
                return false;
            if ( pMaxRange > 0 && dist > pMaxRange )
                return false;
        }

        //Skip targets not in Line Of Sight if requested
        if ( ( ~pFilter & TargetFilter_IgnoreLineOfSight ) && !m_Unit->IsInLineOfSight( UnitTarget ) )
            return false;

        //Handle hostile/friendly
        if ( ( ~pFilter & TargetFilter_Corpse ) && ( pFilter & TargetFilter_Friendly ) ) 
        {
            if ( !UnitTarget->CombatStatus.IsInCombat() )
                return false; //Skip not-in-combat targets if friendly
            if ( sFactionSystem.CanEitherUnitAttack( m_Unit, UnitTarget ) || getThreat( UnitTarget->GetGUID() ) > 0 )
                return false;
        }
    }

    return true; //This is a valid unit target
}

Unit *AIInterface::GetNearestTargetInSet(std::set<Unit*> pTargetSet)
{
    Unit *NearestUnit = NULL;
    float Distance, NearestDistance = 99999;
    for ( std::set<Unit*>::iterator UnitIter = pTargetSet.begin(); UnitIter != pTargetSet.end(); ++UnitIter )
    {
        Distance = m_Unit->CalcDistance( castPtr<Unit>( *UnitIter ) );
        if ( Distance < NearestDistance )
        {
            NearestDistance = Distance;
            NearestUnit = ( *UnitIter );
        }
    }

    return NearestUnit;
}

Unit *AIInterface::GetSecondMostHatedTargetInSet(std::set<Unit*> pTargetSet )
{
    Unit *  TargetUnit = NULL;
    Unit *  MostHatedUnit = NULL;
    Unit *  CurrentTarget = castPtr<Unit>( m_nextTarget );
    uint32  Threat = 0, HighestThreat = 0;
    for ( std::set<Unit*>::iterator UnitIter = pTargetSet.begin(); UnitIter != pTargetSet.end(); ++UnitIter )
    {
        TargetUnit = castPtr<Unit>( *UnitIter );
        if ( TargetUnit != CurrentTarget )
        {
            Threat = getThreat( TargetUnit->GetGUID() );
            if ( Threat > HighestThreat )
            {
                MostHatedUnit = TargetUnit;
                HighestThreat = Threat;
            }
        }
    }

    return MostHatedUnit;
}

Unit *AIInterface::ChooseBestTargetInSet( std::set<Unit*> pTargetSet, uint32 pTargetFilter )
{
    // If we don't have a size, return null
    if( pTargetSet.size() == 0 )
        return NULL;

    //If only one possible target, return it
    if ( pTargetSet.size() == 1 )
        return (*pTargetSet.begin());

    //Find closest unit if requested
    if ( pTargetFilter & TargetFilter_Closest )
        return GetNearestTargetInSet( pTargetSet );

    //Find second most hated if requested
    if ( pTargetFilter & TargetFilter_SecondMostHated )
        return GetSecondMostHatedTargetInSet( pTargetSet );

    uint32 balance = RandomUInt((uint32)pTargetSet.size()-1);
    std::set<Unit*>::iterator itr = pTargetSet.begin();
    for(uint32 i = 0; i < balance; i++)
        itr++;

    // Choose random unit in array
    return (*itr);
}

Unit *AIInterface::GetBestUnitTarget( SpellEntry *info, uint32 pTargetFilter, float pMinRange, float pMaxRange)
{
    //Build potential target list
    std::set<Unit*> TargetSet;
    for ( WorldObject::InRangeMap::iterator ObjectIter = m_Unit->GetInRangeMapBegin(); ObjectIter != m_Unit->GetInRangeMapEnd(); ++ObjectIter )
    {
        if( IsValidUnitTarget(ObjectIter->second, info, pTargetFilter, pMinRange, pMaxRange) )
        {
            if(pTargetFilter & TargetFilter_ManaClass && castPtr<Unit>(ObjectIter->second)->getPowerType() != POWER_TYPE_MANA)
                continue;
            TargetSet.insert( castPtr<Unit>( ObjectIter->second ) );
        }
    }

    if ( pTargetFilter & TargetFilter_Friendly && IsValidUnitTarget( m_Unit, info, pTargetFilter ) )
        TargetSet.insert( m_Unit ); //Also add self as possible friendly target
    return ChooseBestTargetInSet( TargetSet, pTargetFilter );
}

Unit *AIInterface::GetBestPlayerTarget( SpellEntry *info, uint32 pTargetFilter, float pMinRange, float pMaxRange)
{
    //Build potential target list
    std::set<Unit*> TargetSet;
    for ( PlayerSet::iterator PlayerIter = m_Unit->GetInRangePlayerSetBegin(); PlayerIter != m_Unit->GetInRangePlayerSetEnd(); PlayerIter++ ) 
    {
        if ( IsValidUnitTarget( *PlayerIter, info, pTargetFilter, pMinRange, pMaxRange ) )
            TargetSet.insert( castPtr<Unit>( *PlayerIter ) );
    }

    return ChooseBestTargetInSet( TargetSet, pTargetFilter );
}

Unit *AIInterface::GetTargetForSpell( AI_Spell* pSpell )
{
    if(pSpell == NULL)
        return NULL;

    // Find a suitable target for the described situation :)
    switch( pSpell->TargetType )
    {
    case TargetGen_RandomPlayer:
    case TargetGen_RandomPlayerApplyAura:
    case TargetGen_RandomPlayerDestination:
        return GetBestPlayerTarget( pSpell->info, pSpell->TargetFilter, pSpell->mindist2cast, pSpell->maxdist2cast );
    case TargetGen_RandomUnit:
    case TargetGen_RandomUnitApplyAura:
    case TargetGen_RandomUnitDestination:
        return GetBestUnitTarget( pSpell->info, pSpell->TargetFilter, pSpell->mindist2cast, pSpell->maxdist2cast );
    default:
        {
            //Check if run-to-target cache and return it if its valid
            if ( m_nextTarget && IsValidUnitTarget( m_nextTarget, pSpell ) )
                return m_nextTarget;

            switch( pSpell->TargetType )
            {
            case TargetGen_ManaClass:
                return GetBestUnitTarget( pSpell->info, pSpell->TargetFilter, pSpell->mindist2cast, pSpell->maxdist2cast );
            case TargetGen_Self:
                {
                    Unit *m_Result = m_Unit;
                    if(pSpell->mPredefinedTarget != NULL)
                        m_Result = pSpell->mPredefinedTarget;

                    if ( !m_Result->isAlive() )
                        m_Result = NULL;
                    else if ( (pSpell->TargetFilter & TargetFilter_Wounded) && m_Result->GetHealthPct() >= 99 )
                        m_Result = NULL;
                    else if(!IsValidUnitTarget(m_Result, pSpell))
                        m_Result = NULL;
                    return m_Result;
                }break;
            case TargetGen_SecondMostHated:
                {
                    Unit* m_Result = GetSecondHated(pSpell);
                    if(m_Result == NULL)
                        m_Result = GetMostHated(pSpell);
                    if(!IsValidUnitTarget(m_Result, pSpell))
                        return NULL;
                    return m_Result;
                }break;
            case TargetGen_Current:
            case TargetGen_Destination:
                return NULL; // This is handled by previous checks, but whatever
            case TargetGen_Predefined:
                {
                    Unit *m_Result = pSpell->mPredefinedTarget;
                    if(!IsValidUnitTarget(m_Result, pSpell))
                        return NULL;
                    return m_Result;
                }break;
            default:
                sLog.outDebug("MoonScriptCreatureAI::GetTargetForSpell() : Invalid target type!\n");
                return NULL;
            }
        }
    }
}
