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

#define MAKE_ACTION_BUTTON(A,T) uint32(uint32(A) | (uint32(T) << 24))
#define UF_TARGET_DIED  1
#define UF_ATTACKING    2 // this unit is attacking it's selection
#define UF_EVADING      4
#define UF_CORPSE       8

#define SPELL_GROUPS    96
#define SPELL_MODIFIERS 30
#define DIMINISH_GROUPS 13
#define NUM_MECHANIC 32

#define UT_MASK_HUMANOID (1 << (UT_HUMANOID-1)) //should get computed by precompiler ;)

class Aura;
class Spell;
class BaseSpell;
class GameObject;
class Creature;
struct CreatureData;

typedef struct
{
    uint32 spellid;
    uint32 mindmg;
    uint32 maxdmg;
} OnHitSpell;

typedef struct
{
    SpellEntry *spell_info;
    uint32 charges;
    bool deleted;
    uint32 i;
} ExtraStrike;

typedef struct
{
    uint32 spell;
    uint32 chance;
    bool self;
    bool deleted;
} onAuraRemove;

enum DeathState
{
    ALIVE = 0,  // Unit is alive and well
    DEAD,       // Unit is dead and his corpse is gone from the world
    JUST_DIED   // Unit has JUST died
};

enum UnitTeam
{
    TEAM_ALLIANCE = 0,
    TEAM_HORDE = 1,
    TEAM_GUARD = 2,
    TEAM_MONSTER = 3,
    TEAM_NONE
};

static char *teamNames[5] = { "Alliance", "Horde", "Guard", "Monster", "None" };

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

static char *classNames[CLASS_MAX-1] = { "Warrior", "Paladin", "Hunter", "Rogue", "Priest", "Death Knight", "Shaman", "Mage", "Warlock", "", "Druid" };

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
    RACE_MAX = 23,
    RACE_COUNT = 12
};

enum RaceMasks
{
    RACEMASK_HUMAN      = 0x000001,
    RACEMASK_ORC        = 0x000002,
    RACEMASK_DWARF      = 0x000004,
    RACEMASK_NIGHTELF   = 0x000008,
    RACEMASK_UNDEAD     = 0x000010,
    RACEMASK_TAUREN     = 0x000020,
    RACEMASK_GNOME      = 0x000040,
    RACEMASK_TROLL      = 0x000080,
    RACEMASK_GOBLIN     = 0x000100,
    RACEMASK_BLOODELF   = 0x000200,
    RACEMASK_DRAENEI    = 0x000400,
    RACEMASK_WORGEN     = 0x200000,
    RACEMASK_ALL_PLAYABLE   = RACEMASK_HUMAN | RACEMASK_ORC | RACEMASK_DWARF | RACEMASK_NIGHTELF | RACEMASK_UNDEAD | RACEMASK_TAUREN
    | RACEMASK_GNOME | RACEMASK_TROLL | RACEMASK_GOBLIN | RACEMASK_BLOODELF | RACEMASK_DRAENEI | RACEMASK_WORGEN
};

enum PowerType : int8
{
    POWER_TYPE_HEALTH       = -2,
    POWER_TYPE_MANA         = 0,
    POWER_TYPE_RAGE         = 1,
    POWER_TYPE_FOCUS        = 2,
    POWER_TYPE_ENERGY       = 3,
    POWER_TYPE_HAPPINESS    = 4,
    POWER_TYPE_RUNE         = 5,
    POWER_TYPE_RUNIC        = 6,
    POWER_TYPE_SOUL_SHARDS  = 7,
    POWER_TYPE_ECLIPSE      = 8,
    POWER_TYPE_HOLY_POWER   = 9,
    POWER_TYPE_ALTERNATE    = 10,
    POWER_TYPE_MAX          = 11,
    POWER_TYPE_ALL          = 127,
    POWER_FIELDS_MAX        = 5
};

enum StandState
{
    STANDSTATE_STAND            = 0,
    STANDSTATE_SIT              = 1,
    STANDSTATE_SIT_CHAIR        = 2,
    STANDSTATE_SLEEP            = 3,
    STANDSTATE_SIT_LOW_CHAIR    = 4,
    STANDSTATE_SIT_MEDIUM_CHAIR = 5,
    STANDSTATE_SIT_HIGH_CHAIR   = 6,
    STANDSTATE_DEAD             = 7,
    STANDSTATE_KNEEL            = 8,
    STANDSTATE_SUBMERGED        = 9
};

enum UnitSpecialStates
{
    UNIT_STATE_NORMAL    = 0x0,
    UNIT_STATE_DISARMED  = 0X1,
    UNIT_STATE_CHARM     = 0x2,
    UNIT_STATE_FEAR      = 0x4,
    UNIT_STATE_ROOT      = 0x8,
    UNIT_STATE_SLEEP     = 0x10,  // never set
    UNIT_STATE_SNARE     = 0x20,  // never set
    UNIT_STATE_STUN      = 0x40,
    UNIT_STATE_KNOCKOUT  = 0x80,  // not used
    UNIT_STATE_BLEED     = 0x100,  // not used
    UNIT_STATE_POLYMORPH = 0x200,  // not used
    UNIT_STATE_BANISH    = 0x400,  // not used
    UNIT_STATE_CONFUSE   = 0x800,
    UNIT_STATE_PACIFY    = 0x1000,
    UNIT_STATE_SILENCE   = 0x2000,
    UNIT_STATE_FROZEN    = 0x4000,
};

enum UnitFieldBytes1
{
    U_FIELD_BYTES_ANIMATION_FROZEN = 0x01,
};

enum UnitFieldBytes2
{
    U_FIELD_BYTES_FLAG_PVP      = 0x01,
    U_FIELD_BYTES_FLAG_FFA_PVP  = 0x04,
    U_FIELD_BYTES_FLAG_AURAS    = 0x10,
};

