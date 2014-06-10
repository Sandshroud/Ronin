/***
 * Demonstrike Core
 */

#include "StdAfx.h"

AuraInterface::AuraInterface()
{

}

AuraInterface::~AuraInterface()
{

}

void AuraInterface::Init(Unit* unit)
{
    m_Unit = unit;
}

void AuraInterface::DeInit()
{
    m_Unit = NULL;
    m_auras.clear();
}

void AuraInterface::RelocateEvents()
{
    //Relocate our aura's (must be done after object is removed from world
    for(uint8 x = 0; x < TOTAL_AURAS; ++x)
    {
        if(m_auras.find(x) != m_auras.end())
            m_auras.at(x)->RelocateEvents();
    }
}

void AuraInterface::SaveAuras(stringstream& ss)
{
    for(uint8 x = 0; x < MAX_AURAS; x++) // Crow: Changed to max auras in r1432, since we skip passive auras.
    {
        if(m_auras.find(x) != m_auras.end())
        {
            Aura* aur = m_auras.at(x);

            // skipped spells due to bugs
            switch(aur->m_spellProto->Id)
            {
            case 642:
            case 1020:              // Divine Shield
            case 11129:             // Combustion
            case 12043:             // Presence of mind
            case 16188:             // Natures Swiftness
            case 17116:             // Natures Swiftness
            case 23333:             // WSG
            case 23335:             // WSG
            case 28682:             // Combustion proc
            case 31665:             // Master of Subtlety (buff)
            case 32724:             // Gold Team
            case 32725:             // Green Team
            case 32727:             // Arena Preparation
            case 32728:             // Arena Preparation
            case 32071:             // Hellfire Superority
            case 32049:             // Hellfire Superority
            case 34936:             // Backlash
            case 35076:             // Blessing of A'dal
            case 35774:             // Gold Team
            case 35775:             // Green Team
            case 44521:             // Preparation?
            case 44683:             // Team A
            case 44684:             // Team B
            case 45438:             // Ice Block
            case 48418:             // Master Shapeshifter Damage (buff)
            case 48420:             // Master Shapeshifter Critical Strike (buff)
            case 48421:             // Master Shapeshifter Spell Damage (buff)
            case 48422:             // Master Shapeshifter Healing (buff)
                continue;
                break;
            }

            bool stop = false;
            for(uint32 i = 0; i < 3; i++)
            {
                if(!stop)
                {
                    if((aur->m_spellProto->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA && aur->GetCasterGUID() != m_Unit->GetGUID()) ||
                        aur->m_spellProto->Effect[i] == SPELL_EFFECT_APPLY_AURA_128 ||
                        aur->m_spellProto->Effect[i] == SPELL_EFFECT_ADD_FARSIGHT)
                    {
                        stop = true;
                        break;
                    }
                }
            }
            if(stop)
                continue;

            // We are going to cast passive spells anyway on login so no need to save auras for them
            if( aur->IsPassive() || aur->m_spellProto->c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET || aur->m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP )
                continue; // To prevent food/drink bug

            ss << aur->GetSpellId() << "," << aur->GetTimeLeft() << ",";
        }
    }
}

uint8 AuraInterface::GetFreeSlot(bool ispositive)
{
    if (ispositive)
    {
        for (uint8 i = 0; i < MAX_POSITIVE_AURAS; i++)
        {
            if(m_auras.find(i) == m_auras.end())
            {
                return i;
                break;
            }
        }
    }
    else
    {
        for (uint8 i = MAX_POSITIVE_AURAS; i < MAX_AURAS; i++)
        {
            if(m_auras.find(i) == m_auras.end())
            {
                return i;
                break;
            }
        }
    }

    return 0xFF;
}

void AuraInterface::OnAuraRemove(Aura* aura, uint8 aura_slot)
{
    map<uint32, Aura*>::iterator itr;
    if(aura_slot > TOTAL_AURAS)
    {
        for(itr = m_auras.begin(); itr != m_auras.end(); itr++)
        {
            if(itr->second == aura)
            {   // Completely unnecessary.
                m_auras.erase(itr);
                break;
            }
        }
    }
    else
    {
        if((itr = m_auras.find(aura_slot)) != m_auras.end())
        {
            if(itr->second == aura)
                m_auras.erase(itr);
        }
    }
}

bool AuraInterface::IsDazed()
{
    for(uint8 x = 0; x < MAX_AURAS; ++x)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->MechanicsType == MECHANIC_ENSNARED)
                return true;

            for(uint32 y = 0; y < 3; y++)
            {
                if(m_auras.at(x)->GetSpellProto()->EffectMechanic[y]==MECHANIC_ENSNARED)
                    return true;
            }
        }
    }

    return false;
}

bool AuraInterface::IsPoisoned()
{
    for(uint8 x = 0; x < TOTAL_AURAS; ++x)
    {
        if(m_auras.find(x) != m_auras.end())
            if( m_auras.at(x)->GetSpellProto()->poison_type )
                return true;
    }

    return false;
}

void AuraInterface::UpdateDuelAuras()
{
    for( uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; ++x )
        if( m_auras.find(x) != m_auras.end())
            if(m_auras.at(x)->WasCastInDuel())
                RemoveAuraBySlot(x);
}

void AuraInterface::BuildAllAuraUpdates()
{
    for( uint8 x = 0; x < MAX_AURAS; ++x )
        if( m_auras.find(x) != m_auras.end() )
            m_auras.at(x)->BuildAuraUpdate();
}

bool AuraInterface::BuildAuraUpdateAllPacket(WorldPacket* data)
{
    if(!m_auras.size())
        return false;

    bool res = false;
    Aura* aur = NULL;
    for (uint8 i = 0; i < MAX_AURAS; i++)
    {
        if(m_auras.find(i) != m_auras.end())
        {
            res = true;
            aur = m_auras.at(i);
            aur->BuildAuraUpdatePacket(data);
        }
    }
    return res;
}

