/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Unit::Unit()
{
#ifdef SHAREDPTR_DEBUGMODE
    printf("Unit::Unit()\n");
#endif
    m_lastHauntInitialDamage = 0;
    m_attackTimer = 0;
    m_attackTimer_1 = 0;
    m_duelWield = false;

    memset(&movement_info, 0, sizeof(MovementInfo));
    m_ignoreArmorPct = 0.0f;
    m_ignoreArmorPctMaceSpec = 0.0f;
    m_fearmodifiers = 0;
    m_state = 0;
    m_special_state = 0;
    m_deathState = ALIVE;
    m_currentSpell = NULLSPELL;
    m_meleespell = 0;
    m_meleespell_cn = 0;
    m_addDmgOnce = 0;

    m_silenced = 0;
    disarmed = false;
    disarmedShield = false;

    // Pet
    m_isPet = false;
    // Pet Talents...WOOT!
    m_PetTalentPointModifier = 0;

    //Vehicle
    m_teleportAckCounter = 0;
    m_inVehicleSeatId = 0xFF;
    m_CurrentVehicle = NULLVEHICLE;
    ExitingVehicle = false;

    //DK:modifiers
    PctRegenModifier = 0;
    for( uint32 x = 0; x < 4; x++ )
    {
        m_ObjectSlots[x] = 0;
        PctPowerRegenModifier[x] = 1;
    }
    m_speedModifier = 0;
    m_slowdown = 0;
    m_mountedspeedModifier=0;
    m_maxSpeed = 0;
    for(uint32 x=0;x<MECHANIC_COUNT;x++)
    {
        MechanicsDispels[x]=0;
        MechanicsResistancesPCT[x]=0;
        ModDamageTakenByMechPCT[x]=0;
    }

    for (uint32 i=0; i<NUM_DISPELS; i++)
        DispelResistancesPCT[i] = 0;
    //SM
    memset(SM, 0, 2*SPELL_MODIFIERS*sizeof(int32 *));

    m_pacified = 0;
    m_interruptRegen = 0;
    m_resistChance = 0;
    m_powerRegenPCT = 0;
    RAPvModifier=0;
    APvModifier=0;
    stalkedby=0;

    m_extraattacks[0] = 0;
    m_extraattacks[1] = 0;
    m_stunned = 0;
    m_manashieldamt=0;
    m_rooted = 0;
    m_triggerSpell = 0;
    m_triggerDamage = 0;
    m_canMove = 0;
    m_noInterrupt = 0;
    m_modlanguage = -1;
    m_modelhalfsize = 1.0f; //worst case unit size. (Should be overwritten)

    m_useAI = false;
    for(uint32 x=0;x<10;x++)
        dispels[x]=0;

    for(uint32 x=0;x<12;x++)
    {
        CreatureAttackPowerMod[x] = 0;
        CreatureRangedAttackPowerMod[x] = 0;
    }
    //REMIND:Update these if you make any changes
    CreatureAttackPowerMod[UNIT_TYPE_MISC] = 0;
    CreatureRangedAttackPowerMod[UNIT_TYPE_MISC] = 0;
    CreatureAttackPowerMod[11] = 0;
    CreatureRangedAttackPowerMod[11] = 0;

    m_invisible = false;
    m_invisFlag = INVIS_FLAG_NORMAL;

    for(int i = 0; i < INVIS_FLAG_TOTAL; i++)
        m_invisDetect[i] = 0;

    m_stealthLevel = 0;
    m_stealthDetectBonus = 0;
    m_stealth = 0;
    m_can_stealth = true;

    for(uint32 x=0;x<5;x++)
    {
        BaseStats[x] = 0;
        SpellHealDoneByAttribute[x] = 0;
        for(uint32 i = 0; i < 7; i++)
            SpellDmgDoneByAttribute[x][i] = 0;
    }

    m_AreaUpdateTimer = 0;
    m_lastAreaPosition.ChangeCoords(0.0f, 0.0f, 0.0f);
    m_emoteState = 0;
    m_oldEmote = 0;
    m_charmtemp = 0;

    BaseDamage[0]=0;
    BaseOffhandDamage[0]=0;
    BaseRangedDamage[0]=0;
    BaseDamage[1]=0;
    BaseOffhandDamage[1]=0;
    BaseRangedDamage[1]=0;

    m_CombatUpdateTimer = 0;
    HealDoneBonusBySpell.clear();

    HealDoneBase = 0;
    HealDoneModPos = 0;
    HealDonePctMod = 1.0f;
    HealTakenMod = 0;
    HealTakenPctMod = 1.0f;
    SpellHealFromAP = 0;
    Expertise[0] = Expertise[1] = 0;

    for(uint32 x=0;x<7;x++)
    {
        SchoolImmunityList[x] = 0;
        BaseResistance[x] = 0;
        DamageDoneMod[x] = 0;
        DamageDonePosMod[x] = 0;
        DamageDoneNegMod[x] = 0;
        DamageDonePctMod[x] = 1.0f;
        DamageTakenMod[x] = 0;
        SchoolCastPrevent[x] = 0;
        DamageTakenPctMod[x] = 1;
        SpellCritChanceSchool[x] = 0;
        SpellDamageFromAP[x] = 0;
        PowerCostMod[x] = 0;
        PowerCostPctMod[x] = 0; // armor penetration & spell penetration
        AttackerCritChanceMod[x] = 0;
        CritMeleeDamageTakenPctMod[x] = 0;
        CritRangedDamageTakenPctMod[x] = 0;
    }

    RangedDamageTaken = 0;
    AOEDmgMod = 1.0f;

    for(int i = 0; i < 5; i++)
    {
        m_detectRangeGUID[i] = 0;
        m_detectRangeMOD[i] = 0;
    }

    m_threatModifyer = 0;
    m_generatedThreatModifyer = 0;
    m_AuraInterface.DeInit();

    // diminishing return stuff
    memset(m_diminishAuraCount, 0, DIMINISH_GROUPS);
    memset(m_diminishCount, 0, DIMINISH_GROUPS*sizeof(uint16));
    memset(m_diminishTimer, 0, DIMINISH_GROUPS*sizeof(uint16));

    m_diminishActive = false;
    pLastSpell = 0;
    m_flyspeedModifier = 0;
    bInvincible = false;
    m_redirectSpellPackets = NULLPLR;
    can_parry = false;
    bProcInUse = false;
    spellcritperc = 0;

    polySpell = 0;
    RangedDamageTaken = 0;
    m_procCounter = 0;
    m_procOverspill = 0;
    m_damgeShieldsInUse = false;
    m_temp_summon=false;
    m_p_DelayTimer = 0;
    mAngerManagement = false;
    mRecentlyBandaged = false;

    mThreatRTarget = NULLUNIT;
    mThreatRAmount = 0;

    m_vampiricTouch = 0;

    m_soulSiphon.amt = 0;
    m_soulSiphon.max = 0;

    m_frozenTargetCharges = 0;
    m_frozenTargetId = 0;
    m_incanterAbsorption = 0;
    m_hotStreakCount = 0;

    trigger_on_stun = 0;
    trigger_on_stun_chance = 100;

    memset(&m_damageSplitTarget, 0, sizeof(DamageSplitTarget));
    m_damageSplitTarget.active = false;

    m_extrastriketarget = 0;
    m_extrastriketargetc = 0;
    m_extraStrikeTargets.clear();

    m_onAuraRemoveSpells.clear();

    m_damageShields.clear();
    m_reflectSpellSchool.clear();
    m_procSpells.clear();
    m_chargeSpells.clear();
    m_chargeSpellRemoveQueue.clear();
    tmpAura.clear();
    m_DummyAuras.clear();
    m_LastSpellManaCost = 0;

    for (uint32 x = 0;x<NUM_CUSTOM_TIMERS;x++)
    {
        m_CustomTimers[x] = 0;
    }

    m_mountSpell = 0;
    m_vehicleEntry = 0;
    for(uint8 i = 0; i < 8; i++)
    {
        m_vehicleSeats[i] = NULL;
        seatisusable[i] = false;
        m_passengers[i] = NULL;
    }
    pVehicle = NULL;
}

Unit::~Unit()
{

}

void Unit::Init()
{
    Object::Init();

    m_AuraInterface.Init(this);

    m_aiInterface = new AIInterface();
    m_aiInterface->Init(TO_UNIT(this), AITYPE_AGRO, MOVEMENTTYPE_NONE);

    CombatStatus.SetUnit(TO_UNIT(this));
    SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER );
}

void Unit::Destruct()
{
    sEventMgr.RemoveEvents(this);

    m_AuraInterface.RemoveAllAuras();

    if (IsInWorld())
        RemoveFromWorld(true);

    for(uint32 x = 0; x < SPELL_MODIFIERS; ++x)
    {
        for(uint32 y = 0; y < 2; ++y)
            if(SM[x][y])
                delete [] SM[x][y];
    }

    delete m_aiInterface;

    m_redirectSpellPackets = NULLPLR;

    if(m_currentSpell)
        m_currentSpell->cancel();

    if( GetVehicle() != NULL )
    {
        GetVehicle()->RemovePassenger( TO_UNIT(this) );
        SetVehicle(NULLVEHICLE);
    }

    // clear tmpAura pointers
    for(map<uint32, Aura* >::iterator itr = tmpAura.begin(); itr != tmpAura.end(); itr++)
    {
        if( itr->second )
        {
            itr->second->m_tmpAuradeleted = true;
            itr->second->Remove();
        }
    }
    tmpAura.clear();
    m_DummyAuras.clear();

    for(std::list<ExtraStrike*>::iterator itr = m_extraStrikeTargets.begin();itr != m_extraStrikeTargets.end();itr++)
        delete (*itr);
    m_extraStrikeTargets.clear();

    HM_NAMESPACE::hash_map<uint32, onAuraRemove*>::iterator itr;
    for ( itr = m_onAuraRemoveSpells.begin() ; itr != m_onAuraRemoveSpells.end() ; itr++)
    {
        delete itr->second;
    }
    m_onAuraRemoveSpells.clear();

    if(GetMapMgr())
    {
        CombatStatus.Vanished();
        CombatStatus.SetUnit( NULLUNIT );
    }

    m_damageShields.clear();

    for (std::list<ReflectSpellSchool*>::iterator itr=m_reflectSpellSchool.begin(); itr!=m_reflectSpellSchool.end(); itr++)
        delete (*itr);
    m_reflectSpellSchool.clear();
    m_procSpells.clear();

    DamageTakenPctModPerCaster.clear();

    m_chargeSpells.clear();
    m_chargeSpellRemoveQueue.clear();
    Object::Destruct();
}

void Unit::SetDiminishTimer(uint32 index)
{
    assert(index < DIMINISH_GROUPS);

    m_diminishActive = true;
    m_diminishTimer[index] = 15000;
}

void Unit::Update( uint32 p_time )
{
    _UpdateSpells( p_time );

    if(!isDead())
    {
        if(!IsPlayer())
        {
            m_AreaUpdateTimer += p_time;
            if(m_AreaUpdateTimer >= 2000)
            {
                if(m_lastAreaPosition.Distance(GetPosition()) > sWorld.AreaUpdateDistance)
                {
                    // Update our area id and position
                    UpdateAreaInfo();
                    m_lastAreaPosition = GetPosition();
                }
                m_AreaUpdateTimer = 0;
            }
        }

        CombatStatus.UpdateTargets();

        /*-----------------------POWER & HP REGENERATION-----------------*/
        if( p_time >= m_H_regenTimer )
            RegenerateHealth();
        else
            m_H_regenTimer -= p_time;

        if(!IsPlayer())
        {
            if(m_p_DelayTimer > p_time)
                m_p_DelayTimer -= p_time;
            else m_p_DelayTimer = 0;

            if( p_time < m_P_regenTimer )
                RegeneratePower( m_p_DelayTimer > 0 );
            else m_P_regenTimer -= p_time;
        }

        if(m_aiInterface != NULL && m_useAI)
            m_aiInterface->Update(p_time);
    }

    if(m_diminishActive)
    {
        uint32 count = 0;
        for(uint32 x = 0; x < DIMINISH_GROUPS; ++x)
        {
            // diminishing return stuff
            if(m_diminishTimer[x] && !m_diminishAuraCount[x])
            {
                if(p_time >= m_diminishTimer[x])
                {
                    // resetting after 15 sec
                    m_diminishTimer[x] = 0;
                    m_diminishCount[x] = 0;
                }
                else
                {
                    // reducing, still.
                    m_diminishTimer[x] -= p_time;
                    ++count;
                }
            }
        }
        if(!count)
            m_diminishActive = false;
    }
}

bool Unit::canReachWithAttack(Unit* pVictim)
{
    if(GetMapId() != pVictim->GetMapId())
        return false;

//  float targetreach = pVictim->GetCombatReach();
    float selfreach;
    if(IsPlayer())
        selfreach = 5.0f; // minimum melee range, UNIT_FIELD_COMBATREACH is too small and used eg. in melee spells
    else
        selfreach = m_floatValues[UNIT_FIELD_COMBATREACH];

    float targetradius;
//  targetradius = pVictim->m_floatValues[UNIT_FIELD_BOUNDINGRADIUS]; //this is plain wrong. Represents i have no idea what :)
    targetradius = pVictim->GetModelHalfSize();
    float selfradius;
//  selfradius = m_floatValues[UNIT_FIELD_BOUNDINGRADIUS];
    selfradius = GetModelHalfSize();
//  float targetscale = pVictim->m_floatValues[OBJECT_FIELD_SCALE_X];
//  float selfscale = m_floatValues[OBJECT_FIELD_SCALE_X];

    //float distance = sqrt(GetDistanceSq(pVictim));
    float delta_x = pVictim->GetPositionX() - GetPositionX();
    float delta_y = pVictim->GetPositionY() - GetPositionY();
    float distance = sqrt(delta_x * delta_x + delta_y * delta_y);


//  float attackreach = (((targetradius*targetscale) + selfreach) + (((selfradius*selfradius)*selfscale)+1.50f));
    float attackreach = targetradius + selfreach + selfradius;

    //formula adjustment for player side.
    if(IsPlayer())
    {
        // latency compensation!!
        // figure out how much extra distance we need to allow for based on our movespeed and latency.
        if(pVictim->IsPlayer() && TO_PLAYER(pVictim)->m_isMoving)
        {
            // this only applies to PvP.
            uint32 lat = TO_PLAYER(pVictim)->GetSession() ? TO_PLAYER(pVictim)->GetSession()->GetLatency() : 0;

            // if we're over 500 get fucked anyway.. your gonna lag! and this stops cheaters too
            lat = (lat > 500) ? 500 : lat;

            // calculate the added distance
            attackreach += m_runSpeed * 0.001f * lat;
        }

        if(TO_PLAYER(this)->m_isMoving)
        {
            // this only applies to PvP.
            uint32 lat = TO_PLAYER(this)->GetSession() ? TO_PLAYER(this)->GetSession()->GetLatency() : 0;

            // if we're over 500 get fucked anyway.. your gonna lag! and this stops cheaters too
            lat = (lat > 500) ? 500 : lat;

            // calculate the added distance
            attackreach += m_runSpeed * 0.001f * lat;
        }
    }
    return (distance <= attackreach);
}

void Unit::GiveGroupXP(Unit* pVictim, Player* PlayerInGroup)
{
    if(!PlayerInGroup)
        return;
    if(!pVictim)
        return;
    if(!PlayerInGroup->InGroup())
        return;
    Group *pGroup = PlayerInGroup->GetGroup();
    uint32 xp = 0;
    if(!pGroup)
        return;

    //Get Highest Level Player, Calc Xp and give it to each group member
    Player* pHighLvlPlayer = NULLPLR;
    Player* pGroupGuy = NULLPLR;
      int active_player_count=0;
    Player* active_player_list[MAX_GROUP_SIZE_RAID];//since group is small we can afford to do this ratehr then recheck again the whole active player set
    int total_level=0;
    float xp_mod = 1.0f;

    //change on 2007 04 22 by Zack
    //we only take into count players that are near us, on same map
    GroupMembersSet::iterator itr;
    pGroup->Lock();
    for(uint32 i = 0; i < pGroup->GetSubGroupCount(); i++)
    {
        for(itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); itr++)
        {
            pGroupGuy = (*itr)->m_loggedInPlayer;
            if( pGroupGuy && pGroupGuy->isAlive()
                && pVictim->GetMapMgr() == pGroupGuy->GetMapMgr()
                && pGroupGuy->GetDistanceSq(pVictim)<100*100 )
            {
                active_player_list[active_player_count] = pGroupGuy;
                active_player_count++;
                total_level += pGroupGuy->getLevel();
                if(pHighLvlPlayer)
                {
                    if(pGroupGuy->getLevel() > pHighLvlPlayer->getLevel())
                        pHighLvlPlayer = pGroupGuy;
                }
                else
                    pHighLvlPlayer = pGroupGuy;
            }
        }
    }
    pGroup->Unlock();
    if(active_player_count<1) //killer is always close to the victim. This should never execute
    {
        if(PlayerInGroup == 0)
        {
            PlayerInfo * pleaderinfo = pGroup->GetLeader();
            if(!pleaderinfo->m_loggedInPlayer)
                return;

            PlayerInGroup = pleaderinfo->m_loggedInPlayer;
        }

        xp = CalculateXpToGive(pVictim, PlayerInGroup);
        PlayerInGroup->GiveXP(xp, pVictim->GetGUID(), true);
    }
    else
    {
        if( pGroup->GetGroupType() == GROUP_TYPE_PARTY)
        {
            if(active_player_count==3)
                xp_mod=1.1666f;
            else if(active_player_count==4)
                xp_mod=1.3f;
            else if(active_player_count==5)
                xp_mod=1.4f;
            else xp_mod=1;//in case we have only 2 members ;)
        }
        else if(pGroup->GetGroupType() == GROUP_TYPE_RAID)
            xp_mod=0.5f;

        if(pHighLvlPlayer == 0)
        {
            PlayerInfo * pleaderinfo = pGroup->GetLeader();
            if(!pleaderinfo->m_loggedInPlayer)
                return;

            pHighLvlPlayer = pleaderinfo->m_loggedInPlayer;
        }

        xp = CalculateXpToGive(pVictim, pHighLvlPlayer);
        //i'm not sure about this formula is correct or not. Maybe some brackets are wrong placed ?
        for(int i=0;i<active_player_count;++i)
            active_player_list[i]->GiveXP( float2int32(((xp*active_player_list[i]->getLevel()) / total_level)*xp_mod), pVictim->GetGUID(), true );
    }
}

