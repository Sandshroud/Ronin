/***
 * Demonstrike Core
 */

#pragma once

class WorldSession;
class Unit;
class WorldObject;
class DynamicObj;
class Player;
class Item;
class Group;
class Aura;

//! 4-bit flag
enum AURA_FLAGS
{
    AFLAG_EMPTY = 0x0,
    AFLAG_SET = 0x9
};

enum AURA_STATE_FLAGS
{
    AURASTATE_FLAG_DODGE_BLOCK          = 0x00000001,
    AURASTATE_FLAG_HEALTH20             = 0x00000002,
    AURASTATE_FLAG_BERSERK              = 0x00000004,
    AURASTATE_FLAG_FROZEN               = 0x00000008,
    AURASTATE_FLAG_JUDGEMENT            = 0x00000010,
    AURASTATE_FLAG_STUNNED              = 0x00000020, // CUSTOM!
    AURASTATE_FLAG_PARRY                = 0x00000040,
    AURASTATE_FLAG_UNK2                 = 0x00000080,
    AURASTATE_FLAG_UNK3                 = 0x00000100,
    AURASTATE_FLAG_VICTORIOUS           = 0x00000200, // after you kill an enemy that yields experience or honor
    AURASTATE_FLAG_CRITICAL             = 0x00000400,
    AURASTATE_FLAG_UNK4                 = 0x00000800,
    AURASTATE_FLAG_HEALTH35             = 0x00001000,
    AURASTATE_FLAG_IMMOLATE             = 0x00002000,
    AURASTATE_FLAG_REJUVENATE           = 0x00004000,
    AURASTATE_FLAG_POISON               = 0x00008000,
    AURASTATE_FLAG_ENRAGE               = 0x00010000,
    AURASTATE_FLAG_UNK5                 = 0x00020000,
    AURASTATE_FLAG_UNK6                 = 0x00040000,
    AURASTATE_FLAG_UNK7                 = 0x00080000,
    AURASTATE_FLAG_UNK8                 = 0x00100000,
    AURASTATE_FLAG_UNK9                 = 0x00200000,
    AURASTATE_FLAG_HEALTHABOVE75        = 0x00400000,
};

