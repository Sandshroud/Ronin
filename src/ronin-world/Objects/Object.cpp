/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
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

Object::Object() : m_eventHandler(this), m_valuesCount(0), m_updateFlags(0), m_notifyFlags(0), m_objGuid(0), m_updateMask(0), m_inWorld(false) { }

void Object::Construct(WoWGuid guid, uint32 fieldCount)
{
    m_objGuid = guid;
    m_valuesCount = fieldCount;
    m_updateMask.SetCount(fieldCount);
    m_dynamicFields.SetCount(fieldCount);

    m_uint32Values = new uint32[m_valuesCount];
    memset(m_uint32Values, 0, sizeof(uint32)*m_valuesCount);

    SetUInt64Value(OBJECT_FIELD_GUID, m_objGuid);
    SetUInt32Value(OBJECT_FIELD_ENTRY, m_objGuid.getEntry());

    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.f);
    SetTypeFlags(TYPEMASK_TYPE_OBJECT);
    m_objType = TYPEID_OBJECT;

    m_loot.gold = 0;
    m_lootGenerated = false;
    m_looted = false;
}

Object::~Object()
{
    delete [] m_uint32Values;
}

void Object::Init()
{
    m_eventHandler.Init();
}

void Object::Destruct()
{
    delete this;
}

void Object::Update(uint32 msTime, uint32 diff)
{
    m_eventHandler.Update(diff, UNIXTIME);
}

void Object::SetByte(uint16 index, uint8 flag, uint8 value)
{
    ASSERT( index < m_valuesCount );

    if(flag >= 4)
    {
        sLog.outError("Object::SetByteValue: wrong offset %u", flag);
        return;
    }

    if(uint8(m_uint32Values[ index ] >> (flag * 8)) == value)
        return;

    m_uint32Values[ index ] &= ~uint32(uint32(0xFF) << (flag * 8));
    m_uint32Values[ index ] |= uint32(uint32(value) << (flag * 8));
    SetUpdateField(index);
}

void Object::SetByteFlag(const uint16 index, const uint8 flag, uint8 newFlag)
{
    if( HasByteFlag(index,flag,newFlag))
        return;

    SetByte(index, flag, GetByte(index,flag)|newFlag);
    SetUpdateField(index);
}

void Object::RemoveByteFlag(const uint16 index, const uint8 flag, uint8 checkFlag)
{
    if( !HasByteFlag(index,flag,checkFlag))
        return;

    SetByte(index,flag, GetByte(index,flag) & ~checkFlag );
    SetUpdateField(index);
}

bool Object::HasByteFlag(const uint16 index, const uint8 flag, uint8 checkFlag)
{
    if( GetByte(index,flag) & checkFlag )
        return true;
    return false;
}

void Object::SetUInt16Value(uint16 index, uint8 offset, uint16 value)
{
    ASSERT( index < m_valuesCount );
    if (uint16(m_uint32Values[index] >> (offset * 16)) == value)
        return;

    m_uint32Values[index] &= ~uint32(uint32(0xFFFF) << (offset * 16));
    m_uint32Values[index] |= uint32(uint32(value) << (offset * 16));
    SetUpdateField(index);
}

void Object::SetUInt32Value( const uint16 index, const uint32 value )
{
    ASSERT( index < m_valuesCount );
    if(m_uint32Values[index] == value)
        return;

    m_uint32Values[ index ] = value;
    SetUpdateField(index);
}

void Object::SetUInt64Value( const uint16 index, const uint64 value )
{
    assert( index + 1 < m_valuesCount );
    if(m_uint32Values[index] == ((uint32*)&value)[0] && m_uint32Values[index+1] == ((uint32*)&value)[1])
        return;

    m_uint32Values[ index ] = ((uint32*)&value)[0];
    m_uint32Values[ index + 1 ] = ((uint32*)&value)[1];

    SetUpdateField(index);
    SetUpdateField(index+1);
}

void Object::SetFlag( const uint16 index, uint32 newFlag )
{
    ASSERT( index < m_valuesCount );

    uint32 prevValue = m_uint32Values[index];
    m_uint32Values[index] |= newFlag;
    // Only set the field for update if the value changes
    if(prevValue != m_uint32Values[index])
        SetUpdateField(index);
}

void Object::RemoveFlag( const uint16 index, uint32 oldFlag )
{
    ASSERT( index < m_valuesCount );
    if((m_uint32Values[ index ] & oldFlag) == 0)
        return;

    m_uint32Values[ index ] &= ~oldFlag;
    SetUpdateField(index);
}

void Object::SetFloatValue( const uint16 index, const float value )
{
    ASSERT( index < m_valuesCount );
    if(m_floatValues[index] == value)
        return;

    m_floatValues[ index ] = value;
    SetUpdateField(index);
}

void Object::ModFloatValue(const uint16 index, const float value )
{
    ASSERT( index < m_valuesCount );
    m_floatValues[ index ] += value;
    SetUpdateField(index);
}

void Object::ModSignedInt32Value(uint16 index, int32 value )
{
    ASSERT( index < m_valuesCount );
    if(value == 0)
        return;

    m_uint32Values[ index ] += value;
    SetUpdateField(index);
}

void Object::ModUnsigned32Value(uint16 index, int32 mod)
{
    ASSERT( index < m_valuesCount );
    if(mod == 0)
        return;

    m_uint32Values[ index ] += mod;
    if( (int32)m_uint32Values[index] < 0 )
        m_uint32Values[index] = 0;
    SetUpdateField(index);
}

uint32 Object::GetModPUInt32Value(const uint16 index, const int32 value)
{
    ASSERT( index < m_valuesCount );
    int32 basevalue = (int32)m_uint32Values[ index ];
    return ((basevalue*value)/100);
}

void Object::SetUpdateField(uint16 index)
{
    if(!IsInWorld())
        return;

    m_updateMask.SetBit( index );
    OnFieldUpdated(index);
}

bool Object::_SetUpdateBits(UpdateMask *updateMask, uint32 updateFlags)
{
    bool res = false;
    uint16 typeMask = GetTypeFlags(), offset = 0, *flags, fLen = 0;
    for(uint8 f = 0; f < 10; f++)
    {
        if(typeMask & 1<<f)
        {
            GetUpdateFieldData(f, flags, fLen);
            for(uint32 i = 0; i < fLen; i++, offset++)
            {
                if(!m_updateMask.GetBit(offset))
                    continue;
                if(flags[i] != 0 && (flags[i] & updateFlags) == 0)
                    continue;
                res = true;
                updateMask->SetBit(offset);
            }
        }
    }
    return res;
}

uint16 Object::GetUpdateFlag(Player *target)
{
    uint16 flag = (target == this ? UF_FLAGMASK_SELF : UF_FLAGMASK_PUBLIC);
    if(target)
    {
        switch (GetTypeId())
        {
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
            {
                if (target->GetGUID() == castPtr<Unit>(this)->GetUInt64Value(UNIT_FIELD_SUMMONEDBY))
                    flag |= UF_FLAG_OWNER;
                else if (target->GetGUID() == castPtr<Unit>(this)->GetUInt64Value(UNIT_FIELD_CREATEDBY))
                    flag |= UF_FLAG_OWNER;
                if (IsPlayer() && castPtr<Player>(this)->InGroup() && castPtr<Player>(this)->GetGroupGuid() == target->GetGroupGuid())
                    flag |= UF_FLAG_PARTY_MEMBER;
            }break;
        case TYPEID_GAMEOBJECT:
            {
                if (target->GetGUID() == castPtr<GameObject>(this)->GetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY))
                    flag |= UF_FLAG_OWNER;
            }break;
        case TYPEID_DYNAMICOBJECT:
            {
                if (target->GetGUID() == castPtr<DynamicObject>(this)->GetCasterGuid())
                    flag |= UF_FLAG_OWNER;
            }break;
        case TYPEID_CORPSE:
            {
                if (target->GetGUID() == castPtr<Corpse>(this)->GetUInt64Value(CORPSE_FIELD_OWNER))
                    flag |= UF_FLAG_OWNER;
            }break;
        case TYPEID_ITEM:
            flag = 0xFFFF;
            break;
        }
    }

    return flag;
}

void Object::GetUpdateFieldData(uint8 type, uint16 *&flags, uint16 &length)
{
    switch (type)
    {
    case TYPEID_OBJECT: { length = OBJECT_LENGTH; flags = ObjectUpdateFieldFlags; }break;
    case TYPEID_ITEM: { length = ITEM_LENGTH; flags = ItemUpdateFieldFlags; }break;
    case TYPEID_CONTAINER: { length = CONTAINER_LENGTH; flags = ContainerUpdateFieldFlags; }break;
    case TYPEID_UNIT: { length = UNIT_LENGTH; flags = UnitUpdateFieldFlags; }break;
    case TYPEID_PLAYER: { length = PLAYER_LENGTH; flags = PlayerUpdateFieldFlags; }break;
    case TYPEID_GAMEOBJECT: { length = GAMEOBJECT_LENGTH; flags = GameObjectUpdateFieldFlags; }break;
    case TYPEID_DYNAMICOBJECT: { length = DYNAMICOBJECT_LENGTH; flags = DynamicObjectUpdateFieldFlags; }break;
    case TYPEID_CORPSE: { length = CORPSE_LENGTH; flags = CorpseUpdateFieldFlags; }break;
    case TYPEID_AREATRIGGER: { length = AREATRIGGER_LENGTH; flags = AreaTriggerUpdateFieldFlags; }break;
    }
}

