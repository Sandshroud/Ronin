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

Summon::Summon(CreatureData* data, uint64 guid, int32 duration) : Creature(), m_expireTime(duration), m_deleted(false)
{
    // Call object construct first
    Creature::Construct(data, guid);
    // Init

    m_Internal = NULL;
}

Summon::~Summon()
{

}

void Summon::Init()
{
    Creature::Init();
}

void Summon::Destruct()
{
    if(m_Internal != NULL)
        m_Internal->Destruct();
    Creature::Destruct();
}

void Summon::Update(uint32 msTime, uint32 uiDiff)
{
    if(m_deleted == true)
        return;

    Creature::Update(msTime, uiDiff);
    if(m_expireTime == -1)
        return;

    if(m_expireTime > uiDiff)
    {
        m_expireTime -= uiDiff;
        return;
    }

    s_Owner->RemoveSummon(this);
    Deactivate(0);
}

void Summon::InactiveUpdate(uint32 msTime, uint32 uiDiff)
{
    Cleanup();
}

void Summon::DetatchFromSummoner()
{
    if(m_deleted)
        return;

    m_deleted = true;
    Deactivate(0);
}

void Summon::OnPushToWorld(uint32 msTime)
{
    if(m_Internal != NULL)
        m_Internal->OnPushToWorld();
    WorldObject::OnPushToWorld(msTime);
}

void Summon::CreateAs(SummonHandler* NewHandle)
{
    m_Internal = NewHandle;
    m_Internal->Initialize(this);
}

void Summon::Load(Unit* m_owner, LocationVector & position, uint32 spellid, int32 summonslot)
{
    ASSERT(m_owner);

    Creature::Load(m_owner->GetMapId(), position.x, position.y, position.z, position.o, 0);

    SetInstanceID(m_owner->GetInstanceID());
    SetFaction(m_owner->GetFactionID());
    setLevel(m_owner->getLevel());
    SetCreatedBySpell(spellid);
    summonslot = summonslot;

    if(m_owner->IsPvPFlagged())
        SetPvPFlag();
    else RemovePvPFlag();

    if(m_owner->IsFFAPvPFlagged())
        SetFFAPvPFlag();
    else RemoveFFAPvPFlag();

    SetCreatedByGUID(m_owner->GetGUID());

    if(m_owner->IsSummon())
        SetSummonedByGUID(m_owner->GetSummonedByGUID());
    else SetSummonedByGUID(m_owner->GetGUID());

    if(m_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_STATUS))
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_STATUS);

    s_Owner = m_owner;

    if(m_Internal != NULL)
        m_Internal->Load(m_owner, position, spellid, summonslot);
}

void SummonHandler::Initialize(Summon* n_summon)
{
    m_summon = n_summon;
}

void CompanionSummon::Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot)
{
    SummonHandler::Load(owner, position, spellid, summonslot);
    m_summon->SetFaction(35);
    m_summon->setLevel(1);
    m_summon->RemovePvPFlag();
    m_summon->RemoveFFAPvPFlag();
}

void GuardianSummon::Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot)
{
    SummonHandler::Load(owner, position, spellid, summonslot);
    m_summon->SetPowerType(POWER_TYPE_MANA);
    m_summon->SetMaxPower(POWER_TYPE_MANA, m_summon->GetMaxPower(POWER_TYPE_MANA) + 28 + 10 * m_summon->getLevel());
    m_summon->SetPower(POWER_TYPE_MANA, m_summon->GetPower(POWER_TYPE_MANA) + 28 + 10 * m_summon->getLevel());
    m_summon->setLevel(owner->getLevel());
    m_summon->SetMaxHealth(m_summon->GetMaxHealth() + 28 + 30 * m_summon->getLevel());
    m_summon->SetHealth(m_summon->GetMaxHealth());

    m_summon->m_noRespawn = true;
}

void PossessedSummon::Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot)
{
    SummonHandler::Load(owner, position, spellid, summonslot);
    m_summon->setLevel(owner->getLevel());
}

