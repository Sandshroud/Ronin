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

#pragma once

class Player;
class GMWarden;

class ChatCommand
{
public:
    const char* Name;
    char                CommandGroup;
    char                NumericCGroup;
    bool (GMWarden::* Handler)(const char* args, WorldSession* m_session);
    std::string         Help;
    ChatCommand* ChildCommands;
    uint32              NormalValueField;
    uint32              MaxValueField;
    uint16              ValueType;  // 0 = nothing, 1 = uint, 2 = float
};

class SERVER_DECL CommandTableStorage : public Singleton<CommandTableStorage>
{
    ChatCommand* _modifyCommandTable;
    ChatCommand* _debugCommandTable;
    ChatCommand* _GMTicketCommandTable;
    ChatCommand* _GameObjectCommandTable;
    ChatCommand* _BattlegroundCommandTable;
    ChatCommand* _NPCCommandTable;
    ChatCommand* _gamemasterCommandTable;
    ChatCommand* _trackerCommandTable;
    ChatCommand* _warnCommandTable;
    ChatCommand* _administratorCommandTable;
    ChatCommand* _accountCommandTable;
    ChatCommand* _CheatCommandTable;
    ChatCommand* _honorCommandTable;
    ChatCommand* _questCommandTable;
    ChatCommand* _lookupCommandTable;
    ChatCommand* _petCommandTable;
    ChatCommand* _commandTable;
    ChatCommand* _GuildCommandTable;
    ChatCommand* _TitleCommandTable;
    ChatCommand* _wintergraspCommandTable;
    ChatCommand* _FactionCommandTable;

    // DB commands
    ChatCommand* _DBItemCommandTable;
    ChatCommand* _DatabaseCommandTable;
    // End Database Commands

    ChatCommand* GetSubCommandTable(const char* name);
public:
    void Init();
    void Dealloc();
    RONIN_INLINE ChatCommand* Get() { return _commandTable; }
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

class GMWarden : public Singleton<GMWarden>
{
    friend class CommandTableStorage;
private:

public:
    GMWarden();
    ~GMWarden();

    void Init();

    void LoadGMData();

    void InitializeGM(Player* plr);

    void ProcessGMCommand(Player* gm, const char* message);

private:
    Player* getSelectedChar(WorldSession* m_session, bool showerror = true, bool autoSelectSelf = true);
    Creature* getSelectedCreature(WorldSession* m_session, bool showerror = true);
    Unit* getSelectedUnit(WorldSession* m_session, bool showerror = true);

protected:
    bool ExecuteCommandInTable(ChatCommand* table, const char* text, WorldSession* m_session);
    bool ShowHelpForCommand(WorldSession* m_session, ChatCommand* table, const char* cmd);

    ChatCommand* getCommandTable();

    // Level 0 commands
    bool HandleHelpCommand(const char* args, WorldSession* m_session);
    bool HandleCommandsCommand(const char* args, WorldSession* m_session);
    bool HandleNYICommand(const char* args, WorldSession* m_session);
    bool HandleAcctCommand(const char* args, WorldSession* m_session);
    bool HandleStartCommand(const char* args, WorldSession* m_session);
    bool HandleInfoCommand(const char* args, WorldSession* m_session);
    bool HandleDismountCommand(const char* args, WorldSession* m_session);
    bool HandleFullDismountCommand(const char* args, WorldSession* m_session);
    bool HandleSaveCommand(const char* args, WorldSession* m_session);
    bool HandleGMListCommand(const char* args, WorldSession* m_session);
    bool HandleGmLogCommentCommand(const char* args, WorldSession* m_session);
    bool HandleRatingsCommand(const char* args, WorldSession* m_session);