uint32 Object::BuildCreateUpdateBlockForPlayer(ByteBuffer *data, Player* target)
{
    uint8 updatetype = UPDATETYPE_CREATE_OBJECT;
    // Players or player linked units
    if(IsPlayer() || IsCorpse() || ((IsDynamicObj() || IsTotem() || IsSummon()) && IsSummonerPlayer()))
        updatetype = UPDATETYPE_CREATE_PLAYEROBJ;

    uint16 updateFlags = m_updateFlags;
    if(target == this) // player creating self
        updateFlags |= UPDATEFLAG_SELF;
    else if(Unit *unit = (IsUnit() ? castPtr<Unit>(this) : NULL))
    {
        if(unit->GetUInt64Value(UNIT_FIELD_TARGET))
            updateFlags |= UPDATEFLAG_HAS_TARGET;

        if(unit->GetVehicleKitId())
            updateFlags |= UPDATEFLAG_VEHICLE;

        if (unit->GetAIAnimKitId() || unit->GetMovementAnimKitId() || unit->GetMeleeAnimKitId())
            updateFlags |= UPDATEFLAG_ANIMKITS;
    }
    else if(IsGameObject())
    {
        switch(GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID))
        {
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
    }

    // build our actual update
    *data << uint8(updatetype);
    *data << m_objGuid.asPacked();
    *data << uint8(GetTypeId());
    // Send our object type update
    _BuildMovementUpdate(data, updateFlags, target);
    // this will cache automatically if needed
    _BuildCreateValuesUpdate( data, target );

    return 1; // update count: 1 ;)
}

uint32 Object::BuildValuesUpdateBlockForPlayer(ByteBuffer *data, Player *target, uint32 updateFlags, uint32 expectedField)
{
    UpdateMask updateMask(m_valuesCount);
    if(expectedField || _SetUpdateBits(&updateMask, updateFlags))
    {
        if(expectedField != 0) updateMask.SetBit(expectedField);
        *data << uint8(UPDATETYPE_VALUES);     // update type == update
        *data << m_objGuid.asPacked();
        _BuildChangedValuesUpdate( data, target, &updateMask );
        return 1;
    }
    return 0;
}

void Object::OnUpdateProcess()
{
    if(IsUnit() && HasUpdateField(UNIT_FIELD_HEALTH))
        castPtr<Unit>(this)->EventHealthChangeSinceLastUpdate();
    if(IsUnit() && HasUpdateField(UNIT_FIELD_DISPLAYID))
        castPtr<Unit>(this)->EventModelChange();
}

//=======================================================================================
//  Creates an update block containing all data needed for a new object
//=======================================================================================
void Object::_BuildCreateValuesUpdate(ByteBuffer * data, Player* target)
{
    ByteBuffer fields; // Cut down on value cycling for players that aren't us
    UpdateMask mask((IsPlayer() && target != this) ? PLAYER_END_NOT_SELF : m_valuesCount);
    uint16 typeMask = GetTypeFlags(), uFlag = GetUpdateFlag(target), offset = 0, *flags, fLen = 0;
    for(uint8 f = 0; f < 10; f++)
    {
        if((typeMask & 1<<f) == 0)
            continue;
        GetUpdateFieldData(f, flags, fLen);
        for(uint16 i = 0; i < fLen; i++, offset++)
        {
            if(offset >= mask.GetCount())
                break;
            if(m_uint32Values[offset] == 0)
                continue;

            if(flags[i] & (uFlag|m_notifyFlags))
            {
                mask.SetBit(offset);
                fields << _GetSwappedValueForUpdate(offset, flags[i], target);
            }
        }
    }

    *data << uint8(mask.GetBlockCount());
    data->append(mask.GetMask(), mask.GetLength());
    data->append(fields.contents(), fields.size());
}

//=======================================================================================
//  Creates an update block with the values of this object as
//  determined by the updateMask.
//=======================================================================================
void Object::_BuildChangedValuesUpdate(ByteBuffer * data, Player* target, UpdateMask *updateMask)
{
    WPAssert( updateMask && updateMask->GetCount() == m_valuesCount );
    *data << uint8(updateMask->GetBlockCount());
    data->append( updateMask->GetMask(), updateMask->GetLength() );
    uint16 typeMask = GetTypeFlags(), offset = 0, *flags, fLen = 0;
    for(uint8 f = 0; f < 10; f++)
    {
        if((typeMask & 1<<f) == 0)
            continue;
        GetUpdateFieldData(f, flags, fLen);
        for(uint16 i = 0; i < fLen; i++, offset++)
        {
            if(offset >= updateMask->GetCount())
                break;
            if(!updateMask->GetBit(offset))
                continue;
            *data << _GetSwappedValueForUpdate(offset, flags[i], target);
        }
    }
}

uint32 Object::_GetSwappedValueForUpdate(uint32 index, uint16 fieldFlag, Player *target)
{
    if((fieldFlag & UF_FLAG_DYNAMIC) == 0 && !m_dynamicFields.GetBit(index))
        return m_uint32Values[index];

    uint32 val = m_uint32Values[index];
    if(IsUnit() && index == UNIT_DYNAMIC_FLAGS && ((val & U_DYN_FLAG_TAPPED_BY_PLAYER) == 0))
    {
        // Manage our own loot flags
        val &= ~(U_DYN_FLAG_LOOTABLE|U_DYN_FLAG_TAGGED_BY_OTHER);
        Creature *curObj = castPtr<Creature>(this);
        if(!curObj->IsTaggedByPlayer(target))
            val |= U_DYN_FLAG_TAGGED_BY_OTHER;
        else if(curObj->isDead() && curObj->GetLoot()->HasLoot(target)) 
            val |= U_DYN_FLAG_LOOTABLE;
    }
    else if(IsUnit())
    {
        switch(index)
        {
        case PLAYER_FIELD_MOD_HASTE:
        case PLAYER_FIELD_MOD_RANGED_HASTE:
        case UNIT_MOD_CAST_HASTE:
            {
                union {
                    uint32 intVal;
                    float floatVal;
                };

                floatVal = (100.f-m_floatValues[index])/100.f;
                val = intVal;
            }break;
        
        }
    }
    else if(IsGameObject())
    {
        GameObject *gThis = castPtr<GameObject>(this);
        switch(index)
        {
        case GAMEOBJECT_DYNAMIC:
            break;
        case GAMEOBJECT_FLAGS:
            {   // Clear our in use flags if we can use it
                if(gThis->CanUse(target))
                    val &= ~GO_FLAG_IN_USE;
            }break;
        case GAMEOBJECT_BYTES_1:
            {
                if(gThis->CanUse(target))
                {   // Set us ready to activate
                    val &= ~0x000000FF;
                    val |= GO_STATE_READY_TO_ACTIVATE;
                }
            }break;
        }
    }
    return val;
}

