/***
 * Demonstrike Core
 */

// thank god for macros

#include "StdAfx.h"

std::map<uint8, SpellEffectClass::pSpellEffect> SpellEffectClass::m_spellEffectMap;

SpellEffectClass::SpellEffectClass(WorldObject* caster, SpellEntry *info, uint8 castNumber, WoWGuid itemGuid) : BaseSpell(caster, info, castNumber, itemGuid)
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
    int32 value = m_spellInfo->CalculateSpellPoints(i, m_caster->getLevel(), 0);
    if( m_caster->IsUnit() )
    {
        float fValue = value;
        Unit *u_caster = castPtr<Unit>(m_caster);
        float spell_mods[2] = { 0.f, 100.f };
        u_caster->SM_FFValue(SMT_MISC_EFFECT, &spell_mods[0], m_spellInfo->SpellGroupType);
        u_caster->SM_PFValue(SMT_MISC_EFFECT, &spell_mods[1], m_spellInfo->SpellGroupType);

        if( i == 0 )
        {
            u_caster->SM_FFValue(SMT_FIRST_EFFECT_BONUS, &spell_mods[0], m_spellInfo->SpellGroupType);
            u_caster->SM_PFValue(SMT_FIRST_EFFECT_BONUS, &spell_mods[1], m_spellInfo->SpellGroupType);
        }
        else if( i == 1 )
        {
            u_caster->SM_FFValue(SMT_SECOND_EFFECT_BONUS, &spell_mods[0], m_spellInfo->SpellGroupType);
            u_caster->SM_PFValue(SMT_SECOND_EFFECT_BONUS, &spell_mods[1], m_spellInfo->SpellGroupType);
        }

        if( ( i == 2 ) || ( i == 1 && m_spellInfo->Effect[2] == 0 ) || ( i == 0 && m_spellInfo->Effect[1] == 0 && m_spellInfo->Effect[2] == 0 ) )
        {
            u_caster->SM_FFValue(SMT_LAST_EFFECT_BONUS, &spell_mods[0], m_spellInfo->SpellGroupType);
            u_caster->SM_PFValue(SMT_LAST_EFFECT_BONUS, &spell_mods[1], m_spellInfo->SpellGroupType);
        }

        fValue *= spell_mods[1];
        fValue /= 100.f;
        fValue += spell_mods[0];
        value = float2int32(fValue);
    }

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
    if(spTarget->accumAmount && unitTarget->isAlive() && (m_spellInfo->HasEffect(SPELL_EFFECT_SCHOOL_DAMAGE, spTarget->EffectMask) || m_spellInfo->HasEffect(SPELL_EFFECT_ENVIRONMENTAL_DAMAGE, spTarget->EffectMask)
        || m_spellInfo->HasEffect(SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL, spTarget->EffectMask) || m_spellInfo->HasEffect(SPELL_EFFECT_WEAPON_PERCENT_DAMAGE, spTarget->EffectMask)
        || m_spellInfo->HasEffect(SPELL_EFFECT_WEAPON_DAMAGE, spTarget->EffectMask) || m_spellInfo->HasEffect(SPELL_EFFECT_DUMMYMELEE, spTarget->EffectMask)))
    {
        if(m_caster->IsPlayer() && unitTarget->IsPlayer() && m_caster != unitTarget)
        {
            if( unitTarget->IsPvPFlagged() )
                castPtr<Player>(m_caster)->SetPvPFlag();
            if( unitTarget->IsFFAPvPFlagged() )
                castPtr<Player>(m_caster)->SetFFAPvPFlag();
        }

        if(!m_caster->IsUnit() || m_spellInfo->speed > 0 || m_spellInfo->spellType == NON_WEAPON)
            m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, spTarget->accumAmount, spTarget->resistMod, false, false);
        else castPtr<Unit>(m_caster)->Strike(unitTarget, m_spellInfo->spellType, m_spellInfo, spTarget->accumAmount, false, true);

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

    // Target alive check then trigger spell/apply aura
    if(unitTarget->isAlive() && spTarget->resistMod == 0.f)
        HandleAddAura(unitTarget);
}

void SpellEffectClass::HandleAddAura(Unit *target)
{
    AuraTargetMap::iterator itr;
    if(target == NULL || (itr = m_tempAuras.find(target->GetGUID())) == m_tempAuras.end())
        return;

    if( m_spellInfo->MechanicsType == 31 )
        target->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_ENRAGE);

    Aura *aur = itr->second;
    m_tempAuras.erase(itr);

    // did our effects kill the target?
    if( target->isDead() && !m_spellInfo->isDeathPersistentAura())
    {
        // free pointer
        target->RemoveAura(aur);
        return;
    }

    // Add the aura to our target
    target->AddAura(aur);
}

void SpellEffectClass::HandleTeleport(uint32 id, Unit* Target)
{
    if(Target == NULL || Target->GetTypeId() != TYPEID_PLAYER || !id || id == 1)
        return;

    Player* pTarget = castPtr<Player>( Target );

    uint32 mapid;
    float x,y,z,o;

    TeleportCoords* TC = TeleportCoordStorage.LookupEntry(id);
    if(TC == NULL)
    {
        switch(id)
        {
    /*  case :
            {
                mapid = ;
                x = f;
                y = f;
                z = f;
                o = 0.0f;
            }break;*/
        case 556: // Ronin effects.
        case 8690:
        case 39937:
            {
                mapid = pTarget->GetBindMapId();
                x = pTarget->GetBindPositionX();
                y = pTarget->GetBindPositionY();
                z = pTarget->GetBindPositionZ();
                o = pTarget->GetOrientation();
            }break;
        case 59901: // Portal Effect: Caverns Of Time
            {
                mapid = 1;
                x = -8164.8f;
                y = -4768.5f;
                z = 34.3f;
                o = 0.0f;
            }break;
        case 61419: // Portal Effect: The purple parlor
            {
                mapid = 571;
                x = 5848.48f;
                y = 853.706f;
                z = 843.182f;
                o = 0.0f;
            }break;
        case 61420: // Portal Effect: Violet Citadel
            {
                mapid = 571;
                x = 5819.26f;
                y = 829.774f;
                z = 680.22f;
                o = 0.0f;
            }break;

        default:
            {
                if(m_targets.m_dest.x && m_targets.m_dest.y)
                {
                    mapid = pTarget->GetMapId();
                    x = m_targets.m_dest.x;
                    y = m_targets.m_dest.y;
                    z = m_targets.m_dest.z;
                    o = pTarget->GetOrientation();
                }
                else
                {
                    sLog.outDebug("Unknown teleport spell: %u", id);
                    return;
                }
            }break;
        }
    }
    else
    {
        mapid = TC->mapId;
        x = TC->x;
        y = TC->y;
        z = TC->z;
        o = TC->o;
    }

    pTarget->EventAttackStop();
    pTarget->SetSelection(NULL);
}

