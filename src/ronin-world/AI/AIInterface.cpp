/***
 * Demonstrike Core
 */

#include "StdAfx.h"

AIInterface::AIInterface()
{
    m_CastNext = NULL;
    m_CastTimer = 0;
    m_creatureState = STOPPED;
    m_canCallForHelp = false;
    m_hasCalledForHelp = false;
    m_fleeTimer = 0;
    m_FleeDuration = 0;
    m_canFlee = false;
    m_hasFled = false;
    m_canRangedAttack = false;
    m_FleeHealth = m_CallForHelpHealth = 0.0f;
    m_AIState = STATE_IDLE;
    unitBehavior = Behavior_Default;

    m_updateListTimer = 1;
    m_updateTargetsTimer = TARGET_UPDATE_INTERVAL;

    m_nextTarget = NULL;
    m_Unit = NULL;
    m_PetOwner = NULL;
    firstLeaveCombat = true;
    m_outOfCombatRange = 10000;
    m_totemSpell = NULL;
    m_totemSpellTimer = m_totemSpellTime = 0;

    tauntedBy = NULL;
    isTaunted = false;
    m_AllowedToEnterCombat = true;
    m_currentHighestThreat = 0;

    disable_combat = false;
    disable_melee = false;
    disable_ranged = false;
    disable_spell = false;
    disable_targeting = false;

    waiting_for_cooldown = false;
    m_is_in_instance = false;
    skip_reset_hp = false;

    m_aiTargets.clear();
    m_spells.clear();
}

AIInterface::~AIInterface()
{
    if(!m_spells.empty())
    {
        for(AISpellMap::iterator itr = m_spells.begin(); itr != m_spells.end(); itr++)
            delete itr->second;
        m_spells.clear();
    }

    if(m_aiTargets.empty())
        return;
    m_aiTargets.clear();
}

void AIInterface::Init(Unit* un, AIType at, Unit *owner)
{
    ASSERT(un != NULL);
    ASSERT(owner ? (at == AITYPE_PET || at == AITYPE_TOTEM) : at != AITYPE_PET);

    m_Unit = un;
    m_AIType = at;
    m_PetOwner = owner;
    m_AIState = STATE_IDLE;

    if( m_Unit->IsCreature() && castPtr<Creature>(m_Unit)->GetCreatureData() && castPtr<Creature>(m_Unit)->GetCreatureData()->type == CRITTER )
        disable_targeting = true;
}

void AIInterface::HandleEvent(uint32 eevent, Unit* pUnit, uint32 misc1)
{
    ASSERT(m_Unit != NULL);
    if(m_Unit == NULL)
        return;

    if(eevent >= NUM_AI_EVENTS)
        return;

    // Passive NPCs (like target dummies) shouldn't do anything.
    if(m_Unit->IsCreature())
    {
        if(m_AIType == AITYPE_DUMMY)
        {
            if(eevent == EVENT_ENTERCOMBAT || eevent == EVENT_DAMAGETAKEN)
                m_Unit->SetInCombat(pUnit);
            return;
        }

        if(castPtr<Creature>(m_Unit)->GetCreatureData() && castPtr<Creature>(m_Unit)->GetCreatureData()->type == CRITTER)
            return;
    }
}

void AIInterface::Update(uint32 p_time)
{
    if(m_AIType == AITYPE_TOTEM && m_Unit->IsTotem())
    {
        _UpdateTotem(p_time);
        return;
    }

}

void AIInterface::_UpdateTotem(uint32 p_time)
{
    if(m_totemSpell == NULL)
        return;
    if(m_totemSpellTime == 0)
        return;

    if(m_totemSpellTimer <= p_time)
    {
        SpellCastTargets targets;
        if(m_totemSpell->isSpellAreaOfEffect())
        {
            m_nextTarget = NULL;
            targets.m_targetMask = TARGET_FLAG_SELF|TARGET_FLAG_SOURCE_LOCATION|TARGET_FLAG_DEST_LOCATION;
            targets.m_unitTarget = m_Unit->GetGUID();
            targets.m_src = targets.m_dest = m_Unit->GetPosition();
        }
        else
        {
            if(m_nextTarget == NULL) // Find a totem target
                SetNextTarget(FindTarget());
            else if(!m_nextTarget->IsInWorld() || !IsInrange(m_Unit, m_nextTarget, m_Unit->GetCombatReach())
                || !sFactionSystem.CanEitherUnitAttack(m_Unit, m_nextTarget, false))
                SetNextTarget(NULL);
            if(m_nextTarget==NULL)
                return;
        }

        if(m_nextTarget)
        {
            targets.m_targetMask = TARGET_FLAG_OBJECT|TARGET_FLAG_UNIT;
            targets.m_unitTarget = m_nextTarget->GetGUID();
        }

        if(Spell *pSpell = new Spell(m_Unit, m_totemSpell))
            pSpell->prepare(&targets, true);

        m_totemSpellTimer = m_totemSpellTime;
    } else m_totemSpellTimer -= p_time;
}

