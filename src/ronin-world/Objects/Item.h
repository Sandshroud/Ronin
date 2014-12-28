/***
 * Demonstrike Core
 */

#pragma once

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

const static ItemProf prof[22] = { {4, 2}, {4, 4}, {4, 8}, {4, 16}, {4, 64},
                                   {2, 1}, {2, 2}, {2, 4}, {2, 8}, {2, 16}, {2, 32}, {2, 64}, {2, 128}, {2, 256}, {2, 1024}, {2, 8192}, {2, 32768}, {2, 65536}, {2, 131072},
                                   {2, 262144}, {2, 524288}, {2, 1048576} };

const static uint16 arm_skills[7] = { 0, SKILL_CLOTH, SKILL_LEATHER, SKILL_MAIL, SKILL_PLATE_MAIL, 0, SKILL_SHIELD };

const static uint16 weap_skills[21] = {
    SKILL_AXES,
    SKILL_2H_AXES,
    SKILL_BOWS,
    SKILL_GUNS,
    SKILL_MACES,
    SKILL_2H_MACES,
    SKILL_POLEARMS,
    SKILL_SWORDS,
    SKILL_2H_SWORDS,
    0,
    SKILL_STAVES,
    0,
    0,
    SKILL_FIST_WEAPONS,
    0,//<-13
    SKILL_DAGGERS,
    SKILL_THROWN,
    SKILL_SPEARS,
    SKILL_CROSSBOWS,
    SKILL_WANDS,
    SKILL_FISHING };

const static float pricemod[9] = {
    1.f,    // HATED
    1.f,    // HOSTILE
    1.f,    // UNFRIENDLY
    1.f,    // NEUTRAL
    0.95f,  // FRIENDLY
    0.90f,  // HONORED
    0.85f,  // REVERED
    0.80f };// EXHALTED

const static float SuffixMods[NUM_INVENTORY_TYPES] = {
    0.f,    // No slot
    0.46f,  // HEAD
    0.26f,  // NECK
    0.35f,  // SHOULDERS
    0.46f,  // BODY
    0.46f,  // CHEST
    0.35f,  // WAIST
    0.46f,  // LEGS
    0.34f,  // FEET
    0.26f,  // WRISTS
    0.35f,  // HANDS
    0.26f,  // FINGER
    0.f,    // TRINKET
    0.19f,  // WEAPON
    0.25f,  // SHEILD
    0.14f,  // RANGED
    0.26f,  // CLOAK
    0.46f,  // 2H-WEAPON
    0.f,    // BAG
    0.f,    // TABARD
    0.46f,  // ROBE
    0.19f,  // MAIN-HAND WEAPON
    0.19f,  // OFF-HAND WEAPON
    0.26f,  // HOLDABLE
    0.f,    // AMMO
    0.26f,  // THROWN
    0.14f,  // RANGED
    0.f,    // QUIVER
    0.26f };// RELIC

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

typedef std::map< uint32, EnchantmentInstance > EnchantmentMap;

#define RANDOM_SUFFIX_MAGIC_CALCULATION( __suffix, __scale ) int32( float( ( float( ( __suffix ) ) * float( ( __scale ) ) ) ) / 13340.0f );

class SERVER_DECL Item : public Object
{
public:
    Item( WoWGuid guid, uint32 fieldCount = ITEM_END );
    virtual ~Item();
    virtual void Init();
    virtual void Destruct();

    void Create( uint32 itemid, Player* owner );

    uint32 CalcMinDamage();
    uint32 CalcMaxDamage();

    RONIN_INLINE ItemPrototype* GetProto() const { return m_itemProto; }
    RONIN_INLINE void SetProto( ItemPrototype* pr ) { m_itemProto = pr; }

    RONIN_INLINE Player* GetOwner() const { return m_owner; }
    void SetOwner( Player* owner );

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetOwnerGUID(uint64 GUID) { SetUInt64Value(ITEM_FIELD_OWNER, GUID);   }
    uint64 GetOwnerGUID() { return GetUInt64Value(ITEM_FIELD_OWNER);  }

    void SetContainerGUID(uint64 GUID) { SetUInt64Value(ITEM_FIELD_CONTAINED, GUID);   }
    uint64 GetContainerGUID() { return GetUInt64Value(ITEM_FIELD_CONTAINED);   }

    void SetCreatorGUID(uint64 GUID) { SetUInt64Value(ITEM_FIELD_CREATOR, GUID); }
    void SetGiftCreatorGUID(uint64 GUID) { SetUInt64Value(ITEM_FIELD_GIFTCREATOR, GUID); }

    uint64 GetCreatorGUID() { return GetUInt64Value(ITEM_FIELD_CREATOR); }
    uint64 GetGiftCreatorGUID() { return GetUInt64Value(ITEM_FIELD_GIFTCREATOR); }

