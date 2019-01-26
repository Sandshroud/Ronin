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

#define INC_INDEXORBLOCK_MACRO(x, skipNeg) ( (x < MAX_POSITIVE_AURAS && x+1 > m_maxPosAuraSlot) ? x = (skipNeg ? MAX_AURAS : MAX_POSITIVE_AURAS) : (x < MAX_AURAS && x+1 > m_maxNegAuraSlot) ? x = MAX_AURAS : x++)

AuraInterface::AuraInterface(Unit *unit) : m_Unit(unit), m_maxPosAuraSlot(0), m_maxNegAuraSlot(MAX_POSITIVE_AURAS), m_maxPassiveAuraSlot(MAX_AURAS)
{
    for(uint8 i = 0; i < TOTAL_AURAS; i++)
        m_auras[i] = NULL;
}

AuraInterface::~AuraInterface()
{

}

void AuraInterface::Destruct()
{
    m_Unit = NULL;
    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if(Aura *aur = m_auras[x])
            delete aur;
    while(!m_modifierHolders.empty())
    {
        delete m_modifierHolders.begin()->second;
        m_modifierHolders.erase(m_modifierHolders.begin());
    }
    m_modifierHolders.clear();
}

void AuraInterface::Update(uint32 diff)
{
    // Passive auras don't expire so only update non passive auras
    // Do not use iterators because update can invalidate them when they're removed
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if(Aura *aur = m_auras[x])
            aur->Update(diff);
}

void AppendAuraAndModifierData(std::stringstream *ss, uint32 lowGuid, uint32 auraSlot, Aura *aur, Modifier **mods)
{
    uint16 auraFlags = aur->GetAuraFlags() & 0xFF;
    for(uint8 i = 0; i < 3; i++)
        if(mods[i] != NULL)
            auraFlags |= (1<<(i+8));

    *ss << "(" << lowGuid
        << ", " << auraSlot
        << ", " << aur->GetSpellId()
        << ", " << uint32(auraFlags)
        << ", " << uint32(aur->GetAuraLevel())
        << ", " << int32(aur->getStackSizeOrProcCharges())
        << ", " << uint64(aur->GetCasterGUID())
        << ", " << uint64(aur->GetExpirationTime())
        << ", " << int32(mods[0] ? mods[0]->m_baseAmount : 0)
        << ", " << int32(mods[1] ? mods[1]->m_baseAmount : 0)
        << ", " << int32(mods[2] ? mods[2]->m_baseAmount : 0)
        << ", " << uint32(mods[0] ? mods[0]->m_bonusAmount : 0)
        << ", " << uint32(mods[1] ? mods[1]->m_bonusAmount : 0)
        << ", " << uint32(mods[2] ? mods[2]->m_bonusAmount : 0)
        << ", " << int32(mods[0] ? mods[0]->fixed_amount : 0)
        << ", " << int32(mods[1] ? mods[1]->fixed_amount : 0)
        << ", " << int32(mods[2] ? mods[2]->fixed_amount : 0)
        << ", " << float(mods[0] ? mods[0]->fixed_float_amount : 0.f)
        << ", " << float(mods[1] ? mods[1]->fixed_float_amount : 0.f)
        << ", " << float(mods[2] ? mods[2]->fixed_float_amount : 0.f);
    *ss << ")";
}

void AuraInterface::SavePlayerAuras(std::stringstream *ss)
{
    for(uint8 i = 0; i < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(i, false))
    {
        Aura *aur = FindAuraBySlot(i);
        if(aur == NULL)
            continue;
        if(aur->GetCasterGUID() != m_Unit->GetGUID())
        {   // Only save apply aura spells, area auras should be ignored
            if(!aur->GetSpellProto()->HasEffect(SPELL_EFFECT_APPLY_AURA))
                continue;
        }

        if(ss->str().length())
            *ss << ", ";

        Modifier *mods[3] = { aur->GetMod(0), aur->GetMod(1), aur->GetMod(2) };
        AppendAuraAndModifierData(ss, m_Unit->GetLowGUID(), i, aur, mods);
    }
}

void AuraInterface::OnChangeLevel(uint32 newLevel)
{
    // On target level change recalculate modifiers where caster is unit
    for(uint8 i = 0; i < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(i, true))
    {
        Aura *aur = m_auras[i];
        if(aur == NULL)
            continue;

        // For now, we'll only update auras we've casted on ourself
        if(m_Unit->GetGUID() != aur->GetCasterGUID())
            continue;

        aur->OnTargetChangeLevel(newLevel, m_Unit->GetGUID());
    }
}

uint8 AuraInterface::GetFreeSlot(bool ispositive)
{
    uint8 begin = ispositive ? 0 : MAX_POSITIVE_AURAS, end = ispositive ? MAX_POSITIVE_AURAS : MAX_AURAS;
    for (uint8 i = begin; i < end; i++)
    {
        if(m_auras[i] == NULL)
        {
            // Return the index
            return i;
        }
    }

    return 0xFF;
}

void AuraInterface::OnAuraRemove(Aura* aura, uint8 aura_slot)
{
    if(aura_slot == 0xFF)
        return;

    if(aura_slot > TOTAL_AURAS)
    {
        for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
            if(m_auras[x] == aura)
                m_auras[x] = NULL;
    } else if(m_auras[aura_slot] == aura)
        m_auras[aura_slot] = NULL;

    if(uint8 index = aura->GetSpellProto()->buffIndex)
        if(m_buffIndexAuraSlots[index] == aura_slot)
            m_buffIndexAuraSlots.erase(index);

    if(m_shapeShiftAuras.find(aura_slot) != m_shapeShiftAuras.end())
        m_shapeShiftAuras.erase(aura_slot);
}

void AuraInterface::OnSetShapeShift(uint8 SS, uint8 oldSS)
{
    SpellEntry *proto = NULL;
    std::set<uint8> m_aurasToRemove;
    for(uint8 x = 0; x < m_maxPosAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if((proto = aur->GetSpellProto())->AppliesAura(SPELL_AURA_MOD_SHAPESHIFT))
            {
                bool isOldSS = false;
                for(uint8 i = 0; i < 3; ++i)
                {
                    if(proto->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT)
                    {
                        isOldSS = proto->EffectMiscValue[i] != SS;
                        break;
                    }
                }

                if(isOldSS == true)
                    m_aurasToRemove.insert(x);
                continue;
            }

            // Check if it's a shapeshift required aura and if we're either a 0 ss or if we don't meet the requirement
            if((m_shapeShiftAuras.find(x) != m_shapeShiftAuras.end()) && (SS == NULL || ((aur->GetSpellProto()->RequiredShapeShift & (uint32)1 << (SS-1)) == 0)))
                m_aurasToRemove.insert(x);
        }
    }

    for(std::set<uint8>::iterator itr = m_aurasToRemove.begin(); itr != m_aurasToRemove.end(); itr++)
        RemoveAuraBySlot(*itr);
    m_aurasToRemove.clear();
}

