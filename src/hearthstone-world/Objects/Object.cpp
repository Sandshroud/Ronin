/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Object::Object(uint64 guid) : m_updateMask(), m_notifyFlags(0), m_objectUpdated(false)
{
    m_valuesCount = OBJECT_LENGTH;
    m_updateMask.SetCount(OBJECT_END);
    m_raw.values[OBJECT_LAYER_OBJECT] = new uint32[OBJECT_LENGTH];
    memset(m_raw.values[OBJECT_LAYER_OBJECT], 0, OBJECT_LENGTH*sizeof(uint32));
    m_object.m_wowGUID = guid;
    m_object.m_objType = TYPEMASK_TYPE_OBJECT;
    m_object.m_objScale = 1.f;

    m_loot.gold = 0;
    m_looted = false;
}

void Object::SetByte(uint32 index, uint32 index1,uint8 value)
{
    ASSERT( index < m_valuesCount );

    if(index1 >= 4)
    {
        sLog.outError("WorldObject::SetByteValue: wrong offset %u", index1);
        return;
    }

    if(uint8(m_uint32.values[ index ] >> (index1 * 8)) == value)
        return;

    m_uint32.values[ index ] &= ~uint32(uint32(0xFF) << (index1 * 8));
    m_uint32.values[ index ] |= uint32(uint32(value) << (index1 * 8));
    SetUpdateField(index);
}

void Object::SetByteFlag(const uint32 index, const uint32 flag, uint8 newFlag)
{
    if( HasByteFlag(index,flag,newFlag))
        return;

    SetByte(index, flag, GetByte(index,flag)|newFlag);
    SetUpdateField(index);
}

void Object::RemoveByteFlag(const uint32 index, const uint32 flag, uint8 checkFlag)
{
    if( !HasByteFlag(index,flag,checkFlag))
        return;

    SetByte(index,flag, GetByte(index,flag) & ~checkFlag );
    SetUpdateField(index);
}

bool Object::HasByteFlag(const uint32 index, const uint32 flag, uint8 checkFlag)
{
    if( GetByte(index,flag) & checkFlag )
        return true;
    return false;
}

void Object::SetUInt16Value(uint16 index, uint8 offset, uint16 value)
{
    ASSERT( index < m_valuesCount );
    if (uint16(m_uint32.values[index] >> (offset * 16)) == value)
        return;

    m_uint32.values[index] &= ~uint32(uint32(0xFFFF) << (offset * 16));
    m_uint32.values[index] |= uint32(uint32(value) << (offset * 16));
    SetUpdateField(index);
}

void Object::SetUInt32Value( const uint32 index, const uint32 value )
{
    if(index > m_valuesCount)
        printf("Index: %u, m_valuesCount: %u, Value: %u Test:%s\n", index, m_valuesCount, value, __FUNCTION__);

    ASSERT( index < m_valuesCount );
    if(m_uint32.values[index] == value)
        return;

    m_uint32.values[ index ] = value;
    SetUpdateField(index);
}

void Object::SetUInt64Value( const uint32 index, const uint64 value )
{
    assert( index + 1 < m_valuesCount );
    if(m_uint32.values[index] == GUID_LOPART(value) && m_uint32.values[index+1] == GUID_HIPART(value))
        return;

    m_uint32.values[ index ] = *((uint32*)&value);
    m_uint32.values[ index + 1 ] = *(((uint32*)&value) + 1);

    SetUpdateField(index);
    SetUpdateField(index+1);
}

void Object::SetFlag( const uint32 index, uint32 newFlag )
{
    ASSERT( index < m_valuesCount );
    if((m_uint32.values[ index ] & newFlag) == newFlag)
        return;

    m_uint32.values[ index ] |= newFlag;
    SetUpdateField(index);
}

void Object::RemoveFlag( const uint32 index, uint32 oldFlag )
{
    ASSERT( index < m_valuesCount );
    if((m_uint32.values[ index ] & oldFlag) == 0)
        return;

    m_uint32.values[ index ] &= ~oldFlag;
    SetUpdateField(index);
}

void Object::SetFloatValue( const uint32 index, const float value )
{
    ASSERT( index < m_valuesCount );
    if(m_float.values[index] == value)
        return;

    m_float.values[ index ] = value;
    SetUpdateField(index);
}

void Object::ModFloatValue(const uint32 index, const float value )
{
    ASSERT( index < m_valuesCount );
    m_float.values[ index ] += value;
    SetUpdateField(index);
}

void Object::ModSignedInt32Value(uint32 index, int32 value )
{
    ASSERT( index < m_valuesCount );
    if(value == 0)
        return;

    m_uint32.values[ index ] += value;
    SetUpdateField(index);
}

void Object::ModUnsigned32Value(uint32 index, int32 mod)
{
    ASSERT( index < m_valuesCount );
    if(mod == 0)
        return;

    m_uint32.values[ index ] += mod;
    if( (int32)m_uint32.values[index] < 0 )
        m_uint32.values[index] = 0;
    SetUpdateField(index);
}

uint32 Object::GetModPUInt32Value(const uint32 index, const int32 value)
{
    ASSERT( index < m_valuesCount );
    int32 basevalue = (int32)m_uint32.values[ index ];
    return ((basevalue*value)/100);
}

void Object::SetUpdateField(uint32 index)
{
    m_updateMask.SetBit( index );
    OnFieldUpdated(index);
}

////////////////////////////////////////////////////////////////////////////
/// Fill the object's Update Values from a space deliminated list of values.
void Object::LoadValues(const char* data)
{
    // thread-safe ;) strtok is not.
    std::string ndata = data;
    std::string::size_type last_pos = 0, pos = 0;
    uint32 index = 0;
    uint32 val;
    do
    {
        // prevent overflow
        if(index >= m_valuesCount)
        {
            break;
        }
        pos = ndata.find(" ", last_pos);
        val = atol(ndata.substr(last_pos, (pos-last_pos)).c_str());
        if(m_uint32.values[index] == 0)
            m_uint32.values[index] = val;
        last_pos = pos+1;
        ++index;
    } while(pos != std::string::npos);
}

void Object::_SetUpdateBits(UpdateMask *updateMask, Player* target) const
{
    *updateMask = m_updateMask;
}

void Object::_SetCreateBits(UpdateMask *updateMask, Player* target) const
{
    for(uint32 i = 0; i < m_valuesCount; i++)
        if(m_uint32.values[i] != 0)
            updateMask->SetBit(i);
}

uint32 Object::BuildCreateUpdateBlockForPlayer(ByteBuffer *data, Player* target)
{
    uint16 updateFlags = UPDATEFLAG_NONE;
    uint8 updatetype = UPDATETYPE_CREATE_OBJECT;
    if(m_object.m_objType & TYPEMASK_TYPE_UNIT)
    {
        updateFlags |= UPDATEFLAG_LIVING;
        updatetype = UPDATETYPE_CREATE_PLAYEROBJ;
    }
    else if(m_object.m_objType & TYPEMASK_TYPE_GAMEOBJECT)
    {
        updateFlags = UPDATEFLAG_STATIONARY_POS|UPDATEFLAG_ROTATION;
        switch(GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID))
        {
        case GAMEOBJECT_TYPE_TRANSPORT:
        case GAMEOBJECT_TYPE_MO_TRANSPORT:
            {
                updateFlags |= UPDATEFLAG_DYN_MODEL|UPDATEFLAG_TRANSPORT;
            }break;
        case GAMEOBJECT_TYPE_TRAP:
        case GAMEOBJECT_TYPE_DUEL_ARBITER:
        case GAMEOBJECT_TYPE_FLAGSTAND:
        case GAMEOBJECT_TYPE_FLAGDROP:
            {
                // duel flags have to stay as updatetype 3, otherwise
                // it won't animate
                updatetype = UPDATETYPE_CREATE_PLAYEROBJ;
            }break;
        }
    } else updateFlags |= UPDATEFLAG_STATIONARY_POS;
    if(IsVehicle()) updateFlags |= UPDATEFLAG_VEHICLE;
    if(target == this)
    {
        // player creating self
        updateFlags |= UPDATEFLAG_SELF;
        updatetype = UPDATETYPE_CREATE_PLAYEROBJ;
    }

    if(IsUnit())
    {
        if (castPtr<Unit>(this)->GetUInt64Value(UNIT_FIELD_TARGET))
            updateFlags |= UPDATEFLAG_HAS_TARGET;
    }

    // build our actual update
    *data << updatetype;
    *data << m_object.m_wowGUID.asPacked();
    *data << GetTypeId();

    _BuildMovementUpdate(data, updateFlags, target);

    // this will cache automatically if needed
    _BuildCreateValuesUpdate( data, target );

    // update count: 1 ;)
    return 1;
}

//=======================================================================================
//  Creates an update block containing all data needed for a new object
//=======================================================================================
void Object::_BuildCreateValuesUpdate(ByteBuffer * data, Player* target)
{
    ByteBuffer fields;
    UpdateMask mask(m_valuesCount);
    uint32 *flags, uFlag = GetUpdateFieldData(flags, target);
    for( uint32 index = 0; index < m_valuesCount; index++ )
    {
        if(uint32 value = m_uint32.values[index])
        {
            if(m_notifyFlags & flags[index] || (m_uint32.values[index] && flags[index] & uFlag))
            {
                mask.SetBit(index);
                fields << value;
            }
        }
    }

    uint32 byteCount = mask.GetUpdateBlockCount();
    *data << uint8(byteCount);
    data->append( mask.GetMask(), byteCount*4 );
    data->append(fields);
}

