/***
 * Demonstrike Core
 */

#include "StdAfx.h"

static const uint32 DKNodesMask[12] = {0xFFFFFFFF,0xF3FFFFFF,0x317EFFFF,0,0x2004000,0x1400E0,0xC1C02014,0x12018,0x380,0x4000C10,0,0};//all old continents are available to DK's by default.

Player::Player(uint64 guid, uint32 fieldCount) : m_playerInfo(NULL), Unit(guid, fieldCount), m_talentInterface(this), m_inventory(this), m_bank(this), m_currency(this)
{
    SetTypeFlags(TYPEMASK_TYPE_PLAYER);

    m_runemask = 0x3F;
    m_bgRatedQueue = false;
    m_massSummonEnabled = false;
    m_deathRuneMasteryChance = 0;
    itemBonusMask.SetCount(ITEM_STAT_MAXIMUM);
}

Player::~Player ( )
{

}

void Player::Init()
{
    Unit::Init();
    m_lastAreaUpdateMap             = -1;
    m_oldZone                       = 0;
    m_oldArea                       = 0;
    m_mailBox                       = new Mailbox( GetUInt32Value(OBJECT_FIELD_GUID) );
    m_bgSlot                        = 0;
    m_feralAP                       = 0;
    m_finishingmovesdodge           = false;
    iActivePet                      = 0;
    resurrector                     = 0;
    SpellCrtiticalStrikeRatingBonus = 0;
    SpellHasteRatingBonus           = 0;
    m_lifetapbonus                  = 0;
    info                            = NULL;
    SoulStone                       = 0;
    SoulStoneReceiver               = 0;
    bReincarnation                  = false;
    m_session                       = 0;
    TrackingSpell                   = 0;
    m_status                        = 0;
    m_ShapeShifted                  = 0;
    m_curSelection                  = 0;
    m_lootGuid                      = 0;
    m_Summon                        = NULL;
    hasqueuedpet                    = false;
    m_hasInRangeGuards              = 0;
    m_PetNumberMax                  = 0;
    m_lastShotTime                  = 0;
    m_H_regenTimer                  = 0;
    m_P_regenTimer                  = 0;
    m_onTaxi                        = false;
    m_taxi_pos_x                    = 0;
    m_taxi_pos_y                    = 0;
    m_taxi_pos_z                    = 0;
    m_taxi_ride_time                = 0;
    m_meleeattackspeedmod           = 1.0f;
    m_rangedattackspeedmod          = 1.0f;
    m_cheatDeathRank                = 0;
    ForceSaved                      = false;
    m_nextSave                      = 120000;
    m_currentSpell                  = NULL;
    m_resurrectHealth               = 0;
    m_resurrectMana                 = 0;
    m_GroupInviter                  = 0;
    Lfgcomment                      = "";
    m_lastWarnCounter               = 0;
    for(uint8 i = 0; i < 3; i++)
    {
        LfgType[i]      = 0;
        LfgDungeonId[i] = 0;
    }
    OnlineTime                      = 0;
    load_health                     = 0;
    for(uint8 i = 0; i < POWER_TYPE_MAX; i++)
        load_power[i]               = 0;
    m_Autojoin                      = false;
    m_AutoAddMem                    = false;
    LfmDungeonId                    = 0;
    LfmType                         = 0;
    m_invitersGuid                  = 0;
    forget                          = 0;
    m_invitersGuid                  = 0;
    ResetTradeVariables();
    DuelingWith                     = NULL;
    m_duelCountdownTimer            = 0;
    m_duelStatus                    = 0;
    m_duelState                     = DUEL_STATE_FINISHED;
    waypointunit                    = NULL;
    m_lootGuid                      = 0;
    m_banned                        = false;
    //Bind possition
    m_bind_pos_x                    = 0;
    m_bind_pos_y                    = 0;
    m_bind_pos_z                    = 0;
    m_bind_mapid                    = 0;
    m_bind_zoneid                   = 0;
    m_timeLogoff                    = 0;
    m_isResting                     = 0;
    m_restState                     = 0;
    m_restAmount                    = 0;
    LastAreaTrigger                 = NULL;
    m_afk_reason                    = "";
    m_AllowAreaTriggerPort  = true;
    m_bgEntryPointMap               = 0;
    m_bgEntryPointX                 = 0;
    m_bgEntryPointY                 = 0;
    m_bgEntryPointZ                 = 0;
    m_bgEntryPointO                 = 0;
    ReclaimCount                    = 0;
    for(uint8 i = 0; i < 2; i++)
    {
        m_bgQueueType[i] = 0;
        m_bgQueueInstanceId[i] = 0;
        m_bgIsQueued[i] = false;
        m_bgQueueTime[i] = 0;
    }
    m_bg                            = NULL;
    m_bgHasFlag                     = false;
    m_bgEntryPointInstance          = 0;
    bGMTagOn                        = false;
    vendorpass_cheat                = false;
    ignoreitemreq_cheat             = false;
    DisableDevTag                   = false;
    CooldownCheat                   = false;
    CastTimeCheat                   = false;
    PowerCheat                      = false;
    FlyCheat                        = false;
    weapon_proficiency              = 0x4000;//2^14
    armor_proficiency               = 1;
    m_LastUnderwaterState           = 0;
    m_MirrorTimer[0]                = -1;
    m_MirrorTimer[1]                = -1;
    m_MirrorTimer[2]                = -1;
    m_UnderwaterTime                = 180000;
    m_AutoShotTarget                = 0;
    m_onAutoShot                    = false;
    m_AutoShotDuration              = 0;
    m_AutoShotAttackTimer           = 0;
    m_AutoShotSpell                 = NULL;
    m_AttackMsgTimer                = 0;
    m_GM_SelectedGO                 = NULL;

    m_regenTimerCount = 0;
    for (uint8 i = 0; i < 6; i++)
        m_powerFraction[i] = 0;
    sentMOTD                        = false;
    m_targetIcon                    = 0;
    m_MountSpellId                  = 0;
    bHasBindDialogOpen              = false;
    m_CurrentCharm                  = NULL;
    m_CurrentTransporter            = NULL;
    m_SummonedObject                = NULL;
    m_currentLoot                   = (uint64)NULL;
    roll                            = 0;
    mUpdateDataCount                = 0;
    mOutOfRangeIdCount              = 0;
    bUpdateDataBuffer.reserve(65000);
    mOutOfRangeIds.reserve(1000);
    bProcessPending                 = false;

    for(uint8 i = 0; i < QUEST_LOG_COUNT; i++)
        m_questlog[i] = NULL;

    CurrentGossipMenu               = NULL;
    rageFromDamageDealt             = 0;
    m_attacking                     = false;
    myCorpse                        = NULL;
    blinked                         = false;
    blinktimer                      = getMSTime();
    m_explorationTimer              = getMSTime();
    linkTarget                      = NULL;
    stack_cheat                     = false;
    triggerpass_cheat               = false;
    m_pvpTimer                      = 0;
    m_globalCooldown                = 0;
    m_lastHonorResetTime            = 0;
    m_TeleportState                 = 1;
    m_beingPushed                   = false;
    m_FlyingAura                    = 0;
    rename_pending                  = false;
    titanGrip                       = false;
    iInstanceType                   = 0;
    iRaidType                       = 0;
    m_XPoff                         = false;
    customizable                    = false;
    memset(reputationByListId, 0, sizeof(FactionReputation*) * 128);
    AnnihilationProcChance          = 0;

    for(uint8 i = 0; i < 6; i++)
        m_runes[i] = baseRunes[i];

    JudgementSpell                  = 0;
    ok_to_remove                    = false;
    trigger_on_stun                 = 0;
    trigger_on_stun_chance          = 100;
    m_rap_mod_pct                   = 0;
    m_summoner                      = NULL;
    m_summonInstanceId              = m_summonMapId = 0;
    m_tempSummon                    = NULL;
    m_pendingBattleground[0]        = NULL;
    m_pendingBattleground[1]        = NULL;
    m_deathVision                   = false;
    m_speedChangeCounter            = 1;
    memset(&m_bgScore,0,sizeof(BGScore));
    m_arenateaminviteguid           = 0;
    raidgrouponlysent               = false;
    m_setwaterwalk                  = false;
    m_areaSpiritHealer_guid         = 0;
    m_CurrentTaxiPath               = NULL;
    m_lfgMatch                      = NULL;
    m_lfgInviterGuid                = 0;
    m_mountCheckTimer               = 0;
    m_taxiMapChangeNode             = 0;
    m_safeFall                      = 0;
    safefall                        = false;
    m_KickDelay                     = 0;
    m_passOnLoot                    = false;
    m_changingMaps                  = true;
    m_mageInvisibility              = false;
    m_bgFlagIneligible              = 0;
    m_insigniaTaken                 = true;
    m_BeastMaster                   = false;

    watchedchannel                  = NULL;
    PreventRes                      = false;
    MobXPGainRate                   = 0.0f;
    NoReagentCost                   = false;
    fromrandombg                    = false;
    randombgwinner                  = false;
    m_drunkTimer                    = 0;
    m_drunk                         = 0;

    m_QuestGOInProgress.clear();
    m_completedQuests.clear();
    m_completedDailyQuests.clear();
    quest_spells.clear();
    quest_mobs.clear();
    OnMeleeAuras.clear();
    m_Pets.clear();
    m_channels.clear();
    m_channelsbyDBCID.clear();
    m_visibleObjects.clear();
    mSpells.clear();
}

void Player::Destruct()
{
    sEventMgr.RemoveEvents(castPtr<Player>(this));

    if(!ok_to_remove)
    {
        printf("Player deleted from non-logoutplayer!\n");
        OutputCrashLogLine("Player deleted from non-logoutplayer!");
        CThreadPool::Suicide();
    }

    objmgr.RemovePlayer(this);

    if(m_session)
    {
        m_session->SetPlayer(NULL);
    }

    Player* pTarget = objmgr.GetPlayer(GetInviter());
    if(pTarget)
        pTarget->SetInviter(0);
    pTarget = NULL;

    if(m_tradeData)
    {
        if((pTarget = GetTradeTarget()) && pTarget->m_tradeData)
        {
            delete pTarget->m_tradeData;
            pTarget->m_tradeData = NULL;
        }
        delete m_tradeData;
        m_tradeData = NULL;
    }

    if(m_Summon)
    {
        m_Summon->Dismiss(true);
        m_Summon->ClearPetOwner();
    }

    if (m_GM_SelectedGO)
        m_GM_SelectedGO = NULL;

    if(m_SummonedObject)
    {
        if(m_SummonedObject->GetInstanceID() == GetInstanceID())
        {
            if(m_SummonedObject->IsInWorld())
                m_SummonedObject->RemoveFromWorld(true);
            m_SummonedObject->Destruct();
        } else sEventMgr.AddEvent(m_SummonedObject, &GameObject::Despawn, uint32(0), uint32(0), EVENT_GAMEOBJECT_EXPIRE, 100, 1,0);
        m_SummonedObject = NULL;
    }

    if( m_mailBox )
        delete m_mailBox;

    if( DuelingWith != NULL )
        DuelingWith->DuelingWith = NULL;
    DuelingWith = NULL;

    CleanupGossipMenu();
    ASSERT(!IsInWorld());

    // delete m_talenttree

    CleanupChannels();

    for(int i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if(m_questlog[i] != NULL)
        {
            delete m_questlog[i];
            m_questlog[i] = NULL;
        }
    }

    if(m_reputation.size())
    {
        for(ReputationMap::iterator itr = m_reputation.begin(); itr != m_reputation.end(); itr++)
            delete itr->second;
    }

    if(m_playerInfo)
        m_playerInfo->m_loggedInPlayer=NULL;

    if( !delayedPackets.empty() )
    {
        while( delayedPackets.size() )
        {
            WorldPacket * pck = delayedPackets.next();
            delete pck;
        }
    }

    //  SetSession(NULL);
    if(myCorpse)
        myCorpse = NULL;

    if(linkTarget)
        linkTarget = NULL;

    m_QuestGOInProgress.clear();
    m_completedQuests.clear();
    quest_spells.clear();
    quest_mobs.clear();
    OnMeleeAuras.clear();

    for(std::map<uint32, PlayerPet*>::iterator itr = m_Pets.begin(); itr != m_Pets.end(); itr++)
        delete itr->second;
    while(!m_loadAuras.empty())
    {
        Aura *aur = m_loadAuras.front();
        m_loadAuras.pop_front();
        delete aur;
    }

    m_Pets.clear();
    m_channels.clear();
    m_channelsbyDBCID.clear();
    mSpells.clear();
    Unit::Destruct();
}

void Player::Update( uint32 p_time )
{
    if(!IsInWorld())
        return;

    Unit::Update( p_time );
    if (isAlive())
    {
        m_P_regenTimer += p_time;
        if(p_time < m_p_DelayTimer)
        {
            m_p_DelayTimer -= p_time;
            PlayerRegeneratePower(true);
        }
        else
        {
            m_p_DelayTimer = 0;
            PlayerRegeneratePower(false);
        }
        m_P_regenTimer = 0;
    }

    // Handle our water stuff
    HandleBreathing(p_time);

    // Autosave
    if(m_nextSave > p_time)
        m_nextSave -= p_time;
    else
    {
        m_nextSave = 0;
        SaveToDB(false);
    }

    if(m_CurrentTransporter && !m_movementInterface.isTransportLocked())
    {
        // Update our position, using trnasporter X/Y
        float c_tposx, c_tposy, c_tposz, c_tposo;
        m_movementInterface.GetTransportPosition(c_tposx, c_tposy, c_tposz, c_tposo);
        c_tposx += m_CurrentTransporter->GetPositionX();
        c_tposy += m_CurrentTransporter->GetPositionY();
        c_tposz += m_CurrentTransporter->GetPositionZ();
        SetPosition(c_tposx, c_tposy, c_tposz, c_tposo);
    }

    if(GetVehicle())
    {
        // Update our position
        float vposx = GetVehicle()->GetPositionX();
        float vposy = GetVehicle()->GetPositionY();
        float vposz = GetVehicle()->GetPositionZ();
        float vposo = GetVehicle()->GetOrientation();
        GetVehicle()->MoveVehicle(vposx, vposy, vposz, vposo);
    }

    if(m_pvpTimer)
    {
        if(p_time >= m_pvpTimer)
            m_pvpTimer -= p_time;
        else m_pvpTimer = 0;
        if(m_pvpTimer == 0 || !IsPvPFlagged())
        {
            StopPvPTimer();
            RemovePvPFlag();    // Reset Timer Status
        }
    }

    // Exploration
    if(IsInWorld())
    {
        m_explorationTimer += p_time;
        if(m_explorationTimer >= 1500)
        {
            _EventExploration();
            m_explorationTimer = 0;
        }
    }

    if (m_drunk)
    {
        m_drunkTimer += p_time;

        if (m_drunkTimer > 10*1000)
            EventHandleSobering();
    }
}

void Player::OnFieldUpdated(uint32 index)
{
    Group *group;
    if((group = GetGroup()) && IsInWorld())
        group->HandleUpdateFieldChange(index, this);

    Unit::OnFieldUpdated(index);
}

void Player::UpdateFieldValues()
{
    UpdateStatValues();
    UpdatePlayerRatings();

    Unit::UpdateFieldValues();

    m_needStatRecalculation = m_statValuesChanged = false;
    m_AuraInterface.ClearModMaskBits();
    itemBonusMask.Clear();
}

static uint32 ratingsToModBonus[26] = 
{
    0,
    ITEM_STAT_DEFENSE_RATING,
    ITEM_STAT_DODGE_RATING,
    ITEM_STAT_PARRY_RATING,
    ITEM_STAT_BLOCK_RATING,
    ITEM_STAT_MELEE_HIT_RATING,
    ITEM_STAT_RANGED_HIT_RATING,
    ITEM_STAT_SPELL_HIT_RATING,
    ITEM_STAT_MELEE_CRITICAL_STRIKE_RATING,
    ITEM_STAT_RANGED_CRITICAL_STRIKE_RATING,
    ITEM_STAT_SPELL_CRITICAL_STRIKE_RATING,
    ITEM_STAT_MELEE_HIT_AVOIDANCE_RATING,
    ITEM_STAT_RANGED_HIT_AVOIDANCE_RATING,
    ITEM_STAT_SPELL_HIT_AVOIDANCE_RATING,
    ITEM_STAT_MELEE_RESILIENCE,
    ITEM_STAT_RANGED_RESILIENCE,
    ITEM_STAT_SPELL_RESILIENCE,
    ITEM_STAT_MELEE_HASTE_RATING,
    ITEM_STAT_RANGED_HASTE_RATING,
    ITEM_STAT_SPELL_HASTE_RATING,
    0,
    0,
    0,
    ITEM_STAT_EXPERTISE_RATING,
    ITEM_STAT_ARMOR_PENETRATION_RATING,
    ITEM_STAT_MASTERY_RATING
};

int32 Player::CalculatePlayerCombatRating(uint8 combatRating)
{
    ASSERT(combatRating < 26);

    int32 val = 0;
    if(ratingsToModBonus[combatRating])
        val += GetBonusesFromItems(ratingsToModBonus[combatRating]);
    switch(combatRating)
    {
    case 5: case 6: case 7:
        {   // Add base hit to our individual ratings
            val += GetBonusesFromItems(ITEM_STAT_HIT_RATING);
        }break;
    case 8: case 9: case 10:
        {   // Add base crit to our individual ratings
            val += GetBonusesFromItems(ITEM_STAT_CRITICAL_STRIKE_RATING);
        }break;
    case 11: case 12: case 13:
        {
            // TODO: Hit avoidance rating is a bit harder to do
            val += GetBonusesFromItems(ITEM_STAT_HIT_REDUCTION_RATING);
        }break;
    case 14: case 15: case 16:
        {   // Add base resilience to our individual res ratings
            val += GetBonusesFromItems(ITEM_STAT_RESILIENCE_RATING);
            // Also add crit reduction rating, though it's not entirely used
            val += GetBonusesFromItems(ITEM_STAT_CRITICAL_REDUCTION_RATING);
        }break;
    case 17: case 18: case 19:
        {   // Add base haste to our individual haste ratings
            val += GetBonusesFromItems(ITEM_STAT_HASTE_RATING);
        }break;
    case 20: case 21: case 22:
        {
            Item *item = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+(combatRating-20));
            if(item)
            {
                AuraInterface::modifierMap ratingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RATING);
                for(AuraInterface::modifierMap::iterator itr = ratingMod.begin(); itr != ratingMod.end(); itr++)
                {
                    // Weapon ratings are 0x01, so skip them here
                    if((itr->second->m_miscValue[0] & 0x1) == 0)
                        continue;
                    if(itr->second->m_spellInfo->EquippedItemSubClass & (1<<item->GetProto()->SubClass))
                        val += itr->second->m_amount;
                }
            }
        }break;
    }

    return val;
}

bool Player::CombatRatingUpdateRequired(uint32 combatRating)
{
    bool res = (m_statValuesChanged && m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RATING_FROM_STAT).size());
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_RATING);
    res |= m_AuraInterface.GetModMaskBit(SPELL_AURA_MOD_RATING_FROM_STAT);
    if(ratingsToModBonus[combatRating])
        res |= itemBonusMask.GetBit(ratingsToModBonus[combatRating]);
    switch(combatRating)
    {
    case 5: case 6: case 7:
        {
            res |= itemBonusMask.GetBit(ITEM_STAT_HIT_RATING);
        }break;
    case 8: case 9: case 10:
        {
            res |= itemBonusMask.GetBit(ITEM_STAT_CRITICAL_STRIKE_RATING);
        }break;
    case 11: case 12: case 13:
        {
            res |= itemBonusMask.GetBit(ITEM_STAT_HIT_REDUCTION_RATING);
        }break;
    case 14: case 15: case 16:
        {
            res |= itemBonusMask.GetBit(ITEM_STAT_RESILIENCE_RATING);
            res |= itemBonusMask.GetBit(ITEM_STAT_CRITICAL_REDUCTION_RATING);
        }break;
    case 17: case 18: case 19:
        {
            res |= itemBonusMask.GetBit(ITEM_STAT_HASTE_RATING);
        }break;
    }

    return res;
}

float Player::GetRatioForCombatRating(uint32 combatRating)
{
    ASSERT(combatRating<26);
    float value = 0.f;
    uint32 reallevel = getLevel();
    uint32 level = reallevel > MAXIMUM_ATTAINABLE_LEVEL ? MAXIMUM_ATTAINABLE_LEVEL : reallevel;
    if(gtFloat * pDBCEntry = dbcCombatRating.LookupEntry( combatRating * 100 + level - 1 ))
        value = pDBCEntry->val;
    if(value == 0.f) value=1.f;
    return value;
}

void Player::UpdatePlayerRatings()
{
    for(uint32 cr = 1, index = PLAYER_RATING_MODIFIER_DEFENCE; index < PLAYER_RATING_MODIFIER_MAX; cr++, index++)
    {
        if(!CombatRatingUpdateRequired(cr))
            continue;
        uint32 val = CalculatePlayerCombatRating(cr);
        AuraInterface::modifierMap ratingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RATING);
        for(AuraInterface::modifierMap::iterator itr = ratingMod.begin(); itr != ratingMod.end(); itr++)
        {
            // Weapon ratings are 0x01, so skip them here
            if(itr->second->m_miscValue[0] & 0x1)
                continue;
            if(itr->second->m_miscValue[0] & (1<<cr))
                val += itr->second->m_amount;
        }
        ratingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RATING_FROM_STAT);
        for(AuraInterface::modifierMap::iterator itr = ratingMod.begin(); itr != ratingMod.end(); itr++)
            if(itr->second->m_miscValue[0] & (1<<cr))
                val += (float(GetStat(itr->second->m_miscValue[1]))*float(itr->second->m_amount/100.f));

        // Now that we have the calculated value, set it for player
        SetUInt32Value(index, val);
        // Multiply the overall rating with the set ratio
        float rating = float(float(val)/GetRatioForCombatRating(cr));
    }
}

static uint32 statToModBonus[MAX_STAT] = 
{
    ITEM_STAT_STRENGTH,
    ITEM_STAT_AGILITY,
    ITEM_STAT_STAMINA,
    ITEM_STAT_INTELLECT,
    ITEM_STAT_SPIRIT
};

bool Player::StatUpdateRequired()
{
    bool res = false;
    for(uint8 i = 0; i < MAX_STAT; i++)
        res |= itemBonusMask.GetBit(statToModBonus[i]);
    return res || Unit::StatUpdateRequired();
}

bool Player::HealthUpdateRequired()
{
    bool res = false;
    res |= itemBonusMask.GetBit(ITEM_STAT_HEALTH);
    return res || Unit::HealthUpdateRequired();
}

bool Player::PowerUpdateRequired()
{
    bool res = false;
    res |= itemBonusMask.GetBit(ITEM_STAT_POWER);
    return res || Unit::PowerUpdateRequired();
}

bool Player::AttackTimeUpdateRequired(uint8 weaponType)
{
    bool res = false;
    res |= itemBonusMask.GetBit(ITEM_STAT_CUSTOM_WEAPON_DELAY);
    res |= itemBonusMask.GetBit(ITEM_STAT_MELEE_HASTE_RATING);
    return res || Unit::AttackTimeUpdateRequired(weaponType);
}

bool Player::APUpdateRequired()
{
    bool res = false;
    res |= itemBonusMask.GetBit(ITEM_STAT_ATTACK_POWER);
    return res || Unit::APUpdateRequired();
}

bool Player::RAPUpdateRequired()
{
    bool res = false;
    res |= itemBonusMask.GetBit(ITEM_STAT_RANGED_ATTACK_POWER);
    res |= itemBonusMask.GetBit(ITEM_STAT_ATTACK_POWER);
    return res || Unit::RAPUpdateRequired();
}

static uint32 schooltoResModBonus[MAX_RESISTANCE] =
{
    ITEM_STAT_PHYSICAL_RESISTANCE,
    ITEM_STAT_HOLY_RESISTANCE,
    ITEM_STAT_FIRE_RESISTANCE,
    ITEM_STAT_NATURE_RESISTANCE,
    ITEM_STAT_FROST_RESISTANCE,
    ITEM_STAT_SHADOW_RESISTANCE,
    ITEM_STAT_ARCANE_RESISTANCE
};

bool Player::ResUpdateRequired()
{
    bool res = false;
    for(uint8 i = 0; i < MAX_RESISTANCE; i++)
        res |= itemBonusMask.GetBit(schooltoResModBonus[i]);
    return res || Unit::ResUpdateRequired();
}

int32 Player::GetBonusMana()
{
    return GetBonusesFromItems(ITEM_STAT_HEALTH);
}

int32 Player::GetBonusHealth()
{
    return GetBonusesFromItems(ITEM_STAT_HEALTH);
}

int32 Player::GetBonusStat(uint8 stat)
{
    return GetBonusesFromItems(statToModBonus[stat]);
}

int32 Player::GetBonusResistance(uint8 school)
{
    return GetBonusesFromItems(schooltoResModBonus[school]);
}

int32 Player::GetBaseAttackTime(uint8 weaponType)
{
    int32 speed = 0;
    if(weaponType == 0)
        speed = 2000;
    if( GetShapeShift() == FORM_CAT && weaponType != 2 )
        speed = 1500;
    else if( weaponType == 0 && GetShapeShift() == FORM_BEAR || GetShapeShift() == FORM_DIREBEAR )
        speed = 2500;
    else if( !disarmed )
    {
        if(Item *item = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+weaponType))
            speed = item->GetProto()->Delay;
    }
    return speed;
}

int32 Player::GetBaseMinDamage(uint8 weaponType)
{
    int32 damage = 1.0f;
    if( Item *item = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+weaponType) )
        if(disarmed == false) damage += item->GetProto()->minDamage;
    return damage;
}

int32 Player::GetBaseMaxDamage(uint8 weaponType)
{
    int32 damage = 2.0f;
    if( Item *item = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+weaponType) )
        if(disarmed == false) damage += item->GetProto()->maxDamage;
    return damage;
}

int32 Player::GetBonusAttackPower()
{
    return GetBonusesFromItems(ITEM_STAT_ATTACK_POWER);
}

int32 Player::GetBonusRangedAttackPower()
{
    return GetBonusesFromItems(ITEM_STAT_ATTACK_POWER)+GetBonusesFromItems(ITEM_STAT_RANGED_ATTACK_POWER);
}

#define IS_ARENA(x) ( (x) >= BATTLEGROUND_ARENA_2V2 && (x) <= BATTLEGROUND_ARENA_5V5 )

void Player::SaveToDB(bool bNewCharacter /* =false */)
{
    bool in_arena = false;
    QueryBuffer * buf = ((!sWorld.DisableBufferSaving || bNewCharacter) ? new QueryBuffer() : NULL);
    if( m_bg != NULL && IS_ARENA( m_bg->GetType() ) )
        in_arena = true;

    std::stringstream ss;
    ss << "REPLACE INTO characters VALUES ("
    << GetLowGUID() << ", "
    << GetSession()->GetAccountId() << ","
    << "'" << m_name.c_str() << "', "
    << uint32(getRace()) << ","
    << uint32(getClass()) << ","
    << uint32(getGender()) << ","
    << uint32(customizable ? 1 : 0) << ","
    << uint32(getLevel()) << ","
    << uint32(m_XPoff ? 1 : 0) << ","
    << GetUInt32Value(PLAYER_XP) << ","
    << GetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX) << ","
    << GetUInt32Value(PLAYER_CHOSEN_TITLE) << ","
    << GetUInt64Value(PLAYER_FIELD_COINAGE) << ","
    << GetUInt32Value(PLAYER_CHARACTER_POINTS) << ","
    << GetUInt32Value(UNIT_FIELD_HEALTH) << ","
    << uint32(GetPVPRank()) << ","
    << GetUInt32Value(PLAYER_BYTES) << ","
    << GetUInt32Value(PLAYER_BYTES_2) << ","
    << GetUInt32Value(PLAYER_BYTES_3) << ","
    << GetUInt32Value(PLAYER_FIELD_BYTES) << ","
    << GetUInt32Value(PLAYER_FIELD_BYTES2) << ",";

    uint32 mapid = m_mapId, instanceid = m_instanceId;
    float posx = 0.f, posy = 0.f, posz = 0.f, poso = 0.f;
    if( in_arena )
    {
        // if its an arena, save the entry coords instead
        posx = m_bgEntryPointX;
        posy = m_bgEntryPointY;
        posz = m_bgEntryPointZ;
        poso = m_bgEntryPointO;
        mapid = m_bgEntryPointMap;
        instanceid = m_bgEntryPointInstance;
    } else GetPosition(posx, posy, posz, poso);

    if(m_playerInfo)
    {
        m_playerInfo->curInstanceID = m_instanceId;
        m_playerInfo->lastmapid = mapid;
        m_playerInfo->lastpositionx = posx;
        m_playerInfo->lastpositiony = posy;
        m_playerInfo->lastpositionz = posz;
        m_playerInfo->lastorientation = poso;
    }

    ss << mapid << ", "
    << posx << ", "
    << posy << ", "
    << posz << ", "
    << poso << ", "
    << instanceid << ", "
    << m_zoneId << ", ";
    //online state
    if(GetSession()->_loggingOut || bNewCharacter)
        ss << "0, ";
    else ss << "1, ";

    ss << m_bind_mapid << ", "
    << m_bind_pos_x << ", "
    << m_bind_pos_y << ", "
    << m_bind_pos_z << ", "
    << m_bind_zoneid << ", "

    << uint32(m_isResting) << ", "
    << uint32(m_restState) << ", "
    << uint32(m_restAmount) << ", "
    << uint32(m_deathState) << ", "
    << uint32(m_FirstLogin) << ", "
    << uint32(rename_pending) << ", "
    << uint32(m_StableSlotCount) << ", ";

    // instances
    ss << m_bgEntryPointMap << ", "
    << m_bgEntryPointX      << ", "
    << m_bgEntryPointY      << ", "
    << m_bgEntryPointZ      << ", "
    << m_bgEntryPointO      << ", ";

    // taxi
    if(m_onTaxi && m_CurrentTaxiPath) {
        ss << m_CurrentTaxiPath->GetID() << ", ";
        ss << lastNode << ", ";
        ss << GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID) << ", ";
    } else ss << "0, 0, 0, ";

    float transx, transy, transz, transo;
    m_movementInterface.GetTransportPosition(transx, transy, transz, transo);
    ss << uint32(m_CurrentTransporter ? m_CurrentTransporter->GetLowGUID() : 0) << ", ";
    ss << transx << ", " << transy << ", " << transz << ", ";
    ss << uint32(iInstanceType) << ", "
    << uint32(iRaidType) << ", "
    << uint32(m_talentInterface.GetActiveSpec()) << ", "
    << uint32(m_talentInterface.GetSpecCount()) << ", "
    << uint32(m_talentInterface.GetTalentResets()) << ", "
    << uint32(m_talentInterface.GetAvailableTalentPoints()) << ", ";
    uint16 talentStack = 0xFFFF;
    m_talentInterface.GetActiveTalentTabStack(talentStack);
    ss << uint32(talentStack) << ", " << "0, 0)";  // Reset for position and talents

    if(buf)
        buf->AddQueryStr(ss.str());
    else CharacterDatabase.WaitExecuteNA(ss.str().c_str());

    // Glyphs
    m_talentInterface.SaveActionButtonData(buf);

    // Auras
    _SavePlayerAuras(buf);

    // Cooldowns
    _SavePlayerCooldowns(buf);

    // Equipment sets
    _SaveEquipmentSets(buf);

    // Exploration data
    _SaveExplorationData(buf);

    // Faction data
    //_SaveFactionData(buf);

    // Glyphs
    m_talentInterface.SaveGlyphData(buf);

    // Inventory
    m_inventory.SaveToDB(bNewCharacter, buf);

    // Banking
    m_bank.SaveToDB(buf);

    // Currency
    m_currency.SaveToDB(buf);

    // Known titles
    _SaveKnownTitles(buf);

    // Powers
    _SavePlayerPowers(buf);

    // Quest log
    _SavePlayerQuestLog(buf);

    // Completed quests
    _SaveCompletedQuests(buf);

    // Skills
    _SaveSkills(buf);

    // Social
    _SaveSocial(buf);

    // Spells
    _SaveSpells(buf);

    // Talents
    m_talentInterface.SaveTalentData(buf);

    // Taxi masks
    _SaveTaxiMasks(buf);

    // timestamps
    _SaveTimeStampData(buf);

    // save quest progress
    _SavePlayerQuestLog(buf);

    // GM Ticket
    if(GM_Ticket* ticket = sTicketMgr.GetGMTicketByPlayer(GetGUID()))
        sTicketMgr.SaveGMTicket(ticket, buf);

    // Pets
    if(getClass() == HUNTER || getClass() == WARLOCK)
    {
        _SavePet(buf);
        _SavePetSpells(buf);
    }

    ForceSaved = false;
    m_nextSave = 120000;
    if(buf)
        CharacterDatabase.AddQueryBuffer(buf);
}

bool Player::LoadFromDB()
{
    AsyncQuery * q = new AsyncQuery( new SQLClassCallbackP0<Player>(this, &Player::LoadFromDBProc) );
    q->AddQuery("SELECT * FROM character_data WHERE guid='%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_auras WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_bans WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_cooldowns WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_criteria_data WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_equipmentsets WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_exploration WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_factions WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_glyphs WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_items WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_known_titles WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_powers WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_pvp_data WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_quests_completed WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_skills WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_social WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_spells WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_talents WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_taximasks WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_timestamps WHERE guid = '%u'", m_objGuid.getLow());
    CharacterDatabase.QueueAsyncQuery(q);
    return true;
}

