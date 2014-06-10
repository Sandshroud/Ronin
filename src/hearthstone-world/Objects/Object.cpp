/***
 * Demonstrike Core
 */

#include "StdAfx.h"
using namespace std;

//#define DEG2RAD (M_PI/180.0)
#ifdef M_PI
#undef M_PI
#endif

#define M_PI        3.14159265358979323846f
#define M_H_PI      1.57079632679489661923f
#define M_Q_PI      0.785398163397448309615f

// Basic Height Checks
#define BASIC_OUTDOOR_WMO_2_DIFF 35.0f // Stuff
#define BASIC_HOUSE_FLOOR_DIFF 7.2f // Inn and houses

Object::Object() : m_position(0,0,0,0), m_spawnLocation(0,0,0,0)
{
#ifdef SHAREDPTR_DEBUGMODE
    printf("Object::Object()\n");
#endif
    m_phaseAura = NULLAURA;
    AllPhases = false;
    OwnPhase = false;
    m_phaseMask = 1;
    m_mapId = -1;
    m_areaId = 0;
    m_zoneId = 0;
    m_areaFlags = 0;

    m_uint32Values = 0;
    m_objectUpdated = false;
    m_isTransport = false;
    m_isVehicle = false;
    m_isSummon = false;
    m_isTotem = false;
    m_isPet = false;

    m_valuesCount = 0;

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
    m_faction = NULL;

    m_instanceId = 0;
    Active = false;
    m_inQueue = false;
    m_extensions = NULL;
    m_loadedFromDB = false;
    m_loot.gold = 0;
    m_looted = false;

    m_unitsInRange.clear();
    m_objectsInRange.clear();
    m_inRangePlayers.clear();
    m_oppFactsInRange.clear();
}

Object::~Object( )
{

}

void Object::Init()
{

}

void Object::Destruct()
{
    if(m_phaseAura)
    {
        m_phaseAura->Remove();
        m_phaseAura = NULLAURA;
    }

    if(m_objectTypeId != TYPEID_ITEM && m_objectTypeId != TYPEID_CONTAINER)
        ASSERT(!m_inQueue);

    if(IsInWorld() && m_objectTypeId != TYPEID_ITEM && m_objectTypeId != TYPEID_CONTAINER)
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
    m_objectTypeId = TYPEID_UNUSED;

    if( m_extensions != NULL )
        delete m_extensions;

    sEventMgr.RemoveEvents(this);
    delete this;
}

/* Crow:
    This function is a really heavy height check that is used for getting all types of height checks!
    Since it is in the object class, we can skip any actual variables and instead use our current position.
    This function gets map height and also checks the position of WMO's so that the height is set to the WMO height
    based on the Z position that is given. If no Z position is given, but x and y positions are given, we will instead
    use basic map heights as our Z position. */
float Object::GetCHeightForPosition(bool checkwater, float x, float y, float z)
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

    float vmapheight = sVMapInterface.GetHeight(GetMapId(), GetInstanceID(), GetPhaseMask(), x, y, z);
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

/*
    // Crow: WE SHOULD GET HIGHEST REASONABLE VALUE BASED ON Z AND THE CALCULATIONS BELOW
    // For now return the lowest reasonable one!
    if(ccollidemapheight != NO_WMO_HEIGHT)
    {
        if(checkwater == true)
        {
            float wz = mgr->GetWaterHeight(x, y);
            if(wz > ccollidemapheight && !(wz > (WMO_MAX_HEIGHT/2)))
                ccollidemapheight = wz;
        }

        if(ccollidemapheight < cvmapheight && ccollidemapheight+3.0f > cvmapheight || ccollidemapheight < cmapheight && ccollidemapheight+3.0f > cmapheight)
            return ccollidemapheight + ((z-1.0f > ccollidemapheight) ? 1.0f : 0.21563f);
    }

    if(cvmapheight != NO_WMO_HEIGHT)
    {
        if(checkwater == true)
        {
            float wz = mgr->GetWaterHeight(x, y);
            if(wz > cvmapheight && !(wz > (WMO_MAX_HEIGHT/2)))
                cvmapheight = wz;
        }

        if(cvmapheight < ccollidemapheight && cvmapheight+3.0f > ccollidemapheight || cvmapheight < cmapheight && cvmapheight+3.0f > cmapheight)
            return cvmapheight + ((z-1.0f > cvmapheight) ? 1.0f : 0.21563f);
    }

    if(checkwater == true)
    {
        float wz = mgr->GetWaterHeight(x, y);
        if(wz > cmapheight && !(wz > (WMO_MAX_HEIGHT/2)))
            cmapheight = wz;
    }

    return cmapheight + ((z-1.0f > cmapheight) ? 1.0f : 0.21563f);

    float offset = 10.0f;
    if(vmapheight < z) // Make sure we don't have some shit.
    {
        if(mgr->CanUseCollision(this))
        {
            if(sVMapInterface.IsIndoor(GetMapId(), x, y, z))
                offset = 5.0f;

            float pointx = 0.0f;
            float pointy = 0.0f;
            float pointz = 0.0f;
            if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, vmapheight+2.0f, x, y, z, pointx, pointy, pointz, 0.0f)) // Meaning there is a break inbetween us.
            {
                if(pointz+2.0f < vmapheight) // Distance is more than a roof.
                {
                    float pointz2 = 0.0f;
                    if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, vmapheight+2.0f, x, y, pointz, pointx, pointy, pointz2, 0.0f)) // Meaning there is a break inbetween us.
                    {
                        if(pointz2+2.0f < pointz) // Distance is more than a roof.
                        {
                            float pointz3 = 0.0f;
                            if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, pointz, x, y, pointz2, pointx, pointy, pointz3, 0.0f)) // Meaning there is a break inbetween us.
                                vmapheight = pointz3;
                        }
                        else
                            vmapheight = pointz2;
                    }
                    else
                        vmapheight = pointz;
                }
                else
                    vmapheight = pointz;
            }

            if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, z+offset, x, y, vmapheight, pointx, pointy, pointz, 0.0f)) // Meaning there is a break inbetween us.
            {
                if(pointz+2.0f < vmapheight) // Distance is more than a roof.
                {
                    float pointz2 = 0.0f;
                    if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, z+offset, x, y, pointz, pointx, pointy, pointz2, 0.0f)) // Meaning there is a break inbetween us.
                    {
                        if(pointz2+2.0f < pointz) // Distance is more than a roof.
                        {
                            float pointz3 = 0.0f;
                            if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, pointz, x, y, pointz2, pointx, pointy, pointz3, 0.0f)) // Meaning there is a break inbetween us.
                                vmapheight = pointz3;
                        }
                        else
                            vmapheight = pointz2;
                    }
                    else
                        vmapheight = pointz;
                }
                else
                    vmapheight = pointz;
            }
        }
    }
    else if(z+2.0f < vmapheight)
    {
        if(mgr->CanUseCollision(this))
        {
            if(sVMapInterface.IsIndoor(GetMapId(), x, y, z))
                offset = 5.0f; // Use a smaller offset

            float pointx = 0.0f;
            float pointy = 0.0f;
            float pointz = 0.0f;
            if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, z+2.0f, x, y, vmapheight, pointx, pointy, pointz, 0.0f)) // Meaning there is a break inbetween us.
            {
                if(pointz+2.0f < vmapheight) // Distance is more than a roof.
                {
                    float pointz2 = 0.0f;
                    if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, pointz+2.0f, x, y, vmapheight, pointx, pointy, pointz2, 0.0f)) // Meaning there is a break inbetween us.
                    {
                        if(pointz2+2.0f < pointz) // Distance is more than a roof.
                        {
                            float pointz3 = 0.0f;
                            if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, pointz2+2.0f, x, y, pointz, pointx, pointy, pointz3, 0.0f)) // Meaning there is a break inbetween us.
                                vmapheight = pointz3;
                        }
                        else
                            vmapheight = pointz2;
                    }
                    else
                        vmapheight = pointz;
                }
                else
                    vmapheight = pointz;
            }

            if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, vmapheight+offset, x, y, z, pointx, pointy, pointz, 0.0f)) // Meaning there is a break inbetween us.
            {
                if(pointz+2.0f < vmapheight) // Distance is more than a roof.
                {
                    float pointz2 = 0.0f;
                    if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, vmapheight+offset, x, y, pointz, pointx, pointy, pointz2, 0.0f)) // Meaning there is a break inbetween us.
                    {
                        if(pointz2+2.0f < pointz) // Distance is more than a roof.
                        {
                            float pointz3 = 0.0f;
                            if(sVMapInterface.GetFirstPoint(GetMapId(), x, y, pointz, x, y, pointz2, pointx, pointy, pointz3, 0.0f)) // Meaning there is a break inbetween us.
                                vmapheight = pointz3;
                        }
                        else
                            vmapheight = pointz2;
                    }
                    else
                        vmapheight = pointz;
                }
                else
                    vmapheight = pointz;
            }
        }
    }

    if(checkwater == true) // Crow: Pretty sure this is all we need.
    {
        float wz = mgr->GetWaterHeight(x, y);
        if(wz > vmapheight)
            vmapheight = wz;
    }

    return vmapheight+0.00321f; // We have a direct offset*/
}

void Object::SetPhaseMask(int32 phase)
{
    if(phase == -1)
    {
        AllPhases = true;
        m_phaseMask = 0xFFFF;
        SendPhaseShift();
        return;
    }

    m_phaseMask = phase;
    SendPhaseShift();
}

void Object::EnablePhase(int32 phaseMode)
{
    if(phaseMode == -1)
    {
        AllPhases = true;
        m_phaseMask = 0xFFFF;
        SendPhaseShift();
        return;
    }

    AllPhases = false;
    m_phaseMask |= phaseMode;
    SendPhaseShift();
}

void Object::DisablePhase(int32 phaseMode)
{
    AllPhases = false;
    if(phaseMode == -1)
        return;

    m_phaseMask &= ~phaseMode;
    SendPhaseShift();
}

void Object::SendPhaseShift()
{
    WorldPacket data(SMSG_SET_PHASE_SHIFT, 9);
    data << GetGUID();
    data << uint32(0) << uint32(0);
    data << uint32(2) << uint16(m_phaseMask);
    data << uint32(0) << uint32(0x08);
    SendMessageToSet(&data, (IsPlayer() ? true : false));
}

