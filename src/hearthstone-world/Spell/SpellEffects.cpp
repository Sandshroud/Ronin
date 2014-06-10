/***
 * Demonstrike Core
 */

// thank god for macros

#include "StdAfx.h"

pSpellEffect SpellEffectsHandler[TOTAL_SPELL_EFFECTS] = {
    &Spell::SpellEffectNULL,                        // 0 - SPELL_EFFECT_NULL
    &Spell::SpellEffectInstantKill,                 // 1 - SPELL_EFFECT_INSTAKILL
    &Spell::SpellEffectSchoolDMG,                   // 2 - SPELL_EFFECT_SCHOOL_DAMAGE
    &Spell::SpellEffectDummy,                       // 3 - SPELL_EFFECT_DUMMY
    &Spell::SpellEffectNULL,                        // 4 - SPELL_EFFECT_PORTAL_TELEPORT
    &Spell::SpellEffectTeleportUnits,               // 5 - SPELL_EFFECT_TELEPORT_UNITS
    &Spell::SpellEffectApplyAura,                   // 6 - SPELL_EFFECT_APPLY_AURA
    &Spell::SpellEffectEnvironmentalDamage,         // 7 - SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
    &Spell::SpellEffectPowerDrain,                  // 8 - SPELL_EFFECT_POWER_DRAIN
    &Spell::SpellEffectHealthLeech,                 // 9 - SPELL_EFFECT_HEALTH_LEECH
    &Spell::SpellEffectHeal,                        // 10 - SPELL_EFFECT_HEAL
    &Spell::SpellEffectBind,                        // 11 - SPELL_EFFECT_BIND
    &Spell::SpellEffectTeleportUnits,               // 12 - SPELL_EFFECT_PORTAL
    &Spell::SpellEffectNULL,                        // 13 - Unused
    &Spell::SpellEffectNULL,                        // 14 - Unused
    &Spell::SpellEffectNULL,                        // 15 - Unused
    &Spell::SpellEffectQuestComplete,               // 16 - SPELL_EFFECT_QUEST_COMPLETE
    &Spell::SpellEffectWeapondamageNoschool,        // 17 - SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL
    &Spell::SpellEffectResurrect,                   // 18 - SPELL_EFFECT_RESURRECT
    &Spell::SpellEffectAddExtraAttacks,             // 19 - SPELL_EFFECT_ADD_EXTRA_ATTACKS
    &Spell::SpellEffectDodge,                       // 20 - SPELL_EFFECT_DODGE
    &Spell::SpellEffectNULL,                        // 21 - Unused
    &Spell::SpellEffectParry,                       // 22 - SPELL_EFFECT_PARRY
    &Spell::SpellEffectBlock,                       // 23 - SPELL_EFFECT_BLOCK
    &Spell::SpellEffectCreateItem,                  // 24 - SPELL_EFFECT_CREATE_ITEM
    &Spell::SpellEffectWeapon,                      // 25 - SPELL_EFFECT_WEAPON
    &Spell::SpellEffectDefense,                     // 26 - SPELL_EFFECT_DEFENSE
    &Spell::SpellEffectPersistentAA,                // 27 - SPELL_EFFECT_PERSISTENT_AREA_AURA
    &Spell::SpellEffectSummon,                      // 28 - SPELL_EFFECT_SUMMON
    &Spell::SpellEffectLeap,                        // 29 - SPELL_EFFECT_LEAP
    &Spell::SpellEffectEnergize,                    // 30 - SPELL_EFFECT_ENERGIZE
    &Spell::SpellEffectWeaponDmgPerc,               // 31 - SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
    &Spell::SpellEffectTriggerMissile,              // 32 - SPELL_EFFECT_TRIGGER_MISSILE
    &Spell::SpellEffectOpenLock,                    // 33 - SPELL_EFFECT_OPEN_LOCK
    &Spell::SpellEffectTranformItem,                // 34 - SPELL_EFFECT_TRANSFORM_ITEM
    &Spell::SpellEffectApplyAA,                     // 35 - SPELL_EFFECT_APPLY_AREA_AURA
    &Spell::SpellEffectLearnSpell,                  // 36 - SPELL_EFFECT_LEARN_SPELL
    &Spell::SpellEffectSpellDefense,                // 37 - SPELL_EFFECT_SPELL_DEFENSE
    &Spell::SpellEffectDispel,                      // 38 - SPELL_EFFECT_DISPEL
    &Spell::SpellEffectLanguage,                    // 39 - SPELL_EFFECT_LANGUAGE
    &Spell::SpellEffectDualWield,                   // 40 - SPELL_EFFECT_DUAL_WIELD
    &Spell::SpellEffectJump,                        // 41 - SPELL_EFFECT_JUMP
    &Spell::SpellEffectJump,                        // 42 - SPELL_EFFECT_JUMP_2
    &Spell::SpellEffectTeleportToCaster,            // 43 - SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER
    &Spell::SpellEffectSkillStep,                   // 44 - SPELL_EFFECT_SKILL_STEP
    &Spell::SpellEffectAddHonor,                    // 45 - SPELL_ADD_HONOR
    &Spell::SpellEffectSpawn,                       // 46 - SPELL_EFFECT_SPAWN
    &Spell::SpellEffectNULL,                        // 47 - SPELL_EFFECT_TRADE_SKILL
    &Spell::SpellEffectNULL,                        // 48 - SPELL_EFFECT_STEALTH
    &Spell::SpellEffectDetect,                      // 49 - SPELL_EFFECT_DETECT
    &Spell::SpellEffectSummonObject,                // 50 - SPELL_EFFECT_SUMMON_OBJECT
    &Spell::SpellEffectNULL,                        // 51 - Unused
    &Spell::SpellEffectNULL,                        // 52 - Unused
    &Spell::SpellEffectEnchantItem,                 // 53 - SPELL_EFFECT_ENCHANT_ITEM
    &Spell::SpellEffectEnchantItemTemporary,        // 54 - SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
    &Spell::SpellEffectTameCreature,                // 55 - SPELL_EFFECT_TAMECREATURE
    &Spell::SpellEffectSummonPet,                   // 56 - SPELL_EFFECT_SUMMON_PET
    &Spell::SpellEffectLearnPetSpell,               // 57 - SPELL_EFFECT_LEARN_PET_SPELL
    &Spell::SpellEffectWeapondamage,                // 58 - SPELL_EFFECT_WEAPON_DAMAGE
    &Spell::SpellEffectOpenLockItem,                // 59 - SPELL_EFFECT_OPEN_LOCK_ITEM
    &Spell::SpellEffectProficiency,                 // 60 - SPELL_EFFECT_PROFICIENCY
    &Spell::SpellEffectSendEvent,                   // 61 - SPELL_EFFECT_SEND_EVENT
    &Spell::SpellEffectPowerBurn,                   // 62 - SPELL_EFFECT_POWER_BURN
    &Spell::SpellEffectThreat,                      // 63 - SPELL_EFFECT_THREAT
    &Spell::SpellEffectTriggerSpell,                // 64 - SPELL_EFFECT_TRIGGER_SPELL
    &Spell::SpellEffectHealthFunnel,                // 65 - SPELL_EFFECT_HEALTH_FUNNEL
    &Spell::SpellEffectPowerFunnel,                 // 66 - SPELL_EFFECT_POWER_FUNNEL
    &Spell::SpellEffectHealMaxHealth,               // 67 - SPELL_EFFECT_HEAL_MAX_HEALTH
    &Spell::SpellEffectInterruptCast,               // 68 - SPELL_EFFECT_INTERRUPT_CAST
    &Spell::SpellEffectDistract,                    // 69 - SPELL_EFFECT_DISTRACT
    &Spell::SpellEffectPull,                        // 70 - SPELL_EFFECT_PULL
    &Spell::SpellEffectPickpocket,                  // 71 - SPELL_EFFECT_PICKPOCKET
    &Spell::SpellEffectAddFarsight,                 // 72 - SPELL_EFFECT_ADD_FARSIGHT
    &Spell::SpellEffectResetTalents,                // 73 - SPELL_EFFECT_RESET_TALENTS
    &Spell::SpellEffectUseGlyph,                    // 74 - SPELL_EFFECT_USE_GLYPH
    &Spell::SpellEffectHealMechanical,              // 75 - SPELL_EFFECT_HEAL_MECHANICAL
    &Spell::SpellEffectSummonObject,                // 76 - SPELL_EFFECT_SUMMON_OBJECT_WILD
    &Spell::SpellEffectScriptEffect,                // 77 - SPELL_EFFECT_SCRIPT_EFFECT
    &Spell::SpellEffectNULL,                        // 78 - SPELL_EFFECT_ATTACK
    &Spell::SpellEffectSanctuary,                   // 79 - SPELL_EFFECT_SANCTUARY
    &Spell::SpellEffectAddComboPoints,              // 80 - SPELL_EFFECT_ADD_COMBO_POINTS
    &Spell::SpellEffectCreateHouse,                 // 81 - SPELL_EFFECT_CREATE_HOUSE
    &Spell::SpellEffectNULL,                        // 82 - SPELL_EFFECT_BIND_SIGHT
    &Spell::SpellEffectDuel,                        // 83 - SPELL_EFFECT_DUEL
    &Spell::SpellEffectStuck,                       // 84 - SPELL_EFFECT_STUCK
    &Spell::SpellEffectSummonPlayer,                // 85 - SPELL_EFFECT_SUMMON_PLAYER
    &Spell::SpellEffectActivateObject,              // 86 - SPELL_EFFECT_ACTIVATE_OBJECT
    &Spell::SpellEffectWMODamage,                   // 87 - SPELL_EFFECT_WMO_DAMAGE
    &Spell::SpellEffectWMORepair,                   // 88 - SPELL_EFFECT_WMO_REPAIR
    &Spell::SpellEffectChangeWMOState,              // 89 - SPELL_EFFECT_WMO_CHANGE
    &Spell::SpellEffectKillCredit,                  // 90 - SPELL_EFFECT_KILL_CREDIT
    &Spell::SpellEffectNULL,                        // 91 - SPELL_EFFECT_THREAT_ALL
    &Spell::SpellEffectEnchantHeldItem,             // 92 - SPELL_EFFECT_ENCHANT_HELD_ITEM
    &Spell::SpellEffectNULL,                        // 93 - SPELL_EFFECT_SUMMON_PHANTASM
    &Spell::SpellEffectSelfResurrect,               // 94 - SPELL_EFFECT_SELF_RESURRECT
    &Spell::SpellEffectSkinning,                    // 95 - SPELL_EFFECT_SKINNING
    &Spell::SpellEffectCharge,                      // 96 - SPELL_EFFECT_CHARGE
    &Spell::SpellEffectPlaceTotemsOnBar,            // 97 - SPELL_EFFECT_PLACE_ALL_TOTEMS_ON_BAR
    &Spell::SpellEffectKnockBack,                   // 98 - SPELL_EFFECT_KNOCK_BACK
    &Spell::SpellEffectDisenchant,                  // 99 - SPELL_EFFECT_DISENCHANT
    &Spell::SpellEffectInebriate,                   // 100 - SPELL_EFFECT_INEBRIATE
    &Spell::SpellEffectFeedPet,                     // 101 - SPELL_EFFECT_FEED_PET
    &Spell::SpellEffectDismissPet,                  // 102 - SPELL_EFFECT_DISMISS_PET
    &Spell::SpellEffectReputation,                  // 103 - SPELL_EFFECT_REPUTATION
    &Spell::SpellEffectSummonObjectSlot,            // 104 - SPELL_EFFECT_SUMMON_OBJECT_SLOT1
    &Spell::SpellEffectSummonObjectSlot,            // 105 - SPELL_EFFECT_SUMMON_OBJECT_SLOT2
    &Spell::SpellEffectSummonObjectSlot,            // 106 - SPELL_EFFECT_SUMMON_OBJECT_SLOT3
    &Spell::SpellEffectSummonObjectSlot,            // 107 - SPELL_EFFECT_SUMMON_OBJECT_SLOT4
    &Spell::SpellEffectDispelMechanic,              // 108 - SPELL_EFFECT_DISPEL_MECHANIC
    &Spell::SpellEffectSummonDeadPet,               // 109 - SPELL_EFFECT_SUMMON_DEAD_PET
    &Spell::SpellEffectDestroyAllTotems,            // 110 - SPELL_EFFECT_DESTROY_ALL_TOTEMS
    &Spell::SpellEffectNULL,                        // 111 - SPELL_EFFECT_DURABILITY_DAMAGE
    &Spell::SpellEffectNULL,                        // 112 - Unused
    &Spell::SpellEffectResurrectNew,                // 113 - SPELL_EFFECT_RESURRECT_NEW
    &Spell::SpellEffectAttackMe,                    // 114 - SPELL_EFFECT_ATTACK_ME
    &Spell::SpellEffectNULL,                        // 115 - SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    &Spell::SpellEffectSkinPlayerCorpse,            // 116 - SPELL_EFFECT_SKIN_PLAYER_CORPSE
    &Spell::SpellEffectNULL,                        // 117 - SPELL_EFFECT_SPIRIT_HEAL
    &Spell::SpellEffectSkill,                       // 118 - SPELL_EFFECT_SKILL
    &Spell::SpellEffectApplyPetAura,                // 119 - SPELL_EFFECT_APPLY_PET_AURA
    &Spell::SpellEffectNULL,                        // 120 - SPELL_EFFECT_TELEPORT_GRAVEYARD
    &Spell::SpellEffectDummyMelee,                  // 121 - SPELL_EFFECT_DUMMYMELEE
    &Spell::SpellEffectNULL,                        // 122 - Unused
    &Spell::SpellEffectSendTaxi,                    // 123 - SPELL_EFFECT_SEND_TAXI  taxi/flight related (misc value is taxi path id)
    &Spell::SpellEffectPull,                        // 124 - SPELL_EFFECT_PULL
    &Spell::SpellEffectNULL,                        // 125 - Reduce Threat by % http://www.thottbot.com/?sp=32835
    &Spell::SpellEffectSpellSteal,                  // 126 - SPELL_EFFECT_SPELL_STEAL Steal Beneficial Buff (Magic) //http://www.thottbot.com/?sp=30449
    &Spell::SpellEffectProspecting,                 // 127 Search 5 ore of a base metal for precious gems.  This will destroy the ore in the process.
    &Spell::SpellEffectApplyAA,                     // 128 - Area Aura
    &Spell::SpellEffectApplyAA,                     // 129 - Aura Aura
    &Spell::SpellEffectRedirectThreat,              // 130 - Redirect your threat.
    &Spell::SpellEffectNULL,                        // 131 - send notifaction like "You have entered a no-fly zone"
    &Spell::SpellEffectPlayMusic,                   // 132 - Play music derp
    &Spell::SpellEffectNULL,                        // 133 - SPELL_EFFECT_FORGET_SPECIALIZATION
    &Spell::SpellEffectKillCredit,                  // 134 - SPELL_EFFECT_KILL_CREDIT misc value is creature entry
    &Spell::SpellEffectSummonPet,                   // 135 - I can haz pet?
    &Spell::SpellEffectRestoreHealthPct,            // 136 - Restore Health %
    &Spell::SpellEffectRestoreManaPct,              // 137 - Restore Mana %
    &Spell::SpellEffectDisengage,                   // 138 - knockback backwards :3
    &Spell::SpellEffectClearFinishedQuest,          // 139 - remove a finished quest.
    &Spell::SpellEffectTeleportUnits,               // 140 - SPELL_EFFECT_TELEPORT_UNITS
    &Spell::SpellEffectTriggerSpell,                // 141 - Triggers a spell
    &Spell::SpellEffectTriggerSpellWithValue,       // 142 - triggers some kind of "Put spell on target" thing... (dono for sure) http://www.thottbot.com/s40872 and http://www.thottbot.com/s33076
    &Spell::SpellEffectApplyDemonAura,              // 143 - Apply pet aura for summons :3
    &Spell::SpellEffectKnockBack,                   // 144 - Spectral Blast
    &Spell::SpellEffectPull,                        // SPELL_EFFECT_TRACTOR_BEAM_FROM_DEST - 145
    &Spell::SpellEffectActivateRune,                // 146 - Finish a rune's cooldown
    &Spell::SpellEffectFailQuest,                   // 147 - Fail quest.
    &Spell::SpellEffectTriggerSpell,                // 148 - Appears to trigger a spell?
    &Spell::SpellEffectCharge,                      // SPELL_EFFECT_CHARGE2
    &Spell::SpellEffectStartQuest,                  // SPELL_EFFECT_START_QUEST
    &Spell::SpellEffectTriggerSpell,                // 151 SPELL_EFFECT_TRIGGER_SPELL_2
    &Spell::SpellEffectSummonPlayer,                // 152 - This is RAF summon but we don't support RAF.
    &Spell::SpellEffectCreatePet,                   // 153 SPELL_EFFECT_CREATE_PET
    &Spell::SpellEffectNULL,                        // 154 unused
    &Spell::SpellEffectTitanGrip,                   // 155 - Titan's Grip
    &Spell::SpellEffectAddPrismaticSocket,          // 156 - Add Socket
    &Spell::SpellEffectCreateRandomItem,            // 157 - create/learn random item/spell for profession
    &Spell::SpellEffectMilling,                     // 158 - milling
    &Spell::SpellEffectAllowPetRename,              // 159 - allow rename pet once again
    &Spell::SpellEffectTriggerSpell,                // 160 - Only 1 spell has this and it has a trigger spell so fuck it!
    &Spell::SpellEffectSetTalentSpecsCount,         // 161 - Sets number of talent specs available to the player
    &Spell::SpellEffectActivateTalentSpec,          // 162 - Activates one of talent specs
    &Spell::SpellEffectNULL,                        // 163 Unused
    &Spell::SpellEffectRemoveAura,                  // 164
};