void Player::LoadFromDBProc(QueryResultVector & results)
{
    // set playerinfo
    m_playerInfo = objmgr.GetPlayerInfo(GetLowGUID());
    if(GetSession() == NULL || m_playerInfo == NULL || results[PLAYER_LO_DATA].result == NULL)
    {
        RemovePendingPlayer();
        return;
    }
    if(results[PLAYER_LO_BANS].result != NULL)
    {
        bool banned = true;
        if(Field *fields = results[PLAYER_LO_BANS].result->Fetch())
            if(fields[1].GetBool() && fields[1].GetUInt64() <= UNIXTIME)
                banned = false;
        if(banned)
        {
            RemovePendingPlayer(CHAR_LOGIN_TEMPORARY_GM_LOCK);
            return;
        }
    }

    Field *fields = results[PLAYER_LO_DATA].result->Fetch();
    if(fields[PLAYERLOAD_FIELD_ACCOUNT_ID].GetUInt32() != m_session->GetAccountId())
    {
        sWorld.LogCheater(m_session, "player tried to load character not belonging to them (guid %u, on account %u)", fields[0].GetUInt32(), fields[1].GetUInt32());
        RemovePendingPlayer();
        return;
    }

    // Load name
    m_name = fields[PLAYERLOAD_FIELD_NAME].GetString();

    // Load race/class from fields
    setRace(fields[PLAYERLOAD_FIELD_RACE].GetUInt8());
    setClass(fields[PLAYERLOAD_FIELD_CLASS].GetUInt8());
    setGender(fields[PLAYERLOAD_FIELD_GENDER].GetUInt8());
    customizable = fields[PLAYERLOAD_FIELD_CUSTOMIZE_FLAGS].GetBool();

    // set race dbc
    myRace = dbcCharRace.LookupEntry(getRace());
    myClass = dbcCharClass.LookupEntry(getClass());
    if( myClass == NULL || myRace == NULL )
    {
        printf("No class/race data\n");
        // bad character
        printf("guid %u failed to login, no race or class dbc found. (race %u class %u)\n", uint(GetLowGUID()), uint(getRace()), uint(getClass()));
        RemovePendingPlayer();
        return;
    }

    if(GuildInfo *gInfo = guildmgr.GetGuildInfo(m_playerInfo->GuildId))
    {
        SetGuildId(m_playerInfo->GuildId);
        SetGuildRank(m_playerInfo->GuildRank);
        SetGuildLevel(gInfo->m_guildLevel);
    }

    m_bgTeam = m_team = myRace->TeamId;

    SetNoseLevel();

    // set power type
    SetPowerType(myClass->power_type);

    // obtain player create info
    info = objmgr.GetPlayerCreateInfo(getRace(), getClass());
    if( info == NULL )
    {
        printf("No player create info\n");
        sLog.Error("PlayerCreateInfo", "Character of guid %u creation failed due to non existant or invalid playercreateinfo.", uint(GetLowGUID()));
        RemovePendingPlayer();
        return;
    }
    assert(info);

    // set level
    uint32 maxlevel = sWorld.GetMaxLevel(this), level = fields[PLAYERLOAD_FIELD_LEVEL].GetUInt32();
    if(level > maxlevel) level = maxlevel;
    SetUInt32Value(UNIT_FIELD_LEVEL, level);

    // Set correct maximum level
    SetUInt32Value(PLAYER_FIELD_MAX_LEVEL, maxlevel);

    m_talentInterface.InitGlyphSlots();
    m_talentInterface.InitGlyphsForLevel(level);

    // level dependant taxi node
    SetTaximaskNode( 213, level >= 68 ? false : true );  //Add 213 (Shattered Sun Staging Area) if lvl >=68

    // Set our base stats
    baseStats = sStatSystem.GetUnitBaseStats(getRace(), getClass(), level);

    m_XPoff = fields[PLAYERLOAD_FIELD_XP_DISABLED].GetBool();
    // set xp
    SetUInt32Value(PLAYER_XP, fields[PLAYERLOAD_FIELD_EXPERIENCE].GetUInt32());

    // set the rest of the shit
    SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, fields[PLAYERLOAD_FIELD_WATCHED_FACTION_INDEX].GetUInt32());
    SetUInt32Value(PLAYER_CHOSEN_TITLE, fields[PLAYERLOAD_FIELD_SELECTED_TITLE].GetUInt32());
    SetUInt64Value(PLAYER_FIELD_COINAGE, fields[PLAYERLOAD_FIELD_GOLD].GetUInt64());
    SetUInt32Value(PLAYER_CHARACTER_POINTS, fields[PLAYERLOAD_FIELD_AVAILABLE_PROF_POINTS].GetUInt32());

    load_health = fields[PLAYERLOAD_FIELD_CURRENTHP].GetUInt32();
    uint8 pvprank = fields[PLAYERLOAD_FIELD_CURRENT_PVPRANK].GetUInt8();
    SetUInt32Value( PLAYER_BYTES, fields[PLAYERLOAD_FIELD_PLAYERBYTES].GetUInt32() );
    SetUInt32Value( PLAYER_BYTES_2, fields[PLAYERLOAD_FIELD_PLAYERBYTES2].GetUInt32() );
    SetUInt32Value( PLAYER_BYTES_3, fields[PLAYERLOAD_FIELD_PLAYERBYTES3].GetUInt32() );
    SetUInt32Value( PLAYER_FIELD_BYTES, fields[PLAYERLOAD_FIELD_PLAYER_BYTES].GetUInt32() );
    SetUInt32Value( PLAYER_FIELD_BYTES2, fields[PLAYERLOAD_FIELD_PLAYER_BYTES2].GetUInt32() );

    // Set our position
    m_mapId         = fields[PLAYERLOAD_FIELD_MAPID].GetUInt32();
    m_position.x    = fields[PLAYERLOAD_FIELD_POSITION_X].GetFloat();
    m_position.y    = fields[PLAYERLOAD_FIELD_POSITION_Y].GetFloat();
    m_position.z    = fields[PLAYERLOAD_FIELD_POSITION_Z].GetFloat();
    m_position.o    = fields[PLAYERLOAD_FIELD_POSITION_O].GetFloat();
    m_instanceId    = fields[PLAYERLOAD_FIELD_INSTANCE_ID].GetUInt32();
    m_zoneId        = fields[PLAYERLOAD_FIELD_ZONEID].GetUInt32();

    // Calculate the base stats now they're all loaded
    for(uint32 x = 0; x < 7; x++)
        SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+x, 1.f);

    // Initialize 'normal' fields
    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.f);
    if(getClass() == WARRIOR && !HasAura(21156) && !HasAura(7376) && !HasAura(7381))
        CastSpell(this, 2457, true); // We have no shapeshift aura, set our shapeshift.

    // We're players!
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0.388999998569489f );
    SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f );

    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, info->factiontemplate);
    SetUInt32Value(UNIT_FIELD_DISPLAYID, info->displayId[getGender()]);
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, info->displayId[getGender()]);
    EventModelChange();

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    m_bind_mapid = fields[PLAYERLOAD_FIELD_BINDMAPID].GetUInt32();
    m_bind_pos_x = fields[PLAYERLOAD_FIELD_BINDPOSITION_X].GetFloat();
    m_bind_pos_y = fields[PLAYERLOAD_FIELD_BINDPOSITION_Y].GetFloat();
    m_bind_pos_z = fields[PLAYERLOAD_FIELD_BINDPOSITION_Z].GetFloat();
    m_bind_zoneid = fields[PLAYERLOAD_FIELD_BINDZONEID].GetUInt32();

    m_isResting = fields[PLAYERLOAD_FIELD_ISRESTING].GetUInt8();
    m_restState = fields[PLAYERLOAD_FIELD_RESTSTATE].GetUInt8();
    m_restAmount = fields[PLAYERLOAD_FIELD_RESTTIME].GetUInt32();

    m_deathState = DeathState(fields[PLAYERLOAD_FIELD_DEATHSTATE].GetUInt32());
    if(load_health && m_deathState == JUST_DIED)
    {
        m_deathState = CORPSE;
        load_health = 0;
    }

    SetUInt32Value(UNIT_FIELD_HEALTH, load_health);
    m_FirstLogin = fields[PLAYERLOAD_FIELD_FIRST_LOGON].GetBool();
    rename_pending = fields[PLAYERLOAD_FIELD_FORCE_RENAME_PENDING].GetBool();
    m_StableSlotCount = fields[PLAYERLOAD_FIELD_TOTALSTABLESLOTS].GetUInt32();
    m_bgEntryPointMap = fields[PLAYERLOAD_FIELD_ENTRYPOINT_MAP].GetUInt32();
    m_bgEntryPointX = fields[PLAYERLOAD_FIELD_ENTRYPOINT_X].GetFloat();
    m_bgEntryPointY = fields[PLAYERLOAD_FIELD_ENTRYPOINT_Y].GetFloat();
    m_bgEntryPointZ = fields[PLAYERLOAD_FIELD_ENTRYPOINT_Z].GetFloat();
    m_bgEntryPointO = fields[PLAYERLOAD_FIELD_ENTRYPOINT_O].GetFloat();

    uint32 taxipath = fields[PLAYERLOAD_FIELD_TAXI_PATH].GetUInt32();
    if(TaxiPath *path = sTaxiMgr.GetTaxiPath(taxipath))
    {
        lastNode = fields[PLAYERLOAD_FIELD_TAXI_LASTNODE].GetUInt32();
        SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, fields[PLAYERLOAD_FIELD_TAXI_MOUNTID].GetUInt32());
        SetTaxiPath(path);
        m_onTaxi = true;
    }

    uint32 transGuid = fields[PLAYERLOAD_FIELD_TRANSPORTERGUID].GetUInt32();
    GetMovementInterface()->SetTransportData(uint64(transGuid ? MAKE_NEW_GUID(transGuid, 0, HIGHGUID_TYPE_TRANSPORTER) : 0), 0,
        fields[PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_X].GetFloat(), fields[PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_Y].GetFloat(),
        fields[PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_Z].GetFloat(), 0.f, 0);

    iInstanceType = fields[PLAYERLOAD_FIELD_INSTANCE_DIFFICULTY].GetUInt32();
    iRaidType = fields[PLAYERLOAD_FIELD_RAID_DIFFICULTY].GetUInt32();

    uint32 talentActiveSpec = fields[PLAYERLOAD_FIELD_ACTIVE_SPEC].GetUInt32(),
        talentSpecCount = fields[PLAYERLOAD_FIELD_SPEC_COUNT].GetUInt32(),
        talentResetCounter = fields[PLAYERLOAD_FIELD_TALENT_RESET_COUNTER].GetUInt32(),
        talentAvailablePoints = fields[PLAYERLOAD_FIELD_AVAILABLE_TALENT_POINTS].GetUInt32(),
        activeTalentSpecStack = fields[PLAYERLOAD_FIELD_ACTIVE_TALENT_SPECSTACK].GetUInt32();
    m_talentInterface.SetTalentData(talentActiveSpec, talentSpecCount, talentResetCounter, talentAvailablePoints, activeTalentSpecStack);

    m_talentInterface.LoadActionButtonData(results[PLAYER_LO_ACTIONS].result);
    _LoadPlayerAuras(results[PLAYER_LO_AURAS].result);
    _LoadPlayerCooldowns(results[PLAYER_LO_COOLDOWNS].result);
    //m_achievmentInterface.LoadCriteriaData(results[PLAYER_LO_CRITERIA_DATA].result);
    _LoadEquipmentSets(results[PLAYER_LO_EQUIPMENTSETS].result);
    _LoadExplorationData(results[PLAYER_LO_EXPLORATION].result);
    //m_factionInterface.LoadFactionData(results[PLAYER_LO_FACTIONS].result);
    m_talentInterface.LoadGlyphData(results[PLAYER_LO_GLYPHS].result);
    _LoadKnownTitles(results[PLAYER_LO_KNOWN_TITLES].result);
    _LoadPlayerPowers(results[PLAYER_LO_POWERS].result);
    _LoadPlayerQuestLog(results[PLAYER_LO_QUEST_LOG].result);
    _LoadCompletedQuests(results[PLAYER_LO_QUESTS_COMPLETED].result);
    _LoadSkills(results[PLAYER_LO_SKILLS].result);
    _LoadSocial(results[PLAYER_LO_SOCIAL].result);
    _LoadSpells(results[PLAYER_LO_SPELLS].result);
    m_talentInterface.LoadTalentData(results[PLAYER_LO_TALENTS].result);
    _LoadTaxiMasks(results[PLAYER_LO_TAXIMASKS].result);
    _LoadTimeStampData(results[PLAYER_LO_TIMESTAMPS].result);

    _setFaction();

    if(m_session->CanUseCommand('c'))
        _AddLanguages(true);
    else _AddLanguages(sWorld.cross_faction_world);

    OnlineTime = UNIXTIME;
    if(IsInGuild()) SetUInt32Value(PLAYER_GUILD_TIMESTAMP, OnlineTime);
    if( fields[PLAYERLOAD_FIELD_NEEDS_POSITION_RESET].GetBool() )
        EjectFromInstance();
    if( fields[PLAYERLOAD_FIELD_NEEDS_TALENT_RESET].GetBool() )
        m_talentInterface.ResetAllSpecs();

    SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER); // enables automatic power regen
    m_session->FullLogin(this);

    if( !isAlive() )
    {
        if((myCorpse = objmgr.GetCorpseByOwner(GetLowGUID())) == NULL)
            if((myCorpse = objmgr.LoadCorpse(GetLowGUID())) == NULL)
                myCorpse = CreateCorpse();
        if(myCorpse)
        {
            WorldPacket* data = new WorldPacket(MSG_CORPSE_QUERY, 21);
            BuildCorpseInfo(data, myCorpse);
            SendDelayedPacket(data);
        }
    }
}

void Player::_LoadPlayerAuras(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint8 auraSlot = fields[1].GetUInt8();
        uint32 spellId = fields[2].GetUInt32();
        SpellEntry *sp = dbcSpell.LookupEntry(spellId);
        if(sp == NULL)
            continue;

        uint16 auraFlags = fields[3].GetUInt16();
        uint8 auraLevel = fields[4].GetUInt8();
        int16 auraStackCharge = fields[5].GetInt16();
        WoWGuid casterGuid = fields[6].GetUInt64();
        time_t expiration = 0;
        if((expiration = fields[7].GetUInt64()) != 0)
        {
            if(expiration < 172800)
                expiration += UNIXTIME;
            else if(expiration <= UNIXTIME+5)
                continue;
        }

        Aura *aur = new Aura(this, sp, auraFlags, auraLevel, auraStackCharge, expiration, casterGuid);
        for(uint8 x = 0; x < 3; x++)
        {
            if(sp->Effect[x] != SPELL_EFFECT_APPLY_AURA)
                continue;
            aur->AddMod(x, sp->EffectApplyAuraName[x], fields[8+x].GetInt32(), fields[11+x].GetUInt32(), fields[14+x].GetInt32(), fields[17+x].GetFloat());
        }
        m_loadAuras.push_back(aur);
    }while(result->NextRow());
}

void Player::_SavePlayerAuras(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_auras WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_auras WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(uint8 i = 0; i < MAX_AURAS; i++)
    {
        Aura *aur = m_AuraInterface.FindAuraBySlot(i);
        if(aur == NULL)
            continue;
        uint8 auraMask=0;
        for(uint8 x = 0; x < 3; x++)
        {
            if(aur->GetSpellProto()->Effect[i] != SPELL_EFFECT_APPLY_AURA)
                continue;
            auraMask |= 1<<x;
        }
        if(auraMask == 0)
            continue;

        if(ss.str().length())
            ss << ", ";

        Modifier *mods[3] = { aur->GetMod(0), aur->GetMod(1), aur->GetMod(2) };
        ss << "(" << GetLowGUID()
        << ", " << uint32(i)
        << ", " << aur->GetSpellId()
        << ", " << aur->GetAuraFlags()
        << ", " << aur->GetAuraLevel()
        << ", " << aur->getStackSizeOrProcCharges()
        << ", '" << uint64(aur->GetCasterGUID())
        << "', " << aur->GetExpirationTime()
        << ", " << int32(mods[0] ? mods[0]->m_baseAmount : 0)
        << ", " << int32(mods[1] ? mods[1]->m_baseAmount : 0)
        << ", " << int32(mods[2] ? mods[2]->m_baseAmount : 0)
        << ", " << uint32(mods[0] ? mods[0]->m_bonusAmount : 0)
        << ", " << uint32(mods[1] ? mods[1]->m_bonusAmount : 0)
        << ", " << uint32(mods[2] ? mods[2]->m_bonusAmount : 0)
        << ", " << int32(mods[0] ? mods[0]->fixed_amount : 0)
        << ", " << int32(mods[1] ? mods[1]->fixed_amount : 0)
        << ", " << int32(mods[2] ? mods[2]->fixed_amount : 0)
        << ", " << float(mods[0] ? mods[0]->fixed_float_amount : 0.f)
        << ", " << float(mods[1] ? mods[1]->fixed_float_amount : 0.f)
        << ", " << float(mods[2] ? mods[2]->fixed_float_amount : 0.f);
        ss << ")";
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_auras VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_auras VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadPlayerCooldowns(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint32 spellId = fields[1].GetUInt32();
        uint16 type = fields[2].GetUInt8();
        if( type >= NUM_COOLDOWN_TYPES )
            continue;
        uint16 category = fields[3].GetUInt16();
        time_t expireTime = fields[4].GetUInt64();
        if(expireTime <= UNIXTIME)
            continue;

        PlayerCooldown &cd = m_cooldownMap[type][category];
        cd.SpellId = spellId;
        cd.ExpireTime = expireTime;
        cd.ItemId = fields[5].GetUInt32();
    }while(result->NextRow());
}

void Player::_SavePlayerCooldowns(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_cooldowns WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_cooldowns WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(uint8 i = 0; i < NUM_COOLDOWN_TYPES; i++)
    {
        for(PlayerCooldownMap::iterator itr = m_cooldownMap[i].begin(); itr != m_cooldownMap[i].end(); itr++)
        {
            if(ss.str().length())
                ss << ", ";

            ss << "(" << GetLowGUID()
            << ", " << uint32(itr->second.SpellId)
            << ", " << uint32(i)
            << ", " << uint32(itr->first)
            << ", " << uint64(itr->second.ExpireTime)
            << ", " << uint32(itr->second.ItemId);
            ss << ")";
        }
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_cooldowns VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_cooldowns VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadEquipmentSets(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {

    }while(result->NextRow());
}

void Player::_SaveEquipmentSets(QueryBuffer * buf)
{

}

void Player::_LoadExplorationData(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint32 field = PLAYER_EXPLORED_ZONES_1+fields[1].GetUInt8();
        SetUInt32Value(field, fields[2].GetUInt32());
    }while(result->NextRow());
}

void Player::_SaveExplorationData(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_exploration WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_exploration WHERE guid = '%u';", GetLowGUID());

    uint32 offset = 0;
    std::stringstream ss;
    for(uint32 field = 0; field < 156; field++)
    {
        if(uint32 val = GetUInt32Value(PLAYER_EXPLORED_ZONES_1+field))
        {
            if(ss.str().length())
                ss << ", ";
            ss << "(" << GetLowGUID()
            << ", " << field
            << ", " << val;
            ss << ")";
        }
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_exploration VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_exploration VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadKnownTitles(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint32 field = PLAYER__FIELD_KNOWN_TITLES+fields[1].GetUInt8();
        SetUInt32Value(field, fields[2].GetUInt32());
    }while(result->NextRow());
}

void Player::_SaveKnownTitles(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_known_titles WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_known_titles WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(uint32 field = 0; field < 4; field++)
    {
        if(uint32 val = GetUInt32Value(PLAYER__FIELD_KNOWN_TITLES+field))
        {
            if(ss.str().length())
                ss << ", ";
            ss << "(" << GetLowGUID()
            << ", " << field
            << ", " << val;
            ss << ")";
        }
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_known_titles VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_known_titles VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadPlayerPowers(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        uint8 powerType = result->Fetch()[1].GetUInt8();
        if(powerType >= POWER_TYPE_MAX)
            continue;

        load_power[powerType] = result->Fetch()[2].GetUInt32();
    }while(result->NextRow());
}

void Player::_SavePlayerPowers(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_powers WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_powers WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(uint8 i = 0; i < POWER_TYPE_MAX; i++)
    {
        uint32 power = 0;
        if((power = GetPower(PowerType(i))) == 0)
            continue;
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint32(i)
        << ", " << power;
        ss << ")";
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_powers VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_powers VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadPlayerQuestLog(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint8 slot = fields[1].GetUInt32();
        Quest *quest = sQuestMgr.GetQuestPointer(fields[2].GetUInt32());
        if(quest == NULL)
            continue;
        if(m_questlog[slot] != 0)
            continue;

        QuestLogEntry *entry = new QuestLogEntry();
        entry->Init(quest, castPtr<Player>(this), slot);
        entry->Load(fields);
        entry->UpdatePlayerFields();
    }while(result->NextRow());
}

void Player::_SavePlayerQuestLog(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_questlog WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_questlog WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(uint8 i = 0; i < QUEST_LOG_COUNT; i++)
    {
        QuestLogEntry *questLog = GetQuestLogInSlot(i);
        if(questLog == NULL)
            continue;
        if(ss.str().length())
            ss << ", ";

        ss << "(" << GetLowGUID()
        << ", " << uint32(i)
        << ", " << uint32(questLog->GetQuest()->id)
        << ", " << questLog->getExpirationTime()
        << ", " << questLog->GetObjectiveCount(0)
        << ", " << questLog->GetObjectiveCount(1)
        << ", " << questLog->GetObjectiveCount(2)
        << ", " << questLog->GetObjectiveCount(3)
        << ", " << questLog->GetExplorationFlag()
        << ", " << questLog->GetPlayerSlainCount();
        ss << ")";
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_questlog VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_questlog VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadCompletedQuests(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint32 questId = fields[1].GetUInt32();
        Quest *qst = sQuestMgr.GetQuestPointer(questId);
        if(qst == NULL)
            continue;

        time_t timeStamp = fields[2].GetUInt64();
        if(qst->qst_is_repeatable == REPEATABLE_DAILY)
        {
            if(timeStamp <= sDayWatcher.GetLastDailyResetTime())
                continue; // Don't load up dailies if they're from before the last reset

            m_completedDailyQuests.insert(std::make_pair(qst->id, timeStamp));
        } else m_completedQuests.insert(std::make_pair(qst->id, timeStamp));
    }while(result->NextRow());
}

void Player::_SaveCompletedQuests(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_quests_completed WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_quests_completed WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    std::map<uint32, time_t>::iterator itr;
    for(itr = m_completedQuests.begin(); itr != m_completedQuests.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint32(itr->first)
        << ", " << itr->second;
        ss << ")";
    }
    for(itr = m_completedDailyQuests.begin(); itr != m_completedDailyQuests.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint32(itr->first)
        << ", " << itr->second;
        ss << ")";
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_quests_completed VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_quests_completed VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadSkills(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint16 skillId = fields[1].GetUInt16();
        SkillLineEntry *skillLine = dbcSkillLine.LookupEntry(skillId);
        if(skillLine == NULL)
            continue;

        PlayerSkill inf;
        inf.Skill = skillLine;
        inf.SkillPos = fields[2].GetUInt8();
        inf.CurrentValue = fields[3].GetUInt16();
        inf.MaximumValue = fields[4].GetUInt16();
        inf.BonusValue = inf.BonusTalent = 0;
        m_skills.insert( std::make_pair( skillId, inf ) );
        m_skillsByIndex.insert(std::make_pair(inf.SkillPos, &inf));
    }while(result->NextRow());
    _UpdateSkillFields();
}

void Player::_SaveSkills(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_skills WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_skills WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint32(itr->first)
        << ", " << uint32(itr->second.SkillPos)
        << ", " << uint32(itr->second.CurrentValue)
        << ", " << uint32(itr->second.MaximumValue);
        ss << ")";
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_skills VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_skills VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadSocial(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        std::pair<WoWGuid, std::string> social_pair = std::make_pair(WoWGuid(fields[2].GetUInt64()), fields[3].GetString());
        switch(fields[1].GetUInt8())
        {
        case 0: m_friends.insert(social_pair); break;
        case 1: m_ignores.insert(social_pair); break;
        case 2: m_mutes.insert(social_pair); break;
        }
    }while(result->NextRow());
}

void Player::_SaveSocial(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_social WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_social WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    std::map<WoWGuid, std::string>::iterator itr;
    for(itr = m_friends.begin(); itr != m_friends.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint64(itr->first)
        << ", " << uint32(0)
        << ", " << itr->second;
        ss << ")";
    }
    for(itr = m_ignores.begin(); itr != m_ignores.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint64(itr->first)
        << ", " << uint32(1)
        << ", " << itr->second;
        ss << ")";
    }
    for(itr = m_mutes.begin(); itr != m_mutes.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint64(itr->first)
        << ", " << uint32(2)
        << ", " << itr->second;
        ss << ")";
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_social VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_social VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadSpells(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        if(SpellEntry *sp = dbcSpell.LookupEntry(result->Fetch()[1].GetUInt32()))
            mSpells.insert(sp->Id);
    }while(result->NextRow());
}

void Player::_SaveSpells(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_spells WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_spells WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(auto itr = mSpells.begin(); itr != mSpells.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID() << ", " << uint32(*itr) << ")";
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_social VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_social VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadTaxiMasks(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        SetTaximask(fields[1].GetUInt8(), fields[2].GetUInt32());
    }while(result->NextRow());
}

void Player::_SaveTaxiMasks(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_taximasks WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_taximasks WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(uint8 i = 0; i < MAX_TAXI; i++)
    {
        if(uint32 taxiMask = GetTaximask(i))
        {
            if(ss.str().length())
                ss << ", ";
            ss << "(" << GetLowGUID() << ", " << taxiMask << ")";
        }
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_taximasks VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_taximasks VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadTimeStampData(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {

    }while(result->NextRow());
}

void Player::_SaveTimeStampData(QueryBuffer * buf)
{

}

uint32 GetSpellForLanguageID(uint32 LanguageID)
{
    switch(LanguageID)
    {
    case LANG_COMMON: return 668;
    case LANG_ORCISH: return 669;
    case LANG_TAURAHE: return 670;
    case LANG_DARNASSIAN: return 671;
    case LANG_DWARVISH: return 672;
    case LANG_THALASSIAN: return 813;
    case LANG_DRACONIC: return 814;
    case LANG_DEMONIC: return 815;
    case LANG_TITAN: return 816;
    case LANG_GNOMISH: return 7430;
    case LANG_TROLL: return 7341;
    case LANG_GUTTERSPEAK: return 17737;
    case LANG_DRAENEI: return 29932;
    } return 0;
}

uint32 GetSpellForLanguageSkill(uint32 SkillID)
{
    switch(SkillID)
    {
    case SKILL_LANG_COMMON: return 668;
    case SKILL_LANG_ORCISH: return 669;
    case SKILL_LANG_TAURAHE: return 670;
    case SKILL_LANG_DARNASSIAN: return 671;
    case SKILL_LANG_DWARVEN: return 672;
    case SKILL_LANG_THALASSIAN: return 813;
    case SKILL_LANG_DRACONIC: return 814;
    case SKILL_LANG_DEMON_TONGUE: return 815;
    case SKILL_LANG_TITAN: return 816;
    case SKILL_LANG_OLD_TONGUE: return 817;
    case SKILL_LANG_GNOMISH: return 7430;
    case SKILL_LANG_TROLL: return 7341;
    case SKILL_LANG_GUTTERSPEAK: return 17737;
    case SKILL_LANG_DRAENEI: return 29932;
    } return 0;
}

///====================================================================
///  Create
///  params: p_newChar
///  desc:   data from client to create a new character
///====================================================================
bool Player::Create(WorldPacket& data )
{
    uint8 race,class_,gender,skin,face,hairStyle,hairColor,facialHair,outfitId;

    // unpack data into member variables
    data >> m_name;

    // correct capitalization
    CapitalizeString(m_name);

    data >> race >> class_ >> gender >> skin >> face;
    data >> hairStyle >> hairColor >> facialHair >> outfitId;

    if((info = objmgr.GetPlayerCreateInfo(race, class_)) == NULL)
    {
        // info not found.
        WorldPacket data(SMSG_CHARACTER_CREATE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        GetSession()->SendPacket(&data);
        return false;
    }

    // check that the account CAN create TBC or Cata characters, if we're making some
    if(race >= RACE_BLOODELF && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_01))
    {
        WorldPacket data(SMSG_CHARACTER_CREATE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        GetSession()->SendPacket(&data);
        return false;
    }

    m_mapId = info->mapId;
    m_zoneId = info->zoneId;
    SetPosition( info->positionX, info->positionY, info->positionZ, info->Orientation);
    m_bind_pos_x = info->positionX;
    m_bind_pos_y = info->positionY;
    m_bind_pos_z = info->positionZ;
    m_bind_mapid = info->mapId;
    m_bind_zoneid = info->zoneId;
    m_isResting = 0;
    m_restAmount = 0;
    m_restState = 0;

    memset(m_taximask, 0, sizeof(uint32)*MAX_TAXI);

    // set race dbc
    myRace = dbcCharRace.LookupEntry(race);
    myClass = dbcCharClass.LookupEntry(class_);
    if(!myRace || !myClass)
    {
        // information not found
        WorldPacket data(SMSG_CHARACTER_CREATE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        GetSession()->SendPacket(&data);
        return false;
    }

    m_team = myRace->TeamId;
    uint8 powertype = uint8(myClass->power_type);

    // Automatically add the race's taxi hub to the character's taximask at creation time ( 1 << (taxi_node_id-1) )
    memset(m_taximask,0,sizeof(m_taximask));
    if(sWorld.Start_With_All_Taximasks)
    {
        for(uint8 i = 0; i < MAX_TAXI; i++)
            m_taximask[i] = 0xFFFFFFFF;
    }
    else if(class_ == DEATHKNIGHT)
    {
        for(uint8 i = 0; i < MAX_TAXI; i++)
            m_taximask[i] |= DKNodesMask[i];
    }

    switch(race)
    {
        case RACE_TAUREN:   AddTaximaskNode(22);                        break;
        case RACE_HUMAN:    AddTaximaskNode(2);                         break;
        case RACE_DWARF:    AddTaximaskNode(6);                         break;
        case RACE_GNOME:    AddTaximaskNode(6);                         break;
        case RACE_ORC:      AddTaximaskNode(23);                        break;
        case RACE_TROLL:    AddTaximaskNode(23);                        break;
        case RACE_UNDEAD:   AddTaximaskNode(11);                        break;
        case RACE_NIGHTELF: {AddTaximaskNode(26); AddTaximaskNode(27);} break;
        case RACE_BLOODELF: AddTaximaskNode(82);                        break;
        case RACE_DRAENEI:  AddTaximaskNode(94);                        break;
    }
    // team dependant taxi node
    AddTaximaskNode(100-m_team);

    uint32 level = std::max(uint8(1), sWorld.StartLevel);
    if(class_ == DEATHKNIGHT && level < 55) level = 55;
    setLevel(level);

    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    SetUInt32Value(PLAYER_FIELD_COINAGE, sWorld.StartGold);

    SetFaction( info->factiontemplate );

    SetUInt32Value(UNIT_FIELD_BYTES_0, ( ( race ) | ( class_ << 8 ) | ( gender << 16 ) | ( powertype << 24 ) ) );
    if(class_ == WARRIOR)
        SetShapeShift(FORM_BATTLESTANCE);

    SetUInt32Value(PLAYER_CHARACTER_POINTS, 2);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);
    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0.388999998569489f );
    SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f );
    SetByte(PLAYER_BYTES, 0, skin);
    SetByte(PLAYER_BYTES, 1, face);
    SetByte(PLAYER_BYTES, 2, hairStyle);
    SetByte(PLAYER_BYTES, 3, hairColor);
    SetByte(PLAYER_BYTES_2, 0, facialHair);
    SetByte(PLAYER_BYTES_2, 3, 0x02); // No Recruit a friend flag
    SetByte(PLAYER_BYTES_3, 0, gender);

    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, 400);// sWorld.GetXPToNextLevel(getLevel()));
    SetUInt32Value(PLAYER_FIELD_MAX_LEVEL, sWorld.GetMaxLevel(castPtr<Player>(this)));
    SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

    m_StableSlotCount = 0;

    for(std::set<uint32>::iterator sp = info->spell_list.begin();sp!=info->spell_list.end();sp++)
        mSpells.insert((*sp));

    m_FirstLogin = true;

    SkillLineEntry * se;
    for(std::list<CreateInfo_SkillStruct>::iterator ss = info->skills.begin(); ss!=info->skills.end(); ss++)
    {
        se = dbcSkillLine.LookupEntry(ss->skillid);
        if(se == NULL)
            continue;

        if(se->categoryId != SKILL_TYPE_LANGUAGE)
        {
            if( sWorld.StartLevel > 1 )
                _AddSkillLine(se->id, sWorld.StartLevel * 5, sWorld.StartLevel * 5 );
            else _AddSkillLine(se->id, ss->currentval, ss->maxval);
        }
    }
    _UpdateMaxSkillCounts();

    _InitialReputation();

    // Add actionbars
    for(std::list<CreateInfo_ActionBarStruct>::iterator itr = info->actionbars.begin();itr!=info->actionbars.end();itr++)
    {
        m_talentInterface.setAction(itr->button, itr->action, itr->type, 0);
        m_talentInterface.setAction(itr->button, itr->action, itr->type, 1);
    }

    if( GetSession()->HasGMPermissions() && sWorld.gm_force_robes )
    {
        // Force GM robes on GM's except 'az' status (if set to 1 in world.conf)
        if( strstr(GetSession()->GetPermissions(), "az") == NULL)
        {
            //We need to dupe this
            PlayerCreateInfo *GMinfo = new PlayerCreateInfo();
            memcpy(GMinfo, info, sizeof(info));

            GMinfo->items.clear();
            CreateInfo_ItemStruct itm;

            itm.protoid = 11508; //Feet
            itm.slot = 7;
            itm.amount = 1;
            GMinfo->items.push_back(itm);

            itm.protoid = 2586;//Chest
            itm.slot = 4;
            itm.amount = 1;
            GMinfo->items.push_back(itm);

            itm.protoid = 12064;//head
            itm.slot = 0;
            itm.amount = 1;
            GMinfo->items.push_back(itm);

            EquipInit(GMinfo);
            delete GMinfo;
        } else EquipInit(info);
    } else EquipInit(info);

    load_health = GetUInt32Value(UNIT_FIELD_HEALTH);
    for(uint8 i = 0; i < POWER_TYPE_MAX; i++)
        load_power[i] = GetPower(i);
    return true;
}

void Player::EquipInit(PlayerCreateInfo *EquipInfo)
{

}

void Player::setLevel(uint32 level)
{
    uint32 currLevel = GetUInt32Value(UNIT_FIELD_LEVEL);
    Unit::setLevel(level);
    if(currLevel == level)
        return;

    _UpdateMaxSkillCounts();
    if (m_playerInfo)
        m_playerInfo->lastLevel = level;

    if(IsInWorld())
    {
        m_talentInterface.InitGlyphsForLevel(level);
        if(currLevel > 9 || level > 9)
        {
            if(level <= 9)
                m_talentInterface.ResetAllSpecs();
            else m_talentInterface.ModAvailableTalentPoints(level-currLevel);
        }
        UpdateNearbyQuestGivers(); // For quests that require levels
        SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
        SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));
    }

    sLog.outDebug("Player %s set parameters to level %u", GetName(), level);
}

void Player::HandleBreathing(uint32 time_diff)
{
    uint8 underwaterState = m_movementInterface.GetUnderwaterState();
    if(underwaterState == 0)
        return;

    // In water
    if (underwaterState & UNDERWATERSTATE_UNDERWATER && !bInvincible && isAlive())
    {
        // Breath timer not activated - activate it
        if (m_MirrorTimer[BREATH_TIMER] == -1)
        {
            m_MirrorTimer[BREATH_TIMER] = m_UnderwaterTime;
            SendMirrorTimer(BREATH_TIMER, m_MirrorTimer[BREATH_TIMER], m_MirrorTimer[BREATH_TIMER], -1);
        }
        else
        {
            m_MirrorTimer[BREATH_TIMER] -= time_diff;

            // Timer limit - need deal damage
            if (m_MirrorTimer[BREATH_TIMER] < 0)
            {
                m_MirrorTimer[BREATH_TIMER] += 1*1000;

                // Calculate and deal damage
                uint32 damage = GetMaxHealth() / 5 + RandomUInt(getLevel()-1);
                SendEnvironmentalDamageLog( GetGUID(), DAMAGE_DROWNING, damage );
                DealDamage( this, damage, 0, 0, 0 );
            } else if (!(m_LastUnderwaterState & UNDERWATERSTATE_UNDERWATER)) // Update time in client if need
                SendMirrorTimer(BREATH_TIMER, m_UnderwaterTime, m_MirrorTimer[BREATH_TIMER], -1);
        }
    }
    else if (m_MirrorTimer[BREATH_TIMER] != -1)     // Regen timer
    {
        int32 UnderWaterTime = m_UnderwaterTime;

        // Need breath regen
        m_MirrorTimer[BREATH_TIMER] += 10*time_diff;
        if (m_MirrorTimer[BREATH_TIMER] >= UnderWaterTime || !isAlive())
            StopMirrorTimer(BREATH_TIMER);
        else if (m_LastUnderwaterState & UNDERWATERSTATE_UNDERWATER)
            SendMirrorTimer(BREATH_TIMER, UnderWaterTime, m_MirrorTimer[BREATH_TIMER], 10);
    }

    // In dark water
    if(sWorld.EnableFatigue)
    {
        if (!bInvincible && underwaterState & UNDERWATERSTATE_FATIGUE)
        {
            // Fatigue timer not activated - activate it
            if (m_MirrorTimer[FATIGUE_TIMER] == -1)
            {
                m_MirrorTimer[FATIGUE_TIMER] = 60000;
                SendMirrorTimer(FATIGUE_TIMER, m_MirrorTimer[FATIGUE_TIMER], m_MirrorTimer[FATIGUE_TIMER], -1);
            }
            else
            {
                m_MirrorTimer[FATIGUE_TIMER] -= time_diff;
                // Timer limit - need deal damage or teleport ghost to graveyard
                if (m_MirrorTimer[FATIGUE_TIMER] < 0)
                {
                    m_MirrorTimer[FATIGUE_TIMER] += 1000;
                    if (isAlive())                                          // Calculate and deal damage
                    {
                        uint32 damage = GetMaxHealth() / 5 + RandomUInt(getLevel()-1);
                        SendEnvironmentalDamageLog( GetGUID(), DAMAGE_DROWNING, damage );
                        DealDamage( this, damage, 0, 0, 0 );
                    } else if (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE))    // Teleport ghost to graveyard
                        RepopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
                } else if (!(m_LastUnderwaterState & UNDERWATERSTATE_FATIGUE))
                    SendMirrorTimer(FATIGUE_TIMER, 60000, m_MirrorTimer[FATIGUE_TIMER], -1);
            }
        }
        else if (m_MirrorTimer[FATIGUE_TIMER] != -1)       // Regen timer
        {
            int32 DarkWaterTime = 60000;
            m_MirrorTimer[FATIGUE_TIMER]+=10*time_diff;
            if (m_MirrorTimer[FATIGUE_TIMER] >= DarkWaterTime || !isAlive())
                StopMirrorTimer(FATIGUE_TIMER);
            else if (m_LastUnderwaterState & UNDERWATERSTATE_FATIGUE)
                SendMirrorTimer(FATIGUE_TIMER, DarkWaterTime, m_MirrorTimer[FATIGUE_TIMER], 10);
        }
    }

    if (!bInvincible && underwaterState & (UNDERWATERSTATE_LAVA|UNDERWATERSTATE_SLIME))
    {
        // Breath timer not activated - activate it
        if (m_MirrorTimer[FIRE_TIMER] == -1)
            m_MirrorTimer[FIRE_TIMER] = 3000;
        else
        {
            m_MirrorTimer[FIRE_TIMER] -= time_diff;
            if (m_MirrorTimer[FIRE_TIMER] < 0)
            {
                m_MirrorTimer[FIRE_TIMER] += 3000;

                // Calculate and deal damage
                uint32 damage = 600+RandomUInt(100);
                if (underwaterState & UNDERWATERSTATE_LAVA)
                {
                    SendEnvironmentalDamageLog( GetGUID(), DAMAGE_LAVA, damage );
                    DealDamage( this, damage, 0, 0, 0 );
                }
                else if (GetZoneId() != 1497)
                {
                    // need to skip Slime damage in Undercity,
                    SendEnvironmentalDamageLog( GetGUID(), DAMAGE_LAVA, damage );
                    DealDamage( this, damage, 0, 0, 0 );
                }
            }
        }
    } else m_MirrorTimer[FIRE_TIMER] = -1;

    // Recheck timers flag
    bool hasTimer = false;
    for (uint8 i = 0; i< 3; ++i)
    {
        if (m_MirrorTimer[i] != -1)
        {
            hasTimer = true;
            break;
        }
    }

    if(hasTimer) m_movementInterface.AddUnderwaterStateTimerPresent();
    m_LastUnderwaterState = underwaterState;
}

void Player::SendMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, int32 Regen)
{
    if (int(MaxValue) == -1)
    {
        if (int(CurrentValue) != -1)
            StopMirrorTimer(Type);
        return;
    }

    WorldPacket data(SMSG_START_MIRROR_TIMER, (21));
    data << (uint32)Type;
    data << CurrentValue;
    data << MaxValue;
    data << Regen;
    data << (uint8)0;
    data << (uint32)0;
    SendPacket(&data);
}

void Player::StopMirrorTimer(MirrorTimerType Type)
{
    m_MirrorTimer[Type] = -1;
    WorldPacket data(SMSG_STOP_MIRROR_TIMER, 4);
    data << (uint32)Type;
    SendPacket(&data);
}

void Player::EventDismount(uint32 money, float x, float y, float z)
{
    ModUnsigned32Value( PLAYER_FIELD_COINAGE , -(int32)money );

    SetPosition(x, y, z, true);
    if(!m_taxiPaths.size())
        SetTaxiState(false);

    SetTaxiPath(NULL);
    UnSetTaxiPos();
    m_taxi_ride_time = 0;

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    m_movementInterface.OnTaxiEnd();

    sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_PLAYER_TAXI_INTERPOLATE);

    // Save to database on dismount
    SaveToDB(false);

    // If we have multiple "trips" to do, "jump" on the next one :p
    if(m_taxiPaths.size())
    {
        TaxiPath * p = *m_taxiPaths.begin();
        m_taxiPaths.erase(m_taxiPaths.begin());
        TaxiStart(p, taxi_model_id, 0);
    }
}

void Player::_EventAttack( bool offhand )
{
/*    if (m_currentSpell)
    {
        if(m_currentSpell->GetSpellProto()->IsSpellChannelSpell()) // this is a channeled spell - ignore the attack event
            return;

        m_currentSpell->cancel();
        setAttackTimer(500, offhand);
        return;
    }

    if( IsFeared() || IsStunned() )
        return;

    Unit* pVictim = NULL;
    if(m_curSelection)
        pVictim = GetMapMgr()->GetUnit(m_curSelection);

    // Can't find victim, stop attacking
    if (!pVictim || !sFactionSystem.isAttackable( this, pVictim ) )
    {
        sLog.outDebug("Player::Update:  No valid current selection to attack, stopping attack.");
        smsg_AttackStop(pVictim);
        setHRegenTimer(5000); //prevent clicking off creature for a quick heal
        EventAttackStop();
        return;
    }

    if (!canReachWithAttack(pVictim))
    {
        if(m_AttackMsgTimer != 1)
        {
            m_session->OutPacket(SMSG_ATTACKSWING_NOTINRANGE);
            m_AttackMsgTimer = 1;
        }
        setAttackTimer(300, offhand);
    }
    else if(!isTargetInFront(pVictim))
    {
        // We still have to do this one.
        if(m_AttackMsgTimer != 2)
        {
            m_session->OutPacket(SMSG_ATTACKSWING_BADFACING);
            m_AttackMsgTimer = 2;
        }
        setAttackTimer(300, offhand);
    }
    else
    {
        m_AttackMsgTimer = 0;

        // Set to weapon time.
        setAttackTimer(0, offhand);

        if(InStealth())
        {
            RemoveAura( m_stealth );
            SetStealth(0);
        }

        if (!GetOnMeleeSpell() || offhand)
            Strike( pVictim, ( offhand ? OFFHAND : MELEE ), NULL, 0, 0, 0, false, false, true);
        else
            CastOnMeleeSpell();
    }*/
}

void Player::_EventCharmAttack()
{
    if(!m_CurrentCharm)
        return;

    Unit* pVictim = NULL;
    if(!IsInWorld())
    {
        m_CurrentCharm=NULL;
        sEventMgr.RemoveEvents(castPtr<Player>(this),EVENT_PLAYER_CHARM_ATTACK);
        return;
    }

    if(m_curSelection.empty())
    {
        sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_PLAYER_CHARM_ATTACK);
        return;
    }

    pVictim= GetMapMgr()->GetUnit(m_curSelection);

    //Can't find victim, stop attacking
    if (!pVictim)
    {
        sLog.Debug( "WORLD"," "I64FMT" doesn't exist.",m_curSelection);
        sLog.outDebug("Player::Update:  No valid current selection to attack, stopping attack\n");
        setHRegenTimer(5000); //prevent clicking off creature for a quick heal
        clearStateFlag(UF_ATTACKING);
        EventAttackStop();
    }
    else
    {
        if (!m_CurrentCharm->canReachWithAttack(MELEE, pVictim))
        {
            if(m_AttackMsgTimer == 0)
            {
                //m_session->OutPacket(SMSG_ATTACKSWING_NOTINRANGE);
                m_AttackMsgTimer = 2000;        // 2 sec till next msg.
            }
            // Shorten, so there isnt a delay when the client IS in the right position.
            sEventMgr.ModifyEventTimeLeft(castPtr<Player>(this), EVENT_PLAYER_CHARM_ATTACK, 100);
        }
        else if(!m_CurrentCharm->isTargetInFront(pVictim))
        {
            if(m_AttackMsgTimer == 0)
            {
                m_session->OutPacket(SMSG_ATTACKSWING_BADFACING);
                m_AttackMsgTimer = 2000;        // 2 sec till next msg.
            }
            // Shorten, so there isnt a delay when the client IS in the right position.
            sEventMgr.ModifyEventTimeLeft(castPtr<Player>(this), EVENT_PLAYER_CHARM_ATTACK, 100);
        }
        else
        {
            //if(pVictim->GetTypeId() == TYPEID_UNIT)
            //  pVictim->GetAIInterface()->StopMovement(5000);

            //pvp timeout reset
            /*if(pVictim->IsPlayer())
            {
                if( castPtr<Player>( pVictim )->DuelingWith == NULL)//Dueling doesn't trigger PVP
                    castPtr<Player>( pVictim )->PvPTimeoutUpdate(false); //update targets timer

                if(DuelingWith == NULL)//Dueling doesn't trigger PVP
                    PvPTimeoutUpdate(false); //update casters timer
            }*/

            if (!m_CurrentCharm->GetOnMeleeSpell())
            {
                m_CurrentCharm->Strike( pVictim, MELEE, NULL, 0, 0, 0, false, false, true );
            }
            else
            {
                SpellEntry *spellInfo = dbcSpell.LookupEntry(m_CurrentCharm->GetOnMeleeSpell());
                uint8 cn = m_meleespell_cn;
                m_CurrentCharm->SetOnMeleeSpell(0, 0);
                SpellCastTargets targets(GetSelection());
                if(Spell* spell = new Spell(m_CurrentCharm,spellInfo,cn))
                    spell->prepare(&targets, true);
            }
        }
    }
}

void Player::EventAttackStart()
{
    m_attacking = true;
    if( IsMounted() )
        Dismount();
}

void Player::EventAttackStop()
{
    if( m_CurrentCharm != NULL )
        sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_PLAYER_CHARM_ATTACK);

    m_attacking = false;
}

void Player::_EventExploration()
{
    if(!IsInWorld())
        return;
    if(m_position.x > _maxX || m_position.x < _minX || m_position.y > _maxY || m_position.y < _minY)
        return;
    if(GetMapMgr()->GetCellByCoords(GetPositionX(),GetPositionY()) == NULL)
        return;
    if(m_lastAreaUpdateMap == GetMapId() && m_lastAreaPosition.Distance(GetPosition()) < sWorld.AreaUpdateDistance)
        return;
    m_lastAreaUpdateMap = GetMapId();
    m_lastAreaPosition = GetPosition();

    m_oldZone = m_zoneId;
    m_oldArea = m_areaId;
    UpdateAreaInfo();

    bool restmap = false;
    World::RestedAreaInfo* restinfo = sWorld.GetRestedMapInfo(GetMapId());
    if(restinfo != NULL && (restinfo->ReqTeam == -1 || restinfo->ReqTeam == GetTeam()))
        restmap = true;

    if(m_zoneId == 0xFFFF)
    {
        // Clear our worldstates when we have no data.
        if(m_oldZone != 0xFFFF)
            GetMapMgr()->GetStateManager().ClearWorldStates(this);
        // This must be called every update, to keep data fresh.
        EventDBCChatUpdate(0xFFFFFFFF);
    }
    else if( m_oldZone != m_zoneId )
    {
        sWeatherMgr.SendWeather(this);

        m_AuraInterface.RemoveAllAurasByInterruptFlag( AURA_INTERRUPT_ON_LEAVE_AREA );

        m_playerInfo->lastZone = m_zoneId;

        TRIGGER_INSTANCE_EVENT( GetMapMgr(), OnZoneChange )( castPtr<Player>(this), m_zoneId, m_oldZone );

        EventDBCChatUpdate(0xFFFFFFFF);

        GetMapMgr()->GetStateManager().SendWorldStates(this);
    }

    if(m_areaFlags & OBJECT_AREA_FLAG_INDOORS)
    {
        //Mount expired?
        if(IsMounted())
        {
            switch(m_mapId)
            {
            case 531:
                {
                    // Qiraj battletanks work everywhere on map 531
                    if(m_MountSpellId == 25953 || m_MountSpellId == 26054 || m_MountSpellId == 26055 || m_MountSpellId == 26056)
                        break;
                }
            default:
                Dismount();
                break;
            }
        }

        // Now remove all auras that are only usable outdoors (e.g. Travel form)
        m_AuraInterface.RemoveAllAurasWithAttributes(0, ATTRIBUTES_ONLY_OUTDOORS);
    }
    if(!m_areaId || m_areaId == 0xFFFF)
    {
        if(m_FlyingAura)
            RemoveAura(m_FlyingAura); // remove flying buff
        HandleRestedCalculations(restmap);
        return;
    }

    AreaTableEntry* at = dbcAreaTable.LookupEntry(m_areaId);
    if(at == NULL)
        at = dbcAreaTable.LookupEntry(m_zoneId); // These maps need their own chat channels.
    if(at == NULL)
    {
        if(m_FlyingAura)
            RemoveAura(m_FlyingAura); // remove flying buff
        HandleRestedCalculations(restmap);
        return;
    }

    if(m_FlyingAura && !(at->AreaFlags & AREA_FLYING_PERMITTED))
        RemoveAura(m_FlyingAura); // remove flying buff

    UpdatePvPArea();

    if(HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY))
    {
        Unit* pUnit = GetSelection() ? m_mapMgr->GetUnit(GetSelection()) : NULL;
        if(pUnit && !sFactionSystem.isAttackable(this, pUnit))
        {
            EventAttackStop();
            smsg_AttackStop(pUnit);
        }

        if(DuelingWith != NULL)
            DuelingWith->EndDuel(DUEL_WINNER_RETREAT);
    }

    TRIGGER_INSTANCE_EVENT( m_mapMgr, OnChangeArea )( this, m_zoneId, m_areaId, m_oldArea );

    // bur: we dont want to explore new areas when on taxi
    if(!GetTaxiState() && !GetTransportGuid())
    {
        uint32 offset = at->explorationFlag / 32;
        if(offset < 156)
        {
            offset += PLAYER_EXPLORED_ZONES_1;

            uint32 val = (uint32)(1 << (at->explorationFlag % 32));
            uint32 currFields = GetUInt32Value(offset);
            if(!(currFields & val))//Unexplored Area
            {
                SetUInt32Value(offset, (uint32)(currFields | val));

                uint32 explore_xp = at->level * 10 * sWorld.getRate(RATE_XP);
                WorldPacket data(SMSG_EXPLORATION_EXPERIENCE, 8);
                data << at->AreaId << explore_xp;
                m_session->SendPacket(&data);

                if(getLevel() < GetUInt32Value(PLAYER_FIELD_MAX_LEVEL) && explore_xp)
                    GiveXP(explore_xp, 0, false);
            }
        }

        sQuestMgr.OnPlayerExploreArea(this, at->AreaId);
    }

    // Check for a restable area
    bool rest_on = restmap;
    restinfo = sWorld.GetRestedAreaInfo(at->AreaId);
    if(restinfo)
    {
        if(restinfo->ReqTeam == -1 || restinfo->ReqTeam == GetTeam())
            rest_on = true;
        else
            rest_on = false;
    }
    else if(at->ZoneId) // Crow: Shouldn't have to do this.
    {
        //second AT check for subzones.
        restinfo = sWorld.GetRestedAreaInfo(at->ZoneId);
        if(restinfo)
        {
            if(restinfo->ReqTeam == -1 || restinfo->ReqTeam == GetTeam())
                rest_on = true;
            else
                rest_on = false;
        }
    }

    HandleRestedCalculations(rest_on);
}

void Player::EventDeath()
{
    if (m_state & UF_ATTACKING)
        EventAttackStop();

    if (m_onTaxi)
        sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_PLAYER_TAXI_DISMOUNT);

    if(!IS_INSTANCE(GetMapId()) && !sEventMgr.HasEvent(castPtr<Player>(this),EVENT_PLAYER_FORCED_RESURECT)) //Should never be true
        sEventMgr.AddEvent(castPtr<Player>(this),&Player::EventRepopRequestedPlayer,EVENT_PLAYER_FORCED_RESURECT,PLAYER_FORCED_RESURECT_INTERVAL,1,0); //in case he forgets to release spirit (afk or something)
}

