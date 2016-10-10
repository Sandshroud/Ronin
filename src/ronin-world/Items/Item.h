/***
 * Demonstrike Core
 */

#pragma once

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

struct EnchantmentInstance
{
    SpellItemEnchantEntry *Enchantment;
    uint32 Slot;
    uint32 RandomSuffix;
    uint32 Duration;
    time_t ApplyTime;
    bool RemoveAtLogout;
    bool Dummy;
};

typedef std::vector<EnchantmentInstance*> enchantVector;

class SERVER_DECL Item : public Object
{
public:
    Item(ItemPrototype *proto, uint32 counter, uint32 fieldcount = ITEM_END);

    virtual ~Item();
    virtual void Init();
    virtual void Destruct();
    virtual void OnFieldUpdated(uint16 index);

    void SetItemInWorld(bool res);

    //! DB Serialization
    void LoadFromDB( Field *fields );
    void SaveToDB( int8 containerslot, uint8 slot, bool firstsave, QueryBuffer* buf );
    bool LoadAuctionItemFromDB( uint64 guid );
    void DeleteFromDB();

    bool IsWrapped() { return false; }

    RONIN_INLINE ItemPrototype* GetProto() const { return m_proto; }
    RONIN_INLINE Player* GetOwner() const { return m_owner; }
    void SetOwner( Player* owner );

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

    void SoulBind() { SetFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_SOULBOUND); }
    uint32 IsSoulbound() { return HasFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_SOULBOUND); }

    void AccountBind() { SetFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_BIND_TO_ACCOUNT); }
    uint32 IsAccountbound() { return HasFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_BIND_TO_ACCOUNT);  }

    void MakeConjured() { SetFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_CONJURED); }
    uint32 IsConjured() { return HasFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_CONJURED); }

    void Lock() { RemoveFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_OPENABLE); }
    void UnLock() { SetFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_OPENABLE); }

    void Wrap() { SetFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_ITEM_WRAPPER); }
    void UnWrap() { RemoveFlag(ITEM_FIELD_FLAGS, DBC_ITEMFLAG_ITEM_WRAPPER); }

    void ClearFlags() { SetFlag(ITEM_FIELD_FLAGS, 0); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    uint32 GetItemRandomPropertyId() const { return m_uint32Values[ITEM_FIELD_RANDOM_PROPERTIES_ID]; }
    uint32 GetItemPropertySeed() { return m_uint32Values[ITEM_FIELD_PROPERTY_SEED]; }

    void SetItemRandomPropertyData(uint32 id, uint32 seed)
    {
        SetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID, id);
        SetUInt32Value(ITEM_FIELD_PROPERTY_SEED, seed);
    }

    void SetDurability(uint32 Value) { SetUInt32Value(ITEM_FIELD_DURABILITY, Value); };
    void SetDurabilityMax(uint32 Value) { SetUInt32Value(ITEM_FIELD_MAXDURABILITY, Value); };

    uint32 GetDurability() { return GetUInt32Value(ITEM_FIELD_DURABILITY); }
    uint32 GetDurabilityMax() { return GetUInt32Value(ITEM_FIELD_MAXDURABILITY); }

    void SetDurabilityToMax() { SetUInt32Value(ITEM_FIELD_DURABILITY, GetUInt32Value(ITEM_FIELD_MAXDURABILITY)); }

    uint32 GetEnchantmentId(uint32 index) { return GetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA + 3 * index); }
    void SetEnchantmentId(uint32 index, uint32 value) { SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA + 3 * index, value); }

    uint32 GetEnchantmentDuration(uint32 index) { return GetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA + 1 + 3 * index); }
    void SetEnchantmentDuration(uint32 index, uint32 value) { SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA + 1 + 3 * index, value); }

    uint32 GetEnchantmentCharges(uint32 index) { return GetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA + 2 + 3 * index); }
    void SetEnchantmentCharges(uint32 index, uint32 value) { SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA + 2 + 3 * index, value); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool IsEligibleForRefund();
    RONIN_INLINE void Bind(uint32 bondtype)
    {
        if(ItemPrototype *proto = GetProto())
        {
            if(bondtype != ITEM_BIND_NONE && proto->Bonding == bondtype)
                SoulBind();
            if(proto->Flags & DBC_ITEMFLAG_BIND_TO_ACCOUNT)
                AccountBind();
        }
    }

    RONIN_INLINE uint32 GetChargesLeft()
    {
        for( uint32 x = 0; x < 5; x++ )
            if( m_proto->Spells[x].Id )
                return GetUInt32Value( ITEM_FIELD_SPELL_CHARGES + x );
        return 0;
    }

    RONIN_INLINE time_t GetEnchantmentApplytime( uint32 slot )
    {
        time_t ret = 0;
        m_enchantLock.Acquire();
        if(EnchantmentInstance *enchant = m_enchantments[slot])
            ret = enchant->ApplyTime;
        m_enchantLock.Release();
        return ret;
    }

    //! Adds an enchantment to the item.
    int32 AddEnchantment( SpellItemEnchantEntry* Enchantment, uint32 Duration, bool Perm = false, bool apply = true, bool RemoveAtLogout = false, uint32 Slot_ = 0, uint32 RandomSuffix = 0, bool dummy = false );
    void LoadEnchantment(uint8 slot, uint32 enchantId, uint32 suffix, uint32 expireTime, uint32 charges);

    const char* ConstructItemLink() { return m_proto->ConstructItemLink(GetItemRandomPropertyId(), GetItemPropertySeed(), GetStackCount()).c_str(); }

    //! Removes an enchantment from the item.
    void RemoveEnchantment( uint32 EnchantmentSlot );

    // Removes related temporary enchants
    void RemoveRelatedEnchants( SpellItemEnchantEntry* newEnchant );

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

    //! Loads any random properties the item has.
    void LoadRandomProperties();

    void RemovePermanentEnchant();
    void RemoveTemporaryEnchant();
    void RemoveSocketBonusEnchant();

    RONIN_INLINE void SetCount( uint32 amt ) { SetUInt32Value( ITEM_FIELD_STACK_COUNT, amt ); }
    RONIN_INLINE bool IsAmmoBag() { return (m_proto->Class == ITEM_CLASS_QUIVER); }

    void RemoveFromWorld();

    bool locked;
    bool m_isDirty, m_deleted;

    uint32 CountGemsWithLimitId(uint32 Limit);
    EnchantmentInstance* GetEnchantment( uint32 slot ) { return m_enchantments[slot]; }
    bool IsGemRelated( SpellItemEnchantEntry* Enchantment );

    bool HasEnchantments()
    {
        bool ret = false;
        m_enchantLock.Acquire();
        for(uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; i++)
        {
            if(m_enchantments[i])
            {
                ret = true;
                break;
            }
        }
        m_enchantLock.Release();
        return false;
    }

    uint32 GetTextID() { return m_textId; };
    void SetTextID(uint32 newtxt) { m_textId = newtxt; };

protected:
    Player* m_owner; // let's not bother the manager with unneeded requests
    ItemPrototype* m_proto;
    uint32 m_textId;

    Mutex m_enchantLock;
    enchantVector m_enchantments;
};
