/***
 * Demonstrike Core
 */

#pragma once

// Auto generated for version 4.0.6a, 13623

enum EObjectFields
{
    OBJECT_FIELD_GUID                         = 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    OBJECT_FIELD_TYPE                         = 0x0002, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    OBJECT_FIELD_ENTRY                        = 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    OBJECT_FIELD_SCALE_X                      = 0x0004, // Size: 1, Type: FLOAT, Flags: PUBLIC
    OBJECT_FIELD_DATA                         = 0x0005, // Size: 2, Type: LONG, Flags: PUBLIC
    OBJECT_FIELD_PADDING                      = 0x0007, // Size: 1, Type: INT, Flags: NONE
    OBJECT_END                                = 0x0008,
};

enum EItemFields
{
    ITEM_FIELD_OWNER                          = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_CONTAINED                      = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_CREATOR                        = OBJECT_END + 0x0004, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_GIFTCREATOR                    = OBJECT_END + 0x0006, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_STACK_COUNT                    = OBJECT_END + 0x0008, // Size: 1, Type: INT, Flags: OWNER, UNUSED1
    ITEM_FIELD_DURATION                       = OBJECT_END + 0x0009, // Size: 1, Type: INT, Flags: OWNER, UNUSED1
    ITEM_FIELD_SPELL_CHARGES                  = OBJECT_END + 0x000A, // Size: 5, Type: INT, Flags: OWNER, UNUSED1
    ITEM_FIELD_FLAGS                          = OBJECT_END + 0x000F, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_1_1                = OBJECT_END + 0x0010, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_1_3                = OBJECT_END + 0x0012, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_2_1                = OBJECT_END + 0x0013, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_2_3                = OBJECT_END + 0x0015, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_3_1                = OBJECT_END + 0x0016, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_3_3                = OBJECT_END + 0x0018, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_4_1                = OBJECT_END + 0x0019, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_4_3                = OBJECT_END + 0x001B, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_5_1                = OBJECT_END + 0x001C, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_5_3                = OBJECT_END + 0x001E, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_6_1                = OBJECT_END + 0x001F, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_6_3                = OBJECT_END + 0x0021, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_7_1                = OBJECT_END + 0x0022, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_7_3                = OBJECT_END + 0x0024, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_8_1                = OBJECT_END + 0x0025, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_8_3                = OBJECT_END + 0x0027, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_9_1                = OBJECT_END + 0x0028, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_9_3                = OBJECT_END + 0x002A, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_10_1               = OBJECT_END + 0x002B, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_10_3               = OBJECT_END + 0x002D, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_11_1               = OBJECT_END + 0x002E, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_11_3               = OBJECT_END + 0x0030, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_12_1               = OBJECT_END + 0x0031, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_12_3               = OBJECT_END + 0x0033, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_13_1               = OBJECT_END + 0x0034, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_13_3               = OBJECT_END + 0x0036, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_14_1               = OBJECT_END + 0x0037, // Size: 2, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_14_3               = OBJECT_END + 0x0039, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    ITEM_FIELD_PROPERTY_SEED                  = OBJECT_END + 0x003A, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_RANDOM_PROPERTIES_ID           = OBJECT_END + 0x003B, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_DURABILITY                     = OBJECT_END + 0x003C, // Size: 1, Type: INT, Flags: OWNER, UNUSED1
    ITEM_FIELD_MAXDURABILITY                  = OBJECT_END + 0x003D, // Size: 1, Type: INT, Flags: OWNER, UNUSED1
    ITEM_FIELD_CREATE_PLAYED_TIME             = OBJECT_END + 0x003E, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_PAD                            = OBJECT_END + 0x003F, // Size: 1, Type: INT, Flags: NONE
    ITEM_END                                  = OBJECT_END + 0x0040,
};

