/***
 * Demonstrike Core
 */

#pragma once

class SpellCastTargets
{
public:
    void read ( WorldPacket & data, uint64 caster );
    void write ( WorldPacket & data);

    SpellCastTargets() : m_castFlags(0), m_targetIndex(0), m_targetMask(0), m_src(0.f, 0.f, 0.f), m_dest(0.f, 0.f, 0.f), m_dest_transGuid(0), m_src_transGuid(0),
        m_unitTarget(0), m_itemTarget(0), missilespeed(0), missilepitch(0), traveltime(0) { }

    SpellCastTargets(uint8 castFlags, uint32 targetIndex, uint32 TargetMask, uint64 unitTarget, uint64 itemTarget, float srcX, float srcY, float srcZ, float destX, float destY, float destZ)
        : m_castFlags(castFlags), m_targetIndex(targetIndex), m_targetMask(TargetMask), m_src(srcX, srcY, srcZ), m_dest(destX, destY, destZ), m_dest_transGuid(0), m_src_transGuid(0),
        m_unitTarget(unitTarget), m_itemTarget(itemTarget), missilespeed(0), missilepitch(0), traveltime(0) { }

    SpellCastTargets(uint64 unitTarget) : m_castFlags(0), m_targetIndex(0), m_targetMask(0x2), m_src(0.f, 0.f, 0.f), m_dest(0.f, 0.f, 0.f), m_dest_transGuid(0), m_src_transGuid(0),
        m_unitTarget(unitTarget), m_itemTarget(0), missilespeed(0), missilepitch(0), traveltime(0) { }

    SpellCastTargets(WorldPacket & data, uint64 caster) : m_castFlags(0), m_targetIndex(0), m_targetMask(0), m_src(0.f, 0.f, 0.f), m_dest(0.f, 0.f, 0.f),
        missilespeed(0), missilepitch(0), traveltime(0) { m_unitTarget = m_itemTarget = m_dest_transGuid = m_src_transGuid = 0; read(data, caster); }

    SpellCastTargets& operator=(const SpellCastTargets &target)
    {
        m_castFlags = target.m_castFlags;
        m_targetIndex = target.m_targetIndex;
        m_targetMask = target.m_targetMask;
        m_unitTarget = target.m_unitTarget;
        m_itemTarget = target.m_itemTarget;

        m_src = target.m_src;
        m_dest = target.m_dest;

        traveltime = target.traveltime;
        missilespeed = target.missilespeed;
        missilepitch = target.missilepitch;
        m_strTarget = target.m_strTarget;

        m_dest_transGuid = target.m_dest_transGuid;
        m_src_transGuid = target.m_src_transGuid;
        return *this;
    }

    uint8 m_castFlags;
    uint32 m_targetIndex, m_targetMask;
    WoWGuid m_unitTarget, m_itemTarget;

    WoWGuid m_src_transGuid, m_dest_transGuid;
    LocationVector m_src, m_dest;
    float traveltime, missilespeed, missilepitch;
    std::string m_strTarget;
};

enum SpellEffectTargetFlags
{
    EFF_TARGET_FLAGS_NONE       = 0,
    EFF_TARGET_FLAGS_GAMEOBJ    = 1,
    EFF_TARGET_FLAGS_UNIT       = 2,
    EFF_TARGET_FLAGS_PLAYER     = 3
};

#define GO_FISHING_BOBBER 35591

#define SPELL_SPELL_CHANNEL_UPDATE_INTERVAL 1000

struct SpellTarget
{
    uint64 Guid;
    uint8 HitResult;
    uint8 EffectMask;
    uint8 ReflectResult;
    uint32 DestinationTime;
};

typedef std::map<uint64, SpellTarget> SpellTargetMap;

// Spell instance
class SERVER_DECL Spell : public SpellEffectClass
{
public:
    Spell( WorldObject* Caster, SpellEntry *info, uint8 castNumber = 0, Aura* aur = NULL);
    ~Spell();
    virtual void Destruct();

    void GetSpellDestination(LocationVector &dest)
    {
        SpellEffectClass::GetSpellDestination(dest);
        if((m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION) == 0)
            return;
        dest = m_targets.m_dest;
    }

    float m_missilePitch;
    uint32 m_missileTravelTime, m_MSTimeToAddToTravel;

