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
#define PLAYER_NORMAL_RUN_SPEED 7.0f
#define PLAYER_NORMAL_SWIM_SPEED 4.722222f
#define PLAYER_NORMAL_FLIGHT_SPEED 7.0f
#define PLAYER_HONORLESS_TARGET_SPELL 2479
#define MONSTER_NORMAL_RUN_SPEED 8.0f

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
    WoWGuid guid;
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
    uint32 CurrentValue;
    uint32 MaximumValue;
    uint32 BonusValue;
    uint32 BonusTalent;
    float GetSkillUpChance();
    bool Reset(uint32 Id);
    uint8 SkillPos;
};

class ArenaTeam;
struct PlayerCooldown
{
    uint32 ExpireTime;
    uint32 ItemId;
    uint32 SpellId;
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
    void _LoadPlayerCooldowns(QueryResult * result);
    void _SavePlayerCooldowns(QueryBuffer * buf);

    // END COOLDOWNS

public:

    bool ok_to_remove;
    void EquipInit(PlayerCreateInfo *EquipInfo);
    void SendDungeonDifficulty();
    void SendRaidDifficulty();

    void AddToWorld(bool loggingin = false);
    void AddToWorld(MapMgr* pMapMgr);
    void RemoveFromWorld();
    bool Create ( WorldPacket &data );

