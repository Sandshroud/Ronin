
#include "StdAfx.h"

SpellInterface::SpellInterface(Unit *unit) : m_Unit(unit)
{
    m_currentSpell = NULL;
    m_nextMeleeSpell = NULL;
    m_lastSpell = NULL;
}

SpellInterface::~SpellInterface()
{

}

void SpellInterface::Update(uint32 msTime, uint32 uiDiff)
{
    _spellLock.Acquire();
    if(m_currentSpell != NULL)
        m_currentSpell->Update(uiDiff);
    _spellLock.Release();
}

void SpellInterface::Cleanup()
{
    CleanupCurrentSpell();
    ClearNextMeleeSpell();
}

void SpellInterface::OnRemoveFromWorld()
{
    CleanupCurrentSpell();
    ClearNextMeleeSpell();
}

void SpellInterface::CleanupCurrentSpell()
{
    _spellLock.Acquire();
    if(m_currentSpell != NULL)
        m_currentSpell->cancel();
    m_currentSpell = NULL;
    _spellLock.Release();
}

bool SpellInterface::CleanupSpecificSpell(SpellEntry *sp)
{
    if(m_currentSpell && m_currentSpell->GetSpellProto() == sp)
    {
        m_currentSpell->cancel();
        m_currentSpell = NULL;
        return true;
    }
    return false;
}

void SpellInterface::OnChangeSelection(WoWGuid guid)
{
    ClearNextMeleeSpell();
}

SpellEntry *SpellInterface::GetCurrentSpellProto()
{
    if(m_currentSpell)
        return m_currentSpell->GetSpellProto();
    return NULL;
}

void SpellInterface::ProcessSpell(Spell *castingSpell)
{
    _spellLock.Acquire();
    m_currentSpell = castingSpell;
    m_lastSpell = castingSpell->GetSpellProto();
    _spellLock.Release();
}

void SpellInterface::FinishSpell(Spell *finishedSpell)
{
    _spellLock.Acquire();
    if(m_currentSpell == finishedSpell)
        m_currentSpell = NULL;
    _spellLock.Release();
}

void SpellInterface::InterruptCast(Spell *interruptSpell, uint32 interruptTime)
{
    if(m_currentSpell == NULL || m_currentSpell == interruptSpell)
        return;
    if(m_currentSpell->getState() >= SPELL_STATE_FINISHED)
        return;
    if(interruptTime && m_currentSpell->GetSpellProto()->School)
        PreventSchoolCast(m_currentSpell->GetSpellProto()->School, interruptTime);
    CleanupCurrentSpell();
}

void SpellInterface::ProcessNextMeleeSpell(Spell *nextMeleeSpell)
{
    if(m_nextMeleeSpell)
        return;

    _spellLock.Acquire();
    m_nextMeleeSpell = nextMeleeSpell;
    _spellLock.Release();
}

void SpellInterface::LaunchSpell(SpellEntry *info, uint8 castNumber, SpellCastTargets &targets)
{
    uint8 errorOut;
    if(!checkCast(info, targets, errorOut))
    {
        if(errorOut != SPELL_FAILED_DONT_REPORT)
            castPtr<Player>(m_Unit)->SendCastResult(info->Id, errorOut, castNumber, 0);
        return;
    }

    if(Spell *spell = new Spell(m_Unit, info, castNumber))
        spell->prepare(&targets, false);
}

void SpellInterface::LaunchSpell(SpellEntry *info, float posX, float posY, float posZ)
{
    SpellCastTargets targets(m_Unit, posX, posY, posZ);
    if(Spell *spell = new Spell(m_Unit, info))
        spell->prepare(&targets, false);
}

void SpellInterface::LaunchSpell(SpellEntry *info, Unit *target)
{
    SpellCastTargets targets(target ? target->GetGUID() : m_Unit->GetGUID());
    if(Spell *spell = new Spell(m_Unit, info))
        spell->prepare(&targets, false);
}

void SpellInterface::TriggerSpell(SpellEntry *info, float posX, float posY, float posZ)
{
    SpellCastTargets targets(m_Unit, posX, posY, posZ);
    if(Spell *spell = new Spell(m_Unit, info))
        spell->prepare(&targets, true);
}

void SpellInterface::TriggerSpell(SpellEntry *info, Unit *target)
{
    SpellCastTargets targets(target->GetGUID());
    if(Spell *spell = new Spell(m_Unit, info))
        spell->prepare(&targets, true);
}

