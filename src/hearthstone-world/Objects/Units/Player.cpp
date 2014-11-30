/***
 * Demonstrike Core
 */

#include "StdAfx.h"

UpdateMask Player::m_visibleUpdateMask;

#define COLLISION_MOUNT_CHECK_INTERVAL 1000

static const uint32 DKNodesMask[12] = {0xFFFFFFFF,0xF3FFFFFF,0x317EFFFF,0,0x2004000,0x1400E0,0xC1C02014,0x12018,0x380,0x4000C10,0,0};//all old continents are available to DK's by default.

Player::Player(uint64 guid) : Unit(guid), m_talentInterface()
{
    m_valuesCount += PLAYER_LENGTH;
    m_updateMask.SetCount(m_valuesCount);
    m_object.m_objType |= TYPEMASK_TYPE_PLAYER;
    m_raw.values[OBJECT_LAYER_PLAYER] = new uint32[PLAYER_LENGTH];
    memset(m_raw.values[OBJECT_LAYER_PLAYER], 0, PLAYER_LENGTH*sizeof(uint32));

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
    m_talentInterface.Initialize(this);
    m_lastAreaUpdateMap             = -1;
    m_oldZone                       = 0;
    m_oldArea                       = 0;
    m_mailBox                       = new Mailbox( GetUInt32Value(OBJECT_FIELD_GUID) );
    m_ItemInterface                 = new ItemInterface(this);
    m_achievementInterface          = new AchievementInterface(this);
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
    Seal                            = 0;
    m_session                       = 0;
    TrackingSpell                   = 0;
    m_status                        = 0;
    offhand_dmg_mod                 = 0.5;
    m_walkSpeed                     = 2.5f;
    m_runSpeed                      = PLAYER_NORMAL_RUN_SPEED;
    m_isMoving                      = false;
    m_isWaterWalking                = 0;
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
    m_blockfromspellPCT             = 0;
    m_blockfromspell                = 0;
    m_critfromspell                 = 0;
    m_spellcritfromspell            = 0;
    m_dodgefromspell                = 0;
    m_parryfromspell                = 0;
    m_hitfromspell                  = 0;
    m_hitfrommeleespell             = 0;
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
    m_flyHackChances                = 5;
    m_WaterWalkTimer                = 0;
    m_lastWarnCounter               = 0;
    for(uint8 i = 0; i < 3; i++)
    {
        LfgType[i]      = 0;
        LfgDungeonId[i] = 0;
    }
    OnlineTime                      = 0;
    load_health                     = 0;
    for(uint8 i = 0; i < 5; i++)
        load_power[i]               = 0;
    m_Autojoin                      = false;
    m_AutoAddMem                    = false;
    LfmDungeonId                    = 0;
    LfmType                         = 0;
    m_invitersGuid                  = 0;
    forget                          = 0;
    m_currentMovement               = MOVE_UNROOT;
    m_isGmInvisible                 = false;
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
    m_playedtime[0]                 = 0;
    m_playedtime[1]                 = 0;
    m_playedtime[2]                 = (uint32)UNIXTIME;
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
    m_bUnlimitedBreath              = false;
    m_UnderwaterState               = 0;
    m_LastUnderwaterState           = 0;
    m_MirrorTimer[0]                = -1;
    m_MirrorTimer[1]                = -1;
    m_MirrorTimer[2]                = -1;
    m_UnderwaterTime                = 180000;
    m_UnderwaterMaxTime             = 180000;
    m_AutoShotTarget                = 0;
    m_onAutoShot                    = false;
    m_AutoShotDuration              = 0;
    m_AutoShotAttackTimer           = 0;
    m_AutoShotSpell                 = NULL;
    m_AttackMsgTimer                = 0;
    m_GM_SelectedGO                 = NULL;

    for(uint8 x = 0;x < 7; ++x)
    {
        SpellDelayResist[x] = 0;
        m_casted_amount[x] = 0;
    }

    for(uint8 x = 0; x < 5; ++x)
    {
        StatModPctPos[x] = 0;
        StatModPctNeg[x] = 0;
        TotalStatModPctPos[x] = 0;
        TotalStatModPctNeg[x] = 0;
    }

    for(uint8 x = 0; x < 12; ++x)
    {
        IncreaseDamageByType[x] = 0;
        IncreaseDamageByTypePCT[x] = 0;
        IncreaseCricticalByTypePCT[x] = 0;
    }

    m_regenTimerCount = 0;
    for (uint8 i = 0; i < 6; i++)
        m_powerFraction[i] = 0;
    sentMOTD                        = false;
    PctIgnoreRegenModifier          = 0.0f;
    DetectedRange                   = 0;
    m_targetIcon                    = 0;
    m_MountSpellId                  = 0;
    bHasBindDialogOpen              = false;
    m_CurrentCharm                  = NULL;
    m_CurrentTransporter            = NULL;
    m_SummonedObject                = NULL;
    m_currentLoot                   = (uint64)NULL;
    pctReputationMod                = 0;
    roll                            = 0;
    mUpdateDataCount                = 0;
    mOutOfRangeIdCount              = 0;
    bUpdateDataBuffer.reserve(65000);
    mOutOfRangeIds.reserve(1000);
    bProcessPending                 = false;

    for(uint8 i = 0; i < QUEST_LOG_COUNT; i++)
        m_questlog[i] = NULL;

    CurrentGossipMenu               = NULL;
    ResetHeartbeatCoords();
    rageFromDamageDealt             = 0;
    m_honorToday                    = 0;
    m_honorYesterday                = 0;
    m_honorPoints                   = 0;
    m_killsToday                    = 0;
    m_killsYesterday                = 0;
    m_killsLifetime                 = 0;
    m_honorless                     = false;
    m_lastSeenWeather               = 0;
    m_attacking                     = false;
    myCorpse                        = NULL;
    blinked                         = false;
    blinktimer                      = getMSTime();
    m_speedhackChances              = 3;
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
    resend_speed                    = false;
    rename_pending                  = false;
    titanGrip                       = false;
    iInstanceType                   = 0;
    iRaidType                       = 0;
    m_XPoff                         = false;
    customizable                    = false;
    memset(reputationByListId, 0, sizeof(FactionReputation*) * 128);
    AnnihilationProcChance          = 0;
    m_comboTarget                   = 0;
    m_comboPoints                   = 0;
    UpdateLastSpeeds();
    m_resist_critical[0] = m_resist_critical[1] = 0;
    for (uint8 x = 0; x < 3; ++x)
    {
        m_resist_hit[x] = 0;
        m_skipCastCheck[x] = 0;
        m_castFilter[x] = 0;
    }
    for(uint8 i = 0; i < 6; i++)
        m_runes[i] = baseRunes[i];

    JudgementSpell                  = 0;
    ok_to_remove                    = false;
    trigger_on_stun                 = 0;
    trigger_on_stun_chance          = 100;
    m_modphyscritdmgPCT             = 0;
    m_RootedCritChanceBonus         = 0;
    m_Illumination_amount           = 0;
    m_rap_mod_pct                   = 0;
    m_modblockabsorbvalue           = 0;
    m_modblockvaluefromspells       = 0;
    Damageshield_amount             = 0.0f;
    m_summoner                      = NULL;
    m_summonInstanceId              = m_summonMapId = 0;
    m_lastMoveType                  = 0;
    m_tempSummon                    = NULL;
    m_spellcomboPoints              = 0;
    m_pendingBattleground[0]        = NULL;
    m_pendingBattleground[1]        = NULL;
    m_deathVision                   = false;
    m_retainComboPoints             = false;
    last_heal_spell                 = NULL;
    m_playerInfo                    = NULL;
    m_sentTeleportPosition.ChangeCoords(999999.0f,999999.0f,999999.0f);
    m_speedChangeCounter            = 1;
    memset(&m_bgScore,0,sizeof(BGScore));
    m_arenaPoints                   = 0;
    m_base_runSpeed                 = m_runSpeed;
    m_base_walkSpeed                = m_walkSpeed;
    m_arenateaminviteguid           = 0;
    m_arenaPoints                   = 0;
    m_honorRolloverTime             = 0;
    raidgrouponlysent               = false;
    m_setwaterwalk                  = false;
    m_areaSpiritHealer_guid         = 0;
    m_CurrentTaxiPath               = NULL;
    m_fallDisabledUntil             = 0;
    m_lfgMatch                      = NULL;
    m_lfgInviterGuid                = 0;
    m_mountCheckTimer               = 0;
    m_taxiMapChangeNode             = 0;
    m_startMoveTime                 = 0;
    m_heartbeatDisable          = 0;
    m_safeFall                  = 0;
    safefall                    = false;
    z_axisposition              = 0.0f;
    m_KickDelay                 = 0;
    m_speedhackCheckTimer       = 0;
    m_mallCheckTimer            = 0;
    m_UpdateHookTimer           = 0;
    m_speedChangeInProgress     = false;
    m_passOnLoot                = false;
    m_changingMaps              = true;
    m_magnetAura                = NULL;
    m_lastMoveTime              = 0;
    m_lastMovementPacketTimestamp = 0;
    m_cheatEngineChances        = 2;
    m_mageInvisibility          = false;
    mAvengingWrath              = true;
    m_bgFlagIneligible          = 0;
    m_insigniaTaken             = true;
    m_BeastMaster               = false;

    m_QuestGOInProgress.clear();
    m_removequests.clear();
    m_finishedQuests.clear();
    m_finishedDailyQuests.clear();
    quest_spells.clear();
    quest_mobs.clear();
    m_loginAuras.clear();
    OnMeleeAuras.clear();
    m_Pets.clear();
    m_itemsets.clear();
    m_channels.clear();
    m_channelsbyDBCID.clear();
    m_visibleObjects.clear();
    mSpells.clear();

    watchedchannel          = NULL;
    PreventRes              = false;
    MobXPGainRate           = 0.0f;
    NoReagentCost           = false;
    fromrandombg            = false;
    randombgwinner          = false;
    m_drunkTimer            = 0;
    m_drunk                 = 0;
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

    if(!mTradeTarget.empty())
    {
        pTarget = GetTradeTarget();
        if(pTarget)
            pTarget->mTradeTarget = 0;
        pTarget = NULL;
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

    mTradeTarget = 0;

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

    if(m_ItemInterface)
    {
        delete m_ItemInterface;
        m_ItemInterface = NULL;
    }

    if(m_achievementInterface)
    {
        delete m_achievementInterface;
        m_achievementInterface = NULL;
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
    m_removequests.clear();
    m_finishedQuests.clear();
    quest_spells.clear();
    quest_mobs.clear();
    m_loginAuras.clear();
    OnMeleeAuras.clear();

    for(std::map<uint32, PlayerPet*>::iterator itr = m_Pets.begin(); itr != m_Pets.end(); itr++)
        delete itr->second;

    m_Pets.clear();
    m_itemsets.clear();
    m_channels.clear();
    m_channelsbyDBCID.clear();
    mSpells.clear();
    Unit::Destruct();
}

HEARTHSTONE_INLINE uint32 GetSpellForLanguageID(uint32 LanguageID)
{
    switch(LanguageID)
    {
    case LANG_COMMON:
        return 668;
        break;
    case LANG_ORCISH:
        return 669;
        break;
    case LANG_TAURAHE:
        return 670;
        break;
    case LANG_DARNASSIAN:
        return 671;
        break;
    case LANG_DWARVISH:
        return 672;
        break;
    case LANG_THALASSIAN:
        return 813;
        break;
    case LANG_DRACONIC:
        return 814;
        break;
    case LANG_DEMONIC:
        return 815;
        break;
    case LANG_TITAN:
        return 816;
        break;
    case LANG_GNOMISH:
        return 7430;
        break;
    case LANG_TROLL:
        return 7341;
        break;
    case LANG_GUTTERSPEAK:
        return 17737;
        break;
    case LANG_DRAENEI:
        return 29932;
        break;
    }

    return 0;
}

HEARTHSTONE_INLINE uint32 GetSpellForLanguageSkill(uint32 SkillID)
{
    switch(SkillID)
    {
    case SKILL_LANG_COMMON:
        return 668;
        break;

    case SKILL_LANG_ORCISH:
        return 669;
        break;

    case SKILL_LANG_TAURAHE:
        return 670;
        break;

    case SKILL_LANG_DARNASSIAN:
        return 671;
        break;

    case SKILL_LANG_DWARVEN:
        return 672;
        break;

    case SKILL_LANG_THALASSIAN:
        return 813;
        break;

    case SKILL_LANG_DRACONIC:
        return 814;
        break;

    case SKILL_LANG_DEMON_TONGUE:
        return 815;
        break;

    case SKILL_LANG_TITAN:
        return 816;
        break;

    case SKILL_LANG_OLD_TONGUE:
        return 817;
        break;

    case SKILL_LANG_GNOMISH:
        return 7430;
        break;

    case SKILL_LANG_TROLL:
        return 7341;
        break;

    case SKILL_LANG_GUTTERSPEAK:
        return 17737;
        break;

    case SKILL_LANG_DRAENEI:
        return 29932;
        break;
    }

    return 0;
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

    info = objmgr.GetPlayerCreateInfo(race, class_);
    if(!info)
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
    UpdateFieldValues();

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
    if( race != RACE_BLOODELF )
    {
        SetUInt32Value(UNIT_FIELD_DISPLAYID, info->displayId + gender );
        SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, info->displayId + gender );
    }
    else
    {
        SetUInt32Value(UNIT_FIELD_DISPLAYID, info->displayId - gender );
        SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, info->displayId - gender );
    }
    EventModelChange();

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

        if(se->type != SKILL_TYPE_LANGUAGE)
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

    sHookInterface.OnCharacterCreate(castPtr<Player>(this));

    load_health = GetUInt32Value(UNIT_FIELD_HEALTH);
    for(uint8 i = 0; i < 5; i++)
        load_power[i] = GetPower(i);
    return true;
}

void Player::EquipInit(PlayerCreateInfo *EquipInfo)
{
    ItemPrototype* proto = NULL;
    for(std::list<CreateInfo_ItemStruct>::iterator is = EquipInfo->items.begin(); is!=EquipInfo->items.end(); is++)
    {
        if ( (*is).protoid != 0)
        {
            proto = ItemPrototypeStorage.LookupEntry((*is).protoid);
            if(proto != NULL)
            {
                Item* item = objmgr.CreateItem((*is).protoid,castPtr<Player>(this));
                if(item != NULL)
                {
                    item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, (*is).amount);
                    if((*is).slot < INVENTORY_SLOT_BAG_END)
                    {
                        if( !GetItemInterface()->SafeAddItem(item, INVENTORY_SLOT_NOT_SET, (*is).slot) )
                        {
                            item->Destruct();
                            item = NULL;
                        }
                    }
                    else
                    {
                        if( !GetItemInterface()->AddItemToFreeSlot(item) )
                        {
                            item->Destruct();
                            item = NULL;
                        }
                    }
                }
            }
        }
    }
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

    uint32 mstime = getMSTime();
    if(mstime >= m_UpdateHookTimer)
    {
        sHookInterface.OnPlayerUpdate(this);
        m_UpdateHookTimer = mstime+1000;
    }

    if(m_attacking)
    {
        // Check attack timer.
        if(mstime >= m_attackTimer)
            _EventAttack(false);

        if( m_duelWield && mstime >= m_attackTimer_1 )
            _EventAttack(true);
    }

    if( m_onAutoShot )
    {
        if( m_AutoShotAttackTimer > p_time )
            m_AutoShotAttackTimer -= p_time;
        else EventRepeatSpell();
    }
    else if(m_AutoShotAttackTimer > 0)
    {
        if(m_AutoShotAttackTimer > p_time)
            m_AutoShotAttackTimer -= p_time;
        else m_AutoShotAttackTimer = 0;
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

    if(m_CurrentTransporter && !GetMovementInfo()->GetTransportLock())
    {
        // Update our position, using trnasporter X/Y
        float c_tposx, c_tposy, c_tposz, c_tposo;
        GetMovementInfo()->GetTransportPosition(c_tposx, c_tposy, c_tposz, c_tposo);
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

    // Exploration
    if(IsInWorld())
    {
        if(mstime >= m_explorationTimer)
        {
            _EventExploration();
            m_explorationTimer = mstime + 1500;
        }
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

    if (IsInWorld() && GetMapMgr()->CanUseCollision(this))
    {
        if( mstime >= m_mountCheckTimer )
        {
            if( sVMapInterface.IsIndoor( m_mapId, m_position.x, m_position.y, m_position.z ) )
            {
                //Mount expired?
                if(IsMounted())
                {
                    // Qiraj battletanks work everywhere on map 531
                    if (! (m_mapId == 531 && ( m_MountSpellId == 25953 || m_MountSpellId == 26054 || m_MountSpellId == 26055 || m_MountSpellId == 26056 )) )
                        Dismount();
                }

                // Now remove all auras that are only usable outdoors (e.g. Travel form)
                m_AuraInterface.RemoveAllAurasWithAttributes(0, ATTRIBUTES_ONLY_OUTDOORS);
            }
            m_mountCheckTimer = mstime + COLLISION_MOUNT_CHECK_INTERVAL;
        }
    }

    if( mstime >= m_speedhackCheckTimer )
    {
        _SpeedhackCheck();
        m_speedhackCheckTimer = mstime + 1000;
    }

    if (m_drunk)
    {
        m_drunkTimer += p_time;

        if (m_drunkTimer > 10*1000)
            EventHandleSobering();
    }

    if(mstime >= m_mallCheckTimer)
    {
        if( sWorld.FunServerMall != -1 && GetAreaId() == uint32(sWorld.FunServerMall) )
            if( IsPvPFlagged() )
                RemovePvPFlag();
        m_mallCheckTimer = mstime + 2000;
    }
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
            Item *item = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+(combatRating-20));
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
        if(Item *item = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+weaponType))
            speed = item->GetProto()->Delay;
    }
    return speed;
}

int32 Player::GetBaseMinDamage(uint8 weaponType)
{
    int32 damage = 1.0f;
    if( Item *item = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+weaponType) )
        if(disarmed == false) damage += item->CalcMinDamage();
    return damage;
}

int32 Player::GetBaseMaxDamage(uint8 weaponType)
{
    int32 damage = 2.0f;
    if( Item *item = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+weaponType) )
        if(disarmed == false) damage += item->CalcMaxDamage();
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
        UpdateStats();
        GetAchievementInterface()->HandleAchievementCriteriaLevelUp( level );
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
    if (!m_UnderwaterState)
        return;

    // In water
    if (m_UnderwaterState & UNDERWATERSTATE_UNDERWATER && !bInvincible && isAlive() && !m_bUnlimitedBreath)
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
            }
            else if (!(m_LastUnderwaterState & UNDERWATERSTATE_UNDERWATER)) // Update time in client if need
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
        if (!bInvincible && m_UnderwaterState & UNDERWATERSTATE_FATIGUE)
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
                    }
                    else if (HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE))    // Teleport ghost to graveyard
                        RepopAtGraveyard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
                }
                else if (!(m_LastUnderwaterState & UNDERWATERSTATE_FATIGUE))
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

    if (!bInvincible && m_UnderwaterState & (UNDERWATERSTATE_LAVA|UNDERWATERSTATE_SLIME))
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
                if (m_UnderwaterState & UNDERWATERSTATE_LAVA)
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
    }
    else
        m_MirrorTimer[FIRE_TIMER] = -1;

    // Recheck timers flag
    m_UnderwaterState &= ~UNDERWATERSTATE_TIMERS_PRESENT;
    for (uint8 i = 0; i< 3; ++i)
    {
        if (m_MirrorTimer[i] != -1)
        {
            m_UnderwaterState |= UNDERWATERSTATE_TIMERS_PRESENT;
            break;
        }
    }
    m_LastUnderwaterState = m_UnderwaterState;
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

    SetPlayerSpeed(RUN, m_runSpeed);

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

    ResetHeartbeatCoords();
}

void Player::_EventAttack( bool offhand )
{
    if (m_currentSpell)
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
    }
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
        if (!m_CurrentCharm->canReachWithAttack(pVictim))
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
                Spell* spell = new Spell(m_CurrentCharm,spellInfo,true,NULL);
                SpellCastTargets targets;
                targets.m_unitTarget = GetSelection();
                spell->extra_cast_number = cn;
                spell->prepare(&targets);
            }
        }
    }
}

