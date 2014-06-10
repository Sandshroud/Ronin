/***
 * Demonstrike Core
 */

#pragma once

class ChatHandler;
class WorldSession;
class Player;
class Unit;

enum ChatMsg
{
    CHAT_MSG_ADDON                                         = 0xFFFFFFFF,
    CHAT_MSG_SYSTEM                                        = 0x00,
    CHAT_MSG_SAY                                           = 0x01,
    CHAT_MSG_PARTY                                         = 0x02,
    CHAT_MSG_RAID                                          = 0x03,
    CHAT_MSG_GUILD                                         = 0x04,
    CHAT_MSG_OFFICER                                       = 0x05,
    CHAT_MSG_YELL                                          = 0x06,
    CHAT_MSG_WHISPER                                       = 0x07,
    CHAT_MSG_WHISPER_FOREIGN                               = 0x08,
    CHAT_MSG_WHISPER_INFORM                                = 0x09,
    CHAT_MSG_EMOTE                                         = 0x0A,
    CHAT_MSG_TEXT_EMOTE                                    = 0x0B,
    CHAT_MSG_MONSTER_SAY                                   = 0x0C,
    CHAT_MSG_MONSTER_PARTY                                 = 0x0D,
    CHAT_MSG_MONSTER_YELL                                  = 0x0E,
    CHAT_MSG_MONSTER_WHISPER                               = 0x0F,
    CHAT_MSG_MONSTER_EMOTE                                 = 0x10,
    CHAT_MSG_CHANNEL                                       = 0x11,
    CHAT_MSG_CHANNEL_JOIN                                  = 0x12,
    CHAT_MSG_CHANNEL_LEAVE                                 = 0x13,
    CHAT_MSG_CHANNEL_LIST                                  = 0x14,
    CHAT_MSG_CHANNEL_NOTICE                                = 0x15,
    CHAT_MSG_CHANNEL_NOTICE_USER                           = 0x16,
    CHAT_MSG_AFK                                           = 0x17,
    CHAT_MSG_DND                                           = 0x18,
    CHAT_MSG_IGNORED                                       = 0x19,
    CHAT_MSG_SKILL                                         = 0x1A,
    CHAT_MSG_LOOT                                          = 0x1B,
    CHAT_MSG_MONEY                                         = 0x1C,
    CHAT_MSG_OPENING                                       = 0x1D,
    CHAT_MSG_TRADESKILLS                                   = 0x1E,
    CHAT_MSG_PET_INFO                                      = 0x1F,
    CHAT_MSG_COMBAT_MISC_INFO                              = 0x20,
    CHAT_MSG_COMBAT_XP_GAIN                                = 0x21,
    CHAT_MSG_COMBAT_HONOR_GAIN                             = 0x22,
    CHAT_MSG_COMBAT_FACTION_CHANGE                         = 0x23,
    CHAT_MSG_BG_SYSTEM_NEUTRAL                             = 0x24,
    CHAT_MSG_BG_SYSTEM_ALLIANCE                            = 0x25,
    CHAT_MSG_BG_SYSTEM_HORDE                               = 0x26,
    CHAT_MSG_RAID_LEADER                                   = 0x27,
    CHAT_MSG_RAID_WARNING                                  = 0x28,
    CHAT_MSG_RAID_BOSS_EMOTE                               = 0x29,
    CHAT_MSG_RAID_BOSS_WHISPER                             = 0x2A,
    CHAT_MSG_FILTERED                                      = 0x2B,
    CHAT_MSG_BATTLEGROUND                                  = 0x2C,
    CHAT_MSG_BATTLEGROUND_LEADER                           = 0x2D,
    CHAT_MSG_RESTRICTED                                    = 0x2E,
    CHAT_MSG_BATTLENET                                     = 0x2F,
    CHAT_MSG_ACHIEVEMENT                                   = 0x30,
    CHAT_MSG_GUILD_ACHIEVEMENT                             = 0x31,
    CHAT_MSG_ARENA_POINTS                                  = 0x32,
    CHAT_MSG_PARTY_LEADER                                  = 0x33
};

enum Languages
{
    LANG_ADDON                                  = -1,
    LANG_UNIVERSAL                              = 0x00,
    LANG_ORCISH                                 = 0x01,
    LANG_DARNASSIAN                             = 0x02,
    LANG_TAURAHE                                = 0x03,
    LANG_DWARVISH                               = 0x06,
    LANG_COMMON                                 = 0x07,
    LANG_DEMONIC                                = 0x08,
    LANG_TITAN                                  = 0x09,
    LANG_THALASSIAN                             = 0x0A,
    LANG_DRACONIC                               = 0x0B,
    LANG_KALIMAG                                = 0x0C,
    LANG_GNOMISH                                = 0x0D,
    LANG_TROLL                                  = 0x0E,
    LANG_GUTTERSPEAK                            = 0x21,
    LANG_DRAENEI                                = 0x23,
    LANG_ZOMBIE                                 = 0x24,
    LANG_GNOMISHBINARY                          = 0x25,
    LANG_GOBLINBINARY                           = 0x26,
    NUM_LANGUAGES                               = 0x27
};