///  This function sends the message displaying the purple XP gain for the char
///  It assumes you will send out an UpdateObject packet at a later time.
void Player::GiveXP(uint32 xp, const uint64 &guid, bool allowbonus)
{
    if ( xp < 1 || m_XPoff)
        return;
    if(getLevel() >= GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
        return;

    uint32 restxp = 0; //add reststate bonus
    if(m_restState == RESTSTATE_RESTED && allowbonus)
    {
        restxp = SubtractRestXP(xp);
        xp += restxp;
    }

    UpdateRestState();
    SendLogXPGain(guid, xp, restxp, guid == 0 ? true : false);

    int32 newxp = GetUInt32Value(PLAYER_XP) + xp;
    uint32 level = GetUInt32Value(UNIT_FIELD_LEVEL);
    int32 nextlevelxp = sWorld.GetXPToNextLevel(level);
    uint32 hpGain = 0, manaGain = 0, statGain[5] = {0, 0, 0, 0, 0};
    UnitBaseStats *stats = baseStats;
    bool levelup = false;

    while(newxp >= nextlevelxp && newxp > 0)
    {
        ++level;
        newxp -= nextlevelxp;
        nextlevelxp = sWorld.GetXPToNextLevel(level);
        if(UnitBaseStats *new_stats = sStatSystem.GetUnitBaseStats(getRace(), getClass(), level))
        {
            hpGain += new_stats->baseHP-stats->baseHP;
            manaGain += new_stats->basePower-stats->basePower;
            for(uint8 i = 0; i < 5; i++)
                statGain[i] += new_stats->baseStat[i]-stats->baseStat[i];
            stats = new_stats;
        }
        levelup = true;

        if(level >= GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
            break;
    }

    if(level > GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
        level = GetUInt32Value(PLAYER_FIELD_MAX_LEVEL);

    if(levelup)
    {
        setLevel(level);
        SendLevelupInfo(level, hpGain, manaGain, statGain);
    }

    // Set the update bit
    SetUInt32Value(PLAYER_XP, newxp);
}

void Player::smsg_InitialSpells()
{
    PlayerCooldownMap::iterator itr, itr2;

    uint16 spellCount = (uint16)mSpells.size();
    size_t itemCount = m_cooldownMap[0].size() + m_cooldownMap[1].size();
    uint32 mstime = getMSTime();
    size_t pos;

    WorldPacket data(SMSG_INITIAL_SPELLS, 5 + (spellCount * 4) + (itemCount * 4) );
    data << uint8(0);
    data << uint16(spellCount); // spell count

    SpellSet::iterator sitr;
    for (sitr = mSpells.begin(); sitr != mSpells.end(); ++sitr)
    {
        // todo: check out when we should send 0x0 and when we should send 0xeeee
        // this is not slot, values is always eeee or 0, seems to be cooldown
        data << uint32(*sitr);                 // spell id
        data << uint16(0x0000);
    }

    pos = data.wpos();
    data << uint16(0);        // placeholder

    itemCount = 0;
    for( itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].begin(); itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end(); )
    {
        itr2 = itr++;

        // don't keep around expired cooldowns
        if( itr2->second.ExpireTime < mstime || (itr2->second.ExpireTime - mstime) < 10000 )
        {
            m_cooldownMap[COOLDOWN_TYPE_SPELL].erase( itr2 );
            continue;
        }

        data << uint32( itr2->second.SpellId );             // spell id
        data << uint32( itr2->second.ItemId );              // item id
        data << uint16( 0 );                                // spell category
        data << uint32( itr2->second.ExpireTime - mstime ); // cooldown remaining in ms (for spell)
        data << uint32( 0 );                                // cooldown remaining in ms (for category)

        ++itemCount;

        sLog.outDebug("sending spell cooldown for spell %u to %u ms", itr2->first, itr2->second.ExpireTime - mstime);
    }

    for( itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].begin(); itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end(); )
    {
        itr2 = itr++;

        // don't keep around expired cooldowns
        if( itr2->second.ExpireTime < mstime || (itr2->second.ExpireTime - mstime) < 10000 )
        {
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase( itr2 );
            continue;
        }

        data << uint32( itr2->second.SpellId );             // spell id
        data << uint32( itr2->second.ItemId );              // item id
        data << uint16( itr2->first );                      // spell category
        data << uint32( 0 );                                // cooldown remaining in ms (for spell)
        data << uint32( itr2->second.ExpireTime - mstime ); // cooldown remaining in ms (for category)

        ++itemCount;

        sLog.outDebug("InitialSpells", "sending category cooldown for cat %u to %u ms", itr2->first, itr2->second.ExpireTime - mstime);
    }

    *(uint16*)&data.contents()[pos] = (uint16)itemCount;

    GetSession()->SendPacket(&data);
}

void Player::_SavePet(QueryBuffer * buf)
{
    // Remove any existing info
    if(buf == NULL)
    {
        CharacterDatabase.Execute("DELETE FROM playerpets WHERE ownerguid = %u", GetUInt32Value(OBJECT_FIELD_GUID));
        CharacterDatabase.Execute("DELETE FROM playerpettalents WHERE ownerguid=%u", GetLowGUID());
        CharacterDatabase.Execute("DELETE FROM playerpetactionbar WHERE ownerguid=%u", GetLowGUID());
        CharacterDatabase.Execute("DELETE FROM playerpetspells WHERE ownerguid=%u", GetLowGUID());
    }
    else
    {
        buf->AddQuery("DELETE FROM playerpets WHERE ownerguid = %u", GetUInt32Value(OBJECT_FIELD_GUID));
        buf->AddQuery("DELETE FROM playerpetactionbar WHERE ownerguid=%u", GetLowGUID());
        buf->AddQuery("DELETE FROM playerpetspells WHERE ownerguid=%u", GetLowGUID());
        buf->AddQuery("DELETE FROM playerpettalents WHERE ownerguid=%u", GetLowGUID());
    }

    std::stringstream ss;
    if(m_Summon && m_Summon->IsInWorld() && m_Summon->GetPetOwner() == castPtr<Player>(this)) // update PlayerPets array with current pet's info
    {
        PlayerPet*pPet = GetPlayerPet(m_Summon->m_PetNumber);
        if(!pPet || pPet->active == false)
            m_Summon->UpdatePetInfo(true);
        else
            m_Summon->UpdatePetInfo(false);

        if(!m_Summon->Summon)      // is a pet
        {
            // save pet spellz
            PetSpellMap::iterator itr = m_Summon->mSpells.begin();
            uint32 pn = m_Summon->m_PetNumber;

            // Start inserting spells into a bulk SQL
            ss.rdbuf()->str("");
            bool first = true;
            if(m_Summon->mSpells.size())
            {
                ss << "INSERT INTO playerpetspells VALUES ";
                for(; itr != m_Summon->mSpells.end(); itr++)
                {
                    if(first)
                        first = false;
                    else
                        ss << ",";

                    ss << "(" << uint32(GetLowGUID()) << ", " << uint32(pn) << ", " << uint32(itr->first->Id) << ", " <<  uint32(itr->second) << ")";
                }
                ss << ";";

                if(buf == NULL)
                    CharacterDatabase.Execute(ss.str().c_str());
                else // Execute or add our bulk inserts
                    buf->AddQuery(ss.str().c_str());
            }

            // Start inserting talents into a bulk SQL
            ss.rdbuf()->str("");
            if(m_Summon->m_talents.size())
            {
                PetTalentMap::iterator itr2 = m_Summon->m_talents.begin();
                ss << "INSERT INTO playerpettalents VALUES ";
                first = true;
                for(; itr2 != m_Summon->m_talents.end(); itr2++)
                {
                    if(first)
                        first = false;
                    else
                        ss << ",";

                    ss << "(" << uint32(GetLowGUID()) << ", " << uint32(pn) << ", " << uint32(itr2->first) << ", " <<  uint32(itr2->second) << ")";
                }
                ss << ";";

                if(buf == NULL)
                    CharacterDatabase.Execute(ss.str().c_str());
                else // Execute or add our bulk inserts
                    buf->AddQuery(ss.str().c_str());
            }
        }
    }

    PetLocks.Acquire();
    for(std::map<uint32, PlayerPet*>::iterator itr = m_Pets.begin(); itr != m_Pets.end(); itr++)
    {
        ss.rdbuf()->str("");
        ss << "REPLACE INTO playerpets VALUES('"
            << GetLowGUID() << "','"
            << itr->second->number << "','"
            << CharacterDatabase.EscapeString(itr->second->name).c_str() << "','"
            << itr->second->entry << "','"
            << itr->second->fields.c_str() << "','"
            << itr->second->xp << "','"
            << uint32((itr->second->active ?  1 : 0) + itr->second->stablestate * 10) << "','"
            << itr->second->level << "','"
            << itr->second->happiness << "','" //happiness/loyalty xp
            << itr->second->happinessupdate << "','"
            << uint32(itr->second->summon ?  1 : 0) << "')";

        if(buf == NULL)
            CharacterDatabase.ExecuteNA(ss.str().c_str());
        else
            buf->AddQueryStr(ss.str());

        ss.rdbuf()->str("");
        ss << "REPLACE INTO playerpetactionbar VALUES('";
        // save action bar
        ss << GetLowGUID() << "','"
        << itr->second->number << "'";
        for(uint8 i = 0; i < 4; i++)
            ss << ", '" << itr->second->actionbarspell[i] << "'";

        for(uint8 i = 0; i < 4; i++)
            ss << ", '" << itr->second->actionbarspellstate[i] << "'";

        ss << ")";

        if(buf == NULL)
            CharacterDatabase.ExecuteNA(ss.str().c_str());
        else
            buf->AddQueryStr(ss.str());
    }
    PetLocks.Release();
}

void Player::_LoadPet(QueryResult * result)
{
    m_PetNumberMax= 0;
    if(!result)
        return;

    do
    {
        Field *fields = result->Fetch();
        PlayerPet* pet = NULL;
        pet = new PlayerPet;
        pet->number  = fields[1].GetUInt32();
        pet->name   = fields[2].GetString();
        pet->entry   = fields[3].GetUInt32();
        pet->fields  = fields[4].GetString();
        pet->xp   = fields[5].GetUInt32();
        pet->active  = fields[6].GetInt8()%10 > 0 ? true : false;
        pet->stablestate = fields[6].GetInt8() / 10;
        pet->level   = fields[7].GetUInt32();
        pet->happiness = fields[8].GetUInt32();
        pet->happinessupdate = fields[9].GetUInt32();
        pet->summon = (fields[10].GetUInt32()>0 ? true : false);

        m_Pets[pet->number] = pet;
        if(pet->active)
        {
            if(iActivePet)  // how the hell can this happen
            {
                //printf("pet warning - >1 active pet.. weird..");
            } else iActivePet = pet->number;
        }

        if(pet->number > m_PetNumberMax)
            m_PetNumberMax =  pet->number;
    }while(result->NextRow());
}

void Player::_LoadPetActionBar(QueryResult * result)
{
    if(!result)
        return;

    if(!m_Pets.size())
        return;

    do
    {
        Field *fields = result->Fetch();
        PlayerPet* pet = NULL;
        uint32 number  = fields[1].GetUInt32();
        pet = m_Pets[number];
        if(!pet)
            continue;

        for(uint8 i = 0; i < 4; i++)
        {
            pet->actionbarspell[i] = fields[2+i].GetUInt32();
            pet->actionbarspellstate[i] = fields[6+i].GetUInt32();
        }
    }while(result->NextRow());
}

void Player::SpawnPet(uint32 pet_number)
{
    PetLocks.Acquire();
    std::map<uint32, PlayerPet*>::iterator itr = m_Pets.find(pet_number);
    if(itr == m_Pets.end())
    {
        PetLocks.Release();
        sLog.outDebug("PET SYSTEM: "I64FMT" Tried to load invalid pet %d", GetGUID(), pet_number);
        return;
    }

    if( m_Summon != NULL )
    {
        m_Summon->Remove(true, true, true);
        m_Summon = NULL;
    }

    if(CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(itr->second->entry))
    {
        // Crow: Should be that it recasts summon spell, but without cost.
        if(Pet* pPet = objmgr.CreatePet(ctrData))
        {
            pPet->SetInstanceID(GetInstanceID());
            pPet->LoadFromDB(castPtr<Player>(this), itr->second);
            if( IsPvPFlagged() )
                pPet->SetPvPFlag();
        }
    }
    PetLocks.Release();
}

SpellEntry* Player::FindLowerRankSpell(SpellEntry* sp, int32 rankdiff)
{
    SpellSet::iterator itr;
    SpellEntry* spell = NULL;
    if(sp->RankNumber)
    {
        for(itr = mSpells.begin(); itr != mSpells.end(); itr++)
        {
            spell = NULL;
            if((spell = dbcSpell.LookupEntry(*itr)) != NULL)
            {
                if(spell->NameHash == sp->NameHash)
                    if(sp->RankNumber + rankdiff == spell->RankNumber)
                        return spell;
            }
            spell = NULL;
        }
    }

    return spell;
}

void Player::addSpell(uint32 spell_id)
{
    SpellSet::iterator iter = mSpells.find(spell_id);
    if(iter != mSpells.end())
        return;
    if(spell_id == 3018 && (getClass() == DEATHKNIGHT || getClass() == WARLOCK 
        || getClass() == PRIEST || getClass() == MAGE || getClass() == PALADIN))
        return;
    SpellEntry* spell = dbcSpell.LookupEntry(spell_id);
    if(spell == NULL)
        return;

    mSpells.insert(spell_id);

    // Add the skill line for this spell if we don't already have it.
    SkillLineAbilityEntry *sk = objmgr.GetSpellSkill(spell_id);
    if(sk && !_HasSkillLine(sk->skilline))
    {
        SkillLineEntry * skill = dbcSkillLine.LookupEntry(sk->skilline);
        uint32 max = 1;
        switch(skill->categoryId)
        {
        case SKILL_TYPE_PROFESSION:
            max=75*((spell->RankNumber)+1);
            break;
        case SKILL_TYPE_SECONDARY:
            max=75*((spell->RankNumber)+1);
            break;
        case SKILL_TYPE_WEAPON:
            max=5*getLevel();
            break;
        case SKILL_TYPE_CLASS:
        case SKILL_TYPE_ARMOR:
            if(skill->id == SKILL_LOCKPICKING)
                max=5*getLevel();
            break;
        }

        if(sWorld.StartLevel > 1 && skill->categoryId != SKILL_TYPE_PROFESSION && skill->categoryId != SKILL_TYPE_SECONDARY)
            _AddSkillLine(sk->skilline, sWorld.StartLevel*5, max);
        else
            _AddSkillLine(sk->skilline, 1, max);
        _UpdateMaxSkillCounts();
    }

    // Check if we're logging in.
    if(!IsInWorld())
        return;

    SpellEntry* sp2 = FindLowerRankSpell(spell, -1);
    if(sp2 != NULL && sp2->Id == forget)
    {
        WorldPacket data(SMSG_SUPERCEDED_SPELL, 8);
        data << sp2->Id << spell_id;
        m_session->SendPacket(&data);
    }
    else
    {
        WorldPacket data(SMSG_LEARNED_SPELL, 6);
        data << spell_id << uint16(0);
        m_session->SendPacket(&data);
    }
}

void Player::DestroyForPlayer( Player* target, bool anim )
{
    WorldObject::DestroyForPlayer( target, anim );
    if(GetInventory())
        GetInventory()->DestroyForPlayer(target);
}

bool Player::canCast(SpellEntry *m_spellInfo)
{
    if (m_spellInfo->EquippedItemClass != 0)
    {
        if( disarmed )
            return false;

        if(m_spellInfo->EquippedItemClass == 4)
        {
            Item* item = NULL;
            switch(m_spellInfo->EquippedItemSubClass)
            {
            case 64:
                {
                    if((item = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND)) != NULL)
                    {
                        if(item->GetProto() && item->GetProto()->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD)
                            return false;
                    }
                    else return false;
                }break;
            default:
                {
                    printf("Unknown Equipped Item Requirements: %u/%u\n", m_spellInfo->EquippedItemClass, m_spellInfo->EquippedItemSubClass);
                }break;
            }
        }
        else // We want a weapon, but is this even required? Some weapon requiring spells don't have any data...
        {
            if(GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
            {
                if((int32)GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)->GetProto()->Class == m_spellInfo->EquippedItemClass)
                {
                    if (m_spellInfo->EquippedItemSubClass != 0)
                    {
                        if (m_spellInfo->EquippedItemSubClass != 173555 && m_spellInfo->EquippedItemSubClass != 96 && m_spellInfo->EquippedItemSubClass != 262156)
                        {
                            if (!((1 << GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)->GetProto()->SubClass)
                                & m_spellInfo->EquippedItemSubClass))
                                return false;
                        }
                    }
                }
            }
            else if(m_spellInfo->EquippedItemSubClass == 173555)
                return false;

            if (GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED))
            {
                if((int32)GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)->GetProto()->Class == m_spellInfo->EquippedItemClass)
                {
                    if (m_spellInfo->EquippedItemSubClass != 0)
                    {
                        if (m_spellInfo->EquippedItemSubClass != 173555 && m_spellInfo->EquippedItemSubClass != 96 && m_spellInfo->EquippedItemSubClass != 262156)
                        {
                            if (!((1 << GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)->GetProto()->SubClass)
                                & m_spellInfo->EquippedItemSubClass))
                                return false;
                        }
                    }
                }
            }
            else if (m_spellInfo->EquippedItemSubClass == 262156)
                return false;
        }
    }
    return true;
}

void Player::RemovePendingPlayer(uint8 reason)
{
    if(m_session)
    {
        m_session->OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &reason);
        m_session->m_loggingInPlayer = NULL;
    }

    ok_to_remove = true;
    Destruct();
}

bool Player::HasSpell(uint32 spell)
{
    return mSpells.find(spell) != mSpells.end();
}

SpellEntry* Player::GetSpellWithNamehash(uint32 namehash)
{
    SpellSet::iterator itr;
    SpellEntry* sp = NULL;
    for(itr = mSpells.begin(); itr != mSpells.end(); itr++)
    {
        sp = NULL;
        if(sp = dbcSpell.LookupEntry(*itr))
        {
            if(sp->NameHash == namehash)
                return sp;
        }
        sp = NULL;
    }
    return NULL;
}

bool Player::HasHigherSpellForSkillLine(SpellEntry* sp)
{
    if(sp->NameHash == SPELL_HASH_COLD_WEATHER_FLYING)
        return false;   // Cold Weather Hackfix
    if(sp->NameHash == SPELL_HASH_FLIGHT_FORM)
        if(HasSpell(40120))
            return true; // We should only have 1 flight form.
    if(sp->NameHash == SPELL_HASH_BEAR_FORM)
        if(HasSpell(9634))
            return true; // We should only have 1 bear form.

    uint32 oskillline = sp->SpellSkillLine;
    if(oskillline == 0)
        return false;

    SkillLineEntry* sle = dbcSkillLine.LookupEntry(oskillline);
    if(sle == NULL)
        return false;

    if(sle->categoryId == SKILL_TYPE_PROFESSION)
        return false;

    if(GetSpellClass(sp))
        return false;

    SpellSet::iterator itr;
    SpellEntry* spell = NULL;
    if(_HasSkillLine(oskillline))
    {
        for(itr = mSpells.begin(); itr != mSpells.end(); itr++)
        {
            if((spell = dbcSpell.LookupEntry(*itr)) != NULL)
            {
                if(spell->SpellSkillLine == oskillline)
                    if(spell->RankNumber > sp->RankNumber)
                        return true;
            }
        }
    }

    return false;
}

uint32 Player::FindSpellWithNamehash(uint32 namehash)
{
    SpellSet::iterator itr = mSpells.begin();
    if(itr != mSpells.end())
    {
        for(itr; itr != mSpells.end(); itr++)
        {
            SpellEntry* sp = dbcSpell.LookupEntry(*itr);
            if(sp && sp->NameHash == namehash)
                return (*itr);
        }
    }
    return 0;
}

uint32 Player::FindHigherRankingSpellWithNamehash(uint32 namehash, uint32 minimumrank)
{
    SpellEntry* sp = NULL;
    SpellSet::iterator itr = mSpells.begin();
    if(itr != mSpells.end())
    {
        for(itr; itr != mSpells.end(); itr++)
        {
            sp = dbcSpell.LookupEntry(*itr);
            if(sp && sp->NameHash == namehash && sp->RankNumber > minimumrank)
                return (*itr);
        }
    }
    return 0;
}

uint32 Player::FindHighestRankingSpellWithNamehash(uint32 namehash)
{
    if(!mSpells.size())
        return 0;

    SpellSet::iterator itr;
    uint32 rank = 0, spellid = 0;
    for(itr = mSpells.begin(); itr != mSpells.end(); itr++)
    {
        SpellEntry* sp = dbcSpell.LookupEntry(*itr);
        if(sp->NameHash != namehash)
            continue;
        if(sp->RankNumber == 0 && spellid != 0)
            continue;
        if(sp->RankNumber <= rank)
            continue;
        if(sp && sp->RankNumber && sp->NameHash == namehash && sp->RankNumber > rank)
        {
            rank = sp->RankNumber;
            spellid = (*itr);
        }
    }
    return spellid;
}

// Use instead of cold weather flying
bool Player::CanFlyInCurrentZoneOrMap()
{
    AreaTableEntry *area = dbcAreaTable.LookupEntry(GetAreaId());
    if(area == NULL || !(area->AreaFlags & AREA_FLYING_PERMITTED))
        return false; // can't fly in non-flying zones

    if(GetMapId() == 530)
        return true; // We can fly in outlands all the time

    if(GetMapId() == 571)
    {
        if(HasDummyAura(SPELL_HASH_COLD_WEATHER_FLYING) || HasSpell(54197))
            return true;
    }

    return false;
}

QuestLogEntry* Player::GetQuestLogForEntry(uint32 quest)
{
    for(uint8 i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if(m_questlog[i] == NULL)
            continue;
        if(m_questlog[i]->GetQuest()->id == quest)
            return m_questlog[i];
    }
    return NULL;
}