enum EContainerFields
{
    CONTAINER_FIELD_NUM_SLOTS                 = ITEM_END + 0x0000, // Size: 1, Type: INT, Flags: PUBLIC
    CONTAINER_ALIGN_PAD                       = ITEM_END + 0x0001, // Size: 1, Type: BYTES, Flags: NONE
    CONTAINER_FIELD_SLOT_1                    = ITEM_END + 0x0002, // Size: 72, Type: LONG, Flags: PUBLIC
    CONTAINER_END                             = ITEM_END + 0x004A,
};

enum EUnitFields
{
    UNIT_FIELD_CHARM                          = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_SUMMON                         = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CRITTER                        = OBJECT_END + 0x0004, // Size: 2, Type: LONG, Flags: PRIVATE
    UNIT_FIELD_CHARMEDBY                      = OBJECT_END + 0x0006, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_SUMMONEDBY                     = OBJECT_END + 0x0008, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CREATEDBY                      = OBJECT_END + 0x000A, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_TARGET                         = OBJECT_END + 0x000C, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CHANNEL_OBJECT                 = OBJECT_END + 0x000E, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_CHANNEL_SPELL                        = OBJECT_END + 0x0010, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BYTES_0                        = OBJECT_END + 0x0011, // Size: 1, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_HEALTH                         = OBJECT_END + 0x0012, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER                          = OBJECT_END + 0x0013, // Size: 11, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXHEALTH                      = OBJECT_END + 0x001E, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWER                       = OBJECT_END + 0x001F, // Size: 11, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER      = OBJECT_END + 0x002A, // Size: 11, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER = OBJECT_END + 0x0035, // Size: 11, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_LEVEL                          = OBJECT_END + 0x0040, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_FACTIONTEMPLATE                = OBJECT_END + 0x0041, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_VIRTUAL_ITEM_SLOT_ID                 = OBJECT_END + 0x0042, // Size: 3, Type: INT, Flags: PUBLIC
    UNIT_FIELD_FLAGS                          = OBJECT_END + 0x0045, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_FLAGS_2                        = OBJECT_END + 0x0046, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_AURASTATE                      = OBJECT_END + 0x0047, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BASEATTACKTIME                 = OBJECT_END + 0x0048, // Size: 2, Type: INT, Flags: PUBLIC
    UNIT_FIELD_RANGEDATTACKTIME               = OBJECT_END + 0x004A, // Size: 1, Type: INT, Flags: PRIVATE
    UNIT_FIELD_BOUNDINGRADIUS                 = OBJECT_END + 0x004B, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_COMBATREACH                    = OBJECT_END + 0x004C, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_DISPLAYID                      = OBJECT_END + 0x004D, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_NATIVEDISPLAYID                = OBJECT_END + 0x004E, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MOUNTDISPLAYID                 = OBJECT_END + 0x004F, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MINDAMAGE                      = OBJECT_END + 0x0050, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, ITEM_OWNER
    UNIT_FIELD_MAXDAMAGE                      = OBJECT_END + 0x0051, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, ITEM_OWNER
    UNIT_FIELD_MINOFFHANDDAMAGE               = OBJECT_END + 0x0052, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, ITEM_OWNER
    UNIT_FIELD_MAXOFFHANDDAMAGE               = OBJECT_END + 0x0053, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, ITEM_OWNER
    UNIT_FIELD_BYTES_1                        = OBJECT_END + 0x0054, // Size: 1, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_PETNUMBER                      = OBJECT_END + 0x0055, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_PET_NAME_TIMESTAMP             = OBJECT_END + 0x0056, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_PETEXPERIENCE                  = OBJECT_END + 0x0057, // Size: 1, Type: INT, Flags: OWNER
    UNIT_FIELD_PETNEXTLEVELEXP                = OBJECT_END + 0x0058, // Size: 1, Type: INT, Flags: OWNER
    UNIT_DYNAMIC_FLAGS                        = OBJECT_END + 0x0059, // Size: 1, Type: INT, Flags: UNUSED2
    UNIT_MOD_CAST_SPEED                       = OBJECT_END + 0x005A, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_CREATED_BY_SPELL                     = OBJECT_END + 0x005B, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_NPC_FLAGS                            = OBJECT_END + 0x005C, // Size: 1, Type: INT, Flags: UNUSED2
    UNIT_NPC_EMOTESTATE                       = OBJECT_END + 0x005D, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_STATS                          = OBJECT_END + 0x005E, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POSSTATS                       = OBJECT_END + 0x0063, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_NEGSTATS                       = OBJECT_END + 0x0068, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RESISTANCES                    = OBJECT_END + 0x006D, // Size: 7, Type: INT, Flags: PRIVATE, OWNER, ITEM_OWNER
    UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE     = OBJECT_END + 0x0074, // Size: 7, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE     = OBJECT_END + 0x007B, // Size: 7, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_BASE_MANA                      = OBJECT_END + 0x0082, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BASE_HEALTH                    = OBJECT_END + 0x0083, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_BYTES_2                        = OBJECT_END + 0x0084, // Size: 1, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_ATTACK_POWER                   = OBJECT_END + 0x0085, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_ATTACK_POWER_MOD_POS           = OBJECT_END + 0x0086, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_ATTACK_POWER_MOD_NEG           = OBJECT_END + 0x0087, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_ATTACK_POWER_MULTIPLIER        = OBJECT_END + 0x0088, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER            = OBJECT_END + 0x0089, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS    = OBJECT_END + 0x008A, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG    = OBJECT_END + 0x008B, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER = OBJECT_END + 0x008C, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_MINRANGEDDAMAGE                = OBJECT_END + 0x008D, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_MAXRANGEDDAMAGE                = OBJECT_END + 0x008E, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POWER_COST_MODIFIER            = OBJECT_END + 0x008F, // Size: 7, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POWER_COST_MULTIPLIER          = OBJECT_END + 0x0096, // Size: 7, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_MAXHEALTHMODIFIER              = OBJECT_END + 0x009D, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_HOVERHEIGHT                    = OBJECT_END + 0x009E, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_MAXITEMLEVEL                   = OBJECT_END + 0x009F, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_END                                  = OBJECT_END + 0x00A0,