uint32 Object::GetUpdateFieldData(uint32*& flags, Player * target)
{
    uint32 visibleFlag = UF_FLAG_PUBLIC, pos = 0;
    if (target == this)
        visibleFlag |= UF_FLAG_PRIVATE;

    switch (GetTypeId())
    {
    case TYPEID_ITEM:
    case TYPEID_CONTAINER:
        {
            flags = ItemUpdateFieldFlags;
            if (castPtr<Item>(this)->GetOwnerGUID() == (uint64)target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER | UF_FLAG_ITEM_OWNER;
        }break;
    case TYPEID_UNIT:
    case TYPEID_PLAYER:
        {
            flags = UnitUpdateFieldFlags;
            if (castPtr<Unit>(this)->GetGUIDValue(UNIT_FIELD_SUMMONEDBY) == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
            else if (castPtr<Unit>(this)->GetGUIDValue(UNIT_FIELD_CREATEDBY) == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
            if (IsPlayer() && castPtr<Player>(this)->InGroup() && castPtr<Player>(this)->GetGroupID() == target->GetGroupID())
                visibleFlag |= UF_FLAG_PARTY_MEMBER;
        }break;
    case TYPEID_GAMEOBJECT:
        {
            flags = GameObjectUpdateFieldFlags;
            if (castPtr<GameObject>(this)->GetGUIDValue(GAMEOBJECT_FIELD_CREATED_BY) == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
        }break;
    case TYPEID_DYNAMICOBJECT:
        {
            flags = DynamicObjectUpdateFieldFlags;
            if (castPtr<DynamicObject>(this)->GetCasterGuid() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
        }break;
    case TYPEID_CORPSE:
        {
            flags = CorpseUpdateFieldFlags;
            if (castPtr<Corpse>(this)->GetGUIDValue(CORPSE_FIELD_OWNER) == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
        }break;
    case TYPEID_AREATRIGGER:
        flags = AreaTriggerUpdateFieldFlags;
        break;
    case TYPEID_OBJECT:
        break;
    }

    return visibleFlag;
}

///////////////////////////////////////////////////////////////
/// Build the Movement Data portion of the update packet
/// Fills the data with this object's movement/speed info
void Object::_BuildMovementUpdate(ByteBuffer * data, uint16 flags, Player* target )
{
    ByteBuffer bytes;
    uint32 stopFrameCount = 0;
    for(uint32 i = 0; i < stopFrameCount; i++)
        bytes << uint32(0);
    data->WriteBit(0);
    data->WriteBit(0);
    data->WriteBit(flags & UPDATEFLAG_ROTATION);
    data->WriteBit(flags & UPDATEFLAG_ANIMKITS);
    data->WriteBit(flags & UPDATEFLAG_HAS_TARGET);
    data->WriteBit(flags & UPDATEFLAG_SELF);
    data->WriteBit(flags & UPDATEFLAG_VEHICLE);
    data->WriteBit(flags & UPDATEFLAG_LIVING);
    data->WriteBits(stopFrameCount, 24);
    data->WriteBit(0);
    data->WriteBit(flags & UPDATEFLAG_GO_TRANSPORT_POS);
    data->WriteBit(flags & UPDATEFLAG_STATIONARY_POS);
    data->WriteBit(flags & UPDATEFLAG_UNK5);
    data->WriteBit(0);
    data->WriteBit(flags & UPDATEFLAG_TRANSPORT);

    if(flags & UPDATEFLAG_LIVING)
        _WriteLivingMovementUpdate(data, &bytes, target);

    if(flags & UPDATEFLAG_VEHICLE)
        bytes << float(TO_VEHICLE(this)->GetOrientation()) << TO_VEHICLE(this)->GetVehicleEntry();

    if(flags & UPDATEFLAG_ROTATION)
    {
        uint64 rotation = 0;
        if(IsGameObject()) rotation = castPtr<GameObject>(this)->m_rotation;
        bytes << uint64(rotation); //blizz 64bit rotation
    }

    if (flags & UPDATEFLAG_STATIONARY_POS)
        _WriteStationaryPosition(data, &bytes, target);

    if(flags & UPDATEFLAG_HAS_TARGET)
    {
        WoWGuid targetGuid = WoWGuid::ConstructWoWGUID(GetUInt64Value(UNIT_FIELD_TARGET)); // Compressed target guid.
        data->WriteBitString(4, targetGuid[2], targetGuid[7], targetGuid[0], targetGuid[4]);
        data->WriteBitString(4, targetGuid[5], targetGuid[6], targetGuid[1], targetGuid[3]);
        bytes.WriteByteSeq(targetGuid[4]);
        bytes.WriteByteSeq(targetGuid[0]);
        bytes.WriteByteSeq(targetGuid[3]);
        bytes.WriteByteSeq(targetGuid[5]);
        bytes.WriteByteSeq(targetGuid[7]);
        bytes.WriteByteSeq(targetGuid[6]);
        bytes.WriteByteSeq(targetGuid[2]);
        bytes.WriteByteSeq(targetGuid[1]);
    }

    if (flags & UPDATEFLAG_ANIMKITS)
    {
        data->WriteBitString(3, 1, 1, 1); // Missing Anim kits 1,2,3
        if(false) bytes << uint16(0); // AnimKit1
        if(false) bytes << uint16(0); // AnimKit2
        if(false) bytes << uint16(0); // AnimKit3
    }

    if(flags & UPDATEFLAG_TRANSPORT)
    {
        if(IsTransport())
            bytes << TO_TRANSPORT(this)->m_timer;
        else bytes << (uint32)getMSTime();
    }
    data->append(bytes.contents(), bytes.size());
}

uint32 Object::BuildValuesUpdateBlockForPlayer(ByteBuffer *data, Player* target)
{
    UpdateMask updateMask;
    updateMask.SetCount( m_valuesCount );
    _SetUpdateBits( &updateMask, target );
    for(uint32 x = 0; x < m_valuesCount; ++x)
    {
        if(updateMask.GetBit(x))
        {
            *data << (uint8) UPDATETYPE_VALUES;     // update type == update
            *data << m_object.m_wowGUID;

            _BuildChangedValuesUpdate( data, &updateMask, target );
            return 1;
        }
    }

    return 0;
}

uint32 Object::BuildValuesUpdateBlockForPlayer(ByteBuffer * buf, UpdateMask * mask )
{
    // returns: update count
    *buf << (uint8) UPDATETYPE_VALUES;      // update type == update
    *buf << m_object.m_wowGUID;

    _BuildChangedValuesUpdate( buf, mask, NULL );

    // 1 update.
    return 1;
}

//=======================================================================================
//  Creates an update block with the values of this object as
//  determined by the updateMask.
//=======================================================================================
void Object::_BuildChangedValuesUpdate(ByteBuffer * data, UpdateMask *updateMask, Player* target)
{
    int32 DummyFlags = -1, DummyFlags2 = -1, DummyNpcFlags = -1, DummyDynFlags = -1;
    if(updateMask->GetBit(OBJECT_FIELD_GUID) && target)    // We're creating.
    {
        if(IsUnit())
        {
            DummyFlags = GetUInt32Value(UNIT_FIELD_FLAGS);
            DummyFlags2 = GetUInt32Value(UNIT_FIELD_FLAGS_2);
            DummyNpcFlags = GetUInt32Value(UNIT_NPC_FLAGS);
            DummyDynFlags = GetUInt32Value(UNIT_DYNAMIC_FLAGS);
            if(IsCreature())       // tagged group will have tagged player
            {
                Creature* cThis = castPtr<Creature>(this);
                if(cThis->m_taggingPlayer)
                {
                    // set tagged visual
                    if( (cThis->m_taggingGroup != 0 && target->m_playerInfo->m_Group != NULL && target->m_playerInfo->m_Group->GetID() == cThis->m_taggingGroup) ||
                        (cThis->m_taggingPlayer == target->GetLowGUID()) )
                    {
                        DummyDynFlags |= U_DYN_FLAG_TAPPED_BY_PLAYER;
                        if( cThis->m_loot.HasLoot(target) )
                            DummyDynFlags |= U_DYN_FLAG_LOOTABLE;
                    } else DummyDynFlags |= U_DYN_FLAG_TAGGED_BY_OTHER;
                }

                if(Trainer * pTrainer = cThis->GetTrainer())
                    if(!CanTrainAt(target, pTrainer))
                        DummyNpcFlags &= ~(UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_TRAINER_PROF | UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_ARMORER);
                if(cThis->IsVehicle() && sFactionSystem.isAttackable(target, cThis, false))
                    DummyNpcFlags &= ~(UNIT_NPC_FLAG_VEHICLE_MOUNT);
            }

            updateMask->SetBit(UNIT_NPC_FLAGS);
            updateMask->SetBit(UNIT_FIELD_FLAGS);
            updateMask->SetBit(UNIT_FIELD_FLAGS_2);
            updateMask->SetBit(UNIT_DYNAMIC_FLAGS);
        }
        else if(IsGameObject())
        {
            GameObject* go = castPtr<GameObject>(this);
            DummyFlags = GetUInt32Value(GAMEOBJECT_FLAGS);
            DummyDynFlags = GetUInt32Value(GAMEOBJECT_DYNAMIC);
            if(GameObjectInfo *info = go->GetInfo())
            {
                if(set<uint32>* involvedquestids = objmgr.GetInvolvedQuestIds(info->ID))
                {
                    for(set<uint32>::iterator itr = involvedquestids->begin(); itr != involvedquestids->end(); itr++)
                    {
                        if( target->GetQuestLogForEntry(*itr) != NULL )
                        {
                            DummyDynFlags = GO_DYNFLAG_QUEST;
                            break;
                        }
                    }
                }
            }

            updateMask->SetBit(GAMEOBJECT_FLAGS);
            updateMask->SetBit(GAMEOBJECT_DYNAMIC);
        }
    }

    WPAssert( updateMask && updateMask->GetCount() == m_valuesCount );
    uint32 byteCount = updateMask->GetUpdateBlockCount();
    uint32 valueCount = (uint32)std::min(byteCount*32, m_valuesCount);

    *data << (uint8)byteCount;
    data->append( updateMask->GetMask(), byteCount*4 );
    for( uint32 index = 0; index < valueCount; index++ )
    {
        if( updateMask->GetBit( index ) )
        {
            uint32 value = m_uint32.values[index];
            if(IsGameObject())
            {
                if(index == GAMEOBJECT_FLAGS)
                {
                    if(DummyFlags > 0 && value != DummyFlags)
                        value = DummyFlags;
                }
                else if(index == GAMEOBJECT_DYNAMIC)
                {
                    if(DummyDynFlags > 0 && value != DummyDynFlags)
                        value = DummyDynFlags;
                }
            }
            else if(IsUnit())
            {
                if(index == UNIT_FIELD_FLAGS)
                {
                    if(DummyFlags > 0 && value != DummyFlags)
                        value = DummyFlags;
                }
                else if(index == UNIT_FIELD_FLAGS_2)
                {
                    if(DummyFlags2 > 0 && value != DummyFlags2)
                        value = DummyFlags2;
                }
                else if(index == UNIT_DYNAMIC_FLAGS)
                {
                    if(DummyDynFlags > 0 && value != DummyDynFlags)
                        value = DummyDynFlags;
                }
                else if(index == UNIT_NPC_FLAGS)
                {
                    if(DummyNpcFlags > 0 && value != DummyNpcFlags)
                        value = DummyNpcFlags;
                }
            }
            *data << value;
        }
    }
}

void Object::DestroyForPlayer(Player* target, bool anim)
{
    if(target == NULL)
        return;

    WorldPacket data(SMSG_DESTROY_OBJECT, 9);
    data << GetGUID();
    data << uint8(anim ? 1 : 0);
    target->GetSession()->SendPacket( &data );
}

void Object::ClearLoot()
{
    // better cancel any rolls just in case.
    for(vector<__LootItem>::iterator itr = m_loot.items.begin(); itr != m_loot.items.end(); itr++)
    {
        if( itr->roll != NULL )
        {
            sEventMgr.RemoveEvents(itr->roll);
            itr->roll = NULLROLL; // buh-bye!
        }
    }

    m_loot.gold = 0;
    m_loot.items.clear();
    m_loot.looters.clear();
}

//===============================================
// WorldObject class functions
//===============================================
WorldObject::WorldObject(uint64 guid) : Object(guid), m_position(0,0,0,0), m_spawnLocation(0,0,0,0)
{
    m_mapId = -1;
    m_areaId = 0;
    m_zoneId = 0;
    m_areaFlags = 0;
    m_lastMovementZone = 0;

    m_objectUpdated = false;

    //official Values
    m_walkSpeed = 2.5f;
    m_runSpeed = 8.0f;
    m_base_runSpeed = m_runSpeed;
    m_base_walkSpeed = m_walkSpeed;

    m_flySpeed = 7.0f;
    m_pitchRate = 3.141593f;
    m_backFlySpeed = 4.5f;

    m_backWalkSpeed = 4.5f; // this should really be named m_backRunSpeed
    m_swimSpeed = 4.722222f;
    m_backSwimSpeed = 2.5f;
    m_turnRate = 3.141593f;

    m_mapMgr = NULLMAPMGR;
    m_mapCell = 0;

    m_factionTemplate = NULL;

    m_instanceId = 0;
    Active = false;
    m_loadedFromDB = false;

    m_unitsInRange.clear();
    m_objectsInRange.clear();
    m_inRangePlayers.clear();
    m_oppFactsInRange.clear();
}

WorldObject::~WorldObject( )
{
    Object::~Object();
}

void WorldObject::Init()
{

}

void WorldObject::Destruct()
{
    if(IsInWorld())
        RemoveFromWorld(false);

    if(GetMapCell())
    {
        // Remove object from cell
        GetMapCell()->RemoveObject(this);

        // Unset object's cell
        SetMapCell(NULL);
    }

    ClearInRangeSet();

    // for linux
    m_instanceId = -1;
    m_objectTypeMask = TYPEMASK_TYPE_UNUSED;
    sEventMgr.RemoveEvents(this);
    delete this;
}

//That is dirty fix it actually creates update of 1 field with
//the given value ignoring existing changes in fields and so on
//usefull if we want update this field for certain players
//NOTE: it does not change fields. This is also very fast method
WorldPacket *WorldObject::BuildFieldUpdatePacket(uint32 index, uint32 value)
{
    WorldPacket * packet = new WorldPacket(SMSG_UPDATE_OBJECT, 1500);
    *packet << uint16(GetMapId());
    *packet << uint32(1);//number of update/create blocks
    BuildFieldUpdatePacket(packet, index, value);
    return packet;
}

void WorldObject::BuildFieldUpdatePacket(Player* Target, uint32 Index, uint32 Value)
{
    ByteBuffer buf(500);
    BuildFieldUpdatePacket(&buf, Index, Value);
    Target->PushUpdateBlock(&buf, 1);
}

void WorldObject::BuildFieldUpdatePacket(ByteBuffer * buf, uint32 Index, uint32 Value)
{
    *buf << uint8(UPDATETYPE_VALUES);
    *buf << GetGUID();

    uint32 mBlocks = Index/32+1;
    *buf << (uint8)mBlocks;

    for(uint32 dword_n = mBlocks-1; dword_n; dword_n--)
        *buf <<(uint32)0;

    *buf <<(((uint32)(1))<<(Index%32));
    *buf << Value;
}

/* Crow:
    This function is a really heavy height check that is used for getting all types of height checks!
    Since it is in the object class, we can skip any actual variables and instead use our current position.
    This function gets map height and also checks the position of WMO's so that the height is set to the WMO height
    based on the Z position that is given. If no Z position is given, but x and y positions are given, we will instead
    use basic map heights as our Z position. */
float WorldObject::GetCHeightForPosition(bool checkwater, float x, float y, float z)
{
    if(!IsInWorld())
        return 0.0f;

    MapMgr* mgr = GetMapMgr();
    float offset = 0.12156f;
    if(x == 0.0f && y == 0.0f)
    {
        x = GetPositionX();
        y = GetPositionY();
    }

    if(z == 0.0f)
        z = GetPositionZ();

    if(!mgr->GetBaseMap() || !mgr->GetBaseMap()->GetMapTerrain())
        return z;

    float waterheight = mgr->GetWaterHeight(x, y, z);
    float mapheight = mgr->GetLandHeight(x, y);
    if(!mgr->CanUseCollision(this))
    {
        if(checkwater && waterheight != NO_WATER_HEIGHT)
            if(waterheight > mapheight)
                return waterheight+offset;
        return mapheight+offset;
    }

    float vmapheight = sVMapInterface.GetHeight(GetMapId(), GetInstanceID(), 0, x, y, z);
    if(IS_INSTANCE(mgr->GetMapId()) || !sWorld.CalculatedHeightChecks)
    {
        if(vmapheight != NO_WMO_HEIGHT)
        {
            if(checkwater && waterheight != NO_WATER_HEIGHT)
                if(waterheight > vmapheight)
                    return waterheight+offset;
            return vmapheight+offset;
        }

        if(checkwater && waterheight != NO_WATER_HEIGHT)
            if(waterheight > mapheight)
                return waterheight+offset;
        return mapheight+offset;
    }

    float phx = 0.0f;
    float phy = 0.0f;
    float phz = 0.0f;
    float CMapHeight = NO_LAND_HEIGHT;
    sVMapInterface.GetFirstPoint(GetMapId(), GetInstanceID(), GetPhaseMask(), x, y, z-10.0f, x, y, z+10.0f, phx, phy, CMapHeight, 0.0f);

    // Mapheight first.
    if(mapheight != NO_LAND_HEIGHT)
    {
        if(mapheight-2.0f < z)
        {
            if(mapheight+2.0f > z) // Accurate map height
            {
                if(checkwater && waterheight != NO_WATER_HEIGHT)
                    if(waterheight > mapheight)
                        return waterheight+offset;
                return mapheight+offset;
            }

            if(!sVMapInterface.GetFirstPoint(GetMapId(), GetInstanceID(), GetPhaseMask(), x, y, mapheight, x, y, z, phx, phy, phz, 0.0f))
            {
                if(checkwater && waterheight != NO_WATER_HEIGHT)
                    if(waterheight > mapheight)
                        return waterheight+offset;
                return mapheight+offset; // No breaks inbetween us, so its the right height, we're just a bunch of fuckers!
            }

            // TODO
        }
        else if(mapheight-10.0f > z)
        {
            // TODO
        }
    }

    // Now Vmap Height
    if(vmapheight != NO_WMO_HEIGHT)
    {
        if(vmapheight-2.0f < z)
        {
            if(vmapheight+2.0f > z) // Accurate map height
            {
                if(checkwater && waterheight != NO_WATER_HEIGHT)
                    if(waterheight > vmapheight)
                        return waterheight+offset;
                return vmapheight+offset;
            }

            if(!sVMapInterface.GetFirstPoint(GetMapId(), GetInstanceID(), GetPhaseMask(), x, y, vmapheight, x, y, z, phx, phy, phz, 0.0f))
            {
                if(checkwater && waterheight != NO_WATER_HEIGHT)
                    if(waterheight > vmapheight)
                        return waterheight+offset;
                return vmapheight+offset; // No breaks inbetween us, so its the right height, we're just a bunch of fuckers!
            }

            if(phz > z)
            {
                if(vmapheight < z)
                {
                    float mmapheight = NavMeshInterface.GetWalkingHeight(GetMapId(), x, y, z, vmapheight);
                    if(mmapheight != MMAP_UNAVAILABLE)
                    {
                        if(checkwater && waterheight != NO_WATER_HEIGHT)
                            if(waterheight > mmapheight)
                                return waterheight+offset;
                        return mmapheight+offset;
                    }
                }
            }
            else
            {
                float mmapheight = NavMeshInterface.GetWalkingHeight(GetMapId(), x, y, z, phz);
                if(mmapheight != MMAP_UNAVAILABLE)
                {
                    if(checkwater && waterheight != NO_WATER_HEIGHT)
                        if(waterheight > mmapheight)
                            return waterheight+offset;
                    return mmapheight+offset;
                }
            }
        }
        else
        {
            float mmapheight = NavMeshInterface.GetWalkingHeight(GetMapId(), x, y, z, vmapheight);
            if(mmapheight != MMAP_UNAVAILABLE)
            {
                if(checkwater && waterheight != NO_WATER_HEIGHT)
                    if(waterheight > mmapheight)
                        return waterheight+offset;
                return mmapheight+offset;
            }
        }
    }

    // I think it's safe to say, no one is ever perfect.
    if((CMapHeight != z+10.0f) && (CMapHeight != z-10.0f))
    {
        if(checkwater && waterheight != NO_WATER_HEIGHT)
            if(waterheight > CMapHeight)
                return waterheight+offset;
        return CMapHeight+offset;
    }

    if(checkwater && waterheight != NO_WATER_HEIGHT)
        return waterheight;
    return z;
}

void WorldObject::_Create( uint32 mapid, float x, float y, float z, float ang )
{
    m_mapId = mapid;
    m_position.ChangeCoords(x, y, z, ang);
    m_spawnLocation.ChangeCoords(x, y, z, ang);
    m_lastMapUpdatePosition.ChangeCoords(x,y,z,ang);
}

WorldPacket * WorldObject::BuildTeleportAckMsg(const LocationVector & v)
{
    ///////////////////////////////////////
    //Update player on the client with TELEPORT_ACK
    if( IsInWorld() )       // only send when inworld
        castPtr<Player>(this)->SetPlayerStatus( TRANSFER_PENDING );

    WorldPacket * data = new WorldPacket(MSG_MOVE_TELEPORT_ACK, 80);
    *data << GetGUID();
    *data << uint32(0); // m_teleportAckCounter;
    *data << uint32(2); // flags
    *data << uint16(0);
    *data << getMSTime();
    data->appendvector(v, true);
    *data << uint32(0);
    return data;
}

void WorldObject::SetPosition( float newX, float newY, float newZ, float newOrientation )
{
    bool updateMap = false;
    if(m_lastMapUpdatePosition.Distance2DSq(newX, newY) > 4.0f)     /* 2.0f */
        updateMap = true;

    m_position.ChangeCoords(newX, newY, newZ, newOrientation);
    if (IsInWorld() && updateMap)
    {
        m_lastMapUpdatePosition.ChangeCoords(newX,newY,newZ,newOrientation);
        m_mapMgr->ChangeObjectLocation(this);

        if( IsPlayer() && castPtr<Player>(this)->GetGroup() && castPtr<Player>(this)->m_last_group_position.Distance2DSq(m_position) > 25.0f ) // distance of 5.0
            castPtr<Player>(this)->GetGroup()->HandlePartialChange( PARTY_UPDATE_FLAG_POSITION, castPtr<Player>(this) );
    }
}

void WorldObject::DestroyForInrange(bool anim)
{
    WorldPacket data(SMSG_DESTROY_OBJECT, 9);
    data << GetGUID();
    data << uint8(anim ? 1 : 0);
    SendMessageToSet(&data, false);
}

void WorldObject::SetRotation( uint64 guid )
{
    WorldPacket data(SMSG_AI_REACTION, 12);
    data << guid;
    data << uint32(2);
    SendMessageToSet(&data, false);
}

void WorldObject::OutPacketToSet(uint16 Opcode, uint16 Len, const void * Data, bool self)
{
    if(self && m_objectTypeId == TYPEID_PLAYER)
        castPtr<Player>(this)->GetSession()->OutPacket(Opcode, Len, Data);

    if(!IsInWorld())
        return;

    unordered_set<Player*  >::iterator itr = m_inRangePlayers.begin();
    unordered_set<Player*  >::iterator it_end = m_inRangePlayers.end();
    int gm = ( m_objectTypeId == TYPEID_PLAYER ? castPtr<Player>(this)->m_isGmInvisible : 0 );
    for(; itr != it_end; itr++)
    {
        if((*itr))
        {
            ASSERT((*itr)->GetSession());
            if( gm )
            {
                if( (*itr)->GetSession()->GetPermissionCount() > 0 )
                    (*itr)->GetSession()->OutPacket(Opcode, Len, Data);
            }
            else
                (*itr)->GetSession()->OutPacket(Opcode, Len, Data);
        }
    }
}

void WorldObject::SendMessageToSet(WorldPacket *data, bool bToSelf, bool myteam_only)
{
    if(!IsInWorld())
        return;

    if(bToSelf && m_objectTypeId == TYPEID_PLAYER)
        castPtr<Player>(this)->GetSession()->SendPacket(data);

    unordered_set<Player*>::iterator itr = m_inRangePlayers.begin();
    unordered_set<Player*>::iterator it_end = m_inRangePlayers.end();
    bool gminvis = (m_objectTypeId == TYPEID_PLAYER ? castPtr<Player>(this)->m_isGmInvisible : false);
    //Zehamster: Splitting into if/else allows us to avoid testing "gminvis==true" at each loop...
    //         saving cpu cycles. Chat messages will be sent to everybody even if player is invisible.
    if(myteam_only)
    {
        uint32 myteam = castPtr<Player>(this)->GetTeam();
        if(gminvis && data->GetOpcode()!=SMSG_MESSAGECHAT)
        {
            for(; itr != it_end; itr++)
            {
                ASSERT((*itr)->GetSession());
                if((*itr)->GetSession()->GetPermissionCount() > 0 && (*itr)->GetTeam()==myteam && PhasedCanInteract(*itr))
                    (*itr)->GetSession()->SendPacket(data);
            }
        }
        else
        {
            for(; itr != it_end; itr++)
            {
                ASSERT((*itr)->GetSession());
                if((*itr)->GetTeam()==myteam && PhasedCanInteract(*itr))
                    (*itr)->GetSession()->SendPacket(data);
            }
        }
    }
    else
    {
        if(gminvis && data->GetOpcode()!=SMSG_MESSAGECHAT)
        {
            for(; itr != it_end; itr++)
            {
                ASSERT((*itr)->GetSession());
                if((*itr)->GetSession()->GetPermissionCount() > 0 && PhasedCanInteract(*itr))
                    (*itr)->GetSession()->SendPacket(data);
            }
        }
        else
        {
            for(; itr != it_end; itr++)
            {
                ASSERT((*itr)->GetSession());
                if( PhasedCanInteract(*itr) )
                    (*itr)->GetSession()->SendPacket(data);
            }
        }
    }
}

void WorldObject::AddToWorld()
{
    MapMgr* mapMgr = sInstanceMgr.GetInstance(this);
    if(mapMgr == NULL)
        return;

    if(IsPlayer())
    {
        // battleground checks
        Player* p = castPtr<Player>(this);
        if( p->m_bg == NULL && mapMgr->m_battleground != NULL )
        {
            // player hasn't been registered in the battleground, ok.
            // that means we re-logged into one. if it's an arena, don't allow it!
            // also, don't allow them in if the bg is full.

            if( !mapMgr->m_battleground->CanPlayerJoin(p) && !p->bGMTagOn)
                return;
        }

        // players who's group disbanded cannot remain in a raid instances alone(no soloing them:P)
        if( !p->triggerpass_cheat && p->GetGroup()== NULL && (mapMgr->GetdbcMap()->IsRaid() || mapMgr->GetMapInfo()->type == INSTANCE_MULTIMODE))
            return;
    } else UpdateAreaInfo(mapMgr);

    m_mapMgr = mapMgr;

    mapMgr->AddObject(this);

    // correct incorrect instance id's
    m_instanceId = m_mapMgr->GetInstanceID();
}

void WorldObject::AddToWorld(MapMgr* pMapMgr)
{
    if(!pMapMgr)
        return; //instance add failed

    if(!IsPlayer())
        UpdateAreaInfo(pMapMgr);
    m_mapMgr = pMapMgr;

    pMapMgr->AddObject(this);

    // correct incorrect instance id's
    m_instanceId = pMapMgr->GetInstanceID();
}

//Unlike addtoworld it pushes it directly ignoring add pool
//this can only be called from the thread of mapmgr!!!
void WorldObject::PushToWorld(MapMgr* mgr)
{
    ASSERT(mgr != NULL);
    if(mgr == NULL)
    {
        // Reset these so session will get updated properly.
        if(IsPlayer())
        {
            sLog.Error("WorldObject","Kicking Player %s due to empty MapMgr;",castPtr<Player>(this)->GetName());
            castPtr<Player>(this)->GetSession()->LogoutPlayer(false);
        }
        return; //instance add failed
    }

    m_mapId = mgr->GetMapId();
    m_instanceId = mgr->GetInstanceID();

    if(!IsPlayer())
        UpdateAreaInfo(mgr);
    m_mapMgr = mgr;
    OnPrePushToWorld();

    mgr->PushObject(this);

    // correct incorrect instance id's
    event_Relocate();

    // call virtual function to handle stuff.. :P
    OnPushToWorld();
}

void WorldObject::RemoveFromWorld(bool free_guid)
{
    // clear loot
    ClearLoot();

    ASSERT(m_mapMgr);
    MapMgr* m = m_mapMgr;
    m_mapMgr = NULLMAPMGR;

    mSemaphoreTeleport = true;

    m->RemoveObject(this, free_guid);

    // remove any spells / free memory
    sEventMgr.RemoveEvents(this, EVENT_UNIT_SPELL_HIT);

    // update our event holder
    event_Relocate();
}

bool WorldObject::canWalk()
{
    if(IsCreature())
    {
        Creature* ctr = castPtr<Creature>(this);
        if(ctr->GetCanMove() & LIMIT_ANYWHERE)
            return true;
        if(ctr->GetCanMove() & LIMIT_GROUND)
            return true;
    }
    else if(IsPlayer())
        return true;

    return false;
}

bool WorldObject::canSwim()
{
    if(IsCreature())
    {
        Creature* ctr = castPtr<Creature>(this);
        if(ctr->GetCanMove() & LIMIT_ANYWHERE)
            return true;
        if(ctr->GetCanMove() & LIMIT_WATER)
            return true;
    }
    else if(IsPlayer())
        return true;

    return false;
}

bool WorldObject::canFly()
{
    if(IsVehicle())
    {
        return false;
    }
    else if(IsCreature())
    {
        Creature* ctr = castPtr<Creature>(this);
        if(ctr->GetCanMove() & LIMIT_ANYWHERE)
            return true;
        if(ctr->GetCanMove() & LIMIT_AIR)
            return true;
    }
    else if(IsPlayer())
    {
        Player* plr = castPtr<Player>(this);
        if(plr->m_FlyingAura)
            return true;
        if(plr->FlyCheat)
            return true;
    }

    return false;
}

bool WorldObject::IsInBox(float centerX, float centerY, float centerZ, float BLength, float BWidth, float BHeight, float BOrientation, float delta)
{
    double rotation = 2*M_PI-BOrientation;
    double sinVal = sin(rotation);
    double cosVal = cos(rotation);
    float playerBoxDistX = GetPositionX() - centerX;
    float playerBoxDistY = GetPositionY() - centerY;
    float rotationPlayerX = (float)(centerX + playerBoxDistX * cosVal - playerBoxDistY*sinVal);
    float rotationPlayerY = (float)(centerY + playerBoxDistY * cosVal + playerBoxDistX*sinVal);
    float dx = rotationPlayerX - centerX;
    float dy = rotationPlayerY - centerY;
    float dz = GetPositionZ() - centerZ;
    if(!((fabs(dx) > BLength/2 + delta) || (fabs(dy) > BWidth/2 + delta) || (fabs(dz) > BHeight/2 + delta)))
        return true;
    return false;
}

////////////////////////////////////////////////////////////

float WorldObject::CalcDistance(WorldObject* Ob)
{
    return CalcDistance(GetPositionX(), GetPositionY(), GetPositionZ(), Ob->GetPositionX(), Ob->GetPositionY(), Ob->GetPositionZ());
}

float WorldObject::CalcDistance(float ObX, float ObY, float ObZ)
{
    return CalcDistance(GetPositionX(), GetPositionY(), GetPositionZ(), ObX, ObY, ObZ);
}

float WorldObject::CalcDistance(WorldObject* Oa, WorldObject* Ob)
{
    return CalcDistance(Oa->GetPositionX(), Oa->GetPositionY(), Oa->GetPositionZ(), Ob->GetPositionX(), Ob->GetPositionY(), Ob->GetPositionZ());
}

float WorldObject::CalcDistance(WorldObject* Oa, float ObX, float ObY, float ObZ)
{
    return CalcDistance(Oa->GetPositionX(), Oa->GetPositionY(), Oa->GetPositionZ(), ObX, ObY, ObZ);
}

float WorldObject::CalcDistance(float OaX, float OaY, float OaZ, float ObX, float ObY, float ObZ)
{
    float xdest = fabs(ObX - OaX);
    float ydest = fabs(ObY - OaY);
    float zdest = fabs(ObZ - OaZ);
    return sqrtf((zdest*zdest) + (ydest*ydest) + (xdest*xdest));
}

float WorldObject::calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y )
{
    float dx = Position2X-Position1X;
    float dy = Position2Y-Position1Y;
    double angle=0.0f;

    // Calculate angle
    if (dx == 0.0)
    {
        if (dy == 0.0)
            angle = 0.0;
        else if (dy > 0.0)
            angle = M_PI * 0.5 /* / 2 */;
        else
            angle = M_PI * 3.0 * 0.5/* / 2 */;
    }
    else if (dy == 0.0)
    {
        if (dx > 0.0)
            angle = 0.0;
        else
            angle = M_PI;
    }
    else
    {
        if (dx < 0.0)
            angle = atanf(dy/dx) + M_PI;
        else if (dy < 0.0)
            angle = atanf(dy/dx) + (2*M_PI);
        else
            angle = atanf(dy/dx);
    }

    // Convert to degrees
    angle = angle * float(180 / M_PI);

    // Return
    return float(angle);
}

float WorldObject::calcRadAngle( float Position1X, float Position1Y, float Position2X, float Position2Y )
{
    double dx = double(Position2X-Position1X);
    double dy = double(Position2Y-Position1Y);
    double angle=0.0;

    // Calculate angle
    if (dx == 0.0)
    {
        if (dy == 0.0)
            angle = 0.0;
        else if (dy > 0.0)
            angle = M_PI * 0.5/*/ 2.0*/;
        else
            angle = M_PI * 3.0 * 0.5/*/ 2.0*/;
    }
    else if (dy == 0.0)
    {
        if (dx > 0.0)
            angle = 0.0;
        else
            angle = M_PI;
    }
    else
    {
        if (dx < 0.0)
            angle = atan(dy/dx) + M_PI;
        else if (dy < 0.0)
            angle = atan(dy/dx) + (2*M_PI);
        else
            angle = atan(dy/dx);
    }

    // Return
    return float(angle);
}

float WorldObject::getEasyAngle( float angle )
{
    while ( angle < 0 ) {
        angle = angle + 360;
    }
    while ( angle >= 360 ) {
        angle = angle - 360;
    }
    return angle;
}

bool WorldObject::inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y )
{
    float angle = calcAngle( Position1X, Position1Y, Position2X, Position2Y );
    float lborder = getEasyAngle( ( Orientation - (FOV*0.5f/*/2*/) ) );
    float rborder = getEasyAngle( ( Orientation + (FOV*0.5f/*/2*/) ) );
    //sLog.outDebug("Orientation: %f Angle: %f LeftBorder: %f RightBorder %f",Orientation,angle,lborder,rborder);
    if(((angle >= lborder) && (angle <= rborder)) || ((lborder > rborder) && ((angle < rborder) || (angle > lborder))))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Return angle in range 0..2*pi
float GetAngle(float x, float y, float Targetx, float Targety)
{
    float dx = Targetx - x;
    float dy = Targety - y;

    float ang = atan2(dy, dx);
    ang = (ang >= 0) ? ang : 2 * M_PI + ang;
    return ang;
}

float Normalize(float o)
{
    if (o < 0)
    {
        float mod = o *-1;
        mod = fmod(mod, 2.0f * static_cast<float>(M_PI));
        mod = -mod + 2.0f * static_cast<float>(M_PI);
        return mod;
    }
    return fmod(o, 2.0f * static_cast<float>(M_PI));
}

bool WorldObject::isTargetInFront(WorldObject* target)
{
    // always have self in arc
    if (target == this)
        return true;

    // move arc to range 0.. 2*pi
    float arc = Normalize(static_cast<float>(M_PI/2));

    float angle = GetAngle(GetPositionX(), GetPositionY(), target->GetPositionX(), target->GetPositionY());
    angle -= m_position.o;

    // move angle to range -pi +pi
    angle = Normalize(angle);
    if(angle > M_PI)
        angle -= 2.0f*M_PI;

    float lborder =  -1 * (arc/2.0f);               // in range -pi..0
    float rborder = (arc/2.0f);                     // in range 0..pi
    return ((angle >= lborder) && (angle <= rborder));
}

bool WorldObject::isInArc(WorldObject* target , float angle) // angle in degrees
{
    return inArc( GetPositionX() , GetPositionY() , angle , GetOrientation() , target->GetPositionX() , target->GetPositionY() );
}

bool WorldObject::isInRange(WorldObject* target, float range)
{
    float dist = CalcDistance( target );
    return( dist <= range );
}

void WorldObject::_setFaction()
{
    // Clear our old faction info
    if(GetTypeId() == TYPEID_UNIT || IsPlayer())
        m_factionTemplate = dbcFactionTemplate.LookupEntry(GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    else if(GetTypeId() == TYPEID_GAMEOBJECT)
        m_factionTemplate = dbcFactionTemplate.LookupEntry(GetUInt32Value(GAMEOBJECT_FACTION));

    //Lets update our faction sets since we have changed faction.
    UpdateOppFactionSet();
}

void WorldObject::UpdateOppFactionSet()
{
    if(!IsUnit())
        return;

    m_oppFactsInRange.clear();
    for(WorldObject::InRangeUnitSet::iterator i = GetInRangeUnitSetBegin(); i != GetInRangeUnitSetEnd(); i++)
    {
        if (((*i)->GetTypeId() == TYPEID_UNIT) || ((*i)->IsPlayer()) || ((*i)->GetTypeId() == TYPEID_GAMEOBJECT))
        {
            if (sFactionSystem.isHostile(castPtr<Unit>(this), (*i)))
            {
                if(!(*i)->IsInRangeOppFactSet(castPtr<Unit>(this)))
                    (*i)->m_oppFactsInRange.insert(castPtr<Unit>(this));
                if (!IsInRangeOppFactSet((*i)))
                    m_oppFactsInRange.insert((*i));
            }
            else
            {
                if((*i)->IsInRangeOppFactSet(castPtr<Unit>(this)))
                    (*i)->m_oppFactsInRange.erase(castPtr<Unit>(this));
                if (IsInRangeOppFactSet((*i)))
                    m_oppFactsInRange.erase((*i));
            }
        }
    }
}

int32 WorldObject::DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras)
{
    if(!IsInWorld())
        return 0;
    if( !pVictim || !pVictim->isAlive() || !pVictim->IsInWorld())
        return 0;
    if( pVictim->bInvincible == true )
        return 0;
    if( pVictim->IsSpiritHealer() )
        return 0;

    if( pVictim->GetStandState() )//not standing-> standup
        pVictim->SetStandState( STANDSTATE_STAND );//probably mobs also must standup

    Player* plr = NULL;
    if(IsPet())
        plr = castPtr<Pet>(this)->GetPetOwner();
    else if(IsPlayer())
        plr = castPtr<Player>(this);

    // Player we are attacking, or the owner of totem/pet/etc
    Player *pOwner = pVictim->IsPlayer() ? castPtr<Player>(pVictim) : NULL;

    // This is the player or the player controlling the totem/pet/summon
    Player *pAttacker = IsPlayer() ? castPtr<Player>(this) : NULL;

    // We identified both the attacker and the victim as possible PvP combatants, if we are not dueling we will flag the attacker
    if( pOwner != NULL && pAttacker != NULL && pOwner != pAttacker && pOwner != pAttacker->DuelingWith )
    {
        if( !pAttacker->IsPvPFlagged() )
        {
            pAttacker->PvPToggle();
        }
    }

    // PvP NPCs will flag the player when attacking them
    if( pVictim->IsCreature() && pVictim->IsPvPFlagged() && pAttacker != NULL )
    {
        if( !pAttacker->IsPvPFlagged() )
        {
            pAttacker->PvPToggle();
        }
    }

    //If our pet attacks  - flag us.
    if( pVictim->IsPlayer() && IsPet() )
    {
        Player* owner = castPtr<Player>( castPtr<Pet>(this)->GetPetOwner() );
        if( owner != NULL )
            if( owner->isAlive() && castPtr<Player>( pVictim )->DuelingWith != owner )
                owner->SetPvPFlag();
    }

    if(!no_remove_auras)
    {
        float breakchance = 35.0f;
        if( spellId == 51514)// && IsUnit() && HasDummyAura(SPELL_HASH_GLYPH_OF_HEX) ) wait till 3.1
            breakchance += 15.0f;

        //zack 2007 04 24 : root should not remove self (and also other unknown spells)
        if(spellId)
            pVictim->m_AuraInterface.RemoveAllAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN, spellId);
        else
            pVictim->m_AuraInterface.RemoveAllAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);
    }

    // Paladin: Blessing of Sacrifice, and Warlock: Soul Link
    if( pVictim->m_damageSplitTarget.active)
    {
        if( spellId )
        {
            SpellEntry *spell = dbcSpell.LookupEntry(spellId);
            damage = (float)pVictim->DoDamageSplitTarget(damage, spell->School, false);
        }
        else
        {
            damage = (float)pVictim->DoDamageSplitTarget(damage, 0, true);
        }
    }

    if(IsUnit() && castPtr<Unit>(this)->isAlive() )
    {
        if( castPtr<Unit>(this) != pVictim && pVictim->IsPlayer() && IsPlayer() && castPtr<Player>(this)->m_hasInRangeGuards )
        {
            castPtr<Player>(this)->SetGuardHostileFlag(true);
            castPtr<Player>(this)->CreateResetGuardHostileFlagEvent();
        }

        if(pVictim->IsPlayer() && !pVictim->CombatStatus.IsInCombat())
            sHookInterface.OnEnterCombat( castPtr<Player>( pVictim ), castPtr<Unit>(this) );

        if(IsPlayer() && ! castPtr<Player>(this)->CombatStatus.IsInCombat())
            sHookInterface.OnEnterCombat( castPtr<Player>(this), castPtr<Player>(this) );

        if(plr != NULL && pVictim->IsCreature())
            castPtr<Creature>(pVictim)->Tag(plr);

        // Pepsi1x1: is this correct this
        if( pVictim != castPtr<Unit>(this))
            castPtr<Unit>(this)->CombatStatus.OnDamageDealt( pVictim, damage );
    }

    ///Rage
    if( pVictim->GetMaxPower(POWER_TYPE_RAGE) > 0
        && pVictim != castPtr<Unit>(this)
        && pVictim->IsPlayer())
    {
        float level = (float)pVictim->getLevel();
        float c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;
        uint32 rage = pVictim->GetPower(POWER_TYPE_RAGE);
        float val = 2.5f * damage / c;
        rage += float2int32(val) * 10;
        if( rage > pVictim->GetMaxPower(POWER_TYPE_RAGE) )
            rage = pVictim->GetMaxPower(POWER_TYPE_RAGE);

        pVictim->SetPower(POWER_TYPE_RAGE, rage);
        pVictim->SendPowerUpdate();
    }

    //* BATTLEGROUND DAMAGE COUNTER *//
    if( pVictim != castPtr<Unit>(this) && plr != NULL )
    {
        if(plr->m_bg != NULL)
        {
            plr->m_bgScore.DamageDone += damage;
            plr->m_bg->UpdatePvPData();
        }
    }

    uint32 health = pVictim->GetUInt32Value(UNIT_FIELD_HEALTH );

    /*------------------------------------ DUEL HANDLERS --------------------------*/
    if(pVictim->IsPlayer() && castPtr<Player>(pVictim)->DuelingWith != NULL) //Both Players
    {
        if(health <= damage)
        {
            if(IsPlayer() && (castPtr<Player>(pVictim)->DuelingWith == castPtr<Player>(this)))
            {
                // End Duel
                castPtr<Player>(this)->EndDuel(DUEL_WINNER_KNOCKOUT);
                castPtr<Player>(this)->GetAchievementInterface()->HandleAchievementCriteriaWinDuel();
                castPtr<Player>(pVictim)->GetAchievementInterface()->HandleAchievementCriteriaLoseDuel();

                // surrender emote
                castPtr<Player>(pVictim)->Emote(EMOTE_ONESHOT_BEG);           // Animation

                // Remove Negative Auras from duelist.
                castPtr<Player>(pVictim)->m_AuraInterface.RemoveAllNegAurasFromGUID(GetGUID());

                // Player in Duel and Player Victim has lost
                castPtr<Player>(pVictim)->CombatStatus.Vanish(GetLowGUID());
                castPtr<Player>(this)->CombatStatus.Vanish(pVictim->GetLowGUID());

                damage = health-5;
            }
            else if(castPtr<Player>(pVictim)->DuelingWith != NULL)
            {
                // We have to call the achievement interface from the duelingwith before, otherwise we crash.
                castPtr<Player>(pVictim)->DuelingWith->GetAchievementInterface()->HandleAchievementCriteriaWinDuel();
                castPtr<Player>(pVictim)->DuelingWith->EndDuel(DUEL_WINNER_KNOCKOUT);
//              castPtr<Player>(pVictim)->GetAchievementInterface()->HandleAchievementCriteriaLoseDuel(); Disable because someone cheated!
            }
        }
    }

    if((pVictim->IsPlayer()) && (IsPet()))
    {
        if((health <= damage) && castPtr<Player>(pVictim)->DuelingWith == castPtr<Pet>(this)->GetPetOwner())
        {
            Player* petOwner = castPtr<Pet>(this)->GetPetOwner();
            if(petOwner)
            {
                //Player in Duel and Player Victim has lost
                uint32 NewHP = pVictim->GetUInt32Value(UNIT_FIELD_MAXHEALTH)/100;
                if(NewHP < 5)
                    NewHP = 5;

                //Set there health to 1% or 5 if 1% is lower then 5
                pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, NewHP);
                //End Duel
                petOwner->EndDuel(DUEL_WINNER_KNOCKOUT);
                return health-5;
            }
        }
    }

    /*------------------------------------ DUEL HANDLERS END--------------------------*/

    bool isCritter = false;
    if(pVictim->GetTypeId() == TYPEID_UNIT && castPtr<Creature>(pVictim)->GetCreatureData())
    {
        if(castPtr<Creature>(pVictim)->GetCreatureData()->Type == CRITTER)
            isCritter = true;
        else if(isTargetDummy(castPtr<Creature>(pVictim)->GetCreatureData()->Entry) && health <= damage)
        {   //Dummy trainers can't die
            uint32 newh = 5; // Just limit to 5HP (can't use 1HP here).
            if(pVictim->GetMaxHealth() < 5)
                newh = pVictim->GetMaxHealth();

            pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, newh);
            return health-5;
        }
    }

    /* -------------------------- HIT THAT CAUSES VICTIM TO DIE ---------------------------*/
    if ((isCritter || health <= damage) )
    {
        if( IsUnit() )
        {
            if(!sHookInterface.OnPreUnitDie( castPtr<Unit>(this), pVictim) )
                return 0;
        }

        if( pVictim->HasDummyAura(SPELL_HASH_GUARDIAN_SPIRIT) )
        {
            pVictim->CastSpell(pVictim, dbcSpell.LookupEntry(48153), true);
            pVictim->RemoveDummyAura(SPELL_HASH_GUARDIAN_SPIRIT);
            return 0;
        }

        //warlock - seed of corruption
        if( IsUnit() )
        {
            if( IsPlayer() && pVictim->IsUnit() && !pVictim->IsPlayer() && m_mapMgr->m_battleground && m_mapMgr->m_battleground->GetType() == BATTLEGROUND_ALTERAC_VALLEY )
                TO_ALTERACVALLEY(m_mapMgr->m_battleground)->HookOnUnitKill( castPtr<Player>(this), pVictim );
        }

        // check if pets owner is combat participant
        bool owner_participe = false;
        if( IsPet() )
        {
            Player* owner = castPtr<Pet>(this)->GetPetOwner();
            if( owner != NULL && pVictim->GetAIInterface()->getThreatByPtr( owner ) > 0 )
                owner_participe = true;
        }

        /* victim died! */
        Unit* pKiller = pVictim->CombatStatus.GetKiller();
        if( pVictim->IsPlayer() )
        {
            // let's see if we have shadow of death
            if( !pVictim->m_AuraInterface.FindPositiveAuraByNameHash(SPELL_HASH_SHADOW_OF_DEATH) && castPtr<Player>( pVictim)->HasSpell( 49157 )  &&
                !(castPtr<Player>(pVictim)->m_bg && castPtr<Player>(pVictim)->m_bg->IsArena())) //check for shadow of death
            {
                SpellEntry* sorInfo = dbcSpell.LookupEntry(54223);
                if( sorInfo != NULL && castPtr<Player>(pVictim)->Cooldown_CanCast( sorInfo ))
                {
                    Spell* sor(new Spell( pVictim, sorInfo, false, NULL ));
                    SpellCastTargets targets;
                    targets.m_unitTarget = pVictim->GetGUID();
                    sor->prepare(&targets);
                    return 0;
                }
            }

            castPtr<Player>( pVictim )->KillPlayer();

            /* Remove all Auras */
            pVictim->m_AuraInterface.EventDeathAuraRemoval();

            /* Set victim health to 0 */
            pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
            CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnPlayerDeath )( castPtr<Player>(pVictim), pKiller );

            if( IsCreature() )
                castPtr<Player>(pVictim)->GetAchievementInterface()->HandleAchievementCriteriaKilledByCreature( GetEntry() );
            else if(IsPlayer())
                castPtr<Player>(pVictim)->GetAchievementInterface()->HandleAchievementCriteriaKilledByPlayer();
        }
        else
        {
            pVictim->SetDeathState( JUST_DIED );
            /* Remove all Auras */
            pVictim->m_AuraInterface.EventDeathAuraRemoval();
            /* Set victim health to 0 */
            pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);

            CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnCreatureDeath )( castPtr<Creature>(pVictim), pKiller );
        }

        pVictim->SummonExpireAll(false);

        if( pVictim->IsPlayer() && (!IsPlayer() || pVictim == castPtr<Unit>(this) ) )
            castPtr<Player>( pVictim )->DeathDurabilityLoss(0.10);

        /* Zone Under Attack */
        MapInfo * pZMapInfo = LimitedMapInfoStorage.LookupEntry(GetMapId());
        if( pZMapInfo != NULL && pZMapInfo->type == INSTANCE_NULL && !pVictim->IsPlayer() && !pVictim->IsPet() && ( IsPlayer() || IsPet() ) )
        {
            // Only NPCs that bear the PvP flag can be truly representing their faction.
            if( castPtr<Creature>(pVictim)->IsPvPFlagged() )
            {
                Player* pAttacker = NULL;
                if( IsPet() )
                    pAttacker = castPtr<Pet>(this)->GetPetOwner();
                else if(IsPlayer())
                    pAttacker = castPtr<Player>(this);

                if( pAttacker != NULL)
                {
                    uint8 teamId = (uint8)pAttacker->GetTeam();
                    if(teamId == 0) // Swap it.
                        teamId = 1;
                    else
                        teamId = 0;
                    uint32 AreaID = pVictim->GetAreaId();
                    if(AreaID)
                    {
                        WorldPacket data(SMSG_ZONE_UNDER_ATTACK, 4);
                        data << AreaID;
                        sWorld.SendFactionMessage(&data, teamId);
                    }
                }
            }
        }

        if(pVictim->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) > 0)
        {
            if(pVictim->GetCurrentSpell())
            {
                Spell* spl = pVictim->GetCurrentSpell();
                for(int i = 0; i < 3; i++)
                {
                    if(spl->GetSpellProto()->Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                    {
                        DynamicObject* dObj = GetMapMgr()->GetDynamicObject(pVictim->GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT));
                        if(dObj != NULL)
                        {
                            WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
                            data << dObj->GetGUID();
                            dObj->SendMessageToSet(&data, false);
                            dObj->RemoveFromWorld(true);
                            dObj->Destruct();
                            dObj = NULL;
                        }
                    }
                }
                if(spl->GetSpellProto()->ChannelInterruptFlags == 48140)
                    spl->cancel();
            }
        }

        if(pVictim->IsPlayer())
        {
            uint32 self_res_spell = 0;
            Player* plrVictim = castPtr<Player>(pVictim);
            if(!(plrVictim->m_bg && plrVictim->m_bg->IsArena())) // Can't self res in Arena
            {
                self_res_spell = plrVictim->SoulStone;
                plrVictim->SoulStone = plrVictim->SoulStoneReceiver = 0;

                if( !self_res_spell && plrVictim->bReincarnation )
                {
                    SpellEntry* m_reincarnSpellInfo = dbcSpell.LookupEntry( 20608 );
                    if( plrVictim->Cooldown_CanCast( m_reincarnSpellInfo ) )
                    {
                        uint32 ankh_count = plrVictim->GetItemInterface()->GetItemCount( 17030 );
                        if( ankh_count || castPtr<Player>(plrVictim)->HasDummyAura(SPELL_HASH_GLYPH_OF_RENEWED_LIFE ))
                            self_res_spell = 21169;
                    }
                }
            }

            pVictim->SetUInt32Value( PLAYER_SELF_RES_SPELL, self_res_spell );
            pVictim->Dismount();
        }

        // Wipe our attacker set on death
        pVictim->CombatStatus.Vanished();

        /* Stop Units from attacking */
        if( pAttacker && pAttacker->IsInWorld() )
            pAttacker->EventAttackStop();

        if( IsUnit() )
        {
            castPtr<Unit>(this)->smsg_AttackStop( pVictim );

            /* Tell Unit that it's target has Died */
            castPtr<Unit>(this)->addStateFlag( UF_TARGET_DIED );
        }

        if( pVictim->IsPlayer() )
        {
            if( castPtr<Player>( pVictim)->HasDummyAura(SPELL_HASH_SPIRIT_OF_REDEMPTION) ) //check for spirit of Redemption
            {
                SpellEntry* sorInfo = dbcSpell.LookupEntry(27827);
                if( sorInfo != NULL )
                {
                    pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 1);
                    Spell* sor(new Spell( pVictim, sorInfo, true, NULL ));
                    SpellCastTargets targets;
                    targets.m_unitTarget = pVictim->GetGUID();
                    sor->prepare(&targets);
                }
            }
        }

        /* -------------------------------- HONOR + BATTLEGROUND CHECKS ------------------------ */
        if( plr != NULL)
        {
            if( plr->m_bg != NULL )
                plr->m_bg->HookOnPlayerKill( plr, pVictim );
            CALL_INSTANCE_SCRIPT_EVENT( plr->GetMapMgr(), OnPlayerKillPlayer )( plr, pVictim );

            if( pVictim->IsPlayer() )
            {
                sHookInterface.OnKillPlayer( plr, castPtr<Player>( pVictim ) );
                HonorHandler::OnPlayerKilled( plr, castPtr<Player>( pVictim ) );
            }
            else
            {
                // REPUTATION
                if( !isCritter )
                    plr->Reputation_OnKilledUnit( pVictim, false );
            }

            if(plr->getLevel() <= (pVictim->getLevel() + 8) && plr->getClass() == WARRIOR)
            {   // currently only warriors seem to use it (Victory Rush)
                plr->SetFlag( UNIT_FIELD_AURASTATE, AURASTATE_FLAG_VICTORIOUS );
                if( !sEventMgr.HasEvent(castPtr<Unit>(plr), EVENT_VICTORIOUS_FLAG_EXPIRE ) )
                    sEventMgr.AddEvent( castPtr<Unit>(plr), &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_VICTORIOUS, EVENT_VICTORIOUS_FLAG_EXPIRE, 20000, 1, 0 );
                else
                    sEventMgr.ModifyEventTimeLeft( castPtr<Unit>(plr), EVENT_VICTORIOUS_FLAG_EXPIRE, 20000 , false );
            }
        }
        /* -------------------------------- HONOR + BATTLEGROUND CHECKS END------------------------ */

        uint64 victimGuid = pVictim->GetGUID();
        SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_DEAD );

        // player loot for battlegrounds
        if( pVictim->IsPlayer() )
        {
            // set skinning flag, this is the "remove insignia"
            if(ManagerCheck(m_mapMgr))
            {
                if(FunctionCall(m_mapMgr, MapSupportsPlayerLoot)())
                {
                    pVictim->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
                    castPtr<Player>(pVictim)->m_insigniaTaken = false;
                }
            }
            if( castPtr<Player>(pVictim)->m_bg != NULL && castPtr<Player>(pVictim)->m_bg->SupportsPlayerLoot() )
            {
                pVictim->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
                castPtr<Player>(pVictim)->m_insigniaTaken = false;
            }
        }
        else if(castPtr<Creature>(pVictim)->m_taggingPlayer != 0 )    // only execute loot code if we were tagged
        {
            // fill loot vector
            castPtr<Creature>(pVictim)->GenerateLoot();

            // update visual.
            castPtr<Creature>(pVictim)->UpdateLootAnimation(pAttacker);
        }

        if(pVictim->IsCreature())
        {
            //--------------------------------- POSSESSED CREATURES -----------------------------------------
            if( pVictim->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE) )
            {   //remove possess aura from controller
                Player* vController = GetMapMgr()->GetPlayer( (uint32)pVictim->GetUInt64Value(UNIT_FIELD_CHARMEDBY) );
                if( vController )
                {
                    if( vController->GetUInt64Value( UNIT_FIELD_CHARM ) == victimGuid )//make sure he is target controller
                    {
                        vController->UnPossess();
                    }
                }
            }

            //--------------------------------- PARTY LOG -----------------------------------------
            if(pVictim->IsCreature() && pVictim->GetAIInterface())
                pVictim->GetAIInterface()->OnDeath( this );

            if(IsPlayer())
            {
                WorldPacket data(SMSG_PARTYKILLLOG, 16);
                data << GetGUID() << victimGuid;
                SendMessageToSet(&data, true);
            }

            // it Seems that pets some how dont get a name and cause a crash here
            //bool isCritter = (pVictim->GetCreatureName() != NULL)? pVictim->GetCreatureName()->Type : 0;

            //---------------------------------looot-----------------------------------------
            if( IsPlayer() && !pVictim->IsPet() &&
                pVictim->GetUInt64Value( UNIT_FIELD_CREATEDBY ) == 0 &&
                pVictim->GetUInt64Value( UNIT_FIELD_SUMMONEDBY ) == 0 )
            {
                // TODO: lots of casts are bad make a temp member pointer to use for batches like this
                // that way no local loadhitstore and its just one assignment

                //Not all NPC's give XP, check for it in proto no_XP
                bool can_give_xp = true;
                if(pVictim->IsCreature())
                    if(castPtr<Creature>(pVictim)->GetExtraInfo())
                        can_give_xp = (castPtr<Creature>(pVictim)->GetExtraInfo()->no_xp ? false : true);

                if(can_give_xp)
                {
                    // Is this player part of a group
                    if( castPtr<Player>(this)->InGroup() )
                    {
                        //Calc Group XP
                        castPtr<Player>(this)->GiveGroupXP( pVictim, castPtr<Player>(this) );
                        //TODO: pet xp if player in group
                    }
                    else
                    {
                        uint32 xp = CalculateXpToGive( pVictim, castPtr<Unit>(this) );
                        if( xp > 0 )
                        {
                            if(castPtr<Player>(this)->MobXPGainRate)
                                xp += (xp*(castPtr<Player>(this)->MobXPGainRate/100));

                            castPtr<Player>(this)->GiveXP( xp, victimGuid, true );
                            if( castPtr<Player>(this)->GetSummon() && castPtr<Player>(this)->GetSummon()->GetUInt32Value( UNIT_CREATED_BY_SPELL ) == 0 )
                            {
                                xp = CalculateXpToGive( pVictim, castPtr<Player>(this)->GetSummon() );
                                if( xp > 0 )
                                    castPtr<Player>(this)->GetSummon()->GiveXP( xp );
                            }
                        }
                    }
                }

                // Achievement: on kill unit
                if( !pVictim->IsPlayer() && IsPlayer() )
                {
                    Player* pThis = castPtr<Player>(this);
                    pThis->GetAchievementInterface()->HandleAchievementCriteriaKillCreature( pVictim->GetUInt32Value(OBJECT_FIELD_ENTRY) );
                }

                if( pVictim->GetTypeId() != TYPEID_PLAYER )
                    sQuestMgr.OnPlayerKill( castPtr<Player>(this), castPtr<Creature>( pVictim ) );
            }
            else /* is Creature or GameObject* */
            {
                /* ----------------------------- PET XP HANDLING -------------- */
                if( owner_participe && IsPet() && !pVictim->IsPet() )
                {
                    Player* petOwner = castPtr<Pet>(this)->GetPetOwner();
                    if( petOwner != NULL && petOwner->IsPlayer() )
                    {
                        if( petOwner->InGroup() )
                        {
                            //Calc Group XP
                            castPtr<Unit>(this)->GiveGroupXP( pVictim, petOwner );
                            //TODO: pet xp if player in group
                        }
                        else
                        {
                            uint32 xp = CalculateXpToGive( pVictim, petOwner );
                            if( xp > 0 )
                            {
                                petOwner->GiveXP( xp, victimGuid, true );
                                if( !castPtr<Pet>(this)->IsSummonedPet() )
                                {
                                    xp = CalculateXpToGive( pVictim, castPtr<Pet>(this) );
                                    if( xp > 0 )
                                        castPtr<Pet>(this)->GiveXP( xp );
                                }
                            }
                        }
                    }
                    if( petOwner != NULL && pVictim->GetTypeId() != TYPEID_PLAYER &&
                        pVictim->GetTypeId() == TYPEID_UNIT )
                        sQuestMgr.OnPlayerKill( petOwner, castPtr<Creature>( pVictim ) );
                }
                /* ----------------------------- PET XP HANDLING END-------------- */

                /* ----------------------------- PET DEATH HANDLING -------------- */
                if( pVictim->IsPet() )
                {
                    // dying pet looses 1 happiness level
                    if( !castPtr<Pet>( pVictim )->IsSummonedPet() )
                    {
                        uint32 hap = castPtr<Pet>( pVictim )->GetHappiness();
                        hap = hap - PET_HAPPINESS_UPDATE_VALUE > 0 ? hap - PET_HAPPINESS_UPDATE_VALUE : 0;
                        castPtr<Pet>( pVictim )->SetHappiness(hap);
                    }

                    castPtr<Pet>( pVictim )->DelayedRemove( false, true );

                    //remove owner warlock soul link from caster
                    Player* owner = castPtr<Pet>( pVictim )->GetPetOwner();
                    if( owner != NULL )
                        owner->EventDismissPet();
                }
                /* ----------------------------- PET DEATH HANDLING END -------------- */
                else if( pVictim->GetUInt64Value( UNIT_FIELD_CHARMEDBY ) )
                {
                    //remove owner warlock soul link from caster
                    Unit* owner=pVictim->GetMapMgr()->GetUnit( pVictim->GetUInt64Value( UNIT_FIELD_CHARMEDBY ) );
                    if( owner != NULL && owner->IsPlayer())
                        castPtr<Player>( owner )->EventDismissPet();
                }
            }
        }
        else if( pVictim->IsPlayer() )
        {
            /* -------------------- RESET BREATH STATE ON DEATH -------------- */
            castPtr<Player>( pVictim )->m_UnderwaterState = 0;

            /* -------------------- REMOVE PET WHEN PLAYER DIES ---------------*/
            if( castPtr<Player>( pVictim )->GetSummon() != NULL )
            {
                if( pVictim->GetUInt32Value( UNIT_CREATED_BY_SPELL ) > 0 )
                    castPtr<Player>( pVictim )->GetSummon()->Dismiss( true );
                else
                    castPtr<Player>( pVictim )->GetSummon()->Remove( true, true, true );
            }
            /* -------------------- REMOVE PET WHEN PLAYER DIES END---------------*/
        } else sLog.outDebug("DealDamage for unknown obj.");

        if(IsUnit())
            sHookInterface.OnPostUnitDie(castPtr<Unit>(this), pVictim);
        return health;
    }
    else /* ---------- NOT DEAD YET --------- */
    {
        if(pVictim != castPtr<Unit>(this) /* && updateskill */)
        {
            // Send AI Reaction UNIT vs UNIT
            if( IsCreature() )
                castPtr<Creature>(this)->GetAIInterface()->AttackReaction( pVictim, damage, spellId );

            // Send AI Victim Reaction
            if( IsUnit() && pVictim->IsCreature() )
                castPtr<Creature>( pVictim )->GetAIInterface()->AttackReaction( castPtr<Unit>(this), damage, spellId );
        }

        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, health - damage );
    }
    return damage;
}

