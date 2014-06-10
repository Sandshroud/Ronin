/***
 * Demonstrike Core
 */

#pragma once

class SpellCastTargets
{
public:
    void read ( WorldPacket & data,uint64 caster, uint8 castFlags = 0 );
    void write ( WorldPacket & data);

    SpellCastTargets() : m_targetMask(0), m_unitTarget(0), m_itemTarget(0), m_srcX(0), m_srcY(0), m_srcZ(0),
        m_destX(0), m_destY(0), m_destZ(0), missilespeed(0), missilepitch(0), traveltime(0), m_dest_transGuid(), m_src_transGuid() {}

    SpellCastTargets(uint16 TargetMask, uint64 unitTarget, uint64 itemTarget, float srcX, float srcY, float srcZ, float destX, float destY, float destZ)
        : m_targetMask(TargetMask), m_unitTarget(unitTarget), m_itemTarget(itemTarget), m_srcX(srcX), m_srcY(srcY), m_srcZ(srcZ),
        m_destX(destX), m_destY(destY), m_destZ(destZ), missilespeed(0), missilepitch(0), traveltime(0), m_dest_transGuid(), m_src_transGuid() {}

    SpellCastTargets(uint64 unitTarget) : m_targetMask(0x2), m_unitTarget(unitTarget), m_itemTarget(0), m_srcX(0), m_srcY(0), m_srcZ(0),
        m_destX(0), m_destY(0), m_destZ(0), missilespeed(0), missilepitch(0), traveltime(0), m_dest_transGuid(), m_src_transGuid() {}

    SpellCastTargets(WorldPacket & data, uint64 caster) : m_targetMask(0), m_unitTarget(0), m_itemTarget(0), m_srcX(0), m_srcY(0), m_srcZ(0),
        m_destX(0), m_destY(0), m_destZ(0), missilespeed(0), missilepitch(0), traveltime(0), m_dest_transGuid(), m_src_transGuid()
    {
        read(data, caster);
    }

    SpellCastTargets& operator=(const SpellCastTargets &target)
    {
        m_targetMask = target.m_targetMask;
        m_unitTarget = target.m_unitTarget;
        m_itemTarget = target.m_itemTarget;

        m_srcX = target.m_srcX;
        m_srcY = target.m_srcY;
        m_srcZ = target.m_srcZ;

        m_destX = target.m_destX;
        m_destY = target.m_destY;
        m_destZ = target.m_destZ;

        traveltime = target.traveltime;
        missilespeed = target.missilespeed;
        missilepitch = target.missilepitch;
        m_strTarget = target.m_strTarget;

        m_dest_transGuid = target.m_dest_transGuid;
        m_src_transGuid = target.m_src_transGuid;
        return *this;
    }

    uint32 m_targetMask;
    uint64 m_unitTarget;
    uint64 m_itemTarget;

    WoWGuid m_src_transGuid, m_dest_transGuid;
    float m_srcX, m_srcY, m_srcZ;
    float m_destX, m_destY, m_destZ;
    float traveltime, missilespeed, missilepitch;
    std::string m_strTarget;
};

typedef void(Spell::*pSpellEffect)(uint32 i);

#define GO_FISHING_BOBBER 35591

#define SPELL_SPELL_CHANNEL_UPDATE_INTERVAL 1000

struct SpellTarget
{
    uint64 Guid;
    uint8 HitResult;
    uint8 EffectMask;
    uint8 ReflectResult;
    int64 DestinationTime;
};

typedef std::map<uint64, SpellTarget> SpellTargetMap;

// Spell instance
class SERVER_DECL Spell : public EventableObject
{
public:
    friend class DummySpellHandler;
    Spell( Object* Caster, SpellEntry *info, bool triggered, Aura* aur);
    ~Spell();
    virtual void Destruct();

    float m_missilePitch;
    uint32 m_missileTravelTime;
    uint32 MSTimeToAddToTravel;

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
    uint8 prepare(SpellCastTargets * targets);
    // Cancels the current spell
    void cancel();
    // Update spell state based on time difference
    void update(uint32 difftime);
    void updatePosition(float x, float y, float z);
    // Casts the spell
    void cast(bool);
    // Finishes the casted spell
    void finish();
    // Handle the Effects of the Spell
    void HandleEffects(uint32 i);
    // Cancel spell effect due to things like a glyph
    bool CanHandleSpellEffect(uint32 i, uint32 namehash);
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
    int32 CalculateEffect(uint32, Unit* target);
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
    void CalcDestLocationHit();
    bool Reflect(Unit* refunit);
    uint32 StartDestLocationCastTime;
    void HandleRemoveDestTarget(uint64 guid);
    bool HandleDestTargetHit(uint64 guid, uint32 MSTime);

