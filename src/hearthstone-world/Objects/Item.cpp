/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Item::Item( WoWGuid guid, uint32 fieldCount ) : Object(guid, fieldCount) 
{
    SetTypeFlags(TYPEMASK_TYPE_ITEM);
    SetUInt32Value(OBJECT_FIELD_ENTRY, guid.getEntry());

    m_inWorld = false;
    m_itemProto = NULL;
    m_owner = NULL;
    m_locked = false;
    StatsApplied = false;
    wrapped_item_id = 0;
    memset(OnUseSpells, 0, sizeof(uint32)*3);
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
    EnchantmentMap::iterator itr;
    for( itr = Enchantments.begin(); itr != Enchantments.end(); itr++ )
    {
        if( itr->second.Enchantment->type == 0 && itr->second.Slot == 0 && itr->second.ApplyTime == 0 && itr->second.Duration == 0 )
        {
            delete itr->second.Enchantment;
            itr->second.Enchantment = NULL;
        }
    }

    m_owner = NULL;
    Object::Destruct();
}

void Item::Create( uint32 itemid, Player* owner )
{
    SetUInt32Value( OBJECT_FIELD_ENTRY, itemid );

    if( owner != NULL )
    {
        SetUInt64Value( ITEM_FIELD_OWNER, owner->GetGUID() );
        SetUInt64Value( ITEM_FIELD_CONTAINED, owner->GetGUID() );
    }

    SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );

    m_itemProto = sItemMgr.LookupEntry( itemid );

    ASSERT( m_itemProto );

    for(uint8 i = 0; i < 5; i++)
        SetUInt32Value( ITEM_FIELD_SPELL_CHARGES+i, m_itemProto->Spells[i].Charges );

    SetUInt32Value(ITEM_FIELD_DURATION, abs((int)m_itemProto->Duration));
    SetUInt32Value(ITEM_FIELD_MAXDURABILITY, m_itemProto->MaxDurability );
    SetUInt32Value(ITEM_FIELD_DURABILITY, m_itemProto->MaxDurability );
    SetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME, 0);

    m_owner = owner;
    if( m_itemProto->LockId > 1 )
        m_locked = true;
    else m_locked = false;
}

uint32 Item::CalcMinDamage()
{
    ASSERT(m_itemProto);
    return m_itemProto->minDamage;
}

uint32 Item::CalcMaxDamage()
{
    ASSERT(m_itemProto);
    return m_itemProto->maxDamage;
}