uint32 Unit::HandleProc( uint32 flag, uint32 flag2, Unit* victim, SpellEntry* CastingSpell, int32 dmg, uint32 abs, uint32 weapon_damage_type )
{
    ++m_procCounter;
    uint32 resisted_dmg = 0;
    bool can_delete = !bProcInUse; //if this is a nested proc then we should have this set to TRUE by the father proc
    bProcInUse = true; //locking the proc list
    uint32 mstimenow = getMSTime();

    ProcDataHolder procData(flag, flag2, victim, CastingSpell, dmg, abs, weapon_damage_type);
    std::list< struct ProcTriggerSpell >::iterator itr,itr2;
    for( itr = m_procSpells.begin(); itr != m_procSpells.end(); )  // Proc Trigger Spells for Victim
    {
        itr2 = itr;
        ++itr;

        if( itr2->deleted )
        {
            if( can_delete )
                m_procSpells.erase( itr2 );
            continue;
        }

        if (itr2->LastTrigger + 50 >= mstimenow)
            continue;

        uint32 origId = itr2->origId;
        if( CastingSpell != NULL )
        {
            //this is to avoid spell proc on spellcast loop. We use dummy that is same for both spells
            if( CastingSpell->Id == origId || CastingSpell->Id == itr2->spellId )
                continue;
        }

        SpellEntry* sp = dbcSpell.LookupEntry( itr2->spellId );
        SpellEntry* ospinfo = dbcSpell.LookupEntry( origId );
        if(sp == NULL)
            continue;

        //this requires some specific spell check,not yet implemented
        if( (flag && (itr2->procFlags & flag)) || (flag2 && (itr2->procflags2 & flag2)) ) // Check both.
        {
            if(itr2->weapon_damage_type > 0 && itr2->weapon_damage_type < 3 &&
                (itr2->procFlags & (PROC_ON_MELEE_ATTACK | PROC_ON_CRIT_ATTACK)) &&
                itr2->weapon_damage_type != weapon_damage_type)
                continue; // This spell should proc only from other hand attacks

            uint32 spellId = itr2->spellId;
            uint32 proc_Chance = itr2->procChance;
            if( itr2->procFlags & PROC_ON_CAST_SPELL || itr2->procFlags & PROC_ON_SPELL_LAND || itr2->procFlags & PROC_ON_CAST_SPECIFIC_SPELL || itr2->procFlags & PROC_ON_ANY_HOSTILE_ACTION || (itr2->procFlags & PROC_ON_PHYSICAL_ATTACK && sp->Spell_Dmg_Type & SPELL_DMG_TYPE_MELEE))
            {
                if( CastingSpell == NULL )
                    continue;

                if( itr2->SpellClassMask[0] || itr2->SpellClassMask[1] || itr2->SpellClassMask[2] )
                {
                    if (!(itr2->SpellClassMask[0] & CastingSpell->SpellGroupType[0]) &&
                        !(itr2->SpellClassMask[1] & CastingSpell->SpellGroupType[1]) &&
                        !(itr2->SpellClassMask[2] & CastingSpell->SpellGroupType[2]))
                        continue;
                }
                else if( itr2->procFlags & PROC_ON_CAST_SPECIFIC_SPELL )
                {
                    //this is wrong, dummy is too common to be based on this, we should use spellgroup or something
                    SpellEntry* sp = dbcSpell.LookupEntry( spellId );
                    if( sp->SpellIconID != CastingSpell->SpellIconID )
                    {
                        if( !ospinfo->School )
                            continue;
                        if( ospinfo->School != CastingSpell->School )
                            continue;
                        if( CastingSpell->EffectImplicitTargetA[0] == 1 ||
                            CastingSpell->EffectImplicitTargetA[1] == 1 ||
                            CastingSpell->EffectImplicitTargetA[2] == 1) //Prevents school based procs affecting caster when self buffing
                            continue;
                    }
                    else if( sp->SpellIconID == 1 )
                        continue;
                }
            }

            //Custom procchance modifications based on equipped weapon speed.
            if( IsPlayer() && ospinfo != NULL && ospinfo->ProcsPerMinute > 0.0f )
            {
                float ppm = ospinfo->ProcsPerMinute;

                Player* plr = TO_PLAYER( this );
                Item* weapon = NULLITEM;
                if(plr->GetItemInterface() && weapon_damage_type > 0 && weapon_damage_type < 3)
                    weapon = plr->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_MAINHAND + weapon_damage_type - 1 );
                if(weapon && weapon->GetProto())
                {
                    float speed = float( weapon->GetProto()->Delay );
                    proc_Chance = float2int32( speed * 0.001f * ppm / 0.6f );
                }

                if( plr->IsInFeralForm() )
                {
                    if( plr->GetShapeShift() == FORM_CAT )
                    {
                        proc_Chance = float2int32( ppm / 0.6f );
                    }
                    else if( plr->GetShapeShift() == FORM_BEAR || plr->GetShapeShift() == FORM_DIREBEAR )
                    {
                        proc_Chance = float2int32( ppm / 0.24f );
                    }
                }
            }

            //hack shit for different proc rates
            if( spellId == 40472 )
            {
                if( !CastingSpell )
                    continue;

                if( CastingSpell->NameHash == SPELL_HASH_FLASH_OF_LIGHT ||
                    CastingSpell->NameHash == SPELL_HASH_HOLY_LIGHT )
                {
                    spellId = 40471;
                    proc_Chance -= 35;
                }
                else if( CastingSpell->buffIndexType != SPELL_TYPE_INDEX_JUDGEMENT )
                    continue;
            }

            if(ospinfo != NULL)
                SM_FIValue( SM[SMT_PROC_CHANCE][0], (int32*)&proc_Chance, ospinfo->SpellGroupType );

            if( spellId && Rand( proc_Chance ) )
            {
                SpellCastTargets targets;
                if( itr2->procflags2 & PROC_TARGET_SELF )
                    targets.m_unitTarget = GetGUID();
                else if( victim != NULL )
                    targets.m_unitTarget = victim->GetGUID();

                //to be sure, check targeting
                if( spellId != 47930 )
                {
                    for(uint32 j = 0; j < 3; ++j)
                    {
                        if( sp->EffectImplicitTargetA[j] == 1 )
                        {
                            targets.m_unitTarget = GetGUID();
                            break;
                        }
                    }
                }

                /* hmm whats a reasonable value here */
                if( m_procCounter > 40 )
                {
                    /* something has proceed over 10 times in a loop :/ dump the spellids to the crashlog, as the crashdump will most likely be useless. */
                    // BURLEX FIX ME!
                    //OutputCrashLogLine("HandleProc %u SpellId %u (%s) %u", flag, spellId, sSpellStore.LookupString(sSpellStore.LookupEntry(spellId)->Name), m_procCounter);
                    m_procOverspill++;
                    return 0;
                }

                //check if we can trigger due to time limitation
                if( ospinfo != NULL && ospinfo->proc_interval )
                {
                    if( itr2->LastTrigger + ospinfo->proc_interval > mstimenow )
                        continue; //we can't trigger it yet.
                }

                //since we did not allow to remove auras like these with interrupt flag we have to remove them manually.
                if( itr2->procflags2 & PROC_REMOVEONUSE )
                {
                    Aura* aura = m_AuraInterface.FindActiveAura(origId);
                    if(aura != NULL)
                    {
                        aura->procCharges--;
                        if(aura->procCharges > 0)
                            aura->BuildAuraUpdate();
                        else
                            RemoveAura(aura);
                    }
                }

                int32 damage = itr2->procValue;
                if(!sScriptMgr.HandleScriptedProcLimits(this, spellId, damage, targets, &(*itr2), &procData))
                    continue;
                SpellEntry *spellInfo = dbcSpell.LookupEntry( spellId );
                if( victim == TO_UNIT( this ) && spellInfo->c_is_flags & SPELL_FLAG_CANNOT_PROC_ON_SELF )
                    continue;
                // consider it triggered
                itr2->LastTrigger = mstimenow;

                Spell* spell = new Spell(TO_UNIT( this ), spellInfo, true, NULLAURA);
                spell->forced_basepoints[0] = damage;
                spell->ProcedOnSpell = CastingSpell;
                spell->pSpellId = origId;
                spell->prepare(&targets);
            }//not always we have a spell to cast
        }
    }

    // Proc charges removal
    if( !m_chargeSpells.empty() )
    {
        m_chargeSpellsInUse=true;
        Aura* aura = NULLAURA;
        std::list<Aura*>::iterator iter;

        for(iter = m_chargeSpells.begin(); iter != m_chargeSpells.end(); iter++)
        {
            aura = *iter;

            if(aura && !aura->m_deleted && aura->procCharges > 0 && aura->m_spellProto && ((aura->m_spellProto->procFlags & flag) || (aura->m_spellProto->procflags2 & flag2)))
            {
                //Fixes for spells that dont lose charges when dmg is absorbed
                if((aura->m_spellProto->procFlags == 680) && dmg == 0)
                    continue;

                if(CastingSpell)
                {
                    SpellEntry * spe = aura->m_spellProto;
                    uint32 *SpellClassMask = spe->EffectSpellClassMask[0];

                    if (SpellClassMask && (SpellClassMask[0] || SpellClassMask[1] || SpellClassMask[2])) {
                        if (!Spell::EffectAffectsSpell(spe, 0, CastingSpell))
                            continue;
                    }
                    SpellCastTime *sd = dbcSpellCastTime.LookupEntry(CastingSpell->CastingTimeIndex);
                    if(!sd) continue; // this shouldnt happen though :P
                    //if we did not proc these then we should not remove them
                    if( CastingSpell->Id == spe->Id)
                        continue;
                    switch(spe->Id)
                    {
                    case 12043:
                        {
                            //Presence of Mind and Nature's Swiftness should only get removed
                            //when a non-instant and bellow 10 sec. Also must be nature :>
                            //                          if(!sd->CastTime||sd->CastTime>10000) continue;
                            if(sd->CastTime==0)
                                continue;
                        }break;
                    case 17116: //Shaman - Nature's Swiftness
                    case 16188:
                        {
                            //                          if( CastingSpell->School!=SCHOOL_NATURE||(!sd->CastTime||sd->CastTime>10000)) continue;
                            if( CastingSpell->School!=SCHOOL_NATURE||(sd->CastTime==0)) continue;
                        }break;
                    case 16166:
                        {
                            if(!(CastingSpell->School==SCHOOL_FIRE||CastingSpell->School==SCHOOL_FROST||CastingSpell->School==SCHOOL_NATURE))
                                continue;
                        }break;
                    case 55166: // Tidal Force
                        {
                            // Aura gets removed when last stack is removed
                            aura->ModStackSize(-1);
                            continue;
                        }break;
                    case 17364: // Stormstrike
                        {
                            if(victim->GetGUID() != aura->GetCasterGUID())
                                continue;   // charges spent only for particular caster
                        }break;
                    }
                    if(spe->NameHash == SPELL_HASH_FLURRY)
                        continue;  // only removed on auto attack swings not abilities
                }
                aura->ModProcCharges(-1);
            }
        }

        if(can_delete)
        {
            for(;!m_chargeSpellRemoveQueue.empty();)
            {
                iter = std::find(m_chargeSpells.begin(), m_chargeSpells.end(), m_chargeSpellRemoveQueue.front());
                if(iter != m_chargeSpells.end())
                {
                    m_chargeSpells.erase(iter);
                }
                m_chargeSpellRemoveQueue.pop_front();
            }
        }

        m_chargeSpellsInUse=false;
    }

    if(can_delete) //are we the upper level of nested procs ? If yes then we can remove the lock
        bProcInUse = false;

    if(m_procOverspill)
    {
        m_procCounter -= m_procOverspill;
        m_procOverspill = 0;
    }
    m_procCounter--;
    return resisted_dmg;
}

//damage shield is a triggered spell by owner to atacker
void Unit::HandleProcDmgShield(uint32 flag, Unit* attacker)
{
    //make sure we do not loop dmg procs
    if( !attacker || TO_UNIT(this) == attacker )
        return;

    if(m_damgeShieldsInUse)
        return;

    m_damgeShieldsInUse = true;
    //charges are already removed in handleproc
    WorldPacket data(24);
    std::list<DamageProc>::iterator i;
    std::list<DamageProc>::iterator i2;
    for(i = m_damageShields.begin();i != m_damageShields.end();)     // Deal Damage to Attacker
    {
        i2 = i; //we should not proc on proc.. not get here again.. not needed.Better safe then sorry.
        ++i;
        if( ((*i2).m_flags) & flag )
        {
            if( (*i2).destination )
            {
                uint32 overkill = attacker->computeOverkill((*i2).m_damage);
                data.Initialize(SMSG_SPELLDAMAGESHIELD);
                data << GetGUID();
                data << attacker->GetGUID();
                data << (*i2).m_spellId;
                data << (*i2).m_damage;
                data << uint32(overkill);
                data << SchoolMask((*i2).m_school);
                SendMessageToSet(&data,true);
                DealDamage(attacker,(*i2).m_damage,0,0,(*i2).m_spellId);
            }
            else
            {
                SpellEntry *ability = dbcSpell.LookupEntry((*i2).m_spellId);
                if(!ability)
                    continue;

                Strike( attacker, RANGED, ability, 0, 0, (*i2).m_damage, false, true );
            }
        }
    }
    m_damgeShieldsInUse = false;
}

bool Unit::isCasting()
{
    return (m_currentSpell != NULL);
}

bool Unit::IsInInstance()
{
    MapInfo *pUMapinfo = LimitedMapInfoStorage.LookupEntry(GetMapId());
    return (pUMapinfo == NULL ? false : (pUMapinfo->type != INSTANCE_NULL && pUMapinfo->type != INSTANCE_PVP));
}

void Unit::RegenerateHealth()
{
    m_H_regenTimer = 2000;//set next regen time

    if(!isAlive())
        return;

    // player regen
    if(IsPlayer())
    {
        m_H_regenTimer = 1000;//set next regen time to 1 for players.
        TO_PLAYER(this)->RegenerateHealth(CombatStatus.IsInCombat());
    }
    else
        TO_CREATURE(this)->RegenerateHealth(CombatStatus.IsInCombat());
}

void Unit::RegenerateEnergy()
{
    if( m_interruptRegen > 0 )
        return;

    uint32 cur = GetUInt32Value(UNIT_FIELD_POWER4);
    uint32 mp = GetUInt32Value(UNIT_FIELD_MAXPOWER4);
    if( cur >= mp )
        return;

    cur += float2int32(floor(float(0.01f * m_P_regenTimer * PctPowerRegenModifier[POWER_TYPE_ENERGY])));
    SetUInt32Value(UNIT_FIELD_POWER4, (cur >= mp) ? mp : cur);
}

void Unit::RegenerateFocus()
{
    if (m_interruptRegen)
        return;

    uint32 cur = GetUInt32Value(UNIT_FIELD_POWER3);
    uint32 mp = GetUInt32Value(UNIT_FIELD_MAXPOWER3);
    if( cur >= mp )
        return;

    cur += float2int32(floor(float(1.0f * PctPowerRegenModifier[POWER_TYPE_FOCUS])));
    SetUInt32Value(UNIT_FIELD_POWER3, (cur >= mp)? mp : cur);
}

void Unit::LosePower(uint32 powerField, int32 decayValue)
{
    if( m_interruptRegen > 0 )
        return;

    uint32 cur = GetUInt32Value(powerField);
    uint32 newpower = ((int)cur <= decayValue) ? 0 : cur-decayValue;
    if (newpower > 1000 )
        newpower = 1000;

    SetUInt32Value(powerField,newpower);
}

void Unit::RegeneratePower(bool isinterrupted)
{
    if(!isAlive())
        return;

    // player regen
    if(IsPlayer())
        TO_PLAYER(this)->PlayerRegeneratePower(isinterrupted);
    else
    {
        // This is only 100 IF the power is not rage
        m_P_regenTimer = 100; //set next regen time

        uint32 powertype = GetPowerType();
        switch(powertype)
        {
        case POWER_TYPE_MANA:
            TO_CREATURE(this)->RegenerateMana(isinterrupted);
            break;
        case POWER_TYPE_ENERGY:
            {
                if(!IsVehicle() || TO_VEHICLE(this)->GetVehiclePowerType() != POWER_TYPE_PYRITE)
                    RegenerateEnergy();
            }break;
        case POWER_TYPE_FOCUS:
            RegenerateFocus();
            break;
        case POWER_TYPE_RAGE:
            {
                m_P_regenTimer = 3000;
                if(!CombatStatus.IsInCombat())
                    LosePower(UNIT_FIELD_POWER2, 30);
            }break;
        case POWER_TYPE_RUNE:
            {
                m_P_regenTimer = 3000;
                if(!CombatStatus.IsInCombat())
                    LosePower(UNIT_FIELD_POWER6, 50);
                else
                    LosePower(UNIT_FIELD_POWER6, -100);
            }break;
        case POWER_TYPE_RUNIC:
            {
                if(!CombatStatus.IsInCombat())
                {
                    m_P_regenTimer = 800;
                    LosePower(UNIT_FIELD_POWER7, 10);
                }
            }break;
        }
    }
}

double Unit::GetResistanceReducion(Unit* pVictim, uint32 school, float armorReducePct)
{
    double reduction = 0.0;
    if(school == 0) // physical
    {
        float armor = pVictim->GetResistance(0) - PowerCostPctMod[0];
        if(IsPlayer()) // apply armor penetration
        {
            float maxArmorPen = 0;
            if (pVictim->getLevel() < 60)
                maxArmorPen = float(400 + 85 * pVictim->getLevel());
            else
                maxArmorPen = 400 + 85 * pVictim->getLevel() + 4.5f * 85 * (pVictim->getLevel() - 59);

            // Cap armor penetration to this number
            maxArmorPen = std::min((armor + maxArmorPen) / 3, armor);

            // Figure out how much armor we ignore
            float armorPen = armorReducePct+TO_PLAYER(this)->CalcRating(PLAYER_RATING_MODIFIER_ARMOR_PENETRATION_RATING);

            // Apply armor pen cap to our calculated armor penetration
            armor -= std::min(armorPen, maxArmorPen);
        }

        if (armor < 0.0f)
            armor = 0.0f;

        float levelModifier = getLevel();
        if (levelModifier > 59)
            levelModifier = levelModifier + (4.5f * (levelModifier - 59));
        reduction = 0.1f * armor / (8.5f * levelModifier + 40);
        reduction = reduction / (1.0f + reduction);
    }
    else 
    {   // non-physical
        float resistance = (float) pVictim->GetResistance(school);
        double RResist = resistance + float((pVictim->getLevel() > getLevel()) ? (pVictim->getLevel() - getLevel()) * 5 : 0) - PowerCostPctMod[school];
        reduction = RResist / (double)(getLevel() * 5) * 0.75f;
    }

    if(reduction > 0.75)
        reduction = 0.75;
    else if(reduction < 0)
        reduction = 0;
    return reduction;
}

void Unit::CalculateResistanceReduction(Unit* pVictim,dealdamage * dmg, SpellEntry* ability, float armorReducePct)
{
    if( (dmg->school_type && ability && Spell::IsBinary(ability)) || dmg->school_type == SCHOOL_HOLY )  // damage isn't reduced for binary spells
    {
        (*dmg).resisted_damage = 0;
        return;
    }

    double reduction = GetResistanceReducion(pVictim, dmg->school_type, armorReducePct);

    // only for physical or non binary spells
    if(reduction > 0)
        (*dmg).resisted_damage = (uint32)(((*dmg).full_damage)*reduction);
    else
        (*dmg).resisted_damage = 0;
}

uint32 roll_results[5] =
{
    SPELL_DID_HIT_MISS,
    SPELL_DID_HIT_DODGE,
    SPELL_DID_HIT_PARRY,
    SPELL_DID_HIT_BLOCK,
    SPELL_DID_HIT_SUCCESS
};

uint32 Unit::GetSpellDidHitResult( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability )
{
    Item* it = NULLITEM;

    float hitchance         = 0.0f;
    float dodge             = 0.0f;
    float parry             = 0.0f;
    float block             = 0.0f;

    float hitmodifier       = 0;
    int32 self_skill        = 0;
    int32 victim_skill      = 0;
    uint32 SubClassSkill    = SKILL_UNARMED;

    bool backAttack         = pVictim->isTargetInBack(this);
    uint32 vskill           = 0;

    //==========================================================================================
    //==============================Victim Skill Base Calculation===============================
    //==========================================================================================
    if( pVictim->IsPlayer() )
    {
        vskill = TO_PLAYER( pVictim )->_GetSkillLineCurrent( SKILL_DEFENSE );
        if( weapon_damage_type != RANGED )
        {
            if( !backAttack )
            {
//--------------------------------block chance----------------------------------------------
                if( !pVictim->disarmedShield )
                    block = pVictim->GetFloatValue(PLAYER_BLOCK_PERCENTAGE); //shield check already done in Update chances
//--------------------------------dodge chance----------------------------------------------
                if(pVictim->m_stunned<=0)
                {
                    dodge = pVictim->GetFloatValue( PLAYER_DODGE_PERCENTAGE );
                }
//--------------------------------parry chance----------------------------------------------
                if(pVictim->can_parry && !disarmed)
                {
                    parry = pVictim->GetFloatValue( PLAYER_PARRY_PERCENTAGE );
                }
            }
        }
        victim_skill = float2int32( vskill + TO_PLAYER( pVictim )->CalcRating( PLAYER_RATING_MODIFIER_DEFENCE ) );
    }
    //--------------------------------mob defensive chances-------------------------------------
    else if(pVictim->IsCreature())
    {
        Creature* c = TO_CREATURE(pVictim);

        if( weapon_damage_type != RANGED && pVictim->m_stunned <= 0)
            dodge = pVictim->GetUInt32Value(UNIT_FIELD_AGILITY) / 14.5f; // what is this value?

        victim_skill = pVictim->getLevel() * 5;
        if(c && c->GetCreatureInfo() && (c->GetCreatureInfo()->Rank == ELITE_WORLDBOSS || c->GetCreatureInfo()->Flags1 & CREATURE_FLAGS1_BOSS))
        {
            victim_skill = std::max(victim_skill,((int32)getLevel()+3)*5); //used max to avoid situation when lowlvl hits boss.
        }

        if( !backAttack )
        {
            if( c->b_has_shield && !pVictim->disarmedShield && pVictim->GetUInt32Value(UNIT_FIELD_STRENGTH))
                block = 5.0f+(victim_skill*0.04f)+(pVictim->GetUInt32Value(UNIT_FIELD_STRENGTH)/2);
            if(pVictim->can_parry && !disarmed) // VictimSkill*0.04 per point
                parry = (victim_skill*0.04f);
        }
    }
    //==========================================================================================
    //==============================Attacker Skill Base Calculation=============================
    //==========================================================================================
    if(IsPlayer())
    {
        self_skill = 0;
        Player* pr = TO_PLAYER(this);
        hitmodifier = pr->GetHitFromMeleeSpell();

        switch( weapon_damage_type )
        {
        case MELEE:   // melee main hand weapon
            it = disarmed ? NULLITEM : pr->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_MAINHAND );
            hitmodifier += pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_HIT );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_MAIN_HAND_SKILL ) );
            break;
        case OFFHAND: // melee offhand weapon (dualwield)
            it = disarmed ? NULLITEM : pr->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
            hitmodifier += pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_HIT );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_OFF_HAND_SKILL ) );
            break;
        case RANGED:  // ranged weapon
            it = disarmed ? NULLITEM : pr->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_RANGED );
            hitmodifier += pr->CalcRating( PLAYER_RATING_MODIFIER_RANGED_HIT );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_RANGED_SKILL ) );
            break;
        }

        // erm. some spells don't use ranged weapon skill but are still a ranged spell and use melee stats instead
        // i.e. hammer of wrath
        if( ability && (ability->NameHash == SPELL_HASH_HAMMER_OF_WRATH || ability->NameHash == SPELL_HASH_AVENGER_S_SHIELD) )
        {
            it = pr->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_MAINHAND );
            hitmodifier += pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_HIT );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_MAIN_HAND_SKILL ) );
        }

        if(it && it->GetProto())
            SubClassSkill = Item::GetSkillByProto(it->GetProto()->Class,it->GetProto()->SubClass);
        else
            SubClassSkill = SKILL_UNARMED;

        if( SubClassSkill == SKILL_FIST_WEAPONS )
            SubClassSkill = SKILL_UNARMED;

        //chances in feral form don't depend on weapon skill
        if(TO_PLAYER(this)->IsInFeralForm())
        {
            uint8 form = TO_PLAYER(this)->GetShapeShift();
            if(form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR)
            {
                SubClassSkill = SKILL_FERAL_COMBAT;
                // Adjust skill for Level * 5 for Feral Combat
                self_skill += pr->getLevel() * 5;
            }
        }

        self_skill += pr->_GetSkillLineCurrent(SubClassSkill);
    }
    else
    {
        self_skill = getLevel() * 5;
        if(m_objectTypeId == TYPEID_UNIT)
        {
            Creature* c = TO_CREATURE(this);
            if(c && c->GetCreatureInfo() && (c->GetCreatureInfo()->Rank == ELITE_WORLDBOSS || c->GetCreatureInfo()->Flags1 & CREATURE_FLAGS1_BOSS))
                self_skill = std::max(self_skill,((int32)pVictim->getLevel()+3)*5);//used max to avoid situation when lowlvl hits boss.
        }
    }
    //==========================================================================================
    //==============================Special Chances Base Calculation============================
    //==========================================================================================
    //<THE SHIT> to avoid Linux bug.
    float diffVcapped = (float)self_skill;
    if(int32(pVictim->getLevel()*5)>victim_skill)
        diffVcapped -=(float)victim_skill;
    else
        diffVcapped -=(float)(pVictim->getLevel()*5);

    float diffAcapped = (float)victim_skill;
    if(int32(getLevel()*5)>self_skill)
        diffAcapped -=(float)self_skill;
    else
        diffAcapped -=(float)(getLevel()*5);
    //<SHIT END>

    //--------------------------------by victim state-------------------------------------------
    if(pVictim->IsPlayer()&&pVictim->GetStandState()) //every not standing state is >0
        hitchance = 100.0f;

    //--------------------------------by damage type and by weapon type-------------------------
    if( weapon_damage_type == RANGED )
    {
        dodge = 0.0f;
        parry = 0.0f;
    }
    else if(IsPlayer())
    {
        it = TO_PLAYER(this)->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
        if( it != NULL && (it->GetProto()->InventoryType == INVTYPE_WEAPON ||
            it->GetProto()->InventoryType == INVTYPE_2HWEAPON) && !ability )//dualwield to-hit penalty
        {
            hitmodifier -= 19.0f;
        }
    }

    //--------------------------------by skill difference---------------------------------------
    float vsk = (float)self_skill - (float)victim_skill;
    if( dodge )
        dodge = std::max( 0.0f, dodge - vsk * 0.04f );
    if( parry )
        parry = std::max( 0.0f, parry - vsk * 0.04f );
    if( block )
        block = std::max( 0.0f, block - vsk * 0.04f );

    if( pVictim->IsPlayer() )
    {
        if( vsk > 0 )
            hitchance = std::max( hitchance, 95.0f + vsk * 0.02f);
        else
            hitchance = std::max( hitchance, 95.0f + vsk * 0.04f);
    }
    else
    {
        if(vsk >= -10 && vsk <= 10)
            hitchance = std::max( hitchance, 95.0f + vsk * 0.1f);
        else
            hitchance = std::max( hitchance, 93.0f + (vsk - 10.0f) * 0.4f);
    }
    hitchance += hitmodifier;

    if( ability && ability->SpellGroupType )
    {
        SM_FFValue( SM[SMT_HITCHANCE][0], &hitchance, ability->SpellGroupType );
        SM_PFValue( SM[SMT_HITCHANCE][1], &hitchance, ability->SpellGroupType );
    }

    // overpower nana
    if( ability != NULL && ability->Attributes & ATTRIBUTES_CANT_BE_DPB )
    {
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }

    //==========================================================================================
    //==============================One Roll Processing=========================================
    //==========================================================================================
    //--------------------------------cummulative chances generation----------------------------
    uint32 r = 0;
    float chances[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    chances[0] = std::max(0.0f, 100.0f-hitchance);
    if(!backAttack)
    {
        chances[1] = chances[0]+dodge;
        chances[2] = chances[1]+parry;
        chances[3] = chances[2]+block;
    }
    else if(pVictim->IsCreature())
        chances[1] = chances[0]+dodge;

    //--------------------------------roll------------------------------------------------------
    float Roll = RandomFloat(100.0f);
    while (r < 4 && Roll > chances[r])
        r++;
    return roll_results[r];
}