    HEARTHSTONE_INLINE uint32 getState() { return m_spellState; }
    HEARTHSTONE_INLINE void SetUnitTarget(Unit* punit){unitTarget=punit;}
    HEARTHSTONE_INLINE SpellEntry *GetSpellProto() { return m_spellInfo; }

    HEARTHSTONE_INLINE bool RequiresComboPoints(SpellEntry const* spellInfo)
    {
        if(spellInfo->AttributesEx & ATTRIBUTESEX_REQ_COMBO_POINTS1 || spellInfo->AttributesEx & ATTRIBUTESEX_REQ_COMBO_POINTS2 || spellInfo->c_is_flags & SPELL_FLAG_IS_FINISHING_MOVE)
            return true;
        return false;
    }

    // Send Packet functions
    bool IsNeedSendToClient();
    void SendCastResult(uint8 result);
    void SendSpellStart();
    void SendSpellGo();
    void SendProjectileUpdate();
    void SendInterrupted(uint8 result);
    void SendChannelUpdate(uint32 time);
    void SendChannelStart(int32 duration);
    void SendResurrectRequest(Player* target);
    static void SendHealSpellOnPlayer(Object* caster, Object* target, uint32 dmg, bool critical, uint32 overheal, uint32 spellid);
    static void SendHealManaSpellOnPlayer(Object* caster, Object* target, uint32 dmg, uint32 powertype, uint32 spellid);


    void HandleAddAura(uint64 guid);
    void writeSpellGoTargets( WorldPacket * data );

    SpellEntry* m_spellInfo;
    uint32 pSpellId;
    SpellEntry *ProcedOnSpell;  //some spells need to know the origins of the proc too
    SpellCastTargets m_targets;

    void CreateItem(uint32 itemId);

