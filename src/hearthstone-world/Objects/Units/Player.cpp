/***
 * Demonstrike Core
 */

#include "StdAfx.h"

UpdateMask Player::m_visibleUpdateMask;

#define COLLISION_MOUNT_CHECK_INTERVAL 1000

static const uint32 DKNodesMask[12] = {4294967295,4093640703,830406655,0,33570816,1310944,3250593812,73752,896,67111952,0,0};//all old continents are available to DK's by default.

Player::Player( uint32 guid )
{
#ifdef SHAREDPTR_DEBUGMODE
    printf("Player::Player()\n");
#endif
    m_runemask = 0x3F;
    m_bgRatedQueue = false;
    m_massSummonEnabled = false;
    m_objectTypeId = TYPEID_PLAYER;
    m_valuesCount = PLAYER_END;
    m_uint32Values = _fields;
    memset(m_uint32Values, 0, (PLAYER_END*sizeof(uint32)));
    m_updateMask.SetCount(PLAYER_END);
    SetUInt32Value( OBJECT_FIELD_TYPE,TYPEMASK_PLAYER|TYPEMASK_UNIT|TYPEMASK_OBJECT);
    SetUInt64Value( OBJECT_FIELD_GUID, MAKE_NEW_GUID(guid, 0, HIGHGUID_TYPE_PLAYER));
    m_wowGuid.Init(GetGUID());
    m_deathRuneMasteryChance = 0;
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
    m_ItemInterface                 = new ItemInterface(this);
    m_achievementInterface          = new AchievementInterface(this);
    m_bgSlot                        = 0;
    mAngerManagement                = false;
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
    m_Summon                        = NULLPET;
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
    m_healthfromspell               = 0;
    m_manafromspell                 = 0;
    m_healthfromitems               = 0;
    m_manafromitems                 = 0;
    m_talentresettimes              = 0;
    ForceSaved                      = false;
    m_nextSave                      = 120000;
    m_currentSpell                  = NULLSPELL;
    m_resurrectHealth               = 0;
    m_resurrectMana                 = 0;
    m_GroupInviter                  = 0;
    Lfgcomment                      = "";
    m_flyHackChances                = 5;
    m_WaterWalkTimer                = 0;
    m_lastWarnCounter               = 0;
    for(int i = 0; i < 3; i++)
    {
        LfgType[i]      = 0;
        LfgDungeonId[i] = 0;
    }
    m_Autojoin                  = false;
    m_AutoAddMem                = false;
    LfmDungeonId                = 0;
    LfmType                     = 0;
    for(int32 i = 0; i < NUM_MECHANIC; i++)
        MechanicDurationPctMod[i] = 1.0f;
    m_invitersGuid                  = 0;
    forget                          = 0;
    m_currentMovement               = MOVE_UNROOT;
    m_isGmInvisible                 = false;
    m_invitersGuid                  = 0;
    ResetTradeVariables();
    DuelingWith                     = NULLPLR;
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
    for(uint32 i = 0; i < 2; i++)
    {
        m_bgQueueType[i] = 0;
        m_bgQueueInstanceId[i] = 0;
        m_bgIsQueued[i] = false;
        m_bgQueueTime[i] = 0;
    }
    m_bg                            = NULLBATTLEGROUND;
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
    m_GM_SelectedGO                 = NULLGOB;

    for(uint8 x = 0;x < 7; ++x)
    {
        FlatResistanceModifierPos[x] = 0;
        FlatResistanceModifierNeg[x] = 0;
        BaseResistanceModPctPos[x] = 0;
        BaseResistanceModPctNeg[x] = 0;
        ResistanceModPctPos[x] = 0;
        ResistanceModPctNeg[x] = 0;
        SpellDelayResist[x] = 0;
        m_casted_amount[x] = 0;
    }

    for(uint8 x = 0; x < 5; ++x)
    {
        FlatStatModPos[x] = 0;
        FlatStatModNeg[x] = 0;
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
    m_retainedrage                  = 0;
    DetectedRange                   = 0;
    m_targetIcon                    = 0;
    m_MountSpellId                  = 0;
    bHasBindDialogOpen              = false;
    m_CurrentCharm                  = NULLUNIT;
    m_CurrentTransporter            = NULLTRANSPORT;
    m_SummonedObject                = NULLOBJ;
    m_currentLoot                   = (uint64)NULL;
    pctReputationMod                = 0;
    roll                            = 0;
    mUpdateDataCount                = 0;
    bUpdateDataBuffer.reserve(65000);
    mOutOfRangeIds.reserve(1000);
    mOutOfRangeIdCount              = 0;
    bProcessPending                 = false;

    for(int i = 0; i < QUEST_LOG_COUNT; i++)
        m_questlog[i] = NULL;

    CurrentGossipMenu               = NULL;
    ResetHeartbeatCoords();
    m_actionsDirty                  = false;
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
    myCorpse                        = NULLCORPSE;
    blinked                         = false;
    blinktimer                      = getMSTime();
    m_speedhackChances              = 3;
    m_explorationTimer              = getMSTime();
    linkTarget                      = NULLUNIT;
    stack_cheat                     = false;
    triggerpass_cheat               = false;
    m_pvpTimer                      = 0;
    m_globalCooldown                = 0;
    m_lastHonorResetTime            = 0;
    memset(&mActions, 0, PLAYER_ACTION_BUTTON_COUNT*2*sizeof(ActionButton));
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
    SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, 0.0f);
    SetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER, 0.0f);
    UpdateLastSpeeds();
    m_resist_critical[0] = m_resist_critical[1] = 0;
    for (uint8 x = 0; x < 3; ++x)
    {
        m_resist_hit[x] = 0;
        m_skipCastCheck[x] = 0;
        m_castFilter[x] = 0;
    }
    for(int i = 0; i < 6; i++)
    {
        m_runes[i] = baseRunes[i];
    }
    m_maxTalentPoints               = 0;
    m_talentActiveSpec              = 0;
    m_talentSpecsCount              = 1;
    JudgementSpell                  = 0;
    ok_to_remove                    = false;
    trigger_on_stun                 = 0;
    trigger_on_stun_chance          = 100;
    m_modphyscritdmgPCT             = 0;
    m_RootedCritChanceBonus         = 0;
    m_Illumination_amount           = 0;
    m_ModInterrMRegenPCT            = 0;
    m_ModInterrMRegen               = 0;
    m_rap_mod_pct                   = 0;
    m_modblockabsorbvalue           = 0;
    m_modblockvaluefromspells       = 0;
    Damageshield_amount             = 0.0f;
    m_summoner                      = NULLOBJ;
    m_summonInstanceId              = m_summonMapId = 0;
    m_lastMoveType                  = 0;
    m_tempSummon                    = NULLCREATURE;
    m_spellcomboPoints              = 0;
    memset( &m_pendingBattleground, 0, sizeof(CBattleground*) * 2);
    m_deathVision                   = false;
    m_retainComboPoints             = false;
    last_heal_spell                 = NULL;
    m_playerInfo                    = NULL;
    m_sentTeleportPosition.ChangeCoords(999999.0f,999999.0f,999999.0f);
    m_speedChangeCounter            = 1;
    memset(&m_bgScore,0,sizeof(BGScore));
    m_arenaPoints                   = 0;
    memset(&m_spellIndexTypeTargets, 0, sizeof(uint64)*NUM_SPELL_TYPE_INDEX);
    m_base_runSpeed                 = m_runSpeed;
    m_base_walkSpeed                = m_walkSpeed;
    m_arenateaminviteguid           = 0;
    m_arenaPoints                   = 0;
    m_honorRolloverTime             = 0;
    hearth_of_wild_pct              = 0;
    raidgrouponlysent               = false;
    m_setwaterwalk                  = false;
    m_areaSpiritHealer_guid         = 0;
    m_CurrentTaxiPath               = NULL;
    m_setflycheat                   = false;
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
    m_vampiricEmbrace           = 0;
    m_magnetAura                = NULLAURA;
    m_lastMoveTime              = 0;
    m_lastMovementPacketTimestamp = 0;
    m_cheatEngineChances        = 2;
    m_mageInvisibility          = false;
    mWeakenedSoul               = false;
    mForbearance                = false;
    mExhaustion                 = false;
    mHypothermia                = false;
    mSated                      = false;
    mAvengingWrath              = true;
    m_bgFlagIneligible          = 0;
    m_moltenFuryDamageIncreasePct = 0;
    m_insigniaTaken             = true;
    m_BeastMaster               = false;

    m_wratings.clear();
    m_QuestGOInProgress.clear();
    m_removequests.clear();
    m_finishedQuests.clear();
    m_finishedDailyQuests.clear();
    quest_spells.clear();
    quest_mobs.clear();
    loginauras.clear();
    OnMeleeAuras.clear();
    m_Pets.clear();
    m_itemsets.clear();
    m_channels.clear();
    m_channelsbyDBCID.clear();
    m_visibleObjects.clear();
    mSpells.clear();
    areaphases.clear();

    for(uint32 i = 0; i < 21; i++)
        m_WeaponSubClassDamagePct[i] = 1.0f;

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
    sEventMgr.RemoveEvents(TO_PLAYER(this));

    if(!ok_to_remove)
    {
        printf("Player deleted from non-logoutplayer!\n");
        OutputCrashLogLine("Player deleted from non-logoutplayer!");
        CThreadPool::Suicide();
    }

    objmgr.RemovePlayer(this);

    if(m_session)
    {
        m_session->SetPlayer(NULLPLR);
    }

    Player* pTarget = objmgr.GetPlayer(GetInviter());
    if(pTarget)
        pTarget->SetInviter(0);
    pTarget = NULLPLR;

    if(mTradeTarget != 0)
    {
        pTarget = GetTradeTarget();
        if(pTarget)
            pTarget->mTradeTarget = 0;
        pTarget = NULLPLR;
    }

    if(m_Summon)
    {
        m_Summon->Dismiss(true);
        m_Summon->ClearPetOwner();
    }

    if (m_GM_SelectedGO)
        m_GM_SelectedGO = NULLGOB;

    if (m_SummonedObject)
    {
        m_SummonedObject->Destruct();
        m_SummonedObject = NULLOBJ;
    }

    if( m_mailBox )
        delete m_mailBox;

    mTradeTarget = 0;

    if( DuelingWith != NULL )
        DuelingWith->DuelingWith = NULLPLR;
    DuelingWith = NULLPLR;

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
    m_objectTypeId = TYPEID_UNUSED;

    if(m_playerInfo)
        m_playerInfo->m_loggedInPlayer=NULLPLR;

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
        myCorpse = NULLCORPSE;

    if(linkTarget)
        linkTarget = NULLUNIT;

    m_wratings.clear();
    m_QuestGOInProgress.clear();
    m_removequests.clear();
    m_finishedQuests.clear();
    quest_spells.clear();
    quest_mobs.clear();
    loginauras.clear();
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

    // Set Starting stats for char
    SetUInt32Value(UNIT_FIELD_STAT0, info->strength );
    SetUInt32Value(UNIT_FIELD_STAT1, info->ability );
    SetUInt32Value(UNIT_FIELD_STAT2, info->stamina );
    SetUInt32Value(UNIT_FIELD_STAT3, info->intellect );
    SetUInt32Value(UNIT_FIELD_STAT4, info->spirit );

    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    SetUInt32Value(UNIT_FIELD_POWER3, info->focus );
    SetUInt32Value(UNIT_FIELD_POWER4, info->energy );
    SetUInt32Value(UNIT_FIELD_POWER6, getClass() == DEATHKNIGHT ? 8 : 0);
    SetUInt32Value(UNIT_FIELD_POWER7, 0 );

    SetUInt32Value(UNIT_FIELD_MAXPOWER2, info->rage );
    SetUInt32Value(UNIT_FIELD_MAXPOWER3, info->focus );
    SetUInt32Value(UNIT_FIELD_MAXPOWER4, info->energy );
    SetUInt32Value(UNIT_FIELD_MAXPOWER6, 8);
    SetUInt32Value(UNIT_FIELD_MAXPOWER7, 1000 );
    SetUInt32Value(PLAYER_FIELD_COINAGE, sWorld.StartGold);

    if(sWorld.StartLevel > 1)
    {
        SetUInt32Value(UNIT_FIELD_LEVEL, sWorld.StartLevel);
        if(class_ == DEATHKNIGHT)
        {
            if(sWorld.StartLevel < 55)
                SetUInt32Value(UNIT_FIELD_LEVEL, 55);
            else SetUInt32Value(UNIT_FIELD_LEVEL, sWorld.StartLevel-55);
        }
    }
    else SetUInt32Value(UNIT_FIELD_LEVEL, uint32(class_ != DEATHKNIGHT ? 1 : 55));

    InitGlyphSlots();
    InitGlyphsForLevel();
    // lookup level information
    uint32 lvl = GetUInt32Value(UNIT_FIELD_LEVEL);
    lvlinfo = objmgr.GetLevelInfo(getRace(), getClass(), lvl);
    ApplyLevelInfo(lvl);

    //THIS IS NEEDED
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, lvlinfo ? lvlinfo->BaseHP : info->health);
    SetUInt32Value(UNIT_FIELD_BASE_MANA, lvlinfo ? lvlinfo->BaseMana : info->mana );
    SetUInt32Value(UNIT_FIELD_HEALTH, lvlinfo ? lvlinfo->HP : info->health);
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, lvlinfo ? lvlinfo->HP : info->health);
    SetUInt32Value(UNIT_FIELD_POWER1, lvlinfo ? lvlinfo->Mana : info->mana );
    SetUInt32Value(UNIT_FIELD_MAXPOWER1, lvlinfo ? lvlinfo->Mana : info->mana );
    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, info->factiontemplate );

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

    SetUInt32Value(UNIT_FIELD_ATTACK_POWER, info->attackpower );
    SetByte(PLAYER_BYTES, 0, skin);
    SetByte(PLAYER_BYTES, 1, face);
    SetByte(PLAYER_BYTES, 2, hairStyle);
    SetByte(PLAYER_BYTES, 3, hairColor);
    SetByte(PLAYER_BYTES_2, 0, facialHair);
    SetByte(PLAYER_BYTES_2, 3, 0x02); // No Recruit a friend flag
    SetByte(PLAYER_BYTES_3, 0, gender);

    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, lvlinfo ? lvlinfo->XPToNextLevel : 400);
    SetUInt32Value(PLAYER_FIELD_MAX_LEVEL, sWorld.GetMaxLevel(TO_PLAYER(this)));

    for(uint32 x = 0; x < 7; x++)
        SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+x, 1.f);
    SetFloatValue(PLAYER_FIELD_MOD_HEALING_PCT, 1.f);
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
            else
                _AddSkillLine(se->id, ss->currentval, ss->maxval);
        }
    }
    _UpdateMaxSkillCounts();

    _InitialReputation();

    // Add actionbars
    for(std::list<CreateInfo_ActionBarStruct>::iterator itr = info->actionbars.begin();itr!=info->actionbars.end();itr++)
    {
        setAction(itr->button, itr->action, itr->type, 0);
        setAction(itr->button, itr->action, itr->type, 1);
    }

    if( GetSession()->HasGMPermissions() && sWorld.gm_force_robes )
    {
        // Force GM robes on GM's except 'az' status (if set to 1 in world.conf)
        if( strstr(GetSession()->GetPermissions(), "az") == NULL)
        {
            //We need to dupe this
            PlayerCreateInfo *GMinfo = NULL;
            GMinfo = new PlayerCreateInfo;
            memcpy(GMinfo,info, sizeof(info));

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
        }
        else
            EquipInit(info);
    }
    else
        EquipInit(info);

    sHookInterface.OnCharacterCreate(TO_PLAYER(this));

    load_health = m_uint32Values[UNIT_FIELD_HEALTH];
    load_mana = m_uint32Values[UNIT_FIELD_POWER1];
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
                Item* item = objmgr.CreateItem((*is).protoid,TO_PLAYER(this));
                if(item != NULL)
                {
                    item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, (*is).amount);
                    if((*is).slot < INVENTORY_SLOT_BAG_END)
                    {
                        if( !GetItemInterface()->SafeAddItem(item, INVENTORY_SLOT_NOT_SET, (*is).slot) )
                        {
                            item->DeleteMe();
                            item = NULLITEM;
                        }
                    }
                    else
                    {
                        if( !GetItemInterface()->AddItemToFreeSlot(item) )
                        {
                            item->DeleteMe();
                            item = NULLITEM;
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

    if (GetMapMgr())
    {
        if( GetMapMgr()->CanUseCollision(this) && mstime >= m_mountCheckTimer )
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
                m_AuraInterface.RemoveAllAurasWithAttributes(ATTRIBUTES_ONLY_OUTDOORS);
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

    sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_PLAYER_TAXI_INTERPOLATE);

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
        if(m_currentSpell->GetSpellProto()->IsChannelSpell()) // this is a channeled spell - ignore the attack event
            return;

        m_currentSpell->cancel();
        setAttackTimer(500, offhand);
        return;
    }

    if( IsFeared() || IsStunned() )
        return;

    Unit* pVictim = NULLUNIT;
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

    Unit* pVictim = NULLUNIT;
    if(!IsInWorld())
    {
        m_CurrentCharm=NULLUNIT;
        sEventMgr.RemoveEvents(TO_PLAYER(this),EVENT_PLAYER_CHARM_ATTACK);
        return;
    }

    if(m_curSelection == 0)
    {
        sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_PLAYER_CHARM_ATTACK);
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
            sEventMgr.ModifyEventTimeLeft(TO_PLAYER(this), EVENT_PLAYER_CHARM_ATTACK, 100);
        }
        else if(!m_CurrentCharm->isTargetInFront(pVictim))
        {
            if(m_AttackMsgTimer == 0)
            {
                m_session->OutPacket(SMSG_ATTACKSWING_BADFACING);
                m_AttackMsgTimer = 2000;        // 2 sec till next msg.
            }
            // Shorten, so there isnt a delay when the client IS in the right position.
            sEventMgr.ModifyEventTimeLeft(TO_PLAYER(this), EVENT_PLAYER_CHARM_ATTACK, 100);
        }
        else
        {
            //if(pVictim->GetTypeId() == TYPEID_UNIT)
            //  pVictim->GetAIInterface()->StopMovement(5000);

            //pvp timeout reset
            /*if(pVictim->IsPlayer())
            {
                if( TO_PLAYER( pVictim )->DuelingWith == NULL)//Dueling doesn't trigger PVP
                    TO_PLAYER( pVictim )->PvPTimeoutUpdate(false); //update targets timer

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
                Spell* spell = NULLSPELL;
                spell = (new Spell(m_CurrentCharm,spellInfo,true,NULLAURA));
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
        sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_PLAYER_CHARM_ATTACK);

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

        sHookInterface.OnZone(TO_PLAYER(this), m_zoneId, m_oldZone);
        CALL_INSTANCE_SCRIPT_EVENT( GetMapMgr(), OnZoneChange )( TO_PLAYER(this), m_zoneId, m_oldZone );

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
        Unit* pUnit = (GetSelection() == 0) ? NULLUNIT : (m_mapMgr ? m_mapMgr->GetUnit(GetSelection()) : NULLUNIT);
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
            if(target && !sFactionSystem.isAttackable(this, target) && target != TO_PLAYER(this))
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
        sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_PLAYER_TAXI_DISMOUNT);

    if(!IS_INSTANCE(GetMapId()) && !sEventMgr.HasEvent(TO_PLAYER(this),EVENT_PLAYER_FORCED_RESURECT)) //Should never be true
        sEventMgr.AddEvent(TO_PLAYER(this),&Player::EventRepopRequestedPlayer,EVENT_PLAYER_FORCED_RESURECT,PLAYER_FORCED_RESURECT_INTERVAL,1,0); //in case he forgets to release spirit (afk or something)
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

    int32 newxp = m_uint32Values[PLAYER_XP] + xp;
    int32 nextlevelxp = lvlinfo->XPToNextLevel;
    uint32 level = m_uint32Values[UNIT_FIELD_LEVEL];
    LevelInfo * li;
    bool levelup = false;

    while(newxp >= nextlevelxp && newxp > 0)
    {
        ++level;
        li = objmgr.GetLevelInfo(getRace(), getClass(), level);
        if(li == NULL)
            break;
        newxp -= nextlevelxp;
        nextlevelxp = li->XPToNextLevel;
        levelup = true;

        if(level >= GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
            break;
    }

    if(level > GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
        level = GetUInt32Value(PLAYER_FIELD_MAX_LEVEL);

    if(levelup)
    {
        m_playedtime[0] = 0; //Reset the "Current level played time"

        LevelInfo * oldlevel = lvlinfo;
        lvlinfo = objmgr.GetLevelInfo(getRace(), getClass(), level);

        ApplyLevelInfo(level);

        // Generate Level Info Packet and Send to client
        SendLevelupInfo(
            level,
            lvlinfo->HP - oldlevel->HP,
            lvlinfo->Mana - oldlevel->Mana,
            lvlinfo->Stat[0] - oldlevel->Stat[0],
            lvlinfo->Stat[1] - oldlevel->Stat[1],
            lvlinfo->Stat[2] - oldlevel->Stat[2],
            lvlinfo->Stat[3] - oldlevel->Stat[3],
            lvlinfo->Stat[4] - oldlevel->Stat[4]);

        if( getClass() == WARLOCK && GetSummon() && GetSummon()->IsInWorld() && GetSummon()->isAlive())
        {
            GetSummon()->ModUnsigned32Value( UNIT_FIELD_LEVEL, 1 );
            GetSummon()->ApplyStatsForLevel();
        }
        InitGlyphsForLevel();

        _UpdateMaxSkillCounts();

        // ScriptMgr hook for OnPostLevelUp
        sHookInterface.OnPostLevelUp(this);
    }

    // Set the update bit
    SetUInt32Value(PLAYER_XP, newxp);

    HandleProc(PROC_ON_GAIN_EXPIERIENCE, NULL, TO_PLAYER(this), NULL);
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

void Player::BuildPlayerTalentsInfo(WorldPacket *data)
{
    if(m_talentSpecsCount > 2)
        m_talentSpecsCount = 2; // Hack fix

    *data << uint32(0); // Unspent talents
    *data << uint8(m_talentSpecsCount);
    *data << uint8(m_talentActiveSpec);
    if(m_talentSpecsCount)
    {
        for(uint8 s = 0; s < m_talentSpecsCount; ++s)
        {
            PlayerSpec spec = m_specs[s];
            *data << uint32(0); // Spec tree
            // Send Talents
            *data << uint8(spec.talents.size());
            if(!spec.talents.empty())
            {
                std::map<uint32, uint8>::iterator itr;
                for(itr = spec.talents.begin(); itr != spec.talents.end(); itr++)
                {
                    *data << uint32(itr->first);    // TalentId
                    *data << uint8(itr->second);    // TalentRank
                }
            }

            // Send Glyph info
            *data << uint8(GLYPHS_COUNT);
            for(uint8 i = 0; i < GLYPHS_COUNT; i++)
                *data << uint16(spec.glyphs[i]);
        }
    }
}

void Player::BuildPetTalentsInfo(WorldPacket *data)
{
    Pet *pPet = GetSummon();
    if(pPet == NULL || !pPet->IsPet())
        return;

    *data << uint32(pPet->GetUnspentPetTalentPoints());
    *data << uint8(pPet->m_talents.size());

    for(PetTalentMap::iterator itr = pPet->m_talents.begin(); itr != pPet->m_talents.end(); itr++)
    {
        *data << uint32(itr->first);
        *data << uint8(itr->second);
    }
}

void Player::smsg_TalentsInfo(bool pet)
{
    WorldPacket data(SMSG_TALENTS_INFO, 1000);
    data << uint8(pet ? 1 : 0);
    if(pet)
        BuildPetTalentsInfo(&data);
    else    // initialize sending all info
        BuildPlayerTalentsInfo(&data);

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
    if(m_Summon && m_Summon->IsInWorld() && m_Summon->GetPetOwner() == TO_PLAYER(this)) // update PlayerPets array with current pet's info
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
        map<uint32, set<uint32> >::iterator itr = SummonSpells.begin();
        stringstream ss;
        ss << "INSERT INTO playersummonspells VALUES ";
        bool first = true;
        for(; itr != SummonSpells.end(); itr++)
        {
            set<uint32>::iterator it = itr->second.begin();
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
    map<uint32, set<uint32> >::iterator itr = SummonSpells.find(Entry);
    if(itr == SummonSpells.end())
        SummonSpells[Entry].insert(SpellID);
    else
    {
        set<uint32>::iterator it3;
        for(set<uint32>::iterator it2 = itr->second.begin(); it2 != itr->second.end();)
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
    map<uint32, set<uint32> >::iterator itr = SummonSpells.find(Entry);
    if(itr != SummonSpells.end())
    {
        itr->second.erase(SpellID);
        if(itr->second.size() == 0)
            SummonSpells.erase(itr);
    }
}

set<uint32>* Player::GetSummonSpells(uint32 Entry)
{
    map<uint32, set<uint32> >::iterator itr = SummonSpells.find(Entry);
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
        PlayerPet* pet = NULLPET;
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
            }
            else
                iActivePet = pet->number;
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
        PlayerPet* pet = NULLPET;
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
        m_Summon = NULLPET;
    }

    // Crow: Should be that it recasts summon spell, but without cost.
    Pet* pPet = objmgr.CreatePet();
    pPet->SetInstanceID(GetInstanceID());
    pPet->LoadFromDB(TO_PLAYER(this), itr->second);
    if( IsPvPFlagged() )
        pPet->SetPvPFlag();
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
        Object::_SetCreateBits(updateMask, target);
    }
    else
    {
        for(uint32 index = 0; index < m_valuesCount; index++)
        {
            if(m_uint32Values[index] != 0 && Player::m_visibleUpdateMask.GetBit(index))
                updateMask->SetBit(index);
        }
    }
}


void Player::_SetUpdateBits(UpdateMask *updateMask, Player* target) const
{
    if(target == this)
    {
        Object::_SetUpdateBits(updateMask, target);
    }
    else
    {
        Object::_SetUpdateBits(updateMask, target);
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
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER2);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER3);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER4);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER5);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER6);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER7);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER8);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER9);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_POWER10);

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXHEALTH);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER1);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER2);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER3);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER4);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER5);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER6);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER7);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER8);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER9);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_MAXPOWER10);

    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_LEVEL);
    Player::m_visibleUpdateMask.SetBit(UNIT_FIELD_FACTIONTEMPLATE);
    Player::m_visibleUpdateMask.SetBit(UNIT_VIRTUAL_ITEM_SLOT_ID);
    Player::m_visibleUpdateMask.SetBit(UNIT_VIRTUAL_ITEM_SLOT_ID+1);
    Player::m_visibleUpdateMask.SetBit(UNIT_VIRTUAL_ITEM_SLOT_ID+2);
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

    for(uint32 i = PLAYER_QUEST_LOG_1_1; i <= PLAYER_QUEST_LOG_50_1; i += 5)
        Player::m_visibleUpdateMask.SetBit(i);

    for(uint8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        uint32 offset = i * PLAYER_VISIBLE_ITEM_LENGTH;

        // item entry
        Player::m_visibleUpdateMask.SetBit(PLAYER_VISIBLE_ITEM_1_ENTRYID + offset);
        // enchant
        Player::m_visibleUpdateMask.SetBit(PLAYER_VISIBLE_ITEM_1_ENCHANTMENT + offset);
    }

    Player::m_visibleUpdateMask.SetBit(PLAYER_CHOSEN_TITLE);
}


void Player::DestroyForPlayer( Player* target, bool anim )
{
    Object::DestroyForPlayer( target, anim );
    if(GetItemInterface())
        GetItemInterface()->m_DestroyForPlayer(target);
}

#define IS_ARENA(x) ( (x) >= BATTLEGROUND_ARENA_2V2 && (x) <= BATTLEGROUND_ARENA_5V5 )

void Player::SaveToDB(bool bNewCharacter /* =false */)
{
    bool in_arena = false;
    QueryBuffer * buf = NULL;
    if(!bNewCharacter)
        buf = new QueryBuffer;

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

    if(m_uint32Values[UNIT_FIELD_FACTIONTEMPLATE] != info->factiontemplate)
        ss << m_uint32Values[UNIT_FIELD_FACTIONTEMPLATE] << ",";
    else
        ss << "0,";

    ss << uint32(getLevel()) << ","
    << uint32(m_XPoff ? 1 : 0) << ","
    << m_uint32Values[PLAYER_XP] << ","

    // dump exploration data
    << "'";

    for(uint32 i = 0; i < 144; i++)
        ss << m_uint32Values[PLAYER_EXPLORED_ZONES_1 + i] << ",";

    ss << "','0', "; //skip saving oldstyle skills, just fill with 0

    uint32 player_flags = m_uint32Values[PLAYER_FLAGS];
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

    ss << m_uint32Values[PLAYER_FIELD_WATCHED_FACTION_INDEX] << ","
    << m_uint32Values[PLAYER_CHOSEN_TITLE] << ","
    << GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES) << ","
    << GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES1) << ","
    << GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES2) << ","
    << m_uint32Values[PLAYER_FIELD_COINAGE] << ","
    << uint32(0) << ","
    << m_uint32Values[PLAYER_CHARACTER_POINTS] << ","
    << m_maxTalentPoints << ","
    << load_health << ","
    << load_mana << ","
    << uint32(GetPVPRank()) << ","
    << m_uint32Values[PLAYER_BYTES] << ","
    << m_uint32Values[PLAYER_BYTES_2] << ","
    << player_flags << ","
    << m_uint32Values[PLAYER_FIELD_BYTES] << ",";

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

    for(uint8 i = 0; i < 14; i++ )
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

    << m_talentresettimes      << ", "
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

    // Add player action bars
    for(uint32 i = 0; i < PLAYER_ACTION_BUTTON_COUNT; i++)
        ss << uint32(0) << "," << uint32(mActions[0][i].PackedData) << ",";
    for(uint32 i = 0; i < PLAYER_ACTION_BUTTON_COUNT; i++)
        ss << uint32(1) << "," << uint32(mActions[1][i].PackedData) << ",";
    ss << "','";

    if(!bNewCharacter)
        SaveAuras(ss);

    //ss << LoadAuras;
    ss << "','";

    // Add player finished quests
    set<uint32>::iterator fq = m_finishedQuests.begin();
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

    ss << uint32(m_talentActiveSpec) << ", ";
    ss << uint32(m_talentSpecsCount) << ", ";

    ss << "0, 0)";  // Reset for talents and position

    if(bNewCharacter)
        CharacterDatabase.WaitExecuteNA(ss.str().c_str());
    else
        buf->AddQueryStr(ss.str());

    //Save Other related player stuff
    sHookInterface.OnPlayerSaveToDB(this, buf);

    // Skills
    if(sWorld.DisableBufferSaving)
        _SaveSkillsToDB(NULL);
    else
        _SaveSkillsToDB(buf);

    // Talents
    if(sWorld.DisableBufferSaving)
        _SaveTalentsToDB(NULL);
    else
        _SaveTalentsToDB(buf);

    // Spells
    if(sWorld.DisableBufferSaving)
        _SaveSpellsToDB(NULL);
    else
        _SaveSpellsToDB(buf);

    // Equipment Sets
    if(sWorld.DisableBufferSaving)
        _SaveEquipmentSets(NULL);
    else
        _SaveEquipmentSets(buf);

    // Area Phase Info
    if(sWorld.DisableBufferSaving)
        _SaveAreaPhaseInfo(NULL);
    else
        _SaveAreaPhaseInfo(buf);

    // Glyphs
    if(sWorld.DisableBufferSaving)
        _SaveGlyphsToDB(NULL);
    else
        _SaveGlyphsToDB(buf);

    // Inventory
    if(sWorld.DisableBufferSaving)
        GetItemInterface()->mSaveItemsToDatabase(bNewCharacter, NULL);
    else
        GetItemInterface()->mSaveItemsToDatabase(bNewCharacter, buf);

    // save quest progress
    if(sWorld.DisableBufferSaving)
        _SaveQuestLogEntry(NULL);
    else
        _SaveQuestLogEntry(buf);

    // GM Ticket
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetGUID());
    if(ticket != NULL)
        objmgr.SaveGMTicket(ticket, buf);

    // Cooldown Items
    if(sWorld.DisableBufferSaving)
        _SavePlayerCooldowns( NULL );
    else
        _SavePlayerCooldowns( buf );

    // Pets
    if(getClass() == HUNTER || getClass() == WARLOCK)
    {
        if(sWorld.DisableBufferSaving)
        {
            _SavePet(NULL);
            _SavePetSpells(NULL);
        }
        else
        {
            _SavePet(buf);
            _SavePetSpells(buf);
        }
    }

    // Achievements
    if(sWorld.DisableBufferSaving)
        GetAchievementInterface()->SaveToDB( NULL );
    else
        GetAchievementInterface()->SaveToDB( buf );

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

