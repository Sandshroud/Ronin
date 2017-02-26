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

class Channel;
class Creature;
class TaxiPath;
class GameObject;
class Transporter;
class Corpse;
struct GuildRank;
class Charter;
class LFGMatch;
struct PlayerCreateInfo;

#define PLAYER_HONORLESS_TARGET_SPELL 2479

// gold cap
#define PLAYER_MAX_GOLD 0x7FFFFFFF

//====================================================================
//  Inventory
//  Holds the display id and item type id for objects in
//  a character's inventory
//====================================================================

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
const float baseDodge[CLASS_MAX] = { 0.0f, 3.664f, 3.4943f, -4.0873f, 2.0957f, 3.4178f, 3.66400f, 2.108f, 3.66f, 2.4211f, 0.0f, 5.6097f };

// Crit/agility to dodge/agility coefficient multipliers; 3.2.0 increased required agility by 15%
static const float crit_to_dodge[CLASS_MAX] = { 0.f, 0.85f / 1.15f, 1.00f / 1.15f, 1.11f / 1.15f, 2.00f / 1.15f, 1.00f / 1.15f, 0.85f / 1.15f, 1.60f / 1.15f, 1.00f / 1.15f, 0.97f / 1.15f, 0.0f, 2.00f / 1.15f };

// Dodge ( class Cap )      UNUSED  War Paladin hunter  rogue   priest  DK  Shaman  Mage    Lock UNUSED Druid
const float DodgeCap[CLASS_MAX] = { 0.0f, 84.8f, 59.9f, 86.2f, 47.8f, 59.9f, 84.7f, 59.9f, 58.8f, 59.9f, 0.0f, 47.8f };

enum RuneTypes
{
    RUNE_TYPE_BLOOD         = 0,
    RUNE_TYPE_FROST         = 1,
    RUNE_TYPE_UNHOLY        = 2,
    RUNE_TYPE_DEATH         = 3,
    RUNE_TYPE_RECHARGING    = 4
};
static const uint8 baseRunes[6] = {RUNE_TYPE_BLOOD,RUNE_TYPE_BLOOD,RUNE_TYPE_FROST,RUNE_TYPE_FROST,RUNE_TYPE_UNHOLY,RUNE_TYPE_UNHOLY};

struct PlayerInfo
{
    PlayerInfo(WoWGuid _guid);
    ~PlayerInfo();

    Mutex dataLock;
    WoWGuid charGuid;
    std::string charName;
    uint8 charRace;
    uint8 charClass;
    uint8 charTeam;
    uint32 charAppearance;
    uint32 charAppearance2;
    uint32 charAppearance3;
    uint8 charCustomizeFlags;

    // Cached information
    uint8 lastDeathState;
    uint32 lastLevel;
    uint32 lastMapID;
    uint32 lastInstanceID;
    float lastPositionX;
    float lastPositionY;
    float lastPositionZ;
    float lastOrientation;
    uint32 lastZone;
    time_t lastOnline;

    // Misc data
    uint32 achievementPoints;
    uint32 professionId[2], professionSkill[2], professionRank[2];

    Group *m_Group;
    int8 subGroup;

    uint32 GuildId;
    uint32 GuildRank;
    Player* m_loggedInPlayer;
    uint32 charterId[NUM_CHARTER_TYPES];
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
    // Can be spell power or other
    MOD_SLOT_BONUS_STAT,
    MOD_SLOT_MAX_STAT = 15,

    // Define this here ahead of time, as these are based on item enchant slots
    MOD_SLOT_MAX_ENCHANT = MOD_SLOT_MAX_STAT+MAX_ENCHANTMENT_SLOT,

    // Start our item enchantment mod slots
    MOD_SLOT_PERM_ENCHANT = 15,
    MOD_SLOT_PERM_ENCHANT_1,
    MOD_SLOT_PERM_ENCHANT_2,
    MOD_SLOT_PERM_ENCHANT_3,

    MOD_SLOT_TEMP_ENCHANT,
    MOD_SLOT_TEMP_ENCHANT_1,
    MOD_SLOT_TEMP_ENCHANT_2,
    MOD_SLOT_TEMP_ENCHANT_3,

    MOD_SLOT_SOCKET_ENCHANT_1,
    MOD_SLOT_SOCKET_ENCHANT_1_1,
    MOD_SLOT_SOCKET_ENCHANT_1_2,
    MOD_SLOT_SOCKET_ENCHANT_1_3,

    MOD_SLOT_SOCKET_ENCHANT_2,
    MOD_SLOT_SOCKET_ENCHANT_2_1,
    MOD_SLOT_SOCKET_ENCHANT_2_2,
    MOD_SLOT_SOCKET_ENCHANT_2_3,

    MOD_SLOT_SOCKET_ENCHANT_3,
    MOD_SLOT_SOCKET_ENCHANT_3_1,
    MOD_SLOT_SOCKET_ENCHANT_3_2,
    MOD_SLOT_SOCKET_ENCHANT_3_3,

    MOD_SLOT_BONUS_ENCHANT,
    MOD_SLOT_BONUS_ENCHANT_1,
    MOD_SLOT_BONUS_ENCHANT_2,
    MOD_SLOT_BONUS_ENCHANT_3,

    MOD_SLOT_PRISMATIC_ENCHANT,
    MOD_SLOT_PRISMATIC_ENCHANT_1,
    MOD_SLOT_PRISMATIC_ENCHANT_2,
    MOD_SLOT_PRISMATIC_ENCHANT_3,

    MOD_SLOT_UNKNOWN_ENCHANT,
    MOD_SLOT_UNKNOWN_ENCHANT_1,
    MOD_SLOT_UNKNOWN_ENCHANT_2,
    MOD_SLOT_UNKNOWN_ENCHANT_3,

    MOD_SLOT_REFORGE_ENCHANT,
    MOD_SLOT_REFORGE_ENCHANT_1,
    MOD_SLOT_REFORGE_ENCHANT_2,
    MOD_SLOT_REFORGE_ENCHANT_3,

    // Spacer, not actually used
    MOD_SLOT_TRANSMOG_ENCHANT,
    MOD_SLOT_TRANSMOG_ENCHANT_1,
    MOD_SLOT_TRANSMOG_ENCHANT_2,
    MOD_SLOT_TRANSMOG_ENCHANT_3,