    void BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag);
    std::string m_afk_reason;
    void SetAFKReason(std::string reason) { m_afk_reason = reason; };
    HEARTHSTONE_INLINE const char* GetName() { return m_name.c_str(); }
    HEARTHSTONE_INLINE std::string* GetNameString() { return &m_name; }

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
    std::vector<TaxiPath*>   m_taxiPaths;
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
    uint32 GetQuestSlotQuestId(uint16 slot) const { return GetUInt32Value(PLAYER_QUEST_LOG + slot * 5 + (uint32)NULL); }

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

    void CalcDamage();
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
    SpellOverrideMap    mSpellOverrideMap;
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
    void EventGroupFullUpdate();
    void GroupUninvite(Player* player, PlayerInfo *info);

    void ClearGroupInviter() { m_GroupInviter.Clean(); }
    void SetInviter(WoWGuid pInviter) { m_GroupInviter = pInviter; }
    HEARTHSTONE_INLINE WoWGuid      GetInviter() { return m_GroupInviter; }
    HEARTHSTONE_INLINE bool         InGroup() { return (m_playerInfo->m_Group != NULL && m_GroupInviter.empty()); }
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
    HEARTHSTONE_INLINE int      HasBeenInvited() { return !m_GroupInviter.empty(); }
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

    HEARTHSTONE_INLINE void SetBanned() { m_banned = 4;}
    HEARTHSTONE_INLINE void SetBanned(std::string Reason) { m_banned = 4; m_banreason = Reason;}
    HEARTHSTONE_INLINE void SetBanned(uint32 timestamp, std::string& Reason) { m_banned = timestamp; m_banreason = Reason; }
    HEARTHSTONE_INLINE void UnSetBanned() { m_banned = 0; }
    HEARTHSTONE_INLINE std::string GetBanReason() {return m_banreason;}

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
    HEARTHSTONE_INLINE uint32 GetGuildLevel() { return GetUInt32Value(PLAYER_GUILDLEVEL); }

    void SetGuildId(uint32 guildId);
    HEARTHSTONE_INLINE void SetGuildRank(uint32 guildRank) { SetUInt32Value(PLAYER_GUILDRANK, guildRank); }
    HEARTHSTONE_INLINE void SetGuildLevel(uint32 guildLevel) { SetUInt32Value(PLAYER_GUILDLEVEL, guildLevel); }

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
    HEARTHSTONE_INLINE const WoWGuid& GetLootGUID() const { return m_lootGuid; }
    HEARTHSTONE_INLINE void SetLootGUID(const WoWGuid &guid) { m_lootGuid = guid; }
    void SendLoot(WoWGuid guid, uint32 mapid, uint8 loot_type);
    // loot variables
    WoWGuid m_lootGuid;
    WoWGuid m_currentLoot;
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

    std::list<LoginAura*> m_loginAuras;

    std::set<uint32> OnMeleeAuras;

    /************************************************************************/
    /* Player loading and savings                                           */
    /* Serialize character to db                                            */
    /************************************************************************/
    void SaveToDB(bool bNewCharacter);
    void SaveAuras(std::stringstream&);
    bool LoadFromDB();
    void LoadFromDBProc(QueryResultVector & results);

    void LoadNamesFromDB(uint32 guid);
    bool m_FirstLogin;

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
    void SendPetUntrainConfirm();
    void SendXPToggleConfirm();
    void SetPlayerStatus(uint8 pStatus) { m_status = pStatus; }
    void CheckPlayerStatus(uint8 pStatus) { if(m_status == pStatus) m_status = NONE; }
    HEARTHSTONE_INLINE uint8 GetPlayerStatus() { return m_status; }
    const float& GetBindPositionX( ) const { return m_bind_pos_x; }
    const float& GetBindPositionY( ) const { return m_bind_pos_y; }
    const float& GetBindPositionZ( ) const { return m_bind_pos_z; }
    const uint32& GetBindMapId( ) const { return m_bind_mapid; }
    const uint32& GetBindZoneId( ) const { return m_bind_zoneid; }
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
    bool CanSee(WorldObject* obj);
    HEARTHSTONE_INLINE bool IsVisible(WorldObject* pObj) { return !(m_visibleObjects.find(pObj) == m_visibleObjects.end()); }
    void AddInRangeObject(WorldObject* pObj);
    void OnRemoveInRangeObject(WorldObject* pObj);
    void ClearInRangeSet();
    HEARTHSTONE_INLINE void AddVisibleObject(WorldObject* pObj) { m_visibleObjects.insert(pObj); }
    HEARTHSTONE_INLINE void RemoveVisibleObject(WorldObject* pObj) { m_visibleObjects.erase(pObj); }
    HEARTHSTONE_INLINE void RemoveVisibleObject(InRangeSet::iterator itr) { m_visibleObjects.erase(itr); }
    HEARTHSTONE_INLINE InRangeSet::iterator FindVisible(WorldObject* obj) { return m_visibleObjects.find(obj); }
    HEARTHSTONE_INLINE void RemoveIfVisible(WorldObject* obj)
    {
        InRangeSet::iterator itr = m_visibleObjects.find(obj);
        if(itr == m_visibleObjects.end())
            return;

        m_visibleObjects.erase(obj);
        PushOutOfRange(obj->GetGUID());
    }

    HEARTHSTONE_INLINE bool GetVisibility(WorldObject* obj, InRangeSet::iterator *itr)
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

    HEARTHSTONE_INLINE float res_M_crit_get(){return m_resist_critical[0];}
    HEARTHSTONE_INLINE void res_M_crit_set(float newvalue){m_resist_critical[0]=newvalue;}
    HEARTHSTONE_INLINE float res_R_crit_get(){return m_resist_critical[1];}
    HEARTHSTONE_INLINE void res_R_crit_set(float newvalue){m_resist_critical[1]=newvalue;}

    float m_resist_critical[2];//when we are a victim we can have talents to decrease chance for critical hit. This is a negative value and it's added to critchances
    float m_resist_hit[3]; // 0 = melee; 1= ranged; 2=spells
    uint32 m_modphyscritdmgPCT;
    uint32 m_RootedCritChanceBonus; // Class Script Override: Shatter
    uint32 m_Illumination_amount; // Class Script Override: Illumination
    float Damageshield_amount; // Damage Shield

    uint32 m_casted_amount[7]; //Last casted spells amounts. Need for some spells. Like Ignite etc. DOesn't count HoTs and DoTs. Only directs

    uint32 StatModPctPos[5];
    uint32 StatModPctNeg[5];
    uint32 TotalStatModPctPos[5];
    uint32 TotalStatModPctNeg[5];
    int32 IncreaseDamageByType[12]; //mod dmg by creature type
    float IncreaseDamageByTypePCT[12];
    float IncreaseCricticalByTypePCT[12];
    int32 DetectedRange;
    float PctIgnoreRegenModifier;
    HEARTHSTONE_INLINE uint32* GetPlayedtime() { return m_playedtime; };
    HEARTHSTONE_INLINE float CalcRating(uint32 index) { return CalcPercentForRating(index, GetUInt32Value(index)); };
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

    std::map<uint32, WeaponModifier> damagedone;
    std::map<uint32, WeaponModifier> tocritchance;
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
    bool ignoreitemreq_cheat;

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

    GameObject* GetSummonedObject () {return m_SummonedObject;};
    void SetSummonedObject (GameObject* t_SummonedObject) {m_SummonedObject = t_SummonedObject;};
    uint32 roll;

    void ClearCooldownsOnLine(uint32 skill_line, uint32 called_from);
    void ClearCooldownsOnLines(std::set<uint32> skill_lines, uint32 called_from);
    void ResetAllCooldowns();
    void ClearCooldownForSpell(uint32 spell_id);
    PlayerCooldownMap GetCooldownMap(uint8 index = COOLDOWN_TYPE_SPELL) { return m_cooldownMap[index]; };
    bool SpellHasCooldown(uint32 spellid) { return (m_cooldownMap[COOLDOWN_TYPE_SPELL].find(spellid) != m_cooldownMap[COOLDOWN_TYPE_SPELL].end()); };

    bool sentMOTD;
    void sendMOTD();

    void PushOutOfRange(const WoWGuid & guid);
    void PushUpdateBlock(ByteBuffer *data, uint32 updatecount);
    void PopPendingUpdates();

    bool CompressAndSendUpdateBuffer(uint32 size, const uint8* update_buffer, ByteBuffer *pCompressionBuffer);
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
    std::unordered_set<WorldObject* > m_visibleFarsightObjects;
    void EventTeleport(uint32 mapid, float x, float y, float z, float o);
    void EventTeleport(uint32 mapid, float x, float y, float z)
    {
        EventTeleport(mapid, x, y, z, 0.0f);
    }

    void BroadcastMessage(const char* Format, ...);
    std::map<uint32, std::set<uint32> > SummonSpells;
    std::map<uint32, PetSpellMap*> PetSpells;
    void AddSummonSpell(uint32 Entry, uint32 SpellID);
    void RemoveSummonSpell(uint32 Entry, uint32 SpellID);
    std::set<uint32>* GetSummonSpells(uint32 Entry);
    LockedQueue<WorldPacket*> delayedPackets;
    std::set<Player*  > gmTargets;
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
    uint32 load_health, load_power[5];
    std::set<SpellEntry *> castSpellAtLogin;

    /////
    void OnPushToWorld();
    void OnPrePushToWorld();
    void OnWorldPortAck();
    void OnWorldLogin();
    void CompleteLoading();
    void SendObjectUpdate(WoWGuid guid);

    uint32 m_TeleportState;
    std::set<Unit* > visiblityChangableSet;
    bool m_beingPushed;
    bool CanSignCharter(Charter * charter, Player* requester);
    uint32 m_FlyingAura;
    std::stringstream LoadAuras;
    bool resend_speed;
    bool rename_pending;
    bool m_XPoff;
    bool customizable;
    uint32 iInstanceType;
    uint32 iRaidType;
    uint32 AnnihilationProcChance;
    HEARTHSTONE_INLINE void SetName(std::string& name) { m_name = name; }
    // spell to (delay, last time)
    FactionReputation * reputationByListId[128];
    Channel* watchedchannel;

    WoWGuid m_comboTarget;
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
        if(!IsInWorld()) return NULL;
        return m_mapMgr->GetPlayer(mTradeTarget);
    }

    Item* getTradeItem(uint32 slot) { return mTradeItems[slot]; };

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

    LocationVector m_last_group_position;
    int32 m_rap_mod_pct;
    void SummonRequest(WorldObject* Requestor, uint32 ZoneID, uint32 MapID, uint32 InstanceID, const LocationVector & Position);
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