uint32 Unit::GetSpellDidHitResult( uint32 index, Unit* pVictim, Spell* pSpell, uint8&reflectout )
{
    //
    float baseresist[3] = { 4.0f, 5.0f, 6.0f };
    int32 lvldiff           = 0;
    float hitchance         = 0.0f;
    float resistchance      = 0.0f;
    SpellEntry* m_spellEntry = pSpell->GetSpellProto();
    lvldiff = pVictim->getLevel() - getLevel();
    if(lvldiff > 5) // Crow: This has to be capped, in retail level 50's hit level 80's in dungeons with spells a lot.
        lvldiff = 5;
    if (lvldiff < 3)
        hitchance = 96 - lvldiff;
    else
        hitchance = 94 - (lvldiff - 2) * (pVictim->IsPlayer() ? 7 : 11);

    if(m_spellEntry->SpellGroupType)
    {
        SM_FFValue(SM[SMT_HITCHANCE][0], &hitchance, m_spellEntry->SpellGroupType);
        SM_PFValue(SM[SMT_HITCHANCE][1], &hitchance, m_spellEntry->SpellGroupType);
    }

    //rating bonus
    if( IsPlayer() )
    {
        hitchance += TO_PLAYER(this)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_HIT );
        hitchance += TO_PLAYER(this)->GetHitFromSpell();
    }

    if(pVictim->IsPlayer())
        hitchance += TO_PLAYER(pVictim)->m_resist_hit[2];

    // 160: Mod AOE avoidance implementation needed.

    if(!Rand(hitchance))
        return SPELL_DID_HIT_MISS;

    /************************************************************************/
    /* Check if the spell is resisted.                                    */
    /************************************************************************/
    if( m_spellEntry->School == SCHOOL_NORMAL  || m_spellEntry->is_ranged_spell ) // all ranged spells are physical too...
        return SPELL_DID_HIT_SUCCESS;
    if( m_spellEntry->Flags4 & FLAGS4_IGNORE_HIT_RESULT )
        return SPELL_DID_HIT_SUCCESS;

    resistchance = 100.0f-hitchance;
    if (m_spellEntry->DispelType < NUM_DISPELS)
        resistchance += pVictim->DispelResistancesPCT[m_spellEntry->DispelType];

    // Our resist to dispel
    if( m_spellEntry->Effect[index] == SPELL_EFFECT_DISPEL && m_spellEntry->SpellGroupType)
    {
        SM_FFValue(pVictim->SM[SMT_RESIST_DISPEL][0], &resistchance, m_spellEntry->SpellGroupType);
        SM_PFValue(pVictim->SM[SMT_RESIST_DISPEL][1], &resistchance, m_spellEntry->SpellGroupType);
    }

    if(Spell::IsBinary(m_spellEntry))
    { // need to apply resistance mitigation
        float mitigation = 1.0f - float (GetResistanceReducion(pVictim, m_spellEntry->School, 0.0f));
        resistchance = 100 - (100 - resistchance) * mitigation; // meaning hitchance * mitigation
    }

    if(resistchance < 1.0f)
        resistchance = 1.0f;

    if(resistchance > 99.0f)
        resistchance = 99.0f;

    if (m_spellEntry->Attributes & ATTRIBUTES_IGNORE_INVULNERABILITY)
        resistchance = 0.0f;

    if( m_spellEntry->c_is_flags & SPELL_FLAG_IS_NOT_RESISTABLE )
        resistchance = 0.0f;

    uint32 res = Rand(resistchance) ? SPELL_DID_HIT_RESIST : SPELL_DID_HIT_SUCCESS;
    if(res == SPELL_DID_HIT_SUCCESS) // spell reflect handler
        if(!(reflectout = (pSpell->Reflect(pVictim) == true ? SPELL_DID_HIT_SUCCESS : SPELL_DID_HIT_MISS)))
            res = SPELL_DID_HIT_REFLECT;

    return res;
}

int32 Unit::Strike( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability, int32 add_damage, int32 pct_dmg_mod, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool proc_extrastrike )
{
//==========================================================================================
//==============================Unacceptable Cases Processing===============================
//==========================================================================================
    if(!pVictim->isAlive() || !isAlive()  || IsStunned() || IsPacified() || IsFeared())
        return 0;

    if(!isTargetInFront(pVictim))
    {
        if(IsPlayer())
        {
            if( !(ability && ability->AllowBackAttack) )
            {
                TO_PLAYER(this)->GetSession()->OutPacket(SMSG_ATTACKSWING_BADFACING);
                return 0;
            }
        }
    }
//==========================================================================================
//==============================Variables Initialization====================================
//==========================================================================================
    dealdamage dmg            = {0,0,0};

    Item* it = NULLITEM;

    float armorreducepct =  m_ignoreArmorPct;

    float hitchance         = 0.0f;
    float dodge             = 0.0f;
    float parry             = 0.0f;
    float glanc             = 0.0f;
    float block             = 0.0f;
    float crit              = 0.0f;
    float crush             = 0.0f;

    uint32 targetEvent      = 0;
    uint32 hit_status       = 0;

    uint32 blocked_damage   = 0;
    int32  realdamage       = 0;

    uint32 vstate           = 1;
    uint32 aproc            = 0;
    uint32 aproc2           = 0;
    uint32 vproc            = 0;
    uint32 vproc2           = 0;

    float hitmodifier       = 0;
    int32 self_skill;
    int32 victim_skill;
    uint32 SubClassSkill    = SKILL_UNARMED;

    bool backAttack         = pVictim->isTargetInBack(this);
    uint32 vskill           = 0;
    bool disable_dR         = false;

    if(ability)
        dmg.school_type = ability->School;
    else
    {
        if (GetTypeId() == TYPEID_UNIT)
            dmg.school_type = TO_CREATURE(this)->BaseAttackType;
        else
            dmg.school_type = SCHOOL_NORMAL;
    }

//==========================================================================================
//==============================Victim Skill Base Calculation===============================
//==========================================================================================
    if(pVictim->IsPlayer())
    {
        vskill = TO_PLAYER( pVictim )->_GetSkillLineCurrent( SKILL_DEFENSE );
        if( !backAttack )
        {
            if( weapon_damage_type != RANGED )
            {
//--------------------------------block chance----------------------------------------------
                if( !pVictim->disarmedShield )
                    block = pVictim->GetFloatValue(PLAYER_BLOCK_PERCENTAGE); //shield check already done in Update chances
//--------------------------------dodge chance----------------------------------------------
                if(pVictim->m_stunned<=0)
                {
                    dodge = pVictim->GetFloatValue( PLAYER_DODGE_PERCENTAGE );
                }
//--------------------------------parry chance----------------------------------------------
                if(pVictim->can_parry && !disarmed)
                {
                    parry = pVictim->GetFloatValue( PLAYER_PARRY_PERCENTAGE );
                }
            }
        }
        victim_skill = float2int32( vskill + TO_PLAYER( pVictim )->CalcRating( 1 ) );
    }
//--------------------------------mob defensive chances-------------------------------------
    else
    {
        if( weapon_damage_type != RANGED )
            dodge = pVictim->GetUInt32Value( UNIT_FIELD_STAT1 ) / 14.5f; // what is this value? (Agility)

        victim_skill = pVictim->getLevel() * 5;
        if( pVictim->m_objectTypeId == TYPEID_UNIT )
        {
            Creature* c = TO_CREATURE( pVictim );
            if( c != NULL && c->GetCreatureInfo() && (c->GetCreatureInfo()->Rank == ELITE_WORLDBOSS || c->GetCreatureInfo()->Flags1 & CREATURE_FLAGS1_BOSS) )
            {
                victim_skill = std::max( victim_skill, ( (int32)getLevel() + 3 ) * 5 ); //used max to avoid situation when lowlvl hits boss.
            }
        }
    }
//==========================================================================================
//==============================Attacker Skill Base Calculation=============================
//==========================================================================================
    if( IsPlayer() )
    {
        self_skill = 0;
        Player* pr = TO_PLAYER(this);
        hitmodifier = pr->GetHitFromMeleeSpell();

        switch( weapon_damage_type )
        {
        case MELEE:   // melee main hand weapon
            it = disarmed ? NULLITEM : pr->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_MAINHAND );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_MAIN_HAND_SKILL ) );
            if (it && it->GetProto())
            {
                dmg.school_type = it->GetProto()->DamageType;
                if( it->GetProto()->SubClass == ITEM_SUBCLASS_WEAPON_MACE )
                    armorreducepct += m_ignoreArmorPctMaceSpec;
            }
            break;
        case OFFHAND: // melee offhand weapon (dualwield)
            it = disarmed ? NULLITEM : pr->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_MELEE_OFF_HAND_SKILL ) );
            hit_status |= HITSTATUS_DUALWIELD;//animation
            if (it && it->GetProto())
            {
                dmg.school_type = it->GetProto()->DamageType;
                if( it->GetProto()->SubClass == ITEM_SUBCLASS_WEAPON_MACE )
                    armorreducepct += m_ignoreArmorPctMaceSpec;
            }
            break;
        case RANGED:  // ranged weapon
            it = disarmed ? NULLITEM : pr->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_RANGED );
            self_skill = float2int32( pr->CalcRating( PLAYER_RATING_MODIFIER_RANGED_SKILL ) );
            if (it && it->GetProto())
                dmg.school_type = it->GetProto()->DamageType;
            break;
        }

        if(it && it->GetProto())
        {
            SubClassSkill = Item::GetSkillByProto(it->GetProto()->Class,it->GetProto()->SubClass);
            if(SubClassSkill==SKILL_FIST_WEAPONS)
                SubClassSkill = SKILL_UNARMED;
        }
        else
            SubClassSkill = SKILL_UNARMED;


        //chances in feral form don't depend on weapon skill
        if(pr->IsInFeralForm())
        {
            uint8 form = pr->GetShapeShift();
            if(form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR)
            {
                SubClassSkill = SKILL_FERAL_COMBAT;
                self_skill += pr->getLevel() * 5;
            }
        }

        self_skill += pr->_GetSkillLineCurrent(SubClassSkill);
        crit = GetFloatValue(PLAYER_CRIT_PERCENTAGE);
    }
    else
    {
        self_skill = getLevel() * 5;
        if(m_objectTypeId == TYPEID_UNIT)
        {
            Creature* c = TO_CREATURE(this);
            if(c && c->GetCreatureInfo() && (c->GetCreatureInfo()->Rank == ELITE_WORLDBOSS || c->GetCreatureInfo()->Flags1 & CREATURE_FLAGS1_BOSS))
                self_skill = std::max(self_skill,((int32)pVictim->getLevel()+3)*5);//used max to avoid situation when lowlvl hits boss.
        }
        crit = 5.0f; //will be modified later
    }

//==========================================================================================
//==============================Special Chances Base Calculation============================
//==========================================================================================
    //<THE SHIT> to avoid Linux bug.
    float diffVcapped = (float)self_skill;
    if(int32(pVictim->getLevel()*5)>victim_skill)
        diffVcapped -=(float)victim_skill;
    else
        diffVcapped -=(float)(pVictim->getLevel()*5);

    float diffAcapped = (float)victim_skill;
    if(int32(getLevel()*5)>self_skill)
        diffAcapped -=(float)self_skill;
    else
        diffAcapped -=(float)(getLevel()*5);
    //<SHIT END>

//--------------------------------crushing blow chance--------------------------------------
    if(pVictim->IsPlayer()&&!IsPlayer()&&!ability && !dmg.school_type)
    {
        int32 LevelDiff = GetUInt32Value(UNIT_FIELD_LEVEL) - pVictim->GetUInt32Value(UNIT_FIELD_LEVEL);
        if(diffVcapped>=15.0f && LevelDiff >= 4)
            crush = -15.0f+2.0f*diffVcapped;
        else
            crush = 0.0f;
    }
//--------------------------------glancing blow chance--------------------------------------
    if(IsPlayer()&&!pVictim->IsPlayer()&&!ability)
    {
        glanc = 10.0f + diffAcapped;
        if(glanc<0)
            glanc = 0.0f;
    }
//==========================================================================================
//==============================Advanced Chances Modifications==============================
//==========================================================================================
//--------------------------------by talents------------------------------------------------
    if(pVictim->IsPlayer())
    {
        if( weapon_damage_type != RANGED )
        {
            crit += TO_PLAYER(pVictim)->res_M_crit_get();
            hitmodifier += TO_PLAYER(pVictim)->m_resist_hit[0];
        }
        else
        {
            crit += TO_PLAYER(pVictim)->res_R_crit_get(); //this could be ability but in that case we overwrite the value
            hitmodifier += TO_PLAYER(pVictim)->m_resist_hit[1];
        }
    }
    crit += (float)(pVictim->AttackerCritChanceMod[0]);
//--------------------------------by skill difference---------------------------------------
    float vsk = (float)self_skill - (float)victim_skill;
    dodge = std::max( 0.0f, dodge - vsk * 0.04f );
    if( parry )
        parry = std::max( 0.0f, parry - vsk * 0.04f );
    if( block )
        block = std::max( 0.0f, block - vsk * 0.04f );

    crit += pVictim->IsPlayer() ? vsk * 0.04f : min( vsk * 0.2f, 0.0f );

    if( pVictim->IsPlayer() )
    {
        if( vsk > 0 )
            hitchance = std::max( hitchance, 95.0f + vsk * 0.02f);
        else
            hitchance = std::max( hitchance, 95.0f + vsk * 0.04f);
    } else
    {
        if(vsk >= -10 && vsk <= 10)
            hitchance = std::max( hitchance, 95.0f + vsk * 0.1f);
        else
            hitchance = std::max( hitchance, 93.0f + (vsk - 10.0f) * 0.4f);
    }
//--------------------------------by ratings------------------------------------------------
    crit -= pVictim->IsPlayer() ? TO_PLAYER(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE ) : 0.0f;
    if(crit < 0.0f)
        crit = 0.0f;

    if (IsPlayer())
    {
        if(weapon_damage_type == RANGED)
            hitmodifier += TO_PLAYER(this)->CalcRating( PLAYER_RATING_MODIFIER_RANGED_HIT );
        else
        {
            hitmodifier += TO_PLAYER(this)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_HIT );

            float expertise_bonus = 0.0f;
            if(weapon_damage_type == MELEE)
                expertise_bonus += GetUInt32Value(PLAYER_EXPERTISE)*0.25f;
            else if(weapon_damage_type == OFFHAND)
                expertise_bonus += GetUInt32Value(PLAYER_OFFHAND_EXPERTISE)*0.25f;

            if(dodge < expertise_bonus)
                dodge = 0.0f;
            else dodge -= expertise_bonus;

            if(parry < expertise_bonus)
                parry = 0.0f;
            else parry -= expertise_bonus;
        }
    }

//--------------------------------by damage type and by weapon type-------------------------
    if( weapon_damage_type == RANGED )
    {
        dodge = 0.0f;
        parry = 0.0f;
        glanc = 0.0f;
    }
    else if(IsPlayer())
    {
        it = TO_PLAYER(this)->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
        if( it != NULL && (it->GetProto()->InventoryType == INVTYPE_WEAPON ||
            it->GetProto()->InventoryType == INVTYPE_2HWEAPON) && !ability )//dualwield to-hit penalty
        {
            hitmodifier -= 19.0f;
        }
    }

    hitchance += hitmodifier;

    if(ability && ability->SpellGroupType)
    {
        SM_FFValue(SM[SMT_CRITICAL][0],&crit,ability->SpellGroupType);
        SM_PFValue(SM[SMT_CRITICAL][1],&crit,ability->SpellGroupType);
        SM_FFValue(SM[SMT_HITCHANCE][0],&hitchance,ability->SpellGroupType);
        SM_PFValue(SM[SMT_HITCHANCE][1],&hitchance,ability->SpellGroupType);
    }

    //Hackfix for Surprise Attacks
    if( IsPlayer() && ability && TO_PLAYER(this)->m_finishingmovesdodge && ability->c_is_flags & SPELL_FLAG_IS_FINISHING_MOVE)
        dodge = 0.0f;

    if( skip_hit_check )
    {
        hitchance = 100.0f;
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }

    if( ability != NULL )
    {
        if( ability->NameHash == SPELL_HASH_FEROCIOUS_BITE && HasDummyAura( SPELL_HASH_REND_AND_TEAR ) && pVictim->m_AuraInterface.HasNegAuraWithMechanic(MECHANIC_BLEEDING) )
            crit += 5.0f * GetDummyAura(SPELL_HASH_REND_AND_TEAR )->RankNumber;
        else if( pVictim->IsStunned() && ability->Id == 20467 )
            crit = 100.0f;
        else if( ability->Attributes & ATTRIBUTES_CANT_BE_DPB )
        {
            dodge = 0.0f;
            parry = 0.0f;
            block = 0.0f;
        }
    }

//--------------------------------by victim state-------------------------------------------
    if(pVictim->IsPlayer() && pVictim->GetStandState()) //every not standing state is >0
    {
        hitchance = 100.0f;
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
        crush = 0.0f;
        crit = 100.0f;
    }
    else if(backAttack)
    {
        if(pVictim->IsPlayer())
            dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }
//==========================================================================================
//==============================One Roll Processing=========================================
//==========================================================================================
//--------------------------------cummulative chances generation----------------------------
    float chances[7];
    chances[0]=std::max(0.0f,100.0f-hitchance);
    chances[1]=chances[0]+dodge;
    chances[2]=chances[1]+parry;
    chances[3]=chances[2]+glanc;
    chances[4]=chances[3]+block;
    chances[5]=chances[4]+crit;
    chances[6]=chances[5]+crush;
//--------------------------------roll------------------------------------------------------
    float Roll = RandomFloat(100.0f);
    uint32 r = 0;
    while (r<7&&Roll>chances[r])
    {
        r++;
    }
//--------------------------------postroll processing---------------------------------------
    uint32 abs = 0;

    switch(r)
    {
//--------------------------------miss------------------------------------------------------
    case 0:
        hit_status |= HITSTATUS_MISS;
        // dirty ai agro fix
        if(pVictim->GetTypeId() == TYPEID_UNIT && pVictim->GetAIInterface()->GetNextTarget() == NULL)
            pVictim->GetAIInterface()->AttackReaction(TO_UNIT(this), 1, 0);
        break;
//--------------------------------dodge-----------------------------------------------------
    case 1:
        // dirty ai agro fix
        if(pVictim->GetTypeId() == TYPEID_UNIT && pVictim->GetAIInterface()->GetNextTarget() == NULL)
            pVictim->GetAIInterface()->AttackReaction(TO_UNIT(this), 1, 0);

        CALL_SCRIPT_EVENT(pVictim, OnTargetDodged)(TO_UNIT(this));
        CALL_SCRIPT_EVENT(TO_UNIT(this), OnDodged)(TO_UNIT(this));
        targetEvent = 1;
        vstate = DODGE;
        vproc2 |= PROC_ON_DODGE_VICTIM;
        pVictim->Emote(EMOTE_ONESHOT_PARRYUNARMED);         // Animation
        //allmighty warrior overpower

        if( IsPlayer() )
        {
            Player* plr = TO_PLAYER( this );
            if( plr->getClass() == WARRIOR  )
            {
                plr->AddComboPoints( pVictim->GetGUID(), 1 );
                plr->UpdateComboPoints();
                if( !sEventMgr.HasEvent( plr, EVENT_COMBO_POINT_CLEAR_FOR_TARGET ) )
                    sEventMgr.AddEvent( plr, &Player::NullComboPoints, (uint32)EVENT_COMBO_POINT_CLEAR_FOR_TARGET, (uint32)5000, (uint32)1, (uint32)0 );
                else
                    sEventMgr.ModifyEventTimeLeft( plr, EVENT_COMBO_POINT_CLEAR_FOR_TARGET, 5000 ,0 );
            }
            else if( plr->getClass() == DEATHKNIGHT )
            {
                CastSpell(GetGUID(), 56817, true);  // client side aura enabling Rune Strike
            }
        }
        if( pVictim->IsPlayer() )
        {
            Player* vplr = TO_PLAYER( pVictim );
            if( vplr->getClass() == DRUID )
            {
                if( (vplr->GetShapeShift() == FORM_BEAR ||
                     vplr->GetShapeShift() == FORM_DIREBEAR) &&
                     vplr->HasDummyAura(SPELL_HASH_NATURAL_REACTION) )
                {
                    switch(vplr->GetDummyAura(SPELL_HASH_NATURAL_REACTION)->RankNumber)
                    {
                    case 1: vplr->CastSpell(vplr, 57893, true); break;
                    case 2: vplr->CastSpell(vplr, 59071, true); break;
                    case 3: vplr->CastSpell(vplr, 59072, true); break;
                    }
                }
            }
            else if( vplr->getClass() == ROGUE )
            {
                if( vplr->HasDummyAura(SPELL_HASH_SETUP) )
                {
                    if( Rand((vplr->GetDummyAura(SPELL_HASH_SETUP)->RankNumber * 33) + 1) )
                        vplr->CastSpell(vplr, 15250, true);
                }
            }
        }

        pVictim->SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_DODGE_BLOCK);  //SB@L: Enables spells requiring dodge
        if(!sEventMgr.HasEvent(pVictim,EVENT_DODGE_BLOCK_FLAG_EXPIRE))
            sEventMgr.AddEvent(pVictim,&Unit::EventAurastateExpire,(uint32)AURASTATE_FLAG_DODGE_BLOCK,EVENT_DODGE_BLOCK_FLAG_EXPIRE,5000,1,0);
        else sEventMgr.ModifyEventTimeLeft(pVictim,EVENT_DODGE_BLOCK_FLAG_EXPIRE,5000,0);
        break;