/*#define MSG_COLOR_YELLOW      "|r"
#define MSG_COLOR_RED           "|cffff2020"
#define MSG_COLOR_GREEN         "|c1f40af20"
#define MSG_COLOR_LIGHTRED      "|cffff6060"*/

#define MSG_COLOR_LIGHTRED      "|cffff6060"
#define MSG_COLOR_LIGHTBLUE     "|cff00ccff"
#define MSG_COLOR_BLUE          "|cff0000ff"
#define MSG_COLOR_GREEN         "|cff00ff00"
#define MSG_COLOR_RED           "|cffff0000"
#define MSG_COLOR_GOLD          "|cffffcc00"
#define MSG_COLOR_GREY          "|cff888888"
#define MSG_COLOR_WHITE         "|cffffffff"
#define MSG_COLOR_SUBWHITE      "|cffbbbbbb"
#define MSG_COLOR_MAGENTA       "|cffff00ff"
#define MSG_COLOR_YELLOW        "|cffffff00"
#define MSG_COLOR_CYAN          "|cff00ffff"
#define MSG_COLOR_TORQUISEBLUE  "|cff00C78C"
#define MSG_COLOR_GREENYELLOW   "|cffADFF2F"
#define MSG_COLOR_PURPLE        "|cffDA70D6"
#define MSG_COLOR_ORANGEY       "|cffFF4500"
#define MSG_COLOR_CHOCOLATE     "|cffCD661D"
#define MSG_COLOR_IVORY         "|cff8B8B83"
#define MSG_COLOR_LIGHTYELLOW   "|cffFFFFE0"
#define MSG_COLOR_SEXGREEN      "|cff71C671"
#define MSG_COLOR_SEXTEAL       "|cff388E8E"
#define MSG_COLOR_SEXPINK       "|cffC67171"
#define MSG_COLOR_SEXBLUE       "|cff00E5EE"
#define MSG_COLOR_SEXHOTPINK    "|cffFF6EB4"

#define CHECKSESSION if(m_session == NULL) return NULL; \
    if(m_session->GetPlayer() == NULL) return NULL;


class ChatCommand
{
public:
    const char *        Name;
    char                CommandGroup;
    char                NumericCGroup;
    bool (ChatHandler::*Handler)(const char* args, WorldSession *m_session) ;
    std::string         Help;
    ChatCommand *       ChildCommands;
    uint32              NormalValueField;
    uint32              MaxValueField;
    uint16              ValueType;  // 0 = nothing, 1 = uint, 2 = float
};

class SERVER_DECL CommandTableStorage : public Singleton<CommandTableStorage>
{
    ChatCommand * _modifyCommandTable;
    ChatCommand * _debugCommandTable;
    ChatCommand * _waypointCommandTable;
    ChatCommand * _GMTicketCommandTable;
    ChatCommand * _GameObjectCommandTable;
    ChatCommand * _BattlegroundCommandTable;
    ChatCommand * _NPCCommandTable;
    ChatCommand * _gamemasterCommandTable;
    ChatCommand * _trackerCommandTable;
    ChatCommand * _warnCommandTable;
    ChatCommand * _administratorCommandTable;
    ChatCommand * _accountCommandTable;
    ChatCommand * _CheatCommandTable;
    ChatCommand * _honorCommandTable;
    ChatCommand * _questCommandTable;
    ChatCommand * _lookupCommandTable;
    ChatCommand * _petCommandTable;
    ChatCommand * _recallCommandTable;
    ChatCommand * _commandTable;
    ChatCommand * _GuildCommandTable;
    ChatCommand * _TitleCommandTable;
    ChatCommand * _wintergraspCommandTable;
    ChatCommand * _FactionCommandTable;

    // DB commands
    ChatCommand * _DBItemCommandTable;
    ChatCommand * _DatabaseCommandTable;
    // End Database Commands

    ChatCommand * GetSubCommandTable(const char * name);
public:
    void Init();
    void Dealloc();
    void Load();
    void Override(const char * command, const char * level);
    HEARTHSTONE_INLINE ChatCommand * Get() { return _commandTable; }
};

class SERVER_DECL ChatHandler : public Singleton<ChatHandler>
{
    friend class CommandTableStorage;
public:
    ChatHandler();
    ~ChatHandler();

    WorldPacket * FillMessageData( uint32 type, int32 language,  const char* message,uint64 guid, uint8 flag = 0) const;
    WorldPacket * FillSystemMessageData( const char* message ) const;

