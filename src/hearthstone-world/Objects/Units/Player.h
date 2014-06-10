/***
 * Demonstrike Core
 */

#pragma once

struct BGScore;
class Channel;
class Creature;
class Battleground;
class TaxiPath;
class GameObject;
class Transporter;
class Corpse;
struct GuildRank;
class Pet;
class Charter;
class LFGMatch;
struct LevelInfo;
#define MAX_PET_NO 3
#define PLAYER_NORMAL_RUN_SPEED 7.0f
#define PLAYER_NORMAL_SWIM_SPEED 4.722222f
#define PLAYER_NORMAL_FLIGHT_SPEED 7.0f
#define PLAYER_HONORLESS_TARGET_SPELL 2479
#define MONSTER_NORMAL_RUN_SPEED 8.0f
/* action button defines */
#define PLAYER_ACTION_BUTTON_COUNT 144
#define MAX_SPEC_COUNT 2
#define GLYPHS_COUNT 9

#define ALLIANCE 0
#define HORDE 1

// gold cap
#define PLAYER_MAX_GOLD 0x7FFFFFFF

#define MAX_TAXI 12

//====================================================================
//  Inventory
//  Holds the display id and item type id for objects in
//  a character's inventory
//====================================================================

enum Classes
{
    WARRIOR = 1,
    PALADIN = 2,
    HUNTER = 3,
    ROGUE = 4,
    PRIEST = 5,
    DEATHKNIGHT = 6,
    SHAMAN = 7,
    MAGE = 8,
    WARLOCK = 9,
    DRUID = 11,
};

enum ClassMasks
{
    CLASSMASK_WARRIOR       = 0x0001, // 1
    CLASSMASK_PALADIN       = 0x0002, // 2
    CLASSMASK_HUNTER        = 0x0004, // 4
    CLASSMASK_ROGUE         = 0x0008, // 8
    CLASSMASK_PRIEST        = 0x0010, // 16
    CLASSMASK_DEATHKNIGHT   = 0x0020, // 32
    CLASSMASK_SHAMAN        = 0x0040, // 64
    CLASSMASK_MAGE          = 0x0080, // 128
    CLASSMASK_WARLOCK       = 0x0100, // 256
    CLASSMASK_NONE          = 0x0200, // 512
    CLASSMASK_DRUID         = 0x0400  // 1024
};

enum Races
{
    RACE_HUMAN = 1,
    RACE_ORC = 2,
    RACE_DWARF = 3,
    RACE_NIGHTELF = 4,
    RACE_UNDEAD = 5,
    RACE_TAUREN = 6,
    RACE_GNOME = 7,
    RACE_TROLL = 8,
    RACE_BLOODELF = 10,
    RACE_DRAENEI = 11
};

enum PlayerStatus
{
    NONE             = 0,
    TRANSFER_PENDING = 1,
};

enum MirrorTimerType
{
    FATIGUE_TIMER   = 0,
    BREATH_TIMER    = 1,
    FIRE_TIMER      = 2
};

enum PvPAreaStatus
{
    AREA_ALLIANCE   = 1,
    AREA_HORDE      = 2,
    AREA_CONTESTED  = 3,
    AREA_PVPARENA   = 4,
};

enum PlayerMovementType
{
    MOVE_ROOT       = 1,
    MOVE_UNROOT     = 2,
    MOVE_WATER_WALK = 3,
    MOVE_LAND_WALK  = 4,
};

enum LFGroleflags
{
    LEADER      = 1,
    TANK        = 2,
    HEALER      = 4,
    DAMAGE      = 8
};

/*
Exalted         1,000    Access to racial mounts. Capped at 999.7
Revered         21,000   Heroic mode keys for Outland dungeons
Honored         12,000   10% discount from faction vendors
Friendly        6,000
Neutral         3,000
Unfriendly      3,000    Cannot buy, sell or interact.
Hostile         3,000    You will always be attacked on sight
Hated           36,000
*/
enum Standing
{
    STANDING_HATED = 0,
    STANDING_HOSTILE,
    STANDING_UNFRIENDLY,
    STANDING_NEUTRAL,
    STANDING_FRIENDLY,
    STANDING_HONORED,
    STANDING_REVERED,
    STANDING_EXALTED
};

enum PlayerFlags
{
    PLAYER_FLAG_PARTY_LEADER        = 0x01,
    PLAYER_FLAG_AFK                 = 0x02,
    PLAYER_FLAG_DND                 = 0x04,
    PLAYER_FLAG_GM                  = 0x08,
    PLAYER_FLAG_DEATH_WORLD_ENABLE  = 0x10,
    PLAYER_FLAG_RESTING             = 0x20,
    PLAYER_FLAG_UNKNOWN1            = 0x40,
    PLAYER_FLAG_FREE_FOR_ALL_PVP    = 0x80,
    PLAYER_FLAG_UNKNOWN2            = 0x100,
    PLAYER_FLAG_PVP_TOGGLE          = 0x200,
    PLAYER_FLAG_NOHELM              = 0x400,
    PLAYER_FLAG_NOCLOAK             = 0x800,
    PLAYER_FLAG_NEED_REST_3_HOURS   = 0x1000,
    PLAYER_FLAG_NEED_REST_5_HOURS   = 0x2000,
    PLAYER_FLAG_UNK15               = 0x4000,
    PLAYER_FLAG_DEVELOPER           = 0x8000,
    PLAYER_FLAG_UNK17               = 0x10000,
    PLAYER_FLAG_UNK18               = 0x20000,
    PLAYER_FLAG_PVP_TIMER           = 0x40000,
    PLAYER_FLAG_UNK20               = 0x80000,
    PLAYER_FLAG_UNK21               = 0x100000,
    PLAYER_FLAG_UNK22               = 0x200000,
    PLAYER_FLAG_UNK23               = 0x400000,
    PLAYER_FLAG_ALLOW_ONLY_ABILITY  = 0x800000,
    PLAYER_FLAG_UNK25               = 0x1000000,
};

enum CharterTypes
{
    CHARTER_TYPE_GUILD          = 0,
    CHARTER_TYPE_ARENA_2V2      = 1,
    CHARTER_TYPE_ARENA_3V3      = 2,
    CHARTER_TYPE_ARENA_5V5      = 3,
    NUM_CHARTER_TYPES           = 4,
};

enum ArenaTeamTypes
{
    ARENA_TEAM_TYPE_2V2         = 0,
    ARENA_TEAM_TYPE_3V3         = 1,
    ARENA_TEAM_TYPE_5V5         = 2,
    NUM_ARENA_TEAM_TYPES        = 3,
};

enum CooldownTypes
{
    COOLDOWN_TYPE_SPELL         = 0,
    COOLDOWN_TYPE_CATEGORY      = 1,
    NUM_COOLDOWN_TYPES,
};

enum LootType
{
    LOOT_CORPSE                 = 1,
    LOOT_PICKPOCKETING          = 2,
    LOOT_FISHING                = 3,
    LOOT_DISENCHANTING          = 4,

    LOOT_SKINNING               = 6,
    LOOT_PROSPECTING            = 7,
    LOOT_MILLING                = 8,
};

enum DrunkenState
{
    DRUNKEN_SOBER   = 0,
    DRUNKEN_TIPSY   = 1,
    DRUNKEN_DRUNK   = 2,
    DRUNKEN_SMASHED = 3,
    DRUNKEN_VOMIT   = 4 //Custom to mark when to barf.
};

struct spells
{
    uint16  spellId;
    uint16  slotId;
};

#pragma pack(PRAGMA_PACK)

struct ActionButton
{
    uint32 PackedData;
    uint8 GetType() { return ACTION_BUTTON_TYPE(PackedData); };
    uint32 GetAction() { return ACTION_BUTTON_ACTION(PackedData); };
};

#pragma pack(PRAGMA_POP)

struct CreateInfo_ItemStruct
{
    uint32  protoid;
    uint8   slot;
    uint32  amount;
};

struct CreateInfo_SkillStruct
{
    uint32  skillid;
    uint32  currentval;
    uint32  maxval;
};

struct CreateInfo_ActionBarStruct
{
    uint8  button;
    uint32  action;
    uint8  type;
};

struct PlayerCreateInfo
{
    uint8   index;
    uint8   race;
    uint32  factiontemplate;
    uint8   class_;
    uint32  mapId;
    uint32  zoneId;
    float   positionX;
    float   positionY;
    float   positionZ;
    float   Orientation;
    uint16  displayId;
    uint8   strength;
    uint8   ability;
    uint8   stamina;
    uint8   intellect;
    uint8   spirit;
    uint32  health;
    uint32  mana;
    uint32  rage;
    uint32  focus;
    uint32  energy;
    uint32  runic;
    uint32  attackpower;
    float   mindmg;
    float   maxdmg;
    std::list<CreateInfo_ItemStruct> items;
    std::list<CreateInfo_SkillStruct> skills;
    std::list<CreateInfo_ActionBarStruct> actionbars;
    std::set<uint32> spell_list;
};

struct DamageSplit
{
    Player* caster;
    Aura*   aura;
    uint32  miscVal;
    union
    {
        uint32 damage;
        float damagePCT;
    };
};

struct LoginAura
{
    uint32 id;
    int32 dur;
};

static const uint32 TalentTreesPerClass[DRUID+1][3] =  {
    { 0, 0, 0 },        // NONE
    { 161, 163, 164 },  // WARRIOR
    { 382, 383, 381 },  // PALADIN
    { 361, 363, 362 },  // HUNTER
    { 182, 181, 183 },  // ROGUE
    { 201, 202, 203 },  // PRIEST
    { 398, 399, 400 },  // DEATH KNIGHT
    { 261, 263, 262 },  // SHAMAN
    { 81, 41, 61 },     // MAGE
    { 302, 303, 301 },  // WARLOCK
    { 0, 0, 0 },        // NONE
    { 283, 281, 282 },  // DRUID
};

struct AreaPhaseData
{   // We might need more later.
    int32 phase;
};

//Manaregen
const float BaseRegen[100] =
{
    0.034965f, 0.034191f, 0.033465f, 0.032526f, 0.031661f, 0.031076f, 0.030523f, 0.029994f, 0.029307f, 0.028661f,
    0.027584f, 0.026215f, 0.025381f, 0.024300f, 0.023345f, 0.022748f, 0.021958f, 0.021386f, 0.020790f, 0.020121f,
    0.019733f, 0.019155f, 0.018819f, 0.018316f, 0.017936f, 0.017576f, 0.017201f, 0.016919f, 0.016581f, 0.016233f,
    0.015994f, 0.015707f, 0.015464f, 0.015204f, 0.014956f, 0.014744f, 0.014495f, 0.014302f, 0.014094f, 0.013895f,
    0.013724f, 0.013522f, 0.013363f, 0.013175f, 0.012996f, 0.012853f, 0.012687f, 0.012539f, 0.012384f, 0.012233f,
    0.012113f, 0.011973f, 0.011859f, 0.011714f, 0.011575f, 0.011473f, 0.011342f, 0.011245f, 0.011110f, 0.010999f,
    0.010700f, 0.010522f, 0.010290f, 0.010119f, 0.009968f, 0.009808f, 0.009651f, 0.009553f, 0.009445f, 0.009327f,
    0.008859f, 0.008415f, 0.007993f, 0.007592f, 0.007211f, 0.006849f, 0.006506f, 0.006179f, 0.005869f, 0.005575f,
    0.005560f, 0.005546f, 0.005533f, 0.005521f, 0.005510f, 0.005490f, 0.005471f, 0.005453f, 0.005436f, 0.005420f,
    0.005405f, 0.005391f, 0.005378f, 0.005366f, 0.005355f, 0.005345f, 0.005325f, 0.005316f, 0.005298f, 0.005281f
};