void Player::EventAttackStart()
{
    m_attacking = true;
    if( IsMounted() )
    {
        Dismount();
        SetPlayerSpeed(RUN, m_runSpeed);
    }
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

        sHookInterface.OnZone(castPtr<Player>(this), m_zoneId, m_oldZone);
        CALL_INSTANCE_SCRIPT_EVENT( GetMapMgr(), OnZoneChange )( castPtr<Player>(this), m_zoneId, m_oldZone );

        EventDBCChatUpdate(0xFFFFFFFF);

        GetMapMgr()->GetStateManager().SendWorldStates(this);
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

        if(m_currentSpell)
        {
            Unit* target = m_currentSpell->GetUnitTarget();
            if(target && !sFactionSystem.isAttackable(this, target) && target != castPtr<Player>(this))
                m_currentSpell->cancel();
        }
    }

    sHookInterface.OnPlayerChangeArea(this, m_zoneId, m_areaId, m_oldArea);
    CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnChangeArea )( this, m_zoneId, m_areaId, m_oldArea );

    // bur: we dont want to explore new areas when on taxi
    if(!GetTaxiState() && !GetTransportGuid())
    {
        uint32 offset = at->explorationFlag / 32;
        if(offset < 144)
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
        GetAchievementInterface()->HandleAchievementCriteriaExploreArea( at->AreaId, GetUInt32Value(offset) );
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
    bool levelup = false;

    while(newxp >= nextlevelxp && newxp > 0)
    {
        ++level;
        newxp -= nextlevelxp;
        nextlevelxp = sWorld.GetXPToNextLevel(level);
        levelup = true;

        if(level >= GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
            break;
    }

    if(level > GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
        level = GetUInt32Value(PLAYER_FIELD_MAX_LEVEL);

    if(levelup)
    {
        m_playedtime[0] = 0; //Reset the "Current level played time"

        setLevel(level);

        // ScriptMgr hook for OnPostLevelUp
        sHookInterface.OnPostLevelUp(this);
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
    data << uint16( 0 );        // placeholder

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

        data << uint32( itr2->first );                      // spell id
        data << uint16( itr2->second.ItemId );              // item id
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
        data << uint16( itr2->second.ItemId );              // item id
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

void Player::_SavePetSpells(QueryBuffer * buf)
{
    m_lock.Acquire();
    // Remove any existing
    if(buf == NULL)
        CharacterDatabase.Execute("DELETE FROM playersummonspells WHERE ownerguid=%u", GetLowGUID());
    else
        buf->AddQuery("DELETE FROM playersummonspells WHERE ownerguid=%u", GetLowGUID());

    if(SummonSpells.size())
    {
        // Save summon spells
        std::map<uint32, std::set<uint32> >::iterator itr = SummonSpells.begin();
        std::stringstream ss;
        ss << "INSERT INTO playersummonspells VALUES ";
        bool first = true;
        for(; itr != SummonSpells.end(); itr++)
        {
            std::set<uint32>::iterator it = itr->second.begin();
            for(; it != itr->second.end(); it++)
            {
                if(first)
                    first = false;
                else
                    ss << ",";

                ss << "(" << GetLowGUID() << ", " << itr->first << ", " << (*it) << ")";
            }
        }
        ss << ";";

        if(buf == NULL)
            CharacterDatabase.Execute(ss.str().c_str());
        else // Execute or add our bulk inserts
            buf->AddQuery(ss.str().c_str());
    }

    m_lock.Release();
}

void Player::AddSummonSpell(uint32 Entry, uint32 SpellID)
{
    SpellEntry * sp = dbcSpell.LookupEntry(SpellID);
    std::map<uint32, std::set<uint32> >::iterator itr = SummonSpells.find(Entry);
    if(itr == SummonSpells.end())
        SummonSpells[Entry].insert(SpellID);
    else
    {
        std::set<uint32>::iterator it3;
        for(std::set<uint32>::iterator it2 = itr->second.begin(); it2 != itr->second.end();)
        {
            it3 = it2++;
            if(dbcSpell.LookupEntry(*it3)->NameHash == sp->NameHash)
                itr->second.erase(it3);
        }
        itr->second.insert(SpellID);
    }
}

void Player::RemoveSummonSpell(uint32 Entry, uint32 SpellID)
{
    std::map<uint32, std::set<uint32> >::iterator itr = SummonSpells.find(Entry);
    if(itr != SummonSpells.end())
    {
        itr->second.erase(SpellID);
        if(itr->second.size() == 0)
            SummonSpells.erase(itr);
    }
}

std::set<uint32>* Player::GetSummonSpells(uint32 Entry)
{
    std::map<uint32, std::set<uint32> >::iterator itr = SummonSpells.find(Entry);
    if(itr != SummonSpells.end())
    {
        return &(itr->second);
    }
    return 0;
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

void Player::_LoadPetSpells(QueryResult * result)
{
    //std::stringstream query;
    //std::map<uint32, std::list<uint32>* >::iterator itr;
    uint32 entry = 0;
    uint32 spell = 0;

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            entry = fields[1].GetUInt32();
            spell = fields[2].GetUInt32();
            AddSummonSpell(entry, spell);
        }
        while( result->NextRow() );
    }
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
    SkillLineSpell * sk = objmgr.GetSpellSkill(spell_id);
    if(sk && !_HasSkillLine(sk->skilline))
    {
        SkillLineEntry * skill = dbcSkillLine.LookupEntry(sk->skilline);
        uint32 max = 1;
        switch(skill->type)
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

        if(sWorld.StartLevel > 1 && skill->type != SKILL_TYPE_PROFESSION && skill->type != SKILL_TYPE_SECONDARY)
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

//===================================================================================================================
//  Set Create Player Bits -- Sets bits required for creating a player in the updateMask.
//  Note:  Doesn't set Quest or Inventory bits
//  updateMask - the updatemask to hold the set bits
//===================================================================================================================
void Player::_SetCreateBits(UpdateMask *updateMask, Player* target) const
{
    if(target == this)
    {
        WorldObject::_SetCreateBits(updateMask, target);
    }
    else
    {
        for(uint32 index = 0; index < m_valuesCount; index++)
        {
            if(m_uint32.values[index] != 0 && Player::m_visibleUpdateMask.GetBit(index))
                updateMask->SetBit(index);
        }
    }
}


void Player::_SetUpdateBits(UpdateMask *updateMask, Player* target) const
{
    if(target == this)
    {
        WorldObject::_SetUpdateBits(updateMask, target);
    }
    else
    {
        WorldObject::_SetUpdateBits(updateMask, target);
        *updateMask &= Player::m_visibleUpdateMask;
    }
}

void Player::InitVisibleUpdateBits()
{
    Player::m_visibleUpdateMask.SetCount(PLAYER_END);
    Player::m_visibleUpdateMask.SetBit(OBJECT_FIELD_GUID);
    Player::m_visibleUpdateMask.SetBit(OBJECT_FIELD_TYPE);
    Player::m_visibleUpdateMask.SetBit(OBJECT_FIELD_DATA);
    Player::m_visibleUpdateMask.SetBit(OBJECT_FIELD_DATA+1);
    Player::m_visibleUpdateMask.SetBit(OBJECT_FIELD_SCALE_X);

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_CHARM);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_CHARM+1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_SUMMON);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_SUMMON+1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_CHARMEDBY);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_CHARMEDBY+1);

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_TARGET);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_TARGET+1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_CHANNEL_OBJECT);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_CHANNEL_OBJECT+1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BYTES_0);

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_HEALTH);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXHEALTH);
    for(uint8 i = 0; i < 5; i++)
    {
        Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWERS+i);
        Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWERS+i);
    }

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_LEVEL);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_FACTIONTEMPLATE);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BYTES_0);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_FLAGS);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_FLAGS_2);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_AURASTATE);

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BASEATTACKTIME);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BASEATTACKTIME+1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BOUNDINGRADIUS);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_COMBATREACH);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_DISPLAYID);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_NATIVEDISPLAYID);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MOUNTDISPLAYID);
    Player::m_visibleUpdateMask.SetBit(UNIT_NPC_FLAGS);
    Player::m_visibleUpdateMask.SetBit(UNIT_DYNAMIC_FLAGS);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_PETNUMBER);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_PET_NAME_TIMESTAMP);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BYTES_1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BYTES_2);
    Player::m_visibleUpdateMask.SetBit(UNIT_CHANNEL_SPELL);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_CHANNEL_OBJECT);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_CHANNEL_OBJECT+1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_BASE_MANA);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_HOVERHEIGHT);

    Player::m_visibleUpdateMask.SetBit(PLAYER_FLAGS);
    Player::m_visibleUpdateMask.SetBit(PLAYER_BYTES);
    Player::m_visibleUpdateMask.SetBit(PLAYER_BYTES_2);
    Player::m_visibleUpdateMask.SetBit(PLAYER_BYTES_3);
    Player::m_visibleUpdateMask.SetBit(PLAYER_GUILDRANK);
    Player::m_visibleUpdateMask.SetBit(PLAYER_GUILDDELETE_DATE);
    Player::m_visibleUpdateMask.SetBit(PLAYER_GUILDLEVEL);
    Player::m_visibleUpdateMask.SetBit(PLAYER_GUILD_TIMESTAMP);
    Player::m_visibleUpdateMask.SetBit(PLAYER_DUEL_TEAM);
    Player::m_visibleUpdateMask.SetBit(PLAYER_DUEL_ARBITER);
    Player::m_visibleUpdateMask.SetBit(PLAYER_DUEL_ARBITER+1);

    for(uint32 i = PLAYER_QUEST_LOG; i < PLAYER_QUEST_LOG_END; i += 5)
        Player::m_visibleUpdateMask.SetBit(i);

    for(uint8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        uint32 offset = i * PLAYER_VISIBLE_ITEM_LENGTH;

        // item entry
        Player::m_visibleUpdateMask.SetBit(PLAYER_VISIBLE_ITEM + offset);
        // enchant
        Player::m_visibleUpdateMask.SetBit(PLAYER_VISIBLE_ITEM+1 + offset);
    }

    Player::m_visibleUpdateMask.SetBit(PLAYER_CHOSEN_TITLE);
}


void Player::DestroyForPlayer( Player* target, bool anim )
{
    WorldObject::DestroyForPlayer( target, anim );
    if(GetItemInterface())
        GetItemInterface()->m_DestroyForPlayer(target);
}

#define IS_ARENA(x) ( (x) >= BATTLEGROUND_ARENA_2V2 && (x) <= BATTLEGROUND_ARENA_5V5 )

void Player::SaveToDB(bool bNewCharacter /* =false */)
{
    bool in_arena = false;
    QueryBuffer * buf = ((!sWorld.DisableBufferSaving || bNewCharacter) ? new QueryBuffer() : NULL);
    if( m_bg != NULL && IS_ARENA( m_bg->GetType() ) )
        in_arena = true;

    //Calc played times
    uint32 playedt = (uint32)UNIXTIME - m_playedtime[2];
    m_playedtime[0] += playedt;
    m_playedtime[1] += playedt;
    m_playedtime[2] += playedt;

    std::stringstream ss;
    ss << "REPLACE INTO characters VALUES ("

    << GetLowGUID() << ", "
    << GetSession()->GetAccountId() << ","

    // stat saving
    << "'" << m_name << "', "
    << uint32(getRace()) << ","
    << uint32(getClass()) << ","
    << uint32(getGender()) << ","
    << uint32(customizable ? 1 : 0) << ",";

    if(GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) != info->factiontemplate)
        ss << GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) << ",";
    else
        ss << "0,";

    ss << uint32(getLevel()) << ","
    << uint32(m_XPoff ? 1 : 0) << ","
    << GetUInt32Value(PLAYER_XP) << ","

    // dump exploration data
    << "'";

    for(uint32 i = 0; i < 144; i++)
        ss << GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + i) << ",";

    ss << "','0', "; //skip saving oldstyle skills, just fill with 0

    uint32 player_flags = GetUInt32Value(PLAYER_FLAGS);
    {
        // Remove un-needed and problematic player flags from being saved :p
        if(player_flags & PLAYER_FLAG_PARTY_LEADER)
            player_flags &= ~PLAYER_FLAG_PARTY_LEADER;
        if(player_flags & PLAYER_FLAG_AFK)
            player_flags &= ~PLAYER_FLAG_AFK;
        if(player_flags & PLAYER_FLAG_DND)
            player_flags &= ~PLAYER_FLAG_DND;
        if(player_flags & PLAYER_FLAG_GM)
            player_flags &= ~PLAYER_FLAG_GM;
        if(player_flags & PLAYER_FLAG_PVP_TOGGLE)
            player_flags &= ~PLAYER_FLAG_PVP_TOGGLE;
        if(player_flags & PLAYER_FLAG_FREE_FOR_ALL_PVP)
            player_flags &= ~PLAYER_FLAG_FREE_FOR_ALL_PVP;
        if(player_flags & PLAYER_FLAG_PVP_TIMER)
            player_flags &= ~PLAYER_FLAG_PVP_TIMER;
        if(player_flags & PLAYER_FLAG_UNKNOWN2)
            player_flags &= ~PLAYER_FLAG_UNKNOWN2;
        if(player_flags & PLAYER_FLAG_DEVELOPER)
            player_flags &= ~PLAYER_FLAG_DEVELOPER;
        if(player_flags & PLAYER_FLAG_ALLOW_ONLY_ABILITY)
            player_flags &= ~PLAYER_FLAG_ALLOW_ONLY_ABILITY;
    }

    ss << GetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX) << ","
    << GetUInt32Value(PLAYER_CHOSEN_TITLE) << ","
    << GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES) << ","
    << GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES1) << ","
    << GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES2) << ","
    << GetUInt64Value(PLAYER_FIELD_COINAGE) << ","
    << GetUInt32Value(PLAYER_CHARACTER_POINTS) << ","
    << load_health << ","
    << load_power[0] << ","
    << load_power[1] << ","
    << load_power[2] << ","
    << load_power[3] << ","
    << load_power[4] << ","
    << uint32(GetPVPRank()) << ","
    << GetUInt32Value(PLAYER_BYTES) << ","
    << GetUInt32Value(PLAYER_BYTES_2) << ","
    << player_flags << ","
    << GetUInt32Value(PLAYER_FIELD_BYTES) << ",";

    float posx = 0.0f;
    float posy = 0.0f;
    float posz = 0.0f;
    float orientation = 0.0f;
    uint32 map = m_mapId;
    uint32 instanceid = m_instanceId;
    if( in_arena )
    {
        // if its an arena, save the entry coords instead
        posx = m_bgEntryPointX;
        posy = m_bgEntryPointY;
        posz = m_bgEntryPointZ;
        orientation = m_bgEntryPointO;

        map = m_bgEntryPointMap;
        instanceid = m_bgEntryPointInstance;
    }
    else
    {
        // save the normal position
        posx = m_position.x;
        posy = m_position.y;
        posz = m_position.z;
        orientation = m_position.o;
    }

    if(m_playerInfo)
    {
        m_playerInfo->curInstanceID = m_instanceId;
        m_playerInfo->lastmapid = map;
        m_playerInfo->lastpositionx = posx;
        m_playerInfo->lastpositiony = posy;
        m_playerInfo->lastpositionz = posz;
        m_playerInfo->lastorientation = orientation;
    }

    ss << posx << ", "
        << posy << ", "
        << posz << ", "
        << orientation << ", "
        << map << ", ";

    ss << m_zoneId << ", '";

    for(uint8 i = 0; i < MAX_TAXI; i++ )
        ss << m_taximask[i] << " ";
    ss << "', "

    << m_banned << ", '"
    << CharacterDatabase.EscapeString(m_banreason) << "', "
    << (uint32)UNIXTIME << ",";

    //online state
    if(GetSession()->_loggingOut || bNewCharacter)
    {
        ss << "0,";
    }
    else
    {
        ss << "1,";
    }

    ss
    << m_bind_pos_x          << ", "
    << m_bind_pos_y          << ", "
    << m_bind_pos_z          << ", "
    << m_bind_mapid          << ", "
    << m_bind_zoneid            << ", "

    << uint32(m_isResting)    << ", "
    << uint32(m_restState)    << ", "
    << uint32(m_restAmount)  << ", '"

    << uint32(m_playedtime[0])  << " "
    << uint32(m_playedtime[1])  << " "
    << uint32(playedt)        << " ', "
    << uint32(m_deathState)  << ", "

    << m_FirstLogin          << ", "
    << rename_pending       << ","
    << m_arenaPoints         << ","
    << (uint32)m_StableSlotCount << ",";

    // instances
    ss << instanceid        << ", "
    << m_bgEntryPointMap    << ", "
    << m_bgEntryPointX      << ", "
    << m_bgEntryPointY      << ", "
    << m_bgEntryPointZ      << ", "
    << m_bgEntryPointO      << ", "
    << m_bgEntryPointInstance << ", ";

    // taxi
    if(m_onTaxi && m_CurrentTaxiPath) {
        ss << m_CurrentTaxiPath->GetID() << ", ";
        ss << lastNode << ", ";
        ss << GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID);
    }
    else
        ss << "0, 0, 0";

    float transx, transy, transz, transo;
    GetMovementInfo()->GetTransportPosition(transx, transy, transz, transo);
    ss << "," << (m_CurrentTransporter && GetVehicle() == NULL ? m_CurrentTransporter->GetLowGUID() : uint32(0));
    ss << ",'" << transx << "','" << transy << "','" << transz << "'";
    ss << ",'";

    // Dump reputation data
    ReputationMap::iterator iter = m_reputation.begin();
    for(; iter != m_reputation.end(); iter++)
        ss << int32(iter->first) << "," << int32(iter->second->flag) << "," << int32(iter->second->baseStanding) << "," << int32(iter->second->standing) << ",";
    ss << "','";

    m_talentInterface.SaveActionButtonData(ss);
    ss << "','";

    if(!bNewCharacter)
        SaveAuras(ss);

    //ss << LoadAuras;
    ss << "','";

    // Add player finished quests
    std::set<uint32>::iterator fq = m_finishedQuests.begin();
    for(; fq != m_finishedQuests.end(); ++fq)
        ss << (*fq) << ",";

    ss << "','";
    // Add player finished daily quests
    fq = m_finishedDailyQuests.begin();
    for(; fq != m_finishedDailyQuests.end(); ++fq)
        ss << (*fq) << ",";

    ss << "', ";
    ss << m_honorRolloverTime << ", ";
    ss << m_killsToday << ", " << m_killsYesterday << ", " << m_killsLifetime << ", ";
    ss << m_honorToday << ", " << m_honorYesterday << ", ";
    ss << m_honorPoints << ", ";
    ss << iInstanceType << ", ";
    ss << iRaidType << ", ";

    // Talents
    m_talentInterface.SaveTalentData(buf, ss);

    ss << "0, 0)";  // Reset for talents and position

    if(buf)
        buf->AddQueryStr(ss.str());
    else CharacterDatabase.WaitExecuteNA(ss.str().c_str());

    //Save Other related player stuff
    sHookInterface.OnPlayerSaveToDB(this, buf);

    // Skills
    _SaveSkillsToDB(buf);

    // Spells
    _SaveSpellsToDB(buf);

    // Equipment Sets
    _SaveEquipmentSets(buf);

    // Glyphs
    m_talentInterface.SaveGlyphData(buf);

    // Inventory
    GetItemInterface()->mSaveItemsToDatabase(bNewCharacter, buf);

    // save quest progress
    _SaveQuestLogEntry(buf);

    // GM Ticket
    if(GM_Ticket* ticket = sTicketMgr.GetGMTicketByPlayer(GetGUID()))
        sTicketMgr.SaveGMTicket(ticket, buf);

    // Cooldown Items
    _SavePlayerCooldowns( buf );

    // Pets
    if(getClass() == HUNTER || getClass() == WARLOCK)
    {
        _SavePet(buf);
        _SavePetSpells(buf);
    }

    // Achievements
    m_achievementInterface->SaveToDB( buf );

    ForceSaved = false;
    m_nextSave = 120000;
    if(buf)
        CharacterDatabase.AddQueryBuffer(buf);
}

void Player::_SaveSkillsToDB(QueryBuffer * buf)
{
    // if we have nothing to save why save?
    if (m_skills.size() == 0)
        return;
    m_lock.Acquire();

    if(buf == NULL)
        CharacterDatabase.Execute("DELETE FROM playerskills WHERE Player_Guid = %u", GetLowGUID() );
    else
        buf->AddQuery("DELETE FROM playerskills WHERE Player_Guid = %u", GetLowGUID() );

    std::stringstream ss;
    ss << "INSERT INTO playerskills (Player_Guid, skill_id, type, currentlvl, maxlvl ) VALUES ";
    uint32 iI = uint32(m_skills.size())-1;
    for(SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end() ; itr++)
    {
        if(itr->first)
        {
            ss  << "(" << GetLowGUID() << ","
                << itr->first << ","
                << itr->second.Skill->type << ","
                << itr->second.CurrentValue << ","
                << itr->second.MaximumValue << ")";
            if (iI)
                ss << ",";
        }
        iI -= 1;
    }

    if(buf == NULL)
        CharacterDatabase.Execute(ss.str().c_str());
    else
        buf->AddQueryStr(ss.str());

    m_lock.Release();
}

void Player::_LoadSpells(QueryResult * result)
{
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            SpellEntry * spProto = dbcSpell.LookupEntry(fields[0].GetInt32());
            if(spProto)
                mSpells.insert(spProto->Id);
        } while(result->NextRow());
    }
    else // Player without spells, create defaults.
    {
        Reset_Spells();
    }
}