    int ParseCommands(const char* text, WorldSession *m_session);

    void SystemMessage(WorldSession *m_session, const char *message, ...);
    void ColorSystemMessage(WorldSession *m_session, const char *colorcode, const char *message, ...);
    void RedSystemMessage(WorldSession *m_session, const char *message, ...);
    void GreenSystemMessage(WorldSession *m_session, const char *message, ...);
    void WhiteSystemMessage(WorldSession *m_session, const char *message, ...);
    void BlueSystemMessage(WorldSession *m_session, const char *message, ...);
    void RedSystemMessageToPlr(Player* plr, const char *message, ...);
    void GreenSystemMessageToPlr(Player* plr, const char *message, ...);
    void BlueSystemMessageToPlr(Player* plr, const char *message, ...);
    void SystemMessageToPlr(Player* plr, const char *message, ...);

    static bool hasStringAbbr(const char* s1, const char* s2);

protected:
    void SendMultilineMessage(WorldSession *m_session, const char *str);

    bool ExecuteCommandInTable(ChatCommand *table, const char* text, WorldSession *m_session);
    bool ShowHelpForCommand(WorldSession *m_session, ChatCommand *table, const char* cmd);

    ChatCommand* getCommandTable();

    // Level 0 commands
    bool HandleHelpCommand(const char* args, WorldSession *m_session);
    bool HandleCommandsCommand(const char* args, WorldSession *m_session);
    bool HandleNYICommand(const char* args, WorldSession *m_session);
    bool HandleAcctCommand(const char* args, WorldSession *m_session);
    bool HandleStartCommand(const char* args, WorldSession *m_session);
    bool HandleInfoCommand(const char* args, WorldSession *m_session);
    bool HandleDismountCommand(const char* args, WorldSession *m_session);
    bool HandleFullDismountCommand(const char * args, WorldSession *m_session);
    bool HandleSaveCommand(const char* args, WorldSession *m_session);
    bool HandleGMListCommand(const char* args, WorldSession *m_session);
    bool HandleGmLogCommentCommand( const char *args , WorldSession *m_session);
    bool HandleRatingsCommand( const char *args , WorldSession *m_session );

    // Level 1 commands
    bool CmdSetValueField(WorldSession *m_session, uint32 field, uint32 fieldmax, const char *fieldname, const char* args);
    bool CmdSetFloatField(WorldSession *m_session, uint32 field, uint32 fieldmax, const char *fieldname, const char* args);
    bool HandleSummonCommand(const char* args, WorldSession *m_session);
    bool HandleAppearCommand(const char* args, WorldSession *m_session);
    bool HandleAnnounceCommand(const char* args, WorldSession *m_session);
    bool HandleAdminAnnounceCommand(const char* args, WorldSession *m_session);
    bool HandleGMAnnounceCommand(const char* args, WorldSession *m_session);
    bool HandleWAnnounceCommand(const char* args, WorldSession *m_session);
    bool HandleGMOnCommand(const char* args, WorldSession *m_session);
    bool HandleGMOffCommand(const char* args, WorldSession *m_session);
    bool HandleGPSCommand(const char* args, WorldSession *m_session);
    bool HandleKickCommand(const char* args, WorldSession *m_session);
    bool HandleTaxiCheatCommand(const char* args, WorldSession *m_session);
    bool HandleModifySpeedCommand(const char* args, WorldSession *m_session);
    bool HandleModifyTPsCommand(const char* args, WorldSession *m_session);
    bool HandleToggleDevCommand(const char* args, WorldSession *m_session);

    // Debug Commands
    bool HandleDebugSetPhase(const char* args, WorldSession *m_session);
    bool HandleDebugGoDamage(const char* args, WorldSession *m_session);
    bool HandleDebugGoRepair(const char* args, WorldSession *m_session);
    bool HandleDebugInFrontCommand(const char* args, WorldSession *m_session);
    bool HandleShowReactionCommand(const char* args, WorldSession *m_session);
    bool HandleMoveInfoCommand(const char* args, WorldSession *m_session);
    bool HandleDistanceCommand(const char* args, WorldSession *m_session);
    bool HandleFaceCommand(const char* args, WorldSession *m_session);
    bool HandleSetBytesCommand(const char* args, WorldSession *m_session);
    bool HandleGetBytesCommand(const char* args, WorldSession *m_session);
    bool HandleDebugLandWalk(const char* args, WorldSession *m_session);
    bool HandleDebugUnroot(const char* args, WorldSession *m_session);
    bool HandleDebugRoot(const char* args, WorldSession *m_session);
    bool HandleDebugWaterWalk(const char* args, WorldSession *m_session);
    bool HandleAggroRangeCommand(const char* args, WorldSession *m_session);
    bool HandleKnockBackCommand(const char* args, WorldSession *m_session);
    bool HandleFadeCommand(const char* args, WorldSession *m_session);
    bool HandleThreatModCommand(const char* args, WorldSession *m_session);
    bool HandleCalcThreatCommand(const char* args, WorldSession *m_session);
    bool HandleThreatListCommand(const char* args, WorldSession *m_session);
    bool HandleNpcSpawnLinkCommand(const char* args, WorldSession *m_session);
    bool HandleRangeCheckCommand( const char * args , WorldSession * m_session );

