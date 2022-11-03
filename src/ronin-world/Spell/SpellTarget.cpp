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

    m_temporaryStorage = NULL;
}

SpellTargetClass::~SpellTargetClass()
{

}

void SpellTargetClass::Destruct()
{

    SpellEffectClass::Destruct();
}

bool SpellTargetClass::isSpellAOEStrikeable(SpellEntry *sp, uint8 effIndex)
{
    if(sp->isSpellBackAttackCapable())
        return true;
    // Non weapons can't strike
    if(sp->spellType == NON_WEAPON)
        return false;
    if(sp->EffectImplicitTargetA[effIndex] && (m_implicitTargetFlags[sp->EffectImplicitTargetA[effIndex]] & SPELL_TARGET_AREA_MASK))
        return true;
    if(sp->EffectImplicitTargetB[effIndex] && (m_implicitTargetFlags[sp->EffectImplicitTargetB[effIndex]] & SPELL_TARGET_AREA_MASK))
        return true;
    return false;
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
    /* Check if the spell is a triggered event spell                        */
    /************************************************************************/
    if(m_triggeredSpell && !m_spellInfo->isSpellDamagingEffect())
        return SPELL_DID_HIT_SUCCESS;

    /************************************************************************/
    /* Check if the spell is a melee attack and if it was missed/parried    */
    /************************************************************************/
    uint32 meleeResult = 0;
    if( m_spellInfo->IsSpellWeaponSpell() && (meleeResult = _unitCaster->GetSpellDidHitResult(target, m_spellInfo->spellType, m_spellInfo)) )
        return meleeResult;
    return _unitCaster->GetSpellDidHitResult(target, this, resistOut, reflectout);
}

bool SpellTargetClass::requiresCombatSupport(uint32 effIndex)
{
    uint32 targetType = GetTargetType(m_spellInfo->EffectImplicitTargetA[effIndex], effIndex);
    if(m_spellInfo->EffectImplicitTargetB[effIndex] != 0)
        targetType |= GetTargetType(m_spellInfo->EffectImplicitTargetB[effIndex], effIndex);
    // Check target type for friendly requirement, if so just use isFriendly for consistency
    if(targetType & SPELL_TARGET_REQUIRE_FRIENDLY)
        return false;
    return true;
}

bool SpellTargetClass::EffectRequiresAnyTarget(uint32 effIndex)
{
    if(IsTriggerSpellEffect(effIndex))
        return true;
    if(m_spellInfo->HasEffect(SPELL_EFFECT_ADD_FARSIGHT, (1<<effIndex)))
        return true;
    if(m_spellInfo->HasEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA, (1<<effIndex)))
        return true;
    return false;
}

bool SpellTargetClass::IsTargetMapFull(uint32 effIndex, WoWGuid guidCheck)
{
    if(uint32 targetCap = m_spellInfo->MaxTargets)
        if(m_fullTargetMap.size() >= targetCap && (guidCheck.empty() || m_fullTargetMap.find(guidCheck) == m_fullTargetMap.end()))
            return true;

    // Jumps are from main target, so effective -1 is to use > instead of >=
    if(uint32 maxJumps = m_spellInfo->EffectChainTarget[effIndex])
        if(m_effectTargetMaps[effIndex].size() > maxJumps)
            return true;
    return false;
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
    // Build any modifier data here, persistent area auras are handled differently so make sure we don't handle these unless the effect is a different handler
    if(m_spellInfo->isSpellAuraApplicator(effIndex) && target->IsUnit() && !m_spellInfo->HasEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA, m_spellInfo->GetAuraApplicationMask()))
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
                    tgt->aura = new Aura(unitTarget, m_spellInfo, m_spellParent, auraFlags, GetEffectiveCasterLevel(), stackSize, UNIXTIME, _unitCaster->GetGUID());
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
    bool selfTarget = m_targets.m_unitTarget == _unitCaster->GetGUID();
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
        if(targetTypes[i] & SPELL_TARGET_NOT_IMPLEMENTED)
            continue;

        float radius = GetRadius(i);
        if(targetTypes[i] & SPELL_TARGET_NO_OBJECT)  //summon spells that appear infront of caster
        {
            HandleTargetNoObject(i, radius);
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
        if(m_effectTargetMaps[i].empty() && (m_targets.m_unitTarget.empty() || selfTarget))
            AddTarget(i, ((selfTarget && m_triggeredSpell) || EffectRequiresAnyTarget(i)) ? SPELL_TARGET_NONE : targetTypes[i], _unitCaster);
    }
}

