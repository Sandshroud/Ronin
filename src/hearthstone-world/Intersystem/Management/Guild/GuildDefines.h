/***
 * Demonstrike Core
 */

#pragma once

class Charter;

#define MAX_GUILD_RANKS 10
#define MAX_GUILD_MEMBERS 500

enum PETITION_TURNIN_ERRORS
{
    ERR_PETITION_OK,
    ERR_PETITION_ALREADY_SIGNED,
    ERR_PETITION_IN_GUILD,
    ERR_PETITION_CREATOR,
    ERR_PETITION_NOT_ENOUGH_SIGNATURES,
};

enum GUILDEMBLEM_ERRORS
{
    ERR_GUILDEMBLEM_SUCCESS,
    ERR_GUILDEMBLEM_INVALID_TABARD_COLORS,
    ERR_GUILDEMBLEM_NOGUILD,
    ERR_GUILDEMBLEM_NOTGUILDMASTER,
    ERR_GUILDEMBLEM_NOTENOUGHMONEY,
    ERR_GUILDEMBLEM_INVALIDVENDOR,
};

enum GuildMessageTypes
{
    G_MSGTYPE_ALL,
    G_MSGTYPE_ALLBUTONE,
    G_MSGTYPE_PUBLICCHAT,
    G_MSGTYPE_OFFICERCHAT,
};

enum MONEY
{
    MONEY_ONE_COPPER            = 1,
    MONEY_ONE_SILVER            = MONEY_ONE_COPPER * 100,
    MONEY_ONE_GOLD              = MONEY_ONE_SILVER * 100,
};

enum CommandErrors
{
    GUILD_U_HAVE_INVITED            = 0x00,
    GUILD_INTERNAL                  = 0x01,
    GUILD_ALREADY_IN_GUILD          = 0x02,
    ALREADY_IN_GUILD                = 0x03,
    INVITED_TO_GUILD                = 0x04,
    ALREADY_INVITED_TO_GUILD        = 0x05,
    GUILD_NAME_INVALID              = 0x06,
    GUILD_NAME_EXISTS               = 0x07,
    GUILD_LEADER_LEAVE              = 0x08,
    GUILD_PERMISSIONS               = 0x08,
    GUILD_PLAYER_NOT_IN_GUILD       = 0x09,
    GUILD_PLAYER_NOT_IN_GUILD_S     = 0x0A,
    GUILD_PLAYER_NOT_FOUND          = 0x0B,
    GUILD_NOT_ALLIED                = 0x0C,
};

//for uint32(0)<<name<<code
enum GUILD_COMMAND_RESULTS
{
    C_R_CREATED                             = 1,
    C_R_ALREADY_IN_GUILD                    = 2,
    C_R_TARGET_ALREADY_IN_GUILD             = 3,
    C_R_ALREADY_INVITED_TO_GUILD            = 4,
    C_R_TARGET_ALREADY_INVITED_TO_GUILD     = 5,
    C_R_GUILDNAME_HAS_INVALID_CHARACTERS    = 6,
    C_R_GUILD_NAME_EXISTS                   = 7,
    C_R_DONT_HAVE_PERMISSION                = 8,
    C_R_NOT_IN_GUILD                        = 9,
    C_R_TARGET_IS_NOT_IN_YOUR_GUILD         = 10,
    C_R_NAME_NOT_FOUND                      = 11,
    C_R_CANT_INVITE_PLYRS_FROM_OPP_ALLIANCE = 12,
    C_R_NAME_RANK_TOO_HIGH                  = 13,
    C_R_NAME_RANK_AT_LOWEST_RANK            = 14,
};

enum typecommand
{
    GUILD_CREATE_S                  = 0x00,
    GUILD_INVITE_S                  = 0x01,
    GUILD_QUIT_S                    = 0x02,
    GUILD_PROMOTE_S                 = 0x03,
    GUILD_FOUNDER_S                 = 0x0C,
    GUILD_MEMBER_S                  = 0x0D,
    GUILD_PUBLIC_NOTE_CHANGED_S     = 0x13,
    GUILD_OFFICER_NOTE_CHANGED_S    = 0x14,
};