void Object::_Create( uint32 mapid, float x, float y, float z, float ang )
{
    m_mapId = mapid;
    m_position.ChangeCoords(x, y, z, ang);
    m_spawnLocation.ChangeCoords(x, y, z, ang);
    m_lastMapUpdatePosition.ChangeCoords(x,y,z,ang);
}

uint32 Object::BuildCreateUpdateBlockForPlayer(ByteBuffer *data, Player* target)
{
    uint16 updateFlags = UPDATEFLAG_NONE;
    uint8 updatetype = UPDATETYPE_CREATE_OBJECT;

    // any other case
    switch(m_objectTypeId)
    {
    case TYPEID_UNIT:
    case TYPEID_PLAYER:
        {
            updateFlags = UPDATEFLAG_LIVING|UPDATEFLAG_HAS_POSITION;
        }break;
    case TYPEID_GAMEOBJECT:
        {
            updateFlags = UPDATEFLAG_HAS_POSITION|UPDATEFLAG_POSITION|UPDATEFLAG_ROTATION;
            switch(GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID))
            {
            case GAMEOBJECT_TYPE_TRANSPORT:
            case GAMEOBJECT_TYPE_MO_TRANSPORT:
                {
                    updateFlags |= UPDATEFLAG_TRANSPORT;
                }break;
            case GAMEOBJECT_TYPE_TRAP:
            case GAMEOBJECT_TYPE_DUEL_ARBITER:
            case GAMEOBJECT_TYPE_FLAGSTAND:
            case GAMEOBJECT_TYPE_FLAGDROP:
                {
                    // duel flags have to stay as updatetype 3, otherwise
                    // it won't animate
                    updatetype = UPDATETYPE_CREATE_YOURSELF;
                }break;
            }
        }break;
    case TYPEID_CORPSE:
    case TYPEID_DYNAMICOBJECT:
        updateFlags = UPDATEFLAG_HAS_POSITION|UPDATEFLAG_POSITION;
        break;
    }

    if(GetTypeFromGUID() == HIGHGUID_TYPE_VEHICLE)
        updateFlags |= UPDATEFLAG_VEHICLE;
    if(target == this)
    {
        // player creating self
        updateFlags |= UPDATEFLAG_SELF;
        updatetype = UPDATETYPE_CREATE_YOURSELF;
    }

    if(IsUnit())
    {
        if (TO_UNIT(this)->GetUInt64Value(UNIT_FIELD_TARGET))
            updateFlags |= UPDATEFLAG_HAS_TARGET;
    }

    // build our actual update
    *data << updatetype;

    // we shouldn't be here, under any circumstances, unless we have a wowguid..
    ASSERT(m_wowGuid.GetNewGuidLen());
    *data << m_wowGuid;

    *data << m_objectTypeId;

    _BuildMovementUpdate(data, updateFlags, target);

    // we have dirty data, or are creating for ourself.
    UpdateMask updateMask;
    updateMask.SetCount( m_valuesCount );
    _SetCreateBits( &updateMask, target );

    // this will cache automatically if needed
    _BuildValuesUpdate( data, &updateMask, target );

    // update count: 1 ;)
    return 1;
}

//That is dirty fix it actually creates update of 1 field with
//the given value ignoring existing changes in fields and so on
//usefull if we want update this field for certain players
//NOTE: it does not change fields. This is also very fast method
WorldPacket *Object::BuildFieldUpdatePacket( uint32 index,uint32 value)
{
    // uint64 guidfields = GetGUID();
    // uint8 guidmask = 0;
    WorldPacket * packet = new WorldPacket(SMSG_UPDATE_OBJECT, 1500);
    *packet << uint16(GetMapId());
    *packet << (uint32)1;//number of update/create blocks
    BuildFieldUpdatePacket(packet, index, value);
    return packet;
}

void Object::BuildFieldUpdatePacket(Player* Target, uint32 Index, uint32 Value)
{
    ByteBuffer buf(500);
    BuildFieldUpdatePacket(&buf, Index, Value);
    Target->PushUpdateData(&buf, 1);
}

void Object::BuildFieldUpdatePacket(ByteBuffer * buf, uint32 Index, uint32 Value)
{
    *buf << uint8(UPDATETYPE_VALUES);
    *buf << GetNewGUID();

    uint32 mBlocks = Index/32+1;
    *buf << (uint8)mBlocks;

    for(uint32 dword_n = mBlocks-1; dword_n; dword_n--)
        *buf <<(uint32)0;

    *buf <<(((uint32)(1))<<(Index%32));
    *buf << Value;
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
            ASSERT(m_wowGuid.GetNewGuidLen());
            *data << m_wowGuid;

            _BuildValuesUpdate( data, &updateMask, target );
            return 1;
        }
    }

    return 0;
}

uint32 Object::BuildValuesUpdateBlockForPlayer(ByteBuffer * buf, UpdateMask * mask )
{
    // returns: update count
    *buf << (uint8) UPDATETYPE_VALUES;      // update type == update

    ASSERT(m_wowGuid.GetNewGuidLen());
    *buf << m_wowGuid;

    _BuildValuesUpdate( buf, mask, NULLPLR );

    // 1 update.
    return 1;
}

void Object::DestroyForInrange(bool anim)
{
    WorldPacket data(SMSG_DESTROY_OBJECT, 9);
    data << GetGUID();
    data << uint8(anim ? 1 : 0);
    SendMessageToSet(&data, false);
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

///////////////////////////////////////////////////////////////
/// Build the Movement Data portion of the update packet
/// Fills the data with this object's movement/speed info
void Object::_BuildMovementUpdate(ByteBuffer * data, uint16 flags, Player* target )
{
    *data << uint16(flags);

    if(flags & UPDATEFLAG_LIVING)
    {
        if(!IsUnit())
            return;

        size_t pos = data->wpos();
        TO_UNIT(this)->GetMovementInfo()->write(*data);
        data->put<uint8>(pos, 0);   // Clear our movement bits
        *data << m_walkSpeed;       // walk speed
        *data << m_runSpeed;        // run speed
        *data << m_backWalkSpeed;   // backwards run speed
        *data << m_swimSpeed;       // swim speed
        *data << m_backSwimSpeed;   // backwards swim speed
        *data << m_flySpeed;        // fly speed
        *data << m_backFlySpeed;    // back fly speed
        *data << m_turnRate;        // turn rate
        *data << m_pitchRate;       // pitch rate
    }
    else if(flags & UPDATEFLAG_POSITION)
    {
        *data << uint8(0);              // unk PGUID!
        *data << float(m_position.x);
        *data << float(m_position.y);
        *data << float(m_position.z);
        *data << float(m_position.x);
        *data << float(m_position.y);
        *data << float(m_position.z);
        *data << float(m_position.o);
        *data << float(0);
    }
    else if(flags & UPDATEFLAG_HAS_POSITION)
    {
        if(flags & UPDATEFLAG_TRANSPORT && (GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID) == GAMEOBJECT_TYPE_MO_TRANSPORT))
        {
            *data << float(0);
            *data << float(0);
            *data << float(0);
        }
        else
        {
            *data << float(m_position.x);
            *data << float(m_position.y);
            *data << float(m_position.z);
        }
        *data << float(m_position.o);
    }

    if(flags & UPDATEFLAG_HAS_TARGET)
        FastGUIDPack(*data, GetUInt64Value(UNIT_FIELD_TARGET)); // Compressed target guid.

    if(flags & UPDATEFLAG_TRANSPORT)
    {
        if(IsTransport())
            *data << TO_TRANSPORT(this)->m_timer;
        else *data << (uint32)getMSTime();
    }

    if(flags & UPDATEFLAG_VEHICLE)
        *data << TO_VEHICLE(this)->GetVehicleEntry() << float(TO_VEHICLE(this)->GetOrientation());

    if(flags & UPDATEFLAG_ANIMKITS)
        *data << uint16(0) << uint16(0) << uint16(0);

    // 0x200
    if(flags & UPDATEFLAG_ROTATION)
    {
        uint64 rotation = 0;
        if(IsGameObject())
            rotation = TO_GAMEOBJECT(this)->m_rotation;
        *data << uint64(rotation); //blizz 64bit rotation
    }

    if(flags & UPDATEFLAG_UNK5)
        *data << uint8(0);
}