void SpellEffectClass::Heal(Unit *target, uint8 effIndex, int32 amount)
{
    Unit *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if( u_caster == NULL || target == NULL || !target->isAlive() )
        return;

    //self healing shouldn't flag himself
    if( m_caster->IsPlayer() && u_caster->GetGUID() != target->GetGUID() )
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
        float critChance = u_caster->IsPlayer() ? castPtr<Player>(u_caster)->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE) : 5.f;
        if( m_spellInfo->SpellGroupType )
        {
            u_caster->SM_FFValue(SMT_CRITICAL, &critChance, m_spellInfo->SpellGroupType);
            u_caster->SM_PFValue(SMT_CRITICAL, &critChance, m_spellInfo->SpellGroupType);
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

    if( m_caster )
        SendHealSpellOnPlayer( m_caster, target, amount, critical, overheal, m_spellInfo->Id);
}

void SpellEffectClass::DetermineSkillUp(Player *target, uint32 skillid,uint32 targetlevel, uint32 multiplicator)
{
    if(target->GetSkillUpChance(skillid)<0.01f)
        return;//to preven getting higher skill than max

    int32 diff = abs(int(target->_GetSkillLineCurrent(skillid,false)/5 - targetlevel));
    float chance = ( diff <=5  ? 95.0f : diff <=10 ? 66.0f : diff <=15 ? 33.0f : 0.0f );
    if( Rand(int32(chance * sWorld.getRate(RATE_SKILLCHANCE) * (multiplicator?multiplicator:1))))
        target->_AdvanceSkillLine(skillid, float2int32(1.0f * sWorld.getRate(RATE_SKILLRATE)));
}

void SpellEffectClass::DetermineSkillUp(Player *target, uint32 skillid)
{
    float chance = 0.0f;
    SkillLineAbilityEntry* skill = objmgr.GetSpellSkill(m_spellInfo->Id);
    if( skill != NULL && target->_HasSkillLine( skill->skilline ) )
    {
        uint32 amt = target->_GetSkillLineCurrent( skill->skilline, false );
        uint32 max = target->_GetSkillLineMax( skill->skilline );
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
        target->_AdvanceSkillLine(skillid, float2int32( 1.0f * sWorld.getRate(RATE_SKILLRATE)));
}

void SpellEffectClass::InitializeSpellEffectClass()
{
    m_spellEffectMap[SPELL_EFFECT_NULL]                         = &SpellEffectClass::SpellEffectNULL;
    m_spellEffectMap[SPELL_EFFECT_INSTANT_KILL]                 = &SpellEffectClass::SpellEffectInstantKill; // 1
    m_spellEffectMap[SPELL_EFFECT_SCHOOL_DAMAGE]                = &SpellEffectClass::SpellEffectSchoolDMG; // 2
    m_spellEffectMap[SPELL_EFFECT_DUMMY]                        = &SpellEffectClass::SpellEffectDummy; // 3
    m_spellEffectMap[SPELL_EFFECT_TELEPORT_UNITS]               = &SpellEffectClass::SpellEffectTeleportUnits; // 5
    m_spellEffectMap[SPELL_EFFECT_APPLY_AURA]                   = &SpellEffectClass::SpellEffectApplyAura; // 6
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
    m_spellEffectMap[SPELL_EFFECT_WEAPON]                       = &SpellEffectClass::SpellEffectWeapon; // 25
    m_spellEffectMap[SPELL_EFFECT_PERSISTENT_AREA_AURA]         = &SpellEffectClass::SpellEffectPersistentAA; // 27
    m_spellEffectMap[SPELL_EFFECT_SUMMON]                       = &SpellEffectClass::SpellEffectSummon; // 28
    m_spellEffectMap[SPELL_EFFECT_LEAP]                         = &SpellEffectClass::SpellEffectLeap; // 29
    m_spellEffectMap[SPELL_EFFECT_ENERGIZE]                     = &SpellEffectClass::SpellEffectEnergize; // 30
    m_spellEffectMap[SPELL_EFFECT_WEAPON_PERCENT_DAMAGE]        = &SpellEffectClass::SpellEffectWeaponDmgPerc; // 31
    m_spellEffectMap[SPELL_EFFECT_TRIGGER_MISSILE]              = &SpellEffectClass::SpellEffectTriggerMissile; // 32
    m_spellEffectMap[SPELL_EFFECT_OPEN_LOCK]                    = &SpellEffectClass::SpellEffectOpenLock; // 33
    m_spellEffectMap[SPELL_EFFECT_TRANSFORM_ITEM]               = &SpellEffectClass::SpellEffectTranformItem; // 34
    m_spellEffectMap[SPELL_EFFECT_APPLY_AREA_AURA]              = &SpellEffectClass::SpellEffectApplyAA; // 35
    m_spellEffectMap[SPELL_EFFECT_LEARN_SPELL]                  = &SpellEffectClass::SpellEffectLearnSpell; // 36
    m_spellEffectMap[SPELL_EFFECT_DISPEL]                       = &SpellEffectClass::SpellEffectDispel; // 38
    m_spellEffectMap[SPELL_EFFECT_LANGUAGE]                     = &SpellEffectClass::SpellEffectLanguage; // 39
    m_spellEffectMap[SPELL_EFFECT_DUAL_WIELD]                   = &SpellEffectClass::SpellEffectDualWield; // 40
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
    m_spellEffectMap[SPELL_EFFECT_APPLY_RAID_AURA]              = &SpellEffectClass::SpellEffectApplyAA; // 65
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
    m_spellEffectMap[SPELL_EFFECT_APPLY_PET_AURA]               = &SpellEffectClass::SpellEffectApplyAura; // 119
    m_spellEffectMap[SPELL_EFFECT_DUMMYMELEE]                   = &SpellEffectClass::SpellEffectDummyMelee; // 121
    m_spellEffectMap[SPELL_EFFECT_SEND_TAXI]                    = &SpellEffectClass::SpellEffectSendTaxi; // 123
    m_spellEffectMap[SPELL_EFFECT_PLAYER_PULL]                  = &SpellEffectClass::SpellEffectPull; // 124
    m_spellEffectMap[SPELL_EFFECT_SPELL_STEAL]                  = &SpellEffectClass::SpellEffectSpellSteal; // 126
    m_spellEffectMap[SPELL_EFFECT_PROSPECTING]                  = &SpellEffectClass::SpellEffectProspecting; // 127
    m_spellEffectMap[SPELL_EFFECT_APPLY_AREA_AURA_FRIEND]       = &SpellEffectClass::SpellEffectApplyAA; // 128
    m_spellEffectMap[SPELL_EFFECT_APPLY_AREA_AURA_ENEMY]        = &SpellEffectClass::SpellEffectApplyAA; // 129
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
    m_spellEffectMap[SPELL_EFFECT_APPLY_DEMON_AURA]             = &SpellEffectClass::SpellEffectApplyDemonAura; // 143
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
    m_spellEffectMap[SPELL_EFFECT_REMOVE_TARGET_AURA]           = &SpellEffectClass::SpellEffectRemoveAura;
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
    uint32 spellId = m_spellInfo->Id;
    switch(spellId)
    {
    case 7814: case 7815: case 7816: case 7876: case 7877: case 7878:
    case 11778: case 11779: case 11780: case 15968: case 15969: case 18128:
    case 18129: case 20398: case 20399: case 20400: case 20401: case 20402:
        break;

    case 18788: //Demonic Sacrifice (508745)
        {
            uint32 spellid1 = 0;
            switch(unitTarget->GetEntry())
            {
            case 416: //Imp
                spellid1 = 18789;
                break;
            case 417: //Felhunter
                spellid1 = 18792;
                break;
            case 1860: //VoidWalker
                spellid1 = 18790;
                break;
            case 1863: //Succubus
                spellid1 = 18791;
                break;
            case 17252: //felguard
                spellid1 = 35701;
                break;
            }

            if (spellid1) //now caster gains this buff
                m_caster->CastSpell(m_caster, dbcSpell.LookupEntry(spellid1), true);
        }break;

    case 7812: //Sacrifice Voidwalker
    case 19438:
    case 19440:
    case 19441:
    case 19442:
    case 19443:
    case 27273:
        {
            if( unitTarget->GetEntry() != 1860 )
                return;
        }break;
    case 29364: //Encapsulate Voidwalker
        {
            if( unitTarget->GetEntry() != 16975 )
                    return;
        }break;
    case 33974: //Power Burn for each Point consumed mana (Effect1) target get damage(Effect3) no better idea :P
        {
            m_caster->DealDamage(unitTarget, (unitTarget->getPowerType() == POWER_TYPE_RAGE ? m_spellInfo->EffectBasePoints[0] : m_spellInfo->EffectBasePoints[1]), 0, 0, spellId);
            return;
        }break;
    case 36484: //Mana Burn same like Power Burn
        {
            m_caster->DealDamage(unitTarget, m_spellInfo->EffectBasePoints[0], 0, 0, spellId);
            return;
        }break;
    case 37056: //Kill Legion Hold Infernals
        {
            if( unitTarget->GetEntry() != 21316 )
                return;
        }break;
    case 40105: //Infusion (don't know why this should kill anything makes no sence)
        {
            return;
        }break;
    case 43135: //Bubbling Ooze
        {
            return;
        }break;
    case 41626: //Destroy Spirit
    case 44659:
        {
            if( unitTarget->GetEntry() != 23109 )
                return;
        }break;
    case 45259: //Karazhan - Kill Chest Bunny
        {
            if( unitTarget->GetEntry() != 25213 )
                return;
        }break;
    case 48743:
        {
            return; //We do not want the generated targets!!!!!!!!!!
        }break;

    }

    if(m_caster->IsPlayer() && castPtr<Player>(m_caster)->GetSession()->GetPermissionCount() == 0)
        return;

    m_caster->DealDamage(unitTarget, unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH), 0, 0, 0);
    WorldPacket data(SMSG_SPELLINSTAKILLLOG, 200);
    data << m_caster->GetGUID() << unitTarget->GetGUID() << spellId;
    m_caster->SendMessageToSet(&data, true);
}

