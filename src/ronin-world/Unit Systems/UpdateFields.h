/***
 * Demonstrike Core
 */

#pragma once

// Auto generated for version 4.3.4, 15595

enum EObjectFields
{
    OBJECT_FIELD_GUID                                = 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    OBJECT_FIELD_DATA                                = 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    OBJECT_FIELD_TYPE                                = 0x0004, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    OBJECT_FIELD_ENTRY                               = 0x0005, // Size: 1, Type: INT, Flags: PUBLIC
    OBJECT_FIELD_SCALE_X                             = 0x0006, // Size: 1, Type: FLOAT, Flags: PUBLIC
    OBJECT_FIELD_PADDING                             = 0x0007, // Size: 1, Type: INT, Flags: NONE
    OBJECT_END                                       = 0x0008,
    OBJECT_LENGTH                                    = OBJECT_END
};

enum EItemFields
{
    ITEM_FIELD_OWNER                                 = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_CONTAINED                             = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_CREATOR                               = OBJECT_END + 0x0004, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_GIFTCREATOR                           = OBJECT_END + 0x0006, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_STACK_COUNT                           = OBJECT_END + 0x0008, // Size: 1, Type: INT, Flags: OWNER, ITEM_OWNER
    ITEM_FIELD_DURATION                              = OBJECT_END + 0x0009, // Size: 1, Type: INT, Flags: OWNER, ITEM_OWNER
    ITEM_FIELD_SPELL_CHARGES                         = OBJECT_END + 0x000A, // Size: 5, Type: INT, Flags: OWNER, ITEM_OWNER
    ITEM_FIELD_FLAGS                                 = OBJECT_END + 0x000F, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT_DATA                      = OBJECT_END + 0x0010, // Size: 45, Type: INT, Flags: PUBLIC
    ITEM_FIELD_PROPERTY_SEED                         = OBJECT_END + 0x003D, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_RANDOM_PROPERTIES_ID                  = OBJECT_END + 0x003E, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_DURABILITY                            = OBJECT_END + 0x003F, // Size: 1, Type: INT, Flags: OWNER, ITEM_OWNER
    ITEM_FIELD_MAXDURABILITY                         = OBJECT_END + 0x0040, // Size: 1, Type: INT, Flags: OWNER, ITEM_OWNER
    ITEM_FIELD_CREATE_PLAYED_TIME                    = OBJECT_END + 0x0041, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_END                                         = OBJECT_END + 0x0042,
    ITEM_LENGTH                                      = ITEM_END-OBJECT_END
};

enum EContainerFields
{
    CONTAINER_FIELD_NUM_SLOTS                        = ITEM_END + 0x0000, // Size: 1, Type: INT, Flags: PUBLIC
    CONTAINER_ALIGN_PAD                              = ITEM_END + 0x0001, // Size: 1, Type: BYTES, Flags: NONE
    CONTAINER_FIELD_SLOT_1                           = ITEM_END + 0x0002, // Size: 72, Type: LONG, Flags: PUBLIC
    CONTAINER_END                                    = ITEM_END + 0x004A,
    CONTAINER_LENGTH                                 = CONTAINER_END-ITEM_END
};

