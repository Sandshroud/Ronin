/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Item::Item() //this is called when constructing as container
{
    m_itemProto = NULL;
    m_owner = NULLPLR;
    locked = false;
    wrapped_item_id = 0;
    memset(Gems, 0, sizeof(uint32)*3);
    Enchantments.clear();
}

Item::Item( uint32 high, uint32 low )
{
    m_objectTypeId = TYPEID_ITEM;
    m_valuesCount = ITEM_END;
    m_uint32Values = _fields;
    memset( m_uint32Values, 0, (ITEM_END) * sizeof( uint32 ) );
    m_updateMask.SetCount(ITEM_END);
    SetUInt32Value( OBJECT_FIELD_TYPE,TYPEMASK_ITEM | TYPEMASK_OBJECT );
    SetUInt64Value( OBJECT_FIELD_GUID, MAKE_NEW_GUID(low, 0, high));
    m_wowGuid.Init( GetGUID() );

    SetFloatValue( OBJECT_FIELD_SCALE_X, 1 );//always 1

    m_itemProto = NULL;
    m_owner = NULLPLR;
    locked = false;
    m_isDirty = true;
    StatsApplied = false;
    random_prop = 0;
    random_suffix = 0;
    wrapped_item_id = 0;
    memset(Gems, 0, sizeof(uint32)*3);
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
    sEventMgr.RemoveEvents( this );

    EnchantmentMap::iterator itr;
    for( itr = Enchantments.begin(); itr != Enchantments.end(); itr++ )
    {
        if( itr->second.Enchantment->type == 0 && itr->second.Slot == 0 && itr->second.ApplyTime == 0 && itr->second.Duration == 0 )
        {
            delete itr->second.Enchantment;
            itr->second.Enchantment = NULL;
        }
    }

    if( IsInWorld() )
        RemoveFromWorld();

    m_owner = NULLPLR;
    Object::Destruct();
}

void Item::DeleteMe()
{
    if( IsContainer() )
        TO_CONTAINER(this)->Destruct();
    else
        Destruct();
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

    m_itemProto = ItemPrototypeStorage.LookupEntry( itemid );

    ASSERT( m_itemProto );

    for(uint8 i = 0; i < 5; i++)
        SetUInt32Value( ITEM_FIELD_SPELL_CHARGES+i, m_itemProto->Spells[i].Charges );

    SetUInt32Value(ITEM_FIELD_DURATION, abs((int)m_itemProto->Duration));
    SetUInt32Value(ITEM_FIELD_MAXDURABILITY, m_itemProto->MaxDurability );
    SetUInt32Value(ITEM_FIELD_DURABILITY, m_itemProto->MaxDurability );
    SetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME, 0);

    m_owner = owner;
    if( m_itemProto->LockId > 1 )
        locked = true;
    else
        locked = false;
}

char* GemReadFormat[3] =
{
    "0:%u;",
    "1:%u;",
    "2:%u;",
};