//--------------------------------parry-----------------------------------------------------
    case 2:
        // dirty ai agro fix
        if(pVictim->GetTypeId() == TYPEID_UNIT && pVictim->GetAIInterface()->GetNextTarget() == NULL)
            pVictim->GetAIInterface()->AttackReaction(TO_UNIT(this), 1, 0);

        CALL_SCRIPT_EVENT(pVictim, OnTargetParried)(TO_UNIT(this));
        CALL_SCRIPT_EVENT(TO_UNIT(this), OnParried)(TO_UNIT(this));
        targetEvent = 3;
        vstate = PARRY;
        pVictim->Emote(EMOTE_ONESHOT_PARRYUNARMED);         // Animation
        if(pVictim->IsPlayer())
        {
            if( TO_PLAYER( pVictim )->getClass() == 1 || TO_PLAYER( pVictim )->getClass() == 4 )//warriors for 'revenge' and rogues for 'riposte'
            {
                //pVictim->SetFlag( UNIT_FIELD_AURASTATE,AURASTATE_FLAG_DODGE_BLOCK );  //SB@L: Enables spells requiring dodge
                if(!sEventMgr.HasEvent( pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE ) )
                    sEventMgr.AddEvent( pVictim, &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_DODGE_BLOCK, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0 );
                else
                    sEventMgr.ModifyEventTimeLeft( pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000 );
            }

            pVictim->SetFlag( UNIT_FIELD_AURASTATE,AURASTATE_FLAG_PARRY );  //SB@L: Enables spells requiring parry
            if(!sEventMgr.HasEvent( pVictim, EVENT_PARRY_FLAG_EXPIRE ) )
                sEventMgr.AddEvent( pVictim, &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_PARRY,EVENT_PARRY_FLAG_EXPIRE, 5000, 1, 0 );
            else
                sEventMgr.ModifyEventTimeLeft( pVictim, EVENT_PARRY_FLAG_EXPIRE, 5000 );
        }
        break;
//--------------------------------not miss,dodge or parry-----------------------------------
    default:
        hit_status |= HITSTATUS_HITANIMATION;//hit animation on victim
        if( pVictim->SchoolImmunityList[0] )
            vstate = IMMUNE;
        else
        {
//--------------------------------state proc initialization---------------------------------
            vproc |= PROC_ON_ANY_DAMAGE_VICTIM;
            if( weapon_damage_type != RANGED )
            {
                aproc |= PROC_ON_MELEE_ATTACK;
                vproc |= PROC_ON_MELEE_ATTACK_VICTIM;
            }
            else
            {
                aproc |= PROC_ON_RANGED_ATTACK;
                vproc |= PROC_ON_RANGED_ATTACK_VICTIM;
                if( ability && ability->Id == 3018 && IsPlayer() && getClass() == HUNTER )
                    aproc |= PROC_ON_AUTO_SHOT_HIT;
            }
//--------------------------------base damage calculation-----------------------------------
            if(exclusive_damage)
                dmg.full_damage = exclusive_damage;
            else
                dmg.full_damage = CalculateDamage( TO_UNIT(this), pVictim, weapon_damage_type, ability );

            if( weapon_damage_type == RANGED )
            {
                dmg.full_damage += pVictim->RangedDamageTaken;
            }

            if( ability && ability->MechanicsType == MECHANIC_BLEEDING )
                disable_dR = true;

            if( pct_dmg_mod > 0 )
                dmg.full_damage = float2int32( dmg.full_damage *  ( float( pct_dmg_mod) / 100.0f ) );

            dmg.full_damage += add_damage;

            // Bonus damage
            if( ability )
                dmg.full_damage = GetSpellBonusDamage(pVictim, ability, dmg.full_damage, false);
            else
            {
                dmg.full_damage += GetDamageDoneMod(SCHOOL_NORMAL);
                dmg.full_damage *= pVictim->DamageTakenPctMod[SCHOOL_NORMAL];
            }

            //pet happiness state dmg modifier
            if( IsPet() && !TO_PET(this)->IsSummonedPet() )
                dmg.full_damage = ( dmg.full_damage <= 0 ) ? 0 : float2int32( dmg.full_damage * TO_PET(this)->GetHappinessDmgMod() );

            if( HasDummyAura(SPELL_HASH_REND_AND_TEAR) && ability &&
                ( ability->NameHash == SPELL_HASH_MAUL || ability->NameHash == SPELL_HASH_SHRED) && pVictim->m_AuraInterface.HasNegAuraWithMechanic(MECHANIC_BLEEDING) )
            {
                dmg.full_damage += float2int32(dmg.full_damage * ( ( GetDummyAura(SPELL_HASH_REND_AND_TEAR)->RankNumber * 4 ) / 100.f ) );
            }


            if(dmg.full_damage < 0)
                dmg.full_damage = 0;
//--------------------------------check for special hits------------------------------------
            switch(r)
            {
//--------------------------------glancing blow---------------------------------------------
            case 3:
                {
                    float low_dmg_mod = 1.5f - (0.05f * diffAcapped);
                    if(getClass() == MAGE || getClass() == PRIEST || getClass() == WARLOCK) //casters = additional penalty.
                    {
                        low_dmg_mod -= 0.7f;
                    }
                    if(low_dmg_mod<0.01)
                        low_dmg_mod = 0.01f;
                    if(low_dmg_mod>0.91)
                        low_dmg_mod = 0.91f;
                    float high_dmg_mod = 1.2f - (0.03f * diffAcapped);
                    if(getClass() == MAGE || getClass() == PRIEST || getClass() == WARLOCK) //casters = additional penalty.
                    {
                        high_dmg_mod -= 0.3f;
                    }
                    if(high_dmg_mod>0.99)
                        high_dmg_mod = 0.99f;
                    if(high_dmg_mod<0.2)
                        high_dmg_mod = 0.2f;

                    float damage_reduction = (high_dmg_mod + low_dmg_mod) / 2.0f;
                    if(damage_reduction > 0)
                    {
                            dmg.full_damage = float2int32(damage_reduction * float(dmg.full_damage));
                    }
                    hit_status |= HITSTATUS_GLANCING;
                }
                break;
//--------------------------------block-----------------------------------------------------
            case 4:
                {
                    Item* shield = TO_PLAYER( pVictim )->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if( shield != NULL && !pVictim->disarmedShield )
                    {
                        targetEvent = 2;
                        pVictim->Emote(EMOTE_ONESHOT_PARRYSHIELD);// Animation

                        if( shield->GetProto()->InventoryType == INVTYPE_SHIELD )
                        {
                            float block_multiplier = ( 100.0f + float( TO_PLAYER( pVictim )->m_modblockabsorbvalue ) ) / 100.0f;
                            if( block_multiplier < 1.0f )block_multiplier = 1.0f;

                            blocked_damage = pVictim->GetUInt32Value(PLAYER_SHIELD_BLOCK);
                            hit_status |= HITSTATUS_BLOCK;
                        }
                        else
                        {
                            blocked_damage = 0;
                        }

                        if(dmg.full_damage <= (int32)blocked_damage)
                        {
                            CALL_SCRIPT_EVENT(pVictim, OnTargetBlocked)(TO_UNIT(this), blocked_damage);
                            CALL_SCRIPT_EVENT(TO_UNIT(this), OnBlocked)(pVictim, blocked_damage);
                            vstate = BLOCK;
                            vproc2 |= PROC_ON_BLOCK_VICTIM;
                        }
                        if( pVictim->IsPlayer() )//not necessary now but we'll have blocking mobs in future
                        {
                            pVictim->SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_DODGE_BLOCK);  //SB@L: Enables spells requiring dodge
                            if(!sEventMgr.HasEvent(pVictim,EVENT_DODGE_BLOCK_FLAG_EXPIRE))
                                sEventMgr.AddEvent(pVictim,&Unit::EventAurastateExpire,(uint32)AURASTATE_FLAG_DODGE_BLOCK,EVENT_DODGE_BLOCK_FLAG_EXPIRE,5000,1,0);
                            else
                                sEventMgr.ModifyEventTimeLeft(pVictim,EVENT_DODGE_BLOCK_FLAG_EXPIRE,5000);
                        }

                        // Holy shield fix
                        pVictim->HandleProcDmgShield(PROC_ON_BLOCK_VICTIM, TO_UNIT(this));
                    }
                }
                break;
//--------------------------------critical hit----------------------------------------------
            case 5:
                {
                    hit_status |= HITSTATUS_CRICTICAL;
                    float dmg_bonus_pct = 100.0f;
                    if(ability && ability->SpellGroupType)
                        SM_FFValue(SM[SMT_CRITICAL_DAMAGE][1],&dmg_bonus_pct,ability->SpellGroupType);

                    if( IsPlayer() )
                    {
                        // m_modphyscritdmgPCT
                        if( weapon_damage_type != RANGED )
                        {
                            dmg_bonus_pct += (float)TO_PLAYER( this )->m_modphyscritdmgPCT;
                        }

                        // IncreaseCriticalByTypePct
                        if( !pVictim->IsPlayer() )
                        {
                            CreatureInfo *pCreatureName = TO_CREATURE(pVictim)->GetCreatureInfo();
                            if( pCreatureName != NULL )
                                dmg_bonus_pct += TO_PLAYER( this )->IncreaseCricticalByTypePCT[pCreatureName->Type];
                        }

                        // UGLY GOUGE HAX
                        // too lazy to fix this properly...
                        if( this && IsPlayer() && TO_PLAYER(this)->HasDummyAura(SPELL_HASH_SEAL_FATE) && ability && ( ability->NameHash == SPELL_HASH_GOUGE || ability->NameHash == SPELL_HASH_MUTILATE ) )
                        {
                            if( Rand( TO_PLAYER(this)->GetDummyAura(SPELL_HASH_SEAL_FATE)->RankNumber * 20.0f ) )
                                TO_PLAYER(this)->AddComboPoints(pVictim->GetGUID(), 1);
                        }
                        else if( HasDummyAura( SPELL_HASH_PREY_ON_THE_WEAK ) )
                        {
                            if( pVictim->GetHealthPct() < GetHealthPct() )
                                dmg.full_damage += dmg.full_damage * ((GetDummyAura(SPELL_HASH_PREY_ON_THE_WEAK)->RankNumber * 4) / 100);
                        }

                        SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_CRITICAL);  //SB@L: Enables spells requiring critical strike
                        if(!sEventMgr.HasEvent( this ,EVENT_CRIT_FLAG_EXPIRE))
                            sEventMgr.AddEvent( TO_UNIT( this ),&Unit::EventAurastateExpire,(uint32)AURASTATE_FLAG_CRITICAL,EVENT_CRIT_FLAG_EXPIRE,5000,1,0);
                        else
                            sEventMgr.ModifyEventTimeLeft( this ,EVENT_CRIT_FLAG_EXPIRE,5000);
                    }

                    // SpellAuraReduceCritRangedAttackDmg
                    if( weapon_damage_type == RANGED )
                        dmg_bonus_pct -= CritRangedDamageTakenPctMod[dmg.school_type];
                    else
                        dmg_bonus_pct -= CritMeleeDamageTakenPctMod[dmg.school_type];

                    // actual crit damage?
                    if( dmg_bonus_pct > 0 )
                        dmg.full_damage += float2int32( float(dmg.full_damage) * (dmg_bonus_pct / 100.0f));

                    if(pVictim->IsPlayer())
                    {
                        //Resilience is a special new rating which was created to reduce the effects of critical hits against your character.
                        float dmg_reduction_pct = 2.2f * TO_PLAYER(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE ) / 100.0f;
                        if( dmg_reduction_pct > 0.33f )
                            dmg_reduction_pct = 0.33f; // 3.0.3
                        dmg.full_damage = float2int32( dmg.full_damage - dmg.full_damage*dmg_reduction_pct );
                    }

                    //emote
                    pVictim->Emote(EMOTE_ONESHOT_WOUNDCRITICAL);
                    vproc |= PROC_ON_CRIT_HIT_VICTIM;
                    aproc |= PROC_ON_CRIT_ATTACK;
                    if( weapon_damage_type == RANGED )
                    {
                        vproc |= PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
                        aproc |= PROC_ON_RANGED_CRIT_ATTACK;
                    }


                    CALL_SCRIPT_EVENT(pVictim, OnTargetCritHit)(TO_UNIT(this), float(dmg.full_damage));
                    CALL_SCRIPT_EVENT(TO_UNIT(this), OnCritHit)(pVictim, float(dmg.full_damage));
                }
                break;
//--------------------------------crushing blow---------------------------------------------
            case 6:
                hit_status |= HITSTATUS_CRUSHINGBLOW;
                dmg.full_damage = (dmg.full_damage * 3) >> 1;
                break;
//--------------------------------regular hit-----------------------------------------------
            default:
                break;
            }
//==========================================================================================
//==============================Post Roll Damage Processing=================================
//==========================================================================================
//--------------------------absorption------------------------------------------------------
            uint32 dm = dmg.full_damage;
            abs = pVictim->AbsorbDamage(this, dmg.school_type,(uint32*)&dm, ability);

            if(dmg.full_damage > (int32)blocked_damage)
            {
                uint32 sh = pVictim->ManaShieldAbsorb(dmg.full_damage, ability);
//--------------------------armor reducing--------------------------------------------------
                if(sh)
                {
                    dmg.full_damage -= sh;
                    if(dmg.full_damage && !disable_dR)
                        CalculateResistanceReduction(pVictim,&dmg, ability,armorreducepct);
                    dmg.full_damage += sh;
                    abs+=sh;
                }
                else if(!disable_dR)
                    CalculateResistanceReduction(pVictim,&dmg, ability,armorreducepct);
            }

            if(abs)
                vproc |= PROC_ON_ABSORB;

            if (dmg.school_type == SCHOOL_NORMAL)
            {
                abs+=dmg.resisted_damage;
                dmg.resisted_damage=0;
            }

            realdamage = dmg.full_damage-abs-dmg.resisted_damage-blocked_damage;
            if(realdamage < 0)
            {
                realdamage = 0;
                vstate = IMMUNE;
                if (!(hit_status & HITSTATUS_BLOCK))
                    hit_status |= HITSTATUS_ABSORBED;
            }
        }
        break;
    }

//==========================================================================================
//==============================Post Roll Special Cases Processing==========================
//==========================================================================================
//--------------------------special states processing---------------------------------------

    if(pVictim->bInvincible == true)
    {
        dmg.resisted_damage = dmg.full_damage; //godmode
    }

//--------------------------dirty fixes-----------------------------------------------------
    //vstate=1-wound,2-dodge,3-parry,4-interrupt,5-block,6-evade,7-immune,8-deflect
    // the above code was remade it for reasons : damage shield needs moslty same flags as handleproc + dual wield should proc too ?
    if( !disable_proc )
    {
        uint32 resisted_dmg;

        HandleProc(aproc, aproc2, pVictim, ability, realdamage, abs, weapon_damage_type + 1); //maybe using dmg.resisted_damage is better sometimes but then if using godmode dmg is resisted instead of absorbed....bad
        resisted_dmg = pVictim->HandleProc(vproc, vproc2, TO_UNIT(this), ability, realdamage, abs, weapon_damage_type + 1);

        if(realdamage > 0)
        {
            pVictim->HandleProcDmgShield(vproc, TO_UNIT(this));
            HandleProcDmgShield(aproc, pVictim);
        }

        if(resisted_dmg)
        {
            dmg.resisted_damage+= resisted_dmg;
            dmg.full_damage-= resisted_dmg;
            realdamage-= resisted_dmg;
        }
    }

//--------------------------split damage-----------------------------------------------

    // Paladin: Blessing of Sacrifice, and Warlock: Soul Link
    if( pVictim->m_damageSplitTarget.active)
    {
        if( ability )
            realdamage = pVictim->DoDamageSplitTarget(realdamage, ability->School, false);
        else
            realdamage = pVictim->DoDamageSplitTarget(realdamage, 0, true);
    }

//--------------------------spells triggering-----------------------------------------------
    if(realdamage > 0 && ability == 0)
    {
        if( IsPlayer() && TO_PLAYER(this)->m_onStrikeSpells.size() )
        {
            SpellCastTargets targets;
            targets.m_unitTarget = pVictim->GetGUID();
            targets.m_targetMask = 0x2;
            Spell* cspell = NULLSPELL;

            // Loop on hit spells, and strike with those.
            for( map< SpellEntry*, pair< uint32, uint32 > >::iterator itr = TO_PLAYER(this)->m_onStrikeSpells.begin();
                itr != TO_PLAYER(this)->m_onStrikeSpells.end(); itr++ )
            {
                if( itr->second.first )
                {
                    // We have a *periodic* delayed spell.
                    uint32 t = getMSTime();
                    if( t > itr->second.second )  // Time expired
                    {
                        // Set new time
                        itr->second.second = t + itr->second.first;
                    }

                    // Cast.
                    cspell = (new Spell(TO_UNIT(this), itr->first, true, NULLAURA));
                    cspell->prepare(&targets);
                }
                else
                {
                    cspell = (new Spell(TO_UNIT(this), itr->first, true, NULLAURA));
                    cspell->prepare(&targets);
                }
            }
        }

        if( IsPlayer() && TO_PLAYER(this)->m_onStrikeSpellDmg.size() )
        {
            map< uint32, OnHitSpell >::iterator it2 = TO_PLAYER(this)->m_onStrikeSpellDmg.begin();
            map< uint32, OnHitSpell >::iterator itr;
            uint32 min_dmg, max_dmg, range, dmg;
            for(; it2 != TO_PLAYER(this)->m_onStrikeSpellDmg.end(); )
            {
                itr = it2;
                ++it2;

                min_dmg = itr->second.mindmg;
                max_dmg = itr->second.maxdmg;
                range = min_dmg - max_dmg;
                dmg = min_dmg;
                if(range) range += RandomUInt(range);

                SpellNonMeleeDamageLog(pVictim, itr->second.spellid, dmg, true);
            }
        }

        //ugly hack for shadowfiend restoring mana
        if( GetUInt64Value(UNIT_FIELD_SUMMONEDBY) != 0 && GetUInt32Value(OBJECT_FIELD_ENTRY) == 19668 )
        {
            Player* owner = GetMapMgr()->GetPlayer((uint32)GetUInt64Value(UNIT_FIELD_SUMMONEDBY));
            if ( owner != NULL )    // restore 4% of max mana on each hit
                Energize(owner, 34433, owner->GetUInt32Value(UNIT_FIELD_MAXPOWER1) / 25, POWER_TYPE_MANA );
        }
    }

//==========================================================================================
//==============================Data Sending================================================
//==========================================================================================
    if( !ability )
    {
        if( dmg.full_damage > 0 )
        {
            if( abs > 0 )
                hit_status |= HITSTATUS_ABSORBED;
            else if (dmg.resisted_damage > 0)
                hit_status |= HITSTATUS_RESIST;
        }

        if( dmg.full_damage < 0 )
            dmg.full_damage = 0;

        if( realdamage < 0 )
            realdamage = 0;

        SendAttackerStateUpdate(pVictim, &dmg, realdamage, abs, blocked_damage, hit_status, vstate);
    }
    else
    {
        if( realdamage > 0 )//FIXME: add log for miss,block etc for ability and ranged
        {
            // here we send "dmg.resisted_damage" for "AbsorbedDamage", "0" for "ResistedDamage", and "false" for "PhysicalDamage" even though "School" is "SCHOOL_NORMAL"   o_O
            SendSpellNonMeleeDamageLog( TO_UNIT(this), pVictim, ability->Id, realdamage, dmg.school_type, dmg.resisted_damage, 0, false, blocked_damage, ( ( hit_status & HITSTATUS_CRICTICAL ) != 0 ), true );
        }
        //FIXME: add log for miss,block etc for ability and ranged
        //example how it works
        //SendSpellLog(this,pVictim,ability->Id,SPELL_LOG_MISS);
    }

    if(ability && realdamage == 0)
    {
        SendSpellLog(TO_OBJECT(this),pVictim,ability->Id,SPELL_LOG_RESIST);
    }
//==========================================================================================
//==============================Damage Dealing==============================================
//==========================================================================================

    if(IsPlayer() && ability)
        TO_PLAYER(this)->m_casted_amount[dmg.school_type]=(uint32)(realdamage+abs);
    if(realdamage)
    {
        DealDamage(pVictim, realdamage, 0, targetEvent, 0);

        if(pVictim->GetCurrentSpell())
            pVictim->GetCurrentSpell()->AddTime(0);
    }
    else
    {
        // have to set attack target here otherwise it wont be set
        // because dealdamage is not called.
        //setAttackTarget(pVictim);
    }
