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
    m_spellEffectMap[SPELL_EFFECT_EVADE]                        = &SpellEffectClass::SpellEffectNULL; // 21
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
    m_spellEffectMap[SPELL_EFFECT_TEACH_TAXI_NODE]              = &SpellEffectClass::SpellEffectNULL;
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
/*
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
    if(unitTarget->SchoolImmunityList[GetSpellProto()->School])
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

    if(target->IsPlayer())
        HandleTeleport(spellId, target);
}

void SpellEffectClass::SpellEffectApplyAura(uint32 i, WorldObject *target, int32 amount)  // Apply Aura
{
    if(unitTarget == NULL)
        return;

    //Aura Immune Flag Check
    if ( playerTarget == NULL)
    {
        if(Creature* c = castPtr<Creature>( unitTarget ))
        {
            if(c->GetCreatureData()->AuraMechanicImmunity)
            {
                if(c->GetCreatureData()->AuraMechanicImmunity & (uint32(1)<<GetSpellProto()->MechanicsType))
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
        if(g_caster && g_caster->GetUInt32Value(GAMEOBJECT_FIELD_CREATED_BY) && g_caster->m_summoner)
            pAura = new Aura(GetSpellProto(), g_caster->m_summoner, unitTarget);
        else pAura = new Aura(GetSpellProto(), m_caster, unitTarget);
        if(pAura == NULL)
            return;

        pAura->SetTriggerSpellId(m_triggeredSpellId); //this is required for triggered spells
        m_tempAuras.insert(std::make_pair(unitTarget->GetGUID(), pAura));
    } else pAura = m_tempAuras.at(unitTarget->GetGUID());

    int32 miscValue = GetSpellProto()->EffectMiscValue[i];
    if(i_caster && m_caster->IsPlayer() && GetSpellProto()->EffectApplyAuraName[i]==SPELL_AURA_PROC_TRIGGER_SPELL)
        miscValue = p_caster->GetInventory()->GetInventorySlotByGuid( i_caster->GetGUID() ); // Need to know on which hands attacks spell should proc

    //Interactive spells
    uint32 spellID = GetSpellProto()->Id;
    switch(spellID)
    {
    case 27907:
        {
            if(unitTarget->GetEntry() == 15941)
            {
                sQuestMgr.OnPlayerKill(p_caster, ((Creature*)unitTarget));
                unitTarget->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "What? Oh, not this again!");
            }
            else if(unitTarget->GetEntry() == 15945)
            {
                sQuestMgr.OnPlayerKill(p_caster, ((Creature*)unitTarget));
                unitTarget->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You can't do this to me! We had a deal!");
            }
            else
            {
                SendCastResult(SPELL_FAILED_BAD_TARGETS);
                return;
            }
        }break;
    case 28880:
        {
            if(!p_caster)
                break;

            if(unitTarget->GetEntry() == 16483)
            {
                unitTarget->RemoveAura(29152);
                unitTarget->SetStandState(0);
                sQuestMgr.OnPlayerKill(p_caster, ((Creature*)unitTarget));
                static const char* testo[12] =
                {"None","Warrior", "Paladin", "Hunter", "Rogue", "Priest", "Death Knight", "Shaman", "Mage", "Warlock", "None", "Druid"};
                char msg[150];
                snprintf(msg, 150, "Many thanks to you %s. I'd best get to the crash site and see how I can help out. Until we meet again...", testo[p_caster->getClass()]);
                unitTarget->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg);
                ((Creature*)unitTarget)->Despawn(900000, 300000); //removed after 15minutes, respawned after another 5 minutes
            }
        }break;
    case 38177: //Blackwhelp Net
        {
            if(!p_caster)
                break;

            if(unitTarget->GetEntry() == 21387)
            {
                ((Creature*)unitTarget)->Despawn(5000, 360000);
                p_caster->CastSpell(p_caster, 38178, true);
            }else
            {
                SendCastResult(SPELL_FAILED_BAD_TARGETS);
                return;
            }
        }break;
    }

    pAura->AddMod(GetSpellProto()->EffectApplyAuraName[i],damage,miscValue,GetSpellProto()->EffectMiscValueB[i],i);
}

void SpellEffectClass::SpellEffectPowerDrain(uint32 i, WorldObject *target, int32 amount)  // Power Drain
{
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 powerField = UNIT_FIELD_POWERS;
    if(GetSpellProto()->EffectMiscValue[i] < 5)
        powerField += GetSpellProto()->EffectMiscValue[i];
    uint32 curPower = unitTarget->GetUInt32Value(powerField);
    uint32 amt = u_caster->GetSpellBonusDamage(unitTarget, GetSpellProto(), i, damage, false);

    if( GetPlayerTarget() )
        amt *= float2int32( 1 - ( ( castPtr<Player>(GetPlayerTarget())->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE ) * 2 ) / 100.0f ) );

    if(amt > curPower)
        amt = curPower;

    unitTarget->SetUInt32Value(powerField, curPower - amt);
    u_caster->Energize(u_caster, m_triggeredSpellId ? m_triggeredSpellId : GetSpellProto()->Id, amt, GetSpellProto()->EffectMiscValue[i]);
    unitTarget->SendPowerUpdate();
}

void SpellEffectClass::SpellEffectHealthLeech(uint32 i, WorldObject *target, int32 amount) // Health Leech
{
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 amt = damage;
    if(amt > curHealth)
        amt = curHealth;

    TotalDamage += m_caster->DealDamage(unitTarget, damage, 0, 0, GetSpellProto()->Id);

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
    }
    else
    {
        m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, playerCurHealth + amt);
    }
    SendHealSpellOnPlayer(m_caster, m_caster, amt, false, overheal, GetSpellProto()->Id);
}

void SpellEffectClass::SpellEffectHeal(uint32 i, WorldObject *target, int32 amount) // Heal
{
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    if(GetSpellProto()->EffectChainTarget[i])//chain
    {
        if(chaindamage)
        {
            int32 reduce = (int32)(GetSpellProto()->EffectDamageMultiplier[i] * 100.0f);
            chaindamage -= (reduce * chaindamage) / 100;
        }else chaindamage = damage;
        Heal(i, chaindamage);
        return;
    }

    Heal(i, damage);
}

void SpellEffectClass::SpellEffectBind(uint32 i, WorldObject *target, int32 amount) // Innkeeper Bind
{
    if(!playerTarget)
        return;
    playerTarget->SetBindPoint(playerTarget->GetPositionX(), playerTarget->GetPositionY(), playerTarget->GetPositionZ(), playerTarget->GetMapId(), playerTarget->GetZoneId());
}

void SpellEffectClass::SpellEffectQuestComplete(uint32 i, WorldObject *target, int32 amount) // Quest Complete
{
    if( playerTarget != NULL )
    {
        QuestLogEntry *qle = playerTarget->GetQuestLogForEntry(GetSpellProto()->EffectMiscValue[i]);
        if( qle == NULL )
            return;

        qle->SendQuestComplete();
        qle->SetQuestStatus(QUEST_STATUS__COMPLETE);
        qle->UpdatePlayerFields();
    }
}

//wand->
void SpellEffectClass::SpellEffectWeaponDamageNoSchool(uint32 i, WorldObject *target, int32 amount) // Weapon damage + (no School)
{
    if( unitTarget == NULL  || u_caster == NULL )
        return;

    if( GetType() == SPELL_DMG_TYPE_RANGED && GetSpellProto()->speed > 0.0f )
        TotalDamage += u_caster->Strike( unitTarget, RANGED, GetSpellProto(), i, 0, 0, 0, false, true );
    else TotalDamage += u_caster->Strike( unitTarget, ( GetType() == SPELL_DMG_TYPE_RANGED ? RANGED : MELEE ), GetSpellProto(), i, damage, 0, 0, false, true );
}

void SpellEffectClass::SpellEffectAddExtraAttacks(uint32 i, WorldObject *target, int32 amount) // Add Extra Attacks
{
    if( u_caster == NULL )
        return;

    if( ProcedOnSpell )
        u_caster->m_extraattacks[0] = ProcedOnSpell->procCharges;

    if( GetSpellProto()->procCharges > 0 )
        u_caster->m_extraattacks[0] = GetSpellProto()->procCharges;

    u_caster->m_extraattacks[1] += damage;
}

void SpellEffectClass::SpellEffectDodge(uint32 i, WorldObject *target, int32 amount)
{
    //i think this actually enbles the skill to be able to dodge melee+ranged attacks
    //value is static and sets value directly which will be modified by other factors
    //this is only basic value and will be overwiten elsewhere !!!
//  if(playerTarget)
//      unitTarget->SetFloatValue(PLAYER_DODGE_PERCENTAGE,damage);
}

void SpellEffectClass::SpellEffectParry(uint32 i, WorldObject *target, int32 amount)
{
    if(unitTarget != NULL )
        unitTarget->setcanperry(true);
}

void SpellEffectClass::SpellEffectBlock(uint32 i, WorldObject *target, int32 amount)
{
    //i think this actually enbles the skill to be able to block melee+ranged attacks
    //value is static and sets value directly which will be modified by other factors
//  if(playerTarget)
//      unitTarget->SetFloatValue(PLAYER_BLOCK_PERCENTAGE,damage);
}

void SpellEffectClass::SpellEffectCreateItem(uint32 i, WorldObject *target, int32 amount) // Create item
{

}

void SpellEffectClass::SpellEffectWeapon(uint32 i, WorldObject *target, int32 amount)
{
    if( playerTarget == NULL )
        return;

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
    //i think this actually enbles the skill to be able to use defense
    //value is static and sets value directly which will be modified by other factors
    //this is only basic value and will be overwiten elsewhere !!!
//  if(unitTarget != NULL && playerTarget)
//      unitTarget->SetFloatValue(UNIT_FIELD_RESISTANCES,damage);
}

void SpellEffectClass::SpellEffectPersistentAA(uint32 i, WorldObject *target, int32 amount) // Persistent Area Aura
{
    if(m_AreaAura || m_caster == NULL || !m_caster->IsInWorld())
        return;
    int32 dur = GetDuration();
    float r = GetRadius(i);

    DynamicObject* dynObj = m_caster->GetMapMgr()->CreateDynamicObject();
    if(g_caster != NULL && g_caster->m_summoner && !unitTarget)
    {
        Unit* caster = g_caster->m_summoner;
        dynObj->Create(caster, this, g_caster->GetPositionX(), g_caster->GetPositionY(), g_caster->GetPositionZ(), dur, r);
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
            dynObj->Create(u_caster ? u_caster : g_caster->m_summoner, this, m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, dur, r);
        }break;
    default:
        dynObj->Remove();
        return;
    }

    m_AreaAura = true;
}

void SpellEffectClass::SpellEffectLeap(uint32 i, WorldObject *target, int32 amount) // Leap
{
    if(m_caster == NULL )
        return;
    float radius = GetRadius(i);

    //FIXME: check for obstacles

    if(!p_caster)
        return;

    // remove movement impeding auras
    p_caster->m_AuraInterface.RemoveAllAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);

    if(p_caster->m_bg && !p_caster->m_bg->HasStarted())
        return;

    // just in case
    p_caster->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_STUN);
    p_caster->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_ROOT);

    if(p_caster->GetMapMgr() && !IS_INSTANCE(p_caster->GetMapId()) && p_caster->GetMapMgr()->CanUseCollision(p_caster))
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
    data << u_caster->GetGUID();
    data << getMSTime();
    data << cosf(u_caster->GetOrientation()) << sinf(u_caster->GetOrientation());
    data << radius;
    data << float(-10.0f);
    p_caster->GetSession()->SendPacket(&data);
}

void SpellEffectClass::SpellEffectEnergize(uint32 i, WorldObject *target, int32 amount) // Energize
{
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 modEnergy = damage;
    u_caster->Energize(unitTarget, (m_triggeredSpellId ? m_triggeredSpellId : GetSpellProto()->Id), modEnergy, GetSpellProto()->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectWeaponDmgPerc(uint32 i, WorldObject *target, int32 amount) // Weapon Percent damage
{
    if( unitTarget == NULL || u_caster == NULL )
        return;

    uint32 _type = MELEE;
    if( GetType() == SPELL_DMG_TYPE_RANGED )
        _type = RANGED;
    else if (GetSpellProto()->reqOffHandWeapon())
        _type = OFFHAND;
    if( GetType() == SPELL_DMG_TYPE_MAGIC )
    {
        float fdmg = (float)sStatSystem.CalculateDamage( u_caster, unitTarget, _type, GetSpellProto() );
        uint32 dmg = float2int32(fdmg*(float(damage/100.0f)));
        TotalDamage += u_caster->SpellNonMeleeDamageLog(unitTarget, GetSpellProto()->Id, dmg, false, false, false);
    } else TotalDamage += u_caster->Strike( unitTarget, _type, GetSpellProto(), i, add_damage, damage, 0, false, false );
}

void SpellEffectClass::SpellEffectTriggerMissile(uint32 i, WorldObject *target, int32 amount) // Trigger Missile
{
    //Used by mortar team
    //Triggers area affect spell at destinatiom
    if(u_caster == NULL )
        return;

    uint32 spellid = GetSpellProto()->EffectTriggerSpell[i];
    if(spellid == 0)
        return;

    SpellEntry *spInfo = dbcSpell.LookupEntry(spellid);
    if(spInfo == NULL )
        return;

    // Just send this spell where he wants :S
    u_caster->CastSpellAoF(m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, spInfo, true);
}

void SpellEffectClass::SpellEffectOpenLock(uint32 i, WorldObject *target, int32 amount) // Open Lock
{
    if( p_caster  == NULL )
        return;

    if( p_caster->InStealth() )
    {
        p_caster->RemoveAura(p_caster->m_stealth);
        p_caster->m_stealth = 0;
    }

    uint32 locktype = GetSpellProto()->EffectMiscValue[i];
    switch(locktype)
    {
    case LOCKTYPE_PICKLOCK:
        {
            uint32 v = 0;
            uint32 lockskill = p_caster->_GetSkillLineCurrent(SKILL_LOCKPICKING);

            if(itemTarget)
            {
                if(itemTarget->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_UNLOCKED))
                    return;

                LockEntry *lock = dbcLock.LookupEntry( itemTarget->GetProto()->LockId );
                if(lock == NULL)
                    return;

                for(int8 i = 0; i < 8; ++i)
                {
                    if(lock->locktype[i] == 2 && lock->minlockskill[i] && lockskill >= lock->minlockskill[i])
                    {
                        v = lock->minlockskill[i];
                        itemTarget->AddItemFlag(ITEM_FLAG_UNLOCKED);
                        itemTarget->SetFlag(ITEM_FIELD_FLAGS, 4); // unlock
                        DetermineSkillUp(SKILL_LOCKPICKING,v/5);
                        break;
                    }
                }
            }
            else if(gameObjTarget)
            {
                if(gameObjTarget->GetState() == 0)
                    return;

                LockEntry *lock = dbcLock.LookupEntry(gameObjTarget->GetInfo()->GetLockID());
                if( lock == NULL )
                    return;

                for(int i=0; i < 8;++i)
                {
                    if(lock->locktype[i] == 2 && lock->minlockskill[i] && lockskill >= lock->minlockskill[i])
                    {
                        v = lock->minlockskill[i];
                        gameObjTarget->SetFlags(0);
                        gameObjTarget->SetState(1);
                        lootmgr.FillGOLoot(gameObjTarget->GetLoot(),gameObjTarget->GetEntry(), (gameObjTarget->GetMapMgr() ? gameObjTarget->GetMapMgr()->iInstanceMode : 0), p_caster->GetTeam());
                        DetermineSkillUp(SKILL_LOCKPICKING,v/5);
                        break;
                    }
                }
            }
        }break;
    case LOCKTYPE_HERBALISM:
        {
            if(!gameObjTarget) return;

            uint32 v = gameObjTarget->GetGOReqSkill();
            bool bAlreadyUsed = false;

            if(Rand(100.0f)) // 3% chance to fail//why?
            {
                if( castPtr<Player>( m_caster )->_GetSkillLineCurrent( SKILL_HERBALISM ) < v )
                {
                    //SendCastResult(SPELL_FAILED_LOW_CASTLEVEL);
                    return;
                }
                else
                {
                    if( gameObjTarget->GetLoot()->items.size() == 0 )
                    {
                        lootmgr.FillGOLoot(gameObjTarget->GetLoot(),gameObjTarget->GetEntry(), (gameObjTarget->GetMapMgr() ? gameObjTarget->GetMapMgr()->iInstanceMode : 0), p_caster->GetTeam());
                    } else bAlreadyUsed = true;
                }
            } else SendCastResult(SPELL_FAILED_TRY_AGAIN);

            //Skill up
            if(!bAlreadyUsed) //Avoid cheats with opening/closing without taking the loot
                DetermineSkillUp(SKILL_HERBALISM,v/5);
        }break;
    case LOCKTYPE_MINING:
        {
            if(!gameObjTarget) return;

            uint32 v = gameObjTarget->GetGOReqSkill();
            bool bAlreadyUsed = false;

            if( Rand( 100.0f ) ) // 3% chance to fail//why?
            {
                if( castPtr<Player>( m_caster )->_GetSkillLineCurrent( SKILL_MINING ) < v )
                    return;

                if( gameObjTarget->GetLoot()->items.size() )
                    bAlreadyUsed = true;
                else lootmgr.FillGOLoot(gameObjTarget->GetLoot(),gameObjTarget->GetEntry(), (gameObjTarget->GetMapMgr() ? gameObjTarget->GetMapMgr()->iInstanceMode : 0), p_caster->GetTeam());
            } else SendCastResult(SPELL_FAILED_TRY_AGAIN);

            //Skill up
            if(!bAlreadyUsed) //Avoid cheats with opening/closing without taking the loot
                DetermineSkillUp(SKILL_MINING,v/5);
        }break;
    case LOCKTYPE_SLOW_OPEN: // used for BG go's
        {
            if(!gameObjTarget )
                return;

            if(p_caster->m_bgFlagIneligible)
                return;

            if(p_caster && p_caster->m_bg)
                if(p_caster->m_bg->HookSlowLockOpen(gameObjTarget,p_caster,this))
                    return;

            uint32 spellid = 23932;
            if(gameObjTarget->GetInfo()->RawData.ListedData[10])
                spellid = gameObjTarget->GetInfo()->RawData.ListedData[10];

            SpellEntry*en = dbcSpell.LookupEntry(spellid);
            if(en == NULL)
                return;

            Spell* sp = new Spell(p_caster,en,true,NULL);
            SpellCastTargets tgt;
            tgt.m_unitTarget = gameObjTarget->GetGUID();
            sp->prepare(&tgt);
        }break;
    case LOCKTYPE_QUICK_CLOSE:
        {
            if(!gameObjTarget )
                return;

            gameObjTarget->EventCloseDoor();
        }break;
    default://not profession
        {
            if(!gameObjTarget)
                return;

            if(sQuestMgr.OnActivateQuestGiver(gameObjTarget, p_caster))
                return;

            if( gameObjTarget->GetType() == GAMEOBJECT_TYPE_GOOBER)
            {
                TRIGGER_GO_EVENT(gameObjTarget, OnActivate)(castPtr<Player>(p_caster));
                TRIGGER_INSTANCE_EVENT( gameObjTarget->GetMapMgr(), OnGameObjectActivate )( gameObjTarget, p_caster );
            }

            if(sQuestMgr.OnGameObjectActivate(p_caster, gameObjTarget))
            {
                p_caster->UpdateNearbyGameObjects();
                return;
            }

            if(gameObjTarget->GetLoot()->items.size() == 0)
            {
                lootmgr.FillGOLoot(gameObjTarget->GetLoot(),gameObjTarget->GetEntry(), (gameObjTarget->GetMapMgr() ? gameObjTarget->GetMapMgr()->iInstanceMode : 0), p_caster->GetTeam());
            }
        }break;
    }

    if( gameObjTarget != NULL && gameObjTarget->GetType() == GAMEOBJECT_TYPE_CHEST)
        castPtr<Player>( m_caster )->SendLoot( gameObjTarget->GetGUID(), gameObjTarget->GetMapId(), LOOT_SKINNING );
}

void SpellEffectClass::SpellEffectOpenLockItem(uint32 i, WorldObject *target, int32 amount)
{
    Unit* caster = u_caster;
    if(caster == NULL && i_caster != NULL )
        caster = i_caster->GetOwner();

    if( caster == NULL || gameObjTarget  == NULL || !gameObjTarget->IsInWorld())
        return;

    if( caster->IsPlayer() && sQuestMgr.OnGameObjectActivate( (castPtr<Player>(caster)), gameObjTarget ) )
        castPtr<Player>(caster)->UpdateNearbyGameObjects();

    TRIGGER_GO_EVENT(gameObjTarget, OnActivate)(castPtr<Player>(caster));
    TRIGGER_INSTANCE_EVENT( gameObjTarget->GetMapMgr(), OnGameObjectActivate )( gameObjTarget, castPtr<Player>( caster ) );
    gameObjTarget->SetState(0);

    if( gameObjTarget->GetType() == GAMEOBJECT_TYPE_CHEST)
    {
        lootmgr.FillGOLoot(gameObjTarget->GetLoot(),gameObjTarget->GetEntry(), (gameObjTarget->GetMapMgr() ? gameObjTarget->GetMapMgr()->iInstanceMode : 0), p_caster->GetTeam());
        if(gameObjTarget->GetLoot()->items.size() > 0)
        {
            castPtr<Player>(caster)->SendLoot(gameObjTarget->GetGUID(), gameObjTarget->GetMapId(), LOOT_CORPSE);
        }
        gameObjTarget->SetFlags(1);
    }

    if( gameObjTarget->GetType() == GAMEOBJECT_TYPE_DOOR)
    {
        gameObjTarget->SetFlags(33);

        if(gameObjTarget->GetMapMgr()->GetMapInfo()->type==INSTANCE_NULL)//dont close doors for instances
            sEventMgr.AddEvent(gameObjTarget,&GameObject::EventCloseDoor, EVENT_GAMEOBJECT_DOOR_CLOSE,10000,1,0);
    }

    sEventMgr.AddEvent(gameObjTarget, &GameObject::Despawn, (uint32)0, (uint32)1, EVENT_GAMEOBJECT_ITEM_SPAWN, 6*60*1000, 1, 0);
}

void SpellEffectClass::SpellEffectProficiency(uint32 i, WorldObject *target, int32 amount)
{
    if(playerTarget == NULL)
        return;

    uint32 skill = 0;
    if (SkillLineAbilityEntry* skillability = objmgr.GetSpellSkill(GetSpellProto()->Id))
        skill = skillability->skilline;
    if(SkillLineEntry* sk = dbcSkillLine.LookupEntry(skill))
    {
        if(!playerTarget->_HasSkillLine(skill))
        {
            if(sk && sk->categoryId == SKILL_TYPE_WEAPON)
            {
                if(sWorld.StartLevel > 1)
                    playerTarget->_AddSkillLine(skill, 5*sWorld.StartLevel, 5*playerTarget->getLevel());
                else playerTarget->_AddSkillLine(skill, 1, 5*playerTarget->getLevel());
            } else playerTarget->_AddSkillLine(skill, 1, 1);
        }
    }
}

void SpellEffectClass::SpellEffectSendEvent(uint32 i, WorldObject *target, int32 amount) //Send Event
{
    sLog.outDebug("Event spell not handled: %u%s\n", m_spellInfo->Id, ((ProcedOnSpell != NULL) ? (format(" proc'd on: %u", ProcedOnSpell->Id).c_str()) : ""));
}

void SpellEffectClass::SpellEffectApplyAA(uint32 i, WorldObject *target, int32 amount) // Apply Area Aura
{

}

void SpellEffectClass::SpellEffectLearnSpell(uint32 i, WorldObject *target, int32 amount) // Learn Spell
{
    if(playerTarget == 0 && unitTarget != NULL && unitTarget->IsPet())
    {
        SpellEffectLearnPetSpell(i);
        return;
    }

    if( GetSpellProto()->Id == 483 || GetSpellProto()->Id == 55884 )        // "Learning"
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
    }

    if(playerTarget!=NULL)
    {
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
        if(spellinfo->HasEffect(SPELL_EFFECT_WEAPON) || spellinfo->HasEffect(SPELL_EFFECT_DUAL_WIELD)
            || spellinfo->HasEffect(SPELL_EFFECT_PROFICIENCY))
        {
            Spell* sp = new Spell(unitTarget, spellinfo, true, NULL);
            SpellCastTargets targets;
            targets.m_unitTarget = unitTarget->GetGUID();
            targets.m_targetMask = TARGET_FLAG_UNIT;
            sp->prepare(&targets);
        }
        return;
    }

    // if we got here... try via pet spells..
    SpellEffectLearnPetSpell(i);
}

void SpellEffectClass::SpellEffectSpellDefense(uint32 i, WorldObject *target, int32 amount)
{
    //used to enable this ability. We use it all the time ...
}

void SpellEffectClass::SpellEffectLearnPetSpell(uint32 i, WorldObject *target, int32 amount)
{
    if(unitTarget && unitTarget->IsPet() && p_caster)
    {
        if (GetSpellProto()->EffectTriggerSpell[i])
        {
            Pet* pPet = castPtr<Pet>( unitTarget );
            if(pPet->IsSummonedPet())
                p_caster->AddSummonSpell(unitTarget->GetEntry(), GetSpellProto()->EffectTriggerSpell[i]);

            pPet->AddSpell( dbcSpell.LookupEntry( GetSpellProto()->EffectTriggerSpell[i] ), true );
        }
    }
}

void SpellEffectClass::SpellEffectDispel(uint32 i, WorldObject *target, int32 amount) // Dispel
{
    if(u_caster == NULL || unitTarget == NULL )
        return;

    uint32 start = 0, end;
    if(sFactionSystem.isAttackable(u_caster,unitTarget))
        end = MAX_POSITIVE_AURAS;
    else
    {
        start = MAX_POSITIVE_AURAS;
        end = MAX_AURAS;
    }

    unitTarget->m_AuraInterface.MassDispel(u_caster, i, GetSpellProto(), damage, start, end);
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
    if(p_caster == NULL)
        return;

    if( !p_caster->_HasSkillLine( SKILL_DUAL_WIELD ) )
         p_caster->_AddSkillLine( SKILL_DUAL_WIELD, 1, 1 );

        // Increase it by one
        //dual wield is 1/1 , it never increases it's not even displayed in skills tab

    //note: probably here must be not caster but unitVictim
}

void SpellEffectClass::SpellEffectSkillStep(uint32 i, WorldObject *target, int32 amount) // Skill Step
{
    Player* target = NULL;
    if(u_caster == NULL)
        return;

    // Check targets
    if( m_targets.m_unitTarget )
        target = u_caster->GetMapMgr()->GetPlayer(m_targets.m_unitTarget);
    else target = castPtr<Player>( m_caster );

    if( target == NULL )
        return;

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
        max = damage * 75;
        break;
    case SKILL_TYPE_WEAPON:
        max = 5 * target->getLevel();
        break;
    case SKILL_TYPE_CLASS:
    case SKILL_TYPE_ARMOR:
        if( skill == SKILL_LOCKPICKING )
            max = damage * 75;
        else
            max = 1;
        break;
    default: //u cant learn other types in game
        return;
    };

    if( target->_HasSkillLine( skill ) )
        target->_ModifySkillMaximum( skill, max );
    else
    {
        if( skill == SKILL_RIDING )
            target->_AddSkillLine( skill, max, max );
        else target->_AddSkillLine( skill, 1, max );
    }

    //professions fix, for unknow reason when u learn profession it
    //does not teach find herbs for herbalism etc. moreover there is no spell
    //in spell.dbc that would teach u this. It means blizz does it in some tricky way too
    switch( skill )
    {
    case SKILL_ALCHEMY:
        target->addSpell( 2330 );//Minor Healing Potion
        target->addSpell( 2329 );//Elixir of Lion's Strength
        target->addSpell( 7183 );//Elixir of Minor Defense
        break;
    case SKILL_ENCHANTING:
        target->addSpell( 7418 );//Enchant Bracer - Minor Health
        target->addSpell( 7428 );//Enchant Bracer - Minor Deflection
        target->addSpell( 7421 );//Runed Copper Rod
        target->addSpell( 13262 );//Disenchant
        break;
    case SKILL_HERBALISM:
        target->addSpell( 2383 );//find herbs
        break;
    case SKILL_MINING:
        target->addSpell( 2657 );//smelt copper
        target->addSpell( 2656 );//smelting
        target->addSpell( 2580 );//find minerals
        break;
    case SKILL_FIRST_AID:
        target->addSpell( 3275 );//Linen Bandage
        break;
    case SKILL_TAILORING:
        target->addSpell( 2963 );//Bolt of Linen Cloth
        target->addSpell( 2387 );//Linen Cloak
        target->addSpell( 2393 );//White Linen Shirt
        target->addSpell( 3915 );//Brown Linen Shirt
        target->addSpell( 12044 );//Simple Linen Pants
        break;
    case SKILL_LEATHERWORKING:
        target->addSpell( 2149 );//Handstitched Leather Boots
        target->addSpell( 2152 );//Light Armor Kit
        target->addSpell( 2881 );//Light Leather
        target->addSpell( 7126 );//Handstitched Leather Vest
        target->addSpell( 9058 );//Handstitched Leather Cloak
        target->addSpell( 9059 );//Handstitched Leather Bracers
        break;
    case SKILL_ENGINERING:
        target->addSpell( 3918 );//Rough Blasting Powder
        target->addSpell( 3919 );//Rough Dynamite
        target->addSpell( 3920 );//Crafted Light Shot
        break;
    case SKILL_COOKING:
        target->addSpell( 2538 );//Charred Wolf Meat
        target->addSpell( 2540 );//Roasted Boar Meat
        target->addSpell( 818 );//Basic Campfire
        target->addSpell( 8604 );//Herb Baked Egg
        break;
    case SKILL_BLACKSMITHING:
        target->addSpell( 2660 );//Rough Sharpening Stone
        target->addSpell( 2663 );//Copper Bracers
        target->addSpell( 12260 );//Rough Copper Vest
        target->addSpell( 2662 );//Copper Chain Pants
        target->addSpell( 3115 );//Rough Weightstone
        break;
    case SKILL_JEWELCRAFTING:
        target->addSpell( 25255 );// Delicate Copper Wire
        target->addSpell( 25493 );// Braided Copper Ring
        target->addSpell( 26925 );// Woven Copper Ring
        target->addSpell( 32259 );// Rough Stone Statue
        break;
    case SKILL_INSCRIPTION:
        target->addSpell( 51005 );// Milling
        target->addSpell( 48116 );// Scroll of Spirit
        target->addSpell( 48114 );// Scroll of Intellect
        target->addSpell( 45382 );// Scroll of Stamina
        target->addSpell( 52738 );// Ivory Ink
        break;
    }
}

void SpellEffectClass::SpellEffectDetect(uint32 i, WorldObject *target, int32 amount)
{
    if( u_caster == NULL )
        return;
    u_caster->UpdateVisibility();
}

void SpellEffectClass::SpellEffectSummonObject(uint32 i, WorldObject *target, int32 amount)
{
    if( u_caster == NULL )
        return;

    uint32 entry = GetSpellProto()->EffectMiscValue[i];

    uint32 mapid = u_caster->GetMapId();
    float px = u_caster->GetPositionX();
    float py = u_caster->GetPositionY();
    float pz = u_caster->GetPositionZ();
    float orient = m_caster->GetOrientation();
    float posx = 0,posy = 0,posz = 0;

    if( entry == GO_FISHING_BOBBER && p_caster)
    {
        float co = cos( orient );
        float si = sin( orient );
        MapMgr* map = m_caster->GetMapMgr();
        Spell* spell = u_caster->GetCurrentSpell();

        float r;
        for( r = 20; r > 10; r-- )
        {
            posx = px+r*co;
            posy = py+r*si;
            if(map->GetWaterHeight(posx, posy, pz) != NO_WATER_HEIGHT)//water
                continue;
            posz = map->GetLandHeight( posx, posy );
            break;
        }

        posx = px + r * co;
        posy = py + r * si;

        // Todo / Fix me: This should be loaded / cached
        uint32 zone = p_caster->GetAreaId();
        if( zone == 0 ) // If the player's area ID is 0, use the zone ID instead
            zone = p_caster->GetZoneId();

        spell->SendChannelStart( 20000 ); // 30 seconds

        GameObject* go = u_caster->GetMapMgr()->CreateGameObject(GO_FISHING_BOBBER);
        if( go == NULL || !go->CreateFromProto( GO_FISHING_BOBBER, mapid, posx, posy, posz, orient ))
            return;

        go->SetUInt32Value( GAMEOBJECT_FLAGS, 0 );
        go->SetByte(GAMEOBJECT_BYTES_1, 0, 0 );
        go->SetUInt64Value( GAMEOBJECT_FIELD_CREATED_BY, m_caster->GetGUID() );
        u_caster->SetUInt64Value( UNIT_FIELD_CHANNEL_OBJECT, go->GetGUID() );

        go->SetInstanceID( m_caster->GetInstanceID() );
        go->PushToWorld( m_caster->GetMapMgr() );

        if( lootmgr.IsFishable( zone ) ) // Only set a 'splash' if there is any loot in this area / zone
        {
            uint32 seconds = RandomUInt(17) + 2;
            sEventMgr.AddEvent( go, &GameObject::FishHooked, castPtr<Player>( m_caster ), EVENT_GAMEOBJECT_FISH_HOOKED, seconds * 1000, 1, 0 );
        }
        sEventMgr.AddEvent( go, &GameObject::EndFishing, castPtr<Player>( m_caster ),true, EVENT_GAMEOBJECT_END_FISHING, uint32(GetDuration() + 1000), 1, 0 );
        p_caster->SetSummonedObject( go );
    }
    else
    {//portal
        posx=px;
        posy=py;
        GameObjectInfo * goI = GameObjectNameStorage.LookupEntry(entry);
        if(!goI)
        {
            if( p_caster != NULL )
            {
                sChatHandler.BlueSystemMessage(p_caster->GetSession(),
                "non-existant gameobject %u tried to be created by SpellEffectSummonObject. Report to devs!", entry);
            }
            return;
        }

        GameObject* go = u_caster->GetMapMgr()->CreateGameObject(entry);
        if( go == NULL )
            return;

        if( m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION )
        {
            px = m_targets.m_dest.x;
            py = m_targets.m_dest.y;
            pz = m_targets.m_dest.z;
        }
        if( !go->CreateFromProto(entry, mapid, posx, posy, pz, orient) )
            return;

        go->SetInstanceID(m_caster->GetInstanceID());
        go->SetState(1);
        go->SetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY, m_caster->GetGUID());
        go->PushToWorld(m_caster->GetMapMgr());
        go->ExpireAndDelete(GetDuration());
        if(p_caster)
        {
            if(entry ==17032)//this is a portal
            {
                //enable it for party only
                go->SetByte(GAMEOBJECT_BYTES_1, 0, 0);

                //disable by default
                WorldPacket *pkt = go->BuildFieldUpdatePacket(GAMEOBJECT_BYTES_1, 1);
                if(SubGroup * pGroup = p_caster->GetGroup() ? p_caster->GetGroup()->GetSubGroup(p_caster->GetSubGroup()) : NULL)
                {
                    p_caster->GetGroup()->Lock();
                    for(GroupMembersSet::iterator itr = pGroup->GetGroupMembersBegin(); itr != pGroup->GetGroupMembersEnd(); itr++)
                    {
                        if((*itr)->m_loggedInPlayer && m_caster != (*itr)->m_loggedInPlayer)
                            (*itr)->m_loggedInPlayer->GetSession()->SendPacket(pkt);
                    }
                    p_caster->GetGroup()->Unlock();
                }
                delete pkt;
            }
            else if(entry == 36727 || entry == 177193) // Portal of doom
            {
                Player* pTarget = NULL;
                pTarget = p_caster->GetMapMgr()->GetPlayer(p_caster->GetSelection());
                if(pTarget == NULL)
                    return;

                go->m_ritualmembers[0] = p_caster->GetLowGUID();
                go->SetGOui32Value(GO_UINT32_M_RIT_CASTER, p_caster->GetLowGUID());
                go->SetGOui32Value(GO_UINT32_M_RIT_TARGET, pTarget->GetLowGUID());
                go->SetGOui32Value(GO_UINT32_RIT_SPELL, GetSpellProto()->Id);
            }
            else if(entry == 194108) // Ritual of Summoning
            {
                go->m_ritualmembers[0] = p_caster->GetLowGUID();
                go->SetGOui32Value(GO_UINT32_M_RIT_CASTER, p_caster->GetLowGUID());
                go->SetGOui32Value(GO_UINT32_RIT_SPELL, GetSpellProto()->Id);
            }
            else if( entry == 186811 || entry == 193062 ) // Ritual of refreshment
            {
                go->m_ritualmembers[0] = p_caster->GetLowGUID();
                go->SetGOui32Value(GO_UINT32_M_RIT_CASTER, p_caster->GetLowGUID());
                go->SetGOui32Value(GO_UINT32_RIT_SPELL, GetSpellProto()->Id);
            }
            else if( entry == 181622 || entry == 193168 ) // Ritual of Souls
            {
                go->m_ritualmembers[0] = p_caster->GetLowGUID();
                go->SetGOui32Value(GO_UINT32_M_RIT_CASTER, p_caster->GetLowGUID());
                go->SetGOui32Value(GO_UINT32_RIT_SPELL, GetSpellProto()->Id);
            } else go->charges = 10;
            p_caster->SetSummonedObject(go);
        }
    }
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
    Creature* tame = (unitTarget->IsCreature() ? castPtr<Creature>(unitTarget) : NULL);
    if(tame== NULL)
        return;

    CreatureFamilyEntry *cf = dbcCreatureFamily.LookupEntry(tame->GetCreatureData()->Family);
    uint8 result = SPELL_CANCAST_OK;

    if(!tame || !p_caster || !p_caster->isAlive() || !tame->isAlive() || p_caster->getClass() != HUNTER )
        result = SPELL_FAILED_BAD_TARGETS;
    else if(!tame->GetCreatureData())
        result = SPELL_FAILED_BAD_TARGETS;
    else if(tame->GetCreatureData()->Type != BEAST)
        result = SPELL_FAILED_BAD_TARGETS;
    else if(tame->getLevel() > p_caster->getLevel())
        result = SPELL_FAILED_HIGHLEVEL;
    else if(p_caster->GeneratePetNumber() == 0)
        result = SPELL_FAILED_BAD_TARGETS;
    else if(cf == NULL || (cf && !cf->skillLine[1]))
        result = SPELL_FAILED_BAD_TARGETS;
    else if(isExotic(cf->ID) && !p_caster->m_BeastMaster)
        result = SPELL_FAILED_BAD_TARGETS;
    else if(p_caster->GetSummon() || p_caster->GetUnstabledPetNumber())
        result = SPELL_FAILED_ALREADY_HAVE_SUMMON;

    if(result != SPELL_CANCAST_OK)
    {
        SendCastResult(result);
        return;
    }

    // Remove target
    tame->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, p_caster, 0);

    Pet* pPet = objmgr.CreatePet(tame->GetCreatureData());
    pPet->SetInstanceID(p_caster->GetInstanceID());
    pPet->SetPosition(p_caster->GetPosition());
    pPet->CreateAsSummon(tame, p_caster, NULL, NULL, 2, 0);

    // Add removal event.
    sEventMgr.AddEvent(tame, &Creature::Despawn, uint32(1), tame->GetRespawnTime(), EVENT_CORPSE_DESPAWN, 5, 0, 0);
}

void SpellEffectClass::SpellEffectSummonPet(uint32 i, WorldObject *target, int32 amount) //summon - pet
{
    if(GetSpellProto()->Id == 883)
    {
        // "Call Pet" spell
        if(p_caster== NULL )
            return;

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

    if( p_caster == NULL || p_caster->getClass() != WARLOCK)
        return;

    // remove old pet
    Pet* old = castPtr<Player>(m_caster)->GetSummon();
    if(old != NULL)
        old->Dismiss(false);

    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(GetSpellProto()->EffectMiscValue[i]);
    if(ctrData == NULL)
        return;

    //if demonic sacrifice auras are still active, remove them
    //uint32 spids[] = { 18789, 18790, 18791, 18792, 35701, 0 };
    //p_caster->RemoveAuras(spids);
    p_caster->RemoveAura(18789);
    p_caster->RemoveAura(18790);
    p_caster->RemoveAura(18791);
    p_caster->RemoveAura(18792);
    p_caster->RemoveAura(35701);

    Pet* summon = objmgr.CreatePet(ctrData);
    summon->SetInstanceID(m_caster->GetInstanceID());
    summon->SetPosition(m_caster->GetPosition());
    summon->CreateAsSummon(NULL, p_caster, NULL, GetSpellProto(), 1, 0);
    if( u_caster->IsPvPFlagged() )
        summon->SetPvPFlag();
}

void SpellEffectClass::SpellEffectWeaponDamage(uint32 i, WorldObject *target, int32 amount) // Weapon damage +
{
    if( unitTarget == NULL || u_caster == NULL )
        return;

    if( GetSpellProto()->NameHash == SPELL_HASH_MANGLE && u_caster->IsPlayer() && castPtr<Player>(u_caster)->GetShapeShift() == FORM_CAT )
        castPtr<Player>( u_caster )->AddComboPoints( unitTarget->GetGUID(), 1 );

    // Hacky fix for druid spells where it would "double attack".
    if( GetSpellProto()->Effect[2] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE || GetSpellProto()->Effect[1] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE )
    {
        add_damage += damage;
        return;
    }

    uint32 _type = MELEE;
    if( GetType() == SPELL_DMG_TYPE_RANGED )
        _type = RANGED;
    else if (GetSpellProto()->reqOffHandWeapon())
        _type =  OFFHAND;
    TotalDamage += u_caster->Strike( unitTarget, _type, GetSpellProto(), i, damage, 0, 0, false, true );
}

void SpellEffectClass::SpellEffectPowerBurn(uint32 i, WorldObject *target, int32 amount) // power burn
{
    if( unitTarget == NULL ||!unitTarget->isAlive() )
        return;
    uint32 maxMana = unitTarget->GetMaxPower(POWER_TYPE_MANA);
    if(maxMana == 0)
        return;

    //Resilience reduction
    if(playerTarget)
    {
        float dmg_reduction_pct = 2.2f * playerTarget->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE ) / 100.0f;
        if( dmg_reduction_pct > 0.33f )
            dmg_reduction_pct = 0.33f; // 3.0.3
        damage = float2int32( damage - damage * dmg_reduction_pct );
    }

    //this is pct...
    int32 mana = float2int32( unitTarget->GetPower(POWER_TYPE_MANA) * (damage/100.0f));
    unitTarget->ModPower(POWER_TYPE_MANA, -mana);

    float coef = GetSpellProto()->EffectValueMultiplier[i]; // damage per mana burned
    if(u_caster)
    {
        u_caster->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, GetSpellProto()->SpellGroupType);
        u_caster->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, GetSpellProto()->SpellGroupType);
    }
    mana = float2int32((float)mana * coef);

    m_caster->SpellNonMeleeDamageLog(unitTarget,GetSpellProto()->Id, mana, m_triggeredSpellId==0,true);
}

void SpellEffectClass::SpellEffectThreat(uint32 i, WorldObject *target, int32 amount) // Threat
{

}

void SpellEffectClass::SpellEffectTriggerSpell(uint32 i, WorldObject *target, int32 amount) // Trigger Spell
{
    if(unitTarget == NULL || m_caster == NULL )
        return;

    SpellEntry *spe = dbcSpell.LookupEntry(GetSpellProto()->EffectTriggerSpell[i]);
    if(spe == NULL )
        return;

    if(GetSpellProto() == spe) // Infinite loop fix.
        return;

    Spell* sp = new Spell( m_caster,spe,true,NULL);
    SpellCastTargets tgt(spe->isNotSelfTargettable() ? unitTarget->GetGUID() : m_caster->GetGUID());
    sp->prepare(&tgt);
}

void SpellEffectClass::SpellEffectHealthFunnel(uint32 i, WorldObject *target, int32 amount) // Health Funnel
{
//  if(unitTarget == NULL || !unitTarget->isAlive() || !unitTarget->IsPet())
//      return;

    //does not exist
}

void SpellEffectClass::SpellEffectPowerFunnel(uint32 i, WorldObject *target, int32 amount) // Power Funnel
{
//  if(unitTarget == NULL || !unitTarget->isAlive() || !unitTarget->IsPet())
//      return;

    //does not exist
}

void SpellEffectClass::SpellEffectHealMaxHealth(uint32 i, WorldObject *target, int32 amount)   // Heal Max Health
{
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    uint32 dif = unitTarget->GetUInt32Value( UNIT_FIELD_MAXHEALTH ) - unitTarget->GetUInt32Value( UNIT_FIELD_HEALTH );
    if( !dif )
    {
        SendCastResult( SPELL_FAILED_ALREADY_AT_FULL_HEALTH );
        return;
    }

    if( unitTarget->IsPlayer())
    {
         SendHealSpellOnPlayer( castPtr<Player>( m_caster ), playerTarget, dif, false, 0, m_triggeredSpellId ? m_triggeredSpellId : GetSpellProto()->Id );
    }
    unitTarget->ModUnsigned32Value( UNIT_FIELD_HEALTH, dif );
}

void SpellEffectClass::SpellEffectInterruptCast(uint32 i, WorldObject *target, int32 amount) // Interrupt Cast
{
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    // can't apply stuns/fear/polymorph/root etc on boss
    if(unitTarget->GetTypeId() == TYPEID_UNIT)
    {
        Creature* c = castPtr<Creature>( unitTarget );
        if (c && c->GetCreatureData() && (c->GetCreatureData()->Rank == ELITE_WORLDBOSS || c->GetCreatureData()->Flags & CREATURE_FLAGS1_BOSS))
            return;
    }
    // FIXME:This thing prevent target from spell casting too but cant find.
    uint32 school=0;
    if( unitTarget->GetCurrentSpell() != NULL && (unitTarget->GetCurrentSpell()->getState() == SPELL_STATE_PREPARING || unitTarget->GetCurrentSpell()->getState() == SPELL_STATE_CASTING) && unitTarget->GetCurrentSpell() != this )
    {
        school=unitTarget->GetCurrentSpell()->GetSpellProto()->School;
        unitTarget->InterruptCurrentSpell();
        if(school)//prevent from casts in this school
        {
            unitTarget->SchoolCastPrevent[school] = GetDuration() + getMSTime();
        }
    }

}

void SpellEffectClass::SpellEffectDistract(uint32 i, WorldObject *target, int32 amount) // Distract
{
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
    if( unitTarget == NULL || p_caster == NULL || unitTarget->GetTypeId() != TYPEID_UNIT)
        return;

    //Show random loot based on roll,
    Creature* target = castPtr<Creature>( unitTarget );
    if(target->IsPickPocketed() || (target->GetCreatureData() && target->GetCreatureData()->Type != HUMANOID))
    {
        SendCastResult(SPELL_FAILED_TARGET_NO_POCKETS);
        return;
    }

    lootmgr.FillPickpocketingLoot(target->GetLoot(), target->GetEntry());

    uint32 _rank = target->GetCreatureData() ? target->GetCreatureData()->Rank : 0;
    unitTarget->GetLoot()->gold = float2int32((_rank+1) * target->getLevel() * (RandomUInt(5) + 1) * sWorld.getRate(RATE_MONEY));

    p_caster->SendLoot(target->GetGUID(), target->GetMapId(), LOOT_PICKPOCKETING);
    target->SetPickPocketed(true);
}

void SpellEffectClass::SpellEffectAddFarsight(uint32 i, WorldObject *target, int32 amount) // Add Farsight
{
    if( p_caster == NULL )
        return;

    DynamicObject* dyn = p_caster->GetMapMgr()->CreateDynamicObject();
    if(dyn == NULL)
        return;

    dyn->Create(p_caster, this, m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, GetDuration(), GetRadius(i));
    dyn->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dyn->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x80000002);
    dyn->PushToWorld(p_caster->GetMapMgr());
    p_caster->SetUInt64Value(PLAYER_FARSIGHT, dyn->GetGUID());
    p_caster->SetUInt32Value(PLAYER_FARSIGHT, dyn->GetLowGUID());
    p_caster->GetMapMgr()->ChangeFarsightLocation(p_caster, m_targets.m_dest.x, m_targets.m_dest.y, true);
}

void SpellEffectClass::SpellEffectResetTalents(uint32 i, WorldObject *target, int32 amount) // Used by Trainers.
{
    if( !playerTarget )
        return;

    playerTarget->ResetSpec(playerTarget->m_talentInterface.GetActiveSpec());
}

void SpellEffectClass::SpellEffectUseGlyph(uint32 i, WorldObject *target, int32 amount)
{
    if( p_caster == NULL )
        return;

    if(uint8 result = p_caster->m_talentInterface.ApplyGlyph(m_targets.m_targetIndex, GetSpellProto()->EffectMiscValue[i])) // there was an error
        SendCastResult(result);
    else p_caster->m_talentInterface.SendTalentInfo();
}

void SpellEffectClass::SpellEffectHealMechanical(uint32 i, WorldObject *target, int32 amount)
{
    if(unitTarget == NULL || unitTarget->GetCreatureType() != MECHANICAL)
        return;

    Heal(i, (int32)damage);
}

void SpellEffectClass::SpellEffectSummonObjectWild(uint32 i, WorldObject *target, int32 amount)
{
    if(u_caster == NULL )
        return;

    // spawn a new one
    GameObject* GoSummon = u_caster->GetMapMgr()->CreateGameObject(GetSpellProto()->EffectMiscValue[i]);
    if( GoSummon == NULL || !GoSummon->CreateFromProto(GetSpellProto()->EffectMiscValue[i], m_caster->GetMapId(), m_caster->GetPosition()))
        return;

    GoSummon->SetUInt32Value(GAMEOBJECT_LEVEL, u_caster->getLevel());
    GoSummon->SetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY, m_caster->GetGUID());
    GoSummon->SetState(0);
    GoSummon->PushToWorld(u_caster->GetMapMgr());
    GoSummon->SetSummoned(u_caster);
    GoSummon->ExpireAndDelete(GetDuration());
}

void SpellEffectClass::SpellEffectScriptEffect(uint32 i, WorldObject *target, int32 amount) // Script Effect
{
    sLog.outDebug("Unhandled Scripted Effect In Spell %u", m_spellInfo->Id);
}

void SpellEffectClass::SpellEffectSanctuary(uint32 i, WorldObject *target, int32 amount) // Stop all attacks made to you
{
    if( unitTarget == NULL )
        return;

    WorldObject::InRangeSet::iterator itr, it2;
    for( itr = unitTarget->GetInRangeUnitSetBegin(); itr != unitTarget->GetInRangeUnitSetEnd(); )
    {
        it2 = itr++;
        if(castPtr<Unit>(*it2)->IsPlayer())
            continue;
        castPtr<Unit>(*it2)->GetAIInterface()->RemoveThreat(unitTarget->GetGUID());
    }
    

    // also cancel any spells we are casting
    if( unitTarget->GetCurrentSpell() != NULL && unitTarget->GetCurrentSpell() != this && unitTarget->GetCurrentSpell()->getState() == SPELL_STATE_PREPARING )
        unitTarget->InterruptCurrentSpell();
    unitTarget->smsg_AttackStop( unitTarget );

    if( playerTarget != NULL )
        playerTarget->EventAttackStop();
}

void SpellEffectClass::SpellEffectAddComboPoints(uint32 i, WorldObject *target, int32 amount) // Add Combo Points
{
    if(p_caster == NULL)
        return;

    p_caster->AddComboPoints(p_caster->GetSelection(), damage);
}

void SpellEffectClass::SpellEffectCreateHouse(uint32 i, WorldObject *target, int32 amount) // Create House
{


}

void SpellEffectClass::SpellEffectDuel(uint32 i, WorldObject *target, int32 amount) // Duel
{
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

    if (p_caster->InStealth())
    {
        SendCastResult(SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED);
        return; // Player is stealth // Crow: No shit?
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
    if( playerTarget == NULL || playerTarget != p_caster)
        return;

    uint32 mapid = playerTarget->GetBindMapId();
    float x = playerTarget->GetBindPositionX();
    float y = playerTarget->GetBindPositionY();
    float z = playerTarget->GetBindPositionZ();
    float orientation = 0;

    sEventMgr.AddEvent(playerTarget,&Player::EventTeleport,mapid,x,y,z,orientation,EVENT_PLAYER_TELEPORT,50,1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void SpellEffectClass::SpellEffectSummonPlayer(uint32 i, WorldObject *target, int32 amount)
{
    if( playerTarget == NULL)
        return;

    if(m_caster->GetMapMgr()->GetMapInfo() && m_caster->GetMapMgr()->GetMapInfo()->type != INSTANCE_NULL)
        return;

    playerTarget->SummonRequest(m_caster, m_caster->GetZoneId(), m_caster->GetMapId(),
        m_caster->GetInstanceID(), m_caster->GetPosition());
}

void SpellEffectClass::SpellEffectActivateObject(uint32 i, WorldObject *target, int32 amount) // Activate WorldObject
{

}

void SpellEffectClass::SpellEffectWMODamage(uint32 i, WorldObject *target, int32 amount)
{
    if(p_caster == NULL && v_caster && v_caster->GetControllingPlayer() )
        p_caster = castPtr<Player>(v_caster->GetControllingPlayer());
    if(gameObjTarget == NULL)
    {
        DamageGosAround(i);
        return;
    }

    gameObjTarget->TakeDamage(damage,m_caster,p_caster, GetSpellProto()->Id);
}

void SpellEffectClass::SpellEffectWMORepair(uint32 i, WorldObject *target, int32 amount)
{
    if(gameObjTarget == NULL)
        return;

    gameObjTarget->SetStatusRebuilt();
}

void SpellEffectClass::SpellEffectChangeWMOState(uint32 i, WorldObject *target, int32 amount)
{
    if(gameObjTarget == NULL)
        return;

    switch(GetSpellProto()->EffectMiscValue[i])
    {
    case 1:
        {
            gameObjTarget->SetStatusDamaged();
        }break;
    case 2:
        {
            gameObjTarget->SetStatusDestroyed();
        }break;
    default:
        {
            gameObjTarget->SetStatusRebuilt();
        }break;
    }
}

void SpellEffectClass::SpellEffectSelfResurrect(uint32 i, WorldObject *target, int32 amount)
{
    if( p_caster == NULL || playerTarget->isAlive() || playerTarget->PreventRes)
        return;
    uint32 mana;
    uint32 health;
    uint32 class_=unitTarget->getClass();

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
            mana = -damage;
        }break;
    case 21169: //Reincarnation. Ressurect with 20% health and mana
        {
            health = uint32((unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*damage)/100);
            mana = uint32((unitTarget->GetMaxPower(POWER_TYPE_MANA)*damage)/100);
        }
        break;
    default:
        {
            if(damage < 0) return;
            health = uint32(unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*damage/100);
            mana = uint32(unitTarget->GetMaxPower(POWER_TYPE_MANA)*damage/100);
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

    if(GetSpellProto()->Id==21169)
        AddCooldown();
}

void SpellEffectClass::SpellEffectSkinning(uint32 i, WorldObject *target, int32 amount)
{
    if( unitTarget == NULL || !unitTarget->IsCreature() || p_caster == NULL )
        return;

    Creature* cr = castPtr<Creature>( unitTarget );
    uint32 skill = cr->GetRequiredLootSkill();

    uint32 sk = castPtr<Player>( m_caster )->_GetSkillLineCurrent( skill );
    uint32 lvl = cr->getLevel();

    if( ( sk >= lvl * 5 ) || ( ( sk + 100 ) >= lvl * 10 ) )
    {
        //Fill loot for Skinning
        lootmgr.FillGatheringLoot(cr->GetLoot(), cr->GetEntry());
        castPtr<Player>( m_caster )->SendLoot( cr->GetGUID(), cr->GetMapId(), LOOT_SKINNING );

        //Not skinable again
        cr->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        cr->Skinned = true;
        //double chance from elite
        if(cr->GetCreatureData()->Rank > 0)
            DetermineSkillUp(skill ,sk < lvl * 5 ? sk/5 : lvl, 2);
        else
            DetermineSkillUp(skill ,sk < lvl * 5 ? sk/5 : lvl, 1);
    }
    else
        SendCastResult(SPELL_FAILED_TARGET_UNSKINNABLE);
}

void SpellEffectClass::SpellEffectCharge(uint32 i, WorldObject *target, int32 amount)
{
    if( unitTarget == NULL || !u_caster || !unitTarget->isAlive())
        return;

    if (u_caster->IsStunned() || u_caster->m_rooted || u_caster->IsPacified() || u_caster->IsFeared())
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
        if(!unitTarget)
            return;
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
    if(unitTarget)
        u_caster->smsg_AttackStart( unitTarget );
    u_caster->resetAttackTimer(0xFF);
    if(p_caster) p_caster->EventAttackStart();
}

void SpellEffectClass::SpellEffectPlaceTotemsOnBar(uint32 i, WorldObject *target, int32 amount)
{
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
        if(!IsTotemSpell(sp) || p_caster->SpellHasCooldown(spell))
            continue;
        Spell* pSpell = new Spell(p_caster, sp, true, NULL);
        if(!pSpell->HasPower())
            continue;

        SpellCastTargets targets;
        pSpell->GenerateTargets(&targets);
        pSpell->prepare(&targets);
    }
}

void SpellEffectClass::SpellEffectSendTaxi(uint32 i, WorldObject *target, int32 amount)
{
    if ( playerTarget == NULL || !playerTarget->isAlive() )
        return;

    if( playerTarget->HasFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER ) )
        return;

    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath( GetSpellProto()->EffectMiscValue[i] );

    if( !taxipath )
        return;

    TaxiNode* taxinode = sTaxiMgr.GetTaxiNode( taxipath->GetSourceNode() );

    if( !taxinode )
        return;

    uint32 mount_entry = 0;
    uint32 modelid = 0;

    if( playerTarget->GetTeam() )       // HORDE
    {
        mount_entry = taxinode->horde_mount;
        if( !mount_entry )
            mount_entry = taxinode->alliance_mount;

        CreatureData* ctrData = sCreatureDataMgr.GetCreatureData( mount_entry );
        if( ctrData == NULL )
            return;
        if((modelid = ctrData->DisplayInfo[1]) == 0)
            return;
    }
    else                                // ALLIANCE
    {
        mount_entry = taxinode->alliance_mount;
        if( !mount_entry )
            mount_entry = taxinode->horde_mount;

        CreatureData* ctrData = sCreatureDataMgr.GetCreatureData( mount_entry );
        if( ctrData == NULL )
            return;
        if((modelid = ctrData->DisplayInfo[0]) == 0)
            return;
    }

    playerTarget->TaxiStart( taxipath, modelid, 0 );
}

void SpellEffectClass::SpellEffectPull(uint32 i, WorldObject *target, int32 amount)
{
    if( unitTarget == NULL && u_caster != NULL)
        unitTarget = u_caster;
    if(unitTarget == NULL)
        return;

    if(unitTarget->IsCreature() && isTargetDummy(unitTarget->GetEntry()))
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

    int32 basePoints = 0;
    basePoints += CalculateEffect(i, unitTarget, basePoints);
    uint32 time = uint32((basePoints / arc) * 100);
    unitTarget->GetAIInterface()->StopMovement(time);
    unitTarget->SetPosition(pullX, pullY, pullZ, 0.0f);
    unitTarget->GetAIInterface()->SendJumpTo(pullX, pullY, pullZ, time, arc);
    if(playerTarget)
    {
        if( playerTarget->IsPvPFlagged() )
            u_caster->SetPvPFlag();
        if( playerTarget->IsFFAPvPFlagged() )
            u_caster->SetFFAPvPFlag();
    }
}

void SpellEffectClass::SpellEffectKnockBack(uint32 i, WorldObject *target, int32 amount)
{
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    if(unitTarget->IsCreature() && isTargetDummy(unitTarget->GetEntry()))
        return;

    unitTarget->knockback(GetSpellProto()->EffectBasePoints[i]+1, GetSpellProto()->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectDisenchant(uint32 i, WorldObject *target, int32 amount)
{
    if(p_caster == NULL)
        return;

    Item* it = p_caster->GetInventory()->GetInventoryItem(m_targets.m_itemTarget);
    if( it == NULL )
    {
        SendCastResult(SPELL_FAILED_CANT_BE_DISENCHANTED);
        return;
    }

    if( !it->IsLooted() )
    {
        lootmgr.FillItemLoot(it->GetLoot(), it->GetEntry(), p_caster->GetTeam());

        if( it->GetLoot()->items.size() > 0 )
        {
            //Check for skill, we can increase it upto 75
            uint32 skill = p_caster->_GetSkillLineCurrent( SKILL_ENCHANTING );
            if(skill < 75 && Rand(100.f-float(skill)*100.f/75.f))
                p_caster->_AdvanceSkillLine(SKILL_ENCHANTING, float2int32( 1.0f * sWorld.getRate(RATE_SKILLRATE)));

            sLog.outDebug("SpellEffect","Succesfully disenchanted item %d", uint32(itemTarget->GetEntry()));
            p_caster->SendLoot( itemTarget->GetGUID(), itemTarget->GetOwner()->GetMapId(), LOOT_DISENCHANTING );
        }
        else
        {
            sLog.outDebug("SpellEffect","Disenchanting failed, item %d has no loot", uint32(itemTarget->GetEntry()));
            SendCastResult(SPELL_FAILED_CANT_BE_DISENCHANTED);
            return;
        }
        // delete from db so it won't be re-saved
        sItemMgr.DeleteItemFromDatabase(it->GetGUID(), ITEM_DELETION_DISENCHANTED);
        it->SetLooted();
    }
    if(it == i_caster)
        i_caster = NULL;
}

void SpellEffectClass::SpellEffectInebriate(uint32 i, WorldObject *target, int32 amount) // lets get drunk!
{
    if( playerTarget == NULL )
        return;

    uint16 currentDrunk = playerTarget->m_drunk;
    uint16 drunkMod = damage * 256;
    if (currentDrunk + drunkMod > 0xFFFF)
        currentDrunk = 0xFFFF;
    else
        currentDrunk += drunkMod;
    playerTarget->SetDrunk(currentDrunk, i_caster ? i_caster->GetProto()->ItemId : 0);
}

void SpellEffectClass::SpellEffectFeedPet(uint32 i, WorldObject *target, int32 amount)  // Feed Pet
{
    // food flags and food level are checked in Spell::CanCast()
    if( itemTarget == NULL || p_caster == NULL )
        return;

    Pet* pPet = NULL;
    pPet = p_caster->GetSummon();
    if(pPet== NULL)
        return;

    //// Cast feed pet effect
    // effect is item level and pet level dependent, aura ticks are 35, 17, 8 (*1000) happiness
    // http://petopia.brashendeavors.net/html/articles/basics_feeding.shtml
    int8 deltaLvl = pPet->getLevel() - itemTarget->GetProto()->ItemLevel;
    damage /= 1000; //damage of Feed pet spell is 35000
    if(deltaLvl > 20)
        damage >>= 2;
    else if(deltaLvl > 10)
        damage >>= 1;//divide by 2
    damage *= 1000;

    if(itemTarget->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > 1)
        itemTarget->ModifyStackSize(-1);
    else
    {
        Item *rItem = p_caster->GetInventory()->RemoveInventoryItem(itemTarget->GetInventorySlot());
        if(rItem != itemTarget)
            return;

        sItemMgr.DeleteItemFromDatabase(rItem->GetGUID(), ITEM_DELETION_USED);
        sItemMgr.DeleteItemData(rItem->GetGUID(), true);
        rItem->Destruct();
    }

    Spell* sp = new Spell(p_caster, dbcSpell.LookupEntry(GetSpellProto()->EffectTriggerSpell[i]), true, NULL);
    SpellCastTargets tgt;
    tgt.m_unitTarget = pPet->GetGUID();
    sp->prepare(&tgt);
}

void SpellEffectClass::SpellEffectReputation(uint32 i, WorldObject *target, int32 amount)
{
    if( playerTarget == NULL)
        return;

    playerTarget->ModStanding(GetSpellProto()->EffectMiscValue[i], damage);
}

void SpellEffectClass::SpellEffectSummonObjectSlot(uint32 i, WorldObject *target, int32 amount)
{
    if( u_caster  == NULL || !u_caster->IsInWorld())
        return;

    GameObjectInfo * goi = NULL;
    goi = GameObjectNameStorage.LookupEntry(GetSpellProto()->EffectMiscValue[i]);
    if( goi == NULL )
    {
        if( p_caster != NULL )
        {
            sChatHandler.BlueSystemMessage(p_caster->GetSession(),
            "non-existant gameobject %u tried to be created by SpellEffectSummonObjectSlot. Report to devs!", GetSpellProto()->EffectMiscValue[i]);
        }
        return;
    }

    GameObject* GoSummon = NULL;
    uint32 slot = GetSpellProto()->Effect[i] - SPELL_EFFECT_SUMMON_OBJECT_SLOT1;
    GoSummon = u_caster->m_ObjectSlots[slot] ? u_caster->GetMapMgr()->GetGameObject(u_caster->m_ObjectSlots[slot]) : NULL;
    u_caster->m_ObjectSlots[slot] = 0;

    //slot allready used, empty it now.
    if( GoSummon != NULL)
    {
        if(GoSummon->GetInstanceID() != u_caster->GetInstanceID())
            GoSummon->ExpireAndDelete();
        else
        {
            if( GoSummon->IsInWorld() )
                GoSummon->RemoveFromWorld(true);

            GoSummon->Destruct();
            GoSummon = NULL;
        }
    }

    //create a new GoSummon
    GoSummon = u_caster->GetMapMgr()->CreateGameObject( goi->ID );
    if(GoSummon == NULL)
        return;
    GoSummon->SetInstanceID( u_caster->GetInstanceID() );
    if( GetSpellProto()->EffectImplicitTargetA[i] == EFF_TARGET_SIMPLE_AOE )
        GoSummon->CreateFromProto( goi->ID, m_caster->GetMapId(), m_targets.m_dest.x, m_targets.m_dest.y, m_targets.m_dest.z, m_caster->GetOrientation());
    else GoSummon->CreateFromProto( goi->ID, m_caster->GetMapId(), m_caster->GetPosition());

    GoSummon->SetUInt32Value(GAMEOBJECT_LEVEL, u_caster->getLevel());

    if(GoSummon->GetType() == GAMEOBJECT_TYPE_TRAP)
    {
        GoSummon->SetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY, u_caster->GetGUID());
        GoSummon->invisible = true;
        GoSummon->invisibilityFlag = INVIS_FLAG_TRAP;
        GoSummon->charges = 1;
        GoSummon->checkrate = 1;
        sEventMgr.AddEvent(GoSummon, &GameObject::TrapSearchTarget, EVENT_GAMEOBJECT_TRAP_SEARCH_TARGET, 100, 0,0);
    }
    else
        GoSummon->ExpireAndDelete(GetDuration());

    GoSummon->PushToWorld(m_caster->GetMapMgr());
    GoSummon->SetSummoned(u_caster);
    u_caster->m_ObjectSlots[slot] = GoSummon->GetLowGUID();
}

void SpellEffectClass::SpellEffectDispelMechanic(uint32 i, WorldObject *target, int32 amount)
{
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    int32 sMisc = (int32)GetSpellProto()->EffectMiscValue[i];
    unitTarget->m_AuraInterface.AttemptDispel(u_caster, sMisc, (unitTarget == u_caster || !sFactionSystem.isAttackable( u_caster, unitTarget )));

    if( playerTarget && GetSpellProto()->NameHash == SPELL_HASH_DAZED && playerTarget->IsMounted() )
        playerTarget->Dismount();
}

void SpellEffectClass::SpellEffectSummonDeadPet(uint32 i, WorldObject *target, int32 amount)
{//this is pet resurrect
    if(p_caster == NULL )
        return;
    Pet* pPet = p_caster->GetSummon();
    if(pPet)
    {
        pPet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
        pPet->SetUInt32Value(UNIT_FIELD_HEALTH, (uint32)((pPet->GetUInt32Value(UNIT_FIELD_MAXHEALTH) * damage) / 100));
        pPet->SetDeathState(ALIVE);
        pPet->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, pPet, 0);
        sEventMgr.RemoveEvents(pPet, EVENT_PET_DELAYED_REMOVE);
    }
}

uint32 TotemSpells[4] = { 63, 81, 82, 83 };
void SpellEffectClass::SpellEffectDestroyAllTotems(uint32 i, WorldObject *target, int32 amount)
{
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
                        energize_amt += float2int32( (sp->ManaCost) * (damage/100.0f) );
                    else if( sp->ManaCostPercentage )
                        energize_amt += float2int32(((p_caster->GetUInt32Value(UNIT_FIELD_BASE_MANA)*sp->ManaCostPercentage)/100.0f) * (damage/100.0f) );
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
    if(p_caster == NULL ) //p_caster->getClass() != WARLOCK ) //summoning a demon shouldn't be warlock only, see spells 25005, 24934, 24810 etc etc
        return;

    Pet* pPet = p_caster->GetSummon();
    if(pPet!= NULL)
    {
        pPet->Dismiss(false);
        pPet = NULL;
    }

    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(GetSpellProto()->EffectMiscValue[i]);
    if(ctrData == NULL)
        return;

    pPet = objmgr.CreatePet(ctrData);
    pPet->SetInstanceID(p_caster->GetInstanceID());
    pPet->CreateAsSummon(NULL, p_caster, NULL, GetSpellProto(), 1, 300000);

    //Create Enslave Aura if its inferno spell
    if(GetSpellProto()->Id == 1122)
    {
        SpellEntry *spellInfo = dbcSpell.LookupEntry(11726);

        Spell* sp = new Spell(pPet,spellInfo,true,NULL);
        SpellCastTargets tgt;
        tgt.m_unitTarget=pPet->GetGUID();
        sp->prepare(&tgt);
    }
}

void SpellEffectClass::SpellEffectResurrect(uint32 i, WorldObject *target, int32 amount) // Resurrect (Flat)
{
    if( playerTarget == NULL )
    {
        if( corpseTarget == NULL )
        {
            // unit resurrection handler
            if(unitTarget != NULL )
            {
                if(unitTarget->GetTypeId()==TYPEID_UNIT && unitTarget->IsPet() && unitTarget->isDead())
                {
                    uint32 hlth = ((uint32)GetSpellProto()->EffectBasePoints[i] > unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH)) ? unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH) : (uint32)GetSpellProto()->EffectBasePoints[i];
                    uint32 mana = ((uint32)GetSpellProto()->EffectBasePoints[i] > unitTarget->GetMaxPower(POWER_TYPE_MANA)) ? unitTarget->GetMaxPower(POWER_TYPE_MANA) : (uint32)GetSpellProto()->EffectBasePoints[i];

                    if(!unitTarget->IsPet())
                    {
                        sEventMgr.RemoveEvents(unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    else
                    {
                        sEventMgr.RemoveEvents(unitTarget, EVENT_PET_DELAYED_REMOVE);
                        sEventMgr.RemoveEvents(unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH, hlth);
                    unitTarget->SetPower(POWER_TYPE_MANA, mana);
                    unitTarget->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
                    unitTarget->SetDeathState(ALIVE);
                    unitTarget->ClearLoot();
                    castPtr<Creature>(unitTarget)->ClearTag();
                }
            }

            return;
        }
        playerTarget = objmgr.GetPlayer(corpseTarget->GetUInt64Value(CORPSE_FIELD_OWNER));
        if( playerTarget == NULL )
            return;
    }

    if(playerTarget->isAlive() || !playerTarget->IsInWorld() || playerTarget->PreventRes)
        return;

    uint32 health = GetSpellProto()->EffectBasePoints[i];
    uint32 mana = GetSpellProto()->EffectMiscValue[i];

    playerTarget->m_resurrectHealth = health;
    playerTarget->m_resurrectMana = mana;

    SendResurrectRequest(playerTarget);
}

void SpellEffectClass::SpellEffectAttackMe(uint32 i, WorldObject *target, int32 amount)
{
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    unitTarget->GetAIInterface()->AttackReaction(u_caster,1,0);
}

void SpellEffectClass::SpellEffectSkinPlayerCorpse(uint32 i, WorldObject *target, int32 amount)
{
    Corpse* corpse = NULL;
    if(playerTarget == NULL)
    {
        // means we're "skinning" a corpse
        corpse = objmgr.GetCorpse(m_targets.m_unitTarget.getLow());  // hacky
    }
    else if(playerTarget->getDeathState() == CORPSE)    // repopped while we were casting
    {
        corpse = objmgr.GetCorpse(playerTarget->GetLowGUID());
    }

    if(m_caster == NULL || !m_caster->IsPlayer())
        return;

    if(playerTarget != NULL && corpse == NULL )
    {
        if(!playerTarget->isDead() || playerTarget->m_insigniaTaken)
            return;

        // Set all the lootable stuff on the player. If he repops before we've looted, we'll set the flags
        // on corpse then :p
        playerTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

        // don't allow him to spawn a corpse
        playerTarget->m_insigniaTaken = true;

        // and.. force him to the graveyard and repop him.
        // this will also convert the corpse to bones.
        Corpse* pCorpse = playerTarget->RepopRequestedPlayer();

        // sanity checks
        if( pCorpse == NULL )
        {
            playerTarget->m_insigniaTaken = false;
            return;
        }

        // Send a message to the died player, telling him he has to resurrect at the graveyard.
        // Send an empty corpse location too, :P
        playerTarget->GetSession()->OutPacket(SMSG_PLAYER_SKINNED, 1, "\x00");
        playerTarget->GetSession()->OutPacket(MSG_CORPSE_QUERY, 1, "\x00");

        // fill loot vector
        playerTarget->GenerateLoot(pCorpse);

        // flag as lootable
        pCorpse->SetUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS, CORPSE_DYN_FLAG_LOOTABLE);

        // send the corpse's loot
        if( pCorpse != NULL )       // should never be null but /shrug
            p_caster->SendLoot(pCorpse->GetGUID(), pCorpse->GetMapId(), LOOT_CORPSE);

    }
    else if(corpse!= NULL)
    {
        if(corpse->GetUInt32Value(CORPSE_FIELD_FLAGS) != 60)
        {
            // cheater?
            return;
        }

        // find the corpses' owner
        Player* owner = NULL;
        //must be on same map!!
        owner = m_caster->GetMapMgr()->GetPlayer(corpse->GetUInt64Value(CORPSE_FIELD_OWNER));
        if(owner == NULL || owner->m_bg == NULL)
            return;

        owner->GetSession()->OutPacket(SMSG_PLAYER_SKINNED, 1, "\x00");
        owner->GetSession()->OutPacket(MSG_CORPSE_QUERY, 1, "\x00");

        // fill loot vector
        owner->GenerateLoot(corpse);

        // set lootable flag
        corpse->SetUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS, 1); // sets it so you can loot the plyr

        // remove skinnable flag
        corpse->SetUInt32Value(CORPSE_FIELD_FLAGS, 5);

        // remove owner association
        corpse->SetUInt64Value(CORPSE_FIELD_OWNER, 0);
        corpse->SetCorpseState(CORPSE_STATE_BONES);
        corpse->DeleteFromDB();
        objmgr.CorpseAddEventDespawn(corpse);

        // send loot
        p_caster->SendLoot(corpse->GetGUID(), corpse->GetMapId(), LOOT_CORPSE);
    }
}

void SpellEffectClass::SpellEffectSkill(uint32 i, WorldObject *target, int32 amount)
{
    // Used by professions only
    // Effect should be renamed in RequireSkill
    if(p_caster == NULL )
        return;

    uint32 val = damage * 75 > 450 ? 450 :damage * 75;

    if( p_caster->_GetSkillLineMax(GetSpellProto()->EffectMiscValue[i]) >= val )
        return;

    if( GetSpellProto()->EffectMiscValue[i] == SKILL_RIDING )
        p_caster->_AddSkillLine( GetSpellProto()->EffectMiscValue[i], val, val );
    else
    {
        if( p_caster->_HasSkillLine(GetSpellProto()->EffectMiscValue[i]) )
            p_caster->_ModifySkillMaximum(GetSpellProto()->EffectMiscValue[i], val);
        else
            p_caster->_AddSkillLine( GetSpellProto()->EffectMiscValue[i], 1, val);
    }
}

void SpellEffectClass::SpellEffectApplyPetAura(uint32 i, WorldObject *target, int32 amount)
{
    SpellEffectApplyAura(i);
}

void SpellEffectClass::SpellEffectDummyMelee(uint32 i, WorldObject *target, int32 amount) // Normalized Weapon damage +
{

    if( unitTarget == NULL || u_caster == NULL )
        return;

    uint32 pct_dmg_mod = 100;
    if( p_caster != NULL &&  GetSpellProto()->NameHash == SPELL_HASH_OVERPOWER) //warrior : overpower - let us clear the event and the combopoint count
    {
        p_caster->NullComboPoints(); //some say that we should only remove 1 point per dodge. Due to cooldown you can't cast it twice anyway..
    }
    else if( GetSpellProto()->NameHash == SPELL_HASH_DEVASTATE)
    {
        // Player can apply only 58567 Sunder Armor effect.
        if(Aura* aura = u_caster->m_AuraInterface.FindActiveAura(58567))
        {
            aura->AddStackSize(u_caster->HasAura(58388) ? 2 : 1);
            damage *= aura->getStackSize();
        } else u_caster->CastSpell(unitTarget, 58567, true);
    }
    // rogue - mutilate ads dmg if target is poisoned
    // pure hax (damage join)
    else if( GetSpellProto()->NameHash == SPELL_HASH_MUTILATE )
    {
        damage = damage * 2;
        damage += sStatSystem.CalculateDamage(u_caster, unitTarget, MELEE, GetSpellProto());
        damage += sStatSystem.CalculateDamage(u_caster, unitTarget, OFFHAND, GetSpellProto());

        if( unitTarget->IsPoisoned() )
            pct_dmg_mod = 120;
    }
    //Hemorrhage
    else if( p_caster != NULL && GetSpellProto()->NameHash == SPELL_HASH_HEMORRHAGE )
        p_caster->AddComboPoints(p_caster->GetSelection(), 1);

    switch( GetSpellProto()->Id )
    {
    // AMBUSH
    case 8676: add_damage = 101; return; break;         // r1
    case 8724: add_damage = 145; return; break;         // r2
    case 8725: add_damage = 180; return; break;         // r3
    case 11267: add_damage = 269; return; break;        // r4
    case 11268: add_damage = 332; return; break;        // r5
    case 11269: add_damage = 420; return; break;        // r6
    case 27441: add_damage = 487; return; break;        // r7
    case 48689: add_damage = 670; return; break;        // r8
    case 48690: add_damage = 1018; return; break;       // r9
    case 48691: add_damage = 1198; return; break;       // r10

    // BACKSTAB
    case 53: add_damage = 15; return; break;            // r1
    case 2589: add_damage = 30; return; break;          // r2
    case 2590: add_damage = 48; return; break;          // r3
    case 2591: add_damage = 69; return; break;          // r4
    case 8721: add_damage = 90; return; break;          // r5
    case 11279: add_damage = 135; return; break;        // r6
    case 11280: add_damage = 165; return; break;        // r7
    case 11281: add_damage = 210; return; break;        // r8
    case 25300: add_damage = 225; return; break;        // r9
    case 26863: add_damage = 255; return; break;        // r10
    case 48656: add_damage = 383; return; break;        // r11
    case 48657: add_damage = 465; return; break;        // r12
    }

    // rogue ambush etc
    for (uint32 x =0;x<3;x++)
    {
        if(GetSpellProto()->Effect[x] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)
        {
            add_damage = damage * (GetSpellProto()->EffectBasePoints[x]+1) /100;
            return;
        }
    }

    uint32 _type = MELEE;
    if( GetType() == SPELL_DMG_TYPE_RANGED )
        _type = RANGED;
    else  if (GetSpellProto()->reqOffHandWeapon())
        _type =  OFFHAND;
    u_caster->Strike( unitTarget, _type, GetSpellProto(), i, damage, pct_dmg_mod, 0, false, false );
}

void SpellEffectClass::SpellEffectSpellSteal(uint32 i, WorldObject *target, int32 amount)
{
    if ( unitTarget  == NULL ||  u_caster  == NULL || !unitTarget->isAlive())
        return;

    if( p_caster && p_caster->GetGUID() != unitTarget->GetGUID() )
    {
        if( unitTarget->IsPvPFlagged() )
        {
            if( p_caster->IsPlayer() )
                castPtr<Player>( p_caster )->PvPToggle();
            else
                p_caster->SetPvPFlag();
        }
    }

    if(!sFactionSystem.isAttackable(u_caster,unitTarget))
        return;

    unitTarget->m_AuraInterface.SpellStealAuras(u_caster, damage);
}

void SpellEffectClass::SpellEffectProspecting(uint32 i, WorldObject *target, int32 amount)
{
    if( p_caster == NULL)
        return;

    if( itemTarget == NULL) // this should never happen
    {
        SendCastResult(SPELL_FAILED_ITEM_GONE);
        return;
    }

    uint32 entry = itemTarget->GetEntry();

    if(p_caster->GetInventory()->RemoveInventoryStacks(entry, 5, false))
    {
        p_caster->SetLootGUID(p_caster->GetGUID());
        lootmgr.FillItemLoot(p_caster->GetLoot(), entry, p_caster->GetTeam());
        p_caster->SendLoot(p_caster->GetGUID(), p_caster->GetMapId(), LOOT_PROSPECTING);
    }
    else // this should never happen either
        SendCastResult(SPELL_FAILED_NEED_MORE_ITEMS);
}

void SpellEffectClass::SpellEffectResurrectNew(uint32 i, WorldObject *target, int32 amount)
{
    if(!u_caster)
        return;
    //base p =hp,misc mana
    if( playerTarget == NULL)
    {
        if( corpseTarget == NULL)
        {
            // unit resurrection handler
            if(unitTarget != NULL)
            {
                if(unitTarget->GetTypeId()==TYPEID_UNIT && unitTarget->IsPet() && unitTarget->isDead())
                {
                    uint32 hlth = ((uint32)GetSpellProto()->EffectBasePoints[i] > unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH)) ? unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH) : (uint32)GetSpellProto()->EffectBasePoints[i];
                    uint32 mana = ((uint32)GetSpellProto()->EffectBasePoints[i] > unitTarget->GetMaxPower(POWER_TYPE_MANA)) ? unitTarget->GetMaxPower(POWER_TYPE_MANA) : (uint32)GetSpellProto()->EffectBasePoints[i];

                    if(!unitTarget->IsPet())
                    {
                        sEventMgr.RemoveEvents(unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    else
                    {
                        sEventMgr.RemoveEvents(unitTarget, EVENT_PET_DELAYED_REMOVE);
                        sEventMgr.RemoveEvents(unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH, hlth);
                    unitTarget->SetPower(POWER_TYPE_MANA, mana);
                    unitTarget->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
                    unitTarget->SetDeathState(ALIVE);
                    unitTarget->ClearLoot();
                    castPtr<Creature>(unitTarget)->ClearTag();
                }
            }

            return;
        }
        playerTarget = objmgr.GetPlayer(corpseTarget->GetUInt64Value(CORPSE_FIELD_OWNER));
        if(playerTarget == NULL)
            return;
    }

    if(playerTarget->isAlive() || !playerTarget->IsInWorld() || playerTarget->PreventRes)
        return;
    //resurr
    playerTarget->resurrector = u_caster->GetLowGUID();
    playerTarget->m_resurrectHealth = damage;
    playerTarget->m_resurrectMana = GetSpellProto()->EffectMiscValue[i];
    playerTarget->m_resurrectLoction = u_caster->GetPosition();

    SendResurrectRequest(playerTarget);
}

void SpellEffectClass::SpellEffectTranformItem(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectEnvironmentalDamage(uint32 i, WorldObject *target, int32 amount)
{
    if(playerTarget == NULL)
        return;

    if(playerTarget->SchoolImmunityList[GetSpellProto()->School])
    {
        SendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }

    //this is GO, not unit
    m_caster->SpellNonMeleeDamageLog(playerTarget,GetSpellProto()->Id,damage, m_triggeredSpellId==0);

    WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, 13);
    data << playerTarget->GetGUID();
    data << uint8(DAMAGE_FIRE);
    data << uint32(damage);
    playerTarget->SendMessageToSet( &data, true );
}

void SpellEffectClass::SpellEffectDismissPet(uint32 i, WorldObject *target, int32 amount)
{
    // remove pet.. but don't delete so it can be called later
    if( p_caster == NULL)
        return;

    Pet* pPet = NULL;
    pPet = p_caster->GetSummon();
    if(pPet == NULL)
        return;
    pPet->Remove(true, true, true);
}

void SpellEffectClass::SpellEffectEnchantHeldItem(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectAddHonor(uint32 i, WorldObject *target, int32 amount)
{
    if( playerTarget != NULL )
        HonorHandler::AddHonorPointsToPlayer( playerTarget, GetSpellProto()->EffectBasePoints[i] );
}

void SpellEffectClass::SpellEffectSpawn(uint32 i, WorldObject *target, int32 amount)
{
    // this effect is mostly for custom teleporting
    switch(GetSpellProto()->Id)
    {
        case 10418: // Arugal spawn-in spell , teleports it to 3 locations randomly sneeking players (bastard ;P)
        {
            if( u_caster == NULL || u_caster->IsPlayer())
                return;

            static float coord[3][3]= {{-108.9034f,2129.5678f,144.9210f},{-108.9034f,2155.5678f,155.678f},{-77.9034f,2155.5678f,155.678f}};

            int i = (int)(rand()%3);
            u_caster->GetAIInterface()->SendMoveToPacket(coord[i][0], coord[i][1], coord[i][2], 0.0f, 0, u_caster->GetAIInterface()->getMoveFlags());
        }
    }
}

void SpellEffectClass::SpellEffectRedirectThreat(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectPlayMusic(uint32 i, WorldObject *target, int32 amount)
{
    if(!unitTarget)
        return;

    unitTarget->PlaySoundToSet(m_spellInfo->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectKillCredit(uint32 i, WorldObject *target, int32 amount)
{
    if(playerTarget == NULL)
        return;

    if(CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(GetSpellProto()->EffectMiscValue[i]))
        sQuestMgr._OnPlayerKill( playerTarget, GetSpellProto()->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectRestoreManaPct(uint32 i, WorldObject *target, int32 amount)
{
    if( u_caster == NULL || unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 maxMana = (uint32)unitTarget->GetMaxPower(POWER_TYPE_MANA);
    uint32 modMana = damage * maxMana / 100;

    u_caster->Energize(unitTarget, m_triggeredSpellId ? m_triggeredSpellId : GetSpellProto()->Id, modMana, POWER_TYPE_MANA);
}

void SpellEffectClass::SpellEffectRestoreHealthPct(uint32 i, WorldObject *target, int32 amount)
{
    if( u_caster == NULL || unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 maxHp = (uint32)unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    uint32 modHp = damage * maxHp / 100;

    u_caster->Heal(unitTarget, GetSpellProto()->Id, modHp);
}

void SpellEffectClass::SpellEffectForceCast(uint32 i, WorldObject *target, int32 amount)
{
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
    if( unitTarget == NULL )
        return;

    SpellEntry* TriggeredSpell = dbcSpell.LookupEntry(GetSpellProto()->EffectTriggerSpell[i]);
    if( TriggeredSpell == NULL )
        return;

    if(GetSpellProto() == TriggeredSpell) // Infinite loop fix.
        return;

    Spell* sp= new Spell(m_caster,dbcSpell.LookupEntry(TriggeredSpell->Id),true,NULL);
    SpellCastTargets tgt(unitTarget->GetGUID());
    sp->prepare(&tgt);
}

void SpellEffectClass::SpellEffectJump(uint32 i, WorldObject *target, int32 amount)
{
    if(unitTarget == NULL)
        unitTarget = u_caster;
    if( unitTarget == NULL)
        return;

    if(unitTarget->IsCreature() && isTargetDummy(unitTarget->GetEntry()))
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
        if( unitTarget == NULL )
            return;

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

    int32 basePoints = 0;
    basePoints += CalculateEffect(i, unitTarget, basePoints);
    uint32 time = uint32((basePoints / arc) * 100);
    u_caster->GetAIInterface()->StopMovement(time);
    u_caster->SetPosition(x, y, z, ang);
    u_caster->GetAIInterface()->SendJumpTo(x, y, z, time, arc);
}

void SpellEffectClass::SpellEffectTeleportToCaster(uint32 i, WorldObject *target, int32 amount)
{
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
    if( !unitTarget || !unitTarget->IsPet() )
        return;

    unitTarget->SetByte( UNIT_FIELD_BYTES_2, 2, 0x03);
}

void SpellEffectClass::SpellEffectStartQuest(uint32 i, WorldObject *target, int32 amount)
{
    if( !playerTarget )
        return;

    playerTarget->StartQuest(GetSpellProto()->EffectMiscValue[i]);
}

// Spells: 46716-46730
void SpellEffectClass::SpellEffectCreatePet(uint32 i, WorldObject *target, int32 amount)
{
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
            {
                pPet->Destruct();
                pPet = NULL;
            }
        }
    }
}

void SpellEffectClass::SpellEffectTitanGrip(uint32 i, WorldObject *target, int32 amount)
{
    if (playerTarget == NULL)
        return;

    playerTarget->titanGrip = true;
}

bool SpellEffectClass::SpellEffectUpdateQuest(uint32 questid)
{
    QuestLogEntry *en = p_caster->GetQuestLogForEntry( questid );
    if( en != NULL && en->GetObjectiveCount( 0 ) < en->GetQuest()->required_mobcount[0])
    {
        en->SetObjectiveCount( 0, en->GetObjectiveCount( 0 ) + 1 );//(collumn,count)
        en->SendUpdateAddKill( 0 );//(collumn)
        en->UpdatePlayerFields();
        return true;
    }
    return false;
}

void SpellEffectClass::SpellEffectCreateRandomItem(uint32 i, WorldObject *target, int32 amount) // Create Random Item
{

}

void SpellEffectClass::SpellEffectSetTalentSpecsCount(uint32 i, WorldObject *target, int32 amount)
{
    if(p_caster == NULL)
        return;

    p_caster->m_talentInterface.UnlockSpec(damage);
}

void SpellEffectClass::SpellEffectActivateTalentSpec(uint32 i, WorldObject *target, int32 amount)
{
    damage -= 1; // Add our negative offset
    if(p_caster == NULL)
        return;

    if(p_caster->m_bg)
    {
        SendCastResult(SPELL_FAILED_NOT_IN_BATTLEGROUND);
        return;
    }

    // 0 = primary, 1 = secondary
    p_caster->m_talentInterface.ApplySpec(damage);
    p_caster->SetPower(p_caster->getPowerType(), 0);
}

void SpellEffectClass::SpellEffectDisengage(uint32 i, WorldObject *target, int32 amount)
{
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    unitTarget->knockback(GetSpellProto()->EffectBasePoints[i]+1, GetSpellProto()->EffectMiscValue[i], true);
}

void SpellEffectClass::SpellEffectClearFinishedQuest(uint32 i, WorldObject *target, int32 amount)
{
    if (playerTarget == NULL)
        return;

    playerTarget->m_completedQuests.erase(GetSpellProto()->EffectMiscValue[i]);
}

void SpellEffectClass::SpellEffectApplyDemonAura(uint32 i, WorldObject *target, int32 amount)
{

}

void SpellEffectClass::SpellEffectRemoveAura(uint32 i, WorldObject *target, int32 amount)
{
    if (!unitTarget)
        return;

    unitTarget->RemoveAura(GetSpellProto()->EffectTriggerSpell[i], unitTarget->GetGUID());
}

void SpellEffectClass::SpellEffectActivateRune(uint32 i, WorldObject *target, int32 amount)
{
    if( !p_caster )
        return;

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
    if( !p_caster )
        return;
    Quest * qst = sQuestMgr.GetQuestPointer(GetSpellProto()->EffectMiscValue[i]);
    if(!qst)
        return;

    sQuestMgr.SendQuestFailed(FAILED_REASON_FAILED, qst, p_caster);
}*/