void AuraInterface::SpellStealAuras(Unit* caster, int32 MaxSteals)
{
    Aura* aur = NULL;
    int32 spells_to_steal = MaxSteals > 1 ? MaxSteals : 1;
    for(uint8 x = 0; x < MAX_POSITIVE_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            aur = m_auras.at(x);
            if(aur != NULL && aur->GetSpellId() != 15007 && !aur->IsPassive() && aur->IsPositive()) //Nothing can dispel resurrection sickness
            {
                if(aur->GetSpellProto()->DispelType == DISPEL_MAGIC && aur->GetDuration() > 0)
                {
                    WorldPacket data(SMSG_SPELLDISPELLOG, 16);
                    data << caster->GetNewGUID();
                    data << m_Unit->GetNewGUID();
                    data << uint32(1);
                    data << aur->GetSpellId();
                    caster->SendMessageToSet(&data,true);

                    Aura* aura = new Aura(aur->GetSpellProto(), (aur->GetDuration()>120000) ? 120000 : aur->GetDuration(), caster, caster);
                    aura->stackSize = aur->stackSize;

                    // copy the mods across
                    for( uint32 m = 0; m < aur->GetModCount(); ++m )
                    {
                        Modifier *mod = aur->GetMod(m);
                        aura->AddMod(mod->m_type, mod->m_baseAmount, mod->m_miscValue, mod->i);
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

void AuraInterface::UpdateDeadlyPoisons(uint32 eatcount)
{
    uint32 doses = GetPoisonDosesCount( POISON_TYPE_DEADLY );
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; ++x)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->m_spellProto->poison_type == POISON_TYPE_DEADLY )
            {
                if (eatcount >= doses)
                    m_auras.at(x)->Remove();
                else
                    m_auras.at(x)->ModStackSize(-int32(eatcount));
                break;
            }
        }
    }
}

void AuraInterface::UpdateAuraStateAuras(uint32 oldflag)
{
    for(uint8 i = 0; i < TOTAL_AURAS; i++)
    {
        if(m_auras.find(i) != m_auras.end())
        {
            if( !m_auras.at(i)->m_applied) // try to apply
                m_auras.at(i)->ApplyModifiers(true);

            if( m_auras.at(i)->m_applied) // try to remove, if we lack the aurastate
                m_auras.at(i)->RemoveIfNecessary();
        }
    }
}

uint32 AuraInterface::GetPoisonDosesCount( uint32 poison_type )
{
    uint32 doses = 0;
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; ++x)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->m_spellProto->poison_type == poison_type )
            {
                doses += m_auras.at(x)->stackSize;
            }
        }
    }
    return doses;
}

void AuraInterface::UpdateShapeShiftAuras(uint32 oldSS, uint32 newSS)
{
    // TODO: Passive auras should not be removed, but deactivated.
    for( uint8 x = 0; x < TOTAL_AURAS; x++ )
    {
        if( m_auras.find(x) != m_auras.end() )
        {
            uint32 reqss = m_auras.at(x)->GetSpellProto()->RequiredShapeShift;
            if( reqss != 0 && m_auras.at(x)->IsPositive() )
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

            if( m_Unit->getClass() == DRUID )
            {
                for (uint8 y = 0; y < 3; y++ )
                {
                    switch( m_auras.at(x)->GetSpellProto()->EffectApplyAuraName[y])
                    {
                    case SPELL_AURA_MOD_ROOT: //Root
                    case SPELL_AURA_MOD_DECREASE_SPEED: //Movement speed
                    case SPELL_AURA_MOD_CONFUSE:  //Confuse (polymorph)
                        {
                            RemoveAuraBySlot(x);
                        }break;
                    default:
                        break;
                    }

                    if( m_auras.find(x) == m_auras.end() )
                        break;
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
        for( uint8 x = 0; x < MAX_POSITIVE_AURAS; x++ )
        {
            if( m_auras.find(x) != m_auras.end() )
            {
                if(m_auras.at(x)->IsPositive())
                {
                    p = m_auras.at(x)->GetSpellProto();
                    if( Spell::HasMechanic(p, Mechanic) )
                    {
                        m_auras.at(x)->AttemptDispel( caster );
                    }
                }
            }
        }
    }
    else
    {
        for( uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; x++ )
        {
            if( m_auras.find(x) != m_auras.end() )
            {
                if(!m_auras.at(x)->IsPositive())
                {
                    p = m_auras.at(x)->GetSpellProto();
                    if( Spell::HasMechanic(p, Mechanic) )
                    {
                        m_auras.at(x)->AttemptDispel( caster );
                    }
                }
            }
        }
    }
}

void AuraInterface::MassDispel(Unit* caster, uint32 index, SpellEntry* Dispelling, uint32 MaxDispel, uint8 start, uint8 end)
{
    ASSERT(start < TOTAL_AURAS && end < TOTAL_AURAS);
    WorldPacket data(SMSG_SPELLDISPELLOG, 16);

    Aura* aur = NULL;
    for(uint8 x = start; x < end; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            aur = m_auras.at(x);

            //Nothing can dispel resurrection sickness;
            if(aur != NULL && !aur->IsPassive() && !(aur->GetSpellProto()->Attributes & ATTRIBUTES_IGNORE_INVULNERABILITY))
            {
                int32 resistchance = 0;
                Unit* caster = aur->GetUnitCaster();
                if( caster )
                    SM_FIValue(caster->SM[SMT_RESIST_DISPEL][0], &resistchance, aur->GetSpellProto()->SpellGroupType);

                if( !Rand(resistchance) )
                {
                    if(Dispelling->DispelType == DISPEL_ALL)
                    {
                        m_Unit->HandleProc( PROC_ON_DISPEL_AURA_VICTIM, NULL, caster, Dispelling, aur->GetSpellId() );
                        data.clear();
                        data << caster->GetNewGUID();
                        data << m_Unit->GetNewGUID();
                        data << (uint32)1;//probably dispel type
                        data << aur->GetSpellId();
                        caster->SendMessageToSet(&data,true);
                        aur->AttemptDispel( caster );
                        if(!--MaxDispel)
                            return;
                    }
                    else if(aur->GetSpellProto()->DispelType == Dispelling->EffectMiscValue[index])
                    {
                        if( (aur->GetSpellProto()->NameHash != SPELL_HASH_ICE_BARRIER &&
                            aur->GetSpellProto()->NameHash != SPELL_HASH_DIVINE_SHIELD)
                            || Dispelling->NameHash == SPELL_HASH_MASS_DISPEL )
                        {
                            m_Unit->HandleProc( PROC_ON_DISPEL_AURA_VICTIM, NULL, caster, Dispelling, aur->GetSpellId() );
                            data.clear();
                            data << caster->GetNewGUID();
                            data << m_Unit->GetNewGUID();
                            data << (uint32)1;
                            data << aur->GetSpellId();
                            caster->SendMessageToSet(&data,true);
                            aur->AttemptDispel( caster );
                            if(!--MaxDispel)
                                return;
                        }
                    }
                }
                else if( !--MaxDispel )
                    return;
            }
        }
    }
}

void AuraInterface::RemoveAllAurasWithDispelType(uint32 DispelType)
{
    for( uint8 x = 0; x < MAX_AURAS; x++ )
    {
        if(m_auras.find(x) != m_auras.end())
            if(m_auras.at(x)->m_spellProto->DispelType == DispelType)
                RemoveAuraBySlot(x);
    }
}

void AuraInterface::RemoveAllAurasWithAttributes(uint32 attributeFlag)
{
    for(uint8 x=0;x<MAX_AURAS;x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->m_spellProto && (m_auras.at(x)->m_spellProto->Attributes & attributeFlag))
            {
                RemoveAuraBySlot(x);
            }
        }
    }
}