    PLAYER_DUEL_ARBITER                       = UNIT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_FLAGS                              = UNIT_END + 0x0002, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILDRANK                          = UNIT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILDDELETE_DATE                   = UNIT_END + 0x0004, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILDLEVEL                         = UNIT_END + 0x0005, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_BYTES                              = UNIT_END + 0x0006, // Size: 1, Type: BYTES, Flags: PUBLIC
    PLAYER_BYTES_2                            = UNIT_END + 0x0007, // Size: 1, Type: BYTES, Flags: PUBLIC
    PLAYER_BYTES_3                            = UNIT_END + 0x0008, // Size: 1, Type: BYTES, Flags: PUBLIC
    PLAYER_DUEL_TEAM                          = UNIT_END + 0x0009, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILD_TIMESTAMP                    = UNIT_END + 0x000A, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_QUEST_LOG                          = UNIT_END + 0x000B, // Size: 250, Type: INT, Flags: PARTY_LEADER
    PLAYER_VISIBLE_ITEM                       = UNIT_END + 0x0105, // Size: 38, Type: INT, Flags: PUBLIC
    PLAYER_CHOSEN_TITLE                       = UNIT_END + 0x012B, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_FAKE_INEBRIATION                   = UNIT_END + 0x012C, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_FIELD_PAD_0                        = UNIT_END + 0x012D, // Size: 1, Type: INT, Flags: NONE
    PLAYER_FIELD_INV_SLOT_HEAD                = UNIT_END + 0x012E, // Size: 46, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_PACK_SLOT_1                  = UNIT_END + 0x015C, // Size: 32, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_BANK_SLOT_1                  = UNIT_END + 0x017C, // Size: 56, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_BANKBAG_SLOT_1               = UNIT_END + 0x01B4, // Size: 14, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_VENDORBUYBACK_SLOT_1         = UNIT_END + 0x01C2, // Size: 24, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_KEYRING_SLOT_1               = UNIT_END + 0x01DA, // Size: 64, Type: LONG, Flags: PRIVATE
    PLAYER_FARSIGHT                           = UNIT_END + 0x021A, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER__FIELD_KNOWN_TITLES                = UNIT_END + 0x021C, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER__FIELD_KNOWN_TITLES1               = UNIT_END + 0x021E, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER__FIELD_KNOWN_TITLES2               = UNIT_END + 0x0220, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER_XP                                 = UNIT_END + 0x0222, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_NEXT_LEVEL_XP                      = UNIT_END + 0x0223, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_SKILL_INFO_1_1                     = UNIT_END + 0x0224, // Size: 384, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_CHARACTER_POINTS                   = UNIT_END + 0x03A4, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_TRACK_CREATURES                    = UNIT_END + 0x03A5, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_TRACK_RESOURCES                    = UNIT_END + 0x03A6, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_BLOCK_PERCENTAGE                   = UNIT_END + 0x03A7, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_DODGE_PERCENTAGE                   = UNIT_END + 0x03A8, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_PARRY_PERCENTAGE                   = UNIT_END + 0x03A9, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_EXPERTISE                          = UNIT_END + 0x03AA, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_OFFHAND_EXPERTISE                  = UNIT_END + 0x03AB, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_CRIT_PERCENTAGE                    = UNIT_END + 0x03AC, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_RANGED_CRIT_PERCENTAGE             = UNIT_END + 0x03AD, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_OFFHAND_CRIT_PERCENTAGE            = UNIT_END + 0x03AE, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_SPELL_CRIT_PERCENTAGE1             = UNIT_END + 0x03AF, // Size: 7, Type: FLOAT, Flags: PRIVATE
    PLAYER_SHIELD_BLOCK                       = UNIT_END + 0x03B6, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_SHIELD_BLOCK_CRIT_PERCENTAGE       = UNIT_END + 0x03B7, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_MASTERY                            = UNIT_END + 0x03B8, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_EXPLORED_ZONES_1                   = UNIT_END + 0x03B9, // Size: 144, Type: BYTES, Flags: PRIVATE
    PLAYER_REST_STATE_EXPERIENCE              = UNIT_END + 0x0449, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_COINAGE                      = UNIT_END + 0x044A, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_MOD_DAMAGE_DONE_POS          = UNIT_END + 0x044C, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_DAMAGE_DONE_NEG          = UNIT_END + 0x0453, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_DAMAGE_DONE_PCT          = UNIT_END + 0x045A, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_HEALING_DONE_POS         = UNIT_END + 0x0461, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_HEALING_PCT              = UNIT_END + 0x0462, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_HEALING_DONE_PCT         = UNIT_END + 0x0463, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_SPELL_POWER_PCT          = UNIT_END + 0x0464, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_TARGET_RESISTANCE        = UNIT_END + 0x0465, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE = UNIT_END + 0x0466, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BYTES                        = UNIT_END + 0x0467, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_SELF_RES_SPELL                     = UNIT_END + 0x0468, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_PVP_MEDALS                   = UNIT_END + 0x0469, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BUYBACK_PRICE_1              = UNIT_END + 0x046A, // Size: 12, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BUYBACK_TIMESTAMP_1          = UNIT_END + 0x0476, // Size: 12, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_KILLS                        = UNIT_END + 0x0482, // Size: 1, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_FIELD_LIFETIME_HONORBALE_KILLS     = UNIT_END + 0x0483, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BYTES2                       = UNIT_END + 0x0484, // Size: 1, Type: 6, Flags: PRIVATE
    PLAYER_FIELD_WATCHED_FACTION_INDEX        = UNIT_END + 0x0485, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_COMBAT_RATING_1              = UNIT_END + 0x0486, // Size: 26, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_ARENA_TEAM_INFO_1_1          = UNIT_END + 0x04A0, // Size: 21, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BATTLEGROUND_RATING          = UNIT_END + 0x04B5, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MAX_LEVEL                    = UNIT_END + 0x04B6, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_DAILY_QUESTS_1               = UNIT_END + 0x04B7, // Size: 25, Type: INT, Flags: PRIVATE
    PLAYER_RUNE_REGEN_1                       = UNIT_END + 0x04D0, // Size: 4, Type: FLOAT, Flags: PRIVATE
    PLAYER_NO_REAGENT_COST_1                  = UNIT_END + 0x04D4, // Size: 3, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_GLYPH_SLOTS_1                = UNIT_END + 0x04D7, // Size: 9, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_GLYPHS_1                     = UNIT_END + 0x04E0, // Size: 9, Type: INT, Flags: PRIVATE
    PLAYER_GLYPHS_ENABLED                     = UNIT_END + 0x04E9, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_PET_SPELL_POWER                    = UNIT_END + 0x04EA, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_RESEARCHING_1                = UNIT_END + 0x04EB, // Size: 8, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_FIELD_RESERACH_SITE_1              = UNIT_END + 0x04F3, // Size: 8, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_PROFESSION_SKILL_LINE_1            = UNIT_END + 0x04FB, // Size: 2, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_UI_HIT_MODIFIER              = UNIT_END + 0x04FD, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_UI_SPELL_HIT_MODIFIER        = UNIT_END + 0x04FE, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_HOME_REALM_TIME_OFFSET       = UNIT_END + 0x04FF, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_HASTE                    = UNIT_END + 0x0500, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_RANGED_HASTE             = UNIT_END + 0x0501, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_PET_HASTE                = UNIT_END + 0x0502, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_HASTE_REGEN              = UNIT_END + 0x0503, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_END                                = UNIT_END + 0x0504,
};

