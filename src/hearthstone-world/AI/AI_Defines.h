/***
 * Demonstrike Core
 */

#pragma once

class AIInterface;

/* platforms that already define M_PI in math.h */
#ifdef M_PI
#undef M_PI
#endif

#define M_PI 3.14159265358979323846f
#define UNIT_MOVEMENT_INTERPOLATE_INTERVAL 200/*750*/ // ms smoother server/client side moving vs less cpu/ less b/w
#define TARGET_UPDATE_INTERVAL 600 // ms
#define oocr 50.0f // out of combat range
#define CREATURE_SPELL_TO_DAZE 1604
#define CREATURE_DAZE_TRIGGER_ANGLE M_PI/2 //for the beginners this means 45 degrees
#define DISTANCE_TO_SMALL_TO_WALK 2.0f //this is required so creature will not try to reposition itself to obtain perfect combat range. Not using this might lead to exploits

class Object;
class Creature;
class Unit;
class Player;
class WorldSession;
class SpellCastTargets;

enum AIType
{
    AITYPE_LONER,
    AITYPE_AGRO,
    AITYPE_SOCIAL,
    AITYPE_PET,
    AITYPE_TOTEM,
    AITYPE_GUARDIAN, //we got a master but he cannot control us, we follow and battle oposite factions
    AITYPE_DUMMY
};

enum AI_State
{
    STATE_IDLE,
    STATE_ATTACKING,
    STATE_CASTING,
    STATE_FLEEING,
    STATE_FOLLOWING,
    STATE_EVADE,
    STATE_MOVEWP,
    STATE_FEAR,
    STATE_WANDER,
    STATE_STOPPED,
    STATE_SCRIPTMOVE,
    STATE_SCRIPTIDLE
};

enum AiEvents
{
    EVENT_ENTERCOMBAT = 0,
    EVENT_LEAVECOMBAT = 1,
    EVENT_DAMAGETAKEN = 2,
    EVENT_FEAR = 3,
    EVENT_UNFEAR = 4,
    EVENT_FOLLOWOWNER = 5,
    EVENT_WANDER = 6,
    EVENT_UNWANDER = 7,
    EVENT_UNITDIED = 8,
    EVENT_UNITRESPAWN = 9,
    EVENT_HOSTILEACTION = 10,
    EVENT_FORCEREDIRECTED = 11,
    NUM_AI_EVENTS = 12
};

enum AiCombatEvents
{

};

enum CreatureState
{
    STOPPED,
    MOVING,
    ATTACKING
};

enum MovementType
{
    MOVEMENTTYPE_NONE,
    MOVEMENTTYPE_RANDOMWP,
    MOVEMENTTYPE_CIRCLEWP,
    MOVEMENTTYPE_WANTEDWP,
    MOVEMENTTYPE_DONTMOVEWP,
    MOVEMENTTYPE_QUEST = 10,
    MOVEMENTTYPE_FORWARDTHANSTOP = 11,
};

enum LimitedMovementFlag
{
    LIMIT_ROOT          = 0x0,
    LIMIT_GROUND        = 0x1,
    LIMIT_WATER         = 0x2,
    LIMIT_AIR           = 0x4,
    LIMIT_ON_OBJ        = 0x8,
    LIMIT_ANYWHERE      = LIMIT_GROUND | LIMIT_WATER | LIMIT_AIR
};

enum MovementState
{
    MOVEMENTSTATE_MOVE,
    MOVEMENTSTATE_FOLLOW,
    MOVEMENTSTATE_STOP,
    MOVEMENTSTATE_FOLLOW_OWNER
};

enum MonsterMoveFlags   // for AIInterface::SendMoveToPacket
{
    MONSTER_MOVE_FLAG_STAND     = 0x0,
    MONSTER_MOVE_FLAG_TELEPORT  = 0x100,
    MONSTER_MOVE_FLAG_JUMP      = 0x800,
    MONSTER_MOVE_FLAG_WALK      = 0x1000,
    MONSTER_MOVE_FLAG_FLY       = 0x2000,
    MONSTER_MOVE_FLAG_FLY2      = 0x3000,
};

enum BehaviorType
{
    Behavior_Default = 0,
    Behavior_Melee,
    Behavior_Ranged,
    Behavior_Spell,
    Behavior_Flee,
    Behavior_CallForHelp
};

enum TargetGenType
{
    // Self
    TargetGen_Self = 0,                     // Target self ( Note: doesn't always mean self, also means the spell can choose various target )

    // Current
    TargetGen_Current,                      // Current highest aggro ( attacking target )
    TargetGen_Destination,                  // Target is a destination coordinates ( X, Y, Z )
    TargetGen_SummonOwner,                  // Target the owner of ourselves

    // Second most hated
    TargetGen_SecondMostHated,              // Second highest aggro

    // Predefined target
    TargetGen_Predefined,                   // Pre-defined target unit

    // Random Unit
    TargetGen_RandomUnit,                   // Random target unit ( players, totems, pets, etc. )
    TargetGen_RandomUnitDestination,        // Random destination coordinates ( X, Y, Z )
    TargetGen_RandomUnitApplyAura,          // Random target unit to self cast aura

    // Random Player
    TargetGen_RandomPlayer,                 // Random target player
    TargetGen_RandomPlayerDestination,      // Random player destination coordinates ( X, Y, Z )
    TargetGen_RandomPlayerApplyAura,        // Random target player to self cast aura

    // Mana Classes
    TargetGen_ManaClass,                    // Target with mana as power type
};