void Player::_SaveSpellsToDB(QueryBuffer * buf)
{
    m_lock.Acquire();
    // delete old first
    if(buf == NULL)
        CharacterDatabase.Execute("DELETE FROM playerspells WHERE guid = %u", GetLowGUID() );
    else
        buf->AddQuery("DELETE FROM playerspells WHERE guid = %u", GetLowGUID() );

    // Dump spell data to stringstream
    std::stringstream ss;
    ss << "INSERT INTO playerspells VALUES ";
    SpellSet::iterator spellItr = mSpells.begin();
    bool first = true;
    for(; spellItr != mSpells.end(); ++spellItr)
    {
        SpellEntry * sp = dbcSpell.LookupEntry( *spellItr );
        if( !sp || sp->RequiredShapeShift == FORM_ZOMBIE )
            continue;

        if(!first)
            ss << ",";
        else
            first = false;

        ss << "("<< GetLowGUID() << "," << uint32(*spellItr) << ")";
    }
    if(buf == NULL)
        CharacterDatabase.Execute(ss.str().c_str());
    else
        buf->AddQueryStr(ss.str());
    m_lock.Release();
}

void Player::_SaveQuestLogEntry(QueryBuffer * buf)
{
    m_lock.Acquire();
    std::stringstream ss;
    bool first = true;
    if(m_removequests.size())
    {
        for(std::set<uint32>::iterator itr = m_removequests.begin(); itr != m_removequests.end(); itr++)
        {
            if(first)
                first = false;
            else
                ss << ",";
            ss << (*itr);
        }

        if(buf == NULL)
            CharacterDatabase.Execute("DELETE FROM questlog WHERE player_guid = %u AND quest_id IN (%s)", GetLowGUID(), ss.str().c_str());
        else
            buf->AddQuery("DELETE FROM questlog WHERE player_guid = %u AND quest_id IN (%s)", GetLowGUID(), ss.str().c_str());
    }
    m_removequests.clear();

    for(int i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if(m_questlog[i] != NULL)
            m_questlog[i]->SaveToDB(buf);
    }
    m_lock.Release();
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
                    if((item = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND)) != NULL)
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
            if(GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
            {
                if((int32)GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)->GetProto()->Class == m_spellInfo->EquippedItemClass)
                {
                    if (m_spellInfo->EquippedItemSubClass != 0)
                    {
                        if (m_spellInfo->EquippedItemSubClass != 173555 && m_spellInfo->EquippedItemSubClass != 96 && m_spellInfo->EquippedItemSubClass != 262156)
                        {
                            if (!((1 << GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)->GetProto()->SubClass)
                                & m_spellInfo->EquippedItemSubClass))
                                return false;
                        }
                    }
                }
            }
            else if(m_spellInfo->EquippedItemSubClass == 173555)
                return false;

            if (GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED))
            {
                if((int32)GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)->GetProto()->Class == m_spellInfo->EquippedItemClass)
                {
                    if (m_spellInfo->EquippedItemSubClass != 0)
                    {
                        if (m_spellInfo->EquippedItemSubClass != 173555 && m_spellInfo->EquippedItemSubClass != 96 && m_spellInfo->EquippedItemSubClass != 262156)
                        {
                            if (!((1 << GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)->GetProto()->SubClass)
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

void Player::RemovePendingPlayer()
{
    if(m_session)
    {
        uint8 response = CHAR_LOGIN_NO_CHARACTER;
        m_session->OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &response);
        m_session->m_loggingInPlayer = NULL;
    }

    ok_to_remove = true;
    Destruct();
}

bool Player::LoadFromDB()
{
    AsyncQuery * q = new AsyncQuery( new SQLClassCallbackP0<Player>(castPtr<Player>(this), &Player::LoadFromDBProc) );
    q->AddQuery("SELECT * FROM characters WHERE guid=%u AND forced_rename_pending = 0", GetLowGUID());
    q->AddQuery("SELECT cooldown_type, cooldown_misc, cooldown_expire_time, cooldown_spellid, cooldown_itemid FROM playercooldowns WHERE player_guid=%u", GetLowGUID());
    q->AddQuery("SELECT * FROM questlog WHERE player_guid=%u", GetLowGUID());
    q->AddQuery("SELECT * FROM playeritems WHERE ownerguid=%u ORDER BY containerslot ASC", GetLowGUID());
    q->AddQuery("SELECT * FROM playerpets WHERE ownerguid=%u ORDER BY petnumber", GetLowGUID());
    q->AddQuery("SELECT * FROM playersummonspells where ownerguid=%u ORDER BY entryid", GetLowGUID());
    q->AddQuery("SELECT * FROM mailbox WHERE player_guid = %u", GetLowGUID());

    // social
    q->AddQuery("SELECT friend_guid, note FROM social_friends WHERE character_guid = %u", GetLowGUID());
    q->AddQuery("SELECT character_guid FROM social_friends WHERE friend_guid = %u", GetLowGUID());
    q->AddQuery("SELECT ignore_guid FROM social_ignores WHERE character_guid = %u", GetLowGUID());

    //Achievements
    q->AddQuery("SELECT * from achievements WHERE player = %u", GetLowGUID());

    //skills
    q->AddQuery("SELECT * FROM playerskills WHERE player_guid = %u AND type <> %u ORDER BY skill_id ASC, currentlvl DESC", GetLowGUID(), SKILL_TYPE_LANGUAGE ); //load skill, skip languages

    //pet action bar
    q->AddQuery("SELECT * FROM playerpetactionbar WHERE ownerguid=%u ORDER BY petnumber", GetLowGUID());

    //Talents
    q->AddQuery("SELECT spec, tid, rank FROM playertalents WHERE guid = %u", GetLowGUID());

    //Glyphs
    q->AddQuery("SELECT * FROM playerglyphs WHERE guid = %u", GetLowGUID());

    //Spells
    q->AddQuery("SELECT spellid FROM playerspells WHERE guid = %u", GetLowGUID());

    //Equipmentsets
    q->AddQuery("SELECT * FROM equipmentsets WHERE guid = %u", GetLowGUID());

    // queue it!
    CharacterDatabase.QueueAsyncQuery(q);
    return true;
}

void Player::LoadFromDBProc(QueryResultVector & results)
{
    uint32 field_index = 2;
#define get_next_field fields[field_index++]

    // set playerinfo
    m_playerInfo = objmgr.GetPlayerInfo(GetLowGUID());
    if( m_playerInfo == NULL )
    {
        RemovePendingPlayer();
        return;
    }

    if(GetSession() == NULL || results.size() < 8)      // should have 8 query results for a player load.
    {
        RemovePendingPlayer();
        return;
    }

    QueryResult *result = results[0].result;
    if(!result)
    {
        printf("Player login query failed., guid %u\n", GetLowGUID());
        RemovePendingPlayer();
        return;
    }

    Field *fields = result->Fetch();

    if(fields[1].GetUInt32() != m_session->GetAccountId())
    {
        sWorld.LogCheater(m_session, "player tried to load character not belonging to them (guid %u, on account %u)",
            fields[0].GetUInt32(), fields[1].GetUInt32());
        RemovePendingPlayer();
        return;
    }

    uint32 banned = fields[36].GetUInt32();
    if(banned && (banned < 100 || banned > (uint32)UNIXTIME))
    {
        RemovePendingPlayer();
        return;
    }

    // Load name
    m_name = get_next_field.GetString();

    // Load race/class from fields
    setRace(get_next_field.GetUInt8());
    setClass(get_next_field.GetUInt8());
    setGender(get_next_field.GetUInt8());
    customizable = get_next_field.GetBool();
    uint32 cfaction = get_next_field.GetUInt32();

    // set race dbc
    myRace = dbcCharRace.LookupEntry(getRace());
    myClass = dbcCharClass.LookupEntry(getClass());
    if( myClass == NULL || myRace == NULL )
    {
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
        sLog.Error("PlayerCreateInfo", "Character of guid %u creation failed due to non existant or invalid playercreateinfo.", uint(GetLowGUID()));
        RemovePendingPlayer();
        return;
    }
    assert(info);

    // set level
    uint8 plvl = get_next_field.GetUInt32();
    SetUInt32Value(UNIT_FIELD_LEVEL, plvl);

    m_talentInterface.InitGlyphSlots();
    m_talentInterface.InitGlyphsForLevel(plvl);

    // level dependant taxi node
    SetTaximaskNode( 213, plvl >= 68 ? false : true );  //Add 213 (Shattered Sun Staging Area) if lvl >=68

    // Set our base stats
    baseStats = sStatSystem.GetUnitBaseStats(getRace(), getClass(), plvl);

    m_XPoff = get_next_field.GetBool();
    // set xp
    SetUInt32Value(PLAYER_XP, get_next_field.GetUInt32());

    // Process exploration data.
    uint32 Counter = 0;
    char * end;
    char * start = (char*)get_next_field.GetString();//buff;
    while(Counter < 144)
    {
        end = strchr(start,',');
        if(!end)break;
        *end=0;
        SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + Counter, atol(start));
        start = end +1;
        Counter++;
    }

    if(QueryResult *checkskills = results[11].result)
    {
        _LoadSkills(checkskills); field_index++;
        sLog.Debug("WorldSession","Skills loaded");
    }
    else
    {
        // old format
        Counter = 0;
        start = (char*)get_next_field.GetString();//buff old system;

        if(!(start == ((char*)("0"))))
        {
            const ItemProf * prof;
            if(!strchr(start, ' ') && !strchr(start,';'))
            {
                // no skills - reset to defaults
                for(std::list<CreateInfo_SkillStruct>::iterator ss = info->skills.begin(); ss!=info->skills.end(); ss++)
                {
                    if(ss->skillid && ss->currentval && ss->maxval && !::GetSpellForLanguageSkill(ss->skillid))
                        _AddSkillLine(ss->skillid, ss->currentval, ss->maxval);
                }
            }
            else
            {
                char * f = strdup(start);
                start = f;

                uint32 v1,v2,v3;
                PlayerSkill sk;
                for(;;)
                {
                    end = strchr(start, ';');
                    if(!end)
                        break;

                    *end = 0;
                    v1 = atol(start);
                    start = end + 1;

                    end = strchr(start, ';');
                    if(!end)
                        break;

                    *end = 0;
                    v2 = atol(start);
                    start = end + 1;

                    end = strchr(start, ';');
                    if(!end)
                        break;

                    v3 = atol(start);
                    start = end + 1;

                    /* add the skill */
                    if(v1)
                    {
                        if(sk.Reset(v1))
                        {
                            sk.CurrentValue = v2;
                            sk.MaximumValue = v3;
                            if (v1 == SKILL_RIDING)
                                sk.CurrentValue = sk.MaximumValue;
                            m_skills.insert(std::make_pair(v1, sk));

                            prof = GetProficiencyBySkill(v1);
                            if(prof)
                            {
                                if(prof->itemclass==4)
                                    armor_proficiency|=prof->subclass;
                                else
                                    weapon_proficiency|=prof->subclass;
                            }
                        }
                    }
                }
                free(f);
                _UpdateMaxSkillCounts();
                sLog.Debug("Player","loaded old style skills for player %s", m_name.c_str());
            }
        }
        else
        {   // Reset player's skills.
            _RemoveAllSkills();

            // Load skills from create info.
            PlayerCreateInfo * info = objmgr.GetPlayerCreateInfo(getRace(), getClass());
            if(info)
            {
                SkillLineEntry* se = NULL;
                for(std::list<CreateInfo_SkillStruct>::iterator ss = info->skills.begin(); ss!=info->skills.end(); ss++)
                {
                    se = dbcSkillLine.LookupEntry(ss->skillid);
                    if(se != NULL && se->type != SKILL_TYPE_LANGUAGE && ss->skillid && ss->currentval && ss->maxval)
                        _AddSkillLine(ss->skillid, ss->currentval, ss->maxval);

                    se = NULL;
                }
            }

            //Chances depend on stats must be in this order!
            UpdateStats();
            _UpdateMaxSkillCounts();
            _AddLanguages(sWorld.cross_faction_world);
        }
    }

    // set the rest of the shit
    SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, get_next_field.GetUInt32());
    SetUInt32Value(PLAYER_CHOSEN_TITLE, get_next_field.GetUInt32());
    SetUInt64Value( PLAYER__FIELD_KNOWN_TITLES, get_next_field.GetUInt64() );
    SetUInt64Value( PLAYER__FIELD_KNOWN_TITLES1, get_next_field.GetUInt64() );
    SetUInt64Value( PLAYER__FIELD_KNOWN_TITLES2, get_next_field.GetUInt64() );
    SetUInt64Value(PLAYER_FIELD_COINAGE, get_next_field.GetUInt64());
    SetUInt32Value(PLAYER_CHARACTER_POINTS, get_next_field.GetUInt32());

    load_health = get_next_field.GetUInt32();
    for(uint8 i = 0; i < 5; i++)
        load_power[i] = get_next_field.GetUInt32();
    uint8 pvprank = get_next_field.GetUInt8();
    SetUInt32Value( PLAYER_BYTES, get_next_field.GetUInt32() );
    SetUInt32Value( PLAYER_BYTES_2, get_next_field.GetUInt32() );
    SetUInt32Value( PLAYER_BYTES_3, getGender());
    SetUInt32Value( PLAYER_FLAGS, get_next_field.GetUInt32() );
    SetUInt32Value( PLAYER_FIELD_BYTES, get_next_field.GetUInt32() );

    // Set our position
    m_position.x                                        = get_next_field.GetFloat();
    m_position.y                                        = get_next_field.GetFloat();
    m_position.z                                        = get_next_field.GetFloat();
    m_position.o                                        = get_next_field.GetFloat();
    movement_info.SetPosition(m_position);

    m_mapId                                             = get_next_field.GetUInt32();
    m_zoneId                                            = get_next_field.GetUInt32();

    // Calculate the base stats now they're all loaded
    for(uint32 x = 0; x < 7; x++)
        SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+x, 1.f);

    // Normal processing...
//  UpdateMaxSkills();
    UpdateStats();
    //UpdateChances();

    // Initialize 'normal' fields
    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    if(getClass() == WARRIOR && !HasAura(21156) && !HasAura(7376) && !HasAura(7381))
        CastSpell(this, 2457, true); // We have no shapeshift aura, set our shapeshift.

    // We're players!
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0.388999998569489f );
    SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f   );

    if( getRace() != RACE_BLOODELF )
    {
        SetUInt32Value(UNIT_FIELD_DISPLAYID, info->displayId + getGender() );
        SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, info->displayId + getGender() );
    }
    else
    {
        SetUInt32Value(UNIT_FIELD_DISPLAYID, info->displayId - getGender() );
        SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, info->displayId - getGender() );
    }
    EventModelChange();

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, info->factiontemplate);
    if(cfaction)
    {
        // re-calculate team
        switch(cfaction)
        {
        case 1: // human
        case 3: // dwarf
        case 4: // ne
        case 8: // gnome
        case 927:   // draenei
            m_team = m_bgTeam = 0;
            break;

        default:
            m_team = m_bgTeam = 1;
            break;
        }
    }

    LoadTaxiMask( get_next_field.GetString() );

    m_banned = get_next_field.GetUInt32(); //Character ban
    m_banreason = get_next_field.GetString();
    m_timeLogoff = get_next_field.GetUInt32();
    field_index++;

    m_bind_pos_x = get_next_field.GetFloat();
    m_bind_pos_y = get_next_field.GetFloat();
    m_bind_pos_z = get_next_field.GetFloat();
    m_bind_mapid = get_next_field.GetUInt32();
    m_bind_zoneid = get_next_field.GetUInt32();

    m_isResting = get_next_field.GetUInt8();
    m_restState = get_next_field.GetUInt8();
    m_restAmount = get_next_field.GetUInt32();


    std::string tmpStr = get_next_field.GetString();
    m_playedtime[0] = (uint32)atoi((const char*)strtok((char*)tmpStr.c_str()," "));
    m_playedtime[1] = (uint32)atoi((const char*)strtok(NULL," "));

    m_deathState = (DeathState)get_next_field.GetUInt32();

    if(load_health && m_deathState == JUST_DIED)
    {
        m_deathState = CORPSE;
        load_health = 0;
    }
    SetUInt32Value(UNIT_FIELD_HEALTH, load_health);
    for(uint8 i = 0; i < 5; i++)
        SetPower(UNIT_FIELD_POWERS+i, load_power[i]);

    m_FirstLogin = get_next_field.GetBool();
    rename_pending = get_next_field.GetBool();
    m_arenaPoints = get_next_field.GetUInt32();
    m_StableSlotCount = get_next_field.GetUInt32();
    m_instanceId = get_next_field.GetUInt32();
    m_bgEntryPointMap = get_next_field.GetUInt32();
    m_bgEntryPointX = get_next_field.GetFloat();
    m_bgEntryPointY = get_next_field.GetFloat();
    m_bgEntryPointZ = get_next_field.GetFloat();
    m_bgEntryPointO = get_next_field.GetFloat();
    m_bgEntryPointInstance = get_next_field.GetUInt32();

    uint32 taxipath = get_next_field.GetUInt32();
    TaxiPath *path = NULL;
    if(taxipath)
    {
        path = sTaxiMgr.GetTaxiPath(taxipath);
        lastNode = get_next_field.GetUInt32();
        if(path)
        {
            SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, get_next_field.GetUInt32());
            SetTaxiPath(path);
            m_onTaxi = true;
        }
        else
            ++field_index;
    }
    else
    {
        ++field_index;
        ++field_index;
    }

    uint32 transGuid = get_next_field.GetUInt32();
    GetMovementInfo()->SetTransportData(uint64(transGuid ? MAKE_NEW_GUID(transGuid, 0, HIGHGUID_TYPE_TRANSPORTER) : 0), get_next_field.GetFloat(), get_next_field.GetFloat(), get_next_field.GetFloat(), 0.0f, 0);

    // Load Reputatation CSV Data
    start =(char*) get_next_field.GetString();
    FactionEntry * faction;
    FactionReputation * rep;
    uint32 id;
    int32 basestanding;
    int32 standing;
    uint32 fflag;
    while(true)
    {
        end = strchr(start,',');
        if(!end)
            break;
        *end=0;
        id = atol(start);
        start = end +1;

        end = strchr(start,',');
        if(!end)break;
        *end=0;
        fflag = atol(start);
        start = end +1;

        end = strchr(start,',');
        if(!end)
            break;
        *end=0;
        basestanding = atoi(start);//atol(start);
        start = end +1;

        end = strchr(start,',');
        if(!end)
            break;
        *end=0;
        standing  = atoi(start);// atol(start);
        start = end +1;

        // listid stuff
        faction = dbcFaction.LookupEntry(id);
        if(!faction) continue;
        ReputationMap::iterator rtr = m_reputation.find(id);
        if(rtr != m_reputation.end())
            delete rtr->second;

        rep = new FactionReputation();
        rep->baseStanding = basestanding;
        rep->standing = standing;
        rep->flag = fflag;
        m_reputation[id]=rep;

        // do listid stuff
        if(faction->RepListId >= 0)
            reputationByListId[faction->RepListId] = rep;
    }

    if(!m_reputation.size())
        _InitialReputation();

    // Load saved actionbars
    m_talentInterface.LoadActionButtonData(fields, field_index);

    //LoadAuras = get_next_field.GetString();
    start = (char*)get_next_field.GetString();//buff;
    do
    {
        end = strchr(start,',');
        if(!end)
            break;
        *end=0;
        LoginAura *la = new LoginAura();
        la->id = atol(start);
        start = end +1;
        end = strchr(start,',');
        if(!end)
            break;
        *end=0;
        la->dur = atol(start);
        start = end +1;
        if(la->id == 43869 || la->id == 43958)
            delete la;
        else m_loginAuras.push_back(la);
    } while(true);

    // Load saved finished quests

    start =  (char*)get_next_field.GetString();
    while(true)
    {
        end = strchr(start,',');
        if(!end)
            break;
        *end=0;
        m_finishedQuests.insert(atol(start));
        start = end +1;
    }
    GetAchievementInterface()->HandleAchievementCriteriaQuestCount( uint32(m_finishedQuests.size()));

    DailyMutex.Acquire();
    start = (char*)get_next_field.GetString();
    while(true)
    {
        end = strchr(start,',');
        if(!end || m_finishedDailyQuests.size() == 25)
            break;
        *end=0;
        SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1 + uint32(m_finishedDailyQuests.size()), atol(start));
        m_finishedDailyQuests.insert(atol(start));
        start = end +1;
    }
    DailyMutex.Release();

    m_honorRolloverTime = get_next_field.GetUInt32();
    m_killsToday = get_next_field.GetUInt32();
    m_killsYesterday = get_next_field.GetUInt32();
    m_killsLifetime = get_next_field.GetUInt32();

    m_honorToday = get_next_field.GetUInt32();
    m_honorYesterday = get_next_field.GetUInt32();
    m_honorPoints = get_next_field.GetUInt32();

    RolloverHonor();
    iInstanceType = get_next_field.GetUInt32();
    iRaidType = get_next_field.GetUInt32();

    HonorHandler::RecalculateHonorFields(this);

    _setFaction();

    //class fixes
    switch(getClass())
    {
    case PALADIN:
        armor_proficiency |= ( 1 << 7 );//LIBRAM
        break;
    case DRUID:
        armor_proficiency |= ( 1 << 8 );//IDOL
        break;
    case SHAMAN:
        armor_proficiency |= ( 1 << 9 );//TOTEM
        break;
    case DEATHKNIGHT:
        armor_proficiency |= ( 1 << 10 );//SIGIL
        break;
    case WARLOCK:
    case HUNTER:
        _LoadPet(results[4].result);
        _LoadPetSpells(results[5].result);
        _LoadPetActionBar(results[12].result);
        break;
    }

    if(m_session->CanUseCommand('c'))
        _AddLanguages(true);
    else
        _AddLanguages(sWorld.cross_faction_world);

    OnlineTime  = (uint32)UNIXTIME;
    if(IsInGuild())
        SetUInt32Value(PLAYER_GUILD_TIMESTAMP, (uint32)UNIXTIME);

    m_talentInterface.LoadTalentData(results[13].result, fields, field_index);

    bool needTalentReset = get_next_field.GetBool();
    bool NeedsPositionReset = get_next_field.GetBool();

