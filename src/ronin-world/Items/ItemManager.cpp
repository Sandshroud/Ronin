
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
        Standing plrstanding = player->GetStandingRank( vendor->GetFactionID() );
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

        ItemPrototype* proto = new ItemPrototype();
        // Non dbc data
        proto->minDamage = 0;
        proto->maxDamage = 0;
        proto->Armor = 0;
        proto->Durability = 0;
        // These are set later.
        proto->ItemId = itemData->ID;
        proto->Class = itemData->Class;
        proto->SubClass = itemData->SubClass;
        proto->subClassSound = itemData->SoundOverrideSubclass;
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
            if(i > 5)
                continue;
            proto->Spells[i].Id = sparse->SpellId[i];
            proto->Spells[i].Trigger = sparse->SpellTrigger[i];
            proto->Spells[i].Charges = sparse->SpellCharges[i];
            proto->Spells[i].Cooldown = sparse->SpellCooldown[i];
            proto->Spells[i].Category = sparse->SpellCategory[i];
            proto->Spells[i].CategoryCooldown = sparse->SpellCategoryCooldown[i];
            if(i > 3)
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
        if(fieldCount != 112)
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
                    ItemPrototype* proto = new ItemPrototype();
                    proto->ItemId = entry;
                    proto->Class = 0;
                    proto->SubClass = 0;
                    proto->subClassSound = 0;
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
                    proto->minDamage = 0;
                    proto->maxDamage = 0;
                    proto->Armor = 0;
                    proto->Durability = 0;
                    for(uint8 i = 0; i < 10; i++)
                    {
                        proto->Stats[i].Type = 0;
                        proto->Stats[i].Value = 0;
                        if(i > 5)
                            continue;
                        proto->Spells[i].Id = 0;
                        proto->Spells[i].Trigger = 0;
                        proto->Spells[i].Charges = 0;
                        proto->Spells[i].Cooldown = 0;
                        proto->Spells[i].Category = 0;
                        proto->Spells[i].CategoryCooldown = 0;
                        if(i > 3)
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
            float avgDamage = Damage->mod_DPS[Quality] * proto->Delay * 0.001f;
            if(proto->ArmorDamageModifier) avgDamage *= proto->ArmorDamageModifier;
            proto->minDamage = (proto->StatScalingFactor * -0.5f + 1.0f) * avgDamage;
            proto->maxDamage = floor(float(avgDamage * (proto->StatScalingFactor * 0.5f + 1.0f) + 0.5f));
        }

        if(ArmorS)
            proto->Armor = float2int32((proto->ArmorDamageModifier ? proto->ArmorDamageModifier : 1.f)*float(ArmorS->mod_Resist[Quality]));
        else if(ArmorQ && ArmorT && ArmorE)
            proto->Armor = float2int32((proto->ArmorDamageModifier ? proto->ArmorDamageModifier : 1.f)*(ArmorQ->mod_Resist[Quality] * ArmorT->mod_Resist[proto->SubClass-1] * ArmorE->Value[proto->SubClass-1] + 0.5f));
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
