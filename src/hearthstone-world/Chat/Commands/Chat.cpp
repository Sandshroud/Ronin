/*
 * Sandshroud Hearthstone
 * Copyright (C) 2010 - 2011 Sandshroud <http://www.sandshroud.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

initialiseSingleton( ChatHandler );
initialiseSingleton(CommandTableStorage);

ChatCommand * ChatHandler::getCommandTable()
{
    ASSERT(false);
    return 0;
}

ChatCommand * CommandTableStorage::GetSubCommandTable(const char * name)
{
    if(!stricmp(name, "modify"))
        return _modifyCommandTable;
    else if(!stricmp(name, "debug"))
        return _debugCommandTable;
    else if(!stricmp(name, "waypoint"))
        return _waypointCommandTable;
    else if(!stricmp(name, "ticket"))
        return _GMTicketCommandTable;
    else if(!stricmp(name, "gobject"))
        return _GameObjectCommandTable;
    else if(!stricmp(name, "battleground"))
        return _BattlegroundCommandTable;
    else if(!stricmp(name, "npc"))
        return _NPCCommandTable;
    else if(!stricmp(name, "gm"))
        return _gamemasterCommandTable;
    else if(!stricmp(name, "tracker"))
        return _trackerCommandTable;
    else if(!stricmp(name, "warn"))
        return _warnCommandTable;
    else if(!stricmp(name, "admin"))
        return _administratorCommandTable;
    else if(!stricmp(name, "cheat"))
        return _CheatCommandTable;
    else if(!stricmp(name, "account"))
        return _accountCommandTable;
    else if(!stricmp(name, "pet"))
        return _petCommandTable;
    else if(!stricmp(name, "recall"))
        return _recallCommandTable;
    else if(!stricmp(name, "honor"))
        return _honorCommandTable;
    else if(!stricmp(name, "guild"))
        return _GuildCommandTable;
    else if(!stricmp(name, "title"))
        return _TitleCommandTable;
    else if(!stricmp(name, "quest"))
        return _questCommandTable;
    else if(!stricmp(name, "lookup"))
        return _lookupCommandTable;
    else if(!stricmp(name, "faction"))
        return _FactionCommandTable;
    return 0;
}

#define dupe_command_table(ct, dt) this->dt = (ChatCommand*)allocate_and_copy(sizeof(ct)/* / sizeof(ct[0])*/, ct)
HEARTHSTONE_INLINE void* allocate_and_copy(uint32 len, void * pointer)
{
    void * data = (void*)malloc(len);
    memcpy(data, pointer, len);
    return data;
}

void CommandTableStorage::Load()
{
    QueryResult * result = WorldDatabase.Query("SELECT * FROM command_overrides");
    if(!result) return;

    do
    {
        const char * name = result->Fetch()[0].GetString();
        const char * level = result->Fetch()[1].GetString();
        Override(name, level);
    } while(result->NextRow());
    delete result;
}

void CommandTableStorage::Override(const char * command, const char * level)
{
    ASSERT(level[0] != '\0');
    char * cmd = strdup(command);

    // find the command we're talking about
    char * sp = strchr(cmd, ' ');
    const char * command_name = cmd;
    const char * subcommand_name = 0;

    if(sp != 0)
    {
        // we're dealing with a subcommand.
        *sp = 0;
        subcommand_name = sp + 1;
    }

    size_t len1 = strlen(command_name);
    size_t len2 = subcommand_name ? strlen(subcommand_name) : 0;

    // look for the command.
    ChatCommand * p = &_commandTable[0];
    while(p->Name != 0)
    {
        if(!strnicmp(p->Name, command_name, len1))
        {
            // this is the one we wanna modify
            if(!subcommand_name)
            {
                // no subcommand, we can change it.
                p->CommandGroup = level[0];
                printf("Changing command level of command `%s` to %c.\n", p->Name, level[0]);
            }
            else
            {
                // assume this is a subcommand, loop the second set.
                ChatCommand * p2 = p->ChildCommands;
                if(!p2)
                {
                    printf("Invalid command specified for override: %s\n", command_name);
                }
                else
                {
                    while(p2->Name != 0)
                    {
                        if(!strnicmp("*",subcommand_name,1))
                        {
                                p2->CommandGroup = level[0];
                                printf("Changing command level of command (wildcard) `%s`:`%s` to %c.\n", p->Name, p2->Name, level[0]);
                        }else{
                            if(!strnicmp(p2->Name, subcommand_name, len2))
                            {
                                // change the level
                                p2->CommandGroup = level[0];
                                printf("Changing command level of command `%s`:`%s` to %c.\n", p->Name, p2->Name, level[0]);
                                break;
                            }
                        }
                        p2++;
                    }
                    if(p2->Name == 0)
                    {
                        if(strnicmp("*",subcommand_name,1)) //Hacky.. meh.. -DGM
                        {
                            printf("Invalid subcommand referenced: `%s` under `%s`.\n", subcommand_name, p->Name);
                        }
                        break;
                    }
                }
            }
            break;
        }
        ++p;
    }

    if(p->Name == 0)
    {
        printf("Invalid command referenced: `%s`\n", command_name);
    }

    free(cmd);
}

void CommandTableStorage::Dealloc()
{
    free( _modifyCommandTable );
    free( _debugCommandTable );
    free( _waypointCommandTable );
    free( _GMTicketCommandTable );
    free( _GameObjectCommandTable );
    free( _BattlegroundCommandTable );
    free( _NPCCommandTable );
    free( _gamemasterCommandTable );
    free( _trackerCommandTable );
    free( _warnCommandTable );
    free( _administratorCommandTable );
    free( _CheatCommandTable );
    free( _accountCommandTable );
    free( _petCommandTable );
    free( _recallCommandTable );
    free( _honorCommandTable );
    free( _GuildCommandTable);
    free( _TitleCommandTable);
    free( _questCommandTable );
    free( _lookupCommandTable );
    free( _commandTable );
}