//=======================================================================================
//  Creates an update block with the values of this object as
//  determined by the updateMask.
//=======================================================================================
void Object::_BuildValuesUpdate(ByteBuffer * data, UpdateMask *updateMask, Player* target)
{
    int32 DummyFlags = -1, DummyFlags2 = -1, DummyNpcFlags = -1, DummyDynFlags = -1;
    if(updateMask->GetBit(OBJECT_FIELD_GUID) && target)    // We're creating.
    {
        if(IsPlayer())
        {
            Player* pThis = TO_PLAYER(this);
            DummyFlags = m_uint32Values[UNIT_FIELD_FLAGS];
            DummyFlags2 = m_uint32Values[UNIT_FIELD_FLAGS_2];
            DummyNpcFlags = m_uint32Values[UNIT_NPC_FLAGS];
            DummyDynFlags = m_uint32Values[UNIT_DYNAMIC_FLAGS];
        }
        else if(IsCreature())       // tagged group will have tagged player
        {
            DummyFlags = m_uint32Values[UNIT_FIELD_FLAGS];
            DummyFlags2 = m_uint32Values[UNIT_FIELD_FLAGS_2];
            DummyNpcFlags = m_uint32Values[UNIT_NPC_FLAGS];
            DummyDynFlags = m_uint32Values[UNIT_DYNAMIC_FLAGS];
            Creature* cThis = TO_CREATURE(this);
            if(cThis->m_taggingPlayer)
            {
                // set tagged visual
                if( (cThis->m_taggingGroup != 0 && target->m_playerInfo->m_Group != NULL && target->m_playerInfo->m_Group->GetID() == cThis->m_taggingGroup) ||
                    (cThis->m_taggingPlayer == target->GetLowGUID()) )
                {
                    DummyDynFlags |= U_DYN_FLAG_TAPPED_BY_PLAYER;
                    if( cThis->m_loot.HasLoot(target) )
                        DummyDynFlags |= U_DYN_FLAG_LOOTABLE;
                }
                else
                    DummyDynFlags |= U_DYN_FLAG_TAGGED_BY_OTHER;
            }

            Trainer * pTrainer = cThis->GetTrainer();
            if(pTrainer != NULL)
            {
                if(!CanTrainAt(target, pTrainer))
                {
                    DummyNpcFlags &= ~(UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_TRAINER_PROF | UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_ARMORER);
                }
            }

            if(cThis->IsVehicle())
            {
                if(sFactionSystem.isAttackable(target, cThis, false))
                {
                    DummyNpcFlags &= ~(UNIT_NPC_FLAG_VEHICLE_MOUNT);
                }
            }

            updateMask->SetBit(UNIT_NPC_FLAGS);
            updateMask->SetBit(UNIT_FIELD_FLAGS);
            updateMask->SetBit(UNIT_FIELD_FLAGS_2);
            updateMask->SetBit(UNIT_DYNAMIC_FLAGS);
        }
        else if(IsGameObject())
        {
            DummyFlags = m_uint32Values[GAMEOBJECT_FLAGS];
            DummyDynFlags = m_uint32Values[GAMEOBJECT_DYNAMIC];
            GameObject* go = TO_GAMEOBJECT(this);
            GameObjectInfo *info = go->GetInfo();
            if(info)
            {
                set<uint32>* involvedquestids = objmgr.GetInvolvedQuestIds(info->ID);
                if(involvedquestids != NULL)
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
    else if(target)
    {
        if(IsPlayer())
        {
            Player* pThis = TO_PLAYER(this);
            // Player Vehicles.
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
            uint32 value = m_uint32Values[index];
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

WorldPacket * Object::BuildTeleportAckMsg(const LocationVector & v)
{
    ///////////////////////////////////////
    //Update player on the client with TELEPORT_ACK
    if( IsInWorld() )       // only send when inworld
        TO_PLAYER(this)->SetPlayerStatus( TRANSFER_PENDING );

    WorldPacket * data = new WorldPacket(MSG_MOVE_TELEPORT_ACK, 80);
    *data << GetNewGUID();
    *data << uint32(0); // m_teleportAckCounter;
    *data << uint32(2); // flags
    *data << uint16(0);
    *data << getMSTime();
    data->appendvector(v, true);
    *data << uint32(0);
    return data;
}

void Object::SetPosition( float newX, float newY, float newZ, float newOrientation )
{
    bool updateMap = false;
    if(m_lastMapUpdatePosition.Distance2DSq(newX, newY) > 4.0f)     /* 2.0f */
        updateMap = true;

    m_position.ChangeCoords(newX, newY, newZ, newOrientation);
    if (IsInWorld() && updateMap)
    {
        m_lastMapUpdatePosition.ChangeCoords(newX,newY,newZ,newOrientation);
        m_mapMgr->ChangeObjectLocation(this);

        if( m_objectTypeId == TYPEID_PLAYER && TO_PLAYER(this)->GetGroup() && TO_PLAYER(this)->m_last_group_position.Distance2DSq(m_position) > 25.0f ) // distance of 5.0
        {
            TO_PLAYER(this)->GetGroup()->HandlePartialChange( PARTY_UPDATE_FLAG_POSITION, TO_PLAYER(this) );
        }
    }
}

void Object::SetRotation( uint64 guid )
{
    WorldPacket data(SMSG_AI_REACTION, 12);
    data << guid;
    data << uint32(2);
    SendMessageToSet(&data, false);
}

void Object::OutPacketToSet(uint16 Opcode, uint16 Len, const void * Data, bool self)
{
    if(self && m_objectTypeId == TYPEID_PLAYER)
        TO_PLAYER(this)->GetSession()->OutPacket(Opcode, Len, Data);

    if(!IsInWorld())
        return;

    unordered_set<Player*  >::iterator itr = m_inRangePlayers.begin();
    unordered_set<Player*  >::iterator it_end = m_inRangePlayers.end();
    int gm = ( m_objectTypeId == TYPEID_PLAYER ? TO_PLAYER(this)->m_isGmInvisible : 0 );
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

void Object::SendMessageToSet(WorldPacket *data, bool bToSelf, bool myteam_only)
{
    if(!IsInWorld())
        return;

    if(bToSelf && m_objectTypeId == TYPEID_PLAYER)
        TO_PLAYER(this)->GetSession()->SendPacket(data);

    unordered_set<Player*>::iterator itr = m_inRangePlayers.begin();
    unordered_set<Player*>::iterator it_end = m_inRangePlayers.end();
    bool gminvis = (m_objectTypeId == TYPEID_PLAYER ? TO_PLAYER(this)->m_isGmInvisible : false);
    //Zehamster: Splitting into if/else allows us to avoid testing "gminvis==true" at each loop...
    //         saving cpu cycles. Chat messages will be sent to everybody even if player is invisible.
    if(myteam_only)
    {
        uint32 myteam = TO_PLAYER(this)->GetTeam();
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
        if(m_uint32Values[index] == 0)
            m_uint32Values[index] = val;
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
        if(m_uint32Values[i] != 0)
            updateMask->SetBit(i);
}

void Object::AddToWorld()
{
    MapMgr* mapMgr = sInstanceMgr.GetInstance(this);
    if(mapMgr == NULL)
    {
        if(IsPlayer() && TO_PLAYER(this)->m_bg != NULL && TO_PLAYER(this)->m_bg->IsArena())
            mapMgr = TO_PLAYER(this)->m_bg->GetMapMgr();
        else
            return; //instance add failed
    }

    if(!IsPlayer())
        UpdateAreaInfo(mapMgr);
    else
    {
        // battleground checks
        Player* p = TO_PLAYER(this);
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
    }

    m_mapMgr = mapMgr;
    m_inQueue = true;

    mapMgr->AddObject(this);

    // correct incorrect instance id's
    m_instanceId = m_mapMgr->GetInstanceID();
}

void Object::AddToWorld(MapMgr* pMapMgr)
{
    if(!pMapMgr)
        return; //instance add failed

    if(!IsPlayer())
        UpdateAreaInfo(pMapMgr);
    m_mapMgr = pMapMgr;
    m_inQueue = true;

    pMapMgr->AddObject(this);

    // correct incorrect instance id's
    m_instanceId = pMapMgr->GetInstanceID();
}

//Unlike addtoworld it pushes it directly ignoring add pool
//this can only be called from the thread of mapmgr!!!
void Object::PushToWorld(MapMgr* mgr)
{
    ASSERT(mgr != NULL);
    if(mgr == NULL)
    {
        // Reset these so session will get updated properly.
        m_inQueue = false;

        if(IsPlayer())
        {
            sLog.Error("Object","Kicking Player %s due to empty MapMgr;",TO_PLAYER(this)->GetName());
            TO_PLAYER(this)->GetSession()->LogoutPlayer(false);
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
    m_inQueue = false;

    event_Relocate();

    // call virtual function to handle stuff.. :P
    OnPushToWorld();
}

void Object::RemoveFromWorld(bool free_guid)
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

void Object::SetByte(uint32 index, uint32 index1,uint8 value)
{
    ASSERT( index < m_valuesCount );

    if(index1 > 4)
    {
        sLog.outError("Object::SetByteValue: wrong offset %u", index1);
        return;
    }

    if(uint8(m_uint32Values[ index ] >> (index1 * 8)) != value)
    {
        m_uint32Values[ index ] &= ~uint32(uint32(0xFF) << (index1 * 8));
        m_uint32Values[ index ] |= uint32(uint32(value) << (index1 * 8));

        if(IsInWorld())
        {
            m_updateMask.SetBit( index );

            if(!m_objectUpdated)
            {
                m_mapMgr->ObjectUpdated(this);
                m_objectUpdated = true;
            }
        }
    }
}

//! Set uint32 property
void Object::SetUInt32Value( const uint32 index, const uint32 value )
{
    if(index > m_valuesCount)
        printf("Index: %u, m_valuesCount: %u, Value: %u Test:%s\n", index, m_valuesCount, value, __FUNCTION__);

    ASSERT( index < m_valuesCount );
    // save updating when val isn't changing.
    if(m_uint32Values[index] == value)
        return;

    m_uint32Values[ index ] = value;

    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }

    // Group update handling
    if(m_objectTypeId == TYPEID_PLAYER)
    {
        if(IsInWorld())
        {
            Group* pGroup = TO_PLAYER(this)->GetGroup();
            if( pGroup != NULL )
                pGroup->HandleUpdateFieldChange( index, TO_PLAYER(this) );
        }
    }

    if(IsUnit())
    {
        if(index > UNIT_FIELD_POWER1 && index < UNIT_FIELD_POWER11)
            static_cast< Unit* >( this )->SendPowerUpdate();
    }
}

//! Set uint16 property
void Object::SetUInt16Value(uint16 index, uint8 offset, uint16 value)
{
    ASSERT( index < m_valuesCount );

    // save updating when val isn't changing.
    if (uint16(m_uint32Values[index] >> (offset * 16)) == value)
        return;

    m_uint32Values[index] &= ~uint32(uint32(0xFFFF) << (offset * 16));
    m_uint32Values[index] |= uint32(uint32(value) << (offset * 16));
    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }

    // Group update handling
    if(m_objectTypeId == TYPEID_PLAYER)
    {
        if(IsInWorld())
        {
            Group* pGroup = TO_PLAYER(this)->GetGroup();
            if( pGroup != NULL )
                pGroup->HandleUpdateFieldChange( index, TO_PLAYER(this) );
        }
    }

    if(IsUnit())
    {
        if(index > UNIT_FIELD_POWER1 && index < UNIT_FIELD_POWER11)
            static_cast< Unit* >( this )->SendPowerUpdate();
    }
}

uint32 Object::GetModPUInt32Value(const uint32 index, const int32 value)
{
    ASSERT( index < m_valuesCount );
    int32 basevalue = (int32)m_uint32Values[ index ];
    return ((basevalue*value)/100);
}

void Object::ModUnsigned32Value(uint32 index, int32 mod)
{
    ASSERT( index < m_valuesCount );
    if(mod == 0)
        return;

    m_uint32Values[ index ] += mod;
    if( (int32)m_uint32Values[index] < 0 )
        m_uint32Values[index] = 0;

    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }

    if(m_objectTypeId == TYPEID_PLAYER)
    {
        if(index > UNIT_FIELD_POWER1 && index < UNIT_FIELD_POWER11)
            static_cast< Unit* >( this )->SendPowerUpdate();
    }
}

void Object::ModSignedInt32Value(uint32 index, int32 value )
{
    ASSERT( index < m_valuesCount );
    if(value == 0)
        return;

    m_uint32Values[ index ] += value;
    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}

void Object::ModFloatValue(const uint32 index, const float value )
{
    ASSERT( index < m_valuesCount );
    m_floatValues[ index ] += value;

    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}
//! Set uint64 property
void Object::SetUInt64Value( const uint32 index, const uint64 value )
{
    assert( index + 1 < m_valuesCount );
    if(m_uint32Values[index] == GUID_LOPART(value) && m_uint32Values[index+1] == GUID_HIPART(value))
        return;

    m_uint32Values[ index ] = *((uint32*)&value);
    m_uint32Values[ index + 1 ] = *(((uint32*)&value) + 1);

    if(IsInWorld())
    {
        m_updateMask.SetBit( index );
        m_updateMask.SetBit( index + 1 );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}

void Object::SetUpdateField(uint32 index)
{
    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}

//! Set float property
void Object::SetFloatValue( const uint32 index, const float value )
{
    ASSERT( index < m_valuesCount );
    if(m_floatValues[index] == value)
        return;

    m_floatValues[ index ] = value;

    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}


void Object::SetFlag( const uint32 index, uint32 newFlag )
{
    ASSERT( index < m_valuesCount );

    //no change -> no update
    if((m_uint32Values[ index ] & newFlag) == newFlag)
        return;

    m_uint32Values[ index ] |= newFlag;

    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }

    // we're modifying an aurastate, so we need to update the auras.
    if( index == UNIT_FIELD_AURASTATE )
        TO_UNIT(this)->m_AuraInterface.UpdateAuraStateAuras(newFlag);
}


void Object::RemoveFlag( const uint32 index, uint32 oldFlag )
{
    ASSERT( index < m_valuesCount );

    //no change -> no update
    if((m_uint32Values[ index ] & oldFlag) == 0)
        return;

    m_uint32Values[ index ] &= ~oldFlag;

    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }

    // we're modifying an aurastate, so we need to update the auras.
    if( index == UNIT_FIELD_AURASTATE )
        TO_UNIT(this)->m_AuraInterface.UpdateAuraStateAuras(oldFlag);
}

bool Object::canWalk()
{
    if(IsCreature())
    {
        Creature* ctr = TO_CREATURE(this);
        if(ctr->GetCanMove() & LIMIT_ANYWHERE)
            return true;
        if(ctr->GetCanMove() & LIMIT_GROUND)
            return true;
    }
    else if(IsPlayer())
        return true;

    return false;
}

bool Object::canSwim()
{
    if(IsCreature())
    {
        Creature* ctr = TO_CREATURE(this);
        if(ctr->GetCanMove() & LIMIT_ANYWHERE)
            return true;
        if(ctr->GetCanMove() & LIMIT_WATER)
            return true;
    }
    else if(IsPlayer())
        return true;

    return false;
}

bool Object::canFly()
{
    if(IsVehicle())
    {
        return false;
    }
    else if(IsCreature())
    {
        Creature* ctr = TO_CREATURE(this);
        if(ctr->GetCanMove() & LIMIT_ANYWHERE)
            return true;
        if(ctr->GetCanMove() & LIMIT_AIR)
            return true;
    }
    else if(IsPlayer())
    {
        Player* plr = TO_PLAYER(this);
        if(plr->m_FlyingAura)
            return true;
        if(plr->FlyCheat)
            return true;
    }

    return false;
}

bool Object::IsInBox(float centerX, float centerY, float centerZ, float BLength, float BWidth, float BHeight, float BOrientation, float delta)
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

float Object::CalcDistance(Object* Ob)
{
    return CalcDistance(GetPositionX(), GetPositionY(), GetPositionZ(), Ob->GetPositionX(), Ob->GetPositionY(), Ob->GetPositionZ());
}

float Object::CalcDistance(float ObX, float ObY, float ObZ)
{
    return CalcDistance(GetPositionX(), GetPositionY(), GetPositionZ(), ObX, ObY, ObZ);
}

float Object::CalcDistance(Object* Oa, Object* Ob)
{
    return CalcDistance(Oa->GetPositionX(), Oa->GetPositionY(), Oa->GetPositionZ(), Ob->GetPositionX(), Ob->GetPositionY(), Ob->GetPositionZ());
}

float Object::CalcDistance(Object* Oa, float ObX, float ObY, float ObZ)
{
    return CalcDistance(Oa->GetPositionX(), Oa->GetPositionY(), Oa->GetPositionZ(), ObX, ObY, ObZ);
}

float Object::CalcDistance(float OaX, float OaY, float OaZ, float ObX, float ObY, float ObZ)
{
    float xdest = fabs(ObX - OaX);
    float ydest = fabs(ObY - OaY);
    float zdest = fabs(ObZ - OaZ);
    return sqrtf((zdest*zdest) + (ydest*ydest) + (xdest*xdest));
}

float Object::calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y )
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

float Object::calcRadAngle( float Position1X, float Position1Y, float Position2X, float Position2Y )
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

float Object::getEasyAngle( float angle )
{
    while ( angle < 0 ) {
        angle = angle + 360;
    }
    while ( angle >= 360 ) {
        angle = angle - 360;
    }
    return angle;
}

bool Object::inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y )
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

bool Object::isTargetInFront(Object* target)
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

bool Object::isInArc(Object* target , float angle) // angle in degrees
{
    return inArc( GetPositionX() , GetPositionY() , angle , GetOrientation() , target->GetPositionX() , target->GetPositionY() );
}

bool Object::isInRange(Object* target, float range)
{
    float dist = CalcDistance( target );
    return( dist <= range );
}

void Object::_setFaction()
{
    // Clear our old faction info
    m_factionTemplate = NULL;
    m_faction = NULL;

    FactionTemplateEntry* factionTemplate = NULL;

    if(GetTypeId() == TYPEID_UNIT || IsPlayer())
        factionTemplate = dbcFactionTemplate.LookupEntry(GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    else if(GetTypeId() == TYPEID_GAMEOBJECT)
        factionTemplate = dbcFactionTemplate.LookupEntry(GetUInt32Value(GAMEOBJECT_FACTION));
    if(factionTemplate)
    {
        m_factionTemplate = factionTemplate;
        m_faction = dbcFaction.LookupEntry(factionTemplate->Faction);
    }
    //Lets update our faction sets since we have changed faction.
    UpdateOppFactionSet();
}

void Object::UpdateOppFactionSet()
{
    if(!IsUnit())
        return;

    m_oppFactsInRange.clear();
    for(Object::InRangeUnitSet::iterator i = GetInRangeUnitSetBegin(); i != GetInRangeUnitSetEnd(); i++)
    {
        if (((*i)->GetTypeId() == TYPEID_UNIT) || ((*i)->IsPlayer()) || ((*i)->GetTypeId() == TYPEID_GAMEOBJECT))
        {
            if (sFactionSystem.isHostile(TO_UNIT(this), (*i)))
            {
                if(!(*i)->IsInRangeOppFactSet(TO_UNIT(this)))
                    (*i)->m_oppFactsInRange.insert(TO_UNIT(this));
                if (!IsInRangeOppFactSet((*i)))
                    m_oppFactsInRange.insert((*i));
            }
            else
            {
                if((*i)->IsInRangeOppFactSet(TO_UNIT(this)))
                    (*i)->m_oppFactsInRange.erase(TO_UNIT(this));
                if (IsInRangeOppFactSet((*i)))
                    m_oppFactsInRange.erase((*i));
            }
        }
    }
}

void Object::EventSetUInt32Value(uint32 index, uint32 value)
{
    SetUInt32Value(index,value);
}

int32 Object::DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras)
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

    Player* plr = NULLPLR;
    if(IsPet())
        plr = TO_PET(this)->GetPetOwner();
    else if(IsPlayer())
        plr = TO_PLAYER(this);

    // Player we are attacking, or the owner of totem/pet/etc
    Player *pOwner = pVictim->IsPlayer() ? TO_PLAYER(pVictim) : NULL;

    // This is the player or the player controlling the totem/pet/summon
    Player *pAttacker = IsPlayer() ? TO_PLAYER(this) : NULL;

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
        Player* owner = TO_PLAYER( TO_PET(this)->GetPetOwner() );
        if( owner != NULL )
            if( owner->isAlive() && TO_PLAYER( pVictim )->DuelingWith != owner )
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

    if(IsUnit() && TO_UNIT(this)->isAlive() )
    {
        if( TO_UNIT(this) != pVictim && pVictim->IsPlayer() && IsPlayer() && TO_PLAYER(this)->m_hasInRangeGuards )
        {
            TO_PLAYER(this)->SetGuardHostileFlag(true);
            TO_PLAYER(this)->CreateResetGuardHostileFlagEvent();
        }

        if(pVictim->IsPlayer() && !pVictim->CombatStatus.IsInCombat())
            sHookInterface.OnEnterCombat( TO_PLAYER( pVictim ), TO_UNIT(this) );

        if(IsPlayer() && ! TO_PLAYER(this)->CombatStatus.IsInCombat())
            sHookInterface.OnEnterCombat( TO_PLAYER(this), TO_PLAYER(this) );

        if(plr != NULL && pVictim->IsCreature())
            TO_CREATURE(pVictim)->Tag(plr);

        // Pepsi1x1: is this correct this
        if( pVictim != TO_UNIT(this))
            TO_UNIT(this)->CombatStatus.OnDamageDealt( pVictim, damage );
    }

    ///Rage
    if( pVictim->GetMaxPower(POWER_TYPE_RAGE) > 0
        && pVictim != TO_UNIT(this)
        && pVictim->IsPlayer())
    {
        float level = (float)pVictim->getLevel();
        float c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;
        uint32 rage = pVictim->GetUInt32Value( UNIT_FIELD_POWER2 );
        float val = 2.5f * damage / c;
        rage += float2int32(val) * 10;
        if( rage > pVictim->GetUInt32Value(UNIT_FIELD_MAXPOWER2) )
            rage = pVictim->GetUInt32Value(UNIT_FIELD_MAXPOWER2);

        pVictim->SetUInt32Value(UNIT_FIELD_POWER2, rage);
        pVictim->SendPowerUpdate();
    }

    //* BATTLEGROUND DAMAGE COUNTER *//
    if( pVictim != TO_UNIT(this) && plr != NULL )
    {
        if(plr->m_bg != NULL)
        {
            plr->m_bgScore.DamageDone += damage;
            plr->m_bg->UpdatePvPData();
        }
    }

    uint32 health = pVictim->GetUInt32Value(UNIT_FIELD_HEALTH );
    if(pVictim->getClass() == ROGUE && health <= damage && pVictim->IsPlayer() && pVictim->m_CustomTimers[CUSTOM_TIMER_CHEATDEATH] <= getMSTime() )
    {
        Player* plrVictim = TO_PLAYER(pVictim);
        uint32 rank = plrVictim->m_cheatDeathRank;

        uint32 chance = rank == 3 ? 100 : rank * 33;
        if(Rand(chance))
        {
            // Proc that cheating death!
            SpellEntry *spellInfo = dbcSpell.LookupEntry(45182);
            Spell* spell(new Spell(pVictim, spellInfo, true, NULLAURA));
            SpellCastTargets targets;
            targets.m_unitTarget = pVictim->GetGUID();
            spell->prepare(&targets);
            TO_PLAYER(pVictim)->m_CustomTimers[CUSTOM_TIMER_CHEATDEATH] = getMSTime() + 60000;

            // Why return? So this damage isn't counted. ;)
            // On official, it seems Blizzard applies it's Cheating Death school absorb aura for 1 msec, but it's too late
            // for us by now.
            return 0;
        }
    }

    /*------------------------------------ DUEL HANDLERS --------------------------*/
    if(pVictim->IsPlayer() && TO_PLAYER(pVictim)->DuelingWith != NULL) //Both Players
    {
        if(health <= damage)
        {
            if(IsPlayer() && (TO_PLAYER(pVictim)->DuelingWith == TO_PLAYER(this)))
            {
                // End Duel
                TO_PLAYER(this)->EndDuel(DUEL_WINNER_KNOCKOUT);
                TO_PLAYER(this)->GetAchievementInterface()->HandleAchievementCriteriaWinDuel();
                TO_PLAYER(pVictim)->GetAchievementInterface()->HandleAchievementCriteriaLoseDuel();

                // surrender emote
                TO_PLAYER(pVictim)->Emote(EMOTE_ONESHOT_BEG);           // Animation

                // Remove Negative Auras from duelist.
                TO_PLAYER(pVictim)->m_AuraInterface.RemoveAllNegAurasFromGUID(GetGUID());

                // Player in Duel and Player Victim has lost
                TO_PLAYER(pVictim)->CombatStatus.Vanish(GetLowGUID());
                TO_PLAYER(this)->CombatStatus.Vanish(pVictim->GetLowGUID());

                damage = health-5;
            }
            else if(TO_PLAYER(pVictim)->DuelingWith != NULL)
            {
                // We have to call the achievement interface from the duelingwith before, otherwise we crash.
                TO_PLAYER(pVictim)->DuelingWith->GetAchievementInterface()->HandleAchievementCriteriaWinDuel();
                TO_PLAYER(pVictim)->DuelingWith->EndDuel(DUEL_WINNER_KNOCKOUT);
//              TO_PLAYER(pVictim)->GetAchievementInterface()->HandleAchievementCriteriaLoseDuel(); Disable because someone cheated!
            }
        }
    }

    if((pVictim->IsPlayer()) && (IsPet()))
    {
        if((health <= damage) && TO_PLAYER(pVictim)->DuelingWith == TO_PET(this)->GetPetOwner())
        {
            Player* petOwner = TO_PET(this)->GetPetOwner();
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
    if(pVictim->GetTypeId() == TYPEID_UNIT && TO_CREATURE(pVictim)->GetCreatureInfo())
    {
        if(TO_CREATURE(pVictim)->GetCreatureInfo()->Type == CRITTER)
            isCritter = true;
        else if(TO_CREATURE(pVictim)->proto)
        {
            //Dummy trainers can't die
            if(isTargetDummy(TO_CREATURE(pVictim)->proto->Id) && health <= damage)
            {
                // Just limit to 5HP (can't use 1HP here).
                uint32 newh = 5;
                if(pVictim->GetMaxHealth() < 5)
                    newh = pVictim->GetMaxHealth();

                pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, newh);
                return health-5;
            }
        }
    }

    /* -------------------------- HIT THAT CAUSES VICTIM TO DIE ---------------------------*/
    if ((isCritter || health <= damage) )
    {
        if( IsUnit() )
        {
            if(!sHookInterface.OnPreUnitDie( TO_UNIT(this), pVictim) )
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
                TO_ALTERACVALLEY(m_mapMgr->m_battleground)->HookOnUnitKill( TO_PLAYER(this), pVictim );
            SpellEntry *killerspell;
            if( spellId )
                killerspell = dbcSpell.LookupEntry( spellId );
            else
                killerspell = NULL;

            pVictim->HandleProc( NULL, PROC_ON_DIE, TO_UNIT(this), killerspell );
            TO_UNIT(this)->HandleProc( PROC_ON_TARGET_DIE, NULL, pVictim, killerspell );
        }

        // check if pets owner is combat participant
        bool owner_participe = false;
        if( IsPet() )
        {
            Player* owner = TO_PET(this)->GetPetOwner();
            if( owner != NULL && pVictim->GetAIInterface()->getThreatByPtr( owner ) > 0 )
                owner_participe = true;
        }

        /* victim died! */
        Unit* pKiller = pVictim->CombatStatus.GetKiller();
        if( pVictim->IsPlayer() )
        {
            // let's see if we have shadow of death
            if( !pVictim->m_AuraInterface.FindPositiveAuraByNameHash(SPELL_HASH_SHADOW_OF_DEATH) && TO_PLAYER( pVictim)->HasSpell( 49157 )  &&
                !(TO_PLAYER(pVictim)->m_bg && TO_PLAYER(pVictim)->m_bg->IsArena())) //check for shadow of death
            {
                SpellEntry* sorInfo = dbcSpell.LookupEntry(54223);
                if( sorInfo != NULL && TO_PLAYER(pVictim)->Cooldown_CanCast( sorInfo ))
                {
                    Spell* sor(new Spell( pVictim, sorInfo, false, NULLAURA ));
                    SpellCastTargets targets;
                    targets.m_unitTarget = pVictim->GetGUID();
                    sor->prepare(&targets);
                    return 0;
                }
            }

            TO_PLAYER( pVictim )->KillPlayer();

            /* Remove all Auras */
            pVictim->m_AuraInterface.EventDeathAuraRemoval();

            /* Set victim health to 0 */
            pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
            CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnPlayerDeath )( TO_PLAYER(pVictim), pKiller );

            if( IsCreature() )
                TO_PLAYER(pVictim)->GetAchievementInterface()->HandleAchievementCriteriaKilledByCreature( GetEntry() );
            else if(IsPlayer())
                TO_PLAYER(pVictim)->GetAchievementInterface()->HandleAchievementCriteriaKilledByPlayer();
        }
        else
        {
            pVictim->setDeathState( JUST_DIED );
            /* Remove all Auras */
            pVictim->m_AuraInterface.EventDeathAuraRemoval();
            /* Set victim health to 0 */
            pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 0);

            CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnCreatureDeath )( TO_CREATURE(pVictim), pKiller );
        }

        pVictim->SummonExpireAll(false);

        if( pVictim->IsPlayer() && (!IsPlayer() || pVictim == TO_UNIT(this) ) )
            TO_PLAYER( pVictim )->DeathDurabilityLoss(0.10);

        /* Zone Under Attack */
        MapInfo * pZMapInfo = LimitedMapInfoStorage.LookupEntry(GetMapId());
        if( pZMapInfo != NULL && pZMapInfo->type == INSTANCE_NULL && !pVictim->IsPlayer() && !pVictim->IsPet() && ( IsPlayer() || IsPet() ) )
        {
            // Only NPCs that bear the PvP flag can be truly representing their faction.
            if( TO_CREATURE(pVictim)->IsPvPFlagged() )
            {
                Player* pAttacker = NULLPLR;
                if( IsPet() )
                    pAttacker = TO_PET(this)->GetPetOwner();
                else if(IsPlayer())
                    pAttacker = TO_PLAYER(this);

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
            Player* plrVictim = TO_PLAYER(pVictim);
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
                        if( ankh_count || TO_PLAYER(plrVictim)->HasDummyAura(SPELL_HASH_GLYPH_OF_RENEWED_LIFE ))
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
            TO_UNIT(this)->smsg_AttackStop( pVictim );

            /* Tell Unit that it's target has Died */
            TO_UNIT(this)->addStateFlag( UF_TARGET_DIED );
        }

        if( pVictim->IsPlayer() )
        {
            if( TO_PLAYER( pVictim)->HasDummyAura(SPELL_HASH_SPIRIT_OF_REDEMPTION) ) //check for spirit of Redemption
            {
                SpellEntry* sorInfo = dbcSpell.LookupEntry(27827);
                if( sorInfo != NULL )
                {
                    pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, 1);
                    Spell* sor(new Spell( pVictim, sorInfo, true, NULLAURA ));
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
                sHookInterface.OnKillPlayer( plr, TO_PLAYER( pVictim ) );
                HonorHandler::OnPlayerKilled( plr, TO_PLAYER( pVictim ) );
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
                if( !sEventMgr.HasEvent(TO_UNIT(plr), EVENT_VICTORIOUS_FLAG_EXPIRE ) )
                    sEventMgr.AddEvent( TO_UNIT(plr), &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_VICTORIOUS, EVENT_VICTORIOUS_FLAG_EXPIRE, 20000, 1, 0 );
                else
                    sEventMgr.ModifyEventTimeLeft( TO_UNIT(plr), EVENT_VICTORIOUS_FLAG_EXPIRE, 20000 , false );
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
                    TO_PLAYER(pVictim)->m_insigniaTaken = false;
                }
            }
            if( TO_PLAYER(pVictim)->m_bg != NULL && TO_PLAYER(pVictim)->m_bg->SupportsPlayerLoot() )
            {
                pVictim->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
                TO_PLAYER(pVictim)->m_insigniaTaken = false;
            }
        }
        else if(TO_CREATURE(pVictim)->m_taggingPlayer != 0 )    // only execute loot code if we were tagged
        {
            // fill loot vector
            TO_CREATURE(pVictim)->GenerateLoot();

            // update visual.
            TO_CREATURE(pVictim)->UpdateLootAnimation(pAttacker);
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
                pVictim->GetUInt64Value( OBJECT_FIELD_CREATED_BY ) == 0 )
            {
                // TODO: lots of casts are bad make a temp member pointer to use for batches like this
                // that way no local loadhitstore and its just one assignment

                //Not all NPC's give XP, check for it in proto no_XP
                bool can_give_xp = true;
                if(pVictim->IsCreature())
                    if(TO_CREATURE(pVictim)->GetExtraInfo())
                        can_give_xp = (TO_CREATURE(pVictim)->GetExtraInfo()->no_xp ? false : true);

                if(can_give_xp)
                {
                    // Is this player part of a group
                    if( TO_PLAYER(this)->InGroup() )
                    {
                        //Calc Group XP
                        TO_PLAYER(this)->GiveGroupXP( pVictim, TO_PLAYER(this) );
                        //TODO: pet xp if player in group
                    }
                    else
                    {
                        uint32 xp = CalculateXpToGive( pVictim, TO_UNIT(this) );
                        if( xp > 0 )
                        {
                            if(TO_PLAYER(this)->MobXPGainRate)
                                xp += (xp*(TO_PLAYER(this)->MobXPGainRate/100));

                            TO_PLAYER(this)->GiveXP( xp, victimGuid, true );
                            if( TO_PLAYER(this)->GetSummon() && TO_PLAYER(this)->GetSummon()->GetUInt32Value( UNIT_CREATED_BY_SPELL ) == 0 )
                            {
                                xp = CalculateXpToGive( pVictim, TO_PLAYER(this)->GetSummon() );
                                if( xp > 0 )
                                    TO_PLAYER(this)->GetSummon()->GiveXP( xp );
                            }
                        }
                    }
                }

                // Achievement: on kill unit
                if( !pVictim->IsPlayer() && IsPlayer() )
                {
                    Player* pThis = TO_PLAYER(this);
                    pThis->GetAchievementInterface()->HandleAchievementCriteriaKillCreature( pVictim->GetUInt32Value(OBJECT_FIELD_ENTRY) );
                }

                if( pVictim->GetTypeId() != TYPEID_PLAYER )
                    sQuestMgr.OnPlayerKill( TO_PLAYER(this), TO_CREATURE( pVictim ) );
            }
            else /* is Creature or GameObject* */
            {
                /* ----------------------------- PET XP HANDLING -------------- */
                if( owner_participe && IsPet() && !pVictim->IsPet() )
                {
                    Player* petOwner = TO_PET(this)->GetPetOwner();
                    if( petOwner != NULL && petOwner->IsPlayer() )
                    {
                        if( petOwner->InGroup() )
                        {
                            //Calc Group XP
                            TO_UNIT(this)->GiveGroupXP( pVictim, petOwner );
                            //TODO: pet xp if player in group
                        }
                        else
                        {
                            uint32 xp = CalculateXpToGive( pVictim, petOwner );
                            if( xp > 0 )
                            {
                                petOwner->GiveXP( xp, victimGuid, true );
                                if( !TO_PET(this)->IsSummonedPet() )
                                {
                                    xp = CalculateXpToGive( pVictim, TO_PET(this) );
                                    if( xp > 0 )
                                        TO_PET(this)->GiveXP( xp );
                                }
                            }
                        }
                    }
                    if( petOwner != NULL && pVictim->GetTypeId() != TYPEID_PLAYER &&
                        pVictim->GetTypeId() == TYPEID_UNIT )
                        sQuestMgr.OnPlayerKill( petOwner, TO_CREATURE( pVictim ) );
                }
                /* ----------------------------- PET XP HANDLING END-------------- */

                /* ----------------------------- PET DEATH HANDLING -------------- */
                if( pVictim->IsPet() )
                {
                    // dying pet looses 1 happiness level
                    if( !TO_PET( pVictim )->IsSummonedPet() )
                    {
                        uint32 hap = TO_PET( pVictim )->GetHappiness();
                        hap = hap - PET_HAPPINESS_UPDATE_VALUE > 0 ? hap - PET_HAPPINESS_UPDATE_VALUE : 0;
                        TO_PET( pVictim )->SetHappiness(hap);
                    }

                    TO_PET( pVictim )->DelayedRemove( false, true );

                    //remove owner warlock soul link from caster
                    Player* owner = TO_PET( pVictim )->GetPetOwner();
                    if( owner != NULL )
                        owner->EventDismissPet();
                }
                /* ----------------------------- PET DEATH HANDLING END -------------- */
                else if( pVictim->GetUInt64Value( UNIT_FIELD_CHARMEDBY ) )
                {
                    //remove owner warlock soul link from caster
                    Unit* owner=pVictim->GetMapMgr()->GetUnit( pVictim->GetUInt64Value( UNIT_FIELD_CHARMEDBY ) );
                    if( owner != NULL && owner->IsPlayer())
                        TO_PLAYER( owner )->EventDismissPet();
                }
            }
        }
        else if( pVictim->IsPlayer() )
        {
            /* -------------------- RESET BREATH STATE ON DEATH -------------- */
            TO_PLAYER( pVictim )->m_UnderwaterState = 0;

            /* -------------------- REMOVE PET WHEN PLAYER DIES ---------------*/
            if( TO_PLAYER( pVictim )->GetSummon() != NULL )
            {
                if( pVictim->GetUInt32Value( UNIT_CREATED_BY_SPELL ) > 0 )
                    TO_PLAYER( pVictim )->GetSummon()->Dismiss( true );
                else
                    TO_PLAYER( pVictim )->GetSummon()->Remove( true, true, true );
            }
            /* -------------------- REMOVE PET WHEN PLAYER DIES END---------------*/
        }
        else
            sLog.outDebug("DealDamage for Unknown Object.");

        if(IsUnit())
            sHookInterface.OnPostUnitDie(TO_UNIT(this), pVictim);
        return health;
    }
    else /* ---------- NOT DEAD YET --------- */
    {
        if(pVictim != TO_UNIT(this) /* && updateskill */)
        {
            // Send AI Reaction UNIT vs UNIT
            if( IsCreature() )
                TO_CREATURE(this)->GetAIInterface()->AttackReaction( pVictim, damage, spellId );

            // Send AI Victim Reaction
            if( IsUnit() )
                if( pVictim->IsCreature() )
                    TO_CREATURE( pVictim )->GetAIInterface()->AttackReaction( TO_UNIT(this), damage, spellId );

            if( IsUnit() && TO_UNIT( this )->HasDummyAura( SPELL_HASH_MARK_OF_BLOOD ) )
            {
                SpellEntry * tmpsp = dbcSpell.LookupEntry(50424);
                CastSpell(pVictim, tmpsp, true);
            }
            else if( IsPlayer() && spellId )
            {
                Player* plra = TO_PLAYER(this);
                SpellEntry *spentry = dbcSpell.LookupEntry( spellId );
                if( (spentry->NameHash == SPELL_HASH_CORRUPTION || spentry->NameHash == SPELL_HASH_UNSTABLE_AFFLICTION) && plra->HasDummyAura(SPELL_HASH_PANDEMIC) )
                {
                    if( Rand( plra->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1+5) ) && pVictim )
                    {
                        Spell* sp(new Spell(this, dbcSpell.LookupEntry(58691), true, NULLAURA));
                        SpellCastTargets targets;
                        targets.m_unitTarget = pVictim->GetGUID();
                        sp->forced_basepoints[0] = float2int32(damage * ( plra->GetDummyAura(SPELL_HASH_PANDEMIC)->RankNumber * 0.33f + 0.01f));
                        sp->prepare(&targets);
                    }
                }
            }
        }

        if(IsUnit() && pVictim->GetVehicle() != NULL)
        {
            if(pVictim->GetVehicle() == TO_UNIT(this))
                return 0;
            if(pVictim->GetVehicle() == TO_UNIT(this)->GetVehicle())
                return 0;
        }

        pVictim->SetUInt32Value(UNIT_FIELD_HEALTH, health - damage );
    }
    return damage;
}