void SpellEffectClass::SpellEffectSchoolDMG(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // dmg school
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    if((target->IsUnit() && !castPtr<Unit>(target)->isAlive()) || spTarget == NULL)
        return;

    Unit *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(u_caster && unitTarget && rawAmt)
        amount = u_caster->GetSpellBonusDamage(unitTarget, m_spellInfo, i, amount, false);

    // Add to our accumulative damage
    spTarget->accumAmount += amount;
}

void SpellEffectClass::SpellEffectDummy(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Dummy(Scripted events)
{
    if(sSpellMgr.HandleDummyEffect(this, i, m_caster, target, amount))
        return;

    // Heal: 47633, 47632
    sLog.outDebug("Dummy spell not handled: %u\n", m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectTeleportUnits(uint32 i, WorldObject *target, int32 amount, bool rawAmt)  // Teleport Units
{

}

void SpellEffectClass::SpellEffectApplyAura(uint32 i, WorldObject *target, int32 amount, bool rawAmt)  // Apply Aura
{
    if(!target->IsUnit())
        return;
    Unit *unitTarget = castPtr<Unit>(target);

    //Aura Immune Flag Check
    if ( unitTarget->IsCreature() )
    {
        if(Creature* c = castPtr<Creature>( unitTarget ))
        {
            if(c->GetCreatureData()->auraMechanicImmunity)
            {
                if(c->GetCreatureData()->auraMechanicImmunity & (uint32(1)<<m_spellInfo->MechanicsType))
                    return;
            }
        }
    }

    // Aura Mastery + Aura Of Concentration = No Interrupting effects
    if(m_spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SILENCE && unitTarget->HasAura(31821) && unitTarget->HasAura(19746))
        return;

    if( unitTarget->isDead() && !m_spellInfo->isDeathPersistentAura() )
        return;

    // avoid map corruption.
    if(unitTarget->GetInstanceID()!=m_caster->GetInstanceID())
        return;

    //check if we already have stronger aura
    Aura* pAura = NULL;
    AuraTargetMap::iterator itr;
    if((itr = m_tempAuras.find(unitTarget->GetGUID())) == m_tempAuras.end())
    {
        if(m_caster->IsGameObject() && m_caster->GetUInt32Value(GAMEOBJECT_FIELD_CREATED_BY) && castPtr<GameObject>(m_caster)->m_summoner)
            pAura = new Aura(m_spellInfo, castPtr<GameObject>(m_caster)->m_summoner, unitTarget);
        else pAura = new Aura(m_spellInfo, m_caster, unitTarget);
        m_tempAuras.insert(std::make_pair(unitTarget->GetGUID(), pAura));
    } else pAura = itr->second;

    pAura->AddMod(i, m_spellInfo->EffectApplyAuraName[i], amount);
}

void SpellEffectClass::SpellEffectPowerDrain(uint32 i, WorldObject *target, int32 amount, bool rawAmt)  // Power Drain
{
    Unit *uCaster = castPtr<Unit>(m_caster);
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 powerField = UNIT_FIELD_POWERS;
    if(m_spellInfo->EffectMiscValue[i] < 5)
        powerField += m_spellInfo->EffectMiscValue[i];
    uint32 curPower = unitTarget->GetUInt32Value(powerField);
    uint32 amt = uCaster->GetSpellBonusDamage(unitTarget, m_spellInfo, i, amount, false);

    if( unitTarget->IsPlayer() )
        amt *= float2int32( 1 - ( ( castPtr<Player>(unitTarget)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE ) * 2 ) / 100.0f ) );

    if(amt > curPower)
        amt = curPower;

    unitTarget->SetUInt32Value(powerField, curPower - amt);
    uCaster->Energize(uCaster, m_spellInfo->Id, amt, m_spellInfo->EffectMiscValue[i]);
    unitTarget->SendPowerUpdate();
}

void SpellEffectClass::SpellEffectHealthLeech(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Health Leech
{
    Unit *u_caster = castPtr<Unit>(m_caster);
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 amt = std::min<uint32>(curHealth, amount);

    m_caster->DealDamage(unitTarget, amt, 0, 0, m_spellInfo->Id);

    float coef = m_spellInfo->EffectValueMultiplier[i]; // how much health is restored per damage dealt
    if( u_caster && m_spellInfo )
    {
        u_caster->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, m_spellInfo->SpellGroupType);
        u_caster->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, m_spellInfo->SpellGroupType);
    }

    amt = float2int32((float)amt * coef);

    uint32 playerCurHealth = m_caster->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 playerMaxHealth = m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    uint32 overheal = 0;
    if(playerCurHealth + amt > playerMaxHealth)
    {
        m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, playerMaxHealth);
        overheal = playerCurHealth + amt - playerMaxHealth;
    } else m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, playerCurHealth + amt);

    SendHealSpellOnPlayer(m_caster, m_caster, amt, false, overheal, m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectHeal(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Heal
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    Unit *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( spTarget == NULL || unitTarget == NULL || !unitTarget->isAlive() )
        return;
    if(u_caster != NULL && rawAmt == true)
        amount = u_caster->GetSpellBonusDamage(unitTarget, m_spellInfo, i, amount, true);
    // Add healing amount to accumulated data
    spTarget->accumAmount += amount;
}

void SpellEffectClass::SpellEffectBind(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Innkeeper Bind
{
    if(!target->IsPlayer())
        return;
    castPtr<Player>(target)->SetBindPoint(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetMapId(), target->GetZoneId());
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

}

void SpellEffectClass::SpellEffectAddExtraAttacks(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Add Extra Attacks
{

}

void SpellEffectClass::SpellEffectCreateItem(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Create item
{

}

void SpellEffectClass::SpellEffectWeapon(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if( !target->IsPlayer() )
        return;

    Player *playerTarget = castPtr<Player>(target);
    uint32 skill = 0, spell = 0;
    switch( m_spellInfo->Id )
    {
    case 201:   // one-handed swords
        {
            skill = SKILL_SWORDS;
        }break;
    case 202:   // two-handed swords
        {
            skill = SKILL_2H_SWORDS;
        }break;
    case 203:   // Unarmed
        {
            skill = SKILL_UNARMED;
        }break;
    case 199:   // two-handed maces
        {
            skill = SKILL_2H_MACES;
        }break;
    case 198:   // one-handed maces
        {
            skill = SKILL_MACES;
        }break;
    case 197:   // two-handed axes
        {
            skill = SKILL_2H_AXES;
        }break;
    case 196:   // one-handed axes
        {
            skill = SKILL_AXES;
        }break;
    case 5011: // crossbows
        {
            skill = SKILL_CROSSBOWS;
            spell = SPELL_RANGED_GENERAL;
        }break;
    case 227:   // staves
        {
            skill = SKILL_STAVES;
        }break;
    case 1180:  // daggers
        {
            skill = SKILL_DAGGERS;
        }break;
    case 200:   // polearms
        {
            skill = SKILL_POLEARMS;
        }break;
    case 15590: // fist weapons
        {
            skill = SKILL_UNARMED;
        }break;
    case 264:   // bows
        {
            skill = SKILL_BOWS;
            spell = SPELL_RANGED_GENERAL;
        }break;
    case 266: // guns
        {
            skill = SKILL_GUNS;
            spell = SPELL_RANGED_GENERAL;
        }break;
    case 2567:  // thrown
        {
            skill = SKILL_THROWN;
            spell = SPELL_RANGED_THROW;
        }break;
    case 5009:  // wands
        {
            skill = SKILL_WANDS;
            spell = SPELL_RANGED_GENERAL;
        }break;
    case 2382:  //Generic Weapon Spell
        {
            skill = SKILL_DODGE;
            spell = SPELL_ATTACK;
        }break;
    case 9125:  //Generic Block Spell
        {
            skill = SKILL_BLOCK;
        }break;
    default:
        {
            skill = 0;
            sLog.Warning("Spell","Could not determine skill for spell id %d (SPELL_EFFECT_WEAPON)", m_spellInfo->Id);
        }break;
    }

    if(skill)
    {
        if(spell)
            playerTarget->addSpell(spell);

        // if we do not have the skill line
        if(!playerTarget->_HasSkillLine(skill))
        {
            if(sWorld.StartLevel > 1)
                playerTarget->_AddSkillLine(skill, 5*sWorld.StartLevel, playerTarget->getLevel()*5);
            else
                playerTarget->_AddSkillLine(skill, 1, playerTarget->getLevel()*5);
        }
    }
}

void SpellEffectClass::SpellEffectPersistentAA(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Persistent Area Aura
{
    return;
    Unit *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(m_AreaAura || u_caster == NULL || !m_caster->IsInWorld())
        return;
    int32 dur = GetDuration();
    float r = GetRadius(i);

    DynamicObject* dynObj = m_caster->GetMapInstance()->CreateDynamicObject();
    if(m_caster->IsGameObject() && castPtr<GameObject>(m_caster)->m_summoner && !unitTarget)
    {
        dynObj->Create(castPtr<GameObject>(m_caster)->m_summoner, this, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), dur, r);
        m_AreaAura = true;
        return;
    }

    switch(m_targets.m_targetMask)
    {
    case TARGET_FLAG_SELF:
        {
            dynObj->Create(u_caster, this,  m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), dur, r);
        }break;
    case TARGET_FLAG_UNIT:
        {
            if(!unitTarget || !unitTarget->isAlive())
            {
                dynObj->Remove();
                return;
            }

            dynObj->Create(u_caster, this, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), dur, r);
        }break;
    case TARGET_FLAG_OBJECT:
        {
            if(!unitTarget || !unitTarget->isAlive())
            {
                dynObj->Remove();
                return;
            }

            dynObj->Create(u_caster, this, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), dur, r);
        }break;
    case TARGET_FLAG_SOURCE_LOCATION:
        {
            dynObj->Create(u_caster, this, m_targets.m_src.x, m_targets.m_src.y, m_targets.m_src.z, dur, r);
        }break;
    case TARGET_FLAG_DEST_LOCATION:
        {
            dynObj->Create(u_caster ? u_caster : castPtr<GameObject>(m_caster)->m_summoner, this, m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, dur, r);
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
    if(!m_caster->IsPlayer())
        return;
    Player *p_caster = castPtr<Player>(m_caster);

    float radius = GetRadius(i);

    // remove movement impeding auras
    p_caster->m_AuraInterface.RemoveAllAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);

    // just in case
    p_caster->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_STUN);
    p_caster->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_ROOT);

    if(p_caster->GetMapInstance() && !IS_INSTANCE(p_caster->GetMapId()) && p_caster->GetMapInstance()->CanUseCollision(p_caster))
    {
        float ori = m_caster->GetOrientation();
        float posX = m_caster->GetPositionX()+(radius*(cosf(ori)));
        float posY = m_caster->GetPositionY()+(radius*(sinf(ori)));
        float posZ = m_caster->GetPositionZ();

        if( sVMapInterface.GetFirstPoint(p_caster->GetMapId(), p_caster->GetInstanceID(), p_caster->GetPhaseMask(), p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ() + Player::NoseHeight(p_caster->getRace(), p_caster->getGender()), posX, posY, p_caster->GetPositionZ(), posX, posY, posZ, -1.5f) )
        {
            posZ = p_caster->GetMapHeight(posX, posY, posZ);
            float diff = fabs(fabs(posZ) - fabs(m_caster->GetPositionZ()));
            if( diff <= 10.0f)
            {
                p_caster->blinked = true;
                p_caster->blinktimer = getMSTime()+5000;
                p_caster->SafeTeleport( p_caster->GetMapId(), p_caster->GetInstanceID(), posX, posY, posZ, m_caster->GetOrientation() );
            }
        }
        else
        {
            // either no objects in the way, or no wmo height
            posZ = p_caster->GetMapHeight(posX, posY, posZ);
            float diff = fabs(fabs(posZ) - fabs(m_caster->GetPositionZ()));
            if( diff <= 10.0f)
            {
                p_caster->blinked = true;
                p_caster->blinktimer = getMSTime()+5000;
                p_caster->SafeTeleport( p_caster->GetMapId(), p_caster->GetInstanceID(), posX, posY, posZ, m_caster->GetOrientation() );
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
    p_caster->GetSession()->SendPacket(&data);
}

void SpellEffectClass::SpellEffectEnergize(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Energize
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    castPtr<Unit>(m_caster)->Energize(unitTarget, m_spellInfo->Id, amount, m_spellInfo->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectWeaponDmgPerc(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Weapon Percent damage
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || target == NULL || !m_caster->IsUnit() || m_spellInfo->spellType == NON_WEAPON)
        return;

    float minDamage = 0.f, damageDiff = 1.f;
    if(m_caster->IsCreature() || !(false/* Check to see if damage is based on weapon proto or player stat*/))
    {   // Creatures we don't have weapons so just grab our actual min and max damage
        minDamage = m_caster->GetFloatValue(UNIT_FIELD_MINDAMAGE);
        damageDiff = m_caster->GetFloatValue(UNIT_FIELD_MAXDAMAGE)-minDamage;
    }
    else // Players we grab actual weapon damage values
    {
        Player *p_caster = castPtr<Player>(m_caster);
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
    return;
    //Used by mortar team
    //Triggers area affect spell at destinatiom
    if(!m_caster->IsUnit())
        return;

    uint32 spellid = m_spellInfo->EffectTriggerSpell[i];
    if(spellid == 0)
        return;

    SpellEntry *spInfo = dbcSpell.LookupEntry(spellid);
    if(spInfo == NULL )
        return;

    // Just send this spell where he wants :S
    castPtr<Unit>(m_caster)->CastSpellAoF(m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, spInfo, true);
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

void SpellEffectClass::SpellEffectApplyAA(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Apply Area Aura
{
    if(!target->IsUnit() || m_caster != target)
        return;

    Unit *unitTarget = castPtr<Unit>(target);
    if(!unitTarget->isAlive())
        return;

    Aura* pAura = NULL;
    AuraTargetMap::iterator itr;
    if((itr = m_tempAuras.find(unitTarget->GetGUID())) == m_tempAuras.end())
    {
        if(m_caster->IsGameObject() && m_caster->GetUInt32Value(GAMEOBJECT_FIELD_CREATED_BY) && castPtr<GameObject>(m_caster)->m_summoner)
            pAura = new Aura(m_spellInfo, castPtr<GameObject>(m_caster)->m_summoner, unitTarget);
        else pAura = new Aura(m_spellInfo, m_caster, unitTarget);
        m_tempAuras.insert(std::make_pair(unitTarget->GetGUID(), pAura));
    } else pAura = itr->second;

    pAura->AddMod(i, m_spellInfo->EffectApplyAuraName[i], amount);
}

void SpellEffectClass::SpellEffectLearnSpell(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Learn Spell
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;

    if( m_spellInfo->Id == 483 || m_spellInfo->Id == 55884 )        // "Learning"
    {
        if(!m_caster->IsPlayer() || m_targets.m_itemTarget.empty())
            return;
        Player *p_caster = castPtr<Player>(m_caster);
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
    if(!m_caster->IsUnit() || unitTarget == NULL )
        return;
    Unit *u_caster = castPtr<Unit>(m_caster);

    uint32 start = 0, end = MAX_POSITIVE_AURAS;
    if(!sFactionSystem.isAttackable(u_caster,unitTarget))
        start = MAX_POSITIVE_AURAS, end = TOTAL_AURAS;

    unitTarget->m_AuraInterface.MassDispel(u_caster, i, m_spellInfo, amount, start, end);
}

void SpellEffectClass::SpellEffectLanguage(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* pPlayer = castPtr<Player>( m_caster );

    if(!pPlayer->GetSession()->HasGMPermissions())
    {
        if(pPlayer->GetTeam() == TEAM_ALLIANCE)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_COMMON ) )
                pPlayer->_AddSkillLine( SKILL_LANG_COMMON, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_COMMON );

        if(pPlayer->GetTeam() == TEAM_HORDE)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_ORCISH ) )
                pPlayer->_AddSkillLine( SKILL_LANG_ORCISH, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_ORCISH );

        if(pPlayer->getRace() == RACE_DWARF)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_DWARVEN ) )
                pPlayer->_AddSkillLine( SKILL_LANG_DWARVEN, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_DWARVEN );

        if(pPlayer->getRace() == RACE_NIGHTELF)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_DARNASSIAN ) )
                pPlayer->_AddSkillLine( SKILL_LANG_DARNASSIAN, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_DARNASSIAN );

        if(pPlayer->getRace() == RACE_UNDEAD)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_GUTTERSPEAK ) )
                pPlayer->_AddSkillLine( SKILL_LANG_GUTTERSPEAK, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_GUTTERSPEAK );

        if(pPlayer->getRace() == RACE_TAUREN)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_TAURAHE ) )
                pPlayer->_AddSkillLine( SKILL_LANG_TAURAHE, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_TAURAHE );

        if(pPlayer->getRace() == RACE_GNOME)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_GNOMISH ) )
                pPlayer->_AddSkillLine( SKILL_LANG_GNOMISH, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_GNOMISH );

        if(pPlayer->getRace() == RACE_TROLL)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_TROLL ) )
                pPlayer->_AddSkillLine( SKILL_LANG_TROLL, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_TROLL );

        if(pPlayer->getRace() == RACE_BLOODELF)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_THALASSIAN ) )
                pPlayer->_AddSkillLine( SKILL_LANG_THALASSIAN, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_THALASSIAN );

        if(pPlayer->getRace() == RACE_DRAENEI)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_DRAENEI ) )
                pPlayer->_AddSkillLine( SKILL_LANG_DRAENEI, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_DRAENEI );
    }
}