enum EUnitFields
{
    UNIT_FIELD_CHARM                                 = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_SUMMON                                = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CRITTER                               = OBJECT_END + 0x0004, // Size: 2, Type: LONG, Flags: PRIVATE
    UNIT_FIELD_CHARMEDBY                             = OBJECT_END + 0x0006, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_SUMMONEDBY                            = OBJECT_END + 0x0008, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CREATEDBY                             = OBJECT_END + 0x000A, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_TARGET                                = OBJECT_END + 0x000C, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CHANNEL_OBJECT                        = OBJECT_END + 0x000E, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_CHANNEL_SPELL                               = OBJECT_END + 0x0010, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BYTES_0                               = OBJECT_END + 0x0011, // Size: 1, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_HEALTH                                = OBJECT_END + 0x0012, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWERS                                = OBJECT_END + 0x0013, // Size: 5, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXHEALTH                             = OBJECT_END + 0x0018, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWERS                             = OBJECT_END + 0x0019, // Size: 5, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER             = OBJECT_END + 0x001E, // Size: 5, Type: FLOAT, Flags: PRIVATE, OWNER, UNUSED2
    UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER = OBJECT_END + 0x0023, // Size: 5, Type: FLOAT, Flags: PRIVATE, OWNER, UNUSED2
    UNIT_FIELD_LEVEL                                 = OBJECT_END + 0x0028, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_FACTIONTEMPLATE                       = OBJECT_END + 0x0029, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_VIRTUAL_ITEM_SLOT_ID                        = OBJECT_END + 0x002A, // Size: 3, Type: INT, Flags: PUBLIC
    UNIT_FIELD_FLAGS                                 = OBJECT_END + 0x002D, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_FLAGS_2                               = OBJECT_END + 0x002E, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_AURASTATE                             = OBJECT_END + 0x002F, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BASEATTACKTIME                        = OBJECT_END + 0x0030, // Size: 2, Type: INT, Flags: PUBLIC
    UNIT_FIELD_RANGEDATTACKTIME                      = OBJECT_END + 0x0032, // Size: 1, Type: INT, Flags: PRIVATE
    UNIT_FIELD_BOUNDINGRADIUS                        = OBJECT_END + 0x0033, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_COMBATREACH                           = OBJECT_END + 0x0034, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_DISPLAYID                             = OBJECT_END + 0x0035, // Size: 1, Type: INT, Flags: DYNAMIC
    UNIT_FIELD_NATIVEDISPLAYID                       = OBJECT_END + 0x0036, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MOUNTDISPLAYID                        = OBJECT_END + 0x0037, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MINDAMAGE                             = OBJECT_END + 0x0038, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, SPECIAL_INFO
    UNIT_FIELD_MAXDAMAGE                             = OBJECT_END + 0x0039, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, SPECIAL_INFO
    UNIT_FIELD_MINOFFHANDDAMAGE                      = OBJECT_END + 0x003A, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, SPECIAL_INFO
    UNIT_FIELD_MAXOFFHANDDAMAGE                      = OBJECT_END + 0x003B, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER, SPECIAL_INFO
    UNIT_FIELD_BYTES_1                               = OBJECT_END + 0x003C, // Size: 1, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_PETNUMBER                             = OBJECT_END + 0x003D, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_PET_NAME_TIMESTAMP                    = OBJECT_END + 0x003E, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_PETEXPERIENCE                         = OBJECT_END + 0x003F, // Size: 1, Type: INT, Flags: OWNER
    UNIT_FIELD_PETNEXTLEVELEXP                       = OBJECT_END + 0x0040, // Size: 1, Type: INT, Flags: OWNER
    UNIT_DYNAMIC_FLAGS                               = OBJECT_END + 0x0041, // Size: 1, Type: INT, Flags: DYNAMIC
    UNIT_MOD_CAST_SPEED                              = OBJECT_END + 0x0042, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_MOD_CAST_HASTE                              = OBJECT_END + 0x0043, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_CREATED_BY_SPELL                            = OBJECT_END + 0x0044, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_NPC_FLAGS                                   = OBJECT_END + 0x0045, // Size: 1, Type: INT, Flags: DYNAMIC
    UNIT_NPC_EMOTESTATE                              = OBJECT_END + 0x0046, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_STATS                                 = OBJECT_END + 0x0047, // Size: 5, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POSSTATS                              = OBJECT_END + 0x004C, // Size: 5, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_NEGSTATS                              = OBJECT_END + 0x0051, // Size: 5, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RESISTANCES                           = OBJECT_END + 0x0056, // Size: 7, Type: INT, Flags: PRIVATE, OWNER, SPECIAL_INFO
    UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE            = OBJECT_END + 0x005D, // Size: 7, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE            = OBJECT_END + 0x0064, // Size: 7, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_BASE_MANA                             = OBJECT_END + 0x006B, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BASE_HEALTH                           = OBJECT_END + 0x006C, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_BYTES_2                               = OBJECT_END + 0x006D, // Size: 1, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_ATTACK_POWER                          = OBJECT_END + 0x006E, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_ATTACK_POWER_MOD_POS                  = OBJECT_END + 0x006F, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_ATTACK_POWER_MOD_NEG                  = OBJECT_END + 0x0070, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_ATTACK_POWER_MULTIPLIER               = OBJECT_END + 0x0071, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER                   = OBJECT_END + 0x0072, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS           = OBJECT_END + 0x0073, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG           = OBJECT_END + 0x0074, // Size: 1, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER        = OBJECT_END + 0x0075, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_MINRANGEDDAMAGE                       = OBJECT_END + 0x0076, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_MAXRANGEDDAMAGE                       = OBJECT_END + 0x0077, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POWER_COST_MODIFIER                   = OBJECT_END + 0x0078, // Size: 7, Type: INT, Flags: PRIVATE, OWNER
    UNIT_FIELD_POWER_COST_MULTIPLIER                 = OBJECT_END + 0x007F, // Size: 7, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_MAXHEALTHMODIFIER                     = OBJECT_END + 0x0086, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER
    UNIT_FIELD_HOVERHEIGHT                           = OBJECT_END + 0x0087, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_MAXITEMLEVEL                          = OBJECT_END + 0x0088, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_PADDING                               = OBJECT_END + 0x0089, // Size: 1, Type: INT, Flags: NONE
    UNIT_END                                         = OBJECT_END + 0x008A,
    UNIT_LENGTH                                      = UNIT_END-OBJECT_END
};

