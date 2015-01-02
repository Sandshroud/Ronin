/***
 * Demonstrike Core
 */

#pragma once

const static ItemProf prof[22];
const static uint16 arm_skills[7];
const static uint16 weap_skills[21];
const static float pricemod[9];
const static float SuffixMods[NUM_INVENTORY_TYPES];

// -1 from client enchantment slot number
enum EnchantmentSlot
{
    PERM_ENCHANTMENT_SLOT           = 0,
    TEMP_ENCHANTMENT_SLOT           = 1,
    SOCK_ENCHANTMENT_SLOT1          = 2,
    SOCK_ENCHANTMENT_SLOT2          = 3,
    SOCK_ENCHANTMENT_SLOT3          = 4,
    BONUS_ENCHANTMENT_SLOT          = 5,
    PRISMATIC_ENCHANTMENT_SLOT      = 6,
    // No clue what 7 is yet..
    REFORGE_ENCHANTMENT_SLOT        = 8,
    TRANSMOG_ENCHANTMENT_SLOT       = 9,
    MAX_INSPECTED_ENCHANTMENT_SLOT  = 10,

    PROP_ENCHANTMENT_SLOT_0         = 10, // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_1         = 11, // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_2         = 12, // used with RandomSuffix and RandomProperty
    PROP_ENCHANTMENT_SLOT_3         = 13, // used with RandomProperty
    PROP_ENCHANTMENT_SLOT_4         = 14, // used with RandomProperty
    MAX_ENCHANTMENT_SLOT            = 15
};

enum ItemFlags : uint32
{
    ITEM_FLAG_SOULBOUND     = 0x000001,
    ITEM_FLAG_UNLOCKED      = 0x000004,
    ITEM_FLAG_WRAPPED       = 0x000008,
    ITEM_FLAG_BOP_TRADABLE  = 0x000100,
    ITEM_FLAG_READABLE      = 0x000200,
    ITEM_FLAG_REFUNDABLE    = 0x010000,
};

struct EnchantmentInstance
{
    SpellItemEnchantEntry *Enchantment;
    bool BonusApplied;
    uint32 Slot;
    time_t ApplyTime;
    uint32 Duration;
    bool RemoveAtLogout;
    uint32 RandomSuffix;
    bool Dummy;
};

typedef std::map< uint32, EnchantmentInstance > EnchantmentMap;

#define RANDOM_SUFFIX_MAGIC_CALCULATION( __suffix, __scale ) int32( float( ( float( ( __suffix ) ) * float( ( __scale ) ) ) ) / 13340.0f );

class SERVER_DECL Item : public Object
{
public:
    Item(ItemData *data, uint32 fieldCount = ITEM_END);
    ~Item();

    virtual void Initialize(Player *owner);
    virtual void Destruct();

    void Create( uint32 itemid, Player* owner );

    RONIN_INLINE ItemPrototype* GetProto() const { return m_itemProto; }
    RONIN_INLINE Player* GetOwner() const { return m_owner; }
    void SetOwner( Player* owner );

    bool IsEligibleForRefund();

    bool isSoulBound() { return GetItemFlags() & ITEM_FLAG_SOULBOUND; };
    bool isAccountBound() { return (m_itemProto->Flags & DBC_ITEMFLAG_BIND_TO_ACCOUNT); }
    uint8 GetBoundStatus()
    {
        if(isAccountBound())
            return 0x08;
        if(isSoulBound())
            return 0x04;
        return 0x00;
    }

    RONIN_INLINE void Bind(uint32 bondtype)
    {
        if(bondtype == ITEM_BIND_NONE || GetBoundStatus())
            return;
        if(GetProto()->Bonding != bondtype)
            return;
        AddItemFlag(ITEM_FLAG_SOULBOUND);
    }

    void SetTextID(uint32 newtextId);
    uint32 GetTextID() { return m_textid; };

    void SetWrappedItemGuid(WoWGuid guid);
    WoWGuid GetWrappedItemGuid() { return m_wrappedItemGuid; }
    bool isWrapped() { return !m_wrappedItemGuid.empty(); }

protected:
    ItemPrototype* m_itemProto;
    EnchantmentMap m_enchantments;
    WoWGuid m_wrappedItemGuid;
    uint32 m_textid;
    bool m_locked;

    Player *m_owner; // let's not bother the manager with unneeded requests

public:
    void SetContainerData(WoWGuid newContainer, uint16 slot);
    WoWGuid GetContainerGuid() { return GetUInt64Value(ITEM_FIELD_CONTAINED); }
    WoWGuid GetCreatorGuid() { return GetUInt64Value(ITEM_FIELD_CREATOR); }

    uint16 GetContainerSlot() { return _itemData->containerSlot; }
    uint8 GetBagSlot() { return _itemData->containerSlot>>8; }
    uint8 GetSlot() { return (_itemData->containerSlot&0xFF); }

    void SetStackSize(uint32 newStackSize);
    void ModStackSize(int32 stackSizeMod);
    uint32 GetStackSize() { return GetUInt32Value(ITEM_FIELD_STACK_COUNT); }

    void AddItemFlag(uint32 itemFlag);
    void RemoveItemFlag(uint32 itemFlag);
    uint32 GetItemFlags() { return GetUInt32Value(ITEM_FIELD_FLAGS); }

    void SetRandomPropData(uint32 randomProp, uint32 randomSeed = 0);
    uint32 GetRandomSeed() { return GetUInt32Value(ITEM_FIELD_PROPERTY_SEED); }
    uint32 GetRandomProperty() { return GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID); }

    void SetDurability(uint32 newDurability);
    void ModDurability(float modPct);
    uint32 GetDurability() { return GetUInt32Value(ITEM_FIELD_DURABILITY); }

    void UpdatePlayedTime();
    void ModPlayedTime(uint32 timeToAdd);
    uint32 GetPlayedTime() { return GetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME); }

private:
    ItemData *_itemData;
};
