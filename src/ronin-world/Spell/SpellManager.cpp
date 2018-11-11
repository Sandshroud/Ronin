/***
* Demonstrike Core
*/

#include "StdAfx.h"

initialiseSingleton( SpellManager );

SpellManager::SpellManager()
{

}

SpellManager::~SpellManager()
{

}

void SpellManager::Initialize()
{

}

void SpellManager::ParseSpellDBC()
{
    sLog.Notice("SpellManager", "Filling %u spell default values...", dbcSpell.GetNumRows());

    SpellEntry *sp = NULL;
    for(uint32 x = 0; x < dbcSpell.GetNumRows(); x++)
    {
        if((sp = dbcSpell.LookupRow(x)) == NULL)
            continue;

        SetSingleSpellDefaults(sp);
    }

    sLog.Notice("SpellManager", "Parsing %u skill lines...", dbcSkillLine.GetNumRows());
    SkillLineEntry *skillLine = NULL;
    for(uint32 x = 0; x < dbcSkillLine.GetNumRows(); x++)
    {
        if((skillLine = dbcSkillLine.LookupRow(x)) == NULL)
            continue;

        m_skillLinesByCategory[skillLine->categoryId].push_back(skillLine);
        m_skillLinesByName.insert(std::make_pair(skillLine->name, skillLine));

    }

    std::set<uint32> processedSkillLines;
    sLog.Notice("SpellManager", "Parsing %u spell skills...", dbcSkillLineSpell.GetNumRows());
    for(uint32 i = 0; i < dbcSkillLineSpell.GetNumRows(); i++)
    {
        if (SkillLineAbilityEntry *skillLineAbility = dbcSkillLineSpell.LookupRow(i))
        {
            if(SpellEntry *sp = dbcSpell.LookupEntry(skillLineAbility->spell))
            {
                sp->SpellSkillLine = skillLineAbility->skilline;
                if((skillLine = dbcSkillLine.LookupEntry(sp->SpellSkillLine)) && processedSkillLines.find(sp->SpellSkillLine) == processedSkillLines.end())
                {
                    switch(skillLine->categoryId)
                    {
                    case SKILL_TYPE_CLASS:
                        {
                            if(skillLineAbility->classMask)
                            {
                                uint8 _class = 0;
                                for(uint8 i = 1; i < CLASS_MAX; ++i)
                                {
                                    if(skillLineAbility->classMask & (1<<(i-1)))
                                    {
                                        _class = i;
                                        break;
                                    }
                                }

                                if(_class)
                                {
                                    m_skillLineClasses.insert(std::make_pair(sp->SpellSkillLine, _class));
                                    processedSkillLines.insert(sp->SpellSkillLine);
                                }
                            }
                        }break;
                    case SKILL_TYPE_SECONDARY:
                        {

                        }break;
                    case SKILL_TYPE_PROFESSION:
                        {

                        }break;
                    }
                }
            }

            m_skillLineEntriesBySkillLine[skillLineAbility->skilline].push_back(skillLineAbility->Id);
        }
    }
}

void SpellManager::LoadSpellFixes()
{
    sLog.Notice("SpellManager", "Applying spell fixes...");

    SpellEntry *sp = NULL;
    for(uint32 x = 0; x < dbcSpell.GetNumRows(); x++)
    {
        if((sp = dbcSpell.LookupRow(x)) == NULL)
            continue;

        // parse rank text
        if( !sscanf( sp->Rank, "Rank %d", (unsigned int*)&sp->RankNumber) )
        {
            const char* ranktext = sp->Rank;
            //stupid spell ranking problem
            if( strstr( ranktext, "Apprentice"))
                sp->RankNumber = 1;
            else if( strstr( ranktext, "Journeyman"))
                sp->RankNumber = 2;
            else if( strstr( ranktext, "Expert"))
                sp->RankNumber = 3;
            else if( strstr( ranktext, "Artisan"))
                sp->RankNumber = 4;
            else if( strstr( ranktext, "Master"))
                sp->RankNumber = 5;
            else if( strstr( ranktext, "Grandmaster"))
                sp->RankNumber = 6;
            else sp->RankNumber = 0;
        }

        if(sp->HasEffect(SPELL_EFFECT_LANGUAGE))
        {
            char buff[55];
            // Language has to be parsed, since skills use a colon
            sscanf_s(sp->Name, "Language %s", buff, 55);
            // Check our list of language categories to see if we can find one with our language in the name
            for(std::vector<SkillLineEntry*>::iterator itr = m_skillLinesByCategory[SKILL_TYPE_LANGUAGE].begin(); itr != m_skillLinesByCategory[SKILL_TYPE_LANGUAGE].end(); itr++)
            {
                if(RONIN_UTIL::FindXinYString(buff, (*itr)->name))
                {
                    sp->SpellSkillLine = (*itr)->id;
                    break;
                }
            }
        }
        else if(sp->HasEffect(SPELL_EFFECT_WEAPON))
        {
            char buff[55];
            // Stupid fucking one handed spells are different than their skill names
            if(sscanf_s(sp->Name, "One-Handed %s", buff, 55) == 0)
                sprintf(buff, sp->Name); // If we're not one-handed then just use our name
            // Check our category of weapon skills to see if we can find a name that matches
            for(std::vector<SkillLineEntry*>::iterator itr = m_skillLinesByCategory[SKILL_TYPE_WEAPON].begin(); itr != m_skillLinesByCategory[SKILL_TYPE_WEAPON].end(); itr++)
            {
                if(strcmp(buff, (*itr)->name) == 0)
                {
                    sp->SpellSkillLine = (*itr)->id;
                    break;
                }
            }
        }
        else if(sp->HasEffect(SPELL_EFFECT_DUAL_WIELD))
        {
            std::map<std::string, SkillLineEntry*>::iterator itr;
            if((itr = m_skillLinesByName.find("Dual Wield")) != m_skillLinesByName.end())
                sp->SpellSkillLine = itr->second->id;
        }

        // Apply spell fixes.
        ApplySingleSpellFixes(sp);
        ApplyCoeffSpellFixes(sp);
        SetProcFlags(sp);
        ProcessSpellInformation(sp);
    }

    sLog.Notice("SpellManager", "Processing %u dummy spells...", m_dummySpells.size());
    for(std::set<uint32>::iterator itr = m_dummySpells.begin(); itr != m_dummySpells.end(); itr++)
    {   // Crow: Create the dummy spell, and apply fixs :D
        SetSingleSpellDefaults(sp = _CreateDummySpell(*itr));
        ApplySingleSpellFixes(sp);
        ApplyCoeffSpellFixes(sp);
        SetProcFlags(sp);
        ProcessSpellInformation(sp);
    }

    sLog.Notice("SpellManager", "Processing %u shapeshift forms...", dbcSpellShapeshiftForm.GetNumRows());
    for(uint32 i = 0; i < dbcSpellShapeshiftForm.GetNumRows(); i++)
    {
        if(SpellShapeshiftFormEntry *ssEntry = dbcSpellShapeshiftForm.LookupRow(i))
        {
            ssEntry->forcedPowerType = POWER_TYPE_MANA;
            switch(ssEntry->id)
            {
            case FORM_GHOUL:
            case FORM_CAT:
                ssEntry->forcedPowerType = POWER_TYPE_ENERGY;
                break;
            case FORM_BEAR:
            case FORM_DIREBEAR:
                ssEntry->forcedPowerType = POWER_TYPE_RAGE;
                break;
            }
        }
    }

    // Register class specific fixes
    _RegisterWarriorFixes();
    _RegisterPaladinFixes();
    _RegisterHunterFixes();
    _RegisterRogueFixes();
    _RegisterPriestFixes();
    _RegisterDeathKnightFixes();
    _RegisterShamanFixes();
    _RegisterMageFixes();
    _RegisterWarlockFixes();
    _RegisterDruidFixes();

    _RegisterHomeboundData();
    _RegisterTeleportData();
    _RegisterTranslocateData();

    // Register zone specific fixes
    _RegisterTirisfalGladesScripts();
    _RegisterSilvermoonCityScripts();
    _RegisterDalaranScripts();
}

