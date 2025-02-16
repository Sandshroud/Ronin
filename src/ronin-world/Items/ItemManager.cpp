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

#include "StdAfx.h"

initialiseSingleton( ItemManager );

const uint32 ItemManager::arm_skills[7] = { 0, SKILL_CLOTH, SKILL_LEATHER, SKILL_MAIL, SKILL_PLATE_MAIL, 0, SKILL_SHIELD };
const uint32 ItemManager::weap_skills[21] = { SKILL_AXES, SKILL_2H_AXES, SKILL_BOWS, SKILL_GUNS, SKILL_MACES, SKILL_2H_MACES, SKILL_POLEARMS, SKILL_SWORDS, SKILL_2H_SWORDS, 0, SKILL_STAVES, 0, 0, SKILL_FIST_WEAPONS, 0, SKILL_DAGGERS, SKILL_THROWN, SKILL_SPEARS, SKILL_CROSSBOWS, SKILL_WANDS, SKILL_FISHING };
const char *ItemManager::g_itemQualityColours[ITEM_QUALITY_DBC_MAX] = {
    "|cff9d9d9d",   // Grey
    "|cffffffff",   // White
    "|cff1eff00",   // Green
    "|cff0070dd",   // Blue
    "|cffa335ee",   // Purple
    "|cffff8000",   // Orange
    "|cffe6cc80"    // Artifact
};

ItemManager::ItemManager()
{

}

ItemManager::~ItemManager()
{
    while(!m_itemPrototypeContainer.empty())
    {
        delete m_itemPrototypeContainer.begin()->second;
        m_itemPrototypeContainer.erase(m_itemPrototypeContainer.begin());
    }
}

uint32 ItemManager::GetSkillForItem(ItemPrototype *proto)
{
    uint32 skill = SKILL_UNARMED;
    if(proto->Class == 4 && proto->SubClass < 7 )
        skill = arm_skills[proto->SubClass];
    else if( proto->Class == 2 && proto->SubClass < 20 )//no skill for fishing(21)
        skill = weap_skills[proto->SubClass];
    if( skill == 0 || skill == SKILL_FIST_WEAPONS )
        skill = SKILL_UNARMED;
    return SKILL_UNARMED;
}

uint64 ItemManager::CalculateBuyPrice(uint32 itemId, uint32 count, Player *player, Creature *vendor, ItemExtendedCostEntry *ec)
{
    if(ec && !(ec->flags & 0x1000))
        return 0; // Items with the right extended cost have no gold cost

    uint64 buyPrice = 0xFFFFFFFFFFFFFFFF;
    if(ItemPrototype *proto = LookupEntry(itemId))
    {
        buyPrice = proto->BuyPrice;
        if(proto->BuyCount) // Divide our buy price by our buycount
            buyPrice /= proto->BuyCount;
        buyPrice *= count;
    }

    // Prevent overflow of double conversions by only allowing 2^48
    if( (buyPrice&0xFFFF000000000000) == 0 && player != NULL && vendor != NULL )
    {
        Standing plrstanding = player->GetFactionInterface()->GetStandingRank( vendor->GetFactionID() );
        if(plrstanding > STANDING_NEUTRAL)
            buyPrice = double2int64(double(buyPrice) * (1.f - (0.05f*uint8(plrstanding-STANDING_NEUTRAL))));
    }
    return buyPrice;
}

uint64 ItemManager::CalculateSellPrice(uint32 itemId, uint32 count)
{
    uint64 sellPrice = 0;
    if(ItemPrototype *proto = LookupEntry(itemId))
    {
        sellPrice = proto->SellPrice;
        sellPrice *= count;
    }
    return sellPrice;
}

int32 ItemManager::GetScalingDPSMod(ItemPrototype *proto, ScalingStatValuesEntry *scalingStat, float &damageMod)
{
    if(uint8 type = proto->weaponType)
    {
        damageMod = 0.3f;
        if(type == PROTO_WEAPON_TYPE_TWO_HANDER || type == PROTO_WEAPON_TYPE_TWO_HANDER_CASTER)
            damageMod = 0.2f;
        return scalingStat->dpsMod[type-1];
    }
    return 0;
}