void SpellInterface::LaunchSpellFromSpell(SpellEntry *info, Unit *target, SpellEntry *parent)
{
    SpellCastTargets targets(target ? target->GetGUID() : m_Unit->GetGUID());
    if(Spell *spell = new Spell(m_Unit, info))
    {
        spell->setSpellParent(parent);
        spell->prepare(&targets, false);
    }
}

uint32 SpellInterface::getNextMeleeSpell()
{
    _spellLock.Acquire();
    uint32 ret = m_nextMeleeSpell ? m_nextMeleeSpell->GetSpellProto()->Id : 0;
    _spellLock.Release();
    return ret;
}

void SpellInterface::TriggerNextMeleeSpell(Unit *target)
{
    _spellLock.Acquire();
    if(Spell *triggerSpell = m_nextMeleeSpell)
    {
        m_nextMeleeSpell = NULL;
        _spellLock.Release();

        SpellCastTargets targets(target->GetGUID());
        triggerSpell->prepare(&targets, true);
        triggerSpell = NULL;
    } else _spellLock.Release();
}

void SpellInterface::ClearNextMeleeSpell()
{
    _spellLock.Acquire();
    if(Spell *triggerSpell = m_nextMeleeSpell)
    {
        m_nextMeleeSpell = NULL;
        _spellLock.Release();
        triggerSpell->finish();
    } else _spellLock.Release();
}

void SpellInterface::PushbackCast(uint32 school)
{
    if(m_currentSpell)
        m_currentSpell->AddTime(school);
}

void SpellInterface::PreventSchoolCast(uint32 school, uint32 duration)
{

}