// Dodge ( class base ) - UNUSED, Warrior, Paladin, Hunter, Rogue,   Priest, Death Knight, Shaman, Mage, Warlock, UNUSED, Druid
const float baseDodge[12] = { 0.0f, 3.664f, 3.4943f, -4.0873f, 2.0957f, 3.4178f, 3.66400f, 2.108f, 3.66f, 2.4211f, 0.0f, 5.6097f };

// Dodge ( class Cap )      UNUSED  War Paladin hunter  rogue   priest  DK  Shaman  Mage    Lock UNUSED Druid
const float DodgeCap[12] = { 0.0f, 84.8f, 59.9f, 86.2f, 47.8f, 59.9f, 84.7f, 59.9f, 58.8f, 59.9f, 0.0f, 47.8f };

enum RuneTypes
{
    RUNE_TYPE_BLOOD         = 0,
    RUNE_TYPE_FROST         = 1,
    RUNE_TYPE_UNHOLY        = 2,
    RUNE_TYPE_DEATH         = 3,
    RUNE_TYPE_RECHARGING    = 4
};
static const uint8 baseRunes[6] = {RUNE_TYPE_BLOOD,RUNE_TYPE_BLOOD,RUNE_TYPE_FROST,RUNE_TYPE_FROST,RUNE_TYPE_UNHOLY,RUNE_TYPE_UNHOLY};

struct FactionReputation
{
    int32 standing;
    uint8 flag;
    int32 baseStanding;
    HEARTHSTONE_INLINE int32 CalcStanding() { return standing - baseStanding; }
    HEARTHSTONE_INLINE bool Positive() { return standing >= 0; }
};

struct PlayerInfo
{
    ~PlayerInfo();
    uint32 guid;
    uint32 acct;
    char * name;
    uint32 race;
    uint32 gender;
    uint32 _class;
    uint32 team;
    uint32 curInstanceID;
    uint32 lastmapid;
    float lastpositionx;
    float lastpositiony;
    float lastpositionz;
    float lastorientation;

    time_t lastOnline;
    uint32 lastZone;
    uint32 lastLevel;

    Group * m_Group;
    int8 subGroup;

    uint32 GuildId;
    uint32 GuildRank;
    Player* m_loggedInPlayer;
    ArenaTeam * arenaTeam[NUM_ARENA_TEAM_TYPES];
    uint32 charterId[NUM_CHARTER_TYPES];
};

struct PlayerPet
{
    string name;
    uint32 entry;
    string fields;
    uint32 xp;
    bool active;
    uint8 stablestate;
    uint32 number;
    uint32 level;
    uint32 happiness;
    uint32 happinessupdate;
    uint32 actionbarspell[4];
    uint32 actionbarspellstate[4];
    bool summon;
};

enum MeetingStoneQueueStatus
{
    MEETINGSTONE_STATUS_NONE                                = 0,
    MEETINGSTONE_STATUS_JOINED_MEETINGSTONE_QUEUE_FOR      = 1,
    MEETINGSTONE_STATUS_PARTY_MEMBER_LEFT_LFG              = 2,
    MEETINGSTONE_STATUS_PARTY_MEMBER_REMOVED_PARTY_REMOVED  = 3,
    MEETINGSTONE_STATUS_LOOKING_FOR_NEW_PARTY_IN_QUEUE    = 4,
    MEETINGSTONE_STATUS_NONE_UNK                            = 5,
};

enum ItemPushResultTypes
{
    ITEM_PUSH_TYPE_LOOT          = 0x00000000,
    ITEM_PUSH_TYPE_RECEIVE        = 0x00000001,
    ITEM_PUSH_TYPE_CREATE          = 0x00000002,
};

struct WeaponModifier
{
    uint32 wclass;
    uint32 subclass;
    float value;
};

struct classScriptOverride
{
    uint32 id;
    uint32 effect;
    uint32 aura;
    uint32 damage;
    bool percent;
};
class Spell;
class Item;
class Container;
class WorldSession;
class ItemInterface;
class GossipMenu;
class AchievementInterface;
struct TaxiPathNode;

#define RESTSTATE_RESTED             1
#define RESTSTATE_NORMAL             2
#define RESTSTATE_TIRED100           3
#define RESTSTATE_TIRED50            4
#define RESTSTATE_EXHAUSTED          5

enum UnderwaterStates
{
    UNDERWATERSTATE_NONE            = 0x00,
    UNDERWATERSTATE_UNDERWATER      = 0x02,
    UNDERWATERSTATE_FATIGUE         = 0x04,
    UNDERWATERSTATE_LAVA            = 0x08,
    UNDERWATERSTATE_SLIME           = 0x10,
    UNDERWATERSTATE_TIMERS_PRESENT  = 0X20
};

enum TRADE_STATUS
{
    TRADE_STATUS_PLAYER_BUSY        = 0x00,
    TRADE_STATUS_PROPOSED           = 0x01,
    TRADE_STATUS_INITIATED          = 0x02,
    TRADE_STATUS_CANCELLED          = 0x03,
    TRADE_STATUS_ACCEPTED           = 0x04,
    TRADE_STATUS_ALREADY_TRADING    = 0x05,
    TRADE_STATUS_PLAYER_NOT_FOUND   = 0x06,
    TRADE_STATUS_STATE_CHANGED      = 0x07,
    TRADE_STATUS_COMPLETE           = 0x08,
    TRADE_STATUS_UNACCEPTED         = 0x09,
    TRADE_STATUS_TOO_FAR_AWAY       = 0x0A,
    TRADE_STATUS_WRONG_FACTION      = 0x0B,
    TRADE_STATUS_FAILED             = 0x0C,
    TRADE_STATUS_UNK2               = 0x0D,
    TRADE_STATUS_PLAYER_IGNORED     = 0x0E,
    TRADE_STATUS_YOU_STUNNED        = 0x0F,
    TRADE_STATUS_TARGET_STUNNED     = 0x10,
    TRADE_STATUS_DEAD               = 0x11,
    TRADE_STATUS_TARGET_DEAD        = 0x12,
    TRADE_STATUS_YOU_LOGOUT         = 0x13,
    TRADE_STATUS_TARGET_LOGOUT      = 0x14,
    TRADE_STATUS_TRIAL_ACCOUNT      = 0x15,
    TRADE_STATUS_ONLY_CONJURED      = 0x16,
};
enum TRADE_DATA
{
    TRADE_GIVE      = 0x00,
    TRADE_RECEIVE    = 0x01,
};
enum DUEL_STATUS
{
    DUEL_STATUS_OUTOFBOUNDS,
    DUEL_STATUS_INBOUNDS
};
enum DUEL_STATE
{
    DUEL_STATE_REQUESTED,
    DUEL_STATE_STARTED,
    DUEL_STATE_FINISHED
};
enum DUEL_WINNER
{
    DUEL_WINNER_KNOCKOUT,
    DUEL_WINNER_RETREAT,
};

#define PLAYER_ATTACK_TIMEOUT_INTERVAL  5000
#define PLAYER_FORCED_RESURECT_INTERVAL 360000 // 1000*60*6= 6 minutes

// Crow: Equipment set shit.
enum EquipmentSetUpdateState
{
    EQUIPMENT_SET_UNCHANGED = 0,
    EQUIPMENT_SET_CHANGED   = 1,
    EQUIPMENT_SET_NEW       = 2,
    EQUIPMENT_SET_DELETED   = 3
};

struct EquipmentSet
{
    EquipmentSet() : Guid(0), state(EQUIPMENT_SET_NEW)
    {
        for(uint8 i = 0; i < EQUIPMENT_SLOT_END; i++)
            Items[i] = 0;
    }

    uint64 Guid;
    std::string Name;
    std::string IconName;
    WoWGuid Items[EQUIPMENT_SLOT_END];
    EquipmentSetUpdateState state;
};

#define MAX_EQUIPMENT_SET_INDEX 10  // client limit

struct PlayerSkill
{
    SkillLineEntry * Skill;
    uint32 CurrentValue;
    uint32 MaximumValue;
    uint32 BonusValue;
    float GetSkillUpChance();
    bool Reset(uint32 Id);
};

enum SPELL_INDEX
{
    SPELL_TYPE_INDEX_MARK           = 1,
    SPELL_TYPE_INDEX_POLYMORPH      = 2,
    SPELL_TYPE_INDEX_FEAR           = 3,
    SPELL_TYPE_INDEX_SAP            = 4,
    SPELL_TYPE_INDEX_SCARE_BEAST    = 5,
    SPELL_TYPE_INDEX_HIBERNATE      = 6,
    SPELL_TYPE_INDEX_EARTH_SHIELD   = 7,
    SPELL_TYPE_INDEX_CYCLONE        = 8,
    SPELL_TYPE_INDEX_BANISH         = 9,
    SPELL_TYPE_INDEX_JUDGEMENT      = 10,
    SPELL_TYPE_INDEX_REPENTANCE     = 11,
    SPELL_TYPE_INDEX_SLOW           = 12,
    NUM_SPELL_TYPE_INDEX            = 13,
};

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
#define PLAYER_RATING_MODIFIER_HIT_AVOIDANCE_RATING             PLAYER_FIELD_COMBAT_RATING_1+22
#define PLAYER_RATING_MODIFIER_EXPERTISE                        PLAYER_FIELD_COMBAT_RATING_1+23
#define PLAYER_RATING_MODIFIER_ARMOR_PENETRATION_RATING         PLAYER_FIELD_COMBAT_RATING_1+24