int32 ItemManager::GetScalingArmor(ItemPrototype *proto, ScalingStatValuesEntry *scalingStat)
{
    if(proto->Class == ITEM_CLASS_ARMOR)
    {
        uint8 armorType = std::min<uint8>(4, proto->SubClass - 1);
        switch (proto->InventoryType)
        {
            case INVTYPE_SHOULDERS: return scalingStat->armorMod[0][armorType];
            case INVTYPE_CHEST: case INVTYPE_ROBE: return scalingStat->armorMod[1][armorType];
            case INVTYPE_HEAD: return scalingStat->armorMod[2][armorType];
            case INVTYPE_LEGS: return scalingStat->armorMod[3][armorType];
            case INVTYPE_FEET: return scalingStat->armorMod[4][armorType];
            case INVTYPE_WAIST: return scalingStat->armorMod[5][armorType];
            case INVTYPE_HANDS: return scalingStat->armorMod[6][armorType];
            case INVTYPE_WRISTS: return scalingStat->armorMod[7][armorType];
            case INVTYPE_CLOAK: return scalingStat->cloakArmor;
        }
    }
    return 0;
}

int32 ItemManager::CalcStatMod(ItemPrototype *proto, ScalingStatValuesEntry *scalingStat, int32 modifier)
{
    if (proto->InventoryType < NUM_INVENTORY_TYPES)
    {
        switch (proto->InventoryType)
        {
        case INVTYPE_HEAD: case INVTYPE_CHEST: case INVTYPE_LEGS: case INVTYPE_2HWEAPON: case INVTYPE_ROBE:                             return (scalingStat->ssdMultiplier[0] * modifier)/10000;
        case INVTYPE_SHOULDERS: case INVTYPE_WAIST: case INVTYPE_FEET: case INVTYPE_HANDS: case INVTYPE_TRINKET:                        return (scalingStat->ssdMultiplier[1] * modifier)/10000;
        case INVTYPE_NECK: case INVTYPE_WRISTS: case INVTYPE_FINGER: case INVTYPE_SHIELD: case INVTYPE_CLOAK: case INVTYPE_HOLDABLE:    return (scalingStat->ssdMultiplier[2] * modifier)/10000;
        case INVTYPE_RANGED: case INVTYPE_THROWN: case INVTYPE_RANGEDRIGHT: case INVTYPE_RELIC:                                         return (scalingStat->ssdMultiplier[3] * modifier)/10000;
        case INVTYPE_WEAPON: case INVTYPE_WEAPONMAINHAND: case INVTYPE_WEAPONOFFHAND:                                                   return (scalingStat->ssdMultiplier[4] * modifier)/10000;
        }
    }
    return 0;
}

uint32 CalcWeaponDurability(uint32 subClass, uint32 quality, uint32 itemLevel);
uint32 CalcArmorDurability(uint32 inventoryType, uint32 quality, uint32 itemLevel);