void Item::LoadFromDB(Field* fields, Player* plr, bool light )
{
    uint32 itemid = fields[2].GetUInt32(), random_prop = fields[9].GetUInt32(), random_suffix = fields[10].GetUInt32();
    m_itemProto = sItemMgr.LookupEntry( itemid );

    ASSERT( m_itemProto );

    if(m_itemProto->LockId > 1)
        m_locked = true;
    else m_locked = false;

    SetUInt32Value( OBJECT_FIELD_ENTRY, itemid );
    m_owner = plr;

    wrapped_item_id = fields[3].GetUInt32();
    SetUInt32Value(ITEM_FIELD_GIFTCREATOR, fields[4].GetUInt32());
    SetUInt32Value(ITEM_FIELD_CREATOR, fields[5].GetUInt32());

    if(IsContainer())
        SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);
    else
    {
        int32 count = fields[6].GetUInt32();
        if(m_itemProto->MaxCount > 0 && count > m_itemProto->MaxCount)
            count = m_itemProto->MaxCount;
        SetUInt32Value( ITEM_FIELD_STACK_COUNT, count);
    }

    // Again another for that did not indent to make it do anything for more than
    // one iteration x == 0 was the only one executed
    for( uint32 x = 0; x < 5; x++ )
    {
        if( m_itemProto->Spells[x].Id )
        {
            SetUInt32Value( ITEM_FIELD_SPELL_CHARGES + x , fields[7].GetUInt32() );
            break;
        }
    }

    SetUInt32Value( ITEM_FIELD_FLAGS, fields[8].GetUInt32() );
    Bind(ITEM_BIND_ON_PICKUP); // Check if we need to bind our shit.

    if( random_prop )
        SetRandomProperty( random_prop );
    else if( random_suffix )
        SetRandomSuffix( random_suffix );

    SetTextID( fields[11].GetUInt32() );

    SetUInt32Value( ITEM_FIELD_MAXDURABILITY, m_itemProto->MaxDurability );
    SetUInt32Value( ITEM_FIELD_DURABILITY, fields[12].GetUInt32() );

    if( light == true )
        return;

    std::string enchant_field = fields[15].GetString();
    std::vector< std::string > enchants = RONIN_UTIL::StrSplit( enchant_field, ";" );
    uint32 enchant_id = 0, time_left = 0, enchslot = 0, dummy = 0;
    SpellItemEnchantEntry* entry;

    for( std::vector<std::string>::iterator itr = enchants.begin(); itr != enchants.end(); itr++ )
    {
        if( sscanf( (*itr).c_str(), "%u,%u,%u,%u", (unsigned int*)&enchant_id, (unsigned int*)&time_left, (unsigned int*)&enchslot, (unsigned int*)&dummy) > 3 )
        {
            if(SpellItemEnchantEntry *entry = dbcSpellItemEnchant.LookupEntry( enchant_id ))
            {
                if( entry->Id == enchant_id )
                {
                    AddEnchantment( entry, time_left, ( time_left == 0 ), false, false, enchslot, 0, ((dummy > 0) ? true : false) );
                    //(enchslot != 2) ? false : true, false);
                }
            }
        }
    }

    ApplyRandomProperties( false );

    if(Charter* charter = guildmgr.GetCharterByItemGuid(GetLowGUID()))
    {
        SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1, charter->GetID());
        SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
        SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
        switch(GetUInt32Value(OBJECT_FIELD_ENTRY))
        {
        case ITEM_ENTRY_GUILD_CHARTER:
            {
                if( plr->m_playerInfo->charterId[CHARTER_TYPE_GUILD] )
                    SetUInt32Value( ITEM_FIELD_ENCHANTMENT_1_1, plr->m_playerInfo->charterId[CHARTER_TYPE_GUILD] );
            }break;
        case ARENA_TEAM_CHARTER_2v2:
            {
                if( plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_2V2] )
                    SetUInt32Value( ITEM_FIELD_ENCHANTMENT_1_1, plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_2V2] );
            }break;
        case ARENA_TEAM_CHARTER_3v3:
            {
                if( plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_3V3] )
                    SetUInt32Value( ITEM_FIELD_ENCHANTMENT_1_1, plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_3V3] );
            }break;
        case ARENA_TEAM_CHARTER_5v5:
            {
                if( plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_5V5] )
                    SetUInt32Value( ITEM_FIELD_ENCHANTMENT_1_1, plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_5V5] );
            }break;
        }
    }
}

void Item::ApplyRandomProperties( bool apply )
{
    // apply random properties
    if( int32 randomProp = GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID) )
    {
        if( randomProp > 0 )      // Random Property
        {
            if(ItemRandomPropertiesEntry* rp= dbcItemRandomProperties.LookupEntry(randomProp))
            {
                for( int k = 0; k < 3; k++ )
                {
                    if( rp->enchant_id[k] != 0 )
                    {
                        if(SpellItemEnchantEntry* ee = dbcSpellItemEnchant.LookupEntry( rp->enchant_id[k] ))
                        {
                            int32 Slot = HasEnchantment( ee->Id );
                            if( Slot < 0 )
                            {
                                AddEnchantment( ee, 0, false, apply, true, FindFreeEnchantSlot( ee, 1 ) );
                            } else if( apply ) ApplyEnchantmentBonus( Slot, true );
                        }
                    }
                }
            }
        }
        else if(ItemRandomSuffixEntry* rs = dbcItemRandomSuffix.LookupEntry(abs(randomProp)))
        {
            for( uint32 k = 0; k < 3; ++k )
            {
                if( rs->enchantments[k] != 0 )
                {
                    SpellItemEnchantEntry* ee = dbcSpellItemEnchant.LookupEntry( rs->enchantments[k] );
                    int32 Slot = HasEnchantment( ee->Id );
                    if( Slot < 0 )
                    {
                        AddEnchantment( ee, 0, false, apply, true, FindFreeEnchantSlot( ee, 2 ), rs->prefixes[k] );
                    } else if( apply ) ApplyEnchantmentBonus( Slot, true );
                }
            }
        }
    }
}

