/***
 * Demonstrike Core
 */

// thank god for macros

#include "StdAfx.h"

SpellEffectClass::SpellEffectClass(WorldObject* caster, SpellEntry *info, uint8 castNumber) : BaseSpell(caster, info, castNumber)
{

}

SpellEffectClass::~SpellEffectClass()
{

}

void SpellEffectClass::Destruct()
{
    for(std::map<uint64, Aura*>::iterator itr = m_tempAuras.begin(); itr != m_tempAuras.end(); itr++)
        delete itr->second;
    m_tempAuras.clear();

    BaseSpell::Destruct();
}

int32 SpellEffectClass::CalculateEffect(uint32 i, WorldObject* target)
{
    int32 value = GetSpellProto()->CalculateSpellPoints(i, m_caster->getLevel(), 0);
    if( m_caster->IsUnit() )
    {
        Unit * u_caster = castPtr<Unit>(m_caster);
        int32 spell_mods[2] = { 0, 0 };
        u_caster->SM_FIValue(SMT_MISC_EFFECT, &spell_mods[0], GetSpellProto()->SpellGroupType);
        u_caster->SM_FIValue(SMT_MISC_EFFECT, &spell_mods[1], GetSpellProto()->SpellGroupType);

        if( i == 0 )
        {
            u_caster->SM_FIValue(SMT_FIRST_EFFECT_BONUS, &spell_mods[0], GetSpellProto()->SpellGroupType);
            u_caster->SM_FIValue(SMT_FIRST_EFFECT_BONUS, &spell_mods[1], GetSpellProto()->SpellGroupType);
        }
        else if( i == 1 )
        {
            u_caster->SM_FIValue(SMT_SECOND_EFFECT_BONUS, &spell_mods[0], GetSpellProto()->SpellGroupType);
            u_caster->SM_FIValue(SMT_SECOND_EFFECT_BONUS, &spell_mods[1], GetSpellProto()->SpellGroupType);
        }

        if( ( i == 2 ) || ( i == 1 && GetSpellProto()->Effect[2] == 0 ) || ( i == 0 && GetSpellProto()->Effect[1] == 0 && GetSpellProto()->Effect[2] == 0 ) )
        {
            u_caster->SM_FIValue(SMT_LAST_EFFECT_BONUS, &spell_mods[0], GetSpellProto()->SpellGroupType);
            u_caster->SM_FIValue(SMT_LAST_EFFECT_BONUS, &spell_mods[1], GetSpellProto()->SpellGroupType);
        }
        value += float2int32(value * float(spell_mods[1] / 100.f)) + spell_mods[0];
    }

    return value;
}

void SpellEffectClass::HandleEffects(uint32 i, WorldObject *target)
{
    uint32 effect = GetSpellProto()->Effect[i];
    if(SpellEffectClass::m_spellEffectMap.find(effect) != SpellEffectClass::m_spellEffectMap.end())
        (*this.*SpellEffectClass::m_spellEffectMap.at(effect))(i, target, CalculateEffect(i, target));
    else sLog.Error("Spell", "Unknown effect %u spellid %u", effect, GetSpellProto()->Id);
}