void Player::_LoadGlyphs(QueryResult * result)
{
    // init with 0s just in case
    for(uint8 s = 0; s < MAX_SPEC_COUNT; s++)
    {
        for(uint32 i=0; i < GLYPHS_COUNT; i++)
        {
            m_specs[s].glyphs[i] = 0;
        }
    }
    // Load info from DB
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint8 spec = fields[1].GetInt8();
            if(spec >= MAX_SPEC_COUNT)
            {
                sLog.outDebug("Out of range spec number [%d] for player with GUID [%d] in playerglyphs",
                    spec, fields[0].GetUInt32());
                continue;
            }
            for(uint32 i=0; i < 6; i++)
            {
                m_specs[spec].glyphs[i] = fields[2 + i].GetUInt16();
            }
        } while(result->NextRow());
    }
}

void Player::_SaveGlyphsToDB(QueryBuffer * buf)
{
    bool empty = true;
    for(uint8 s = 0; s < m_talentSpecsCount; s++)
    {
        for(uint32 i=0; i < GLYPHS_COUNT; i++)
        {
            if(m_specs[s].glyphs[i] != 0)
            {
                empty = false;
                break;
            }
        }
    }
    if(empty)
        return; // nothing to save

    m_lock.Acquire();
    for(uint8 s = 0; s < m_talentSpecsCount; s++)
    {
        std::stringstream ss;
        ss << "REPLACE INTO playerglyphs (guid, spec, glyph1, glyph2, glyph3, glyph4, glyph5, glyph6) VALUES "
            << "(" << GetLowGUID() << ","
            << uint32(s) << ",";
        for(uint32 i = 0; i < 6; i++)
        {
            if(i != 0) ss << ", ";
            ss << uint32(m_specs[s].glyphs[i]);
        }
        ss << ")";

        if(buf == NULL)
            CharacterDatabase.Execute(ss.str().c_str());
        else
            buf->AddQueryStr(ss.str());
    }
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

void Player::_LoadTalents(QueryResult * result)
{
    // Load info from DB
    uint32 talentId;
    uint8 talentRank;
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint8 spec = fields[0].GetInt8();
            if(spec >= MAX_SPEC_COUNT)
            {
                sLog.outDebug("Out of range spec number [%d] for player with GUID [%d] in playertalents",
                    spec, GetLowGUID());
                continue;
            }
            talentId = fields[1].GetUInt32();
            talentRank = fields[2].GetUInt8();
            m_specs[spec].talents.insert(make_pair(talentId, talentRank));
        } while(result->NextRow());
    }
}

void Player::_SaveTalentsToDB(QueryBuffer * buf)
{
    m_lock.Acquire();
    // delete old talents first
    if(buf == NULL)
        CharacterDatabase.Execute("DELETE FROM playertalents WHERE guid = %u", GetLowGUID() );
    else
        buf->AddQuery("DELETE FROM playertalents WHERE guid = %u", GetLowGUID() );

    for(uint8 s = 0; s < m_talentSpecsCount; s++)
    {
        if(s > MAX_SPEC_COUNT)
            break;
        std::map<uint32, uint8> *talents = &m_specs[s].talents;
        if(talents->size())
        {
            bool first = true;
            std::stringstream ss;
            ss << "INSERT INTO playertalents VALUES ";
            std::map<uint32, uint8>::iterator itr;
            for(itr = talents->begin(); itr != talents->end(); itr++)
            {
                if(first)
                    first = false;
                else
                    ss << ",";

                ss << "(" << GetLowGUID() << "," << uint32(s) << "," << itr->first << "," << uint32(itr->second) << ")";
            }

            if(buf == NULL)
                CharacterDatabase.Execute(ss.str().c_str());
            else
                buf->AddQueryStr(ss.str());
        }
    }
    m_lock.Release();
}

void Player::_SaveQuestLogEntry(QueryBuffer * buf)
{
    m_lock.Acquire();
    stringstream ss;
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
        m_session->m_loggingInPlayer = NULLPLR;
    }

    ok_to_remove = true;
    Destruct();
}

