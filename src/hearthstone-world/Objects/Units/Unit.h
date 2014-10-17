/***
 * Demonstrike Core
 */

#pragma once

#define MAKE_ACTION_BUTTON(A,T) uint32(uint32(A) | (uint32(T) << 24))
#define UF_TARGET_DIED  1
#define UF_ATTACKING    2 // this unit is attacking it's selection
#define SPELL_GROUPS    96
#define SPELL_MODIFIERS 30
#define DIMINISH_GROUPS 13
#define NUM_MECHANIC 32

#define UNIT_TYPE_HUMANOID_BIT (1 << (HUMANOID-1)) //should get computed by precompiler ;)

class Aura;
class Spell;
class AIInterface;
class GameObject;
class Creature;
struct CreatureData;

typedef struct
{
    uint32 spellid;
    uint32 mindmg;
    uint32 maxdmg;
} OnHitSpell;

struct DamageSplitTarget
{
    uint64 m_target; // we store them
    uint32 m_spellId;
    float m_pctDamageSplit; // % of taken damage to transfer (i.e. Soul Link)
    uint32 m_flatDamageSplit; // flat damage to transfer (i.e. Blessing of Sacrifice)
    bool active;
};

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
    JUST_DIED,  // Unit has JUST died
    CORPSE,     // Unit has died but remains in the world as a corpse
    DEAD        // Unit is dead and his corpse is gone from the world
};

#define MAX_POWER_FIELDS 5

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
    POWER_TYPE_ALL          = 127
};

enum SpeedTypes
{
    WALK            = 0,
    RUN             = 1,
    RUNBACK         = 2,
    SWIM            = 3,
    SWIMBACK        = 4,
    TURN            = 5,
    FLY             = 6,
    FLYBACK         = 7,
    PITCH_RATE      = 8
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
    UNIT_FLAG_NOT_ATTACKABLE_2           = 0x2, // 2          2  client won't let you attack them
    UNIT_FLAG_LOCK_PLAYER                = 0x4, // 3          4  ? does nothing to client (probably wrong) - only taxi code checks this
    UNIT_FLAG_PLAYER_CONTROLLED          = 0x8, // 4          8  makes players and NPCs attackable / not attackable
    UNIT_FLAG_UNKNOWN_5                  = 0x10, // 5         16  ? some NPCs have this
    UNIT_FLAG_PREPARATION                = 0x20, // 6         32  don't take reagents for spells with SPELL_ATTR_EX5_NO_REAGENT_WHILE_PREP
    UNIT_FLAG_PLUS_MOB                   = 0x40, // 7         64  ? some NPCs have this (Rare/Elite/Boss?)
    UNIT_FLAG_NOT_ATTACKABLE_1           = 0x80, // 8         128  ? can change attackable status (UNIT_FLAG_PLAYER_CONTROLLED | UNIT_FLAG_NOT_ATTACKABLE_1) is NON_PVP_ATTACKABLE
    UNIT_FLAG_NOT_ATTACKABLE_9           = 0x100, // 9        256  changes attackable status
    UNIT_FLAG_UNKNOWN_10                 = 0x200, // 10       512  3.0.3 - makes you unable to attack everything
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
    MELEE   = 0,
    OFFHAND = 1,
    RANGED  = 2,
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

enum FIELD_PADDING//Since this field isnt used you can expand it for you needs
{
    PADDING_NONE
};

typedef std::list<struct ProcTriggerSpellOnSpell> ProcTriggerSpellOnSpellList;

/************************************************************************/
/* "In-Combat" Handler                                                  */
/************************************************************************/

class Unit;
class SERVER_DECL CombatStatusHandler
{
    typedef set<uint64> AttackerMap;
    typedef map<uint64, uint32> AttackTMap;
    typedef set<uint32> HealedSet;      // Must Be Players!
    Unit* m_Unit;
    bool m_lastStatus;
    HealedSet m_healed;
    HealedSet m_healers;
    AttackerMap m_attackers;
    AttackTMap m_attackTargets;
    uint32 condom[16]; // wear protection, damagemap! don't get access violated!
    map<uint64,uint32> DamageMap;

public:
    CombatStatusHandler() : m_lastStatus(false)
    {
        m_attackers.clear();
        m_healers.clear();
        m_healed.clear();
        m_attackTargets.clear();
        DamageMap.clear();
    }

    Unit* GetKiller();                                                  // Gets this unit's current killer.

    void OnDamageDealt(Unit* pTarget, uint32 damage);                   // this is what puts the other person in combat.
    void WeHealed(Unit* pHealTarget);                                   // called when a player heals another player, regardless of combat state.
    void RemoveAttackTarget(Unit* pTarget);                             // means our DoT expired.
    void ForceRemoveAttacker(const uint64& guid);                       // when target is invalid pointer

    void UpdateFlag();                                                  // detects if we have changed combat state (in/out), and applies the flag.

    HEARTHSTONE_INLINE bool IsInCombat() { return m_lastStatus; }       // checks if we are in combat or not.

    void OnRemoveFromWorld();                                           // called when we are removed from world, kills all references to us.

    void Vanish(uint32 guidLow);

    HEARTHSTONE_INLINE void Vanished()
    {
        ClearAttackers();
        ClearHealers();
        DamageMap.clear();
    }

    bool DidHeal(uint32 guidLow)
    {
        return (m_healed.find(guidLow) != m_healed.end());
    }

    bool HealedBy(uint32 guidLow)
    {
        return (m_healers.find(guidLow) != m_healers.end());
    }

    bool DidDamageTo(uint64 guid)
    {
        return (DamageMap.find(guid) != DamageMap.end());
    }

    HEARTHSTONE_INLINE void SetUnit(Unit* p) { m_Unit = p; }
    void UpdateTargets();

protected:
    bool InternalIsInCombat();                                          // called by UpdateFlag, do not call from anything else!
    bool IsAttacking(Unit* pTarget);                                    // internal function used to determine if we are still attacking target x.
    void RemoveHealed(Unit* pHealTarget);                               // usually called only by updateflag
    void ClearHealers();                                                // this is called on instance change.
    void ClearAttackers();                                              // means we vanished, or died.
    void ClearMyHealers();
};

/************************************************************************/
/* Movement Handler                                                     */
/************************************************************************/
enum MoveFlags : uint32
{
    // Byte 1 (Resets on Movement Key Press)
    MOVEFLAG_MOVE_STOP                  = 0x00,         //verified
    MOVEFLAG_MOVE_FORWARD               = 0x01,         //verified
    MOVEFLAG_MOVE_BACKWARD              = 0x02,         //verified
    MOVEFLAG_STRAFE_LEFT                = 0x04,         //verified
    MOVEFLAG_STRAFE_RIGHT               = 0x08,         //verified
    MOVEFLAG_TURN_LEFT                  = 0x10,         //verified
    MOVEFLAG_TURN_RIGHT                 = 0x20,         //verified
    MOVEFLAG_PITCH_DOWN                 = 0x40,         //Unconfirmed
    MOVEFLAG_PITCH_UP                   = 0x80,         //Unconfirmed