void AuraInterface::RemoveAllAurasOfSchool(uint32 School, bool Positive, bool Immune)
{
    for(uint8 x = 0; x < MAX_AURAS; ++x)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(!Immune && (m_auras.at(x)->GetSpellProto()->Attributes & ATTRIBUTES_IGNORE_INVULNERABILITY))
                continue;
            if(m_auras.at(x)->GetSpellProto()->School == School && (!m_auras.at(x)->IsPositive() || Positive))
                RemoveAuraBySlot(x);
        }
    }
}

void AuraInterface::RemoveAllAurasByInterruptFlagButSkip(uint32 flag, uint32 skip)
{
    Aura* a = NULL;
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        a = NULL;
        if(m_auras.find(x) == m_auras.end())
            continue;

        a = m_auras.at(x);
        if( a->GetDuration() > 0 && (int32)(a->GetTimeLeft()+500) > a->GetDuration() )
            continue;//pretty new aura, don't remove

        //some spells do not get removed all the time only at specific intervals
        if((a->m_spellProto->AuraInterruptFlags & flag) && (a->m_spellProto->Id != skip) && a->m_spellProto->proc_interval==0)
        {
            //the black sheeps of sociaty
            if(a->m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_CAST_SPELL)
            {
                switch(a->GetSpellProto()->Id)
                {
                // priest - holy conc
                case 34754:
                    {
                        if( m_Unit->GetCurrentSpell() !=NULL &&
                                !(m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_FLASH_HEAL ||
                                m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_BINDING_HEAL ||
                                m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_GREATER_HEAL))
                            continue;
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && spi->NameHash != SPELL_HASH_FLASH_HEAL && spi->NameHash != SPELL_HASH_BINDING_HEAL && spi->NameHash != SPELL_HASH_GREATER_HEAL)
                            continue;
                    }break;
                //Arcane Potency
                case 57529:
                case 57531:
                    {
                        if( m_Unit->GetCurrentSpell() != NULL && !(m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_PRESENCE_OF_MIND
                            || m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_CLEARCASTING ))
                            continue;

                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL || !(spi->c_is_flags & SPELL_FLAG_IS_DAMAGING) )
                            continue;

                    }break;
                //paladin - Art of war
                case 53489:
                case 59578:
                    {
                        if( m_Unit->GetCurrentSpell() != NULL && m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_FLASH_OF_LIGHT )
                            continue;
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && spi->NameHash != SPELL_HASH_FLASH_OF_LIGHT )
                            continue;
                    }break;
                //paladin - Infusion of light
                case 53672:
                case 54149:
                    {
                        if( m_Unit->GetCurrentSpell() != NULL && !(m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_FLASH_OF_LIGHT
                            || m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_HOLY_LIGHT))
                            continue;
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && spi->NameHash != SPELL_HASH_FLASH_OF_LIGHT && spi->NameHash != SPELL_HASH_HOLY_LIGHT)
                            continue;
                    }break;
                //Mage - Firestarter
                case 54741:
                    {
                        if( m_Unit->GetCurrentSpell() != NULL && m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_FLAMESTRIKE )
                            continue;
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && spi->NameHash != SPELL_HASH_FLAMESTRIKE )
                            continue;
                    }break;
                case 34936:     // Backlash
                    {
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && spi->NameHash != SPELL_HASH_SHADOW_BOLT && spi->NameHash != SPELL_HASH_INCINERATE )
                            continue;
                    }break;

                case 17941: //Shadow Trance
                    {
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && spi->NameHash != SPELL_HASH_SHADOW_BOLT )
                            continue;
                    }break;
                // Glyph of Revenge Proc
                case 58363:
                    {
                        if( m_Unit->GetCurrentSpell() != NULL && m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_HEROIC_STRIKE )
                            continue;
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && spi->NameHash != SPELL_HASH_HEROIC_STRIKE )
                            continue;
                    }break;
                case 18708: //Fel Domination
                    {
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && spi->NameHash != SPELL_HASH_SUMMON_IMP &&
                            spi->NameHash != SPELL_HASH_SUMMON_VOIDWALKER &&
                            spi->NameHash != SPELL_HASH_SUMMON_SUCCUBUS &&
                            spi->NameHash != SPELL_HASH_SUMMON_FELHUNTER &&
                            spi->NameHash != SPELL_HASH_SUMMON_FELGUARD )
                            continue;
                    }break;
                case 46916: // Bloodsurge
                    {
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && spi->NameHash != SPELL_HASH_SLAM )
                            continue;
                    }break;
                case 14177: // Cold Blood
                    {
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && !(spi->c_is_flags & SPELL_FLAG_IS_DAMAGING) && spi->NameHash != SPELL_HASH_MUTILATE )
                            continue;
                    }break;
                case 31834: // Light's Grace
                    {
                        if( m_Unit->GetCurrentSpell() != NULL && m_Unit->GetCurrentSpell()->GetSpellProto()->NameHash == SPELL_HASH_HOLY_LIGHT )
                            continue;

                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && spi->NameHash != SPELL_HASH_HOLY_LIGHT )
                            continue;
                    }break;
                // Shadowstep
                case 44373:
                case 36563:
                    {
                        SpellEntry *spi = dbcSpell.LookupEntry( skip );
                        if( spi != NULL && !(spi->c_is_flags & SPELL_FLAG_IS_DAMAGING) )
                            continue;
                    }break;
                }
            }
            RemoveAuraBySlot(x);
        }
    }
}