void Item::SaveToDB( int16 containerslot, int16 slot, bool firstsave, QueryBuffer* buf )
{
    std::stringstream ss;

    ss << "REPLACE INTO item_data VALUES(";
    ss << uint64(m_objGuid)
    << ", " << GetUInt64Value(ITEM_FIELD_CONTAINED)
    << ", " << GetUInt64Value(ITEM_FIELD_CREATOR)
    << ", " << GetUInt32Value(ITEM_FIELD_STACK_COUNT)
    << ", " << GetUInt32Value(ITEM_FIELD_FLAGS)
    << ", " << GetUInt32Value(ITEM_FIELD_PROPERTY_SEED)
    << ", " << GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID)
    << ", " << GetUInt32Value(ITEM_FIELD_DURABILITY)
    << ", " << GetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME)
    << ", " << wrapped_item_id
    << ", " << GetUInt64Value(ITEM_FIELD_GIFTCREATOR);

    ss << wrapped_item_id << ",";
    ss << GetUInt32Value(ITEM_FIELD_GIFTCREATOR) << ",";
    ss << GetUInt32Value(ITEM_FIELD_CREATOR) << ",";

    ss << GetUInt32Value(ITEM_FIELD_STACK_COUNT) << ",";
    ss << int32(GetChargesLeft()) << ",";
    ss << GetUInt32Value(ITEM_FIELD_FLAGS) << ",";
    ss << random_prop << "," << random_suffix << ",";
    ss << GetTextID() << ",";
    ss << GetUInt32Value(ITEM_FIELD_DURABILITY) << ",";
    ss << int32(containerslot) << ",";
    ss << int32(slot) << ",'";

    // Pack together enchantment fields
    if( Enchantments.size() > 0 )
    {
        for(EnchantmentMap::iterator itr = Enchantments.begin(); itr != Enchantments.end(); itr++)
        {
            if( itr->second.RemoveAtLogout )
                continue;

            uint32 elapsed_duration = uint32( UNIXTIME - itr->second.ApplyTime );
            int32 remaining_duration = itr->second.Duration - elapsed_duration;
            if( remaining_duration < 0 )
                remaining_duration = 0;

            if( itr->second.Enchantment && ( remaining_duration && remaining_duration > 5 ) || ( itr->second.Duration == 0 ) )
            {
                ss << itr->second.Enchantment->Id << ",";
                ss << remaining_duration << ",";
                ss << itr->second.Slot << ",";
                ss << uint32(itr->second.Dummy ? 1 : 0) << ";";
            }
        }
    }

    ss << "',' ')";

    if( firstsave || buf == NULL )
        CharacterDatabase.Execute( ss.str().c_str() );
    else buf->AddQueryStr( ss.str() );
}

void Item::DeleteFromDB()
{
    if( m_itemProto->ContainerSlots > 0 && GetTypeId() == TYPEID_CONTAINER )
    {
        /* deleting a Container* */
        for( int32 i = 0; i < m_itemProto->ContainerSlots; i++ )
        {
            if( castPtr<Container>(this)->GetItem( i ) != NULL )
            {
                /* abort the delete */
                return;
            }
        }
    }

    CharacterDatabase.Execute( "DELETE FROM playeritems WHERE guid = %u", GetUInt32Value(OBJECT_FIELD_GUID) );
}

uint32 Item::GetSkillByProto( uint32 Class, uint32 SubClass )
{
    if( Class == 4 && SubClass < 7 )
        return arm_skills[SubClass];
    else if( Class == 2 && SubClass < 20 )//no skill for fishing
        return weap_skills[SubClass];
    return 0;
}

//This map is used for profess.
//Prof packe strcut: {SMSG_SET_PROFICIENCY,(uint8)item_class,(uint32)1<<item_subclass}
//ie: for fishing (it's class=2--weapon, subclass ==20 -- fishing rod) permissive packet
// will have structure 0x2,524288
//this table is needed to get class/subclass by skill, valid classes are 2 and 4