void Player::SetQuestLogSlot(QuestLogEntry *entry, uint32 slot)
{
    ASSERT(slot < QUEST_LOG_COUNT);
    m_questlog[slot] = entry;
}

void Player::AddToWorld(bool loggingin /* = false */)
{
    FlyCheat = false;

    // check transporter
    if(m_movementInterface.OnTransport() && m_CurrentTransporter)
    {
        LocationVector pos;
        m_movementInterface.GetTransportPosition(pos);
        SetPosition(m_CurrentTransporter->GetPositionX() + pos.x,
            m_CurrentTransporter->GetPositionY() + pos.y,
            m_CurrentTransporter->GetPositionZ() + pos.z,
            GetOrientation());
    }

    if(IsInWorld())
        return;

    m_beingPushed = true;
    WorldObject::AddToWorld();

    // Add failed.
    if(m_mapMgr == NULL)
    {
        sLog.Debug("WorldSession","Adding player %s to map %u failed.",GetName(),GetMapId());
        // eject from instance
        m_beingPushed = false;
        EjectFromInstance();
        return;
    }

    if(m_session)
        m_session->SetInstance(m_mapMgr->GetInstanceID());

    EventHealthChangeSinceLastUpdate(); // just in case
}

void Player::OnPrePushToWorld()
{
    SendInitialLogonPackets();
}

void Player::OnPushToWorld()
{
    // Process create packet
    PopPendingUpdates();

    if(m_TeleportState == 2)   // Worldport Ack
        OnWorldPortAck();

    m_beingPushed = false;
    m_inventory.AddToWorld();
    GetMovementInterface()->UnlockTransportData();

    // delay the unlock movement packet
    WorldPacket data(SMSG_TIME_SYNC_REQ, 4);
    data << uint32(0);
    SendPacket(&data);
    sWorld.mInWorldPlayerCount++;

    // Login spell
    CastSpell(this, 836, true);

    Unit::OnPushToWorld();

    // Update PVP Situation
    LoginPvPSetup();

    // Send our auras
    data.Initialize(SMSG_AURA_UPDATE_ALL);
    data << GetGUID().asPacked();
    m_AuraInterface.BuildAuraUpdateAllPacket(&data);
    SendPacket(&data);

    if(m_FirstLogin)
    {
        sEventMgr.AddEvent(this, &Player::FullHPMP, EVENT_PLAYER_FULL_HPMP, 200, 0, 0);
        m_FirstLogin = false;
    }

    // send world states
    if( m_mapMgr != NULL )
        m_mapMgr->GetStateManager().SendWorldStates(castPtr<Player>(this));

    TRIGGER_INSTANCE_EVENT( m_mapMgr, OnZoneChange )( castPtr<Player>(this), m_zoneId, 0 );
    TRIGGER_INSTANCE_EVENT( m_mapMgr, OnPlayerEnter )( castPtr<Player>(this) );

    if(m_TeleportState == 1)        // First world enter
        CompleteLoading();

    m_TeleportState = 0;

    if(GetTaxiState())
    {
        if( m_taxiMapChangeNode != 0 )
        {
            lastNode = m_taxiMapChangeNode;
        }

        // Create HAS to be sent before this!
        TaxiStart(GetTaxiPath(),
            GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID),
            lastNode);

        m_taxiMapChangeNode = 0;
    }

    /* send weather */
    sWeatherMgr.SendWeather(castPtr<Player>(this));

    if( load_health > 0 )
    {
        if( load_health > GetUInt32Value(UNIT_FIELD_MAXHEALTH) )
            SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH) );
        else SetUInt32Value(UNIT_FIELD_HEALTH, load_health);
    }

    for(uint8 i = 0; i < POWER_TYPE_MAX; i++)
    {
        if(load_power[i] == 0)
            continue;
        if(load_power[i] > GetMaxPower(i))
            SetUInt32Value(UNIT_FIELD_POWERS+i, GetMaxPower(i));
        else SetUInt32Value(UNIT_FIELD_POWERS+i, load_power[i]);
    }

    if( m_mapMgr != NULL && m_mapMgr->m_battleground != NULL && m_bg != m_mapMgr->m_battleground )
    {
        m_bg = m_mapMgr->m_battleground;
        m_bg->PortPlayer( castPtr<Player>(this), true );
    }

    if( m_bg != NULL && m_mapMgr != NULL )
        m_bg->OnPlayerPushed( castPtr<Player>(this) );

    m_changingMaps = false;
}

void Player::OnWorldLogin()
{

}

void Player::SendObjectUpdate(WoWGuid guid)
{
    uint32 count = 1;
    WorldPacket data(SMSG_UPDATE_OBJECT, 200);
    data << uint16(GetMapId());
    data << count;
    if(guid == GetGUID())
    {
        count += WorldObject::BuildCreateUpdateBlockForPlayer(&data, this);
    }
    else if(IsInWorld())
    {
        WorldObject* obj = GetMapMgr()->_GetObject(guid);
        if(obj != NULL)
        {
            count += obj->BuildCreateUpdateBlockForPlayer(&data, this);
        }
    }
    else
    {
        // We aren't pushed yet.
        return;
    }

    printf("Sending update with size %u type %s %s\n", data.size(), (GetGUID() == guid ? "Player" : "Non Player"), (IsInWorld() ? "Is in world" : "Out of world"));
    data.put<uint32>(2, count);
    // send uncompressed because it's specified
    m_session->SendPacket(&data);
    PopPendingUpdates();
}

void Player::RemoveFromWorld()
{
    EndDuel( 0 );

    if( m_CurrentCharm && m_CurrentCharm != GetVehicle() )
        UnPossess();

    if( GetUInt64Value(UNIT_FIELD_CHARMEDBY) != 0 && IsInWorld() )
    {
        Player* charmer = m_mapMgr->GetPlayer(GetUInt64Value(UNIT_FIELD_CHARMEDBY));
        if( charmer != NULL )
            charmer->UnPossess();
    }

    m_resurrectHealth = 0;
    m_resurrectMana = 0;
    resurrector = 0;

    // just in case
    SetUInt32Value(UNIT_FIELD_FLAGS, GetUInt32Value(UNIT_FIELD_FLAGS) & ~UNIT_FLAG_SKINNABLE);

    //Cancel any forced teleport pending.
    if(raidgrouponlysent)
    {
        event_RemoveEvents(EVENT_PLAYER_EJECT_FROM_INSTANCE);
        raidgrouponlysent = false;
    }

    load_health = GetUInt32Value(UNIT_FIELD_HEALTH);
    for(uint8 i = 0; i < POWER_TYPE_MAX; i++)
        load_power[i] = GetPower(i);

    TRIGGER_INSTANCE_EVENT( m_mapMgr, OnChangeArea )( this, 0, 0, GetAreaId() );

    m_mapMgr->GetStateManager().ClearWorldStates(this);

    if(m_bg)
    {
        m_bg->RemovePlayer(castPtr<Player>(this), true);
        m_bg = NULL;
    }

    // Cancel trade if it's active.
    if(m_tradeData)
    {
        if(Player* pTarget = GetTradeTarget())
            pTarget->ResetTradeVariables();

        ResetTradeVariables();
    }

    if(m_Summon)
    {
        m_Summon->GetAIInterface()->SetPetOwner(0);
        m_Summon->Remove(true, true, false);
        if(m_Summon)
        {
            m_Summon->ClearPetOwner();
            m_Summon=NULL;
        }
    }

    if(m_SummonedObject)
    {
        if(m_SummonedObject->GetInstanceID() == GetInstanceID())
        {
            if(m_SummonedObject->IsInWorld())
                m_SummonedObject->RemoveFromWorld(true);
            m_SummonedObject->Destruct();
        } else sEventMgr.AddEvent(m_SummonedObject, &GameObject::Despawn, uint32(0), uint32(0), EVENT_GAMEOBJECT_EXPIRE, 100, 1,0);
        m_SummonedObject = NULL;
    }

    m_inventory.RemoveFromWorld(false);

    Unit::RemoveFromWorld(false);

    sWorld.mInWorldPlayerCount--;

    if(GetTaxiState())
        event_RemoveEvents( EVENT_PLAYER_TAXI_INTERPOLATE );

    if( m_CurrentTransporter && !m_movementInterface.isTransportLocked() )
    {
        m_CurrentTransporter->RemovePlayer(castPtr<Player>(this));
        m_CurrentTransporter = NULL;
        GetMovementInterface()->ClearTransportData();
    }

    if( GetVehicle() )
    {
        GetVehicle()->RemovePassenger(castPtr<Player>(this));
        SetVehicle(NULL);
    }

    m_changingMaps = true;
}

void Player::_ApplyItemMods(Item* item, uint8 slot, bool apply, bool justdrokedown /* = false */, bool skip_stat_apply /* = false  */)
{
    ASSERT( item );
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;
    ASSERT(item->GetOwner() == this);

    //fast check to skip mod applying if the item doesnt meat the requirements.
    if(item->GetUInt32Value( ITEM_FIELD_DURABILITY ) == 0 && item->GetUInt32Value( ITEM_FIELD_MAXDURABILITY ) && justdrokedown == false )
        return;

    ItemPrototype* proto = item->GetProto();
    if(proto->minDamage && proto->maxDamage)
    {
        ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_MINDAMAGE, ITEM_STAT_CUSTOM_DAMAGE_MIN, proto->minDamage);
        ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_MAXDAMAGE, ITEM_STAT_CUSTOM_DAMAGE_MAX, proto->maxDamage);
        ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_WEAPONDELAY, ITEM_STAT_CUSTOM_WEAPON_DELAY, proto->Delay);
    }
    if(proto->Armor) ModifyBonuses( apply, item->GetGUID(), MOD_SLOT_ARMOR, ITEM_STAT_PHYSICAL_RESISTANCE, proto->Armor);

    // Stats
    for( uint8 i = 0; i < 10; i++ )
        if(int32 val = proto->Stats[i].Value)
            ModifyBonuses( apply, item->GetGUID(), i, proto->Stats[i].Type, val);

    for( uint8 k = 0; k < 5; k++ )
    {
        if( item->GetProto()->Spells[k].Id == 0 )
            continue;

        if( item->GetProto()->Spells[k].Trigger == ON_EQUIP )
        {
            if(SpellEntry *spells = dbcSpell.LookupEntry(item->GetProto()->Spells[k].Id))
            {
                if( spells->RequiredShapeShift )
                {
                    if(apply)
                        AddShapeShiftSpell( spells->Id );
                    else RemoveShapeShiftSpell( spells->Id );
                    continue;
                }

                if(apply == false)
                    RemoveAura( item->GetProto()->Spells[k].Id );
                else
                {
                    SpellCastTargets targets;
                    targets.m_unitTarget = GetGUID();
                    if(Spell *spell = new Spell(this, spells))
                    {
                        spell->castedItemId = item->GetEntry();
                        spell->prepare( &targets, true );
                    }
                }
            }
        }
        else if( item->GetProto()->Spells[k].Trigger == CHANCE_ON_HIT )
        {
            // Todo:PROC
        }
    }
}

void Player::BuildPlayerRepop()
{
    SetUInt32Value(UNIT_FIELD_HEALTH, 1 );

    //8326 --for all races but ne, 20584--ne
    SpellCastTargets tgt;
    tgt.m_unitTarget = GetGUID();

    SpellEntry *inf = dbcSpell.LookupEntry(Wispform ? 20584 : 8326);
    if(Spell* sp = new Spell(this, inf))
        sp->prepare(&tgt, true);

    StopMirrorTimer(0);
    StopMirrorTimer(1);
    StopMirrorTimer(2);

    SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE);

    m_movementInterface.OnRepop();
}

Corpse* Player::RepopRequestedPlayer()
{
    if( myCorpse != NULL )
    {
        GetSession()->SendNotification( NOTIFICATION_MESSAGE_NO_PERMISSION );
        return NULL;
    }

    if( m_CurrentTransporter != NULL )
    {
        m_CurrentTransporter->RemovePlayer( castPtr<Player>(this) );
        m_CurrentTransporter = NULL;
        m_movementInterface.ClearTransportData();

        ResurrectPlayer();
        RepopAtGraveyard( GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId() );
        return NULL;
    }

    sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_PLAYER_FORCED_RESURECT ); //in case somebody resurrected us before this event happened

    // Set death state to corpse, that way players will lose visibility
    SetDeathState( CORPSE );

    // Update visibility, that way people wont see running corpses :P
    UpdateVisibility();

    // If we're in battleground, remove the skinnable flag.. has bad effects heheh
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE );

    bool corpse = (m_bg != NULL) ? m_bg->CreateCorpse( castPtr<Player>(this) ) : true;
    Corpse* ret = corpse ? CreateCorpse() : NULL;
    BuildPlayerRepop();

    if( corpse )
    {
        /* Send Spirit Healer Location */
        WorldPacket data( SMSG_DEATH_RELEASE_LOC, 16 );
        data << m_mapId;
        data.appendvector(m_position, false);
        m_session->SendPacket( &data );

        /* Corpse reclaim delay */
        WorldPacket data2( SMSG_CORPSE_RECLAIM_DELAY, 4 );
        data2 << uint32((ReclaimCount*15) * 1000);
        GetSession()->SendPacket( &data2 );
    }

    if( myCorpse != NULL )
        myCorpse->ResetDeathClock();

    MapInfo * pPMapinfo = NULL;
    pPMapinfo = LimitedMapInfoStorage.LookupEntry( GetMapId() );
    if( pPMapinfo != NULL )
    {
        if( pPMapinfo->type == INSTANCE_NULL || pPMapinfo->type == INSTANCE_PVP )
        {
            RepopAtGraveyard( GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId() );
        }
        else
        {
            RepopAtGraveyard( pPMapinfo->repopx, pPMapinfo->repopy, pPMapinfo->repopz, pPMapinfo->repopmapid );
        }
    }
    else
    {
        RepopAtGraveyard( GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId() );
    }

    return ret;
}

void Player::ResurrectPlayer(Unit* pResurrector /* = NULL */)
{
    if (PreventRes)
        return;

    sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_PLAYER_FORCED_RESURECT); //in case somebody resurected us before this event happened
    if( m_resurrectHealth ) SetUInt32Value(UNIT_FIELD_HEALTH, std::min( m_resurrectHealth, GetUInt32Value(UNIT_FIELD_MAXHEALTH) ) );
    else SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    if( m_resurrectMana ) SetPower(POWER_TYPE_MANA, std::min( m_resurrectMana, GetMaxPower(POWER_TYPE_MANA) ) );
    else SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));

    m_resurrectHealth = m_resurrectMana = 0;

    SpawnCorpseBones();
    RemoveAura(Wispform ? 20584 : 8326);

    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE);
    SetDeathState(ALIVE);
    UpdateVisibility();
    m_movementInterface.OnRessurect();

    if(pResurrector == this)
        return;

    if(IsInWorld() && pResurrector != NULL && pResurrector->IsInWorld())
    {
        //make sure corpse and resurrector are on the same map.
        if( GetMapId() == pResurrector->GetMapId() )
        {
            if(pResurrector->IsPlayer())
            {
                if(m_resurrectLoction.DistanceSq(0.f, 0.f, 0.f) == 0.f)
                    SafeTeleport(pResurrector->GetMapId(), pResurrector->GetInstanceID(), pResurrector->GetPosition());
                else SafeTeleport(GetMapId(), GetInstanceID(), m_resurrectLoction);
            }
            m_resurrectLoction.ChangeCoords(0.0f, 0.0f, 0.0f);
        }
    }
    else
    {
        // update player counts in arenas
        if( m_bg != NULL && m_bg->IsArena() )
            castPtr<Arena>(m_bg)->UpdatePlayerCounts();
    }
}

void Player::KillPlayer()
{
    SetDeathState(JUST_DIED);

    // Battleground stuff
    if(m_bg)
        m_bg->HookOnPlayerDeath(castPtr<Player>(this));

    EventDeath();

    m_session->OutPacket(SMSG_CANCEL_COMBAT);
    m_session->OutPacket(SMSG_CANCEL_AUTO_REPEAT);

    m_movementInterface.OnDeath();

    StopMirrorTimer(0);
    StopMirrorTimer(1);
    StopMirrorTimer(2);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED); //player death animation, also can be used with DYNAMIC_FLAGS <- huh???
    SetUInt32Value( UNIT_DYNAMIC_FLAGS, 0x00 );
    SetPower(POWER_TYPE_RAGE, 0);
    SetPower(POWER_TYPE_RUNIC, 0);
}

Corpse* Player::CreateCorpse()
{
    uint32 _uf, _pb, _pb2, _cfb1, _cfb2;

    objmgr.DelinkPlayerCorpses(castPtr<Player>(this));
    Corpse *pCorpse = objmgr.CreateCorpse();
    pCorpse->SetInstanceID(GetInstanceID());
    pCorpse->Create(castPtr<Player>(this), GetMapId(), GetPositionX(),
        GetPositionY(), GetPositionZ(), GetOrientation());

    _uf = GetUInt32Value(UNIT_FIELD_BYTES_0);
    _pb = GetUInt32Value(PLAYER_BYTES);
    _pb2 = GetUInt32Value(PLAYER_BYTES_2);

    uint8 race     = (uint8)(_uf);
    uint8 skin     = (uint8)(_pb);
    uint8 face     = (uint8)(_pb >> 8);
    uint8 hairstyle  = (uint8)(_pb >> 16);
    uint8 haircolor  = (uint8)(_pb >> 24);
    uint8 facialhair = (uint8)(_pb2);

    _cfb1 = ((0x00) | (race << 8) | (0x00 << 16) | (skin << 24));
    _cfb2 = ((face) | (hairstyle << 8) | (haircolor << 16) | (facialhair << 24));

    pCorpse->SetZoneId( GetZoneId() );
    pCorpse->SetUInt32Value( CORPSE_FIELD_BYTES_1, _cfb1 );
    pCorpse->SetUInt32Value( CORPSE_FIELD_BYTES_2, _cfb2 );
    pCorpse->SetUInt32Value( CORPSE_FIELD_FLAGS, 4 );
    pCorpse->SetUInt32Value( CORPSE_FIELD_DISPLAY_ID, GetUInt32Value(UNIT_FIELD_DISPLAYID) );

    uint32 _cfi = 0, iDisplayID = 0;
    uint16 iIventoryType = 0;
    for (int8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if(Item *pItem = GetInventory()->GetInventoryItem(i))
        {
            iDisplayID = pItem->GetProto()->DisplayInfoID;
            iIventoryType = (uint16)pItem->GetProto()->InventoryType;

            _cfi =  (uint16(iDisplayID)) | (iIventoryType)<< 24;
            pCorpse->SetUInt32Value(CORPSE_FIELD_ITEM + i,_cfi);
        }
    }

    // are we going to bones straight away?
    if(m_insigniaTaken)
    {
        if( m_bg != NULL && m_bg->SupportsPlayerLoot() )
            m_insigniaTaken = false;   // for next time
        pCorpse->SetUInt32Value(CORPSE_FIELD_FLAGS, 5);
        pCorpse->SetUInt64Value(CORPSE_FIELD_OWNER, 0); // remove corpse owner association
        //remove item association
        for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
        {
            if(pCorpse->GetUInt32Value(CORPSE_FIELD_ITEM + i))
                pCorpse->SetUInt32Value(CORPSE_FIELD_ITEM + i, 0);
        }
        pCorpse->SetCorpseState(CORPSE_STATE_BONES);
    }
    else
    {
        //save corpse in db for future use
        if(m_mapMgr && m_mapMgr->GetMapInfo()->type == INSTANCE_NULL)
            pCorpse->SaveToDB();

        myCorpse = pCorpse;

        if( m_bg != NULL && m_bg->SupportsPlayerLoot() )
        {
            if( !m_insigniaTaken )
                pCorpse->SetFlag(CORPSE_FIELD_FLAGS, 60);
        }
    }

    // spawn
    if( m_mapMgr == NULL )
        pCorpse->AddToWorld();
    else
        pCorpse->PushToWorld(m_mapMgr);

    // add deletion event if bone corpse
    if( pCorpse->GetUInt64Value(CORPSE_FIELD_OWNER) == 0 )
        objmgr.CorpseAddEventDespawn(pCorpse);

    return pCorpse;
}

void Player::SpawnCorpseBones()
{
    myCorpse = NULL;
    if(Corpse* pCorpse = objmgr.GetCorpseByOwner(GetLowGUID()))
    {
        if (pCorpse->IsInWorld() && pCorpse->GetCorpseState() == CORPSE_STATE_BODY)
        {
            if(pCorpse->GetInstanceID() != GetInstanceID())
            {
                sEventMgr.AddEvent(pCorpse, &Corpse::SpawnBones, EVENT_CORPSE_SPAWN_BONES, 100, 1,0);
            } else pCorpse->SpawnBones();
        }
    }
}

void Player::DeathDurabilityLoss(double percent)
{
    m_session->OutPacket(SMSG_DURABILITY_DAMAGE_DEATH);
    uint32 pDurability;
    uint32 pMaxDurability;
    int32 pNewDurability;
    Item* pItem;

    for (int8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if((pItem = GetInventory()->GetInventoryItem(i)))
        {
            pMaxDurability = pItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
            pDurability =  pItem->GetUInt32Value(ITEM_FIELD_DURABILITY);
            if(pDurability)
            {
                pNewDurability = (uint32)(pMaxDurability*percent);
                pNewDurability = (pDurability - pNewDurability);
                if(pNewDurability < 0)
                    pNewDurability = 0;

                if(pNewDurability <= 0)
                    ApplyItemMods(pItem, i, false, true);

                pItem->SetUInt32Value(ITEM_FIELD_DURABILITY,(uint32)pNewDurability);
            }
        }
    }
}

void Player::RepopAtGraveyard(float ox, float oy, float oz, uint32 mapid)
{
    bool first = true;
    //float closestX = 0, closestY = 0, closestZ = 0, closestO = 0;
    StorageContainerIterator<GraveyardTeleport> * itr;

    LocationVector src(ox, oy, oz);
    LocationVector dest(0, 0, 0, 0);
    LocationVector temp;
    float closest_dist = 999999.0f;
    float dist;

    if(m_bg && m_bg->HookHandleRepop(castPtr<Player>(this)))
        return;

    itr = GraveyardStorage.MakeIterator();
    while(!itr->AtEnd())
    {
        GraveyardTeleport *pGrave = itr->Get();
        if(pGrave->MapId == mapid && (pGrave->FactionID == GetTeam() || pGrave->FactionID == 3))
        {
            temp.ChangeCoords(pGrave->X, pGrave->Y, pGrave->Z, pGrave->O);
            dist = src.DistanceSq(temp);
            if( first || dist < closest_dist )
            {
                first = false;
                closest_dist = dist;
                dest = temp;
            }
        }

        if(!itr->Inc())
            break;
    }
    itr->Destruct();

    if(dest.x != 0 && dest.y != 0 && dest.z != 0)
        SafeTeleport(mapid, 0, dest);
}

void Player::JoinedChannel(Channel *c)
{
    if( c != NULL )
    {
        m_channels.insert(c->m_channelId);
        if(c->pDBC)
        {
            std::map<uint32, Channel*>::iterator itr = m_channelsbyDBCID.find(c->pDBC->id);
            if(itr != m_channelsbyDBCID.end())
                m_channelsbyDBCID.erase(c->pDBC->id);
            m_channelsbyDBCID.insert(std::make_pair(c->pDBC->id, c));
        }
    }
}

void Player::LeftChannel(Channel *c)
{
    if( c != NULL )
    {
        m_channels.erase(c->m_channelId);
        if(c->pDBC)
            m_channelsbyDBCID.erase(c->pDBC->id);
    }
}

void Player::CleanupChannels()
{
    std::set<uint32>::iterator i;
    Channel * c;
    uint32 cid;
    for(i = m_channels.begin(); i != m_channels.end();)
    {
        cid = *i;
        ++i;

        c = channelmgr.GetChannel(cid);
        if( c != NULL )
            c->Part(castPtr<Player>(this), false);
    }
    m_channelsbyDBCID.clear();
}

//Groupcheck
bool Player::IsGroupMember(Player* plyr)
{
    if(m_playerInfo->m_Group != NULL)
        return m_playerInfo->m_Group->HasMember(plyr->m_playerInfo);
    return false;
}

int32 Player::GetOpenQuestSlot()
{
    for(uint32 i = 0; i < QUEST_LOG_COUNT; i++)
        if (m_questlog[i] == NULL)
            return i;

    return -1;
}

uint32 Player::GetQuestStatusForQuest(uint32 questid, uint8 type, bool skiplevelcheck)
{
    return sQuestMgr.CalcQuestStatus(this, GetQuestLogForEntry(questid)->GetQuest(), type, skiplevelcheck);
}

void Player::AddToCompletedQuests(uint32 quest_id)
{
    m_completedQuests.insert(std::make_pair(quest_id, UNIXTIME));
}

bool Player::HasFinishedQuest(uint32 quest_id)
{
    return m_completedQuests.find(quest_id) != m_completedQuests.end();
}

void Player::AddToCompletedDailyQuests(uint32 quest_id)
{
    if(m_completedDailyQuests.size() >= 25)
        return;

    DailyMutex.Acquire();
    SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1 + uint32(m_completedDailyQuests.size()), quest_id);
    m_completedDailyQuests.insert(std::make_pair(quest_id, UNIXTIME));
    DailyMutex.Release();
}

void Player::ResetDailyQuests()
{
    m_completedDailyQuests.clear();

    for(uint32 i = 0; i < 25; i++)
        SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1 + i, 0);
}

bool Player::HasFinishedDailyQuest(uint32 quest_id)
{
    return m_completedDailyQuests.find(quest_id) != m_completedDailyQuests.end();
}

float Player::CalculateCritFromAgilForClassAndLevel(uint32 _class, uint32 _level)
{
    gtFloat* baseCrit = dbcMeleeCritBase.LookupEntry(_class-1);
    gtFloat* CritPerAgi = dbcMeleeCrit.LookupEntry((_class-1)*100+(_level - 1));
    if(baseCrit == NULL || CritPerAgi == NULL)
        return 0.0f;
    uint32 agility = GetUInt32Value(UNIT_FIELD_AGILITY);
    float base = 100*baseCrit->val, ratio = 100*CritPerAgi->val;
    if(ratio < 0.0f) ratio = 0.00001f;
    return (base + float(agility*ratio));
}

float Player::CalculateDefenseFromAgilForClassAndLevel(uint32 _class, uint32 _level)
{
    gtFloat* CritPerAgi = dbcMeleeCrit.LookupEntry((_class-1)*100+(_level - 1));
    if(CritPerAgi == NULL)
        return 0.0f;
    float class_multiplier = (_class == WARRIOR ? 1.1f : _class == HUNTER ? 1.6f : _class == ROGUE ? 2.0f : _class == DRUID ? 1.7f : 1.0f);
    uint32 agility = GetUInt32Value(UNIT_FIELD_AGILITY)*class_multiplier;
    float base = baseDodge[_class], ratio = 100*CritPerAgi->val;
    return (base + (agility*ratio));
}

void Player::HandleRestedCalculations(bool rest_on)
{
    LocationVector loc = GetPosition();
    if (rest_on)
    {
        if(!m_isResting)
            ApplyPlayerRestState(true);
    }
    else if(GetMapMgr()->CanUseCollision(this))
    {
        if(LastAreaTrigger != NULL && LastAreaTrigger->Type == ATTYPE_INN)
        {
            if(m_isResting)
            {
                if(!sVMapInterface.IsIndoor(GetMapId(), loc.x, loc.y, loc.z + 2.0f))
                    ApplyPlayerRestState(false);
            }
            else if(sVMapInterface.IsIndoor(GetMapId(), loc.x, loc.y, loc.z + 2.0f))
            {
                if(AreaTriggerEntry* ATE = dbcAreaTrigger.LookupEntry(LastAreaTrigger->AreaTriggerID))
                {
                    float delta = 3.2f;
                    if(ATE->radius) // If there is a radius, check our distance with the middle.
                    {
                        if(CalcDistance(ATE->base_x, ATE->base_y, ATE->base_z) < ATE->radius+delta)
                            ApplyPlayerRestState(true);
                    }
                    else
                    {
                        if(IsInBox(ATE->base_x, ATE->base_y, ATE->base_z, ATE->box_length, ATE->box_width, ATE->box_height, ATE->box_yaw, delta))
                            ApplyPlayerRestState(true);
                    }
                }
                else
                {   // Clear our trigger, since it's wrong anyway.
                    LastAreaTrigger = NULL;
                    ApplyPlayerRestState(false);
                }
            }
        }
        else if(m_isResting)
            ApplyPlayerRestState(false);
    }
    else ApplyPlayerRestState(false);
}

uint32 Player::SubtractRestXP(uint32 &amount)
{
    if(GetUInt32Value(UNIT_FIELD_LEVEL) >= GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))      // Save CPU, don't waste time on this if you've reached max_level
        return (amount = 0);

    uint32 restedBonus = (m_restAmount > amount ? amount : m_restAmount);
    m_restAmount -= restedBonus; // Subtract the rested bonus
    UpdateRestState(); // Update clients interface with new values.
    return amount;
}

void Player::AddCalculatedRestXP(uint32 seconds)
{
    // When a player rests in a city or at an inn they will gain rest bonus at a very slow rate.
    // Eight hours of rest will be needed for a player to gain one "bubble" of rest bonus.
    // At any given time, players will be able to accumulate a maximum of 30 "bubbles" worth of rest bonus which
    // translates into approximately 1.5 levels worth of rested play (before your character returns to normal rest state).
    // Thanks to the comforts of a warm bed and a hearty meal, players who rest or log out at an Inn will
    // accumulate rest credit four times faster than players logged off outside of an Inn or City.
    // Players who log out anywhere else in the world will earn rest credit four times slower.
    // http://www.worldofwarcraft.com/info/basics/resting.html


    // Define xp for a full bar ( = 20 bubbles)
    uint32 xp_to_lvl = sWorld.GetXPToNextLevel(getLevel());

    // get RestXP multiplier from config.
    float bubblerate = sWorld.getRate(RATE_RESTXP);

    // One bubble (5% of xp_to_level) for every 8 hours logged out.
    // if multiplier RestXP (from Ronin.config) is f.e 2, you only need 4hrs/bubble.
    uint32 rested_xp = uint32(0.05f * xp_to_lvl * ( seconds / (3600 * ( 8 / bubblerate))));

    // if we are at a resting area rest_XP goes 4 times faster (making it 1 bubble every 2 hrs)
    if (m_isResting) rested_xp *= 4;

    // Add result to accumulated rested XP
    m_restAmount += uint32(rested_xp);

    // and set limit to be max 1.5 * 20 bubbles * multiplier (1.5 * xp_to_level * multiplier)
    if (m_restAmount > xp_to_lvl + (uint32)((float)( xp_to_lvl*1.5f ) * bubblerate ))
        m_restAmount = xp_to_lvl + (uint32)((float)( xp_to_lvl*1.5f ) * bubblerate );

    sLog.Debug("REST","Add %d rest XP to a total of %d, RestState %d", rested_xp, m_restAmount,m_isResting);

    // Update clients interface with new values.
    UpdateRestState();
}

void Player::UpdateRestState()
{
    if(m_restAmount && GetUInt32Value(UNIT_FIELD_LEVEL) < GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
        m_restState = RESTSTATE_RESTED;
    else m_restState = RESTSTATE_NORMAL;

    // Update RestState 100%/200%
    SetByte(PLAYER_BYTES_2, 3, m_restState);

    //update needle (weird, works at 1/2 rate)
    SetUInt32Value(PLAYER_REST_STATE_EXPERIENCE, (m_restAmount ? 1+(m_restAmount >> 1) : 0));
}

void Player::ApplyPlayerRestState(bool apply)
{
    if(apply)
    {
        m_restState = RESTSTATE_RESTED;
        m_isResting = true;
        SetFlag(PLAYER_FLAGS, PLAYER_FLAG_RESTING); //put zzz icon
    }
    else
    {
        m_isResting = false;
        RemoveFlag(PLAYER_FLAGS,PLAYER_FLAG_RESTING);   //remove zzz icon
    }
    UpdateRestState();
}

#define CORPSE_VIEW_DISTANCE 1600 // 40*40

bool Player::CanSee(WorldObject* obj) // * Invisibility & Stealth Detection - Partha *
{
    if (obj == castPtr<Player>(this))
       return true;

    uint32 object_type = obj->GetTypeId();

    // We can't see any objects in another phase
    // unless they're in ALL_PHASES
    if( !PhasedCanInteract(obj) )
        return false;

    /* I'm a GM, I can see EVERYTHING! :D */
    if( bGMTagOn )
        return true;

    if(obj->IsPlayer() && castPtr<Player>(obj)->m_isGmInvisible)
        return false;

    if(getDeathState() == CORPSE) // we are dead and we have released our spirit
    {
        if(object_type == TYPEID_PLAYER)
        {
            Player* pObj = castPtr<Player>(obj);

            if(myCorpse && myCorpse->GetDistanceSq(obj) > CORPSE_VIEW_DISTANCE)
                if(pObj->IsPlayer() && pObj->getDeathState() == CORPSE)
                    return !pObj->m_isGmInvisible; // we can see all players within range of our corpse except invisible GMs

            if(myCorpse && myCorpse->GetDistanceSq(obj) <= CORPSE_VIEW_DISTANCE)
                return !pObj->m_isGmInvisible; // we can see all players within range of our corpse except invisible GMs

            if(m_deathVision) // if we have arena death-vision we can see all players except invisible GMs
                return !pObj->m_isGmInvisible;

            return (pObj->getDeathState() == CORPSE); // we can only see players that are spirits
        }

        if(myCorpse)
        {
            if(myCorpse == obj)
                return true;

            if(myCorpse->GetDistanceSq(obj) <= CORPSE_VIEW_DISTANCE)
                return true; // we can see everything within range of our corpse
        }

        if(m_deathVision) // if we have arena death-vision we can see everything
        {
            /*if(obj->IsPlayer() && castPtr<Player>(obj)->InStealth())
                return false;*/

            return true;
        }

        if(object_type == TYPEID_UNIT)
        {
            Unit* uObj = castPtr<Unit>(obj);

            return uObj->IsSpiritHealer(); // we can't see any NPCs except spirit-healers
        }

        return false;
    }
    //------------------------------------------------------------------

    // mage invisibility - ugh.
    if( IsPlayer() && m_mageInvisibility )
    {
        if( object_type == TYPEID_PLAYER )
            return castPtr<Player>(obj)->m_mageInvisibility;

        if( object_type == TYPEID_UNIT )
            return false;

        return true;
    }

    switch(object_type) // we are alive or we haven't released our spirit yet
    {
        case TYPEID_PLAYER:
            {
                Player* pObj = castPtr<Player>(obj);

                if( pObj->m_mageInvisibility )
                    return false;

                if(pObj->m_invisible) // Invisibility - Detection of Players
                {
                    if(pObj->getDeathState() == CORPSE)
                        return bGMTagOn; // only GM can see players that are spirits

                    if(GetGroup() && pObj->GetGroup() == GetGroup() // can see invisible group members except when dueling them
                            && DuelingWith != pObj)
                        return true;

                    if(GetGUID() == pObj->stalkedby) // Hunter's Mark / MindVision is visible to the caster
                        return true;

                    if(m_invisDetect[INVIS_FLAG_NORMAL] < 1 // can't see invisible without proper detection
                            || pObj->m_isGmInvisible) // can't see invisible GM
                        return bGMTagOn; // GM can see invisible players
                }

                return true;
            }
        //------------------------------------------------------------------

        case TYPEID_UNIT:
            {
                Unit* uObj = castPtr<Unit>(obj);

                if(uObj->IsSpiritHealer()) // can't see spirit-healers when alive
                    return false;

                if(uObj->m_invisible // Invisibility - Detection of Units
                        && m_invisDetect[uObj->m_invisFlag] < 1) // can't see invisible without proper detection
                    return bGMTagOn; // GM can see invisible units

                return true;
            }
        //------------------------------------------------------------------

        case TYPEID_GAMEOBJECT:
            {
                GameObject* gObj = castPtr<GameObject>(obj);

                if(gObj->invisible) // Invisibility - Detection of GameObjects
                {
                    uint64 owner = gObj->GetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY);

                    if(GetGUID() == owner) // the owner of an object can always see it
                        return true;

                    if(GetGroup())
                    {
                        Player * gplr = NULL;
                        gplr = GetMapMgr()->GetPlayer((uint32)owner);
                        if(gplr != NULL && GetGroup()->HasMember(gplr))
                            return true;
                    }

                    if(m_invisDetect[gObj->invisibilityFlag] < 1) // can't see invisible without proper detection
                        return bGMTagOn; // GM can see invisible objects
                }

                return true;
            }
        //------------------------------------------------------------------

        default:
            return true;
    }
}

