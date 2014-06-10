/***
 * Demonstrike Core
 */

#include "StdAfx.h"

pAIEvent AIEventHandlers[NUM_AI_EVENTS] =
{
    &AIInterface::EventEnterCombat,
    &AIInterface::EventLeaveCombat,
    &AIInterface::EventDamageTaken,
    &AIInterface::EventFear,
    &AIInterface::EventUnfear,
    &AIInterface::EventFollowOwner,
    &AIInterface::EventWander,
    &AIInterface::EventUnwander,
    &AIInterface::EventUnitDied,
    &AIInterface::EventUnitRespawn,
    &AIInterface::EventHostileAction,
    &AIInterface::EventForceRedirected
};

void AIInterface::EventEnterCombat(Unit* pUnit, uint32 misc1)
{
    if( pUnit == NULL )
        return;

    /* send the message */
    if( m_Unit->IsCreature() )
    {
        Creature* cr = TO_CREATURE(m_Unit);
        if( cr->has_combat_text )
            objmgr.HandleMonsterSayEvent( cr, MONSTER_SAY_EVENT_ENTER_COMBAT );

        if( cr->m_spawn && cr->m_spawn->ChannelData )
        {
            m_Unit->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
            m_Unit->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
        }
    }

    // Stop the emote
    m_Unit->SetUInt32Value(UNIT_NPC_EMOTESTATE, GetWeaponEmoteType(false));

    MovementHandler.EventEnterCombat(misc1);

    // dismount if mounted
    if(m_Unit->IsCreature() && !(TO_CREATURE(m_Unit)->GetCreatureInfo()->Flags1 & 2048))
        m_Unit->Dismount();

    if(m_AIState != STATE_ATTACKING)
        StopMovement(0);

    m_AIState = STATE_ATTACKING;
    firstLeaveCombat = true;

    if(pUnit->GetInstanceID() == m_Unit->GetInstanceID())
        m_Unit->SetUInt64Value(UNIT_FIELD_TARGET, pUnit->GetGUID());

    CALL_SCRIPT_EVENT(m_Unit, OnCombatStart)(pUnit);

    //Mark raid as combat in progress if it concerns a boss
    if(m_Unit->IsCreature())
        if(m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->GetMapInfo() && m_Unit->GetMapMgr()->GetdbcMap()->IsRaid())
            if(Creature* cr = TO_CREATURE(m_Unit))
                if(cr->GetCreatureInfo() && (cr->GetCreatureInfo()->Rank == ELITE_WORLDBOSS || cr->GetCreatureInfo()->Flags1 & CREATURE_FLAGS1_BOSS))
                    m_Unit->GetMapMgr()->AddCombatInProgress(m_Unit->GetGUID());

    if(pUnit->IsPlayer() && TO_PLAYER(pUnit)->InGroup())
    {
        modThreatByPtr(pUnit, 1);
        Group* pGroup = TO_PLAYER(pUnit)->GetGroup();

        Player* pGroupGuy;
        GroupMembersSet::iterator itr;
        pGroup->Lock();
        for(uint32 i = 0; i < pGroup->GetSubGroupCount(); i++)
        {
            for(itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
            {
                pGroupGuy = (*itr)->m_loggedInPlayer;
                if(pGroupGuy && pGroupGuy->isAlive() && m_Unit->GetMapMgr() == pGroupGuy->GetMapMgr() && pGroupGuy->GetDistanceSq(pUnit) <= 40 * 40) //50 yards for now. lets see if it works
                {
                    AttackReaction(pGroupGuy, 1, 0);
                }
            }
        }
        pGroup->Unlock();
    }

    //Zack : Put mob into combat animation. Take out weapons and start to look serious :P
    m_Unit->smsg_AttackStart(pUnit);
}

void AIInterface::EventLeaveCombat(Unit* /*pUnit*/, uint32 misc1)
{
    if(m_Unit->IsCreature())
    {
        Creature* cr = TO_CREATURE(m_Unit);
        if(!cr->IsPet() && !cr->isAlive())
            cr->m_AuraInterface.RemoveAllNegativeAuras();

        //cancel spells that we are casting. Should remove bug where creatures cast a spell after they died
        CancelSpellCast();
        cr->SetSheatheForAttackType(0); // sheathe your weapons!

        if( cr->has_combat_text )
            objmgr.HandleMonsterSayEvent( cr, MONSTER_SAY_EVENT_ON_COMBAT_STOP );

        if(cr->m_spawn)
        {
            if(cr->m_spawn->ChannelData)
            {
                if(cr->m_spawn->ChannelData->channel_target_go)
                    sEventMgr.AddEvent( cr, &Creature::ChannelLinkUpGO, cr->m_spawn->ChannelData->channel_target_go, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0 );

                if(cr->m_spawn->ChannelData->channel_target_creature)
                    sEventMgr.AddEvent( cr, &Creature::ChannelLinkUpCreature, cr->m_spawn->ChannelData->channel_target_creature, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0 );
            }

            // Remount if mounted
            if( cr->m_spawn->MountedDisplay )
                m_Unit->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID, cr->m_spawn->MountedDisplay->MountedDisplayID );
        }
        m_Unit->SetUInt32Value(UNIT_NPC_EMOTESTATE, cr->original_emotestate);
    }

    //reset ProcCount
    ResetProcCounts(true);
    setMoveRunFlag(true);
    ai_TargetLock.Acquire();
    m_aiTargets.clear();
    ai_TargetLock.Release();
    m_fleeTimer = 0;
    m_hasFled = false;
    m_hasCalledForHelp = false;
    m_CastNext = NULL;
    SetNextTarget(NULLUNIT);
    m_Unit->CombatStatus.Vanished();

    if(m_AIType == AITYPE_PET)
    {
        m_AIState = STATE_FOLLOWING;
        SetUnitToFollow(m_PetOwner);
        SetFollowDistance(3.0f);
        ResetLastFollowPos();
        if(m_Unit->IsPet())
        {
            TO_PET(m_Unit)->SetPetAction(PET_ACTION_FOLLOW);
            if( m_Unit->GetEntry() == 416 && m_Unit->isAlive() && m_Unit->IsInWorld() )
            {
                TO_PET(m_Unit)->HandleAutoCastEvent(AUTOCAST_EVENT_LEAVE_COMBAT);
            }
        }
        HandleEvent(EVENT_FOLLOWOWNER, NULLUNIT, 0);
    }
    else
    {
        CALL_SCRIPT_EVENT(m_Unit, OnCombatStop)(getUnitToFollow());
        m_AIState = STATE_EVADE;

        SetUnitToFollow(NULLUNIT);
        SetFollowDistance(0.0f);
        ResetLastFollowPos();

        if(m_Unit->isAlive())
        {
            LocationVector m_ret = GetReturnPos();
            if(m_ret.x != 0.0f && m_ret.y != 0.0f && m_ret.z != 0.0f)
                MoveTo(m_ret.x, m_ret.y, m_ret.z, m_ret.o);
            else
            {
                SetReturnPos(m_Unit->GetSpawnX(), m_Unit->GetSpawnY(), m_Unit->GetSpawnZ(), m_Unit->GetSpawnO());
                MoveTo(m_Unit->GetSpawnX(), m_Unit->GetSpawnY(), m_Unit->GetSpawnZ(), m_Unit->GetSpawnO());
            }

            // clear tagger
            if(m_Unit->IsCreature())
                TO_CREATURE(m_Unit)->ClearTag();
        }
    }

    if( m_Unit->IsCreature() && m_Unit->GetMapMgr() )
        m_Unit->GetMapMgr()->RemoveCombatInProgress(m_Unit->GetGUID());
}