void SpellEffectClass::SpellEffectDualWield(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if(!m_caster->IsPlayer())
        return;

    Player *p_caster = castPtr<Player>(m_caster);
    if( !p_caster->_HasSkillLine( SKILL_DUAL_WIELD ) )
         p_caster->_AddSkillLine( SKILL_DUAL_WIELD, 1, 1 );
}

void SpellEffectClass::SpellEffectSkillStep(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Skill Step
{
    if(!target->IsPlayer())
        return;
    Player* ptarget = castPtr<Player>(target);
    uint32 skill = m_spellInfo->EffectMiscValue[i];
    if( skill == 242 )
        skill = SKILL_LOCKPICKING; // somehow for lockpicking misc is different than the skill :s

    SkillLineEntry* sk = NULL;
    sk = dbcSkillLine.LookupEntry( skill );
    if( sk == NULL )
        return;

    uint32 max = 1;
    switch( sk->categoryId )
    {
    case SKILL_TYPE_PROFESSION:
    case SKILL_TYPE_SECONDARY:
        max = amount * 75;
        break;
    case SKILL_TYPE_WEAPON:
        max = 5 * target->getLevel();
        break;
    case SKILL_TYPE_CLASS:
    case SKILL_TYPE_ARMOR:
        if( skill == SKILL_LOCKPICKING )
            max = amount * 75;
        break;
    default: //u cant learn other types in game
        return;
    };

    if( ptarget->_HasSkillLine( skill ) )
        ptarget->_ModifySkillMaximum( skill, max );
    else if( skill == SKILL_RIDING )
        ptarget->_AddSkillLine( skill, max, max );
    else ptarget->_AddSkillLine( skill, 1, max );

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
    case SKILL_ENGINERING:
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

    SpellItemEnchantEntry *enchant = dbcSpellItemEnchant.LookupEntry(m_spellInfo->EffectMiscValue[i]);
    if(enchant == NULL )
    {
        sLog.outError("Invalid enchantment entry %u for Spell %u", m_spellInfo->EffectMiscValue[i], m_spellInfo->Id);
        return;
    }

    //remove other perm enchantment that was enchanted by profession
    itemTarget->RemovePermanentEnchant();
    int32 Slot = itemTarget->AddEnchantment(enchant, NULL, true, true, false, PERM_ENCHANTMENT_SLOT);
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

    SpellItemEnchantEntry *enchant = dbcSpellItemEnchant.LookupEntry(m_spellInfo->EffectMiscValue[i]);
    if(enchant == NULL )
    {
        sLog.outError("Invalid enchantment entry %u for Spell %u", m_spellInfo->EffectMiscValue[i], m_spellInfo->Id);
        return;
    }

    //remove other perm enchantment that was enchanted by profession
    itemTarget->RemoveTemporaryEnchant();
    int32 Slot = itemTarget->AddEnchantment(enchant, amount, false, true, false, TEMP_ENCHANTMENT_SLOT);
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
    spTarget->accumAmount += amount;
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
    if(m_caster->IsUnit())
    {
        castPtr<Unit>(m_caster)->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, m_spellInfo->SpellGroupType);
        castPtr<Unit>(m_caster)->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, m_spellInfo->SpellGroupType);
    }
    mana = float2int32((float)mana * coef);

    m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, mana, 0.f, true,true);
}