static uint32 ItemRatingValues[BLOCK_VALUE+1] = {
    0, // POWER
    0, // HEALTH
    0, // UNK
    0, // AGILITY
    0, // STRENGTH
    0, // INTELLECT
    0, // SPIRIT
    0, // STAMINA
    0, // WEAPON_SKILL_RATING
    0, // UNK8
    0, // UNK9
    0, // UNK10
    PLAYER_RATING_MODIFIER_DEFENCE, // DEFENSE RATING
    PLAYER_RATING_MODIFIER_DODGE, // DODGE RATING
    PLAYER_RATING_MODIFIER_PARRY, // PARRY RATING
    PLAYER_RATING_MODIFIER_BLOCK, // SHIELD_BLOCK_RATING
    PLAYER_RATING_MODIFIER_MELEE_HIT, // MELEE HIT_RATING
    PLAYER_RATING_MODIFIER_RANGED_HIT, // RANGED HIT_RATING
    PLAYER_RATING_MODIFIER_SPELL_HIT, // SPELL HIT_RATING
    PLAYER_RATING_MODIFIER_MELEE_CRIT, // MELEE_CRITICAL_STRIKE_RATING
    PLAYER_RATING_MODIFIER_RANGED_CRIT, // RANGED_CRITICAL_STRIKE_RATING
    PLAYER_RATING_MODIFIER_SPELL_CRIT, // SPELL_CRITICAL_STRIKE_RATING
    PLAYER_RATING_MODIFIER_MELEE_HIT_AVOIDANCE, // MELEE_HIT_AVOIDANCE_RATING
    PLAYER_RATING_MODIFIER_RANGED_HIT_AVOIDANCE, // RANGED_HIT_AVOIDANCE_RATING
    PLAYER_RATING_MODIFIER_SPELL_HIT_AVOIDANCE, // SPELL_HIT_AVOIDANCE_RATING
    PLAYER_RATING_MODIFIER_MELEE_RESILIENCE, // MELEE_CRITICAL_AVOIDANCE_RATING
    PLAYER_RATING_MODIFIER_RANGED_RESILIENCE, // RANGED_CRITICAL_AVOIDANCE_RATING
    PLAYER_RATING_MODIFIER_SPELL_RESILIENCE, // SPELL_CRITICAL_AVOIDANCE_RATING
    PLAYER_RATING_MODIFIER_MELEE_HASTE, // MELEE_HASTE_RATING
    PLAYER_RATING_MODIFIER_RANGED_HASTE, // RANGED_HASTE_RATING
    PLAYER_RATING_MODIFIER_SPELL_HASTE, // SPELL_HASTE_RATING
    PLAYER_RATING_MODIFIER_MELEE_HIT, // HIT_RATING
    PLAYER_RATING_MODIFIER_MELEE_CRIT, // CRITICAL_STRIKE_RATING
    PLAYER_RATING_MODIFIER_MELEE_HIT_AVOIDANCE, // HIT_AVOIDANCE_RATING
    PLAYER_RATING_MODIFIER_MELEE_RESILIENCE, // CRITICAL_AVOIDANCE_RATING
    PLAYER_RATING_MODIFIER_MELEE_RESILIENCE, // RESILIENCE_RATING
    PLAYER_RATING_MODIFIER_MELEE_HASTE, // HASTE_RATING
    PLAYER_RATING_MODIFIER_EXPERTISE, // EXPERTISE_RATING
    0, // ATTACK_POWER
    0, // RANGED_ATTACK_POWER
    0, // FERAL_ATTACK_POWER
    0, // SPELL_HEALING_DONE
    0, // SPELL_DAMAGE_DONE
    0, // MANA_REGENERATION
    PLAYER_RATING_MODIFIER_ARMOR_PENETRATION_RATING, // ARMOR_PENETRATION_RATING
    0, // SPELL_POWER
    0, // HEALTH_REGEN
    0, // SPELL_PENETRATION
    0  // BLOCK_VALUE
};

class ArenaTeam;
struct PlayerCooldown
{
    uint32 ExpireTime;
    uint32 ItemId;
    uint32 SpellId;
};

//====================================================================
//  Player
//  Class that holds every created character on the server.
//
//  TODO:  Attach characters to user accounts
//====================================================================
typedef std::set<uint32>                            SpellSet;
typedef std::list<uint32>                           SpellList;
typedef std::list<classScriptOverride*>             ScriptOverrideList;
typedef std::set<uint32>                            SaveSet;
typedef std::map<uint32, ScriptOverrideList* >      SpellOverrideMap;
typedef std::map<uint32, uint32>                    SpellOverrideExtraAuraMap;
typedef std::map<uint32, FactionReputation*>        ReputationMap;
typedef std::map<uint32, uint64>                    SoloSpells;
typedef std::map<SpellEntry*, pair<uint32, uint32> >StrikeSpellMap;
typedef std::map<uint32, OnHitSpell >               StrikeSpellDmgMap;
typedef std::unordered_map<uint32, PlayerSkill>     SkillMap;
typedef std::set<Player* *>                         ReferenceSet;
typedef std::map<uint32, PlayerCooldown>            PlayerCooldownMap;

class SERVER_DECL Player : public Unit
{
    friend class WorldSession;
    friend class Pet;
    friend class SkillIterator;

public:
    Player ( uint32 guid );
    ~Player ( );
    virtual void Init();
    virtual void Destruct();

    void EventGroupFullUpdate();
    void GroupUninvite(Player* player, PlayerInfo *info);

    /************************************************************************/
    /* Skill System                                                         */
    /************************************************************************/

    void _AdvanceSkillLine(uint32 SkillLine, uint32 Count = 1);
    void _AddSkillLine(uint32 SkillLine, uint32 Current, uint32 Max);
    uint32 _GetSkillLineMax(uint32 SkillLine);
    uint32 _GetSkillLineCurrent(uint32 SkillLine, bool IncludeBonus = true);
    void _RemoveSkillLine(uint32 SkillLine);
    void _UpdateMaxSkillCounts();
    void _ModifySkillBonus(uint32 SkillLine, int32 Delta);
    void _ModifySkillBonusByType(uint32 SkillType, int32 Delta);
    bool _HasSkillLine(uint32 SkillLine);
    void RemoveSpellsFromLine(uint32 skill_line);
    void _RemoveAllSkills();
    void _RemoveLanguages();
    void _AddLanguages(bool All);
    void _AdvanceAllSkills(uint32 count, bool skipprof = false, uint32 max = 0);
    void _ModifySkillMaximum(uint32 SkillLine, uint32 NewMax);


    void RecalculateHonor();

    LfgMatch * m_lfgMatch;
    uint32 m_lfgInviterGuid;

    void EventTimeoutLfgInviter();


public:
    void EventDBCChatUpdate(uint32 dbcID);
protected:
    bool AllowChannelAtLocation(uint32 dbcID, AreaTableEntry *areaTable);
    bool UpdateChatChannel(const char* areaName, AreaTableEntry *areaTable, ChatChannelDBC* entry, Channel* channel);

protected:

    void _UpdateSkillFields();

    SkillMap m_skills;

    // COOLDOWNS
    PlayerCooldownMap m_cooldownMap[NUM_COOLDOWN_TYPES];
    uint32 m_globalCooldown;

public:
    void Cooldown_OnCancel(SpellEntry *pSpell);
    void Cooldown_AddStart(SpellEntry * pSpell);
    void Cooldown_Add(SpellEntry * pSpell, Item* pItemCaster);
    void Cooldown_AddItem(ItemPrototype * pProto, uint32 x);
    bool Cooldown_CanCast(SpellEntry * pSpell);
    bool Cooldown_CanCast(ItemPrototype * pProto, uint32 x);

protected:
    void _Cooldown_Add(uint32 Type, uint32 Misc, uint32 Time, uint32 SpellId, uint32 ItemId);
    void _LoadPlayerCooldowns(QueryResult * result);
    void _SavePlayerCooldowns(QueryBuffer * buf);

    // END COOLDOWNS

public:

    bool ok_to_remove;
    uint64 m_spellIndexTypeTargets[NUM_SPELL_TYPE_INDEX];
    void EquipInit(PlayerCreateInfo *EquipInfo);
    void RemoveSpellTargets(uint32 Type);
    void RemoveSpellIndexReferences(uint32 Type);
    void SetSpellTargetType(uint32 Type, Unit* target);
    void SendMeetingStoneQueue(uint32 DungeonId, uint8 Status);
    void SendDungeonDifficulty();
    void SendRaidDifficulty();

    void AddToWorld(bool loggingin = false);
    void AddToWorld(MapMgr* pMapMgr);
    void RemoveFromWorld();
    bool Create ( WorldPacket &data );

    void Update( uint32 time );

