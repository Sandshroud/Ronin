/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Item::Item(ItemPrototype *proto, uint32 counter, uint32 fieldcount) : Object(MAKE_NEW_GUID(counter, proto->ItemId, HIGHGUID_TYPE_ITEM), fieldcount), m_owner(0), m_proto(proto), m_textId(0), locked(false), m_isDirty(false), m_deleted(false)
{
    SetTypeFlags(TYPEMASK_TYPE_ITEM);
    m_objType = TYPEID_ITEM;

    SetUInt32Value(OBJECT_FIELD_ENTRY, proto->ItemId);

    // Enchantment slot reserve, TODO: limit based on item proto
    m_enchantments.resize(MAX_ENCHANTMENT_SLOT);
}

Item::~Item()
{

}

void Item::Init()
{
    Object::Init();

    // Set our defaults, these will be overwritten by loading
    SetUInt32Value(ITEM_FIELD_STACK_COUNT, 0x00000001);
    SetUInt32Value(ITEM_FIELD_DURABILITY, m_proto->Durability);
    SetUInt32Value(ITEM_FIELD_MAXDURABILITY, m_proto->Durability);
}

void Item::Destruct()
{
    if( IsInWorld() )
        RemoveFromWorld();

    Object::Destruct();
}

void Item::OnFieldUpdated(uint16 field)
{
    if(m_owner && IsInWorld())
        m_owner->ItemFieldUpdated(this);
}

void Item::SetItemInWorld(bool res)
{
    Object::SetInWorld(res);
    if(res == false || m_owner == NULL)
        return;

    // Update our item owned criteria when adding items to world
    AchieveMgr.UpdateCriteriaValue(m_owner, ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM, m_owner->GetInventory()->GetItemCount(GetEntry()), GetEntry(), 0, !m_owner->IsInWorld());
}

void Item::LoadFromDB(Field* fields)
{
    SetUInt32Value(OBJECT_FIELD_ENTRY, fields[2].GetUInt32());
    m_uint32Values[ITEM_FIELD_CONTAINED] = fields[3].GetUInt32();
    m_uint32Values[ITEM_FIELD_CREATOR] = fields[4].GetUInt32();

    uint32 count = fields[5].GetUInt32();
    if(m_proto->MaxCount > 0 && count > m_proto->MaxCount)
        count = m_proto->MaxCount;
    SetUInt32Value( ITEM_FIELD_STACK_COUNT, count);
    SetUInt32Value( ITEM_FIELD_FLAGS, fields[6].GetUInt32() );
    SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, fields[7].GetUInt32() );
    SetUInt32Value( ITEM_FIELD_RANDOM_PROPERTIES_ID, fields[8].GetUInt32() );
    SetUInt32Value( ITEM_FIELD_DURABILITY, fields[9].GetUInt32() );
    SetUInt32Value( ITEM_FIELD_MAXDURABILITY, m_proto->Durability );
    SetTextID( fields[10].GetUInt32() );
    SetUInt32Value( ITEM_FIELD_CREATE_PLAYED_TIME, fields[11].GetUInt32() );

    // Again another for that did not indent to make it do anything for more than
    // one iteration x == 0 was the only one executed
    for( uint32 x = 0; x < 5; x++ )
    {
        if( m_proto->Spells[x].Id )
        {
            SetUInt32Value( ITEM_FIELD_SPELL_CHARGES + x , fields[12].GetUInt32() );
            break;
        }
    }
    SetUInt32Value( ITEM_FIELD_GIFTCREATOR, fields[14].GetUInt32() );

    Bind(ITEM_BIND_ON_PICKUP); // Check if we need to bind our shit.

    LoadRandomProperties();

    if(Charter* charter = guildmgr.GetCharterByItemGuid(GetLowGUID()))
    {
        SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA, charter->GetID());

        // Charter stuff
        if(m_uint32Values[OBJECT_FIELD_ENTRY] == ITEM_ENTRY_GUILD_CHARTER)
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( uint32 charterId = m_owner->getPlayerInfo()->charterId[CHARTER_TYPE_GUILD] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_DATA, charterId );
        }

        if( m_uint32Values[OBJECT_FIELD_ENTRY] == ARENA_TEAM_CHARTER_2v2 )
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( uint32 charterId = m_owner->getPlayerInfo()->charterId[CHARTER_TYPE_ARENA_2V2] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_DATA, charterId );
        }

        if( m_uint32Values[OBJECT_FIELD_ENTRY] == ARENA_TEAM_CHARTER_3v3 )
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( uint32 charterId = m_owner->getPlayerInfo()->charterId[CHARTER_TYPE_ARENA_3V3] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_DATA, charterId );
        }

        if( m_uint32Values[OBJECT_FIELD_ENTRY] == ARENA_TEAM_CHARTER_5v5 )
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( uint32 charterId = m_owner->getPlayerInfo()->charterId[CHARTER_TYPE_ARENA_5V5] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_DATA, charterId );
        }
    }
}