enum MOD_TYPES
{
    SPELL_AURA_NONE = 0,
    SPELL_AURA_BIND_SIGHT = 1,
    SPELL_AURA_MOD_POSSESS = 2,
    SPELL_AURA_PERIODIC_DAMAGE = 3,
    SPELL_AURA_DUMMY = 4,
    SPELL_AURA_MOD_CONFUSE = 5,
    SPELL_AURA_MOD_CHARM = 6,
    SPELL_AURA_MOD_FEAR = 7,
    SPELL_AURA_PERIODIC_HEAL = 8,
    SPELL_AURA_MOD_ATTACKSPEED = 9,
    SPELL_AURA_MOD_THREAT = 10,
    SPELL_AURA_MOD_TAUNT = 11,
    SPELL_AURA_MOD_STUN = 12,
    SPELL_AURA_MOD_DAMAGE_DONE = 13,
    SPELL_AURA_MOD_DAMAGE_TAKEN = 14,
    SPELL_AURA_DAMAGE_SHIELD = 15,
    SPELL_AURA_MOD_STEALTH = 16,
    SPELL_AURA_MOD_STEALTH_DETECT = 17,
    SPELL_AURA_MOD_INVISIBILITY = 18,
    SPELL_AURA_MOD_INVISIBILITY_DETECT = 19,
    SPELL_AURA_MOD_TOTAL_HEALTH_REGEN_PCT = 20,
    SPELL_AURA_MOD_TOTAL_MANA_REGEN_PCT = 21,
    SPELL_AURA_MOD_RESISTANCE = 22,
    SPELL_AURA_PERIODIC_TRIGGER_SPELL = 23,
    SPELL_AURA_PERIODIC_ENERGIZE = 24,
    SPELL_AURA_MOD_PACIFY = 25,
    SPELL_AURA_MOD_ROOT = 26,
    SPELL_AURA_MOD_SILENCE = 27,
    SPELL_AURA_REFLECT_SPELLS = 28,
    SPELL_AURA_MOD_STAT = 29,
    SPELL_AURA_MOD_SKILL = 30,
    SPELL_AURA_MOD_INCREASE_SPEED = 31,
    SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED = 32,
    SPELL_AURA_MOD_DECREASE_SPEED = 33,
    SPELL_AURA_MOD_INCREASE_HEALTH = 34,
    SPELL_AURA_MOD_INCREASE_ENERGY = 35,
    SPELL_AURA_MOD_SHAPESHIFT = 36,
    SPELL_AURA_EFFECT_IMMUNITY = 37,
    SPELL_AURA_STATE_IMMUNITY = 38,
    SPELL_AURA_SCHOOL_IMMUNITY = 39,
    SPELL_AURA_DAMAGE_IMMUNITY = 40,
    SPELL_AURA_DISPEL_IMMUNITY = 41,
    SPELL_AURA_PROC_TRIGGER_SPELL = 42,
    SPELL_AURA_PROC_TRIGGER_DAMAGE = 43,
    SPELL_AURA_TRACK_CREATURES = 44,
    SPELL_AURA_TRACK_RESOURCES = 45,
    SPELL_AURA_MOD_PARRY_PERCENT = 47,
    SPELL_AURA_MOD_DODGE_PERCENT = 49,
    SPELL_AURA_MOD_CRITICAL_HEALING_AMOUNT = 50,
    SPELL_AURA_MOD_BLOCK_PERCENT = 51,
    SPELL_AURA_MOD_WEAPON_CRIT_PERCENT = 52,
    SPELL_AURA_PERIODIC_LEECH = 53,
    SPELL_AURA_MOD_HIT_CHANCE = 54,
    SPELL_AURA_MOD_SPELL_HIT_CHANCE = 55,
    SPELL_AURA_TRANSFORM = 56,
    SPELL_AURA_MOD_SPELL_CRIT_CHANCE = 57,
    SPELL_AURA_MOD_INCREASE_SWIM_SPEED = 58,
    SPELL_AURA_MOD_DAMAGE_DONE_CREATURE = 59,
    SPELL_AURA_MOD_PACIFY_SILENCE = 60,
    SPELL_AURA_MOD_SCALE = 61,
    SPELL_AURA_PERIODIC_HEALTH_FUNNEL = 62,
    SPELL_AURA_PERIODIC_MANA_LEECH = 64,
    SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK = 65,
    SPELL_AURA_FEIGN_DEATH = 66,
    SPELL_AURA_MOD_DISARM = 67,
    SPELL_AURA_MOD_STALKED = 68,
    SPELL_AURA_SCHOOL_ABSORB = 69,
    SPELL_AURA_EXTRA_ATTACKS = 70,
    SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL = 71,
    SPELL_AURA_MOD_POWER_COST = 72,
    SPELL_AURA_MOD_POWER_COST_SCHOOL = 73,
    SPELL_AURA_REFLECT_SPELLS_SCHOOL = 74,
    SPELL_AURA_MOD_LANGUAGE = 75,
    SPELL_AURA_FAR_SIGHT = 76,
    SPELL_AURA_MECHANIC_IMMUNITY = 77,
    SPELL_AURA_MOUNTED = 78,
    SPELL_AURA_MOD_DAMAGE_PERCENT_DONE = 79,
    SPELL_AURA_MOD_PERCENT_STAT = 80,
    SPELL_AURA_SPLIT_DAMAGE_PCT = 81,
    SPELL_AURA_WATER_BREATHING = 82,
    SPELL_AURA_MOD_BASE_RESISTANCE = 83,
    SPELL_AURA_MOD_REGEN = 84,
    SPELL_AURA_MOD_POWER_REGEN = 85,
    SPELL_AURA_CHANNEL_DEATH_ITEM = 86,
    SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN = 87,
    SPELL_AURA_MOD_HEALTH_REGEN_PERCENT = 88,
    SPELL_AURA_PERIODIC_DAMAGE_PERCENT = 89,
    SPELL_AURA_MOD_DETECT_RANGE = 91,
    SPELL_AURA_PREVENTS_FLEEING = 92,
    SPELL_AURA_MOD_UNATTACKABLE = 93,
    SPELL_AURA_INTERRUPT_REGEN = 94,
    SPELL_AURA_GHOST = 95,
    SPELL_AURA_SPELL_MAGNET = 96,
    SPELL_AURA_MANA_SHIELD = 97,
    SPELL_AURA_MOD_SKILL_TALENT = 98,
    SPELL_AURA_MOD_ATTACK_POWER = 99,
    SPELL_AURA_AURAS_VISIBLE = 100,
    SPELL_AURA_MOD_RESISTANCE_PCT = 101,
    SPELL_AURA_MOD_CREATURE_ATTACK_POWER = 102,
    SPELL_AURA_MOD_TOTAL_THREAT = 103,
    SPELL_AURA_WATER_WALK = 104,
    SPELL_AURA_FEATHER_FALL = 105,
    SPELL_AURA_HOVER = 106,
    SPELL_AURA_ADD_FLAT_MODIFIER = 107,
    SPELL_AURA_ADD_PCT_MODIFIER = 108,
    SPELL_AURA_ADD_TARGET_TRIGGER = 109,
    SPELL_AURA_MOD_POWER_REGEN_PERCENT = 110,
    SPELL_AURA_ADD_CASTER_HIT_TRIGGER = 111,
    SPELL_AURA_OVERRIDE_CLASS_SCRIPTS = 112,
    SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN = 113,
    SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT = 114,
    SPELL_AURA_MOD_HEALING = 115,
    SPELL_AURA_MOD_REGEN_DURING_COMBAT = 116,
    SPELL_AURA_MOD_MECHANIC_RESISTANCE = 117,
    SPELL_AURA_MOD_HEALING_PCT = 118,
    SPELL_AURA_UNTRACKABLE = 120,
    SPELL_AURA_EMPATHY = 121,
    SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT = 122,
    SPELL_AURA_MOD_TARGET_RESISTANCE = 123,
    SPELL_AURA_MOD_RANGED_ATTACK_POWER = 124,
    SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN = 125,
    SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT = 126,
    SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS = 127,
    SPELL_AURA_MOD_POSSESS_PET = 128,
    SPELL_AURA_MOD_SPEED_ALWAYS = 129,
    SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS = 130,
    SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER = 131,
    SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT = 132,
    SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT = 133,
    SPELL_AURA_MOD_MANA_REGEN_INTERRUPT = 134,
    SPELL_AURA_MOD_HEALING_DONE = 135,
    SPELL_AURA_MOD_HEALING_DONE_PERCENT = 136,
    SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE = 137,
    SPELL_AURA_MOD_MELEE_HASTE = 138,
    SPELL_AURA_FORCE_REACTION = 139,
    SPELL_AURA_MOD_RANGED_HASTE = 140,
    SPELL_AURA_MOD_RANGED_AMMO_HASTE = 141,
    SPELL_AURA_MOD_BASE_RESISTANCE_PCT = 142,
    SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE = 143,
    SPELL_AURA_SAFE_FALL = 144,
    SPELL_AURA_MOD_PET_TALENT_POINTS = 145,
    SPELL_AURA_ALLOW_TAME_PET_TYPE = 146,
    SPELL_AURA_ADD_CREATURE_IMMUNITY = 147,
    SPELL_AURA_RETAIN_COMBO_POINTS = 148,
    SPELL_AURA_REDUCE_PUSHBACK  = 149,
    SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT = 150,
    SPELL_AURA_TRACK_STEALTHED  = 151,
    SPELL_AURA_MOD_DETECTED_RANGE = 152,
    SPELL_AURA_SPLIT_DAMAGE_FLAT = 153,
    SPELL_AURA_MOD_STEALTH_LEVEL = 154,
    SPELL_AURA_MOD_WATER_BREATHING = 155,
    SPELL_AURA_MOD_REPUTATION_GAIN = 156,
    SPELL_AURA_PET_DAMAGE_MULTI = 157,
    SPELL_AURA_MOD_SHIELD_BLOCKVALUE = 158,
    SPELL_AURA_NO_PVP_CREDIT = 159,
    SPELL_AURA_MOD_AOE_AVOIDANCE = 160,
    SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT = 161,
    SPELL_AURA_POWER_BURN_MANA = 162,
    SPELL_AURA_MOD_CRIT_DAMAGE_BONUS_MELEE = 163,
    SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS = 165,
    SPELL_AURA_MOD_ATTACK_POWER_PCT = 166,
    SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT = 167,
    SPELL_AURA_MOD_DAMAGE_DONE_VERSUS = 168,
    SPELL_AURA_MOD_CRIT_PERCENT_VERSUS = 169,
    SPELL_AURA_DETECT_AMORE = 170,
    SPELL_AURA_MOD_SPEED_NOT_STACK = 171,
    SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK = 172,
    SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT = 174,
    SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT = 175,
    SPELL_AURA_SPIRIT_OF_REDEMPTION = 176,
    SPELL_AURA_AOE_CHARM = 177,
    SPELL_AURA_MOD_DEBUFF_RESISTANCE = 178,
    SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE = 179,
    SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS = 180,
    SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT = 182,
    SPELL_AURA_MOD_CRITICAL_THREAT = 183,
    SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE = 184,
    SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE= 185,
    SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE = 186,
    SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE = 187,
    SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE = 188,
    SPELL_AURA_MOD_RATING = 189,
    SPELL_AURA_MOD_FACTION_REPUTATION_GAIN = 190,
    SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED = 191,
    SPELL_AURA_MOD_MELEE_RANGED_HASTE = 192,
    SPELL_AURA_MELEE_SLOW = 193,
    SPELL_AURA_MOD_TARGET_ABSORB_SCHOOL  = 194,
    SPELL_AURA_MOD_TARGET_ABILITY_ABSORB_SCHOOL  = 195,
    SPELL_AURA_MOD_COOLDOWN = 196,
    SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE = 197,
    SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT = 199,
    SPELL_AURA_MOD_XP_PCT = 200,
    SPELL_AURA_FLY = 201,
    SPELL_AURA_IGNORE_COMBAT_RESULT = 202,
    SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE = 203,
    SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE = 204,
    SPELL_AURA_MOD_SCHOOL_CRIT_DMG_TAKEN = 205,
    SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED = 206,
    SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED = 207,
    SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED = 208,
    SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS = 209,
    SPELL_AURA_MOD_VEHICLE_SPEED_ALWAYS = 210,
    SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK = 211,
    SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT = 212,
    SPELL_AURA_MOD_RAGE_FROM_DAMAGE_DEALT = 213,
    SPELL_AURA_ARENA_PREPARATION = 215,
    SPELL_AURA_HASTE_SPELLS = 216,
    SPELL_AURA_HASTE_RANGED = 218,
    SPELL_AURA_MOD_MANA_REGEN_FROM_STAT = 219,
    SPELL_AURA_MOD_RATING_FROM_STAT = 220,
    SPELL_AURA_MOD_DETAUNT = 221,
    SPELL_AURA_RAID_PROC_FROM_CHARGE = 223,
    SPELL_AURA_RAID_PROC_FROM_CHARGE_WITH_VALUE = 225,
    SPELL_AURA_PERIODIC_DUMMY = 226,
    SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE = 227,
    SPELL_AURA_DETECT_STEALTH = 228,
    SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE = 229,
    SPELL_AURA_MOD_INCREASE_MAX_HEALTH = 230,
    SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE = 231,
    SPELL_AURA_MECHANIC_DURATION_MOD = 232,
    SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK = 234,
    SPELL_AURA_MOD_DISPEL_RESIST = 235,
    SPELL_AURA_CONTROL_VEHICLE = 236,
    SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER = 237,
    SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER = 238,
    SPELL_AURA_MOD_SCALE_2 = 239,
    SPELL_AURA_MOD_EXPERTISE = 240,
    SPELL_AURA_FORCE_MOVE_FORWARD = 241,
    SPELL_AURA_MOD_SPELL_DAMAGE_FROM_HEALING = 242,
    SPELL_AURA_MOD_FACTION = 243,
    SPELL_AURA_COMPREHEND_LANGUAGE = 244,
    SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL = 245,
    SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL_NOT_STACK = 246,
    SPELL_AURA_CLONE_CASTER = 247,
    SPELL_AURA_MOD_COMBAT_RESULT_CHANCE = 248,
    SPELL_AURA_CONVERT_RUNE = 249,
    SPELL_AURA_MOD_INCREASE_HEALTH_2 = 250,
    SPELL_AURA_MOD_ENEMY_DODGE = 251,
    SPELL_AURA_MOD_SLOW_ALL = 252,
    SPELL_AURA_MOD_BLOCK_CRIT_CHANCE = 253,
    SPELL_AURA_MOD_DISARM_OFFHAND = 254,
    SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT = 255,
    SPELL_AURA_NO_REAGENT_USE = 256,
    SPELL_AURA_MOD_TARGET_RESIST_BY_SPELL_CLASS = 257,
    SPELL_AURA_MOD_HOT_PCT = 259,
    SPELL_AURA_SCREEN_EFFECT = 260,
    SPELL_AURA_PHASE = 261,
    SPELL_AURA_ABILITY_IGNORE_AURASTATE = 262,
    SPELL_AURA_ALLOW_ONLY_ABILITY = 263,
    SPELL_AURA_MOD_IMMUNE_AURA_APPLY_SCHOOL = 267,
    SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT = 268,
    SPELL_AURA_MOD_IGNORE_TARGET_RESIST = 269,
    SPELL_AURA_MOD_ABILITY_IGNORE_TARGET_RESIST = 270,
    SPELL_AURA_MOD_DAMAGE_FROM_CASTER = 271,
    SPELL_AURA_IGNORE_MELEE_RESET = 272,
    SPELL_AURA_X_RAY = 273,
    SPELL_AURA_ABILITY_CONSUME_NO_AMMO = 274,
    SPELL_AURA_MOD_IGNORE_SHAPESHIFT = 275,
    SPELL_AURA_MOD_MAX_AFFECTED_TARGETS = 277,
    SPELL_AURA_MOD_DISARM_RANGED = 278,
    SPELL_AURA_INITIALIZE_IMAGES = 279,
    SPELL_AURA_MOD_ARMOR_PENETRATION_PCT = 280,
    SPELL_AURA_MOD_HONOR_GAIN_PCT = 281,
    SPELL_AURA_MOD_BASE_HEALTH_PCT = 282,
    SPELL_AURA_MOD_HEALING_RECEIVED = 283,
    SPELL_AURA_LINKED = 284,
    SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR = 285,
    SPELL_AURA_ABILITY_PERIODIC_CRIT = 286,
    SPELL_AURA_DEFLECT_SPELLS = 287,
    SPELL_AURA_IGNORE_HIT_DIRECTION = 288,
    SPELL_AURA_MOD_CRIT_PCT = 290,
    SPELL_AURA_MOD_XP_QUEST_PCT = 291,
    SPELL_AURA_OPEN_STABLE = 292,
    SPELL_AURA_OVERRIDE_SPELLS = 293,
    SPELL_AURA_PREVENT_REGENERATE_POWER = 294,
    SPELL_AURA_SET_VEHICLE_ID = 296,
    SPELL_AURA_BLOCK_SPELL_FAMILY = 297,
    SPELL_AURA_STRANGULATE = 298,
    SPELL_AURA_SHARE_DAMAGE_PCT = 300,
    SPELL_AURA_SCHOOL_HEAL_ABSORB = 301,
    SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE = 303,
    SPELL_AURA_MOD_FAKE_INEBRIATE = 304,
    SPELL_AURA_MOD_MINIMUM_SPEED = 305,
    SPELL_AURA_HEAL_ABSORB_TEST = 307,
    SPELL_AURA_MOD_CREATURE_AOE_DAMAGE_AVOIDANCE = 310,
    SPELL_AURA_PREVENT_RESSURECTION = 314,
    SPELL_AURA_UNDERWATER_WALKING = 315,
    SPELL_AURA_PERIODIC_HASTE = 316,
    SPELL_AURA_TOTAL = 317
};

enum AuraTickFlags
{
    FLAG_PERIODIC_DAMAGE            = SPELL_AURA_PERIODIC_DAMAGE,   // also 89
    FLAG_PERIODIC_HEAL              = SPELL_AURA_PERIODIC_HEAL,     // also 20
    FLAG_PERIODIC_ENERGIZE          = SPELL_AURA_PERIODIC_ENERGIZE, // also 24
    FLAG_PERIODIC_LEECH             = SPELL_AURA_PERIODIC_LEECH,
};

enum AuraFlags
{
    AFLAG_NONE              = 0x00,
    AFLAG_EFF_INDEX_0       = 0x01,
    AFLAG_EFF_INDEX_1       = 0x02,
    AFLAG_EFF_INDEX_2       = 0x04,
    AFLAG_NOT_GUID          = 0x08,
    AFLAG_HAS_DURATION      = 0x20,
    AFLAG_EFF_AMOUNT_SEND   = 0x40,
    AFLAG_NEGATIVE          = 0x80,
    AFLAG_POSITIVE          = 0x1F,
    AFLAG_MASK              = 0xFF
};