void AuraInterface::OnDeath()
{
    std::set<uint8> m_aurasToRemove;
    for (uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if (Aura *aur = m_auras[x])
        {
            if (aur->GetSpellProto()->isDeathPersistentAura())
                continue;
            m_aurasToRemove.insert(x);
        }
    }

    for (std::set<uint8>::iterator itr = m_aurasToRemove.begin(); itr != m_aurasToRemove.end(); itr++)
        RemoveAuraBySlot(*itr);
    m_aurasToRemove.clear();
}

bool AuraInterface::IsDazed()
{
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->MechanicsType == MECHANIC_ENSNARED)
                return true;

            for(uint32 y = 0; y < 3; y++)
            {
                if(aur->GetSpellProto()->EffectMechanic[y]==MECHANIC_ENSNARED)
                    return true;
            }
        }
    }

    return false;
}

bool AuraInterface::IsPoisoned()
{
    uint8 buffIndex = BUFF_ROGUE_POISON_START;
    while(BuffSets[buffIndex] != BUFF_NONE)
    {
        if(m_buffIndexAuraSlots.find(BuffSets[buffIndex]) != m_buffIndexAuraSlots.end())
            return true;
    }
    return false;
}

void AuraInterface::UpdateDuelAuras()
{
    for( uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; ++x )
        if( Aura *aur = m_auras[x])
            if(aur->WasCastInDuel())
                RemoveAuraBySlot(x);
}

void AuraInterface::SendAuraData()
{
    if(!m_Unit->IsPlayer())
        return;

    Player *plr = castPtr<Player>(m_Unit);
    WorldPacket data(SMSG_AURA_UPDATE_ALL, 200);
    data << plr->GetGUID().asPacked();

    bool empty = true;
    for (uint8 i = 0; i < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(i, false))
    {
        Aura *aur = m_auras[i];
        if(aur == NULL)
            continue;
        aur->BuildAuraUpdatePacket(&data);
        empty = false;
    }

    if(empty == false)
        plr->PushPacket(&data);

    for(uint8 i = 0; i < 2; i++)
    {
        uint32 modCount = 0;
        data.Initialize(SMSG_SET_FLAT_SPELL_MODIFIER+i, 200);
        data << modCount;
        for(Loki::AssocVector<uint16, Loki::AssocVector<uint8, int32>>::iterator itr = m_spellGroupModifiers.begin(); itr != m_spellGroupModifiers.end(); itr++)
        {
            uint8 modType = (itr->first >> 8);
            // Indexing for our mapping
            if(modType != i)
                continue;

            modCount++;
            data << uint32(itr->second.size());
            data << uint8(itr->first & 0x00FF);
            if(itr->second.empty())
                continue;

            for(Loki::AssocVector<uint8, int32>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++)
            {
                data.append<uint8>(itr2->first);
                data.append<float>(itr2->second);
            }
        }

        if(modCount)
        {
            data.put<uint32>(0, modCount);
            plr->PushPacket(&data);
        }
    }
}

void AuraInterface::BuildOutOfRangeAuraUpdate(WorldPacket *data)
{
    // We're cutting off the last auras above index 32
    static uint32 indexOffset = MAX_POSITIVE_AURAS-32;

    *data << uint16(0) << uint8(1); // Full update
    size_t pos = data->wpos();
    uint64 auraMask = 0;
    *data << auraMask;
    *data << uint32(MAX_AURAS-indexOffset);
    for(uint8 i = 0; i < std::min<uint8>(m_maxPosAuraSlot, 32); i++)
    {
        Aura *aur = m_auras[i];
        if(aur == NULL)
            continue;

        auraMask |= (((uint64)1)<<((uint64)i));
        *data << aur->GetSpellId();
        uint16 auraFlags = aur->GetAuraFlags();
        *data << auraFlags;
        if (auraFlags & AFLAG_EFF_AMOUNT_SEND)
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                Modifier *mod = aur->GetMod(i);
                *data << uint32(mod ? mod->m_amount : 0);
            }
        }
    }

    for(uint8 i = MAX_POSITIVE_AURAS; i < m_maxNegAuraSlot; i++)
    {
        Aura *aur = m_auras[i];
        if(aur == NULL)
            continue;

        auraMask |= (((uint64)1)<<((uint64)(i-indexOffset)));
        *data << aur->GetSpellId();
        uint16 auraFlags = aur->GetAuraFlags();
        *data << auraFlags;
        if (auraFlags & AFLAG_EFF_AMOUNT_SEND)
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                Modifier *mod = aur->GetMod(i);
                *data << uint32(mod ? mod->m_amount : 0);
            }
        }
    }

    data->put<uint64>(pos, auraMask);
}

bool AuraInterface::BuildAuraUpdateAllPacket(WorldPacket* data)
{
    bool res = false;
    for (uint8 i = 0; i < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(i, false))
    {
        Aura *aur = m_auras[i];
        if(aur == NULL)
            continue;
        aur->BuildAuraUpdatePacket(data);
        res = true;
    }
    return res;
}

void AuraInterface::SpellStealAuras(Unit* caster, int32 MaxSteals)
{
    int32 spells_to_steal = MaxSteals > 1 ? MaxSteals : 1;
    for(uint8 x = 0; x < m_maxPosAuraSlot; x++)
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur != NULL && aur->GetSpellId() != 15007 && !aur->IsPassive() && aur->IsPositive()) //Nothing can dispel resurrection sickness
            {
                if(aur->GetSpellProto()->DispelType == DISPEL_MAGIC && aur->GetDuration() > 0)
                {
                    WorldPacket data(SMSG_SPELLDISPELLOG, 16);
                    data << caster->GetGUID();
                    data << m_Unit->GetGUID();
                    data << uint32(1);
                    data << aur->GetSpellId();
                    caster->SendMessageToSet(&data,true);

                    //caster->target->ApplyAura(aur->GetSpellProto(), caster, caster, aur->getStackSize()-1);

                    RemoveAuraBySlot(x);
                    if( --spells_to_steal <= 0 )
                        break; //exit loop now
                }
            }
        }
    }
}

void AuraInterface::AttemptDispel(Unit* caster, int32 Mechanic, bool hostile)
{
    SpellEntry *p = NULL;
    if( hostile )
    {
        for( uint8 x = 0; x < m_maxPosAuraSlot; x++ )
        {
            if( Aura *aur = m_auras[x] )
            {
                if(aur->IsPositive())
                {
                    p = aur->GetSpellProto();
                    if( Spell::HasMechanic(p, Mechanic) )
                    {
                        aur->AttemptDispel( caster );
                    }
                }
            }
        }
    }
    else
    {
        for( uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; x++ )
        {
            if( Aura *aur = m_auras[x] )
            {
                if(!aur->IsPositive())
                {
                    p = aur->GetSpellProto();
                    if( Spell::HasMechanic(p, Mechanic) )
                    {
                        aur->AttemptDispel( caster );
                    }
                }
            }
        }
    }
}