bool validateSpellFamily(SpellEntry *sp, uint8 &outClass)
{
    switch(sp->SpellFamilyName)
    {
    case 3: outClass = MAGE; break;
    case 4: outClass = WARRIOR; break;
    case 5: outClass = WARLOCK; break;
    case 6: outClass = PRIEST; break;
    case 7: outClass = DRUID; break;
    case 8: outClass = ROGUE; break;
    case 9: outClass = HUNTER; break;
    case 10: outClass = PALADIN; break;
    case 11: outClass = SHAMAN; break;
    case 15: outClass = DEATHKNIGHT; break;
    default: return false;
    }
    return true;
}

void SpellManager::PoolSpellData()
{
    sLog.Notice("SpellManager", "Filling spell pool data...");

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
        spellInfo->EffectBonusCoefficient[effectEntry->EffectIndex] = effectEntry->EffectBonusCoefficient;
        spellInfo->EffectDamageMultiplier[effectEntry->EffectIndex] = effectEntry->EffectDamageMultiplier;
        spellInfo->EffectChainTarget[effectEntry->EffectIndex] = effectEntry->EffectChainTarget;
        spellInfo->EffectDieSides[effectEntry->EffectIndex] = effectEntry->EffectDieSides;
        spellInfo->EffectItemType[effectEntry->EffectIndex] = effectEntry->EffectItemType;
        spellInfo->EffectMechanic[effectEntry->EffectIndex] = effectEntry->EffectMechanic;
        spellInfo->EffectMiscValue[effectEntry->EffectIndex] = effectEntry->EffectMiscValue;
        spellInfo->EffectMiscValueB[effectEntry->EffectIndex] = effectEntry->EffectMiscValueB;
        spellInfo->EffectPointsPerComboPoint[effectEntry->EffectIndex] = effectEntry->EffectPointsPerComboPoint;
        spellInfo->EffectRadiusIndex[effectEntry->EffectIndex] = effectEntry->EffectRadiusIndex;
        spellInfo->EffectRadiusMaxIndex[effectEntry->EffectIndex] = effectEntry->EffectRadiusMaxIndex;
        spellInfo->EffectRealPointsPerLevel[effectEntry->EffectIndex] = effectEntry->EffectRealPointsPerLevel;
        spellInfo->EffectSpellClassMask[effectEntry->EffectIndex][0] = effectEntry->EffectSpellClassMask[0];
        spellInfo->EffectSpellClassMask[effectEntry->EffectIndex][1] = effectEntry->EffectSpellClassMask[1];
        spellInfo->EffectSpellClassMask[effectEntry->EffectIndex][2] = effectEntry->EffectSpellClassMask[2];
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
        if(SpellAuraOptionsEntry* AuraOptions = dbcSpellAuraOptions.LookupEntry(spellInfo->SpellAuraOptionsId))
        {
            spellInfo->maxstack = AuraOptions->StackAmount;
            spellInfo->procChance = AuraOptions->procChance;
            spellInfo->procCharges = AuraOptions->procCharges;
            spellInfo->procFlags = AuraOptions->procFlags;
        }

        //SpellAuraRestrictionsEntry
        if(SpellAuraRestrictionsEntry* AuraRestrict = dbcSpellAuraRestrictions.LookupEntry(spellInfo->SpellAuraRestrictionsId))
        {
            spellInfo->CasterAuraState = AuraRestrict->CasterAuraState;
            spellInfo->TargetAuraState = AuraRestrict->TargetAuraState;
        }

        //SpellCategoriesEntry
        if(SpellCategoriesEntry* sCategory = dbcSpellCategories.LookupEntry(spellInfo->SpellCategoriesId))
        {
            spellInfo->Category = sCategory->Category;
            spellInfo->Spell_Dmg_Type = sCategory->DmgClass;
            spellInfo->DispelType = sCategory->Dispel;
            spellInfo->MechanicsType = sCategory->Mechanic;
            spellInfo->PreventionType = sCategory->PreventionType;
            spellInfo->StartRecoveryCategory = sCategory->StartRecoveryCategory;
        }

        if(SpellCastTimeEntry *sCastTime = dbcSpellCastTime.LookupEntry(spellInfo->CastingTimeIndex))
        {
            spellInfo->castTime = sCastTime->castTime;
            spellInfo->castTimePerLevel = sCastTime->castTimePerLevel;
            spellInfo->baseCastTime = sCastTime->baseCastTime;
        }

        //SpellCastingRequirementsEntry
        if(SpellCastingRequirementsEntry* CastRequirements = dbcSpellCastingRequirements.LookupEntry(spellInfo->SpellCastingRequirementsId))
        {
            spellInfo->FacingCasterFlags = CastRequirements->FacingCasterFlags;
            spellInfo->AreaGroupId = CastRequirements->AreaGroupId;
            spellInfo->RequiresSpellFocus = CastRequirements->RequiresSpellFocus;
        }

        //SpellClassOptionsEntry
        if(SpellClassOptionsEntry* ClassOptions = dbcSpellClassOptions.LookupEntry(spellInfo->SpellClassOptionsId))
        {
            for(uint8 c = 0; c < 3; ++c)
                spellInfo->SpellGroupType[c] = ClassOptions->SpellFamilyFlags[c];

            spellInfo->SpellFamilyName = ClassOptions->SpellFamilyName;
            spellInfo->spellBookDescription = ClassOptions->Description;
        }

        //SpellCooldownsEntry
        if(SpellCooldownsEntry* sCooldowns = dbcSpellCooldowns.LookupEntry(spellInfo->SpellCooldownsId))
        {
            spellInfo->CategoryRecoveryTime = sCooldowns->CategoryRecoveryTime;
            spellInfo->RecoveryTime = sCooldowns->RecoveryTime;
            spellInfo->StartRecoveryTime = sCooldowns->StartRecoveryTime;
        }

        //SpellDurationEntry
        if(SpellDurationEntry *sDuration = dbcSpellDuration.LookupEntry(spellInfo->DurationIndex))
        {
            spellInfo->Duration[0] = sDuration->Duration1;
            spellInfo->Duration[1] = sDuration->Duration2;
            spellInfo->Duration[2] = sDuration->Duration3;
        }

        //SpellEffectEntry

        //SpellEquippedItemsEntry
        if(SpellEquippedItemsEntry* EquippedItems = dbcSpellEquippedItems.LookupEntry(spellInfo->SpellEquippedItemsId))
        {
            spellInfo->EquippedItemClass = EquippedItems->EquippedItemClass;
            spellInfo->EquippedItemInventoryTypeMask = EquippedItems->EquippedItemInventoryTypeMask;
            spellInfo->EquippedItemSubClassMask = EquippedItems->EquippedItemSubClassMask;
        }

        //SpellInterruptsEntry
        if(SpellInterruptsEntry* sInterrupts = dbcSpellInterrupts.LookupEntry(spellInfo->SpellInterruptsId))
        {
            spellInfo->AuraInterruptFlags = sInterrupts->AuraInterruptFlags;
            spellInfo->ChannelInterruptFlags = sInterrupts->ChannelInterruptFlags;
            spellInfo->InterruptFlags = sInterrupts->InterruptFlags;
        }

        //SpellLevelsEntry
        if(SpellLevelsEntry* sLevel = dbcSpellLevels.LookupEntry(spellInfo->SpellLevelsId))
        {
            spellInfo->spellLevelBaseLevel = sLevel->baseLevel;
            spellInfo->spellLevelMaxLevel = sLevel->maxLevel;
            spellInfo->spellLevelSpellLevel = sLevel->spellLevel;
        }

        //SpellPowerEntry
        if(SpellPowerEntry* sPower = dbcSpellPower.LookupEntry(spellInfo->SpellPowerId))
        {
            spellInfo->ManaCost = sPower->manaCost;
            spellInfo->ManaCostPerlevel = sPower->manaCostPerlevel;
            spellInfo->ManaCostPercentage = sPower->manaCostPercentage;
            spellInfo->ManaPerSecond = sPower->manaPerSecond;
        }

        //SpellRangeEntry
        if(SpellRangeEntry *sRange = dbcSpellRange.LookupEntry(spellInfo->rangeIndex))
        {
            spellInfo->minRange[0] = sRange->minRangeHostile;
            spellInfo->minRange[1] = sRange->minRangeFriend;
            spellInfo->maxRange[0] = sRange->maxRangeHostile;
            spellInfo->maxRange[1] = sRange->maxRangeFriend;
        }

        //SpellRadiusEntry
        for(uint8 i = 0; i < 3; i++)
        {
            bool hasMinRadius = true;
            SpellRadiusEntry *sRadius = dbcSpellRadius.LookupEntry(spellInfo->EffectRadiusIndex[i]);
            if(sRadius == NULL && (sRadius = dbcSpellRadius.LookupEntry(spellInfo->EffectRadiusMaxIndex[i])) == NULL)
                continue;

            spellInfo->radiusEnemy[i] = sRadius->radiusMin;
            spellInfo->radiusFriendly[i] = sRadius->radiusMax;
        }

        //SpellReagentsEntry
        if(SpellReagentsEntry* sReagent = dbcSpellReagents.LookupEntry(spellInfo->SpellReagentsId))
        {
            for(uint8 c = 0; c < 8; ++c)
            {
                spellInfo->Reagent[c] = sReagent->Reagent[c];
                spellInfo->ReagentCount[c] = sReagent->ReagentCount[c];
            }
        }

        //SpellReagentsEntry
        if(SpellRuneCostEntry* sRuneCost = dbcSpellRuneCost.LookupEntry(spellInfo->SpellRuneCostID))
        {
            spellInfo->runeCost[0] = sRuneCost->bloodRuneCost;
            spellInfo->runeCost[1] = sRuneCost->frostRuneCost;
            spellInfo->runeCost[2] = sRuneCost->unholyRuneCost;
            spellInfo->runicGain = sRuneCost->runicPowerGain;
        }

        //SpellScalingEntry
        if(SpellScalingEntry *scalingEntry = dbcSpellScaling.LookupEntry(spellInfo->SpellScalingId))
        {
            spellInfo->castTimeMin = scalingEntry->castTimeMin;
            spellInfo->castTimeMax = scalingEntry->castTimeMax;
            spellInfo->castScalingMaxLevel = scalingEntry->castScalingMaxLevel;
            spellInfo->playerClass = scalingEntry->playerClass;
            for(uint8 c = 0; c < 3; c++)
                for(uint8 s = 0; s < 3; s++)
                    spellInfo->coeff[c][s] = scalingEntry->coeff[c][s];
            spellInfo->coefBase = scalingEntry->CoefBase;
            spellInfo->coefLevelBase = scalingEntry->CoefLevelBase;
        }

        //SpellShapeshiftEntry
        if(SpellShapeshiftEntry* shapeShift = dbcSpellShapeshift.LookupEntry(spellInfo->SpellShapeshiftId))
        {
            spellInfo->RequiredShapeShift = shapeShift->Stances[0];
            spellInfo->RequiredShapeShift |= (((uint64)shapeShift->Stances[1])<<32);
            spellInfo->ShapeshiftExclude = shapeShift->StancesNot[0];
            spellInfo->ShapeshiftExclude |= (((uint64)shapeShift->StancesNot[1])<<32);
        }

        //SpellTargetRestrictionsEntry
        if(SpellTargetRestrictionsEntry* TargetRestrict = dbcSpellTargetRestrictions.LookupEntry(spellInfo->SpellTargetRestrictionsId))
        {
            spellInfo->MaxTargets = TargetRestrict->MaxAffectedTargets;
            spellInfo->MaxTargetLevel = TargetRestrict->MaxTargetLevel;
            spellInfo->TargetCreatureType = TargetRestrict->TargetCreatureType;
            spellInfo->Targets = TargetRestrict->Targets;
        }

        //SpellTotemsEntry
        if(SpellTotemsEntry* Totems = dbcSpellTotems.LookupEntry(spellInfo->SpellTotemsId))
        {
            for(uint8 c = 0; c < 2; ++c)
                spellInfo->Totem[c] = Totems->Totem[c];
        }

        validateSpellFamily(spellInfo, spellInfo->Class);
    }

    sLog.Notice("SpellManager", "Setting target flags...");
    SetupSpellTargets();

    sLog.Notice("SpellManager", "Initializing spell effect class...");
    SpellEffectClass::InitializeSpellEffectClass();
    Aura::InitializeAuraHandlerClass();

    dbcSpellAuraOptions.Unload();
    dbcSpellAuraRestrictions.Unload();
    dbcSpellCastingRequirements.Unload();
    dbcSpellCastTime.Unload();
    dbcSpellCategories.Unload();
    dbcSpellClassOptions.Unload();
    dbcSpellCooldowns.Unload();
    dbcSpellDifficulty.Unload();
    dbcSpellDuration.Unload();
    dbcSpellEffect.Unload();
    dbcSpellEquippedItems.Unload();
    dbcSpellInterrupts.Unload();
    dbcSpellLevels.Unload();
    dbcSpellPower.Unload();
    dbcSpellRadius.Unload();
    dbcSpellReagents.Unload();
    dbcSpellRuneCost.Unload();
    dbcSpellScaling.Unload();
    dbcSpellShapeshift.Unload();
    dbcSpellTargetRestrictions.Unload();
    dbcSpellTotems.Unload();
    sLog.Notice("SpellManager", "Finished unloading pooled DBCs...");
}