void SpellEffectClass::HandleAddAura(Unit *target)
{
    if(target == NULL)
        return;

    // Applying an aura to a flagged target will cause you to get flagged.
    // self casting doesnt flag himself.
    if( m_caster->IsPlayer() && m_caster->GetGUID() != target->GetGUID() && target->IsPvPFlagged() )
    {
        Player *plr = castPtr<Player>(m_caster);
        if( !plr->IsPvPFlagged() )
            plr->PvPToggle();
        else plr->SetPvPFlag();
    }

    if( GetSpellProto()->MechanicsType == 31 )
        target->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_ENRAGE);

    if(m_tempAuras.find(target->GetGUID()) != m_tempAuras.end())
    {
        Aura *aur = m_tempAuras.at(target->GetGUID());
        m_tempAuras.erase(target->GetGUID());

        // did our effects kill the target?
        if( target->isDead() && !GetSpellProto()->isDeathPersistentAura())
        {
            // free pointer
            target->RemoveAura(aur);
            return;
        }

        // Add the aura to our target
        target->AddAura(aur);
    }
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

    // We use a teleport event on this one. Reason being because of UpdateCellActivity,
    // the game object set of the updater thread WILL Get messed up if we teleport from a gameobject caster.
    if(!sEventMgr.HasEvent(pTarget, EVENT_PLAYER_TELEPORT))
        sEventMgr.AddEvent(pTarget, &Player::EventTeleport, mapid, x, y, z, o, EVENT_PLAYER_TELEPORT, 1, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void SpellEffectClass::Heal(Unit *target, uint8 effIndex, int32 amount)
{
    Unit *u_caster = castPtr<Unit>(m_caster);
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
    //int32 bonus = 0;
    if( m_caster->IsUnit() )
    {
        // All calculations are done in getspellbonusdamage
        // 3.0.2 Spellpower change: In order to keep the effective amount healed for a given spell the same, we’d expect the original coefficients to be multiplied by 1/0.532 or 1.88.
        amount = u_caster->GetSpellBonusDamage(target, m_spellInfo, effIndex, amount, true);
    }

    if(amount < 0)
        amount = 0;

    uint32 overheal = 0;
    uint32 curHealth = target->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 maxHealth = target->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if((curHealth + amount) >= maxHealth)
    {
        target->SetUInt32Value(UNIT_FIELD_HEALTH, maxHealth);
        overheal = curHealth + amount - maxHealth;
    } else target->ModUnsigned32Value(UNIT_FIELD_HEALTH, amount);

    if( m_caster )
        SendHealSpellOnPlayer( m_caster, target, amount, critical, overheal, GetSpellProto()->Id);

    if( m_caster->IsPlayer() )
    {
        castPtr<Player>(m_caster)->m_bgScore.HealingDone += amount - overheal;
        if( castPtr<Player>(m_caster)->m_bg != NULL )
            castPtr<Player>(m_caster)->m_bg->UpdatePvPData();
    }

    // add threat
    if( m_caster->IsUnit() )
    {
        if(uint32 base_threat = amount)
        {
            std::vector<Unit* > target_threat;
            for(WorldObject::InRangeSet::iterator itr = m_caster->GetInRangeUnitSetBegin(); itr != m_caster->GetInRangeUnitSetEnd(); itr++)
            {
                Unit *unit = m_caster->GetInRangeObject<Unit>(*itr);
                if(unit->GetTypeId() != TYPEID_UNIT)
                    continue;
                if(unit->GetAIInterface()->GetNextTarget() == target)
                    target_threat.push_back(unit);
            }

            if(!target_threat.empty())
            {
                /*  When a tank hold multiple mobs, the threat of a heal on the tank will be split between all the mobs.
                The exact formula is not yet known, but it is more than the Threat/number of mobs.
                So if a tank holds 5 mobs and receives a heal, the threat on each mob will be less than Threat(heal)/5.
                Current speculation is Threat(heal)/(num of mobs *2) */
                uint32 threat = base_threat / (target_threat.size() * 2);
                for(std::vector<Unit* >::iterator itr = target_threat.begin(); itr != target_threat.end(); itr++)
                    castPtr<Unit>(*itr)->GetAIInterface()->HealReaction( castPtr<Unit>(m_caster), target, threat, m_spellInfo );
            }
        }
    }
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
    SkillLineAbilityEntry* skill = objmgr.GetSpellSkill(GetSpellProto()->Id);
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

std::map<uint8, SpellEffectClass::pSpellEffect> SpellEffectClass::m_spellEffectMap;

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
    m_spellEffectMap[SPELL_EFFECT_HEALTH_FUNNEL]                = &SpellEffectClass::SpellEffectHealthFunnel; // 65
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

void SpellEffectClass::SpellEffectNULL(uint32 i, WorldObject *target, int32 amount)
{
    sLog.Debug("Spell","Unhandled spell effect %u in spell %u.", m_spellInfo->Effect[i], m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectInstantKill(uint32 i, WorldObject *target, int32 amount)
{
    if(!target->IsUnit() || castPtr<Unit>(target)->isAlive())
        return;

    Unit *unitTarget = castPtr<Unit>(target);
    uint32 spellId = GetSpellProto()->Id;
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
            m_caster->DealDamage(unitTarget, (unitTarget->getPowerType() == POWER_TYPE_RAGE ? GetSpellProto()->EffectBasePoints[0] : GetSpellProto()->EffectBasePoints[1]), 0, 0, spellId);
            return;
        }break;
    case 36484: //Mana Burn same like Power Burn
        {
            m_caster->DealDamage(unitTarget, GetSpellProto()->EffectBasePoints[0], 0, 0, spellId);
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
            if(!m_caster->IsPlayer() || castPtr<Player>(m_caster)->GetSummon() == NULL)
                return;
            Pet *summon = castPtr<Player>(m_caster)->GetSummon();
            m_caster->DealDamage(summon, summon->GetUInt32Value(UNIT_FIELD_HEALTH), 0, 0, 0);

            WorldPacket data(SMSG_SPELLINSTAKILLLOG, 200);
            data << m_caster->GetGUID() << summon->GetGUID() << spellId;
            m_caster->SendMessageToSet(&data, true);
            return; //We do not want the generated targets!!!!!!!!!!
        }break;

    }

    switch( GetSpellProto()->NameHash )
    {
    case SPELL_HASH_SACRIFICE:
        {
            if( !m_caster->IsPet() )
                return;

            castPtr<Pet>(m_caster)->Dismiss( true );
            return;
        }break;
    default:
        {
            if( (m_caster->IsPlayer() && castPtr<Player>(m_caster)->GetSession()->GetPermissionCount() == 0) || m_caster->IsPet())
                return;
        }break;
    }
    m_caster->DealDamage(unitTarget, unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH), 0, 0, 0);
    WorldPacket data(SMSG_SPELLINSTAKILLLOG, 200);
    data << m_caster->GetGUID() << unitTarget->GetGUID() << spellId;
    m_caster->SendMessageToSet(&data, true);
}

void SpellEffectClass::SpellEffectSchoolDMG(uint32 i, WorldObject *target, int32 amount) // dmg school
{
    if(!target->IsUnit() || !castPtr<Unit>(target)->isAlive())
        return;

    Unit *unitTarget = castPtr<Unit>(target);
    if(m_caster->IsPlayer() && unitTarget->IsPlayer() && m_caster != unitTarget)
    {
        if( unitTarget->IsPvPFlagged() )
            castPtr<Player>(m_caster)->SetPvPFlag();
        if( unitTarget->IsFFAPvPFlagged() )
            castPtr<Player>(m_caster)->SetFFAPvPFlag();
    }

    // check for no more damage left (chains)
    if (amount < 0)
        return;

    if(GetSpellProto()->speed > 0)
        m_caster->SpellNonMeleeDamageLog(unitTarget, GetSpellProto()->Id, amount, false, false);
    else
    {
        if(false)
        {
            m_caster->SpellNonMeleeDamageLog(unitTarget, GetSpellProto()->Id, amount, false, false);
        }
        else if (m_caster->IsUnit())
        {
            uint32 _type = MELEE;
            if( false )
                _type = RANGED;
            else if (GetSpellProto()->reqOffHandWeapon())
                _type =  OFFHAND;
            castPtr<Unit>(m_caster)->Strike(unitTarget, _type, GetSpellProto(), i, 0, 0, amount, false, true);
        }
    }
}

void SpellEffectClass::SpellEffectDummy(uint32 i, WorldObject *target, int32 amount) // Dummy(Scripted events)
{
    sLog.outDebug("Dummy spell not handled: %u\n", m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectTeleportUnits(uint32 i, WorldObject *target, int32 amount)  // Teleport Units
{
    uint32 spellId = GetSpellProto()->Id;

    if (target == NULL)
        return;

    // Shadowstep
    if( (GetSpellProto()->NameHash == SPELL_HASH_SHADOWSTEP) && m_caster->IsPlayer() && m_caster->IsInWorld() )
    {
        // this is rather tricky actually. we have to calculate the orientation of the creature/player, and then calculate a little bit of distance behind that.
        float ang;
        WorldObject* pTarget = target;
        if( pTarget == m_caster )
        {
            // try to get a selection
            pTarget = m_caster->GetMapInstance()->GetUnit(castPtr<Player>(m_caster)->GetSelection());
            if( (pTarget == NULL ) || !sFactionSystem.isAttackable(m_caster, pTarget, !GetSpellProto()->isSpellStealthTargetCapable() ) || (pTarget->CalcDistance(m_caster) > 30.0f))
                return;
        }

        if( pTarget->GetTypeId() == TYPEID_UNIT )
        {
            if( pTarget->GetUInt64Value( UNIT_FIELD_TARGET ) != 0 )
            {
                // We're chasing a target. We have to calculate the angle to this target, this is our orientation.
                ang = m_caster->calcAngle(m_caster->GetPositionX(), m_caster->GetPositionY(), pTarget->GetPositionX(), pTarget->GetPositionY());

                // convert degree angle to radians
                ang = ang * float(M_PI) / 180.0f;
            }
            else
            {
                // Our orientation has already been set.
                ang = target->GetOrientation();
            }
        }
        else
        {
            // Players orientation is sent in movement packets
            ang = pTarget->GetOrientation();
        }

        // avoid teleporting into the model on scaled models
        const static float shadowstep_distance = 1.6f * GetDBCScale( dbcCreatureDisplayInfo.LookupEntry( target->GetUInt32Value(UNIT_FIELD_DISPLAYID)));
        float new_x = pTarget->GetPositionX() - (shadowstep_distance * cosf(ang));
        float new_y = pTarget->GetPositionY() - (shadowstep_distance * sinf(ang));
        float new_z = pTarget->GetCHeightForPosition(true);
        // Send a movement packet to "charge" at this target. Similar to warrior charge.
        castPtr<Player>(m_caster)->z_axisposition = 0.0f;
        castPtr<Player>(m_caster)->SafeTeleport(m_caster->GetMapId(), m_caster->GetInstanceID(), LocationVector(new_x, new_y, new_z, pTarget->GetOrientation()));
        return;
    }

    if(target->IsPlayer())
        HandleTeleport(spellId, castPtr<Player>(target));
}

void SpellEffectClass::SpellEffectApplyAura(uint32 i, WorldObject *target, int32 amount)  // Apply Aura
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
                if(c->GetCreatureData()->auraMechanicImmunity & (uint32(1)<<GetSpellProto()->MechanicsType))
                    return;
            }
        }
    }

    // Aura Mastery + Aura Of Concentration = No Interrupting effects
    if(GetSpellProto()->EffectApplyAuraName[i] == SPELL_AURA_MOD_SILENCE && unitTarget->HasAura(31821) && unitTarget->HasAura(19746))
        return;

    if( unitTarget->isDead() && !GetSpellProto()->isDeathPersistentAura() )
        return;

    // avoid map corruption.
    if(unitTarget->GetInstanceID()!=m_caster->GetInstanceID())
        return;

    //check if we already have stronger aura
    Aura* pAura = NULL;
    if(m_tempAuras.find(unitTarget->GetGUID()) == m_tempAuras.end())
    {
        if(m_caster->IsGameObject() && m_caster->GetUInt32Value(GAMEOBJECT_FIELD_CREATED_BY) && castPtr<GameObject>(m_caster)->m_summoner)
            pAura = new Aura(GetSpellProto(), castPtr<GameObject>(m_caster)->m_summoner, unitTarget);
        else pAura = new Aura(GetSpellProto(), m_caster, unitTarget);
        m_tempAuras.insert(std::make_pair(unitTarget->GetGUID(), pAura));
    } else pAura = m_tempAuras.at(unitTarget->GetGUID());

    pAura->AddMod(i, GetSpellProto()->EffectApplyAuraName[i], amount);
}

