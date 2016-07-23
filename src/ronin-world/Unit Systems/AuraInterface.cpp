/***
 * Demonstrike Core
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
    *ss << "(" << lowGuid
        << ", " << auraSlot
        << ", " << aur->GetSpellId()
        << ", " << uint32(aur->GetAuraFlags())
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
        if(aur == NULL || !aur->GetSpellProto()->HasEffect(SPELL_EFFECT_APPLY_AURA))
            continue;
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

void AuraInterface::BuildAllAuraUpdates()
{
    for( uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
        if( Aura *aur = m_auras[x] )
            aur->BuildAuraUpdate();
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

                    Aura* aura = new Aura(aur->GetSpellProto(), caster, caster);
                    aura->AddStackSize(aur->getStackSize()-1);

                    // copy the mods across
                    for( uint32 m = 0; m < aur->GetModCount(); ++m )
                    {
                        Modifier *mod = aur->GetMod(m);
                        aura->AddMod(mod->m_type, mod->m_baseAmount, mod->m_miscValue[0], mod->m_miscValue[1], mod->i);
                    }

                    caster->AddAura(aura);
                    RemoveAuraBySlot(x);
                    if( --spells_to_steal <= 0 )
                        break; //exit loop now
                }
            }
        }
    }
}

void AuraInterface::UpdateShapeShiftAuras(uint32 oldSS, uint32 newSS)
{
    for( uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false) )
    {
        if( Aura *aur = m_auras[x] )
        {
            SpellEntry *sp = aur->GetSpellProto();
            uint32 reqss = sp->RequiredShapeShift;
            if( reqss != 0 && aur->IsPositive() )
            {
                if( oldSS > 0 && oldSS != 28)
                {
                    if( ( ((uint32)1 << (oldSS-1)) & reqss ) && // we were in the form that required it
                        !( ((uint32)1 << (newSS-1) & reqss) ) )         // new form doesnt have the right form
                    {
                        RemoveAuraBySlot(x);
                        continue;
                    }
                }
            }

            if( m_Unit->getClass() == DRUID && ( sp->AppliesAura(SPELL_AURA_MOD_ROOT)
                || sp->AppliesAura(SPELL_AURA_MOD_DECREASE_SPEED) || sp->AppliesAura(SPELL_AURA_MOD_CONFUSE) ) )
            {
                RemoveAuraBySlot(x);
                continue;
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
    ASSERT(start < MAX_AURAS && end < MAX_AURAS);
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

bool AuraInterface::OverrideSimilarAuras(Unit *caster, Aura *aur)
{
    std::set<uint8> m_aurasToRemove;
    uint32 maxStack = aur->GetSpellProto()->maxstack;
    if( maxStack && m_Unit->IsPlayer() && castPtr<Player>(m_Unit)->stack_cheat )
        maxStack = 0xFF;

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
    for( uint8 x = 0; x < m_maxNegAuraSlot; (x+1 == m_maxPosAuraSlot ? x = MAX_POSITIVE_AURAS : x++) )
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
                        curAura->UpdateModifiers();
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

void AuraInterface::AddAura(Aura* aur, uint8 slot)
{
    Unit* pCaster = NULL;
    if(aur->GetUnitTarget() != NULL)
        pCaster = aur->GetUnitCaster();
    else if( m_Unit->GetGUID() == aur->GetCasterGUID() )
        pCaster = m_Unit;
    else if( m_Unit->GetMapInstance() && aur->GetCasterGUID())
        pCaster = m_Unit->GetMapInstance()->GetUnit( aur->GetCasterGUID());
    if(pCaster == NULL)
        return;

    if(!aur->IsPassive() && !OverrideSimilarAuras(pCaster, aur))
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

    Unit* target = aur->GetUnitTarget();
    if(target == NULL)
        return; // Should never happen.

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

    m_auras[aur->GetAuraSlot()] = aur;
    if(aur->GetAuraSlot() < MAX_POSITIVE_AURAS)
        m_maxPosAuraSlot = std::max<uint8>(m_maxPosAuraSlot, 1+aur->GetAuraSlot());
    else if(aur->GetAuraSlot() < MAX_AURAS)
        m_maxNegAuraSlot = std::max<uint8>(m_maxNegAuraSlot, 1+aur->GetAuraSlot());
    else if(aur->GetAuraSlot() < TOTAL_AURAS)
        m_maxPassiveAuraSlot = std::max<uint8>(m_maxPassiveAuraSlot, 1+aur->GetAuraSlot());

    if(uint8 index = aur->GetSpellProto()->buffIndex)
        m_buffIndexAuraSlots[index] = aur->GetAuraSlot();
    aur->ApplyModifiers(true);

    // Reaction from enemy AI
	if( !aur->IsPositive() && CanAgroHash( aur->GetSpellProto()->NameHash ) )
		if(pCaster != NULL && m_Unit->isAlive())
			m_Unit->SetInCombat(pCaster);

    if (aur->GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_ON_INVINCIBLE)
    {
        if( pCaster != NULL )
        {
            /*pCaster->RemoveStealth();
            pCaster->RemoveInvisibility();*/
            pCaster->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_ICE_BLOCK, false);
            pCaster->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_DIVINE_SHIELD, false);
            pCaster->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_BLESSING_OF_PROTECTION, false);
        }
    }
}