    void BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag);
    std::string m_afk_reason;
    void SetAFKReason(std::string reason) { m_afk_reason = reason; };
    HEARTHSTONE_INLINE const char* GetName() { return m_name.c_str(); }
    HEARTHSTONE_INLINE std::string* GetNameString() { return &m_name; }

    //void KilledMonster(uint32 entry, const uint64 &guid);
    void GiveXP(uint32 xp, const uint64 &guid, bool allowbonus);   // to stop rest xp being given
    void ModifyBonuses(uint32 type,int32 val);
    std::map<uint32, uint32> m_wratings;

    /************************************************************************/
    /* Taxi                                                                 */
    /************************************************************************/
    HEARTHSTONE_INLINE TaxiPath*    GetTaxiPath() { return m_CurrentTaxiPath; }
    HEARTHSTONE_INLINE bool         GetTaxiState() { return m_onTaxi; }
    const uint32&       GetTaximask( uint8 index ) const { ASSERT(index < MAX_TAXI); return m_taximask[index]; }
    void                LoadTaxiMask(const char* data);
    void                TaxiStart(TaxiPath* path, uint32 modelid, uint32 start_node);
    void                JumpToEndTaxiNode(TaxiPath * path);
    void                EventDismount(uint32 money, float x, float y, float z);
    void                EventTaxiInterpolate();

    HEARTHSTONE_INLINE void         SetTaxiState    (bool state) { m_onTaxi = state; }
    HEARTHSTONE_INLINE void         SetTaximask     (uint8 index, uint32 value ) { ASSERT(index < MAX_TAXI); m_taximask[index] = value; }
    HEARTHSTONE_INLINE void         SetTaxiPath     (TaxiPath *path) { m_CurrentTaxiPath = path; }
    HEARTHSTONE_INLINE void         SetTaxiPos()    {m_taxi_pos_x = m_position.x; m_taxi_pos_y = m_position.y; m_taxi_pos_z = m_position.z;}
    HEARTHSTONE_INLINE void         UnSetTaxiPos()  {m_taxi_pos_x = 0; m_taxi_pos_y = 0; m_taxi_pos_z = 0; }

    // Taxi related variables
    vector<TaxiPath*>   m_taxiPaths;
    TaxiPath*           m_CurrentTaxiPath;
    uint32              taxi_model_id;
    uint32              lastNode;
    uint32              m_taxi_ride_time;
    uint32              m_taximask[MAX_TAXI];
    float               m_taxi_pos_x;
    float               m_taxi_pos_y;
    float               m_taxi_pos_z;
    bool                m_onTaxi;
    uint32              m_taxiMapChangeNode;

    /************************************************************************/
    /* Quests                                                               */
    /************************************************************************/
    bool HasQuests()
    {
        for(int i = 0; i < QUEST_LOG_COUNT; i++)
        {
            if(m_questlog[i] != 0)
                return true;
        }
        return false;
    }

    bool HasQuest(uint32 qid)
    {
        return GetQuestLogForEntry(qid) != NULL;
    }

    int32               GetOpenQuestSlot();
    uint32              GetQuestStatusForQuest(uint32 questid, uint8 type = 1, bool skiplevelcheck = false);
    QuestLogEntry*      GetQuestLogForEntry(uint32 quest);
    HEARTHSTONE_INLINE  QuestLogEntry*GetQuestLogInSlot(uint32 slot){ return m_questlog[slot]; }
    HEARTHSTONE_INLINE  uint32 GetQuestSharer(){ return m_questSharer; }

    HEARTHSTONE_INLINE void SetQuestSharer(uint32 guid){ m_questSharer = guid; }
    void                SetQuestLogSlot(QuestLogEntry *entry, uint32 slot);

    HEARTHSTONE_INLINE void     PushToRemovedQuests(uint32 questid) { m_removequests.insert(questid);}
    HEARTHSTONE_INLINE uint32       GetFinishedDailiesCount() { return (uint32)m_finishedDailyQuests.size(); }
    void                AddToFinishedQuests(uint32 quest_id);
    void                AddToFinishedDailyQuests(uint32 quest_id);
    void                EventTimedQuestExpire(Quest *qst, QuestLogEntry *qle, uint32 log_slot, uint32 interval);

    bool                HasFinishedQuest(uint32 quest_id);
    bool                HasFinishedDailyQuest(uint32 quest_id);
    bool                HasQuestForItem(uint32 itemid);
    bool                CanFinishQuest(Quest* qst);
    bool                HasQuestSpell(uint32 spellid);
    void                RemoveQuestSpell(uint32 spellid);
    bool                HasQuestMob(uint32 entry);
    void                RemoveQuestMob(uint32 entry);
    void                RemoveQuestsFromLine(uint32 skill_line);
    void                ResetDailyQuests();
    uint16              FindQuestSlot(uint32 questid);
    uint32 GetQuestSlotQuestId(uint16 slot) const { return GetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot * 5 + (uint32)NULL); }

    //Quest related variables
    QuestLogEntry*      m_questlog[QUEST_LOG_COUNT];
    std::set<uint32>    m_QuestGOInProgress;
    std::set<uint32>    m_removequests;
    std::set<uint32>    m_finishedQuests;
    std::set<uint32>    m_finishedDailyQuests;
    uint32              m_questSharer;
    std::set<uint32>    quest_spells;
    std::set<uint32>    quest_mobs;
    Mutex               DailyMutex;

    void EventPortToGM(uint32 guid);
    HEARTHSTONE_INLINE uint32 GetTeam() { return m_team; }
    HEARTHSTONE_INLINE void SetTeam(uint32 t) { m_team = t; m_bgTeam=t; }
    HEARTHSTONE_INLINE void ResetTeam() { m_team = myRace->TeamId; m_bgTeam=m_team; }

    HEARTHSTONE_INLINE bool IsInFeralForm()
    {
        int s = GetShapeShift();
        if( s <= 0 )
            return false;

        // Fight forms that do not use player's weapon
        //          Cat       Bear      Dire Bear
        return ( s == 1 || s == 5 || s == 8 );
    }
    void CalcDamage();
    uint32 GetMainMeleeDamage(uint32 AP_owerride); //i need this for windfury

    const uint64& GetSelection( ) const { return m_curSelection; }
    void SetSelection(const uint64 &guid) { m_curSelection = guid; }

    /************************************************************************/
    /* Spells                                                               */
    /************************************************************************/
    bool HasSpell(uint32 spell);
    SpellEntry* GetSpellWithNamehash(uint32 namehash);
    bool HasHigherSpellForSkillLine(SpellEntry* sp);
    void smsg_InitialSpells();
    void smsg_TalentsInfo(bool pet);
    void BuildPlayerTalentsInfo(WorldPacket *data);
    void BuildPetTalentsInfo(WorldPacket *data);
    void addSpell(uint32 spell_idy);
    void removeSpellByHashName(uint32 hash);
    bool removeSpell(uint32 SpellID);
    uint32 FindSpellWithNamehash(uint32 namehash);
    uint32 FindHigherRankingSpellWithNamehash(uint32 namehash, uint32 minimumrank);
    uint32 FindHighestRankingSpellWithNamehash(uint32 namehash);
    bool CanFlyInCurrentZoneOrMap();
    SpellEntry* FindLowerRankSpell(SpellEntry* sp, int32 rankdiff);
    uint32 forget;

    // PLEASE DO NOT INLINE!
    void AddOnStrikeSpell(SpellEntry* sp, uint32 delay)
    {
        m_onStrikeSpells.insert( map< SpellEntry*, pair<uint32, uint32> >::value_type( sp, make_pair( delay, 0 ) ) );
    }
    void RemoveOnStrikeSpell(SpellEntry *sp)
    {
        m_onStrikeSpells.erase(sp);
    }
    void AddOnStrikeSpellDamage(uint32 spellid, uint32 mindmg, uint32 maxdmg)
    {
        OnHitSpell sp;
        sp.spellid = spellid;
        sp.mindmg = mindmg;
        sp.maxdmg = maxdmg;
        m_onStrikeSpellDmg[spellid] = sp;
    }
    void RemoveOnStrikeSpellDamage(uint32 spellid)
    {
        m_onStrikeSpellDmg.erase(spellid);
    }

    //Spells variables
    StrikeSpellMap      m_onStrikeSpells;
    StrikeSpellDmgMap   m_onStrikeSpellDmg;
    SpellOverrideMap    mSpellOverrideMap;
    SpellSet            mSpells;
    SpellSet            mShapeShiftSpells;

    void AddShapeShiftSpell(uint32 id);
    void RemoveShapeShiftSpell(uint32 id);

    /************************************************************************/
    /* Talents                                                              */
    /************************************************************************/
    bool HasTalent(uint8 spec, uint32 talentid);

    /************************************************************************/
    /* Actionbar                                                            */
    /************************************************************************/
    void                setAction(uint8 button, uint32 action, uint8 type, int8 SpecOverride = -1);
    void                SendInitialActions();
    bool                m_actionsDirty;

    /************************************************************************/
    /* Reputation                                                           */
    /************************************************************************/
    void                ModStanding(uint32 Faction, int32 Value);
    int32               GetStanding(uint32 Faction);
    int32               GetBaseStanding(uint32 Faction);
    void                SetStanding(uint32 Faction, int32 Value);
    void                SetAtWar(uint32 Faction, bool Set);
    bool                IsAtWar(uint32 Faction);
    Standing            GetStandingRank(uint32 Faction);
    bool                IsHostileBasedOnReputation(FactionEntry *faction);
    void                UpdateInrangeSetsBasedOnReputation();
    void                Reputation_OnKilledUnit(Unit* pUnit, bool InnerLoop);
    void                Reputation_OnTalk(FactionEntry *faction);
    bool                AddNewFaction( FactionEntry *faction, int32 standing, bool base );
    void                OnModStanding( FactionEntry *faction, FactionReputation * rep );
    static Standing     GetReputationRankFromStanding(int32 Standing_);

    bool titanGrip;
    void ResetTitansGrip();

    /************************************************************************/
    /* Factions                                                             */
    /************************************************************************/
    void smsg_InitialFactions();
    uint32 GetInitialFactionId();
    // factions variables
    int32 pctReputationMod;

    /************************************************************************/
    /* PVP                                                                  */
    /************************************************************************/
    HEARTHSTONE_INLINE uint8 GetPVPRank()
    {
        return (uint8)((GetUInt32Value(PLAYER_BYTES_3) >> 24) & 0xFF);
    }
    HEARTHSTONE_INLINE void SetPVPRank(int newrank)
    {
        SetUInt32Value(PLAYER_BYTES_3, ((GetUInt32Value(PLAYER_BYTES_3) & 0x00FFFFFF) | (uint8(newrank) << 24)));
    }
    uint32 GetMaxPersonalRating(bool Ignore2v2);


    /************************************************************************/
    /* Groups                                                               */
    /************************************************************************/
    void                            SetInviter(uint32 pInviter) { m_GroupInviter = pInviter; }
    HEARTHSTONE_INLINE uint32       GetInviter() { return m_GroupInviter; }
    HEARTHSTONE_INLINE bool         InGroup() { return (m_playerInfo->m_Group != NULL && !m_GroupInviter); }
    bool                            IsGroupLeader()
    {
        if(m_playerInfo->m_Group != NULL)
        {
            if(m_playerInfo->m_Group->GetLeader() == m_playerInfo)
                return true;
        }
        return false;
    }
    bool IsGroupMember(Player* plyr);
    HEARTHSTONE_INLINE int      HasBeenInvited() { return m_GroupInviter != 0; }
    HEARTHSTONE_INLINE Group*   GetGroup() { return m_playerInfo != NULL ? m_playerInfo->m_Group : NULL; }
    HEARTHSTONE_INLINE uint32   GetGroupID() { return m_playerInfo != NULL ? m_playerInfo->m_Group != NULL ? m_playerInfo->m_Group->GetID(): NULL: NULL; }
    HEARTHSTONE_INLINE int8     GetSubGroup() { return m_playerInfo->subGroup; }
    HEARTHSTONE_INLINE bool     IsBanned()
    {
        if(m_banned)
        {
            if(m_banned < 100 || (uint32)UNIXTIME < m_banned)
                return true;
        }
        return false;
    }

    HEARTHSTONE_INLINE void     SetBanned() { m_banned = 4;}
    HEARTHSTONE_INLINE void     SetBanned(string Reason) { m_banned = 4; m_banreason = Reason;}
    HEARTHSTONE_INLINE void     SetBanned(uint32 timestamp, string& Reason) { m_banned = timestamp; m_banreason = Reason; }
    HEARTHSTONE_INLINE void     UnSetBanned() { m_banned = 0; }
    HEARTHSTONE_INLINE string   GetBanReason() {return m_banreason;}

    void SetGuardHostileFlag(bool val) { if(val) SetFlag(PLAYER_FLAGS, PLAYER_FLAG_UNKNOWN2); else RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_UNKNOWN2); UpdateOppFactionSet(); }
    void CreateResetGuardHostileFlagEvent()
    {
        event_RemoveEvents( EVENT_GUARD_HOSTILE );
        sEventMgr.AddEvent(this, &Player::SetGuardHostileFlag, false, EVENT_GUARD_HOSTILE, 10000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    uint32 m_hasInRangeGuards;

    /************************************************************************/
    /* Guilds                                                               */
    /************************************************************************/
    HEARTHSTONE_INLINE bool IsInGuild() { return (GetUInt32Value(PLAYER_GUILDID) > 0); }
    HEARTHSTONE_INLINE uint32 GetGuildId() { return GetUInt32Value(PLAYER_GUILDID); }
    HEARTHSTONE_INLINE uint32 GetGuildRank() { return GetUInt32Value(PLAYER_GUILDRANK); }

    void SetGuildId(uint32 guildId)
    {
        uint32 objectType = GetUInt32Value(OBJECT_FIELD_TYPE);
        if(guildId) objectType |= TYPEMASK_IN_GUILD;
        else objectType &= ~TYPEMASK_IN_GUILD;
        SetUInt64Value(PLAYER_GUILDID, guildId ? MAKE_NEW_GUID(guildId, 0, HIGHGUID_TYPE_GUILD) : 0);
        SetUInt32Value(OBJECT_FIELD_TYPE, objectType);
    }
    void SetGuildRank(uint32 guildRank) { SetUInt32Value(PLAYER_GUILDRANK, guildRank); }

    uint32 GetGuildInvitersGuid() { return m_invitersGuid; }
    void SetGuildInvitersGuid( uint32 guid ) { m_invitersGuid = guid; }
    void UnSetGuildInvitersGuid() { m_invitersGuid = 0; }

    /************************************************************************/
    /* Duel                                                                 */
    /************************************************************************/
    void                RequestDuel(Player* pTarget);
    void                DuelBoundaryTest();
    void                EndDuel(uint8 WinCondition);
    void                DuelCountdown();
    void                SetDuelStatus(uint8 status) { m_duelStatus = status; }
    HEARTHSTONE_INLINE uint8        GetDuelStatus() { return m_duelStatus; }
    void                SetDuelState(uint8 state) { m_duelState = state; }
    HEARTHSTONE_INLINE uint8        GetDuelState() { return m_duelState; }
    // duel variables
    Player*             DuelingWith;

    /************************************************************************/
    /* Trade                                                                */
    /************************************************************************/
    void SendTradeUpdate(void);
    void ResetTradeVariables()
    {
        mTradeGold = 0;
        memset(&mTradeItems, 0, sizeof(Item*)*7);
        mTradeStatus = 0;
        mTradeTarget = 0;
        m_tradeSequence = 2;
    }

    /************************************************************************/
    /* Pets                                                                 */
    /************************************************************************/
    HEARTHSTONE_INLINE void SetSummon(Pet* pet) { m_Summon = pet; }
    HEARTHSTONE_INLINE Pet* GetSummon(void) { return m_Summon; }
    uint32 GeneratePetNumber(void);
    void RemovePlayerPet(uint32 pet_number);
    HEARTHSTONE_INLINE void AddPlayerPet(PlayerPet* pet, uint32 index) { m_Pets[index] = pet; }
    HEARTHSTONE_INLINE PlayerPet* GetPlayerPet(uint8 idx)
    {
        PlayerPet* petReturn = NULL;
        PetLocks.Acquire();
        std::map<uint32, PlayerPet*>::iterator itr = m_Pets.find(idx);
        if(itr != m_Pets.end())
            petReturn = itr->second;
        PetLocks.Release();
        return petReturn;
    }

    void SpawnPet(uint32 pet_number);
    void DespawnPet();
    uint32 GetFirstPetNumber(void)
    {
        if(m_Pets.size() == 0)
            return 0;

        PetLocks.Acquire();
        std::map<uint32, PlayerPet*>::iterator itr = m_Pets.begin();
        uint32 petNum = itr->first;
        PetLocks.Release();
        return petNum;
    }
    HEARTHSTONE_INLINE PlayerPet* GetFirstPet(void) { return GetPlayerPet(GetFirstPetNumber()); }
    HEARTHSTONE_INLINE void SetStableSlotCount(uint8 count) { m_StableSlotCount = count; }
    HEARTHSTONE_INLINE uint8 GetStableSlotCount(void) { return m_StableSlotCount; }
    uint8 GetUnstabledPetNumber(void)
    {
        if(m_Pets.size() == 0)
            return 0;

        uint8 UnstabledPetNum = 0;
        PetLocks.Acquire();
        std::map<uint32, PlayerPet*>::iterator itr = m_Pets.begin();
        for(;itr != m_Pets.end();itr++)
            if(itr->second->stablestate == STABLE_STATE_ACTIVE)
                UnstabledPetNum = itr->first;
        PetLocks.Release();
        return UnstabledPetNum;
    }
    void EventSummonPet(Pet* new_pet); //if we charmed or simply summoned a pet, this function should get called
    void EventDismissPet(); //if pet/charm died or whatever happned we should call this function

    /************************************************************************/
    /* Item Interface                                                       */
    /************************************************************************/
    HEARTHSTONE_INLINE ItemInterface* GetItemInterface() { return m_ItemInterface; } // Player Inventory Item storage
    HEARTHSTONE_INLINE void ApplyItemMods(Item* item, int16 slot, bool apply,bool justdrokedown=false) {  _ApplyItemMods(item, slot, apply, justdrokedown); }

    // item interface variables
    ItemInterface* m_ItemInterface;

    HEARTHSTONE_INLINE AchievementInterface* GetAchievementInterface() { return m_achievementInterface; }
    AchievementInterface * m_achievementInterface;

    /************************************************************************/
    /* Loot                                                                 */
    /************************************************************************/
    HEARTHSTONE_INLINE const uint64& GetLootGUID() const { return m_lootGuid; }
    HEARTHSTONE_INLINE          void SetLootGUID(const uint64 &guid) { m_lootGuid = guid; }
    void SendLoot(uint64 guid, uint32 mapid, uint8 loot_type);
    // loot variables
    uint64 m_lootGuid;
    uint64 m_currentLoot;
    bool m_insigniaTaken;
    bool CanNeedItem(ItemPrototype* proto);

    /************************************************************************/
    /* World Session                                                        */
    /************************************************************************/

    HEARTHSTONE_INLINE WorldSession* GetSession() const { return m_session; }
    void SetSession(WorldSession *s) { m_session = s; }
    void SetBindPoint(float x, float y, float z, uint32 m, uint32 v) { m_bind_pos_x = x; m_bind_pos_y = y; m_bind_pos_z = z; m_bind_mapid = m; m_bind_zoneid = v;}
    float offhand_dmg_mod;
    float GetSpellTimeMod(uint32 id);
    int GetSpellDamageMod(uint32 id);
    int32 GetSpellManaMod(uint32 id);

    // Talents
    // These functions build a specific type of A9 packet
    uint32 __fastcall BuildCreateUpdateBlockForPlayer( ByteBuffer *data, Player* target );
    virtual void DestroyForPlayer( Player* target, bool anim = false);
    void SetTalentHearthOfWildPCT(int value){hearth_of_wild_pct=value;}
    void EventTalentHeartOfWildChange(bool apply);

    std::list<LoginAura> loginauras;

    std::set<uint32> OnMeleeAuras;

    /************************************************************************/
    /* Player loading and savings                                           */
    /* Serialize character to db                                            */
    /************************************************************************/
    void SaveToDB(bool bNewCharacter);
    void SaveAuras(stringstream&);
    bool LoadFromDB(uint32 guid);
    void LoadFromDBProc(QueryResultVector & results);

    void LoadNamesFromDB(uint32 guid);
    bool m_FirstLogin;

    /************************************************************************/
    /* Death system                                                         */
    /************************************************************************/
    void SpawnCorpseBones();
    Corpse* CreateCorpse();
    void KillPlayer();
    void ResurrectPlayer(Unit* pResurrector = NULLPLR);
    void BuildPlayerRepop();
    Corpse* RepopRequestedPlayer();
    void DecReclaimCount() { if(ReclaimCount > 0) --ReclaimCount; };
    uint32 ReclaimCount;
    bool Wispform;

    // silly event handler
    void EventRepopRequestedPlayer() { RepopRequestedPlayer(); }

    void DeathDurabilityLoss(double percent);
    void RepopAtGraveyard(float ox, float oy, float oz, uint32 mapid);

    /************************************************************************/
    /* Movement system                                                      */
    /************************************************************************/
    void SetMovement(uint8 pType, uint32 flag);
    void SetPlayerSpeed(uint8 SpeedType, float value);
    float GetPlayerSpeed(){return m_runSpeed;}
    uint8 m_currentMovement;
    bool m_isMoving;
    uint8 m_isWaterWalking;
    uint32 m_WaterWalkTimer;
    //Invisibility stuff
    bool m_isGmInvisible;

    /************************************************************************/
    /* Channel stuff                                                        */
    /************************************************************************/
    void JoinedChannel(Channel *c);
    void LeftChannel(Channel *c);
    void CleanupChannels();
    //Attack stuff
    void EventAttackStart();
    void EventAttackStop();
    void EventAttackUpdateSpeed() { }
    void EventDeath();
    //Note:ModSkillLine -> value+=amt;ModSkillMax -->value=amt; --wierd
    float GetSkillUpChance(uint32 id);
    float SpellCrtiticalStrikeRatingBonus;
    float CalculateCritFromAgilForClassAndLevel(uint32 _class, uint32 _level);
    float CalculateDefenseFromAgilForClassAndLevel(uint32 _class, uint32 _level);
    float SpellHasteRatingBonus;
    void UpdateAttackSpeed();
    void UpdateChances();
    void UpdateStats();
    void UpdateHit(int32 hit);

    bool canCast(SpellEntry *m_spellInfo);
    HEARTHSTONE_INLINE float GetBlockFromSpell() { return m_blockfromspell; }
    HEARTHSTONE_INLINE float GetSpellCritFromSpell() { return m_spellcritfromspell; }
    HEARTHSTONE_INLINE float GetHitFromMeleeSpell() { return m_hitfrommeleespell; }
    HEARTHSTONE_INLINE float GetHitFromSpell() { return m_hitfromspell; }
    HEARTHSTONE_INLINE float GetParryFromSpell() { return m_parryfromspell; }
    HEARTHSTONE_INLINE float GetDodgeFromSpell() { return m_dodgefromspell; }
    void SetBlockFromSpell(float value) { m_blockfromspell = value; }
    void SetSpellCritFromSpell(float value) { m_spellcritfromspell = value; }
    void SetParryFromSpell(float value) { m_parryfromspell = value; }
    void SetDodgeFromSpell(float value) { m_dodgefromspell = value; }
    void SetHitFromMeleeSpell(float value) { m_hitfrommeleespell = value; }
    void SetHitFromSpell(float value) { m_hitfromspell = value; }
    HEARTHSTONE_INLINE int32 GetHealthFromSpell() { return m_healthfromspell; }
    HEARTHSTONE_INLINE uint32 GetManaFromSpell() { return m_manafromspell; }
    void SetHealthFromSpell(int32 value) { m_healthfromspell = value;}
    void SetManaFromSpell(uint32 value) { m_manafromspell = value;}
    uint32 CalcTalentResetCost(uint32 resetnum);
    void SendTalentResetConfirm();
    void SendPetUntrainConfirm();
    void SendXPToggleConfirm();
    uint32 GetTalentResetTimes() { return m_talentresettimes; }
    HEARTHSTONE_INLINE void SetTalentResetTimes(uint32 value) { m_talentresettimes = value; }
    void SetPlayerStatus(uint8 pStatus) { m_status = pStatus; }
    void CheckPlayerStatus(uint8 pStatus) { if(m_status == pStatus) m_status = NONE; }
    HEARTHSTONE_INLINE uint8 GetPlayerStatus() { return m_status; }
    const float& GetBindPositionX( ) const { return m_bind_pos_x; }
    const float& GetBindPositionY( ) const { return m_bind_pos_y; }
    const float& GetBindPositionZ( ) const { return m_bind_pos_z; }
    const uint32& GetBindMapId( ) const { return m_bind_mapid; }
    const uint32& GetBindZoneId( ) const { return m_bind_zoneid; }
    HEARTHSTONE_INLINE uint8 GetShapeShift()
    {
        return GetByte(UNIT_FIELD_BYTES_2, 3);
    }

    void delayAttackTimer(int32 delay)
    {
        if(!delay)
            return;

        m_attackTimer += delay;
        m_attackTimer_1 += delay;
    }

    void SetShapeShift(uint8 ss);
    uint32 GetSpellForShapeshiftForm(uint8 ss, bool spellchecks = false);

    //Showing Units WayPoints
    AIInterface* waypointunit;

    bool ForceSaved;
    uint32 m_nextSave;
    void SendCinematic(uint32 cinematicid) { GetSession()->OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &cinematicid); };

    //Base stats calculations
    //void CalcBaseStats();

    // Rest
    void HandleRestedCalculations(bool rest_on);
    uint32 SubtractRestXP(uint32 &amount);
    void AddCalculatedRestXP(uint32 seconds);
    void ApplyPlayerRestState(bool apply);
    void SetLastAreaTrigger(AreaTrigger*trigger) { LastAreaTrigger = trigger; };
    void UpdateRestState();
    float z_axisposition;
    int32 m_safeFall;
    bool safefall;
    // Gossip
    GossipMenu* CurrentGossipMenu;
    void CleanupGossipMenu();
    void Gossip_Complete();
    int m_lifetapbonus;
    uint32 m_lastShotTime;

    bool m_bUnlimitedBreath;
    void SendMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, int32 Regen);
    void StopMirrorTimer(MirrorTimerType Type);
    void HandleBreathing(uint32 time_diff);
    int32 m_MirrorTimer[3];
    uint32 m_UnderwaterState;
    uint32 m_LastUnderwaterState;
    uint32 m_UnderwaterTime;

    // Beast
    bool m_BeastMaster;
    uint32 m_lastWarnCounter;

    // Visible objects
    bool CanSee(Object* obj);
    HEARTHSTONE_INLINE bool IsVisible(Object* pObj) { return !(m_visibleObjects.find(pObj) == m_visibleObjects.end()); }
    void AddInRangeObject(Object* pObj);
    void OnRemoveInRangeObject(Object* pObj);
    void ClearInRangeSet();
    HEARTHSTONE_INLINE void AddVisibleObject(Object* pObj) { m_visibleObjects.insert(pObj); }
    HEARTHSTONE_INLINE void RemoveVisibleObject(Object* pObj) { m_visibleObjects.erase(pObj); }
    HEARTHSTONE_INLINE void RemoveVisibleObject(InRangeSet::iterator itr) { m_visibleObjects.erase(itr); }
    HEARTHSTONE_INLINE InRangeSet::iterator FindVisible(Object* obj) { return m_visibleObjects.find(obj); }
    HEARTHSTONE_INLINE void RemoveIfVisible(Object* obj)
    {
        InRangeSet::iterator itr = m_visibleObjects.find(obj);
        if(itr == m_visibleObjects.end())
            return;

        m_visibleObjects.erase(obj);
        PushOutOfRange(obj->GetNewGUID());
    }

    HEARTHSTONE_INLINE bool GetVisibility(Object* obj, InRangeSet::iterator *itr)
    {
        *itr = m_visibleObjects.find(obj);
        return ((*itr) != m_visibleObjects.end());
    }

    HEARTHSTONE_INLINE InRangeSet::iterator GetVisibleSetBegin() { return m_visibleObjects.begin(); }
    HEARTHSTONE_INLINE InRangeSet::iterator GetVisibleSetEnd() { return m_visibleObjects.end(); }

    // Misc
    void SetDrunk(uint16 value, uint32 itemId = 0);
    void EventHandleSobering();
    bool m_AllowAreaTriggerPort;
    void EventAllowTiggerPort(bool enable);
    float m_meleeattackspeedmod;
    float m_rangedattackspeedmod;
    uint32 m_modblockabsorbvalue;
    uint32 m_modblockvaluefromspells;
    void SendInitialLogonPackets();
    void Reset_Spells();
    void LearnTalent(uint32 talent_id, uint32 requested_rank);
    void Reset_Talents(bool all = false);
    void Reset_ToLevel1();
    void RetroactiveCompleteQuests();

    uint32 GetBGQueueSlotByBGType(uint32 type);
    uint32 HasBGQueueSlotOfType(uint32 type);

    // Battlegrounds xD
    CBattleground* m_bg;
    CBattleground* m_pendingBattleground[2];
    uint32 m_bgSlot;
    bool m_bgRatedQueue;
    uint32 m_bgEntryPointMap;
    float m_bgEntryPointX;
    float m_bgEntryPointY;
    float m_bgEntryPointZ;
    float m_bgEntryPointO;
    int32 m_bgEntryPointInstance;
    bool m_bgHasFlag;
    bool m_bgIsQueued[3];
    uint32 m_bgQueueType[2];
    uint32 m_bgQueueInstanceId[2];
    uint32 m_bgQueueTime[2];
    uint32 m_bgFlagIneligible;

    bool HasBattlegroundQueueSlot();

    uint32 GetBGQueueSlot();

    void EventRepeatSpell();
    void EventCastRepeatedSpell(uint32 spellid, Unit* target);
    int32 CanShootRangedWeapon(uint32 spellid, Unit* target, bool autoshot);
    uint32 m_AutoShotDuration;
    uint32 m_AutoShotAttackTimer;
    bool m_onAutoShot;
    uint64 m_AutoShotTarget;
    SpellEntry *m_AutoShotSpell;
    void _InitialReputation();
    void UpdateNearbyGameObjects();
    void UpdateNearbyQuestGivers();
    void EventMassSummonReset() { m_massSummonEnabled = false; }

    bool m_massSummonEnabled;

    uint32 m_moltenFuryDamageIncreasePct;

    void CalcResistance(uint32 type);
    HEARTHSTONE_INLINE float res_M_crit_get(){return m_resist_critical[0];}
    HEARTHSTONE_INLINE void res_M_crit_set(float newvalue){m_resist_critical[0]=newvalue;}
    HEARTHSTONE_INLINE float res_R_crit_get(){return m_resist_critical[1];}
    HEARTHSTONE_INLINE void res_R_crit_set(float newvalue){m_resist_critical[1]=newvalue;}
    uint32 FlatResistanceModifierPos[7];
    uint32 FlatResistanceModifierNeg[7];
    uint32 BaseResistanceModPctPos[7];
    uint32 BaseResistanceModPctNeg[7];
    uint32 ResistanceModPctPos[7];
    uint32 ResistanceModPctNeg[7];
    float m_resist_critical[2];//when we are a victim we can have talents to decrease chance for critical hit. This is a negative value and it's added to critchances
    float m_resist_hit[3]; // 0 = melee; 1= ranged; 2=spells
    uint32 m_modphyscritdmgPCT;
    uint32 m_RootedCritChanceBonus; // Class Script Override: Shatter
    uint32 m_Illumination_amount; // Class Script Override: Illumination
    float Damageshield_amount; // Damage Shield

    uint32 m_ModInterrMRegenPCT;
    int32 m_ModInterrMRegen;
    uint32 m_casted_amount[7]; //Last casted spells amounts. Need for some spells. Like Ignite etc. DOesn't count HoTs and DoTs. Only directs

    uint32 FlatStatModPos[5];
    uint32 FlatStatModNeg[5];
    uint32 StatModPctPos[5];
    uint32 StatModPctNeg[5];
    uint32 TotalStatModPctPos[5];
    uint32 TotalStatModPctNeg[5];
    int32 IncreaseDamageByType[12]; //mod dmg by creature type
    float IncreaseDamageByTypePCT[12];
    float IncreaseCricticalByTypePCT[12];
    int32 DetectedRange;
    float PctIgnoreRegenModifier;
    uint32 m_retainedrage;
    HEARTHSTONE_INLINE uint32* GetPlayedtime() { return m_playedtime; };
    void CalcStat(uint32 t);
    HEARTHSTONE_INLINE float CalcRating(uint32 index) { return CalcPercentForRating(index, m_uint32Values[index]); };
    float CalcPercentForRating(uint32 index, uint32 rating);
    void RecalcAllRatings();

    uint32 m_regenTimerCount;
    float m_powerFraction[6];
    void PlayerRegeneratePower(bool isinterrupted);
    void RegenerateHealth(bool inCombat);

    void BattlegroundKick() { if(m_bg) m_bg->RemovePlayer(this, false); };
    void ForceLogout(bool save) { GetSession()->LogoutPlayer(save); };

    uint32 SoulStone;
    uint32 SoulStoneReceiver;
    void removeSoulStone();

    HEARTHSTONE_INLINE uint32 GetSoulStoneReceiver(){return SoulStoneReceiver;}
    HEARTHSTONE_INLINE void SetSoulStoneReceiver(uint32 StoneGUID){SoulStoneReceiver = StoneGUID;}
    HEARTHSTONE_INLINE uint32 GetSoulStone(){return SoulStone;}
    HEARTHSTONE_INLINE void SetSoulStone(uint32 StoneID){SoulStone = StoneID;}

    bool bReincarnation;

    map<uint32, WeaponModifier> damagedone;
    map<uint32, WeaponModifier> tocritchance;
    uint32 Seal;
    int32 rageFromDamageDealt;
    // GameObject commands
    GameObject* m_GM_SelectedGO;

    void _Relocate(uint32 mapid,const LocationVector & v, bool sendpending, bool force_new_world, uint32 instance_id);
    void AddItemsToWorld();
    void RemoveItemsFromWorld();
    void UpdateKnownCurrencies(uint32 itemId, bool apply);
    uint32 GetTotalItemLevel();
    uint32 GetAverageItemLevel(bool skipmissing = false);

    uint32 m_ShapeShifted;
    uint32 m_MountSpellId;

    HEARTHSTONE_INLINE bool IsMounted() {return (m_MountSpellId!=0 ? true : false); }

    bool bGMTagOn;
    bool stack_cheat;
    bool DisableDevTag;
    bool vendorpass_cheat;
    bool triggerpass_cheat;
    bool ignoreitemreq_cheat; // Crow: Gonna do so much with this. HEHEHEHEHAHAHAHAHAHAHAHA!!!

    bool bHasBindDialogOpen;
    uint32 TrackingSpell;
    void _EventCharmAttack();
    void _Warn(const char *message);
    void _Kick();
    void _Disconnect();
    void Kick(uint32 delay = 0);
    void SoftDisconnect();
    uint32 m_KickDelay;
    Unit* m_CurrentCharm;
    Transporter* m_CurrentTransporter;
    bool IgnoreSpellFocusRequirements;

    Object* GetSummonedObject () {return m_SummonedObject;};
    void SetSummonedObject (Object* t_SummonedObject) {m_SummonedObject = t_SummonedObject;};
    uint32 roll;

    void ClearCooldownsOnLine(uint32 skill_line, uint32 called_from);
    void ClearCooldownsOnLines(set<uint32> skill_lines, uint32 called_from);
    void ResetAllCooldowns();
    void ClearCooldownForSpell(uint32 spell_id);
    PlayerCooldownMap GetCooldownMap(uint8 index = COOLDOWN_TYPE_SPELL) { return m_cooldownMap[index]; };
    bool SpellHasCooldown(uint32 spellid) { return (m_cooldownMap[COOLDOWN_TYPE_SPELL].find(spellid) != m_cooldownMap[COOLDOWN_TYPE_SPELL].end()); };

    bool sentMOTD;
    void sendMOTD();
    bool bProcessPending;
    Mutex _bufferS;
    void PushUpdateData(ByteBuffer *data, uint32 updatecount);
    void PushOutOfRange(const WoWGuid & guid);
    void ProcessPendingUpdates(ByteBuffer *pBuildBuffer, ByteBuffer *pCompressionBuffer);
    bool __fastcall CompressAndSendUpdateBuffer(uint32 size, const uint8* update_buffer, ByteBuffer *pCompressionBuffer);
    void ClearAllPendingUpdates();

    uint32 GetArmorProficiency() { return armor_proficiency; }
    uint32 GetWeaponProficiency() { return weapon_proficiency; }

    void ResetHeartbeatCoords();

    // speedhack buster!
    LocationVector                      m_lastHeartbeatPosition;
    uint32                              m_startMoveTime;    // time
    uint32                              m_lastMovementPacketTimestamp;
    int32                               m_heartbeatDisable;
    uint32                              m_lastMoveTime;
    bool                                m_speedChangeInProgress;
    uint32                              m_flyHackChances;

    bool CooldownCheat;
    bool CastTimeCheat;
    bool PowerCheat;
    bool FlyCheat;

    int32 m_lastAreaUpdateMap;
    uint32 m_oldZone, m_oldArea;

    // Phase stuff
    void _LoadAreaPhaseInfo(QueryResult *result);
    void _SaveAreaPhaseInfo(QueryBuffer* buff);
    void EnablePhase(int32 phaseMode, bool save = false);
    void DisablePhase(int32 phaseMode, bool save = false);
    void SetPhaseMask(int32 phase, bool save = false);
    int32 GetPhaseForArea(uint32 areaid);
    void SetPhaseForArea(uint32 areaid, int32 phase);
    map<uint32, AreaPhaseData*> areaphases; // Map<Areaid, AreaPhaseData>

    std::string Lfgcomment;
    uint16 LfgDungeonId[3];
    uint8 LfgType[3];
    uint16 LfmDungeonId;
    uint8 LfmType;
    uint32 roleflags;
    bool m_Autojoin;
    bool m_AutoAddMem;
    void StopMirrorTimer(uint32 Type);
    BGScore m_bgScore;
    uint32 m_bgTeam;
    void UpdateChanceFields();
    //Honor Variables
    time_t m_fallDisabledUntil;
    uint32 m_honorToday;
    uint32 m_honorYesterday;

    void RolloverHonor();
    uint32 m_honorPoints;
    uint32 m_honorRolloverTime;
    uint32 m_killsToday;
    uint32 m_killsYesterday;
    uint32 m_killsLifetime;
    uint32 m_arenaPoints;
    bool m_honorless;
    uint32 m_lastSeenWeather;
    unordered_set<Object* > m_visibleFarsightObjects;
    void EventTeleport(uint32 mapid, float x, float y, float z, float o, int32 phase = 1);
    void EventTeleport(uint32 mapid, float x, float y, float z, int32 phase = 1)
    {
        EventTeleport(mapid, x, y, z, 0.0f, phase);
    }
    void ApplyLevelInfo(uint32 Level);
    void BroadcastMessage(const char* Format, ...);
    map<uint32, set<uint32> > SummonSpells;
    map<uint32, PetSpellMap*> PetSpells;
    void AddSummonSpell(uint32 Entry, uint32 SpellID);
    void RemoveSummonSpell(uint32 Entry, uint32 SpellID);
    set<uint32>* GetSummonSpells(uint32 Entry);
    LockedQueue<WorldPacket*> delayedPackets;
    set<Player*  > gmTargets;
    uint32 m_UnderwaterMaxTime;
    HEARTHSTONE_INLINE void setMyCorpse(Corpse* corpse) { myCorpse = corpse; }
    HEARTHSTONE_INLINE Corpse* getMyCorpse() { return myCorpse; }

    uint32 m_resurrectHealth, m_resurrectMana;
    uint32 resurrector;
    LocationVector m_resurrectLoction;
    uint32 blinktimer;
    bool blinked;
    uint16 m_speedhackChances;
    uint16 m_cheatEngineChances;
    uint32 m_explorationTimer;

    // DBC stuff
    CharRaceEntry * myRace;
    CharClassEntry * myClass;

    Unit* linkTarget;
    bool SafeTeleport(uint32 MapID, uint32 InstanceID, float X, float Y, float Z, float O, int32 phase = 1);
    bool SafeTeleport(uint32 MapID, uint32 InstanceID, LocationVector vec, int32 phase = 1);
    void SafeTeleport(MapMgr* mgr, LocationVector vec, int32 phase = 1);
    void EjectFromInstance();
    bool raidgrouponlysent;

    void EventSafeTeleport(uint32 MapID, uint32 InstanceID, LocationVector vec, int32 phase = 1)
    {
        SafeTeleport(MapID, InstanceID, vec, phase);
    }

    /*****************
      PVP Stuff
    ******************/
    uint32 m_pvpTimer;

    HEARTHSTONE_INLINE void AddCoins( int32 coins ){
        ModUnsigned32Value( PLAYER_FIELD_COINAGE , coins );
    }
    HEARTHSTONE_INLINE void TakeCoins( int32 coins ){
        ModUnsigned32Value(PLAYER_FIELD_COINAGE, -coins);
    }

    //! Do this on /pvp off
    HEARTHSTONE_INLINE void ResetPvPTimer();
    //! Stop the timer for pvp off
    HEARTHSTONE_INLINE void StopPvPTimer() { RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TIMER); m_pvpTimer = 0; }

    //! Called at login to add the honorless buff, etc.
    void LoginPvPSetup();
    //! Update our pvp area (called when zone changes)
    void UpdatePvPArea();
    //! PvP Toggle (called on /pvp)
    void PvPToggle();

    HEARTHSTONE_INLINE uint32 LastHonorResetTime() const { return m_lastHonorResetTime; }
    HEARTHSTONE_INLINE void LastHonorResetTime(uint32 val) { m_lastHonorResetTime = val; }
    uint32 OnlineTime;
    LevelInfo * lvlinfo;
    void CalculateBaseStats();
    uint32 load_health;
    uint32 load_mana;
    set<SpellEntry *> castSpellAtLogin;

    /////
    void OnPushToWorld();
    void OnPrePushToWorld();
    void OnWorldPortAck();
    void OnWorldLogin();
    void CompleteLoading();
    void SendObjectUpdate(uint64 guid);

    uint32 m_TeleportState;
    set<Unit* > visiblityChangableSet;
    bool m_beingPushed;
    bool CanSignCharter(Charter * charter, Player* requester);
    uint32 m_FlyingAura;
    stringstream LoadAuras;
    bool resend_speed;
    bool rename_pending;
    bool m_XPoff;
    bool customizable;
    uint32 iInstanceType;
    uint32 iRaidType;
    uint32 AnnihilationProcChance;
    HEARTHSTONE_INLINE void SetName(string& name) { m_name = name; }
    // spell to (delay, last time)
    FactionReputation * reputationByListId[128];
    Channel* watchedchannel;

    uint64 m_comboTarget;
    int8 m_comboPoints;
    bool m_retainComboPoints;
    int8 m_spellcomboPoints; // rogue talent Ruthlessness will change combopoints while consuming them. solutions 1) add post cast prochandling, 2) delay adding the CP
    void UpdateComboPoints();

    HEARTHSTONE_INLINE void AddComboPoints(uint64 target, uint8 count)
    {
        if(m_comboTarget == target)
            m_comboPoints += count;
        else
        {
            m_comboTarget = target;
            m_comboPoints = count;
        }
        UpdateComboPoints();
    }

    HEARTHSTONE_INLINE void NullComboPoints()
    {
        if(!m_retainComboPoints)
        {
            m_comboTarget = m_comboPoints = m_spellcomboPoints = 0;
        }
        UpdateComboPoints();
        sEventMgr.RemoveEvents( this, EVENT_COMBO_POINT_CLEAR_FOR_TARGET );
    }

    uint32 m_speedChangeCounter;

    // HACKKKKK
    uint32 m_cheatDeathRank;

    void SendAreaTriggerMessage(const char * message, ...);

    // Trade Target
    //Player* getTradeTarget() {return mTradeTarget;};

    HEARTHSTONE_INLINE Player* GetTradeTarget()
    {
        if(!IsInWorld()) return NULLPLR;
        return m_mapMgr->GetPlayer((uint32)mTradeTarget);
    }

    Item* getTradeItem(uint32 slot) {return mTradeItems[slot];};

    // Water level related stuff (they are public because they need to be accessed fast)
    // Nose level of the character (needed for proper breathing)
    float m_noseLevel;

    /* Mind Control */
    void Possess(Unit* pTarget);
    void UnPossess();

    /* Last Speeds */
    HEARTHSTONE_INLINE void UpdateLastSpeeds()
    {
        m_lastRunSpeed = m_runSpeed;
        m_lastRunBackSpeed = m_backWalkSpeed;
        m_lastSwimSpeed = m_swimSpeed;
        m_lastRunBackSpeed = m_backSwimSpeed;
        m_lastFlySpeed = m_flySpeed;
    }

    void RemoteRevive(bool hp = true)
    {
        ResurrectPlayer();
        SetMovement(MOVE_UNROOT, 5);
        SetPlayerSpeed(RUN, (float)7);
        SetPlayerSpeed(SWIM, (float)4.9);
        SetMovement(MOVE_LAND_WALK, 8);
        if(hp)
            SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH) );
    }

    void ResetSpeedHack();
    void DelaySpeedHack(uint32 ms);

    float m_WeaponSubClassDamagePct[21];

    LocationVector m_last_group_position;
    int32 m_rap_mod_pct;
    void SummonRequest(Object* Requestor, uint32 ZoneID, uint32 MapID, uint32 InstanceID, const LocationVector & Position);
    uint8 m_lastMoveType;

    Creature* m_tempSummon;
    bool m_deathVision;
    SpellEntry * last_heal_spell;
    LocationVector m_sentTeleportPosition;

    void RemoveFromBattlegroundQueue(uint32 queueSlot, bool forced = false);
    void FullHPMP();
    uint32 m_arenateaminviteguid;

    /************************************************************************/
    /* Spell Packet wrapper Please keep this separated                      */
    /************************************************************************/
    void SendLevelupInfo(uint32 level, uint32 Hp, uint32 Mana, uint32 Stat0, uint32 Stat1, uint32 Stat2, uint32 Stat3, uint32 Stat4);
    void SendLogXPGain(uint64 guid, uint32 NormalXP, uint32 RestedXP, bool type);
    void SendEnvironmentalDamageLog(const uint64 & guid, uint8 type, uint32 damage);
    void SendWorldStateUpdate(uint32 WorldState, uint32 Value);
    void SendCastResult(uint32 SpellId, uint8 ErrorMessage, uint8 MultiCast, uint32 Extra);
    void Gossip_SendPOI(float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, const char* Name);
    /************************************************************************/
    /* End of SpellPacket wrapper                                           */
    /************************************************************************/

    Mailbox* m_mailBox;
    bool m_setwaterwalk;
    bool m_setflycheat;
    uint64 m_areaSpiritHealer_guid;
    bool m_finishingmovesdodge;

    HEARTHSTONE_INLINE bool IsAttacking() {return m_attacking; }

    static void InitVisibleUpdateBits();
    static UpdateMask m_visibleUpdateMask;

    void SendPacket(WorldPacket* data);
    void SendDelayedPacket(WorldPacket * data);
    void CopyAndSendDelayedPacket(WorldPacket * data);
    void PartLFGChannel();

    uint32 GetLastLoginTime() { return  m_timeLogoff; };

    //Current value of Feral Attack Power from items
    int32 m_feralAP;
    bool    hasqueuedpet;
    uint32 JudgementSpell;
    Mutex PetLocks;
    std::map<uint32, PlayerPet*> m_Pets;
    uint8 m_StableSlotCount;

    //Player Action Bar
    ActionButton mActions[2][PLAYER_ACTION_BUTTON_COUNT];

    ActionButton GetActionButton(uint32 button)
    {
        if(button > PLAYER_ACTION_BUTTON_COUNT)
            return mActions[0][0];
        return mActions[GetActiveSpec()][button];
    }