enum AURAIMMUNE_FLAG
{
    AURAIMMUNE_NONE             = 0x0,
    AURAIMMUNE_CONFUSE          = 0X1,          //SPELL_AURA_MOD_CONFUSE = 5
    AURAIMMUNE_CHARM            = 0x2,          //SPELL_AURA_MOD_CHARM = 6
    AURAIMMUNE_FEAR             = 0X4,          //SPELL_AURA_MOD_FEAR = 7
    AURAIMMUNE_STUN             = 0x8,          //SPELL_AURA_MOD_STUN = 12
    AURAIMMUNE_PACIFY           = 0x10,         //SPELL_AURA_MOD_PACIFY = 25
    AURAIMMUNE_ROOT             = 0x20,         //SPELL_AURA_MOD_ROOT = 26
    AURAIMMUNE_SILENCE          = 0x40,         //SPELL_AURA_MOD_SILENCE = 27
    AURAIMMUNE_INCSPEED         = 0x80,         //SPELL_AURA_MOD_INCREASE_SPEED = 31
    AURAIMMUNE_DECSPEED         = 0x100,        //SPELL_AURA_MOD_DECREASE_SPEED = 33
    AURAIMMUNE_TRANSFORM        = 0x200,        //SPELL_AURA_TRANSFORM = 56
    AURAIMMUNE_TAUNT            = 0x400,        //SPELL_AURA_MOD_TAUNT = 11
    AURAIMMUNE_BLEED            = 0x800,        //MECHANIC BLEED = 15
    AURAIMMUNE_POISON           = 0x1000,       //DISPEL TYPE POISON = 4
    AURAIMMUNE_MANALEECH        = 0x2000,       //SPELL_AURA_PERIODIC_MANA_LEECH = 64,
    AURAIMMUNE_LEECH            = 0x4000,       //SPELL_AURA_PERIODIC_LEECH = 53

    AURAIMMUNE_GENERIC_BOSS     = AURAIMMUNE_CONFUSE | AURAIMMUNE_CHARM | AURAIMMUNE_FEAR |
                                    AURAIMMUNE_STUN | AURAIMMUNE_PACIFY | AURAIMMUNE_ROOT |
                                    AURAIMMUNE_SILENCE | AURAIMMUNE_INCSPEED | AURAIMMUNE_DECSPEED |
                                    AURAIMMUNE_TRANSFORM, //1023
};

enum SpellCastError
{
    SPELL_FAILED_SUCCESS                                = 0,
    SPELL_FAILED_AFFECTING_COMBAT                       = 1,
    SPELL_FAILED_ALREADY_AT_FULL_HEALTH                 = 2,
    SPELL_FAILED_ALREADY_AT_FULL_MANA                   = 3,
    SPELL_FAILED_ALREADY_AT_FULL_POWER                  = 4,
    SPELL_FAILED_ALREADY_BEING_TAMED                    = 5,
    SPELL_FAILED_ALREADY_HAVE_CHARM                     = 6,
    SPELL_FAILED_ALREADY_HAVE_SUMMON                    = 7,
    SPELL_FAILED_ALREADY_HAVE_PET                       = 8,
    SPELL_FAILED_ALREADY_OPEN                           = 9,
    SPELL_FAILED_AURA_BOUNCED                           = 10,
    SPELL_FAILED_AUTOTRACK_INTERRUPTED                  = 11,
    SPELL_FAILED_BAD_IMPLICIT_TARGETS                   = 12,
    SPELL_FAILED_BAD_TARGETS                            = 13,
    SPELL_FAILED_CANT_BE_CHARMED                        = 14,
    SPELL_FAILED_CANT_BE_DISENCHANTED                   = 15,
    SPELL_FAILED_CANT_BE_DISENCHANTED_SKILL             = 16,
    SPELL_FAILED_CANT_BE_MILLED                         = 17,
    SPELL_FAILED_CANT_BE_PROSPECTED                     = 18,
    SPELL_FAILED_CANT_CAST_ON_TAPPED                    = 19,
    SPELL_FAILED_CANT_DUEL_WHILE_INVISIBLE              = 20,
    SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED              = 21,
    SPELL_FAILED_CANT_STEALTH                           = 22,
    SPELL_FAILED_CASTER_AURASTATE                       = 23,
    SPELL_FAILED_CASTER_DEAD                            = 24,
    SPELL_FAILED_CHARMED                                = 25,
    SPELL_FAILED_CHEST_IN_USE                           = 26,
    SPELL_FAILED_CONFUSED                               = 27,
    SPELL_FAILED_DONT_REPORT                            = 28,
    SPELL_FAILED_EQUIPPED_ITEM                          = 29,
    SPELL_FAILED_EQUIPPED_ITEM_CLASS                    = 30,
    SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND           = 31,
    SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND            = 32,
    SPELL_FAILED_ERROR                                  = 33,
    SPELL_FAILED_FALLING                                = 34,
    SPELL_FAILED_FIZZLE                                 = 35,
    SPELL_FAILED_FLEEING                                = 36,
    SPELL_FAILED_FOOD_LOWLEVEL                          = 37,
    SPELL_FAILED_HIGHLEVEL                              = 38,
    SPELL_FAILED_HUNGER_SATIATED                        = 39,
    SPELL_FAILED_IMMUNE                                 = 40,
    SPELL_FAILED_INCORRECT_AREA                         = 41,
    SPELL_FAILED_INTERRUPTED                            = 42,
    SPELL_FAILED_INTERRUPTED_COMBAT                     = 43,
    SPELL_FAILED_ITEM_ALREADY_ENCHANTED                 = 44,
    SPELL_FAILED_ITEM_GONE                              = 45,
    SPELL_FAILED_ITEM_NOT_FOUND                         = 46,
    SPELL_FAILED_ITEM_NOT_READY                         = 47,
    SPELL_FAILED_LEVEL_REQUIREMENT                      = 48,
    SPELL_FAILED_LINE_OF_SIGHT                          = 49,
    SPELL_FAILED_LOWLEVEL                               = 50,
    SPELL_FAILED_LOW_CASTLEVEL                          = 51,
    SPELL_FAILED_MAINHAND_EMPTY                         = 52,
    SPELL_FAILED_MOVING                                 = 53,
    SPELL_FAILED_NEED_AMMO                              = 54,
    SPELL_FAILED_NEED_AMMO_POUCH                        = 55,
    SPELL_FAILED_NEED_EXOTIC_AMMO                       = 56,
    SPELL_FAILED_NEED_MORE_ITEMS                        = 57,
    SPELL_FAILED_NOPATH                                 = 58,
    SPELL_FAILED_NOT_BEHIND                             = 59,
    SPELL_FAILED_NOT_FISHABLE                           = 60,
    SPELL_FAILED_NOT_FLYING                             = 61,
    SPELL_FAILED_NOT_HERE                               = 62,
    SPELL_FAILED_NOT_INFRONT                            = 63,
    SPELL_FAILED_NOT_IN_CONTROL                         = 64,
    SPELL_FAILED_NOT_KNOWN                              = 65,
    SPELL_FAILED_NOT_MOUNTED                            = 66,
    SPELL_FAILED_NOT_ON_TAXI                            = 67,
    SPELL_FAILED_NOT_ON_TRANSPORT                       = 68,
    SPELL_FAILED_NOT_READY                              = 69,
    SPELL_FAILED_NOT_SHAPESHIFT                         = 70,
    SPELL_FAILED_NOT_STANDING                           = 71,
    SPELL_FAILED_NOT_TRADEABLE                          = 72,
    SPELL_FAILED_NOT_TRADING                            = 73,
    SPELL_FAILED_NOT_UNSHEATHED                         = 74,
    SPELL_FAILED_NOT_WHILE_GHOST                        = 75,
    SPELL_FAILED_NOT_WHILE_LOOTING                      = 76,
    SPELL_FAILED_NO_AMMO                                = 77,
    SPELL_FAILED_NO_CHARGES_REMAIN                      = 78,
    SPELL_FAILED_NO_CHAMPION                            = 79,
    SPELL_FAILED_NO_COMBO_POINTS                        = 80,
    SPELL_FAILED_NO_DUELING                             = 81,
    SPELL_FAILED_NO_ENDURANCE                           = 82,
    SPELL_FAILED_NO_FISH                                = 83,
    SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED            = 84,
    SPELL_FAILED_NO_MOUNTS_ALLOWED                      = 85,
    SPELL_FAILED_NO_PET                                 = 86,
    SPELL_FAILED_NO_POWER                               = 87,
    SPELL_FAILED_NOTHING_TO_DISPEL                      = 88,
    SPELL_FAILED_NOTHING_TO_STEAL                       = 89,
    SPELL_FAILED_ONLY_ABOVEWATER                        = 90,
    SPELL_FAILED_ONLY_DAYTIME                           = 91,
    SPELL_FAILED_ONLY_INDOORS                           = 92,
    SPELL_FAILED_ONLY_MOUNTED                           = 93,
    SPELL_FAILED_ONLY_NIGHTTIME                         = 94,
    SPELL_FAILED_ONLY_OUTDOORS                          = 95,
    SPELL_FAILED_ONLY_SHAPESHIFT                        = 96,
    SPELL_FAILED_ONLY_STEALTHED                         = 97,
    SPELL_FAILED_ONLY_UNDERWATER                        = 98,
    SPELL_FAILED_OUT_OF_RANGE                           = 99,
    SPELL_FAILED_PACIFIED                               = 100,
    SPELL_FAILED_POSSESSED                              = 101,
    SPELL_FAILED_REAGENTS                               = 102,
    SPELL_FAILED_REQUIRES_AREA                          = 103,
    SPELL_FAILED_REQUIRES_SPELL_FOCUS                   = 104,
    SPELL_FAILED_ROOTED                                 = 105,
    SPELL_FAILED_SILENCED                               = 106,
    SPELL_FAILED_SPELL_IN_PROGRESS                      = 107,
    SPELL_FAILED_SPELL_LEARNED                          = 108,
    SPELL_FAILED_SPELL_UNAVAILABLE                      = 109,
    SPELL_FAILED_STUNNED                                = 110,
    SPELL_FAILED_TARGETS_DEAD                           = 111,
    SPELL_FAILED_TARGET_AFFECTING_COMBAT                = 112,
    SPELL_FAILED_TARGET_AURASTATE                       = 113,
    SPELL_FAILED_TARGET_DUELING                         = 114,
    SPELL_FAILED_TARGET_ENEMY                           = 115,
    SPELL_FAILED_TARGET_ENRAGED                         = 116,
    SPELL_FAILED_TARGET_FRIENDLY                        = 117,
    SPELL_FAILED_TARGET_IN_COMBAT                       = 118,
    SPELL_FAILED_TARGET_IS_PLAYER                       = 119,
    SPELL_FAILED_TARGET_IS_PLAYER_CONTROLLED            = 120,
    SPELL_FAILED_TARGET_NOT_DEAD                        = 121,
    SPELL_FAILED_TARGET_NOT_IN_PARTY                    = 122,
    SPELL_FAILED_TARGET_NOT_LOOTED                      = 123,
    SPELL_FAILED_TARGET_NOT_PLAYER                      = 124,
    SPELL_FAILED_TARGET_NO_POCKETS                      = 125,
    SPELL_FAILED_TARGET_NO_WEAPONS                      = 126,
    SPELL_FAILED_TARGET_NO_RANGED_WEAPONS               = 127,
    SPELL_FAILED_TARGET_UNSKINNABLE                     = 128,
    SPELL_FAILED_THIRST_SATIATED                        = 129,
    SPELL_FAILED_TOO_CLOSE                              = 130,
    SPELL_FAILED_TOO_MANY_OF_ITEM                       = 131,
    SPELL_FAILED_TOTEM_CATEGORY                         = 132,
    SPELL_FAILED_TOTEMS                                 = 133,
    SPELL_FAILED_TRY_AGAIN                              = 134,
    SPELL_FAILED_UNIT_NOT_BEHIND                        = 135,
    SPELL_FAILED_UNIT_NOT_INFRONT                       = 136,
    SPELL_FAILED_VISION_OBSCURED                        = 137,
    SPELL_FAILED_WRONG_PET_FOOD                         = 138,
    SPELL_FAILED_NOT_WHILE_FATIGUED                     = 139,
    SPELL_FAILED_TARGET_NOT_IN_INSTANCE                 = 140,
    SPELL_FAILED_NOT_WHILE_TRADING                      = 141,
    SPELL_FAILED_TARGET_NOT_IN_RAID                     = 142,
    SPELL_FAILED_TARGET_FREEFORALL                      = 143,
    SPELL_FAILED_NO_EDIBLE_CORPSES                      = 144,
    SPELL_FAILED_ONLY_BATTLEGROUNDS                     = 145,
    SPELL_FAILED_TARGET_NOT_GHOST                       = 146,
    SPELL_FAILED_TRANSFORM_UNUSABLE                     = 147,
    SPELL_FAILED_WRONG_WEATHER                          = 148,
    SPELL_FAILED_DAMAGE_IMMUNE                          = 149,
    SPELL_FAILED_PREVENTED_BY_MECHANIC                  = 150,
    SPELL_FAILED_PLAY_TIME                              = 151,
    SPELL_FAILED_REPUTATION                             = 152,
    SPELL_FAILED_MIN_SKILL                              = 153,
    SPELL_FAILED_NOT_IN_RATED_BATTLEGROUND              = 154,
    SPELL_FAILED_NOT_ON_SHAPESHIFT                      = 155,
    SPELL_FAILED_NOT_ON_STEALTHED                       = 156,
    SPELL_FAILED_NOT_ON_DAMAGE_IMMUNE                   = 157,
    SPELL_FAILED_NOT_ON_MOUNTED                         = 158,
    SPELL_FAILED_TOO_SHALLOW                            = 159,
    SPELL_FAILED_TARGET_NOT_IN_SANCTUARY                = 160,
    SPELL_FAILED_TARGET_IS_TRIVIAL                      = 161,
    SPELL_FAILED_BM_OR_INVISGOD                         = 162,
    SPELL_FAILED_EXPERT_RIDING_REQUIREMENT              = 163,
    SPELL_FAILED_ARTISAN_RIDING_REQUIREMENT             = 164,
    SPELL_FAILED_NOT_IDLE                               = 165,
    SPELL_FAILED_NOT_INACTIVE                           = 166,
    SPELL_FAILED_PARTIAL_PLAYTIME                       = 167,
    SPELL_FAILED_NO_PLAYTIME                            = 168,
    SPELL_FAILED_NOT_IN_BATTLEGROUND                    = 169,
    SPELL_FAILED_NOT_IN_RAID_INSTANCE                   = 170,
    SPELL_FAILED_ONLY_IN_ARENA                          = 171,
    SPELL_FAILED_TARGET_LOCKED_TO_RAID_INSTANCE         = 172,
    SPELL_FAILED_ON_USE_ENCHANT                         = 173,
    SPELL_FAILED_NOT_ON_GROUND                          = 174,
    SPELL_FAILED_CUSTOM_ERROR                           = 175,
    SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW                 = 176,
    SPELL_FAILED_TOO_MANY_SOCKETS                       = 177,
    SPELL_FAILED_INVALID_GLYPH                          = 178,
    SPELL_FAILED_UNIQUE_GLYPH                           = 179,
    SPELL_FAILED_GLYPH_SOCKET_LOCKED                    = 180,
    SPELL_FAILED_NO_VALID_TARGETS                       = 181,
    SPELL_FAILED_ITEM_AT_MAX_CHARGES                    = 182,
    SPELL_FAILED_NOT_IN_BARBERSHOP                      = 183,
    SPELL_FAILED_FISHING_TOO_LOW                        = 184,
    SPELL_FAILED_ITEM_ENCHANT_TRADE_WINDOW              = 185,
    SPELL_FAILED_SUMMON_PENDING                         = 186,
    SPELL_FAILED_MAX_SOCKETS                            = 187,
    SPELL_FAILED_PET_CAN_RENAME                         = 188,
    SPELL_FAILED_TARGET_CANNOT_BE_RESURRECTED           = 189,
    SPELL_FAILED_NO_ACTIONS                             = 190,
    SPELL_FAILED_CURRENCY_WEIGHT_MISMATCH               = 191,
    SPELL_FAILED_WEIGHT_NOT_ENOUGH                      = 192,
    SPELL_FAILED_WEIGHT_TOO_MUCH                        = 193,
    SPELL_FAILED_NO_VACANT_SEAT                         = 194,
    SPELL_FAILED_NO_LIQUID                              = 195,
    SPELL_FAILED_ONLY_NOT_SWIMMING                      = 196,
    SPELL_FAILED_BY_NOT_MOVING                          = 197,
    SPELL_FAILED_IN_COMBAT_RES_LIMIT_REACHED            = 198,
    SPELL_FAILED_NOT_IN_ARENA                           = 199,
    SPELL_FAILED_TARGET_NOT_GROUNDED                    = 200,
    SPELL_FAILED_EXCEEDED_WEEKLY_USAGE                  = 201,
    SPELL_FAILED_NOT_IN_LFG_DUNGEON                     = 202,
    // Following are custom
    SPELL_FAILED_UNKNOWN                                = 254,
    SPELL_CANCAST_OK,
};