///////////////////////////////////////////////////////////////
/// Build the Movement Data portion of the update packet
/// Fills the data with this object's movement/speed info
void Object::_BuildMovementUpdate(ByteBuffer * data, uint16 flags, Player* target )
{
    GameObject *tThis = NULL;
    Unit *uThis = IsUnit() ? castPtr<Unit>(this) : NULL;
    ByteBuffer livingBuffer, stopFrameBuffer;
    if (IsGameObject() && castPtr<GameObject>(this)->GetType() == GAMEOBJECT_TYPE_TRANSPORT)
        tThis = castPtr<GameObject>(this);

    data->WriteBit(0);
    data->WriteBit(0);
    data->WriteBit(flags & UPDATEFLAG_ROTATION);
    data->WriteBit(flags & UPDATEFLAG_ANIMKITS);
    data->WriteBit(flags & UPDATEFLAG_HAS_TARGET);
    data->WriteBit(flags & UPDATEFLAG_SELF);
    data->WriteBit(flags & UPDATEFLAG_VEHICLE);
    data->WriteBit(flags & UPDATEFLAG_LIVING);
    data->WriteBits(tThis ? tThis->BuildStopFrameData(&stopFrameBuffer) : 0, 24);
    data->WriteBit(0);
    data->WriteBit(flags & UPDATEFLAG_GO_TRANSPORT_POS);
    data->WriteBit(flags & UPDATEFLAG_STATIONARY_POS);
    data->WriteBit(flags & UPDATEFLAG_TRANSPORT_ARR);
    data->WriteBit(0);
    data->WriteBit(flags & UPDATEFLAG_TRANSPORT);

    if(flags & UPDATEFLAG_LIVING)
    {
        if(target == this) uThis->GetMovementInterface()->SetSelfTempData(true);
        uThis->GetMovementInterface()->WriteObjectUpdate(data, &livingBuffer);
        if(target == this) uThis->GetMovementInterface()->SetSelfTempData(false);
    }

    // used only with GO's, placeholder
    if (flags & UPDATEFLAG_GO_TRANSPORT_POS)
    {
        WoWGuid guid = uThis->GetTransportGuid();
        MovementInterface *mi = uThis->GetMovementInterface();
        data->WriteBit(guid[5]);
        data->WriteBit(mi->GetTransportVehicleId() > 0);// Has GO transport time 3
        data->WriteBit(guid[0]);
        data->WriteBit(guid[3]);
        data->WriteBit(guid[6]);
        data->WriteBit(guid[1]);
        data->WriteBit(guid[4]);
        data->WriteBit(guid[2]);
        data->WriteBit(mi->GetTransportTime2() > 0);    // Has GO transport time 2
        data->WriteBit(guid[7]);
    }

    if(flags & UPDATEFLAG_HAS_TARGET)
    {
        WoWGuid targetGuid = GetUInt64Value(UNIT_FIELD_TARGET);
        data->WriteBitString(4, targetGuid[2], targetGuid[7], targetGuid[0], targetGuid[4]);
        data->WriteBitString(4, targetGuid[5], targetGuid[6], targetGuid[1], targetGuid[3]);
    }

    if (flags & UPDATEFLAG_ANIMKITS)
    {
        Unit *uThis = castPtr<Unit>(this);
        data->WriteBit(uThis->GetAIAnimKitId() == 0);
        data->WriteBit(uThis->GetMovementAnimKitId() == 0);
        data->WriteBit(uThis->GetMeleeAnimKitId() == 0);
    }

    data->FlushBits();

    // Now we do byte data
    data->append(stopFrameBuffer.contents(), stopFrameBuffer.size());

    if(flags & UPDATEFLAG_LIVING)
        data->append(livingBuffer.contents(), livingBuffer.size());

    if(flags & UPDATEFLAG_VEHICLE)
        *data << float(castPtr<WorldObject>(this)->GetOrientation()) << uint32(castPtr<Unit>(this)->GetVehicleKitId());

    if (flags & UPDATEFLAG_GO_TRANSPORT_POS)
    {
        Unit *uThis = castPtr<Unit>(this);
        WoWGuid guid = uThis->GetTransportGuid();
        MovementInterface *mi = uThis->GetMovementInterface();
        float tX, tY, tZ, tO;
        mi->GetTransportPosition(tX, tY, tZ, tO);
        data->WriteByteSeq(guid[0]);
        data->WriteByteSeq(guid[5]);
        if (uint32 vehicleId = mi->GetTransportVehicleId())
            *data << uint32(vehicleId);
        data->WriteByteSeq(guid[3]);
        *data << float(tX);
        data->WriteByteSeq(guid[4]);
        data->WriteByteSeq(guid[6]);
        data->WriteByteSeq(guid[1]);
        *data << uint32(mi->GetTransportTime());
        *data << float(tY);
        data->WriteByteSeq(guid[2]);
        data->WriteByteSeq(guid[7]);
        *data << float(tZ);
        *data << int8(mi->GetTransportSeat());
        *data << float(tO);
        if (uint32 transportTime2 = mi->GetTransportTime2())
            *data << uint32(transportTime2);
    }

    if(flags & UPDATEFLAG_ROTATION)
        *data << int64(GameObject::PackRotation(&castPtr<GameObject>(this)->m_rotation)); //blizz 64bit rotation

    if (flags & UPDATEFLAG_TRANSPORT_ARR)
    {
        *data << float(0.0f) << float(0.0f) << float(0.0f) << float(0.0f);
        *data << uint8(0);
        *data << float(0.0f) << float(0.0f) << float(0.0f) << float(0.0f);
        *data << float(0.0f) << float(0.0f) << float(0.0f) << float(0.0f);
        *data << float(0.0f) << float(0.0f) << float(0.0f) << float(0.0f);
    }

    if (flags & UPDATEFLAG_STATIONARY_POS)
    {
        WorldObject *obj = castPtr<WorldObject>(this);
        *data << float(obj->GetSpawnO());
        *data << float(obj->GetSpawnX());
        *data << float(obj->GetSpawnY());
        *data << float(obj->GetSpawnZ());
    }

    if(flags & UPDATEFLAG_HAS_TARGET)
    {
        WoWGuid targetGuid = GetUInt64Value(UNIT_FIELD_TARGET);
        data->WriteSeqByteString(8, targetGuid, 4, 0, 3, 5, 7, 6, 2, 1);
    }

    if (flags & UPDATEFLAG_ANIMKITS)
    {
        Unit *uThis = castPtr<Unit>(this);
        if (uint16 animKit = uThis->GetAIAnimKitId())
            *data << uint16(animKit);
        if (uint16 animKit = uThis->GetMovementAnimKitId())
            *data << uint16(animKit);
        if (uint16 animKit = uThis->GetMeleeAnimKitId())
            *data << uint16(animKit);
    }

    if(flags & UPDATEFLAG_TRANSPORT)
        *data << uint32(tThis->GetTransportTick());
}

void Object::DestroyForPlayer(Player* target, bool anim)
{
    if(target == NULL)
        return;

    WorldPacket data(SMSG_DESTROY_OBJECT, 9);
    data << GetGUID();
    data << uint8(anim ? 1 : 0);
    target->PushPacket( &data );
}

void Object::ClearLoot()
{
    // better cancel any rolls just in case.
    for(std::vector<__LootItem>::iterator itr = m_loot.items.begin(); itr != m_loot.items.end(); itr++)
    {
        if( itr->roll != NULL )
        {
            itr->roll = NULL; // buh-bye!
        }
    }

    m_loot.gold = 0;
    m_loot.items.clear();
    m_loot.looters.clear();
    m_lootGenerated = false;
}

//===============================================
// Object Cell Management functions
//===============================================

unsigned int ObjectCellManager::VisibleCellRange = 1;
bool ObjectCellManager::cutCorners = (VisibleCellRange > 1);

void ObjectCellManager::OnRelocate(MapInstance *instance, LocationVector &destination)
{
    uint32 posX = _getCellId(destination.x), posY = _getCellId(destination.y);
    if(!(posX < _lowX || posX > _highX || posY < _lowY || posY > _highY))
        return;

    std::set<uint32> cellSet;
    for(uint16 x = _lowX; x <= _highX; x++)
    {
        for(uint16 y = _lowY; y <= _highY; y++)
        {
            if(cutCorners && isCorner(x, y, _lowX, _highX, _lowY, _highY, VisibleCellRange))
                continue;

            cellSet.insert(_makeCell(x, y));
        }
    }

    instance->RemoveCellData(_object, cellSet, true);
}

void ObjectCellManager::ClearInRangeObjects(MapInstance *instance)
{
    _visRange = _currX = _currY = 0;
    _lowX = _lowY = _highX = _highY = 0;

    // Clear our visible spectrum,
    while(!m_visibleTo.empty())
    {
        WoWGuid guid = *m_visibleTo.begin();
        m_visibleTo.erase(m_visibleTo.begin());
        if(Player *plr = instance->GetPlayer(guid))
            plr->RemoveIfVisible(instance->GetMapId(), _object);
    }

    instance->RemoveCachedCell(_object->GetGUID());
    if(MapCell *cell = _object->GetMapCell())
        cell->RemoveObject(_object);
    _object->SetMapCell(NULL);
}

void ObjectCellManager::OnUnitDeath(MapInstance *instance)
{
    instance->ClearCombatTimers(_object->GetGUID());
}

void ObjectCellManager::PostRemoveFromWorld()
{
    _visRange = _currX = _currY = 0;
    _lowX = _lowY = _highX = _highY = 0;
}

void ObjectCellManager::UpdateVisibility(MapInstance *instance)
{
    std::vector<uint32> cellSet;
    // Use pre-existing cell range function
    FillCellRange(&cellSet);
    // Push cell set to map instance
    instance->UpdateObjectCellVisibility(_object, &cellSet);
}

bool ObjectCellManager::hasCell(uint32 cellId)
{
    std::pair<uint16, uint16> cellPair = unPack(cellId);
    if(cutCorners && isCorner(cellPair.first, cellPair.second, _lowX, _highX, _lowY, _highY, VisibleCellRange))
        return false;
    return (cellPair.first >= _lowX && cellPair.first <= _highX) && (cellPair.second >= _lowY && cellPair.second <= _highY);
}

void ObjectCellManager::FillCellRange(std::vector<uint32> *fillVector)
{
    // Push our current cell into the vector
    fillVector->push_back(_makeCell(_currX, _currY));

    // Push based on range calculations
    for(uint16 x = _lowX; x <= _highX; x++)
    {
        for(uint16 y = _lowY; y <= _highY; y++)
        {
            if(x == _currX && y == _currY)
                continue;
            if(cutCorners && isCorner(x, y, _lowX, _highX, _lowY, _highY, VisibleCellRange))
                continue;

            fillVector->push_back(_makeCell(x, y));
        }
    }
}