    // WayPoint Commands
    bool HandleWPAddCommand(const char* args, WorldSession *m_session);
    bool HandleWPShowCommand(const char* args, WorldSession *m_session);
    bool HandleWPHideCommand(const char* args, WorldSession *m_session);
    bool HandleWPDeleteCommand(const char* args, WorldSession *m_session);
    bool HandleWPFlagsCommand(const char* args, WorldSession *m_session);
    bool HandleWPMoveHereCommand(const char* args, WorldSession *m_session);
    bool HandleWPWaitCommand(const char* args, WorldSession *m_session);
    bool HandleWPEmoteCommand(const char* args, WorldSession *m_session);
    bool HandleWPStandStateCommand(const char* args, WorldSession *m_session);
    bool HandleWPSpellToCastCommand(const char* args, WorldSession *m_session);
    bool HandleWPSkinCommand(const char* args, WorldSession *m_session);
    bool HandleWPChangeNoCommand(const char* args, WorldSession *m_session);
    bool HandleWPInfoCommand(const char* args, WorldSession *m_session);
    bool HandleWPMoveTypeCommand(const char* args, WorldSession *m_session);
    bool HandleSaveWaypoints(const char* args, WorldSession * m_session);
    bool HandleGenerateWaypoints(const char* args, WorldSession * m_session);
    bool HandleDeleteWaypoints(const char* args, WorldSession * m_session);
    bool HandleWaypointGettextCommand(const char* args, WorldSession *m_session);
    bool HandleWaypointBackwardTextCommand(const char* args, WorldSession *m_session);
    bool HandleWaypointForwardTextCommand(const char* args, WorldSession *m_session);
    bool HandleWaypointSetOrientationCommand(const char* args, WorldSession *m_session);

    // Guild commands
    bool CreateGuildCommand(const char* args, WorldSession *m_session);
    bool HandleGuildRemovePlayerCommand(const char* args, WorldSession *m_session);
    bool HandleGuildDisbandCommand(const char* args, WorldSession *m_session);

    // Level 2 commands
    bool HandleNameCommand(const char* args, WorldSession *m_session);
    bool HandleSubNameCommand(const char* args, WorldSession *m_session);
    bool HandleDeleteCommand(const char* args, WorldSession *m_session);
    bool HandleDeMorphCommand(const char* args, WorldSession *m_session);
    bool HandleItemCommand(const char* args, WorldSession *m_session);
    bool HandleItemRemoveCommand(const char* args, WorldSession *m_session);
    bool HandleNPCFlagCommand(const char* args, WorldSession *m_session);
    bool HandleSaveAllCommand(const char* args, WorldSession *m_session);
    bool HandleRegenerateGossipCommand(const char* args, WorldSession *m_session);
    bool HandleStartBGCommand(const char* args, WorldSession *m_session);
    bool HandlePauseBGCommand(const char* args, WorldSession *m_session);
    bool HandleResetScoreCommand(const char* args, WorldSession *m_session);
    bool HandleBGInfoCommnad(const char *args, WorldSession *m_session);
    bool HandleInvincibleCommand(const char *args, WorldSession *m_session);
    bool HandleInvisibleCommand(const char *args, WorldSession *m_session);
    bool HandleKillCommand(const char *args, WorldSession *m_session);
    bool HandleKillByPlrCommand( const char *args , WorldSession *m_session );
    bool HandleCreatureSpawnCommand(const char *args, WorldSession *m_session);
    bool HandleCreatureRespawnCommand(const char *args, WorldSession *m_session);
    bool HandleGOSelect(const char *args, WorldSession *m_session);
    bool HandleGODelete(const char *args, WorldSession *m_session);
    bool HandleGOSpawn(const char *args, WorldSession *m_session);
    bool HandleGOInfo(const char *args, WorldSession *m_session);
    bool HandleGOEnable(const char *args, WorldSession *m_session);
    bool HandleGOActivate(const char* args, WorldSession *m_session);
    bool HandleGORotate(const char * args, WorldSession * m_session);
    bool HandleGOMove(const char * args, WorldSession * m_session);