protected:
    uint32 m_timeLogoff;
    LocationVector m_summonPos;
    int32 m_summonInstanceId;
    uint32 m_summonMapId;
    Object* m_summoner;

    uint32 iActivePet;
    void _SetCreateBits(UpdateMask *updateMask, Player* target) const;
    void _SetUpdateBits(UpdateMask *updateMask, Player* target) const;

    /* Update system components */
    uint32 mUpdateDataCount;
    ByteBuffer bUpdateDataBuffer;
    uint32 mOutOfRangeIdCount;
    ByteBuffer mOutOfRangeIds;
    /* End update system */

    void _LoadTutorials(QueryResult * result);
    void _SaveTutorials(QueryBuffer * buf);

    void _SaveInventory(bool firstsave);

    void _SaveQuestLogEntry(QueryBuffer * buf);
    void _LoadQuestLogEntry(QueryResult * result);

    void _LoadSkills(QueryResult * result);
    void _SaveSkillsToDB(QueryBuffer * buf);

    void _LoadSpells(QueryResult * result);
    void _SaveSpellsToDB(QueryBuffer * buf);

    void _LoadTalents(QueryResult * result);
    void _SaveTalentsToDB(QueryBuffer * buf);

    void _LoadGlyphs(QueryResult * result);
    void _SaveGlyphsToDB(QueryBuffer * buf);

    void _LoadPet(QueryResult * result);

    void _LoadPetActionBar(QueryResult * result);
    void _LoadPetNo();
    void _LoadPetSpells(QueryResult * result);
    void _SavePet(QueryBuffer * buf);
    void _SavePetSpells(QueryBuffer * buf);
    void _ApplyItemMods( Item* item, int16 slot, bool apply, bool justdrokedown = false, bool skip_stat_apply = false );
    void _EventAttack( bool offhand );
    void _EventExploration();

    // Water level related stuff
    void SetNoseLevel();

    /************************************************************************/
    /* Trade                                                                */
    /************************************************************************/
    Item* mTradeItems[7];
    uint32 mTradeGold;
    uint32 mTradeTarget;
    uint32 mTradeStatus;
    uint32 m_tradeSequence;

    /************************************************************************/
    /* Player Class systems, info and misc things                           */
    /************************************************************************/
    PlayerCreateInfo *info;
    uint32      m_AttackMsgTimer;   // "too far away" and "wrong facing" timer
    bool        m_attacking;
    std::string m_name; // max 21 character name

    // Character Ban
    uint32      m_banned;
    string      m_banreason;
    Pet*        m_Summon;
    uint32      m_PetNumberMax;
    uint32      m_invitersGuid; // It is guild inviters guid, 0 when its not used

    // bind
    float m_bind_pos_x;
    float m_bind_pos_y;
    float m_bind_pos_z;
    uint32 m_bind_mapid;
    uint32 m_bind_zoneid;
    std::list<ItemSet> m_itemsets;
    //Duel
    uint32 m_duelCountdownTimer;
    uint8 m_duelStatus;
    uint8 m_duelState;
    // Played time
    uint32 m_playedtime[3];
    uint8 m_isResting;
    uint8 m_restState;
    uint32 m_restAmount;
    AreaTrigger* LastAreaTrigger;
    //combat mods
    float m_blockfromspell;
    float m_blockfromspellPCT;
    float m_critfromspell;
    float m_spellcritfromspell;
    float m_dodgefromspell;
    float m_parryfromspell;
    float m_hitfromspell;
    float m_hitfrommeleespell;
    //stats mods
    int32 m_healthfromspell;
    uint32 m_manafromspell;
    uint32 m_healthfromitems;
    uint32 m_manafromitems;

    uint32 armor_proficiency;
    uint32 weapon_proficiency;
    // Talents
    uint32 m_talentresettimes;
    // STATUS
    uint8 m_status;
    // guid of current selection
    uint64 m_curSelection;
    // Raid
    uint8 m_targetIcon;
    // Player Reputation
    ReputationMap m_reputation;
    // Pointer to this char's game client
    WorldSession *m_session;
    // Channels
    std::set<uint32> m_channels;
    std::map<uint32, Channel*> m_channelsbyDBCID;
    // Visible objects
    unordered_set<Object* > m_visibleObjects;
    // Groups/Raids
    uint32 m_GroupInviter;

    // Fishing related
    Object* m_SummonedObject;

    // other system
    Corpse* myCorpse;

    uint32 m_lastHonorResetTime;
    uint32 _fields[PLAYER_END];
    uint32 trigger_on_stun;         //bah, warrior talent but this will not get triggered on triggered spells if used on proc so i'm forced to used a special variable
    uint32 trigger_on_stun_chance;  //also using this for mage "Frostbite" talent
    int hearth_of_wild_pct;         //druid hearth of wild talent used on shapeshifting. We eighter know what is last talent level or memo on learn

    uint32 m_team;
    float m_lastRunSpeed;
    float m_lastRunBackSpeed;
    float m_lastSwimSpeed;
    float m_lastBackSwimSpeed;
    float m_lastFlySpeed;
    float m_lastBackFlySpeed;

    uint32 m_mountCheckTimer;
    void RemovePendingPlayer();