enum UnitFieldFlags // UNIT_FIELD_FLAGS #46 - these are client flags
{   //                                     Hex    Bit     Decimal  Comments
    UNIT_FLAG_UNKNOWN_1                  = 0x1, // 1          1
    UNIT_FLAG_NON_ATTACKABLE             = 0x2, // 2          2  client won't let you attack them
    UNIT_FLAG_LOCK_PLAYER                = 0x4, // 3          4  ? does nothing to client (probably wrong) - only taxi code checks this
    UNIT_FLAG_PLAYER_STATUS              = 0x8, // 4          8  makes players and NPCs attackable / not attackable
    UNIT_FLAG_UNKNOWN_5                  = 0x10, // 5         16  ? some NPCs have this
    UNIT_FLAG_PREPARATION                = 0x20, // 6         32  don't take reagents for spells with SPELL_ATTR_EX5_NO_REAGENT_WHILE_PREP
    UNIT_FLAG_PLUS_MOB                   = 0x40, // 7         64  ? some NPCs have this (Rare/Elite/Boss?)
    UNIT_FLAG_NOT_ATTACKABLE_1           = 0x80, // 8         128  ? can change attackable status (UNIT_FLAG_PLAYER_CONTROLLED | UNIT_FLAG_NOT_ATTACKABLE_1) is NON_PVP_ATTACKABLE
    UNIT_FLAG_IGNORE_PC                  = 0x100, // 9        256  disables combat with player characters
    UNIT_FLAG_IGNORE_NPC                 = 0x200, // 10       512  disables combat with non player units
    UNIT_FLAG_LOOTING                    = 0x400, // 11       1024
    UNIT_FLAG_SELF_RES                   = 0x800, // 12       2048  ? some NPCs have this
    UNIT_FLAG_PVP                        = 0x1000, // 13      4096  sets PvP flag
    UNIT_FLAG_SILENCED                   = 0x2000, // 14      8192
    UNIT_FLAG_DEAD                       = 0x4000, // 15      16384  used for special "dead" NPCs like Withered Corpses
    UNIT_FLAG_UNKNOWN_16                 = 0x8000, // 16      32768  ? some NPCs have this
    UNIT_FLAG_ALIVE                      = 0x10000, // 17     65536  ?
    UNIT_FLAG_PACIFIED                   = 0x20000, // 18     131072
    UNIT_FLAG_STUNNED                    = 0x40000, // 19     262144
    UNIT_FLAG_COMBAT                     = 0x80000, // 20     524288  sets combat flag
    UNIT_FLAG_MOUNTED_TAXI               = 0x100000, // 21    1048576  mounted on a taxi
    UNIT_FLAG_DISARMED                   = 0x200000, // 22    2097152  3.0.3, disable melee spells casting..., "Required melee weapon" added to melee spells tooltip.
    UNIT_FLAG_CONFUSED                   = 0x400000, // 23    4194304
    UNIT_FLAG_FLEEING                    = 0x0800000, // 24   8388608  fear
    UNIT_FLAG_PLAYER_CONTROLLED_CREATURE = 0x01000000, // 25  16777216  used in spell Eyes of the Beast for pet
    UNIT_FLAG_NOT_SELECTABLE             = 0x02000000, // 26  33554432  cannot select the unit
    UNIT_FLAG_SKINNABLE                  = 0x04000000, // 27  67108864
    UNIT_FLAG_MOUNT                      = 0x08000000, // 28  134217728  ? was MAKE_CHAR_UNTOUCHABLE (probably wrong), nothing ever set it
    UNIT_FLAG_UNKNOWN_29                 = 0x10000000, // 29  268435456
    UNIT_FLAG_FEIGN_DEATH                = 0x20000000, // 30  536870912
    UNIT_FLAG_SHEATHE                    = 0x40000000, // 31  1073741824  ? was WEAPON_OFF and being used for disarm
    UNIT_FLAG_UNKNOWN_32                 = 0x80000000, // 32  2147483648
};

enum UnitFlags2
{
    UNIT_FLAG2_FEIGN_DEATH              = 0x00000001,
    UNIT_FLAG2_UNK1                     = 0x00000002, // Hide unit model (show only player equip)
    UNIT_FLAG2_COMPREHEND_LANG          = 0x00000008,
    UNIT_FLAG2_FORCE_MOVE               = 0x00000040,
    UNIT_FLAG2_REGENERATE_POWER         = 0x00000800
};

enum UnitDynamicFlags
{
    U_DYN_FLAG_LOOTABLE         = 0x01,
    U_DYN_FLAG_UNIT_TRACKABLE   = 0x02,
    U_DYN_FLAG_TAGGED_BY_OTHER  = 0x04,
    U_DYN_FLAG_TAPPED_BY_PLAYER = 0x08,
    U_DYN_FLAG_PLAYER_INFO      = 0x10,
    U_DYN_FLAG_DEAD             = 0x20,
    U_DYN_FLAG_REFER_A_FRIEND   = 0x40,
    U_DYN_FLAG_TAPPED_BY_ALL_THREAT_LIST = 0x80        // Lua_UnitIsTappedByAllThreatList
};

enum DamageFlags
{
    DAMAGE_FLAG_MELEE           = 1,
    DAMAGE_FLAG_HOLY            = 2,
    DAMAGE_FLAG_FIRE            = 4,
    DAMAGE_FLAG_NATURE          = 8,
    DAMAGE_FLAG_FROST           = 16,
    DAMAGE_FLAG_SHADOW          = 32,
    DAMAGE_FLAG_ARCANE          = 64
};

enum WeaponDamageType // this is NOT the same as SPELL_ENTRY_Spell_Dmg_Type, or Spell::GetType(), or SPELL_ENTRY_School !!
{
    MELEE           = 0,
    OFFHAND         = 1,
    RANGED          = 2,
    RANGED_AUTOSHOT = 3,
    NON_WEAPON
};