const ItemProf* GetProficiencyBySkill( uint32 skill )
{
    switch( skill )
    {
    case SKILL_CLOTH: return &prof[0];
    case SKILL_LEATHER: return &prof[1];
    case SKILL_MAIL: return &prof[2];
    case SKILL_PLATE_MAIL: return &prof[3];
    case SKILL_SHIELD: return &prof[4];
    case SKILL_AXES: return &prof[5];
    case SKILL_2H_AXES: return &prof[6];
    case SKILL_BOWS: return &prof[7];
    case SKILL_GUNS: return &prof[8];
    case SKILL_MACES: return &prof[9];
    case SKILL_2H_MACES: return &prof[10];
    case SKILL_POLEARMS: return &prof[11];
    case SKILL_SWORDS: return &prof[12];
    case SKILL_2H_SWORDS: return &prof[13];
    case SKILL_STAVES: return &prof[14];
    case SKILL_FIST_WEAPONS: return &prof[15];
    case SKILL_DAGGERS: return &prof[16];
    case SKILL_THROWN: return &prof[17];
    case SKILL_SPEARS: return &prof[18];
    case SKILL_CROSSBOWS: return &prof[19];
    case SKILL_WANDS: return &prof[20];
    case SKILL_FISHING: return &prof[21];
    }
    return NULL;
}

uint32 Item::GetSellPriceForItem( ItemPrototype *proto, uint32 count )
{
    int32 cost;
    cost = proto->SellPrice * ( ( count < 1 ) ? 1 : count );
    return cost;
}

uint32 Item::GetBuyPriceForItem( ItemPrototype* proto, uint32 count, Player* plr, Creature* vendor )
{
    int32 cost = proto->BuyPrice;

    if( plr != NULL && vendor != NULL )
    {
        Standing plrstanding = plr->GetStandingRank( vendor->GetFactionID() );
        cost = float2int32( ceilf( float( proto->BuyPrice ) * pricemod[plrstanding] ) );
    }

    return cost * count;
}

uint32 Item::GetSellPriceForItem( uint32 itemid, uint32 count )
{
    if( ItemPrototype* proto = sItemMgr.LookupEntry( itemid ) )
        return Item::GetSellPriceForItem(proto, count);
    return 1;
}

uint32 Item::GetBuyPriceForItem( uint32 itemid, uint32 count, Player* plr, Creature* vendor )
{
    if( ItemPrototype* proto = sItemMgr.LookupEntry( itemid ) )
        return Item::GetBuyPriceForItem( proto, count, plr, vendor );
    return 1;
}

void Item::RemoveFromWorld(bool destroy)
{
    // if we have an owner->send destroy
    if (IsInWorld() && m_owner != NULL)
        DestroyForPlayer( m_owner );

    m_inWorld = false;
    if (destroy) Destruct();
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
    Enchantments.insert(std::make_pair((uint32)Slot, Instance));

    if( m_owner == NULL )
        return Slot;

    // Add the removal event.
//    if (Duration) m_owner->AddItemEnchantDuration(this, Slot, Duration*1000);

    // No need to send the log packet, if the owner isn't in world (we're still loading)
    if( !m_owner->IsInWorld() )
        return Slot;

    if( apply )
    {
        if( m_owner->GetTradeTarget() )
            m_owner->SendTradeUpdate();

        if(Instance.Dummy)
            return Slot;

        /* Only apply the enchantment bonus if we're equipped */
        uint8 slot = m_owner->GetItemInterface()->GetInventorySlotByGuid( GetGUID() );
        if( slot > EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END )
            ApplyEnchantmentBonus( Slot, true );
    }

    m_owner->SaveToDB(false);
    return Slot;
}