void Player::AddInRangeObject(WorldObject* pObj)
{
    //Send taxi move if we're on a taxi
    if (m_CurrentTaxiPath && (pObj->IsPlayer()))
    {
        uint32 ntime = getMSTime();

        if (ntime > m_taxi_ride_time)
            m_CurrentTaxiPath->SendMoveForTime( castPtr<Player>(this), castPtr<Player>( pObj ), ntime - m_taxi_ride_time);
        /*else
            m_CurrentTaxiPath->SendMoveForTime( castPtr<Player>(this), castPtr<Player>( pObj ), m_taxi_ride_time - ntime);*/
    }

    if( pObj->IsCreature() && pObj->GetFactionTemplate() && pObj->GetFactionTemplate()->FactionFlags & 0x1000 )
        m_hasInRangeGuards++;

    Unit::AddInRangeObject(pObj);

    //if the object is a unit send a move packet if they have a destination
    if(pObj->GetTypeId() == TYPEID_UNIT)
    {
        //add an event to send move update have to send guid as pointer was causing a crash :(
        castPtr<Creature>( pObj )->GetAIInterface()->SendCurrentMove(this);
    }

    //unit based objects, send aura data
    if (pObj->IsUnit())
    {
        Unit* pUnit=castPtr<Unit>(pObj);
        Aura* aur = NULL;

        if (GetSession() != NULL)
        {
            WorldPacket* data = new WorldPacket(SMSG_AURA_UPDATE_ALL, 28 * MAX_AURAS);
            *data << pUnit->GetGUID().asPacked();
            if(pUnit->m_AuraInterface.BuildAuraUpdateAllPacket(data))
                SendPacket(data);
            else delete data;
        }
    }
}

void Player::OnRemoveInRangeObject(WorldObject* pObj)
{
    if( pObj->IsCreature() && pObj->GetFactionTemplate() && pObj->GetFactionTemplate()->FactionFlags & 0x1000 )
        m_hasInRangeGuards--;

    if(m_tempSummon == pObj)
    {
        m_tempSummon->RemoveFromWorld(false, true);
        if(m_tempSummon)
            m_tempSummon->SafeDelete();

        m_tempSummon = NULL;
        SetUInt64Value(UNIT_FIELD_SUMMON, 0);
    }

    if(m_curSelection == pObj->GetGUID())
        m_curSelection.Clean();

    pObj->DestroyForPlayer(this);
    m_visibleObjects.erase(pObj);
    Unit::OnRemoveInRangeObject(pObj);

    if( pObj == m_CurrentCharm)
    {
        Unit* p = m_CurrentCharm;

        if(pObj == GetVehicle())
            GetVehicle()->RemovePassenger(castPtr<Player>(this));
        else
            UnPossess();
        if(m_currentSpell)
            m_currentSpell->cancel();      // cancel the spell
        m_CurrentCharm=NULL;

        if( p->m_temp_summon&&p->GetTypeId() == TYPEID_UNIT )
            castPtr<Creature>( p )->SafeDelete();
    }

    if(pObj == m_Summon)
    {
        if(m_Summon->IsSummonedPet())
            m_Summon->Dismiss(true);
        else
            m_Summon->Remove(true, true, false);
        if(m_Summon)
        {
            m_Summon->ClearPetOwner();
            m_Summon = NULL;
        }
    }

    if( pObj == DuelingWith )
        sEventMgr.AddEvent(castPtr<Player>(this), &Player::EndDuel, (uint8)DUEL_WINNER_RETREAT, EVENT_PLAYER_DUEL_COUNTDOWN, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Player::ClearInRangeSet()
{
    WorldObject* pObj = NULL;
    while(!m_inRangeObjects.empty())
    {
        pObj = m_inRangeObjects.begin()->second;
        RemoveInRangeObject(pObj);
    }
}

void Player::SetDrunk(uint16 value, uint32 itemId)
{
    uint32 oldDrunkenState = GetDrunkenstateByValue(m_drunk);
    m_invisDetect[INVIS_FLAG_DRUNK] = int32(value - m_drunk) / 256;
    m_drunk = value;
    SetUInt32Value(PLAYER_BYTES_3,(GetUInt32Value(PLAYER_BYTES_3) & 0xFFFF0001) | (m_drunk & 0xFFFE));
    uint32 newDrunkenState = GetDrunkenstateByValue(m_drunk);
    /*if(newDrunkenState == DRUNKEN_VOMIT)
    {
        sEventMgr.AddEvent(this, &Player::EventDrunkenVomit, EVENT_DRUNKEN_VOMIT, 5000, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return;
    }*/
    UpdateVisibility();
    if (newDrunkenState == oldDrunkenState)
        return;
    WorldPacket data(SMSG_CROSSED_INEBRIATION_THRESHOLD, 16);
    data << GetGUID();
    data << newDrunkenState;
    data << itemId;
    GetSession()->SendPacket(&data);
}

void Player::EventHandleSobering()
{
    m_drunkTimer = 0;
    uint32 drunk = (m_drunk <= 256) ? 0 : (m_drunk - 256);
    SetDrunk(drunk);
}

void Player::LoadTaxiMask(const char* data)
{
    std::vector<std::string> tokens = RONIN_UTIL::StrSplit(data, " ");

    int index;
    std::vector<std::string>::iterator iter;

    for (iter = tokens.begin(), index = 0;
        (index < MAX_TAXI) && (iter != tokens.end()); iter++, ++index)
    {
        m_taximask[index] = atol((*iter).c_str());
    }
}

bool Player::HasQuestForItem(uint32 itemid)
{
    Quest *qst;
    for(uint32 i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if( m_questlog[i] != NULL )
        {
            qst = m_questlog[i]->GetQuest();

            // No item_quest association found, check the quest requirements
            if( !qst->count_required_item )
                continue;

            for( uint8 j = 0; j < 6; ++j )
                if( qst->required_item[j] == itemid && ( GetInventory()->GetItemCount( itemid ) < qst->required_itemcount[j] ) )
                    return true;
        }
    }
    return false;
}

bool Player::CanNeedItem(ItemPrototype* proto)
{
    if(proto->Class == ITEM_CLASS_WEAPON)
        if(!(GetWeaponProficiency() & (uint32(1) << proto->SubClass)))
            return false;
    else if(proto->Class == ITEM_CLASS_ARMOR)
        if(!(GetArmorProficiency() & (uint32(1) << proto->SubClass)))
            return false;
    if(proto->AllowableClass)
        if(!(proto->AllowableClass & getClassMask()))
            return false;
    if(proto->AllowableRace)
        if(!(proto->AllowableRace & getRaceMask()))
            return false;
    // Crow: Should be all we need for now.
    return true;
}

/*Loot type MUST be
1-corpse, go
2-skinning/herbalism/minning
3-Fishing
*/
void Player::SendLoot(WoWGuid guid, uint32 mapid, uint8 loot_type)
{
    Group * m_Group = m_playerInfo->m_Group;
    if(!IsInWorld())
        return;

    Object* lootEnt;
    // handle items
    if(GUID_HIPART(guid) == HIGHGUID_TYPE_ITEM)
        lootEnt = m_inventory.GetInventoryItem(guid);
    else lootEnt = m_mapMgr->_GetObject(guid);
    if( lootEnt == NULL )
        return;

    int8 loot_method = -1;

    //lootsteal fix
    if( lootEnt->GetTypeId() == TYPEID_UNIT )
    {
        Creature* LootOwner = castPtr<Creature>( lootEnt );
        if( uint32 GroupId = LootOwner->m_taggingGroup )
            if(m_Group == NULL || GroupId != m_Group->GetID())
                return;
        loot_method = LootOwner->m_lootMethod;
    }

    if( loot_method < 0 )
    {
        // not set
        if( m_Group != NULL )
            loot_method = m_Group->GetMethod();
        else loot_method = PARTY_LOOT_FFA;
    }

    // add to looter set
    lootEnt->GetLoot()->looters.insert(GetLowGUID());
    m_lootGuid = guid;

    WorldPacket data(SMSG_LOOT_RESPONSE, 32), data2(32);
    data << uint64(guid);
    data << uint8(loot_type);//loot_type;
    data << uint32(lootEnt->GetLoot()->gold);
    data << uint8(0);//loot size reserve
    data << uint8(0);//unk

    uint32 count = 0;
    std::vector<__LootItem>::iterator iter = lootEnt->GetLoot()->items.begin();
    for(uint8 x = 0; iter != lootEnt->GetLoot()->items.end(); iter++, x++)
    {
        if(x == 0xFF)
            break;
        ItemPrototype* itemProto = iter->proto;
        if (!itemProto)
            continue;
        if((itemProto->Flags & DBC_ITEMFLAG_PARTY_LOOT) == 0)
        {
            if(iter->has_looted.size())
                continue;
        } else if (iter->has_looted.find(GetGUID()) != iter->has_looted.end())
            continue;

        //quest items check. type 4/5
        //quest items that dont start quests.
        if((itemProto->Bonding == ITEM_BIND_QUEST) && !(itemProto->QuestId) && !HasQuestForItem(itemProto->ItemId))
            continue;

        if((itemProto->Bonding == ITEM_BIND_QUEST2) && !(itemProto->QuestId) && !HasQuestForItem(itemProto->ItemId))
            continue;

        //quest items that start quests need special check to avoid drops all the time.
        if((itemProto->Bonding == ITEM_BIND_QUEST) && (itemProto->QuestId) && GetQuestLogForEntry(itemProto->QuestId))
            continue;

        if((itemProto->Bonding == ITEM_BIND_QUEST2) && (itemProto->QuestId) && GetQuestLogForEntry(itemProto->QuestId))
            continue;

        if((itemProto->Bonding == ITEM_BIND_QUEST) && (itemProto->QuestId) && HasFinishedQuest(itemProto->QuestId))
            continue;

        if((itemProto->Bonding == ITEM_BIND_QUEST2) && (itemProto->QuestId) && HasFinishedQuest(itemProto->QuestId))
            continue;

        //check for starting item quests that need questlines.
        if(itemProto->QuestId && itemProto->Bonding != ITEM_BIND_QUEST && itemProto->Bonding != ITEM_BIND_QUEST2)
        {
            bool HasRequiredQuests = true;
            Quest * pQuest = sQuestMgr.GetQuestPointer(itemProto->QuestId);
            if(pQuest == NULL)
                continue;

            //check if its a questline.
            uint32 RequiredQuest = 0;
            for(uint32 i = 0; i < pQuest->count_requiredquests; i++)
            {
                if(!HasRequiredQuests)
                    continue;

                if((RequiredQuest = pQuest->required_quests[i]))
                    if(HasFinishedQuest(RequiredQuest) || !GetQuestLogForEntry(RequiredQuest))
                        HasRequiredQuests = false;
            }

            if(!HasRequiredQuests)
                continue;
        }

        uint8 slottype = 0;
        if(lootEnt->IsObject() && m_Group != NULL && loot_type < 2)
        {
            switch(loot_method)
            {
            case PARTY_LOOT_MASTER:
                slottype = 2;
                break;
            case PARTY_LOOT_GROUP:
            case PARTY_LOOT_RR:
            case PARTY_LOOT_NBG:
                slottype = 1;
                break;
            default:
                slottype = 0;
                break;
            }

            // only quality items are distributed
            if(itemProto->Quality < m_Group->GetThreshold())
                slottype = 0;

            /* if all people passed anyone can loot it? :P */
            if(iter->all_passed || itemProto->Flags & DBC_ITEMFLAG_PARTY_LOOT)
                slottype = 0; // All players passed on the loot
        }

        data << uint8(x);
        data << uint32(itemProto->ItemId);
        data << uint32(iter->StackSize);//nr of items of this type
        data << uint32(itemProto->DisplayInfoID);
        data << uint32(iter->randProp);
        data << uint32(iter->randSeed);
        data << slottype;   // "still being rolled for" flag
        if(slottype == 1)
        {
            WorldObject *lootObj = castPtr<WorldObject>(lootEnt);
            if(iter->roll == NULL && !iter->all_passed)
            {
                iter->roll = new LootRoll();
                iter->roll->Init(60000, (m_Group != NULL ? m_Group->MemberCount() : 1),  guid, x, itemProto->ItemId, iter->randProp, iter->randSeed, GetMapMgr());

                data2.Initialize(SMSG_LOOT_START_ROLL);
                data2 << uint64(guid);
                data2 << uint32(mapid);
                data2 << uint32(x);
                data2 << uint32(itemProto->ItemId);
                data2 << uint32(iter->randProp);
                data2 << uint32(iter->randSeed);
                data2 << uint32(iter->StackSize);
                data2 << uint32(60000); // countdown

                if(Group* pGroup = m_playerInfo->m_Group)
                {
                    size_t maskpos = data2.wpos();
                    data2 << uint8(0);

                    pGroup->Lock();
                    Player* plr = NULL;
                    for(uint32 i = 0; i < pGroup->GetSubGroupCount(); i++)
                    {
                        for(GroupMembersSet::iterator itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); itr++)
                        {
                            plr = (*itr)->m_loggedInPlayer;
                            //if(plr && plr->GetInventory()->CanReceiveItem(itemProto, iter->StackSize, NULL) == 0)
                            {   // If we have pass on, or if we're not in range, we have to pass.
                                if( plr->m_passOnLoot || ( !lootObj->IsInRangeMap(plr) ) )
                                    iter->roll->PlayerRolled( (*itr), PASS );       // passed
                                else
                                {
                                    uint8 canusemask = ROLLMASK_PASS|ROLLMASK_GREED;
                                    if(plr->CanNeedItem(itemProto))
                                        canusemask |= ROLLMASK_NEED;
                                    if(pGroup->HasAcceptableDisenchanters(itemProto->DisenchantReqSkill))
                                        canusemask |= ROLLMASK_DISENCHANT;

                                    data2.put<uint8>(maskpos, canusemask);
                                    plr->GetSession()->SendPacket(&data2);
                                }
                            }// else iter->roll->PlayerRolled( (*itr), PASS );       // passed
                        }
                    }
                    pGroup->Unlock();
                }
                else
                {
                    uint8 canusemask = ROLLMASK_PASS|ROLLMASK_GREED;
                    if(CanNeedItem(itemProto))
                        canusemask |= ROLLMASK_NEED;
                    if(itemProto->DisenchantReqSkill > -1)
                        if(_HasSkillLine(SKILL_ENCHANTING) && (_GetSkillLineCurrent(SKILL_ENCHANTING, true) > uint32(itemProto->DisenchantReqSkill)))
                            canusemask |= ROLLMASK_DISENCHANT;

                    data2 << canusemask;
                    GetSession()->SendPacket(&data2);
                }
            }
        }
        count++;
    }
    data.put<uint8>(13, count);

    GetSession ()->SendPacket(&data);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);
}

void Player::SendXPToggleConfirm()
{
    if(m_XPoff)
        m_XPoff = false;
    else
        m_XPoff = true;
}

void Player::EventAllowTiggerPort(bool enable)
{
    m_AllowAreaTriggerPort = enable;
}

uint32 Player::CalcTalentResetCost(uint32 resetnum)
{
    if(resetnum == 0 )
        return  10000;
    if(resetnum >= 10)
        return  500000;
    return resetnum*50000;
}

void Player::SendTalentResetConfirm()
{
    WorldPacket data(MSG_TALENT_WIPE_CONFIRM, 12);
    data << GetGUID();
    data << CalcTalentResetCost(m_talentInterface.GetTalentResets());
    GetSession()->SendPacket(&data);
}

void Player::SendPetUntrainConfirm()
{
    Pet* pPet = GetSummon();
    if( pPet == NULL )
        return;


}

int32 Player::CanShootRangedWeapon( uint32 spellid, Unit* target, bool autoshot )
{
    uint8 fail = 0;

    SpellEntry* spellinfo = NULL;
    if( autoshot )
        spellinfo = dbcSpell.LookupEntry( 75 );
    else
        spellinfo = dbcSpell.LookupEntry( spellid );

    if( spellinfo == NULL )
        return -1;

    // Player has clicked off target. Fail spell.
    if( m_curSelection != m_AutoShotTarget )
        fail = SPELL_FAILED_INTERRUPTED;

    if( target->isDead() )
        fail = SPELL_FAILED_TARGETS_DEAD;

    if( GetCurrentSpell() )
        return -1;

    // Supalosa - The hunter ability Auto Shot is using Shoot range, which is 5 yards shorter.
    // So we'll use 114, which is the correct 35 yard range used by the other Hunter abilities (arcane shot, concussive shot...)
    uint32 rIndex = spellinfo->rangeIndex;
    SpellRangeEntry* range = dbcSpellRange.LookupEntry( rIndex );
    float minrange = GetDBCMinRange( range );
    float dist = GetDistance2dSq( target ) - target->GetSize() - GetSize();
    float maxr = GetDBCMaxRange( range );

    if( spellinfo->SpellGroupType )
    {
        SM_FFValue(SMT_RANGE, &maxr, spellinfo->SpellGroupType );
        SM_PFValue(SMT_RANGE, &maxr, spellinfo->SpellGroupType );
    }

    maxr += 4.0f; // Matches client range
    maxr *= maxr; // square me!
    minrange *= minrange;

    if( spellid != SPELL_RANGED_WAND )//no min limit for wands
        if( minrange > dist )
            fail = SPELL_FAILED_TOO_CLOSE;

    if( dist > maxr )
    {
        //  sLog.outString( "Auto shot failed: out of range (Maxr: %f, Dist: %f)" , maxr , dist );
        fail = SPELL_FAILED_OUT_OF_RANGE;
    }

    if (GetMapMgr() && GetMapMgr()->CanUseCollision(this))
    {
        if( !IsInLineOfSight(target) )
        {
            fail = SPELL_FAILED_LINE_OF_SIGHT;
        }
    }

    if( fail > 0 )
    {
        WorldPacket data(SMSG_CAST_FAILED, 20);
        data << uint32(spellinfo->Id) << uint8(fail);
        m_session->SendPacket(&data);

        if( fail != SPELL_FAILED_OUT_OF_RANGE )
        {
            uint32 spellid2 = spellinfo->Id;
            m_session->OutPacket( SMSG_CANCEL_AUTO_REPEAT, 4, &spellid2 );
        }
        return fail;
    }

    return 0;
}

void Player::EventRepeatSpell()
{
    if( !m_curSelection || !IsInWorld() )
        return;

    if(m_AuraInterface.GetAuraStatus() & AURA_STATUS_SPELL_IMPARING_MASK)
        return;

    Unit* target = GetMapMgr()->GetUnit( m_curSelection );
    if( target == NULL || !sFactionSystem.isAttackable(this, target))
    {
        m_AutoShotAttackTimer = 0; //avoid flooding client with error messages
        m_onAutoShot = false;
        return;
    }

    m_AutoShotDuration = GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME);
    if( m_movementInterface.isMoving() )
    {
        m_AutoShotAttackTimer = 400; // shoot when we can
        return;
    }

    if(int32 f = CanShootRangedWeapon( m_AutoShotSpell->Id, target, true ))
    {
        if( f != SPELL_FAILED_OUT_OF_RANGE )
        {
            m_AutoShotAttackTimer = 0;
            m_onAutoShot = false;
        } else m_AutoShotAttackTimer = m_AutoShotDuration;//avoid flooding client with error mesages
        return;
    }
    else
    {
        m_AutoShotAttackTimer = m_AutoShotDuration;

        SpellCastTargets tgt(m_curSelection);
        tgt.m_targetMask = TARGET_FLAG_UNIT;
        if(Spell* sp = new Spell( castPtr<Player>(this), m_AutoShotSpell ))
            sp->prepare( &tgt, true );
    }
}

void Player::removeSpellByNameHash(uint32 hash)
{
    SpellSet::iterator it,iter;

    for(iter= mSpells.begin();iter != mSpells.end();)
    {
        it = iter++;
        uint32 SpellID = *it;
        SpellEntry *e = dbcSpell.LookupEntry(SpellID);
        if(e->NameHash == hash)
        {
            if(info->spell_list.find(e->Id) != info->spell_list.end())
                continue;

            RemoveAura(SpellID,GetGUID());
            m_session->OutPacket(SMSG_REMOVED_SPELL, 4, &SpellID);
            mSpells.erase(it);
        }
    }
}

bool Player::removeSpell(uint32 SpellID)
{
    SpellSet::iterator iter = mSpells.find(SpellID);
    if(iter != mSpells.end())
    {
        mSpells.erase(iter);
        RemoveAura(SpellID,GetGUID());
    }
    else
        return false;

    if(!IsInWorld())
        return true;

    m_session->OutPacket(SMSG_REMOVED_SPELL, 4, &SpellID);
    return true;
}

void Player::RemoveQuestsFromLine(uint32 skill_line)
{
    for(int i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if (m_questlog[i])
        {
            Quest* qst = m_questlog[i]->GetQuest();
            if(qst == NULL)
                continue;

            if(qst->required_tradeskill == skill_line)
            {
                m_questlog[i]->Finish();

                delete m_questlog[i];
                m_questlog[i] = NULL;
            }
        }
    }

    std::map<uint32, time_t>::iterator itr;
    for (itr = m_completedQuests.begin(); itr != m_completedQuests.end();)
    {
        Quest * qst = sQuestMgr.GetQuestPointer(itr->first);
        if(qst->required_tradeskill == skill_line)
            itr = m_completedQuests.erase(itr);
        else itr++;
    }

    UpdateNearbyGameObjects();
}

void Player::SendInitialLogonPackets()
{
    // Initial Packets... they seem to be re-sent on port.
    WorldPacket data(SMSG_BINDPOINTUPDATE, 32);
    data << m_bind_pos_x;
    data << m_bind_pos_y;
    data << m_bind_pos_z;
    data << m_bind_mapid;
    data << m_bind_zoneid;
    GetSession()->SendPacket( &data );

    // Proficiencies
    data.Initialize(SMSG_SET_PROFICIENCY, 5);
    data << uint8(0x02) << weapon_proficiency;
    GetSession()->SendPacket(&data);
    data.Initialize(SMSG_SET_PROFICIENCY, 5);
    data << uint8(0x04) << armor_proficiency;
    GetSession()->SendPacket(&data);

    //Initial Spells
    smsg_InitialSpells();

    data.Initialize(SMSG_SEND_UNLEARN_SPELLS, 4);
    data << uint32(0); // count, for (count) uint32;
    GetSession()->SendPacket(&data);

    if(sWorld.m_useAccountData)
    {   //Lock Initial Actions
        data.Initialize(SMSG_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_COUNT*4+1);
        m_talentInterface.BuildPlayerActionInfo(&data);
        data.put<uint8>(0, 0);
        SendPacket(&data);
    }

    //Factions
    smsg_InitialFactions();

    // Login speed
    data.Initialize(SMSG_LOGIN_SETTIMESPEED);
    data << uint32(RONIN_UTIL::secsToTimeBitFields(UNIXTIME));
    data << float(0.01666667f) << uint32(0);
    GetSession()->SendPacket( &data );

    m_currency.SendInitialCurrency();

    data.Initialize(SMSG_MOVE_SET_ACTIVE_MOVER);
    data.WriteBitString(4, m_objGuid[5], m_objGuid[7], m_objGuid[3], m_objGuid[6]);
    data.WriteBitString(4, m_objGuid[0], m_objGuid[4], m_objGuid[1], m_objGuid[2]);
    data.WriteByteSeq(m_objGuid[6]);
    data.WriteByteSeq(m_objGuid[2]);
    data.WriteByteSeq(m_objGuid[3]);
    data.WriteByteSeq(m_objGuid[0]);
    data.WriteByteSeq(m_objGuid[5]);
    data.WriteByteSeq(m_objGuid[7]);
    data.WriteByteSeq(m_objGuid[1]);
    data.WriteByteSeq(m_objGuid[4]);
    GetSession()->SendPacket( &data );

    sLog.Debug("WORLD","Sent initial logon packets for %s.", GetName());
}

void Player::Reset_Spells()
{
    Mutex lock;
    lock.Acquire();

    PlayerCreateInfo *info = objmgr.GetPlayerCreateInfo(getRace(), getClass());
    ASSERT(info);
    SpellSet spelllist;
    SpellSet::iterator itr;
    bool profession = false;

    for(itr = mSpells.begin(); itr != mSpells.end(); itr++)
    {
        SpellEntry *sp = dbcSpell.LookupEntry((*itr));
        if(sp == NULL)
            continue;

        for( uint32 lp = 0; lp < 3; lp++ )
        {
            if( sp->Effect[lp] == SPELL_EFFECT_SKILL )
                profession = true;
        }

        if( !profession )
            spelllist.insert((*itr));

        profession = false;
    }

    for(itr = spelllist.begin(); itr != spelllist.end(); itr++)
        removeSpell((*itr));

    for(itr = info->spell_list.begin(); itr != info->spell_list.end(); itr++)
    {
        if(*itr)
            addSpell(*itr);
    }

    lock.Release();
}

void Player::Reset_ToLevel1()
{
    m_AuraInterface.RemoveAllAuras();

    setLevel(1);
    UpdateFieldValues();
}

void Player::UpdateNearbyGameObjects()
{
    GameObject* Gobj = NULL;
    for (WorldObject::InRangeSet::iterator itr = GetInRangeGameObjectSetBegin(); itr != GetInRangeGameObjectSetEnd(); itr++)
    {
        Gobj = GetInRangeObject<GameObject>(*itr);
        ByteBuffer buff(500);
        Gobj->SetUpdateField(OBJECT_FIELD_GUID);
        Gobj->SetUpdateField(OBJECT_FIELD_GUID+1);
        Gobj->BuildValuesUpdateBlockForPlayer(&buff, UF_FLAGMASK_PUBLIC);
        PushUpdateBlock(&buff, 1);
    }
}

void Player::UpdateNearbyQuestGivers()
{
    for (WorldObject::InRangeMap::iterator itr = GetInRangeMapBegin(); itr != GetInRangeMapEnd(); itr++)
    {
        if(itr->second->IsGameObject())
        {
            if(castPtr<GameObject>(itr->second)->isQuestGiver())
            {
                uint32 status = sQuestMgr.CalcStatus(itr->second, this);
                if(status != QMGR_QUEST_NOT_AVAILABLE)
                {
                    WorldPacket data(SMSG_QUESTGIVER_STATUS, 12);
                    data << itr->first << status;
                    SendPacket( &data );
                }
            }
        }
        else if(itr->second->IsCreature())
        {
            if(castPtr<Creature>(itr->second)->isQuestGiver())
            {
                uint32 status = sQuestMgr.CalcStatus(itr->second, this);
                if(status != QMGR_QUEST_NOT_AVAILABLE)
                {
                    WorldPacket data(SMSG_QUESTGIVER_STATUS, 12);
                    data << itr->first << status;
                    SendPacket( &data );
                }
            }
        }
    }
}

void Player::EventTaxiInterpolate()
{
    if(!m_CurrentTaxiPath || m_mapMgr==NULL) return;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    uint32 ntime = getMSTime();
    if (ntime > m_taxi_ride_time)
        m_CurrentTaxiPath->SetPosForTime(x, y, z, ntime - m_taxi_ride_time, &lastNode, m_mapId);

    if(x < _minX || x > _maxX || y < _minY || y > _maxX)
        return;

    SetPosition(x,y,z,0);
}

void Player::TaxiStart(TaxiPath *path, uint32 modelid, uint32 start_node)
{
    int32 mapchangeid = -1;
    float mapchangex = 0.0f;
    float mapchangey = 0.0f;
    float mapchangez = 0.0f;
    float orientation = 0;
    uint32 cn = m_taxiMapChangeNode;

    m_taxiMapChangeNode = 0;

    if( IsMounted() )
        Dismount();

    //also remove morph spells
    if(GetUInt32Value(UNIT_FIELD_DISPLAYID) != GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID))
    {
        m_AuraInterface.RemoveAllAurasOfType(SPELL_AURA_TRANSFORM);
        m_AuraInterface.RemoveAllAurasOfType(SPELL_AURA_MOD_SHAPESHIFT);
    }

    SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID, modelid );
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    SetTaxiPath(path);
    SetTaxiPos();
    SetTaxiState(true);
    if(GetSession())
    {
        GetSession()->m_isFalling = false;
        GetSession()->m_isJumping = false;
        GetSession()->m_isKnockedback = false;
    }

    m_taxi_ride_time = getMSTime();

    //uint32 traveltime = uint32(path->getLength() * TAXI_TRAVEL_SPEED); // 36.7407
    float traveldist = 0;

    // temporary workaround for taximodes with changing map
    if (   path->GetID() == 766 || path->GetID() == 767 || path->GetID() == 771 || path->GetID() == 772
        || path->GetID() == 775 || path->GetID() == 776 || path->GetID() == 796 || path->GetID() == 797
        || path->GetID() == 807)
    {
        JumpToEndTaxiNode(path);
        return;
    }

    float lastx = 0, lasty = 0, lastz = 0;
    TaxiPathNode *firstNode = path->GetPathNode(start_node);
    uint32 add_time = 0;
    if(start_node)
    {
        TaxiPathNode *pn = path->GetPathNode(0);
        float dist = 0;
        lastx = pn->x;
        lasty = pn->y;
        lastz = pn->z;
        for(uint32 i = 1; i <= start_node; i++)
        {
            pn = path->GetPathNode(i);
            if(!pn)
            {
                JumpToEndTaxiNode(path);
                return;
            }

            dist += CalcDistance(lastx, lasty, lastz, pn->x, pn->y, pn->z);
            lastx = pn->x;
            lasty = pn->y;
            lastz = pn->z;
        }
        add_time = uint32( dist * TAXI_TRAVEL_SPEED );
        lastx = lasty = lastz = 0;
    }
    size_t endn = path->GetNodeCount();
    if(m_taxiPaths.size())
        endn-= 2;

    for(uint32 i = start_node; i < endn; i++)
    {
        TaxiPathNode *pn = path->GetPathNode(i);
        if(!pn)
        {
            JumpToEndTaxiNode(path);
            return;
        }

        if( pn->mapid != m_mapId )
        {
            endn = (i - 1);
            m_taxiMapChangeNode = i;

            mapchangeid = (int32)pn->mapid;
            mapchangex = pn->x;
            mapchangey = pn->y;
            mapchangez = pn->z;
            break;
        }

        if(!lastx || !lasty || !lastz)
        {
            lastx = pn->x;
            lasty = pn->y;
            lastz = pn->z;
        }
        else
        {
            float dist = CalcDistance(lastx,lasty,lastz,
                pn->x,pn->y,pn->z);
            traveldist += dist;
            lastx = pn->x;
            lasty = pn->y;
            lastz = pn->z;
        }
    }

    uint32 traveltime = uint32(traveldist * TAXI_TRAVEL_SPEED);

    if( start_node > endn || (endn - start_node) > 200 )
        return;

    WorldPacket data(SMSG_MONSTER_MOVE, 38 + ( (endn - start_node) * 12 ) );
    data << GetGUID();
    data << uint8(0);
    data << firstNode->x << firstNode->y << firstNode->z;
    data << m_taxi_ride_time;
    data << uint8( 0 );
    data << uint32( MONSTER_MOVE_FLAG_FLY );
    data << uint32( traveltime );

    if(!cn)
        m_taxi_ride_time -= add_time;

    data << uint32( endn - start_node );
