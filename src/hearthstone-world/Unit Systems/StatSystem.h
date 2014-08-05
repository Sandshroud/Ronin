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

    bool LoadUnitStats();
    UnitBaseStats *GetUnitBaseStats(uint8 race, uint8 _class, uint16 level);

private:
    std::map<uint32, UnitBaseStats*> m_UnitBaseStats;
};

#define sStatSystem StatSystem::getSingleton()

SERVER_DECL uint32 getConColor(uint16 AttackerLvl, uint16 VictimLvl);
SERVER_DECL uint32 CalculateXpToGive(Unit* pVictim, Unit* pAttacker);
SERVER_DECL uint32 CalculateStat(uint16 level, float inc);
SERVER_DECL uint32 CalculateStat(uint16 level, double a3, double a2, double a1, double a0);
SERVER_DECL uint32 CalculateDamage( Unit* pAttacker, Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability);
SERVER_DECL uint32 CalcStatForLevel(uint16 level, uint8 playerrace, uint8 playerclass, uint8 Stat);
SERVER_DECL bool isEven (int num);
