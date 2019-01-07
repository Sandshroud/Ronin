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

// thank god for macros

#include "StdAfx.h"

std::map<uint8, SpellEffectClass::pSpellEffect> SpellEffectClass::m_spellEffectMap;

SpellEffectClass::SpellEffectClass(Unit* caster, SpellEntry *info, uint8 castNumber, WoWGuid itemGuid) : BaseSpell(caster, info, castNumber, itemGuid)
{

}

SpellEffectClass::~SpellEffectClass()
{
}

void SpellEffectClass::Destruct()
{
    while(!m_tempAuras.empty())
    {
        Aura *aur = m_tempAuras.begin()->second;
        m_tempAuras.erase(m_tempAuras.begin());
        delete aur;
    }
    m_tempAuras.clear();

    BaseSpell::Destruct();
}

int32 SpellEffectClass::CalculateEffect(uint32 i, WorldObject* target)
{
    int32 value = m_spellInfo->CalculateSpellPoints(i, GetEffectiveCasterLevel(), m_spellComboPoints);

    float fValue = value;
    float spell_mods[2] = { 0.f, 100.f };
    _unitCaster->SM_FFValue(SMT_MISC_EFFECT, &spell_mods[0], m_spellInfo->SpellGroupType);
    _unitCaster->SM_PFValue(SMT_MISC_EFFECT, &spell_mods[1], m_spellInfo->SpellGroupType);

    if( i == 0 )
    {
        _unitCaster->SM_FFValue(SMT_FIRST_EFFECT_BONUS, &spell_mods[0], m_spellInfo->SpellGroupType);
        _unitCaster->SM_PFValue(SMT_FIRST_EFFECT_BONUS, &spell_mods[1], m_spellInfo->SpellGroupType);
    }
    else if( i == 1 )
    {
        _unitCaster->SM_FFValue(SMT_SECOND_EFFECT_BONUS, &spell_mods[0], m_spellInfo->SpellGroupType);
        _unitCaster->SM_PFValue(SMT_SECOND_EFFECT_BONUS, &spell_mods[1], m_spellInfo->SpellGroupType);
    }

    if( ( i == 2 ) || ( i == 1 && m_spellInfo->Effect[2] == 0 ) || ( i == 0 && m_spellInfo->Effect[1] == 0 && m_spellInfo->Effect[2] == 0 ) )
    {
        _unitCaster->SM_FFValue(SMT_LAST_EFFECT_BONUS, &spell_mods[0], m_spellInfo->SpellGroupType);
        _unitCaster->SM_PFValue(SMT_LAST_EFFECT_BONUS, &spell_mods[1], m_spellInfo->SpellGroupType);
    }

    fValue *= spell_mods[1];
    fValue /= 100.f;
    fValue += spell_mods[0];
    value = float2int32(fValue);
    return value;
}

void SpellEffectClass::HandleEffects(uint32 i, SpellTarget *spTarget, WorldObject *target)
{
    uint32 effect = m_spellInfo->Effect[i];
    if(SpellEffectClass::m_spellEffectMap.find(effect) != SpellEffectClass::m_spellEffectMap.end())
        (*this.*SpellEffectClass::m_spellEffectMap.at(effect))(i, target, spTarget->effectAmount[i], !spTarget->moddedAmount[i]);
    else sLog.Error("Spell", "Unknown effect %u spellid %u", effect, m_spellInfo->Id);
}

void SpellEffectClass::HandleDelayedEffects(Unit *unitTarget, SpellTarget *spTarget)
{
    uint8 effIndex = 0xFF;
    if(spTarget->accumAmount && unitTarget->isAlive() && (m_spellInfo->HasEffect(SPELL_EFFECT_SCHOOL_DAMAGE, spTarget->EffectMask, &effIndex) || m_spellInfo->HasEffect(SPELL_EFFECT_ENVIRONMENTAL_DAMAGE, spTarget->EffectMask, &effIndex)
        || m_spellInfo->HasEffect(SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL, spTarget->EffectMask, &effIndex) || m_spellInfo->HasEffect(SPELL_EFFECT_WEAPON_PERCENT_DAMAGE, spTarget->EffectMask, &effIndex)
        || m_spellInfo->HasEffect(SPELL_EFFECT_WEAPON_DAMAGE, spTarget->EffectMask, &effIndex) || m_spellInfo->HasEffect(SPELL_EFFECT_DUMMYMELEE, spTarget->EffectMask, &effIndex)))
    {
        if(_unitCaster->IsPlayer() && unitTarget->IsPlayer() && _unitCaster != unitTarget)
        {
            if( unitTarget->IsPvPFlagged() )
                castPtr<Player>(_unitCaster)->SetPvPFlag();
            if( unitTarget->IsFFAPvPFlagged() )
                castPtr<Player>(_unitCaster)->SetFFAPvPFlag();
        }

        if(m_spellInfo->speed > 0 || m_spellInfo->spellType == NON_WEAPON)
            _unitCaster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, spTarget->accumAmount, spTarget->resistMod, false, false);
        else _unitCaster->Strike(unitTarget, m_spellInfo->spellType, m_spellInfo, effIndex, spTarget->accumAmount, false, true);

        if(m_spellInfo->HasEffect(SPELL_EFFECT_ENVIRONMENTAL_DAMAGE, spTarget->EffectMask))
        {
            WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, 13);
            data << unitTarget->GetGUID();
            data << uint8(DAMAGE_FIRE);
            data << uint32(spTarget->accumAmount);
            unitTarget->SendMessageToSet( &data, true );
        }
    }

    // Do an alive check then use our heal function
    if(unitTarget->isAlive() && m_spellInfo->HasEffect(SPELL_EFFECT_HEAL, spTarget->EffectMask))
        Heal(unitTarget, spTarget->EffectMask, spTarget->accumAmount);

    if(spTarget->AuraAddResult != AURA_APPL_NOT_RUN)
    {
        switch(spTarget->AuraAddResult)
        {
        case AURA_APPL_SUCCESS:
            {
                Aura *Aur = NULL;
                if((Aur = spTarget->aura) && !(unitTarget->isDead() && !m_spellInfo->isDeathPersistentAura()))
                {
                    // Clear out our pointer
                    spTarget->aura = NULL;

                    // Trigger application data update
                    Aur->UpdatePreApplication();
                    unitTarget->AddAura(Aur);
                }
            }break;
        case AURA_APPL_REFRESH:
            if(!(unitTarget->isDead() && !m_spellInfo->isDeathPersistentAura()))
                unitTarget->RefreshAura(m_spellInfo, m_casterGuid);
            break;
        case AURA_APPL_STACKED:
            if(!(unitTarget->isDead() && !m_spellInfo->isDeathPersistentAura()))
                unitTarget->AddAuraStack(m_spellInfo, m_casterGuid);
            break;
        }
    }
}

void SpellEffectClass::Heal(Unit *target, uint8 effIndex, int32 amount)
{
    if( target == NULL || !target->isAlive() )
        return;

    //self healing shouldn't flag himself
    if( _unitCaster->IsPlayer() && _unitCaster->GetGUID() != target->GetGUID() )
    {
        // Healing a flagged target will flag you.
        if( target->IsPvPFlagged() )
        {
            /*if( !u_caster->IsPvPFlagged() )
                u_caster->PvPToggle();
            else u_caster->SetPvPFlag();*/
        }
    }

    //Make it critical
    bool critical = false;
    if(!m_spellInfo->isUncrittableSpell())
    {
        float critChance = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster)->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE) : 5.f;
        if( m_spellInfo->SpellGroupType )
        {
            _unitCaster->SM_FFValue(SMT_CRITICAL, &critChance, m_spellInfo->SpellGroupType);
            _unitCaster->SM_PFValue(SMT_CRITICAL, &critChance, m_spellInfo->SpellGroupType);
        }
        critical = critChance > 0.f ? Rand(std::min<float>(critChance, 95.f)) : false;
    }

    uint32 overheal = 0;
    uint32 curHealth = target->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 maxHealth = target->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if((curHealth + amount) >= maxHealth)
    {
        target->SetUInt32Value(UNIT_FIELD_HEALTH, maxHealth);
        overheal = curHealth + amount - maxHealth;
    } else target->ModUnsigned32Value(UNIT_FIELD_HEALTH, amount);

    SendHealSpellOnPlayer(_unitCaster, target, amount, critical, overheal, m_spellInfo->Id);
}

void SpellEffectClass::DetermineSkillUp(Player *target, uint32 skillid,uint32 targetlevel, uint32 multiplicator)
{
    if(target->GetSkillUpChance(skillid)<0.01f)
        return;//to preven getting higher skill than max

    int32 diff = abs(int(target->getSkillLineVal(skillid,false)/5 - targetlevel));
    float chance = ( diff <=5  ? 95.0f : diff <=10 ? 66.0f : diff <=15 ? 33.0f : 0.0f );
    if( Rand(int32(chance * sWorld.getRate(RATE_SKILLCHANCE) * (multiplicator?multiplicator:1))))
        target->ModSkillLineAmount(skillid, float2int32(1.0f * sWorld.getRate(RATE_SKILLRATE)), false);
}

void SpellEffectClass::DetermineSkillUp(Player *target, uint32 skillid)
{
    /*float chance = 0.0f;
    SkillLineAbilityEntry* skill = objmgr.GetSpellSkill(m_spellInfo->Id);
    if( skill != NULL && target->HasSkillLine( skill->skilline ) )
    {
        uint32 amt = target->getSkillLineVal( skill->skilline, false );
        uint32 max = target->getSkillLineMax( skill->skilline );
        if( amt >= max )
            return;
        if( amt >= skill->RankMax ) //grey
            chance = 0.0f;
        else if( ( amt >= ( ( ( skill->RankMax - skill->RankMin) / 2 ) + skill->RankMin ) ) ) //green
            chance = 33.0f;
        else if( amt >= skill->RankMin ) //yellow
            chance = 66.0f;
        else //orange
            chance=100.0f;
    }

    if(Rand(chance*sWorld.getRate(RATE_SKILLCHANCE)))
        target->ModSkillLineAmount(skillid, float2int32( 1.0f * sWorld.getRate(RATE_SKILLRATE)), false);*/
}