bool Player::LoadFromDB(uint32 guid)
{
    AsyncQuery * q = new AsyncQuery( new SQLClassCallbackP0<Player>(TO_PLAYER(this), &Player::LoadFromDBProc) );
    q->AddQuery("SELECT * FROM characters WHERE guid=%u AND forced_rename_pending = 0",guid);
    q->AddQuery("SELECT cooldown_type, cooldown_misc, cooldown_expire_time, cooldown_spellid, cooldown_itemid FROM playercooldowns WHERE player_guid=%u", guid);
    q->AddQuery("SELECT * FROM questlog WHERE player_guid=%u",guid);
    q->AddQuery("SELECT * FROM playeritems WHERE ownerguid=%u ORDER BY containerslot ASC", guid);
    q->AddQuery("SELECT * FROM playerpets WHERE ownerguid=%u ORDER BY petnumber", guid);
    q->AddQuery("SELECT * FROM playersummonspells where ownerguid=%u ORDER BY entryid", guid);
    q->AddQuery("SELECT * FROM mailbox WHERE player_guid = %u", guid);

    // social
    q->AddQuery("SELECT friend_guid, note FROM social_friends WHERE character_guid = %u", guid);
    q->AddQuery("SELECT character_guid FROM social_friends WHERE friend_guid = %u", guid);
    q->AddQuery("SELECT ignore_guid FROM social_ignores WHERE character_guid = %u", guid);

    //Achievements
    q->AddQuery("SELECT * from achievements WHERE player = %u", guid);

    //skills
    q->AddQuery("SELECT * FROM playerskills WHERE player_guid = %u AND type <> %u ORDER BY skill_id ASC, currentlvl DESC", guid,SKILL_TYPE_LANGUAGE ); //load skill, skip languages

    //pet action bar
    q->AddQuery("SELECT * FROM playerpetactionbar WHERE ownerguid=%u ORDER BY petnumber", guid);

    //Talents
    q->AddQuery("SELECT spec, tid, rank FROM playertalents WHERE guid = %u", guid);

    //Glyphs
    q->AddQuery("SELECT * FROM playerglyphs WHERE guid = %u", guid);

    //Spells
    q->AddQuery("SELECT spellid FROM playerspells WHERE guid = %u", guid);

    //Equipmentsets
    q->AddQuery("SELECT * FROM equipmentsets WHERE guid = %u", guid);

    //Area Phase Info
    q->AddQuery("SELECT * FROM playerphaseinfo WHERE guid = %u", guid);

    // queue it!
    m_uint32Values[OBJECT_FIELD_GUID] = guid;
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

    SetGuildId(m_playerInfo->GuildId);
    SetGuildRank(m_playerInfo->GuildRank);

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

    // level dependant taxi node
    SetTaximaskNode( 213, plvl >= 68 ? false : true );  //Add 213 (Shattered Sun Staging Area) if lvl >=68

    // obtain level/stats information
    lvlinfo = objmgr.GetLevelInfo(getRace(), getClass(), getLevel());

    CalculateBaseStats();

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

    QueryResult *checkskills = results[11].result;
    if(checkskills)
    {
        _LoadSkills(results[11].result);
        field_index++;
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
                            m_skills.insert(make_pair(v1, sk));

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
    SetUInt64Value(PLAYER_FIELD_COINAGE, get_next_field.GetUInt32());
    get_next_field;
    SetUInt32Value(PLAYER_CHARACTER_POINTS, get_next_field.GetUInt32());
    m_maxTalentPoints                                   = get_next_field.GetUInt16();
    load_health                                         = get_next_field.GetUInt32();
    load_mana                                           = get_next_field.GetUInt32();
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
    for(uint32 i = 0; i < 5; i++)
        CalcStat(i);
    for(uint32 x = 0; x < 7; x++)
        SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+x, 1.f);

    // Normal processing...
//  UpdateMaxSkills();
    UpdateStats();
    //UpdateChances();

    // Initialize 'normal' fields
    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    SetUInt32Value(UNIT_FIELD_POWER3, info->focus);
    SetUInt32Value(UNIT_FIELD_POWER4, info->energy );
    SetUInt32Value(UNIT_FIELD_POWER6, getClass() == DEATHKNIGHT ? 8 : 0);
    SetUInt32Value(UNIT_FIELD_POWER7, 0);
    SetUInt32Value(UNIT_FIELD_MAXPOWER2, info->rage );
    SetUInt32Value(UNIT_FIELD_MAXPOWER3, info->focus );
    SetUInt32Value(UNIT_FIELD_MAXPOWER4, info->energy );
    SetUInt32Value(UNIT_FIELD_MAXPOWER6, 8);
    SetUInt32Value(UNIT_FIELD_MAXPOWER7, 1000 );
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

    m_talentresettimes = get_next_field.GetUInt32();
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
    GetMovementInfo()->SetTransportData(uint64(transGuid ? (uint64(HIGHGUID_TYPE_TRANSPORTER)<<32 | uint64(transGuid)) : 0), get_next_field.GetFloat(), get_next_field.GetFloat(), get_next_field.GetFloat(), 0.0f, 0);

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

        rep = NULL;
        rep = new FactionReputation;
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
    start = (char*)get_next_field.GetString();
    if(!strlen(start))
    {   // Reset our action bars
        for(std::list<CreateInfo_ActionBarStruct>::iterator itr = info->actionbars.begin(); itr != info->actionbars.end(); itr++)
        {
            setAction(itr->button, itr->action, itr->type, 0);
            setAction(itr->button, itr->action, itr->type, 1);
        }
    }
    else
    {
        Counter = 0;
        while(Counter < PLAYER_ACTION_BUTTON_COUNT*2)
        {
            end = strchr(start,',');
            if(!end)
                break;
            *end = 0;
            uint8 spec = (uint8)atol(start);
            start = end +1;

            end = strchr(start,',');
            if(!end)
                break;
            *end = 0;
            mActions[spec][(Counter-(spec*PLAYER_ACTION_BUTTON_COUNT))].PackedData = (uint32)atol(start);
            start = end +1;
            Counter++;
        }
    }

    //LoadAuras = get_next_field.GetString();
    start = (char*)get_next_field.GetString();//buff;
    do
    {
        end = strchr(start,',');
        if(!end)
            break;
        *end=0;
        LoginAura la;
        la.id = atol(start);
        start = end +1;
        end = strchr(start,',');
        if(!end)
            break;
        *end=0;
        la.dur = atol(start);
        start = end +1;
        if(la.id != 43869 && la.id != 43958)
            loginauras.push_back(la);
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
    InitGlyphSlots();
    InitGlyphsForLevel();

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

    m_talentActiveSpec = get_next_field.GetUInt32();
    m_talentSpecsCount = get_next_field.GetUInt32();
    if(m_talentSpecsCount > MAX_SPEC_COUNT)
        m_talentSpecsCount = MAX_SPEC_COUNT;
    if(m_talentActiveSpec >= m_talentSpecsCount )
        m_talentActiveSpec = 0;

    bool needTalentReset = get_next_field.GetBool();
    if( needTalentReset )
        Reset_Talents(true);

    bool NeedsPositionReset = get_next_field.GetBool();
    if( NeedsPositionReset )
        EjectFromInstance();

#undef get_next_field

    // load properties
    _LoadTalents(results[13].result);
    _LoadGlyphs(results[14].result);
    _LoadSpells(results[15].result);
    _LoadEquipmentSets(results[16].result);
    _LoadAreaPhaseInfo(results[17].result);
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
                m_friends.insert( make_pair( fields[0].GetUInt32(), strdup(fields[1].GetString()) ) );
            else
                m_friends.insert( make_pair( fields[0].GetUInt32(), (char*)NULL) );

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

    // Set correct maximum level
    uint32 maxLevel = sWorld.GetMaxLevel(this);
    SetUInt32Value(PLAYER_FIELD_MAX_LEVEL, maxLevel);

    SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER); // enables automatic power regen
    m_session->FullLogin(this);
    if(m_session)
        m_session->m_loggingInPlayer = NULLPLR;

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

    uint32 oskillline = sp->skilline;
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
            spell = NULL;
            if((spell = dbcSpell.LookupEntry(*itr)) != NULL)
            {
                if(spell->skilline == oskillline)
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

bool Player::HasTalent(uint8 spec, uint32 talentid)
{
    return m_specs[spec].talents.find(talentid) != m_specs[spec].talents.end();
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
        baseindex = PLAYER_QUEST_LOG_1_1 + (i * 5);
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
            entry->Init(quest, TO_PLAYER(this), slot);
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
    m_setflycheat = false;

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

    // If we join an invalid instance and get booted out, TO_PLAYER(this) will prevent our stats from doubling :P
    if(IsInWorld())
        return;

    m_beingPushed = true;
    Object::AddToWorld();

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
    m_setflycheat = false;
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

    // If we join an invalid instance and get booted out, TO_PLAYER(this) will prevent our stats from doubling :P
    if(IsInWorld())
        return;

    m_beingPushed = true;
    Object::AddToWorld(pMapMgr);

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
    ProcessPendingUpdates(NULL, NULL);

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
    data << GetNewGUID();
    m_AuraInterface.BuildAuraUpdateAllPacket(&data);
    SendPacket(&data);

    if(m_FirstLogin)
    {
        sEventMgr.AddEvent(this, &Player::FullHPMP, EVENT_PLAYER_FULL_HPMP, 200, 0, 0);
        sHookInterface.OnFirstEnterWorld(TO_PLAYER(this));
        m_FirstLogin = false;
    }

    // send world states
    if( m_mapMgr != NULL )
        m_mapMgr->GetStateManager().SendWorldStates(TO_PLAYER(this));

    // execute some of zeh hooks
    sHookInterface.OnEnterWorld(TO_PLAYER(this));
    sHookInterface.OnZone(TO_PLAYER(this), m_zoneId, 0);
    CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnZoneChange )( TO_PLAYER(this), m_zoneId, 0 );
    CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnPlayerEnter )( TO_PLAYER(this) );

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
    sWeatherMgr.SendWeather(TO_PLAYER(this));

    if( load_health > 0 )
    {
        if( load_health > m_uint32Values[UNIT_FIELD_MAXHEALTH] )
            SetUInt32Value(UNIT_FIELD_HEALTH, m_uint32Values[UNIT_FIELD_MAXHEALTH] );
        else
            SetUInt32Value(UNIT_FIELD_HEALTH, load_health);
    }

    if( load_mana > m_uint32Values[UNIT_FIELD_MAXPOWER1] )
        SetUInt32Value( UNIT_FIELD_POWER1, m_uint32Values[UNIT_FIELD_MAXPOWER1] );
    else
        SetUInt32Value(UNIT_FIELD_POWER1, load_mana);

    if( m_mapMgr != NULL && m_mapMgr->m_battleground != NULL && m_bg != m_mapMgr->m_battleground )
    {
        m_bg = m_mapMgr->m_battleground;
        m_bg->PortPlayer( TO_PLAYER(this), true );
    }

    if( m_bg != NULL && m_mapMgr != NULL )
        m_bg->OnPlayerPushed( TO_PLAYER(this) );

    z_axisposition = 0.0f;
    m_changingMaps = false;

    SendPowerUpdate();

    if(m_mapMgr != NULL)
    {
        MapInfo* info = GetMapMgr()->GetMapInfo();
        if(info != NULL && (info->phasehorde != 0 && info->phasealliance != 0 ))
        {
            if(GetSession()->HasGMPermissions())
                SetPhaseMask(info->phasehorde | info->phasealliance, false);
            else if(GetTeam())
                SetPhaseMask(info->phasehorde, false);
            else
                SetPhaseMask(info->phasealliance, false);
        }

        if(info != NULL && (info->phasehorde == 0 && info->phasealliance == 0 ))
            if(GetPhaseMask() == 1)
                SetPhaseMask(GetPhaseForArea(GetAreaId()), false);
    }

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

void Player::SendObjectUpdate(uint64 guid)
{
    uint32 count = 1;
    WorldPacket data(SMSG_UPDATE_OBJECT, 200);
    data << uint16(GetMapId());
    data << count;
    if(guid == GetGUID())
    {
        count += Object::BuildCreateUpdateBlockForPlayer(&data, this);
    }
    else if(IsInWorld())
    {
        Object* obj = GetMapMgr()->_GetObject(guid);
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

    printf("Sending update with size %u type %s %s\n", data.size(), (guid == GetGUID() ? "Player" : "Non Player"), (IsInWorld() ? "Is in world" : "Out of world"));
    data.put<uint32>(2, count);
    // send uncompressed because it's specified
    m_session->SendPacket(&data);
    ProcessPendingUpdates(NULL, NULL);
}

void Player::ResetHeartbeatCoords()
{
    m_lastHeartbeatPosition = m_position;
    if( m_isMoving )
        m_startMoveTime = m_lastMoveTime;
    else
        m_startMoveTime = 0;

    m_cheatEngineChances = 2;
    //_lastHeartbeatT = getMSTime();
}

void Player::RemoveFromWorld()
{
    EndDuel( 0 );

    if( m_CurrentCharm && m_CurrentCharm != GetVehicle() )
        UnPossess();

    if( m_uint32Values[UNIT_FIELD_CHARMEDBY] != 0 && IsInWorld() )
    {
        Player* charmer = m_mapMgr->GetPlayer(m_uint32Values[UNIT_FIELD_CHARMEDBY]);
        if( charmer != NULL )
            charmer->UnPossess();
    }

    m_resurrectHealth = 0;
    m_resurrectMana = 0;
    resurrector = 0;

    // just in case
    m_uint32Values[UNIT_FIELD_FLAGS] &= ~(UNIT_FLAG_SKINNABLE);

    //Cancel any forced teleport pending.
    if(raidgrouponlysent)
    {
        event_RemoveEvents(EVENT_PLAYER_EJECT_FROM_INSTANCE);
        raidgrouponlysent = false;
    }

    load_health = m_uint32Values[UNIT_FIELD_HEALTH];
    load_mana = m_uint32Values[UNIT_FIELD_POWER1];

    sHookInterface.OnPlayerChangeArea(this, 0, 0, GetAreaId());
    CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnChangeArea )( this, 0, 0, GetAreaId() );

    m_mapMgr->GetStateManager().ClearWorldStates(this);

    if(m_bg)
    {
        m_bg->RemovePlayer(TO_PLAYER(this), true);
        m_bg = NULLBATTLEGROUND;
    }

    // Cancel trade if it's active.
    Player* pTarget;
    if(mTradeTarget != 0)
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
        if(m_SummonedObject->GetInstanceID() != GetInstanceID())
        {
            sEventMgr.AddEvent(m_SummonedObject, &Object::Destruct, EVENT_GAMEOBJECT_EXPIRE, 100, 1,0);
        }
        else
        {
            if(m_SummonedObject->GetTypeId() != TYPEID_PLAYER)
            {
                if(m_SummonedObject->IsInWorld())
                {
                    m_SummonedObject->RemoveFromWorld(true);
                }
                delete m_SummonedObject;
            }
        }
        m_SummonedObject = NULLOBJ;
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
        m_CurrentTransporter->RemovePlayer(TO_PLAYER(this));
        m_CurrentTransporter = NULLTRANSPORT;
        GetMovementInfo()->ClearTransportData();
    }

    if( GetVehicle() )
    {
        GetVehicle()->RemovePassenger(TO_PLAYER(this));
        SetVehicle(NULLVEHICLE);
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
                            Spell* spell = new Spell( this, info, true, NULLAURA );
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

    // Armor
    if( proto->Armor )
    {
        if( apply )
        {
            BaseResistance[RESISTANCE_ARMOR]+= proto->Armor;
        }
        else
        {
            BaseResistance[RESISTANCE_ARMOR] -= proto->Armor;
        }
        CalcResistance(RESISTANCE_ARMOR);
    }

    /*if(proto->ScalingStatsEntry != 0) // This item is an Heirloom, we need to calculate it differently.
    {
        // This is Dfighter's code for Heirlooms, modified slightly. Danke Dfighter.
        ScalingStatDistributionEntry *ssdrow = dbcScalingStatDistribution.LookupEntry( proto->ScalingStatsEntry );
        ScalingStatValuesEntry *ssvrow = dbcScalingStatValues.LookupEntry(getLevel() > 80 ? 80 : getLevel());

        int i = 0;
        int32 StatValue;
        for(i = 0; ssdrow->StatMod[i] != -1; i++)
        {
            uint32 StatType = ssdrow->StatMod[i];
            StatValue = (ssdrow->Modifier[i])/10000;
            ModifyBonuses(StatType, (apply ? StatValue : -StatValue));
        }

        if((proto->ScalingStatsFlag & 32768) && i < 10)
        {
            StatValue = (ssdrow->Modifier[i]*GetscalestatSpellBonus(ssvrow))/10000;
            ModifyBonuses(SPELL_POWER, (apply ? StatValue : -StatValue));
        }

        int32 scaledarmorval = GetDBCscalestatArmorMod(ssvrow, proto->ScalingStatsFlag);
        BaseResistance[0] += (apply ? scaledarmorval : -scaledarmorval);
        CalcResistance(0);

        uint32 scaleddps = GetDBCscalestatDPSMod(ssvrow, proto->ScalingStatsFlag);
        float dpsmod = 1.0;

        if (proto->ScalingStatsFlag & 0x1400)
            dpsmod = 0.2f;
        else
            dpsmod = 0.3f;

        float scaledmindmg = (scaleddps - (scaleddps * dpsmod)) * (proto->Delay/1000);
        float scaledmaxdmg = (scaleddps * (dpsmod+1.0f)) * (proto->Delay/1000);

        if( proto->InventoryType == INVTYPE_RANGED || proto->InventoryType == INVTYPE_RANGEDRIGHT || proto->InventoryType == INVTYPE_THROWN )
        {
            BaseRangedDamage[0] += apply ? scaledmindmg : -scaledmindmg;
            BaseRangedDamage[1] += apply ? scaledmaxdmg : -scaledmaxdmg;
        }
        else
        {
            if( slot == EQUIPMENT_SLOT_OFFHAND )
            {
                BaseOffhandDamage[0] = apply ? scaledmindmg : 0;
                BaseOffhandDamage[1] = apply ? scaledmaxdmg : 0;
            }
            else
            {
                BaseDamage[0] = apply ? scaledmindmg : 0;
                BaseDamage[1] = apply ? scaledmaxdmg : 0;
            }
        }
    }
    else // Normal Items.*/
    {   // Stats
        for( int i = 0; i < 10; i++ )
        {
            int32 val = proto->Stats[i].Value;
            if( val == 0 )
                continue;
            ModifyBonuses( proto->Stats[i].Type, apply ? val : -val );
        }

        // Damage
        if( proto->minDamage && proto->maxDamage )
        {
            if( proto->InventoryType == INVTYPE_RANGED || proto->InventoryType == INVTYPE_RANGEDRIGHT || proto->InventoryType == INVTYPE_THROWN )
            {
                BaseRangedDamage[0] += apply ? proto->minDamage : -proto->minDamage;
                BaseRangedDamage[1] += apply ? proto->maxDamage : -proto->maxDamage;
            }
            else
            {
                if( slot == EQUIPMENT_SLOT_OFFHAND )
                {
                    BaseOffhandDamage[0] = apply ? proto->minDamage : 0;
                    BaseOffhandDamage[1] = apply ? proto->maxDamage : 0;
                }
                else
                {
                    BaseDamage[0] = apply ? proto->minDamage : 0;
                    BaseDamage[1] = apply ? proto->maxDamage : 0;
                }
            }
        }
    }

    // Misc
    if( apply )
    {
        // Apply all enchantment bonuses
        item->ApplyEnchantmentBonuses();

        for( int k = 0; k < 5; k++ )
        {
            // stupid fucked dbs
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

                    Spell* spell = NULLSPELL;
                    spell = (new Spell( TO_PLAYER(this), spells ,true, NULLAURA ));
                    SpellCastTargets targets;
                    targets.m_unitTarget = GetGUID();
                    spell->castedItemId = item->GetEntry();
                    spell->prepare( &targets );
                }
            }
            else if( item->GetProto()->Spells[k].Trigger == CHANCE_ON_HIT )
            {
                ProcTriggerSpell ts;
                memset(&ts, 0, sizeof(ProcTriggerSpell));
                ts.origId = 0;
                ts.spellId = item->GetProto()->Spells[k].Id;
                ts.procChance = 5;
                ts.caster = GetGUID();
                ts.procFlags = PROC_ON_MELEE_ATTACK;
                if(slot == EQUIPMENT_SLOT_MAINHAND)
                    ts.weapon_damage_type = 1; // Proc only on main hand attacks
                else if(slot == EQUIPMENT_SLOT_OFFHAND)
                    ts.weapon_damage_type = 2; // Proc only on off hand attacks
                else
                    ts.weapon_damage_type = 0; // Doesn't depend on weapon
                ts.deleted = false;
                m_procSpells.push_front( ts );
            }
        }
    }
    else
    {
        // Remove all enchantment bonuses
        item->RemoveEnchantmentBonuses();
        for( int k = 0; k < 5; k++ )
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
                std::list<struct ProcTriggerSpell>::iterator i;
                // Debug: i changed this a bit the if was not indented to the for
                // so it just set last one to deleted looks like unintended behaviour
                // because you can just use end()-1 to remove last so i put the if
                // into the for
                for( i = m_procSpells.begin(); i != m_procSpells.end(); i++ )
                {
                    if( (*i).spellId == item->GetProto()->Spells[k].Id && !(*i).deleted )
                    {
                        //m_procSpells.erase(i);
                        i->deleted = true;
                        break;
                    }
                }
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
            data.SetOpcode(SMSG_FORCE_MOVE_ROOT);
            data << GetNewGUID();
            data << flag;
            m_currentMovement = MOVE_ROOT;
        }break;
    case MOVE_UNROOT:
        {
            data.SetOpcode(SMSG_FORCE_MOVE_UNROOT);
            data << GetNewGUID();
            data << flag;
            m_currentMovement = MOVE_UNROOT;
        }break;
    case MOVE_WATER_WALK:
        {
            m_setwaterwalk = true;
            data.SetOpcode(SMSG_MOVE_WATER_WALK);
            data << GetNewGUID();
            data << flag;
        }break;
    case MOVE_LAND_WALK:
        {
            m_setwaterwalk = false;
            m_WaterWalkTimer = getMSTime()+500;
            data.SetOpcode(SMSG_MOVE_LAND_WALK);
            data << GetNewGUID();
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

    WorldPacket data(SMSG_FORCE_RUN_SPEED_CHANGE, 200);
    if( SpeedType != SWIMBACK )
    {
        data << GetNewGUID();
        data << m_speedChangeCounter++;
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
            if(value == m_lastRunSpeed)
                return;

            data.SetOpcode(SMSG_FORCE_RUN_SPEED_CHANGE);
            m_runSpeed = value;
            m_lastRunSpeed = value;
        }break;

    case RUNBACK:
        {
            if(value == m_lastRunBackSpeed)
                return;

            data.SetOpcode(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            m_backWalkSpeed = value;
            m_lastRunBackSpeed = value;
        }break;

    case SWIM:
        {
            if(value == m_lastSwimSpeed)
                return;

            data.SetOpcode(SMSG_FORCE_SWIM_SPEED_CHANGE);
            m_swimSpeed = value;
            m_lastSwimSpeed = value;
        }break;

    case SWIMBACK:
        {
            if(value == m_lastBackSwimSpeed)
                break;

            data.SetOpcode(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            m_backSwimSpeed = value;
            m_lastBackSwimSpeed = value;
        }break;

    case TURN:
        {
            data.SetOpcode(SMSG_FORCE_TURN_RATE_CHANGE);
            m_turnRate = value;
        }break;

    case FLY:
        {
            if(value == m_lastFlySpeed)
                return;

            data.SetOpcode(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            m_flySpeed = value;
            m_lastFlySpeed = value;
        }break;

    case FLYBACK:
        {
            if(value == m_lastBackFlySpeed)
                return;

            data.SetOpcode(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            m_backFlySpeed = value;
            m_lastBackFlySpeed = value;
        }break;

    case PITCH_RATE:
        {
            data.SetOpcode(SMSG_FORCE_PITCH_RATE_CHANGE);
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
    Spell* sp = NULLSPELL;
    sp = (new Spell(TO_PLAYER(this),inf,true,NULLAURA));
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
        return NULLCORPSE;
    }

    if( m_CurrentTransporter != NULL )
    {
        m_CurrentTransporter->RemovePlayer( TO_PLAYER(this) );
        m_CurrentTransporter = NULLTRANSPORT;
        GetMovementInfo()->ClearTransportData();

        ResurrectPlayer();
        RepopAtGraveyard( GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId() );
        return NULLCORPSE;
    }

    sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_PLAYER_FORCED_RESURECT ); //in case somebody resurrected us before this event happened

    // Set death state to corpse, that way players will lose visibility
    setDeathState( CORPSE );

    // Update visibility, that way people wont see running corpses :P
    UpdateVisibility();

    // If we're in battleground, remove the skinnable flag.. has bad effects heheh
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE );

    bool corpse = (m_bg != NULL) ? m_bg->CreateCorpse( TO_PLAYER(this) ) : true;
    Corpse* ret = NULLCORPSE;

    if( corpse )
        ret = CreateCorpse();

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

void Player::ResurrectPlayer(Unit* pResurrector /* = NULLPLR */)
{
    if (!sHookInterface.OnResurrect(this))
        return;
    if(PreventRes)
        return;
    sEventMgr.RemoveEvents(TO_PLAYER(this),EVENT_PLAYER_FORCED_RESURECT); //in case somebody resurected us before this event happened
    if( m_resurrectHealth )
        SetUInt32Value(UNIT_FIELD_HEALTH, (uint32)min( m_resurrectHealth, m_uint32Values[UNIT_FIELD_MAXHEALTH] ) );
    if( m_resurrectMana )
        SetUInt32Value( UNIT_FIELD_POWER1, (uint32)min( m_resurrectMana, m_uint32Values[UNIT_FIELD_MAXPOWER1] ) );

    m_resurrectHealth = m_resurrectMana = 0;

    SpawnCorpseBones();
    RemoveAura(Wispform ? 20584 : 8326);

    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE);
    setDeathState(ALIVE);
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
            TO_ARENA(m_bg)->UpdatePlayerCounts();
    }
}

void Player::KillPlayer()
{
    setDeathState(JUST_DIED);

    // Battleground stuff
    if(m_bg)
        m_bg->HookOnPlayerDeath(TO_PLAYER(this));

    EventDeath();

    m_session->OutPacket(SMSG_CANCEL_COMBAT);
    m_session->OutPacket(SMSG_CANCEL_AUTO_REPEAT);

    SetMovement(MOVE_ROOT, 0);

    StopMirrorTimer(0);
    StopMirrorTimer(1);
    StopMirrorTimer(2);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED); //player death animation, also can be used with DYNAMIC_FLAGS <- huh???
    SetUInt32Value( UNIT_DYNAMIC_FLAGS, 0x00 );
    if(getClass() == WARRIOR) //rage resets on death
        SetUInt32Value(UNIT_FIELD_POWER2, 0);
    if(getClass() == DEATHKNIGHT)
        SetUInt32Value(UNIT_FIELD_POWER7, 0);

    // combo points reset upon death
    NullComboPoints();

    GetAchievementInterface()->HandleAchievementCriteriaConditionDeath();
    GetAchievementInterface()->HandleAchievementCriteriaDeath();

    sHookInterface.OnDeath(TO_PLAYER(this));
}

Corpse* Player::CreateCorpse()
{
    Corpse* pCorpse = NULL;
    uint32 _uf, _pb, _pb2, _cfb1, _cfb2;

    objmgr.DelinkPlayerCorpses(TO_PLAYER(this));
    pCorpse = objmgr.CreateCorpse();
    pCorpse->SetInstanceID(GetInstanceID());
    pCorpse->Create(TO_PLAYER(this), GetMapId(), GetPositionX(),
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
    Corpse* pCorpse;
    pCorpse = objmgr.GetCorpseByOwner(GetLowGUID());
    myCorpse = NULLCORPSE;
    if(pCorpse)
    {
        if (pCorpse->IsInWorld() && pCorpse->GetCorpseState() == CORPSE_STATE_BODY)
        {
            if(pCorpse->GetInstanceID() != GetInstanceID())
            {
                sEventMgr.AddEvent(pCorpse, &Corpse::SpawnBones, EVENT_CORPSE_SPAWN_BONES, 100, 1,0);
            }
            else
                pCorpse->SpawnBones();
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
    if(m_bg && m_bg->HookHandleRepop(TO_PLAYER(this)))
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

    if(sHookInterface.OnRepop(TO_PLAYER(this)) && dest.x != 0 && dest.y != 0 && dest.z != 0)
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
            m_channelsbyDBCID.insert(make_pair(c->pDBC->id, c));
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
    set<uint32>::iterator i;
    Channel * c;
    uint32 cid;
    for(i = m_channels.begin(); i != m_channels.end();)
    {
        cid = *i;
        ++i;

        c = channelmgr.GetChannel(cid);
        if( c != NULL )
            c->Part(TO_PLAYER(this), false);
    }
    m_channelsbyDBCID.clear();
}

void Player::SendInitialActions()
{
    uint8 spec = GetActiveSpec();
    WorldPacket data(SMSG_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_COUNT*4 + 1);
    data << uint8(1);
    for(uint32 slot = 0; slot < PLAYER_ACTION_BUTTON_COUNT; slot++)
        data << mActions[spec][slot].PackedData;
    m_session->SendPacket(&data);
}

void Player::setAction(uint8 button, uint32 action, uint8 type, int8 SpecOverride)
{
    if( button >= PLAYER_ACTION_BUTTON_COUNT )
        return;
    if(SpecOverride < 0)
        SpecOverride = GetActiveSpec();

    mActions[SpecOverride][button].PackedData = ACTION_PACK_BUTTION(action, type);
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
                    sEventMgr.AddEvent( TO_PLAYER(this), &Player::_Warn, message, EVENT_UNIT_SENDMESSAGE, 60000, 1, 0);
                    sEventMgr.AddEvent( TO_PLAYER(this), &Player::_Kick, EVENT_PLAYER_KICK, 360000, 1, 0 );
                }
            }
            sk.CurrentValue = fields[3].GetUInt32();
            sk.MaximumValue = fields[4].GetUInt32();
            m_skills.insert(make_pair(v1,sk));

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
    uint32 agility = GetUInt32Value(UNIT_FIELD_STAT1);
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
    uint32 agility = GetUInt32Value(UNIT_FIELD_STAT1)*class_multiplier;
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
    tmp = min( max( 5.0f, tmp), DodgeCap[pClass] );

    // Add dodge from spell after checking cap and base.
    tmp += GetDodgeFromSpell();
    SetFloatValue( PLAYER_DODGE_PERCENTAGE, tmp);

    // Block
    Item* it = GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
    if( it != NULL && it->GetProto() && it->GetProto()->InventoryType == INVTYPE_SHIELD )
    {
        tmp = 5.0f + max( 0.0f, CalcRating( PLAYER_RATING_MODIFIER_BLOCK )) + max( 0.0f, GetBlockFromSpell());
        tmp += defence_contribution;
    }
    else
        tmp = 0.0f;

    SetFloatValue( PLAYER_BLOCK_PERCENTAGE, max( 0.0f, min( tmp, 95.0f ) )  );

    //parry
    tmp = 5.0f + CalcRating( PLAYER_RATING_MODIFIER_PARRY ) + GetParryFromSpell();
    if(pClass == DEATHKNIGHT) // DK gets 1/4 of strength as parry rating
        tmp += CalcPercentForRating(PLAYER_RATING_MODIFIER_PARRY, GetUInt32Value(UNIT_FIELD_STAT0) / 4);
    tmp += defence_contribution;

    SetFloatValue( PLAYER_PARRY_PERCENTAGE, std::max( 5.0f, std::min( tmp, 95.0f ) ) ); //let us not use negative parry. Some spells decrease it

    //critical
    map< uint32, WeaponModifier >::iterator itr = tocritchance.begin();
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
    SetFloatValue( PLAYER_CRIT_PERCENTAGE, min( cr, 71.0f ) );

    float rcr = tmp + CalcRating( PLAYER_RATING_MODIFIER_RANGED_CRIT ) + ranged_bonus;
    SetFloatValue( PLAYER_RANGED_CRIT_PERCENTAGE, min( rcr, 71.0f ) );

    gtFloat* SpellCritBase  = dbcSpellCritBase.LookupEntry(pClass-1);
    gtFloat* SpellCritPerInt = dbcSpellCrit.LookupEntry(pLevel - 1 + (pClass-1)*100);

    spellcritperc = 100*(SpellCritBase->val + (GetUInt32Value( UNIT_FIELD_STAT3 ) * SpellCritPerInt->val)) +
        GetSpellCritFromSpell() + CalcRating( PLAYER_RATING_MODIFIER_SPELL_CRIT );

    UpdateChanceFields();
}

void Player::UpdateChanceFields()
{
    uint32 stat = 0;
    for(; stat < 5; stat++)
        CalcStat(stat);

    // Update spell crit values in fields
    for(uint32 i = 0; i < 7; i++)
    {
        SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + i, SpellCritChanceSchool[i]+spellcritperc);
        SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT + i, DamageDonePctMod[i] );

        int32 damagedonepos = DamageDonePosMod[i], damagedoneneg = DamageDoneNegMod[i];
        for(stat = 0; stat < 5; stat++)
        {
            if(SpellDmgDoneByAttribute[stat][i] && GetStat(stat) > 0)
            {
                damagedonepos += ((SpellDmgDoneByAttribute[stat][i]*GetStat(stat))/100);
            }
        }
        if(SpellDamageFromAP[i])
            damagedonepos += (SpellDamageFromAP[i]*GetAP())/100;

        DamageDoneMod[i] = damagedonepos-damagedoneneg;
        SetUInt32Value( PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i, damagedonepos );
        SetUInt32Value( PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + i, damagedoneneg );
    }

    HealDoneModPos = HealDoneBase;
    for(stat = 0; stat < 5; stat++)
    {
        if(SpellHealDoneByAttribute[stat] && GetStat(stat) > 0)
        {
            HealDoneModPos += ((SpellHealDoneByAttribute[stat]*GetStat(stat))/100);
        }
    }
    HealDoneModPos += (SpellHealFromAP*GetAP())/100;

    SetUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS, HealDoneModPos);
    SetUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_PCT, HealDonePctMod);

    uint32 expertise_base = TO_PLAYER(this)->CalcRating( PLAYER_RATING_MODIFIER_EXPERTISE );
    if(expertise_base > 60)
        expertise_base = 60; // 15% before spell additions.

    SetUInt32Value(PLAYER_EXPERTISE, expertise_base+Expertise[0]);
    SetUInt32Value(PLAYER_OFFHAND_EXPERTISE, expertise_base+Expertise[1]);
}