void Item::LoadRandomProperties()
{
    if(m_uint32Values[ITEM_FIELD_RANDOM_PROPERTIES_ID] == 0)
        return;

    int32 randomProp = m_uint32Values[ITEM_FIELD_RANDOM_PROPERTIES_ID];
    if( randomProp > 0 )      // Random Property
    {
        // We just need to load the enchantments into the vectors
        if(ItemRandomPropertiesEntry* rp= dbcItemRandomProperties.LookupEntry(randomProp))
            for( int k = 0; k < 3; k++ )
                if( rp->enchant_id[k] != 0 )
                    LoadEnchantment(PROP_ENCHANTMENT_SLOT_2+k, rp->enchant_id[k], 0, 0, 0);
        return;
        // Finished loading random prop enchants into enchantment vector
    }

    // If the value is negative then 
    if(ItemRandomSuffixEntry* rs = dbcItemRandomSuffix.LookupEntry(-randomProp))
        for( uint32 k = 0; k < 3; ++k )
            if( rs->enchantments[k] != 0 )
                LoadEnchantment(PROP_ENCHANTMENT_SLOT_0+k, rs->enchantments[k], rs->prefixes[k], 0, 0);
    // Finished loading random suffix enchants into enchantment vector
}

void Item::SaveToDB( int8 containerslot, uint8 slot, bool firstsave, QueryBuffer* buf )
{
    if( !m_isDirty && !firstsave || m_deleted )
        return;

    std::stringstream ss;

    ss << "REPLACE INTO item_data VALUES(";

    ss << m_uint32Values[ITEM_FIELD_OWNER] << ",";
    ss << m_objGuid.getLow() << ",";
    ss << m_uint32Values[OBJECT_FIELD_ENTRY] << ",";
    ss << m_uint32Values[ITEM_FIELD_CONTAINED] << ",";
    ss << m_uint32Values[ITEM_FIELD_CREATOR] << ",";
    ss << GetUInt32Value(ITEM_FIELD_STACK_COUNT) << ",";
    ss << GetUInt32Value(ITEM_FIELD_FLAGS) << ",";
    ss << GetUInt32Value(ITEM_FIELD_PROPERTY_SEED) << ",";
    ss << GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID) << ",";
    ss << GetUInt32Value(ITEM_FIELD_DURABILITY) << ",";
    ss << GetTextID() << ",";
    ss << GetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME) << ",";
    ss << (int32)GetChargesLeft() << ",";
    ss << uint32(0) << ", " << GetUInt32Value(ITEM_FIELD_GIFTCREATOR) << ")";

    std::stringstream ssench;
    for(uint8 i = PERM_ENCHANTMENT_SLOT; i < MAX_ENCHANTMENT_SLOT; i++)
    {
        if(EnchantmentInstance *enchantInst = GetEnchantment(i))
        {
            if(!ssench.str().empty())
                ssench << ", ";
            ssench << "('" << m_uint32Values[OBJECT_FIELD_GUID] << "', '"
                << uint32(i) << "', '" << enchantInst->EnchantmentId << "', '"
                << uint32(enchantInst->RandomSuffix) << "', '"<< uint32(0) << "', '" << uint32(0) << "')";
        }
    }

    if( firstsave || buf == NULL )
    {
        CharacterDatabase.WaitExecute( ss.str().c_str() );
        CharacterDatabase.WaitExecute("REPLACE INTO character_inventory VALUES(%u, %u, %i, %u);", m_owner->GetLowGUID(), GetLowGUID(), containerslot, slot);
        CharacterDatabase.WaitExecute("DELETE FROM item_enchantments WHERE itemguid = '%u'", m_objGuid.getLow());
        if(ssench.str().length()) CharacterDatabase.WaitExecute("REPLACE INTO item_enchantments VALUES %s", ssench.str().c_str());
    }
    else
    {
        buf->AddQueryStr( ss.str() );
        buf->AddQueryStr(format("REPLACE INTO character_inventory VALUES(%u, %u, %i, %u);", m_owner->GetLowGUID(), GetLowGUID(), containerslot, slot));
        buf->AddQueryStr(format("DELETE FROM item_enchantments WHERE itemguid = '%u'", m_objGuid.getLow()));
        if(ssench.str().length()) buf->AddQueryStr(format("REPLACE INTO item_enchantments VALUES %s", ssench.str().c_str()));
    }

    m_isDirty = false;
}