void Item::LoadFromDB(Field* fields, Player* plr, bool light )
{
    int32 count;
    uint32 itemid = fields[2].GetUInt32(), random_prop = fields[9].GetUInt32(), random_suffix = fields[10].GetUInt32();
    m_itemProto = ItemPrototypeStorage.LookupEntry( itemid );

    ASSERT( m_itemProto );

    if(m_itemProto->LockId > 1)
        locked = true;
    else
        locked = false;

    SetUInt32Value( OBJECT_FIELD_ENTRY, itemid );
    m_owner = plr;

    wrapped_item_id=fields[3].GetUInt32();
    m_uint32Values[ITEM_FIELD_GIFTCREATOR] = fields[4].GetUInt32();
    m_uint32Values[ITEM_FIELD_CREATOR] = fields[5].GetUInt32();

    count = fields[6].GetUInt32();
    if(m_itemProto->MaxCount > 0 && count > m_itemProto->MaxCount)
        count = m_itemProto->MaxCount;
    SetUInt32Value( ITEM_FIELD_STACK_COUNT, count);

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

    if( light )
        return;

    string enchant_field = fields[15].GetString();
    vector< string > enchants = StrSplit( enchant_field, ";" );
    uint32 enchant_id;
    EnchantEntry* entry;
    uint32 time_left;
    uint32 enchslot;
    uint32 dummy = 0;

    for( vector<string>::iterator itr = enchants.begin(); itr != enchants.end(); itr++ )
    {
        if( sscanf( (*itr).c_str(), "%u,%u,%u,%u", (unsigned int*)&enchant_id, (unsigned int*)&time_left, (unsigned int*)&enchslot, (unsigned int*)&dummy) > 3 )
        {
            entry = dbcEnchant.LookupEntry( enchant_id );
            if( entry && entry->Id == enchant_id )
            {
                AddEnchantment( entry, time_left, ( time_left == 0 ), false, false, enchslot, 0, ((dummy > 0) ? true : false) );
                //(enchslot != 2) ? false : true, false);
            }
        }
    }

    string gem_field = fields[16].GetString();
    for( uint8 k = 0; k < 3; k++ )
        sscanf( gem_field.c_str(), GemReadFormat[k], &Gems[k]);

    ApplyRandomProperties( false );

    Charter* charter = guildmgr.GetCharterByItemGuid(GetLowGUID());
    if(charter != NULL)
    {
        SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1, charter->GetID());

        // Charter stuff
        if(m_uint32Values[OBJECT_FIELD_ENTRY] == ITEM_ENTRY_GUILD_CHARTER)
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( plr->m_playerInfo->charterId[CHARTER_TYPE_GUILD] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_1_1, plr->m_playerInfo->charterId[CHARTER_TYPE_GUILD] );
        }

        if( m_uint32Values[OBJECT_FIELD_ENTRY] == ARENA_TEAM_CHARTER_2v2 )
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_2V2] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_1_1, plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_2V2] );
        }

        if( m_uint32Values[OBJECT_FIELD_ENTRY] == ARENA_TEAM_CHARTER_3v3 )
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_3V3] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_1_1, plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_3V3] );
        }

        if( m_uint32Values[OBJECT_FIELD_ENTRY] == ARENA_TEAM_CHARTER_5v5 )
        {
            SetUInt32Value( ITEM_FIELD_STACK_COUNT, 1 );
            SetUInt32Value( ITEM_FIELD_PROPERTY_SEED, 57813883 );
            if( plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_5V5] )
                SetUInt32Value( ITEM_FIELD_ENCHANTMENT_1_1, plr->m_playerInfo->charterId[CHARTER_TYPE_ARENA_5V5] );
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
                if( rp->spells[k] != 0 )
                {
                    EnchantEntry* ee = dbcEnchant.LookupEntry( rp->spells[k] );
                    Slot = HasEnchantment( ee->Id );
                    if( Slot < 0 )
                    {
                        Slot = FindFreeEnchantSlot( ee, 1 );
                        AddEnchantment( ee, 0, false, apply, true, Slot );
                    }
                    else
                        if( apply )
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
                    EnchantEntry* ee = dbcEnchant.LookupEntry( rs->enchantments[k] );
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

void Item::SaveToDB( int16 containerslot, int16 slot, bool firstsave, QueryBuffer* buf )
{
    if( !m_isDirty && !firstsave )
        return;

    std::stringstream ss;

    ss << "REPLACE INTO playeritems VALUES(";

    ss << m_uint32Values[ITEM_FIELD_OWNER] << ",";
    ss << m_uint32Values[OBJECT_FIELD_GUID] << ",";
    ss << m_uint32Values[OBJECT_FIELD_ENTRY] << ",";
    ss << wrapped_item_id << ",";
    ss << m_uint32Values[ITEM_FIELD_GIFTCREATOR] << ",";
    ss << m_uint32Values[ITEM_FIELD_CREATOR] << ",";

    ss << GetUInt32Value(ITEM_FIELD_STACK_COUNT) << ",";
    ss << (int32)GetChargesLeft() << ",";
    ss << GetUInt32Value(ITEM_FIELD_FLAGS) << ",";
    ss << random_prop << "," << random_suffix << ",";
    ss << GetTextID() << ",";
    ss << GetUInt32Value(ITEM_FIELD_DURABILITY) << ",";
    ss << static_cast<int>(containerslot) << ",";
    ss << static_cast<int>(slot) << ",'";

    // Pack together enchantment fields
    if( Enchantments.size() > 0 )
    {
        EnchantmentMap::iterator itr = Enchantments.begin();
        for(; itr != Enchantments.end(); itr++)
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

    ss << "','";
    if(Gems[0] || Gems[1] || Gems[2])
    {   // We get socket.
        for(uint32 g = 0; g < 3; g++)
        {
            // Socket screen turn on.
            if(Gems[g])
            {
                ss << g; // All your gem are belong to us.
                ss << ":" << Gems[g] << ";";
            }
        }
    }
    ss << "')";

    if( firstsave )
        CharacterDatabase.WaitExecute( ss.str().c_str() );
    else
    {
        if( buf == NULL )
            CharacterDatabase.Execute( ss.str().c_str() );
        else
            buf->AddQueryStr( ss.str() );
    }

    m_isDirty = false;
}

void Item::DeleteFromDB()
{
    if( m_itemProto->ContainerSlots > 0 && GetTypeId() == TYPEID_CONTAINER )
    {
        /* deleting a Container* */
        for( int32 i = 0; i < m_itemProto->ContainerSlots; i++ )
        {
            if( CAST(Container,this)->GetItem( i ) != NULL )
            {
                /* abort the delete */
                return;
            }
        }
    }

    CharacterDatabase.Execute( "DELETE FROM playeritems WHERE guid = %u", m_uint32Values[OBJECT_FIELD_GUID] );
}

uint32 Item::GetSkillByProto( uint32 Class, uint32 SubClass )
{
    if( Class == 4 && SubClass < 7 )
    {
        return arm_skills[SubClass];
    }
    else if( Class == 2 )
    {
        if( SubClass < 20 )//no skill for fishing
        {
            return weap_skills[SubClass];
        }
    }
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
        case SKILL_CLOTH:
            return &prof[0];
        case SKILL_LEATHER:
            return &prof[1];
        case SKILL_MAIL:
            return &prof[2];
        case SKILL_PLATE_MAIL:
            return &prof[3];
        case SKILL_SHIELD:
            return &prof[4];
        case SKILL_AXES:
            return &prof[5];
        case SKILL_2H_AXES:
            return &prof[6];
        case SKILL_BOWS:
            return &prof[7];
        case SKILL_GUNS:
            return &prof[8];
        case SKILL_MACES:
            return &prof[9];
        case SKILL_2H_MACES:
            return &prof[10];
        case SKILL_POLEARMS:
            return &prof[11];
        case SKILL_SWORDS:
            return &prof[12];
        case SKILL_2H_SWORDS:
            return &prof[13];
        case SKILL_STAVES:
            return &prof[14];
        case SKILL_FIST_WEAPONS:
            return &prof[15];
        case SKILL_DAGGERS:
            return &prof[16];
        case SKILL_THROWN:
            return &prof[17];
        case SKILL_SPEARS:
            return &prof[18];
        case SKILL_CROSSBOWS:
            return &prof[19];
        case SKILL_WANDS:
            return &prof[20];
        case SKILL_FISHING:
            return &prof[21];
        default:
            return NULL;
    }
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
        Standing plrstanding = plr->GetStandingRank( vendor->m_factionTemplate->Faction );
        cost = float2int32( ceilf( float( proto->BuyPrice ) * pricemod[plrstanding] ) );
    }

    return cost * count;
}