void CommandTableStorage::Init()
{
    static ChatCommand modifyCommandTable[] =
    {
        { "hp",                         COMMAND_LEVEL_M, NULL,                                          "Health Points/HP",         NULL, UNIT_FIELD_HEALTH,            UNIT_FIELD_MAXHEALTH,   1 },
        { "gender",                     COMMAND_LEVEL_M, &ChatHandler::HandleGenderChanger,             "Changes gender",           NULL, 0,                            0,                      0 },
        { "mana",                       COMMAND_LEVEL_M, NULL,                                          "Mana Points/MP",           NULL, UNIT_FIELD_POWER1,            UNIT_FIELD_MAXPOWER1,   1 },
        { "rage",                       COMMAND_LEVEL_M, NULL,                                          "Rage Points",              NULL, UNIT_FIELD_POWER2,            UNIT_FIELD_MAXPOWER2,   1 },
        { "runicpower",                 COMMAND_LEVEL_M, NULL,                                          "Runic Power",              NULL, UNIT_FIELD_POWER7,            UNIT_FIELD_MAXPOWER7,   1 },
        { "energy",                     COMMAND_LEVEL_M, NULL,                                          "Energy Points",            NULL, UNIT_FIELD_POWER4,            UNIT_FIELD_MAXPOWER4,   1 },
        { "level",                      COMMAND_LEVEL_M, &ChatHandler::HandleModifyLevelCommand,        "Level",                    NULL, 0,                            0,                      0 },
        { "armor",                      COMMAND_LEVEL_M, NULL,                                          "Armor",                    NULL, UNIT_FIELD_STAT1,             0,                      1 },
        { "holy",                       COMMAND_LEVEL_M, NULL,                                          "Holy Resistance",          NULL, UNIT_FIELD_RESISTANCES+1,     0,                      1 },
        { "fire",                       COMMAND_LEVEL_M, NULL,                                          "Fire Resistance",          NULL, UNIT_FIELD_RESISTANCES+2,     0,                      1 },
        { "nature",                     COMMAND_LEVEL_M, NULL,                                          "Nature Resistance",        NULL, UNIT_FIELD_RESISTANCES+3,     0,                      1 },
        { "frost",                      COMMAND_LEVEL_M, NULL,                                          "Frost Resistance",         NULL, UNIT_FIELD_RESISTANCES+4,     0,                      1 },
        { "shadow",                     COMMAND_LEVEL_M, NULL,                                          "Shadow Resistance",        NULL, UNIT_FIELD_RESISTANCES+5,     0,                      1 },
        { "arcane",                     COMMAND_LEVEL_M, NULL,                                          "Arcane Resistance",        NULL, UNIT_FIELD_RESISTANCES+6,     0,                      1 },
        { "damage",                     COMMAND_LEVEL_M, NULL,                                          "Unit Damage Min/Max",      NULL, UNIT_FIELD_MINDAMAGE,         UNIT_FIELD_MAXDAMAGE,   2 },
        { "scale",                      COMMAND_LEVEL_M, &ChatHandler::HandleModifyScaleCommand,        "Size/Scale",               NULL, 0,                            0,                      2 },
        { "gold",                       COMMAND_LEVEL_M, &ChatHandler::HandleModifyGoldCommand,         "Gold/Money/Copper",        NULL, 0,                            0,                      0 },
        { "speed",                      COMMAND_LEVEL_M, &ChatHandler::HandleModifySpeedCommand,        "Movement Speed",           NULL, 0,                            0,                      0 },
        { "nativedisplayid",            COMMAND_LEVEL_M, NULL,                                          "Native Display ID",        NULL, UNIT_FIELD_NATIVEDISPLAYID,   0,                      1 },
        { "displayid",                  COMMAND_LEVEL_M, NULL,                                          "Display ID",               NULL, UNIT_FIELD_DISPLAYID,         0,                      1 },
        { "flags",                      COMMAND_LEVEL_M, NULL,                                          "Unit Flags",               NULL, UNIT_FIELD_FLAGS,             0,                      1 },
        { "faction",                    COMMAND_LEVEL_M, &ChatHandler::HandleModifyFactionCommand,      "Faction Template",         NULL, 0,                            0,                      1 },
        { "dynamicflags",               COMMAND_LEVEL_M, NULL,                                          "Dynamic Flags",            NULL, UNIT_DYNAMIC_FLAGS,           0,                      1 },
        { "talentpoints",               COMMAND_LEVEL_M, &ChatHandler::HandleModifyTPsCommand,          "Talent points",            NULL, 0,                            0,                      0 },
        { "happiness",                  COMMAND_LEVEL_M, NULL,                                          "Happiness",                NULL, UNIT_FIELD_POWER5,            UNIT_FIELD_MAXPOWER5,   1 },
        { "spirit",                     COMMAND_LEVEL_M, NULL,                                          "Spirit",                   NULL, UNIT_FIELD_STAT0,             0,                      1 },
        { "boundingraidius",            COMMAND_LEVEL_M, NULL,                                          "Bounding Radius",          NULL, UNIT_FIELD_BOUNDINGRADIUS,    0,                      2 },
        { "combatreach",                COMMAND_LEVEL_M, NULL,                                          "Combat Reach",             NULL, UNIT_FIELD_COMBATREACH,       0,                      2 },
        { "bytes",                      COMMAND_LEVEL_M, NULL,                                          "Bytes",                    NULL, UNIT_FIELD_BYTES_0,           0,                      1 },
        { "playerflags",                COMMAND_LEVEL_M, &ChatHandler::HandleModifyPlayerFlagsCommand,  "modify a player's flags",  NULL, 0,                            0,                      0 },
        { "aurastate",                  COMMAND_LEVEL_M, &ChatHandler::HandleModifyAuraStateCommand,    "mods player's aurastate",  NULL, 0,                            0,                      0 },
        { NULL,                         COMMAND_LEVEL_0, NULL,                                          "",                         NULL, 0,                            0,                      0 }
    };
    dupe_command_table(modifyCommandTable, _modifyCommandTable);

    static ChatCommand warnCommandTable[] =
    {
        { "add",                        COMMAND_LEVEL_T, &ChatHandler::HandleWarnPlayerCommand,     "Warns a player, Syntax: !warn add <playername> <reason>",                      NULL, 0, 0, 0 },
        { "list",                       COMMAND_LEVEL_T, &ChatHandler::HandleWarnListCommand,       "Warns a player, Syntax: !warn list <playername>",                              NULL, 0, 0, 0 },
        { "clearall",                   COMMAND_LEVEL_Z, &ChatHandler::HandleWarnClearCommand,      "Clears warns from a player, Syntax: !warn clear <playername>",                 NULL, 0, 0, 0 },
        { "delete",                     COMMAND_LEVEL_T, &ChatHandler::HandleWarnSingleDelCommand,  "Deletes a warn from a player, Syntax: !warn delete <playername> <WarnID>",     NULL, 0, 0, 0 },
        { NULL,                         COMMAND_LEVEL_0, NULL,                                      "",                                                                             NULL, 0, 0, 0 }
    };
    dupe_command_table(warnCommandTable, _warnCommandTable);

    static ChatCommand debugCommandTable[] =
    {
        { "retroactivequest",           COMMAND_LEVEL_D, &ChatHandler::HandleDebugRetroactiveQuestAchievements,     "",                                                                                                                     NULL, 0, 0, 0 },
        { "setphase",                   COMMAND_LEVEL_D, &ChatHandler::HandleDebugSetPhase,                         "",                                                                                                                     NULL, 0, 0, 0 },
        { "infront",                    COMMAND_LEVEL_D, &ChatHandler::HandleDebugInFrontCommand,                   "",                                                                                                                     NULL, 0, 0, 0 },
        { "showreact",                  COMMAND_LEVEL_D, &ChatHandler::HandleShowReactionCommand,                   "",                                                                                                                     NULL, 0, 0, 0 },
        { "dist",                       COMMAND_LEVEL_D, &ChatHandler::HandleDistanceCommand,                       "",                                                                                                                     NULL, 0, 0, 0 },
        { "face",                       COMMAND_LEVEL_D, &ChatHandler::HandleFaceCommand,                           "",                                                                                                                     NULL, 0, 0, 0 },
        { "moveinfo",                   COMMAND_LEVEL_D, &ChatHandler::HandleMoveInfoCommand,                       "",                                                                                                                     NULL, 0, 0, 0 },
        { "setbytes",                   COMMAND_LEVEL_Z, &ChatHandler::HandleSetBytesCommand,                       "",                                                                                                                     NULL, 0, 0, 0 },
        { "getbytes",                   COMMAND_LEVEL_D, &ChatHandler::HandleGetBytesCommand,                       "",                                                                                                                     NULL, 0, 0, 0 },
        { "unroot",                     COMMAND_LEVEL_D, &ChatHandler::HandleDebugUnroot,                           "",                                                                                                                     NULL, 0, 0, 0 },
        { "root",                       COMMAND_LEVEL_D, &ChatHandler::HandleDebugRoot,                             "",                                                                                                                     NULL, 0, 0, 0 },
        { "landwalk",                   COMMAND_LEVEL_D, &ChatHandler::HandleDebugLandWalk,                         "",                                                                                                                     NULL, 0, 0, 0 },
        { "waterwalk",                  COMMAND_LEVEL_D, &ChatHandler::HandleDebugWaterWalk,                        "",                                                                                                                     NULL, 0, 0, 0 },
        { "castspellne",                COMMAND_LEVEL_D, &ChatHandler::HandleCastSpellNECommand,                    ".castspellne <spellid> - Casts spell on target (only plays animations, doesnt handle effects or range/facing/etc.",    NULL, 0, 0, 0 },
        { "aggrorange",                 COMMAND_LEVEL_D, &ChatHandler::HandleAggroRangeCommand,                     ".aggrorange - Shows aggro Range of the selected Creature.",                                                            NULL, 0, 0, 0 },
        { "knockback ",                 COMMAND_LEVEL_D, &ChatHandler::HandleKnockBackCommand,                      ".knockback <hspeed> <vspeed> - Knocks selected player back.",                                                          NULL, 0, 0, 0 },
        { "fade ",                      COMMAND_LEVEL_D, &ChatHandler::HandleFadeCommand,                           ".fade <value> - calls ModThreatModifyer().",                                                                           NULL, 0, 0, 0 },
        { "threatMod ",                 COMMAND_LEVEL_D, &ChatHandler::HandleThreatModCommand,                      ".threatMod <value> - calls ModGeneratedThreatModifyer().",                                                             NULL, 0, 0, 0 },
        { "calcThreat ",                COMMAND_LEVEL_D, &ChatHandler::HandleCalcThreatCommand,                     ".calcThreat <dmg> <spellId> - calculates threat.",                                                                     NULL, 0, 0, 0 },
        { "threatList ",                COMMAND_LEVEL_D, &ChatHandler::HandleThreatListCommand,                     ".threatList - returns all AI_Targets of the selected Creature.",                                                       NULL, 0, 0, 0 },
        { "gettptime",                  COMMAND_LEVEL_D, &ChatHandler::HandleGetTransporterTime,                    "grabs transporter travel time",                                                                                        NULL, 0, 0, 0 },
        { "setbit",                     COMMAND_LEVEL_D, &ChatHandler::HandleModifyBitCommand,                      "",                                                                                                                     NULL, 0, 0, 0 },
        { "setvalue",                   COMMAND_LEVEL_D, &ChatHandler::HandleModifyValueCommand,                    "",                                                                                                                     NULL, 0, 0, 0 },
        { "rangecheck",                 COMMAND_LEVEL_D, &ChatHandler::HandleRangeCheckCommand,                     "Checks the 'yard' range and internal range between the player and the target.",                                        NULL, 0, 0, 0 },
        { "setallratings",              COMMAND_LEVEL_D, &ChatHandler::HandleRatingsCommand,                        "Sets rating values to incremental numbers based on their index.",                                                      NULL, 0, 0, 0 },
        { "sendmirrortimer",            COMMAND_LEVEL_D, &ChatHandler::HandleMirrorTimerCommand,                    "Sends a mirror Timer opcode to target syntax: <type>",                                                                 NULL, 0, 0, 0 },
        { "setstartlocation",           COMMAND_LEVEL_D, &ChatHandler::HandleSetPlayerStartLocation,                "",                                                                                                                     NULL, 0, 0, 0 },
        { NULL,                         COMMAND_LEVEL_0, NULL,                                                      "",                                                                                                                     NULL, 0, 0, 0 }
    };
    dupe_command_table(debugCommandTable, _debugCommandTable);

    static ChatCommand waypointCommandTable[] =
    {
        { "add",                    COMMAND_LEVEL_W, &ChatHandler::HandleWPAddCommand,                  "Add wp at current pos",                                NULL, 0, 0, 0 },
        { "show",                   COMMAND_LEVEL_W, &ChatHandler::HandleWPShowCommand,                 "Show wp's for creature",                               NULL, 0, 0, 0 },
        { "hide",                   COMMAND_LEVEL_W, &ChatHandler::HandleWPHideCommand,                 "Hide wp's for creature",                               NULL, 0, 0, 0 },
        { "delete",                 COMMAND_LEVEL_W, &ChatHandler::HandleWPDeleteCommand,               "Delete selected wp",                                   NULL, 0, 0, 0 },
        { "movehere",               COMMAND_LEVEL_W, &ChatHandler::HandleWPMoveHereCommand,             "Move to this wp",                                      NULL, 0, 0, 0 },
        { "flags",                  COMMAND_LEVEL_W, &ChatHandler::HandleWPFlagsCommand,                "Wp flags",                                             NULL, 0, 0, 0 },
        { "waittime",               COMMAND_LEVEL_W, &ChatHandler::HandleWPWaitCommand,                 "Wait time at this wp",                                 NULL, 0, 0, 0 },
        { "standstate",             COMMAND_LEVEL_W, &ChatHandler::HandleWPStandStateCommand,           "StandState at this wp <direction><standstate0-8)",     NULL, 0, 0, 0 },
        { "SpellToCast",            COMMAND_LEVEL_W, &ChatHandler::HandleWPSpellToCastCommand,          "Spell to cast at this wp <direction><spell_id>",       NULL, 0, 0, 0 },
        { "emote",                  COMMAND_LEVEL_W, &ChatHandler::HandleWPEmoteCommand,                "Emote at this wp",                                     NULL, 0, 0, 0 },
        { "skin",                   COMMAND_LEVEL_W, &ChatHandler::HandleWPSkinCommand,                 "Skin at this wp",                                      NULL, 0, 0, 0 },
        { "change",                 COMMAND_LEVEL_W, &ChatHandler::HandleWPChangeNoCommand,             "Change at this wp",                                    NULL, 0, 0, 0 },
        { "info",                   COMMAND_LEVEL_W, &ChatHandler::HandleWPInfoCommand,                 "Show info for wp",                                     NULL, 0, 0, 0 },
        { "movetype",               COMMAND_LEVEL_W, &ChatHandler::HandleWPMoveTypeCommand,             "Movement type at wp",                                  NULL, 0, 0, 0 },
        { "generate",               COMMAND_LEVEL_W, &ChatHandler::HandleGenerateWaypoints,             "Randomly generate wps",                                NULL, 0, 0, 0 },
        { "saveall",                COMMAND_LEVEL_W, &ChatHandler::HandleSaveWaypoints,                 "Save all waypoints",                                   NULL, 0, 0, 0 },
        { "deleteall",              COMMAND_LEVEL_W, &ChatHandler::HandleDeleteWaypoints,               "Delete all waypoints",                                 NULL, 0, 0, 0 },
        { "addfly",                 COMMAND_LEVEL_W, &ChatHandler::HandleWaypointAddFlyCommand,         "Adds a flying waypoint",                               NULL, 0, 0, 0 },
        { "gettext",                COMMAND_LEVEL_W, &ChatHandler::HandleWaypointGettextCommand,        "Shows saytext for current waypoint.",                  NULL, 0, 0, 0 },
        { "backwardtext",           COMMAND_LEVEL_W, &ChatHandler::HandleWaypointBackwardTextCommand,   "Adds backward text to current waypoint <SayText>",     NULL, 0, 0, 0 },
        { "forwardtext",            COMMAND_LEVEL_W, &ChatHandler::HandleWaypointForwardTextCommand,    "Adds forward text to current waypoint <SayText>",      NULL, 0, 0, 0 },
        { "orient",                 COMMAND_LEVEL_W, &ChatHandler::HandleWaypointSetOrientationCommand, "Adds forward text to current waypoint <SayText>",      NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                              "",                                                     NULL, 0, 0, 0 }
    };
    dupe_command_table(waypointCommandTable, _waypointCommandTable);

    static ChatCommand GMTicketCommandTable[] =
    {
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        { "get",                    COMMAND_LEVEL_C, &ChatHandler::HandleGMTicketListCommand,                       "Gets GM Ticket list.",                                             NULL, 0, 0, 0 },
        { "getId",                  COMMAND_LEVEL_C, &ChatHandler::HandleGMTicketGetByIdCommand,                    "Gets GM Ticket by player name.",                                   NULL, 0, 0, 0 },
        { "delId",                  COMMAND_LEVEL_C, &ChatHandler::HandleGMTicketRemoveByIdCommand,                 "Deletes GM Ticket by player name.",                                NULL, 0, 0, 0 },
#else
        { "list",                   COMMAND_LEVEL_C, &ChatHandler::HandleGMTicketListCommand,                       "Lists all active GM Tickets.",                                     NULL, 0, 0, 0 },
        { "get",                    COMMAND_LEVEL_C, &ChatHandler::HandleGMTicketGetByIdCommand,                    "Gets GM Ticket with ID x.",                                        NULL, 0, 0, 0 },
        { "remove",                 COMMAND_LEVEL_C, &ChatHandler::HandleGMTicketRemoveByIdCommand,                 "Removes GM Ticket with ID x.",                                     NULL, 0, 0, 0 },
        { "deletepermanent",        COMMAND_LEVEL_Z, &ChatHandler::HandleGMTicketDeletePermanentCommand,            "Deletes GM Ticket with ID x permanently.",                         NULL, 0, 0, 0 },
        { "assign",                 COMMAND_LEVEL_C, &ChatHandler::HandleGMTicketAssignToCommand,                   "Assigns GM Ticket with id x to GM y (if empty to your self).",     NULL, 0, 0, 0 },
        { "release",                COMMAND_LEVEL_C, &ChatHandler::HandleGMTicketReleaseCommand,                    "Releases assigned GM Ticket with ID x.",                           NULL, 0, 0, 0 },
        { "comment",                COMMAND_LEVEL_C, &ChatHandler::HandleGMTicketCommentCommand,                    "Sets comment x to GM Ticket with ID y.",                           NULL, 0, 0, 0 },
#endif
        { NULL,                     COMMAND_LEVEL_0, NULL,                                                          "",                                                                 NULL, 0, 0, 0 }
    };
    dupe_command_table(GMTicketCommandTable, _GMTicketCommandTable);

    static ChatCommand GuildCommandTable[] =
    {
        { "create",                 COMMAND_LEVEL_M, &ChatHandler::CreateGuildCommand,              "Creates a guild.",                                         NULL, 0, 0, 0 },
        { "removeplayer",           COMMAND_LEVEL_M, &ChatHandler::HandleGuildRemovePlayerCommand,  "Removes the target from its guild.",                       NULL, 0, 0, 0 },
        { "disband",                COMMAND_LEVEL_M, &ChatHandler::HandleGuildDisbandCommand,       "Disbands the guild of your target.",                       NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                         NULL, 0, 0, 0 }
    };
    dupe_command_table(GuildCommandTable, _GuildCommandTable);

    static ChatCommand TitleCommandTable[] =
    {
        { "add",                    COMMAND_LEVEL_M, &ChatHandler::HandleAddTitleCommand,           "<TitleNumber> - Adds known title to the selected player",          NULL, 0, 0, 0 },
        { "remove",                 COMMAND_LEVEL_M, &ChatHandler::HandleRemoveTitleCommand,        "<TitleNumber> - Removes known title from the selected player",     NULL, 0, 0, 0 },
        { "known",                  COMMAND_LEVEL_M, &ChatHandler::HandleGetKnownTitlesCommand,     "Shows all titles known by the player",                             NULL, 0, 0, 0 },
        { "setchosen",              COMMAND_LEVEL_M, &ChatHandler::HandleSetChosenTitleCommand,     "<TitleNumber> - Sets chosen title for the selected player",        NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                                 NULL, 0, 0, 0 }
    };
    dupe_command_table(TitleCommandTable, _TitleCommandTable);

    static ChatCommand GameObjectCommandTable[] =
    {
        { "select",                 COMMAND_LEVEL_O, &ChatHandler::HandleGOSelect,          "Selects the nearest GameObject to you",        NULL, 0,                    0, 0 },
        { "delete",                 COMMAND_LEVEL_O, &ChatHandler::HandleGODelete,          "Deletes selected GameObject",                  NULL, 0,                    0, 0 },
        { "spawn",                  COMMAND_LEVEL_O, &ChatHandler::HandleGOSpawn,           "Spawns a GameObject by ID",                    NULL, 0,                    0, 0 },
        { "info",                   COMMAND_LEVEL_O, &ChatHandler::HandleGOInfo,            "Gives you informations about selected GO",     NULL, 0,                    0, 0 },
        { "activate",               COMMAND_LEVEL_O, &ChatHandler::HandleGOActivate,        "Activates/Opens the selected GO.",             NULL, 0,                    0, 0 },
        { "enable",                 COMMAND_LEVEL_O, &ChatHandler::HandleGOEnable,          "Enables the selected GO for use.",             NULL, 0,                    0, 0 },
        { "scale",                  COMMAND_LEVEL_O, &ChatHandler::HandleGOScale,           "Sets scale of selected GO",                    NULL, 0,                    0, 0 },
        { "animprogress",           COMMAND_LEVEL_O, &ChatHandler::HandleGOAnimProgress,    "Sets anim progress",                           NULL, 0,                    0, 0 },
        { "move",                   COMMAND_LEVEL_G, &ChatHandler::HandleGOMove,            "Moves gameobject to player xyz",               NULL, 0,                    0, 0 },
        { "rotate",                 COMMAND_LEVEL_G, &ChatHandler::HandleGORotate,          "Rotates gameobject x degrees",                 NULL, 0,                    0, 0 },
        { "damage",                 COMMAND_LEVEL_G, &ChatHandler::HandleDebugGoDamage,     "Damages the gameobject for <args>",            NULL, 0,                    0, 0 },
        { "rebuild",                COMMAND_LEVEL_G, &ChatHandler::HandleDebugGoRepair,     "Resets the gameobject health and state",       NULL, 0,                    0, 0 },
        { "f_flags",                COMMAND_LEVEL_G, NULL,                                  "Flags",                                        NULL, GAMEOBJECT_FLAGS,     0, 3 },
        { "f_dynflags",             COMMAND_LEVEL_G, NULL,                                  "Dynflags",                                     NULL, GAMEOBJECT_DYNAMIC,   0, 3 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                  "",                                             NULL, 0,                    0, 0 }
    };
    dupe_command_table(GameObjectCommandTable, _GameObjectCommandTable);

    static ChatCommand BattlegroundCommandTable[] =
    {
        { "setbgscore",             COMMAND_LEVEL_E, &ChatHandler::HandleSetBGScoreCommand,                 "<Teamid> <Score> - Sets battleground score. 2 Arguments. ",        NULL, 0, 0, 0 },
        { "startbg",                COMMAND_LEVEL_E, &ChatHandler::HandleStartBGCommand,                    "Starts current battleground match.",                               NULL, 0, 0, 0 },
        { "pausebg",                COMMAND_LEVEL_E, &ChatHandler::HandlePauseBGCommand,                    "Pauses current battleground match.",                               NULL, 0, 0, 0 },
        { "bginfo",                 COMMAND_LEVEL_E, &ChatHandler::HandleBGInfoCommnad,                     "Displays information about current battleground.",                 NULL, 0, 0, 0 },
        { "setworldstate",          COMMAND_LEVEL_E, &ChatHandler::HandleSetWorldStateCommand,              "<var> <val> - Var can be in hex. WS Value.",                       NULL, 0, 0, 0 },
        { "playsound",              COMMAND_LEVEL_E, &ChatHandler::HandlePlaySoundCommand,                  "<val>. Val can be in hex.",                                        NULL, 0, 0, 0 },
        { "setbfstatus",            COMMAND_LEVEL_E, &ChatHandler::HandleSetBattlefieldStatusCommand,       ".setbfstatus - NYI.",                                              NULL, 0, 0, 0 },
        { "leave",                  COMMAND_LEVEL_E, &ChatHandler::HandleBattlegroundExitCommand,           "Leaves the current battleground.",                                 NULL, 0, 0, 0 },
        { "forcestart",             COMMAND_LEVEL_E, &ChatHandler::HandleBattlegroundForcestartCommand,     "Forcestart current battlegrounds.",                                NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                                  "",                                                                 NULL, 0, 0, 0 }
    };
    dupe_command_table(BattlegroundCommandTable, _BattlegroundCommandTable);

    static ChatCommand NPCCommandTable[] =
    {
        { "vendorclear",            COMMAND_LEVEL_Z, &ChatHandler::HandleVendorClearCommand,        "Clears all items from the vendor",                                                                                                             NULL, 0, 0, 0 },
        { "vendorsetadd",           COMMAND_LEVEL_Z, &ChatHandler::HandleItemSetCommand,            "Adds item set to vendor",                                                                                                                      NULL, 0, 0, 0 },
        { "vendorsetremove",        COMMAND_LEVEL_Z, &ChatHandler::HandleItemSetRemoveCommand,      "Removes item set from vendor",                                                                                                                 NULL, 0, 0, 0 },
        { "vendoradditem",          COMMAND_LEVEL_Z, &ChatHandler::HandleItemCommand,               "Adds to vendor",                                                                                                                               NULL, 0, 0, 0 },
        { "vendorremoveitem",       COMMAND_LEVEL_Z, &ChatHandler::HandleItemRemoveCommand,         "Removes from vendor.",                                                                                                                         NULL, 0, 0, 0 },
        { "traineraddlearn",        COMMAND_LEVEL_Z, &ChatHandler::HandleTrainerAddLearnSpell,      "Adds a spell that is learned to the player. req:<spellid> option:<spellcost><reqspell><reqskill><reqskillval><reqlvl><deletespell><isprof>",   NULL, 0, 0, 0 },
        { "traineraddcast",         COMMAND_LEVEL_Z, &ChatHandler::HandleTrainerAddCastSpell,       "Adds a spell that is cast on the player. req:<spellid> option:<spellcost><reqspell><reqskill><reqskillval><reqlvl><deletespell><isprof>",      NULL, 0, 0, 0 },
        { "flags",                  COMMAND_LEVEL_N, &ChatHandler::HandleNPCFlagCommand,            "Changes NPC flags",                                                                                                                            NULL, 0, 0, 0 },
        { "emote",                  COMMAND_LEVEL_N, &ChatHandler::HandleEmoteCommand,              ".emote - Sets emote state",                                                                                                                    NULL, 0, 0, 0 },
        { "setstandstate",          COMMAND_LEVEL_N, &ChatHandler::HandleStandStateCommand,         ".setstandstate - Sets stand state",                                                                                                            NULL, 0, 0, 0 },
        { "delete",                 COMMAND_LEVEL_N, &ChatHandler::HandleDeleteCommand,             "Deletes mobs from db and world.",                                                                                                              NULL, 0, 0, 0 },
        { "info",                   COMMAND_LEVEL_N, &ChatHandler::HandleNpcInfoCommand,            "Displays NPC information",                                                                                                                     NULL, 0, 0, 0 },
        { "say",                    COMMAND_LEVEL_N, &ChatHandler::HandleMonsterSayCommand,         ".npc say <text> - Makes selected mob say text <text>.",                                                                                        NULL, 0, 0, 0 },
        { "yell",                   COMMAND_LEVEL_N, &ChatHandler::HandleMonsterYellCommand,        ".npc yell <Text> - Makes selected mob yell text <text>.",                                                                                      NULL, 0, 0, 0 },
        { "come",                   COMMAND_LEVEL_N, &ChatHandler::HandleNpcComeCommand,            ".npc come - Makes npc move to your position",                                                                                                  NULL, 0, 0, 0 },
        { "jumpto",                 COMMAND_LEVEL_N, &ChatHandler::HandleNpcJumpCommand,            ".npc come - Makes npc move to your position",                                                                                                  NULL, 0, 0, 0 },
        { "return",                 COMMAND_LEVEL_N, &ChatHandler::HandleNpcReturnCommand,          ".npc return - Returns npc to spawnpoint.",                                                                                                     NULL, 0, 0, 0 },
        { "respawn",                COMMAND_LEVEL_N, &ChatHandler::HandleCreatureRespawnCommand,    ".respawn - Respawns a dead npc from its corpse.",                                                                                              NULL, 0, 0, 0 },
        { "spawn",                  COMMAND_LEVEL_N, &ChatHandler::HandleCreatureSpawnCommand,      ".npc spawn <id> <save> - Spawns npc of entry <id>",                                                                                            NULL, 0, 0, 0 },
        { "spawnlink",              COMMAND_LEVEL_N, &ChatHandler::HandleNpcSpawnLinkCommand,       ".spawnlink sqlentry",                                                                                                                          NULL, 0, 0, 0 },
        { "possess",                COMMAND_LEVEL_N, &ChatHandler::HandleNpcPossessCommand,         ".npc possess - Possess an npc (mind control)",                                                                                                 NULL, 0, 0, 0 },
        { "unpossess",              COMMAND_LEVEL_N, &ChatHandler::HandleNpcUnPossessCommand,       ".npc unpossess - Unposses any currently possessed npc.",                                                                                       NULL, 0, 0, 0 },
        { "select",                 COMMAND_LEVEL_N, &ChatHandler::HandleNpcSelectCommand,          ".npc select - selects npc closest",                                                                                                            NULL, 0, 0, 0 },
        { "cast",                   COMMAND_LEVEL_D, &ChatHandler::HandleMonsterCastCommand,        ".npc cast <spellId> - Makes selected mob cast the specified spell on you.",                                                                    NULL, 0, 0, 0 },
        { "equip",                  COMMAND_LEVEL_A, &ChatHandler::HandleNPCEquipCommand,           "Use: .npc equip <slot> <itemid> - use .npc equip <slot> 0 to remove the item",                                                                 NULL, 0, 0, 0 },
        { "setongameobject",        COMMAND_LEVEL_A, &ChatHandler::HandleNPCSetOnObjectCommand,     "Updates spawn information so that the creature does not fall through objects when spawning into world.",                                       NULL, 0, 0, 0 },
        { "save",                   COMMAND_LEVEL_Z, &ChatHandler::HandleNPCSaveCommand,            "",                                                                                                                                             NULL, 0, 0, 0 },
        { "setvendormask",          COMMAND_LEVEL_Z, &ChatHandler::HandleNPCSetVendorMaskCommand,   "",                                                                                                                                             NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                                                                                                             NULL, 0, 0, 0 }
    };
    dupe_command_table(NPCCommandTable, _NPCCommandTable);

    static ChatCommand GMCommandTable[] =
    {
        { "list",                   COMMAND_LEVEL_1, &ChatHandler::HandleGMListCommand,         "Shows active GM's",                                    NULL,                   0, 0, 0 },
        { "off",                    COMMAND_LEVEL_T, &ChatHandler::HandleGMOffCommand,          "Sets GM tag off",                                      NULL,                   0, 0, 0 },
        { "on",                     COMMAND_LEVEL_T, &ChatHandler::HandleGMOnCommand,           "Sets GM tag on",                                       NULL,                   0, 0, 0 },
        { "disabledev",             COMMAND_LEVEL_Z, &ChatHandler::HandleToggleDevCommand,      "Toggles <Dev> Tag",                                    NULL,                   0, 0, 0 },
        { "announce",               COMMAND_LEVEL_U, &ChatHandler::HandleGMAnnounceCommand,     "Announce to GM's and Admin's",                         NULL,                   0, 0, 0 },
        { "allowwhispers",          COMMAND_LEVEL_C, &ChatHandler::HandleAllowWhispersCommand,  "Allows whispers from player <s> while in gmon mode.",  NULL,                   0, 0, 0 },
        { "blockwhispers",          COMMAND_LEVEL_C, &ChatHandler::HandleBlockWhispersCommand,  "Blocks whispers from player <s> while in gmon mode.",  NULL,                   0, 0, 0 },
        { "ticket",                 COMMAND_LEVEL_C, NULL,                                      "",                                                     GMTicketCommandTable,   0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                      "",                                                     NULL,                   0, 0, 0 }
    };
    dupe_command_table(GMCommandTable, _gamemasterCommandTable);

    static ChatCommand trackerCommandTable[] =
    {
        { "add",                    COMMAND_LEVEL_Z, &ChatHandler::HandleAddTrackerCommand,         "Attaches a tracker to a GM's IP allowing them to be followed across accounts.",    NULL, 0, 0, 0 },
        { "del",                    COMMAND_LEVEL_Z, &ChatHandler::HandleDelTrackerCommand,         "Removes a tracker from a GM's IP, syntax: .tracker del <trackerId>",               NULL, 0, 0, 0 },
        { "list",                   COMMAND_LEVEL_Z, &ChatHandler::HandleTrackerListCommand,        "Checks if there are any accounts connected from the tracked GMs' IP.",             NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                                                 NULL, 0, 0, 0 }
    };
    dupe_command_table(trackerCommandTable, _trackerCommandTable);

    static ChatCommand AdminCommandTable[] =
    {
        { "announce",               COMMAND_LEVEL_Z, &ChatHandler::HandleAdminAnnounceCommand,      "Command to Announce to Admin's",               NULL, 0, 0, 0 },
        { "castall",                COMMAND_LEVEL_Z, &ChatHandler::HandleCastAllCommand,            "Makes all players online cast spell <x>.",     NULL, 0, 0, 0 },
        { "playall",                COMMAND_LEVEL_Z, &ChatHandler::HandleGlobalPlaySoundCommand,    "Plays a sound to the entire server.",          NULL, 0, 0, 0 },
        { "saveall",                COMMAND_LEVEL_Z, &ChatHandler::HandleSaveAllCommand,            "Save's all playing characters",                NULL, 0, 0, 0 },
        { "rehash",                 COMMAND_LEVEL_Z, &ChatHandler::HandleRehashCommand,             "Reloads config file.",                         NULL, 0, 0, 0 },
        { "enableauctionhouse",     COMMAND_LEVEL_Z, &ChatHandler::HandleEnableAH,                  "Enables Auctionhouse",                         NULL, 0, 0, 0 },
        { "disableauctionhouse",    COMMAND_LEVEL_Z, &ChatHandler::HandleDisableAH,                 "Disables Auctionhouse",                        NULL, 0, 0, 0 },
        { "masssummon",             COMMAND_LEVEL_Z, &ChatHandler::HandleMassSummonCommand,         "Summons all players.",                         NULL, 0, 0, 0 },
        { "restart",                COMMAND_LEVEL_Z, &ChatHandler::HandleRestartCommand,            "Initiates server restart in <x> seconds.",     NULL, 0, 0, 0 },
        { "shutdown",               COMMAND_LEVEL_Z, &ChatHandler::HandleShutdownCommand,           "Initiates server shutdown in <x> seconds.",    NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                             NULL, 0, 0, 0 }
    };
    dupe_command_table(AdminCommandTable, _administratorCommandTable);

    static ChatCommand CheatCommandTable[] =
    {
        { "status",                 COMMAND_LEVEL_M, &ChatHandler::HandleShowCheatsCommand,             "Shows active cheats.",                             NULL, 0, 0, 0 },
        { "taxi",                   COMMAND_LEVEL_M, &ChatHandler::HandleTaxiCheatCommand,              "Enables all taxi nodes.",                          NULL, 0, 0, 0 },
        { "cooldown",               COMMAND_LEVEL_M, &ChatHandler::HandleCooldownCheatCommand,          "Enables no cooldown cheat.",                       NULL, 0, 0, 0 },
        { "casttime",               COMMAND_LEVEL_M, &ChatHandler::HandleCastTimeCheatCommand,          "Enables no cast time cheat.",                      NULL, 0, 0, 0 },
        { "power",                  COMMAND_LEVEL_M, &ChatHandler::HandlePowerCheatCommand,             "Disables mana consumption etc.",                   NULL, 0, 0, 0 },
        { "fly",                    COMMAND_LEVEL_M, &ChatHandler::HandleFlyCommand,                    "Sets fly mode",                                    NULL, 0, 0, 0 },
        { "explore",                COMMAND_LEVEL_M, &ChatHandler::HandleExploreCheatCommand,           "Reveals the unexplored parts of the map.",         NULL, 0, 0, 0 },
        { "flyspeed",               COMMAND_LEVEL_M, &ChatHandler::HandleFlySpeedCheatCommand,          "Modifies fly speed.",                              NULL, 0, 0, 0 },
        { "stack",                  COMMAND_LEVEL_M, &ChatHandler::HandleStackCheatCommand,             "Enables aura stacking cheat.",                     NULL, 0, 0, 0 },
        { "triggerpass",            COMMAND_LEVEL_M, &ChatHandler::HandleTriggerpassCheatCommand,       "Ignores area trigger prerequisites.",              NULL, 0, 0, 0 },
        { "vendorpass",             COMMAND_LEVEL_M, &ChatHandler::HandleVendorPassCheatCommand,        "Ignores npc vendor prerequisites.",                NULL, 0, 0, 0 },
        { "itempass",               COMMAND_LEVEL_M, &ChatHandler::HandleItemReqCheatCommand,           "Ignores item requirements(incomplete).",           NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                              "",                                                 NULL, 0, 0, 0 },
    };
    dupe_command_table(CheatCommandTable, _CheatCommandTable);

    static ChatCommand accountCommandTable[] =
    {
        { "ban",                    COMMAND_LEVEL_A, &ChatHandler::HandleAccountBannedCommand,      "Ban account. .account ban name timeperiod reason",     NULL, 0, 0, 0 },
        { "unban",                  COMMAND_LEVEL_Z, &ChatHandler::HandleAccountUnbanCommand,       "Unbans account x.",                                    NULL, 0, 0, 0 },
        { "level",                  COMMAND_LEVEL_Z, &ChatHandler::HandleAccountLevelCommand,       "Sets gm level on account. <username><gm_lvl>.",        NULL, 0, 0, 0 },
        { "mute",                   COMMAND_LEVEL_A, &ChatHandler::HandleAccountMuteCommand,        "Mutes account for <timeperiod>.",                      NULL, 0, 0, 0 },
        { "unmute",                 COMMAND_LEVEL_A, &ChatHandler::HandleAccountUnmuteCommand,      "Unmutes account <x>",                                  NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                     NULL, 0, 0, 0 },
    };
    dupe_command_table(accountCommandTable, _accountCommandTable);

    static ChatCommand honorCommandTable[] =
    {
        { "addpoints",              COMMAND_LEVEL_M, &ChatHandler::HandleAddHonorCommand,                       "Adds x amount of honor points/currency",                   NULL, 0, 0, 0 },
        { "addkills",               COMMAND_LEVEL_M, &ChatHandler::HandleAddKillCommand,                        "Adds x amount of honor kills",                             NULL, 0, 0, 0 },
        { "globaldailyupdate",      COMMAND_LEVEL_M, &ChatHandler::HandleGlobalHonorDailyMaintenanceCommand,    "Daily honor field moves",                                  NULL, 0, 0, 0 },
        { "singledailyupdate",      COMMAND_LEVEL_M, &ChatHandler::HandleNextDayCommand,                        "Daily honor field moves for selected player only",         NULL, 0, 0, 0 },
        { "pvpcredit",              COMMAND_LEVEL_M, &ChatHandler::HandlePVPCreditCommand,                      "Sends PVP credit packet, with specified rank and points",  NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                                      "",                                                         NULL, 0, 0, 0 },
    };
    dupe_command_table(honorCommandTable, _honorCommandTable);

    static ChatCommand petCommandTable[] =
    {
        { "createpet",              COMMAND_LEVEL_M, &ChatHandler::HandleCreatePetCommand,      "Creates a pet with <entry>.",  NULL, 0, 0, 0 },
        { "renamepet",              COMMAND_LEVEL_M, &ChatHandler::HandleRenamePetCommand,      "Renames a pet to <name>.",     NULL, 0, 0, 0 },
        { "addspell",               COMMAND_LEVEL_M, &ChatHandler::HandleAddPetSpellCommand,    "Teaches pet <spell>.",         NULL, 0, 0, 0 },
        { "removespell",            COMMAND_LEVEL_M, &ChatHandler::HandleRemovePetSpellCommand, "Removes pet spell <spell>.",   NULL, 0, 0, 0 },
        { "addtalentpoints",        COMMAND_LEVEL_M, &ChatHandler::HandleAddPetTalentPoints,    "Adds <x> talent points to pet",NULL, 0, 0, 0 },
        { "resettalents",           COMMAND_LEVEL_M, &ChatHandler::HandleResetPetTalents,       "Resets the pets talents",      NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                      "",                             NULL, 0, 0, 0 },
    };
    dupe_command_table(petCommandTable, _petCommandTable);

    static ChatCommand recallCommandTable[] =
    {
        { "list",                   COMMAND_LEVEL_Q, &ChatHandler::HandleRecallListCommand,         "List recall locations [filter].",  NULL, 0, 0, 0 },
        { "port",                   COMMAND_LEVEL_Q, &ChatHandler::HandleRecallGoCommand,           "Port to recalled location",        NULL, 0, 0, 0 },
        { "add",                    COMMAND_LEVEL_Q, &ChatHandler::HandleRecallAddCommand,          "Add recall location",              NULL, 0, 0, 0 },
        { "del",                    COMMAND_LEVEL_Q, &ChatHandler::HandleRecallDelCommand,          "Remove a recall location",         NULL, 0, 0, 0 },
        { "portplayer",             COMMAND_LEVEL_M, &ChatHandler::HandleRecallPortPlayerCommand,   "recall ports player",              NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                 NULL, 0, 0, 0 },
    };
    dupe_command_table(recallCommandTable, _recallCommandTable);

    static ChatCommand lookupCommandTable[] =
    {
        { "item",                   COMMAND_LEVEL_L, &ChatHandler::HandleLookupItemCommand,         "Looks up item string x.",                  NULL, 0, 0, 0 },
        { "itemset",                COMMAND_LEVEL_L, &ChatHandler::HandleLookupItemSetCommand,      "Looks up item set string x.",              NULL, 0, 0, 0 },
        { "quest",                  COMMAND_LEVEL_L, &ChatHandler::HandleQuestLookupCommand,        "Looks up quest string x.",                 NULL, 0, 0, 0 },
        { "creature",               COMMAND_LEVEL_L, &ChatHandler::HandleLookupCreatureCommand,     "Looks up item string x.",                  NULL, 0, 0, 0 },
        { "object",                 COMMAND_LEVEL_L, &ChatHandler::HandleLookupObjectCommand,       "Looks up object string x.",                NULL, 0, 0, 0 },
        { "spell",                  COMMAND_LEVEL_L, &ChatHandler::HandleLookupSpellCommand,        "Looks up spell string x.",                 NULL, 0, 0, 0 },
        { "spellspecific",          COMMAND_LEVEL_L, &ChatHandler::HandleLookupSpellSpecificCommand,"Looks up spell string x.",                 NULL, 0, 0, 0 },
        { "spellname",              COMMAND_LEVEL_L, &ChatHandler::HandleLookupSpellNameCommand,    "Looks up spell id x.",                     NULL, 0, 0, 0 },
        { "title",                  COMMAND_LEVEL_L, &ChatHandler::HandleLookupTitleCommand,        "Looks up title based on inputted name.",   NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                         NULL, 0, 0, 0 },
    };
    dupe_command_table(lookupCommandTable, _lookupCommandTable);

    static ChatCommand questCommandTable[] =
    {
        { "addboth",                COMMAND_LEVEL_2, &ChatHandler::HandleQuestAddBothCommand,       "Add quest <id> to the targeted NPC as start & finish",         NULL, 0, 0, 0 },
        { "addfinish",              COMMAND_LEVEL_2, &ChatHandler::HandleQuestAddFinishCommand,     "Add quest <id> to the targeted NPC as finisher",               NULL, 0, 0, 0 },
        { "addstart",               COMMAND_LEVEL_2, &ChatHandler::HandleQuestAddStartCommand,      "Add quest <id> to the targeted NPC as starter",                NULL, 0, 0, 0 },
        { "delboth",                COMMAND_LEVEL_2, &ChatHandler::HandleQuestDelBothCommand,       "Delete quest <id> from the targeted NPC as start & finish",    NULL, 0, 0, 0 },
        { "delfinish",              COMMAND_LEVEL_2, &ChatHandler::HandleQuestDelFinishCommand,     "Delete quest <id> from the targeted NPC as finisher",          NULL, 0, 0, 0 },
        { "delstart",               COMMAND_LEVEL_2, &ChatHandler::HandleQuestDelStartCommand,      "Delete quest <id> from the targeted NPC as starter",           NULL, 0, 0, 0 },
        { "complete",               COMMAND_LEVEL_2, &ChatHandler::HandleQuestFinishCommand,        "Complete/Finish quest <id>",                                   NULL, 0, 0, 0 },
        { "finisher",               COMMAND_LEVEL_2, &ChatHandler::HandleQuestFinisherCommand,      "Lookup quest finisher for quest <id>",                         NULL, 0, 0, 0 },
        { "item",                   COMMAND_LEVEL_2, &ChatHandler::HandleQuestItemCommand,          "Lookup itemid necessary for quest <id>",                       NULL, 0, 0, 0 },
        { "liststart",              COMMAND_LEVEL_2, &ChatHandler::HandleQuestListStarterCommand,   "Lists the quests for the npc <id>",                            NULL, 0, 0, 0 },
        { "listfinish",             COMMAND_LEVEL_2, &ChatHandler::HandleQuestListFinisherCommand,  "Lists the quests for the npc <id>",                            NULL, 0, 0, 0 },
        { "load",                   COMMAND_LEVEL_2, &ChatHandler::HandleQuestLoadCommand,          "Loads quests from database",                                   NULL, 0, 0, 0 },
        { "lookup",                 COMMAND_LEVEL_2, &ChatHandler::HandleQuestLookupCommand,        "Looks up quest string x",                                      NULL, 0, 0, 0 },
        { "giver",                  COMMAND_LEVEL_2, &ChatHandler::HandleQuestGiverCommand,         "Lookup quest giver for quest <id>",                            NULL, 0, 0, 0 },
        { "remove",                 COMMAND_LEVEL_2, &ChatHandler::HandleQuestRemoveCommand,        "Removes the quest <id> from the targeted player",              NULL, 0, 0, 0 },
        { "reward",                 COMMAND_LEVEL_2, &ChatHandler::HandleQuestRewardCommand,        "Shows reward for quest <id>",                                  NULL, 0, 0, 0 },
        { "status",                 COMMAND_LEVEL_2, &ChatHandler::HandleQuestStatusCommand,        "Lists the status of quest <id>",                               NULL, 0, 0, 0 },
        { "spawn",                  COMMAND_LEVEL_2, &ChatHandler::HandleQuestSpawnCommand,         "Port to spawn location for quest <id>",                        NULL, 0, 0, 0 },
        { "start",                  COMMAND_LEVEL_2, &ChatHandler::HandleQuestStartCommand,         "Starts quest <id>",                                            NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                          "",                                                             NULL, 0, 0, 0 },
    };
    dupe_command_table(questCommandTable, _questCommandTable);

    static ChatCommand FactionCommandTable[] =
    {
        { "modstanding",            COMMAND_LEVEL_Z, &ChatHandler::HandleFactionModStanding,    "Mods a player's faction's standing based on a value",  NULL, 0, 0, 0 },
        { "setstanding",            COMMAND_LEVEL_Z, &ChatHandler::HandleFactionSetStanding,    "Changes a player's faction's standing to a value",     NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                      "",                                                     NULL, 0, 0, 0 },
    };
    dupe_command_table(FactionCommandTable, _FactionCommandTable);

    static ChatCommand commandTable[] = {
        { "commands",               COMMAND_LEVEL_0, &ChatHandler::HandleCommandsCommand,                           "Shows Commands",               NULL, 0, 0, 0 },
        { "help",                   COMMAND_LEVEL_0, &ChatHandler::HandleHelpCommand,                               "Shows help for command",       NULL, 0, 0, 0 },
        { "announce",               COMMAND_LEVEL_U, &ChatHandler::HandleAnnounceCommand,                           "Sends Msg To All",             NULL, 0, 0, 0 },
        { "wannounce",              COMMAND_LEVEL_U, &ChatHandler::HandleWAnnounceCommand,                          "Sends Widescreen Msg To All",  NULL, 0, 0, 0 },
        { "appear",                 COMMAND_LEVEL_V, &ChatHandler::HandleAppearCommand,                             "Teleports to x's position.",   NULL, 0, 0, 0 },
        { "summon",                 COMMAND_LEVEL_V, &ChatHandler::HandleSummonCommand,                             "Summons x to your position",   NULL, 0, 0, 0 },
        { "banchar",                COMMAND_LEVEL_B, &ChatHandler::HandleBanCharacterCommand,                       "Bans character x with or without reason",              NULL, 0, 0, 0 },
        { "unbanchar",              COMMAND_LEVEL_B, &ChatHandler::HandleUnBanCharacterCommand,                     "Unbans character x",           NULL, 0, 0, 0 },
        { "kick",                   COMMAND_LEVEL_B, &ChatHandler::HandleKickCommand,                               "Kicks player from server",     NULL, 0, 0, 0 },
        { "kill",                   COMMAND_LEVEL_R, &ChatHandler::HandleKillCommand,                               ".kill - Kills selected unit.", NULL, 0, 0, 0 },
        { "killplr" ,               COMMAND_LEVEL_R, &ChatHandler::HandleKillByPlrCommand,                          ".killplr <name> - Kills specified player" , NULL , 0 , 0 , 0 },
        { "revive",                 COMMAND_LEVEL_R, &ChatHandler::HandleReviveCommand,                             "Revives you.",                 NULL, 0, 0, 0 },
        { "reviveplr",              COMMAND_LEVEL_R, &ChatHandler::HandleReviveStringcommand,                       "Revives player specified.",        NULL, 0, 0, 0 },
        { "demorph",                COMMAND_LEVEL_M, &ChatHandler::HandleDeMorphCommand,                            "Demorphs from morphed model.", NULL, 0, 0, 0 },
        { "mount",                  COMMAND_LEVEL_M, &ChatHandler::HandleMountCommand,                              "Mounts into modelid x.",       NULL, 0, 0, 0 },
        { "dismount",               COMMAND_LEVEL_M, &ChatHandler::HandleDismountCommand,                           "Dismounts.",                   NULL, 0, 0, 0 },
        { "fulldismount",           COMMAND_LEVEL_M, &ChatHandler::HandleFullDismountCommand,                       "Force a player to full dismount (taxi)",   NULL, 0, 0, 0 },
        { "gps",                    COMMAND_LEVEL_0, &ChatHandler::HandleGPSCommand,                                "Shows Position",               NULL, 0, 0, 0 },
        { "info",                   COMMAND_LEVEL_0, &ChatHandler::HandleInfoCommand,                               "Server info",                  NULL, 0, 0, 0 },
        { "worldport",              COMMAND_LEVEL_V, &ChatHandler::HandleWorldPortCommand,                          "",                             NULL, 0, 0, 0 },
        { "save",                   COMMAND_LEVEL_S, &ChatHandler::HandleSaveCommand,                               "Save's your character",            NULL, 0, 0, 0 },
        { "start",                  COMMAND_LEVEL_M, &ChatHandler::HandleStartCommand,                              "Teleport's you to a starting location",                                NULL, 0, 0, 0 },
        { "additem",                COMMAND_LEVEL_M, &ChatHandler::HandleAddInvItemCommand,                         "",                             NULL, 0, 0, 0 },
        { "additemset",             COMMAND_LEVEL_M, &ChatHandler::HandleAddItemSetCommand,                         "Adds item set to inv.",            NULL, 0, 0, 0 },
        { "removeitem",             COMMAND_LEVEL_M, &ChatHandler::HandleRemoveItemCommand,                         "Removes item %u count %u.", NULL, 0, 0, 0 },
        { "invincible",             COMMAND_LEVEL_I, &ChatHandler::HandleInvincibleCommand,                         ".invincible - Toggles INVINCIBILITY (mobs won't attack you)", NULL, 0, 0, 0 },
        { "invisible",              COMMAND_LEVEL_I, &ChatHandler::HandleInvisibleCommand,                          ".invisible - Toggles INVINCIBILITY and INVISIBILITY (mobs won't attack you and nobody can see you, but they can see your chat messages)", NULL, 0, 0, 0 },
        { "resetreputation",        COMMAND_LEVEL_N, &ChatHandler::HandleResetReputationCommand,                    ".resetreputation - Resets reputation to start levels. (use on characters that were made before reputation fixes.)", NULL, 0, 0, 0 },
        { "resetspells",            COMMAND_LEVEL_N, &ChatHandler::HandleResetSpellsCommand,                        ".resetspells - Resets all spells to starting spells of targeted player. DANGEROUS.", NULL, 0, 0, 0 },
        { "resettalents",           COMMAND_LEVEL_N, &ChatHandler::HandleResetTalentsCommand,                       ".resettalents - Resets all talents of targeted player to that of their current level. DANGEROUS.", NULL, 0, 0, 0 },
        { "resetskills",            COMMAND_LEVEL_N, &ChatHandler::HandleResetSkillsCommand ,                       ".resetskills - Resets all skills.", NULL, 0, 0, 0 },
        { "learn",                  COMMAND_LEVEL_M, &ChatHandler::HandleLearnCommand,                              "Learns spell",                 NULL, 0, 0, 0 },
        { "unlearn",                COMMAND_LEVEL_M, &ChatHandler::HandleUnlearnCommand,                            "Unlearns spell",               NULL, 0, 0, 0 },
        { "getskilllevel",          COMMAND_LEVEL_M, &ChatHandler::HandleGetSkillLevelCommand,                      "Gets the current level of a skill",NULL,0,0,0 }, //DGM (maybe add to playerinfo?)
        { "getskillinfo",           COMMAND_LEVEL_M, &ChatHandler::HandleGetSkillsInfoCommand,                      "Gets all the skills from a player",NULL,0,0,0 },
        { "learnskill",             COMMAND_LEVEL_M, &ChatHandler::HandleLearnSkillCommand,                         ".learnskill <skillid> (optional) <value> <maxvalue> - Learns skill id skillid.", NULL, 0, 0, 0 },
        { "advanceskill",           COMMAND_LEVEL_M, &ChatHandler::HandleModifySkillCommand,                        "advanceskill <skillid> <amount, optional, default = 1> - Advances skill line x times..", NULL, 0, 0, 0 },
        { "removeskill",            COMMAND_LEVEL_M, &ChatHandler::HandleRemoveSkillCommand,                        ".removeskill <skillid> - Removes skill",       NULL, 0, 0, 0 },
        { "increaseweaponskill",    COMMAND_LEVEL_M, &ChatHandler::HandleIncreaseWeaponSkill,                       ".increaseweaponskill <count> - Increase eqipped weapon skill x times (defaults to 1).", NULL, 0, 0, 0 },
        { "playerinfo",             COMMAND_LEVEL_M, &ChatHandler::HandlePlayerInfo,                                ".playerinfo - Displays informations about the selected character (account...)", NULL, 0, 0, 0 },

        { "modify",                 COMMAND_LEVEL_M, NULL,                                                          "",                 modifyCommandTable, 0, 0, 0 },
        { "waypoint",               COMMAND_LEVEL_W, NULL,                                                          "",                 waypointCommandTable, 0, 0, 0 },
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
        { "pet",                    COMMAND_LEVEL_M, NULL,                                                          "",                 petCommandTable, 0, 0, 0 },
        { "recall",                 COMMAND_LEVEL_Q, NULL,                                                          "",                 recallCommandTable, 0, 0, 0 },
        { "guild",                  COMMAND_LEVEL_M, NULL,                                                          "",                 GuildCommandTable, 0, 0, 0 },
        { "title",                  COMMAND_LEVEL_M, NULL,                                                          "",                 TitleCommandTable, 0, 0, 0 },
        { "lookup",                 COMMAND_LEVEL_0, NULL,                                                          "",                 lookupCommandTable, 0, 0, 0 },
        { "faction",                COMMAND_LEVEL_0, NULL,                                                          "",                 FactionCommandTable, 0, 0, 0 },

        { "teleport",               COMMAND_LEVEL_Q, &ChatHandler::HandleRecallGoCommand,                           "Port to recalled location",        NULL, 0, 0, 0 },
        { "getpos",                 COMMAND_LEVEL_D, &ChatHandler::HandleGetPosCommand,                             "",                 NULL, 0, 0, 0 },
        { "clearcooldowns",         COMMAND_LEVEL_M, &ChatHandler::HandleClearCooldownsCommand,                     "Clears all cooldowns for your class.", NULL, 0, 0, 0 },
        { "removeauras",            COMMAND_LEVEL_M, &ChatHandler::HandleRemoveAurasCommand,                        "Removes all auras from target",    NULL, 0, 0, 0 },
        { "paralyze",               COMMAND_LEVEL_B, &ChatHandler::HandleParalyzeCommand,                           "Roots/Paralyzes the target.",  NULL, 0, 0, 0 },
        { "unparalyze",             COMMAND_LEVEL_B, &ChatHandler::HandleUnParalyzeCommand,                         "Unroots/Unparalyzes the target.",NULL, 0, 0, 0 },
        { "setmotd",                COMMAND_LEVEL_M, &ChatHandler::HandleSetMotdCommand,                            "Sets MOTD",        NULL, 0, 0, 0 },
        { "gotrig",                 COMMAND_LEVEL_V, &ChatHandler::HandleTriggerCommand,                            "Warps to areatrigger <id>",        NULL, 0, 0, 0 },
        { "reloadtable",            COMMAND_LEVEL_M, &ChatHandler::HandleDBReloadCommand,                           "Reloads some of the database tables", NULL, 0, 0, 0 },
        { "advanceallskills",       COMMAND_LEVEL_M, &ChatHandler::HandleAdvanceAllSkillsCommand,                   "Advances all skills <x> points.", NULL, 0, 0, 0 },
        { "killbyplayer",           COMMAND_LEVEL_F, &ChatHandler::HandleKillByPlayerCommand,                       "Disconnects the player with name <s>.", NULL, 0, 0, 0 },
        { "killbyaccount",          COMMAND_LEVEL_F, &ChatHandler::HandleKillBySessionCommand,                      "Disconnects the session with account name <s>.", NULL, 0, 0, 0 },
        { "castspell",              COMMAND_LEVEL_D, &ChatHandler::HandleCastSpellCommand,                          ".castspell <spellid> - Casts spell on target.", NULL, 0, 0, 0 },
        { "modperiod",              COMMAND_LEVEL_M, &ChatHandler::HandleModPeriodCommand,                          "Changes period of current transporter.", NULL, 0, 0, 0 },
        { "npcfollow",              COMMAND_LEVEL_M, &ChatHandler::HandleNpcFollowCommand,                          "Sets npc to follow you", NULL, 0, 0, 0 },
        { "nullfollow",             COMMAND_LEVEL_M, &ChatHandler::HandleNullFollowCommand,                         "Sets npc to not follow anything", NULL, 0, 0, 0 },
        { "addipban",               COMMAND_LEVEL_M, &ChatHandler::HandleIPBanCommand,                              "Adds an address to the IP ban table: <address>/<mask> <duration> <reason>\n Mask represents a subnet mask, use /32 to ban a single ip.\nDuration should be a number followed by a character representing the calendar subdivision to use (h>hours, d>days, w>weeks, m>months, y>years, default minutes).", NULL, 0, 0, 0 },
        { "delipban",               COMMAND_LEVEL_M, &ChatHandler::HandleIPUnBanCommand,                            "Deletes an address from the IP ban table: <address>", NULL, 0, 0, 0 },
        { "renamechar",             COMMAND_LEVEL_M, &ChatHandler::HandleRenameCommand,                             "Renames character x to y.", NULL, 0, 0, 0 },
        { "forcerenamechar",        COMMAND_LEVEL_M, &ChatHandler::HandleForceRenameCommand,                        "Forces character x to rename his char next login", NULL, 0, 0, 0 },
        { "recustomizechar",        COMMAND_LEVEL_M, &ChatHandler::HandleRecustomizeCharCommand,                    "Flags character x for character recustomization", NULL, 0, 0, 0 },
        { "getstanding",            COMMAND_LEVEL_M, &ChatHandler::HandleGetStandingCommand,                        "Gets standing of faction %u.", NULL, 0, 0, 0 },
        { "setstanding",            COMMAND_LEVEL_M, &ChatHandler::HandleSetStandingCommand,                        "Sets stanging of faction %u.", NULL, 0, 0, 0 },
        { "createarenateam",        COMMAND_LEVEL_G, &ChatHandler::HandleCreateArenaTeamCommands,                   "Creates arena team", NULL, 0, 0, 0 },
        { "whisperblock",           COMMAND_LEVEL_G, &ChatHandler::HandleWhisperBlockCommand,                       "Blocks like .gmon except without the <GM> tag", NULL, 0, 0, 0 },
        { "logcomment",             COMMAND_LEVEL_1, &ChatHandler::HandleGmLogCommentCommand,                       "Adds a comment to the GM log for the admins to read." , NULL , 0 , 0 , 0 },
        { "showitems",              COMMAND_LEVEL_M, &ChatHandler::HandleShowItems,                                 "test for ItemIterator", NULL, 0, 0, 0 },
        { "testlos",                COMMAND_LEVEL_M, &ChatHandler::HandleCollisionTestLOS,                          "tests los", NULL, 0, 0, 0 },
        { "testindoor",             COMMAND_LEVEL_M, &ChatHandler::HandleCollisionTestIndoor,                       "tests indoor", NULL, 0, 0, 0 },
        { "getheight",              COMMAND_LEVEL_M, &ChatHandler::HandleCollisionGetHeight,                        "Gets height", NULL, 0, 0, 0 },
        { "renameallinvalidchars",  COMMAND_LEVEL_Z, &ChatHandler::HandleRenameAllCharacter,                        "Renames all invalid character names", NULL, 0,0, 0 },
        { "removesickness",         COMMAND_LEVEL_M, &ChatHandler::HandleRemoveRessurectionSickessAuraCommand,      "Removes ressurrection sickness from the target", NULL, 0, 0, 0 },
        { "fixscale",               COMMAND_LEVEL_M, &ChatHandler::HandleFixScaleCommand,                           "",                 NULL, 0, 0, 0 },
        { "clearcorpses",           COMMAND_LEVEL_M, &ChatHandler::HandleClearCorpsesCommand,                       "",                 NULL, 0, 0, 0 },
        { "clearbones",             COMMAND_LEVEL_M, &ChatHandler::HandleClearBonesCommand,                         "",                 NULL, 0, 0, 0 },
        { "multimute",              COMMAND_LEVEL_B, &ChatHandler::HandleMultiMuteCommand,                          "mutes multiple , .multimute <reason> <player1> <player2> ...",         NULL, 0, 0, 0 },
        { "multiban",               COMMAND_LEVEL_B, &ChatHandler::HandleMultiBanCommand,                           "bans multiple , .multimute <reason> <player1> <player2> ...",          NULL, 0, 0, 0 },
        { "multiaccountban",        COMMAND_LEVEL_B, &ChatHandler::HandleMultiAccountBanCommand,                    "account bans multiple , .multimute <reason> <player1> <player2> ...",  NULL, 0, 0, 0 },
        { "multikick",              COMMAND_LEVEL_B, &ChatHandler::HandleMultiKickCommand,                          "kicks multiple , .multimute <reason> <player1> <player2> ...",         NULL, 0, 0, 0 },
        { NULL,                     COMMAND_LEVEL_0, NULL,                                                          "",                 NULL, 0, 0, 0 }
    };
    dupe_command_table(commandTable, _commandTable);

    /* set the correct pointers */
    ChatCommand * p = &_commandTable[0];
    while(p->Name != 0)
    {
        if(p->ChildCommands != 0)
        {
            // Set the correct pointer.
            ChatCommand * np = GetSubCommandTable(p->Name);
            ASSERT(np);
            p->ChildCommands = np;
        }
        ++p;
    }
}