    // Level 3 commands
    bool HandleMassSummonCommand(const char* args, WorldSession *m_session);
    bool HandleWorldPortCommand(const char* args, WorldSession *m_session);
    bool HandleMoveCommand(const char* args, WorldSession *m_session);
    bool HandleLearnCommand(const char* args, WorldSession *m_session);
    bool HandleReviveCommand(const char* args, WorldSession *m_session);
    bool HandleAddGraveCommand(const char* args, WorldSession *m_session);
    bool HandleAddSHCommand(const char* args, WorldSession *m_session);
    bool HandleExploreCheatCommand(const char* args, WorldSession *m_session);
    bool HandleAddSkillCommand(const char* args, WorldSession *m_session);
    bool HandleAddInvItemCommand(const char* args, WorldSession *m_session);
    bool HandleResetReputationCommand(const char* args, WorldSession *m_session);
    bool HandleLearnSkillCommand(const char* args, WorldSession *m_session);
    bool HandleModifySkillCommand(const char* args, WorldSession *m_session);
    bool HandleRemoveSkillCommand(const char* args, WorldSession *m_session);
    bool HandleNpcInfoCommand(const char* args, WorldSession *m_session);
    bool HandleEmoteCommand(const char* args, WorldSession *m_session);
    bool HandleStandStateCommand(const char* args, WorldSession *m_session);
    bool HandleIncreaseWeaponSkill(const char* args, WorldSession *m_session);
    bool HandleCastSpellCommand(const char* args, WorldSession *m_session);
    bool HandleCastSpellNECommand(const char* args, WorldSession *m_session);
    bool HandleGenderChanger(const char* args, WorldSession *m_session);
    bool HandleModifyGoldCommand(const char* args, WorldSession *m_session);
    bool HandleMonsterSayCommand(const char* args, WorldSession *m_session);
    bool HandleMonsterYellCommand(const char* args, WorldSession* m_session);
    bool HandleMonsterCastCommand(const char * args, WorldSession * m_session);
    bool HandleNPCEquipCommand(const char * args, WorldSession * m_session);
    bool HandleNPCSetOnObjectCommand(const char * args, WorldSession * m_session);
    bool HandleNPCSaveCommand(const char * args, WorldSession * m_session);
    bool HandleNPCSetVendorMaskCommand(const char * args, WorldSession * m_session);
    bool HandleNpcComeCommand(const char* args, WorldSession* m_session);
    bool HandleNpcJumpCommand(const char* args, WorldSession* m_session);
    bool HandleClearCooldownsCommand(const char* args, WorldSession *m_session);
    bool HandleSetWorldStateCommand(const char* args, WorldSession *m_session);
    bool HandlePlaySoundCommand(const char* args, WorldSession *m_session);
    bool HandleSetBattlefieldStatusCommand(const char* args, WorldSession *m_session);
    bool HandleNpcReturnCommand(const char* args, WorldSession* m_session);
    bool HandleAccountBannedCommand(const char * args, WorldSession * m_session);
    bool HandleAccountLevelCommand(const char * args, WorldSession * m_session);
    bool HandleResetTalentsCommand(const char* args, WorldSession *m_session);
    bool HandleResetSpellsCommand(const char* args, WorldSession *m_session);
    bool HandleNpcFollowCommand(const char* args, WorldSession * m_session);
    bool HandleNullFollowCommand(const char* args, WorldSession * m_session);
    bool HandleResetSkillsCommand(const char* args, WorldSession * m_session);
    bool HandleGetSkillLevelCommand(const char* args, WorldSession * m_session);
    bool HandleGetSkillsInfoCommand(const char *args, WorldSession *m_session);
    bool HandlePlayerInfo(const char* args, WorldSession * m_session);

    // Tickets
#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
    void SendGMSurvey();
#endif
    bool HandleGMTicketListCommand(const char* args, WorldSession *m_session);
    bool HandleGMTicketGetByIdCommand(const char* args, WorldSession *m_session);
    bool HandleGMTicketRemoveByIdCommand(const char* args, WorldSession *m_session);
#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
    bool HandleGMTicketAssignToCommand(const char* args, WorldSession *m_session);
    bool HandleGMTicketReleaseCommand(const char* args, WorldSession *m_session);
    bool HandleGMTicketCommentCommand(const char* args, WorldSession *m_session);
    bool HandleGMTicketDeletePermanentCommand(const char* args, WorldSession *m_session);
#endif

    //Ban
    bool HandleBanCharacterCommand(const char* args, WorldSession *m_session);
    bool HandleUnBanCharacterCommand(const char* args, WorldSession *m_session);

    //BG
    bool HandleSetBGScoreCommand(const char* args, WorldSession *m_session);

