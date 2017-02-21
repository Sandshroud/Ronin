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

uint32 Spell::GetTargetType(uint32 implicittarget, uint32 i)
{
    uint32 type = m_implicitTargetFlags[implicittarget];

    //CHAIN SPELLS ALWAYS CHAIN!
    uint32 jumps = m_spellInfo->EffectChainTarget[i];
    if(m_caster->IsUnit())
        castPtr<Unit>(m_caster)->SM_FIValue(SMT_JUMP_REDUCE, (int32*)&jumps, m_spellInfo->SpellGroupType);
    if(jumps != 0)
        type |= SPELL_TARGET_AREA_CHAIN;
    return type;
}

void Spell::FillTargetMap(bool fromDelayed)
{
    bool ignoreAOE = m_isDelayedAOEMissile && fromDelayed == false;
    uint32 targetTypes[3] = {SPELL_TARGET_NOT_IMPLEMENTED, SPELL_TARGET_NOT_IMPLEMENTED, SPELL_TARGET_NOT_IMPLEMENTED};
    // Set destination position for target types when we have target pos flags on current target
    for(uint32 i = 0; i < 3; i++)
    {
        if(m_spellInfo->Effect[i] == 0)
            continue;

        // Fill from A regardless
        targetTypes[i] = GetTargetType(m_spellInfo->EffectImplicitTargetA[i], i);

        //never get info from B if it is 0 :P
        if(m_spellInfo->EffectImplicitTargetB[i] != 0)
            targetTypes[i] |= GetTargetType(m_spellInfo->EffectImplicitTargetB[i], i);

        if(targetTypes[i] & SPELL_TARGET_AREA_CURTARGET)
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

    // Fill out our different targets for spell effects
    for(uint8 i = 0; i < 3; i++)
    {
        float radius = GetRadius(i);
        if(targetTypes[i] & SPELL_TARGET_NOT_IMPLEMENTED)
            continue;

        if(targetTypes[i] & SPELL_TARGET_NO_OBJECT)  //summon spells that appear infront of caster
        {
            HandleTargetNoObject();
            continue;
        }

        bool inWorld = m_caster->IsInWorld(); //always add this guy :P
        if(inWorld && !(targetTypes[i] & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE | SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_OBJECT_PETOWNER)))
            if(Unit* target = m_caster->GetInRangeObject<Unit>(m_targets.m_unitTarget))
                AddTarget(i, targetTypes[i], target);

        // We can always push self to our target map
        if(targetTypes[i] & SPELL_TARGET_OBJECT_SELF)
            AddTarget(i, targetTypes[i], m_caster);

        if (inWorld && m_caster->IsUnit() && (targetTypes[i] & SPELL_TARGET_OBJECT_CURTOTEMS))
        {
            std::vector<Creature*> m_totemList;
            castPtr<Unit>(m_caster)->FillSummonList(m_totemList, SUMMON_TYPE_TOTEM);
            for(std::vector<Creature*>::iterator itr = m_totemList.begin(); itr != m_totemList.end(); itr++)
                AddTarget(i, targetTypes[i], *itr);
        }

        if(radius && inWorld)
        {   // These require that we're in world with people around us
            if(ignoreAOE == false && (targetTypes[i] & SPELL_TARGET_AREA)) // targetted aoe
                AddAOETargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets);
            if((targetTypes[i] & SPELL_TARGET_AREA_SELF)) // targetted aoe near us
                AddAOETargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets);

            //targets party, not raid
            if((targetTypes[i] & SPELL_TARGET_AREA_PARTY) && !(targetTypes[i] & SPELL_TARGET_AREA_RAID))
            {
                if(!m_caster->IsPlayer() && !(m_caster->IsCreature() || m_caster->IsTotem()))
                    AddAOETargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets); //npcs
                else AddPartyTargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets); //players/pets/totems
            }

            if(targetTypes[i] & SPELL_TARGET_AREA_RAID)
            {
                if(!m_caster->IsPlayer() && !(m_caster->IsCreature() || m_caster->IsTotem()))
                    AddAOETargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets); //npcs
                else AddRaidTargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets, (targetTypes[i] & SPELL_TARGET_AREA_PARTY) ? true : false); //players/pets/totems
            }

            if(targetTypes[i] & SPELL_TARGET_AREA_CHAIN)
                AddChainTargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets);

            //target cone
            if(targetTypes[i] & SPELL_TARGET_AREA_CONE)
                AddConeTargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets);

            if(targetTypes[i] & SPELL_TARGET_OBJECT_SCRIPTED)
                AddScriptedOrSpellFocusTargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets);
        }

        // Allow auto self target, especially when map is empty
        if(!m_targets.hasDestination() && (m_effectTargetMaps[i].empty() && (m_targets.m_unitTarget.empty() || m_targets.m_unitTarget == m_caster->GetGUID())))
            AddTarget(i, SPELL_TARGET_NONE, m_caster);
    }
}