int32 WorldObject::SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, bool allowProc, bool static_damage, bool no_remove_auras, uint32 AdditionalCritChance)
{
//==========================================================================================
//==============================Unacceptable Cases Processing===============================
//==========================================================================================
    if(!pVictim || !pVictim->isAlive())
        return 0;

    SpellEntry *spellInfo = dbcSpell.LookupEntry( spellID );
    if(!spellInfo)
        return 0;

    if (IsPlayer() && !castPtr<Player>(this)->canCast(spellInfo))
        return 0;
//==========================================================================================
//==============================Variables Initialization====================================
//==========================================================================================
    uint32 school = spellInfo->School;
    float res = float(damage);
    bool critical = false;
    Unit* caster = IsUnit() ? castPtr<Unit>(this) : NULL;

//==========================================================================================
//==============================+Spell Damage Bonus Calculations============================
//==========================================================================================
//------------------------------by stats----------------------------------------------------
    if( IsUnit() && !static_damage )
    {
        caster->m_AuraInterface.RemoveAllAurasByInterruptFlag( AURA_INTERRUPT_ON_START_ATTACK );

        res = caster->GetSpellBonusDamage( pVictim, spellInfo, 0, ( int )res, false );

        // Aura 271 - Mods Damage for particular casters spells
        Unit::DamageTakenPctModPerCasterType::iterator it = pVictim->DamageTakenPctModPerCaster.find(GetGUID());
        while(it != pVictim->DamageTakenPctModPerCaster.end() && it->first == GetGUID())
        {
            if(spellInfo->SpellGroupType[0] & it->second.first[0] ||
                spellInfo->SpellGroupType[1] & it->second.first[1] ||
                spellInfo->SpellGroupType[2] & it->second.first[2])
            {
                res *= float(100 + it->second.second) / 100;
            }
            it++;
        }

//==========================================================================================
//==============================Post +SpellDamage Bonus Modifications=======================
//==========================================================================================
        if( res < 0 )
            res = 0;
        else if( !spellInfo->isUncrittableSpell() )
        {
//------------------------------critical strike chance--------------------------------------
            // lol ranged spells were using spell crit chance
            float CritChance;
            if( spellInfo->IsSpellWeaponSpell() )
            {
                CritChance = GetFloatValue( PLAYER_RANGED_CRIT_PERCENTAGE );
                if( pVictim->IsPlayer() )
                    CritChance += castPtr<Player>(pVictim)->res_R_crit_get();
                CritChance += float(pVictim->AttackerCritChanceMod[spellInfo->School]);
                CritChance += AdditionalCritChance;
                CritChance -= pVictim->IsPlayer() ? castPtr<Player>(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE ) : 0.0f;
                if( spellInfo->IsSpellMeleeSpell() ) CritChance += (float)(pVictim->AttackerCritChanceMod[0]);
            }
            else
            {
                CritChance = caster->spellcritperc + caster->SpellCritChanceSchool[school] + pVictim->AttackerCritChanceMod[school];
                if( caster->IsPlayer() && ( pVictim->m_rooted - pVictim->m_stunned ) )
                    CritChance += castPtr<Player>( caster )->m_RootedCritChanceBonus;

                if( spellInfo->SpellGroupType )
                {
                    caster->SM_FFValue(SMT_CRITICAL, &CritChance, spellInfo->SpellGroupType);
                    caster->SM_PFValue(SMT_CRITICAL, &CritChance, spellInfo->SpellGroupType);
                }

                CritChance += AdditionalCritChance;
                CritChance -= pVictim->IsPlayer() ? castPtr<Player>(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE ) : 0.0f;
            }

            if( CritChance > 0.f )
            {
                if( CritChance > 95.f )
                    CritChance = 95.f;
                critical = Rand(CritChance);
            }

//==========================================================================================
//==============================Spell Critical Hit==========================================
//==========================================================================================
            if (critical)
            {
                int32 critical_bonus = 100;
                if( spellInfo->SpellGroupType )
                    caster->SM_PIValue(SMT_CRITICAL_DAMAGE, &critical_bonus, spellInfo->SpellGroupType );

                if( critical_bonus > 0 )
                {
                    // the bonuses are halved by 50% (funky blizzard math :S)
                    float b;
                    if( spellInfo->School == 0 || spellInfo->IsSpellWeaponSpell() )     // physical || hackfix SoCommand/JoCommand
                        b = ( ( float(critical_bonus) ) / 100.0f ) + 1.0f;
                    else b = ( ( float(critical_bonus) / 2.0f ) / 100.0f ) + 1.0f;
                    res *= b;

                    if( pVictim->IsPlayer() )
                    {
                        float dmg_reduction_pct = 2.2f * castPtr<Player>(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE ) / 100.0f;
                        if( dmg_reduction_pct > 0.33f )
                            dmg_reduction_pct = 0.33f; // 3.0.3

                        res = res - res * dmg_reduction_pct;
                    }
                }

                pVictim->Emote( EMOTE_ONESHOT_WOUNDCRITICAL );
            }
        }
    }
//==========================================================================================
//==============================Post Roll Calculations======================================
//==========================================================================================

//------------------------------absorption--------------------------------------------------
    uint32 abs_dmg = pVictim->AbsorbDamage(this, school, float2int32(floor(res)), dbcSpell.LookupEntry(spellID));
    res -= abs_dmg; if(res < 1.0f) res = 0.f;

    dealdamage dmg;
    dmg.school_type = school;
    dmg.full_damage = res;
    dmg.resisted_damage = abs_dmg;

    //------------------------------resistance reducing-----------------------------------------
    if(res > 0 && IsUnit())
    {
        castPtr<Unit>(this)->CalculateResistanceReduction(pVictim,&dmg,spellInfo,0.0f);
        if((int32)dmg.resisted_damage >= dmg.full_damage)
            res = 0;
        else res = float(dmg.full_damage - dmg.resisted_damage);
    }
    //------------------------------special states----------------------------------------------
    if(pVictim->bInvincible == true)
    {
        res = 0;
        dmg.resisted_damage = dmg.full_damage;
    }

//==========================================================================================
//==============================Data Sending ProcHandling===================================
//==========================================================================================

    int32 ires = float2int32(res);

//--------------------------split damage-----------------------------------------------
    // Paladin: Blessing of Sacrifice, and Warlock: Soul Link
    if( pVictim->m_damageSplitTarget.active)
        ires = pVictim->DoDamageSplitTarget(ires, spellInfo->School, false);

    SendSpellNonMeleeDamageLog(this, pVictim, spellID, ires, school, abs_dmg, dmg.resisted_damage, false, 0, critical, IsPlayer());

    if( ires > 0 ) // only deal damage if its >0
        DealDamage( pVictim, float2int32( res ), 2, 0, spellID );
    else
    {
        // we still have to tell the combat status handler we did damage so we're put in combat
        if( IsUnit() )
            castPtr<Unit>(this)->CombatStatus.OnDamageDealt(pVictim, 1);
    }

    if( IsPlayer() )
        castPtr<Player>(this)->m_casted_amount[school] = ( uint32 )res;

    if( (dmg.full_damage == 0 && abs_dmg) == 0 )
    {
        //Only pushback the victim current spell if it's not fully absorbed
        if( pVictim->GetCurrentSpell() )
            pVictim->GetCurrentSpell()->AddTime( school );
    }

//==========================================================================================
//==============================Post Damage Processing======================================
//==========================================================================================

    return res;
}