enum EGameObjectFields
{
    OBJECT_FIELD_CREATED_BY                   = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    GAMEOBJECT_DISPLAYID                      = OBJECT_END + 0x0002, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_FLAGS                          = OBJECT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_PARENTROTATION                 = OBJECT_END + 0x0004, // Size: 4, Type: FLOAT, Flags: PUBLIC
    GAMEOBJECT_DYNAMIC                        = OBJECT_END + 0x0008, // Size: 1, Type: TWO_SHORT, Flags: UNUSED2
    GAMEOBJECT_FACTION                        = OBJECT_END + 0x0009, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_LEVEL                          = OBJECT_END + 0x000A, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_BYTES_1                        = OBJECT_END + 0x000B, // Size: 1, Type: BYTES, Flags: PUBLIC
    GAMEOBJECT_END                            = OBJECT_END + 0x000C,
};

enum EDynamicObjectFields
{
    DYNAMICOBJECT_CASTER                      = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    DYNAMICOBJECT_BYTES                       = OBJECT_END + 0x0002, // Size: 1, Type: BYTES, Flags: PUBLIC
    DYNAMICOBJECT_SPELLID                     = OBJECT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    DYNAMICOBJECT_RADIUS                      = OBJECT_END + 0x0004, // Size: 1, Type: FLOAT, Flags: PUBLIC
    DYNAMICOBJECT_CASTTIME                    = OBJECT_END + 0x0005, // Size: 1, Type: INT, Flags: PUBLIC
    DYNAMICOBJECT_END                         = OBJECT_END + 0x0006,
};

