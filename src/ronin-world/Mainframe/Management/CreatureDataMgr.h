
#pragma once

enum CreatureDataExtraFlags : uint16
{
    CREATURE_DATA_EX_FLAGS_NONE         = 0x0000,
    CREATURE_DATA_EX_FLAG_FODDER_SPAWN  = 0x2000,
    CREATURE_DATA_EX_FLAG_TRAINING_DUMMY= 0x4000,
    CREATURE_DATA_EX_FLAG_WORLD_TRIGGER = 0x8000,
};

enum TrainerServiceType
{
    TRAINER_SERVICE_AVAILABLE       =0x0,
    TRAINER_SERVICE_UNAVAILABLE     =0x1,
    TRAINER_SERVICE_USED            =0x2,
    TRAINER_SERVICE_NOT_SHOWN       =0x3,
    TRAINER_SERVICE_NEVER           =0x4,
    TRAINER_SERVICE_NO_PET          =0x5
};
#define NUM_TRAINER_SERVICE_TYPES    0x6

enum TrainerCategory : uint8
{
    TRAINER_CATEGORY_GENERAL        = 0,
    TRAINER_CATEGORY_TALENTS        = 1,
    TRAINER_CATEGORY_TRADESKILLS    = 2,
    TRAINER_CATEGORY_SECONDARY      = 3
};
#define NUM_TRAINER_TYPES             4

enum TrainerSpellState
{
    TRAINER_SPELL_KNOWN = 0,
    TRAINER_SPELL_AVAILABLE = 1,
    TRAINER_SPELL_UNAVAILABLE = 2
};

#pragma pack(PRAGMA_PACK)

struct CreatureData
{
    uint32 entry;
    std::string maleName;
    std::string femaleName;
    std::string subName;
    std::string iconName;
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

    uint8  Class;
    uint32 minLevel;
    uint32 maxLevel;
    uint32 faction;
    uint8  powerType;
    float  scale;
    uint32 lootType;
    uint32 NPCFLags;
    uint32 attackType;
    float  damageMod;
    float  damageRangeMod;
    uint32 meleeAttackTime;
    uint32 rangedAttackTime;
    uint32 inventoryItem[3];
    uint32 respawnTime;
    uint32 resistances[7];
    float  combatReach;
    float  boundingRadius;
    int32  money;
    uint32 invisType;
    float  walkSpeed;
    float  runSpeed;
    float  flySpeed;
    uint32 auraMechanicImmunity;
    uint32 vehicleEntry;
    uint32 spellClickid;
    uint32 movementMask;
    uint32 battleMasterType;

    // Set on server load
    uint32 extraFlags;
    std::set<uint32> Auras;
    std::vector<uint32> combatSpells, supportSpells;

    bool HasValidModelData()
    {
        if(displayInfo[0] == 0 && displayInfo[1] == 0
            && displayInfo[2] == 0 && displayInfo[3] == 0)
            return false;
        return true;
    }

    void VerifyModelInfo(uint32 &model, uint32 &gender)
    {
        std::vector<uint32> modelSet;
        for(uint8 i = 0; i < 4; i++)
        {
            if(uint32 displayId = displayInfo[i])
            {
                if(displayId == model)
                {
                    gender = ((i == 1 || i == 3) ? 1 : 0);
                    return;
                }
                modelSet.push_back(displayId);
            }
        }

        // Just in case
        if(modelSet.empty())
            return;

        uint32 r = modelSet.size() > 1 ? RandomUInt(modelSet.size()-1) : 0;
        gender = ((r == 1 || r == 3) ? 1 : 0);
        model = modelSet[r];//.at(r);
    }

    const char *GetFullName()
    {
        if(femaleName.length())
            return format("%s|%s", maleName.c_str(), femaleName.c_str()).c_str();
        return maleName.c_str();
    }
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
    void LoadCreatureSpells();
    void Reload();

    CreatureData *GetCreatureData(uint32 entry)
    {
        if(m_creatureData.find(entry) != m_creatureData.end())
            return m_creatureData.at(entry);
        return NULL;
    }

    bool IsSpiritHealer(CreatureData *data);

    void CalcBasePower(CreatureData *data, uint8 powerType, uint32 &basePower);
    void CalculateMinMaxDamage(CreatureData *data, float &minDamage, float &maxDamage, uint32 level, float apBonus);

private:
    std::map<uint32, CreatureData*> m_creatureData;
};


#define sCreatureDataMgr CreatureDataManager::getSingleton()