void SpellEffectClass::SpellEffectPowerDrain(uint32 i, WorldObject *target, int32 amount)  // Power Drain
{
    Unit *uCaster = castPtr<Unit>(m_caster);
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 powerField = UNIT_FIELD_POWERS;
    if(GetSpellProto()->EffectMiscValue[i] < 5)
        powerField += GetSpellProto()->EffectMiscValue[i];
    uint32 curPower = unitTarget->GetUInt32Value(powerField);
    uint32 amt = uCaster->GetSpellBonusDamage(unitTarget, GetSpellProto(), i, amount, false);

    if( unitTarget->IsPlayer() )
        amt *= float2int32( 1 - ( ( castPtr<Player>(unitTarget)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE ) * 2 ) / 100.0f ) );

    if(amt > curPower)
        amt = curPower;

    unitTarget->SetUInt32Value(powerField, curPower - amt);
    uCaster->Energize(uCaster, GetSpellProto()->Id, amt, GetSpellProto()->EffectMiscValue[i]);
    unitTarget->SendPowerUpdate();
}

void SpellEffectClass::SpellEffectHealthLeech(uint32 i, WorldObject *target, int32 amount) // Health Leech
{
    Unit *u_caster = castPtr<Unit>(m_caster);
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 amt = std::min<uint32>(curHealth, amount);

    m_caster->DealDamage(unitTarget, amt, 0, 0, GetSpellProto()->Id);

    float coef = GetSpellProto()->EffectValueMultiplier[i]; // how much health is restored per damage dealt
    if( u_caster && GetSpellProto() )
    {
        u_caster->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, GetSpellProto()->SpellGroupType);
        u_caster->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, GetSpellProto()->SpellGroupType);
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

    SendHealSpellOnPlayer(m_caster, m_caster, amt, false, overheal, GetSpellProto()->Id);
}

void SpellEffectClass::SpellEffectHeal(uint32 i, WorldObject *target, int32 amount) // Heal
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    Heal(unitTarget, i, amount);
}

void SpellEffectClass::SpellEffectBind(uint32 i, WorldObject *target, int32 amount) // Innkeeper Bind
{
    if(!target->IsPlayer())
        return;
    castPtr<Player>(target)->SetBindPoint(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetMapId(), target->GetZoneId());
}

void SpellEffectClass::SpellEffectQuestComplete(uint32 i, WorldObject *target, int32 amount) // Quest Complete
{
    if(!target->IsPlayer())
        return;

    if(QuestLogEntry *qle = castPtr<Player>(target)->GetQuestLogForEntry(GetSpellProto()->EffectMiscValue[i]))
    {
        qle->SendQuestComplete();
        qle->SetQuestStatus(QUEST_STATUS__COMPLETE);
        qle->UpdatePlayerFields();
    }
}

void SpellEffectClass::SpellEffectWeaponDamageNoSchool(uint32 i, WorldObject *target, int32 amount) // Weapon damage + (no School)
{
    Unit *u_caster = castPtr<Unit>(m_caster);
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL  || u_caster == NULL )
        return;

    u_caster->Strike( unitTarget, (m_spellInfo->Spell_Dmg_Type == SPELL_DMG_TYPE_RANGED ? RANGED : MELEE), GetSpellProto(), i, 0, 0, 0, false, true );
}

void SpellEffectClass::SpellEffectAddExtraAttacks(uint32 i, WorldObject *target, int32 amount) // Add Extra Attacks
{

}

void SpellEffectClass::SpellEffectCreateItem(uint32 i, WorldObject *target, int32 amount) // Create item
{

}

void SpellEffectClass::SpellEffectWeapon(uint32 i, WorldObject *target, int32 amount)
{
    if( !target->IsPlayer() )
        return;

    Player *playerTarget = castPtr<Player>(target);
    uint32 skill = 0, spell = 0;
    switch( GetSpellProto()->Id )
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
            sLog.Warning("Spell","Could not determine skill for spell id %d (SPELL_EFFECT_WEAPON)", GetSpellProto()->Id);
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

void SpellEffectClass::SpellEffectPersistentAA(uint32 i, WorldObject *target, int32 amount) // Persistent Area Aura
{
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

void SpellEffectClass::SpellEffectLeap(uint32 i, WorldObject *target, int32 amount) // Leap
{
    //FIXME: check for obstacles
    if(!m_caster->IsPlayer())
        return;
    Player *p_caster = castPtr<Player>(m_caster);

    float radius = GetRadius(i);

    // remove movement impeding auras
    p_caster->m_AuraInterface.RemoveAllAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);

    if(p_caster->m_bg && !p_caster->m_bg->HasStarted())
        return;

    // just in case
    p_caster->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_STUN);
    p_caster->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_ROOT);

    if(p_caster->GetMapInstance() && !IS_INSTANCE(p_caster->GetMapId()) && p_caster->GetMapInstance()->CanUseCollision(p_caster))
    {
        float ori = m_caster->GetOrientation();
        float posX = m_caster->GetPositionX()+(radius*(cosf(ori)));
        float posY = m_caster->GetPositionY()+(radius*(sinf(ori)));
        float posZ;

        if( sVMapInterface.GetFirstPoint(p_caster->GetMapId(), p_caster->GetInstanceID(), p_caster->GetPhaseMask(), p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ() + p_caster->m_noseLevel, posX, posY, p_caster->GetPositionZ(), posX, posY, posZ, -1.5f) )
        {
            posZ = p_caster->GetCHeightForPosition(true, posX, posY, posZ);
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
            posZ = p_caster->GetCHeightForPosition(true, posX, posY);
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

void SpellEffectClass::SpellEffectEnergize(uint32 i, WorldObject *target, int32 amount) // Energize
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    castPtr<Unit>(m_caster)->Energize(unitTarget, GetSpellProto()->Id, amount, GetSpellProto()->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectWeaponDmgPerc(uint32 i, WorldObject *target, int32 amount) // Weapon Percent damage
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !m_caster->IsUnit() )
        return;

    uint32 _type = MELEE;
    if( m_spellInfo->Spell_Dmg_Type == SPELL_DMG_TYPE_RANGED )
        _type = RANGED;
    else if (GetSpellProto()->reqOffHandWeapon())
        _type = OFFHAND;

    if( m_spellInfo->Spell_Dmg_Type == SPELL_DMG_TYPE_MAGIC )
    {
        float fdmg = (float)sStatSystem.CalculateDamage( castPtr<Unit>(m_caster), unitTarget, _type, GetSpellProto() );
        uint32 dmg = float2int32(fdmg*(float(amount/100.0f)));
        castPtr<Unit>(m_caster)->SpellNonMeleeDamageLog(unitTarget, GetSpellProto()->Id, dmg, false, false);
    } else castPtr<Unit>(m_caster)->Strike( unitTarget, _type, GetSpellProto(), i, 0, amount, 0, false, false );
}

void SpellEffectClass::SpellEffectTriggerMissile(uint32 i, WorldObject *target, int32 amount) // Trigger Missile
{
    //Used by mortar team
    //Triggers area affect spell at destinatiom
    if(!m_caster->IsUnit())
        return;

    uint32 spellid = GetSpellProto()->EffectTriggerSpell[i];
    if(spellid == 0)
        return;

    SpellEntry *spInfo = dbcSpell.LookupEntry(spellid);
    if(spInfo == NULL )
        return;

    // Just send this spell where he wants :S
    castPtr<Unit>(m_caster)->CastSpellAoF(m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, spInfo, true);
}

void SpellEffectClass::SpellEffectOpenLock(uint32 i, WorldObject *target, int32 amount) // Open Lock
{

}

void SpellEffectClass::SpellEffectOpenLockItem(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectProficiency(uint32 i, WorldObject *target, int32 amount)
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

void SpellEffectClass::SpellEffectSendEvent(uint32 i, WorldObject *target, int32 amount) //Send Event
{
    sLog.outDebug("Event spell not handled: %u\n", m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectApplyAA(uint32 i, WorldObject *target, int32 amount) // Apply Area Aura
{

}

void SpellEffectClass::SpellEffectLearnSpell(uint32 i, WorldObject *target, int32 amount) // Learn Spell
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(playerTarget == 0 && unitTarget != NULL && unitTarget->IsPet())
    {
        SpellEffectLearnPetSpell(i, target, amount);
        return;
    }

    /*if( GetSpellProto()->Id == 483 || GetSpellProto()->Id == 55884 )        // "Learning"
    {
        if( i_caster == NULL || p_caster == NULL)
            return;

        uint32 spellid = 0;
        for(int i = 0; i < 5; i++)
        {
            if( i_caster->GetProto()->Spells[i].Trigger == LEARNING && i_caster->GetProto()->Spells[i].Id != 0 )
            {
                spellid = i_caster->GetProto()->Spells[i].Id;
                break;
            }
        }

        if( !spellid || !dbcSpell.LookupEntry(spellid) )
            return;

        // learn me!
        p_caster->addSpell( spellid );

        // no normal handler
        return;
    }*/

    if(playerTarget!=NULL)
    {
        /*if(u_caster && isHostile(playerTarget, u_caster))
            return;*/

        uint32 spellToLearn = GetSpellProto()->EffectTriggerSpell[i];
        playerTarget->addSpell(spellToLearn);
        //smth is wrong here, first we add this spell to player then we may cast it on player...
        SpellEntry *spellinfo = dbcSpell.LookupEntry(spellToLearn);
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
    SpellEffectLearnPetSpell(i, target, amount);
}

void SpellEffectClass::SpellEffectLearnPetSpell(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget && unitTarget->IsPet() && m_caster->IsPlayer())
    {
        if (GetSpellProto()->EffectTriggerSpell[i])
        {
            Pet* pPet = castPtr<Pet>( unitTarget );
            //if(pPet->IsSummonedPet()) castPtr<Player>(m_caster)->AddSummonSpell(unitTarget->GetEntry(), GetSpellProto()->EffectTriggerSpell[i]);

            pPet->AddSpell( dbcSpell.LookupEntry( GetSpellProto()->EffectTriggerSpell[i] ), true );
        }
    }
}

void SpellEffectClass::SpellEffectDispel(uint32 i, WorldObject *target, int32 amount) // Dispel
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(!m_caster->IsUnit() || unitTarget == NULL )
        return;
    Unit *u_caster = castPtr<Unit>(m_caster);

    uint32 start = 0, end = MAX_POSITIVE_AURAS;
    if(!sFactionSystem.isAttackable(u_caster,unitTarget))
        start = MAX_POSITIVE_AURAS, end = MAX_AURAS;

    unitTarget->m_AuraInterface.MassDispel(u_caster, i, GetSpellProto(), amount, start, end);
}

void SpellEffectClass::SpellEffectLanguage(uint32 i, WorldObject *target, int32 amount)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* pPlayer = castPtr<Player>( m_caster );

    if(!pPlayer->GetSession()->HasGMPermissions())
    {
        if(pPlayer->GetTeam() == ALLIANCE)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_COMMON ) )
                pPlayer->_AddSkillLine( SKILL_LANG_COMMON, 300, 300 );
        } else pPlayer->_RemoveSkillLine( SKILL_LANG_COMMON );

        if(pPlayer->GetTeam() == HORDE)
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

