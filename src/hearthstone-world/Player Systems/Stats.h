/***
 * Demonstrike Core
 */

#pragma once

enum Stats
{
    STAT_STRENGTH,
    STAT_AGILITY,
    STAT_STAMINA,
    STAT_INTELLECT,
    STAT_SPIRIT,
};

enum Resistances
{
    RESISTANCE_ARMOR,
    RESISTANCE_HOLY,
    RESISTANCE_FIRE,
    RESISTANCE_NATURE,
    RESISTANCE_FROST,
    RESISTANCE_SHADOW,
    RESISTANCE_ARCANE
};

SERVER_DECL uint32 getConColor(uint16 AttackerLvl, uint16 VictimLvl);
SERVER_DECL uint32 CalculateXpToGive(Unit* pVictim, Unit* pAttacker);
SERVER_DECL uint32 CalculateStat(uint16 level, float inc);
SERVER_DECL uint32 CalculateStat(uint16 level, double a3, double a2, double a1, double a0);
SERVER_DECL uint32 CalculateDamage( Unit* pAttacker, Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability);
SERVER_DECL uint32 CalcStatForLevel(uint16 level, uint8 playerrace, uint8 playerclass, uint8 Stat);
SERVER_DECL bool isEven (int num);