    MOD_SLOT_PROPRETY_ENCHANT_0,
    MOD_SLOT_PROPRETY_ENCHANT_0_1,
    MOD_SLOT_PROPRETY_ENCHANT_0_2,
    MOD_SLOT_PROPRETY_ENCHANT_0_3,

    MOD_SLOT_PROPRETY_ENCHANT_1,
    MOD_SLOT_PROPRETY_ENCHANT_1_1,
    MOD_SLOT_PROPRETY_ENCHANT_1_2,
    MOD_SLOT_PROPRETY_ENCHANT_1_3,

    MOD_SLOT_PROPRETY_ENCHANT_2,
    MOD_SLOT_PROPRETY_ENCHANT_2_1,
    MOD_SLOT_PROPRETY_ENCHANT_2_2,
    MOD_SLOT_PROPRETY_ENCHANT_2_3,

    MOD_SLOT_PROPRETY_ENCHANT_3,
    MOD_SLOT_PROPRETY_ENCHANT_3_1,
    MOD_SLOT_PROPRETY_ENCHANT_3_2,
    MOD_SLOT_PROPRETY_ENCHANT_3_3,

    MOD_SLOT_PROPRETY_ENCHANT_4,
    MOD_SLOT_PROPRETY_ENCHANT_4_1,
    MOD_SLOT_PROPRETY_ENCHANT_4_2,
    MOD_SLOT_PROPRETY_ENCHANT_4_3
};

typedef std::map<std::pair<uint64, uint32>, std::pair<uint32, int32>> ItemBonusModMap;
typedef std::map<uint32, ItemBonusModMap> ItemBonusModByType;

enum PlayerLoadOrder : uint8
{
    PLAYER_LO_DATA = 0,
    PLAYER_LO_ACTIONS,
    PLAYER_LO_AURAS,
    PLAYER_LO_BANS,
    PLAYER_LO_CURRENCY,
    PLAYER_LO_COOLDOWNS,
    PLAYER_LO_ACHIEVEMENT_DATA,
    PLAYER_LO_CRITERIA_DATA,
    PLAYER_LO_EQUIPMENTSETS,
    PLAYER_LO_EXPLORATION,
    PLAYER_LO_GLYPHS,
    PLAYER_LO_ITEMS,
    PLAYER_LO_ITEM_ENCHANTS,
    PLAYER_LO_KNOWN_TITLES,
    PLAYER_LO_QUEST_LOG,
    PLAYER_LO_QUESTS_COMPLETED,
    PLAYER_LO_QUESTS_COMPLETED_REPEATABLE,
    PLAYER_LO_REPUTATIONS,
    PLAYER_LO_SKILLS,
    PLAYER_LO_SOCIAL,
    PLAYER_LO_SPELLS,
    PLAYER_LO_TALENTS,
    PLAYER_LO_TAXIMASKS,
    PLAYER_LO_TIMESTAMPS
};

enum PlayerLoadFields
{
    PLAYERLOAD_FIELD_LOAD_DATA = 0,
    PLAYERLOAD_FIELD_LOAD_DATA_1,
    PLAYERLOAD_FIELD_PLAYER_BYTES,
    PLAYERLOAD_FIELD_PLAYER_BYTES2,
    PLAYERLOAD_FIELD_XP,
    PLAYERLOAD_FIELD_GOLD,
    PLAYERLOAD_FIELD_AVAILABLE_PROF_POINTS,
    PLAYERLOAD_FIELD_SELECTED_TITLE,
    PLAYERLOAD_FIELD_WATCHED_FACTION_INDEX,
    PLAYERLOAD_FIELD_TALENT_ACTIVE_SPEC,
    PLAYERLOAD_FIELD_TALENT_SPEC_COUNT,
    PLAYERLOAD_FIELD_TALENT_RESET_COUNTER,
    PLAYERLOAD_FIELD_TALENT_BONUS_POINTS,
    PLAYERLOAD_FIELD_TALENT_SPECSTACK,
    PLAYERLOAD_FIELD_BINDMAPID,
    PLAYERLOAD_FIELD_BINDPOSITION_X,
    PLAYERLOAD_FIELD_BINDPOSITION_Y,
    PLAYERLOAD_FIELD_BINDPOSITION_Z,
    PLAYERLOAD_FIELD_BINDZONEID,
    PLAYERLOAD_FIELD_TAXI_PATH,
    PLAYERLOAD_FIELD_TAXI_MOVETIME,
    PLAYERLOAD_FIELD_TAXI_TRAVELTIME,
    PLAYERLOAD_FIELD_TAXI_MOUNTID,
    PLAYERLOAD_FIELD_TRANSPORTERGUID,
    PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_X,
    PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_Y,
    PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_Z,
    PLAYERLOAD_FIELD_ENTRYPOINT_MAP,
    PLAYERLOAD_FIELD_ENTRYPOINT_X,
    PLAYERLOAD_FIELD_ENTRYPOINT_Y,
    PLAYERLOAD_FIELD_ENTRYPOINT_Z,
    PLAYERLOAD_FIELD_ENTRYPOINT_O,
    PLAYERLOAD_FIELD_INSTANCE_DIFFICULTY,
    PLAYERLOAD_FIELD_RAID_DIFFICULTY,
    PLAYERLOAD_FIELD_ISRESTING,
    PLAYERLOAD_FIELD_RESTSTATE,
    PLAYERLOAD_FIELD_RESTTIME,
    PLAYERLOAD_FIELD_RESTAREATRIGGER,
    PLAYERLOAD_FIELD_LAST_WEEK_RESET_TIME,
    PLAYERLOAD_FIELD_LAST_SAVE_TIME,
    PLAYERLOAD_FIELD_NEEDS_POSITION_RESET,
    PLAYERLOAD_FIELD_NEEDS_TALENT_RESET
    //PLAYERLOAD_FIELD_ACHIEVEMENTPOINTS,
};

class PlayerCellManager : public ObjectCellManager
{
public:
    PlayerCellManager(WorldObject *obj) : ObjectCellManager(obj) {}

    virtual void Update(MapInstance *instance, uint32 msTime, uint32 uiDiff);
    virtual void SetCurrentCell(MapInstance *instance, uint16 newX, uint16 newY, uint8 cellRange) override;

    void AddProcessedCell(uint16 cellX, uint16 cellY) { _processedCells.insert(_makeCell(cellX, cellY)); }

private:
    friend class MapInstance;

    // Stack 0 is higher priority, stack 1 is lower priority
    std::unordered_set<uint32> _delayedCells[2], _processedCells;
};