void TotemSummon::Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot)
{
    SummonHandler::Load(owner, position, spellid, summonslot);
    uint32 displayID = m_summon->GetCreatureData()->displayInfo[0];
    if( owner->IsPlayer() && castPtr<Player>(owner)->GetTeam() == 0 )
    {
        if ( m_summon->GetCreatureData()->displayInfo[1] != 0 )
            displayID = m_summon->GetCreatureData()->displayInfo[1];
        else //this is the case when you are using a blizzlike db
        {
            if( m_summon->GetCreatureData()->displayInfo[0] == 4587 )
                displayID = 19075;
            else if( m_summon->GetCreatureData()->displayInfo[0] == 4588 )
                displayID = 19073;
            else if( m_summon->GetCreatureData()->displayInfo[0] == 4589 )
                displayID = 19074;
            else if( m_summon->GetCreatureData()->displayInfo[0] == 4590 )
                displayID = 19071;
            else if( m_summon->GetCreatureData()->displayInfo[0] == 4683 )
                displayID = 19074;
            else displayID = m_summon->GetCreatureData()->displayInfo[0];
        }
    }

    // Set up the creature.
    m_summon->SetMaxPower(POWER_TYPE_FOCUS, owner->getLevel() * 30);
    m_summon->SetPower(POWER_TYPE_FOCUS, owner->getLevel() * 30);
    m_summon->setLevel(owner->getLevel());
    m_summon->setRace(0);
    m_summon->setClass(1);
    m_summon->SetPowerType(1);
    m_summon->SetBoundingRadius(1.0f);
    m_summon->SetCombatReach(1.0f);
    m_summon->SetDisplayId(displayID);
    m_summon->SetNativeDisplayId(m_summon->GetCreatureData()->displayInfo[0]);
    m_summon->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
}

void TotemSummon::SetupSpells()
{
    if(m_summon->GetSummonOwner() == NULL)
        return;

    SpellEntry *TotemSpell = NULL;//objmgr.GetTotemSpell(m_summon->GetCreatedBySpell());
    SpellEntry *creatorspell = dbcSpell.LookupEntry(m_summon->GetCreatedBySpell());
    if(TotemSpell == NULL)
    {
        sLog.outError("Totem %u does not have any spells to cast", m_summon->GetEntry());
        return;
    }

    // Set up AI, depending on our spells.
    bool castingtotem = true;

    if((TotemSpell->HasEffect(SPELL_EFFECT_SUMMON)
        || TotemSpell->HasEffect(SPELL_EFFECT_APPLY_AURA)
        || TotemSpell->HasEffect(SPELL_EFFECT_APPLY_AREA_AURA)
        || TotemSpell->HasEffect(SPELL_EFFECT_APPLY_RAID_AURA)
        || TotemSpell->HasEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA))
        && TotemSpell->AppliesAura(SPELL_AURA_PERIODIC_TRIGGER_SPELL))
        castingtotem = false;

    if(!castingtotem)
    {
        // We're an area aura. Simply cast the spell.
        SpellCastTargets targets;
        if(!TotemSpell->AppliesAura(SPELL_AURA_PERIODIC_TRIGGER_SPELL))
        {
            targets.m_dest = m_summon->GetPosition();
            targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
        }

        if(Spell *spell = new Spell(m_summon, TotemSpell))
            spell->prepare(&targets, true);
    }
}

void WildSummon::Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot)
{
    m_summon->setLevel(owner->getLevel());
    SummonHandler::Load(owner, position, spellid, summonslot);
}

void SpellEffectClass::SpellEffectSummon(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    uint32 entry = m_spellInfo->EffectMiscValue[i];

    SummonPropertiesEntry * spe = dbcSummonProperties.LookupEntry( GetSpellProto()->EffectMiscValueB[i] );
    if( spe == NULL )
        return;

    CreatureData* ctrData = sCreatureDataMgr.GetCreatureData(entry);
    if(ctrData == NULL)
    {
        sLog.outError("Spell %u ( %s ) tried to summon creature %u without database data", m_spellInfo->Id, m_spellInfo->Name, entry);
        return;
    }

    // Damage must be 1 or more
    amount = std::max(1, amount);

    LocationVector v;
    GetSpellDestination(v);

    // Client adds these spells to the companion window, it's weird but then it happens anyways
    if(spe->slot == 5)
    {
        SummonCompanion(_unitCaster, i, amount, spe, ctrData, v);
        return;
    }

    // Delete any objects in my slots. Slot 0 can have unlimited objects.
    if( target->IsUnit() && spe->slot != 0 && spe->slot < 7 && castPtr<Unit>(target)->m_Summons[ spe->slot ].size() )
        castPtr<Unit>(target)->SummonExpireSlot(spe->slot);

    switch( spe->controltype )
    {
    case DBC_SUMMON_CONTROL_TYPE_GUARDIAN:
        {
            if(spe->Id == 121)
            {
                SummonTotem(_unitCaster, i, amount, spe, ctrData, v);
                return;
            }
        }break;
    case DBC_SUMMON_CONTROL_TYPE_PET:
        {
            SummonTemporaryPet(_unitCaster, i, amount, spe, ctrData, v);
            return;
        }break;
    case DBC_SUMMON_CONTROL_TYPE_POSSESSED:
        {
            SummonPossessed(_unitCaster, i, amount, spe, ctrData, v);
            return;
        }break;
    case DBC_SUMMON_CONTROL_TYPE_VEHICLE:
        {
            SummonVehicle(_unitCaster, i, amount, spe, ctrData, v);
            return;
        }break;
    }

    switch( spe->type )
    {
    case DBC_SUMMON_TYPE_NONE:
    case DBC_SUMMON_TYPE_CONSTRUCT:
    case DBC_SUMMON_TYPE_OPPONENT:
        {
            if(spe->controltype == DBC_SUMMON_CONTROL_TYPE_GUARDIAN)
                SummonGuardian(_unitCaster, i, amount, spe, ctrData, v, spe->type != DBC_SUMMON_TYPE_OPPONENT);
            else SummonWild(_unitCaster, i, amount, spe, ctrData, v);
            return;
        }break;
    case DBC_SUMMON_TYPE_PET:
        {
            SummonTemporaryPet(_unitCaster, i, amount, spe, ctrData, v);
        }break;
    case DBC_SUMMON_TYPE_GUARDIAN:
    case DBC_SUMMON_TYPE_MINION:
    case DBC_SUMMON_TYPE_RUNEBLADE:
        {
            SummonGuardian(_unitCaster, i, amount, spe, ctrData, v);
        }break;
    case DBC_SUMMON_TYPE_TOTEM:
        {
            SummonTotem(_unitCaster, i, amount, spe, ctrData, v);
        }break;
    case DBC_SUMMON_TYPE_COMPANION:
        {   // These are used as guardians in some quests
            if(spe->slot == 6)
                SummonGuardian(_unitCaster, i, amount, spe, ctrData, v);
            else SummonCompanion(_unitCaster, i, amount, spe, ctrData, v);
        }break;
    case DBC_SUMMON_TYPE_VEHICLE:
    case DBC_SUMMON_TYPE_MOUNT:
        {
            SummonVehicle(_unitCaster, i, amount, spe, ctrData, v);
        }break;
    case DBC_SUMMON_TYPE_LIGHTWELL:
        {
            SummonGuardian(_unitCaster, i, amount, spe, ctrData, v);
        }break;
    default:
        {
            sLog.outError("Spell Id %u, has an invalid summon type %u report this to devs.", GetSpellProto()->Id, spe->Id);
        }break;
    }
}

