
#pragma once

#pragma pack(PRAGMA_PACK)
struct CreatureData
{
    uint32 Entry;
    char * Name;
    char * SubName;
    char * InfoStr;
    uint32 Flags;
    uint32 Type;
    uint32 TypeFlags;
    uint32 Family;
    uint32 Rank;
    uint32 KillCredit[2];
    uint32 DisplayInfo[4];
    float  HealthMod;
    float  PowerMod;
    uint8  Civilian;
    uint8  Leader;

    uint32 MinLevel;
    uint32 MaxLevel;
    uint32 Faction;
    uint32 MinHealth;
    uint32 MaxHealth;
    uint8 Powertype;
    uint32 MinPower;
    uint32 MaxPower;
    float  Scale;
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
    uint32 Boss;
    int32  Money;
    uint32 Invisibility_type;
    float  Walk_speed;
    float  Run_speed;
    float  Fly_speed;
    uint32 Extra_a9_flags;
    uint32 AuraMechanicImmunity;
    int32  Vehicle_entry;
    uint32 BattleMasterType;
    uint32 SpellClickid;
    uint32 CanMove;

    bool GenerateModelId(uint8 &gender, uint32 &model)
    {
        std::vector<uint32> modelSet;
        for(uint8 i = 0; i < 4; i++)
            if(DisplayInfo[i])
                modelSet.push_back(DisplayInfo[i]);
        if(modelSet.empty())
            return false;
        uint32 r = modelSet.size() > 1 ? RandomUInt(modelSet.size()-1) : 0;
        gender = ((r == 1 || r == 3) ? 1 : 0);
        model = modelSet[r];//.at(r);
        return true;
    }

    std::string lowercase_name;
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