#undef get_next_field

    // load properties
    m_talentInterface.LoadGlyphData(results[14].result);
    _LoadSpells(results[15].result);
    _LoadEquipmentSets(results[16].result);
    _LoadPlayerCooldowns(results[1].result);
    m_ItemInterface->mLoadItemsFromDatabase(results[3].result);

    // Note: For quest finishing to load correctly, this has to be done after loading items!
    _LoadQuestLogEntry(results[2].result);
    m_mailBox->Load(results[6].result);

    // SOCIAL
    if( result = results[7].result )         // this query is "who are our friends?"
    {
        do
        {
            fields = result->Fetch();
            if( strlen( fields[1].GetString() ) )
                m_friends.insert( std::make_pair( fields[0].GetUInt32(), strdup(fields[1].GetString()) ) );
            else
                m_friends.insert( std::make_pair( fields[0].GetUInt32(), (char*)NULL) );

        } while (result->NextRow());
    }

    if( result = results[8].result )         // this query is "who has us in their friends?"
    {
        do
        {
            m_hasFriendList.insert( result->Fetch()[0].GetUInt32() );
        } while (result->NextRow());
    }

    if( result = results[9].result )        // this query is "who are we ignoring"
    {
        do
        {
            m_ignores.insert( result->Fetch()[0].GetUInt32() );
        } while (result->NextRow());
    }

    // END SOCIAL

    // Load achievements
    GetAchievementInterface()->LoadFromDB( results[10].result );

    if( needTalentReset )
        m_talentInterface.ResetAllSpecs();
    if( NeedsPositionReset )
        EjectFromInstance();

    // Set correct maximum level
    uint32 maxLevel = sWorld.GetMaxLevel(this);
    SetUInt32Value(PLAYER_FIELD_MAX_LEVEL, maxLevel);

    SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER); // enables automatic power regen
    m_session->FullLogin(this);
    if(m_session)
        m_session->m_loggingInPlayer = NULL;

    if( !isAlive() )
    {
        myCorpse = objmgr.GetCorpseByOwner(GetLowGUID());
        if(myCorpse == NULL)
        {
            myCorpse = objmgr.LoadCorpse(GetLowGUID());
            if(myCorpse == NULL)
                myCorpse = CreateCorpse();
        }
    }

    if(myCorpse != NULL)
    {
        WorldPacket* data = new WorldPacket(MSG_CORPSE_QUERY, 21);
        BuildCorpseInfo(data, myCorpse);
        SendDelayedPacket(data);
    }
}

void Player::RolloverHonor()
{
    uint32 current_val = (g_localTime.tm_year << 16) | g_localTime.tm_yday;
    if( current_val != m_honorRolloverTime )
    {
        m_honorRolloverTime = current_val;
        m_honorYesterday = m_honorToday;
        m_killsYesterday = m_killsToday;
        m_honorToday = m_killsToday = 0;
    }
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

    if(sle->type == SKILL_TYPE_PROFESSION)
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
    uint32 rank = 0;
    uint32 spellid = 0;
    SpellSet::iterator itr = mSpells.begin();
    if(itr != mSpells.end())
    {
        for(itr; itr != mSpells.end(); itr++)
        {
            SpellEntry* sp = dbcSpell.LookupEntry(*itr);
            if(sp && sp->RankNumber && sp->NameHash == namehash && sp->RankNumber > rank)
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

void Player::_LoadQuestLogEntry(QueryResult * result)
{
    QuestLogEntry *entry;
    Quest *quest;
    Field *fields;
    uint32 questid;
    uint32 baseindex;

    // clear all fields
    for(int i = 0; i < 50; i++)
    {
        baseindex = PLAYER_QUEST_LOG + (i * 5);
        SetUInt32Value(baseindex + 0, 0);
        SetUInt32Value(baseindex + 1, 0);
        SetUInt64Value(baseindex + 2, 0);
        SetUInt32Value(baseindex + 4, 0);
    }

    int slot = 0;

    if(result)
    {
        do
        {
            fields = result->Fetch();
            questid = fields[1].GetUInt32();
            quest = sQuestMgr.GetQuestPointer(questid);
            slot = fields[2].GetUInt32();
            ASSERT(slot != -1);

            // remove on next save if bad quest
            if(!quest)
            {
                m_removequests.insert(questid);
                continue;
            }
            if(m_questlog[slot] != 0)
                continue;

            if(slot >= QUEST_LOG_COUNT)
                break;

            entry = NULL;
            entry = new QuestLogEntry();
            entry->Init(quest, castPtr<Player>(this), slot);
            entry->LoadFromDB(fields);
            entry->UpdatePlayerFields();
        } while(result->NextRow());
    }
}

QuestLogEntry* Player::GetQuestLogForEntry(uint32 quest)
{
    for(int i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if(m_questlog[i] == ((QuestLogEntry*)0x00000001))
            m_questlog[i] = NULL;

        if(m_questlog[i] != NULL)
        {
            if(m_questlog[i]->GetQuest() && m_questlog[i]->GetQuest()->id == quest)
                return m_questlog[i];
        }
    }
    return NULL;
    /*uint32 x = PLAYER_QUEST_LOG_1_1;
    uint32 y = 0;
    for(; x < PLAYER_VISIBLE_ITEM_1_CREATOR && y < 25; x += 3, y++)
    {
        if(m_uint32Values[x] == quest)
            return m_questlog[y];
    }
    return NULL;*/
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
    if(GetTransportGuid() && m_CurrentTransporter)
    {
        LocationVector pos;
        GetMovementInfo()->GetTransportPosition(pos);
        SetPosition(m_CurrentTransporter->GetPositionX() + pos.x,
            m_CurrentTransporter->GetPositionY() + pos.y,
            m_CurrentTransporter->GetPositionZ() + pos.z,
            GetOrientation());
    }

    // If we join an invalid instance and get booted out, castPtr<Player>(this) will prevent our stats from doubling :P
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
}

void Player::AddToWorld(MapMgr* pMapMgr)
{
    FlyCheat = false;
    // check transporter
    if(GetTransportGuid() && m_CurrentTransporter)
    {
        LocationVector pos;
        GetMovementInfo()->GetTransportPosition(pos);
        SetPosition(m_CurrentTransporter->GetPositionX() + pos.x,
            m_CurrentTransporter->GetPositionY() + pos.y,
            m_CurrentTransporter->GetPositionZ() + pos.z,
            GetOrientation());
    }

    // If we join an invalid instance and get booted out, castPtr<Player>(this) will prevent our stats from doubling :P
    if(IsInWorld())
        return;

    m_beingPushed = true;
    WorldObject::AddToWorld(pMapMgr);

    // Add failed.
    if(m_mapMgr == NULL)
    {
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
    GetItemInterface()->CheckAreaItems();

    // Process create packet
    PopPendingUpdates();

    if(m_TeleportState == 2)   // Worldport Ack
        OnWorldPortAck();

    ResetSpeedHack();
    m_beingPushed = false;
    AddItemsToWorld();
    GetMovementInfo()->SetTransportLock(false);

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
    data << GetGUID();
    m_AuraInterface.BuildAuraUpdateAllPacket(&data);
    SendPacket(&data);

    if(m_FirstLogin)
    {
        sEventMgr.AddEvent(this, &Player::FullHPMP, EVENT_PLAYER_FULL_HPMP, 200, 0, 0);
        sHookInterface.OnFirstEnterWorld(castPtr<Player>(this));
        m_FirstLogin = false;
    }

    // send world states
    if( m_mapMgr != NULL )
        m_mapMgr->GetStateManager().SendWorldStates(castPtr<Player>(this));

    // execute some of zeh hooks
    sHookInterface.OnEnterWorld(castPtr<Player>(this));
    sHookInterface.OnZone(castPtr<Player>(this), m_zoneId, 0);
    CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnZoneChange )( castPtr<Player>(this), m_zoneId, 0 );
    CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnPlayerEnter )( castPtr<Player>(this) );

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

    ResetHeartbeatCoords();
    m_heartbeatDisable = 0;
    m_speedChangeInProgress =false;
    m_lastMoveType = 0;

    /* send weather */
    sWeatherMgr.SendWeather(castPtr<Player>(this));

    if( load_health > 0 )
    {
        if( load_health > GetUInt32Value(UNIT_FIELD_MAXHEALTH) )
            SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH) );
        else SetUInt32Value(UNIT_FIELD_HEALTH, load_health);
    }

    for(uint8 i = 0; i < 5; i++)
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

    z_axisposition = 0.0f;
    m_changingMaps = false;

    SendPowerUpdate();

    if(!sWorld.m_blockgmachievements || !GetSession()->HasGMPermissions())
    {
        if(sWorld.RealAchievement && !GetAchievementInterface()->HasAchievement(sWorld.AnniversaryAchievement))
        {   // Doh!
            GetAchievementInterface()->ForceEarnedAchievement(sWorld.AnniversaryAchievement);
        }
    }
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

void Player::ResetHeartbeatCoords()
{
    m_lastHeartbeatPosition = m_position;
    if( m_isMoving )
        m_startMoveTime = m_lastMoveTime;
    else m_startMoveTime = 0;

    m_cheatEngineChances = 2;
    //_lastHeartbeatT = getMSTime();
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
    for(uint8 i = 0; i < 5; i++)
        load_power[i] = GetPower(i);

    sHookInterface.OnPlayerChangeArea(this, 0, 0, GetAreaId());
    CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnChangeArea )( this, 0, 0, GetAreaId() );

    m_mapMgr->GetStateManager().ClearWorldStates(this);

    if(m_bg)
    {
        m_bg->RemovePlayer(castPtr<Player>(this), true);
        m_bg = NULL;
    }

    // Cancel trade if it's active.
    Player* pTarget;
    if(!mTradeTarget.empty())
    {
        pTarget = GetTradeTarget();
        if(pTarget)
            pTarget->ResetTradeVariables();

        ResetTradeVariables();
    }
    //clear buyback
    GetItemInterface()->EmptyBuyBack();

    ResetHeartbeatCoords();

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

    if(IsInWorld())
    {
        RemoveItemsFromWorld();
        Unit::RemoveFromWorld(false);
    }

    sWorld.mInWorldPlayerCount--;

    if(GetTaxiState())
        event_RemoveEvents( EVENT_PLAYER_TAXI_INTERPOLATE );

    if( m_CurrentTransporter && !GetMovementInfo()->GetTransportLock() )
    {
        m_CurrentTransporter->RemovePlayer(castPtr<Player>(this));
        m_CurrentTransporter = NULL;
        GetMovementInfo()->ClearTransportData();
    }

    if( GetVehicle() )
    {
        GetVehicle()->RemovePassenger(castPtr<Player>(this));
        SetVehicle(NULL);
    }

    m_changingMaps = true;
}

// TODO: perhaps item should just have a list of mods, that will simplify code
void Player::_ApplyItemMods(Item* item, int16 slot, bool apply, bool justdrokedown /* = false */, bool skip_stat_apply /* = false  */)
{
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    ASSERT( item );
    if(item->StatsApplied == apply)
        return; // Don't apply or remove twice.
    ItemPrototype* proto = item->GetProto();

    //fast check to skip mod applying if the item doesnt meat the requirements.
    if( item->GetUInt32Value( ITEM_FIELD_DURABILITY ) == 0 && item->GetUInt32Value( ITEM_FIELD_MAXDURABILITY ) && justdrokedown == false )
        return;

    //check for rnd prop
    item->ApplyRandomProperties( true );

    //Items Set check
    uint32 setid = proto->ItemSet;

    // These season pvp itemsets are interchangeable and each set group has the same
    // bonuses if you have a full set made up of parts from any of the 3 similar sets
    // you will get the highest sets bonus

    switch (setid)
    {
        // * Gladiator's Battlegear
        case 701:
        case 736:
        case 567:
            setid = 746;
            break;

        // * Gladiator's Dreadgear
        case 702:
        case 734:
        case 568:
            setid = 734;
            break;

        // * Gladiator's Earthshaker
        case 703:
        case 732:
        case 578:
            setid = 734;
            break;

        // * Gladiator's Felshroud
        case 704:
        case 735:
        case 615:
            setid = 735;
            break;

        // * Gladiator's Investiture
        case 705:
        case 278:
        case 687:
            setid = 728;
            break;

        // * Gladiator's Pursuit
        case 706:
        case 723:
        case 586:
            setid = 723;
            break;

        // * Gladiator's Raiment
        case 707:
        case 729:
        case 581:
            setid = 729;
            break;

        // * Gladiator's Redemption
        case 708:
        case 725:
        case 690:
            setid = 725;
            break;

        // * Gladiator's Refuge
        case 709:
        case 720:
        case 685:
            setid = 720;
            break;

        // * Gladiator's Regalia
        case 710:
        case 724:
        case 579:
            setid = 724;
            break;

        // * Gladiator's Sanctuary
        case 711:
        case 721:
        case 584:
            setid = 721;
            break;

        // * Gladiator's Thunderfist
        case 712:
        case 733:
        case 580:
            setid = 733;
            break;

        // * Gladiator's Vestments
        case 713:
        case 730:
        case 577:
            setid = 730;
            break;

        // * Gladiator's Vindication
        case 714:
        case 726:
        case 583:
            setid = 726;
            break;

        // * Gladiator's Wartide
        case 715:
        case 731:
        case 686:
            setid = 731;
            break;

        // * Gladiator's Wildhide
        case 716:
        case 722:
        case 585:
            setid = 722;
            break;
    }

    // Set
    if( setid != 0 )
    {
        ItemSetEntry* set = dbcItemSet.LookupEntry( setid );
        if( set != NULL)
        {
            ASSERT( set );
            ItemSet* Set = NULL;
            std::list<ItemSet>::iterator i;
            for( i = m_itemsets.begin(); i != m_itemsets.end(); i++ )
            {
                if( i->setid == setid )
                {
                    Set = &(*i);
                    break;
                }
            }

            if( apply )
            {
                if( Set == NULL )
                {
                    Set = new ItemSet;
                    memset( Set, 0, sizeof( ItemSet ) );
                    Set->itemscount = 1;
                    Set->setid = setid;
                }
                else
                    Set->itemscount++;

                if( !set->RequiredSkillID || ( _GetSkillLineCurrent( set->RequiredSkillID, true ) >= set->RequiredSkillAmt ) )
                {
                    for( uint8 x = 0; x < 8; x++)
                    {
                        if( Set->itemscount==set->itemscount[x])
                        {//cast new spell
                            SpellEntry *info = dbcSpell.LookupEntry( set->SpellID[x] );
                            Spell* spell = new Spell( this, info, true, NULL );
                            SpellCastTargets targets;
                            targets.m_unitTarget = GetGUID();
                            spell->prepare( &targets );
                        }
                    }
                }
                if( i == m_itemsets.end() )
                {
                    m_itemsets.push_back( *Set );
                    delete Set;
                }
            }
            else
            {
                if( Set )
                {
                    for( uint32 x = 0; x < 8; x++ )
                    {
                        if( Set->itemscount == set->itemscount[x] )
                        {
                            RemoveAura( set->SpellID[x], GetGUID() );
                        }
                    }

                    if(!(--Set->itemscount))
                        m_itemsets.erase(i);
                }
            }
        }
    }

    if(proto->minDamage && proto->maxDamage)
    {
        ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_MINDAMAGE, ITEM_STAT_CUSTOM_DAMAGE_MIN, proto->minDamage);
        ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_MAXDAMAGE, ITEM_STAT_CUSTOM_DAMAGE_MAX, proto->maxDamage);
        ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_WEAPONDELAY, ITEM_STAT_CUSTOM_WEAPON_DELAY, proto->Delay);
    }
    if(proto->Armor) ModifyBonuses( apply, item->GetGUID(), MOD_SLOT_ARMOR, ITEM_STAT_PHYSICAL_RESISTANCE, proto->Armor);

    // Stats
    for( int i = 0; i < 10; i++ )
    {
        int32 val = proto->Stats[i].Value;
        if( val == 0 )
            continue;
        ModifyBonuses( apply, item->GetGUID(), i, proto->Stats[i].Type, val);
    }

    // Misc
    if( apply )
    {
        // Apply all enchantment bonuses
        item->ApplyEnchantmentBonuses();

        for( int8 k = 0; k < 5; k++ )
        {
            if( item->GetProto()->Spells[k].Id == 0 )
                continue;

            if( item->GetProto()->Spells[k].Trigger == ON_EQUIP )
            {
                SpellEntry* spells = dbcSpell.LookupEntry( item->GetProto()->Spells[k].Id );
                if(spells != NULL)
                {
                    if( spells->RequiredShapeShift )
                    {
                        AddShapeShiftSpell( spells->Id );
                        continue;
                    }

                    Spell* spell = new Spell( castPtr<Player>(this), spells ,true, NULL );
                    SpellCastTargets targets;
                    targets.m_unitTarget = GetGUID();
                    spell->castedItemId = item->GetEntry();
                    spell->prepare( &targets );
                }
            }
            else if( item->GetProto()->Spells[k].Trigger == CHANCE_ON_HIT )
            {
                // Todo:PROC
            }
        }
    }
    else
    {
        // Remove all enchantment bonuses
        item->RemoveEnchantmentBonuses();
        for( int8 k = 0; k < 5; k++ )
        {
            if( item->GetProto()->Spells[k].Trigger == ON_EQUIP )
            {
                SpellEntry* spells = dbcSpell.LookupEntry( item->GetProto()->Spells[k].Id );
                if(spells)
                {
                    if( spells->RequiredShapeShift )
                        RemoveShapeShiftSpell( spells->Id );
                    else
                        RemoveAura( item->GetProto()->Spells[k].Id );
                }
            }
            else if( item->GetProto()->Spells[k].Trigger == CHANCE_ON_HIT )
            {
                // Todo:PROC
            }
        }
    }

    if( !skip_stat_apply )
        UpdateStats();
    item->StatsApplied = apply;
}


void Player::SetMovement(uint8 pType, uint32 flag)
{
    WorldPacket data(12);
    switch(pType)
    {
    case MOVE_ROOT:
        {
            data.SetOpcode(SMSG_MOVE_ROOT);
            data << GetGUID();
            data << flag;
            m_currentMovement = MOVE_ROOT;
        }break;
    case MOVE_UNROOT:
        {
            data.SetOpcode(SMSG_MOVE_UNROOT);
            data << GetGUID();
            data << flag;
            m_currentMovement = MOVE_UNROOT;
        }break;
    case MOVE_WATER_WALK:
        {
            m_setwaterwalk = true;
            data.SetOpcode(SMSG_MOVE_WATER_WALK);
            data << GetGUID();
            data << flag;
        }break;
    case MOVE_LAND_WALK:
        {
            m_setwaterwalk = false;
            m_WaterWalkTimer = getMSTime()+500;
            data.SetOpcode(SMSG_MOVE_LAND_WALK);
            data << GetGUID();
            data << flag;
        }break;
    default:break;
    }

    if(data.size() > 0)
        SendMessageToSet(&data, true);
}