void ItemManager::InitializeItemPrototypes()
{
    // It has to be empty for us to fill in db2 data
    if(!m_itemPrototypeContainer.empty())
        return;

    // Ensure that our db2 tables are loaded
    db2Item.Load();
    db2ItemSparse.Load();

    sLog.Notice("ItemPrototypeSystem", "Loading %u items!", db2Item.GetNumRows());
    ItemDataEntry *itemData = NULL;
    ItemSparseEntry *sparse = NULL;
    for(uint32 i = 0; i < db2Item.GetNumRows(); i++)
    {
        itemData = db2Item.LookupRow(i);
        sparse = db2ItemSparse.LookupEntry(itemData->ID);
        if(sparse == NULL)
            continue;
        if(m_itemPrototypeContainer.find(itemData->ID) != m_itemPrototypeContainer.end())
            continue;

        ItemPrototype* proto = new ItemPrototype();
        // Non dbc data
        proto->minDamage = 0.f;
        proto->maxDamage = 0.f;
        proto->Armor = 0;
        proto->Durability = 0;
        // These are set later.
        proto->ItemId = itemData->ID;
        proto->Class = itemData->Class;
        proto->SubClass = itemData->SubClass;
        proto->subClassSound = itemData->SoundOverrideSubclass;
        proto->f_unk[0] = sparse->f_unk[0];
        proto->f_unk[1] = sparse->f_unk[1];
        proto->Name = sparse->Name;
        proto->DisplayInfoID = itemData->DisplayId;
        proto->Quality = sparse->Quality;
        proto->Flags = sparse->Flags;
        proto->FlagsExtra = sparse->Flags2;
        proto->BuyCount = sparse->BuyCount;
        proto->BuyPrice = sparse->BuyPrice;
        proto->SellPrice = sparse->SellPrice;
        proto->InventoryType = itemData->InventoryType;
        proto->AllowableClass = sparse->AllowableClass;
        proto->AllowableRace = sparse->AllowableRace;
        proto->ItemLevel = sparse->ItemLevel;
        proto->RequiredLevel = sparse->RequiredLevel;
        proto->RequiredSkill = sparse->RequiredSkill;
        proto->RequiredSkillRank = sparse->RequiredSkillRank;
        proto->RequiredSpell = sparse->RequiredSpell;
        proto->RequiredPlayerRank1 = sparse->RequiredHonorRank;
        proto->RequiredPlayerRank2 = sparse->RequiredCityRank;
        proto->RequiredFaction = sparse->RequiredReputationFaction;
        proto->RequiredFactionStanding = sparse->RequiredReputationRank;
        proto->MaxCount = sparse->Stackable;
        proto->Unique = sparse->MaxCount;
        proto->ContainerSlots = sparse->ContainerSlots;
        for(uint8 i = 0; i < 10; i++)
        {
            proto->Stats[i].Type = sparse->ItemStatType[i];
            proto->Stats[i].Value = sparse->ItemStatValue[i];
            if(i >= 5)
                continue;
            proto->Spells[i].Id = sparse->SpellId[i];
            proto->Spells[i].Trigger = sparse->SpellTrigger[i];
            proto->Spells[i].Charges = sparse->SpellCharges[i];
            proto->Spells[i].Cooldown = sparse->SpellCooldown[i];
            proto->Spells[i].Category = sparse->SpellCategory[i];
            proto->Spells[i].CategoryCooldown = sparse->SpellCategoryCooldown[i];
            if(i >= 3)
                continue;
            proto->ItemSocket[i] = sparse->SocketColor[i];
            proto->ItemContent[i] = sparse->SocketContent[i];
        }

        proto->ScalingStatDistribution = sparse->ScalingStatDistribution;
        proto->DamageType = sparse->DamageType;
        proto->Delay = sparse->Delay;
        proto->Range = sparse->RangedModRange;
        proto->Bonding = sparse->Bonding;
        proto->Description = sparse->Description;
        proto->PageId = sparse->PageText;
        proto->PageLanguage = sparse->LanguageID;
        proto->PageMaterial = sparse->PageMaterial;
        proto->QuestId = sparse->StartQuest;
        proto->LockId = sparse->LockID;
        proto->LockMaterial = sparse->Material;
        proto->SheathID = sparse->Sheath;
        proto->RandomPropId = sparse->RandomProperty;
        proto->RandomSuffixId = sparse->RandomSuffix;
        proto->ItemSet = sparse->ItemSet;
        proto->ZoneNameID = sparse->Area;
        proto->MapID = sparse->Map;
        proto->BagFamily = sparse->BagFamily;
        proto->TotemCategory = sparse->TotemCategory;
        proto->SocketBonus = sparse->SocketBonus;
        proto->GemProperties = sparse->GemProperties;
        proto->ArmorDamageModifier = sparse->ArmorDamageModifier;
        proto->Duration = sparse->Duration;
        proto->ItemLimitCategory = sparse->ItemLimitCategory;
        proto->HolidayId = sparse->HolidayId;
        proto->StatScalingFactor = sparse->StatScalingFactor;
        proto->lowercase_name = proto->Name.c_str();
        for(uint32 j = 0; j < proto->lowercase_name.length(); ++j)
            proto->lowercase_name[j] = tolower(proto->lowercase_name[j]);

        // Custom field defaults
        proto->FoodType = 0;
        proto->ForcedPetId = 0;
        proto->ItemSetRank = 0;
        proto->DisenchantReqSkill = 0;
        proto->weaponType = PROTO_WEAPON_TYPE_NONE;
        m_itemPrototypeContainer.insert(std::make_pair(itemData->ID, proto));
    }
    // Unload our db2 files since we won't need them
    db2Item.Unload();
    db2ItemSparse.Unload();

    // Load our database overrides as well as precalculated item data
    LoadItemOverrides();
}

