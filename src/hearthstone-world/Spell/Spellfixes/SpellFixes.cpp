/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void PoolSpellData();
void SetSingleSpellDefaults(SpellEntry *sp);

void ApplyNormalFixes()
{
    //Updating spell.dbc
    SpellEntry *sp;
    set<uint32> DummySpells;
    map<uint32, uint32>::iterator talentSpellIterator;
    map<uint32, uint32> talentSpells, dummySpellLevels;
    sLog.Notice("World", "Processing %u spells...", dbcSpell.GetNumRows());
    sLog.Notice("World", "Highest spell found %u...", dbcSpell.GetMaxRow());
    for(uint i = 0; i < dbcTalent.GetNumRows(); i++)
    {
        TalentEntry *tal = dbcTalent.LookupRow(i);
        for(uint j = 0; j < 5; ++j)
            if(tal->RankID[j] != 0)
                talentSpells.insert(make_pair(tal->RankID[j], tal->TalentTree));
    }

    sLog.Notice("World", "Filling spell default values...");
    // We must set spell defaults before we load static pool data
    for(uint32 x = 0; x < dbcSpell.GetNumRows(); x++)
    {
        sp = dbcSpell.LookupRow(x);
        if(sp == NULL)
            continue;

        SetSingleSpellDefaults(sp);
    }

    sLog.Notice("World", "Filling spell pool data...");
    PoolSpellData();

    sLog.Notice("World", "Filling spell pool data...");
    for(uint32 x = 0; x < dbcSpell.GetNumRows(); x++)
    {
        sp = dbcSpell.LookupRow(x);
        if(sp == NULL)
            continue;

        if(DummySpells.find(sp->Id) != DummySpells.end())
            continue; // Dummy spells will be handled later.

        uint32 type = 0;
        for(uint i = 0; i < 8; i++)
        {
            if(sp->School & (1<<i))
            {
                sp->School = i;
                break;
            }
        }

        for(uint32 b = 0; b < 3; ++b)
        {
            if(sp->EffectTriggerSpell[b] != 0 && dbcSpell.LookupEntry(sp->EffectTriggerSpell[b]) == NULL)
            {
                /* proc spell referencing non-existant spell. create a dummy spell for use w/ it. */
                DummySpells.insert(sp->EffectTriggerSpell[b]);
            }

            /** Load teaching spells (used for hunters when learning pets wild abilities) */
            if(sp->Effect[b] == SPELL_EFFECT_LEARN_SPELL && sp->EffectImplicitTargetA[b] == EFF_TARGET_PET)
            {
                map<uint32,uint32>::iterator itr = sWorld.TeachingSpellMap.find(sp->EffectTriggerSpell[b]);
                if(itr == sWorld.TeachingSpellMap.end())
                    sWorld.TeachingSpellMap.insert(make_pair(sp->EffectTriggerSpell[b],sp->Id));
            }
            if( sp->Attributes & ATTRIBUTES_ONLY_OUTDOORS && sp->EffectApplyAuraName[b] == SPELL_AURA_MOUNTED )
            {
                sp->Attributes &= ~ATTRIBUTES_ONLY_OUTDOORS;
            }
        }

        if(strstr(sp->Name, "Hearthstone") || strstr(sp->Name, "Stuck") || strstr(sp->Name, "Astral Recall"))
            sp->self_cast_only = true;

        talentSpellIterator = talentSpells.find(sp->Id);
        if(talentSpellIterator != talentSpells.end())
            sp->talent_tree = talentSpellIterator->second;
        else sp->talent_tree = 0;

        SkillLineSpell *sk = objmgr.GetSpellSkill(sp->Id);
        sp->skilline = sk ? sk->skilline : 0;

        //Rogue: Posion time fix for 2.3
        if(strstr( sp->Name, "Mind-numbing Poison") || (strstr( sp->Name, "Wound Poison") || strstr( sp->Name, "Crippling Poison")
            || strstr( sp->Name, "Instant Poison") || strstr( sp->Name, "Deadly Poison") || strstr( sp->Name, "Anesthetic Poison")
            || strstr( sp->Name, "Sharpen Blade"))
            && sp->Effect[0] == 54 )
        {
            sp->EffectBasePoints[0] = 3599;
            sp->self_cast_only = true; // Just for double checks
        }

        // these mostly do not mix so we can use else
        // look for seal, etc in name
        bool ccchanged = false;
        switch(sp->NameHash)
        {
        case SPELL_HASH_TRACK_HUMANOIDS: // apply on shapeshift change
            {
                sp->apply_on_shapeshift_change = true;
                ccchanged = true;
            }break;

        case SPELL_HASH_BLOOD_FURY:
        case SPELL_HASH_SHADOWSTEP:
            {
                sp->always_apply = true;
                ccchanged = true;
            }break;

        case SPELL_HASH_JUDGEMENT_OF_COMMAND: //judgement of command
            {
                sp->Spell_Dmg_Type = SPELL_DMG_TYPE_MAGIC;
                ccchanged = true;
            }break;

        case SPELL_HASH_MARK_OF_THE_WILD:
            {
                type |= SPELL_TYPE_MARK_GIFT;
                ccchanged = true;
            }break;

        case SPELL_HASH_IMMOLATION_TRAP:
        case SPELL_HASH_EXPLOSIVE_TRAP:
        case SPELL_HASH_FREEZING_TRAP:
        case SPELL_HASH_FROST_TRAP:
        case SPELL_HASH_SNAKE_TRAP:
            {
                type |= SPELL_TYPE_HUNTER_TRAP;
                ccchanged = true;
            }break;

        case SPELL_HASH_AMPLIFY_MAGIC:
        case SPELL_HASH_DAMPEN_MAGIC:
            {
                type |= SPELL_TYPE_MAGE_MAGI;
                ccchanged = true;
            }break;

        case SPELL_HASH_FROST_WARD:
        case SPELL_HASH_FIRE_WARD:
            {
                type |= SPELL_TYPE_MAGE_WARDS;
                ccchanged = true;
            }break;

        case SPELL_HASH_PRAYER_OF_SHADOW_PROTECTION:
        case SPELL_HASH_SHADOW_PROTECTION:
            {
                type |= SPELL_TYPE_PRIEST_SH_PPROT;
                ccchanged = true;
            }break;

        case SPELL_HASH_LIGHTNING_SHIELD:
        case SPELL_HASH_WATER_SHIELD:
        case SPELL_HASH_EARTH_SHIELD:
            {
                type |= SPELL_TYPE_SHIELD;
                ccchanged = true;
            }break;

        case SPELL_HASH_POWER_WORD__FORTITUDE:
        case SPELL_HASH_PRAYER_OF_FORTITUDE:
            {
                type |= SPELL_TYPE_FORTITUDE;
                ccchanged = true;
            }break;

        case SPELL_HASH_DIVINE_SPIRIT:
            {
                type |= SPELL_TYPE_SPIRIT;
                ccchanged = true;
            }break;

        case SPELL_HASH_HUNTER_S_MARK: // hunter's mark
            {
                type |= SPELL_TYPE_HUNTER_MARK;
                ccchanged = true;
            }break;

        case SPELL_HASH_COMMANDING_SHOUT:
        case SPELL_HASH_BATTLE_SHOUT:
            {
                type |= SPELL_TYPE_WARRIOR_SHOUT;
                ccchanged = true;
            }break;

        case SPELL_HASH_FROST_PRESENCE:
        case SPELL_HASH_BLOOD_PRESENCE:
        case SPELL_HASH_UNHOLY_PRESENCE:
            {
                if(( sp->Id != 61261 ) && ( sp->Id != 49772 ))
                {
                    type |= SPELL_TYPE_DK_PRESENCE;
                    ccchanged = true;
                }
            }break;
        case SPELL_HASH_ARCANE_INTELLECT:
        case SPELL_HASH_ARCANE_BRILLIANCE:
        case SPELL_HASH_DALARAN_BRILLIANCE:
            {
                type |= SPELL_TYPE_MAGE_INTEL;
                ccchanged = true;
            }break;
        }

        if(!ccchanged)
        {
            if( strstr( sp->Name, "Seal"))
                type |= SPELL_TYPE_SEAL;
            else if( strstr( sp->Name, "Blessing"))
                type |= SPELL_TYPE_BLESSING;
            else if( strstr( sp->Name, "Curse"))
                type |= SPELL_TYPE_CURSE;
            else if( strstr( sp->Name, "Aspect"))
                type |= SPELL_TYPE_ASPECT;
            else if( strstr( sp->Name, "Sting") || strstr( sp->Name, "sting"))
                type |= SPELL_TYPE_STING;
            else if( strstr( sp->Name, "Judgement of") && !(strstr( sp->Name, "Increased") || strstr( sp->Name, "Improved")))
                type |= SPELL_TYPE_JUDGEMENT;
            // don't break armor items!
            else if(strcmp(sp->Name, "Armor") && strstr( sp->Name, "Armor") || strstr( sp->Name, "Demon Skin"))
                type |= SPELL_TYPE_ARMOR;
            else if( strstr( sp->Name, "Concentration Aura") || strstr( sp->Name, "Crusader Aura") || strstr( sp->Name, "Devotion Aura") || strstr( sp->Name, "Fire Resistance Aura") || strstr( sp->Name, "Frost Resistance Aura") || strstr( sp->Name, "Retribution Aura") || strstr( sp->Name, "Shadow Resistance Aura") )
                type |= SPELL_TYPE_AURA;
            else if( strstr( sp->Name, "Track")==sp->Name)
                type |= SPELL_TYPE_TRACK;
            else if( strstr( sp->Name, "Immolate") || strstr( sp->Name, "Conflagrate"))
                type |= SPELL_TYPE_WARLOCK_IMMOLATE;
            else if( strstr( sp->Name, "Amplify Magic") || strstr( sp->Name, "Dampen Magic"))
                type |= SPELL_TYPE_MAGE_AMPL_DUMP;
            else if( strstr( sp->Description, "Battle Elixir"))
                type |= SPELL_TYPE_ELIXIR_BATTLE;
            else if( strstr( sp->Description, "Guardian Elixir"))
                type |= SPELL_TYPE_ELIXIR_GUARDIAN;
            else if( strstr( sp->Description, "Battle and Guardian elixir"))
                type |= SPELL_TYPE_ELIXIR_FLASK;
            else if( strstr( sp->Description, "Finishing move")==sp->Description)
                sp->c_is_flags |= SPELL_FLAG_IS_FINISHING_MOVE;
        }

        for(uint32 b = 0; b < 3; ++b)
        {
            if( sp->Effect[b] == SPELL_EFFECT_PERSISTENT_AREA_AURA )
            {
                sp->EffectImplicitTargetA[b] = EFF_TARGET_SELF;
                sp->EffectImplicitTargetB[b] = 0;
            }

            // 3.0.3 totemzzz
            if( sp->Effect[b] == SPELL_EFFECT_HEALTH_FUNNEL )
            {
                sp->Effect[b] = SPELL_EFFECT_APPLY_AREA_AURA;
            }
        }

        // parse rank text
        if( !sscanf( sp->Rank, "Rank %d", (unsigned int*)&sp->RankNumber) )
        {
            const char* ranktext = sp->Rank;
            uint32 rank = 0;

            //stupid spell ranking problem
            if( strstr( ranktext, "Apprentice"))
                rank = 1;
            else if( strstr( ranktext, "Journeyman"))
                rank = 2;
            else if( strstr( ranktext, "Expert"))
                rank = 3;
            else if( strstr( ranktext, "Artisan"))
                rank = 4;
            else if( strstr( ranktext, "Master"))
                rank = 5;
            else if( strstr( ranktext, "Grandmaster"))
                rank = 6;
            sp->RankNumber = rank;
        }

        if(sp->spellLevel == 0)
        {
            uint32 new_level = sp->RankNumber;
            if(new_level)
            {
                uint32 teachspell = 0;
                if(sp->Effect[0]==SPELL_EFFECT_LEARN_SPELL)
                    teachspell = sp->EffectTriggerSpell[0];
                else if(sp->Effect[1]==SPELL_EFFECT_LEARN_SPELL)
                    teachspell = sp->EffectTriggerSpell[1];
                else if(sp->Effect[2]==SPELL_EFFECT_LEARN_SPELL)
                    teachspell = sp->EffectTriggerSpell[2];

                if(teachspell)
                {
                    SpellEntry *spellInfo = dbcSpell.LookupEntry(teachspell);
                    if(spellInfo == NULL)
                    {
                        DummySpells.insert(teachspell);
                        dummySpellLevels.insert(make_pair(teachspell, new_level));
                    }
                    else spellInfo->spellLevel = new_level;
                    sp->spellLevel = new_level;
                }
            }
        }

        sp->buffIndexType = 0;
        switch(sp->NameHash)
        {
        case SPELL_HASH_HUNTER_S_MARK:      // Hunter's mark
            sp->buffIndexType = SPELL_TYPE_INDEX_MARK;
            break;

        case SPELL_HASH_POLYMORPH:          // Polymorph
        case SPELL_HASH_POLYMORPH__CHICKEN: // Polymorph: Chicken
        case SPELL_HASH_POLYMORPH__SHEEP:   // Polymorph: Sheep
            sp->buffIndexType = SPELL_TYPE_INDEX_POLYMORPH;
            break;

        case SPELL_HASH_FEAR:               // Fear
            sp->buffIndexType = SPELL_TYPE_INDEX_FEAR;
            break;

        case SPELL_HASH_SAP:                // Sap
            sp->buffIndexType = SPELL_TYPE_INDEX_SAP;
            break;

        case SPELL_HASH_SCARE_BEAST:        // Scare Beast
            sp->buffIndexType = SPELL_TYPE_INDEX_SCARE_BEAST;
            break;

        case SPELL_HASH_HIBERNATE:          // Hibernate
            sp->buffIndexType = SPELL_TYPE_INDEX_HIBERNATE;
            break;

        case SPELL_HASH_EARTH_SHIELD:       // Earth Shield
            sp->buffIndexType = SPELL_TYPE_INDEX_EARTH_SHIELD;
            break;

        case SPELL_HASH_CYCLONE:            // Cyclone
            sp->buffIndexType = SPELL_TYPE_INDEX_CYCLONE;
            break;

        case SPELL_HASH_BANISH:             // Banish
            sp->buffIndexType = SPELL_TYPE_INDEX_BANISH;
            break;

        //case SPELL_HASH_JUDGEMENT_OF_VENGEANCE:
        case SPELL_HASH_JUDGEMENT_OF_LIGHT:
            sp->buffIndexType = SPELL_TYPE_INDEX_JUDGEMENT;
            break;

        case SPELL_HASH_REPENTANCE:
            sp->buffIndexType = SPELL_TYPE_INDEX_REPENTANCE;
            break;
        case SPELL_HASH_SLOW:
            sp->buffIndexType = SPELL_TYPE_INDEX_SLOW;
            break;
        }

        // HACK FIX: Break roots/fear on damage.. this needs to be fixed properly!
        uint32 z;
        if(!(sp->AuraInterruptFlags & AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN))
        {
            for(z = 0; z < 3; ++z)
            {
                if(sp->EffectApplyAuraName[z] == SPELL_AURA_MOD_FEAR ||
                    sp->EffectApplyAuraName[z] == SPELL_AURA_MOD_ROOT)
                {
                    sp->AuraInterruptFlags |= AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN;
                    break;
                }
            }
        }

        for(z = 0;z < 3; ++z)
        {
            if( ( sp->Effect[z] == SPELL_EFFECT_SCHOOL_DAMAGE && sp->Spell_Dmg_Type == SPELL_DMG_TYPE_MELEE ) || sp->Effect[z] == SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL || sp->Effect[z] == SPELL_EFFECT_WEAPON_DAMAGE || sp->Effect[z] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE || sp->Effect[z] == SPELL_EFFECT_DUMMYMELEE )
                sp->is_melee_spell = true;
            if( ( sp->Effect[z] == SPELL_EFFECT_SCHOOL_DAMAGE && sp->Spell_Dmg_Type == SPELL_DMG_TYPE_RANGED ) )
            {
                //sLog.outString( "Ranged Spell: %u [%s]" , sp->Id , sp->Name );
                sp->is_ranged_spell = true;
            }

            switch(sp->EffectImplicitTargetA[z])
            {
            case 6:
            case 22:
            case 24:
            case 25:
                sp->c_is_flags |= SPELL_FLAG_CAN_BE_REFLECTED;
                break;
            }

            if( sp->Effect[z] == SPELL_EFFECT_DISPEL )
                sp->c_is_flags |= SPELL_FLAG_IS_DISPEL_SPELL;

            if( sp->Effect[z] == SPELL_EFFECT_HEAL )
                sp->c_is_flags |= SPELL_FLAG_IS_HEALING_SPELL;

            if( sp->Effect[z] == SPELL_EFFECT_ENERGIZE )
                sp->c_is_flags |= SPELL_FLAG_IS_HEALING_MANA_SPELL;

        }

        // set extra properties
        sp->buffType = type;
        SetProcFlags(sp);

        if( strstr( sp->Description, "Must remain seated"))
        {
            sp->RecoveryTime = 1000;
            sp->CategoryRecoveryTime = 1000;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////
        // procintervals
        //////////////////////////////////////////////////////////////////////////////////////////////////////
        //lightning shield trigger spell id's are all wrong ?
        //if you are bored you could make these by hand but i guess we might find other spells with this problem..and this way it's safe
        if( strstr( sp->Name, "Lightning Shield" ) && sp->EffectTriggerSpell[0] )
        {
            sp->proc_interval = 3000; //few seconds
        }
        //mage ignite talent should proc only on some chances
        else if( strstr( sp->Name, "Ignite") && sp->Id>=11119 && sp->Id<=12848 && sp->EffectApplyAuraName[0] == 4 )
        {
            //check if we can find in the description
            char *startofid=strstr(sp->Description, "an additional ");
            if(startofid)
            {
                startofid += strlen("an additional ");
                sp->EffectBasePoints[0]=atoi(startofid); //get new value. This is actually level*8 ;)
                if( strstr(sp->Description, "over $") )
                    sp->EffectTriggerSpell[0] = GetTriggerSpellFromDescription("over $", sp->Description);
            }
            sp->Effect[0] = 6; //aura
            sp->EffectApplyAuraName[0] = 42; //force him to use procspell effect
            sp->procflags2 = PROC_ON_SPELL_CRIT_HIT; //add procflag here since this was not processed with the others !
        }
        // Winter's Chill handled by frost school
        else if( strstr( sp->Name, "Winter's Chill"))
        {
            sp->School = 4;
        }
        // Blackout handled by Shadow school
        else if( strstr( sp->Name, "Blackout"))
        {
            sp->School = 5;
        }

        else if( strstr( sp->Name, "Touch of Weakness"))
        {
            //check if we can find in the description
            if( strstr(sp->Description, "cause $") )
            {
                sp->EffectTriggerSpell[0] = GetTriggerSpellFromDescription("cause $", sp->Description);
                sp->EffectTriggerSpell[1]=sp->EffectTriggerSpell[0]; //later versions of this spell changed to eff[1] the aura
                sp->procFlags = uint32(PROC_ON_MELEE_ATTACK_VICTIM);
            }
        }
        else if( strstr( sp->Name, "Firestone Passive"))
        {
            //Enchants the main hand weapon with fire, granting each attack a chance to deal $17809s1 additional fire damage.
            //check if we can find in the description
            if( strstr(sp->Description, "to deal $") )
            {
                sp->EffectTriggerSpell[0] = GetTriggerSpellFromDescription("to deal $", sp->Description);
                sp->EffectApplyAuraName[0] = 42;
                sp->procFlags = PROC_ON_MELEE_ATTACK;
                sp->procChance = 50;
            }
        }
        //some procs trigger at intervals
        else if(sp->NameHash == SPELL_HASH_WATER_SHIELD)
        {
            sp->proc_interval = 3000; //few seconds
            sp->procflags2 |= PROC_TARGET_SELF;
        }
        else if(sp->NameHash == SPELL_HASH_EARTH_SHIELD)
            sp->proc_interval = 3000; //few seconds
        else if( strstr( sp->Name, "Shadowguard"))
            sp->proc_interval = 3000; //few seconds
        else if( strstr( sp->Name, "Poison Shield"))
            sp->proc_interval = 3000; //few seconds
        else if( strstr( sp->Name, "Infused Mushroom"))
            sp->proc_interval = 10000; //10 seconds
        else if( strstr( sp->Name, "Aviana's Purpose"))
            sp->proc_interval = 10000; //10 seconds
        //don't change to namehash since we are searching only a protion of the name
        else if( strstr( sp->Name, "Crippling Poison"))
        {
            sp->c_is_flags |= SPELL_FLAG_IS_POISON;
            sp->poison_type = POISON_TYPE_CRIPPLING;
        }
        else if( strstr( sp->Name, "Mind-Numbing Poison"))
        {
            sp->c_is_flags |= SPELL_FLAG_IS_POISON;
            sp->poison_type = POISON_TYPE_MIND_NUMBING;
        }
        else if( strstr( sp->Name, "Instant Poison"))
        {
            sp->poison_type = POISON_TYPE_INSTANT;
            sp->c_is_flags |= SPELL_FLAG_IS_POISON;
        }
        else if( strstr( sp->Name, "Scorpid Poison") )
        {
            sp->c_is_flags |= SPELL_FLAG_IS_POISON;
            sp->poison_type = POISON_TYPE_SCORPID;
        }
        else if( strstr( sp->Name, "Deadly Poison"))
        {
            sp->c_is_flags |= SPELL_FLAG_IS_POISON;
            sp->poison_type = POISON_TYPE_DEADLY;
        }
        else if( strstr( sp->Name, "Wound Poison"))
        {
            sp->c_is_flags |= SPELL_FLAG_IS_POISON;
            sp->poison_type = POISON_TYPE_WOUND;
        }

        if( strstr( sp->Description, "cannot be resisted" ) )
            sp->c_is_flags |= SPELL_FLAG_IS_NOT_RESISTABLE;

        if( sp->School == SCHOOL_HOLY && !(sp->c_is_flags & SPELL_FLAG_IS_NOT_RESISTABLE) )
            sp->c_is_flags |= SPELL_FLAG_IS_NOT_RESISTABLE;

        if( strstr( sp->Description, "pierces through all absorption effects" ) )
            sp->c_is_flags |= SPELL_FLAG_PIERCES_ABSORBTION_EFF;

        //Set Silencing spells mech.
                // Set default mechanics if we don't already have one
        if( !sp->MechanicsType )
        {
            //Set Silencing spells mechanic.
            if( sp->EffectApplyAuraName[0] == 27 ||
                sp->EffectApplyAuraName[1] == 27 ||
                sp->EffectApplyAuraName[2] == 27 )
                sp->MechanicsType = MECHANIC_SILENCED;

            //Set Stunning spells mechanic.
            if( sp->EffectApplyAuraName[0] == 12 ||
                sp->EffectApplyAuraName[1] == 12 ||
                sp->EffectApplyAuraName[2] == 12 )
                sp->MechanicsType = MECHANIC_STUNNED;

            //Set Fearing spells mechanic
            if( sp->EffectApplyAuraName[0] == 7 ||
                sp->EffectApplyAuraName[1] == 7 ||
                sp->EffectApplyAuraName[2] == 7 )
                sp->MechanicsType = MECHANIC_FLEEING;
        }

        // Sap, Gouge, Blehhhh
        if( sp->NameHash == SPELL_HASH_GOUGE ||
            sp->NameHash == SPELL_HASH_SAP ||
            sp->NameHash == SPELL_HASH_REPENTANCE ||
            sp->NameHash == SPELL_HASH_MAIM ||
            sp->NameHash == SPELL_HASH_GOBLIN_ROCKET_HELMET ||
            sp->NameHash == SPELL_HASH_RECKLESS_CHARGE)
            sp->MechanicsType = MECHANIC_INCAPACIPATED;

        if( sp->proc_interval != 0 )
            sp->procflags2 |= PROC_REMOVEONUSE;

        if( sp->NameHash == SPELL_HASH_DIVINE_SHIELD || sp->NameHash == SPELL_HASH_DIVINE_PROTECTION || sp->NameHash == SPELL_HASH_BLESSING_OF_PROTECTION || sp->NameHash == SPELL_HASH_HAND_OF_PROTECTION )
            sp->MechanicsType = 25;

        if(sp->NameHash == SPELL_HASH_DRINK && sp->EffectBasePoints[0] == -1 &&
            sp->EffectApplyAuraName[1] == 226 && sp->EffectBasePoints[1] > 0)
        {
            sp->EffectBasePoints[0] = sp->EffectBasePoints[1];
            sp->Effect[1] = SPELL_EFFECT_NULL;
        }

        if(
            ((sp->Attributes & ATTRIBUTES_TRIGGER_COOLDOWN) && (sp->AttributesEx & ATTRIBUTESEX_NOT_BREAK_STEALTH)) //rogue cold blood
            || ((sp->Attributes & ATTRIBUTES_TRIGGER_COOLDOWN) && (!sp->AttributesEx || sp->AttributesEx & ATTRIBUTESEX_REMAIN_OOC))
            )
        {
            sp->c_is_flags |= SPELL_FLAG_IS_REQUIRECOOLDOWNUPDATE;
        }

        /////////////////////////////////////////////////////////////////
        //SPELL COEFFICIENT SETTINGS START
        //////////////////////////////////////////////////////////////////
        for(uint8 i = 0 ; i < 3; i++)
        {
            switch (sp->EffectImplicitTargetA[i])
            {
                //AoE
            case EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS:
            case EFF_TARGET_ALL_ENEMY_IN_AREA:
            case EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT:
            case EFF_TARGET_ALL_PARTY_AROUND_CASTER:
            case EFF_TARGET_ALL_ENEMIES_AROUND_CASTER:
            case EFF_TARGET_IN_FRONT_OF_CASTER:
            case EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
            case EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED:
            case EFF_TARGET_ALL_FRIENDLY_IN_AREA:
            case EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME:
            case EFF_TARGET_ALL_PARTY:
            case EFF_TARGET_LOCATION_INFRONT_CASTER:
            case EFF_TARGET_BEHIND_TARGET_LOCATION:
            case EFF_TARGET_LOCATION_INFRONT_CASTER_AT_RANGE:
                {
                    sp->isAOE = true;
                    break;
                }
            }

            switch (sp->EffectImplicitTargetB[i])
            {
                //AoE
            case EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS:
            case EFF_TARGET_ALL_ENEMY_IN_AREA:
            case EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT:
            case EFF_TARGET_ALL_PARTY_AROUND_CASTER:
            case EFF_TARGET_ALL_ENEMIES_AROUND_CASTER:
            case EFF_TARGET_IN_FRONT_OF_CASTER:
            case EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
            case EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED:
            case EFF_TARGET_ALL_FRIENDLY_IN_AREA:
            case EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME:
            case EFF_TARGET_ALL_PARTY:
            case EFF_TARGET_LOCATION_INFRONT_CASTER:
            case EFF_TARGET_BEHIND_TARGET_LOCATION:
            case EFF_TARGET_LOCATION_INFRONT_CASTER_AT_RANGE:
                {
                    sp->isAOE = true;
                    break;
                }
            }
            if(sp->Effect[i] == 0)
                continue;

            //Apply aura fixes
            if( sp->Effect[i] == SPELL_EFFECT_APPLY_AURA )
            {
                switch( sp->EffectApplyAuraName[i] )
                {
                case SPELL_AURA_MOD_CONFUSE:
                    sp->auraimmune_flag |= AURAIMMUNE_CONFUSE;break;
                case SPELL_AURA_MOD_CHARM:
                    sp->auraimmune_flag |= AURAIMMUNE_CHARM;break;
                case SPELL_AURA_MOD_FEAR:
                    sp->auraimmune_flag |= AURAIMMUNE_FEAR;break;
                case SPELL_AURA_MOD_STUN:
                    sp->auraimmune_flag |= AURAIMMUNE_STUN;break;
                case SPELL_AURA_MOD_PACIFY:
                    sp->auraimmune_flag |= AURAIMMUNE_PACIFY;break;
                case SPELL_AURA_MOD_ROOT:
                    sp->auraimmune_flag |= AURAIMMUNE_ROOT;break;
                case SPELL_AURA_MOD_SILENCE:
                    sp->auraimmune_flag |= AURAIMMUNE_SILENCE;break;
                case SPELL_AURA_MOD_INCREASE_SPEED:
                    sp->auraimmune_flag |= AURAIMMUNE_INCSPEED;break;
                case SPELL_AURA_MOD_DECREASE_SPEED:
                    sp->auraimmune_flag |= AURAIMMUNE_DECSPEED;break;
                case SPELL_AURA_TRANSFORM:
                    sp->auraimmune_flag |= AURAIMMUNE_TRANSFORM;break;
                case SPELL_AURA_MOD_TAUNT:
                    sp->auraimmune_flag |= AURAIMMUNE_TAUNT;break;
                }
            }
        }

        /// SPELLS CAN CRIT ///
        sp->spell_can_crit = true; // - except in special cases noted in this section

        //////////////////////////////////////////
        // OTHER                                //
        //////////////////////////////////////////
        // fix for relics procs - pure hax
        if( sp->EffectSpellClassMask[0][1] == 16 &&
            sp->EffectSpellClassMask[1][0] == 995691415 &&
            sp->EffectSpellClassMask[1][1] == 64 )
        {
            sp->EffectSpellClassMask[0][1] = 0;
            sp->EffectSpellClassMask[1][0] = 0;
            sp->EffectSpellClassMask[1][1] = 0;
        }

        // Crow: We should put this at the front some time in the future.
        // Apply spell fixes.
        ApplySingleSpellFixes(sp);
        ApplyCoeffSpellFixes(sp);
    }

    sLog.Notice("World", "Setting target flags...");
    SetupSpellTargets();

    sLog.Notice("World", "Processing %u dummy spells...", DummySpells.size());
    set<uint32>::iterator itr = DummySpells.begin();
    if(itr != DummySpells.end())
    {
        SpellEntry* Sp;
        for(; itr != DummySpells.end(); itr++)
        {   // Crow: Create the dummy spell, and apply fixs :D
            SetSingleSpellDefaults(Sp = CreateDummySpell(*itr));
            ApplySingleSpellFixes(Sp);
            ApplyCoeffSpellFixes(Sp);
            SetProcFlags(Sp);
            if(dummySpellLevels.find(*itr) != dummySpellLevels.end())
                Sp->spellLevel = dummySpellLevels[*itr];
        }
    }

    /////////////////////////////////////////////////////////////////
    //FORCER CREATURE SPELL TARGETING
    //////////////////////////////////////////////////////////////////
    sLog.Notice("World", "Loading forced targets for spells...");
    QueryResult * resultfcst = WorldDatabase.Query("SELECT * FROM spell_forced_targets");
    if( resultfcst != NULL )
    {
        sLog.Notice("World", "Forcing targets for %u spells...", resultfcst->GetRowCount());
        do
        {
            Field * f = resultfcst->Fetch();
            sp = dbcSpell.LookupEntry( f[0].GetUInt32() );
            if( sp )
                sp->forced_creature_target = f[1].GetUInt32();

        }while( resultfcst->NextRow() );
        delete resultfcst;
    }

    //GenerateNameHashesFile();

    sp = dbcSpell.LookupEntry( 26659 );
    SpellEntry* sp2 = sp;
    sp2->Id = 62388;
    sp2->Name = ((char*)"Dummy Shit");
    sp2->DurationIndex = 41;
    sp2->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    dbcSpell.SetRow(62388,sp2);
}

void SetProcFlags(SpellEntry *sp)
{
    uint32 effect;
    uint32 pr = sp->procFlags;
    uint32 pr2 = sp->procflags2;
    for(uint32 y = 0; y < 3; y++)
    {
        // Get the effect number from the spell
        effect = sp->Effect[y];

        if(effect == SPELL_EFFECT_APPLY_AURA)
        {
            uint32 aura = sp->EffectApplyAuraName[y];
            if( aura == SPELL_AURA_PROC_TRIGGER_SPELL ||
                aura == SPELL_AURA_PROC_TRIGGER_DAMAGE ||
                aura == SPELL_AURA_DUMMY )
            {
                uint32 procspellid = 0;
                for(uint j = 0; j < 3; j++)
                    if(sp->EffectTriggerSpell[j])
                        procspellid = sp->EffectTriggerSpell[j];

                char desc[4096];
                strcpy(desc, sp->Description);
                uint32 len = (uint32)strlen(desc);
                for(uint i = 0; i < len; i++)
                    desc[i] = tolower(desc[i]);

                //dirty code for procs, if any1 got any better idea-> u are welcome
                //139944 --- some magic number, it will trigger on all hits etc
                // Crow: All of the below should be in lower case, but its late.
                if((pr & PROC_ON_MELEE_ATTACK) == 0 && (strstr( desc,"chance on hit") || strstr( desc,"your auto attacks") || strstr( desc,"character strikes an enemy")
                    || strstr( desc,"when it hits") || strstr( desc,"when successfully hit") || strstr( desc,"an enemy on hit")
                    || strstr( desc,"when the caster is using melee attacks") || strstr( desc,"successful melee attack")
                    || strstr( desc,"chance per hit") || strstr( desc,"you deal melee damage") || strstr( desc,"your melee attacks")
                    || strstr( desc,"chance per attack") || strstr( desc,"damage with your sword") || strstr( desc,"on a successful hit")
                    || strstr( desc,"takes damage") || strstr( desc,"when damaging an enemy in melee") || strstr( desc,"on a hit")
                    || strstr( desc,"on successful melee or ranged attack") ||  strstr( desc,"when ranged or melee damage is dealt")
                    || strstr( desc,"damaging melee attacks") || strstr( desc,"attackers when hit") || strstr( desc,"on a melee swing")
                    || strstr( desc,"on melee or ranged attack") || strstr( desc,"chance on melee") ||  strstr( desc,"melee attacks has")
                    || strstr( desc,"each melee attack a chance") || strstr( desc, "a chance to deal additional")
                    || strstr( desc,"chance to get an extra attack") || strstr( desc,"giving each melee attack")
                    || strstr( desc,"each strike has") || strstr( desc,"chance on hit") || strstr( desc,"with a melee weapon")
                    || strstr( desc,"damage to melee attackers") || strstr( desc,"into flame, causing an additional")
                    || strstr( desc,"damage on every attack") || strstr( desc,"your melee and ranged attacks")
                    || strstr( desc, "gives your melee") || strstr( desc, "granting each melee")))
                    pr |= PROC_ON_MELEE_ATTACK;

                if((pr & PROC_ON_MELEE_ATTACK_VICTIM) == 0 && (strstr( desc,"attackers when hit") || strstr( desc,"strike you with a melee attack")
                    || strstr( desc,"enemy strikes the caster") || strstr( desc,"strikes you with a melee attack")
                    || strstr( desc,"enemy that strikes you in melee") || strstr( desc,"the next melee attack on the caster")
                    || strstr( desc,"when struck in melee combat") ||  strstr( desc,"the next melee attack against the caster")
                    || strstr( desc,"damage to attacker on hit") || strstr( desc,"melee and ranged attacks against you")
                    || strstr( desc,"when struck in combat") || strstr( desc,"striking melee or ranged attackers")
                    || strstr( desc,"strikes the caster") || strstr( desc,"each melee or ranged damage hit against the priest")
                    || strstr( desc,"hit by a melee or ranged attack") || strstr( desc,"when struck in combat")
                    || strstr( desc,"that strikes a party member") || strstr( desc,"when hit by a melee attack")
                    || strstr( desc,"ranged and melee attacks to deal") || strstr( desc,"striking melee or ranged attackers")
                    || strstr( desc,"damage to attackers when hit") || strstr( desc,"striking melee attackers")
                    || strstr( desc,"striking melee attackers")))
                    pr |= PROC_ON_MELEE_ATTACK_VICTIM;

                if((pr & PROC_ON_CAST_SPELL) == 0 && (strstr( desc,"target casts a spell") || strstr( desc,"your harmful spells land")
                    || strstr( desc, "any damage spell hits a target") || strstr( desc,"gives your finishing moves")
                    || strstr( desc,"gives your sinister strike, backstab, gouge and shiv") || strstr( desc,"chance on spell hit")
                    || strstr( desc,"your shadow word: pain, mind flay and vampiric touch spells also cause the target")
                    || strstr( desc,"corruption, curse of agony, siphon life and seed of corruption spells also cause")
                    || strstr( desc,"chance on spell hit") || strstr( desc,"your spell casts have") || strstr( desc,"chance on spell cast")
                    || strstr( desc,"your spell casts have") || strstr( desc,"your Fire damage spell hits")
                    || strstr( desc,"pain, mind flay and vampiric touch spells also cause")
                    || strstr( desc,"next offensive ability") || strstr( desc,"on successful spellcast")
                    || strstr( desc,"shadow damage spells have") || strstr( desc,"your next offensive ability")))
                    pr |= PROC_ON_CAST_SPELL;

                if((pr & PROC_ON_ANY_DAMAGE_VICTIM) == 0 && (strstr( desc,"any damage caused") || strstr( desc,"when caster takes damage") || strstr( desc,"damage on hit")
                    || strstr( desc,"after being hit by any damaging attack") || strstr( desc,"whenever the caster takes damage")
                    || strstr( desc, "damaging attack is taken") || strstr( desc,"a spell, melee or ranged attack hits the caster")
                    || strstr( desc,"whenever damage is dealt to you") || strstr( desc, "damage when hit")))
                    pr |= PROC_ON_ANY_DAMAGE_VICTIM;

                if((pr & PROC_ON_RANGED_ATTACK_VICTIM) == 0 && (strstr( desc,"each melee or ranged damage hit against the priest")
                    || strstr( desc,"melee and ranged attacks against you") || strstr( desc,"striking melee or ranged attackers")
                    || strstr( desc,"hit by a melee or ranged attack") || strstr( desc,"striking melee or ranged attackers")
                    || strstr( desc,"ranged and melee attacks to deal")))
                    pr |= PROC_ON_RANGED_ATTACK_VICTIM;

                if((pr & PROC_ON_CRIT_ATTACK) == 0 && (strstr( desc,"landing a melee critical strike") || strstr( desc,"your critical strikes") || strstr( desc,"critical hit")
                    || strstr( desc, "melee critical strike") || strstr( desc,"after dealing a critical strike")
                    || strstr( desc,"dealing a critical strike from a weapon swing, spell, or ability")
                    || strstr( desc,"after getting a critical strike")))
                    pr |= PROC_ON_CRIT_ATTACK;

                if((pr & PROC_ON_RANGED_ATTACK) == 0 && (strstr( desc,"on successful melee or ranged attack") ||  strstr( desc,"when ranged or melee damage is dealt")
                    || strstr( desc,"on melee or ranged attack") || strstr( desc,"damage on every attack")
                    || strstr( desc,"your melee and ranged attacks") || strstr( desc,"whenever you deal ranged damage")))
                    pr |= PROC_ON_RANGED_ATTACK;

                if((pr & PROC_ON_SPELL_HIT_VICTIM) == 0 && (strstr( desc,"any successful spell cast against the priest") || strstr( desc,"chance to reflect Fire spells")
                    || strstr( desc, "struck by a Stun or Immobilize")))
                    pr |= PROC_ON_SPELL_HIT_VICTIM;

                if((pr & PROC_ON_SPELL_CRIT_HIT_VICTIM) == 0 && (strstr( desc,"your spell criticals have") || strstr( desc, "getting a critical effect from")
                    || strstr( desc,"spell criticals against you")))
                    pr |= PROC_ON_SPELL_CRIT_HIT_VICTIM;

                if(strstr( desc,"dealing a critical strike from a weapon swing, spell, or ability")
                    || strstr( desc,"your spell criticals have"))
                    pr2 |= PROC_ON_SPELL_CRIT_HIT;

                if( strstr( desc,"hunter takes on the aspects of a hawk") || strstr( desc,"hunter takes on the aspects of a dragonhawk"))
                {
                    if((pr & PROC_ON_RANGED_ATTACK) == 0)
                        pr |= PROC_ON_RANGED_ATTACK;
                    pr2 |= PROC_TARGET_SELF;
                }

                if((pr & PROC_ON_CRIT_HIT_VICTIM) == 0 && (strstr( desc,"victim of a critical strike") || strstr( desc,"after being struck by a melee or ranged critical hit")
                    || strstr( desc, "victim of a melee or ranged critical strike") || strstr( desc,"victim of a critical melee strike")))
                    pr |= PROC_ON_CRIT_HIT_VICTIM;

                if((pr & PROC_ON_RANGED_CRIT_ATTACK) == 0 && (strstr( desc,"your ranged criticals")))
                    pr |= PROC_ON_RANGED_CRIT_ATTACK;

                if((pr & PROC_ON_GAIN_EXPIERIENCE) == 0 && (strstr( desc, "experience or honor")))
                    pr |= PROC_ON_GAIN_EXPIERIENCE;

                if((pr & PROC_ON_SPELL_LAND_VICTIM) == 0 && (strstr( desc,"after being hit with a shadow or fire spell")))
                    pr |= PROC_ON_SPELL_LAND_VICTIM;

                if((pr & PROC_ON_AUTO_SHOT_HIT) == 0 && (strstr( desc,"successful auto shot attacks")))
                    pr |= PROC_ON_AUTO_SHOT_HIT;

                if( strstr( desc, "gives your"))
                {
                    if(strstr( desc,"chance to daze the target") && (pr & PROC_ON_CAST_SPELL) == 0)
                        pr |= PROC_ON_CAST_SPELL;
                    else // We should find that specific spell (or group) on what we will trigger
                        if((pr & PROC_ON_CAST_SPECIFIC_SPELL) == 0)
                            pr |= PROC_ON_CAST_SPECIFIC_SPELL;
                }

                if((pr & PROC_ON_CRIT_ATTACK) == 0 && (strstr( desc, "chance to add an additional combo") && strstr(desc, "critical")))
                    pr |= PROC_ON_CRIT_ATTACK;
                else if((pr & PROC_ON_CAST_SPELL) == 0 && (strstr( desc, "chance to add an additional combo")))
                    pr |= PROC_ON_CAST_SPELL;

                if(strstr( desc,"being able to resurrect"))
                    pr2 |= PROC_ON_DIE;

                if( strstr( desc,"after dodging their attack"))
                {
                    pr2 |= PROC_ON_DODGE_VICTIM;
                    if( strstr( desc,"add a combo point"))
                        pr2 |= PROC_TARGET_SELF;
                }

                if( strstr( sp->Name, "Bloodthirst"))
                {
                    pr |= PROC_ON_MELEE_ATTACK;
                    pr2 |= PROC_TARGET_SELF;
                }

                if(strstr( desc,"fully resisting") || strstr( desc,"fully resist"))
                    pr2 |= PROC_ON_FULL_RESIST;

                if(strstr( desc,"each attack blocked") || strstr( desc,"target blocks a melee attack")
                    || strstr( desc,"when a block occurs"))
                    pr2 |= PROC_ON_BLOCK_VICTIM;

                if(strstr( desc,"shadow bolt critical strikes increase shadow damage")
                    || strstr( desc,"after getting a critical effect from your")
                    || strstr( desc,"on spell critical hit") || strstr( desc,"spell critical strikes"))
                    pr2 |= PROC_ON_SPELL_CRIT_HIT;

            }//end "if procspellaura"

            // Aura 109 fix
            if(sp->EffectApplyAuraName[y] == SPELL_AURA_ADD_TARGET_TRIGGER)
            {
                sp->EffectApplyAuraName[y] = SPELL_AURA_PROC_TRIGGER_SPELL;
                pr = PROC_ON_CAST_SPELL;
            }
        }//end "if aura"
    }//end "for each effect"
    sp->procFlags = pr;
    sp->procflags2 = pr2;
}

uint32 GetTriggerSpellFromDescription(std::string delimiter, std::string desc)
{
    std::string token;

    // find the delimiter.
    size_t i = desc.find(delimiter);
    if (i == string::npos)
        return 0;

    // find the first character of the spell id.
    size_t j = desc.find_first_not_of(delimiter, i);
    if (j == string::npos)
        return 0;

    // find the end of the spell id.
    size_t k = desc.find("s1", j);
    if (k == string::npos)
        return 0;

    // get our token
    token = desc.substr(j, k - j);

    // convert to int
    uint32 id = 0;
    std::istringstream iss(token);
    iss >> id;

    // and return our spell id
    return id;
}

SpellEntry* CreateDummySpell(uint32 id)
{
    std::string name = "Dummy Trigger";
    SpellEntry* sp = new SpellEntry();
    memset(sp, 0, sizeof(SpellEntry*));
    sp->Id = id;
    sp->Attributes = 384;
    sp->AttributesEx = 268435456;
    sp->Flags3 = 4;
    sp->Name = ((char*)name.c_str());
    sp->Rank = ((char*)" ");
    sp->Description = ((char*)" ");
    sp->CastingTimeIndex = 1;
    sp->procChance = 75;
    sp->rangeIndex = 13;
    sp->spellLevel = 0;
    sp->EquippedItemClass = uint32(-1);
    sp->Effect[0] = SPELL_EFFECT_DUMMY;
    sp->EffectImplicitTargetA[0] = 25;
    sp->NameHash = crc32((const unsigned char*)name.c_str(), (unsigned int)name.length());
    dbcSpell.SetRow(id, sp);
    sWorld.dummyspells.push_back(sp);
    return sp;
}

uint32 GetSpellClass(SpellEntry *sp)
{
    switch(sp->skilline)
    {
    case SKILL_ARMS:
    case SKILL_FURY:
    case SKILL_PROTECTION:
        return WARRIOR;
    case SKILL_HOLY2:
    case SKILL_PROTECTION2:
    case SKILL_RETRIBUTION:
        return PALADIN;
    case SKILL_BEAST_MASTERY:
    case SKILL_SURVIVAL:
    case SKILL_MARKSMANSHIP:
        return HUNTER;
    case SKILL_ASSASSINATION:
    case SKILL_COMBAT:
    case SKILL_SUBTLETY:
        return ROGUE;
    case SKILL_DISCIPLINE:
    case SKILL_HOLY:
    case SKILL_SHADOW:
        return PRIEST;
    case SKILL_ENHANCEMENT:
    case SKILL_RESTORATION:
    case SKILL_ELEMENTAL_COMBAT:
        return SHAMAN;
    case SKILL_FROST:
    case SKILL_FIRE:
    case SKILL_ARCANE:
        return MAGE;
    case SKILL_AFFLICTION:
    case SKILL_DEMONOLOGY:
    case SKILL_DESTRUCTION:
        return WARLOCK;
    case SKILL_RESTORATION2:
    case SKILL_BALANCE:
    case SKILL_FERAL_COMBAT:
        return DRUID;
    case SKILL_DK_FROST:
    case SKILL_UNHOLY:
    case SKILL_BLOOD:
        return DEATHKNIGHT;
    }

    return 0;
}

// Generates SpellNameHashes.h
void GenerateNameHashesFile()
{
    const uint32 fieldSize = 81;
    const char* prefix = "SPELL_HASH_";
    uint32 prefixLen = uint32(strlen(prefix));
    FILE * f = fopen("SpellNameHashes.h", "w");
    char spaces[fieldSize], namearray[fieldSize];
    strcpy(namearray, prefix);
    char* name = &namearray[prefixLen];
    for(int i = 0;i < fieldSize-1; ++i)
        spaces[i] = ' ';

    std::set<uint32> namehashes;
    std::map<std::string, uint8> namearrays;

    spaces[fieldSize-1] = 0;
    uint32 nameTextLen = 0, nameLen = 0;
    for (uint32 i = 0; i < dbcSpell.GetNumRows(); ++i)
    {
        SpellEntry *sp = dbcSpell.LookupRow(i);
        if(sp == NULL)
            continue;

        strncpy(name, sp->Name, fieldSize-prefixLen-4); // Cut it to fit in field size
        name[fieldSize-prefixLen-2] = 0; // in case nametext is too long and strncpy didn't copy the null
        nameLen = (unsigned int)strlen(name);
        for(uint32 i = 0;i<nameLen;++i)
        {
            if(name[i] >= 'a' && name[i] <= 'z')
                name[i] = toupper(name[i]);
            else if(!(name[i] >= '0' && name[i] <= '9') &&
                !(name[i] >= 'A' && name[i] <= 'Z'))
                name[i] = '_';
        }

        if(namehashes.find(sp->NameHash) != namehashes.end())
            continue; // Skip namehashes we've already done.
        namehashes.insert(sp->NameHash);
        std::string name_array(namearray);
        if(namearrays.find(name_array) != namearrays.end())
        {
            uint8 count = namearrays[name_array]+1;
            name_array.append(format("_%u", count).c_str());
            namearrays[name_array] = count;
        }
        else namearrays.insert(std::make_pair(name_array, 1));

        int32 numSpaces = fieldSize-name_array.length()-1;
        if(numSpaces < 0)
            fprintf(f, "WTF");

        spaces[numSpaces] = 0;
        fprintf(f, "#define %s%s0x%08X\n", name_array.c_str(), spaces, sp->NameHash);
        spaces[numSpaces] = ' ';
    }
    fclose(f);
}

// Copies effect number 'fromEffect' in 'fromSpell' to effect number 'toEffect' in 'toSpell'
void CopyEffect(SpellEntry *fromSpell, uint8 fromEffect, SpellEntry *toSpell, uint8 toEffect)
{
    if(!fromSpell || !toSpell || fromEffect > 2 || toEffect > 2)
        return;

    uint32 *from = fromSpell->Effect;
    uint32 *to = toSpell->Effect;
    // Copy 20 values starting at Effect
    for(uint8 index = 0;index < 20;index++)
    {
        to[index * 3 + toEffect] = from[index * 3 + fromEffect];
    }
}

void SetSingleSpellDefaults(SpellEntry *sp)
{
    /// 4.0.6 field copies
    //SpellAuraOptionsEntry
    sp->maxstack = 1;
    sp->procChance = 0;
    sp->procCharges = 0;
    sp->procFlags = 0;
    //SpellAuraRestrictionsEntry
    sp->CasterAuraState = 0;
    sp->TargetAuraState = 0;
    sp->CasterAuraStateNot = 0;
    sp->TargetAuraStateNot = 0;
    // SpellCastingRequirementsEntry
    sp->FacingCasterFlags = 0;
    sp->AreaGroupId = 0;
    sp->RequiresSpellFocus = 0;
    // SpellCategoriesEntry
    sp->Category = 0;
    sp->Spell_Dmg_Type = 0;
    sp->DispelType = 0;
    sp->MechanicsType = 0;
    sp->PreventionType = 0;
    sp->StartRecoveryCategory = 0;
    // SpellClassOptionsEntry
    sp->SpellGroupType[0] = 0;
    sp->SpellGroupType[1] = 0;
    sp->SpellGroupType[2] = 0;
    sp->SpellFamilyName = 0;
    // SpellCooldownsEntry
    sp->CategoryRecoveryTime = 0;
    sp->RecoveryTime = 0;
    sp->StartRecoveryTime = 0;
    // SpellEffectEntry
    for(uint8 i = 0; i < 3; i++)
    {
        sp->Effect[i] = 0;
        sp->EffectValueMultiplier[i] = 0;
        sp->EffectApplyAuraName[i] = 0;
        sp->EffectAmplitude[i] = 0;
        sp->EffectBasePoints[i] = 0;
        sp->EffectBonusCoefficient[i] = 0;
        sp->EffectDamageMultiplier[i] = 0;
        sp->EffectChainTarget[i] = 0;
        sp->EffectDieSides[i] = 0;
        sp->EffectItemType[i] = 0;
        sp->EffectMechanic[i] = 0;
        sp->EffectMiscValue[i] = 0;
        sp->EffectMiscValueB[i] = 0;
        sp->EffectPointsPerComboPoint[i] = 0;
        sp->EffectRadiusIndex[i] = 0;
        sp->EffectRealPointsPerLevel[i] = 0;
        sp->EffectSpellClassMask[i][0] = 0;
        sp->EffectSpellClassMask[i][1] = 0;
        sp->EffectSpellClassMask[i][2] = 0;
        sp->EffectTriggerSpell[i] = 0;
        sp->EffectImplicitTargetA[i] = 0;
        sp->EffectImplicitTargetB[i] = 0;
    }
    // SpellEquippedItemsEntry
    sp->EquippedItemClass = 0;
    sp->RequiredItemFlags = 0;
    sp->EquippedItemSubClass = 0;
    // SpellInterruptsEntry
    sp->AuraInterruptFlags = 0;
    sp->ChannelInterruptFlags = 0;
    sp->InterruptFlags = 0;
    // SpellLevelsEntry
    sp->baseLevel = 0;
    sp->maxLevel = 0;
    sp->spellLevel = 0;
    // SpellPowerEntry
    sp->ManaCost = 0;
    sp->ManaCostPerlevel = 0;
    sp->ManaCostPercentage = 0;
    sp->ManaPerSecond = 0;
    // SpellReagentsEntry
    for(uint8 i = 0; i < 8; i++)
    {
        sp->Reagent[i] = 0;
        sp->ReagentCount[i] = 0;
    }
    // SpellShapeshiftEntry
    sp->RequiredShapeShift = 0;
    sp->ShapeshiftExclude = 0;
    // SpellTargetRestrictionsEntry
    sp->MaxTargets = 0;
    sp->MaxTargetLevel = 0;
    sp->TargetCreatureType = 0;
    sp->Targets = 0;
    // SpellTotemsEntry
    sp->Totem[0] = 0;
    sp->Totem[1] = 0;
    sp->RequiredShapeShift = 0;

    /// Custom defaults
    sp->forced_creature_target = 0;
    sp->AdditionalAura = 0;
    sp->poison_type = 0;
    sp->self_cast_only = false;
    sp->Unique = false;
    sp->apply_on_shapeshift_change = false;
    sp->always_apply = false;
    sp->proc_interval = 0; //trigger at each event
    sp->ProcsPerMinute = 0;
    sp->c_is_flags = 0;
    sp->isAOE = false;
    sp->SP_coef_override = 0;
    sp->AP_coef_override = 0;
    sp->RAP_coef_override = 0;
    sp->auraimmune_flag = 0;
    sp->AllowBackAttack = false;
    sp->procflags2 = 0.0f;
    sp->cone_width = 0.0f;
    sp->area_aura_update_interval = 2000;
    sp->trnr_req_clsmsk = 0;

    float radius = 0.0f;
    if(sp->EffectRadiusIndex[0] != 0)
        radius = ::GetDBCRadius(dbcSpellRadius.LookupEntry(sp->EffectRadiusIndex[0]));
    if( sp->EffectRadiusIndex[1] != 0 )
        radius = std::max(radius,::GetDBCRadius(dbcSpellRadius.LookupEntry(sp->EffectRadiusIndex[1])));
    if( sp->EffectRadiusIndex[2] != 0 )
        radius = std::max(::GetDBCRadius(dbcSpellRadius.LookupEntry(sp->EffectRadiusIndex[2])),radius);
    radius = std::max(GetDBCMaxRange(dbcSpellRange.LookupEntry(sp->rangeIndex)), radius);

    sp->base_range_or_radius = radius;
    sp->base_range_or_radius_sqr = radius*radius;

    radius = 0.0f;
    if(sp->EffectRadiusIndex[0] != 0)
        radius = ::GetDBCFriendlyRadius(dbcSpellRadius.LookupEntry(sp->EffectRadiusIndex[0]));
    if( sp->EffectRadiusIndex[1] != 0 )
        radius = std::max(radius, ::GetDBCFriendlyRadius(dbcSpellRadius.LookupEntry(sp->EffectRadiusIndex[1])));
    if( sp->EffectRadiusIndex[2] != 0 )
        radius = std::max(::GetDBCFriendlyRadius(dbcSpellRadius.LookupEntry(sp->EffectRadiusIndex[2])), radius);
    radius = std::max(GetDBCFriendlyMaxRange(dbcSpellRange.LookupEntry(sp->rangeIndex)), radius);

    sp->base_range_or_radius_friendly = radius;
    sp->base_range_or_radius_sqr_friendly = radius*radius;

    // hash the name
    //!!!!!!! representing all strings on 32 bits is dangerous. There is a chance to get same hash for a lot of strings ;)
    sp->NameHash = crc32((const unsigned char*)sp->Name, (unsigned int)strlen(sp->Name)); //need these set before we start processing spells
}

extern uint32 implicitTargetFlags[150];

void SetupSpellTargets()
{
    memset(implicitTargetFlags, SPELL_TARGET_NONE, sizeof(uint32)*150);

    implicitTargetFlags[0] = (SPELL_TARGET_REQUIRE_ITEM | SPELL_TARGET_REQUIRE_GAMEOBJECT);
    implicitTargetFlags[1] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[3] = (SPELL_TARGET_REQUIRE_FRIENDLY);
    implicitTargetFlags[4] = (SPELL_TARGET_AREA_SELF | SPELL_TARGET_REQUIRE_FRIENDLY);
    implicitTargetFlags[5] = (SPELL_TARGET_OBJECT_CURPET);
    implicitTargetFlags[6] = (SPELL_TARGET_REQUIRE_ATTACKABLE);
    implicitTargetFlags[7] = (SPELL_TARGET_OBJECT_SCRIPTED);
    implicitTargetFlags[8] = (SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE);
    implicitTargetFlags[15] = (SPELL_TARGET_AREA_SELF | SPELL_TARGET_REQUIRE_ATTACKABLE);
    implicitTargetFlags[16] = (SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE);
    //implicitTargetFlags[17] = (SPELL_TARGET_AREA);
    implicitTargetFlags[18] = (SPELL_TARGET_AREA_SELF | SPELL_TARGET_NO_OBJECT);
    implicitTargetFlags[20] = (SPELL_TARGET_AREA_PARTY);
    implicitTargetFlags[21] = (SPELL_TARGET_REQUIRE_FRIENDLY);
    implicitTargetFlags[22] = (SPELL_TARGET_AREA_SELF);
    implicitTargetFlags[23] = (SPELL_TARGET_REQUIRE_GAMEOBJECT);
    implicitTargetFlags[24] = (SPELL_TARGET_AREA_CONE | SPELL_TARGET_REQUIRE_ATTACKABLE);
    implicitTargetFlags[25] = (SPELL_TARGET_ANY_OBJECT);
    implicitTargetFlags[26] = (SPELL_TARGET_REQUIRE_GAMEOBJECT | SPELL_TARGET_REQUIRE_ITEM);
    implicitTargetFlags[27] = (SPELL_TARGET_OBJECT_PETOWNER);
    implicitTargetFlags[28] = (SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE);
    implicitTargetFlags[29] = (SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_SELF);
    implicitTargetFlags[30] = (SPELL_TARGET_REQUIRE_FRIENDLY);
    implicitTargetFlags[31] = (SPELL_TARGET_REQUIRE_FRIENDLY | SPELL_TARGET_AREA);
    //implicitTargetFlags[32] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[33] = (SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_PARTY);
    implicitTargetFlags[35] = (SPELL_TARGET_AREA_PARTY);
    implicitTargetFlags[36] = (SPELL_TARGET_OBJECT_SCRIPTED);
    implicitTargetFlags[37] = (SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID);
    implicitTargetFlags[39] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[40] = (SPELL_TARGET_OBJECT_SCRIPTED);
    implicitTargetFlags[41] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[42] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[43] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[44] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[45] = (SPELL_TARGET_AREA_CHAIN | SPELL_TARGET_REQUIRE_FRIENDLY);
    implicitTargetFlags[46] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[47] = (SPELL_TARGET_AREA_SELF | SPELL_TARGET_NO_OBJECT); //dont fill target map for this (fucks up some spell visuals)
    implicitTargetFlags[48] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[49] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[50] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[52] = (SPELL_TARGET_AREA);
    implicitTargetFlags[53] = (SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_REQUIRE_ATTACKABLE);
    implicitTargetFlags[54] = (SPELL_TARGET_AREA_CONE | SPELL_TARGET_REQUIRE_ATTACKABLE);
    implicitTargetFlags[56] = (SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_RAID); //used by commanding shout] = (targets raid now
    implicitTargetFlags[57] = (SPELL_TARGET_REQUIRE_FRIENDLY | SPELL_TARGET_AREA_PARTY);
    implicitTargetFlags[61] = (SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_RAID | SPELL_TARGET_OBJECT_TARCLASS | SPELL_TARGET_REQUIRE_FRIENDLY);
    implicitTargetFlags[63] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[64] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[65] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[66] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[67] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[69] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[72] = (SPELL_TARGET_AREA_RANDOM);
    implicitTargetFlags[73] = (SPELL_TARGET_OBJECT_SELF);
    implicitTargetFlags[76] = (SPELL_TARGET_REQUIRE_ATTACKABLE);
    implicitTargetFlags[77] = (SPELL_TARGET_REQUIRE_ATTACKABLE);
    implicitTargetFlags[86] = (SPELL_TARGET_AREA_RANDOM);
    implicitTargetFlags[87] = (SPELL_TARGET_AREA);
    implicitTargetFlags[89] = (SPELL_TARGET_AREA);
    implicitTargetFlags[90] = (SPELL_TARGET_OBJECT_CURCRITTER);
    implicitTargetFlags[104] = (SPELL_TARGET_REQUIRE_ATTACKABLE | SPELL_TARGET_AREA_CONE);
    implicitTargetFlags[149] = SPELL_TARGET_NOT_IMPLEMENTED;
}

void PoolSpellData()
{
    for (uint32 i = 0; i < dbcSpellEffect.GetNumRows(); ++i)
    {
        SpellEffectEntry* effectEntry = dbcSpellEffect.LookupRow(i);
        if(effectEntry == NULL)
            continue; // No effect

        SpellEntry* spellInfo = dbcSpell.LookupEntry(effectEntry->EffectSpellId);
        if(spellInfo == NULL)
            continue; // Missing Spell?

        spellInfo->Effect[effectEntry->EffectIndex] = effectEntry->Effect;
        spellInfo->EffectValueMultiplier[effectEntry->EffectIndex] = effectEntry->EffectValueMultiplier;
        spellInfo->EffectApplyAuraName[effectEntry->EffectIndex] = effectEntry->EffectApplyAuraName;
        spellInfo->EffectAmplitude[effectEntry->EffectIndex] = effectEntry->EffectAmplitude;
        spellInfo->EffectBasePoints[effectEntry->EffectIndex] = effectEntry->EffectBasePoints;
        spellInfo->EffectDamageMultiplier[effectEntry->EffectIndex] = effectEntry->EffectDamageMultiplier;
        spellInfo->EffectChainTarget[effectEntry->EffectIndex] = effectEntry->EffectChainTarget;
        spellInfo->EffectDieSides[effectEntry->EffectIndex] = effectEntry->EffectDieSides;
        spellInfo->EffectItemType[effectEntry->EffectIndex] = effectEntry->EffectItemType;
        spellInfo->EffectMechanic[effectEntry->EffectIndex] = effectEntry->EffectMechanic;
        spellInfo->EffectMiscValue[effectEntry->EffectIndex] = effectEntry->EffectMiscValue;
        spellInfo->EffectMiscValueB[effectEntry->EffectIndex] = effectEntry->EffectMiscValueB;
        spellInfo->EffectPointsPerComboPoint[effectEntry->EffectIndex] = effectEntry->EffectPointsPerComboPoint;
        spellInfo->EffectRadiusIndex[effectEntry->EffectIndex] = effectEntry->EffectRadiusIndex;
        spellInfo->EffectRealPointsPerLevel[effectEntry->EffectIndex] = effectEntry->EffectRealPointsPerLevel;
        spellInfo->EffectSpellClassMask[0][effectEntry->EffectIndex] = effectEntry->EffectSpellClassMask[0];
        spellInfo->EffectSpellClassMask[1][effectEntry->EffectIndex] = effectEntry->EffectSpellClassMask[1];
        spellInfo->EffectSpellClassMask[2][effectEntry->EffectIndex] = effectEntry->EffectSpellClassMask[2];
        spellInfo->EffectTriggerSpell[effectEntry->EffectIndex] = effectEntry->EffectTriggerSpell;
        spellInfo->EffectImplicitTargetA[effectEntry->EffectIndex] = effectEntry->EffectImplicitTargetA;
        spellInfo->EffectImplicitTargetB[effectEntry->EffectIndex] = effectEntry->EffectImplicitTargetB;
    }

    for (uint32 i = 0; i < dbcSpell.GetNumRows(); ++i)
    {
        SpellEntry* spellInfo = dbcSpell.LookupRow(i);
        if(spellInfo == NULL)
            continue;

        //SpellAuraOptionsEntry
        SpellAuraOptionsEntry* AuraOptions = dbcSpellAuraOptions.LookupEntry(spellInfo->SpellAuraOptionsId);
        if(AuraOptions != NULL)
        {
            spellInfo->maxstack = AuraOptions->StackAmount;
            spellInfo->procChance = AuraOptions->procChance;
            spellInfo->procCharges = AuraOptions->procCharges;
            spellInfo->procFlags = AuraOptions->procFlags;
        }

        //SpellAuraRestrictionsEntry
        SpellAuraRestrictionsEntry* AuraRestrict = dbcSpellAuraRestrictions.LookupEntry(spellInfo->SpellAuraRestrictionsId);
        if(AuraRestrict != NULL)
        {
            spellInfo->CasterAuraState = AuraRestrict->CasterAuraState;
            spellInfo->TargetAuraState = AuraRestrict->TargetAuraState;
        }

        //SpellCategoriesEntry
        SpellCategoriesEntry* sCategory = dbcSpellCategories.LookupEntry(spellInfo->SpellCategoriesId);
        if(sCategory != NULL)
        {
            spellInfo->Category = sCategory->Category;
            spellInfo->Spell_Dmg_Type = sCategory->DmgClass;
            spellInfo->DispelType = sCategory->Dispel;
            spellInfo->MechanicsType = sCategory->Mechanic;
            spellInfo->PreventionType = sCategory->PreventionType;
            spellInfo->StartRecoveryCategory = sCategory->StartRecoveryCategory;
        }

        //SpellCastingRequirementsEntry
        SpellCastingRequirementsEntry* CastRequirements = dbcSpellCastingRequirements.LookupEntry(spellInfo->SpellCastingRequirementsId);
        if(CastRequirements != NULL)
        {
            spellInfo->FacingCasterFlags = CastRequirements->FacingCasterFlags;
            spellInfo->AreaGroupId = CastRequirements->AreaGroupId;
            spellInfo->RequiresSpellFocus = CastRequirements->RequiresSpellFocus;
        }

        //SpellClassOptionsEntry
        SpellClassOptionsEntry* ClassOptions = dbcSpellClassOptions.LookupEntry(spellInfo->SpellClassOptionsId);
        if(ClassOptions != NULL)
        {
            for(int i = 0; i < 3; ++i)
                spellInfo->SpellGroupType[i] = ClassOptions->SpellFamilyFlags[i];

            spellInfo->SpellFamilyName = ClassOptions->SpellFamilyName;
        }

        //SpellCooldownsEntry
        SpellCooldownsEntry* sCooldowns = dbcSpellCooldowns.LookupEntry(spellInfo->SpellCooldownsId);
        if(sCooldowns != NULL)
        {
            spellInfo->CategoryRecoveryTime = sCooldowns->CategoryRecoveryTime;
            spellInfo->RecoveryTime = sCooldowns->RecoveryTime;
            spellInfo->StartRecoveryTime = sCooldowns->StartRecoveryTime;
        }

        //SpellEffectEntry

        //SpellEquippedItemsEntry
        SpellEquippedItemsEntry* EquippedItems = dbcSpellEquippedItems.LookupEntry(spellInfo->SpellEquippedItemsId);
        if(EquippedItems != NULL)
        {
            spellInfo->EquippedItemClass = EquippedItems->EquippedItemClass;
            spellInfo->RequiredItemFlags = EquippedItems->EquippedItemInventoryTypeMask;
            spellInfo->EquippedItemSubClass = EquippedItems->EquippedItemSubClassMask;
        }

        //SpellInterruptsEntry
        SpellInterruptsEntry* sInterrupts = dbcSpellInterrupts.LookupEntry(spellInfo->SpellInterruptsId);
        if(sInterrupts != NULL)
        {
            spellInfo->AuraInterruptFlags = sInterrupts->AuraInterruptFlags;
            spellInfo->ChannelInterruptFlags = sInterrupts->ChannelInterruptFlags;
            spellInfo->InterruptFlags = sInterrupts->InterruptFlags;
        }

        //SpellLevelsEntry
        SpellLevelsEntry* sLevel = dbcSpellLevels.LookupEntry(spellInfo->SpellLevelsId);
        if(sLevel != NULL)
        {
            spellInfo->baseLevel = sLevel->baseLevel;
            spellInfo->maxLevel = sLevel->maxLevel;
            spellInfo->spellLevel = sLevel->spellLevel;
        }

        //SpellPowerEntry
        SpellPowerEntry* sPower = dbcSpellPower.LookupEntry(spellInfo->SpellPowerId);
        if(sPower != NULL)
        {
            spellInfo->ManaCost = sPower->manaCost;
            spellInfo->ManaCostPerlevel = sPower->manaCostPerlevel;
            spellInfo->ManaCostPercentage = sPower->manaCostPercentage;
            spellInfo->ManaPerSecond = sPower->manaPerSecond;
        }

        //SpellReagentsEntry
        SpellReagentsEntry* sReagent = dbcSpellReagents.LookupEntry(spellInfo->SpellReagentsId);
        if(sReagent != NULL)
        {
            for(int i = 0; i < 8; ++i)
            {
                spellInfo->Reagent[i] = sReagent->Reagent[i];
                spellInfo->ReagentCount[i] = sReagent->ReagentCount[i];
            }
        }

        //SpellShapeshiftEntry
        SpellShapeshiftEntry* shapeShift = dbcSpellShapeshift.LookupEntry(spellInfo->SpellShapeshiftId);
        if(shapeShift != NULL)
        {
            spellInfo->RequiredShapeShift = shapeShift->Stances;
            spellInfo->ShapeshiftExclude = shapeShift->StancesNot;
        }

        //SpellTargetRestrictionsEntry
        SpellTargetRestrictionsEntry* TargetRestrict = dbcSpellTargetRestrictions.LookupEntry(spellInfo->SpellTargetRestrictionsId);
        if(TargetRestrict != NULL)
        {
            spellInfo->MaxTargets = TargetRestrict->MaxAffectedTargets;
            spellInfo->MaxTargetLevel = TargetRestrict->MaxTargetLevel;
            spellInfo->TargetCreatureType = TargetRestrict->TargetCreatureType;
            spellInfo->Targets = TargetRestrict->Targets;
        }

        //SpellTotemsEntry
        SpellTotemsEntry* Totems = dbcSpellTotems.LookupEntry(spellInfo->SpellTotemsId);
        if(Totems != NULL)
        {
            for(int i = 0; i < 2; ++i)
                spellInfo->Totem[i] = Totems->Totem[i];
        }
    }
}

