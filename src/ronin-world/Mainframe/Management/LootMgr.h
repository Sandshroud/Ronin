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

#define LOOT_CREATURE       "loot_creature"
#define LOOT_ZONE           "loot_zone"
#define LOOT_FISHING        "loot_fishing"
#define LOOT_GAMEOBJECT     "loot_gameobject"
#define LOOT_GATHERING      "loot_gathering"
#define LOOT_ITEMS          "loot_items"
#define LOOT_PICKPOCKETING  "loot_pickpocketing"

struct ItemPrototype;
class MapInstance;
class Player;
struct PlayerInfo;

class LootRoll
{
public:
    LootRoll();
    ~LootRoll();
    void Init(uint32 timer, uint32 groupcount, uint64 guid, uint32 slotid, uint32 itemid, uint32 randomsuffixid, uint32 randompropertyid, MapInstance* instance);
    void PlayerRolled(PlayerInfo* pInfo, uint8 choice);
    void Finalize();

    int32 event_GetInstanceID();

private:
    Mutex mLootLock;
    std::map<WoWGuid, uint32> m_NeedRolls, m_GreedRolls, m_DisenchantRolls;
    std::set<WoWGuid> m_passRolls;
    uint32 _groupcount;
    uint32 _slotid;
    uint32 _itemid;
    uint32 _randomProp;
    uint32 _randomSeed;
    uint32 _remaining;
    uint64 _guid;
    MapInstance* _instance;
};

typedef std::vector<std::pair<ItemRandomPropertiesEntry*, float> > RandomPropertyVector;
typedef std::vector<std::pair<ItemRandomSuffixEntry*, float> > RandomSuffixVector;

typedef std::set<WoWGuid> LooterSet;

struct __LootItem
{
    LootRoll *roll;
    ItemPrototype *proto;
    uint32 StackSize;
    uint32 randSeed, randProp;

    bool all_passed;
    LooterSet has_looted;
};

struct ObjectLoot
{
    uint64 gold;
    LooterSet looters;
    bool HasItems(Player* Looter);
    bool HasLoot(Player* Looter);

    Mutex _lock;
    std::vector<__LootItem> items;
};

struct StoreLootItem
{
    float *chance;
    bool multiChance;
    ItemPrototype *proto;
    uint32 minCount, maxCount;
};

typedef std::set<StoreLootItem*> StoreLootList;
typedef std::map<uint32, StoreLootList > LootStore;

//////////////////////////////////////////////////////////////////////////////////////////

enum PARTY_LOOT
{
    PARTY_LOOT_FFA      = 0,
    PARTY_LOOT_RR       = 1,
    PARTY_LOOT_MASTER   = 2,
    PARTY_LOOT_GROUP    = 3,
    PARTY_LOOT_NBG      = 4,
};

// Thanks to mangos for mask values.
enum ROLL_ENABLE_MASK
{
    ROLLMASK_PASS       = 0x01,
    ROLLMASK_NEED       = 0x02,
    ROLLMASK_GREED      = 0x04,
    ROLLMASK_DISENCHANT = 0x08
};

enum PARTY_ROLL
{
    NEED        = 1,
    GREED       = 2,
    DISENCHANT  = 3,
    PASS        = 4
};

struct ObjectQuestLoot
{
    std::set<uint32> questLoot;
};

class SERVER_DECL LootMgr : public Singleton < LootMgr >
{
public:
    LootMgr();
    ~LootMgr();

    void AddLoot(ObjectLoot * loot, uint32 itemid, uint32 mincount, uint32 maxcount, uint32 ffa_loot);
    void FillCreatureLoot(ObjectLoot * loot,uint32 loot_id, uint8 difficulty, uint8 team);
    void FillZoneLoot(ObjectLoot * loot, MapInstance *instance, uint32 zoneId, uint8 rankModifier, uint8 difficulty, uint8 team);
    void FillGOLoot(ObjectLoot * loot,uint32 loot_id, uint8 difficulty, uint8 team);
    void FillItemLoot(ObjectLoot *loot, uint32 loot_id, uint8 team);
    void FillFishingLoot(ObjectLoot * loot,uint32 loot_id);
    void FillGatheringLoot(ObjectLoot * loot,uint32 loot_id);
    void FillPickpocketingLoot(ObjectLoot *loot, uint32 loot_id);

    bool CanGODrop(uint32 LootId,uint32 itemid);
    bool IsPickpocketable(uint32 creatureId);
    bool IsSkinnable(uint32 creatureId);
    bool IsFishable(uint32 zoneid);

    void LoadLoot();
    void LoadDelayedLoot();
    void LoadLootProp();

    LootStore CreatureLoot;
    LootStore ZoneLoot;

    LootStore FishingLoot;
    LootStore GatheringLoot;
    LootStore GOLoot;
    LootStore ItemLoot;
    LootStore PickpocketingLoot;

    void GenerateRandomProperties(__LootItem *item, float luck = 1.f);

    bool is_loading;

    void FillObjectLootMap(std::map<uint32, std::vector<uint32> > *dest);

    std::vector<uint32> *GetCreatureQuestLoot(uint32 entry) { if(_creaturequestloot.find(entry) == _creaturequestloot.end()) return NULL; return &_creaturequestloot[entry]; };
    std::vector<uint32> *GetGameObjectQuestLoot(uint32 entry) { if(_gameobjectquestloot.find(entry) == _gameobjectquestloot.end()) return NULL; return &_gameobjectquestloot[entry]; };

private:
    void LoadLootTables(const char * szTableName, LootStore * LootTable, bool MultiDifficulty);
    void PushLoot(StoreLootList *list, ObjectLoot *loot, uint8 difficulty, uint8 team, bool disenchant);

    std::map<uint32, std::vector<uint32>> _creaturequestloot;
    std::map<uint32, std::vector<uint32>> _gameobjectquestloot;
    std::map<uint32, RandomPropertyVector> _randomprops;
    std::map<uint32, RandomSuffixVector> _randomsuffix;
};

#define lootmgr LootMgr::getSingleton()