void Item::DeleteFromDB()
{
    m_deleted = true;
    if( m_proto->ContainerSlots > 0 && GetTypeId() == TYPEID_CONTAINER )
    {
        /* deleting a Container* */
        for( int32 i = 0; i < m_proto->ContainerSlots; i++ )
        {
            if( castPtr<Container>(this)->GetItem( i ) != NULL )
            {
                /* abort the delete */
                return;
            }
        }
    }

    CharacterDatabase.Execute( "DELETE FROM item_data WHERE ownerguid = %u AND itemguid = %u", m_uint32Values[ITEM_FIELD_OWNER], m_uint32Values[OBJECT_FIELD_GUID] );
    if(m_uint32Values[ITEM_FIELD_OWNER])
        CharacterDatabase.Execute( "DELETE FROM character_inventory WHERE guid = %u AND itemguid = %u", m_uint32Values[ITEM_FIELD_OWNER], m_uint32Values[OBJECT_FIELD_GUID] );
    CharacterDatabase.Execute("DELETE FROM item_enchantments WHERE itemguid = '%u'", m_objGuid.getLow());
}

void Item::RemoveFromWorld()
{
    // if we have an owner->send destroy
    if( m_owner != NULL )
    {
        m_owner->ItemDestructed(this);
        DestroyForPlayer( m_owner );
    }
}

void Item::SetOwner( Player* owner )
{
    if( owner != NULL )
        SetUInt64Value( ITEM_FIELD_OWNER, owner->GetGUID() );
    else SetUInt64Value( ITEM_FIELD_OWNER, 0 );

    m_owner = owner;
}

int32 Item::AddEnchantment(uint32 enchantId, uint32 Duration, bool Perm /* = false */, bool apply /* = true */, bool RemoveAtLogout /* = false */, uint32 Slot_, uint32 RandomSuffix, bool dummy /* = false */ )
{
    int32 Slot = Slot_;
    m_isDirty = true;

    uint8 invSlot = m_owner->GetInventory()->GetInventorySlotByGuid( GetGUID() );

    // Create the enchantment struct.
    EnchantmentInstance *Instance = new EnchantmentInstance();
    Instance->ApplyTime = UNIXTIME;
    Instance->Slot = Slot;
    Instance->Enchantment = dbcSpellItemEnchant.LookupEntry(enchantId);
    Instance->EnchantmentId = enchantId;
    Instance->Duration = Duration;
    Instance->RemoveAtLogout = RemoveAtLogout;
    Instance->RandomSuffix = RandomSuffix;
    Instance->Dummy = dummy;

    m_owner->ApplyItemMods(this, invSlot, false);

    // Set the enchantment in the item fields.
    SetEnchantmentId(Slot, enchantId);
    SetEnchantmentDuration(Slot, (uint32)Instance->ApplyTime);
    SetEnchantmentCharges(Slot, 0);

    // Add it to our map.
    if(m_enchantments[Slot])
        delete m_enchantments[Slot];
    m_enchantments[Slot] = Instance;
    m_owner->ApplyItemMods(this, invSlot, true);

    // No need to send the log packet, if the owner isn't in world (we're still loading)
    if( !m_owner->IsInWorld() )
        return Slot;

    m_owner->SaveToDB(false);
    return Slot;
}