enum ECorpseFields
{
    CORPSE_FIELD_OWNER                        = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    CORPSE_FIELD_PARTY                        = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    CORPSE_FIELD_DISPLAY_ID                   = OBJECT_END + 0x0004, // Size: 1, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_ITEM                         = OBJECT_END + 0x0005, // Size: 19, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_BYTES_1                      = OBJECT_END + 0x0018, // Size: 1, Type: BYTES, Flags: PUBLIC
    CORPSE_FIELD_BYTES_2                      = OBJECT_END + 0x0019, // Size: 1, Type: BYTES, Flags: PUBLIC
    CORPSE_FIELD_FLAGS                        = OBJECT_END + 0x001A, // Size: 1, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_DYNAMIC_FLAGS                = OBJECT_END + 0x001B, // Size: 1, Type: INT, Flags: UNUSED2
    CORPSE_END                                = OBJECT_END + 0x001C,
};

/************************************************************************/
/* Manual Fields                                                        */
/************************************************************************/
#define UNIT_FIELD_STRENGTH                     UNIT_FIELD_STATS
#define UNIT_FIELD_AGILITY                      UNIT_FIELD_STATS+1
#define UNIT_FIELD_STAMINA                      UNIT_FIELD_STATS+2
#define UNIT_FIELD_INTELLECT                    UNIT_FIELD_STATS+3
#define UNIT_FIELD_SPIRIT                       UNIT_FIELD_STATS+4

