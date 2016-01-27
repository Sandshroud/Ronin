/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Corpse::Corpse(uint32 high, uint32 low, uint32 fieldCount) : WorldObject(MAKE_NEW_GUID(low, 0, high), fieldCount)
{
    SetTypeFlags(TYPEMASK_TYPE_CORPSE);
    m_objType = TYPEID_CORPSE;

    m_updateFlags |= UPDATEFLAG_STATIONARY_POS;

    m_state = CORPSE_STATE_BODY;
    _loadedfromdb = false;
}

Corpse::~Corpse()
{

}

void Corpse::Init()
{
    WorldObject::Init();
    if(GetUInt32Value(OBJECT_FIELD_GUID+1) != 0)
        objmgr.AddCorpse(castPtr<Corpse>(this));
}

void Corpse::Destruct()
{
    if(objmgr.GetCorpse(GetLowGUID()))
        objmgr.RemoveCorpse(this);
    WorldObject::Destruct();
}

void Corpse::Create( uint32 owner, uint32 mapid, float x, float y, float z, float ang )
{
    WorldObject::_Create( mapid, x, y, z, ang);
    SetUInt32Value( CORPSE_FIELD_OWNER, owner);
}

void Corpse::Create( Player* owner, uint32 mapid, float x, float y, float z, float ang )
{
    WorldObject::_Create( mapid, x, y, z, ang);
    if(owner)
    {
        SetUInt32Value( CORPSE_FIELD_OWNER, owner->GetLowGUID());
        _loadedfromdb = false; // can't be created from db ;)
    }
}

void Corpse::OnPushToWorld()
{
    // add deletion event if bone corpse
    if( GetUInt64Value(CORPSE_FIELD_OWNER) == 0 )
        objmgr.CorpseAddEventDespawn(this);
}

void Corpse::SaveToDB()
{
    QueryBuffer* qb = new QueryBuffer();

    //save corpse to DB
    std::stringstream ss;
    ss << "DELETE FROM corpses WHERE guid = " << GetLowGUID();
    qb->AddQuery(ss.str().c_str());
//  CharacterDatabase.Execute( ss.str( ).c_str( ) );

    ss.rdbuf()->str("");
    ss << "INSERT INTO corpses VALUES ("
        << GetLowGUID() << ", "
        << GetUInt32Value(CORPSE_FIELD_OWNER) << ", "
        << GetUInt32Value(CORPSE_FIELD_DISPLAY_ID) << ", "
        << GetUInt32Value(CORPSE_FIELD_FLAGS) << ", "
        << GetPositionX() << ", "
        << GetPositionY() << ", "
        << GetPositionZ() << ", "
        << GetOrientation() << ", "
        << GetZoneId() << ", "
        << GetMapId() << ", "
        << uint32(IsInWorld() ? GetMapInstance()->GetInstanceID() : GetInstanceID()) << ", '";
    for(uint16 i = 0; i < m_valuesCount; i++ )
        ss << GetUInt32Value(i) << " ";
    ss << "')";

    qb->AddQuery(ss.str().c_str());
    CharacterDatabase.AddQueryBuffer(qb);
//  CharacterDatabase.Execute( ss.str().c_str() );
}

void Corpse::DeleteFromDB()
{
    //delete corpse from db when its not needed anymore
    char sql[256];

    snprintf(sql, 256, "DELETE FROM corpses WHERE guid=%u", (unsigned int)GetLowGUID());
    CharacterDatabase.Execute(sql);
}

void CorpseData::DeleteFromDB()
{
    char sql[256];

    snprintf(sql, 256, "DELETE FROM corpses WHERE guid=%u", (unsigned int)LowGuid);
    CharacterDatabase.Execute(sql);
}

void Corpse::Despawn()
{
    if(IsInWorld())
        RemoveFromWorld();
}

void Corpse::SpawnBones()
{
    SetUInt32Value(CORPSE_FIELD_FLAGS, 5);
    SetUInt64Value(CORPSE_FIELD_OWNER, 0); // remove corpse owner association
    //remove item association
    for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if(GetUInt32Value(CORPSE_FIELD_ITEM + i))
            SetUInt32Value(CORPSE_FIELD_ITEM + i, 0);
    }
    DeleteFromDB();
    objmgr.CorpseAddEventDespawn(castPtr<Corpse>(this));
    SetCorpseState(CORPSE_STATE_BONES);
}

void Corpse::Delink()
{
    SetUInt32Value(CORPSE_FIELD_FLAGS,5);
    SetUInt64Value(CORPSE_FIELD_OWNER,0);
    SetCorpseState(CORPSE_STATE_BONES);
    DeleteFromDB();
}