    Player* getSelectedChar(WorldSession *m_session, bool showerror = true);
    Creature* getSelectedCreature(WorldSession *m_session, bool showerror = true);
    Unit* getSelectedUnit(WorldSession *m_session, bool showerror = true);
    bool HandleGOScale(const char* args, WorldSession *m_session);
    bool HandleReviveStringcommand(const char* args, WorldSession* m_session);
    bool HandleMountCommand(const char* args, WorldSession* m_session);
    bool HandleGetPosCommand(const char* args, WorldSession* m_session);
    bool HandleGetTransporterTime(const char* args, WorldSession* m_session);
    bool HandleGOAnimProgress(const char * args, WorldSession * m_session);
    bool HandleRemoveAurasCommand(const char *args, WorldSession *m_session);
    bool HandleParalyzeCommand(const char* args, WorldSession *m_session);
    bool HandleUnParalyzeCommand(const char* args, WorldSession *m_session);
    bool HandleSetMotdCommand(const char* args, WorldSession* m_session);
    bool HandleAddItemSetCommand(const char* args, WorldSession* m_session);
    bool HandleTriggerCommand(const char* args, WorldSession* m_session);
    bool HandleModifyValueCommand(const char* args, WorldSession* m_session);
    bool HandleModifyBitCommand(const char* args, WorldSession* m_session);
    bool HandleBattlegroundExitCommand(const char* args, WorldSession* m_session);
    bool HandleBattlegroundForcestartCommand(const char* args, WorldSession* m_session);
    bool HandleCooldownCheatCommand(const char* args, WorldSession* m_session);
    bool HandleCastTimeCheatCommand(const char* args, WorldSession* m_session);
    bool HandlePowerCheatCommand(const char* args, WorldSession* m_session);
    bool HandleShowCheatsCommand(const char* args, WorldSession* m_session);
    bool HandleFlySpeedCheatCommand(const char* args, WorldSession * m_session);
    bool HandleStackCheatCommand(const char* args, WorldSession * m_session);
    bool HandleTriggerpassCheatCommand(const char* args, WorldSession * m_session);
    bool HandleVendorPassCheatCommand(const char* args, WorldSession * m_session);
    bool HandleItemReqCheatCommand(const char* args, WorldSession * m_session);
    bool HandleFlyCommand(const char* args, WorldSession* m_session);
    bool HandleRemoveRessurectionSickessAuraCommand(const char *args, WorldSession *m_session);
    bool HandleDBReloadCommand(const char* args, WorldSession* m_session);
    bool HandleItemSetCommand(const char* args, WorldSession* m_session);
    bool HandleVendorClearCommand(const char* args, WorldSession* m_session);
    bool HandleItemSetRemoveCommand(const char* args, WorldSession* m_session);
    bool HandleTrainerAddLearnSpell(const char* args, WorldSession* m_session);
    bool HandleTrainerAddCastSpell(const char* args, WorldSession* m_session);

    // honor
    bool HandleAddHonorCommand(const char* args, WorldSession* m_session);
    bool HandleAddKillCommand(const char* args, WorldSession* m_session);
    bool HandleGlobalHonorDailyMaintenanceCommand(const char* args, WorldSession* m_session);
    bool HandleNextDayCommand(const char* args, WorldSession* m_session);
    bool HandlePVPCreditCommand(const char* args, WorldSession* m_session);

    bool HandleAddTitleCommand(const char* args, WorldSession* m_session);
    bool HandleRemoveTitleCommand(const char* args, WorldSession* m_session);
    bool HandleGetKnownTitlesCommand(const char* args, WorldSession* m_session);
    bool HandleSetChosenTitleCommand(const char* args, WorldSession* m_session);

    bool HandleUnlearnCommand(const char* args, WorldSession * m_session);
    bool HandleModifyLevelCommand(const char* args, WorldSession* m_session);

    // Pet
    bool HandleCreatePetCommand(const char* args, WorldSession* m_session);
    bool HandleAddPetSpellCommand(const char* args, WorldSession* m_session);
    bool HandleRemovePetSpellCommand(const char* args, WorldSession* m_session);
    bool HandleAddPetTalentPoints(const char* args, WorldSession* m_session);
    bool HandleResetPetTalents(const char* args, WorldSession* m_session);
    bool HandleRenamePetCommand(const char* args, WorldSession* m_session);

    // Shutdown
    bool HandleShutdownCommand(const char* args, WorldSession* m_session);
    bool HandleRestartCommand(const char* args, WorldSession* m_session);

    // Whispers
    bool HandleAllowWhispersCommand(const char* args, WorldSession* m_session);
    bool HandleBlockWhispersCommand(const char* args, WorldSession* m_session);

    // Skills
    bool HandleAdvanceAllSkillsCommand(const char* args, WorldSession* m_session);

    // Kill
    bool HandleKillBySessionCommand(const char* args, WorldSession* m_session);
    bool HandleKillByPlayerCommand(const char* args, WorldSession* m_session);

