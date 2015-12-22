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
struct PlayerCreateInfo;

#define MAX_PET_NO 3
#define PLAYER_HONORLESS_TARGET_SPELL 2479

#define ALLIANCE 0
#define HORDE 1

// gold cap
#define PLAYER_MAX_GOLD 0x7FFFFFFF

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
    CLASS_MAX = 12
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
    CLASSMASK_DRUID         = 0x0400, // 1024
    CLASSMASK_ALL_PLAYABLE   = CLASSMASK_WARRIOR | CLASSMASK_PALADIN | CLASSMASK_HUNTER | CLASSMASK_ROGUE | CLASSMASK_PRIEST | CLASSMASK_DEATHKNIGHT
    | CLASSMASK_SHAMAN | CLASSMASK_MAGE | CLASSMASK_WARLOCK | CLASSMASK_DRUID
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
    RACE_GOBLIN = 9,
    RACE_BLOODELF = 10,
    RACE_DRAENEI = 11,
    RACE_WORGEN = 22,
    RACEMASK_ALL_PLAYABLE = ((1<<(RACE_HUMAN-1))  |(1<<(RACE_ORC-1))   |(1<<(RACE_DWARF-1))   | \
                            (1<<(RACE_NIGHTELF-1))|(1<<(RACE_UNDEAD-1))|(1<<(RACE_TAUREN-1))  | \
                            (1<<(RACE_GNOME-1))   |(1<<(RACE_TROLL-1)) |(1<<(RACE_BLOODELF-1))| \
                            (1<<(RACE_DRAENEI-1)) |(1<<(RACE_GOBLIN-1))|(1<<(RACE_WORGEN-1)))

};