void Spell::SpellEffectNULL(uint32 i)
{
    sLog.Debug("Spell","Unhandled spell effect %u in spell %u.",GetSpellProto()->Effect[i],GetSpellProto()->Id);
}

void Spell::SpellEffectInstantKill(uint32 i)
{
    if( unitTarget == NULL || !unitTarget->isAlive() || u_caster == NULL)
        return;

    uint32 spellId = GetSpellProto()->Id;
    switch(spellId)
    {
    case 7814:
    case 7815:
    case 7816:
    case 7876:
    case 7877:
    case 7878:
    case 11778:
    case 11779:
    case 11780:
    case 15968:
    case 15969:
    case 18128:
    case 18129:
    case 20398:
    case 20399:
    case 20400:
    case 20401:
    case 20402:
        {
        }break;

    case 18788: //Demonic Sacrifice (508745)
        {
            if( p_caster )
            {
                uint32 spellid1 = 0;
                switch(unitTarget->GetEntry())
                {
                case 416: //Imp
                    {
                        spellid1 = 18789;
                    }break;
                case 417: //Felhunter
                    {
                        spellid1 = 18792;
                    }break;
                case 1860: //VoidWalker
                    {
                        spellid1 = 18790;
                    }break;
                case 1863: //Succubus
                    {
                        spellid1 = 18791;
                    }break;
                case 17252: //felguard
                    {
                        spellid1 = 35701;
                    }break;
                }
                //now caster gains this buff
                if (spellid1 && spellid1 != 0)
                {
                    u_caster->CastSpell(u_caster, dbcSpell.LookupEntry(spellid1), true);
                }
            }
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
            TotalDamage += (unitTarget->GetPowerType() == POWER_TYPE_RAGE ? m_caster->DealDamage(unitTarget, GetSpellProto()->EffectBasePoints[0], 0, 0, spellId) : m_caster->DealDamage(unitTarget, GetSpellProto()->EffectBasePoints[1], 0, 0, spellId));
            return;
        }break;
    case 36484: //Mana Burn same like Power Burn
        {
            TotalDamage += m_caster->DealDamage(unitTarget, GetSpellProto()->EffectBasePoints[0], 0, 0, spellId);
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
            if(p_caster == NULL || p_caster->GetSummon() == NULL)
                return;
            TotalDamage += m_caster->DealDamage(p_caster->GetSummon(), p_caster->GetSummon()->GetUInt32Value(UNIT_FIELD_HEALTH), 0, 0, 0);
            WorldPacket data(SMSG_SPELLINSTAKILLLOG, 200);
            data << m_caster->GetGUID() << p_caster->GetSummon()->GetGUID() << spellId;
            m_caster->SendMessageToSet(&data, true);
            return; //We do not want the generated targets!!!!!!!!!!
        }break;

    }

    switch( GetSpellProto()->NameHash )
    {
    case SPELL_HASH_SACRIFICE:
        {
            if( !u_caster->IsPet() )
                return;

            TO_PET(u_caster)->Dismiss( true );
            return;

        }break;
    case SPELL_HASH_DEMONIC_SACRIFICE:
        {
        }break;
    default:
        {
            // moar cheaters
            if( p_caster == NULL || (u_caster != NULL && u_caster->IsPet() ))
                return;

            if( p_caster->GetSession()->GetPermissionCount() == 0 )
                return;
        }break;
    }
    TotalDamage += m_caster->DealDamage(unitTarget, unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH), 0, 0, 0);
    WorldPacket data(SMSG_SPELLINSTAKILLLOG, 200);
    data << m_caster->GetGUID() << unitTarget->GetGUID() << spellId;
    m_caster->SendMessageToSet(&data, true);
}

void Spell::SpellEffectSchoolDMG(uint32 i) // dmg school
{
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    if(unitTarget->SchoolImmunityList[GetSpellProto()->School])
    {
        SendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }

    if(p_caster != NULL && playerTarget != NULL && p_caster != playerTarget)
    {
        if( playerTarget->IsPvPFlagged() )
            u_caster->SetPvPFlag();
        if( playerTarget->IsFFAPvPFlagged() )
            u_caster->SetFFAPvPFlag();
    }

    uint32 dmg = damage;
    uint32 AdditionalCritChance = 0;
    if(GetSpellProto()->EffectChainTarget[i])//chain
    {
        if( GetSpellProto()->Id == 53595 ) // Hammer of the righteous
        {
            if( p_caster == NULL )
                return;

            static_damage = true;
            float WMIN = 1.0f;
            float WMAX = 2.0f;
            float MWS = 2.0f;
            Item* it = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
            if( it && it->GetProto() )
            {
                WMIN = it->GetProto()->minDamage;
                WMAX = it->GetProto()->maxDamage;
                MWS = it->GetProto()->Delay / 1000.0f;
            }   // Crow: Do not randomize 0, it will crash.

            dmg = float2int32(((WMIN + RandomUInt(WMAX-WMIN)) / (2 * MWS)) * 4 + (u_caster->GetStrength() / 5.5f) * 4 + (u_caster->GetAP() / 14) * 4);
        }
        else
        {
            int32 reduce = (int32)(GetSpellProto()->EffectDamageMultiplier[i] * 100.0f);

            if(reduce && chaindamage)
                chaindamage = chaindamage * reduce / 100;
        }
    }

    // check for no more damage left (chains)
    if( dmg < 0 )
        return;

    // stealthed stuff
    if( m_projectileWait && unitTarget->InStealth() )
        return;

    if(GetSpellProto()->speed > 0)
    {
        TotalDamage += m_caster->SpellNonMeleeDamageLog(unitTarget, GetSpellProto()->Id, dmg, pSpellId == 0, false, false, AdditionalCritChance);
    }
    else
    {
        if( GetType() == SPELL_DMG_TYPE_MAGIC )
        {
            TotalDamage += m_caster->SpellNonMeleeDamageLog( unitTarget, GetSpellProto()->Id, dmg, pSpellId == 0, static_damage, false, AdditionalCritChance );
        }
        else
        {
            if( u_caster != NULL )
            {
                uint32 _type;
                if( GetType() == SPELL_DMG_TYPE_RANGED )
                    _type = RANGED;
                else
                {
                    if (GetSpellProto()->Flags4 & FLAGS4_OFFHAND)
                        _type =  OFFHAND;
                    else
                        _type = MELEE;
                }

                TotalDamage += u_caster->Strike( unitTarget, _type, GetSpellProto(), 0, 0, dmg, pSpellId != 0, true );
            }
        }
    }
}

void Spell::SpellEffectDummy(uint32 i) // Dummy(Scripted events)
{
    if(sScriptMgr.CallScriptedDummySpell(m_spellInfo->Id, i, this))
        return;

    sLog.outDebug("Dummy spell not handled: %u%s\n", m_spellInfo->Id, ((ProcedOnSpell != NULL) ? (format(" proc'd on: %u", ProcedOnSpell->Id).c_str()) : ""));
}

void Spell::SpellEffectTeleportUnits( uint32 i )  // Teleport Units
{
    uint32 spellId = GetSpellProto()->Id;

    if( unitTarget == NULL )
        return;

    // Shadowstep
    if( (GetSpellProto()->NameHash == SPELL_HASH_SHADOWSTEP) && p_caster && p_caster->IsInWorld() )
    {
        /* this is rather tricky actually. we have to calculate the orientation of the creature/player, and then calculate a little bit of distance behind that. */
        float ang;
        Unit* pTarget = unitTarget;
        if( pTarget == m_caster )
        {
            /* try to get a selection */
            pTarget = m_caster->GetMapMgr()->GetUnit(p_caster->GetSelection());
            if( (pTarget == NULL ) || !sFactionSystem.isAttackable(p_caster, pTarget, !(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED) ) || (pTarget->CalcDistance(p_caster) > 30.0f))
                return;
        }

        if( pTarget->GetTypeId() == TYPEID_UNIT )
        {
            if( pTarget->GetUInt64Value( UNIT_FIELD_TARGET ) != 0 )
            {
                /* We're chasing a target. We have to calculate the angle to this target, this is our orientation. */
                ang = m_caster->calcAngle(m_caster->GetPositionX(), m_caster->GetPositionY(), pTarget->GetPositionX(), pTarget->GetPositionY());

                /* convert degree angle to radians */
                ang = ang * float(M_PI) / 180.0f;
            }
            else
            {
                /* Our orientation has already been set. */
                ang = pTarget->GetOrientation();
            }
        }
        else
        {
            /* Players orientation is sent in movement packets */
            ang = pTarget->GetOrientation();
        }

        // avoid teleporting into the model on scaled models
        const static float shadowstep_distance = 1.6f * GetDBCScale( dbcCreatureDisplayInfo.LookupEntry( unitTarget->GetUInt32Value(UNIT_FIELD_DISPLAYID)));
        float new_x = pTarget->GetPositionX() - (shadowstep_distance * cosf(ang));
        float new_y = pTarget->GetPositionY() - (shadowstep_distance * sinf(ang));
        float new_z = pTarget->GetCHeightForPosition(true);
        /* Send a movement packet to "charge" at this target. Similar to warrior charge. */
        p_caster->z_axisposition = 0.0f;
        p_caster->SafeTeleport(p_caster->GetMapId(), p_caster->GetInstanceID(), LocationVector(new_x, new_y, new_z, pTarget->GetOrientation()));
        return;
    }

    if(unitTarget->IsPlayer())
        HandleTeleport(spellId, unitTarget);
}

void Spell::SpellEffectApplyAura(uint32 i)  // Apply Aura
{
    if(unitTarget == NULL)
        return;

    //Aura Immune Flag Check
    if ( playerTarget == NULL)
    {
        Creature* c = TO_CREATURE( unitTarget );
        if(c != NULL)
        {
            if(c->LoadedProto != NULL)
            {
                if(c->LoadedProto->auraimmune_flag && (c->LoadedProto->auraimmune_flag & GetSpellProto()->auraimmune_flag))
                    return;
            }
            else if(c->proto != NULL)
                if( c->proto->auraimmune_flag && (c->proto->auraimmune_flag & GetSpellProto()->auraimmune_flag ))
                    return;
        }
    }

    /* Aura Mastery + Aura Of Concentration = No Interrupting effects */
    if(GetSpellProto()->EffectApplyAuraName[i] == SPELL_AURA_MOD_SILENCE && unitTarget->HasAura(31821) && unitTarget->HasAura(19746))
        return;

    if( unitTarget->isDead() && !(GetSpellProto()->Flags4 & FLAGS4_DEATH_PERSISTENT) )
        return;

    // avoid map corruption.
    if(unitTarget->GetInstanceID()!=m_caster->GetInstanceID())
        return;

    //check if we already have stronger aura
    Aura* pAura = NULL;

    std::map<uint32,Aura* >::iterator itr=unitTarget->tmpAura.find(GetSpellProto()->Id);
    //if we do not make a check to see if the aura owner is the same as the caster then we will stack the 2 auras and they will not be visible client sided
    if(itr == unitTarget->tmpAura.end())
    {
        int32 Duration = GetDuration();

        // Handle diminishing returns, if it should be resisted, it'll make duration 0 here.
        if(!(GetSpellProto()->Attributes & ATTRIBUTES_PASSIVE)) // Passive
            ::ApplyDiminishingReturnTimer(&Duration, unitTarget, GetSpellProto());

        if(!Duration) //maybe add some resist messege to client here ?
            return;

        if(g_caster && g_caster->GetUInt32Value(OBJECT_FIELD_CREATED_BY) && g_caster->m_summoner)
            pAura = new Aura(GetSpellProto(), Duration, g_caster->m_summoner, unitTarget);
        else
            pAura = new Aura(GetSpellProto(), Duration, m_caster, unitTarget);

        if(pAura == NULL)
            return;

        pAura->pSpellId = pSpellId; //this is required for triggered spells

        unitTarget->tmpAura[GetSpellProto()->Id] = pAura;
    }
    else
         pAura=itr->second;

    int32 miscValue = GetSpellProto()->EffectMiscValue[i];

    if(i_caster && m_caster->IsPlayer() && GetSpellProto()->EffectApplyAuraName[i]==SPELL_AURA_PROC_TRIGGER_SPELL)
        miscValue = p_caster->GetItemInterface()->GetInventorySlotByGuid( i_caster->GetGUID() ); // Need to know on which hands attacks spell should proc

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

    pAura->AddMod(GetSpellProto()->EffectApplyAuraName[i],damage,miscValue,i);
}

void Spell::SpellEffectPowerDrain(uint32 i)  // Power Drain
{
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 powerField = UNIT_FIELD_POWER1;
    if(GetSpellProto()->EffectMiscValue[i] < 7)
        powerField += GetSpellProto()->EffectMiscValue[i];
    uint32 curPower = unitTarget->GetUInt32Value(powerField);
    uint32 amt = u_caster->GetSpellBonusDamage(unitTarget, GetSpellProto(), damage, false);

    if( GetPlayerTarget() )
        amt *= float2int32( 1 - ( ( TO_PLAYER(GetPlayerTarget())->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE ) * 2 ) / 100.0f ) );

    if(amt > curPower)
        amt = curPower;

    unitTarget->SetUInt32Value(powerField, curPower - amt);
    u_caster->Energize(u_caster, pSpellId ? pSpellId : GetSpellProto()->Id, amt, GetSpellProto()->EffectMiscValue[i]);
    unitTarget->SendPowerUpdate();
}

void Spell::SpellEffectHealthLeech(uint32 i) // Health Leech
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
        SM_FFValue(u_caster->SM[SMT_MULTIPLE_VALUE][0], &coef, GetSpellProto()->SpellGroupType);
        SM_PFValue(u_caster->SM[SMT_MULTIPLE_VALUE][1], &coef, GetSpellProto()->SpellGroupType);
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