void SpellTargetClass::HandleTargetNoObject(uint32 i, float r)
{
    _AddTarget(_unitCaster, i);
    if(m_targets.hasDestination())
        return;

    m_targets.m_targetMask |= TARGET_FLAG_DEST_LOCATION;
    float srcX = _unitCaster->GetPositionX(), newx = srcX;
    float srcY = _unitCaster->GetPositionY(), newy = srcY;
    float srcZ = _unitCaster->GetPositionZ(), newz = srcZ;

    float dist = 0.f;
    if((dist = r) > 0.f)
    {
        newx += cosf(_unitCaster->GetOrientation()) * dist;
        newy += sinf(_unitCaster->GetOrientation()) * dist;
        newz = _unitCaster->GetMapInstance()->GetWalkableHeight(_unitCaster, newx, newy, srcZ, srcZ);
        //if not in line of sight, or too far away we summon inside caster
        if(fabs(newz - srcZ) > 10 || !sVMapInterface.CheckLOS(_unitCaster->GetMapId(), _unitCaster->GetInstanceID(), _unitCaster->GetPhaseMask(), srcX, srcY, srcZ, newx, newy, newz + 2))
            newx = srcX, newy = srcY, newz = srcZ;
    }
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

    if(_unitCaster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IGNORE_PC) && (obj->IsPlayer() || _unitCaster->IsPlayer()) && !m_triggeredSpell)
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

    //final checks, require line of sight unless range/radius is 50000 yards
    if(sWorld.Collision && m_spellInfo->maxRange[0] < 50000 && GetRadius(i) < 50000 && !obj->IsItem())
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

    _AddTarget(obj, i);
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
                t->m_dest.z = _unitCaster->GetMapInstance()->GetWalkableHeight(_unitCaster, t->m_dest.x, t->m_dest.y, _unitCaster->GetPositionZ() + 2.0f, _unitCaster->GetPositionZ());
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

void SpellTargetClass::FillSpecifiedTargetsInArea( float srcx, float srcy, float srcz, uint32 effIndex, uint32 typeMask )
{
    FillSpecifiedTargetsInArea( effIndex, srcx, srcy, srcz, GetRadius(effIndex), typeMask );
}

void FillAreaTargetsCallback::operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target)
{
    if(spell->IsTargetMapFull(i, target->GetGUID()))
        return;

    Unit *unitCaster = spell->GetCaster();
    if(target->IsUnit())
    {
        if(spell->GetSpellProto()->TargetCreatureType)
        {
            Unit* Target = castPtr<Unit>(target);
            if(uint32 creatureType = Target->GetCreatureType())
            {
                if(((1<<(creatureType-1)) & spell->GetSpellProto()->TargetCreatureType) == 0)
                    return;
            } else return;
        }

        if( !sFactionSystem.CanEitherUnitAttack(unitCaster, castPtr<Unit>(target), !spell->GetSpellProto()->isSpellStealthTargetCapable()) )
            return;
    }

    if(spell->m_temporaryStorage)
        spell->m_temporaryStorage->push_back(target);
    else spell->AddTarget(i, targetType, target);
}