void AuraInterface::MassDispel(Unit* caster, uint32 index, SpellEntry* Dispelling, uint32 MaxDispel, uint8 start, uint8 end)
{
    ASSERT(start < MAX_AURAS && end <= MAX_AURAS);
    WorldPacket data(SMSG_SPELLDISPELLOG, 16);

    for(uint8 x = start; x < end; x++)
    {
        if(Aura *aur = m_auras[x])
        {
            //Nothing can dispel resurrection sickness;
            if(aur != NULL && !aur->IsPassive() && !(aur->GetSpellProto()->isUnstoppableForce2()))
            {
                int32 resistchance = 0;
                if(Unit* caster = aur->GetUnitCaster())
                {
                    caster->SM_FIValue(SMT_RESIST_DISPEL, &resistchance, aur->GetSpellProto()->SpellGroupType);

                    if( !Rand(resistchance) )
                    {
                        if(Dispelling->DispelType == DISPEL_ALL)
                        {
                            data.clear();
                            data << caster->GetGUID();
                            data << m_Unit->GetGUID();
                            data << (uint32)1;//probably dispel type
                            data << aur->GetSpellId();
                            caster->SendMessageToSet(&data,true);
                            aur->AttemptDispel( caster );
                            if(!--MaxDispel)
                                return;
                        }
                        else if(aur->GetSpellProto()->DispelType == Dispelling->EffectMiscValue[index])
                        {
                            data.clear();
                            data << caster->GetGUID();
                            data << m_Unit->GetGUID();
                            data << (uint32)1;
                            data << aur->GetSpellId();
                            caster->SendMessageToSet(&data,true);
                            aur->AttemptDispel( caster );
                            if(!--MaxDispel)
                                return;
                        }
                    }
                    else if( !--MaxDispel )
                        return;
                }
            }
        }
    }
}

void AuraInterface::RemoveAllAurasWithDispelType(uint32 DispelType)
{
    Aura *aur = NULL;
    for( uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
        if((aur = m_auras[x]) && aur->GetSpellProto()->DispelType == DispelType)
            RemoveAuraBySlot(x);
}

void AuraInterface::RemoveAllAurasWithAttributes(uint8 index, uint32 attributeFlag)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
        if((aur = m_auras[x]) && aur->GetSpellProto()->Attributes[index] & attributeFlag)
            RemoveAuraBySlot(x);
}

void AuraInterface::RemoveAllAurasOfSchool(uint32 School, bool Positive, bool Immune)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
    {
        if(Aura *aur = m_auras[x])
        {
            if(!Immune && (aur->GetSpellProto()->isUnstoppableForce2()))
                continue;
            if(aur->GetSpellProto()->School == School && (!aur->IsPositive() || Positive))
                RemoveAuraBySlot(x);
        }
    }
}

void AuraInterface::RemoveAllAurasByInterruptFlagButSkip(uint32 flag, uint32 skip)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
    {
        Aura *aur = m_auras[x];
        if(aur == NULL)
            continue;

        if((aur->GetSpellProto()->AuraInterruptFlags & flag) && (aur->GetSpellProto()->Id != skip))
            RemoveAuraBySlot(x);
    }
}

uint32 AuraInterface::GetSpellIdFromAuraSlot(uint32 slot)
{
    if(Aura *aur = m_auras[slot])
        return aur->GetSpellId();
    return 0;
}

AuraCheckResponse AuraInterface::AuraCheck(SpellEntry *info, WoWGuid casterGuid)
{
    AuraCheckResponse resp;

    // no error for now
    resp.Error = AURA_CHECK_RESULT_NONE;
    resp.Misc  = 0;

    if(uint8 index = info->buffIndex)
    {
        uint8 indexStart = GetBuffSetStartForIndex(index);
        if(indexStart != MAX_BUFFS)
        {
            std::map<uint8, uint8>::iterator itr;
            while(BuffSets[++indexStart] != BUFF_NONE)
            {
                if((itr = m_buffIndexAuraSlots.find(BuffSets[indexStart])) != m_buffIndexAuraSlots.end())
                {
                    Aura *curAura = m_auras[itr->second];
                    if(curAura->GetCasterGUID() == casterGuid)
                    {
                        resp.Error = AURA_CHECK_RESULT_LOWER_BUFF_PRESENT;
                        resp.Misc = itr->second;
                    }
                    else if(BuffSets[indexStart] == index)
                    {
                        if(curAura->GetSpellProto()->Rank > info->Rank)
                        {
                            resp.Error = AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT;
                            break;
                        }
                    }
                }
            }
        }
    }

    if(resp.Error != AURA_CHECK_RESULT_NONE)
        return resp;

    // look for spells with same namehash
    bool stronger = false;
    for(uint8 x = 0; x < m_maxNegAuraSlot; (x+1 == m_maxPosAuraSlot ? x = MAX_POSITIVE_AURAS : x++))
    {
        Aura *aur = m_auras[x];
        if(aur == NULL)
            continue;
        if(casterGuid == aur->GetCasterGUID())
            continue; // We can skip auras cast by ourself

        SpellEntry *currInfo = aur->GetSpellProto();
        if(info->NameHash == currInfo->NameHash)
        {
            if(info->RankNumber < currInfo->RankNumber)
                stronger = true;
            else if(info->maxstack > 1 && aur->getStackSize() > 1)
                stronger = true;
            else
            {
                resp.Error = AURA_CHECK_RESULT_LOWER_BUFF_PRESENT;
                resp.Misc = x;
            }
        }

        if( stronger )
        {
            resp.Error = AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT;
            break;
        }
    }

    return resp; // return it back to our caller
}

uint32 AuraInterface::GetAuraSpellIDWithNameHash(uint32 name_hash)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
        if((aur = m_auras[x]) && aur->GetSpellProto()->NameHash == name_hash)
            return aur->GetSpellProto()->Id;
    return 0;
}

bool AuraInterface::HasAura(uint32 spellid)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
        if((aur = m_auras[x]) && aur->GetSpellId() == spellid)
            return true;
    return false;
}

bool AuraInterface::HasAuraVisual(uint32 visualid)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
        if((aur = m_auras[x]) && (aur->GetSpellProto()->SpellVisual[0] == visualid || aur->GetSpellProto()->SpellVisual[1] == visualid))
            return true;
    return false;
}

bool AuraInterface::HasActiveAura(uint32 spellid)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
        if((aur = m_auras[x]) && aur->GetSpellId() == spellid)
            return true;
    return false;
}

bool AuraInterface::HasNegativeAura(uint32 spell_id)
{
    Aura *aur = NULL;
    for(uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; ++x)
        if((aur = m_auras[x]) && aur->GetSpellProto()->Id == spell_id)
            return true;
    return false;
}