bool AIInterface::SetNextTarget(Unit* nextTarget)
{
    if(nextTarget) m_Unit->SetUInt64Value(UNIT_FIELD_TARGET, nextTarget->GetGUID());
    else m_Unit->SetUInt64Value(UNIT_FIELD_TARGET, 0);
    return (m_nextTarget = nextTarget) != NULL;
}

void AIInterface::AttackReaction(Unit* pUnit, uint32 damage_dealt, uint32 spellId)
{
    ASSERT(m_Unit != NULL);
    if( m_AIState == STATE_EVADE || m_fleeTimer != 0 || !pUnit || !pUnit->isAlive() || !m_Unit->isAlive() )
        return;
    if( m_Unit == pUnit || m_Unit->IsVehicle() )
        return;

}

bool AIInterface::HealReaction(Unit* caster, Unit* victim, uint32 amount, SpellEntry * sp)
{
    ASSERT(m_Unit != NULL);

    if(!caster || !victim)
    {
        //printf("!!!BAD POINTER IN AIInterface::HealReaction!!!\n");
        return false;
    }

    // apply spell modifiers
    if (sp != NULL && sp->SpellGroupType)
    {
        caster->SM_FIValue(SMT_THREAT_REDUCED,(int32*)&amount,sp->SpellGroupType);
        caster->SM_PIValue(SMT_THREAT_REDUCED,(int32*)&amount,sp->SpellGroupType);
    }

    bool casterInList = false, victimInList = false;

    ai_TargetLock.Acquire();
    if(m_aiTargets.find(caster->GetGUID()) != m_aiTargets.end())
        casterInList = true;
    if(m_aiTargets.find(victim->GetGUID()) != m_aiTargets.end())
        victimInList = true;
    ai_TargetLock.Release();

    if(!victimInList && !casterInList) // none of the Casters is in the Creatures Threat list
        return false;

    if(!casterInList && victimInList) // caster is not yet in Combat but victim is
    {
        // get caster into combat if he's hostile
        if(sFactionSystem.CanEitherUnitAttack(m_Unit, caster))
        {
            ai_TargetLock.Acquire();
            m_aiTargets.insert(std::make_pair(caster->GetGUID(), amount));
            ai_TargetLock.Release();
            return true;
        }
        return false;
    }
    else if(casterInList && victimInList) // both are in combat already
    {
        // mod threat for caster
        modThreat(caster->GetGUID(), amount);
        return true;
    }
    else // caster is in Combat already but victim is not
    {
        modThreat(caster->GetGUID(), amount);
        // both are players so they might be in the same group
        if( caster->IsPlayer() && victim->IsPlayer() )
        {
            if( castPtr<Player>( caster )->GetGroup() == castPtr<Player>( victim )->GetGroup() )
            {
                // get victim into combat since they are both
                // in the same party
                if( sFactionSystem.CanEitherUnitAttack( m_Unit, victim ) )
                {
                    ai_TargetLock.Acquire();
                    m_aiTargets.insert(std::make_pair( victim->GetGUID(), 1 ) );
                    ai_TargetLock.Release();
                    return true;
                }
                return false;
            }
        }
    }

    return false;
}

void AIInterface::OnDeath(WorldObject* pKiller)
{
    ASSERT(m_Unit != NULL);

    if(pKiller != NULL && pKiller->IsUnit())
        HandleEvent(EVENT_UNITDIED, castPtr<Unit>(pKiller), 0);
    else
        HandleEvent(EVENT_UNITDIED, m_Unit, 0);
}

void AIInterface::OnRespawn(Unit* unit)
{
    ASSERT(unit != NULL);

    HandleEvent(EVENT_UNITRESPAWN, unit, 0);
}

bool AIInterface::FindFriends(float dist)
{
    ASSERT(m_Unit != NULL);
    if( m_Unit->IsPet() ) //pet's do not have friends; Players are exploiting this :-/
        return false;
    if(!ai_TargetLock.AttemptAcquire())
        return false;
    ai_TargetLock.Release();

    Creature *result = NULL;
    for(WorldObject::InRangeSet::iterator itr = m_Unit->GetInRangeUnitSetBegin(); itr != m_Unit->GetInRangeUnitSetEnd(); itr++)
    {
        Unit *unit = m_Unit->GetInRangeObject<Unit>(*itr);
        if(unit == NULL || !unit->IsInWorld() || unit->IsPlayer())
            continue;

        Creature *pCreature = castPtr<Creature>(unit);
        if(!pCreature->isAlive())
            continue;
        if(pCreature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
            continue;
        if(pCreature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9))
            continue;
        if( pCreature->GetAIInterface()->getAIState() != STATE_IDLE )
            continue;
        if( !sFactionSystem.CanEitherUnitAttack(GetMostHated(), pCreature) )
            continue;
        if( sFactionSystem.isCombatSupport( m_Unit, pCreature ) )
        {
            if( m_Unit->GetDistanceSq(pCreature) < dist)
            {
                result = pCreature;
                break;
            }
        }
    }

    if(result)
    {
        ai_TargetLock.Acquire();
        TargetMap::iterator it, it2;
        for(TargetMap::iterator it = m_aiTargets.begin(), it2; it != m_aiTargets.end();)
            if(Unit *unit = m_Unit->GetMapInstance()->GetUnit((it2 = it++)->first))
                result->GetAIInterface()->AttackReaction( unit, 1, 0 );
        ai_TargetLock.Release();
        return true;
    }
    return false;
}