void ObjectCellManager::CreateCellRange(std::vector<uint32> *fillVector, float range)
{
    if(range <= 1.f)
    {
        FillCellRange(fillVector);
        return;
    }

    // Push our current cell into the vector
    fillVector->push_back(_makeCell(_currX, _currY));

    // Push based on range calculations
    uint32 lowX = std::max<uint32>(_getCellId(_object->GetPositionX()-range), _lowX), highX = std::min<uint32>(_getCellId(_object->GetPositionX()+range), _highX);
    uint32 lowY = std::max<uint32>(_getCellId(_object->GetPositionY()-range), _lowY), highY = std::min<uint32>(_getCellId(_object->GetPositionY()+range), _highY);
    if(highX < lowX) std::swap(lowX, highX);
    if(highY < lowY) std::swap(lowY, highY);
    for(uint16 x = lowX; x <= highX; x++)
    {
        for(uint16 y = lowY; y <= highY; y++)
        {
            if(x == _currX && y == _currY)
                continue;
            fillVector->push_back(_makeCell(x, y));
        }
    }
}

void ObjectCellManager::CreateCellRange(std::vector<uint32> *fillVector, uint32 range)
{
    if(range == 0)
    {
        FillCellRange(fillVector);
        return;
    }

    uint32 lowX = _currX-range, highX = _currX+range;
    uint32 lowY = _currY-range, highY = _currY+range;
    for(uint16 x = lowX; x <= highX; x++)
        for(uint16 y = lowY; y <= highY; y++)
            fillVector->push_back(_makeCell(x, y));
}

bool ObjectCellManager::IsLocationChanged(float x, float y, float z)
{
    float dist = LocationVector::DistanceSq(x, y, z, _luX, _luY, _luZ);
    if(RONIN_UTIL::fuzzyGt(dist, 4.f))
    {
        _luX = x;
        _luY = y;
        _luZ = z;
        return true;
    }
    return false;
}

void ObjectCellManager::ConstructCellData(float x, float y, float range, std::vector<uint32> *fillVector)
{
    // Update position pair
    float dist = std::min<float>(range, TileSize); // Make sure we're less than an entire tile
    uint32 lowX = _getCellId(x-dist), highX = _getCellId(x+dist), lowY = _getCellId(y-dist), highY = _getCellId(y+dist);
    if(highX < lowX) std::swap(lowX, highX);
    if(highY < lowY) std::swap(lowY, highY);
    for(uint16 x = lowX; x <= highX; x++)
        for(uint16 y = lowY; y <= highY; y++)
            fillVector->push_back(_makeCell(x, y));

}

void ObjectCellManager::Update(MapInstance *instance, uint32 msTime, uint32 uiDiff)
{
    //Ny
}

void ObjectCellManager::SetCurrentCell(MapInstance *instance, float newX, float newY, float newZ, uint8 cellRange)
{
    std::set<uint32> prev;
    std::vector<uint32> newCells;
    if(instance && _visRange)
    {
        for(uint16 x = _lowX; x <= _highX; x++)
        {
            for(uint16 y = _lowY; y <= _highY; y++)
            {
                if(cutCorners && isCorner(x, y, _lowX, _highX, _lowY, _highY, VisibleCellRange))
                    continue;
                prev.insert(_makeCell(x, y));
            }
        }
    }

    _luX = newX;
    _luY = newY;
    _luZ = newZ;

    _visRange = cellRange;
    _currX = _getCellId(_luX);
    _currY = _getCellId(_luY);
    _lowX = _currX >= cellRange ? _currX-cellRange : 0;
    _lowY = _currY >= cellRange ? _currY-cellRange : 0;
    _highX = std::min<uint16>(_currX+cellRange, _sizeX-1);
    _highY = std::min<uint16>(_currY+cellRange, _sizeY-1);

    // Return here if we're a soft load(instance == NULL)
    if(instance == NULL)
        return;

    for(uint16 x = _lowX; x <= _highX; x++)
    {
        for(uint16 y = _lowY; y <= _highY; y++)
        {
            if(cutCorners && isCorner(x, y, _lowX, _highX, _lowY, _highY, VisibleCellRange))
                continue;
            uint32 cellId = _makeCell(x, y);
            if(prev.find(cellId) != prev.end())
            {   // We already have this cell, remove from previous cell removal set
                prev.erase(cellId); 
                continue;
            }

            // Add new cells to processing vector
            newCells.push_back(cellId);
        }
    }

    // Update for our current cell here, other cell updates will occur in WorldObject::Update
    instance->UpdateObjectCellVisibility(_object, &newCells);
    // Push calls to remove cell data
    instance->RemoveCellData(_object, prev, false);
}

uint32 ObjectCellManager::_getCellId(float pos)
{
    return getId(pos, _cellSize, _maxX);
}

//===============================================
// WorldObject class functions
//===============================================
WorldObject::WorldObject() : Object(), m_position(0,0,0,0), m_mapInstance(NULL), m_isMarkedForCleanup(false), m_isManagedBulkSpawn(true) { }

void WorldObject::Construct(WoWGuid guid, uint32 fieldCount)
{
    Object::Construct(guid, fieldCount);

    m_mapId = -1;
    m_wmoId = m_zoneId = m_areaId = 0;
    m_phaseMask = 0xFFFF;
    m_areaFlags = 0;
    m_lastMovementZone = 0;
    m_lastMovementArea = 0;

    m_mapCell = 0;

    m_factionTemplate = NULL;

    m_instanceId = 0;
    m_inactiveFlags = 0;
    m_objDeactivationTimer = 0;

    m_cellManager = guid.getHigh() ? new ObjectCellManager(this) : new PlayerCellManager(this);
}

WorldObject::~WorldObject( )
{

}

void WorldObject::Init()
{

}

void WorldObject::Destruct()
{
    if(IsInWorld())
        RemoveFromWorld();

    if(GetMapCell())
    {
        // Remove object from cell
        GetMapCell()->RemoveObject(this);

        // Unset object's cell
        SetMapCell(NULL);
    }

    m_factionTemplate = NULL;

    m_mapId = -1;
    m_wmoId = m_zoneId = m_areaId = 0;
    m_areaFlags = 0;
    m_lastMovementZone = 0;
    m_instanceId = -1;

    if(m_isManagedBulkSpawn == false)
        Object::Destruct();
}

void WorldObject::Cleanup()
{
    if(m_isMarkedForCleanup)
        return;

    if(m_mapInstance)
        m_mapInstance->QueueCleanup(this);
    else Destruct();
}

void WorldObject::Update(uint32 msTime, uint32 uiDiff)
{
    Object::Update(msTime, uiDiff);

    // Update our internal cell processor
    m_cellManager->Update(GetMapInstance(), msTime, uiDiff);
}

void WorldObject::InactiveUpdate(uint32 msTime, uint32 uiDiff)
{
    if((m_inactiveFlags & OBJECT_INACTIVE_FLAG_INACTIVE) == 0)
        return;

    if(m_inactiveFlags & OBJECT_INACTIVE_FLAG_DESPAWNED)
    {
        if(m_objDeactivationTimer <= uiDiff)
            m_inactiveFlags &= ~OBJECT_INACTIVE_FLAG_DESPAWNED;
        else
        {
            m_objDeactivationTimer -= uiDiff;
            return;
        }
    }

    if(m_inactiveFlags & OBJECT_INACTIVE_FLAG_CONDITION)
    {
        if(m_objDeactivationTimer > uiDiff)
        {
            m_objDeactivationTimer -= uiDiff;
            return;
        }
        else if(!m_mapInstance->HasActivatedCondition(getConditionID(), this))
        {
            m_objDeactivationTimer = 5000;
            return;
        } else m_objDeactivationTimer = 0;
    }

    if(m_inactiveFlags & OBJECT_INACTIVE_FLAG_EVENTS)
    {
        if(m_objDeactivationTimer > uiDiff)
        {
            m_objDeactivationTimer -= uiDiff;
            return;
        }
        else if(!sWorld.HasActiveEvents(this))
        {
            m_objDeactivationTimer = 5000;
            return;
        } else m_objDeactivationTimer = 0;
    }

    if(m_objDeactivationTimer > uiDiff)
    {
        m_objDeactivationTimer -= uiDiff;
        return;
    }

    if(!CanReactivate())
    {   // Try again in 5 seconds
        m_objDeactivationTimer = 5000;
        return;
    }

    m_objDeactivationTimer = 0;
    m_inactiveFlags &= ~OBJECT_INACTIVE_FLAG_INACTIVE;
    if(MapCell *cell = m_mapCell)
        cell->ReactivateObject(this);
    Reactivate();
}

//Unlike addtoworld it pushes it directly ignoring add pool
//this can only be called from the thread of mapmgr!!!
void WorldObject::PushToWorld(MapInstance* instance, uint32 msTime)
{
    ASSERT(instance != NULL);
    if(instance == NULL)
    {
        // Reset these so session will get updated properly.
        if(IsPlayer())
        {
            sLog.Error("WorldObject","Kicking Player %s due to empty MapMgr",castPtr<Player>(this)->GetName());
            castPtr<Player>(this)->GetSession()->LogoutPlayer();
        }
        return; //instance add failed
    }

    OnPreSetInWorld();

    m_mapId = instance->GetMapId();
    m_instanceId = instance->GetInstanceID();

    // Call a quick exploration event with a mgr override
    EventExploration(instance);

    // Call area update
    UpdateAreaInfo(instance);

    // Set our map manager
    m_mapInstance = instance;

    // Call prepush after setting map pointer
    OnPrePushToWorld();

    // Push into our map pool
    instance->PushObject(this);

    // call virtual function to handle stuff.. :P
    OnPushToWorld(msTime);

    // Set Object in world
    Object::SetInWorld(true);
}