void SpellEffectClass::SpellEffectThreat(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Threat
{

}

void SpellEffectClass::SpellEffectTriggerSpell(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Trigger Spell
{
    return;
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || m_caster == NULL )
        return;

    SpellEntry *spe = dbcSpell.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
    if(spe == NULL )
        return;

    if(m_spellInfo == spe) // Infinite loop fix.
        return;

    SpellCastTargets tgt(spe->isNotSelfTargettable() ? unitTarget->GetGUID() : m_caster->GetGUID());
    if(Spell* sp = new Spell(m_caster, spe))
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
         SendHealSpellOnPlayer( castPtr<Player>( m_caster ), castPtr<Player>(unitTarget), dif, false, 0, m_spellInfo->Id );
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
    uint32 school=0;
    if( unitTarget->GetCurrentSpell() != NULL && (unitTarget->GetCurrentSpell()->getState() == SPELL_STATE_PREPARING || unitTarget->GetCurrentSpell()->getState() == SPELL_STATE_CASTING) && unitTarget->GetCurrentSpell() != this )
    {
        school=unitTarget->GetCurrentSpell()->m_spellInfo->School;
        unitTarget->InterruptCurrentSpell();
        //if(school)//prevent from casts in this school
            //unitTarget->SchoolCastPrevent[school] = GetDuration() + getMSTime();
    }
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
    if( unitTarget == NULL || !m_caster->IsPlayer() || unitTarget->GetTypeId() != TYPEID_UNIT)
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

    castPtr<Player>(m_caster)->SendLoot(cTarget->GetGUID(), cTarget->GetMapId(), LOOTTYPE_PICKPOCKETING);
    cTarget->SetPickPocketed(true);
}

void SpellEffectClass::SpellEffectAddFarsight(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Add Farsight
{
    return;
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
    if( p_caster == NULL )
        return;

    DynamicObject* dyn = p_caster->GetMapInstance()->CreateDynamicObject();
    if(dyn == NULL)
        return;

    dyn->Create(p_caster, this, m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, GetDuration(), GetRadius(i));
    dyn->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dyn->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x80000002);
    dyn->PushToWorld(p_caster->GetMapInstance());
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
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
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
    sLog.outDebug("Unhandled Scripted Effect In Spell %u", m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectSanctuary(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Stop all attacks made to you
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL )
        return;

    // also cancel any spells we are casting
    if( unitTarget->GetCurrentSpell() != NULL && unitTarget->GetCurrentSpell() != this && unitTarget->GetCurrentSpell()->getState() == SPELL_STATE_PREPARING )
        unitTarget->InterruptCurrentSpell();
    unitTarget->smsg_AttackStop( unitTarget );

    if( unitTarget->IsPlayer() )
        castPtr<Player>(unitTarget)->EventAttackStop();
}

void SpellEffectClass::SpellEffectAddComboPoints(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Add Combo Points
{

}

void SpellEffectClass::SpellEffectDuel(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Duel
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL, *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( p_caster == NULL  || !p_caster->isAlive() || playerTarget == p_caster )
        return;

    uint32 areaId = p_caster->GetAreaId();
    AreaTableEntry * at = dbcAreaTable.LookupEntry(areaId);
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
    if (playerTarget->DuelingWith != NULL)
    {
        SendCastResult(SPELL_FAILED_TARGET_DUELING);
        return; // Already Dueling
    }

    if(playerTarget->hasGMTag() && !p_caster->GetSession()->HasPermissions())
    {
        SendCastResult(SPELL_FAILED_BAD_TARGETS);
        return;
    }

    p_caster->RequestDuel(playerTarget);
}

void SpellEffectClass::SpellEffectStuck(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL, *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
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

    if(!m_caster->GetMapInstance()->GetdbcMap()->IsContinent())
        return;

    playerTarget->SummonRequest(m_caster, m_caster->GetZoneId(), m_caster->GetMapId(), m_caster->GetInstanceID(), m_caster->GetPosition());
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
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL, *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
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
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
    if( unitTarget == NULL || !unitTarget->IsCreature() || p_caster == NULL )
        return;

    Creature* cr = castPtr<Creature>( unitTarget );
    uint32 skill = cr->GetRequiredLootSkill();

    uint32 sk = castPtr<Player>( m_caster )->_GetSkillLineCurrent( skill ), lvl = cr->getLevel();
    if( ( sk >= lvl * 5 ) || ( ( sk + 100 ) >= lvl * 10 ) )
    {
        //Fill loot for Skinning
        lootmgr.FillGatheringLoot(cr->GetLoot(), cr->GetEntry());
        castPtr<Player>( m_caster )->SendLoot( cr->GetGUID(), cr->GetMapId(), LOOTTYPE_SKINNING );

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
    Unit *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if( unitTarget == NULL || u_caster == NULL || !unitTarget->isAlive())
        return;

    if (u_caster->IsStunned() || u_caster->IsRooted() || u_caster->IsPacified() || u_caster->IsFeared())
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

    dx = x-m_caster->GetPositionX();
    dy = y-m_caster->GetPositionY();
    if(dx == 0.0f || dy == 0.0f)
        return;

    uint32 time = uint32( (sqrtf(m_caster->GetDistanceSq(x,y,z)) / ((u_caster->GetMovementInterface()->GetMoveSpeed(MOVE_SPEED_RUN) * 3.5) * 0.001f)) + 0.5);
    u_caster->SetPosition(x,y,z,0.0f);

    u_caster->addStateFlag(UF_ATTACKING);
    if(WoWGuid guid = unitTarget->GetGUID())
        u_caster->EventAttackStart(guid);
}

void SpellEffectClass::SpellEffectPlaceTotemsOnBar(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    return;
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
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
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL && m_caster->IsUnit() )
        unitTarget = castPtr<Unit>(m_caster);
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
        pullX = m_caster->GetPositionX();
        pullY = m_caster->GetPositionY();
        pullZ = m_caster->GetPositionZ();
    }

    float arc = 10.f;
    if(m_spellInfo->EffectMiscValue[i])
        arc = m_spellInfo->EffectMiscValue[i]/10.f;
    else if(m_spellInfo->EffectMiscValueB[i])
        arc = m_spellInfo->EffectMiscValueB[i]/10.f;

    uint32 time = uint32((amount / arc) * 100);
    unitTarget->SetPosition(pullX, pullY, pullZ, 0.0f);
    if(m_caster->IsUnit())
    {
        if( unitTarget->IsPvPFlagged() )
            castPtr<Unit>(m_caster)->SetPvPFlag();
        if( unitTarget->IsFFAPvPFlagged() )
            castPtr<Unit>(m_caster)->SetFFAPvPFlag();
    }
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
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    int32 sMisc = (int32)m_spellInfo->EffectMiscValue[i];
    unitTarget->m_AuraInterface.AttemptDispel(u_caster, sMisc, (unitTarget == u_caster || !sFactionSystem.isAttackable( u_caster, unitTarget )));

    if( unitTarget->IsPlayer() && m_spellInfo->NameHash == SPELL_HASH_DAZED && castPtr<Player>(unitTarget)->IsMounted() )
        castPtr<Player>(unitTarget)->Dismount();
}

void SpellEffectClass::SpellEffectSummonDeadPet(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
    if(p_caster == NULL )
        return;
}

uint32 TotemSpells[4] = { 63, 81, 82, 83 };

void SpellEffectClass::SpellEffectDestroyAllTotems(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
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
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
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
    if(!m_caster->IsUnit() || unitTarget == NULL || !unitTarget->isAlive())
        return;

    //castPtr<Creature>(unitTarget)->GetAIInterface()->AttackReaction(castPtr<Unit>(m_caster), 1, 0);
}

void SpellEffectClass::SpellEffectSkinPlayerCorpse(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectSkill(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    // Used by professions only
    // Effect should be renamed in RequireSkill
    if(!m_caster->IsPlayer())
        return;

    uint32 val = std::min<uint32>(450, amount * 75);
    if( castPtr<Player>(m_caster)->_GetSkillLineMax(m_spellInfo->EffectMiscValue[i]) >= val )
        return;

    if( m_spellInfo->EffectMiscValue[i] == SKILL_RIDING )
        castPtr<Player>(m_caster)->_AddSkillLine( m_spellInfo->EffectMiscValue[i], val, val );
    else if( castPtr<Player>(m_caster)->_HasSkillLine(m_spellInfo->EffectMiscValue[i]) )
        castPtr<Player>(m_caster)->_ModifySkillMaximum(m_spellInfo->EffectMiscValue[i], val);
    else castPtr<Player>(m_caster)->_AddSkillLine( m_spellInfo->EffectMiscValue[i], 1, val);
}

void SpellEffectClass::SpellEffectDummyMelee(uint32 i, WorldObject *target, int32 amount, bool rawAmt) // Normalized Weapon damage +
{
    SpellTarget *spTarget = GetSpellTarget(target->GetGUID());
    if(spTarget == NULL)
        return;

    spTarget->accumAmount += amount;    
}

void SpellEffectClass::SpellEffectSpellSteal(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if ( unitTarget  == NULL ||  u_caster  == NULL || !unitTarget->isAlive())
        return;

    if( u_caster->IsPlayer() && u_caster->GetGUID() != unitTarget->GetGUID() )
        if( unitTarget->IsPvPFlagged() )
            castPtr<Player>( u_caster )->PvPToggle();

    if(!sFactionSystem.isAttackable(u_caster,unitTarget))
        return;

    unitTarget->m_AuraInterface.SpellStealAuras(u_caster, amount);
}

void SpellEffectClass::SpellEffectProspecting(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

}

void SpellEffectClass::SpellEffectResurrectNew(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(u_caster == NULL)
        return;
    //base p =hp,misc mana
    if( playerTarget == NULL)
        return;
    if(playerTarget->isAlive() || !playerTarget->IsInWorld() || playerTarget->PreventRes)
        return;

    //resurr
    playerTarget->resurrector = u_caster->GetLowGUID();
    playerTarget->m_resurrectHealth = amount;
    playerTarget->m_resurrectMana = m_spellInfo->EffectMiscValue[i];
    playerTarget->m_resurrectLoction = u_caster->GetPosition();

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
    if( m_caster->IsPlayer() )
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
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if( u_caster == NULL || unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 maxMana = (uint32)unitTarget->GetMaxPower(POWER_TYPE_MANA);
    uint32 modMana = amount * maxMana / 100;

    u_caster->Energize(unitTarget, m_spellInfo->Id, modMana, POWER_TYPE_MANA);
}

void SpellEffectClass::SpellEffectRestoreHealthPct(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if( u_caster == NULL || unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 maxHp = (uint32)unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    uint32 modHp = amount * maxHp / 100;

    u_caster->Heal(unitTarget, m_spellInfo->Id, modHp);
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
    return;
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL )
        return;

    SpellEntry* TriggeredSpell = dbcSpell.LookupEntry(m_spellInfo->EffectTriggerSpell[i]);
    if( TriggeredSpell == NULL )
        return;

    if(m_spellInfo == TriggeredSpell) // Infinite loop fix.
        return;

    SpellCastTargets tgt(unitTarget->GetGUID());
    if(Spell* sp= new Spell(m_caster, TriggeredSpell))
        sp->prepare(&tgt, true);
}

void SpellEffectClass::SpellEffectJump(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if(!m_caster->IsUnit())
        return;

    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = castPtr<Unit>(m_caster);
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
        ang = atan2(y - m_caster->GetPositionY(), x - m_caster->GetPositionX());
        ang = (ang >= 0) ? ang : 2 * M_PI + ang;
    }
    else
    {
        x = unitTarget->GetPositionX();
        y = unitTarget->GetPositionY();
        z = unitTarget->GetPositionZ();
        ang = atan2(y - m_caster->GetPositionY(), x - m_caster->GetPositionX());
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
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if(!u_caster || !unitTarget)
        return;
    float x = u_caster->GetPositionX();
    float y = u_caster->GetPositionY();
    float z = u_caster->GetPositionZ();
    unitTarget->Teleport(x,y,z,u_caster->GetOrientation());
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
    if( !m_caster->IsPlayer() )
        return;

    castPtr<Player>(m_caster)->GetTalentInterface()->UnlockSpec(amount);
}

void SpellEffectClass::SpellEffectActivateTalentSpec(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{
    if( !m_caster->IsPlayer() )
        return;
    amount -= 1; // Add our negative offset

    // 0 = primary, 1 = secondary
    Player *p_caster = castPtr<Player>(m_caster);
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

void SpellEffectClass::SpellEffectApplyDemonAura(uint32 i, WorldObject *target, int32 amount, bool rawAmt)
{

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
    if( !m_caster->IsPlayer() )
        return;

    if(Quest * qst = sQuestMgr.GetQuestPointer(m_spellInfo->EffectMiscValue[i]))
        sQuestMgr.SendQuestFailed(FAILED_REASON_FAILED, qst, castPtr<Player>(m_caster));
}