enum EPlayerFields
{
    PLAYER_DUEL_ARBITER                              = UNIT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_FLAGS                                     = UNIT_END + 0x0002, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILDRANK                                 = UNIT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILDDELETE_DATE                          = UNIT_END + 0x0004, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILDLEVEL                                = UNIT_END + 0x0005, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_BYTES                                     = UNIT_END + 0x0006, // Size: 1, Type: BYTES, Flags: PUBLIC
    PLAYER_BYTES_2                                   = UNIT_END + 0x0007, // Size: 1, Type: BYTES, Flags: PUBLIC
    PLAYER_BYTES_3                                   = UNIT_END + 0x0008, // Size: 1, Type: BYTES, Flags: PUBLIC
    PLAYER_DUEL_TEAM                                 = UNIT_END + 0x0009, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILD_TIMESTAMP                           = UNIT_END + 0x000A, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_QUEST_LOG                                 = UNIT_END + 0x000B, // Size: 250, Type: INT, Flags: PARTY_MEMBER
    PLAYER_VISIBLE_ITEM                              = UNIT_END + 0x0105, // Size: 38, Type: INT, Flags: PUBLIC
    PLAYER_CHOSEN_TITLE                              = UNIT_END + 0x012B, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_FAKE_INEBRIATION                          = UNIT_END + 0x012C, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_FIELD_PAD_0                               = UNIT_END + 0x012D, // Size: 1, Type: INT, Flags: NONE
    PLAYER_FIELD_INV_SLOT_HEAD                       = UNIT_END + 0x012E, // Size: 46, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_PACK_SLOT_1                         = UNIT_END + 0x015C, // Size: 32, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_BANK_SLOT_1                         = UNIT_END + 0x017C, // Size: 56, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_BANKBAG_SLOT_1                      = UNIT_END + 0x01B4, // Size: 14, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_VENDORBUYBACK_SLOT_1                = UNIT_END + 0x01C2, // Size: 24, Type: LONG, Flags: PRIVATE
    PLAYER_FARSIGHT                                  = UNIT_END + 0x01DA, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER__FIELD_KNOWN_TITLES                       = UNIT_END + 0x01DC, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER__FIELD_KNOWN_TITLES1                      = UNIT_END + 0x01DE, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER__FIELD_KNOWN_TITLES2                      = UNIT_END + 0x01E0, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER__FIELD_KNOWN_TITLES3                      = UNIT_END + 0x01E2, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER_XP                                        = UNIT_END + 0x01E4, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_NEXT_LEVEL_XP                             = UNIT_END + 0x01E5, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_SKILL_LINEID_0                            = UNIT_END + 0x01E6, // Size: 64, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_SKILL_STEP_0                              = UNIT_END + 0x0226, // Size: 64, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_SKILL_RANK_0                              = UNIT_END + 0x0266, // Size: 64, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_SKILL_MAX_RANK_0                          = UNIT_END + 0x02A6, // Size: 64, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_SKILL_MODIFIER_0                          = UNIT_END + 0x02E6, // Size: 64, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_SKILL_TALENT_0                            = UNIT_END + 0x0326, // Size: 64, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_CHARACTER_POINTS                          = UNIT_END + 0x0366, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_TRACK_CREATURES                           = UNIT_END + 0x0367, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_TRACK_RESOURCES                           = UNIT_END + 0x0368, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_EXPERTISE                                 = UNIT_END + 0x0369, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_OFFHAND_EXPERTISE                         = UNIT_END + 0x036A, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_BLOCK_PERCENTAGE                          = UNIT_END + 0x036B, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_DODGE_PERCENTAGE                          = UNIT_END + 0x036C, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_PARRY_PERCENTAGE                          = UNIT_END + 0x036D, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_CRIT_PERCENTAGE                           = UNIT_END + 0x036E, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_RANGED_CRIT_PERCENTAGE                    = UNIT_END + 0x036F, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_OFFHAND_CRIT_PERCENTAGE                   = UNIT_END + 0x0370, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_SPELL_CRIT_PERCENTAGE                     = UNIT_END + 0x0371, // Size: 7, Type: FLOAT, Flags: PRIVATE
    PLAYER_SHIELD_BLOCK                              = UNIT_END + 0x0378, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_SHIELD_BLOCK_CRIT_PERCENTAGE              = UNIT_END + 0x0379, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_MASTERY                                   = UNIT_END + 0x037A, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_EXPLORED_ZONES_1                          = UNIT_END + 0x037B, // Size: 156, Type: BYTES, Flags: PRIVATE
    PLAYER_REST_STATE_EXPERIENCE                     = UNIT_END + 0x0417, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_COINAGE                             = UNIT_END + 0x0418, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_MOD_DAMAGE_DONE_POS                 = UNIT_END + 0x041A, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_DAMAGE_DONE_NEG                 = UNIT_END + 0x0421, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_DAMAGE_DONE_PCT                 = UNIT_END + 0x0428, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_HEALING_DONE_POS                = UNIT_END + 0x042F, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_HEALING_PCT                     = UNIT_END + 0x0430, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_HEALING_DONE_PCT                = UNIT_END + 0x0431, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_WEAPON_DMG_MULTIPLIERS              = UNIT_END + 0x0432, // Size: 3, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_SPELL_POWER_PCT                 = UNIT_END + 0x0435, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_OVERRIDE_SPELL_POWER_BY_AP_PCT      = UNIT_END + 0x0436, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_TARGET_RESISTANCE               = UNIT_END + 0x0437, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE      = UNIT_END + 0x0438, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BYTES                               = UNIT_END + 0x0439, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_SELF_RES_SPELL                            = UNIT_END + 0x043A, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_PVP_MEDALS                          = UNIT_END + 0x043B, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BUYBACK_PRICE_1                     = UNIT_END + 0x043C, // Size: 12, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BUYBACK_TIMESTAMP_1                 = UNIT_END + 0x0448, // Size: 12, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_KILLS                               = UNIT_END + 0x0454, // Size: 1, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_FIELD_LIFETIME_HONORABLE_KILLS            = UNIT_END + 0x0455, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BYTES2                              = UNIT_END + 0x0456, // Size: 1, Type: 6, Flags: PRIVATE
    PLAYER_FIELD_WATCHED_FACTION_INDEX               = UNIT_END + 0x0457, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_COMBAT_RATING_1                     = UNIT_END + 0x0458, // Size: 26, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_ARENA_TEAM_INFO_1_1                 = UNIT_END + 0x0472, // Size: 21, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BATTLEGROUND_RATING                 = UNIT_END + 0x0487, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MAX_LEVEL                           = UNIT_END + 0x0488, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_DAILY_QUESTS_1                      = UNIT_END + 0x0489, // Size: 25, Type: INT, Flags: PRIVATE
    PLAYER_RUNE_REGEN_1                              = UNIT_END + 0x04A2, // Size: 4, Type: FLOAT, Flags: PRIVATE
    PLAYER_NO_REAGENT_COST_1                         = UNIT_END + 0x04A6, // Size: 3, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_GLYPH_SLOTS_1                       = UNIT_END + 0x04A9, // Size: 9, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_GLYPHS_1                            = UNIT_END + 0x04B2, // Size: 9, Type: INT, Flags: PRIVATE
    PLAYER_GLYPHS_ENABLED                            = UNIT_END + 0x04BB, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_PET_SPELL_POWER                           = UNIT_END + 0x04BC, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_RESEARCHING_1                       = UNIT_END + 0x04BD, // Size: 8, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_FIELD_RESERACH_SITE_1                     = UNIT_END + 0x04C5, // Size: 8, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_PROFESSION_SKILL_LINE_1                   = UNIT_END + 0x04CD, // Size: 2, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_UI_HIT_MODIFIER                     = UNIT_END + 0x04CF, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_UI_SPELL_HIT_MODIFIER               = UNIT_END + 0x04D0, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_HOME_REALM_TIME_OFFSET              = UNIT_END + 0x04D1, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_HASTE                           = UNIT_END + 0x04D2, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_RANGED_HASTE                    = UNIT_END + 0x04D3, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_PET_HASTE                       = UNIT_END + 0x04D4, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_FIELD_MOD_HASTE_REGEN                     = UNIT_END + 0x04D5, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_END                                       = UNIT_END + 0x04D6,
    PLAYER_LENGTH                                    = PLAYER_END-UNIT_END
};