uint32 Item::GetSellPriceForItem( uint32 itemid, uint32 count )
{
    if( ItemPrototype* proto = ItemPrototypeStorage.LookupEntry( itemid ) )
        return Item::GetSellPriceForItem(proto, count);
    else
        return 1;
}

uint32 Item::GetBuyPriceForItem( uint32 itemid, uint32 count, Player* plr, Creature* vendor )
{
    if( ItemPrototype* proto = ItemPrototypeStorage.LookupEntry( itemid ) )
        return Item::GetBuyPriceForItem( proto, count, plr, vendor );
    else
        return 1;
}

void Item::RemoveFromWorld()
{
    // if we have an owner->send destroy
    if( m_owner != NULL )
    {
        DestroyForPlayer( m_owner );
    }

    if( !IsInWorld() )
        return;

    m_mapMgr->RemoveObject( TO_OBJECT(this), false );
    m_mapMgr = NULLMAPMGR;

    // update our event holder
    event_Relocate();
}

void Item::SetOwner( Player* owner )
{
    if( owner != NULL )
        SetUInt64Value( ITEM_FIELD_OWNER, owner->GetGUID() );
    else SetUInt64Value( ITEM_FIELD_OWNER, 0 );

    m_owner = owner;
}

int32 Item::AddEnchantment(EnchantEntry* Enchantment, uint32 Duration, bool Perm /* = false */, bool apply /* = true */, bool RemoveAtLogout /* = false */, uint32 Slot_, uint32 RandomSuffix, bool dummy /* = false */ )
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
    Enchantments.insert(make_pair((uint32)Slot, Instance));

    if( m_owner == NULL )
        return Slot;

    // Add the removal event.
    if( Duration )
        sEventMgr.AddEvent( this, &Item::RemoveEnchantment, uint32(Slot), EVENT_REMOVE_ENCHANTMENT1 + Slot, Duration * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );

    // No need to send the log packet, if the owner isn't in world (we're still loading)
    if( !m_owner->IsInWorld() )
        return Slot;

    if( apply )
    {
/*      WorldPacket EnchantLog( SMSG_ENCHANTMENTLOG, 25 );
        EnchantLog << m_owner->GetGUID();
        EnchantLog << m_owner->GetGUID();
        EnchantLog << m_uint32Values[OBJECT_FIELD_ENTRY];
        EnchantLog << Enchantment->Id;
        EnchantLog << uint8(0);
        m_owner->GetSession()->SendPacket( &EnchantLog );*/

        if( m_owner->GetTradeTarget() )
            m_owner->SendTradeUpdate();

        if(Instance.Dummy)
            return Slot;

        /* Only apply the enchantment bonus if we're equipped */
        uint8 slot = m_owner->GetItemInterface()->GetInventorySlotByGuid( GetGUID() );
        if( slot > EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END )
            ApplyEnchantmentBonus( Slot, APPLY );
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

    m_isDirty = true;
    uint32 Slot = itr->first;
    if( itr->second.BonusApplied )
        ApplyEnchantmentBonus( EnchantmentSlot, REMOVE );

    // Unset the item fields.
    uint32 EnchantBase = Slot * 3 + ITEM_FIELD_ENCHANTMENT_1_1;
    SetUInt32Value( EnchantBase + 0, 0 );
    SetUInt32Value( EnchantBase + 1, 0 );
    SetUInt32Value( EnchantBase + 2, 0 );

    // Remove the enchantment event for removal.
    event_RemoveEvents( EVENT_REMOVE_ENCHANTMENT1 + Slot );

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

    EnchantEntry* Entry = itr->second.Enchantment;
    uint32 RandomSuffixAmount = itr->second.RandomSuffix;

    if( itr->second.Dummy )
        return;

    if( itr->second.BonusApplied == Apply )
        return;

    itr->second.BonusApplied = Apply;

    // Apply the visual on the player.
    uint32 ItemSlot = m_owner->GetItemInterface()->GetInventorySlotByGuid( GetGUID() );
    if(ItemSlot < EQUIPMENT_SLOT_END && Slot < 1)
    {
        uint32 VisibleBase = PLAYER_VISIBLE_ITEM_1_ENCHANTMENT + ItemSlot * PLAYER_VISIBLE_ITEM_LENGTH;
        m_owner->SetUInt16Value( VisibleBase, Slot, Apply ? Entry->Id : 0 );
    }

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
                    if( Apply && Entry->spell[c] != 0 )
                    {
                        // Create a proc trigger spell
                        ProcTriggerSpell TS;
                        memset(&TS, 0, sizeof(ProcTriggerSpell));
                        TS.caster = m_owner->GetGUID();
                        TS.procFlags = PROC_ON_MELEE_ATTACK;
                        TS.origId = 0;
                        TS.procflags2 = 0;
                        TS.SpellClassMask[0] = 0;
                        TS.SpellClassMask[1] = 0;
                        TS.SpellClassMask[2] = 0;
                        TS.ProcType = 0;
                        TS.LastTrigger = 0;
                        TS.procValue = 0;
                        TS.procCharges = 0;
                        TS.procChance = 10;

                        if(ItemSlot == EQUIPMENT_SLOT_MAINHAND)
                            TS.weapon_damage_type = 1; // Proc only on main hand attacks
                        else if(ItemSlot == EQUIPMENT_SLOT_OFFHAND)
                            TS.weapon_damage_type = 2; // Proc only on off hand attacks
                        else
                            TS.weapon_damage_type = 0; // Doesn't depend on weapon

                        /* This needs to be modified based on the attack speed of the weapon.
                         * Secondly, need to assign some static chance for instant attacks (ss,
                         * gouge, etc.) */
                        if( GetProto()->Class == ITEM_CLASS_WEAPON )
                        {
                            if( !Entry->min[c] )
                            {
                                float speed = (float)GetProto()->Delay;

                                /////// procChance calc ///////
                                float ppm = 0;
                                SpellEntry* sp = dbcSpell.LookupEntry( Entry->spell[c] );
                                if( sp )
                                {
                                    switch( sp->NameHash )
                                    {
                                    case SPELL_HASH_FROSTBRAND_ATTACK:
                                        ppm = 9;
                                        break;
                                    }
                                }
                                if( ppm != 0 )
                                {
                                    float pcount = 60/ppm;
                                    float chance = (speed/10) / pcount;
                                    TS.procChance = (uint32)chance;
                                }
                                else
                                    TS.procChance = (uint32)( speed / 600.0f );
                                ///////////////////////////////
                            }
                            else
                                TS.procChance = Entry->min[c];
                        }

                        sLog.Debug( "Enchant", "Setting procChance to %u%%.", TS.procChance );
                        TS.deleted = false;
                        TS.spellId = Entry->spell[c];
                        m_owner->m_procSpells.push_back( TS );
                    }
                    else
                    {
                        // Remove the proctriggerspell
                        uint32 SpellId;
                        list< struct ProcTriggerSpell >::iterator itr/*, itr2*/;
                        for( itr = m_owner->m_procSpells.begin(); itr != m_owner->m_procSpells.end(); )
                        {
                            SpellId = itr->spellId;
                            /*itr2 = itr;*/
                            /*++itr;*/

                            if( SpellId == Entry->spell[c] )
                            {
                                //m_owner->m_procSpells.erase(itr2);
                                itr->deleted = true;
                            }
                            ++itr;
                        }
                    }
                }break;

            case 2:      // Mod damage done.
                {
                    int32 val = Entry->min[c];
                    if( RandomSuffixAmount )
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION( RandomSuffixAmount, GetItemRandomSuffixFactor() );

                    if( Apply )
                        m_owner->DamageDonePosMod[0] += val;
                    else
                        m_owner->DamageDonePosMod[0] -= val;
                    m_owner->UpdateStats();
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

                            Spell* spell = NULLSPELL;
                            //Never found out why,
                            //but this Blade of Life's Inevitability spell must be casted by the item, not owner.
                            if( m_itemProto->ItemId != 34349  )
                                spell = (new Spell( m_owner, sp, true, NULLAURA ));
                            else
                                spell = (new Spell( TO_ITEM(this), sp, true, NULLAURA ));

                            spell->i_caster = TO_ITEM(this);
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
                    int32 val = Entry->min[c];
                    if( RandomSuffixAmount )
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION( RandomSuffixAmount, GetItemRandomSuffixFactor() );

                    if( Apply )
                        m_owner->FlatResistanceModifierPos[Entry->spell[c]] += val;
                    else
                        m_owner->FlatResistanceModifierPos[Entry->spell[c]] -= val;
                    m_owner->CalcResistance( Entry->spell[c] );
                }break;

            case 5:  //Modify rating ...order is PLAYER_FIELD_COMBAT_RATING_1 and above
                {
                    //spellid is enum ITEM_STAT_TYPE
                    //min=max is amount
                    int32 val = Entry->min[c];
                    if( RandomSuffixAmount )
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION( RandomSuffixAmount, GetItemRandomSuffixFactor() );

                    m_owner->ModifyBonuses( Entry->spell[c], Apply ? val : -val );
                }break;

            case 6:  // Rockbiter weapon (increase damage per second... how the hell do you calc that)
                {
                    if( Apply )
                    {
                        //if i'm not wrong then we should apply DMPS formula for this. This will have somewhat a larger value 28->34
                        int32 val = Entry->min[c];
                        if( RandomSuffixAmount )
                            val = RANDOM_SUFFIX_MAGIC_CALCULATION( RandomSuffixAmount, GetItemRandomSuffixFactor() );

                        //int32 value = GetProto()->Delay * val / 1000;
                        m_owner->DamageDonePosMod[0] += val;
                    }
                    else
                    {
                        int32 val = Entry->min[c];
                        if( RandomSuffixAmount )
                            val = RANDOM_SUFFIX_MAGIC_CALCULATION( RandomSuffixAmount, GetItemRandomSuffixFactor() );

                        //int32 value =- (int32)(GetProto()->Delay * val / 1000 );
                        m_owner->DamageDonePosMod[0] += val;
                    }
                    m_owner->UpdateStats();
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
            ApplyEnchantmentBonus( itr2->first, APPLY );
    }

    uint32 value = 0;
    ItemPrototype* proto = NULL;
    for(uint32 gem = 0; gem < 3; gem++)
    {
        if(!Gems[gem])
            continue;

        proto = ItemPrototypeStorage.LookupEntry(Gems[gem]);
        if(proto == NULL)
            continue;

        if(proto->Armor)
        {
            m_owner->BaseResistance[RESISTANCE_ARMOR] += proto->Armor;
            m_owner->CalcResistance(RESISTANCE_ARMOR);
        }

        for(uint32 i = 0; i < 10; i++)
        {
            if(value = proto->Stats[i].Value)
                m_owner->ModifyBonuses( proto->Stats[i].Type, value );
        }
        proto = NULL;
    }
}