public:
    uint32 m_mallCheckTimer;
    uint32 m_UpdateHookTimer;

public:
    void SetLastRunSpeed(float value) { m_lastRunSpeed = value;}
    map<uint32, uint32> m_forcedReactions;

    uint32 m_speedhackCheckTimer;
    void _SpeedhackCheck();     // save a call to getMSTime() yes i am a stingy bastard

    bool m_passOnLoot;
    bool m_changingMaps;

    /************************************************************************/
    /* SOCIAL                                                               */
    /************************************************************************/
private:
    /* we may have multiple threads on this(chat) - burlex */
    Mutex m_socialLock;
    map<uint32, char*> m_friends;
    set<uint32> m_ignores;
    set<uint32> m_hasFriendList;

    void Social_SendFriendList(uint32 flag);

    void Social_AddFriend(const char * name, const char * note);
    void Social_RemoveFriend(uint32 guid);

    void Social_AddIgnore(const char * name);
    void Social_RemoveIgnore(uint32 guid);

    void Social_SetNote(uint32 guid, const char * note);

public:
    bool Social_IsIgnoring(PlayerInfo * m_info);
    bool Social_IsIgnoring(uint32 guid);

    void Social_TellOnlineStatus(bool online = true);
    void Social_TellFriendsStatus();

    /************************************************************************/
    /* end social                                                           */
    /************************************************************************/

    PlayerInfo * m_playerInfo;
    PlayerInfo * getPlayerInfo() const {return m_playerInfo;}
    uint32 m_skipCastCheck[3];  // spell group relation of spell types that should ignore some cancast checks
    uint32 m_castFilter[3];     // spell group relation of only spells that player can currently cast

    uint32 m_vampiricEmbrace;
    void VampiricSpell(uint32 dmg, Unit* pTarget, SpellEntry *spellinfo);

    // grounding totem
    Aura* m_magnetAura;

    // mage invisibility
    bool m_mageInvisibility;

    HEARTHSTONE_INLINE bool HasKnownTitleByEntry(uint32 entry) { CharTitlesEntry *titleEntry = dbcCharTitles.LookupEntry(entry); return (titleEntry && HasKnownTitleByIndex(titleEntry->index)); }
    HEARTHSTONE_INLINE bool HasKnownTitleByIndex(uint32 bitIndex) { return HasFlag((PLAYER__FIELD_KNOWN_TITLES+(bitIndex / 32)), uint32(uint32(1) << (bitIndex % 32))); }
    void SetKnownTitle( int32 title, bool set );

    // debuffs
    bool mWeakenedSoul;
    bool mForbearance;
    bool mHypothermia;
    bool mExhaustion;
    bool mSated;

    // Avenging Wrath...
    bool mAvengingWrath;
    void AvengingWrath() { mAvengingWrath = true; }
    // Talent Specs
    uint16 m_maxTalentPoints;
    uint16 GetMaxTalentPoints();
    void ApplySpec(uint8 spec, bool init);
    void ApplyTalent(uint32 spellId);
    void RemoveTalent(uint32 spellid);
    uint8 m_talentSpecsCount;
    uint8 m_talentActiveSpec;
    HEARTHSTONE_INLINE uint8 GetActiveSpec() { uint8 spec = m_talentActiveSpec; if(spec > 1) spec = 0; return spec; };
    struct PlayerSpec
    {
        std::map<uint32, uint8> talents;    // map of <talentId, talentRank>
        uint16  glyphs[GLYPHS_COUNT];
    };
    PlayerSpec m_specs[MAX_SPEC_COUNT];

    // Glyphs
    void UnapplyGlyph(uint32 slot);
    uint8 SetGlyph(uint32 slot, uint32 glyphId);
    void InitGlyphSlots();
    void InitGlyphsForLevel();

    // Equipment Sets
    void SendEquipmentSets();
    void SetEquipmentSet(uint32 index, EquipmentSet eqset);
    void DeleteEquipmentSet(uint64 setGuid);
    void _LoadEquipmentSets(QueryResult *result);
    void _SaveEquipmentSets(QueryBuffer* buff);

