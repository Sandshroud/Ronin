/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#define MAXIMUM_CHAR_PER_ENUM 10
#define MAXIMUM_CEXPANSION_LEVEL 85 // Current expansion's max level
#define MAXIMUM_ATTAINABLE_LEVEL 100 // Crow: Lets use 100, since its the highest the DBCs will support

//Crow: SQRT is a resource heavy function, so we calculate it at startup
static float fMaxLevelSqrt = sqrt<uint32>(MAXIMUM_ATTAINABLE_LEVEL);

enum Races : uint8
{
    RACE_HUMAN = 1,
    RACE_ORC = 2,
    RACE_DWARF = 3,
    RACE_NIGHTELF = 4,
    RACE_UNDEAD = 5,
    RACE_TAUREN = 6,
    RACE_GNOME = 7,
    RACE_TROLL = 8,
    RACE_GOBLIN = 9,
    RACE_BLOODELF = 10,
    RACE_DRAENEI = 11,
    RACE_FELORC = 12,
    RACE_NAGA = 13,
    RACE_BROKEN = 14,
    RACE_SKELETON = 15,
    RACE_VRYKUL = 16,
    RACE_TUSKARR = 17,
    RACE_FORESTTROLL = 18,
    RACE_TAUNKA = 19,
    RACE_NORTHREND_SKELETON = 20,
    RACE_ICE_TROLL = 21,
    RACE_WORGEN = 22,
    RACE_SUBHUMAN = 23,
    RACE_MAX = 24,
    RACE_COUNT = 12
};

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

    gtFloat *baseHP, *baseMP;
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

static uint32 classMasterySpells[12-1] = { 86479, 86474, 86472, 86476, 86475, 86471, 86477, 86473, 86478, 0, 86470 };

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

    uint32 GetXPackModifierForLevel(uint32 level, uint32 mod);
    uint32 CalculateDamage( Unit* pAttacker, Unit* pVictim, uint32 weapon_damage_type, SpellEntry* ability);
    std::vector<uint8> *GetUnitPowersForClass(uint8 _class) { if(m_unitPowersForClass.find(_class) == m_unitPowersForClass.end()) return NULL; return &m_unitPowersForClass.at(_class); }

    // Race substitute
    uint8 GetViableOptionForRace(uint8 race);

    // Talent data acquisition
    typedef std::multimap<std::pair<uint8, uint8>, uint32> TalentSpellStorage;
    typedef std::pair<TalentSpellStorage::iterator, TalentSpellStorage::iterator> TalentSpellPair;

    // Primary spells for talent tabs
    TalentSpellPair GetTalentPrimarySpells(uint8 classId, uint8 tabId) { return m_talentPrimarySpells.equal_range(std::make_pair(classId, tabId)); }
    // Mastery spells for talent tabs
    TalentSpellPair GetTalentMasterySpells(uint8 classId, uint8 tabId) { return m_talentMasterySpells.equal_range(std::make_pair(classId, tabId)); }

protected:
    void LoadClassBaseStats();
    void LoadClassPowers();
    void LoadClassTalentData();
    bool LoadUnitStats();

private:
    std::map<uint32, UnitBaseStats*> m_UnitBaseStats;
    std::map<uint8, std::vector<uint8>> m_unitPowersForClass;
    std::map<std::pair<uint8, uint8>, EUnitFields> m_unitPowersByClass;
    std::map<std::pair<uint8, uint8>, UnitBaseStats*> m_maxBaseStats;

    // Talent handling
    TalentSpellStorage m_talentMasterySpells, m_talentPrimarySpells;
    std::map<std::pair<uint8, uint8>, uint32> m_talentRoleMasks;

    // Base HP/MP storage
    std::map<std::pair<uint8, uint32>, gtFloat*> m_baseHPByClassLevel, m_baseMPByClassLevel;
};

#define sStatSystem StatSystem::getSingleton()

SERVER_DECL uint32 CalculateXpToGive(Unit* pVictim, Unit* pAttacker, uint32 mapModifier);
SERVER_DECL bool isEven (int num);