void Item::RemoveEnchantmentBonuses()
{
    EnchantmentMap::iterator itr, itr2;
    for( itr = Enchantments.begin(); itr != Enchantments.end(); )
    {
        itr2 = itr++;
        if(!itr2->second.Dummy)
            ApplyEnchantmentBonus( itr2->first, REMOVE );
    }

    int32 value = 0;
    ItemPrototype* proto = NULL;
    for(uint32 gem = 0; gem < 3; gem++)
    {
        if(!Gems[gem])
            continue;

        proto = ItemPrototypeStorage.LookupEntry(Gems[gem]);
        if(proto == NULL)
            continue;

        if(proto->Armor)
        {
            m_owner->BaseResistance[RESISTANCE_ARMOR] -= proto->Armor;
            m_owner->CalcResistance(RESISTANCE_ARMOR);
        }

        for(uint32 i = 0; i < 10; i++)
        {
            if(value = proto->Stats[i].Value)
                m_owner->ModifyBonuses( proto->Stats[i].Type, -value );
        }
        proto = NULL;
    }
}

void Item::EventRemoveEnchantment( uint32 Slot )
{
    // Remove the enchantment.
    RemoveEnchantment( Slot );
}

int32 Item::FindFreeEnchantSlot( EnchantEntry* Enchantment, uint32 random_type )
{
    uint32 GemSlotsReserve = GetMaxSocketsCount();
    if( GetProto()->SocketBonus )
        GemSlotsReserve++;

    if( random_type == 1 )      // random prop
    {
        for( uint32 Slot = 8; Slot < 11; ++Slot )
            if( m_uint32Values[ITEM_FIELD_ENCHANTMENT_1_1 + Slot * 3] == 0 )
                return Slot;
    }
    else if( random_type == 2 ) // random suffix
    {
        for( uint32 Slot = 6; Slot < 11; ++Slot )
            if( m_uint32Values[ITEM_FIELD_ENCHANTMENT_1_1 + Slot * 3] == 0 )
                return Slot;
    }

    for( uint32 Slot = GemSlotsReserve + 2; Slot < 11; Slot++ )
    {
        if( m_uint32Values[ITEM_FIELD_ENCHANTMENT_1_1 + Slot * 3] == 0 )
            return Slot;
    }

    return -1;
}