void SpellEffectClass::SpellEffectDualWield(uint32 i, WorldObject *target, int32 amount)
{
    if(!m_caster->IsPlayer())
        return;

    Player *p_caster = castPtr<Player>(m_caster);
    if( !p_caster->_HasSkillLine( SKILL_DUAL_WIELD ) )
         p_caster->_AddSkillLine( SKILL_DUAL_WIELD, 1, 1 );
}

void SpellEffectClass::SpellEffectSkillStep(uint32 i, WorldObject *target, int32 amount) // Skill Step
{
    if(!target->IsPlayer())
        return;
    Player* ptarget = castPtr<Player>(target);
    uint32 skill = GetSpellProto()->EffectMiscValue[i];
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

void SpellEffectClass::SpellEffectDetect(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSummonObject(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectEnchantItem(uint32 i, WorldObject *target, int32 amount) // Enchant Item Permanent
{

}

void SpellEffectClass::SpellEffectEnchantItemTemporary(uint32 i, WorldObject *target, int32 amount)  // Enchant Item Temporary
{

}

void SpellEffectClass::SpellEffectAddPrismaticSocket(uint32 i, WorldObject *target, int32 amount)
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

void SpellEffectClass::SpellEffectTameCreature(uint32 i, WorldObject *target, int32 amount)
{
    if(!m_caster->IsPlayer() || !target->IsCreature())
        return;

    Player *plr = castPtr<Player>(m_caster);
    Creature* tame = castPtr<Creature>(target);
    CreatureFamilyEntry *cf = dbcCreatureFamily.LookupEntry(tame->GetCreatureData()->family);
    uint8 result = SPELL_CANCAST_OK;
    if(!tame || plr->isAlive() || !tame->isAlive() || plr->getClass() != HUNTER )
        result = SPELL_FAILED_BAD_TARGETS;
    else if(!tame->GetCreatureData())
        result = SPELL_FAILED_BAD_TARGETS;
    else if(tame->GetCreatureData()->type != BEAST)
        result = SPELL_FAILED_BAD_TARGETS;
    else if(tame->getLevel() > plr->getLevel())
        result = SPELL_FAILED_HIGHLEVEL;
    else if(plr->GeneratePetNumber() == 0)
        result = SPELL_FAILED_BAD_TARGETS;
    else if(cf == NULL || (cf && !cf->skillLine[1]))
        result = SPELL_FAILED_BAD_TARGETS;
    else if(isExotic(cf->ID) && !plr->m_BeastMaster)
        result = SPELL_FAILED_BAD_TARGETS;
    else if(plr->GetSummon() || plr->GetUnstabledPetNumber())
        result = SPELL_FAILED_ALREADY_HAVE_SUMMON;

    if(result != SPELL_CANCAST_OK)
    {
        SendCastResult(result);
        return;
    }

    // Remove target
    tame->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, plr, 0);

    Pet* pPet = objmgr.CreatePet(tame->GetCreatureData());
    pPet->SetInstanceID(plr->GetInstanceID());
    pPet->SetPosition(plr->GetPosition());
    pPet->CreateAsSummon(tame, plr, NULL, NULL, 2, 0);

    // Add removal event.
    sEventMgr.AddEvent(tame, &Creature::Despawn, uint32(1), tame->GetRespawnTime(), EVENT_CORPSE_DESPAWN, 5, 0, 0);
}

void SpellEffectClass::SpellEffectSummonPet(uint32 i, WorldObject *target, int32 amount) //summon - pet
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
    if( p_caster == NULL )
        return;

    if(GetSpellProto()->Id == 883) // "Call Pet" spell
    {
        if(p_caster->GetSummon() != 0)
        {
            p_caster->GetSession()->SendNotification("You already have a pet summoned.");
            return;
        }

        if(uint8 petno = p_caster->GetUnstabledPetNumber())
            p_caster->SpawnPet(petno);
        else
        {
            WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 50);
            data << uint32(0) << "You do not have any pets to call." << uint8(0);
            p_caster->GetSession()->SendPacket(&data);
        }
        return;
    }

    //uint32 entryId = GetSpellProto()->EffectMiscValue[i];

    //VoidWalker:torment, sacrifice, suffering, consume shadows
    //Succubus:lash of pain, soothing kiss, seduce , lesser invisibility
    //felhunter:     Devour Magic,Paranoia,Spell Lock,  Tainted Blood
    if( p_caster->getClass() != WARLOCK)
        return;

    // remove old pet
    if(Pet* old = castPtr<Player>(m_caster)->GetSummon())
        old->Dismiss(false);

    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(GetSpellProto()->EffectMiscValue[i]);
    if(ctrData == NULL)
        return;

    //if demonic sacrifice auras are still active, remove them
    uint32 spids[5] = { 18789, 18790, 18791, 18792, 35701 };
    for(uint8 i = 0; i < 5; i++)
        p_caster->RemoveAura(spids[i]);

    Pet* summon = objmgr.CreatePet(ctrData);
    summon->SetInstanceID(m_caster->GetInstanceID());
    summon->SetPosition(m_caster->GetPosition());
    summon->CreateAsSummon(NULL, p_caster, NULL, GetSpellProto(), 1, 0);
    if( p_caster->IsPvPFlagged() )
        summon->SetPvPFlag();
}

