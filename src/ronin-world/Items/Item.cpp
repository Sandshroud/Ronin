/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Item::Item(ItemData *data) : _itemData(data), m_itemProto(data->proto), m_isContainer(m_itemProto->InventoryType == INVTYPE_BAG),
    m_textid(_itemData->itemTextID), m_wrappedItemId(0), currentSlot(data->inventorySlot),
    Object(data->itemGuid, (IsContainer() ? CONTAINER_END : ITEM_END))
{
    SetTypeFlags(TYPEMASK_TYPE_ITEM);
    SetUInt32Value(OBJECT_FIELD_ENTRY, data->itemGuid.getEntry());
    SetUInt64Value(ITEM_FIELD_CONTAINED, _itemData->itemContainer);
    SetUInt64Value(ITEM_FIELD_CREATOR, _itemData->itemCreator);
    SetUInt32Value(ITEM_FIELD_STACK_COUNT, _itemData->itemStackCount);
    SetUInt32Value(ITEM_FIELD_FLAGS, _itemData->itemFlags);
    SetUInt32Value(ITEM_FIELD_PROPERTY_SEED, _itemData->itemRandomSeed);
    SetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID, _itemData->itemRandomProperty);
    SetUInt32Value(ITEM_FIELD_DURABILITY, _itemData->itemDurability);
    SetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME, _itemData->itemPlayedTime);
    if(_itemData->giftData)
    {
        SetUInt64Value(ITEM_FIELD_GIFTCREATOR, _itemData->giftData->giftCreatorGuid);
        m_wrappedItemId = _itemData->giftData->giftItemId;
    }

    for(uint8 i = 0; i < 5; i++)
    {
        if(_itemData->proto->Spells[i].Charges == 0)
            continue;
        if(_itemData->proto->Spells[i].Charges == -1)
        {
            SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+i, 0xFFFFFFFF);
            continue;
        }
        SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+i, _itemData->itemSpellCharges);
    }

    for(uint8 i = 0; i < 10; i++)
    {
        if(_itemData->enchantData[i])
        {
            SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA+(i*3), _itemData->enchantData[i]->enchantId);
            SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA+1+(i*3), _itemData->enchantData[i]->CalcTimeLeft());
            SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA+2+(i*3), _itemData->enchantData[i]->enchantCharges);
        }
    }

    if(_itemData->itemRandomProperty)
    {
        int32 randomPropertyId = _itemData->itemRandomProperty;
        if(randomPropertyId < 0)
        {
            if(ItemRandomSuffixEntry *randomSuffix = dbcItemRandomSuffix.LookupEntry(abs(randomPropertyId)))
            {
                for(uint8 i = 0; i < 3; i++)
                {
                    if(randomSuffix->enchantments[i] == 0)
                        continue;
                    SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA+((PROP_ENCHANTMENT_SLOT_0+i)*3), randomSuffix->enchantments[i]);
                }
            }
        }
        else if(ItemRandomPropertiesEntry *randomProperties = dbcItemRandomProperties.LookupEntry(randomPropertyId))
        {
            for(uint8 i = 0; i < 3; i++)
            {
                if(randomProperties->enchant_id[i] == 0)
                    continue;
                SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA+((PROP_ENCHANTMENT_SLOT_2+i)*3), randomProperties->enchant_id[i]);
            }
        }
    }

    if(m_isContainer)
    {
        _container = new Item::Container();
        SetUInt32Value(CONTAINER_FIELD_NUM_SLOTS, (_container->numSlots = m_itemProto->ContainerSlots));
        for(std::map<uint8, WoWGuid>::iterator itr = data->containerData->m_items.begin(); itr != data->containerData->m_items.end(); itr++)
            AddItem(itr->first, itr->second);
    }

    Init();
}

Item::~Item()
{

}

bool CalculateEnchantDuration(time_t expirationTime, uint32 &duration);

void Item::Init()
{
    Object::Init();
}

bool CalculateEnchantDuration(time_t expirationTime, uint32 &duration)
{
    if(expirationTime < 172800)
        duration = expirationTime;
    else if(expirationTime <= UNIXTIME+5)
        return false;
    else duration = (expirationTime-UNIXTIME);
    duration *= 1000;
    return true;
}

void Item::Destruct()
{
    m_owner = NULL;
    Object::Destruct();
}

void Item::AddToWorld()
{
    Object::AddToWorld();
    if(m_owner && m_owner->IsInWorld())
    {
        ByteBuffer *buff = &m_owner->GetMapMgr()->m_createBuffer;
        if(uint32 count = BuildCreateUpdateBlockForPlayer(buff, m_owner))
            m_owner->PushUpdateBlock(buff, count);
    }
}

void Item::SetOwner( Player* owner )
{
    if( owner != NULL )
        SetUInt64Value( ITEM_FIELD_OWNER, owner->GetGUID() );
    else SetUInt64Value( ITEM_FIELD_OWNER, 0 );
    m_owner = owner;
}

bool Item::IsEligibleForRefund()
{
    return false;
}