//====================================================================
//  Player
//  Class that holds every created character on the server.
//
//  TODO:  Attach characters to user accounts
//====================================================================
typedef std::set<uint32>                            SpellSet;
typedef std::list<uint32>                           SpellList;
typedef std::set<uint32>                            SaveSet;
typedef std::map<uint32, uint32>                    SpellOverrideExtraAuraMap;
typedef std::map<uint32, uint64>                    SoloSpells;
typedef std::map<SpellEntry*, std::pair<uint32, uint32> >StrikeSpellMap;
typedef std::map<uint32, OnHitSpell >               StrikeSpellDmgMap;
typedef std::set<Player* *>                         ReferenceSet;
typedef std::map<uint32, PlayerCooldown>            PlayerCooldownMap;

class SERVER_DECL Player : public Unit
{
    friend class WorldSession;
    friend class SkillIterator;

public:
    Player(PlayerInfo *pInfo, WorldSession *session, uint32 fieldCount = PLAYER_END);
    ~Player();

    bool Initialize();
    virtual void Init() {};
    virtual void Destruct();

    virtual void RemoveFromWorld();
    virtual void EventExploration(MapInstance *instance);

    virtual bool IsPlayer() { return true; }

    // Reactivate is a function players don't need
    virtual void Reactivate() {}

    void Update( uint32 msTime, uint32 diff );

    virtual bool IsInWorld() { return m_mapInstance != NULL && Object::IsInWorld(); }

    // Session interaction
    // SESSION SAFE | Pushes packets when inworld, if not then check if queue size and push to back or send directly
    void PushPacket(WorldPacket *data, bool direct = false);

    // Do not use for packets, use push instead
    RONIN_INLINE WorldSession* GetSession() const { return m_session; }

    // GM functionality
    bool isGM() { return m_gmData != NULL; }

    uint8 getGMSight() { return m_gmData->gmSightType; }
    void setGMSight(uint8 sightType) { m_gmData->gmSightType = sightType; }

    uint32 getGMEventSight() { return m_gmData->gmSightEventID; }
    void setGMEventSight(uint32 eventId) { m_gmData->gmSightEventID = eventId; }

    uint32 getGMPhaseSight() { return m_gmData->gmSightPhaseMask; }
    void setGMPhaseSight(uint32 phaseMask) { m_gmData->gmSightPhaseMask = phaseMask; }

    bool hasGMTag() { return HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM|PLAYER_FLAG_DEVELOPER); }
    bool hasCooldownCheat() { return false; }

public: /// Interface Interaction Functions
    RONIN_INLINE PlayerInventory *GetInventory() { return &m_inventory; }
    RONIN_INLINE PlayerCurrency *GetCurrency() { return &m_currency; }
    RONIN_INLINE TalentInterface *GetTalentInterface() { return &m_talentInterface; }
    RONIN_INLINE FactionInterface *GetFactionInterface() { return &m_factionInterface; }

public: /// Player field based functions
    // Stat calculations/Field recalcs
    RONIN_INLINE virtual float GetDodgeChance() { return GetFloatValue(PLAYER_DODGE_PERCENTAGE); }
    RONIN_INLINE virtual float GetHitChance() { return GetFloatValue(PLAYER_FIELD_UI_HIT_MODIFIER); }

    // PvP rank
    RONIN_INLINE uint8 GetPVPRank() { return (uint8)((GetUInt32Value(PLAYER_BYTES_3) >> 24) & 0xFF); }
    RONIN_INLINE void SetPVPRank(int newrank) { SetUInt32Value(PLAYER_BYTES_3, ((GetUInt32Value(PLAYER_BYTES_3) & 0x00FFFFFF) | (uint8(newrank) << 24))); }

    // Quest slots
    RONIN_INLINE uint32 GetQuestSlotId(uint16 slot) const { return GetUInt32Value(PLAYER_QUEST_LOG + slot * 5); }
    RONIN_INLINE void SetQuestSlotId(uint16 slot, uint32 value) { SetUInt32Value(PLAYER_QUEST_LOG + slot * 5, value); }

    // Guard hostility flags
    RONIN_INLINE void SetGuardHostileFlag() { SetFlag(PLAYER_FLAGS, PLAYER_FLAG_UNKNOWN2); }
    RONIN_INLINE void RemoveGuardHostileFlag() { RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_UNKNOWN2); }

    // Guild data
    RONIN_INLINE bool IsInGuild() { return (GetUInt32Value(PLAYER_GUILDID) > 0); }
    RONIN_INLINE uint32 GetGuildId() { return GetUInt32Value(PLAYER_GUILDID); }
    RONIN_INLINE uint32 GetGuildRank() { return GetUInt32Value(PLAYER_GUILDRANK); }
    RONIN_INLINE uint32 GetGuildLevel() { return GetUInt32Value(PLAYER_GUILDLEVEL); }
    RONIN_INLINE void SetGuildRank(uint32 guildRank) { SetUInt32Value(PLAYER_GUILDRANK, guildRank); }
    RONIN_INLINE void SetGuildLevel(uint32 guildLevel) { SetUInt32Value(PLAYER_GUILDLEVEL, guildLevel); }

    // Currency gold
    RONIN_INLINE void AddCoins( int32 coins ) { ModUnsigned32Value(PLAYER_FIELD_COINAGE , coins); }
    RONIN_INLINE void TakeCoins( int32 coins ) { ModUnsigned32Value(PLAYER_FIELD_COINAGE, -coins); }

    // Title function
    RONIN_INLINE bool HasKnownTitleByIndex(uint32 bitIndex) { return HasFlag((PLAYER__FIELD_KNOWN_TITLES+(bitIndex / 32)), uint32(uint32(1) << (bitIndex % 32))); }

public: /// Player loading and savings
    void CreateInDatabase();

    void SaveToDB(bool bNewCharacter = false);
    static void DeleteFromDB(WoWGuid guid);

    bool LoadFromDB();
    void LoadFromDBProc(QueryResultVector & results);

private:
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

    void _LoadPlayerQuestLog(QueryResult * result);
    void _SavePlayerQuestLog(QueryBuffer * buf);

    void _LoadCompletedQuests(QueryResult *completionMasks, QueryResult *repeatable);
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

public: /// Stat Calculation
    void ProcessImmediateItemUpdate(Item *item);
    void ProcessPendingItemUpdates();
    void ItemFieldUpdated(Item *item);
    void ItemDestructed(Item *item);

    void OnFieldUpdated(uint16 index);

    void UpdatePlayerRatings();
    void UpdatePlayerDamageDoneMods();