#define UNIT_FIELD_MANA                         UNIT_FIELD_POWER
#define UNIT_FIELD_RAGE                         UNIT_FIELD_POWER+1
#define UNIT_FIELD_FOCUS                        UNIT_FIELD_POWER+2
#define UNIT_FIELD_ENERGY                       UNIT_FIELD_POWER+3
#define UNIT_FIELD_HAPPINESS                    UNIT_FIELD_POWER+4
#define UNIT_FIELD_RUNE_POWER                   UNIT_FIELD_POWER+5
#define UNIT_FIELD_RUNIC_POWER                  UNIT_FIELD_POWER+6
#define UNIT_FIELD_SOUL_SHARDS                  UNIT_FIELD_POWER+7
#define UNIT_FIELD_ECLIPSE_POWER                UNIT_FIELD_POWER+8
#define UNIT_FIELD_HOLY_POWER                   UNIT_FIELD_POWER+9

#define UNIT_FIELD_MAX_MANA                     UNIT_FIELD_MAXPOWER
#define UNIT_FIELD_MAX_RAGE                     UNIT_FIELD_MAXPOWER+1
#define UNIT_FIELD_MAX_FOCUS                    UNIT_FIELD_MAXPOWER+2
#define UNIT_FIELD_MAX_ENERGY                   UNIT_FIELD_MAXPOWER+3
#define UNIT_FIELD_MAX_HAPPINESS                UNIT_FIELD_MAXPOWER+4
#define UNIT_FIELD_MAX_RUNE_POWER               UNIT_FIELD_MAXPOWER+5
#define UNIT_FIELD_MAX_RUNIC_POWER              UNIT_FIELD_MAXPOWER+6
#define UNIT_FIELD_MAX_SOUL_SHARDS              UNIT_FIELD_MAXPOWER+7
#define UNIT_FIELD_MAX_ECLIPSE_POWER            UNIT_FIELD_MAXPOWER+8
#define UNIT_FIELD_MAX_HOLY_POWER               UNIT_FIELD_MAXPOWER+9

#define GAMEOBJECT_ROTATION                     GAMEOBJECT_PARENTROTATION
#define GAMEOBJECT_ROTATION_01                  GAMEOBJECT_PARENTROTATION+1
#define GAMEOBJECT_ROTATION_02                  GAMEOBJECT_PARENTROTATION+2
#define GAMEOBJECT_ROTATION_03                  GAMEOBJECT_PARENTROTATION+3

#define PLAYER_GUILDID                          OBJECT_FIELD_DATA
#define PLAYER_FIELD_LIFETIME_HONORABLE_KILLS   PLAYER_FIELD_LIFETIME_HONORBALE_KILLS
#define PLAYER_MAX_SKILL_INFO_FIELD             PLAYER_SKILL_INFO_1_1+384 // PLAYER_CHARACTER_POINTS
#define PLAYER_QUEST_LOG_END                    PLAYER_QUEST_LOG+250