void Player::UpdateAttackSpeed()
{
    Item* weap;
    uint32 calspeed = 2000;
    uint32 speed = calspeed;
    if( GetShapeShift() == FORM_CAT )
        speed = 1000;
    else if( GetShapeShift() == FORM_BEAR || GetShapeShift() == FORM_DIREBEAR )
        speed = 2500;
    else if( !disarmed )
    {
        weap = GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_MAINHAND ) ;
        if( weap != NULL )
            speed = weap->GetProto()->Delay;
    }

    calspeed = uint32((float)speed/(m_meleeattackspeedmod*(1+CalcRating(PLAYER_RATING_MODIFIER_MELEE_HASTE)/100.0f)));
    if(calspeed < 500)
        calspeed = 500;
    if(calspeed > 8000)
        calspeed = 8000;

    SetUInt32Value( UNIT_FIELD_BASEATTACKTIME, calspeed);

    weap = GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND );
    if( weap != NULL && weap->GetProto()->Class == ITEM_CLASS_WEAPON )
    {
        speed = weap->GetProto()->Delay;
        calspeed = uint32((float)speed/(m_meleeattackspeedmod*(1+CalcRating(PLAYER_RATING_MODIFIER_MELEE_HASTE)/100.0f)));
        if(calspeed < 500)
            calspeed = 500;
        if(calspeed > 8000)
            calspeed = 8000;

        SetUInt32Value( UNIT_FIELD_BASEATTACKTIME + 1, calspeed);
    }

    weap = GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_RANGED );
    if( weap != NULL )
    {
        speed = weap->GetProto()->Delay;
        calspeed = uint32((float)speed/(m_rangedattackspeedmod*(1+CalcRating(PLAYER_RATING_MODIFIER_RANGED_HASTE)/100.0f)));
        if(calspeed < 500)
            calspeed = 500;
        if(calspeed > 8000)
            calspeed = 8000;

        SetUInt32Value( UNIT_FIELD_RANGEDATTACKTIME, calspeed);
    }
}

// formulas from wowwiki
void Player::UpdateStats()
{
    UpdateAttackSpeed();

    int32 AP = 0;
    int32 RAP = 0;
    int32 stam = 0;
    int32 intl = 0;

    uint32 str = GetUInt32Value(UNIT_FIELD_STAT0);
    uint32 agi = GetUInt32Value(UNIT_FIELD_STAT1);
    uint32 lev = getLevel(), levmax = sWorld.GetMaxLevelStatCalc();
    if(levmax && levmax < lev)
        lev = levmax;
    if(lev > MAXIMUM_ATTAINABLE_LEVEL)
        lev = MAXIMUM_ATTAINABLE_LEVEL;

    // Attack power
    uint32 cl = getClass();
    switch (cl)
    {
    case DRUID:
        {
            //(Strength x 2) - 20
            AP = str * 2 - 20;
            //Agility - 10
            RAP = agi - 10;
        }break;

    case ROGUE:
        {
            //Strength + Agility + (Character Level x 2) - 20
            AP = str + agi + (lev *2) - 20;
            //Character Level + Agility - 10
            RAP = lev + agi - 10;
        }break;

    case HUNTER:
        {
            //Strength + Agility + (Character Level x 2) - 20
            AP = str + agi + (lev *2) - 20;
            //(Character Level x 2) + Agility - 10
            RAP = (lev *2) + agi - 10;
        }break;

    case SHAMAN:
        {
            //(Strength) + (Agility) + (Character Level x 2) - 20
            AP = str + agi + (lev *2) - 20;
            //Agility - 10
            RAP = agi - 10;
        }break;

    case PALADIN:
        {
            //(Strength x 2) + (Character Level x 3) - 20
            AP = (str *2) + (lev *3) - 20;
            //Agility - 10
            RAP = agi - 10;
        }break;
    case WARRIOR:
    case DEATHKNIGHT:
        {
            AP = (str *2) + (lev *3) - 20; //(Strength x 2) + (Character Level x 3) - 20
            RAP = lev + agi - 10; //Character Level + Agility - 10
        }break;

    default: //mage,priest,warlock
        {
            AP = agi - 10;
        }
    }

    if( GetShapeShift() == FORM_CAT )
    {
        // Checked 3.3.2, Agil + 40
        AP += agi + 40;
        SetPowerType(POWER_TYPE_ENERGY);
    }
    else if( GetShapeShift() == FORM_DIREBEAR)
    {
        // Checked 3.3.2, 120
        AP += 120;
        SetPowerType(myClass->power_type);
        SetPowerType(POWER_TYPE_RAGE);
    }
    else if( GetShapeShift() == FORM_BEAR)
    {
        // Checked 3.3.2, 30
        AP += 30;
        SetPowerType(myClass->power_type);
        SetPowerType(POWER_TYPE_RAGE);
    }
    else // set power type
        SetPowerType(myClass->power_type);

    if( IsInFeralForm() )
    {
        AP += m_feralAP;

        // counting and adding AP from weapon to total AP.
        Item* it = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        float dps = 0;
        if(it)
        {
            ItemPrototype *ip = it->GetProto();
            if(ip)
            {
                /*if(ip->ScalingStatsEntry) // Heirlooms
                {   // Crow: This could be done easier, but I'm lazy.
                    ScalingStatValuesEntry *ssvrow = dbcScalingStatValues.LookupEntry(getLevel() > 80 ? 80 : getLevel());
                    uint32 scaleddps = GetDBCscalestatDPSMod(ssvrow, ip->ScalingStatsFlag);
                    float dpsmod = 1.0;

                    if (ip->ScalingStatsFlag & 0x1400)
                        dpsmod = 0.2f;
                    else
                        dpsmod = 0.3f;

                    float wpnspeed = (float(ip->Delay)/1000);
                    float scaledmindmg = (scaleddps - (scaleddps * dpsmod)) * wpnspeed;
                    float scaledmaxdmg = (scaleddps * (dpsmod+1.0f)) * wpnspeed;
                    dps = (((scaledmindmg + scaledmaxdmg)/2)/wpnspeed);
                }
                else*/
                {
                    float wpndmg = ((ip->maxDamage + ip->minDamage)/2);
                    float wpnspeed = (float(ip->Delay)/1000);
                    dps = wpndmg/wpnspeed;
                }
            }

            if(dps > 54.8)
            {
                dps = ((dps - 54.8) * 14);
                AP += float2int32(dps);
            }
        }
    }

    /* modifiers */
    RAP += int32((float(m_rap_mod_pct) * (float(m_uint32Values[UNIT_FIELD_STAT3]) / 100.0f)));

    if( RAP < 0 )
        RAP = 0;
    if( AP < 0 )
        AP = 0;

    SetUInt32Value( UNIT_FIELD_ATTACK_POWER, AP );
    SetUInt32Value( UNIT_FIELD_RANGED_ATTACK_POWER, RAP );

    LevelInfo* lvlinfo = objmgr.GetLevelInfo( getRace(), getClass(), lev );
    if( lvlinfo != NULL )
    {
        stam = lvlinfo->Stat[STAT_STAMINA];
        intl = lvlinfo->Stat[STAT_INTELLECT];
    }

    int32 hp = GetUInt32Value( UNIT_FIELD_BASE_HEALTH );
    stam += GetUInt32Value( UNIT_FIELD_POSSTAT2 ) - GetUInt32Value( UNIT_FIELD_NEGSTAT2 );
    int32 res = hp + std::max(20, stam) + std::max(0, stam - 20) * 10 + m_healthfromspell + m_healthfromitems;

    if (res < 1)
        res = 1;

    int32 oldmaxhp = GetUInt32Value( UNIT_FIELD_MAXHEALTH );

    if( res < hp ) res = hp;
    SetUInt32Value( UNIT_FIELD_MAXHEALTH, res );

    if( ( int32 )GetUInt32Value( UNIT_FIELD_HEALTH ) > res )
        SetUInt32Value(UNIT_FIELD_HEALTH, res );
    else if( ( cl == DRUID) && ( GetShapeShift() == FORM_BEAR || GetShapeShift() == FORM_DIREBEAR ) )
    {
        res = float2int32( ( float )GetUInt32Value( UNIT_FIELD_MAXHEALTH ) * ( float )GetUInt32Value( UNIT_FIELD_HEALTH ) / float( oldmaxhp ) );
        SetUInt32Value(UNIT_FIELD_HEALTH, res );
    }

    // MP
    int32 mana = GetUInt32Value( UNIT_FIELD_BASE_MANA );
    if(cl != WARRIOR && cl != ROGUE && cl != DEATHKNIGHT)
    {
        intl += GetUInt32Value( UNIT_FIELD_POSSTAT3 ) - GetUInt32Value( UNIT_FIELD_NEGSTAT3 );

        res = mana + std::max(20, intl) + std::max(0, intl - 20) * 15 + m_manafromspell + m_manafromitems;
        if( res < mana )
            res = mana;
    }
    else res = mana;

    SetUInt32Value(UNIT_FIELD_MAXPOWER1, res);

    if((int32)GetUInt32Value(UNIT_FIELD_POWER1) > res)
        SetUInt32Value(UNIT_FIELD_POWER1, res);

    uint32 lvl = getLevel();
    if(lvl > 100)
        lvl = 100;

    float regen = m_ModInterrMRegen/5.0f, stat_regen = (0.001f+sqrt(float(GetUInt32Value(UNIT_FIELD_INTELLECT)))*float(GetUInt32Value(UNIT_FIELD_SPIRIT))*BaseRegen[lvl-1]) * PctPowerRegenModifier[POWER_TYPE_MANA];
    int32 modRegen = m_ModInterrMRegenPCT;
    if(modRegen > 100)
        modRegen = 100;

    float ManaIncreaseRate = 1.0f;
    if (lvl < 15)
        ManaIncreaseRate *= (2.066f - (lvl * 0.066f));

    SetFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER, (regen+stat_regen)*ManaIncreaseRate);
    SetFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER, (1.0f+regen+(stat_regen*float(modRegen)/100.0f))*ManaIncreaseRate);

    /////////////////////RATINGS STUFF/////////////////
    float haste = std::max((CalcRating( PLAYER_RATING_MODIFIER_SPELL_HASTE ) / 100.0f), 0.0f); // Must be positive
    haste += 1.0f; // Must be more than 0
    if( haste != SpellHasteRatingBonus )
    {
        float value = GetFloatValue(UNIT_MOD_CAST_SPEED) * SpellHasteRatingBonus / haste; // remove previous mod and apply current

        SetFloatValue(UNIT_MOD_CAST_SPEED, value);
        SpellHasteRatingBonus = haste;  // keep value for next run
    }
    ////////////////////RATINGS STUFF//////////////////////

    // Shield Block
    Item* shield = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if( shield != NULL && shield->GetProto()->InventoryType == INVTYPE_SHIELD )
    {
        float block_multiplier = ( 100.0f + float( m_modblockabsorbvalue ) ) / 100.0f;
        if( block_multiplier < 1.0f )
            block_multiplier = 1.0f;

        int32 blockable_damage = float2int32(( float( m_modblockvaluefromspells + GetUInt32Value(PLAYER_RATING_MODIFIER_BLOCK))*block_multiplier)+((float(str)/2.0f))), max_blockable_damage = float2int32(float(lvl)*34.5f);
        if(blockable_damage > max_blockable_damage)
            blockable_damage = max_blockable_damage;
        SetUInt32Value( PLAYER_SHIELD_BLOCK, blockable_damage );
    }
    else
        SetUInt32Value( PLAYER_SHIELD_BLOCK, 0 );

    UpdateChances();
    CalcDamage();
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
                float delta = 3.2f;
                AreaTriggerEntry* ATE = dbcAreaTrigger.LookupEntry(LastAreaTrigger->AreaTriggerID);
                if(ATE != NULL)
                {
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
    uint32 xp_to_lvl = uint32(lvlinfo->XPToNextLevel);

    // get RestXP multiplier from config.
    float bubblerate = sWorld.getRate(RATE_RESTXP);

    // One bubble (5% of xp_to_level) for every 8 hours logged out.
    // if multiplier RestXP (from Hearthstone.config) is f.e 2, you only need 4hrs/bubble.
    uint32 rested_xp = uint32(0.05f * xp_to_lvl * ( seconds / (3600 * ( 8 / bubblerate))));

    // if we are at a resting area rest_XP goes 4 times faster (making it 1 bubble every 2 hrs)
    if (m_isResting)
        rested_xp <<= 2;

    // Add result to accumulated rested XP
    m_restAmount += uint32(rested_xp);

    // and set limit to be max 1.5 * 20 bubbles * multiplier (1.5 * xp_to_level * multiplier)
    if (m_restAmount > xp_to_lvl + (uint32)((float)( xp_to_lvl >> 1 ) * bubblerate ))
        m_restAmount = xp_to_lvl + (uint32)((float)( xp_to_lvl >> 1 ) * bubblerate );

    sLog.Debug("REST","Add %d rest XP to a total of %d, RestState %d", rested_xp, m_restAmount,m_isResting);

    // Update clients interface with new values.
    UpdateRestState();
}

