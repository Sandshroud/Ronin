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
    BaseSpell::Destruct();
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
    m_spellEffectMap[SPELL_EFFECT_DODGE]                        = &SpellEffectClass::SpellEffectDodge; // 20
    m_spellEffectMap[SPELL_EFFECT_PARRY]                        = &SpellEffectClass::SpellEffectParry; // 22
    m_spellEffectMap[SPELL_EFFECT_BLOCK]                        = &SpellEffectClass::SpellEffectBlock; // 23
    m_spellEffectMap[SPELL_EFFECT_CREATE_ITEM]                  = &SpellEffectClass::SpellEffectCreateItem; // 24
    m_spellEffectMap[SPELL_EFFECT_WEAPON]                       = &SpellEffectClass::SpellEffectWeapon; // 25
    m_spellEffectMap[SPELL_EFFECT_DEFENSE]                      = &SpellEffectClass::SpellEffectDefense; // 26
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
    m_spellEffectMap[SPELL_EFFECT_SPELL_DEFENSE]                = &SpellEffectClass::SpellEffectSpellDefense; // 37
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
    m_spellEffectMap[SPELL_EFFECT_CREATE_HOUSE]                 = &SpellEffectClass::SpellEffectCreateHouse; // 81
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
    m_spellEffectMap[SPELL_EFFECT_APPLY_PET_AURA]               = &SpellEffectClass::SpellEffectApplyPetAura; // 119
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
    if(false)//unitTarget->SchoolImmunityList[GetSpellProto()->School])
    {
        SendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }

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
            pTarget = m_caster->GetMapMgr()->GetUnit(castPtr<Player>(m_caster)->GetSelection());
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

    /*if(target->IsPlayer())
        HandleTeleport(spellId, target);*/
}

void SpellEffectClass::SpellEffectApplyAura(uint32 i, WorldObject *target, int32 amount)  // Apply Aura
{

}

void SpellEffectClass::SpellEffectPowerDrain(uint32 i, WorldObject *target, int32 amount)  // Power Drain
{

}

void SpellEffectClass::SpellEffectHealthLeech(uint32 i, WorldObject *target, int32 amount) // Health Leech
{

}

void SpellEffectClass::SpellEffectHeal(uint32 i, WorldObject *target, int32 amount) // Heal
{

}

void SpellEffectClass::SpellEffectBind(uint32 i, WorldObject *target, int32 amount) // Innkeeper Bind
{

}

void SpellEffectClass::SpellEffectQuestComplete(uint32 i, WorldObject *target, int32 amount) // Quest Complete
{

}

void SpellEffectClass::SpellEffectWeaponDamageNoSchool(uint32 i, WorldObject *target, int32 amount) // Weapon damage + (no School)
{

}

void SpellEffectClass::SpellEffectAddExtraAttacks(uint32 i, WorldObject *target, int32 amount) // Add Extra Attacks
{

}