void SpellEffectClass::SpellEffectWeaponDamage(uint32 i, WorldObject *target, int32 amount) // Weapon damage +
{
    Unit *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || u_caster == NULL )
        return;

    uint32 _type = GetSpellProto()->reqOffHandWeapon() ? OFFHAND : MELEE;
    if( m_spellInfo->Spell_Dmg_Type == SPELL_DMG_TYPE_RANGED )
        _type = RANGED;
    u_caster->Strike( unitTarget, _type, GetSpellProto(), i, amount, 0, 0, false, true );
}

void SpellEffectClass::SpellEffectPowerBurn(uint32 i, WorldObject *target, int32 amount) // power burn
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

    float coef = GetSpellProto()->EffectValueMultiplier[i]; // damage per mana burned
    if(m_caster->IsUnit())
    {
        castPtr<Unit>(m_caster)->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, GetSpellProto()->SpellGroupType);
        castPtr<Unit>(m_caster)->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, GetSpellProto()->SpellGroupType);
    }
    mana = float2int32((float)mana * coef);

    m_caster->SpellNonMeleeDamageLog(unitTarget,GetSpellProto()->Id, mana, true,true);
}

void SpellEffectClass::SpellEffectThreat(uint32 i, WorldObject *target, int32 amount) // Threat
{

}

void SpellEffectClass::SpellEffectTriggerSpell(uint32 i, WorldObject *target, int32 amount) // Trigger Spell
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || m_caster == NULL )
        return;

    SpellEntry *spe = dbcSpell.LookupEntry(GetSpellProto()->EffectTriggerSpell[i]);
    if(spe == NULL )
        return;

    if(GetSpellProto() == spe) // Infinite loop fix.
        return;

    SpellCastTargets tgt(spe->isNotSelfTargettable() ? unitTarget->GetGUID() : m_caster->GetGUID());
    if(Spell* sp = new Spell(m_caster, spe))
        sp->prepare(&tgt, true);
}

void SpellEffectClass::SpellEffectHealthFunnel(uint32 i, WorldObject *target, int32 amount) // Health Funnel
{

}

void SpellEffectClass::SpellEffectPowerFunnel(uint32 i, WorldObject *target, int32 amount) // Power Funnel
{

}

void SpellEffectClass::SpellEffectHealMaxHealth(uint32 i, WorldObject *target, int32 amount)   // Heal Max Health
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
         SendHealSpellOnPlayer( castPtr<Player>( m_caster ), castPtr<Player>(unitTarget), dif, false, 0, GetSpellProto()->Id );
    unitTarget->ModUnsigned32Value( UNIT_FIELD_HEALTH, dif );
}

void SpellEffectClass::SpellEffectInterruptCast(uint32 i, WorldObject *target, int32 amount) // Interrupt Cast
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
        school=unitTarget->GetCurrentSpell()->GetSpellProto()->School;
        unitTarget->InterruptCurrentSpell();
        //if(school)//prevent from casts in this school
            //unitTarget->SchoolCastPrevent[school] = GetDuration() + getMSTime();
    }
}

void SpellEffectClass::SpellEffectDistract(uint32 i, WorldObject *target, int32 amount) // Distract
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
        float newo=unitTarget->calcRadAngle(unitTarget->GetPositionX(),unitTarget->GetPositionY(),m_targets.m_dest.x,m_targets.m_dest.y);
        unitTarget->GetAIInterface()->StopMovement(Stare_duration);
        unitTarget->SetFacing(newo);
    }

    //Smoke Emitter 164870
    //Smoke Emitter Big 179066
    //Unit Field Target of
}

void SpellEffectClass::SpellEffectPickpocket(uint32 i, WorldObject *target, int32 amount) // pickpocket
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL || !m_caster->IsPlayer() || unitTarget->GetTypeId() != TYPEID_UNIT)
        return;

    //Show random loot based on roll,
    Creature* cTarget = castPtr<Creature>( unitTarget );
    if(cTarget->IsPickPocketed() || (cTarget->GetCreatureData() && cTarget->GetCreatureData()->type != HUMANOID))
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

void SpellEffectClass::SpellEffectAddFarsight(uint32 i, WorldObject *target, int32 amount) // Add Farsight
{
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

void SpellEffectClass::SpellEffectResetTalents(uint32 i, WorldObject *target, int32 amount) // Used by Trainers.
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( !playerTarget )
        return;

    playerTarget->ResetSpec(playerTarget->m_talentInterface.GetActiveSpec());
}

void SpellEffectClass::SpellEffectUseGlyph(uint32 i, WorldObject *target, int32 amount)
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
    if( p_caster == NULL )
        return;

    if(uint8 result = p_caster->m_talentInterface.ApplyGlyph(m_targets.m_targetIndex, GetSpellProto()->EffectMiscValue[i])) // there was an error
        SendCastResult(result);
    else p_caster->m_talentInterface.SendTalentInfo();
}

void SpellEffectClass::SpellEffectHealMechanical(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || unitTarget->GetCreatureType() != MECHANICAL)
        return;

    Heal(unitTarget, i, amount);
}

void SpellEffectClass::SpellEffectSummonObjectWild(uint32 i, WorldObject *target, int32 amount)
{
    Unit *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if(u_caster == NULL )
        return;

    // spawn a new one
    GameObject* GoSummon = u_caster->GetMapInstance()->CreateGameObject(GetSpellProto()->EffectMiscValue[i]);
    if( GoSummon == NULL || !GoSummon->CreateFromProto(GetSpellProto()->EffectMiscValue[i], m_caster->GetMapId(), m_caster->GetPosition()))
        return;

    GoSummon->SetUInt32Value(GAMEOBJECT_LEVEL, u_caster->getLevel());
    GoSummon->SetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY, m_caster->GetGUID());
    GoSummon->SetState(0);
    GoSummon->PushToWorld(u_caster->GetMapInstance());
    GoSummon->SetSummoned(u_caster);
    GoSummon->ExpireAndDelete(GetDuration());
}

void SpellEffectClass::SpellEffectScriptEffect(uint32 i, WorldObject *target, int32 amount) // Script Effect
{
    sLog.outDebug("Unhandled Scripted Effect In Spell %u", m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectSanctuary(uint32 i, WorldObject *target, int32 amount) // Stop all attacks made to you
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL )
        return;

    for(WorldObject::InRangeSet::iterator itr = unitTarget->GetInRangePlayerSetBegin(); itr != unitTarget->GetInRangePlayerSetEnd(); itr++)
        if(Player *player = unitTarget->GetInRangeObject<Player>(*itr))
            player->GetAIInterface()->RemoveThreat(unitTarget->GetGUID());

    // also cancel any spells we are casting
    if( unitTarget->GetCurrentSpell() != NULL && unitTarget->GetCurrentSpell() != this && unitTarget->GetCurrentSpell()->getState() == SPELL_STATE_PREPARING )
        unitTarget->InterruptCurrentSpell();
    unitTarget->smsg_AttackStop( unitTarget );

    if( unitTarget->IsPlayer() )
        castPtr<Player>(unitTarget)->EventAttackStop();
}

void SpellEffectClass::SpellEffectAddComboPoints(uint32 i, WorldObject *target, int32 amount) // Add Combo Points
{

}

void SpellEffectClass::SpellEffectDuel(uint32 i, WorldObject *target, int32 amount) // Duel
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

    if(p_caster->m_bg)
    {
        SendCastResult(SPELL_FAILED_NOT_IN_BATTLEGROUND);
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

    if(playerTarget->bGMTagOn && !p_caster->GetSession()->HasPermissions())
    {
        SendCastResult(SPELL_FAILED_BAD_TARGETS);
        return;
    }

    p_caster->RequestDuel(playerTarget);
}