void SpellEffectClass::InitializeSpellEffectClass()
{
    m_spellEffectMap[SPELL_EFFECT_NULL]                         = &SpellEffectClass::SpellEffectNULL;
    m_spellEffectMap[SPELL_EFFECT_INSTANT_KILL]                 = &SpellEffectClass::SpellEffectInstantKill; // 1
    m_spellEffectMap[SPELL_EFFECT_SCHOOL_DAMAGE]                = &SpellEffectClass::SpellEffectSchoolDMG; // 2
    m_spellEffectMap[SPELL_EFFECT_DUMMY]                        = &SpellEffectClass::SpellEffectDummy; // 3
    m_spellEffectMap[SPELL_EFFECT_TELEPORT_UNITS]               = &SpellEffectClass::SpellEffectTeleportUnits; // 5
    m_spellEffectMap[SPELL_EFFECT_APPLY_AURA]                   = &SpellEffectClass::SpellEffectDelayed; // 6
    m_spellEffectMap[SPELL_EFFECT_ENVIRONMENTAL_DAMAGE]         = &SpellEffectClass::SpellEffectEnvironmentalDamage; // 7
    m_spellEffectMap[SPELL_EFFECT_POWER_DRAIN]                  = &SpellEffectClass::SpellEffectPowerDrain; // 8
    m_spellEffectMap[SPELL_EFFECT_HEALTH_LEECH]                 = &SpellEffectClass::SpellEffectHealthLeech; // 9
    m_spellEffectMap[SPELL_EFFECT_HEAL]                         = &SpellEffectClass::SpellEffectHeal; // 10
    m_spellEffectMap[SPELL_EFFECT_BIND]                         = &SpellEffectClass::SpellEffectBind; // 11
    m_spellEffectMap[SPELL_EFFECT_PORTAL]                       = &SpellEffectClass::SpellEffectTeleportUnits; // 12
    m_spellEffectMap[SPELL_EFFECT_QUEST_COMPLETE]               = &SpellEffectClass::SpellEffectQuestComplete; // 16
    m_spellEffectMap[SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL]       = &SpellEffectClass::SpellEffectWeaponDamageNoSchool; // 17
    m_spellEffectMap[SPELL_EFFECT_RESURRECT]                    = &SpellEffectClass::SpellEffectResurrect; // 18
    m_spellEffectMap[SPELL_EFFECT_ADD_EXTRA_ATTACKS]            = &SpellEffectClass::SpellEffectAddExtraAttacks; // 19
    m_spellEffectMap[SPELL_EFFECT_CREATE_ITEM]                  = &SpellEffectClass::SpellEffectCreateItem; // 24
    m_spellEffectMap[SPELL_EFFECT_WEAPON]                       = &SpellEffectClass::SpellEffectSkill; // 25
    m_spellEffectMap[SPELL_EFFECT_PERSISTENT_AREA_AURA]         = &SpellEffectClass::SpellEffectPersistentAA; // 27
    m_spellEffectMap[SPELL_EFFECT_SUMMON]                       = &SpellEffectClass::SpellEffectSummon; // 28
    m_spellEffectMap[SPELL_EFFECT_LEAP]                         = &SpellEffectClass::SpellEffectLeap; // 29
    m_spellEffectMap[SPELL_EFFECT_ENERGIZE]                     = &SpellEffectClass::SpellEffectEnergize; // 30
    m_spellEffectMap[SPELL_EFFECT_WEAPON_PERCENT_DAMAGE]        = &SpellEffectClass::SpellEffectWeaponDmgPerc; // 31
    m_spellEffectMap[SPELL_EFFECT_TRIGGER_MISSILE]              = &SpellEffectClass::SpellEffectTriggerMissile; // 32
    m_spellEffectMap[SPELL_EFFECT_OPEN_LOCK]                    = &SpellEffectClass::SpellEffectOpenLock; // 33
    m_spellEffectMap[SPELL_EFFECT_TRANSFORM_ITEM]               = &SpellEffectClass::SpellEffectTranformItem; // 34
    m_spellEffectMap[SPELL_EFFECT_APPLY_AREA_AURA]              = &SpellEffectClass::SpellEffectDelayed; // 35
    m_spellEffectMap[SPELL_EFFECT_LEARN_SPELL]                  = &SpellEffectClass::SpellEffectLearnSpell; // 36
    m_spellEffectMap[SPELL_EFFECT_DISPEL]                       = &SpellEffectClass::SpellEffectDispel; // 38
    m_spellEffectMap[SPELL_EFFECT_LANGUAGE]                     = &SpellEffectClass::SpellEffectSkill; // 39
    m_spellEffectMap[SPELL_EFFECT_DUAL_WIELD]                   = &SpellEffectClass::SpellEffectSkill; // 40
    m_spellEffectMap[SPELL_EFFECT_JUMP_TO_TARGET]               = &SpellEffectClass::SpellEffectJump; // 41
    m_spellEffectMap[SPELL_EFFECT_JUMP_TO_DESTIONATION]         = &SpellEffectClass::SpellEffectJump; // 42
    m_spellEffectMap[SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER]   = &SpellEffectClass::SpellEffectTeleportToCaster; // 43
    m_spellEffectMap[SPELL_EFFECT_SKILL_STEP]                   = &SpellEffectClass::SpellEffectSkillStep; // 44
    m_spellEffectMap[SPELL_EFFECT_ADD_HONOR]                    = &SpellEffectClass::SpellEffectAddHonor; // 45
    m_spellEffectMap[SPELL_EFFECT_SPAWN]                        = &SpellEffectClass::SpellEffectSpawn; // 46
    m_spellEffectMap[SPELL_EFFECT_DETECT]                       = &SpellEffectClass::SpellEffectDetect; // 49
    m_spellEffectMap[SPELL_EFFECT_SUMMON_OBJECT]                = &SpellEffectClass::SpellEffectSummonObject; // 50
    m_spellEffectMap[SPELL_EFFECT_ENCHANT_ITEM]                 = &SpellEffectClass::SpellEffectEnchantItem; // 53
    m_spellEffectMap[SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY]       = &SpellEffectClass::SpellEffectEnchantItemTemporary; // 54
    m_spellEffectMap[SPELL_EFFECT_TAMECREATURE]                 = &SpellEffectClass::SpellEffectTameCreature; // 55
    m_spellEffectMap[SPELL_EFFECT_SUMMON_PET]                   = &SpellEffectClass::SpellEffectSummonPet; // 56
    m_spellEffectMap[SPELL_EFFECT_LEARN_PET_SPELL]              = &SpellEffectClass::SpellEffectLearnPetSpell; // 57
    m_spellEffectMap[SPELL_EFFECT_WEAPON_DAMAGE]                = &SpellEffectClass::SpellEffectWeaponDamage; // 58
    m_spellEffectMap[SPELL_EFFECT_OPEN_LOCK_ITEM]               = &SpellEffectClass::SpellEffectOpenLockItem; // 59
    m_spellEffectMap[SPELL_EFFECT_PROFICIENCY]                  = &SpellEffectClass::SpellEffectProficiency; // 60
    m_spellEffectMap[SPELL_EFFECT_SEND_EVENT]                   = &SpellEffectClass::SpellEffectSendEvent; // 61
    m_spellEffectMap[SPELL_EFFECT_POWER_BURN]                   = &SpellEffectClass::SpellEffectPowerBurn; // 62
    m_spellEffectMap[SPELL_EFFECT_THREAT]                       = &SpellEffectClass::SpellEffectThreat; // 63
    m_spellEffectMap[SPELL_EFFECT_TRIGGER_SPELL]                = &SpellEffectClass::SpellEffectTriggerSpell; // 64
    m_spellEffectMap[SPELL_EFFECT_APPLY_RAID_AURA]              = &SpellEffectClass::SpellEffectDelayed; // 65
    m_spellEffectMap[SPELL_EFFECT_POWER_FUNNEL]                 = &SpellEffectClass::SpellEffectPowerFunnel; // 66
    m_spellEffectMap[SPELL_EFFECT_HEAL_MAX_HEALTH]              = &SpellEffectClass::SpellEffectHealMaxHealth; // 67
    m_spellEffectMap[SPELL_EFFECT_INTERRUPT_CAST]               = &SpellEffectClass::SpellEffectInterruptCast; // 68
    m_spellEffectMap[SPELL_EFFECT_DISTRACT]                     = &SpellEffectClass::SpellEffectDistract; // 69
    m_spellEffectMap[SPELL_EFFECT_PULL]                         = &SpellEffectClass::SpellEffectPull; // 70
    m_spellEffectMap[SPELL_EFFECT_PICKPOCKET]                   = &SpellEffectClass::SpellEffectPickpocket; // 71
    m_spellEffectMap[SPELL_EFFECT_ADD_FARSIGHT]                 = &SpellEffectClass::SpellEffectAddFarsight; // 72
    m_spellEffectMap[SPELL_EFFECT_UNTRAIN_TALENTS]              = &SpellEffectClass::SpellEffectResetTalents; // 73
    m_spellEffectMap[SPELL_EFFECT_USE_GLYPH]                    = &SpellEffectClass::SpellEffectUseGlyph; // 74
    m_spellEffectMap[SPELL_EFFECT_HEAL_MECHANICAL]              = &SpellEffectClass::SpellEffectHealMechanical; // 75
    m_spellEffectMap[SPELL_EFFECT_SUMMON_OBJECT_WILD]           = &SpellEffectClass::SpellEffectSummonObject; // 76
    m_spellEffectMap[SPELL_EFFECT_SCRIPT_EFFECT]                = &SpellEffectClass::SpellEffectScriptEffect; // 77
    m_spellEffectMap[SPELL_EFFECT_SANCTUARY]                    = &SpellEffectClass::SpellEffectSanctuary; // 79
    m_spellEffectMap[SPELL_EFFECT_ADD_COMBO_POINTS]             = &SpellEffectClass::SpellEffectAddComboPoints; // 80
    m_spellEffectMap[SPELL_EFFECT_DUEL]                         = &SpellEffectClass::SpellEffectDuel; // 83
    m_spellEffectMap[SPELL_EFFECT_STUCK]                        = &SpellEffectClass::SpellEffectStuck; // 84
    m_spellEffectMap[SPELL_EFFECT_SUMMON_PLAYER]                = &SpellEffectClass::SpellEffectSummonPlayer; // 85
    m_spellEffectMap[SPELL_EFFECT_ACTIVATE_OBJECT]              = &SpellEffectClass::SpellEffectActivateObject; // 86
    m_spellEffectMap[SPELL_EFFECT_WMO_DAMAGE]                   = &SpellEffectClass::SpellEffectWMODamage; // 87
    m_spellEffectMap[SPELL_EFFECT_WMO_REPAIR]                   = &SpellEffectClass::SpellEffectWMORepair; // 88
    m_spellEffectMap[SPELL_EFFECT_WMO_CHANGE]                   = &SpellEffectClass::SpellEffectChangeWMOState; // 89
    m_spellEffectMap[SPELL_EFFECT_KILL_CREDIT]                  = &SpellEffectClass::SpellEffectKillCredit; // 90
    m_spellEffectMap[SPELL_EFFECT_ENCHANT_HELD_ITEM]            = &SpellEffectClass::SpellEffectEnchantHeldItem; // 92
    m_spellEffectMap[SPELL_EFFECT_SELF_RESURRECT]               = &SpellEffectClass::SpellEffectSelfResurrect; // 94
    m_spellEffectMap[SPELL_EFFECT_SKINNING]                     = &SpellEffectClass::SpellEffectSkinning; // 95
    m_spellEffectMap[SPELL_EFFECT_CHARGE]                       = &SpellEffectClass::SpellEffectCharge; // 96
    m_spellEffectMap[SPELL_EFFECT_SUMMON_ALL_TOTEMS]            = &SpellEffectClass::SpellEffectPlaceTotemsOnBar; // 97
    m_spellEffectMap[SPELL_EFFECT_KNOCK_BACK]                   = &SpellEffectClass::SpellEffectKnockBack; // 98
    m_spellEffectMap[SPELL_EFFECT_DISENCHANT]                   = &SpellEffectClass::SpellEffectDisenchant; // 99
    m_spellEffectMap[SPELL_EFFECT_INEBRIATE]                    = &SpellEffectClass::SpellEffectInebriate; // 100
    m_spellEffectMap[SPELL_EFFECT_FEED_PET]                     = &SpellEffectClass::SpellEffectFeedPet; // 101
    m_spellEffectMap[SPELL_EFFECT_DISMISS_PET]                  = &SpellEffectClass::SpellEffectDismissPet; // 102
    m_spellEffectMap[SPELL_EFFECT_REPUTATION]                   = &SpellEffectClass::SpellEffectReputation; // 103
    m_spellEffectMap[SPELL_EFFECT_SUMMON_OBJECT_SLOT1]          = &SpellEffectClass::SpellEffectSummonObjectSlot; // 104
    m_spellEffectMap[SPELL_EFFECT_SUMMON_OBJECT_SLOT2]          = &SpellEffectClass::SpellEffectSummonObjectSlot; // 105
    m_spellEffectMap[SPELL_EFFECT_SUMMON_OBJECT_SLOT3]          = &SpellEffectClass::SpellEffectSummonObjectSlot; // 106
    m_spellEffectMap[SPELL_EFFECT_SUMMON_OBJECT_SLOT4]          = &SpellEffectClass::SpellEffectSummonObjectSlot; // 107
    m_spellEffectMap[SPELL_EFFECT_DISPEL_MECHANIC]              = &SpellEffectClass::SpellEffectDispelMechanic; // 108
    m_spellEffectMap[SPELL_EFFECT_SUMMON_DEAD_PET]              = &SpellEffectClass::SpellEffectSummonDeadPet; // 109
    m_spellEffectMap[SPELL_EFFECT_DESTROY_ALL_TOTEMS]           = &SpellEffectClass::SpellEffectDestroyAllTotems; // 110
    m_spellEffectMap[SPELL_EFFECT_RESURRECT_FLAT]               = &SpellEffectClass::SpellEffectResurrectNew; // 113
    m_spellEffectMap[SPELL_EFFECT_ATTACK_ME]                    = &SpellEffectClass::SpellEffectAttackMe; // 114
    m_spellEffectMap[SPELL_EFFECT_SKIN_PLAYER_CORPSE]           = &SpellEffectClass::SpellEffectSkinPlayerCorpse; // 116
    m_spellEffectMap[SPELL_EFFECT_SKILL]                        = &SpellEffectClass::SpellEffectSkill; // 118
    m_spellEffectMap[SPELL_EFFECT_APPLY_PET_AURA]               = &SpellEffectClass::SpellEffectDelayed; // 119
    m_spellEffectMap[SPELL_EFFECT_DUMMYMELEE]                   = &SpellEffectClass::SpellEffectDummyMelee; // 121
    m_spellEffectMap[SPELL_EFFECT_SEND_TAXI]                    = &SpellEffectClass::SpellEffectSendTaxi; // 123
    m_spellEffectMap[SPELL_EFFECT_PLAYER_PULL]                  = &SpellEffectClass::SpellEffectPull; // 124
    m_spellEffectMap[SPELL_EFFECT_SPELL_STEAL]                  = &SpellEffectClass::SpellEffectSpellSteal; // 126
    m_spellEffectMap[SPELL_EFFECT_PROSPECTING]                  = &SpellEffectClass::SpellEffectProspecting; // 127
    m_spellEffectMap[SPELL_EFFECT_APPLY_AREA_AURA_FRIEND]       = &SpellEffectClass::SpellEffectDelayed; // 128
    m_spellEffectMap[SPELL_EFFECT_APPLY_AREA_AURA_ENEMY]        = &SpellEffectClass::SpellEffectDelayed; // 129
    m_spellEffectMap[SPELL_EFFECT_REDIRECT_THREAT]              = &SpellEffectClass::SpellEffectRedirectThreat; // 130
    m_spellEffectMap[SPELL_EFFECT_PLAY_MUSIC]                   = &SpellEffectClass::SpellEffectPlayMusic; // 132
    m_spellEffectMap[SPELL_EFFECT_KILL_CREDIT_2]                = &SpellEffectClass::SpellEffectKillCredit; // 134
    m_spellEffectMap[SPELL_EFFECT_CALL_PET]                     = &SpellEffectClass::SpellEffectSummonPet; // 135
    m_spellEffectMap[SPELL_EFFECT_HEAL_PCT]                     = &SpellEffectClass::SpellEffectRestoreHealthPct; // 136
    m_spellEffectMap[SPELL_EFFECT_ENERGIZE_PCT]                 = &SpellEffectClass::SpellEffectRestoreManaPct; // 137
    m_spellEffectMap[SPELL_EFFECT_LEAP_BACK]                    = &SpellEffectClass::SpellEffectDisengage; // 138
    m_spellEffectMap[SPELL_EFFECT_CLEAR_QUEST]                  = &SpellEffectClass::SpellEffectClearFinishedQuest; // 139
    m_spellEffectMap[SPELL_EFFECT_FORCE_CAST]                   = &SpellEffectClass::SpellEffectTeleportUnits; // 140
    m_spellEffectMap[SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE]     = &SpellEffectClass::SpellEffectTriggerSpellWithValue; // 142
    m_spellEffectMap[SPELL_EFFECT_APPLY_DEMON_AURA]             = &SpellEffectClass::SpellEffectDelayed; // 143
    m_spellEffectMap[SPELL_EFFECT_KNOCKBACK_2]                  = &SpellEffectClass::SpellEffectKnockBack; // 144
    m_spellEffectMap[SPELL_EFFECT_TRACTOR_BEAM_FROM_DEST]       = &SpellEffectClass::SpellEffectPull; // 145
    m_spellEffectMap[SPELL_EFFECT_ACTIVATE_RUNE]                = &SpellEffectClass::SpellEffectActivateRune; // 146
    m_spellEffectMap[SPELL_EFFECT_QUEST_FAIL]                   = &SpellEffectClass::SpellEffectFailQuest; // 147
    m_spellEffectMap[SPELL_EFFECT_CHARGE2]                      = &SpellEffectClass::SpellEffectCharge; // 149
    m_spellEffectMap[SPELL_EFFECT_QUEST_OFFER]                  = &SpellEffectClass::SpellEffectStartQuest; // 150
    m_spellEffectMap[SPELL_EFFECT_TRIGGER_SPELL_2]              = &SpellEffectClass::SpellEffectTriggerSpell;
    m_spellEffectMap[SPELL_EFFECT_CREATE_PET]                   = &SpellEffectClass::SpellEffectCreatePet;
    m_spellEffectMap[SPELL_EFFECT_TITAN_GRIP]                   = &SpellEffectClass::SpellEffectTitanGrip;
    m_spellEffectMap[SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC]       = &SpellEffectClass::SpellEffectAddPrismaticSocket;
    m_spellEffectMap[SPELL_EFFECT_CREATE_ITEM_2]                = &SpellEffectClass::SpellEffectCreateRandomItem;
    m_spellEffectMap[SPELL_EFFECT_MILLING]                      = &SpellEffectClass::SpellEffectMilling;
    m_spellEffectMap[SPELL_EFFECT_ALLOW_PET_RENAME]             = &SpellEffectClass::SpellEffectAllowPetRename;
    m_spellEffectMap[SPELL_EFFECT_SET_TALENT_SPECS_COUNT]       = &SpellEffectClass::SpellEffectSetTalentSpecsCount;
    m_spellEffectMap[SPELL_EFFECT_ACTIVATE_TALENT_SPEC]         = &SpellEffectClass::SpellEffectActivateTalentSpec;
    m_spellEffectMap[SPELL_EFFECT_DAMAGE_FROM_MAX_HEALTH_PCT]   = &SpellEffectClass::SpellEffectDelayed;
}

