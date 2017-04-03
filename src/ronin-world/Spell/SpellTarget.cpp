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

SpellTargetClass::SpellTargetClass(Unit* caster, SpellEntry *info, uint8 castNumber, WoWGuid itemGuid) : SpellEffectClass(caster, info, castNumber, itemGuid)
{

}

SpellTargetClass::~SpellTargetClass()
{

}

void SpellTargetClass::Destruct()
{

    SpellEffectClass::Destruct();
}

uint32 SpellTargetClass::GetTargetType(uint32 implicittarget, uint32 i)
{
    uint32 type = m_implicitTargetFlags[implicittarget];

    //CHAIN SPELLS ALWAYS CHAIN!
    uint32 jumps = m_spellInfo->EffectChainTarget[i];
    _unitCaster->SM_FIValue(SMT_JUMP_REDUCE, (int32*)&jumps, m_spellInfo->SpellGroupType);
    if(jumps != 0)
        type |= SPELL_TARGET_AREA_CHAIN;
    return type;
}

uint8 SpellTargetClass::_DidHit(Unit* target, float *resistOut, uint8 *reflectout)
{
    //note resistchance is vise versa, is full hit chance
    if( target == NULL )
        return SPELL_DID_HIT_MISS;

    /************************************************************************/
    /* Can't can't miss your own spells                                     */
    /************************************************************************/
    if(_unitCaster == target)
        return SPELL_DID_HIT_SUCCESS;

    /************************************************************************/
    /* Check if the unit is evading                                      */
    /************************************************************************/
    if(target->IsCreature() && castPtr<Creature>(target)->hasStateFlag(UF_EVADING))
        return SPELL_DID_HIT_EVADE;

    if(uint32 mechanic = m_spellInfo->MechanicsType)
    {
        /*************************************************************************/
        /* Check if the target is immune to this mechanic                       */
        /*************************************************************************/
        if(target->GetMechanicDispels(mechanic))
            return SPELL_DID_HIT_IMMUNE; // Moved here from Spell::CanCast

        // Creature Aura Immune Flag Check
        if (Creature* cTarget = target->IsCreature() ? castPtr<Creature>(target) : NULL)
            if(cTarget->GetCreatureData()->auraMechanicImmunity && (cTarget->GetCreatureData()->auraMechanicImmunity & (uint32(1)<<(mechanic-1))))
                return SPELL_DID_HIT_IMMUNE;

        /************************************************************************/
        /* Check if the target has a % resistance to this mechanic            */
        /************************************************************************/
        if( mechanic < MECHANIC_COUNT)
        {
            float res = target->GetMechanicResistPCT(Spell::GetMechanic(m_spellInfo));
            if( !m_spellInfo->isSpellResistanceIgnorant() && Rand(res))
                return SPELL_DID_HIT_RESIST;
        }
    }

    /************************************************************************/
    /* Check if the spell is a melee attack and if it was missed/parried    */
    /************************************************************************/
    uint32 meleeResult = 0;
    if( m_spellInfo->IsSpellWeaponSpell() && (meleeResult = _unitCaster->GetSpellDidHitResult(target, m_spellInfo->spellType, m_spellInfo)) )
        return meleeResult;
    return _unitCaster->GetSpellDidHitResult(target, this, resistOut, reflectout);
}

AuraApplicationResult SpellTargetClass::CheckAuraApplication(Unit *target)
{
    // If it's passive and we already have it, reject, should happen earlier than this though...
    if(m_spellInfo->isPassiveSpell() && target->HasAura(m_spellInfo->Id))
        return AURA_APPL_REJECTED;

    // Check if we have the aura to be updated
    if(Aura *aur = target->m_AuraInterface.FindActiveAuraWithNameHash(m_spellInfo->NameHash))
    {
        SpellEntry *targetEntry = aur->GetSpellProto();
        // Check to see if we can just stack up our aura
        if(targetEntry->maxstack > 1)
            return AURA_APPL_STACKED;
        // Just refresh the aura, part of cata changes is it doesn't matter what rank
        if(targetEntry->procCharges || (m_spellInfo->RankNumber && targetEntry->RankNumber >= m_spellInfo->RankNumber))
            return AURA_APPL_REFRESH;
    }

    // No aura or complications found, pass through
    return AURA_APPL_SUCCESS;
}