bool SpellInterface::checkCast(SpellEntry *sp, SpellCastTargets &targets, uint8 &errorOut)
{
    if(!m_Unit->IsInWorld())
    {
        errorOut = SPELL_FAILED_ERROR;
        return false;
    }

    Unit *unitTarget = m_Unit->GetInRangeObject<Unit>(targets.m_unitTarget);
    if(targets.m_unitTarget && unitTarget == NULL)
    {
        errorOut = SPELL_FAILED_BAD_TARGETS;
        return false;
    }

    // Self target
    if( targets.m_targetMask == TARGET_FLAG_SELF )
        unitTarget = m_Unit;

    // check for cooldowns
    if(m_Unit->IsPlayer() && !castPtr<Player>(m_Unit)->Cooldown_CanCast(sp))
    {
        errorOut = SPELL_FAILED_NOT_READY;
        return false;
    }

    if(m_Unit->IsPlayer() && castPtr<Player>(m_Unit)->IsInDuel() && unitTarget && unitTarget->IsPlayer())
    {
        Player *pCaster = castPtr<Player>(m_Unit);
        if(pCaster->IsDuelTarget(castPtr<Player>(unitTarget), false) && !pCaster->GetDuelStorage()->isActive())
        {   // We're a duel target and the duel hasn't started yet
            errorOut = SPELL_FAILED_TARGET_DUELING;
            return false;
        }
    }

    if(m_Unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED))
    {
        errorOut = SPELL_FAILED_STUNNED;
        return false;
    }

    if(sp->Id == getNextMeleeSpell())
    {
        errorOut = SPELL_FAILED_DONT_REPORT;
        return false;
    }
    else if(sp->isNextMeleeAttack1() && getNextMeleeSpell())
    {
        errorOut = SPELL_FAILED_SPELL_IN_PROGRESS;
        return false;
    }

    if(m_currentSpell && m_currentSpell->getState() != SPELL_STATE_CASTING)
    {
        errorOut = SPELL_FAILED_SPELL_IN_PROGRESS;
        return false;
    }

    if(sp->isSpellSelfCastOnly() && targets.m_unitTarget != m_Unit->GetGUID())
    {
        errorOut = SPELL_FAILED_BAD_TARGETS;
        return false;
    }

    if(sp->isSpellCastableOnlyInOutlands() && m_Unit->GetMapId() != 530)
    {
        errorOut = SPELL_FAILED_NOT_HERE;
        return false;
    }

    if( unitTarget && m_Unit != unitTarget && sp->isSpellDamagingEffect() && !sFactionSystem.isAttackable( m_Unit, unitTarget, false ))
    {
        errorOut = SPELL_FAILED_BAD_TARGETS;
        return false;
    }

    /* Spells for the zombie event */
    if( m_Unit->GetShapeShift() == FORM_ZOMBIE && !( ((uint32)1 << (m_Unit->GetShapeShift()-1)) & sp->RequiredShapeShift  ))
    {
        errorOut = SPELL_FAILED_SPELL_UNAVAILABLE;
        return false;
    }

    if(sp->Id == 20271 && !m_Unit->HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_JUDGEMENT))
    {
        errorOut = SPELL_FAILED_SPELL_UNAVAILABLE;
        return false;
    }

    if(sp->NameHash == SPELL_HASH_DISENGAGE && !m_Unit->IsInCombat())
    {
        errorOut = SPELL_FAILED_SPELL_UNAVAILABLE;
        return false;
    }

    if(unitTarget != NULL)
    {
        // GM flagged players should be immune to other players' casts, but not their own.
        if(unitTarget->IsPlayer() && m_Unit->IsPlayer() && unitTarget != m_Unit && castPtr<Player>(m_Unit)->hasGMTag())
        {
            errorOut =SPELL_FAILED_BM_OR_INVISGOD;
            return false;
        }

        //you can't mind control someone already mind controlled
        if (sp->NameHash == SPELL_HASH_MIND_CONTROL && unitTarget->m_AuraInterface.GetAuraSpellIDWithNameHash(SPELL_HASH_MIND_CONTROL))
        {
            errorOut =SPELL_FAILED_CANT_BE_CHARMED;
            return false;
        }

        //these spells can be cast only on certain objects. Otherwise cool exploit
        //Most of this things goes to spell_forced_target table
        switch (sp->Id)
        {
            case 27907:// Disciplinary Rod
            {
                if( unitTarget->IsPlayer() )
                {
                    errorOut =SPELL_FAILED_BAD_TARGETS;
                    return false;
                }
            }break;
        }
    }

    if( m_Unit->IsPlayer() )
    {
        Player *p_caster = castPtr<Player>(m_Unit);
        if( sp->Id == 51721 && p_caster->GetAreaId() != 4281 )
        {
            errorOut =SPELL_FAILED_NOT_HERE;
            return false;
        }

        if( sp->Id == 53822 && p_caster->getClass()!=DEATHKNIGHT) // DeathGate
        {
            errorOut =SPELL_FAILED_SPELL_UNAVAILABLE;
            return false;
        }

        if( sp->NameHash == SPELL_HASH_LIFE_TAP && p_caster->GetPowerPct(POWER_TYPE_MANA) == 100 )
        {
            errorOut =SPELL_FAILED_ALREADY_AT_FULL_POWER;
            return false;
        }

        // flying auras
        if( sp->isSpellFlyingSpell() )
        {
            for(uint8 i = 0; i < 3; i++)
            {
                if(sp->EffectApplyAuraName[i] != SPELL_AURA_MOUNTED)
                    continue;

                if(p_caster->GetMountCapability(sp->EffectMiscValueB[i]) == NULL)
                {
                    errorOut =SPELL_FAILED_SPELL_UNAVAILABLE;
                    return false;
                }
                break;
            }
        }

        uint32 self_rez = p_caster->GetUInt32Value(PLAYER_SELF_RES_SPELL);
        // if theres any spells that should be cast while dead let me know
        if( !p_caster->isAlive() && self_rez != sp->Id)
        {
            if( (targets.m_targetMask & TARGET_FLAG_SELF || targets.m_unitTarget == p_caster->GetGUID() || !IsHealingSpell(sp)) && p_caster->GetShapeShift() == FORM_SPIRITOFREDEMPTION)      // not a holy spell
            {
                errorOut =SPELL_FAILED_SPELL_UNAVAILABLE;
                return false;
            }

            if(!sp->isCastableWhileDead())
            {
                errorOut =SPELL_FAILED_NOT_WHILE_GHOST;
                return false;
            }
        }

        if(unitTarget && m_Unit != unitTarget)
        {
            if(sFactionSystem.GetFactionsInteractStatus(p_caster, unitTarget, true) >= FI_STATUS_NEUTRAL)
            {
                if(!sFactionSystem.CanEitherUnitAttack(p_caster, unitTarget, false))
                {
                    errorOut =SPELL_FAILED_BAD_TARGETS;
                    return false;
                }
            } // Temporarily disabled
            else if(false && sFactionSystem.CanEitherUnitAttack(p_caster, unitTarget, false))
            {
                errorOut =SPELL_FAILED_BAD_TARGETS;
                return false;
            }
        }

        if (p_caster->GetMapInstance() && p_caster->GetMapInstance()->CanUseCollision(p_caster))
        {
            if (sp->MechanicsType == MECHANIC_MOUNTED)
            {
                // Qiraj battletanks work everywhere on map 531
                if ( p_caster->GetMapId() == 531 && ( sp->Id == 25953 || sp->Id == 26054 || sp->Id == 26055 || sp->Id == 26056 ) )
                    return true;

                if(p_caster->HasAreaFlag(OBJECT_AREA_FLAG_INDOORS))
                {
                    errorOut =SPELL_FAILED_NO_MOUNTS_ALLOWED;
                    return false;
                }
            } else if( sp->isOutdoorOnly() && p_caster->HasAreaFlag(OBJECT_AREA_FLAG_INDOORS))
            {
                errorOut =SPELL_FAILED_ONLY_OUTDOORS;
                return false;
            }
        }

        if(sp->isSpellNotActiveInBattleArena() || sp->reqInBattleground())
        {
            MapInstance *instance = p_caster->GetMapInstance();
            if(instance->IsBattleArena() && sp->isSpellNotActiveInBattleArena())
            {
                errorOut =SPELL_FAILED_NOT_IN_ARENA;
                return false;
            }
            else if(sp->reqInBattleground() && !instance->GetdbcMap()->IsBattleGround())
            {
                errorOut =SPELL_FAILED_ONLY_BATTLEGROUNDS;
                return false;
            }
        }

        // Requires ShapeShift (stealth only atm, need more work)
        if( sp->RequiredShapeShift )
        {
            if( sp->RequiredShapeShift == (uint32)1 << (FORM_STEALTH-1) )
            {
                if( !(((uint32)1 << (p_caster->GetShapeShift()-1)) & sp->RequiredShapeShift) && !p_caster->HasDummyAura(SPELL_HASH_SHADOW_DANCE) )
                {
                    errorOut =SPELL_FAILED_ONLY_STEALTHED;
                    return false;
                }
            }
        }

        if (sp->isUnavailableInCombat() && m_Unit->IsInCombat())
        {
            // Charge In Combat, it's broke since 3.3.5, ??? maybe an aura state that needs to be set now
            //if ((m_spellInfo->Id !=  100 && m_spellInfo->Id != 6178 && m_spellInfo->Id != 11578 ) )
            errorOut =SPELL_FAILED_TARGET_IN_COMBAT;
            return false;
        }


        if( sp->IsSpellWeaponSpell() && m_Unit->disarmed )
        {
            errorOut =SPELL_FAILED_EQUIPPED_ITEM_CLASS;
            return false;
        }
        if( m_Unit->disarmedShield && sp->EquippedItemInventoryTypeMask && (sp->EquippedItemInventoryTypeMask & (1 << INVTYPE_SHIELD)) )
        {
            errorOut =SPELL_FAILED_EQUIPPED_ITEM_CLASS;
            return false;
        }

        // check for duel areas
        if( sp->Id == 7266 )
        {
            if(p_caster->HasAreaFlag(OBJECT_AREA_FLAG_INCITY))
            {
                errorOut =SPELL_FAILED_NO_DUELING;
                return false;
            }

            MapInstance *instance = p_caster->GetMapInstance();
            if(instance->GetdbcMap()->IsBattleGround() || instance->GetdbcMap()->IsBattleArena())
            {
                errorOut =SPELL_FAILED_NO_DUELING;
                return false;
            }
        }

        // check if spell is allowed while player is on a taxi
        if(p_caster->GetTaxiState())
        {
            // This uses the same flag as ordinary mounts
            if(!sp->isCastableWhileMounted())
            {
                errorOut =SPELL_FAILED_NOT_ON_TAXI;
                return false;
            }
        }

        // check if spell is allowed while not mounted
        if(!p_caster->m_AuraInterface.HasMountAura())
        {
            if( sp->Id == 25860) // Reindeer Transformation
            {
                errorOut =SPELL_FAILED_ONLY_MOUNTED;
                return false;
            }
        }
        else
        {
            if (!sp->isCastableWhileMounted())
            {
                errorOut =SPELL_FAILED_NOT_MOUNTED;
                return false;
            }
        }

        // check if we have the required tools, totems, etc
        if( sp->Totem[0] != 0)
        {
            if(!p_caster->GetInventory()->GetItemCount(sp->Totem[0]))
            {
                errorOut =SPELL_FAILED_TOTEMS;
                return false;
            }
        }

        if( sp->Totem[1] != 0)
        {
            if(!p_caster->GetInventory()->GetItemCount(sp->Totem[1]))
            {
                errorOut =SPELL_FAILED_TOTEMS;
                return false;
            }
        }

        // stealth check
        if( (sp->NameHash == SPELL_HASH_STEALTH || sp->NameHash == SPELL_HASH_PROWL) && p_caster->IsInCombat() )
        {
            errorOut =SPELL_FAILED_TARGET_IN_COMBAT;
            return false;
        }

        if( ( sp->NameHash == SPELL_HASH_CANNIBALIZE || sp->Id == 46584 ))
        {
            bool check = false;
/*            for(WorldObject::InRangeArray::iterator i = p_caster->GetInRangeUnitSetBegin(); i != p_caster->GetInRangeUnitSetEnd(); i++)
            {
                Unit *target = p_caster->GetInRangeObject<Unit>(*i);
                if(p_caster->GetDistance2dSq(target) <= 25)
                    if( target->isDead() )
                        check = true;
            }*/

            if(check == false)
            {
                errorOut =SPELL_FAILED_NO_EDIBLE_CORPSES;
                return false;
            }
        }

        // check if we have the required gameobject focus
        if( sp->RequiresSpellFocus)
        {
            float focusRange;
            bool found = false;
            /*for(WorldObject::InRangeArray::iterator itr = m_caster->GetInRangeGameObjectSetBegin(); itr != m_caster->GetInRangeGameObjectSetEnd(); itr++ )
            {
                GameObject *target = p_caster->GetInRangeObject<GameObject>(*itr);
                if(target == NULL || target->GetType() != GAMEOBJECT_TYPE_SPELL_FOCUS)
                    continue;

                GameObjectInfo *info = target->GetInfo();
                if(!info)
                {
                    sLog.Debug("Spell","Warning: could not find info about game object %u", target->GetEntry());
                    continue;
                }

                // lets read the distance from database
                focusRange = (float)info->data.spellFocus.dist;

                // is that possible?
                if( !focusRange )
                    focusRange = 5.0f;

                if(!IsInrange(p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ(), target, (focusRange * focusRange)))
                    continue;

                if(info->data.spellFocus.focusId == m_spellInfo->RequiresSpellFocus)
                {
                    found = true;
                    break;
                }
            }*/

            if(!found)
            {
                errorOut =SPELL_FAILED_REQUIRES_SPELL_FOCUS;
                return false;
            }
        }

        if( sp->AreaGroupId > 0)
        {
            bool found = false;
            uint16 area_id = p_caster->GetAreaId();
            uint32 zone_id = p_caster->GetZoneId();

            AreaGroupEntry *groupEntry = dbcAreaGroup.LookupEntry( sp->AreaGroupId );
            if( groupEntry )
            {
                for ( uint8 i=0; i<7; i++ )
                {
                    if( groupEntry->AreaId[i] == zone_id || groupEntry->AreaId[i] == area_id )
                    {
                        found = true;
                        break;
                    }
                }
            }

            if(!found)
            {
                errorOut =SPELL_FAILED_REQUIRES_AREA;
                return false;
            }
        }

        // aurastate check
        if( sp->CasterAuraState)
        {
            if( !p_caster->HasFlag( UNIT_FIELD_AURASTATE, 1 << (sp->CasterAuraState-1) ) )
            {
                errorOut =SPELL_FAILED_CASTER_AURASTATE;
                return false;
            }
        }
    }

    // set up our max Range
    float maxRange = sp->maxRange[0];
    if(unitTarget && sFactionSystem.isCombatSupport(m_Unit, unitTarget))
        maxRange = sp->maxRange[1];

    if( sp->SpellGroupType )
    {
        m_Unit->SM_FFValue(SMT_RANGE, &maxRange, sp->SpellGroupType );
        m_Unit->SM_PFValue(SMT_RANGE, &maxRange, sp->SpellGroupType );
    }

    // Targeted Location Checks (AoE spells)
    if( targets.m_targetMask == TARGET_FLAG_DEST_LOCATION )
    {
        if( !IsInrange( targets.m_dest.x, targets.m_dest.y, targets.m_dest.z, m_Unit, ( maxRange * maxRange ) ) )
        {
            errorOut =SPELL_FAILED_OUT_OF_RANGE;
            return false;
        }
    }

    // Collision 2 broken for this :|
    //if(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION && !m_caster->IsInLineOfSight(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ))
    //{
    //    errorOut =SPELL_FAILED_LINE_OF_SIGHT;
    //    return false;
    //}

    if( unitTarget && unitTarget != m_Unit )
    {
        // Partha: +2.52yds to max range, this matches the range the client is calculating.
        // see extra/supalosa_range_research.txt for more info
        float targetRange = maxRange + unitTarget->GetSize() + m_Unit->GetSize() + 2.52f;
        if( !IsInrange(m_Unit, unitTarget, targetRange * targetRange ) )
        {
            errorOut =SPELL_FAILED_OUT_OF_RANGE;
            return false;
        }
    }

    if( unitTarget && m_Unit->IsPlayer() )
    {
        Player *pUnit = castPtr<Player>(m_Unit);
        if ( m_Unit != unitTarget && !m_Unit->IsInLineOfSight(unitTarget) )
        {
            errorOut =SPELL_FAILED_LINE_OF_SIGHT;
            return false;
        }

        // check aurastate
        if( sp->TargetAuraState && !unitTarget->HasFlag( UNIT_FIELD_AURASTATE, 1<<(sp->TargetAuraState-1) ) )
        {
            errorOut =SPELL_FAILED_TARGET_AURASTATE;
            return false;
        }

        if(unitTarget->IsPlayer())
        {
            // disallow spell casting in sanctuary zones
            // allow attacks in duels
            if(pUnit->IsInDuel() && pUnit->GetDuelStorage() != castPtr<Player>(unitTarget)->GetDuelStorage() && !sFactionSystem.isFriendly( m_Unit, unitTarget ) )
            {
                AreaTableEntry* atCaster = dbcAreaTable.LookupEntry( m_Unit->GetAreaId() );
                AreaTableEntry* atTarget = dbcAreaTable.LookupEntry( unitTarget->GetAreaId() );
                if( atCaster != NULL && atTarget != NULL && atCaster->AreaFlags & 0x800 || atTarget->AreaFlags & 0x800 )
                {
                    errorOut =SPELL_FAILED_NOT_HERE;
                    return false;
                }
            }

            if( sp->EffectApplyAuraName[0] == 2)//mind control
            {
                if( sp->EffectBasePoints[0])//got level req;
                {
                    if((int32)unitTarget->getLevel() > sp->EffectBasePoints[0]+1 + int32(castPtr<Player>(unitTarget)->getLevel() - sp->spellLevelSpellLevel))
                    {
                        errorOut =SPELL_FAILED_HIGHLEVEL;
                        return false;
                    }
                    else if(unitTarget->GetTypeId() == TYPEID_UNIT)
                    {
                        Creature* c =  castPtr<Creature>(unitTarget);
                        if (c&&c->GetCreatureData()&&c->GetCreatureData()->rank >ELITE_ELITE)
                        {
                            errorOut =SPELL_FAILED_HIGHLEVEL;
                            return false;
                        }
                    }
                }
            }
        }

        /***********************************************************
        * Inface checks, these are checked in 2 ways
        * 1e way is check for damage type, as 3 is always ranged
        * 2e way is trough the data in the extraspell db
        *
        **********************************************************/

        /* burlex: units are always facing the target! */
        if(castPtr<Player>(m_Unit) && sp->FacingCasterFlags)
        {
            if(!castPtr<Player>(m_Unit)->isTargetInFront(unitTarget))
            {
                errorOut =SPELL_FAILED_UNIT_NOT_INFRONT;
                return false;
            }
            if(sp->reqCasterIsBehindTarget() && (sp->Id != SPELL_RANGED_THROW) && !castPtr<Player>(m_Unit)->isInBackOfTarget(unitTarget))
            {
                errorOut =SPELL_FAILED_NOT_BEHIND;
                return false;
            }
        }

        // target 39 is fishing, all fishing spells are handled
        if( sp->EffectImplicitTargetA[0] == 39 )
        {
            uint32 entry = sp->EffectMiscValue[0];
            if(entry == GO_FISHING_BOBBER)
            {
                float px=m_Unit->GetPositionX();
                float py=m_Unit->GetPositionY();
                float orient = m_Unit->GetOrientation();
                float posx = 0,posy = 0,posz = 0;
                float co = cos(orient);
                float si = sin(orient);
                MapInstance* map = m_Unit->GetMapInstance();
                uint16 liqType;

                float r;
                for(r=20; r>10; r--)
                {
                    posx = px + r * co;
                    posy = py + r * si;
                    posz = map->GetADTWaterHeight(posx, posy, liqType);
                    if(posz > map->GetADTLandHeight(posx, posy))//water
                        break;
                }
                if(r<=10)
                {
                    errorOut =SPELL_FAILED_NOT_FISHABLE;
                    return false;
                }

                // if we are already fishing, dont cast it again
                if(castPtr<Player>(m_Unit)->GetSummonedObject() && castPtr<Player>(m_Unit)->GetSummonedObject()->GetEntry() == GO_FISHING_BOBBER)
                {
                    errorOut =SPELL_FAILED_SPELL_IN_PROGRESS;
                    return false;
                }
            }
        }

        if( sp->NameHash == SPELL_HASH_GOUGE && !unitTarget->isTargetInFront(m_Unit) )
        {
            errorOut =SPELL_FAILED_NOT_INFRONT;
            return false;
        }

        if( sp->Category==1131)//Hammer of wrath, requires target to have 20- % of hp
        {
            if(unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH) == 0)
            {
                errorOut =SPELL_FAILED_BAD_TARGETS;
                return false;
            }

            if(unitTarget->GetHealthPct() > 20)
            {
                errorOut =SPELL_FAILED_BAD_TARGETS;
                return false;
            }
        }
        else if( sp->NameHash == SPELL_HASH_CONFLAGRATE)//Conflagrate, requires immolation spell on victim
        {
            if(!unitTarget->HasAurasOfNameHashWithCaster(SPELL_HASH_IMMOLATION, NULL))
            {
                errorOut =SPELL_FAILED_BAD_TARGETS;
                return false;
            }
        }

        if( sp->NameHash == SPELL_HASH_ENVENOM )
        {
            if( !unitTarget->m_AuraInterface.HasAuraVisual(5100) )
            {
                errorOut =SPELL_FAILED_BAD_TARGETS ;
                return false;
            }
        }

        if(unitTarget->GetDispelImmunity(sp->DispelType))
        {
            errorOut =SPELL_FAILED_PREVENTED_BY_MECHANIC-1;            // hackfix - burlex
            return false;
        }
    }

    // scripted spell stuff
    switch(sp->Id)
    {
    case 603: //curse of doom, can't be casted on players
    case 30910:
    case 47867:
        {
            if(unitTarget && unitTarget->IsPlayer())
            {
                errorOut =SPELL_FAILED_TARGET_IS_PLAYER;
                return false;
            }
        }break;

    case 13907:
        {
            if (unitTarget == NULL || unitTarget->IsPlayer() || unitTarget->GetCreatureType()!=TARGET_TYPE_DEMON )
            {
                errorOut =SPELL_FAILED_SPELL_UNAVAILABLE;
                return false;
            }
        }break;

        // disable spell
    case 25997: // Eye for an Eye
    case 38554: //Absorb Eye of Grillok
        {
            // do not allow spell to be cast
            errorOut =SPELL_FAILED_SPELL_UNAVAILABLE;
            return false;
        }break;

        //These spells are NPC only.
    case 25166: //Call Glyphs of Warding
    case 38892: //Shadow Bolt
    case 40536: //Chain Lightning
    case 41078: //Shadow Blast
        {
            if(m_Unit->IsPlayer())
            {
                errorOut =SPELL_FAILED_BAD_TARGETS;
                return false;
            }
        }break;
    }

    if(unitTarget)
    {
        // if target is already skinned, don't let it be skinned again
        if( sp->Effect[0] == SPELL_EFFECT_SKINNING && unitTarget->IsUnit() && (castPtr<Creature>(unitTarget)->m_skinned) )
        {
            errorOut =SPELL_FAILED_TARGET_UNSKINNABLE;
            return false;
        }

        bool auraApplicator = false;
        if(sp->HasEffect(SPELL_EFFECT_APPLY_AURA))
            auraApplicator = true;
        else
        {
            for(uint8 i = 0; i < 3; i++)
                if(sp->EffectApplyAuraName[i])
                    auraApplicator = true;
        }

        // if we're replacing a higher rank, deny it
        if(auraApplicator)
        {
            AuraCheckResponse acr = unitTarget->m_AuraInterface.AuraCheck(sp, m_Unit->GetGUID());
            if( acr.Error == AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT )
            {
                errorOut =SPELL_FAILED_AURA_BOUNCED;
                return false;
            }
        }

        //check if we are trying to stealth or turn invisible but it is not allowed right now
        /*if( IsStealthSpell() || IsInvisibilitySpell() )
        {
            //if we have Faerie Fire, we cannot stealth or turn invisible
            if( m_Unit->m_AuraInterface.HasNegativeAuraWithNameHash( SPELL_HASH_FAERIE_FIRE ) || m_Unit->m_AuraInterface.HasNegativeAuraWithNameHash( SPELL_HASH_FAERIE_FIRE__FERAL_ ) )
            {
                errorOut =SPELL_FAILED_SPELL_UNAVAILABLE;
                return false;
            }
        }*/

        switch( sp->NameHash )
        {
        case SPELL_HASH_DIVINE_PROTECTION:
        case SPELL_HASH_DIVINE_SHIELD:
        case SPELL_HASH_HAND_OF_PROTECTION:
            {
                if( unitTarget->HasAura(25771) ) // Forbearance
                {
                    errorOut =SPELL_FAILED_DAMAGE_IMMUNE;
                    return false;
                }

                if( !unitTarget->HasDummyAura(SPELL_HASH_AVENGING_WRATH) )
                {
                    errorOut =SPELL_FAILED_DAMAGE_IMMUNE;
                    return false;
                }
            }break;

        case SPELL_HASH_AVENGING_WRATH:
            {
                if( !unitTarget->HasDummyAura(SPELL_HASH_AVENGING_WRATH) )
                {
                    errorOut =SPELL_FAILED_DAMAGE_IMMUNE;
                    return false;
                }
            }break;

        case SPELL_HASH_ICE_BLOCK:
            {
                if( unitTarget->HasAura(41425) ) // Hypothermia
                {
                    errorOut =SPELL_FAILED_DAMAGE_IMMUNE;
                    return false;
                }
            }break;

        case SPELL_HASH_POWER_WORD__SHIELD:
            {
                if( unitTarget->HasAura(6788) ) // Weakened Soul
                {
                    errorOut =SPELL_FAILED_DAMAGE_IMMUNE;
                    return false;
                }
            }break;

        case SPELL_HASH_FIRST_AID:
            {
                if( unitTarget->HasAura(11196) ) // Recently Bandaged
                {
                    errorOut =SPELL_FAILED_DAMAGE_IMMUNE;
                    return false;
                }
            }break;
        case SPELL_HASH_BLOODLUST:
            {
                if( unitTarget->HasAurasOfNameHashWithCaster(SPELL_HASH_SATED, NULL) )
                {
                    errorOut =SPELL_FAILED_DAMAGE_IMMUNE;
                    return false;
                }
            }break;
        }

        if (sp->MechanicsType == 16 && unitTarget->HasAura(11196))
        {
            errorOut =SPELL_FAILED_DAMAGE_IMMUNE;
            return false;
        }
    }

    if(m_Unit->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) > 0)
    {
        if(SpellEntry *t_spellInfo = m_Unit->GetSpellInterface()->GetCurrentSpellProto())
        {
            if( t_spellInfo->EffectTriggerSpell[0] != sp->Id &&
                t_spellInfo->EffectTriggerSpell[1] != sp->Id &&
                t_spellInfo->EffectTriggerSpell[2] != sp->Id)
            {
                errorOut =SPELL_FAILED_SPELL_IN_PROGRESS;
                return false;
            }
        }
    }

    return true;
}

void SpellInterface::AddProcData(SpellProcData *procData)
{
    m_spellProcData.insert(procData);
}

void SpellInterface::RemoveProcData(SpellProcData *procData)
{
    m_spellProcData.erase(procData);
}

bool SpellInterface::CanTriggerProc(SpellProcData *procData, time_t triggerTime, uint32 msTime)
{
    return true;
}

void SpellInterface::TriggerProc(SpellProcData *procData, Unit *target)
{
    if(m_spellProcData.find(procData) == m_spellProcData.end())
        return;

    SpellEntry *proto = procData->GetSpellProto();
    for(uint8 j = 0; j < 3; ++j)
    {
        switch(proto->EffectApplyAuraName[j])
        {
        case SPELL_AURA_PROC_TRIGGER_SPELL:
        case SPELL_AURA_PROC_TRIGGER_DAMAGE:
        case SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE:
        case SPELL_AURA_PROC_TRIGGER_SPELL_2:
            {
                if(SpellEntry *triggeredSpell = dbcSpell.LookupEntry(proto->EffectTriggerSpell[j]))
                    TriggerSpell(triggeredSpell, ((target == NULL || triggeredSpell->isSpellSelfCastOnly()) ? m_Unit : target));
            }break;
        }
    }
}