void Spell::SpellEffectHeal(uint32 i) // Heal
{
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    if(GetSpellProto()->EffectChainTarget[i])//chain
    {
        if(!chaindamage)
        {
            Aura* riptide = NULLAURA;
            riptide = unitTarget->m_AuraInterface.FindPositiveAuraByNameHash(SPELL_HASH_RIPTIDE);
            if(unitTarget != NULL && riptide != NULL && GetSpellProto()->NameHash == SPELL_HASH_CHAIN_HEAL)
            {
                damage += damage / 4; // +25%
                unitTarget->RemoveAura(riptide);
            }
            chaindamage = damage;
            Heal((int32)chaindamage);
        }
        else
        {
            int32 reduce = (int32)(GetSpellProto()->EffectDamageMultiplier[i] * 100.0f);
            chaindamage -= (reduce * chaindamage) / 100;
            Heal((int32)chaindamage);
        }
    }
    else
    {
        //yep, the usual special case. This one is shaman talent : Nature's guardian
        //health is below 30%, we have a mother spell to get value from
        switch (GetSpellProto()->Id)
        {
        case 34299: //Druid: Improved Leader of the PAck
            {
                if ( !playerTarget )
                    break;

                if( !playerTarget->IsInFeralForm() || (
                    playerTarget->GetShapeShift() != FORM_CAT &&
                    playerTarget->GetShapeShift() != FORM_BEAR &&
                    playerTarget->GetShapeShift() != FORM_DIREBEAR ) )
                    break;

                uint32 max = playerTarget->GetUInt32Value( UNIT_FIELD_MAXHEALTH );
                uint32 val = float2int32( ( ( playerTarget->m_AuraInterface.FindAura( 34300 ) ) ? 0.04f : 0.02f ) * max );
                if (val)
                    playerTarget->Heal(playerTarget,34299,(uint32)(val));
            }break;
        case 22845: // Druid: Frenzied Regeneration
            {
                if ( !playerTarget )
                    break;

                if (!playerTarget->IsInFeralForm() ||
                    (playerTarget->GetShapeShift() != FORM_BEAR &&
                    playerTarget->GetShapeShift() != FORM_DIREBEAR))
                    break;

                uint32 val = playerTarget->GetUInt32Value(UNIT_FIELD_POWER2);
                if (val > 100)
                    val = 100;

                playerTarget->SetUInt32Value(UNIT_FIELD_POWER2, playerTarget->GetUInt32Value(UNIT_FIELD_POWER2) - val);
                if (val)
                    playerTarget->Heal(playerTarget, 22845, ( playerTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH) * 0.003f ) * (val / 10) );

            }break;
        case 18562: //druid - swiftmend
            {
                uint32 new_dmg = 0;
                //consume rejuvenetaion and regrowth
                Aura* taura = unitTarget->m_AuraInterface.FindPositiveAuraByNameHash( SPELL_HASH_REGROWTH ); //Regrowth
                if( taura != NULL && taura->GetSpellProto() != NULL)
                {
                    uint32 amplitude = taura->GetSpellProto()->EffectAmplitude[1] / 1000;
                    if( !amplitude )
                        amplitude = 3;

                    //our hapiness is that we did not store the aura mod amount so we have to recalc it
                    Spell* spell = new Spell( m_caster, taura->GetSpellProto(), false, NULLAURA );
                    uint32 healamount = spell->CalculateEffect( 1, unitTarget );
                    spell->Destruct();
                    spell = NULLSPELL;
                    new_dmg = healamount * 18 / amplitude;

                    unitTarget->RemoveAura( taura );

                    //do not remove flag if we still can cast it again
                    if( !unitTarget->m_AuraInterface.GetAuraSpellIDWithNameHash( SPELL_HASH_REJUVENATION ) )
                    {
                        unitTarget->RemoveFlag( UNIT_FIELD_AURASTATE, AURASTATE_FLAG_REJUVENATE );
                        sEventMgr.RemoveEvents( unitTarget, EVENT_REJUVENATION_FLAG_EXPIRE );
                    }
                }
                else
                {
                    taura = unitTarget->m_AuraInterface.FindPositiveAuraByNameHash( SPELL_HASH_REJUVENATION );//Rejuvenation
                    if( taura != NULL && taura->GetSpellProto() != NULL )
                    {
                        uint32 amplitude = taura->GetSpellProto()->EffectAmplitude[0] / 1000;
                        if( !amplitude )
                            amplitude = 3;

                        //our hapiness is that we did not store the aura mod amount so we have to recalc it
                        Spell* spell = new Spell( m_caster, taura->GetSpellProto(), false, NULLAURA );
                        uint32 healamount = spell->CalculateEffect( 0, unitTarget );
                        spell->Destruct();
                        spell = NULLSPELL;
                        new_dmg = healamount * 12 / amplitude;

                        unitTarget->RemoveAura( taura );
                        unitTarget->RemoveFlag( UNIT_FIELD_AURASTATE,AURASTATE_FLAG_REJUVENATE );
                        sEventMgr.RemoveEvents( unitTarget,EVENT_REJUVENATION_FLAG_EXPIRE );
                    }
                }

                if( new_dmg > 0 )
                    Heal( (int32)new_dmg );
            }break;
        case 48743://death pact
            {
                if( p_caster == NULL || p_caster->GetSummon() == NULL)
                    return;

                Heal( float2int32(float(p_caster->GetMaxHealth()) * 0.3f) );
            }break;
        case 48153: // Guardian spirit
            {
                if( p_caster == NULL )
                    return;

                Heal( float2int32(unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH) * (damage/100.0f) ));
            }break;
        case 20267: // judgement of light heal effect
            {
                if( u_caster != NULL )
                {
                    Aura* aur = u_caster->m_AuraInterface.FindAura(20185);
                    if( aur != NULL)
                    {
                        Unit* orgcstr = u_caster->m_AuraInterface.FindAura(20185)->GetUnitCaster();
                        if( orgcstr )
                            Heal( float2int32(orgcstr->GetAP() * 0.10f + orgcstr->GetDamageDoneMod(SCHOOL_HOLY) * 0.10f) );
                    }
                }
            }break;

        case 379: // Earth Shield
        case 54172: // Divine storm heal
        case 54968: // Glyph of Holy Light
            {
                Heal((int32)forced_basepoints[0]);
            }break;

        case 23880: // Bloodthirst
            {
                Heal( float2int32( unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH) / 100.0f ) );
            }break;
        case 50464: // Druid Nourish
            {
                if( m_caster == NULL )
                    return;

                int32 amounttoheal = damage;
                bool bonus = unitTarget->HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_REJUVENATE);
                if( bonus )
                    amounttoheal += float2int32(damage*0.2f);

                Heal(amounttoheal);
            }break;
        default:
            Heal((int32)damage);
            break;
        }
    }
}

void Spell::SpellEffectBind(uint32 i) // Innkeeper Bind
{
    if(!playerTarget)
        return;
    playerTarget->SetBindPoint(playerTarget->GetPositionX(), playerTarget->GetPositionY(), playerTarget->GetPositionZ(), playerTarget->GetMapId(), playerTarget->GetZoneId());
}

void Spell::SpellEffectQuestComplete(uint32 i) // Quest Complete
{
    if( playerTarget != NULL )
    {
        QuestLogEntry *qle = playerTarget->GetQuestLogForEntry(GetSpellProto()->EffectMiscValue[i]);
        if( qle == NULL )
            return;

        qle->SendQuestComplete();
        qle->Quest_Status = QUEST_STATUS__COMPLETE;
        qle->UpdatePlayerFields();
    }
}

//wand->
void Spell::SpellEffectWeapondamageNoschool(uint32 i) // Weapon damage + (no School)
{
    if( unitTarget == NULL  || u_caster == NULL )
        return;

    if( GetType() == SPELL_DMG_TYPE_RANGED && GetSpellProto()->speed > 0.0f )
        TotalDamage += u_caster->Strike( unitTarget, RANGED, GetSpellProto(), 0, 0, 0, false, true );
    else
        TotalDamage += u_caster->Strike( unitTarget, ( GetType() == SPELL_DMG_TYPE_RANGED ? RANGED : MELEE ), GetSpellProto(), damage, 0, 0, false, true );
}

void Spell::SpellEffectAddExtraAttacks(uint32 i) // Add Extra Attacks
{
    if( u_caster == NULL )
        return;

    if( ProcedOnSpell )
        u_caster->m_extraattacks[0] = ProcedOnSpell->procCharges;

    if( GetSpellProto()->procCharges > 0 )
        u_caster->m_extraattacks[0] = GetSpellProto()->procCharges;

    u_caster->m_extraattacks[1] += damage;
}

void Spell::SpellEffectDodge(uint32 i)
{
    //i think this actually enbles the skill to be able to dodge melee+ranged attacks
    //value is static and sets value directly which will be modified by other factors
    //this is only basic value and will be overwiten elsewhere !!!
//  if(playerTarget)
//      unitTarget->SetFloatValue(PLAYER_DODGE_PERCENTAGE,damage);
}

void Spell::SpellEffectParry(uint32 i)
{
    if(unitTarget != NULL )
        unitTarget->setcanperry(true);
}

void Spell::SpellEffectBlock(uint32 i)
{
    //i think this actually enbles the skill to be able to block melee+ranged attacks
    //value is static and sets value directly which will be modified by other factors
//  if(playerTarget)
//      unitTarget->SetFloatValue(PLAYER_BLOCK_PERCENTAGE,damage);
}

void Spell::SpellEffectCreateItem(uint32 i) // Create item
{
    if(!playerTarget)
        return;

    if(GetSpellProto()->Id == 3286)
    {
        // Add a hearthstone if they don't have one
        if(!playerTarget->GetItemInterface()->GetItemCount(6948, true))
        {
            // We don't have a hearthstone. Add one.
            if(playerTarget->GetItemInterface()->CalculateFreeSlots(NULL) > 0)
            {
                Item* item = objmgr.CreateItem( 6948, playerTarget);
                if( playerTarget->GetItemInterface()->AddItemToFreeSlot(item) )
                {
                    SlotResult * lr = playerTarget->GetItemInterface()->LastSearchResult();
                    playerTarget->GetSession()->SendItemPushResult(item,false,true,false,true,lr->ContainerSlot,lr->Slot,1);
                }
                else
                {
                    item->DeleteMe();
                    item = NULLITEM;
                }
            }
        }return;
    }

    if(GetSpellProto()->EffectItemType[i] == 0)
        return;

    SlotResult slotresult;
    ItemPrototype *m_itemProto = ItemPrototypeStorage.LookupEntry( GetSpellProto()->EffectItemType[i] );
    if (!m_itemProto)
        return;

    uint32 item_count = 0;
    if (m_itemProto->Class != ITEM_CLASS_CONSUMABLE || GetSpellProto()->SpellFamilyName != SPELLFAMILY_MAGE)
        item_count = damage;
    else if(playerTarget->getLevel() >= GetSpellProto()->spellLevel)
    {
        item_count = ((playerTarget->getLevel() - (GetSpellProto()->spellLevel-1))*damage);
        // These spells can only create one stack!
        if((m_itemProto->MaxCount > 0) && (item_count > (uint32)m_itemProto->MaxCount))
            item_count = m_itemProto->MaxCount;
    }

    if(!item_count)
        item_count = damage;

    //conjure water ranks 7,8 & 9 and conjure food ranks 7 & 8 have different starting amounts
    // tailoring specializations get +1 cloth bonus
    switch(GetSpellProto()->Id)
    {
        case 27389: //Conjure Food 7
        case 10140: //Conjure Water 7
        case 37420: //Conjure Water 8
            {
                if(item_count <= 12)
                    item_count += 8;
            }break;
        case 36686: //Shadowcloth
            if(playerTarget->HasSpell(26801))
                item_count += 1;
            break;
        case 26751: // Primal Mooncloth
            if(playerTarget->HasSpell(26798))
                item_count += 1;
            break;
        case 31373: //Spellcloth
            if(playerTarget->HasSpell(26797))
                item_count += 1;
            break;
    }

    SkillLineSpell* skill = objmgr.GetSpellSkill(GetSpellProto()->Id);
    if(skill)
    {
        // Alchemy Specializations
        // http://www.wowwiki.com/Alchemy#Alchemy_Specializations
        if ( skill->skilline == SKILL_ALCHEMY && Rand(15) )
        {
            //Potion Master and Elixer Master (Elixers and Flasks)
            if(( playerTarget->HasSpell(28675) && m_itemProto->SubClass == ITEM_SUBCLASS_CONSUMABLE_POTION ) ||
                ( playerTarget->HasSpell(28677) && ( m_itemProto->SubClass == ITEM_SUBCLASS_CONSUMABLE_ELIXIR || m_itemProto->SubClass == ITEM_SUBCLASS_CONSUMABLE_FLASK ) ))
            {
                for(int x=0; x<5; x++)
                {
                    SpellEntry *spellInfo;
                    uint32 spellid = m_itemProto->Spells[x].Id;
                    if( spellid )
                    {
                        spellInfo = dbcSpell.LookupEntry(spellid);
                        if ( spellInfo )
                        {
                            item_count = item_count + rand() % 4 + 1;
                            break;
                        }
                    }
                }
            }
            //Transmutation Master
            else if( playerTarget->HasSpell(28672) && GetSpellProto()->Category == 310 )
            {
                item_count = item_count + rand() % 4 + 1;
            }
        }

        // Profession Discoveries
        uint32 discovered_recipe = 0;
        std::set<ProfessionDiscovery*>::iterator itr = objmgr.ProfessionDiscoveryTable.begin();
        for ( ; itr != objmgr.ProfessionDiscoveryTable.end(); itr++ )
        {
            ProfessionDiscovery * pf = ( *itr );
            if ( pf != NULL && GetSpellProto()->Id == pf->SpellId && playerTarget->_GetSkillLineCurrent( skill->skilline ) >= pf->SkillValue && !playerTarget->HasSpell( pf->SpellToDiscover ) && Rand( pf->Chance ) )
            {
                discovered_recipe = pf->SpellToDiscover;
                break;
            }
        }
        // if something discovered learn playerTarget that recipe and broadcast message
        if ( discovered_recipe != 0 )
        {
            SpellEntry * se = dbcSpell.LookupEntry( discovered_recipe );
            if ( se != NULL )
            {
                playerTarget->addSpell( discovered_recipe );
                WorldPacket * data;
                char msg[256];
                sprintf( msg, "%sDISCOVERY! %s has discovered how to create %s.|r", MSG_COLOR_GOLD, playerTarget->GetName(), se->Name );
                data = sChatHandler.FillMessageData( CHAT_MSG_SYSTEM, LANG_UNIVERSAL,  msg, playerTarget->GetGUID(), 0 );
                playerTarget->GetMapMgr()->SendChatMessageToCellPlayers( playerTarget, data, 2, 1, LANG_UNIVERSAL, playerTarget->GetSession() );
                delete data;
            }
        }
    }

    // item count cannot be more than item unique value
    if(m_itemProto->Unique > 0 && item_count > (uint32)m_itemProto->Unique)
        item_count = m_itemProto->Unique;

    if(playerTarget->GetItemInterface()->CanReceiveItem(m_itemProto, item_count, NULL)) //reversed since it sends >1 as invalid and 0 as valid
    {
        SendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
        return;
    }

    if(!playerTarget->GetItemInterface()->AddItemById(GetSpellProto()->EffectItemType[i], (item_count > 1 ? item_count : 1 ), m_itemProto->RandomPropId ? m_itemProto->RandomPropId : 0, true, playerTarget))
        return;

    if(skill)
        DetermineSkillUp(skill->skilline);

    playerTarget->Cooldown_Add(GetSpellProto(), NULLITEM);
}

void Spell::SpellEffectWeapon(uint32 i)
{
    if( playerTarget == NULL )
        return;

    uint32 skill = 0;
    uint32 spell = 0;

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

void Spell::SpellEffectDefense(uint32 i)
{
    //i think this actually enbles the skill to be able to use defense
    //value is static and sets value directly which will be modified by other factors
    //this is only basic value and will be overwiten elsewhere !!!
//  if(unitTarget != NULL && playerTarget)
//      unitTarget->SetFloatValue(UNIT_FIELD_RESISTANCES,damage);
}

void Spell::SpellEffectPersistentAA(uint32 i) // Persistent Area Aura
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
            dynObj->Create(u_caster, this, m_targets.m_srcX, m_targets.m_srcY, m_targets.m_srcZ, dur, r);
        }break;
    case TARGET_FLAG_DEST_LOCATION:
        {
            dynObj->Create(u_caster ? u_caster : g_caster->m_summoner, this, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, dur, r);
        }break;
    default:
        dynObj->Remove();
        return;
    }

    m_AreaAura = true;
}

void Spell::SpellEffectLeap(uint32 i) // Leap
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

                // reset heartbeat for a little while, 5 seconds maybe?
                p_caster->DelaySpeedHack( 5000 );
                ++p_caster->m_heartbeatDisable;
                p_caster->z_axisposition = 0.0f;
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

                // reset heartbeat for a little while, 5 seconds maybe?
                p_caster->DelaySpeedHack( 5000 );
                ++p_caster->m_heartbeatDisable;
                p_caster->z_axisposition = 0.0f;
            }
        }

        if(p_caster->blinked)
            return;
    }

    p_caster->blinked = true;

    WorldPacket data(SMSG_MOVE_KNOCK_BACK, 50);
    data << u_caster->GetNewGUID();
    data << getMSTime();
    data << cosf(u_caster->GetOrientation()) << sinf(u_caster->GetOrientation());
    data << radius;
    data << float(-10.0f);
    p_caster->GetSession()->SendPacket(&data);

    // reset heartbeat for a little while, 2 seconds maybe?
    p_caster->DelaySpeedHack( 10000 );
    ++p_caster->m_heartbeatDisable;
    p_caster->z_axisposition = 0.0f;
}

void Spell::SpellEffectEnergize(uint32 i) // Energize
{
    if( unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 modEnergy = 0;
    switch( GetSpellProto()->Id )
    {
    case 58883: //Rapid Recuperation
    case 57669: // replenishment
        {
            modEnergy = float2int32(0.01f * unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER1));
        }break;
    case 31930:
        {
            modEnergy = float2int32(0.25f * u_caster->GetUInt32Value(UNIT_FIELD_BASE_MANA));
        }break;
    case 31786:
        {
            if( ProcedOnSpell )
            {
                SpellEntry *motherspell=dbcSpell.LookupEntry(pSpellId);
                if(motherspell)
                {
                    //heal amount from procspell (we only proced on a heal spell)
                    uint32 healamt=0;
                    if(ProcedOnSpell->Effect[0]==SPELL_EFFECT_HEAL || ProcedOnSpell->Effect[0]==SPELL_EFFECT_SCRIPT_EFFECT)
                        healamt=ProcedOnSpell->EffectBasePoints[0]+1;
                    else if(ProcedOnSpell->Effect[1]==SPELL_EFFECT_HEAL || ProcedOnSpell->Effect[1]==SPELL_EFFECT_SCRIPT_EFFECT)
                        healamt=ProcedOnSpell->EffectBasePoints[1]+1;
                    else if(ProcedOnSpell->Effect[2]==SPELL_EFFECT_HEAL || ProcedOnSpell->Effect[2]==SPELL_EFFECT_SCRIPT_EFFECT)
                        healamt=ProcedOnSpell->EffectBasePoints[2]+1;
                    modEnergy = (motherspell->EffectBasePoints[0]+1)*(healamt)/100;
                }
            }
        }break;
    case 2687:
        {
            modEnergy = damage;
            if( p_caster != NULL )
            {
                if(p_caster->HasSpell(12818))
                    modEnergy += 60;
                else if(p_caster->HasSpell(12301))
                    modEnergy += 30;
            }
        }break;
    case 20268:
    case 29442:
        {
            if( unitTarget != NULL )
            {
                modEnergy = float2int32(unitTarget->GetUInt32Value( UNIT_FIELD_MAXPOWER1 ) * 0.02f);
            }
        }break;
    case 20272:
    case 47755:
        {
            modEnergy = forced_basepoints[0];
        }break;
    default:
        {
            modEnergy = damage;
        }break;
    }

    u_caster->Energize(unitTarget, GetSpellProto()->logsId ? GetSpellProto()->logsId : (pSpellId ? pSpellId : GetSpellProto()->Id), modEnergy, GetSpellProto()->EffectMiscValue[i]);
}