    // Level 1 commands
    bool CmdSetValueField(WorldSession* m_session, uint32 field, uint32 fieldmax, const char* fieldname, const char* args);
    bool CmdSetFloatField(WorldSession* m_session, uint32 field, uint32 fieldmax, const char* fieldname, const char* args);
    bool HandleSummonCommand(const char* args, WorldSession* m_session);
    bool HandleAppearCommand(const char* args, WorldSession* m_session);
    bool HandleTeleportCommand(const char* args, WorldSession* m_session);
    bool HandleTeleportXYZCommand(const char* args, WorldSession* m_session);
    bool HandleAnnounceCommand(const char* args, WorldSession* m_session);
    bool HandleAdminAnnounceCommand(const char* args, WorldSession* m_session);
    bool HandleGMAnnounceCommand(const char* args, WorldSession* m_session);
    bool HandleWAnnounceCommand(const char* args, WorldSession* m_session);
    bool HandleGMOnCommand(const char* args, WorldSession* m_session);
    bool HandleGMOffCommand(const char* args, WorldSession* m_session);
    bool HandleGPSCommand(const char* args, WorldSession* m_session);
    bool HandleKickCommand(const char* args, WorldSession* m_session);
    bool HandleTaxiCheatCommand(const char* args, WorldSession* m_session);
    bool HandleModifyTPsCommand(const char* args, WorldSession* m_session);

    // Debug Commands
    bool HandleDebugGoDamage(const char* args, WorldSession* m_session);
    bool HandleDebugGoRepair(const char* args, WorldSession* m_session);
    bool HandleDebugInFrontCommand(const char* args, WorldSession* m_session);
    bool HandleShowReactionCommand(const char* args, WorldSession* m_session);
    bool HandleMoveInfoCommand(const char* args, WorldSession* m_session);
    bool HandleDistanceCommand(const char* args, WorldSession* m_session);
    bool HandleFaceCommand(const char* args, WorldSession* m_session);
    bool HandleSetBytesCommand(const char* args, WorldSession* m_session);
    bool HandleGetBytesCommand(const char* args, WorldSession* m_session);
    bool HandleAggroRangeCommand(const char* args, WorldSession* m_session);
    bool HandleKnockBackCommand(const char* args, WorldSession* m_session);
    bool HandleNpcSpawnLinkCommand(const char* args, WorldSession* m_session);
    bool HandleRangeCheckCommand(const char* args, WorldSession* m_session);

    // Guild commands
    bool CreateGuildCommand(const char* args, WorldSession* m_session);
    bool HandleGuildRemovePlayerCommand(const char* args, WorldSession* m_session);
    bool HandleGuildDisbandCommand(const char* args, WorldSession* m_session);
    bool HandleGuildModifyLevelCommand(const char* args, WorldSession* m_session);
    bool HandleGuildGainXPCommand(const char* args, WorldSession* m_session);

    // Level 2 commands
    bool HandleNameCommand(const char* args, WorldSession* m_session);
    bool HandleSubNameCommand(const char* args, WorldSession* m_session);
    bool HandleDeleteCommand(const char* args, WorldSession* m_session);
    bool HandleDeMorphCommand(const char* args, WorldSession* m_session);
    bool HandleItemCommand(const char* args, WorldSession* m_session);
    bool HandleItemRemoveCommand(const char* args, WorldSession* m_session);
    bool HandleNPCFlagCommand(const char* args, WorldSession* m_session);
    bool HandleSaveAllCommand(const char* args, WorldSession* m_session);
    bool HandleRegenerateGossipCommand(const char* args, WorldSession* m_session);
    bool HandleStartBGCommand(const char* args, WorldSession* m_session);
    bool HandlePauseBGCommand(const char* args, WorldSession* m_session);
    bool HandleResetScoreCommand(const char* args, WorldSession* m_session);
    bool HandleBGInfoCommnad(const char* args, WorldSession* m_session);
    bool HandleKillCommand(const char* args, WorldSession* m_session);
    bool HandleKillByPlrCommand(const char* args, WorldSession* m_session);
    bool HandleCreatureSpawnCommand(const char* args, WorldSession* m_session);
    bool HandleCreatureRespawnCommand(const char* args, WorldSession* m_session);
    bool HandleGOSelect(const char* args, WorldSession* m_session);
    bool HandleGODelete(const char* args, WorldSession* m_session);
    bool HandleGOSpawn(const char* args, WorldSession* m_session);
    bool HandleGOInfo(const char* args, WorldSession* m_session);
    bool HandleGOEnable(const char* args, WorldSession* m_session);
    bool HandleGOActivate(const char* args, WorldSession* m_session);
    bool HandleGORotate(const char* args, WorldSession* m_session);
    bool HandleGOMove(const char* args, WorldSession* m_session);

