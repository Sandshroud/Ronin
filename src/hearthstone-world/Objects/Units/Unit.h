/***
 * Demonstrike Core
 */

#pragma once

class AIInterface;

#define MAX_POSITIVE_AURAS 48 // ?
#define MAX_AURAS 80 // 48 buff slots, 32 debuff slots.
#define MAX_PASSIVE_AURAS 165
#define TOTAL_AURAS MAX_AURAS+MAX_PASSIVE_AURAS

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
struct CreatureInfo;

struct ReflectSpellSchool
{
    uint32 spellId;
    int32 school;
    int32 chance;
    int32 require_aura_hash;
    bool infinity;
    bool infront;
};

typedef struct
{
    uint32 spellid;
    uint64 caster;//not yet in use
    int32 amt;
    int32 reflect_pct;
}Absorb;

typedef std::list<Absorb*> SchoolAbsorb;

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
    uint8 damage_type; // bitwise 0-127 thingy
    bool active;
};

typedef struct
{
    SpellEntry *spell_info;
    uint32 charges;
    bool deleted;
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

enum PowerType
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
    MAX_POWER_TYPE          = 10,
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

enum VehiclePowerType
{
    POWER_TYPE_PYRITE   = 41,
    POWER_TYPE_STEAM    = 61,
    POWER_TYPE_HEAT     = 101,
    POWER_TYPE_OOZ      = 121,
    POWER_TYPE_BLOOD    = 141,
    POWER_TYPE_WRATH    = 142,
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

enum CUSTOM_TIMERS
{
    CUSTOM_TIMER_CHIMERA_SCORPID,
    CUSTOM_TIMER_ECLIPSE,
    CUSTOM_TIMER_ERADICATION,
    CUSTOM_TIMER_CHEATDEATH,
    CUSTOM_TIMER_RAPTURE,
    NUM_CUSTOM_TIMERS
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

    // Transport data
    void GetTransportPosition(LocationVector &loc) { loc.x = t_x; loc.y = t_y; loc.z = t_z; loc.o = t_orient; };
    void GetTransportPosition(float &_x, float &_y, float &_z, float &_o) { _x = t_x; _y = t_y; _z = t_z; _o = t_orient; };
    void SetTransportData(uint64 guid, float x, float y, float z, float o, uint8 seat) { transGuid.Init(guid); t_x = x; t_y = y; t_z = z; t_orient = o; transSeat = seat; }
    void ClearTransportData() { transGuid.Clear(); t_x = t_y = t_z = t_orient = 0.0f; transTime = 0; transSeat = 0; };
    void SetTransportLock(bool locked) { m_lockTransport = locked; }
    bool GetTransportLock() { return m_lockTransport; }

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
    uint32 transTime, transTime2;
    uint8 transSeat;
    float pitch;
    uint32 fallTime;
    float j_sin, j_cos, j_vel, j_speed;
    float splineAngle;

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

    virtual void OnPushToWorld();
    virtual void RemoveFromWorld(bool free_guid);

    virtual void SetPosition( float newX, float newY, float newZ, float newOrientation );
    virtual void SetPosition( const LocationVector & v) { SetPosition(v.x, v.y, v.z, v.o); }

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
    HEARTHSTONE_INLINE void setLevel(uint32 level) { SetUInt32Value(UNIT_FIELD_LEVEL, level); };
    HEARTHSTONE_INLINE uint32 getLevel() { return m_uint32Values[ UNIT_FIELD_LEVEL ]; };
    HEARTHSTONE_INLINE uint8 getRace() { return GetByte(UNIT_FIELD_BYTES_0,0); }
    HEARTHSTONE_INLINE uint8 getClass() { return GetByte(UNIT_FIELD_BYTES_0,1); }
    HEARTHSTONE_INLINE void setRace(uint8 race) { SetByte(UNIT_FIELD_BYTES_0,0,race); }
    HEARTHSTONE_INLINE void setClass(uint8 class_) { SetByte(UNIT_FIELD_BYTES_0,1, class_ ); }
    HEARTHSTONE_INLINE uint32 getClassMask() { return 1 << (getClass() - 1); }
    HEARTHSTONE_INLINE uint32 getRaceMask() { return 1 << (getRace() - 1); }
    HEARTHSTONE_INLINE uint8 getGender() { return GetByte(UNIT_FIELD_BYTES_0,2); }
    HEARTHSTONE_INLINE void setGender(uint8 gender) { SetByte(UNIT_FIELD_BYTES_0,2,gender); }
    HEARTHSTONE_INLINE uint8 getStandState() { return ((uint8)m_uint32Values[UNIT_FIELD_BYTES_1]); }
    HEARTHSTONE_INLINE char* GetClassNamec(bool FullCaps = false)
    {
        char* _class = "UNKNOWN";
        switch(getClass()) {
        case 1: { _class = "Warrior"; }break; case 2: { _class = "Paladin"; }break;
        case 3: { _class = "Hunter"; }break; case 4: { _class = "Rogue"; }break;
        case 5: { _class = "Priest"; }break; case 6: { _class = "Deathknight"; }break;
        case 7: { _class = "Shaman"; }break; case 8: { _class = "Mage"; }break;
        case 9: { _class = "Warlock"; }break; case 11: { _class = "Druid"; }break; }
        if(FullCaps)
            for(size_t i = 0; i < strlen(_class); ++i)
                _class[i] = (char)toupper(_class[i]);
        return _class;
    };

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


    uint32 GetSpellDidHitResult( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability );
    uint32 GetSpellDidHitResult( uint32 index, Unit* pVictim, Spell* pSpell, uint8 &reflectout );
    int32 Strike( Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability, int32 add_damage, int32 pct_dmg_mod, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool proc_extrastrike = false );

    uint32 m_procCounter, m_procOverspill;
    uint32 HandleProc(uint32 flag, uint32 flag2, Unit* victim, SpellEntry* CastingSpell, int32 dmg = -1, uint32 abs = 0, uint32 weapon_damage_type = 0);
    void HandleProcDmgShield(uint32 flag, Unit* attacker);//almost the same as handleproc :P

    void RemoveExtraStrikeTarget(SpellEntry *spell_info);
    void AddExtraStrikeTarget(SpellEntry *spell_info, uint32 charges);

    int32 GetAP();
    int32 GetRAP();

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
    void LosePower(uint32 powerField, int32 value);
    void SendPowerUpdate(int8 power = -1);

    void EventModelChange();
    HEARTHSTONE_INLINE float GetModelHalfSize() { return m_modelhalfsize * m_floatValues[OBJECT_FIELD_SCALE_X]; }

    HEARTHSTONE_INLINE void setHRegenTimer(uint32 time) {m_H_regenTimer = time; }
    HEARTHSTONE_INLINE void setPRegenTimer(uint32 time) {m_P_regenTimer = time; }
    HEARTHSTONE_INLINE void DelayPowerRegeneration(uint32 time) { m_p_DelayTimer = time; }

    void DeMorph();
    uint32 ManaShieldAbsorb(uint32 dmg, SpellEntry* sp);
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
    void setDeathState(DeathState s);
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
    int32 GetSpellBonusDamage(Unit* pVictim, SpellEntry *spellInfo,int32 base_dmg, bool healing);

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
    bool m_damgeShieldsInUse;
    std::list<struct DamageProc> m_damageShields;
    std::list<struct ReflectSpellSchool*> m_reflectSpellSchool;
    std::list<struct ProcTriggerSpell> m_procSpells;
    bool HasProcSpell(uint32 spellid);

    bool m_chargeSpellsInUse;
    std::deque<Aura*> m_chargeSpellRemoveQueue;
    std::list<Aura*> m_chargeSpells;
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

    //Custom timers
    uint32 m_CustomTimers[NUM_CUSTOM_TIMERS];

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

    int32 DamageTakenMod[7];
    float DamageTakenPctMod[7];
    uint32 SchoolCastPrevent[7];
    float MechanicDurationPctMod[NUM_MECHANIC];
    HEARTHSTONE_INLINE int32 GetDamageDoneMod(uint32 school) { if(DamageDoneMod[school]) return DamageDoneMod[school]; return DamageDonePosMod[school] - DamageDoneNegMod[school]; };
    HEARTHSTONE_INLINE float GetDamageDonePctMod(uint32 school) { return DamageDonePctMod[school]; };
    HEARTHSTONE_INLINE int32 GetHealingDoneMod() { return HealDoneModPos; };
    HEARTHSTONE_INLINE int32 GetHealingTakenMod() { return HealTakenMod; };

    //float DamageTakenPctModOnHP35; DEPRECATED, YAY!
    float CritMeleeDamageTakenPctMod[7];
    float CritRangedDamageTakenPctMod[7];
    int32 RangedDamageTaken;

    void CalcDamage();
    float BaseDamage[2];
    float BaseOffhandDamage[2];
    float BaseRangedDamage[2];
    SchoolAbsorb Absorbs[7];

    uint32 AbsorbDamage(Object* Attacker, uint32 School,uint32 * dmg, SpellEntry * pSpell);//returns amt of absorbed dmg, decreases dmg by absorbed value
    int32 RAPvModifier;
    int32 APvModifier;
    uint64 stalkedby;
    uint32 dispels[10];
    uint32 MechanicsDispels[NUM_MECHANIC];
    float MechanicsResistancesPCT[NUM_MECHANIC];
    float ModDamageTakenByMechPCT[NUM_MECHANIC];
    float DispelResistancesPCT[10];
    //int32 RangedDamageTakenPct;

    //SM
    int32 * SM[SPELL_MODIFIERS][2]; // 0 = flat, 1 = percent
    void InheritSMMods(Unit* inherit_from);
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
    HEARTHSTONE_INLINE int32 GetManaPct() { return (int32)(GetUInt32Value(UNIT_FIELD_POWER1) * 100 / std::max(1, (int32)GetUInt32Value(UNIT_FIELD_MAXPOWER1))); }
    HEARTHSTONE_INLINE float GetStat(uint32 stat) const { return float(GetUInt32Value(UNIT_FIELD_STAT0+stat)); }

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

    //Pet
    HEARTHSTONE_INLINE void SetIsPet(bool chck) { m_isPet = chck; }

    //In-Range
    virtual void AddInRangeObject(Object* pObj);
    virtual void OnRemoveInRangeObject(Object* pObj);
    void ClearInRangeSet();

    HEARTHSTONE_INLINE void AddDelayedSpell(Spell* toAdd) { DelayedSpells.insert(toAdd); };
    HEARTHSTONE_INLINE Spell* GetCurrentSpell() { return m_currentSpell; }
    HEARTHSTONE_INLINE void SetCurrentSpell(Spell* cSpell) { m_currentSpell = cSpell; }

    uint32 m_CombatUpdateTimer;

    HEARTHSTONE_INLINE void setcanperry(bool newstatus){can_parry=newstatus;}

    std::map<uint32,Aura* > tmpAura;

    float m_modelhalfsize; // used to calculate if something is in range of this unit

    uint32 BaseResistance[7]; //there are resistances for silence, fear, mechanics ....
    uint32 BaseStats[5];

    int32 DamageDoneMod[7];
    int32 DamageDonePosMod[7];
    int32 DamageDoneNegMod[7];
    float DamageDonePctMod[7];
    int32 SpellDmgDoneByAttribute[5][7];
    int32 SpellDamageFromAP[7];

    map<uint32, uint32> HealDoneBonusBySpell;
    int32 HealDoneModPos;
    int32 HealDoneBase;
    float HealDonePctMod;
    int32 HealTakenMod;
    float HealTakenPctMod;
    int32 SpellHealDoneByAttribute[5];
    int32 SpellHealFromAP;
    int32 Expertise[2];

    uint32 SchoolImmunityList[7];
    float SpellCritChanceSchool[7];
    int32 PowerCostMod[7];
    float PowerCostPctMod[7]; // armor penetration & spell penetration
    int32 AttackerCritChanceMod[7];
    uint32 SpellDelayResist[7];
    int32 CreatureAttackPowerMod[12];
    int32 CreatureRangedAttackPowerMod[12];
    float AOEDmgMod;

    int32 PctRegenModifier;
    float PctPowerRegenModifier[4];
    void RemoveSoloAura(uint32 type);

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

    uint32 GetPoisonDosesCount( uint32 poison_type );

    uint16 m_diminishCount[DIMINISH_GROUPS];
    uint8  m_diminishAuraCount[DIMINISH_GROUPS];
    uint16 m_diminishTimer[DIMINISH_GROUPS];
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

    struct {
        uint32 amt;
        uint32 max;
    } m_soulSiphon;

    //solo target auras
    uint32 m_hotStreakCount;
    uint32 m_incanterAbsorption;
    uint32 m_frozenTargetCharges;
    uint32 m_frozenTargetId;
    uint32 polySpell;
    uint32 m_special_state; //flags for special states (stunned,rooted etc)

//  uint32 fearSpell;
    CombatStatusHandler CombatStatus;
    bool m_temp_summon;

    // Redirect Threat shit
    Unit* mThreatRTarget;
    float mThreatRAmount;

    uint32 m_vampiricTouch;

    void EventCancelSpell(Spell* ptr);
    void EventStrikeWithAbility(uint64 guid, SpellEntry * sp, uint32 damage);

    /////////////////////////////////////////////////////// Unit properties ///////////////////////////////////////////////////
    HEARTHSTONE_INLINE void SetCharmedUnitGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CHARM, GUID); }
    HEARTHSTONE_INLINE void SetSummonedUnitGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_SUMMON, GUID); }
    HEARTHSTONE_INLINE void SetSummonedCritterGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CRITTER, GUID); }

    HEARTHSTONE_INLINE void SetCharmedByGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CHARMEDBY, GUID); }
    HEARTHSTONE_INLINE void SetSummonedByGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_SUMMONEDBY, GUID); }
    HEARTHSTONE_INLINE void SetCreatedByGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CREATEDBY, GUID); }


    HEARTHSTONE_INLINE uint64 GetCharmedUnitGUID() { return GetUInt64Value(UNIT_FIELD_CHARM); }
    HEARTHSTONE_INLINE uint64 GetSummonedUnitGUID() { return GetUInt64Value(UNIT_FIELD_SUMMON); }
    HEARTHSTONE_INLINE uint64 GetSummonedCritterGUID() { return GetUInt64Value(UNIT_FIELD_CRITTER); }

    HEARTHSTONE_INLINE uint64 GetCharmedByGUID() { return GetUInt64Value(UNIT_FIELD_CHARMEDBY); }
    HEARTHSTONE_INLINE uint64 GetSummonedByGUID() { return GetUInt64Value(UNIT_FIELD_SUMMONEDBY); }
    HEARTHSTONE_INLINE uint64 GetCreatedByGUID() { return GetUInt64Value(UNIT_FIELD_CREATEDBY); }

    HEARTHSTONE_INLINE void SetTargetGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_TARGET, GUID); }
    HEARTHSTONE_INLINE uint64 GetTargetGUID() { return GetUInt64Value(UNIT_FIELD_TARGET); }

    HEARTHSTONE_INLINE void SetChannelSpellTargetGUID(uint64 GUID) { SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, GUID); }
    HEARTHSTONE_INLINE void SetChannelSpellId(uint32 SpellId) { SetUInt32Value(UNIT_CHANNEL_SPELL, SpellId); }

    HEARTHSTONE_INLINE uint64 GetChannelSpellTargetGUID() { return GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT); }
    HEARTHSTONE_INLINE uint32 GetChannelSpellId() { return GetUInt32Value(UNIT_CHANNEL_SPELL); }

    HEARTHSTONE_INLINE void SetEquippedItem(uint8 slot, uint32 id) { SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + slot, id); }
    HEARTHSTONE_INLINE uint32 GetEquippedItem(uint8 slot) { return GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + slot); }

    HEARTHSTONE_INLINE void SetBaseAttackTime(uint8 slot, uint32 time) { SetUInt32Value(UNIT_FIELD_BASEATTACKTIME + slot, time); }
    HEARTHSTONE_INLINE uint32 GetBaseAttackTime(uint8 slot) { return GetUInt32Value(UNIT_FIELD_BASEATTACKTIME + slot); }
    HEARTHSTONE_INLINE void ModBaseAttackTime(uint8 slot, int32 mod) { ModUnsigned32Value(UNIT_FIELD_BASEATTACKTIME + slot, mod); }

    HEARTHSTONE_INLINE void SetBoundingRadius(float rad) { SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, rad); }
    HEARTHSTONE_INLINE float GetBoundingRadius() { return GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS); }

    HEARTHSTONE_INLINE void SetCombatReach(float len) { SetFloatValue(UNIT_FIELD_COMBATREACH, len); }
    HEARTHSTONE_INLINE float GetCombatReach() { return GetFloatValue(UNIT_FIELD_COMBATREACH); }

    HEARTHSTONE_INLINE void SetDisplayId(uint32 id) { SetUInt32Value(UNIT_FIELD_DISPLAYID, id); }
    HEARTHSTONE_INLINE uint32 GetDisplayId() { return GetUInt32Value(UNIT_FIELD_DISPLAYID); }

    HEARTHSTONE_INLINE void SetNativeDisplayId(uint32 id) { SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, id); }
    HEARTHSTONE_INLINE uint32 GetNativeDisplayId() { return GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HEARTHSTONE_INLINE void SetMinDamage(float amt) { SetFloatValue(UNIT_FIELD_MINDAMAGE, amt); }
    HEARTHSTONE_INLINE float GetMinDamage() { return GetFloatValue(UNIT_FIELD_MINDAMAGE); }

    HEARTHSTONE_INLINE void SetMaxDamage(float amt) { SetFloatValue(UNIT_FIELD_MAXDAMAGE, amt); }
    HEARTHSTONE_INLINE float GetMaxDamage() { return GetFloatValue(UNIT_FIELD_MAXDAMAGE); }

    HEARTHSTONE_INLINE void SetMinOffhandDamage(float amt) { SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, amt); }
    HEARTHSTONE_INLINE float GetMinOffhandDamage() { return GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE); }

    HEARTHSTONE_INLINE void SetMaxOffhandDamage(float amt) { SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, amt); }
    HEARTHSTONE_INLINE float GetMaxOffhandDamage() { return GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE); }

    HEARTHSTONE_INLINE void SetMinRangedDamage(float amt) { SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, amt); }
    HEARTHSTONE_INLINE float GetMinRangedDamage() { return GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE); }

    HEARTHSTONE_INLINE void SetMaxRangedDamage(float amt) { SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, amt); }
    HEARTHSTONE_INLINE float GetMaxRangedDamage() { return GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HEARTHSTONE_INLINE void SetMount(uint32 id) { SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, id); }
    HEARTHSTONE_INLINE uint32 GetMount() { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }

    HEARTHSTONE_INLINE void SetCastSpeedMod(float amt) { SetFloatValue(UNIT_MOD_CAST_SPEED, amt); }
    HEARTHSTONE_INLINE float GetCastSpeedMod() { return GetFloatValue(UNIT_MOD_CAST_SPEED); }
    HEARTHSTONE_INLINE void ModCastSpeedMod(float mod) { ModFloatValue(UNIT_MOD_CAST_SPEED, mod); }

    HEARTHSTONE_INLINE void SetCreatedBySpell(uint32 id) { SetUInt32Value(UNIT_CREATED_BY_SPELL, id); }
    HEARTHSTONE_INLINE uint32 GetCreatedBySpell() { return GetUInt32Value(UNIT_CREATED_BY_SPELL); }

    HEARTHSTONE_INLINE void SetEmoteState(uint32 id) { SetUInt32Value(UNIT_NPC_EMOTESTATE, id); }
    HEARTHSTONE_INLINE uint32 GetEmoteState() { return GetUInt32Value(UNIT_NPC_EMOTESTATE); }

    HEARTHSTONE_INLINE void SetStat(uint32 stat, uint32 amt) { SetUInt32Value(UNIT_FIELD_STAT0 + stat, amt); }
    HEARTHSTONE_INLINE uint32 GetStat(uint32 stat) { return GetUInt32Value(UNIT_FIELD_STAT0 + stat); }

    HEARTHSTONE_INLINE void SetResistance(uint32 type, uint32 amt) { SetUInt32Value(UNIT_FIELD_RESISTANCES + type, amt); }
    HEARTHSTONE_INLINE uint32 GetResistance(uint32 type) { return GetUInt32Value(UNIT_FIELD_RESISTANCES + type); }

    HEARTHSTONE_INLINE void SetBaseMana(uint32 amt) { SetUInt32Value(UNIT_FIELD_BASE_MANA, amt); }
    HEARTHSTONE_INLINE uint32 GetBaseMana() { return GetUInt32Value(UNIT_FIELD_BASE_MANA); }

    HEARTHSTONE_INLINE void SetBaseHealth(uint32 amt) { SetUInt32Value(UNIT_FIELD_BASE_HEALTH, amt); }
    HEARTHSTONE_INLINE uint32 GetBaseHealth() { return GetUInt32Value(UNIT_FIELD_BASE_HEALTH); }

    HEARTHSTONE_INLINE void SetPowerCostMultiplier(uint32 school, float amt) { SetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + school, amt); }
    HEARTHSTONE_INLINE void ModPowerCostMultiplier(uint32 school, float amt) { ModFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + school, amt); }
    HEARTHSTONE_INLINE float GetPowerCostMultiplier(uint32 school) { return GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + school); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HEARTHSTONE_INLINE void SetAttackPower(uint32 amt) { SetUInt32Value(UNIT_FIELD_ATTACK_POWER, amt); }
    HEARTHSTONE_INLINE uint32 GetAttackPower() { return GetUInt32Value(UNIT_FIELD_ATTACK_POWER); }

    HEARTHSTONE_INLINE void SetAttackPowerPositiveMods(uint32 amt) { SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_POS, amt); }
    HEARTHSTONE_INLINE uint32 GetAttackPowerPositiveMods() { return GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_POS); }
    HEARTHSTONE_INLINE void ModAttackPowerPositiveMods(uint32 amt) { ModUnsigned32Value(UNIT_FIELD_ATTACK_POWER_MOD_POS, amt); }

    HEARTHSTONE_INLINE void SetAttackPowerNegativeMods(uint32 amt) { SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_NEG, amt); }
    HEARTHSTONE_INLINE uint32 GetAttackPowerNegativeMods() { return GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MOD_NEG); }
    HEARTHSTONE_INLINE void ModAttackPowerNegativeMods(uint32 amt) { ModUnsigned32Value(UNIT_FIELD_ATTACK_POWER_MOD_NEG, amt); }

    HEARTHSTONE_INLINE void SetAttackPowerMultiplier(float amt) { SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, amt); }
    HEARTHSTONE_INLINE float GetAttackPowerMultiplier() { return GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER); }
    HEARTHSTONE_INLINE void ModAttackPowerMultiplier(float amt) { ModFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, amt); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HEARTHSTONE_INLINE void SetRangedAttackPower(uint32 amt) { SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER, amt); }
    HEARTHSTONE_INLINE uint32 GetRangedAttackPower() { return GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER); }

    HEARTHSTONE_INLINE void SetRangedAttackPowerPositiveMods(uint32 amt) { SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS, amt); }
    HEARTHSTONE_INLINE uint32 GetRangedAttackPowerPositiveMods() { return GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS); }
    HEARTHSTONE_INLINE void ModRangedAttackPowerPositiveMods(uint32 amt) { ModUnsigned32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS, amt); }

    HEARTHSTONE_INLINE void SetRangedAttackPowerNegativeMods(uint32 amt) { SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG, amt); }
    HEARTHSTONE_INLINE uint32 GetRangedAttackPowerNegativeMods() { return GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG); }
    HEARTHSTONE_INLINE void ModRangedAttackPowerNegativeMods(uint32 amt) { ModUnsigned32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_NEG, amt); }

    HEARTHSTONE_INLINE void SetRangedAttackPowerMultiplier(float amt) { SetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER, amt); }
    HEARTHSTONE_INLINE float GetRangedAttackPowerMultiplier() { return GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER); }
    HEARTHSTONE_INLINE void ModRangedAttackPowerMultiplier(float amt) { ModFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER, amt); }

    void SetPowerType(uint8 type);
    void SetPower(uint32 type, int32 value);
    HEARTHSTONE_INLINE uint32 GetPowerType() { return (GetByte(UNIT_FIELD_BYTES_0, 3));}
    HEARTHSTONE_INLINE uint32 GetPower(uint8 power) const { return GetUInt32Value(UNIT_FIELD_POWER1 + power); }

    bool HasAurasOfNameHashWithCaster(uint32 namehash, Unit* caster);
    bool mAngerManagement;
    bool mRecentlyBandaged;

    float m_ignoreArmorPctMaceSpec;
    float m_ignoreArmorPct;

    int32 m_LastSpellManaCost;

    void OnPositionChange();
    void Dismount();
    //  custom functions for scripting
    void SetWeaponDisplayId(uint8 slot, uint32 ItemId);

    HEARTHSTONE_INLINE uint32 GetMaxPower( uint32 index ){ return GetUInt32Value( UNIT_FIELD_MAXPOWER1 + index ); }
    HEARTHSTONE_INLINE void SetMaxPower( uint32 index, uint32 value ){SetUInt32Value(UNIT_FIELD_MAXPOWER1+index,value );}

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