enum EGameObjectFields
{
    GAMEOBJECT_FIELD_CREATED_BY                      = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    GAMEOBJECT_DISPLAYID                             = OBJECT_END + 0x0002, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_FLAGS                                 = OBJECT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_PARENTROTATION                        = OBJECT_END + 0x0004, // Size: 4, Type: FLOAT, Flags: PUBLIC
    GAMEOBJECT_DYNAMIC                               = OBJECT_END + 0x0008, // Size: 1, Type: TWO_SHORT, Flags: DYNAMIC
    GAMEOBJECT_FACTION                               = OBJECT_END + 0x0009, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_LEVEL                                 = OBJECT_END + 0x000A, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_BYTES_1                               = OBJECT_END + 0x000B, // Size: 1, Type: BYTES, Flags: PUBLIC
    GAMEOBJECT_END                                   = OBJECT_END + 0x000C,
    GAMEOBJECT_LENGTH                                = GAMEOBJECT_END-OBJECT_END
};

enum EDynamicObjectFields
{
    DYNAMICOBJECT_CASTER                             = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    DYNAMICOBJECT_BYTES                              = OBJECT_END + 0x0002, // Size: 1, Type: INT, Flags: DYNAMIC
    DYNAMICOBJECT_SPELLID                            = OBJECT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    DYNAMICOBJECT_RADIUS                             = OBJECT_END + 0x0004, // Size: 1, Type: FLOAT, Flags: PUBLIC
    DYNAMICOBJECT_CASTTIME                           = OBJECT_END + 0x0005, // Size: 1, Type: INT, Flags: PUBLIC
    DYNAMICOBJECT_END                                = OBJECT_END + 0x0006,
    DYNAMICOBJECT_LENGTH                             = DYNAMICOBJECT_END-OBJECT_END
};

