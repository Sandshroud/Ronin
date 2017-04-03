
#pragma once

class SpellTargetClass : public SpellEffectClass
{
public:
    SpellTargetClass(Unit* caster, SpellEntry *info, uint8 castNumber, WoWGuid itemGuid);
    ~SpellTargetClass();
    virtual void Destruct();

private:
    // Get Target Type
    uint32 GetTargetType(uint32 implicittarget, uint32 i);

    // adds a target to the list, performing DidHit checks on units
    void _AddTarget(WorldObject* target, const uint32 effIndex);

    uint8 _DidHit(Unit* target, float *resistOut = NULL, uint8 *reflectout = NULL);

public:
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
    bool GenerateTargets(SpellCastTargets *);
    // Fills the target map of the spell effects
    void FillTargetMap(bool fromDelayed);

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

    static std::map<uint8, uint32> m_implicitTargetFlags;

private:
    AuraApplicationResult CheckAuraApplication(Unit *target);
};