// for the moment we do invisible targets
void SpellTargetClass::FillSpecifiedTargetsInArea(uint32 i, float srcx, float srcy, float srcz, float r, uint32 typeMask)
{
    if(!_unitCaster->IsInWorld())
        return;

    static FillAreaTargetsCallback _fillSpecificCallback;
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(&_fillSpecificCallback, this, i, SPELL_TARGET_NONE, srcx, srcy, srcz, 0.f, r, typeMask);
}

void SpellTargetClass::FillAllTargetsInArea(LocationVector & location, uint32 effIndex)
{
    FillAllTargetsInArea(effIndex, location.x, location.y, location.z, GetRadius(effIndex));
}

void SpellTargetClass::FillAllTargetsInArea(float srcx,float srcy,float srcz, uint32 effIndex)
{
    FillAllTargetsInArea(effIndex, srcx, srcy, srcz, GetRadius(effIndex));
}

/// We fill all the targets in the area, including ones in stealth
void SpellTargetClass::FillAllTargetsInArea(uint32 i, float srcx, float srcy, float srcz, float r, bool includegameobjects)
{
    std::vector<WorldObject*> ChainTargetContainer;
    // If we have max targets, we can use our temp storage to randomize our targetting
    m_temporaryStorage = m_spellInfo->MaxTargets ? &ChainTargetContainer : NULL;

    static FillAreaTargetsCallback _fillAllCallback;
    uint32 targetMask = (includegameobjects || Spell::CanEffectTargetGameObjects(m_spellInfo, i)) ? 0x00000000 : (TYPEMASK_TYPE_UNIT|TYPEMASK_TYPE_PLAYER);
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(&_fillAllCallback, this, i, SPELL_TARGET_NONE, srcx, srcy, srcz, 0.f, r, targetMask);
    if(m_temporaryStorage)
    {   // Process our temporary storage
        WorldObject* chaintarget = NULL;
        uint32 targetCount = m_spellInfo->MaxTargets;
        while(targetCount && ChainTargetContainer.size())
        {
            uint32 placeholder = (rand()%ChainTargetContainer.size());
            chaintarget = ChainTargetContainer.at(placeholder);
            if(chaintarget == NULL)
                continue;

            if(chaintarget->IsUnit())
                _AddTarget(castPtr<Unit>(chaintarget), i);
            else _AddTarget(chaintarget, i);
            ChainTargetContainer.erase(ChainTargetContainer.begin()+placeholder);
            targetCount--;
        }

        // Clear container and clear pointer
        ChainTargetContainer.clear();
        m_temporaryStorage = NULL;
    }
}

void FillAreaFriendliesCallback::operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target)
{
    if(spell->IsTargetMapFull(i, target->GetGUID()))
        return;

    Unit *unitCaster = spell->GetCaster();
    if(target->IsUnit())
    {
        if(spell->GetSpellProto()->TargetCreatureType)
        {
            Unit* Target = castPtr<Unit>(target);
            if(uint32 creatureType = Target->GetCreatureType())
            {
                if(((1<<(creatureType-1)) & spell->GetSpellProto()->TargetCreatureType) == 0)
                    return;
            } else return;
        }
    }

    // Check either combat support or friendly status
    if(!(spell->requiresCombatSupport(i) ? sFactionSystem.isCombatSupport(unitCaster, target) : sFactionSystem.isFriendly(unitCaster, target)))
        return;

    if(spell->m_temporaryStorage)
        spell->m_temporaryStorage->push_back(target);
    else spell->AddTarget(i, targetType, target);
}

// We fill all the targets in the area, including the stealthed one's
void SpellTargetClass::FillAllFriendlyInArea( uint32 i, float srcx, float srcy, float srcz, float r )
{
    if(!_unitCaster->IsInWorld())
        return;

    static FillAreaFriendliesCallback _fillFriendliesCallback;
    uint32 targetMask = Spell::CanEffectTargetGameObjects(m_spellInfo, i) ? 0x00000000 : (TYPEMASK_TYPE_UNIT|TYPEMASK_TYPE_PLAYER);
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(&_fillFriendliesCallback, this, i, SPELL_TARGET_NONE, srcx, srcy, srcz, 0.f, r, targetMask);
}

