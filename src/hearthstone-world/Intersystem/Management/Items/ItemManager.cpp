
#include "StdAfx.h"

initialiseSingleton( ItemPrototypeSystem );

void ItemPrototypeSystem::Init()
{
    // It has to be empty for us to fill in db2 data
    if(!ItemPrototypeContainer.empty())
        return;

    sLog.Notice("ItemPrototypeSystem", "Loading %u items!", db2Item.GetNumRows());
    ItemDataEntry *itemData = NULL;
    ItemSparseEntry *sparse = NULL;
    for(ConstructDB2StorageIterator(ItemDataEntry) itr = db2Item.begin(); itr != db2Item.end(); ++itr)
    {
        itemData = (*itr);
        sparse = db2ItemSparse.LookupEntry(itemData->ID);
        if(sparse == NULL)
            continue;

        ItemPrototype* proto = new ItemPrototype();
        // Non dbc data
        proto->minDamage = 0;
        proto->maxDamage = 0;
        proto->Armor = 0;
        // These are set later.
        proto->ItemId = itemData->ID;
        proto->Class = itemData->Class;
        proto->SubClass = itemData->SubClass;
        proto->subClassSound = itemData->SoundOverrideSubclass;
        proto->Name1 = sparse->Name;
        proto->DisplayInfoID = itemData->DisplayId;
        proto->Quality = sparse->Quality;
        proto->Flags = sparse->Flags;
        proto->FlagsExtra = sparse->Flags2;
        proto->BuyPrice = sparse->BuyPrice;
        proto->SellPrice = sparse->SellPrice;
        proto->InventoryType = sparse->InventoryType;
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
        proto->MaxDurability = sparse->MaxDurability;
        proto->ZoneNameID = sparse->Area;
        proto->MapID = sparse->Map;
        proto->BagFamily = sparse->BagFamily;
        proto->SocketBonus = sparse->SocketBonus;
        proto->GemProperties = sparse->GemProperties;
        proto->ArmorDamageModifier = sparse->ArmorDamageModifier;
        proto->Duration = sparse->Duration;
        proto->ItemLimitCategory = sparse->ItemLimitCategory;
        proto->HolidayId = sparse->HolidayId;
        proto->StatScalingFactor = sparse->StatScalingFactor;
        proto->lowercase_name = string(proto->Name1);
        for(uint32 j = 0; j < proto->lowercase_name.length(); ++j)
            proto->lowercase_name[j] = tolower(proto->lowercase_name[j]);
        ItemPrototypeContainer.insert(make_pair(itemData->ID, proto));
    }
    LoadItemOverrides();
}

