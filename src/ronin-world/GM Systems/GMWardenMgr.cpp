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

initialiseSingleton(GMWarden);
initialiseSingleton(CommandTableStorage);


GMWarden::GMWarden() : SkillNameManager(NULL)
{
    new CommandTableStorage;
    sComTableStore.Init();

    SkillNameManager = new SkillNameMgr();
}

GMWarden::~GMWarden()
{
    sComTableStore.Dealloc();
    delete SkillNameManager;
}

void GMWarden::Init()
{

}

void GMWarden::LoadGMData()
{
    sLog.Notice("GMWarden", "Loading GM data");

}

void GMWarden::InitializeGM(Player* plr)
{
    // Set force flag
    plr->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_GM | PLAYER_FLAG_DEVELOPER);

}

bool GMWarden::ExecuteCommandInTable(ChatCommand* table, const char* text, WorldSession* gmSession)
{
    Player *gm = gmSession->GetPlayer();
    std::string cmd = "";

    // get command
    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        text++;
    }

    while (*text == ' ') text++; // skip whitespace

    if (!cmd.length())
        return false;

    for (uint32 i = 0; table[i].Name != NULL; i++)
    {
        if (!ChatHandler::hasStringAbbr(table[i].Name, cmd.c_str()))
            continue;

        if (table[i].CommandGroup != '0')
        {
            if (sWorld.NumericCommandGroups)
            {
                if (!gmSession->CanUseCommand(table[i].NumericCGroup))
                    continue;
            }
            else
            {
                if (!gmSession->CanUseCommand(table[i].CommandGroup))
                    continue;
            }
        }

        if (table[i].ChildCommands != NULL)
        {
            if (!ExecuteCommandInTable(table[i].ChildCommands, text, gmSession))
            {
                if (table[i].Help != "")
                    sChatHandler.SendMultilineMessage(gmSession, table[i].Help.c_str());
                else
                {
                    sChatHandler.GreenSystemMessage(gmSession, "Available Subcommands:");
                    for (uint32 k = 0; table[i].ChildCommands[k].Name; k++)
                    {
                        if (table[i].ChildCommands[k].CommandGroup != '0' && gmSession->CanUseCommand(table[i].ChildCommands[k].CommandGroup))
                            sChatHandler.BlueSystemMessage(gmSession, " %s - %s", table[i].ChildCommands[k].Name, table[i].ChildCommands[k].Help.size() ? table[i].ChildCommands[k].Help.c_str() : "No Help Available");
                    }
                }
            }

            return true;
        }

        // Check for field-based commands
        if (table[i].Handler == NULL && (table[i].MaxValueField || table[i].NormalValueField))
        {
            bool result = false;
            if (strlen(text) == 0)
            {
                sChatHandler.RedSystemMessage(gmSession, "No values specified.");
            }
            if (table[i].ValueType == 2)
                result = CmdSetFloatField(gmSession, table[i].NormalValueField, table[i].MaxValueField, table[i].Name, text);
            else if (table[i].ValueType == 3)
            {
                result = true;
                GameObject* go = NULL;
                if (!gm->m_selectedGo.empty() && (go = gm->GetInRangeObject<GameObject>(gm->m_selectedGo)) != NULL)
                {
                    go->SetUInt32Value(table[i].NormalValueField, atoi(text));
                    go->SaveToDB();
                }
                else result = false;
            }
            else result = CmdSetValueField(gmSession, table[i].NormalValueField, table[i].MaxValueField, table[i].Name, text);

            if (!result)
                sChatHandler.RedSystemMessage(gmSession, "Must be in the form of (command) <value>, or, (command) <value> <maxvalue>");
        }
        else
        {
            if (!(this->*(table[i].Handler))(text, gmSession))
            {
                if (table[i].Help != "")
                    sChatHandler.SendMultilineMessage(gmSession, table[i].Help.c_str());
                else
                {
                    sChatHandler.RedSystemMessage(gmSession, "Incorrect syntax specified. Try .help %s for the correct syntax.", table[i].Name);
                }
            }
        }

        return true;
    }
    return false;
}

void GMWarden::ProcessGMCommand(Player* gm, const char* message)
{
    WorldSession* gmSession = gm->GetSession();

    // Start command calldown
    if (!ExecuteCommandInTable(sComTableStore.Get(), message, gmSession))
        sChatHandler.SystemMessage(gmSession, "There is no such command, or you do not have access to it.");
}

Player* GMWarden::getSelectedChar(WorldSession* m_session, bool showerror, bool autoSelectSelf)
{
    Player* chr = NULL;
    WoWGuid guid = m_session->GetPlayer()->GetSelection();
    if (guid.empty())
    {
        if (autoSelectSelf)
        {
            if (showerror)
                sChatHandler.GreenSystemMessage(m_session, "Auto-targeting self.");
            chr = m_session->GetPlayer(); // autoselect
        }
    }
    else if (guid.getHigh() == HIGHGUID_TYPE_PLAYER)
        chr = m_session->GetPlayer()->GetInRangeObject<Player>(guid);

    if (chr == NULL && showerror)
        sChatHandler.RedSystemMessage(m_session, "This command requires that you select a player.");
    return chr;
}

Creature* GMWarden::getSelectedCreature(WorldSession* m_session, bool showerror)
{
    if (!m_session->GetPlayer()->IsInWorld())
        return NULL;

    WoWGuid guid = m_session->GetPlayer()->GetSelection();
    if (guid.empty() || (guid.getHigh() != HIGHGUID_TYPE_UNIT && guid.getHigh() != HIGHGUID_TYPE_VEHICLE))
    {
        if (showerror) sChatHandler.RedSystemMessage(m_session, "This command requires that you select a creature.");
        return NULL;
    }

    return m_session->GetPlayer()->GetInRangeObject<Creature>(guid);
}

Unit* GMWarden::getSelectedUnit(WorldSession* m_session, bool showerror)
{
    if (!m_session->GetPlayer()->IsInWorld())
        return NULL;

    WoWGuid guid = m_session->GetPlayer()->GetSelection();
    if (guid.empty() || (guid.getHigh() != HIGHGUID_TYPE_UNIT && guid.getHigh() != HIGHGUID_TYPE_VEHICLE && guid.getHigh() != HIGHGUID_TYPE_PLAYER))
    {
        if (showerror) sChatHandler.RedSystemMessage(m_session, "This command requires that you select a unit.");
        return NULL;
    }

    return m_session->GetPlayer()->GetInRangeObject<Unit>(guid);
}

ChatCommand* GMWarden::getCommandTable()
{
    ASSERT(false);
    return 0;
}

ChatCommand* CommandTableStorage::GetSubCommandTable(const char* name)
{
    if (!stricmp(name, "modify"))
        return _modifyCommandTable;
    else if (!stricmp(name, "debug"))
        return _debugCommandTable;
    else if (!stricmp(name, "ticket"))
        return _GMTicketCommandTable;
    else if (!stricmp(name, "gobject"))
        return _GameObjectCommandTable;
    else if (!stricmp(name, "battleground"))
        return _BattlegroundCommandTable;
    else if (!stricmp(name, "npc"))
        return _NPCCommandTable;
    else if (!stricmp(name, "gm"))
        return _gamemasterCommandTable;
    else if (!stricmp(name, "tracker"))
        return _trackerCommandTable;
    else if (!stricmp(name, "warn"))
        return _warnCommandTable;
    else if (!stricmp(name, "admin"))
        return _administratorCommandTable;
    else if (!stricmp(name, "cheat"))
        return _CheatCommandTable;
    else if (!stricmp(name, "account"))
        return _accountCommandTable;
    else if (!stricmp(name, "honor"))
        return _honorCommandTable;
    else if (!stricmp(name, "guild"))
        return _GuildCommandTable;
    else if (!stricmp(name, "title"))
        return _TitleCommandTable;
    else if (!stricmp(name, "quest"))
        return _questCommandTable;
    else if (!stricmp(name, "lookup"))
        return _lookupCommandTable;
    else if (!stricmp(name, "faction"))
        return _FactionCommandTable;
    return 0;
}

#define dupe_command_table(ct, dt) this->dt = (ChatCommand*)allocate_and_copy(sizeof(ct)/* / sizeof(ct[0])*/, ct)
RONIN_INLINE void* allocate_and_copy(uint32 len, void* pointer)
{
    void* data = (void*)malloc(len);
    memcpy(data, pointer, len);
    return data;
}

void CommandTableStorage::Dealloc()
{
    free(_modifyCommandTable);
    free(_debugCommandTable);
    free(_GMTicketCommandTable);
    free(_GameObjectCommandTable);
    free(_BattlegroundCommandTable);
    free(_NPCCommandTable);
    free(_gamemasterCommandTable);
    free(_trackerCommandTable);
    free(_warnCommandTable);
    free(_administratorCommandTable);
    free(_CheatCommandTable);
    free(_accountCommandTable);
    free(_honorCommandTable);
    free(_GuildCommandTable);
    free(_TitleCommandTable);
    free(_questCommandTable);
    free(_lookupCommandTable);
    free(_commandTable);
}