/// We fill all the gameobject targets in the area
void SpellTargetClass::FillAllGameObjectTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float r)
{
    static FillAreaTargetsCallback _fillSpecificCallback;
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(&_fillSpecificCallback, this, i, SPELL_TARGET_NONE, srcx, srcy, srcz, 0.f, r, TYPEMASK_TYPE_GAMEOBJECT);
}

WoWGuid SpellTargetClass::GetSinglePossibleEnemy(uint32 i,float prange)
{
    float rMax, rMin = m_spellInfo->minRange[0];
    if((rMax = prange) == 0.f)
    {
        rMax = m_spellInfo->maxRange[0];
        if( m_spellInfo->SpellGroupType)
        {
            _unitCaster->SM_FFValue(SMT_RADIUS,&rMax,m_spellInfo->SpellGroupType);
            _unitCaster->SM_PFValue(SMT_RADIUS,&rMax,m_spellInfo->SpellGroupType);
        }
    }

    WoWGuid ret = 0;
    std::vector<WorldObject*> tempStorage;
    m_temporaryStorage = &tempStorage;
    uint32 targetMask = TYPEMASK_TYPE_UNIT;
    if(m_spellInfo->TargetCreatureType == 0)
        targetMask |= TYPEMASK_TYPE_PLAYER;

    static FillAreaTargetsCallback _fillEnemyCallback;
    float srcx = _unitCaster->GetPositionX(), srcy = _unitCaster->GetPositionY(), srcz = _unitCaster->GetPositionZ();
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(&_fillEnemyCallback, this, i, SPELL_TARGET_NONE, srcx, srcy, srcz, rMin, rMax, targetMask);
    for(std::vector<WorldObject*>::iterator itr = tempStorage.begin(); itr != tempStorage.end(); itr++)
    {
        Unit *unit = castPtr<Unit>(*itr);
        if( m_spellInfo->TargetCreatureType && ((1<<(unit->GetCreatureType()-1) & m_spellInfo->TargetCreatureType) == 0))
            continue;
        if(_DidHit(unit) != SPELL_DID_HIT_SUCCESS)
            continue;
        ret = unit->GetGUID();
        break;
    }

    m_temporaryStorage = NULL;
    tempStorage.clear();
    return ret;
}

WoWGuid SpellTargetClass::GetSinglePossibleFriend(uint32 i,float prange)
{
    float rMax, rMin = m_spellInfo->minRange[1];
    if((rMax = prange) == 0.f)
    {
        rMax = m_spellInfo->maxRange[1];
        if( m_spellInfo->SpellGroupType)
        {
            _unitCaster->SM_FFValue(SMT_RADIUS,&rMax,m_spellInfo->SpellGroupType);
            _unitCaster->SM_PFValue(SMT_RADIUS,&rMax,m_spellInfo->SpellGroupType);
        }
    }

    WoWGuid ret = 0;
    std::vector<WorldObject*> tempStorage;
    m_temporaryStorage = &tempStorage;
    uint32 targetMask = TYPEMASK_TYPE_UNIT;
    if(m_spellInfo->TargetCreatureType == 0)
        targetMask |= TYPEMASK_TYPE_PLAYER;

    static FillAreaFriendliesCallback _fillFriendlyCallback;
    float srcx = _unitCaster->GetPositionX(), srcy = _unitCaster->GetPositionY(), srcz = _unitCaster->GetPositionZ();
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(&_fillFriendlyCallback, this, i, SPELL_TARGET_NONE, srcx, srcy, srcz, rMin, rMax, targetMask);
    for(std::vector<WorldObject*>::iterator itr = tempStorage.begin(); itr != tempStorage.end(); itr++)
    {
        Unit *unit = castPtr<Unit>(*itr);
        if( m_spellInfo->TargetCreatureType && ((1<<(unit->GetCreatureType()-1) & m_spellInfo->TargetCreatureType) == 0))
            continue;
        if(_DidHit(unit) != SPELL_DID_HIT_SUCCESS)
            continue;
        ret = unit->GetGUID();
        break;
    }

    m_temporaryStorage = NULL;
    tempStorage.clear();
    return ret;
}