/*struct SpecStruct
{
    std::map<uint32, uint8> talents;    // map of <talentId, talentRank>
    uint16 glyphs[GLYPHS_COUNT];
};

bool ChatHandler::xxx(const char* args, WorldSession *m_session)
{
    char * end;
    char * start;
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM characters_extra");

    do
    {
        std::set<uint32> m_spells;
        SpecStruct m_spec[2];

        Field *fields = result->Fetch();
        uint32 i = 1;

        // Load Spells from CSV data.
        start = (char*)fields[i++].GetString();//buff;
        SpellEntry * spProto;
        while(true)
        {
            end = strchr(start,',');
            if(!end)break;
            *end = 0;
            //mSpells.insert(atol(start));
            spProto = dbcSpell.LookupEntry(atol(start));

            if(spProto)
                m_spells.insert(spProto->Id);

            start = end +1;
        }

        for( uint8 s = 0; s < MAX_SPEC_COUNT; ++s )
        {
            start = (char*)fields[i++].GetString();
            uint8 glyphid = 0;
            while(glyphid < GLYPHS_COUNT)
            {
                end = strchr(start,',');
                if(!end)break;
                *end= 0;
                m_spec[s].glyphs[glyphid] = (uint16)atol(start);
                ++glyphid;
                start = end + 1;
            }

            //Load talents for spec
            start = (char*)fields[i++].GetString();
            while(end != NULL)
            {
                end = strchr(start,',');
                if(!end)
                    break;
                *end= 0;
                uint32 talentid = atol(start);
                start = end + 1;

                end = strchr(start,',');
                if(!end)
                    break;
                *end = 0;
                uint8 rank = (uint8)atol(start);
                start = end + 1;

                m_spec[s].talents.insert(make_pair<uint32, uint8>(talentid, rank));
            }
        }

        std::stringstream ss;
        ss << "INSERT INTO playerspells (guid, spellid) VALUES ";
        SpellSet::iterator spellItr = m_spells.begin();
        bool first = true;
        for(; spellItr != m_spells.end(); ++spellItr)
        {
            SpellEntry * sp = dbcSpell.LookupEntry( *spellItr );
            if( !sp)
                continue;

            if(!first)
                ss << ",";
            else
                first = false;

            ss << "("<< fields[0].GetUInt32() << "," << uint32(*spellItr) << ")";
        }
        CharacterDatabase.Execute(ss.str().c_str());

        for(uint8 s = 0; s < 2; s++)
        {
            std::map<uint32, uint8> *talents = &m_spec[s].talents;
            std::map<uint32, uint8>::iterator itr;
            for(itr = talents->begin(); itr != talents->end(); itr++)
            {
                std::stringstream ss;
                ss << "INSERT INTO playertalents (guid, spec, tid, rank) VALUES "
                    << "(" << fields[0].GetUInt32() << ","
                    << uint32(s) << ","
                    << itr->first << ","
                    << uint32(itr->second) << ")";

                CharacterDatabase.Execute(ss.str().c_str());
            }
        }

    }while(result->NextRow());

    RedSystemMessage(m_session, "No values specified.");
    return true;
}*/