void CommandTableStorage::Init()
{
    static ChatCommand modifyCommandTable[] =
    {
        { "hp",                         COMMAND_LEVEL_M, NULL,                                          "Health Points/HP",         NULL, UNIT_FIELD_HEALTH,            UNIT_FIELD_MAXHEALTH,   1 },
        { "gender",                     COMMAND_LEVEL_M, &GMWarden::HandleGenderChanger,             "Changes gender",           NULL, 0,                            0,                      0 },
        { "level",                      COMMAND_LEVEL_M, &GMWarden::HandleModifyLevelCommand,        "Level",                    NULL, 0,                            0,                      0 },
        { "armor",                      COMMAND_LEVEL_M, NULL,                                          "Armor",                    NULL, UNIT_FIELD_RESISTANCES,       0,                      1 },
        { "holy",                       COMMAND_LEVEL_M, NULL,                                          "Holy Resistance",          NULL, UNIT_FIELD_RESISTANCES + 1,     0,                      1 },
        { "fire",                       COMMAND_LEVEL_M, NULL,                                          "Fire Resistance",          NULL, UNIT_FIELD_RESISTANCES + 2,     0,                      1 },
        { "nature",                     COMMAND_LEVEL_M, NULL,                                          "Nature Resistance",        NULL, UNIT_FIELD_RESISTANCES + 3,     0,                      1 },
        { "frost",                      COMMAND_LEVEL_M, NULL,                                          "Frost Resistance",         NULL, UNIT_FIELD_RESISTANCES + 4,     0,                      1 },
        { "shadow",                     COMMAND_LEVEL_M, NULL,                                          "Shadow Resistance",        NULL, UNIT_FIELD_RESISTANCES + 5,     0,                      1 },
        { "arcane",                     COMMAND_LEVEL_M, NULL,                                          "Arcane Resistance",        NULL, UNIT_FIELD_RESISTANCES + 6,     0,                      1 },
        { "damage",                     COMMAND_LEVEL_M, NULL,                                          "Unit Damage Min/Max",      NULL, UNIT_FIELD_MINDAMAGE,         UNIT_FIELD_MAXDAMAGE,   2 },
        { "scale",                      COMMAND_LEVEL_M, &GMWarden::HandleModifyScaleCommand,        "Size/Scale",               NULL, 0,                            0,                      2 },
        { "gold",                       COMMAND_LEVEL_M, &GMWarden::HandleModifyGoldCommand,         "Gold/Money/Copper",        NULL, 0,                            0,                      0 },
        { "nativedisplayid",            COMMAND_LEVEL_M, NULL,                                          "Native Display ID",        NULL, UNIT_FIELD_NATIVEDISPLAYID,   0,                      1 },
        { "displayid",                  COMMAND_LEVEL_M, NULL,                                          "Display ID",               NULL, UNIT_FIELD_DISPLAYID,         0,                      1 },
        { "flags",                      COMMAND_LEVEL_M, NULL,                                          "Unit Flags",               NULL, UNIT_FIELD_FLAGS,             0,                      1 },
        { "faction",                    COMMAND_LEVEL_M, &GMWarden::HandleModifyFactionCommand,      "Faction Template",         NULL, 0,                            0,                      1 },
        { "dynamicflags",               COMMAND_LEVEL_M, NULL,                                          "Dynamic Flags",            NULL, UNIT_DYNAMIC_FLAGS,           0,                      1 },
        { "talentpoints",               COMMAND_LEVEL_M, &GMWarden::HandleModifyTPsCommand,          "Talent points",            NULL, 0,                            0,                      0 },
        { "spirit",                     COMMAND_LEVEL_M, NULL,                                          "Spirit",                   NULL, UNIT_FIELD_SPIRIT,            0,                      1 },
        { "boundingraidius",            COMMAND_LEVEL_M, NULL,                                          "Bounding Radius",          NULL, UNIT_FIELD_BOUNDINGRADIUS,    0,                      2 },
        { "combatreach",                COMMAND_LEVEL_M, NULL,                                          "Combat Reach",             NULL, UNIT_FIELD_COMBATREACH,       0,                      2 },
        { "bytes",                      COMMAND_LEVEL_M, NULL,                                          "Bytes",                    NULL, UNIT_FIELD_BYTES_0,           0,                      1 },
        { "playerflags",                COMMAND_LEVEL_M, &GMWarden::HandleModifyPlayerFlagsCommand,  "modify a player's flags",  NULL, 0,                            0,                      0 },
        { "aurastate",                  COMMAND_LEVEL_M, &GMWarden::HandleModifyAuraStateCommand,    "mods player's aurastate",  NULL, 0,                            0,                      0 },
        { "speed",                      COMMAND_LEVEL_M, &GMWarden::HandleModifySpeedCommand,        "mods unit's speed",        NULL, 0,                            0,                      0 },
        { "swim",                       COMMAND_LEVEL_M, &GMWarden::HandleModifySwimSpeedCommand,    "mods unit's swim speed",   NULL, 0,                            0,                      0 },
        { "flight",                     COMMAND_LEVEL_M, &GMWarden::HandleModifyFlightSpeedCommand,  "mods unit's flight speed", NULL, 0,                            0,                      0 },
        { NULL,                         COMMAND_LEVEL_0, NULL,                                          "",                         NULL, 0,                            0,                      0 }
    };
    dupe_command_table(modifyCommandTable, _modifyCommandTable);

    static ChatCommand warnCommandTable[] =
    {
        { "add",                        COMMAND_LEVEL_T, &GMWarden::HandleWarnPlayerCommand,     "Warns a player, Syntax: !warn add <playername> <reason>",                      NULL, 0, 0, 0 },
        { "list",                       COMMAND_LEVEL_T, &GMWarden::HandleWarnListCommand,       "Warns a player, Syntax: !warn list <playername>",                              NULL, 0, 0, 0 },
        { "clearall",                   COMMAND_LEVEL_Z, &GMWarden::HandleWarnClearCommand,      "Clears warns from a player, Syntax: !warn clear <playername>",                 NULL, 0, 0, 0 },
        { "delete",                     COMMAND_LEVEL_T, &GMWarden::HandleWarnSingleDelCommand,  "Deletes a warn from a player, Syntax: !warn delete <playername> <WarnID>",     NULL, 0, 0, 0 },
        { NULL,                         COMMAND_LEVEL_0, NULL,                                      "",                                                                             NULL, 0, 0, 0 }
    };
    dupe_command_table(warnCommandTable, _warnCommandTable);

    static ChatCommand debugCommandTable[] =
    {
        { "retroactivequest",           COMMAND_LEVEL_D, &GMWarden::HandleDebugRetroactiveQuestAchievements,     "",                                                                                                                     NULL, 0, 0, 0 },
        { "infront",                    COMMAND_LEVEL_D, &GMWarden::HandleDebugInFrontCommand,                   "",                                                                                                                     NULL, 0, 0, 0 },
        { "showreact",                  COMMAND_LEVEL_D, &GMWarden::HandleShowReactionCommand,                   "",                                                                                                                     NULL, 0, 0, 0 },
        { "dist",                       COMMAND_LEVEL_D, &GMWarden::HandleDistanceCommand,                       "",                                                                                                                     NULL, 0, 0, 0 },
        { "face",                       COMMAND_LEVEL_D, &GMWarden::HandleFaceCommand,                           "",                                                                                                                     NULL, 0, 0, 0 },
        { "moveinfo",                   COMMAND_LEVEL_D, &GMWarden::HandleMoveInfoCommand,                       "",                                                                                                                     NULL, 0, 0, 0 },
        { "setbytes",                   COMMAND_LEVEL_Z, &GMWarden::HandleSetBytesCommand,                       "",                                                                                                                     NULL, 0, 0, 0 },
        { "getbytes",                   COMMAND_LEVEL_D, &GMWarden::HandleGetBytesCommand,                       "",                                                                                                                     NULL, 0, 0, 0 },
        { "castspellne",                COMMAND_LEVEL_D, &GMWarden::HandleCastSpellNECommand,                    ".castspellne <spellid> - Casts spell on target (only plays animations, doesnt handle effects or range/facing/etc.",    NULL, 0, 0, 0 },
        { "aggrorange",                 COMMAND_LEVEL_D, &GMWarden::HandleAggroRangeCommand,                     ".aggrorange - Shows aggro Range of the selected Creature.",                                                            NULL, 0, 0, 0 },
        { "knockback ",                 COMMAND_LEVEL_D, &GMWarden::HandleKnockBackCommand,                      ".knockback <hspeed> <vspeed> - Knocks selected player back.",                                                          NULL, 0, 0, 0 },
        { "gettptime",                  COMMAND_LEVEL_D, &GMWarden::HandleGetTransporterTime,                    "grabs transporter travel time",                                                                                        NULL, 0, 0, 0 },
        { "setbit",                     COMMAND_LEVEL_D, &GMWarden::HandleModifyBitCommand,                      "",                                                                                                                     NULL, 0, 0, 0 },
        { "setvalue",                   COMMAND_LEVEL_D, &GMWarden::HandleModifyValueCommand,                    "",                                                                                                                     NULL, 0, 0, 0 },
        { "rangecheck",                 COMMAND_LEVEL_D, &GMWarden::HandleRangeCheckCommand,                     "Checks the 'yard' range and internal range between the player and the target.",                                        NULL, 0, 0, 0 },
        { "setallratings",              COMMAND_LEVEL_D, &GMWarden::HandleRatingsCommand,                        "Sets rating values to incremental numbers based on their index.",                                                      NULL, 0, 0, 0 },
        { "sendmirrortimer",            COMMAND_LEVEL_D, &GMWarden::HandleMirrorTimerCommand,                    "Sends a mirror Timer opcode to target syntax: <type>",                                                                 NULL, 0, 0, 0 },
        { "setstartlocation",           COMMAND_LEVEL_D, &GMWarden::HandleSetPlayerStartLocation,                "",                                                                                                                     NULL, 0, 0, 0 },
        { NULL,                         COMMAND_LEVEL_0, NULL,                                                      "",                                                                                                                     NULL, 0, 0, 0 }
    };
    dupe_command_table(debugCommandTable, _debugCommandTable);

    static ChatCommand GMTicketCommandTable[] =
    {
        { "list",                   COMMAND_LEVEL_C, &GMWarden::HandleGMTicketListCommand,                       "Lists all active GM Tickets.",                                     NULL, 0, 0, 0 },
        { "get",                    COMMAND_LEVEL_C, &GMWarden::HandleGMTicketGetByIdCommand,                    "Gets GM Ticket with ID x.",                                        NULL, 0, 0, 0 },
        { "remove",                 COMMAND_LEVEL_C, &GMWarden::HandleGMTicketRemoveByIdCommand,                 "Removes GM Ticket with ID x.",                                     NULL, 0, 0, 0 },
        { "deletepermanent",        COMMAND_LEVEL_Z, &GMWarden::HandleGMTicketDeletePermanentCommand,            "Deletes GM Ticket with ID x permanently.",                         NULL, 0, 0, 0 },
        { "assign",                 COMMAND_LEVEL_C, &GMWarden::HandleGMTicketAssignToCommand,                   "Assigns GM Ticket with id x to GM y (if empty to your self).",     NULL, 0, 0, 0 },
        { "release",                COMMAND_LEVEL_C, &GMWarden::HandleGMTicketReleaseCommand,                    "Releases assigned GM Ticket with ID x.",                           NULL, 0, 0, 0 },
        { "comment",                COMMAND_LEVEL_C, &GMWarden::HandleGMTicketCommentCommand,                    "Sets comment x to GM Ticket with ID y.",                           NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                                          "",                                                                 NULL, 0, 0, 0 }
    };
    dupe_command_table(GMTicketCommandTable, _GMTicketCommandTable);

    static ChatCommand GuildCommandTable[] =
    {
        { "create",                 COMMAND_LEVEL_M, &GMWarden::CreateGuildCommand,              "Creates a guild.",                                         NULL, 0, 0, 0 },
        { "removeplayer",           COMMAND_LEVEL_M, &GMWarden::HandleGuildRemovePlayerCommand,  "Removes the target from its guild.",                       NULL, 0, 0, 0 },
        { "disband",                COMMAND_LEVEL_M, &GMWarden::HandleGuildDisbandCommand,       "Disbands the guild of your target.",                       NULL, 0, 0, 0 },
        { "modlevel",               COMMAND_LEVEL_M, &GMWarden::HandleGuildModifyLevelCommand,   "Modifies target player's guild's level.",                  NULL, 0, 0, 0 },
        { "gainxp",                 COMMAND_LEVEL_M, &GMWarden::HandleGuildGainXPCommand,        "Adds XP to target player's guild.",                        NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                         NULL, 0, 0, 0 }
    };
    dupe_command_table(GuildCommandTable, _GuildCommandTable);

    static ChatCommand TitleCommandTable[] =
    {
        { "add",                    COMMAND_LEVEL_M, &GMWarden::HandleAddTitleCommand,           "<TitleNumber> - Adds known title to the selected player",          NULL, 0, 0, 0 },
        { "remove",                 COMMAND_LEVEL_M, &GMWarden::HandleRemoveTitleCommand,        "<TitleNumber> - Removes known title from the selected player",     NULL, 0, 0, 0 },
        { "known",                  COMMAND_LEVEL_M, &GMWarden::HandleGetKnownTitlesCommand,     "Shows all titles known by the player",                             NULL, 0, 0, 0 },
        { "setchosen",              COMMAND_LEVEL_M, &GMWarden::HandleSetChosenTitleCommand,     "<TitleNumber> - Sets chosen title for the selected player",        NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                                 NULL, 0, 0, 0 }
    };
    dupe_command_table(TitleCommandTable, _TitleCommandTable);

    static ChatCommand GameObjectCommandTable[] =
    {
        { "select",                 COMMAND_LEVEL_O, &GMWarden::HandleGOSelect,          "Selects the nearest GameObject to you",        NULL, 0,                    0, 0 },
        { "delete",                 COMMAND_LEVEL_O, &GMWarden::HandleGODelete,          "Deletes selected GameObject",                  NULL, 0,                    0, 0 },
        { "spawn",                  COMMAND_LEVEL_O, &GMWarden::HandleGOSpawn,           "Spawns a GameObject by ID",                    NULL, 0,                    0, 0 },
        { "info",                   COMMAND_LEVEL_O, &GMWarden::HandleGOInfo,            "Gives you informations about selected GO",     NULL, 0,                    0, 0 },
        { "activate",               COMMAND_LEVEL_O, &GMWarden::HandleGOActivate,        "Activates/Opens the selected GO.",             NULL, 0,                    0, 0 },
        { "enable",                 COMMAND_LEVEL_O, &GMWarden::HandleGOEnable,          "Enables the selected GO for use.",             NULL, 0,                    0, 0 },
        { "scale",                  COMMAND_LEVEL_O, &GMWarden::HandleGOScale,           "Sets scale of selected GO",                    NULL, 0,                    0, 0 },
        { "animprogress",           COMMAND_LEVEL_O, &GMWarden::HandleGOAnimProgress,    "Sets anim progress",                           NULL, 0,                    0, 0 },
        { "move",                   COMMAND_LEVEL_G, &GMWarden::HandleGOMove,            "Moves gameobject to player xyz",               NULL, 0,                    0, 0 },
        { "rotate",                 COMMAND_LEVEL_G, &GMWarden::HandleGORotate,          "Rotates gameobject x degrees",                 NULL, 0,                    0, 0 },
        { "damage",                 COMMAND_LEVEL_G, &GMWarden::HandleDebugGoDamage,     "Damages the gameobject for <args>",            NULL, 0,                    0, 0 },
        { "rebuild",                COMMAND_LEVEL_G, &GMWarden::HandleDebugGoRepair,     "Resets the gameobject health and state",       NULL, 0,                    0, 0 },
        { "f_flags",                COMMAND_LEVEL_G, NULL,                                  "Flags",                                        NULL, GAMEOBJECT_FLAGS,     0, 3 },
        { "f_dynflags",             COMMAND_LEVEL_G, NULL,                                  "Dynflags",                                     NULL, GAMEOBJECT_DYNAMIC,   0, 3 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                  "",                                             NULL, 0,                    0, 0 }
    };
    dupe_command_table(GameObjectCommandTable, _GameObjectCommandTable);

    static ChatCommand BattlegroundCommandTable[] =
    {
        { "setbgscore",             COMMAND_LEVEL_E, &GMWarden::HandleSetBGScoreCommand,                 "<Teamid> <Score> - Sets battleground score. 2 Arguments. ",        NULL, 0, 0, 0 },
        { "startbg",                COMMAND_LEVEL_E, &GMWarden::HandleStartBGCommand,                    "Starts current battleground match.",                               NULL, 0, 0, 0 },
        { "pausebg",                COMMAND_LEVEL_E, &GMWarden::HandlePauseBGCommand,                    "Pauses current battleground match.",                               NULL, 0, 0, 0 },
        { "bginfo",                 COMMAND_LEVEL_E, &GMWarden::HandleBGInfoCommnad,                     "Displays information about current battleground.",                 NULL, 0, 0, 0 },
        { "setworldstate",          COMMAND_LEVEL_E, &GMWarden::HandleSetWorldStateCommand,              "<var> <val> - Var can be in hex. WS Value.",                       NULL, 0, 0, 0 },
        { "playsound",              COMMAND_LEVEL_E, &GMWarden::HandlePlaySoundCommand,                  "<val>. Val can be in hex.",                                        NULL, 0, 0, 0 },
        { "setbfstatus",            COMMAND_LEVEL_E, &GMWarden::HandleSetBattlefieldStatusCommand,       ".setbfstatus - NYI.",                                              NULL, 0, 0, 0 },
        { "leave",                  COMMAND_LEVEL_E, &GMWarden::HandleBattlegroundExitCommand,           "Leaves the current battleground.",                                 NULL, 0, 0, 0 },
        { "forcestart",             COMMAND_LEVEL_E, &GMWarden::HandleBattlegroundForcestartCommand,     "Forcestart current battlegrounds.",                                NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                                  "",                                                                 NULL, 0, 0, 0 }
    };
    dupe_command_table(BattlegroundCommandTable, _BattlegroundCommandTable);

    static ChatCommand NPCCommandTable[] =
    {
        { "vendorclear",            COMMAND_LEVEL_Z, &GMWarden::HandleVendorClearCommand,        "Clears all items from the vendor",                                                                                                             NULL, 0, 0, 0 },
        { "vendorsetadd",           COMMAND_LEVEL_Z, &GMWarden::HandleItemSetCommand,            "Adds item set to vendor",                                                                                                                      NULL, 0, 0, 0 },
        { "vendorsetremove",        COMMAND_LEVEL_Z, &GMWarden::HandleItemSetRemoveCommand,      "Removes item set from vendor",                                                                                                                 NULL, 0, 0, 0 },
        { "vendoradditem",          COMMAND_LEVEL_Z, &GMWarden::HandleItemCommand,               "Adds to vendor",                                                                                                                               NULL, 0, 0, 0 },
        { "vendorremoveitem",       COMMAND_LEVEL_Z, &GMWarden::HandleItemRemoveCommand,         "Removes from vendor.",                                                                                                                         NULL, 0, 0, 0 },
        { "flags",                  COMMAND_LEVEL_N, &GMWarden::HandleNPCFlagCommand,            "Changes NPC flags",                                                                                                                            NULL, 0, 0, 0 },
        { "emote",                  COMMAND_LEVEL_N, &GMWarden::HandleEmoteCommand,              ".emote - Sets emote state",                                                                                                                    NULL, 0, 0, 0 },
        { "setstandstate",          COMMAND_LEVEL_N, &GMWarden::HandleStandStateCommand,         ".setstandstate - Sets stand state",                                                                                                            NULL, 0, 0, 0 },
        { "delete",                 COMMAND_LEVEL_N, &GMWarden::HandleDeleteCommand,             "Deletes mobs from db and world.",                                                                                                              NULL, 0, 0, 0 },
        { "info",                   COMMAND_LEVEL_N, &GMWarden::HandleNpcInfoCommand,            "Displays NPC information",                                                                                                                     NULL, 0, 0, 0 },
        { "say",                    COMMAND_LEVEL_N, &GMWarden::HandleMonsterSayCommand,         ".npc say <text> - Makes selected mob say text <text>.",                                                                                        NULL, 0, 0, 0 },
        { "yell",                   COMMAND_LEVEL_N, &GMWarden::HandleMonsterYellCommand,        ".npc yell <Text> - Makes selected mob yell text <text>.",                                                                                      NULL, 0, 0, 0 },
        { "come",                   COMMAND_LEVEL_N, &GMWarden::HandleNpcComeCommand,            ".npc come - Makes npc move to your position",                                                                                                  NULL, 0, 0, 0 },
        { "jumpto",                 COMMAND_LEVEL_N, &GMWarden::HandleNpcJumpCommand,            ".npc come - Makes npc move to your position",                                                                                                  NULL, 0, 0, 0 },
        { "return",                 COMMAND_LEVEL_N, &GMWarden::HandleNpcReturnCommand,          ".npc return - Returns npc to spawnpoint.",                                                                                                     NULL, 0, 0, 0 },
        { "respawn",                COMMAND_LEVEL_N, &GMWarden::HandleCreatureRespawnCommand,    ".respawn - Respawns a dead npc from its corpse.",                                                                                              NULL, 0, 0, 0 },
        { "spawn",                  COMMAND_LEVEL_N, &GMWarden::HandleCreatureSpawnCommand,      ".npc spawn <id> <save> - Spawns npc of entry <id>",                                                                                            NULL, 0, 0, 0 },
        { "spawnlink",              COMMAND_LEVEL_N, &GMWarden::HandleNpcSpawnLinkCommand,       ".spawnlink sqlentry",                                                                                                                          NULL, 0, 0, 0 },
        { "possess",                COMMAND_LEVEL_N, &GMWarden::HandleNpcPossessCommand,         ".npc possess - Possess an npc (mind control)",                                                                                                 NULL, 0, 0, 0 },
        { "unpossess",              COMMAND_LEVEL_N, &GMWarden::HandleNpcUnPossessCommand,       ".npc unpossess - Unposses any currently possessed npc.",                                                                                       NULL, 0, 0, 0 },
        { "cast",                   COMMAND_LEVEL_D, &GMWarden::HandleMonsterCastCommand,        ".npc cast <spellId> - Makes selected mob cast the specified spell on you.",                                                                    NULL, 0, 0, 0 },
        { "equip",                  COMMAND_LEVEL_A, &GMWarden::HandleNPCEquipCommand,           "Use: .npc equip <slot> <itemid> - use .npc equip <slot> 0 to remove the item",                                                                 NULL, 0, 0, 0 },
        { "setongameobject",        COMMAND_LEVEL_A, &GMWarden::HandleNPCSetOnObjectCommand,     "Updates spawn information so that the creature does not fall through objects when spawning into world.",                                       NULL, 0, 0, 0 },
        { "save",                   COMMAND_LEVEL_Z, &GMWarden::HandleNPCSaveCommand,            "",                                                                                                                                             NULL, 0, 0, 0 },
        { "setvendormask",          COMMAND_LEVEL_Z, &GMWarden::HandleNPCSetVendorMaskCommand,   "",                                                                                                                                             NULL, 0, 0, 0 },
        { "getspeed",               COMMAND_LEVEL_Z, &GMWarden::HandleNPCGetSpeed,               "",                                                                                                                                             NULL, 0, 0, 0 },
        { "getresist",              COMMAND_LEVEL_Z, &GMWarden::HandleNPCGetResist,              "",                                                                                                                                             NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                                                                                                             NULL, 0, 0, 0 }
    };
    dupe_command_table(NPCCommandTable, _NPCCommandTable);

    static ChatCommand GMCommandTable[] =
    {
        { "list",                   COMMAND_LEVEL_1, &GMWarden::HandleGMListCommand,         "Shows active GM's",                                    NULL,                   0, 0, 0 },
        { "off",                    COMMAND_LEVEL_T, &GMWarden::HandleGMOffCommand,          "Sets GM tag off",                                      NULL,                   0, 0, 0 },
        { "on",                     COMMAND_LEVEL_T, &GMWarden::HandleGMOnCommand,           "Sets GM tag on",                                       NULL,                   0, 0, 0 },
        { "announce",               COMMAND_LEVEL_U, &GMWarden::HandleGMAnnounceCommand,     "Announce to GM's and Admin's",                         NULL,                   0, 0, 0 },
        { "allowwhispers",          COMMAND_LEVEL_C, &GMWarden::HandleAllowWhispersCommand,  "Allows whispers from player <s> while in gmon mode.",  NULL,                   0, 0, 0 },
        { "blockwhispers",          COMMAND_LEVEL_C, &GMWarden::HandleBlockWhispersCommand,  "Blocks whispers from player <s> while in gmon mode.",  NULL,                   0, 0, 0 },
        { "ticket",                 COMMAND_LEVEL_C, NULL,                                      "",                                                     GMTicketCommandTable,   0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                      "",                                                     NULL,                   0, 0, 0 }
    };
    dupe_command_table(GMCommandTable, _gamemasterCommandTable);

    static ChatCommand trackerCommandTable[] =
    {
        { "add",                    COMMAND_LEVEL_Z, &GMWarden::HandleAddTrackerCommand,         "Attaches a tracker to a GM's IP allowing them to be followed across accounts.",    NULL, 0, 0, 0 },
        { "del",                    COMMAND_LEVEL_Z, &GMWarden::HandleDelTrackerCommand,         "Removes a tracker from a GM's IP, syntax: .tracker del <trackerId>",               NULL, 0, 0, 0 },
        { "list",                   COMMAND_LEVEL_Z, &GMWarden::HandleTrackerListCommand,        "Checks if there are any accounts connected from the tracked GMs' IP.",             NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                                                 NULL, 0, 0, 0 }
    };
    dupe_command_table(trackerCommandTable, _trackerCommandTable);

    static ChatCommand AdminCommandTable[] =
    {
        { "announce",               COMMAND_LEVEL_Z, &GMWarden::HandleAdminAnnounceCommand,      "Command to Announce to Admin's",               NULL, 0, 0, 0 },
        { "castall",                COMMAND_LEVEL_Z, &GMWarden::HandleCastAllCommand,            "Makes all players online cast spell <x>.",     NULL, 0, 0, 0 },
        { "playall",                COMMAND_LEVEL_Z, &GMWarden::HandleGlobalPlaySoundCommand,    "Plays a sound to the entire server.",          NULL, 0, 0, 0 },
        { "saveall",                COMMAND_LEVEL_Z, &GMWarden::HandleSaveAllCommand,            "Save's all playing characters",                NULL, 0, 0, 0 },
        { "rehash",                 COMMAND_LEVEL_Z, &GMWarden::HandleRehashCommand,             "Reloads config file.",                         NULL, 0, 0, 0 },
        { "enableauctionhouse",     COMMAND_LEVEL_Z, &GMWarden::HandleEnableAH,                  "Enables Auctionhouse",                         NULL, 0, 0, 0 },
        { "disableauctionhouse",    COMMAND_LEVEL_Z, &GMWarden::HandleDisableAH,                 "Disables Auctionhouse",                        NULL, 0, 0, 0 },
        { "masssummon",             COMMAND_LEVEL_Z, &GMWarden::HandleMassSummonCommand,         "Summons all players.",                         NULL, 0, 0, 0 },
        { "restart",                COMMAND_LEVEL_Z, &GMWarden::HandleRestartCommand,            "Initiates server restart in <x> seconds.",     NULL, 0, 0, 0 },
        { "shutdown",               COMMAND_LEVEL_Z, &GMWarden::HandleShutdownCommand,           "Initiates server shutdown in <x> seconds.",    NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                             NULL, 0, 0, 0 }
    };
    dupe_command_table(AdminCommandTable, _administratorCommandTable);

    static ChatCommand CheatCommandTable[] =
    {
        { "status",                 COMMAND_LEVEL_M, &GMWarden::HandleShowCheatsCommand,             "Shows active cheats.",                             NULL, 0, 0, 0 },
        { "taxi",                   COMMAND_LEVEL_M, &GMWarden::HandleTaxiCheatCommand,              "Enables all taxi nodes.",                          NULL, 0, 0, 0 },
        { "cooldown",               COMMAND_LEVEL_M, &GMWarden::HandleCooldownCheatCommand,          "Enables no cooldown cheat.",                       NULL, 0, 0, 0 },
        { "casttime",               COMMAND_LEVEL_M, &GMWarden::HandleCastTimeCheatCommand,          "Enables no cast time cheat.",                      NULL, 0, 0, 0 },
        { "power",                  COMMAND_LEVEL_M, &GMWarden::HandlePowerCheatCommand,             "Disables mana consumption etc.",                   NULL, 0, 0, 0 },
        { "fly",                    COMMAND_LEVEL_M, &GMWarden::HandleFlyCommand,                    "Sets fly mode",                                    NULL, 0, 0, 0 },
        { "explore",                COMMAND_LEVEL_M, &GMWarden::HandleExploreCheatCommand,           "Reveals the unexplored parts of the map.",         NULL, 0, 0, 0 },
        { "stack",                  COMMAND_LEVEL_M, &GMWarden::HandleStackCheatCommand,             "Enables aura stacking cheat.",                     NULL, 0, 0, 0 },
        { "triggerpass",            COMMAND_LEVEL_M, &GMWarden::HandleTriggerpassCheatCommand,       "Ignores area trigger prerequisites.",              NULL, 0, 0, 0 },
        { "vendorpass",             COMMAND_LEVEL_M, &GMWarden::HandleVendorPassCheatCommand,        "Ignores npc vendor prerequisites.",                NULL, 0, 0, 0 },
        { "itempass",               COMMAND_LEVEL_M, &GMWarden::HandleItemReqCheatCommand,           "Ignores item requirements(incomplete).",           NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                              "",                                                 NULL, 0, 0, 0 },
    };
    dupe_command_table(CheatCommandTable, _CheatCommandTable);

    static ChatCommand accountCommandTable[] =
    {
        { "ban",                    COMMAND_LEVEL_A, &GMWarden::HandleAccountBannedCommand,      "Ban account. .account ban name timeperiod reason",     NULL, 0, 0, 0 },
        { "unban",                  COMMAND_LEVEL_Z, &GMWarden::HandleAccountUnbanCommand,       "Unbans account x.",                                    NULL, 0, 0, 0 },
        { "level",                  COMMAND_LEVEL_Z, &GMWarden::HandleAccountLevelCommand,       "Sets gm level on account. <username><gm_lvl>.",        NULL, 0, 0, 0 },
        { "mute",                   COMMAND_LEVEL_A, &GMWarden::HandleAccountMuteCommand,        "Mutes account for <timeperiod>.",                      NULL, 0, 0, 0 },
        { "unmute",                 COMMAND_LEVEL_A, &GMWarden::HandleAccountUnmuteCommand,      "Unmutes account <x>",                                  NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                     NULL, 0, 0, 0 },
    };
    dupe_command_table(accountCommandTable, _accountCommandTable);

    static ChatCommand honorCommandTable[] =
    {
        { NULL,                     COMMAND_LEVEL_0, NULL,                                                      "",                                                         NULL, 0, 0, 0 },
    };
    dupe_command_table(honorCommandTable, _honorCommandTable);

    static ChatCommand lookupCommandTable[] =
    {
        { "item",                   COMMAND_LEVEL_L, &GMWarden::HandleLookupItemCommand,         "Looks up item string x.",                  NULL, 0, 0, 0 },
        { "itemset",                COMMAND_LEVEL_L, &GMWarden::HandleLookupItemSetCommand,      "Looks up item set string x.",              NULL, 0, 0, 0 },
        { "quest",                  COMMAND_LEVEL_L, &GMWarden::HandleQuestLookupCommand,        "Looks up quest string x.",                 NULL, 0, 0, 0 },
        { "creature",               COMMAND_LEVEL_L, &GMWarden::HandleLookupCreatureCommand,     "Looks up item string x.",                  NULL, 0, 0, 0 },
        { "object",                 COMMAND_LEVEL_L, &GMWarden::HandleLookupObjectCommand,       "Looks up object string x.",                NULL, 0, 0, 0 },
        { "spell",                  COMMAND_LEVEL_L, &GMWarden::HandleLookupSpellCommand,        "Looks up spell string x.",                 NULL, 0, 0, 0 },
        { "spellspecific",          COMMAND_LEVEL_L, &GMWarden::HandleLookupSpellSpecificCommand,"Looks up spell string x.",                 NULL, 0, 0, 0 },
        { "spellname",              COMMAND_LEVEL_L, &GMWarden::HandleLookupSpellNameCommand,    "Looks up spell id x.",                     NULL, 0, 0, 0 },
        { "title",                  COMMAND_LEVEL_L, &GMWarden::HandleLookupTitleCommand,        "Looks up title based on entered name.",    NULL, 0, 0, 0 },
        { "currency",               COMMAND_LEVEL_L, &GMWarden::HandleLookupCurrencyCommand,     "Looks up currency based on entered name.", NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                         NULL, 0, 0, 0 },
    };
    dupe_command_table(lookupCommandTable, _lookupCommandTable);

    static ChatCommand questCommandTable[] =
    {
        { "addboth",                COMMAND_LEVEL_2, &GMWarden::HandleQuestAddBothCommand,       "Add quest <id> to the targeted NPC as start & finish",         NULL, 0, 0, 0 },
        { "addfinish",              COMMAND_LEVEL_2, &GMWarden::HandleQuestAddFinishCommand,     "Add quest <id> to the targeted NPC as finisher",               NULL, 0, 0, 0 },
        { "addstart",               COMMAND_LEVEL_2, &GMWarden::HandleQuestAddStartCommand,      "Add quest <id> to the targeted NPC as starter",                NULL, 0, 0, 0 },
        { "delboth",                COMMAND_LEVEL_2, &GMWarden::HandleQuestDelBothCommand,       "Delete quest <id> from the targeted NPC as start & finish",    NULL, 0, 0, 0 },
        { "delfinish",              COMMAND_LEVEL_2, &GMWarden::HandleQuestDelFinishCommand,     "Delete quest <id> from the targeted NPC as finisher",          NULL, 0, 0, 0 },
        { "delstart",               COMMAND_LEVEL_2, &GMWarden::HandleQuestDelStartCommand,      "Delete quest <id> from the targeted NPC as starter",           NULL, 0, 0, 0 },
        { "complete",               COMMAND_LEVEL_2, &GMWarden::HandleQuestFinishCommand,        "Complete/Finish quest <id>",                                   NULL, 0, 0, 0 },
        { "finisher",               COMMAND_LEVEL_2, &GMWarden::HandleQuestFinisherCommand,      "Lookup quest finisher for quest <id>",                         NULL, 0, 0, 0 },
        { "item",                   COMMAND_LEVEL_2, &GMWarden::HandleQuestItemCommand,          "Lookup itemid necessary for quest <id>",                       NULL, 0, 0, 0 },
        { "liststart",              COMMAND_LEVEL_2, &GMWarden::HandleQuestListStarterCommand,   "Lists the quests for the npc <id>",                            NULL, 0, 0, 0 },
        { "listfinish",             COMMAND_LEVEL_2, &GMWarden::HandleQuestListFinisherCommand,  "Lists the quests for the npc <id>",                            NULL, 0, 0, 0 },
        { "load",                   COMMAND_LEVEL_2, &GMWarden::HandleQuestLoadCommand,          "Loads quests from database",                                   NULL, 0, 0, 0 },
        { "lookup",                 COMMAND_LEVEL_2, &GMWarden::HandleQuestLookupCommand,        "Looks up quest string x",                                      NULL, 0, 0, 0 },
        { "giver",                  COMMAND_LEVEL_2, &GMWarden::HandleQuestGiverCommand,         "Lookup quest giver for quest <id>",                            NULL, 0, 0, 0 },
        { "remove",                 COMMAND_LEVEL_2, &GMWarden::HandleQuestRemoveCommand,        "Removes the quest <id> from the targeted player",              NULL, 0, 0, 0 },
        { "reward",                 COMMAND_LEVEL_2, &GMWarden::HandleQuestRewardCommand,        "Shows reward for quest <id>",                                  NULL, 0, 0, 0 },
        { "status",                 COMMAND_LEVEL_2, &GMWarden::HandleQuestStatusCommand,        "Lists the status of quest <id>",                               NULL, 0, 0, 0 },
        { "spawn",                  COMMAND_LEVEL_2, &GMWarden::HandleQuestSpawnCommand,         "Port to spawn location for quest <id>",                        NULL, 0, 0, 0 },
        { "start",                  COMMAND_LEVEL_2, &GMWarden::HandleQuestStartCommand,         "Starts quest <id>",                                            NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                             NULL, 0, 0, 0 },
    };
    dupe_command_table(questCommandTable, _questCommandTable);

    static ChatCommand FactionCommandTable[] =
    {
        { "modstanding",            COMMAND_LEVEL_Z, &GMWarden::HandleFactionModStanding,    "Mods a player's faction's standing based on a value",  NULL, 0, 0, 0 },
        { "setstanding",            COMMAND_LEVEL_Z, &GMWarden::HandleFactionSetStanding,    "Changes a player's faction's standing to a value",     NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                      "",                                                     NULL, 0, 0, 0 },
    };
    dupe_command_table(FactionCommandTable, _FactionCommandTable);

    static ChatCommand commandTable[] = {
        { "commands",               COMMAND_LEVEL_0, &GMWarden::HandleCommandsCommand,                           "Shows Commands",               NULL, 0, 0, 0 },
        { "help",                   COMMAND_LEVEL_0, &GMWarden::HandleHelpCommand,                               "Shows help for command",       NULL, 0, 0, 0 },
        { "announce",               COMMAND_LEVEL_U, &GMWarden::HandleAnnounceCommand,                           "Sends Msg To All",             NULL, 0, 0, 0 },
        { "wannounce",              COMMAND_LEVEL_U, &GMWarden::HandleWAnnounceCommand,                          "Sends Widescreen Msg To All",  NULL, 0, 0, 0 },
        { "appear",                 COMMAND_LEVEL_V, &GMWarden::HandleAppearCommand,                             "Teleports to x's position.",   NULL, 0, 0, 0 },
        { "summon",                 COMMAND_LEVEL_V, &GMWarden::HandleSummonCommand,                             "Summons x to your position",   NULL, 0, 0, 0 },
        { "teleport",               COMMAND_LEVEL_V, &GMWarden::HandleTeleportCommand,                           "Teleports to location by name", NULL, 0, 0, 0 },
        { "teleportxyz",            COMMAND_LEVEL_V, &GMWarden::HandleTeleportXYZCommand,                        "Teleports to u|xyz location",   NULL, 0, 0, 0 },
        { "banchar",                COMMAND_LEVEL_B, &GMWarden::HandleBanCharacterCommand,                       "Bans character x with or without reason",              NULL, 0, 0, 0 },
        { "unbanchar",              COMMAND_LEVEL_B, &GMWarden::HandleUnBanCharacterCommand,                     "Unbans character x",           NULL, 0, 0, 0 },
        { "kick",                   COMMAND_LEVEL_B, &GMWarden::HandleKickCommand,                               "Kicks player from server",     NULL, 0, 0, 0 },
        { "kill",                   COMMAND_LEVEL_R, &GMWarden::HandleKillCommand,                               ".kill - Kills selected unit.", NULL, 0, 0, 0 },
        { "killplr" ,               COMMAND_LEVEL_R, &GMWarden::HandleKillByPlrCommand,                          ".killplr <name> - Kills specified player" , NULL , 0 , 0 , 0 },
        { "revive",                 COMMAND_LEVEL_R, &GMWarden::HandleReviveCommand,                             "Revives you.",                 NULL, 0, 0, 0 },
        { "reviveplr",              COMMAND_LEVEL_R, &GMWarden::HandleReviveStringcommand,                       "Revives player specified.",        NULL, 0, 0, 0 },
        { "demorph",                COMMAND_LEVEL_M, &GMWarden::HandleDeMorphCommand,                            "Demorphs from morphed model.", NULL, 0, 0, 0 },
        { "mount",                  COMMAND_LEVEL_M, &GMWarden::HandleMountCommand,                              "Mounts into modelid x.",       NULL, 0, 0, 0 },
        { "dismount",               COMMAND_LEVEL_M, &GMWarden::HandleDismountCommand,                           "Dismounts.",                   NULL, 0, 0, 0 },
        { "fulldismount",           COMMAND_LEVEL_M, &GMWarden::HandleFullDismountCommand,                       "Force a player to full dismount (taxi)",   NULL, 0, 0, 0 },
        { "gps",                    COMMAND_LEVEL_0, &GMWarden::HandleGPSCommand,                                "Shows Position",               NULL, 0, 0, 0 },
        { "info",                   COMMAND_LEVEL_0, &GMWarden::HandleInfoCommand,                               "Server info",                  NULL, 0, 0, 0 },
        { "worldport",              COMMAND_LEVEL_V, &GMWarden::HandleWorldPortCommand,                          "",                             NULL, 0, 0, 0 },
        { "save",                   COMMAND_LEVEL_S, &GMWarden::HandleSaveCommand,                               "Save's your character",            NULL, 0, 0, 0 },
        { "start",                  COMMAND_LEVEL_M, &GMWarden::HandleStartCommand,                              "Teleport's you to a starting location",                                NULL, 0, 0, 0 },
        { "iteminfo",               COMMAND_LEVEL_M, &GMWarden::HandleItemInfoCommand,                           "",                             NULL, 0, 0, 0 },
        { "additem",                COMMAND_LEVEL_M, &GMWarden::HandleAddInvItemCommand,                         "",                             NULL, 0, 0, 0 },
        { "additemset",             COMMAND_LEVEL_M, &GMWarden::HandleAddItemSetCommand,                         "Adds item set to inv.",            NULL, 0, 0, 0 },
        { "removeitem",             COMMAND_LEVEL_M, &GMWarden::HandleRemoveItemCommand,                         "Removes item %u count %u.", NULL, 0, 0, 0 },
        { "resetreputation",        COMMAND_LEVEL_N, &GMWarden::HandleResetReputationCommand,                    ".resetreputation - Resets reputation to start levels. (use on characters that were made before reputation fixes.)", NULL, 0, 0, 0 },
        { "resetspells",            COMMAND_LEVEL_N, &GMWarden::HandleResetSpellsCommand,                        ".resetspells - Resets all spells to starting spells of targeted player. DANGEROUS.", NULL, 0, 0, 0 },
        { "resettalents",           COMMAND_LEVEL_N, &GMWarden::HandleResetTalentsCommand,                       ".resettalents - Resets all talents of targeted player to that of their current level. DANGEROUS.", NULL, 0, 0, 0 },
        { "resetskills",            COMMAND_LEVEL_N, &GMWarden::HandleResetSkillsCommand ,                       ".resetskills - Resets all skills.", NULL, 0, 0, 0 },
        { "learn",                  COMMAND_LEVEL_M, &GMWarden::HandleLearnCommand,                              "Learns spell",                 NULL, 0, 0, 0 },
        { "unlearn",                COMMAND_LEVEL_M, &GMWarden::HandleUnlearnCommand,                            "Unlearns spell",               NULL, 0, 0, 0 },
        { "getskilllevel",          COMMAND_LEVEL_M, &GMWarden::HandleGetSkillLevelCommand,                      "Gets the current level of a skill",NULL,0,0,0 }, //DGM (maybe add to playerinfo?)
        { "getskillinfo",           COMMAND_LEVEL_M, &GMWarden::HandleGetSkillsInfoCommand,                      "Gets all the skills from a player",NULL,0,0,0 },
        { "learnskill",             COMMAND_LEVEL_M, &GMWarden::HandleLearnSkillCommand,                         ".learnskill <skillid> (optional) <value> <maxvalue> - Learns skill id skillid.", NULL, 0, 0, 0 },
        { "advanceskill",           COMMAND_LEVEL_M, &GMWarden::HandleModifySkillCommand,                        "advanceskill <skillid> <amount, optional, default = 1> - Advances skill line x times..", NULL, 0, 0, 0 },
        { "removeskill",            COMMAND_LEVEL_M, &GMWarden::HandleRemoveSkillCommand,                        ".removeskill <skillid> - Removes skill",       NULL, 0, 0, 0 },
        { "increaseweaponskill",    COMMAND_LEVEL_M, &GMWarden::HandleIncreaseWeaponSkill,                       ".increaseweaponskill <count> - Increase eqipped weapon skill x times (defaults to 1).", NULL, 0, 0, 0 },
        { "earnachievement",        COMMAND_LEVEL_M, &GMWarden::HandleEarnAchievement,                           "Earns achievement for the target player", NULL, 0, 0, 0 },
        { "addcurrency",            COMMAND_LEVEL_M, &GMWarden::HandleAddCurrency,                               "(syntax: type/amount) Adds Y amount of currency X to target player", NULL, 0, 0, 0 },
        { "removecurrency",         COMMAND_LEVEL_M, &GMWarden::HandleRemoveCurrency,                            "(syntax: type/amount) Removes Y amount of currency X to target player", NULL, 0, 0, 0 },
        { "playerinfo",             COMMAND_LEVEL_M, &GMWarden::HandlePlayerInfo,                                ".playerinfo - Displays informations about the selected character (account...)", NULL, 0, 0, 0 },

        { "modify",                 COMMAND_LEVEL_M, NULL,                                                          "",                 modifyCommandTable, 0, 0, 0 },
        { "debug",                  COMMAND_LEVEL_D, NULL,                                                          "",                 debugCommandTable, 0, 0, 0 },
        { "gobject",                COMMAND_LEVEL_O, NULL,                                                          "",                 GameObjectCommandTable, 0, 0, 0 },
        { "battleground",           COMMAND_LEVEL_E, NULL,                                                          "",                 BattlegroundCommandTable, 0, 0, 0 },
        { "npc",                    COMMAND_LEVEL_N, NULL,                                                          "",                 NPCCommandTable, 0, 0, 0 },
        { "gm",                     COMMAND_LEVEL_N, NULL,                                                          "",                 GMCommandTable, 0, 0, 0 },
        { "tracker",                COMMAND_LEVEL_N, NULL,                                                          "",                 trackerCommandTable, 0, 0, 0 },
        { "warn",                   COMMAND_LEVEL_0, NULL,                                                          "",                 warnCommandTable, 0, 0, 0 },
        { "admin",                  COMMAND_LEVEL_Z, NULL,                                                          "",                 AdminCommandTable, 0, 0, 0 },
        { "cheat",                  COMMAND_LEVEL_M, NULL,                                                          "",                 CheatCommandTable, 0, 0, 0 },
        { "account",                COMMAND_LEVEL_A, NULL,                                                          "",                 accountCommandTable, 0, 0, 0 },
        { "honor",                  COMMAND_LEVEL_M, NULL,                                                          "",                 honorCommandTable, 0, 0, 0 },
        { "quest",                  COMMAND_LEVEL_Q, NULL,                                                          "",                 questCommandTable, 0, 0, 0 },
        { "guild",                  COMMAND_LEVEL_M, NULL,                                                          "",                 GuildCommandTable, 0, 0, 0 },
        { "title",                  COMMAND_LEVEL_M, NULL,                                                          "",                 TitleCommandTable, 0, 0, 0 },
        { "lookup",                 COMMAND_LEVEL_0, NULL,                                                          "",                 lookupCommandTable, 0, 0, 0 },
        { "faction",                COMMAND_LEVEL_0, NULL,                                                          "",                 FactionCommandTable, 0, 0, 0 },

        { "tgravity",               COMMAND_LEVEL_D, &GMWarden::HandleTestGravityOpcode,                         "",                 NULL, 0, 0, 0 },
        { "getpos",                 COMMAND_LEVEL_D, &GMWarden::HandleGetPosCommand,                             "",                 NULL, 0, 0, 0 },
        { "clearcooldowns",         COMMAND_LEVEL_M, &GMWarden::HandleClearCooldownsCommand,                     "Clears all cooldowns for your class.", NULL, 0, 0, 0 },
        { "removeauras",            COMMAND_LEVEL_M, &GMWarden::HandleRemoveAurasCommand,                        "Removes all auras from target",    NULL, 0, 0, 0 },
        { "paralyze",               COMMAND_LEVEL_B, &GMWarden::HandleParalyzeCommand,                           "Roots/Paralyzes the target.",  NULL, 0, 0, 0 },
        { "unparalyze",             COMMAND_LEVEL_B, &GMWarden::HandleUnParalyzeCommand,                         "Unroots/Unparalyzes the target.",NULL, 0, 0, 0 },
        { "setmotd",                COMMAND_LEVEL_M, &GMWarden::HandleSetMotdCommand,                            "Sets MOTD",        NULL, 0, 0, 0 },
        { "reloadtable",            COMMAND_LEVEL_M, &GMWarden::HandleDBReloadCommand,                           "Reloads some of the database tables", NULL, 0, 0, 0 },
        { "advanceallskills",       COMMAND_LEVEL_M, &GMWarden::HandleAdvanceAllSkillsCommand,                   "Advances all skills <x> points.", NULL, 0, 0, 0 },
        { "killbyplayer",           COMMAND_LEVEL_F, &GMWarden::HandleKillByPlayerCommand,                       "Disconnects the player with name <s>.", NULL, 0, 0, 0 },
        { "killbyaccount",          COMMAND_LEVEL_F, &GMWarden::HandleKillBySessionCommand,                      "Disconnects the session with account name <s>.", NULL, 0, 0, 0 },
        { "castspell",              COMMAND_LEVEL_D, &GMWarden::HandleCastSpellCommand,                          ".castspell <spellid> - Casts spell on target.", NULL, 0, 0, 0 },
        { "modperiod",              COMMAND_LEVEL_M, &GMWarden::HandleModPeriodCommand,                          "Changes period of current transporter.", NULL, 0, 0, 0 },
        { "npcfollow",              COMMAND_LEVEL_M, &GMWarden::HandleNpcFollowCommand,                          "Sets npc to follow you", NULL, 0, 0, 0 },
        { "nullfollow",             COMMAND_LEVEL_M, &GMWarden::HandleNullFollowCommand,                         "Sets npc to not follow anything", NULL, 0, 0, 0 },
        { "addipban",               COMMAND_LEVEL_M, &GMWarden::HandleIPBanCommand,                              "Adds an address to the IP ban table: <address>/<mask> <duration> <reason>\n Mask represents a subnet mask, use /32 to ban a single ip.\nDuration should be a number followed by a character representing the calendar subdivision to use (h>hours, d>days, w>weeks, m>months, y>years, default minutes).", NULL, 0, 0, 0 },
        { "delipban",               COMMAND_LEVEL_M, &GMWarden::HandleIPUnBanCommand,                            "Deletes an address from the IP ban table: <address>", NULL, 0, 0, 0 },
        { "renamechar",             COMMAND_LEVEL_M, &GMWarden::HandleRenameCommand,                             "Renames character x to y.", NULL, 0, 0, 0 },
        { "forcerenamechar",        COMMAND_LEVEL_M, &GMWarden::HandleForceRenameCommand,                        "Forces character x to rename his char next login", NULL, 0, 0, 0 },
        { "recustomizechar",        COMMAND_LEVEL_M, &GMWarden::HandleRecustomizeCharCommand,                    "Flags character x for character recustomization", NULL, 0, 0, 0 },
        { "getstanding",            COMMAND_LEVEL_M, &GMWarden::HandleGetStandingCommand,                        "Gets standing of faction %u.", NULL, 0, 0, 0 },
        { "setstanding",            COMMAND_LEVEL_M, &GMWarden::HandleSetStandingCommand,                        "Sets stanging of faction %u.", NULL, 0, 0, 0 },
        { "createarenateam",        COMMAND_LEVEL_G, &GMWarden::HandleCreateArenaTeamCommands,                   "Creates arena team", NULL, 0, 0, 0 },
        { "whisperblock",           COMMAND_LEVEL_G, &GMWarden::HandleWhisperBlockCommand,                       "Blocks like .gmon except without the <GM> tag", NULL, 0, 0, 0 },
        { "logcomment",             COMMAND_LEVEL_1, &GMWarden::HandleGmLogCommentCommand,                       "Adds a comment to the GM log for the admins to read." , NULL , 0 , 0 , 0 },
        { "showitems",              COMMAND_LEVEL_M, &GMWarden::HandleShowItems,                                 "test for ItemIterator", NULL, 0, 0, 0 },
        { "testlos",                COMMAND_LEVEL_M, &GMWarden::HandleCollisionTestLOS,                          "tests los", NULL, 0, 0, 0 },
        { "testindoor",             COMMAND_LEVEL_M, &GMWarden::HandleCollisionTestIndoor,                       "tests indoor", NULL, 0, 0, 0 },
        { "getheight",              COMMAND_LEVEL_M, &GMWarden::HandleCollisionGetHeight,                        "Gets height", NULL, 0, 0, 0 },
        { "renameallinvalidchars",  COMMAND_LEVEL_Z, &GMWarden::HandleRenameAllCharacter,                        "Renames all invalid character names", NULL, 0,0, 0 },
        { "removesickness",         COMMAND_LEVEL_M, &GMWarden::HandleRemoveRessurectionSickessAuraCommand,      "Removes ressurrection sickness from the target", NULL, 0, 0, 0 },
        { "fixscale",               COMMAND_LEVEL_M, &GMWarden::HandleFixScaleCommand,                           "",                 NULL, 0, 0, 0 },
        { "clearcorpses",           COMMAND_LEVEL_M, &GMWarden::HandleClearCorpsesCommand,                       "",                 NULL, 0, 0, 0 },
        { "clearbones",             COMMAND_LEVEL_M, &GMWarden::HandleClearBonesCommand,                         "",                 NULL, 0, 0, 0 },
        { "multimute",              COMMAND_LEVEL_B, &GMWarden::HandleMultiMuteCommand,                          "mutes multiple , .multimute <reason> <player1> <player2> ...",         NULL, 0, 0, 0 },
        { "multiban",               COMMAND_LEVEL_B, &GMWarden::HandleMultiBanCommand,                           "bans multiple , .multimute <reason> <player1> <player2> ...",          NULL, 0, 0, 0 },
        { "multiaccountban",        COMMAND_LEVEL_B, &GMWarden::HandleMultiAccountBanCommand,                    "account bans multiple , .multimute <reason> <player1> <player2> ...",  NULL, 0, 0, 0 },
        { "multikick",              COMMAND_LEVEL_B, &GMWarden::HandleMultiKickCommand,                          "kicks multiple , .multimute <reason> <player1> <player2> ...",         NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                                          "",                 NULL, 0, 0, 0 }
    };
    dupe_command_table(commandTable, _commandTable);

    /* set the correct pointers */
    ChatCommand* p = &_commandTable[0];
    while (p->Name != 0)
    {
        if (p->ChildCommands != 0)
        {
            // Set the correct pointer.
            ChatCommand* np = GetSubCommandTable(p->Name);
            ASSERT(np);
            p->ChildCommands = np;
        }
        ++p;
    }
}