enum ECorpseFields
{
    CORPSE_FIELD_OWNER                               = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    CORPSE_FIELD_PARTY                               = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    CORPSE_FIELD_DISPLAY_ID                          = OBJECT_END + 0x0004, // Size: 1, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_ITEM                                = OBJECT_END + 0x0005, // Size: 19, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_BYTES_1                             = OBJECT_END + 0x0018, // Size: 1, Type: BYTES, Flags: PUBLIC
    CORPSE_FIELD_BYTES_2                             = OBJECT_END + 0x0019, // Size: 1, Type: BYTES, Flags: PUBLIC
    CORPSE_FIELD_FLAGS                               = OBJECT_END + 0x001A, // Size: 1, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_DYNAMIC_FLAGS                       = OBJECT_END + 0x001B, // Size: 1, Type: INT, Flags: DYNAMIC
    CORPSE_END                                       = OBJECT_END + 0x001C,
    CORPSE_LENGTH                                    = CORPSE_END-OBJECT_END
};

enum EAreaTriggerFields
{
    AREATRIGGER_SPELLID                              = OBJECT_END + 0x0000, // Size: 1, Type: INT, Flags: PUBLIC
    AREATRIGGER_SPELLVISUALID                        = OBJECT_END + 0x0001, // Size: 1, Type: INT, Flags: PUBLIC
    AREATRIGGER_DURATION                             = OBJECT_END + 0x0002, // Size: 1, Type: INT, Flags: PUBLIC
    AREATRIGGER_FINAL_POS                            = OBJECT_END + 0x0003, // Size: 3, Type: FLOAT, Flags: PUBLIC
    AREATRIGGER_END                                  = OBJECT_END + 0x0006,
    AREATRIGGER_LENGTH                               = AREATRIGGER_END-OBJECT_END
};

