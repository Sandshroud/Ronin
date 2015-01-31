
#pragma once

#pragma pack(PRAGMA_PACK)

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
    struct ItemStat
    {
        uint32 Type;
        int32 Value;
    } Stats[10];
    uint32 minDamage;
    uint32 maxDamage;
    uint32 Armor;
    uint32 DamageType;
    uint32 Delay;
    float  Range;
    struct ItemSpell
    {
        int32 Id;
        int32 Trigger;
        int32 Charges;
        int32 Cooldown;
        int32 Category;
        int32 CategoryCooldown;
    } Spells[5];
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

    std::string lowercase_name;  // used in auctions
    uint32 FoodType;        //pet food type
    int32 ForcedPetId;
    uint32 ItemSetRank;
    uint32 DisenchantReqSkill;

    std::string ConstructItemLink(uint32 random_prop, uint32 random_suffix, uint32 stack);
    bool ValidateItemLink(const char *szLink);
    bool ValidateItemSpell(uint32 SpellID);
};

enum ItemDataFields : uint8
{
    ITEMDATA_FIELD_ITEM_GUID        = 0,
    ITEMDATA_FIELD_CONTAINER_GUID   = 1,
    ITEMDATA_FIELD_CREATOR_GUID     = 2,
    ITEMDATA_FIELD_CONTAINER_SLOT   = 3,
    ITEMDATA_FIELD_ITEMSTACKCOUNT   = 4,
    ITEMDATA_FIELD_ITEMFLAGS        = 5,
    ITEMDATA_FIELD_ITEMRANDOMSEED   = 6,
    ITEMDATA_FIELD_ITEMRANDOMPROP   = 7,
    ITEMDATA_FIELD_ITEM_DURABILITY  = 8,
    ITEMDATA_FIELD_ITEMTEXTID       = 9,
    ITEMDATA_FIELD_ITEM_PLAYEDTIME  = 10,
    ITEMDATA_FIELD_ITEM_CHARGES     = 11,
    ITEMDATA_FIELD_ITEM_GIFT_GUID   = 12,
    ITEMDATA_FIELD_ITEM_GIFT_CREATOR= 13,
    ITEMDATA_FIELD_MAX
};

static const char *fieldNames[ITEMDATA_FIELD_MAX] =
{
    "itemguid",
    "containerguid",
    "creatorguid",
    "containerslot",
    "itemstackcount",
    "itemflags",
    "itemrandomseed",
    "itemrandomprop",
    "itemdurability",
    "itemtextid",
    "itemplayedtime",
    "itemcharges",
    "itemgiftguid",
    "itemgiftcreator"
};

#define MAKE_INVSLOT(bag, item) uint16((uint16(bag)<<8)|uint16(item))
#define INVSLOT_SET_ITEMSLOT(slot, itemslot) (slot&=~0xFF) |= itemslot
#define INVSLOT_BAG(slot) uint8(((slot&0xFF00)>>8))
#define INVSLOT_ITEM(slot) uint8(slot&0x00FF)

struct ItemData
{
    WoWGuid itemGuid;
    WoWGuid itemContainer;
    WoWGuid itemCreator;
    uint16 containerSlot;
    uint32 itemStackCount;
    uint32 itemFlags;
    uint32 itemRandomSeed;
    uint32 itemRandomProperty;
    uint32 itemDurability;
    uint32 itemTextID;
    uint32 itemPlayedTime;
    int32 itemSpellCharges;
    ItemPrototype *proto;

    struct ItemGiftData
    {
        WoWGuid giftItemGuid, giftCreator;
    } *giftData;

    struct ContainerData
    {
        std::map<uint8, WoWGuid> m_items;
    } *containerData;

    struct EnchantData
    {
        uint32 enchantId;
        uint32 enchantCharges;
        time_t expirationTime;

        uint32 CalcTimeLeft()
        {
            if(expirationTime <= UNIXTIME)
                return 0;
            return uint32(1000*(UNIXTIME-expirationTime));
        }
    } *enchantData[10];
};

struct GuildBankItemStorage
{
    std::map<uint8, WoWGuid> bankTabs[6];
};

struct PlayerInventory
{
    std::map<uint16, WoWGuid> m_playerInventory;
};

#pragma pack(PRAGMA_POP)

class SERVER_DECL ItemManager : public Singleton<ItemManager>
{
public:
    ItemManager();
    ~ItemManager();

    void LoadItemData();

    ItemData *GetItemData(WoWGuid itemGuid);
    ItemData *CreateItemData(uint32 entry);

    void DeleteItemData(WoWGuid itemGuid);

private:
    std::map<WoWGuid, ItemData*> m_itemData;

    Mutex itemGuidLock;
    uint32 m_hiItemGuid;

public:
    PlayerInventory *GetPlayerInventory(WoWGuid playerGuid)
    {
        if(m_playerInventories.find(playerGuid) != m_playerInventories.end())
            return m_playerInventories.at(playerGuid);
        PlayerInventory *inventory = new PlayerInventory();
        m_playerInventories.insert(std::make_pair(playerGuid, inventory));
        return inventory;
    }

    GuildBankItemStorage *GetGuildBankStorage(WoWGuid guildGuid)
    {
        if(m_guildBankStorage.find(guildGuid) != m_guildBankStorage.end())
            return m_guildBankStorage.at(guildGuid);
        GuildBankItemStorage *storage = new GuildBankItemStorage();
        m_guildBankStorage.insert(std::make_pair(guildGuid, storage));
        return storage;
    }

private:
    std::map<WoWGuid, PlayerInventory*> m_playerInventories;
    std::map<WoWGuid, GuildBankItemStorage*> m_guildBankStorage;

public:
    void InitializeItemPrototypes();
    void LoadItemOverrides();

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

    ItemPrototype *LookupEntry(uint32 entry);
    iterator itemPrototypeBegin() { return iterator(m_itemPrototypeContainer.begin()); }
    iterator itemPrototypeEnd() { return iterator(m_itemPrototypeContainer.end()); }

    bool HasHotfixOverride(uint32 entry) { return (m_overwritten.find(entry) == m_overwritten.end() ? true : false); };
    std::map<uint32, uint8>::iterator HotfixOverridesBegin() { return m_overwritten.begin(); }
    std::map<uint32, uint8>::iterator HotfixOverridesEnd() { return m_overwritten.end(); }
    size_t HotfixOverridesSize() { return m_overwritten.size(); }

private:
    iterator itemPrototypeFind(uint32 entry) { return iterator(m_itemPrototypeContainer.find(entry)); }
    std::map<uint32, ItemPrototype*> m_itemPrototypeContainer;
    std::map<uint32, uint8> m_overwritten;
};

#define sItemMgr ItemManager::getSingleton()

typedef struct
{
    uint32 setid;
    uint32 itemscount;
}ItemSet;