void Player::SetPlayerSpeed(uint8 SpeedType, float value)
{
    if( value < 0.1f )
        value = 0.1f;

    WorldPacket data(SMSG_MOVE_SET_RUN_SPEED, 200);
    if( SpeedType != SWIMBACK )
    {
        data << GetGUID();
        data << m_speedChangeCounter++;
        if( SpeedType == RUN )
            data << uint8(1);

        data << value;
    }
    else
    {
        data << GetGUID();
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
            if(value == m_lastRunSpeed)
                return;

            data.SetOpcode(SMSG_MOVE_SET_RUN_SPEED);
            m_runSpeed = value;
            m_lastRunSpeed = value;
        }break;

    case RUNBACK:
        {
            if(value == m_lastRunBackSpeed)
                return;

            data.SetOpcode(SMSG_MOVE_SET_RUN_BACK_SPEED);
            m_backWalkSpeed = value;
            m_lastRunBackSpeed = value;
        }break;

    case SWIM:
        {
            if(value == m_lastSwimSpeed)
                return;

            data.SetOpcode(SMSG_MOVE_SET_SWIM_SPEED);
            m_swimSpeed = value;
            m_lastSwimSpeed = value;
        }break;

    case SWIMBACK:
        {
            if(value == m_lastBackSwimSpeed)
                break;

            data.SetOpcode(SMSG_MOVE_SET_SWIM_BACK_SPEED);
            m_backSwimSpeed = value;
            m_lastBackSwimSpeed = value;
        }break;

    case TURN:
        {
            data.SetOpcode(SMSG_MOVE_SET_TURN_RATE);
            m_turnRate = value;
        }break;

    case FLY:
        {
            if(value == m_lastFlySpeed)
                return;

            data.SetOpcode(SMSG_MOVE_SET_FLIGHT_SPEED);
            m_flySpeed = value;
            m_lastFlySpeed = value;
        }break;

    case FLYBACK:
        {
            if(value == m_lastBackFlySpeed)
                return;

            data.SetOpcode(SMSG_MOVE_SET_FLIGHT_BACK_SPEED);
            m_backFlySpeed = value;
            m_lastBackFlySpeed = value;
        }break;

    case PITCH_RATE:
        {
            data.SetOpcode(SMSG_MOVE_SET_PITCH_RATE);
            m_pitchRate = value;
        }break;

    default:
        return;
    }

    SendMessageToSet(&data, true);

    // dont mess up on these
    m_speedChangeInProgress = true;
}

void Player::BuildPlayerRepop()
{
    SetUInt32Value(UNIT_FIELD_HEALTH, 1 );

    //8326 --for all races but ne, 20584--ne
    SpellCastTargets tgt;
    tgt.m_unitTarget = GetGUID();

    SpellEntry *inf = dbcSpell.LookupEntry(Wispform ? 20584 : 8326);
    Spell* sp = new Spell(castPtr<Player>(this), inf, true, NULL);
    sp->prepare(&tgt);

    StopMirrorTimer(0);
    StopMirrorTimer(1);
    StopMirrorTimer(2);

    SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE);

    SetMovement(MOVE_UNROOT, 1);
    SetMovement(MOVE_WATER_WALK, 1);
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
        GetMovementInfo()->ClearTransportData();

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
    if (!sHookInterface.OnResurrect(this))
        return;
    if(PreventRes)
        return;
    sEventMgr.RemoveEvents(castPtr<Player>(this),EVENT_PLAYER_FORCED_RESURECT); //in case somebody resurected us before this event happened
    if( m_resurrectHealth )
        SetUInt32Value(UNIT_FIELD_HEALTH, std::min( m_resurrectHealth, GetUInt32Value(UNIT_FIELD_MAXHEALTH) ) );
    if( m_resurrectMana )
        SetPower(POWER_TYPE_MANA, std::min( m_resurrectMana, GetMaxPower(POWER_TYPE_MANA) ) );

    m_resurrectHealth = m_resurrectMana = 0;

    SpawnCorpseBones();
    RemoveAura(Wispform ? 20584 : 8326);

    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE);
    SetDeathState(ALIVE);
    UpdateVisibility();
    SetMovement(MOVE_LAND_WALK, 1);
    if(ReclaimCount < 8) // 2 minute limit.
        ReclaimCount++;
    // Crow: Guessed intervals.
    // TODO: sEventMgr.AddEvent(this, &Player::DecReclaimCount, EVENT_DEC_CORPSE_RECLAIM_COUNT, 60000, 1, 0);

    if(IsInWorld() && pResurrector != NULL && pResurrector->IsInWorld())
    {
        //make sure corpse and resurrector are on the same map.
        if( GetMapId() == pResurrector->GetMapId() )
        {
            if( m_resurrectLoction.x == 0.0f && m_resurrectLoction.y == 0.0f && m_resurrectLoction.z == 0.0f )
            {
                if(pResurrector->IsPlayer())
                    SafeTeleport(pResurrector->GetMapId(),pResurrector->GetInstanceID(),pResurrector->GetPosition());
            }
            else
            {
                if(pResurrector->IsPlayer())
                    SafeTeleport(GetMapId(), GetInstanceID(), m_resurrectLoction);
                m_resurrectLoction.ChangeCoords(0.0f, 0.0f, 0.0f);
            }
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

    SetMovement(MOVE_ROOT, 0);

    StopMirrorTimer(0);
    StopMirrorTimer(1);
    StopMirrorTimer(2);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED); //player death animation, also can be used with DYNAMIC_FLAGS <- huh???
    SetUInt32Value( UNIT_DYNAMIC_FLAGS, 0x00 );
    SetPower(POWER_TYPE_RAGE, 0);
    SetPower(POWER_TYPE_RUNIC, 0);

    // combo points reset upon death
    NullComboPoints();

    GetAchievementInterface()->HandleAchievementCriteriaConditionDeath();
    GetAchievementInterface()->HandleAchievementCriteriaDeath();

    sHookInterface.OnDeath(castPtr<Player>(this));
}

Corpse* Player::CreateCorpse()
{
    Corpse* pCorpse = NULL;
    uint32 _uf, _pb, _pb2, _cfb1, _cfb2;

    objmgr.DelinkPlayerCorpses(castPtr<Player>(this));
    pCorpse = objmgr.CreateCorpse();
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

    uint32 iDisplayID = 0;
    uint16 iIventoryType = 0;
    uint32 _cfi = 0;
    Item* pItem;
    for (int8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if(( pItem = GetItemInterface()->GetInventoryItem(i)))
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
        if(ManagerCheck(m_mapMgr))
            if(FunctionCall(m_mapMgr, MapSupportsPlayerLoot)())
                m_insigniaTaken = false;   // for next time
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

        // insignia stuff
        if(ManagerCheck(m_mapMgr))
        {
            if(FunctionCall(m_mapMgr, MapSupportsPlayerLoot)())
            {
                if( !m_insigniaTaken )
                    pCorpse->SetFlag(CORPSE_FIELD_FLAGS, 60);
            }
        }
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
        else
        {
            //Cheater!
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
        if((pItem = GetItemInterface()->GetInventoryItem(i)))
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
                {
                    ApplyItemMods(pItem, i, false, true);
                }

                pItem->SetUInt32Value(ITEM_FIELD_DURABILITY,(uint32)pNewDurability);
                pItem->m_isDirty = true;
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

    if ( ManagerCheck(m_mapMgr) )
    {
        if( FunctionCall(m_mapMgr, OnPlayerRepopRequest)(this) )
            return;
    }
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

    if(sHookInterface.OnRepop(castPtr<Player>(this)) && dest.x != 0 && dest.y != 0 && dest.z != 0)
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

void Player::AddToFinishedQuests(uint32 quest_id)
{
    m_finishedQuests.insert(quest_id);
    GetAchievementInterface()->HandleAchievementCriteriaQuestCount( uint32(m_finishedQuests.size()));
}

bool Player::HasFinishedQuest(uint32 quest_id)
{
    return (m_finishedQuests.find(quest_id) != m_finishedQuests.end());
}

void Player::AddToFinishedDailyQuests(uint32 quest_id)
{
    if(m_finishedDailyQuests.size() >= 25)
        return;

    DailyMutex.Acquire();
    SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1 + uint32(m_finishedDailyQuests.size()), quest_id);
    m_finishedDailyQuests.insert(quest_id);
    DailyMutex.Release();
}

void Player::ResetDailyQuests()
{
    m_finishedDailyQuests.clear();

    for(uint32 i = 0; i < 25; i++)
        SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1 + i, 0);
}

bool Player::HasFinishedDailyQuest(uint32 quest_id)
{
    return (m_finishedDailyQuests.find(quest_id) != m_finishedDailyQuests.end());
}
void Player::_LoadSkills(QueryResult * result)
{
    int32 proff_counter = 2;
    if(result)
    {
        const ItemProf * prof;
        do
        {
            PlayerSkill sk;
            uint32 v1;
            Field *fields = result->Fetch();
            v1 = fields[1].GetUInt32();
            if(!sk.Reset(v1))
                continue; // Just skip it, it'll be wiped when we save.

            if (fields[2].GetUInt32() == SKILL_TYPE_PROFESSION )
            {
                proff_counter--;
                if( proff_counter < 0 )
                {
                    proff_counter = 0;

                    sWorld.LogCheater(GetSession(),"Bug_Primary_Professions Player %s [%d] \n", GetName(), GetLowGUID());

                    const char * message = "Your character has more then 2 primary professions.\n You have 5 minutes remaining to unlearn and relog.";

                    // Send warning after 2 minutes, as he might miss it if it's send inmedeately.
                    sEventMgr.AddEvent( castPtr<Player>(this), &Player::_Warn, message, EVENT_UNIT_SENDMESSAGE, 60000, 1, 0);
                    sEventMgr.AddEvent( castPtr<Player>(this), &Player::_Kick, EVENT_PLAYER_KICK, 360000, 1, 0 );
                }
            }
            sk.CurrentValue = fields[3].GetUInt32();
            sk.MaximumValue = fields[4].GetUInt32();
            m_skills.insert(std::make_pair(v1,sk));

            prof = GetProficiencyBySkill(v1);
            if(prof)
            {
                if(prof->itemclass==4)
                    armor_proficiency|=prof->subclass;
                else
                    weapon_proficiency|=prof->subclass;
            }
        } while(result->NextRow());
    }
    else // no result from db set up from create_info instead
    {
        for(std::list<CreateInfo_SkillStruct>::iterator ss = info->skills.begin(); ss!=info->skills.end(); ss++)
        {
            if(ss->skillid && ss->currentval && ss->maxval && !::GetSpellForLanguageSkill(ss->skillid))
                _AddSkillLine(ss->skillid, ss->currentval, ss->maxval);
        }
    }

    _UpdateMaxSkillCounts();
}

void Player::UpdateHit(int32 hit)
{

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

void Player::UpdateChances()
{
    uint32 pClass = (uint32)getClass();
    uint32 pLevel = getLevel(), levmax = sWorld.GetMaxLevelStatCalc();
    if(levmax && levmax < pLevel)
        pLevel = levmax;
    if(pLevel > MAXIMUM_ATTAINABLE_LEVEL)
        pLevel = MAXIMUM_ATTAINABLE_LEVEL;

    float tmp = 0;
    float defence_contribution = 0;

    // defence contribution estimate
    defence_contribution = ( float( _GetSkillLineCurrent( SKILL_DEFENSE, true ) ) - ( float( pLevel ) * 5.0f ) ) * 0.04f;
    if( defence_contribution < 0.0f )
        defence_contribution = 0.0f;

    defence_contribution += (CalcRating(PLAYER_RATING_MODIFIER_DEFENCE) * 0.04f);
    if( defence_contribution < 0.0f )
        defence_contribution = 0.0f;

    // dodge
    tmp = CalculateDefenseFromAgilForClassAndLevel(pClass, pLevel);
    tmp += CalcRating( PLAYER_RATING_MODIFIER_DODGE ) + defence_contribution;
    tmp = std::min( std::max( 5.0f, tmp), DodgeCap[pClass] );

    // Add dodge from spell after checking cap and base.
    tmp += GetDodgeFromSpell();
    SetFloatValue( PLAYER_DODGE_PERCENTAGE, tmp);

    // Block
    Item* it = GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
    if( it != NULL && it->GetProto() && it->GetProto()->InventoryType == INVTYPE_SHIELD )
    {
        tmp = 5.0f + std::max( 0.0f, CalcRating( PLAYER_RATING_MODIFIER_BLOCK )) + std::max( 0.0f, GetBlockFromSpell());
        tmp += defence_contribution;
    } else tmp = 0.0f;

    SetFloatValue( PLAYER_BLOCK_PERCENTAGE, std::max( 0.0f, std::min( tmp, 95.0f ) )  );

    //parry
    tmp = 5.0f + CalcRating( PLAYER_RATING_MODIFIER_PARRY ) + GetParryFromSpell();
    if(pClass == DEATHKNIGHT) // DK gets 1/4 of strength as parry rating
        tmp += CalcPercentForRating(PLAYER_RATING_MODIFIER_PARRY, GetUInt32Value(UNIT_FIELD_STRENGTH) / 4);
    tmp += defence_contribution;

    SetFloatValue( PLAYER_PARRY_PERCENTAGE, std::max( 5.0f, std::min( tmp, 95.0f ) ) ); //let us not use negative parry. Some spells decrease it

    //critical
    std::map< uint32, WeaponModifier >::iterator itr = tocritchance.begin();
    Item* tItemRanged = GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_RANGED );
    Item* tItemMelee = GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_MAINHAND );

    float melee_bonus = 0;
    float ranged_bonus = 0;

    //-1 = any weapon
    for(; itr != tocritchance.end(); itr++ )
    {
        if( itr->second.wclass == ( uint32 )-1 || ( tItemMelee != NULL && ( 1 << tItemMelee->GetProto()->SubClass & itr->second.subclass ) ) )
            melee_bonus += itr->second.value;
        if( itr->second.wclass == ( uint32 )-1 || ( tItemRanged != NULL && ( 1 << tItemRanged->GetProto()->SubClass & itr->second.subclass ) ) )
            ranged_bonus += itr->second.value;
    }

    tmp = CalculateCritFromAgilForClassAndLevel(pClass, pLevel);
    float cr = tmp + CalcRating( PLAYER_RATING_MODIFIER_MELEE_CRIT ) + melee_bonus;
    SetFloatValue( PLAYER_CRIT_PERCENTAGE, std::min( cr, 71.0f ) );

    float rcr = tmp + CalcRating( PLAYER_RATING_MODIFIER_RANGED_CRIT ) + ranged_bonus;
    SetFloatValue( PLAYER_RANGED_CRIT_PERCENTAGE, std::min( rcr, 71.0f ) );

    gtFloat* SpellCritBase  = dbcSpellCritBase.LookupEntry(pClass-1);
    gtFloat* SpellCritPerInt = dbcSpellCrit.LookupEntry(pLevel - 1 + (pClass-1)*100);

    spellcritperc = 100*(SpellCritBase->val + (GetUInt32Value( UNIT_FIELD_INTELLECT ) * SpellCritPerInt->val)) +
        GetSpellCritFromSpell() + CalcRating( PLAYER_RATING_MODIFIER_SPELL_CRIT );

    UpdateChanceFields();
}

void Player::UpdateChanceFields()
{
    gtFloat* SpellCritBase  = dbcSpellCritBase.LookupEntry(getClass()-1);
    gtFloat* SpellCritPerInt = dbcSpellCrit.LookupEntry(getLevel() - 1 + (getClass()-1)*100);

    float spellcritperc = 100*(SpellCritBase->val + (GetUInt32Value( UNIT_FIELD_INTELLECT ) * SpellCritPerInt->val)) + GetSpellCritFromSpell() + CalcRating( PLAYER_RATING_MODIFIER_SPELL_CRIT );

    // Update spell crit values in fields
    for(uint32 i = 0; i < 7; i++)
        SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE + i, spellcritperc);
}