void FillInRangeTargetsCallback::operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target)
{
    if(spell->IsTargetMapFull(i, target->GetGUID()))
        return;
    if(target == spell->GetCaster())
        return;

    spell->AddTarget(i, targetType, target);
}

void SpellTargetClass::AddAOETargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    //cant do raid/party stuff here, seperate functions for it
    if(TargetType & (SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID))
        return;

    LocationVector source;
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

    static FillInRangeTargetsCallback _inRangeCallback;
    uint32 targetMask = Spell::CanEffectTargetGameObjects(m_spellInfo, i) ? 0x00000000 : (TYPEMASK_TYPE_UNIT|TYPEMASK_TYPE_PLAYER);
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(&_inRangeCallback, this, i, TargetType, source.x, source.y, source.z, 0.f, r, targetMask);
}

void SpellTargetClass::AddPartyTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    WorldObject* u = _unitCaster->GetInRangeObject(m_targets.m_unitTarget);
    if((u == NULL && (u = _unitCaster) == NULL) || !u->IsPlayer())
        return;

    Player *target = castPtr<Player>(u);
    uint32 targetClassMask = 0;
    /* //Not needed in 4.3.4
    if(m_spellInfo->buffIndex > BUFF_PALADIN_BLESSING_START && m_spellInfo->buffIndex < BUFF_PALADIN_AURA_START)
        targetClassMask = target->getClassMask();*/

    AddTarget(i, TargetType, target);
    if(Group *grp = target->GetGroup())
        grp->AddMemberTargets(this, i, true, target, TargetType, r*r, maxtargets, targetClassMask);
}

void SpellTargetClass::AddRaidTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets, bool partylimit)
{
    WorldObject* u = _unitCaster->GetInRangeObject(m_targets.m_unitTarget);
    if((u == NULL && (u = _unitCaster) == NULL) || !u->IsPlayer())
        return;

    Player *target = castPtr<Player>(u);
    uint32 targetClassMask = 0;
    /* //Not needed in 4.3.4
    if(m_spellInfo->buffIndex > BUFF_PALADIN_BLESSING_START && m_spellInfo->buffIndex < BUFF_PALADIN_AURA_START)
        targetClassMask = target->getClassMask();*/

    AddTarget(i, TargetType, target);
    if(Group *grp = target->GetGroup())
        grp->AddMemberTargets(this, i, false, target, TargetType, r*r, maxtargets, targetClassMask);
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

    //is this party only?
    Player* casterFrom = NULL;
    if(_unitCaster->IsPlayer())
        casterFrom = castPtr<Player>(_unitCaster);

    Player* pfirstTargetFrom = NULL;
    if(firstTarget->IsPlayer())
        pfirstTargetFrom = castPtr<Player>(firstTarget);

    bool RaidOnly = false;
    if(casterFrom != NULL && pfirstTargetFrom != NULL && casterFrom->GetGroup() == pfirstTargetFrom->GetGroup())
        RaidOnly = true;

    uint32 jumps = m_spellInfo->EffectChainTarget[i];
    if(m_spellInfo->SpellGroupType)
        _unitCaster->SM_FIValue(SMT_JUMP_REDUCE, (int32*)&jumps, m_spellInfo->SpellGroupType);

    AddTarget(i, TargetType, firstTarget);
    if(jumps <= 1 || m_effectTargetMaps[i].size() == 0) //1 because we've added the first target, 0 size if spell is resisted
        return;

    std::vector<WorldObject*> ChainTargetContainer;
    // If we have max targets, we can use our temp storage to randomize our targetting
    m_temporaryStorage = &ChainTargetContainer;

    bool damaging = !IsHealingSpell(m_spellInfo);
    static FillAreaTargetsCallback _fillChainCallback;
    static FillAreaFriendliesCallback _fillChainFriendlyCallback;

    MapTargetCallback *callback = NULL;
    // Note, compiler doesn't like simple looking code for callbacks like these
    if(damaging) callback = &_fillChainCallback;
    else callback = &_fillChainFriendlyCallback;
    // We've successfully set a callback pointer
    uint32 targetMask = RaidOnly ? TYPEMASK_TYPE_PLAYER : TYPEMASK_TYPE_UNIT;
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(callback, this, i, TargetType, firstTarget->GetPositionX(), firstTarget->GetPositionY(), firstTarget->GetPositionZ(), 0.f, r, targetMask);
    // TODO: this technically during raid only should reprocess from each jump point to the nearest but lets be lazy
    std::vector<Unit*> backupPlans;
    for(std::vector<WorldObject*>::iterator itr = ChainTargetContainer.begin(); itr != ChainTargetContainer.end(); itr++)
    {
        Unit *target = castPtr<Unit>(*itr);
        if(target->IsPlayer() && !pfirstTargetFrom->IsGroupMember(castPtr<Player>(target)))
            continue;

        if(damaging == false && target->isFullHealth())
        {
            backupPlans.push_back(target);
            continue;
        }

        AddTarget(i, TargetType, target);
        if(--jumps == 0)
            break;
    }

    while(jumps && !backupPlans.empty())
    {
        AddTarget(i, TargetType, *backupPlans.begin());
        backupPlans.erase(backupPlans.begin());
        --jumps;
    }
    backupPlans.clear();

    // Clear container and clear pointer
    ChainTargetContainer.clear();
    m_temporaryStorage = NULL;
}