//  uint32 timer = 0, nodecount = 0;
//  TaxiPathNode *lastnode = NULL;

    for(uint32 i = start_node; i < endn; i++)
    {
        TaxiPathNode *pn = path->GetPathNode(i);
        if(!pn)
        {
            JumpToEndTaxiNode(path);
            return;
        }

        data << pn->x << pn->y << pn->z;
    }

    SendMessageToSet(&data, true);

    sEventMgr.AddEvent(castPtr<Player>(this), &Player::EventTaxiInterpolate, EVENT_PLAYER_TAXI_INTERPOLATE, 900, 0, 0);
    if( mapchangeid < 0 )
    {
        TaxiPathNode *pn = path->GetPathNode((uint32)path->GetNodeCount() - 1);
        sEventMgr.AddEvent(this, &Player::EventDismount, path->GetPrice(), pn->x, pn->y, pn->z, EVENT_PLAYER_TAXI_DISMOUNT, traveltime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    } else sEventMgr.AddEvent(this, &Player::EventTeleport, (uint32)mapchangeid, mapchangex, mapchangey, mapchangez, orientation, EVENT_PLAYER_TELEPORT, traveltime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Player::JumpToEndTaxiNode(TaxiPath * path)
{
    // this should *always* be safe in case it cant build your position on the path!
    TaxiPathNode * pathnode = path->GetPathNode((uint32)path->GetNodeCount()-1);
    if(!pathnode) return;

    SetTaxiState(false);
    SetTaxiPath(NULL);
    UnSetTaxiPos();
    m_taxi_ride_time = 0;

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    m_movementInterface.OnTaxiEnd();

    SafeTeleport(pathnode->mapid, 0, LocationVector(pathnode->x, pathnode->y, pathnode->z));
}

void Player::RemoveSpellsFromLine(uint16 skill_line)
{
    uint32 cnt = dbcSkillLineSpell.GetNumRows();
    for(uint32 i = 0; i < cnt; i++)
    {
        if(SkillLineAbilityEntry* sp = dbcSkillLineSpell.LookupRow(i))
        {
            if(sp->skilline == skill_line)
            {
                // Check ourselves for this spell, and remove it..
                removeSpell(sp->spell);
            }
        }
    }
}

void Player::PlayerRegeneratePower(bool is_interrupted)
{
    uint32 m_regenTimer = m_P_regenTimer; //set next regen time
    m_regenTimerCount += m_regenTimer;

    for(uint8 power = POWER_TYPE_MANA; power < POWER_TYPE_MAX; power++)
    {
        EUnitFields powerField = GetPowerFieldForType(power);
        if (powerField == UNIT_END)
            continue;

        uint32 curValue = GetPower(powerField), maxValue = GetMaxPower(EUnitFields(powerField+(UNIT_FIELD_MAXPOWERS-UNIT_FIELD_POWERS)));

        float addvalue = 0.0f;
        switch (power)
        {
        case POWER_TYPE_MANA:
            {
                addvalue += GetFloatValue(is_interrupted ? UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER : UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER) * 0.001f * m_regenTimer;
            }break;

        case POWER_TYPE_RAGE:
            {
                if(!curValue)
                    continue;

                if (!CombatStatus.IsInCombat() && !is_interrupted)
                {
                    addvalue += -20/0.05f;  // 2 rage by tick (= 2 seconds => 1 rage/sec)
                    if(m_regenTimer)
                        addvalue /= m_regenTimer;
                }
            }break;

        case POWER_TYPE_ENERGY:
            {
                addvalue += 0.01f * m_regenTimer;
            }break;

        case POWER_TYPE_RUNIC:
            {
                if(!curValue)
                    continue;

                if (!CombatStatus.IsInCombat() && !is_interrupted)
                {
                    addvalue += -30/0.3f;
                    if(m_regenTimer)
                        addvalue /= m_regenTimer;
                }
            }break;

        case POWER_TYPE_RUNE:
        case POWER_TYPE_FOCUS:
        case POWER_TYPE_HAPPINESS:
            continue;
            break;

        default:
            break;
        }

        if (addvalue < 0.0f)
        {
            if (curValue == 0)
                continue;
        }
        else if (addvalue > 0.0f)
        {
            if (curValue == maxValue)
                continue;
        }

        addvalue += m_powerFraction[power];
        float intval = 0.0f;
        m_powerFraction[power] = modf(addvalue, &intval);
        uint32 integerValue = uint32(abs(intval));
        if (addvalue < 0.0f)
        {
            if (curValue > integerValue)
                curValue -= integerValue;
            else
            {
                curValue = 0;
                m_powerFraction[power] = 0;
            }
        }
        else
        {
            curValue += integerValue;
            if (curValue > maxValue)
            {
                curValue = maxValue;
                m_powerFraction[power] = 0;
            }
        }

        if (m_regenTimerCount >= 2000 || m_powerFraction[power] == 0 || curValue == maxValue)
        {
            m_regenTimerCount = 0;
            SetPower(powerField, curValue);
        } else m_uint32Values[powerField] = curValue;
        continue;
    }
}

void Player::RegenerateHealth( bool inCombat )
{
    uint32 cur = GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 mh = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(cur >= mh)
        return;

    float Spirit = (float) GetUInt32Value(UNIT_FIELD_SPIRIT);
    uint8 Class = getClass(), level = ((getLevel() > MAXIMUM_ATTAINABLE_LEVEL) ? MAXIMUM_ATTAINABLE_LEVEL : getLevel());

    // This has some bad naming. HPRegen* is actually out of combat base, while HPRegenBase* is mana per spirit.
    float basespirit = ((Spirit > 50) ? 50 : Spirit);
    float basespiritdiff = Spirit - basespirit;
    float amt = ((basespiritdiff *0.2f)+basespirit);

    if(IsSitting())
        amt *= 1.5f;

    if(amt > 0)
    {
        if(amt <= 1.0f)//this fixes regen like 0.98
            cur++;
        else
            cur += float2int32(floor(amt)); // Crow: client always rounds down
        SetUInt32Value(UNIT_FIELD_HEALTH,(cur>=mh) ? mh : cur);
    }
}

uint32 Player::GeneratePetNumber()
{
    uint32 val = m_PetNumberMax + 1;
    for (uint32 i = 1; i < m_PetNumberMax; i++)
        if(m_Pets.find(i) == m_Pets.end())
            return i;                      // found a free one

    return val;
}

void Player::RemovePlayerPet(uint32 pet_number)
{
    PetLocks.Acquire();
    std::map<uint32, PlayerPet*>::iterator itr = m_Pets.find( pet_number );
    if( itr != m_Pets.end() )
    {
        delete itr->second;
        m_Pets.erase(itr);
        EventDismissPet();
    }
    PetLocks.Release();
}

void Player::_Relocate(uint32 mapid, const LocationVector& v, bool sendpending, bool force_new_world, uint32 instance_id)
{
    //Send transfer pending only when switching between differnt mapids!
    WorldPacket data(SMSG_TRANSFER_PENDING, 41);
    if(sendpending && mapid != m_mapId && force_new_world)
    {
        data << mapid;
        GetSession()->SendPacket(&data);
    }

    LocationVector destination(v);

    //are we changing maps?
    if(m_mapId != mapid || force_new_world)
    {
        //Preteleport will try to find an instance (saved or active), or create a new one if none found.
        uint32 status = sInstanceMgr.PreTeleport(mapid, this, instance_id);
        if(status != INSTANCE_OK && status != INSTANCE_OK_RESET_POS)
        {
            data.Initialize(SMSG_TRANSFER_ABORTED);
            data << mapid << status;
            GetSession()->SendPacket(&data);
            return;
        }

        if(status == INSTANCE_OK_RESET_POS)
        {
            MapInfo* info = WorldMapInfoStorage.LookupEntry(mapid);
            if(LastAreaTrigger != NULL && LastAreaTrigger->Type == ATTYPE_INSTANCE && LastAreaTrigger->Mapid == mapid)
            {
                destination.x = LastAreaTrigger->x;
                destination.y = LastAreaTrigger->y;
                destination.z = LastAreaTrigger->z;
                destination.o = LastAreaTrigger->o;
            }
            else if(info != NULL)
            {
                AreaTrigger* trigger = AreaTriggerStorage.LookupEntry(info->LinkedAreaTrigger);
                if(trigger != NULL && trigger->Type == ATTYPE_INSTANCE)
                {
                    destination.x = trigger->x;
                    destination.y = trigger->y;
                    destination.z = trigger->z;
                    destination.o = trigger->o;
                }
            }
            else
            {
                data.Initialize(SMSG_TRANSFER_ABORTED);
                data << mapid << status;
                GetSession()->SendPacket(&data);
                return;
            }
        }

        //remove us from this map
        if(IsInWorld()) RemoveFromWorld();

        //send new world
        m_movementInterface.TeleportToPosition(mapid, instance_id, destination);
        SetPlayerStatus(TRANSFER_PENDING);
    }
    else
    {
        // we are on same map allready, no further checks needed,
        // send teleport ack msg
        m_movementInterface.TeleportToPosition(destination);

        //reset transporter if we where on one.
        if( m_CurrentTransporter && !m_movementInterface.isTransportLocked() )
        {
            m_CurrentTransporter->RemovePlayer(castPtr<Player>(this));
            m_CurrentTransporter = NULL;
        }
    }

    //update position
    ApplyPlayerRestState(false); // If we don't, and we teleport inside, we'll be rested regardless.
}

uint32 Player::BuildCreateUpdateBlockForPlayer(ByteBuffer *data, Player* target )
{
    uint32 count = 0;
    if(target == castPtr<Player>(this)) // we need to send create objects for all items.
    {
        count += m_inventory.BuildCreateUpdateBlocks(data);
        count += m_bank.BuildCreateUpdateBlocks(data);
    }
    count += Unit::BuildCreateUpdateBlockForPlayer(data, target);
    return count;
}

void Player::_Warn(const char *message)
{
    sChatHandler.RedSystemMessage(GetSession(), message);
}

void Player::Kick(uint32 delay /* = 0 */)
{
    if(m_KickDelay = delay)
        sEventMgr.AddEvent(castPtr<Player>(this), &Player::_Kick, EVENT_PLAYER_KICK, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    else _Kick();
}

void Player::_Kick()
{
    if(m_KickDelay)
    {
        if(m_KickDelay > 1500)
            m_KickDelay -= 1000;
        else m_KickDelay = 0;

        if(m_KickDelay)
            return;
    }

    sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_PLAYER_KICK);
    m_session->Disconnect();
}

void Player::ClearCooldownForSpell(uint32 spell_id)
{
    if( IsInWorld() )
    {
        WorldPacket data(SMSG_CLEAR_COOLDOWN, 12);
        data << spell_id << GetGUID();
        GetSession()->SendPacket(&data);
    }

    // remove cooldown data from Server side lists
    PlayerCooldownMap::iterator itr, itr2;
    SpellEntry * spe = dbcSpell.LookupEntry(spell_id);
    if(spe == NULL)
        return;

    for(uint32 i = 0; i < NUM_COOLDOWN_TYPES; i++)
    {
        for( itr = m_cooldownMap[i].begin(); itr != m_cooldownMap[i].end(); )
        {
            itr2 = itr++;
            if( ( i == COOLDOWN_TYPE_CATEGORY && itr2->first == spe->Category ) ||
                ( i == COOLDOWN_TYPE_SPELL && itr2->first == spe->Id ) )
            {
                m_cooldownMap[i].erase( itr2 );
            }
        }
    }
}

void Player::ClearCooldownsOnLine(uint32 skill_line, uint32 called_from)
{
    // found an easier way.. loop spells, check skill line
    SpellSet::const_iterator itr = mSpells.begin();
    SkillLineAbilityEntry *sk;
    for(; itr != mSpells.end(); itr++)
    {
        if((*itr) == called_from)      // skip calling spell.. otherwise spammies! :D
            continue;

        sk = objmgr.GetSpellSkill((*itr));
        if(sk && sk->skilline == skill_line)
            ClearCooldownForSpell((*itr));
    }
}

void Player::ClearCooldownsOnLines(std::set<uint32> skill_lines, uint32 called_from)
{
    SpellSet::const_iterator itr = mSpells.begin();
    SkillLineAbilityEntry *sk;
    for(; itr != mSpells.end(); itr++)
    {
        if((*itr) == called_from)      // skip calling spell.. otherwise spammies! :D
            continue;

        sk = objmgr.GetSpellSkill((*itr));
        if(sk && sk->skilline && (skill_lines.find(sk->skilline) != skill_lines.end()))
            ClearCooldownForSpell((*itr));
    }
}

void Player::ResetAllCooldowns()
{
    std::set<uint32> skilllines;
    switch(getClass())
    {
    case WARRIOR:
        {
            skilllines.insert(26);
            skilllines.insert(256);
            skilllines.insert(257);
        } break;
    case PALADIN:
        {
            skilllines.insert(184);
            skilllines.insert(267);
            skilllines.insert(594);
        } break;
    case HUNTER:
        {
            skilllines.insert(50);
            skilllines.insert(51);
            skilllines.insert(163);
        } break;
    case ROGUE:
        {
            skilllines.insert(38);
            skilllines.insert(39);
            skilllines.insert(253);
        } break;
    case PRIEST:
        {
            skilllines.insert(56);
            skilllines.insert(78);
            skilllines.insert(613);
        } break;
    case DEATHKNIGHT:
        {
            skilllines.insert(770);
            skilllines.insert(771);
            skilllines.insert(772);
        } break;
    case SHAMAN:
        {
            skilllines.insert(373);
            skilllines.insert(374);
            skilllines.insert(375);
        } break;
    case MAGE:
        {
            skilllines.insert(6);
            skilllines.insert(8);
            skilllines.insert(237);
        } break;
    case WARLOCK:
        {
            skilllines.insert(355);
            skilllines.insert(354);
            skilllines.insert(593);
        } break;
    case DRUID:
        {
            skilllines.insert(573);
            skilllines.insert(574);
            skilllines.insert(134);
        } break;
    default:
        return;
        break;
    }
    ClearCooldownsOnLines(skilllines, 0);
}

void Player::sendMOTD()
{
    // Send first line of MOTD
    WorldPacket datat(SMSG_MOTD, 10);
    datat << uint32(1);
    datat << sWorld.GetMotd();
    SendPacket(&datat);

    // Send revision
    BroadcastMessage("%sServer:|r%s Project Ronin %s|r %s r%u/%s-%s-%s", MSG_COLOR_GOLD,
        MSG_COLOR_ORANGEY, MSG_COLOR_TORQUISEBLUE, BUILD_TAG, BUILD_REVISION, BUILD_HASH_STR, ARCH, CONFIG);
    BroadcastMessage("%sOnline Players:|r%s %u |r%sPeak:|r%s %u |r%sAccepted Connections:|r%s %u |r", MSG_COLOR_GOLD,
        MSG_COLOR_TORQUISEBLUE, sWorld.GetSessionCount(), MSG_COLOR_GOLD, MSG_COLOR_TORQUISEBLUE,
        sWorld.PeakSessionCount, MSG_COLOR_GOLD, MSG_COLOR_TORQUISEBLUE, sWorld.mAcceptedConnections);

    BroadcastMessage("%sServer Uptime:|r%s %s|r", MSG_COLOR_GOLD, MSG_COLOR_TORQUISEBLUE, sWorld.GetUptimeString().c_str());

    //Issue a message telling all guild members that this player has signed on
    guildmgr.PlayerLoggedIn(m_playerInfo);
}

void Player::PushOutOfRange(const WoWGuid & guid)
{
    _bufferS.Acquire();
    mOutOfRangeIds << guid;
    ++mOutOfRangeIdCount;

    // add to process queue
    if(m_mapMgr && !bProcessPending)
    {
        bProcessPending = true;
        m_mapMgr->PushToProcessed(castPtr<Player>(this));
    }
    _bufferS.Release();
}

void Player::PushUpdateBlock(ByteBuffer *data, uint32 updatecount)
{
    // imagine the bytebuffer getting appended from 2 threads at once! :D
    _bufferS.Acquire();

    // Set data size for limiting update blocks to 45Kb
    if( (data->size() + bUpdateDataBuffer.size() ) >= 0xAFFF )
        PopPendingUpdates();

    mUpdateDataCount += updatecount;
    bUpdateDataBuffer.append(data->contents(), data->size());

    // add to process queue
    if(m_mapMgr && !bProcessPending)
    {
        bProcessPending = true;
        m_mapMgr->PushToProcessed(castPtr<Player>(this));
    }

    _bufferS.Release();
}

void Player::PopPendingUpdates()
{
    _bufferS.Acquire();
    if(bUpdateDataBuffer.size() || mOutOfRangeIdCount)
    {
        size_t c = 0, bBuffer_size = 6 + bUpdateDataBuffer.size();
        if(mOutOfRangeIdCount) bBuffer_size += 5+mOutOfRangeIds.size();
        uint8 *update_buffer = NULL;
        if(IsInWorld())
        {
            GetMapMgr()->m_updateBuildBuffer.resize(bBuffer_size);
            update_buffer = (uint8*)GetMapMgr()->m_updateBuildBuffer.contents();
        } else update_buffer = new uint8[bBuffer_size];

        *(uint16*)&update_buffer[c] = uint16(GetMapId()); c += 2;
        *(uint32*)&update_buffer[c] = mUpdateDataCount+(mOutOfRangeIdCount?1:0); c += 4;
        if(mOutOfRangeIdCount)
        {
            *(uint8*)&update_buffer[c] = UPDATETYPE_OUT_OF_RANGE_OBJECTS; c += 1;
            *(uint32*)&update_buffer[c] = mOutOfRangeIdCount; c += 4;
            memcpy(&update_buffer[c], mOutOfRangeIds.contents(), mOutOfRangeIds.size());
            c += mOutOfRangeIds.size();
            mOutOfRangeIds.clear();
            mOutOfRangeIdCount = 0;
        }

        memcpy(&update_buffer[c], bUpdateDataBuffer.contents(), bUpdateDataBuffer.size());
        c += bUpdateDataBuffer.size();
        // clear our update buffer and count
        bUpdateDataBuffer.clear();
        mUpdateDataCount = 0;

        // compress update packet
        m_session->OutPacket(SMSG_UPDATE_OBJECT, (uint16)c, update_buffer); // send uncompressed packet -> because we failed

        if(IsInWorld())
            GetMapMgr()->m_updateBuildBuffer.clear();
        else delete [] update_buffer;
    }

    bProcessPending = false;
    _bufferS.Release();

    // send any delayed packets
    WorldPacket * pck;
    while(delayedPackets.size())
    {
        pck = delayedPackets.next();
        m_session->SendPacket(pck);
        delete pck;
    }
}

void Player::ClearAllPendingUpdates()
{
    _bufferS.Acquire();
    bProcessPending = false;
    mUpdateDataCount=0;
    bUpdateDataBuffer.clear();
    _bufferS.Release();
}

void Player::CleanupGossipMenu()
{
    if(CurrentGossipMenu)
    {
        delete CurrentGossipMenu;
        CurrentGossipMenu = NULL;
    }
}

void Player::Gossip_Complete()
{
    GetSession()->OutPacket(SMSG_GOSSIP_COMPLETE, 0, NULL);
    CleanupGossipMenu();
}

bool Player::AllowChannelAtLocation(uint32 dbcID, AreaTableEntry *areaTable)
{
    bool result = true;
    switch(dbcID)
    {
    case 2:
        {
            if(sWorld.trade_world_chat == 0)
                result = areaTable ? ((areaTable->AreaFlags & AREA_CITY_AREA) || (areaTable->AreaFlags & AREA_CITY)) : false;
        }break;
    case 26:
        {
            result = (LfgDungeonId[0] > 0 || LfgDungeonId[1] > 0 || LfgDungeonId[2] > 0);
        }break;
    }
    return result;
}

bool Player::UpdateChatChannel(const char* areaName, AreaTableEntry *areaTable, ChatChannelEntry* entry, Channel* channel)
{
    if(!AllowChannelAtLocation(entry->id, areaTable))
    {
        if(channel)
            channel->Part(this, false, true); // Part with notice but keep data!
        return true;
    }

    char name[255];
    sprintf(name, "%s", entry->pattern);
    if(entry->flags & 0x02)
        sprintf(name, entry->pattern, areaName);
    Channel *chn = channelmgr.GetCreateDBCChannel(name, this, entry->id);
    if(chn == NULL)
    {
        if(channel)
            channel->Part(this, true);
        return false;
    }
    else if(chn->HasMember(this))
        return false;

    if(channel && strcmp(name, channel->m_name.c_str()))
        channel->Part(this, true);
    chn->AttemptJoin(this, "");
    sLog.Debug("ChannelJoin", "Player %s joined channel %s", GetName(), name);
    return false;
}

void Player::EventDBCChatUpdate(uint32 dbcID)
{
    char areaName[255];
    AreaTableEntry *areaTable = dbcAreaTable.LookupEntry(m_zoneId);
    if(areaTable == NULL)
        areaTable = dbcAreaTable.LookupEntry(m_areaId);
    if(areaTable)
        sprintf(areaName, "%s", areaTable->name);
    else if(IsInWorld() && GetMapMgr()->GetMapInfo())
        sprintf(areaName, "%s", GetMapMgr()->GetMapInfo()->name);
    else sprintf(areaName, "City_%03u", GetMapId());

    if(dbcID == 0xFFFFFFFF)
    {
        if(!m_channelsbyDBCID.size())
            return;

        for(uint32 i = 0; i < dbcChatChannels.GetNumRows(); i++)
        {
            Channel *channel = NULL;
            ChatChannelEntry* entry = dbcChatChannels.LookupRow(i);
            if(m_channelsbyDBCID.find(entry->id) != m_channelsbyDBCID.end())
                channel = m_channelsbyDBCID.at(entry->id);
            if(UpdateChatChannel(areaName, areaTable, entry, channel))
                m_channelsbyDBCID.erase(entry->id);
        }
    }
    else
    {
        ChatChannelEntry* entry = dbcChatChannels.LookupEntry(dbcID);
        if(entry == NULL || !strlen(entry->pattern))
            return;

        std::map<uint32, Channel*>::iterator itr = m_channelsbyDBCID.find(dbcID);
        if(itr != m_channelsbyDBCID.end())
        {
            if(UpdateChatChannel(areaName, areaTable, entry, itr->second))
                m_channelsbyDBCID.erase(entry->id);
        }
        else
        {
            if(!AllowChannelAtLocation(dbcID, areaTable))
                return;

            char name[255];
            if(entry->flags & 0x02)
                sprintf(name, entry->pattern, areaName);
            else sprintf(name, "%s", entry->pattern);
            Channel *chn = channelmgr.GetCreateDBCChannel(name, this, entry->id);
            if(chn == NULL || chn->HasMember(this))
                return;
            chn->AttemptJoin(this, "");
        }
    }
}

void Player::SetGuildId(uint32 guildId)
{
    if(guildId) SetFlag(OBJECT_FIELD_TYPE, TYPEMASK_FLAG_IN_GUILD);
    else RemoveFlag(OBJECT_FIELD_TYPE, TYPEMASK_FLAG_IN_GUILD);
    SetUInt64Value(PLAYER_GUILDID, guildId ? MAKE_NEW_GUID(guildId, 0, HIGHGUID_TYPE_GUILD) : 0);
}

void Player::SendTradeUpdate(bool extended, PlayerTradeStatus status, bool ourStatus, uint32 misc, uint32 misc2)
{
    WorldPacket data;
    if(extended)
    {
        data.Initialize(SMSG_TRADE_STATUS_EXTENDED, 100);
        uint32 count = 0;
        if(m_tradeData)
        {
            for(uint32 i = 0; i < 7; i++)
                if(m_tradeData->tradeItems[i])
                    count++;
        }

        data << uint32(0); // TradeId
        data << uint32(0); // unk
        data << uint64(m_tradeData->gold);
        data << uint32(m_tradeData->enchantId);
        data << uint32(7);
        data << uint32(0);
        data << uint8(ourStatus);
        data << uint32(7);
        data.WriteBits(count, 22);
        ByteBuffer itemData;
        if(m_tradeData)
        {
            for(uint32 i = 0; i < 7; i++)
            {
                /*if(Item *item = m_tradeData->tradeItems[i])
                {
                    WoWGuid giftCreatorGuid = m_tradeData->tradeItems[i]->GetUInt64Value(ITEM_FIELD_GIFTCREATOR);
                    WoWGuid creatorGuid = m_tradeData->tradeItems[i]->GetUInt64Value(ITEM_FIELD_CREATOR);

                    data.WriteBit(giftCreatorGuid[7]);
                    data.WriteBit(giftCreatorGuid[1]);
                    bool notWrapped = data.WriteBit(!item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAPPED));
                    data.WriteBit(giftCreatorGuid[3]);

                    if (notWrapped)
                    {
                        data.WriteBit(creatorGuid[7]);
                        data.WriteBit(creatorGuid[1]);
                        data.WriteBit(creatorGuid[4]);
                        data.WriteBit(creatorGuid[6]);
                        data.WriteBit(creatorGuid[2]);
                        data.WriteBit(creatorGuid[3]);
                        data.WriteBit(creatorGuid[5]);
                        data.WriteBit(item->GetProto()->LockId != 0);
                        data.WriteBit(creatorGuid[0]);
                        itemData.WriteByteSeq(creatorGuid[1]);

                        itemData << uint32(0); // Perm enchantment
                        // Sockets
                        itemData << uint32(0) << uint32(0) << uint32(0);
                        itemData << uint32(item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY));

                        itemData.WriteByteSeq(creatorGuid[6]);
                        itemData.WriteByteSeq(creatorGuid[2]);
                        itemData.WriteByteSeq(creatorGuid[7]);
                        itemData.WriteByteSeq(creatorGuid[4]);

                        itemData << uint32(0); // Reforge type
                        itemData << uint32(item->GetUInt32Value(ITEM_FIELD_DURABILITY));
                        itemData << uint32(0); // Property id
                        itemData.WriteByteSeq(creatorGuid[3]);
                        itemData << uint32(0); // unk7
                        itemData.WriteByteSeq(creatorGuid[0]);
                        itemData << uint32(5); // Highest charges
                        itemData << uint32(0); // Seed
                        itemData.WriteByteSeq(creatorGuid[5]);
                    }

                    data.WriteBit(giftCreatorGuid[6]);
                    data.WriteBit(giftCreatorGuid[4]);
                    data.WriteBit(giftCreatorGuid[2]);
                    data.WriteBit(giftCreatorGuid[0]);
                    data.WriteBit(giftCreatorGuid[5]);
                    itemData.WriteByteSeq(giftCreatorGuid[6]);
                    itemData.WriteByteSeq(giftCreatorGuid[1]);
                    itemData.WriteByteSeq(giftCreatorGuid[7]);
                    itemData.WriteByteSeq(giftCreatorGuid[4]);
                    itemData << uint32(item->GetEntry());
                    itemData.WriteByteSeq(giftCreatorGuid[0]);
                    itemData << uint32(item->GetStackSize());
                    itemData.WriteByteSeq(giftCreatorGuid[5]);
                    itemData << uint8(i);
                    itemData.WriteByteSeq(giftCreatorGuid[2]);
                    itemData.WriteByteSeq(giftCreatorGuid[3]);
                }*/
            }
        }
        data.FlushBits();
        if(itemData.size())
            data.append(itemData.contents(), itemData.size());
    }
    else
    {
        data.Initialize(SMSG_TRADE_STATUS, 20);
        data.WriteBit(0);
        data.WriteBits(status, 5);
        if(status == TRADE_STATUS_BEGIN_TRADE)
        {
            if(m_tradeData == NULL)
                data.WriteBitString(8, 0, 0, 0, 0, 0, 0, 0, 0);
            else
            {
                data.WriteBit(m_tradeData->targetGuid[2]);
                data.WriteBit(m_tradeData->targetGuid[4]);
                data.WriteBit(m_tradeData->targetGuid[6]);
                data.WriteBit(m_tradeData->targetGuid[0]);
                data.WriteBit(m_tradeData->targetGuid[1]);
                data.WriteBit(m_tradeData->targetGuid[3]);
                data.WriteBit(m_tradeData->targetGuid[7]);
                data.WriteBit(m_tradeData->targetGuid[5]);
                data.WriteByteSeq(m_tradeData->targetGuid[4]);
                data.WriteByteSeq(m_tradeData->targetGuid[1]);
                data.WriteByteSeq(m_tradeData->targetGuid[2]);
                data.WriteByteSeq(m_tradeData->targetGuid[3]);
                data.WriteByteSeq(m_tradeData->targetGuid[0]);
                data.WriteByteSeq(m_tradeData->targetGuid[7]);
                data.WriteByteSeq(m_tradeData->targetGuid[6]);
                data.WriteByteSeq(m_tradeData->targetGuid[5]);
            }
        }
        else if(status == TRADE_STATUS_OPEN_WINDOW)
            data << uint32(0);
        else if(status == TRADE_STATUS_CLOSE_WINDOW)
        {
            data.WriteBit(ourStatus);
            data << uint32(misc);
            data << uint32(misc2);
        }
        else if(status == TRADE_STATUS_ONLY_CONJURABLE_CROSSREALM || status == TRADE_STATUS_NOT_ON_TAPLIST)
            data << uint8(misc);
        else if(status == TRADE_STATUS_CURRENCY_IS_BOUND || status == TRADE_STATUS_TRADING_CURRENCY)
            data << uint32(misc) << uint32(status == TRADE_STATUS_TRADING_CURRENCY ? misc2 : 0);
        else data.FlushBits();
    }

    if(ourStatus || m_tradeData == NULL)
        SendPacket(&data);
    else if(Player *plr = objmgr.GetPlayer(m_tradeData->targetGuid))
        if(plr->GetInstanceID() == GetInstanceID())
            plr->SendPacket(&data);
}

void Player::RequestDuel(Player* pTarget)
{
    // We Already Dueling or have already Requested a Duel
    if( DuelingWith != NULL )
        return;

    if( m_duelState != DUEL_STATE_FINISHED )
        return;

    SetDuelState( DUEL_STATE_REQUESTED );

    //Setup Duel
    pTarget->DuelingWith = castPtr<Player>(this);
    DuelingWith = pTarget;

    //Get Flags position
    float dist = CalcDistance(pTarget);
    dist = dist * 0.5f; //half way
    float x = (GetPositionX() + pTarget->GetPositionX()*dist)/(1+dist) + cos(GetOrientation()+(float(M_PI)/2))*2;
    float y = (GetPositionY() + pTarget->GetPositionY()*dist)/(1+dist) + sin(GetOrientation()+(float(M_PI)/2))*2;
    float z = (GetPositionZ() + pTarget->GetPositionZ()*dist)/(1+dist);

    //Create flag/arbiter
    GameObject* pGameObj = GetMapMgr()->CreateGameObject(21680);
    if( pGameObj == NULL || !pGameObj->CreateFromProto(21680,GetMapId(), x, y, z, GetOrientation()))
        return;
    pGameObj->SetInstanceID(GetInstanceID());

    //Spawn the Flag
    pGameObj->SetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY, GetGUID());
    pGameObj->SetUInt32Value(GAMEOBJECT_FACTION, GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, GetUInt32Value(UNIT_FIELD_LEVEL));

    //Assign the Flag
    SetUInt64Value(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());
    pTarget->SetUInt64Value(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());

    WorldPacket data(SMSG_DUEL_REQUESTED, 16);
    data << pGameObj->GetGUID();
    data << GetGUID();
    pTarget->GetSession()->SendPacket(&data);

    pGameObj->PushToWorld(m_mapMgr);
}

void Player::DuelCountdown()
{
    if( DuelingWith == NULL )
        return;

    m_duelCountdownTimer -= 1000;

    if( int32(m_duelCountdownTimer) < 0 )
        m_duelCountdownTimer = 0;

    if( m_duelCountdownTimer == 0 )
    {
        // Start Duel.
        SetPower(POWER_TYPE_RAGE, 0 );
        DuelingWith->SetPower(POWER_TYPE_RAGE, 0 );

        //Give the players a Team
        DuelingWith->SetUInt32Value( PLAYER_DUEL_TEAM, 1 ); // Duel Requester
        SetUInt32Value( PLAYER_DUEL_TEAM, 2 );

        SetDuelState( DUEL_STATE_STARTED );
        DuelingWith->SetDuelState( DUEL_STATE_STARTED );

        sEventMgr.AddEvent(castPtr<Player>(this), &Player::DuelBoundaryTest, EVENT_PLAYER_DUEL_BOUNDARY_CHECK, 500, 0, 0 );
        sEventMgr.AddEvent( DuelingWith, &Player::DuelBoundaryTest, EVENT_PLAYER_DUEL_BOUNDARY_CHECK, 500, 0, 0 );
    }
}

void Player::DuelBoundaryTest()
{
    //check if in bounds
    if(!IsInWorld())
        return;

    GameObject* pGameObject = GetMapMgr()->GetGameObject(GUID_LOPART(GetUInt64Value(PLAYER_DUEL_ARBITER)));
    if(!pGameObject)
    {
        EndDuel(DUEL_WINNER_RETREAT);
        return;
    }

    float Dist = GetDistance2dSq(pGameObject);

    if(Dist > 5625.0f)
    {
        // Out of bounds
        if(m_duelStatus == DUEL_STATUS_OUTOFBOUNDS)
        {
            // we already know, decrease timer by 500
            m_duelCountdownTimer -= 500;
            if(m_duelCountdownTimer == 0)
            {
                // Times up :p
                DuelingWith->EndDuel(DUEL_WINNER_RETREAT);
            }
        }
        else
        {
            // we just went out of bounds
            // set timer
            m_duelCountdownTimer = 10000;

            // let us know
            m_session->OutPacket(SMSG_DUEL_OUTOFBOUNDS, 4, &m_duelCountdownTimer);
            m_duelStatus = DUEL_STATUS_OUTOFBOUNDS;
        }
    }
    else
    {
        // we're in range
        if(m_duelStatus == DUEL_STATUS_OUTOFBOUNDS)
        {
            // just came back in range
            m_session->OutPacket(SMSG_DUEL_INBOUNDS);
            m_duelStatus = DUEL_STATUS_INBOUNDS;
        }
    }
}

void Player::EndDuel(uint8 WinCondition)
{
    if( m_duelState == DUEL_STATE_FINISHED )
        return;

    // Remove the events
    sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_PLAYER_DUEL_COUNTDOWN );
    sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_PLAYER_DUEL_BOUNDARY_CHECK );

    m_AuraInterface.UpdateDuelAuras();

    m_duelState = DUEL_STATE_FINISHED;
    if( DuelingWith == NULL )
        return;

    sEventMgr.RemoveEvents( DuelingWith, EVENT_PLAYER_DUEL_BOUNDARY_CHECK );
    sEventMgr.RemoveEvents( DuelingWith, EVENT_PLAYER_DUEL_COUNTDOWN );

    // spells waiting to hit
    sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_SPELL_DAMAGE_HIT);

    DuelingWith->m_AuraInterface.UpdateDuelAuras();
    DuelingWith->m_duelState = DUEL_STATE_FINISHED;

    //Announce Winner
    WorldPacket data( SMSG_DUEL_WINNER, 500 );
    data << uint8( WinCondition );
    data << GetName() << DuelingWith->GetName();
    SendMessageToSet( &data, true );

    data.Initialize(SMSG_DUEL_COMPLETE, 1);
    data << uint8( 1 );
    SendPacket(&data);
    DuelingWith->SendPacket(&data);

    //Clear Duel Related Stuff
    if( GameObject* arbiter = m_mapMgr ? GetMapMgr()->GetGameObject(GUID_LOPART(GetUInt64Value(PLAYER_DUEL_ARBITER))) : NULL )
    {
        arbiter->RemoveFromWorld( true );
        arbiter->Destruct();
        arbiter = NULL;
    }

    SetUInt64Value( PLAYER_DUEL_ARBITER, 0 );
    DuelingWith->SetUInt64Value( PLAYER_DUEL_ARBITER, 0 );

    SetUInt32Value( PLAYER_DUEL_TEAM, 0 );
    DuelingWith->SetUInt32Value( PLAYER_DUEL_TEAM, 0 );

    EventAttackStop();
    DuelingWith->EventAttackStop();

    // Call off pet
    if( GetSummon() != NULL )
    {
        GetSummon()->CombatStatus.Vanished();
        GetSummon()->GetAIInterface()->SetUnitToFollow( castPtr<Player>(this) );
        GetSummon()->GetAIInterface()->HandleEvent( EVENT_FOLLOWOWNER, GetSummon(), 0 );
        GetSummon()->GetAIInterface()->WipeTargetList();
    }

    // removing auras that kills players after if low HP
    m_AuraInterface.RemoveAllNegativeAuras(); // NOT NEEDED. External targets can always gank both duelers with DoTs. :D
    DuelingWith->m_AuraInterface.RemoveAllNegativeAuras();

    //Stop Players attacking so they don't kill the other player
    m_session->OutPacket( SMSG_CANCEL_COMBAT );
    DuelingWith->m_session->OutPacket( SMSG_CANCEL_COMBAT );

    smsg_AttackStop( DuelingWith );
    DuelingWith->smsg_AttackStop( castPtr<Player>(this) );

    DuelingWith->m_duelCountdownTimer = 0;
    m_duelCountdownTimer = 0;

    DuelingWith->DuelingWith = NULL;
    DuelingWith = NULL;
}

void Player::StopMirrorTimer(uint32 Type)
{
    m_session->OutPacket(SMSG_STOP_MIRROR_TIMER, 4, &Type);
}

void Player::EventTeleport(uint32 mapid, float x, float y, float z, float o)
{
    SafeTeleport(mapid, 0, LocationVector(x, y, z, o));
    sEventMgr.RemoveEvents(this,EVENT_PLAYER_TELEPORT);
}

void Player::BroadcastMessage(const char* Format, ...)
{
    va_list l;
    va_start(l, Format);
    char Message[1024];
    vsnprintf(Message, 1024, Format, l);
    va_end(l);

    WorldPacket data;
    sChatHandler.FillSystemMessageData(&data, Message);
    m_session->SendPacket(&data);
}

float Player::CalcPercentForRating( uint32 index, uint32 rating )
{
    uint32 relative_index = index - (PLAYER_FIELD_COMBAT_RATING_1);
    uint32 reallevel = GetUInt32Value(UNIT_FIELD_LEVEL);
    uint32 level = reallevel > MAXIMUM_ATTAINABLE_LEVEL ? MAXIMUM_ATTAINABLE_LEVEL : reallevel;
    float val = 1.0f;
    if( gtFloat * pDBCEntry = dbcCombatRating.LookupEntry( relative_index * 100 + level - 1 ) )
        val = pDBCEntry->val;
    return float(rating/val);
}

bool Player::SafeTeleport(uint32 MapID, uint32 InstanceID, float X, float Y, float Z, float O)
{
    return SafeTeleport(MapID, InstanceID, LocationVector(X, Y, Z, O));
}

bool Player::SafeTeleport(uint32 MapID, uint32 InstanceID, LocationVector vec)
{
    //abort duel if other map or new distance becomes bigger then 1600
    if(DuelingWith && (MapID != GetMapId() || m_position.Distance2DSq(vec) >= 1600) )       // 40
        EndDuel(DUEL_WINNER_RETREAT);

    //retrieve our bind point in case vector is 0,0,0
    if( vec.x == 0 && vec.y == 0 && vec.z == 0 )
    {
        vec.x = GetBindPositionX();
        vec.y = GetBindPositionY();
        vec.z = GetBindPositionZ();
        MapID = GetBindMapId();
    }

    if(GetShapeShift())
    {
        // Extra Check
        SetShapeShift(GetShapeShift());
    }

    /* Normal Version */
    bool force_new_world = false;

    // Lookup map info
    MapInfo * mapInfo = LimitedMapInfoStorage.LookupEntry(MapID);
    if(mapInfo == NULL)
        return false;

    //are we changing instance or map?
    if(InstanceID && (uint32)m_instanceId != InstanceID)
    {
        force_new_world = true;
        SetInstanceID(InstanceID);
    }
    else if(m_mapId != MapID)
    {
        force_new_world = true;
    }

    //We are going to another map
    if( force_new_world )
    {
        //Do we need TBC expansion?
        if(mapInfo->flags & WMI_INSTANCE_XPACK_01 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_01) && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
        {
            WorldPacket data;
            sChatHandler.FillSystemMessageData(&data, "You must have The Burning Crusade Expansion to access this content.");
            m_session->SendPacket(&data);
            return false;
        }

        //Do we need WOTLK expansion?
        if(mapInfo->flags & WMI_INSTANCE_XPACK_02 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
        {
            WorldPacket data;
            sChatHandler.FillSystemMessageData(&data, "You must have the Wrath of the Lich King Expansion to access this content.");
            m_session->SendPacket(&data);
            return false;
        }

        // Xpack gonna give it to ya
        if(mapInfo->flags & WMI_INSTANCE_XPACK_03 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_03))
        {
            WorldPacket data;
            sChatHandler.FillSystemMessageData(&data, "You must have the Cataclysm Expansion to access this content.");
            m_session->SendPacket(&data);
            return false;
        }
        // Dismount
        Dismount();
    }

    //no flying outside new continents
    if((GetShapeShift() == FORM_FLIGHT || GetShapeShift() == FORM_SWIFT) && MapID != 530 && MapID != 571 )
        RemoveShapeShiftSpell(m_ShapeShifted);

    //all set...relocate
    _Relocate(MapID, vec, true, force_new_world, InstanceID);
    return true;
}

void Player::SafeTeleport(MapMgr* mgr, LocationVector vec)
{
    if(IsInWorld())
        RemoveFromWorld();

    m_mapId = mgr->GetMapId();
    m_instanceId = mgr->GetInstanceID();
    WorldPacket data(SMSG_TRANSFER_PENDING, 20);
    data << mgr->GetMapId();
    GetSession()->SendPacket(&data);

    data.Initialize(SMSG_NEW_WORLD);
    data << vec.x << vec.o << vec.z;
    data << mgr->GetMapId() << vec.y;
    GetSession()->SendPacket(&data);

    SetPlayerStatus(TRANSFER_PENDING);
    SetPosition(vec);

    if(uint8 ss = GetShapeShift()) // Extra Check
        SetShapeShift(ss);

    if(DuelingWith != NULL)
        EndDuel(DUEL_WINNER_RETREAT);
}