// formulas from wowwiki
void Player::UpdateStats()
{
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
    // if multiplier RestXP (from Hearthstone.config) is f.e 2, you only need 4hrs/bubble.
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
    {
        if(bGMTagOn)
            return true;

        return false;
    }

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
            if(obj->IsPlayer() && castPtr<Player>(obj)->InStealth())
                return false;

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

                if(pObj->InStealth()) // Stealth Detection
                {
                    if(GetGroup() && pObj->GetGroup() == GetGroup()) // can see stealthed group members
                        return true;

                    WoWGuid stalker = pObj->stalkedby;
                    if(stalker)
                    {
                        WorldObject* pStalker = IsInWorld() ? GetMapMgr()->_GetObject(stalker) : NULL;
                        if(pStalker == castPtr<Player>(this))
                            return true;

                        if(pStalker && pStalker->IsPlayer())
                        {
                            if(GetGroup() && GetGroup()->HasMember(castPtr<Player>(pStalker)))
                                return true;
                        }
                    }

                    // 2d distance SQUARED!
                    float base_range = 49.0f;
                    float modDistance = 0.0f;

                    int32 hide_level = (getLevel() * 5 + GetStealthDetectBonus()) - pObj->GetStealthLevel();
                    modDistance += hide_level * 0.2f;

                    if(isInBackOfTarget(pObj))
                        base_range /= 2.0f;

                    if( base_range + modDistance <= 0 )
                        return false;

                    base_range += modDistance;

                    float distance = GetDistance2dSq(pObj);
                    if(distance < base_range)
                        return true;
                    else
                        return false;
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
            *data << pUnit->GetGUID();
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
    std::set<WoWGuid> unique;
    WorldObject* pObj = NULL;
    for( WorldObject::InRangeSet::iterator itr = m_visibleObjects.begin(); itr != m_visibleObjects.end();)
    {
        pObj = (*itr);
        ++itr;
        if(unique.find(pObj->GetGUID()) == unique.end())
        {
            if(pObj->IsPlayer())
                castPtr<Player>(pObj)->DestroyForPlayer(this);
            else
                pObj->DestroyForPlayer(this);
            unique.insert(pObj->GetGUID());
        }
    }
    m_visibleObjects.clear();
    for( WorldObject::InRangeSet::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end();)
    {
        pObj = (*itr);
        ++itr;
        if(unique.find(pObj->GetGUID()) == unique.end())
        {
            if(pObj->IsPlayer())
                castPtr<Player>(pObj)->DestroyForPlayer(this);
            else
                pObj->DestroyForPlayer(this);
            unique.insert(pObj->GetGUID());
        }
    }
    m_objectsInRange.clear();
    for( std::unordered_set<Player*>::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end();)
    {
        pObj = (*itr);
        ++itr;
        if(unique.find(pObj->GetGUID()) == unique.end())
        {
            castPtr<Player>(pObj)->DestroyForPlayer(this);
            unique.insert(pObj->GetGUID());
        }
    }
    m_inRangePlayers.clear();
    for( WorldObject::InRangeUnitSet::iterator itr = GetInRangeOppFactsSetBegin(); itr != GetInRangeOppFactsSetEnd();)
    {
        pObj = (*itr);
        ++itr;
        if(unique.find(pObj->GetGUID()) == unique.end())
        {
            if(pObj->IsPlayer())
                castPtr<Player>(pObj)->DestroyForPlayer(this);
            else
                pObj->DestroyForPlayer(this);
            unique.insert(pObj->GetGUID());
        }
    }
    m_oppFactsInRange.clear();
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

            // Check the item_quest_association table for an entry related to this item
            QuestAssociationList *tempList = sQuestMgr.GetQuestAssociationListForItemId( itemid );
            if( tempList != NULL )
            {
                uint32 countNeeded = 0;
                for(QuestAssociationList::iterator itr = tempList->begin(); itr != tempList->end(); itr++)
                    countNeeded += (*itr)->item_count;
                if(countNeeded < GetItemInterface()->GetItemCount(itemid))
                    return true;
            }

            // No item_quest association found, check the quest requirements
            if( !qst->count_required_item )
                continue;

            for( uint8 j = 0; j < 6; ++j )
                if( qst->required_item[j] == itemid && ( GetItemInterface()->GetItemCount( itemid ) < qst->required_itemcount[j] ) )
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
        lootEnt = m_ItemInterface->GetItemByGUID(guid);
    else lootEnt = m_mapMgr->_GetObject(guid);
    if( lootEnt == NULL )
        return;

    int8 loot_method = -1;

    //lootsteal fix
    if( lootEnt->GetTypeId() == TYPEID_UNIT )
    {
        Creature* LootOwner = castPtr<Creature>( lootEnt );
        uint32 GroupId = LootOwner->m_taggingGroup;
        if( GroupId != 0 )
            if(m_Group == NULL || GroupId != m_Group->GetID())
                return;
        loot_method = LootOwner->m_lootMethod;
    }

    if( loot_method < 0 )
    {
        // not set
        if( m_Group != NULL )
            loot_method = m_Group->GetMethod();
        else
            loot_method = PARTY_LOOT_FFA;
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
    uint8 slottype = 0;
    std::vector<__LootItem>::iterator iter = lootEnt->GetLoot()->items.begin();
    for(uint32 x = 0; iter != lootEnt->GetLoot()->items.end(); iter++, x++)
    {
        if(iter->ffa_loot != 2)
        {
            if(iter->has_looted.size())
                continue;
        }
        else
        {
            LooterSet::iterator itr = iter->has_looted.find(GetLowGUID());
            if (itr != iter->has_looted.end())
                continue;
        }

        ItemPrototype* itemProto = iter->item.itemproto;
        if (!itemProto)
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

        slottype = 0;
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
            if(iter->passed)
                slottype = 0;                   // All players passed on the loot

            //if it is ffa loot and not an masterlooter
            if(iter->ffa_loot)
                slottype = 0;
        }

        uint32 filldata[2] = {0, 0};
        if(iter->iRandomSuffix)
        {
            filldata[0] = Item::GenerateRandomSuffixFactor(itemProto);
            filldata[1] = uint32(-int32(iter->iRandomSuffix->id));
        }
        else if(iter->iRandomProperty)
            filldata[1] = uint32(iter->iRandomProperty->ID);

        data << uint8(x);
        data << uint32(itemProto->ItemId);
        data << uint32(iter->StackSize);//nr of items of this type
        data << uint32(iter->item.displayid);
        data << filldata[0] << filldata[1];
        data << slottype;   // "still being rolled for" flag

        if(slottype == 1)
        {
            WorldObject *lootObj = castPtr<WorldObject>(lootEnt);
            if(iter->roll == NULL && !iter->passed && iter->item.itemproto)
            {
                uint32 ipid = 0;
                uint32 factor = 0;
                if(iter->iRandomProperty)
                    ipid = iter->iRandomProperty->ID;
                else if(iter->iRandomSuffix)
                {
                    factor = Item::GenerateRandomSuffixFactor(iter->item.itemproto);
                    ipid = uint32(-int32(iter->iRandomSuffix->id));
                }

                iter->roll = new LootRoll;
                iter->roll->Init(60000, (m_Group != NULL ? m_Group->MemberCount() : 1),  guid, x, iter->item.itemproto->ItemId, factor, uint32(ipid), GetMapMgr());

                data2.Initialize(SMSG_LOOT_START_ROLL);
                data2 << uint64(guid);
                data2 << uint32(mapid);
                data2 << uint32(x);
                data2 << uint32(iter->item.itemproto->ItemId);
                data2 << uint32(factor);
                data2 << uint32(ipid);
                data2 << uint32(iter->StackSize);
                data2 << uint32(60000); // countdown

                Group* pGroup = m_playerInfo->m_Group;
                if(pGroup)
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
                            if(plr && plr->GetItemInterface()->CanReceiveItem(itemProto, iter->StackSize, NULL) == 0)
                            {   // If we have pass on, or if we're not in range, we have to pass.
                                if( plr->m_passOnLoot || ( !lootObj->IsInRangeSet(plr) ) )
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
                            } else iter->roll->PlayerRolled( (*itr), PASS );       // passed
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
                        if(_HasSkillLine(333) && (_GetSkillLineCurrent(333, true) > uint32(itemProto->DisenchantReqSkill)))
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
    SpellRange* range = dbcSpellRange.LookupEntry( rIndex );
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
        packetSMSG_CASTRESULT cr;
        cr.SpellId = spellinfo->Id;
        cr.ErrorMessage = fail;
        cr.MultiCast = 0;
        m_session->OutPacket( SMSG_CAST_FAILED, sizeof(packetSMSG_CASTRESULT), &cr );
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

    if( m_special_state & ( UNIT_STATE_FEAR | UNIT_STATE_CHARM | UNIT_STATE_SLEEP | UNIT_STATE_STUN | UNIT_STATE_CONFUSE ) || IsStunned() || IsFeared() )
        return;

    Unit* target = GetMapMgr()->GetUnit( m_curSelection );
    if( target == NULL || !sFactionSystem.isAttackable(this, target))
    {
        m_AutoShotAttackTimer = 0; //avoid flooding client with error messages
        m_onAutoShot = false;
        return;
    }

    m_AutoShotDuration = GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME);
    if( m_isMoving )
    {
        m_AutoShotAttackTimer = 400; // shoot when we can
        return;
    }

    int32 f = CanShootRangedWeapon( m_AutoShotSpell->Id, target, true );

    if( f != 0 )
    {
        if( f != SPELL_FAILED_OUT_OF_RANGE )
        {
            m_AutoShotAttackTimer = 0;
            m_onAutoShot = false;
        }
        else
        {
            m_AutoShotAttackTimer = m_AutoShotDuration;//avoid flooding client with error mesages
        }
        return;
    }
    else
    {
        m_AutoShotAttackTimer = m_AutoShotDuration;

        Spell* sp = NULL;
        sp = (new Spell( castPtr<Player>(this), m_AutoShotSpell, true, NULL ));
        SpellCastTargets tgt;
        tgt.m_unitTarget = m_curSelection;
        tgt.m_targetMask = TARGET_FLAG_UNIT;
        sp->prepare( &tgt );
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

void Player::EventTimedQuestExpire(Quest *qst, QuestLogEntry *qle, uint32 log_slot, uint32 interval)
{
    qle->SubtractTime(interval);
    sLog.Debug("QuestLogEntry","qle TimeLeft:%u[ms]",qle->GetTimeLeft());

    if(qle->GetTimeLeft() == 0)
    {
        WorldPacket fail;
        sQuestMgr.BuildQuestFailed(&fail, qst->id);
        GetSession()->SendPacket(&fail);
        sHookInterface.OnQuestCancelled(castPtr<Player>(this), qst);
        CALL_QUESTSCRIPT_EVENT(qst->id, OnQuestCancel)(castPtr<Player>(this));
        qle->Quest_Status = QUEST_STATUS__FAILED;
    } else sEventMgr.AddEvent(castPtr<Player>(this), &Player::EventTimedQuestExpire, qst, qle, log_slot, interval, EVENT_TIMED_QUEST_EXPIRE, interval, 1, 0);
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

                uint32 srcItem = qst->srcitem;
                for( uint32 y = 0; y < 6; y++) //always remove collected items (need to be recollectable again in case of repeatable).
                    if( qst->required_item[y] && qst->required_item[y] != srcItem )
                        GetItemInterface()->RemoveItemAmt(qst->required_item[y], qst->required_itemcount[y]);

                // Remove all items given by the questgiver at the beginning
                for(uint32 j = 0; j < 4; j++)
                    if(qst->receive_items[j] && qst->receive_items[j] != srcItem )
                        GetItemInterface()->RemoveItemAmt(qst->receive_items[j], qst->receive_itemcount[i] );

                delete m_questlog[i];
                m_questlog[i] = NULL;
            }
        }
    }

    std::set<uint32>::iterator itr, itr2;
    for (itr = m_finishedQuests.begin(); itr != m_finishedQuests.end();)
    {
        itr2 = itr++;
        Quest * qst = sQuestMgr.GetQuestPointer((*itr2));
        if(qst->required_tradeskill == skill_line)
            m_finishedQuests.erase(itr2);
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
    packetSMSG_SET_PROFICICENCY pr;
    pr.ItemClass = 4;
    pr.Profinciency = armor_proficiency;
    m_session->OutPacket( SMSG_SET_PROFICIENCY, sizeof(packetSMSG_SET_PROFICICENCY), &pr );
    pr.ItemClass = 2;
    pr.Profinciency = weapon_proficiency;
    m_session->OutPacket( SMSG_SET_PROFICIENCY, sizeof(packetSMSG_SET_PROFICICENCY), &pr );

    //Initial Spells
    smsg_InitialSpells();

    data.Initialize(SMSG_SEND_UNLEARN_SPELLS, 4);
    data << uint32(0);                                      // count, for (count) uint32;
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

    data.Initialize(SMSG_ALL_ACHIEVEMENT_DATA, 2000);
    m_achievementInterface->BuildAllAchievementDataPacket(&data);
    GetSession()->SendPacket(&data);

    // Sets
    SendEquipmentSets();

    // Login speed
    data.Initialize(SMSG_LOGIN_SETTIMESPEED);
    data << uint32(RONIN_UTIL::secsToTimeBitFields(UNIXTIME));
    data << float(0.0166666669777748f);
    data << uint32(0);
    GetSession()->SendPacket( &data );

    // Currencies
    data.Initialize(SMSG_INIT_CURRENCY);
    data.WriteBits(0, 23); // Count
    data.FlushBits();
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

void Player::ResetTitansGrip()
{
    if(titanGrip || !GetItemInterface() || HasSpell(46917) || HasTalent(1867) || GetSession()->HasGMPermissions())
        return; // If we have the aura, return, spell, return talent, return, GM, who gives a shit, return.

    Item* mainhand = GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_MAINHAND);
    Item* offhand = GetItemInterface()->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND);
    if(offhand && (offhand->GetProto()->InventoryType == INVTYPE_2HWEAPON || mainhand && mainhand->GetProto()->InventoryType == INVTYPE_2HWEAPON))
    {
        // we need to de-equip this
        offhand = GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND, false);
        if( offhand == NULL )
            return;     // should never happen

        SlotResult result = GetItemInterface()->FindFreeInventorySlot(offhand->GetProto());
        if( !result.Result )
        {
            // no free slots for this item. try to send it by mail
            offhand->RemoveFromWorld(false);
            offhand->SetOwner( NULL );
            offhand->SaveToDB( INVENTORY_SLOT_NOT_SET, 0, true, NULL );
            sMailSystem.DeliverMessage(MAILTYPE_NORMAL, GetGUID(), GetGUID(), "Your offhand item", "", 0, 0, offhand->GetUInt32Value(OBJECT_FIELD_GUID), 1, true);
            offhand->Destruct();
            offhand = NULL;
        }
        else
        {
            if( !GetItemInterface()->SafeAddItem(offhand, result.ContainerSlot, result.Slot) )
            {
                if( !GetItemInterface()->AddItemToFreeSlot(offhand) )   // shouldn't happen either.
                {
                    offhand->Destruct();
                    offhand = NULL;
                }
            }
        }
    }
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
    for (WorldObject::InRangeSet::iterator itr = GetInRangeSetBegin(); itr != GetInRangeSetEnd(); itr++)
    {
        if((*itr)->IsGameObject())
        {
            Gobj = castPtr<GameObject>(*itr);
            ByteBuffer buff(500);
            Gobj->SetUpdateField(OBJECT_FIELD_GUID);
            Gobj->SetUpdateField(OBJECT_FIELD_GUID+1);
            Gobj->BuildValuesUpdateBlockForPlayer(&buff, this);
            PushUpdateBlock(&buff, 1);
        }
    }
}

void Player::UpdateNearbyQuestGivers()
{
    GameObject* Gobj = NULL;
    for (WorldObject::InRangeSet::iterator itr = GetInRangeSetBegin(); itr != GetInRangeSetEnd(); itr++)
    {
        if((*itr)->IsGameObject())
        {
            if(castPtr<GameObject>(*itr)->isQuestGiver())
            {
                uint32 status = sQuestMgr.CalcStatus((*itr), this);
                if(status != QMGR_QUEST_NOT_AVAILABLE)
                {
                    WorldPacket data(SMSG_QUESTGIVER_STATUS, 12);
                    data << (*itr)->GetGUID() << status;
                    SendPacket( &data );
                }
            }
        }
        else if((*itr)->IsCreature())
        {
            if(castPtr<Creature>(*itr)->isQuestGiver())
            {
                uint32 status = sQuestMgr.CalcStatus((*itr), this);
                if(status != QMGR_QUEST_NOT_AVAILABLE)
                {
                    WorldPacket data(SMSG_QUESTGIVER_STATUS, 12);
                    data << (*itr)->GetGUID() << status;
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
    if(GetUInt32Value(UNIT_FIELD_DISPLAYID)!=GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID))
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

    SetPlayerSpeed(RUN, m_runSpeed);

    SafeTeleport(pathnode->mapid, 0, LocationVector(pathnode->x, pathnode->y, pathnode->z));
}

void Player::RemoveSpellsFromLine(uint32 skill_line)
{
    uint32 cnt = dbcSkillLineSpell.GetNumRows();
    for(uint32 i=0; i < cnt; i++)
    {
        SkillLineSpell* sp = dbcSkillLineSpell.LookupRow(i);
        if(sp)
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
        } else m_uint32.values[powerField] = curValue;
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

    if(PctRegenModifier)
        amt += (amt * PctRegenModifier) / 100;

    if(IsSitting())
        amt *= 1.5f;

    if(inCombat)
        amt *= PctIgnoreRegenModifier;

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
    WorldPacket data(0, 41);
    if(sendpending && mapid != m_mapId && force_new_world)
    {
        data.SetOpcode(SMSG_TRANSFER_PENDING);
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

        //did we get a new instanceid?
        if(instance_id)
            m_instanceId = instance_id;

        //remove us from this map
        if(IsInWorld())
            RemoveFromWorld();

        //send new world
        data.Initialize(SMSG_NEW_WORLD);
        data << destination.x << destination.o << destination.z;
        data << mapid << destination.y;
        GetSession()->SendPacket( &data );
        SetMapId(mapid);
        SetPlayerStatus(TRANSFER_PENDING);
    }
    else
    {
        // we are on same map allready, no further checks needed,
        // send teleport ack msg
        WorldPacket * data = BuildTeleportAckMsg(v);
        m_session->SendPacket(data);
        delete data;

        //reset transporter if we where on one.
        if( m_CurrentTransporter && !GetMovementInfo()->GetTransportLock() )
        {
            m_CurrentTransporter->RemovePlayer(castPtr<Player>(this));
            m_CurrentTransporter = NULL;
        }
    }

    //update position
    m_sentTeleportPosition = destination;
    SetPosition(destination);
    ResetHeartbeatCoords();
    ApplyPlayerRestState(false); // If we don't, and we teleport inside, we'll be rested regardless.
    z_axisposition = 0.0f;
}

// Player::AddItemsToWorld
// Adds all items to world, applies any modifiers for them.

void Player::AddItemsToWorld()
{
    Item* pItem;
    for(uint32 i = 0; i < MAX_INVENTORY_SLOT; i++)
    {
        if( pItem = GetItemInterface()->GetInventoryItem(i) )
        {
            pItem->SetInWorld();

            if(i < INVENTORY_SLOT_BAG_END)    // only equipment slots get mods.
                _ApplyItemMods(pItem, i, true, false, true);

            if(pItem->IsContainer() && GetItemInterface()->IsBagSlot(i))
            {
                for(int32 e = 0; e < pItem->GetProto()->ContainerSlots; e++)
                {
                    if(Item* item = castPtr<Container>(pItem)->GetItem(e))
                        item->SetInWorld();
                }
            }
        }
    }

    UpdateStats();
}

// Player::RemoveItemsFromWorld
// Removes all items from world, reverses any modifiers.

void Player::RemoveItemsFromWorld()
{
    Item* pItem;
    for(uint32 i = 0; i < MAX_INVENTORY_SLOT; i++)
    {
        pItem = m_ItemInterface->GetInventoryItem((int8)i);
        if(pItem)
        {
            if(pItem->IsInWorld())
            {
                if(i < INVENTORY_SLOT_BAG_END)    // only equipment slots get mods.
                    _ApplyItemMods(pItem, i, false, false, true);
                pItem->RemoveFromWorld(false);
            }

            if(pItem->IsContainer() && GetItemInterface()->IsBagSlot(i))
            {
                for(int32 e = 0; e < pItem->GetProto()->ContainerSlots; e++)
                {
                    Item* item = castPtr<Container>(pItem)->GetItem(e);
                    if(item && item->IsInWorld())
                        item->RemoveFromWorld(false);
                }
            }
        }
    }

    UpdateStats();
}

uint32 Player::BuildCreateUpdateBlockForPlayer(ByteBuffer *data, Player* target )
{
    int count = 0;
    if(target == castPtr<Player>(this)) // we need to send create objects for all items.
        count += GetItemInterface()->m_CreateForPlayer(data);

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
    SkillLineSpell *sk;
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
    SkillLineSpell *sk;
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
    bUpdateDataBuffer.append(*data);

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
        if(c < size_t(500) || !CompressAndSendUpdateBuffer((uint32)c, update_buffer, (IsInWorld() ? &GetMapMgr()->m_compressionBuffer : NULL)))
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

    // resend speed if needed
    if(resend_speed)
    {
        SetPlayerSpeed(RUN, m_runSpeed);
        SetPlayerSpeed(FLY, m_flySpeed);
        resend_speed = false;
    }
}

bool Player::CompressAndSendUpdateBuffer(uint32 size, const uint8* update_buffer, ByteBuffer *pCompressionBuffer)
{
    uint32 destsize = compressBound(size);
    int rate = 2 + (size > 25000 ? float2int32(float(size)/15000.f) : 0);
    if(rate < 1 || rate > 9)
        rate = 1;

    // set up stream
    z_stream stream;
    stream.zalloc = 0;
    stream.zfree  = 0;
    stream.opaque = 0;

    if(deflateInit(&stream, rate) != Z_OK)
    {
        sLog.outDebug("deflateInit failed.");
        return false;
    }

    uint8 *buffer;
    if(pCompressionBuffer != NULL)
    {
        pCompressionBuffer->resize(destsize);
        buffer = (uint8*)pCompressionBuffer->contents();
    } else buffer = new uint8[destsize];

    // set up stream pointers
    stream.next_out  = (Bytef*)buffer;
    stream.avail_out = destsize;
    stream.next_in   = (Bytef*)update_buffer;
    stream.avail_in  = size;

    // call the actual process
    if(deflate(&stream, Z_NO_FLUSH) != Z_OK ||
        stream.avail_in != 0)
    {
        sLog.outDebug("deflate failed.");
        if(pCompressionBuffer != NULL)
            pCompressionBuffer->clear();
        else delete [] buffer;
        return false;
    }

    // finish the deflate
    if(deflate(&stream, Z_FINISH) != Z_STREAM_END)
    {
        sLog.outDebug("deflate failed: did not end stream");
        if(pCompressionBuffer != NULL)
            pCompressionBuffer->clear();
        else delete [] buffer;
        return false;
    }

    // finish up
    if(deflateEnd(&stream) != Z_OK)
    {
        sLog.outDebug("deflateEnd failed.");
        if(pCompressionBuffer != NULL)
            pCompressionBuffer->clear();
        else delete [] buffer;
        return false;
    }

    // fill in the full size of the compressed stream
    WorldPacket data(SMSG_UPDATE_OBJECT|OPCODE_COMPRESSION_MASK, destsize);
    data << uint32(stream.total_in);
    data.append(buffer, uint32(stream.total_out));

    // send it
    SendPacket(&data);

    // cleanup memory
    if(pCompressionBuffer != NULL)
        pCompressionBuffer->clear();
    else delete [] buffer;
    return true;
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

bool Player::UpdateChatChannel(const char* areaName, AreaTableEntry *areaTable, ChatChannelDBC* entry, Channel* channel)
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

        for(ConstructDBCStorageIterator(ChatChannelDBC) itr = dbcChatChannels.begin(); itr != dbcChatChannels.end(); ++itr)
        {
            Channel *channel = NULL;
            ChatChannelDBC* entry = (*itr);
            if(m_channelsbyDBCID.find(entry->id) != m_channelsbyDBCID.end())
                channel = m_channelsbyDBCID.at(entry->id);
            if(UpdateChatChannel(areaName, areaTable, entry, channel))
                m_channelsbyDBCID.erase(entry->id);
        }
    }
    else
    {
        ChatChannelDBC* entry = dbcChatChannels.LookupEntry(dbcID);
        if(entry == NULL || !strlen(entry->pattern))
            return;

        std::map<uint32, Channel*>::iterator itr = m_channelsbyDBCID.find(dbcID);
        if(itr != m_channelsbyDBCID.end())
        {
            if(UpdateChatChannel(areaName, areaTable, entry, itr->second))
                m_channelsbyDBCID.erase(entry->id);
        }
        else if(itr != m_channelsbyDBCID.end() && itr->second != NULL)
            itr->second->Part(this, false, true);
        else if(itr == m_channelsbyDBCID.end())
        {
            if(!AllowChannelAtLocation(dbcID, areaTable))
                return;

            char name[255];
            sprintf(name, "%s", entry->pattern);
            if(entry->flags & 0x02)
                sprintf(name, entry->pattern, areaName);
            Channel *chn = channelmgr.GetCreateDBCChannel(name, this, entry->id);
            if(chn == NULL || chn->HasMember(this))
                return;
            chn->AttemptJoin(this, "");
        }
    }
}

void Player::SetGuildId(uint32 guildId)
{
    if(guildId)
        m_object.m_objType |= TYPEMASK_FLAG_IN_GUILD;
    else m_object.m_objType &= ~TYPEMASK_FLAG_IN_GUILD;
    SetUInt64Value(PLAYER_GUILDID, guildId ? MAKE_NEW_GUID(guildId, 0, HIGHGUID_TYPE_GUILD) : 0);
}

void Player::SendTradeUpdate()
{
    Player* pTarget = GetTradeTarget();
    if( pTarget == NULL )
        return;

    WorldPacket data( SMSG_TRADE_STATUS_EXTENDED, 100 );

    data << uint8(1);
    data << uint32(0x19);
    data << m_tradeSequence;
    data << m_tradeSequence++;
    data << mTradeGold << uint32(0);

    // Items
    for( uint32 Index = 0; Index < 7; Index++ )
    {
        data << uint8(Index);
        Item* pItem = mTradeItems[Index];
        if(pItem != 0)
        {
            ItemPrototype * pProto = pItem->GetProto();
            ASSERT(pProto != 0);

            data << pProto->ItemId;
            data << pProto->DisplayInfoID;
            data << pItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT);  // Amount          OK

            // Enchantment stuff
            data << uint32(0);                                    // unknown
            data << pItem->GetUInt64Value(ITEM_FIELD_GIFTCREATOR);  // gift creator  OK
            data << pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1);  // Item Enchantment OK
            data << uint32(0);                                    // unknown
            data << uint32(0);                                    // unknown
            data << uint32(0);                                    // unknown
            data << pItem->GetUInt64Value(ITEM_FIELD_CREATOR);    // item creator    OK
            data << pItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT);  // Spell Charges    OK

            data << pItem->GetItemRandomSuffixFactor();
            data << pItem->GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID);
            data << pProto->LockId;                              // lock ID       OK
            data << pItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
            data << pItem->GetUInt32Value(ITEM_FIELD_DURABILITY);
        }
        else
        {
            for(uint8 j = 0; j < 18; j++)
                data << uint32(0);
        }
    }

    pTarget->GetSession()->SendPacket(&data);
}