void Spell::HandleTargetNoObject()
{
    float dist = 3;
    float newx = m_caster->GetPositionX() + cosf(m_caster->GetOrientation()) * dist;
    float newy = m_caster->GetPositionY() + sinf(m_caster->GetOrientation()) * dist;
    float newz = m_caster->GetMapInstance()->GetWalkableHeight(m_caster, newx, newy, m_caster->GetPositionZ());

    //if not in line of sight, or too far away we summon inside caster
    if(fabs(newz - m_caster->GetPositionZ()) > 10 || !sVMapInterface.CheckLOS(m_caster->GetMapId(), m_caster->GetInstanceID(), m_caster->GetPhaseMask(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ() + 2, newx, newy, newz + 2))
    {
        newx = m_caster->GetPositionX();
        newy = m_caster->GetPositionY();
        newz = m_caster->GetPositionZ();
    }

    m_targets.m_targetMask |= TARGET_FLAG_DEST_LOCATION;
    m_targets.m_dest.ChangeCoords(newx, newy, newz);
}

bool Spell::AddTarget(uint32 i, uint32 TargetType, WorldObject* obj)
{
    if(obj == NULL || (obj != m_caster && !obj->IsInWorld()))
        return false;

    //GO target, not item
    if((TargetType & SPELL_TARGET_REQUIRE_GAMEOBJECT) && !(TargetType & SPELL_TARGET_REQUIRE_ITEM) && !obj->IsGameObject())
        return false;

    //target go, not able to target go
    if(obj->IsGameObject() && !(TargetType & SPELL_TARGET_OBJECT_SCRIPTED) && !(TargetType & SPELL_TARGET_REQUIRE_GAMEOBJECT) && !m_triggeredSpell)
        return false;
    //target item, not able to target item
    if(obj->IsItem() && !(TargetType & SPELL_TARGET_REQUIRE_ITEM) && !m_triggeredSpell)
        return false;

    if(m_caster->IsUnit() && castPtr<Unit>(m_caster)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IGNORE_PC) && (obj->IsPlayer() || m_caster->IsPlayer()))
        return false;

    if(TargetType & SPELL_TARGET_REQUIRE_FRIENDLY && !sFactionSystem.isFriendly(m_caster, obj))
        return false;
    if(TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE && !sFactionSystem.isAttackable(m_caster, obj))
        return false;
    if(TargetType & SPELL_TARGET_OBJECT_TARCLASS)
    {
        WorldObject* originaltarget = m_caster->GetInRangeObject(m_targets.m_unitTarget);
        if(originaltarget == NULL || (originaltarget->IsPlayer() && obj->IsPlayer() && castPtr<Player>(originaltarget)->getClass() != castPtr<Player>(obj)->getClass()) || (originaltarget->IsPlayer() && !obj->IsPlayer()) || (!originaltarget->IsPlayer() && obj->IsPlayer()))
            return false;
    }

    if(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID) && ((obj->IsUnit() && !castPtr<Unit>(obj)->isAlive()) || (obj->IsCreature() && obj->IsTotem())))
        return false;

    _AddTarget(obj, i);

    //final checks, require line of sight unless range/radius is 50000 yards
    SpellRangeEntry* r = dbcSpellRange.LookupEntry(m_spellInfo->rangeIndex);
    if(sWorld.Collision && r->maxRangeHostile < 50000 && GetRadius(i) < 50000 && !obj->IsItem())
    {
        float x = m_caster->GetPositionX(), y = m_caster->GetPositionY(), z = m_caster->GetPositionZ() + 0.5f;

        //are we using a different location?
        if(TargetType & SPELL_TARGET_AREA)
        {
            x = m_targets.m_dest.x;
            y = m_targets.m_dest.y;
            z = m_targets.m_dest.z;
        }
        else if(TargetType & SPELL_TARGET_AREA_CHAIN)
        {
            //TODO: Support
        }

        if(!sVMapInterface.CheckLOS(m_caster->GetMapId(), m_caster->GetInstanceID(), m_caster->GetPhaseMask(), x, y, z + 2, obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ() + 2))
            return false;
    }
    return true;
}