void Spell::SpellEffectWeaponDmgPerc(uint32 i) // Weapon Percent damage
{
    if( unitTarget == NULL || u_caster == NULL )
        return;

    uint32 _type;
    if( GetType() == SPELL_DMG_TYPE_RANGED )
        _type = RANGED;
    else
    {
        if (GetSpellProto()->Flags4 & FLAGS4_OFFHAND)
            _type = OFFHAND;
        else
            _type = MELEE;
    }

    if( GetType() == SPELL_DMG_TYPE_MAGIC )
    {
        float fdmg = (float)CalculateDamage( u_caster, unitTarget, _type, GetSpellProto() );
        uint32 dmg = float2int32(fdmg*(float(damage/100.0f)));
        TotalDamage += u_caster->SpellNonMeleeDamageLog(unitTarget, GetSpellProto()->Id, dmg, false, false, false);
    }
    else
    {
        TotalDamage += u_caster->Strike( unitTarget, _type, GetSpellProto(), add_damage, damage, 0, false, false );
    }
}

void Spell::SpellEffectTriggerMissile(uint32 i) // Trigger Missile
{
    //Used by mortar team
    //Triggers area affect spell at destinatiom
    if(u_caster == NULL )
        return;

    uint32 spellid = GetSpellProto()->EffectTriggerSpell[i];
    if(spellid == 0)
        return;

    SpellEntry *spInfo = NULL;
    spInfo = dbcSpell.LookupEntry(spellid);
    if(spInfo == NULL )
        return;

    // Just send this spell where he wants :S
    u_caster->CastSpellAoF(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, spInfo, true);
}

void Spell::SpellEffectOpenLock(uint32 i) // Open Lock
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
                if(!itemTarget->locked)
                return;

                Lock *lock = dbcLock.LookupEntry( itemTarget->GetProto()->LockId );
                if(!lock) return;
                for(int i=0; i < 8; ++i)
                {
                    if(lock->locktype[i] == 2 && lock->minlockskill[i] && lockskill >= lock->minlockskill[i])
                    {
                        v = lock->minlockskill[i];
                        itemTarget->locked = false;
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

                Lock *lock = dbcLock.LookupEntry(gameObjTarget->GetInfo()->GetLockID());
                if( lock == NULL )
                    return;

                for(int i=0; i < 8;++i)
                {
                    if(lock->locktype[i] == 2 && lock->minlockskill[i] && lockskill >= lock->minlockskill[i])
                    {
                        v = lock->minlockskill[i];
                        gameObjTarget->SetFlags(0);
                        gameObjTarget->SetState(1);
                        lootmgr.FillGOLoot(&gameObjTarget->m_loot,gameObjTarget->GetEntry(), (gameObjTarget->GetMapMgr() ? gameObjTarget->GetMapMgr()->iInstanceMode : 0), p_caster->GetTeam());
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
                if( TO_PLAYER( m_caster )->_GetSkillLineCurrent( SKILL_HERBALISM ) < v )
                {
                    //SendCastResult(SPELL_FAILED_LOW_CASTLEVEL);
                    return;
                }
                else
                {
                    if( gameObjTarget->m_loot.items.size() == 0 )
                    {
                        lootmgr.FillGOLoot(&gameObjTarget->m_loot,gameObjTarget->GetEntry(), (gameObjTarget->GetMapMgr() ? gameObjTarget->GetMapMgr()->iInstanceMode : 0), p_caster->GetTeam());
                    }
                    else
                        bAlreadyUsed = true;
                }
            }
            else
                SendCastResult(SPELL_FAILED_TRY_AGAIN);

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
                if( TO_PLAYER( m_caster )->_GetSkillLineCurrent( SKILL_MINING ) < v )
                {
                    //SendCastResult(SPELL_FAILED_LOW_CASTLEVEL);
                    return;
                }
                else if( gameObjTarget->m_loot.items.size() == 0 )
                {
                    lootmgr.FillGOLoot(&gameObjTarget->m_loot,gameObjTarget->GetEntry(), (gameObjTarget->GetMapMgr() ? gameObjTarget->GetMapMgr()->iInstanceMode : 0), p_caster->GetTeam());
                }
                else
                    bAlreadyUsed = true;
            }
            else
                SendCastResult(SPELL_FAILED_TRY_AGAIN);

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

            sHookInterface.OnSlowLockOpen(gameObjTarget,p_caster);

            uint32 spellid = 23932;
            if(gameObjTarget->GetInfo()->RawData.ListedData[10])
                spellid = gameObjTarget->GetInfo()->RawData.ListedData[10];

            SpellEntry*en = dbcSpell.LookupEntry(spellid);
            if(en == NULL)
                return;

            Spell* sp = new Spell(p_caster,en,true,NULLAURA);
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
                CALL_GO_SCRIPT_EVENT(gameObjTarget, OnActivate)(TO_PLAYER(p_caster));
                CALL_INSTANCE_SCRIPT_EVENT( gameObjTarget->GetMapMgr(), OnGameObjectActivate )( gameObjTarget, p_caster );
            }

            if(sQuestMgr.OnGameObjectActivate(p_caster, gameObjTarget))
            {
                p_caster->UpdateNearbyGameObjects();
                return;
            }

            if(gameObjTarget->m_loot.items.size() == 0)
            {
                lootmgr.FillGOLoot(&gameObjTarget->m_loot,gameObjTarget->GetEntry(), (gameObjTarget->GetMapMgr() ? gameObjTarget->GetMapMgr()->iInstanceMode : 0), p_caster->GetTeam());
            }
        }break;
    }

    if( gameObjTarget != NULL && gameObjTarget->GetType() == GAMEOBJECT_TYPE_CHEST)
        TO_PLAYER( m_caster )->SendLoot( gameObjTarget->GetGUID(), gameObjTarget->GetMapId(), LOOT_SKINNING );
}

void Spell::SpellEffectOpenLockItem(uint32 i)
{
    Unit* caster = u_caster;
    if(caster == NULL && i_caster != NULL )
        caster = i_caster->GetOwner();

    if( caster == NULL || gameObjTarget  == NULL || !gameObjTarget->IsInWorld())
        return;

    if( caster->IsPlayer() && sQuestMgr.OnGameObjectActivate( (TO_PLAYER(caster)), gameObjTarget ) )
        TO_PLAYER(caster)->UpdateNearbyGameObjects();

    CALL_GO_SCRIPT_EVENT(gameObjTarget, OnActivate)(TO_PLAYER(caster));
    CALL_INSTANCE_SCRIPT_EVENT( gameObjTarget->GetMapMgr(), OnGameObjectActivate )( gameObjTarget, TO_PLAYER( caster ) );
    gameObjTarget->SetState(0);

    if( gameObjTarget->GetType() == GAMEOBJECT_TYPE_CHEST)
    {
        lootmgr.FillGOLoot(&gameObjTarget->m_loot,gameObjTarget->GetEntry(), (gameObjTarget->GetMapMgr() ? gameObjTarget->GetMapMgr()->iInstanceMode : 0), p_caster->GetTeam());
        if(gameObjTarget->m_loot.items.size() > 0)
        {
            TO_PLAYER(caster)->SendLoot(gameObjTarget->GetGUID(), gameObjTarget->GetMapId(), LOOT_CORPSE);
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

void Spell::SpellEffectProficiency(uint32 i)
{
    uint32 skill = 0;
    SkillLineSpell* skillability = objmgr.GetSpellSkill(GetSpellProto()->Id);
    if (skillability)
        skill = skillability->skilline;
    SkillLineEntry* sk = dbcSkillLine.LookupEntry(skill);
    if(skill)
    {
        if(playerTarget != NULL)
        {
            if(!playerTarget->_HasSkillLine(skill))
            {
                // Don't add skills to players logging in.
                /*if((GetSpellProto()->Attributes & 64) && playerTarget->m_TeleportState == 1)
                    return;*/

                if(sk && sk->type == SKILL_TYPE_WEAPON)
                {
                    if(sWorld.StartLevel > 1)
                        playerTarget->_AddSkillLine(skill, 5*sWorld.StartLevel, 5*playerTarget->getLevel());
                    else
                        playerTarget->_AddSkillLine(skill, 1, 5*playerTarget->getLevel());
                }
                else
                    playerTarget->_AddSkillLine(skill, 1, 1);
            }
        }
    }
}

void Spell::SpellEffectSendEvent(uint32 i) //Send Event
{
    //This is mostly used to trigger events on quests or some places
    if(sScriptMgr.CallScriptedDummySpell(m_spellInfo->Id, i, this))
        return;

    if(sScriptMgr.HandleScriptedSpellEffect(m_spellInfo->Id, i, this))
        return;

    sLog.outDebug("Event spell not handled: %u%s\n", m_spellInfo->Id, ((ProcedOnSpell != NULL) ? (format(" proc'd on: %u", ProcedOnSpell->Id).c_str()) : ""));
}

void Spell::SpellEffectApplyAA(uint32 i) // Apply Area Aura
{
    if(!unitTarget || !unitTarget->isAlive() || u_caster != unitTarget)
        return;

    Aura* pAura;
    std::map<uint32,Aura* >::iterator itr = unitTarget->tmpAura.find(GetSpellProto()->Id);
    if(itr == unitTarget->tmpAura.end())
    {
        pAura = (new Aura(GetSpellProto(),GetDuration(),m_caster,unitTarget));

        unitTarget->tmpAura [GetSpellProto()->Id]= pAura;

        float r=GetRadius(i);
        r *= r;

        if( u_caster->IsPlayer() || ( u_caster->GetTypeId() == TYPEID_UNIT && (TO_CREATURE(u_caster)->IsTotem() || TO_CREATURE(u_caster)->IsPet()) ) )
        {
            sEventMgr.AddEvent(pAura, &Aura::EventUpdatePlayerAA, r, EVENT_AREAAURA_UPDATE, GetSpellProto()->area_aura_update_interval, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
        else if( u_caster->GetTypeId() == TYPEID_UNIT )
        {
            sEventMgr.AddEvent(pAura, &Aura::EventUpdateCreatureAA, r, EVENT_AREAAURA_UPDATE, GetSpellProto()->area_aura_update_interval, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            pAura->m_creatureAA = true;
        }
    }
    else
        pAura = itr->second;

    pAura->AddMod(GetSpellProto()->EffectApplyAuraName[i],damage,GetSpellProto()->EffectMiscValue[i],i);
}

void Spell::SpellEffectLearnSpell(uint32 i) // Learn Spell
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
        for(uint32 i=0;i<3;++i)
            if(spellinfo->Effect[i] == SPELL_EFFECT_WEAPON ||
               spellinfo->Effect[i] == SPELL_EFFECT_PROFICIENCY ||
               spellinfo->Effect[i] == SPELL_EFFECT_DUAL_WIELD )
            {
                Spell* sp = new Spell(unitTarget, spellinfo, true, NULLAURA);
                SpellCastTargets targets;
                targets.m_unitTarget = unitTarget->GetGUID();
                targets.m_targetMask = TARGET_FLAG_UNIT;
                sp->prepare(&targets);
                break;
            }
        return;
    }

    // if we got here... try via pet spells..
    SpellEffectLearnPetSpell(i);
}

void Spell::SpellEffectSpellDefense(uint32 i)
{
    //used to enable this ability. We use it all the time ...
}

void Spell::SpellEffectLearnPetSpell(uint32 i)
{
    if(unitTarget && unitTarget->IsPet() && p_caster)
    {
        if (GetSpellProto()->EffectTriggerSpell[i])
        {
            Pet* pPet = TO_PET( unitTarget );
            if(pPet->IsSummonedPet())
                p_caster->AddSummonSpell(unitTarget->GetEntry(), GetSpellProto()->EffectTriggerSpell[i]);

            pPet->AddSpell( dbcSpell.LookupEntry( GetSpellProto()->EffectTriggerSpell[i] ), true );
        }
    }
}

void Spell::SpellEffectDispel(uint32 i) // Dispel
{
    if(u_caster == NULL || unitTarget == NULL )
        return;

    uint32 start,end;
    if(sFactionSystem.isAttackable(u_caster,unitTarget))
    {
        start=0;
        end=MAX_POSITIVE_AURAS;
    }
    else
    {
        start=MAX_POSITIVE_AURAS;
        end=MAX_AURAS;
    }


    unitTarget->m_AuraInterface.MassDispel(u_caster, i, GetSpellProto(), damage, start, end);
}

void Spell::SpellEffectLanguage(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* pPlayer = TO_PLAYER( m_caster );

    if(!pPlayer->GetSession()->HasGMPermissions())
    {
        if(pPlayer->GetTeam() == ALLIANCE)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_COMMON ) )
                pPlayer->_AddSkillLine( SKILL_LANG_COMMON, 300, 300 );
        }
        else
            pPlayer->_RemoveSkillLine( SKILL_LANG_COMMON );

        if(pPlayer->GetTeam() == HORDE)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_ORCISH ) )
                pPlayer->_AddSkillLine( SKILL_LANG_ORCISH, 300, 300 );
        }
        else
            pPlayer->_RemoveSkillLine( SKILL_LANG_ORCISH );

        if(pPlayer->getRace() == RACE_DWARF)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_DWARVEN ) )
                pPlayer->_AddSkillLine( SKILL_LANG_DWARVEN, 300, 300 );
        }
        else
            pPlayer->_RemoveSkillLine( SKILL_LANG_DWARVEN );

        if(pPlayer->getRace() == RACE_NIGHTELF)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_DARNASSIAN ) )
                pPlayer->_AddSkillLine( SKILL_LANG_DARNASSIAN, 300, 300 );
        }
        else
            pPlayer->_RemoveSkillLine( SKILL_LANG_DARNASSIAN );

        if(pPlayer->getRace() == RACE_UNDEAD)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_GUTTERSPEAK ) )
                pPlayer->_AddSkillLine( SKILL_LANG_GUTTERSPEAK, 300, 300 );
        }
        else
            pPlayer->_RemoveSkillLine( SKILL_LANG_GUTTERSPEAK );

        if(pPlayer->getRace() == RACE_TAUREN)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_TAURAHE ) )
                pPlayer->_AddSkillLine( SKILL_LANG_TAURAHE, 300, 300 );
        }
        else
            pPlayer->_RemoveSkillLine( SKILL_LANG_TAURAHE );

        if(pPlayer->getRace() == RACE_GNOME)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_GNOMISH ) )
                pPlayer->_AddSkillLine( SKILL_LANG_GNOMISH, 300, 300 );
        }
        else
            pPlayer->_RemoveSkillLine( SKILL_LANG_GNOMISH );

        if(pPlayer->getRace() == RACE_TROLL)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_TROLL ) )
                pPlayer->_AddSkillLine( SKILL_LANG_TROLL, 300, 300 );
        }
        else
            pPlayer->_RemoveSkillLine( SKILL_LANG_TROLL );

        if(pPlayer->getRace() == RACE_BLOODELF)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_THALASSIAN ) )
                pPlayer->_AddSkillLine( SKILL_LANG_THALASSIAN, 300, 300 );
        }
        else
            pPlayer->_RemoveSkillLine( SKILL_LANG_THALASSIAN );

        if(pPlayer->getRace() == RACE_DRAENEI)
        {
            if( !pPlayer->_HasSkillLine( SKILL_LANG_DRAENEI ) )
                pPlayer->_AddSkillLine( SKILL_LANG_DRAENEI, 300, 300 );
        }
        else
            pPlayer->_RemoveSkillLine( SKILL_LANG_DRAENEI );
    }
}

void Spell::SpellEffectDualWield(uint32 i)
{
    if(p_caster == NULL)
        return;

    if( !p_caster->_HasSkillLine( SKILL_DUAL_WIELD ) )
         p_caster->_AddSkillLine( SKILL_DUAL_WIELD, 1, 1 );

        // Increase it by one
        //dual wield is 1/1 , it never increases it's not even displayed in skills tab

    //note: probably here must be not caster but unitVictim
}