uint32 AuraInterface::GetSpellIdFromAuraSlot(uint32 slot)
{
    if(m_auras.find(slot) != m_auras.end())
        return m_auras.at(slot)->GetSpellId();
    return 0;
}

AuraCheckResponse AuraInterface::AuraCheck(SpellEntry *info)
{
    AuraCheckResponse resp;

    // no error for now
    resp.Error = AURA_CHECK_RESULT_NONE;
    resp.Misc  = 0;

    // look for spells with same namehash
    bool stronger = false;
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if( m_auras.find(x) == m_auras.end() )
            continue;

        for( uint32 loop = 0; loop < 3; loop++ )
        {
            if( m_auras.at(x)->GetSpellProto()->Effect[loop] == info->Effect[loop] && info->Effect[loop] > 0 )
            {
                if( info->EffectBasePoints[loop] < 0 )
                {
                    if( info->EffectBasePoints[loop] <= m_auras.at(x)->GetSpellProto()->EffectBasePoints[loop] )
                    {
                        stronger = true;
                        break;
                    }
                }
                else if( info->EffectBasePoints[loop] > 0 )
                {
                    if( info->EffectBasePoints[loop] >= m_auras.at(x)->GetSpellProto()->EffectBasePoints[loop] )
                    {
                        stronger = true;
                        break;
                    }
                }
            }
        }

        if( stronger )
        {
            resp.Error = AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT;
            break;
        }
    }

    resp.Error = AURA_CHECK_RESULT_LOWER_BUFF_PRESENT;
    return resp; // return it back to our caller
}

uint32 AuraInterface::GetAuraSpellIDWithNameHash(uint32 name_hash)
{
    for(uint8 x = 0; x < MAX_AURAS; ++x)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->NameHash == name_hash)
            {
                return m_auras.at(x)->m_spellProto->Id;
            }
        }
    }

    return 0;
}

bool AuraInterface::HasAura(uint32 spellid)
{
    for(uint8 x = 0; x < TOTAL_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellId() == spellid)
            {
                return true;
            }
        }
    }

    return false;
}

bool AuraInterface::HasAuraVisual(uint32 visualid)
{
    for(uint8 x = 0; x < TOTAL_AURAS; ++x)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->SpellVisual[0] == visualid || m_auras.at(x)->GetSpellProto()->SpellVisual[1] == visualid)
            {
                return true;
            }
        }
    }

    return false;
}

bool AuraInterface::HasActiveAura(uint32 spellid)
{
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellId() == spellid)
            {
                return true;
            }
        }
    }
    return false;
}

bool AuraInterface::HasNegativeAura(uint32 spell_id)
{
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; ++x)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->Id == spell_id)
            {
                return true;
            }
        }
    }

    return false;
}

bool AuraInterface::HasAuraWithMechanic(uint32 mechanic)
{
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetMechanic() == mechanic)
            {
                return true;
            }
        }
    }
    return false;
}

bool AuraInterface::HasActiveAura(uint32 spellid, uint64 guid)
{
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellId() == spellid && (!guid || m_auras.at(x)->GetCasterGUID() == guid))
            {
                return true;
            }
        }
    }

    return false;
}

bool AuraInterface::HasPosAuraWithMechanic(uint32 mechanic)
{
    for(uint8 x = 0; x < MAX_POSITIVE_AURAS; x++)
    {
        if( m_auras.find(x) != m_auras.end() )
        {
            if( m_auras.at(x)->GetMechanic() == mechanic )
            {
                return true;
            }
        }
    }
    return false;
}

bool AuraInterface::HasNegAuraWithMechanic(uint32 mechanic)
{
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; x++)
    {
        if( m_auras.find(x) != m_auras.end() )
        {
            if( m_auras.at(x)->GetMechanic() == mechanic )
            {
                return true;
            }
        }
    }
    return false;
}

bool AuraInterface::HasNegativeAuraWithNameHash(uint32 name_hash)
{
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; ++x)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->NameHash == name_hash)
            {
                return true;
            }
        }
    }

    return false;
}

bool AuraInterface::HasCombatStatusAffectingAuras(uint64 checkGuid)
{
    for(uint8 i = MAX_POSITIVE_AURAS; i < MAX_AURAS; i++)
    {
        if(m_auras.find(i) != m_auras.end())
        {
            if(checkGuid == m_auras.at(i)->GetCasterGUID() && m_auras.at(i)->IsCombatStateAffecting())
                return true;
        }
    }
    return false;
}