void Player::UpdateRestState()
{
    if(m_restAmount && GetUInt32Value(UNIT_FIELD_LEVEL) < GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
        m_restState = RESTSTATE_RESTED;
    else
        m_restState = RESTSTATE_NORMAL;

    // Update RestState 100%/200%
    SetUInt32Value(PLAYER_BYTES_2, ((GetUInt32Value(PLAYER_BYTES_2) & 0x00FFFFFF) | (m_restState << 24)));

    //update needle (weird, works at 1/2 rate)
    SetUInt32Value(PLAYER_REST_STATE_EXPERIENCE, m_restAmount >> 1);
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

bool Player::CanSee(Object* obj) // * Invisibility & Stealth Detection - Partha *
{
    if (obj == TO_PLAYER(this))
       return true;

    uint32 object_type = obj->GetTypeId();

    // We can't see any objects in another phase
    // unless they're in ALL_PHASES
    if( !PhasedCanInteract(obj) )
        return false;

    /* I'm a GM, I can see EVERYTHING! :D */
    if( bGMTagOn )
        return true;

    if(obj->IsPlayer() && TO_PLAYER(obj)->m_isGmInvisible)
    {
        if(bGMTagOn)
            return true;

        return false;
    }

    if(getDeathState() == CORPSE) // we are dead and we have released our spirit
    {
        if(object_type == TYPEID_PLAYER)
        {
            Player* pObj = TO_PLAYER(obj);

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
            if(obj->IsPlayer() && TO_PLAYER(obj)->InStealth())
                return false;

            return true;
        }

        if(object_type == TYPEID_UNIT)
        {
            Unit* uObj = TO_UNIT(obj);

            return uObj->IsSpiritHealer(); // we can't see any NPCs except spirit-healers
        }

        return false;
    }
    //------------------------------------------------------------------

    // mage invisibility - ugh.
    if( IsPlayer() && m_mageInvisibility )
    {
        if( object_type == TYPEID_PLAYER )
            return TO_PLAYER(obj)->m_mageInvisibility;

        if( object_type == TYPEID_UNIT )
            return false;

        return true;
    }

    switch(object_type) // we are alive or we haven't released our spirit yet
    {
        case TYPEID_PLAYER:
            {
                Player* pObj = TO_PLAYER(obj);

                if( pObj->m_mageInvisibility )
                    return false;

                if(pObj->m_invisible) // Invisibility - Detection of Players
                {
                    if(pObj->getDeathState() == CORPSE)
                        return bGMTagOn; // only GM can see players that are spirits

                    if(GetGroup() && pObj->GetGroup() == GetGroup() // can see invisible group members except when dueling them
                            && DuelingWith != pObj)
                        return true;

                    if(pObj->stalkedby == GetGUID()) // Hunter's Mark / MindVision is visible to the caster
                        return true;

                    if(m_invisDetect[INVIS_FLAG_NORMAL] < 1 // can't see invisible without proper detection
                            || pObj->m_isGmInvisible) // can't see invisible GM
                        return bGMTagOn; // GM can see invisible players
                }

                if(pObj->InStealth()) // Stealth Detection
                {
                    if(GetGroup() && pObj->GetGroup() == GetGroup()) // can see stealthed group members
                        return true;

                    uint64 stalker = pObj->stalkedby;
                    if(stalker)
                    {
                        Object* pStalker = GetMapMgr() ? GetMapMgr()->_GetObject(stalker) : NULLOBJ;
                        if(pStalker == TO_PLAYER(this))
                            return true;

                        if(pStalker && pStalker->IsPlayer())
                        {
                            if(GetGroup() && GetGroup()->HasMember(TO_PLAYER(pStalker)))
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
                Unit* uObj = TO_UNIT(obj);

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
                GameObject* gObj = TO_GAMEOBJECT(obj);

                if(gObj->invisible) // Invisibility - Detection of GameObjects
                {
                    uint64 owner = gObj->GetUInt64Value(OBJECT_FIELD_CREATED_BY);

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

void Player::AddInRangeObject(Object* pObj)
{
    //Send taxi move if we're on a taxi
    if (m_CurrentTaxiPath && (pObj->IsPlayer()))
    {
        uint32 ntime = getMSTime();

        if (ntime > m_taxi_ride_time)
            m_CurrentTaxiPath->SendMoveForTime( TO_PLAYER(this), TO_PLAYER( pObj ), ntime - m_taxi_ride_time);
        /*else
            m_CurrentTaxiPath->SendMoveForTime( TO_PLAYER(this), TO_PLAYER( pObj ), m_taxi_ride_time - ntime);*/
    }

    if( pObj->IsCreature() && pObj->m_faction && pObj->m_factionTemplate->FactionFlags & 0x1000 )
        m_hasInRangeGuards++;

    Unit::AddInRangeObject(pObj);

    //if the object is a unit send a move packet if they have a destination
    if(pObj->GetTypeId() == TYPEID_UNIT)
    {
        //add an event to send move update have to send guid as pointer was causing a crash :(
        TO_CREATURE( pObj )->GetAIInterface()->SendCurrentMove(this);
    }

    //unit based objects, send aura data
    if (pObj->IsUnit())
    {
        Unit* pUnit=TO_UNIT(pObj);
        Aura* aur = NULLAURA;

        if (GetSession() != NULL)
        {
            WorldPacket* data = new WorldPacket(SMSG_AURA_UPDATE_ALL, 28 * MAX_AURAS);
            *data << pUnit->GetNewGUID();
            if(pUnit->m_AuraInterface.BuildAuraUpdateAllPacket(data))
                SendPacket(data);
            else delete data;
        }
    }
}

void Player::OnRemoveInRangeObject(Object* pObj)
{
    if( pObj->IsCreature() && pObj->m_faction && pObj->m_factionTemplate->FactionFlags & 0x1000 )
        m_hasInRangeGuards--;

    if(m_tempSummon == pObj)
    {
        m_tempSummon->RemoveFromWorld(false, true);
        if(m_tempSummon)
            m_tempSummon->SafeDelete();

        m_tempSummon = NULLCREATURE;
        SetUInt64Value(UNIT_FIELD_SUMMON, 0);
    }

    pObj->DestroyForPlayer(this);
    m_visibleObjects.erase(pObj);
    Unit::OnRemoveInRangeObject(pObj);

    if( pObj == m_CurrentCharm)
    {
        Unit* p = m_CurrentCharm;

        if(pObj == GetVehicle())
            GetVehicle()->RemovePassenger(TO_PLAYER(this));
        else
            UnPossess();
        if(m_currentSpell)
            m_currentSpell->cancel();      // cancel the spell
        m_CurrentCharm=NULLUNIT;

        if( p->m_temp_summon&&p->GetTypeId() == TYPEID_UNIT )
            TO_CREATURE( p )->SafeDelete();
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
            m_Summon = NULLPET;
        }
    }

    if(pObj->IsUnit())
    {
        for(uint32 x = 0; x < NUM_SPELL_TYPE_INDEX; ++x)
            if(m_spellIndexTypeTargets[x] == pObj->GetGUID())
                m_spellIndexTypeTargets[x] = 0;
    }

    if( pObj == DuelingWith )
        sEventMgr.AddEvent(TO_PLAYER(this), &Player::EndDuel, (uint8)DUEL_WINNER_RETREAT, EVENT_PLAYER_DUEL_COUNTDOWN, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Player::ClearInRangeSet()
{
    set<uint64> unique;
    Object* pObj = NULL;
    for( Object::InRangeSet::iterator itr = m_visibleObjects.begin(); itr != m_visibleObjects.end();)
    {
        pObj = (*itr);
        ++itr;
        if(unique.find(pObj->GetGUID()) == unique.end())
        {
            if(pObj->IsPlayer())
                TO_PLAYER(pObj)->DestroyForPlayer(this);
            else
                pObj->DestroyForPlayer(this);
            unique.insert(pObj->GetGUID());
        }
    }
    m_visibleObjects.clear();
    for( Object::InRangeSet::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end();)
    {
        pObj = (*itr);
        ++itr;
        if(unique.find(pObj->GetGUID()) == unique.end())
        {
            if(pObj->IsPlayer())
                TO_PLAYER(pObj)->DestroyForPlayer(this);
            else
                pObj->DestroyForPlayer(this);
            unique.insert(pObj->GetGUID());
        }
    }
    m_objectsInRange.clear();
    for( unordered_set<Player*>::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end();)
    {
        pObj = (*itr);
        ++itr;
        if(unique.find(pObj->GetGUID()) == unique.end())
        {
            TO_PLAYER(pObj)->DestroyForPlayer(this);
            unique.insert(pObj->GetGUID());
        }
    }
    m_inRangePlayers.clear();
    for( Object::InRangeUnitSet::iterator itr = m_oppFactsInRange.begin(); itr != m_oppFactsInRange.end();)
    {
        pObj = (*itr);
        ++itr;
        if(unique.find(pObj->GetGUID()) == unique.end())
        {
            if(pObj->IsPlayer())
                TO_PLAYER(pObj)->DestroyForPlayer(this);
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
    vector<string> tokens = StrSplit(data, " ");

    int index;
    vector<string>::iterator iter;

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
void Player::SendLoot(uint64 guid, uint32 mapid, uint8 loot_type)
{
    Group * m_Group = m_playerInfo->m_Group;
    if(!IsInWorld())
        return;

    Object* lootObj;
    // handle items
    if(GUID_HIPART(guid) == HIGHGUID_TYPE_ITEM)
        lootObj = m_ItemInterface->GetItemByGUID(guid);
    else
        lootObj = m_mapMgr->_GetObject(guid);
    if( lootObj == NULL )
        return;

    int8 loot_method = -1;

    //lootsteal fix
    if( lootObj->GetTypeId() == TYPEID_UNIT )
    {
        Creature* LootOwner = TO_CREATURE( lootObj );
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
    lootObj->m_loot.looters.insert(GetLowGUID());

    m_lootGuid = guid;

    WorldPacket data(SMSG_LOOT_RESPONSE, 32), data2(32);
    data << uint64(guid);
    data << uint8(loot_type);//loot_type;
    data << uint32(lootObj->m_loot.gold);
    data << uint8(0);//loot size reserve
    data << uint8(0);//unk

    uint32 count = 0;
    uint8 slottype = 0;
    std::vector<__LootItem>::iterator iter = lootObj->m_loot.items.begin();
    for(uint32 x = 0; iter != lootObj->m_loot.items.end(); iter++, x++)
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
        if(m_Group != NULL && loot_type < 2)
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
                            }
                            else
                                iter->roll->PlayerRolled( (*itr), PASS );       // passed
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
    else
    {
        if(resetnum > 10)
            return  500000;
        else
            return resetnum*50000;
    }
}

void Player::SendTalentResetConfirm()
{
    WorldPacket data(MSG_TALENT_WIPE_CONFIRM, 12);
    data << GetGUID();
    data << CalcTalentResetCost(GetTalentResetTimes());
    GetSession()->SendPacket(&data);
}

void Player::SendPetUntrainConfirm()
{
    Pet* pPet = GetSummon();
    if( pPet == NULL )
        return;

    WorldPacket data( SMSG_PET_UNLEARN_CONFIRM, 12 );
    data << pPet->GetGUID();
    data << pPet->GetUntrainCost();
    GetSession()->SendPacket( &data );
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
        SM_FFValue( SM[SMT_RANGE][0], &maxr, spellinfo->SpellGroupType );
        SM_PFValue( SM[SMT_RANGE][1], &maxr, spellinfo->SpellGroupType );
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

    m_AutoShotDuration = m_uint32Values[UNIT_FIELD_RANGEDATTACKTIME];

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

        Spell* sp = NULLSPELL;
        sp = (new Spell( TO_PLAYER(this), m_AutoShotSpell, true, NULLAURA ));
        SpellCastTargets tgt;
        tgt.m_unitTarget = m_curSelection;
        tgt.m_targetMask = TARGET_FLAG_UNIT;
        sp->prepare( &tgt );
    }
}

void Player::removeSpellByHashName(uint32 hash)
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
        sHookInterface.OnQuestCancelled(TO_PLAYER(this), qst);
        CALL_QUESTSCRIPT_EVENT(qst->id, OnQuestCancel)(TO_PLAYER(this));
        qle->Quest_Status = QUEST_STATUS__FAILED;
    }
    else
        sEventMgr.AddEvent(TO_PLAYER(this), &Player::EventTimedQuestExpire, qst, qle, log_slot, interval, EVENT_TIMED_QUEST_EXPIRE, interval, 1, 0);
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

    set<uint32>::iterator itr, itr2;
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

    // SMSG_INSTANCE_DIFFICULTY
    data.Initialize(SMSG_INSTANCE_DIFFICULTY, 4+4);
    data << uint32(0);
    GetSession()->SendPacket(&data);

    //Initial Spells
    smsg_InitialSpells();

    data.Initialize(SMSG_SEND_UNLEARN_SPELLS, 4);
    data << uint32(0);                                      // count, for (count) uint32;
    GetSession()->SendPacket(&data);

    //Initial Actions
    SendInitialActions();

    //Factions
    smsg_InitialFactions();

    data.Initialize(SMSG_ALL_ACHIEVEMENT_DATA, 2000);
    m_achievementInterface->BuildAllAchievementDataPacket(&data);
    GetSession()->SendPacket(&data);

    // Sets
    SendEquipmentSets();

    // Login speed
    data.Initialize(SMSG_LOGIN_SETTIMESPEED);
    data << uint32(secsToTimeBitFields(UNIXTIME));
    data << float(0.0166666669777748f);
    data << uint32(0);
    GetSession()->SendPacket( &data );

    // Currencies
    data.Initialize(SMSG_INIT_CURRENCY);
    data << uint32(0);
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
    if(titanGrip || !GetItemInterface() || HasSpell(46917) || HasTalent(m_talentActiveSpec, 1867) || GetSession()->HasGMPermissions())
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
            offhand->RemoveFromWorld();
            offhand->SetOwner( NULLPLR );
            offhand->SaveToDB( INVENTORY_SLOT_NOT_SET, 0, true, NULL );
            sMailSystem.DeliverMessage(MAILTYPE_NORMAL, GetGUID(), GetGUID(), "Your offhand item", "", 0, 0, offhand->GetUInt32Value(OBJECT_FIELD_GUID), 1, true);
            offhand->DeleteMe();
            offhand = NULLITEM;
        }
        else
        {
            if( !GetItemInterface()->SafeAddItem(offhand, result.ContainerSlot, result.Slot) )
            {
                if( !GetItemInterface()->AddItemToFreeSlot(offhand) )   // shouldn't happen either.
                {
                    offhand->DeleteMe();
                    offhand = NULLITEM;
                }
            }
        }
    }
}

void Player::Reset_Talents(bool all)
{
    if(all)
        m_specs[m_talentActiveSpec == 1 ? 0 : 1].talents.clear();

    std::map<uint32, uint8> *talents = &m_specs[m_talentActiveSpec].talents;
    std::map<uint32, uint8>::iterator itr;
    for(itr = talents->begin(); itr != talents->end(); itr++)
    {
        TalentEntry *te = dbcTalent.LookupEntry(itr->first);
        if(!te)
            continue;
        RemoveTalent(te->RankID[itr->second]);
    }
    talents->clear();

    // The dual wield skill for shamans can only be added by talents.
    // so when reset, the dual wield skill should be removed too.
    // (see also void Spell::SpellEffectDualWield)
    if( getClass() == SHAMAN && _HasSkillLine( SKILL_DUAL_WIELD ) )
        _RemoveSkillLine( SKILL_DUAL_WIELD );

    if( getClass() == WARRIOR )
    {
        titanGrip = false;
        ResetTitansGrip();
    }
    smsg_TalentsInfo(false);
    SetTalentResetTimes(GetTalentResetTimes() + 1);
}

uint16 Player::GetMaxTalentPoints()
{
    // see if we have a custom value
    if(m_maxTalentPoints > 0)
        return m_maxTalentPoints;

    // otherwise use blizzlike value
    uint32 l = getLevel();
    if(l > 9)
    {
        return l - 9;
    }
    else
    {
        return 0;
    }
}

void Player::ApplySpec(uint8 spec, bool init)
{
    if(spec > m_talentSpecsCount || spec > 1/*MAX_SPEC_COUNT*/) // Crow: We use 0 and 1.
        return;

    std::map<uint32, uint8> *talents;
    std::map<uint32, uint8>::iterator itr;
//#define UNTESTED_NEW_SPEC_CHANGE_SYSTEM
#ifdef UNTESTED_NEW_SPEC_CHANGE_SYSTEM
    if(init == false)   // unapply old spec
    {
        for (uint32 i = 0; i < dbcTalent.GetNumRows(); ++i)
        {
            TalentEntry const *talentInfo = dbcTalent.LookupEntry(i);

            if (!talentInfo)
                continue;

            TalentTabEntry const *talentTabInfo = dbcTalentTab.LookupEntry(talentInfo->TalentTree);

            if (!talentTabInfo)
                continue;

            if ((getClassMask() & talentTabInfo->ClassMask) == 0)
                continue;

            for (int8 rank = 5-1; rank >= 0; --rank)
            {
                // skip non-existant talent ranks
                if (talentInfo->RankID[rank] == 0)
                    continue;
                    RemoveTalent(talentInfo->RankID[rank]);

                if (const SpellEntry *_spellEntry = dbcSpell.LookupEntry(talentInfo->RankID[rank]))
                    for (uint8 i = 0; i < 3; ++i)                  // search through the SpellEntry for valid trigger spells
                        if (_spellEntry->EffectTriggerSpell[i] > 0 && _spellEntry->Effect[i] == SPELL_EFFECT_LEARN_SPELL)
                            removeSpell(_spellEntry->EffectTriggerSpell[i], false, false, 0); // and remove any spells that the talent teaches
            }
        }
        if( getClass() == WARRIOR )
        {
            titanGrip = false;
            ResetTitansGrip();
        }
        if( getClass() == DRUID )
            SetShapeShift(0);

        //Dismiss any pets
        if(GetSummon())
        {
            if(GetSummon()->GetUInt32Value(UNIT_CREATED_BY_SPELL) > 0)
                GetSummon()->Dismiss(false);                // warlock summon -> dismiss
            else
                GetSummon()->Remove(false, true, true); // hunter pet -> just remove for later re-call
        }
    }

    // apply new spec
    talents = &m_specs[spec].talents;
    uint32 spentPoints = 0;

    for (uint32 talentId = 0; talentId < dbcTalent.GetNumRows(); ++talentId)
    {
        TalentEntry const *talentInfo = dbcTalent.LookupEntry(talentId);
        if (!talentInfo)
            continue;
        TalentTabEntry const *talentTabInfo = dbcTalentTab.LookupEntry(talentInfo->TalentTree);
        if (!talentTabInfo)
            continue;
        if ((getClassMask() & talentTabInfo->ClassMask) == 0)
            continue;

        for (int8 rank = 5-1; rank >= 0; --rank)
        {
            if (talentInfo->RankID[rank] == 0)
                continue;
            if (HasTalent(spec, talentInfo->RankID[rank]))
            {
                ApplyTalent(talentInfo->RankID[rank]);
                spentPoints += (rank + 1);
            }
        }
    }
#else
    if(init == false)   // unapply old spec
    {
        talents = &m_specs[m_talentActiveSpec].talents;
        if(talents->size())
        {
            for(itr = talents->begin(); itr != talents->end(); itr++)
            {
                TalentEntry * talentInfo = dbcTalent.LookupEntry(itr->first);
                if(!talentInfo || itr->second > 4)
                    continue;
                RemoveTalent(talentInfo->RankID[itr->second]);
            }

            if( getClass() == WARRIOR )
            {
                titanGrip = false;
                ResetTitansGrip();
            }
            if( getClass() == DRUID )
            {
                SetShapeShift(0);
            }

            //Dismiss any pets
            if(GetSummon())
            {
                if(GetSummon()->GetUInt32Value(UNIT_CREATED_BY_SPELL) > 0)
                    GetSummon()->Dismiss(false);                // warlock summon -> dismiss
                else
                    GetSummon()->Remove(false, true, true); // hunter pet -> just remove for later re-call
            }
        }
    }

    // apply new spec
    talents = &m_specs[spec].talents;
    uint32 spentPoints = 0;
    if(talents->size())
    {
        for(itr = talents->begin(); itr != talents->end(); itr++)
        {
            TalentEntry * talentInfo = dbcTalent.LookupEntry(itr->first);
            if(!talentInfo || itr->second > 4)
                continue;

            ApplyTalent(talentInfo->RankID[itr->second]);
            spentPoints += itr->second + 1;
        }
    }
#endif
    m_talentActiveSpec = spec;

    // update available Talent Points
    uint32 maxTalentPoints = GetMaxTalentPoints();
    uint32 newTalentPoints;
    if(spentPoints >= maxTalentPoints)
        newTalentPoints = 0;    // just in case
    else
        newTalentPoints = maxTalentPoints - spentPoints;

    // Apply glyphs
    for(uint32 i = 0; i < GLYPHS_COUNT; i++)
    {
        UnapplyGlyph(i);
        SetGlyph(i, m_specs[m_talentActiveSpec].glyphs[i]);
    }
    smsg_TalentsInfo(false);
    SendInitialActions();
}

void Player::ApplyTalent(uint32 spellid)
{
    SpellEntry *spellInfo = dbcSpell.LookupEntry( spellid ), *spellInfo2 = NULL;
    if(!spellInfo)
        return; // not found

    if(!(spellInfo->Attributes & ATTRIBUTES_PASSIVE))
    {
        if(spellInfo->RankNumber)
        {
            if(!FindHigherRankingSpellWithNamehash(spellInfo->NameHash, spellInfo->RankNumber))
                addSpell(spellid);  // in this case we need to learn the spell itself
        }
        else
            addSpell(spellid);  // in this case we need to learn the spell itself
    }

    if( (spellInfo->Attributes & ATTRIBUTES_PASSIVE || (spellInfo->Effect[0] == SPELL_EFFECT_LEARN_SPELL ||
        spellInfo->Effect[1] == SPELL_EFFECT_LEARN_SPELL ||
        spellInfo->Effect[2] == SPELL_EFFECT_LEARN_SPELL)
        && ( (spellInfo->c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET) == 0 || ( (spellInfo->c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET) && GetSummon() ) ) )
        )
    {
        for(uint8 i = 0; i < 3; i++)
        {
            if(spellInfo->Effect[i] == SPELL_EFFECT_LEARN_SPELL)
            {
                if((spellInfo2 = dbcSpell.LookupEntry(spellInfo->EffectTriggerSpell[i])) != NULL)
                {
                    if(spellInfo2->RankNumber)
                    {
                        if(FindHigherRankingSpellWithNamehash(spellInfo2->NameHash, spellInfo2->RankNumber))
                            return;
                    }
                }
            }
        }

        if( !(spellInfo->RequiredShapeShift && !( (uint32)1 << (GetShapeShift()-1) & spellInfo->RequiredShapeShift ) ) )
        {
            Spell* sp = new Spell(this,spellInfo,true,NULLAURA);
            SpellCastTargets tgt;
            tgt.m_unitTarget = GetGUID();
            sp->prepare(&tgt);
        }
    }
}

void Player::RemoveTalent(uint32 spellid)
{
    SpellEntry * sp = dbcSpell.LookupEntry(spellid);
    if(!sp)
        return; // not found

    for(uint8 k = 0; k < 3; ++k)
    {
        if(sp->Effect[k] == SPELL_EFFECT_LEARN_SPELL)
        {
            SpellEntry * sp2 = dbcSpell.LookupEntry(sp->EffectTriggerSpell[k]);
            if(!sp2)
                continue;
            removeSpellByHashName(sp2->NameHash);
        }
    }

    removeSpellByHashName(sp->NameHash);
    m_AuraInterface.RemoveAllAurasByNameHash(sp->NameHash, true);
}

void Player::LearnTalent(uint32 talent_id, uint32 requested_rank)
{
    unsigned int i;
    if (requested_rank > 4)
        return;

    TalentEntry * talentInfo = dbcTalent.LookupEntry(talent_id);
    if(!talentInfo)return;

    uint32 CurTalentPoints = 0;
    std::map<uint32, uint8> *talents = &m_specs[m_talentActiveSpec].talents;
    uint8 currentRank = 0;
    std::map<uint32, uint8>::iterator itr = talents->find(talent_id);
    if(itr != talents->end())
        currentRank = itr->second + 1;

    if(currentRank >= requested_rank + 1)
        return; // player already knows requested or higher rank for this talent

    uint32 RequiredTalentPoints = requested_rank + 1 - currentRank;
    if(CurTalentPoints < RequiredTalentPoints )
        return; // player doesn't have enough points to get this rank for this talent

    // Check if it requires another talent
    if (talentInfo->DependsOn > 0)
    {
        TalentEntry *depTalentInfo = NULL;
        depTalentInfo = dbcTalent.LookupEntry(talentInfo->DependsOn);
        if (depTalentInfo)
        {
            itr = talents->find(talentInfo->DependsOn);
            if(itr == talents->end())
                return; // player doesn't have the talent this one depends on
            if(talentInfo->DependsOnRank > itr->second)
                return; // player doesn't have the talent rank this one depends on
        }
    }

    // Check that the requested talent belongs to a tree from player's class
    uint32 tTree = talentInfo->TalentTree;
    uint32 cl = getClass();

    for(i = 0; i < 3; i++)
        if(tTree == TalentTreesPerClass[cl][i])
            break;

    if(i == 3)
    {
        // cheater!
        GetSession()->Disconnect();
        return;
    }

    // Find out how many points we have in this field
    uint32 spentPoints = 0;
    if (talentInfo->Row > 0)
    {
        for(itr = talents->begin(); itr != talents->end(); itr++)
        {
            TalentEntry *tmpTalent = dbcTalent.LookupEntry(itr->first);
            if (tmpTalent->TalentTree == tTree)
            {
                spentPoints += itr->second + 1;
            }
        }
    }

    uint32 spellid = talentInfo->RankID[requested_rank];
    if( spellid == 0 )
    {
        sLog.outDebug("Talent: %u Rank: %u = 0", talent_id, requested_rank);
        return;
    }

    if(spentPoints < (talentInfo->Row * 5))          // Min points spent
    {
        return;
    }

    (*talents)[talent_id] = requested_rank;

    // More cheat death hackage! :)
    if(spellid == 31330)
        m_cheatDeathRank = 3;
    else if(spellid == 31329)
        m_cheatDeathRank = 2;
    else if(spellid == 31328)
        m_cheatDeathRank = 1;

    if(requested_rank > 0 ) // remove old rank aura
    {
        uint32 respellid = talentInfo->RankID[currentRank - 1];
        if(respellid)
        {
            RemoveAura(respellid);
        }
    }

    ApplyTalent(spellid);
}

void Player::Reset_ToLevel1()
{
    m_AuraInterface.RemoveAllAuras();

    SetUInt32Value(UNIT_FIELD_LEVEL, 1);
    PlayerCreateInfo *info = objmgr.GetPlayerCreateInfo(getRace(), getClass());
    ASSERT(info);

    SetUInt32Value(UNIT_FIELD_HEALTH, info->health);
    SetUInt32Value(UNIT_FIELD_POWER1, info->mana );
    SetUInt32Value(UNIT_FIELD_POWER2, 0 ); // this gets devided by 10
    SetUInt32Value(UNIT_FIELD_POWER3, info->focus );
    SetUInt32Value(UNIT_FIELD_POWER4, info->energy );
    SetUInt32Value(UNIT_FIELD_POWER7, 0 );
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, info->health);
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, info->health);
    SetUInt32Value(UNIT_FIELD_BASE_MANA, info->mana);
    SetUInt32Value(UNIT_FIELD_MAXPOWER1, info->mana );
    SetUInt32Value(UNIT_FIELD_MAXPOWER2, info->rage );
    SetUInt32Value(UNIT_FIELD_MAXPOWER3, info->focus );
    SetUInt32Value(UNIT_FIELD_MAXPOWER4, info->energy );
    SetUInt32Value(UNIT_FIELD_MAXPOWER7, 1000 );
    SetUInt32Value(UNIT_FIELD_STAT0, info->strength );
    SetUInt32Value(UNIT_FIELD_STAT1, info->ability );
    SetUInt32Value(UNIT_FIELD_STAT2, info->stamina );
    SetUInt32Value(UNIT_FIELD_STAT3, info->intellect );
    SetUInt32Value(UNIT_FIELD_STAT4, info->spirit );
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER, info->attackpower );
    for(uint32 x=0;x<7;x++)
        SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+x, 1.00);

}

void Player::CalcResistance(uint32 type)
{
    ASSERT(type < 7);

    int32 pos = FlatResistanceModifierPos[type]+float2int32( BaseResistance[type] * (float)( BaseResistanceModPctPos[ type ] / 100.0f ) );
    int32 neg = FlatResistanceModifierNeg[type]+float2int32( BaseResistance[type] * (float)( BaseResistanceModPctNeg[ type ] / 100.0f ) );

    int32 res = BaseResistance[type] + pos - neg;
    if( type == 0 )
        res += m_uint32Values[UNIT_FIELD_STAT1] * 2;//fix armor from agi
    if( res < 0 )
        res = 0;

    pos += float2int32( res * (float)( ResistanceModPctPos[type] / 100.0f ) );
    neg += float2int32( res * (float)( ResistanceModPctNeg[type] / 100.0f ) );
    res = pos - neg + BaseResistance[type];
    if( type == 0 )
        res += m_uint32Values[UNIT_FIELD_STAT1] * 2;//fix armor from agi

    res = res < 0 ? 0 : res;
    SetUInt32Value(UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+type,pos);
    SetUInt32Value(UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+type,neg);
    SetUInt32Value(UNIT_FIELD_RESISTANCES+type,res);
}

void Player::UpdateNearbyGameObjects()
{
    GameObject* Gobj = NULL;
    for (Object::InRangeSet::iterator itr = GetInRangeSetBegin(); itr != GetInRangeSetEnd(); itr++)
    {
        if((*itr)->IsGameObject())
        {
            Gobj = TO_GAMEOBJECT(*itr);
            ByteBuffer buff(500);
            Gobj->SetUpdateField(OBJECT_FIELD_GUID);
            Gobj->SetUpdateField(OBJECT_FIELD_GUID+1);
            Gobj->BuildValuesUpdateBlockForPlayer(&buff, this);
            PushUpdateData(&buff, 1);
        }
    }
}

void Player::UpdateNearbyQuestGivers()
{
    GameObject* Gobj = NULL;
    for (Object::InRangeSet::iterator itr = GetInRangeSetBegin(); itr != GetInRangeSetEnd(); itr++)
    {
        if((*itr)->IsGameObject())
        {
            if(TO_GAMEOBJECT(*itr)->isQuestGiver())
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
            if(TO_CREATURE(*itr)->isQuestGiver())
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
    /*else
        m_CurrentTaxiPath->SetPosForTime(x, y, z, m_taxi_ride_time - ntime, &lastNode);*/

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
    data << GetNewGUID();
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

    sEventMgr.AddEvent(TO_PLAYER(this), &Player::EventTaxiInterpolate,
        EVENT_PLAYER_TAXI_INTERPOLATE, 900, 0,0);

    if( mapchangeid < 0 )
    {
        TaxiPathNode *pn = path->GetPathNode((uint32)path->GetNodeCount() - 1);
        sEventMgr.AddEvent(TO_PLAYER(this), &Player::EventDismount, path->GetPrice(),
            pn->x, pn->y, pn->z, EVENT_PLAYER_TAXI_DISMOUNT, traveltime, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        sEventMgr.AddEvent(TO_PLAYER(this), &Player::EventTeleport, (uint32)mapchangeid, mapchangex, mapchangey, mapchangez, orientation, 1, EVENT_PLAYER_TELEPORT, traveltime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
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

void Player::CalcStat(uint32 type)
{
    int32 res;
    ASSERT( type < 5 );
    int32 pos = float2int32(((int32)BaseStats[type] * (int32)StatModPctPos[type] ) / 100.0f + (int32)FlatStatModPos[type]);
    int32 neg = float2int32(((int32)BaseStats[type] * (int32)StatModPctNeg[type] ) / 100.0f + (int32)FlatStatModNeg[type]);
    res = pos + int32(BaseStats[type]) - neg;

    if( res <= 0 )
    {
        res = 1;
    }
    else
    {
        pos += float2int32(( res * (int32)TotalStatModPctPos[type] ) / 100.0f);
        neg += float2int32(( res * (int32)TotalStatModPctNeg[type] ) / 100.0f);
        res = pos + int32(BaseStats[type]) - neg;
        if( res <= 0 )
            res = 1;
    }

    SetUInt32Value( UNIT_FIELD_POSSTAT0 + type, pos );
    SetUInt32Value( UNIT_FIELD_NEGSTAT0 + type, neg );
    SetUInt32Value( UNIT_FIELD_STAT0 + type, res > 0 ? res : 0 );
    if( type == 1 )
        CalcResistance(RESISTANCE_ARMOR);
}

void Player::PlayerRegeneratePower(bool is_interrupted)
{
    uint32 m_regenTimer = m_P_regenTimer; //set next regen time
    m_regenTimerCount += m_regenTimer;

    for(uint8 power = POWER_TYPE_MANA; power < MAX_POWER_TYPE; power++)
    {
        uint32 maxValue = GetMaxPower(power);
        if (!maxValue)
            continue;

        uint32 curValue = GetPower(power);

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
            SetPower(power, curValue);
        }
        else
            m_uint32Values[UNIT_FIELD_POWER1 + power] = curValue;
        continue;
    }
}

void Player::RegenerateHealth( bool inCombat )
{
    uint32 cur = GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 mh = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(cur >= mh)
        return;

    float Spirit = (float) GetUInt32Value(UNIT_FIELD_STAT4);
    uint8 Class = getClass(), level = ((getLevel() > MAXIMUM_ATTAINABLE_LEVEL) ? MAXIMUM_ATTAINABLE_LEVEL : getLevel());
    gtFloat *HPRegen = dbcHPRegen.LookupEntry((Class-1)*MAXIMUM_ATTAINABLE_LEVEL + (level-1));
    if(HPRegen == NULL)
        return;

    // This has some bad naming. HPRegen* is actually out of combat base, while HPRegenBase* is mana per spirit.
    float basespirit = ((Spirit > 50) ? 50 : Spirit);
    float basespiritdiff = Spirit - basespirit;
    float amt = ((basespiritdiff *HPRegen->val)+basespirit);

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
        data << mapid;
        data.appendvector(destination, true);
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
            m_CurrentTransporter->RemovePlayer(TO_PLAYER(this));
            m_CurrentTransporter = NULLTRANSPORT;
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
        pItem = GetItemInterface()->GetInventoryItem(i);
        if( pItem != NULL )
        {
            pItem->PushToWorld(m_mapMgr);

            if(i < INVENTORY_SLOT_BAG_END)    // only equipment slots get mods.
                _ApplyItemMods(pItem, i, true, false, true);

            if(pItem->IsContainer() && GetItemInterface()->IsBagSlot(i))
            {
                for(int32 e = 0; e < pItem->GetProto()->ContainerSlots; e++)
                {
                    Item* item = TO_CONTAINER(pItem)->GetItem(e);
                    if(item)
                    {
                        item->PushToWorld(m_mapMgr);
                    }
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
                pItem->RemoveFromWorld();
            }

            if(pItem->IsContainer() && GetItemInterface()->IsBagSlot(i))
            {
                for(int32 e = 0; e < pItem->GetProto()->ContainerSlots; e++)
                {
                    Item* item = TO_CONTAINER(pItem)->GetItem(e);
                    if(item && item->IsInWorld())
                        item->RemoveFromWorld();
                }
            }
        }
    }

    UpdateStats();
}

uint32 Player::BuildCreateUpdateBlockForPlayer(ByteBuffer *data, Player* target )
{
    int count = 0;
    if(target == TO_PLAYER(this)) // we need to send create objects for all items.
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
    if(!delay)
    {
        m_KickDelay = 0;
        _Kick();
    } else {
        m_KickDelay = delay;
        sEventMgr.AddEvent(TO_PLAYER(this), &Player::_Kick, EVENT_PLAYER_KICK, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Player::_Kick()
{
    if(!m_KickDelay)
    {
        sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_PLAYER_KICK);
        // remove now
        //GetSession()->LogoutPlayer(true);
        m_session->Disconnect();
    } else {
        if((m_KickDelay - 1000) < 500)
        {
            m_KickDelay = 0;
        } else {
            m_KickDelay -= 1000;
        }
        sChatHandler.BlueSystemMessageToPlr(TO_PLAYER(this), "You will be removed from the server in %u seconds.", (uint32)(m_KickDelay/1000));
    }
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

void Player::ClearCooldownsOnLines(set<uint32> skill_lines, uint32 called_from)
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
    set<uint32> skilllines;
    uint32 guid = (uint32)GetSelection();
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
    ClearCooldownsOnLines(skilllines, guid);
}

void Player::sendMOTD()
{
    // Send first line of MOTD
    WorldPacket datat(SMSG_MOTD, 10);
    datat << uint32(1);
    datat << sWorld.GetMotd();
    SendPacket(&datat);

    // Send revision
    BroadcastMessage("%sServer:|r%s Sandshroud Hearthstone %s|r %s r%u/%s-%s-%s", MSG_COLOR_GOLD,
        MSG_COLOR_ORANGEY, MSG_COLOR_TORQUISEBLUE, BUILD_TAG, BUILD_REVISION, BUILD_HASH_STR, ARCH, CONFIG);
    BroadcastMessage("%sPlease report all bugs to |r%shttps://github.com/Sandshroud|r", MSG_COLOR_GOLD, MSG_COLOR_TORQUISEBLUE);
    BroadcastMessage("%sOnline Players:|r%s %u |r%sPeak:|r%s %u |r%sAccepted Connections:|r%s %u |r", MSG_COLOR_GOLD,
        MSG_COLOR_TORQUISEBLUE, sWorld.GetSessionCount(), MSG_COLOR_GOLD, MSG_COLOR_TORQUISEBLUE,
        sWorld.PeakSessionCount, MSG_COLOR_GOLD, MSG_COLOR_TORQUISEBLUE, sWorld.mAcceptedConnections);

    BroadcastMessage("%sServer Uptime:|r%s %s|r", MSG_COLOR_GOLD, MSG_COLOR_TORQUISEBLUE, sWorld.GetUptimeString().c_str());

    //Issue a message telling all guild members that this player has signed on
    guildmgr.PlayerLoggedIn(m_playerInfo);
}

void Player::PushUpdateData(ByteBuffer *data, uint32 updatecount)
{
    // imagine the bytebuffer getting appended from 2 threads at once! :D
    _bufferS.Acquire();

    // unfortunately there is no guarantee that all data will be compressed at a ratio
    // that will fit into 2^16 bytes ( stupid client limitation on server packets )
    // so if we get more than 63KB of update data, force an update and then append it
    // to the clean buffer.
    if( (data->size() + bUpdateDataBuffer.size() ) >= 45000 )
    {
        if( IsInWorld() ) // With our allocated resources already existing, use those instead
            ProcessPendingUpdates(&m_mapMgr->m_updateBuildBuffer, &m_mapMgr->m_compressionBuffer);
        else
            ProcessPendingUpdates(NULL, NULL);
    }

    mUpdateDataCount += updatecount;
    bUpdateDataBuffer.append(*data);

    // add to process queue
    if(m_mapMgr && !bProcessPending)
    {
        bProcessPending = true;
        m_mapMgr->PushToProcessed(TO_PLAYER(this));
    }

    _bufferS.Release();
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
        m_mapMgr->PushToProcessed(TO_PLAYER(this));
    }
    _bufferS.Release();
}

void Player::ProcessPendingUpdates(ByteBuffer *pBuildBuffer, ByteBuffer *pCompressionBuffer)
{
    _bufferS.Acquire();
    if(!bUpdateDataBuffer.size() && !mOutOfRangeIds.size() && !delayedPackets.size())
    {
        _bufferS.Release();
        return;
    }

    if(bUpdateDataBuffer.size() || mOutOfRangeIds.size())
    {
        size_t bBuffer_size = 6 + bUpdateDataBuffer.size() + (4 + 1 + mOutOfRangeIds.size() * 9);
        uint8 *update_buffer = NULL;
        if(pBuildBuffer != NULL)
        {
            pBuildBuffer->resize(bBuffer_size);
            update_buffer = (uint8*)pBuildBuffer->contents();
        }
        else update_buffer = new uint8[bBuffer_size];
        size_t c = 0;

        *(uint16*)&update_buffer[c] = (uint16)GetMapId();
        c += 2;
        *(uint32*)&update_buffer[c] = mUpdateDataCount + (mOutOfRangeIds.size() ? 1 : 0);
        c += 4;

        // append any out of range updates
        if(mOutOfRangeIdCount)
        {
            update_buffer[c] = UPDATETYPE_OUT_OF_RANGE_OBJECTS;
            ++c;
            *(uint32*)&update_buffer[c]  = mOutOfRangeIdCount;
            c += 4;
            memcpy(&update_buffer[c], mOutOfRangeIds.contents(), mOutOfRangeIds.size());
            c += mOutOfRangeIds.size();
            mOutOfRangeIds.clear();
            mOutOfRangeIdCount = 0;
        }

        if(mUpdateDataCount)
        {
            memcpy(&update_buffer[c], bUpdateDataBuffer.contents(), bUpdateDataBuffer.size());
            c += bUpdateDataBuffer.size();
            // clear our creation buffer
            bUpdateDataBuffer.clear();
            mUpdateDataCount = 0;
        }

        // compress update packet
        //if(c < size_t(500) || !CompressAndSendUpdateBuffer((uint32)c, update_buffer, pCompressionBuffer))
        {
            // send uncompressed packet -> because we failed
            m_session->OutPacket(SMSG_UPDATE_OBJECT, (uint16)c, update_buffer);
        }

        bProcessPending = false;
        _bufferS.Release();
        if(pBuildBuffer != NULL)
            pBuildBuffer->clear();
        else
            delete [] update_buffer;
    }

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
    int rate = 3;
    if(size >= 40000 && rate < 6)
        rate = 6;
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
        else
            delete [] buffer;
        return false;
    }

    // finish the deflate
    if(deflate(&stream, Z_FINISH) != Z_STREAM_END)
    {
        sLog.outDebug("deflate failed: did not end stream");
        if(pCompressionBuffer != NULL)
            pCompressionBuffer->clear();
        else
            delete [] buffer;
        return false;
    }

    // finish up
    if(deflateEnd(&stream) != Z_OK)
    {
        sLog.outDebug("deflateEnd failed.");
        if(pCompressionBuffer != NULL)
            pCompressionBuffer->clear();
        else
            delete [] buffer;
        return false;
    }

    // fill in the full size of the compressed stream
    WorldPacket data(SMSG_COMPRESSED_UPDATE_OBJECT, destsize);
    data << uint32(stream.total_in);
    data.append(buffer, uint32(stream.total_out));

    // send it
    SendPacket(&data);

    // cleanup memory
    if(pCompressionBuffer != NULL)
        pCompressionBuffer->clear();
    else
        delete [] buffer;
    return true;
}

void Player::ClearAllPendingUpdates()
{
    _bufferS.Acquire();
    bProcessPending = false;
    mUpdateDataCount = 0;
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
    pTarget->DuelingWith = TO_PLAYER(this);
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
    pGameObj->SetUInt64Value(OBJECT_FIELD_CREATED_BY, GetGUID());
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
        SetUInt32Value( UNIT_FIELD_POWER2, 0 );
        DuelingWith->SetUInt32Value( UNIT_FIELD_POWER2, 0 );

        //Give the players a Team
        DuelingWith->SetUInt32Value( PLAYER_DUEL_TEAM, 1 ); // Duel Requester
        SetUInt32Value( PLAYER_DUEL_TEAM, 2 );

        SetDuelState( DUEL_STATE_STARTED );
        DuelingWith->SetDuelState( DUEL_STATE_STARTED );

        sEventMgr.AddEvent(TO_PLAYER(this), &Player::DuelBoundaryTest, EVENT_PLAYER_DUEL_BOUNDARY_CHECK, 500, 0, 0 );
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
    sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_PLAYER_DUEL_COUNTDOWN );
    sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_PLAYER_DUEL_BOUNDARY_CHECK );

    m_AuraInterface.UpdateDuelAuras();

    m_duelState = DUEL_STATE_FINISHED;
    if( DuelingWith == NULL )
        return;

    sEventMgr.RemoveEvents( DuelingWith, EVENT_PLAYER_DUEL_BOUNDARY_CHECK );
    sEventMgr.RemoveEvents( DuelingWith, EVENT_PLAYER_DUEL_COUNTDOWN );

    // spells waiting to hit
    sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_SPELL_DAMAGE_HIT);

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

    GameObject* arbiter = m_mapMgr ? GetMapMgr()->GetGameObject(GUID_LOPART(GetUInt64Value(PLAYER_DUEL_ARBITER))) : NULLGOB;

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
        GetSummon()->GetAIInterface()->SetUnitToFollow( TO_PLAYER(this) );
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
    DuelingWith->smsg_AttackStop( TO_PLAYER(this) );

    DuelingWith->m_duelCountdownTimer = 0;
    m_duelCountdownTimer = 0;

    DuelingWith->DuelingWith = NULLPLR;
    DuelingWith = NULLPLR;
}

void Player::StopMirrorTimer(uint32 Type)
{
    m_session->OutPacket(SMSG_STOP_MIRROR_TIMER, 4, &Type);
}

void Player::EventTeleport(uint32 mapid, float x, float y, float z, float o = 0.0f, int32 phase)
{
    SafeTeleport(mapid, 0, LocationVector(x, y, z, o), phase);
    sEventMgr.RemoveEvents(this,EVENT_PLAYER_TELEPORT);
}

void Player::ApplyLevelInfo(uint32 Level)
{
    // Apply level
    uint32 PreviousLevel = GetUInt32Value(UNIT_FIELD_LEVEL);
    SetUInt32Value(UNIT_FIELD_LEVEL, Level);

    CalculateBaseStats();

    // Set stats
    if(lvlinfo != NULL)
    {
        for(uint32 i = 0; i < 5; i++)
        {
            BaseStats[i] = lvlinfo->Stat[i];
            CalcStat(i);
        }
    }

    _UpdateMaxSkillCounts();
    if (m_playerInfo)
        m_playerInfo->lastLevel = Level;

    if(IsInWorld())
    {
        UpdateStats();
        int32 Talents = Level - PreviousLevel;
        if(PreviousLevel < 9)
            Talents = Level - 9;
        if(Talents < 0)
            Reset_Talents(true);
        else if(Level >= 10)
        {
            // Set talents
        }

        SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
        SetUInt32Value(UNIT_FIELD_POWER1, GetUInt32Value(UNIT_FIELD_MAXPOWER1));

        GetAchievementInterface()->HandleAchievementCriteriaLevelUp( getLevel() );
        InitGlyphsForLevel();
        smsg_TalentsInfo(false);
        UpdateNearbyQuestGivers(); // For quests that require levels
    }

    sLog.outDebug("Player %s set parameters to level %u", GetName(), Level);
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
    uint32 reallevel = m_uint32Values[UNIT_FIELD_LEVEL];
    uint32 level = reallevel > MAXIMUM_ATTAINABLE_LEVEL ? MAXIMUM_ATTAINABLE_LEVEL : reallevel;
    gtFloat * pDBCEntry = dbcCombatRating.LookupEntry( relative_index * 100 + level - 1 );
    float val = 1.0f;
    if( pDBCEntry != NULL )
        val = pDBCEntry->val;

    return float(rating/val);
}

bool Player::SafeTeleport(uint32 MapID, uint32 InstanceID, float X, float Y, float Z, float O, int32 phase)
{
    return SafeTeleport(MapID, InstanceID, LocationVector(X, Y, Z, O), phase);
}

bool Player::SafeTeleport(uint32 MapID, uint32 InstanceID, LocationVector vec, int32 phase)
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

void Player::SafeTeleport(MapMgr* mgr, LocationVector vec, int32 phase)
{
    if(IsInWorld())
        RemoveFromWorld();

    m_mapId = mgr->GetMapId();
    m_instanceId = mgr->GetInstanceID();
    WorldPacket data(SMSG_TRANSFER_PENDING, 20);
    data << mgr->GetMapId();
    GetSession()->SendPacket(&data);

    data.Initialize(SMSG_NEW_WORLD);
    data << mgr->GetMapId();
    data.appendvector(vec, true);
    GetSession()->SendPacket(&data);

    SetPlayerStatus(TRANSFER_PENDING);
    m_sentTeleportPosition = vec;
    SetPosition(vec);
    ResetHeartbeatCoords();

    int32 phase2 = GetPhaseForArea(GetAreaId());
    if(phase2 != 1 && phase == 1)
        phase = phase2;
    SetPhaseMask(phase, false);

    if(GetShapeShift())
    {
        // Extra Check
        SetShapeShift(GetShapeShift());
    }

    if(DuelingWith != NULL)
    {
        EndDuel(DUEL_WINNER_RETREAT);
    }
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
    Object* curObj;
    for (Object::InRangeSet::iterator iter = GetInRangeSetBegin(); iter != GetInRangeSetEnd();)
    {
        curObj = *iter;
        iter++;
        if(curObj->IsPlayer())
        {
            pGroup = TO_PLAYER( curObj )->GetGroup();
            if( pGroup == NULL || pGroup != GetGroup())
                BuildFieldUpdatePacket( TO_PLAYER( curObj ), index, flag );
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
        CastSpell(TO_UNIT(this), PLAYER_HONORLESS_TARGET_SPELL, true);

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

void Player::CalculateBaseStats()
{
    if(!lvlinfo)
        return;

    memcpy(BaseStats, lvlinfo->Stat, sizeof(uint32) * 5);

    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, lvlinfo->XPToNextLevel);
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, lvlinfo->BaseHP);
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, lvlinfo->HP);
    SetUInt32Value(UNIT_FIELD_BASE_MANA, lvlinfo->BaseMana);
    SetUInt32Value(UNIT_FIELD_MAXPOWER1, lvlinfo->Mana);
}

void Player::CompleteLoading()
{
    // cast passive initial spells    -- grep note: these shouldn't require plyr to be in world
    SpellSet::iterator itr;
    SpellEntry *info;
    SpellCastTargets targets;
    targets.m_unitTarget = GetGUID();
    targets.m_targetMask = TARGET_FLAG_UNIT;

    for (itr = mSpells.begin(); itr != mSpells.end(); itr++)
    {
        info = dbcSpell.LookupEntry(*itr);

        if( info  && (info->Attributes & ATTRIBUTES_PASSIVE) && !( info->c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET ))
        {
            if( info->RequiredShapeShift )
            {
                if( !( ((uint32)1 << (GetShapeShift()-1)) & info->RequiredShapeShift ) )
                    continue;
            }

            Spell* spell = new Spell(this, info, true, NULLAURA);
            spell->prepare(&targets);
        }
    }

    ApplySpec(m_talentActiveSpec, true);
    if(!isDead())//only add aura's to the living (death aura set elsewhere)
    {
        std::list<LoginAura>::iterator i,i2;
        for(i = loginauras.begin(); i != loginauras.end();)
        {
            i2 = i;
            ++i;

            // this stuff REALLY needs to be fixed - Burlex
            SpellEntry* sp = dbcSpell.LookupEntry((*i2).id);

            //do not load auras that only exist while pet exist. We should recast these when pet is created anyway
            if ( sp->c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET )
                continue;

            Aura* a = new Aura(sp, (*i2).dur, TO_OBJECT(this),TO_UNIT(this));
            for(uint32 x = 0; x < 3; x++)
            {
                if(sp->Effect[x] == SPELL_EFFECT_APPLY_AURA)
                    a->AddMod(sp->EffectApplyAuraName[x], sp->EffectBasePoints[x]+1, sp->EffectMiscValue[x], x);
            }
            AddAura(a);
            loginauras.erase(i2);
        }
        loginauras.clear();

        // warrior has to have battle stance
        if( getClass() == WARRIOR && !HasAura(2457))
            CastSpell(TO_UNIT(this), dbcSpell.LookupEntry(2457), true);
    }
    // this needs to be after the cast of passive spells, because it will cast ghost form, after the remove making it in ghost alive, if no corpse.

    if(iActivePet)
        SpawnPet(iActivePet);      // only spawn if >0

    // Banned
    if(IsBanned())
    {
        const char * message = ("This character is banned for  %s.\n You will be kicked in 30 secs.", GetBanReason().c_str());

        // Send warning after 30sec, as he might miss it if it's send inmedeately.
        sEventMgr.AddEvent( TO_PLAYER(this), &Player::_Warn, message, EVENT_UNIT_SENDMESSAGE, 30000, 1, 0);
        sEventMgr.AddEvent( TO_PLAYER(this), &Player::_Kick, EVENT_PLAYER_KICK, 60000, 1, 0 );
    }

    if(m_playerInfo->m_Group)
        sEventMgr.AddEvent(TO_PLAYER(this), &Player::EventGroupFullUpdate, EVENT_UNK, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    if(raidgrouponlysent)
    {
        WorldPacket data2(SMSG_RAID_GROUP_ONLY, 8);
        data2 << uint32(0xFFFFFFFF) << uint32(0);
        GetSession()->SendPacket(&data2);
        raidgrouponlysent=false;
    }

    sInstanceMgr.BuildSavedInstancesForPlayer(TO_PLAYER(this));
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

        if(pPMapinfo->phasehorde != 0 && pPMapinfo->phasealliance !=0)
        {
            if(GetSession()->HasGMPermissions())
                SetPhaseMask(pPMapinfo->phasehorde | pPMapinfo->phasealliance);
            else if(GetTeam())
                SetPhaseMask(pPMapinfo->phasehorde);
            else
                SetPhaseMask(pPMapinfo->phasealliance);
        }
        if(info != NULL && (pPMapinfo->phasehorde == 0 && pPMapinfo->phasealliance == 0 ))
            if(GetPhaseMask() == 1)
                SetPhaseMask(GetPhaseForArea(GetAreaId()), false);

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
                welcome_msg += ConvertTimeStampToString((uint32)m_mapMgr->pInstance->m_expiration - UNIXTIME);
            }
            sChatHandler.SystemMessage(m_session, welcome_msg.c_str());
        }
    }

    ResetHeartbeatCoords();
}

void Player::ModifyBonuses(uint32 type,int32 val)
{
    // Added some updateXXXX calls so when an item modifies a stat they get updated
    // also since this is used by auras now it will handle it for those
    switch (type)
    {
    case POWER:
        ModUnsigned32Value( UNIT_FIELD_MAXPOWER1, val );
        m_manafromitems += val;
        break;
    case HEALTH:
        ModUnsigned32Value( UNIT_FIELD_MAXHEALTH, val );
        m_healthfromitems += val;
        break;
    case AGILITY: // modify agility
        FlatStatModPos[1] += val;
        CalcStat( 1 );
        break;
    case STRENGTH: //modify strength
        FlatStatModPos[0] += val;
        CalcStat( 0 );
        break;
    case INTELLECT: //modify intellect
        FlatStatModPos[3] += val;
        CalcStat( 3 );
        break;
     case SPIRIT: //modify spirit
        FlatStatModPos[4] += val;
        CalcStat( 4 );
        break;
    case STAMINA: //modify stamina
        FlatStatModPos[2] += val;
        CalcStat( 2 );
        break;
    case WEAPON_SKILL_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_SKILL, val ); // ranged
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_MAIN_HAND_SKILL, val ); // melee main hand
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_OFF_HAND_SKILL, val ); // melee off hand
        }break;
    case DEFENSE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_DEFENCE, val );
        }break;
    case DODGE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_DODGE, val );
        }break;
    case PARRY_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_PARRY, val );
        }break;
    case SHIELD_BLOCK_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_BLOCK, val );
        }break;
    case MELEE_HIT_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_HIT, val );
        }break;
    case RANGED_HIT_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_HIT, val );
        }break;
    case SPELL_HIT_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_SPELL_HIT, val );
        }break;
    case MELEE_CRITICAL_STRIKE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_CRIT, val );
        }break;
    case RANGED_CRITICAL_STRIKE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_CRIT, val );
        }break;
    case SPELL_CRITICAL_STRIKE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_SPELL_CRIT, val );
        }break;
    case MELEE_HIT_AVOIDANCE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_HIT_AVOIDANCE, val );
        }break;
    case RANGED_HIT_AVOIDANCE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_HIT_AVOIDANCE, val );
        }break;
    case SPELL_HIT_AVOIDANCE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_SPELL_HIT_AVOIDANCE, val );
        }break;
    case MELEE_CRITICAL_AVOIDANCE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE, val );//melee
        }break;
    case RANGED_CRITICAL_AVOIDANCE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_RESILIENCE, val );//ranged
        }break;
    case SPELL_CRITICAL_AVOIDANCE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE, val );//spell
        }break;
    case MELEE_HASTE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_HASTE, val );//melee
        }break;
    case RANGED_HASTE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_HASTE, val );//ranged
        }break;
    case SPELL_HASTE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_SPELL_HASTE, val );//spell
        }break;
    case HIT_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_HIT, val );//melee
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_HIT, val );//ranged
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_SPELL_HIT, val );
        }break;
    case CRITICAL_STRIKE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_CRIT, val );//melee
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_CRIT, val );//ranged
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_SPELL_CRIT, val );
        }break;
    case HIT_AVOIDANCE_RATING:// this is guessed based on layout of other fields
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_HIT_AVOIDANCE, val );//melee
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_HIT_AVOIDANCE, val );//ranged
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_SPELL_HIT_AVOIDANCE, val );//spell
        }break;
    case CRITICAL_AVOIDANCE_RATING:
        {
            // todo. what is it?
        }break;
    case EXPERTISE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_EXPERTISE, val );
        }break;
    case RESILIENCE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE, val );//melee
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_RESILIENCE, val );//ranged
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE, val );//spell
        }break;
    case HASTE_RATING:
        {
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_MELEE_HASTE, val );//melee
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_RANGED_HASTE, val );//ranged
            ModUnsigned32Value( PLAYER_RATING_MODIFIER_SPELL_HASTE, val );
        }break;
    case ATTACK_POWER:
        {
            ModUnsigned32Value( UNIT_FIELD_ATTACK_POWER_MOD_POS, val );
            ModUnsigned32Value( UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS, val );
        }break;
    case RANGED_ATTACK_POWER:
        {
            ModUnsigned32Value( UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS, val );
        }break;
    case FERAL_ATTACK_POWER:
        {
            m_feralAP += val;
        }break;
    case SPELL_HEALING_DONE:
        {
            HealDoneBase += val;
        }break;
    case SPELL_DAMAGE_DONE:
        {
            for( uint8 school = 1; school < 7; ++school )
                DamageDonePosMod[school] += val;
        }break;
    case MANA_REGENERATION:
        {
            m_ModInterrMRegen += val;
        }break;
    case ARMOR_PENETRATION_RATING:
        {
            ModUnsigned32Value(PLAYER_RATING_MODIFIER_ARMOR_PENETRATION_RATING, val);
        }break;
    case SPELL_POWER:
        {
            for( uint8 school = 1; school < 7; ++school )
                DamageDonePosMod[ school ] += val;
            HealDoneBase += val;
        }break;
    case HEALTH_REGEN:
        {
            PctRegenModifier += val;
        }break;
    case SPELL_PENETRATION:
        {
            for( uint8 school = 1; school < 7; ++school )
                PowerCostPctMod[school] += val;
        }break;
    }
    UpdateStats();
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