enum VisualState
{
    ATTACK = 1,
    DODGE,
    PARRY,
    INTERRUPT,
    BLOCK,
    EVADE,
    IMMUNE,
    DEFLECT
};

enum HitStatus
{
    HITSTATUS_unk               = 0x01,
    HITSTATUS_HITANIMATION      = 0x02,
    HITSTATUS_DUALWIELD         = 0x04,
    HITSTATUS_MISS              = 0x10,
    HITSTATUS_ABSORBED          = 0x20,
    HITSTATUS_ABSORBED2         = 0x40,
    HITSTATUS_RESIST            = 0x80,
    HITSTATUS_RESIST2           = 0x100,
    HITSTATUS_CRICTICAL         = 0x200,
    HITSTATUS_BLOCK             = 0x2000,
    HITSTATUS_CRUSHINGBLOW      = 0x8000,
    HITSTATUS_GLANCING          = 0x10000,
    HITSTATUS_CRUSHING          = 0x20000,
    HITSTATUS_NOACTION          = 0x40000,
    HITSTATUS_SWINGNOHITSOUND   = 0x200000 // as in miss?
};

enum INVIS_FLAG
{
    INVIS_FLAG_NORMAL, // players and units with no special invisibility flags
    INVIS_FLAG_SPIRIT1,
    INVIS_FLAG_SPIRIT2,
    INVIS_FLAG_TRAP,
    INVIS_FLAG_QUEST,
    INVIS_FLAG_GHOST,
    INVIS_FLAG_DRUNK,
    INVIS_FLAG_UNKNOWN7,
    INVIS_FLAG_SHADOWMOON,
    INVIS_FLAG_NETHERSTORM,
    INVIS_FLAG_PHASED,
    INVIS_FLAG_TOTAL
};

enum LimitedMovementFlag
{
	LIMIT_ROOT = 0x0,
	LIMIT_GROUND = 0x1,
	LIMIT_WATER = 0x2,
	LIMIT_AIR = 0x4,
	LIMIT_ANYWHERE = LIMIT_GROUND | LIMIT_WATER | LIMIT_AIR
};

// Threat Redirect Handler
class SERVER_DECL ThreatRedirectHandler
{

};

enum UnitFieldUpdateType
{
    UF_UTYPE_STATS = 1,
    UF_UTYPE_HEALTH,
    UF_UTYPE_POWER,
    UF_UTYPE_REGEN,
    UF_UTYPE_ATTACKTIME,
    UF_UTYPE_RESISTANCE,
    UF_UTYPE_ATTACKPOWER,
    UF_UTYPE_RANGEDATTACKPOWER,
    UF_UTYPE_ATTACKDAMAGE,
    UF_UTYPE_POWERCOST,
    UF_UTYPE_HOVER,
    UF_UTYPE_PLAYERDAMAGEMODS,
    UF_UTYPE_PLAYERRATINGS,
    UF_UTYPE_MOVEMENT,

};

//====================================================================
//  Unit
//  Base object for Players and Creatures
//====================================================================

class SERVER_DECL Unit : public WorldObject
{
public:

    Unit(uint64 guid, uint32 fieldCount = UNIT_END);
    virtual ~Unit ( );
    virtual void Init();
    virtual void Destruct();

    virtual const char* GetName() = 0;

    virtual void Update(uint32 msTime, uint32 uiDiff);
    virtual void UpdateFieldValues();

    uint32 GetData(uint32 );
    void SetData(uint32, uint32);

    void UpdateStatValues();
    void UpdateHealthValues();
    void UpdatePowerValues();
    void UpdateRegenValues();
    void UpdateAttackTimeValues();
    void UpdateAttackDamageValues();
    void UpdateResistanceValues();
    void UpdateAttackPowerValues(std::vector<uint32> modMap);
    void UpdateRangedAttackPowerValues(std::vector<uint32> modMap);
    void UpdatePowerCostValues(std::vector<uint32> modMap);
    void UpdateHoverValues();

    virtual float GetPowerMod() = 0;
    virtual float GetHealthMod() = 0;
    virtual int32 GetBonusMana() = 0;
    virtual int32 GetBonusHealth() = 0;
    virtual int32 GetBonusStat(uint8 type) = 0;
    virtual int32 GetBaseAttackTime(uint8 weaponType) = 0;
    virtual int32 GetBaseMinDamage(uint8 weaponType) = 0;
    virtual int32 GetBaseMaxDamage(uint8 weaponType) = 0;
    virtual int32 GetBonusAttackPower() = 0;
    virtual int32 GetBonusRangedAttackPower() = 0;
    virtual int32 GetBonusResistance(uint8 school) = 0;
    virtual float GetDodgeChance() { return 5.f; }
    virtual float GetHitChance() { return 0.f; }

    virtual void OnPreSetInWorld();
    virtual void OnPrePushToWorld();
    virtual void OnPushToWorld();
    virtual void RemoveFromWorld();
    virtual void EventExploration(MapInstance *instance);

    virtual void SetPosition( float newX, float newY, float newZ, float newOrientation );
    virtual void SetPosition( const LocationVector & v) { SetPosition(v.x, v.y, v.z, v.o); }

    WeaponDamageType GetPreferredAttackType(SpellEntry **sp = NULL);
    bool __fastcall validateAttackTarget(WorldObject *target);
    bool __fastcall calculateAttackRange(WeaponDamageType type, float &minRange, float &maxRange, SpellEntry *sp = NULL);
    bool __fastcall canReachWithAttack(WeaponDamageType attackType, Unit* pVictim, uint32 spellId = 0);