void Player::UpdatePvPArea()
{
    AreaTableEntry *areaDBC = dbcAreaTable.LookupEntry(GetAreaId()), *zoneDBC = dbcAreaTable.LookupEntry(GetZoneId());
    if(areaDBC == NULL)
    {
        RemoveFFAPvPFlag();
        RemovePvPFlag();
        StopPvPTimer();
        return;
    }

    // This is where all the magic happens :P
    if((areaDBC->category == AREAC_ALLIANCE_TERRITORY && GetTeam() == 0) || (areaDBC->category == AREAC_HORDE_TERRITORY && GetTeam() == 1) ||
        zoneDBC && ((zoneDBC->category == AREAC_ALLIANCE_TERRITORY && GetTeam() == 0) || (zoneDBC->category == AREAC_HORDE_TERRITORY && GetTeam() == 1)))
    {
        if(!HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE) && !m_pvpTimer)
        {
            // I'm flagged and I just walked into a zone of my type. Start the 5min counter.
            ResetPvPTimer();
        }
        return;
    }
    else
    {
        //Enemy city check
        if(areaDBC->AreaFlags & AREA_CITY_AREA || areaDBC->AreaFlags & AREA_CITY)
        {
            if((areaDBC->category == AREAC_ALLIANCE_TERRITORY && GetTeam() == 1) || (areaDBC->category == AREAC_HORDE_TERRITORY && GetTeam() == 0) ||
                zoneDBC && ((zoneDBC->category == AREAC_ALLIANCE_TERRITORY && GetTeam() == 1) || (zoneDBC->category == AREAC_HORDE_TERRITORY && GetTeam() == 0)))
            {
                if(!IsPvPFlagged())
                    SetPvPFlag();
                StopPvPTimer();
                return;
            }
        }

        // I just walked into either an enemies town, or a contested zone.
        // Force flag me if i'm not already.
        if(HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY))
        {
            if(IsPvPFlagged())
                RemovePvPFlag();

            RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_FREE_FOR_ALL_PVP);

            StopPvPTimer();
        }
        else
        {
            //contested territory
            if(sWorld.IsPvPRealm)
            {
                //automaticaly sets pvp flag on contested territorys.
                if(!IsPvPFlagged())
                    SetPvPFlag();
                StopPvPTimer();
            }
            else
            {
                if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE))
                {
                    if(!IsPvPFlagged())
                        SetPvPFlag();
                }
                else if(!HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE) && IsPvPFlagged() && !m_pvpTimer)
                    ResetPvPTimer();
            }
        }
    }

    /* ffa pvp arenas will come later */
    if(areaDBC->AreaFlags & AREA_PVP_ARENA)
    {
        if(!IsPvPFlagged())
            SetPvPFlag();

        SetFFAPvPFlag();
    }
    else
    {
        RemoveFFAPvPFlag();
    }
}

void Player::BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag)
{
    Group* pGroup = NULL;
    Player *curPlr;
    for (WorldObject::InRangeSet::iterator iter = GetInRangePlayerSetBegin(); iter != GetInRangePlayerSetEnd();)
    {
        curPlr = GetInRangeObject<Player>(*iter);
        iter++;
        if((pGroup = curPlr->GetGroup()) && pGroup != GetGroup())
            BuildFieldUpdatePacket( curPlr, index, flag );
    }
}

void Player::LoginPvPSetup()
{
    // Make sure we know our area ID.
    _EventExploration();

    if(isAlive())
    {
        // Honorless target at 1st entering world.
        CastSpell(castPtr<Unit>(this), PLAYER_HONORLESS_TARGET_SPELL, true);

        //initialise BG
        if(GetMapMgr() && GetMapMgr()->m_battleground != NULL && !GetMapMgr()->m_battleground->HasStarted())
            GetMapMgr()->m_battleground->OnAddPlayer(this);
    }
}

void Player::PvPToggle()
{
    AreaTableEntry* at = dbcAreaTable.LookupEntry(GetAreaId());
    if(!sWorld.IsPvPRealm)
    {
        if(m_pvpTimer > 0)
        {
            // Means that we typed /pvp while we were "cooling down". Stop the timer.
            StopPvPTimer();

            SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);

            if(!IsPvPFlagged())
                SetPvPFlag();
        }
        else
        {
            if(IsPvPFlagged())
            {
                if(at != NULL && ( at->AreaFlags & AREA_CITY_AREA || at->AreaFlags & AREA_CITY) )
                {
                    if(!(at->category == AREAC_ALLIANCE_TERRITORY && GetTeam() == 1) || (at->category == AREAC_HORDE_TERRITORY && GetTeam() == 0))
                    {
                        // Start the "cooldown" timer.
                        ResetPvPTimer();
                    }
                }
                else
                {
                    // Start the "cooldown" timer.
                    ResetPvPTimer();
                }
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
            }
            else
            {
                // Move into PvP state.
                SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);

                StopPvPTimer();
                SetPvPFlag();
            }
        }
    }
    else
    {
        // This is where all the magic happens :P
        if((at->category == AREAC_ALLIANCE_TERRITORY && GetTeam() == ALLIANCE) || (at->category == AREAC_HORDE_TERRITORY && GetTeam() == HORDE))
        {
            if(m_pvpTimer > 0)
            {
                // Means that we typed /pvp while we were "cooling down". Stop the timer.
                StopPvPTimer();

                SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);

                if(!IsPvPFlagged())
                    SetPvPFlag();
            }
            else
            {
                if(IsPvPFlagged())
                {
                    // Start the "cooldown" timer.
                    ResetPvPTimer();

                    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
                }
                else
                {
                    // Move into PvP state.
                    SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);

                    StopPvPTimer();
                    SetPvPFlag();
                }
            }
        }
        else
        {
            if(!HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE))
                SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
            else
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
        }
    }
}

void Player::ResetPvPTimer()
{
    SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TIMER);
    m_pvpTimer = 300000;
}

void Player::CompleteLoading()
{
    SpellCastTargets targets;
    targets.m_unitTarget = GetGUID();
    targets.m_targetMask = TARGET_FLAG_UNIT;
    for(SpellSet::iterator itr = mSpells.begin(); itr != mSpells.end(); itr++)
    {
        SpellEntry *info = dbcSpell.LookupEntry(*itr);
        if( info  && info->isPassiveSpell() && !( info->isSpellExpiringWithPet() ))
        {
            if( info->RequiredShapeShift )
            {
                if( !( ((uint32)1 << (GetShapeShift()-1)) & info->RequiredShapeShift ) )
                    continue;
            }

            if(Spell* spell = new Spell(this, info))
                spell->prepare(&targets, true);
        }
    }

    m_talentInterface.InitActiveSpec();

    if(isDead()) // only add aura's to the living (death aura set elsewhere)
    {
        while(!m_loadAuras.empty())
        {
            Aura *aur = m_loadAuras.front();
            m_loadAuras.pop_front();
            delete aur;
        }
    }
    else
    {
        while(!m_loadAuras.empty())
        {
            Aura *aur = m_loadAuras.front();
            m_loadAuras.pop_front();
            AddAura(aur);
        }

        // warrior has to have battle stance
        if( getClass() == WARRIOR && !HasAura(2457))
            CastSpell(castPtr<Unit>(this), dbcSpell.LookupEntry(2457), true);
    }
    // this needs to be after the cast of passive spells, because it will cast ghost form, after the remove making it in ghost alive, if no corpse.

    // Update our field values
    UpdateFieldValues();

    if(iActivePet)
        SpawnPet(iActivePet);      // only spawn if >0

    // Banned
    if(IsBanned())
    {
        const char * message = ("This character is banned for  %s.\n You will be kicked in 30 secs.", GetBanReason().c_str());

        // Send warning after 30sec, as he might miss it if it's send inmedeately.
        sEventMgr.AddEvent( castPtr<Player>(this), &Player::_Warn, message, EVENT_UNIT_SENDMESSAGE, 30000, 1, 0);
        sEventMgr.AddEvent( castPtr<Player>(this), &Player::_Kick, EVENT_PLAYER_KICK, 60000, 1, 0 );
    }

    if(m_playerInfo->m_Group)
        sEventMgr.AddEvent(castPtr<Player>(this), &Player::EventGroupFullUpdate, EVENT_UNK, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    if(raidgrouponlysent)
    {
        WorldPacket data2(SMSG_RAID_GROUP_ONLY, 8);
        data2 << uint32(0xFFFFFFFF) << uint32(0);
        GetSession()->SendPacket(&data2);
        raidgrouponlysent=false;
    }

    sInstanceMgr.BuildSavedInstancesForPlayer(castPtr<Player>(this));
}

void Player::OnWorldPortAck()
{
    //only resurrect if player is porting to a instance portal
    MapInfo *pPMapinfo = NULL;
    pPMapinfo = LimitedMapInfoStorage.LookupEntry(GetMapId());
    MapEntry* map = dbcMap.LookupEntry(GetMapId());

    if(pPMapinfo != NULL)
    {
        if(isDead() && pPMapinfo->type != INSTANCE_NULL && pPMapinfo->type != INSTANCE_PVP)
            ResurrectPlayer();

        if(pPMapinfo->HasFlag(WMI_INSTANCE_WELCOME) && GetMapMgr())
        {
            std::string welcome_msg;
            welcome_msg = "Welcome to ";
            welcome_msg += pPMapinfo->name;
            if(map->IsRaid())
            {
                switch(iRaidType)
                {
                case MODE_10PLAYER_NORMAL:
                    welcome_msg += " (10 Player)";
                    break;
                case MODE_25PLAYER_NORMAL:
                    welcome_msg += " (25 Player)";
                    break;
                case MODE_10PLAYER_HEROIC:
                    welcome_msg += " (10 Player Heroic)";
                    break;
                case MODE_25PLAYER_HEROIC:
                    welcome_msg += " (25 Player Heroic)";
                    break;
                default:
                    break;
                }
            }
            else
            {
                switch(iInstanceType)
                {
                case MODE_5PLAYER_NORMAL:
                    welcome_msg += " (5 Player)";
                    break;
                case MODE_5PLAYER_HEROIC:
                    welcome_msg += " (5 Player Heroic)";
                    break;
                default:
                    break;
                }
            }
            welcome_msg += ". ";
            if(pPMapinfo->type != INSTANCE_NONRAID && m_mapMgr->pInstance)
            {
                /*welcome_msg += "This instance is scheduled to reset on ";
                welcome_msg += asctime(localtime(&m_mapMgr->pInstance->m_expiration));*/
                welcome_msg += "Instance Locks are scheduled to expire in ";
                welcome_msg += RONIN_UTIL::ConvertTimeStampToString((uint32)m_mapMgr->pInstance->m_expiration - UNIXTIME);
            }
            sChatHandler.SystemMessage(m_session, welcome_msg.c_str());
        }
    }
}

int32 Player::GetBonusesFromItems(uint32 statType)
{
    int32 bonus = 0;
    for(ItemBonusModMap::iterator itr = itemBonusMapByType[statType].begin(); itr != itemBonusMapByType[statType].end(); itr++)
        bonus += itr->second.second;
    return bonus;
}

void Player::ModifyBonuses(bool apply, uint64 guid, uint32 slot, uint32 type, int32 val)
{
    itemBonusMask.SetBit(type);
    std::pair<uint64, uint32> guid_slot = std::make_pair(guid, slot);
    if(apply)
    {
        if(itemBonusMap.find(guid_slot) == itemBonusMap.end())
            itemBonusMap.insert(std::make_pair(guid_slot, std::make_pair(type, val)));
        if(itemBonusMapByType[type].find(guid_slot) == itemBonusMapByType[type].end())
            itemBonusMapByType[type].insert(std::make_pair(guid_slot, std::make_pair(type, val)));
    }
    else
    {
        itemBonusMap.erase(guid_slot);
        itemBonusMapByType[type].erase(guid_slot);
    }
}

bool Player::CanSignCharter(Charter * charter, Player* requester)
{
    if(charter->CharterType >= CHARTER_TYPE_ARENA_2V2 && m_playerInfo->arenaTeam[charter->CharterType-1] != NULL)
        return false;

    if(charter->CharterType == CHARTER_TYPE_GUILD && m_playerInfo->GuildId != 0)
        return false;

    if(m_playerInfo->charterId[charter->CharterType] != 0 || requester->GetTeam() != GetTeam())
        return false;
    else
        return true;
}

void Player::SetShapeShift(uint8 ss)
{
    uint8 old_ss = GetByte( UNIT_FIELD_BYTES_2, 3 );
    SetByte( UNIT_FIELD_BYTES_2, 3, ss );

    //remove auras that we should not have
    m_AuraInterface.UpdateShapeShiftAuras(old_ss, ss);

    // apply any talents/spells we have that apply only in this form.
    std::set<uint32>::iterator itr;
    SpellCastTargets t(GetGUID());

    for( itr = mSpells.begin(); itr != mSpells.end(); itr++ )
    {
        SpellEntry *sp = dbcSpell.LookupEntry( *itr );
        if( sp == NULL)
            continue;
        if( sp->isSpellAppliedOnShapeshift() || sp->isPassiveSpell() )     // passive/talent
            if( sp->RequiredShapeShift && ((uint32)1 << (ss-1)) & sp->RequiredShapeShift )
                if(Spell *spe = new Spell( this, sp ))
                    spe->prepare( &t, true );
    }

    // now dummy-handler stupid hacky fixed shapeshift spells (leader of the pack, etc)
    for( itr = mShapeShiftSpells.begin(); itr != mShapeShiftSpells.end(); )
    {
        SpellEntry *sp = dbcSpell.LookupEntry( *itr );
        if( sp == NULL)
            continue;
        ++itr;

        if( sp->RequiredShapeShift && ((uint32)1 << (ss-1)) & sp->RequiredShapeShift )
            if(Spell *spe = new Spell( this, sp))
                spe->prepare( &t, true );
    }
}

uint32 Player::GetMainMeleeDamage(uint32 AP_owerride)
{
    float min_dmg, max_dmg;
/////////////////MAIN HAND
    float ap_bonus = (AP_owerride ? AP_owerride : CalculateAttackPower())/14000.0f;
    float delta = GetUInt32FloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS)-GetUInt32FloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG);
    if(IsInFeralForm())
    {
        float r = 0.f;
        uint32 lev = getLevel();
        if(GetShapeShift() == FORM_CAT)
            r = lev + delta + ap_bonus * 1000.0f;
        else r = lev + delta + ap_bonus * 2500.0f;
        min_dmg = r * 0.9f;
        max_dmg = r * 1.1f;
        return float2int32(std::max((min_dmg + max_dmg)/2.0f,0.0f));
    }
//////no druid ss
    ItemPrototype *proto = NULL;
    if(Item *it = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
        proto = disarmed ? NULL : it->GetProto();

    float bonus = ap_bonus*float(proto ? proto->Delay : 2000), tmp = 1.0f;
    min_dmg = ((delta+bonus+(proto ? proto->minDamage : 0))*tmp) * 0.9f;
    max_dmg = ((delta+bonus+(proto ? proto->maxDamage : 0))*tmp) * 1.1f;
    return float2int32(std::max((min_dmg + max_dmg)/2.0f, 0.0f));
}

void Player::EventPortToGM(uint32 guid)
{
    if( !IsInWorld() )
        return;

    Player* p = objmgr.GetPlayer(guid);
    if( p == NULL )
        return;

    SafeTeleport(p->GetMapId(),p->GetInstanceID(),p->GetPosition());
}

void Player::SendAreaTriggerMessage(const char * message, ...)
{
    va_list ap;
    va_start(ap, message);
    char msg[500];
    vsnprintf(msg, 500, message, ap);
    va_end(ap);

    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 6 + strlen(msg));
    data << (uint32)0 << msg << (uint8)0x00;
    m_session->SendPacket(&data);
}

void Player::removeSoulStone()
{
    if(!SoulStone)
        return;
    uint32 sSoulStone = 0;
    switch(SoulStone)
    {
    case 3026:
        {
            sSoulStone = 20707;
        }break;
    case 20758:
        {
            sSoulStone = 20762;
        }break;
    case 20759:
        {
            sSoulStone = 20763;
        }break;
    case 20760:
        {
            sSoulStone = 20764;
        }break;
    case 20761:
        {
            sSoulStone = 20765;
        }break;
    case 27240:
        {
            sSoulStone = 27239;
        }break;
    case 47882:
        {
            sSoulStone = 47883;
        }break;
    }
    RemoveAura(sSoulStone);
    SoulStone = SoulStoneReceiver = 0; //just incase
}

void Player::SoftDisconnect()
{
    //basic, but it stops crashes ^^
    m_session->Disconnect();
}

void Player::SetNoseLevel()
{
    // Set the height of the player
    switch (getRace())
    {
    case RACE_HUMAN:
        {
            if (getGender())
                m_noseLevel = 1.72f;
            else
                m_noseLevel = 1.78f;
        }break;

    case RACE_ORC:
        {
            if (getGender())
                m_noseLevel = 1.82f;
            else
                m_noseLevel = 1.98f;
        }break;

    case RACE_DWARF:
        {
            if (getGender())
                m_noseLevel = 1.27f;
            else
                m_noseLevel = 1.4f;
        }break;

    case RACE_NIGHTELF:
        {
            if (getGender())
                m_noseLevel = 1.84f;
            else
                m_noseLevel = 2.13f;
        }break;

    case RACE_UNDEAD:
        {
            if (getGender())
                m_noseLevel = 1.61f;
            else
                m_noseLevel = 1.8f;
        }break;

    case RACE_TAUREN:
        {
            if (getGender())
                m_noseLevel = 2.48f;
            else
                m_noseLevel = 2.01f;
        }break;

    case RACE_GNOME:
        {
            if (getGender())
                m_noseLevel = 1.06f;
            else
                m_noseLevel = 1.04f;
        }break;

    case RACE_TROLL:
        {
            if (getGender())
                m_noseLevel = 2.02f;
            else
                m_noseLevel = 1.93f;
        }break;

    case RACE_BLOODELF:
        {
            if (getGender())
                m_noseLevel = 1.83f;
            else
                m_noseLevel = 1.93f;
        }break;

    case RACE_DRAENEI:
        {
            if (getGender())
                m_noseLevel = 2.09f;
            else
                m_noseLevel = 2.36f;
        }break;
    }
}

void Player::Possess(Unit* pTarget)
{
    if( m_CurrentCharm )
        return;

    m_CurrentCharm = pTarget;
    if(pTarget->GetTypeId() == TYPEID_UNIT)
    {
        // unit-only stuff.
        pTarget->GetAIInterface()->StopMovement(0);
    }

    m_noInterrupt++;
    SetUInt64Value(UNIT_FIELD_CHARM, pTarget->GetGUID());
    SetUInt64Value(PLAYER_FARSIGHT, pTarget->GetGUID());
    pTarget->GetMapMgr()->ChangeFarsightLocation(castPtr<Player>(this), pTarget, true);

    pTarget->SetUInt64Value(UNIT_FIELD_CHARMEDBY, GetGUID());
    pTarget->SetCharmTempVal(pTarget->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    pTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    pTarget->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    /* send "switch mover" packet */
    WorldPacket data1(SMSG_CLIENT_CONTROL_UPDATE, 10);      /* burlex: this should be renamed SMSG_SWITCH_ACTIVE_MOVER :P */
    data1 << pTarget->GetGUID() << uint8(1);
    m_session->SendPacket(&data1);

    /* update target faction set */
    pTarget->_setFaction();

    /* build + send pet_spells packet */
    if(pTarget->m_temp_summon)
        return;

    std::list<uint32> avail_spells;
    for(std::map<uint32, AI_Spell*>::iterator itr = pTarget->GetAIInterface()->m_spells.begin(); itr != pTarget->GetAIInterface()->m_spells.end(); ++itr)
        avail_spells.push_back(itr->second->info->Id);
    std::list<uint32>::iterator itr = avail_spells.begin();

    WorldPacket data(SMSG_PET_SPELLS, pTarget->GetAIInterface()->m_spells.size() * 4 + 18);
    data << pTarget->GetGUID();
    data << uint16(0x0000);//unk1
    data << uint32(0x00000101);//unk2
    data << uint32(0x00000000);//unk3

    // First spell is attack.
    data << uint32(PET_SPELL_ATTACK);

    // Send the actionbar
    for(uint32 i = 1; i < 10; i++)
    {
        if(itr != avail_spells.end())
        {
            data << uint16((*itr)) << uint16(DEFAULT_SPELL_STATE);
            ++itr;
        }
        else
            data << uint16(0) << uint8(0) << uint8(i+5);
    }

    // Send the rest of the spells.
    data << uint8(avail_spells.size());
    for(itr = avail_spells.begin(); itr != avail_spells.end(); itr++)
        data << uint16(*itr) << uint16(DEFAULT_SPELL_STATE);

    SpellEntry* sp = NULL;
    data << uint8(avail_spells.size());
    for(itr = avail_spells.begin(); itr != avail_spells.end(); itr++)
    {
        data << uint32((sp = dbcSpell.LookupEntry(*itr))->Id);
        data << uint16(sp->Category);
        data << uint32(sp->RecoveryTime);
        data << uint32(sp->CategoryRecoveryTime);
    }

    m_session->SendPacket(&data);
}

void Player::UnPossess()
{
    if(!m_CurrentCharm)
        return;

    Unit* pTarget = m_CurrentCharm;
    m_CurrentCharm = NULL;

    m_noInterrupt--;
    SetUInt64Value(PLAYER_FARSIGHT, 0);
    pTarget->GetMapMgr()->ChangeFarsightLocation(castPtr<Player>(this), pTarget, false);
    SetUInt64Value(UNIT_FIELD_CHARM, 0);
    pTarget->SetUInt64Value(UNIT_FIELD_CHARMEDBY, 0);

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);
    pTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE);
    pTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, pTarget->GetCharmTempVal());
    pTarget->_setFaction();

    /* send "switch mover" packet */
    WorldPacket data(SMSG_CLIENT_CONTROL_UPDATE, 10);
    data << GetGUID() << uint8(1);
    m_session->SendPacket(&data);

    if(pTarget->m_temp_summon)
        return;

    data.Initialize(SMSG_PET_SPELLS);
    data << uint64(0);
    m_session->SendPacket(&data);
}

void Player::SummonRequest(WorldObject* Requestor, uint32 ZoneID, uint32 MapID, uint32 InstanceID, const LocationVector & Position)
{
    m_summonInstanceId = InstanceID;
    m_summonPos = Position;
    m_summoner = Requestor;
    m_summonMapId = MapID;

    WorldPacket data(SMSG_SUMMON_REQUEST, 16);
    data << Requestor->GetGUID() << ZoneID << uint32(120000);       // 2 minutes
    m_session->SendPacket(&data);
}

void Player::RemoveFromBattlegroundQueue(uint32 queueSlot, bool forced)
{
    if(queueSlot > 1)
        return;

    if( m_pendingBattleground[queueSlot] )
    {
        m_pendingBattleground[queueSlot]->RemovePendingPlayer(castPtr<Player>(this));
        m_pendingBattleground[queueSlot] = NULL;
    }
    m_bgIsQueued[queueSlot] = false;
    m_bgQueueType[queueSlot] = 0;
    m_bgQueueInstanceId[queueSlot] = 0;

    BattlegroundManager.SendBattlegroundQueueStatus(castPtr<Player>(this), queueSlot);
    sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_BATTLEGROUND_QUEUE_UPDATE_SLOT_1 + queueSlot);

    if(forced)
        sChatHandler.SystemMessage(m_session, "You were removed from the queue for the battleground for not joining after 2 minutes.");
}

void Player::_AddSkillLine(uint16 SkillLine, uint16 Curr_sk, uint16 Max_sk)
{
    SkillLineEntry * CheckedSkill = dbcSkillLine.LookupEntry(SkillLine);
    if (!CheckedSkill) //skill doesn't exist, exit here
        return;

    uint16 maxSkillLevel = 50+GetUInt32Value(PLAYER_FIELD_MAX_LEVEL)*5;

    // force to be within limits
    Max_sk = std::min(Max_sk, maxSkillLevel);
    Curr_sk = std::max(std::min(Curr_sk, Max_sk), (uint16)1);

    ItemProf * prof;
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if(itr != m_skills.end())
    {
        if( (Curr_sk > itr->second.CurrentValue && Max_sk >= itr->second.MaximumValue) || (Curr_sk == itr->second.CurrentValue && Max_sk > itr->second.MaximumValue) )
        {
            itr->second.CurrentValue = Curr_sk;
            itr->second.MaximumValue = Max_sk;
            _UpdateMaxSkillCounts();
        }
    }
    else
    {
        PlayerSkill inf;
        inf.Skill = CheckedSkill;
        inf.MaximumValue = Max_sk;
        inf.CurrentValue = ( inf.Skill->id != SKILL_RIDING ? Curr_sk : Max_sk );
        inf.SkillPos = GetFreeSkillPosition();
        inf.BonusValue = inf.BonusTalent = 0;
        m_skills.insert( std::make_pair( SkillLine, inf ) );
        m_skillsByIndex.insert(std::make_pair(inf.SkillPos, &inf));
        _UpdateSkillFields();
    }

    //Add to proficiency
    if(const ItemProf * prof = GetProficiencyBySkill(SkillLine))
    {
        if(prof->itemclass == 2 && ((weapon_proficiency&prof->subclass) == 0)
            || prof->itemclass == 4 && ((armor_proficiency&prof->subclass) == 0))
        {
            WorldPacket data(SMSG_SET_PROFICIENCY, 8);
            data << uint32(prof->itemclass);
            if(prof->itemclass == 4)
                data << uint32(armor_proficiency |= prof->subclass);
            else data << uint32(weapon_proficiency |= prof->subclass);
            m_session->SendPacket(&data);
        }
    }

    // hackfix for runeforging
    if(SkillLine == SKILL_RUNEFORGING)
    {
        if(!HasSpell(53341))
            addSpell(53341); // Rune of Cinderglacier
        if(!HasSpell(53343))
            addSpell(53343); // Rune of Razorice
    }
}

uint8 Player::GetFreeSkillPosition()
{
    uint8 index = 0xFF;
    for(uint8 i = 0; i < MAX_PLAYER_SKILLS; i++)
    {
        if(m_skillsByIndex.find(i) == m_skillsByIndex.end())
        {
            index = i;
            break;
        }
    }
    return index;
}

void Player::_UpdateSkillFields()
{
    for(uint8 i = 0; i < MAX_PLAYER_SKILLS; i++)
    {
        uint32 field = i/2, offset = i&1;
        if(m_skillsByIndex.find(i) != m_skillsByIndex.end())
        {
            PlayerSkill *skill = m_skillsByIndex.at(i);
            SetUInt16Value(PLAYER_SKILL_LINEID_0 + field, offset, skill->Skill->id);
            SetUInt16Value(PLAYER_SKILL_STEP_0 + field, offset, 0);
            SetUInt16Value(PLAYER_SKILL_RANK_0 + field, offset, skill->CurrentValue);
            SetUInt16Value(PLAYER_SKILL_MAX_RANK_0 + field, offset, skill->MaximumValue);
            SetUInt16Value(PLAYER_SKILL_MODIFIER_0 + field, offset, skill->BonusValue);
            SetUInt16Value(PLAYER_SKILL_TALENT_0 + field, offset, skill->BonusTalent);
        }
        else
        {
            SetUInt16Value(PLAYER_SKILL_LINEID_0 + field, offset, 0);
            SetUInt16Value(PLAYER_SKILL_STEP_0 + field, offset, 0);
            SetUInt16Value(PLAYER_SKILL_RANK_0 + field, offset, 0);
            SetUInt16Value(PLAYER_SKILL_MAX_RANK_0 + field, offset, 0);
            SetUInt16Value(PLAYER_SKILL_MODIFIER_0 + field, offset, 0);
            SetUInt16Value(PLAYER_SKILL_TALENT_0 + field, offset, 0);
        }
    }
}

bool Player::_HasSkillLine(uint16 SkillLine)
{
    return (m_skills.find(SkillLine) != m_skills.end());
}

void Player::_AdvanceSkillLine(uint16 SkillLine, uint16 Count /* = 1 */)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if(itr == m_skills.end())
    {
        /* Add it */
        _AddSkillLine(SkillLine, Count, getLevel() * 5);
        _UpdateMaxSkillCounts();
    }
    else
    {
        uint16 curr_sk = itr->second.CurrentValue;
        itr->second.CurrentValue = std::min(uint16(curr_sk + Count), itr->second.MaximumValue);

        if (itr->second.CurrentValue != curr_sk)
            _UpdateSkillFields();
    }
}

uint16 Player::_GetSkillLineMax(uint16 SkillLine)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    return (itr == m_skills.end()) ? 0 : itr->second.MaximumValue;
}

uint16 Player::_GetSkillLineCurrent(uint16 SkillLine, bool IncludeBonus /* = true */)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if(itr == m_skills.end())
        return 0;

    return (IncludeBonus ? itr->second.CurrentValue + itr->second.BonusValue : itr->second.CurrentValue);
}

void Player::_RemoveSkillLine(uint16 SkillLine)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if(itr == m_skills.end())
        return;

    m_skillsByIndex.erase(itr->second.SkillPos);
    m_skills.erase(itr);
    _UpdateSkillFields();
}

void Player::_UpdateMaxSkillCounts()
{
    if(m_skills.empty())
        return;

    for(SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); itr++)
    {
        uint16 new_max = 1;
        if( itr->second.Skill->id == SKILL_LOCKPICKING )
            new_max = 5 * getLevel();
        else
        {
            switch(itr->second.Skill->categoryId)
            {
            case SKILL_TYPE_WEAPON:
                {
                    new_max = 5 * getLevel();
                }break;
            case SKILL_TYPE_LANGUAGE:
                {
                    new_max = 300;
                }break;
            case SKILL_TYPE_PROFESSION:
            case SKILL_TYPE_SECONDARY:
                {
                    new_max = itr->second.MaximumValue;
                    if(itr->second.Skill->id == SKILL_RIDING)
                        itr->second.CurrentValue = new_max;
                }break;
            }
        }

        uint16 levelCap = 50+GetUInt32Value(PLAYER_FIELD_MAX_LEVEL)*5;

        //Update new max, forced to be within limits
        itr->second.MaximumValue = std::min(new_max, levelCap);

        //Check if current is above new max, if so, set new current to new max
        itr->second.CurrentValue = std::min(itr->second.CurrentValue, itr->second.MaximumValue);
    }

    //Always update client to prevent cached data messing up things later.
    _UpdateSkillFields();
}

void Player::_ModifySkillBonus(uint16 SkillLine, int16 Delta)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if(itr == m_skills.end())
        return;

    itr->second.BonusValue += Delta;
    _UpdateSkillFields();
}

void Player::_ModifySkillBonusByType(uint16 SkillType, int16 Delta)
{
    bool dirty = false;
    for(SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); itr++)
    {
        if(itr->second.Skill->categoryId == SkillType)
        {
            itr->second.BonusValue += Delta;
            dirty=true;
        }
    }

    if(dirty)
        _UpdateSkillFields();
}

/** Maybe this formula needs to be checked?
 * - Burlex
 */

float PlayerSkill::GetSkillUpChance()
{
    float diff = float(MaximumValue - CurrentValue);
    return (diff * 100.0f / float(MaximumValue)) / 3.0f;
}

void Player::_RemoveLanguages()
{
    for(SkillMap::iterator itr = m_skills.begin(), it2; itr != m_skills.end();)
    {
        if(itr->second.Skill->categoryId == SKILL_TYPE_LANGUAGE)
        {
            it2 = itr++;
            m_skillsByIndex.erase(it2->second.SkillPos);
            m_skills.erase(it2);
        } else ++itr;
    }
}

bool PlayerSkill::Reset(uint32 Id)
{
    MaximumValue = 0;
    CurrentValue = 0;
    BonusValue = 0;
    Skill = (Id == 0) ? NULL : dbcSkillLine.LookupEntry(Id);
    if(Skill == NULL)
        return false;

    return true;
}

void Player::_AddLanguages(bool All)
{
    /** This function should only be used at login, and after _RemoveLanguages is called.
     * Otherwise weird stuff could happen :P
     * - Burlex
     */

    PlayerSkill sk;
    uint32 spell_id;
    static uint32 skills[] = { SKILL_LANG_COMMON, SKILL_LANG_ORCISH, SKILL_LANG_DWARVEN, SKILL_LANG_DARNASSIAN, SKILL_LANG_TAURAHE, SKILL_LANG_THALASSIAN,
        SKILL_LANG_TROLL, SKILL_LANG_GUTTERSPEAK, SKILL_LANG_DRAENEI, 0 };

    if(All)
    {
        for(uint32 i = 0; skills[i] != 0; i++)
        {
            if(!skills[i])
                break;
            if(m_skills.find(skills[i]) != m_skills.end())
                continue;
            if(!sk.Reset(skills[i]))
                continue;

            sk.MaximumValue = sk.CurrentValue = 300;
            sk.SkillPos = GetFreeSkillPosition();
            m_skills.insert( std::make_pair(skills[i], sk) );
            m_skillsByIndex.insert(std::make_pair(sk.SkillPos, &sk));
            if((spell_id = ::GetSpellForLanguageSkill(skills[i])))
                addSpell(spell_id);
        }
    }
    else
    {
        for(std::list<CreateInfo_SkillStruct>::iterator itr = info->skills.begin(); itr != info->skills.end(); itr++)
        {
            SkillLineEntry *en = dbcSkillLine.LookupEntry(itr->skillid);
            if(en == NULL)
                continue;
            if(m_skills.find(itr->skillid) != m_skills.end())
                continue;

            if(en->categoryId == SKILL_TYPE_LANGUAGE)
            {
                if(sk.Reset(itr->skillid))
                {
                    sk.MaximumValue = sk.CurrentValue = 300;
                    sk.SkillPos = GetFreeSkillPosition();
                    m_skills.insert( std::make_pair(itr->skillid, sk) );
                    m_skillsByIndex.insert(std::make_pair(sk.SkillPos, &sk));
                    if((spell_id = ::GetSpellForLanguageSkill(itr->skillid)))
                        addSpell(spell_id);
                }
            }
        }
    }

    _UpdateSkillFields();
}

float Player::GetSkillUpChance(uint32 id)
{
    SkillMap::iterator itr = m_skills.find(id);
    if(itr == m_skills.end())
        return 0.0f;

    return itr->second.GetSkillUpChance();
}

void Player::_RemoveAllSkills()
{
    m_skills.clear();
    _UpdateSkillFields();
}

void Player::_AdvanceAllSkills(uint16 count, bool skipprof /* = false */, uint16 max /* = 0 */)
{
    bool dirty=false;
    for(SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); itr++)
    {
        if(itr->second.CurrentValue != itr->second.MaximumValue)
        {
            if((skipprof == true) && (itr->second.Skill->categoryId == SKILL_TYPE_PROFESSION))
                continue;

            if(max != 0)
            {
                if(max > itr->second.MaximumValue)
                    max = itr->second.MaximumValue;

                if((itr->second.CurrentValue + count) >= max)
                    itr->second.CurrentValue = max;

                dirty = true;
                continue;
            }

            itr->second.CurrentValue += count;
            if(itr->second.CurrentValue >= itr->second.MaximumValue)
                itr->second.CurrentValue = itr->second.MaximumValue;

            dirty = true;
        }
    }

    if(dirty)
        _UpdateSkillFields();
}

void Player::_ModifySkillMaximum(uint16 SkillLine, uint16 NewMax)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if(itr == m_skills.end())
        return;

    if(NewMax > itr->second.MaximumValue)
    {
        //Max out riding skill
        if(SkillLine == SKILL_RIDING)
            itr->second.CurrentValue = NewMax;

        itr->second.MaximumValue = NewMax;
        _UpdateSkillFields();
    }
}

void Player::RecalculateHonor()
{
    HonorHandler::RecalculateHonorFields(castPtr<Player>(this));
}

void Player::EventGroupFullUpdate()
{
    if(m_playerInfo->m_Group)
    {
        //m_playerInfo->m_Group->Update();
        m_playerInfo->m_Group->UpdateAllOutOfRangePlayersFor(castPtr<Player>(this));
    }
}

