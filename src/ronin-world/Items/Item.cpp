/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Item::Item(ItemPrototype *proto, uint32 counter, uint32 fieldcount) : Object(MAKE_NEW_GUID(HIGHGUID_TYPE_ITEM, proto->ItemId, counter), fieldcount), m_proto(proto), m_textId(0)
{
    SetTypeFlags(TYPEMASK_TYPE_ITEM);

    SetUInt32Value(OBJECT_FIELD_ENTRY, proto->ItemId);
}

Item::~Item()
{

}

void Item::Init()
{
    Object::Init();
}

void Item::Destruct()
{
    if( IsInWorld() )
        RemoveFromWorld();

    Object::Destruct();
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

    ApplyRandomProperties( false );

    if(Charter* charter = guildmgr.GetCharterByItemGuid(GetLowGUID()))
    {
        SetUInt32Value(ITEM_FIELD_ENCHANTMENT_DATA, charter->GetID());

        // Charter stuff
        if(m_uint32Values[OBJECT_FIELD_ENTRY] == ITEM_ENTRY_GUILD_CHARTER)
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( m_owner->m_playerInfo->charterId[CHARTER_TYPE_GUILD] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_DATA, m_owner->m_playerInfo->charterId[CHARTER_TYPE_GUILD] );
        }

        if( m_uint32Values[OBJECT_FIELD_ENTRY] == ARENA_TEAM_CHARTER_2v2 )
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( m_owner->m_playerInfo->charterId[CHARTER_TYPE_ARENA_2V2] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_DATA, m_owner->m_playerInfo->charterId[CHARTER_TYPE_ARENA_2V2] );
        }

        if( m_uint32Values[OBJECT_FIELD_ENTRY] == ARENA_TEAM_CHARTER_3v3 )
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( m_owner->m_playerInfo->charterId[CHARTER_TYPE_ARENA_3V3] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_DATA, m_owner->m_playerInfo->charterId[CHARTER_TYPE_ARENA_3V3] );
        }

        if( m_uint32Values[OBJECT_FIELD_ENTRY] == ARENA_TEAM_CHARTER_5v5 )
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( m_owner->m_playerInfo->charterId[CHARTER_TYPE_ARENA_5V5] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_DATA, m_owner->m_playerInfo->charterId[CHARTER_TYPE_ARENA_5V5] );
        }
    }
}

void Item::ApplyRandomProperties( bool apply )
{
    // apply random properties
    if( m_uint32Values[ITEM_FIELD_RANDOM_PROPERTIES_ID] != 0 )
    {
        if( int32( m_uint32Values[ITEM_FIELD_RANDOM_PROPERTIES_ID] ) > 0 )      // Random Property
        {
            ItemRandomPropertiesEntry* rp= dbcItemRandomProperties.LookupEntry( m_uint32Values[ITEM_FIELD_RANDOM_PROPERTIES_ID] );
            if(rp == NULL)
                return;

            int32 Slot;
            for( int k = 0; k < 3; k++ )
            {
                if( rp->enchant_id[k] != 0 )
                {
                    SpellItemEnchantEntry* ee = dbcSpellItemEnchant.LookupEntry( rp->enchant_id[k] );
                    Slot = HasEnchantment( ee->Id );
                    if( Slot < 0 )
                    {
                        Slot = FindFreeEnchantSlot( ee, 1 );
                        AddEnchantment( ee, 0, false, apply, true, Slot );
                    } else if( apply )
                        ApplyEnchantmentBonus( Slot, true );
                }
            }
        }
        else
        {
            ItemRandomSuffixEntry* rs = dbcItemRandomSuffix.LookupEntry( abs( int( m_uint32Values[ITEM_FIELD_RANDOM_PROPERTIES_ID] ) ) );
            if(rs == NULL)
                return;
            int32 Slot;
            for( uint32 k = 0; k < 3; ++k )
            {
                if( rs->enchantments[k] != 0 )
                {
                    SpellItemEnchantEntry* ee = dbcSpellItemEnchant.LookupEntry( rs->enchantments[k] );
                    Slot = HasEnchantment( ee->Id );
                    if( Slot < 0 )
                    {
                        Slot = FindFreeEnchantSlot( ee, 2 );
                        AddEnchantment( ee, 0, false, apply, true, Slot, rs->prefixes[k] );
                    }
                    else
                        if( apply )
                            ApplyEnchantmentBonus( Slot, true );
                }
            }
        }
    }
}