bool isAreaAuraApplicator(SpellEntry *sp, uint32 effectMask)
{
    if(sp->HasEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA, effectMask))
        return true;
    if(sp->HasEffect(SPELL_EFFECT_APPLY_AREA_AURA, effectMask))
        return true;
    if(sp->HasEffect(SPELL_EFFECT_APPLY_AREA_AURA_FRIEND, effectMask))
        return true;
    if(sp->HasEffect(SPELL_EFFECT_APPLY_AREA_AURA_ENEMY, effectMask))
        return true;
    return false;
}

void SpellTargetClass::_AddTarget(WorldObject* target, const uint32 effIndex)
{
    // Check if we're in the current list already, and if so, don't readd us.
    if(m_effectTargetMaps[effIndex].find(target->GetGUID()) != m_effectTargetMaps[effIndex].end())
        return;

    SpellTarget *tgt = NULL;
    // look for the target in the list already
    SpellTargetStorage::iterator itr = m_fullTargetMap.find(target->GetGUID());
    if(itr != m_fullTargetMap.end())
        tgt = itr->second;
    else
    {
        tgt = new SpellTarget(target->GetGUID());
        tgt->HitResult = target->IsUnit() ? _DidHit(castPtr<Unit>(target), &tgt->resistMod, &tgt->ReflectResult) : SPELL_DID_HIT_SUCCESS;

        if( tgt->HitResult != SPELL_DID_HIT_SUCCESS )
            m_spellMisses.push_back(std::make_pair(tgt->Guid, tgt->HitResult));

        // Add us to the full target map
        m_fullTargetMap.insert(std::make_pair(target->GetGUID(), tgt));
        // If we're a delayed spell push us into our delayed vector
        if(m_missileSpeed > 0.f) m_delayTargets.insert(tgt->Guid);
    }

    if(tgt->HitResult != SPELL_DID_HIT_SUCCESS)
        return;

    // Effect mask used for storage
    uint32 effectMask = (1<<effIndex);
    // Add effect mask
    tgt->EffectMask |= effectMask;
    // Calculate effect amount
    tgt->effectAmount[effIndex] = CalculateEffect(effIndex, target);
    // Call to spell manager to modify the spell amount
    tgt->moddedAmount[effIndex] = sSpellMgr.ModifyEffectAmount(this, effIndex, _unitCaster, target, tgt->effectAmount[effIndex]);
    // Build any modifier data here, area auras are handled differently so make sure we don't handle these unless the effect is a different handler
    if(m_spellInfo->isSpellAuraApplicator() && target->IsUnit() && !isAreaAuraApplicator(m_spellInfo, effectMask))
    {
        Unit *unitTarget = castPtr<Unit>(target);
        if(tgt->resistMod)
        {
            tgt->AuraAddResult = AURA_APPL_RESISTED;
            ASSERT(tgt->aura == NULL);
        }
        else
        {
            if(tgt->aura == NULL && tgt->AuraAddResult == AURA_APPL_NOT_RUN)
            {
                if((tgt->AuraAddResult = CheckAuraApplication(unitTarget)) == AURA_APPL_SUCCESS)
                {
                    uint16 auraFlags = m_spellInfo->isPassiveSpell() ? 0x0000 : (AFLAG_EFF_AMOUNT_SEND | (m_spellInfo->isNegativeSpell1() ? AFLAG_NEGATIVE : AFLAG_POSITIVE));
                    int16 stackSize = 1;
                    if(m_spellInfo->procCharges && m_spellInfo->SpellGroupType)
                    {
                        stackSize = (m_spellInfo->procCharges&0xFF);
                        _unitCaster->SM_FIValue(SMT_CHARGES, (int32*)&stackSize, m_spellInfo->SpellGroupType);
                        _unitCaster->SM_PIValue(SMT_CHARGES, (int32*)&stackSize, m_spellInfo->SpellGroupType);
                        stackSize *= -1;
                    }
                    tgt->aura = new Aura(unitTarget, m_spellInfo, auraFlags, _unitCaster->getLevel(), stackSize, UNIXTIME, _unitCaster->GetGUID());
                }
            }

            if(tgt->AuraAddResult == AURA_APPL_SUCCESS)
                tgt->aura->AddMod(effIndex, m_spellInfo->EffectApplyAuraName[effIndex], tgt->effectAmount[effIndex]);
        }
    }

    // add to the effect target map
    m_effectTargetMaps[effIndex].insert(std::make_pair(target->GetGUID(), tgt));
}