ChatHandler::ChatHandler()
{
    new CommandTableStorage;
    sComTableStore.Init();
    SkillNameManager = new SkillNameMgr;
}

ChatHandler::~ChatHandler()
{
    sComTableStore.Dealloc();
    delete CommandTableStorage::getSingletonPtr();
    delete SkillNameManager;
}

bool ChatHandler::hasStringAbbr(const char* s1, const char* s2)
{
    for(;;)
    {
        if( !*s2 )
            return true;
        else if( !*s1 )
            return false;
        else if( tolower( *s1 ) != tolower( *s2 ) )
            return false;
        s1++; s2++;
    }
}

void ChatHandler::SendMultilineMessage(WorldSession *m_session, const char *str)
{
    char * start = (char*)str, *end;
    for(;;)
    {
        end = strchr(start, '\n');
        if(!end)
            break;

        *end = '\0';
        SystemMessage(m_session, start);
        start = end + 1;
    }
    if(*start != '\0')
        SystemMessage(m_session, start);
}

bool ChatHandler::ExecuteCommandInTable(ChatCommand *table, const char* text, WorldSession *m_session)
{
    std::string cmd = "";

    // get command
    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        text++;
    }

    while (*text == ' ') text++; // skip whitespace

    if(!cmd.length())
        return false;

    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(!hasStringAbbr(table[i].Name, cmd.c_str()))
            continue;

        if(table[i].CommandGroup != '0' )
        {
            if(sWorld.NumericCommandGroups)
            {
                if(!m_session->CanUseCommand(table[i].NumericCGroup))
                    continue;
            }
            else
            {
                if(!m_session->CanUseCommand(table[i].CommandGroup))
                    continue;
            }
        }

        if(table[i].ChildCommands != NULL)
        {
            if(!ExecuteCommandInTable(table[i].ChildCommands, text, m_session))
            {
                if(table[i].Help != "")
                    SendMultilineMessage(m_session, table[i].Help.c_str());
                else
                {
                    GreenSystemMessage(m_session, "Available Subcommands:");
                    for(uint32 k=0; table[i].ChildCommands[k].Name;k++)
                    {
                        if(table[i].ChildCommands[k].CommandGroup != '0' && m_session->CanUseCommand(table[i].ChildCommands[k].CommandGroup))
                            BlueSystemMessage(m_session, " %s - %s", table[i].ChildCommands[k].Name, table[i].ChildCommands[k].Help.size() ? table[i].ChildCommands[k].Help.c_str() : "No Help Available");
                    }
                }
            }

            return true;
        }

        // Check for field-based commands
        if(table[i].Handler == NULL && (table[i].MaxValueField || table[i].NormalValueField))
        {
            bool result = false;
            if(strlen(text) == 0)
            {
                RedSystemMessage(m_session, "No values specified.");
            }
            if(table[i].ValueType == 2)
                result = CmdSetFloatField(m_session, table[i].NormalValueField, table[i].MaxValueField, table[i].Name, text);
            else if(table[i].ValueType == 3)
            {
                result = true;
                GameObject* go = NULL;
                if((go = m_session->GetPlayer()->m_GM_SelectedGO) != NULL)
                {
                    go->SetUInt32Value(table[i].NormalValueField, atoi(text));
                    go->SaveToDB();
                }
                else
                    result = false;
            }
            else
                result = CmdSetValueField(m_session, table[i].NormalValueField, table[i].MaxValueField, table[i].Name, text);

            if(!result)
                RedSystemMessage(m_session, "Must be in the form of (command) <value>, or, (command) <value> <maxvalue>");
        }
        else
        {
            if(!(this->*(table[i].Handler))(text, m_session))
            {
                if(table[i].Help != "")
                    SendMultilineMessage(m_session, table[i].Help.c_str());
                else
                {
                    RedSystemMessage(m_session, "Incorrect syntax specified. Try .help %s for the correct syntax.", table[i].Name);
                }
            }
        }

        return true;
    }
    return false;
}

