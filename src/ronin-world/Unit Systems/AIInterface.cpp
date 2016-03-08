/***
 * Demonstrike Core
 */

#include "StdAfx.h"

AIInterface::AIInterface(Unit *unit, Unit *owner) : m_Unit(unit)
{

}

AIInterface::~AIInterface()
{

}

void AIInterface::Update(uint32 p_time)
{

}

/*void AIInterface::_UpdateTargetLocation()
{
	Unit *unitTarget = m_Unit->GetInRangeObject<Unit>(m_unitMoveTarget);
	if (unitTarget == NULL || unitTarget->isDead())
	{
		m_path.StopMoving();
		return;
	}

	float followRange = 0.f, x, y, z, o;
	if (m_Unit->validateAttackTarget(unitTarget))
	{
		SpellEntry *sp = NULL;
		float minRange = 0.f; followRange = 5.f;
		if (m_Unit->calculateAttackRange(m_Unit->GetPreferredAttackType(&sp), minRange, followRange, sp))
			followRange *= 0.8f;
	}
	else followRange = 5.f;//m_Unit->GetFollowRange(unitTarget);

	m_Unit->GetPosition(x, y, z);
	m_path.GetDestination(x, y);
	if (unitTarget->GetDistance2dSq(x, y) < followRange*followRange)
		return;

	unitTarget->GetPosition(x, y, z);
	o = m_Unit->calcAngle(m_serverLocation->x, m_serverLocation->y, x, y) * M_PI / 180.f;
	x -= followRange * cosf(o);
	y -= followRange * sinf(o);
	m_path.MoveToPoint(x, y, z, o);
}*/