void Player::RequestDuel(Player* pTarget)
{
    if( sWorld.FunServerMall != -1 && GetAreaId() == (uint32)sWorld.FunServerMall )
        return;

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
    if( sWorld.FunServerMall != -1 && GetAreaId() == (uint32)sWorld.FunServerMall )
        return;

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
    if( sWorld.FunServerMall != -1 && GetAreaId() == (uint32)sWorld.FunServerMall )
        return;

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

    WorldPacket* Data2 = new WorldPacket(SMSG_DUEL_COMPLETE);
    data << uint8( 1 );
    SendPacket(Data2);

    Data2 = new WorldPacket(SMSG_DUEL_COMPLETE);
    data << uint8( 1 );
    DuelingWith->SendPacket(Data2);

    // Handle OnDuelFinished hook
    if ( WinCondition != 0 )
        sHookInterface.OnDuelFinished( DuelingWith, this );
    else
        sHookInterface.OnDuelFinished( this, DuelingWith );

    //Clear Duel Related Stuff

    GameObject* arbiter = m_mapMgr ? GetMapMgr()->GetGameObject(GUID_LOPART(GetUInt64Value(PLAYER_DUEL_ARBITER))) : NULL;

    if( arbiter != NULL )
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

void Player::EventTeleport(uint32 mapid, float x, float y, float z, float o = 0.0f)
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

    WorldPacket * data = sChatHandler.FillSystemMessageData(Message);
    m_session->SendPacket(data);
    delete data;
}

/*
const double BaseRating []= {
    2.5,//weapon_skill_ranged!!!!
    1.5,//defense=comba_r_1
    12,//dodge
    20,//parry=3
    5,//block=4
    10,//melee hit
    10,//ranged hit
    8,//spell hit=7
    14,//melee critical strike=8
    14,//ranged critical strike=9
    14,//spell critical strike=10
    0,//
    0,
    0,
    25,//resilience=14
    25,//resil .... meaning unknown
    25,//resil .... meaning unknown
    10,//MELEE_HASTE_RATING=17
    10,//RANGED_HASTE_RATING=18
    10,//spell_haste_rating = 19???
    2.5,//melee weapon skill==20
    2.5,//melee second hand=21

};
*/
float Player::CalcPercentForRating( uint32 index, uint32 rating )
{
    uint32 relative_index = index - (PLAYER_FIELD_COMBAT_RATING_1);
    uint32 reallevel = GetUInt32Value(UNIT_FIELD_LEVEL);
    uint32 level = reallevel > MAXIMUM_ATTAINABLE_LEVEL ? MAXIMUM_ATTAINABLE_LEVEL : reallevel;
    gtFloat * pDBCEntry = dbcCombatRating.LookupEntry( relative_index * 100 + level - 1 );
    float val = 1.0f;
    if( pDBCEntry != NULL )
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
    MapInfo * mi = LimitedMapInfoStorage.LookupEntry(MapID);
    if(!mi)
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
        if(mi->flags & WMI_INSTANCE_XPACK_01 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_01) && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
        {
            WorldPacket msg(SMSG_MOTD, 50);
            msg << uint32(1) << "You must have The Burning Crusade Expansion to access this content." << uint8(0);
            m_session->SendPacket(&msg);
            return false;
        }

        //Do we need WOTLK expansion?
        if(mi->flags & WMI_INSTANCE_XPACK_02 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
        {
            WorldPacket msg(SMSG_MOTD, 50);
            msg << uint32(1) << "You must have Wrath of the Lich King Expansion to access this content." << uint8(0);
            m_session->SendPacket(&msg);
            return false;
        }

        // Dismount
        Dismount();
    }

    //no flying outside new continents
    if((GetShapeShift() == FORM_FLIGHT || GetShapeShift() == FORM_SWIFT) && MapID != 530 && MapID != 571 )
        RemoveShapeShiftSpell(m_ShapeShifted);

    // make sure player does not drown when teleporting from under water
    if (m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
        m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;

    //all set...relocate
    _Relocate(MapID, vec, true, force_new_world, InstanceID);

    DelaySpeedHack(5000);
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
    m_sentTeleportPosition = vec;
    SetPosition(vec);
    ResetHeartbeatCoords();

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
    WorldObject* curObj;
    for (WorldObject::InRangeSet::iterator iter = GetInRangeSetBegin(); iter != GetInRangeSetEnd();)
    {
        curObj = *iter;
        iter++;
        if(curObj->IsPlayer())
        {
            pGroup = castPtr<Player>( curObj )->GetGroup();
            if( pGroup == NULL || pGroup != GetGroup())
                BuildFieldUpdatePacket( castPtr<Player>( curObj ), index, flag );
        }
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
    if( sWorld.FunServerMall != -1 && GetAreaId() == (uint32)sWorld.FunServerMall )
        return;

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

            Spell* spell = new Spell(this, info, true, NULL);
            spell->prepare(&targets);
        }
    }

    m_talentInterface.InitActiveSpec();

    if(!isDead())//only add aura's to the living (death aura set elsewhere)
    {
        for(std::list<LoginAura*>::iterator i = m_loginAuras.begin(); i != m_loginAuras.end();)
        {
            LoginAura *lAura = *i;
            i = m_loginAuras.erase(i);

            // this stuff REALLY needs to be fixed - Burlex
            SpellEntry* sp = dbcSpell.LookupEntry(lAura->id);

            //do not load auras that only exist while pet exist. We should recast these when pet is created anyway
            if ( sp->isSpellExpiringWithPet() )
                continue;

            Aura* a = new Aura(sp, lAura->dur, this, castPtr<Unit>(this));
            for(uint32 x = 0; x < 3; x++)
            {
                if(sp->Effect[x] == SPELL_EFFECT_APPLY_AURA)
                    a->AddMod(sp->EffectApplyAuraName[x], sp->EffectBasePoints[x]+1, sp->EffectMiscValue[x], sp->EffectMiscValueB[x], x);
            }
            AddAura(a);
            delete lAura;
        }

        // warrior has to have battle stance
        if( getClass() == WARRIOR && !HasAura(2457))
            CastSpell(castPtr<Unit>(this), dbcSpell.LookupEntry(2457), true);
    }
    m_loginAuras.clear();
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

    ResetHeartbeatCoords();
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

void Player::SaveAuras(std::stringstream& ss)
{
    // Add player auras
    m_AuraInterface.SaveAuras(ss);
}

void Player::SetShapeShift(uint8 ss)
{
    uint8 old_ss = GetByte( UNIT_FIELD_BYTES_2, 3 );
    SetByte( UNIT_FIELD_BYTES_2, 3, ss );

    //remove auras that we should not have
    m_AuraInterface.UpdateShapeShiftAuras(old_ss, ss);

    // apply any talents/spells we have that apply only in this form.
    std::set<uint32>::iterator itr;
    SpellEntry * sp;
    Spell* spe = NULL;
    SpellCastTargets t(GetGUID());

    for( itr = mSpells.begin(); itr != mSpells.end(); itr++ )
    {
        sp = dbcSpell.LookupEntry( *itr );
        if( sp == NULL)
            continue;
        if( sp->isSpellAppliedOnShapeshift() || sp->isPassiveSpell() )     // passive/talent
        {
            if( sp->RequiredShapeShift && ((uint32)1 << (ss-1)) & sp->RequiredShapeShift )
            {
                spe = new Spell( this, sp, true, NULL );
                spe->prepare( &t );
            }
        }
    }

    // now dummy-handler stupid hacky fixed shapeshift spells (leader of the pack, etc)
    for( itr = mShapeShiftSpells.begin(); itr != mShapeShiftSpells.end(); )
    {
        sp = dbcSpell.LookupEntry( *itr );
        if( sp == NULL)
            continue;
        ++itr;

        if( sp->RequiredShapeShift && ((uint32)1 << (ss-1)) & sp->RequiredShapeShift )
        {
            spe = (new Spell( this, sp, true, NULL ));
            spe->prepare( &t );
        }
    }

    // kill speedhack detection for 2 seconds (not needed with new code but bleh)
    DelaySpeedHack( 2000 );

    UpdateStats();
}

void Player::CalcDamage()
{

}

uint32 Player::GetMainMeleeDamage(uint32 AP_owerride)
{
    float min_dmg,max_dmg;
    float delta;
    float r;
    int ss = GetShapeShift();
/////////////////MAIN HAND
    float ap_bonus;
    if(AP_owerride)
        ap_bonus = AP_owerride/14000.0f;
    else ap_bonus = CalculateAttackPower()/14000.0f;
    delta = (float)GetUInt32Value( PLAYER_FIELD_MOD_DAMAGE_DONE_POS ) - (float)GetUInt32Value( PLAYER_FIELD_MOD_DAMAGE_DONE_NEG );
    if(IsInFeralForm())
    {
        uint32 lev = getLevel();
        if(ss == FORM_CAT)
            r = lev + delta + ap_bonus * 1000.0f;
        else
            r = lev + delta + ap_bonus * 2500.0f;
        min_dmg = r * 0.9f;
        max_dmg = r * 1.1f;
        return float2int32(std::max((min_dmg + max_dmg)/2.0f,0.0f));
    }
//////no druid ss
    Item* it = disarmed ? NULL : GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    uint32 speed = it ? it->GetProto()->Delay : 2000;

    float bonus = ap_bonus*speed;
    float tmp = 1.0f;
    std::map<uint32, WeaponModifier>::iterator i;
    for(i = damagedone.begin();i!=damagedone.end();++i)
    {
        if((i->second.wclass == 0xFFFFFFFF) || //any weapon
            (it && ((1 << it->GetProto()->SubClass) & i->second.subclass) ))
                tmp += i->second.value/100.0f;
    }

    r = delta+bonus+(it ? it->CalcMinDamage() : 0);
    r *= tmp;
    min_dmg = r * 0.9f;
    r = delta+bonus+(it ? it->CalcMaxDamage() : 0);
    r *= tmp;
    max_dmg = r * 1.1f;

    return float2int32(std::max((min_dmg + max_dmg)/2.0f,0.0f));
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

void Player::UpdateComboPoints()
{
    ByteBuffer buff;
    // check for overflow
    m_comboPoints = std::min(std::max(m_comboPoints, int8(0)), int8(5));
    if(!m_comboTarget.empty())
    {
        Unit* target = (m_mapMgr != NULL) ? m_mapMgr->GetUnit(m_comboTarget) : NULL;
        if(!target || target->isDead() || GetSelection() != m_comboTarget)
            buff.append<uint16>(0);
        else buff << m_comboTarget.asPacked() << int8(m_comboPoints);
    } else buff.append<uint16>(0);

    m_session->OutPacket(SMSG_UPDATE_COMBO_POINTS, buff.size(), buff.contents());
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

    ResetHeartbeatCoords();
    if( pTarget->IsPlayer() )
        castPtr<Player>(pTarget)->ResetHeartbeatCoords();

    m_CurrentCharm = pTarget;
    if(pTarget->GetTypeId() == TYPEID_UNIT)
    {
        // unit-only stuff.
        pTarget->DisableAI();
        pTarget->GetAIInterface()->StopMovement(0);
        pTarget->m_redirectSpellPackets = castPtr<Player>(this);
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

    if(pTarget->GetTypeId() == TYPEID_UNIT)
    {
        // unit-only stuff.
        pTarget->EnableAI();
        pTarget->m_redirectSpellPackets = NULL;
    }

    ResetHeartbeatCoords();
    DelaySpeedHack(5000);
    if( pTarget->IsPlayer() )
        castPtr<Player>(pTarget)->DelaySpeedHack(5000);

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

void Player::_AddSkillLine(uint32 SkillLine, uint32 Curr_sk, uint32 Max_sk)
{
    SkillLineEntry * CheckedSkill = dbcSkillLine.LookupEntry(SkillLine);
    if (!CheckedSkill) //skill doesn't exist, exit here
        return;

    uint32 customlvlcapskcap = 50+(sWorld.LevelCap_Custom_All*5);

    // force to be within limits
    Max_sk = Max_sk > customlvlcapskcap ? customlvlcapskcap : Max_sk;
    Curr_sk = Curr_sk > Max_sk ? Max_sk : Curr_sk < 1 ? 1 : Curr_sk ;

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
        inf.BonusValue = 0;
        m_skills.insert( std::make_pair( SkillLine, inf ) );
        _UpdateSkillFields();
    }

    //Add to proficiency
    if(const ItemProf * prof = GetProficiencyBySkill(SkillLine))
    {
        packetSMSG_SET_PROFICICENCY pr;
        pr.ItemClass = prof->itemclass;
        if(prof->itemclass == 4)
        {
            armor_proficiency |= prof->subclass;
            pr.Profinciency = armor_proficiency;
        }
        else
        {
            weapon_proficiency |= prof->subclass;
            pr.Profinciency = weapon_proficiency;
        }
        m_session->OutPacket( SMSG_SET_PROFICIENCY, sizeof( packetSMSG_SET_PROFICICENCY ), &pr );
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
            PlayerSkill skill = m_skillsByIndex.at(i);
            SetUInt16Value(PLAYER_SKILL_LINEID_0 + field, offset, skill.Skill->id);
            SetUInt16Value(PLAYER_SKILL_STEP_0 + field, offset, 0);
            SetUInt16Value(PLAYER_SKILL_RANK_0 + field, offset, skill.CurrentValue);
            SetUInt16Value(PLAYER_SKILL_MAX_RANK_0 + field, offset, skill.MaximumValue);
            SetUInt16Value(PLAYER_SKILL_MODIFIER_0 + field, offset, skill.BonusValue);
            SetUInt16Value(PLAYER_SKILL_TALENT_0 + field, offset, skill.BonusTalent);
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

bool Player::_HasSkillLine(uint32 SkillLine)
{
    return (m_skills.find(SkillLine) != m_skills.end());
}

void Player::_AdvanceSkillLine(uint32 SkillLine, uint32 Count /* = 1 */)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if(itr == m_skills.end())
    {
        /* Add it */
        _AddSkillLine(SkillLine, Count, getLevel() * 5);
        _UpdateMaxSkillCounts();
        sHookInterface.OnAdvanceSkillLine(this, SkillLine, Count);
    }
    else
    {
        uint32 curr_sk = itr->second.CurrentValue;
        itr->second.CurrentValue = std::min(curr_sk + Count,itr->second.MaximumValue);
        sHookInterface.OnAdvanceSkillLine(this, SkillLine, curr_sk);

        if (itr->second.CurrentValue != curr_sk)
            _UpdateSkillFields();
    }
}

uint32 Player::_GetSkillLineMax(uint32 SkillLine)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    return (itr == m_skills.end()) ? 0 : itr->second.MaximumValue;
}

uint32 Player::_GetSkillLineCurrent(uint32 SkillLine, bool IncludeBonus /* = true */)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if(itr == m_skills.end())
        return 0;

    return (IncludeBonus ? itr->second.CurrentValue + itr->second.BonusValue : itr->second.CurrentValue);
}

void Player::_RemoveSkillLine(uint32 SkillLine)
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
    uint32 new_max;
    if(m_skills.size())
    {
        for(SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); itr++)
        {
            if( itr->second.Skill->id == SKILL_LOCKPICKING )
                new_max = 5 * getLevel();
            else
            {
                switch(itr->second.Skill->type)
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

                    // default the rest to max = 1, so they won't mess up skill frame for player.
                default: new_max = 1; break;
                }
            }

            uint32 customlvlcapskcap = 50+(sWorld.LevelCap_Custom_All*5);

            //Update new max, forced to be within limits
            itr->second.MaximumValue = new_max > customlvlcapskcap ? customlvlcapskcap : new_max < 1 ? 1 : new_max;

            //Check if current is below nem max, if so, set new current to new max
            itr->second.CurrentValue = itr->second.CurrentValue > new_max ? new_max : itr->second.CurrentValue;
        }
        //Always update client to prevent cached data messing up things later.
        _UpdateSkillFields();
    }
}

void Player::_ModifySkillBonus(uint32 SkillLine, int32 Delta)
{
    SkillMap::iterator itr = m_skills.find(SkillLine);
    if(itr == m_skills.end())
        return;

    itr->second.BonusValue += Delta;
    _UpdateSkillFields();
}