void Player::SaveAuras(stringstream& ss)
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
    set<uint32>::iterator itr;
    SpellEntry * sp;
    Spell* spe = NULLSPELL;
    SpellCastTargets t(GetGUID());

    for( itr = mSpells.begin(); itr != mSpells.end(); itr++ )
    {
        sp = dbcSpell.LookupEntry( *itr );
        if( sp == NULL)
            continue;
        if( sp->apply_on_shapeshift_change || sp->Attributes & ATTRIBUTES_PASSIVE )     // passive/talent
        {
            if( sp->RequiredShapeShift && ((uint32)1 << (ss-1)) & sp->RequiredShapeShift )
            {
                spe = (new Spell( this, sp, true, NULLAURA ));
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
            spe = (new Spell( this, sp, true, NULLAURA ));
            spe->prepare( &t );
        }
    }

    // kill speedhack detection for 2 seconds (not needed with new code but bleh)
    DelaySpeedHack( 2000 );

    UpdateStats();
}

void Player::CalcDamage()
{
    int ss = GetShapeShift();
    float ap_bonus = GetAP()/14000.0f, r = 0.0f, delta = (float)GetUInt32Value( PLAYER_FIELD_MOD_DAMAGE_DONE_POS ) - (float)GetUInt32Value( PLAYER_FIELD_MOD_DAMAGE_DONE_NEG );

/////////////////MAIN HAND
    if(IsInFeralForm())
    {
        uint32 lev = getLevel();

        if(ss == FORM_CAT)
            r = lev + delta + ap_bonus * 1000.0f;
        else
            r = lev + delta + ap_bonus * 2500.0f;

        //SetFloatValue(UNIT_FIELD_MINDAMAGE,r);
        //SetFloatValue(UNIT_FIELD_MAXDAMAGE,r);

        r *= 0.9f;
        r *= 1.1f;

        SetFloatValue(UNIT_FIELD_MINDAMAGE,r>0?r:0);
        SetFloatValue(UNIT_FIELD_MAXDAMAGE,r>0?r:0);

        return;
    }
//////no druid ss
    uint32 speed=2000;
    Item* it = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

    if(!disarmed && it != NULL)
        speed = it->GetProto()->Delay;

    float bonus = ap_bonus*speed;
    float tmp = GetDamageDonePctMod(SCHOOL_NORMAL);

    r = BaseDamage[0]+delta+bonus;
    r *= tmp;
    if( it )
        r *= m_WeaponSubClassDamagePct[it->GetProto()->SubClass];

    SetFloatValue(UNIT_FIELD_MINDAMAGE,r>0?r:0);

    r = BaseDamage[1]+delta+bonus;
    r *= tmp;
    if( it )
        r *= m_WeaponSubClassDamagePct[it->GetProto()->SubClass];

    SetFloatValue(UNIT_FIELD_MAXDAMAGE,r>0?r:0);

    uint32 cr = 0;
    if( it )
    {
        if( m_wratings.size() )
        {
            std::map<uint32, uint32>::iterator itr = m_wratings.find( it->GetProto()->SubClass );
            if( itr != m_wratings.end() )
                cr=itr->second;
        }
    }
    SetUInt32Value( PLAYER_RATING_MODIFIER_MELEE_MAIN_HAND_SKILL, cr );
/////////////// MAIN HAND END

/////////////// OFF HAND START
    cr = 0;
    it = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if(it)
    {
        if(!disarmed)
        {
            speed =it->GetProto()->Delay;
        }
        else speed  = 2000;

        bonus = ap_bonus * speed;

        r = (BaseOffhandDamage[0]+delta+bonus)*offhand_dmg_mod;
        r *= tmp;
        r *= m_WeaponSubClassDamagePct[it->GetProto()->SubClass];
        SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE,r>0?r:0);

        r = (BaseOffhandDamage[1]+delta+bonus)*offhand_dmg_mod;
        r *= tmp;
        r *= m_WeaponSubClassDamagePct[it->GetProto()->SubClass];
        SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE,r>0?r:0);

        if(m_wratings.size ())
        {
            std::map<uint32, uint32>::iterator itr=m_wratings.find(it->GetProto()->SubClass);
            if(itr!=m_wratings.end())
                cr=itr->second;
        }
    }
    SetUInt32Value( PLAYER_RATING_MODIFIER_MELEE_OFF_HAND_SKILL, cr );

