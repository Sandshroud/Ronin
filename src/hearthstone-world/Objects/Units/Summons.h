/***
 * Demonstrike Core
 */

#pragma once

class Summon;

enum SummonType
{
    SUMMON_TYPE_NONE = 0,
    SUMMON_TYPE_COMPANION,
    SUMMON_TYPE_GUARDIAN,
    SUMMON_TYPE_POSSESSED,
    SUMMON_TYPE_TOTEM,
    SUMMON_TYPE_WILD,
    SUMMON_TYPE_CRITTER
};

class SERVER_DECL SummonHandler
{
public:
    SummonHandler() { };
    ~SummonHandler() { m_summon = NULL; };

    void Initialize(Summon* n_summon);

    virtual void Destruct() { };
    virtual void OnPushToWorld() { };
    virtual void OnRemoveInRangeObject(Object* object) { };
    virtual uint8 GetSummonType() { return SUMMON_TYPE_NONE; };
    virtual void Load(CreatureProto* proto, Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot) { };

protected:
    Summon* m_summon;
};

class SERVER_DECL CompanionSummon : public SummonHandler
{
public:
    CompanionSummon() { };
    ~CompanionSummon() { };

    void Destruct() { };
    void OnPushToWorld() { };
    void OnRemoveInRangeObject(Object* object) { };
    uint8 GetSummonType() { return SUMMON_TYPE_COMPANION; };
    void Load(CreatureProto* proto, Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
};

class SERVER_DECL GuardianSummon : public SummonHandler
{
public:
    GuardianSummon() { };
    ~GuardianSummon() { };

    void Destruct() { };
    void OnPushToWorld() { };
    void OnRemoveInRangeObject(Object* object) { };
    uint8 GetSummonType() { return SUMMON_TYPE_GUARDIAN; };
    void Load(CreatureProto* proto, Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
};

class SERVER_DECL PossessedSummon : public SummonHandler
{
public:
    PossessedSummon() { };
    ~PossessedSummon() { };

    void Destruct() { };
    void OnPushToWorld() { };
    void OnRemoveInRangeObject(Object* object) { };
    uint8 GetSummonType() { return SUMMON_TYPE_POSSESSED; };
    void Load(CreatureProto* proto, Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
};

class SERVER_DECL TotemSummon : public SummonHandler
{
public:
    TotemSummon() { };
    ~TotemSummon() { };

    void SetupSpells();
    void Destruct() { };
    void OnPushToWorld() { SetupSpells(); };
    void OnRemoveInRangeObject(Object* object) { };
    uint8 GetSummonType() { return SUMMON_TYPE_TOTEM; };
    void Load(CreatureProto* proto, Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
};

class SERVER_DECL WildSummon : public SummonHandler
{
public:
    WildSummon() { };
    ~WildSummon() { };

    void Destruct() { };
    void OnPushToWorld() { };
    void OnRemoveInRangeObject(Object* object) { };
    uint8 GetSummonType() { return SUMMON_TYPE_WILD; };
    void Load(CreatureProto* proto, Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
};

class SERVER_DECL Summon : public Creature
{
public:
    Summon(uint64 guid);
    ~Summon();

    void Init();
    void Destruct();
    void OnPushToWorld();
    void CreateAs(SummonHandler* NewHandle);
    void OnRemoveInRangeObject(Object* object);
    void Load(CreatureProto* proto, Unit* m_owner, LocationVector & position, uint32 spellid, int32 summonslot);

    int32 GetSummonSlot() { return summonslot; };
    Object* GetSummonOwner() { return s_Owner; };
    // Returns the internal summon type or none
    uint8 GetSummonType() { return m_Internal ? m_Internal->GetSummonType() : SUMMON_TYPE_NONE; };

private:
    SummonHandler* m_Internal;

    int32 summonslot;   // Summon slot of the creature in the owner's summonhandler, -1 means no slot
    Unit* s_Owner;      // Summoner of the creature
};
