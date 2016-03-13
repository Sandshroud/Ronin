/***
 * Demonstrike Core
 */

#include "StdAfx.h"

AIInterface::AIInterface(Unit *unit, UnitPathSystem *unitPath, Unit *owner) : m_Unit(unit), m_path(unitPath), m_waypointCounter(0), m_waypointMap(NULL),
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

            if(!m_path->hasDestination() && FindTarget() == false)
            {
                FindNextPoint();
                return;
            }
        }
    case AI_STATE_COMBAT:
        {
            Unit *unitTarget = NULL;
            while(true)
            {
                unitTarget = m_Unit->GetInRangeObject<Unit>(m_targetGuid);
                if (unitTarget == NULL || unitTarget->isDead() || (unitTarget->GetDistance2dSq(m_Unit->GetSpawnX(), m_Unit->GetSpawnY()) > MAX_COMBAT_MOVEMENT_DIST)
                    || !sFactionSystem.CanEitherUnitAttack(m_Unit, unitTarget))
                {
                    m_path->StopMoving();
                    m_targetGuid.Clean();
                    m_Unit->EventAttackStop();
                    if(FindTarget() == false)
                    {
                        m_Unit->addStateFlag(UF_EVADING);
                        m_AIState = AI_STATE_IDLE;
                        FindNextPoint();
                        return;
                    }
                    continue;
                }
                break;
            }

            m_AIState = AI_STATE_COMBAT;
            float attackRange = 0.f, x, y, z, o;
            if (m_Unit->validateAttackTarget(unitTarget))
            {
                SpellEntry *sp = NULL;
                float minRange = 0.f; attackRange = 5.f;
                if (m_Unit->calculateAttackRange(m_Unit->GetPreferredAttackType(&sp), minRange, attackRange, sp))
                    attackRange *= 0.8f;
            } else attackRange = 5.f;//m_Unit->GetFollowRange(unitTarget);

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
        }break;
    case AI_STATE_SCRIPT:
        {

        }break;
    }
}

void AIInterface::OnDeath()
{
    m_path->StopMoving();
    m_AIState = AI_STATE_DEAD;
    m_Unit->EventAttackStop();
    m_Unit->clearStateFlag(UF_EVADING);
}

bool AIInterface::FindTarget()
{
    if(m_Unit->hasStateFlag(UF_EVADING))
        return false;

    Unit *target = NULL;
    float targetDist = 0.f;
    for(WorldObject::InRangeSet::iterator itr = m_Unit->GetInRangeUnitSetBegin(); itr != m_Unit->GetInRangeUnitSetEnd(); itr++)
    {
        if(Unit *unitTarget = m_Unit->GetInRangeObject<Unit>(*itr))
        {
            float aggroRange = m_Unit->ModAggroRange(unitTarget, 20.f);
            if(!m_Unit->isInRange(unitTarget, aggroRange))
                continue;
            float dist = m_Unit->GetDistanceSq(unitTarget);
            if(target && targetDist <= dist)
                continue;
            if(!sFactionSystem.CanEitherUnitAttack(m_Unit, unitTarget))
                continue;

            target = unitTarget;
            targetDist = dist;
        }
    }

    if(target)
        m_targetGuid = target->GetGUID();
    return target != NULL;
}

void AIInterface::FindNextPoint()
{
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