bool AuraInterface::HasAuraWithMechanic(uint32 mechanic)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
        if((aur = m_auras[x]) && aur->GetMechanic() == mechanic)
            return true;
    return false;
}

bool AuraInterface::HasActiveAura(uint32 spellid, WoWGuid guid)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
        if((aur = m_auras[x]) && aur->GetSpellId() == spellid && (guid.empty() || aur->GetCasterGUID() == guid))
            return true;
    return false;
}

bool AuraInterface::HasPosAuraWithMechanic(uint32 mechanic)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxPosAuraSlot; x++)
        if( (aur = m_auras[x] ) && aur->GetMechanic() == mechanic )
            return true;
    return false;
}

bool AuraInterface::HasNegAuraWithMechanic(uint32 mechanic)
{
    Aura *aur = NULL;
    for(uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; ++x)
        if((aur = m_auras[x]) && aur->GetMechanic() == mechanic )
            return true;
    return false;
}

bool AuraInterface::HasNegativeAuraWithNameHash(uint32 name_hash)
{
    Aura *aur = NULL;
    for(uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; ++x)
        if((aur = m_auras[x]) && aur->GetSpellProto()->NameHash == name_hash)
            return true;
    return false;
}

bool AuraInterface::HasCombatStatusAffectingAuras(WoWGuid checkGuid)
{
    Aura *aur = NULL;
    for(uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; ++x)
        if((aur = m_auras[x]) && checkGuid == aur->GetCasterGUID() && aur->IsCombatStateAffecting())
            return true;
    return false;
}

bool AuraInterface::HasAurasOfNameHashWithCaster(uint32 namehash, WoWGuid casterguid)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
        if((aur = m_auras[x]) && aur->GetSpellProto()->NameHash == namehash && (casterguid.empty() || aur->GetCasterGUID() == casterguid))
            return true;
    return false;
}