    // Byte 2 (Resets on Situation Change)
    MOVEFLAG_WALK                       = 0x0100,       //verified
    MOVEFLAG_TAXI                       = 0x0200,
    MOVEFLAG_NO_COLLISION               = 0x0400,
    MOVEFLAG_FLYING                     = 0x0800,       //verified
    MOVEFLAG_REDIRECTED                 = 0x1000,       //Jumping
    MOVEFLAG_FALLING                    = 0x2000,       //verified
    MOVEFLAG_FALLING_FAR                = 0x4000,       //verified
    MOVEFLAG_FREE_FALLING               = 0x8000,       //half verified

    // Byte 3 (Set by server. TB = Third Byte. Completely unconfirmed.)
    MOVEFLAG_TB_PENDING_STOP            = 0x010000,     // (MOVEFLAG_PENDING_STOP)
    MOVEFLAG_TB_PENDING_UNSTRAFE        = 0x020000,     // (MOVEFLAG_PENDING_UNSTRAFE)
    MOVEFLAG_TB_PENDING_FALL            = 0x040000,     // (MOVEFLAG_PENDING_FALL)
    MOVEFLAG_TB_PENDING_FORWARD         = 0x080000,     // (MOVEFLAG_PENDING_FORWARD)
    MOVEFLAG_TB_PENDING_BACKWARD        = 0x100000,     // (MOVEFLAG_PENDING_BACKWARD)
    MOVEFLAG_SWIMMING                   = 0x200000,     //  verified
    MOVEFLAG_FLYING_PITCH_UP            = 0x400000,     // (half confirmed)(MOVEFLAG_PENDING_STR_RGHT)
    MOVEFLAG_TB_MOVED                   = 0x800000,     // Send to client on entervehicle

    // Byte 4 (Script Based Flags. Never reset, only turned on or off.)
    MOVEFLAG_AIR_SUSPENSION             = 0x01000000,   // confirmed allow body air suspension(good name? lol).
    MOVEFLAG_AIR_SWIMMING               = 0x02000000,   // confirmed while flying.
    MOVEFLAG_SPLINE_MOVER               = 0x04000000,   // Unconfirmed
    MOVEFLAG_SPLINE_ENABLED             = 0x08000000,
    MOVEFLAG_WATER_WALK                 = 0x10000000,
    MOVEFLAG_FEATHER_FALL               = 0x20000000,   // Does not negate fall damage.

    // Masks
    MOVEFLAG_MOVING_MASK                = MOVEFLAG_MOVE_FORWARD|MOVEFLAG_MOVE_BACKWARD,
    MOVEFLAG_STRAFING_MASK              = MOVEFLAG_STRAFE_LEFT|MOVEFLAG_STRAFE_RIGHT,
    MOVEFLAG_TURNING_MASK               = MOVEFLAG_TURN_LEFT|MOVEFLAG_TURN_RIGHT,
    MOVEFLAG_FALLING_MASK               = 0x6000,
    MOVEFLAG_MOTION_MASK                = 0xE00F,       // Forwards, Backwards, Strafing, Falling
    MOVEFLAG_PENDING_MASK               = 0x7F0000,
    MOVEFLAG_PENDING_STRAFE_MASK        = 0x600000,
    MOVEFLAG_PENDING_MOVE_MASK          = 0x180000,
    MOVEFLAG_FULL_FALLING_MASK          = 0xE000,
};

enum MoveFlags2 : uint16
{
    MOVEFLAG2_NONE                      = 0x0000,
    MOVEFLAG2_NO_STRAFE                 = 0x0001,
    MOVEFLAG2_NO_JUMPING                = 0x0002,
    MOVEFLAG2_UNK3                      = 0x0004, // Overrides various clientside checks
    MOVEFLAG2_FULL_SPEED_TURNING        = 0x0008,
    MOVEFLAG2_FULL_SPEED_PITCHING       = 0x0010,
    MOVEFLAG2_ALWAYS_ALLOW_PITCHING     = 0x0020,
    MOVEFLAG2_UNK7                      = 0x0040,
    MOVEFLAG2_UNK8                      = 0x0080,
    MOVEFLAG2_UNK9                      = 0x0100,
    MOVEFLAG2_UNK10                     = 0x0200,
    MOVEFLAG2_INTERPOLATED_MOVEMENT     = 0x0400,
    MOVEFLAG2_INTERPOLATED_TURNING      = 0x0800,
};

class MovementInfo
{
public:
    MovementInfo() : transGuid(), movementFlags(0), movementFlags2(0), moveTime(0),
        x(0.f), y(0.f), z(0.f), orient(0.f), t_x(0.f), t_y(0.f), t_z(0.f), t_orient(0.f),
        transTime(0), transSeat(0), pitch(0.f), fallTime(0), j_sin(0.f), j_cos(0.f), j_vel(0.f), j_speed(0.f),
        splineAngle(0.f) { };
    ~MovementInfo() { };

    void read(ByteBuffer &data);
    void write(ByteBuffer &data);

    // Position data
    void SetPosition(float newX, float newY, float newZ, float newO = 0.0f) { m_position.ChangeCoords(newX, newY, newZ, newO); };
    void SetPosition(LocationVector loc) { m_position.ChangeCoords(loc.x, loc.y, loc.z, loc.o); };
    void GetPosition(LocationVector &loc) { loc.ChangeCoords(m_position.x, m_position.y, m_position.z, m_position.o); };
    void GetPosition(float &_x, float &_y, float &_z, float &_o) { _x = m_position.x; _y = m_position.y; _z = m_position.z; _o = m_position.o; };
    float GetPositionX() { return m_position.x; } float GetPositionY() { return m_position.y; }
    float GetPositionZ() { return m_position.z; } float GetPositionO() { return m_position.o; }

