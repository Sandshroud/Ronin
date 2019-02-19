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

AIInterface::AIInterface(Creature *creature, UnitPathSystem *unitPath, Unit *owner) : m_Creature(creature), m_path(unitPath), m_AISeed(RandomUInt()),
m_AIState(creature->isAlive() ? AI_STATE_IDLE : AI_STATE_DEAD), // Initialize AI state idle if unit is not dead
m_AIFlags(AI_FLAG_NONE)
{

}

AIInterface::~AIInterface()
{

}

void AIInterface::Init()
{
    if(m_Creature->isTrainingDummy())
        m_AIFlags |= AI_FLAG_DISABLED;
}

void AIInterface::Update(uint32 msTime, uint32 p_time)
{
    if(m_AIState == AI_STATE_DEAD || m_AIFlags & AI_FLAG_DISABLED)
        return;

    switch(m_AIState)
    {
    case AI_STATE_IDLE:
        {
            if(m_Creature->hasStateFlag(UF_EVADING))
            {
                if(!m_path->hasDestination())
                {
                    m_Creature->clearStateFlag(UF_EVADING);
                    m_path->EnableAutoPath();
                }
                return;
            }

            if(m_Creature->isCasting() && m_Creature->GetSpellInterface()->GetCurrentSpellProto()->isSpellInterruptOnMovement())
            {
                m_path->DisableAutoPath();
                return;
            }

            if(FindTarget(msTime) == false)
            {
                m_path->EnableAutoPath();
                return;
            }
        }
    case AI_STATE_COMBAT:
        {
            m_path->DisableAutoPath();
            m_AIState = AI_STATE_COMBAT;
            _HandleCombatAI(msTime);
        }break;
    case AI_STATE_SCRIPT:
        {
            if(!m_targetGuid.empty())
                _HandleCombatAI(msTime);
        }break;
    }
}

void AIInterface::OnAttackStop()
{
    m_targetGuid.Clean();
}

void AIInterface::OnDeath()
{
    m_targetGuid.Clean();

    m_AIState = AI_STATE_DEAD;
    m_Creature->EventAttackStop();
    m_Creature->clearStateFlag(UF_EVADING);
}

void AIInterface::OnRespawn()
{
    m_AIState = AI_STATE_IDLE;
}

void AIInterface::OnPathChange()
{

}

void AIInterface::OnStartCast(Spell *newSpell)
{
    SpellEntry *sp = newSpell->GetSpellProto();
    if ((sp->CastingTimeIndex || sp->isSpellInterruptOnMovement()) && m_path->hasDestination())
    {
        m_path->StopMoving();
        // Since we were moving, update our cast position
        newSpell->resetCastPosition();
    }
}

void AIInterface::OnTakeDamage(uint32 msTime, Unit *attacker, uint32 damage)
{
    if(m_AIFlags & AI_FLAG_DISABLED)
        return;

    if(m_targetGuid.empty() || m_AIState == AI_STATE_IDLE)
    {
        // On enter combat

    }

    // Set into combat AI State
    if(m_AIState <= AI_STATE_COMBAT)
        m_AIState = AI_STATE_COMBAT;
    // Set target guid for combat
    if(attacker && m_targetGuid.empty())
        m_targetGuid = attacker->GetGUID();
}

void AIInterface::OnAlterUnitState(uint32 modType)
{
    if(m_Creature->IsRooted() && !m_Creature->IsStunned())
    {
        m_path->ToggleMovementLock(true);
        if(m_path->hasDestination())
            m_path->StopMoving();
    }
    else if(m_Creature->IsStunned())
    {
        m_path->ToggleMovementLock(true);
        m_path->ToggleOrientationLock(true);
        if(m_path->hasDestination())
            m_path->StopMoving();
    }
    else if(m_path->IsMovementLocked() || m_path->IsOrientationLocked())
    {
        m_path->ToggleMovementLock(false);
        m_path->ToggleOrientationLock(false);
    }
}

bool AIInterface::FindTarget(uint32 msTime)
{
    if(m_AIFlags & AI_FLAG_DISABLED || !m_Creature->IsInWorld() || m_Creature->hasStateFlag(UF_EVADING) || m_Creature->IsStunned())
        return false;
    // If we're a non hostile faction, only search if we have a threatlist
    if(m_Creature->IsFactionNonHostile() && !m_Creature->IsInCombat())
        return false;
    if(!m_targetGuid.empty() || _PullTargetFromThreatList())
        return true;
    uint32 targetTypeMask = (TYPEMASK_TYPE_PLAYER | (m_Creature->IsFactionNPCHostile() ? TYPEMASK_TYPE_UNIT : 0x0000));

    float baseAggro = m_Creature->GetAggroRange();
    if(Unit *target = m_Creature->GetMapInstance()->FindInRangeTarget(msTime, m_Creature, baseAggro, targetTypeMask))
    {
        m_targetGuid = target->GetGUID();
        m_Creature->SetInCombat(target);
        return true;
    }
    return false;
}