void Spell::SpellEffectSkillStep(uint32 i) // Skill Step
{
    Player* target = NULL;
    if(u_caster == NULL)
        return;

    // Check targets
    if( m_targets.m_unitTarget )
        target = u_caster->GetMapMgr()->GetPlayer(uint32(m_targets.m_unitTarget));
    else
        target = TO_PLAYER( m_caster );

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
    switch( sk->type )
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
        else
            target->_AddSkillLine( skill, 1, max );
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

void Spell::SpellEffectDetect(uint32 i)
{
    if( u_caster == NULL )
        return;
    /* Crow:
    Makes me afraid to see what this us used for.
    Notes below...
    */

    // Crow: We'll just do a visibility update....
    u_caster->UpdateVisibility();
}

void Spell::SpellEffectSummonObject(uint32 i)
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
        go->SetUInt64Value( OBJECT_FIELD_CREATED_BY, m_caster->GetGUID() );
        u_caster->SetUInt64Value( UNIT_FIELD_CHANNEL_OBJECT, go->GetGUID() );

        go->SetInstanceID( m_caster->GetInstanceID() );
        go->PushToWorld( m_caster->GetMapMgr() );

        if( lootmgr.IsFishable( zone ) ) // Only set a 'splash' if there is any loot in this area / zone
        {
            uint32 seconds = RandomUInt(17) + 2;
            sEventMgr.AddEvent( go, &GameObject::FishHooked, TO_PLAYER( m_caster ), EVENT_GAMEOBJECT_FISH_HOOKED, seconds * 1000, 1, 0 );
        }
        sEventMgr.AddEvent( go, &GameObject::EndFishing, TO_PLAYER( m_caster ),true, EVENT_GAMEOBJECT_END_FISHING, uint32(GetDuration() + 1000), 1, 0 );
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

        GameObject* go= NULL;
        go = u_caster->GetMapMgr()->CreateGameObject(entry);
        if( go == NULL )
            return;

        if( m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION )
        {
            px = m_targets.m_destX;
            py = m_targets.m_destY;
            pz = m_targets.m_destZ;
        }
        if( !go->CreateFromProto(entry, mapid, posx, posy, pz, orient) )
            return;

        go->SetInstanceID(m_caster->GetInstanceID());
        go->SetState(1);
        go->SetUInt64Value(OBJECT_FIELD_CREATED_BY, m_caster->GetGUID());
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
                SubGroup * pGroup = p_caster->GetGroup() ? p_caster->GetGroup()->GetSubGroup(p_caster->GetSubGroup()) : NULL;

                if(pGroup)
                {
                    p_caster->GetGroup()->Lock();
                    for(GroupMembersSet::iterator itr = pGroup->GetGroupMembersBegin();
                        itr != pGroup->GetGroupMembersEnd(); itr++)
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
                pTarget = p_caster->GetMapMgr()->GetPlayer((uint32)p_caster->GetSelection());
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
            }
            else
                go->charges = 10;
            p_caster->SetSummonedObject(go);
        }
    }
}

void Spell::SpellEffectEnchantItem(uint32 i) // Enchant Item Permanent
{
    if( itemTarget == NULL || p_caster == NULL )
        return;

    //Start Scroll Creation
    if( itemTarget->GetEntry() == 38682 || itemTarget->GetEntry() == 39349 || itemTarget->GetEntry() == 37602 || itemTarget->GetEntry() == 39350 || itemTarget->GetEntry() == 43145 || itemTarget->GetEntry() == 43146 )
    {
        Item* newItem = NULL;
        Item* add = NULL;
        uint8 slot;
        uint32 itemid;
        SlotResult slotresult;

        ItemPrototype *m_itemProto = NULL;
        // Scroll Creation
        ScrollCreation * sc = ScrollCreationStorage.LookupEntry( GetSpellProto()->Id );
        if(sc)
        {
            m_itemProto = ItemPrototypeStorage.LookupEntry( sc->ItemId );
            itemid  =   sc->ItemId;

            if (!m_itemProto)
                 return;
            if(itemid == 0)
                return;
            uint32 item_count = 0;
            if(p_caster->GetItemInterface()->CanReceiveItem(m_itemProto, item_count, NULL))
            {
                SendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
                return;
            }

            slot = 0;
            add = p_caster->GetItemInterface()->FindItemLessMax(itemid,1, false);
            if (add == NULL )
            {
                slotresult = p_caster->GetItemInterface()->FindFreeInventorySlot(m_itemProto);
                if(!slotresult.Result)
                {
                    SendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
                    return;
                }

                newItem =objmgr.CreateItem(itemid,p_caster);
                if(newItem == NULL)
                    return;
                newItem->SetUInt64Value(ITEM_FIELD_CREATOR,m_caster->GetGUID());
                newItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT, item_count);

                if(p_caster->GetItemInterface()->SafeAddItem(newItem,slotresult.ContainerSlot, slotresult.Slot))
                {
                    p_caster->GetSession()->SendItemPushResult(newItem,true,false,true,true,slotresult.ContainerSlot,slotresult.Slot,item_count);
                }
                else
                {
                    newItem->DeleteMe();
                    newItem = NULLITEM;
                }
                DetermineSkillUp(SKILL_ENCHANTING);
            }
            else
            {
                //scale item_count down if total stack will be more than 20
                if(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + item_count > 20)
                {
                    uint32 item_count_filled;
                    item_count_filled = 20 - add->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
                    add->SetCount(20);
                    add->m_isDirty = true;

                    slotresult = p_caster->GetItemInterface()->FindFreeInventorySlot(m_itemProto);
                    if(!slotresult.Result)
                        item_count = item_count_filled;
                    else
                    {
                        newItem =objmgr.CreateItem(itemid,p_caster);
                        if(newItem == NULL)
                            return;
                        newItem->SetUInt64Value(ITEM_FIELD_CREATOR,m_caster->GetGUID());
                        newItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT, item_count - item_count_filled);
                        if(!p_caster->GetItemInterface()->SafeAddItem(newItem,slotresult.ContainerSlot, slotresult.Slot))
                        {
                            newItem->DeleteMe();
                            newItem = NULLITEM;
                            item_count = item_count_filled;
                        }
                        else
                            p_caster->GetSession()->SendItemPushResult(newItem, true, false, true, true, slotresult.ContainerSlot, slotresult.Slot, item_count-item_count_filled);
                    }
                }
                else
                {
                    add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + item_count);
                    add->m_isDirty = true;
                    p_caster->GetSession()->SendItemPushResult(add, true,false,true,false,p_caster->GetItemInterface()->GetBagSlotByGuid(add->GetGUID()),0xFFFFFFFF,item_count);
                }
                DetermineSkillUp(SKILL_ENCHANTING);
            }
        }
    }
    //End of Scroll Creation

    EnchantEntry * Enchantment = dbcEnchant.LookupEntry(GetSpellProto()->EffectMiscValue[i]);
    if(Enchantment == NULL )
    {
        sLog.outError("Invalid enchantment entry %u for Spell %u", GetSpellProto()->EffectMiscValue[i], GetSpellProto()->Id);
        return;
    }

    WorldSession* session = p_caster->GetSession();
    if(session->GetPermissionCount() > 0)
        sWorld.LogGM(session, "enchanted item for %s", itemTarget->GetOwner()->GetName());

    //remove other perm enchantment that was enchanted by profession
    itemTarget->RemoveProfessionEnchant();
    int32 Slot = itemTarget->AddEnchantment(Enchantment, 0, true, true, false, 0);
    if(Slot < 0)
        return; // Apply failed

    if(i_caster == NULL)
        DetermineSkillUp(SKILL_ENCHANTING);
    itemTarget->m_isDirty = true;
}

void Spell::SpellEffectEnchantItemTemporary(uint32 i)  // Enchant Item Temporary
{
    if(itemTarget == NULL || p_caster == NULL )
        return;

    uint32 Duration = m_spellInfo->EffectBasePoints[i];
    uint32 EnchantmentID = m_spellInfo->EffectMiscValue[i];

    // don't allow temporary enchants unless we're the owner of the item
    if(itemTarget->GetOwner() != p_caster)
        return;

    if(Duration == 0)
    {
        sLog.outError("Spell %u ( %s ) has no enchantment duration. Spell needs to be fixed!", m_spellInfo->Id, m_spellInfo->Name);
        return;
    }

    if(EnchantmentID == 0)
    {
        sLog.outError("Spell %u ( %s ) has no enchantment ID. Spell needs to be fixed!", m_spellInfo->Id, m_spellInfo->Name);
        return;
    }

    EnchantEntry * Enchantment = dbcEnchant.LookupEntry(EnchantmentID);
    if(!Enchantment)
    {
        sLog.outError("Invalid enchantment entry %u for Spell %u", EnchantmentID, m_spellInfo->Id);
        return;
    }

    itemTarget->RemoveEnchantment(TEMP_ENCHANTMENT_SLOT);

    int32 Slot = itemTarget->AddEnchantment(Enchantment, Duration, false, true, false, TEMP_ENCHANTMENT_SLOT);
    if(Slot < 0)
        return; // Apply failed

    SkillLineSpell* skill = objmgr.GetSpellSkill(GetSpellProto()->Id);
    if(skill)
        DetermineSkillUp(skill->skilline,itemTarget->GetProto()->ItemLevel);

    itemTarget->m_isDirty = true;
}

void Spell::SpellEffectAddPrismaticSocket(uint32 i)
{
    if( p_caster == NULL)
        return;

    if(!itemTarget)
        return;

    EnchantEntry* pEnchant = dbcEnchant.LookupEntry(GetSpellProto()->EffectMiscValue[i]);
    if(!pEnchant)
        return;

    bool add_socket = false;
    for(uint8 i = 0; i < 3; i++)
    {
        if(pEnchant->type[i] == 8)
        {
            add_socket = true;
            break;
        }
    }

    if(!add_socket) // Wrong spell.
        return;

    // Item can be in trade slot and have owner diff. from caster
    Player* item_owner = itemTarget->GetOwner();
    if(!item_owner)
        return;

    if(itemTarget->GetMaxSocketsCount() >= 3)
    {
        SendCastResult(SPELL_FAILED_MAX_SOCKETS);
        return;
    }

    itemTarget->RemoveProfessionEnchant();
    itemTarget->AddEnchantment(pEnchant, 0, true, true, false, 6); // 6 is profession slot.
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

void Spell::SpellEffectTameCreature(uint32 i)
{
    Creature* tame = NULL;
    tame = (unitTarget->IsCreature() ? TO_CREATURE(unitTarget) : NULLCREATURE);
    if(tame== NULL)
        return;

    CreatureFamilyEntry *cf = dbcCreatureFamily.LookupEntry(tame->GetCreatureInfo()->Family);
    uint8 result = SPELL_CANCAST_OK;

    if(!tame || !p_caster || !p_caster->isAlive() || !tame->isAlive() || p_caster->getClass() != HUNTER )
        result = SPELL_FAILED_BAD_TARGETS;
    else if(!tame->GetCreatureInfo())
        result = SPELL_FAILED_BAD_TARGETS;
    else if(tame->GetCreatureInfo()->Type != BEAST)
        result = SPELL_FAILED_BAD_TARGETS;
    else if(tame->getLevel() > p_caster->getLevel())
        result = SPELL_FAILED_HIGHLEVEL;
    else if(p_caster->GeneratePetNumber() == 0)
        result = SPELL_FAILED_BAD_TARGETS;
    else if(!cf || cf && !cf->tameable)
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

    Pet* pPet = objmgr.CreatePet();
    pPet->SetInstanceID(p_caster->GetInstanceID());
    pPet->SetPosition(p_caster->GetPosition());
    pPet->CreateAsSummon(tame->GetProto(), tame->GetCreatureInfo(), tame, p_caster, NULL, NULL, 2, 0);

    // Add removal event.
    sEventMgr.AddEvent(tame, &Creature::Despawn, uint32(1), tame->GetRespawnTime(), EVENT_CORPSE_DESPAWN, 5, 0, 0);
}

void Spell::SpellEffectSummonPet(uint32 i) //summon - pet
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

        uint8 petno = p_caster->GetUnstabledPetNumber();

        if(petno)
        {
            p_caster->SpawnPet(petno);
        }
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
    Pet* old = NULL;
    old = TO_PLAYER(m_caster)->GetSummon();
    if(old != NULL)
        old->Dismiss(false);

    CreatureInfo *ci = CreatureNameStorage.LookupEntry(GetSpellProto()->EffectMiscValue[i]);
    CreatureProto* proto = CreatureProtoStorage.LookupEntry(GetSpellProto()->EffectMiscValue[i]);
    if(ci && proto)
    {
        //if demonic sacrifice auras are still active, remove them
        //uint32 spids[] = { 18789, 18790, 18791, 18792, 35701, 0 };
        //p_caster->RemoveAuras(spids);
        p_caster->RemoveAura(18789);
        p_caster->RemoveAura(18790);
        p_caster->RemoveAura(18791);
        p_caster->RemoveAura(18792);
        p_caster->RemoveAura(35701);

        Pet* summon = objmgr.CreatePet();
        if(summon == NULL)
            return;
        summon->SetInstanceID(m_caster->GetInstanceID());
        summon->SetPosition(m_caster->GetPosition());
        summon->CreateAsSummon(proto, ci, NULL, p_caster, NULL, GetSpellProto(), 1, 0);
        if( u_caster->IsPvPFlagged() )
            summon->SetPvPFlag();
    }
}

void Spell::SpellEffectWeapondamage( uint32 i ) // Weapon damage +
{
    if( unitTarget == NULL || u_caster == NULL )
        return;

    if( GetSpellProto()->NameHash == SPELL_HASH_MANGLE && u_caster->IsPlayer() && TO_PLAYER(u_caster)->GetShapeShift() == FORM_CAT )
        TO_PLAYER( u_caster )->AddComboPoints( unitTarget->GetGUID(), 1 );

    // Hacky fix for druid spells where it would "double attack".
    if( GetSpellProto()->Effect[2] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE || GetSpellProto()->Effect[1] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE )
    {
        add_damage += damage;
        return;
    }

    switch(GetSpellProto()->Id)
    {
    case 60103: // Lava Lash
        {
            if(p_caster != NULL)
            {
                Item* offhandweapon = NULL;
                ItemInterface * ii = p_caster->GetItemInterface();
                if(ii && (offhandweapon = ii->GetInventoryItem(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND)))
                {
                    bool hasenchantment = false;
                    uint32 flame[10] = {3, 4, 5, 523, 1665, 1666, 2634, 3779, 3780, 3781};
                    for(uint8 i = 0; i < 10; i++)
                    {
                        if(offhandweapon->HasEnchantment(flame[i]))
                        {
                            hasenchantment = true;
                            break;
                        }
                    }

                    if(hasenchantment)
                    {
                        uint32 bonus = 125;
                        if(u_caster && u_caster->HasAura(55444)) // Glyph of Lava Lash
                            bonus += 10;
                        damage = uint32(float(damage * bonus) / 100);
                    }
                }
            }
        }
    }

    uint32 _type;
    if( GetType() == SPELL_DMG_TYPE_RANGED )
        _type = RANGED;
    else
    {
        if (GetSpellProto()->Flags4 & FLAGS4_OFFHAND)
            _type =  OFFHAND;
        else
            _type = MELEE;
    }
    TotalDamage += u_caster->Strike( unitTarget, _type, GetSpellProto(), damage, 0, 0, false, true );
}

void Spell::SpellEffectPowerBurn(uint32 i) // power burn
{
    if( unitTarget == NULL ||!unitTarget->isAlive() || unitTarget->GetPowerType() != POWER_TYPE_MANA)
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
    int32 mana = float2int32( unitTarget->GetUInt32Value( UNIT_FIELD_POWER1 ) * (damage/100.0f));
    if( GetSpellProto()->Id == 8129 )
        if( mana > u_caster->GetUInt32Value( UNIT_FIELD_MAXPOWER1 ) * 0.26 )
            mana = int32(u_caster->GetUInt32Value( UNIT_FIELD_MAXPOWER1 ) * 0.26);

    unitTarget->ModUnsigned32Value(UNIT_FIELD_POWER1,-mana);

    float coef = GetSpellProto()->EffectValueMultiplier[i]; // damage per mana burned
    if(u_caster)
    {
        SM_FFValue(u_caster->SM[SMT_MULTIPLE_VALUE][0], &coef, GetSpellProto()->SpellGroupType);
        SM_PFValue(u_caster->SM[SMT_MULTIPLE_VALUE][1], &coef, GetSpellProto()->SpellGroupType);
    }
    mana = float2int32((float)mana * coef);

    TotalDamage += m_caster->SpellNonMeleeDamageLog(unitTarget,GetSpellProto()->Id, mana, pSpellId==0,true);
}

void Spell::SpellEffectThreat(uint32 i) // Threat
{
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    bool chck = unitTarget->GetAIInterface()->modThreatByPtr(u_caster,damage);
    if(chck == false)
        unitTarget->GetAIInterface()->AttackReaction(u_caster,1,0);
}

void Spell::SpellEffectTriggerSpell(uint32 i) // Trigger Spell
{
    if(unitTarget == NULL || m_caster == NULL )
        return;

    SpellEntry *spe = dbcSpell.LookupEntry(GetSpellProto()->EffectTriggerSpell[i]);
    if(spe == NULL )
        return;

    if(GetSpellProto() == spe) // Infinite loop fix.
        return;

    Spell* sp = new Spell( m_caster,spe,true,NULLAURA);
    memcpy(sp->forced_basepoints, GetSpellProto()->EffectBasePoints, sizeof(uint32)*3);
    SpellCastTargets tgt((spe->procflags2 & PROC_TARGET_SELF) ? m_caster->GetGUID() : unitTarget->GetGUID());
    sp->prepare(&tgt);
}

void Spell::SpellEffectHealthFunnel(uint32 i) // Health Funnel
{
//  if(unitTarget == NULL || !unitTarget->isAlive() || !unitTarget->IsPet())
//      return;

    //does not exist
}

