/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Item::Item(ItemData *data, uint32 fieldCount) : _itemData(data), Object(data->itemGuid, fieldCount), m_locked(false), m_textid(0), m_wrappedItemGuid(0)
{
    SetTypeFlags(TYPEMASK_TYPE_ITEM);
    SetUInt32Value(OBJECT_FIELD_ENTRY, data->itemGuid.getEntry());
    m_itemProto = sItemMgr.LookupEntry(GetEntry());
    ASSERT(m_itemProto);
}

Item::~Item()
{

}

bool CalculateEnchantDuration(time_t expirationTime, uint32 &duration);

void Item::Initialize(Player *owner)
{
    if(m_owner = owner)
        SetUInt64Value(ITEM_FIELD_OWNER, owner->GetGUID());

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
        m_wrappedItemGuid = _itemData->giftData->giftItemGuid;
        SetUInt64Value(ITEM_FIELD_GIFTCREATOR, _itemData->giftData->giftCreator);
    }

    if(_itemData->chargeData)
    {
        for(uint8 i = 0; i < 5; i++)
        {
            if(uint32 charges = _itemData->chargeData->spellCharges[i])
                SetUInt32Value(ITEM_FIELD_SPELL_CHARGES+i, charges);
        }
    }

    for(uint8 i = 0; i < 10; i++)
    {
        if(_itemData->enchantData[i])
        {
            uint32 duration;
            if(!CalculateEnchantDuration(_itemData->enchantData[i]->expirationTime, duration))
                continue;
            SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA+(i*3), _itemData->enchantData[i]->enchantId);
            SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA+1+(i*3), duration);
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

    if( m_itemProto->LockId > 1 )
        m_locked = (GetUInt32Value(ITEM_FIELD_FLAGS) & 0x04) == 0;
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

}

void Item::SetWrappedItemGuid(WoWGuid guid)
{
    m_wrappedItemGuid = guid;

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
