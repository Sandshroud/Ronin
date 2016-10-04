/***
 * Demonstrike Core
 */

#pragma once

#define MAXIMUM_CHAR_PER_ENUM 10
#define MAXIMUM_CEXPANSION_LEVEL 85 // Current expansion's max level
#define MAXIMUM_ATTAINABLE_LEVEL 100 // Crow: Lets use 100, since its the highest the DBCs will support

//Crow: SQRT is a resource heavy function, so we calculate it at startup
static float fMaxLevelSqrt = sqrt<uint32>(MAXIMUM_ATTAINABLE_LEVEL);

enum Stats : uint8
{
    STAT_STRENGTH=0,
    STAT_AGILITY,
    STAT_STAMINA,
    STAT_INTELLECT,
    STAT_SPIRIT,
    MAX_STAT
};

enum Resistances : uint8
{
    RESISTANCE_ARMOR=0,
    RESISTANCE_HOLY,
    RESISTANCE_FIRE,
    RESISTANCE_NATURE,
    RESISTANCE_FROST,
    RESISTANCE_SHADOW,
    RESISTANCE_ARCANE,
    MAX_RESISTANCE
};

struct UnitBaseStats
{
    uint16 level;

    uint32 baseHP;
    uint32 basePower;
    uint32 baseStat[5];
};

static uint32 xpPerLevel[MAXIMUM_ATTAINABLE_LEVEL] =
{
    0, 400, 900, 1400, 1200, 2800, 3600, 4500, 5400, 6500,
    7600, 8700, 9800, 11000, 12300, 13600, 15000, 16500, 17800, 19300,
    20800, 22400, 24000, 25500, 27200, 28900, 30500, 32200, 33900, 36300,
    38800, 41600, 44600, 48000, 51400, 55000, 58700, 62400, 66200, 70200,
    74300, 78500, 82800, 87100, 91600, 96300, 101000, 105800, 110700, 115700,
    120900, 126100, 131500, 137000, 142500, 148200, 154000, 159900, 165800, 172000,
    290000, 317000, 349000, 386000, 428000, 475000, 527000, 585000, 648000, 717000,
    1523800, 1539600, 1555700, 1571800, 1587900, 1604200, 1620700, 1637400, 1653900, 1670800,
    1686300, 2121500, 4004000, 5203400, 9165100, 13000000, 15080000, 18980000, 22880000, 27560000,
    35430000, 39880000, 41150000, 46800000, 52250000, 58150000, 65500000, 71250000, 79800000, 88050000
};

class Unit;
class Player;
class Creature;

class SERVER_DECL StatSystem : public Singleton<StatSystem>
{
public:
    StatSystem();
    ~StatSystem();

    bool Load();
    uint32 GetXPToNextLevel(uint32 level) { return xpPerLevel[std::min<uint32>(level, MAXIMUM_ATTAINABLE_LEVEL)]; }

    UnitBaseStats *GetMaxUnitBaseStats(uint8 race, uint8 _class);
    UnitBaseStats *GetUnitBaseStats(uint8 race, uint8 _class, uint16 level);
    EUnitFields GetPowerFieldForClassAndType(uint8 _class, uint8 powerType);

    uint32 CalculateDamage( Unit* pAttacker, Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability);
    std::vector<uint8> *GetUnitPowersForClass(uint8 _class) { if(m_unitPowersForClass.find(_class) == m_unitPowersForClass.end()) return NULL; return &m_unitPowersForClass.at(_class); }

protected:
    void LoadClassPowers();
    bool LoadUnitStats();

private:
    std::map<uint32, UnitBaseStats*> m_UnitBaseStats;
    std::map<uint8, std::vector<uint8>> m_unitPowersForClass;
    std::map<std::pair<uint8, uint8>, EUnitFields> m_unitPowersByClass;
    std::map<std::pair<uint8, uint8>, UnitBaseStats*> m_maxBaseStats;
};

#define sStatSystem StatSystem::getSingleton()

SERVER_DECL uint32 CalculateXpToGive(Unit* pVictim, Unit* pAttacker);
SERVER_DECL bool isEven (int num);