void Spell::SpellEffectPowerFunnel(uint32 i) // Power Funnel
{
//  if(unitTarget == NULL || !unitTarget->isAlive() || !unitTarget->IsPet())
//      return;

    //does not exist
}

void Spell::SpellEffectHealMaxHealth(uint32 i)   // Heal Max Health
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
         SendHealSpellOnPlayer( TO_PLAYER( m_caster ), playerTarget, dif, false, 0, pSpellId ? pSpellId : GetSpellProto()->Id );
    }
    unitTarget->ModUnsigned32Value( UNIT_FIELD_HEALTH, dif );
}

void Spell::SpellEffectInterruptCast(uint32 i) // Interrupt Cast
{
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    // can't apply stuns/fear/polymorph/root etc on boss
    if(unitTarget->GetTypeId() == TYPEID_UNIT)
    {
        Creature* c = TO_CREATURE( unitTarget );
        if (c && c->GetCreatureInfo() && (c->GetCreatureInfo()->Rank == ELITE_WORLDBOSS || c->GetCreatureInfo()->Flags1 & CREATURE_FLAGS1_BOSS))
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

void Spell::SpellEffectDistract(uint32 i) // Distract
{
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    //spellId 1725 Distract:Throws a distraction attracting the all monsters for ten sec's
    if(m_targets.m_destX != 0.0f || m_targets.m_destY != 0.0f || m_targets.m_destZ != 0.0f)
    {
//      unitTarget->GetAIInterface()->MoveTo(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, 0);
        int32 Stare_duration=GetDuration();
        if(Stare_duration>30*60*1000)
            Stare_duration=10000;//if we try to stare for more then a half an hour then better not stare at all :P (bug)
        float newo=unitTarget->calcRadAngle(unitTarget->GetPositionX(),unitTarget->GetPositionY(),m_targets.m_destX,m_targets.m_destY);
        unitTarget->GetAIInterface()->StopMovement(Stare_duration);
        unitTarget->SetFacing(newo);
    }

    //Smoke Emitter 164870
    //Smoke Emitter Big 179066
    //Unit Field Target of
}

void Spell::SpellEffectPickpocket(uint32 i) // pickpocket
{
    if( unitTarget == NULL || p_caster == NULL || unitTarget->GetTypeId() != TYPEID_UNIT)
        return;

    //Show random loot based on roll,
    Creature* target = TO_CREATURE( unitTarget );
    if(target->IsPickPocketed() || (target->GetCreatureInfo() && target->GetCreatureInfo()->Type != HUMANOID))
    {
        SendCastResult(SPELL_FAILED_TARGET_NO_POCKETS);
        return;
    }

    lootmgr.FillPickpocketingLoot(&target->m_loot, target->GetEntry());

    uint32 _rank = target->GetCreatureInfo() ? target->GetCreatureInfo()->Rank : 0;
    unitTarget->m_loot.gold = float2int32((_rank+1) * target->getLevel() * (RandomUInt(5) + 1) * sWorld.getRate(RATE_MONEY));

    p_caster->SendLoot(target->GetGUID(), target->GetMapId(), LOOT_PICKPOCKETING);
    target->SetPickPocketed(true);
}

void Spell::SpellEffectAddFarsight(uint32 i) // Add Farsight
{
    if( p_caster == NULL )
        return;

    DynamicObject* dyn = p_caster->GetMapMgr()->CreateDynamicObject();
    if(dyn == NULL)
        return;

    dyn->Create(p_caster, this, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, GetDuration(), GetRadius(i));
    dyn->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dyn->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x80000002);
    dyn->AddToWorld(p_caster->GetMapMgr());
    p_caster->SetUInt64Value(PLAYER_FARSIGHT, dyn->GetGUID());
    p_caster->SetUInt32Value(PLAYER_FARSIGHT, dyn->GetLowGUID());
    p_caster->GetMapMgr()->ChangeFarsightLocation(p_caster, m_targets.m_destX, m_targets.m_destY, true);
}

void Spell::SpellEffectResetTalents(uint32 i) // Used by Trainers.
{
    if( !playerTarget )
        return;

    playerTarget->Reset_Talents();
}

void Spell::SpellEffectUseGlyph(uint32 i)
{
    if( p_caster == NULL )
        return;
    uint8 result = 0;
    result = p_caster->SetGlyph(m_glyphIndex, GetSpellProto()->EffectMiscValue[i]);
    if(result) // there was an error
        SendCastResult(result);
    else // success, need to update client display
        p_caster->smsg_TalentsInfo(false);
}

void Spell::SpellEffectHealMechanical(uint32 i)
{
    if(unitTarget == NULL || unitTarget->GetCreatureType() != MECHANICAL)
        return;

    Heal((int32)damage);
}

void Spell::SpellEffectSummonObjectWild(uint32 i)
{
    if(u_caster == NULL )
        return;

    // spawn a new one
    GameObject* GoSummon = u_caster->GetMapMgr()->CreateGameObject(GetSpellProto()->EffectMiscValue[i]);
    if( GoSummon == NULL || !GoSummon->CreateFromProto(GetSpellProto()->EffectMiscValue[i], m_caster->GetMapId(), m_caster->GetPosition()))
        return;

    GoSummon->SetUInt32Value(GAMEOBJECT_LEVEL, u_caster->getLevel());
    GoSummon->SetUInt64Value(OBJECT_FIELD_CREATED_BY, m_caster->GetGUID());
    GoSummon->SetState(0);
    GoSummon->PushToWorld(u_caster->GetMapMgr());
    GoSummon->SetSummoned(u_caster);
    GoSummon->ExpireAndDelete(GetDuration());
}

void Spell::SpellEffectScriptEffect(uint32 i) // Script Effect
{
    // Try a dummy SpellHandler
    if(sScriptMgr.CallScriptedDummySpell(m_spellInfo->Id, i, this))
        return;

    if(sScriptMgr.HandleScriptedSpellEffect(m_spellInfo->Id, i, this))
        return;

    sLog.outDebug("Unhandled Scripted Effect In Spell %u", m_spellInfo->Id);
}

void Spell::SpellEffectSanctuary(uint32 i) // Stop all attacks made to you
{
    if( unitTarget == NULL )
        return;

    Object::InRangeUnitSet::iterator itr, it2;
    Unit* pUnit;

    for( itr = unitTarget->GetInRangeOppFactsSetBegin(); itr != unitTarget->GetInRangeOppFactsSetEnd(); )
    {
        it2 = itr++;
        pUnit = TO_UNIT(*it2);
        if( pUnit->GetTypeId() == TYPEID_UNIT )
            pUnit->GetAIInterface()->RemoveThreatByPtr( unitTarget );
    }
    

    // also cancel any spells we are casting
    if( unitTarget->GetCurrentSpell() != NULL && unitTarget->GetCurrentSpell() != this && unitTarget->GetCurrentSpell()->getState() == SPELL_STATE_PREPARING )
        unitTarget->InterruptCurrentSpell();
    unitTarget->smsg_AttackStop( unitTarget );

    if( playerTarget != NULL )
        playerTarget->EventAttackStop();
}

void Spell::SpellEffectAddComboPoints(uint32 i) // Add Combo Points
{
    if(p_caster == NULL)
        return;

    //if this is a procspell Ruthlessness (maybe others later)
    if(pSpellId && GetSpellProto()->Id==14157)
    {
        //it seems this combo adding procspell is going to change combopoint count before they will get reseted. We add it after the reset
        /* burlex - this is wrong, and exploitable.. :/ if someone uses this they will have unlimited combo points */
        //re-enabled this by Zack. Explained why it is used + recheked to make sure initialization is good ...
        // while casting a spell talent will trigger uppon the spell prepare faze
        // the effect of the talent is to add 1 combo point but when triggering spell finishes it will clear the extra combo point
        p_caster->m_spellcomboPoints += damage;
        return;
    }
    p_caster->AddComboPoints(p_caster->GetSelection(), damage);
}

void Spell::SpellEffectCreateHouse(uint32 i) // Create House
{


}

void Spell::SpellEffectDuel(uint32 i) // Duel
{
    if( p_caster == NULL  || !p_caster->isAlive() || playerTarget == p_caster )
        return;

    uint32 areaId = p_caster->GetAreaId();
    AreaTableEntry * at = dbcAreaTable.LookupEntry(areaId);
    if( sWorld.FunServerMall != -1 && areaId == (uint32)sWorld.FunServerMall )
    {
        if(at != NULL)
            p_caster->SendAreaTriggerMessage("Sandshroud System: Dueling is not allowed in %s.", at->name);
        else
            p_caster->SendAreaTriggerMessage("Sandshroud System: Dueling is not allowed in the mall.");
        return;
    }

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

void Spell::SpellEffectStuck(uint32 i)
{
    if( playerTarget == NULL || playerTarget != p_caster)
        return;

    uint32 mapid = playerTarget->GetBindMapId();
    float x = playerTarget->GetBindPositionX();
    float y = playerTarget->GetBindPositionY();
    float z = playerTarget->GetBindPositionZ();
    float orientation = 0;

    sEventMgr.AddEvent(playerTarget,&Player::EventTeleport,mapid,x,y,z,orientation,1,
        EVENT_PLAYER_TELEPORT,50,1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Spell::SpellEffectSummonPlayer(uint32 i)
{
    if( playerTarget == NULL)
        return;

    if(m_caster->GetMapMgr()->GetMapInfo() && m_caster->GetMapMgr()->GetMapInfo()->type != INSTANCE_NULL)
        return;

    playerTarget->SummonRequest(m_caster, m_caster->GetZoneId(), m_caster->GetMapId(),
        m_caster->GetInstanceID(), m_caster->GetPosition());
}

void Spell::SpellEffectActivateObject(uint32 i) // Activate Object
{
/*  if( p_caster == NULL)
        return;

    if( gameObjTarget == NULL)
        return;

    gameObjTarget->SetUInt32Value(GAMEOBJECT_DYNAMIC, 1);

    sEventMgr.AddEvent(gameObjTarget, &GameObject::Deactivate, EVENT_GAMEOBJECT_DEACTIVATE, GetDuration(), 1);*/
}

void Spell::SpellEffectWMODamage(uint32 i)
{
    if(p_caster == NULL && v_caster && v_caster->GetControllingPlayer() )
        p_caster = TO_PLAYER(v_caster->GetControllingPlayer());
    if(gameObjTarget == NULL)
    {
        DamageGosAround(i);
        return;
    }

    gameObjTarget->TakeDamage(damage,m_caster,p_caster, GetSpellProto()->Id);
}

void Spell::SpellEffectWMORepair(uint32 i)
{
    if(gameObjTarget == NULL)
        return;

    gameObjTarget->Rebuild();
}

void Spell::SpellEffectChangeWMOState(uint32 i)
{
    if(gameObjTarget == NULL)
        return;

    switch(GetSpellProto()->EffectMiscValue[i])
    {
    case 1:
        {
            gameObjTarget->Damage();
        }break;
    case 2:
        {
            gameObjTarget->Destroy();
        }break;
    default:
        {
            gameObjTarget->Rebuild();
        }break;
    }
}

void Spell::SpellEffectSelfResurrect(uint32 i)
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
            mana = uint32((unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER1)*damage)/100);
        }
        break;
    default:
        {
            if(damage < 0) return;
            health = uint32(unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*damage/100);
            mana = uint32(unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER1)*damage/100);
        }break;
        }

    if(class_==WARRIOR||class_==ROGUE)
        mana=0;

    playerTarget->m_resurrectHealth = health;
    playerTarget->m_resurrectMana = mana;

    playerTarget->ResurrectPlayer();
    playerTarget->m_resurrectHealth = 0;
    playerTarget->m_resurrectMana = 0;
    playerTarget->SetMovement(MOVE_UNROOT, 1);

    playerTarget->SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);

    if(GetSpellProto()->Id==21169)
        AddCooldown();
}

void Spell::SpellEffectSkinning(uint32 i)
{
    if( unitTarget == NULL || !unitTarget->IsCreature() || p_caster == NULL )
        return;

    Creature* cr = TO_CREATURE( unitTarget );
    uint32 skill = cr->GetRequiredLootSkill();

    uint32 sk = TO_PLAYER( m_caster )->_GetSkillLineCurrent( skill );
    uint32 lvl = cr->getLevel();

    if( ( sk >= lvl * 5 ) || ( ( sk + 100 ) >= lvl * 10 ) )
    {
        //Fill loot for Skinning
        lootmgr.FillGatheringLoot(&cr->m_loot, cr->GetEntry());
        TO_PLAYER( m_caster )->SendLoot( cr->GetGUID(), cr->GetMapId(), LOOT_SKINNING );

        //Not skinable again
        cr->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        cr->Skinned = true;
        //double chance from elite
        if(cr->GetCreatureInfo()->Rank > 0)
            DetermineSkillUp(skill ,sk < lvl * 5 ? sk/5 : lvl, 2);
        else
            DetermineSkillUp(skill ,sk < lvl * 5 ? sk/5 : lvl, 1);
    }
    else
        SendCastResult(SPELL_FAILED_TARGET_UNSKINNABLE);
}

void Spell::SpellEffectCharge(uint32 i)
{
    if( unitTarget == NULL || !u_caster || !unitTarget->isAlive())
        return;

    if (u_caster->IsStunned() || u_caster->m_rooted || u_caster->IsPacified() || u_caster->IsFeared())
        return;

    float dx,dy;

    float x, y, z;
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        if(m_targets.m_destX == 0.0f || m_targets.m_destY == 0.0f)
            return;
        x = m_targets.m_destX;
        y = m_targets.m_destY;
        z = m_targets.m_destZ;
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

    uint32 time = uint32( (m_caster->CalcDistance(x,y,z) / ((u_caster->m_runSpeed * 3.5) * 0.001f)) + 0.5);
    u_caster->GetAIInterface()->SendMoveToPacket(x, y, z, 0.0f, time, MONSTER_MOVE_FLAG_WALK);
    u_caster->SetPosition(x,y,z,0.0f);

    if(unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
        unitTarget->GetAIInterface()->StopMovement(time);

    u_caster->addStateFlag(UF_ATTACKING);
    if(unitTarget)
        u_caster->smsg_AttackStart( unitTarget );
    u_caster->setAttackTimer(time, false);
    u_caster->setAttackTimer(time, true);
    if(p_caster)
    {
        p_caster->EventAttackStart();
        p_caster->ResetHeartbeatCoords();
        p_caster->DelaySpeedHack( time + 1000 );
        p_caster->z_axisposition = 0.0f;
    }
}

void Spell::SpellEffectPlaceTotemsOnBar(uint32 i)
{
    if(!p_caster)
        return;

    uint32 button = 132+m_spellInfo->EffectMiscValue[i];
    uint32 button_count = m_spellInfo->EffectMiscValueB[i];
    for (uint32 slot = button; slot < button+button_count; slot++)
    {
        if(p_caster->GetActionButton(slot).GetType() != ACTION_BUTTON_SPELL)
             continue;
        uint32 spell = p_caster->GetActionButton(slot).GetAction();
        if(spell == 0)
            continue;
        SpellEntry* sp = dbcSpell.LookupEntry(spell);
        if(sp == NULL || !p_caster->HasSpell(spell))
            continue;
        if(/*!IsTotemSpell(sp) || */p_caster->SpellHasCooldown(spell))
            continue;
        Spell* pSpell = new Spell(p_caster, sp, true, NULL);
        if(!pSpell->HasPower())
            continue;

        SpellCastTargets targets;
        pSpell->GenerateTargets(&targets);
        pSpell->prepare(&targets);
    }
}

void Spell::SpellEffectSendTaxi( uint32 i )
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

        CreatureInfo* ci = CreatureNameStorage.LookupEntry( mount_entry );
        if( !ci )
            return;

        modelid = ci->Female_DisplayID;
        if( !modelid )
            return;
    }
    else                                // ALLIANCE
    {
        mount_entry = taxinode->alliance_mount;
        if( !mount_entry )
            mount_entry = taxinode->horde_mount;

        CreatureInfo* ci = CreatureNameStorage.LookupEntry( mount_entry );
        if( !ci )
            return;

        modelid = ci->Male_DisplayID;
        if( !modelid )
            return;
    }

    playerTarget->TaxiStart( taxipath, modelid, 0 );
}