//*****************************************************************************************
//* SpellLog packets just to keep the code cleaner and better to read
//*****************************************************************************************

void WorldObject::SendSpellLog(WorldObject* Caster, WorldObject* Target, uint32 Ability, uint8 SpellLogType)
{
    if ((!Caster || !Target) && Ability)
        return;

    WorldPacket data(SMSG_SPELLLOGMISS, 26);
    data << uint32(Ability);            // spellid
    data << Caster->GetGUID();          // caster / player
    data << uint8(0);                   // unknown but I think they are const
    data << uint32(1);                  // unknown but I think they are const
    data << Target->GetGUID();          // target
    data << uint8(SpellLogType);        // spelllogtype
    Caster->SendMessageToSet(&data, true);
}

void WorldObject::SendSpellNonMeleeDamageLog( WorldObject* Caster, Unit* Target, uint32 SpellID, uint32 Damage, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToset )
{
    if ( !Caster || !Target )
        return;
    SpellEntry *sp = dbcSpell.LookupEntry(SpellID);
    if( !sp )
        return;

    uint32 overkill = Target->computeOverkill(Damage);
    uint32 Hit_flags = (0x00001|0x00004|0x00020);
    if(CriticalHit)
        Hit_flags |= 0x00002;

    uint32 dmg = Damage-AbsorbedDamage-ResistedDamage-BlockedDamage;
    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, 16+4+4+4+1+4+4+1+1+4+4+1);
    data << Target->GetGUID();
    data << Caster->GetGUID();
    data << uint32(SpellID);                // SpellID / AbilityID
    data << uint32(dmg);                    // All Damage
    data << uint32(overkill);               // Overkill
    data << uint8(SchoolMask(School));      // School
    data << uint32(AbsorbedDamage);         // Absorbed Damage
    data << uint32(ResistedDamage);         // Resisted Damage
    data << uint8(PhysicalDamage ? 1 : 0);  // Physical Damage (true/false)
    data << uint8(0);                       // unknown or it binds with Physical Damage
    data << uint32(BlockedDamage);          // Physical Damage (true/false)
    data << uint32(Hit_flags);
    data << uint8(0);
    Caster->SendMessageToSet( &data, bToset );
}