bool AuraInterface::HasAurasOfNameHashWithCaster(uint32 namehash, uint64 casterguid)
{
    for(uint8 i = 0; i < MAX_AURAS; i++)
    {
        if(m_auras.find(i) != m_auras.end())
        {
            if(casterguid)
            {
                if(casterguid == m_auras.at(i)->GetCasterGUID() && m_auras.at(i)->GetSpellProto()->NameHash == namehash)
                    return true;
            }
            else
            {
                if(m_auras.at(i)->GetSpellProto()->NameHash == namehash)
                    return true;
            }
        }
    }
    return false;
}

bool AuraInterface::HasAurasOfBuffType(uint32 buff_type, const uint64 &guid, uint32 skip)
{
    uint64 sguid = (buff_type == SPELL_TYPE_BLESSING || buff_type == SPELL_TYPE_WARRIOR_SHOUT) ? guid : 0;
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->buffType & buff_type && m_auras.at(x)->GetSpellId() != skip)
            {
                if(!sguid || (sguid && m_auras.at(x)->GetCasterGUID() == sguid))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool AuraInterface::OverrideSimilarAuras(Unit *caster, Aura *aur)
{
    uint32 maxStack = aur->GetSpellProto()->maxstack;
    if( m_Unit->IsPlayer() && TO_PLAYER(m_Unit)->stack_cheat )
        maxStack = 255;

    std::set<uint8> m_aurasToRemove;
    SpellEntry *info = aur->GetSpellProto();
    for( uint8 x = 0; x < MAX_AURAS; x++ )
    {
        if(m_auras.find(x) == m_auras.end())
            continue;
        Aura* curAura = m_auras.at(x);
        if(curAura == NULL || curAura->m_deleted)
            continue;

        //if this is a proc spell then it should not remove it's mother : test with combustion later
        if( curAura->GetSpellProto()->Id != aur->GetSpellId() && aur->pSpellId != curAura->GetSpellProto()->Id)
        {
            if(info->buffType && curAura->GetSpellProto()->buffType && (info->buffType & curAura->GetSpellProto()->buffType) != 0)
            {
                bool needsRemoval = false;
                if( curAura->GetSpellProto()->buffType & SPELL_TYPE_BLESSING )
                {
                    // Shouldn't happen, aura should be rejected at cast
                    if(curAura->GetSpellProto()->NameHash == info->NameHash && info->RankNumber < curAura->GetSpellProto()->RankNumber)
                        return false;
                }
                else if( curAura->GetSpellProto()->buffType & SPELL_TYPE_AURA )
                {
                    if( curAura->GetUnitCaster() != aur->GetUnitCaster() && curAura->GetSpellProto()->NameHash != info->NameHash )
                        continue;
                }
                m_aurasToRemove.insert(x);
            }
            else if( info->poison_type && curAura->GetSpellProto()->poison_type == info->poison_type )
            {
                if( curAura->GetSpellProto()->RankNumber < info->RankNumber || maxStack == 0)
                {
                    RemoveAuraBySlot(x);
                    continue;
                }
                else if( curAura->GetSpellProto()->RankNumber > info->RankNumber )
                {
                    RemoveAuraBySlot(x);
                    break;
                }
            }
            else if( curAura->GetSpellProto()->NameHash == info->NameHash )
            {
                if( curAura->GetUnitCaster() == aur->GetUnitCaster() )
                    m_aurasToRemove.insert(x);
                else if( curAura->GetSpellProto()->Unique )
                {
                    if( curAura->GetSpellProto()->RankNumber <= info->RankNumber )
                    {
                        m_aurasToRemove.insert(x);
                        continue;
                    }
                    return false;
                }
            }
        }
        else if(aur->GetSpellId() == curAura->GetSpellId() && info->always_apply)
            m_aurasToRemove.insert(x);
        else if( aur->IsPositive() && curAura->GetSpellId() == aur->GetSpellId() && curAura->GetCasterGUID() == aur->GetCasterGUID())
        {
            if(info->Unique)
                continue;

            // target already has this aura. Update duration, time left, procCharges
            curAura->SetDuration(aur->GetDuration());
            curAura->SetTimeLeft(aur->GetDuration());
            curAura->procCharges = curAura->GetMaxProcCharges(caster);
            curAura->UpdateModifiers();
            curAura->ModStackSize(1);   // increment stack size
            return false;
        }
    }

    for(std::set<uint8>::iterator itr = m_aurasToRemove.begin(); itr != m_aurasToRemove.end(); itr++)
        RemoveAuraBySlot(*itr);
    m_aurasToRemove.clear();
    return true;
}

void AuraInterface::AddAura(Aura* aur)
{
    Unit* pCaster = NULLUNIT;
    if(aur->GetUnitTarget() != NULL)
        pCaster = aur->GetUnitCaster();
    else if( aur->GetCasterGUID() == m_Unit->GetGUID() )
        pCaster = m_Unit;
    else if( m_Unit->GetMapMgr() && aur->GetCasterGUID())
        pCaster = m_Unit->GetMapMgr()->GetUnit( aur->GetCasterGUID());
    if(pCaster == NULL)
        return;
    if(!aur->IsPassive() && !OverrideSimilarAuras(pCaster, aur))
    {
        sEventMgr.RemoveEvents(aur);
        RemoveAura(aur);
        return;
    }

    ////////////////////////////////////////////////////////
    if( aur->m_auraSlot != 255 && aur->m_auraSlot < TOTAL_AURAS)
    {
        if( m_auras.find(aur->m_auraSlot) != m_auras.end() )
            RemoveAuraBySlot(aur->m_auraSlot);
    }

    aur->m_auraSlot = 255;

    Unit* target = aur->GetUnitTarget();
    if(target == NULL)
        return; // Should never happen.

    aur->SetAuraFlags(AFLAG_EFF_INDEX_0 | AFLAG_EFF_INDEX_1 | AFLAG_EFF_INDEX_2 | AFLAG_NOT_GUID | (aur->IsPositive() ? (AFLAG_POSITIVE) : (AFLAG_NEGATIVE)));
    if(aur->GetDuration())
        aur->SetAuraFlag(AFLAG_HAS_DURATION);
    if(!aur->IsPassive())
        aur->SetAuraFlag(AFLAG_EFF_AMOUNT_SEND);

    aur->SetAuraLevel(aur->GetUnitCaster() != NULL ? aur->GetUnitCaster()->getLevel() : MAXIMUM_ATTAINABLE_LEVEL);

    uint8 x;
    if(!aur->IsPassive())
    {
        aur->AddAuraVisual();
        if(aur->m_auraSlot == 255)
        {
            //add to invisible slot
            for(x = MAX_AURAS; x < TOTAL_AURAS; x++)
            {
                if(m_auras.find(x) == m_auras.end())
                {
                    m_auras.insert(make_pair(x, aur));
                    aur->m_auraSlot = x;
                    break;
                }
            }

            if(aur->m_auraSlot == 255)
            {
                sLog.Debug("Unit","AddAura error in active aura. removing. SpellId: %u", aur->GetSpellProto()->Id);
                RemoveAura(aur);
                return;
            }
        }
        else
        {
            m_auras.insert(make_pair(aur->m_auraSlot, aur));
        }
    }
    else
    {
        if(aur->m_spellProto->AttributesEx & 1024)
            aur->AddAuraVisual();

        for(x = MAX_AURAS; x < TOTAL_AURAS; x++)
        {
            if(m_auras.find(x) == m_auras.end())
            {
                m_auras.insert(make_pair(x, aur));
                aur->m_auraSlot = x;
                break;
            }
        }

        if(aur->m_auraSlot == 255)
        {
            sLog.Debug("Unit","AddAura error in passive aura. removing. SpellId: %u", aur->GetSpellProto()->Id);
            RemoveAura(aur);
            return;
        }
    }

    if(aur->GetSpellId() == 15007) //Resurrection sickness
    {
        aur->SetNegative(100); //we're negative
        aur->SetDuration(target->getLevel() > 19 ? 600000 : 60000);
    }

    aur->ApplyModifiers(true);

    // We add 500ms here to allow for the last tick in DoT spells. This is a dirty hack, but at least it doesn't crash like my other method.
    // - Burlex, Crow: Changed to 400ms
    if(aur->GetDuration() > 0)
    {
        uint32 addTime = 400;
        for(uint32 spx = 0; spx < 3; spx++)
        {
            if( aur->GetSpellProto()->EffectApplyAuraName[spx] == SPELL_AURA_MOD_STUN ||
                aur->GetSpellProto()->EffectApplyAuraName[spx] == SPELL_AURA_MOD_FEAR ||
                aur->GetSpellProto()->EffectApplyAuraName[spx] == SPELL_AURA_MOD_ROOT ||
                aur->GetSpellProto()->EffectApplyAuraName[spx] == SPELL_AURA_MOD_CHARM )
                addTime = 50;
        }

        sEventMgr.AddAuraEvent(m_Unit, &Unit::RemoveAuraBySlot, uint8(aur->m_auraSlot), aur->GetDuration() + addTime, 1,
            EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT, aur->GetSpellId());
    }

    aur->RelocateEvents();

    // Send log to client
    if (target != NULL)
    {
        //send the aura log
        WorldPacket data(SMSG_AURACASTLOG, 28);

        data << aur->GetCasterGUID();
        data << aur->GetTargetGUID();
        data << aur->m_spellProto->Id;
        data << uint64(0);

        target->SendMessageToSet(&data, true);
    }

    m_Unit->m_chargeSpellsInUse = true;
    if( aur->procCharges > 0 && !(aur->GetSpellProto()->procflags2 & PROC_REMOVEONUSE))
        m_Unit->m_chargeSpells.push_back(aur);
    m_Unit->m_chargeSpellsInUse = false;

    aur->m_added = true;

    // Reaction from enemy AI
    if( !aur->IsPositive() && CanAgroHash( aur->GetSpellProto()->NameHash ) )
    {
        if(pCaster != NULL && m_Unit->isAlive())
        {
            pCaster->CombatStatus.OnDamageDealt(TO_UNIT(this), 1);

            if(m_Unit->IsCreature())
                m_Unit->GetAIInterface()->AttackReaction(pCaster, 1, aur->GetSpellId());
        }
    }

    if (aur->GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_ON_INVINCIBLE)
    {
        if( pCaster != NULL )
        {
            pCaster->RemoveStealth();
            pCaster->RemoveInvisibility();
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

    for(uint8 x = 0; x < TOTAL_AURAS; x++)
        if(m_auras.find(x) != m_auras.end())
            if(m_auras.at(x) == aur)
                m_auras.erase(x); // Null it every time we find it.
    aur->Remove(); // Call remove once.
}

void AuraInterface::RemoveAuraBySlot(uint8 Slot)
{
    if(m_auras.find(Slot) != m_auras.end())
    {
        m_auras.at(Slot)->Remove();
        m_auras.erase(Slot);
    }
}

bool AuraInterface::RemoveAuras(uint32 * SpellIds)
{
    if(!SpellIds || *SpellIds == 0)
        return false;

    bool res = false;
    for(uint8 x = 0; x < TOTAL_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            for(uint32 y = 0; SpellIds[y] != 0; ++y)
            {
                if(m_auras.at(x)->GetSpellId()==SpellIds[y])
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
{   //this can be speed up, if we know passive \pos neg
    for(uint8 x = 0; x < TOTAL_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if( m_auras.at(x)->GetSpellId() == spellId )
            {
                RemoveAuraBySlot(x);
                return;
            }
        }
    }
    return;
}

bool AuraInterface::RemovePositiveAura(uint32 spellId)
{
    for(uint8 x = 0; x < MAX_POSITIVE_AURAS; x++)
    {
        if( m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellId() == spellId)
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
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; x++)
    {
        if( m_auras.find(x) != m_auras.end())
        {
            if( m_auras.at(x)->GetSpellId() == spellId )
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
    for(uint8 x = 0; x < MAX_POSITIVE_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->NameHash == namehash)
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
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->NameHash == namehash)
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
    if(m_auras.find(Slot) != m_auras.end())
    {
        if(m_auras.at(Slot)->stackSize > 1)
        {
            m_auras.at(Slot)->RemoveStackSize(1);
            return;
        }
        m_auras.at(Slot)->Remove();
        m_auras.erase(Slot);
    }
}

bool AuraInterface::RemoveAura(uint32 spellId, uint64 guid )
{
    for(uint8 x = 0; x < TOTAL_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellId() == spellId && (!guid || m_auras.at(x)->GetCasterGUID() == guid))
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
    for(uint8 x = 0; x < TOTAL_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
            RemoveAuraBySlot(x);
    }
}

void AuraInterface::RemoveAllExpiringAuras()
{
    for(uint8 x = 0; x < TOTAL_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->DurationIndex && !(m_auras.at(x)->GetSpellProto()->Flags4 & FLAGS4_DEATH_PERSISTENT))
            {
                RemoveAuraBySlot(x);
            }
        }
    }
}

void AuraInterface::RemoveAllNegativeAuras()
{
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(!(m_auras.at(x)->GetSpellProto()->Flags4 & FLAGS4_DEATH_PERSISTENT))
            {
                RemoveAuraBySlot(x);
            }
        }
    }
}

void AuraInterface::RemoveAllNonPassiveAuras()
{
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
            RemoveAuraBySlot(x);
    }
}

void AuraInterface::RemoveAllAreaAuras(uint64 skipguid)
{
    for (uint8 i = 0; i < MAX_POSITIVE_AURAS; ++i)
    {
        if(m_auras.find(i) != m_auras.end())
        {
            if (m_auras.at(i)->m_areaAura && m_auras.at(i)->GetUnitCaster() && (!m_auras.at(i)->GetUnitCaster()
                || (m_auras.at(i)->GetUnitCaster()->IsPlayer() && (!skipguid || skipguid != m_auras.at(i)->GetCasterGUID()))))
                RemoveAuraBySlot(i);
        }
    }
}

bool AuraInterface::RemoveAllAurasFromGUID(uint64 guid)
{
    bool res = false;
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetCasterGUID() == guid)
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
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            SpellEntry *proto = NULL;
            proto = m_auras.at(x)->GetSpellProto();
            if(proto != NULL && proto->EffectApplyAuraName[0] == auratype || proto->EffectApplyAuraName[1] == auratype || proto->EffectApplyAuraName[2] == auratype)
                RemoveAura(m_auras.at(x)->GetSpellId());//remove all morph auras containig to this spell (like wolf motph also gives speed)
        }
    }
}