void Item::SaveToDB( int8 containerslot, uint8 slot, bool firstsave, QueryBuffer* buf )
{
    if( !m_isDirty && !firstsave )
        return;

    std::stringstream ss;

    ss << "REPLACE INTO item_data VALUES(";

    ss << m_uint32Values[ITEM_FIELD_OWNER] << ",";
    ss << m_uint32Values[OBJECT_FIELD_GUID] << ",";
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

    if( firstsave )
    {
        CharacterDatabase.WaitExecute( ss.str().c_str() );
        CharacterDatabase.WaitExecute("REPLACE INTO character_inventory VALUES(%u, %u, %i, %u);", m_owner->GetLowGUID(), GetLowGUID(), containerslot, slot);
    }
    else
    {
        if( buf == NULL )
        {
            CharacterDatabase.Execute( ss.str().c_str() );
            CharacterDatabase.Execute("REPLACE INTO character_inventory VALUES(%u, %u, %i, %u);", m_owner->GetLowGUID(), GetLowGUID(), containerslot, slot);
        }
        else
        {
            buf->AddQueryStr( ss.str() );
            buf->AddQueryStr(format("REPLACE INTO character_inventory VALUES(%u, %u, %i, %u);", m_owner->GetLowGUID(), GetLowGUID(), containerslot, slot));
        }
    }

    m_isDirty = false;
}

void Item::DeleteFromDB()
{
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

    CharacterDatabase.Execute( "DELETE FROM playeritems WHERE guid = %u", m_uint32Values[OBJECT_FIELD_GUID] );
}

void Item::RemoveFromWorld()
{
    // if we have an owner->send destroy
    if( m_owner != NULL )
        DestroyForPlayer( m_owner );
}

void Item::SetOwner( Player* owner )
{
    if( owner != NULL )
        SetUInt64Value( ITEM_FIELD_OWNER, owner->GetGUID() );
    else SetUInt64Value( ITEM_FIELD_OWNER, 0 );

    m_owner = owner;
}

int32 Item::AddEnchantment(SpellItemEnchantEntry* Enchantment, uint32 Duration, bool Perm /* = false */, bool apply /* = true */, bool RemoveAtLogout /* = false */, uint32 Slot_, uint32 RandomSuffix, bool dummy /* = false */ )
{
    int32 Slot = Slot_;
    m_isDirty = true;

    // Create the enchantment struct.
    EnchantmentInstance Instance;
    Instance.ApplyTime = UNIXTIME;
    Instance.BonusApplied = false;
    Instance.Slot = Slot;
    Instance.Enchantment = Enchantment;
    Instance.Duration = Duration;
    Instance.RemoveAtLogout = RemoveAtLogout;
    Instance.RandomSuffix = RandomSuffix;
    Instance.Dummy = dummy;

    // Set the enchantment in the item fields.
    SetEnchantmentId(Slot, Enchantment->Id);
    SetEnchantmentDuration(Slot, (uint32)Instance.ApplyTime);
    SetEnchantmentCharges(Slot, 0);

    // Add it to our map.
    m_enchantments.insert(std::make_pair((uint32)Slot, Instance));

    if( m_owner == NULL )
        return Slot;

    // No need to send the log packet, if the owner isn't in world (we're still loading)
    if( !m_owner->IsInWorld() )
        return Slot;

    if( apply )
    {
        if(Instance.Dummy)
            return Slot;

        /* Only apply the enchantment bonus if we're equipped */
        uint8 slot = m_owner->GetInventory()->GetInventorySlotByGuid( GetGUID() );
        if( slot > EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END )
            ApplyEnchantmentBonus( Slot, true );
    }

    m_owner->SaveToDB(false);
    return Slot;
}