void Player::_ModifySkillBonusByType(uint32 SkillType, int32 Delta)
{
    bool dirty = false;
    for(SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); itr++)
    {
        if(itr->second.Skill->type == SkillType)
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
        if(itr->second.Skill->type == SKILL_TYPE_LANGUAGE)
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
            m_skillsByIndex.insert(std::make_pair(sk.SkillPos, sk));
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

            if(en->type == SKILL_TYPE_LANGUAGE)
            {
                if(sk.Reset(itr->skillid))
                {
                    sk.MaximumValue = sk.CurrentValue = 300;
                    sk.SkillPos = GetFreeSkillPosition();
                    m_skills.insert( std::make_pair(itr->skillid, sk) );
                    m_skillsByIndex.insert(std::make_pair(sk.SkillPos, sk));
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

void Player::_AdvanceAllSkills(uint32 count, bool skipprof /* = false */, uint32 max /* = 0 */)
{
    bool dirty=false;
    for(SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end(); itr++)
    {
        if(itr->second.CurrentValue != itr->second.MaximumValue)
        {
            if((skipprof == true) && (itr->second.Skill->type == SKILL_TYPE_PROFESSION))
                continue;

            if(max != 0)
            {
                if(max > itr->second.MaximumValue)
                    max = itr->second.MaximumValue;

                if((itr->second.CurrentValue + count) >= max)
                    itr->second.CurrentValue = max;

                sHookInterface.OnAdvanceSkillLine(this, itr->second.Skill->id, itr->second.CurrentValue);

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

void Player::_ModifySkillMaximum(uint32 SkillLine, uint32 NewMax)
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

void Player::PartLFGChannel()
{
    Channel * pChannel = channelmgr.GetChannel("LookingForGroup", castPtr<Player>(this));
    if( pChannel == NULL )
        return;

    /*for(std::list<Channel*>::iterator itr = m_channels.begin(); itr != m_channels.end(); itr++)
    {
        if( (*itr) == pChannel )
        {
            pChannel->Part(castPtr<Player>(this));
            return;
        }
    }*/
    if( m_channels.find( pChannel->m_channelId ) == m_channels.end() )
        return;

    pChannel->Part( castPtr<Player>(this), false );
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
            SpellCastTargets targets( GetGUID() );
            Spell* spell = new Spell(castPtr<Player>(this), spellInfo ,true, NULL);    //we cast it as a proc spell, maybe we should not !
            spell->prepare(&targets);
        }
        if( spellInfo->isSpellCastOnPetOnSummon() )
        {
            m_AuraInterface.RemoveAllAuras( SpellID, GetGUID() ); //this is required since unit::addaura does not check for talent stacking
            SpellCastTargets targets( new_pet->GetGUID() );
            Spell* spell = new Spell(castPtr<Player>(this), spellInfo ,true, NULL);    //we cast it as a proc spell, maybe we should not !
            spell->prepare(&targets);
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
        Spell* spe = new Spell( castPtr<Player>(this), sp, true, NULL );
        SpellCastTargets t(GetGUID());
        spe->prepare( &t );
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

    ItemSpell* isp = &pProto->Spells[x];
    uint32 mstime = getMSTime();

    if( isp->CategoryCooldown > 0)
        _Cooldown_Add( COOLDOWN_TYPE_CATEGORY, isp->Category, isp->CategoryCooldown + mstime, isp->Id, pProto->ItemId );

    if( isp->Cooldown > 0 )
        _Cooldown_Add( COOLDOWN_TYPE_SPELL, isp->Id, isp->Cooldown + mstime, isp->Id, pProto->ItemId );
}

bool Player::Cooldown_CanCast(ItemPrototype * pProto, uint32 x)
{
    PlayerCooldownMap::iterator itr;
    ItemSpell* isp = &pProto->Spells[x];
    uint32 mstime = getMSTime();

    if( isp->Category )
    {
        itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find( isp->Category );
        if( itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end( ) )
        {
            if( mstime < itr->second.ExpireTime )
                return false;
            else
                m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase( itr );
        }
    }

    itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find( isp->Id );
    if( itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end( ) )
    {
        if( mstime < itr->second.ExpireTime )
            return false;
        else
            m_cooldownMap[COOLDOWN_TYPE_SPELL].erase( itr );
    }

    return true;
}

#define COOLDOWN_SKIP_SAVE_IF_MS_LESS_THAN 10000

void Player::_SavePlayerCooldowns(QueryBuffer * buf)
{
    m_lock.Acquire();
    PlayerCooldownMap::iterator itr;
    PlayerCooldownMap::iterator itr2;
    uint32 i;
    uint32 seconds;
    uint32 mstime = getMSTime();

    // clear them (this should be replaced with an update queue later)
    if( buf != NULL )
        buf->AddQuery("DELETE FROM playercooldowns WHERE player_guid = %u", GetLowGUID());        // 0 is guid always
    else CharacterDatabase.Execute("DELETE FROM playercooldowns WHERE player_guid = %u", GetLowGUID());        // 0 is guid always

    for( i = 0; i < NUM_COOLDOWN_TYPES; i++ )
    {
        itr = m_cooldownMap[i].begin( );
        for( ; itr != m_cooldownMap[i].end( ); )
        {
            itr2 = itr++;

            // expired ones - no point saving, nor keeping them around, wipe em
            if( mstime >= itr2->second.ExpireTime )
            {
                m_cooldownMap[i].erase( itr2 );
                continue;
            }

            // skip small cooldowns which will end up expiring by the time we log in anyway
            if( ( itr2->second.ExpireTime - mstime ) < COOLDOWN_SKIP_SAVE_IF_MS_LESS_THAN )
                continue;

            // work out the cooldown expire time in unix timestamp format
            // burlex's reason: 30 day overflow of 32bit integer, also
            // under windows we use GetTickCount() which is the system uptime, if we reboot
            // the server all these timestamps will appear to be messed up.

            seconds = (itr2->second.ExpireTime - mstime) / 1000;
            // this shouldn't ever be nonzero because of our check before, so no check needed

            if( buf != NULL )
                buf->AddQuery( "INSERT INTO playercooldowns VALUES(%u, %u, %u, %u, %u, %u)", GetLowGUID(), i, itr2->first, seconds + (uint32)UNIXTIME, itr2->second.SpellId, itr2->second.ItemId );
            else CharacterDatabase.Execute( "INSERT INTO playercooldowns VALUES(%u, %u, %u, %u, %u, %u)", GetLowGUID(), i, itr2->first, seconds + (uint32)UNIXTIME, itr2->second.SpellId, itr2->second.ItemId );
        }
    }
    m_lock.Release();
}

void Player::_LoadPlayerCooldowns(QueryResult * result)
{
    if( result == NULL )
        return;

    // we should only really call getMSTime() once to avoid user->system transitions, plus
    // the cost of calling a function for every cooldown the player has
    uint32 mstime = getMSTime();
    uint32 type;
    uint32 misc;
    uint32 rtime;
    uint32 realtime;
    uint32 itemid;
    uint32 spellid;
    PlayerCooldown cd;

    do
    {
        type = result->Fetch()[0].GetUInt32();
        misc = result->Fetch()[1].GetUInt32();
        rtime = result->Fetch()[2].GetUInt32();
        spellid = result->Fetch()[3].GetUInt32();
        itemid = result->Fetch()[4].GetUInt32();

        if( type >= NUM_COOLDOWN_TYPES )
            continue;

        // remember the cooldowns were saved in unix timestamp format for the reasons outlined above,
        // so restore them back to mstime upon loading

        if( (uint32)UNIXTIME > rtime )
            continue;

        rtime -= (uint32)UNIXTIME;

        if( rtime < 10 )
            continue;

        realtime = mstime + ( ( rtime ) * 1000 );

        // apply it back into cooldown map
        cd.ExpireTime = realtime;
        cd.ItemId = itemid;
        cd.SpellId = spellid;
        m_cooldownMap[type].insert( std::make_pair( misc, cd ) );

    } while ( result->NextRow( ) );
}

void Player::_SpeedhackCheck()
{
    if(!sWorld.antihack_speed || !sWorld.antihack_cheatengine)
        return;

    if(GetSession()->HasGMPermissions() && sWorld.no_antihack_on_gm)
        return;

    if(!m_isMoving || GetSession()->m_isFalling)
        return;

    if(GetTransportGuid() || !IsInWorld())
        return;

    if((m_special_state & UNIT_STATE_CONFUSE) || GetUInt64Value(UNIT_FIELD_CHARMEDBY))
        return;

    // this means the client is probably lagging. don't update the timestamp, don't do anything until we start to receive
    if( m_position == m_lastHeartbeatPosition && m_isMoving ) // packets again (give the poor laggers a chance to catch up)
        return;

    // simplified; just take the fastest speed. less chance of fuckups too
    float speed = (( m_FlyingAura || FlyCheat ) ? (m_runSpeed > m_flySpeed ? m_runSpeed : m_flySpeed) : m_runSpeed);
    if( m_swimSpeed > speed )
        speed = m_swimSpeed;

    if(!m_heartbeatDisable && !GetUInt64Value(UNIT_FIELD_CHARMEDBY) && !m_speedChangeInProgress )
    {
        // latency compensation a little
        if( sWorld.m_speedHackLatencyMultiplier > 0.0f )
            speed += (float(m_session->GetLatency()) / 100.0f) * sWorld.m_speedHackLatencyMultiplier;

        float distance = m_position.Distance2D( m_lastHeartbeatPosition );
        uint32 time_diff = m_lastMoveTime - m_startMoveTime;
        uint32 move_time = float2int32( ( distance / ( speed * 0.001f ) ) );
        int32 difference = time_diff - move_time;
        sLog.Debug("Player","SpeedhackCheck: speed=%f diff=%i dist=%f move=%u tdiff=%u", speed, difference, distance, move_time, time_diff );
        if( difference < sWorld.m_speedHackThreshold )
        {
            BroadcastMessage("Speedhack detected. Please contact an admin with the below information if you believe this is a false detection." );
            BroadcastMessage("%sSpeed: %f diff: %i dist: %f move: %u tdiff: %u\n", MSG_COLOR_WHITE, speed, difference, distance, move_time, time_diff );
            if( m_speedhackChances == 0 )
            {
                SetMovement( MOVE_ROOT, 1 );
                BroadcastMessage( "You will be disconnected in 10 seconds." );
                sWorld.LogCheater(GetSession(), "Speed hack detected! Distance: %i, Speed: %f, Move: %u, tdiff: %u", distance, speed, move_time, time_diff);
                if(m_bg)
                    m_bg->RemovePlayer(castPtr<Player>(this), false);

                sEventMgr.AddEvent(castPtr<Player>(this), &Player::_Disconnect, EVENT_PLAYER_KICK, 10000, 1, 0 );
                m_speedhackChances = 0;
            }
            else if (m_speedhackChances > 0 )
                m_speedhackChances--;
        }
    }
}

void Player::_Disconnect()
{
    m_session->Disconnect();
}

void Player::ResetSpeedHack()
{
    ResetHeartbeatCoords();
    m_heartbeatDisable = 0;
}

void Player::DelaySpeedHack(uint32 ms)
{
    uint32 t;
    m_heartbeatDisable = 1;

    if( event_GetTimeLeft( EVENT_PLAYER_RESET_HEARTBEAT, &t ) )
    {
        if( t > ms )        // dont override a slower reset
            return;

        // override it
        event_ModifyTimeAndTimeLeft( EVENT_PLAYER_RESET_HEARTBEAT, ms );
        return;
    }

    // add a new event
    sEventMgr.AddEvent(castPtr<Player>(this), &Player::ResetSpeedHack, EVENT_PLAYER_RESET_HEARTBEAT, ms, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );
}

/************************************************************************/
/* SOCIAL                                                               */
/************************************************************************/

void Player::Social_AddFriend(const char * name, const char * note)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    std::map<WoWGuid, char*>::iterator itr;
    PlayerInfo * info;

    // lookup the player
    info = objmgr.GetPlayerInfoByName(name);
    if( info == NULL )
    {
        data << uint8(FRIEND_NOT_FOUND);
        m_session->SendPacket(&data);
        return;
    }

    // team check
    if( info->team != m_playerInfo->team )
    {
        data << uint8(FRIEND_ENEMY) << uint64(info->guid);
        m_session->SendPacket(&data);
        return;
    }

    // are we ourselves?
    if( info == m_playerInfo )
    {
        data << uint8(FRIEND_SELF) << GetGUID();
        m_session->SendPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    itr = m_friends.find(info->guid);
    if( itr != m_friends.end() )
    {
        data << uint8(FRIEND_ALREADY) << info->guid;
        m_session->SendPacket(&data);
        m_socialLock.Release();
        return;
    }

    if( info->m_loggedInPlayer != NULL )
    {
        data << uint8(FRIEND_ADDED_ONLINE);
        data << info->guid;
        if( note != NULL )
            data << note;
        else data << uint8(0);

        data << info->m_loggedInPlayer->GetChatTag();
        data << info->m_loggedInPlayer->GetZoneId();
        data << info->lastLevel;
        data << uint32(info->_class);

        info->m_loggedInPlayer->m_socialLock.Acquire();
        info->m_loggedInPlayer->m_hasFriendList.insert( GetLowGUID() );
        info->m_loggedInPlayer->m_socialLock.Release();
    }
    else
    {
        data << uint8(FRIEND_ADDED_OFFLINE);
        data << info->guid;
    }

    if( note != NULL )
        m_friends.insert( std::make_pair(info->guid, strdup(note)) );
    else m_friends.insert( std::make_pair(info->guid, (char*)NULL) );

    m_socialLock.Release();
    m_session->SendPacket(&data);

    // dump into the db
    CharacterDatabase.Execute("INSERT INTO social_friends VALUES(%u, %u, \'%s\')", GetLowGUID(), info->guid.getLow(), note ? CharacterDatabase.EscapeString(std::string(note)).c_str() : "");
}

void Player::Social_RemoveFriend(WoWGuid guid)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    std::map<WoWGuid, char*>::iterator itr;

    // are we ourselves?
    if( guid == GetGUID() )
    {
        data << uint8(FRIEND_SELF) << GetGUID();
        m_session->SendPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    itr = m_friends.find(guid);
    if( itr != m_friends.end() )
    {
        if( itr->second != NULL )
            free(itr->second);

        m_friends.erase(itr);
    }

    data << uint8(FRIEND_REMOVED);
    data << uint64(guid);

    m_socialLock.Release();

    Player* pl = objmgr.GetPlayer( guid );
    if( pl != NULL )
    {
        pl->m_socialLock.Acquire();
        pl->m_hasFriendList.erase( GetLowGUID() );
        pl->m_socialLock.Release();
    }

    m_session->SendPacket(&data);

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u AND friend_guid = %u", GetLowGUID(), guid.getLow());
}

void Player::Social_SetNote(WoWGuid guid, const char * note)
{
    std::map<WoWGuid, char*>::iterator itr;

    m_socialLock.Acquire();
    itr = m_friends.find(guid);

    if( itr == m_friends.end() )
    {
        m_socialLock.Release();
        return;
    }

    if( itr->second != NULL )
        free(itr->second);

    if( note != NULL )
        itr->second = strdup( note );
    else
        itr->second = NULL;

    m_socialLock.Release();
    CharacterDatabase.Execute("UPDATE social_friends SET note = \'%s\' WHERE character_guid = %u AND friend_guid = %u",
        note ? CharacterDatabase.EscapeString(std::string(note)).c_str() : "", GetLowGUID(), guid);
}

void Player::Social_AddIgnore(const char * name)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    std::set<WoWGuid>::iterator itr;

    // lookup the player
    PlayerInfo *info = objmgr.GetPlayerInfoByName(name);
    if( info == NULL )
    {
        data << uint8(FRIEND_IGNORE_NOT_FOUND);
        m_session->SendPacket(&data);
        return;
    }

    // are we ourselves?
    if( info == m_playerInfo )
    {
        data << uint8(FRIEND_IGNORE_SELF) << GetGUID();
        m_session->SendPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    itr = m_ignores.find(info->guid);
    if( itr != m_ignores.end() )
    {
        data << uint8(FRIEND_IGNORE_ALREADY) << uint64(info->guid);
        m_session->SendPacket(&data);
        m_socialLock.Release();
        return;
    }

    data << uint8(FRIEND_IGNORE_ADDED);
    data << uint64(info->guid);

    m_ignores.insert( info->guid );

    m_socialLock.Release();
    m_session->SendPacket(&data);

    // dump into db
    CharacterDatabase.Execute("INSERT INTO social_ignores VALUES(%u, %u)", GetLowGUID(), info->guid);
}

void Player::Social_RemoveIgnore(WoWGuid guid)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    std::set<WoWGuid>::iterator itr;

    // are we ourselves?
    if( guid == GetGUID() )
    {
        data << uint8(FRIEND_IGNORE_SELF) << GetGUID();
        m_session->SendPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    itr = m_ignores.find(guid);
    if( itr != m_ignores.end() )
    {
        m_ignores.erase(itr);
    }

    data << uint8(FRIEND_IGNORE_REMOVED);
    data << uint64(guid);

    m_socialLock.Release();

    m_session->SendPacket(&data);

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u AND ignore_guid = %u", GetLowGUID(), guid.getLow());
}

bool Player::Social_IsIgnoring(PlayerInfo * m_info)
{
    bool res;
    m_socialLock.Acquire();
    if( m_ignores.find( m_info->guid ) == m_ignores.end() )
        res = false;
    else
        res = true;

    m_socialLock.Release();
    return res;
}

bool Player::Social_IsIgnoring(WoWGuid guid)
{
    bool res;
    m_socialLock.Acquire();
    if( m_ignores.find( guid ) == m_ignores.end() )
        res = false;
    else res = true;

    m_socialLock.Release();
    return res;
}

void Player::Social_TellOnlineStatus(bool online)
{
    if( m_hasFriendList.empty() )
        return;

    Player* pl;
    std::set<WoWGuid>::iterator itr;
    WorldPacket data(SMSG_FRIEND_STATUS, 22);
    if(online)
    {
        data << uint8( FRIEND_ONLINE ) << GetGUID() << uint8(GetChatTag());
        data << GetAreaId() << getLevel() << uint32(getClass());
    } else data << uint8( FRIEND_OFFLINE ) << GetGUID() << uint8( 0 );

    m_socialLock.Acquire();
    for( itr = m_hasFriendList.begin(); itr != m_hasFriendList.end(); itr++ )
    {
        pl = objmgr.GetPlayer(*itr);
        if( pl != NULL )
            pl->GetSession()->SendPacket(&data);
    }
    m_socialLock.Release();
}

void Player::Social_TellFriendsStatus()
{
    if( m_hasFriendList.empty() )
        return;

    Player* pl;
    std::set<WoWGuid>::iterator itr;
    WorldPacket data(SMSG_FRIEND_STATUS, 22);
    data << uint8( FRIEND_UNK7 ) << GetGUID() << uint8(GetChatTag());

    m_socialLock.Acquire();
    for( itr = m_hasFriendList.begin(); itr != m_hasFriendList.end(); itr++ )
    {
        pl = objmgr.GetPlayer(*itr);
        if( pl != NULL )
            pl->GetSession()->SendPacket(&data);
    }
    m_socialLock.Release();
}

void Player::Social_SendFriendList(uint32 flag)
{
    WorldPacket data(SMSG_CONTACT_LIST, 500);
    std::map<WoWGuid,char*>::iterator itr;
    std::set<WoWGuid>::iterator itr2;
    PlayerInfo* plr;

    m_socialLock.Acquire();

    data << flag;
    data << uint32( m_friends.size() + m_ignores.size() );
    for( itr = m_friends.begin(); itr != m_friends.end(); itr++ )
    {
        // guid
        data << uint64( itr->first );

        // friend/ignore flag.
        // 0x01 - friend
        // 0x02 - ignore
        data << uint32( 0x01 );

        // player note
        if( itr->second != NULL && strlen(itr->second))
            data << itr->second;
        else data << uint8(0);

        // online/offline flag
        plr = objmgr.GetPlayerInfo( itr->first );
        if(plr->m_loggedInPlayer != NULL)
        {
            data << plr->m_loggedInPlayer->GetChatTag();
            data << plr->m_loggedInPlayer->GetZoneId();
            data << plr->m_loggedInPlayer->getLevel();
            data << uint32( plr->m_loggedInPlayer->getClass() );
        } else data << uint8( 0 );// << float((UNIXTIME - plr->lastOnline) / 86400.0);
    }

    for( itr2 = m_ignores.begin(); itr2 != m_ignores.end(); itr2++ )
    {
        // guid
        data << uint64( (*itr2) );

        // ignore flag - 2
        data << uint32( 0x02 );

        // no note
        data << uint8(0);
    }

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
    CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnPlayerLootGen )( this, pCorpse );
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
    CharTitlesEntry *entry = dbcCharTitles.LookupEntry(title);
    if(entry == NULL || HasKnownTitleByIndex(entry->index))
        return;

    if(set == false && entry->index == GetUInt32Value(PLAYER_CHOSEN_TITLE))
        SetUInt32Value(PLAYER_CHOSEN_TITLE, 0);

    uint32 field = PLAYER__FIELD_KNOWN_TITLES+(entry->index / 32);
    uint32 flag = uint32(1 << (entry->index % 32));

    if( set ) SetFlag(field, flag);
    else RemoveFlag(field, flag);

    WorldPacket data( SMSG_TITLE_EARNED, 8 );
    data << uint32( entry->index ) << uint32( set ? 1 : 0 );
    m_session->SendPacket( &data );

    if(set && GetUInt32Value(PLAYER_CHOSEN_TITLE) == 0)
        SetUInt32Value(PLAYER_CHOSEN_TITLE, entry->index);
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
    std::set<uint32>::iterator itr = m_finishedQuests.begin();
    for(; itr != m_finishedQuests.end(); itr++)
    {
        Quest * pQuest = sQuestMgr.GetQuestPointer( *itr );
        if(!pQuest || !pQuest->qst_zone_id)
            continue;

        GetAchievementInterface()->HandleAchievementCriteriaCompleteQuestsInZone( pQuest->qst_zone_id );
    }
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
    ItemInterface *Ii = GetItemInterface();

    uint32 playertotalitemlevel = 0;

    for(int8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_TABARD; i++)
    {
        uint32 previoustil = playertotalitemlevel;
        Item* item = Ii->GetInventoryItem(i);

        if(!item)
            continue;

        playertotalitemlevel = previoustil + item->GetProto()->ItemLevel;
    }
    return playertotalitemlevel;
}

uint32 Player::GetAverageItemLevel(bool skipmissing)
{
    ItemInterface *Ii = GetItemInterface();

    uint8 itemcount = 0;
    uint32 playertotalitemlevel = 0;

    for(int8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_TABARD; i++)
    {
        if(i == EQUIPMENT_SLOT_BODY)
            continue;

        uint32 previoustil = playertotalitemlevel;
        Item* item = Ii->GetInventoryItem(i);
        if(!skipmissing)
            itemcount++;

        if(!item || !item->GetProto())
            continue;

        if(skipmissing)
            itemcount++;
        playertotalitemlevel = previoustil + item->GetProto()->ItemLevel;
    }
    return playertotalitemlevel/itemcount;
}

void Player::_LoadEquipmentSets(QueryResult *result)
{

}

void Player::SendEquipmentSets()
{
/*  uint32 count = 0;
    WorldPacket data(SMSG_EQUIPMENT_SET_LIST, 4);
    size_t count_pos = data.wpos();
    data << uint32(count);
    for(count)
    {
        data << WoWGuid(setguid);
        data << Index;
        data << Name;
        data << IconName;
        for(uint32 i = 0; i < EQUIPMENT_SLOT_END; i++)
        {
            if(itr->second.Items[i].GetOldGuid())
                data << uint32(Ignore/Equip) << WoWGuid(ItemGuid);
            else
                data << uint32(0) << uint8(0);
        }

        ++count;
    }
    GetSession()->SendPacket(&data);*/
}

void Player::SetEquipmentSet(uint32 index, EquipmentSet eqset)
{

}

void Player::DeleteEquipmentSet(uint64 setGuid)
{

}

void Player::_SaveEquipmentSets(QueryBuffer* buff)
{

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

void Player::AddArenaPoints( uint32 arenapoints )
{
    m_arenaPoints += arenapoints;
    HonorHandler::RecalculateHonorFields(this);
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
                if(!GetItemInterface()->AddItemToFreeSlot(item))
                {
                    item->Destruct();
                } else GetSession()->SendItemPushResult(item, false, true, false, true, GetItemInterface()->LastSearchItemBagSlot(), GetItemInterface()->LastSearchItemSlot(),1);
            }
        }
    }

    if(qst->srcitem && qst->srcitem != qst->receive_items[0])
    {
        if(Item* item = objmgr.CreateItem( qst->srcitem, this ))
        {
            item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, qst->srcitemcount ? qst->srcitemcount : 1);
            if(!GetItemInterface()->AddItemToFreeSlot(item))
                item->Destruct();
        }
    }
    CALL_QUESTSCRIPT_EVENT(Id, OnQuestStart)(this, qle);

    sQuestMgr.OnQuestAccepted(this,qst,NULL);

    sHookInterface.OnQuestAccept(this, qst, NULL);
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