    // Level 3 commands
    bool HandleMassSummonCommand(const char* args, WorldSession* m_session);
    bool HandleWorldPortCommand(const char* args, WorldSession* m_session);
    bool HandleMoveCommand(const char* args, WorldSession* m_session);
    bool HandleLearnCommand(const char* args, WorldSession* m_session);
    bool HandleReviveCommand(const char* args, WorldSession* m_session);
    bool HandleAddGraveCommand(const char* args, WorldSession* m_session);
    bool HandleAddSHCommand(const char* args, WorldSession* m_session);
    bool HandleExploreCheatCommand(const char* args, WorldSession* m_session);
    bool HandleAddSkillCommand(const char* args, WorldSession* m_session);
    bool HandleItemInfoCommand(const char* args, WorldSession* m_session);
    bool HandleAddInvItemCommand(const char* args, WorldSession* m_session);
    bool HandleResetReputationCommand(const char* args, WorldSession* m_session);
    bool HandleLearnSkillCommand(const char* args, WorldSession* m_session);
    bool HandleModifySkillCommand(const char* args, WorldSession* m_session);
    bool HandleRemoveSkillCommand(const char* args, WorldSession* m_session);
    bool HandleNpcInfoCommand(const char* args, WorldSession* m_session);
    bool HandleEmoteCommand(const char* args, WorldSession* m_session);
    bool HandleStandStateCommand(const char* args, WorldSession* m_session);
    bool HandleIncreaseWeaponSkill(const char* args, WorldSession* m_session);
    bool HandleCastSpellCommand(const char* args, WorldSession* m_session);
    bool HandleCastSpellNECommand(const char* args, WorldSession* m_session);
    bool HandleGenderChanger(const char* args, WorldSession* m_session);
    bool HandleModifyGoldCommand(const char* args, WorldSession* m_session);
    bool HandleMonsterSayCommand(const char* args, WorldSession* m_session);
    bool HandleMonsterYellCommand(const char* args, WorldSession* m_session);
    bool HandleMonsterCastCommand(const char* args, WorldSession* m_session);
    bool HandleNPCEquipCommand(const char* args, WorldSession* m_session);
    bool HandleNPCSetOnObjectCommand(const char* args, WorldSession* m_session);
    bool HandleNPCSaveCommand(const char* args, WorldSession* m_session);
    bool HandleNPCSetVendorMaskCommand(const char* args, WorldSession* m_session);
    bool HandleNpcComeCommand(const char* args, WorldSession* m_session);
    bool HandleNpcJumpCommand(const char* args, WorldSession* m_session);
    bool HandleClearCooldownsCommand(const char* args, WorldSession* m_session);
    bool HandleSetWorldStateCommand(const char* args, WorldSession* m_session);
    bool HandlePlaySoundCommand(const char* args, WorldSession* m_session);
    bool HandleSetBattlefieldStatusCommand(const char* args, WorldSession* m_session);
    bool HandleNpcReturnCommand(const char* args, WorldSession* m_session);
    bool HandleAccountBannedCommand(const char* args, WorldSession* m_session);
    bool HandleAccountLevelCommand(const char* args, WorldSession* m_session);
    bool HandleResetTalentsCommand(const char* args, WorldSession* m_session);
    bool HandleResetSpellsCommand(const char* args, WorldSession* m_session);
    bool HandleNpcFollowCommand(const char* args, WorldSession* m_session);
    bool HandleNullFollowCommand(const char* args, WorldSession* m_session);
    bool HandleResetSkillsCommand(const char* args, WorldSession* m_session);
    bool HandleGetSkillLevelCommand(const char* args, WorldSession* m_session);
    bool HandleGetSkillsInfoCommand(const char* args, WorldSession* m_session);
    bool HandleEarnAchievement(const char* args, WorldSession* m_session);
    bool HandleAddCurrency(const char* args, WorldSession* m_session);
    bool HandleRemoveCurrency(const char* args, WorldSession* m_session);
    bool HandlePlayerInfo(const char* args, WorldSession* m_session);
    bool HandleNPCGetSpeed(const char* args, WorldSession* m_session);
    bool HandleNPCGetResist(const char* args, WorldSession* m_session);