void Item::RemoveEnchantment( uint32 EnchantmentSlot )
{
    // Make sure we actually exist.
    EnchantmentMap::iterator itr = Enchantments.find( EnchantmentSlot );
    if( itr == Enchantments.end() )
        return;

    uint32 Slot = itr->first;
    if( itr->second.BonusApplied )
        ApplyEnchantmentBonus( EnchantmentSlot, false );

    // Unset the item fields.
    uint32 EnchantBase = Slot * 3 + ITEM_FIELD_ENCHANTMENT_1_1;
    SetUInt32Value( EnchantBase + 0, 0 );
    SetUInt32Value( EnchantBase + 1, 0 );
    SetUInt32Value( EnchantBase + 2, 0 );

    // Remove the enchantment event for removal.
//    if(m_owner) m_owner->RemoveItemEnchantDuration(this, Slot);

    // Remove the enchantment instance.
    Enchantments.erase( itr );
}

void Item::ApplyEnchantmentBonus( uint32 Slot, bool Apply )
{
    if( m_owner == NULL )
        return;

    EnchantmentMap::iterator itr = Enchantments.find( Slot );
    if( itr == Enchantments.end() )
        return;

    SpellItemEnchantEntry* Entry = itr->second.Enchantment;
    uint32 RandomSuffixAmount = itr->second.RandomSuffix;

    if( itr->second.Dummy )
        return;

    if( itr->second.BonusApplied == Apply )
        return;

    itr->second.BonusApplied = Apply;

    // Apply the visual on the player.
    uint32 ItemSlot = m_owner->GetItemInterface()->GetInventorySlotByGuid( GetGUID() );
    if(ItemSlot < EQUIPMENT_SLOT_END && Slot < 1)
        m_owner->SetUInt16Value( (PLAYER_VISIBLE_ITEM+1 + ItemSlot * PLAYER_VISIBLE_ITEM_LENGTH), Slot, Apply ? Entry->Id : 0 );

    if( Apply )
    {
        // Send the enchantment time update packet.
        SendEnchantTimeUpdate( itr->second.Slot, itr->second.Duration );
    }

    // Another one of those for loop that where not indented properly god knows what will break
    // but i made it actually affect the code below it
    for( uint32 c = 0; c < 3; c++ )
    {
        if( Entry->type[c] )
        {
            // Depending on the enchantment type, take the appropriate course of action.
            switch( Entry->type[c] )
            {
            case 1:      // Trigger spell on melee attack.
                {
                    //m_owner->ModifyProc(apply, Entry->spell[c]);
                }break;

            case 2:      // Mod damage done.
                {
                    int32 val = Entry->maxPoints[c];
                    if( RandomSuffixAmount )
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION( RandomSuffixAmount, GetItemRandomSuffixFactor() );

                    m_owner->ModifyBonuses( Apply, GetGUID(), MOD_SLOT_PERM_ENCHANT+Slot, ITEM_STAT_MOD_DAMAGE_PHYSICAL, val);
                }break;

            case 3:      // Cast spell (usually means apply aura)
                {
                    if( Apply )
                    {
                        SpellCastTargets targets( m_owner->GetGUID() );
                        SpellEntry* sp;

                        if( Entry->spell[c] != 0 )
                        {
                            sp = dbcSpell.LookupEntry( Entry->spell[c] );
                            if( sp == NULL )
                                continue;

                            Spell* spell = new Spell( m_owner, sp, true, NULL );
                            spell->i_caster = castPtr<Item>(this);
                            spell->prepare( &targets );
                        }
                    }
                    else
                    {
                        if( Entry->spell[c] != 0 )
                            m_owner->RemoveAura( Entry->spell[c] );
                    }
                }break;

            case 4:      // Modify physical resistance
                {
                    int32 val = Entry->maxPoints[c];
                    if( RandomSuffixAmount )
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION( RandomSuffixAmount, GetItemRandomSuffixFactor() );

                    m_owner->ModifyBonuses( Apply, GetGUID(), MOD_SLOT_PERM_ENCHANT+Slot, ITEM_STAT_PHYSICAL_RESISTANCE+Entry->spell[c], val);
                }break;

            case 5:  //Modify rating ...order is PLAYER_FIELD_COMBAT_RATING_1 and above
                {
                    //spellid is enum ITEM_STAT_TYPE
                    //min=max is amount
                    int32 val = Entry->maxPoints[c];
                    if( RandomSuffixAmount )
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION( RandomSuffixAmount, GetItemRandomSuffixFactor() );

                    m_owner->ModifyBonuses( Apply, GetGUID(), MOD_SLOT_PERM_ENCHANT+Slot, Entry->spell[c], val);
                }break;

            case 6:  // Rockbiter weapon (increase damage per second... how the hell do you calc that)
                {
                    //if i'm not wrong then we should apply DMPS formula for this. This will have somewhat a larger value 28->34
                    int32 val = Entry->maxPoints[c];
                    if( RandomSuffixAmount )
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION( RandomSuffixAmount, GetItemRandomSuffixFactor() );

                    //int32 value = GetProto()->Delay * val / 1000;
                    m_owner->ModifyBonuses( Apply, GetGUID(), MOD_SLOT_PERM_ENCHANT+Slot, ITEM_STAT_MOD_DAMAGE_PHYSICAL, val);
                }break;

            case 7:
                {
                    if( Apply )
                    {
                        for( uint32 i = 0; i < 3; ++i )
                            OnUseSpells[ i ] = Entry->spell[ i ];
                    }
                    else
                    {
                        for( uint32 i = 0; i < 3; ++i )
                            OnUseSpells[ i ] = 0;
                    }
                }break;

            case 8:{}break;

            default:
                {
                    sLog.Debug( "Enchant","Unknown enchantment type: %u (%u)", Entry->type[c], Entry->Id );
                }break;
            }
        }
    }
}