    // Transport data
    void GetTransportPosition(LocationVector &loc) { loc.x = t_x; loc.y = t_y; loc.z = t_z; loc.o = t_orient; };
    void GetTransportPosition(float &_x, float &_y, float &_z, float &_o) { _x = t_x; _y = t_y; _z = t_z; _o = t_orient; };
    void SetTransportData(uint64 guid, float x, float y, float z, float o, uint8 seat) { transGuid = guid; t_x = x; t_y = y; t_z = z; t_orient = o; transSeat = seat; }
    void ClearTransportData() { transGuid = 0; t_x = t_y = t_z = t_orient = 0.0f; transTime = 0; transSeat = 0; };
    void SetTransportLock(bool locked) { m_lockTransport = locked; }
    bool GetTransportLock() { return m_lockTransport; }
    float GetTPositionX() { return t_x; } float GetTPositionY() { return t_y; }
    float GetTPositionZ() { return t_z; } float GetTPositionO() { return t_orient; }

    bool HasFallData() { return ((movementFlags & MOVEFLAG_FALLING) || fallTime > 0); }
    bool HasPitchFlags() { return ((movementFlags & (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || (movementFlags2 & MOVEFLAG2_ALWAYS_ALLOW_PITCHING)); };

public: // Spline data
    bool HasSplineInfo() { return false; } // Todo
    bool HasElevatedSpline() { return (movementFlags & MOVEFLAG_AIR_SWIMMING); }

public:
    void HandleBreathing(Player* _player, WorldSession * pSession);
    void GetRawPosition(float &_x, float &_y, float &_z, float &_o) { _x = x; _y = y; _z = z; _o = orient; };

public: // Server position
    LocationVector m_position;

    void UpdatePosition() { m_position.ChangeCoords(x, y, z, orient); };

public: // Raw data
    uint32 movementFlags;
    uint16 movementFlags2;
    uint32 moveTime;

private:
    float x, y, z, orient;

public:
    WoWGuid transGuid;

private:
    float t_x, t_y, t_z, t_orient;

public:
    uint32 transTime, transTime2, transTime3;
    uint8 transSeat;
    float pitch;
    uint32 fallTime;
    float j_sin, j_cos, j_vel, j_speed;
    float splineAngle, splineElevation;

private:
    bool m_lockTransport;
};

//====================================================================
//  Unit
//  Base object for Players and Creatures
//====================================================================

class SERVER_DECL Unit : public Object
{
    friend class AIInterface;
public:
    void CombatStatusHandler_UpdateTargets();

    Unit();
    virtual ~Unit ( );
    virtual void Init();
    virtual void Destruct();

    virtual void Update( uint32 time );
    virtual void UpdateFieldValues();
    virtual void ClearFieldUpdateValues();

    virtual bool StatUpdateRequired();
    virtual bool HealthUpdateRequired();
    virtual bool PowerUpdateRequired();
    virtual bool RegenUpdateRequired();
    virtual bool AttackTimeUpdateRequired(uint8 weaponType);
    virtual bool AttackDamageUpdateRequired(uint8 weaponType);
    virtual bool ResUpdateRequired();
    virtual bool APUpdateRequired();
    virtual bool RAPUpdateRequired();

    void UpdateStatValues();
    void UpdateHealthValues();
    void UpdatePowerValues();
    void UpdateRegenValues();
    void UpdateAttackTimeValues();
    void UpdateAttackDamageValues();
    void UpdateResistanceValues();
    void UpdateAttackPowerValues();
    void UpdateRangedAttackPowerValues();
    void UpdatePowerCostValues();
    void UpdateHoverValues();

    bool m_needStatRecalculation;
    bool m_statValuesChanged;
    virtual int32 GetBonusMana() = 0;
    virtual int32 GetBonusHealth() = 0;
    virtual int32 GetBonusStat(uint8 type) = 0;
    virtual int32 GetBaseAttackTime(uint8 weaponType) = 0;
    virtual int32 GetBaseMinDamage(uint8 weaponType) = 0;
    virtual int32 GetBaseMaxDamage(uint8 weaponType) = 0;
    virtual int32 GetBonusAttackPower() = 0;
    virtual int32 GetBonusRangedAttackPower() = 0;
    virtual int32 GetBonusResistance(uint8 school) = 0;

    virtual void OnPushToWorld();
    virtual void RemoveFromWorld(bool free_guid);

    virtual void SetPosition( float newX, float newY, float newZ, float newOrientation );
    virtual void SetPosition( const LocationVector & v) { SetPosition(v.x, v.y, v.z, v.o); }

    virtual void _WriteLivingMovementUpdate(ByteBuffer *bits, ByteBuffer *bytes, Player *target);

    void setAttackTimer(int32 time, bool offhand);
    bool isAttackReady(bool offhand);
    bool __fastcall canReachWithAttack(Unit* pVictim);

    HEARTHSTONE_INLINE void SetDuelWield(bool enabled)
    {
        m_duelWield = enabled;
    }

    /// State flags are server-only flags to help me know when to do stuff, like die, or attack
    HEARTHSTONE_INLINE void addStateFlag(uint32 f) { m_state |= f; };
    HEARTHSTONE_INLINE bool hasStateFlag(uint32 f) { return (m_state & f ? true : false); }
    HEARTHSTONE_INLINE void clearStateFlag(uint32 f) { m_state &= ~f; };

    /// Stats
    uint32 GetStrength() { return GetUInt32Value(UNIT_FIELD_STRENGTH); }
    uint32 GetAgility() { return GetUInt32Value(UNIT_FIELD_AGILITY); }
    uint32 GetStamina() { return GetUInt32Value(UNIT_FIELD_STAMINA); }
    uint32 GetIntellect() { return GetUInt32Value(UNIT_FIELD_INTELLECT); }
    uint32 GetSpirit() { return GetUInt32Value(UNIT_FIELD_SPIRIT); }

    virtual void setLevel(uint32 level);
    HEARTHSTONE_INLINE void setRace(uint8 race) { SetByte(UNIT_FIELD_BYTES_0,0,race); }
    HEARTHSTONE_INLINE void setClass(uint8 class_) { SetByte(UNIT_FIELD_BYTES_0,1, class_ ); }
    HEARTHSTONE_INLINE void setGender(uint8 gender) { SetByte(UNIT_FIELD_BYTES_0,2,gender); }

    UnitBaseStats *baseStats;

    HEARTHSTONE_INLINE uint32 getLevel() { return m_uint32Values[ UNIT_FIELD_LEVEL ]; };
    HEARTHSTONE_INLINE uint8 getRace() { return GetByte(UNIT_FIELD_BYTES_0,0); }
    HEARTHSTONE_INLINE uint8 getClass() { return GetByte(UNIT_FIELD_BYTES_0,1); }
    HEARTHSTONE_INLINE uint32 getClassMask() { return 1 << (getClass() - 1); }
    HEARTHSTONE_INLINE uint32 getRaceMask() { return 1 << (getRace() - 1); }
    HEARTHSTONE_INLINE uint8 getGender() { return GetByte(UNIT_FIELD_BYTES_0,2); }
    HEARTHSTONE_INLINE uint8 getPowerType() { return (GetByte(UNIT_FIELD_BYTES_0, 3));}
    HEARTHSTONE_INLINE uint8 getStandState() { return ((uint8)m_uint32Values[UNIT_FIELD_BYTES_1]); }
    HEARTHSTONE_INLINE uint8 GetShapeShift() { return GetByte(UNIT_FIELD_BYTES_2, 3); }

    HEARTHSTONE_INLINE string GetClassNames(bool FullCaps = false)
    {
        std::string _class = "UNKNOWN";
        switch(getClass()) {
        case 1: { _class = "Warrior"; }break; case 2: { _class = "Paladin"; }break;
        case 3: { _class = "Hunter"; }break; case 4: { _class = "Rogue"; }break;
        case 5: { _class = "Priest"; }break; case 6: { _class = "Deathknight"; }break;
        case 7: { _class = "Shaman"; }break; case 8: { _class = "Mage"; }break;
        case 9: { _class = "Warlock"; }break; case 11: { _class = "Druid"; }break; }
        if(FullCaps)
            HEARTHSTONE_TOUPPER(_class);
        return _class;
    };

    HEARTHSTONE_INLINE bool IsInFeralForm()
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

    HEARTHSTONE_INLINE int32 GetDamageDoneMod(uint8 school);
    HEARTHSTONE_INLINE float GetDamageDonePctMod(uint8 school);
    HEARTHSTONE_INLINE int32 GetHealingDoneMod();
    HEARTHSTONE_INLINE float GetHealingDonePctMod();
    int32 GetHealingTakenMod() { return HealTakenMod; }

    uint32 GetSpellDidHitResult( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability );
    uint32 GetSpellDidHitResult( uint32 index, Unit* pVictim, Spell* pSpell, uint8 &reflectout );
    int32 Strike( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability, uint8 abEffindex, int32 add_damage, int32 pct_dmg_mod, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool proc_extrastrike = false );

    void RemoveExtraStrikeTarget(SpellEntry *spell_info);
    void AddExtraStrikeTarget(SpellEntry *spell_info, uint8 effIndex, uint32 charges);

    int32 CalculateAttackPower();
    int32 CalculateRangedAttackPower();

    HEARTHSTONE_INLINE float GetSize() { return GetFloatValue(OBJECT_FIELD_SCALE_X) * GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS); }

    void CastSpell(Unit* Target, uint32 SpellID, bool triggered, uint32 forcedCastTime = 0);
    void CastSpell(Unit* Target, SpellEntry* Sp, bool triggered, uint32 forcedCastTime = 0);
    void CastSpell(uint64 targetGuid, uint32 SpellID, bool triggered, uint32 forcedCastTime = 0);
    void CastSpell(uint64 targetGuid, SpellEntry* Sp, bool triggered, uint32 forcedCastTime = 0);
    uint8 CastSpellAoF(float x,float y,float z,SpellEntry* Sp, bool triggered, uint32 forcedCastTime = 0);
    void EventCastSpell(Unit* Target, SpellEntry * Sp);

    bool isCasting();
    bool IsInInstance();
    double GetResistanceReducion(Unit* pVictim, uint32 type, float armorReducePct);
    void CalculateResistanceReduction(Unit* pVictim,dealdamage *dmg,SpellEntry* ability, float armorreducepct) ;
    void RegenerateHealth();
    void RegenerateEnergy();
    void RegenerateFocus();
    void RegeneratePower(bool isinterrupted);
    void SendPowerUpdate(EUnitFields powerField = UNIT_END);

    void EventModelChange();
    HEARTHSTONE_INLINE float GetModelHalfSize() { return m_modelhalfsize * m_floatValues[OBJECT_FIELD_SCALE_X]; }

    HEARTHSTONE_INLINE void setHRegenTimer(uint32 time) {m_H_regenTimer = time; }
    HEARTHSTONE_INLINE void setPRegenTimer(uint32 time) {m_P_regenTimer = time; }
    HEARTHSTONE_INLINE void DelayPowerRegeneration(uint32 time) { m_p_DelayTimer = time; }

    void DeMorph();
    void smsg_AttackStart(Unit* pVictim);
    void smsg_AttackStop(Unit* pVictim);
    void smsg_AttackStop(uint64 victimGuid);

    bool IsDazed();
    float CalculateDazeCastChance(Unit* target);

    // Stealth
    HEARTHSTONE_INLINE int32 GetStealthLevel() { return (m_stealthLevel + (getLevel() * 5)); }
    HEARTHSTONE_INLINE int32 GetStealthDetectBonus() { return m_stealthDetectBonus; }
    HEARTHSTONE_INLINE void SetStealth(uint32 id) { m_stealth = id; }
    HEARTHSTONE_INLINE bool InStealth() { return (m_stealth!=0 ? true : false); }
    float detectRange;

    // Invisibility
    HEARTHSTONE_INLINE void SetInvisibility(uint32 id) { m_invisibility = id; }
    HEARTHSTONE_INLINE bool IsInvisible() { return (m_invisible != 0 ? true : false); }
    uint32 m_invisibility;
    bool m_invisible;
    uint8 m_invisFlag;
    int32 m_invisDetect[INVIS_FLAG_TOTAL];

    /************************************************************************/
    /* Stun Immobilize                                                      */
    /************************************************************************/
    uint32 trigger_on_stun;             // second wind warrior talent
    uint32 trigger_on_stun_chance;

    void SetTriggerStunOrImmobilize(uint32 newtrigger,uint32 new_chance);
    void EventStunOrImmobilize();

    // Group XP!
    void GiveGroupXP(Unit* pVictim, Player* PlayerInGroup);

    /// Combat / Death Status
    HEARTHSTONE_INLINE bool isAlive() { return m_deathState == ALIVE; };
    HEARTHSTONE_INLINE bool isDead() { return  m_deathState != ALIVE; };
    HEARTHSTONE_INLINE DeathState getDeathState() { return m_deathState; }
    virtual void SetDeathState(DeathState s);
    void OnDamageTaken();

    bool IsFFAPvPFlagged();
    void SetFFAPvPFlag();
    void RemoveFFAPvPFlag();

    /*******************
    *** Aura start
    ********/
    AuraInterface m_AuraInterface;

    // Quick calls
    HEARTHSTONE_INLINE void AddAura(Aura* aur) { m_AuraInterface.AddAura(aur); };

    // Has
    HEARTHSTONE_INLINE bool HasAura(uint32 spellid) { return m_AuraInterface.HasAura(spellid); };
    HEARTHSTONE_INLINE bool HasAurasOfNameHashWithCaster(uint32 namehash, Unit* caster) { return m_AuraInterface.HasAurasOfNameHashWithCaster(namehash, caster ? caster->GetGUID() : 0); };

    // Remove
    HEARTHSTONE_INLINE void RemoveAura(Aura* aur) { m_AuraInterface.RemoveAura(aur); };
    HEARTHSTONE_INLINE void RemoveAuraBySlot(uint8 slot) { m_AuraInterface.RemoveAuraBySlot(slot); };
    HEARTHSTONE_INLINE bool RemoveAura(uint32 spellid, uint64 guid = 0) { return m_AuraInterface.RemoveAura(spellid, guid); };

    /*******************
    *** Aura end
    ********/

    float CalculateLevelPenalty(SpellEntry* sp);
    void CastSpell(Spell* pSpell);
    void InterruptCurrentSpell();

    //caller is the caster
    int32 GetSpellBonusDamage(Unit* pVictim, SpellEntry *spellInfo, uint8 effIndex, int32 base_dmg, bool healing);

    //guardians are temporary spawn that will inherit master faction and will folow them. Apart from that they have their own mind
    Unit* CreateTemporaryGuardian(uint32 guardian_entry,uint32 duration,float angle, uint32 lvl, uint8 Slot);

    void RemoveSummon(Creature* summon);

    std::map< uint32, std::set<Creature*> > m_Summons;

    uint32 m_addDmgOnce;
    void SummonExpireSlot(uint8 slot); // Empties just slot x.
    void SummonExpireAll(bool clearowner); //Empties all slots (NPC's + GameObjects
    HEARTHSTONE_INLINE void AddSummonToSlot(uint8 slot, Creature* toAdd) { m_Summons[slot].insert(toAdd); };
    void FillSummonList(std::vector<Creature*> &summonList, uint8 summonType);

    uint32 m_ObjectSlots[4];
    uint32 m_triggerSpell;
    uint32 m_triggerDamage;
    uint32 m_canMove;

    uint32 m_lastHauntInitialDamage;

    // Spell Effect Variables
    int32 m_silenced;

    HEARTHSTONE_INLINE void SetOnMeleeSpell(uint32 spell, uint8 cast_number ) { m_meleespell = spell; m_meleespell_cn = cast_number; }
    HEARTHSTONE_INLINE uint32 GetOnMeleeSpell() { return m_meleespell; }
    uint8 GetOnMeleeSpellEcn() { return m_meleespell_cn; }
    void CastOnMeleeSpell();

    // On Aura Remove Procs
    HM_NAMESPACE::hash_map<uint32, onAuraRemove* > m_onAuraRemoveSpells;

    void AddOnAuraRemoveSpell(uint32 NameHash, uint32 procSpell, uint32 procChance, bool procSelf);
    void RemoveOnAuraRemoveSpell(uint32 NameHash);
    void OnAuraRemove(uint32 NameHash, Unit* m_target);

    // Split Damage
    struct DamageSplitTarget m_damageSplitTarget;
    uint32 DoDamageSplitTarget(uint32 res, uint32 school_type, bool melee_dmg);

    // Spell Crit
    float spellcritperc;

    //dummy auras, spell stuff
    HEARTHSTONE_INLINE void AddDummyAura( SpellEntry* sp ) { m_DummyAuras[sp->NameHash] = sp; }
    HEARTHSTONE_INLINE bool HasDummyAura( uint32 namehash ) { return m_DummyAuras[namehash] != NULL; }
    HEARTHSTONE_INLINE SpellEntry* GetDummyAura( uint32 namehash ) { return m_DummyAuras[namehash]; }
    HEARTHSTONE_INLINE void RemoveDummyAura( uint32 namehash ) { m_DummyAuras[namehash] = NULL; }

    // AIInterface
    HEARTHSTONE_INLINE AIInterface *GetAIInterface() { return m_aiInterface; }

    void ReplaceAIInterface(AIInterface *new_interface);

    HEARTHSTONE_INLINE int32 GetThreatModifier() { return m_threatModifyer; }
    HEARTHSTONE_INLINE void ModThreatModifier(int32 mod) { m_threatModifyer += mod; }
    HEARTHSTONE_INLINE int32 GetGeneratedThreatModifier() { return m_generatedThreatModifyer; }
    HEARTHSTONE_INLINE void ModGeneratedThreatModifier(int32 mod) { m_generatedThreatModifyer += mod; }

    // DK:Affect
    HEARTHSTONE_INLINE uint32 IsPacified() { return m_pacified; }
    HEARTHSTONE_INLINE uint32 IsStunned() { return m_stunned; }
    HEARTHSTONE_INLINE uint32 IsFeared() { return m_fearmodifiers; }
    HEARTHSTONE_INLINE uint32 GetResistChanceMod() { return m_resistChance; }
    HEARTHSTONE_INLINE void SetResistChanceMod(uint32 amount) { m_resistChance=amount; }

    HEARTHSTONE_INLINE uint16 HasNoInterrupt() { return m_noInterrupt; }
    bool setDetectRangeMod(uint64 guid, int32 amount);
    void unsetDetectRangeMod(uint64 guid);
    int32 getDetectRangeMod(uint64 guid);
    uint32 Heal(Unit* target,uint32 SpellId, uint32 amount, bool silent = false);
    void Energize(Unit* target,uint32 SpellId, uint32 amount, uint32 type);

    uint32 SchoolCastPrevent[7];

    uint32 AbsorbDamage(Object* Attacker, uint32 School, int32 dmg, SpellEntry * pSpell);//returns amt of absorbed dmg, decreases dmg by absorbed value
    int32 RAPvModifier;
    int32 APvModifier;
    uint64 stalkedby;
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

    //SM
    void SM_FIValue( uint32 modifier, int32* v, uint32* group ) { m_AuraInterface.SM_FIValue(modifier, v, group); }
    void SM_FFValue( uint32 modifier, float* v, uint32* group ) { m_AuraInterface.SM_FFValue(modifier, v, group); }
    void SM_PIValue( uint32 modifier, int32* v, uint32* group ) { m_AuraInterface.SM_PIValue(modifier, v, group); }
    void SM_PFValue( uint32 modifier, float* v, uint32* group ) { m_AuraInterface.SM_PFValue(modifier, v, group); }


    // Multimap used to handle aura 271
    // key is caster GUID and value is a pair of SpellMask pointer and mod value
    typedef tr1::unordered_multimap<uint64, pair<uint32*, int32> > DamageTakenPctModPerCasterType;
    DamageTakenPctModPerCasterType DamageTakenPctModPerCaster;

    //Events
    void Emote (EmoteType emote);
    void EventAddEmote(EmoteType emote, uint32 time);
    void EventAllowCombat(bool allow);
    void EmoteExpire();
    HEARTHSTONE_INLINE void setEmoteState(uint8 emote) { m_emoteState = emote; };
    HEARTHSTONE_INLINE uint32 GetOldEmote() { return m_oldEmote; }
    void EventSummonPetExpire();
    HEARTHSTONE_INLINE void EventAurastateExpire(uint32 aurastateflag) { RemoveFlag(UNIT_FIELD_AURASTATE,aurastateflag); }
    void EventHealthChangeSinceLastUpdate();

    void SetStandState (uint8 standstate);

    HEARTHSTONE_INLINE StandState GetStandState()
    {
        uint32 bytes1 = GetUInt32Value (UNIT_FIELD_BYTES_1);
        return StandState (uint8 (bytes1));
    }

    HEARTHSTONE_INLINE uint32 computeOverkill(uint32 damage)
    {
        uint32 curHealth = GetUInt32Value(UNIT_FIELD_HEALTH);
        return (damage > curHealth) ? damage - curHealth : 0;
    }

    void SendChatMessage(uint8 type, uint32 lang, const char *msg);
    void SendChatMessageToPlayer(uint8 type, uint32 lang, const char *msg, Player* plr);
    void SendChatMessageAlternateEntry(uint32 entry, uint8 type, uint32 lang, const char * msg);
    void RegisterPeriodicChatMessage(uint32 delay, uint32 msgid, std::string message, bool sendnotify);
    void SetFaction(uint32 faction, bool save = true);
    void ResetFaction();

    HEARTHSTONE_INLINE void SetHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_HEALTH,  val); }
    HEARTHSTONE_INLINE void SetMaxHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_MAXHEALTH,  val); }
    HEARTHSTONE_INLINE uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
    HEARTHSTONE_INLINE uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAXHEALTH); }
    HEARTHSTONE_INLINE void ModHealth(int32 val) { ModUnsigned32Value(UNIT_FIELD_HEALTH, val); }
    HEARTHSTONE_INLINE void ModMaxHealth(int32 val) { ModUnsigned32Value(UNIT_FIELD_MAXHEALTH, val); }
    HEARTHSTONE_INLINE int32 GetHealthPct() { return (int32)(GetUInt32Value(UNIT_FIELD_HEALTH) * 100 / std::max(1, (int32)GetUInt32Value(UNIT_FIELD_MAXHEALTH))); }
    HEARTHSTONE_INLINE void SetHealthPct(uint32 val) { if (val>0) SetUInt32Value(UNIT_FIELD_HEALTH,float2int32(val*0.01f*GetUInt32Value(UNIT_FIELD_MAXHEALTH))); }
    HEARTHSTONE_INLINE float GetStat(uint32 stat) const { return float(GetUInt32Value(UNIT_FIELD_STATS+stat)); }

    uint32 m_teleportAckCounter;
    //Vehicle
    bool ExitingVehicle;
    bool ChangingSeats;
    Player* pVehicle;
    bool seatisusable[8];
    Unit* m_passengers[8];
    uint32 m_mountSpell;
    uint32 m_vehicleEntry;
    VehicleSeatEntry* m_vehicleSeats[8];
    HEARTHSTONE_INLINE Unit* GetControllingUnit() { return m_passengers[0]; }
    HEARTHSTONE_INLINE Player* GetControllingPlayer() { return (m_passengers[0] ? m_passengers[0]->IsPlayer() ? TO_PLAYER(m_passengers[0]) : NULL : NULL); }

    HEARTHSTONE_INLINE uint32 GetVehicleEntry() { return m_vehicleEntry; };
    HEARTHSTONE_INLINE void SetVehicleEntry(uint32 entry) { m_vehicleEntry = entry; }

    void RemovePassenger(Unit* passenger);
    int8 GetPassengerSlot(Unit* pPassenger);
    void DeletePassengerData(Unit* pPassenger);
    void ChangeSeats(Unit* pPassenger, uint8 seatid);
    void MoveVehicle(float x, float y, float z, float o);

    HEARTHSTONE_INLINE int8 GetSeatID() { return m_inVehicleSeatId; }
    HEARTHSTONE_INLINE Unit* GetVehicle(bool forcevehicle = false)
    {
        if(m_CurrentVehicle)
            return TO_UNIT(m_CurrentVehicle);
        if(pVehicle && !forcevehicle)
            return TO_UNIT(pVehicle);
        return NULL;
    }

    HEARTHSTONE_INLINE void SetSeatID(int8 seat) { m_inVehicleSeatId = seat; }
    HEARTHSTONE_INLINE void SetVehicle(Unit *v)
    {
        if(v == NULL)
        {
            m_CurrentVehicle = NULL;
            pVehicle = NULL;
            return;
        }

        if(v->IsVehicle())
            m_CurrentVehicle = TO_VEHICLE(v);
        else if(v->IsPlayer())
            pVehicle = TO_PLAYER(v);
    }

    bool CanEnterVehicle(Player * requester);

    //In-Range
    virtual void AddInRangeObject(Object* pObj);
    virtual void OnRemoveInRangeObject(Object* pObj);
    void ClearInRangeSet();

    HEARTHSTONE_INLINE void AddDelayedSpell(Spell* toAdd) { DelayedSpells.insert(toAdd); };
    HEARTHSTONE_INLINE Spell* GetCurrentSpell() { return m_currentSpell; }
    HEARTHSTONE_INLINE void SetCurrentSpell(Spell* cSpell) { m_currentSpell = cSpell; }

    uint32 m_CombatUpdateTimer;

    HEARTHSTONE_INLINE void setcanperry(bool newstatus){can_parry=newstatus;}

    float m_modelhalfsize; // used to calculate if something is in range of this unit

    float DamageDonePctMod[7];

    map<uint32, uint32> HealDoneBonusBySpell;
    int32 HealDoneModPos;
    int32 HealDoneBase;
    float HealDonePctMod;
    int32 HealTakenMod;
    float HealTakenPctMod;

    uint32 SchoolImmunityList[7];
    float SpellCritChanceSchool[7];
    int32 PowerCostMod[7];
    float PowerCostPctMod[7]; // armor penetration & spell penetration
    int32 AttackerCritChanceMod[7];
    uint32 SpellDelayResist[7];

    int32 PctRegenModifier;
    float PctPowerRegenModifier[4];

    // Auras Modifiers
    int32 m_pacified;
    int32 m_interruptRegen;
    int32 m_resistChance;
    int32 m_powerRegenPCT;
    int32 m_stunned;
    int32 m_extraattacks[2];
    int32 m_extrastriketarget;
    int32 m_extrastriketargetc;
    std::list<ExtraStrike*> m_extraStrikeTargets;
    int32 m_fearmodifiers;

    int32 m_noInterrupt;
    int32 m_rooted;
    bool disarmed;
    bool disarmedShield;
    uint64 m_detectRangeGUID[5];
    int32  m_detectRangeMOD[5];
    // Affect Speed
    int32 m_speedModifier;
    int32 m_slowdown;
    float m_maxSpeed;
    map< uint32, int32 > speedReductionMap;
    bool GetSpeedDecrease();
    int32 m_mountedspeedModifier;
    int32 m_flyspeedModifier;
    void UpdateSpeed();
    void EnableFlight();
    void DisableFlight();
    void EventRegainFlight();

    void MoveToWaypoint(uint32 wp_id);
    void PlaySpellVisual(uint64 target, uint32 spellVisual);

    void RemoveStealth();
    void RemoveInvisibility();

    HEARTHSTONE_INLINE void ChangePetTalentPointModifier(bool Increment) { Increment ? m_PetTalentPointModifier++ : m_PetTalentPointModifier-- ; };
    uint32 m_stealth;
    bool m_can_stealth;
    int32 m_modlanguage;

    HEARTHSTONE_INLINE uint32 GetCharmTempVal() { return m_charmtemp; }
    HEARTHSTONE_INLINE void SetCharmTempVal(uint32 val) { m_charmtemp = val; }
    set<uint32> m_SpellList;

    HEARTHSTONE_INLINE void DisableAI() { m_useAI = false; }
    HEARTHSTONE_INLINE void EnableAI() { m_useAI = true; }

    HEARTHSTONE_INLINE bool IsSpiritHealer()
    {
        if(GetEntry() == 6491 || GetEntry() == 13116 || GetEntry() == 13117)
            return true;

        return false;
    }

    HEARTHSTONE_INLINE bool ClassMaskAffect(uint32 *classmask, SpellEntry* testSpell)
    {
        bool cl1 = false, cl2 = false, cl3 = false;

        if( testSpell->SpellGroupType[0] )
        {
            if( classmask[0] & testSpell->SpellGroupType[0] )
                cl1 = true;
        }
        else
            cl1 = true;

        if( testSpell->SpellGroupType[1] )
        {
            if( classmask[1] & testSpell->SpellGroupType[1] )
                cl2 = true;
        }
        else
            cl2 = true;

        if( testSpell->SpellGroupType[2] )
        {
            if( testSpell->SpellGroupType[0] == 0 && testSpell->SpellGroupType[1] == 0 )
            {
                if( classmask[2] & testSpell->SpellGroupType[2] )
                    cl3 = true;
            }
            else
                cl3 = true;
        }
        else
            cl3 = true;

        return cl1 && cl2 && cl3;
    }

    void Root();
    void UnRoot();

    void SetFacing(float newo);//only working if creature is idle

    bool IsPoisoned();

    uint16 m_diminishCount[DIMINISH_GROUPS];
    uint8  m_diminishAuraCount[DIMINISH_GROUPS];
    std::map<uint8, uint16> m_diminishTimer;
    bool   m_diminishActive;

    void SetDiminishTimer(uint32 index);

    SpellEntry * pLastSpell;
    bool bProcInUse;
    bool bInvincible;
    Player* m_redirectSpellPackets;
    void UpdateVisibility();

    //! Is PVP flagged?
    bool IsPvPFlagged();
    void SetPvPFlag();
    //! Removal
    void RemovePvPFlag();

    //solo target auras
    uint32 m_special_state; //flags for special states (stunned,rooted etc)