void Spell::SpellEffectPull( uint32 i )
{
    if( unitTarget == NULL && u_caster != NULL)
        unitTarget = u_caster;
    if(unitTarget == NULL)
        return;

    if(unitTarget->IsCreature() && isTargetDummy(unitTarget->GetEntry()))
        return;

    float pullX = 0.0f;
    float pullY = 0.0f;
    float pullZ = 0.0f;
    if(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        pullX = m_targets.m_destX;
        pullY = m_targets.m_destY;
        pullZ = m_targets.m_destZ;
    }
    else
    {
        pullX = m_caster->GetPositionX();
        pullY = m_caster->GetPositionY();
        pullZ = m_caster->GetPositionZ();
    }

    float arc = 0.0f;
    if(GetSpellProto()->EffectMiscValue[i])
        arc = GetSpellProto()->EffectMiscValue[i]/10;
    else if(GetSpellProto()->EffectMiscValueB[i])
        arc = GetSpellProto()->EffectMiscValueB[i]/10;
    else
        arc = 10.0f;

    uint32 time = uint32((CalculateEffect(i, unitTarget) / arc) * 100);
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

void Spell::SpellEffectKnockBack(uint32 i)
{
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    if(unitTarget->IsCreature() && isTargetDummy(unitTarget->GetEntry()))
        return;

    unitTarget->knockback(GetSpellProto()->EffectBasePoints[i]+1, GetSpellProto()->EffectMiscValue[i]);
}

void Spell::SpellEffectDisenchant(uint32 i)
{
    if(p_caster == NULL)
        return;

    Item* it = p_caster->GetItemInterface()->GetItemByGUID(m_targets.m_itemTarget);
    if( it == NULL )
    {
        SendCastResult(SPELL_FAILED_CANT_BE_DISENCHANTED);
        return;
    }

    if( !it->m_looted )
    {
        lootmgr.FillItemLoot(&it->m_loot, it->GetEntry(), p_caster->GetTeam());

        if( it->m_loot.items.size() > 0 )
        {
            //Check for skill, we can increase it upto 75
            uint32 skill=p_caster->_GetSkillLineCurrent( SKILL_ENCHANTING );
            if(skill < 75)//can up skill
            {
                if(Rand(float(100-skill*100.0/75.0)))
                    p_caster->_AdvanceSkillLine(SKILL_ENCHANTING, float2int32( 1.0f * sWorld.getRate(RATE_SKILLRATE)));
            }
            sLog.outDebug("SpellEffect","Succesfully disenchanted item %d", uint32(itemTarget->GetEntry()));
            p_caster->SendLoot( itemTarget->GetGUID(), itemTarget->GetMapId(), LOOT_DISENCHANTING );
        }
        else
        {
            sLog.outDebug("SpellEffect","Disenchanting failed, item %d has no loot", uint32(itemTarget->GetEntry()));
            SendCastResult(SPELL_FAILED_CANT_BE_DISENCHANTED);
            return;
        }
        // delete from db so it won't be re-saved
        it->DeleteFromDB();
        it->m_looted = true;
    }
    if(it == i_caster)
        i_caster = NULLITEM;
}
void Spell::SpellEffectInebriate(uint32 i) // lets get drunk!
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

void Spell::SpellEffectFeedPet(uint32 i)  // Feed Pet
{
    // food flags and food level are checked in Spell::CanCast()
    if( itemTarget == NULL || p_caster == NULL )
        return;

    Pet* pPet = NULL;
    pPet = p_caster->GetSummon();
    if(pPet== NULL)
        return;

    /** Cast feed pet effect
    - effect is item level and pet level dependent, aura ticks are 35, 17, 8 (*1000) happiness
    - http://petopia.brashendeavors.net/html/articles/basics_feeding.shtml */
    int8 deltaLvl = pPet->getLevel() - itemTarget->GetProto()->ItemLevel;
    damage /= 1000; //damage of Feed pet spell is 35000
    if(deltaLvl > 10)
        damage = damage >> 1;//divide by 2
    if(deltaLvl > 20)
        damage = damage >> 1;
    damage *= 1000;

    SpellEntry *spellInfo = dbcSpell.LookupEntry(GetSpellProto()->EffectTriggerSpell[i]);
    Spell* sp = new Spell(p_caster, spellInfo, true, NULLAURA);
    sp->forced_basepoints[0] = damage - 1;
    SpellCastTargets tgt;
    tgt.m_unitTarget = pPet->GetGUID();
    sp->prepare(&tgt);

    if(itemTarget->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > 1)
    {
        itemTarget->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT, -1);
        itemTarget->m_isDirty = true;
    }
    else
    {
        p_caster->GetItemInterface()->SafeFullRemoveItemByGuid(itemTarget->GetGUID());
        itemTarget = NULLITEM;
    }
}

void Spell::SpellEffectReputation(uint32 i)
{
    if( playerTarget == NULL)
        return;

    playerTarget->ModStanding(GetSpellProto()->EffectMiscValue[i], damage);
}

void Spell::SpellEffectSummonObjectSlot(uint32 i)
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

    GameObject* GoSummon = NULLGOB;
    uint32 slot = GetSpellProto()->Effect[i] - SPELL_EFFECT_SUMMON_OBJECT_SLOT1;
    GoSummon = u_caster->m_ObjectSlots[slot] ? u_caster->GetMapMgr()->GetGameObject(u_caster->m_ObjectSlots[slot]) : NULLGOB;
    u_caster->m_ObjectSlots[slot] = 0;

    //slot allready used, empty it now.
    if( GoSummon != NULLGOB)
    {
        if(GoSummon->GetInstanceID() != u_caster->GetInstanceID())
            GoSummon->ExpireAndDelete();
        else
        {
            if( GoSummon->IsInWorld() )
                GoSummon->RemoveFromWorld(true);

            GoSummon->Destruct();
            GoSummon = NULLGOB;
        }
    }

    //create a new GoSummon
    GoSummon = u_caster->GetMapMgr()->CreateGameObject( goi->ID );
    if(GoSummon == NULL)
        return;
    GoSummon->SetInstanceID( u_caster->GetInstanceID() );
    if( GetSpellProto()->EffectImplicitTargetA[i] == EFF_TARGET_SIMPLE_AOE )
        GoSummon->CreateFromProto( goi->ID, m_caster->GetMapId(), m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, m_caster->GetOrientation());
    else
        GoSummon->CreateFromProto( goi->ID, m_caster->GetMapId(), m_caster->GetPosition());

    GoSummon->SetRotation( m_caster->GetOrientation() );
    GoSummon->SetUInt32Value(GAMEOBJECT_LEVEL, u_caster->getLevel());

    if(GoSummon->GetType() == GAMEOBJECT_TYPE_TRAP)
    {
        GoSummon->SetUInt64Value(OBJECT_FIELD_CREATED_BY, u_caster->GetGUID());
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
    GoSummon->SetPhaseMask(u_caster->GetPhaseMask());
    u_caster->m_ObjectSlots[slot] = GoSummon->GetUIdFromGUID();
}

void Spell::SpellEffectDispelMechanic(uint32 i)
{
    if( unitTarget == NULL || !unitTarget->isAlive() )
        return;

    int32 sMisc = (int32)GetSpellProto()->EffectMiscValue[i];
    unitTarget->m_AuraInterface.AttemptDispel(u_caster, sMisc, (unitTarget == u_caster || !sFactionSystem.isAttackable( u_caster, unitTarget )));

    if( playerTarget && GetSpellProto()->NameHash == SPELL_HASH_DAZED && playerTarget->IsMounted() )
        playerTarget->Dismount();
}

void Spell::SpellEffectSummonDeadPet(uint32 i)
{//this is pet resurrect
    if(p_caster == NULL )
        return;
    Pet* pPet = p_caster->GetSummon();
    if(pPet)
    {
        pPet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
        pPet->SetUInt32Value(UNIT_FIELD_HEALTH, (uint32)((pPet->GetUInt32Value(UNIT_FIELD_MAXHEALTH) * damage) / 100));
        pPet->setDeathState(ALIVE);
        pPet->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, pPet, 0);
        sEventMgr.RemoveEvents(pPet, EVENT_PET_DELAYED_REMOVE);
    }
}

/* This effect has 2 functions
 * 1. It delete's all current totems from the player
 * 2. It returns a percentage of the mana back to the player
 *
 * Bur kick my ass if this is not safe:P
*/

uint32 TotemSpells[4] = { 63, 81, 82, 83 };

void Spell::SpellEffectDestroyAllTotems(uint32 i)
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
            for(set<Creature*>::iterator itr = p_caster->m_Summons[spe->slot].begin(); itr != p_caster->m_Summons[spe->slot].end(); itr++)
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

void Spell::SpellEffectSummonDemonOld(uint32 i)
{
    if(p_caster == NULL ) //p_caster->getClass() != WARLOCK ) //summoning a demon shouldn't be warlock only, see spells 25005, 24934, 24810 etc etc
        return;

    Pet* pPet = p_caster->GetSummon();
    if(pPet!= NULL)
    {
        pPet->Dismiss(false);
        pPet = NULL;
    }

    CreatureInfo *ci = CreatureNameStorage.LookupEntry(GetSpellProto()->EffectMiscValue[i]);
    CreatureProto* proto = CreatureProtoStorage.LookupEntry(GetSpellProto()->EffectMiscValue[i]);
    if(ci && proto)
    {
        pPet = objmgr.CreatePet();
        pPet->SetInstanceID(p_caster->GetInstanceID());
        pPet->CreateAsSummon(proto, ci, NULL, p_caster, NULL, GetSpellProto(), 1, 300000);
    }

    //Create Enslave Aura if its inferno spell
    if(GetSpellProto()->Id == 1122)
    {
        SpellEntry *spellInfo = dbcSpell.LookupEntry(11726);

        Spell* sp(new Spell(TO_OBJECT(pPet),spellInfo,true,NULLAURA));
        SpellCastTargets tgt;
        tgt.m_unitTarget=pPet->GetGUID();
        sp->prepare(&tgt);
    }
}

void Spell::SpellEffectResurrect(uint32 i) // Resurrect (Flat)
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
                    uint32 mana = ((uint32)GetSpellProto()->EffectBasePoints[i] > unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER1)) ? unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER1) : (uint32)GetSpellProto()->EffectBasePoints[i];

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
                    unitTarget->SetUInt32Value(UNIT_FIELD_POWER1, mana);
                    unitTarget->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
                    unitTarget->setDeathState(ALIVE);
                    unitTarget->ClearLoot();
                    TO_CREATURE(unitTarget)->ClearTag();
                }
            }

            return;
        }
        playerTarget = objmgr.GetPlayer(corpseTarget->GetUInt32Value(CORPSE_FIELD_OWNER));
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
    playerTarget->SetMovement(MOVE_UNROOT, 1);
}

void Spell::SpellEffectAttackMe(uint32 i)
{
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    unitTarget->GetAIInterface()->AttackReaction(u_caster,1,0);
}

void Spell::SpellEffectSkinPlayerCorpse(uint32 i)
{
    Corpse* corpse = NULLCORPSE;
    if(playerTarget == NULL)
    {
        // means we're "skinning" a corpse
        corpse = objmgr.GetCorpse((uint32)m_targets.m_unitTarget);  // hacky
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
        owner = m_caster->GetMapMgr()->GetPlayer(corpse->GetUInt32Value(CORPSE_FIELD_OWNER));
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

void Spell::SpellEffectSkill(uint32 i)
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

void Spell::SpellEffectApplyPetAura(uint32 i)
{
    SpellEffectApplyAura(i);
}

void Spell::SpellEffectDummyMelee( uint32 i ) // Normalized Weapon damage +
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
        Aura* aura = u_caster->m_AuraInterface.FindActiveAura(58567);
        if(aura == NULL)
            u_caster->CastSpell(unitTarget, 58567, true);
        else
        {
            if(u_caster->HasAura(58388))
                aura->ModStackSize(2);
            else
                aura->ModStackSize(1);
            damage *= aura->stackSize;
        }
    }
    // rogue - mutilate ads dmg if target is poisoned
    // pure hax (damage join)
    else if( GetSpellProto()->NameHash == SPELL_HASH_MUTILATE )
    {
        damage = damage * 2;
        damage += CalculateDamage(u_caster, unitTarget, MELEE, GetSpellProto());
        damage += CalculateDamage(u_caster, unitTarget, OFFHAND, GetSpellProto());

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

    uint32 _type;
    if( GetType() == SPELL_DMG_TYPE_RANGED )
        _type = RANGED;
    else
    {
        if (GetSpellProto()->Flags4 & FLAGS4_OFFHAND)
            _type =  OFFHAND;
        else
            _type = MELEE;
    }
    TotalDamage += u_caster->Strike( unitTarget, _type, GetSpellProto(), damage, pct_dmg_mod, 0, false, false );
}

void Spell::SpellEffectSpellSteal( uint32 i )
{
    if ( unitTarget  == NULL ||  u_caster  == NULL || !unitTarget->isAlive())
        return;

    if( p_caster && p_caster->GetGUID() != unitTarget->GetGUID() )
    {
        if( unitTarget->IsPvPFlagged() )
        {
            if( p_caster->IsPlayer() )
                TO_PLAYER( p_caster )->PvPToggle();
            else
                p_caster->SetPvPFlag();
        }
    }

    if(!sFactionSystem.isAttackable(u_caster,unitTarget))
        return;

    unitTarget->m_AuraInterface.SpellStealAuras(u_caster, damage);
}

void Spell::SpellEffectProspecting(uint32 i)
{
    if( p_caster == NULL)
        return;

    if( itemTarget == NULL) // this should never happen
    {
        SendCastResult(SPELL_FAILED_ITEM_GONE);
        return;
    }

    uint32 entry = itemTarget->GetEntry();

    if(p_caster->GetItemInterface()->RemoveItemAmt(entry, 5))
    {
        p_caster->SetLootGUID(p_caster->GetGUID());
        lootmgr.FillItemLoot(&p_caster->m_loot, entry, p_caster->GetTeam());
        p_caster->SendLoot(p_caster->GetGUID(), p_caster->GetMapId(), LOOT_PROSPECTING);
    }
    else // this should never happen either
        SendCastResult(SPELL_FAILED_NEED_MORE_ITEMS);
}

void Spell::SpellEffectResurrectNew(uint32 i)
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
                    uint32 mana = ((uint32)GetSpellProto()->EffectBasePoints[i] > unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER1)) ? unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER1) : (uint32)GetSpellProto()->EffectBasePoints[i];

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
                    unitTarget->SetUInt32Value(UNIT_FIELD_POWER1, mana);
                    unitTarget->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
                    unitTarget->setDeathState(ALIVE);
                    unitTarget->ClearLoot();
                    TO_CREATURE(unitTarget)->ClearTag();
                }
            }

            return;
        }
        playerTarget = objmgr.GetPlayer(corpseTarget->GetUInt32Value(CORPSE_FIELD_OWNER));
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

void Spell::SpellEffectTranformItem(uint32 i)
{
    bool result;
    AddItemResult result2;

    if( i_caster == NULL)
        return;
    uint32 itemid=GetSpellProto()->EffectItemType[i];
    if(!itemid)
        return;

    //Save durability of the old item
    Player* owner=i_caster->GetOwner();
    uint32 dur= i_caster->GetUInt32Value(ITEM_FIELD_DURABILITY);

    result  = owner->GetItemInterface()->SafeFullRemoveItemByGuid(i_caster->GetGUID());
    if(!result)
    {
        //something went wrong if this happen, item doesnt exist, so it wasnt destroyed.
        return;
    }

    i_caster = NULLITEM;

    Item* it=objmgr.CreateItem(itemid,owner);
    it->SetDurability(dur);

    //additem
    result2 = owner->GetItemInterface()->AddItemToFreeSlot(it);
    if(!result2) //should never get here
    {
        owner->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM,INV_ERR_BAG_FULL);
        it->DeleteMe();
        it = NULLITEM;
    }
}

void Spell::SpellEffectEnvironmentalDamage(uint32 i)
{
    if(playerTarget == NULL)
        return;

    if(playerTarget->SchoolImmunityList[GetSpellProto()->School])
    {
        SendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }

    //this is GO, not unit
    TotalDamage += m_caster->SpellNonMeleeDamageLog(playerTarget,GetSpellProto()->Id,damage, pSpellId==0);

    WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, 13);
    data << playerTarget->GetGUID();
    data << uint8(DAMAGE_FIRE);
    data << uint32(damage);
    playerTarget->SendMessageToSet( &data, true );
}

void Spell::SpellEffectDismissPet(uint32 i)
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

void Spell::SpellEffectEnchantHeldItem( uint32 i )
{
    if( playerTarget == NULL )
        return;

    Item* item = playerTarget->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_MAINHAND );
    if( item == NULL )
        return;

    uint32 Duration = 1800; // Needs to be found in dbc.. I guess?
    switch(GetSpellProto()->NameHash)
    {
    case SPELL_HASH_WINDFURY_WEAPON: // Windfury Weapon Effect
    case SPELL_HASH_FLAMETONGUE_WEAPON: // Flametongue Weapon Effect
        {
            Duration = 10;
        }break;
    }

    EnchantEntry * Enchantment = dbcEnchant.LookupEntry( GetSpellProto()->EffectMiscValue[i] );
    if( Enchantment == NULL )
        return;

    item->RemoveEnchantment( 1 );
    item->AddEnchantment( Enchantment, Duration, false, true, false, 1 );
    item->SaveToDB(-1, EQUIPMENT_SLOT_MAINHAND, true, NULL);
}

void Spell::SpellEffectAddHonor(uint32 i)
{
    if( playerTarget != NULL )
        HonorHandler::AddHonorPointsToPlayer( playerTarget, GetSpellProto()->EffectBasePoints[i] );
}

void Spell::SpellEffectSpawn(uint32 i)
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

void Spell::SpellEffectApplyAura128(uint32 i)
{
    if(m_caster == NULL || !m_caster->IsInWorld())
        return;

    if( u_caster )
    {
        if(GetSpellProto()->EffectApplyAuraName[i] != 0)
            SpellEffectApplyAA(i);
    }
    else if(GetSpellProto() != NULL)
    {
        if(GetSpellProto()->EffectApplyAuraName[i] != 0)
            SpellEffectApplyAura(i);
    }
}