protected:
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

    void UpdateCombatRating(uint8 combatRating, float value);
    float GetRatioForCombatRating(uint8 combatRating);
    int32 CalculatePlayerCombatRating(uint8 combatRating);

public: /// Interface functions


private:

public: /// Player field based functions
    virtual void setLevel(uint32 level);

    void ClearSession() { m_session = NULL; }

    /************************************************************************/
    /* Talent System                                                        */
    /************************************************************************/
    bool LearnTalent(uint32 talent_id, uint8 requested_rank) { return m_talentInterface.LearnTalent(talent_id, requested_rank); }
    bool HasTalent(uint32 talentid, int8 rank = -1) { return m_talentInterface.HasTalent(talentid, rank); }
    void ResetSpec(uint8 spec) { m_talentInterface.ResetSpec(spec, false); }

    uint32 CalcTalentResetCost(uint32 resetnum);
    void SendTalentResetConfirm();

    // Instance system
    void ResetAllInstanceLinks();
    bool CanCreateNewDungeon(uint32 mapId);
    bool LinkToInstance(MapInstance *instance);
    uint32 GetLinkedInstanceID(MapEntry *mapEntry);
    std::map<std::pair<uint32, uint8>, uint32> m_savedInstanceIDs;
    std::deque<std::pair<uint32, time_t>> m_instanceLinkTimers;

    /************************************************************************/
    /* Skill System                                                         */
    /************************************************************************/

    bool HasSkillLine(uint16 skillLine);
    void AddSkillLine(uint16 skillLine, uint16 step, uint16 skillMax, uint16 skillCurrent, uint16 skillModifier = 0, uint16 bonusTalent = 0);
    void RemoveSkillLine(uint16 skillLine);

    void UpdateSkillLine(uint16 skillLine, uint16 step, uint16 skillMax, bool forced = false);
    void ModSkillLineAmount(uint16 skillLine, int16 amount, bool bonus);

    uint16 getSkillLineMax(uint16 SkillLine);
    uint16 getSkillLineVal(uint16 SkillLine, bool IncludeBonus = true);
    uint16 getSkillLineStep(uint16 skillLine);

    void _UpdateMaxSkillCounts();
    void RemoveSpellsFromLine(uint16 skill_line);

    void _AddLanguages(bool All);
    void _AdvanceAllSkills(uint16 count, bool skipprof = false, uint16 max = 0);

    // Get mount capability based on mount skill
    SpellEntry *GetMountCapability(uint32 mountType);

    void RecalculateHonor();

    RONIN_INLINE uint8 GetBGTeam() { return m_bgTeam; }
    RONIN_INLINE void SetBGTeam(uint8 t) { m_bgTeam = t; }

    static bool IsValidGender(uint8 Gender) { return Gender <= 1; }
    static bool IsValidClass(uint8 Class) { return ((1 << (Class - 1)) & CLASSMASK_ALL_PLAYABLE) != 0; }
    static bool IsValidRace(uint8 Race) { return ((1 << (Race - 1)) & RACEMASK_ALL_PLAYABLE) != 0; }

    uint8 GetTrainerSpellStatus(TrainerSpell *spell);

public:
    void EventDBCChatUpdate(uint32 dbcID = 0xFFFFFFFF);
protected:
    bool AllowChannelAtLocation(uint32 dbcID, AreaTableEntry *areaTable);
    bool UpdateChatChannel(const char* areaName, AreaTableEntry *areaTable, ChatChannelEntry* entry, Channel* channel);

public:
    void Cooldown_OnCancel(SpellEntry *pSpell);
    void Cooldown_AddStart(SpellEntry * pSpell);
    void Cooldown_Add(SpellEntry * pSpell, Item* pItemCaster);
    void Cooldown_AddItem(ItemPrototype * pProto, uint32 x);
    bool Cooldown_CanCast(SpellEntry * pSpell);
    bool Cooldown_CanCast(ItemPrototype * pProto, uint32 x);

protected:
    void _Cooldown_Add(uint32 Type, uint32 Misc, time_t Time, uint32 SpellId, uint32 ItemId);
    // END COOLDOWNS