/************************************************************************/
/* Manual Fields                                                        */
/************************************************************************/
#define UNIT_FIELD_STRENGTH                     UNIT_FIELD_STATS
#define UNIT_FIELD_AGILITY                      UNIT_FIELD_STATS+1
#define UNIT_FIELD_STAMINA                      UNIT_FIELD_STATS+2
#define UNIT_FIELD_INTELLECT                    UNIT_FIELD_STATS+3
#define UNIT_FIELD_SPIRIT                       UNIT_FIELD_STATS+4

#define PLAYER_GUILDID                          OBJECT_FIELD_DATA
#define PLAYER_QUEST_LOG_END                    PLAYER_QUEST_LOG+250

#define PLAYER_VISIBLE_ITEM_LENGTH              2
#define PLAYER_VISIBLE_ITEM_END                 PLAYER_VISIBLE_ITEM+38

#define PLAYER_RATING_MODIFIER_WEAPON_SKILL                     PLAYER_FIELD_COMBAT_RATING_1
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
#define PLAYER_RATING_MODIFIER_MAX                              PLAYER_FIELD_COMBAT_RATING_1+26

extern uint16 ObjectUpdateFieldFlags[OBJECT_LENGTH];
extern uint16 ItemUpdateFieldFlags[ITEM_LENGTH];
extern uint16 ContainerUpdateFieldFlags[CONTAINER_LENGTH];
extern uint16 UnitUpdateFieldFlags[UNIT_LENGTH];
extern uint16 PlayerUpdateFieldFlags[PLAYER_LENGTH];
extern uint16 GameObjectUpdateFieldFlags[GAMEOBJECT_LENGTH];
extern uint16 DynamicObjectUpdateFieldFlags[DYNAMICOBJECT_LENGTH];
extern uint16 CorpseUpdateFieldFlags[CORPSE_LENGTH];
extern uint16 AreaTriggerUpdateFieldFlags[AREATRIGGER_LENGTH];

enum UpdatefieldFlags : uint16
{
    UF_FLAG_NONE         = 0x000,
    UF_FLAG_PUBLIC       = 0x001,
    UF_FLAG_PRIVATE      = 0x002,
    UF_FLAG_OWNER        = 0x004,
    UF_FLAG_ITEM_OWNER   = 0x010,
    UF_FLAG_SPECIAL_INFO = 0x020,
    UF_FLAG_PARTY_MEMBER = 0x040,
    UF_FLAG_DYNAMIC      = 0x100,

    UF_FLAGMASK_PUBLIC = UF_FLAG_PUBLIC|UF_FLAG_DYNAMIC,
    UF_FLAGMASK_OWN_PET = UF_FLAG_PUBLIC|UF_FLAG_OWNER|UF_FLAG_DYNAMIC,
    UF_FLAGMASK_PARTY_MEMBER = UF_FLAG_PUBLIC|UF_FLAG_PARTY_MEMBER|UF_FLAG_DYNAMIC,
    UF_FLAGMASK_SELF = UF_FLAG_PUBLIC|UF_FLAG_PRIVATE|UF_FLAG_SPECIAL_INFO|UF_FLAG_DYNAMIC
};