int32 WorldObject::event_GetInstanceID()
{
    // return -1 for non-inworld.. so we get our shit moved to the right thread
    if(!IsInWorld())
        return -1;
    else
        return m_instanceId;
}

void WorldObject::EventSpellHit(Spell* pSpell)
{
    if( IsInWorld() && pSpell->m_caster != NULL )
        pSpell->cast(false);
    else
        pSpell->Destruct();
}


bool WorldObject::CanActivate()
{
    switch(m_objectTypeId)
    {
    case TYPEID_UNIT:
        {
            if(!IsPet())
                return true;
        }break;

    case TYPEID_GAMEOBJECT:
        {
            if(castPtr<GameObject>(this)->HasAI() && GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID) != GAMEOBJECT_TYPE_TRAP)
                return true;
        }break;
    }

    return false;
}

void WorldObject::Activate(MapMgr* mgr)
{
    switch(m_objectTypeId)
    {
    case TYPEID_UNIT:
        if(IsVehicle())
            mgr->activeVehicles.insert(TO_VEHICLE(this));
        else
            mgr->activeCreatures.insert(castPtr<Creature>(this));
        break;

    case TYPEID_GAMEOBJECT:
        mgr->activeGameObjects.insert(castPtr<GameObject>(this));
        break;
    }

    Active = true;
}