void ItemManager::LoadItemOverrides()
{
#define CHECK_OVERRIDE_VALUE(proto, field, function, count, flag) if(proto->field != fields[count].function()) { proto->field = fields[count].function(); overrideFlags |= flag; }count++;

    ItemPrototype* proto = NULL;
    sLog.Notice("ItemPrototypeSystem", "Loading item overrides...");
    if(QueryResult* result = WorldDatabase.Query("SELECT * FROM item_overrides"))
    {
        uint32 fieldCount = result->GetFieldCount();
        if(fieldCount != 107)
            sLog.outError("Incorrect field count in item override table %u\n", fieldCount);
        else
        {
            uint8 overrideFlags = 0x00;
            uint32 entry = 0, field_Count = 0;
            do
            {
                Field *fields = result->Fetch();
                entry = fields[field_Count++].GetUInt32();
                if(m_itemPrototypeContainer.find(entry) == m_itemPrototypeContainer.end())
                {
                    proto = new ItemPrototype();
                    proto->ItemId = entry;
                    proto->Class = 0;
                    proto->SubClass = 0;
                    proto->subClassSound = 0;
                    proto->f_unk[0] = 0.f;
                    proto->f_unk[1] = 0.f;
                    proto->Name = "";
                    proto->DisplayInfoID = 0;
                    proto->Quality = 0;
                    proto->Flags = 0;
                    proto->FlagsExtra = 0;
                    proto->BuyCount = 0;
                    proto->BuyPrice = 0;
                    proto->SellPrice = 0;
                    proto->InventoryType = 0;
                    proto->AllowableClass = 0;
                    proto->AllowableRace = 0;
                    proto->ItemLevel = 0;
                    proto->RequiredLevel = 0;
                    proto->RequiredSkill = 0;
                    proto->RequiredSkillRank = 0;
                    proto->RequiredSpell = 0;
                    proto->RequiredPlayerRank1 = 0;
                    proto->RequiredPlayerRank2 = 0;
                    proto->RequiredFaction = 0;
                    proto->RequiredFactionStanding = 0;
                    proto->MaxCount = 0;
                    proto->Unique = 0;
                    proto->ContainerSlots = 0;
                    proto->minDamage = 0.f;
                    proto->maxDamage = 0.f;
                    proto->Armor = 0;
                    proto->Durability = 0;
                    for(uint8 i = 0; i < 10; i++)
                    {
                        proto->Stats[i].Type = 0;
                        proto->Stats[i].Value = 0;
                        if(i >= 5)
                            continue;
                        proto->Spells[i].Id = 0;
                        proto->Spells[i].Trigger = 0;
                        proto->Spells[i].Charges = 0;
                        proto->Spells[i].Cooldown = 0;
                        proto->Spells[i].Category = 0;
                        proto->Spells[i].CategoryCooldown = 0;
                        if(i >= 3)
                            continue;
                        proto->ItemSocket[i] = 0;
                        proto->ItemContent[i] = 0;
                    }

                    proto->ScalingStatDistribution = 0;
                    proto->DamageType = 0;
                    proto->Delay = 0;
                    proto->Range = 0;
                    proto->Bonding = 0;
                    proto->Description = "";
                    proto->PageId = 0;
                    proto->PageLanguage = 0;
                    proto->PageMaterial = 0;
                    proto->QuestId = 0;
                    proto->LockId = 0;
                    proto->LockMaterial = 0;
                    proto->SheathID = 0;
                    proto->RandomPropId = 0;
                    proto->RandomSuffixId = 0;
                    proto->ItemSet = 0;
                    proto->ZoneNameID = 0;
                    proto->MapID = 0;
                    proto->BagFamily = 0;
                    proto->TotemCategory = 0;
                    proto->SocketBonus = 0;
                    proto->GemProperties = 0;
                    proto->ArmorDamageModifier = 0;
                    proto->Duration = 0;
                    proto->ItemLimitCategory = 0;
                    proto->HolidayId = 0;
                    proto->StatScalingFactor = 0;
                    proto->lowercase_name = "";
                    // Custom field defaults
                    proto->FoodType = 0;
                    proto->ForcedPetId = 0;
                    proto->ItemSetRank = 0;
                    proto->DisenchantReqSkill = 0;
                    proto->weaponType = PROTO_WEAPON_TYPE_NONE;
                    m_itemPrototypeContainer.insert(std::make_pair(entry, proto));
                    overrideFlags = 0x01 | 0x02;
                } else proto = m_itemPrototypeContainer.at(entry);

                CHECK_OVERRIDE_VALUE(proto, Class, GetUInt32, field_Count, 0x01);
                CHECK_OVERRIDE_VALUE(proto, SubClass, GetUInt32, field_Count, 0x01);
                CHECK_OVERRIDE_VALUE(proto, subClassSound, GetInt32, field_Count, 0x01);
                // Avoid macro usage for name as we need to update lowercase as well
                if(strcmp(proto->Name.c_str(), fields[field_Count].GetString()))
                {
                    proto->lowercase_name = (proto->Name = fields[field_Count].GetString()).c_str();
                    for(uint32 j = 0; j < proto->lowercase_name.length(); ++j)
                        proto->lowercase_name[j] = tolower(proto->lowercase_name[j]);
                    overrideFlags |= 0x02;
                }field_Count++;
                CHECK_OVERRIDE_VALUE(proto, DisplayInfoID, GetUInt32, field_Count, 0x01);
                CHECK_OVERRIDE_VALUE(proto, Quality, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, Flags, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, FlagsExtra, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, BuyCount, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, BuyPrice, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, SellPrice, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, InventoryType, GetUInt32, field_Count, 0x01);
                CHECK_OVERRIDE_VALUE(proto, AllowableClass, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, AllowableRace, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, ItemLevel, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, RequiredLevel, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, RequiredSkill, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, RequiredSkillRank, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, RequiredSpell, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, RequiredPlayerRank1, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, RequiredPlayerRank2, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, RequiredFaction, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, RequiredFactionStanding, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, MaxCount, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, Unique, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, ContainerSlots, GetUInt32, field_Count, 0x02);
                for(uint8 i = 0; i < 10; i++)
                {
                    CHECK_OVERRIDE_VALUE(proto, Stats[i].Type, GetUInt32, field_Count, 0x02);
                    CHECK_OVERRIDE_VALUE(proto, Stats[i].Value, GetInt32, field_Count, 0x02);
                }
                CHECK_OVERRIDE_VALUE(proto, ScalingStatDistribution, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, DamageType, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, Delay, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, Range, GetFloat, field_Count, 0x02);
                for(uint8 i = 0; i < 5; i++)
                {
                    CHECK_OVERRIDE_VALUE(proto, Spells[i].Id, GetUInt32, field_Count, 0x02);
                    CHECK_OVERRIDE_VALUE(proto, Spells[i].Trigger, GetUInt32, field_Count, 0x02);
                    CHECK_OVERRIDE_VALUE(proto, Spells[i].Charges, GetInt32, field_Count, 0x02);
                    CHECK_OVERRIDE_VALUE(proto, Spells[i].Cooldown, GetInt32, field_Count, 0x02);
                    CHECK_OVERRIDE_VALUE(proto, Spells[i].Category, GetUInt32, field_Count, 0x02);
                    CHECK_OVERRIDE_VALUE(proto, Spells[i].CategoryCooldown, GetInt32, field_Count, 0x02);
                }
                CHECK_OVERRIDE_VALUE(proto, Bonding, GetUInt32, field_Count, 0x02);
                // Avoid macro for description as well, it needs to use strdup
                if(strcmp(proto->Description.c_str(), fields[field_Count].GetString()))
                {
                    proto->Description = fields[field_Count].GetString();
                    overrideFlags |= 0x02;
                }field_Count++;
                CHECK_OVERRIDE_VALUE(proto, PageId, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, PageLanguage, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, PageMaterial, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, QuestId, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, LockId, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, LockMaterial, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, SheathID, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, RandomPropId, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, RandomSuffixId, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, ItemSet, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, ZoneNameID, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, MapID, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, BagFamily, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, TotemCategory, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, ItemSocket[0], GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, ItemSocket[1], GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, ItemSocket[2], GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, SocketBonus, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, GemProperties, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, ArmorDamageModifier, GetFloat, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, Duration, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, ItemLimitCategory, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, HolidayId, GetUInt32, field_Count, 0x02);
                CHECK_OVERRIDE_VALUE(proto, StatScalingFactor, GetFloat, field_Count, 0x02);

                if(overrideFlags > 0)
                    m_overwritten.insert(std::make_pair(entry, overrideFlags));
                proto = NULL;
                overrideFlags = 0;
                entry = field_Count = 0;
            } while( result->NextRow() );
            sLog.Notice("ItemPrototypeSystem", "%u item overrides loaded!", m_overwritten.size());
        }
        delete result;
    }

#undef CHECK_OVERRIDE_VALUE

    sLog.Notice("ItemPrototypeSystem", "Setting static dbc data...");
    dbcArmorLocation.Load();
    dbcArmorQuality.Load();
    dbcArmorShield.Load();
    dbcArmorTotal.Load();
    dbcDamageAmmo.Load();
    dbcDamageOneHand.Load();
    dbcDamageOneHandCaster.Load();
    dbcDamageRanged.Load();
    dbcDamageThrown.Load();
    dbcDamageTwoHand.Load();
    dbcDamageTwoHandCaster.Load();
    dbcDamageDamageWand.Load();

    uint8 i = 0;
    ItemArmorQuality *ArmorQ = NULL; ItemArmorShield *ArmorS = NULL;
    ItemArmorTotal *ArmorT = NULL; ArmorLocationEntry *ArmorE = NULL;
    ItemDamageEntry *Damage = NULL;
    for(auto itr = m_itemPrototypeContainer.begin(); itr != m_itemPrototypeContainer.end(); ++itr)
    {
        proto = itr->second;
        uint32 Quality = proto->Quality;
        if(Quality >= ITEM_QUALITY_DBC_MAX)
            continue;

        ArmorQ = NULL, ArmorS = NULL, ArmorT = NULL, ArmorE = NULL, Damage = NULL;
        if(proto->Class == ITEM_CLASS_WEAPON)
        {
            switch(proto->SubClass)
            {
            case ITEM_SUBCLASS_WEAPON_AXE:
            case ITEM_SUBCLASS_WEAPON_MACE:
            case ITEM_SUBCLASS_WEAPON_SWORD:
            case ITEM_SUBCLASS_WEAPON_DAGGER:
            case ITEM_SUBCLASS_WEAPON_THROWN:
                proto->weaponType = PROTO_WEAPON_TYPE_ONE_HANDER;
                if(proto->FlagsExtra & 0x200)
                    proto->weaponType += 2;
                break;
            case ITEM_SUBCLASS_WEAPON_TWOHAND_AXE:
            case ITEM_SUBCLASS_WEAPON_TWOHAND_MACE:
            case ITEM_SUBCLASS_WEAPON_POLEARM:
            case ITEM_SUBCLASS_WEAPON_TWOHAND_SWORD:
            case ITEM_SUBCLASS_WEAPON_STAFF:
            case ITEM_SUBCLASS_WEAPON_FISHING_POLE:
                proto->weaponType = PROTO_WEAPON_TYPE_TWO_HANDER;
                if(proto->FlagsExtra & 0x200)
                    proto->weaponType += 2;
                break;

            case ITEM_SUBCLASS_WEAPON_BOW:
            case ITEM_SUBCLASS_WEAPON_GUN:
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                proto->weaponType = PROTO_WEAPON_TYPE_RANGED;
                if(proto->FlagsExtra & 0x200)
                    proto->weaponType++;
                break;
            case ITEM_SUBCLASS_WEAPON_WAND:
                proto->weaponType = PROTO_WEAPON_TYPE_RANGED_CASTER;
                break;
            case ITEM_SUBCLASS_WEAPON_FIST_WEAPON:
                if(proto->FlagsExtra & 0x200)
                    proto->weaponType = PROTO_WEAPON_TYPE_RANGED_CASTER;
                break;
            }

            proto->Durability = CalcWeaponDurability(proto->SubClass, proto->Quality, proto->ItemLevel);
            switch(proto->InventoryType)
            {
            case INVTYPE_WEAPON:
            case INVTYPE_WEAPONMAINHAND:
            case INVTYPE_WEAPONOFFHAND:
                {
                    if (proto->FlagsExtra & 0x200)
                        Damage = dbcDamageOneHandCaster.LookupEntry(proto->ItemLevel);
                    else Damage = dbcDamageOneHand.LookupEntry(proto->ItemLevel);
                }break;
            case INVTYPE_AMMO:
                {
                    Damage = dbcDamageAmmo.LookupEntry(proto->ItemLevel);
                }break;
            case INVTYPE_2HWEAPON:
                {
                    if (proto->FlagsExtra & 0x200)
                        Damage = dbcDamageTwoHandCaster.LookupEntry(proto->ItemLevel);
                    else Damage = dbcDamageTwoHand.LookupEntry(proto->ItemLevel);
                }break;
            case INVTYPE_RANGED:
            case INVTYPE_THROWN:
            case INVTYPE_RANGEDRIGHT:
                {
                    switch(proto->SubClass)
                    {
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_GUN:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                        Damage = dbcDamageRanged.LookupEntry(proto->ItemLevel);
                        break;
                    case ITEM_SUBCLASS_WEAPON_THROWN:
                        Damage = dbcDamageThrown.LookupEntry(proto->ItemLevel);
                        break;
                    case ITEM_SUBCLASS_WEAPON_WAND:
                        Damage = dbcDamageDamageWand.LookupEntry(proto->ItemLevel);
                        break;
                    default:
                        break;
                    }
                }break;
            default: break;
            }
        }
        else if(proto->Class == ITEM_CLASS_ARMOR)
        {
            uint32 inventoryType = proto->InventoryType;
            if (proto->InventoryType == INVTYPE_ROBE)
                inventoryType = INVTYPE_CHEST;
            if (proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
                ArmorS = dbcArmorShield.LookupEntry(proto->ItemLevel);
            else if(ArmorE = dbcArmorLocation.LookupEntry(inventoryType))
            {
                if(proto->SubClass && proto->SubClass <= 5)
                {
                    ArmorQ = dbcArmorQuality.LookupEntry(proto->ItemLevel);
                    ArmorT = dbcArmorTotal.LookupEntry(proto->ItemLevel);
                }
            }
            proto->Durability = CalcArmorDurability(inventoryType, proto->Quality, proto->ItemLevel);
        }

        if(Damage)
        {
            // Grab our values and convert to double for accurate precision calculations
            float statScalingFactor = proto->StatScalingFactor/2.f, ADMod = proto->ArmorDamageModifier, mod_dps = Damage->mod_DPS[Quality] + ADMod;
            float calc_dps = mod_dps * 10.f;
            if(proto->ArmorDamageModifier)
            {   // Behold, armor damage modifier has changed our DPS and now it threatens to change our damage range as well!
                float middle = (((float)proto->Delay) * 0.001f * mod_dps);
                float modifier = middle * statScalingFactor, adMod = ADMod * statScalingFactor *2.f;
                proto->minDamage = floor((middle - modifier) + adMod);
                proto->maxDamage = ceil((middle + modifier) - adMod);

                // Cancle these calculations for now
                proto->minDamage = 1; proto->maxDamage = proto->minDamage+1;
            }
            else
            {
                // ceil and then divide by 10 and add a small value to equal out precision errors
                calc_dps = ceil(calc_dps)/10.f + 0.000001f;
                // Middle point is our dps plus our armor damage modifier
                float middle = (((float)proto->Delay) * 0.001f * calc_dps);
                // Max damage should be floor'd to cut off anything above
                proto->maxDamage = ceil(middle * (1.+statScalingFactor) - 0.5f);
                // Min damage should be converted, to avoid 0 values when a fraction exists
                proto->minDamage = floor(middle * (1.-statScalingFactor));
            }
        }

        if(ArmorS)
            proto->Armor = float2int32(float(ArmorS->mod_Resist[Quality]) + proto->ArmorDamageModifier);
        else if(ArmorQ && ArmorT && ArmorE)
            proto->Armor = float2int32((ArmorQ->mod_Resist[Quality] * ArmorT->mod_Resist[proto->SubClass-1] * ArmorE->Value[proto->SubClass-1] + 0.5f) + proto->ArmorDamageModifier);
    }

    // Unload all DBC files
    dbcArmorLocation.Unload();
    dbcArmorQuality.Unload();
    dbcArmorShield.Unload();
    dbcArmorTotal.Unload();
    dbcDamageAmmo.Unload();
    dbcDamageOneHand.Unload();
    dbcDamageOneHandCaster.Unload();
    dbcDamageRanged.Unload();
    dbcDamageThrown.Unload();
    dbcDamageTwoHand.Unload();
    dbcDamageTwoHandCaster.Unload();
    dbcDamageDamageWand.Unload();
}


static float const qualityDurabilityMultipliers[ITEM_QUALITY_DBC_MAX] = { 1.0f, 1.0f, 1.0f, 1.17f, 1.37f, 1.68f, 0.0f };

uint32 CalcWeaponDurability(uint32 subClass, uint32 quality, uint32 itemLevel)
{
    static float const weaponMultipliers[ITEM_SUBCLASS_WEAPON_FISHING_POLE+1] =
    { 0.89f, 1.03f, 0.77f, 0.77f, 0.89f, 1.03f, 1.03f, 0.89f, 1.03f, 0.00f, 1.03f, 0.00f, 0.00f, 0.64f, 0.00f, 0.64f, 0.64f, 0.00f, 0.77f, 0.64f, 0.64f, };
    return 5 * uint32(17.f * qualityDurabilityMultipliers[quality] * weaponMultipliers[subClass] * (itemLevel <= 17 ? (0.966f - (17.f - float(itemLevel)) / 54.0f) : 1.0f) + 0.5f);
}

uint32 CalcArmorDurability(uint32 inventoryType, uint32 quality, uint32 itemLevel)
{
    static float const armorMultipliers[NUM_INVENTORY_TYPES] =
    { 0.00f, 0.59f, 0.00f, 0.59f, 0.00f, 1.00f, 0.35f, 0.75f, 0.49f, 0.35f, 0.35f, 0.00f, 0.00f, 0.00f, 
    1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, };
    return 5 * uint32(23.f * qualityDurabilityMultipliers[quality] * armorMultipliers[inventoryType] * (itemLevel <= 28 ? (0.966f - (28.f - float(itemLevel)) / 54.0f) : 1.0f) + 0.5f);
}

ItemPrototype* ItemManager::LookupEntry(uint32 entry)
{
    iterator itr = itemPrototypeFind(entry);
    if(itr == itemPrototypeEnd())
        return NULL;
    return (*itr)->second;
}