    // Castall
    bool HandleCastAllCommand(const char* args, WorldSession* m_session);

    // Recall
    bool HandleRecallListCommand(const char* args, WorldSession *m_session);
    bool HandleRecallGoCommand(const char* args, WorldSession *m_session);
    bool HandleRecallAddCommand(const char* args, WorldSession *m_session);
    bool HandleRecallDelCommand(const char* args, WorldSession *m_session);
    bool HandleModPeriodCommand(const char* args, WorldSession * m_session);
    bool HandleGlobalPlaySoundCommand(const char* args, WorldSession * m_session);
    bool HandleRecallPortPlayerCommand(const char* args, WorldSession * m_session);

    // Bans
    bool HandleIPBanCommand(const char * args, WorldSession * m_session);
    bool HandleIPUnBanCommand(const char * args, WorldSession * m_session);
    bool HandleAccountUnbanCommand(const char * args, WorldSession * m_session);

    // Item removal
    bool HandleRemoveItemCommand(const char * args, WorldSession * m_session);

    // Rename
    bool HandleRenameCommand(const char * args, WorldSession * m_session);
    bool HandleForceRenameCommand(const char * args, WorldSession * m_session);
    bool HandleRecustomizeCharCommand(const char * args, WorldSession * m_session);

    // Reputation
    bool HandleGetStandingCommand(const char * args, WorldSession * m_session);
    bool HandleSetStandingCommand(const char * args, WorldSession * m_session);
    bool HandleGetBaseStandingCommand(const char * args, WorldSession * m_session);

    // Lookups
    bool HandleLookupItemCommand(const char * args, WorldSession * m_session);
    bool HandleLookupItemSetCommand(const char * args, WorldSession * m_session);
    bool HandleLookupCreatureCommand(const char * args, WorldSession * m_session);
    bool HandleLookupObjectCommand(const char * args, WorldSession * m_session);
    bool HandleLookupSpellCommand(const char * args, WorldSession * m_session);
    bool HandleLookupSpellSpecificCommand(const char * args, WorldSession * m_session);
    bool HandleLookupSpellNameCommand(const char * args, WorldSession * m_session);
    bool HandleLookupTitleCommand(const char * args, WorldSession * m_session);

    //bool HandleReloadScriptsCommand(const char * args, WorldSession * m_session);
    bool HandleNpcPossessCommand(const char * args, WorldSession * m_session);
    bool HandleNpcUnPossessCommand(const char * args, WorldSession * m_session);
    bool HandleRehashCommand(const char * args, WorldSession * m_session);

    /* QUEST COMMANDS */
    bool HandleQuestAddBothCommand(const char * args, WorldSession * m_session);
    bool HandleQuestAddFinishCommand(const char * args, WorldSession * m_session);
    bool HandleQuestAddStartCommand(const char * args, WorldSession * m_session);
    bool HandleQuestDelBothCommand(const char * args, WorldSession * m_session);
    bool HandleQuestDelFinishCommand(const char * args, WorldSession * m_session);
    bool HandleQuestFinisherCommand(const char * args, WorldSession * m_session);
    bool HandleQuestDelStartCommand(const char * args, WorldSession * m_session);
    bool HandleQuestFinishCommand(const char * args, WorldSession * m_session);
    bool HandleQuestGiverCommand(const char * args, WorldSession * m_session);
    bool HandleQuestItemCommand(const char * args, WorldSession * m_session);
    bool HandleQuestListStarterCommand(const char * args, WorldSession * m_session);
    bool HandleQuestListFinisherCommand(const char * args, WorldSession * m_session);
    bool HandleQuestLoadCommand(const char * args, WorldSession * m_session);
    bool HandleQuestLookupCommand(const char * args, WorldSession * m_session);
    bool HandleQuestRemoveCommand(const char * args, WorldSession * m_session);
    bool HandleQuestRewardCommand(const char * args, WorldSession * m_session);
    bool HandleQuestSpawnCommand(const char * args, WorldSession * m_session);
    bool HandleQuestStartCommand(const char * args, WorldSession * m_session);
    bool HandleQuestStatusCommand(const char * args, WorldSession * m_session);

    bool HandleCreateArenaTeamCommands(const char * args, WorldSession * m_session);
    bool HandleNpcSelectCommand(const char * args, WorldSession * m_session);
    bool HandleWaypointAddFlyCommand(const char * args, WorldSession * m_session);
    bool HandleWhisperBlockCommand(const char * args, WorldSession * m_session);
    bool HandleShowItems(const char * args, WorldSession * m_session);
    bool HandleCollisionTestIndoor(const char * args, WorldSession * m_session);
    bool HandleCollisionTestLOS(const char * args, WorldSession * m_session);
    bool HandleRenameAllCharacter(const char * args, WorldSession * m_session);
    bool HandleCollisionGetHeight(const char * args, WorldSession * m_session);
    bool HandleAccountMuteCommand(const char * args, WorldSession * m_session);
    bool HandleAccountUnmuteCommand(const char * args, WorldSession * m_session);
    /* For skill related GM commands */
    SkillNameMgr *SkillNameManager;