    // Fills specified targets at the area of effect
    void FillSpecifiedTargetsInArea(float srcx,float srcy,float srcz,uint32 ind, uint32 specification);
    // Fills specified targets at the area of effect. We suppose we already inited this spell and know the details
    void FillSpecifiedTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float range, uint32 specification);
    // Fills the targets at the area of effect
    void FillAllTargetsInArea(uint32 i, float srcx,float srcy,float srcz, float range, bool includegameobjects = false);
    // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
    void FillAllTargetsInArea(float srcx,float srcy,float srcz,uint32 ind);
    // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
    void FillAllTargetsInArea(LocationVector & location,uint32 ind);
    // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
    void FillAllFriendlyInArea(uint32 i, float srcx,float srcy,float srcz, float range);
    // Fills the gameobject targets at the area of effect
    void FillAllGameObjectTargetsInArea(uint32 i, float srcx,float srcy,float srcz, float range);
    //get single Enemy as target
    uint64 GetSinglePossibleEnemy(uint32 i, float prange=0);
    //get single Enemy as target
    uint64 GetSinglePossibleFriend(uint32 i, float prange=0);
    //generate possible target list for a spell. Use as last resort since it is not acurate
    bool GenerateTargets(SpellCastTargets *t);
    // Fills the target map of the spell packet
    void FillTargetMap(uint32);
    // Prepares the spell thats going to cast to targets
    uint8 prepare(SpellCastTargets *targets, bool triggered);
    // Cancels the current spell
    void cancel();
    // Update spell state based on time difference
    void Update(uint32 difftime);
    void updatePosition(float x, float y, float z);
    void _UpdateChanneledSpell(uint32 difftime);
    // Casts the spell
    void cast(bool);
    // Finishes the casted spell
    void finish();
    // Handle the Effects of the Spell
    void HandleEffects(uint32 i, WorldObject *target);
    // Cancel spell effect due to things like a glyph
    bool CanHandleSpellEffect(uint32 i);
    // Take Power from the caster based on spell power usage
    bool TakePower();
    // Has power?
    bool HasPower();
    // Calculate power to take
    int32 CalculateCost(int32 &powerField);
    // Checks the caster is ready for cast
    uint8 CanCast(bool tolerate);
    // Removes reagents, ammo, and items/charges
    void RemoveItems();
    // Calculates the i'th effect value
    int32 CalculateEffect(uint32 effIndex, WorldObject* target);
    // Handles Teleport function
    void HandleTeleport(uint32 id, Unit* Target);
    // Determines how much skill caster going to gain
    void DetermineSkillUp();
    // Increases cast time of the spell
    void AddTime(uint32 type);
    // Get Target Type
    uint32 GetTargetType(uint32 implicittarget, uint32 i);
    void AddCooldown();
    void AddStartCooldown();

    bool Reflect(Unit* refunit);

    RONIN_INLINE uint32 getState() { return m_spellState; }
    RONIN_INLINE SpellEntry *GetSpellProto() { return m_spellInfo; }

    // Send Packet functions
    bool IsNeedSendToClient();
    void SendSpellStart();
    void SendSpellGo();
    void SendProjectileUpdate();
    void SendResurrectRequest(Player* target);
    static void SendHealSpellOnPlayer(WorldObject* caster, WorldObject* target, uint32 dmg, bool critical, uint32 overheal, uint32 spellid);
    static void SendHealManaSpellOnPlayer(WorldObject* caster, WorldObject* target, uint32 dmg, uint32 powertype, uint32 spellid);


    void HandleAddAura(Unit *target);
    void writeSpellGoTargets( WorldPacket * data );

    uint32 m_triggeredSpellId;
    SpellEntry *ProcedOnSpell;  //some spells need to know the origins of the proc too
    SpellCastTargets m_targets;

    void CreateItem(uint32 itemId);

    // Spell Targets
    void HandleTargetNoObject();
    bool AddTarget(uint32 i, uint32 TargetType, WorldObject* obj);
    void AddAOETargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddPartyTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddRaidTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets, bool partylimit = false);
    void AddChainTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddConeTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddScriptedOrSpellFocusTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);

    uint64 static FindLowestHealthRaidMember(Player* Target, uint32 dist);

    void Heal(Unit *target, uint8 effIndex, int32 amount);

    std::map<uint64, Aura*> m_tempAuras;

    bool SpellEffectUpdateQuest(uint32 questid);

    // 15007 = resurecting sickness

    // This returns SPELL_ENTRY_Spell_Dmg_Type where 0 = SPELL_DMG_TYPE_NONE, 1 = SPELL_DMG_TYPE_MAGIC, 2 = SPELL_DMG_TYPE_MELEE, 3 = SPELL_DMG_TYPE_RANGED
    // It should NOT be used for weapon_damage_type which needs: 0 = MELEE, 1 = OFFHAND, 2 = RANGED
    RONIN_INLINE uint32 GetType() { return ( GetSpellProto()->Spell_Dmg_Type == SPELL_DMG_TYPE_NONE ? SPELL_DMG_TYPE_MAGIC : GetSpellProto()->Spell_Dmg_Type ); }

    int32 chaindamage;
    // -------------------------------------------
    static bool IsBinary(SpellEntry * sp);

    RONIN_INLINE static bool HasMechanic(SpellEntry * sp, uint32 MechanicsType)
    {
        if(sp->MechanicsType == MechanicsType)
            return true;
        for(uint8 i = 0; i < 3; i++)
            if(sp->EffectMechanic[i] == MechanicsType)
                return true;
        return false;
    }

    RONIN_INLINE static uint32 GetMechanic(SpellEntry * sp)
    {
        if(sp->MechanicsType)
            return sp->MechanicsType;
        if(sp->EffectMechanic[2])
            return sp->EffectMechanic[2];
        if(sp->EffectMechanic[1])
            return sp->EffectMechanic[1];
        if(sp->EffectMechanic[0])
            return sp->EffectMechanic[0];
        return 0;
    }

    RONIN_INLINE static uint32 GetMechanicOfEffect(SpellEntry * sp, uint32 i)
    {
        if(sp->EffectMechanic[i])
            return sp->EffectMechanic[i];
        if(sp->MechanicsType)
            return sp->MechanicsType;
        return 0;
    }

    bool IsAuraApplyingSpell();
    bool IsStealthSpell();
    bool IsInvisibilitySpell();
    bool CanEffectTargetGameObjects(uint32 i);

    int32 damage;
    Aura* m_triggeredByAura;

    bool static_damage;
    bool m_triggeredSpell;
    bool m_AreaAura;
    //uint32 TriggerSpellId;        // used to set next spell to use
    //uint64 TriggerSpellTarget;    // used to set next spell target
    float m_castPositionX;
    float m_castPositionY;
    float m_castPositionZ;

    int32 damageToHit;
    uint32 castedItemId;
    uint32 m_pushbackCount;

    bool duelSpell;

    RONIN_INLINE void safe_cancel()
    {
        m_cancelled = true;
    }

    Spell* m_reflectedParent;
    uint32 m_ForcedCastTime;

    // Returns true iff spellEffect's effectNum effect affects testSpell based on EffectSpellClassMask
    RONIN_INLINE static bool EffectAffectsSpell(SpellEntry* spellEffect, uint32 effectNum, SpellEntry* testSpell)
    {
        return ((testSpell->SpellGroupType[0] && (spellEffect->EffectSpellClassMask[effectNum][0] & testSpell->SpellGroupType[0])) ||
            (testSpell->SpellGroupType[1] && (spellEffect->EffectSpellClassMask[effectNum][1] & testSpell->SpellGroupType[1])) ||
            (testSpell->SpellGroupType[2] && (spellEffect->EffectSpellClassMask[effectNum][2] & testSpell->SpellGroupType[2])));
    }

    RONIN_INLINE uint32 GetDifficultySpell(SpellEntry * sp, uint32 difficulty)
    {
        uint32 spellid = 0;
        SpellDifficultyEntry * sd = dbcSpellDifficulty.LookupEntry(sp->SpellDifficulty);
        if(sd != NULL && sd->SpellId[difficulty] != 0 )
            if(dbcSpell.LookupEntry(sd->SpellId[difficulty]) != NULL)
                spellid = sd->SpellId[difficulty];
        return spellid;
    }