//wooohooo, there are 19 spells that actually require to add a proccounter for these
//first spell catched is "presence of mind"
//an ugly solution would be to add a proc flag to remove aura on event it should expire (like attack or cast) but that is only if count=1
enum SPELL_MODIFIER_TYPE
{
    SMT_DAMAGE_DONE             = 0,// increases the damage done by spell by x% dmg (flat as x dmg)
    SMT_DURATION                = 1,// spell    duration increase   // GOOD
    //hmm shaman spirit weapon clearly states that this should be parry chance
    SMT_THREAT_REDUCED          = 2,// reduces threat generated by this spell by x% // GOOD but need more work
    SMT_FIRST_EFFECT_BONUS      = 3,// modifies points of second spell effect   // GOOD
    SMT_CHARGES                 = 4,// increases the number of charges
    SMT_RANGE                   = 5,// spell range bonus    // GOOD
    SMT_RADIUS                  = 6,// spell radius bonus   // GOOD
    SMT_CRITICAL                = 7,// critical chance bonus    // GOOD //only 1 example in 2.1.1
    SMT_MISC_EFFECT             = 8,// dummy effect // needs work is not only pets Hp but effect of spells to
    SMT_NONINTERRUPT            = 9,// x% chance not to be interrupted by received damage (no flat)
    SMT_CAST_TIME               = 10,// cast time decrease  // GOOD
    SMT_COOLDOWN_DECREASE       = 11,// cooldown decrease <-probably fully handled by client    // GOOD
    SMT_SECOND_EFFECT_BONUS     = 12,// modifies points of second spell effect  // GOOD
    // 13 dont exist spells with it
    SMT_COST                    = 14,// mana/energy/rage cost reduction // GOOD
    SMT_CRITICAL_DAMAGE         = 15,// increases critical strike damage bonus (no flat)
    SMT_HITCHANCE               = 16,// enemy resist chance decrease (flat as %)    // GOOD need work
    SMT_ADDITIONAL_TARGET       = 17,// Your Healing Wave will now jump to additional nearby targets. Each jump reduces the effectiveness of the heal by 80%    // GOOD
    SMT_PROC_CHANCE             = 18,// adds/increases chance to trigger some spell for example increase chance to apply poisons or entaglin    // GOOD need work
    SMT_TIME                    = 19,// delay for nova, redirection time bonus for totem,maybe smth else    // GOOD need work
    SMT_JUMP_REDUCE             = 20,// Increases the amount healed by Chain Heal to targets beyond the first by x%. (no flat)
    SMT_GLOBAL_COOLDOWN         = 21,// Reduces the global cooldown
    //!!! most spells have both SMT_DAMAGE_DONE and this value. Be carefull case there is no need to apply both !
    SMT_SPELL_VALUE_PCT         = 22,// damage done by ability by x% : SELECT id,name,description FROM dbc_spell where (EffectApplyAuraName_1=108 and EffectMiscValue_1=22) or (EffectApplyAuraName_2=108 and EffectMiscValue_2=22) or (EffectApplyAuraName_3=108 and EffectMiscValue_3=22) its DoT actually
    SMT_LAST_EFFECT_BONUS       = 23,// increases points of last effect in affected spells  // GOOD
    SMT_SP_BONUS                = 24,// This is a modifer for the amount of +spell damage applied to the spell group from spell bonuses
    // 25 dont exist spells with it
    // 26 is obsolete stuff
    SMT_MULTIPLE_VALUE          = 27,// mana lost cost per point of damage taken for mana shield,Health or Mana gained from Drain Life and Drain Mana increased by x%.
    SMT_RESIST_DISPEL           = 28,// GOOD
    // 29 Mod Crowd Damage Test 45365 - Increases the critical strike damage bonus of your Frost spells by 100%
};

enum DBC_SummonControlTypes
{
    DBC_SUMMON_CONTROL_TYPE_WILD        = 0,
    DBC_SUMMON_CONTROL_TYPE_GUARDIAN    = 1,
    DBC_SUMMON_CONTROL_TYPE_PET         = 2,
    DBC_SUMMON_CONTROL_TYPE_POSSESSED   = 3,
    DBC_SUMMON_CONTROL_TYPE_VEHICLE     = 4,
};

