
#pragma once

#pragma pack(PRAGMA_PACK)
struct CreatureData
{
    uint32 entry;
    char * maleName;
    char * femaleName;
    char * subName;
    char * iconName;
    uint32 flags;
    uint32 flags2;
    uint32 type;
    uint32 family;
    uint32 rank;
    uint32 killCredit[2];
    uint32 displayInfo[4];
    float  healthMod;
    float  powerMod;
    uint8  leader;
    uint32 questItems[6];
    uint32 dbcMovementId;
    uint32 expansionId;

    uint32 MinLevel;
    uint32 MaxLevel;
    uint32 Faction;
    uint32 MinHealth;
    uint32 MaxHealth;
    uint8 Powertype;
    uint32 MinPower;
    uint32 MaxPower;
    float  Scale;
    uint32 LootType;
    uint32 NPCFLags;
    uint32 AttackTime;
    uint32 AttackType;
    float  MinDamage;
    float  MaxDamage;
    uint32 RangedAttackTime;
    float  RangedMinDamage;
    float  RangedMaxDamage;
    uint32 Item1;
    uint32 Item2;
    uint32 Item3;
    uint32 RespawnTime;
    uint32 Resistances[7];
    float  CombatReach;
    float  BoundingRadius;
    std::set<uint32> Auras;
    uint8  Civilian;
    uint8  Boss;
    int32  Money;
    uint32 Invisibility_type;
    float  walkSpeed;
    float  runSpeed;
    float  flySpeed;
    uint32 Extra_a9_flags;
    uint32 AuraMechanicImmunity;
    int32  Vehicle_entry;
    uint32 BattleMasterType;
    uint32 SpellClickid;
    uint32 CanMove;

    bool HasValidModelData()
    {
        return true;
    }

    void GenerateModelId(uint8 &gender, uint32 &model)
    {
        std::vector<uint32> modelSet;
        for(uint8 i = 0; i < 4; i++)
            if(uint32 displayId = displayInfo[i])
                modelSet.push_back(displayId);
        // Just in case
        if(modelSet.empty())
            return;

        uint32 r = modelSet.size() > 1 ? RandomUInt(modelSet.size()-1) : 0;
        gender = ((r == 1 || r == 3) ? 1 : 0);
        model = modelSet[r];//.at(r);
    }

    const char *GetFullName()
    {
        if(strlen(femaleName))
            return format("%s|%s", maleName, femaleName).c_str();
        return maleName;
    }

    std::list<AI_Spell*> spells;
};
#pragma pack(PRAGMA_POP)

class SERVER_DECL CreatureDataManager : public Singleton<CreatureDataManager>
{
public:
    class iterator
    {
    private:
        std::map<uint32, CreatureData*>::iterator p;

    public:
        iterator(std::map<uint32, CreatureData*>::iterator ip) { p = ip; };
        iterator& operator++() { ++p; return *this; };

        bool operator != (const iterator &i) { return (p != i.p); };
        bool operator == (const iterator &i) { return (p == i.p); };
        std::map<uint32, CreatureData*>::iterator operator*() { return p; };
    };

    iterator begin() { return iterator(m_creatureData.begin()); }
    iterator end() { return iterator(m_creatureData.end()); }

public:
    ~CreatureDataManager();

    void LoadFromDB();
    void Reload();

    CreatureData *GetCreatureData(uint32 entry)
    {
        if(m_creatureData.find(entry) != m_creatureData.end())
            return m_creatureData.at(entry);
        return NULL;
    }

private:
    std::map<uint32, CreatureData*> m_creatureData;
};


#define sCreatureDataMgr CreatureDataManager::getSingleton()