void SpellTargetClass::FillTargetMap(bool fromDelayed)
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
            if(WorldObject* target = _unitCaster->GetInRangeObject(m_targets.m_unitTarget))
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

        bool inWorld = _unitCaster->IsInWorld(); //always add this guy :P
        if(inWorld && !(targetTypes[i] & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE | SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_OBJECT_PETOWNER)))
            if(Unit* target = _unitCaster->GetInRangeObject<Unit>(m_targets.m_unitTarget))
                AddTarget(i, targetTypes[i], target);

        // We can always push self to our target map
        if(targetTypes[i] & SPELL_TARGET_OBJECT_SELF)
            AddTarget(i, targetTypes[i], _unitCaster);

        if (inWorld && (targetTypes[i] & SPELL_TARGET_OBJECT_CURTOTEMS))
        {
            std::vector<Creature*> m_totemList;
            _unitCaster->FillSummonList(m_totemList, SUMMON_TYPE_TOTEM);
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
                if(!_unitCaster->IsPlayer() && !(_unitCaster->IsCreature() || _unitCaster->IsTotem()))
                    AddAOETargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets); //npcs
                else AddPartyTargets(i, targetTypes[i], radius, m_spellInfo->MaxTargets); //players/pets/totems
            }

            if(targetTypes[i] & SPELL_TARGET_AREA_RAID)
            {
                if(!_unitCaster->IsPlayer() && !(_unitCaster->IsCreature() || _unitCaster->IsTotem()))
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
        if(!m_targets.hasDestination() && (m_effectTargetMaps[i].empty() && (m_targets.m_unitTarget.empty() || m_targets.m_unitTarget == _unitCaster->GetGUID())))
            AddTarget(i, SPELL_TARGET_NONE, _unitCaster);
    }
}

void SpellTargetClass::HandleTargetNoObject()
{
    float dist = 3;
    float srcX = _unitCaster->GetPositionX(), newx = srcX + cosf(_unitCaster->GetOrientation()) * dist;
    float srcY = _unitCaster->GetPositionY(), newy = srcY + sinf(_unitCaster->GetOrientation()) * dist;
    float srcZ = _unitCaster->GetPositionZ(), newz = _unitCaster->GetMapInstance()->GetWalkableHeight(_unitCaster, newx, newy, srcZ);

    //if not in line of sight, or too far away we summon inside caster
    if(fabs(newz - srcZ) > 10 || !sVMapInterface.CheckLOS(_unitCaster->GetMapId(), _unitCaster->GetInstanceID(), _unitCaster->GetPhaseMask(), srcX, srcY, srcZ, newx, newy, newz + 2))
        newx = srcX, newy = srcY, newz = srcZ;

    m_targets.m_targetMask |= TARGET_FLAG_DEST_LOCATION;
    m_targets.m_dest.ChangeCoords(newx, newy, newz);
}