void AIInterface::EventDamageTaken(Unit* pUnit, uint32 misc1)
{
    if( pUnit == NULL )
        return;

    if( m_Unit->IsCreature() && TO_CREATURE(m_Unit)->has_combat_text )
        objmgr.HandleMonsterSayEvent( TO_CREATURE( m_Unit ), MONSTER_SAY_EVENT_ON_DAMAGE_TAKEN );

    pUnit->RemoveAura(24575);

    CALL_SCRIPT_EVENT(m_Unit, OnDamageTaken)(pUnit, float(misc1));
    if(!modThreatByPtr(pUnit, misc1))
    {
        if(pUnit->mThreatRTarget)
        {
            if(!modThreatByPtr(pUnit->mThreatRTarget, misc1))
            {
                ai_TargetLock.Acquire();
                m_aiTargets.insert(make_pair(pUnit->mThreatRTarget, misc1));
                ai_TargetLock.Release();
            }
        }
        else
        {
            ai_TargetLock.Acquire();
            m_aiTargets.insert(make_pair(pUnit, misc1));
            ai_TargetLock.Release();
        }
    }

    pUnit->CombatStatus.OnDamageDealt(m_Unit, 1);
}

void AIInterface::EventFollowOwner(Unit* /*pUnit*/, uint32 misc1)
{
    m_AIState = STATE_FOLLOWING;
    if(m_Unit->IsPet())
        TO_PET(m_Unit)->SetPetAction(PET_ACTION_FOLLOW);
    SetUnitToFollow(m_PetOwner);
    ResetLastFollowPos();
    SetFollowDistance(4.0f);

    ai_TargetLock.Acquire();
    m_aiTargets.clear();
    ai_TargetLock.Release();
    m_fleeTimer = 0;
    m_hasFled = false;
    m_hasCalledForHelp = false;
    m_CastNext = NULL;
    SetNextTarget(NULLUNIT);

    setMoveRunFlag(true);
}

