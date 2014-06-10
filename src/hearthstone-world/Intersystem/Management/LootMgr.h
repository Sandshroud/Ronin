/***
 * Demonstrike Core
 */

#pragma once

#define OBJECT_LOOT "objectloot"
#define CREATURE_LOOT "creatureloot"
#define CREATURE_LOOT_GATHERING "creatureloot_gathering"
#define FISHING_LOOT "fishingloot"
#define ITEM_LOOT "itemloot"
#define PICKPOCKETING_LOOT "pickpocketingloot"

struct ItemPrototype;
class MapMgr;
class Player;
struct PlayerInfo;

class LootRoll : public EventableObject
{
public:
    LootRoll();
    ~LootRoll();
    void Init(uint32 timer, uint32 groupcount, uint64 guid, uint32 slotid, uint32 itemid, uint32 randomsuffixid, uint32 randompropertyid, MapMgr* mgr);
    void PlayerRolled(PlayerInfo* pInfo, uint8 choice);
    void Finalize();

    int32 event_GetInstanceID();

private:
    Mutex mLootLock;
    std::map<uint32, uint32> m_NeedRolls;
    std::map<uint32, uint32> m_GreedRolls;
    std::map<uint32, uint32> m_DisenchantRolls;
    set<uint32> m_passRolls;
    uint32 _groupcount;
    uint32 _slotid;
    uint32 _itemid;
    uint32 _randomsuffixid;
    uint32 _randompropertyid;
    uint32 _remaining;
    uint64 _guid;
    MapMgr* _mgr;
};

typedef vector<pair<ItemRandomPropertiesEntry*, float> > RandomPropertyVector;
typedef vector<pair<ItemRandomSuffixEntry*, float> > RandomSuffixVector;

typedef struct _LootItem
{
    ItemPrototype * itemproto;
    uint32 displayid;
}_LootItem;

typedef std::set<uint32> LooterSet;

typedef struct __LootItem
{
    _LootItem item;
    uint32 StackSize;
    ItemRandomPropertiesEntry * iRandomProperty;
    ItemRandomSuffixEntry * iRandomSuffix;
    LootRoll* roll;
    bool passed;
    LooterSet has_looted;
    uint32 ffa_loot;
}__LootItem;


typedef struct StoreLootItem
{
    _LootItem item;
    float chance[4];
    uint32 mincount;
    uint32 maxcount;
    uint32 ffa_loot;
}StoreLootItem;


typedef struct StoreLootList
{
    uint32 count;
    StoreLootItem *items;
}StoreLootList;

struct Loot
{
    std::vector<__LootItem> items;
    uint32 gold;
    LooterSet looters;
    bool HasItems(Player* Looter);
    bool HasLoot(Player* Looter);
};

//////////////////////////////////////////////////////////////////////////////////////////


typedef HM_NAMESPACE::hash_map<uint32, StoreLootList > LootStore;

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
    uint8 index;
    uint32 QuestLoot[6];
};

class SERVER_DECL LootMgr : public Singleton < LootMgr >
{
public:
    LootMgr();
    ~LootMgr();

    void AddLoot(Loot * loot, uint32 itemid, uint32 mincount, uint32 maxcount, uint32 ffa_loot);
    void FillCreatureLoot(Loot * loot,uint32 loot_id, uint8 difficulty, uint8 team);
    void FillGOLoot(Loot * loot,uint32 loot_id, uint8 difficulty, uint8 team);
    void FillItemLoot(Loot *loot, uint32 loot_id, uint8 team);
    void FillFishingLoot(Loot * loot,uint32 loot_id);
    void FillGatheringLoot(Loot * loot,uint32 loot_id);
    void FillPickpocketingLoot(Loot *loot, uint32 loot_id);

    bool CanGODrop(uint32 LootId,uint32 itemid);
    bool IsPickpocketable(uint32 creatureId);
    bool IsSkinnable(uint32 creatureId);
    bool IsFishable(uint32 zoneid);

    void LoadLoot();
    void LoadDelayedLoot();
    void LoadLootProp();

    LootStore   CreatureLoot;
    LootStore   FishingLoot;
    LootStore   GatheringLoot;
    LootStore   GOLoot;
    LootStore   ItemLoot;
    LootStore   PickpocketingLoot;

    ItemRandomPropertiesEntry* GetRandomProperties(ItemPrototype * proto);
    ItemRandomSuffixEntry * GetRandomSuffix(ItemPrototype * proto);

    bool is_loading;

    void FillObjectLootMap(map<uint32, vector<uint32> > *dest);

    ObjectQuestLoot* GetCreatureQuestLoot(uint32 entry) { if(_creaturequestloot.find(entry) == _creaturequestloot.end()) return NULL; return _creaturequestloot[entry]; };
    ObjectQuestLoot* GetGameObjectQuestLoot(uint32 entry) { if(_gameobjectquestloot.find(entry) == _gameobjectquestloot.end()) return NULL; return _gameobjectquestloot[entry]; };

private:
    void LoadLootTables(const char * szTableName, LootStore * LootTable, bool MultiDifficulty);
    void PushLoot(StoreLootList *list,Loot * loot, uint8 difficulty, uint8 team, bool disenchant);

    map<uint32, ObjectQuestLoot*> _creaturequestloot;
    map<uint32, ObjectQuestLoot*> _gameobjectquestloot;
    map<uint32, RandomPropertyVector> _randomprops;
    map<uint32, RandomSuffixVector> _randomsuffix;
};

#define lootmgr LootMgr::getSingleton()