bool SpellTargetClass::AddTarget(uint32 i, uint32 TargetType, WorldObject* obj)
{
    if(obj == NULL || (obj != _unitCaster && !obj->IsInWorld()))
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

    if(_unitCaster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IGNORE_PC) && (obj->IsPlayer() || _unitCaster->IsPlayer()))
        return false;

    if(TargetType & SPELL_TARGET_REQUIRE_FRIENDLY && !sFactionSystem.isFriendly(_unitCaster, obj))
        return false;
    if(TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE && !sFactionSystem.isAttackable(_unitCaster, obj))
        return false;
    if(TargetType & SPELL_TARGET_OBJECT_TARCLASS)
    {
        WorldObject* originaltarget = _unitCaster->GetInRangeObject(m_targets.m_unitTarget);
        if(originaltarget == NULL || (originaltarget->IsPlayer() && obj->IsPlayer() && castPtr<Player>(originaltarget)->getClass() != castPtr<Player>(obj)->getClass())
            || (originaltarget->IsPlayer() && !obj->IsPlayer()) || (!originaltarget->IsPlayer() && obj->IsPlayer()))
            return false;
    }

    if(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID)
        && ((obj->IsUnit() && !castPtr<Unit>(obj)->isAlive()) || (obj->IsCreature() && obj->IsTotem())))
        return false;

    _AddTarget(obj, i);

    //final checks, require line of sight unless range/radius is 50000 yards
    SpellRangeEntry* r = dbcSpellRange.LookupEntry(m_spellInfo->rangeIndex);
    if(sWorld.Collision && r->maxRangeHostile < 50000 && GetRadius(i) < 50000 && !obj->IsItem())
    {
        float x = _unitCaster->GetPositionX(), y = _unitCaster->GetPositionY(), z = _unitCaster->GetPositionZ() + 0.5f;

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

        if(!sVMapInterface.CheckLOS(_unitCaster->GetMapId(), _unitCaster->GetInstanceID(), _unitCaster->GetPhaseMask(), x, y, z + 2, obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ() + 2))
            return false;
    }
    return true;
}

