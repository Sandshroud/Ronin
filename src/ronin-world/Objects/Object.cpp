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

Object::Object(WoWGuid guid, uint32 fieldCount) : m_eventHandler(this), m_valuesCount(fieldCount), m_updateFlags(0), m_notifyFlags(0), m_objGuid(guid), m_updateMask(m_valuesCount), m_inWorld(false)
{
    m_uint32Values = new uint32[m_valuesCount];
    memset(m_uint32Values, 0, sizeof(uint32)*m_valuesCount);

    SetUInt64Value(OBJECT_FIELD_GUID, guid);
    SetUInt32Value(OBJECT_FIELD_ENTRY, guid.getEntry());

    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.f);
    SetTypeFlags(TYPEMASK_TYPE_OBJECT);
    m_objType = TYPEID_OBJECT;

    m_loot.gold = 0;
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
                if (IsPlayer() && castPtr<Player>(this)->InGroup() && castPtr<Player>(this)->GetGroupID() == target->GetGroupID())
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
    if(IsPlayer() || IsCorpse() || IsDynamicObj() || IsTotem() || IsSummon())
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

uint32 Object::BuildValuesUpdateBlockForPlayer(ByteBuffer *data, uint32 updateFlags, uint32 expectedField)
{
    UpdateMask updateMask(m_valuesCount);
    if(expectedField || _SetUpdateBits(&updateMask, updateFlags))
    {
        if(expectedField != 0) updateMask.SetBit(expectedField);
        *data << uint8(UPDATETYPE_VALUES);     // update type == update
        *data << m_objGuid.asPacked();
        _BuildChangedValuesUpdate( data, &updateMask );
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
        if(typeMask & 1<<f)
        {
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
                    fields << uint32(m_uint32Values[offset]);
                }
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
void Object::_BuildChangedValuesUpdate(ByteBuffer * data, UpdateMask *updateMask)
{
    WPAssert( updateMask && updateMask->GetCount() == m_valuesCount );
    *data << uint8(updateMask->GetBlockCount());
    data->append( updateMask->GetMask(), updateMask->GetLength() );
    for( uint16 index = 0; index < updateMask->GetCount(); index++ )
        if( updateMask->GetBit( index ) )
            *data << m_uint32Values[index];
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
        *data << uint32(getMSTime());
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
}

//===============================================
// Object Cell Management functions
//===============================================
#define MAX_CELL_UPDATES 4

void ObjectCellManager::ClearInRangeObjects(MapInstance *instance)
{
    _currX = _currY = 0;
    _lowX = _lowY = _highX = _highY = 0;
    _processedCells.clear();
    _delayedCells[0].clear();
    _delayedCells[1].clear();

    _object->RemoveFromInRangeObjects(instance->GetMapId());
}

void ObjectCellManager::PostRemoveFromWorld()
{
    _currX = _currY = 0;
    _lowX = _lowY = _highX = _highY = 0;
    _processedCells.clear();
    _delayedCells[0].clear();
    _delayedCells[1].clear();
}

void ObjectCellManager::Update(MapInstance *instance, uint32 msTime, uint32 uiDiff)
{
    uint32 count = 0;
    while(!_delayedCells[0].empty())
    {
        std::pair<uint16, uint16> pair = unPack(*_delayedCells[0].begin());
        _delayedCells[0].erase(_delayedCells[0].begin());

        _processedCells.insert(_makeCell(pair.first, pair.second));
        instance->UpdateCellData(_object, pair.first, pair.second, true);
        if(++count >= MAX_CELL_UPDATES)
            return;
    }

    while(!_delayedCells[1].empty())
    {
        std::pair<uint16, uint16> pair = unPack(*_delayedCells[1].begin());
        _delayedCells[1].erase(_delayedCells[1].begin());

        _processedCells.insert(_makeCell(pair.first, pair.second));
        instance->UpdateCellData(_object, pair.first, pair.second, false);
        if(++count >= MAX_CELL_UPDATES)
            return;
    }
}

void ObjectCellManager::SetCurrentCell(MapInstance *instance, uint16 newX, uint16 newY, uint8 cellRange)
{
    _currX = newX;
    _currY = newY;

    // Remove any pending cell handling
    _delayedCells[0].clear();
    _delayedCells[1].clear();

    std::set<uint32> preProcessed;
    // Push old processed data to preProcessed group
    if(_lowX != _highX && _lowY != _highY)
    {
        for(uint16 x = _lowX; x <= _highX; x++)
            for(uint16 y = _lowY; y <= _highY; y++)
                preProcessed.insert(_makeCell(x, y));
    }
    // Clear old processed cells
    _processedCells.clear();
    // Push current cell to proccessed, we'll handle it in this function
    _processedCells.insert(_makeCell(_currX, _currY));

    if(cellRange)
    {   // Fill priority cells from a range of 1
        _lowX = _currX >= 1 ? _currX-1 : 0;
        _lowY = _currY >= 1 ? _currY-1 : 0;
        _highX = std::min<uint16>(_currX+1, _sizeX-1);
        _highY = std::min<uint16>(_currY+1, _sizeY-1);
        for(uint16 x = _lowX; x <= _highX; x++)
        {
            for(uint16 y = _lowY; y <= _highY; y++)
            {
                uint32 cellId = _makeCell(x, y);

                // Check to see if we're a preprocessed cell
                if(preProcessed.find(cellId) != preProcessed.end())
                {
                    _processedCells.insert(cellId);
                    preProcessed.erase(cellId);
                    continue;
                }

                // Skip processed cells(current usually)
                if(_processedCells.find(cellId) != _processedCells.end())
                    continue;

                // Add as a priority delayed cell
                _delayedCells[0].insert(cellId);
            }
        }

        // Process non priority cell bounding
        if(cellRange > 1)
        {   // Calculate our new bounds based on range
            _lowX = _currX >= cellRange ? _currX-cellRange : 0;
            _lowY = _currY >= cellRange ? _currY-cellRange : 0;
            _highX = std::min<uint16>(_currX+cellRange, _sizeX-1);
            _highY = std::min<uint16>(_currY+cellRange, _sizeY-1);

            // Only add extra cells if we're a player
            if(_object->IsPlayer())
            {
                for(uint16 x = _lowX; x <= _highX; x++)
                {
                    for(uint16 y = _lowY; y <= _highY; y++)
                    {
                        uint32 cellId = _makeCell(x, y);

                        // Check to see if we're a preprocessed cell
                        if(preProcessed.find(cellId) != preProcessed.end())
                        {
                            _processedCells.insert(cellId);
                            preProcessed.erase(cellId);
                            continue;
                        }

                        // Skip processed cells(current usually)
                        if(_processedCells.find(cellId) != _processedCells.end())
                            continue;

                        // Add as a low priority delayed cell
                        _delayedCells[1].insert(cellId);
                    }
                }
            }
        }
    }

    // If our instance is null, then it's part of preloading
    if(instance == NULL)
    {   // Set the data as processed
        _processedCells.insert(_delayedCells[0].begin(), _delayedCells[0].end());
        _processedCells.insert(_delayedCells[1].begin(), _delayedCells[1].end());
        // empty the tables
        _delayedCells[0].clear();
        _delayedCells[1].clear();
        return;
    }

    // Update for our current cell here, other cell updates will occur in WorldObject::Update
    instance->UpdateCellData(_object, _currX, _currY, true);
    // Push calls to remove cell data
    instance->RemoveCellData(_object, preProcessed, false);
}

//===============================================
// WorldObject class functions
//===============================================
WorldObject::WorldObject(WoWGuid guid, uint32 fieldCount) : Object(guid, fieldCount), m_cellManager(this), m_position(0,0,0,0)
{
    m_mapId = -1;
    m_wmoId = m_zoneId = m_areaId = 0;
    m_phaseMask = 0xFFFF;
    m_areaFlags = 0;
    m_lastMovementZone = 0;
    m_lastMovementArea = 0;

    m_mapInstance = NULL;
    m_mapCell = 0;

    m_factionTemplate = NULL;

    m_instanceId = 0;
    m_inactiveFlags = 0;
    m_objDeactivationTimer = 0;
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

    ClearInRangeObjects();

    m_factionTemplate = NULL;

    m_mapId = -1;
    m_wmoId = m_zoneId = m_areaId = 0;
    m_areaFlags = 0;
    m_lastMovementZone = 0;
    m_instanceId = -1;

    Object::Destruct();
}

void WorldObject::Update(uint32 msTime, uint32 uiDiff)
{
    Object::Update(msTime, uiDiff);

    // Update our internal cell processor
    m_cellManager.Update(GetMapInstance(), msTime, uiDiff);
    if(GetInRangePlayerCount() && GetMapInstance()->GetPlayerCount() == 0)
        ASSERT(false);
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
    Reactivate();
}

//Unlike addtoworld it pushes it directly ignoring add pool
//this can only be called from the thread of mapmgr!!!
void WorldObject::PushToWorld(MapInstance* instance)
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
    OnPushToWorld();

    // Set Object in world
    Object::SetInWorld(true);
}

void WorldObject::RemoveFromWorld()
{
    // clear loot
    ClearLoot();

    ASSERT(m_mapInstance);
    MapInstance* m = m_mapInstance;
    m_mapInstance = NULL;

    m->RemoveObject(this);

    // Set Object out of world
    Object::SetInWorld(false);

    m_cellManager.PostRemoveFromWorld();
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

    DestroyForInrange(IsGameObject());

    // clear our managed cells
    GetCellManager()->ClearInRangeObjects(m_mapInstance);
}

void WorldObject::EventExploration(MapInstance *instance)
{

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
    if(!IsInWorld() || RONIN_UTIL::fuzzyEq(dist, 0.0f))
        return;

    m_mapInstance->ObjectLocationChange(this);
}

void WorldObject::DestroyForInrange(bool anim)
{
    WorldPacket data(SMSG_DESTROY_OBJECT, 9);
    data << GetGUID();
    data << uint8(anim ? 1 : 0);
    SendMessageToSet(&data, false);
}

void WorldObject::RemoveFromInRangeObjects(uint16 mapId)
{
    // Remove object from all objects 'seeing' him
    WorldObject::InRangeHashMap inrangeObjects(*GetInRangeMap());
    for (WorldObject::InRangeHashMap::iterator iter = inrangeObjects.begin(); iter != inrangeObjects.end(); iter++)
    {
        if(WorldObject *wObj = iter->second)
        {
            if(wObj->IsPlayer())
            {
                if(Player *plr = castPtr<Player>(wObj))
                {
                    if( plr->IsVisible( this ) && plr->GetTransportGuid() != GetGUID())
                        plr->PushOutOfRange(mapId, GetGUID());
                    DestroyForPlayer(plr, IsGameObject());
                }
            }
            wObj->RemoveInRangeObject(this);
        }
    }

    // Clear object's in-range set
    ClearInRangeObjects();
}

void WorldObject::OutPacketToSet(uint16 Opcode, uint16 Len, const void * Data, bool self)
{
    if(self && GetTypeId() == TYPEID_PLAYER)
        castPtr<Player>(this)->GetSession()->OutPacket(Opcode, Len, Data);

    if(!IsInWorld())
        return;

    for(InRangeArray::iterator itr = GetInRangePlayerSetBegin(); itr != GetInRangePlayerSetEnd(); itr++)
    {
        if(Player *plr = GetInRangeObject<Player>(*itr))
        {
            if(plr->GetSession() == NULL)
                continue;
            plr->GetSession()->OutPacket(Opcode, Len, Data);
        }
    }
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

    if(!HasInRangePlayers())
        return;

    float range = maxRange*maxRange;
    for(InRangeArray::iterator itr = GetInRangePlayerSetBegin(); itr != GetInRangePlayerSetEnd(); itr++)
    {
        if(Player *plr = GetInRangeObject<Player>(*itr))
        {
            if(myteam_only && plr->GetTeam() != myTeam)
                continue;
            if(maxRange > 1.f && plr->GetDistanceSq(this) > range)
                continue;
            plr->PushPacket(data);
        }
    }
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

                // surrender emote
                castPtr<Player>(pVictim)->Emote(EMOTE_ONESHOT_BEG);           // Animation

                // Remove Negative Auras from duelist.
                castPtr<Player>(pVictim)->m_AuraInterface.RemoveAllNegAurasFromGUID(GetGUID());

                damage = health-5;
            } else if(castPtr<Player>(pVictim)->DuelingWith != NULL)
                castPtr<Player>(pVictim)->DuelingWith->EndDuel(DUEL_WINNER_KNOCKOUT);
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
            pVictim->CastSpell(pVictim, dbcSpell.LookupEntry(48153), true);
            pVictim->RemoveDummyAura(SPELL_HASH_GUARDIAN_SPIRIT);
            return 0;
        }

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
                    SpellCastTargets targets(pVictim->GetGUID());
                    if(Spell* sor = new Spell( pVictim, sorInfo))
                        sor->prepare(&targets, false);
                    return 0;
                }
            }

            castPtr<Player>( pVictim )->KillPlayer();

            /* Remove all Auras */
            pVictim->m_AuraInterface.EventDeathAuraRemoval();

            /* Set victim health to 0 */
            pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
            TRIGGER_INSTANCE_EVENT( m_mapInstance, OnPlayerDeath )( castPtr<Player>(pVictim), pKiller );
        }
        else
        {
            pVictim->SetDeathState( JUST_DIED );
            /* Remove all Auras */
            pVictim->m_AuraInterface.EventDeathAuraRemoval();
            /* Set victim health to 0 */
            pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);

            TRIGGER_INSTANCE_EVENT( m_mapInstance, OnCreatureDeath )( castPtr<Creature>(pVictim), pKiller );
        }

        pVictim->SummonExpireAll(false);

        if( pVictim->IsPlayer() && (!IsPlayer() || pVictim == castPtr<Unit>(this) ) )
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
            if(pVictim->GetCurrentSpell())
            {
                Spell* spl = pVictim->GetCurrentSpell();
                for(int i = 0; i < 3; i++)
                {
                    if(spl->GetSpellProto()->Effect[i] != SPELL_EFFECT_PERSISTENT_AREA_AURA)
                        continue;
                    if(DynamicObject* dObj = GetMapInstance()->GetDynamicObject(pVictim->GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT)))
                    {
                        WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
                        data << dObj->GetGUID();
                        dObj->SendMessageToSet(&data, false);
                        dObj->RemoveFromWorld();
                        dObj->Destruct();
                    }
                }

                if(spl->GetSpellProto()->ChannelInterruptFlags == 48140)
                    spl->cancel();
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

        if( pVictim->IsPlayer() )
        {
            if( castPtr<Player>( pVictim)->HasDummyAura(SPELL_HASH_SPIRIT_OF_REDEMPTION) ) //check for spirit of Redemption
            {
                if (SpellEntry* sorInfo = dbcSpell.LookupEntry(27827))
                {
                    pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 1);
                    SpellCastTargets targets(pVictim->GetGUID());
                    if(Spell* sor = new Spell( pVictim, sorInfo ))
                        sor->prepare(&targets, true);
                }
            }
        }

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
                                if ( (*it)->m_loggedInPlayer && (*it)->m_loggedInPlayer != plr && (*it)->m_loggedInPlayer->IsInRangeSet(plr) )
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

        if(pVictim->IsCreature() && castPtr<Creature>(pVictim)->m_taggingPlayer != 0 )    // only execute loot code if we were tagged
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
                        if( plr->InGroup() )
                        {
                            //Calc Group XP
                            plr->GiveGroupXP( pVictim, plr );
                            //TODO: pet xp if player in group
                        }
                        else if( uint32 xp = CalculateXpToGive( pVictim, plr ) )
                        {
                            if(plr->MobXPGainRate)
                                xp += (xp*(plr->MobXPGainRate/100));

                            plr->GiveXP( xp, victimGuid, true, false);
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

        return health;
	}   /* ---------- NOT DEAD YET --------- */
    else
    {
        if(IsUnit() && pVictim->IsCreature())
            castPtr<Creature>(pVictim)->GetAIInterface()->OnTakeDamage(castPtr<Unit>(this), damage);

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

    if( (dmg.full_damage == 0 && abs_dmg) == 0 )
    {
        //Only pushback the victim current spell if it's not fully absorbed
        if( pVictim->GetCurrentSpell() )
            pVictim->GetCurrentSpell()->AddTime( school );
    }
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
    if(m_liquidHeight == NO_WMO_HEIGHT && (isHole || (m_groundHeight != ADTHeight && GetPositionZ() < ADTHeight)))
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
            if(at->category == AREAC_CONTESTED)
                m_areaFlags |= OBJECT_AREA_FLAG_CONTESTED;
            if(at->category == AREAC_ALLIANCE_TERRITORY)
                m_areaFlags |= OBJECT_AREA_FLAG_ALLIANCE_ZONE;
            if(at->category == AREAC_HORDE_TERRITORY)
                m_areaFlags |= OBJECT_AREA_FLAG_HORDE_ZONE;
            if(at->AreaFlags & AREA_CITY_AREA)
                m_areaFlags |= OBJECT_AREA_FLAG_INCITY;
            if(at->AreaFlags & AREA_PVP_ARENA)
                m_areaFlags |= OBJECT_AREA_FLAG_ARENA_ZONE;
            // Still unsure how Vash'Jir is detected so just go based on ID for now
            if(at->AreaId != 5146 && (at->ZoneId == 5144 || at->ZoneId == 5145 || at->ZoneId == 5146))
                m_areaFlags |= OBJECT_AREA_FLAG_UNDERWATER_AREA;
        }
    }
}