int32 Object::SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, bool allowProc, bool static_damage, bool no_remove_auras, uint32 AdditionalCritChance)
{
//==========================================================================================
//==============================Unacceptable Cases Processing===============================
//==========================================================================================
    if(!pVictim || !pVictim->isAlive())
        return 0;

    SpellEntry *spellInfo = dbcSpell.LookupEntry( spellID );
    if(!spellInfo)
        return 0;

    if (IsPlayer() && !TO_PLAYER(this)->canCast(spellInfo))
        return 0;
//==========================================================================================
//==============================Variables Initialization====================================
//==========================================================================================
    uint32 school = spellInfo->School;
    float res = float(damage);
    uint32 aproc = PROC_ON_ANY_HOSTILE_ACTION;
    uint32 aproc2 = NULL;
    uint32 vproc = PROC_ON_ANY_HOSTILE_ACTION | PROC_ON_ANY_DAMAGE_VICTIM;
    uint32 vproc2 = NULL;
    bool critical = false;
    float dmg_reduction_pct;

    float res_after_spelldmg;

    Unit* caster = NULLUNIT;
    if( IsUnit() )
        caster = TO_UNIT(this);

    //A school damage is not necessarily magic
    switch( spellInfo->Spell_Dmg_Type )
    {
    case SPELL_DMG_TYPE_RANGED:
        {
            aproc |= PROC_ON_RANGED_ATTACK;
            vproc |= PROC_ON_RANGED_ATTACK_VICTIM;
        }break;
    case SPELL_DMG_TYPE_MELEE:
        {
            aproc |= PROC_ON_MELEE_ATTACK;
            vproc |= PROC_ON_MELEE_ATTACK_VICTIM;
        }break;
    case SPELL_DMG_TYPE_MAGIC:
        {
            aproc |= PROC_ON_SPELL_LAND;
            vproc |= PROC_ON_SPELL_HIT_VICTIM;
        }break;
    }

//==========================================================================================
//==============================+Spell Damage Bonus Calculations============================
//==========================================================================================
//------------------------------by stats----------------------------------------------------
    if( IsUnit() && !static_damage )
    {
        caster->m_AuraInterface.RemoveAllAurasByInterruptFlag( AURA_INTERRUPT_ON_START_ATTACK );

        res = caster->GetSpellBonusDamage( pVictim, spellInfo, ( int )res, false );

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

        res_after_spelldmg = res;
//==========================================================================================
//==============================Post +SpellDamage Bonus Modifications=======================
//==========================================================================================
        if( res < 0 )
            res = 0;
        else if( spellInfo->spell_can_crit == true )
        {
//------------------------------critical strike chance--------------------------------------
            // lol ranged spells were using spell crit chance
            float CritChance;
            if( spellInfo->is_ranged_spell || spellInfo->is_melee_spell )
            {

                if( IsPlayer() )
                {
                    CritChance = GetFloatValue( PLAYER_RANGED_CRIT_PERCENTAGE );
                    if( pVictim->IsPlayer() )
                        CritChance += TO_PLAYER(pVictim)->res_R_crit_get();

                    CritChance += (float)(pVictim->AttackerCritChanceMod[spellInfo->School]);
                }
                else
                    CritChance = 5.0f; // static value for mobs.. not blizzlike, but an unfinished formula is not fatal :)

                CritChance += AdditionalCritChance;
                CritChance -= pVictim->IsPlayer() ? TO_PLAYER(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE ) : 0.0f;
                if( spellInfo->is_melee_spell )
                    CritChance += (float)(pVictim->AttackerCritChanceMod[0]);
            }
            else
            {
                CritChance = caster->spellcritperc + caster->SpellCritChanceSchool[school] + pVictim->AttackerCritChanceMod[school];
                if( caster->IsPlayer() && ( pVictim->m_rooted - pVictim->m_stunned ) )
                    CritChance += TO_PLAYER( caster )->m_RootedCritChanceBonus;

                if( spellInfo->SpellGroupType )
                {
                    SM_FFValue(caster->SM[SMT_CRITICAL][0], &CritChance, spellInfo->SpellGroupType);
                    SM_PFValue(caster->SM[SMT_CRITICAL][1], &CritChance, spellInfo->SpellGroupType);
                }

                CritChance += AdditionalCritChance;
                if( pVictim->IsPlayer() )
                    CritChance -= TO_PLAYER(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE );
            }
            if( CritChance < 0 )
                CritChance = 0;
            if( CritChance > 95 )
                CritChance = 95;
            critical = Rand(CritChance);

            //sLog.outString( "SpellNonMeleeDamageLog: Crit Chance %f%%, WasCrit = %s" , CritChance , critical ? "Yes" : "No" );
            Aura* fs = NULLAURA;
            fs = spellInfo->NameHash == SPELL_HASH_LAVA_BURST ? pVictim->m_AuraInterface.FindNegativeAuraByNameHash(SPELL_HASH_FLAME_SHOCK): NULL;
            if( fs != NULL)
            {
                critical = true;
                if(caster && !caster->HasAura(55447))   // Glyph of Flame Shock
                    pVictim->RemoveAura(fs);
            }
//==========================================================================================
//==============================Spell Critical Hit==========================================
//==========================================================================================
            if (critical)
            {
                int32 critical_bonus = 100;
                if( spellInfo->SpellGroupType )
                    SM_FIValue( caster->SM[SMT_CRITICAL_DAMAGE][1], &critical_bonus, spellInfo->SpellGroupType );

                if( critical_bonus > 0 )
                {
                    // the bonuses are halved by 50% (funky blizzard math :S)
                    float b;
                    if( spellInfo->School == 0 || spellInfo->is_melee_spell || spellInfo->is_ranged_spell )     // physical || hackfix SoCommand/JoCommand
                        b = ( ( float(critical_bonus) ) / 100.0f ) + 1.0f;
                    else
                        b = ( ( float(critical_bonus) / 2.0f ) / 100.0f ) + 1.0f;

                    res *= b;

                    if( pVictim->IsPlayer() )
                    {
                        dmg_reduction_pct = 2.2f * TO_PLAYER(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_RESILIENCE ) / 100.0f;
                        if( dmg_reduction_pct > 0.33f )
                            dmg_reduction_pct = 0.33f; // 3.0.3

                        res = res - res * dmg_reduction_pct;
                    }
                }

                pVictim->Emote( EMOTE_ONESHOT_WOUNDCRITICAL );
                aproc2 |= PROC_ON_SPELL_CRIT_HIT;
                vproc |= PROC_ON_SPELL_CRIT_HIT_VICTIM;
            }
        }
    }
//==========================================================================================
//==============================Post Roll Calculations======================================
//==========================================================================================
    // Special cases
    if (caster)
    {
        if (caster->m_frozenTargetCharges > 0 && spellInfo->School == SCHOOL_FROST)
        {
            caster->m_frozenTargetCharges--;
            if (caster->m_frozenTargetCharges <= 0)
                caster->RemoveAura(caster->m_frozenTargetId);
        }

        // [Mage] Hot Streak
        if (!(aproc2 & PROC_ON_SPELL_CRIT_HIT))
            caster->m_hotStreakCount = 0;

        if( aproc2 & PROC_ON_SPELL_CRIT_HIT && caster->HasDummyAura(SPELL_HASH_ECLIPSE))
        {
            if( caster->m_CustomTimers[CUSTOM_TIMER_ECLIPSE] <= getMSTime() )
            {
                caster->m_CustomTimers[CUSTOM_TIMER_ECLIPSE] = getMSTime() + MSTIME_SECOND*30;
                if( spellInfo->NameHash == SPELL_HASH_STARFIRE )
                {
                    caster->CastSpell( caster, 48517, true );
                }else if( spellInfo->NameHash == SPELL_HASH_WRATH )
                {
                    caster->CastSpell( caster, 48518, true );
                }
            }
        }
    }

//------------------------------absorption--------------------------------------------------
    uint32 ress = (uint32)res;
    uint32 abs_dmg = pVictim->AbsorbDamage(this, school, &ress, dbcSpell.LookupEntry(spellID));
    uint32 ms_abs_dmg= pVictim->ManaShieldAbsorb(ress, dbcSpell.LookupEntry(spellID));
    if (ms_abs_dmg)
    {
        if(ms_abs_dmg > ress)
            ress = 0;
        else
            ress -= ms_abs_dmg;

        abs_dmg += ms_abs_dmg;
    }

    if(ress < 0)
        ress = 0;

    res = (float)ress;
    dealdamage dmg;
    dmg.school_type = school;
    dmg.full_damage = ress;
    dmg.resisted_damage = 0;

    if(res <= 0)
        dmg.resisted_damage = dmg.full_damage;

    //------------------------------resistance reducing-----------------------------------------
    if(res > 0 && IsUnit())
    {
        TO_UNIT(this)->CalculateResistanceReduction(pVictim,&dmg,spellInfo,0.0f);
        if((int32)dmg.resisted_damage >= dmg.full_damage)
            res = 0;
        else
            res = float(dmg.full_damage - dmg.resisted_damage);
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
            TO_UNIT(this)->CombatStatus.OnDamageDealt(pVictim, 1);
    }

    if( IsUnit() && allowProc && spellInfo->Id != 25501 )
    {
        pVictim->HandleProc( vproc, vproc2, TO_UNIT(this), spellInfo, float2int32( res ) );
        TO_UNIT(this)->HandleProc( aproc, aproc2, pVictim, spellInfo, float2int32( res ) );
    }

    if( IsPlayer() )
        TO_PLAYER(this)->m_casted_amount[school] = ( uint32 )res;

    if( (dmg.full_damage == 0 && abs_dmg) == 0 )
    {
        //Only pushback the victim current spell if it's not fully absorbed
        if( pVictim->GetCurrentSpell() )
            pVictim->GetCurrentSpell()->AddTime( school );
    }

//==========================================================================================
//==============================Post Damage Processing======================================
//==========================================================================================
    if( caster && (int32)dmg.resisted_damage == dmg.full_damage && !abs_dmg )
        caster->HandleProc(NULL, PROC_ON_FULL_RESIST, pVictim, spellInfo);

    if( school == SHADOW_DAMAGE )
    {
        if( IsPlayer() && TO_UNIT(this)->isAlive() && TO_PLAYER(this)->getClass() == PRIEST )
            TO_PLAYER(this)->VampiricSpell(float2int32(res), pVictim, dbcSpell.LookupEntry(spellID));

        if( pVictim->isAlive() && IsUnit() )
        {
            //Shadow Word: Death
            if( spellID == 32379 || spellID == 32996 || spellID == 48157 || spellID == 48158 )
            {
                uint32 damage = (uint32)( res + abs_dmg );

                if( TO_UNIT( this )->HasDummyAura(SPELL_HASH_PAIN_AND_SUFFERING) )
                    damage += float2int32(damage * ((TO_UNIT( this )->GetDummyAura(SPELL_HASH_PAIN_AND_SUFFERING)->EffectBasePoints[1]+1) / 100.0f));

                uint32 absorbed = TO_UNIT(this)->AbsorbDamage(this, school, &damage, dbcSpell.LookupEntry(spellID) );
                DealDamage( TO_UNIT(this), damage, 2, 0, spellID );
                SendSpellNonMeleeDamageLog( this, TO_UNIT(this), spellID, damage, school, absorbed, 0, false, 0, false, IsPlayer() );
            }
            else if( spellInfo->NameHash == SPELL_HASH_HAUNT )
                caster->m_lastHauntInitialDamage = res;
        }
    }
    return res;
}

