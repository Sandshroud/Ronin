/*
 * Lacrimi Scripts Copyright 2010 - 2011
 *
 * ############################################################
 * # ##            #       ####### ####### ##    #    #    ## #
 * # ##           ###      ##      ##   ## ##   ###  ###   ## #
 * # ##          ## ##     ##      ##   ## ##   ###  ###   ## #
 * # ##         #######    ##      ####### ##  ## #### ##  ## #
 * # ##        ##     ##   ##      #####   ##  ## #### ##  ## #
 * # ##       ##       ##  ##      ##  ##  ## ##   ##   ## ## #
 * # ####### ##         ## ####### ##   ## ## ##   ##   ## ## #
 * # :::::::.::.........::.:::::::.::...::.::.::...::...::.:: #
 * ############################################################
 *
 */

#ifndef BASE_H
#define BASE_H

#define INVALIDATE_TIMER            -1
#define DEFAULT_UPDATE_FREQUENCY    1000    //milliseconds
#define DEFAULT_DESPAWN_TIMER       2000    //milliseconds

#define MOONSCRIPT_FACTORY_FUNCTION(ClassName, ParentClassName)\
public:\
    ADD_CREATURE_FACTORY_FUNCTION(ClassName);\
    typedef ParentClassName ParentClass;

enum TextType
{
    Text_Say,
    Text_Yell,
    Text_Emote
};

enum EventType
{
    Event_OnCombatStart,
    Event_OnTargetDied,
    Event_OnDied,
    Event_OnTaunt
};

enum MoveType
{
    Move_None,
    Move_RandomWP,
    Move_CircleWP,
    Move_WantedWP,
    Move_DontMoveWP,
    Move_Quest,
    Move_ForwardThenStop
};

enum MoveFlag
{
    Flag_Walk = 0,
    Flag_Run = 256,
    Flag_Fly = 768
};

struct EmoteDesc
{
    EmoteDesc(const char* pText, TextType pType, uint32 pSoundId, EmoteType pEmoteType)
    {
        mText = ( pText && strlen(pText) > 0 ) ? pText : "";
        mType = pType;
        mSoundId = pSoundId;
        mEmoteType = pEmoteType;
    }

    std::string mText;
    TextType    mType;
    uint32      mSoundId;
    EmoteType   mEmoteType;
};

struct Coords
{
    float   mX;
    float   mY;
    float   mZ;
    float   mO;
    uint32  mAddition;
};

struct LootDesc
{
    uint32 mItemID;
    uint32 mChance;
    uint32 mMinCount;
    uint32 mMaxCount;
    uint32 mFFA;
};

enum RangeStatus
{
    RangeStatus_Ok,
    RangeStatus_TooFar,
    RangeStatus_TooClose
};

class TargetType;
class SpellDesc;
class MoonScriptCreatureAI;
class MoonScriptBossAI;

typedef void(*SpellFunc)(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit *pTarget, TargetType pType);
typedef std::vector<EmoteDesc*> EmoteArray;
typedef std::vector<Player *> PlayerArray;
typedef std::vector<Unit *> UnitArray;
typedef std::vector<SpellDesc*> SpellDescArray;
typedef std::list<SpellDesc*> SpellDescList;
typedef std::pair<int32, SpellDesc*> PhaseSpellPair;
typedef std::vector<PhaseSpellPair> PhaseSpellArray;
typedef std::pair<int32, int32> TimerPair;
typedef std::vector<TimerPair> TimerArray;
typedef std::vector<LootDesc> LootTable;
typedef std::pair< RangeStatus, float > RangeStatusPair;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class TargetType
class SCRIPT_DECL TargetType
{
public:
    TargetType( uint32 pTargetGen = TargetGen_Self, TargetFilters pTargetFilter = TargetFilter_None, uint32 pMinTargetNumber = 0, uint32 pMaxTargetNumber = 0 );
    ~TargetType();

    uint32          mTargetGenerator;   // Defines what kind of target should we try to find
    TargetFilters   mTargetFilter;      // Defines filter of target
    uint32          mTargetNumber[ 2 ]; // 0: Defines min. number of creature on hatelist ( 0 - any, 1 - the most hated etc. )
                                        // 1: Defines max. number of creature on hatelist ( 0 - any, HateList.size + 1 - the least hated etc. )
};