void AuraInterface::RemoveAura(Aura* aur)
{
    if(aur == NULL)
        return;

    for(uint8 x = 0; x < m_maxPassiveAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if(aur == m_auras[x])
            m_auras[x] = NULL;
    aur->Remove(); // Call remove once.
}

void AuraInterface::RemoveAuraBySlot(uint8 Slot)
{
    if(m_auras[Slot] != NULL)
    {
        m_auras[Slot]->Remove();
        m_auras[Slot] = NULL;
    }
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
void AuraInterface::RemoveAllAurasOfType(uint32 auratype)
{
    Aura *aur = NULL;
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
        if((aur = m_auras[x]) && aur->GetSpellProto()->AppliesAura(auratype))
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
            if (aur->GetSpellProto()->SpellFamilyName == spellFamily)
            {
                SkillLineAbilityEntry *sk = objmgr.GetSpellSkill(aur->GetSpellId());
                if(sk && sk->skilline == SkillLine)
                    count++;
            }
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
    for(uint8 x = 0; x < m_maxNegAuraSlot; INC_INDEXORBLOCK_MACRO(x, false))
    {
        if(Aura *aur = m_auras[x])
        {
            if(aur->GetSpellProto()->isDeathPersistentAura())
                continue;

            RemoveAuraBySlot(x);
        }
    }
}

void AuraInterface::UpdateModifier(uint8 auraSlot, uint8 index, Modifier *mod, bool apply)
{
    m_Unit->OnAuraModChanged(mod->m_type);

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
        if(modHolder == NULL || modHolder->mod[index] == mod)
            return;

        m_modifiersByModType[mod->m_type].insert(std::make_pair(mod_index, mod));
        modHolder->mod[index] = mod;
    }
    else if((itr = m_modifierHolders.find(auraSlot)) != m_modifierHolders.end())
    {
        m_modifiersByModType[mod->m_type].erase(mod_index);

        ModifierHolder *modHolder = itr->second;
        modHolder->mod[index] = NULL;
        for(uint8 i=0;i<3;i++)
            if(modHolder->mod[i])
                return;
        m_modifierHolders.erase(auraSlot);
        delete modHolder;
    }

    if(mod->m_type == SPELL_AURA_ADD_FLAT_MODIFIER || mod->m_type == SPELL_AURA_ADD_PCT_MODIFIER)
        UpdateSpellGroupModifiers(apply, mod);
}

void AuraInterface::UpdateSpellGroupModifiers(bool apply, Modifier *mod)
{
    assert(mod->m_miscValue[0] < SPELL_MODIFIERS);
    uint8 index1 = mod->m_miscValue[0] & 0x7F, index2 = mod->m_type == SPELL_AURA_ADD_PCT_MODIFIER ? 1 : 0;
    uint16 index = createModifierIndex(index1, index2);
    Loki::AssocVector<uint8, int32> groupModMap = m_spellGroupModifiers[index];

    uint32 count = 0;
    WorldPacket *data = NULL;
    if(m_Unit->IsPlayer())
    {
        data = new WorldPacket(SMSG_SET_FLAT_SPELL_MODIFIER+index2, 20);
        *data << uint32(1) << count << uint8(index1);
    }
    for(uint32 bit = 0, intbit = 0; bit < SPELL_GROUPS; ++bit, ++intbit)
    {
        if(bit && (bit%32 == 0)) ++intbit;
        if( ( 1 << bit%32 ) & mod->m_spellInfo->EffectSpellClassMask[mod->i][intbit] )
        {
            if(apply) groupModMap[bit] += mod->m_amount;
            else groupModMap[bit] -= mod->m_amount;
            if(data) *data << uint8(bit) << groupModMap[bit];
            count++;
        }
    }

    if(data)
    {
        data->put<uint32>(4, count);
        castPtr<Player>(m_Unit)->SendPacket(data);
        delete data;
    }
}

uint32 AuraInterface::get32BitOffsetAndGroup(uint32 value, uint8 &group)
{
    group = uint8(float2int32(floor(float(value)/32.f)));
    return value%32;
}

void AuraInterface::SM_FIValue( uint32 modifier, int32* v, uint32* group )
{
    assert(modifier < SPELL_MODIFIERS);
    uint16 index = createModifierIndex(modifier, 0);
    Loki::AssocVector<uint8, int32> groupModMap = m_spellGroupModifiers[index];
    if(groupModMap.size() == 0)
        return;

    uint32 flag;
    uint8 groupOffset;
    for(Loki::AssocVector<uint8, int32>::iterator itr = groupModMap.begin(); itr != groupModMap.end(); itr++)
    {
        flag = (uint32(1)<<get32BitOffsetAndGroup(itr->second, groupOffset));
        if(flag & group[groupOffset]) (*v) += itr->second;
    }
}

void AuraInterface::SM_FFValue( uint32 modifier, float* v, uint32* group )
{
    assert(modifier < SPELL_MODIFIERS);
    uint16 index = createModifierIndex(modifier, 0);
    Loki::AssocVector<uint8, int32> groupModMap = m_spellGroupModifiers[index];
    if(groupModMap.size() == 0)
        return;

    uint32 flag;
    uint8 groupOffset;
    for(Loki::AssocVector<uint8, int32>::iterator itr = groupModMap.begin(); itr != groupModMap.end(); itr++)
    {
        flag = (uint32(1)<<get32BitOffsetAndGroup(itr->second, groupOffset));
        if(flag & group[groupOffset]) (*v) += itr->second;
    }
}

void AuraInterface::SM_PIValue( uint32 modifier, int32* v, uint32* group )
{
    assert(modifier < SPELL_MODIFIERS);
    uint16 index = createModifierIndex(modifier, 1);
    Loki::AssocVector<uint8, int32> groupModMap = m_spellGroupModifiers[index];
    if(groupModMap.size() == 0)
        return;

    uint32 flag;
    uint8 groupOffset;
    for(Loki::AssocVector<uint8, int32>::iterator itr = groupModMap.begin(); itr != groupModMap.end(); itr++)
    {
        flag = (uint32(1)<<get32BitOffsetAndGroup(itr->second, groupOffset));
        if(flag & group[groupOffset]) (*v) += float2int32(floor((float(*v)*float(itr->second))/100.f));
    }
}

void AuraInterface::SM_PFValue( uint32 modifier, float* v, uint32* group )
{
    assert(modifier < SPELL_MODIFIERS);
    uint16 index = createModifierIndex(modifier, 1);
    Loki::AssocVector<uint8, int32> groupModMap = m_spellGroupModifiers[index];
    if(groupModMap.size() == 0)
        return;

    uint32 flag;
    uint8 groupOffset;
    for(Loki::AssocVector<uint8, int32>::iterator itr = groupModMap.begin(); itr != groupModMap.end(); itr++)
    {
        flag = (uint32(1)<<get32BitOffsetAndGroup(itr->second, groupOffset));
        if(flag & group[groupOffset]) (*v) += ((*v)*float(itr->second))/100.f;
    }
}