#define PLAYER_VISIBLE_ITEM_LENGTH              2
#define PLAYER_VISIBLE_ITEM_END                 PLAYER_VISIBLE_ITEM+38

#define PLAYER_RATING_MODIFIER_RANGED_SKILL                     PLAYER_FIELD_COMBAT_RATING_1
#define PLAYER_RATING_MODIFIER_DEFENCE                          PLAYER_FIELD_COMBAT_RATING_1+1
#define PLAYER_RATING_MODIFIER_DODGE                            PLAYER_FIELD_COMBAT_RATING_1+2
#define PLAYER_RATING_MODIFIER_PARRY                            PLAYER_FIELD_COMBAT_RATING_1+3
#define PLAYER_RATING_MODIFIER_BLOCK                            PLAYER_FIELD_COMBAT_RATING_1+4
#define PLAYER_RATING_MODIFIER_MELEE_HIT                        PLAYER_FIELD_COMBAT_RATING_1+5
#define PLAYER_RATING_MODIFIER_RANGED_HIT                       PLAYER_FIELD_COMBAT_RATING_1+6
#define PLAYER_RATING_MODIFIER_SPELL_HIT                        PLAYER_FIELD_COMBAT_RATING_1+7
#define PLAYER_RATING_MODIFIER_MELEE_CRIT                       PLAYER_FIELD_COMBAT_RATING_1+8
#define PLAYER_RATING_MODIFIER_RANGED_CRIT                      PLAYER_FIELD_COMBAT_RATING_1+9
#define PLAYER_RATING_MODIFIER_SPELL_CRIT                       PLAYER_FIELD_COMBAT_RATING_1+10
#define PLAYER_RATING_MODIFIER_MELEE_HIT_AVOIDANCE              PLAYER_FIELD_COMBAT_RATING_1+11 // GUESSED
#define PLAYER_RATING_MODIFIER_RANGED_HIT_AVOIDANCE             PLAYER_FIELD_COMBAT_RATING_1+12 // GUESSED
#define PLAYER_RATING_MODIFIER_SPELL_HIT_AVOIDANCE              PLAYER_FIELD_COMBAT_RATING_1+13 // GUESSED
#define PLAYER_RATING_MODIFIER_MELEE_RESILIENCE                 PLAYER_FIELD_COMBAT_RATING_1+14
#define PLAYER_RATING_MODIFIER_RANGED_RESILIENCE                PLAYER_FIELD_COMBAT_RATING_1+15
#define PLAYER_RATING_MODIFIER_SPELL_RESILIENCE                 PLAYER_FIELD_COMBAT_RATING_1+16
#define PLAYER_RATING_MODIFIER_MELEE_HASTE                      PLAYER_FIELD_COMBAT_RATING_1+17
#define PLAYER_RATING_MODIFIER_RANGED_HASTE                     PLAYER_FIELD_COMBAT_RATING_1+18
#define PLAYER_RATING_MODIFIER_SPELL_HASTE                      PLAYER_FIELD_COMBAT_RATING_1+19
#define PLAYER_RATING_MODIFIER_MELEE_MAIN_HAND_SKILL            PLAYER_FIELD_COMBAT_RATING_1+20
#define PLAYER_RATING_MODIFIER_MELEE_OFF_HAND_SKILL             PLAYER_FIELD_COMBAT_RATING_1+21
#define PLAYER_RATING_MODIFIER_RANGED_WEAPON_SKILL              PLAYER_FIELD_COMBAT_RATING_1+22
#define PLAYER_RATING_MODIFIER_EXPERTISE                        PLAYER_FIELD_COMBAT_RATING_1+23
#define PLAYER_RATING_MODIFIER_ARMOR_PENETRATION_RATING         PLAYER_FIELD_COMBAT_RATING_1+24
#define PLAYER_RATING_MODIFIER_MASTERY                          PLAYER_FIELD_COMBAT_RATING_1+25
