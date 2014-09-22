/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void PoolSpellData();
void SetSingleSpellDefaults(SpellEntry *sp);

void ApplyNormalFixes()
{
    sLog.Notice("World", "Processing %u spells, highest %u...", dbcSpell.GetNumRows(), dbcSpell.GetMaxRow());

    sLog.Notice("World", "Filling spell default values...");
    SpellEntry *sp; // We must set spell defaults before we load static pool data
    for(uint32 x = 0; x < dbcSpell.GetNumRows(); x++)
    {
        if((sp = dbcSpell.LookupRow(x)) == NULL)
            continue;

        SetSingleSpellDefaults(sp);
    }

    std::set<uint32> dummySpells;
    sLog.Notice("World", "Filling spell pool data...");
    PoolSpellData();

    sLog.Notice("World", "Filling spell pool data...");
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

        // Apply spell fixes.
        ApplySingleSpellFixes(sp);
        ApplyCoeffSpellFixes(sp);
        SetProcFlags(sp);
    }

    sLog.Notice("World", "Setting target flags...");
    SetupSpellTargets();

    sLog.Notice("World", "Processing %u dummy spells...", dummySpells.size());
    for(set<uint32>::iterator itr = dummySpells.begin(); itr != dummySpells.end(); itr++)
    {   // Crow: Create the dummy spell, and apply fixs :D
        SetSingleSpellDefaults(sp = CreateDummySpell(*itr));
        ApplySingleSpellFixes(sp);
        ApplyCoeffSpellFixes(sp);
        SetProcFlags(sp);
    }

    SpellEntry* sp2 = new SpellEntry(*(sp = dbcSpell.LookupEntry(26659)));
    sp2->Id = 62388;
    sp2->Name = ((char*)"Dummy Shit");
    sp2->DurationIndex = 41;
    sp2->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    dbcSpell.SetRow(62388, sp2);
}

void SetProcFlags(SpellEntry *sp)
{

}

SpellEntry* CreateDummySpell(uint32 id)
{
    std::string name = "Dummy Trigger";
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
    switch(sp->SpellSkillLine)
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

void SetSingleSpellDefaults(SpellEntry *sp)
{
    /// 4.0.6 field copies
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
    // SpellRangeEntry
    sp->Range[0] = 0.0f;
    sp->Range[1] = 0.0f;
    sp->Range[2] = 0.0f;
    sp->Range[3] = 0.0f;
    /// Spell Pointers
    sp->Duration[0] = 0;
    sp->Duration[1] = 0;
    sp->Duration[2] = 0;

    /// Defaults
    sp->Class = 0;
    sp->School = FirstBitValue(sp->SchoolMask);
    sp->NameHash = crc32((const unsigned char*)sp->Name, (unsigned int)strlen(sp->Name)); //need these set before we start processing spells
    sp->RankNumber = sp->GeneratedThreat = sp->SpellSkillLine = 0;
    for(uint8 i = 0; i < 3; i++) sp->CustomAttributes[i] = 0;
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