    // Tickets
    void SendGMSurvey();
    bool HandleGMTicketListCommand(const char* args, WorldSession* m_session);
    bool HandleGMTicketGetByIdCommand(const char* args, WorldSession* m_session);
    bool HandleGMTicketRemoveByIdCommand(const char* args, WorldSession* m_session);
    bool HandleGMTicketAssignToCommand(const char* args, WorldSession* m_session);
    bool HandleGMTicketReleaseCommand(const char* args, WorldSession* m_session);
    bool HandleGMTicketCommentCommand(const char* args, WorldSession* m_session);
    bool HandleGMTicketDeletePermanentCommand(const char* args, WorldSession* m_session);

    //Ban
    bool HandleBanCharacterCommand(const char* args, WorldSession* m_session);
    bool HandleUnBanCharacterCommand(const char* args, WorldSession* m_session);

    //BG
    bool HandleSetBGScoreCommand(const char* args, WorldSession* m_session);

    bool HandleGOScale(const char* args, WorldSession* m_session);
    bool HandleReviveStringcommand(const char* args, WorldSession* m_session);
    bool HandleMountCommand(const char* args, WorldSession* m_session);
    bool HandleTestGravityOpcode(const char* args, WorldSession* m_session);
    bool HandleGetPosCommand(const char* args, WorldSession* m_session);
    bool HandleGetTransporterTime(const char* args, WorldSession* m_session);
    bool HandleGOAnimProgress(const char* args, WorldSession* m_session);
    bool HandleRemoveAurasCommand(const char* args, WorldSession* m_session);
    bool HandleParalyzeCommand(const char* args, WorldSession* m_session);
    bool HandleUnParalyzeCommand(const char* args, WorldSession* m_session);
    bool HandleSetMotdCommand(const char* args, WorldSession* m_session);
    bool HandleAddItemSetCommand(const char* args, WorldSession* m_session);
    bool HandleModifyValueCommand(const char* args, WorldSession* m_session);
    bool HandleModifyBitCommand(const char* args, WorldSession* m_session);
    bool HandleBattlegroundExitCommand(const char* args, WorldSession* m_session);
    bool HandleBattlegroundForcestartCommand(const char* args, WorldSession* m_session);
    bool HandleCooldownCheatCommand(const char* args, WorldSession* m_session);
    bool HandleCastTimeCheatCommand(const char* args, WorldSession* m_session);
    bool HandlePowerCheatCommand(const char* args, WorldSession* m_session);
    bool HandleShowCheatsCommand(const char* args, WorldSession* m_session);
    bool HandleStackCheatCommand(const char* args, WorldSession* m_session);
    bool HandleTriggerpassCheatCommand(const char* args, WorldSession* m_session);
    bool HandleVendorPassCheatCommand(const char* args, WorldSession* m_session);
    bool HandleItemReqCheatCommand(const char* args, WorldSession* m_session);
    bool HandleFlyCommand(const char* args, WorldSession* m_session);
    bool HandleRemoveRessurectionSickessAuraCommand(const char* args, WorldSession* m_session);
    bool HandleDBReloadCommand(const char* args, WorldSession* m_session);
    bool HandleItemSetCommand(const char* args, WorldSession* m_session);
    bool HandleVendorClearCommand(const char* args, WorldSession* m_session);
    bool HandleItemSetRemoveCommand(const char* args, WorldSession* m_session);