/////////////second hand end
///////////////////////////RANGED
    cr = 0;
    if((it = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)))
    {
        if(it->GetProto()->SubClass != 19)//wands do not have bonuses from RAP & ammo
        {
//              ap_bonus = (GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER)+(int32)GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS))/14000.0;
            //modified by Zack : please try to use premade functions if possible to avoid forgetting stuff
            ap_bonus = GetRAP()/14000.0f;
            bonus = ap_bonus*it->GetProto()->Delay;
        }else bonus = 0;

        r = BaseRangedDamage[0]+delta+bonus;
        r *= tmp;
        r *= m_WeaponSubClassDamagePct[it->GetProto()->SubClass];
        SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,r>0?r:0);

        r = BaseRangedDamage[1]+delta+bonus;
        r *= tmp;
        r *= m_WeaponSubClassDamagePct[it->GetProto()->SubClass];
        SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,r>0?r:0);

        if(m_wratings.size ())
        {
            std::map<uint32, uint32>::iterator i=m_wratings.find(it->GetProto()->SubClass);
            if(i != m_wratings.end())
                cr=i->second;
        }

    }
    SetUInt32Value( PLAYER_RATING_MODIFIER_RANGED_SKILL, cr );
/////////////////////////////////RANGED end
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
    else
        ap_bonus = GetAP()/14000.0f;
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
    uint32 speed=2000;
    Item* it = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    if(!disarmed)
    {
        if(it)
            speed = it->GetProto()->Delay;
    }
    float bonus = ap_bonus*speed;
    float tmp = 1;
    map<uint32, WeaponModifier>::iterator i;
    for(i = damagedone.begin();i!=damagedone.end();++i)
    {
        if((i->second.wclass == (uint32)-1) || //any weapon
            (it && ((1 << it->GetProto()->SubClass) & i->second.subclass) )
            )
                tmp+=i->second.value/100.0f;
    }

    r = BaseDamage[0]+delta+bonus;
    r *= tmp;
    min_dmg = r * 0.9f;
    r = BaseDamage[1]+delta+bonus;
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
    // fuck bytebuffers :D
    unsigned char buffer[10];
    uint32 c = 2;

    // check for overflow
    if(m_comboPoints > 5)
        m_comboPoints = 5;

    if(m_comboPoints < 0)
        m_comboPoints = 0;

    if(m_comboTarget != 0)
    {
        Unit* target = (m_mapMgr != NULL) ? m_mapMgr->GetUnit(m_comboTarget) : NULLUNIT;
        if(!target || target->isDead() || GetSelection() != m_comboTarget)
        {
            buffer[0] = buffer[1] = 0;
        }
        else
        {
            c = FastGUIDPack(m_comboTarget, buffer, 0);
            buffer[c++] = m_comboPoints;
        }
    }
    else
        buffer[0] = buffer[1] = 0;

    m_session->OutPacket(SMSG_UPDATE_COMBO_POINTS, c, buffer);
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
        TO_PLAYER(pTarget)->ResetHeartbeatCoords();

    m_CurrentCharm = pTarget;
    if(pTarget->GetTypeId() == TYPEID_UNIT)
    {
        // unit-only stuff.
        pTarget->DisableAI();
        pTarget->GetAIInterface()->StopMovement(0);
        pTarget->m_redirectSpellPackets = TO_PLAYER(this);
    }

    m_noInterrupt++;
    SetUInt64Value(UNIT_FIELD_CHARM, pTarget->GetGUID());
    SetUInt64Value(PLAYER_FARSIGHT, pTarget->GetGUID());
    pTarget->GetMapMgr()->ChangeFarsightLocation(TO_PLAYER(this), pTarget, true);

    pTarget->SetUInt64Value(UNIT_FIELD_CHARMEDBY, GetGUID());
    pTarget->SetCharmTempVal(pTarget->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    pTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    pTarget->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    /* send "switch mover" packet */
    WorldPacket data1(SMSG_CLIENT_CONTROL_UPDATE, 10);      /* burlex: this should be renamed SMSG_SWITCH_ACTIVE_MOVER :P */
    data1 << pTarget->GetNewGUID() << uint8(1);
    m_session->SendPacket(&data1);

    /* update target faction set */
    pTarget->_setFaction();

    /* build + send pet_spells packet */
    if(pTarget->m_temp_summon)
        return;

    list<uint32> avail_spells;
    for(map<uint32, AI_Spell*>::iterator itr = pTarget->GetAIInterface()->m_spells.begin(); itr != pTarget->GetAIInterface()->m_spells.end(); ++itr)
        avail_spells.push_back(itr->second->info->Id);
    list<uint32>::iterator itr = avail_spells.begin();

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
    m_CurrentCharm = NULLUNIT;

    if(pTarget->GetTypeId() == TYPEID_UNIT)
    {
        // unit-only stuff.
        pTarget->EnableAI();
        pTarget->m_redirectSpellPackets = NULLPLR;
    }

    ResetHeartbeatCoords();
    DelaySpeedHack(5000);
    if( pTarget->IsPlayer() )
        TO_PLAYER(pTarget)->DelaySpeedHack(5000);

    m_noInterrupt--;
    SetUInt64Value(PLAYER_FARSIGHT, 0);
    pTarget->GetMapMgr()->ChangeFarsightLocation(TO_PLAYER(this), pTarget, false);
    SetUInt64Value(UNIT_FIELD_CHARM, 0);
    pTarget->SetUInt64Value(UNIT_FIELD_CHARMEDBY, 0);

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);
    pTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE);
    pTarget->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, pTarget->GetCharmTempVal());
    pTarget->_setFaction();

    /* send "switch mover" packet */
    WorldPacket data(SMSG_CLIENT_CONTROL_UPDATE, 10);
    data << GetNewGUID() << uint8(1);
    m_session->SendPacket(&data);

    if(pTarget->m_temp_summon)
        return;

    data.Initialize(SMSG_PET_SPELLS);
    data << uint64(0);
    m_session->SendPacket(&data);
}

void Player::SummonRequest(Object* Requestor, uint32 ZoneID, uint32 MapID, uint32 InstanceID, const LocationVector & Position)
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
        m_pendingBattleground[queueSlot]->RemovePendingPlayer(TO_PLAYER(this));
        m_pendingBattleground[queueSlot] = NULLBATTLEGROUND;
    }
    m_bgIsQueued[queueSlot] = false;
    m_bgQueueType[queueSlot] = 0;
    m_bgQueueInstanceId[queueSlot] = 0;

    BattlegroundManager.SendBattlegroundQueueStatus(TO_PLAYER(this), queueSlot);
    sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_BATTLEGROUND_QUEUE_UPDATE_SLOT_1 + queueSlot);

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
        inf.BonusValue = 0;
        m_skills.insert( make_pair( SkillLine, inf ) );
        _UpdateSkillFields();
    }

    //Add to proficiency
    if(( prof = (ItemProf *)GetProficiencyBySkill(SkillLine)) )
    {
        packetSMSG_SET_PROFICICENCY pr;
        pr.ItemClass = prof->itemclass;
        if(prof->itemclass==4)
        {
                armor_proficiency|=prof->subclass;
                //SendSetProficiency(prof->itemclass,armor_proficiency);
                pr.Profinciency = armor_proficiency;
        }
        else
        {
                weapon_proficiency|=prof->subclass;
                //SendSetProficiency(prof->itemclass,weapon_proficiency);
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

void Player::_UpdateSkillFields()
{
    uint32 f = PLAYER_SKILL_INFO_1_1;
    /* Set the valid skills */
    for(SkillMap::iterator itr = m_skills.begin(); itr != m_skills.end();)
    {
        if(!itr->first)
        {
            SkillMap::iterator it2 = itr++;

            m_skills.erase(it2);
            continue;
        }

        ASSERT(f < PLAYER_MAX_SKILL_INFO_FIELD);
        if(itr->second.Skill->type == SKILL_TYPE_PROFESSION)
            SetUInt32Value(f++, itr->first | 0x10000);
        else
            SetUInt32Value(f++, itr->first);

        uint32 combinedData = uint32(uint16(itr->second.MaximumValue)) << 16;
        SetUInt32Value(f, combinedData|itr->second.CurrentValue);
        f++;

        SetUInt32Value(f++, itr->second.BonusValue);

        GetAchievementInterface()->HandleAchievementCriteriaReachSkillLevel( itr->second.Skill->id, itr->second.CurrentValue );
        ++itr;
    }

    /* Null out the rest of the fields */
    for(; f < PLAYER_MAX_SKILL_INFO_FIELD; ++f)
    {
        if(m_uint32Values[f] != 0)
            SetUInt32Value(f, 0);
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
        itr->second.CurrentValue = min(curr_sk + Count,itr->second.MaximumValue);
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
            if(itr->second.Skill == NULL)
            {
                m_skills.erase(itr->first);
                continue;
            }

            if( itr->second.Skill->id == SKILL_LOCKPICKING )
            {
                new_max = 5 * getLevel();
            }
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
                    default:
                        new_max = 1;
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
    if( m_skills.empty() )
        return;

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

            m_skills.erase(it2);
        }
        else
            ++itr;
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
    SkillLineEntry * en;
    uint32 spell_id;
    static uint32 skills[] = { SKILL_LANG_COMMON, SKILL_LANG_ORCISH, SKILL_LANG_DWARVEN, SKILL_LANG_DARNASSIAN, SKILL_LANG_TAURAHE, SKILL_LANG_THALASSIAN,
        SKILL_LANG_TROLL, SKILL_LANG_GUTTERSPEAK, SKILL_LANG_DRAENEI, 0 };

    if(All)
    {
        for(uint32 i = 0; skills[i] != 0; i++)
        {
            if(!skills[i])
                break;

            if(!sk.Reset(skills[i]))
                continue;

            sk.MaximumValue = sk.CurrentValue = 300;
            m_skills.insert( make_pair(skills[i], sk) );
            if((spell_id = ::GetSpellForLanguageSkill(skills[i])))
                addSpell(spell_id);
        }
    }
    else
    {
        for(list<CreateInfo_SkillStruct>::iterator itr = info->skills.begin(); itr != info->skills.end(); itr++)
        {
            en = dbcSkillLine.LookupEntry(itr->skillid);
            if(en == NULL)
                continue;

            if(en->type == SKILL_TYPE_LANGUAGE)
            {
                if(sk.Reset(itr->skillid))
                {
                    sk.MaximumValue = sk.CurrentValue = 300;
                    m_skills.insert( make_pair(itr->skillid, sk) );
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

void Player::RemoveSpellTargets(uint32 Type)
{
    if(m_spellIndexTypeTargets[Type] != 0)
    {
        Unit* pUnit = m_mapMgr ? m_mapMgr->GetUnit(m_spellIndexTypeTargets[Type]) : NULLUNIT;
        if(pUnit)
            pUnit->m_AuraInterface.RemoveAllAurasByBuffIndexType(Type, GetGUID());

        m_spellIndexTypeTargets[Type] = 0;
    }
}

void Player::RemoveSpellIndexReferences(uint32 Type)
{
    m_spellIndexTypeTargets[Type] = 0;
}

void Player::SetSpellTargetType(uint32 Type, Unit* target)
{
    m_spellIndexTypeTargets[Type] = target->GetGUID();
}

void Player::RecalculateHonor()
{
    HonorHandler::RecalculateHonorFields(TO_PLAYER(this));
}

//wooot, crappy code rulez.....NOT
void Player::EventTalentHeartOfWildChange(bool apply)
{
    if(!hearth_of_wild_pct)
        return;

    //druid hearth of the wild should add more features based on form
    int tval;
    if(apply)
        tval = hearth_of_wild_pct;
    else tval = -hearth_of_wild_pct;

    uint32 SS=GetShapeShift();

    //increase stamina if :
    if(SS == FORM_BEAR || SS == FORM_DIREBEAR)
    {
        TotalStatModPctPos[STAT_STAMINA] += tval;
        CalcStat(STAT_STAMINA);
        UpdateStats();
    }
    //increase attackpower if :
    else if(SS == FORM_CAT)
    {
        SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER,GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER)+float(tval/200.0f));
        SetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER, GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER)+float(tval/200.0f));
        UpdateStats();
    }
}

void Player::EventGroupFullUpdate()
{
    if(m_playerInfo->m_Group)
    {
        //m_playerInfo->m_Group->Update();
        m_playerInfo->m_Group->UpdateAllOutOfRangePlayersFor(TO_PLAYER(this));
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

void Player::SendMeetingStoneQueue(uint32 DungeonId, uint8 Status)
{
    WorldPacket data(SMSG_MEETINGSTONE_SETQUEUE, 5);
    data << DungeonId << Status;
    m_session->SendPacket(&data);
}

void Player::PartLFGChannel()
{
    Channel * pChannel = channelmgr.GetChannel("LookingForGroup", TO_PLAYER(this));
    if( pChannel == NULL )
        return;

    /*for(list<Channel*>::iterator itr = m_channels.begin(); itr != m_channels.end(); itr++)
    {
        if( (*itr) == pChannel )
        {
            pChannel->Part(TO_PLAYER(this));
            return;
        }
    }*/
    if( m_channels.find( pChannel->m_channelId ) == m_channels.end() )
        return;

    pChannel->Part( TO_PLAYER(this), false );
}

//if we charmed or simply summoned a pet, TO_PLAYER(this) function should get called
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
        if( spellInfo->c_is_flags & SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_PET_OWNER )
        {
            m_AuraInterface.RemoveAllAuras( SpellID, GetGUID() ); //this is required since unit::addaura does not check for talent stacking
            SpellCastTargets targets( GetGUID() );
            Spell* spell = NULLSPELL;
            spell = (new Spell(TO_PLAYER(this), spellInfo ,true, NULLAURA));    //we cast it as a proc spell, maybe we should not !
            spell->prepare(&targets);
        }
        if( spellInfo->c_is_flags & SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_ON_PET )
        {
            m_AuraInterface.RemoveAllAuras( SpellID, GetGUID() ); //this is required since unit::addaura does not check for talent stacking
            SpellCastTargets targets( new_pet->GetGUID() );
            Spell* spell = NULLSPELL;
            spell = (new Spell(TO_PLAYER(this), spellInfo ,true, NULLAURA));    //we cast it as a proc spell, maybe we should not !
            spell->prepare(&targets);
        }
    }

    //there are talents that stop working after you gain pet
    m_AuraInterface.RemoveAllAurasByCIsFlag(SPELL_FLAG_IS_EXPIREING_ON_PET);

    //pet should inherit some of the talents from caster
    //new_pet->InheritSMMods(); //not required yet. We cast full spell to have visual effect too
}

//if pet/charm died or whatever happened we should call this function
//!! note function might get called multiple times :P
void Player::EventDismissPet()
{
    m_AuraInterface.RemoveAllAurasByCIsFlag(SPELL_FLAG_IS_EXPIREING_ON_PET);
}

void Player::AddShapeShiftSpell(uint32 id)
{
    SpellEntry * sp = dbcSpell.LookupEntry( id );
    mShapeShiftSpells.insert( id );

    if( sp->RequiredShapeShift && ((uint32)1 << (GetShapeShift()-1)) & sp->RequiredShapeShift )
    {
        Spell* spe = NULLSPELL;
        spe = (new Spell( TO_PLAYER(this), sp, true, NULLAURA ));
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

        m_cooldownMap[Type].insert( make_pair( Misc, cd ) );
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
            SM_FIValue(SM[SMT_COOLDOWN_DECREASE][0], &cool_time, pSpell->SpellGroupType);
            SM_PIValue(SM[SMT_COOLDOWN_DECREASE][1], &cool_time, pSpell->SpellGroupType);
        }

        _Cooldown_Add( COOLDOWN_TYPE_CATEGORY, pSpell->Category, mstime + cool_time, pSpell->Id, pItemCaster ? pItemCaster->GetProto()->ItemId : 0 );
    }

    if( pSpell->RecoveryTime > 0 )
    {
        cool_time = pSpell->RecoveryTime;
        if( pSpell->SpellGroupType )
        {
            SM_FIValue(SM[SMT_COOLDOWN_DECREASE][0], &cool_time, pSpell->SpellGroupType);
            SM_PIValue(SM[SMT_COOLDOWN_DECREASE][1], &cool_time, pSpell->SpellGroupType);
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
    if( m_floatValues[UNIT_MOD_CAST_SPEED] >= 1.0f )
        atime = pSpell->StartRecoveryTime;
    else
        atime = float2int32( float(pSpell->StartRecoveryTime) * m_floatValues[UNIT_MOD_CAST_SPEED] );

    if( pSpell->SpellGroupType )
    {
        SM_FIValue(SM[SMT_GLOBAL_COOLDOWN][0], &atime, pSpell->SpellGroupType);
        SM_PIValue(SM[SMT_GLOBAL_COOLDOWN][1], &atime, pSpell->SpellGroupType);
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
    int32 atime = float2int32( float(pSpell->StartRecoveryTime) * m_floatValues[UNIT_MOD_CAST_SPEED] );
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
        buf->AddQuery("DELETE FROM playercooldowns WHERE player_guid = %u", m_uint32Values[OBJECT_FIELD_GUID] );        // 0 is guid always
    else
        CharacterDatabase.Execute("DELETE FROM playercooldowns WHERE player_guid = %u", m_uint32Values[OBJECT_FIELD_GUID] );        // 0 is guid always

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
            {
                buf->AddQuery( "INSERT INTO playercooldowns VALUES(%u, %u, %u, %u, %u, %u)", m_uint32Values[OBJECT_FIELD_GUID],
                    i, itr2->first, seconds + (uint32)UNIXTIME, itr2->second.SpellId, itr2->second.ItemId );
            }
            else
            {
                CharacterDatabase.Execute( "INSERT INTO playercooldowns VALUES(%u, %u, %u, %u, %u, %u)", m_uint32Values[OBJECT_FIELD_GUID],
                    i, itr2->first, seconds + (uint32)UNIXTIME, itr2->second.SpellId, itr2->second.ItemId );
            }
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
        m_cooldownMap[type].insert( make_pair( misc, cd ) );

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

    if((m_special_state & UNIT_STATE_CONFUSE) || m_uint32Values[UNIT_FIELD_CHARMEDBY])
        return;

    // this means the client is probably lagging. don't update the timestamp, don't do anything until we start to receive
    if( m_position == m_lastHeartbeatPosition && m_isMoving ) // packets again (give the poor laggers a chance to catch up)
        return;

    // simplified; just take the fastest speed. less chance of fuckups too
    float speed = (( m_FlyingAura || FlyCheat ) ? (m_runSpeed > m_flySpeed ? m_runSpeed : m_flySpeed) : m_runSpeed);
    if( m_swimSpeed > speed )
        speed = m_swimSpeed;

    if(!m_heartbeatDisable && !m_uint32Values[UNIT_FIELD_CHARM] && !m_speedChangeInProgress )
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
                    m_bg->RemovePlayer(TO_PLAYER(this), false);

                sEventMgr.AddEvent(TO_PLAYER(this), &Player::_Disconnect, EVENT_PLAYER_KICK, 10000, 1, 0 );
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
    sEventMgr.AddEvent(TO_PLAYER(this), &Player::ResetSpeedHack, EVENT_PLAYER_RESET_HEARTBEAT, ms, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );
}

/************************************************************************/
/* SOCIAL                                                               */
/************************************************************************/

void Player::Social_AddFriend(const char * name, const char * note)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    map<uint32, char*>::iterator itr;
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
        data << uint8(FRIEND_ALREADY) << uint64(info->guid);
        m_session->SendPacket(&data);
        m_socialLock.Release();
        return;
    }

    if( info->m_loggedInPlayer != NULL )
    {
        data << uint8(FRIEND_ADDED_ONLINE);
        data << uint64(info->guid);
        if( note != NULL )
            data << note;
        else
            data << uint8(0);

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
        data << uint64(info->guid);
    }

    if( note != NULL )
        m_friends.insert( make_pair(info->guid, strdup(note)) );
    else
        m_friends.insert( make_pair(info->guid, (char*)NULL) );

    m_socialLock.Release();
    m_session->SendPacket(&data);

    // dump into the db
    CharacterDatabase.Execute("INSERT INTO social_friends VALUES(%u, %u, \'%s\')",
        GetLowGUID(), info->guid, note ? CharacterDatabase.EscapeString(string(note)).c_str() : "");
}

void Player::Social_RemoveFriend(uint32 guid)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    map<uint32, char*>::iterator itr;

    // are we ourselves?
    if( guid == GetLowGUID() )
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

    Player* pl = objmgr.GetPlayer( (uint32)guid );
    if( pl != NULL )
    {
        pl->m_socialLock.Acquire();
        pl->m_hasFriendList.erase( GetLowGUID() );
        pl->m_socialLock.Release();
    }

    m_session->SendPacket(&data);

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u AND friend_guid = %u",
        GetLowGUID(), (uint32)guid);
}

void Player::Social_SetNote(uint32 guid, const char * note)
{
    map<uint32,char*>::iterator itr;

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
        note ? CharacterDatabase.EscapeString(string(note)).c_str() : "", GetLowGUID(), guid);
}