void Item::ApplyEnchantmentBonuses()
{
    EnchantmentMap::iterator itr, itr2;
    for( itr = Enchantments.begin(); itr != Enchantments.end();  )
    {
        itr2 = itr++;
        if(!itr2->second.Dummy)
            ApplyEnchantmentBonus( itr2->first, true );
    }
}

void Item::RemoveEnchantmentBonuses()
{
    EnchantmentMap::iterator itr, itr2;
    for( itr = Enchantments.begin(); itr != Enchantments.end(); )
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
            if( GetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + Slot * 3) == 0 )
                return Slot;
    }
    else if( random_type == 2 ) // random suffix
    {
        for( uint32 Slot = 6; Slot < 11; ++Slot )
            if( GetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + Slot * 3) == 0 )
                return Slot;
    }

    for( uint32 Slot = GemSlotsReserve + 2; Slot < 11; Slot++ )
    {
        if( GetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + Slot * 3) == 0 )
            return Slot;
    }

    return -1;
}

int32 Item::HasEnchantment( uint32 Id )
{
    for( uint32 Slot = 0; Slot < 11; Slot++ )
    {
        if( GetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + Slot * 3) == Id )
            return Slot;
    }

    return -1;
}

void Item::ModifyEnchantmentTime( uint32 Slot, uint32 Duration )
{
    EnchantmentMap::iterator itr = Enchantments.find( Slot );
    if( itr == Enchantments.end() )
        return;

    // Reset the apply time.
    itr->second.ApplyTime = UNIXTIME;
    itr->second.Duration = Duration;

    // Change the enchantment event timer.
//    if(m_owner) m_owner->UpdateItemEnchantDuration(this, Slot, Duration * 1000);

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
    for( itr = Enchantments.begin(); itr != Enchantments.end(); )
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
    for( itr = Enchantments.begin(); itr != Enchantments.end(); )
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
    for( itr = Enchantments.begin(); itr != Enchantments.end(); itr++ )
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

    for( itr = Enchantments.begin(); itr != Enchantments.end(); itr++ )
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
    EnchantmentMap::iterator itr = Enchantments.find( slot );
    if( itr != Enchantments.end() )
        return &itr->second;
    else
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

uint32 Item::GenerateRandomSuffixFactor( ItemPrototype* m_itemProto )
{
    double value;

    if( m_itemProto->Class == ITEM_CLASS_ARMOR && m_itemProto->Quality > ITEM_QUALITY_UNCOMMON_GREEN )
        value = SuffixMods[m_itemProto->InventoryType] * 1.24;
    else
        value = SuffixMods[m_itemProto->InventoryType];

    value = ( value * double( m_itemProto->ItemLevel ) ) + 0.5;
    return long2int32( value );
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