//  uint32 fearSpell;
    CombatStatusHandler CombatStatus;
    bool m_temp_summon;

    // Redirect Threat shit
    Unit* mThreatRTarget;
    float mThreatRAmount;

    void EventCancelSpell(Spell* ptr);

    /////////////////////////////////////////////////////// Unit properties ///////////////////////////////////////////////////

    HEARTHSTONE_INLINE uint64 GetCharmedUnitGUID() { return GetUInt64Value(UNIT_FIELD_CHARM); }
    HEARTHSTONE_INLINE uint64 GetSummonedUnitGUID() { return GetUInt64Value(UNIT_FIELD_SUMMON); }
    HEARTHSTONE_INLINE uint64 GetSummonedCritterGUID() { return GetUInt64Value(UNIT_FIELD_CRITTER); }
    HEARTHSTONE_INLINE uint64 GetCharmedByGUID() { return GetUInt64Value(UNIT_FIELD_CHARMEDBY); }
    HEARTHSTONE_INLINE uint64 GetSummonedByGUID() { return GetUInt64Value(UNIT_FIELD_SUMMONEDBY); }
    HEARTHSTONE_INLINE uint64 GetCreatedByGUID() { return GetUInt64Value(UNIT_FIELD_CREATEDBY); }
    HEARTHSTONE_INLINE uint64 GetTargetGUID() { return GetUInt64Value(UNIT_FIELD_TARGET); }
    HEARTHSTONE_INLINE uint64 GetChannelSpellTargetGUID() { return GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT); }
    HEARTHSTONE_INLINE uint32 GetChannelSpellId() { return GetUInt32Value(UNIT_CHANNEL_SPELL); }
    HEARTHSTONE_INLINE uint32 GetEquippedItem(uint8 slot) { return GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + slot); }
    HEARTHSTONE_INLINE float GetBoundingRadius() { return GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS); }
    HEARTHSTONE_INLINE float GetCombatReach() { return GetFloatValue(UNIT_FIELD_COMBATREACH); }
    HEARTHSTONE_INLINE uint32 GetDisplayId() { return GetUInt32Value(UNIT_FIELD_DISPLAYID); }
    HEARTHSTONE_INLINE uint32 GetNativeDisplayId() { return GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID); }

    HEARTHSTONE_INLINE void SetCharmedUnitGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CHARM, GUID); }
    HEARTHSTONE_INLINE void SetSummonedUnitGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_SUMMON, GUID); }
    HEARTHSTONE_INLINE void SetSummonedCritterGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CRITTER, GUID); }
    HEARTHSTONE_INLINE void SetCharmedByGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CHARMEDBY, GUID); }
    HEARTHSTONE_INLINE void SetSummonedByGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_SUMMONEDBY, GUID); }
    HEARTHSTONE_INLINE void SetCreatedByGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CREATEDBY, GUID); }
    HEARTHSTONE_INLINE void SetTargetGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_TARGET, GUID); }
    HEARTHSTONE_INLINE void SetChannelSpellTargetGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, GUID); }
    HEARTHSTONE_INLINE void SetChannelSpellId(uint32 SpellId) { SetUInt32Value(UNIT_CHANNEL_SPELL, SpellId); }
    HEARTHSTONE_INLINE void SetEquippedItem(uint8 slot, uint32 id) { SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + slot, id); }
    HEARTHSTONE_INLINE void SetBoundingRadius(float rad) { SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, rad); }
    HEARTHSTONE_INLINE void SetCombatReach(float len) { SetFloatValue(UNIT_FIELD_COMBATREACH, len); }
    HEARTHSTONE_INLINE void SetDisplayId(uint32 id) { SetUInt32Value(UNIT_FIELD_DISPLAYID, id); }
    HEARTHSTONE_INLINE void SetNativeDisplayId(uint32 id) { SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, id); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HEARTHSTONE_INLINE uint32 GetMount() { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
    HEARTHSTONE_INLINE float GetCastSpeedMod() { return GetFloatValue(UNIT_MOD_CAST_SPEED); }
    HEARTHSTONE_INLINE uint32 GetCreatedBySpell() { return GetUInt32Value(UNIT_CREATED_BY_SPELL); }
    HEARTHSTONE_INLINE uint32 GetEmoteState() { return GetUInt32Value(UNIT_NPC_EMOTESTATE); }
    HEARTHSTONE_INLINE uint32 GetStat(uint32 stat) { return GetUInt32Value(UNIT_FIELD_STATS + stat); }
    HEARTHSTONE_INLINE uint32 GetResistance(uint32 type) { return GetUInt32Value(UNIT_FIELD_RESISTANCES + type); }
    HEARTHSTONE_INLINE float GetPowerCostMultiplier(uint32 school) { return GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + school); }

    HEARTHSTONE_INLINE void SetMount(uint32 id) { SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, id); }
    HEARTHSTONE_INLINE void SetCreatedBySpell(uint32 id) { SetUInt32Value(UNIT_CREATED_BY_SPELL, id); }
    HEARTHSTONE_INLINE void SetEmoteState(uint32 id) { SetUInt32Value(UNIT_NPC_EMOTESTATE, id); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HEARTHSTONE_INLINE uint32 GetAttackPower() { return GetUInt32Value(UNIT_FIELD_ATTACK_POWER); }
    HEARTHSTONE_INLINE uint32 GetAttackPowerPositiveMods() { return GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_POS); }
    HEARTHSTONE_INLINE uint32 GetAttackPowerNegativeMods() { return GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_NEG); }
    HEARTHSTONE_INLINE float GetAttackPowerMultiplier() { return GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER); }

    HEARTHSTONE_INLINE void SetAttackPower(uint32 amt) { SetUInt32Value(UNIT_FIELD_ATTACK_POWER, amt); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HEARTHSTONE_INLINE uint32 GetRangedAttackPower() { return GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER); }
    HEARTHSTONE_INLINE uint32 GetRangedAttackPowerPositiveMods() { return GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS); }
    HEARTHSTONE_INLINE uint32 GetRangedAttackPowerNegativeMods() { return GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG); }
    HEARTHSTONE_INLINE float GetRangedAttackPowerMultiplier() { return GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    float m_ignoreArmorPctMaceSpec;
    float m_ignoreArmorPct;

    int32 m_LastSpellManaCost;

    void OnPositionChange();
    void Dismount();
    //  custom functions for scripting
    void SetWeaponDisplayId(uint8 slot, uint32 ItemId);

public: // Movement Info.
    uint64 GetTransportGuid() { return movement_info.transGuid.GetOldGuid(); };

    HEARTHSTONE_INLINE MovementInfo* GetMovementInfo() { return &movement_info; }

    //
    MovementInfo movement_info;

protected:
    LocationVector m_lastAreaPosition;
    uint32 m_AreaUpdateTimer;

public:
    uint32 m_meleespell;
    uint8 m_meleespell_cn;
    void _UpdateSpells(uint32 time);

    uint32 m_H_regenTimer;
    uint32 m_P_regenTimer;
    uint32 m_p_DelayTimer;
    float m_Total_Regen;
    uint32 m_state;      // flags for keeping track of some states
    uint32 m_attackTimer;   // timer for attack
    uint32 m_attackTimer_1;
    bool m_duelWield;

    /// Combat
    DeathState m_deathState;

    // Stealth
    uint32 m_stealthLevel;
    uint32 m_stealthDetectBonus;

    int32 m_manashieldamt;
    SpellEntry * m_manaShieldSpell;

    // DK:pet
    //uint32 m_pet_state;
    //uint32 m_pet_action;
    uint8 m_PetTalentPointModifier;

    // Spell currently casting
    Spell* m_currentSpell;
    std::set<Spell*> DelayedSpells;

    // AI
    AIInterface *m_aiInterface;

    bool m_useAI;
    bool m_spellsbusy;
    bool can_parry;//will be enabled by block spell
    int32 m_threatModifyer;
    int32 m_generatedThreatModifyer;

    // Quest emote
    uint8 m_emoteState;
    uint32 m_oldEmote;

    uint32 m_charmtemp;

    std::map<uint32, SpellEntry*> m_DummyAuras;

public:
    void knockback(int32 basepoint, uint32 miscvalue, bool disengage = false );
    void Teleport(float x, float y, float z, float o, int32 phasemask = 1);
    void SetRedirectThreat(Unit * target, float amount, uint32 Duaration);
    void EventResetRedirectThreat();
    void SetSpeed(uint8 SpeedType, float value);
    void SendHeartBeatMsg( bool toself );
    uint32 GetCreatureType();
    virtual const char* GetName() = 0;
    bool IsSitting();
private:
    uint8 m_inVehicleSeatId;
    Vehicle* m_CurrentVehicle;
};