    bool HandleFixScaleCommand(const char * args, WorldSession * m_session);
    bool HandleClearCorpsesCommand(const char *args, WorldSession *m_session);
    bool HandleClearBonesCommand(const char *args, WorldSession *m_session);

    bool HandleMultiMuteCommand(const char *args, WorldSession *m_session);
    bool HandleMultiBanCommand(const char *args, WorldSession *m_session);
    bool HandleMultiAccountBanCommand(const char *args, WorldSession *m_session);
    bool HandleMultiKickCommand(const char *args, WorldSession *m_session);

    // Reputation system command
    bool HandlePosRepCommand(const char *args, WorldSession *m_session);
    bool HandleNegRepCommand(const char *args, WorldSession *m_session);
    bool HandleGetRepCommand(const char *args, WorldSession *m_session);

    bool HandleDebugRetroactiveQuestAchievements(const char *args, WorldSession *m_session);
    bool HandleModifyScaleCommand(const char *args, WorldSession *m_session);
    bool HandleModifyFactionCommand(const char *args, WorldSession *m_session);
    bool HandleModifyPlayerFlagsCommand(const char *args, WorldSession *m_session);
    bool HandleModifyAuraStateCommand(const char *args, WorldSession *m_session);
    bool HandleMirrorTimerCommand(const char *args, WorldSession *m_session);
    bool HandleSetPlayerStartLocation(const char *args, WorldSession *m_session);

    bool HandleEnableAH(const char *args, WorldSession *m_session);
    bool HandleDisableAH(const char *args, WorldSession *m_session);

    bool HandleFactionSetStanding(const char *args, WorldSession *m_session);
    bool HandleFactionModStanding(const char *args, WorldSession *m_session);

    // Warn System
    bool HandleWarnPlayerCommand(const char* args, WorldSession *m_session);
    bool HandleWarnListCommand(const char* args, WorldSession *m_session);
    bool HandleWarnClearCommand(const char* args, WorldSession *m_session);
    bool HandleWarnSingleDelCommand(const char* args, WorldSession *m_session);

    // GM Tracker related commands.
    bool HandleTrackerListCommand(const char * args, WorldSession * m_session);
    bool HandleAddTrackerCommand(const char * args, WorldSession * m_session);
    bool HandleDelTrackerCommand(const char * args, WorldSession * m_session);

    //Id from links
    uint16 GetItemIDFromLink(const char* link, uint32* itemid);
    int32 GetSpellIDFromLink(const char* link);
private:
    Mutex m_lock;
};

//#define USE_NUMERIC_COMMAND_LEVELS

#define COMMAND_LEVEL_0 '0', '0'
#define COMMAND_LEVEL_1 '1', '1'
#define COMMAND_LEVEL_2 '2', '2'
#define COMMAND_LEVEL_3 '3', '3'
#define COMMAND_LEVEL_4 '4', '4'

#define COMMAND_LEVEL_A 'a', '3'
#define COMMAND_LEVEL_B 'b', '2'
#define COMMAND_LEVEL_C 'c', '2'
#define COMMAND_LEVEL_D 'd', '3'
#define COMMAND_LEVEL_E 'e', '2'
#define COMMAND_LEVEL_F 'f', '3'
#define COMMAND_LEVEL_G 'g', '2'
#define COMMAND_LEVEL_H 'h', '0'
#define COMMAND_LEVEL_I 'i', '1'
#define COMMAND_LEVEL_J 'j', '0'
#define COMMAND_LEVEL_K 'k', '0'
#define COMMAND_LEVEL_L 'l', '1'
#define COMMAND_LEVEL_M 'm', '3'
#define COMMAND_LEVEL_N 'n', '2'
#define COMMAND_LEVEL_O 'o', '2'
#define COMMAND_LEVEL_P 'p', '0'
#define COMMAND_LEVEL_Q 'q', '2'
#define COMMAND_LEVEL_R 'r', '2'
#define COMMAND_LEVEL_S 's', '1'
#define COMMAND_LEVEL_T 't', '2'
#define COMMAND_LEVEL_U 'u', '2'
#define COMMAND_LEVEL_V 'v', '2'
#define COMMAND_LEVEL_W 'w', '3'
#define COMMAND_LEVEL_X 'x', '0'
#define COMMAND_LEVEL_Y 'y', '0'
#define COMMAND_LEVEL_Z 'z', '4'

#define sChatHandler ChatHandler::getSingleton()
#define sComTableStore CommandTableStorage::getSingleton()