private:
    void SetTaximaskNode(uint32 nodeidx, bool UnSet = false);
public:
    void AddTaximaskNode(uint32 nodeidx){SetTaximaskNode(nodeidx, false);}
    void RemoveTaximaskNode(uint32 nodeidx){SetTaximaskNode(nodeidx, true);}

    uint8 GetChatTag() const;
    void AddArenaPoints( uint32 arenapoints );
    bool PreventRes;

    // loooooot
    void GenerateLoot(Corpse* pCorpse);
    uint32 GenerateShapeshiftModelId(uint32 form);
    PlayerCreateInfo * GetInfo() const { return info;};
    float MobXPGainRate;
    bool NoReagentCost;
    void StartQuest(uint32 id);
    bool fromrandombg;
    bool randombgwinner;
    //Calculate honor for random bgs
    uint32 GenerateRBGReward(uint32 level, uint32 count = 1) { uint32 honor = uint32(ceil(float(count * level * 1.55f))); return honor; }
    uint16 m_drunk;
    uint32 m_drunkTimer;
    static DrunkenState GetDrunkenstateByValue(uint16 value);
    void EventDrunkenVomit();

public:
    void ClearRuneCooldown(uint8 index);
    void ConvertRune(uint8 index, uint8 value);
    void ScheduleRuneRefresh(uint8 index, bool forceDeathRune = false);

    bool CanUseRunes(uint8 blood, uint8 frost, uint8 unholy);
    void UseRunes(uint8 blood, uint8 frost, uint8 unholy, SpellEntry * pSpell = NULL);
    uint8 TheoreticalUseRunes(uint8 blood, uint8 frost, uint8 unholy);

    uint8 GetRuneMask() { return m_runemask; }
    uint8 GetRune(uint32 index) { ASSERT(index < 6); return m_runes[index]; }
    uint8 GetBaseRune(uint8 index) { ASSERT(index < 6); return baseRunes[index]; }
    uint32 GetRuneCooldown(uint32 index) { return m_runeCD[index]; }
    void SetRune(uint8 index, uint8 value) { m_runes[index] = value; }
    void SetRuneCooldown(uint8 index, uint32 cooldown) { m_runeCD[index] = cooldown; }
    void SetDeathRuneChance(uint32 chance) { m_deathRuneMasteryChance = chance; }