void FillInRangeConeTargetsCallback::operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target)
{
    if(spell->IsTargetMapFull(i, target->GetGUID()))
        return;
    Unit *caster = spell->GetCaster(); // TODO: check cone width instead
    if(caster == target || !caster->isTargetInFront(target))
        return;

    spell->AddTarget(i, targetType, target);
}

void SpellTargetClass::AddConeTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    static FillInRangeConeTargetsCallback _inRangeConeCallback;
    uint32 targetMask = Spell::CanEffectTargetGameObjects(m_spellInfo, i) ? 0x00000000 : (TYPEMASK_TYPE_UNIT|TYPEMASK_TYPE_PLAYER);
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(&_inRangeConeCallback, this, i, TargetType, _unitCaster->GetPositionX(), _unitCaster->GetPositionY(), _unitCaster->GetPositionZ(), 0.f, r, targetMask);
}

void FillSpecificGameObjectsCallback::operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target)
{
    if(spell->IsTargetMapFull(i, target->GetGUID()))
        return;

    GameObject* go = castPtr<GameObject>(target);
    if(go->GetInfo()->data.spellFocus.focusId != spell->GetSpellProto()->RequiresSpellFocus)
        return;
    spell->_AddTarget(target, i);
}

void SpellTargetClass::AddScriptedOrSpellFocusTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets)
{
    static FillSpecificGameObjectsCallback _specificGameObjectsCallback;
    _unitCaster->GetMapInstance()->HandleSpellTargetMapping(&_specificGameObjectsCallback, this, i, TargetType, _unitCaster->GetPositionX(), _unitCaster->GetPositionY(), _unitCaster->GetPositionZ(), 0.f, r, TYPEMASK_TYPE_GAMEOBJECT);
}

// returns Guid of lowest percentage health friendly party or raid target within sqrt('dist') yards
WoWGuid SpellTargetClass::FindLowestHealthRaidMember(Player* Target, uint32 dist)
{
    if(!Target || !Target->IsInWorld())
        return 0;

    WoWGuid lowestHealthTarget = Target->GetGUID();
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