//*****************************************************************************************
//* SpellLog packets just to keep the code cleaner and better to read
//*****************************************************************************************

void Object::SendSpellLog(Object* Caster, Object* Target, uint32 Ability, uint8 SpellLogType)
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

void Object::SendSpellNonMeleeDamageLog( Object* Caster, Unit* Target, uint32 SpellID, uint32 Damage, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToset )
{
    if ( !Caster || !Target )
        return;
    SpellEntry *sp = dbcSpell.LookupEntry(SpellID);
    if( !sp )
        return;
    SpellID = sp->logsId ? sp->logsId : sp->Id;
    uint32 overkill = Target->computeOverkill(Damage);

    uint32 Hit_flags = (0x00001|0x00004|0x00020);
    if(CriticalHit)
        Hit_flags |= 0x00002;

    uint32 dmg = Damage-AbsorbedDamage-ResistedDamage-BlockedDamage;
    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, 16+4+4+4+1+4+4+1+1+4+4+1);
    data << Target->GetNewGUID();
    data << Caster->GetNewGUID();
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

int32 Object::event_GetInstanceID()
{
    // return -1 for non-inworld.. so we get our shit moved to the right thread
    if(!IsInWorld())
        return -1;
    else
        return m_instanceId;
}

void Object::EventSpellHit(Spell* pSpell)
{
    if( IsInWorld() && pSpell->m_caster != NULL )
        pSpell->cast(false);
    else
        pSpell->Destruct();
}


