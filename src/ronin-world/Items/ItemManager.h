/*
 * Sandshroud Project Ronin
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
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

#pragma pack(PRAGMA_PACK)

struct ItemPrototype
{
    uint32 ItemId;
    uint32 Class;
    uint32 SubClass;
    int32  subClassSound;
    float f_unk[2];
    std::string Name;
    uint32 DisplayInfoID;
    uint32 Quality;
    uint32 Flags;
    uint32 FlagsExtra;
    uint32 BuyCount;
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
    float  minDamage;
    float  maxDamage;
    uint32 Armor;
    uint32 Durability;

    uint32 ScalingStatDistribution;
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
    std::string Description;
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
    uint32 ZoneNameID;
    uint32 MapID;
    uint32 BagFamily;
    uint32 TotemCategory;
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
    uint8 weaponType;

    std::string ConstructItemLink(uint32 random_prop, uint32 random_suffix, uint32 stack);
    bool ValidateItemLink(const char *szLink);
    bool ValidateItemSpell(uint32 SpellID);
};

#pragma pack(PRAGMA_POP)

enum ItemProtoWeaponType
{
    PROTO_WEAPON_TYPE_NONE,
    PROTO_WEAPON_TYPE_ONE_HANDER,
    PROTO_WEAPON_TYPE_TWO_HANDER,
    PROTO_WEAPON_TYPE_ONE_HANDER_CASTER,
    PROTO_WEAPON_TYPE_TWO_HANDER_CASTER,
    PROTO_WEAPON_TYPE_RANGED,
    PROTO_WEAPON_TYPE_RANGED_CASTER
};

class SERVER_DECL ItemManager : public Singleton<ItemManager>
{
public:
    ItemManager();
    ~ItemManager();

    uint32 GetSkillForItem(ItemPrototype *proto);
    uint64 CalculateBuyPrice(uint32 itemId, uint32 count, Player *player, Creature *vendor, ItemExtendedCostEntry *ec);
    uint64 CalculateSellPrice(uint32 itemId, uint32 count);

    int32 GetScalingDPSMod(ItemPrototype *proto, ScalingStatValuesEntry *scalingStat, float &damageMod);
    int32 GetScalingArmor(ItemPrototype *proto, ScalingStatValuesEntry *scalingStat);
    int32 CalcStatMod(ItemPrototype *proto, ScalingStatValuesEntry *scalingStat, int32 modifier);

public:
    static const uint32 arm_skills[7];
    static const uint32 weap_skills[21];
    static const char *g_itemQualityColours[ITEM_QUALITY_DBC_MAX];

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