enum DBC_SUMMON_TYPE
{
    DBC_SUMMON_TYPE_NONE = 0,
    DBC_SUMMON_TYPE_PET = 1,
    DBC_SUMMON_TYPE_GUARDIAN = 2,
    DBC_SUMMON_TYPE_MINION = 3,
    DBC_SUMMON_TYPE_TOTEM = 4,
    DBC_SUMMON_TYPE_COMPANION = 5,
    DBC_SUMMON_TYPE_RUNEBLADE = 6,
    DBC_SUMMON_TYPE_CONSTRUCT = 7,
    DBC_SUMMON_TYPE_OPPONENT = 8,
    DBC_SUMMON_TYPE_VEHICLE = 9,
    DBC_SUMMON_TYPE_MOUNT = 10,
    DBC_SUMMON_TYPE_LIGHTWELL = 11,
    DBC_SUMMON_TYPE_BUTLER = 12
};

enum SPELL_INFRONT_STATUS
{
    SPELL_INFRONT_STATUS_REQUIRE_SKIPCHECK  = 0,
    SPELL_INFRONT_STATUS_REQUIRE_INFRONT    = 1,
    SPELL_INFRONT_STATUS_REQUIRE_INBACK     = 2,
};

enum SPELL_DMG_TYPE     // SPELL_ENTRY_Spell_Dmg_Type
{
    SPELL_DMG_TYPE_NONE     = 0,
    SPELL_DMG_TYPE_MAGIC    = 1,
    SPELL_DMG_TYPE_MELEE    = 2,
    SPELL_DMG_TYPE_RANGED   = 3
};

// value's for SendSpellLog
enum SPELL_LOG
{
    SPELL_LOG_NONE,
    SPELL_LOG_MISS,
    SPELL_LOG_RESIST,
    SPELL_LOG_DODGE,
    SPELL_LOG_PARRY,
    SPELL_LOG_BLOCK,
    SPELL_LOG_EVADE,
    SPELL_LOG_IMMUNE,
    SPELL_LOG_IMMUNE2,
    SPELL_LOG_DEFLECT,
    SPELL_LOG_ABSORB,
    SPELL_LOG_REFLECT
};

enum SpellCastTargetFlags
{
    TARGET_FLAG_SELF                = 0x00000000,   // they are checked in following order
    TARGET_FLAG_UNIT                = 0x00000002,
    TARGET_FLAG_UNIT_RAID           = 0x00000004,
    TARGET_FLAG_UNIT_PARTY          = 0x00000008,
    TARGET_FLAG_ITEM                = 0x00000010,
    TARGET_FLAG_SOURCE_LOCATION     = 0x00000020,
    TARGET_FLAG_DEST_LOCATION       = 0x00000040,
    TARGET_FLAG_OBJECT_CASTER       = 0x00000080,
    TARGET_FLAG_CASTER              = 0x00000100,
    TARGET_FLAG_PLR_CORPSE          = 0x00000200,
    TARGET_FLAG_CORPSE              = 0x00000400,
    TARGET_FLAG_OBJECT              = 0x00000800,
    TARGET_FLAG_TRADE_ITEM          = 0x00001000,
    TARGET_FLAG_STRING              = 0x00002000,
    TARGET_FLAG_ITEM2               = 0x00004000,
    TARGET_FLAG_CORPSE2             = 0x00008000,
    TARGET_FLAG_GLYPH               = 0x00020000,
    TARGET_FLAG_TARGET_DEST         = 0x00040000,
    TARGET_FLAG_EXTRA_TARGETS       = 0x00080000,
};

enum CastInterruptFlags
{
    CAST_INTERRUPT_NULL                 = 0x0,
    CAST_INTERRUPT_ON_SILENCE           = 0x1,
    CAST_INTERRUPT_ON_SLEEP             = 0x2,  // could be wrong
    CAST_INTERRUPT_ON_STUNNED           = 0x4,  // could be wrong
    CAST_INTERRUPT_ON_MOVEMENT          = 0x8,
    CAST_INTERRUPT_ON_DAMAGE_TAKEN      = 0x10
};

/************************************************************************/
/* General Spell Go Flags, for documentation reasons                    */
/************************************************************************/
enum SpellCastFlag
{
    SPELL_CASTFLAG_NONE                 = 0x00000000,
    SPELL_CASTFLAG_NO_VISUAL            = 0x00000001,
    SPELL_CASTFLAG_HAS_TRAJECTORY       = 0x00000002,
    SPELL_CASTFLAG_PROJECTILE           = 0x00000020,
    SPELL_CASTFLAG_POWER_UPDATE         = 0x00000800,
    SPELL_CASTFLAG_MISSILE_INFO         = 0x00020000,
    SPELL_CASTFLAG_NO_GCD               = 0x00040000,
    SPELL_CASTFLAG_VISUAL_CHAIN         = 0x00080000,
    SPELL_CASTFLAG_RUNE_UPDATE          = 0x00200000,
    SPELL_CASTFLAG_EXTRA_MESSAGE        = 0x04000000,
    SPELL_CASTFLAG_HEAL_UPDATE          = 0x40000000,
};

enum AuraInterruptFlags
{
    AURA_INTERRUPT_ON_HOSTILE_SPELL_INFLICTED   = 0x1,
    AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN          = 0x2,
    AURA_INTERRUPT_ON_MOVEMENT                  = 0x8,  // could be AURA_INTERRUPT_ON_MOVEMENT
    AURA_INTERRUPT_ON_TURNING                   = 0x10,
    AURA_INTERRUPT_ON_ENTER_COMBAT              = 0x20,
    AURA_INTERRUPT_ON_DISMOUNT                  = 0x40,
    AURA_INTERRUPT_ON_ENTER_WATER               = 0x80,
    AURA_INTERRUPT_ON_LEAVE_WATER               = 0x100, // could be AURA_INTERRUPT_ON_LEAVE_CURRENT_SURFACE
    AURA_INTERRUPT_ON_START_ATTACK              = 0x1000, // Auto Attack?
    AURA_INTERRUPT_ON_CAST_SPELL                = 0x8000,
    AURA_INTERRUPT_ON_MOUNT                     = 0x20000,
    AURA_INTERRUPT_ON_STAND_UP                  = 0x40000,
    AURA_INTERRUPT_ON_LEAVE_AREA                = 0x80000,
    AURA_INTERRUPT_ON_INVINCIBLE                = 0x100000,
    AURA_INTERRUPT_ON_STEALTH                   = 0x200000,
    AURA_INTERRUPT_ON_PVP_ENTER                 = 0x800000,
    AURA_INTERRUPT_ON_DIRECT_DAMAGE             = 0x1000000,
    AURA_INTERRUPT_ON_AFTER_CAST_SPELL          = 0x80000000,
};

enum ChannelInterruptFlags
{
    CHANNEL_INTERRUPT_ON_TAKE_DAMAGE    = 0x2,
    CHANNEL_INTERRUPT_ON_MOVEMENT       = 0x8,
    CHANNEL_INTERRUPT_ON_TURN           = 0x10,
    CHANNEL_INTERRUPT_ON_DAMAGE         = 0x80,
    CHANNEL_INTERRUPT_REQUIRES_WATER    = 0x100,
    CHANNEL_INTERRUPT_DELAY             = 0x4000,
};

enum Attributes
{
    ATTRIBUTES_REQ_AMMO                         = 0x2,  // requires ammo
    ATTRIBUTES_ON_NEXT_ATTACK                   = 0x4,
    ATTRIBUTES_ABILITY                          = 0x10,
    ATTRIBUTES_PASSIVE                          = 0x40,
    ATTRIBUTES_NO_VISUAL_AURA                   = 0x80,
    ATTRIBUTES_NO_CAST                          = 0x100,    //seems to be afflicts pet
    ATTRIBUTES_ON_NEXT_SWING_2                  = 0x400,    //completely the same as ATTRIBUTE_ON_NEXT_ATTACK for class spells. So difference somewhere in mob abilities.
    ATTRIBUTES_DAYTIME_ONLY                     = 0x1000,
    ATTRIBUTES_NIGHT_ONLY                       = 0x2000,
    ATTRIBUTES_INDOORS_ONLY                     = 0x4000,
    ATTRIBUTES_ONLY_OUTDOORS                    = 0x8000,
    ATTRIBUTES_NOT_SHAPESHIFT                   = 0x10000,
    ATTRIBUTES_REQ_STEALTH                      = 0x20000,
    ATTRIBUTES_LEVEL_DAMAGE_CALCULATION         = 0x80000, //spelldamage depends on caster level
    ATTRIBUTES_STOP_ATTACK                      = 0x100000,//switch off auto attack on use. Maim,Gouge,Disengage,Polymorph etc
    ATTRIBUTES_CANT_BE_DPB                      = 0x200000,//can't be dodged, blocked, parried
    ATTRIBUTES_CASTABLE_WHILE_DEAD              = 0x800000,
    ATTRIBUTES_MOUNT_CASTABLE                   = 0x1000000,    //castable on mounts
    ATTRIBUTES_TRIGGER_COOLDOWN                 = 0x2000000,    //also requires atributes ex    = 32 ?
    ATTRIBUTES_CANCAST_WHILE_SITTING            = 0x8000000,
    ATTRIBUTES_REQ_OOC                          = 0x10000000,   // ATTRIBUTES_REQ_OUT_OF_COMBAT
    ATTRIBUTES_IGNORE_INVULNERABILITY           = 0x20000000,   // debuffs that can't be removed by any spell and spells that can't be resisted in any case
    ATTRIBUTES_CANT_CANCEL                      = 0x80000000
};

enum SpellCastFlags
{
    CAST_FLAG_UNKNOWN1                  = 0x2,
    CAST_FLAG_UNKNOWN2                  = 0x10, // no idea yet, i saw it in blizzard spell
    CAST_FLAG_AMMO                      = 0x20  // load ammo display id (uint32) and ammo inventory type (uint32)
};

enum School
{
    SCHOOL_NORMAL   = 0,
    SCHOOL_HOLY     = 1,
    SCHOOL_FIRE     = 2,
    SCHOOL_NATURE   = 3,
    SCHOOL_FROST    = 4,
    SCHOOL_SHADOW   = 5,
    SCHOOL_ARCANE   = 6,
    SCHOOL_SPELL    = 7,
    SCHOOL_MAGIC    = 8,
    SCHOOL_ALL      = 9
};

// converting schools for 2.4.0 client
static const uint32 spellMaskArray[SCHOOL_ALL+1] = {
    1,              // SCHOOL_NORMAL
    2,              // SCHOOL_HOLY
    4,              // SCHOOL_FIRE
    8,              // SCHOOL_NATURE
    16,             // SCHOOL_FROST
    32,             // SCHOOL_SHADOW
    64,             // SCHOOL_ARCANE
    124,            // SCHOOL_SPELL
    126,            // SCHOOL_MAGIC
    127             // SCHOOL_ALL
};

RONIN_INLINE uint8 SchoolMask(uint8 school)
{
    if(school >= SCHOOL_ALL)
        return spellMaskArray[SCHOOL_ALL];
    return spellMaskArray[school];
}