bool Object::CanActivate()
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
            if(TO_GAMEOBJECT(this)->HasAI() && GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID) != GAMEOBJECT_TYPE_TRAP)
                return true;
        }break;
    }

    return false;
}

void Object::Activate(MapMgr* mgr)
{
    switch(m_objectTypeId)
    {
    case TYPEID_UNIT:
        if(IsVehicle())
            mgr->activeVehicles.insert(TO_VEHICLE(this));
        else
            mgr->activeCreatures.insert(TO_CREATURE(this));
        break;

    case TYPEID_GAMEOBJECT:
        mgr->activeGameObjects.insert(TO_GAMEOBJECT(this));
        break;
    }

    Active = true;
}

void Object::Deactivate(MapMgr* mgr)
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
                if( mgr->__creature_iterator != mgr->activeCreatures.end() && (*mgr->__creature_iterator) == TO_CREATURE(this) )
                    ++mgr->__creature_iterator;

                mgr->activeCreatures.erase(TO_CREATURE(this));
            }
        }break;

    case TYPEID_GAMEOBJECT:
        {
            // check iterator
            if( mgr->__gameobject_iterator != mgr->activeGameObjects.end() && (*mgr->__gameobject_iterator) == TO_GAMEOBJECT(this) )
                ++mgr->__gameobject_iterator;

            mgr->activeGameObjects.erase(TO_GAMEOBJECT(this));
        }break;
    }
    Active = false;
    mgr->ActiveLock.Release();
}