void WorldObject::Deactivate(MapMgr* mgr)
{
    mgr->ActiveLock.Acquire();
    switch(m_objectTypeId)
    {
    case TYPEID_UNIT:
        {
            if(IsVehicle())
            {
                // check iterator
                if( mgr->__vehicle_iterator != mgr->activeVehicles.end() && (*mgr->__vehicle_iterator) == TO_VEHICLE(this) )
                    ++mgr->__vehicle_iterator;

                mgr->activeVehicles.erase(TO_VEHICLE(this));
            }
            else
            {
                // check iterator
                if( mgr->__creature_iterator != mgr->activeCreatures.end() && (*mgr->__creature_iterator) == castPtr<Creature>(this) )
                    ++mgr->__creature_iterator;

                mgr->activeCreatures.erase(castPtr<Creature>(this));
            }
        }break;

    case TYPEID_GAMEOBJECT:
        {
            // check iterator
            if( mgr->__gameobject_iterator != mgr->activeGameObjects.end() && (*mgr->__gameobject_iterator) == castPtr<GameObject>(this) )
                ++mgr->__gameobject_iterator;

            mgr->activeGameObjects.erase(castPtr<GameObject>(this));
        }break;
    }
    Active = false;
    mgr->ActiveLock.Release();
}

void WorldObject::SetZoneId(uint32 newZone)
{
    m_zoneId = newZone;
    if( m_objectTypeId == TYPEID_PLAYER && castPtr<Player>(this)->GetGroup() )
        castPtr<Player>(this)->GetGroup()->HandlePartialChange( PARTY_UPDATE_FLAG_ZONEID, castPtr<Player>(this) );
}