void ItemPrototypeSystem::LoadItemOverrides()
{
    ItemPrototype* proto = NULL;
    sLog.Notice("ItemPrototypeSystem", "Loading item overrides...");
    QueryResult* result = WorldDatabase.Query("SELECT * FROM item_overrides");
    if(result != NULL && result->GetFieldCount() == 109)
    {
        uint8 overridden = 0x00;
        uint32 entry = 0, field_Count = 0;
        do
        {
            Field *fields = result->Fetch();
            entry = fields[field_Count++].GetUInt32();
            if(ItemPrototypeContainer.find(entry) == ItemPrototypeContainer.end())
            {
                ItemPrototype* proto = new ItemPrototype();
                proto->ItemId = entry;
                proto->Class = 0;
                proto->SubClass = 0;
                proto->subClassSound = 0;
                proto->Name1 = "";
                proto->DisplayInfoID = 0;
                proto->Quality = 0;
                proto->Flags = 0;
                proto->FlagsExtra = 0;
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
                proto->MaxDurability = 0;
                proto->ZoneNameID = 0;
                proto->MapID = 0;
                proto->BagFamily = 0;
                proto->SocketBonus = 0;
                proto->GemProperties = 0;
                proto->ArmorDamageModifier = 0;
                proto->Duration = 0;
                proto->ItemLimitCategory = 0;
                proto->HolidayId = 0;
                proto->StatScalingFactor = 0;
                ItemPrototypeContainer.insert(make_pair(entry, proto));
                overridden = 0x01 | 0x02;
            }

            proto = ItemPrototypeContainer.at(entry);
            if(proto->Class != fields[field_Count].GetUInt32())
            {
                proto->Class = fields[field_Count].GetUInt32();
                overridden |= 0x01;
            }field_Count++;
            if(proto->SubClass != fields[field_Count].GetUInt32())
            {
                proto->SubClass = fields[field_Count].GetUInt32();
                overridden |= 0x01;
            }field_Count++;
            if(proto->subClassSound != fields[field_Count].GetInt32())
            {
                proto->subClassSound = fields[field_Count].GetInt32();
                overridden |= 0x01;
            }field_Count++;
            if(strcmp(proto->Name1, fields[field_Count].GetString()))
            {
                proto->Name1 = strdup(fields[field_Count].GetString());
                overridden |= 0x02;

                proto->lowercase_name = string(proto->Name1);
                for(uint32 j = 0; j < proto->lowercase_name.length(); ++j)
                    proto->lowercase_name[j] = tolower(proto->lowercase_name[j]);
            }field_Count++;
            if(proto->DisplayInfoID != fields[field_Count].GetUInt32())
            {
                proto->DisplayInfoID = fields[field_Count].GetUInt32();
                overridden |= 0x01;
            }field_Count++;
            if(proto->Quality != fields[field_Count].GetUInt32())
            {
                proto->Quality = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->Flags != fields[field_Count].GetUInt32())
            {
                proto->Flags = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->FlagsExtra != fields[field_Count].GetUInt32())
            {
                proto->FlagsExtra = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->BuyPrice != fields[field_Count].GetUInt32())
            {
                proto->BuyPrice = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->SellPrice != fields[field_Count].GetUInt32())
            {
                proto->SellPrice = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->InventoryType != fields[field_Count].GetUInt32())
            {
                proto->InventoryType = fields[field_Count].GetUInt32();
                overridden |= 0x01;
            }field_Count++;
            if(proto->AllowableClass != fields[field_Count].GetUInt32())
            {
                proto->AllowableClass = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->AllowableRace != fields[field_Count].GetUInt32())
            {
                proto->AllowableRace = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->ItemLevel != fields[field_Count].GetUInt32())
            {
                proto->ItemLevel = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->RequiredLevel != fields[field_Count].GetUInt32())
            {
                proto->RequiredLevel = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->RequiredSkill != fields[field_Count].GetUInt32())
            {
                proto->RequiredSkill = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->RequiredSkillRank != fields[field_Count].GetUInt32())
            {
                proto->RequiredSkillRank = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->RequiredSpell != fields[field_Count].GetUInt32())
            {
                proto->RequiredSpell = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->RequiredPlayerRank1 != fields[field_Count].GetUInt32())
            {
                proto->RequiredPlayerRank1 = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->RequiredPlayerRank2 != fields[field_Count].GetUInt32())
            {
                proto->RequiredPlayerRank2 = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->RequiredFaction != fields[field_Count].GetUInt32())
            {
                proto->RequiredFaction = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->RequiredFactionStanding != fields[field_Count].GetUInt32())
            {
                proto->RequiredFactionStanding = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->MaxCount != fields[field_Count].GetUInt32())
            {
                proto->MaxCount = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->Unique != fields[field_Count].GetUInt32())
            {
                proto->Unique = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->ContainerSlots != fields[field_Count].GetUInt32())
            {
                proto->ContainerSlots = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            for(uint8 i = 0; i < 10; i++)
            {
                if(proto->Stats[i].Type != fields[field_Count].GetUInt32())
                {
                    proto->Stats[i].Type = fields[field_Count].GetUInt32();
                    overridden |= 0x02;
                }field_Count++;
                if(proto->Stats[i].Value != fields[field_Count].GetInt32())
                {
                    proto->Stats[i].Value = fields[field_Count].GetInt32();
                    overridden |= 0x02;
                }field_Count++;
            }
            if(proto->DamageType != fields[field_Count].GetUInt32())
            {
                proto->DamageType = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->Delay != fields[field_Count].GetUInt32())
            {
                proto->Delay = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->Range != fields[field_Count].GetFloat())
            {
                proto->Range = fields[field_Count].GetFloat();
                overridden |= 0x02;
            }field_Count++;
            for(uint8 i = 0; i < 5; i++)
            {
                if(proto->Spells[i].Id != fields[field_Count].GetUInt32())
                {
                    proto->Spells[i].Id = fields[field_Count].GetUInt32();
                    overridden |= 0x02;
                }field_Count++;
                if(proto->Spells[i].Trigger != fields[field_Count].GetUInt32())
                {
                    proto->Spells[i].Trigger = fields[field_Count].GetUInt32();
                    overridden |= 0x02;
                }field_Count++;
                if(proto->Spells[i].Charges != fields[field_Count].GetInt32())
                {
                    proto->Spells[i].Charges = fields[field_Count].GetInt32();
                    overridden |= 0x02;
                }field_Count++;
                if(proto->Spells[i].Cooldown != fields[field_Count].GetInt32())
                {
                    proto->Spells[i].Cooldown = fields[field_Count].GetInt32();
                    overridden |= 0x02;
                }field_Count++;
                if(proto->Spells[i].Category != fields[field_Count].GetUInt32())
                {
                    proto->Spells[i].Category = fields[field_Count].GetUInt32();
                    overridden |= 0x02;
                }field_Count++;
                if(proto->Spells[i].CategoryCooldown != fields[field_Count].GetInt32())
                {
                    proto->Spells[i].CategoryCooldown = fields[field_Count].GetInt32();
                    overridden |= 0x02;
                }field_Count++;
            }
            if(proto->Bonding != fields[field_Count].GetUInt32())
            {
                proto->Bonding = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(strcmp(proto->Description, fields[field_Count].GetString()))
            {
                proto->Description = strdup(fields[field_Count].GetString());
                overridden |= 0x02;
            }field_Count++;
            if(proto->PageId != fields[field_Count].GetUInt32())
            {
                proto->PageId = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->PageLanguage != fields[field_Count].GetUInt32())
            {
                proto->PageLanguage = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->PageMaterial != fields[field_Count].GetUInt32())
            {
                proto->PageMaterial = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->QuestId != fields[field_Count].GetUInt32())
            {
                proto->QuestId = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->LockId != fields[field_Count].GetUInt32())
            {
                proto->LockId = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->LockMaterial != fields[field_Count].GetUInt32())
            {
                proto->LockMaterial = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->SheathID != fields[field_Count].GetUInt32())
            {
                proto->SheathID = fields[field_Count].GetUInt32();
                overridden |= 0x01;
            }field_Count++;
            if(proto->RandomPropId != fields[field_Count].GetUInt32())
            {
                proto->RandomPropId = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->RandomSuffixId != fields[field_Count].GetUInt32())
            {
                proto->RandomSuffixId = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->ItemSet != fields[field_Count].GetUInt32())
            {
                proto->ItemSet = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->MaxDurability != fields[field_Count].GetUInt32())
            {
                proto->MaxDurability = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->ZoneNameID != fields[field_Count].GetUInt32())
            {
                proto->ZoneNameID = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->MapID != fields[field_Count].GetUInt32())
            {
                proto->MapID = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->BagFamily != fields[field_Count].GetUInt32())
            {
                proto->BagFamily = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->SocketBonus != fields[field_Count].GetUInt32())
            {
                proto->SocketBonus = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->GemProperties != fields[field_Count].GetUInt32())
            {
                proto->GemProperties = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->ArmorDamageModifier != fields[field_Count].GetFloat())
            {
                proto->ArmorDamageModifier = fields[field_Count].GetFloat();
                overridden |= 0x02;
            }field_Count++;
            if(proto->Duration != fields[field_Count].GetUInt32())
            {
                proto->Duration = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->ItemLimitCategory != fields[field_Count].GetUInt32())
            {
                proto->ItemLimitCategory = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->HolidayId != fields[field_Count].GetUInt32())
            {
                proto->HolidayId = fields[field_Count].GetUInt32();
                overridden |= 0x02;
            }field_Count++;
            if(proto->StatScalingFactor != fields[field_Count].GetFloat())
            {
                proto->StatScalingFactor = fields[field_Count].GetFloat();
                overridden |= 0x02;
            }field_Count++;
            if(overridden > 0)
                Overridden.insert(make_pair(entry, overridden));

            proto = NULL;
            overridden = 0;
            entry = field_Count = 0;
        } while( result->NextRow() );
        sLog.Notice("ItemPrototypeSystem", "%u item overrides loaded!", Overridden.size());
    }
    else
    {
        // Declare some errors?
    }

    sLog.Notice("ItemPrototypeSystem", "Setting static dbc data...");
    uint8 i = 0;
    ItemArmorQuality *ArmorQ = NULL; ItemArmorShield *ArmorS = NULL;
    ItemArmorTotal *ArmorT = NULL; ItemDamageEntry *Damage = NULL;
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        proto = (*itr)->second;
        uint32 Quality = proto->Quality;
        if(Quality >= ITEM_QUALITY_DBC_MAX)
            continue;

        ArmorQ = NULL;
        ArmorS = NULL;
        ArmorT = NULL;
        Damage = NULL;
        if(proto->Class == ITEM_CLASS_WEAPON)
        {
            switch(proto->InventoryType)
            {
            case INVTYPE_WEAPON:
            case INVTYPE_WEAPONMAINHAND:
            case INVTYPE_WEAPONOFFHAND:
                {
                    if (proto->FlagsExtra & 0x200)
                        Damage = dbcDamageOneHandCaster.LookupEntry(proto->ItemLevel);
                    else
                        Damage = dbcDamageOneHand.LookupEntry(proto->ItemLevel);
                }break;
            case INVTYPE_AMMO:
                {
                    Damage = dbcDamageAmmo.LookupEntry(proto->ItemLevel);
                }break;
            case INVTYPE_2HWEAPON:
                {
                    if (proto->FlagsExtra & 0x200)
                        Damage = dbcDamageTwoHandCaster.LookupEntry(proto->ItemLevel);
                    else
                        Damage = dbcDamageTwoHand.LookupEntry(proto->ItemLevel);
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
            default:
                break;
            }
        }
        if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
            ArmorS = dbcArmorShield.LookupEntry(proto->ItemLevel);
        else if(!(proto->InventoryType != INVTYPE_HEAD && proto->InventoryType != INVTYPE_CHEST && proto->InventoryType != INVTYPE_SHOULDERS
                && proto->InventoryType != INVTYPE_LEGS && proto->InventoryType != INVTYPE_FEET && proto->InventoryType != INVTYPE_WRISTS
                && proto->InventoryType != INVTYPE_HANDS && proto->InventoryType != INVTYPE_WAIST && proto->InventoryType != INVTYPE_CLOAK
                && proto->InventoryType != INVTYPE_ROBE))
        {
            ArmorQ = dbcArmorQuality.LookupEntry(proto->ItemLevel);
            ArmorT = dbcArmorTotal.LookupEntry(proto->ItemLevel);
        }

        if(Damage)
        {
            float dps = Damage->mod_DPS[Quality];
            float avgDamage = dps * proto->Delay * 0.001f;
            proto->minDamage = (proto->StatScalingFactor * -0.5f + 1.0f) * avgDamage;
            proto->maxDamage = floor(float(avgDamage * (proto->StatScalingFactor * 0.5f + 1.0f) + 0.5f));
        }

        if(ArmorS)
            proto->Armor = ArmorS->mod_Resist[Quality];
        else if(ArmorQ && proto->SubClass <= 4)
        {
            uint8 armorClass = 0;
            if(proto->SubClass)
                armorClass = proto->SubClass-1;
            if(ArmorLocationEntry* entry = dbcArmorLocation.LookupEntry(proto->InventoryType))
                proto->Armor = uint32(ArmorQ->mod_Resist[Quality] * ArmorT->mod_Resist[armorClass] * entry->Value[armorClass] + 0.5f);
        }
    }
}

ItemPrototype* ItemPrototypeSystem::LookupEntry(uint32 entry)
{
    iterator itr = find(entry);
    if(itr == end())
        return NULL;
    return (*itr)->second;
}