//==========================================================================================
//==============================Post Damage Dealing Processing==============================
//==========================================================================================
//--------------------------durability processing-------------------------------------------
    if(pVictim->IsPlayer())
    {
        TO_PLAYER( pVictim )->GetItemInterface()->ReduceItemDurability();
        if( !IsPlayer() )
        {
            Player* pr = TO_PLAYER( pVictim );
            if( Rand( pr->GetSkillUpChance( SKILL_DEFENSE ) * sWorld.getRate( RATE_SKILLCHANCE ) ) )
            {
                pr->_AdvanceSkillLine( SKILL_DEFENSE, float2int32( 1.0f * sWorld.getRate(RATE_SKILLRATE)));
                pr->UpdateChances();
            }
        }
        else
        {
             TO_PLAYER(this)->GetItemInterface()->ReduceItemDurability();
        }
    }
    else if(pVictim->IsCreature())
    {   // Not PvP, proto, and skill up.
        if(IsPlayer())
        {
            if(!TO_CREATURE(pVictim)->GetExtraInfo() || !TO_CREATURE(pVictim)->GetExtraInfo()->no_skill_up)
            {
                TO_PLAYER(this)->GetItemInterface()->ReduceItemDurability();
                Player* pr = TO_PLAYER(this);
                if( Rand( pr->GetSkillUpChance( SubClassSkill) * sWorld.getRate( RATE_SKILLCHANCE ) ) )
                {
                    pr->_AdvanceSkillLine( SubClassSkill, float2int32( 1.0f * sWorld.getRate(RATE_SKILLRATE)));
                    pr->UpdateChances();
                }
            }
        }
    }
    //--------------------------rage processing-------------------------------------------------
    //http://www.wowwiki.com/Formulas:Rage_generation

    if( dmg.full_damage && IsPlayer() && GetPowerType() == POWER_TYPE_RAGE && !ability)
    {
        float val;
        float level = (float)getLevel();

        // Conversion Value
        float c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;

        // Hit Factor
        float f = ( weapon_damage_type == OFFHAND ) ? 1.75f : 3.5f;

        if( hit_status & HITSTATUS_CRICTICAL )
            f *= 2.0f;

        float s = 1.0f;

        // Weapon speed (normal)
        Item* weapon = ( TO_PLAYER(this)->GetItemInterface())->GetInventoryItem( INVENTORY_SLOT_NOT_SET, ( weapon_damage_type == OFFHAND ? EQUIPMENT_SLOT_OFFHAND : EQUIPMENT_SLOT_MAINHAND ) );
        if( weapon == NULL )
        {
            if( weapon_damage_type == OFFHAND )
                s = GetUInt32Value( UNIT_FIELD_BASEATTACKTIME + 1 ) / 1000.0f;
            else
                s = GetUInt32Value( UNIT_FIELD_BASEATTACKTIME ) / 1000.0f;
        }
        else
        {
            uint32 entry = weapon->GetEntry();
            ItemPrototype* pProto = ItemPrototypeStorage.LookupEntry( entry );
            if( pProto != NULL )
            {
                s = pProto->Delay / 1000.0f;
            }
        }

        val = ( 7.5f * dmg.full_damage / c + f * s ) / 2.0f;;
        val *= ( 1 + ( TO_PLAYER(this)->rageFromDamageDealt / 100.0f ) );
        val *= 10;

        //float r = ( 7.5f * dmg.full_damage / c + f * s ) / 2.0f;
        //float p = ( 1 + ( TO_PLAYER(this)->rageFromDamageDealt / 100.0f ) );
        //sLog.outDebug( "Rd(%i) d(%i) c(%f) f(%f) s(%f) p(%f) r(%f) rage = %f", realdamage, dmg.full_damage, c, f, s, p, r, val );

        ModUnsigned32Value( UNIT_FIELD_POWER2, (int32)val );
        if( GetUInt32Value( UNIT_FIELD_POWER2 ) > 1000 )
            ModUnsigned32Value( UNIT_FIELD_POWER2, 1000 - GetUInt32Value( UNIT_FIELD_POWER2 ) );

        SendPowerUpdate();
    }

    // I am receiving damage!
    if( dmg.full_damage && pVictim->IsPlayer() && pVictim->GetPowerType() == POWER_TYPE_RAGE && pVictim->CombatStatus.IsInCombat() )
    {
        float val;
        float level = (float)getLevel();

        // Conversion Value
        float c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;

        val = 2.5f * dmg.full_damage / c;
        val *= 10;

        //sLog.outDebug( "Rd(%i) d(%i) c(%f) rage = %f", realdamage, dmg.full_damage, c, val );

        pVictim->ModUnsigned32Value( UNIT_FIELD_POWER2, (int32)val );
        if( pVictim->GetUInt32Value( UNIT_FIELD_POWER2) > 1000 )
            pVictim->ModUnsigned32Value( UNIT_FIELD_POWER2, 1000 - pVictim->GetUInt32Value( UNIT_FIELD_POWER2 ) );
        pVictim->SendPowerUpdate();
    }

    m_AuraInterface.RemoveAllAurasByInterruptFlag(AURA_INTERRUPT_ON_START_ATTACK);

//--------------------------extra strikes processing----------------------------------------
    //check for extra targets
    /*if( ability && ability->SpellGroupType )
    {
        SM_FIValue(SM[SMT_ADDITIONAL_TARGET][0], &m_extraattacks, ability->SpellGroupType);
    }*/

    if( m_extraattacks[1] > 0 && proc_extrastrike)
    {
        do
        {
            m_extraattacks[1]--;
            realdamage += Strike( pVictim, weapon_damage_type, NULL, 0, 0, 0, true, false );
        }while( m_extraattacks[1] > 0 && m_extraattacks[0] == 0 );
    }

    if(m_extrastriketargetc > 0 && m_extrastriketarget == 0)
    {
        m_extrastriketarget = 1;

        for(std::list<ExtraStrike*>::iterator itx = m_extraStrikeTargets.begin();itx != m_extraStrikeTargets.end(); itx++)
        {
            ExtraStrike *ex = *itx;

            if (ex->deleted)
                continue;

            for(unordered_set<Object* >::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); itr++)
            {
                if (!(*itr) || (*itr) == pVictim || !(*itr)->IsUnit())
                    continue;

                if(CalcDistance(*itr) < 5.0f && sFactionSystem.isAttackable(TO_UNIT(this), (*itr)) && isTargetInFront(*itr) && !TO_UNIT(*itr)->IsPacified())
                {
                    // Sweeping Strikes hits cannot be dodged, missed or parried (from wowhead)
                    bool skip_hit_check = ex->spell_info->Id == 12328 ? true : false;
                    realdamage += Strike( TO_UNIT( *itr ), weapon_damage_type, ex->spell_info, add_damage, pct_dmg_mod, exclusive_damage, false, skip_hit_check );
                    break;
                }
            }

            // Sweeping Strikes charges are used up regardless whether there is a secondary target in range or not. (from wowhead)
            if (ex->charges > 0)
            {
                ex->charges--;
                if (ex->charges <= 0)
                {
                    ex->deleted = true;
                    m_extrastriketargetc--;
                }
            }
        }

        m_extrastriketarget = 0;
    }
    return realdamage;
}


void Unit::smsg_AttackStop(Unit* pVictim)
{
    if(!pVictim)
        return;

    WorldPacket data(SMSG_ATTACKSTOP, 20);
    if(m_objectTypeId == TYPEID_PLAYER)
    {
        data << pVictim->GetNewGUID();
        data << uint8(0);
        data << uint32(0);
        TO_PLAYER(this)->GetSession()->SendPacket( &data );
        data.clear();
    }

    data << GetNewGUID();
    data << pVictim->GetNewGUID();
    data << uint32(0);
    SendMessageToSet(&data, true );
}

void Unit::smsg_AttackStop(uint64 victimGuid)
{
    WorldPacket data(20);
    data.Initialize( SMSG_ATTACKSTOP );
    data << GetNewGUID();
    FastGUIDPack(data, victimGuid);
    data << uint32( 0 );
    SendMessageToSet(&data, IsPlayer());
}

void Unit::smsg_AttackStart(Unit* pVictim)
{
    // Send out ATTACKSTART
    WorldPacket data(SMSG_ATTACKSTART, 16);
    data << GetGUID();
    data << pVictim->GetGUID();
    SendMessageToSet(&data, false);
}

void Unit::_UpdateSpells( uint32 time )
{
    if(m_currentSpell != NULL)
        m_currentSpell->update(time);

    uint32 MSTime = getMSTime();
    for(std::set<Spell*>::iterator itr = DelayedSpells.begin(); itr != DelayedSpells.end();)
    {
        if((*itr)->HandleDestTargetHit(GetGUID(), MSTime))
            itr = DelayedSpells.erase(itr);
        else itr++;
    }
}

void Unit::CastSpell( Spell* pSpell )
{
    // check if we have a spell already casting etc
    if(m_currentSpell && pSpell != m_currentSpell)
        sEventMgr.AddEvent(TO_UNIT(this), &Unit::EventCancelSpell, m_currentSpell, EVENT_UNK, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    m_currentSpell = pSpell;
    pLastSpell = pSpell->m_spellInfo;
}

int32 Unit::GetSpellBonusDamage(Unit* pVictim, SpellEntry *spellInfo,int32 base_dmg, bool healing)
{
    int32 bonus_damage = base_dmg;
    Unit* caster = TO_UNIT(this);
    uint32 school = spellInfo->School;
    float summaryPCTmod = 0.0f;
    float levelPenalty = CalculateLevelPenalty(spellInfo);

    if( caster->IsPet() )
        caster = TO_UNIT(TO_PET(caster)->GetPetOwner());
    else if( caster->IsSummon() && TO_SUMMON(caster)->GetSummonOwner() )
        caster = TO_SUMMON(caster)->GetSummonOwner()->IsUnit() ? TO_UNIT(TO_SUMMON(caster)->GetSummonOwner()) : NULLUNIT;
    else if( caster->GetTypeId() == TYPEID_GAMEOBJECT && caster->GetMapMgr() && caster->GetUInt64Value(OBJECT_FIELD_CREATED_BY) )
        caster = TO_UNIT(caster->GetMapMgr()->GetUnit(caster->GetUInt64Value(OBJECT_FIELD_CREATED_BY)));
    if( caster == NULL || pVictim == NULL)
        return bonus_damage;

    //---------------------------------------------------------
    // victim type
    //---------------------------------------------------------

    if( pVictim->IsCreature() && TO_CREATURE(pVictim)->GetCreatureInfo() && caster->IsPlayer() && !pVictim->IsPlayer() )
        bonus_damage += TO_PLAYER(caster)->IncreaseDamageByType[TO_CREATURE(pVictim)->GetCreatureInfo()->Type];

    //---------------------------------------------------------
    // coefficient
    //---------------------------------------------------------
    float coefficient = IsCreature() ? 1.0f : 0.0f;

    if(spellInfo->SP_coef_override > 0.0f)
        coefficient = spellInfo->SP_coef_override;

    //---------------------------------------------------------
    // modifiers (increase spell dmg by spell power)
    //---------------------------------------------------------
    if( spellInfo->SpellGroupType )
    {
        float modifier = 0;
        if( caster->HasDummyAura( SPELL_HASH_ARCANE_EMPOWERMENT ) )
        {
            if( spellInfo->NameHash == SPELL_HASH_ARCANE_MISSILES )
            {
                modifier += 15 * caster->GetDummyAura( SPELL_HASH_ARCANE_EMPOWERMENT )->RankNumber;
            }
            else if( spellInfo->NameHash == SPELL_HASH_ARCANE_BLAST )
            {
                modifier += 3 * caster->GetDummyAura( SPELL_HASH_ARCANE_EMPOWERMENT )->RankNumber;
            }
        }

        SM_FFValue( caster->SM[SMT_SP_BONUS][0], &modifier, spellInfo->SpellGroupType );
        SM_FFValue( caster->SM[SMT_SP_BONUS][1], &modifier, spellInfo->SpellGroupType );
        coefficient += modifier / 100.0f;
    }

    //---------------------------------------------------------
    // MISC COEFFICIENT
    //---------------------------------------------------------
    if( spellInfo->NameHash == SPELL_HASH_IMMOLATE && HasDummyAura( SPELL_HASH_FIRE_AND_BRIMSTONE ))
        coefficient += ( (GetDummyAura(SPELL_HASH_FIRE_AND_BRIMSTONE)->RankNumber * 3) / 100.0f );

    //---------------------------------------------------------
    // Apply coefficient
    //---------------------------------------------------------
    if( !healing )
    {
        if(coefficient) // Saves us some time.
            bonus_damage += caster->GetDamageDoneMod(school) * coefficient * levelPenalty;
        bonus_damage += pVictim->DamageTakenMod[school] * levelPenalty;
    }
    else
    {
        if(coefficient) // Saves us some time.
            bonus_damage += caster->GetHealingDoneMod() * coefficient * levelPenalty;
        bonus_damage += pVictim->GetHealingTakenMod() * levelPenalty;
    }

    //---------------------------------------------------------
    // AP coefficient
    //---------------------------------------------------------
    if( spellInfo->AP_coef_override > 0 )
        bonus_damage += float2int32(caster->GetAP() * spellInfo->AP_coef_override * levelPenalty);

    if( spellInfo->RAP_coef_override > 0 )
        bonus_damage += float2int32(caster->GetRAP() * spellInfo->RAP_coef_override * levelPenalty);

    //---------------------------------------------------------
    // modifiers (damage done by x)
    //---------------------------------------------------------
    if( spellInfo->SpellGroupType )
    {
        float dmg_bonus_pct = 0;
        SM_FIValue(caster->SM[SMT_DAMAGE_DONE][0], &bonus_damage, spellInfo->SpellGroupType);
        SM_FFValue(caster->SM[SMT_SPELL_VALUE_PCT][1], &dmg_bonus_pct, spellInfo->SpellGroupType);
        SM_FFValue(caster->SM[SMT_DAMAGE_DONE][1], &dmg_bonus_pct, spellInfo->SpellGroupType);

        // Molten Fury - Should be done in SpellAuraOverrideClassScripts, but heh xD
        if(IsPlayer() && pVictim->GetHealthPct() <= 35)
        {
            if( spellInfo->School == SCHOOL_SHADOW && HasDummyAura(SPELL_HASH_DEATH_S_EMBRACE) )
                dmg_bonus_pct += ((GetDummyAura(SPELL_HASH_DEATH_S_EMBRACE)->RankNumber * 4) / 100.0f );
            else
                dmg_bonus_pct += (int)TO_PLAYER(this)->m_moltenFuryDamageIncreasePct;
        }

        dmg_bonus_pct /= 100;
        bonus_damage += float2int32(bonus_damage * dmg_bonus_pct);
    }

    //---------------------------------------------------------
    // MISC BONUSDAMAGE
    //---------------------------------------------------------

    if( caster->IsPlayer() )
    {
        Player* plrCaster = TO_PLAYER(caster);
        // Improved Tree of Life
        if( plrCaster->IsInFeralForm() )
        {
            if( pVictim->IsPlayer() && TO_PLAYER( pVictim )->GetShapeShift() == FORM_CAT )
            {
                if( pVictim->HasDummyAura(SPELL_HASH_NURTURING_INSTINCT) && healing )
                    bonus_damage *= 1.2f;
            }
        }
        else if(spellInfo->Id == 31804 )
        {
            Aura* aur = pVictim->m_AuraInterface.FindAura(31803);
            if( !aur )
                aur = pVictim->m_AuraInterface.FindAura(53742);

            uint32 charges = 0;
            if( aur )
                charges = aur->stackSize;

            if( charges < 6 )
                bonus_damage = float2int32(bonus_damage * (1.0f + charges * 0.1f));
        }
    }

    if( (spellInfo->c_is_flags & SPELL_FLAG_IS_DAMAGING) && (spellInfo->isAOE) )
        bonus_damage *= pVictim->AOEDmgMod;

    if( pVictim->RAPvModifier && spellInfo->is_ranged_spell )
        bonus_damage += float2int32(pVictim->RAPvModifier * (GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME) / 14000.0f));
    else if( pVictim->APvModifier && spellInfo->is_melee_spell )
        bonus_damage += float2int32(pVictim->APvModifier * (GetUInt32Value(UNIT_FIELD_BASEATTACKTIME) / 14000.0f));


    //---------------------------------------------------------
    // PCT mods
    //---------------------------------------------------------

    if( !healing )
    {
        summaryPCTmod += caster->GetDamageDonePctMod(school)-1; //value is initialized with 1
        summaryPCTmod += pVictim->DamageTakenPctMod[school]-1;//value is initialized with 1
        summaryPCTmod += pVictim->ModDamageTakenByMechPCT[Spell::GetMechanic(spellInfo)];
    }
    else
    {
        summaryPCTmod += caster->HealDonePctMod-1;
        summaryPCTmod += pVictim->HealTakenPctMod-1;
    }

    if((spellInfo->SpellGroupType[0] & 0x100821 || spellInfo->SpellGroupType[1] & 0x8000) &&
        caster->HasDummyAura(SPELL_HASH_TORMENT_THE_WEAK) &&
        pVictim->m_speedModifier < 0 )
    {
        summaryPCTmod += (caster->GetDummyAura(SPELL_HASH_TORMENT_THE_WEAK)->EffectBasePoints[0] / 100.f);
    }

    int32 res = float2int32((bonus_damage) * summaryPCTmod) + bonus_damage;
    if( res < 0 )
        res = 0;

    //---------------------------------------------------------
    // Last Calcs
    //---------------------------------------------------------
    if( caster->IsPlayer() )
    {
        if( spellInfo->Id == 31893 || spellInfo->Id == 53719 )
        {
            int32 selfdamage = float2int32((( bonus_damage * summaryPCTmod) + bonus_damage ) * 0.1f);
            if( caster->GetUInt32Value(UNIT_FIELD_HEALTH) - selfdamage < 0 )
                caster->SetUInt32Value(UNIT_FIELD_HEALTH, 1);
            else
                caster->ModUnsigned32Value(UNIT_FIELD_HEALTH, -selfdamage);
        }
        else if( spellInfo->Id == 31898 || spellInfo->Id == 53726 )
        {
            int32 selfdamage = float2int32((( bonus_damage * summaryPCTmod) + bonus_damage ) * 0.33f);
            if( caster->GetUInt32Value(UNIT_FIELD_HEALTH) - selfdamage < 0 )
                caster->SetUInt32Value(UNIT_FIELD_HEALTH, 1);
            else
                caster->ModUnsigned32Value(UNIT_FIELD_HEALTH, -selfdamage);
        }
        else if(spellInfo->Id == 25742 && res > 0)
        {
            res *= float2int32(float(float(GetUInt32Value(UNIT_FIELD_BASEATTACKTIME))/float(1000)));
            if(HasAura(56414))
                res += uint32((float)res*0.1f);
        }
    }

    return res;
}

float Unit::CalculateLevelPenalty(SpellEntry* sp)
{
    if (sp->spellLevel <= 0 || sp->spellLevel >= sp->maxLevel || sp->maxLevel >= MAXIMUM_CEXPANSION_LEVEL)
        return 1.0f; // For custom content purposes, do not reduce damage done by the highest available teir of spells.

    float LvlPenalty = 0.0f;
    if (sp->spellLevel < 20)
        LvlPenalty = 20.0f - sp->spellLevel * 3.75f;
    float LvlFactor = (float(sp->spellLevel) + 6.0f) / float(getLevel());
    if (LvlFactor > 1.0f)
        LvlFactor = 1.0f;

    return float((100.0f - LvlPenalty) * LvlFactor / 100.0f);
}

void Unit::InterruptCurrentSpell()
{
    if(m_currentSpell)
        m_currentSpell->cancel();
}

void Unit::DeMorph()
{
    // hope it solves it :)
    uint32 displayid = GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    SetUInt32Value(UNIT_FIELD_DISPLAYID, displayid);
    EventModelChange();
}

void Unit::Emote(EmoteType emote)
{
    if(emote == 0)
        return;

    WorldPacket data(SMSG_EMOTE, 12);
    data << uint32(emote);
    data << GetGUID();
    SendMessageToSet (&data, true);
}

void Unit::SendChatMessageToPlayer(uint8 type, uint32 lang, const char *msg, Player* plr)
{
    size_t UnitNameLength = 0, MessageLength = 0;
    CreatureInfo *ci = (m_objectTypeId == TYPEID_UNIT) ? TO_CREATURE(this)->creature_info : NULL;

    if(ci == NULL || plr == NULL)
        return;

    UnitNameLength = strlen((char*)ci->Name) + 1;
    MessageLength = strlen((char*)msg) + 1;

    WorldPacket data(SMSG_MESSAGECHAT, 35 + UnitNameLength + MessageLength);
    data << type;
    data << lang;
    data << GetGUID();
    data << uint32(0);          // new in 2.1.0
    data << uint32(UnitNameLength);
    data << ci->Name;
    data << uint64(0);
    data << uint32(MessageLength);
    data << msg;
    data << uint8(0x00);
    plr->GetSession()->SendPacket(&data);
}

void Unit::SendChatMessageAlternateEntry(uint32 entry, uint8 type, uint32 lang, const char * msg)
{
    size_t UnitNameLength = 0, MessageLength = 0;
    CreatureInfo *ci;

    ci = CreatureNameStorage.LookupEntry(entry);
    if(!ci)
        return;

    UnitNameLength = strlen((char*)ci->Name) + 1;
    MessageLength = strlen((char*)msg) + 1;

    WorldPacket data(SMSG_MESSAGECHAT, 35 + UnitNameLength + MessageLength);
    data << type;
    data << lang;
    data << GetGUID();
    data << uint32(0);          // new in 2.1.0
    data << uint32(UnitNameLength);
    data << ci->Name;
    data << uint64(0);
    data << uint32(MessageLength);
    data << msg;
    data << uint8(0x00);
    SendMessageToSet(&data, true);
}

void Unit::SendChatMessage(uint8 type, uint32 lang, const char *msg)
{
    size_t UnitNameLength = 0, MessageLength = 0;
    CreatureInfo *ci = IsCreature() ? TO_CREATURE(this)->GetCreatureInfo() : NULL;
    if(ci == NULL)
        return;

    UnitNameLength = strlen((char*)ci->Name) + 1;
    MessageLength = strlen((char*)msg) + 1;

    WorldPacket data(SMSG_MESSAGECHAT, 35 + UnitNameLength + MessageLength);
    data << type;
    data << lang;
    data << GetGUID();
    data << uint32(0);          // new in 2.1.0
    data << uint32(UnitNameLength);
    data << ci->Name;
    data << uint64(0);
    data << uint32(MessageLength);
    data << msg;
    data << uint8(0x00);
    SendMessageToSet(&data, true);
}

void Unit::AddInRangeObject(Object* pObj)
{
    if((pObj->GetTypeId() == TYPEID_UNIT) || (pObj->IsPlayer()))
    {
        if( sFactionSystem.isHostile( TO_OBJECT(this), pObj ) )
            m_oppFactsInRange.insert(TO_UNIT(pObj));
    }

    Object::AddInRangeObject(pObj);
}//427

void Unit::OnRemoveInRangeObject(Object* pObj)
{
    if(pObj->IsUnit())
        m_oppFactsInRange.erase(TO_UNIT(pObj));

    if(pObj->GetTypeId() == TYPEID_UNIT || pObj->IsPlayer())
    {
        /*if(m_useAI)*/

        Unit* pUnit = TO_UNIT(pObj);
        GetAIInterface()->CheckTarget(pUnit);

        if(GetUInt64Value(UNIT_FIELD_CHARM) == pObj->GetGUID())
            if(m_currentSpell)
                m_currentSpell->cancel();

        Object::OnRemoveInRangeObject(pObj);
    }
    else
    {
        Object::OnRemoveInRangeObject(pObj);
    }
}

void Unit::ClearInRangeSet()
{
    Object::ClearInRangeSet();
}

//Events
void Unit::EventAddEmote(EmoteType emote, uint32 time)
{
    m_oldEmote = GetUInt32Value(UNIT_NPC_EMOTESTATE);
    SetUInt32Value(UNIT_NPC_EMOTESTATE,emote);
    sEventMgr.AddEvent(TO_UNIT(this), &Unit::EmoteExpire, EVENT_UNIT_EMOTE, time, 1,0);
}

void Unit::EventAllowCombat(bool allow)
{
    m_aiInterface->SetAllowedToEnterCombat(allow);
    m_aiInterface->setCanMove(allow);
}

void Unit::EmoteExpire()
{
    SetUInt32Value(UNIT_NPC_EMOTESTATE, m_oldEmote);
    sEventMgr.RemoveEvents(this, EVENT_UNIT_EMOTE);
}

