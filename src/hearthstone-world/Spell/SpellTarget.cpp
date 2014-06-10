/***
 * Demonstrike Core
 */

#include "StdAfx.h"

uint32 implicitTargetFlags[150];

uint32 Spell::GetTargetType(uint32 implicittarget, uint32 i)
{
    uint32 type = implicitTargetFlags[implicittarget];

    //CHAIN SPELLS ALWAYS CHAIN!
    uint32 jumps = m_spellInfo->EffectChainTarget[i];
    if(u_caster != NULL)
        SM_FIValue(u_caster->SM[SMT_JUMP_REDUCE][0], (int32*)&jumps, m_spellInfo->SpellGroupType);
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

    uint32 TargetType = SPELL_TARGET_NONE;

    // Get our info from A regardless of nullity
    TargetType |= GetTargetType(m_spellInfo->EffectImplicitTargetA[i], i);

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
    {
        Object* target = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);
        AddTarget(i, TargetType, target);
    }

    if(TargetType & SPELL_TARGET_OBJECT_SELF)
        AddTarget(i, TargetType, m_caster);

    if(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF))  //targetted aoe
        AddAOETargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets);
    if (TargetType & SPELL_TARGET_OBJECT_CURTOTEMS && u_caster != NULL)
    {
        std::vector<Creature*> m_totemList;
        u_caster->FillSummonList(m_totemList, SUMMON_TYPE_TOTEM);
        for(std::vector<Creature*>::iterator itr = m_totemList.begin(); itr != m_totemList.end(); itr++)
            AddTarget(i, TargetType, *itr);
    }

    if(TargetType & SPELL_TARGET_OBJECT_CURPET && p_caster != NULL)
        AddTarget(i, TargetType, p_caster->GetSummon());

    if(TargetType & SPELL_TARGET_OBJECT_PETOWNER)
    {
        uint64 guid = m_targets.m_unitTarget;
        if(GUID_HIPART(guid) == HIGHGUID_TYPE_PET)
        {
            Pet* p = m_caster->GetMapMgr()->GetPet(GUID_LOPART(guid));

            if(p != NULL)
                AddTarget(i, TargetType, p->GetPetOwner());
        }
    }

    //targets party, not raid
    if((TargetType & SPELL_TARGET_AREA_PARTY) && !(TargetType & SPELL_TARGET_AREA_RAID))
    {
        if(p_caster == NULL && !m_caster->IsPet() && (!m_caster->IsCreature() || !m_caster->IsTotem()))
            AddAOETargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets); //npcs
        else
            AddPartyTargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets); //players/pets/totems
    }

    if(TargetType & SPELL_TARGET_AREA_RAID)
    {
        if(p_caster == NULL && !m_caster->IsPet() && (!m_caster->IsCreature() || !m_caster->IsTotem()))
            AddAOETargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets); //npcs
        else
            AddRaidTargets(i, TargetType, GetRadius(i), m_spellInfo->MaxTargets, (TargetType & SPELL_TARGET_AREA_PARTY) ? true : false); //players/pets/totems
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
    float newz = m_caster->GetPositionZ();

    //clamp Z
    newz = m_caster->GetCHeightForPosition(true, newx, newy, newz);

    //if not in line of sight, or too far away we summon inside caster
    if(fabs(newz - m_caster->GetPositionZ()) > 10 || !sVMapInterface.CheckLOS(m_caster->GetMapId(), m_caster->GetInstanceID(), m_caster->GetPhaseMask(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ() + 2, newx, newy, newz + 2))
    {
        newx = m_caster->GetPositionX();
        newy = m_caster->GetPositionY();
        newz = m_caster->GetPositionZ();
    }

    m_targets.m_targetMask |= TARGET_FLAG_DEST_LOCATION;
    m_targets.m_destX = newx;
    m_targets.m_destY = newy;
    m_targets.m_destZ = newz;
}