void Spell::SpellEffectRedirectThreat(uint32 i)
{
    if(!p_caster || !playerTarget)
        return;
    if(GetSpellProto()->NameHash == SPELL_HASH_TRICKS_OF_THE_TRADE)
        p_caster->CastSpell(playerTarget,57933,true);

    p_caster->SetRedirectThreat(playerTarget,GetSpellProto()->EffectBasePoints[i]+1,GetDuration());
}

void Spell::SpellEffectPlayMusic(uint32 i)
{
    if(!unitTarget)
        return;

    unitTarget->PlaySoundToSet(m_spellInfo->EffectMiscValue[i]);
}

void Spell::SpellEffectKillCredit( uint32 i )
{
    CreatureProto * cp = CreatureProtoStorage.LookupEntry( GetSpellProto()->EffectMiscValue[i] );
    if ( playerTarget != NULL && cp != NULL )
        sQuestMgr._OnPlayerKill( playerTarget, GetSpellProto()->EffectMiscValue[i]);
}

void Spell::SpellEffectRestoreManaPct(uint32 i)
{
    if( u_caster == NULL || unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 maxMana = (uint32)unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER1);
    uint32 modMana = damage * maxMana / 100;

    u_caster->Energize(unitTarget, pSpellId ? pSpellId : GetSpellProto()->Id, modMana, POWER_TYPE_MANA);
}

void Spell::SpellEffectRestoreHealthPct(uint32 i)
{
    if( u_caster == NULL || unitTarget == NULL || !unitTarget->isAlive())
        return;

    uint32 maxHp = (uint32)unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    uint32 modHp = damage * maxHp / 100;

    u_caster->Heal(unitTarget, GetSpellProto()->Id, modHp);
}

void Spell::SpellEffectForceCast( uint32 i )
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

void Spell::SpellEffectTriggerSpellWithValue(uint32 i)
{
    if( unitTarget == NULL )
        return;

    SpellEntry* TriggeredSpell = dbcSpell.LookupEntry(GetSpellProto()->EffectTriggerSpell[i]);
    if( TriggeredSpell == NULL )
        return;

    if(GetSpellProto() == TriggeredSpell) // Infinite loop fix.
        return;

    Spell* sp= new Spell(m_caster,dbcSpell.LookupEntry(TriggeredSpell->Id),true,NULLAURA);

    for(uint32 x=0;x<3;x++)
    {
        sp->forced_basepoints[x] = TriggeredSpell->EffectBasePoints[i];
    }

    SpellCastTargets tgt(unitTarget->GetGUID());
    sp->prepare(&tgt);
}

void Spell::SpellEffectJump(uint32 i)
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
        if(m_targets.m_destX == 0 || m_targets.m_destY == 0 || m_targets.m_destZ == 0)
            return;

        x = m_targets.m_destX;
        y = m_targets.m_destY;
        z = m_targets.m_destZ;
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
    else
        arc = 10.0f;

    uint32 time = uint32((CalculateEffect(i, unitTarget) / arc) * 100);
    u_caster->GetAIInterface()->StopMovement(time);
    u_caster->SetPosition(x, y, z, ang);
    u_caster->GetAIInterface()->SendJumpTo(x, y, z, time, arc);
    if( p_caster != NULL)
    {
        p_caster->ResetHeartbeatCoords();
        p_caster->DelaySpeedHack(time);
    }
}

void Spell::SpellEffectTeleportToCaster(uint32 i)
{
    if(!u_caster || !unitTarget)
        return;
    float x = u_caster->GetPositionX();
    float y = u_caster->GetPositionY();
    float z = u_caster->GetPositionZ();
    unitTarget->Teleport(x,y,z,u_caster->GetOrientation(),u_caster->GetPhaseMask());
}

void Spell::SpellEffectMilling(uint32 i)
{
    if(p_caster == NULL)
        return;

    if(itemTarget == NULL)
    {
        SendCastResult(SPELL_FAILED_ITEM_GONE);
        return;
    }

    uint32 entry = itemTarget->GetEntry();

    if(p_caster->GetItemInterface()->RemoveItemAmt(entry, 5))
    {
        p_caster->SetLootGUID(p_caster->GetGUID());
        lootmgr.FillItemLoot(&p_caster->m_loot, entry, p_caster->GetTeam());
        p_caster->SendLoot(p_caster->GetGUID(), p_caster->GetMapId(), LOOT_MILLING);
    }
    else
        SendCastResult(SPELL_FAILED_NEED_MORE_ITEMS);
}

void Spell::SpellEffectAllowPetRename( uint32 i )
{
    if( !unitTarget || !unitTarget->IsPet() )
        return;

    unitTarget->SetByte( UNIT_FIELD_BYTES_2, 2, 0x03);
}

void Spell::SpellEffectStartQuest(uint32 i)
{
    if( !playerTarget )
        return;

    playerTarget->StartQuest(GetSpellProto()->EffectMiscValue[i]);
}

// Spells: 46716-46730
void Spell::SpellEffectCreatePet(uint32 i)
{
    if( !playerTarget )
        return;

    if( playerTarget->getClass() != HUNTER )
        return;

    if( playerTarget->GetSummon() )
        playerTarget->GetSummon()->Remove( true, true, true );

    CreatureInfo *ci = CreatureNameStorage.LookupEntry( GetSpellProto()->EffectMiscValue[i] );
    CreatureProto* proto = CreatureProtoStorage.LookupEntry(GetSpellProto()->EffectMiscValue[i]);
    if( ci != NULL && proto != NULL )
    {
        Pet *pPet = objmgr.CreatePet();
        if(pPet != NULL)
        {
            pPet->CreateAsSummon( proto, ci, NULL, playerTarget, NULL, GetSpellProto(), 2, 0 );
            if(!pPet->IsInWorld())
            {
                pPet->Destruct();
                pPet = NULL;
            }
        }
    }
}

void Spell::SpellEffectTitanGrip(uint32 i)
{
    if (playerTarget == NULL)
        return;

    playerTarget->titanGrip = true;
}

bool Spell::SpellEffectUpdateQuest(uint32 questid)
{
    QuestLogEntry *en = p_caster->GetQuestLogForEntry( questid );
    if( en != NULL && en->GetMobCount( 0 ) < en->GetQuest()->required_mobcount[0])
    {
        en->SetMobCount( 0, en->GetMobCount( 0 ) + 1 );//(collumn,count)
        en->SendUpdateAddKill( 0 );//(collumn)
        en->UpdatePlayerFields();
        return true;
    }
    return false;
}

void Spell::SpellEffectCreateRandomItem(uint32 i) // Create Random Item
{
    if( p_caster == NULL)
        return;

    ItemPrototype *m_itemProto = NULL;
    Item* newItem = NULL;
    Item* add = NULL;
    uint8 slot = NULL;
    uint32 itemid;
    SlotResult slotresult;

    SkillLineSpell* skill = NULL;
    skill = objmgr.GetSpellSkill(GetSpellProto()->Id);

    itemid = GetSpellProto()->EffectItemType[i];
    m_itemProto = ItemPrototypeStorage.LookupEntry( GetSpellProto()->EffectItemType[i] );
    if ( m_itemProto == NULL || itemid == 0)
        return;

    uint32 item_count = 0;
    // Random Item to Create Jewelcrafting part
    RandomItemCreation * ric = RandomItemCreationStorage.LookupEntry( GetSpellProto()->Id );
    if(ric)
    {
        // If we Have Perfect Gem Cutting then we have a chance to create a Perfect Gem, according to comments on wowhead chance is between 20 and 30%
        if (Rand(ric->Chance) && ric->Skill == SKILL_JEWELCRAFTING && p_caster->HasSpell(55534))
        {
            m_itemProto = ItemPrototypeStorage.LookupEntry( ric->ItemToCreate );
            itemid  =   ric->ItemToCreate;
        }

        //Tarot and Decks from Inscription + Northrend Inscription Research + Minor Inscription Research
        //Northrend Alchemy
        if (ric->Skill == SKILL_INSCRIPTION || ric->Skill == SKILL_ALCHEMY)
        {
            uint32 k;
            RandomCardCreation * rcc = RandomCardCreationStorage.LookupEntry(GetSpellProto()->Id);
            if(rcc)
            {
                //Same chance for every card to appear according wowhead and wowwiki info
                k = RandomUInt(rcc->itemcount-1);
                m_itemProto = ItemPrototypeStorage.LookupEntry( rcc->ItemId[k] );
                itemid = rcc->ItemId[k];
                item_count = 1;
                switch(GetSpellProto()->Id)
                {
                    case 61288:
                    case 61177:
                        {
                            item_count = RandomUInt(2);//This 2 can make random scrolls and vellum 1 or 2 according to info
                        }break;
                    case 60893:
                        {
                            item_count = RandomUInt(3);//Creates 3 random elixir/potion from alchemy
                        }break;
                }
            }
        }
    }
    // Profession Discoveries used in Northrend Alchemy and Inscription Research plus Minor research
    uint32 discovered_recipe = 0;
    std::set<ProfessionDiscovery*>::iterator itr = objmgr.ProfessionDiscoveryTable.begin();
    for ( ; itr != objmgr.ProfessionDiscoveryTable.end(); itr++ )
    {
        ProfessionDiscovery * pf = NULL;
        pf = ( *itr );
        if ( pf != NULL && GetSpellProto()->Id == pf->SpellId && p_caster->_GetSkillLineCurrent( skill->skilline ) >= pf->SkillValue && !p_caster->HasSpell( pf->SpellToDiscover ))
        {
            discovered_recipe = pf->SpellToDiscover;
            break;
        }
    }

    // if something discovered learn p_caster that recipe and broadcast message
    if ( discovered_recipe != 0 )
    {
        SpellEntry * se = NULL;
        se = dbcSpell.LookupEntry( discovered_recipe );
        if ( se != NULL )
        {
            p_caster->addSpell( discovered_recipe );
            WorldPacket * data;
            char msg[256];
            sprintf( msg, "%sDISCOVERY! %s has discovered how to create %s.|r", MSG_COLOR_GOLD, p_caster->GetName(), se->Name );
            data = sChatHandler.FillMessageData( CHAT_MSG_SYSTEM, LANG_UNIVERSAL,  msg, p_caster->GetGUID(), 0 );
            p_caster->GetMapMgr()->SendChatMessageToCellPlayers( p_caster, data, 2, 1, LANG_UNIVERSAL, p_caster->GetSession() );
            delete data;
        }
    }
    if( m_itemProto == NULL )
        return;

    // item count cannot be more than allowed in a single stack
    if (m_itemProto->MaxCount > 0 && item_count > (uint32)m_itemProto->MaxCount)
        item_count = m_itemProto->MaxCount;

    // item count cannot be more than item unique value
    if (m_itemProto->Unique > 0 && item_count > (uint32)m_itemProto->Unique)
        item_count = m_itemProto->Unique;

    if(p_caster->GetItemInterface()->CanReceiveItem(m_itemProto, item_count, NULL))
        {
            SendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
            return;
        }

    slot = 0;
    add = p_caster->GetItemInterface()->FindItemLessMax(itemid,1, false);
    if (add == NULL)
    {
        slotresult = p_caster->GetItemInterface()->FindFreeInventorySlot(m_itemProto);
        if(!slotresult.Result)
        {
              SendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
              return;
        }

        newItem =objmgr.CreateItem(itemid,p_caster);
        if(newItem == NULL)
            return;
        newItem->SetUInt64Value(ITEM_FIELD_CREATOR,m_caster->GetGUID());
        newItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT, item_count);


        if(p_caster->GetItemInterface()->SafeAddItem(newItem,slotresult.ContainerSlot, slotresult.Slot))
            p_caster->GetSession()->SendItemPushResult(newItem,true,false,true,true,slotresult.ContainerSlot,slotresult.Slot,item_count);
        else
        {
            newItem->DeleteMe();
            newItem = NULLITEM;
        }

        if(skill!= NULL)
            DetermineSkillUp(skill->skilline);
    }
    else
    {
        //scale item_count down if total stack will be more than 20
        if(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + item_count > 20)
        {
            uint32 item_count_filled;
            item_count_filled = 20 - add->GetUInt32Value(ITEM_FIELD_STACK_COUNT);
            add->SetCount(20);
            add->m_isDirty = true;

            slotresult = p_caster->GetItemInterface()->FindFreeInventorySlot(m_itemProto);
            if(!slotresult.Result)
                item_count = item_count_filled;
            else
            {
                newItem =objmgr.CreateItem(itemid,p_caster);
                if(newItem == NULL)
                    return;
                newItem->SetUInt64Value(ITEM_FIELD_CREATOR,m_caster->GetGUID());
                newItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT, item_count - item_count_filled);
                if(!p_caster->GetItemInterface()->SafeAddItem(newItem,slotresult.ContainerSlot, slotresult.Slot))
                {
                    newItem->DeleteMe();
                    newItem = NULLITEM;
                    item_count = item_count_filled;
                }
                else
                    p_caster->GetSession()->SendItemPushResult(newItem, true, false, true, true, slotresult.ContainerSlot, slotresult.Slot, item_count-item_count_filled);
            }
        }
        else
        {
            add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + item_count);
            add->m_isDirty = true;
            p_caster->GetSession()->SendItemPushResult(add, true,false,true,false,p_caster->GetItemInterface()->GetBagSlotByGuid(add->GetGUID()),0xFFFFFFFF,item_count);
        }
        if(skill)
            DetermineSkillUp(skill->skilline);
    }
}

void Spell::SpellEffectSetTalentSpecsCount(uint32 i)
{
    if(p_caster == NULL)
        return;

    if(p_caster->m_talentActiveSpec >= damage) // activate primary spec
        p_caster->ApplySpec(0, false);

    p_caster->m_talentSpecsCount = damage;

    // Send update
    p_caster->smsg_TalentsInfo(false);
}

void Spell::SpellEffectActivateTalentSpec(uint32 i)
{
    if(p_caster == NULL)
        return;

    if(p_caster->m_bg)
    {
        SendCastResult(SPELL_FAILED_NOT_IN_BATTLEGROUND);
        return;
    }

    // 1 = primary, 2 = secondary
    p_caster->ApplySpec(uint8(damage - 1), false);

    // Use up all our power.
    switch(p_caster->GetPowerType())
    {
    case POWER_TYPE_MANA:
        p_caster->SetPower(POWER_TYPE_MANA, 0);
        break;
    case POWER_TYPE_RAGE:
        p_caster->SetPower(POWER_TYPE_RAGE, 0);
        break;
    case POWER_TYPE_ENERGY:
        p_caster->SetPower(POWER_TYPE_ENERGY, 0);
        break;
    case POWER_TYPE_RUNE:
        p_caster->SetPower(POWER_TYPE_RUNE, 0);
        break;
    }
}

void Spell::SpellEffectDisengage(uint32 i)
{
    if(unitTarget == NULL || !unitTarget->isAlive())
        return;

    unitTarget->knockback(GetSpellProto()->EffectBasePoints[i]+1, GetSpellProto()->EffectMiscValue[i], true);
}

void Spell::SpellEffectClearFinishedQuest(uint32 i)
{
    if (playerTarget == NULL)
        return;

    playerTarget->m_finishedQuests.erase(GetSpellProto()->EffectMiscValue[i]);
}

void Spell::SpellEffectApplyDemonAura( uint32 i )
{
    if (u_caster == NULL || !u_caster->IsPet() || TO_PET(u_caster)->GetPetOwner() == NULL)
        return;
    Aura* pAura = NULL;

    pAura = new Aura(GetSpellProto(), GetDuration(), u_caster, u_caster);
    Aura* otheraura = new Aura(GetSpellProto(), GetDuration(), u_caster, TO_PET(u_caster)->GetPetOwner());
    pAura->targets.insert(TO_PET(u_caster)->GetPetOwner()->GetUIdFromGUID());
    for (uint32 j=0; j<3; ++j)
    {
        pAura->AddMod(GetSpellProto()->EffectApplyAuraName[j], j == i? damage : CalculateEffect(j, unitTarget), GetSpellProto()->EffectMiscValue[j], j);
        otheraura->AddMod(GetSpellProto()->EffectApplyAuraName[j], j == i? damage : CalculateEffect(j, unitTarget), GetSpellProto()->EffectMiscValue[j], j);
    }

    u_caster->AddAura(pAura);
    TO_PET(u_caster)->GetPetOwner()->AddAura(otheraura);
}

void Spell::SpellEffectRemoveAura(uint32 i)
{
    if (!unitTarget)
        return;

    unitTarget->RemoveAura(GetSpellProto()->EffectTriggerSpell[i], unitTarget->GetGUID());
}

void Spell::SpellEffectActivateRune(uint32 i)
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

void Spell::SpellEffectFailQuest(uint32 i)
{
    if( !p_caster )
        return;
    Quest * qst = sQuestMgr.GetQuestPointer(GetSpellProto()->EffectMiscValue[i]);
    if(!qst)
        return;

    sQuestMgr.SendQuestFailed(FAILED_REASON_FAILED, qst, p_caster);
}