void SpellEffectClass::SpellEffectDodge(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectParry(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectBlock(uint32 i, WorldObject *target, int32 amount)
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

void SpellEffectClass::SpellEffectDefense(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectPersistentAA(uint32 i, WorldObject *target, int32 amount) // Persistent Area Aura
{

}

void SpellEffectClass::SpellEffectLeap(uint32 i, WorldObject *target, int32 amount) // Leap
{

}

void SpellEffectClass::SpellEffectEnergize(uint32 i, WorldObject *target, int32 amount) // Energize
{

}

void SpellEffectClass::SpellEffectWeaponDmgPerc(uint32 i, WorldObject *target, int32 amount) // Weapon Percent damage
{

}

void SpellEffectClass::SpellEffectTriggerMissile(uint32 i, WorldObject *target, int32 amount) // Trigger Missile
{

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
    printf("Sending proficiency\n");
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

}

void SpellEffectClass::SpellEffectSpellDefense(uint32 i, WorldObject *target, int32 amount)
{
    //used to enable this ability. We use it all the time ...
}

void SpellEffectClass::SpellEffectLearnPetSpell(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectDispel(uint32 i, WorldObject *target, int32 amount) // Dispel
{

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

}

void SpellEffectClass::SpellEffectSkillStep(uint32 i, WorldObject *target, int32 amount) // Skill Step
{

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

}

void SpellEffectClass::SpellEffectSummonPet(uint32 i, WorldObject *target, int32 amount) //summon - pet
{

}

void SpellEffectClass::SpellEffectWeaponDamage(uint32 i, WorldObject *target, int32 amount) // Weapon damage +
{

}

void SpellEffectClass::SpellEffectPowerBurn(uint32 i, WorldObject *target, int32 amount) // power burn
{

}

void SpellEffectClass::SpellEffectThreat(uint32 i, WorldObject *target, int32 amount) // Threat
{

}

void SpellEffectClass::SpellEffectTriggerSpell(uint32 i, WorldObject *target, int32 amount) // Trigger Spell
{

}

void SpellEffectClass::SpellEffectHealthFunnel(uint32 i, WorldObject *target, int32 amount) // Health Funnel
{

}

void SpellEffectClass::SpellEffectPowerFunnel(uint32 i, WorldObject *target, int32 amount) // Power Funnel
{

}

void SpellEffectClass::SpellEffectHealMaxHealth(uint32 i, WorldObject *target, int32 amount)   // Heal Max Health
{

}

void SpellEffectClass::SpellEffectInterruptCast(uint32 i, WorldObject *target, int32 amount) // Interrupt Cast
{

}

void SpellEffectClass::SpellEffectDistract(uint32 i, WorldObject *target, int32 amount) // Distract
{

}

void SpellEffectClass::SpellEffectPickpocket(uint32 i, WorldObject *target, int32 amount) // pickpocket
{

}

void SpellEffectClass::SpellEffectAddFarsight(uint32 i, WorldObject *target, int32 amount) // Add Farsight
{

}

void SpellEffectClass::SpellEffectResetTalents(uint32 i, WorldObject *target, int32 amount) // Used by Trainers.
{

}

void SpellEffectClass::SpellEffectUseGlyph(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectHealMechanical(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSummonObjectWild(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectScriptEffect(uint32 i, WorldObject *target, int32 amount) // Script Effect
{
    sLog.outDebug("Unhandled Scripted Effect In Spell %u", m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectSanctuary(uint32 i, WorldObject *target, int32 amount) // Stop all attacks made to you
{

}

void SpellEffectClass::SpellEffectAddComboPoints(uint32 i, WorldObject *target, int32 amount) // Add Combo Points
{

}

void SpellEffectClass::SpellEffectCreateHouse(uint32 i, WorldObject *target, int32 amount) // Create House
{


}

void SpellEffectClass::SpellEffectDuel(uint32 i, WorldObject *target, int32 amount) // Duel
{

}

void SpellEffectClass::SpellEffectStuck(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSummonPlayer(uint32 i, WorldObject *target, int32 amount)
{

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

}

void SpellEffectClass::SpellEffectSkinning(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectCharge(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectPlaceTotemsOnBar(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSendTaxi(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectPull(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectKnockBack(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectDisenchant(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectInebriate(uint32 i, WorldObject *target, int32 amount) // lets get drunk!
{

}

void SpellEffectClass::SpellEffectFeedPet(uint32 i, WorldObject *target, int32 amount)  // Feed Pet
{

}

void SpellEffectClass::SpellEffectReputation(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSummonObjectSlot(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectDispelMechanic(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSummonDeadPet(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectDestroyAllTotems(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSummonDemonOld(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectResurrect(uint32 i, WorldObject *target, int32 amount) // Resurrect (Flat)
{

}

void SpellEffectClass::SpellEffectAttackMe(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSkinPlayerCorpse(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSkill(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectApplyPetAura(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectDummyMelee(uint32 i, WorldObject *target, int32 amount) // Normalized Weapon damage +
{

}

void SpellEffectClass::SpellEffectSpellSteal(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectProspecting(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectResurrectNew(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectTranformItem(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectEnvironmentalDamage(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectDismissPet(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectEnchantHeldItem(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectAddHonor(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectSpawn(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectRedirectThreat(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectPlayMusic(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectKillCredit(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectRestoreManaPct(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectRestoreHealthPct(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectForceCast(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectTriggerSpellWithValue(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectJump(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectTeleportToCaster(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectMilling(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectAllowPetRename(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectStartQuest(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectCreatePet(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectTitanGrip(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectCreateRandomItem(uint32 i, WorldObject *target, int32 amount) // Create Random Item
{

}

void SpellEffectClass::SpellEffectSetTalentSpecsCount(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectActivateTalentSpec(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectDisengage(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectClearFinishedQuest(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectApplyDemonAura(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectRemoveAura(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectActivateRune(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectFailQuest(uint32 i, WorldObject *target, int32 amount)
{

}