void AIInterface::_HandleCombatAI(uint32 msTime)
{
    // Check if we're unable to utilize AI
    if(m_Creature->IsFeared())
        return;

    bool tooFarFromSpawn = false;
    Unit *unitTarget = m_Creature->GetInRangeObject<Unit>(m_targetGuid);
    if(m_Creature->isCasting() && m_Creature->GetSpellInterface()->GetCurrentSpellProto()->isSpellAttackInterrupting())
    {
        if(unitTarget) m_Creature->SetOrientation(m_Creature->GetAngle(unitTarget));
        return;
    }

    if (unitTarget == NULL || unitTarget->isDead() || (tooFarFromSpawn = (unitTarget->GetDistance2dSq(m_Creature->GetSpawnX(), m_Creature->GetSpawnY()) > MAX_COMBAT_MOVEMENT_DIST))
        || (tooFarFromSpawn = (unitTarget->CheckFlightStatus() && !m_Creature->canFly())) || !sFactionSystem.CanEitherUnitAttack(m_Creature, unitTarget))
    {
        // If we already have a target but he doesn't qualify back us out of combat
        if(unitTarget)
        {
            if(m_path->hasDestination())
                m_path->StopMoving();
            m_targetGuid.Clean();
            m_Creature->EventAttackStop();
        } else m_targetGuid.Clean();

        bool returnToSpawn = true;
        if(tooFarFromSpawn == false && unitTarget == NULL)
            if((returnToSpawn = !FindTarget(msTime)) == false && (unitTarget = m_Creature->GetInRangeObject<Unit>(m_targetGuid)) == NULL)
                returnToSpawn = true;

        if(returnToSpawn)
        {
            m_AIState = AI_STATE_IDLE;
            m_targetGuid.Clean();
            m_Creature->EventAttackStop();
            if(!m_path->hasDestination())
            {
                m_Creature->addStateFlag(UF_EVADING);
                m_path->EnterEvade();
            }
            return;
        }
    }

    SpellEntry *sp = NULL;
    float attackRange = 0.f, minRange = 0.f, x, y, z, o, dist = unitTarget->GetDistance2dSq(m_Creature->GetPositionX(), m_Creature->GetPositionY());
    if (!m_path->IsMovementLocked() && m_Creature->calculateAttackRange(m_Creature->GetPreferredAttackType(&sp), minRange, attackRange, sp) && (dist > (attackRange*attackRange*0.9f)))
    {
        bool forceRecalcPath = false;

        // Grab our current position for our xyz pos
        if (m_path->hasDestination())
        {
            m_path->GetDestination(x, y);
            float destDiff = m_Creature->GetDistance2dSq(x, y);
            // Recalculate movement path if our target has moved closer
            forceRecalcPath = dist < destDiff && (RONIN_UTIL::Diff(dist, destDiff) > attackRange*0.2f);
        } else m_Creature->GetPosition(x, y, z);

        float dist2 = unitTarget->GetDistance2dSq(x, y);
        unitTarget->GetPosition(x, y, z);

        // Check to see if our current destination is in reach of our target
        if (forceRecalcPath || ((dist2 > (attackRange*attackRange*1.2f)) && (dist >= attackRange * attackRange)))
        {
            o = m_Creature->calcAngle(m_Creature->GetPositionX(), m_Creature->GetPositionY(), x, y) * M_PI / 180.f;
            x -= attackRange * 0.8f * cosf(o);
            y -= attackRange * 0.8f * sinf(o);

            m_path->SetSpeed(MOVE_SPEED_RUN);//unitTarget->GetCombatMovement());
            m_path->MoveToPoint(x, y, z, o);
        } else if (!m_path->IsOrientationLocked() && !m_path->CheckFinalOrientation(m_Creature->calcAngle(m_Creature->GetPositionX(), m_Creature->GetPositionY(), x, y) * M_PI / 180.f))
            m_path->UpdateOrientation(unitTarget);
    } else if(!m_Creature->isTargetInFront(unitTarget) && !m_path->IsOrientationLocked())
        m_path->UpdateOrientation(unitTarget);

    if(!m_Creature->checkAttackTarget(m_targetGuid))
        m_Creature->EventAttackStart(m_targetGuid);
    m_Creature->GetMapInstance()->TriggerCombatTimer(m_Creature->GetGUID(), unitTarget->GetGUID(), 5000);
}

bool AIInterface::_PullTargetFromThreatList()
{
    return false;
}