void Item::LoadEnchantment(uint8 slot, uint32 enchantId, uint32 suffix, uint32 expireTime, uint32 charges)
{
    uint8 invSlot = m_owner->GetInventory()->GetInventorySlotByGuid( GetGUID() );
    EnchantmentInstance *Instance = new EnchantmentInstance();
    Instance->Enchantment = dbcSpellItemEnchant.LookupEntry(enchantId);
    Instance->EnchantmentId = enchantId;
    Instance->Slot = slot;
    Instance->RandomSuffix = suffix;
    Instance->Duration = expireTime;
    Instance->ApplyTime = UNIXTIME;
    Instance->RemoveAtLogout = false;
    Instance->Dummy = false;

    // Set the enchantment in the item fields.
    SetEnchantmentId(slot, enchantId);
    SetEnchantmentDuration(slot, UNIXTIME);
    SetEnchantmentCharges(slot, charges);

    // Add it to our map.
    if(m_enchantments[slot])
        delete m_enchantments[slot];
    m_enchantments[slot] = Instance;
}

void Item::RemoveEnchantment( uint32 Slot )
{
    // Make sure we actually exist.
    EnchantmentInstance *instance = m_enchantments[Slot];
    if(instance == NULL)
        return;

    uint8 invSlot = m_owner->GetInventory()->GetInventorySlotByGuid( GetGUID() );
    m_owner->ApplyItemMods(this, invSlot, false);

    m_isDirty = true;

    // Unset the item fields.
    uint32 EnchantBase = Slot * 3 + ITEM_FIELD_ENCHANTMENT_DATA;
    SetUInt32Value( EnchantBase + 0, 0 );
    SetUInt32Value( EnchantBase + 1, 0 );
    SetUInt32Value( EnchantBase + 2, 0 );

    // Remove the enchantment instance.
    m_enchantments[Slot] = NULL;
    delete instance;
    m_owner->ApplyItemMods(this, invSlot, true);
}

void Item::EventRemoveEnchantment( uint32 Slot )
{
    // Remove the enchantment.
    RemoveEnchantment( Slot );
}

int32 Item::FindFreeEnchantSlot( SpellItemEnchantEntry* Enchantment, uint32 random_type )
{
    uint32 GemSlotsReserve = 3;
    if( GetProto()->SocketBonus )
        GemSlotsReserve++;

    if( random_type == 1 )      // random prop
    {
        for( uint32 Slot = 8; Slot < 11; ++Slot )
            if( m_uint32Values[ITEM_FIELD_ENCHANTMENT_DATA + Slot * 3] == 0 )
                return Slot;
    }
    else if( random_type == 2 ) // random suffix
    {
        for( uint32 Slot = 6; Slot < 11; ++Slot )
            if( m_uint32Values[ITEM_FIELD_ENCHANTMENT_DATA + Slot * 3] == 0 )
                return Slot;
    }

    for( uint32 Slot = GemSlotsReserve + 2; Slot < 11; Slot++ )
    {
        if( m_uint32Values[ITEM_FIELD_ENCHANTMENT_DATA + Slot * 3] == 0 )
            return Slot;
    }

    return -1;
}

int32 Item::HasEnchantment( uint32 Id )
{
    for( uint32 Slot = 0; Slot < 11; Slot++ )
    {
        if( m_uint32Values[ITEM_FIELD_ENCHANTMENT_DATA + Slot * 3] == Id )
            return Slot;
    }

    return -1;
}

void Item::ModifyEnchantmentTime( uint32 Slot, uint32 Duration )
{
    EnchantmentInstance *instance = m_enchantments[Slot];
    if(instance == NULL)
        return;

    // Reset the apply time.
    instance->ApplyTime = UNIXTIME;
    instance->Duration = Duration;

    // Send update packet
    SendEnchantTimeUpdate( instance->Slot, Duration );
}

