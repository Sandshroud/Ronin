/***
 * Demonstrike Core
 */

#include "StdAfx.h"

DynamicObject::DynamicObject(uint32 high, uint32 low)
{
    m_objectTypeId = TYPEID_DYNAMICOBJECT;
    m_valuesCount = DYNAMICOBJECT_END;
    m_uint32Values = _fields;
    memset(m_uint32Values, 0,(DYNAMICOBJECT_END)*sizeof(uint32));
    m_updateMask.SetCount(DYNAMICOBJECT_END);
    m_uint32Values[OBJECT_FIELD_TYPE] = TYPEMASK_DYNAMICOBJECT|TYPEMASK_OBJECT;
    SetUInt64Value( OBJECT_FIELD_GUID, MAKE_NEW_GUID(low, 0, high));
    m_wowGuid.Init(GetGUID());
    m_floatValues[OBJECT_FIELD_SCALE_X] = 1.f;
    m_aliveDuration = 0;
    m_spellProto = NULL;
}

DynamicObject::~DynamicObject()
{

}

void DynamicObject::Init()
{
    Object::Init();
}

void DynamicObject::Destruct()
{
    m_aliveDuration = 0;
    m_spellProto = 0;
    Object::Destruct();
}

void DynamicObject::Create(Object* caster, Spell* pSpell, float x, float y, float z, int32 duration, float radius)
{
    // Call the object create function
    Object::_Create(caster->GetMapId(), x, y, z, 0.0f);
    casterGuid = caster->GetGUID();
    if(!caster->IsPlayer() && pSpell->p_caster)
        casterGuid = pSpell->p_caster->GetGUID();

    m_spellProto = pSpell->m_spellInfo;
    SetUInt64Value(DYNAMICOBJECT_CASTER, caster->GetGUID());

    m_uint32Values[DYNAMICOBJECT_BYTES] = 0x01;
    m_uint32Values[OBJECT_FIELD_ENTRY] = m_spellProto->Id;
    m_uint32Values[DYNAMICOBJECT_SPELLID] = m_spellProto->Id;

    m_floatValues[DYNAMICOBJECT_RADIUS] = radius;
    m_position.x = x; //m_floatValues[DYNAMICOBJECT_POS_X]  = x;
    m_position.y = y; //m_floatValues[DYNAMICOBJECT_POS_Y]  = y;
    m_position.z = z; //m_floatValues[DYNAMICOBJECT_POS_Z]  = z;
    m_uint32Values[DYNAMICOBJECT_CASTTIME] = getMSTime();

    m_aliveDuration = duration;
    m_factionTemplate = caster->m_factionTemplate;
    m_faction = caster->m_faction;
    SetPhaseMask(caster->GetPhaseMask());

    if(pSpell->g_caster)
        PushToWorld(pSpell->g_caster->GetMapMgr());
    else
        PushToWorld(caster->GetMapMgr());

    if(caster->IsUnit() && m_spellProto->IsChannelSpell())
    {
        TO_UNIT(caster)->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, GetGUID());
        TO_UNIT(caster)->SetUInt32Value(UNIT_CHANNEL_SPELL, m_spellProto->Id);
    }
    UpdateTargets(0);
}

void DynamicObject::AddInRangeObject( Object* pObj )
{
    Object::AddInRangeObject(pObj);
}

void DynamicObject::OnRemoveInRangeObject( Object* pObj )
{
    if( pObj->IsUnit() )
        targets.erase( pObj->GetGUID() );

    Object::OnRemoveInRangeObject( pObj );
}

void DynamicObject::UpdateTargets(uint32 p_time)
{
    Unit* u_caster = NULL;
    if(GUID_HIPART(casterGuid) == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* goCaster = GetMapMgr()->GetGameObject(casterGuid);
        if(goCaster == NULL || !goCaster->IsInWorld())
            m_aliveDuration = 0; // Set alive duration to 0
        else if(goCaster->m_summoner)
            u_caster = goCaster->m_summoner;
    }
    else
    {
        u_caster = GetMapMgr()->GetUnit(casterGuid);
        if(u_caster == NULL || !u_caster->IsInWorld())
            m_aliveDuration = 0; // Set alive duration to 0
    }

    // If we're a channelled spell, we are required to be the caster channel target
    if(m_spellProto->IsChannelSpell() && u_caster)
    {
        if(u_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) != GetGUID())
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

        float radius = m_floatValues[DYNAMICOBJECT_RADIUS] * m_floatValues[DYNAMICOBJECT_RADIUS];

        // Looking for targets in the Object set
        for(std::unordered_set< Unit* >::iterator itr = m_unitsInRange.begin(); itr != m_unitsInRange.end(); ++itr)
        {
            target = *itr;
            if(!target->isAlive())
                continue;

            if(!sFactionSystem.isAttackable(u_caster, target, !(m_spellProto->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
                continue;

            // skip units already hit, their range will be tested later
            if(targets.find(target->GetGUID()) != targets.end())
                continue;

            if(GetDistanceSq(target) <= radius)
            {
                pAura = new Aura(m_spellProto, m_aliveDuration, u_caster, target);
                for(uint32 i = 0; i < 3; ++i)
                {
                    if(m_spellProto->Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                    {
                        pAura->AddMod(m_spellProto->EffectApplyAuraName[i], m_spellProto->EffectBasePoints[i]+1, m_spellProto->EffectMiscValue[i], i);
                    }
                }

                target->AddAura(pAura);
                u_caster->HandleProc(PROC_ON_CAST_SPECIFIC_SPELL | PROC_ON_CAST_SPELL, NULL, target, m_spellProto);

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

            target = GetMapMgr() ? GetMapMgr()->GetUnit(*jtr2) : NULL;
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
        Unit* target;
        for(std::set< uint64 >::iterator itr = targets.begin(); itr != targets.end(); ++itr)
        {

            uint64 TargetGUID = *itr;

            target = m_mapMgr->GetUnit(TargetGUID);

            if(target != NULL)
                target->RemoveAura(m_spellProto->Id);
        }

        WorldPacket data(SMSG_DESTROY_OBJECT, 8);
        data << GetGUID() << uint8(1);
        SendMessageToSet(&data, true);

        if(m_spellProto->IsChannelSpell() && GUID_HIPART(casterGuid) != HIGHGUID_TYPE_GAMEOBJECT)
        {
            if(Unit* u_caster = GetMapMgr()->GetUnit(casterGuid))
            {
                if(u_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) == GetGUID())
                {
                    u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
                    u_caster->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
                }
            }
        }

        RemoveFromWorld(true);
    }
    Destruct();
}
