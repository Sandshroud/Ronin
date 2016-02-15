/***
 * Demonstrike Core
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

/// Fill the target map with the targets
/// the targets are specified with numbers and handled accordingly
void Spell::FillTargetMap(uint32 i)
{
    if(!m_caster->IsInWorld())
        return;

    // Get our info from A regardless of nullity
    uint32 TargetType = GetTargetType(m_spellInfo->EffectImplicitTargetA[i], i);

    //never get info from B if it is 0 :P
    if(m_spellInfo->EffectImplicitTargetB[i] != 0)
        TargetType |= GetTargetType(m_spellInfo->EffectImplicitTargetB[i], i);
    if(TargetType & SPELL_TARGET_NOT_IMPLEMENTED)
        return;
    if(TargetType & SPELL_TARGET_NO_OBJECT)  //summon spells that appear infront of caster
    {
        HandleTargetNoObject();
        return;
    }

    //always add this guy :P
    if(!(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE | SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_OBJECT_PETOWNER)))
        if(WorldObject* target = m_caster->GetMapInstance()->_GetObject(m_targets.m_unitTarget))
            AddTarget(i, TargetType, target);

    if(TargetType & SPELL_TARGET_OBJECT_SELF)
        AddTarget(i, TargetType, m_caster);

    if(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF))  //targetted aoe
        AddAOETargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets);
    if (TargetType & SPELL_TARGET_OBJECT_CURTOTEMS && m_caster->IsUnit())
    {
        std::vector<Creature*> m_totemList;
        castPtr<Unit>(m_caster)->FillSummonList(m_totemList, SUMMON_TYPE_TOTEM);
        for(std::vector<Creature*>::iterator itr = m_totemList.begin(); itr != m_totemList.end(); itr++)
            AddTarget(i, TargetType, *itr);
    }

    if(TargetType & SPELL_TARGET_OBJECT_CURPET && m_caster->IsPlayer())
        AddTarget(i, TargetType, castPtr<Player>(m_caster)->GetSummon());

    if(TargetType & SPELL_TARGET_OBJECT_PETOWNER)
    {
        if(m_targets.m_unitTarget.getHigh() == HIGHGUID_TYPE_PET)
        {
            if(Pet* p = m_caster->GetMapInstance()->GetPet(m_targets.m_unitTarget))
                AddTarget(i, TargetType, p->GetPetOwner());
        }
    }

    //targets party, not raid
    if((TargetType & SPELL_TARGET_AREA_PARTY) && !(TargetType & SPELL_TARGET_AREA_RAID))
    {
        if(!m_caster->IsPlayer() && !m_caster->IsPet() && (!m_caster->IsCreature() || !m_caster->IsTotem()))
            AddAOETargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets); //npcs
        else AddPartyTargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets); //players/pets/totems
    }

    if(TargetType & SPELL_TARGET_AREA_RAID)
    {
        if(!m_caster->IsPlayer() && !m_caster->IsPet() && (!m_caster->IsCreature() || !m_caster->IsTotem()))
            AddAOETargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets); //npcs
        else AddRaidTargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets, (TargetType & SPELL_TARGET_AREA_PARTY) ? true : false); //players/pets/totems
    }

    if(TargetType & SPELL_TARGET_AREA_CHAIN)
        AddChainTargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets);

    //target cone
    if(TargetType & SPELL_TARGET_AREA_CONE)
        AddConeTargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets);

    if(TargetType & SPELL_TARGET_OBJECT_SCRIPTED)
        AddScriptedOrSpellFocusTargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets);
}

void Spell::HandleTargetNoObject()
{
    float dist = 3;
    float newx = m_caster->GetPositionX() + cosf(m_caster->GetOrientation()) * dist;
    float newy = m_caster->GetPositionY() + sinf(m_caster->GetOrientation()) * dist;
    float newz = m_caster->GetMapHeight(newx, newy, m_caster->GetPositionZ());

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
    if(obj == NULL || !obj->IsInWorld())
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

    if(m_caster->IsUnit() && castPtr<Unit>(m_caster)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9) && ((obj->IsPlayer() || obj->IsPet()) || (m_caster->IsPlayer() || m_caster->IsPet())))
        return false;

    if(TargetType & SPELL_TARGET_REQUIRE_FRIENDLY && !sFactionSystem.isFriendly(m_caster, obj))
        return false;
    if(TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE && !sFactionSystem.isAttackable(m_caster, obj))
        return false;
    if(TargetType & SPELL_TARGET_OBJECT_TARCLASS)
    {
        WorldObject* originaltarget = m_caster->GetMapInstance()->_GetObject(m_targets.m_unitTarget);

        if(originaltarget == NULL || (originaltarget->IsPlayer() && obj->IsPlayer() && castPtr<Player>(originaltarget)->getClass() != castPtr<Player>(obj)->getClass()) || (originaltarget->IsPlayer() && !obj->IsPlayer()) || (!originaltarget->IsPlayer() && obj->IsPlayer()))
            return false;
    }
    if(TargetType & SPELL_TARGET_OBJECT_CURPET && !obj->IsPet())
        return false;
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
    if(TargetType & (SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID) && !(!m_caster->IsPlayer() && !m_caster->IsPet() && (!m_caster->IsCreature() || !m_caster->IsTotem())))
        return;

    WorldObject* tarobj = m_caster->GetMapInstance()->_GetObject(m_targets.m_unitTarget);

    if(TargetType & SPELL_TARGET_AREA_SELF)
        source = m_caster->GetPosition();
    else if(TargetType & SPELL_TARGET_AREA_CURTARGET && tarobj != NULL)
        source = tarobj->GetPosition();
    else
    {
        m_targets.m_targetMask |= TARGET_FLAG_DEST_LOCATION;
        source = m_targets.m_dest;
    }

    //caster might be in the aoe LOL
    if(m_caster->CalcDistance(source) <= r)
        AddTarget(i, TargetType, m_caster);

    WorldObject *wObj = NULL;
    for(WorldObject::InRangeMap::iterator itr = m_caster->GetInRangeMapBegin(); itr != m_caster->GetInRangeMapEnd(); itr++ )
    {
        if((wObj = itr->second) == NULL)
            continue;
        if(maxtargets != 0 && m_effectTargetMaps[i].size() >= maxtargets)
            break;
        if(wObj->CalcDistance(source) > r)
            continue;
        AddTarget(i, TargetType, wObj);
    }
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

    WorldObject::InRangeSet::iterator itr;
    for(itr = u->GetInRangePlayerSetBegin(); itr != u->GetInRangePlayerSetEnd(); itr++)
    {
        Player *target = u->GetInRangeObject<Player>(*itr);
        if(target == NULL || !target->isAlive())
            continue;
        if(!p->IsGroupMember(target))
            continue;
        if(Pet *pet = target->GetSummon())
        {
            if(u->CalcDistance(pet) <= radius)
                AddTarget(i, TargetType, pet);
        }
        if(u->CalcDistance(target) > radius)
            continue;

        AddTarget(i, TargetType, target);
    }
}

void Spell::AddRaidTargets(uint32 i, uint32 TargetType, float radius, uint32 maxtargets, bool partylimit)
{
    WorldObject* u = m_caster->GetMapInstance()->_GetObject(m_targets.m_unitTarget);
    if(u == NULL && (u = m_caster) == NULL)
        return;
    if(!u->IsPlayer())
        return;

    Player* p = castPtr<Player>(u);
    AddTarget(i, TargetType, p);

    WorldObject::InRangeSet::iterator itr;
    for(itr = u->GetInRangePlayerSetBegin(); itr != u->GetInRangePlayerSetEnd(); itr++)
    {
        Player *target = u->GetInRangeObject<Player>(*itr);
        if(target == NULL || !target->isAlive())
            continue;
        if(!p->IsGroupMember(target))
            continue;
        if(Pet *pet = target->GetSummon())
        {
            if(u->CalcDistance(pet) <= radius)
                AddTarget(i, TargetType, pet);
        }
        if(u->CalcDistance(target) > radius)
            continue;

        AddTarget(i, TargetType, target);
    }
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

    WorldObject::InRangeSet::iterator itr;
    for(itr = firstTarget->GetInRangeUnitSetBegin(); itr != firstTarget->GetInRangeUnitSetEnd(); itr++)
    {
        Unit *target = m_caster->GetInRangeObject<Unit>(*itr);
        if(!target->isAlive())
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
    }
}

void Spell::AddConeTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    WorldObject::InRangeSet::iterator itr;
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
    }
}

void Spell::AddScriptedOrSpellFocusTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    for(WorldObject::InRangeSet::iterator itr = m_caster->GetInRangeGameObjectSetBegin(); itr != m_caster->GetInRangeGameObjectSetEnd(); itr++ )
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
    }
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