    void resetAttackTimer(uint8 attackType);
    void resetAttackDelay(uint8 typeMask);
    void interruptAttackTimer(int16 delay)
    {
        if(delay < 0)
        {
            delay *= -1;
            if(m_attackInterrupt > delay)
                m_attackInterrupt -= delay;
            else m_attackInterrupt = 0;
        } else if(m_attackInterrupt + delay < 0x7FFF)
            m_attackInterrupt += delay;
        else m_attackInterrupt = 0x7FFF;
    }

    void QueueExtraAttacks(uint32 spellId, uint32 amount);

    float ModDetectedRange(Unit *detector, float base);

    RONIN_INLINE bool ValidateAttackTarget(WoWGuid guid) { return m_attackTarget == guid; }
    RONIN_INLINE void SetDualWield(bool enabled) { m_dualWield = enabled; }

    /// State flags are server-only flags to help me know when to do stuff, like die, or attack
    RONIN_INLINE void addStateFlag(uint32 f) { m_state |= f; };
    RONIN_INLINE bool hasStateFlag(uint32 f) { return (m_state & f ? true : false); }
    RONIN_INLINE void clearStateFlag(uint32 f) { m_state &= ~f; };

    /// Stats
    RONIN_INLINE uint32 GetStrength() { return GetUInt32Value(UNIT_FIELD_STRENGTH); }
    RONIN_INLINE uint32 GetAgility() { return GetUInt32Value(UNIT_FIELD_AGILITY); }
    RONIN_INLINE uint32 GetStamina() { return GetUInt32Value(UNIT_FIELD_STAMINA); }
    RONIN_INLINE uint32 GetIntellect() { return GetUInt32Value(UNIT_FIELD_INTELLECT); }
    RONIN_INLINE uint32 GetSpirit() { return GetUInt32Value(UNIT_FIELD_SPIRIT); }

    RONIN_INLINE uint8 GetTeam() { return m_unitTeam; }
    RONIN_INLINE void SetTeam(uint8 team) { m_unitTeam = team; }
    RONIN_INLINE const char *GetTeamName() { return teamNames[std::min<uint8>(TEAM_NONE, m_unitTeam)]; }

    virtual void setLevel(uint32 level);
    RONIN_INLINE void setRace(uint8 race) { SetByte(UNIT_FIELD_BYTES_0,0,race); }
    RONIN_INLINE void setClass(uint8 class_) { SetByte(UNIT_FIELD_BYTES_0,1, class_ ); }
    RONIN_INLINE void setGender(uint8 gender) { SetByte(UNIT_FIELD_BYTES_0,2,gender); }

    RONIN_INLINE uint32 getLevel() { return GetUInt32Value(UNIT_FIELD_LEVEL); };
    RONIN_INLINE uint8 getRace() { return GetByte(UNIT_FIELD_BYTES_0,0); }
    RONIN_INLINE uint8 getClass() { return GetByte(UNIT_FIELD_BYTES_0,1); }
    RONIN_INLINE uint32 getClassMask() { return 1 << (getClass() - 1); }
    RONIN_INLINE uint32 getRaceMask() { return 1 << (getRace() - 1); }
    RONIN_INLINE uint8 getGender() { return GetByte(UNIT_FIELD_BYTES_0,2); }
    RONIN_INLINE uint8 getPowerType() { return (GetByte(UNIT_FIELD_BYTES_0, 3));}
    RONIN_INLINE uint8 getStandState() { return ((uint8)GetUInt32Value(UNIT_FIELD_BYTES_1)); }
    RONIN_INLINE uint8 GetShapeShift() { return GetByte(UNIT_FIELD_BYTES_2, 3); }

    RONIN_INLINE std::string getClassName(bool FullCaps = false)
    {
        std::string className = classNames[getClass()-1];
        if(FullCaps) RONIN_UTIL::TOUPPER(className);
        return className;
    };

    RONIN_INLINE std::string getRaceName(bool FullCaps = false)
    {
        std::string _race = "UNKNOWN";
        switch(getRace()) {
        case 1: { _race = "Human"; }break; case 2: { _race = "Orc"; }break;
        case 3: { _race = "Dwarf"; }break; case 4: { _race = "Night Elf"; }break;
        case 5: { _race = "Forsaken"; }break; case 6: { _race = "Tauren"; }break;
        case 7: { _race = "Gnome"; }break; case 8: { _race = "Troll"; }break;
        case 9: { _race = "Goblin"; }break; case 10: { _race = "Blood Elf"; }break;
        case 11: { _race = "Draenei"; }break; case 22: { _race = "Worgen"; }break; }
        if(FullCaps)
            RONIN_UTIL::TOUPPER(_race);
        return _race;
    };

    RONIN_INLINE bool IsInFeralForm()
    {
        uint8 s = GetShapeShift();
        // Fight forms that do not use player's weapon
        return ( s == 1 || s == 5 || s == 8 );
    }

    bool GetsDamageBonusFromOwner(uint8 school)
    {
        bool result = false;
        switch(school)
        {
        case 4: // Frost
            result |= (GetEntry() == 510); // WaterElemental
            break;
        }
        return result;
    }

    int32 GetDamageDoneMod(uint8 school);
    float GetDamageDonePctMod(uint8 school);
    int32 GetHealingDoneMod();
    float GetHealingDonePctMod();

    uint32 GetMechanicDispels(uint8 mechanic);
    float GetMechanicResistPCT(uint8 mechanic);
    float GetDamageTakenByMechPCTMod(uint8 mechanic);
    float GetMechanicDurationPctMod(uint8 mechanic);

    uint32 GetDispelImmunity(uint8 dispel);
    float GetDispelResistancesPCT(uint8 dispel);
    int32 GetCreatureRangedAttackPowerMod(uint32 creatureType);
    int32 GetCreatureAttackPowerMod(uint32 creatureType);
    int32 GetRangedDamageTakenMod();
    float GetCritMeleeDamageTakenModPct(uint32 school);
    float GetCritRangedDamageTakenModPct(uint32 school);
    int32 GetDamageTakenMod(uint32 school);
    float GetDamageTakenModPct(uint32 school);
    float GetAreaOfEffectDamageMod();

