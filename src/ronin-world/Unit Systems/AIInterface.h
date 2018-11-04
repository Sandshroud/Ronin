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

#pragma once

class CreatureData;
class CreatureInfoExtra;
class UnitPathSystem;

#define MAX_RANDOM_MOVEMENT_DIST 25.f
#define MAX_COMBAT_MOVEMENT_DIST 14400.f

enum AI_State : uint8
{
    AI_STATE_DEAD   = 0,
    AI_STATE_IDLE   = 1,
    AI_STATE_COMBAT = 2,
    AI_STATE_SCRIPT = 0xFF
};

enum AI_Flags : uint16
{
    AI_FLAG_NONE        = 0x0000,
    AI_FLAG_DISABLED    = 0x8000
};

class SERVER_DECL AIInterface
{
public:
    AIInterface(Creature *creature, UnitPathSystem *unitPath, Unit *owner = NULL);
    ~AIInterface();

    // Post creation initialization call
    void Init();

    // Update
    void Update(uint32 msTime, uint32 p_time);

    void OnAttackStop();
    void OnDeath();
    void OnRespawn();
    void OnPathChange();
    void OnStartCast(SpellEntry *sp);
    void OnTakeDamage(uint32 msTime, Unit *attacker, uint32 damage);
    void OnAlterUnitState(uint32 modType);

    bool FindTarget(uint32 msTime);

    AI_State GetAIState() { return m_AIState; }

protected:
    AI_State m_AIState;
    uint32 m_AISeed;
    uint32 m_AIFlags;

    void _HandleCombatAI(uint32 msTime);
    bool _PullTargetFromThreatList();

private:
    Creature* m_Creature;
    UnitPathSystem *m_path;

    WoWGuid m_targetGuid;
    std::map<WoWGuid, uint32> m_threatMap;
};