void Object::SetZoneId(uint32 newZone)
{
    m_zoneId = newZone;
    if( m_objectTypeId == TYPEID_PLAYER && TO_PLAYER(this)->GetGroup() )
        TO_PLAYER(this)->GetGroup()->HandlePartialChange( PARTY_UPDATE_FLAG_ZONEID, TO_PLAYER(this) );
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

void Object::UpdateAreaInfo(MapMgr *mgr)
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

void Object::PlaySoundToPlayer( Player* plr, uint32 sound_entry )
{
    if(plr == NULL || plr->GetSession() == NULL)
        return;

    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound_entry;
    plr->GetSession()->SendPacket( &data );
}

void Object::PlaySoundToSet(uint32 sound_entry)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound_entry;
    SendMessageToSet(&data, true);
}

void Object::_SetExtension(const string& name, void* ptr)
{
    if( m_extensions == NULL )
        m_extensions = new ExtensionSet;

    m_extensions->insert( make_pair( name, ptr ) );
}

void Object::SendAttackerStateUpdate( Unit* Target, dealdamage *dmg, uint32 realdamage, uint32 abs, uint32 blocked_damage, uint32 hit_status, uint32 vstate )
{
    if (!Target || !dmg)
        return;

    uint32 overkill = Target->computeOverkill(realdamage);
    uint32 schooldam = SchoolMask(dmg->school_type);
    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, 108);
    data << uint32(hit_status);
    data << GetNewGUID();
    data << Target->GetNewGUID();
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