bool AuraInterface::RemoveAllPosAurasFromGUID(uint64 guid)
{
    bool res = false;
    for(uint8 x = 0; x < MAX_POSITIVE_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetCasterGUID() == guid)
            {
                RemoveAuraBySlot(x);
                res = true;
            }
        }
    }
    return res;
}

bool AuraInterface::RemoveAllNegAurasFromGUID(uint64 guid)
{
    bool res = false;
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetCasterGUID() == guid)
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
    uint8 max = (passive ? TOTAL_AURAS : MAX_AURAS);
    for(uint8 x = 0; x < max; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->NameHash == namehash)
            {
                res = true;
                RemoveAuraBySlot(x);
            }
        }
    }
    return res;
}

void AuraInterface::RemoveAllAurasByCIsFlag(uint32 c_is_flag)
{
    for(uint8 x = 0; x < TOTAL_AURAS; x++)
        if(m_auras.find(x) != m_auras.end())
            if(m_auras.at(x)->GetSpellProto()->c_is_flags & c_is_flag)
                RemoveAuraBySlot(x);
}

void AuraInterface::RemoveAllAurasByInterruptFlag(uint32 flag)
{
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) == m_auras.end())
            continue;
        //some spells do not get removed all the time only at specific intervals
        if((m_auras.at(x)->m_spellProto->AuraInterruptFlags & flag) && !(m_auras.at(x)->m_spellProto->procflags2 & PROC_REMOVEONUSE))
            RemoveAuraBySlot(x);
    }
}