void WorldObject::RemoveFromWorld()
{
    ASSERT(m_mapInstance);

    // clear loot
    ClearLoot();

    MapInstance* m = m_mapInstance;
    m_mapInstance = NULL;

    m->RemoveObject(this);

    // Set Object out of world
    Object::SetInWorld(false);

    m_cellManager->PostRemoveFromWorld();
}

void WorldObject::Deactivate(uint32 reactivationTime)
{
    if(IsPlayer())
        return;

    m_inactiveFlags |= OBJECT_INACTIVE_FLAG_INACTIVE;
    if(reactivationTime)
        m_objDeactivationTimer = reactivationTime;
    else m_objDeactivationTimer = 0;

    if(!IsInWorld())
        return;

    if(MapCell *cell = m_mapCell)
        cell->DeactivateObject(this);

    // clear our managed cells
    GetCellManager()->ClearInRangeObjects(m_mapInstance);
}

void WorldObject::EventExploration(MapInstance *instance)
{

}

void WorldObject::BuildPhaseSet(std::vector<uint16> *phaseSet)
{
    uint8 max = RONIN_UTIL::getLastBit(m_phaseMask);
    for(uint8 i = 1; i <= max; ++i)
    {
        if((m_phaseMask & (1<<(i-1)))== 0)
            continue;
        phaseSet->push_back(i);
    }
}

void WorldObject::_Create( uint32 mapid, float x, float y, float z, float ang )
{
    m_mapId = mapid;
    m_position.ChangeCoords(x, y, z, NormAngle(ang));
    m_spawnLocation = m_position;
}

WorldPacket * WorldObject::BuildTeleportAckMsg(const LocationVector & v)
{
    ///////////////////////////////////////
    //Update player on the client with TELEPORT_ACK
    if(IsPlayer() && IsInWorld() )       // only send when inworld
        castPtr<Player>(this)->SetPlayerStatus( TRANSFER_PENDING );

    WorldPacket * data = new WorldPacket(MSG_MOVE_TELEPORT, 80);
    data->WriteBitString(10, m_objGuid[6], m_objGuid[0], m_objGuid[3], m_objGuid[2], 0, 0, m_objGuid[1], m_objGuid[4], m_objGuid[7], m_objGuid[5]);
    data->FlushBits();
    *data << uint32(0);
    data->WriteByteSeq(m_objGuid[1]);
    data->WriteByteSeq(m_objGuid[2]);
    data->WriteByteSeq(m_objGuid[3]);
    data->WriteByteSeq(m_objGuid[5]);
    *data << float(v.x);
    data->WriteByteSeq(m_objGuid[4]);
    *data << float(v.o);
    data->WriteByteSeq(m_objGuid[7]);
    *data << float(v.z);
    data->WriteByteSeq(m_objGuid[0]);
    data->WriteByteSeq(m_objGuid[6]);
    *data << float(v.y);
    return data;
}

void WorldObject::OnFieldUpdated(uint16 index)
{
    if(IsInWorld() && !m_mapInstance->UpdateQueued(this))
        m_mapInstance->ObjectUpdated(this);

    Object::OnFieldUpdated(index);
}

void WorldObject::SetPosition( float newX, float newY, float newZ, float newOrientation )
{
    float dist = m_position.DistanceSq(newX, newY, newZ);

    m_position.ChangeCoords(newX, newY, newZ, NormAngle(newOrientation));
    if(IsInWorld() && m_cellManager->IsLocationChanged(newX, newY, newZ))
        m_mapInstance->ObjectLocationChange(this);
}

void WorldObject::OutPacketToSet(uint16 Opcode, uint16 Len, const void * Data, bool self, float maxRange)
{
    if(self && GetTypeId() == TYPEID_PLAYER)
        castPtr<Player>(this)->PushData(Opcode, Len, Data);

    if(!IsInWorld())
        return;

    m_mapInstance->MessageToCells(this, Opcode, Len, Data, maxRange);
}

void WorldObject::SendMessageToSet(WorldPacket *data, bool bToSelf, bool myteam_only, float maxRange)
{
    if(!IsInWorld())
        return;

    uint32 myTeam = 0;
    if(IsPlayer())
    {
        if(bToSelf) castPtr<Player>(this)->PushPacket(data);
        myTeam = castPtr<Player>(this)->GetTeam();
    } else if(IsUnit()) myTeam = castPtr<Unit>(this)->GetTeam();

    m_mapInstance->MessageToCells(this, data, maxRange > 0.f ? maxRange*maxRange : 0.f, myteam_only, myTeam);
}

WorldObject *WorldObject::GetInRangeObject(WoWGuid guid)
{
    if(m_objGuid == guid)
        return this;
    if(IsInWorld() && guid.pLen())
        return m_mapInstance->GetInRangeObject(GetCellManager(), guid);
    return NULL;
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

float WorldObject::calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y )
{
    float dx = Position2X-Position1X, dy = Position2Y-Position1Y;
    double angle=0.0f;

    // Calculate angle
    if (dx == 0.0)
    {
        if (dy == 0.0)
            angle = 0.0;
        else if (dy > 0.0)
            angle = M_PI * 0.5 /* / 2 */;
        else angle = M_PI * 3.0 * 0.5/* / 2 */;
    }
    else if (dy == 0.0)
    {
        if (dx > 0.0)
            angle = 0.0;
        else angle = M_PI;
    }
    else if (dx < 0.0)
        angle = atanf(dy/dx) + M_PI;
    else if (dy < 0.0)
        angle = atanf(dy/dx) + (2*M_PI);
    else angle = atanf(dy/dx);

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
        else angle = M_PI * 3.0 * 0.5/*/ 2.0*/;
    }
    else if (dy == 0.0)
    {
        if (dx > 0.0)
            angle = 0.0;
        else angle = M_PI;
    }
    else if (dx < 0.0)
        angle = atan(dy/dx) + M_PI;
    else if (dy < 0.0)
        angle = atan(dy/dx) + (2*M_PI);
    else angle = atan(dy/dx);

    // Return
    return NormAngle(float(angle));
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

float WorldObject::CalcDistanceSq(float x1, float y1, float z1, float x2, float y2, float z2)
{
    float delta_x = fabs(x1 - x2);
    float delta_y = fabs(y1 - y2);
    float delta_z = fabs(z1 - z2);
    return (delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);
}

bool WorldObject::inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y )
{
    float angle = calcAngle( Position1X, Position1Y, Position2X, Position2Y );
    float lborder = getEasyAngle( ( Orientation - (FOV*0.5f/*/2*/) ) );
    float rborder = getEasyAngle( ( Orientation + (FOV*0.5f/*/2*/) ) );
    //sLog.outDebug("Orientation: %f Angle: %f LeftBorder: %f RightBorder %f",Orientation,angle,lborder,rborder);
    if(((angle >= lborder) && (angle <= rborder)) || ((lborder > rborder) && ((angle < rborder) || (angle > lborder))))
        return true;
    return false;
}

// Return angle in range 0..2*pi
float WorldObject::GetAngle(float x, float y, float tX, float tY)
{
    float dx = tX - x;
    float dy = tY - y;
    return NormAngle(atan2(dy, dx));
}

float WorldObject::GetAngle(WorldObject *obj)
{
    return GetAngle(GetPositionX(), GetPositionY(), obj->GetPositionX(), obj->GetPositionY());
}

bool WorldObject::isTargetInFront(WorldObject* target)
{
    // always have self in arc
    if (target == this)
        return true;

    // move arc to range 0.. 2*pi
    float arc = float(M_PI)/2.f;

    float angle = GetAngle(GetPositionX(), GetPositionY(), target->GetPositionX(), target->GetPositionY());
    angle -= m_position.o;

    // move angle to range -pi +pi
    angle = NormAngle(angle);
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
    return GetDistanceSq(target) <= range*range;
}

void WorldObject::SetFactionTemplate(uint32 templateId)
{
    m_factionTemplate = dbcFactionTemplate.LookupEntry(templateId);
    SetUInt32Value(IsGameObject() ? GAMEOBJECT_FACTION : UNIT_FIELD_FACTIONTEMPLATE, templateId);
}