void AIInterface::EventFear(Unit* pUnit, uint32 misc1)
{
    if( pUnit == NULL )
        return;

    setFearTimer(0);
    SetUnitToFear(pUnit);

    CALL_SCRIPT_EVENT(m_Unit, OnFear)(pUnit, 0);
    m_AIState = STATE_FEAR;
    StopMovement(1);

    BackupFollowInformation();
    SetUnitToFollow(NULLUNIT);
    SetFollowDistance(0.0f);
    ResetLastFollowPos();

    m_fleeTimer = 0;
    m_hasFled = false;
    m_hasCalledForHelp = false;

    // update speed
    setMoveRunFlag(true);
    getMoveFlags();

    m_CastNext = NULL;
    SetNextTarget(NULLUNIT);
}

void AIInterface::EventUnfear(Unit* pUnit, uint32 misc1)
{
    RestoreFollowInformation();
    m_AIState = STATE_IDLE; // we need this to prevent permanent fear, wander, and other problems

    SetUnitToFear(NULLUNIT);
    StopMovement(1);
}

void AIInterface::EventWander(Unit* /*pUnit*/, uint32 misc1)
{
    setWanderTimer(0);

    //CALL_SCRIPT_EVENT(m_Unit, OnWander)(pUnit, 0); FIXME
    m_AIState = STATE_WANDER;
    StopMovement(1);

    ai_TargetLock.Acquire();
    m_aiTargets.clear(); // we'll get a new target after we are unwandered
    ai_TargetLock.Release();
    m_fleeTimer = 0;
    m_hasFled = false;
    m_hasCalledForHelp = false;

    // update speed
    setMoveRunFlag(true);
    getMoveFlags();

    m_CastNext = NULL;
    SetNextTarget(NULLUNIT);
}

void AIInterface::EventUnwander(Unit* /*pUnit*/, uint32 misc1)
{
    RestoreFollowInformation();
    m_AIState = STATE_IDLE; // we need this to prevent permanent fear, wander, and other problems

    StopMovement(1);
}

void AIInterface::EventUnitDied(Unit* pUnit, uint32 misc1)
{
    if( m_Unit->GetVehicle(true) )
        TO_VEHICLE(m_Unit->GetVehicle())->RemovePassenger(m_Unit);

    if( m_Unit->IsCreature() )
    {
        if( TO_CREATURE(m_Unit)->has_combat_text )
            objmgr.HandleMonsterSayEvent( TO_CREATURE(m_Unit), MONSTER_SAY_EVENT_ON_DIED );

        ClearFollowInformation();
    }

    CALL_SCRIPT_EVENT(m_Unit, OnDied)(pUnit);

    m_AIState = STATE_IDLE;

    StopMovement(0);
    ai_TargetLock.Acquire();
    m_aiTargets.clear();
    ai_TargetLock.Release();
    SetUnitToFear(NULLUNIT);
    m_fleeTimer = 0;
    m_hasFled = false;
    m_hasCalledForHelp = false;
    m_CastNext = NULL;
    m_Unit->Dismount();
    SetNextTarget(NULLUNIT);

    //reset waypoint to 1
    setWaypointToMove(1);

    // There isn't any need to do any attacker checks here, as
    // they should all be taken care of in DealDamage
    if(m_Unit->IsCreature() && !m_Unit->IsPet())
    {
        //only save creature which exist in db (don't want to save 0 values in db)
        if( m_Unit->m_loadedFromDB && TO_CREATURE(m_Unit)->m_spawn != NULL )
        {
            MapMgr* GMap = m_Unit->GetMapMgr();
            if(GMap != NULL)
            {
                if( GMap && GMap->pInstance && GMap->GetMapInfo()->type != INSTANCE_PVP )
                {
                    GMap->pInstance->m_killedNpcs.insert( TO_CREATURE(m_Unit)->GetSQL_id() );
                    GMap->pInstance->SaveToDB();
                }
            }
        }
    }
}

void AIInterface::EventUnitRespawn(Unit* /*pUnit*/, uint32 misc1)
{
    /* send the message */
    if( m_Unit->IsCreature() )
    {
        if( TO_CREATURE(m_Unit)->has_combat_text )
            objmgr.HandleMonsterSayEvent(TO_CREATURE(m_Unit), MONSTER_SAY_EVENT_ON_SPAWN );
    }
}

void AIInterface::EventHostileAction(Unit* /*pUnit*/, uint32 misc1)
{
    SetReturnPos(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ());
}

void AIInterface::EventForceRedirected(Unit* /*pUnit*/, uint32 misc1)
{
    if(m_AIState == STATE_IDLE)
        SetReturnPos(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ());
}