void Spell::AddAOETargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    LocationVector source;

    //cant do raid/party stuff here, seperate functions for it
    if(TargetType & (SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID) && !(!m_caster->IsPlayer() && !(m_caster->IsCreature() || m_caster->IsTotem())))
        return;

    WorldObject* tarobj = m_caster->GetInRangeObject(m_targets.m_unitTarget);

    if(TargetType & SPELL_TARGET_AREA_SELF)
        source = m_caster->GetPosition();
    else if(TargetType & SPELL_TARGET_AREA_CURTARGET && tarobj != NULL)
        source = tarobj->GetPosition();
    else
    {
        m_targets.m_targetMask |= TARGET_FLAG_DEST_LOCATION;
        source = m_targets.m_dest;
    }

    float range = r*r; //caster might be in the aoe LOL
    if(m_caster->GetDistanceSq(source) <= range)
        AddTarget(i, TargetType, m_caster);

    WorldObject *wObj = NULL;
    /*for(WorldObject::InRangeHashMap::iterator itr = m_caster->GetInRangeMapBegin(); itr != m_caster->GetInRangeMapEnd(); itr++ )
    {
        if((wObj = itr->second) == NULL)
            continue;
        if(maxtargets != 0 && m_effectTargetMaps[i].size() >= maxtargets)
            break;
        if(wObj->GetDistanceSq(source) > range)
            continue;
        AddTarget(i, TargetType, wObj);
    }*/
}

void Spell::AddPartyTargets(uint32 i, uint32 TargetType, float radius, uint32 maxtargets)
{
    WorldObject* u = m_caster->GetInRangeObject(m_targets.m_unitTarget);
    if(u == NULL && (u = m_caster) == NULL)
        return;
    if(!u->IsPlayer())
        return;

    Player* p = castPtr<Player>(u);
    AddTarget(i, TargetType, p);

    float range = radius*radius;
    /*WorldObject::InRangeArray::iterator itr;
    for(itr = u->GetInRangePlayerSetBegin(); itr != u->GetInRangePlayerSetEnd(); itr++)
    {
        Player *target = u->GetInRangeObject<Player>(*itr);
        if(target == NULL || !target->isAlive())
            continue;
        if(!p->IsGroupMember(target))
            continue;
        if(u->GetDistanceSq(target) > range)
            continue;

        AddTarget(i, TargetType, target);
    }*/
}

void Spell::AddRaidTargets(uint32 i, uint32 TargetType, float radius, uint32 maxtargets, bool partylimit)
{
    WorldObject* u = m_caster->GetInRangeObject(m_targets.m_unitTarget);
    if(u == NULL && (u = m_caster) == NULL)
        return;
    if(!u->IsPlayer())
        return;

    Player* p = castPtr<Player>(u);
    AddTarget(i, TargetType, p);

    float range = radius*radius;
    /*WorldObject::InRangeArray::iterator itr;
    for(itr = u->GetInRangePlayerSetBegin(); itr != u->GetInRangePlayerSetEnd(); itr++)
    {
        Player *target = u->GetInRangeObject<Player>(*itr);
        if(target == NULL || !target->isAlive())
            continue;
        if(!p->IsGroupMember(target))
            continue;
        if(u->GetDistanceSq(target) > range)
            continue;

        AddTarget(i, TargetType, target);
    }*/
}