void Item::SendEnchantTimeUpdate( uint32 Slot, uint32 Duration )
{
    WorldPacket data(SMSG_ITEM_ENCHANT_TIME_UPDATE, 24);
    data << GetGUID();
    data << Slot;
    data << Duration;
    data << m_owner->GetGUID();
    m_owner->PushPacket(&data);
}

void Item::RemoveAllEnchantments( bool OnlyTemporary )
{
    for(uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; i++)
    {
        if(EnchantmentInstance *instance = m_enchantments[i])
        {
            if( OnlyTemporary && instance->Duration == 0 )
                continue;
            RemoveEnchantment( instance->Slot );
        }
    }
}

void Item::RemoveRelatedEnchants( SpellItemEnchantEntry* newEnchant )
{
    for(uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; i++)
    {
        if(EnchantmentInstance *instance = m_enchantments[i])
        {
            if( instance->Enchantment && (instance->Enchantment->Id == newEnchant->Id || ( instance->Enchantment->EnchantGroups > 1 && newEnchant->EnchantGroups > 1 ) ))
            {
                RemoveEnchantment( instance->Slot );
            }
        }
    }
}

void Item::RemovePermanentEnchant()
{
    for(uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; i++)
    {
        if(EnchantmentInstance *instance = m_enchantments[i])
        {
            if( instance->Enchantment == NULL || instance->Duration != 0 )// not perm
                continue;
            if(IsGemRelated( instance->Enchantment ))
                continue;
            RemoveEnchantment(i);
            return;
        }
    }
}

void Item::RemoveTemporaryEnchant()
{
    for(uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; i++)
    {
        if(EnchantmentInstance *instance = m_enchantments[i])
        {
            if(instance->Enchantment == NULL || instance->Enchantment->Id == GetProto()->SocketBonus)
                continue;
            RemoveEnchantment(i);
            return;
        }
    }
}

void Item::RemoveSocketBonusEnchant()
{
    for(uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; i++)
    {
        if(EnchantmentInstance *instance = m_enchantments[i])
        {
            if(instance->Enchantment == NULL || instance->Enchantment->Id != GetProto()->SocketBonus)
                continue;
            RemoveEnchantment(i);
            return;
        }
    }
}

uint32 Item::CountGemsWithLimitId(uint32 LimitId)
{
    uint32 result = 0;
    for(uint32 count = 0; count < 3; count++)
    {
        EnchantmentInstance* ei = GetEnchantment(SOCK_ENCHANTMENT_SLOT1 + count);
        if(ei && ei->Enchantment && ei->Enchantment->GemEntry )
        {
            ItemPrototype* ip = sItemMgr.LookupEntry(ei->Enchantment->GemEntry);
            if(ip && ip->ItemLimitCategory == LimitId)
                result++;
        }
    }
    return result;
}

bool Item::IsGemRelated( SpellItemEnchantEntry* Enchantment )
{
    if( GetProto()->SocketBonus == Enchantment->Id )
        return true;

    return( Enchantment->GemEntry != 0 );
}

std::string ItemPrototype::ConstructItemLink(uint32 random_prop, uint32 random_suffix, uint32 stack)
{
    if( Quality > ITEM_QUALITY_DBC_MAX )
        return "INVALID_ITEM";

    char buf[1000], sbuf[50], nbuf[255], rptxt[100], rstxt[100];

    // stack text
    if( stack > 1 )
        snprintf(sbuf, 50, "x%u", stack);
    else sbuf[0] = 0;

    if(Name.length() < 255)
        snprintf(nbuf, 255, "%s", Name.c_str());
    else nbuf[0] = 0;

    // null 'em
    rptxt[0] = 0;
    rstxt[0] = 0;

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
    snprintf(buf, 1000, "%s|Hitem:%u:0:0:0:0:0:%d:0|h[%s%s%s]%s|h|r", ItemManager::g_itemQualityColours[Quality], ItemId, /* suffix/prop */ random_suffix ? (-(int32)random_suffix) : random_prop, nbuf, rstxt, rptxt, sbuf);

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