int ChatHandler::ParseCommands(const char* text, WorldSession *m_session)
{
    if (!m_session)
        return 0;

    if(!*text)
        return 0;

    if(m_session->GetPermissionCount() == 0 && sWorld.m_reqGmForCommands)
        return 0;

    if(text[0] != '.') // let's not confuse users
        return 0;

    //skip '..' :P that pisses me off
    if(text[1] == '.')
        return 0;

    text++;
    if(!ExecuteCommandInTable(sComTableStore.Get(), text, m_session))
        SystemMessage(m_session, "There is no such command, or you do not have access to it.");
    return 1;
}

WorldPacket * ChatHandler::FillMessageData( uint32 type, int32 language, const char *message, uint64 guid, uint8 flag) const
{
    ASSERT(type != CHAT_MSG_CHANNEL);
    //channels are handled in channel handler and so on
    uint32 messageLength = (uint32)strlen((char*)message) + 1;

    WorldPacket *data = new WorldPacket(SMSG_MESSAGECHAT, messageLength + 30);

    *data << (uint8)type;
    *data << language;

    *data << guid;
    *data << uint32(0);

    *data << guid;

    *data << messageLength;
    *data << message;

    *data << uint8(flag);
    return data;
}

WorldPacket* ChatHandler::FillSystemMessageData(const char *message) const
{
    uint32 messageLength = (uint32)strlen((char*)message) + 1;

    WorldPacket * data = new WorldPacket(SMSG_MESSAGECHAT, 30 + messageLength);
    *data << (uint8)CHAT_MSG_SYSTEM;
    *data << (uint32)LANG_UNIVERSAL;

    *data << (uint64)0; // Who cares about guid when there's no nickname displayed heh ?
    *data << (uint32)0;
    *data << (uint64)0;

    *data << messageLength;
    *data << message;

    *data << uint8(0);

    return data;
}