int32 Item::HasEnchantment( uint32 Id )
{
    for( uint32 Slot = 0; Slot < 11; Slot++ )
    {
        if( m_uint32Values[ITEM_FIELD_ENCHANTMENT_1_1 + Slot * 3] == Id )
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

    // Change the event timer.
    event_ModifyTimeAndTimeLeft( EVENT_REMOVE_ENCHANTMENT1 + Slot, Duration * 1000 );

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

void Item::RemoveRelatedEnchants( EnchantEntry* newEnchant )
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
            ItemPrototype* ip = ItemPrototypeStorage.LookupEntry(ei->Enchantment->GemEntry);
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

bool Item::IsGemRelated( EnchantEntry* Enchantment )
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
static const char *g_itemQualityColours[15] = {
    "|cff9d9d9d",       // Grey
    "|cffffffff",       // White
    "|cff1eff00",       // Green
    "|cff0070dd",       // Blue
    "|cffa335ee",       // Purple
    "|cffff8000",       // Orange
    "|cffe6cc80",       // Artifact
    "|cffe5cc80",       // Heirloom
    "|cff00ffff",       // Turquoise
    "|cff00ffff",       //
    "|cff00ffff",       //
    "|cff00ffff",       //
    "|cff00ffff",       //
    "|cff00ffff",       //
    "|cff00ffff",       //
};

string ItemPrototype::ConstructItemLink(uint32 random_prop, uint32 random_suffix, uint32 stack)
{
    if( Quality > 15 )
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
    snprintf(buf, 1000, "%s|Hitem:%u:0:0:0:0:0:%d:0|h[%s%s%s]%s|h|r", g_itemQualityColours[Quality], ItemId, /* suffix/prop */ random_suffix ? (-(int32)random_suffix) : random_prop,
        Name1, rstxt, rptxt, sbuf);

    return string(buf);
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