void Object::SetByteFlag(const uint32 index, const uint32 flag, uint8 newFlag)
{
    if( HasByteFlag(index,flag,newFlag))
        return;

    SetByte(index, flag, GetByte(index,flag)|newFlag);

    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}

void Object::RemoveByteFlag(const uint32 index, const uint32 flag, uint8 checkFlag)
{
    if( !HasByteFlag(index,flag,checkFlag))
        return;

    SetByte(index,flag, GetByte(index,flag) & ~checkFlag );

    if(IsInWorld())
    {
        m_updateMask.SetBit( index );

        if(!m_objectUpdated)
        {
            m_mapMgr->ObjectUpdated(this);
            m_objectUpdated = true;
        }
    }
}

bool Object::HasByteFlag(const uint32 index, const uint32 flag, uint8 checkFlag)
{
    if( GetByte(index,flag) & checkFlag )
        return true;
    else
        return false;
}

bool Object::IsInLineOfSight(Object* pObj)
{
    float Onoselevel = 2.0f;
    float Tnoselevel = 2.0f;
    if(IsPlayer())
        Onoselevel = TO_PLAYER(this)->m_noseLevel;
    if(pObj->IsPlayer())
        Tnoselevel = TO_PLAYER(pObj)->m_noseLevel;

    if (GetMapMgr() && GetMapMgr()->CanUseCollision(this) && GetMapMgr()->CanUseCollision(pObj))
        return (sVMapInterface.CheckLOS( GetMapId(), GetInstanceID(), GetPhaseMask(), GetPositionX(), GetPositionY(), GetPositionZ() + Onoselevel + GetFloatValue(UNIT_FIELD_HOVERHEIGHT), pObj->GetPositionX(), pObj->GetPositionY(), pObj->GetPositionZ() + Tnoselevel + pObj->GetFloatValue(UNIT_FIELD_HOVERHEIGHT)) );
    else
        return true;
}

bool Object::IsInLineOfSight(float x, float y, float z)
{
    float Onoselevel = 2.0f;
    if(IsPlayer())
        Onoselevel = TO_PLAYER(this)->m_noseLevel;

    if (GetMapMgr() && GetMapMgr()->CanUseCollision(this))
        return (sVMapInterface.CheckLOS( GetMapId(), GetInstanceID(), GetPhaseMask(), GetPositionX(), GetPositionY(), GetPositionZ() + Onoselevel + GetFloatValue(UNIT_FIELD_HOVERHEIGHT), x, y, z) );
    else
        return true;
}

bool Object::PhasedCanInteract(Object* pObj)
{
    Player* pObjI = IsPlayer() ? TO_PLAYER(this) : NULLPLR;
    Player* pObjII = pObj->IsPlayer() ? TO_PLAYER(pObj) : NULLPLR;
    if( IsPet() )
        pObjI = TO_PET(this)->GetPetOwner();
    if( pObj->IsPet() )
        pObjII = TO_PET(pObj)->GetPetOwner();

    // Hack for Acherus: Horde/Alliance can't see each other!
    if( pObjI && pObjII && ( ( GetMapId() == 609 && pObjI->GetTeam() != pObjII->GetTeam() ) || ( OwnPhase || pObjII->OwnPhase) ) )
        return false;
    if( pObjI && pObjII && pObjI->GetAreaId() != pObjII->GetAreaId() )
        return true;

    if( AllPhases || pObj->AllPhases )
        return true;

    if( pObj->m_phaseMask == m_phaseMask || pObj->m_phaseMask & m_phaseMask || m_phaseMask & pObj->m_phaseMask )
        return true;

    return false;
}

// Returns the base cost of a spell
int32 Object::GetSpellBaseCost(SpellEntry *sp)
{
    float cost = 0.0f;
    if( sp->ManaCostPercentage && IsUnit() )//Percentage spells cost % of !!!BASE!!! mana
    {
        if( sp->powerType == POWER_TYPE_MANA)
            cost = GetUInt32Value(UNIT_FIELD_BASE_MANA) * (sp->ManaCostPercentage / 100.0f);
        else
            cost = GetUInt32Value(UNIT_FIELD_BASE_HEALTH) * (sp->ManaCostPercentage / 100.0f);
    }
    else
    {
        cost = (float)sp->ManaCost;
    }

    switch(sp->NameHash)
    {
    case SPELL_HASH_LIFE_TAP:
        {
            cost = (sp->EffectBasePoints[0]+1)+(1.5f*GetUInt32Value(UNIT_FIELD_STAT4));
        }break;
    }

    return float2int32(cost); // Truncate zeh decimals!
}

void Object::CastSpell( Object* Target, SpellEntry* Sp, bool triggered )
{
    if( Sp == NULL )
        return;

    Spell* newSpell(new Spell(this, Sp, triggered, NULLAURA));
    SpellCastTargets targets(0);
    if(Target)
    {
        if(Target->IsUnit())
        {
            targets.m_targetMask |= TARGET_FLAG_UNIT;
        }
        else
        {
            targets.m_targetMask |= TARGET_FLAG_OBJECT;
        }
        targets.m_unitTarget = Target->GetGUID();
    }
    else
    {
        newSpell->GenerateTargets(&targets);
    }
    newSpell->prepare(&targets);
}

void Object::CastSpell( Object* Target, uint32 SpellID, bool triggered )
{
    SpellEntry * ent = dbcSpell.LookupEntry(SpellID);
    if(ent == 0) return;

    CastSpell(Target, ent, triggered);
}

void Object::CastSpell( uint64 targetGuid, SpellEntry* Sp, bool triggered )
{
    if( Sp == NULL )
        return;

    SpellCastTargets targets(targetGuid);
    Spell* newSpell(new Spell(this, Sp, triggered, NULLAURA));
    newSpell->prepare(&targets);
}

void Object::CastSpell( uint64 targetGuid, uint32 SpellID, bool triggered )
{
    SpellEntry * ent = dbcSpell.LookupEntry(SpellID);
    if(ent == 0) return;

    CastSpell(targetGuid, ent, triggered);
}