void Item::RemoveEnchantment( uint32 EnchantmentSlot )
{
    // Make sure we actually exist.
    EnchantmentMap::iterator itr = m_enchantments.find( EnchantmentSlot );
    if( itr == m_enchantments.end() )
        return;

    m_isDirty = true;
    uint32 Slot = itr->first;
    if( itr->second.BonusApplied )
        ApplyEnchantmentBonus( EnchantmentSlot, false );

    // Unset the item fields.
    uint32 EnchantBase = Slot * 3 + ITEM_FIELD_ENCHANTMENT_DATA;
    SetUInt32Value( EnchantBase + 0, 0 );
    SetUInt32Value( EnchantBase + 1, 0 );
    SetUInt32Value( EnchantBase + 2, 0 );

    // Remove the enchantment instance.
    m_enchantments.erase( itr );
}

void Item::ApplyEnchantmentBonus( uint32 Slot, bool Apply )
{
    if( m_owner == NULL )
        return;

    EnchantmentMap::iterator itr = m_enchantments.find( Slot );
    if( itr == m_enchantments.end() )
        return;

    SpellItemEnchantEntry* Entry = itr->second.Enchantment;
    uint32 RandomSuffixAmount = itr->second.RandomSuffix;

    if( itr->second.Dummy )
        return;

    if( itr->second.BonusApplied == Apply )
        return;

    itr->second.BonusApplied = Apply;

    // Apply the visual on the player.
    uint32 ItemSlot = m_owner->GetInventory()->GetInventorySlotByGuid( GetGUID() );
    if(ItemSlot < EQUIPMENT_SLOT_END && Slot < 1)
    {
        uint32 VisibleBase = PLAYER_VISIBLE_ITEM + 1 + ItemSlot * PLAYER_VISIBLE_ITEM_LENGTH;
        m_owner->SetUInt16Value( VisibleBase, Slot, Apply ? Entry->Id : 0 );
    }

    if( Apply )
    {
        // Send the enchantment time update packet.
        SendEnchantTimeUpdate( itr->second.Slot, itr->second.Duration );
    }
}

void Item::ApplyEnchantmentBonuses()
{
    EnchantmentMap::iterator itr, itr2;
    for( itr = m_enchantments.begin(); itr != m_enchantments.end();  )
    {
        itr2 = itr++;
        if(!itr2->second.Dummy)
            ApplyEnchantmentBonus( itr2->first, true );
    }
}

void Item::RemoveEnchantmentBonuses()
{
    EnchantmentMap::iterator itr, itr2;
    for( itr = m_enchantments.begin(); itr != m_enchantments.end(); )
    {
        itr2 = itr++;
        if(!itr2->second.Dummy)
            ApplyEnchantmentBonus( itr2->first, false );
    }
}

void Item::EventRemoveEnchantment( uint32 Slot )
{
    // Remove the enchantment.
    RemoveEnchantment( Slot );
}

int32 Item::FindFreeEnchantSlot( SpellItemEnchantEntry* Enchantment, uint32 random_type )
{
    uint32 GemSlotsReserve = GetMaxSocketsCount();
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
    EnchantmentMap::iterator itr = m_enchantments.find( Slot );
    if( itr == m_enchantments.end() )
        return;

    // Reset the apply time.
    itr->second.ApplyTime = UNIXTIME;
    itr->second.Duration = Duration;

    // Send update packet
    SendEnchantTimeUpdate( itr->second.Slot, Duration );
}