bool Spell::AddTarget(uint32 i, uint32 TargetType, Object* obj)
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

    if(u_caster != NULL && u_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9) && ((obj->IsPlayer() || obj->IsPet()) || (p_caster != NULL || m_caster->IsPet())))
        return false;

    if(TargetType & SPELL_TARGET_REQUIRE_FRIENDLY && !sFactionSystem.isFriendly(m_caster, obj))
        return false;
    if(TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE && !sFactionSystem.isAttackable(m_caster, obj))
        return false;
    if(TargetType & SPELL_TARGET_OBJECT_TARCLASS)
    {
        Object* originaltarget = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);

        if(originaltarget == NULL || (originaltarget->IsPlayer() && obj->IsPlayer() && TO_PLAYER(originaltarget)->getClass() != TO_PLAYER(obj)->getClass()) || (originaltarget->IsPlayer() && !obj->IsPlayer()) || (!originaltarget->IsPlayer() && obj->IsPlayer()))
            return false;
    }
    if(TargetType & SPELL_TARGET_OBJECT_CURPET && !obj->IsPet())
        return false;
    if(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID) && ((obj->IsUnit() && !TO_UNIT(obj)->isAlive()) || (obj->IsCreature() && obj->IsTotem())))
        return false;

    if(TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE && obj->IsUnit())
        _AddTarget(TO_UNIT(obj), i);
    else
        _AddTargetForced(obj->GetGUID(), i);

    //final checks, require line of sight unless range/radius is 50000 yards
    SpellRange* r = dbcSpellRange.LookupEntry(m_spellInfo->rangeIndex);
    if(sWorld.Collision && r->maxRangeHostile < 50000 && GetRadius(i) < 50000 && !obj->IsItem())
    {
        float x = m_caster->GetPositionX(), y = m_caster->GetPositionY(), z = m_caster->GetPositionZ() + 0.5f;

        //are we using a different location?
        if(TargetType & SPELL_TARGET_AREA)
        {
            x = m_targets.m_destX;
            y = m_targets.m_destY;
            z = m_targets.m_destZ;
        }
        else if(TargetType & SPELL_TARGET_AREA_CHAIN)
        {
            //TODO: Add support for this in arcemu
            /*Object* lasttarget = NULL;
            if (m_orderedObjects.size() > 0)
            {
                lasttarget = m_caster->GetMapMgr()->_GetObject(m_orderedObjects[m_orderedObjects.size() - 1]);
                if (lasttarget != NULL)
                {
                    x = lasttarget->GetPositionX();
                    y = lasttarget->GetPositionY();
                    z = lasttarget->GetPositionZ();
                }
            }*/
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
    if(TargetType & (SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID) && !(p_caster == NULL && !m_caster->IsPet() && (!m_caster->IsCreature() || !m_caster->IsTotem())))
        return;

    Object* tarobj = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);

    if(TargetType & SPELL_TARGET_AREA_SELF)
        source = m_caster->GetPosition();
    else if(TargetType & SPELL_TARGET_AREA_CURTARGET && tarobj != NULL)
        source = tarobj->GetPosition();
    else
    {
        m_targets.m_targetMask |= TARGET_FLAG_DEST_LOCATION;
        source.x = m_targets.m_destX;
        source.y = m_targets.m_destY;
        source.z = m_targets.m_destZ;
    }

    //caster might be in the aoe LOL
    if(m_caster->CalcDistance(source) <= r)
        AddTarget(i, TargetType, m_caster);

    for(ObjectSet::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); ++itr)
    {
        if(maxtargets != 0 && ManagedTargets.size() >= maxtargets)
            break;

        float dist = (*itr)->CalcDistance(source);
        if(dist <= r)
            AddTarget(i, TargetType, (*itr));
    }
}

void Spell::AddPartyTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    Object* u = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);
    if(u == NULL)
        u = m_caster;

    if(u == NULL || u_caster == NULL || !u->IsPlayer())
        return;

    Player* p = TO_PLAYER(u);
    AddTarget(i, TargetType, p);

    ObjectSet::iterator itr;
    for(itr = u->GetInRangeSetBegin(); itr != u->GetInRangeSetEnd(); itr++)
    {
        if(!(*itr)->IsUnit() || !TO_UNIT(*itr)->isAlive())
            continue;

        //only affect players and pets
        if(!(*itr)->IsPlayer() && !(*itr)->IsPet())
            continue;

        if(!p->IsGroupMember(TO_PLAYER(*itr)))
            continue;

        if(u->CalcDistance(*itr) > r)
            continue;

        AddTarget(i, TargetType, (*itr));
    }
}

void Spell::AddRaidTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets, bool partylimit)
{
    Object* u = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);
    if(u == NULL)
        u = m_caster;

    if(u == NULL || u_caster == NULL || !u->IsPlayer())
        return;

    Player* p = TO_PLAYER(u);
    AddTarget(i, TargetType, p);

    ObjectSet::iterator itr;
    for(itr = u->GetInRangeSetBegin(); itr != u->GetInRangeSetEnd(); itr++)
    {
        if(!(*itr)->IsUnit() || !TO_UNIT(*itr)->isAlive())
            continue;

        //only affect players and pets
        if(!(*itr)->IsPlayer() && !(*itr)->IsPet())
            continue;

        if(!p->IsGroupMember(TO_PLAYER(*itr)))
            continue;

        if(u->CalcDistance(*itr) > r)
            continue;

        AddTarget(i, TargetType, (*itr));
    }
}