public:

    static float NoseHeight(uint8 race, uint8 gender)
    {
        static float noseHeight[RACE_COUNT*2] = { 1.72f, 1.78f, 1.82f, 1.98f, 1.27f, 1.4f, 1.84f, 2.13f, 1.61f, 1.8f, 2.48f, 2.01f, 1.06f, 1.04f, 2.02f, 1.93f, 1.06f, 1.04f, 1.83f, 1.93f, 2.09f, 2.36f, 1.72f, 1.78f };
        uint8 _race = std::min<uint8>(race, RACE_COUNT);
        return noseHeight[_race * 2 + (gender ? 0 : 1)];
    }

    void BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag);
    void SetAFKReason(std::string reason) { m_afk_reason = reason; };
    RONIN_INLINE const char* GetName() { return m_name.c_str(); }
    RONIN_INLINE std::string* GetNameString() { return &m_name; }

    //void KilledMonster(uint32 entry, const uint64 &guid);
    void GiveXP(uint32 xp, const uint64 &guid, bool allowbonus, bool allowGuildXP);   // to stop rest xp being given
    void ModifyBonuses(bool apply, uint64 guid, uint32 slot, uint32 type, int32 val, int32 randSuffixAmt = 0, int32 suffixSeed = 0);

    int32 GetBonusesFromItems(uint32 statType);

    /************************************************************************/
    /* Taxi                                                                 */
    /************************************************************************/
    UpdateMask* GetTaximask() { return &m_taxiMask; }
    void SetTaxiMask(UpdateMask &mask) { m_taxiMask = mask; }

    RONIN_INLINE TaxiPath* GetTaxiPath() { return m_taxiData ? m_taxiData->CurrentPath : NULL; }
    RONIN_INLINE bool GetTaxiState() { return m_taxiData && m_taxiData->CurrentPath; }
    void TaxiStart(TaxiPath* path, uint32 modelid, uint32 startOverride = 0);
    void JumpToEndTaxiNode(TaxiPath * path);
    void EventDismount(uint32 money, float x, float y, float z);
    void EventTaxiInterpolate();
    void InitTaxiNodes();
    bool HasNearbyTaxiNodes(uint32 from);

    RONIN_INLINE bool HasTaxiNode(uint32 node) { return m_taxiMask.GetBit(node); }
    RONIN_INLINE void AddTaxiMask(uint32 index) { m_taxiMask.SetBit(index); }
    RONIN_INLINE void RemoveTaxiMask(uint32 index) { m_taxiMask.UnsetBit(index); }

    /************************************************************************/
    /* Quests                                                               */
    /************************************************************************/
    bool HasQuests()
    {
        for(int i = 0; i < QUEST_LOG_COUNT; i++)
        {
            if(m_questLog[i] != 0)
                return true;
        }
        return false;
    }

    bool HasQuest(uint32 qid) { return GetQuestLogForEntry(qid) != NULL; }

    int32               GetOpenQuestSlot();
    uint32              GetQuestStatusForQuest(uint32 questid, uint8 type = 1, bool skiplevelcheck = false);
    QuestLogEntry*      GetQuestLogForEntry(uint32 quest);
    RONIN_INLINE  QuestLogEntry*GetQuestLogInSlot(uint32 slot){ return m_questLog[slot]; }
    RONIN_INLINE  uint32 GetQuestSharer(){ return m_questSharer; }

    RONIN_INLINE void SetQuestSharer(uint32 guid){ m_questSharer = guid; }
    void                SetQuestLogSlot(QuestLogEntry *entry, uint32 slot);

    RONIN_INLINE uint32 GetFinishedDailiesCount() { return (uint32)m_completedDailyQuests.size(); }

    void AddToCompletedQuests(uint32 quest_id);
    bool HasFinishedQuest(uint32 quest_id);
    bool HasFinishedDailyQuest(uint32 quest_id);
    bool HasQuestForItem(uint32 itemid);
    bool HasQuestSpell(uint32 spellid);
    void RemoveQuestSpell(uint32 spellid);
    bool HasQuestMob(uint32 entry);
    void RemoveQuestMob(uint32 entry);
    void RemoveQuestsFromLine(uint32 skill_line);
    void ResetDailyQuests();
    uint16 FindQuestSlot(uint32 questid);

    void EventPortToGM(uint32 guid);

    uint32 GetMainMeleeDamage(uint32 AP_owerride); //i need this for windfury

    CharRaceEntry *GetRaceDBC() { return myRace; }
    CharClassEntry *GetClassDBC() { return myClass; }

    const WoWGuid& GetSelection( ) const { return m_curSelection; }
    void SetSelection(const uint64 &guid) { m_curSelection = guid; }

    /************************************************************************/
    /* Spells                                                               */
    /************************************************************************/
    bool HasSpell(uint32 spell);
    SpellEntry* GetSpellWithNamehash(uint32 namehash);
    bool HasHigherSpellForSkillLine(SpellEntry* sp);
    void smsg_InitialSpells();
    void addSpell(uint32 spell_idy, uint32 forget = 0);
    void removeSpellByNameHash(uint32 hash);
    bool removeSpell(uint32 SpellID);
    uint32 FindSpellWithNamehash(uint32 namehash);
    uint32 FindHigherRankingSpellWithNamehash(uint32 namehash, uint32 minimumrank);
    uint32 FindHighestRankingSpellWithNamehash(uint32 namehash);
    SpellEntry* FindLowerRankSpell(SpellEntry* sp, int32 rankdiff);
    SpellEntry *FindHighestRankProfessionSpell(uint32 professionSkillLine);

    bool HasSpellWithEffect(uint8 spellEff) { return !m_spellsByEffect[spellEff].empty(); }

    void AddShapeShiftSpell(uint32 id);
    void RemoveShapeShiftSpell(uint32 id);

    /************************************************************************/
    /* Reputation                                                           */
    /************************************************************************/
    static Standing GetReputationRankFromStanding(int32 Standing_);
    void Reputation_OnTalk(FactionEntry *faction) { m_factionInterface.Reputation_OnTalk(faction); }

    /************************************************************************/
    /* Factions                                                             */
    /************************************************************************/

    /************************************************************************/
    /* PVP                                                                  */
    /************************************************************************/
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
    bool IsGroupLeader()
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
        if(m_banned && (m_banned < 100 || (uint32)UNIXTIME < m_banned))
            return true;
        return false;
    }

    RONIN_INLINE void SetBanned() { m_banned = 4;}
    RONIN_INLINE void SetBanned(std::string Reason) { m_banned = 4; m_banreason = Reason;}
    RONIN_INLINE void SetBanned(uint32 timestamp, std::string& Reason) { m_banned = timestamp; m_banreason = Reason; }
    RONIN_INLINE void UnSetBanned() { m_banned = 0; }
    RONIN_INLINE std::string GetBanReason() {return m_banreason;}

    /************************************************************************/
    /* Guilds                                                               */
    /************************************************************************/
    void SetGuildId(uint32 guildId);
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

public:
    /************************************************************************/
    /* Player Items                                                         */
    /************************************************************************/
    void ApplyItemMods(Item* item, uint8 slot, bool apply);

