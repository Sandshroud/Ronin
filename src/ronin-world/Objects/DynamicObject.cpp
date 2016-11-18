/***
 * Demonstrike Core
 */

#include "StdAfx.h"

DynamicObject::DynamicObject(uint32 high, uint32 low, uint32 fieldCount) : WorldObject(MAKE_NEW_GUID(low, 0, high), fieldCount)
{
    SetTypeFlags(TYPEMASK_TYPE_DYNAMICOBJECT);
    m_objType = TYPEID_DYNAMICOBJECT;

    m_updateFlags |= UPDATEFLAG_STATIONARY_POS;
    m_dynamicobjectPool = 0xFF;

    m_aliveDuration = 0;
    m_spellProto = NULL;
}

DynamicObject::~DynamicObject()
{

}

void DynamicObject::Init()
{
    WorldObject::Init();
}

void DynamicObject::Destruct()
{
    m_aliveDuration = 0;
    m_spellProto = 0;
    WorldObject::Destruct();
}

void DynamicObject::Create(WorldObject* caster, BaseSpell* pSpell, float x, float y, float z, int32 duration, float radius)
{
    // Call the object create function
    WorldObject::_Create(caster->GetMapId(), x, y, z, 0.0f);
    casterLevel = caster->getLevel();
    casterGuid = caster->GetGUID();
    m_spellProto = pSpell->GetSpellProto();
    m_position.ChangeCoords(x, y, z);

    SetUInt32Value(OBJECT_FIELD_ENTRY, m_spellProto->Id);
    SetUInt64Value(DYNAMICOBJECT_CASTER, casterGuid);
    SetUInt32Value(DYNAMICOBJECT_BYTES, 0x01);
    SetUInt32Value(DYNAMICOBJECT_SPELLID, m_spellProto->Id);
    SetFloatValue(DYNAMICOBJECT_RADIUS, radius);
    SetUInt32Value(DYNAMICOBJECT_CASTTIME, getMSTime());

    m_aliveDuration = duration;
    m_factionTemplate = caster->GetFactionTemplate();

    PushToWorld(caster->GetMapInstance());

    if(caster->IsUnit() && m_spellProto->isChanneledSpell())
    {
        castPtr<Unit>(caster)->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, GetGUID());
        castPtr<Unit>(caster)->SetUInt32Value(UNIT_CHANNEL_SPELL, m_spellProto->Id);
    }
    UpdateTargets(0);
}

void DynamicObject::OnRemoveInRangeObject( WorldObject* pObj )
{
    if( pObj->IsUnit() )
        targets.erase( pObj->GetGUID() );

    WorldObject::OnRemoveInRangeObject( pObj );
}

void DynamicObject::UpdateTargets(uint32 p_time)
{
    Unit* u_caster = NULL;
    if(GUID_HIPART(casterGuid) == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* goCaster = GetMapInstance()->GetGameObject(casterGuid);
        if(goCaster == NULL || !goCaster->IsInWorld())
            m_aliveDuration = 0; // Set alive duration to 0
        else if(goCaster->m_summoner)
            u_caster = goCaster->m_summoner;
    }
    else
    {
        u_caster = GetMapInstance()->GetUnit(casterGuid);
        if(u_caster == NULL || !u_caster->IsInWorld())
            m_aliveDuration = 0; // Set alive duration to 0
    }

    // If we're a channelled spell, we are required to be the caster channel target
    if(m_spellProto->IsSpellChannelSpell() && u_caster)
    {
        if(GetGUID() != u_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))
            m_aliveDuration = 0;
    }

    if(m_aliveDuration > 0)
    {
        if(m_aliveDuration < p_time)
            m_aliveDuration = 0;
        else m_aliveDuration -= p_time;
    }

    if(m_aliveDuration && u_caster)
    {
        Aura* pAura;
        Unit* target;

        float radius = GetFloatValue(DYNAMICOBJECT_RADIUS);
        radius *= radius;

        // Looking for targets in the WorldObject set
        for(WorldObject::InRangeArray::iterator itr = GetInRangeUnitSetBegin(); itr != GetInRangeUnitSetEnd(); ++itr)
        {
            target = GetInRangeObject<Unit>(*itr);
            if(!target->isAlive())
                continue;

            if(!sFactionSystem.isAttackable(u_caster, target, !m_spellProto->isSpellStealthTargetCapable()))
                continue;

            // skip units already hit, their range will be tested later
            if(targets.find(target->GetGUID()) != targets.end())
                continue;

            if(GetDistanceSq(target) <= radius)
            {
                // Trigger raw aura application
                //target->ApplyAura(m_spellProto, u_caster, target);

                // add to target list
                targets.insert(target->GetGUID());
            }
        }

        // loop the targets, check the range of all of them
        DynamicObjectList::iterator jtr = targets.begin(), jtr2, jend = targets.end();
        while(jtr != jend)
        {
            jtr2 = jtr;
            ++jtr;

            target = GetMapInstance() ? GetMapInstance()->GetUnit(*jtr2) : NULL;
            if(target == NULL || GetDistanceSq(target) > radius)
            {
                if(target)
                    target->RemoveAura(m_spellProto->Id);
                targets.erase(jtr2);
            }
        }
    }
    else
    {
        // call remove here
        Remove();
    }
}

void DynamicObject::Remove()
{
    if(IsInWorld())
    {
        // remove aura from all targets
        for(std::set< uint64 >::iterator itr = targets.begin(); itr != targets.end(); ++itr)
            if(Unit *target = m_mapInstance->GetUnit(*itr))
                target->RemoveAura(m_spellProto->Id);

        WorldPacket data(SMSG_DESTROY_OBJECT, 8);
        data << GetGUID() << uint8(1);
        SendMessageToSet(&data, true);

        if(m_spellProto->IsSpellChannelSpell() && GUID_HIPART(casterGuid) != HIGHGUID_TYPE_GAMEOBJECT)
        {
            if(Unit* u_caster = GetMapInstance()->GetUnit(casterGuid))
            {
                if(GetGUID() == u_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))
                {
                    u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
                    u_caster->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
                }
            }
        }

        RemoveFromWorld();
    }
    Destruct();
}
