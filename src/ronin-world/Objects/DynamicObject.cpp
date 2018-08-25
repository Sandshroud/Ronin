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

DynamicObject::DynamicObject(uint32 high, uint32 low, uint32 fieldCount) : WorldObject()
{
    // Call object construct first
    WorldObject::Construct(MAKE_NEW_GUID(low, 0, high), fieldCount);
    // Init

    SetTypeFlags(TYPEMASK_TYPE_DYNAMICOBJECT);
    m_objType = TYPEID_DYNAMICOBJECT;

    m_updateFlags |= UPDATEFLAG_STATIONARY_POS;

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

void DynamicObject::Update(uint32 msTime, uint32 uiDiff)
{
    UpdateTargets(uiDiff);
}

void DynamicObject::OnPushToWorld(uint32 msTime)
{
    // Update targets with no time diff
    UpdateTargets(0);
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
    SetUInt32Value(DYNAMICOBJECT_BYTES, m_spellProto->SpellVisual[0] | 0x0010000000);
    SetUInt32Value(DYNAMICOBJECT_SPELLID, m_spellProto->Id);
    SetFloatValue(DYNAMICOBJECT_RADIUS, radius);
    SetUInt32Value(DYNAMICOBJECT_CASTTIME, getMSTime());

    m_aliveDuration = duration;
    m_factionTemplate = caster->GetFactionTemplate();

    caster->GetMapInstance()->AddObject(this);
    if(caster->IsUnit() && m_spellProto->isChanneledSpell())
    {
        castPtr<Unit>(caster)->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, GetGUID());
        castPtr<Unit>(caster)->SetUInt32Value(UNIT_CHANNEL_SPELL, m_spellProto->Id);
    }
}

void FillDynamicObjectTargetMapCallback::operator()(DynamicObject *obj, Unit *caster, Unit *target, float range)
{
    // skip units already hit, their range will be tested later
    if(obj->IsInTargetSet(target) || !sFactionSystem.isAttackable(caster, target, obj->IsTargettingStealth()))
        return;

    SpellEntry *spell = obj->m_spellProto;
    if(spell->isPassiveSpell() && target->HasAura(spell->Id))
        return; // We shouldn't be here, area auras don't cast passives afaik

    Aura *aur = NULL; SpellEntry *targetEntry = NULL;
    if((aur = target->m_AuraInterface.FindActiveAuraWithNameHash(spell->NameHash)) && (targetEntry = aur->GetSpellProto()) && targetEntry->maxstack > 1)
        target->AddAuraStack(spell, obj->casterGuid); // Update aura stack if we have a valid aura available
    else if(targetEntry && (targetEntry->procCharges || (spell->RankNumber && targetEntry->RankNumber >= spell->RankNumber)))
        target->RefreshAura(spell, obj->casterGuid); // Just refresh the aura, part of cata changes is it doesn't matter what rank
    else
    {   // We need to create and add a new aura
        uint16 auraFlags = spell->isPassiveSpell() ? 0x0000 : (AFLAG_EFF_AMOUNT_SEND | (spell->isNegativeSpell1() ? AFLAG_NEGATIVE : AFLAG_POSITIVE));
        aur = new Aura(target, spell, auraFlags, obj->casterLevel, 1, UNIXTIME, obj->casterGuid);
        for(uint32 i = 0; i < 3; i++)
            if(spell->EffectApplyAuraName[i])
                aur->AddMod(i, spell->EffectApplyAuraName[i], spell->CalculateSpellPoints(i, obj->casterLevel));

        aur->SetNegative();
        aur->UpdatePreApplication();
        target->AddAura(aur);
    }
    obj->targets.insert(target->GetGUID());
}

void DynamicObject::UpdateTargets(uint32 p_time)
{
    // Don't update targets till we're in world for sure
    if(!IsInWorld())
        return;

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
        float radius = 0.f;
        if(radius = GetFloatValue(DYNAMICOBJECT_RADIUS))
        {
            static FillDynamicObjectTargetMapCallback _callback;
            m_mapInstance->HandleDynamicObjectRangeMapping(&_callback, this, u_caster, 0.f, radius, (TYPEMASK_TYPE_UNIT|TYPEMASK_TYPE_PLAYER));
            radius *= radius;
        }

        // loop the targets, check the range of all of them
        DynamicObjectList::iterator jtr = targets.begin(), jtr2, jend = targets.end();
        while(jtr != jend)
        {
            jtr2 = jtr;
            ++jtr;

            Unit* target = GetMapInstance() ? GetMapInstance()->GetUnit(*jtr2) : NULL;
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
        for(DynamicObjectList::iterator itr = targets.begin(); itr != targets.end(); ++itr)
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

        m_mapInstance->QueueCleanup(this);
    }
    else Destruct();
}