// Pre-made TargetTypes
#define Target_Self TargetType()
#define Target_Current TargetType( TargetGen_Current )
#define Target_SecondMostHated TargetType( TargetGen_SecondMostHated )
#define Target_Destination TargetType( TargetGen_Destination )
#define Target_Predefined TargetType( TargetGen_Predefined )
#define Target_RandomPlayer TargetType( TargetGen_RandomPlayer )
#define Target_RandomPlayerNotCurrent TargetType( TargetGen_RandomPlayer, TargetFilter_NotCurrent )
#define Target_RandomPlayerDestination TargetType( TargetGen_RandomPlayerDestination )
#define Target_RandomPlayerApplyAura TargetType( TargetGen_RandomPlayerApplyAura )
#define Target_RandomUnit TargetType( TargetGen_RandomUnit )
#define Target_RandomUnitNotCurrent TargetType( TargetGen_RandomUnit, TargetFilter_NotCurrent )
#define Target_RandomDestination TargetType( TargetGen_RandomUnitDestination )
#define Target_RandomUnitApplyAura TargetType( TargetGen_RandomUnitApplyAura )
#define Target_RandomFriendly TargetType( TargetGen_RandomUnit, TargetFilter_Friendly )
#define Target_WoundedPlayer TargetType( TargetGen_RandomPlayer, TargetFilter_Wounded )
#define Target_WoundedUnit TargetType( TargetGen_RandomUnit, TargetFilter_Wounded )
#define Target_WoundedFriendly TargetType( TargetGen_RandomUnit, TargetFilter_WoundedFriendly )
#define Target_ClosestPlayer TargetType( TargetGen_RandomPlayer, TargetFilter_Closest )
#define Target_ClosestPlayerNotCurrent TargetType( TargetGen_RandomPlayer, TargetFilter_ClosestNotCurrent )
#define Target_ClosestUnit TargetType( TargetGen_RandomUnit, TargetFilter_Closest )
#define Target_ClosestUnitNotCurrent TargetType( TargetGen_RandomUnit, TargetFilter_ClosestNotCurrent )
#define Target_ClosestFriendly TargetType( TargetGen_RandomUnit, TargetFilter_ClosestFriendly )
#define Target_ClosestCorpse TargetType( TargetGen_RandomUnit, TargetFilter_ClosestFriendlyCorpse )
#define Target_RandomCorpse TargetType( TargetGen_RandomUnit, TargetFilter_FriendlyCorpse )
#define Target_ManaClass TargetType(TargetGen_ManaClass, TargetFilter_ManaClass)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class SpellDesc
class SCRIPT_DECL SpellDesc
{
public:
    SpellDesc(SpellEntry* pInfo, SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange, float pMaxRange, bool pStrictRange, const char* pText, TextType pTextType, uint32 pSoundId, EmoteType pEmoteType);
    virtual ~SpellDesc();

    EmoteDesc*  AddEmote(const char* pText, TextType pType=Text_Yell, uint32 pSoundId=0, EmoteType pEmoteType = EMOTE_ONESHOT_NONE);
    void        TriggerCooldown(uint32 pCurrentTime=0);

    SpellEntry* mInfo;              //Spell Entry information (generally you either want a SpellEntry OR a SpellFunc, not both)
    SpellFunc   mSpellFunc;         //Spell Function to be executed (generally you either want a SpellEntry OR a SpellFunc, not both)
    TargetType  mTargetType;        //Target type (see class above)
    float       mChance;            //Percent of the cast of this spell in a total of 100% of the attacks
    float       mCastTime;          //Duration of the spell cast (seconds). Zero means the spell is instant cast
    int32       mCooldown;          //Spell cooldown (seconds)
    float       mMinRange;          //Minimum range for spell to be cast
    float       mMaxRange;          //Maximum range for spell to be cast
    bool        mStrictRange;       //If true, creature won't run to (or away of) target, it will instead skip that attack
    EmoteArray  mEmotes;            //Emotes (random) shouted on spell cast
    bool        mEnabled;           //True if the spell is enabled for casting, otherwise it will never be scheduled (useful for bosses with phases, etc.)
    Unit *  mPredefinedTarget;  //Pre-defined Target Unit (Only valid with target type TargetGen_Predefined);

    //Those are not properties, they are data member used by the evaluation system
    uint32      mLastCastTime;      //Last time at which the spell casted (used to implement cooldown), set to 0
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class MoonScriptCreatureAI
class SCRIPT_DECL MoonScriptCreatureAI : public CreatureAIScript
{
public:
    MoonScriptCreatureAI(Creature *pCreature);
    virtual ~MoonScriptCreatureAI();