bool SpellTargetClass::GenerateTargets(SpellCastTargets *t)
{
    if(!_unitCaster->IsInWorld())
        return false;

    bool result = false;

    for(uint32 i = 0; i < 3; ++i)
    {
        if(m_spellInfo->Effect[i] == 0)
            continue;
        uint32 TargetType = GetTargetType(m_spellInfo->EffectImplicitTargetA[i], i);

        //never get info from B if it is 0 :P
        if(m_spellInfo->EffectImplicitTargetB[i] != 0)
            TargetType |= GetTargetType(m_spellInfo->EffectImplicitTargetB[i], i);

        if(TargetType & (SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID))
        {
            t->m_targetMask |= TARGET_FLAG_UNIT;
            t->m_unitTarget = _unitCaster->GetGUID();
            result = true;
        }

        if(TargetType & SPELL_TARGET_NO_OBJECT)
        {
            t->m_targetMask = TARGET_FLAG_SELF;
            t->m_unitTarget = _unitCaster->GetGUID();
            result = true;
        }

        if(!(TargetType & (SPELL_TARGET_AREA | SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_AREA_CONE)))
        {
            if(TargetType & SPELL_TARGET_ANY_OBJECT)
            {
                if(_unitCaster->GetUInt64Value(UNIT_FIELD_TARGET))
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    if(WorldObject* target = _unitCaster->GetInRangeObject<WorldObject>(_unitCaster->GetUInt64Value(UNIT_FIELD_TARGET)))
                    {
                        if(target->IsUnit())
                        {
                            t->m_targetMask |= TARGET_FLAG_UNIT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                        else if(target->IsGameObject())
                        {
                            t->m_targetMask |= TARGET_FLAG_OBJECT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                    }
                    result = true;
                }
            }

            if(TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE)
            {
                if(_unitCaster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    if(WorldObject* target = _unitCaster->GetInRangeObject<WorldObject>(_unitCaster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)))
                    {
                        if(target->IsUnit())
                        {
                            t->m_targetMask |= TARGET_FLAG_UNIT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                        else if(target->IsGameObject())
                        {
                            t->m_targetMask |= TARGET_FLAG_OBJECT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                    }
                }
                else if(_unitCaster->GetUInt64Value(UNIT_FIELD_TARGET))
                {
                    //generate targets for things like arcane missiles trigger, tame pet, etc
                    if(WorldObject* target = _unitCaster->GetInRangeObject<WorldObject>(_unitCaster->GetUInt64Value(UNIT_FIELD_TARGET)))
                    {
                        if(target->IsUnit())
                        {
                            t->m_targetMask |= TARGET_FLAG_UNIT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                        else if(target->IsGameObject())
                        {
                            t->m_targetMask |= TARGET_FLAG_OBJECT;
                            t->m_unitTarget = target->GetGUID();
                            result = true;
                        }
                    }
                    result = true;
                }
                else if(_unitCaster->IsCreature() && _unitCaster->IsTotem())
                {
                    if(Unit* target = _unitCaster->GetInRangeObject<Unit>(GetSinglePossibleEnemy(i)))
                    {
                        t->m_targetMask |= TARGET_FLAG_UNIT;
                        t->m_unitTarget = target->GetGUID();
                    }
                }
            }

            if(TargetType & SPELL_TARGET_REQUIRE_FRIENDLY)
            {
                result = true;
                t->m_targetMask |= TARGET_FLAG_UNIT;
                if(Unit* target = _unitCaster->GetInRangeObject<Unit>(GetSinglePossibleFriend(i)))
                    t->m_unitTarget = target->GetGUID();
                else t->m_unitTarget = _unitCaster->GetGUID();
            }
        }

        if(TargetType & SPELL_TARGET_AREA_RANDOM)
        {
            //we always use radius(0) for some reason
            uint8 attempts = 0;
            do
            {
                //prevent deadlock
                ++attempts;
                if(attempts > 10)
                    return false;

                float r = RandomFloat(GetRadius(0));
                float ang = RandomFloat(M_PI * 2);
                t->m_dest.x = _unitCaster->GetPositionX() + (cosf(ang) * r);
                t->m_dest.y = _unitCaster->GetPositionY() + (sinf(ang) * r);
                t->m_dest.z = _unitCaster->GetMapInstance()->GetWalkableHeight(_unitCaster, t->m_dest.x, t->m_dest.y, _unitCaster->GetPositionZ() + 2.0f);
                t->m_targetMask = TARGET_FLAG_DEST_LOCATION;
            }
            while(sWorld.Collision && !sVMapInterface.CheckLOS(_unitCaster->GetMapId(), _unitCaster->GetInstanceID(), _unitCaster->GetPhaseMask(), _unitCaster->GetPositionX(), _unitCaster->GetPositionY(), _unitCaster->GetPositionZ(), t->m_dest.x, t->m_dest.y, t->m_dest.z));
            result = true;
        }
        else if(TargetType & SPELL_TARGET_AREA)  //targetted aoe
        {
            if(TargetType & SPELL_TARGET_REQUIRE_FRIENDLY)
            {
                t->m_targetMask |= TARGET_FLAG_DEST_LOCATION;
                t->m_dest = _unitCaster->GetPosition();
                result = true;
            }
            else if(_unitCaster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)) //spells like blizzard, rain of fire
            {
                if(WorldObject* target = _unitCaster->GetInRangeObject<WorldObject>(_unitCaster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)))
                {
                    t->m_targetMask |= TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_UNIT;
                    t->m_unitTarget = target->GetGUID();
                    t->m_dest = target->GetPosition();
                }
                result = true;
            }
        }
        else if(TargetType & SPELL_TARGET_AREA_SELF)
        {
            t->m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
            t->m_dest = t->m_src = _unitCaster->GetPosition();
            result = true;
        }

        if(TargetType & SPELL_TARGET_AREA_CHAIN && !(TargetType & SPELL_TARGET_REQUIRE_ATTACKABLE))
        {
            t->m_targetMask |= TARGET_FLAG_UNIT;
            t->m_unitTarget = _unitCaster->GetGUID();
            result = true;
        }
    }
    return result;
}

void SpellTargetClass::FillSpecifiedTargetsInArea( float srcx, float srcy, float srcz, uint32 ind, uint32 specification )
{
    FillSpecifiedTargetsInArea( ind, srcx, srcy, srcz, GetRadius(ind), specification );
}

void FillSpecificTargetsCallback::operator()(SpellTargetClass *spell, uint32 i, WorldObject *target)
{
    Unit *unitCaster = spell->GetCaster();
    if(target->IsUnit())
    {
        if(!castPtr<Unit>(target)->isAlive())
            return;
        if(spell->GetSpellProto()->TargetCreatureType)
        {
            Unit* Target = castPtr<Unit>(target);
            if(uint32 creatureType = Target->GetCreatureType())
            {
                if(((1<<(creatureType-1)) & spell->GetSpellProto()->TargetCreatureType) == 0)
                    return;
            } else return;
        }
    } else if(target->IsGameObject() && !Spell::CanEffectTargetGameObjects(spell->GetSpellProto(), i))
        return;

    if( !target->IsUnit() || sFactionSystem.CanEitherUnitAttack(unitCaster, castPtr<Unit>(target), !spell->GetSpellProto()->isSpellStealthTargetCapable()) )
        spell->_AddTarget(target, i);
}

// for the moment we do invisible targets
void SpellTargetClass::FillSpecifiedTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float range, uint32 specification)
{
    if(!_unitCaster->IsInWorld())
        return;

    static FillSpecificTargetsCallback _fillCallback;
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(&_fillCallback, this, i, srcx, srcy, srcz, range*range);
}