void Unit::MoveToWaypoint(uint32 wp_id)
{
    if(m_useAI && GetAIInterface() != NULL)
    {
        AIInterface *ai = GetAIInterface();
        WayPoint *wp = ai->getWayPoint(wp_id);
        if(!wp)
        {
            sLog.outDebug("Database: Invalid WP %u specified for spawnid %u.", wp_id, TO_CREATURE(this)->GetSQL_id());
            return;
        }

        ai->setWaypointToMove(wp_id);
        if(wp->flags!=0)
            ai->setMoveRunFlag(true);
        ai->MoveTo(wp->x, wp->y, wp->z, wp->orientation);
    }
}

void Unit::CalcDamage()
{
    if( IsPlayer() )
        TO_PLAYER(this)->UpdateStats();
    else
    {
        float r;
        float delta;
        float mult;

        float ap_bonus = float(GetAP())/14000.0f;

        float bonus = ap_bonus*GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);

        delta = float(DamageDonePosMod[0]) - float(DamageDoneNegMod[0]);
        mult = DamageDonePctMod[0];
        r = BaseDamage[0]*mult+delta+bonus;
        // give some diversity to pet damage instead of having a 77-78 damage range (as an example)
        SetFloatValue(UNIT_FIELD_MINDAMAGE,r > 0 ? ( IsPet() ? r * 0.9f : r ) : 0 );
        r = BaseDamage[1]*mult+delta+bonus;
        SetFloatValue(UNIT_FIELD_MAXDAMAGE, r > 0 ? ( IsPet() ? r * 1.1f : r ) : 0 );

//      SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,BaseRangedDamage[0]*mult+delta);
//      SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,BaseRangedDamage[1]*mult+delta);
    }
}

//returns absorbed dmg
uint32 Unit::ManaShieldAbsorb(uint32 dmg, SpellEntry* sp)
{
    if(!m_manashieldamt || !m_manaShieldSpell)
        return 0;

    if( sp && sp->c_is_flags & SPELL_FLAG_PIERCES_ABSORBTION_EFF )
        return 0;

    uint32 mana = GetUInt32Value(UNIT_FIELD_POWER1);
    //int32 effectbonus = SM_PEffectBonus ? SM_PEffectBonus[16] : 0;

    int32 potential = (mana*50)/((100));
    if(potential>m_manashieldamt)
        potential = m_manashieldamt;

    if((int32)dmg<potential)
        potential = dmg;

    uint32 cost = (potential*(100))/50;

    SetUInt32Value(UNIT_FIELD_POWER1, mana - cost);
    m_manashieldamt -= potential;
    if(!m_manashieldamt)
        RemoveAura(m_manaShieldSpell->Id);

    if (potential > 0 && m_incanterAbsorption > 0)
    {
        SpellEntry *spInfo = dbcSpell.LookupEntry(44413);
        if(spInfo)
        {
            Spell* sp = new Spell(TO_UNIT(this), spInfo, true, NULLAURA);
            SpellCastTargets tgt;
            int spamount = std::min(float2int32(GetUInt32Value(UNIT_FIELD_HEALTH) * 0.05f), float2int32((potential * m_incanterAbsorption) / 100));
            sp->forced_basepoints[0] = spamount;
            tgt.m_unitTarget=GetGUID();
            sp->prepare(&tgt);
        }
    }

    return potential;
}

// grep: Remove any AA spells that aren't owned by this player.
//       Otherwise, if he logs out and in and joins another group,
//       he'll apply it to them.
/*
void Unit::RemoveAllAreaAuras()
{
    AuraList::iterator itr,it1;
    for(itr = m_auras.begin();itr!=m_auras.end();)
    {
        it1 = itr;
        ++itr;
        if(((*it1)->m_spellProto->Effect[0] == SPELL_EFFECT_APPLY_AREA_AURA ||
            (*it1)->m_spellProto->Effect[1] == SPELL_EFFECT_APPLY_AREA_AURA ||
            (*it1)->m_spellProto->Effect[2] == SPELL_EFFECT_APPLY_AREA_AURA) && (*it1)->GetCaster() != this)
        {
            (*it1)->Remove();
            m_auras.erase(it1);
        }
    }
}
*/
uint32 Unit::AbsorbDamage( Object* Attacker, uint32 School, uint32* dmg, SpellEntry * pSpell )
{
    if( dmg == NULL || Attacker == NULL  || School > 6 )
        return 0;

    if( pSpell && (pSpell->Id == 59653 || pSpell->c_is_flags & SPELL_FLAG_PIERCES_ABSORBTION_EFF ))
        return 0;

    SchoolAbsorb::iterator i, j;
    uint32 abs = 0;
    int32 reflect_pct = 0;

    for( i = Absorbs[School].begin(); i != Absorbs[School].end(); )
    {
        if( (int32)(*dmg) >= (*i)->amt)//remove this absorb
        {
            (*dmg) -= (*i)->amt;
            abs += (*i)->amt;
            reflect_pct += (*i)->reflect_pct;
            j = i;
            ++i;

            if( i != Absorbs[School].end() )
            {
                while( (*i)->spellid == (*j)->spellid )
                {
                    ++i;
                    if( i == Absorbs[School].end() )
                        break;
                }
            }

            RemoveAura((*j)->spellid); //,(*j)->caster);
            if(!*dmg)//absorbed all dmg
                break;
        }
        else //absorb full dmg
        {
            abs += *dmg;
            reflect_pct += (*i)->reflect_pct;
            (*i)->amt -= *dmg;
            *dmg=0;
            break;
        }
    }

    if( abs > 0 )
    {
        if(reflect_pct > 0 && Attacker && Attacker->IsUnit() )
        {
            int32 amt = float2int32(abs * (reflect_pct / 100.0f ));
            DealDamage( TO_UNIT( Attacker ), amt, 0, 0, 0 );
        }
        if(m_incanterAbsorption > 0)
        {
            SpellEntry *spInfo = dbcSpell.LookupEntry(44413);
            if(spInfo)
            {
                Spell* sp = NULLSPELL;
                sp = (new Spell(this, spInfo, true, NULLAURA));
                SpellCastTargets tgt;
                int spamount = std::min(float2int32(GetUInt32Value(UNIT_FIELD_HEALTH) * 0.05f), float2int32((abs * m_incanterAbsorption) / 100));
                sp->forced_basepoints[0] = spamount;
                tgt.m_unitTarget=GetGUID();
                sp->prepare(&tgt);
            }
        }
    }

    return abs;
}

bool Unit::setDetectRangeMod(uint64 guid, int32 amount)
{
    int next_free_slot = -1;
    for(int i = 0; i < 5; i++)
    {
        if(m_detectRangeGUID[i] == 0 && next_free_slot == -1)
        {
            next_free_slot = i;
        }
        if(m_detectRangeGUID[i] == guid)
        {
            m_detectRangeMOD[i] = amount;
            return true;
        }
    }
    if(next_free_slot != -1)
    {
        m_detectRangeGUID[next_free_slot] = guid;
        m_detectRangeMOD[next_free_slot] = amount;
        return true;
    }
    return false;
}

void Unit::unsetDetectRangeMod(uint64 guid)
{
    for(int i = 0; i < 5; i++)
    {
        if(m_detectRangeGUID[i] == guid)
        {
            m_detectRangeGUID[i] = 0;
            m_detectRangeMOD[i] = 0;
        }
    }
}

int32 Unit::getDetectRangeMod(uint64 guid)
{
    for(int i = 0; i < 5; i++)
    {
        if(m_detectRangeGUID[i] == guid)
        {
            return m_detectRangeMOD[i];
        }
    }
    return 0;
}

void Unit::SetStandState(uint8 standstate)
{
    SetByte( UNIT_FIELD_BYTES_1, 0, standstate );
    if( standstate == STANDSTATE_STAND )//standup
        m_AuraInterface.RemoveAllAurasByInterruptFlag(AURA_INTERRUPT_ON_STAND_UP);

    if( m_objectTypeId == TYPEID_PLAYER )
        TO_PLAYER(this)->GetSession()->OutPacket( SMSG_STANDSTATE_UPDATE, 1, &standstate );
}

void Unit::UpdateSpeed()
{
    if(GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID) == 0)
        m_runSpeed = m_base_runSpeed*(1.0f + ((float)m_speedModifier)/100.0f);
    else
    {
        m_runSpeed = m_base_runSpeed*(1.0f + ((float)m_mountedspeedModifier)/100.0f);
        m_runSpeed += (m_speedModifier < 0) ? (m_base_runSpeed*((float)m_speedModifier)/100.0f) : 0;
    }

    m_flySpeed = PLAYER_NORMAL_FLIGHT_SPEED*(1.0f + ((float)m_flyspeedModifier)/100.0f);

    // Limit speed due to effects such as http://www.wowhead.com/?spell=31896 [Judgement of Justice]
    if( m_maxSpeed && m_runSpeed > m_maxSpeed )
            m_runSpeed = m_maxSpeed;

    if(IsVehicle() && TO_VEHICLE(this)->GetControllingUnit())
    {
        Unit* pUnit = TO_VEHICLE(this)->GetControllingUnit();
        pUnit->m_runSpeed = m_runSpeed;
        pUnit->m_flySpeed = m_flySpeed;

        if(pUnit->IsPlayer())
        {
            if(TO_PLAYER(pUnit)->m_changingMaps)
                TO_PLAYER(pUnit)->resend_speed = true;
            else
            {
                TO_PLAYER(pUnit)->SetPlayerSpeed(RUN, m_runSpeed);
                TO_PLAYER(pUnit)->SetPlayerSpeed(FLY, m_flySpeed);
            }
        }
    }

    SetSpeed(WALK, m_walkSpeed);
    SetSpeed(RUN, m_runSpeed);
    SetSpeed(FLY, m_flySpeed);

    if(IsPlayer())
    {
        if(TO_PLAYER(this)->m_changingMaps)
            TO_PLAYER(this)->resend_speed = true;
    }
}

void Unit::EventSummonPetExpire()
{
    /*if(summonPet)
    {
        if(summonPet->GetEntry() == 7915)//Goblin Bomb
        {
            SpellEntry *spInfo = dbcSpell.LookupEntry(13259);
            if(!spInfo)
                return;

            Spell* sp = NULLSPELL;
            sp = (new Spell(summonPet,spInfo,true,NULLAURA));
            SpellCastTargets tgt;
            tgt.m_unitTarget=summonPet->GetGUID();
            sp->prepare(&tgt);
        }
        else
        {
            summonPet->RemoveFromWorld(false, true);
            summonPet->Destruct();
            summonPet = NULL;
        }
    }*/
    sEventMgr.RemoveEvents(this, EVENT_SUMMON_PET_EXPIRE);
}

void Unit::CastSpell(Unit* Target, SpellEntry* Sp, bool triggered, uint32 forcedCastTime)
{
    if( Sp == NULL )
        return;

    Spell* newSpell = new Spell(this, Sp, triggered, NULLAURA);
    SpellCastTargets targets(0);
    if(Target)
    {
        targets.m_targetMask |= TARGET_FLAG_UNIT;
        targets.m_unitTarget = Target->GetGUID();
    }
    else
        newSpell->GenerateTargets(&targets);
    if(forcedCastTime)
        newSpell->m_ForcedCastTime = forcedCastTime;

    newSpell->prepare(&targets);
}

void Unit::CastSpell(Unit* Target, uint32 SpellID, bool triggered, uint32 forcedCastTime)
{
    SpellEntry * ent = dbcSpell.LookupEntry(SpellID);
    if(ent == 0)
        return;

    CastSpell(Target, ent, triggered, forcedCastTime);
}

void Unit::CastSpell(uint64 targetGuid, SpellEntry* Sp, bool triggered, uint32 forcedCastTime)
{
    if( Sp == NULL )
        return;

    SpellCastTargets targets(targetGuid);
    Spell* newSpell = NULLSPELL;
    newSpell = (new Spell(TO_UNIT(this), Sp, triggered, NULLAURA));
    if(forcedCastTime)
        newSpell->m_ForcedCastTime = forcedCastTime;

    newSpell->prepare(&targets);
}

void Unit::CastSpell(uint64 targetGuid, uint32 SpellID, bool triggered, uint32 forcedCastTime)
{
    SpellEntry * ent = dbcSpell.LookupEntry(SpellID);
    if(ent == 0)
        return;

    CastSpell(targetGuid, ent, triggered, forcedCastTime);
}

uint8 Unit::CastSpellAoF(float x,float y,float z,SpellEntry* Sp, bool triggered, uint32 forcedCastTime)
{
    if( Sp == NULL )
        return SPELL_FAILED_ERROR;

    /*creature will not cast spells while moving, just interrupts itself all the time*/
    if(!IsPlayer() && (IsStunned() || IsPacified() || !isAlive() || m_silenced))
        return SPELL_FAILED_INTERRUPTED;

    SpellCastTargets targets;
    targets.m_destX = x;
    targets.m_destY = y;
    targets.m_destZ = z;
    targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
    Spell* newSpell = new Spell(TO_UNIT(this), Sp, triggered, NULLAURA);
    if(forcedCastTime)
        newSpell->m_ForcedCastTime = forcedCastTime;

    return newSpell->prepare(&targets);
}

void Unit::PlaySpellVisual(uint64 target, uint32 spellVisual)
{
    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 12);
    data << target << spellVisual;
    SendMessageToSet(&data, true);
}

void Unit::Root()
{
    m_special_state |= UNIT_STATE_ROOT;

    if(m_objectTypeId == TYPEID_PLAYER)
    {
        TO_PLAYER(this)->SetMovement(MOVE_ROOT, 1);
    }
    else
    {
        m_aiInterface->setCanMove(false);
        m_aiInterface->StopMovement(1);
    }
}

void Unit::UnRoot()
{
    m_special_state &= ~UNIT_STATE_ROOT;

    if(m_objectTypeId == TYPEID_PLAYER)
    {
        TO_PLAYER(this)->SetMovement(MOVE_UNROOT, 5);
    }
    else
    {
        m_aiInterface->setCanMove(true);
    }
}

void Unit::OnPushToWorld()
{
    m_AuraInterface.BuildAllAuraUpdates();
}

void Unit::RemoveFromWorld(bool free_guid)
{
    SummonExpireAll(false);

    if( GetVehicle() )
    {
        if(IsPlayer())
            GetVehicle()->RemovePassenger(this);
        else
            GetVehicle()->DeletePassengerData(this);

        SetVehicle(NULLVEHICLE);
    }

    if(GetInRangePlayersCount())
    {
        for(unordered_set<Player* >::iterator itr = GetInRangePlayerSetBegin(); itr != GetInRangePlayerSetEnd(); itr++)
        {
            if((*itr)->GetSelection() == GetGUID())
            {
                (*itr)->smsg_AttackStop(this);
                (*itr)->SetSelection(0);
                (*itr)->SetUInt64Value(UNIT_FIELD_TARGET, 0);
            }
        }
    }

    // Delete AAura's from our targets (must be done before object is removed from world)
    m_AuraInterface.RemoveAllAurasWithSpEffect(SPELL_EFFECT_APPLY_AREA_AURA);
    m_AuraInterface.RemoveAllAurasWithSpEffect(SPELL_EFFECT_APPLY_AURA_128);

    if(m_currentSpell != NULL)
    {
        m_currentSpell->cancel();
        m_currentSpell = NULLSPELL;
    }

    for(std::set<Spell*>::iterator itr = DelayedSpells.begin(), itr2; itr != DelayedSpells.end(); itr)
        (*(itr2 = itr++))->HandleRemoveDestTarget(GetGUID());
    DelayedSpells.clear();

    CombatStatus.OnRemoveFromWorld();
    Object::RemoveFromWorld(free_guid);
    m_aiInterface->WipeReferences();
    m_AuraInterface.RelocateEvents(); // Relocate our aura's (must be done after object is removed from world
}

void Unit::SetPosition( float newX, float newY, float newZ, float newOrientation )
{
    Object::SetPosition(newX, newY, newZ, newOrientation);
    movement_info.SetPosition(newX, newY, newZ, newOrientation);
}

bool Unit::IsPoisoned()
{
    return m_AuraInterface.IsPoisoned();
}

uint32 Unit::GetPoisonDosesCount( uint32 poison_type )
{
    return m_AuraInterface.GetPoisonDosesCount(poison_type);
}

void Unit::EnableFlight()
{
    WorldPacket data(SMSG_MOVE_SET_CAN_FLY, 13);
    data << GetNewGUID();
    data << uint32(2);
    SendMessageToSet(&data, true);

    if(IsCreature()) // give them a "flying" animation so they don't just airwalk lul
    {
        SetByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
        movement_info.movementFlags = (0x1000000 | 0x2000000);
    }

    if( IsPlayer() )
    {
        TO_PLAYER(this)->m_setflycheat = true;
        TO_PLAYER(this)->GetSession()->m_isFalling = false;
        TO_PLAYER(this)->GetSession()->m_isJumping = false;
        TO_PLAYER(this)->GetSession()->m_isKnockedback = false;
    }
}

void Unit::DisableFlight()
{
    WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
    data << GetNewGUID();
    data << uint32(5);
    SendMessageToSet(&data, true);

    if(IsCreature()) // Remove their "flying"
    {
        RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, 0x03);
        movement_info.movementFlags = (MONSTER_MOVE_FLAG_STAND);
    }

    if( IsPlayer() )
    {
        TO_PLAYER(this)->z_axisposition = 0.0f;
        TO_PLAYER(this)->m_setflycheat = false;
    }
}

void Unit::EventRegainFlight()
{
    if(!IsPlayer())
    {
        if(sEventMgr.HasEvent(this,EVENT_REGAIN_FLIGHT))
            sEventMgr.RemoveEvents(this,EVENT_REGAIN_FLIGHT);
        return;
    }
    Player * plr = TO_PLAYER(this);

    if(!plr->m_FlyingAura)
    {
        if(sEventMgr.HasEvent(this,EVENT_REGAIN_FLIGHT))
            sEventMgr.RemoveEvents(this,EVENT_REGAIN_FLIGHT);
        return;
    }

    EnableFlight();
    if(sEventMgr.HasEvent(this,EVENT_REGAIN_FLIGHT))
        sEventMgr.RemoveEvents(this,EVENT_REGAIN_FLIGHT);
}

bool Unit::IsDazed()
{
    return m_AuraInterface.IsDazed();
}

void Unit::UpdateVisibility()
{
    ByteBuffer buf(2500);
    InRangeSet::iterator itr, it3;
    uint32 count;
    bool can_see;
    bool is_visible;
    Player* pl;
    Object* pObj;
    Player* plr;

    if( m_objectTypeId == TYPEID_PLAYER )
    {
        plr = TO_PLAYER(this);
        for( Object::InRangeSet::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end();)
        {
            pObj = (*itr);
            ++itr;

            can_see = plr->CanSee(pObj);
            is_visible = plr->GetVisibility(pObj, &it3);
            if(can_see)
            {
                if(!is_visible)
                {
                    buf.clear();
                    count = pObj->BuildCreateUpdateBlockForPlayer( &buf, plr );
                    plr->PushUpdateData(&buf, count);
                    plr->AddVisibleObject(pObj);
                }
            }
            else
            {
                if(is_visible)
                {
                    pObj->DestroyForPlayer(plr);
                    plr->RemoveVisibleObject(it3);
                }
            }

            if( pObj->IsPlayer() )
            {
                pl = TO_PLAYER( pObj );
                can_see = pl->CanSee( plr );
                is_visible = pl->GetVisibility( plr, &it3 );
                if( can_see )
                {
                    if(!is_visible)
                    {
                        buf.clear();
                        count = plr->BuildCreateUpdateBlockForPlayer( &buf, pl );
                        pl->PushUpdateData(&buf, count);
                        pl->AddVisibleObject(plr);
                    }
                }
                else
                {
                    if(is_visible)
                    {
                        plr->DestroyForPlayer(pl);
                        pl->RemoveVisibleObject(it3);
                    }
                }
            }
        }
    }
    else            // For units we can save a lot of work
    {
        for(unordered_set<Player*  >::iterator it2 = GetInRangePlayerSetBegin(); it2 != GetInRangePlayerSetEnd(); it2++)
        {
            can_see = (*it2)->CanSee(TO_OBJECT(this));
            is_visible = (*it2)->GetVisibility(TO_OBJECT(this), &itr);
            if(!can_see)
            {
                if(is_visible)
                {
                    DestroyForPlayer(*it2);
                    (*it2)->RemoveVisibleObject(itr);
                }
            }
            else
            {
                if(!is_visible)
                {
                    buf.clear();
                    count = BuildCreateUpdateBlockForPlayer(&buf, *it2);
                    (*it2)->PushUpdateData(&buf, count);
                    (*it2)->AddVisibleObject(TO_OBJECT(this));
                }
            }
        }
    }
}

void Unit::CastOnMeleeSpell()
{
    SpellEntry *spellInfo = dbcSpell.LookupEntry( GetOnMeleeSpell() );
    Spell *spell = new Spell(this, spellInfo, true, NULL );
    spell->extra_cast_number = GetOnMeleeSpellEcn();
    SpellCastTargets targets;
    targets.m_unitTarget = GetUInt64Value(UNIT_FIELD_TARGET);
    spell->prepare( &targets );
    SetOnMeleeSpell(0, 0);
}

void Unit::RemoveSoloAura(uint32 type)
{
    switch(type)
    {
        case 1:// Polymorph
        {
            if(!polySpell)
                return;

            if(m_AuraInterface.HasActiveAura(polySpell))
                RemoveAura(polySpell);
        }break;
/*      case 2:// Sap
        {
            if(!sapSpell) return;
            if(HasActiveAura(sapSpell))
                RemoveAura(sapSpell);
        }break;
        case 3:// Fear (Warlock)
        {
            if(!fearSpell) return;
            if(HasActiveAura(fearSpell))
                RemoveAura(fearSpell);
        }break;*/
        default:
            {
            sLog.outDebug("Warning: we are trying to remove a soloauratype that has no handle");
            }break;
    }
}

void Unit::EventHealthChangeSinceLastUpdate()
{
    int pct = GetHealthPct();

    uint32 toSet = 0;
    uint32 toRemove = 0;
    if( pct <= 35 && !HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH35) )
        toSet |= AURASTATE_FLAG_HEALTH35;
    else if( pct > 35 && HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH35) )
        toRemove |= AURASTATE_FLAG_HEALTH35;

    if( pct <= 20 && !HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH20) )
        toSet |= AURASTATE_FLAG_HEALTH20;
    else if(pct > 20 && HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH20))
        toRemove |= AURASTATE_FLAG_HEALTH20;

    if( pct >= 75 && !HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTHABOVE75) )
        toSet |= AURASTATE_FLAG_HEALTHABOVE75;
    else if(pct < 75 && HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTHABOVE75))
        toRemove |= AURASTATE_FLAG_HEALTHABOVE75;

    if(toSet)
        SetFlag(UNIT_FIELD_AURASTATE, toSet);

    if(toRemove)
        RemoveFlag(UNIT_FIELD_AURASTATE, toRemove);
}