bool GMWarden::HandleRenameCommand(const char* args, WorldSession* m_session)
{
    // prevent buffer overflow
    if (strlen(args) > 100)
        return false;

    char name1[100], name2[100];
    if (sscanf(args, "%s %s", name1, name2) != 2)
        return false;

    if (!sWorld.VerifyName(name2, strlen(name2)))
    {
        sChatHandler.RedSystemMessage(m_session, "That name is invalid or contains invalid characters.");
        return true;
    }

    std::string new_name = name2;
    PlayerInfo* pi = objmgr.GetPlayerInfoByName(name1);
    if (pi == 0)
    {
        sChatHandler.RedSystemMessage(m_session, "Player not found with this name.");
        return true;
    }

    if (objmgr.GetPlayerInfoByName(new_name.c_str()) != NULL)
    {
        sChatHandler.RedSystemMessage(m_session, "Player found with this name in use already.");
        return true;
    }

    objmgr.RenamePlayerInfo(pi, pi->charName.c_str(), new_name.c_str());
    pi->charName = new_name;

    // look in world for him
    if (Player* plr = objmgr.GetPlayer(pi->charGuid))
    {
        plr->SetName(new_name);
        sChatHandler.BlueSystemMessageToPlr(plr, "%s changed your name to '%s'.", m_session->GetPlayer()->GetName(), new_name.c_str());
        plr->SaveToDB(false);
    }
    else CharacterDatabase.WaitExecute("UPDATE character_data SET name = '%s' WHERE guid = %u", CharacterDatabase.EscapeString(new_name).c_str(), pi->charGuid.getLow());

    sChatHandler.GreenSystemMessage(m_session, "Changed name of '%s' to '%s'.", (char*)name1, (char*)name2);
    sWorld.LogGM(m_session, "renamed character %s (GUID: %u) to %s", (char*)name1, pi->charGuid.getLow(), (char*)name2);
    sWorld.LogPlayer(m_session, "GM renamed character %s (GUID: %u) to %s", (char*)name1, pi->charGuid.getLow(), ((char*)name2));
    return true;
}