void SpellTargetClass::FillAllTargetsInArea(LocationVector & location,uint32 ind)
{
    FillAllTargetsInArea(ind,location.x,location.y,location.z,GetRadius(ind));
}

void SpellTargetClass::FillAllTargetsInArea(float srcx,float srcy,float srcz,uint32 ind)
{
    FillAllTargetsInArea(ind,srcx,srcy,srcz,GetRadius(ind));
}

/// We fill all the targets in the area, including the stealth ed one's
void SpellTargetClass::FillAllTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float range, bool includegameobjects)
{
    float r = range*range;
    uint32 placeholder = 0;
    WorldObject *wObj = NULL;
    std::vector<WorldObject*> ChainTargetContainer;
    //bool canAffectGameObjects = CanEffectTargetGameObjects(i);
    /*for(WorldObject::InRangeHashMap::iterator itr = m_caster->GetInRangeMapBegin(); itr != m_caster->GetInRangeMapEnd(); itr++ )
    {
        if((wObj = itr->second) == NULL)
            continue;

        if(wObj->IsUnit())
        {
            Unit *uTarget = castPtr<Unit>(wObj);
            if(!uTarget->isAlive())
                continue;
            if( m_spellInfo->TargetCreatureType && !(m_spellInfo->TargetCreatureType & (1<<(uTarget->GetCreatureType()-1))))
                continue;
        } else if(wObj->IsGameObject() && !canAffectGameObjects)
            continue;

        if(!IsInrange(srcx, srcy, srcz, wObj, r))
            continue;

        if(castPtr<Unit>(m_caster) && wObj->IsUnit() )
        {
            if( sFactionSystem.CanEitherUnitAttack(castPtr<Unit>(m_caster), castPtr<Unit>(wObj), !m_spellInfo->isSpellStealthTargetCapable()) )
            {
                ChainTargetContainer.push_back(wObj);
                placeholder++;
            }
        } else ChainTargetContainer.push_back(wObj);
    }*/

    if(m_spellInfo->MaxTargets)
    {
        WorldObject* chaintarget = NULL;
        uint32 targetCount = m_spellInfo->MaxTargets;
        while(targetCount && ChainTargetContainer.size())
        {
            placeholder = (rand()%ChainTargetContainer.size());
            chaintarget = ChainTargetContainer.at(placeholder);
            if(chaintarget == NULL)
                continue;

            if(chaintarget->IsUnit())
                _AddTarget(castPtr<Unit>(chaintarget), i);
            else _AddTarget(chaintarget, i);
            ChainTargetContainer.erase(ChainTargetContainer.begin()+placeholder);
            targetCount--;
        }
    }
    else
    {
        for(std::vector<WorldObject*>::iterator itr = ChainTargetContainer.begin(); itr != ChainTargetContainer.end(); itr++)
        {
            if((*itr)->IsUnit())
                _AddTarget(castPtr<Unit>(*itr), i);
            else _AddTarget(*itr, i);
        }
    }
    ChainTargetContainer.clear();
}