void SpellEffectClass::SpellEffectNULL(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    sLog.Debug("Spell","Unhandled spell effect %u in spell %u.", m_spellInfo->Effect[i], m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectInstantKill(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if(!target->IsUnit() || castPtr<Unit>(target)->isAlive())
        return;

    Unit *unitTarget = castPtr<Unit>(target);
    if(!sSpellMgr.CanTriggerInstantKillEffect(_unitCaster, unitTarget, m_spellInfo))
        return;

    // Deal the killing damage
    _unitCaster->DealDamage(unitTarget, unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH), 0, 0, m_spellInfo->Id);

    // Send a message log as well
    WorldPacket data(SMSG_SPELLINSTAKILLLOG, 200);
    data << _unitCaster->GetGUID() << unitTarget->GetGUID() << m_spellInfo->Id;
    _unitCaster->SendMessageToSet(&data, true);
}

void SpellEffectClass::SpellEffectSchoolDMG(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // dmg school
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    if((target->IsUnit() && !castPtr<Unit>(target)->isAlive()) || spTarget == NULL)
        return;

    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(_unitCaster && unitTarget && rawAmt)
        amount = _unitCaster->GetSpellBonusDamage(unitTarget, m_spellInfo, i, amount, false);

    // Add to our accumulative damage
    spTarget->accumAmount += amount;
}

void SpellEffectClass::SpellEffectDummy(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Dummy(Scripted events)
{
    if(sSpellMgr.HandleDummyEffect(this, i, _unitCaster, target, amount))
        return;

    // Heal: 47633, 47632
    sLog.outDebug("Dummy spell not handled: %u\n", m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectTeleportUnits(uint32 i, WorldObject *target, int32 amount, bool rawAmt)  // Teleport Units
{
    uint32 mapId;
    float x,y,z,o;
    if(!sSpellMgr.FetchSpellCoordinates(this, i, target, amount, mapId, x, y, z, o))
        return;

    if(target->IsPlayer())
    {
        castPtr<Player>(target)->EventAttackStop();
        castPtr<Player>(target)->SetSelection(NULL);
        castPtr<Player>(target)->SafeTeleport(mapId, 0, x, y, z, o);
        return;
    }
}

void SpellEffectClass::SpellEffectPowerDrain(uint32 i, WorldObject *target, int32 amount, bool rawAmt)  // Power Drain
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 powerField = UNIT_FIELD_POWERS;
    if(m_spellInfo->EffectMiscValue[i] < 5)
        powerField += m_spellInfo->EffectMiscValue[i];
    uint32 curPower = unitTarget->GetUInt32Value(powerField);
    uint32 amt = _unitCaster->GetSpellBonusDamage(unitTarget, m_spellInfo, i, amount, false);

    if( unitTarget->IsPlayer() )
        amt *= float2int32( 1 - ( ( castPtr<Player>(unitTarget)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE ) * 2 ) / 100.0f ) );

    if(amt > curPower)
        amt = curPower;

    unitTarget->SetUInt32Value(powerField, curPower - amt);
    _unitCaster->Energize(_unitCaster, m_spellInfo->Id, amt, m_spellInfo->EffectMiscValue[i]);
    unitTarget->SendPowerUpdate();
}

void SpellEffectClass::SpellEffectHealthLeech(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Health Leech
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 amt = std::min<uint32>(curHealth, amount);

    _unitCaster->DealDamage(unitTarget, amt, 0, 0, m_spellInfo->Id);

    float coef = m_spellInfo->EffectValueMultiplier[i]; // how much health is restored per damage dealt
    _unitCaster->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, m_spellInfo->SpellGroupType);
    _unitCaster->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, m_spellInfo->SpellGroupType);

    amt = float2int32((float)amt * coef);

    uint32 playerCurHealth = _unitCaster->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 playerMaxHealth = _unitCaster->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    uint32 overheal = 0;
    if(playerCurHealth + amt > playerMaxHealth)
    {
        _unitCaster->SetUInt32Value(UNIT_FIELD_HEALTH, playerMaxHealth);
        overheal = playerCurHealth + amt - playerMaxHealth;
    } else _unitCaster->SetUInt32Value(UNIT_FIELD_HEALTH, playerCurHealth + amt);

    SendHealSpellOnPlayer(_unitCaster, _unitCaster, amt, false, overheal, m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectHeal(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Heal
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( spTarget == NULL || unitTarget == NULL || !unitTarget->isAlive() )
        return;
    if(rawAmt == true)
        amount = _unitCaster->GetSpellBonusDamage(unitTarget, m_spellInfo, i, amount, true);
    // Add healing amount to accumulated data
    spTarget->accumAmount += amount;
}

void SpellEffectClass::SpellEffectBind(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Innkeeper Bind
{
    if(!target->IsPlayer())
        return;
    Player *plrTarget = castPtr<Player>(target);

    plrTarget->SetBindPoint(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetMapId(), target->GetZoneId());

    WorldPacket data(SMSG_BINDPOINTUPDATE, 16);
    data << plrTarget->GetBindPositionX() << plrTarget->GetBindPositionY() << plrTarget->GetBindPositionZ() << plrTarget->GetBindMapId() << plrTarget->GetBindZoneId();
    plrTarget->PushPacket( &data );

    data.Initialize(SMSG_PLAYERBOUND);
    data << m_casterGuid << plrTarget->GetBindZoneId();
    plrTarget->PushPacket(&data);

}

void SpellEffectClass::SpellEffectQuestComplete(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Quest Complete
{
    if(!target->IsPlayer())
        return;

    if(QuestLogEntry *qle = castPtr<Player>(target)->GetQuestLogForEntry(m_spellInfo->EffectMiscValue[i]))
    {
        qle->SendQuestComplete();
        qle->SetQuestStatus(QUEST_STATUS__COMPLETE);
        qle->UpdatePlayerFields();
    }
}

void SpellEffectClass::SpellEffectWeaponDamageNoSchool(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Weapon damage + (no School)
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    if(spTarget == NULL || (target->IsUnit() && !castPtr<Unit>(target)->isAlive()))
        return;

    float minDamage = 0.f, damageDiff = 1.f;
    if(_unitCaster->IsCreature() || !(false/* Check to see if damage is based on weapon proto or player stat*/))
    {   // Creatures we don't have weapons so just grab our actual min and max damage
        minDamage = _unitCaster->GetFloatValue(UNIT_FIELD_MINDAMAGE);
        damageDiff = _unitCaster->GetFloatValue(UNIT_FIELD_MAXDAMAGE)-minDamage;
    }
    else // Players we grab actual weapon damage values
    {
        Player *p_caster = castPtr<Player>(_unitCaster);
        Item *equippedWeapon = NULL;
        if(m_spellInfo->spellType >= RANGED)
            equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
        else if(m_spellInfo->spellType == OFFHAND)
            equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        else equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if(equippedWeapon == NULL)
            return;
        minDamage = equippedWeapon->GetProto()->minDamage;
        damageDiff = equippedWeapon->GetProto()->maxDamage-minDamage;
    }

    spTarget->accumAmount += amount + float2int32(minDamage + ((float)(rand()%float2int32(damageDiff))));
}

void SpellEffectClass::SpellEffectAddExtraAttacks(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Add Extra Attacks
{
    if(!target->IsUnit())
        return;
    castPtr<Unit>(target)->QueueExtraAttacks(m_spellInfo->Id, amount);
}

void SpellEffectClass::SpellEffectCreateItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Create item
{
    if(!target->IsPlayer())
        return;

    uint32 count = 0;
    Player *playerTarget = castPtr<Player>(target);
    ItemPrototype *proto = sSpellMgr.GetCreateItemForSpell(playerTarget, m_spellInfo, i, amount, count);
    if(proto == NULL || count == 0)
        return;

    PlayerInventory *inventory = playerTarget->GetInventory();
    if(inventory->CanReceiveItem(proto, count, NULL) != INV_ERR_OK)
    {
        SendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
        return;
    }

    if(inventory->AddItemById(proto->ItemId, count, proto->RandomPropId, ADDITEM_FLAG_CREATED, playerTarget))
    {
        SendCastResult(SPELL_FAILED_TRY_AGAIN);
        return;
    }

    if(uint32 skillLine = m_spellInfo->SpellSkillLine)
        DetermineSkillUp(playerTarget, skillLine);

    // Should be a virtual but this will do
    castPtr<Spell>(this)->AddCooldown();
}

void SpellEffectClass::SpellEffectPersistentAA(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Persistent Area Aura
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(m_AreaAura || !_unitCaster->IsInWorld())
        return;

    DynamicObject* dynObj = _unitCaster->GetMapInstance()->AllocateDynamicObject(m_casterGuid);
    if(dynObj == NULL) // We have to pass our allocation checks
        return;

    int32 dur = GetDuration();
    float r = GetRadius(i);
    switch(m_targets.m_targetMask)
    {
    case TARGET_FLAG_SELF:
        {
            dynObj->Create(_unitCaster, this,  _unitCaster->GetPositionX(), _unitCaster->GetPositionY(), _unitCaster->GetPositionZ(), dur, r);
        }break;
    case TARGET_FLAG_UNIT:
        {
            if(!unitTarget || !unitTarget->isAlive())
            {
                dynObj->Remove();
                return;
            }

            dynObj->Create(_unitCaster, this, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), dur, r);
        }break;
    case TARGET_FLAG_OBJECT:
        {
            if(!unitTarget || !unitTarget->isAlive())
            {
                dynObj->Remove();
                return;
            }

            dynObj->Create(_unitCaster, this, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), dur, r);
        }break;
    case TARGET_FLAG_SOURCE_LOCATION:
        {
            dynObj->Create(_unitCaster, this, m_targets.m_src.x, m_targets.m_src.y, m_targets.m_src.z, dur, r);
        }break;
    case TARGET_FLAG_DEST_LOCATION:
        {
            dynObj->Create(_unitCaster, this, m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, dur, r);
        }break;
    default:
        dynObj->Remove();
        return;
    }

    m_AreaAura = true;
}

void SpellEffectClass::SpellEffectLeap(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Leap
{
    //FIXME: check for obstacles
    if(!_unitCaster->IsPlayer())
        return;
    Player *p_caster = castPtr<Player>(_unitCaster);

    float radius = GetRadius(i);

    // remove movement impeding auras
    p_caster->m_AuraInterface.RemoveAllAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);

    // just in case
    p_caster->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_STUN);
    p_caster->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_ROOT);

    if(p_caster->GetMapInstance() && !IS_INSTANCE(p_caster->GetMapId()) && p_caster->GetMapInstance()->CanUseCollision(p_caster))
    {
        float ori = _unitCaster->GetOrientation();
        float posX = _unitCaster->GetPositionX()+(radius*(cosf(ori)));
        float posY = _unitCaster->GetPositionY()+(radius*(sinf(ori)));
        float posZ = _unitCaster->GetPositionZ();

        if( sVMapInterface.GetFirstPoint(p_caster->GetMapId(), p_caster->GetInstanceID(), p_caster->GetPhaseMask(), p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ() + Player::NoseHeight(p_caster->getRace(), p_caster->getGender()), posX, posY, p_caster->GetPositionZ(), posX, posY, posZ, -1.5f) )
        {
            posZ = p_caster->GetMapInstance()->GetWalkableHeight(p_caster, posX, posY, posZ);
            float diff = fabs(fabs(posZ) - fabs(_unitCaster->GetPositionZ()));
            if( diff <= 10.0f)
            {
                p_caster->blinked = true;
                p_caster->blinktimer = getMSTime()+5000;
                p_caster->SafeTeleport( p_caster->GetMapId(), p_caster->GetInstanceID(), posX, posY, posZ, _unitCaster->GetOrientation() );
            }
        }
        else
        {
            // either no objects in the way, or no wmo height
            posZ = p_caster->GetMapInstance()->GetWalkableHeight(p_caster, posX, posY, posZ);
            float diff = fabs(fabs(posZ) - fabs(_unitCaster->GetPositionZ()));
            if( diff <= 10.0f)
            {
                p_caster->blinked = true;
                p_caster->blinktimer = getMSTime()+5000;
                p_caster->SafeTeleport( p_caster->GetMapId(), p_caster->GetInstanceID(), posX, posY, posZ, _unitCaster->GetOrientation() );
            }
        }

        if(p_caster->blinked)
            return;
    }

    p_caster->blinked = true;

    WorldPacket data(SMSG_MOVE_KNOCK_BACK, 50);
    data << p_caster->GetGUID();
    data << getMSTime();
    data << cosf(p_caster->GetOrientation()) << sinf(p_caster->GetOrientation());
    data << radius;
    data << float(-10.0f);
    p_caster->PushPacket(&data);
}

void SpellEffectClass::SpellEffectEnergize(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Energize
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    _unitCaster->Energize(unitTarget, m_spellInfo->Id, amount, m_spellInfo->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectWeaponDmgPerc(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Weapon Percent damage
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || target == NULL || !_unitCaster->IsUnit() || m_spellInfo->spellType == NON_WEAPON)
        return;

    float minDamage = 0.f, damageDiff = 1.f;
    if(_unitCaster->IsCreature() || !(false/* Check to see if damage is based on weapon proto or player stat*/))
    {   // Creatures we don't have weapons so just grab our actual min and max damage
        minDamage = _unitCaster->GetFloatValue(UNIT_FIELD_MINDAMAGE);
        damageDiff = _unitCaster->GetFloatValue(UNIT_FIELD_MAXDAMAGE)-minDamage;
    }
    else // Players we grab actual weapon damage values
    {
        Player *p_caster = castPtr<Player>(_unitCaster);
        Item *equippedWeapon = NULL;
        if(m_spellInfo->spellType >= RANGED)
            equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
        else if(m_spellInfo->spellType == OFFHAND)
            equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        else equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if(equippedWeapon == NULL)
            return;
        minDamage = equippedWeapon->GetProto()->minDamage;
        damageDiff = equippedWeapon->GetProto()->maxDamage-minDamage;
    }

    spTarget->accumAmount += float2int32(((minDamage + (damageDiff ? ((float)(rand()%float2int32(damageDiff))) : 0.f)) * ((float)amount))/100.f);

    // Double down and do offhand strike
    if(m_spellInfo->NameHash == SPELL_HASH_MUTILATE)
    {

    }
}

void SpellEffectClass::SpellEffectTriggerMissile(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Trigger Missile
{
    //Used by mortar team
    //Triggers area affect spell at destinatiom
    uint32 spellid = m_spellInfo->EffectTriggerSpell[i];
    if(spellid == 0)
        return;

    SpellEntry *spInfo = dbcSpell.LookupEntry(spellid);
    if(spInfo == NULL )
        return;

    // Just send this spell where he wants :S
    _unitCaster->GetSpellInterface()->TriggerSpell(spInfo, m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z);
}

void SpellEffectClass::SpellEffectOpenLock(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Open Lock
{

}

void SpellEffectClass::SpellEffectOpenLockItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectProficiency(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if(!target->IsPlayer())
        return;

    // Only allow armor and weapon proficiencies
    if(m_spellInfo->EquippedItemClass != ITEM_CLASS_ARMOR && m_spellInfo->EquippedItemClass != ITEM_CLASS_WEAPON)
        return;

    Player *playerTarget = castPtr<Player>(target);
    uint32 proficiency = m_spellInfo->EquippedItemClass == ITEM_CLASS_ARMOR ? playerTarget->GetArmorProficiency() : playerTarget->GetWeaponProficiency();
    // See if we already have the proficiency
    if(proficiency & m_spellInfo->EquippedItemSubClassMask)
        return;
    if(m_spellInfo->EquippedItemClass == ITEM_CLASS_ARMOR)
        playerTarget->AddArmorProficiency(m_spellInfo->EquippedItemSubClassMask);
    else playerTarget->AddWeaponProficiency(m_spellInfo->EquippedItemSubClassMask);

    // If we're not in world just return
    if(!playerTarget->IsInWorld())
        return;
    playerTarget->SendProficiency(m_spellInfo->EquippedItemClass == ITEM_CLASS_ARMOR);
}

void SpellEffectClass::SpellEffectSendEvent(uint32 i, WorldObject *target, int32 amount, bool rawAmt) //Send Event
{
    sLog.outDebug("Event spell not handled: %u\n", m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectLearnSpell(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Learn Spell
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;

    if( m_spellInfo->Id == 483 || m_spellInfo->Id == 55884 )        // "Learning"
    {
        if(!_unitCaster->IsPlayer() || m_targets.m_itemTarget.empty())
            return;
        Player *p_caster = castPtr<Player>(_unitCaster);
        Item *useItem = p_caster->GetInventory()->GetItemByGUID(m_targets.m_itemTarget);
        if(useItem == NULL)
            return;

        uint32 spellid = 0;
        for(int i = 0; i < 5; i++)
        {
            if( useItem->GetProto()->Spells[i].Trigger == LEARNING && useItem->GetProto()->Spells[i].Id != 0 )
            {
                spellid = useItem->GetProto()->Spells[i].Id;
                break;
            }
        }

        if( !spellid || !dbcSpell.LookupEntry(spellid) )
            return;

        // learn me!
        p_caster->addSpell( spellid );

        // no normal handler
        return;
    }

    if(playerTarget!=NULL)
    {
        /*if(u_caster && isHostile(playerTarget, u_caster))
            return;*/
        SpellEntry *spellinfo = dbcSpell.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
        if(spellinfo == NULL)
            return;

        playerTarget->addSpell(spellinfo->Id);
        //smth is wrong here, first we add this spell to player then we may cast it on player...
        //remove specializations
        switch(spellinfo->Id)
        {
        case 26801: //Shadoweave Tailoring
            playerTarget->removeSpell(26798); //Mooncloth Tailoring
            playerTarget->removeSpell(26797); //Spellfire Tailoring
            break;
        case 26798: // Mooncloth Tailoring
            playerTarget->removeSpell(26801); //Shadoweave Tailoring
            playerTarget->removeSpell(26797); //Spellfire Tailoring
            break;
        case 26797: //Spellfire Tailoring
            playerTarget->removeSpell(26801); //Shadoweave Tailoring
            playerTarget->removeSpell(26798); //Mooncloth Tailoring
            break;
        case 10656: //Dragonscale Leatherworking
            playerTarget->removeSpell(10658); //Elemental Leatherworking
            playerTarget->removeSpell(10660); //Tribal Leatherworking
            break;
        case 10658: //Elemental Leatherworking
            playerTarget->removeSpell(10656); //Dragonscale Leatherworking
            playerTarget->removeSpell(10660); //Tribal Leatherworking
            break;
        case 10660: //Tribal Leatherworking
            playerTarget->removeSpell(10656); //Dragonscale Leatherworking
            playerTarget->removeSpell(10658); //Elemental Leatherworking
            break;
        case 28677: //Elixir Master
            playerTarget->removeSpell(28675); //Potion Master
            playerTarget->removeSpell(28672); //Transmutation Maste
            break;
        case 28675: //Potion Master
            playerTarget->removeSpell(28677); //Elixir Master
            playerTarget->removeSpell(28672); //Transmutation Maste
            break;
        case 28672: //Transmutation Master
            playerTarget->removeSpell(28675); //Potion Master
            playerTarget->removeSpell(28677); //Elixir Master
            break;
        case 20219: //Gnomish Engineer
            playerTarget->removeSpell(20222); //Goblin Engineer
            break;
        case 20222: //Goblin Engineer
            playerTarget->removeSpell(20219); //Gnomish Engineer
            break;
        case 9788: //Armorsmith
            playerTarget->removeSpell(9787); //Weaponsmith
            playerTarget->removeSpell(17039); //Master Swordsmith
            playerTarget->removeSpell(17040); //Master Hammersmith
            playerTarget->removeSpell(17041); //Master Axesmith
            break;
        case 9787: //Weaponsmith
            playerTarget->removeSpell(9788); //Armorsmith
            break;
        case 17041: //Master Axesmith
            playerTarget->removeSpell(9788); //Armorsmith
            playerTarget->removeSpell(17040); //Master Hammersmith
            playerTarget->removeSpell(17039); //Master Swordsmith
            break;
        case 17040: //Master Hammersmith
            playerTarget->removeSpell(9788); //Armorsmith
            playerTarget->removeSpell(17039); //Master Swordsmith
            playerTarget->removeSpell(17041); //Master Axesmith
            break;
        case 17039: //Master Swordsmith
            playerTarget->removeSpell(9788); //Armorsmith
            playerTarget->removeSpell(17040); //Master Hammersmith
            playerTarget->removeSpell(17041); //Master Axesmith
            break;
        }

        if(spellinfo->HasEffect(SPELL_EFFECT_WEAPON) || spellinfo->HasEffect(SPELL_EFFECT_DUAL_WIELD) || spellinfo->HasEffect(SPELL_EFFECT_PROFICIENCY))
        {
            SpellCastTargets targets(unitTarget->GetGUID());
            if(Spell *sp = new Spell(unitTarget, spellinfo))
                sp->prepare(&targets, true);
        }
        return;
    }

    // if we got here... try via pet spells..
    SpellEffectLearnPetSpell(i, target, amount, rawAmt);
}

void SpellEffectClass::SpellEffectLearnPetSpell(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectDispel(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Dispel
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL )
        return;

    uint32 start = 0, end = MAX_POSITIVE_AURAS;
    if(!sFactionSystem.isAttackable(_unitCaster, unitTarget))
        start = MAX_POSITIVE_AURAS, end = MAX_AURAS;

    unitTarget->m_AuraInterface.MassDispel(_unitCaster, i, m_spellInfo, amount, start, end);
}

void SpellEffectClass::SpellEffectSkillStep(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Skill Step
{
    if(!target->IsPlayer())
        return;
    Player* ptarget = castPtr<Player>(target);
    uint32 skill = m_spellInfo->EffectMiscValue[i];
    if( skill == 242 )
        skill = SKILL_LOCKPICKING; // somehow for lockpicking misc is different than the skill :s

    SkillLineEntry* sk = dbcSkillLine.LookupEntry( skill );
    if( sk == NULL )
        return;

    Player *plr = castPtr<Player>(_unitCaster);
    uint16 current = plr->getSkillLineVal(skill, false), skillMax = std::max<uint16>(1, amount * 75);
    sSpellMgr.getSkillLineValues(plr, sk, amount, skillMax, current);

    if( ptarget->HasSkillLine(skill) )
        ptarget->UpdateSkillLine(skill, amount, skillMax);
    else ptarget->AddSkillLine(skill, amount, skillMax, current);

    //professions fix, for unknow reason when u learn profession it
    //does not teach find herbs for herbalism etc. moreover there is no spell
    //in spell.dbc that would teach u this. It means blizz does it in some tricky way too
    switch( skill )
    {
    case SKILL_ALCHEMY:
        ptarget->addSpell( 2330 );//Minor Healing Potion
        ptarget->addSpell( 2329 );//Elixir of Lion's Strength
        ptarget->addSpell( 7183 );//Elixir of Minor Defense
        break;
    case SKILL_ENCHANTING:
        ptarget->addSpell( 7418 );//Enchant Bracer - Minor Health
        ptarget->addSpell( 7428 );//Enchant Bracer - Minor Deflection
        ptarget->addSpell( 7421 );//Runed Copper Rod
        ptarget->addSpell( 13262 );//Disenchant
        break;
    case SKILL_HERBALISM:
        ptarget->addSpell( 2383 );//find herbs
        break;
    case SKILL_MINING:
        ptarget->addSpell( 2657 );//smelt copper
        ptarget->addSpell( 2656 );//smelting
        ptarget->addSpell( 2580 );//find minerals
        break;
    case SKILL_FIRST_AID:
        ptarget->addSpell( 3275 );//Linen Bandage
        break;
    case SKILL_TAILORING:
        ptarget->addSpell( 2963 );//Bolt of Linen Cloth
        ptarget->addSpell( 2387 );//Linen Cloak
        ptarget->addSpell( 2393 );//White Linen Shirt
        ptarget->addSpell( 3915 );//Brown Linen Shirt
        ptarget->addSpell( 12044 );//Simple Linen Pants
        break;
    case SKILL_LEATHERWORKING:
        ptarget->addSpell( 2149 );//Handstitched Leather Boots
        ptarget->addSpell( 2152 );//Light Armor Kit
        ptarget->addSpell( 2881 );//Light Leather
        ptarget->addSpell( 7126 );//Handstitched Leather Vest
        ptarget->addSpell( 9058 );//Handstitched Leather Cloak
        ptarget->addSpell( 9059 );//Handstitched Leather Bracers
        break;
    case SKILL_ENGINEERING:
        ptarget->addSpell( 3918 );//Rough Blasting Powder
        ptarget->addSpell( 3919 );//Rough Dynamite
        ptarget->addSpell( 3920 );//Crafted Light Shot
        break;
    case SKILL_COOKING:
        ptarget->addSpell( 2538 );//Charred Wolf Meat
        ptarget->addSpell( 2540 );//Roasted Boar Meat
        ptarget->addSpell( 818 );//Basic Campfire
        ptarget->addSpell( 8604 );//Herb Baked Egg
        break;
    case SKILL_BLACKSMITHING:
        ptarget->addSpell( 2660 );//Rough Sharpening Stone
        ptarget->addSpell( 2663 );//Copper Bracers
        ptarget->addSpell( 12260 );//Rough Copper Vest
        ptarget->addSpell( 2662 );//Copper Chain Pants
        ptarget->addSpell( 3115 );//Rough Weightstone
        break;
    case SKILL_JEWELCRAFTING:
        ptarget->addSpell( 25255 );// Delicate Copper Wire
        ptarget->addSpell( 25493 );// Braided Copper Ring
        ptarget->addSpell( 26925 );// Woven Copper Ring
        ptarget->addSpell( 32259 );// Rough Stone Statue
        break;
    case SKILL_INSCRIPTION:
        ptarget->addSpell( 51005 );// Milling
        ptarget->addSpell( 48116 );// Scroll of Spirit
        ptarget->addSpell( 48114 );// Scroll of Intellect
        ptarget->addSpell( 45382 );// Scroll of Stamina
        ptarget->addSpell( 52738 );// Ivory Ink
        break;
    }
}

void SpellEffectClass::SpellEffectDetect(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectSummonObject(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectEnchantItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Enchant Item Permanent
{
    if(!target->IsPlayer())
        return;
    Player *pTarget = castPtr<Player>(target);
    Item *itemTarget = pTarget->GetInventory()->GetItemByGUID(m_targets.m_itemTarget);
    if(itemTarget == NULL)
        return;

    uint32 enchantmentId = m_spellInfo->EffectMiscValue[i];
    SpellItemEnchantEntry *enchant = dbcSpellItemEnchant.LookupEntry(enchantmentId);
    if(enchant == NULL )
    {
        sLog.outError("Invalid enchantment entry %u for Spell %u", m_spellInfo->EffectMiscValue[i], m_spellInfo->Id);
        return;
    }

    //remove other perm enchantment that was enchanted by profession
    itemTarget->RemovePermanentEnchant();
    int32 Slot = itemTarget->AddEnchantment(enchantmentId, NULL, true, true, false, PERM_ENCHANTMENT_SLOT);
    if(Slot < 0)
        return; // Apply failed
    itemTarget->m_isDirty = true;
}

void SpellEffectClass::SpellEffectEnchantItemTemporary(uint32 i, WorldObject *target, int32 amount, bool rawAmt)  // Enchant Item Temporary
{
    if(!target->IsPlayer())
        return;
    Player *pTarget = castPtr<Player>(target);
    Item *itemTarget = pTarget->GetInventory()->GetItemByGUID(m_targets.m_itemTarget);
    if(itemTarget == NULL)
        return;

    uint32 enchantmentId = m_spellInfo->EffectMiscValue[i];
    SpellItemEnchantEntry *enchant = dbcSpellItemEnchant.LookupEntry(enchantmentId);
    if(enchant == NULL )
    {
        sLog.outError("Invalid enchantment entry %u for Spell %u", m_spellInfo->EffectMiscValue[i], m_spellInfo->Id);
        return;
    }

    //remove other perm enchantment that was enchanted by profession
    itemTarget->RemoveTemporaryEnchant();
    int32 Slot = itemTarget->AddEnchantment(enchantmentId, amount, false, true, false, TEMP_ENCHANTMENT_SLOT);
    if(Slot < 0)
        return; // Apply failed
    itemTarget->m_isDirty = true;
}

void SpellEffectClass::SpellEffectAddPrismaticSocket(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

bool isExotic(uint32 family)
{
    switch(family)
    {
    case FAMILY_WORM:
    case FAMILY_RHINO:
    case FAMILY_CHIMAERA:
    case FAMILY_SILITHID:
    case FAMILY_DEVILSAUR:
    case FAMILY_CORE_HOUND:
    case FAMILY_SPIRIT_BEAST:
        return true;
    }
    return false;
}

void SpellEffectClass::SpellEffectTameCreature(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectSummonPet(uint32 i, WorldObject *target, int32 amount, bool rawAmt) //summon - pet
{

}

void SpellEffectClass::SpellEffectWeaponDamage(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Weapon damage +
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    if(spTarget == NULL || (target->IsUnit() && !castPtr<Unit>(target)->isAlive()))
        return;

    float minDamage = 0.f, damageDiff = 1.f;
    if(_unitCaster->IsCreature() || !(false/* Check to see if damage is based on weapon proto or player stat*/))
    {   // Creatures we don't have weapons so just grab our actual min and max damage
        minDamage = _unitCaster->GetFloatValue(UNIT_FIELD_MINDAMAGE);
        damageDiff = _unitCaster->GetFloatValue(UNIT_FIELD_MAXDAMAGE)-minDamage;
    }
    else // Players we grab actual weapon damage values
    {
        Player *p_caster = castPtr<Player>(_unitCaster);
        Item *equippedWeapon = NULL;
        if(m_spellInfo->spellType >= RANGED)
            equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
        else if(m_spellInfo->spellType == OFFHAND)
            equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        else equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if(equippedWeapon == NULL)
            return;
        minDamage = equippedWeapon->GetProto()->minDamage;
        damageDiff = equippedWeapon->GetProto()->maxDamage-minDamage;
    }

    spTarget->accumAmount += amount + float2int32(minDamage + ((float)(rand()%float2int32(damageDiff))));
}

void SpellEffectClass::SpellEffectPowerBurn(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // power burn
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL ||!unitTarget->isAlive() )
        return;
    uint32 maxMana = unitTarget->GetMaxPower(POWER_TYPE_MANA);
    if(maxMana == 0)
        return;

    //Resilience reduction
    if(unitTarget->IsPlayer())
    {
        float dmg_reduction_pct = std::min<float>(0.33f, 2.2f * castPtr<Player>(unitTarget)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE ) / 100.0f);
        amount = float2int32(amount - (amount * dmg_reduction_pct));
    }

    //this is pct...
    int32 mana = float2int32( unitTarget->GetPower(POWER_TYPE_MANA) * (amount/100.0f));
    unitTarget->ModPower(POWER_TYPE_MANA, -mana);

    float coef = m_spellInfo->EffectValueMultiplier[i]; // damage per mana burned
    _unitCaster->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, m_spellInfo->SpellGroupType);
    _unitCaster->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, m_spellInfo->SpellGroupType);
    mana = float2int32((float)mana * coef);

    _unitCaster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, mana, 0.f, true,true);
}

void SpellEffectClass::SpellEffectThreat(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Threat
{

}

void SpellEffectClass::SpellEffectTriggerSpell(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Trigger Spell
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL)
        return;

    SpellEntry *spe = dbcSpell.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
    if(spe == NULL )
        return;

    if(m_spellInfo == spe) // Infinite loop fix.
        return;

    WoWGuid targetGuid = unitTarget->GetGUID();
    if(spe->procFlags & 0x00)
        targetGuid = _unitCaster->GetGUID();

    SpellCastTargets tgt(targetGuid);
    if(Spell* sp = new Spell(_unitCaster, spe))
        sp->prepare(&tgt, true);
}

void SpellEffectClass::SpellEffectPowerFunnel(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Power Funnel
{

}

void SpellEffectClass::SpellEffectHealMaxHealth(uint32 i, WorldObject *target, int32 amount, bool rawAmt)   // Heal Max Health
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    uint32 dif = unitTarget->GetUInt32Value( UNIT_FIELD_MAXHEALTH ) - unitTarget->GetUInt32Value( UNIT_FIELD_HEALTH );
    if( dif == 0 )
    {
        SendCastResult( SPELL_FAILED_ALREADY_AT_FULL_HEALTH );
        return;
    }

    if( unitTarget->IsPlayer())
         SendHealSpellOnPlayer( _unitCaster, unitTarget, dif, false, 0, m_spellInfo->Id );
    unitTarget->ModUnsigned32Value( UNIT_FIELD_HEALTH, dif );
}

void SpellEffectClass::SpellEffectInterruptCast(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Interrupt Cast
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    // can't apply stuns/fear/polymorph/root etc on boss
    if(unitTarget->GetTypeId() == TYPEID_UNIT)
    {
        Creature* c = castPtr<Creature>( unitTarget );
        if (c && c->GetCreatureData() && (c->GetCreatureData()->rank == ELITE_WORLDBOSS || c->GetCreatureData()->flags & CREATURE_FLAGS1_BOSS))
            return;
    }

    // FIXME:This thing prevent target from spell casting too but cant find.
    unitTarget->GetSpellInterface()->InterruptCast(castPtr<Spell>(this));
}

void SpellEffectClass::SpellEffectDistract(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Distract
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    //spellId 1725 Distract:Throws a distraction attracting the all monsters for ten sec's
    if(m_targets.m_dest.x != 0.0f || m_targets.m_dest.y != 0.0f || m_targets.m_dest.z != 0.0f)
    {
//      unitTarget->GetAIInterface()->MoveTo(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, 0);
        int32 Stare_duration=GetDuration();
        if(Stare_duration>30*60*1000)
            Stare_duration=10000;//if we try to stare for more then a half an hour then better not stare at all :P (bug)

        unitTarget->SetFacing(unitTarget->calcRadAngle(unitTarget->GetPositionX(),unitTarget->GetPositionY(),m_targets.m_dest.x,m_targets.m_dest.y));
    }

    //Smoke Emitter 164870
    //Smoke Emitter Big 179066
    //Unit Field Target of
}

void SpellEffectClass::SpellEffectPickpocket(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // pickpocket
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !_unitCaster->IsPlayer() || unitTarget->GetTypeId() != TYPEID_UNIT)
        return;

    //Show random loot based on roll,
    Creature* cTarget = castPtr<Creature>( unitTarget );
    if(cTarget->IsPickPocketed() || (cTarget->GetCreatureData() && cTarget->GetCreatureData()->type != UT_HUMANOID))
    {
        SendCastResult(SPELL_FAILED_TARGET_NO_POCKETS);
        return;
    }

    lootmgr.FillPickpocketingLoot(cTarget->GetLoot(), cTarget->GetEntry());

    uint32 _rank = cTarget->GetCreatureData() ? cTarget->GetCreatureData()->rank : 0;
    unitTarget->GetLoot()->gold = float2int32((_rank+1) * cTarget->getLevel() * (RandomUInt(5) + 1) * sWorld.getRate(RATE_MONEY));

    castPtr<Player>(_unitCaster)->SendLoot(cTarget->GetGUID(), cTarget->GetMapId(), LOOTTYPE_PICKPOCKETING);
    cTarget->SetPickPocketed(true);
}

void SpellEffectClass::SpellEffectAddFarsight(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Add Farsight
{
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL;
    if( p_caster == NULL )
        return;

    DynamicObject* dyn = p_caster->GetMapInstance()->AllocateDynamicObject(m_casterGuid);
    if(dyn == NULL)
        return;

    dyn->Create(p_caster, this, m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, GetDuration(), GetRadius(i));
    dyn->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dyn->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x80000002);
    p_caster->GetMapInstance()->AddObject(dyn);
    p_caster->SetUInt64Value(PLAYER_FARSIGHT, dyn->GetGUID());
    p_caster->SetUInt32Value(PLAYER_FARSIGHT, dyn->GetLowGUID());
    p_caster->GetMapInstance()->ChangeFarsightLocation(p_caster, m_targets.m_dest.x, m_targets.m_dest.y, true);
}

void SpellEffectClass::SpellEffectResetTalents(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Used by Trainers.
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( !playerTarget )
        return;

    playerTarget->ResetSpec(playerTarget->GetTalentInterface()->GetActiveSpec());
}

void SpellEffectClass::SpellEffectUseGlyph(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL;
    if( p_caster == NULL )
        return;

    if(uint8 result = p_caster->GetTalentInterface()->ApplyGlyph(m_targets.m_targetIndex, m_spellInfo->EffectMiscValue[i])) // there was an error
        SendCastResult(result);
    else p_caster->GetTalentInterface()->SendTalentInfo();
}

void SpellEffectClass::SpellEffectHealMechanical(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || unitTarget->GetCreatureType() != UT_MECHANICAL)
        return;

    Heal(unitTarget, i, amount);
}

void SpellEffectClass::SpellEffectScriptEffect(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Script Effect
{
    if(!sSpellMgr.TriggerScriptedEffect(this, i, target, amount))
        return;

    sLog.outDebug("Unhandled Scripted Effect In Spell %u", m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectSanctuary(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Stop all attacks made to you
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL )
        return;

    // also cancel any spells we are casting
    unitTarget->GetSpellInterface()->InterruptCast(castPtr<Spell>(this));
    unitTarget->smsg_AttackStop( unitTarget );

    if( unitTarget->IsPlayer() )
        castPtr<Player>(unitTarget)->EventAttackStop();
}

void SpellEffectClass::SpellEffectAddComboPoints(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Add Combo Points
{
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL;
    if( p_caster == NULL  || !p_caster->IsInWorld() || !p_caster->isAlive() )
        return;

    p_caster->AddComboPoints(target->GetGUID(), amount);
}

void SpellEffectClass::SpellEffectDuel(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Duel
{
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL, *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( p_caster == NULL  || !p_caster->IsInWorld() || !p_caster->isAlive() || playerTarget == p_caster )
        return;

    uint32 areaId = p_caster->GetAreaId();
    MapInstance *instance = p_caster->GetMapInstance();
    if(p_caster->HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY))
    {
        SendCastResult(SPELL_FAILED_NO_DUELING);
        return;
    }

    if(playerTarget == NULL)
    {
        SendCastResult(SPELL_FAILED_BAD_TARGETS);
        return; // invalid Target
    }

    if (!playerTarget->isAlive())
    {
        SendCastResult(SPELL_FAILED_TARGETS_DEAD);
        return; // Target not alive
    }

    if (playerTarget->hasStateFlag(UF_ATTACKING))
    {
        SendCastResult(SPELL_FAILED_TARGET_IN_COMBAT);
        return; // Target in combat with another unit
    }

    if (playerTarget->IsInDuel())
    {
        SendCastResult(SPELL_FAILED_TARGET_DUELING);
        return; // Already Dueling
    }

    if(playerTarget->hasGMTag() && !p_caster->GetSession()->HasPermissions())
    {
        SendCastResult(SPELL_FAILED_BAD_TARGETS);
        return;
    }

    //Get Flags position
    float dist = sqrtf(p_caster->GetDistanceSq(playerTarget)) * 0.5f; //half way
    float x = (p_caster->GetPositionX() + playerTarget->GetPositionX()*dist)/(1+dist) + cos(p_caster->GetOrientation()+(float(M_PI)/2))*2;
    float y = (p_caster->GetPositionY() + playerTarget->GetPositionY()*dist)/(1+dist) + sin(p_caster->GetOrientation()+(float(M_PI)/2))*2;
    float z = (p_caster->GetPositionZ() + playerTarget->GetPositionZ()*dist)/(1+dist);

    //Create flag/arbiter
    if(GameObject* pGameObj = instance->CreateGameObject(21680))
    {
        pGameObj->Load(p_caster->GetMapId(), x, y, z, p_caster->GetOrientation());
        pGameObj->SetInstanceID(p_caster->GetInstanceID());
        // Initialize the duel data, this will handle most fields
        pGameObj->InitializeDuelData(p_caster, playerTarget);
        // Push object into addition queue
        instance->AddObject(pGameObj);
    }
}

void SpellEffectClass::SpellEffectStuck(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL, *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( playerTarget == NULL || playerTarget != p_caster)
        return;

    uint32 mapid = playerTarget->GetBindMapId();
    float x = playerTarget->GetBindPositionX();
    float y = playerTarget->GetBindPositionY();
    float z = playerTarget->GetBindPositionZ();
    float orientation = 0.f;
}

void SpellEffectClass::SpellEffectSummonPlayer(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( playerTarget == NULL)
        return;

    if(!_unitCaster->GetMapInstance()->GetdbcMap()->IsContinent())
        return;

    playerTarget->SummonRequest(_unitCaster, _unitCaster->GetZoneId(), _unitCaster->GetMapId(), _unitCaster->GetInstanceID(), _unitCaster->GetPosition());
}

void SpellEffectClass::SpellEffectActivateObject(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Activate WorldObject
{

}

void SpellEffectClass::SpellEffectWMODamage(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectWMORepair(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectChangeWMOState(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectSelfResurrect(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL, *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( p_caster == NULL || playerTarget == NULL || playerTarget->isAlive() || playerTarget->PreventRes)
        return;

    uint32 mana = 0, health = 0, class_ = playerTarget->getClass();
    switch(m_spellInfo->Id)
    {
    case 3026:
    case 20758:
    case 20759:
    case 20760:
    case 20761:
    case 27240:
        {
            health = m_spellInfo->EffectMiscValue[i];
            mana = -amount;
        }break;
    case 21169: //Reincarnation. Ressurect with 20% health and mana
        {
            health = uint32((playerTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*amount)/100);
            mana = uint32((playerTarget->GetMaxPower(POWER_TYPE_MANA)*amount)/100);
        }
        break;
    default:
        {
            if(amount < 0)
                return;
            health = uint32(playerTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*amount/100);
            mana = uint32(playerTarget->GetMaxPower(POWER_TYPE_MANA)*amount/100);
        }break;
    }

    if(class_==WARRIOR||class_==ROGUE)
        mana=0;

    playerTarget->m_resurrectHealth = health;
    playerTarget->m_resurrectMana = mana;

    playerTarget->ResurrectPlayer();
    playerTarget->m_resurrectHealth = 0;
    playerTarget->m_resurrectMana = 0;
    playerTarget->SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);
}

void SpellEffectClass::SpellEffectSkinning(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsPlayer() ? castPtr<Unit>(target) : NULL;
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL;
    if( unitTarget == NULL || !unitTarget->IsCreature() || p_caster == NULL )
        return;

    Creature* cr = castPtr<Creature>( unitTarget );
    uint32 skill = cr->GetRequiredLootSkill();

    uint32 sk = p_caster->getSkillLineVal( skill ), lvl = cr->getLevel();
    if( ( sk >= lvl * 5 ) || ( ( sk + 100 ) >= lvl * 10 ) )
    {
        //Fill loot for Skinning
        lootmgr.FillGatheringLoot(cr->GetLoot(), cr->GetEntry());
        p_caster->SendLoot( cr->GetGUID(), cr->GetMapId(), LOOTTYPE_SKINNING );

        //Not skinable again
        cr->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        cr->m_skinned = true;
        //double chance from elite
        if(cr->GetCreatureData()->rank > 0)
            DetermineSkillUp(p_caster, skill ,sk < lvl * 5 ? sk/5 : lvl, 2);
        else DetermineSkillUp(p_caster, skill ,sk < lvl * 5 ? sk/5 : lvl, 1);
    } else SendCastResult(SPELL_FAILED_TARGET_UNSKINNABLE);
}

void SpellEffectClass::SpellEffectCharge(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    if (_unitCaster->IsStunned() || _unitCaster->IsRooted() || _unitCaster->IsPacified() || _unitCaster->IsFeared())
        return;

    float dx,dy;

    float x, y, z;
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        if(m_targets.m_dest.x == 0.0f || m_targets.m_dest.y == 0.0f)
            return;
        x = m_targets.m_dest.x;
        y = m_targets.m_dest.y;
        z = m_targets.m_dest.z;
    }
    else
    {
        x = unitTarget->GetPositionX();
        y = unitTarget->GetPositionY();
        z = unitTarget->GetPositionZ();
    }

    dx = x-_unitCaster->GetPositionX();
    dy = y-_unitCaster->GetPositionY();
    if(dx == 0.0f || dy == 0.0f)
        return;

    uint32 time = uint32( (sqrtf(_unitCaster->GetDistanceSq(x,y,z)) / ((_unitCaster->GetMovementInterface()->GetMoveSpeed(MOVE_SPEED_RUN) * 3.5) * 0.001f)) + 0.5);
    _unitCaster->SetPosition(x,y,z,0.0f);

    _unitCaster->addStateFlag(UF_ATTACKING);
    if(WoWGuid guid = unitTarget->GetGUID())
        _unitCaster->EventAttackStart(guid);
}

void SpellEffectClass::SpellEffectPlaceTotemsOnBar(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    return;
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL;
    if(!p_caster)
        return;

    uint32 button = 132+m_spellInfo->EffectMiscValue[i];
    uint32 button_count = m_spellInfo->EffectMiscValueB[i];
    for (uint32 slot = button; slot < button+button_count; slot++)
    {
        if(p_caster->GetTalentInterface()->GetActionButton(slot).GetType() != ACTION_BUTTON_SPELL)
             continue;
        uint32 spell = p_caster->GetTalentInterface()->GetActionButton(slot).GetAction();
        if(spell == 0)
            continue;
        SpellEntry* sp = dbcSpell.LookupEntry(spell);
        if(sp == NULL || !p_caster->HasSpell(spell))
            continue;
        if(/*!IsTotemSpell(sp) || */p_caster->SpellHasCooldown(spell))
            continue;
        Spell* pSpell = new Spell(p_caster, sp);
        if(!pSpell->HasPower())
        {
            delete pSpell;
            continue;
        }

        SpellCastTargets targets;
        pSpell->GenerateTargets(&targets);
        pSpell->prepare(&targets, true);
    }
}

void SpellEffectClass::SpellEffectSendTaxi(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if ( playerTarget == NULL || !playerTarget->isAlive() )
        return;

    if( playerTarget->HasFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER ) )
        return;

    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath( m_spellInfo->EffectMiscValue[i] );
    if( !taxipath )
        return;

    TaxiNodeEntry* taxinode = dbcTaxiNode.LookupEntry(taxipath->GetSourceNode());
    if( !taxinode )
        return;

    uint32 mount_entry = 0, modelid = 0;
    if( playerTarget->GetTeam() )       // TEAM_HORDE
    {
        if((mount_entry = taxinode->mountIdHorde) == 0)
            mount_entry = taxinode->mountIdAlliance;

        CreatureData* ctrData = sCreatureDataMgr.GetCreatureData( mount_entry );
        if( ctrData == NULL )
            return;
        if((modelid = ctrData->displayInfo[1]) == 0)
            return;
    }
    else                                // TEAM_ALLIANCE
    {
        if((mount_entry = taxinode->mountIdAlliance) == 0)
            mount_entry = taxinode->mountIdHorde;

        CreatureData* ctrData = sCreatureDataMgr.GetCreatureData( mount_entry );
        if( ctrData == NULL )
            return;
        if((modelid = ctrData->displayInfo[0]) == 0)
            return;
    }

    playerTarget->TaxiStart( taxipath, modelid );
}

void SpellEffectClass::SpellEffectPull(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : _unitCaster;
    if(unitTarget == NULL)
        return;

    float pullX = 0.f, pullY = 0.f, pullZ = 0.f;
    if(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        pullX = m_targets.m_dest.x;
        pullY = m_targets.m_dest.y;
        pullZ = m_targets.m_dest.z;
    }
    else
    {
        pullX = _unitCaster->GetPositionX();
        pullY = _unitCaster->GetPositionY();
        pullZ = _unitCaster->GetPositionZ();
    }

    float arc = 10.f;
    if(m_spellInfo->EffectMiscValue[i])
        arc = m_spellInfo->EffectMiscValue[i]/10.f;
    else if(m_spellInfo->EffectMiscValueB[i])
        arc = m_spellInfo->EffectMiscValueB[i]/10.f;

    uint32 time = uint32((amount / arc) * 100);
    unitTarget->SetPosition(pullX, pullY, pullZ, 0.0f);

    if( unitTarget->IsPvPFlagged() )
        _unitCaster->SetPvPFlag();
    if( unitTarget->IsFFAPvPFlagged() )
        _unitCaster->SetFFAPvPFlag();
}

void SpellEffectClass::SpellEffectKnockBack(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    unitTarget->knockback(m_spellInfo->EffectBasePoints[i]+1, m_spellInfo->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectDisenchant(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectInebriate(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // lets get drunk!
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( playerTarget == NULL )
        return;

    uint16 currentDrunk = playerTarget->m_drunk;
    uint16 drunkMod = amount * 256;
    if (currentDrunk + drunkMod > 0xFFFF)
        currentDrunk = 0xFFFF;
    else
        currentDrunk += drunkMod;
    playerTarget->SetDrunk(currentDrunk, 0);
}

void SpellEffectClass::SpellEffectFeedPet(uint32 i, WorldObject *target, int32 amount, bool rawAmt)  // Feed Pet
{

}

void SpellEffectClass::SpellEffectReputation(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( playerTarget == NULL)
        return;

    playerTarget->GetFactionInterface()->ModStanding(m_spellInfo->EffectMiscValue[i], amount);
}

void SpellEffectClass::SpellEffectSummonObjectSlot(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectDispelMechanic(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    int32 sMisc = (int32)m_spellInfo->EffectMiscValue[i];
    unitTarget->m_AuraInterface.AttemptDispel(_unitCaster, sMisc, !(unitTarget == _unitCaster || !sFactionSystem.isAttackable( _unitCaster, unitTarget )));

    if( m_spellInfo->NameHash == SPELL_HASH_DAZED )
        unitTarget->Dismount();
}

void SpellEffectClass::SpellEffectSummonDeadPet(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL;
    if(p_caster == NULL )
        return;
}

uint32 TotemSpells[4] = { 63, 81, 82, 83 };

void SpellEffectClass::SpellEffectDestroyAllTotems(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL;
    if(p_caster == NULL || !p_caster->IsInWorld())
        return;

    uint32 energize_amt = 0;
    for(uint32 x = 0; x < 4; x++)
    {
        SummonPropertiesEntry * spe = dbcSummonProperties.LookupEntry(TotemSpells[x]);
        if(spe == NULL)
            continue;

        // atm totems are considert creature's
        if(p_caster->m_Summons[spe->slot].size())
        {
            for(std::vector<Creature*>::iterator itr = p_caster->m_Summons[spe->slot].begin(); itr != p_caster->m_Summons[spe->slot].end(); itr++)
            {
                SpellEntry * sp = dbcSpell.LookupEntry((*itr)->GetUInt32Value(UNIT_CREATED_BY_SPELL));
                if (sp != NULL)
                {
                    if( sp->ManaCost )
                        energize_amt += float2int32( (sp->ManaCost) * (amount/100.0f) );
                    else if( sp->ManaCostPercentage )
                        energize_amt += float2int32(((p_caster->GetUInt32Value(UNIT_FIELD_BASE_MANA)*sp->ManaCostPercentage)/100.0f) * (amount/100.0f) );
                }

                (*itr)->m_AuraInterface.RemoveAllAuras();
                (*itr)->Destruct();
            }
            p_caster->m_Summons[spe->slot].clear();
        }
    }

    if( energize_amt > 0 )
        p_caster->Energize(p_caster, m_spellInfo->Id, energize_amt, POWER_TYPE_MANA);
}

void SpellEffectClass::SpellEffectSummonDemonOld(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *p_caster = _unitCaster->IsPlayer() ? castPtr<Player>(_unitCaster) : NULL;
    if(p_caster == NULL ) //p_caster->getClass() != WARLOCK ) //summoning a demon shouldn't be warlock only, see spells 25005, 24934, 24810 etc etc
        return;

    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(m_spellInfo->EffectMiscValue[i]);
    if(ctrData == NULL)
        return;
}

void SpellEffectClass::SpellEffectResurrect(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Resurrect (Flat)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(playerTarget == NULL || playerTarget->isAlive() || !playerTarget->IsInWorld() || playerTarget->PreventRes)
        return;

    uint32 health = m_spellInfo->EffectBasePoints[i];
    uint32 mana = m_spellInfo->EffectMiscValue[i];

    playerTarget->m_resurrectHealth = health;
    playerTarget->m_resurrectMana = mana;

    SendResurrectRequest(playerTarget);
}

void SpellEffectClass::SpellEffectAttackMe(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    //castPtr<Creature>(unitTarget)->GetAIInterface()->AttackReaction(_unitCaster, 1, 0);
}

void SpellEffectClass::SpellEffectSkinPlayerCorpse(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectSkill(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    uint32 skillLine = m_spellInfo->SpellSkillLine ? m_spellInfo->SpellSkillLine : m_spellInfo->EffectMiscValue[i];
    if(skillLine == 0 || !_unitCaster->IsPlayer())
        return;
    SkillLineEntry *skillLineEntry = dbcSkillLine.LookupEntry(skillLine);
    if(skillLineEntry == NULL)
        return;

    Player *plr = castPtr<Player>(_unitCaster);
    uint16 current = plr->getSkillLineVal(skillLine, false), skillMax = std::max<uint16>(1, amount * 75);
    sSpellMgr.getSkillLineValues(plr, skillLineEntry, amount, skillMax, current);

    if(!plr->HasSkillLine(skillLine))
        plr->AddSkillLine(skillLine, amount, skillMax, current);
    else plr->UpdateSkillLine(skillLine, amount, skillMax);
}

void SpellEffectClass::SpellEffectDummyMelee(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Normalized Weapon damage +
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    if(spTarget == NULL)
        return;

    float minDamage = 0.f, damageDiff = 1.f;
    if(_unitCaster->IsCreature() || !(false/* Check to see if damage is based on weapon proto or player stat*/))
    {   // Creatures we don't have weapons so just grab our actual min and max damage
        minDamage = _unitCaster->GetFloatValue(UNIT_FIELD_MINDAMAGE);
        damageDiff = _unitCaster->GetFloatValue(UNIT_FIELD_MAXDAMAGE)-minDamage;
    }
    else // Players we grab actual weapon damage values
    {
        Player *p_caster = castPtr<Player>(_unitCaster);
        Item *equippedWeapon = NULL;
        if(m_spellInfo->spellType >= RANGED)
            equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
        else if(m_spellInfo->spellType == OFFHAND)
            equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        else equippedWeapon = p_caster->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if(equippedWeapon == NULL)
            return;
        minDamage = equippedWeapon->GetProto()->minDamage;
        damageDiff = equippedWeapon->GetProto()->maxDamage-minDamage;
    }

    spTarget->accumAmount += amount + float2int32(minDamage + ((float)(rand()%float2int32(damageDiff))));
}

void SpellEffectClass::SpellEffectSpellSteal(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if ( unitTarget == NULL || !unitTarget->isAlive())
        return;

    if( _unitCaster->GetGUID() != unitTarget->GetGUID() && unitTarget->IsPvPFlagged())
        _unitCaster->SetPvPFlag();

    if(!sFactionSystem.isAttackable(_unitCaster,unitTarget))
        return;

    unitTarget->m_AuraInterface.SpellStealAuras(_unitCaster, amount);
}

void SpellEffectClass::SpellEffectProspecting(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectResurrectNew(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    //base p =hp,misc mana
    if( playerTarget == NULL)
        return;
    if(playerTarget->isAlive() || !playerTarget->IsInWorld() || playerTarget->PreventRes)
        return;

    //resurr
    playerTarget->resurrector = _unitCaster->GetLowGUID();
    playerTarget->m_resurrectHealth = amount;
    playerTarget->m_resurrectMana = m_spellInfo->EffectMiscValue[i];
    playerTarget->m_resurrectLoction = _unitCaster->GetPosition();

    SendResurrectRequest(playerTarget);
}

void SpellEffectClass::SpellEffectTranformItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectEnvironmentalDamage(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    if(!target->IsUnit() || spTarget == NULL)
        return;

    spTarget->accumAmount += amount;

}

void SpellEffectClass::SpellEffectDismissPet(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    // remove pet.. but don't delete so it can be called later
    if( _unitCaster->IsPlayer() )
        return;

}

void SpellEffectClass::SpellEffectEnchantHeldItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectAddHonor(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectSpawn(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectRedirectThreat(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectPlayMusic(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(!unitTarget)
        return;

    unitTarget->PlaySoundToSet(m_spellInfo->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectKillCredit(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(playerTarget == NULL)
        return;

    if(CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(m_spellInfo->EffectMiscValue[i]))
        sQuestMgr._OnPlayerKill( playerTarget, m_spellInfo->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectRestoreManaPct(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 maxMana = (uint32)unitTarget->GetMaxPower(POWER_TYPE_MANA);
    uint32 modMana = amount * maxMana / 100;

    _unitCaster->Energize(unitTarget, m_spellInfo->Id, modMana, POWER_TYPE_MANA);
}

void SpellEffectClass::SpellEffectRestoreHealthPct(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 maxHp = (uint32)unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    uint32 modHp = amount * maxHp / 100;

    _unitCaster->Heal(unitTarget, modHp, m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectForceCast(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    return;
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL )
        return;

    SpellEntry* TriggeredSpell = dbcSpell.LookupEntry( m_spellInfo->EffectTriggerSpell[i] );
    if( TriggeredSpell == NULL )
        return;

    if(m_spellInfo == TriggeredSpell) // Infinite loop fix.
        return;

    //unitTarget->CastSpell( unitTarget, TriggeredSpell, true );
}

void SpellEffectClass::SpellEffectTriggerSpellWithValue(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL )
        return;

    SpellEntry* TriggeredSpell = dbcSpell.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
    if( TriggeredSpell == NULL )
        return;

    if(m_spellInfo == TriggeredSpell) // Infinite loop fix.
        return;

    SpellCastTargets tgt(unitTarget->GetGUID());
    if(Spell* sp= new Spell(_unitCaster, TriggeredSpell))
        sp->prepare(&tgt, true);
}

void SpellEffectClass::SpellEffectJump(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = _unitCaster;
    if(unitTarget == NULL && (unitTarget = u_caster) == NULL)
        return;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float ang = 0.0f;
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        if(m_targets.m_dest.x == 0 || m_targets.m_dest.y == 0 || m_targets.m_dest.z == 0)
            return;

        x = m_targets.m_dest.x;
        y = m_targets.m_dest.y;
        z = m_targets.m_dest.z;
        ang = atan2(y - _unitCaster->GetPositionY(), x - _unitCaster->GetPositionX());
        ang = (ang >= 0) ? ang : 2 * M_PI + ang;
    }
    else
    {
        x = unitTarget->GetPositionX();
        y = unitTarget->GetPositionY();
        z = unitTarget->GetPositionZ();
        ang = atan2(y - _unitCaster->GetPositionY(), x - _unitCaster->GetPositionX());
        ang = (ang >= 0) ? ang : 2 * M_PI + ang;

        float distance = unitTarget->GetModelHalfSize() * unitTarget->GetFloatValue(OBJECT_FIELD_SCALE_X);
        x = x - (distance * cosf(ang));
        y = y - (distance * sinf(ang));
    }

    float arc = 0.0f;
    if(m_spellInfo->EffectMiscValue[i])
        arc = m_spellInfo->EffectMiscValue[i]/10;
    else if(m_spellInfo->EffectMiscValueB[i])
        arc = m_spellInfo->EffectMiscValueB[i]/10;
    else arc = 10.0f;

}

void SpellEffectClass::SpellEffectTeleportToCaster(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if(Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL)
    {
        float x = _unitCaster->GetPositionX();
        float y = _unitCaster->GetPositionY();
        float z = _unitCaster->GetPositionZ();
        unitTarget->Teleport(x,y,z,_unitCaster->GetOrientation());
    }
}

void SpellEffectClass::SpellEffectMilling(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectAllowPetRename(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectStartQuest(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( !playerTarget )
        return;

    playerTarget->StartQuest(m_spellInfo->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectCreatePet(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( !playerTarget )
        return;

    if( playerTarget->getClass() != HUNTER )
        return;

    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData( m_spellInfo->EffectMiscValue[i] );
	if (ctrData == NULL)
		return;
}

void SpellEffectClass::SpellEffectTitanGrip(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectCreateRandomItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Create Random Item
{

}

void SpellEffectClass::SpellEffectSetTalentSpecsCount(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if( !_unitCaster->IsPlayer() )
        return;

    castPtr<Player>(_unitCaster)->GetTalentInterface()->UnlockSpec(amount);
}

void SpellEffectClass::SpellEffectActivateTalentSpec(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if( !_unitCaster->IsPlayer() )
        return;
    amount -= 1; // Add our negative offset

    // 0 = primary, 1 = secondary
    Player *p_caster = castPtr<Player>(_unitCaster);
    p_caster->GetTalentInterface()->ApplySpec(amount);
    p_caster->SetPower(p_caster->getPowerType(), 0);
}

void SpellEffectClass::SpellEffectDisengage(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    unitTarget->knockback(m_spellInfo->EffectBasePoints[i]+1, m_spellInfo->EffectMiscValue[i], true);
}

void SpellEffectClass::SpellEffectClearFinishedQuest(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if (playerTarget == NULL)
        return;

    playerTarget->m_completedQuests.erase(m_spellInfo->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectRemoveAura(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if (!unitTarget)
        return;

    unitTarget->RemoveAura(m_spellInfo->EffectTriggerSpell[i], unitTarget->GetGUID());
}

void SpellEffectClass::SpellEffectActivateRune(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectFailQuest(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if( !_unitCaster->IsPlayer() )
        return;

    if(Quest * qst = sQuestMgr.GetQuestPointer(m_spellInfo->EffectMiscValue[i]))
        sQuestMgr.SendQuestFailed(FAILED_REASON_FAILED, qst, castPtr<Player>(_unitCaster));
}