    void SetStackCount(uint32 amt) { SetUInt32Value(ITEM_FIELD_STACK_COUNT,  amt); }
    uint32 GetStackCount() { return GetUInt32Value(ITEM_FIELD_STACK_COUNT); }
    void ModStackCount(int32 val) { ModUnsigned32Value(ITEM_FIELD_STACK_COUNT, val); }

    void SetDuration(uint32 durationseconds) { SetUInt32Value(ITEM_FIELD_DURATION, durationseconds); }
    uint32 GetDuration() { return GetUInt32Value(ITEM_FIELD_DURATION); }

    void SetCharges(uint32 index, uint32 charges) { SetUInt32Value(ITEM_FIELD_SPELL_CHARGES + index, charges); }
    void ModCharges(uint32 index, int32 val) { ModSignedInt32Value(ITEM_FIELD_SPELL_CHARGES + index, val); }
    uint32 GetCharges(uint32 index) const { return GetUInt32Value(ITEM_FIELD_SPELL_CHARGES + index); }

    /////////////////////////////////////////////////// FLAGS ////////////////////////////////////////////////////////////

    void SoulBind() { SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_SOULBOUND); }
    uint32 IsSoulbound() { return HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_QUEST | ITEM_FLAG_SOULBOUND); }

    void AccountBind() { SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_BINDONACC); }
    uint32 IsAccountbound() { return HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_BINDONACC);  }

    void MakeConjured() { SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_CONJURED); }
    uint32 IsConjured() { return HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_CONJURED); }

    void Lock() { RemoveFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_LOOTABLE); }
    void UnLock() { SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_LOOTABLE); }

    void Wrap() { SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAPPED); }
    void UnWrap() { RemoveFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAPPED); }
    bool IsWrapped() { return HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAPPED); }

    void ClearFlags() { SetFlag(ITEM_FIELD_FLAGS, 0); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    uint32 GetItemRandomPropertyId() const { return GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID); }
    uint32 GetItemRandomSuffixFactor() { return GetUInt32Value(ITEM_FIELD_PROPERTY_SEED); }

    void SetItemRandomPropertyId(uint32 id)
    {
        SetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID, id);
    }

    void SetItemRandomSuffixFactor(uint32 factor)
    {
        SetUInt32Value(ITEM_FIELD_PROPERTY_SEED, factor);
    }

    void SetRandomSuffix(uint32 id)
    {
        int32 r_id = -(int32(id));
        SetItemRandomPropertyId((uint32)r_id);
        SetItemRandomSuffixFactor(Item::GenerateRandomSuffixFactor(m_itemProto));
    }

    void SetDurability(uint32 Value) { SetUInt32Value(ITEM_FIELD_DURABILITY, Value); };
    void SetDurabilityMax(uint32 Value) { SetUInt32Value(ITEM_FIELD_MAXDURABILITY, Value); };

    uint32 GetDurability() { return GetUInt32Value(ITEM_FIELD_DURABILITY); }
    uint32 GetDurabilityMax() { return GetUInt32Value(ITEM_FIELD_MAXDURABILITY); }

    void SetDurabilityToMax() { SetUInt32Value(ITEM_FIELD_DURABILITY, GetUInt32Value(ITEM_FIELD_MAXDURABILITY)); }

    uint32 GetEnchantmentId(uint32 index) { return GetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + 3 * index); }
    void SetEnchantmentId(uint32 index, uint32 value) { SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + 3 * index, value); }

    uint32 GetEnchantmentDuration(uint32 index) { return GetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + 1 + 3 * index); }
    void SetEnchantmentDuration(uint32 index, uint32 value) { SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + 1 + 3 * index, value); }

    uint32 GetEnchantmentCharges(uint32 index) { return GetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + 2 + 3 * index); }
    void SetEnchantmentCharges(uint32 index, uint32 value) { SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + 2 + 3 * index, value); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //! DB Serialization
    virtual void LoadFromDB( Field *fields, Player* plr, bool light );
    void SaveToDB( int16 containerslot, int16 slot, bool firstsave, QueryBuffer* buf );
    bool LoadAuctionItemFromDB( uint64 guid );
    void DeleteFromDB();
    bool IsEligibleForRefund();

    RONIN_INLINE void Bind(uint32 bondtype)
    {
        if(GetProto())
        {
            if(bondtype != ITEM_BIND_NONE && GetProto()->Bonding == bondtype)
                SoulBind();
            if(GetProto()->Flags & ITEM_FLAG_BINDONACC)
                AccountBind();
        }
    }

    RONIN_INLINE uint32 GetChargesLeft()
    {
        for( uint32 x = 0; x < 5; x++ )
            if( m_itemProto->Spells[x].Id )
                return GetUInt32Value( ITEM_FIELD_SPELL_CHARGES + x );
        return 0;
    }

    RONIN_INLINE time_t GetEnchantmentApplytime( uint32 slot )
    {
        EnchantmentMap::iterator itr = Enchantments.find( slot );
        if( itr == Enchantments.end() )
            return 0;
        else
            return itr->second.ApplyTime;
    }

    //! Adds an enchantment to the item.
    int32 AddEnchantment( SpellItemEnchantEntry* Enchantment, uint32 Duration, bool Perm = false, bool apply = true, bool RemoveAtLogout = false, uint32 Slot_ = 0, uint32 RandomSuffix = 0, bool dummy = false );
    uint32 GetMaxSocketsCount();

    const char* ConstructItemLink() { return m_itemProto->ConstructItemLink(GetItemRandomPropertyId(), GetItemRandomSuffixFactor(), GetStackCount()).c_str(); }

    //! Removes an enchantment from the item.
    void RemoveEnchantment( uint32 EnchantmentSlot );

    // Removes related temporary enchants
    void RemoveRelatedEnchants( SpellItemEnchantEntry* newEnchant );

    //! Adds the bonus on an enchanted item.
    void ApplyEnchantmentBonus( uint32 Slot, bool Apply );

    //! Applies all enchantment bonuses (use on equip)
    void ApplyEnchantmentBonuses();

    //! Removes all enchantment bonuses (use on dequip)
    void RemoveEnchantmentBonuses();

    //! Event to remove an enchantment.
    void EventRemoveEnchantment( uint32 Slot );

    //! Check if we have an enchantment of this id?
    int32 HasEnchantment( uint32 Id );

    //! Modify the time of an existing enchantment.
    void ModifyEnchantmentTime( uint32 Slot, uint32 Duration );

    //! Find free enchantment slot.
    int32 FindFreeEnchantSlot( SpellItemEnchantEntry* Enchantment, uint32 random_type );

    //! Removes all enchantments.
    void RemoveAllEnchantments( bool OnlyTemporary );

    //! Sends SMSG_ITEM_UPDATE_ENCHANT_TIME
    void SendEnchantTimeUpdate( uint32 Slot, uint32 Duration );

    //! Applies any random properties the item has.
    void ApplyRandomProperties( bool apply );

    void RemoveProfessionEnchant();
    void RemoveSocketBonusEnchant();

    RONIN_INLINE void SetCount( uint32 amt ) { SetUInt32Value( ITEM_FIELD_STACK_COUNT, amt ); }
    RONIN_INLINE bool IsAmmoBag() { return (m_itemProto->Class == ITEM_CLASS_QUIVER); }

    void SetInWorld() { m_inWorld = true; }
    bool IsInWorld() { return m_inWorld; }
    void RemoveFromWorld(bool destroy);

    bool m_locked;

    uint32 CountGemsWithLimitId(uint32 Limit);
    EnchantmentInstance* GetEnchantment( uint32 slot );
    bool IsGemRelated( SpellItemEnchantEntry* Enchantment );

    static uint32 GenerateRandomSuffixFactor( ItemPrototype* m_itemProto );

    RONIN_INLINE void SetRandomProperty( uint32 id )
    {
        SetUInt32Value( ITEM_FIELD_RANDOM_PROPERTIES_ID, id );
    }

    bool HasEnchantments() { return ( Enchantments.size() > 0 ) ? true : false; }

    uint32 wrapped_item_id;
    uint32 GetTextID() { return textid; };
    void SetTextID(uint32 newtxt) { textid = newtxt; };

protected:
    ItemData *_itemData;
    ItemPrototype* m_itemProto;
    EnchantmentMap Enchantments;
    Player* m_owner; // let's not bother the manager with unneeded requests
    uint32 textid;
    bool m_inWorld;

public:
    bool StatsApplied;

    uint32 OnUseSpells[3];
    bool HasEnchantedOnUseSpell( uint32 id )
    {
        for( uint32 i = 0; i < 3; ++i )
            if( OnUseSpells[ i ] == id )
                return true;
        return false;
    }

public:
    static uint32 GetSkillByProto( uint32, uint32 );

    static uint32 GetSellPriceForItem( ItemPrototype* proto, uint32 count );
    static uint32 GetBuyPriceForItem( ItemPrototype* proto, uint32 count, Player* plr, Creature* vendor );

    static uint32 GetSellPriceForItem( uint32 itemid, uint32 count);
    static uint32 GetBuyPriceForItem( uint32 itemid, uint32 count, Player* plr, Creature* vendor );

};