// We fill all the targets in the area, including the stealthed one's
void SpellTargetClass::FillAllFriendlyInArea( uint32 i, float srcx, float srcy, float srcz, float range )
{
    float r = range*range;
    WorldObject *wObj = NULL;
    //bool canAffectGameObjects = CanEffectTargetGameObjects(i);
    /*for(WorldObject::InRangeHashMap::iterator itr = m_caster->GetInRangeMapBegin(); itr != m_caster->GetInRangeMapEnd(); itr++ )
    {
        if((wObj = itr->second) == NULL)
            continue;

        if(wObj->IsUnit())
        {
            Unit *uTarget = castPtr<Unit>(wObj);
            if( !uTarget->isAlive() || (uTarget->IsCreature() && castPtr<Creature>(uTarget)->IsTotem()))
                continue;
            if( m_spellInfo->TargetCreatureType && !(m_spellInfo->TargetCreatureType & (1<<(uTarget->GetCreatureType()-1))))
                continue;
        } else if(wObj->IsGameObject() && !canAffectGameObjects)
            continue;

        if( IsInrange( srcx, srcy, srcz, wObj, r ))
        {
            if(castPtr<Unit>(m_caster) && wObj->IsUnit() )
            {
                if( sFactionSystem.CanEitherUnitAttack(castPtr<Unit>(m_caster), castPtr<Unit>(wObj), !m_spellInfo->isSpellStealthTargetCapable()) )
                    _AddTarget(castPtr<Unit>(wObj), i);
            }
            else _AddTarget(wObj, i);

            if( m_spellInfo->MaxTargets && m_effectTargetMaps[i].size() >= m_spellInfo->MaxTargets )
                break;
        }
    }*/
}

/// We fill all the gameobject targets in the area
void SpellTargetClass::FillAllGameObjectTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float range)
{
    float r = range*range;

    /*for(WorldObject::InRangeArray::iterator itr = m_caster->GetInRangeGameObjectSetBegin(); itr != m_caster->GetInRangeGameObjectSetEnd(); itr++ )
    {
        if(GameObject *gObj = m_caster->GetInRangeObject<GameObject>(*itr))
        {
            if(!IsInrange( srcx, srcy, srcz, gObj, r ))
                continue;
            _AddTarget(gObj, i);
        }
    }*/
}

uint64 SpellTargetClass::GetSinglePossibleEnemy(uint32 i,float prange)
{
    float rMin = m_spellInfo->minRange[0], rMax = prange;
    if(rMax == 0.f)
    {
        rMax = m_spellInfo->maxRange[0];
        if( m_spellInfo->SpellGroupType)
        {
            _unitCaster->SM_FFValue(SMT_RADIUS,&rMax,m_spellInfo->SpellGroupType);
            _unitCaster->SM_PFValue(SMT_RADIUS,&rMax,m_spellInfo->SpellGroupType);
        }
    }

    float srcx = _unitCaster->GetPositionX(), srcy = _unitCaster->GetPositionY(), srcz = _unitCaster->GetPositionZ();
    /*for( WorldObject::InRangeArray::iterator itr = m_caster->GetInRangeUnitSetBegin(); itr != m_caster->GetInRangeUnitSetEnd(); itr++ )
    {
        Unit *unit = m_caster->GetInRangeObject<Unit>(*itr);
        if(!unit->isAlive())
            continue;
        if( m_spellInfo->TargetCreatureType && (!(1<<(unit->GetCreatureType()-1) & m_spellInfo->TargetCreatureType)))
            continue;
        if(!IsInrange(srcx,srcy,srcz, unit, rMax, rMin))
            continue;
        if(!sFactionSystem.isAttackable(m_caster, unit,!m_spellInfo->isSpellStealthTargetCapable()))
            continue;
        if(_DidHit(unit) != SPELL_DID_HIT_SUCCESS)
            continue;
        return unit->GetGUID();
    }*/
    return 0;
}