public:
    /************************************************************************/
    /* Loot                                                                 */
    /************************************************************************/
    RONIN_INLINE const WoWGuid& GetLootGUID() const { return m_lootGuid; }
    RONIN_INLINE void SetLootGUID(const WoWGuid &guid) { m_lootGuid = guid; }
    void SendLoot(WoWGuid guid, uint32 mapid, uint8 loot_type);
    bool CanNeedItem(ItemPrototype* proto);

    /************************************************************************/
    /* World Session                                                        */
    /************************************************************************/
    void SetBindPoint(float x, float y, float z, uint32 m, uint32 v) { m_bindData.posX = x; m_bindData.posY = y; m_bindData.posZ = z; m_bindData.mapId = m; m_bindData.zoneId = v;}

    // Talents
    // These functions build a specific type of A9 packet
    uint32 __fastcall BuildCreateUpdateBlockForPlayer( ByteBuffer *data, Player* target );
    virtual void DestroyForPlayer( Player* target, bool anim = false);

    void LoadNamesFromDB(uint32 guid);

    void ClearVendorIndex() { m_vendorIndexSlots.clear(); }
    void AddVendorIndex(uint32 slot, uint32 index) { m_vendorIndexSlots.insert(std::make_pair(slot, index)); }
    uint32 GetVendorIndex(uint32 slot) { if(m_vendorIndexSlots.find(slot) == m_vendorIndexSlots.end()) return 0xFFFFFFFF; return m_vendorIndexSlots.at(slot); }

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

    float GetBaseDodge();
    float GetBaseParry();
    float GetBaseBlock();

    float CalculateCritFromAgilForClassAndLevel(uint32 _class, uint32 _level);
    float SpellHasteRatingBonus;

    bool canCast(SpellEntry *m_spellInfo);
    void SendXPToggleConfirm();
    void SetPlayerStatus(uint8 pStatus) { m_status = pStatus; }
    void CheckPlayerStatus(uint8 pStatus) { if(m_status == pStatus) m_status = NONE; }
    RONIN_INLINE uint8 GetPlayerStatus() { return m_status; }

    RONIN_INLINE float GetBindPositionX() const { return m_bindData.posX; }
    RONIN_INLINE float GetBindPositionY() const { return m_bindData.posY; }
    RONIN_INLINE float GetBindPositionZ() const { return m_bindData.posZ; }
    RONIN_INLINE uint32 GetBindMapId() const { return m_bindData.mapId; }
    RONIN_INLINE uint32 GetBindZoneId() const { return m_bindData.zoneId; }

    void SetShapeShift(uint8 ss);
    uint32 GetSpellForShapeshiftForm(uint8 ss, bool spellchecks = false);
    void SendCinematic(uint32 cinematicid) { GetSession()->OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &cinematicid); };

    // Rest
    void HandleRestedCalculations(bool rest_on);
    uint32 SubtractRestXP(uint32 &amount);
    void AddCalculatedRestXP(uint32 seconds);
    void ApplyPlayerRestState(bool apply);
    void UpdateRestState();
    void SetRestedAreaTrigger(uint32 triggerId);

    void SendMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, int32 Regen);
    void StopMirrorTimer(MirrorTimerType Type);

    // Visible objects
    bool CanSee(WorldObject* obj);
    RONIN_INLINE bool IsVisible(WorldObject* pObj) { return !(m_visibleObjects.find(pObj->GetGUID()) == m_visibleObjects.end()); }

    void ClearInRangeObjects();
    RONIN_INLINE void AddVisibleObject(WorldObject* pObj) { m_visibleObjects.insert(pObj->GetGUID()); }
    RONIN_INLINE void RemoveVisibleObject(WorldObject* pObj) { m_visibleObjects.erase(pObj->GetGUID()); }
    RONIN_INLINE void RemoveVisibleObject(std::set<WoWGuid>::iterator itr) { m_visibleObjects.erase(itr); }
    RONIN_INLINE std::set<WoWGuid>::iterator FindVisible(WorldObject* obj) { return m_visibleObjects.find(obj->GetGUID()); }
    RONIN_INLINE void RemoveIfVisible(uint16 mapId, WorldObject* obj)
    {
        std::set<WoWGuid>::iterator itr = m_visibleObjects.find(obj->GetGUID());
        if(itr == m_visibleObjects.end())
            return;

        m_visibleObjects.erase(obj->GetGUID());
        PushOutOfRange(mapId, obj->GetGUID());
    }

    RONIN_INLINE bool GetVisibility(WorldObject* obj, std::set<WoWGuid>::iterator *itr)
    {
        *itr = m_visibleObjects.find(obj->GetGUID());
        return ((*itr) != m_visibleObjects.end());
    }

    RONIN_INLINE std::set<WoWGuid>::iterator GetVisibleSetBegin() { return m_visibleObjects.begin(); }
    RONIN_INLINE std::set<WoWGuid>::iterator GetVisibleSetEnd() { return m_visibleObjects.end(); }

    // Misc
    void SetDrunk(uint16 value, uint32 itemId = 0);
    void EventHandleSobering();

    void SendInitialLogonPackets();
    void Reset_Spells();
    void Reset_ToLevel1();
    void RetroactiveCompleteQuests();

    void UpdateNearbyGameObjects();
    void UpdateNearbyQuestGivers();
    void EventMassSummonReset() { m_massSummonEnabled = false; }

    uint8 GetLastSwingError() { return m_lastSwingError; }
    void SetLastSwingError(uint8 error) { m_lastSwingError = error; }

    RONIN_INLINE float CalcRating(uint32 index) { return CalcPercentForRating(index, GetUInt32Value(index)); };
    float CalcPercentForRating(uint32 index, uint32 rating);

    void RegeneratePower(bool isinterrupted);
    void RegenerateHealth(bool inCombat);

    void ForceLogout() { GetSession()->LogoutPlayer(); };

    uint32 GetTotalItemLevel();
    uint32 GetAverageItemLevel(bool skipmissing = false);

    void _Warn(const char *message);
    void _Kick();
    void _Disconnect();

    void Kick(uint32 delay = 0, bool hardKick = false);
    void SoftDisconnect();

    GameObject* GetSummonedObject () {return m_SummonedObject;};
    void SetSummonedObject (GameObject* t_SummonedObject) {m_SummonedObject = t_SummonedObject;};

    void ClearCooldownsOnLine(uint32 skill_line, uint32 called_from);
    void ClearCooldownsOnLines(std::set<uint32> skill_lines, uint32 called_from);
    void ResetAllCooldowns();
    void ClearCooldownForSpell(uint32 spell_id);
    PlayerCooldownMap GetCooldownMap(uint8 index = COOLDOWN_TYPE_SPELL) { return m_cooldownMap[index]; };
    bool SpellHasCooldown(uint32 spellid) { return (m_cooldownMap[COOLDOWN_TYPE_SPELL].find(spellid) != m_cooldownMap[COOLDOWN_TYPE_SPELL].end()); };

    void PushOutOfRange(uint16 mapId, WoWGuid guid);
    void PushUpdateBlock(uint16 mapId, ByteBuffer *data, uint32 updatecount);
    void PopPendingUpdates(uint16 mapId);

    void SendProficiency(bool armorProficiency);
    uint32 GetArmorProficiency() { return m_armorProficiency; }
    uint32 GetWeaponProficiency() { return m_weaponProficiency; }
    void AddArmorProficiency(uint32 flag) { m_armorProficiency |= flag; };
    void AddWeaponProficiency(uint32 flag) { m_weaponProficiency |= flag; };

    void StopMirrorTimer(uint32 Type);

    void EventTeleport(uint32 mapid, float x, float y, float z, float o = 0.f);
    void BroadcastMessage(const char* Format, ...);

    RONIN_INLINE void setMyCorpse(Corpse* corpse) { myCorpse = corpse; }
    RONIN_INLINE Corpse* getMyCorpse() { return myCorpse; }

    void _Relocate(uint32 mapid,const LocationVector & v, bool force_new_world, uint32 instance_id);
    bool SafeTeleport(uint32 MapID, uint32 InstanceID, float X, float Y, float Z, float O);
    bool SafeTeleport(uint32 MapID, uint32 InstanceID, LocationVector vec);
    void SafeTeleport(MapInstance* instance, LocationVector vec);
    bool EjectFromInstance();
    void TeleportToHomebind();
    bool raidgrouponlysent;

    void EventSafeTeleport(uint32 MapID, uint32 InstanceID, LocationVector vec)
    {
        SafeTeleport(MapID, InstanceID, vec);
    }

    //! Do this on /pvp off
    RONIN_INLINE void ResetPvPTimer();
    //! Stop the timer for pvp off
    RONIN_INLINE void StopPvPTimer() { RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TIMER); m_pvpTimer = 0; }

    //! Update our pvp area (called when zone changes)
    void UpdatePvPArea();
    //! PvP Toggle (called on /pvp)
    void PvPToggle();

    RONIN_INLINE uint32 LastHonorResetTime() const { return m_lastHonorResetTime; }
    RONIN_INLINE void LastHonorResetTime(uint32 val) { m_lastHonorResetTime = val; }

    /////
    void OnPreSetInWorld();
    void OnPrePushToWorld();
    void OnPushToWorld();
    void OnWorldPortAck();
    void OnWorldLogin();
    void SoftLoadPlayer();
    void CompleteLoading();
    void SendObjectUpdate(WoWGuid guid);

    bool CanSignCharter(Charter * charter, Player* requester);
    RONIN_INLINE void SetName(std::string& name) { m_name = name; }
    void SendAreaTriggerMessage(const char * message, ...);

    // Trade Target
    //Player* getTradeTarget() {return mTradeTarget;};

    RONIN_INLINE Player* GetTradeTarget()
    {
        if(!IsInWorld() || m_tradeData == NULL)
            return NULL;
        return m_mapInstance->GetPlayer(m_tradeData->targetGuid);
    }

    Item* getTradeItem(uint8 slot)
    {
        if(m_tradeData == NULL)
            return NULL;

        return m_tradeData->tradeItems[slot];
    }

    /* Mind Control */
    void Possess(Unit* pTarget);
    void UnPossess();

    void RemoteRevive(bool hp = true)
    {
        ResurrectPlayer();
        m_movementInterface.OnRevive();
        if(hp) SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH) );
    }

    void SummonRequest(WorldObject* Requestor, uint32 ZoneID, uint32 MapID, uint32 InstanceID, const LocationVector & Position);

    void FullHPMP();

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

    uint32 GetLastLoginTime() { return  m_timeLogoff; };