int32 WorldObject::DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras)
{
    uint32 msTime = RONIN_UTIL::ThreadTimer::getThreadTime();
    if(!IsInWorld())
        return 0;
    if( !pVictim || !pVictim->isAlive() || !pVictim->IsInWorld())
        return 0;
    if( pVictim->IsSpiritHealer() )
        return 0;

    if( pVictim->GetStandState() )//not standing-> standup
        pVictim->SetStandState( STANDSTATE_STAND );//probably mobs also must standup

	Player* plr = IsPlayer() ? castPtr<Player>(this) : NULL;

    // Player we are attacking, or the owner of totem/pet/etc
    Player *pOwner = pVictim->IsPlayer() ? castPtr<Player>(pVictim) : NULL;

    // This is the player or the player controlling the totem/pet/summon
    Player *pAttacker = IsPlayer() ? castPtr<Player>(this) : NULL;

    // We identified both the attacker and the victim as possible PvP combatants, if we are not dueling we will flag the attacker
    if( pOwner && pAttacker && pOwner != pAttacker && pOwner->IsInDuel() )
        if(pOwner->GetDuelStorage() != pAttacker->GetDuelStorage())
            pAttacker->SetPvPFlag();

    // PvP NPCs will flag the player when attacking them
    if( pVictim->IsCreature() && pVictim->IsPvPFlagged() && pAttacker != NULL )
        pAttacker->SetPvPFlag();

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

    if(IsUnit() && castPtr<Unit>(this)->isAlive() )
    {
        if( castPtr<Unit>(this) != pVictim && pVictim->IsPlayer() && IsPlayer() )
            castPtr<Player>(this)->SetGuardHostileFlag();

        if(plr != NULL && pVictim->IsCreature())
            castPtr<Creature>(pVictim)->Tag(plr);

        if( pVictim != castPtr<Unit>(this))
            pVictim->SetInCombat(castPtr<Unit>(this));
    }

    ///Rage
    if( pVictim->GetMaxPower(POWER_TYPE_RAGE) > 0 && pVictim != castPtr<Unit>(this) && pVictim->IsPlayer())
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

    uint32 health = pVictim->GetUInt32Value(UNIT_FIELD_HEALTH );

    /*------------------------------------ DUEL HANDLERS --------------------------*/
    if(pVictim->IsPlayer() && castPtr<Player>(pVictim)->IsInDuel() && health <= damage) //Both Players
    {
        bool isDuelHit = false;
        Player *pThis = NULL, *plrVictim = castPtr<Player>(pVictim);
        if(IsPlayer() && (pThis = castPtr<Player>(this)))
        {
            pThis->smsg_AttackStop(pVictim);
            isDuelHit = plrVictim->IsDuelTarget(pThis);
        }

        // End duel here
        if(GameObject *arbiter = GetMapInstance()->GetGameObject(pVictim->GetUInt64Value(PLAYER_DUEL_ARBITER)))
            arbiter->DuelEnd(this, plrVictim, DUEL_WINNER_KNOCKOUT);

        // If this duel ended because a player beat the other, we don't die
        if(isDuelHit)
        {
            // surrender emote
            plrVictim->Emote(EMOTE_ONESHOT_BEG);           // Animation

            // Remove Negative Auras from duelist.
            plrVictim->m_AuraInterface.RemoveAllNegAurasFromGUID(GetGUID());

            // Change damage to leave 5 health
            damage = health-5;
        }
    }

    /*------------------------------------ DUEL HANDLERS END--------------------------*/

    bool isCritter = false, isTrainingDummy = false;
    if(pVictim->IsCreature())
    {
        Creature *cVictim = castPtr<Creature>(pVictim);
        if(cVictim->isTrainingDummy())
            isTrainingDummy = true;
        else if(cVictim->isCritter())
            isCritter = true;
    }

    /* -------------------------- HIT THAT CAUSES VICTIM TO DIE ---------------------------*/
    if (!isTrainingDummy && (isCritter || health <= damage))
    {
        if( pVictim->HasDummyAura(SPELL_HASH_GUARDIAN_SPIRIT) )
        {
            if(SpellEntry *sp = dbcSpell.LookupEntry(48153))
                pVictim->GetSpellInterface()->TriggerSpell(sp, pVictim);
            pVictim->RemoveDummyAura(SPELL_HASH_GUARDIAN_SPIRIT);
            return 0;
        }

        uint16 killProcModifiers = PROC_ON_KILL_MODIFIER_NONE;

        /* victim died! */
        Unit* pKiller = pVictim->m_killer.empty() ? NULL : GetInRangeObject<Unit>(pVictim->m_killer);
        if( pVictim->IsPlayer() )
        {
            // let's see if we have shadow of death
            if( !pVictim->m_AuraInterface.FindPositiveAuraByNameHash(SPELL_HASH_SHADOW_OF_DEATH) && castPtr<Player>( pVictim)->HasSpell( 49157 )) //check for shadow of death
            {
                SpellEntry* sorInfo = dbcSpell.LookupEntry(54223);
                if( sorInfo != NULL && castPtr<Player>(pVictim)->Cooldown_CanCast( sorInfo ))
                {
                    pVictim->GetSpellInterface()->LaunchSpell(sorInfo, pVictim);
                    return 0;
                }
            }

            // Update our proc modifiers
            killProcModifiers |= PROC_ON_KILL_PLAYER;

            castPtr<Player>( pVictim )->KillPlayer();

            /* Remove all Auras */
            pVictim->m_AuraInterface.EventDeathAuraRemoval();

            /* Set victim health to 0 */
            pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
            TRIGGER_INSTANCE_EVENT( m_mapInstance, OnPlayerDeath )( castPtr<Player>(pVictim), pKiller );
        }
        else
        {
            // Update our proc modifiers
            killProcModifiers |= PROC_ON_KILL_CREATURE;

            pVictim->SetDeathState( JUST_DIED );
            /* Remove all Auras */
            pVictim->m_AuraInterface.EventDeathAuraRemoval();
            /* Set victim health to 0 */
            pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);

            TRIGGER_INSTANCE_EVENT( m_mapInstance, OnCreatureDeath )( castPtr<Creature>(pVictim), pKiller );
        }

        pVictim->SummonExpireAll(false);

        if( pVictim->IsPlayer() && (!IsPlayer() || pVictim == this ) )
            castPtr<Player>( pVictim )->DeathDurabilityLoss(0.10);

        /* Zone Under Attack * /
        MapInfo * pZMapInfo = WorldMapInfoStorage.LookupEntry(GetMapId());
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
        }*/

        if(pVictim->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) > 0)
        {
            if(pVictim->isCasting())
            {
                SpellEntry* spl = pVictim->GetSpellInterface()->GetCurrentSpellProto();
                for(int i = 0; i < 3; i++)
                {
                    if(spl->Effect[i] != SPELL_EFFECT_PERSISTENT_AREA_AURA)
                        continue;
                    if(DynamicObject* dObj = GetMapInstance()->GetDynamicObject(pVictim->GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT)))
                    {
                        WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
                        data << dObj->GetGUID();
                        dObj->SendMessageToSet(&data, false);
                        dObj->Cleanup();
                    }
                }

                if(spl->ChannelInterruptFlags == 48140)
                    pVictim->GetSpellInterface()->CleanupCurrentSpell();
            }
        }

        /* Stop Units from attacking */
        if( pAttacker && pAttacker->IsInWorld() )
            pAttacker->EventAttackStop();

        if( IsUnit() )
        {
            castPtr<Unit>(this)->smsg_AttackStop( pVictim );

            /* Tell Unit that it's target has Died */
            castPtr<Unit>(this)->addStateFlag( UF_TARGET_DIED );
        }

        // Proc on death spells
        sSpellProcMgr.QuickProcessProcs(pVictim, PROC_ON_DEATH, 0x0000);

        /* -------------------------------- HONOR + BATTLEGROUND CHECKS ------------------------ */
        if( plr != NULL)
        {
            bool honorOrXPGain = false;

            TRIGGER_INSTANCE_EVENT( plr->GetMapInstance(), OnPlayerKillPlayer )( plr, pVictim );

            if( pVictim->IsPlayer() )
            {}
            else if(pVictim->IsCreature() && !pVictim->IsSummon() && !isCritter)
            {
                // add rep for on kill
                if(FactionEntry *faction = pVictim->GetFaction())
                {
                    if (Group *m_Group = plr->GetGroup())
                    {
                        /* loop the rep for group members */
                        m_Group->getLock().Acquire();
                        GroupMembersSet::iterator it;
                        for ( uint32 i = 0; i < m_Group->GetSubGroupCount(); i++ )
                        {
                            for ( it = m_Group->GetSubGroup(i)->GetGroupMembersBegin(); it != m_Group->GetSubGroup(i)->GetGroupMembersEnd(); ++it )
                                if ( (*it)->m_loggedInPlayer && (*it)->m_loggedInPlayer != plr && false)//(*it)->m_loggedInPlayer->IsInRangeSet(plr) )
                                    (*it)->m_loggedInPlayer->GetFactionInterface()->Reputation_OnKill(pVictim);
                        }
                        m_Group->getLock().Release();
                    }
                    plr->GetFactionInterface()->Reputation_OnKill(pVictim);
                }
            }

            if(plr->getLevel() <= (pVictim->getLevel() + 8) && plr->getClass() == WARRIOR)
            {   // currently only warriors seem to use it (Victory Rush)
                plr->SetFlag( UNIT_FIELD_AURASTATE, AURASTATE_FLAG_VICTORIOUS );
            }

            AchieveMgr.UpdateCriteriaValue(plr, ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, honorOrXPGain ? 1 : 0, pVictim->GetTypeId(), pVictim->IsCreature() ? castPtr<Creature>(pVictim)->GetCreatureType() : 0);
        }
        /* -------------------------------- HONOR + BATTLEGROUND CHECKS END------------------------ */

        uint64 victimGuid = pVictim->GetGUID();
        pVictim->SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_DEAD );

        pVictim->GenerateLoot();
        if(pVictim->IsCreature())
        {
            //--------------------------------- POSSESSED CREATURES -----------------------------------------
            if( pVictim->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE) )
            {   //remove possess aura from controller
                Player* vController = GetMapInstance()->GetPlayer( (uint32)pVictim->GetUInt64Value(UNIT_FIELD_CHARMEDBY) );
                if( vController )
                {
                    if( vController->GetUInt64Value( UNIT_FIELD_CHARM ) == victimGuid )//make sure he is target controller
                    {
                        vController->UnPossess();
                    }
                }
            }

            if(plr)
            {
                WorldPacket data(SMSG_PARTYKILLLOG, 16);
                data << GetGUID() << victimGuid;
                SendMessageToSet(&data, true);

                AchieveMgr.UpdateCriteriaValue(plr, ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, 1, pVictim->GetEntry());

                //---------------------------------looot-----------------------------------------
                if( pVictim->GetUInt64Value( UNIT_FIELD_CREATEDBY ) == 0 &&
                    pVictim->GetUInt64Value( UNIT_FIELD_SUMMONEDBY ) == 0 )
                {
                    //Not all NPC's give XP, check for it in proto no_XP
                    bool can_give_xp = true;
                    if(pVictim->IsCreature() && castPtr<Creature>(pVictim)->GetExtraInfo())
                        can_give_xp = (castPtr<Creature>(pVictim)->GetExtraInfo()->no_xp ? false : true);
                    if(can_give_xp)
                    {
                        // Is this player part of a group
                        if( plr->InGroup() && plr->GiveGroupXP( pVictim, plr ) )
                        {   // Give group XP returns true if the player we're sending can receive XP from the kill, not that they do
                            // Update our proc modifiers
                            killProcModifiers |= PROC_ON_KILL_GRANTS_XP;

                            // Give pet XP: TODO
                            /*if(Pet *playerPet = plr->GetActivePet())
                                playerPet->GiveXP(CalculateXpToGive( pVictim, playerPet, m_mapInstance->GetZoneModifier(m_zoneId) ));*/
                        }
                        else if( uint32 xp = CalculateXpToGive( pVictim, plr, m_mapInstance->GetZoneModifier(m_zoneId) ) )
                        {
                            if(plr->MobXPGainRate)
                                xp += (xp*(plr->MobXPGainRate/100));

                            plr->GiveXP( xp, victimGuid, true, false);

                            // Update our proc modifiers
                            killProcModifiers |= PROC_ON_KILL_GRANTS_XP;
                        }
                    }

                    if( pVictim->GetTypeId() != TYPEID_PLAYER )
                        sQuestMgr.OnPlayerKill( plr, castPtr<Creature>( pVictim ) );
                }
            }
            else /* is Creature or GameObject* */
            {

            }
        }
        else if( pVictim->IsPlayer() )
        {

        } else sLog.outDebug("DealDamage for unknown obj.");

        // Proc on kill spells
        if(IsUnit()) sSpellProcMgr.QuickProcessProcs(castPtr<Unit>(this), PROC_ON_KILL, killProcModifiers);

        return health;
	}   /* ---------- NOT DEAD YET --------- */
    else
    {
        if(IsUnit() && pVictim->IsCreature())
            castPtr<Creature>(pVictim)->GetAIInterface()->OnTakeDamage(msTime, castPtr<Unit>(this), damage);

        if(isTrainingDummy && damage >= health)
            pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 1 );
        else pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, health - damage );
    }
    return damage;
}