uint64 SpellTargetClass::GetSinglePossibleFriend(uint32 i,float prange)
{
    float rMin = m_spellInfo->minRange[1], rMax = prange;
    if(rMax == 0.f)
    {
        rMax = m_spellInfo->maxRange[1];
        if( m_spellInfo->SpellGroupType)
        {
            _unitCaster->SM_FFValue(SMT_RADIUS,&rMax,m_spellInfo->SpellGroupType);
            _unitCaster->SM_PFValue(SMT_RADIUS,&rMax,m_spellInfo->SpellGroupType);
        }
    }

    float srcx = _unitCaster->GetPositionX(), srcy = _unitCaster->GetPositionY(), srcz = _unitCaster->GetPositionZ();
    /*for(WorldObject::InRangeArray::iterator itr = m_caster->GetInRangeUnitSetBegin(); itr != m_caster->GetInRangeUnitSetEnd(); itr++ )
    {
        Unit *unit = m_caster->GetInRangeObject<Unit>(*itr);
        if(!unit->isAlive())
            continue;
        if( m_spellInfo->TargetCreatureType && (!(1<<(unit->GetCreatureType()-1) & m_spellInfo->TargetCreatureType)))
            continue;
        if(!IsInrange(srcx,srcy,srcz, unit, rMax, rMin))
            continue;
        if(!sFactionSystem.isFriendly(m_caster, unit))
            continue;
        if(_DidHit(unit) != SPELL_DID_HIT_SUCCESS)
            continue;
        return unit->GetGUID();
    }*/
    return 0;
}

void SpellTargetClass::AddAOETargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    LocationVector source;

    //cant do raid/party stuff here, seperate functions for it
    if(TargetType & (SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID))
        return;

    WorldObject* tarobj = _unitCaster->GetInRangeObject(m_targets.m_unitTarget);

    if(TargetType & SPELL_TARGET_AREA_SELF)
        source = _unitCaster->GetPosition();
    else if(TargetType & SPELL_TARGET_AREA_CURTARGET && tarobj != NULL)
        source = tarobj->GetPosition();
    else
    {
        m_targets.m_targetMask |= TARGET_FLAG_DEST_LOCATION;
        source = m_targets.m_dest;
    }

    float range = r*r; //caster might be in the aoe LOL
    if(_unitCaster->GetDistanceSq(source) <= range)
        AddTarget(i, TargetType, _unitCaster);

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

void SpellTargetClass::AddPartyTargets(uint32 i, uint32 TargetType, float radius, uint32 maxtargets)
{
    WorldObject* u = _unitCaster->GetInRangeObject(m_targets.m_unitTarget);
    if(u == NULL && (u = _unitCaster) == NULL)
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

void SpellTargetClass::AddRaidTargets(uint32 i, uint32 TargetType, float radius, uint32 maxtargets, bool partylimit)
{
    WorldObject* u = _unitCaster->GetInRangeObject(m_targets.m_unitTarget);
    if(u == NULL && (u = _unitCaster) == NULL)
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

void SpellTargetClass::AddChainTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    if(!_unitCaster->IsInWorld())
        return;

    WorldObject* targ = _unitCaster->GetInRangeObject(m_targets.m_unitTarget);
    if(targ == NULL)
        return;

    //if selected target is party member, then jumps on party
    Unit* firstTarget = NULL;
    if(targ->IsUnit())
        firstTarget = castPtr<Unit>(targ);
    else firstTarget = _unitCaster;

    bool RaidOnly = false;
    float range = m_spellInfo->maxRange[0];
    //this is cast distance, not searching distance
    range *= range;

    //is this party only?
    Player* casterFrom = NULL;
    if(_unitCaster->IsPlayer())
        casterFrom = castPtr<Player>(_unitCaster);

    Player* pfirstTargetFrom = NULL;
    if(firstTarget->IsPlayer())
        pfirstTargetFrom = castPtr<Player>(firstTarget);

    if(casterFrom != NULL && pfirstTargetFrom != NULL && casterFrom->GetGroup() == pfirstTargetFrom->GetGroup())
        RaidOnly = true;

    uint32 jumps = m_spellInfo->EffectChainTarget[i];

    //range
    range /= jumps; //hacky, needs better implementation!

    if(m_spellInfo->SpellGroupType)
        _unitCaster->SM_FIValue(SMT_JUMP_REDUCE, (int32*)&jumps, m_spellInfo->SpellGroupType);

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

void SpellTargetClass::AddConeTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
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

void SpellTargetClass::AddScriptedOrSpellFocusTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
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
uint64 SpellTargetClass::FindLowestHealthRaidMember(Player* Target, uint32 dist)
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