bool AuraInterface::OverrideSimilarAuras(WorldObject *caster, Aura *aur)
{
    std::set<uint8> m_aurasToRemove;
    uint32 maxStack = aur->GetSpellProto()->maxstack;

    SpellEntry *info = aur->GetSpellProto();
    if(uint8 index = info->buffIndex)
    {
        uint8 indexStart = GetBuffSetStartForIndex(index);
        if(indexStart != MAX_BUFFS)
        {
            std::map<uint8, uint8>::iterator itr;
            while(BuffSets[++indexStart] != BUFF_NONE)
            {
                if((itr = m_buffIndexAuraSlots.find(BuffSets[indexStart])) != m_buffIndexAuraSlots.end())
                {
                    Aura *curAura = m_auras[itr->second];
                    if(curAura->GetCasterGUID() == caster->GetGUID())
                        m_aurasToRemove.insert(itr->second);
                    else if(BuffSets[indexStart] == index)
                    {
                        if(curAura->GetSpellProto()->Rank > aur->GetSpellProto()->Rank)
                            return false;
                        for(uint8 i = 0; i < 3; i++)
                        {
                            if(curAura->GetMod(i)->m_amount > aur->GetMod(i)->m_amount)
                                return false;
                        }
                    }
                }
            }
        }
    }

    // Now we process our aura list
    for( uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
    {
        Aura *curAura = m_auras[x];
        if(curAura == NULL || aur == curAura || curAura->IsDeleted())
            continue;
        if(m_aurasToRemove.find(x) != m_aurasToRemove.end())
            continue; // Don't waste time checking aura's we're removing

        SpellEntry *currSP = curAura->GetSpellProto();
        if(info->NameHash && info->NameHash == currSP->NameHash)
        {
            if(aur->IsPositive() == curAura->IsPositive())
            {
                if(info->always_apply)
                    m_aurasToRemove.insert(x);
                else if( curAura->GetCasterGUID() == aur->GetCasterGUID() )
                {
                    if(info->procCharges || maxStack > 1)
                    {
                        // target already has this aura. Update duration, time left, procCharges
                        curAura->ResetExpirationTime();
                        // If we have proc charges, reset the proc charges
                        if(info->procCharges) curAura->SetProcCharges(aur->GetMaxProcCharges(aur->GetUnitCaster()));
                        else curAura->AddStackSize(1);   // increment stack size
                        return false;
                    }
                    m_aurasToRemove.insert(x);
                }
                else
                {
                    if( currSP->RankNumber <= info->RankNumber )
                    {
                        m_aurasToRemove.insert(x);
                        continue;
                    }
                    else if(info->isUnique)
                    {
                        // Unique spells need to rebound, otherwise we can just apply
                        return false;
                    }
                }
            } else if(curAura->IsPositive())
                m_aurasToRemove.insert(x);
            else return false;
        }
    }

    for(std::set<uint8>::iterator itr = m_aurasToRemove.begin(); itr != m_aurasToRemove.end(); itr++)
        RemoveAuraBySlot(*itr);
    m_aurasToRemove.clear();
    return true;
}

bool AuraInterface::UpdateAuraModifier(uint32 spellId, WoWGuid casterGuid, uint8 indexEff, Modifier *mod)
{
    return true;
    if(Aura *aur = FindAura(spellId, casterGuid))
    {
        //aur->Refresh();
        return true;
    }

    return false;
}

void AuraInterface::UpdateAuraModsWithModType(uint32 modType)
{
    m_auraLock.HighAcquire();
    if(m_Unit->IsPlayer() && m_modifiersByModType.find(modType) != m_modifiersByModType.end() && !m_modifiersByModType[modType].empty())
    {
        for(auto itr = m_modifiersByModType[modType].begin(); itr != m_modifiersByModType[modType].end(); itr++)
        {   // First remove the modifier group
            UpdateSpellGroupModifiers(false, itr->second, true);
            _RecalculateModAmountByType(itr->second);
            UpdateSpellGroupModifiers(true, itr->second, true);
        }
    }
    m_auraLock.HighRelease();
}

void AuraInterface::OnDismount()
{
    RWGuard guard(m_auraLock, false);
    if(m_modifiersByModType.find(SPELL_AURA_MOUNTED) == m_modifiersByModType.end() || m_modifiersByModType[SPELL_AURA_MOUNTED].empty())
        return;

    std::set<uint8> m_aurasToRemove;
    for(auto itr = m_modifiersByModType[SPELL_AURA_MOUNTED].begin(); itr != m_modifiersByModType[SPELL_AURA_MOUNTED].end(); itr++)
        m_aurasToRemove.insert((uint8)(itr->first&0xFF));
    guard.Nullify();

    while(!m_aurasToRemove.empty())
    {
        uint8 auraSlot = *m_aurasToRemove.begin();
        m_aurasToRemove.erase(m_aurasToRemove.begin());
        RemoveAuraBySlot(auraSlot);
    }
}

bool AuraInterface::HasMountAura()
{
    RWGuard guard(m_auraLock, false);
    if(m_modifiersByModType.find(SPELL_AURA_MOUNTED) != m_modifiersByModType.end() && !m_modifiersByModType[SPELL_AURA_MOUNTED].empty())
        return true;
    return false;
}

bool AuraInterface::GetMountedAura(uint32 &auraId)
{
    auraId = 0;
    RWGuard guard(m_auraLock, false);
    if(m_modifiersByModType.find(SPELL_AURA_MOUNTED) != m_modifiersByModType.end() && !m_modifiersByModType[SPELL_AURA_MOUNTED].empty())
        auraId = m_modifiersByModType[SPELL_AURA_MOUNTED].begin()->second->m_spellInfo->Id;
    return auraId != 0;
}

bool AuraInterface::HasFlightAura()
{
    RWGuard guard(m_auraLock, false);
    if(m_modifiersByModType.find(SPELL_AURA_FLY) != m_modifiersByModType.end() && !m_modifiersByModType[SPELL_AURA_FLY].empty())
        return true;
    return false;
}

void AuraInterface::RemoveFlightAuras()
{
    RWGuard guard(m_auraLock, false);
    if(m_modifiersByModType.find(SPELL_AURA_FLY) == m_modifiersByModType.end() || m_modifiersByModType[SPELL_AURA_FLY].empty())
        return;

    std::set<uint8> m_aurasToRemove;
    for(auto itr = m_modifiersByModType[SPELL_AURA_FLY].begin(); itr != m_modifiersByModType[SPELL_AURA_FLY].end(); itr++)
        m_aurasToRemove.insert((uint8)(itr->first&0xFF));
    guard.Nullify();

    while(!m_aurasToRemove.empty())
    {
        uint8 auraSlot = *m_aurasToRemove.begin();
        m_aurasToRemove.erase(m_aurasToRemove.begin());
        RemoveAuraBySlot(auraSlot);
    }
}

void AuraInterface::AddAura(Aura* aur, uint8 slot)
{
    WorldObject *caster = NULL;
    if((caster = aur->GetUnitCaster()) == NULL)
    {
        WoWGuid guid = aur->GetCasterGUID();
        if( m_Unit->GetGUID() == guid )
            caster = m_Unit;
        else if(guid.getHigh() == HIGHGUID_TYPE_GAMEOBJECT)
            caster = m_Unit->GetMapInstance()->GetGameObject(guid);
    }

    if(caster == NULL || !aur->IsPassive() && !OverrideSimilarAuras(caster, aur))
    {
        RemoveAura(aur);
        return;
    }

    ////////////////////////////////////////////////////////
    if( aur->GetAuraSlot() != 0xFF && aur->GetAuraSlot() < TOTAL_AURAS && m_auras[aur->GetAuraSlot()] != NULL )
        RemoveAuraBySlot(aur->GetAuraSlot());
    else
    {
        aur->SetAuraSlot(slot);
        if(slot != 0xFF && slot < TOTAL_AURAS && m_auras[slot] != NULL)
            RemoveAuraBySlot(slot);
    }

    RWGuard guard(m_auraLock, false);
    if(aur->IsPassive())
    {
        for(uint8 x = MAX_AURAS; x < TOTAL_AURAS; x++)
        {
            if(m_auras[x] == NULL)
            {
                aur->SetAuraSlot(x);
                break;
            }
        }

        if(aur->GetAuraSlot() == 0xFF)
        {
            sLog.Debug("Unit","AddAura error in passive aura. removing. SpellId: %u", aur->GetSpellProto()->Id);
            RemoveAura(aur);
            return;
        }
    }
    else if(aur->GetAuraSlot() == 0xFF || m_auras[aur->GetAuraSlot()] != NULL)
    {
        if(!aur->AddAuraVisual())
        {
            for(uint8 x = MAX_AURAS; x < TOTAL_AURAS; x++)
            {
                if(m_auras[x] == NULL)
                {
                    aur->SetAuraSlot(x);
                    break;
                }
            }
        }

        if(aur->GetAuraSlot() == 0xFF)
        {
            sLog.Debug("Unit","AddAura error in active aura. removing. SpellId: %u", aur->GetSpellProto()->Id);
            RemoveAura(aur);
            return;
        }

    } else aur->BuildAuraUpdate();
    guard.Nullify();

    m_auraLock.HighAcquire();
    m_auras[aur->GetAuraSlot()] = aur;
    if(aur->GetAuraSlot() < MAX_POSITIVE_AURAS)
        m_maxPosAuraSlot = std::max<uint8>(m_maxPosAuraSlot, 1+aur->GetAuraSlot());
    else if(aur->GetAuraSlot() < MAX_AURAS)
        m_maxNegAuraSlot = std::max<uint8>(m_maxNegAuraSlot, 1+aur->GetAuraSlot());
    else if(aur->GetAuraSlot() < TOTAL_AURAS)
        m_maxPassiveAuraSlot = std::max<uint8>(m_maxPassiveAuraSlot, 1+aur->GetAuraSlot());

    if(uint8 index = aur->GetSpellProto()->buffIndex)
        m_buffIndexAuraSlots[index] = aur->GetAuraSlot();
    if(aur->GetSpellProto()->RequiredShapeShift)
        m_shapeShiftAuras.insert(aur->GetAuraSlot());
    m_auraLock.HighRelease();

    aur->ApplyModifiers(true);

    // Reaction from enemy AI
	if( !aur->IsPositive() && CanAgroHash( aur->GetSpellProto()->NameHash ) )
		if(caster->IsUnit() && m_Unit->isAlive())
			m_Unit->SetInCombat(castPtr<Unit>(caster));

    if (aur->GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_ON_INVINCIBLE && caster->IsUnit() )
    {
        Unit *uCaster = castPtr<Unit>(caster);
        /*uCaster->RemoveStealth();
        uCaster->RemoveInvisibility();*/
        uCaster->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_ICE_BLOCK, false);
        uCaster->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_DIVINE_SHIELD, false);
        uCaster->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_BLESSING_OF_PROTECTION, false);
    }
}

void AuraInterface::RemoveAura(Aura* aur)
{
    if(aur == NULL)
        return;

    m_auraLock.HighAcquire();
    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if(aur == m_auras[x])
            m_auras[x] = NULL;
    m_auraLock.HighRelease();
    aur->Remove(); // Call remove once.
}

void AuraInterface::RemoveAuraBySlot(uint8 Slot)
{
    m_auraLock.HighAcquire();
    Aura *targetAur = m_auras[Slot];
    m_auras[Slot] = NULL;
    m_auraLock.HighRelease();

    if(targetAur != NULL)
        targetAur->Remove();
}

bool AuraInterface::RemoveAuras(uint32 * SpellIds)
{
    if(!SpellIds || *SpellIds == 0)
        return false;

    bool res = false;
    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            for(uint32 y = 0; SpellIds[y] != 0; ++y)
            {
                if(aur->GetSpellId()==SpellIds[y])
                {
                    RemoveAuraBySlot(x);
                    res = true;
                }
            }
        }
    }
    return res;
}