static const uint32 TalentTreesPerClass[DRUID+1][3] =  {
    { 409, 410, 411 },  // PETS
    { 746, 815, 845 },  // WARRIOR
    { 831, 839, 855 },  // PALADIN
    { 811, 807, 809 },  // HUNTER
    { 182, 181, 183 },  // ROGUE
    { 760, 813, 795 },  // PRIEST
    { 398, 399, 400 },  // DEATH KNIGHT
    { 261, 263, 262 },  // SHAMAN
    { 799, 851, 823 },  // MAGE
    { 871, 867, 865 },  // WARLOCK
    { 0, 0, 0 },        // NONE
    { 752, 750, 748 },  // DRUID
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
    LOOTTYPE_CORPSE                 = 1,
    LOOTTYPE_PICKPOCKETING          = 2,
    LOOTTYPE_FISHING                = 3,
    LOOTTYPE_DISENCHANTING          = 4,

    LOOTTYPE_SKINNING               = 6,
    LOOTTYPE_PROSPECTING            = 7,
    LOOTTYPE_MILLING                = 8,
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

struct CreateInfo_ItemStruct
{
    uint32  protoid;
    uint32  amount;
};

struct CreateInfo_ActionBarStruct
{
    uint8  button;
    uint32  action;
    uint8  type;
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
    RONIN_INLINE int32 CalcStanding() { return standing - baseStanding; }
    RONIN_INLINE bool Positive() { return standing >= 0; }
};

struct PlayerInfo
{
    PlayerInfo(WoWGuid _guid);
    ~PlayerInfo();
    WoWGuid charGuid;
    uint32 accountId;
    std::string charName;
    uint8 charRace;
    uint8 charClass;
    uint8 charGender;
    uint32 charTeam;

    // Location information
    uint32 lastMapID;
    uint32 lastInstanceID;
    float lastPositionX;
    float lastPositionY;
    float lastPositionZ;
    float lastOrientation;
    uint32 lastZone;
    uint32 lastLevel;
    time_t lastOnline;

    // Misc data
    uint32 achievementPoints;
    uint32 professionId[2], professionSkill[2], professionRank[2];

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
    std::string name;
    uint32 entry;
    std::string fields;
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
class PlayerInventory;
class GossipMenu;

#define RESTSTATE_RESTED             1
#define RESTSTATE_NORMAL             2
#define RESTSTATE_TIRED100           3
#define RESTSTATE_TIRED50            4
#define RESTSTATE_EXHAUSTED          5

enum PlayerTradeStatus : uint8
{
    TRADE_STATUS_OPEN_WINDOW = 0,
    TRADE_STATUS_NOT_ON_TAPLIST,
    TRADE_STATUS_SELF_CANCEL,
    TRADE_STATUS_TARGET_IGNORING_YOU,
    TRADE_STATUS_TARGET_IS_DEAD,
    TRADE_STATUS_TRADE_ACCEPTED,
    TRADE_STATUS_TARGET_LOGGING_OUT,
    TRADE_STATUS_UNK1,
    TRADE_STATUS_TRADE_COMPLETED,
    TRADE_STATUS_TARGET_TRIAL_ACCOUNT,
    TRADE_STATUS_UNK2,
    TRADE_STATUS_BEGIN_TRADE,
    TRADE_STATUS_YOU_ARE_DEAD,
    TRADE_STATUS_UNK3,
    TRADE_STATUS_UNK4,
    TRADE_STATUS_TARGET_TOO_FAR,
    TRADE_STATUS_NO_TARGET,
    TRADE_STATUS_TARGET_IS_BUSY,
    TRADE_STATUS_CURRENCY_IS_BOUND,
    TRADE_STATUS_TARGET_WRONG_FACTION,
    TRADE_STATUS_TARGET_IS_BUSY_2,
    TRADE_STATUS_UNK5,
    TRADE_STATUS_TRADE_CANCELLED,
    TRADE_STATUS_TRADING_CURRENCY,
    TRADE_STATUS_BACK_TO_TRADE,
    TRADE_STATUS_ONLY_CONJURABLE_CROSSREALM,
    TRADE_STATUS_YOU_ARE_STUNNED,
    TRADE_STATUS_UNK6,
    TRADE_STATUS_TARGET_IS_STUNNED,
    TRADE_STATUS_UNK7,
    TRADE_STATUS_CLOSE_WINDOW
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

#define MAX_EQUIPMENT_SET_INDEX 10  // client limit

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

#define MAX_PLAYER_SKILLS 128

struct PlayerSkill
{
    SkillLineEntry *Skill;
    uint8 SkillPos;
    uint16 CurrentValue;
    uint16 MaximumValue;
    uint16 BonusValue;
    uint16 BonusTalent;
    float GetSkillUpChance();
};

class ArenaTeam;
struct PlayerCooldown
{
    uint32 SpellId;
    uint32 ItemId;
    time_t ExpireTime;
};

enum ItemBonusModSlot
{
    MOD_SLOT_STAT_1 = 0,
    MOD_SLOT_STAT_2,
    MOD_SLOT_STAT_3,
    MOD_SLOT_STAT_4,
    MOD_SLOT_STAT_5,
    MOD_SLOT_STAT_6,
    MOD_SLOT_STAT_7,
    MOD_SLOT_STAT_8,
    MOD_SLOT_STAT_9,
    MOD_SLOT_STAT_10,
    MOD_SLOT_ARMOR,
    MOD_SLOT_MINDAMAGE,
    MOD_SLOT_MAXDAMAGE,
    MOD_SLOT_WEAPONDELAY,
    MOD_SLOT_MAX_STAT = 14,

    // Define this here ahead of time, as these are based on item enchant slots
    MOD_SLOT_MAX_ENCHANT = MOD_SLOT_MAX_STAT+MAX_ENCHANTMENT_SLOT,

    // Start our item enchantment mod slots
    MOD_SLOT_PERM_ENCHANT = 14,
    MOD_SLOT_TEMP_ENCHANT,
    MOD_SLOT_SOCKET_ENCHANT_1,
    MOD_SLOT_SOCKET_ENCHANT_2,
    MOD_SLOT_SOCKET_ENCHANT_3,
    MOD_SLOT_BONUS_ENCHANT,
    MOD_SLOT_PRISMATIC_ENCHANT,
    MOD_SLOT_UNKNOWN_ENCHANT,
    MOD_SLOT_REFORGE_ENCHANT,
    MOD_SLOT_TRANSMOG_ENCHANT,
    MOD_SLOT_PROPRETY_ENCHANT_0,
    MOD_SLOT_PROPRETY_ENCHANT_1,
    MOD_SLOT_PROPRETY_ENCHANT_2,
    MOD_SLOT_PROPRETY_ENCHANT_3,
    MOD_SLOT_PROPRETY_ENCHANT_4,
};

typedef std::map<std::pair<uint64, uint32>, std::pair<uint32, int32>> ItemBonusModMap;
typedef std::map<uint32, ItemBonusModMap> ItemBonusModByType;

enum PlayerLoadOrder : uint8
{
    PLAYER_LO_DATA = 0,
    PLAYER_LO_ACTIONS,
    PLAYER_LO_AURAS,
    PLAYER_LO_BANS,
    PLAYER_LO_COOLDOWNS,
    PLAYER_LO_CRITERIA_DATA,
    PLAYER_LO_EQUIPMENTSETS,
    PLAYER_LO_EXPLORATION,
    PLAYER_LO_FACTIONS,
    PLAYER_LO_GLYPHS,
    PLAYER_LO_ITEMS,
    PLAYER_LO_KNOWN_TITLES,
    PLAYER_LO_POWERS,
    PLAYER_LO_QUEST_LOG,
    PLAYER_LO_QUESTS_COMPLETED,
    PLAYER_LO_SKILLS,
    PLAYER_LO_SOCIAL,
    PLAYER_LO_SPELLS,
    PLAYER_LO_TALENTS,
    PLAYER_LO_TAXIMASKS,
    PLAYER_LO_TIMESTAMPS
};

enum PlayerLoadFields
{
    PLAYERLOAD_FIELD_LOW_GUID = 0,
    PLAYERLOAD_FIELD_ACCOUNT_ID,
    PLAYERLOAD_FIELD_NAME,
    PLAYERLOAD_FIELD_RACE,
    PLAYERLOAD_FIELD_CLASS,
    PLAYERLOAD_FIELD_GENDER,
    PLAYERLOAD_FIELD_LEVEL,
    PLAYERLOAD_FIELD_EXPERIENCE,
    PLAYERLOAD_FIELD_SELECTED_TITLE,
    PLAYERLOAD_FIELD_GOLD,
    PLAYERLOAD_FIELD_AVAILABLE_PROF_POINTS,
    PLAYERLOAD_FIELD_WATCHED_FACTION_INDEX,
    PLAYERLOAD_FIELD_CURRENTHP,
    PLAYERLOAD_FIELD_CURRENT_PVPRANK,
    PLAYERLOAD_FIELD_PLAYERBYTES,
    PLAYERLOAD_FIELD_PLAYERBYTES2,
    PLAYERLOAD_FIELD_PLAYERBYTES3,
    PLAYERLOAD_FIELD_PLAYER_BYTES,
    PLAYERLOAD_FIELD_PLAYER_BYTES2,
    PLAYERLOAD_FIELD_MAPID,
    PLAYERLOAD_FIELD_POSITION_X,
    PLAYERLOAD_FIELD_POSITION_Y,
    PLAYERLOAD_FIELD_POSITION_Z,
    PLAYERLOAD_FIELD_POSITION_O,
    PLAYERLOAD_FIELD_INSTANCE_ID,
    PLAYERLOAD_FIELD_ZONEID,
    PLAYERLOAD_FIELD_ONLINESTATUS,
    PLAYERLOAD_FIELD_BINDMAPID,
    PLAYERLOAD_FIELD_BINDPOSITION_X,
    PLAYERLOAD_FIELD_BINDPOSITION_Y,
    PLAYERLOAD_FIELD_BINDPOSITION_Z,
    PLAYERLOAD_FIELD_BINDZONEID,
    PLAYERLOAD_FIELD_ISRESTING,
    PLAYERLOAD_FIELD_RESTSTATE,
    PLAYERLOAD_FIELD_RESTTIME,
    PLAYERLOAD_FIELD_DEATHSTATE,
    PLAYERLOAD_FIELD_TOTALSTABLESLOTS,
    PLAYERLOAD_FIELD_ENTRYPOINT_MAP,
    PLAYERLOAD_FIELD_ENTRYPOINT_X,
    PLAYERLOAD_FIELD_ENTRYPOINT_Y,
    PLAYERLOAD_FIELD_ENTRYPOINT_Z,
    PLAYERLOAD_FIELD_ENTRYPOINT_O,
    PLAYERLOAD_FIELD_TAXI_PATH,
    PLAYERLOAD_FIELD_TAXI_LASTNODE,
    PLAYERLOAD_FIELD_TAXI_MOUNTID,
    PLAYERLOAD_FIELD_TRANSPORTERGUID,
    PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_X,
    PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_Y,
    PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_Z,
    PLAYERLOAD_FIELD_INSTANCE_DIFFICULTY,
    PLAYERLOAD_FIELD_RAID_DIFFICULTY,
    PLAYERLOAD_FIELD_ACTIVE_SPEC,
    PLAYERLOAD_FIELD_SPEC_COUNT,
    PLAYERLOAD_FIELD_TALENT_RESET_COUNTER,
    PLAYERLOAD_FIELD_BONUS_TALENT_POINTS,
    PLAYERLOAD_FIELD_ACTIVE_TALENT_SPECSTACK,
    PLAYERLOAD_FIELD_NEEDS_POSITION_RESET,
    PLAYERLOAD_FIELD_NEEDS_TALENT_RESET
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
typedef std::map<SpellEntry*, std::pair<uint32, uint32> >StrikeSpellMap;
typedef std::map<uint32, OnHitSpell >               StrikeSpellDmgMap;
typedef std::unordered_map<uint32, PlayerSkill>     SkillMap;
typedef std::map<uint8, PlayerSkill>                SkillReferenceMap;
typedef std::set<Player* *>                         ReferenceSet;
typedef std::map<uint32, PlayerCooldown>            PlayerCooldownMap;

class SERVER_DECL Player : public Unit
{
    friend class WorldSession;
    friend class Pet;
    friend class SkillIterator;

public:
    Player ( uint64 guid, uint32 fieldCount = PLAYER_END );
    ~Player ( );
    virtual void Init();
    virtual void Destruct();

    void Update( uint32 time );

    void ProcessPendingItemUpdates();
    std::set<Item*> m_pendingUpdates;
    void ItemFieldUpdated(Item *item);
    void ItemDestructed(Item *item);

    void OnFieldUpdated(uint32 index);
    /************************************************************************/
    /* Update fields System                                                 */
    /************************************************************************/
    void UpdateFieldValues();
    void UpdatePlayerRatings();
    void ClearFieldUpdateValues() {}; // Players clear field update values manually

    bool StatUpdateRequired();
    bool HealthUpdateRequired();
    bool PowerUpdateRequired();
    bool AttackTimeUpdateRequired(uint8 weaponType);
    bool APUpdateRequired();
    bool RAPUpdateRequired();
    bool ResUpdateRequired();
    bool CombatRatingUpdateRequired(uint32 combatRating);

    float GetPowerMod() { return 1.f; }
    float GetHealthMod() { return 1.f; }
    int32 GetBonusMana();
    int32 GetBonusHealth();
    int32 GetBonusStat(uint8 type);
    int32 GetBaseAttackTime(uint8 weaponType);
    int32 GetBaseMinDamage(uint8 weaponType);
    int32 GetBaseMaxDamage(uint8 weaponType);
    int32 GetBonusAttackPower();
    int32 GetBonusRangedAttackPower();
    int32 GetBonusResistance(uint8 school);

    float GetRatioForCombatRating(uint32 combatRating);
    int32 CalculatePlayerCombatRating(uint8 combatRating);

private:
    bool m_forceStatUpdate;

public:
    virtual void setLevel(uint32 level);

    /************************************************************************/
    /* Player loading and savings                                           */
    /* Serialize character to db                                            */
    /************************************************************************/
    void SaveToDB(bool bNewCharacter);
    static void DeleteFromDB(WoWGuid guid);

    bool LoadFromDB();
    void LoadFromDBProc(QueryResultVector & results);

    void _LoadPlayerAuras(QueryResult * result);
    void _SavePlayerAuras(QueryBuffer * buf);

    void _LoadPlayerCooldowns(QueryResult * result);
    void _SavePlayerCooldowns(QueryBuffer * buf);

    void _LoadEquipmentSets(QueryResult * result);
    void _SaveEquipmentSets(QueryBuffer * buf);

    void _LoadExplorationData(QueryResult * result);
    void _SaveExplorationData(QueryBuffer * buf);

    void _LoadKnownTitles(QueryResult * result);
    void _SaveKnownTitles(QueryBuffer * buf);

    void _LoadPlayerPowers(QueryResult * result);
    void _SavePlayerPowers(QueryBuffer * buf);

    void _LoadPlayerQuestLog(QueryResult * result);
    void _SavePlayerQuestLog(QueryBuffer * buf);

    void _LoadCompletedQuests(QueryResult * result);
    void _SaveCompletedQuests(QueryBuffer * buf);

    void _LoadSkills(QueryResult * result);
    void _SaveSkills(QueryBuffer * buf);

    void _LoadSocial(QueryResult * result);
    void _SaveSocial(QueryBuffer * buf);

    void _LoadSpells(QueryResult * result);
    void _SaveSpells(QueryBuffer * buf);

    void _LoadTaxiMasks(QueryResult * result);
    void _SaveTaxiMasks(QueryBuffer * buf);

    void _LoadTimeStampData(QueryResult * result);
    void _SaveTimeStampData(QueryBuffer * buf);

    /************************************************************************/
    /* Talent System                                                        */
    /************************************************************************/
    bool LearnTalent(uint32 talent_id, uint8 requested_rank) { return m_talentInterface.LearnTalent(talent_id, requested_rank); }
    bool HasTalent(uint32 talentid, int8 rank = -1) { return m_talentInterface.HasTalent(talentid, rank); }
    void ResetSpec(uint8 spec) { m_talentInterface.ResetSpec(spec, false); }

    uint32 CalcTalentResetCost(uint32 resetnum);
    void SendTalentResetConfirm();

    TalentInterface m_talentInterface;

    /************************************************************************/
    /* Skill System                                                         */
    /************************************************************************/

    void _AdvanceSkillLine(uint16 SkillLine, uint16 Count = 1);
    void _AddSkillLine(uint16 SkillLine, uint16 Current, uint16 Max);
    uint16 _GetSkillLineMax(uint16 SkillLine);
    uint16 _GetSkillLineCurrent(uint16 SkillLine, bool IncludeBonus = true);
    void _RemoveSkillLine(uint16 SkillLine);
    void _UpdateMaxSkillCounts();
    void _ModifySkillBonus(uint16 SkillLine, int16 Delta);
    void _ModifySkillBonusByType(uint16 SkillType, int16 Delta);
    bool _HasSkillLine(uint16 SkillLine);
    void RemoveSpellsFromLine(uint16 skill_line);
    void _RemoveAllSkills();
    void _RemoveLanguages();
    void _AddLanguages(bool All);
    void _AdvanceAllSkills(uint16 count, bool skipprof = false, uint16 max = 0);
    void _ModifySkillMaximum(uint16 SkillLine, uint16 NewMax);

    void RecalculateHonor();

    LfgMatch * m_lfgMatch;
    uint32 m_lfgInviterGuid;

    void EventTimeoutLfgInviter();

    static bool IsValidGender(uint8 Gender) { return Gender <= 1; }
    static bool IsValidClass(uint8 Class) { return ((1 << (Class - 1)) & CLASSMASK_ALL_PLAYABLE) != 0; }
    static bool IsValidRace(uint8 Race) { return ((1 << (Race - 1)) & RACEMASK_ALL_PLAYABLE) != 0; }

public:
    void EventDBCChatUpdate(uint32 dbcID);
protected:
    bool AllowChannelAtLocation(uint32 dbcID, AreaTableEntry *areaTable);
    bool UpdateChatChannel(const char* areaName, AreaTableEntry *areaTable, ChatChannelEntry* entry, Channel* channel);

protected:

    uint8 GetFreeSkillPosition();
    void _UpdateSkillFields();

    SkillMap m_skills;
    SkillReferenceMap m_skillsByIndex;

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
    // END COOLDOWNS

public:

    bool ok_to_remove;

    void RemoveFromWorld();
    bool Create ( WorldPacket &data );

    void BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag);
    std::string m_afk_reason;
    void SetAFKReason(std::string reason) { m_afk_reason = reason; };
    RONIN_INLINE const char* GetName() { return m_name.c_str(); }
    RONIN_INLINE std::string* GetNameString() { return &m_name; }

    //void KilledMonster(uint32 entry, const uint64 &guid);
    void GiveXP(uint32 xp, const uint64 &guid, bool allowbonus);   // to stop rest xp being given
    void ModifyBonuses(bool apply, uint64 guid, uint32 slot, uint32 type, int32 val);

    int32 GetBonusesFromItems(uint32 statType);

    UpdateMask itemBonusMask;
    ItemBonusModMap itemBonusMap;
    ItemBonusModByType itemBonusMapByType;

    /************************************************************************/
    /* Taxi                                                                 */
    /************************************************************************/
    RONIN_INLINE TaxiPath*    GetTaxiPath() { return m_CurrentTaxiPath; }
    RONIN_INLINE bool         GetTaxiState() { return m_onTaxi; }
    UpdateMask*         GetTaximask() { return &m_taxiMask; }
    void                TaxiStart(TaxiPath* path, uint32 modelid, uint32 start_node);
    void                JumpToEndTaxiNode(TaxiPath * path);
    void                EventDismount(uint32 money, float x, float y, float z);
    void                EventTaxiInterpolate();
    void                InitTaxiNodes();

    RONIN_INLINE void         SetTaxiState    (bool state) { m_onTaxi = state; }
    RONIN_INLINE bool         HasTaxiNode(uint32 node) { return m_taxiMask.GetBit(node); }
    RONIN_INLINE void         AddTaxiMask     (uint32 index) { m_taxiMask.SetBit(index); }
    RONIN_INLINE void         RemoveTaxiMask  (uint32 index) { m_taxiMask.UnsetBit(index); }
    RONIN_INLINE void         SetTaxiPath     (TaxiPath *path) { m_CurrentTaxiPath = path; }
    RONIN_INLINE void         SetTaxiPos()    {m_taxi_pos_x = m_position.x; m_taxi_pos_y = m_position.y; m_taxi_pos_z = m_position.z;}
    RONIN_INLINE void         UnSetTaxiPos()  {m_taxi_pos_x = 0; m_taxi_pos_y = 0; m_taxi_pos_z = 0; }

    // Taxi related variables
    std::vector<TaxiPath*>   m_taxiPaths;
    TaxiPath*           m_CurrentTaxiPath;
    uint32              taxi_model_id;
    uint32              lastNode;
    uint32              m_taxi_ride_time;
    UpdateMask          m_taxiMask;
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
    RONIN_INLINE  QuestLogEntry*GetQuestLogInSlot(uint32 slot){ return m_questlog[slot]; }
    RONIN_INLINE  uint32 GetQuestSharer(){ return m_questSharer; }

    RONIN_INLINE void SetQuestSharer(uint32 guid){ m_questSharer = guid; }
    void                SetQuestLogSlot(QuestLogEntry *entry, uint32 slot);

    RONIN_INLINE uint32 GetFinishedDailiesCount() { return (uint32)m_completedDailyQuests.size(); }
    void                AddToCompletedQuests(uint32 quest_id);
    void                AddToCompletedDailyQuests(uint32 quest_id);

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
    uint32 GetQuestSlotQuestId(uint16 slot) const { return GetUInt32Value(PLAYER_QUEST_LOG + slot * 5 + (uint32)NULL); }

    //Quest related variables
    QuestLogEntry *m_questlog[QUEST_LOG_COUNT];
    std::set<uint32>    m_QuestGOInProgress;
    uint32              m_questSharer;
    std::set<uint32>    quest_spells;
    std::set<uint32>    quest_mobs;

    Mutex DailyMutex;
    std::map<uint32, time_t> m_completedQuests, m_completedDailyQuests;

    void EventPortToGM(uint32 guid);
    RONIN_INLINE uint8 GetTeam() { return m_team; }
    RONIN_INLINE void SetTeam(uint8 t) { m_team = t; m_bgTeam=t; }
    RONIN_INLINE void ResetTeam() { m_team = myRace->TeamId; m_bgTeam=m_team; }
    RONIN_INLINE uint8 GetBGTeam() { return m_bgTeam; }
    RONIN_INLINE void SetBGTeam(uint8 t) { m_bgTeam = t; }

    uint32 GetMainMeleeDamage(uint32 AP_owerride); //i need this for windfury

    const WoWGuid& GetSelection( ) const { return m_curSelection; }
    void SetSelection(const uint64 &guid) { m_curSelection = guid; }

    /************************************************************************/
    /* Spells                                                               */
    /************************************************************************/
    bool HasSpell(uint32 spell);
    SpellEntry* GetSpellWithNamehash(uint32 namehash);
    bool HasHigherSpellForSkillLine(SpellEntry* sp);
    void smsg_InitialSpells();
    void addSpell(uint32 spell_idy);
    void removeSpellByNameHash(uint32 hash);
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
        m_onStrikeSpells.insert( std::map< SpellEntry*, std::pair<uint32, uint32> >::value_type( sp, std::make_pair( delay, 0 ) ) );
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
    SpellSet            mSpells;
    SpellSet            mShapeShiftSpells;

    void AddShapeShiftSpell(uint32 id);
    void RemoveShapeShiftSpell(uint32 id);

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
    void                Reputation_OnKilledUnit(Unit* pUnit, bool InnerLoop);
    void                Reputation_OnTalk(FactionEntry *faction);
    bool                AddNewFaction( FactionEntry *faction, int32 standing, bool base );
    void                OnModStanding( FactionEntry *faction, FactionReputation * rep );
    static Standing     GetReputationRankFromStanding(int32 Standing_);

    bool titanGrip;

    /************************************************************************/
    /* Factions                                                             */
    /************************************************************************/
    void smsg_InitialFactions();
    uint32 GetInitialFactionId();

    /************************************************************************/
    /* PVP                                                                  */
    /************************************************************************/
    RONIN_INLINE uint8 GetPVPRank()
    {
        return (uint8)((GetUInt32Value(PLAYER_BYTES_3) >> 24) & 0xFF);
    }
    RONIN_INLINE void SetPVPRank(int newrank)
    {
        SetUInt32Value(PLAYER_BYTES_3, ((GetUInt32Value(PLAYER_BYTES_3) & 0x00FFFFFF) | (uint8(newrank) << 24)));
    }
    uint32 GetMaxPersonalRating(bool Ignore2v2);


    /************************************************************************/
    /* Groups                                                               */
    /************************************************************************/
    void EventGroupFullUpdate();
    void GroupUninvite(Player* player, PlayerInfo *info);

    void ClearGroupInviter() { m_GroupInviter.Clean(); }
    void SetInviter(WoWGuid pInviter) { m_GroupInviter = pInviter; }
    RONIN_INLINE WoWGuid      GetInviter() { return m_GroupInviter; }
    RONIN_INLINE bool         InGroup() { return (m_playerInfo->m_Group != NULL && m_GroupInviter.empty()); }
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
    RONIN_INLINE int      HasBeenInvited() { return !m_GroupInviter.empty(); }
    RONIN_INLINE Group*   GetGroup() { return m_playerInfo != NULL ? m_playerInfo->m_Group : NULL; }
    RONIN_INLINE uint32   GetGroupID() { return m_playerInfo != NULL ? m_playerInfo->m_Group != NULL ? m_playerInfo->m_Group->GetID(): NULL: NULL; }
    RONIN_INLINE int8     GetSubGroup() { return m_playerInfo->subGroup; }
    RONIN_INLINE bool     IsBanned()
    {
        if(m_banned)
        {
            if(m_banned < 100 || (uint32)UNIXTIME < m_banned)
                return true;
        }
        return false;
    }

    RONIN_INLINE void SetBanned() { m_banned = 4;}
    RONIN_INLINE void SetBanned(std::string Reason) { m_banned = 4; m_banreason = Reason;}
    RONIN_INLINE void SetBanned(uint32 timestamp, std::string& Reason) { m_banned = timestamp; m_banreason = Reason; }
    RONIN_INLINE void UnSetBanned() { m_banned = 0; }
    RONIN_INLINE std::string GetBanReason() {return m_banreason;}

    void SetGuardHostileFlag(bool val) { if(val) SetFlag(PLAYER_FLAGS, PLAYER_FLAG_UNKNOWN2); else RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_UNKNOWN2); }
    void CreateResetGuardHostileFlagEvent()
    {
        event_RemoveEvents( EVENT_GUARD_HOSTILE );
        sEventMgr.AddEvent(this, &Player::SetGuardHostileFlag, false, EVENT_GUARD_HOSTILE, 10000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    uint32 m_hasInRangeGuards;

    /************************************************************************/
    /* Guilds                                                               */
    /************************************************************************/
    RONIN_INLINE bool IsInGuild() { return (GetUInt32Value(PLAYER_GUILDID) > 0); }
    RONIN_INLINE uint32 GetGuildId() { return GetUInt32Value(PLAYER_GUILDID); }
    RONIN_INLINE uint32 GetGuildRank() { return GetUInt32Value(PLAYER_GUILDRANK); }
    RONIN_INLINE uint32 GetGuildLevel() { return GetUInt32Value(PLAYER_GUILDLEVEL); }

    void SetGuildId(uint32 guildId);
    RONIN_INLINE void SetGuildRank(uint32 guildRank) { SetUInt32Value(PLAYER_GUILDRANK, guildRank); }
    RONIN_INLINE void SetGuildLevel(uint32 guildLevel) { SetUInt32Value(PLAYER_GUILDLEVEL, guildLevel); }

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
    RONIN_INLINE uint8        GetDuelStatus() { return m_duelStatus; }
    void                SetDuelState(uint8 state) { m_duelState = state; }
    RONIN_INLINE uint8        GetDuelState() { return m_duelState; }
    // duel variables
    Player*             DuelingWith;

    /************************************************************************/
    /* Trade                                                                */
    /************************************************************************/
    void SendTradeUpdate(bool extended, PlayerTradeStatus status, bool ourStatus = true, uint32 misc = 0, uint32 misc2 = 0);
    void ResetTradeVariables()
    {
        if(m_tradeData)
            delete m_tradeData;
        m_tradeData = NULL;
    }

    void CreateNewTrade(WoWGuid targetGuid)
    {
        ResetTradeVariables();
        m_tradeData = new Player::PlayerTradeData();
        m_tradeData->targetGuid = targetGuid;
        m_tradeData->enchantId = 0;
        m_tradeData->gold = 0;
        for(uint8 i = 0; i < 7; i++)
            m_tradeData->tradeItems[i] = NULL;
        m_tradeData->tradeStep = TRADE_STATUS_BEGIN_TRADE;
    }

    /************************************************************************/
    /* Pets                                                                 */
    /************************************************************************/
    RONIN_INLINE void SetSummon(Pet* pet) { m_Summon = pet; }
    RONIN_INLINE Pet* GetSummon(void) { return m_Summon; }
    uint32 GeneratePetNumber(void);
    void RemovePlayerPet(uint32 pet_number);
    RONIN_INLINE void AddPlayerPet(PlayerPet* pet, uint32 index) { m_Pets[index] = pet; }
    RONIN_INLINE PlayerPet* GetPlayerPet(uint8 idx)
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
    RONIN_INLINE PlayerPet* GetFirstPet(void) { return GetPlayerPet(GetFirstPetNumber()); }
    RONIN_INLINE void SetStableSlotCount(uint8 count) { m_StableSlotCount = count; }
    RONIN_INLINE uint8 GetStableSlotCount(void) { return m_StableSlotCount; }
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

public:
    /************************************************************************/
    /* Player Items                                                         */
    /************************************************************************/
    RONIN_INLINE void ApplyItemMods(Item* item, uint8 slot, bool apply, bool justdrokedown=false) {  _ApplyItemMods(item, slot, apply, justdrokedown); }

    // Pointer returns for player systems
    RONIN_INLINE PlayerInventory* GetInventory() { return &m_inventory; }
    RONIN_INLINE PlayerCurrency *GetCurrency() { return &m_currency; }

private:
    void _ApplyItemMods( Item* item, uint8 slot, bool apply, bool justdrokedown = false, bool skip_stat_apply = false );

    // item interface variables
    PlayerInventory m_inventory;

    //
    PlayerCurrency m_currency;

public:
    /************************************************************************/
    /* Loot                                                                 */
    /************************************************************************/
    RONIN_INLINE const WoWGuid& GetLootGUID() const { return m_lootGuid; }
    RONIN_INLINE void SetLootGUID(const WoWGuid &guid) { m_lootGuid = guid; }
    void SendLoot(WoWGuid guid, uint32 mapid, uint8 loot_type);
    // loot variables
    WoWGuid m_lootGuid;
    WoWGuid m_currentLoot;
    bool m_insigniaTaken;
    bool CanNeedItem(ItemPrototype* proto);

    /************************************************************************/
    /* World Session                                                        */
    /************************************************************************/

    RONIN_INLINE WorldSession* GetSession() const { return m_session; }
    void SetSession(WorldSession *s) { m_session = s; }
    void SetBindPoint(float x, float y, float z, uint32 m, uint32 v) { m_bind_pos_x = x; m_bind_pos_y = y; m_bind_pos_z = z; m_bind_mapid = m; m_bind_zoneid = v;}

    float GetSpellTimeMod(uint32 id);
    int GetSpellDamageMod(uint32 id);
    int32 GetSpellManaMod(uint32 id);

    // Talents
    // These functions build a specific type of A9 packet
    uint32 __fastcall BuildCreateUpdateBlockForPlayer( ByteBuffer *data, Player* target );
    virtual void DestroyForPlayer( Player* target, bool anim = false);

    std::deque<std::pair<uint8, Aura*>> m_loadAuras;

    std::set<uint32> OnMeleeAuras;
    void LoadNamesFromDB(uint32 guid);

    /************************************************************************/
    /* Death system                                                         */
    /************************************************************************/
    void SpawnCorpseBones();
    Corpse* CreateCorpse();
    void KillPlayer();
    void ResurrectPlayer(Unit* pResurrector = NULL);
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
    /* Channel stuff                                                        */
    /************************************************************************/
    void JoinedChannel(Channel *c);
    void LeftChannel(Channel *c);
    void CleanupChannels();
    //Attack stuff
    void EventDeath();
    //Note:ModSkillLine -> value+=amt;ModSkillMax -->value=amt; --wierd
    float GetSkillUpChance(uint32 id);
    float SpellCrtiticalStrikeRatingBonus;
    float CalculateCritFromAgilForClassAndLevel(uint32 _class, uint32 _level);
    float CalculateDefenseFromAgilForClassAndLevel(uint32 _class, uint32 _level);
    float SpellHasteRatingBonus;

    bool canCast(SpellEntry *m_spellInfo);
    void SendPetUntrainConfirm();
    void SendXPToggleConfirm();
    void SetPlayerStatus(uint8 pStatus) { m_status = pStatus; }
    void CheckPlayerStatus(uint8 pStatus) { if(m_status == pStatus) m_status = NONE; }
    RONIN_INLINE uint8 GetPlayerStatus() { return m_status; }
    const float& GetBindPositionX( ) const { return m_bind_pos_x; }
    const float& GetBindPositionY( ) const { return m_bind_pos_y; }
    const float& GetBindPositionZ( ) const { return m_bind_pos_z; }
    const uint32& GetBindMapId( ) const { return m_bind_mapid; }
    const uint32& GetBindZoneId( ) const { return m_bind_zoneid; }

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

    void SendMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, int32 Regen);
    void StopMirrorTimer(MirrorTimerType Type);
    void HandleBreathing(uint32 time_diff);
    int32 m_MirrorTimer[3];
    uint32 m_LastUnderwaterState;
    uint32 m_UnderwaterTime;

    // Beast
    bool m_BeastMaster;
    uint32 m_lastWarnCounter;

    // Visible objects
    bool CanSee(WorldObject* obj);
    RONIN_INLINE bool IsVisible(WorldObject* pObj) { return !(m_visibleObjects.find(pObj) == m_visibleObjects.end()); }
    void AddInRangeObject(WorldObject* pObj);
    void OnRemoveInRangeObject(WorldObject* pObj);
    void ClearInRangeSet();
    RONIN_INLINE void AddVisibleObject(WorldObject* pObj) { m_visibleObjects.insert(pObj); }
    RONIN_INLINE void RemoveVisibleObject(WorldObject* pObj) { m_visibleObjects.erase(pObj); }
    RONIN_INLINE void RemoveVisibleObject(InRangeWorldObjectSet::iterator itr) { m_visibleObjects.erase(itr); }
    RONIN_INLINE InRangeWorldObjectSet::iterator FindVisible(WorldObject* obj) { return m_visibleObjects.find(obj); }
    RONIN_INLINE void RemoveIfVisible(WorldObject* obj)
    {
        InRangeWorldObjectSet::iterator itr = m_visibleObjects.find(obj);
        if(itr == m_visibleObjects.end())
            return;

        m_visibleObjects.erase(obj);
        PushOutOfRange(obj->GetGUID());
    }

    RONIN_INLINE bool GetVisibility(WorldObject* obj, InRangeWorldObjectSet::iterator *itr)
    {
        *itr = m_visibleObjects.find(obj);
        return ((*itr) != m_visibleObjects.end());
    }

    RONIN_INLINE InRangeWorldObjectSet::iterator GetVisibleSetBegin() { return m_visibleObjects.begin(); }
    RONIN_INLINE InRangeWorldObjectSet::iterator GetVisibleSetEnd() { return m_visibleObjects.end(); }

    // Misc
    void SetDrunk(uint16 value, uint32 itemId = 0);
    void EventHandleSobering();
    float m_meleeattackspeedmod;
    float m_rangedattackspeedmod;

    void SendInitialLogonPackets();
    void Reset_Spells();
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

    RONIN_INLINE float CalcRating(uint32 index) { return CalcPercentForRating(index, GetUInt32Value(index)); };
    float CalcPercentForRating(uint32 index, uint32 rating);
    void RecalcAllRatings();

    uint32 m_regenTimerCount;
    float m_powerFraction[6];
    void PlayerRegeneratePower(bool isinterrupted);
    void RegenerateHealth(bool inCombat);

    void BattlegroundKick() { if(m_bg) m_bg->RemovePlayer(this, false); };
    void ForceLogout() { GetSession()->LogoutPlayer(); };

    uint32 SoulStone;
    uint32 SoulStoneReceiver;
    void removeSoulStone();

    RONIN_INLINE uint32 GetSoulStoneReceiver(){return SoulStoneReceiver;}
    RONIN_INLINE void SetSoulStoneReceiver(uint32 StoneGUID){SoulStoneReceiver = StoneGUID;}
    RONIN_INLINE uint32 GetSoulStone(){return SoulStone;}
    RONIN_INLINE void SetSoulStone(uint32 StoneID){SoulStone = StoneID;}

    bool bReincarnation;

    int32 rageFromDamageDealt;
    // GameObject commands
    GameObject* m_GM_SelectedGO;

    void _Relocate(uint32 mapid,const LocationVector & v, bool sendpending, bool force_new_world, uint32 instance_id);
    void UpdateKnownCurrencies(uint32 itemId, bool apply);
    uint32 GetTotalItemLevel();
    uint32 GetAverageItemLevel(bool skipmissing = false);

    uint32 m_ShapeShifted;
    uint32 m_MountSpellId;

    RONIN_INLINE bool IsMounted() {return (m_MountSpellId!=0 ? true : false); }

    bool bGMTagOn;
    bool stack_cheat;
    bool DisableDevTag;
    bool vendorpass_cheat;
    bool triggerpass_cheat;
    bool ignoreitemreq_cheat;

    bool bHasBindDialogOpen;
    uint32 TrackingSpell;
    void _Warn(const char *message);
    void _Kick();
    void _Disconnect();
    void Kick(uint32 delay = 0);
    void SoftDisconnect();
    uint32 m_KickDelay;
    Unit* m_CurrentCharm;
    Transporter* m_CurrentTransporter;
    bool IgnoreSpellFocusRequirements;

    GameObject* GetSummonedObject () {return m_SummonedObject;};
    void SetSummonedObject (GameObject* t_SummonedObject) {m_SummonedObject = t_SummonedObject;};
    uint32 roll;

    void ClearCooldownsOnLine(uint32 skill_line, uint32 called_from);
    void ClearCooldownsOnLines(std::set<uint32> skill_lines, uint32 called_from);
    void ResetAllCooldowns();
    void ClearCooldownForSpell(uint32 spell_id);
    PlayerCooldownMap GetCooldownMap(uint8 index = COOLDOWN_TYPE_SPELL) { return m_cooldownMap[index]; };
    bool SpellHasCooldown(uint32 spellid) { return (m_cooldownMap[COOLDOWN_TYPE_SPELL].find(spellid) != m_cooldownMap[COOLDOWN_TYPE_SPELL].end()); };

    void PushOutOfRange(WoWGuid guid);
    void PushUpdateBlock(ByteBuffer *data, uint32 updatecount);
    void PopPendingUpdates();

    void SendProficiency(bool armorProficiency);
    uint32 GetArmorProficiency() { return m_armorProficiency; }
    uint32 GetWeaponProficiency() { return m_weaponProficiency; }
    void AddArmorProficiency(uint32 flag) { m_armorProficiency |= flag; };
    void AddWeaponProficiency(uint32 flag) { m_weaponProficiency |= flag; };

    bool CooldownCheat;
    bool CastTimeCheat;
    bool PowerCheat;

    bool m_hasSentMoTD;
    int32 m_lastAreaUpdateMap;
    uint32 m_oldZone, m_oldArea;

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

    void EventTeleport(uint32 mapid, float x, float y, float z, float o = 0.f);
    void BroadcastMessage(const char* Format, ...);

    LockedQueue<WorldPacket*> delayedPackets;
    std::set<Player*  > gmTargets;

    RONIN_INLINE void setMyCorpse(Corpse* corpse) { myCorpse = corpse; }
    RONIN_INLINE Corpse* getMyCorpse() { return myCorpse; }

    uint32 m_resurrectHealth, m_resurrectMana;
    uint32 resurrector;
    LocationVector m_resurrectLoction;
    uint32 blinktimer;
    bool blinked;
    uint32 m_explorationTimer;

    // DBC stuff
    CharRaceEntry * myRace;
    CharClassEntry * myClass;

    Unit* linkTarget;
    bool SafeTeleport(uint32 MapID, uint32 InstanceID, float X, float Y, float Z, float O);
    bool SafeTeleport(uint32 MapID, uint32 InstanceID, LocationVector vec);
    void SafeTeleport(MapMgr* mgr, LocationVector vec);
    void EjectFromInstance();
    bool raidgrouponlysent;

    void EventSafeTeleport(uint32 MapID, uint32 InstanceID, LocationVector vec)
    {
        SafeTeleport(MapID, InstanceID, vec);
    }

    /*****************
      PVP Stuff
    ******************/
    uint32 m_pvpTimer;

    RONIN_INLINE void AddCoins( int32 coins ){
        ModUnsigned32Value( PLAYER_FIELD_COINAGE , coins );
    }
    RONIN_INLINE void TakeCoins( int32 coins ){
        ModUnsigned32Value(PLAYER_FIELD_COINAGE, -coins);
    }

    //! Do this on /pvp off
    RONIN_INLINE void ResetPvPTimer();
    //! Stop the timer for pvp off
    RONIN_INLINE void StopPvPTimer() { RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TIMER); m_pvpTimer = 0; }

    //! Called at login to add the honorless buff, etc.
    void LoginPvPSetup();
    //! Update our pvp area (called when zone changes)
    void UpdatePvPArea();
    //! PvP Toggle (called on /pvp)
    void PvPToggle();

    RONIN_INLINE uint32 LastHonorResetTime() const { return m_lastHonorResetTime; }
    RONIN_INLINE void LastHonorResetTime(uint32 val) { m_lastHonorResetTime = val; }
    uint32 OnlineTime;
    uint32 load_health, load_power[POWER_TYPE_MAX];
    std::set<SpellEntry *> castSpellAtLogin;

    /////
    void OnPushToWorld();
    void OnPrePushToWorld();
    void OnWorldPortAck();
    void OnWorldLogin();
    void SoftLoadPlayer();
    void CompleteLoading();
    void SendObjectUpdate(WoWGuid guid);

    uint32 m_TeleportState;
    std::set<Unit* > visiblityChangableSet;
    bool m_beingPushed;
    bool CanSignCharter(Charter * charter, Player* requester);
    uint32 m_FlyingAura;
    std::stringstream LoadAuras;
    bool m_XPoff;
    uint32 iInstanceType;
    uint32 iRaidType;
    uint32 AnnihilationProcChance;
    RONIN_INLINE void SetName(std::string& name) { m_name = name; }
    // spell to (delay, last time)
    FactionReputation * reputationByListId[256];
    Channel* watchedchannel;

    uint32 m_speedChangeCounter;

    // HACKKKKK
    uint32 m_cheatDeathRank;

    void SendAreaTriggerMessage(const char * message, ...);

    // Trade Target
    //Player* getTradeTarget() {return mTradeTarget;};

    RONIN_INLINE Player* GetTradeTarget()
    {
        if(!IsInWorld() || m_tradeData == NULL)
            return NULL;
        return m_mapMgr->GetPlayer(m_tradeData->targetGuid);
    }

    Item* getTradeItem(uint8 slot)
    {
        if(m_tradeData == NULL)
            return NULL;

        return m_tradeData->tradeItems[slot];
    }

    // Water level related stuff (they are public because they need to be accessed fast)
    // Nose level of the character (needed for proper breathing)
    float m_noseLevel;

    /* Mind Control */
    void Possess(Unit* pTarget);
    void UnPossess();

    void RemoteRevive(bool hp = true)
    {
        ResurrectPlayer();
        m_movementInterface.OnRevive();
        if(hp) SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH) );
    }

    LocationVector m_last_group_position;
    int32 m_rap_mod_pct;
    void SummonRequest(WorldObject* Requestor, uint32 ZoneID, uint32 MapID, uint32 InstanceID, const LocationVector & Position);

    Creature* m_tempSummon;
    bool m_deathVision;

    void RemoveFromBattlegroundQueue(uint32 queueSlot, bool forced = false);
    void FullHPMP();
    uint32 m_arenateaminviteguid;

    /************************************************************************/
    /* Spell Packet wrapper Please keep this separated                      */
    /************************************************************************/
    void SendLevelupInfo(uint32 level, uint32 Hp, uint32 Mana, uint32 *Stats);
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
    uint64 m_areaSpiritHealer_guid;
    bool m_finishingmovesdodge;

    void SendPacket(WorldPacket* data);
    void SendDelayedPacket(WorldPacket * data);
    void CopyAndSendDelayedPacket(WorldPacket * data);

    uint32 GetLastLoginTime() { return  m_timeLogoff; };

    //Current value of Feral Attack Power from items
    int32 m_feralAP;
    bool    hasqueuedpet;
    uint32 JudgementSpell;
    Mutex PetLocks;
    std::map<uint32, PlayerPet*> m_Pets;
    uint8 m_StableSlotCount;

protected:
    uint32 m_timeLogoff;
    LocationVector m_summonPos;
    int32 m_summonInstanceId;
    uint32 m_summonMapId;
    WorldObject* m_summoner;

    uint32 iActivePet;

    /* Update system components */
    Mutex _bufferS;
    bool bProcessPending;
    uint32 m_updateDataCount, m_OutOfRangeIdCount;
    ByteBuffer m_updateDataBuff, m_OutOfRangeIds;
    /* End update system */

    void _LoadPet(QueryResult * result);
    void _LoadPetActionBar(QueryResult * result);
    void _LoadPetNo();
    void _LoadPetSpells(QueryResult * result);
    void _SavePet(QueryBuffer * buf);

    void _EventExploration();

    // Water level related stuff
    void SetNoseLevel();

    /************************************************************************/
    /* Trade                                                                */
    /************************************************************************/
    struct PlayerTradeData
    {
        // Trade data
        uint64 gold;
        uint32 enchantId;
        WoWGuid targetGuid;
        Item* tradeItems[7];
        PlayerTradeStatus tradeStep;
    } *m_tradeData;

    /************************************************************************/
    /* Player Class systems, info and misc things                           */
    /************************************************************************/
    PlayerCreateInfo *info;
    std::string m_name; // max 21 character name

    // Character Ban
    uint32      m_banned;
    std::string m_banreason;
    Pet*        m_Summon;
    uint32      m_PetNumberMax;
    uint32      m_invitersGuid; // It is guild inviters guid, 0 when its not used

    // bind
    float m_bind_pos_x;
    float m_bind_pos_y;
    float m_bind_pos_z;
    uint32 m_bind_mapid;
    uint32 m_bind_zoneid;

    //Duel
    uint32 m_duelCountdownTimer;
    uint8 m_duelStatus;
    uint8 m_duelState;
    bool m_isResting;
    uint8 m_restState;
    uint32 m_restAmount;

    uint32 m_armorProficiency, m_weaponProficiency;
    // STATUS
    uint8 m_status;
    // guid of current selection
    WoWGuid m_curSelection;
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
    InRangeWorldObjectSet m_visibleObjects;
    // Groups/Raids
    WoWGuid m_GroupInviter;

    // Fishing related
    GameObject* m_SummonedObject;

    // other system
    Corpse* myCorpse;

    uint32 m_lastHonorResetTime;
    uint32 trigger_on_stun;         //bah, warrior talent but this will not get triggered on triggered spells if used on proc so i'm forced to used a special variable
    uint32 trigger_on_stun_chance;  //also using this for mage "Frostbite" talent

    uint8 m_team, m_bgTeam;

    uint32 m_mountCheckTimer;
    void RemovePendingPlayer(uint8 reason = CHAR_LOGIN_NO_CHARACTER);

public:
    std::map<uint32, uint32> m_forcedReactions;

    bool m_passOnLoot;
    bool m_changingMaps;

    /************************************************************************/
    /* SOCIAL                                                               */
    /************************************************************************/
private:
    /* we may have multiple threads on this(chat) - burlex */
    Mutex m_socialLock;
    std::map<WoWGuid, std::string> m_friends, m_ignores, m_mutes;

    void Social_SendFriendList(uint32 flag);

    void Social_AddFriend(std::string name, std::string note);
    void Social_RemoveFriend(WoWGuid guid);

    void Social_AddIgnore(std::string name);
    void Social_RemoveIgnore(WoWGuid guid);

    void Social_SetNote(WoWGuid guid, std::string name);

public:
    bool Social_IsIgnoring(WoWGuid guid);
    bool Social_IsIgnoring(PlayerInfo * info) { return Social_IsIgnoring(info->charGuid); }

    /************************************************************************/
    /* end social                                                           */
    /************************************************************************/

    PlayerInfo * m_playerInfo;
    PlayerInfo * getPlayerInfo() const {return m_playerInfo;}
    PlayerCreateInfo *getPlayerCreateInfo() const {return info; }

    // mage invisibility
    bool m_mageInvisibility;

    RONIN_INLINE bool HasKnownTitleByEntry(uint32 entry) { CharTitleEntry *titleEntry = dbcCharTitle.LookupEntry(entry); return (titleEntry && HasKnownTitleByIndex(titleEntry->bit_index)); }
    RONIN_INLINE bool HasKnownTitleByIndex(uint32 bitIndex) { return HasFlag((PLAYER__FIELD_KNOWN_TITLES+(bitIndex / 32)), uint32(uint32(1) << (bitIndex % 32))); }
    void SetKnownTitle( int32 title, bool set );

    uint8 GetChatTag() const;
    uint8 GetGuildMemberFlags();
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
    uint8 TheoreticalUseRunes(uint32 *runeCost);

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