void Item::SetTextID(uint32 newTextId)
{
    m_textid = newTextId;
    _itemData->itemTextID = newTextId;
    QueueItemDataUpdate(ITEMDATA_FIELD_ITEMTEXTID, newTextId);
}

void Item::SetItemSlot(uint8 slot)
{
    INVSLOT_SET_ITEMSLOT(currentSlot, slot);
    _itemData->inventorySlot = currentSlot;
    QueueItemDataUpdate(ITEMDATA_FIELD_CONTAINER_SLOT, uint32(currentSlot));
}

void Item::SetContainerSlot(uint16 containerSlot)
{
    currentSlot = containerSlot;
    _itemData->inventorySlot = containerSlot;
    QueueItemDataUpdate(ITEMDATA_FIELD_CONTAINER_SLOT, uint32(currentSlot));
}

void Item::SetContainerData(WoWGuid containerGuid, uint16 containerSlot)
{
    _itemData->itemContainer = containerGuid;
    SetUInt64Value(ITEM_FIELD_CONTAINED, containerGuid);
    QueueItemDataUpdate(ITEMDATA_FIELD_CONTAINER_GUID, containerGuid);
    SetContainerSlot(containerSlot);
}

void Item::SetCreatorGuid(WoWGuid creatorGuid)
{
    _itemData->itemCreator = creatorGuid;
    SetUInt64Value(ITEM_FIELD_CREATOR, creatorGuid);
    QueueItemDataUpdate(ITEMDATA_FIELD_CREATOR_GUID, creatorGuid);
}

void Item::SetStackSize(uint32 newStackSize)
{
    if(newStackSize == 0)
    {
        QueueItemDeletion(ITEM_DELETION_DELETED);
        RemoveFromWorld(true);
        Destruct();
        return;
    }

    _itemData->itemStackCount = newStackSize;
    SetUInt32Value(ITEM_FIELD_STACK_COUNT, newStackSize);
    QueueItemDataUpdate(ITEMDATA_FIELD_ITEMSTACKCOUNT, newStackSize);
}

void Item::ModStackSize(int32 &stackSizeMod)
{
    if(stackSizeMod == 0)
        return;

    uint32 currentStack = GetStackSize();
    if(stackSizeMod > 0)
    {
        ModStackSize(stackSizeMod);
        return;
    }
    else
    {
        uint32 modSize = abs(stackSizeMod);
        if(modSize > currentStack)
        {
            stackSizeMod += currentStack;
            currentStack = 0;
        }
        else
        {
            currentStack -= modSize;
            stackSizeMod = 0;
        }
    }

    _itemData->itemStackCount = currentStack;
    SetUInt32Value(ITEM_FIELD_STACK_COUNT, currentStack);
    QueueItemDataUpdate(ITEMDATA_FIELD_ITEMSTACKCOUNT, currentStack);
}

void Item::ModStackSize(uint32 &stackSizeMod)
{
    uint32 currentStack = GetStackSize();
    if(m_itemProto->MaxCount)
    {
        if(currentStack == m_itemProto->MaxCount)
            return;

        uint32 stackToAdd = m_itemProto->MaxCount-currentStack;
        currentStack += stackToAdd;
        stackSizeMod -= stackToAdd;
    }
    else
    {
        currentStack += stackSizeMod;
        stackSizeMod = 0;
    }

    _itemData->itemStackCount = currentStack;
    SetUInt32Value(ITEM_FIELD_STACK_COUNT, currentStack);
    QueueItemDataUpdate(ITEMDATA_FIELD_ITEMSTACKCOUNT, currentStack);
}

void Item::ModifyStackSize(int32 stackSizeMod)
{
    if(stackSizeMod == 0)
        return;

    uint32 currentStack = GetStackSize();
    if(stackSizeMod > 0)
    {
        if(m_itemProto->MaxCount)
        {
            if(currentStack == m_itemProto->MaxCount)
                return;

            uint32 stackToAdd = m_itemProto->MaxCount-currentStack;
            currentStack += stackToAdd;
        } else currentStack += stackSizeMod;
    }
    else
    {
        uint32 modSize = abs(stackSizeMod);
        if(modSize > currentStack)
            currentStack = 0;
        else currentStack -= modSize;
    }

    _itemData->itemStackCount = currentStack;
    SetUInt32Value(ITEM_FIELD_STACK_COUNT, currentStack);
    QueueItemDataUpdate(ITEMDATA_FIELD_ITEMSTACKCOUNT, currentStack);
}

void Item::AddItemFlag(uint32 itemFlag)
{
    _itemData->itemFlags |= itemFlag;
    SetUInt32Value(ITEM_FIELD_FLAGS, _itemData->itemFlags);
    QueueItemDataUpdate(ITEMDATA_FIELD_ITEMFLAGS, _itemData->itemFlags);
}

void Item::RemoveItemFlag(uint32 itemFlag)
{
    if((_itemData->itemFlags & itemFlag) == 0)
        return;

    _itemData->itemFlags &= ~itemFlag;
    SetUInt32Value(ITEM_FIELD_FLAGS, _itemData->itemFlags);
    QueueItemDataUpdate(ITEMDATA_FIELD_ITEMFLAGS, _itemData->itemFlags);
}