    // honor
    bool HandleAddTitleCommand(const char* args, WorldSession* m_session);
    bool HandleRemoveTitleCommand(const char* args, WorldSession* m_session);
    bool HandleGetKnownTitlesCommand(const char* args, WorldSession* m_session);
    bool HandleSetChosenTitleCommand(const char* args, WorldSession* m_session);

    bool HandleUnlearnCommand(const char* args, WorldSession* m_session);
    bool HandleModifyLevelCommand(const char* args, WorldSession* m_session);

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

    bool HandleModPeriodCommand(const char* args, WorldSession* m_session);
    bool HandleGlobalPlaySoundCommand(const char* args, WorldSession* m_session);

    // Bans
    bool HandleIPBanCommand(const char* args, WorldSession* m_session);
    bool HandleIPUnBanCommand(const char* args, WorldSession* m_session);
    bool HandleAccountUnbanCommand(const char* args, WorldSession* m_session);

    // Item removal
    bool HandleRemoveItemCommand(const char* args, WorldSession* m_session);

    // Rename
    bool HandleRenameCommand(const char* args, WorldSession* m_session);
    bool HandleForceRenameCommand(const char* args, WorldSession* m_session);
    bool HandleRecustomizeCharCommand(const char* args, WorldSession* m_session);

    // Reputation
    bool HandleGetStandingCommand(const char* args, WorldSession* m_session);
    bool HandleSetStandingCommand(const char* args, WorldSession* m_session);
    bool HandleGetBaseStandingCommand(const char* args, WorldSession* m_session);

    // Lookups
    bool HandleLookupItemCommand(const char* args, WorldSession* m_session);
    bool HandleLookupItemSetCommand(const char* args, WorldSession* m_session);
    bool HandleLookupCreatureCommand(const char* args, WorldSession* m_session);
    bool HandleLookupObjectCommand(const char* args, WorldSession* m_session);
    bool HandleLookupSpellCommand(const char* args, WorldSession* m_session);
    bool HandleLookupSpellSpecificCommand(const char* args, WorldSession* m_session);
    bool HandleLookupSpellNameCommand(const char* args, WorldSession* m_session);
    bool HandleLookupTitleCommand(const char* args, WorldSession* m_session);
    bool HandleLookupCurrencyCommand(const char* args, WorldSession* m_session);

    bool HandleNpcPossessCommand(const char* args, WorldSession* m_session);
    bool HandleNpcUnPossessCommand(const char* args, WorldSession* m_session);
    bool HandleRehashCommand(const char* args, WorldSession* m_session);

    /* QUEST COMMANDS */
    bool HandleQuestAddBothCommand(const char* args, WorldSession* m_session);
    bool HandleQuestAddFinishCommand(const char* args, WorldSession* m_session);
    bool HandleQuestAddStartCommand(const char* args, WorldSession* m_session);
    bool HandleQuestDelBothCommand(const char* args, WorldSession* m_session);
    bool HandleQuestDelFinishCommand(const char* args, WorldSession* m_session);
    bool HandleQuestFinisherCommand(const char* args, WorldSession* m_session);
    bool HandleQuestDelStartCommand(const char* args, WorldSession* m_session);
    bool HandleQuestFinishCommand(const char* args, WorldSession* m_session);
    bool HandleQuestGiverCommand(const char* args, WorldSession* m_session);
    bool HandleQuestItemCommand(const char* args, WorldSession* m_session);
    bool HandleQuestListStarterCommand(const char* args, WorldSession* m_session);
    bool HandleQuestListFinisherCommand(const char* args, WorldSession* m_session);
    bool HandleQuestLoadCommand(const char* args, WorldSession* m_session);
    bool HandleQuestLookupCommand(const char* args, WorldSession* m_session);
    bool HandleQuestRemoveCommand(const char* args, WorldSession* m_session);
    bool HandleQuestRewardCommand(const char* args, WorldSession* m_session);
    bool HandleQuestSpawnCommand(const char* args, WorldSession* m_session);
    bool HandleQuestStartCommand(const char* args, WorldSession* m_session);
    bool HandleQuestStatusCommand(const char* args, WorldSession* m_session);