void SpellEffectClass::SpellEffectStuck(uint32 i, WorldObject *target, int32 amount)
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL, *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( playerTarget == NULL || playerTarget != p_caster)
        return;

    uint32 mapid = playerTarget->GetBindMapId();
    float x = playerTarget->GetBindPositionX();
    float y = playerTarget->GetBindPositionY();
    float z = playerTarget->GetBindPositionZ();
    float orientation = 0.f;

    sEventMgr.AddEvent(playerTarget,&Player::EventTeleport,mapid,x,y,z,orientation,EVENT_PLAYER_TELEPORT,50,1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void SpellEffectClass::SpellEffectSummonPlayer(uint32 i, WorldObject *target, int32 amount)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( playerTarget == NULL)
        return;

    if(!m_caster->GetMapInstance()->GetdbcMap()->IsContinent())
        return;

    playerTarget->SummonRequest(m_caster, m_caster->GetZoneId(), m_caster->GetMapId(), m_caster->GetInstanceID(), m_caster->GetPosition());
}

void SpellEffectClass::SpellEffectActivateObject(uint32 i, WorldObject *target, int32 amount) // Activate WorldObject
{

}

void SpellEffectClass::SpellEffectWMODamage(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectWMORepair(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectChangeWMOState(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSelfResurrect(uint32 i, WorldObject *target, int32 amount)
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL, *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( p_caster == NULL || playerTarget == NULL || playerTarget->isAlive() || playerTarget->PreventRes)
        return;

    uint32 mana = 0, health = 0, class_ = playerTarget->getClass();
    switch(GetSpellProto()->Id)
    {
    case 3026:
    case 20758:
    case 20759:
    case 20760:
    case 20761:
    case 27240:
        {
            health = GetSpellProto()->EffectMiscValue[i];
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

void SpellEffectClass::SpellEffectSkinning(uint32 i, WorldObject *target, int32 amount)
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

void SpellEffectClass::SpellEffectCharge(uint32 i, WorldObject *target, int32 amount)
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

    uint32 time = uint32( (m_caster->CalcDistance(x,y,z) / ((u_caster->GetMovementInterface()->GetMoveSpeed(MOVE_SPEED_RUN) * 3.5) * 0.001f)) + 0.5);
    u_caster->GetAIInterface()->SendMoveToPacket(x, y, z, 0.0f, time, MONSTER_MOVE_FLAG_WALK);
    u_caster->SetPosition(x,y,z,0.0f);

    if(unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
        unitTarget->GetAIInterface()->StopMovement(time);

    u_caster->addStateFlag(UF_ATTACKING);
    if(WoWGuid guid = unitTarget->GetGUID())
        u_caster->EventAttackStart(guid);
    u_caster->resetAttackTimer(0xFF);
}

void SpellEffectClass::SpellEffectPlaceTotemsOnBar(uint32 i, WorldObject *target, int32 amount)
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
    if(!p_caster)
        return;

    uint32 button = 132+m_spellInfo->EffectMiscValue[i];
    uint32 button_count = m_spellInfo->EffectMiscValueB[i];
    for (uint32 slot = button; slot < button+button_count; slot++)
    {
        if(p_caster->m_talentInterface.GetActionButton(slot).GetType() != ACTION_BUTTON_SPELL)
             continue;
        uint32 spell = p_caster->m_talentInterface.GetActionButton(slot).GetAction();
        if(spell == 0)
            continue;
        SpellEntry* sp = dbcSpell.LookupEntry(spell);
        if(sp == NULL || !p_caster->HasSpell(spell))
            continue;
        if(/*!IsTotemSpell(sp) || */p_caster->SpellHasCooldown(spell))
            continue;
        Spell* pSpell = new Spell(p_caster, sp);
        if(!pSpell->HasPower())
            continue;

        SpellCastTargets targets;
        pSpell->GenerateTargets(&targets);
        pSpell->prepare(&targets, true);
    }
}

void SpellEffectClass::SpellEffectSendTaxi(uint32 i, WorldObject *target, int32 amount)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if ( playerTarget == NULL || !playerTarget->isAlive() )
        return;

    if( playerTarget->HasFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER ) )
        return;

    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath( GetSpellProto()->EffectMiscValue[i] );
    if( !taxipath )
        return;

    TaxiNodeEntry* taxinode = dbcTaxiNode.LookupEntry(taxipath->GetSourceNode());
    if( !taxinode )
        return;

    uint32 mount_entry = 0, modelid = 0;
    if( playerTarget->GetTeam() )       // HORDE
    {
        if((mount_entry = taxinode->mountIdHorde) == 0)
            mount_entry = taxinode->mountIdAlliance;

        CreatureData* ctrData = sCreatureDataMgr.GetCreatureData( mount_entry );
        if( ctrData == NULL )
            return;
        if((modelid = ctrData->displayInfo[1]) == 0)
            return;
    }
    else                                // ALLIANCE
    {
        if((mount_entry = taxinode->mountIdAlliance) == 0)
            mount_entry = taxinode->mountIdHorde;

        CreatureData* ctrData = sCreatureDataMgr.GetCreatureData( mount_entry );
        if( ctrData == NULL )
            return;
        if((modelid = ctrData->displayInfo[0]) == 0)
            return;
    }

    playerTarget->TaxiStart( taxipath, modelid, 0 );
}

void SpellEffectClass::SpellEffectPull(uint32 i, WorldObject *target, int32 amount)
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
    if(GetSpellProto()->EffectMiscValue[i])
        arc = GetSpellProto()->EffectMiscValue[i]/10.f;
    else if(GetSpellProto()->EffectMiscValueB[i])
        arc = GetSpellProto()->EffectMiscValueB[i]/10.f;

    uint32 time = uint32((amount / arc) * 100);
    unitTarget->GetAIInterface()->StopMovement(time);
    unitTarget->SetPosition(pullX, pullY, pullZ, 0.0f);
    unitTarget->GetAIInterface()->SendJumpTo(pullX, pullY, pullZ, time, arc);
    if(m_caster->IsUnit())
    {
        if( unitTarget->IsPvPFlagged() )
            castPtr<Unit>(m_caster)->SetPvPFlag();
        if( unitTarget->IsFFAPvPFlagged() )
            castPtr<Unit>(m_caster)->SetFFAPvPFlag();
    }
}

void SpellEffectClass::SpellEffectKnockBack(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    unitTarget->knockback(GetSpellProto()->EffectBasePoints[i]+1, GetSpellProto()->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectDisenchant(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectInebriate(uint32 i, WorldObject *target, int32 amount) // lets get drunk!
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

void SpellEffectClass::SpellEffectFeedPet(uint32 i, WorldObject *target, int32 amount)  // Feed Pet
{

}

void SpellEffectClass::SpellEffectReputation(uint32 i, WorldObject *target, int32 amount)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( playerTarget == NULL)
        return;

    playerTarget->ModStanding(GetSpellProto()->EffectMiscValue[i], amount);
}

void SpellEffectClass::SpellEffectSummonObjectSlot(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectDispelMechanic(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    int32 sMisc = (int32)GetSpellProto()->EffectMiscValue[i];
    unitTarget->m_AuraInterface.AttemptDispel(u_caster, sMisc, (unitTarget == u_caster || !sFactionSystem.isAttackable( u_caster, unitTarget )));

    if( unitTarget->IsPlayer() && GetSpellProto()->NameHash == SPELL_HASH_DAZED && castPtr<Player>(unitTarget)->IsMounted() )
        castPtr<Player>(unitTarget)->Dismount();
}

void SpellEffectClass::SpellEffectSummonDeadPet(uint32 i, WorldObject *target, int32 amount)
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
    if(p_caster == NULL )
        return;

    if(Pet* pPet = p_caster->GetSummon())
    {
        pPet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
        pPet->SetUInt32Value(UNIT_FIELD_HEALTH, (uint32)((pPet->GetUInt32Value(UNIT_FIELD_MAXHEALTH) * amount) / 100));
        pPet->SetDeathState(ALIVE);
        pPet->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, pPet, 0);
        sEventMgr.RemoveEvents(pPet, EVENT_PET_DELAYED_REMOVE);
    }
}

uint32 TotemSpells[4] = { 63, 81, 82, 83 };

void SpellEffectClass::SpellEffectDestroyAllTotems(uint32 i, WorldObject *target, int32 amount)
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
            for(std::set<Creature*>::iterator itr = p_caster->m_Summons[spe->slot].begin(); itr != p_caster->m_Summons[spe->slot].end(); itr++)
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
        p_caster->Energize(p_caster, GetSpellProto()->Id, energize_amt, POWER_TYPE_MANA);
}

void SpellEffectClass::SpellEffectSummonDemonOld(uint32 i, WorldObject *target, int32 amount)
{
    Player *p_caster = m_caster->IsPlayer() ? castPtr<Player>(m_caster) : NULL;
    if(p_caster == NULL ) //p_caster->getClass() != WARLOCK ) //summoning a demon shouldn't be warlock only, see spells 25005, 24934, 24810 etc etc
        return;

    Pet* pPet;
    if(pPet = p_caster->GetSummon())
        pPet->Dismiss(false);

    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(GetSpellProto()->EffectMiscValue[i]);
    if(ctrData == NULL)
        return;

    pPet = objmgr.CreatePet(ctrData);
    pPet->SetInstanceID(p_caster->GetInstanceID());
    pPet->CreateAsSummon(NULL, p_caster, NULL, GetSpellProto(), 1, 300000);

    SpellEntry *spellInfo = dbcSpell.LookupEntry(11726);
    //Create Enslave Aura if its inferno spell
    if(spellInfo && GetSpellProto()->Id == 1122)
    {
        SpellCastTargets tgt(pPet->GetGUID());
        if(Spell* sp = new Spell(pPet, spellInfo))
            sp->prepare(&tgt, true);
    }
}

void SpellEffectClass::SpellEffectResurrect(uint32 i, WorldObject *target, int32 amount) // Resurrect (Flat)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(playerTarget == NULL || playerTarget->isAlive() || !playerTarget->IsInWorld() || playerTarget->PreventRes)
        return;

    uint32 health = GetSpellProto()->EffectBasePoints[i];
    uint32 mana = GetSpellProto()->EffectMiscValue[i];

    playerTarget->m_resurrectHealth = health;
    playerTarget->m_resurrectMana = mana;

    SendResurrectRequest(playerTarget);
}