    /* Gonna make a small documentation for scripts. 
    Currently i'm choosing where to do it. 
    First idea was to make a wiki for it, but it would be so slow to make it works.
    Second idea was to post an articles with docu on sandshroud forums. But I NEED MOAR FORMAT TOOLZ! :D
    Third idea is to put here comments, because while you 'brows' functions to use, you can read a small comments on what that function is doing.
    So, dunno what to do. Summarize all 'yes' and 'no' i'm beggin before wiki and maybe will do it some time later. */

    //Movement
    bool                    GetCanMove();
    void                    SetCanMove(bool pCanMove);
    void                    MoveTo(MoonScriptCreatureAI* pCreature, RangeStatusPair pRangeStatus = make_pair( RangeStatus_TooFar, 0.0f ) );
    void                    MoveTo(Unit *pUnit, RangeStatusPair pRangeStatus = make_pair( RangeStatus_TooFar, 0.0f ) );
    void                    MoveTo(float pX, float pY, float pZ, bool pRun=true);
    void                    MoveToSpawnOrigin();
    void                    StopMovement();

    //Attack and Combat State
    bool                    GetCanEnterCombat();
    void                    SetCanEnterCombat(bool pCanEnterCombat);
    bool                    IsInCombat();
    void                    DelayNextAttack(int32 pMilliseconds);
    void                    SetDespawnWhenInactive(bool pValue);

    //Behavior
    void                    SetBehavior(BehaviorType pBehavior);
    BehaviorType            GetBehavior();
    void                    SetAllowMelee(bool pAllow);
    bool                    GetAllowMelee();
    void                    SetAllowRanged(bool pAllow);
    bool                    GetAllowRanged();
    void                    SetAllowSpell(bool pAllow);
    bool                    GetAllowSpell();
    void                    SetAllowTargeting(bool pAllow);
    bool                    GetAllowTargeting();
    void                    AggroNearestUnit(int pInitialThreat=1);
    void                    AggroRandomUnit(int pInitialThreat=1);
    void                    AggroNearestPlayer(int pInitialThreat=1);
    void                    AggroRandomPlayer(int pInitialThreat=1);
    void                    AttackPlayer(Unit *pPlayer);

    //Status
    void                    ClearHateList();
    void                    WipeHateList();
    int32                   GetHealthPercent();
    int32                   GetManaPercent();
    void                    Regenerate();
    bool                    IsAlive();
    void                    SetScale(float pScale);
    float                   GetScale();
    void                    SetBaseHealth(uint32 mHealth);
    void                    SetDisplayId(uint32 pDisplayId);
    void                    SetWieldWeapon(bool pValue);
    void                    SetDisplayWeapon(bool pMainHand, bool pOffHand);
    void                    SetDisplayWeaponIds(uint32 pItem1Id, uint32 pItem2Id);

    //Environment
    float                   GetRange(MoonScriptCreatureAI* pCreature);
    float                   GetRangeToUnit(Unit *pUnit);

    //Instances
    bool                    IsHeroic();
    uint32                  GetInstanceMode();
    
    Player *                GetNearestPlayer();
    GameObject *            GetNearestGameObject(uint32 pGameObjectId=0);
    MoonScriptCreatureAI*   GetNearestCreature(uint32 pCreatureId=0);
    MoonScriptCreatureAI*   SpawnCreature(uint32 pCreatureId, bool pForceSameFaction=false, Unit* pTarget=NULLUNIT);
    MoonScriptCreatureAI*   SpawnCreature(uint32 pCreatureId, float pX, float pY, float pZ, float pO=0, bool pForceSameFaction=false, Unit* pTarget=NULLUNIT);
    Unit *                  ForceCreatureFind( uint32 pCreatureId );
    Unit *                  ForceCreatureFind( uint32 pCreatureId, float pX, float pY, float pZ );
    void                    Despawn(uint32 pDelay=0, uint32 pRespawnTime=0);


    SpellDesc*              AddSpell(uint32 pSpellId, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange=0, float pMaxRange=0, bool pStrictRange=false, const char* pText=NULL, TextType pTextType=Text_Yell, uint32 pSoundId=0, EmoteType pEmoteType = EMOTE_ONESHOT_NONE);
    SpellDesc*              AddSpellFunc(SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange=0, float pMaxRange=0, bool pStrictRange=false, const char* pText=NULL, TextType pTextType=Text_Yell, uint32 pSoundId=0, EmoteType pEmoteType = EMOTE_ONESHOT_NONE);