void Player::EjectFromInstance()
{
    if(m_bgEntryPointX && m_bgEntryPointY && m_bgEntryPointZ && !IS_INSTANCE(m_bgEntryPointMap))
        if(SafeTeleport(m_bgEntryPointMap, m_bgEntryPointInstance, m_bgEntryPointX, m_bgEntryPointY, m_bgEntryPointZ, m_bgEntryPointO))
            return;

    MapInfo* map = WorldMapInfoStorage.LookupEntry(GetMapId());
    if(map && (map->repopmapid >= 0 && !IS_INSTANCE(map->repopmapid)))
    {
        if(map->repopx && map->repopy && map->repopz)
            if(SafeTeleport(map->repopmapid, 0, map->repopx, map->repopy, map->repopz, 0)) // Should be nearest graveyard.
                return;
    }

    SafeTeleport(m_bind_mapid, 0, m_bind_pos_x, m_bind_pos_y, m_bind_pos_z, 0);
}

bool Player::HasQuestSpell(uint32 spellid) //Only for Cast Quests
{
    if (quest_spells.size()>0 && quest_spells.find(spellid) != quest_spells.end())
        return true;
    return false;
}
void Player::RemoveQuestSpell(uint32 spellid) //Only for Cast Quests
{
    if (quest_spells.size()>0)
        quest_spells.erase(spellid);
}

bool Player::HasQuestMob(uint32 entry) //Only for Kill Quests
{
    if (quest_mobs.size()>0 && quest_mobs.find(entry) != quest_mobs.end())
        return true;
    return false;
}
void Player::RemoveQuestMob(uint32 entry) //Only for Kill Quests
{
    if (quest_mobs.size()>0)
        quest_mobs.erase(entry);
}

PlayerInfo::PlayerInfo(WoWGuid _guid)
{
    guid = _guid;
    acct = 0;
    name = "";
    race = gender = _class = 0;
    team = curInstanceID = lastmapid = 0;
    lastpositionx = lastpositiony = lastpositionz = lastorientation = 0.f;
    lastOnline = 0; lastZone = lastLevel = 0;
    m_Group = NULL; subGroup = 0;
    GuildId = GuildRank = 0;
    m_loggedInPlayer = NULL;
    arenaTeam[0] = arenaTeam[1] = arenaTeam[2] = NULL;
    charterId[0] = charterId[1] = charterId[2] = 0;
}

PlayerInfo::~PlayerInfo()
{
    if(m_Group)
        m_Group->RemovePlayer(this);
}

void Player::SendPacket(WorldPacket * data)
{
    if(GetSession())
        GetSession()->SendPacket(data);
}

void Player::SendDelayedPacket(WorldPacket * data)
{
    delayedPackets.add(data);
}

void Player::CopyAndSendDelayedPacket(WorldPacket * data)
{
    WorldPacket * data2 = new WorldPacket(*data);
    delayedPackets.add(data2);
}

//if we charmed or simply summoned a pet, castPtr<Player>(this) function should get called
void Player::EventSummonPet( Pet* new_pet )
{
    if ( !new_pet )
        return ; //another wtf error

    SpellSet::iterator it,iter;
    for(iter= mSpells.begin();iter != mSpells.end();)
    {
        it = iter++;
        uint32 SpellID = *it;
        SpellEntry *spellInfo = dbcSpell.LookupEntry(SpellID);
        if( spellInfo->isSpellCastOnPetOwnerOnSummon() )
        {
            m_AuraInterface.RemoveAllAuras( SpellID, GetGUID() ); //this is required since unit::addaura does not check for talent stacking
            SpellCastTargets targets(GetGUID());
            if (Spell* spell = new Spell(this, spellInfo))    //we cast it as a proc spell, maybe we should not !
                spell->prepare(&targets, true);
        }
        else if( spellInfo->isSpellCastOnPetOnSummon() )
        {
            m_AuraInterface.RemoveAllAuras( SpellID, GetGUID() ); //this is required since unit::addaura does not check for talent stacking
            SpellCastTargets targets( new_pet->GetGUID() );
            if(Spell* spell = new Spell(this, spellInfo))    //we cast it as a proc spell, maybe we should not !
                spell->prepare(&targets, true);
        }
    }

    //there are talents that stop working after you gain pet
    m_AuraInterface.RemoveAllAurasExpiringWithPet();

    //pet should inherit some of the talents from caster
    //new_pet->InheritSMMods(); //not required yet. We cast full spell to have visual effect too
}

//if pet/charm died or whatever happened we should call this function
//!! note function might get called multiple times :P
void Player::EventDismissPet()
{
    m_AuraInterface.RemoveAllAurasExpiringWithPet();
}

void Player::AddShapeShiftSpell(uint32 id)
{
    SpellEntry * sp = dbcSpell.LookupEntry( id );
    mShapeShiftSpells.insert( id );

    if( sp->RequiredShapeShift && ((uint32)1 << (GetShapeShift()-1)) & sp->RequiredShapeShift )
    {
        SpellCastTargets t(GetGUID());
        if(Spell* spe = new Spell(this, sp))
            spe->prepare( &t, true );
    }
}

void Player::RemoveShapeShiftSpell(uint32 id)
{
    mShapeShiftSpells.erase( id );
    RemoveAura( id );
}

// COOLDOWNS
void Player::_Cooldown_Add(uint32 Type, uint32 Misc, uint32 Time, uint32 SpellId, uint32 ItemId)
{
    PlayerCooldownMap::iterator itr = m_cooldownMap[Type].find( Misc );
    if( itr != m_cooldownMap[Type].end( ) )
    {
        if( itr->second.ExpireTime < Time )
        {
            itr->second.ExpireTime = Time;
            itr->second.ItemId = ItemId;
            itr->second.SpellId = SpellId;
        }
    }
    else
    {
        PlayerCooldown cd;
        cd.ExpireTime = Time;
        cd.ItemId = ItemId;
        cd.SpellId = SpellId;

        m_cooldownMap[Type].insert( std::make_pair( Misc, cd ) );
    }

    sLog.Debug("Player","added cooldown for type %u misc %u time %u item %u spell %u", Type, Misc, Time - getMSTime(), ItemId, SpellId);
}

void Player::Cooldown_Add(SpellEntry * pSpell, Item* pItemCaster)
{
    uint32 mstime = getMSTime();
    int32 cool_time;

    if( CooldownCheat )
        return;

    if( pSpell->CategoryRecoveryTime > 0 && pSpell->Category )
    {
        cool_time = pSpell->CategoryRecoveryTime;
        if( pSpell->SpellGroupType )
        {
            SM_FIValue(SMT_COOLDOWN_DECREASE, &cool_time, pSpell->SpellGroupType);
            SM_PIValue(SMT_COOLDOWN_DECREASE, &cool_time, pSpell->SpellGroupType);
        }

        _Cooldown_Add( COOLDOWN_TYPE_CATEGORY, pSpell->Category, mstime + cool_time, pSpell->Id, pItemCaster ? pItemCaster->GetProto()->ItemId : 0 );
    }

    if( pSpell->RecoveryTime > 0 )
    {
        cool_time = pSpell->RecoveryTime;
        if( pSpell->SpellGroupType )
        {
            SM_FIValue(SMT_COOLDOWN_DECREASE, &cool_time, pSpell->SpellGroupType);
            SM_PIValue(SMT_COOLDOWN_DECREASE, &cool_time, pSpell->SpellGroupType);
        }

        _Cooldown_Add( COOLDOWN_TYPE_SPELL, pSpell->Id, mstime + cool_time, pSpell->Id, pItemCaster ? pItemCaster->GetProto()->ItemId : 0 );
    }
}

void Player::Cooldown_AddStart(SpellEntry * pSpell)
{
    if( pSpell->StartRecoveryTime == 0 || CooldownCheat)
        return;

    uint32 mstime = getMSTime();
    int32 atime;
    if( GetFloatValue(UNIT_MOD_CAST_SPEED) >= 1.0f )
        atime = pSpell->StartRecoveryTime;
    else
        atime = float2int32( float(pSpell->StartRecoveryTime) * GetFloatValue(UNIT_MOD_CAST_SPEED) );

    if( pSpell->SpellGroupType )
    {
        SM_FIValue(SMT_GLOBAL_COOLDOWN, &atime, pSpell->SpellGroupType);
        SM_PIValue(SMT_GLOBAL_COOLDOWN, &atime, pSpell->SpellGroupType);
    }

    if( atime <= 0 )
        return;

    if( pSpell->StartRecoveryCategory && pSpell->StartRecoveryCategory != 133 )     // if we have a different cool category to the actual spell category - only used by few spells
        _Cooldown_Add( COOLDOWN_TYPE_CATEGORY, pSpell->StartRecoveryCategory, mstime + atime, pSpell->Id, 0 );
    else                                    // no category, so it's a gcd
    {
        //sLog.outDebug("Global cooldown adding: %u ms", atime );
        m_globalCooldown = mstime + atime;
    }
}

void Player::Cooldown_OnCancel(SpellEntry *pSpell)
{
    if( pSpell->StartRecoveryTime == 0 || CooldownCheat)
        return;

    uint32 mstime = getMSTime();
    int32 atime = float2int32( float(pSpell->StartRecoveryTime) * GetFloatValue(UNIT_MOD_CAST_SPEED) );
    if( atime <= 0 )
        return;

    if( pSpell->StartRecoveryCategory )     // if we have a different cool category to the actual spell category - only used by few spells
        m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(pSpell->StartRecoveryCategory);
    else                                    // no category, so it's a gcd
        m_globalCooldown = mstime;
}

bool Player::Cooldown_CanCast(SpellEntry * pSpell)
{
    if(pSpell == NULL)
        return false;

    if(CooldownCheat)
        return true;

    PlayerCooldownMap::iterator itr;
    uint32 mstime = getMSTime();
    if( pSpell->Category )
    {
        itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find( pSpell->Category );
        if( itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end( ) )
        {
            if( mstime < itr->second.ExpireTime )
                return false;
            else
                m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase( itr );
        }
    }

    itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find( pSpell->Id );
    if( itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end( ) )
    {
        if( mstime < itr->second.ExpireTime )
            return false;
        else
            m_cooldownMap[COOLDOWN_TYPE_SPELL].erase( itr );
    }

    if( pSpell->StartRecoveryTime && m_globalCooldown )         /* gcd doesn't affect spells without a cooldown it seems */
    {
        if( mstime < m_globalCooldown )
            return false;
        else
            m_globalCooldown = 0;
    }

    return true;
}

void Player::Cooldown_AddItem(ItemPrototype * pProto, uint32 x)
{
    if( pProto->Spells[x].CategoryCooldown <= 0 && pProto->Spells[x].Cooldown <= 0 )
        return;

    ItemPrototype::ItemSpell* isp = &pProto->Spells[x];
    uint32 mstime = getMSTime();

    if( isp->CategoryCooldown > 0)
        _Cooldown_Add( COOLDOWN_TYPE_CATEGORY, isp->Category, isp->CategoryCooldown + mstime, isp->Id, pProto->ItemId );

    if( isp->Cooldown > 0 )
        _Cooldown_Add( COOLDOWN_TYPE_SPELL, isp->Id, isp->Cooldown + mstime, isp->Id, pProto->ItemId );
}

bool Player::Cooldown_CanCast(ItemPrototype * pProto, uint32 x)
{
    PlayerCooldownMap::iterator itr;
    ItemPrototype::ItemSpell* isp = &pProto->Spells[x];
    uint32 mstime = getMSTime();

    if( isp->Category )
    {
        itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find( isp->Category );
        if( itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end( ) )
        {
            if( mstime < itr->second.ExpireTime )
                return false;
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase( itr );
        }
    }

    itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find( isp->Id );
    if( itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end( ) )
    {
        if( mstime < itr->second.ExpireTime )
            return false;
        m_cooldownMap[COOLDOWN_TYPE_SPELL].erase( itr );
    }

    return true;
}

void Player::_Disconnect()
{
    m_session->Disconnect();
}

/************************************************************************/
/* SOCIAL                                                               */
/************************************************************************/

void Player::Social_AddFriend(std::string name, std::string note)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);

    PlayerInfo *info = objmgr.GetPlayerInfoByName(name.c_str());
    if( info == NULL ) // lookup the player
    {
        data << uint8(FRIEND_NOT_FOUND);
        m_session->SendPacket(&data);
        return;
    }

    if( info == m_playerInfo ) // are we ourselves?
    {
        data << uint8(FRIEND_SELF) << info->guid;
        m_session->SendPacket(&data);
        return;
    }

    if( info->team != m_playerInfo->team ) // team check
    {
        data << uint8(FRIEND_ENEMY) << info->guid;
        m_session->SendPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    std::map<WoWGuid, std::string>::iterator itr;
    if((itr = m_friends.find(info->guid)) != m_friends.end())
    {
        data << uint8(FRIEND_ALREADY) << info->guid;
        m_session->SendPacket(&data);
        m_socialLock.Release();
        return;
    }

    Player *newfriend = info->m_loggedInPlayer;
    data << uint8(newfriend ? FRIEND_ADDED_ONLINE : FRIEND_ADDED_OFFLINE);
    data << info->guid;
    if( newfriend )
    {
        data << note;
        data << info->m_loggedInPlayer->GetChatTag();
        data << info->m_loggedInPlayer->GetZoneId();
        data << info->lastLevel;
        data << uint32(info->_class);
    }

    m_friends.insert( std::make_pair(info->guid, note) );
    m_socialLock.Release();
    m_session->SendPacket(&data);

    // dump into the db
    CharacterDatabase.Execute("INSERT INTO social_friends VALUES(%u, %u, '%s')", GetLowGUID(), info->guid.getLow(), CharacterDatabase.EscapeString(std::string(note)).c_str());
}

void Player::Social_RemoveFriend(WoWGuid guid)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    if( guid == GetGUID() ) // are we ourselves?
    {
        data << uint8(FRIEND_SELF) << guid;
        m_session->SendPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    std::map<WoWGuid, std::string>::iterator itr;
    if((itr = m_friends.find(guid)) != m_friends.end())
        m_friends.erase(itr);
    m_socialLock.Release();

    data << uint8(FRIEND_REMOVED) << guid;
    m_session->SendPacket(&data);

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u AND friend_guid = %u", GetLowGUID(), guid.getLow());
}

void Player::Social_SetNote(WoWGuid guid, std::string note)
{
    m_socialLock.Acquire();
    std::map<WoWGuid, std::string>::iterator itr;
    if((itr = m_friends.find(guid)) == m_friends.end())
    {
        m_socialLock.Release();
        return;
    }
    itr->second = note;
    m_socialLock.Release();

    // Update db data
    CharacterDatabase.Execute("UPDATE social_friends SET note = \'%s\' WHERE character_guid = %u AND friend_guid = %u", CharacterDatabase.EscapeString(note).c_str(), GetLowGUID(), guid);
}

void Player::Social_AddIgnore(std::string name)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);

    // lookup the player
    PlayerInfo *info = objmgr.GetPlayerInfoByName(name.c_str());
    if( info == NULL )
    {
        data << uint8(FRIEND_IGNORE_NOT_FOUND);
        m_session->SendPacket(&data);
        return;
    }

    // are we ourselves?
    if( info == m_playerInfo )
    {
        data << uint8(FRIEND_IGNORE_SELF) << info->guid;
        m_session->SendPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    std::map<WoWGuid, std::string>::iterator itr;
    if((itr = m_ignores.find(info->guid)) != m_ignores.end())
    {
        data << uint8(FRIEND_IGNORE_ALREADY) << info->guid;
        m_session->SendPacket(&data);
        m_socialLock.Release();
        return;
    }

    data << uint8(FRIEND_IGNORE_ADDED) << info->guid;
    m_ignores.insert(std::make_pair(info->guid, "IGNORE"));

    m_socialLock.Release();
    m_session->SendPacket(&data);

    // dump into db
    CharacterDatabase.Execute("INSERT INTO social_ignores VALUES(%u, %u)", GetLowGUID(), info->guid);
}

void Player::Social_RemoveIgnore(WoWGuid guid)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);

    // are we ourselves?
    if( guid == GetGUID() )
    {
        data << uint8(FRIEND_IGNORE_SELF) << GetGUID();
        m_session->SendPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    std::map<WoWGuid, std::string>::iterator itr;
    if((itr = m_ignores.find(guid)) != m_ignores.end())
        m_ignores.erase(itr);

    data << uint8(FRIEND_IGNORE_REMOVED);
    data << uint64(guid);

    m_socialLock.Release();

    m_session->SendPacket(&data);

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u AND ignore_guid = %u", GetLowGUID(), guid.getLow());
}

bool Player::Social_IsIgnoring(WoWGuid guid)
{
    m_socialLock.Acquire();
    bool res = m_ignores.find(guid) != m_ignores.end();
    m_socialLock.Release();
    return res;
}

void Player::Social_SendFriendList(uint32 flag)
{
    m_socialLock.Acquire();
    uint32 listFlags = 0, count = 0;
    std::map<WoWGuid, std::string>::iterator itr;
    WorldPacket data(SMSG_CONTACT_LIST, 500);
    data << flag << count;
    if((listFlags=0x01) & flag)
    {
        for( itr = m_friends.begin(); itr != m_friends.end(); itr++ )
        {
            data << itr->first << listFlags << itr->second;
            if(0x01 & 0x01)
            {
                Player *plr = NULL;
                if(PlayerInfo *info = objmgr.GetPlayerInfo(itr->first))
                    plr = info->m_loggedInPlayer;
                data << uint8(plr ? plr->GetChatTag() : 0);
                if(plr)
                {
                    data << uint32(plr->GetZoneId());
                    data << uint32(plr->getLevel());
                    data << uint32(plr->getClass());
                }
            }
            count++;
        }
    }
    if((listFlags=0x02) & flag)
    {
        for( itr = m_ignores.begin(); itr != m_ignores.end(); itr++ )
        {
            data << itr->first << listFlags << itr->second;
            count++;
        }
    }
    if((listFlags=0x04) & flag)
    {
        for( itr = m_mutes.begin(); itr != m_mutes.end(); itr++ )
        {
            data << itr->first << listFlags << itr->second;
            count++;
        }
    }
    data.put<uint32>(4, count);
    m_socialLock.Release();
    m_session->SendPacket(&data);
}

void Player::GenerateLoot(Corpse* pCorpse)
{
    // default gold
    pCorpse->ClearLoot();
    pCorpse->GetLoot()->gold = 500;

    if( m_bg != NULL )
        m_bg->HookGenerateLoot(castPtr<Player>(this), pCorpse);
    TRIGGER_INSTANCE_EVENT( m_mapMgr, OnPlayerLootGen )( this, pCorpse );
}

uint32 Player::GetMaxPersonalRating(bool Ignore2v2)
{
    ASSERT(m_playerInfo != NULL);

    uint32 maxrating = 0;
    for (uint i = (Ignore2v2 ? ARENA_TEAM_TYPE_3V3 : ARENA_TEAM_TYPE_2V2); i < NUM_ARENA_TEAM_TYPES; i++)
    {
        if(m_playerInfo->arenaTeam[i] != NULL)
        {
            ArenaTeamMember *member = m_playerInfo->arenaTeam[i]->GetMemberByGuid(GetLowGUID());
            if (member != NULL)
            {
                if (member->PersonalRating > maxrating)
                    maxrating = member->PersonalRating;
            }
        }
    }

    return maxrating;
}

void Player::FullHPMP()
{
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_FULL_HPMP);

    if( isDead() )
        ResurrectPlayer();

    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));
    SetPower(POWER_TYPE_ENERGY, GetMaxPower(POWER_TYPE_ENERGY));
}

void Player::SetKnownTitle( int32 title, bool set )
{
    CharTitleEntry *entry = dbcCharTitle.LookupEntry(title);
    if(entry == NULL || HasKnownTitleByIndex(entry->bit_index))
        return;

    if(set == false && entry->bit_index == GetUInt32Value(PLAYER_CHOSEN_TITLE))
        SetUInt32Value(PLAYER_CHOSEN_TITLE, 0);

    uint32 field = PLAYER__FIELD_KNOWN_TITLES+(entry->bit_index / 32);
    uint32 flag = uint32(1 << (entry->bit_index % 32));

    if( set ) SetFlag(field, flag);
    else RemoveFlag(field, flag);

    WorldPacket data( SMSG_TITLE_EARNED, 8 );
    data << uint32( entry->bit_index ) << uint32( set ? 1 : 0 );
    m_session->SendPacket( &data );

    if(set && GetUInt32Value(PLAYER_CHOSEN_TITLE) == 0)
        SetUInt32Value(PLAYER_CHOSEN_TITLE, entry->bit_index);
}

bool Player::HasBattlegroundQueueSlot()
{
    if( m_bgIsQueued[0] &&  m_bgIsQueued[1] && m_bgIsQueued[2] )
        return false;

    return true;
}

uint32 Player::GetBGQueueSlot()
{
    for(uint32 i = 0; i < 3; i++)
    {
        if( !m_bgIsQueued[i] )
            return i;
    }

    return 0; // Using 0 so if bad code comes up, we don't make an access violation :P
}

uint32 Player::GetBGQueueSlotByBGType(uint32 type)
{
    for(uint32 i = 0; i < 3; i++)
    {
        if( m_bgQueueType[i] == type)
            return i;
    }
    return 0;
}

uint32 Player::HasBGQueueSlotOfType(uint32 type)
{
    for(uint32 i = 0; i < 3; i++)
    {
        if( m_bgIsQueued[i] &&
            m_bgQueueType[i] == type)
            return i;
    }

    return 4;
}

void Player::RetroactiveCompleteQuests()
{

}

void Player::ConvertRune(uint8 index, uint8 value)
{
    ASSERT(index < 6);
    m_runemask |= (1 << index);
    SetRune(index, value);
    SetRuneCooldown(index, 0);

    WorldPacket data(SMSG_CONVERT_RUNE, 2);
    data << (uint8)index;
    data << (uint8)value;
    GetSession()->SendPacket(&data);
}

bool Player::CanUseRunes(uint8 blood, uint8 frost, uint8 unholy)
{
    uint8 death = 0;
    for(uint8 i = 0; i < 6; i++)
    {
        if( m_runes[ i ] == RUNE_TYPE_BLOOD && blood )
            blood--;
        if( m_runes[ i ] == RUNE_TYPE_FROST && frost )
            frost--;
        if( m_runes[ i ] == RUNE_TYPE_UNHOLY && unholy )
            unholy--;

        if( m_runes[ i ] == RUNE_TYPE_DEATH )
            death++;
    }

    uint8 res = blood + frost + unholy;
    if( res == 0 )
        return true;

    if( death >= (blood + frost + unholy) )
        return true;

    return false;
}

void Player::ScheduleRuneRefresh(uint8 index, bool forceDeathRune)
{
    sEventMgr.RemoveEvents(castPtr<Player>(this), EVENT_PLAYER_RUNE_REGEN + index);
    sEventMgr.AddEvent(castPtr<Player>(this), &Player::ConvertRune, (uint8)index, (forceDeathRune ? uint8(RUNE_TYPE_DEATH) : baseRunes[index]), EVENT_PLAYER_RUNE_REGEN + index, 10000, 0, 0);
}

void Player::UseRunes(uint8 blood, uint8 frost, uint8 unholy, SpellEntry* pSpell)
{
    uint8 death = 0;
    for(uint8 i = 0; i < 6; i++)
    {
        if( m_runes[ i ] == RUNE_TYPE_BLOOD && blood )
        {
            blood--;
            m_runemask &= ~(1 << i);
            m_runes[ i ] = RUNE_TYPE_RECHARGING;
            ScheduleRuneRefresh(i);
            continue;
        }
        if( m_runes[ i ] == RUNE_TYPE_FROST && frost )
        {
            frost--;
            m_runemask &= ~(1 << i);
            m_runes[ i ] = RUNE_TYPE_RECHARGING;

            if( pSpell && pSpell->NameHash == SPELL_HASH_DEATH_STRIKE || pSpell->NameHash == SPELL_HASH_OBLITERATE && Rand(pSpell->procChance) )
                ScheduleRuneRefresh(i, true);
            else
                ScheduleRuneRefresh(i);
            continue;
        }
        if( m_runes[ i ] == RUNE_TYPE_UNHOLY && unholy )
        {
            unholy--;
            m_runemask &= ~(1 << i);
            m_runes[ i ] = RUNE_TYPE_RECHARGING;

            if( pSpell && pSpell->NameHash == SPELL_HASH_DEATH_STRIKE || pSpell->NameHash == SPELL_HASH_OBLITERATE && Rand(pSpell->procChance) )
                ScheduleRuneRefresh(i, true);
            else
                ScheduleRuneRefresh(i);
            continue;
        }

        if( m_runes[ i ] == RUNE_TYPE_DEATH )
            death++;
    }

    uint8 res = blood + frost + unholy;

    if( res == 0 )
        return;

    for(uint8 i = 0; i < 6; i++)
    {
        if( m_runes[ i ] == RUNE_TYPE_DEATH && res )
        {
            res--;
            m_runemask &= ~(1 << i);
            m_runes[ i ] = RUNE_TYPE_RECHARGING;
            ScheduleRuneRefresh(i);
        }
    }
}

uint8 Player::TheoreticalUseRunes(uint8 blood, uint8 frost, uint8 unholy)
{
    uint8 death = 0;
    uint8 runemask = m_runemask;
    for(uint8 i = 0; i < 6; i++)
    {
        if( m_runes[ i ] == RUNE_TYPE_DEATH && blood)
        {
            blood--;
            runemask &= ~(1 << i);
            SetRuneCooldown(i, 10000);
            continue;
        }

        if( m_runes[ i ] == RUNE_TYPE_DEATH && frost)
        {
            frost--;
            runemask &= ~(1 << i);
            SetRuneCooldown(i, 10000);
            continue;
        }

        if( m_runes[ i ] == RUNE_TYPE_DEATH && unholy)
        {
            unholy--;
            runemask &= ~(1 << i);
            SetRuneCooldown(i, 10000);
            continue;
        }

        if( m_runes[ i ] == RUNE_TYPE_BLOOD && blood )
        {
            blood--;
            runemask &= ~(1 << i);
            SetRuneCooldown(i, 10000);
            continue;
        }

        if( m_runes[ i ] == RUNE_TYPE_FROST && frost )
        {
            frost--;
            runemask &= ~(1 << i);
            SetRuneCooldown(i, 10000);
            continue;
        }

        if( m_runes[ i ] == RUNE_TYPE_UNHOLY && unholy )
        {
            unholy--;
            runemask &= ~(1 << i);
            SetRuneCooldown(i, 10000);
            continue;
        }
    }

    return runemask;
}

void Player::GroupUninvite(Player* targetPlayer, PlayerInfo *targetInfo)
{
    if ( targetPlayer == NULL && targetInfo == NULL )
    {
        GetSession()->SendPartyCommandResult(this, 0, "", ERR_PARTY_CANNOT_FIND);
        return;
    }

    if ( !InGroup() || targetInfo->m_Group != GetGroup() )
    {
        GetSession()->SendPartyCommandResult(this, 0, "", ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
        return;
    }

    if ( !IsGroupLeader() || targetInfo->m_Group->HasFlag(GROUP_FLAG_BATTLEGROUND_GROUP) )  // bg group
    {
        if(this != targetPlayer)
        {
            GetSession()->SendPartyCommandResult(this, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
            return;
        }
    }

    if(m_bg)
        return;

    Group *group = GetGroup();

    if(group)
        group->RemovePlayer(targetInfo);
}

void Player::SetTaximaskNode(uint32 nodeidx, bool Unset)
{
    uint8  field   = uint8((nodeidx - 1) / 32);
    uint32 submask = 1<<((nodeidx-1)%32);
    if(Unset)
    {
        //We have this node allready? Remove it
        if ((GetTaximask(field)& submask) == submask)
            SetTaximask(field,(GetTaximask(field)& ~submask));
    }
    else
    {
        //We don't have this node allready? Add it.
        if ((GetTaximask(field)& submask) != submask)
            SetTaximask(field,(GetTaximask(field)|submask));
    }
}

uint16 Player::FindQuestSlot( uint32 questid )
{
    for(uint16 i = 0; i < 50; i++)
        if( (GetUInt32Value(PLAYER_QUEST_LOG + i * 5)) == questid )
            return i;

    return 50;
}

void Player::UpdateKnownCurrencies(uint32 itemId, bool apply)
{
/*    if(CurrencyTypesEntry const* ctEntry = dbcCurrencyTypes.LookupEntry(itemId))
    {
        if(ctEntry)
        {
            if(apply)
            {
                uint64 oldval = GetUInt64Value( PLAYER_FIELD_KNOWN_CURRENCIES );
                uint64 newval = oldval | ( uint64((( uint32 )1) << (ctEntry->BitIndex-1)));
                SetUInt64Value( PLAYER_FIELD_KNOWN_CURRENCIES, newval );
            }
            else
            {
                uint64 oldval = GetUInt64Value( PLAYER_FIELD_KNOWN_CURRENCIES );
                uint64 newval = oldval & ~( uint64((( uint32 )1) << (ctEntry->BitIndex-1)));
                SetUInt64Value( PLAYER_FIELD_KNOWN_CURRENCIES, newval );
            }
        }
    }*/
}

uint32 Player::GetTotalItemLevel()
{
    uint32 playertotalitemlevel = 0;
    return playertotalitemlevel;
}

uint32 Player::GetAverageItemLevel(bool skipmissing)
{
    uint8 itemcount = 1;
    uint32 playertotalitemlevel = 1;
    return playertotalitemlevel/itemcount;
}

// Crow: Spellcheck explanation: Some spells have the required part, but they don't really need it.
uint32 Player::GetSpellForShapeshiftForm(uint8 ss, bool spellchecks)
{
    if(ss < 1)
        return 0;

    switch(ss)
    {
    case FORM_CAT:
        return 768;
        break;

    case FORM_TRAVEL:
        if(!spellchecks)
            return 783;
        break;

    case FORM_BEAR:
        return 5487;
        break;

    case FORM_DIREBEAR:
        if(!spellchecks)
            return 9634;
        break;

    case FORM_MOONKIN:
        if(!spellchecks)
            return 24858;
        break;

    case FORM_TREE:
        if(!spellchecks)
            return 33891;
        break;

    case FORM_GHOSTWOLF:
        if(!spellchecks)
            return 0;
        break;

    default:
        return 0;
        break;
    }
    return 0;
}

uint8 Player::GetChatTag() const
{
    if(bGMTagOn)
        return 4;
    else if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
        return 3;
    else if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
        return 1;
    return 0;
}

uint8 Player::GetGuildMemberFlags()
{
    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
        return 0x04;
    else if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
        return 0x02;
    return 0x01;
}

uint32 Player::GenerateShapeshiftModelId(uint32 form)
{
    switch(form)
    {
    case FORM_CAT:
        {
            if (GetTeam() == ALLIANCE) // Based on Hair color
            {
                uint8 hairColor = GetByte(PLAYER_BYTES, 3);
                switch (hairColor)
                {
                case 7: // Violet
                case 8:
                    return 29405;
                case 3: // Light Blue
                    return 29406;
                case 0: // Green
                case 1: // Light Green
                case 2: // Dark Green
                    return 29407;
                case 4: // White
                    return 29408;
                default: // original - Dark Blue
                    return 892;
                }
            }
            else if (getRace() == RACE_TAUREN) // Based on Skin color
            {
                uint8 skinColor = GetByte(PLAYER_BYTES, 0);
                if (getGender() == 0) // Male
                {
                    switch(skinColor)
                    {
                    case 12: // White
                    case 13:
                    case 14:
                    case 18: // Completly White
                        return 29409;
                    case 9: // Light Brown
                    case 10:
                    case 11:
                        return 29410;
                    case 6: // Brown
                    case 7:
                    case 8:
                        return 29411;
                    case 0: // Dark
                    case 1:
                    case 2:
                    case 3: // Dark Grey
                    case 4:
                    case 5:
                        return 29412;
                    default: // original - Grey
                        return 8571;
                    }
                }
                else switch (skinColor) // Female
                {
                case 10: // White
                    return 29409;
                case 6: // Light Brown
                case 7:
                    return 29410;
                case 4: // Brown
                case 5:
                    return 29411;
                case 0: // Dark
                case 1:
                case 2:
                case 3:
                    return 29412;
                default: // original - Grey
                    return 8571;
                }
            }
        }break;
    case FORM_DIREBEAR:
    case FORM_BEAR:
        {
            if (getRace() == RACE_NIGHTELF)
            {
                // Based on Hair color
                uint8 hairColor = GetByte(PLAYER_BYTES, 3);
                switch (hairColor)
                {
                case 0: // Green
                case 1: // Light Green
                case 2: // Dark Green
                    return 29413; // 29415?
                case 6: // Dark Blue
                    return 29414;
                case 4: // White
                    return 29416;
                case 3: // Light Blue
                    return 29417;
                default: // original - Violet
                    return 2281;
                }
            }
            else if (getRace() == RACE_TAUREN)
            {
                // Based on Skin color
                uint8 skinColor = GetByte(PLAYER_BYTES, 0);
                if (getGender() == 0) // Male
                {
                    switch (skinColor)
                    {
                    case 0: // Dark (Black)
                    case 1:
                    case 2:
                        return 29418;
                    case 3: // White
                    case 4:
                    case 5:
                    case 12:
                    case 13:
                    case 14:
                        return 29419;
                    case 9: // Light Brown/Grey
                    case 10:
                    case 11:
                    case 15:
                    case 16:
                    case 17:
                        return 29420;
                    case 18: // Completly White
                        return 29421;
                    default: // original - Brown
                        return 2289;
                    }
                }
                else switch (skinColor) // Female
                {
                case 0: // Dark (Black)
                case 1:
                    return 29418;
                case 2: // White
                case 3:
                    return 29419;
                case 6: // Light Brown/Grey
                case 7:
                case 8:
                case 9:
                    return 29420;
                case 10: // Completly White
                    return 29421;
                default: // original - Brown
                    return 2289;
                }
            }
        }break;
    case FORM_FLIGHT:
        return GetTeam() ? 20872 : 20857;
    case FORM_MOONKIN:
        return GetTeam() ? 15375 : 15374;
    case FORM_SWIFT:
        return GetTeam() ? 21244 : 21243;
    case FORM_TRAVEL:
        return 632;
    case FORM_AQUA:
        return 2428;
    case FORM_GHOUL:
        return 24994;
    case FORM_CREATUREBEAR:
        return 902;
    case FORM_GHOSTWOLF:
        return 4613;
    case FORM_DEMON:
        return 25277;
    case FORM_MASTER_ANGLER:
        return 15234;
    case FORM_TREE:
        return 864;
    case FORM_SPIRITOFREDEMPTION:
        return 16031;
    }
    return 0;
}

void Player::ClearRuneCooldown(uint8 index)
{
    WorldPacket data(SMSG_ADD_RUNE_POWER, 4);
    data << uint32(1 << index);
    GetSession()->SendPacket(&data);
}

void Player::StartQuest(uint32 Id)
{
    Quest * qst = sQuestMgr.GetQuestPointer(Id);
    if(!qst)
        return;
    
    if( GetQuestLogForEntry( qst->id ) || HasFinishedQuest(qst->id))
        return;

    int32 log_slot = GetOpenQuestSlot();

    if (log_slot == -1)
    {
        sQuestMgr.SendQuestLogFull(this);
        return;
    }
    QuestLogEntry *qle = new QuestLogEntry();
    qle->Init(qst, this, log_slot);
    qle->UpdatePlayerFields();

    // If the quest should give any items on begin, give them the items.
    for(uint32 i = 0; i < 4; i++)
    {
        if(qst->receive_items[i])
        {
            if(Item* item = objmgr.CreateItem( qst->receive_items[i], this))
            {
                item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, qst->receive_itemcount[i]);
                if(!GetInventory()->AddInventoryItemToSlot(item, INVENTORY_SLOT_NONE))
                {
                    sItemMgr.DeleteItemData(item->GetGUID(), true);
                    item->Destruct();
                }
            }
        }
    }

    /*if(qst->srcitem && qst->srcitem != qst->receive_items[0])
    {
        if(Item* item = objmgr.CreateItem( qst->srcitem, this ))
        {
            item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, qst->srcitemcount ? qst->srcitemcount : 1);
            if(!GetInventory()->AddItemToFreeSlot(item))
                item->Destruct();
        }
    }*/
    TRIGGER_QUEST_EVENT(Id, OnQuestStart)(this, qle);

    sQuestMgr.OnQuestAccepted(this,qst,NULL);
}

DrunkenState Player::GetDrunkenstateByValue(uint16 value)
{
    /*if(value >= 25560)
        return DRUNKEN_VOMIT;*/
    if(value >= 23000)
        return DRUNKEN_SMASHED;
    if(value >= 12800)
        return DRUNKEN_DRUNK;
    if(value & 0xFFFE)
        return DRUNKEN_TIPSY;
    return DRUNKEN_SOBER;
}

void Player::EventDrunkenVomit()
{
    CastSpell(this, 67468, false);
    m_drunk -= 2560;
    sEventMgr.RemoveEvents(this, EVENT_DRUNKEN_VOMIT);
}