void SpellEffectClass::SpellEffectAttackMe(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(!m_caster->IsUnit() || unitTarget == NULL || !unitTarget->isAlive())
        return;

    unitTarget->GetAIInterface()->AttackReaction(castPtr<Unit>(m_caster), 1, 0);
}

void SpellEffectClass::SpellEffectSkinPlayerCorpse(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSkill(uint32 i, WorldObject *target, int32 amount)
{
    // Used by professions only
    // Effect should be renamed in RequireSkill
    if(!m_caster->IsPlayer())
        return;

    uint32 val = std::min<uint32>(450, amount * 75);
    if( castPtr<Player>(m_caster)->_GetSkillLineMax(GetSpellProto()->EffectMiscValue[i]) >= val )
        return;

    if( GetSpellProto()->EffectMiscValue[i] == SKILL_RIDING )
        castPtr<Player>(m_caster)->_AddSkillLine( GetSpellProto()->EffectMiscValue[i], val, val );
    else if( castPtr<Player>(m_caster)->_HasSkillLine(GetSpellProto()->EffectMiscValue[i]) )
        castPtr<Player>(m_caster)->_ModifySkillMaximum(GetSpellProto()->EffectMiscValue[i], val);
    else castPtr<Player>(m_caster)->_AddSkillLine( GetSpellProto()->EffectMiscValue[i], 1, val);
}

void SpellEffectClass::SpellEffectDummyMelee(uint32 i, WorldObject *target, int32 amount) // Normalized Weapon damage +
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if( unitTarget == NULL || u_caster == NULL )
        return;

    uint32 pct_dmg_mod = 100;
    if( u_caster->IsPlayer() && GetSpellProto()->NameHash == SPELL_HASH_OVERPOWER) //warrior : overpower - let us clear the event and the combopoint count
    {
        //castPtr<Player>(m_caster)->NullComboPoints(); //some say that we should only remove 1 point per dodge. Due to cooldown you can't cast it twice anyway..
    }
    else if( GetSpellProto()->NameHash == SPELL_HASH_DEVASTATE)
    {
        // Player can apply only 58567 Sunder Armor effect.
        if(Aura* aura = u_caster->m_AuraInterface.FindActiveAura(58567))
        {
            aura->AddStackSize(u_caster->HasAura(58388) ? 2 : 1);
            amount *= aura->getStackSize();
        } else u_caster->CastSpell(unitTarget, 58567, true);
    }
    // rogue - mutilate ads dmg if target is poisoned
    // pure hax (damage join)
    else if( GetSpellProto()->NameHash == SPELL_HASH_MUTILATE )
    {
        amount *= 2;
        amount += sStatSystem.CalculateDamage(u_caster, unitTarget, MELEE, GetSpellProto());
        amount += sStatSystem.CalculateDamage(u_caster, unitTarget, OFFHAND, GetSpellProto());

        if( unitTarget->IsPoisoned() )
            pct_dmg_mod = 120;
    } //Hemorrhage
    else if( u_caster->IsPlayer() && GetSpellProto()->NameHash == SPELL_HASH_HEMORRHAGE )
        ;//p_caster->AddComboPoints(p_caster->GetSelection(), 1);

    uint32 _type = m_spellInfo->Spell_Dmg_Type == SPELL_DMG_TYPE_RANGED ? RANGED : GetSpellProto()->reqOffHandWeapon() ? OFFHAND : MELEE;
    u_caster->Strike( unitTarget, _type, GetSpellProto(), i, amount, pct_dmg_mod, 0, false, false );
}

void SpellEffectClass::SpellEffectSpellSteal(uint32 i, WorldObject *target, int32 amount)
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

void SpellEffectClass::SpellEffectProspecting(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectResurrectNew(uint32 i, WorldObject *target, int32 amount)
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
    playerTarget->m_resurrectMana = GetSpellProto()->EffectMiscValue[i];
    playerTarget->m_resurrectLoction = u_caster->GetPosition();

    SendResurrectRequest(playerTarget);
}

void SpellEffectClass::SpellEffectTranformItem(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectEnvironmentalDamage(uint32 i, WorldObject *target, int32 amount)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(playerTarget == NULL)
        return;

    if(false)//playerTarget->SchoolImmunityList[GetSpellProto()->School])
    {
        SendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }

    //this is GO, not unit
    m_caster->SpellNonMeleeDamageLog(playerTarget, GetSpellProto()->Id, amount, true);

    WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, 13);
    data << playerTarget->GetGUID();
    data << uint8(DAMAGE_FIRE);
    data << uint32(amount);
    playerTarget->SendMessageToSet( &data, true );
}

void SpellEffectClass::SpellEffectDismissPet(uint32 i, WorldObject *target, int32 amount)
{
    // remove pet.. but don't delete so it can be called later
    if( m_caster->IsPlayer() )
        return;

    if(Pet* pPet = castPtr<Player>(m_caster)->GetSummon())
        pPet->Remove(true, true, true);
}