    bool HandleCreateArenaTeamCommands(const char* args, WorldSession* m_session);
    bool HandleWhisperBlockCommand(const char* args, WorldSession* m_session);
    bool HandleShowItems(const char* args, WorldSession* m_session);
    bool HandleCollisionTestIndoor(const char* args, WorldSession* m_session);
    bool HandleCollisionTestLOS(const char* args, WorldSession* m_session);
    bool HandleRenameAllCharacter(const char* args, WorldSession* m_session);
    bool HandleCollisionGetHeight(const char* args, WorldSession* m_session);
    bool HandleAccountMuteCommand(const char* args, WorldSession* m_session);
    bool HandleAccountUnmuteCommand(const char* args, WorldSession* m_session);
    /* For skill related GM commands */
    SkillNameMgr* SkillNameManager;

    bool HandleFixScaleCommand(const char* args, WorldSession* m_session);
    bool HandleClearCorpsesCommand(const char* args, WorldSession* m_session);
    bool HandleClearBonesCommand(const char* args, WorldSession* m_session);

    bool HandleMultiMuteCommand(const char* args, WorldSession* m_session);
    bool HandleMultiBanCommand(const char* args, WorldSession* m_session);
    bool HandleMultiAccountBanCommand(const char* args, WorldSession* m_session);
    bool HandleMultiKickCommand(const char* args, WorldSession* m_session);

    // Reputation system command
    bool HandlePosRepCommand(const char* args, WorldSession* m_session);
    bool HandleNegRepCommand(const char* args, WorldSession* m_session);
    bool HandleGetRepCommand(const char* args, WorldSession* m_session);

    bool HandleDebugRetroactiveQuestAchievements(const char* args, WorldSession* m_session);
    bool HandleModifyScaleCommand(const char* args, WorldSession* m_session);
    bool HandleModifyFactionCommand(const char* args, WorldSession* m_session);
    bool HandleModifyPlayerFlagsCommand(const char* args, WorldSession* m_session);
    bool HandleModifyAuraStateCommand(const char* args, WorldSession* m_session);
    bool HandleMirrorTimerCommand(const char* args, WorldSession* m_session);
    bool HandleSetPlayerStartLocation(const char* args, WorldSession* m_session);
    bool HandleModifySpeedCommand(const char* args, WorldSession* m_session);
    bool HandleModifySwimSpeedCommand(const char* args, WorldSession* m_session);
    bool HandleModifyFlightSpeedCommand(const char* args, WorldSession* m_session);

    bool HandleEnableAH(const char* args, WorldSession* m_session);
    bool HandleDisableAH(const char* args, WorldSession* m_session);

    bool HandleFactionSetStanding(const char* args, WorldSession* m_session);
    bool HandleFactionModStanding(const char* args, WorldSession* m_session);

    // Warn System
    bool HandleWarnPlayerCommand(const char* args, WorldSession* m_session);
    bool HandleWarnListCommand(const char* args, WorldSession* m_session);
    bool HandleWarnClearCommand(const char* args, WorldSession* m_session);
    bool HandleWarnSingleDelCommand(const char* args, WorldSession* m_session);

    // GM Tracker related commands.
    bool HandleTrackerListCommand(const char* args, WorldSession* m_session);
    bool HandleAddTrackerCommand(const char* args, WorldSession* m_session);
    bool HandleDelTrackerCommand(const char* args, WorldSession* m_session);

    //Id from links
    uint16 GetItemIDFromLink(const char* link, uint32* itemid);
    int32 GetSpellIDFromLink(const char* link);
};

#define sGMWarden GMWarden::getSingleton()
#define sComTableStore CommandTableStorage::getSingleton()