protected:
    void RemovePendingPlayer(uint8 reason = CHAR_LOGIN_NO_CHARACTER);

    /************************************************************************/
    /* SOCIAL                                                               */
    /************************************************************************/
private:

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
    PlayerInfo *getPlayerInfo() const { return m_playerInfo; }
    PlayerCreateInfo *getPlayerCreateInfo() const { return m_createInfo; }

    RONIN_INLINE bool HasKnownTitleByEntry(uint32 entry) { CharTitleEntry *titleEntry = dbcCharTitle.LookupEntry(entry); return (titleEntry && HasKnownTitleByIndex(titleEntry->bit_index)); }

    void SetKnownTitle( int32 title, bool set );

    uint8 GetChatTag() const;
    uint8 GetGuildMemberFlags();
    void AddArenaPoints( uint32 arenapoints );

    // loooooot
    virtual void GenerateLoot();
    uint32 GenerateShapeshiftModelId(uint32 form);
    void StartQuest(uint32 id);
    //Calculate honor for random bgs
    uint32 GenerateRBGReward(uint32 level, uint32 count = 1) { uint32 honor = uint32(ceil(float(count * level * 1.55f))); return honor; }
    static DrunkenState GetDrunkenstateByValue(uint16 value);
    void EventDrunkenVomit();