enum TargetFilters
{
    // Standard filters
    TargetFilter_None                       = 0x00, // 0
    TargetFilter_Closest                    = 0x01, // 1
    TargetFilter_Friendly                   = 0x02, // 2
    TargetFilter_NotCurrent                 = 0x04, // 4
    TargetFilter_Wounded                    = 0x08, // 8
    TargetFilter_SecondMostHated            = 0x10, // 16
    TargetFilter_Aggroed                    = 0x20, // 32
    TargetFilter_Corpse                     = 0x40, // 64
    TargetFilter_InMeleeRange               = 0x80, // 128
    TargetFilter_InRangeOnly                = 0x100, // 256
    TargetFilter_IgnoreSpecialStates        = 0x200, // 512 - not really a TargetFilter, more like requirement for spell
    TargetFilter_IgnoreLineOfSight          = 0x400, // 1024
    TargetFilter_ManaClass                  = 0x800, // 2048

    // Predefined filters
    TargetFilter_ClosestFriendly            = TargetFilter_Closest | TargetFilter_Friendly,                         // 3
    TargetFilter_ClosestNotCurrent          = TargetFilter_Closest | TargetFilter_NotCurrent,                       // 5
    TargetFilter_WoundedFriendly            = TargetFilter_Wounded | TargetFilter_Friendly,                         // 10
    TargetFilter_FriendlyCorpse             = TargetFilter_Corpse  | TargetFilter_Friendly,                         // 66
    TargetFilter_ClosestFriendlyCorpse      = TargetFilter_Closest | TargetFilter_FriendlyCorpse,                   // 67
};

// Entry, pSpellId, pChance, CastTime, Cooldown, MinRange, MaxRange, pStrictRange);
struct AI_Spell
{
    AI_Spell()
    {
        info = NULL;

        m_AI_Spell_disabled = false;

        TargetType = TargetGen_Self; TargetFilter = TargetFilter_None;

        perctrigger = 0.0f; attackstoptimer = 0;

        lastcast = 0; cooldown = 0; casttime = 0; soundid = 0;

        ProcLimit = 0; procCounter = 0; ProcResetDelay = 0; ProcResetTimer = 0;

        mindist2cast = 0.0f; maxdist2cast = 0.0f;

        minHPPercentReq = 0.0f;

        FloatData[0] = 0.0f; FloatData[1] = 0.0f; FloatData[2] = 0.0f;

        autocast_type = 1;

        mPredefinedTarget = NULL;
    };

    AI_Spell(AI_Spell* cp, uint32 n_cooldown, uint32 n_proccount)
    {
        info = cp->info;

        m_AI_Spell_disabled = false;

        TargetType = cp->TargetType; TargetFilter = cp->TargetFilter;

        perctrigger = cp->perctrigger; attackstoptimer = cp->attackstoptimer;

        lastcast = 0; cooldown = n_cooldown; casttime = cp->casttime; soundid = cp->soundid;

        ProcLimit = cp->ProcLimit; procCounter = n_proccount; ProcResetDelay = cp->ProcResetDelay; ProcResetTimer = 0;

        mindist2cast = cp->mindist2cast; maxdist2cast = cp->maxdist2cast;

        minHPPercentReq = cp->minHPPercentReq;

        FloatData[0] = cp->FloatData[0]; FloatData[1] = cp->FloatData[1]; FloatData[2] = cp->FloatData[2];

        autocast_type = cp->autocast_type;

        mPredefinedTarget = NULL;
    };

    /* CD = Core Data | DB = Data Base | SD = Script Data */
    bool m_AI_Spell_disabled;       // CD | is spell disabled
    SpellEntry *info;               // CD | spell info
    TargetGenType TargetType;       // DB | Our target type
    uint32 TargetFilter;            // DB | AI_TargetFilters: Our target filter flags

    float perctrigger;              // DB | % of the cast of this spell in a total of 100% of the attacks
    uint32 attackstoptimer;         // DB | stop the creature from attacking

    uint32 lastcast;                // CD | last time the spell was cast
    uint32 cooldown;                // DB | spell cooldown
    uint32 casttime;                // DB | "time" required to cast spell
    uint32 soundid;                 // DB | sound id from DBC

    uint32 ProcLimit;               // DB | number of times the spell can proc
    uint32 procCounter;             // CD | number of times the spell has proc'd
    uint32 ProcResetDelay;          // DB | time difference between proc counter resets
    uint32 ProcResetTimer;          // CD | set timer for when procs will reset

    float mindist2cast;             // DB | min dist from caster to victim to perform cast (dist from caster >= mindist2cast)
    float maxdist2cast;             // DB | max dist from caster to victim to perform cast (dist from caster <= maxdist2cast)

    float minHPPercentReq;          // DB | % of health required to cast

    union                           // DB | Data!
    {
        float FloatData[3];
        uint32 UIntData[3];
    };

    int32 difficulty_mask;          // DB | For what difficulty the spell is usable on.
    int32 autocast_type;            // CD | Pet autocast types

    Unit* mPredefinedTarget;        // SD | Our set target
};

bool isGuard(uint32 id);
uint32 getGuardId(uint32 id);
bool isTargetDummy(uint32 id);

typedef std::map<uint32, AI_Spell*> SpellMap;
typedef map<uint32, LocationVector> LocationVectorMap;
typedef std::map<Unit*, int32> TargetMap;

struct LocationVectorMapContainer
{
    uint32 StartTime;
    uint32 TotalMoveTime;
    LocationVectorMap *InternalMap;
};

typedef void(AIInterface::*pAIEvent)(Unit* punit, uint32 misc1);