bool GMWarden::HandleRenameAllCharacter(const char* args, WorldSession* m_session)
{
    WoWGuid guid;
    uint32 uCount = 0;
    uint32 ts = getMSTime();
    QueryResult* result = CharacterDatabase.Query("SELECT guid, name FROM character_data");
    if (result)
    {
        do
        {
            guid = result->Fetch()[0].GetUInt64();
            const char* pName = result->Fetch()[1].GetString();
            size_t szLen = strlen(pName);

            if (!sWorld.VerifyName(pName, szLen))
            {
                sLog.printf("renaming character %s, %u\n", pName, guid.getLow());
                if (Player* pPlayer = objmgr.GetPlayer(guid))
                {
                    pPlayer->GetSession()->SystemMessage("Your character has had a force rename set, you will be prompted to rename your character at next login in conformance with server rules.");
                }

                CharacterDatabase.WaitExecute("UPDATE character_data SET customizeFlags = customizeFlags|0x01 WHERE guid = %u", guid);
                ++uCount;
            }

        } while (result->NextRow());
        delete result;
    }

    sChatHandler.SystemMessage(m_session, "Procedure completed in %u ms. %u character(s) forced to rename.", getMSTime() - ts, uCount);
    return true;
}

bool GMWarden::CmdSetValueField(WorldSession* m_session, uint32 field, uint32 fieldmax, const char* fieldname, const char* args)
{
    if (!args)
        return false;

    char* pvalue = strtok((char*)args, " ");
    uint32 mv, av;

    if (pvalue)
        av = atol(pvalue);
    else
        return false;

    if (fieldmax)
    {
        char* pvaluemax = strtok(NULL, " ");
        if (!pvaluemax)
            return false;
        else
            mv = atol(pvaluemax);
    }
    else mv = 0;

    //valid UNIT_FIELD?
    if (field <= OBJECT_END || field > UNIT_END)
    {
        sChatHandler.RedSystemMessage(m_session, "Specified field is not valid.");
        return true;
    }
    if (av < 0)
    {
        sChatHandler.RedSystemMessage(m_session, "Values are invalid. Value must be >= 0.");
        return true;
    }
    if (fieldmax && (mv < av || mv <= 0))
    {
        sChatHandler.RedSystemMessage(m_session, "Values are invalid. Max value must be >= new value.");
        return true;
    }

    Player* plr = getSelectedChar(m_session, false);
    if (plr != NULL)
    {
        sWorld.LogGM(m_session, "used modify field value: %s, %u on %s", fieldname, av, plr->GetName());
        if (fieldmax)
        {
            sChatHandler.BlueSystemMessage(m_session, "You set the %s of %s to %d/%d.", fieldname, plr->GetName(), av, mv);
            sChatHandler.GreenSystemMessageToPlr(plr, "%s set your %s to %d/%d.", m_session->GetPlayer()->GetName(), fieldname, av, mv);
        }
        else
        {
            sChatHandler.BlueSystemMessage(m_session, "You set the %s of %s to %d.", fieldname, plr->GetName(), av);
            sChatHandler.GreenSystemMessageToPlr(plr, "%s set your %s to %d.", m_session->GetPlayer()->GetName(), fieldname, av);
        }

        if (field == UNIT_FIELD_BASE_HEALTH)
            plr->SetUInt32Value(UNIT_FIELD_HEALTH, av);

        plr->SetUInt32Value(field, av);

        if (fieldmax)
            plr->SetUInt32Value(fieldmax, mv);
    }
    else
    {
        Creature* cr = getSelectedCreature(m_session, false);
        if (cr)
        {
            std::string creaturename = cr->GetName();

            sWorld.LogGM(m_session, "used modify field value: [creature]%s, %u on %s", fieldname, av, creaturename.c_str());

            switch (field)
            {
            case UNIT_FIELD_BASE_HEALTH:
            {
                cr->SetUInt32Value(UNIT_FIELD_HEALTH, av);
            }break;
            }

            cr->SetUInt32Value(field, av);

            if (fieldmax)
            {
                cr->SetUInt32Value(fieldmax, mv);
                sChatHandler.BlueSystemMessage(m_session, "Setting %s of %s to %u/%u.", fieldname, creaturename.c_str(), av, mv);
            }
            else
                sChatHandler.BlueSystemMessage(m_session, "Setting %s of %s to %u.", fieldname, creaturename.c_str(), av);

            if (m_session->CanUseCommand('z'))
                cr->SaveToDB();
        }
        else
        {
            sChatHandler.RedSystemMessage(m_session, "Invalid Selection.");
        }
    }
    return true;
}

