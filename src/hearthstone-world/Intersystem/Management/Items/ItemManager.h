
#pragma once

#pragma pack(PRAGMA_PACK)

struct ItemStat
{
    uint32 Type;
    int32 Value;
};

struct ItemDamage
{
    float Min;
    float Max;
    uint32 Type;
};

struct ItemSpell
{
    int32 Id;
    int32 Trigger;
    int32 Charges;
    int32 Cooldown;
    int32 Category;
    int32 CategoryCooldown;
};

struct SocketInfo
{
    uint32 SocketColor;
    uint32 Unk;
};

struct ItemPrototype
{
    uint32 ItemId;
    uint32 Class;
    uint32 SubClass;
    int32  subClassSound;
    char * Name1;
    uint32 DisplayInfoID;
    uint32 Quality;
    uint32 Flags;
    uint32 FlagsExtra;
    uint32 BuyPrice;
    uint32 SellPrice;
    uint32 InventoryType;
    uint32 AllowableClass;
    uint32 AllowableRace;
    uint32 ItemLevel;
    uint32 RequiredLevel;
    uint32 RequiredSkill;
    uint32 RequiredSkillRank;
    uint32 RequiredSpell;
    uint32 RequiredPlayerRank1;
    uint32 RequiredPlayerRank2;
    uint32 RequiredFaction;
    uint32 RequiredFactionStanding;
    uint32 MaxCount;
    uint32 Unique;
    uint32 ContainerSlots;
    ItemStat Stats[10];
    uint32 minDamage;
    uint32 maxDamage;
    uint32 Armor;
    uint32 DamageType;
    uint32 Delay;
    float  Range;
    ItemSpell Spells[5];
    uint32 Bonding;
    char * Description;
    uint32 PageId;
    uint32 PageLanguage;
    uint32 PageMaterial;
    uint32 QuestId;
    uint32 LockId;
    uint32 LockMaterial;
    uint32 SheathID;
    uint32 RandomPropId;
    uint32 RandomSuffixId;
    uint32 ItemSet;
    uint32 MaxDurability;
    uint32 ZoneNameID;
    uint32 MapID;
    uint32 BagFamily;
    uint32 ItemSocket[3];
    uint32 ItemContent[3];
    uint32 SocketBonus;
    uint32 GemProperties;
    float ArmorDamageModifier;
    uint32 Duration;
    uint32 ItemLimitCategory;
    uint32 HolidayId;
    float StatScalingFactor;

    string lowercase_name;  // used in auctions
    uint32 FoodType;        //pet food type
    int32 ForcedPetId;
    uint32 ItemSetRank;
    uint32 DisenchantReqSkill;

    string ConstructItemLink(uint32 random_prop, uint32 random_suffix, uint32 stack);
    bool ValidateItemLink(const char *szLink);
    bool ValidateItemSpell(uint32 SpellID);
};

#pragma pack(PRAGMA_POP)

class SERVER_DECL ItemPrototypeSystem : public Singleton<ItemPrototypeSystem>
{
public:
    class iterator
    {
    private:
        std::map<uint32, ItemPrototype*>::iterator p;

    public:
        iterator(std::map<uint32, ItemPrototype*>::iterator ip) { p = ip; };
        iterator& operator++() { ++p; return *this; };

        bool operator != (const iterator &i) { return (p != i.p); };
        bool operator == (const iterator &i) { return (p == i.p); };
        std::map<uint32, ItemPrototype*>::iterator operator*() { return p; };
    };

    iterator begin() { return iterator(ItemPrototypeContainer.begin()); }
    iterator end() { return iterator(ItemPrototypeContainer.end()); }

    std::map<uint32, uint8>::iterator HotfixBegin() { return Overridden.begin(); }
    std::map<uint32, uint8>::iterator HotfixEnd() { return Overridden.end(); }

public: // Item System
    void Init();
    void InitializeItemSystem();
    void LoadItemOverrides();

    ItemPrototype* LookupEntry(uint32 entry);
    bool HasHotfix(uint32 entry) { return (Overridden.find(entry) == Overridden.end() ? true : false); };

private:
    iterator find(uint32 entry) { return iterator(ItemPrototypeContainer.find(entry)); }
    std::map<uint32, ItemPrototype*> ItemPrototypeContainer;
    std::map<uint32, uint8> Overridden;
};

#define ItemPrototypeStorage ItemPrototypeSystem::getSingleton()

typedef struct
{
    uint32 setid;
    uint32 itemscount;
}ItemSet;