    bool canWalk();
    bool canSwim();
    bool canFly();

    uint32 GetSpellDidHitResult( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability );
    uint32 GetSpellDidHitResult( Unit* pVictim, BaseSpell* pSpell, float *resistOut, uint8 *reflectout);

    float CalculateAdvantage(Unit *pVictim, float &hitchance, float &dodgechance, float &parrychance, float &blockchance, float *critChance, float *crushingBlow, float *glancingBlow);
    void Strike( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool proc_extrastrike = false );

    int32 CalculateAttackPower();
    int32 CalculateRangedAttackPower();

    RONIN_INLINE float GetSize() { return GetFloatValue(OBJECT_FIELD_SCALE_X) * GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS); }

    bool isCasting() { return m_spellInterface.IsCasting(); }
    SpellInterface *GetSpellInterface() { return &m_spellInterface; }

    bool IsInInstance();
    virtual void RegenerateHealth(bool inCombat) = 0;
    virtual void RegeneratePower(bool isinterrupted) {};
    void RegenerateEnergy();
    void RegenerateFocus();
    void SendPowerUpdate(EUnitFields powerField = UNIT_END);

    void EventModelChange();
    RONIN_INLINE float GetModelHalfSize() { return m_modelhalfsize * GetObjectScale(); }

    RONIN_INLINE void setHRegenTimer(uint32 time) {m_H_regenTimer = time; }
    RONIN_INLINE void setPRegenTimer(uint32 time) {m_P_regenTimer = time; }
    RONIN_INLINE void DelayPowerRegeneration(uint32 time) { m_p_DelayTimer = time; }

    void DeMorph();

    RONIN_INLINE bool IsAttacking() { return !m_attackTarget.empty(); }

    bool UpdateAutoAttackState();
    void EventAttack(Unit *target, WeaponDamageType attackType);
    void EventAttackStart(WoWGuid guid);
    virtual void EventAttackStop();

    void smsg_AttackStart(WoWGuid victimGuid);
    void smsg_AttackStop(WoWGuid victimGuid);
    void smsg_AttackStop(Unit* pVictim) { smsg_AttackStop(pVictim ? pVictim->GetGUID() : 0); }

    bool IsDazed();
    float CalculateDazeCastChance(Unit* target);

    // Invisibility
    RONIN_INLINE bool IsInvisible() { return m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_INVISIBILITY); }

    /************************************************************************/
    /* Stun Immobilize                                                      */
    /************************************************************************/
    void SetTriggerStunOrImmobilize(uint32 newtrigger,uint32 new_chance);
    void EventStunOrImmobilize();

    // Group XP!
    void GiveGroupXP(Unit* pVictim, Player* PlayerInGroup);

    /// Combat / Death Status
    RONIN_INLINE bool isAlive() { return m_deathState == ALIVE; };
    RONIN_INLINE bool isDead() { return  m_deathState != ALIVE; };
    RONIN_INLINE bool isSpirit() { return  m_deathState == DEAD && !hasStateFlag(UF_CORPSE); };
    RONIN_INLINE DeathState getDeathState() { return m_deathState; }
    virtual void SetDeathState(DeathState s);

    /*******************
    *** Aura start
    ********/

    // Quick calls
    RONIN_INLINE void AddAura(Aura* aur, uint8 slot = 0xFF) { m_AuraInterface.AddAura(aur, slot); };
    RONIN_INLINE void RefreshAura(SpellEntry *sp, WoWGuid guid) { }//m_AuraInterface.RefreshAura(spellId, guid); }
    RONIN_INLINE void AddAuraStack(SpellEntry *sp, WoWGuid guid) { }//m_AuraInterface.RefreshAura(spellId, guid); }

    // Has
    RONIN_INLINE bool HasAura(uint32 spellid) { return m_AuraInterface.HasAura(spellid); };
    RONIN_INLINE bool HasAurasOfNameHashWithCaster(uint32 namehash, Unit* caster) { return m_AuraInterface.HasAurasOfNameHashWithCaster(namehash, caster ? caster->GetGUID() : 0); };

    // Remove
    RONIN_INLINE void RemoveAura(Aura* aur) { m_AuraInterface.RemoveAura(aur); };
    RONIN_INLINE void RemoveAuraBySlot(uint8 slot) { m_AuraInterface.RemoveAuraBySlot(slot); };
    RONIN_INLINE bool RemoveAura(uint32 spellid, uint64 guid = 0) { return m_AuraInterface.RemoveAura(spellid, guid); };

    /*******************
    *** Aura end
    ********/

    float CalculateLevelPenalty(SpellEntry* sp);

    //caller is the caster
    int32 GetSpellBonusDamage(Unit* pVictim, SpellEntry *spellInfo, uint8 effIndex, int32 base_dmg, bool healing);

    //guardians are temporary spawn that will inherit master faction and will folow them. Apart from that they have their own mind
    Unit* CreateTemporaryGuardian(uint32 guardian_entry,uint32 duration,float angle, uint32 lvl, uint8 Slot);

    void RemoveSummon(Creature* summon);

    void SummonExpireSlot(uint8 slot); // Empties just slot x.
    void SummonExpireAll(bool clearowner); //Empties all slots (NPC's + GameObjects
    RONIN_INLINE void AddSummonToSlot(uint8 slot, Creature* toAdd) { m_Summons[slot].push_back(toAdd); };
    void FillSummonList(std::vector<Creature*> &summonList, uint8 summonType);

    uint32 GetVehicleKitId() const { return m_vehicleKitId; }
    void InitVehicleKit(uint32 vehicleKitId);
    uint16 GetAIAnimKitId() const { return m_aiAnimKitId; }
    void SetAIAnimKitId(uint16 animKitId);
    uint16 GetMovementAnimKitId() const { return m_movementAnimKitId; }
    void SetMovementAnimKitId(uint16 animKitId);
    uint16 GetMeleeAnimKitId() const { return m_meleeAnimKitId; }
    void SetMeleeAnimKitId(uint16 animKitId);

    void AddOnAuraRemoveSpell(uint32 NameHash, uint32 procSpell, uint32 procChance, bool procSelf);
    void RemoveOnAuraRemoveSpell(uint32 NameHash);
    void OnAuraRemove(uint32 NameHash, Unit* m_target);

    //dummy auras, spell stuff
    RONIN_INLINE void AddDummyAura( SpellEntry* sp ) { m_DummyAuras[sp->NameHash] = sp; }
    RONIN_INLINE bool HasDummyAura( uint32 namehash ) { return (m_DummyAuras.find(namehash) != m_DummyAuras.end() && m_DummyAuras[namehash] != NULL); }
    RONIN_INLINE SpellEntry* GetDummyAura( uint32 namehash ) { if(m_DummyAuras.find(namehash) == m_DummyAuras.end()) return NULL; return m_DummyAuras[namehash]; }
    RONIN_INLINE void RemoveDummyAura( uint32 namehash ) { m_DummyAuras[namehash] = NULL; m_DummyAuras.erase(namehash); }

    // DK:Affect
    RONIN_INLINE bool IsPacified() { return m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_PACIFY); }
    RONIN_INLINE bool IsStunned() { return m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_STUN); }
    RONIN_INLINE bool IsRooted() { return m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_ROOT); }
    RONIN_INLINE bool IsFeared() { return m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_FEAR); }
    RONIN_INLINE bool IsStealthed() { return m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_STEALTH); }
    RONIN_INLINE bool HasNoInterrupt() { return m_noInterrupt; }

    uint32 Heal(Unit* target, uint32 amount ,uint32 SpellId, bool silent = false);
    void Energize(Unit* target,uint32 SpellId, uint32 amount, uint32 type);

    uint32 AbsorbDamage(WorldObject* Attacker, uint32 School, int32 dmg, SpellEntry * pSpell);//returns amt of absorbed dmg, decreases dmg by absorbed value

    //SM
    void SM_FIValue( uint32 modifier, int32* v, uint32* group ) { m_AuraInterface.SM_FIValue(modifier, v, group); }
    void SM_FFValue( uint32 modifier, float* v, uint32* group ) { m_AuraInterface.SM_FFValue(modifier, v, group); }
    void SM_PIValue( uint32 modifier, int32* v, uint32* group ) { m_AuraInterface.SM_PIValue(modifier, v, group); }
    void SM_PFValue( uint32 modifier, float* v, uint32* group ) { m_AuraInterface.SM_PFValue(modifier, v, group); }

    //Events
    void Emote (EmoteType emote);
    void EventAddEmote(EmoteType emote, uint32 time);
    void EventAllowCombat(bool allow);
    void EmoteExpire();
    RONIN_INLINE void setEmoteState(uint8 emote) { m_emoteState = emote; };
    RONIN_INLINE uint32 GetOldEmote() { return m_oldEmote; }
    void EventSummonPetExpire();
    RONIN_INLINE void EventAurastateExpire(uint32 aurastateflag) { RemoveFlag(UNIT_FIELD_AURASTATE,aurastateflag); }
    void EventHealthChangeSinceLastUpdate();

    void SetStandState (uint8 standstate);

    RONIN_INLINE StandState GetStandState()
    {
        uint32 bytes1 = GetUInt32Value (UNIT_FIELD_BYTES_1);
        return StandState (uint8 (bytes1));
    }

    RONIN_INLINE uint32 computeOverkill(uint32 damage)
    {
        uint32 curHealth = GetUInt32Value(UNIT_FIELD_HEALTH);
        return (damage > curHealth) ? damage - curHealth : 0;
    }

    void SendChatMessage(uint8 type, uint32 lang, const char *msg);
    void SendChatMessageToPlayer(uint8 type, uint32 lang, const char *msg, Player* plr);
    void SendChatMessageAlternateEntry(uint32 entry, uint8 type, uint32 lang, const char * msg);
    void RegisterPeriodicChatMessage(uint32 delay, uint32 msgid, std::string message, bool sendnotify);

    bool IsFactionNonHostile();
    bool IsFactionNPCHostile();
    void SetFaction(uint32 faction, bool save = true);
    void ResetFaction();

    RONIN_INLINE void SetHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_HEALTH,  val); }
    RONIN_INLINE void SetMaxHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_MAXHEALTH,  val); }
    RONIN_INLINE uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
    RONIN_INLINE uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAXHEALTH); }
    RONIN_INLINE void ModHealth(int32 val) { ModUnsigned32Value(UNIT_FIELD_HEALTH, val); }
    RONIN_INLINE void ModMaxHealth(int32 val) { ModUnsigned32Value(UNIT_FIELD_MAXHEALTH, val); }
    RONIN_INLINE int32 GetHealthPct() { return (int32)(GetUInt32Value(UNIT_FIELD_HEALTH) * 100 / std::max(1, (int32)GetUInt32Value(UNIT_FIELD_MAXHEALTH))); }
    RONIN_INLINE void SetHealthPct(uint32 val) { if (val>0) SetUInt32Value(UNIT_FIELD_HEALTH,float2int32(val*0.01f*GetUInt32Value(UNIT_FIELD_MAXHEALTH))); }
    RONIN_INLINE float GetStat(uint32 stat) const { return float(GetUInt32Value(UNIT_FIELD_STATS+stat)); }

    // Full HP/MP checks
    RONIN_INLINE bool isFullHealth() { return m_uint32Values[UNIT_FIELD_HEALTH] == m_uint32Values[UNIT_FIELD_MAXHEALTH]; }

    void PlaySpellVisual(uint64 target, uint32 spellVisual);
    void SendPlaySpellVisualKit(uint32 id, uint32 unkParam);

    RONIN_INLINE bool IsSpiritHealer()
    {
        if(GetEntry() == 6491 || GetEntry() == 13116 || GetEntry() == 13117)
            return true;

        return false;
    }

    RONIN_INLINE bool ClassMaskAffect(uint32 *classmask, SpellEntry* testSpell)
    {
        bool cl1 = false, cl2 = false, cl3 = false;

        if( testSpell->SpellGroupType[0] )
        {
            if( classmask[0] & testSpell->SpellGroupType[0] )
                cl1 = true;
        } else cl1 = true;

        if( testSpell->SpellGroupType[1] )
        {
            if( classmask[1] & testSpell->SpellGroupType[1] )
                cl2 = true;
        } else cl2 = true;

        if( testSpell->SpellGroupType[2] )
        {
            if( testSpell->SpellGroupType[0] == 0 && testSpell->SpellGroupType[1] == 0 )
            {
                if( classmask[2] & testSpell->SpellGroupType[2] )
                    cl3 = true;
            } else cl3 = true;
        } else cl3 = true;

        return cl1 && cl2 && cl3;
    }

    bool IsPoisoned();

    void SetDiminishTimer(uint32 index);

    void UpdateVisibility();

    //! Is unit stunned
    void SetUnitStunned(bool state);

    //! Is PVP flagged?
    bool IsPvPFlagged();
    void SetPvPFlag();
    //! Removal
    void RemovePvPFlag();

    bool IsFFAPvPFlagged();
    void SetFFAPvPFlag();
    void RemoveFFAPvPFlag();

    void SetInCombat(Unit *unit, uint32 timerOverride = 5000);
    bool IsInCombat() { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT); }

    /////////////////////////////////////////////////////// Unit properties ///////////////////////////////////////////////////

    RONIN_INLINE uint64 GetCharmedUnitGUID() { return GetUInt64Value(UNIT_FIELD_CHARM); }
    RONIN_INLINE uint64 GetSummonedUnitGUID() { return GetUInt64Value(UNIT_FIELD_SUMMON); }
    RONIN_INLINE uint64 GetSummonedCritterGUID() { return GetUInt64Value(UNIT_FIELD_CRITTER); }
    RONIN_INLINE uint64 GetCharmedByGUID() { return GetUInt64Value(UNIT_FIELD_CHARMEDBY); }
    RONIN_INLINE uint64 GetSummonedByGUID() { return GetUInt64Value(UNIT_FIELD_SUMMONEDBY); }
    RONIN_INLINE uint64 GetCreatedByGUID() { return GetUInt64Value(UNIT_FIELD_CREATEDBY); }
    RONIN_INLINE uint64 GetTargetGUID() { return GetUInt64Value(UNIT_FIELD_TARGET); }
    RONIN_INLINE uint64 GetChannelSpellTargetGUID() { return GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT); }
    RONIN_INLINE uint32 GetChannelSpellId() { return GetUInt32Value(UNIT_CHANNEL_SPELL); }
    RONIN_INLINE uint32 GetEquippedItem(uint8 slot) { return GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + slot); }
    RONIN_INLINE float GetBoundingRadius() { return GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS); }
    RONIN_INLINE float GetCombatReach() { return GetFloatValue(UNIT_FIELD_COMBATREACH); }
    RONIN_INLINE uint32 GetDisplayId() { return GetUInt32Value(UNIT_FIELD_DISPLAYID); }
    RONIN_INLINE uint32 GetNativeDisplayId() { return GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID); }

    RONIN_INLINE void SetCharmedUnitGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CHARM, GUID); }
    RONIN_INLINE void SetSummonedUnitGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_SUMMON, GUID); }
    RONIN_INLINE void SetSummonedCritterGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CRITTER, GUID); }
    RONIN_INLINE void SetCharmedByGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CHARMEDBY, GUID); }
    RONIN_INLINE void SetSummonedByGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_SUMMONEDBY, GUID); }
    RONIN_INLINE void SetCreatedByGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CREATEDBY, GUID); }
    RONIN_INLINE void SetTargetGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_TARGET, GUID); }
    RONIN_INLINE void SetChannelSpellTargetGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, GUID); }
    RONIN_INLINE void SetChannelSpellId(uint32 SpellId) { SetUInt32Value(UNIT_CHANNEL_SPELL, SpellId); }
    RONIN_INLINE void SetEquippedItem(uint8 slot, uint32 id) { SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + slot, id); }
    RONIN_INLINE void SetBoundingRadius(float rad) { SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, rad); }
    RONIN_INLINE void SetCombatReach(float len) { SetFloatValue(UNIT_FIELD_COMBATREACH, len); }
    RONIN_INLINE void SetDisplayId(uint32 id) { SetUInt32Value(UNIT_FIELD_DISPLAYID, id); }
    RONIN_INLINE void SetNativeDisplayId(uint32 id) { SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, id); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    RONIN_INLINE uint32 GetMount() { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
    RONIN_INLINE float GetCastSpeedMod() { return GetFloatValue(UNIT_MOD_CAST_SPEED); }
    RONIN_INLINE uint32 GetCreatedBySpell() { return GetUInt32Value(UNIT_CREATED_BY_SPELL); }
    RONIN_INLINE uint32 GetEmoteState() { return GetUInt32Value(UNIT_NPC_EMOTESTATE); }
    RONIN_INLINE uint32 GetStat(uint32 stat) { return GetUInt32Value(UNIT_FIELD_STATS + stat); }
    RONIN_INLINE uint32 GetResistance(uint32 type) { return GetUInt32Value(UNIT_FIELD_RESISTANCES + type); }
    RONIN_INLINE float GetPowerCostMultiplier(uint32 school) { return GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + school); }

    RONIN_INLINE void SetMount(uint32 id) { SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, id); }
    RONIN_INLINE void SetCreatedBySpell(uint32 id) { SetUInt32Value(UNIT_CREATED_BY_SPELL, id); }
    RONIN_INLINE void SetEmoteState(uint32 id) { SetUInt32Value(UNIT_NPC_EMOTESTATE, id); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    RONIN_INLINE uint32 GetAttackPower() { return GetUInt32Value(UNIT_FIELD_ATTACK_POWER); }
    RONIN_INLINE uint32 GetAttackPowerPositiveMods() { return GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_POS); }
    RONIN_INLINE uint32 GetAttackPowerNegativeMods() { return GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_NEG); }
    RONIN_INLINE float GetAttackPowerMultiplier() { return GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER); }

    RONIN_INLINE void SetAttackPower(uint32 amt) { SetUInt32Value(UNIT_FIELD_ATTACK_POWER, amt); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    RONIN_INLINE uint32 GetRangedAttackPower() { return GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER); }
    RONIN_INLINE uint32 GetRangedAttackPowerPositiveMods() { return GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS); }
    RONIN_INLINE uint32 GetRangedAttackPowerNegativeMods() { return GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG); }
    RONIN_INLINE float GetRangedAttackPowerMultiplier() { return GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    uint16 GetFieldPctLg(EUnitFields field, EUnitFields maxField);
    int32 GetPowerPct(EUnitFields field, EUnitFields maxField);
    void SetPowerType(uint8 type);
    int32 GetPowerPct(uint8 power);
    void LosePower(uint8 powerType, int32 value);

    uint32 GetPower(uint8 type);
    uint32 GetMaxPower(uint8 type);
    void ModPower(uint8 type, int32 value);
    void SetPower(uint8 type, uint32 value);
    void SetMaxPower(uint8 type, uint32 value);

    uint32 GetPower(EUnitFields powerField);
    uint32 GetMaxPower(EUnitFields powerField);
    void ModPower(EUnitFields powerField, int32 value);
    void SetPower(EUnitFields powerField, uint32 value);
    void SetMaxPower(EUnitFields powerField, uint32 value);

    EUnitFields GetPowerFieldForType(uint8 type);
    EUnitFields GetMaxPowerFieldForType(uint8 type);

    void Dismount();
    void SetWeaponDisplayId(uint8 slot, uint32 ItemId);

    LocationVector GetCombatEnterLoc() { return m_position; };

public:
    void knockback(int32 basepoint, uint32 miscvalue, bool disengage = false );
    void Teleport(float x, float y, float z, float o);
    void SetRedirectThreat(Unit *target, float amount, uint32 Duaration);
    void EventResetRedirectThreat();
    virtual uint32 GetCreatureType();

public:
    MovementInterface *GetMovementInterface() { return &m_movementInterface; }

    bool IsSitting();
    void SetFacing(float o) { m_movementInterface.SetFacing(o); }
    void MoveTo(float x, float y, float z, float o) { m_movementInterface.MoveTo(x, y, z, o); }
    float GetMoveSpeed(MovementSpeedTypes type) { return m_movementInterface.GetMoveSpeed(type); }

    WoWGuid GetTransportGuid() { return m_movementInterface.GetTransportGuid(); }

public:
    virtual void OnAuraModChanged(uint32 modType);
    void ProcessModUpdate(uint8 modUpdateType, std::vector<uint32> modMap);

    // We can trigger an allocation using [] so we just check empty
    void TriggerModUpdate(uint8 modUpdateType) { m_modQueuedModUpdates[modUpdateType].empty(); }
    Loki::AssocVector<uint8, std::vector<uint32> > m_modQueuedModUpdates;

    AuraInterface m_AuraInterface;
    UnitBaseStats *baseStats;

    uint16 _stunStateCounter;

    float detectRange;

    uint8 m_invisFlag;
    int32 m_invisDetect[INVIS_FLAG_TOTAL];

    Loki::AssocVector< uint32, std::vector<Creature*> > m_Summons;

    WoWGuid m_ObjectSlots[4];
    uint32 m_triggerSpell;
    uint32 m_triggerDamage;
    uint32 m_canMove;

    // Spell Effect Variables
    int32 m_silenced;

    // On Aura Remove Procs
    Loki::AssocVector<uint32, onAuraRemove* > m_onAuraRemoveSpells;

    uint32 m_uAckCounter;

    float m_modelhalfsize; // used to calculate if something is in range of this unit

    // Unit team
    uint8 m_unitTeam;

    // Auras Modifiers
    int32 m_interruptRegen;
    int32 m_powerRegenPCT;
    bool m_noInterrupt;
    bool disarmed, disarmedShield;

    WoWGuid m_killer;
    uint32 m_combatStopTimer;

protected:
    MovementInterface m_movementInterface;

public:
    uint32 m_H_regenTimer, m_P_regenTimer, m_p_DelayTimer;
    uint32 m_state;      // flags for keeping track of some states

    WoWGuid m_attackTarget;
    bool m_dualWield, m_autoShot;
    uint32 m_attackUpdateTimer;
    uint16 m_attackInterrupt, m_attackTimer[2], m_attackDelay[2];
    std::vector<uint32> m_extraAttacks;
    SpellEntry *m_autoShotSpell;

    /// Combat
    DeathState m_deathState;

    SpellInterface m_spellInterface;

    // Quest emote
    uint8 m_emoteState;
    uint32 m_oldEmote;

    Loki::AssocVector<uint32, SpellEntry*> m_DummyAuras;

    uint32 m_vehicleKitId;
    uint16 m_aiAnimKitId;
    uint16 m_movementAnimKitId;
    uint16 m_meleeAnimKitId;
};