bool GMWarden::CmdSetFloatField(WorldSession* m_session, uint32 field, uint32 fieldmax, const char* fieldname, const char* args)
{
    char* pvalue = strtok((char*)args, " ");
    float mv, av;

    if (!pvalue)
        return false;
    else
        av = (float)atof(pvalue);

    if (fieldmax)
    {
        char* pvaluemax = strtok(NULL, " ");
        if (!pvaluemax)
            return false;
        else
            mv = (float)atof(pvaluemax);
    }
    else
    {
        mv = 0;
    }

    if (av <= 0)
    {
        sChatHandler.RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
        return true;
    }
    if (fieldmax)
    {
        if (mv < av || mv <= 0)
        {
            sChatHandler.RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
            return true;
        }
    }

    Player* plr = getSelectedChar(m_session, false);
    if (plr != NULL)
    {
        sWorld.LogGM(m_session, "used modify field value: %s, %f on %s", fieldname, av, plr->GetName());

        if (fieldmax)
        {
            sChatHandler.BlueSystemMessage(m_session, "You set the %s of %s to %.1f/%.1f.", fieldname, plr->GetName(), av, mv);
            sChatHandler.GreenSystemMessageToPlr(plr, "%s set your %s to %.1f/%.1f.", m_session->GetPlayer()->GetName(), fieldname, av, mv);
        }
        else
        {
            sChatHandler.BlueSystemMessage(m_session, "You set the %s of %s to %.1f.", fieldname, plr->GetName(), av);
            sChatHandler.GreenSystemMessageToPlr(plr, "%s set your %s to %.1f.", m_session->GetPlayer()->GetName(), fieldname, av);
        }
        plr->SetFloatValue(field, av);
        if (fieldmax) plr->SetFloatValue(fieldmax, mv);
    }
    else
    {
        Creature* cr = getSelectedCreature(m_session, false);
        if (cr)
        {
            if (!(field < UNIT_END && fieldmax < UNIT_END)) return false;
            std::string creaturename = cr->GetName();
            if (fieldmax)
                sChatHandler.BlueSystemMessage(m_session, "Setting %s of %s to %.1f/%.1f.", fieldname, creaturename.c_str(), av, mv);
            else sChatHandler.BlueSystemMessage(m_session, "Setting %s of %s to %.1f.", fieldname, creaturename.c_str(), av);
            cr->SetFloatValue(field, av);
            sWorld.LogGM(m_session, "used modify field value: [creature]%s, %f on %s", fieldname, av, creaturename.c_str());
            if (fieldmax)
                cr->SetFloatValue(fieldmax, mv);
        }
        else
        {
            sChatHandler.RedSystemMessage(m_session, "Invalid Selection.");
        }
    }
    return true;
}