    void                    CastSpell(SpellDesc* pSpell);
    void                    CastSpellNowNoScheduling(SpellDesc* pSpell);
    void                    CastSpellInRange(SpellDesc *pSpell);
    SpellDesc*              FindSpellById(uint32 pSpellId);
    SpellDesc*              FindSpellByFunc(SpellFunc pFnc);
    bool                    IsCasting();
    void                    ApplyAura(uint32 pSpellId);
    void                    RemoveAura(uint32 pSpellId);
    void                    RemoveAuraOnPlayers(uint32 pSpellId);
    void                    RemoveAllAuras();
    void                    TriggerCooldownOnAllSpells();
    void                    CancelAllCooldowns();

    //Emotes
    EmoteDesc*              AddEmote(EventType pEventType, const char* pText, TextType pType, uint32 pSoundId=0, EmoteType pEmoteType = EMOTE_ONESHOT_NONE);
    void                    RemoveEmote(EventType pEventType, EmoteDesc* pEmote);
    void                    RemoveAllEmotes(EventType pEventType);
    void                    Emote(EmoteDesc* pEmote);
    void                    Emote(const char* pText, TextType pType=Text_Yell, uint32 pSoundId=0, EmoteType pEmoteType = EMOTE_ONESHOT_NONE);

    //Timers
    uint32                  AddTimer(int32 pDurationMillisec);
    int32                   GetTimer(int32 pTimerId);
    void                    RemoveTimer(int32& pTimerId);
    void                    ResetTimer(int32 pTimerId, int32 pDurationMillisec);
    bool                    IsTimerFinished(int32 pTimerId);
    void                    CancelAllTimers();

    //Waypoints
    WayPoint*               CreateWaypoint(int pId, uint32 pWaittime, uint32 pMoveFlag, Coords pCoords);
    void                    AddWaypoint(WayPoint* pWayPoint);
    void                    ForceWaypointMove(uint32 pWaypointId);
    void                    SetWaypointToMove(uint32 pWaypointId);
    void                    StopWaypointMovement();
    void                    SetMoveType(MoveType pMoveType);
    MoveType                GetMoveType();
    uint32                  GetCurrentWaypoint();
    size_t                  GetWaypointCount();
    bool                    HasWaypoints();

    //Others
    void                    SetTargetToChannel(Unit *pTarget, uint32 pSpellId);
    Unit *                  GetTargetToChannel();

    //Loot
    void                    AddLootToTable(LootTable* pTable, uint32 pItemID, uint32 pChance, uint32 pMinCount, uint32 pMaxCount, uint32 pFFA);
    void                    ClearLoot(Unit *pTarget);
    void                    AddLootFromTable(Unit *pTarget, LootTable* pTable, uint32 pCount = 1);
    void                    SetGoldLoot(Unit *pTarget, uint32 pMinGold, uint32 pMaxGold);
    void                    AddLoot(Unit *pTarget, uint32 pItemID, uint32 pMinCount, uint32 pMaxCount, uint32 pFFA);
    void                    AddRareLoot(Unit *pTarget, uint32 pItemID, float pPercentChance);

    //Reimplemented Events
    virtual void            AIUpdate(MapManagerScript* MMScript, uint32 p_time);
    virtual void            OnCombatStart(Unit *pTarget);
    virtual void            OnCombatStop(Unit *pTarget);
    virtual void            OnTargetDied(Unit *pTarget);
    virtual void            OnDied(Unit *pKiller);
    virtual void            Destroy();

protected:

    bool                    IsSpellScheduled(SpellDesc* pSpell);
    bool                    CastSpellInternal(SpellDesc* pSpell, uint32 pCurrentTime=0);
    void                    CastSpellOnTarget(Unit *pTarget, TargetType pType, SpellEntry* pEntry, bool pInstant);
    int32                   CalcSpellAttackTime(SpellDesc* pSpell);
    void                    CancelAllSpells();

    RangeStatusPair         GetSpellRangeStatusToUnit( Unit *pTarget, SpellDesc* pSpell );
    Unit *                  GetTargetForSpell( SpellDesc* pSpell );
    Unit *                  GetBestPlayerTarget( TargetFilters pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f );
    Unit *                  GetBestUnitTarget( TargetFilters pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f );
    Unit *                  ChooseBestTargetInArray( UnitArray& pTargetArray, TargetFilters pFilter );
    Unit *                  GetNearestTargetInArray( UnitArray& pTargetArray );
    Unit *                  GetSecondMostHatedTargetInArray( UnitArray& pTargetArray );
    bool                    IsValidUnitTarget( Object *pObject, TargetFilters pFilter, float pMinRange = 0.0f, float pMaxRange = 0.0f  );
    void                    PushRunToTargetCache( Unit *pTarget, SpellDesc* pSpell, RangeStatusPair pRangeStatus = make_pair( RangeStatus_TooFar, 0.0f ) );
    void                    PopRunToTargetCache();