bool SpellManager::CanTriggerInstantKillEffect(Unit *unitCaster, Unit *unitTarget, SpellEntry *sp)
{
    if(unitCaster->IsPlayer() && unitTarget->IsPlayer())
        return false; // Don't allow players to instant kill each other

    // These should be handled at cast checks
    if(sp->NameHash == SPELL_HASH_ENCAPSULATE_VOIDWALKER)
        if( unitTarget->GetEntry() != 16975 )
            return false;
    if(sp->NameHash == SPELL_HASH_KILL_LEGION_HOLD_INFERNALS)
        if( unitTarget->GetEntry() != 21316 )
            return false;
    if(sp->NameHash == SPELL_HASH_DESTROY_SPIRIT)
        if( unitTarget->GetEntry() != 23109 )
            return false;
    if(sp->NameHash == SPELL_HASH_KARAZHAN___CHESS___KILL_CHEST_BUNNY)
        if( unitTarget->GetEntry() != 25213 )
            return false;

    // Don't handle these spells
    if(sp->NameHash == SPELL_HASH_POWER_BURN)
        return false;
    if(sp->NameHash == SPELL_HASH_MANA_BURN)
        return false;
    return true;
}

ItemPrototype *SpellManager::GetCreateItemForSpell(Player *target, SpellEntry *info, uint32 effIndex, int32 amount, uint32 &count)
{
    ItemPrototype *proto = NULL;
    if(info->Id == 3286) // Create Hearthstone
        proto = sItemMgr.LookupEntry(6948);
    else if(uint32 itemId = info->EffectItemType[effIndex])
        proto = sItemMgr.LookupEntry(itemId);

    if(proto != NULL)
    {
        if(proto->Class != ITEM_CLASS_CONSUMABLE || info->SpellFamilyName != SPELLFAMILY_MAGE)
            count = amount;
        else if(target->getLevel() >= info->spellLevelBaseLevel)
        {
            count = ((target->getLevel() - (info->spellLevelBaseLevel-1))*amount);
            if((proto->MaxCount > 0) && count > proto->MaxCount)
                count = proto->MaxCount;
        } else count = 1;
    }

    if(info->SpellSkillLine)
    {
        switch(info->SpellSkillLine)
        {
        case SKILL_ALCHEMY:
            //TODO: Specializations
            break;
        }

        //TODO: Discoveries
    }

    if(count == 0)
        count = amount;
    if(proto && proto->Unique && count > proto->Unique)
        count = proto->Unique;

    return proto;
}