void WorldObject::PlaySoundToPlayer( Player* plr, uint32 sound_entry )
{
    if(plr == NULL || plr->GetSession() == NULL)
        return;

    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound_entry << GetGUID();
    plr->GetSession()->SendPacket( &data );
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

void WorldObject::CastSpell( WorldObject* Target, SpellEntry* Sp, bool triggered )
{
    if( Sp == NULL )
        return;

    Spell* newSpell = new Spell(this, Sp);
    SpellCastTargets targets;
    if(Target)
    {
        if(Target->IsUnit())
            targets.m_targetMask |= TARGET_FLAG_UNIT;
        else targets.m_targetMask |= TARGET_FLAG_OBJECT;
        targets.m_unitTarget = Target->GetGUID();
    } else newSpell->GenerateTargets(&targets);
    newSpell->prepare(&targets, triggered);
}

void WorldObject::CastSpell( WorldObject* Target, uint32 SpellID, bool triggered )
{
    if(SpellEntry * ent = dbcSpell.LookupEntry(SpellID))
        CastSpell(Target, ent, triggered);
}

void WorldObject::CastSpell( uint64 targetGuid, SpellEntry* Sp, bool triggered )
{
    if( Sp == NULL )
        return;

    SpellCastTargets targets(targetGuid);
    if(Spell* newSpell = new Spell(this, Sp))
        newSpell->prepare(&targets, triggered);
}

void WorldObject::CastSpell( uint64 targetGuid, uint32 SpellID, bool triggered )
{
    if(SpellEntry * ent = dbcSpell.LookupEntry(SpellID))
        CastSpell(targetGuid, ent, triggered);
}