enum ReplenishType
{
    REPLENISH_UNDEFINED = 0,
    REPLENISH_HEALTH    = 20,
    REPLENISH_MANA      = 21,
    REPLENISH_RAGE      = 22    //don't know if rage is 22 or what, but will do for now
};

enum SpellTargetType
{
    TARGET_TYPE_NULL        = 0x0,
    TARGET_TYPE_BEAST       = 0x1,
    TARGET_TYPE_DRAGONKIN   = 0x2,
    TARGET_TYPE_DEMON       = 0x4,
    TARGET_TYPE_ELEMENTAL   = 0x8,
    TARGET_TYPE_GIANT       = 0x10,
    TARGET_TYPE_UNDEAD      = 0x20,
    TARGET_TYPE_HUMANOID    = 0x40,
    TARGET_TYPE_CRITTER     = 0x80,
    TARGET_TYPE_MECHANICAL  = 0x100,
};

/****************SpellExtraFlags*****************/
/* SpellExtraFlags defines                      */
/*                                              */
/* Used for infront check and other checks      */
/* when they are not in spell.dbc               */
/*                                              */
/************************************************/
#define SPELL_EXTRA_INFRONT 1
#define SPELL_EXTRA_BEHIND  2
#define SPELL_EXTRA_UNDEF0  4   // not used yet
#define SPELL_EXTRA_UNDEF1  8   // not used yet


/***************Ranged spellid*******************/
/* Note: These spell id's are checked for 3.0.x */
/************************************************/
#define SPELL_ATTACK            6603
#define SPELL_RANGED_GENERAL    3018
#define SPELL_RANGED_THROW      2764
#define SPELL_RANGED_WAND       5019

#pragma pack(PRAGMA_PACK)
struct TeleportCoords
{
    uint32 id;
    uint32 mapId;
    float x;
    float y;
    float z;
    float o;
};
#pragma pack(PRAGMA_POP)

struct TotemSpells
{
    uint32 spellId;
    uint32 spellToCast[3];
};

enum LOCKTYPES
{
    LOCKTYPE_PICKLOCK               = 1,
    LOCKTYPE_HERBALISM              = 2,
    LOCKTYPE_MINING                 = 3,
    LOCKTYPE_DISARM_TRAP            = 4,
    LOCKTYPE_OPEN                   = 5,
    LOCKTYPE_TREASURE               = 6,
    LOCKTYPE_CALCIFIED_ELVEN_GEMS   = 7,
    LOCKTYPE_CLOSE                  = 8,
    LOCKTYPE_ARM_TRAP               = 9,
    LOCKTYPE_QUICK_OPEN             = 10,
    LOCKTYPE_QUICK_CLOSE            = 11,
    LOCKTYPE_OPEN_TINKERING         = 12,
    LOCKTYPE_OPEN_KNEELING          = 13,
    LOCKTYPE_OPEN_ATTACKING         = 14,
    LOCKTYPE_GAHZRIDIAN             = 15,
    LOCKTYPE_BLASTING               = 16,
    LOCKTYPE_SLOW_OPEN              = 17,
    LOCKTYPE_SLOW_CLOSE             = 18,
    LOCKTYPE_FISHING                = 19,
    LOCKTYPE_INSCRIPTION            = 20,
    LOCKTYPE_OPEN_FROM_VEHICLE      = 21
};

enum SpellEffects
{
    SPELL_EFFECT_NULL                       = 0,
    SPELL_EFFECT_INSTANT_KILL               = 1,
    SPELL_EFFECT_SCHOOL_DAMAGE              = 2,
    SPELL_EFFECT_DUMMY                      = 3,
    SPELL_EFFECT_PORTAL_TELEPORT            = 4,
    SPELL_EFFECT_TELEPORT_UNITS             = 5,
    SPELL_EFFECT_APPLY_AURA                 = 6,
    SPELL_EFFECT_ENVIRONMENTAL_DAMAGE       = 7,
    SPELL_EFFECT_POWER_DRAIN                = 8,
    SPELL_EFFECT_HEALTH_LEECH               = 9,
    SPELL_EFFECT_HEAL                       = 10,
    SPELL_EFFECT_BIND                       = 11,
    SPELL_EFFECT_PORTAL                     = 12,
    SPELL_EFFECT_RITUAL_BASE                = 13,
    SPELL_EFFECT_RITUAL_SPECIALIZE          = 14,
    SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL     = 15,
    SPELL_EFFECT_QUEST_COMPLETE             = 16,
    SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL     = 17,
    SPELL_EFFECT_RESURRECT                  = 18,
    SPELL_EFFECT_ADD_EXTRA_ATTACKS          = 19,
    SPELL_EFFECT_DODGE                      = 20,
    SPELL_EFFECT_EVADE                      = 21,
    SPELL_EFFECT_PARRY                      = 22,
    SPELL_EFFECT_BLOCK                      = 23,
    SPELL_EFFECT_CREATE_ITEM                = 24,
    SPELL_EFFECT_WEAPON                     = 25,
    SPELL_EFFECT_DEFENSE                    = 26,
    SPELL_EFFECT_PERSISTENT_AREA_AURA       = 27,
    SPELL_EFFECT_SUMMON                     = 28,
    SPELL_EFFECT_LEAP                       = 29,
    SPELL_EFFECT_ENERGIZE                   = 30,
    SPELL_EFFECT_WEAPON_PERCENT_DAMAGE      = 31,
    SPELL_EFFECT_TRIGGER_MISSILE            = 32,
    SPELL_EFFECT_OPEN_LOCK                  = 33,
    SPELL_EFFECT_TRANSFORM_ITEM             = 34,
    SPELL_EFFECT_APPLY_AREA_AURA            = 35,
    SPELL_EFFECT_LEARN_SPELL                = 36,
    SPELL_EFFECT_SPELL_DEFENSE              = 37,
    SPELL_EFFECT_DISPEL                     = 38,
    SPELL_EFFECT_LANGUAGE                   = 39,
    SPELL_EFFECT_DUAL_WIELD                 = 40,
    SPELL_EFFECT_JUMP_TO_TARGET             = 41,
    SPELL_EFFECT_JUMP_TO_DESTIONATION       = 42,
    SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER = 43,
    SPELL_EFFECT_SKILL_STEP                 = 44,
    SPELL_EFFECT_ADD_HONOR                  = 45,
    SPELL_EFFECT_SPAWN                      = 46,
    SPELL_EFFECT_TRADE_SKILL                = 47,
    SPELL_EFFECT_STEALTH                    = 48,
    SPELL_EFFECT_DETECT                     = 49,
    SPELL_EFFECT_SUMMON_OBJECT              = 50,
    SPELL_EFFECT_FORCE_CRITICAL_HIT         = 51,
    SPELL_EFFECT_GUARANTEE_HIT              = 52,
    SPELL_EFFECT_ENCHANT_ITEM               = 53,
    SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY     = 54,
    SPELL_EFFECT_TAMECREATURE               = 55,
    SPELL_EFFECT_SUMMON_PET                 = 56,
    SPELL_EFFECT_LEARN_PET_SPELL            = 57,
    SPELL_EFFECT_WEAPON_DAMAGE              = 58,
    SPELL_EFFECT_OPEN_LOCK_ITEM             = 59,
    SPELL_EFFECT_PROFICIENCY                = 60,
    SPELL_EFFECT_SEND_EVENT                 = 61,
    SPELL_EFFECT_POWER_BURN                 = 62,
    SPELL_EFFECT_THREAT                     = 63,
    SPELL_EFFECT_TRIGGER_SPELL              = 64,
    SPELL_EFFECT_APPLY_RAID_AURA            = 65,
    SPELL_EFFECT_POWER_FUNNEL               = 66,
    SPELL_EFFECT_HEAL_MAX_HEALTH            = 67,
    SPELL_EFFECT_INTERRUPT_CAST             = 68,
    SPELL_EFFECT_DISTRACT                   = 69,
    SPELL_EFFECT_PULL                       = 70,
    SPELL_EFFECT_PICKPOCKET                 = 71,
    SPELL_EFFECT_ADD_FARSIGHT               = 72,
    SPELL_EFFECT_UNTRAIN_TALENTS            = 73,
    SPELL_EFFECT_USE_GLYPH                  = 74,
    SPELL_EFFECT_HEAL_MECHANICAL            = 75,
    SPELL_EFFECT_SUMMON_OBJECT_WILD         = 76,
    SPELL_EFFECT_SCRIPT_EFFECT              = 77,
    SPELL_EFFECT_ATTACK                     = 78,
    SPELL_EFFECT_SANCTUARY                  = 79,
    SPELL_EFFECT_ADD_COMBO_POINTS           = 80,
    SPELL_EFFECT_CREATE_HOUSE               = 81,
    SPELL_EFFECT_BIND_SIGHT                 = 82,
    SPELL_EFFECT_DUEL                       = 83,
    SPELL_EFFECT_STUCK                      = 84,
    SPELL_EFFECT_SUMMON_PLAYER              = 85,
    SPELL_EFFECT_ACTIVATE_OBJECT            = 86,
    SPELL_EFFECT_WMO_DAMAGE                 = 87,
    SPELL_EFFECT_WMO_REPAIR                 = 88,
    SPELL_EFFECT_WMO_CHANGE                 = 89,
    SPELL_EFFECT_KILL_CREDIT                = 90,
    SPELL_EFFECT_THREAT_ALL                 = 91,
    SPELL_EFFECT_ENCHANT_HELD_ITEM          = 92,
    SPELL_EFFECT_SUMMON_PHANTASM            = 93,
    SPELL_EFFECT_SELF_RESURRECT             = 94,
    SPELL_EFFECT_SKINNING                   = 95,
    SPELL_EFFECT_CHARGE                     = 96,
    SPELL_EFFECT_SUMMON_ALL_TOTEMS          = 97,
    SPELL_EFFECT_KNOCK_BACK                 = 98,
    SPELL_EFFECT_DISENCHANT                 = 99,
    SPELL_EFFECT_INEBRIATE                  = 100,
    SPELL_EFFECT_FEED_PET                   = 101,
    SPELL_EFFECT_DISMISS_PET                = 102,
    SPELL_EFFECT_REPUTATION                 = 103,
    SPELL_EFFECT_SUMMON_OBJECT_SLOT1        = 104,
    SPELL_EFFECT_SUMMON_OBJECT_SLOT2        = 105,
    SPELL_EFFECT_SUMMON_OBJECT_SLOT3        = 106,
    SPELL_EFFECT_SUMMON_OBJECT_SLOT4        = 107,
    SPELL_EFFECT_DISPEL_MECHANIC            = 108,
    SPELL_EFFECT_SUMMON_DEAD_PET            = 109,
    SPELL_EFFECT_DESTROY_ALL_TOTEMS         = 110,
    SPELL_EFFECT_DURABILITY_DAMAGE          = 111,
    SPELL_EFFECT_SUMMON_DEMON               = 112,
    SPELL_EFFECT_RESURRECT_FLAT             = 113,
    SPELL_EFFECT_ATTACK_ME                  = 114,
    SPELL_EFFECT_DURABILITY_DAMAGE_PCT      = 115,
    SPELL_EFFECT_SKIN_PLAYER_CORPSE         = 116,
    SPELL_EFFECT_SPIRIT_HEAL                = 117,
    SPELL_EFFECT_SKILL                      = 118,
    SPELL_EFFECT_APPLY_PET_AURA             = 119,
    SPELL_EFFECT_TELEPORT_GRAVEYARD         = 120,
    SPELL_EFFECT_DUMMYMELEE                 = 121,
    SPELL_EFFECT_SEND_TAXI                  = 123,
    SPELL_EFFECT_PLAYER_PULL                = 124,
    SPELL_EFFECT_SPELL_STEAL                = 126,
    SPELL_EFFECT_PROSPECTING                = 127,
    SPELL_EFFECT_APPLY_AREA_AURA_FRIEND     = 128,
    SPELL_EFFECT_APPLY_AREA_AURA_ENEMY      = 129,
    SPELL_EFFECT_REDIRECT_THREAT            = 130,
    SPELL_EFFECT_PLAY_SOUND                 = 131,
    SPELL_EFFECT_PLAY_MUSIC                 = 132,
    SPELL_EFFECT_FORGET_SPECIALIZATION      = 133,
    SPELL_EFFECT_KILL_CREDIT_2              = 134,
    SPELL_EFFECT_CALL_PET                   = 135,
    SPELL_EFFECT_HEAL_PCT                   = 136,
    SPELL_EFFECT_ENERGIZE_PCT               = 137,
    SPELL_EFFECT_LEAP_BACK                  = 138,
    SPELL_EFFECT_CLEAR_QUEST                = 139,
    SPELL_EFFECT_FORCE_CAST                 = 140,
    SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE   = 142,
    SPELL_EFFECT_APPLY_DEMON_AURA           = 143,
    SPELL_EFFECT_KNOCKBACK_2                = 144,
    SPELL_EFFECT_TRACTOR_BEAM_FROM_DEST     = 145,
    SPELL_EFFECT_ACTIVATE_RUNE              = 146,
    SPELL_EFFECT_QUEST_FAIL                 = 147,
    SPELL_EFFECT_CHARGE2                    = 149,
    SPELL_EFFECT_QUEST_OFFER                = 150,
    SPELL_EFFECT_TRIGGER_SPELL_2            = 151,
    SPELL_EFFECT_CREATE_PET                 = 153,
    SPELL_EFFECT_TEACH_TAXI_NODE            = 154,
    SPELL_EFFECT_TITAN_GRIP                 = 155,
    SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC     = 156,
    SPELL_EFFECT_CREATE_ITEM_2              = 157,
    SPELL_EFFECT_MILLING                    = 158,
    SPELL_EFFECT_ALLOW_PET_RENAME           = 159,
    SPELL_EFFECT_SET_TALENT_SPECS_COUNT     = 161,
    SPELL_EFFECT_ACTIVATE_TALENT_SPEC       = 162,
    SPELL_EFFECT_REMOVE_TARGET_AURA         = 164,
    TOTAL_SPELL_EFFECTS                     = 165
};