bool SpellManager::HandleTakePower(SpellEffectClass *spell, Unit *unitCaster, int32 powerField, int32 &cost, bool &result)
{
    SpellEntry *sp = spell->GetSpellProto();
    if(sp->NameHash == SPELL_HASH_ZEALOTRY)
        cost = 0;

    return false;
}

bool SpellManager::ModifyEffectAmount(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    SpellEntry *sp = spell->GetSpellProto();
    std::pair<uint32, uint32> spEff = std::make_pair(sp->Id, effIndex);
    if(m_amountModifierHandlers.find(spEff) != m_amountModifierHandlers.end())
        return (*m_amountModifierHandlers.at(spEff))(spell, effIndex, caster, target, amount);
    return false;
}

bool SpellManager::HandleDummyEffect(SpellEffectClass *spell, uint32 effIndex, Unit *caster, WorldObject *target, int32 &amount)
{
    SpellEntry *sp = spell->GetSpellProto();
    std::pair<uint32, uint32> spEff = std::make_pair(sp->Id, effIndex);
    if(m_dummyEffectHandlers.find(spEff) != m_dummyEffectHandlers.end())
        return (*m_dummyEffectHandlers.at(spEff))(spell, effIndex, caster, target, amount);
    return false;
}

bool SpellManager::TriggerScriptedEffect(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 modAmt)
{
    SpellEntry *sp = spell->GetSpellProto();
    std::pair<uint32, uint32> spEff = std::make_pair(sp->Id, effIndex);
    if(m_scriptedEffectHandlers.find(spEff) != m_scriptedEffectHandlers.end())
        return (*m_scriptedEffectHandlers.at(spEff))(spell, effIndex, target, modAmt);
    return false;
}

