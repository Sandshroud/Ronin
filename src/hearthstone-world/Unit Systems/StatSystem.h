/***
 * Demonstrike Core
 */

#pragma once

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
    uint8 race;
    uint8 class_;
    uint16 level;

    uint32 baseHP;
    uint32 basePower;
    uint32 baseStat[5];
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
    UnitBaseStats *GetUnitBaseStats(uint8 race, uint8 _class, uint16 level);
    EUnitFields GetPowerFieldForClassAndType(uint8 _class, uint8 powerType);

    uint32 CalculateDamage( Unit* pAttacker, Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability);

protected:
    void LoadClassPowers();
    bool LoadUnitStats();

private:
    std::map<uint32, UnitBaseStats*> m_UnitBaseStats;
    std::map<std::pair<uint8, uint8>, EUnitFields> m_unitPowersByClass;
};

#define sStatSystem StatSystem::getSingleton()

SERVER_DECL uint32 CalculateXpToGive(Unit* pVictim, Unit* pAttacker);
SERVER_DECL bool isEven (int num);