private:
    Mutex accessLock;
    bool ok_to_remove;

    // Player Name, 21 characters max
    std::string m_name;

    // Stat update
    bool m_forceStatUpdate;

    // Player data
    PlayerInfo *m_playerInfo;
    PlayerCreateInfo *m_createInfo;

    // Character DBC data
    CharRaceEntry *myRace;
    CharClassEntry *myClass;

    // Packet storage stack for pushing data
    LockedQueue<WorldPacket*> m_packetQueue;

    // Pointer to this char's game client
    WorldSession *m_session;

    // Player loading data
    std::vector<std::pair<uint8, float>> m_loadData;

    // Object Update system components
    Mutex _bufferS;
    bool bProcessPending;
    uint32 m_updateDataCount, m_OutOfRangeIdCount;
    ByteBuffer m_updateDataBuff, m_OutOfRangeIds, m_itemUpdateData;

    // Player::Update data
    // Used in player exploration functions
    uint32 m_oldZone, m_oldArea;

    // Player Reputation
    FactionInterface m_factionInterface;

    // Player spell storage
    SpellSet m_spells, m_shapeShiftSpells;
    Loki::AssocVector<uint8, SpellSet> m_spellsByEffect;
    Loki::AssocVector<uint16, SpellSet> m_spellsBySkill;

    // Player skill access data
    Loki::AssocVector<uint16, uint8> m_skillIndexes;

    // Load data
    std::deque<std::pair<uint8, Aura*>> m_loadAuras;

    // Battleground data
    struct BattlegroundData
    {
        bool randomBG;
    } *m_bgData;

    // DK rune data
    struct RuneData
    {
        // Runes
        uint8 m_runemask;
        uint8 m_runes[6];
        uint32 m_runeCD[6];
    } *m_runeData;

    // Player cooldown storage
    PlayerCooldownMap m_cooldownMap[NUM_COOLDOWN_TYPES];
    time_t m_globalCooldown; // Global cooldown

    // Stores player items
    PlayerInventory m_inventory;
    // Stores player currency
    PlayerCurrency m_currency;

    // Pending item updates pushed from interfaces
    std::set<Item*> m_pendingUpdates;
    // Item bonus storage
    ItemBonusModMap itemBonusMap;
    // Mapped by type
    ItemBonusModByType itemBonusMapByType;
    // Stores player talent data, action bars and glyphs
    TalentInterface m_talentInterface;
    // Stores player quest data
    //QuestInterface m_questInterface;

    // Power regeneration data
    std::vector<float> m_regenPowerFraction;
    std::vector<uint32> m_regenTimerCounters;

    // Player social interface
    Mutex m_socialLock; // Use social lock for chat multithread
    // Afk reason sent from client
    std::string m_afk_reason;
    // first is friend guid, second is note
    std::map<WoWGuid, std::string> m_friends, m_ignores, m_mutes;

    // Taxi mask data
    UpdateMask m_taxiMask;
    // Taxi related variables
    struct TaxiData
    {
        void StartPath(uint32 currTime, TaxiPath *path, uint32 modelId, uint32 startOverride, uint32 pathLength)
        {
            CurrentPath = path;
            ModelId = modelId;
            LastUpdateTime = currTime;
            UpdateTimer = 0;
            TravelTime = startOverride;
            MoveTime = pathLength;
            ArrivalTime = pathLength;
        }

        TaxiPath* CurrentPath; // Current taxi path pointer
        uint32 ModelId; // Our path modelId, different models for horde and alliance
        uint32 LastUpdateTime; // Last update timer to cut down on excess time removed
        uint32 UpdateTimer; // our update timer used in player::update to update in blocks
        uint32 TravelTime; // current travel time updated via player::update
        uint32 MoveTime; // time it takes to move to target sent in packet
        uint32 ArrivalTime; // our calculated arrival time, not sent to client but used server side in lue of movetime
        float posX, posY, posZ; // Target position
        std::vector<TaxiPath*> paths; // Pending taxi paths
    } *m_taxiData;

    // Player trade related variables
    struct PlayerTradeData
    {
        // Trade data
        uint64 gold;
        uint32 enchantId;
        WoWGuid targetGuid;
        Item* tradeItems[7];
        PlayerTradeStatus tradeStep;
    } *m_tradeData;

    // Player Game Master data
    struct PlayerGMData
    {
        uint8 gmSightType;
        uint32 gmSightEventID;
        uint32 gmSightPhaseMask;

        bool gmCooldownCheat;
        bool gmCastTimeCheat;
        bool gmTriggerPassCheat;
    } *m_gmData;

    // Player bind positioning data
    struct BindData
    {
        // bind
        uint32 mapId, zoneId;
        float posX, posY, posZ;
    } m_bindData;

    // Rested data
    struct RestData
    {
        bool isResting;
        uint8 restState;
        uint32 restAmount;
        uint32 areaTriggerId;
    } m_restData;

    /// Creature Interaction data
    std::map<uint32, uint32> m_vendorIndexSlots;

public:
    uint8 m_lastSwingError;

    //Quest related variables
    std::vector<QuestLogEntry*> m_questLog;
    uint32 m_questSharer;
    std::set<uint32> quest_spells;
    std::set<uint32> quest_mobs;

    std::map<uint16, uint64> m_completedQuests;
    std::map<uint32, time_t> m_completedRepeatableQuests, m_completedDailyQuests, m_completedWeeklyQuests;

    //Spells variables
    Player* DuelingWith;
    // loot variables
    WoWGuid m_lootGuid;
    WoWGuid m_currentLoot;

    bool m_massSummonEnabled;
    bool bReincarnation;

    // GameObject commands
    WoWGuid m_selectedGo;
    uint32 m_ShapeShifted;

    bool bHasBindDialogOpen;
    uint32 TrackingSpell;

    uint32 m_KickDelay;
    bool m_hardKick;

    Unit* m_CurrentCharm;
    Transporter* m_CurrentTransporter;
    bool IgnoreSpellFocusRequirements;
    uint32 roll;
    bool m_cooldownCheat;
    bool CastTimeCheat;
    bool PowerCheat;

    uint32 m_resurrectHealth, m_resurrectMana;
    uint32 resurrector;
    LocationVector m_resurrectLoction;
    uint32 blinktimer;
    bool blinked;

    Unit* linkTarget;
    /*****************
      PVP Stuff
    ******************/
    uint32 m_pvpTimer;
    uint32 OnlineTime;
    uint32 m_TeleportState;
    bool m_beingPushed;
    bool m_XPoff;
    uint32 iInstanceType;
    uint32 iRaidType;
    // spell to (delay, last time)
    Channel* watchedchannel;

    int32 m_rap_mod_pct;
    Creature* m_tempSummon;
    bool m_deathVision;
    uint32 m_arenateaminviteguid;
    //Current value of Feral Attack Power from items
    int32 m_feralAP;
    Mailbox* m_mailBox;
    bool m_setwaterwalk;
    uint64 m_areaSpiritHealer_guid;
    bool m_finishingmovesdodge;
    uint32 m_timeLogoff;
    LocationVector m_summonPos;
    int32 m_summonInstanceId;
    uint32 m_summonMapId;
    WorldObject* m_summoner;

    /************************************************************************/
    /* Trade                                                                */
    /************************************************************************/

    /************************************************************************/
    /* Player Class systems, info and misc things                           */
    /************************************************************************/

    // Character Ban
    uint32      m_banned;
    std::string m_banreason;
    uint32      m_invitersGuid; // It is guild inviters guid, 0 when its not used

    //Duel
    uint32 m_duelCountdownTimer;
    uint8 m_duelStatus;
    uint8 m_duelState;

    uint32 m_armorProficiency, m_weaponProficiency;
    // STATUS
    uint8 m_status;
    // guid of current selection
    WoWGuid m_curSelection;
    // Raid
    uint8 m_targetIcon;
    // Channels
    std::set<uint32> m_channels;
    std::map<uint32, Channel*> m_channelsbyDBCID;
    // Visible objects
    std::set<WoWGuid> m_visibleObjects;
    // Groups/Raids
    WoWGuid m_GroupInviter;

    // Fishing related
    GameObject* m_SummonedObject;

    // other system
    Corpse* myCorpse;

    uint32 m_lastHonorResetTime;
    uint32 trigger_on_stun;         //bah, warrior talent but this will not get triggered on triggered spells if used on proc so i'm forced to used a special variable
    uint32 trigger_on_stun_chance;  //also using this for mage "Frostbite" talent

    uint8 m_bgTeam;

    std::map<uint32, uint32> m_forcedReactions;

    bool m_passOnLoot;
    bool m_changingMaps;

    bool PreventRes;

    // mage invisibility
    bool m_mageInvisibility;
    uint16 m_drunk;
    uint32 m_drunkTimer;
    bool m_hasSentMoTD;
    float MobXPGainRate;
};