void Item::SendEnchantTimeUpdate( uint32 Slot, uint32 Duration )
{
    WorldPacket data(SMSG_ITEM_ENCHANT_TIME_UPDATE, 24);
    data << GetGUID();
    data << Slot;
    data << Duration;
    data << m_owner->GetGUID();
    m_owner->CopyAndSendDelayedPacket(&data);
}

void Item::RemoveAllEnchantments( bool OnlyTemporary )
{
    EnchantmentMap::iterator itr, it2;
    for( itr = m_enchantments.begin(); itr != m_enchantments.end(); )
    {
        it2 = itr++;
        if( OnlyTemporary && it2->second.Duration == 0 )
            continue;

        RemoveEnchantment( it2->first );
    }
}

void Item::RemoveRelatedEnchants( SpellItemEnchantEntry* newEnchant )
{
    EnchantmentMap::iterator itr,itr2;
    for( itr = m_enchantments.begin(); itr != m_enchantments.end(); )
    {
        itr2 = itr++;
        if( itr2->second.Enchantment->Id == newEnchant->Id || ( itr2->second.Enchantment->EnchantGroups > 1 && newEnchant->EnchantGroups > 1 ) )
        {
            RemoveEnchantment( itr2->first );
        }
    }
}

void Item::RemoveProfessionEnchant()
{
    EnchantmentMap::iterator itr;
    for( itr = m_enchantments.begin(); itr != m_enchantments.end(); itr++ )
    {
        if( itr->second.Duration != 0 )// not perm
            continue;

        if( IsGemRelated( itr->second.Enchantment ) )
            continue;

        RemoveEnchantment( itr->first );
        return;
    }
}

void Item::RemoveSocketBonusEnchant()
{
    EnchantmentMap::iterator itr;

    for( itr = m_enchantments.begin(); itr != m_enchantments.end(); itr++ )
    {
        if( itr->second.Enchantment->Id == GetProto()->SocketBonus )
        {
            RemoveEnchantment( itr->first );
            return;
        }
    }
}

uint32 Item::CountGemsWithLimitId(uint32 LimitId)
{
    uint32 result = 0;
    for(uint32 count = 0; count < GetMaxSocketsCount(); count++)
    {
        EnchantmentInstance* ei = GetEnchantment(SOCK_ENCHANTMENT_SLOT1 + count);
        if(ei && ei->Enchantment->GemEntry )
        {
            ItemPrototype* ip = sItemMgr.LookupEntry(ei->Enchantment->GemEntry);
            if(ip && ip->ItemLimitCategory == LimitId)
                result++;
        }
    }
    return result;
}

EnchantmentInstance* Item::GetEnchantment( uint32 slot )
{
    EnchantmentMap::iterator itr = m_enchantments.find( slot );
    if( itr != m_enchantments.end() )
        return &itr->second;
    return NULL;
}

bool Item::IsGemRelated( SpellItemEnchantEntry* Enchantment )
{
    if( GetProto()->SocketBonus == Enchantment->Id )
        return true;

    return( Enchantment->GemEntry != 0 );
}

uint32 Item::GetMaxSocketsCount()
{
    uint32 c = 0;
    for( uint32 x = 0; x < 3; x++ )
        if( GetProto()->ItemSocket[x] )
            c++;
    return c;
}

std::string ItemPrototype::ConstructItemLink(uint32 random_prop, uint32 random_suffix, uint32 stack)
{
    if( Quality > ITEM_QUALITY_DBC_MAX )
        return "INVALID_ITEM";

    char buf[1000];
    char sbuf[50];
    char rptxt[100];
    char rstxt[100];

    // stack text
    if( stack > 1 )
        snprintf(sbuf, 50, "x%u", stack);
    else
        sbuf[0] = 0;

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
    snprintf(buf, 1000, "%s|Hitem:%u:0:0:0:0:0:%d:0|h[%s%s%s]%s|h|r", ItemManager::g_itemQualityColours[Quality], ItemId, /* suffix/prop */ random_suffix ? (-(int32)random_suffix) : random_prop,
        Name1, rstxt, rptxt, sbuf);

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