    void                    RandomEmote(EmoteArray& pEmoteArray);

    SpellDescArray          mSpells;
    SpellDescList           mQueuedSpells;
    SpellDescList           mScheduledSpells;

    Unit *                  mRunToTargetCache;
    SpellDesc*              mRunToTargetSpellCache;

    EmoteArray              mOnCombatStartEmotes;
    EmoteArray              mOnTargetDiedEmotes;
    EmoteArray              mOnDiedEmotes;
    EmoteArray              mOnTauntEmotes;

    TimerArray              mTimers;
    int32                   mTimerIdCounter;
    uint32                  mAIUpdateFrequency;
    uint32                  mBaseAttackTime;
    bool                    mDespawnWhenInactive;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class MoonScriptBossAI
class SCRIPT_DECL MoonScriptBossAI : public MoonScriptCreatureAI
{
public:
    MoonScriptBossAI(Creature *pCreature);
    virtual ~MoonScriptBossAI();

    //Basic Interface
    SpellDesc*      AddPhaseSpell(int32 pPhase, SpellDesc* pSpell);
    int32           GetPhase();
    void            SetPhase(int32 pPhase, SpellDesc* pPhaseChangeSpell=NULL);
    void            SetEnrageInfo(SpellDesc* pSpell, int32 pTriggerMilliseconds);

    //Reimplemented Events
    virtual void    AIUpdate(MapManagerScript* MMScript, uint32 p_time);
    virtual void    OnCombatStart(Unit *pTarget);
    virtual void    OnCombatStop(Unit *pTarget);

protected:
    int32           mPhaseIndex;
    PhaseSpellArray mPhaseSpells;
    SpellDesc*      mEnrageSpell;
    int32           mEnrageTimerDuration;
    int32           mEnrageTimer;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//STL Utilities
template <class Type> inline void DeleteArray(std::vector<Type> pVector)
{
    typename std::vector<Type>::iterator Iter = pVector.begin();
    for( ; Iter != pVector.end(); ++Iter )
    {
        delete (*Iter);
    }
    pVector.clear();
}

//Warning: do not use if item can reside more than once in same vector
template <class Type> inline void DeleteItem(std::vector<Type> pVector, Type pItem)
{
    typename std::vector<Type>::iterator Iter = std::find(pVector.begin(), pVector.end(), pItem);
    if( Iter != pVector.end() )
    {
        delete (*Iter);
        pVector.erase(Iter);
    }
}

struct ScriptSpell
{
    uint32 normal_spellid;
    uint32 heroic_spellid;
    uint32 timer;
    uint32 time;
    uint32 chance;
    uint32 target;
    uint32 phase;
};

enum SPELL_TARGETS
{
    SPELL_TARGET_SELF,
    SPELL_TARGET_CURRENT_ENEMY,
    SPELL_TARGET_RANDOM_PLAYER,
    SPELL_TARGET_SUMMONER,
    SPELL_TARGET_RANDOM_PLAYER_POSITION,
    SPELL_TARGET_GENERATE,
    SPELL_TARGET_LOWEST_THREAT,
    SPELL_TARGET_CUSTOM,
};

struct SP_AI_Spell
{
    SpellEntry *info;       // spell info
    char targettype;        // 0-self , 1-attaking target, ....
    bool instant;           // does it is instant or not?
    float perctrigger;      // % of the cast of this spell in a total of 100% of the attacks
    int attackstoptimer;    // stop the creature from attacking
    int soundid;            // sound id from DBC
    std::string speech;     // text displaied when spell was casted
    uint32 cooldown;        // spell cooldown
    uint32 casttime;        // "time" left to cast spell
    uint32 reqlvl;          // required level ? needed?
    float hpreqtocast;      // ? needed?
    float mindist2cast;     // min dist from caster to victim to perform cast (dist from caster >= mindist2cast)
    float maxdist2cast;     // max dist from caster to victim to perform cast (dist from caster <= maxdist2cast)
    int minhp2cast;         // min hp amount of victim to perform cast on it (health >= minhp2cast)
    int maxhp2cast;         // max hp amount of victim to perform cast on it (health <= maxhp2cast)
};

enum
{
    TARGET_SELF,
    TARGET_VARIOUS,
    TARGET_ATTACKING,
    TARGET_DESTINATION,
    TARGET_SOURCE,
    TARGET_RANDOM_FRIEND,
    TARGET_RANDOM_SINGLE,
    TARGET_RANDOM_DESTINATION,
};

#endif /* BASE_H */