void AuraInterface::RemoveAuraNoReturn(uint32 spellId)
{
    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if( aur->GetSpellId() == spellId )
            {
                RemoveAuraBySlot(x);
                return;
            }
        }
    }
}

bool AuraInterface::RemovePositiveAura(uint32 spellId)
{
    for(uint8 x = 0; x < m_maxPosAuraSlot; x++)
    {
        if( Aura *aur = m_auras[x])
        {
            if(aur->GetSpellId() == spellId)
            {
                RemoveAuraBySlot(x);
                return true;
            }
        }
    }
    return false;
}

bool AuraInterface::RemoveNegativeAura(uint32 spellId)
{
    for(uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; x++)
    {
        if( Aura *aur = m_auras[x])
        {
            if( aur->GetSpellId() == spellId )
            {
                RemoveAuraBySlot(x);
                return true;
            }
        }
    }
    return false;
}

bool AuraInterface::RemoveAuraByNameHash(uint32 namehash)
{
    return RemoveAuraPosByNameHash(namehash) || RemoveAuraNegByNameHash(namehash);
}

bool AuraInterface::RemoveAuraPosByNameHash(uint32 namehash)
{
    for(uint8 x = 0; x < m_maxPosAuraSlot; x++)
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->NameHash == namehash)
            {
                RemoveAuraBySlot(x);
                return true;
            }
        }
    }
    return false;
}

bool AuraInterface::RemoveAuraNegByNameHash(uint32 namehash)
{
    for(uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; x++)
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->NameHash == namehash)
            {
                RemoveAuraBySlot(x);
                return true;
            }
        }
    }
    return false;
}

void AuraInterface::RemoveAuraBySlotOrRemoveStack(uint8 Slot)
{
    if(Aura *aur = m_auras[Slot])
    {
        if(aur->getStackSize() > 1)
        {
            aur->RemoveStackSize(1);
            return;
        }
        aur->Remove();
        m_auras[Slot] = NULL;
    }
}

bool AuraInterface::RemoveAura(uint32 spellId, WoWGuid guid )
{
    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellId() == spellId && (guid.empty() || aur->GetCasterGUID() == guid))
            {
                RemoveAuraBySlot(x);
                return true;
            }
        }
    }
    return false;
}

void AuraInterface::RemoveAllAuras()
{
    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if(Aura *aur = m_auras[x])
            RemoveAuraBySlot(x);
}

void AuraInterface::RemoveAllExpiringAuras()
{
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->DurationIndex == 0)
                continue;
            if(aur->GetSpellProto()->isDeathPersistentAura())
                continue;
            RemoveAuraBySlot(x);
        }
    }
}

void AuraInterface::RemoveAllNegativeAuras()
{
    for(uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; x++)
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->isDeathPersistentAura())
                continue;
            RemoveAuraBySlot(x);
        }
    }
}

void AuraInterface::RemoveAllNonPassiveAuras()
{
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if(Aura *aur = m_auras[x])
            RemoveAuraBySlot(x);
}

void AuraInterface::RemoveAllAreaAuras(WoWGuid skipguid)
{
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        Aura *aur = m_auras[x];
        if(aur == NULL)
            continue;
        if (aur->IsAreaAura() && aur->GetUnitCaster() && (!aur->GetUnitCaster()
            || (aur->GetUnitCaster()->IsPlayer() && (!skipguid || skipguid != aur->GetCasterGUID()))))
            RemoveAuraBySlot(x);
    }
}

bool AuraInterface::RemoveAllAurasFromGUID(WoWGuid guid)
{
    bool res = false;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetCasterGUID() == guid)
            {
                RemoveAuraBySlot(x);
                res = true;
            }
        }
    }
    return res;
}

//ex:to remove morph spells
void AuraInterface::RemoveAllAurasOfType(uint32 auratype, SpellEntry *toSkip)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if((aur = m_auras[x]) && aur->GetSpellProto()->AppliesAura(auratype) && (toSkip == NULL || aur->GetSpellProto() != toSkip))
            RemoveAuraBySlot(x);
}

bool AuraInterface::RemoveAllPosAurasFromGUID(WoWGuid guid)
{
    bool res = false;
    for(uint8 x = 0; x < m_maxPosAuraSlot; x++)
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetCasterGUID() == guid)
            {
                RemoveAuraBySlot(x);
                res = true;
            }
        }
    }
    return res;
}

bool AuraInterface::RemoveAllNegAurasFromGUID(WoWGuid guid)
{
    bool res = false;
    for(uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; x++)
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetCasterGUID() == guid)
            {
                RemoveAuraBySlot(x);
                res = true;
            }
        }
    }
    return res;
}

bool AuraInterface::RemoveAllAurasByNameHash(uint32 namehash, bool passive)
{
    bool res = false;
    uint8 max = (passive ? m_maxPassiveAuraSlot : m_maxNegAuraSlot);
    for(uint8 x = 0; x < max; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->NameHash == namehash)
            {
                res = true;
                RemoveAuraBySlot(x);
            }
        }
    }
    return res;
}

void AuraInterface::RemoveAllAurasExpiringWithPet()
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if((aur = m_auras[x]) && aur->GetSpellProto()->isSpellExpiringWithPet())
            RemoveAuraBySlot(x);
}

void AuraInterface::RemoveAllAurasByInterruptFlag(uint32 flag)
{
    Aura *aur = NULL; //some spells do not get removed all the time only at specific intervals
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if((aur = m_auras[x]) && aur->GetSpellProto()->AuraInterruptFlags & flag)
            RemoveAuraBySlot(x);
}

void AuraInterface::RemoveAllAurasWithAuraName(uint32 auraName)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if((aur = m_auras[x]) && aur->GetSpellProto()->AppliesAura(auraName))
            RemoveAuraBySlot(x);
}

void AuraInterface::RemoveAllAurasWithSpEffect(uint32 EffectId)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if((aur = m_auras[x]) && aur->GetSpellProto()->HasEffect(EffectId))
            RemoveAuraBySlot(x);
}

bool AuraInterface::RemoveAllPosAurasByNameHash(uint32 namehash)
{
    bool res = false;
    for(uint8 x = 0; x < m_maxPosAuraSlot;x++)
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->NameHash == namehash)
            {
                RemoveAuraBySlot(x);
                res = true;
            }
        }
    }
    return res;
}

bool AuraInterface::RemoveAllNegAurasByNameHash(uint32 namehash)
{
    bool res = false;
    for(uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; x++)
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->NameHash == namehash)
            {
                RemoveAuraBySlot(x);
                res = true;
            }
        }
    }
    return res;
}

bool AuraInterface::RemoveAllAuras(uint32 spellId, WoWGuid guid)
{
    bool res = false;
    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellId() == spellId && (!guid || aur->GetCasterGUID() == guid) )
            {
                RemoveAuraBySlot(x);
                res = true;
            }
        }
    }
    return res;
}