enum GuildRankRights
{
    GR_RIGHT_GCHATLISTEN        = 0x00000001,
    GR_RIGHT_GCHATSPEAK         = 0x00000002,
    GR_RIGHT_OFFCHATLISTEN      = 0x00000004,
    GR_RIGHT_OFFCHATSPEAK       = 0x00000008,
    GR_RIGHT_INVITE             = 0x00000010,
    GR_RIGHT_REMOVE             = 0x00000020,
    GR_RIGHT_EMPTY              = 0x00000040,
    GR_RIGHT_PROMOTE            = 0x00000080,
    GR_RIGHT_DEMOTE             = 0x00000100,
    // unknown 0x00200
    // unknown 0x00400
    // unknown 0x00800
    GR_RIGHT_SETMOTD            = 0x00001000,
    GR_RIGHT_EPNOTE             = 0x00002000,
    GR_RIGHT_VIEWOFFNOTE        = 0x00004000,
    GR_RIGHT_EOFFNOTE           = 0x00008000,
    GR_RIGHT_EGUILDINFO         = 0x00010000,
    GR_RIGHT_REPAIR_FROM_GUILD  = 0x00020000,
    GR_RIGHT_WITHDRAW_REPAIR    = 0x00040000,
    GR_RIGHT_WITHDRAW_GOLD      = 0x00080000,
    GR_RIGHT_CREATE_GUILD_EVENT = 0x00100000,
    GR_RIGHT_ALL                = 0x001FF1FF,

    GR_RIGHT_DEFAULT            = GR_RIGHT_EMPTY | GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK,

    GR_RIGHT_GUILD_BANK_VIEW_TAB        = 0x01,
    GR_RIGHT_GUILD_BANK_DEPOSIT_ITEMS   = 0x02,
    GR_RIGHT_GUILD_BANK_CHANGE_TABTXT   = 0x04,
    GR_RIGHT_GUILD_BANK_ALL = GR_RIGHT_GUILD_BANK_VIEW_TAB | GR_RIGHT_GUILD_BANK_DEPOSIT_ITEMS | GR_RIGHT_GUILD_BANK_CHANGE_TABTXT,
};

enum GuildEvent
{
    GUILD_EVENT_PROMOTION           = 0x0,
    GUILD_EVENT_DEMOTION            = 0x1,
    GUILD_EVENT_MOTD                = 0x2,
    GUILD_EVENT_JOINED              = 0x3,
    GUILD_EVENT_LEFT                = 0x4,
    GUILD_EVENT_REMOVED             = 0x5,
    GUILD_EVENT_LEADER_IS           = 0x6,
    GUILD_EVENT_LEADER_CHANGED      = 0x7,
    GUILD_EVENT_DISBANDED           = 0x8,
    GUILD_EVENT_TABARDCHANGE        = 0x9,
    GUILD_EVENT_UNK1                = 0xA,
    GUILD_EVENT_UNK2                = 0xB,
    GUILD_EVENT_HASCOMEONLINE       = 0xC,
    GUILD_EVENT_HASGONEOFFLINE      = 0xD,
    GUILD_EVENT_BANKTABBOUGHT       = 0xF,
    GUILD_EVENT_SETNEWBALANCE       = 0x11,
    GUILD_EVENT_TABINFO             = 0x13,
};

enum GuildLogEventE
{
    GUILD_LOG_EVENT_INVITE      = 1,
    GUILD_LOG_EVENT_JOIN        = 2,
    GUILD_LOG_EVENT_PROMOTION   = 3,
    GUILD_LOG_EVENT_DEMOTION    = 4,
    GUILD_LOG_EVENT_REMOVAL     = 5,
    GUILD_LOG_EVENT_LEFT        = 6,
};

enum GuildBankLogEvents
{
    GUILD_BANK_LOG_EVENT_DEPOSIT_ITEM   = 1,
    GUILD_BANK_LOG_EVENT_WITHDRAW_ITEM  = 2,
    GUILD_BANK_LOG_EVENT_DEPOSIT_MONEY  = 4,
    GUILD_BANK_LOG_EVENT_WITHDRAW_MONEY = 5,
    GUILD_BANK_LOG_EVENT_REPAIR         = 6,
};

enum GuildStatuses
{
    GUILD_STATUS_JUST_LOADED = 1,
    GUILD_STATUS_NORMAL = 2,
    GUILD_STATUS_DIRTY = 3,
    GUILD_STATUS_NEW = 4,
    GUILD_STATUS_DISBANDED = 5,
};

#define ITEM_ENTRY_GUILD_CHARTER        5863
#define ARENA_TEAM_CHARTER_2v2          23560
#define ARENA_TEAM_CHARTER_2v2_COST     800000  // 80 G
#define ARENA_TEAM_CHARTER_3v3          23561
#define ARENA_TEAM_CHARTER_3v3_COST     1200000 // 120 G
#define ARENA_TEAM_CHARTER_5v5          23562
#define ARENA_TEAM_CHARTER_5v5_COST     2000000 // 200 G

#define MAX_GUILD_BANK_SLOTS 98
#define MAX_GUILD_BANK_TABS 6

void SendGuildCommandResult(WorldSession* pClient, uint32 iCmd, const char* szMsg, uint32 iType);
void SendTurnInPetitionResult( WorldSession * pClient, uint32 result );