bool GMWarden::HandleTestGravityOpcode(const char* args, WorldSession* m_session)
{
    static std::vector<uint8> guidString;
    if (guidString.empty())
        for (uint8 i = 0; i < 8; i++)
            guidString.push_back(i);

    int val = atoi(args);
    char buff[15];
    std::string bitData;
    WoWGuid guid = m_session->GetPlayer()->GetGUID();
    WorldPacket data(SMSG_MOVE_GRAVITY_ENABLE);
    if (val == 1) data << uint32(0xFF);
    for (uint8 i = 0; i < 7; i++)
    {
        uint8 val = *guidString.begin();
        guidString.erase(guidString.begin());
        guidString.push_back(val);
        data.WriteBit(guid[val]);
        sprintf(buff, "%u: %u | ", i, val);
        bitData.append(buff);
    }

    sprintf(buff, "%u: %u", 7, *guidString.begin());
    bitData.append(buff);
    data.WriteBit(guid[*guidString.begin()]);
    data.FlushBits();

    if (val == 2) data << uint32(0xFF);
    data.append<uint8>(guid[0]);
    if (val == 3) data << uint32(0xFF);
    m_session->SendPacket(&data);
    sChatHandler.SystemMessage(m_session, bitData.c_str());
    return true;
}

bool GMWarden::HandleGetPosCommand(const char* args, WorldSession* m_session)
{
    Creature* creature = getSelectedCreature(m_session);
    if (!creature)
        return false;
    sChatHandler.BlueSystemMessage(m_session, "Creature Position: \nX: %f\nY: %f\nZ: %f\n", creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ());
    return true;
}