    // Effect Handlers
    void SpellEffectNULL(uint32 i);
    void SpellEffectInstantKill(uint32 i);
    void SpellEffectSchoolDMG(uint32 i);
    void SpellEffectDummy(uint32 i);
    void SpellEffectRestoreHealthPct(uint32 i);
    void SpellEffectRestoreManaPct(uint32 i);
    void SpellEffectTeleportUnits(uint32 i);
    void SpellEffectApplyAura(uint32 i);
    void SpellEffectPowerDrain(uint32 i);
    void SpellEffectHealthLeech(uint32 i);
    void SpellEffectHeal(uint32 i);
    void SpellEffectBind(uint32 i);
    void SpellEffectQuestComplete(uint32 i);
    void SpellEffectWeapondamageNoschool(uint32 i);
    void SpellEffectResurrect(uint32 i);
    void SpellEffectAddExtraAttacks(uint32 i);
    void SpellEffectDodge(uint32 i);
    void SpellEffectBlock(uint32 i);
    void SpellEffectParry(uint32 i);
    void SpellEffectCreateItem(uint32 i);
    void SpellEffectPersistentAA(uint32 i);
    void SpellEffectSummon(uint32 i);
    void SpellEffectLeap(uint32 i);
    void SpellEffectEnergize(uint32 i);
    void SpellEffectWeaponDmgPerc(uint32 i);
    void SpellEffectTriggerMissile(uint32 i);
    void SpellEffectOpenLock(uint32 i);
    void SpellEffectApplyAA(uint32 i);
    void SpellEffectLearnSpell(uint32 i);
    void SpellEffectSpellDefense(uint32 i);
    void SpellEffectDispel(uint32 i);
    void SpellEffectSkillStep(uint32 i);
    void SpellEffectDetect(uint32 i);
    void SpellEffectSummonObject(uint32 i);
    void SpellEffectEnchantItem(uint32 i);
    void SpellEffectEnchantItemTemporary(uint32 i);
    void SpellEffectTameCreature(uint32 i);
    void SpellEffectSummonPet(uint32 i);
    void SpellEffectWeapondamage(uint32 i);
    void SpellEffectPowerBurn(uint32 i);
    void SpellEffectThreat(uint32 i);
    void SpellEffectTriggerSpell(uint32 i);
    void SpellEffectHealthFunnel(uint32 i);
    void SpellEffectPowerFunnel(uint32 i);
    void SpellEffectHealMaxHealth(uint32 i);
    void SpellEffectInterruptCast(uint32 i);
    void SpellEffectDistract(uint32 i);
    void SpellEffectPickpocket(uint32 i);
    void SpellEffectAddFarsight(uint32 i);
    void SpellEffectResetTalents(uint32 i);
    void SpellEffectUseGlyph(uint32 i);
    void SpellEffectHealMechanical(uint32 i);
    void SpellEffectSummonObjectWild(uint32 i);
    void SpellEffectScriptEffect(uint32 i);
    void SpellEffectSanctuary(uint32 i);
    void SpellEffectAddComboPoints(uint32 i);
    void SpellEffectCreateHouse(uint32 i);
    void SpellEffectDuel(uint32 i);
    void SpellEffectStuck(uint32 i);
    void SpellEffectSummonPlayer(uint32 i);
    void SpellEffectActivateObject(uint32 i);
    void SpellEffectWMODamage(uint32 i);
    void SpellEffectWMORepair(uint32 i);
    void SpellEffectChangeWMOState(uint32 i);
    void SpellEffectProficiency(uint32 i);
    void SpellEffectSendEvent(uint32 i);
    void SpellEffectSkinning(uint32 i);
    void SpellEffectCharge(uint32 i);
    void SpellEffectPlaceTotemsOnBar(uint32 i);
    void SpellEffectKnockBack(uint32 i);
    void SpellEffectInebriate(uint32 i);
    void SpellEffectFeedPet(uint32 i);
    void SpellEffectDismissPet(uint32 i);
    void SpellEffectReputation(uint32 i);
    void SpellEffectSummonObjectSlot(uint32 i);
    void SpellEffectDispelMechanic(uint32 i);
    void SpellEffectSummonDeadPet(uint32 i);
    void SpellEffectDestroyAllTotems(uint32 i);
    void SpellEffectSummonDemonOld(uint32 i);
    void SpellEffectAttackMe(uint32 i);
    void SpellEffectSkill(uint32 i);
    void SpellEffectApplyPetAura(uint32 i);
    void SpellEffectDummyMelee(uint32 i);
    void SpellEffectPull( uint32 i );
    void SpellEffectSpellSteal(uint32 i);
    void SpellEffectProspecting(uint32 i);
    void SpellEffectOpenLockItem(uint32 i);
    void SpellEffectSelfResurrect(uint32 i);
    void SpellEffectDisenchant(uint32 i);
    void SpellEffectWeapon(uint32 i);
    void SpellEffectDefense(uint32 i);
    void SpellEffectLanguage(uint32 i);
    void SpellEffectDualWield(uint32 i);
    void SpellEffectSkinPlayerCorpse(uint32 i);
    void SpellEffectResurrectNew(uint32 i);
    void SpellEffectTranformItem(uint32);
    void SpellEffectEnvironmentalDamage(uint32);
    void SpellEffectLearnPetSpell(uint32 i);
    void SpellEffectEnchantHeldItem(uint32 i);
    void SpellEffectAddHonor(uint32 i);
    void SpellEffectSpawn(uint32 i);
    void SpellEffectApplyAura128(uint32 i);
    void SpellEffectRedirectThreat(uint32 i);
    void SpellEffectPlayMusic(uint32 i);
    void SpellEffectTriggerSpellWithValue(uint32 i);
    void SpellEffectJump(uint32 i);
    void SpellEffectTeleportToCaster(uint32 i);
    void SpellEffectMilling(uint32 i);
    void SpellEffectAllowPetRename(uint32 i);
    void SpellEffectAddPrismaticSocket(uint32 i);
    void SpellEffectActivateRune(uint32 i);
    void SpellEffectFailQuest(uint32 i);
    void SpellEffectStartQuest(uint32 i);
    void SpellEffectCreatePet(uint32 i);
    void SpellEffectTitanGrip(uint32 i);
    void SpellEffectCreateRandomItem(uint32 i);
    void SpellEffectSendTaxi(uint32 i);
    void SpellEffectKillCredit(uint32 i);
    void SpellEffectForceCast(uint32 i);
    void SpellEffectSetTalentSpecsCount(uint32 i);
    void SpellEffectActivateTalentSpec(uint32 i);
    void SpellEffectDisengage(uint32 i);
    void SpellEffectClearFinishedQuest(uint32 i);
    void SpellEffectApplyDemonAura(uint32 i);
    void SpellEffectRemoveAura(uint32 i);