void WorldObject::SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, float resistPct, bool allowProc, bool no_remove_auras)
{
//==========================================================================================
//==============================Unacceptable Cases Processing===============================
//==========================================================================================
    if(!pVictim || !pVictim->isAlive())
        return;

    SpellEntry *spellInfo = dbcSpell.LookupEntry( spellID );
    if(!spellInfo)
        return;

    if (IsPlayer() && !castPtr<Player>(this)->canCast(spellInfo))
        return;
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
    if( IsUnit() )
    {
        caster->m_AuraInterface.RemoveAllAurasByInterruptFlag( AURA_INTERRUPT_ON_START_ATTACK );

//==========================================================================================
//==============================Post +SpellDamage Bonus Modifications=======================
//==========================================================================================
        if( res > 0 && !spellInfo->isUncrittableSpell() )
        {
//------------------------------critical strike chance--------------------------------------
            // lol ranged spells were using spell crit chance
            float CritChance;
            if(IsPlayer())
            {
                switch(spellInfo->spellType)
                {
                case MELEE: CritChance = GetFloatValue(PLAYER_CRIT_PERCENTAGE); break;
                case OFFHAND: CritChance = GetFloatValue(PLAYER_OFFHAND_CRIT_PERCENTAGE); break;
                case RANGED: case RANGED_AUTOSHOT: CritChance = GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE); break;
                case NON_WEAPON: CritChance = GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE); break;
                }
            } else CritChance = 5.f;

            if( spellInfo->IsSpellWeaponSpell() ) // Check our target's melee resilience for crit reduction
                CritChance -= pVictim->IsPlayer() ? castPtr<Player>(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE ) : 0.0f;
            else
            {   // Spells have modifiers that can change crit chance, as well as spell resilience for target
                if( spellInfo->SpellGroupType )
                {
                    caster->SM_FFValue(SMT_CRITICAL, &CritChance, spellInfo->SpellGroupType);
                    caster->SM_PFValue(SMT_CRITICAL, &CritChance, spellInfo->SpellGroupType);
                }
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

                pVictim->Emote( EMOTE_ONESHOT_WOUND_CRITICAL );
            } else pVictim->Emote( EMOTE_ONESHOT_WOUND );
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
    dmg.resisted_damage = 0;

    //------------------------------resistance reducing-----------------------------------------
    if(res > 0 && IsUnit())
    {
        dmg.resisted_damage += float2int32((((float)dmg.full_damage) * resistPct)/100.f);
        if((int32)dmg.resisted_damage >= dmg.full_damage)
            res = 0;
        else res = float(dmg.full_damage - dmg.resisted_damage);
    }
    //------------------------------special states----------------------------------------------
    /*if(pVictim->bInvincible == true)
    {
        res = 0;
        dmg.resisted_damage = dmg.full_damage;
    }*/

//==========================================================================================
//==============================Data Sending ProcHandling===================================
//==========================================================================================

    int32 ires = std::max<int32>(0, float2int32(res));

//--------------------------split damage-----------------------------------------------
    SendSpellNonMeleeDamageLog(this, pVictim, spellID, ires, school, abs_dmg, dmg.resisted_damage, false, 0, critical, IsPlayer());

    if( ires > 0 ) // only deal damage if its >0
        DealDamage( pVictim, ires, 2, 0, spellID );
    else if(IsUnit()) // we still have to tell the combat status handler we did damage so we're put in combat
        castPtr<Unit>(this)->SetInCombat(pVictim);

    //Only pushback the victim current spell if it's not fully absorbed
    if( (dmg.full_damage == 0 && abs_dmg) == 0 )
        pVictim->GetSpellInterface()->PushbackCast(school);
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

void WorldObject::SendSpellNonMeleeDamageLog( WorldObject* Caster, Unit* Target, uint32 SpellID, uint32 damageDone, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToset )
{
    if ( !Caster || !Target )
        return;
    SpellEntry *sp = dbcSpell.LookupEntry(SpellID);
    if( !sp )
        return;

    uint32 overkill = Target->computeOverkill(damageDone);
    uint32 Hit_flags = (0x00001|0x00004|0x00020);
    if(CriticalHit)
        Hit_flags |= 0x00002;

    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, 16+4+4+4+1+4+4+1+1+4+4+1);
    data << Target->GetGUID().asPacked();
    data << Caster->GetGUID().asPacked();
    data << uint32(SpellID);                // SpellID / AbilityID
    data << uint32(damageDone);             // All Damage
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

void WorldObject::EventSpellHit(Spell* pSpell)
{
    if( IsInWorld() && pSpell->GetCaster() != NULL )
        pSpell->cast(false);
    else pSpell->Destruct();
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

void WorldObject::UpdateAreaInfo(MapInstance *mgr)
{
    m_liquidFlags = 0;
    m_liquidHeight = NO_WATER_HEIGHT;
    m_wmoId = m_zoneId = m_areaId = 0;
    m_areaFlags = OBJECT_AREA_FLAG_NONE;
    if(mgr == NULL && (mgr = GetMapInstance()) == NULL)
        return;
    if(!IsPlayer())
        return;

    // Holes are used for WMO placement
    bool isHole = false;//mgr->GetADTIsHole(GetPositionX(), GetPositionY());

    // Grab our ADT ground height before WMO checks
    float ADTHeight = mgr->GetADTLandHeight(GetPositionX(), GetPositionY());

    uint16 adtLiqType; // Grab our ADT liquid height before WMO checks
    float ADTLiquid = mgr->GetADTWaterHeight(GetPositionX(), GetPositionY(), adtLiqType);

    // Check WMO data from our position
    sVMapInterface.GetWMOData(mgr, GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), m_wmoId, m_areaId, m_areaFlags, m_groundHeight, m_liquidFlags, m_liquidHeight);
    // Ground height, works pretty well
    if(m_groundHeight == NO_WMO_HEIGHT || (m_wmoId == 0 && m_groundHeight < ADTHeight))
        m_groundHeight = ADTHeight;

    // Liquid heights, needs more work | Don't use ADT height at holes or when under ADT height | TODO: Buildings underwater that cut off ADT liquid
    if(m_liquidHeight == NO_WMO_HEIGHT && (isHole || (m_groundHeight != ADTHeight && GetPositionZ() < ADTHeight) || (m_areaFlags & OBJECT_AREA_FLAG_IGNORE_ADT_WATER)))
        m_liquidFlags = 0, m_liquidHeight = NO_WATER_HEIGHT;
    else if(m_liquidHeight == NO_WMO_HEIGHT || (m_groundHeight == ADTHeight))
    {   // Use ADT liquid and Type
        m_liquidFlags = adtLiqType;
        m_liquidHeight = ADTLiquid;
    } else if(m_liquidFlags == 0 && adtLiqType)
        m_liquidFlags = adtLiqType; // Keep our WMO liquid and check for override type

    // Check if we have an invalid area id, use base map if we do
    if(m_areaId == 0 || m_areaId == 0xFFFF)
        m_areaId = mgr->GetADTAreaId(m_position.x, m_position.y);

    // Check if we have a forced zone for specific maps
    if(uint32 forcedZone = GetZoneForMap(mgr->GetMapId(), m_areaId))
    {
        m_zoneId = forcedZone;
        if(m_areaId == 0)
            m_areaId = m_zoneId;
    } else if(AreaTableEntry* at = dbcAreaTable.LookupEntry(m_areaId))
        m_zoneId = (at && at->ZoneId) ? at->ZoneId : m_areaId; // Otherwise check zone based on areaId

    // Generate new object area flags based on area or zone info
    if(m_zoneId || m_areaId)
    {
        // Santuary data is precached
        if(sWorld.CheckSanctuary(GetMapId(), m_zoneId, m_areaId))
            m_areaFlags |= OBJECT_AREA_FLAG_INSANCTUARY;

        // Grab our table entry to check categories and flags
        AreaTableEntry* at = dbcAreaTable.LookupEntry(m_areaId), *zoneAt = dbcAreaTable.LookupEntry(m_zoneId);
        if(at != NULL || (at = zoneAt) != NULL)
        {
            if(at->category == AREAC_ALLIANCE_TERRITORY || zoneAt->category == AREAC_ALLIANCE_TERRITORY)
                m_areaFlags |= OBJECT_AREA_FLAG_ALLIANCE_ZONE;
            else if(at->category == AREAC_HORDE_TERRITORY || zoneAt->category == AREAC_HORDE_TERRITORY)
                m_areaFlags |= OBJECT_AREA_FLAG_HORDE_ZONE;
            else if(at->category == AREAC_CONTESTED || zoneAt->category == AREAC_CONTESTED)
                m_areaFlags |= OBJECT_AREA_FLAG_CONTESTED;

            if(at->AreaFlags & AREA_CITY_AREA)
                m_areaFlags |= OBJECT_AREA_FLAG_INCITY;
            if(at->AreaFlags & AREA_PVP_ARENA)
                m_areaFlags |= OBJECT_AREA_FLAG_ARENA_ZONE;
            // Still unsure how Vash'Jir is detected so just go based on ID for now
            if(at->AreaId != 5146 && (at->ZoneId == 5144 || at->ZoneId == 5145 || at->ZoneId == 5146))
                m_areaFlags |= OBJECT_AREA_FLAG_UNDERWATER_AREA;
        }
    }

    // Handle pvp area flagging
    if(sWorld.IsPvPRealm && HasAreaFlag(OBJECT_AREA_FLAG_CONTESTED))
        m_areaFlags |= OBJECT_AREA_FLAG_PVP_AREA;
    else if(IsUnit() && castPtr<Unit>(this)->GetTeam() == TEAM_ALLIANCE && HasAreaFlag(OBJECT_AREA_FLAG_HORDE_ZONE))
        m_areaFlags |= OBJECT_AREA_FLAG_PVP_AREA;
    else if(IsUnit() && castPtr<Unit>(this)->GetTeam() == TEAM_HORDE && HasAreaFlag(OBJECT_AREA_FLAG_ALLIANCE_ZONE))
        m_areaFlags |= OBJECT_AREA_FLAG_PVP_AREA;
}

void WorldObject::PlaySoundToPlayer( Player* plr, uint32 sound_entry )
{
    if(plr == NULL || plr->GetSession() == NULL)
        return;

    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound_entry << GetGUID();
    plr->PushPacket( &data );
}

void WorldObject::PlaySoundToSet(uint32 sound_entry)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound_entry << GetGUID();
    SendMessageToSet(&data, true);
}