Player* ChatHandler::getSelectedChar(WorldSession *m_session, bool showerror)
{
    Player* chr = NULLPLR;
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        if(showerror)
            GreenSystemMessage(m_session, "Auto-targeting self.");
        chr = m_session->GetPlayer(); // autoselect
    }
    else
        chr = m_session->GetPlayer()->GetMapMgr()->GetPlayer(GUID_LOPART(guid));

    if(chr == NULL && showerror)
        RedSystemMessage(m_session, "This command requires that you select a player.");
    return chr;
}

Creature* ChatHandler::getSelectedCreature(WorldSession *m_session, bool showerror)
{
    if(!m_session->GetPlayer()->IsInWorld())
        return NULLCREATURE;

    Creature* creature = NULLCREATURE;
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if(GUID_HIPART(guid) == HIGHGUID_TYPE_PET)
        creature = m_session->GetPlayer()->GetMapMgr()->GetPet( GUID_LOPART(guid) );
    else if(GUID_HIPART(guid) == HIGHGUID_TYPE_CREATURE)
        creature = m_session->GetPlayer()->GetMapMgr()->GetCreature( GUID_LOPART(guid) );
    else if(GUID_HIPART(guid) == HIGHGUID_TYPE_VEHICLE)
        creature = m_session->GetPlayer()->GetMapMgr()->GetVehicle( GUID_LOPART(guid) );

    if(creature == NULLCREATURE && showerror)
        RedSystemMessage(m_session, "This command requires that you select a creature.");
    return creature;
}