uint32 AuraInterface::GetAuraCountWithFamilyNameAndSkillLine(uint32 spellFamily, uint32 SkillLine)
{
    uint32 count = 0;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if (aur->GetSpellProto()->SpellFamilyName != spellFamily)
                continue;
            if(aur->GetSpellProto()->SpellSkillLine != SkillLine)
                continue;
            count++;
        }
    }
    return count;
}

/* bool Unit::RemoveAllAurasByMechanic (renamed from MechanicImmunityMassDispel)
- Removes all auras on this unit that are of a specific mechanic.
- Useful for things like.. Apply Aura: Immune Mechanic, where existing (de)buffs are *always supposed* to be removed.
- I'm not sure if this goes here under unit.
* Arguments:
    - uint32 MechanicType
        *

* Returns;
    - False if no buffs were dispelled, true if more than 0 were dispelled.
*/
bool AuraInterface::RemoveAllAurasByMechanic( uint32 MechanicType, int32 MaxDispel, bool HostileOnly )
{
    //sLog.outString( "Unit::MechanicImmunityMassDispel called, mechanic: %u" , MechanicType );
    uint32 DispelCount = 0; // If HostileOnly = 1, then we use aura slots 40-86 (hostile). Otherwise, we use 0-86 (all)
    for(uint8 x = ( HostileOnly ? MAX_POSITIVE_AURAS : 0 ); x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(MaxDispel > 0)
            if(DispelCount >= (uint32)MaxDispel)
                return true;

        if(Aura *aur = m_auras[x])
        {
            if( Spell::HasMechanic(aur->GetSpellProto(), MechanicType) ) // Remove all mechanics of type MechanicType (my english goen boom)
            {
                // TODO: Stop moving if fear was removed.
                RemoveAuraBySlot(x);
                DispelCount ++;
            }
            else if( MechanicType == MECHANIC_ENSNARED ) // if got immunity for slow, remove some that are not in the mechanics
            {
                if(aur->GetSpellProto()->AppliesAura(SPELL_AURA_MOD_DECREASE_SPEED) || aur->GetSpellProto()->AppliesAura(SPELL_AURA_MOD_ROOT))
                    RemoveAuraBySlot(x);
            }
        }
    }
    return ( DispelCount != 0 );
}

Aura* AuraInterface::FindAuraBySlot(uint8 auraSlot)
{
    return m_auras[auraSlot];
}

Aura* AuraInterface::FindAura(uint32 spellId, WoWGuid guid)
{
    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellId() == spellId && (guid.empty() || aur->GetCasterGUID() == guid))
            {
                return aur;
            }
        }
    }
    return NULL;
}

Aura* AuraInterface::FindPositiveAuraByNameHash(uint32 namehash)
{
    for(uint8 x = 0; x < m_maxPosAuraSlot; x++)
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->NameHash == namehash)
            {
                return aur;
            }
        }
    }
    return NULL;
}

Aura* AuraInterface::FindNegativeAuraByNameHash(uint32 namehash)
{
    for(uint8 x = MAX_POSITIVE_AURAS; x < m_maxNegAuraSlot; x++)
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->NameHash == namehash)
            {
                return aur;
            }
        }
    }
    return NULL;
}

Aura* AuraInterface::FindActiveAura(uint32 spellId, WoWGuid guid)
{
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellId()==spellId && (guid.empty() || aur->GetCasterGUID() == guid))
            {
                return aur;
            }
        }
    }
    return NULL;
}

Aura* AuraInterface::FindActiveAuraWithNameHash(uint32 namehash, WoWGuid guid)
{
    RWGuard guard(m_auraLock, false);
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->NameHash == namehash && (guid.empty() || aur->GetCasterGUID() == guid))
            {
                return aur;
            }
        }
    }
    return NULL;
}

void AuraInterface::EventDeathAuraRemoval()
{
    m_auraLock.LowAcquire();
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->isDeathPersistentAura())
                continue;

            m_auraLock.LowRelease();
            RemoveAuraBySlot(x);
            m_auraLock.LowAcquire();
        }
    }
    m_auraLock.LowRelease();
}

void AuraInterface::_RecalculateModAmountByType(Modifier *mod)
{   // Recalculation code is stored here to keep outdated code from sticking around where we don't see it
    switch(mod->m_type)
    {
    case SPELL_AURA_MOD_CD_FROM_HASTE:
        mod->m_amount = (((float)mod->m_baseAmount) * castPtr<Player>(m_Unit)->GetFloatValue(PLAYER_FIELD_MOD_HASTE))/-100.f;
        break;
    }
}

void AuraInterface::UpdateModifier(uint8 auraSlot, uint8 index, Modifier *mod, bool apply)
{
    RWGuard guard(m_auraLock, true);
    uint16 mod_index = createModifierIndex(auraSlot, index);
    Loki::AssocVector<uint8, ModifierHolder*>::iterator itr;
    if(apply)
    {
        ModifierHolder *modHolder = NULL;
        if((itr = m_modifierHolders.find(auraSlot)) != m_modifierHolders.end())
            modHolder = itr->second;
        else if(Aura *aur = m_auras[auraSlot])
        {
            modHolder = new ModifierHolder(auraSlot, aur->GetSpellProto());
            m_modifierHolders.insert(std::make_pair(auraSlot, modHolder));
        }

        ASSERT(modHolder);
        if (modHolder->mod[index] != mod)
        {   // Do a quick recalc if we need it
            _RecalculateModAmountByType(mod);
            m_modifiersByModType[mod->m_type].insert(std::make_pair(mod_index, mod));
            modHolder->mod[index] = mod;
        }
    }
    else if((itr = m_modifierHolders.find(auraSlot)) != m_modifierHolders.end())
    {
        m_modifiersByModType[mod->m_type].erase(mod_index);

        ModifierHolder *modHolder = itr->second;
        modHolder->mod[index] = NULL;
        bool auraClear = true;
        for (uint8 i = 0; i < 3; i++)
        {
            if (modHolder->mod[i])
            {
                auraClear = false;
                break;
            }
        }

        if (auraClear)
        {
            m_modifierHolders.erase(auraSlot);
            delete modHolder;
        }
    }

    if(mod->m_type == SPELL_AURA_ADD_FLAT_MODIFIER || mod->m_type == SPELL_AURA_ADD_PCT_MODIFIER || mod->m_type == SPELL_AURA_MOD_CD_FROM_HASTE)
        UpdateSpellGroupModifiers(apply, mod, false);

    m_Unit->OnAuraModChanged(mod->m_type);
}