protected:

    /// Spell state's
    bool    m_usesMana;
    bool    m_Delayed;
    int32   m_castTime;
    int32   m_timer;
    bool    m_ForceConsumption;

    // Current Targets to be used in effect handler
    WoWGuid objTargetGuid, itemTargetGuid, m_magnetTarget;

    uint8  cancastresult;
    bool   m_cancelled;
    bool   m_isCasting;
    bool   m_projectileWait;

    void DamageGosAround(uint32 i);
    bool UseMissileDelay();
    bool HasSpellEffect( uint32 effect )
    {
        for( uint32 i = 0; i < 3; ++i )
            if( GetSpellProto()->Effect[ i ] == effect )
                return true;
        return false;
    }

private:
    SpellTargetMap m_fullTargetMap, m_effectTargetMaps[3];

    // adds a target to the list, performing DidHit checks on units
    void _AddTarget(WorldObject* target, const uint32 effectid);

    // didhit checker
    uint8 _DidHit(uint32 index, Unit* target, uint8 *reflectout = NULL);

    void DetermineSkillUp(uint32 skillid,uint32 targetlevel, uint32 multiplicator = 1);
    void DetermineSkillUp(uint32 skillid);

    uint32 m_hitTargetCount, m_missTargetCount;

public:
    static std::map<uint8, uint32> m_implicitTargetFlags;
};

void ApplyDiminishingReturnTimer(int32 * Duration, Unit* Target, SpellEntry * spell);
void UnapplyDiminishingReturnTimer(Unit* Target, SpellEntry * spell);
uint32 GetDiminishingGroup(uint32 NameHash);
uint32 GetTriggerSpellFromDescription(std::string delimiter, std::string desc);
SpellEntry* CreateDummySpell(uint32 id);
uint32 GetSpellClass(SpellEntry *sp);
uint32 fill( uint32* arr, ... );
void GenerateNameHashesFile();
void GenerateSpellCoeffFile();
void CopyEffect(SpellEntry *fromSpell, uint8 fromEffect, SpellEntry *toSpell, uint8 toEffect);
void ApplySingleSpellFixes(SpellEntry *sp);
void ApplyCoeffSpellFixes(SpellEntry *sp);
void SetProcFlags(SpellEntry *sp);
void SetupSpellTargets();