protected:
    uint32 m_timeLogoff;
    LocationVector m_summonPos;
    int32 m_summonInstanceId;
    uint32 m_summonMapId;
    WorldObject* m_summoner;

    uint32 iActivePet;
    void _SetCreateBits(UpdateMask *updateMask, Player* target) const;
    void _SetUpdateBits(UpdateMask *updateMask, Player* target) const;

    /* Update system components */
    Mutex _bufferS;
    bool bProcessPending;
    uint32 mUpdateDataCount, mOutOfRangeIdCount;
    ByteBuffer bUpdateDataBuffer, mOutOfRangeIds;
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
    WoWGuid mTradeTarget;
    Item* mTradeItems[7];
    uint32 mTradeGold;
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
    std::list<ItemSet> m_itemsets;
    //Duel
    uint32 m_duelCountdownTimer;
    uint8 m_duelStatus;
    uint8 m_duelState;
    // Played time
    uint32 m_playedtime[3];
    bool m_isResting;
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

    uint32 armor_proficiency;
    uint32 weapon_proficiency;
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
    std::unordered_set<WorldObject* > m_visibleObjects;
    // Groups/Raids
    WoWGuid m_GroupInviter;

    // Fishing related
    GameObject* m_SummonedObject;

    // other system
    Corpse* myCorpse;

    uint32 m_lastHonorResetTime;
    uint32 trigger_on_stun;         //bah, warrior talent but this will not get triggered on triggered spells if used on proc so i'm forced to used a special variable
    uint32 trigger_on_stun_chance;  //also using this for mage "Frostbite" talent

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
    std::map<uint32, uint32> m_forcedReactions;

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
    std::map<WoWGuid, char*> m_friends;
    std::set<WoWGuid> m_ignores;
    std::set<WoWGuid> m_hasFriendList;

    void Social_SendFriendList(uint32 flag);

    void Social_AddFriend(const char * name, const char * note);
    void Social_RemoveFriend(WoWGuid guid);

    void Social_AddIgnore(const char * name);
    void Social_RemoveIgnore(WoWGuid guid);

    void Social_SetNote(WoWGuid guid, const char * note);