int32 Unit::GetAP()
{
    int32 baseap = GetUInt32Value(UNIT_FIELD_ATTACK_POWER) + GetAttackPowerPositiveMods() - GetAttackPowerNegativeMods();
    float totalap = float(baseap)*(GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER)+1);
    if(totalap >= 0)
        return float2int32(totalap);
    return  0;
}

int32 Unit::GetRAP()
{
    int32 baseap = GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER) + GetRangedAttackPowerPositiveMods() - GetRangedAttackPowerNegativeMods();
    float totalap = float(baseap)*(GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER)+1);
    if(totalap >= 0)
        return float2int32(totalap);
    return  0;
}

bool Unit::GetSpeedDecrease()
{
    int32 before=m_speedModifier;
    m_speedModifier -= m_slowdown;
    m_slowdown = 0;
    map< uint32, int32 >::iterator itr = speedReductionMap.begin();
    for(; itr != speedReductionMap.end(); itr++)
        m_slowdown = (int32)min( m_slowdown, itr->second );

    if(m_slowdown<-100)
        m_slowdown = 100; //do not walk backwards !

    m_speedModifier += m_slowdown;
    //save bandwidth :P
    if(m_speedModifier!=before)
        return true;
    return false;
}

void Unit::EventCastSpell(Unit* Target, SpellEntry * Sp)
{
    Spell* pSpell = NULLSPELL;
    pSpell = (new Spell(TO_OBJECT(this), Sp, true, NULLAURA));
    SpellCastTargets targets(Target->GetGUID());
    pSpell->prepare(&targets);
}

void Unit::SetFacing(float newo)
{
    SetOrientation(newo);
    /*WorldPacket data(40);
    data.SetOpcode(MSG_MOVE_SET_FACING);
    data << GetNewGUID();
    data << (uint32)0; //flags
    data << (uint32)0; //time
    data << GetPositionX() << GetPositionY() << GetPositionZ() << newo;
    data << (uint32)0; //unk
    SendMessageToSet( &data, false );*/

    /*WorldPacket data(SMSG_MONSTER_MOVE, 60);
    data << GetNewGUID();
    data << m_position << getMSTime();
    data << uint8(4) << newo;
    data << uint32(0x00000000);     // flags
    data << uint32(0);              // time
    data << m_position;             // position
    SendMessageToSet(&data, true);*/
    m_aiInterface->SendMoveToPacket(m_position.x,m_position.y,m_position.z,m_position.o,1,MONSTER_MOVE_FLAG_WALK);
}

//guardians are temporary spawn that will inherit master faction and will folow them. Apart from that they have their own mind
Unit* Unit::CreateTemporaryGuardian(uint32 guardian_entry,uint32 duration,float angle, uint32 lvl, uint8 Slot)
{
    CreatureProto * proto = CreatureProtoStorage.LookupEntry(guardian_entry);
    CreatureInfo * info = CreatureNameStorage.LookupEntry(guardian_entry);
    if(!proto || !info)
    {
        sLog.outDebug("Warning : Missing summon creature template %u !",guardian_entry);
        return NULLUNIT;
    }

    LocationVector v = GetPositionNC();
    float m_followAngle = angle + v.o;
    float x = v.x +(3*(cosf(m_followAngle)));
    float y = v.y +(3*(sinf(m_followAngle)));

    Creature* p = NULLCREATURE;
    p = GetMapMgr()->CreateCreature(guardian_entry);
    if(p == NULLCREATURE)
        return NULLUNIT;

    p->SetInstanceID(GetMapMgr()->GetInstanceID());
    p->Load(proto, GetMapMgr()->iInstanceMode, x, y, v.z, angle);

    if (lvl != 0)
    {
        /* MANA */
        p->SetPowerType(POWER_TYPE_MANA);
        p->SetUInt32Value(UNIT_FIELD_MAXPOWER1,p->GetUInt32Value(UNIT_FIELD_MAXPOWER1)+28+10*lvl);
        p->SetUInt32Value(UNIT_FIELD_POWER1,p->GetUInt32Value(UNIT_FIELD_POWER1)+28+10*lvl);
        /* HEALTH */
        p->SetUInt32Value(UNIT_FIELD_MAXHEALTH,p->GetUInt32Value(UNIT_FIELD_MAXHEALTH)+28+30*lvl);
        p->SetUInt32Value(UNIT_FIELD_HEALTH,p->GetUInt32Value(UNIT_FIELD_HEALTH)+28+30*lvl);
        /* LEVEL */
        p->SetUInt32Value(UNIT_FIELD_LEVEL, lvl);
    }

    p->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, GetGUID());
    p->SetUInt64Value(UNIT_FIELD_CREATEDBY, GetGUID());
    p->SetZoneId(GetZoneId());
    p->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    p->_setFaction();

    p->GetAIInterface()->Init(p,AITYPE_PET,MOVEMENTTYPE_NONE,TO_UNIT(this));
    p->GetAIInterface()->SetUnitToFollow(TO_UNIT(this));
    p->GetAIInterface()->SetUnitToFollowAngle(angle);
    p->GetAIInterface()->SetFollowDistance(3.0f);

    p->PushToWorld(GetMapMgr());

    if(duration)
        sEventMgr.AddEvent(this, &Unit::SummonExpireSlot, Slot, EVENT_SUMMON_EXPIRE_0+Slot, duration, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );

    return TO_UNIT(p);

}

void Unit::SummonExpireAll(bool clearowner)
{
    //Should be done allready, but better check if all summoned Creatures/Totems/GO's are removed;
    //empty our owners summonslot first (if possible).
    if(clearowner && IsCreature() && TO_CREATURE(this)->IsSummon())
    {
        Unit * Owner = NULL;
        if(Owner != NULL)
        {
            uint8 slot = TO_SUMMON(this)->GetSummonSlot();
            Owner->SummonExpireSlot(slot);
        }
    }

    //remove summoned npc's (7 slots)
    for(uint8 x = 0; x < 7; ++x)
        SummonExpireSlot(x);

    //remove summoned go's (4 slots)
    for(uint32 x = 0; x < 4; ++x)
    {
        if(m_mapMgr != NULL && m_ObjectSlots[x])
        {
            GameObject* obj = NULLGOB;
            obj = m_mapMgr->GetGameObject(m_ObjectSlots[x]);
            if(obj != NULL)
                obj->ExpireAndDelete();
            m_ObjectSlots[x] = 0;
        }
    }
}

void Unit::FillSummonList(std::vector<Creature*> &summonList, uint8 summonType)
{
    for(std::map< uint32, std::set<Creature*> >::iterator itr = m_Summons.begin(); itr != m_Summons.end(); itr++)
    {
        if(!itr->second.size())
            continue;
        for(std::set<Creature*>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++)
        {
            // Should never happen
            if(!(*itr2)->IsSummon())
                continue;
            Summon* summon = TO_SUMMON(*itr2);
            if(summonType == 0xFF || summon->GetSummonType() == summonType)
                summonList.push_back(*itr2);
        }
    }
}

void Unit::RemoveSummon(Creature* summon)
{
    for(std::map< uint32, std::set<Creature*> >::iterator itrMain = m_Summons.begin(); itrMain != m_Summons.end(); itrMain++)
    {
        if(itrMain->second.size())
        {
            for(std::set<Creature*>::iterator itr = itrMain->second.begin(); itr != itrMain->second.end(); itr++)
            {
                if(summon->GetGUID() == summon->GetGUID())
                {
                    itrMain->second.erase(itr);
                    break;
                }
            }
        }
    }
}

void Unit::SummonExpireSlot(uint8 Slot)
{
    if(Slot > 7)
        Slot = 0;

    //remove summons
    if(m_Summons[Slot].size())
    {
        Creature* mSum = NULL;
        for(std::set<Creature*>::iterator itr = m_Summons[Slot].begin(); itr != m_Summons[Slot].end(); itr++)
        {
            mSum = *itr;
            if(mSum->IsPet())
            {
                if(TO_PET(mSum)->GetUInt32Value(UNIT_CREATED_BY_SPELL) > 0)
                    TO_PET(mSum)->Dismiss(false);               // warlock summon -> dismiss
                else
                    TO_PET(mSum)->Remove(false, true, true);    // hunter pet -> just remove for later re-call
            }
            else
            {
                mSum->m_AuraInterface.RemoveAllAuras();
                if(mSum->IsInWorld())
                    mSum->Unit::RemoveFromWorld(true);
                mSum->DeleteMe();
            }
        }
        m_Summons[Slot].clear();
    }
    sEventMgr.RemoveEvents(this, EVENT_SUMMON_EXPIRE_0+Slot);
}

float Unit::CalculateDazeCastChance(Unit* target)
{
    float attack_skill = float( getLevel() ) * 5.0f;
    float defense_skill;
    if( target->IsPlayer() )
        defense_skill = float( TO_PLAYER( target )->_GetSkillLineCurrent( SKILL_DEFENSE, false ) );
    else defense_skill = float( target->getLevel() * 5 );
    if( !defense_skill )
        defense_skill = 1;
    float chance_to_daze = attack_skill * 20 / defense_skill;//if level is equal then we get a 20% chance to daze
    chance_to_daze = chance_to_daze * std::min(target->getLevel() / 30.0f, 1.0f );//for targets below level 30 the chance decreses
    if( chance_to_daze > 40 )
        return 40.0f;
    else
        return chance_to_daze;
}

#define COMBAT_TIMEOUT_IN_SECONDS 5
#define COMBAT_TIMEOUT_RANGE 10000      // 100

void CombatStatusHandler::ClearMyHealers()
{
    // this is where we check all our healers
    HealedSet::iterator i;
    Player* pt;
    for(i = m_healers.begin(); i != m_healers.end(); i++)
    {
        pt = m_Unit->GetMapMgr()->GetPlayer(*i);
        if(pt != NULL)
            pt->CombatStatus.RemoveHealed(m_Unit);
    }

    m_healers.clear();
}

void CombatStatusHandler::WeHealed(Unit* pHealTarget)
{
    if(pHealTarget->GetTypeId() != TYPEID_PLAYER || m_Unit->GetTypeId() != TYPEID_PLAYER || pHealTarget == m_Unit)
        return;

    if(pHealTarget->CombatStatus.IsInCombat())
    {
        m_healed.insert(pHealTarget->GetLowGUID());
        pHealTarget->CombatStatus.m_healers.insert(m_Unit->GetLowGUID());
    }

    UpdateFlag();
}

void CombatStatusHandler::RemoveHealed(Unit* pHealTarget)
{
    m_healed.erase(pHealTarget->GetLowGUID());
    UpdateFlag();
}

void CombatStatusHandler::UpdateFlag()
{
    bool n_status = InternalIsInCombat();
    if(n_status != m_lastStatus)
    {
        m_lastStatus = n_status;
        if(n_status)
        {
            //printf(I64FMT" is now in combat.\n", m_Unit->GetGUID());
            m_Unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
            if(!m_Unit->hasStateFlag(UF_ATTACKING)) m_Unit->addStateFlag(UF_ATTACKING);
        }
        else
        {
            //printf(I64FMT" is no longer in combat.\n", m_Unit->GetGUID());
            m_Unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
            if(m_Unit->hasStateFlag(UF_ATTACKING)) m_Unit->clearStateFlag(UF_ATTACKING);

            // remove any of our healers from combat too, if they are able to be.
            ClearMyHealers();
        }
    }
}

bool CombatStatusHandler::InternalIsInCombat()
{
    if(m_healed.size() > 0)
        return true;

    if(m_attackTargets.size() > 0)
        return true;

    if(m_attackers.size() > 0)
        return true;

    return false;
}

bool CombatStatusHandler::IsAttacking(Unit* pTarget)
{
    // dead targets - no combat
    if( pTarget == NULL || m_Unit->isDead() || pTarget->isDead() )
        return false;

    // check the target for any of our DoT's.
    if(pTarget->m_AuraInterface.HasCombatStatusAffectingAuras(m_Unit->GetGUID()))
        return true;

    // place any additional checks here
    return false;
}

void CombatStatusHandler::ForceRemoveAttacker(const uint64& guid)
{
    // called on aura remove, etc.
    //printf("ForceRemoveAttacker "I64FMT"\n", guid);
    AttackerMap::iterator itr = m_attackers.find(guid);
    if(itr == m_attackers.end())
        return;

    m_attackers.erase(itr);
    UpdateFlag();
}

void CombatStatusHandler::RemoveAttackTarget(Unit* pTarget)
{
    // called on aura remove, etc.
    //printf("Trying to remove attack target "I64FMT" from "I64FMT"\n", pTarget->GetGUID(), m_Unit->GetGUID());
    AttackTMap::iterator itr = m_attackTargets.find(pTarget->GetGUID());
    if(itr == m_attackTargets.end())
        return;

    if(!IsAttacking(pTarget))
    {
        if( pTarget->isDead() )
        {
            // remove naow.
            m_attackTargets.erase(itr);
            pTarget->CombatStatus.m_attackers.erase(m_Unit->GetGUID());
            UpdateFlag();
        }
        else
        {
            uint32 new_t = (uint32)UNIXTIME + COMBAT_TIMEOUT_IN_SECONDS;
            //printf("Setting attack target "I64FMT" on "I64FMT" to time out after 5 seconds.\n", pTarget->GetGUID(), m_Unit->GetGUID());
            if( itr->second < new_t )
                itr->second = new_t;
        }
    }
}

void CombatStatusHandler::OnDamageDealt(Unit* pTarget, uint32 damage)
{
    // we added an aura, or dealt some damage to a target. they need to have us as an attacker, and they need to be our attack target if not.
    //printf("OnDamageDealt to "I64FMT" from "I64FMT" timeout %u\n", pTarget->GetGUID(), m_Unit->GetGUID(), timeout);
    if(pTarget == m_Unit)
        return;

    if(!pTarget->isAlive())
        return;

    if( pTarget->GetDistanceSq(m_Unit) > COMBAT_TIMEOUT_RANGE )
        return;     // don't reset the combat timer when out of range.

    AttackTMap::iterator itr = m_attackTargets.find(pTarget->GetGUID());
    uint32 new_t = (uint32)UNIXTIME + COMBAT_TIMEOUT_IN_SECONDS;
    if(itr != m_attackTargets.end())
    {
        if( itr->second < new_t )
            itr->second = new_t;
    }
    else
    {
        m_attackTargets.insert(make_pair( pTarget->GetGUID(), new_t ));
        pTarget->CombatStatus.m_attackers.insert(m_Unit->GetGUID());

        UpdateFlag();
        pTarget->CombatStatus.UpdateFlag();
    }

    map<uint64,uint32>::iterator ditr = pTarget->CombatStatus.DamageMap.find(m_Unit->GetGUID());
    if(ditr == pTarget->CombatStatus.DamageMap.end())
    {
        pTarget->CombatStatus.DamageMap.insert( make_pair( m_Unit->GetGUID(), damage ));
        return;
    }
    ditr->second += damage;
}

void CombatStatusHandler::UpdateTargets()
{
    uint32 mytm = (uint32)UNIXTIME;
    AttackTMap::iterator itr = m_attackTargets.begin();
    AttackTMap::iterator it2 = m_attackTargets.begin();
    Unit* pUnit;

    for(; itr != m_attackTargets.end();)
    {
        it2 = itr;
        ++itr;
        if( it2->second <= mytm )
        {
            //printf("Timeout for attack target "I64FMT" on "I64FMT" expired.\n", it2->first, m_Unit->GetGUID());
            pUnit = m_Unit->GetMapMgr()->GetUnit(it2->first);
            if( pUnit == NULL || pUnit->isDead() )
                m_attackTargets.erase(it2);
            else
            {
                if( !IsAttacking(pUnit) )
                {
                    pUnit->CombatStatus.m_attackers.erase( m_Unit->GetGUID() );
                    pUnit->CombatStatus.UpdateFlag();

                    m_attackTargets.erase(it2);
                }
            }
        }
    }

    UpdateFlag();
}

Unit* CombatStatusHandler::GetKiller()
{
    // No killer
    if(DamageMap.size() == 0)
        return NULLUNIT;

    map<uint64,uint32>::iterator itr = DamageMap.begin();
    uint64 killer_guid = 0;
    uint32 mDamage = 0;
    for(; itr != DamageMap.end(); itr++)
    {
        if(itr->second > mDamage)
        {
            killer_guid = itr->first;
            mDamage = itr->second;
        }
    }

    if( killer_guid == 0 )
        return NULLUNIT;

    return (m_Unit->IsInWorld()) ? m_Unit->GetMapMgr()->GetUnit(killer_guid) : NULLUNIT;
}

void CombatStatusHandler::Vanish(uint32 guidLow)
{
    Unit* pt = m_Unit->GetMapMgr()->GetUnit(guidLow);
    if(pt)
    {
        pt->CombatStatus.m_attackTargets.erase(m_Unit->GetGUID());
        pt->CombatStatus.m_attackers.erase(m_Unit->GetGUID());
        pt->CombatStatus.UpdateFlag();
    }
    DamageMap.erase(guidLow);
}

void CombatStatusHandler::ClearAttackers()
{
    // this is a FORCED function, only use when the reference will be destroyed.
    AttackTMap::iterator itr = m_attackTargets.begin();
    Unit* pt;
    for(; itr != m_attackTargets.end(); itr++)
    {
        pt = m_Unit->GetMapMgr()->GetUnit(itr->first);
        if(pt)
        {
            pt->CombatStatus.m_attackers.erase(m_Unit->GetGUID());
            pt->CombatStatus.UpdateFlag();
        }
    }

    AttackerMap::iterator it2;
    for(it2 = m_attackers.begin(); it2 != m_attackers.end(); it2++)
    {
        pt = m_Unit->GetMapMgr()->GetUnit(*it2);
        if(pt)
        {
            pt->CombatStatus.m_attackTargets.erase(m_Unit->GetGUID());
            pt->CombatStatus.UpdateFlag();
        }
    }

    m_attackers.clear();
    m_attackTargets.clear();
    UpdateFlag();
}

void CombatStatusHandler::ClearHealers()
{
    HealedSet::iterator itr = m_healed.begin();
    Player* pt;
    for(; itr != m_healed.end(); itr++)
    {
        pt = m_Unit->GetMapMgr()->GetPlayer(*itr);
        if(pt)
        {
            pt->CombatStatus.m_healers.erase(m_Unit->GetLowGUID());
            pt->CombatStatus.UpdateFlag();
        }
    }

    for(itr = m_healers.begin(); itr != m_healers.end(); itr++)
    {
        pt = m_Unit->GetMapMgr()->GetPlayer(*itr);
        if(pt)
        {
            pt->CombatStatus.m_healed.erase(m_Unit->GetLowGUID());
            pt->CombatStatus.UpdateFlag();
        }
    }

    m_healed.clear();
    m_healers.clear();
    UpdateFlag();
}

void CombatStatusHandler::OnRemoveFromWorld()
{
    ClearAttackers();
    ClearHealers();
    DamageMap.clear();
}

uint32 Unit::Heal(Unit* target, uint32 SpellId, uint32 amount, bool silent)
{//Static heal
    if(!target || !SpellId || !amount || !target->isAlive() )
        return amount;

    uint32 overheal = 0;
    uint32 th = target->GetUInt32Value(UNIT_FIELD_HEALTH) + amount;
    uint32 mh = target->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(th > mh)
    {
        target->SetUInt32Value(UNIT_FIELD_HEALTH, mh);
        overheal = th - mh;
    }
    else
        target->SetUInt32Value(UNIT_FIELD_HEALTH, th);

    if(!silent)
        Spell::SendHealSpellOnPlayer(this, target, amount, false, overheal, SpellId);
    return overheal;
}

void Unit::Energize(Unit* target, uint32 SpellId, uint32 amount, uint32 type)
{//Static energize
    if( !target || !SpellId )
        return;

    uint32 cm = target->GetUInt32Value(UNIT_FIELD_POWER1+type);
    uint32 mm = target->GetUInt32Value(UNIT_FIELD_MAXPOWER1+type);
    if(mm != cm)
    {
        if( !amount )
            return;

        cm += amount;
        if(cm > mm)
        {
            target->SetUInt32Value(UNIT_FIELD_POWER1+type, mm);
            amount = mm - cm;
        }
        else
            target->SetUInt32Value(UNIT_FIELD_POWER1+type, cm);

        Spell::SendHealManaSpellOnPlayer(this,target, amount, type, SpellId);
        target->SendPowerUpdate();
    }
}

void Unit::InheritSMMods(Unit* inherit_from)
{
    for(uint32 x = 0; x < SPELL_MODIFIERS; x++)
    {
        for(uint32 y = 0; y < 2; y++)
        {
            if(inherit_from->SM[x][y])
            {
                if(SM[x][y] == 0)
                    SM[x][y] = new int32[SPELL_GROUPS];
                memcpy(SM[x][y], inherit_from->SM[x][y], sizeof(int32)*SPELL_GROUPS);
            }
        }
    }
}

void Unit::EventCancelSpell(Spell* ptr)
{
    if(ptr != NULL)
        ptr->cancel();
    if(ptr == m_currentSpell)
        m_currentSpell = NULL;
}

void Unit::EventStrikeWithAbility(uint64 guid, SpellEntry * sp, uint32 damage)
{
    Unit* victim = m_mapMgr ? m_mapMgr->GetUnit(guid) : NULLUNIT;
    if(victim)
        Strike( victim, RANGED, sp, 0, 0, 0, false, true );
}

void Unit::setAttackTimer(int32 time, bool offhand)
{
    if(!time)
        time = offhand ? m_uint32Values[UNIT_FIELD_BASEATTACKTIME + 1] : m_uint32Values[UNIT_FIELD_BASEATTACKTIME];

    time = std::max(1000, float2int32(float(time) * GetFloatValue(UNIT_MOD_CAST_SPEED)));
    if(time > 300000)       // just in case.. shouldn't happen though
        time = offhand ? m_uint32Values[UNIT_FIELD_BASEATTACKTIME+1] : m_uint32Values[UNIT_FIELD_BASEATTACKTIME];

    if(offhand)
        m_attackTimer_1 = getMSTime() + time;
    else
        m_attackTimer = getMSTime() + time;
}

bool Unit::isAttackReady(bool offhand)
{
    if(offhand)
        return (getMSTime() >= m_attackTimer_1) ? true : false;
    else
        return (getMSTime() >= m_attackTimer) ? true : false;
}

void Unit::ReplaceAIInterface(AIInterface *new_interface)
{
    delete m_aiInterface;   //be carefull when you do this. Might screw unit states !
    m_aiInterface = new_interface;
}

void Unit::EventModelChange()
{
    CreatureBoundData *boundData = dbcCreatureBoundData.LookupEntry(GetUInt32Value(UNIT_FIELD_DISPLAYID));
    if(boundData) //TODO: if has mount, grab mount model and add the z value of attachment 0
        m_modelhalfsize = boundData->High[2]/2;
    else
        m_modelhalfsize = 1.0f;
}