Unit* ChatHandler::getSelectedUnit(WorldSession *m_session, bool showerror)
{
    if(!m_session->GetPlayer()->IsInWorld())
        return NULLUNIT;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    Unit* unit = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid);
    if(unit == NULLUNIT && showerror)
        RedSystemMessage(m_session, "This command requires that you select a unit.");
    return unit;
}

void ChatHandler::SystemMessage(WorldSession *m_session, const char* message, ...)
{
    if( !message )
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    WorldPacket * data = FillSystemMessageData(msg1);
    if(m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

void ChatHandler::ColorSystemMessage(WorldSession *m_session, const char* colorcode, const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", colorcode, msg1);
    WorldPacket * data = FillSystemMessageData(msg);
    if(m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

void ChatHandler::RedSystemMessage(WorldSession *m_session, const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024,message,ap);
    char msg[1024];
    snprintf(msg, 1024,"%s%s|r", MSG_COLOR_LIGHTRED/*MSG_COLOR_RED*/, msg1);
    WorldPacket * data = FillSystemMessageData(msg);
    if(m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

void ChatHandler::GreenSystemMessage(WorldSession *m_session, const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", MSG_COLOR_GREEN, msg1);
    WorldPacket * data = FillSystemMessageData(msg);
    if(m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

void ChatHandler::WhiteSystemMessage(WorldSession *m_session, const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", MSG_COLOR_WHITE, msg1);
    WorldPacket * data = FillSystemMessageData(msg);
    if(m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

void ChatHandler::BlueSystemMessage(WorldSession *m_session, const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    char msg[1024];
    snprintf(msg, 1024,"%s%s|r", MSG_COLOR_LIGHTBLUE, msg1);
    WorldPacket * data = FillSystemMessageData(msg);
    if(m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

void ChatHandler::RedSystemMessageToPlr(Player* plr, const char *message, ...)
{
    if( !message || !plr->GetSession() ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024,message,ap);
    RedSystemMessage(plr->GetSession(), (const char*)msg1);
}

void ChatHandler::GreenSystemMessageToPlr(Player* plr, const char *message, ...)
{
    if( !message || !plr->GetSession() ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024,message,ap);
    GreenSystemMessage(plr->GetSession(), (const char*)msg1);
}

void ChatHandler::BlueSystemMessageToPlr(Player* plr, const char *message, ...)
{
    if( !message || !plr->GetSession() ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024,message,ap);
    BlueSystemMessage(plr->GetSession(), (const char*)msg1);
}

void ChatHandler::SystemMessageToPlr(Player* plr, const char* message, ...)
{
    if( !message || !plr->GetSession() ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024,message,ap);
    SystemMessage(plr->GetSession(), msg1);
}

bool ChatHandler::CmdSetValueField(WorldSession *m_session, uint32 field, uint32 fieldmax, const char *fieldname, const char *args)
{
    if(!args)
        return false;

    char* pvalue = strtok((char*)args, " ");
    uint32 mv, av;

    if (pvalue)
        av = atol(pvalue);
    else
        return false;

    if(fieldmax)
    {
        char* pvaluemax = strtok(NULL, " ");
        if (!pvaluemax)
            return false;
        else
            mv = atol(pvaluemax);
    }
    else
        mv = 0;

    //valid UNIT_FIELD?
    if(field <= OBJECT_END || field > UNIT_END )
    {
        RedSystemMessage(m_session, "Specified field is not valid.");
        return true;
    }
    if (av <= 0)
    {
        RedSystemMessage(m_session, "Values are invalid. Value must be > 0.");
        return true;
    }
    if(fieldmax && (mv < av || mv <= 0))
    {
        RedSystemMessage(m_session, "Values are invalid. Max value must be >= new value.");
        return true;
    }

    Player* plr = getSelectedChar(m_session, false);
    if(plr!=NULL)
    {
        sWorld.LogGM(m_session, "used modify field value: %s, %u on %s", fieldname, av, plr->GetName());
        if(fieldmax)
        {
            BlueSystemMessage(m_session, "You set the %s of %s to %d/%d.", fieldname, plr->GetName(), av, mv);
            GreenSystemMessageToPlr(plr, "%s set your %s to %d/%d.", m_session->GetPlayer()->GetName(), fieldname, av, mv);
        }
        else
        {
            BlueSystemMessage(m_session, "You set the %s of %s to %d.", fieldname, plr->GetName(), av);
            GreenSystemMessageToPlr(plr, "%s set your %s to %d.", m_session->GetPlayer()->GetName(), fieldname, av);
        }

        if(field == UNIT_FIELD_STAT1) av /= 2;
        if(field == UNIT_FIELD_BASE_HEALTH)
            plr->SetUInt32Value(UNIT_FIELD_HEALTH, av);

        plr->SetUInt32Value(field, av);

        if(fieldmax)
            plr->SetUInt32Value(fieldmax, mv);
    }
    else
    {
        Creature* cr = getSelectedCreature(m_session, false);
        if(cr)
        {
            std::string creaturename = cr->GetCreatureInfo() ? cr->GetCreatureInfo()->Name : "Unknown Being";

            sWorld.LogGM(m_session, "used modify field value: [creature]%s, %u on %s", fieldname, av, creaturename.c_str());

            switch(field)
            {
            case UNIT_FIELD_STAT1:
                {
                    av /= 2;
                }break;
            case UNIT_FIELD_BASE_HEALTH:
                {
                    cr->SetUInt32Value(UNIT_FIELD_HEALTH, av);
                }break;
            }

            cr->SetUInt32Value(field, av);

            if(fieldmax)
            {
                cr->SetUInt32Value(fieldmax, mv);
                BlueSystemMessage(m_session, "Setting %s of %s to %u/%u.", fieldname, creaturename.c_str(), av, mv);
            }
            else
                BlueSystemMessage(m_session, "Setting %s of %s to %u.", fieldname, creaturename.c_str(), av);

            if(m_session->CanUseCommand('z'))
                cr->SaveToDB();
        }
        else
        {
            RedSystemMessage(m_session, "Invalid Selection.");
        }
    }
    return true;
}

bool ChatHandler::CmdSetFloatField(WorldSession *m_session, uint32 field, uint32 fieldmax, const char *fieldname, const char *args)
{
    char* pvalue = strtok((char*)args, " ");
    float mv, av;

    if (!pvalue)
        return false;
    else
        av = (float)atof(pvalue);

    if(fieldmax)
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
        RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
        return true;
    }
    if(fieldmax)
    {
        if(mv < av || mv <= 0)
        {
            RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
            return true;
        }
    }

    Player* plr = getSelectedChar(m_session, false);
    if(plr != NULL)
    {
        sWorld.LogGM(m_session, "used modify field value: %s, %f on %s", fieldname, av, plr->GetName());

        if(fieldmax)
        {
            BlueSystemMessage(m_session, "You set the %s of %s to %.1f/%.1f.", fieldname, plr->GetName(), av, mv);
            GreenSystemMessageToPlr(plr, "%s set your %s to %.1f/%.1f.", m_session->GetPlayer()->GetName(), fieldname, av, mv);
        }
        else
        {
            BlueSystemMessage(m_session, "You set the %s of %s to %.1f.", fieldname, plr->GetName(), av);
            GreenSystemMessageToPlr(plr, "%s set your %s to %.1f.", m_session->GetPlayer()->GetName(), fieldname, av);
        }
        plr->SetFloatValue(field, av);
        if(fieldmax) plr->SetFloatValue(fieldmax, mv);
    }
    else
    {
        Creature* cr = getSelectedCreature(m_session, false);
        if(cr)
        {
            if(!(field < UNIT_END && fieldmax < UNIT_END)) return false;
            std::string creaturename = "Unknown Being";
            if(cr->GetCreatureInfo())
                creaturename = cr->GetCreatureInfo()->Name;
            if(fieldmax)
                BlueSystemMessage(m_session, "Setting %s of %s to %.1f/%.1f.", fieldname, creaturename.c_str(), av, mv);
            else
                BlueSystemMessage(m_session, "Setting %s of %s to %.1f.", fieldname, creaturename.c_str(), av);
            cr->SetFloatValue(field, av);
            sWorld.LogGM(m_session, "used modify field value: [creature]%s, %f on %s", fieldname, av, creaturename.c_str());
            if(fieldmax)
                cr->SetFloatValue(fieldmax, mv);
        }
        else
        {
            RedSystemMessage(m_session, "Invalid Selection.");
        }
    }
    return true;
}

bool ChatHandler::HandleGetPosCommand(const char* args, WorldSession *m_session)
{
    Creature* creature = getSelectedCreature(m_session);
    if(!creature) 
        return false;
    BlueSystemMessage(m_session, "Creature Position: \nX: %f\nY: %f\nZ: %f\n", creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ());
    return true;
}


bool ChatHandler::HandleDebugRetroactiveQuestAchievements(const char *args, WorldSession *m_session)
{
    Player* pTarget = getSelectedChar(m_session, true );
    if(!pTarget) return true;

    pTarget->RetroactiveCompleteQuests();
    m_session->GetPlayer()->BroadcastMessage("Done.");
    return true;
}

bool ChatHandler::HandleModifyFactionCommand(const char *args, WorldSession *m_session)
{
    Player* player = m_session->GetPlayer();
    if(player == NULL)
        return true;

    Unit* unit = getSelectedUnit(m_session, false);

    if(unit == NULL)
        unit = player;

    uint32 faction = atol(args);
    if(!faction && unit->IsCreature())
        faction = TO_CREATURE(unit)->GetProto()->Faction;

    BlueSystemMessage(m_session, "Set target's faction to %u", faction);

    unit->SetFaction(faction, m_session->CanUseCommand('z'));
    return true;
}

bool ChatHandler::HandleModifyScaleCommand(const char *args, WorldSession *m_session)
{
    Player* player = m_session->GetPlayer();
    if(player == NULL)
        return true;

    Unit* unit = getSelectedUnit(m_session, false);
    if(unit == NULL)
        unit = player;

    float scale = atof(args);
    int save = m_session->CanUseCommand('z') ? 1 : 0;
    if(sscanf(args, "%f %i", &scale, &save) < 1)
        return false;

    if(scale > 255 || scale < 0)
        return false;

    if(!scale && unit->IsCreature())
        scale = TO_CREATURE(unit)->GetProto() ? TO_CREATURE(unit)->GetProto()->Scale : 1.0f;

    BlueSystemMessage(m_session, "Set target's scale to %f", scale);
    unit->SetFloatValue(OBJECT_FIELD_SCALE_X, scale);
    if(unit->IsCreature() && (save > 0))
        TO_CREATURE(unit)->SaveToDB();

    return true;
}

uint16 ChatHandler::GetItemIDFromLink(const char* link, uint32* itemid)
{
    if(link == NULL)
    {
        *itemid = 0;
        return 0;
    }

    uint16 slen = (uint16)strlen(link);
    const char* ptr = strstr(link, "|Hitem:");
    if(ptr == NULL)
    {
        *itemid = 0;
        return slen;
    }

    ptr += 7;
    *itemid = atoi(ptr);
    
    ptr = strstr(link, "|r");
    if(ptr == NULL)
    {
        *itemid = 0;
        return slen;
    }

    ptr += 2;
    return (ptr-link) & 0x0000ffff;
}

int32 ChatHandler::GetSpellIDFromLink(const char* link)
{
    if(link== NULL)
        return 0;

    const char* ptr = strstr(link, "|Hspell:");
    if(ptr == NULL)
        return 0;

    return atol(ptr+8);
}