void Spell::AddChainTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    if(!m_caster->IsInWorld())
        return;

    WorldObject* targ = m_caster->GetInRangeObject(m_targets.m_unitTarget);
    if(targ == NULL)
        return;

    //if selected target is party member, then jumps on party
    Unit* firstTarget = NULL;
    if(targ->IsUnit())
        firstTarget = castPtr<Unit>(targ);
    else if(m_caster->IsUnit())
        firstTarget = castPtr<Unit>(m_caster);

    bool RaidOnly = false;
    float range = m_spellInfo->maxRange[0];
    //this is cast distance, not searching distance
    range *= range;

    //is this party only?
    Player* casterFrom = NULL;
    if(m_caster->IsPlayer())
        casterFrom = castPtr<Player>(m_caster);

    Player* pfirstTargetFrom = NULL;
    if(firstTarget->IsPlayer())
        pfirstTargetFrom = castPtr<Player>(firstTarget);

    if(casterFrom != NULL && pfirstTargetFrom != NULL && casterFrom->GetGroup() == pfirstTargetFrom->GetGroup())
        RaidOnly = true;

    uint32 jumps = m_spellInfo->EffectChainTarget[i];

    //range
    range /= jumps; //hacky, needs better implementation!

    if(m_spellInfo->SpellGroupType && m_caster->IsUnit())
        castPtr<Unit>(m_caster)->SM_FIValue(SMT_JUMP_REDUCE, (int32*)&jumps, m_spellInfo->SpellGroupType);

    AddTarget(i, TargetType, firstTarget);

    if(jumps <= 1 || m_effectTargetMaps[i].size() == 0) //1 because we've added the first target, 0 size if spell is resisted
        return;

    /*WorldObject::InRangeArray::iterator itr;
    for(itr = firstTarget->GetInRangeUnitSetBegin(); itr != firstTarget->GetInRangeUnitSetEnd(); itr++)
    {
        Unit *target = m_caster->GetInRangeObject<Unit>(*itr);
        if(target == NULL || !target->isAlive())
            continue;

        if(RaidOnly)
        {
            if(!target->IsPlayer())
                continue;

            if(!pfirstTargetFrom->IsGroupMember(castPtr<Player>(target)))
                continue;
        }

        //healing spell, full health target = NONO
        if(IsHealingSpell(m_spellInfo) && target->GetHealthPct() == 100)
            continue;

        size_t oldsize;
        if(IsInrange(firstTarget->GetPositionX(), firstTarget->GetPositionY(), firstTarget->GetPositionZ(), target, range))
        {
            oldsize = m_effectTargetMaps[i].size();
            AddTarget(i, TargetType, target);
            if(m_effectTargetMaps[i].size() == oldsize || m_effectTargetMaps[i].size() >= jumps) //either out of jumps or a resist
                return;
        }
    }*/
}

void Spell::AddConeTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    /*WorldObject::InRangeArray::iterator itr;
    for(itr = m_caster->GetInRangeUnitSetBegin(); itr != m_caster->GetInRangeUnitSetEnd(); itr++)
    {
        Unit *target = m_caster->GetInRangeObject<Unit>(*itr);
        if(!target->isAlive())
            continue;

        //is Creature in range
        if(m_caster->isInRange(target, GetRadius(i)))
        {
            if(m_caster->isTargetInFront(target))  // !!! is the target within our cone ?
            {
                AddTarget(i, TargetType, target);
            }
        }
        if(maxtargets != 0 && m_effectTargetMaps[i].size() >= maxtargets)
            return;
    }*/
}

void Spell::AddScriptedOrSpellFocusTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    /*for(WorldObject::InRangeArray::iterator itr = m_caster->GetInRangeGameObjectSetBegin(); itr != m_caster->GetInRangeGameObjectSetEnd(); itr++ )
    {
        if(GameObject* go = m_caster->GetInRangeObject<GameObject>(*itr))
        {
            if(go->GetInfo()->data.spellFocus.focusId == m_spellInfo->RequiresSpellFocus)
            {
                if(!m_caster->isInRange(go, r))
                    continue;

                if(AddTarget(i, TargetType, go))
                    return;
            }
        }
    }*/
}

// returns Guid of lowest percentage health friendly party or raid target within sqrt('dist') yards
uint64 Spell::FindLowestHealthRaidMember(Player* Target, uint32 dist)
{
    if(!Target || !Target->IsInWorld())
        return 0;

    uint64 lowestHealthTarget = Target->GetGUID();
    uint32 lowestHealthPct = Target->GetHealthPct();
    if(Group *group = Target->GetGroup())
    {
        group->Lock();
        for(uint32 j = 0; j < group->GetSubGroupCount(); ++j) {
            for(GroupMembersSet::iterator itr = group->GetSubGroup(j)->GetGroupMembersBegin(); itr != group->GetSubGroup(j)->GetGroupMembersEnd(); itr++)
            {
                if((*itr)->m_loggedInPlayer && Target->GetDistance2dSq((*itr)->m_loggedInPlayer) <= dist)
                {
                    uint32 healthPct = (*itr)->m_loggedInPlayer->GetHealthPct();
                    if(healthPct < lowestHealthPct)
                    {
                        lowestHealthPct = healthPct;
                        lowestHealthTarget = (*itr)->m_loggedInPlayer->GetGUID();
                    }
                }
            }
        }
        group->Unlock();
    }
    return lowestHealthTarget;
}