bool GMWarden::HandleDebugRetroactiveQuestAchievements(const char* args, WorldSession* m_session)
{
    Player* pTarget = getSelectedChar(m_session, true);
    if (!pTarget) return true;

    pTarget->RetroactiveCompleteQuests();
    m_session->GetPlayer()->BroadcastMessage("Done.");
    return true;
}

bool GMWarden::HandleModifyFactionCommand(const char* args, WorldSession* m_session)
{
    Player* player = m_session->GetPlayer();
    if (player == NULL)
        return true;

    Unit* unit = getSelectedUnit(m_session, false);

    if (unit == NULL)
        unit = player;

    uint32 faction = atol(args);
    if (!faction && unit->IsCreature())
        faction = castPtr<Creature>(unit)->GetCreatureData()->faction;

    sChatHandler.BlueSystemMessage(m_session, "Set target's faction to %u", faction);

    unit->SetFaction(faction, m_session->CanUseCommand('z'));
    return true;
}

bool GMWarden::HandleModifyScaleCommand(const char* args, WorldSession* m_session)
{
    Player* player = m_session->GetPlayer();
    if (player == NULL)
        return true;

    Unit* unit = getSelectedUnit(m_session, false);
    if (unit == NULL)
        unit = player;

    float scale = atof(args);
    int save = m_session->CanUseCommand('z') ? 1 : 0;
    if (sscanf(args, "%f %d", &scale, &save) < 1)
        return false;

    if (scale > 255 || scale < 0)
        return false;

    if (!scale && unit->IsCreature())
        scale = castPtr<Creature>(unit)->GetCreatureData()->scale;

    sChatHandler.BlueSystemMessage(m_session, "Set target's scale to %f", scale);
    unit->SetFloatValue(OBJECT_FIELD_SCALE_X, scale);
    if (unit->IsCreature() && (save > 0))
        castPtr<Creature>(unit)->SaveToDB();

    return true;
}

uint16 GMWarden::GetItemIDFromLink(const char* link, uint32* itemid)
{
    if (link == NULL)
    {
        *itemid = 0;
        return 0;
    }

    uint16 slen = (uint16)strlen(link);
    const char* ptr = strstr(link, "|Hitem:");
    if (ptr == NULL)
    {
        *itemid = 0;
        return slen;
    }

    ptr += 7;
    *itemid = atoi(ptr);

    ptr = strstr(link, "|r");
    if (ptr == NULL)
    {
        *itemid = 0;
        return slen;
    }

    ptr += 2;
    return (ptr - link) & 0x0000ffff;
}

int32 GMWarden::GetSpellIDFromLink(const char* link)
{
    if (link == NULL)
        return 0;

    const char* ptr = strstr(link, "|Hspell:");
    if (ptr == NULL)
        return 0;

    return atol(ptr + 8);
}