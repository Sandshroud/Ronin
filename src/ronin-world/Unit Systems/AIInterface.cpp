/***
 * Demonstrike Core
 */

#include "StdAfx.h"

AIInterface::AIInterface(Unit *unit, UnitPathSystem *unitPath, Unit *owner) : m_Unit(unit), m_path(unitPath), m_AISeed(RandomUInt()), m_findTargetLockout(1), m_waypointCounter(0), m_waypointMap(NULL),
m_AIState(unit->isAlive() ? AI_STATE_IDLE : AI_STATE_DEAD) // Initialize AI state idle if unit is not dead
{

}

AIInterface::~AIInterface()
{

}

void AIInterface::Update(uint32 p_time)
{
    if(m_AIState == AI_STATE_DEAD)
        return;

    if(m_findTargetLockout)
        --m_findTargetLockout;

    if(m_waypointWaitTimer > p_time)
        m_waypointWaitTimer -= p_time;
    else m_waypointWaitTimer = 0;

    switch(m_AIState)
    {
    case AI_STATE_IDLE:
        {
            if(m_Unit->hasStateFlag(UF_EVADING))
            {
                if(!m_path->hasDestination())
                    m_Unit->clearStateFlag(UF_EVADING);
                return;
            }

            if(FindTarget() == false)
            {
                if(!m_path->hasDestination())
                {
                    if(m_pendingWaitTimer)
                    {
                        m_waypointWaitTimer = m_pendingWaitTimer;
                        m_pendingWaitTimer = 0;
                    }
                    else if(m_waypointWaitTimer == 0)
                        FindNextPoint();
                } // The update block should stop here
                return;
            }
        }
    case AI_STATE_COMBAT:
        {
            m_AIState = AI_STATE_COMBAT;
            _HandleCombatAI();
        }break;
    case AI_STATE_SCRIPT:
        {
            if(!m_targetGuid.empty())
                _HandleCombatAI();
        }break;
    }
}

void AIInterface::OnDeath()
{
    m_targetGuid.Clean();
    m_path->StopMoving();
    m_AIState = AI_STATE_DEAD;
    m_Unit->EventAttackStop();
    m_Unit->clearStateFlag(UF_EVADING);
}

void AIInterface::OnPathChange()
{

}

void AIInterface::OnTakeDamage(Unit *attacker, uint32 damage)
{
    if(m_targetGuid.empty() || m_AIState == AI_STATE_IDLE)
    {
        // On enter combat

    }

    // Set into combat AI State
    if(m_AIState <= AI_STATE_COMBAT)
        m_AIState = AI_STATE_COMBAT;
    // Set target guid for combat
    if(attacker && m_targetGuid.empty())
    {
        m_targetGuid = attacker->GetGUID();
        _HandleCombatAI();
    }
}

bool AIInterface::FindTarget()
{
    if(m_Unit->hasStateFlag(UF_EVADING) || !m_targetGuid.empty() || !m_Unit->HasInRangeObjects())
        return false;
    if(m_findTargetLockout)
        return false;
    m_findTargetLockout = m_AISeed%3;

    Unit *target = NULL;
    float baseAggro = 20.f, targetDist = 0.f;
    // detect range auras
    if(AuraInterface::modifierMap *modMap = m_Unit->m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DETECT_RANGE))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            baseAggro += itr->second->m_amount;

    // Begin iterating through our inrange units
    for(WorldObject::InRangeSet::iterator itr = m_Unit->GetInRangeUnitSetBegin(); itr != m_Unit->GetInRangeUnitSetEnd(); itr++)
    {
        if(Unit *unitTarget = m_Unit->GetInRangeObject<Unit>(*itr))
        {
            if(unitTarget->isDead())
                continue;
            float dist = m_Unit->GetDistanceSq(unitTarget);
            float aggroRange = m_Unit->ModAggroRange(unitTarget, baseAggro);
            if(dist > 50.f)
                continue;
            if(dist >= MAX_COMBAT_MOVEMENT_DIST || dist >= aggroRange)
                continue;
            if(target && targetDist <= dist)
                continue;
            if(!sFactionSystem.isHostile(m_Unit, unitTarget))
                continue;
            // LOS is a big system hit so do it last
            if(!m_Unit->IsInLineOfSight(unitTarget))
                continue;

            target = unitTarget;
            targetDist = dist;
        }
    }

    if(target)
    {
        m_targetGuid = target->GetGUID();
        return true;
    }
    return false;
}

void AIInterface::FindNextPoint()
{
    // Waiting before processing next waypoint
    if(m_waypointWaitTimer)
        return;

    if(m_waypointMap && !m_waypointMap->empty())
    {   // Process waypoint map travel

        return;
    }

    float distance = m_Unit->GetDistanceSq(m_Unit->GetSpawnX(), m_Unit->GetSpawnY(), m_Unit->GetSpawnZ());

    if(false)//m_Unit->HasRandomMovement())
    {
        if(distance < MAX_RANDOM_MOVEMENT_DIST)
        {   // Process random movement point generation

            return;
        } // Else we just return to spawn point and generate a random point then
    }
    if(distance == 0.f)
        return;

    // Move back to our original spawn point
    m_path->MoveToPoint(m_Unit->GetSpawnX(), m_Unit->GetSpawnY(), m_Unit->GetSpawnZ(), m_Unit->GetSpawnO());
}

void AIInterface::_HandleCombatAI()
{
    Unit *unitTarget = m_Unit->GetInRangeObject<Unit>(m_targetGuid);
    if (unitTarget == NULL || unitTarget->isDead() || (unitTarget->GetDistance2dSq(m_Unit->GetSpawnX(), m_Unit->GetSpawnY()) > MAX_COMBAT_MOVEMENT_DIST)
        || !sFactionSystem.CanEitherUnitAttack(m_Unit, unitTarget))
    {
        // If we already have a target but he doesn't qualify back us out of combat
        if(unitTarget)
        {
            m_path->StopMoving();
            m_targetGuid.Clean();
            m_Unit->EventAttackStop();
        }

        if(FindTarget() == false)
        {
            m_AIState = AI_STATE_IDLE;
            if(!m_path->hasDestination())
            {
                m_Unit->addStateFlag(UF_EVADING);
                FindNextPoint();
            }
            return;
        }
    }

    SpellEntry *sp = NULL;
    float attackRange = 0.f, minRange = 0.f, x, y, z, o;
    if (m_Unit->calculateAttackRange(m_Unit->GetPreferredAttackType(&sp), minRange, attackRange, sp))
        attackRange *= 0.8f; // Cut our attack range down slightly to prevent range issues

    m_Unit->GetPosition(x, y, z);
    m_path->GetDestination(x, y);
    if (unitTarget->GetDistance2dSq(x, y) >= attackRange*attackRange)
    {
        unitTarget->GetPosition(x, y, z);
        o = m_Unit->calcAngle(m_Unit->GetPositionX(), m_Unit->GetPositionY(), x, y) * M_PI / 180.f;
        x -= attackRange * cosf(o);
        y -= attackRange * sinf(o);
        m_path->MoveToPoint(x, y, z, o);
    } else m_Unit->SetOrientation(m_Unit->GetAngle(unitTarget));

    if(!m_Unit->ValidateAttackTarget(m_targetGuid))
        m_Unit->EventAttackStart(m_targetGuid);
}