void AuraInterface::RemoveAllAurasWithAuraName(uint32 auraName)
{
    for(uint8 i = 0; i < TOTAL_AURAS; i++)
    {
        if(m_auras.find(i) != m_auras.end())
        {
            for(uint32 x = 0; x < 3; x++)
            {
                if( m_auras.at(i)->m_spellProto->EffectApplyAuraName[x] == auraName )
                {
                    RemoveAuraBySlot(i);
                    break;
                }
            }
        }
    }
}

void AuraInterface::RemoveAllAurasWithSpEffect(uint32 EffectId)
{
    for(uint8 i = 0; i < TOTAL_AURAS; i++)
    {
        if(m_auras.find(i) != m_auras.end())
        {
            for(uint32 x = 0; x < 3; x++)
            {
                if( m_auras.at(i)->m_spellProto->Effect[x] == EffectId )
                {
                    if(m_auras.at(i)->GetCasterGUID() == m_Unit->GetGUID())
                        m_auras.at(i)->RemoveAA();
                    else
                        RemoveAuraBySlot(i);
                    break;
                }
            }
        }
    }
}

bool AuraInterface::RemoveAllPosAurasByNameHash(uint32 namehash)
{
    bool res = false;
    for(uint8 x = 0; x < MAX_POSITIVE_AURAS;x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->NameHash == namehash)
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
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->NameHash == namehash)
            {
                RemoveAuraBySlot(x);
                res = true;
            }
        }
    }
    return res;
}