void Item::SetRandomPropData(uint32 randomProp, uint32 randomSeed)
{

}

void Item::SetDurability(uint32 newDurability)
{
    _itemData->itemDurability = newDurability;
    SetUInt32Value(ITEM_FIELD_DURABILITY, newDurability);
    QueueItemDataUpdate(ITEMDATA_FIELD_ITEM_DURABILITY, newDurability);
}

void Item::ModDurability(bool apply, float modPct)
{
    uint32 maxDurability = m_itemProto->MaxDurability;
    if(maxDurability == 0)
        return;
    uint32 currentDurability = GetUInt32Value(ITEM_FIELD_DURABILITY);
    uint32 mod = (uint32)floor(float(maxDurability) * modPct);
    if(apply)
    {
        if(currentDurability + mod >= maxDurability)
            currentDurability = maxDurability;
        else currentDurability += mod;
    }
    else
    {
        if(mod >= currentDurability)
            currentDurability = 0;
        else currentDurability -= mod;
    }

    SetDurability(currentDurability);
}

void Item::UpdatePlayedTime()
{

}

void Item::ModPlayedTime(uint32 timetoadd)
{

}

void Item::QueueItemDeletion(ItemDeletionReason reason)
{
    sItemMgr.DeleteItemFromDatabase(GetGUID(), reason);
    sItemMgr.DeleteItemData(GetGUID(), true);
}

void Item::QueueItemDataUpdate(ItemDataFields fieldType, uint32 fieldValue)
{
    std::stringstream ss;
    ss << "UPDATE `item_data` SET '" << itemdata_fieldNames[fieldType] << "' = '";
    ss << fieldValue << "' WHERE itemguid = '";
    ss << uint64(GetGUID()) << "';";
    if(IsInWorld() && m_owner && m_owner->IsInWorld())
    {}//m_owner->AddItemDataUpdate(GetGUID(), fieldType, ss.str().c_str());
    else CharacterDatabase.Execute(ss.str().c_str());
}

void Item::QueueItemDataUpdate(ItemDataFields fieldType, uint64 fieldValue)
{
    std::stringstream ss;
    ss << "UPDATE `item_data` SET '" << itemdata_fieldNames[fieldType] << "' = '";
    ss << fieldValue << "' WHERE itemguid = '";
    ss << uint64(GetGUID()) << "';";
    if(IsInWorld() && m_owner && m_owner->IsInWorld())
    {}//m_owner->AddItemDataUpdate(GetGUID(), fieldType, ss.str().c_str());
    else CharacterDatabase.Execute(ss.str().c_str());
}

//////////////////////////////////////////////////////////////////////////
// Item Links
//////////////////////////////////////////////////////////////////////////
static const char *g_itemQualityColours[9] = {
    "|cff9d9d9d",       // Grey
    "|cffffffff",       // White
    "|cff1eff00",       // Green
    "|cff0070dd",       // Blue
    "|cffa335ee",       // Purple
    "|cffff8000",       // Orange
    "|cffe6cc80",       // Artifact
    "|cffe5cc80",       // Heirloom
    "|cff00ffff"        // Turquoise
};

std::string ItemPrototype::ConstructItemLink(uint32 random_prop, uint32 random_suffix, uint32 stack)
{
    if( Quality > 15 )
        return "INVALID_ITEM";

    char buf[1000], sbuf[50], rptxt[100], rstxt[100];

    // stack text
    if( stack > 1 )
        snprintf(sbuf, 50, "x%u", stack);
    else sbuf[0] = 0;

    // null 'em
    rptxt[0] = rstxt[0] = 0;

    // lookup properties
    if( random_prop != 0 )
    {
        ItemRandomPropertiesEntry *rp = dbcItemRandomProperties.LookupEntry(random_prop);
        if( rp != NULL )
            snprintf(rptxt, 100, " %s", rp->rpname);
    }

    // suffix
    if( random_suffix != 0 )
    {
        ItemRandomSuffixEntry *rs = dbcItemRandomSuffix.LookupEntry(random_suffix);
        if( rs != NULL )
            snprintf(rstxt, 100, " %s", rs->name);
    }

    // construct full link
    snprintf(buf, 1000, "%s|Hitem:%u:0:0:0:0:0:%d:0|h[%s%s%s]%s|h|r", g_itemQualityColours[(Quality < 9 ? Quality : 9)],
        ItemId, /* suffix/prop */ random_suffix ? (-(int32)random_suffix) : random_prop, Name1, rstxt, rptxt, sbuf);

    return std::string(buf);
}

bool ItemPrototype::ValidateItemLink(const char *szLink)
{
    return true;
}

bool ItemPrototype::ValidateItemSpell(uint32 SpellId)
{
    for(uint8 i = 0; i < 5; i++)
        if(Spells[i].Id == SpellId)
            return true;
    return false;
}