    // Summons
    void SummonWild(uint32 i, SummonPropertiesEntry* spe, CreatureProto* proto, LocationVector & v);
    void SummonTotem(uint32 i, SummonPropertiesEntry* spe, CreatureProto* proto, LocationVector & v);
    void SummonGuardian(uint32 i, SummonPropertiesEntry* spe, CreatureProto* proto, LocationVector & v);
    void SummonTemporaryPet(uint32 i, SummonPropertiesEntry* spe, CreatureProto* proto, LocationVector & v);
    void SummonPossessed(uint32 i, SummonPropertiesEntry* spe, CreatureProto* proto, LocationVector & v);
    void SummonCompanion(uint32 i, SummonPropertiesEntry* spe, CreatureProto* proto, LocationVector & v);
    void SummonVehicle( uint32 i, SummonPropertiesEntry *spe, CreatureProto *proto, LocationVector &v );

    // Spell Targets
    void HandleTargetNoObject();
    bool AddTarget(uint32 i, uint32 TargetType, Object* obj);
    void AddAOETargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddPartyTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddRaidTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets, bool partylimit = false);
    void AddChainTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddConeTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddScriptedOrSpellFocusTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);

    uint64 static FindLowestHealthRaidMember(Player* Target, uint32 dist);

    void Heal(int32 amount);

    GameObject*         g_caster;
    Unit*               u_caster;
    Item*               i_caster;
    Player*             p_caster;
    Object*             m_caster;
    Vehicle*            v_caster;

    bool SpellEffectUpdateQuest(uint32 questid);

    // 15007 = resurecting sickness

    // This returns SPELL_ENTRY_Spell_Dmg_Type where 0 = SPELL_DMG_TYPE_NONE, 1 = SPELL_DMG_TYPE_MAGIC, 2 = SPELL_DMG_TYPE_MELEE, 3 = SPELL_DMG_TYPE_RANGED
    // It should NOT be used for weapon_damage_type which needs: 0 = MELEE, 1 = OFFHAND, 2 = RANGED
    HEARTHSTONE_INLINE uint32 GetType() { return ( GetSpellProto()->Spell_Dmg_Type == SPELL_DMG_TYPE_NONE ? SPELL_DMG_TYPE_MAGIC : GetSpellProto()->Spell_Dmg_Type ); }
    HEARTHSTONE_INLINE Item* GetItemTarget() { return itemTarget; }
    HEARTHSTONE_INLINE Unit* GetUnitTarget() { return unitTarget; }
    HEARTHSTONE_INLINE Player* GetPlayerTarget() { return playerTarget; }
    HEARTHSTONE_INLINE GameObject* GetGameObjectTarget() { return gameObjTarget; }

    uint32 chaindamage;
    // -------------------------------------------

    bool IsAspect();
    bool IsSeal();
    static bool IsTotemSpell(SpellEntry* sp);
    static bool IsBinary(SpellEntry * sp);

    int32 GetDuration()
    {
        if(bDurSet)
            return Dur;

        bDurSet = true;
        Dur = GetSpellInfoDuration(m_spellInfo, u_caster, unitTarget);
        return Dur;
    }

    HEARTHSTONE_INLINE float GetRadius(uint32 i)
    {
        if(bRadSet[i])
            return Rad[i];
        bRadSet[i] = true;
        Rad[i] = ::GetDBCRadius(dbcSpellRadius.LookupEntry(GetSpellProto()->EffectRadiusIndex[i]));
        if(GetSpellProto()->SpellGroupType && u_caster)
        {
            SM_FFValue(u_caster->SM[SMT_RADIUS][0],&Rad[i],GetSpellProto()->SpellGroupType);
            SM_PFValue(u_caster->SM[SMT_RADIUS][1],&Rad[i],GetSpellProto()->SpellGroupType);
        }

        return Rad[i];
    }

    HEARTHSTONE_INLINE float GetFriendlyRadius(uint32 i)
    {
        if(bRadSet[i])
            return Rad[i];
        bRadSet[i] = true;
        Rad[i] = ::GetDBCFriendlyRadius(dbcSpellRadius.LookupEntry(GetSpellProto()->EffectRadiusIndex[i]));
        if(GetSpellProto()->SpellGroupType && u_caster)
        {
            SM_FFValue(u_caster->SM[SMT_RADIUS][0],&Rad[i],GetSpellProto()->SpellGroupType);
            SM_PFValue(u_caster->SM[SMT_RADIUS][1],&Rad[i],GetSpellProto()->SpellGroupType);
        }

        return Rad[i];
    }

    HEARTHSTONE_INLINE static uint32 GetBaseThreat(uint32 dmg)
    {
        //there should be a formula to determine what spell cause threat and which don't
/*      switch(GetSpellProto()->NameHash)
        {
            //hunter's mark
            case 4287212498:
                {
                    return 0;
                }break;
        }*/
        return dmg;
    }

    HEARTHSTONE_INLINE static bool HasMechanic(SpellEntry * sp, uint32 MechanicsType)
    {
        return sp->MechanicsType == MechanicsType ||
            (uint32)sp->EffectMechanic[0] == MechanicsType ||
            (uint32)sp->EffectMechanic[1] == MechanicsType ||
            (uint32)sp->EffectMechanic[2] == MechanicsType;
    }

    HEARTHSTONE_INLINE static uint32 GetMechanic(SpellEntry * sp)
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

    HEARTHSTONE_INLINE static uint32 GetMechanicOfEffect(SpellEntry * sp, uint32 i)
    {
        if(sp->EffectMechanic[i])
            return sp->EffectMechanic[i];
        if(sp->MechanicsType)
            return sp->MechanicsType;

        return 0;
    }

    bool IsStealthSpell();
    bool IsInvisibilitySpell();

    int32 damage;
    int32 TotalDamage;
    Aura* m_triggeredByAura;
    signed int forced_basepoints[3]; //some talent inherit base points from previous caster spells
    uint32 AdditionalCritChance;

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
    uint8 extra_cast_number;
    uint32 m_glyphIndex;
    uint32 m_pushbackCount;

    bool duelSpell;

    HEARTHSTONE_INLINE void safe_cancel()
    {
        m_cancelled = true;
    }

    /// Spell state's
    /// Spell failed
    HEARTHSTONE_INLINE bool GetSpellFailed(){return m_Spell_Failed;}
    HEARTHSTONE_INLINE void SetSpellFailed(bool failed = true) { m_Spell_Failed = failed; }

    Spell* m_reflectedParent;
    uint32 m_ForcedCastTime;

    // Returns true iff spellEffect's effectNum effect affects testSpell based on EffectSpellClassMask
    HEARTHSTONE_INLINE static bool EffectAffectsSpell(SpellEntry* spellEffect, uint32 effectNum, SpellEntry* testSpell)
    {
        return ((testSpell->SpellGroupType[0] && (spellEffect->EffectSpellClassMask[effectNum][0] & testSpell->SpellGroupType[0])) ||
            (testSpell->SpellGroupType[1] && (spellEffect->EffectSpellClassMask[effectNum][1] & testSpell->SpellGroupType[1])) ||
            (testSpell->SpellGroupType[2] && (spellEffect->EffectSpellClassMask[effectNum][2] & testSpell->SpellGroupType[2])));
    }

    HEARTHSTONE_INLINE uint32 GetDifficultySpell(SpellEntry * sp, uint32 difficulty)
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
    bool    m_Spell_Failed;     //for 5sr
    bool    m_Delayed;
    uint32  m_spellState;
    int32   m_castTime;
    int32   m_timer;
    bool    m_ForceConsumption;

    // Current Targets to be used in effect handler
    Unit*           unitTarget;
    Item*           itemTarget;
    GameObject*     gameObjTarget;
    Player*         playerTarget;
    Corpse*         corpseTarget;
    uint32          add_damage;

    uint8           cancastresult;
    int32           Dur;
    bool            bDurSet;
    float           Rad[3];
    bool            bRadSet[3];
    bool            m_cancelled;
    bool            m_isCasting;
    bool            m_projectileWait;

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
    SpellTargetMap TargetMap;
    SpellTargetMap ManagedTargets;

    // adds a target to the list, performing DidHit checks
    void _AddTarget(const Unit* target, const uint32 effectid);

    // adds a target to the list, negating DidHit checks
    void _AddTargetForced(const uint64& guid, const uint32 effectid);
    void _AddTargetForced(Object * target, const uint32 effectid) { if(target) _AddTargetForced(target->GetGUID(), effectid); }

    // didhit checker
    uint8 _DidHit(uint32 index, const Unit* target, uint8 &reflectout);

    // gets the pointer of an object (optimized for spell system)
    Object* _LookupObject(const uint64& guid);

    // sets the pointers (unitTarget, itemTarget, etc) for a given guid
    void _SetTargets(uint64 guid);

    friend class DynamicObject;
    void DetermineSkillUp(uint32 skillid,uint32 targetlevel, uint32 multiplicator = 1);
    void DetermineSkillUp(uint32 skillid);

    uint32 m_hitTargetCount;
    uint32 m_missTargetCount;

    // magnet
    uint64 m_magnetTarget;
};

extern uint32 implicitTargetFlags[150];

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