void AuraInterface::UpdateSpellGroupModifiers(bool apply, Modifier *mod, bool silent)
{
    assert(mod->m_miscValue[0] < SPELL_MODIFIERS);
    uint8 index1 = mod->m_miscValue[0] & 0x7F, index2 = mod->m_type == SPELL_AURA_ADD_FLAT_MODIFIER ? 0 : 1;
    uint16 index = createModifierIndex(index1, index2);
    uint32 count = 0;
    WorldPacket *data = NULL;
    if(m_Unit->IsPlayer() && m_Unit->IsInWorld() && !silent)
    {
        data = new WorldPacket(SMSG_SET_FLAT_SPELL_MODIFIER+index2, 20);
        *data << uint32(1) << count << uint8(index1);
    }
    m_auraLock.HighAcquire();
    Loki::AssocVector<uint8, int32> &groupModMap = m_spellGroupModifiers[index];
    for(uint32 bit = 0, intbit = 0; bit < SPELL_GROUPS; ++bit)
    {
        if(bit && (bit%32 == 0)) ++intbit;
        if(mod->m_spellInfo->EffectSpellClassMask[mod->i][intbit] == 0)
        {   // Jump to the next byteset
            bit += 31;
            continue;
        }

        if( ( ((unsigned int)1) << (bit%32) ) & mod->m_spellInfo->EffectSpellClassMask[mod->i][intbit] )
        {
            if(apply) groupModMap[bit] += mod->m_amount;
            else groupModMap[bit] -= mod->m_amount;
            if(data)
            {
                data->append<uint8>(bit);
                data->append<float>(groupModMap[bit]);
            }
            count++;
        }
    }
    m_auraLock.HighRelease();

    if(data)
    {
        data->put<uint32>(4, count);
        castPtr<Player>(m_Unit)->PushPacket(data);
        delete data;
    }
}

uint32 AuraInterface::get32BitOffsetAndGroup(uint32 value, uint8 &group)
{
    group = uint8(float2int32(floor(float(value)/32.f)));
    return value%32;
}

bool AuraInterface::HasApplicableAurasWithModType(uint32 modType)
{
    bool ret = false;
    m_auraLock.LowAcquire();
    if(!m_modifiersByModType.empty() && m_modifiersByModType.find(modType) != m_modifiersByModType.end())
    {
        for(AuraInterface::modifierMap::iterator itr = m_modifiersByModType[modType].begin(); itr != m_modifiersByModType[modType].end(); ++itr)
        {
            if(sSpellMgr.IsAuraApplicable(m_Unit, itr->second->m_spellInfo))
            {
                ret = true;
                break;
            }
        }
    }
    m_auraLock.LowRelease();
    return ret;
}

void AuraInterface::TraverseModMap(uint32 modType, AuraInterface::ModCallback *callback)
{
    m_auraLock.LowAcquire();
    AuraInterface::modifierMap *modMap = NULL;
    if(m_modifiersByModType.find(modType) != m_modifiersByModType.end() && !(modMap = &m_modifiersByModType[modType])->empty())
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); ++itr)
            (*callback)(itr->second);
    m_auraLock.LowRelease();

    // Tell the callback we've finished processing the mod list
    (*callback).postTraverse(modType);
}

void AuraInterface::SM_FIValue( uint32 modifier, int32* v, uint32* group )
{
    RWGuard guard(m_auraLock, false);
    assert(modifier < SPELL_MODIFIERS);
    uint16 index = createModifierIndex(modifier, 0);
    if(m_spellGroupModifiers.find(index) == m_spellGroupModifiers.end() || m_spellGroupModifiers[index].empty())
        return;

    uint32 flag;
    uint8 groupOffset;
    Loki::AssocVector<uint8, int32> &groupModMap = m_spellGroupModifiers[index];
    for(Loki::AssocVector<uint8, int32>::iterator itr = groupModMap.begin(); itr != groupModMap.end(); itr++)
    {
        flag = (uint32(1)<<get32BitOffsetAndGroup(itr->first, groupOffset));
        if(flag & group[groupOffset]) (*v) += itr->second;
    }
}

void AuraInterface::SM_FFValue( uint32 modifier, float* v, uint32* group )
{
    RWGuard guard(m_auraLock, false);
    assert(modifier < SPELL_MODIFIERS);
    uint16 index = createModifierIndex(modifier, 0);
    if(m_spellGroupModifiers.find(index) == m_spellGroupModifiers.end() || m_spellGroupModifiers[index].empty())
        return;

    uint32 flag;
    uint8 groupOffset;
    Loki::AssocVector<uint8, int32> &groupModMap = m_spellGroupModifiers[index];
    for(Loki::AssocVector<uint8, int32>::iterator itr = groupModMap.begin(); itr != groupModMap.end(); itr++)
    {
        flag = (uint32(1)<<get32BitOffsetAndGroup(itr->first, groupOffset));
        if(flag & group[groupOffset]) (*v) += itr->second;
    }
}

void AuraInterface::SM_PIValue( uint32 modifier, int32* v, uint32* group )
{
    RWGuard guard(m_auraLock, false);
    assert(modifier < SPELL_MODIFIERS);
    uint16 index = createModifierIndex(modifier, 1);
    if(m_spellGroupModifiers.find(index) == m_spellGroupModifiers.end() || m_spellGroupModifiers[index].empty())
        return;

    uint32 flag;
    uint8 groupOffset;
    Loki::AssocVector<uint8, int32> &groupModMap = m_spellGroupModifiers[index];
    for(Loki::AssocVector<uint8, int32>::iterator itr = groupModMap.begin(); itr != groupModMap.end(); itr++)
    {
        flag = (uint32(1)<<get32BitOffsetAndGroup(itr->first, groupOffset));
        if(flag & group[groupOffset]) (*v) += float2int32(floor((float(*v)*float(itr->second))/100.f));
    }
}

void AuraInterface::SM_PFValue( uint32 modifier, float* v, uint32* group )
{
    RWGuard guard(m_auraLock, false);
    assert(modifier < SPELL_MODIFIERS);
    uint16 index = createModifierIndex(modifier, 1);
    if(m_spellGroupModifiers.find(index) == m_spellGroupModifiers.end() || m_spellGroupModifiers[index].empty())
        return;

    uint32 flag;
    uint8 groupOffset;
    Loki::AssocVector<uint8, int32> &groupModMap = m_spellGroupModifiers[index];
    for(Loki::AssocVector<uint8, int32>::iterator itr = groupModMap.begin(); itr != groupModMap.end(); itr++)
    {
        flag = (uint32(1)<<get32BitOffsetAndGroup(itr->first, groupOffset));
        if(flag & group[groupOffset]) (*v) += ((*v)*float(itr->second))/100.f;
    }
}

int32 AuraInterface::getModMapAccumulatedValue(uint32 modType)
{
    int32 retVal = 0;
    m_auraLock.LowAcquire();
    AuraInterface::modifierMap *modMap = NULL;
    if(m_modifiersByModType.find(modType) != m_modifiersByModType.end() && !(modMap = &m_modifiersByModType[modType])->empty())
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); ++itr)
            retVal += itr->second->m_amount;
    m_auraLock.LowRelease();
    return retVal;
}