bool SpellManager::FetchSpellCoordinates(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 modAmt, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    SpellEntry *sp = spell->GetSpellProto();
    std::pair<uint32, uint32> spEff = std::make_pair(sp->Id, effIndex);
    if(m_teleportEffectHandlers.find(spEff) != m_teleportEffectHandlers.end())
        return (*m_teleportEffectHandlers.at(spEff))(spell, effIndex, target, modAmt, mapId, X, Y, Z, O);
    return false;
}

bool SpellManager::CanCastCreatureCombatSpell(SpellEntry *sp, Creature *ctr)
{
    if(m_canCastCCSTriggers.find(sp->Id) != m_canCastCCSTriggers.end())
        return (*m_canCastCCSTriggers.at(sp->Id))(sp, ctr);
    return true;
}

bool SpellManager::GenerateCreatureCombatSpellTargets(SpellEntry *sp, Creature *ctr, SpellCastTargets *targets, WoWGuid attackGuid)
{
    if(m_genCCSTargetTriggers.find(sp->Id) != m_genCCSTargetTriggers.end())
        return (*m_genCCSTargetTriggers.at(sp->Id))(sp, ctr, targets, attackGuid);
    return false;
}

bool SpellManager::IsAuraApplicable(Unit *unit, SpellEntry *spell)
{
    if(spell->RequiredShapeShift && !( ((uint32)1 << (unit->GetShapeShift()-1)) & spell->RequiredShapeShift ))
        return false;
    if(spell->isSpellAppliedOnShapeshift() && !unit->HasAurasOfNameHashWithCaster(spell->TargetNameHash, unit))
        return false;
    if(spell->isSpellNotActiveInBattleArena() && unit->IsInWorld() && unit->GetMapInstance()->IsBattleArena())
        return false;

    if( spell->AreaGroupId > 0 )
    {
        bool areaFound = false;
        AreaGroupEntry *GroupEntry = dbcAreaGroup.LookupEntry( spell->AreaGroupId );
        for( uint8 i = 0; i < 7; i++ )
        {
            if( GroupEntry->AreaId[i] != 0 && GroupEntry->AreaId[i] == unit->GetAreaId() )
            {
                areaFound = true;
                break;
            }
        }

        // Aura doesn't work in this area
        if(areaFound == false)
            return false;
    }

    return true;
}

uint8 SpellManager::GetClassForSkillLine(uint32 skillLine)
{
    std::map<uint32, uint8>::iterator itr;
    if((itr = m_skillLineClasses.find(skillLine)) != m_skillLineClasses.end())
        return itr->second;
    return 0;
}

std::map<uint8, uint32> Spell::m_implicitTargetFlags;