bool AuraInterface::RemoveAllAuras(uint32 spellId, uint64 guid)
{
    bool res = false;
    for(uint8 x = 0; x < TOTAL_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellId() == spellId && (!guid || m_auras.at(x)->GetCasterGUID() == guid) )
            {
                RemoveAuraBySlot(x);
                res = true;
            }
        }
    }
    return res;
}

void AuraInterface::RemoveAllAurasByBuffIndexType(uint32 buff_index_type, const uint64 &guid)
{
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->buffIndexType == buff_index_type)
            {
                if(!guid || (guid && m_auras.at(x)->GetCasterGUID() == guid))
                    RemoveAuraBySlot(x);
            }
        }
    }
}

void AuraInterface::RemoveAllAurasByBuffType(uint32 buff_type, const uint64 &guid, uint32 skip)
{
    uint64 sguid = buff_type >= SPELL_TYPE_BLESSING ? guid : 0;

    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if((m_auras.at(x)->GetSpellProto()->buffType & buff_type) && m_auras.at(x)->GetSpellId() != skip)
            {
                if(!sguid || m_auras.at(x)->GetCasterGUID() == sguid)
                    RemoveAuraBySlot(x);
            }
        }
    }
}

uint32 AuraInterface::GetAuraCountWithFamilyNameAndSkillLine(uint32 spellFamily, uint32 SkillLine)
{
    uint32 count = 0;
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if (m_auras.at(x)->m_spellProto->SpellFamilyName == spellFamily)
            {
                SkillLineSpell *sk = objmgr.GetSpellSkill(m_auras.at(x)->GetSpellId());
                if(sk && sk->skilline == SkillLine)
                {
                    count++;
                }
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
    uint32 DispelCount = 0;
    for(uint8 x = ( HostileOnly ? MAX_POSITIVE_AURAS : 0 ) ; x < MAX_AURAS ; x++ ) // If HostileOnly = 1, then we use aura slots 40-86 (hostile). Otherwise, we use 0-86 (all)
    {
        if(MaxDispel > 0)
            if(DispelCount >= (uint32)MaxDispel)
                return true;

        if(m_auras.find(x) != m_auras.end())
        {
            if( Spell::HasMechanic(m_auras.at(x)->GetSpellProto(), MechanicType) ) // Remove all mechanics of type MechanicType (my english goen boom)
            {
                // TODO: Stop moving if fear was removed.
                RemoveAuraBySlot(x);
                DispelCount ++;
            }
            else if( MechanicType == MECHANIC_ENSNARED ) // if got immunity for slow, remove some that are not in the mechanics
            {
                for( int i = 0; i < 3; i++ )
                {
                    // SNARE + ROOT
                    if( m_auras.at(x)->GetSpellProto()->EffectApplyAuraName[i] == SPELL_AURA_MOD_DECREASE_SPEED || m_auras.at(x)->GetSpellProto()->EffectApplyAuraName[i] == SPELL_AURA_MOD_ROOT )
                    {
                        RemoveAuraBySlot(x);
                        break;
                    }
                }
            }
        }
    }
    return ( DispelCount != 0 );
}

Aura* AuraInterface::FindAuraBySlot(uint8 auraSlot)
{
    if(m_auras.find(auraSlot) == m_auras.end())
        return NULL;
    return m_auras.at(auraSlot);
}

Aura* AuraInterface::FindAura(uint32 spellId, uint64 guid)
{
    for(uint8 x = 0; x < TOTAL_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellId() == spellId && (!guid || m_auras.at(x)->GetCasterGUID() == guid))
            {
                return m_auras.at(x);
            }
        }
    }
    return NULLAURA;
}

Aura* AuraInterface::FindPositiveAuraByNameHash(uint32 namehash)
{
    for(uint8 x = 0; x < MAX_POSITIVE_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->NameHash == namehash)
            {
                return m_auras.at(x);
            }
        }
    }
    return NULLAURA;
}

Aura* AuraInterface::FindNegativeAuraByNameHash(uint32 namehash)
{
    for(uint8 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->NameHash == namehash)
            {
                return m_auras.at(x);
            }
        }
    }
    return NULLAURA;
}

Aura* AuraInterface::FindActiveAura(uint32 spellId, uint64 guid)
{
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellId()==spellId && (!guid || m_auras.at(x)->GetCasterGUID() == guid))
            {
                return m_auras.at(x);
            }
        }
    }
    return NULLAURA;
}

Aura* AuraInterface::FindActiveAuraWithNameHash(uint32 namehash, uint64 guid)
{
    for(uint8 x = 0; x < MAX_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->NameHash == namehash && (!guid || m_auras.at(x)->GetCasterGUID() == guid))
            {
                return m_auras.at(x);
            }
        }
    }
    return NULLAURA;
}

void AuraInterface::EventDeathAuraRemoval()
{
    for(uint8 x = 0; x < TOTAL_AURAS; x++)
    {
        if(m_auras.find(x) != m_auras.end())
        {
            if(m_auras.at(x)->GetSpellProto()->Flags4 & FLAGS4_DEATH_PERSISTENT)
                continue;

            RemoveAuraBySlot(x);
        }
    }
}

bool AuraInterface::SetAuraDuration(uint32 spellId,Unit* caster,int32 duration)
{
    Aura* aur = FindAura(spellId, caster->GetGUID());
    if(aur == NULL)
        return false;

    aur->SetDuration(duration);
    aur->SetTimeLeft(duration);
    return true;
}

bool AuraInterface::SetAuraDuration(uint32 spellId, int32 duration)
{
    Aura* aur = FindAura(spellId, 0);
    if(aur == NULL)
        return false;

    aur->SetDuration(duration);
    aur->SetTimeLeft(duration);
    return true;
}