private:
    // Runes
    uint8 m_runemask;
    uint8 m_runes[6];
    uint32 m_runeCD[6];

    uint32 m_deathRuneMasteryChance;

};

class SkillIterator
{
    SkillMap::iterator m_itr;
    SkillMap::iterator m_endItr;
    bool m_searchInProgress;
    Player* m_target;
public:
    SkillIterator(Player* target) : m_searchInProgress(false),m_target(target) {}
    ~SkillIterator() { if(m_searchInProgress) { EndSearch(); } if(m_target) { m_target = NULLPLR; } }

    void BeginSearch()
    {
        // iteminterface doesn't use mutexes, maybe it should :P
        ASSERT(!m_searchInProgress);
        m_itr = m_target->m_skills.begin();
        m_endItr = m_target->m_skills.end();
        m_searchInProgress=true;
    }

    void EndSearch()
    {
        // nothing here either
        ASSERT(m_searchInProgress);
        m_searchInProgress=false;
    }

    PlayerSkill* operator*() const
    {
        return &m_itr->second;
    }

    PlayerSkill* operator->() const
    {
        return &m_itr->second;
    }

    void Increment()
    {
        if(!m_searchInProgress)
            BeginSearch();

        if(m_itr==m_endItr)
            return;

        ++m_itr;
    }

    HEARTHSTONE_INLINE PlayerSkill* Grab() { return &m_itr->second; }
    HEARTHSTONE_INLINE bool End() { return (m_itr==m_endItr)?true:false; }
};
