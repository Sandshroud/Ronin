
#pragma once

class SpellTargetClass;
class MapTargetCallback { public: virtual void operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target) = 0; };
class FillAreaTargetsCallback : public MapTargetCallback { virtual void operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target); };
class FillAreaFriendliesCallback : public MapTargetCallback { virtual void operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target); };
class FillInRangeTargetsCallback : public MapTargetCallback { virtual void operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target); };
class FillInRangeConeTargetsCallback : public MapTargetCallback { virtual void operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target); };
class FillSpecificGameObjectsCallback : public MapTargetCallback { virtual void operator()(SpellTargetClass *spell, uint32 i, uint32 targetType, WorldObject *target); };

class SpellTargetClass : public SpellEffectClass
{
public:
    SpellTargetClass(Unit* caster, SpellEntry *info, uint8 castNumber, WoWGuid itemGuid);
    ~SpellTargetClass();
    virtual void Destruct();

protected:
    // Get Target Type
    uint32 GetTargetType(uint32 implicittarget, uint32 i);

    // adds a target to the list, performing DidHit checks on units
    void _AddTarget(WorldObject* target, const uint32 effIndex);
    uint8 _DidHit(Unit* target, float *resistOut = NULL, uint8 *reflectout = NULL);

    // If we check friendly or combat support for friendly target checks
    bool requiresCombatSupport(uint32 effIndex);

    // Some spell effects require the caster to trigger an effect
    bool EffectRequiresAnyTarget(uint32 i);

    // If we have max targets we can check if we're full on targets
    bool IsTargetMapFull(uint32 effIndex, WoWGuid guidCheck = 0);

public:
    // Fills specified targets at the area of effect
    void FillSpecifiedTargetsInArea(float srcx,float srcy,float srcz, uint32 effIndex, uint32 typeMask);
    // Fills specified targets at the area of effect. We suppose we already inited this spell and know the details
    void FillSpecifiedTargetsInArea(uint32 i, float srcx,float srcy,float srcz, float r, uint32 typeMask);
    // Fills the targets at the area of effect
    void FillAllTargetsInArea(uint32 i, float srcx,float srcy,float srcz, float r, bool includegameobjects = false);
    // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
    void FillAllTargetsInArea(float srcx,float srcy,float srcz, uint32 effIndex);
    // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
    void FillAllTargetsInArea(LocationVector & location, uint32 effIndex);
    // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
    void FillAllFriendlyInArea(uint32 i, float srcx,float srcy,float srcz, float r);
    // Fills the gameobject targets at the area of effect
    void FillAllGameObjectTargetsInArea(uint32 i, float srcx,float srcy,float srcz, float r);
    //get single Enemy as target
    WoWGuid GetSinglePossibleEnemy(uint32 i, float prange=0);
    //get single Enemy as target
    WoWGuid GetSinglePossibleFriend(uint32 i, float prange=0);
    //generate possible target list for a spell. Use as last resort since it is not acurate
    bool GenerateTargets(SpellCastTargets *);
    // Fills the target map of the spell effects
    void FillTargetMap(bool fromDelayed);

    // Spell Targets
    void HandleTargetNoObject(uint32 i, float r);
    bool AddTarget(uint32 i, uint32 TargetType, WorldObject* obj);
    void AddAOETargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddPartyTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddRaidTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets, bool partylimit = false);
    void AddChainTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddConeTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
    void AddScriptedOrSpellFocusTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);

    WoWGuid static FindLowestHealthRaidMember(Player* Target, uint32 dist);

    static std::map<uint8, uint32> m_implicitTargetFlags;

private:
    std::vector<WorldObject*> *m_temporaryStorage;

protected:
    AuraApplicationResult CheckAuraApplication(Unit *target);

    friend class MapTargetCallback;
    friend class FillAreaTargetsCallback;
    friend class FillAreaFriendliesCallback;
    friend class FillInRangeTargetsCallback;
    friend class FillInRangeConeTargetsCallback;
    friend class FillSpecificGameObjectsCallback;
};