float AIInterface::_CalcDistanceFromHome()
{
    ASSERT(m_Unit != NULL);
    if (m_AIType == AITYPE_PET)
        return m_Unit->GetDistanceSq(m_PetOwner);
    else if(m_Unit->GetTypeId() == TYPEID_UNIT)
    {
        LocationVector m_Ret = m_Unit->GetCombatEnterLoc();
        if(m_Ret.x != 0.0f && m_Ret.y != 0.0f)
            return m_Unit->GetDistanceSq(m_Ret.x, m_Ret.y, m_Ret.z);
    }

    return 0.0f;
}

bool AIInterface::setInFront(Unit* target) // not the best way to do it, though
{
    ASSERT(m_Unit != NULL);

    float dx = target->GetPositionX() - m_Unit->GetPositionX();
    float dy = target->GetPositionY() - m_Unit->GetPositionY();
    float ang = atan2(dy, dx); ang = (ang >= 0) ? ang : 2 * M_PI + ang;
    m_Unit->SetPosition(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ(), ang);
    return m_Unit->isTargetInFront(target);
}

void AIInterface::ResetProcCounts(bool all)
{
    uint32 time = getMSTime();
    if(m_spells.size())
    {
        for(Loki::AssocVector<uint32, AI_Spell*>::iterator itr = m_spells.begin(); itr != m_spells.end(); itr++)
        {
            if(!itr->second->ProcLimit)
                continue;

            if(!all && itr->second->ProcResetDelay)
                if(itr->second->ProcResetTimer > time)
                    continue;
            itr->second->procCounter = 0;
        }
    }
}

void AIInterface::WipeCurrentTarget()
{
    if(m_nextTarget == NULL)
        return;

    TargetMap::iterator itr;
    if( (itr = m_aiTargets.find(m_nextTarget->GetGUID())) != m_aiTargets.end() )
        m_aiTargets.erase( itr );

    SetNextTarget(NULL);
}

/* Crow: THIS FUNCTION IS HEAVILY DEPENDENT ON THE CREATURE PROTO COLUMN!
void AIInterface::CheckHeight()
{
    ASSERT(m_Unit != NULL);

    if(m_Unit->GetMapInstance())
    {
        if(m_Unit->IsCreature())
        {
            if(!(castPtr<Creature>(m_Unit)->CanMove & LIMIT_AIR))
            {
                m_moveFly = false;
                return;
            }
        }

        uint32 m = m_Unit->GetMapId();
        float x = m_Unit->GetPositionX();
        float y = m_Unit->GetPositionY();
        float z = m_Unit->GetPositionZ();
        if(m_destinationX && m_destinationY)
        {
            x = m_destinationX;
            y = m_destinationY;
            z = (z > m_destinationZ ? z : m_destinationZ); // Crow: Call it hacky, but it works.
        }

        float landheight_z = m_Unit->GetCHeightForPosition(true, x, y, z);
        if(landheight_z)
        {
            if(landheight_z < (z-3.0f))
                m_moveFly = true;
            else
                m_moveFly = false;
        }
        m_Unit->UpdateVisibility();
    }
}*/

uint32 AIInterface::GetWeaponEmoteType(bool ranged)
{
    uint32 emotetype = EMOTE_STATE_READY_UNARMED;
    if(ranged) emotetype = EMOTE_STATE_READY_BOW;
    if(!m_Unit->IsCreature())
        return emotetype;

    if(CreatureData *ctrData = castPtr<Creature>(m_Unit)->GetCreatureData())
    {
        if(ranged == false)
        {
            uint32 weaponids[2] = { ctrData->inventoryItem[0], ctrData->inventoryItem[1] };
            for(uint8 i = 0; i < 2; i++)
            {
                if(weaponids[i])
                {
                    if(ItemDataEntry* ItemE = db2Item.LookupEntry(weaponids[i]))
                    {
                        switch(ItemE->InventoryType)
                        {
                        case INVTYPE_WEAPON:
                        case INVTYPE_WEAPONMAINHAND:
                        case INVTYPE_WEAPONOFFHAND:
                            {
                                emotetype = EMOTE_STATE_READY1H;
                            }break;
                        case INVTYPE_2HWEAPON:
                            {
                                emotetype = EMOTE_STATE_READY2H;
                            }break;
                        }
                    }
                }
            }
        }
        else if(ctrData->inventoryItem[2])
        {
            if(ItemDataEntry* ItemE = db2Item.LookupEntry(ctrData->inventoryItem[2]))
                if(ItemE->SubClass == ITEM_SUBCLASS_WEAPON_GUN || ItemE->SubClass == ITEM_SUBCLASS_WEAPON_CROSSBOW)
                    emotetype = EMOTE_STATE_READY_RIFLE;
        }
    }
    return emotetype;
}