void Player::Social_AddIgnore(const char * name)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    set<uint32>::iterator itr;
    PlayerInfo * info;

    // lookup the player
    info = objmgr.GetPlayerInfoByName(name);
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

void Player::Social_RemoveIgnore(uint32 guid)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    set<uint32>::iterator itr;

    // are we ourselves?
    if( guid == GetLowGUID() )
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
    CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u AND ignore_guid = %u",
        GetLowGUID(), (uint32)guid);
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

bool Player::Social_IsIgnoring(uint32 guid)
{
    bool res;
    m_socialLock.Acquire();
    if( m_ignores.find( guid ) == m_ignores.end() )
        res = false;
    else
        res = true;

    m_socialLock.Release();
    return res;
}

void Player::Social_TellOnlineStatus(bool online)
{
    if( m_hasFriendList.empty() )
        return;

    Player* pl;
    set<uint32>::iterator itr;
    WorldPacket data(SMSG_FRIEND_STATUS, 22);
    if(online)
    {
        data << uint8( FRIEND_ONLINE ) << GetGUID() << uint8(GetChatTag());
        data << GetAreaId() << getLevel() << uint32(getClass());
    }
    else
        data << uint8( FRIEND_OFFLINE ) << GetGUID() << uint8( 0 );

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
    set<uint32>::iterator itr;
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
    map<uint32,char*>::iterator itr;
    set<uint32>::iterator itr2;
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
        else
            data << uint8(0);

        // online/offline flag
        plr = objmgr.GetPlayerInfo( itr->first );
        if(plr->m_loggedInPlayer != NULL)
        {
            data << plr->m_loggedInPlayer->GetChatTag();
            data << plr->m_loggedInPlayer->GetZoneId();
            data << plr->m_loggedInPlayer->getLevel();
            data << uint32( plr->m_loggedInPlayer->getClass() );
        }
        else
            data << uint8( 0 );// << float((UNIXTIME - plr->lastOnline) / 86400.0);
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

void Player::VampiricSpell(uint32 dmg, Unit* pTarget, SpellEntry *spellinfo)
{
    float fdmg = float(dmg);
    uint32 bonus;
    Group * pGroup = GetGroup();
    SubGroup * pSubGroup = (pGroup != NULL) ? pGroup->GetSubGroup(GetSubGroup()) : NULL;
    GroupMembersSet::iterator itr;
    if( m_vampiricEmbrace > 0 && pTarget->HasAurasOfNameHashWithCaster(SPELL_HASH_VAMPIRIC_EMBRACE, TO_PLAYER(this)) )
    {
        int32 pct = 15;
        uint32 sgt[3] = {4,0,0};
        SM_PIValue(SM[SMT_MISC_EFFECT][1],&pct,sgt);
        bonus = float2int32(fdmg * pct/100.0f);
        if( bonus > 0 )
        {
            Heal(TO_PLAYER(this), 15286, bonus);

            // loop party
            if( pSubGroup != NULL )
            {
                for( itr = pSubGroup->GetGroupMembersBegin(); itr != pSubGroup->GetGroupMembersEnd(); itr++ )
                {
                    if( (*itr)->m_loggedInPlayer != NULL && (*itr) != m_playerInfo )
                        Heal( (*itr)->m_loggedInPlayer, 15286, bonus / 5 );
                }
            }
        }
    }

    if( pTarget->m_vampiricTouch > 0 && spellinfo->NameHash == SPELL_HASH_MIND_BLAST )
    {
        if( GetGroup() )
        {
            uint32 TargetCount = 0;
            GetGroup()->Lock();
            for(uint32 x = 0; x < GetGroup()->GetSubGroupCount(); ++x)
            {
                if( TargetCount == 10 )
                    break;

                for(GroupMembersSet::iterator itr = GetGroup()->GetSubGroup( x )->GetGroupMembersBegin(); itr != GetGroup()->GetSubGroup( x )->GetGroupMembersEnd(); itr++)
                {
                    if((*itr)->m_loggedInPlayer && TargetCount <= 10)
                    {
                        Player* p_target = (*itr)->m_loggedInPlayer;
                        if( p_target->GetPowerType() != POWER_TYPE_MANA )
                            continue;

                        SpellEntry* Replinishment = dbcSpell.LookupEntry( 57669 );
                        Spell* pSpell = NULLSPELL;
                        pSpell = (new Spell(TO_PLAYER(this), Replinishment, true, NULLAURA));
                        SpellCastTargets tgt;
                        tgt.m_unitTarget = p_target->GetGUID();
                        pSpell->prepare(&tgt);
                        TargetCount++;
                    }
                }
            }
            GetGroup()->Unlock();
        }
        else
        {
            SpellEntry* Replinishment = dbcSpell.LookupEntry( 57669 );
            Spell* pSpell = NULLSPELL;
            pSpell = (new Spell(TO_PLAYER(this), Replinishment, true, NULLAURA));
            SpellCastTargets tgt;
            tgt.m_unitTarget = GetGUID();
            pSpell->prepare(&tgt);
        }
    }
}

void Player::GenerateLoot(Corpse* pCorpse)
{
    // default gold
    pCorpse->ClearLoot();
    pCorpse->m_loot.gold = 500;

    if( m_bg != NULL )
        m_bg->HookGenerateLoot(TO_PLAYER(this), pCorpse);
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
    if( isDead() )
        ResurrectPlayer();

    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    SetUInt32Value(UNIT_FIELD_POWER1, GetUInt32Value(UNIT_FIELD_MAXPOWER1));
    SetUInt32Value(UNIT_FIELD_POWER4, GetUInt32Value(UNIT_FIELD_MAXPOWER4));
    sEventMgr.RemoveEvents(this, EVENT_PLAYER_FULL_HPMP);
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
    set<uint32>::iterator itr = m_finishedQuests.begin();
    for(; itr != m_finishedQuests.end(); itr++)
    {
        Quest * pQuest = sQuestMgr.GetQuestPointer( *itr );
        if(!pQuest || !pQuest->qst_zone_id)
            continue;

        GetAchievementInterface()->HandleAchievementCriteriaCompleteQuestsInZone( pQuest->qst_zone_id );
    }
}

// Update glyphs after level change
void Player::InitGlyphsForLevel()
{
    // Enable number of glyphs depending on level
    uint32 level = getLevel();
    uint32 glyph_mask = 0;
    if (level >= 25)
        glyph_mask |= 0x01 | 0x02 | 0x40;
    if (level >= 50)
        glyph_mask |= 0x04 | 0x08 | 0x80;
    if (level >= 75)
        glyph_mask |= 0x10 | 0x20 | 0x100;
    SetUInt32Value(PLAYER_GLYPHS_ENABLED, glyph_mask);
}

static const uint32 GlyphSlotDataDump[GLYPHS_COUNT] = {21, 22, 23, 24, 25, 26, 41, 42, 43};

void Player::InitGlyphSlots()
{
    for(uint32 i = 0; i < GLYPHS_COUNT; i++)
        SetUInt32Value(PLAYER_FIELD_GLYPH_SLOTS_1 + i, GlyphSlotDataDump[i]);
}

void Player::UnapplyGlyph(uint32 slot)
{
    if(slot >= GLYPHS_COUNT)
        return; // Glyph doesn't exist
    // Get info
    uint32 glyphId = GetUInt32Value(PLAYER_FIELD_GLYPHS_1 + slot);
    if(glyphId == 0)
        return;
    GlyphProperties *glyph = dbcGlyphProperties.LookupEntry(glyphId);
    if(!glyph)
        return;
    SetUInt32Value(PLAYER_FIELD_GLYPHS_1 + slot, 0);
    m_AuraInterface.RemoveAllAuras(glyph->SpellId);
}

static const uint32 glyphType[9] = {0, 1, 1, 0, 1, 0, 2, 2, 2};

uint8 Player::SetGlyph(uint32 slot, uint32 glyphId)
{
    if(slot >= GLYPHS_COUNT)
        return SPELL_FAILED_INVALID_GLYPH;

    // Get info
    GlyphProperties *glyph = dbcGlyphProperties.LookupEntry(glyphId);
    if(glyph == NULL)
        return SPELL_FAILED_INVALID_GLYPH;

    for(uint32 x = 0; x < GLYPHS_COUNT; ++x)
    {
        if(m_specs[m_talentActiveSpec].glyphs[x] == glyphId && slot != x)
            return SPELL_FAILED_UNIQUE_GLYPH;
    }

    if( glyphType[slot] != glyph->TypeFlags || // Glyph type doesn't match
            (GetUInt32Value(PLAYER_GLYPHS_ENABLED) & (1 << slot)) == 0) // slot is not enabled
        return SPELL_FAILED_INVALID_GLYPH;

    UnapplyGlyph(slot);
    SetUInt32Value(PLAYER_FIELD_GLYPHS_1 + slot, glyphId);
    m_specs[m_talentActiveSpec].glyphs[slot] = glyphId;
    CastSpell(TO_PLAYER(this), glyph->SpellId, true);   // Apply the glyph effect
    return 0;
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
    sEventMgr.RemoveEvents(TO_PLAYER(this), EVENT_PLAYER_RUNE_REGEN + index);
    sEventMgr.AddEvent(TO_PLAYER(this), &Player::ConvertRune, (uint8)index, (forceDeathRune ? uint8(RUNE_TYPE_DEATH) : baseRunes[index]), EVENT_PLAYER_RUNE_REGEN + index, 10000, 0, 0);
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
        if( (GetUInt32Value(PLAYER_QUEST_LOG_1_1 + i * 5)) == questid )
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

void Player::AddArenaPoints( uint32 arenapoints )
{
    m_arenaPoints += arenapoints;
    HonorHandler::RecalculateHonorFields(this);
}

void Player::_LoadAreaPhaseInfo(QueryResult *result)
{
    if(result)
    {
        AreaPhaseData* APD = NULL;
        do
        {
            Field *fields = result->Fetch();
            APD = new AreaPhaseData();
            uint32 areaid = fields[1].GetUInt32();
            APD->phase = fields[2].GetInt32();
            areaphases[areaid] = APD;
        }while(result->NextRow());
    }
}

void Player::_SaveAreaPhaseInfo(QueryBuffer* buff)
{
    if(areaphases.size())
    {
        m_lock.Acquire();
        if(buff == NULL)
            CharacterDatabase.Execute("DELETE FROM playerphaseinfo WHERE guid = '%u'", GetGUID());
        else
            buff->AddQuery("DELETE FROM playerphaseinfo WHERE guid = '%u'", GetGUID());

        map<uint32, AreaPhaseData*>::iterator itr;
        for(itr = areaphases.begin(); itr != areaphases.end(); itr++)
        {
            if(itr->second != NULL)
                if(buff == NULL)
                    CharacterDatabase.Execute("INSERT INTO playerphaseinfo VALUES('%u', '%u', '%i')", GetGUID(), itr->first, itr->second->phase);
                else
                    buff->AddQuery("INSERT INTO playerphaseinfo VALUES('%u', '%u', '%i')", GetGUID(), itr->first, itr->second->phase);
        }
        m_lock.Release();
    }
}

void Player::SetPhaseMask(int32 phase, bool save)
{
    Object::SetPhaseMask(phase);

    if(IS_INSTANCE(GetMapId()))
        return; // Don't save instance phases.

    if(areaphases[GetAreaId()] != NULL)
        areaphases[GetAreaId()]->phase = phase;
    else if(save && phase != 1)
    {
        AreaPhaseData* APD = new AreaPhaseData();
        APD->phase = phase;
        areaphases[GetAreaId()] = APD;
    }

    if(phase == 1) // Phase is reset, no point in saving it.
    {
        if(areaphases[GetAreaId()])
        {
            delete areaphases[GetAreaId()];
            areaphases.erase(GetAreaId());
        }
    }
}

void Player::EnablePhase(int32 phaseMode, bool save)
{
    Object::EnablePhase(phaseMode);

    if(IS_INSTANCE(GetMapId()))
        return; // Don't save instance phases.

    int32 phase = GetPhaseMask();
    if(areaphases[GetAreaId()] != NULL)
        areaphases[GetAreaId()]->phase = phase;
    else if(save && phase != 1)
    {
        AreaPhaseData* APD = new AreaPhaseData();
        APD->phase = phase;
        areaphases[GetAreaId()] = APD;
    }

    if(phase == 1) // Phase is reset, no point in saving it.
    {
        if(areaphases[GetAreaId()])
        {
            delete areaphases[GetAreaId()];
            areaphases.erase(GetAreaId());
        }
    }
}

void Player::DisablePhase(int32 phaseMode, bool save)
{
    Object::DisablePhase(phaseMode);

    if(IS_INSTANCE(GetMapId()))
        return; // Don't save instance phases.

    int32 phase = GetPhaseMask();
    if(areaphases[GetAreaId()] != NULL)
        areaphases[GetAreaId()]->phase = phase;
    else if(save && phase != 1)
    {
        AreaPhaseData* APD = new AreaPhaseData();
        APD->phase = GetPhaseMask();
        areaphases[GetAreaId()] = APD;
    }

    if(phase == 1) // Phase is reset, no point in saving it.
    {
        if(areaphases[GetAreaId()])
        {
            delete areaphases[GetAreaId()];
            areaphases.erase(GetAreaId());
        }
    }
}

int32 Player::GetPhaseForArea(uint32 areaid)
{
    if(areaphases[areaid] != NULL)
        return areaphases[areaid]->phase;

    return 1;
}

void Player::SetPhaseForArea(uint32 areaid, int32 phase)
{
    if(areaphases[areaid] == NULL)
        areaphases[areaid] = new AreaPhaseData();
    areaphases[areaid]->phase = phase;

    if(phase == 1) // Phase is reset, no point in saving it.
    {
        if(areaphases[areaid])
        {
            delete areaphases[areaid];
            areaphases.erase(areaid);
        }
    }
}

uint32 Player::GenerateShapeshiftModelId(uint32 form)
{
    switch(form)
    {
        case FORM_CAT:
            // Based on Hair color
            if (getRace() == RACE_NIGHTELF)
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
            // Based on Skin color
            else if (getRace() == RACE_TAUREN)
            {
                uint8 skinColor = GetByte(PLAYER_BYTES, 0);
                // Male
                if (getGender() == 0)
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
                // Female
                else switch (skinColor)
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
            else if (GetTeam() == 0)
                return 892;
            else
                return 8571;
            break;
        case FORM_DIREBEAR:
        case FORM_BEAR:
            // Based on Hair color
            if (getRace() == RACE_NIGHTELF)
            {
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
            // Based on Skin color
            else if (getRace() == RACE_TAUREN)
            {
                uint8 skinColor = GetByte(PLAYER_BYTES, 0);
                // Male
                if (getGender() == 0)
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
                // Female
                else switch (skinColor)
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
            else if (GetTeam() == 0)
                return 2281;
            else
                return 2289;
        case FORM_TRAVEL:
            return 632;
        case FORM_AQUA:
            if (GetTeam() == 0)
                return 2428;
            else
                return 2428;
        case FORM_GHOUL:
            return 24994;
        case FORM_CREATUREBEAR:
            return 902;
        case FORM_GHOSTWOLF:
            return 4613;
        case FORM_FLIGHT:
            if (GetTeam() == 0)
                return 20857;
            else
                return 20872;
        case FORM_MOONKIN:
            if (GetTeam() == 0)
                return 15374;
            else
                return 15375;
        case FORM_SWIFT:
            if (GetTeam() == 0)
                return 21243;
            else
                return 21244;
        case FORM_DEMON:
            return 25277;
        case FORM_MASTER_ANGLER:
            return 15234;
        case FORM_TREE:
            return 864;
        case FORM_SPIRITOFREDEMPTION:
            return 16031;
        default:
            break;
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
            Item* item = objmgr.CreateItem( qst->receive_items[i], this);
            if(item)
            {
                item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, qst->receive_itemcount[i]);
                if(!GetItemInterface()->AddItemToFreeSlot(item))
                {
                    item->DeleteMe();
                    item = NULLITEM;
                }
                else
                    GetSession()->SendItemPushResult(item, false, true, false, true, GetItemInterface()->LastSearchItemBagSlot(), GetItemInterface()->LastSearchItemSlot(),1);
            }
        }
    }

    if(qst->srcitem && qst->srcitem != qst->receive_items[0])
    {
        Item* item = objmgr.CreateItem( qst->srcitem, this );
        if(item)
        {
            item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, qst->srcitemcount ? qst->srcitemcount : 1);
            if(!GetItemInterface()->AddItemToFreeSlot(item))
            {
                item->DeleteMe();
                item = NULLITEM;
            }
        }
    }
    CALL_QUESTSCRIPT_EVENT(Id, OnQuestStart)(this, qle);

    sQuestMgr.OnQuestAccepted(this,qst,NULL);

    if(qst->qst_start_phase != 0 )
        EnablePhase(qst->qst_start_phase, true);

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