// These are our hardcoded values
uint32 GetZoneForMap(uint32 mapid, uint32 areaId)
{
    switch(mapid)
    {
        // These are hardcoded values to keep data in line
    case 44: return 796;
    case 169: return 1397;
    case 449: return 1519;
    case 450: return 1637;
    case 598: return 4131;
    default:
        {
            MapEntry *entry = dbcMap.LookupEntry(mapid);
            if(entry && areaId == 0xFFFF)
                return entry->linked_zone;
        }break;
    }
    return 0;
}

void WorldObject::UpdateAreaInfo(MapMgr *mgr)
{
    if(mgr == NULL && !IsInWorld())
    {
        m_areaFlags = 0;
        m_zoneId = m_areaId = 0;
        return;
    }
    else if(mgr == NULL)
        mgr = GetMapMgr();

    m_zoneId = m_areaId = mgr->GetAreaID(GetPositionX(), GetPositionY(), GetPositionZ());
    if(uint32 forcedZone = GetZoneForMap(mgr->GetMapId(), m_areaId))
        m_zoneId = m_areaId = forcedZone;
    AreaTableEntry* at = dbcAreaTable.LookupEntry(m_areaId);
    if(at != NULL && at->ZoneId) // Set our Zone on add to world!
        SetZoneId(at->ZoneId);

    m_areaFlags = OBJECT_AREA_FLAG_NONE;
    if(sVMapInterface.IsIncity(GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ()))
        m_areaFlags |= OBJECT_AREA_FLAG_INCITY;
    if(m_zoneId || m_areaId)
    {
        if(sWorld.CheckSanctuary(GetMapId(), m_zoneId, m_areaId))
            m_areaFlags |= OBJECT_AREA_FLAG_INSANCTUARY;
        AreaTableEntry* at = dbcAreaTable.LookupEntry(m_areaId);
        if(at == NULL)
            at = dbcAreaTable.LookupEntry(m_zoneId);
        if(at)
        {
            if(at->category == AREAC_CONTESTED)
                m_areaFlags |= OBJECT_AREA_FLAG_CONTESTED;
            if(at->category == AREAC_ALLIANCE_TERRITORY)
                m_areaFlags |= OBJECT_AREA_FLAG_ALLIANCE_ZONE;
            if(at->category == AREAC_HORDE_TERRITORY)
                m_areaFlags |= OBJECT_AREA_FLAG_HORDE_ZONE;
            if(at->AreaFlags & AREA_PVP_ARENA)
                m_areaFlags |= OBJECT_AREA_FLAG_ARENA_ZONE;
        }
    }
}