void SpellManager::SetupSpellTargets()
{
    Spell::m_implicitTargetFlags.insert(std::make_pair(0, (SPELL_TARGET_REQUIRE_ITEM | SPELL_TARGET_REQUIRE_GAMEOBJECT)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(1, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(3, SPELL_TARGET_REQUIRE_FRIENDLY));
    Spell::m_implicitTargetFlags.insert(std::make_pair(4, (SPELL_TARGET_AREA_SELF | SPELL_TARGET_REQUIRE_FRIENDLY)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(5, SPELL_TARGET_OBJECT_CURPET));
    Spell::m_implicitTargetFlags.insert(std::make_pair(6, SPELL_TARGET_REQUIRE_ATTACKABLE));
    Spell::m_implicitTargetFlags.insert(std::make_pair(7, SPELL_TARGET_OBJECT_SCRIPTED));
    Spell::m_implicitTargetFlags.insert(std::make_pair(8, (SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(15, (SPELL_TARGET_AREA_SELF | SPELL_TARGET_REQUIRE_ATTACKABLE)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(16, (SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE)));
    //Spell::m_implicitTargetFlags.insert(std::make_pair(17, SPELL_TARGET_AREA));
    Spell::m_implicitTargetFlags.insert(std::make_pair(18, (SPELL_TARGET_AREA_SELF | SPELL_TARGET_NO_OBJECT)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(20, SPELL_TARGET_AREA_PARTY));
    Spell::m_implicitTargetFlags.insert(std::make_pair(21, SPELL_TARGET_REQUIRE_FRIENDLY));
    Spell::m_implicitTargetFlags.insert(std::make_pair(22, SPELL_TARGET_AREA_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(23, SPELL_TARGET_REQUIRE_GAMEOBJECT));
    Spell::m_implicitTargetFlags.insert(std::make_pair(24, (SPELL_TARGET_AREA_CONE | SPELL_TARGET_REQUIRE_ATTACKABLE)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(25, SPELL_TARGET_ANY_OBJECT));
    Spell::m_implicitTargetFlags.insert(std::make_pair(26, (SPELL_TARGET_REQUIRE_GAMEOBJECT | SPELL_TARGET_REQUIRE_ITEM)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(27, SPELL_TARGET_OBJECT_PETOWNER));
    Spell::m_implicitTargetFlags.insert(std::make_pair(28, (SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(29, (SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_SELF)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(30, SPELL_TARGET_REQUIRE_FRIENDLY));
    Spell::m_implicitTargetFlags.insert(std::make_pair(31, (SPELL_TARGET_REQUIRE_FRIENDLY | SPELL_TARGET_AREA)));
    //Spell::m_implicitTargetFlags.insert(std::make_pair(32, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(33, (SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_PARTY)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(35, SPELL_TARGET_AREA_PARTY));
    Spell::m_implicitTargetFlags.insert(std::make_pair(36, SPELL_TARGET_OBJECT_SCRIPTED));
    Spell::m_implicitTargetFlags.insert(std::make_pair(37, (SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(39, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(40, SPELL_TARGET_OBJECT_SCRIPTED));
    Spell::m_implicitTargetFlags.insert(std::make_pair(41, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(42, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(43, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(44, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(45, (SPELL_TARGET_AREA_CHAIN | SPELL_TARGET_REQUIRE_FRIENDLY)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(46, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(47, (SPELL_TARGET_AREA_SELF | SPELL_TARGET_NO_OBJECT))); //dont fill target map for this (fucks up some spell visuals)
    Spell::m_implicitTargetFlags.insert(std::make_pair(48, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(49, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(50, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(52, SPELL_TARGET_AREA));
    Spell::m_implicitTargetFlags.insert(std::make_pair(53, (SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_REQUIRE_ATTACKABLE)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(54, (SPELL_TARGET_AREA_CONE | SPELL_TARGET_REQUIRE_ATTACKABLE)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(56, (SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_RAID))); //used by commanding shout, targets raid now
    Spell::m_implicitTargetFlags.insert(std::make_pair(57, (SPELL_TARGET_REQUIRE_FRIENDLY | SPELL_TARGET_AREA_PARTY)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(61, (SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_RAID | SPELL_TARGET_OBJECT_TARCLASS | SPELL_TARGET_REQUIRE_FRIENDLY)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(63, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(64, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(65, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(66, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(67, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(69, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(72, SPELL_TARGET_AREA_RANDOM));
    Spell::m_implicitTargetFlags.insert(std::make_pair(73, SPELL_TARGET_OBJECT_SELF));
    Spell::m_implicitTargetFlags.insert(std::make_pair(76, SPELL_TARGET_REQUIRE_ATTACKABLE));
    Spell::m_implicitTargetFlags.insert(std::make_pair(77, SPELL_TARGET_REQUIRE_ATTACKABLE));
    Spell::m_implicitTargetFlags.insert(std::make_pair(86, SPELL_TARGET_AREA_RANDOM));
    Spell::m_implicitTargetFlags.insert(std::make_pair(87, SPELL_TARGET_AREA));
    Spell::m_implicitTargetFlags.insert(std::make_pair(89, SPELL_TARGET_AREA));
    Spell::m_implicitTargetFlags.insert(std::make_pair(90, SPELL_TARGET_OBJECT_CURCRITTER));
    Spell::m_implicitTargetFlags.insert(std::make_pair(104, (SPELL_TARGET_REQUIRE_ATTACKABLE | SPELL_TARGET_AREA_CONE)));
    Spell::m_implicitTargetFlags.insert(std::make_pair(149, SPELL_TARGET_NOT_IMPLEMENTED));
}

SpellEntry* SpellManager::_CreateDummySpell(uint32 id)
{
    static std::string name = "Dummy Trigger";
    SpellEntry* sp = new SpellEntry();
    memset(sp, 0, sizeof(SpellEntry*));
    sp->Id = id;
    sp->Attributes[0] = 384;
    sp->Attributes[1] = 268435456;
    sp->Attributes[2] = 4;
    sp->Name = ((char*)name.c_str());
    sp->Rank = ((char*)" ");
    sp->Description = ((char*)" ");
    sp->CastingTimeIndex = 1;
    sp->procChance = 75;
    sp->rangeIndex = 13;
    sp->EquippedItemClass = uint32(-1);
    sp->NameHash = crc32((const unsigned char*)name.c_str(), (unsigned int)name.length());
    if(!dbcSpell.SetEntry(id, sp))
    {
        delete sp;
        return NULL;
    }
    SetSingleSpellDefaults(sp);
    ApplySingleSpellFixes(sp);
    ApplyCoeffSpellFixes(sp);
    SetProcFlags(sp);
    ProcessSpellInformation(sp);
    sp->Effect[0] = SPELL_EFFECT_DUMMY;
    sp->EffectImplicitTargetA[0] = 25;

    m_dummySpells.insert(id);
    return sp;
}

SpellEntry* SpellManager::_CreateScriptedEffectSpell(uint32 id)
{
    static std::string name = "Dummy Script Trigger";
    SpellEntry* sp = new SpellEntry();
    memset(sp, 0, sizeof(SpellEntry*));
    sp->Id = id;
    sp->Attributes[0] = 384;
    sp->Attributes[1] = 268435456;
    sp->Attributes[2] = 4;
    sp->Name = ((char*)name.c_str());
    sp->Rank = ((char*)" ");
    sp->Description = ((char*)" ");
    sp->CastingTimeIndex = 1;
    sp->procChance = 75;
    sp->rangeIndex = 13;
    sp->EquippedItemClass = uint32(-1);
    sp->NameHash = crc32((const unsigned char*)name.c_str(), (unsigned int)name.length());
    if(!dbcSpell.SetEntry(id, sp))
    {
        delete sp;
        return NULL;
    }
    SetSingleSpellDefaults(sp);
    ApplySingleSpellFixes(sp);
    ApplyCoeffSpellFixes(sp);
    SetProcFlags(sp);
    ProcessSpellInformation(sp);
    sp->Effect[0] = SPELL_EFFECT_SCRIPT_EFFECT;
    sp->EffectImplicitTargetA[0] = 25;

    m_dummySpells.insert(id);
    return sp;
}

void SpellManager::SetSingleSpellDefaults(SpellEntry *sp)
{
    //SpellAuraOptionsEntry
    sp->maxstack = 0;
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
    // SpellCastTimeEntry
    sp->castTime = 0;
    sp->castTimePerLevel = 0;
    sp->baseCastTime = 0;
    // SpellClassOptionsEntry
    sp->SpellGroupType[0] = 0;
    sp->SpellGroupType[1] = 0;
    sp->SpellGroupType[2] = 0;
    sp->SpellFamilyName = 0;
    sp->spellBookDescription = "";
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
    sp->EquippedItemClass = -1;
    sp->EquippedItemInventoryTypeMask = 0;
    sp->EquippedItemSubClassMask = 0;
    // SpellInterruptsEntry
    sp->AuraInterruptFlags = 0;
    sp->ChannelInterruptFlags = 0;
    sp->InterruptFlags = 0;
    // SpellLevelsEntry
    sp->spellLevelBaseLevel = 0;
    sp->spellLevelMaxLevel = 0;
    sp->spellLevelSpellLevel = 0;
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
    sp->runeCost[0] = 0;
    sp->runeCost[1] = 0;
    sp->runeCost[2] = 0;
    sp->runicGain = 0;
    // SpellScalingEntry
    sp->castTimeMin = 0;
    sp->castTimeMax = 0;
    sp->castScalingMaxLevel = 0;
    sp->playerClass = 0;
    for(uint8 i = 0; i < 3; i++)
        for(uint8 s = 0; s < 3; s++)
            sp->coeff[i][s] = 0.f;
    sp->coefBase = 0.f;
    sp->coefLevelBase = 0;
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
    // SpellRangeEntry
    sp->minRange[0] = 0.0f;
    sp->minRange[1] = 0.0f;
    sp->maxRange[0] = 0.0f;
    sp->maxRange[1] = 0.0f;
    // SpellRadius
    for(uint8 i = 0; i < 3; i++)
        sp->radiusEnemy[i] = sp->radiusFriendly[i] = 0.f;
    /// Spell Pointers
    sp->Duration[0] = 0;
    sp->Duration[1] = 0;
    sp->Duration[2] = 0;

    /// Defaults
    sp->Class = sp->buffIndex = 0;
    sp->School = sp->SchoolMask ? RONIN_UTIL::FirstBitValue(sp->SchoolMask) : 0;
    sp->NameHash = crc32((const unsigned char*)sp->Name, (unsigned int)strlen(sp->Name)); //need these set before we start processing spells
    sp->RankNumber = sp->GeneratedThreat = sp->SpellSkillLine = 0;
    sp->CustomAttributes[0] = sp->CustomAttributes[1] = 0;
    sp->TargetNameHash = 0;
    sp->isUnique = sp->always_apply = false;

    // parse rank text
    if( !sscanf( sp->Rank, "Rank %d", (unsigned int*)&sp->RankNumber) )
    {
        const char* ranktext = sp->Rank;
        //stupid spell ranking problem
        if( strstr( ranktext, "Apprentice"))
            sp->RankNumber = 1;
        else if( strstr( ranktext, "Journeyman"))
            sp->RankNumber = 2;
        else if( strstr( ranktext, "Expert"))
            sp->RankNumber = 3;
        else if( strstr( ranktext, "Artisan"))
            sp->RankNumber = 4;
        else if( strstr( ranktext, "Master"))
            sp->RankNumber = 5;
        else if( strstr( ranktext, "Grandmaster"))
            sp->RankNumber = 6;
        else sp->RankNumber = 0;
    }
}

void SpellManager::ApplySingleSpellFixes(SpellEntry *sp)
{

}

void SpellManager::ApplyCoeffSpellFixes(SpellEntry *sp)
{

}

void SpellManager::SetProcFlags(SpellEntry *sp)
{

}

void SpellManager::ProcessSpellInformation(SpellEntry *sp)
{
    if( sp->EquippedItemClass == 2 && sp->EquippedItemSubClassMask & (0x10|0x100|0x40000) ) // 4 + 8 + 262144 ( becomes item classes 2, 3 and 18 which correspond to bow, gun and crossbow respectively)
        sp->CustomAttributes[1] |= 0x10;

    if(strlen(sp->Name))
    {
        // Shape shift forms have associative passives that are applied on shapeshift
        std::string lowercaseName = RONIN_UTIL::TOLOWER_RETURN(sp->Name);
        if(RONIN_UTIL::FindXinYString("form (passive)", lowercaseName))
        {
            // Add flag for shapeshift requirement
            sp->CustomAttributes[0] |= 0x08;
            // Set our target namehash field
            std::string normalName = sp->Name;
            normalName = normalName.substr(0, normalName.size()-strlen(" (Passive)"));
            sp->TargetNameHash = crc32((const unsigned char*)normalName.c_str(), normalName.length());
        }

        // Certain spells shouldn't be shown in our spellbook so we hide them here
        if(RONIN_UTIL::FindXinYString("(old)", lowercaseName)
            || RONIN_UTIL::FindXinYString("(test)", lowercaseName)
            || RONIN_UTIL::FindXinYString("(passive)", lowercaseName)
            || RONIN_UTIL::FindXinYString("(dnd)", lowercaseName))
            sp->CustomAttributes[0] |= 0x80;
    }

    uint8 effIndex = 0x00;
    if(sp->HasEffect(SPELL_EFFECT_LANGUAGE, 0xFF, &effIndex))
        if(uint16 skillLine = sp->SpellSkillLine ? sp->SpellSkillLine : sp->EffectMiscValue[effIndex])
            m_SpellIdForLanguage[skillLine] = sp->Id;

    switch(sp->NameHash)
    {
        // Set our buff index's here
    case SPELL_HASH_BATTLE_SHOUT: if(sp->Class == WARRIOR) sp->buffIndex = BUFF_WARRIOR_BATTLE_SHOUT; break;
    case SPELL_HASH_COMMANDING_SHOUT: if(sp->Class == WARRIOR) sp->buffIndex = BUFF_WARRIOR_COMMANDING_SHOUT; break;
    case SPELL_HASH_BLESSING_OF_MIGHT: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_MIGHT; break;
    case SPELL_HASH_BLESSING_OF_KINGS: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_KINGS; break;
    case SPELL_HASH_DEVOTION_AURA: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_DEVOTION; break;
    case SPELL_HASH_RETRIBUTION_AURA: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_RETRIBUTION; break;
    case SPELL_HASH_CONCENTRATION_AURA: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_CONCENTRATION; break;
    case SPELL_HASH_RESISTANCE_AURA: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_RESISTANCE; break;
    case SPELL_HASH_CRUSADER_AURA: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_CRUSADER; break;
    case SPELL_HASH_SEAL_OF_INSIGHT: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_INSIGHT; break;
    case SPELL_HASH_SEAL_OF_JUSTICE: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_JUSTICE; break;
    case SPELL_HASH_SEAL_OF_RIGHTEOUSNESS: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_RIGHTEOUSNESS; break;
    case SPELL_HASH_SEAL_OF_TRUTH: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_TRUTH; break;
    case SPELL_HASH_HAND_OF_FREEDOM: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_FREEDOM; break;
    case SPELL_HASH_HAND_OF_PROTECTION: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_PROTECTION; break;
    case SPELL_HASH_HAND_OF_SACRIFICE: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_SACRIFICE; break;
    case SPELL_HASH_HAND_OF_SALVATION: if(sp->Class == PALADIN) sp->buffIndex = BUFF_PALADIN_SALVATION; break;
    case SPELL_HASH_BLOOD_PRESENCE: case SPELL_HASH_FROST_PRESENCE: case SPELL_HASH_UNHOLY_PRESENCE: sp->buffIndex = BUFF_DKPRESENCE; break;

        // Handle our mastery spells, can be a bit complicated but we have our top trigger and our underlying trigger to hash
    case SPELL_HASH_MASTERY:
        {
            uint8 targetClass = 0;
            for(uint8 i = WARRIOR; i < CLASS_MAX; i++)
            {
                StatSystem::TalentSpellPair talentPair = sStatSystem.GetTalentMasterySpells(i, 0x00);
                if(talentPair.first != talentPair.second)
                {
                    for(StatSystem::TalentSpellStorage::iterator itr = talentPair.first; itr != talentPair.second; itr++)
                    {
                        if(SpellEntry *talentSpell = dbcSpell.LookupEntry(itr->second))
                        {
                            if(RONIN_UTIL::FindXinYString(talentSpell->Name, sp->Description))
                            {
                                targetClass = i;
                                break;
                            }
                        }
                    }
                }
            }

            if(targetClass == 0)
            {   // We have a hardcoded list so we can just parse through it for the class, also earlier in execution for trainer list forcing
                for(uint8 i = WARRIOR; i < CLASS_MAX; i++)
                {
                    if(classMasterySpells[i-1] == sp->Id)
                    {
                        targetClass = i;
                        break;
                    }
                }

                if(targetClass == 0)
                    break;
            }

            // TODO: store for later trigger on talent spec change for base recalc
            sp->Class = targetClass;
        }break;
    }

    sp->spellType = NON_WEAPON;
    if(sp->_IsSpellMeleeSpell() || (sp->Spell_Dmg_Type == SPELL_DMG_TYPE_MELEE))
        sp->spellType = sp->reqOffHandWeapon() ? OFFHAND : MELEE;
    else if(sp->reqAmmoForSpell() || sp->reqWandForSpell() || (sp->Spell_Dmg_Type == SPELL_DMG_TYPE_RANGED))
        sp->spellType = RANGED;

    /*    if( IsTargetingStealthed( sp ) )
    sp->c_is_flags |= SPELL_FLAG_IS_TARGETINGSTEALTHED;
    if( IsCastedOnFriends(sp) )
    sp->c_is_flags |= SPELL_FLAG_CASTED_ON_FRIENDS;
    if( IsCastedOnEnemies(sp) )
    sp->c_is_flags |= SPELL_FLAG_CASTED_ON_ENEMIES;

    if( IsDamagingSpell( sp ) )
    sp->c_is_flags |= SPELL_FLAG_IS_DAMAGING;
    if( IsHealingSpell( sp ) )
    sp->c_is_flags |= SPELL_FLAG_IS_HEALING;
    if( IsFlyingSpell(sp) )
    sp->c_is_flags |= SPELL_FLAG_IS_FLYING;*/
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
        for(uint32 n = 0;n<nameLen;++n)
        {
            if(name[n] >= 'a' && name[n] <= 'z')
                name[n] = toupper(name[n]);
            else if(!(name[n] >= '0' && name[n] <= '9') &&
                !(name[n] >= 'A' && name[n] <= 'Z'))
                name[n] = '_';
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