enum AreaAuraTargets
{
    AA_TARGET_PARTY             = 0x01,
    AA_TARGET_RAID              = 0x02,
    AA_TARGET_ALLFRIENDLY       = 0x04,
    AA_TARGET_ALLENEMIES        = 0x08,
    AA_TARGET_PET               = 0x10,
    AA_TARGET_NOTSELF           = 0x20,
};

enum SpellFamilyNames
{
    SPELLFAMILY_GENERIC     = 0,
    SPELLFAMILY_UNK1,
    SPELLFAMILY_MAGE        = 3,
    SPELLFAMILY_WARRIOR,
    SPELLFAMILY_WARLOCK,
    SPELLFAMILY_PRIEST,
    SPELLFAMILY_DRUID,
    SPELLFAMILY_ROGUE,
    SPELLFAMILY_HUNTER,
    SPELLFAMILY_PALADIN,
    SPELLFAMILY_SHAMAN,
    SPELLFAMILY_UNK2,
    SPELLFAMILY_POTION,
    SPELLFAMILY_DEATHKNIGHT = 15,
    SPELLFAMILY_PET         = 17
};

enum SpellState
{
    SPELL_STATE_NULL        = 0,
    SPELL_STATE_PREPARING   = 1,
    SPELL_STATE_CASTING     = 2,
    SPELL_STATE_FINISHED    = 3,
    SPELL_STATE_IDLE        = 4
};

enum ShapeshiftForm
{
    FORM_NORMAL             = 0,    //1
    FORM_CAT                = 1,    //2
    FORM_TREE               = 2,    //4
    FORM_TRAVEL             = 3,    //8
    FORM_AQUA               = 4,    //16
    FORM_BEAR               = 5,    //32
    FORM_AMBIENT            = 6,    //64
    FORM_GHOUL              = 7,    //128
    FORM_DIREBEAR           = 8,    //256
    FORM_9                  = 9,    //512
    FORM_10                 = 10,   //1024
    FORM_11                 = 11,   //2048
    FORM_12                 = 12,   //4096
    FORM_13                 = 13,   //8192
    FORM_CREATUREBEAR       = 14,   //16384
    FORM_GHOSTWOLF          = 16,   //65536
    FORM_BATTLESTANCE       = 17,   //131072
    FORM_DEFENSIVESTANCE    = 18,   //262144
    FORM_BERSERKERSTANCE    = 19,   //524288
    FORM_20                 = 20,   //1048576
    FORM_ZOMBIE             = 21,   //2097152
    FORM_DEMON              = 22,   //4194304
    FORM_23                 = 23,   //8388608
    FORM_24                 = 24,   //16777216
    FORM_UNDEAD             = 25,   //33554432
    FORM_MASTER_ANGLER      = 26,   //67108864
    FORM_SWIFT              = 27,   //134217728
    FORM_SHADOW             = 28,   //268435456
    FORM_FLIGHT             = 29,   //536870912
    FORM_STEALTH            = 30,   //1073741824
    FORM_MOONKIN            = 31,   //2147483648
    FORM_SPIRITOFREDEMPTION = 32,   //4294967296
};

enum DISPEL_TYPE
{
    DISPEL_ZGTRINKETS   = -1,
    DISPEL_NULL,
    DISPEL_MAGIC,
    DISPEL_CURSE,
    DISPEL_DISEASE,
    DISPEL_POISON,
    DISPEL_STEALTH,
    DISPEL_INVISIBILTY,
    DISPEL_ALL,
    DISPEL_SPECIAL_NPCONLY,
    DISPEL_FRENZY,
    NUM_DISPELS,
};

enum DISPEL_MECHANIC_TYPE
{
    DISPEL_MECHANIC_CHARM       = 1,
    DISPEL_MECHANIC_FEAR        = 5,
    DISPEL_MECHANIC_ROOT        = 7,
    DISPEL_MECHANIC_SLEEP       = 10,
    DISPEL_MECHANIC_SNARE       = 11,
    DISPEL_MECHANIC_STUN        = 12,
    DISPEL_MECHANIC_KNOCKOUT    = 14,
    DISPEL_MECHANIC_BLEED       = 15,
    DISPEL_MECHANIC_POLYMORPH   = 17,
    DISPEL_MECHANIC_BANISH      = 18,
    DISPEL_MECHANIC_MOUNTED     = 21,
};
enum MECHANICS
{
    MECHANIC_CHARMED        = 1,
    MECHANIC_DISORIENTED,   // 2
    MECHANIC_DISARMED,      // 3
    MECHANIC_DISTRACED,     // 4
    MECHANIC_FLEEING,       // 5
    MECHANIC_CLUMSY,        // 6
    MECHANIC_ROOTED,        // 7
    MECHANIC_PACIFIED,      // 8
    MECHANIC_SILENCED,      // 9
    MECHANIC_ASLEEP,        // 10
    MECHANIC_ENSNARED,      // 11
    MECHANIC_STUNNED,
    MECHANIC_FROZEN,
    MECHANIC_INCAPACIPATED,
    MECHANIC_BLEEDING,
    MECHANIC_HEALING,
    MECHANIC_POLYMORPHED,
    MECHANIC_BANISHED,
    MECHANIC_SHIELDED,
    MECHANIC_SHACKLED,
    MECHANIC_MOUNTED,
    MECHANIC_SEDUCED,
    MECHANIC_TURNED,
    MECHANIC_HORRIFIED,
    MECHANIC_INVULNARABLE,
    MECHANIC_INTERRUPTED,
    MECHANIC_DAZED,
    MECHANIC_DISCOVERY,
    MECHANIC_PROTECTED,
    MECHANIC_SAPPED,
    MECHANIC_ENRAGED,
    MECHANIC_COUNT
};

enum BUFFS
{
    BUFF_NONE = 0,
    // Warrior buffs
    BUFF_WARRIOR_START,
    BUFF_WARRIOR_BATTLE_SHOUT,
    BUFF_WARRIOR_COMMANDING_SHOUT,
    // Paladin buffs
    BUFF_PALADIN_BLESSING_START,
    BUFF_PALADIN_MIGHT,
    BUFF_PALADIN_KINGS,
    BUFF_PALADIN_AURA_START,
    BUFF_PALADIN_DEVOTION,
    BUFF_PALADIN_RETRIBUTION,
    BUFF_PALADIN_CONCENTRATION,
    BUFF_PALADIN_RESISTANCE,
    BUFF_PALADIN_CRUSADER,
    BUFF_PALADIN_SEAL_START,
    BUFF_PALADIN_INSIGHT,
    BUFF_PALADIN_JUSTICE,
    BUFF_PALADIN_RIGHTEOUSNESS,
    BUFF_PALADIN_TRUTH,
    BUFF_PALADIN_HAND_START,
    BUFF_PALADIN_FREEDOM,
    BUFF_PALADIN_PROTECTION,
    BUFF_PALADIN_SACRIFICE,
    BUFF_PALADIN_SALVATION,
    // Hunter buffs
    BUFF_HUNTER_START,
    // Rogue buffs
    BUFF_ROGUE_POISON_START,
    // Priest buffs
    BUFF_PRIEST_START,
    // Death Knight buffs
    BUFF_DEATHKNIGHT_START,
    BUFF_DKPRESENCE,
    // Shaman buffs
    BUFF_SHAMAN_START,
    // Mage buffs
    BUFF_MAGE_START,
    // Warlock buffs
    BUFF_WARLOCK_START,
    // Druid buffs
    BUFF_DRUID_START,

    MAX_BUFFS
};