void Spell::AddChainTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    if(!m_caster->IsInWorld())
        return;

    Object* targ = m_caster->GetMapMgr()->_GetObject(m_targets.m_unitTarget);
    if(targ == NULL)
        return;

    //if selected target is party member, then jumps on party
    Unit* firstTarget = NULL;
    if(targ->IsUnit())
        firstTarget = TO_UNIT(targ);
    else
        firstTarget = u_caster;

    bool RaidOnly = false;
    float range = GetDBCMaxRange(dbcSpellRange.LookupEntry(m_spellInfo->rangeIndex));//this is probably wrong,
    //this is cast distance, not searching distance
    range *= range;

    //is this party only?
    Player* casterFrom = NULL;
    if(u_caster->IsPlayer())
        casterFrom = TO_PLAYER(u_caster);

    Player* pfirstTargetFrom = NULL;
    if(firstTarget->IsPlayer())
        pfirstTargetFrom = TO_PLAYER(firstTarget);

    if(casterFrom != NULL && pfirstTargetFrom != NULL && casterFrom->GetGroup() == pfirstTargetFrom->GetGroup())
        RaidOnly = true;

    uint32 jumps = m_spellInfo->EffectChainTarget[i];

    //range
    range /= jumps; //hacky, needs better implementation!

    if(m_spellInfo->SpellGroupType && u_caster != NULL)
        SM_FIValue(u_caster->SM[SMT_JUMP_REDUCE][0], (int32*)&jumps, m_spellInfo->SpellGroupType);

    AddTarget(i, TargetType, firstTarget);

    if(jumps <= 1 || ManagedTargets.size() == 0) //1 because we've added the first target, 0 size if spell is resisted
        return;

    ObjectSet::iterator itr;
    for(itr = firstTarget->GetInRangeSetBegin(); itr != firstTarget->GetInRangeSetEnd(); itr++)
    {
        if(!(*itr)->IsUnit() || !TO_UNIT((*itr))->isAlive())
            continue;

        if(RaidOnly)
        {
            if(!(*itr)->IsPlayer())
                continue;

            if(!pfirstTargetFrom->IsGroupMember(TO_PLAYER(*itr)))
                continue;
        }

        //healing spell, full health target = NONO
        if(IsHealingSpell(m_spellInfo) && TO_UNIT(*itr)->GetHealthPct() == 100)
            continue;

        size_t oldsize;
        if(IsInrange(firstTarget->GetPositionX(), firstTarget->GetPositionY(), firstTarget->GetPositionZ(), (*itr), range))
        {
            oldsize = ManagedTargets.size();
            AddTarget(i, TargetType, (*itr));
            if(ManagedTargets.size() == oldsize || ManagedTargets.size() >= jumps) //either out of jumps or a resist
                return;
        }
    }
}

void Spell::AddConeTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    ObjectSet::iterator itr;
    for(itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); itr++)
    {
        if(!((*itr)->IsUnit()) || !TO_UNIT((*itr))->isAlive())
            continue;

        //is Creature in range
        if(m_caster->isInRange(TO_UNIT(*itr), GetRadius(i)))
        {
            if(m_spellInfo->cone_width ? m_caster->isInArc(TO_UNIT(*itr), m_spellInfo->cone_width) : m_caster->isTargetInFront(TO_UNIT(*itr)))  // !!! is the target within our cone ?
            {
                AddTarget(i, TargetType, (*itr));
            }
        }
        if(maxtargets != 0 && ManagedTargets.size() >= maxtargets)
            return;
    }
}

void Spell::AddScriptedOrSpellFocusTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    for(ObjectSet::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); ++itr)
    {
        Object* o = *itr;

        if(!o->IsGameObject())
            continue;

        GameObject* go = TO_GAMEOBJECT(o);

        if(go->GetInfo()->TypeSpellFocus.FocusId == m_spellInfo->RequiresSpellFocus)
        {
            if(!m_caster->isInRange(go, r))
                continue;

            bool success = AddTarget(i, TargetType, go);
            if(success)
                return;
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
    Group *group = Target->GetGroup();
    if(group)
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