void SpellEffectClass::SummonWild(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry * Properties, CreatureData *data, LocationVector & v)
{
    int32 duration = GetDuration();
    uint32 slot = Properties ? Properties->slot : 0;
    for (int32 j = 0; j < amount; j++)
    {
        LocationVector SpawnLocation(v);
        float followangle = -1 * M_PI / 2 * j;
        SpawnLocation.x += (GetRadius(i) * (cosf(followangle + v.o)));
        SpawnLocation.y += (GetRadius(i) * (sinf(followangle + v.o)));
        followangle = (u_caster->calcAngle(u_caster->GetPositionX(), u_caster->GetPositionY(), SpawnLocation.x, SpawnLocation.y) * float(M_PI) / 180.0f);

        Summon* s = u_caster->GetMapInstance()->CreateSummon(data->entry, duration);
        if(s == NULL)
            return;

        u_caster->AddSummonToSlot(slot, s);
        s->CreateAs(new WildSummon());
        s->Load(u_caster, SpawnLocation, m_spellInfo->Id, slot);
        u_caster->GetMapInstance()->AddObject(s);

        if(Properties)
        {
            if(u_caster->IsPlayer() && (Properties->slot != 0))
            {
                WorldPacket data(SMSG_TOTEM_CREATED, 17);
                data << uint8(Properties->slot - 1);
                data << uint64(s->GetGUID());
                data << int32(duration);
                data << uint32(m_spellInfo->Id);
                castPtr<Player>(u_caster)->PushPacket(&data);
            }
        }
    }
}

void SpellEffectClass::SummonTotem(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry * Properties, CreatureData *data, LocationVector & v)
{
    int32 duration = GetDuration();
    uint32 slot = Properties ? Properties->slot : 1;
    bool xSubtractX = true;
    bool xSubtractY = true;
    if(slot < 3)
        xSubtractX = false;
    if((slot/2)*2 == slot)
        xSubtractY = false;

    v.x += xSubtractX ? -1.5f : 1.5f;
    v.y += xSubtractY ? -1.5f : 1.5f;
    v.z = u_caster->GetMapInstance()->GetWalkableHeight(u_caster, v.x, v.y, v.z + 2.f);

    Summon* s = u_caster->GetMapInstance()->CreateSummon(data->entry, duration);
    if(s == NULL)
        return;

    u_caster->AddSummonToSlot(slot, s);
    s->CreateAs(new TotemSummon());
    s->Load(u_caster, v, m_spellInfo->Id, slot);
    s->SetMaxHealth(amount);
    s->SetHealth(amount);
    u_caster->GetMapInstance()->AddObject(s);

    if(u_caster->IsPlayer())
    {
        WorldPacket data(SMSG_TOTEM_CREATED, 17);
        data << uint8(slot - 1);
        data << uint64(s->GetGUID());
        data << int32(duration);
        data << uint32(m_spellInfo->Id);
        castPtr<Player>(u_caster)->PushPacket(&data);
    }
}