void WorldObject::PlaySoundToPlayer( Player* plr, uint32 sound_entry )
{
    if(plr == NULL || plr->GetSession() == NULL)
        return;

    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound_entry;
    plr->GetSession()->SendPacket( &data );
}

void WorldObject::PlaySoundToSet(uint32 sound_entry)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound_entry;
    SendMessageToSet(&data, true);
}

void WorldObject::SendAttackerStateUpdate( Unit* Target, dealdamage *dmg, uint32 realdamage, uint32 abs, uint32 blocked_damage, uint32 hit_status, uint32 vstate )
{
    if (!Target || !dmg)
        return;

    uint32 overkill = Target->computeOverkill(realdamage);
    uint32 schooldam = SchoolMask(dmg->school_type);
    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, 108);
    data << uint32(hit_status);
    data << GetGUID();
    data << Target->GetGUID();
    data << uint32(realdamage);                 // Realdamage;
    data << uint32(overkill);                   // Overkill
    data << uint8(1);                           // Damage type counter / swing type
    data << uint32(schooldam);                  // Damage school
    data << float(dmg->full_damage);            // Damage float
    data << uint32(dmg->full_damage);           // Damage amount

    if(hit_status & (HITSTATUS_ABSORBED | HITSTATUS_ABSORBED2))
    {
        data << (uint32)abs;                    // Damage absorbed
    }
    if(hit_status & (HITSTATUS_RESIST | HITSTATUS_RESIST2))
    {
        data << uint32(dmg->resisted_damage);   // Damage resisted
    }

    data << uint8(vstate);                      // new victim state
    data << uint32(0);
    data << uint32(0);

    if(hit_status & HITSTATUS_BLOCK)
        data << uint32(blocked_damage);         // Damage amount blocked

    if (hit_status & 0x00800000)
        data << uint32(0);                      // unknown

    if(hit_status & HITSTATUS_unk)
    {
        data << uint32(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << uint32(0);
    }

    SendMessageToSet(&data, IsPlayer());
}

bool WorldObject::IsInLineOfSight(WorldObject* pObj)
{
    float Onoselevel = 2.0f;
    float Tnoselevel = 2.0f;
    if(IsPlayer())
        Onoselevel = castPtr<Player>(this)->m_noseLevel;
    if(pObj->IsPlayer())
        Tnoselevel = castPtr<Player>(pObj)->m_noseLevel;

    if (GetMapMgr() && GetMapMgr()->CanUseCollision(this) && GetMapMgr()->CanUseCollision(pObj))
        return (sVMapInterface.CheckLOS( GetMapId(), GetInstanceID(), GetPhaseMask(), GetPositionX(), GetPositionY(), GetPositionZ() + Onoselevel + GetFloatValue(UNIT_FIELD_HOVERHEIGHT), pObj->GetPositionX(), pObj->GetPositionY(), pObj->GetPositionZ() + Tnoselevel + pObj->GetFloatValue(UNIT_FIELD_HOVERHEIGHT)) );
    else
        return true;
}

bool WorldObject::IsInLineOfSight(float x, float y, float z)
{
    float Onoselevel = 2.0f;
    if(IsPlayer())
        Onoselevel = castPtr<Player>(this)->m_noseLevel;

    if (GetMapMgr() && GetMapMgr()->CanUseCollision(this))
        return (sVMapInterface.CheckLOS( GetMapId(), GetInstanceID(), GetPhaseMask(), GetPositionX(), GetPositionY(), GetPositionZ() + Onoselevel + GetFloatValue(UNIT_FIELD_HOVERHEIGHT), x, y, z) );
    else
        return true;
}

bool WorldObject::PhasedCanInteract(WorldObject* pObj)
{
    return true;
}

// Returns the base cost of a spell
int32 WorldObject::GetSpellBaseCost(SpellEntry *sp)
{
    float cost = 0.0f;
    if( sp->ManaCostPercentage && IsUnit() )//Percentage spells cost % of !!!BASE!!! mana
    {
        if( sp->powerType == POWER_TYPE_MANA)
            cost = GetUInt32Value(UNIT_FIELD_BASE_MANA) * (sp->ManaCostPercentage / 100.0f);
        else cost = GetUInt32Value(UNIT_FIELD_BASE_HEALTH) * (sp->ManaCostPercentage / 100.0f);
    } else cost = (float)sp->ManaCost;

    return float2int32(cost); // Truncate zeh decimals!
}

void WorldObject::CastSpell( WorldObject* Target, SpellEntry* Sp, bool triggered )
{
    if( Sp == NULL )
        return;

    Spell* newSpell = new Spell(this, Sp, triggered, NULL);
    SpellCastTargets targets(0);
    if(Target)
    {
        if(Target->IsUnit())
            targets.m_targetMask |= TARGET_FLAG_UNIT;
        else targets.m_targetMask |= TARGET_FLAG_OBJECT;
        targets.m_unitTarget = Target->GetGUID();
    } else newSpell->GenerateTargets(&targets);
    newSpell->prepare(&targets);
}

void WorldObject::CastSpell( WorldObject* Target, uint32 SpellID, bool triggered )
{
    SpellEntry * ent = dbcSpell.LookupEntry(SpellID);
    if(ent == NULL) return;

    CastSpell(Target, ent, triggered);
}

void WorldObject::CastSpell( uint64 targetGuid, SpellEntry* Sp, bool triggered )
{
    if( Sp == NULL )
        return;

    SpellCastTargets targets(targetGuid);
    Spell* newSpell(new Spell(this, Sp, triggered, NULL));
    newSpell->prepare(&targets);
}

void WorldObject::CastSpell( uint64 targetGuid, uint32 SpellID, bool triggered )
{
    SpellEntry * ent = dbcSpell.LookupEntry(SpellID);
    if(ent == 0) return;

    CastSpell(targetGuid, ent, triggered);
}