static BUFFS BuffSets[MAX_BUFFS] = 
{
    BUFF_NONE,
    BUFF_NONE,
    BUFF_WARRIOR_BATTLE_SHOUT,
    BUFF_WARRIOR_COMMANDING_SHOUT,
    BUFF_NONE,
    BUFF_PALADIN_MIGHT,
    BUFF_PALADIN_KINGS,
    BUFF_NONE,
    BUFF_PALADIN_DEVOTION,
    BUFF_PALADIN_RETRIBUTION,
    BUFF_PALADIN_CONCENTRATION,
    BUFF_PALADIN_RESISTANCE,
    BUFF_PALADIN_CRUSADER,
    BUFF_NONE,
    BUFF_PALADIN_INSIGHT,
    BUFF_PALADIN_JUSTICE,
    BUFF_PALADIN_RIGHTEOUSNESS,
    BUFF_PALADIN_TRUTH,
    BUFF_NONE,
    BUFF_PALADIN_FREEDOM,
    BUFF_PALADIN_PROTECTION,
    BUFF_PALADIN_SACRIFICE,
    BUFF_PALADIN_SALVATION,
    BUFF_NONE,
    BUFF_NONE,
    BUFF_NONE,
    BUFF_NONE,
    BUFF_DKPRESENCE,
    BUFF_NONE,
    BUFF_NONE,
    BUFF_NONE,
    BUFF_NONE
};

RONIN_INLINE uint8 GetBuffSetStartForIndex(uint8 buffIndex)
{
    if(buffIndex == BUFF_NONE || buffIndex >= MAX_BUFFS)
        return MAX_BUFFS;
    uint8 ret = buffIndex;
    while(BuffSets[ret] != BUFF_NONE)
        ret--;
    return ret;
}

typedef enum SpellEffectTarget
{
    EFF_TARGET_NONE                                     = 0,
    EFF_TARGET_SELF                                     = 1,
    EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS       = 3,
    EFF_TARGET_PET                                      = 5,
    EFF_TARGET_SINGLE_ENEMY                             = 6,
    EFF_TARGET_SCRIPTED_TARGET                          = 7,
    EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS = 8,
    EFF_TARGET_HEARTSTONE_LOCATION                      = 9,
    EFF_TARGET_ALL_ENEMY_IN_AREA                        = 15,
    EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT                = 16,
    EFF_TARGET_TELEPORT_LOCATION                        = 17,
    EFF_TARGET_LOCATION_TO_SUMMON                       = 18,
    EFF_TARGET_ALL_PARTY_AROUND_CASTER                  = 20,
    EFF_TARGET_SINGLE_FRIEND                            = 21,
    EFF_TARGET_ALL_ENEMIES_AROUND_CASTER                = 22,
    EFF_TARGET_GAMEOBJECT                               = 23,
    EFF_TARGET_IN_FRONT_OF_CASTER                       = 24,
    EFF_TARGET_DUEL                                     = 25,//Dont know the real name!!!
    EFF_TARGET_GAMEOBJECT_ITEM                          = 26,
    EFF_TARGET_PET_MASTER                               = 27,
    EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED              = 28,
    EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED              = 29,
    EFF_TARGET_ALL_FRIENDLY_IN_AREA                     = 30,
    EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME   = 31,
    EFF_TARGET_MINION                                   = 32,
    EFF_TARGET_ALL_PARTY_IN_AREA                        = 33,
    EFF_TARGET_SINGLE_PARTY                             = 35,
    EFF_TARGET_PET_SUMMON_LOCATION                      = 36,
    EFF_TARGET_ALL_PARTY                                = 37,
    EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET                = 38,
    EFF_TARGET_SELF_FISHING                             = 39,
    EFF_TARGET_SCRIPTED_GAMEOBJECT                      = 40,
    EFF_TARGET_TOTEM_EARTH                              = 41,
    EFF_TARGET_TOTEM_WATER                              = 42,
    EFF_TARGET_TOTEM_AIR                                = 43,
    EFF_TARGET_TOTEM_FIRE                               = 44,
    EFF_TARGET_CHAIN                                    = 45,
    EFF_TARGET_SCIPTED_OBJECT_LOCATION                  = 46,
    EFF_TARGET_DYNAMIC_OBJECT                           = 47,//not sure exactly where is used
    EFF_TARGET_MULTIPLE_SUMMON_LOCATION                 = 48,
    EFF_TARGET_MULTIPLE_SUMMON_PET_LOCATION             = 49,
    EFF_TARGET_SUMMON_LOCATION                          = 50,
    EFF_TARGET_CALIRI_EGS                               = 51,
    EFF_TARGET_LOCATION_NEAR_CASTER                     = 52,
    EFF_TARGET_CURRENT_SELECTION                        = 53,
    EFF_TARGET_TARGET_AT_ORIENTATION_TO_CASTER          = 54,
    EFF_TARGET_LOCATION_INFRONT_CASTER                  = 55,
    EFF_TARGET_PARTY_MEMBER                             = 57,
    EFF_TARGET_TARGET_FOR_VISUAL_EFFECT                 = 59,
    EFF_TARGET_SCRIPTED_TARGET2                         = 60,
    EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS               = 61,
    EFF_TARGET_PRIEST_CHAMPION                          = 62,   //wtf ?
    EFF_TARGET_NATURE_SUMMON_LOCATION                   = 63,
    EFF_TARGET_BEHIND_TARGET_LOCATION                   = 65,
    EFF_TARGET_MULTIPLE_GUARDIAN_SUMMON_LOCATION        = 72,
    EFF_TARGET_NETHETDRAKE_SUMMON_LOCATION              = 73,
    EFF_TARGET_SCRIPTED_LOCATION                        = 74,
    EFF_TARGET_LOCATION_INFRONT_CASTER_AT_RANGE         = 75,
    EFF_TARGET_ENEMYS_IN_ARE_CHANNELED_WITH_EXCEPTIONS  = 76,
    EFF_TARGET_SELECTED_ENEMY_CHANNELED                 = 77,
    EFF_TARGET_SELECTED_ENEMY_DEADLY_POISON             = 86,
    EFF_TARGET_SIMPLE_AOE                               = 87,   //just seems to be a simple aoe target, with a little circle
    EFF_TARGET_SMALL_AOE                                = 88,   //even smaller aoe circle
    EFF_TARGET_NONCOMBAT_PET                            = 90,   //target non-combat pet :P
    EFF_TARGET_VEHICLE                                  = 94,
    EFF_TARGET_VEHICLEDRIVER                            = 95,
    EFF_TARGET_VEHICLE_PASSENGER_0                      = 96,
    EFF_TARGET_VEHICLE_PASSENGER_1                      = 97,
    EFF_TARGET_VEHICLE_PASSENGER_2                      = 98,
    EFF_TARGET_VEHICLE_PASSENGER_3                      = 99,
    EFF_TARGET_VEHICLE_PASSENGER_4                      = 100,
    EFF_TARGET_VEHICLE_PASSENGER_5                      = 101,
    EFF_TARGET_VEHICLE_PASSENGER_6                      = 102,
    EFF_TARGET_VEHICLE_PASSENGER_7                      = 103,
    EFF_TARGET_IN_FRONT_OF_CASTER_30                    = 104,
    TOTAL_SPELL_TARGET                                  = 111   // note: This is the highest known as of 3.3.3
} SpellEffectTarget;

enum SpellImplicitTargetType
{
    SPELL_TARGET_NONE                   = 0x00000000,
    SPELL_TARGET_REQUIRE_GAMEOBJECT     = 0x00000001,
    SPELL_TARGET_REQUIRE_ITEM           = 0x00000002,
    SPELL_TARGET_REQUIRE_ATTACKABLE     = 0x00000004,
    SPELL_TARGET_REQUIRE_FRIENDLY       = 0x00000008,
    SPELL_TARGET_OBJECT_SCRIPTED        = 0x00000010, //scripted units
    SPELL_TARGET_OBJECT_SELF            = 0x00000020,
    SPELL_TARGET_OBJECT_CURPET          = 0x00000040,
    SPELL_TARGET_OBJECT_CURCRITTER      = 0x00000080,
    SPELL_TARGET_OBJECT_PETOWNER        = 0x00000100,
    SPELL_TARGET_OBJECT_CURTOTEMS       = 0x00000200,
    SPELL_TARGET_OBJECT_TARCLASS        = 0x00000400,
    SPELL_TARGET_AREA                   = 0x00000800,
    SPELL_TARGET_AREA_SELF              = 0x00001000,
    SPELL_TARGET_AREA_CONE              = 0x00002000,
    SPELL_TARGET_AREA_CHAIN             = 0x00004000,
    SPELL_TARGET_AREA_CURTARGET         = 0x00008000,
    SPELL_TARGET_AREA_RANDOM            = 0x00010000,
    SPELL_TARGET_AREA_PARTY             = 0x00020000,
    SPELL_TARGET_AREA_RAID              = 0x00040000,
    SPELL_TARGET_NOT_IMPLEMENTED        = 0x00080000,
    SPELL_TARGET_NO_OBJECT              = 0x00100000,
    SPELL_TARGET_ANY_OBJECT             = 0x00200000,
};

enum SpellDidHitResult
{
    SPELL_DID_HIT_SUCCESS                   = 0,
    SPELL_DID_HIT_MISS                      = 1,
    SPELL_DID_HIT_RESIST                    = 2,
    SPELL_DID_HIT_DODGE                     = 3,
    SPELL_DID_HIT_PARRY                     = 4,
    SPELL_DID_HIT_BLOCK                     = 5,
    SPELL_DID_HIT_EVADE                     = 6,
    SPELL_DID_HIT_IMMUNE                    = 7,
    SPELL_DID_HIT_IMMUNE_2                  = 8,
    SPELL_DID_HIT_DEFLECT                   = 9,
    SPELL_DID_HIT_ABSORB                    = 10,
    SPELL_DID_HIT_REFLECT                   = 11,
};

bool CanAgroHash(uint32 spellhashname);
bool IsCastedOnFriends(SpellEntry *sp);
bool IsCastedOnEnemies(SpellEntry *sp);
bool IsDamagingSpell(SpellEntry *sp);
bool IsHealingSpell(SpellEntry *sp);
bool IsInrange(LocationVector & location, WorldObject* o, float square_r, float square_min = 0.f);
bool IsInrange(float x1,float y1, float z1, WorldObject* o,float square_r, float square_min = 0.f);
bool IsInrange(float x1,float y1, float z1,float x2,float y2, float z2,float square_r, float square_min = 0.f);
bool IsInrange(WorldObject* o1,WorldObject* o2,float square_r, float square_min = 0.f);
bool TargetTypeCheck(WorldObject* obj,uint32 ReqCreatureTypeMask);
bool IsFlyingSpell(SpellEntry *sp);
bool IsTargetingStealthed(SpellEntry *sp);