public:
    bool Social_IsIgnoring(PlayerInfo * m_info);
    bool Social_IsIgnoring(WoWGuid guid);

    void Social_TellOnlineStatus(bool online = true);
    void Social_TellFriendsStatus();

    /************************************************************************/
    /* end social                                                           */
    /************************************************************************/

    PlayerInfo * m_playerInfo;
    PlayerInfo * getPlayerInfo() const {return m_playerInfo;}
    PlayerCreateInfo *getPlayerCreateInfo() const {return info; }
    uint32 m_skipCastCheck[3];  // spell group relation of spell types that should ignore some cancast checks
    uint32 m_castFilter[3];     // spell group relation of only spells that player can currently cast

    // grounding totem
    Aura* m_magnetAura;

    // mage invisibility
    bool m_mageInvisibility;

    HEARTHSTONE_INLINE bool HasKnownTitleByEntry(uint32 entry) { CharTitlesEntry *titleEntry = dbcCharTitles.LookupEntry(entry); return (titleEntry && HasKnownTitleByIndex(titleEntry->index)); }
    HEARTHSTONE_INLINE bool HasKnownTitleByIndex(uint32 bitIndex) { return HasFlag((PLAYER__FIELD_KNOWN_TITLES+(bitIndex / 32)), uint32(uint32(1) << (bitIndex % 32))); }
    void SetKnownTitle( int32 title, bool set );

    // Avenging Wrath...
    bool mAvengingWrath;
    void AvengingWrath() { mAvengingWrath = true; }

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