void WorldObject::SendAttackerStateUpdate( Unit* Target, dealdamage *dmg, uint32 realdamage, uint32 abs, uint32 blocked_damage, uint32 hit_status, uint32 vstate )
{
    WoWGuid targetGuid = Target ? Target->GetGUID() : 0;
    uint32 overkill = Target ? Target->computeOverkill(realdamage) : 0;
    uint32 schooldam = dmg ? SchoolMask(dmg->school_type) : 0x01;
    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, 108);
    data << uint32(hit_status);
    data << GetGUID().asPacked();
    data << targetGuid.asPacked();
    data << uint32(realdamage);                 // Realdamage;
    data << uint32(overkill);                   // Overkill
    data << uint8(1);                           // Damage type counter / swing type
    data << uint32(schooldam);                  // Damage school
    data << float(dmg ? dmg->full_damage : 0.f);// Damage float
    data << uint32(dmg ? dmg->full_damage : 0); // Damage amount

    if(hit_status & (HITSTATUS_ABSORBED | HITSTATUS_ABSORBED2))
        data << (uint32)abs;                    // Damage absorbed
    if(hit_status & (HITSTATUS_RESIST | HITSTATUS_RESIST2))
        data << uint32(dmg ? dmg->resisted_damage : 0);// Damage resisted

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
    if(!IsInWorld() || !GetMapInstance()->CanUseCollision(this) || !GetMapInstance()->CanUseCollision(pObj))
        return true;
    float Onoselevel = IsPlayer() ? Player::NoseHeight(castPtr<Player>(this)->getRace(), castPtr<Player>(this)->getGender()) : 2.f, Tnoselevel = pObj->IsPlayer() ? Player::NoseHeight(castPtr<Player>(pObj)->getRace(), castPtr<Player>(pObj)->getGender()) : 2.f;
    return (sVMapInterface.CheckLOS( GetMapId(), GetInstanceID(), GetPhaseMask(), GetPositionX(), GetPositionY(), GetPositionZ() + Onoselevel, pObj->GetPositionX(), pObj->GetPositionY(), pObj->GetPositionZ() + Tnoselevel) );
}

bool WorldObject::IsInLineOfSight(float x, float y, float z)
{
    if(!IsInWorld() || !GetMapInstance()->CanUseCollision(this))
        return true;
    return (sVMapInterface.CheckLOS( GetMapId(), GetInstanceID(), GetPhaseMask(), GetPositionX(), GetPositionY(), GetPositionZ() + (IsPlayer() ? Player::NoseHeight(castPtr<Player>(this)->getRace(), castPtr<Player>(this)->getGender()) : 2.f), x, y, z) );
}

// Returns the base cost of a spell
int32 WorldObject::GetSpellBaseCost(SpellEntry *sp)
{
    if(sp->powerType == POWER_TYPE_RUNE)
    {
        uint8 runeMask = 0x00;
        if(sp->runeCost[2] >= 2)
            runeMask |= (0x10|0x20);
        else if(sp->runeCost[2])
            runeMask |= 0x10;
        if(sp->runeCost[1] >= 2)
            runeMask |= (0x04|0x08);
        else if(sp->runeCost[1])
            runeMask |= 0x04;
        if(sp->runeCost[0] >= 2)
            runeMask |= (0x01|0x02);
        else if(sp->runeCost[0])
            runeMask |= 0x01;
        return runeMask;
    }
    else if( sp->ManaCostPercentage && IsUnit() )//Percentage spells cost % of !!!BASE!!! mana
    {
        if( sp->powerType == POWER_TYPE_MANA)
            return float2int32(((float)GetUInt32Value(UNIT_FIELD_BASE_MANA)) * (((float)sp->ManaCostPercentage) / 100.f));
        return float2int32(((float)GetUInt32Value(UNIT_FIELD_BASE_HEALTH)) * (((float)sp->ManaCostPercentage) / 100.f));
    } else return sp->ManaCost;
}