void SpellEffectClass::SpellEffectEnchantHeldItem(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectAddHonor(uint32 i, WorldObject *target, int32 amount)
{
    if( !target->IsPlayer() )
        return;

    HonorHandler::AddHonorPointsToPlayer( castPtr<Player>(target), GetSpellProto()->EffectBasePoints[i] );
}

void SpellEffectClass::SpellEffectSpawn(uint32 i, WorldObject *target, int32 amount)
{
    // this effect is mostly for custom teleporting
    switch(GetSpellProto()->Id)
    {
        case 10418: // Arugal spawn-in spell , teleports it to 3 locations randomly sneeking players (bastard ;P)
        {
            if(!m_caster->IsCreature())
                return;

            static float coord[3][3]= {{-108.9034f,2129.5678f,144.9210f},{-108.9034f,2155.5678f,155.678f},{-77.9034f,2155.5678f,155.678f}};

            int i = (int)(rand()%3);
            castPtr<Creature>(m_caster)->GetAIInterface()->SendMoveToPacket(coord[i][0], coord[i][1], coord[i][2], 0.0f, 0, castPtr<Creature>(m_caster)->GetAIInterface()->getMoveFlags());
        }
    }
}

void SpellEffectClass::SpellEffectRedirectThreat(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectPlayMusic(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(!unitTarget)
        return;

    unitTarget->PlaySoundToSet(m_spellInfo->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectKillCredit(uint32 i, WorldObject *target, int32 amount)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(playerTarget == NULL)
        return;

    if(CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(GetSpellProto()->EffectMiscValue[i]))
        sQuestMgr._OnPlayerKill( playerTarget, GetSpellProto()->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectRestoreManaPct(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if( u_caster == NULL || unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 maxMana = (uint32)unitTarget->GetMaxPower(POWER_TYPE_MANA);
    uint32 modMana = amount * maxMana / 100;

    u_caster->Energize(unitTarget, GetSpellProto()->Id, modMana, POWER_TYPE_MANA);
}

void SpellEffectClass::SpellEffectRestoreHealthPct(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if( u_caster == NULL || unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 maxHp = (uint32)unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    uint32 modHp = amount * maxHp / 100;

    u_caster->Heal(unitTarget, GetSpellProto()->Id, modHp);
}

void SpellEffectClass::SpellEffectForceCast(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL )
        return;

    SpellEntry* TriggeredSpell = dbcSpell.LookupEntry( GetSpellProto()->EffectTriggerSpell[i] );
    if( TriggeredSpell == NULL )
        return;

    if(GetSpellProto() == TriggeredSpell) // Infinite loop fix.
        return;

    unitTarget->CastSpell( unitTarget, TriggeredSpell, true );
}

void SpellEffectClass::SpellEffectTriggerSpellWithValue(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( unitTarget == NULL )
        return;

    SpellEntry* TriggeredSpell = dbcSpell.LookupEntry(GetSpellProto()->EffectTriggerSpell[i]);
    if( TriggeredSpell == NULL )
        return;

    if(GetSpellProto() == TriggeredSpell) // Infinite loop fix.
        return;

    SpellCastTargets tgt(unitTarget->GetGUID());
    if(Spell* sp= new Spell(m_caster, TriggeredSpell))
        sp->prepare(&tgt, true);
}

void SpellEffectClass::SpellEffectJump(uint32 i, WorldObject *target, int32 amount)
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
    if(GetSpellProto()->EffectMiscValue[i])
        arc = GetSpellProto()->EffectMiscValue[i]/10;
    else if(GetSpellProto()->EffectMiscValueB[i])
        arc = GetSpellProto()->EffectMiscValueB[i]/10;
    else arc = 10.0f;

    uint32 time = uint32((amount / arc) * 100);
    u_caster->GetAIInterface()->StopMovement(time);
    u_caster->SetPosition(x, y, z, ang);
    u_caster->GetAIInterface()->SendJumpTo(x, y, z, time, arc);
}

void SpellEffectClass::SpellEffectTeleportToCaster(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL, *u_caster = m_caster->IsUnit() ? castPtr<Unit>(m_caster) : NULL;
    if(!u_caster || !unitTarget)
        return;
    float x = u_caster->GetPositionX();
    float y = u_caster->GetPositionY();
    float z = u_caster->GetPositionZ();
    unitTarget->Teleport(x,y,z,u_caster->GetOrientation());
}

void SpellEffectClass::SpellEffectMilling(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectAllowPetRename(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if( !unitTarget || !unitTarget->IsPet() )
        return;

    unitTarget->SetByte( UNIT_FIELD_BYTES_2, 2, 0x03);
}

void SpellEffectClass::SpellEffectStartQuest(uint32 i, WorldObject *target, int32 amount)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( !playerTarget )
        return;

    playerTarget->StartQuest(GetSpellProto()->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectCreatePet(uint32 i, WorldObject *target, int32 amount)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if( !playerTarget )
        return;

    if( playerTarget->getClass() != HUNTER )
        return;

    if( playerTarget->GetSummon() )
        playerTarget->GetSummon()->Remove( true, true, true );

    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData( GetSpellProto()->EffectMiscValue[i] );
    if( ctrData != NULL )
    {
        if(Pet *pPet = objmgr.CreatePet(ctrData))
        {
            pPet->CreateAsSummon( NULL, playerTarget, NULL, GetSpellProto(), 2, 0 );
            if(!pPet->IsInWorld())
                pPet->Destruct();
        }
    }
}

void SpellEffectClass::SpellEffectTitanGrip(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectCreateRandomItem(uint32 i, WorldObject *target, int32 amount) // Create Random Item
{

}

void SpellEffectClass::SpellEffectSetTalentSpecsCount(uint32 i, WorldObject *target, int32 amount)
{
    if( !m_caster->IsPlayer() )
        return;

    castPtr<Player>(m_caster)->m_talentInterface.UnlockSpec(amount);
}

void SpellEffectClass::SpellEffectActivateTalentSpec(uint32 i, WorldObject *target, int32 amount)
{
    if( !m_caster->IsPlayer() )
        return;
    amount -= 1; // Add our negative offset

    Player *p_caster = castPtr<Player>(m_caster);
    if(p_caster->m_bg)
    {
        SendCastResult(SPELL_FAILED_NOT_IN_BATTLEGROUND);
        return;
    }

    // 0 = primary, 1 = secondary
    p_caster->m_talentInterface.ApplySpec(amount);
    p_caster->SetPower(p_caster->getPowerType(), 0);
}

void SpellEffectClass::SpellEffectDisengage(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    unitTarget->knockback(GetSpellProto()->EffectBasePoints[i]+1, GetSpellProto()->EffectMiscValue[i], true);
}

void SpellEffectClass::SpellEffectClearFinishedQuest(uint32 i, WorldObject *target, int32 amount)
{
    Player *playerTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if (playerTarget == NULL)
        return;

    playerTarget->m_completedQuests.erase(GetSpellProto()->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectApplyDemonAura(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectRemoveAura(uint32 i, WorldObject *target, int32 amount)
{
    Unit *unitTarget = target->IsUnit() ? castPtr<Unit>(target) : NULL;
    if (!unitTarget)
        return;

    unitTarget->RemoveAura(GetSpellProto()->EffectTriggerSpell[i], unitTarget->GetGUID());
}

void SpellEffectClass::SpellEffectActivateRune(uint32 i, WorldObject *target, int32 amount)
{
    if( !m_caster->IsPlayer() )
        return;

    Player *p_caster = castPtr<Player>(m_caster);
    if( p_caster->getClass() != DEATHKNIGHT )
        return;

    for( uint8 j = 0; j < 6; ++j )
    {
        if( p_caster->GetRune(j) == RUNE_TYPE_RECHARGING && p_caster->GetBaseRune(j) == GetSpellProto()->EffectMiscValue[i] )
        {
            p_caster->SetRune(j, p_caster->GetBaseRune(j));
            p_caster->ClearRuneCooldown(j);
        }
    }
}

void SpellEffectClass::SpellEffectFailQuest(uint32 i, WorldObject *target, int32 amount)
{
    if( !m_caster->IsPlayer() )
        return;

    if(Quest * qst = sQuestMgr.GetQuestPointer(GetSpellProto()->EffectMiscValue[i]))
        sQuestMgr.SendQuestFailed(FAILED_REASON_FAILED, qst, castPtr<Player>(m_caster));
}