void SpellEffectClass::SummonGuardian(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry * Properties, CreatureData *data, LocationVector & v, bool limitInteraction) // Summon Guardian
{
    int32 count = amount;
    // it's health., or a fucked up infernal.
    if( Properties == NULL )
        count = 1;
    else if (amount < 1 || (Properties->summonPropFlags & 2 || Properties->Id == 711))
        count = 1;
    if(data->entry == 31216) // mirror image
        count = 3;

    float angle_for_each_spawn = count > 1 ? -float((M_PI * 2.f) / count) : 0.f;

    int32 duration = GetDuration();
    uint32 slot = Properties ? Properties->slot : 0;
    for (int j = 0; j < count; j++)
    {
        LocationVector SpawnLocation(v);
        float followangle = angle_for_each_spawn * j;
        if(angle_for_each_spawn)
        {
            SpawnLocation.x += (GetRadius(i) * (cosf(followangle)));
            SpawnLocation.y += (GetRadius(i) * (sinf(followangle)));
            followangle = (u_caster->calcAngle(u_caster->GetPositionX(), u_caster->GetPositionY(), SpawnLocation.x, SpawnLocation.y) * float(M_PI / ((180 / count) * (j + 1))));
        }

        Summon* s = u_caster->GetMapInstance()->CreateSummon(data->entry, duration);
        if(s == NULL)
            return;

        u_caster->AddSummonToSlot(slot, s);
        s->CreateAs(new GuardianSummon());
        s->Load(u_caster, SpawnLocation, m_spellInfo->Id, slot);
        u_caster->GetMapInstance()->AddObject(s);

        if(u_caster->IsPlayer() && (slot != 0))
        {
            WorldPacket data(SMSG_TOTEM_CREATED, 17);
            data << uint8(slot - 1);
            data << uint64(s->GetGUID());
            data << int32(duration);
            data << uint32(m_spellInfo->Id);
            castPtr<Player>(u_caster)->PushPacket(&data);
        }

        // Lightwell
        if(Properties != NULL && Properties->type == DBC_SUMMON_TYPE_LIGHTWELL)
        {
            s->GetMovementInterface()->setRooted(true);
            s->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }
        else if(limitInteraction == true)
        {
            s->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            s->GetMovementInterface()->setRooted(true);
        }
    }
}

void SpellEffectClass::SummonTemporaryPet(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry * Properties, CreatureData *data, LocationVector & v)
{

}

void SpellEffectClass::SummonPossessed(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry * Properties, CreatureData *data, LocationVector & v)
{
    int32 duration = GetDuration();
    uint32 slot = Properties ? Properties->slot : 0;
    Summon* s = u_caster->GetMapInstance()->CreateSummon(data->entry, duration);
    if(s == NULL)
        return;

    v.x += (3 * cos(M_PI / 2 + v.o));
    v.y += (3 * cos(M_PI / 2 + v.o));

    s->CreateAs(new PossessedSummon());
    s->Load(u_caster, v, m_spellInfo->Id, slot);
    s->SetCreatedBySpell(m_spellInfo->Id);
    u_caster->GetMapInstance()->AddObject(s);

    if(u_caster->IsPlayer())
        castPtr<Player>(u_caster)->Possess(s);
    //else s->SetPossessedBy(u_caster);
}

void SpellEffectClass::SummonCompanion(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry * Properties, CreatureData *data, LocationVector & v)
{
    int32 duration = GetDuration();
    uint32 slot = Properties ? Properties->slot : 0;
    if(u_caster->GetSummonedCritterGUID() != 0)
    {
        Creature* critter = u_caster->GetMapInstance()->GetCreature(u_caster->GetSummonedCritterGUID());
        uint32 currententry = critter->GetEntry();
        critter->Cleanup();
        u_caster->SetSummonedCritterGUID(0);

        // Before WOTLK when you casted the companion summon spell the second time it removed the companion
        // Customized servers or old databases could still use this method
        if(data->entry == currententry)
            return;
    }

    Summon* s = u_caster->GetMapInstance()->CreateSummon(data->entry, duration);
    if(s == NULL)
        return;

    s->CreateAs(new CompanionSummon());
    s->Load(u_caster, v, m_spellInfo->Id, slot);
    s->SetCreatedBySpell(m_spellInfo->Id);
    u_caster->GetMapInstance()->AddObject(s);
    u_caster->SetSummonedCritterGUID(s->GetGUID());
}

void SpellEffectClass::SummonVehicle(Unit *u_caster, uint32 i, int32 amount, SummonPropertiesEntry * Properties, CreatureData *data, LocationVector & v)
{

}