bool Unit::HasAurasOfNameHashWithCaster(uint32 namehash, Unit* caster)
{
    if( !namehash )
        return false;

    return m_AuraInterface.HasAurasOfNameHashWithCaster(namehash, caster ? caster->GetGUID() : 0);
}

void Creature::UpdateLootAnimation(Player* Looter)
{
    if( m_loot.HasLoot(Looter) )
    {
        // update players with lootable flags
        for(unordered_set<Player*  >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); itr++)
        {
            Player* plr = *itr;
            if( ( plr->GetLowGUID() == m_taggingPlayer ) ||
                ( plr->GetGroup() != NULL && plr->GetGroupID() == m_taggingGroup ) )
            {
                // only have to do the sparkly animation
                // TODO: do this by loot type for groups
                // switch(m_lootMethod)
                BuildFieldUpdatePacket(plr, UNIT_DYNAMIC_FLAGS, m_uint32Values[UNIT_DYNAMIC_FLAGS] | U_DYN_FLAG_LOOTABLE);
            }
        }
    }
    else
    {
        // we are still alive, probably updating tapped state
        for(unordered_set<Player*  >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); itr++)
        {
            if( !m_taggingPlayer )
            {
                BuildFieldUpdatePacket(*itr, UNIT_DYNAMIC_FLAGS, m_uint32Values[UNIT_DYNAMIC_FLAGS]);
            }
            else if( ( (*itr)->GetLowGUID() == m_taggingPlayer ) ||
                ( (*itr)->GetGroup() != NULL && (*itr)->GetGroup()->GetID() == m_taggingGroup ) )
            {
                // tagger.
                BuildFieldUpdatePacket(*itr, UNIT_DYNAMIC_FLAGS, m_uint32Values[UNIT_DYNAMIC_FLAGS] | U_DYN_FLAG_TAPPED_BY_PLAYER);
            }
            else
            {
                BuildFieldUpdatePacket(*itr, UNIT_DYNAMIC_FLAGS, m_uint32Values[UNIT_DYNAMIC_FLAGS] | U_DYN_FLAG_TAGGED_BY_OTHER);
            }
        }
    }
}

void Creature::ClearTag()
{
    if( isAlive() )
    {
        ClearLoot();
        m_taggingGroup = m_taggingPlayer = 0;
        m_lootMethod = -1;

        // if we are alive, means that we left combat
        if( IsInWorld() )
            UpdateLootAnimation(NULLPLR);
    }
    // dead, don't clear tag
}

void Object::ClearLoot()
{
    // better cancel any rolls just in case.
    for(vector<__LootItem>::iterator itr = m_loot.items.begin(); itr != m_loot.items.end(); itr++)
    {
        if( itr->roll != NULL )
        {
            sEventMgr.RemoveEvents(itr->roll);
            itr->roll = NULLROLL; // buh-bye!
        }
    }

    m_loot.gold = 0;
    m_loot.items.clear();
    m_loot.looters.clear();
}

void Creature::Tag(Player* plr)
{
    // Tagging
    if( m_taggingPlayer != 0 )
        return;

    if(GetCreatureInfo() && GetCreatureInfo()->Type == CRITTER || IsPet())
        return;

    m_taggingPlayer = plr->GetLowGUID();
    m_taggingGroup = plr->m_playerInfo->m_Group ? plr->m_playerInfo->m_Group->GetID() : 0;

    /* set loot method */
    if( plr->GetGroup() != NULL )
        m_lootMethod = plr->GetGroup()->GetMethod();

    // update tag visual
    UpdateLootAnimation(plr);
}

void Unit::SetPower(uint32 type, int32 value)
{
    SetUInt32Value(UNIT_FIELD_POWER1 + type, value);
    SendPowerUpdate(type);
}

void Unit::RemoveStealth()
{
    if( m_stealth != 0 )
    {
        RemoveAura( m_stealth );
        m_stealth = 0;
    }
}

void Unit::RemoveInvisibility()
{
    if( m_invisibility != 0 )
    {
        RemoveAura( m_invisibility );
        m_invisibility = 0;
    }
}

//what is an Immobilize spell ? Have to add it later to spell effect handler
void Unit::EventStunOrImmobilize()
{
    if( trigger_on_stun )
    {
        if( trigger_on_stun_chance < 100 && !Rand( trigger_on_stun_chance ) )
            return;

        CastSpell(TO_UNIT(this), trigger_on_stun, true);
    }
}

void Unit::SetTriggerStunOrImmobilize(uint32 newtrigger,uint32 new_chance)
{
    trigger_on_stun = newtrigger;
    trigger_on_stun_chance = new_chance;
}

void Unit::SendPowerUpdate(int8 power)
{
    uint32 updateCount = 1;
    uint8 PowerType = (power == -1 ? GetPowerType() : power);

    WorldPacket data(SMSG_POWER_UPDATE, 20);
    data << GetNewGUID();
    data << uint32(updateCount); // iteration count
    for (int32 i = 0; i < updateCount; ++i)
    {
        data << uint8(PowerType);
        data << GetUInt32Value(UNIT_FIELD_POWER1+PowerType);
    }
    SendMessageToSet(&data, true);
}

uint32 Unit::DoDamageSplitTarget(uint32 res, uint32 school_type, bool melee_dmg)
{
    Unit* splittarget;
    uint32 splitdamage, tmpsplit;
    DamageSplitTarget * ds = &m_damageSplitTarget;

    splittarget = (GetMapMgr() != NULL) ? GetMapMgr()->GetUnit( ds->m_target ) : NULLUNIT;
    if( splittarget != NULL && res > 0 )
    {
        // calculate damage
        tmpsplit = ds->m_flatDamageSplit;
        if( tmpsplit > res)
            tmpsplit = res; // prevent < 0 damage
        splitdamage = tmpsplit;
        res -= tmpsplit;
        tmpsplit = float2int32( ds->m_pctDamageSplit * res );
        if( tmpsplit > res )
            tmpsplit = res;
        splitdamage += tmpsplit;
        res -= tmpsplit;

        if( splitdamage )
        {
            splittarget->DealDamage(splittarget, splitdamage, 0, 0, 0);

            // Send damage log
            if (melee_dmg)
            {
                dealdamage sdmg;

                sdmg.full_damage = splitdamage;
                sdmg.resisted_damage = 0;
                sdmg.school_type = school_type;
                SendAttackerStateUpdate(splittarget, &sdmg, splitdamage, 0, 0, 0, ATTACK);
            }
            else
            {
                SendSpellNonMeleeDamageLog(TO_UNIT(this), splittarget, ds->m_spellId, splitdamage, school_type, 0, 0, true, 0, 0, true);
            }
        }
    }

    return res;
}

void Unit::RemoveExtraStrikeTarget(SpellEntry *spell_info)
{
    for(std::list<ExtraStrike*>::iterator i = m_extraStrikeTargets.begin();i != m_extraStrikeTargets.end();++i)
    {
        if((*i)->deleted == false && spell_info == (*i)->spell_info)
        {
            m_extrastriketargetc--;
            (*i)->deleted = true;
        }
    }
}

void Unit::AddExtraStrikeTarget(SpellEntry *spell_info, uint32 charges)
{
    for(std::list<ExtraStrike*>::iterator i = m_extraStrikeTargets.begin();i != m_extraStrikeTargets.end();++i)
    {
        //a pointer check or id check ...should be the same
        if(spell_info == (*i)->spell_info)
        {
            if ((*i)->deleted == true)
            {
                (*i)->deleted = false;
                m_extrastriketargetc++;
            }
            (*i)->charges = charges;
            return;
        }
    }

    ExtraStrike *es = NULL;
    es = new ExtraStrike;

    es->spell_info = spell_info;
    es->charges = charges;
    es->deleted = false;
    m_extraStrikeTargets.push_back(es);
    m_extrastriketargetc++;
}

bool Unit::HasProcSpell(uint32 spellid)
{
    std::list<struct ProcTriggerSpell>::iterator itr;
    for(itr = m_procSpells.begin(); itr != m_procSpells.end(); itr++)
    {
        if(itr->spellId == spellid)
            return true;
    }
    return false;
}

void Unit::AddOnAuraRemoveSpell(uint32 NameHash, uint32 procSpell, uint32 procChance, bool procSelf)
{
    HM_NAMESPACE::hash_map<uint32, onAuraRemove*>::iterator itr;
    itr = m_onAuraRemoveSpells.find(NameHash);

    if(itr != m_onAuraRemoveSpells.end())
    {
        itr->second->spell = procSpell;
        itr->second->chance = procChance;
        itr->second->self = procSelf;
        itr->second->deleted = false;
    }
    else
    {
        onAuraRemove *proc = new onAuraRemove;
        proc->spell = procSpell;
        proc->chance = procChance;
        proc->self = procSelf;
        proc->deleted = false;

        m_onAuraRemoveSpells[NameHash] = proc;
    }

}

void Unit::RemoveOnAuraRemoveSpell(uint32 NameHash)
{
    HM_NAMESPACE::hash_map<uint32, onAuraRemove*>::iterator itr;
    itr = m_onAuraRemoveSpells.find(NameHash);

    if(itr != m_onAuraRemoveSpells.end())
        itr->second->deleted = true;
}

// Aura by NameHash has been removed
void Unit::OnAuraRemove(uint32 NameHash, Unit* m_target)
{
    HM_NAMESPACE::hash_map<uint32, onAuraRemove*>::iterator itr;
    itr = m_onAuraRemoveSpells.find(NameHash);

    if(itr != m_onAuraRemoveSpells.end())
    {
        bool apply;

        if (itr->second->deleted == true)
            return;

        if (itr->second->chance != 100)
            apply = RandomUInt(100) < itr->second->chance;
        else
            apply = true;

        if (apply)
        {
            if (itr->second->self)
                CastSpell(TO_UNIT(this), itr->second->spell, true);
            else if (m_target)
                m_target->CastSpell(m_target, itr->second->spell, true);
        }
    }
}

//! Is PVP flagged?
bool Unit::IsPvPFlagged()
{
    return HasByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
}

void Unit::SetPvPFlag()
{
    // reset the timer as well..
    if(IsPlayer())
        TO_PLAYER(this)->StopPvPTimer();

    SetByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
}

//! Removal
void Unit::RemovePvPFlag()
{
    if(IsPlayer())
        TO_PLAYER(this)->StopPvPTimer();
    RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_PVP);
}

void Unit::SetPowerType(uint8 type)
{
    SetByte(UNIT_FIELD_BYTES_0, 3, type);

    if(type == POWER_TYPE_RUNIC && IsPlayer())
    {
        SetFloatValue(PLAYER_RUNE_REGEN_1, 0.100000f);
        SetFloatValue(PLAYER_RUNE_REGEN_1+1, 0.100000f);
        SetFloatValue(PLAYER_RUNE_REGEN_1+2, 0.100000f);
        SetFloatValue(PLAYER_RUNE_REGEN_1+3, 0.100000f);
    }
}

void Unit::setDeathState(DeathState s)
{
    m_deathState = s;
    if(IsCreature())
        TO_CREATURE(this)->CreatureSetDeathState(s);
    if(IsVehicle())
        TO_VEHICLE(this)->VehicleSetDeathState(s);
}

//  custom functions for scripting
void Unit::SetWeaponDisplayId(uint8 slot, uint32 ItemId)
{
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+slot, ItemId);
}

bool Unit::IsFFAPvPFlagged()
{
    return HasByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_FFA_PVP);
}

void Unit::SetFFAPvPFlag()
{
    if(IsFFAPvPFlagged()) return;

    SetByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_FFA_PVP);
}

void Unit::RemoveFFAPvPFlag()
{
    if(!IsFFAPvPFlagged()) return;

    RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, U_FIELD_BYTES_FLAG_FFA_PVP);
}

void Unit::OnPositionChange()
{
    if (GetVehicle() != NULL && GetVehicle()->GetControllingUnit() == TO_UNIT(this) && (m_position != GetVehicle()->GetPosition() || GetOrientation() != GetVehicle()->GetOrientation())) //check orientation too since == operator of locationvector doesnt
    {
        GetVehicle()->MoveVehicle(GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
    }
}

void Unit::Dismount()
{
    if(IsPlayer())
    {
        Player* plr = TO_PLAYER(this);
        if( plr->m_MountSpellId )
        {
            m_AuraInterface.RemoveAllAuras( plr->m_MountSpellId);
            plr->m_MountSpellId = 0;
        }

        if( plr->m_FlyingAura )
        {
            m_AuraInterface.RemoveAllAuras( plr->m_FlyingAura);
            plr->m_FlyingAura = 0;
            plr->SetUInt32Value( UNIT_FIELD_DISPLAYID, plr->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
        }
    }
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI );
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER );
    EventModelChange();
}

void Unit::SetFaction(uint32 faction, bool save)
{
    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, faction);
    _setFaction();

    if(save == false)
        return;

    if(IsCreature() && TO_CREATURE(this)->m_spawn)
        TO_CREATURE(this)->SaveToDB();
}

void Unit::ResetFaction()
{
    uint32 faction = 35;
    if(IsPlayer())
    {
        faction = TO_PLAYER(this)->GetInfo()->factiontemplate;
    }
    else
    {
        CreatureProto* cp = CreatureProtoStorage.LookupEntry(GetEntry());
        faction = cp->Faction;
    }
    SetFaction(faction);
}

void Unit::knockback(int32 basepoint, uint32 miscvalue, bool disengage )
{
    float dx, dy;
    float value1 = float( basepoint );
    float value2 = float( miscvalue );
    float proportion;
    float multiplier;

    if( disengage )
        multiplier = -1.0f;
    else
        multiplier = 1.0f;

    if( value2 != 0 )
        proportion = value1 / value2;
    else
        proportion = 0;

    if(proportion)
    {
        value1 = value1 / (10 * proportion);
        value2 = value2 / 10 * proportion;
    }
    else
    {
        value2 = value1 / 10;
        value1 = 0.1f;
    }

    dx = sinf( GetOrientation() );
    dy = cosf( GetOrientation() );

    if( IsCreature() )
    {
        float x = GetPositionX() + (value1 * dx);
        float y = GetPositionY() + (value1 * dy);
        float z = GetPositionZ();
        float dist = CalcDistance(x, y, z);
        uint32 movetime = GetAIInterface()->GetMovementTime(dist);
        GetAIInterface()->SendMoveToPacket( x, y, z, 0.0f, movetime, MONSTER_MOVE_FLAG_JUMP );
        SetPosition(x, y, z, 0.0f);
        GetAIInterface()->StopMovement(movetime,false);

        if (GetCurrentSpell() != NULL)
            GetCurrentSpell()->cancel();
    }
    else if(IsPlayer())
    {
        WorldPacket data(SMSG_MOVE_KNOCK_BACK, 50);
        data << GetNewGUID();
        data << uint32( getMSTime() );
        data << float( multiplier * dy );
        data << float( multiplier * dx );
        data << float( value1 );
        data << float( -value2 );
        Player *plr = TO_PLAYER(this);
        plr->GetSession()->SendPacket( &data );
        plr->ResetHeartbeatCoords();
        plr->DelaySpeedHack(5000);
        plr->GetSession()->m_isKnockedback = true;
        if(plr->m_FlyingAura)
        {
            plr->DisableFlight();
            sEventMgr.AddEvent( this, &Unit::EventRegainFlight, EVENT_REGAIN_FLIGHT, 5000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
    }
}

void Unit::Teleport(float x, float y, float z, float o, int32 phasemask)
{
    if(IsPlayer())
        TO_PLAYER(this)->SafeTeleport(GetMapId(), GetInstanceID(), x, y, z, o, phasemask);
    else
    {
        WorldPacket data(SMSG_MONSTER_MOVE, 50);
        data << GetNewGUID();
        data << uint8(0);
        data << GetPositionX();
        data << GetPositionY();
        data << GetPositionZ();
        data << getMSTime();
        data << uint8(0x0);
        data << uint32(256);
        data << uint32(1);
        data << uint32(1);
        data << x << y << z;
        SendMessageToSet(&data, true);
        SetPosition( x, y, z, o );
        if(phasemask != GetPhaseMask())
            SetPhaseMask(phasemask);
    }
}

void Unit::SetRedirectThreat(Unit * target, float amount, uint32 Duration)
{
    mThreatRTarget = target;
    mThreatRAmount = amount;
    if(Duration)
        sEventMgr.AddEvent( this, &Unit::EventResetRedirectThreat, EVENT_RESET_REDIRECT_THREAT, Duration, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Unit::EventResetRedirectThreat()
{
    mThreatRTarget = NULLUNIT;
    mThreatRAmount = 0.0f;
    sEventMgr.RemoveEvents(this,EVENT_RESET_REDIRECT_THREAT);
}

void Unit::SetSpeed(uint8 SpeedType, float value)
{
    if( value < 0.1f )
        value = 0.1f;

    WorldPacket data(SMSG_FORCE_RUN_SPEED_CHANGE, 400);

    if( SpeedType != SWIMBACK )
    {
        data << GetNewGUID();
        data << uint32(0);
        if( SpeedType == RUN )
            data << uint8(1);

        data << value;
    }
    else
    {
        data << GetNewGUID();
        data << uint32(0);
        data << uint8(0);
        data << uint32(getMSTime());
        data << m_position.x;
        data << m_position.y;
        data << m_position.z;
        data << m_position.o;
        data << uint32(0);
        data << value;
    }

    switch(SpeedType)
    {
    case RUN:
        {
            data.SetOpcode(SMSG_FORCE_RUN_SPEED_CHANGE);
            m_runSpeed = value;
        }break;
    case RUNBACK:
        {
            data.SetOpcode(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            m_backWalkSpeed = value;
        }break;
    case SWIM:
        {
            data.SetOpcode(SMSG_FORCE_SWIM_SPEED_CHANGE);
            m_swimSpeed = value;
        }break;
    case SWIMBACK:
        {
            data.SetOpcode(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            m_backSwimSpeed = value;
        }break;
    case TURN:
        {
            data.SetOpcode(SMSG_FORCE_TURN_RATE_CHANGE);
            m_turnRate = value;
        }break;
    case FLY:
        {
            data.SetOpcode(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            m_flySpeed = value;
        }break;
    case FLYBACK:
        {
            data.SetOpcode(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            m_backFlySpeed = value;
        }break;
    case PITCH_RATE:
        {
            data.SetOpcode(SMSG_FORCE_PITCH_RATE_CHANGE);
            m_pitchRate = value;
        }break;
    default:
        return;
    }
    SendMessageToSet(&data , true);
}

void Unit::SendHeartBeatMsg( bool toself )
{
    WorldPacket data(MSG_MOVE_HEARTBEAT, 64);
    data << GetNewGUID();
    movement_info.write(data);
    SendMessageToSet(&data, toself);
}

uint32 Unit::GetCreatureType()
{
    if(IsCreature())
    {
        CreatureInfo * ci = TO_CREATURE(this)->GetCreatureInfo();
        if(ci && ci->Type)
            return ci->Type;
        else
            return 0;
    }
    if(IsPlayer())
    {
        Player *plr = TO_PLAYER(this);
        if(plr->GetShapeShift())
        {
            SpellShapeshiftFormEntry* ssf = dbcSpellShapeshiftForm.LookupEntry(plr->GetShapeShift());
            if(ssf && ssf->creatureType)
                return ssf->creatureType;
            else
                return 0;
        }
    }
    return 0;
}

void Unit::RemovePassenger(Unit* pPassenger)
{
    if(IsVehicle())
        TO_VEHICLE(this)->RemovePassenger(pPassenger);
    if(IsPlayer())
        TO_PLAYER(this)->RemovePassenger(pPassenger);
}

void Unit::ChangeSeats(Unit* pPassenger, uint8 seatid)
{
    if(IsVehicle())
        TO_VEHICLE(this)->ChangeSeats(pPassenger, seatid);
    if(IsPlayer())
        TO_PLAYER(this)->ChangeSeats(pPassenger, seatid);
}

/* This function changes a vehicles position server side to
keep us in sync with the client, so that the vehicle doesn't
get dismissed because the server thinks its gone out of range
of its passengers*/
void Unit::MoveVehicle(float x, float y, float z, float o) //thanks andy
{
    SetPosition(x, y, z, o);
    for(uint8 i = 0; i < 8; i++)
    {
        if(m_passengers[i] != NULL)
        {
            m_passengers[i]->SetPosition(x,y,z,o);
        }
    }
}

int8 Unit::GetPassengerSlot(Unit* pPassenger)
{
    for(uint8 i = 0; i < 9; i++)
    {
        if( m_passengers[i] == pPassenger ) // Found a slot
        {
            return i;
            break;
        }
    }
    return -1;
}

void Unit::DeletePassengerData(Unit* pPassenger)
{
    uint8 slot = pPassenger->GetSeatID();
    pPassenger->SetSeatID(NULL);
    m_passengers[slot] = NULL;
}

bool Unit::CanEnterVehicle(Player * requester)
{
    if(requester == NULL || !requester->IsInWorld())
        return false;

    if(GetInstanceID() != requester->GetInstanceID())
        return false;

    if(!isAlive() || !requester->isAlive())
        return false;

    if(requester->CalcDistance(this) >= GetModelHalfSize()+5.0f)
        return false;

    if(sFactionSystem.isHostile(this, requester))
        return false;

    if(requester->m_CurrentCharm)
        return false;

    if(requester->m_isGmInvisible)
    {
        sChatHandler.GreenSystemMessage(requester->GetSession(), "Please turn off invis before entering vehicle.");
        return false;
    }

    if(IsVehicle())
    {
        Vehicle *v = TO_VEHICLE(this);
        if(!v->GetMaxPassengerCount())
            return false;

        if(!v->GetMaxSeat())
            return false;

        if(v->IsFull())
            return false;

        if( sEventMgr.HasEvent( v, EVENT_VEHICLE_SAFE_DELETE ) )
            return false;

        if(GetControllingPlayer())
        {
            Player * p = GetControllingPlayer();
            if(p->GetGroup() == NULL)
                return false;
            if(!p->GetGroup()->HasMember(requester))
                return false;
        }
    }

    if(IsPlayer())
    {
        Player * p = TO_PLAYER(this);

        if(p->GetVehicleEntry() == 0)
            return false;
        if(p->GetGroup() == NULL)
            return false;
        if(!p->GetGroup()->HasMember(requester))
            return false;
    }

    return true;
}

bool Unit::IsSitting()
{
    uint8 s = getStandState();
    return
    s == STANDSTATE_SIT_CHAIR || s == STANDSTATE_SIT_LOW_CHAIR  ||
    s == STANDSTATE_SIT_MEDIUM_CHAIR || s == STANDSTATE_SIT_HIGH_CHAIR ||
    s == STANDSTATE_SIT;
}
