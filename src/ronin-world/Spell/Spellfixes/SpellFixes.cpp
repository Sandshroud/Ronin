/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void PoolSpellData();
void SetSingleSpellDefaults(SpellEntry *sp);
void DumpOpcodeData();

void ApplyNormalFixes()
{
    sLog.Notice("World", "Processing %u spells, highest %u...", dbcSpell.GetNumRows(), dbcSpell.GetMaxEntry());

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
    //DumpOpcodeData();

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

    sLog.Notice("World", "Initializing spell effect class...");
    SpellEffectClass::InitializeSpellEffectClass();

    sLog.Notice("World", "Processing %u dummy spells...", dummySpells.size());
    for(std::set<uint32>::iterator itr = dummySpells.begin(); itr != dummySpells.end(); itr++)
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
    if(!dbcSpell.SetEntry(62388, sp2))
        delete sp2;
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
    if(dbcSpell.SetEntry(62388, sp))
        return sp;
    delete sp;
    return NULL;
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
    sp->EquippedItemInventoryTypeMask = 0;
    sp->EquippedItemSubClassMask = 0;
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
    sp->School = RONIN_UTIL::FirstBitValue(sp->SchoolMask);
    sp->NameHash = crc32((const unsigned char*)sp->Name, (unsigned int)strlen(sp->Name)); //need these set before we start processing spells
    sp->RankNumber = sp->GeneratedThreat = sp->SpellSkillLine = 0;
    for(uint8 i = 0; i < 3; i++) sp->CustomAttributes[i] = 0;
    sp->isUnique = sp->always_apply = sp->inline_effects = false;

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

std::map<uint8, uint32> Spell::m_implicitTargetFlags;

void SetupSpellTargets()
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
            spellInfo->EquippedItemInventoryTypeMask = EquippedItems->EquippedItemInventoryTypeMask;
            spellInfo->EquippedItemSubClassMask = EquippedItems->EquippedItemSubClassMask;
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
            for(int8 i = 0; i < 8; ++i)
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



void DumpOpcodeLine(FILE * output, char *name, uint16 opcode)
{
    std::string spaces(" ");
    for(uint8 i = 0; i < 60; i++)
        if(i >= strlen(name))
            spaces.append(" ");
    fprintf(output, "    SetOpcodeData(%s,%s\"%s\",%s0x%.4X);", name, spaces.c_str(), name, spaces.c_str(), opcode);
}

void DumpOpcodeData()
{
    FILE *output = NULL;
    fopen_s(&output, "opcodeDump.txt", "w+");
    if(output == NULL)
        return;

    DumpOpcodeLine(output, "CMSG_CHARACTER_CREATE", 0x4A36);
    DumpOpcodeLine(output, "CMSG_CHARACTER_CUSTOMIZE", 0x2C34);
    DumpOpcodeLine(output, "CMSG_CHARACTER_DELETE", 0x6425);
    DumpOpcodeLine(output, "CMSG_CHARACTER_ENUM", 0x0502);
    DumpOpcodeLine(output, "CMSG_CHARACTER_FACTION_CHANGE", 0x2735);
    DumpOpcodeLine(output, "CMSG_CHARACTER_RACE_CHANGE", 0x0D24);
    DumpOpcodeLine(output, "CMSG_CHARACTER_RENAME", 0x2327);
    DumpOpcodeLine(output, "CMSG_GMRESPONSE_RESOLVE", 0x6506);
    DumpOpcodeLine(output, "CMSG_GMSURVEY_SUBMIT", 0x2724);
    DumpOpcodeLine(output, "CMSG_GMTICKET_CREATE", 0x0137);
    DumpOpcodeLine(output, "CMSG_GMTICKET_DELETETICKET", 0x6B14);
    DumpOpcodeLine(output, "CMSG_GMTICKET_GETTICKET", 0x0326);
    DumpOpcodeLine(output, "CMSG_GMTICKET_SYSTEMSTATUS", 0x4205);
    DumpOpcodeLine(output, "CMSG_GMTICKET_UPDATETEXT", 0x0636);
    DumpOpcodeLine(output, "CMSG_GM_REPORT_LAG", 0x6726);
    DumpOpcodeLine(output, "SMSG_GMRESPONSE_DB_ERROR", 0x0006);
    DumpOpcodeLine(output, "SMSG_GMRESPONSE_RECEIVED", 0x2E34);
    DumpOpcodeLine(output, "SMSG_GMRESPONSE_STATUS_UPDATE", 0x0A04);
    DumpOpcodeLine(output, "SMSG_GMTICKET_CREATE", 0x2107);
    DumpOpcodeLine(output, "SMSG_GMTICKET_DELETETICKET", 0x6D17);
    DumpOpcodeLine(output, "SMSG_GMTICKET_GETTICKET", 0x2C15);
    DumpOpcodeLine(output, "SMSG_GMTICKET_SYSTEMSTATUS", 0x0D35);
    DumpOpcodeLine(output, "SMSG_GMTICKET_UPDATETEXT", 0x6535);
    DumpOpcodeLine(output, "SMSG_GM_MESSAGECHAT", 0x6434);
    DumpOpcodeLine(output, "SMSG_GM_PLAYER_INFO", 0x4A15);
    DumpOpcodeLine(output, "SMSG_GM_TICKET_STATUS_UPDATE", 0x2C25);
    DumpOpcodeLine(output, "CMSG_ACCEPT_LEVEL_GRANT", 0x0205);
    DumpOpcodeLine(output, "CMSG_ACCEPT_TRADE", 0x7110);
    DumpOpcodeLine(output, "CMSG_ACTIVATETAXI", 0x6E06);
    DumpOpcodeLine(output, "CMSG_ACTIVATETAXIEXPRESS", 0x0515);
    DumpOpcodeLine(output, "CMSG_ADDON_REGISTERED_PREFIXES", 0x0954);
    DumpOpcodeLine(output, "CMSG_ADD_FRIEND", 0x6527);
    DumpOpcodeLine(output, "CMSG_ADD_IGNORE", 0x4726);
    DumpOpcodeLine(output, "CMSG_ADD_VOICE_IGNORE", 0x0F06);
    DumpOpcodeLine(output, "CMSG_ALTER_APPEARANCE", 0x0914);
    DumpOpcodeLine(output, "CMSG_AREATRIGGER", 0x0937);
    DumpOpcodeLine(output, "CMSG_AREA_SPIRIT_HEALER_QUERY", 0x4907);
    DumpOpcodeLine(output, "CMSG_AREA_SPIRIT_HEALER_QUEUE", 0x4815);
    DumpOpcodeLine(output, "CMSG_ARENA_TEAM_ACCEPT", 0x2A25);
    DumpOpcodeLine(output, "CMSG_ARENA_TEAM_CREATE", 0x04A1);
    DumpOpcodeLine(output, "CMSG_ARENA_TEAM_DECLINE", 0x6925);
    DumpOpcodeLine(output, "CMSG_ARENA_TEAM_DISBAND", 0x6504);
    DumpOpcodeLine(output, "CMSG_ARENA_TEAM_INVITE", 0x2F27);
    DumpOpcodeLine(output, "CMSG_ARENA_TEAM_LEADER", 0x4204);
    DumpOpcodeLine(output, "CMSG_ARENA_TEAM_LEAVE", 0x0E16);
    DumpOpcodeLine(output, "CMSG_ARENA_TEAM_QUERY", 0x0514);
    DumpOpcodeLine(output, "CMSG_ARENA_TEAM_REMOVE", 0x2F05);
    DumpOpcodeLine(output, "CMSG_ARENA_TEAM_ROSTER", 0x6F37);
    DumpOpcodeLine(output, "CMSG_ATTACKSTOP", 0x4106);
    DumpOpcodeLine(output, "CMSG_ATTACKSWING", 0x0926);
    DumpOpcodeLine(output, "CMSG_AUCTION_LIST_BIDDER_ITEMS", 0x6937);
    DumpOpcodeLine(output, "CMSG_AUCTION_LIST_ITEMS", 0x0324);
    DumpOpcodeLine(output, "CMSG_AUCTION_LIST_OWNER_ITEMS", 0x0206);
    DumpOpcodeLine(output, "CMSG_AUCTION_LIST_PENDING_SALES", 0x2C17);
    DumpOpcodeLine(output, "CMSG_AUCTION_PLACE_BID", 0x2306);
    DumpOpcodeLine(output, "CMSG_AUCTION_REMOVE_ITEM", 0x6426);
    DumpOpcodeLine(output, "CMSG_AUCTION_SELL_ITEM", 0x4A06);
    DumpOpcodeLine(output, "CMSG_AUTH_SESSION", 0x0449);
    DumpOpcodeLine(output, "CMSG_AUTOBANK_ITEM", 0x2537);
    DumpOpcodeLine(output, "CMSG_AUTOEQUIP_ITEM", 0x4304);
    DumpOpcodeLine(output, "CMSG_AUTOEQUIP_ITEM_SLOT", 0x4A17);
    DumpOpcodeLine(output, "CMSG_AUTOSTORE_BAG_ITEM", 0x0236);
    DumpOpcodeLine(output, "CMSG_AUTOSTORE_BANK_ITEM", 0x0607);
    DumpOpcodeLine(output, "CMSG_AUTOSTORE_LOOT_ITEM", 0x0E34);
    DumpOpcodeLine(output, "CMSG_AUTO_DECLINE_GUILD_INVITES", 0x2034);
    DumpOpcodeLine(output, "CMSG_BANKER_ACTIVATE", 0x0005);
    DumpOpcodeLine(output, "CMSG_BATTLEFIELD_LEAVE", 0x3018);
    DumpOpcodeLine(output, "CMSG_BATTLEFIELD_LIST", 0x3814);
    DumpOpcodeLine(output, "CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE", 0x05A3);
    DumpOpcodeLine(output, "CMSG_BATTLEFIELD_MGR_EXIT_REQUEST", 0x2490);
    DumpOpcodeLine(output, "CMSG_BATTLEFIELD_MGR_QUEUE_INVITE_RESPONSE", 0x0413);
    DumpOpcodeLine(output, "CMSG_BATTLEFIELD_MGR_QUEUE_REQUEST", 0x710C);
    DumpOpcodeLine(output, "CMSG_BATTLEFIELD_PORT", 0x711A);
    DumpOpcodeLine(output, "CMSG_BATTLEFIELD_STATUS", 0x2500);
    DumpOpcodeLine(output, "CMSG_BATTLEGROUND_PLAYER_POSITIONS", 0x3902);
    DumpOpcodeLine(output, "CMSG_BATTLEMASTER_JOIN", 0x7902);
    DumpOpcodeLine(output, "CMSG_BATTLEMASTER_JOIN_ARENA", 0x701C);
    DumpOpcodeLine(output, "CMSG_BATTLEMASTER_JOIN_RATED", 0x3B18);
    DumpOpcodeLine(output, "CMSG_BEGIN_TRADE", 0x721E);
    DumpOpcodeLine(output, "CMSG_BINDER_ACTIVATE", 0x4006);
    DumpOpcodeLine(output, "CMSG_BUG", 0x4035);
    DumpOpcodeLine(output, "CMSG_BUSY_TRADE", 0x331C);
    DumpOpcodeLine(output, "CMSG_BUYBACK_ITEM", 0x6C17);
    DumpOpcodeLine(output, "CMSG_BUY_BANK_SLOT", 0x0425);
    DumpOpcodeLine(output, "CMSG_BUY_ITEM", 0x0736);
    DumpOpcodeLine(output, "CMSG_CALENDAR_ADD_EVENT", 0x0726);
    DumpOpcodeLine(output, "CMSG_CALENDAR_ARENA_TEAM", 0x0204);
    DumpOpcodeLine(output, "CMSG_CALENDAR_COMPLAIN", 0x4C36);
    DumpOpcodeLine(output, "CMSG_CALENDAR_COPY_EVENT", 0x0207);
    DumpOpcodeLine(output, "CMSG_CALENDAR_EVENT_INVITE", 0x2435);
    DumpOpcodeLine(output, "CMSG_CALENDAR_EVENT_MODERATOR_STATUS", 0x6B35);
    DumpOpcodeLine(output, "CMSG_CALENDAR_EVENT_REMOVE_INVITE", 0x4337);
    DumpOpcodeLine(output, "CMSG_CALENDAR_EVENT_RSVP", 0x0227);
    DumpOpcodeLine(output, "CMSG_CALENDAR_EVENT_SIGNUP", 0x6606);
    DumpOpcodeLine(output, "CMSG_CALENDAR_EVENT_STATUS", 0x2D24);
    DumpOpcodeLine(output, "CMSG_CALENDAR_GET_CALENDAR", 0x2814);
    DumpOpcodeLine(output, "CMSG_CALENDAR_GET_EVENT", 0x6416);
    DumpOpcodeLine(output, "CMSG_CALENDAR_GET_NUM_PENDING", 0x4D05);
    DumpOpcodeLine(output, "CMSG_CALENDAR_GUILD_FILTER", 0x4A16);
    DumpOpcodeLine(output, "CMSG_CALENDAR_REMOVE_EVENT", 0x6636);
    DumpOpcodeLine(output, "CMSG_CALENDAR_UPDATE_EVENT", 0x2114);
    DumpOpcodeLine(output, "CMSG_CANCEL_AURA", 0x0E26);
    DumpOpcodeLine(output, "CMSG_CANCEL_AUTO_REPEAT_SPELL", 0x6C35);
    DumpOpcodeLine(output, "CMSG_CANCEL_CAST", 0x0115);
    DumpOpcodeLine(output, "CMSG_CANCEL_CHANNELLING", 0x6C25);
    DumpOpcodeLine(output, "CMSG_CANCEL_MOUNT_AURA", 0x0635);
    DumpOpcodeLine(output, "CMSG_CANCEL_QUEUED_SPELL", 0x7B1C);
    DumpOpcodeLine(output, "CMSG_CANCEL_TEMP_ENCHANTMENT", 0x6C37);
    DumpOpcodeLine(output, "CMSG_CANCEL_TRADE", 0x731E);
    DumpOpcodeLine(output, "CMSG_CAST_SPELL", 0x4C07);
    DumpOpcodeLine(output, "CMSG_CHANGEPLAYER_DIFFICULTY", 0x6107);
    DumpOpcodeLine(output, "CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE", 0x7310);
    DumpOpcodeLine(output, "CMSG_CHANNEL_ANNOUNCEMENTS", 0x1146);
    DumpOpcodeLine(output, "CMSG_CHANNEL_BAN", 0x3D56);
    DumpOpcodeLine(output, "CMSG_CHANNEL_DISPLAY_LIST", 0x2144);
    DumpOpcodeLine(output, "CMSG_CHANNEL_INVITE", 0x0144);
    DumpOpcodeLine(output, "CMSG_CHANNEL_KICK", 0x3156);
    DumpOpcodeLine(output, "CMSG_CHANNEL_LIST", 0x1556);
    DumpOpcodeLine(output, "CMSG_CHANNEL_MODERATE", 0x2944);
    DumpOpcodeLine(output, "CMSG_CHANNEL_MODERATOR", 0x0146);
    DumpOpcodeLine(output, "CMSG_CHANNEL_MUTE", 0x2554);
    DumpOpcodeLine(output, "CMSG_CHANNEL_OWNER", 0x3D44);
    DumpOpcodeLine(output, "CMSG_CHANNEL_PASSWORD", 0x2556);
    DumpOpcodeLine(output, "CMSG_CHANNEL_ROSTER_INFO", 0x3546);
    DumpOpcodeLine(output, "CMSG_CHANNEL_SET_OWNER", 0x3556);
    DumpOpcodeLine(output, "CMSG_CHANNEL_SILENCE_ALL", 0x2154);
    DumpOpcodeLine(output, "CMSG_CHANNEL_SILENCE_VOICE", 0x2D54);
    DumpOpcodeLine(output, "CMSG_CHANNEL_UNBAN", 0x2D46);
    DumpOpcodeLine(output, "CMSG_CHANNEL_UNMODERATOR", 0x1954);
    DumpOpcodeLine(output, "CMSG_CHANNEL_UNMUTE", 0x3554);
    DumpOpcodeLine(output, "CMSG_CHANNEL_UNSILENCE_ALL", 0x2546);
    DumpOpcodeLine(output, "CMSG_CHANNEL_UNSILENCE_VOICE", 0x3146);
    DumpOpcodeLine(output, "CMSG_CHANNEL_VOICE_OFF", 0x3144);
    DumpOpcodeLine(output, "CMSG_CHANNEL_VOICE_ON", 0x1144);
    DumpOpcodeLine(output, "CMSG_CHAT_FILTERED", 0x0946);
    DumpOpcodeLine(output, "CMSG_CHAT_IGNORED", 0x0D54);
    DumpOpcodeLine(output, "CMSG_CLEAR_CHANNEL_WATCH", 0x2604);
    DumpOpcodeLine(output, "CMSG_CLEAR_RAID_MARKER", 0x7300);
    DumpOpcodeLine(output, "CMSG_CLEAR_TRADE_ITEM", 0x7018);
    DumpOpcodeLine(output, "CMSG_COMMENTATOR_ENABLE", 0x0B07);
    DumpOpcodeLine(output, "CMSG_COMMENTATOR_ENTER_INSTANCE", 0x4105);
    DumpOpcodeLine(output, "CMSG_COMMENTATOR_EXIT_INSTANCE", 0x6136);
    DumpOpcodeLine(output, "CMSG_COMMENTATOR_GET_MAP_INFO", 0x0026);
    DumpOpcodeLine(output, "CMSG_COMMENTATOR_GET_PARTY_INFO", 0x2412);
    DumpOpcodeLine(output, "CMSG_COMMENTATOR_GET_PLAYER_INFO", 0x0D14);
    DumpOpcodeLine(output, "CMSG_COMMENTATOR_INSTANCE_COMMAND", 0x0917);
    DumpOpcodeLine(output, "CMSG_COMMENTATOR_SKIRMISH_QUEUE_COMMAND", 0x0025);
    DumpOpcodeLine(output, "CMSG_COMMENTATOR_START_WARGAME", 0x25A0);
    DumpOpcodeLine(output, "CMSG_COMPLAIN", 0x0427);
    DumpOpcodeLine(output, "CMSG_COMPLETE_CINEMATIC", 0x2116);
    DumpOpcodeLine(output, "CMSG_COMPLETE_MOVIE", 0x4136);
    DumpOpcodeLine(output, "CMSG_CONNECT_TO_FAILED", 0x2533);
    DumpOpcodeLine(output, "CMSG_CONTACT_LIST", 0x4534);
    DumpOpcodeLine(output, "CMSG_CORPSE_MAP_POSITION_QUERY", 0x6205);
    DumpOpcodeLine(output, "CMSG_CREATURE_QUERY", 0x2706);
    DumpOpcodeLine(output, "CMSG_DANCE_QUERY", 0x4E07);
    DumpOpcodeLine(output, "CMSG_DEL_FRIEND", 0x6A15);
    DumpOpcodeLine(output, "CMSG_DEL_IGNORE", 0x6D26);
    DumpOpcodeLine(output, "CMSG_DEL_VOICE_IGNORE", 0x0024);
    DumpOpcodeLine(output, "CMSG_DESTROY_ITEM", 0x4A27);
    DumpOpcodeLine(output, "CMSG_DISMISS_CONTROLLED_VEHICLE", 0x3218);
    DumpOpcodeLine(output, "CMSG_DISMISS_CRITTER", 0x4227);
    DumpOpcodeLine(output, "CMSG_DUEL_ACCEPTED", 0x2136);
    DumpOpcodeLine(output, "CMSG_DUEL_CANCELLED", 0x6624);
    DumpOpcodeLine(output, "CMSG_EJECT_PASSENGER", 0x6927);
    DumpOpcodeLine(output, "CMSG_EMOTE", 0x4C26);
    DumpOpcodeLine(output, "CMSG_ENABLETAXI", 0x0C16);
    DumpOpcodeLine(output, "CMSG_ENABLE_NAGLE", 0x4449);
    DumpOpcodeLine(output, "CMSG_EQUIPMENT_SET_DELETE", 0x4D07);
    DumpOpcodeLine(output, "CMSG_EQUIPMENT_SET_SAVE", 0x4F27);
    DumpOpcodeLine(output, "CMSG_EQUIPMENT_SET_USE", 0x0417);
    DumpOpcodeLine(output, "CMSG_FAR_SIGHT", 0x4835);
    DumpOpcodeLine(output, "CMSG_FORCE_MOVE_ROOT_ACK", 0x701E);
    DumpOpcodeLine(output, "CMSG_FORCE_MOVE_UNROOT_ACK", 0x7808);
    DumpOpcodeLine(output, "CMSG_GAMEOBJECT_QUERY", 0x4017);
    DumpOpcodeLine(output, "CMSG_GAMEOBJ_REPORT_USE", 0x4827);
    DumpOpcodeLine(output, "CMSG_GAMEOBJ_USE", 0x4E17);
    DumpOpcodeLine(output, "CMSG_GET_MAIL_LIST", 0x4D37);
    DumpOpcodeLine(output, "CMSG_GET_MIRRORIMAGE_DATA", 0x0C25);
    DumpOpcodeLine(output, "CMSG_GOSSIP_HELLO", 0x4525);
    DumpOpcodeLine(output, "CMSG_GOSSIP_SELECT_OPTION", 0x0216);
    DumpOpcodeLine(output, "CMSG_GRANT_LEVEL", 0x6D16);
    DumpOpcodeLine(output, "CMSG_GROUP_ASSISTANT_LEADER", 0x6025);
    DumpOpcodeLine(output, "CMSG_GROUP_CHANGE_SUB_GROUP", 0x4124);
    DumpOpcodeLine(output, "CMSG_GROUP_DISBAND", 0x2804);
    DumpOpcodeLine(output, "CMSG_GROUP_INVITE", 0x0513);
    DumpOpcodeLine(output, "CMSG_GROUP_INVITE_RESPONSE", 0x0410);
    DumpOpcodeLine(output, "CMSG_GROUP_RAID_CONVERT", 0x6E27);
    DumpOpcodeLine(output, "CMSG_GROUP_REQUEST_JOIN_UPDATES", 0x2583);
    DumpOpcodeLine(output, "CMSG_GROUP_SET_LEADER", 0x4C17);
    DumpOpcodeLine(output, "CMSG_GROUP_SET_ROLES", 0x25B1);
    DumpOpcodeLine(output, "CMSG_GROUP_SWAP_SUB_GROUP", 0x0034);
    DumpOpcodeLine(output, "CMSG_GROUP_UNINVITE_GUID", 0x2E07);
    DumpOpcodeLine(output, "CMSG_GUILD_ACCEPT", 0x2531);
    DumpOpcodeLine(output, "CMSG_GUILD_ACHIEVEMENT_MEMBERS", 0x3025);
    DumpOpcodeLine(output, "CMSG_GUILD_ACHIEVEMENT_PROGRESS_QUERY", 0x3235);
    DumpOpcodeLine(output, "CMSG_GUILD_ADD_RANK", 0x3030);
    DumpOpcodeLine(output, "CMSG_GUILD_ASSIGN_MEMBER_RANK", 0x3032);
    DumpOpcodeLine(output, "CMSG_GUILD_BANKER_ACTIVATE", 0x2E37);
    DumpOpcodeLine(output, "CMSG_GUILD_BANK_BUY_TAB", 0x0C37);
    DumpOpcodeLine(output, "CMSG_GUILD_BANK_DEPOSIT_MONEY", 0x0707);
    DumpOpcodeLine(output, "CMSG_GUILD_BANK_LOG_QUERY", 0x3224);
    DumpOpcodeLine(output, "CMSG_GUILD_BANK_MONEY_WITHDRAWN_QUERY", 0x1225);
    DumpOpcodeLine(output, "CMSG_GUILD_BANK_QUERY_TAB", 0x2E35);
    DumpOpcodeLine(output, "CMSG_GUILD_BANK_QUERY_TEXT", 0x3220);
    DumpOpcodeLine(output, "CMSG_GUILD_BANK_SWAP_ITEMS", 0x2315);
    DumpOpcodeLine(output, "CMSG_GUILD_BANK_UPDATE_TAB", 0x0106);
    DumpOpcodeLine(output, "CMSG_GUILD_BANK_WITHDRAW_MONEY", 0x0037);
    DumpOpcodeLine(output, "CMSG_GUILD_CHANGE_NAME_REQUEST", 0x1232);
    DumpOpcodeLine(output, "CMSG_GUILD_DECLINE", 0x3231);
    DumpOpcodeLine(output, "CMSG_GUILD_DEL_RANK", 0x3234);
    DumpOpcodeLine(output, "CMSG_GUILD_DEMOTE", 0x1020);
    DumpOpcodeLine(output, "CMSG_GUILD_DISBAND", 0x3226);
    DumpOpcodeLine(output, "CMSG_GUILD_EVENT_LOG_QUERY", 0x1220);
    DumpOpcodeLine(output, "CMSG_GUILD_INFO_TEXT", 0x3227);
    DumpOpcodeLine(output, "CMSG_GUILD_INVITE", 0x24B0);
    DumpOpcodeLine(output, "CMSG_GUILD_LEAVE", 0x1021);
    DumpOpcodeLine(output, "CMSG_GUILD_MEMBER_SEND_SOR_REQUEST", 0x3225);
    DumpOpcodeLine(output, "CMSG_GUILD_MOTD", 0x1035);
    DumpOpcodeLine(output, "CMSG_GUILD_NEWS_UPDATE_STICKY", 0x3223);
    DumpOpcodeLine(output, "CMSG_GUILD_PERMISSIONS", 0x3022);
    DumpOpcodeLine(output, "CMSG_GUILD_PROMOTE", 0x1030);
    DumpOpcodeLine(output, "CMSG_GUILD_QUERY", 0x4426);
    DumpOpcodeLine(output, "CMSG_GUILD_QUERY_NEWS", 0x3020);
    DumpOpcodeLine(output, "CMSG_GUILD_QUERY_RANKS", 0x1026);
    DumpOpcodeLine(output, "CMSG_GUILD_REMOVE", 0x1231);
    DumpOpcodeLine(output, "CMSG_GUILD_REPLACE_GUILD_MASTER", 0x1034);
    DumpOpcodeLine(output, "CMSG_GUILD_REQUEST_CHALLENGE_UPDATE", 0x1224);
    DumpOpcodeLine(output, "CMSG_GUILD_REQUEST_MAX_DAILY_XP", 0x3232);
    DumpOpcodeLine(output, "CMSG_GUILD_REQUEST_PARTY_STATE", 0x3900);
    DumpOpcodeLine(output, "CMSG_GUILD_ROSTER", 0x1226);
    DumpOpcodeLine(output, "CMSG_GUILD_SET_ACHIEVEMENT_TRACKING", 0x1027);
    DumpOpcodeLine(output, "CMSG_GUILD_SET_GUILD_MASTER", 0x3034);
    DumpOpcodeLine(output, "CMSG_GUILD_SET_NOTE", 0x1233);
    DumpOpcodeLine(output, "CMSG_GUILD_SET_RANK_PERMISSIONS", 0x1024);
    DumpOpcodeLine(output, "CMSG_GUILD_SWITCH_RANK", 0x1221);
    DumpOpcodeLine(output, "CMSG_HEARTH_AND_RESURRECT", 0x4B34);
    DumpOpcodeLine(output, "CMSG_IGNORE_TRADE", 0x7112);
    DumpOpcodeLine(output, "CMSG_INITIATE_TRADE", 0x7916);
    DumpOpcodeLine(output, "CMSG_INSPECT", 0x0927);
    DumpOpcodeLine(output, "CMSG_INSPECT_HONOR_STATS", 0x791E);
    DumpOpcodeLine(output, "CMSG_INSTANCE_LOCK_WARNING_RESPONSE", 0x6234);
    DumpOpcodeLine(output, "CMSG_ITEM_REFUND", 0x6134);
    DumpOpcodeLine(output, "CMSG_ITEM_REFUND_INFO", 0x2206);
    DumpOpcodeLine(output, "CMSG_ITEM_TEXT_QUERY", 0x2406);
    DumpOpcodeLine(output, "CMSG_JOIN_CHANNEL", 0x0156);
    DumpOpcodeLine(output, "CMSG_KEEP_ALIVE", 0x0015);
    DumpOpcodeLine(output, "CMSG_LEARN_PREVIEW_TALENTS", 0x2415);
    DumpOpcodeLine(output, "CMSG_LEARN_PREVIEW_TALENTS_PET", 0x6E24);
    DumpOpcodeLine(output, "CMSG_LEARN_TALENT", 0x0306);
    DumpOpcodeLine(output, "CMSG_LEAVE_CHANNEL", 0x2D56);
    DumpOpcodeLine(output, "CMSG_LFG_GET_STATUS", 0x2581);
    DumpOpcodeLine(output, "CMSG_LFG_JOIN", 0x2430);
    DumpOpcodeLine(output, "CMSG_LFG_LEAVE", 0x2433);
    DumpOpcodeLine(output, "CMSG_LFG_LFR_JOIN", 0x0531);
    DumpOpcodeLine(output, "CMSG_LFG_LFR_LEAVE", 0x0500);
    DumpOpcodeLine(output, "CMSG_LFG_LOCK_INFO_REQUEST", 0x0412);
    DumpOpcodeLine(output, "CMSG_LFG_PROPOSAL_RESULT", 0x0403);
    DumpOpcodeLine(output, "CMSG_LFG_SET_BOOT_VOTE", 0x04B3);
    DumpOpcodeLine(output, "CMSG_LFG_SET_COMMENT", 0x0530);
    DumpOpcodeLine(output, "CMSG_LFG_SET_ROLES", 0x0480);
    DumpOpcodeLine(output, "CMSG_LFG_TELEPORT", 0x2482);
    DumpOpcodeLine(output, "CMSG_LF_GUILD_ADD_RECRUIT", 0x4448);
    DumpOpcodeLine(output, "CMSG_LF_GUILD_BROWSE", 0x0548);
    DumpOpcodeLine(output, "CMSG_LF_GUILD_DECLINE_RECRUIT", 0x1031);
    DumpOpcodeLine(output, "CMSG_LF_GUILD_GET_APPLICATIONS", 0x1230);
    DumpOpcodeLine(output, "CMSG_LF_GUILD_GET_RECRUITS", 0x3230);
    DumpOpcodeLine(output, "CMSG_LF_GUILD_POST_REQUEST", 0x3237);
    DumpOpcodeLine(output, "CMSG_LF_GUILD_REMOVE_RECRUIT", 0x3027);
    DumpOpcodeLine(output, "CMSG_LF_GUILD_SET_GUILD_POST", 0x0448);
    DumpOpcodeLine(output, "CMSG_LIST_INVENTORY", 0x2806);
    DumpOpcodeLine(output, "CMSG_LOAD_SCREEN", 0x2422);
    DumpOpcodeLine(output, "CMSG_LOGOUT_CANCEL", 0x2324);
    DumpOpcodeLine(output, "CMSG_LOGOUT_REQUEST", 0x0A25);
    DumpOpcodeLine(output, "CMSG_LOG_DISCONNECT", 0x446D);
    DumpOpcodeLine(output, "CMSG_LOOT", 0x0127);
    DumpOpcodeLine(output, "CMSG_LOOT_CURRENCY", 0x781C);
    DumpOpcodeLine(output, "CMSG_LOOT_MASTER_GIVE", 0x4F35);
    DumpOpcodeLine(output, "CMSG_LOOT_METHOD", 0x2F24);
    DumpOpcodeLine(output, "CMSG_LOOT_MONEY", 0x6227);
    DumpOpcodeLine(output, "CMSG_LOOT_RELEASE", 0x2007);
    DumpOpcodeLine(output, "CMSG_LOOT_ROLL", 0x6934);
    DumpOpcodeLine(output, "CMSG_MAIL_CREATE_TEXT_ITEM", 0x0B14);
    DumpOpcodeLine(output, "CMSG_MAIL_DELETE", 0x6104);
    DumpOpcodeLine(output, "CMSG_MAIL_MARK_AS_READ", 0x0C07);
    DumpOpcodeLine(output, "CMSG_MAIL_RETURN_TO_SENDER", 0x0816);
    DumpOpcodeLine(output, "CMSG_MAIL_TAKE_ITEM", 0x2B06);
    DumpOpcodeLine(output, "CMSG_MAIL_TAKE_MONEY", 0x4034);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_ADDON_BATTLEGROUND", 0x0D46);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_ADDON_GUILD", 0x0544);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_ADDON_OFFICER", 0x3954);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_ADDON_PARTY", 0x0546);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_ADDON_RAID", 0x1D56);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_ADDON_WHISPER", 0x2146);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_AFK", 0x0D44);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_BATTLEGROUND", 0x2156);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_CHANNEL", 0x1D44);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_DND", 0x2946);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_EMOTE", 0x1156);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_GUILD", 0x3956);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_OFFICER", 0x1946);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_PARTY", 0x1D46);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_RAID", 0x2D44);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_RAID_WARNING", 0x0944);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_SAY", 0x1154);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_WHISPER", 0x0D56);
    DumpOpcodeLine(output, "CMSG_MESSAGECHAT_YELL", 0x3544);
    DumpOpcodeLine(output, "CMSG_MINIGAME_MOVE", 0x2A34);
    DumpOpcodeLine(output, "CMSG_MOUNTSPECIAL_ANIM", 0x2807);
    DumpOpcodeLine(output, "CMSG_MOVE_CHNG_TRANSPORT", 0x3102);
    DumpOpcodeLine(output, "CMSG_MOVE_FALL_RESET", 0x310A);
    DumpOpcodeLine(output, "CMSG_MOVE_FEATHER_FALL_ACK", 0x3110);
    DumpOpcodeLine(output, "CMSG_MOVE_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK", 0x310E);
    DumpOpcodeLine(output, "CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK", 0x7314);
    DumpOpcodeLine(output, "CMSG_MOVE_FORCE_PITCH_RATE_CHANGE_ACK", 0x3100);
    DumpOpcodeLine(output, "CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK", 0x3216);
    DumpOpcodeLine(output, "CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK", 0x7818);
    DumpOpcodeLine(output, "CMSG_MOVE_FORCE_SWIM_BACK_SPEED_CHANGE_ACK", 0x7A16);
    DumpOpcodeLine(output, "CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK", 0x7A10);
    DumpOpcodeLine(output, "CMSG_MOVE_FORCE_TURN_RATE_CHANGE_ACK", 0x7316);
    DumpOpcodeLine(output, "CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK", 0x7210);
    DumpOpcodeLine(output, "CMSG_MOVE_GRAVITY_DISABLE_ACK", 0x3118);
    DumpOpcodeLine(output, "CMSG_MOVE_GRAVITY_ENABLE_ACK", 0x700A);
    DumpOpcodeLine(output, "CMSG_MOVE_HOVER_ACK", 0x3318);
    DumpOpcodeLine(output, "CMSG_MOVE_KNOCK_BACK_ACK", 0x721C);
    DumpOpcodeLine(output, "CMSG_MOVE_NOT_ACTIVE_MOVER", 0x7A1A);
    DumpOpcodeLine(output, "CMSG_MOVE_SET_CAN_FLY", 0x720E);
    DumpOpcodeLine(output, "CMSG_MOVE_SET_CAN_FLY_ACK", 0x790C);
    DumpOpcodeLine(output, "CMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY_ACK", 0x3014);
    DumpOpcodeLine(output, "CMSG_MOVE_SET_COLLISION_HEIGHT_ACK", 0x7114);
    DumpOpcodeLine(output, "CMSG_MOVE_SPLINE_DONE", 0x790E);
    DumpOpcodeLine(output, "CMSG_MOVE_TIME_SKIPPED", 0x7A0A);
    DumpOpcodeLine(output, "CMSG_MOVE_WATER_WALK_ACK", 0x3B00);
    DumpOpcodeLine(output, "CMSG_NAME_QUERY", 0x2224);
    DumpOpcodeLine(output, "CMSG_NEXT_CINEMATIC_CAMERA", 0x2014);
    DumpOpcodeLine(output, "CMSG_NPC_TEXT_QUERY", 0x4E24);
    DumpOpcodeLine(output, "CMSG_OBJECT_UPDATE_FAILED", 0x3808);
    DumpOpcodeLine(output, "CMSG_OBJECT_UPDATE_RESCUED", 0x3906);
    DumpOpcodeLine(output, "CMSG_OFFER_PETITION", 0x4817);
    DumpOpcodeLine(output, "CMSG_OPENING_CINEMATIC", 0x0A16);
    DumpOpcodeLine(output, "CMSG_OPEN_ITEM", 0x6A34);
    DumpOpcodeLine(output, "CMSG_OPT_OUT_OF_LOOT", 0x6B16);
    DumpOpcodeLine(output, "CMSG_PAGE_TEXT_QUERY", 0x6614);
    DumpOpcodeLine(output, "CMSG_PARTY_SILENCE", 0x6B26);
    DumpOpcodeLine(output, "CMSG_PARTY_UNSILENCE", 0x4D24);
    DumpOpcodeLine(output, "CMSG_PETITION_BUY", 0x4E05);
    DumpOpcodeLine(output, "CMSG_PETITION_QUERY", 0x4424);
    DumpOpcodeLine(output, "CMSG_PETITION_SHOWLIST", 0x4617);
    DumpOpcodeLine(output, "CMSG_PETITION_SHOW_SIGNATURES", 0x4F15);
    DumpOpcodeLine(output, "CMSG_PETITION_SIGN", 0x0E04);
    DumpOpcodeLine(output, "CMSG_PET_ABANDON", 0x0C24);
    DumpOpcodeLine(output, "CMSG_PET_ACTION", 0x0226);
    DumpOpcodeLine(output, "CMSG_PET_CANCEL_AURA", 0x4B25);
    DumpOpcodeLine(output, "CMSG_PET_CAST_SPELL", 0x6337);
    DumpOpcodeLine(output, "CMSG_PET_LEARN_TALENT", 0x6725);
    DumpOpcodeLine(output, "CMSG_PET_NAME_QUERY", 0x6F24);
    DumpOpcodeLine(output, "CMSG_PET_RENAME", 0x6406);
    DumpOpcodeLine(output, "CMSG_PET_SET_ACTION", 0x6904);
    DumpOpcodeLine(output, "CMSG_PET_SPELL_AUTOCAST", 0x2514);
    DumpOpcodeLine(output, "CMSG_PET_STOP_ATTACK", 0x6C14);
    DumpOpcodeLine(output, "CMSG_PING", 0x444D);
    DumpOpcodeLine(output, "CMSG_PLAYED_TIME", 0x0804);
    DumpOpcodeLine(output, "CMSG_PLAYER_LOGIN", 0x05B1);
    DumpOpcodeLine(output, "CMSG_PLAYER_VEHICLE_ENTER", 0x2705);
    DumpOpcodeLine(output, "CMSG_PLAY_DANCE", 0x6914);
    DumpOpcodeLine(output, "CMSG_PUSHQUESTTOPARTY", 0x4B14);
    DumpOpcodeLine(output, "CMSG_PVP_LOG_DATA", 0x7308);
    DumpOpcodeLine(output, "CMSG_QUERY_BATTLEFIELD_STATE", 0x7202);
    DumpOpcodeLine(output, "CMSG_QUERY_GUILD_MEMBERS_FOR_RECIPE", 0x1036);
    DumpOpcodeLine(output, "CMSG_QUERY_GUILD_MEMBER_RECIPES", 0x1037);
    DumpOpcodeLine(output, "CMSG_QUERY_GUILD_RECIPES", 0x3033);
    DumpOpcodeLine(output, "CMSG_QUERY_GUILD_REWARDS", 0x3012);
    DumpOpcodeLine(output, "CMSG_QUERY_GUILD_XP", 0x1237);
    DumpOpcodeLine(output, "CMSG_QUERY_INSPECT_ACHIEVEMENTS", 0x4D27);
    DumpOpcodeLine(output, "CMSG_QUERY_QUESTS_COMPLETED", 0x2317);
    DumpOpcodeLine(output, "CMSG_QUERY_TIME", 0x0A36);
    DumpOpcodeLine(output, "CMSG_QUESTGIVER_ACCEPT_QUEST", 0x6B37);
    DumpOpcodeLine(output, "CMSG_QUESTGIVER_CHOOSE_REWARD", 0x2125);
    DumpOpcodeLine(output, "CMSG_QUESTGIVER_COMPLETE_QUEST", 0x0114);
    DumpOpcodeLine(output, "CMSG_QUESTGIVER_HELLO", 0x0D17);
    DumpOpcodeLine(output, "CMSG_QUESTGIVER_QUERY_QUEST", 0x2F14);
    DumpOpcodeLine(output, "CMSG_QUESTGIVER_REQUEST_REWARD", 0x2534);
    DumpOpcodeLine(output, "CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY", 0x6305);
    DumpOpcodeLine(output, "CMSG_QUESTGIVER_STATUS_QUERY", 0x4407);
    DumpOpcodeLine(output, "CMSG_QUESTLOG_REMOVE_QUEST", 0x0D16);
    DumpOpcodeLine(output, "CMSG_QUEST_CONFIRM_ACCEPT", 0x0D15);
    DumpOpcodeLine(output, "CMSG_QUEST_NPC_QUERY", 0x7302);
    DumpOpcodeLine(output, "CMSG_QUEST_POI_QUERY", 0x4037);
    DumpOpcodeLine(output, "CMSG_QUEST_QUERY", 0x0D06);
    DumpOpcodeLine(output, "CMSG_RANDOMIZE_CHAR_NAME", 0x2413);
    DumpOpcodeLine(output, "CMSG_READY_FOR_ACCOUNT_DATA_TIMES", 0x2B16);
    DumpOpcodeLine(output, "CMSG_READ_ITEM", 0x2F16);
    DumpOpcodeLine(output, "CMSG_REALM_SPLIT", 0x2906);
    DumpOpcodeLine(output, "CMSG_RECLAIM_CORPSE", 0x4036);
    DumpOpcodeLine(output, "CMSG_REDIRECTION_AUTH_PROOF", 0x044D);
    DumpOpcodeLine(output, "CMSG_REFORGE_ITEM", 0x331A);
    DumpOpcodeLine(output, "CMSG_REORDER_CHARACTERS", 0x0593);
    DumpOpcodeLine(output, "CMSG_REPAIR_ITEM", 0x2917);
    DumpOpcodeLine(output, "CMSG_REPOP_REQUEST", 0x6235);
    DumpOpcodeLine(output, "CMSG_REPORT_PVP_AFK", 0x6734);
    DumpOpcodeLine(output, "CMSG_REQUEST_ACCOUNT_DATA", 0x6505);
    DumpOpcodeLine(output, "CMSG_REQUEST_CATEGORY_COOLDOWNS", 0x7102);
    DumpOpcodeLine(output, "CMSG_REQUEST_CEMETERY_LIST", 0x720A);
    DumpOpcodeLine(output, "CMSG_REQUEST_HOTFIX", 0x2401);
    DumpOpcodeLine(output, "CMSG_REQUEST_INSPECT_RATED_BG_STATS", 0x3010);
    DumpOpcodeLine(output, "CMSG_REQUEST_PARTY_MEMBER_STATS", 0x0C04);
    DumpOpcodeLine(output, "CMSG_REQUEST_PET_INFO", 0x4924);
    DumpOpcodeLine(output, "CMSG_REQUEST_PVP_OPTIONS_ENABLED", 0x24A1);
    DumpOpcodeLine(output, "CMSG_REQUEST_PVP_REWARDS", 0x780C);
    DumpOpcodeLine(output, "CMSG_REQUEST_RAID_INFO", 0x2F26);
    DumpOpcodeLine(output, "CMSG_REQUEST_RATED_BG_INFO", 0x2423);
    DumpOpcodeLine(output, "CMSG_REQUEST_RATED_BG_STATS", 0x05B3);
    DumpOpcodeLine(output, "CMSG_REQUEST_RESEARCH_HISTORY", 0x3306);
    DumpOpcodeLine(output, "CMSG_REQUEST_VEHICLE_EXIT", 0x2B35);
    DumpOpcodeLine(output, "CMSG_REQUEST_VEHICLE_NEXT_SEAT", 0x4434);
    DumpOpcodeLine(output, "CMSG_REQUEST_VEHICLE_PREV_SEAT", 0x4C04);
    DumpOpcodeLine(output, "CMSG_REQUEST_VEHICLE_SWITCH_SEAT", 0x4C14);
    DumpOpcodeLine(output, "CMSG_RESET_FACTION_CHEAT", 0x4469);
    DumpOpcodeLine(output, "CMSG_RESET_INSTANCES", 0x6E14);
    DumpOpcodeLine(output, "CMSG_RESURRECT_RESPONSE", 0x6827);
    DumpOpcodeLine(output, "CMSG_RETURN_TO_GRAVEYARD", 0x301E);
    DumpOpcodeLine(output, "CMSG_ROLE_POLL_BEGIN", 0x0430);
    DumpOpcodeLine(output, "CMSG_SAVE_CUF_PROFILES", 0x730E);
    DumpOpcodeLine(output, "CMSG_SELF_RES", 0x6115);
    DumpOpcodeLine(output, "CMSG_SELL_ITEM", 0x4E15);
    DumpOpcodeLine(output, "CMSG_SEND_MAIL", 0x0523);
    DumpOpcodeLine(output, "CMSG_SEND_SOR_REQUEST_VIA_ADDRESS", 0x0420);
    DumpOpcodeLine(output, "CMSG_SEND_SOR_REQUEST_VIA_BNET_ACCOUNT_ID", 0x0482);
    DumpOpcodeLine(output, "CMSG_SETSHEATHED", 0x4326);
    DumpOpcodeLine(output, "CMSG_SET_ACTIONBAR_TOGGLES", 0x2506);
    DumpOpcodeLine(output, "CMSG_SET_ACTION_BUTTON", 0x6F06);
    DumpOpcodeLine(output, "CMSG_SET_ACTIVE_MOVER", 0x3314);
    DumpOpcodeLine(output, "CMSG_SET_ACTIVE_VOICE_CHANNEL", 0x4305);
    DumpOpcodeLine(output, "CMSG_SET_ALLOW_LOW_LEVEL_RAID1", 0x4435);
    DumpOpcodeLine(output, "CMSG_SET_ALLOW_LOW_LEVEL_RAID2", 0x0536);
    DumpOpcodeLine(output, "CMSG_SET_CHANNEL_WATCH", 0x4517);
    DumpOpcodeLine(output, "CMSG_SET_CONTACT_NOTES", 0x6135);
    DumpOpcodeLine(output, "CMSG_SET_CURRENCY_FLAGS", 0x7306);
    DumpOpcodeLine(output, "CMSG_SET_EVERYONE_IS_ASSISTANT", 0x2530);
    DumpOpcodeLine(output, "CMSG_SET_FACTION_ATWAR", 0x0706);
    DumpOpcodeLine(output, "CMSG_SET_FACTION_INACTIVE", 0x0E37);
    DumpOpcodeLine(output, "CMSG_SET_GUILD_BANK_TEXT", 0x3023);
    DumpOpcodeLine(output, "CMSG_SET_PET_SLOT", 0x3A04);
    DumpOpcodeLine(output, "CMSG_SET_PLAYER_DECLINED_NAMES", 0x6316);
    DumpOpcodeLine(output, "CMSG_SET_PREFERED_CEMETERY", 0x311E);
    DumpOpcodeLine(output, "CMSG_SET_PRIMARY_TALENT_TREE", 0x4524);
    DumpOpcodeLine(output, "CMSG_SET_RELATIVE_POSITION", 0x311A);
    DumpOpcodeLine(output, "CMSG_SET_SAVED_INSTANCE_EXTEND", 0x6706);
    DumpOpcodeLine(output, "CMSG_SET_SELECTION", 0x0506);
    DumpOpcodeLine(output, "CMSG_SET_TAXI_BENCHMARK_MODE", 0x4314);
    DumpOpcodeLine(output, "CMSG_SET_TITLE", 0x2117);
    DumpOpcodeLine(output, "CMSG_SET_TRADE_CURRENCY", 0x3312);
    DumpOpcodeLine(output, "CMSG_SET_TRADE_GOLD", 0x3008);
    DumpOpcodeLine(output, "CMSG_SET_TRADE_ITEM", 0x7B0C);
    DumpOpcodeLine(output, "CMSG_SET_VEHICLE_REC_ID_ACK", 0x3108);
    DumpOpcodeLine(output, "CMSG_SET_WATCHED_FACTION", 0x2434);
    DumpOpcodeLine(output, "CMSG_SHOWING_CLOAK", 0x4135);
    DumpOpcodeLine(output, "CMSG_SHOWING_HELM", 0x0735);
    DumpOpcodeLine(output, "CMSG_SOCKET_GEMS", 0x2F04);
    DumpOpcodeLine(output, "CMSG_SPELLCLICK", 0x0805);
    DumpOpcodeLine(output, "CMSG_SPIRIT_HEALER_ACTIVATE", 0x2E26);
    DumpOpcodeLine(output, "CMSG_SPLIT_ITEM", 0x0F17);
    DumpOpcodeLine(output, "CMSG_STANDSTATECHANGE", 0x0535);
    DumpOpcodeLine(output, "CMSG_STOP_DANCE", 0x2907);
    DumpOpcodeLine(output, "CMSG_SUBMIT_BUG", 0x2520);
    DumpOpcodeLine(output, "CMSG_SUBMIT_COMPLAIN", 0x2501);
    DumpOpcodeLine(output, "CMSG_SUGGESTION_SUBMIT", 0x2512);
    DumpOpcodeLine(output, "CMSG_SUMMON_RESPONSE", 0x6F27);
    DumpOpcodeLine(output, "CMSG_SUSPEND_TOKEN", 0x046D);
    DumpOpcodeLine(output, "CMSG_SWAP_INV_ITEM", 0x2614);
    DumpOpcodeLine(output, "CMSG_SWAP_ITEM", 0x6326);
    DumpOpcodeLine(output, "CMSG_SYNC_DANCE", 0x0036);
    DumpOpcodeLine(output, "CMSG_TAXINODE_STATUS_QUERY", 0x2F25);
    DumpOpcodeLine(output, "CMSG_TAXIQUERYAVAILABLENODES", 0x6C06);
    DumpOpcodeLine(output, "CMSG_TELEPORT_castPtr<Unit>", 0x4206);
    DumpOpcodeLine(output, "CMSG_TEXT_EMOTE", 0x2E24);
    DumpOpcodeLine(output, "CMSG_TIME_ADJUSTMENT_RESPONSE", 0x3818);
    DumpOpcodeLine(output, "CMSG_TIME_SYNC_RESP", 0x3B0C);
    DumpOpcodeLine(output, "CMSG_TIME_SYNC_RESP_FAILED", 0x710A);
    DumpOpcodeLine(output, "CMSG_TOGGLE_PVP", 0x6815);
    DumpOpcodeLine(output, "CMSG_TOTEM_DESTROYED", 0x4207);
    DumpOpcodeLine(output, "CMSG_TRAINER_BUY_SPELL", 0x4415);
    DumpOpcodeLine(output, "CMSG_TRAINER_LIST", 0x2336);
    DumpOpcodeLine(output, "CMSG_TRANSMOGRIFY_ITEMS", 0x3B0E);
    DumpOpcodeLine(output, "CMSG_TURN_IN_PETITION", 0x0B27);
    DumpOpcodeLine(output, "CMSG_TUTORIAL_CLEAR", 0x6515);
    DumpOpcodeLine(output, "CMSG_TUTORIAL_FLAG", 0x6C26);
    DumpOpcodeLine(output, "CMSG_TUTORIAL_RESET", 0x2726);
    DumpOpcodeLine(output, "CMSG_UNACCEPT_TRADE", 0x391A);
    DumpOpcodeLine(output, "CMSG_UNLEARN_SKILL", 0x6106);
    DumpOpcodeLine(output, "CMSG_UNLEARN_SPECIALIZATION", 0x3210);
    DumpOpcodeLine(output, "CMSG_UNREGISTER_ALL_ADDON_PREFIXES", 0x3D54);
    DumpOpcodeLine(output, "CMSG_UPDATE_ACCOUNT_DATA", 0x4736);
    DumpOpcodeLine(output, "CMSG_UPDATE_MISSILE_TRAJECTORY", 0x781E);
    DumpOpcodeLine(output, "CMSG_UPDATE_PROJECTILE_POSITION", 0x0E24);
    DumpOpcodeLine(output, "CMSG_USED_FOLLOW", 0x7912);
    DumpOpcodeLine(output, "CMSG_USE_ITEM", 0x2C06);
    DumpOpcodeLine(output, "CMSG_VIOLENCE_LEVEL", 0x7816);
    DumpOpcodeLine(output, "CMSG_VOICE_SESSION_ENABLE", 0x2314);
    DumpOpcodeLine(output, "CMSG_VOID_STORAGE_QUERY", 0x790A);
    DumpOpcodeLine(output, "CMSG_VOID_STORAGE_TRANSFER", 0x380E);
    DumpOpcodeLine(output, "CMSG_VOID_STORAGE_UNLOCK", 0x7B14);
    DumpOpcodeLine(output, "CMSG_VOID_SWAP_ITEM", 0x3204);
    DumpOpcodeLine(output, "CMSG_WARDEN_DATA", 0x25A2);
    DumpOpcodeLine(output, "CMSG_WARGAME_ACCEPT", 0x2410);
    DumpOpcodeLine(output, "CMSG_WARGAME_START", 0x05A0);
    DumpOpcodeLine(output, "CMSG_WHO", 0x6C15);
    DumpOpcodeLine(output, "CMSG_WHOIS", 0x6B05);
    DumpOpcodeLine(output, "CMSG_WORLD_STATE_UI_TIMER_UPDATE", 0x4605);
    DumpOpcodeLine(output, "CMSG_WORLD_TELEPORT", 0x24B2);
    DumpOpcodeLine(output, "CMSG_WRAP_ITEM", 0x4F06);
    DumpOpcodeLine(output, "CMSG_ZONEUPDATE", 0x4F37);
    DumpOpcodeLine(output, "MSG_AUCTION_HELLO", 0x2307);
    DumpOpcodeLine(output, "MSG_CHANNEL_START", 0x0A15); // SMSG only?
    DumpOpcodeLine(output, "MSG_CHANNEL_UPDATE", 0x2417); // SMSG only?
    DumpOpcodeLine(output, "MSG_CORPSE_QUERY", 0x4336);
    DumpOpcodeLine(output, "MSG_INSPECT_ARENA_TEAMS", 0x2704);
    DumpOpcodeLine(output, "MSG_LIST_STABLED_PETS", 0x0834);
    DumpOpcodeLine(output, "MSG_MINIMAP_PING", 0x6635);
    DumpOpcodeLine(output, "MSG_MOVE_CHARM_TELEPORT_CHEAT", 0x7A08);
    DumpOpcodeLine(output, "MSG_MOVE_FALL_LAND", 0x380A);
    DumpOpcodeLine(output, "MSG_MOVE_HEARTBEAT", 0x3914);
    DumpOpcodeLine(output, "MSG_MOVE_JUMP", 0x7A06);
    DumpOpcodeLine(output, "MSG_MOVE_SET_FACING", 0x7914);
    DumpOpcodeLine(output, "MSG_MOVE_SET_PITCH", 0x7312);
    DumpOpcodeLine(output, "MSG_MOVE_SET_RUN_MODE", 0x791A);
    DumpOpcodeLine(output, "MSG_MOVE_SET_WALK_MODE", 0x7002);
    DumpOpcodeLine(output, "MSG_MOVE_START_ASCEND", 0x390A);
    DumpOpcodeLine(output, "MSG_MOVE_START_BACKWARD", 0x330A);
    DumpOpcodeLine(output, "MSG_MOVE_START_DESCEND", 0x3800);
    DumpOpcodeLine(output, "MSG_MOVE_START_FORWARD", 0x7814);
    DumpOpcodeLine(output, "MSG_MOVE_START_PITCH_DOWN", 0x3908);
    DumpOpcodeLine(output, "MSG_MOVE_START_PITCH_UP", 0x3304);
    DumpOpcodeLine(output, "MSG_MOVE_START_STRAFE_LEFT", 0x3A16);
    DumpOpcodeLine(output, "MSG_MOVE_START_STRAFE_RIGHT", 0x3A02);
    DumpOpcodeLine(output, "MSG_MOVE_START_SWIM", 0x3206);
    DumpOpcodeLine(output, "MSG_MOVE_START_TURN_LEFT", 0x700C);
    DumpOpcodeLine(output, "MSG_MOVE_START_TURN_RIGHT", 0x7000);
    DumpOpcodeLine(output, "MSG_MOVE_STOP", 0x320A);
    DumpOpcodeLine(output, "MSG_MOVE_STOP_ASCEND", 0x7B00);
    DumpOpcodeLine(output, "MSG_MOVE_STOP_PITCH", 0x7216);
    DumpOpcodeLine(output, "MSG_MOVE_STOP_STRAFE", 0x3002);
    DumpOpcodeLine(output, "MSG_MOVE_STOP_SWIM", 0x3802);
    DumpOpcodeLine(output, "MSG_MOVE_STOP_TURN", 0x331E);
    DumpOpcodeLine(output, "MSG_MOVE_TELEPORT", 0x55A0);
    DumpOpcodeLine(output, "MSG_MOVE_TELEPORT_ACK", 0x390C);
    DumpOpcodeLine(output, "MSG_MOVE_TELEPORT_CHEAT", 0x3A10);
    DumpOpcodeLine(output, "MSG_MOVE_TIME_SKIPPED", 0x19B3);
    DumpOpcodeLine(output, "MSG_MOVE_TOGGLE_COLLISION_CHEAT", 0x7B04);
    DumpOpcodeLine(output, "MSG_MOVE_WORLDPORT_ACK", 0x2411);
    DumpOpcodeLine(output, "MSG_NOTIFY_PARTY_SQUELCH", 0x4D06);
    DumpOpcodeLine(output, "MSG_PARTY_ASSIGNMENT", 0x0424);
    DumpOpcodeLine(output, "MSG_PETITION_DECLINE", 0x4905);
    DumpOpcodeLine(output, "MSG_PETITION_RENAME", 0x4005);
    DumpOpcodeLine(output, "MSG_QUERY_NEXT_MAIL_TIME", 0x0F04);
    DumpOpcodeLine(output, "MSG_QUEST_PUSH_RESULT", 0x4515);
    DumpOpcodeLine(output, "MSG_RAID_READY_CHECK", 0x2304);
    DumpOpcodeLine(output, "MSG_RAID_READY_CHECK_CONFIRM", 0x4F05);
    DumpOpcodeLine(output, "MSG_RAID_READY_CHECK_FINISHED", 0x2E15);
    DumpOpcodeLine(output, "MSG_RAID_TARGET_UPDATE", 0x2C36);
    DumpOpcodeLine(output, "MSG_RANDOM_ROLL", 0x0905);
    DumpOpcodeLine(output, "MSG_SAVE_GUILD_EMBLEM", 0x2404);
    DumpOpcodeLine(output, "MSG_SET_DUNGEON_DIFFICULTY", 0x4925);
    DumpOpcodeLine(output, "MSG_SET_RAID_DIFFICULTY", 0x0614);
    DumpOpcodeLine(output, "MSG_TABARDVENDOR_ACTIVATE", 0x6926);
    DumpOpcodeLine(output, "MSG_TALENT_WIPE_CONFIRM", 0x0107);
    DumpOpcodeLine(output, "MSG_VERIFY_CONNECTIVITY", 0x4F57);
    DumpOpcodeLine(output, "SMSG_ACCOUNT_DATA_TIMES", 0x4B05);
    DumpOpcodeLine(output, "SMSG_ACCOUNT_INFO_RESPONSE", 0x10A7);
    DumpOpcodeLine(output, "SMSG_ACCOUNT_RESTRICTED_WARNING", 0x51A7);
    DumpOpcodeLine(output, "SMSG_ACHIEVEMENT_DELETED", 0x6A16);
    DumpOpcodeLine(output, "SMSG_ACHIEVEMENT_EARNED", 0x4405);
    DumpOpcodeLine(output, "SMSG_ACTION_BUTTONS", 0x38B5);
    DumpOpcodeLine(output, "SMSG_ACTIVATETAXIREPLY", 0x6A37);
    DumpOpcodeLine(output, "SMSG_ADDON_INFO", 0x2C14);
    DumpOpcodeLine(output, "SMSG_ADD_RUNE_POWER", 0x6915);
    DumpOpcodeLine(output, "SMSG_AI_REACTION", 0x0637);
    DumpOpcodeLine(output, "SMSG_ALL_ACHIEVEMENT_DATA", 0x58B1);
    DumpOpcodeLine(output, "SMSG_AREA_SPIRIT_HEALER_TIME", 0x0734);
    DumpOpcodeLine(output, "SMSG_AREA_TRIGGER_MESSAGE", 0x4505);
    DumpOpcodeLine(output, "SMSG_AREA_TRIGGER_MOVEMENT_UPDATE", 0x3DB1);
    DumpOpcodeLine(output, "SMSG_ARENA_ERROR", 0x2D17);
    DumpOpcodeLine(output, "SMSG_ARENA_UNIT_DESTROYED", 0x2637);
    DumpOpcodeLine(output, "SMSG_ARENA_TEAM_CHANGE_FAILED_QUEUED", 0x6E34);
    DumpOpcodeLine(output, "SMSG_ARENA_TEAM_COMMAND_RESULT", 0x39B3);
    DumpOpcodeLine(output, "SMSG_ARENA_TEAM_EVENT", 0x0617);
    DumpOpcodeLine(output, "SMSG_ARENA_TEAM_INVITE", 0x0F36);
    DumpOpcodeLine(output, "SMSG_ARENA_TEAM_QUERY_RESPONSE", 0x6336);
    DumpOpcodeLine(output, "SMSG_ARENA_TEAM_ROSTER", 0x2717);
    DumpOpcodeLine(output, "SMSG_ARENA_TEAM_STATS", 0x4425);
    DumpOpcodeLine(output, "SMSG_ATTACKERSTATEUPDATE", 0x0B25);
    DumpOpcodeLine(output, "SMSG_ATTACKSTART", 0x2D15);
    DumpOpcodeLine(output, "SMSG_ATTACKSTOP", 0x0934);
    DumpOpcodeLine(output, "SMSG_ATTACKSWING_BADFACING", 0x0B36);
    DumpOpcodeLine(output, "SMSG_ATTACKSWING_CANT_ATTACK", 0x0016);
    DumpOpcodeLine(output, "SMSG_ATTACKSWING_DEADTARGET", 0x2B26);
    DumpOpcodeLine(output, "SMSG_ATTACKSWING_NOTINRANGE", 0x6C07);
    DumpOpcodeLine(output, "SMSG_AUCTION_BIDDER_LIST_RESULT", 0x0027);
    DumpOpcodeLine(output, "SMSG_AUCTION_BIDDER_NOTIFICATION", 0x4E27);
    DumpOpcodeLine(output, "SMSG_AUCTION_COMMAND_RESULT", 0x4C25);
    DumpOpcodeLine(output, "SMSG_AUCTION_LIST_PENDING_SALES", 0x6A27);
    DumpOpcodeLine(output, "SMSG_AUCTION_LIST_RESULT", 0x6637);
    DumpOpcodeLine(output, "SMSG_AUCTION_OWNER_LIST_RESULT", 0x6C34);
    DumpOpcodeLine(output, "SMSG_AUCTION_OWNER_NOTIFICATION", 0x4116);
    DumpOpcodeLine(output, "SMSG_AUCTION_REMOVED_NOTIFICATION", 0x2334);
    DumpOpcodeLine(output, "SMSG_AURA_POINTS_DEPLETED", 0x7CB7);
    DumpOpcodeLine(output, "SMSG_AURA_UPDATE", 0x4707);
    DumpOpcodeLine(output, "SMSG_AURA_UPDATE_ALL", 0x6916);
    DumpOpcodeLine(output, "SMSG_AUTH_CHALLENGE", 0x4542);
    DumpOpcodeLine(output, "SMSG_AUTH_RESPONSE", 0x5DB6);
    DumpOpcodeLine(output, "SMSG_AVAILABLE_VOICE_CHANNEL", 0x2E16);
    DumpOpcodeLine(output, "SMSG_AVERAGE_ITEM_LEVEL_INFORM", 0x5DA7);
    DumpOpcodeLine(output, "SMSG_BARBER_SHOP_RESULT", 0x6125);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_LIST", 0x71B5);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_MGR_EJECTED", 0x7DB7);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_MGR_EJECT_PENDING", 0x34A2);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_MGR_ENTERED", 0x5CA0);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_MGR_ENTRY_INVITE", 0x34B3);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_MGR_EXIT_REQUEST", 0x51B1);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_MGR_QUEUE_INVITE", 0x15A6);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_MGR_QUEUE_REQUEST_RESPONSE", 0x79B6);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_MGR_STATE_CHANGE", 0x35B4);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_PLAYER_POSITIONS", 0x58B4);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_PORT_DENIED", 0x35A3);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_RATED_INFO", 0x54A3);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_STATUS", 0x7DA1);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_STATUS_QUEUED", 0x35A1);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_STATUS_ACTIVE", 0x74A4);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION", 0x59A0);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_STATUS_WAITFORGROUPS", 0x75A2);
    DumpOpcodeLine(output, "SMSG_BATTLEFIELD_STATUS_FAILED", 0x71A7);
    DumpOpcodeLine(output, "SMSG_BATTLEGROUND_INFO_THROTTLED", 0x34B2);
    DumpOpcodeLine(output, "SMSG_BATTLEGROUND_PLAYER_JOINED", 0x50B0);
    DumpOpcodeLine(output, "SMSG_BATTLEGROUND_PLAYER_LEFT", 0x59A6);
    DumpOpcodeLine(output, "SMSG_BINDER_CONFIRM", 0x2835);
    DumpOpcodeLine(output, "SMSG_BINDPOINTUPDATE", 0x0527);
    DumpOpcodeLine(output, "SMSG_BREAK_TARGET", 0x0105);
    DumpOpcodeLine(output, "SMSG_BUY_FAILED", 0x6435);
    DumpOpcodeLine(output, "SMSG_BUY_ITEM", 0x0F26);
    DumpOpcodeLine(output, "SMSG_CALENDAR_ARENA_TEAM", 0x0615);
    DumpOpcodeLine(output, "SMSG_CALENDAR_CLEAR_PENDING_ACTION", 0x2106);
    DumpOpcodeLine(output, "SMSG_CALENDAR_COMMAND_RESULT", 0x6F36);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_INVITE", 0x4E16);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_INVITE_ALERT", 0x2A05);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_INVITE_NOTES", 0x0E17);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_INVITE_NOTES_ALERT", 0x2535);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_INVITE_REMOVED", 0x0725);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_INVITE_REMOVED_ALERT", 0x2617);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_INVITE_STATUS_ALERT", 0x6625);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_MODERATOR_STATUS_ALERT", 0x6B06);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_REMOVED_ALERT", 0x6D35);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_STATUS", 0x2A27);
    DumpOpcodeLine(output, "SMSG_CALENDAR_EVENT_UPDATED_ALERT", 0x0907);
    DumpOpcodeLine(output, "SMSG_CALENDAR_FILTER_GUILD", 0x4A26);
    DumpOpcodeLine(output, "SMSG_CALENDAR_RAID_LOCKOUT_ADDED", 0x2305);
    DumpOpcodeLine(output, "SMSG_CALENDAR_RAID_LOCKOUT_REMOVED", 0x2E25);
    DumpOpcodeLine(output, "SMSG_CALENDAR_RAID_LOCKOUT_UPDATED", 0x4636);
    DumpOpcodeLine(output, "SMSG_CALENDAR_SEND_CALENDAR", 0x6805);
    DumpOpcodeLine(output, "SMSG_CALENDAR_SEND_EVENT", 0x0C35);
    DumpOpcodeLine(output, "SMSG_CALENDAR_SEND_NUM_PENDING", 0x0C17);
    DumpOpcodeLine(output, "SMSG_CAMERA_SHAKE", 0x4214);
    DumpOpcodeLine(output, "SMSG_CANCEL_AUTO_REPEAT", 0x6436);
    DumpOpcodeLine(output, "SMSG_CANCEL_COMBAT", 0x4F04);
    DumpOpcodeLine(output, "SMSG_CAST_FAILED", 0x4D16);
    DumpOpcodeLine(output, "SMSG_CHANNEL_LIST", 0x2214);
    DumpOpcodeLine(output, "SMSG_CHANNEL_MEMBER_COUNT", 0x6414);
    DumpOpcodeLine(output, "SMSG_CHANNEL_NOTIFY", 0x0825);
    DumpOpcodeLine(output, "SMSG_CHARACTER_LOGIN_FAILED", 0x4417);
    DumpOpcodeLine(output, "SMSG_CHARACTER_CREATE", 0x2D05);
    DumpOpcodeLine(output, "SMSG_CHARACTER_CUSTOMIZE", 0x4F16);
    DumpOpcodeLine(output, "SMSG_CHARACTER_DELETE", 0x0304);
    DumpOpcodeLine(output, "SMSG_CHARACTER_ENUM", 0x10B0);
    DumpOpcodeLine(output, "SMSG_CHARACTER_FACTION_CHANGE", 0x4C06);
    DumpOpcodeLine(output, "SMSG_CHARACTER_RENAME", 0x2024);
    DumpOpcodeLine(output, "SMSG_CHAT_IGNORED_ACCOUNT_MUTED", 0x15A4);
    DumpOpcodeLine(output, "SMSG_CHAT_NOT_IN_PARTY", 0x6A14);
    DumpOpcodeLine(output, "SMSG_CHAT_PLAYER_AMBIGUOUS", 0x2F34);
    DumpOpcodeLine(output, "SMSG_CHAT_PLAYER_NOT_FOUND", 0x2526);
    DumpOpcodeLine(output, "SMSG_CHAT_RESTRICTED", 0x6536);
    DumpOpcodeLine(output, "SMSG_CHAT_SERVER_DISCONNECTED", 0x6D34);
    DumpOpcodeLine(output, "SMSG_CHAT_SERVER_RECONNECTED", 0x6905);
    DumpOpcodeLine(output, "SMSG_CHAT_WRONG_FACTION", 0x6724);
    DumpOpcodeLine(output, "SMSG_CLEAR_BOSS_EMOTES", 0x19A3);
    DumpOpcodeLine(output, "SMSG_CLEAR_COOLDOWN", 0x0627);
    DumpOpcodeLine(output, "SMSG_CLEAR_COOLDOWNS", 0x59B4);
    DumpOpcodeLine(output, "SMSG_CLEAR_FAR_SIGHT_IMMEDIATE", 0x2A04);
    DumpOpcodeLine(output, "SMSG_CLEAR_TARGET", 0x4B26);
    DumpOpcodeLine(output, "SMSG_CLIENTCACHE_VERSION", 0x2734);
    DumpOpcodeLine(output, "SMSG_CLIENT_CONTROL_UPDATE", 0x2837);
    DumpOpcodeLine(output, "SMSG_COMBAT_EVENT_FAILED", 0x2B07);
    DumpOpcodeLine(output, "SMSG_COMMENTATOR_MAP_INFO", 0x0327);
    DumpOpcodeLine(output, "SMSG_COMMENTATOR_PARTY_INFO", 0x38B0);
    DumpOpcodeLine(output, "SMSG_COMMENTATOR_PLAYER_INFO", 0x2F36);
    DumpOpcodeLine(output, "SMSG_COMMENTATOR_SKIRMISH_QUEUE_RESULT1", 0x2126);
    DumpOpcodeLine(output, "SMSG_COMMENTATOR_SKIRMISH_QUEUE_RESULT2", 0x6814);
    DumpOpcodeLine(output, "SMSG_COMMENTATOR_STATE_CHANGED", 0x0737);
    DumpOpcodeLine(output, "SMSG_COMPLAIN_RESULT", 0x6D24);
    DumpOpcodeLine(output, "SMSG_COMPRESSED_MOVES", 0x0517);
    DumpOpcodeLine(output, "SMSG_COMSAT_CONNECT_FAIL", 0x6317);
    DumpOpcodeLine(output, "SMSG_COMSAT_DISCONNECT", 0x0316);
    DumpOpcodeLine(output, "SMSG_COMSAT_RECONNECT_TRY", 0x4D35);
    DumpOpcodeLine(output, "SMSG_CONTACT_LIST", 0x6017);
    DumpOpcodeLine(output, "SMSG_CONVERT_RUNE", 0x4F14);
    DumpOpcodeLine(output, "SMSG_COOLDOWN_CHEAT", 0x4537);
    DumpOpcodeLine(output, "SMSG_COOLDOWN_EVENT", 0x4F26);
    DumpOpcodeLine(output, "SMSG_CORPSE_MAP_POSITION_QUERY_RESPONSE", 0x0E35);
    DumpOpcodeLine(output, "SMSG_CORPSE_NOT_IN_INSTANCE", 0x2A14);
    DumpOpcodeLine(output, "SMSG_CORPSE_RECLAIM_DELAY", 0x0D34);
    DumpOpcodeLine(output, "SMSG_CREATURE_QUERY_RESPONSE", 0x6024);
    DumpOpcodeLine(output, "SMSG_CRITERIA_DELETED", 0x2915);
    DumpOpcodeLine(output, "SMSG_CRITERIA_UPDATE", 0x6E37);
    DumpOpcodeLine(output, "SMSG_CROSSED_INEBRIATION_THRESHOLD", 0x2036);
    DumpOpcodeLine(output, "SMSG_CURRENCY_LOOT_REMOVED", 0x1DB4);
    DumpOpcodeLine(output, "SMSG_CURRENCY_LOOT_RESTORED", 0x30A0);
    DumpOpcodeLine(output, "SMSG_CUSTOM_LOAD_SCREEN", 0x1DB6);
    DumpOpcodeLine(output, "SMSG_DAMAGE_CALC_LOG", 0x2436);
    DumpOpcodeLine(output, "SMSG_DANCE_QUERY_RESPONSE", 0x2F06);
    DumpOpcodeLine(output, "SMSG_DB_REPLY", 0x38A4);
    DumpOpcodeLine(output, "SMSG_DEATH_RELEASE_LOC", 0x2F07);
    DumpOpcodeLine(output, "SMSG_DEBUG_RUNE_REGEN", 0x31B3);
    DumpOpcodeLine(output, "SMSG_DEFENSE_MESSAGE", 0x0314);
    DumpOpcodeLine(output, "SMSG_DESTROY_OBJECT", 0x4724);
    DumpOpcodeLine(output, "SMSG_DESTRUCTIBLE_BUILDING_DAMAGE", 0x4825);
    DumpOpcodeLine(output, "SMSG_DIFFERENT_INSTANCE_FROM_PARTY", 0x15B1);
    DumpOpcodeLine(output, "SMSG_DISENCHANT_CREDIT", 0x55A2);
    DumpOpcodeLine(output, "SMSG_DISMOUNT", 0x2135);
    DumpOpcodeLine(output, "SMSG_DISMOUNTRESULT", 0x0D25);
    DumpOpcodeLine(output, "SMSG_DISPEL_FAILED", 0x0307);
    DumpOpcodeLine(output, "SMSG_DISPLAY_GAME_ERROR", 0x31A6);
    DumpOpcodeLine(output, "SMSG_DONT_AUTO_PUSH_SPELLS_TO_ACTION_BAR", 0x38A2);
    DumpOpcodeLine(output, "SMSG_DROP_NEW_CONNECTION", 0x4D40);
    DumpOpcodeLine(output, "SMSG_DUEL_COMPLETE", 0x2527);
    DumpOpcodeLine(output, "SMSG_DUEL_COUNTDOWN", 0x4836);
    DumpOpcodeLine(output, "SMSG_DUEL_INBOUNDS", 0x0A27);
    DumpOpcodeLine(output, "SMSG_DUEL_OUTOFBOUNDS", 0x0C26);
    DumpOpcodeLine(output, "SMSG_DUEL_REQUESTED", 0x4504);
    DumpOpcodeLine(output, "SMSG_DUEL_WINNER", 0x2D36);
    DumpOpcodeLine(output, "SMSG_DUMP_RIDE_TICKETS_RESPONSE", 0x11A3);
    DumpOpcodeLine(output, "SMSG_DURABILITY_DAMAGE_DEATH", 0x4C27);
    DumpOpcodeLine(output, "SMSG_ECHO_PARTY_SQUELCH", 0x0814);
    DumpOpcodeLine(output, "SMSG_EMOTE", 0x0A34);
    DumpOpcodeLine(output, "SMSG_ENABLE_BARBER_SHOP", 0x2D16);
    DumpOpcodeLine(output, "SMSG_ENCHANTMENTLOG", 0x6035);
    DumpOpcodeLine(output, "SMSG_ENVIRONMENTALDAMAGELOG", 0x6C05);
    DumpOpcodeLine(output, "SMSG_EQUIPMENT_SET_LIST", 0x2E04);
    DumpOpcodeLine(output, "SMSG_EQUIPMENT_SET_SAVED", 0x2216);
    DumpOpcodeLine(output, "SMSG_EQUIPMENT_SET_USE_RESULT", 0x2424);
    DumpOpcodeLine(output, "SMSG_EXPECTED_SPAM_RECORDS", 0x4D36);
    DumpOpcodeLine(output, "SMSG_EXPLORATION_EXPERIENCE", 0x6716);
    DumpOpcodeLine(output, "SMSG_FAILED_PLAYER_CONDITION", 0x19A4);
    DumpOpcodeLine(output, "SMSG_FEATURE_SYSTEM_STATUS", 0x3DB7);
    DumpOpcodeLine(output, "SMSG_FEIGN_DEATH_RESISTED", 0x0D05);
    DumpOpcodeLine(output, "SMSG_FISH_ESCAPED", 0x2205);
    DumpOpcodeLine(output, "SMSG_FISH_NOT_HOOKED", 0x0A17);
    DumpOpcodeLine(output, "SMSG_FLIGHT_SPLINE_SYNC", 0x0924);
    DumpOpcodeLine(output, "SMSG_FLOOD_DETECTED", 0x0542);
    DumpOpcodeLine(output, "SMSG_FORCED_DEATH_UPDATE", 0x2606);
    DumpOpcodeLine(output, "SMSG_FORCE_SEND_QUEUED_PACKETS", 0x0140);
    DumpOpcodeLine(output, "SMSG_FORCE_SET_VEHICLE_REC_ID", 0x70A1);
    DumpOpcodeLine(output, "SMSG_FORGE_MASTER_SET", 0x70B7);
    DumpOpcodeLine(output, "SMSG_FRIEND_STATUS", 0x0717);
    DumpOpcodeLine(output, "SMSG_GAMEOBJECT_CUSTOM_ANIM", 0x4936);
    DumpOpcodeLine(output, "SMSG_GAMEOBJECT_DESPAWN_ANIM", 0x6735);
    DumpOpcodeLine(output, "SMSG_GAMEOBJECT_PAGETEXT", 0x2925);
    DumpOpcodeLine(output, "SMSG_GAMEOBJECT_QUERY_RESPONSE", 0x0915);
    DumpOpcodeLine(output, "SMSG_GAMEOBJECT_RESET_STATE", 0x2A16);
    DumpOpcodeLine(output, "SMSG_GAMESPEED_SET", 0x4E34);
    DumpOpcodeLine(output, "SMSG_GAMETIME_SET", 0x0014);
    DumpOpcodeLine(output, "SMSG_GAMETIME_UPDATE", 0x4127);
    DumpOpcodeLine(output, "SMSG_GAME_EVENT_DEBUG_LOG", 0x31A7);
    DumpOpcodeLine(output, "SMSG_GAME_OBJECT_ACTIVATE_ANIM_KIT", 0x14A3);
    DumpOpcodeLine(output, "SMSG_GODMODE", 0x0405);
    DumpOpcodeLine(output, "SMSG_GOSSIP_COMPLETE", 0x0806);
    DumpOpcodeLine(output, "SMSG_GOSSIP_MESSAGE", 0x2035);
    DumpOpcodeLine(output, "SMSG_GOSSIP_POI", 0x4316);
    DumpOpcodeLine(output, "SMSG_GROUPACTION_THROTTLED", 0x6524);
    DumpOpcodeLine(output, "SMSG_GROUP_CANCEL", 0x4D25);
    DumpOpcodeLine(output, "SMSG_GROUP_DECLINE", 0x6835);
    DumpOpcodeLine(output, "SMSG_GROUP_DESTROYED", 0x2207);
    DumpOpcodeLine(output, "SMSG_GROUP_INVITE", 0x31B2);
    DumpOpcodeLine(output, "SMSG_GROUP_LIST", 0x4C24);
    DumpOpcodeLine(output, "SMSG_GROUP_SET_LEADER", 0x0526);
    DumpOpcodeLine(output, "SMSG_GROUP_SET_ROLE", 0x39A6);
    DumpOpcodeLine(output, "SMSG_GROUP_UNINVITE", 0x0A07);
    DumpOpcodeLine(output, "SMSG_GUILD_ACHIEVEMENT_DATA", 0x54B7);
    DumpOpcodeLine(output, "SMSG_GUILD_ACHIEVEMENT_DELETED", 0x35A0);
    DumpOpcodeLine(output, "SMSG_GUILD_ACHIEVEMENT_EARNED", 0x50B5);
    DumpOpcodeLine(output, "SMSG_GUILD_ACHIEVEMENT_MEMBERS", 0x38A5);
    DumpOpcodeLine(output, "SMSG_GUILD_BANK_LIST", 0x78A5);
    DumpOpcodeLine(output, "SMSG_GUILD_BANK_LOG_QUERY_RESULT", 0x30B2);
    DumpOpcodeLine(output, "SMSG_GUILD_BANK_MONEY_WITHDRAWN", 0x5DB4);
    DumpOpcodeLine(output, "SMSG_GUILD_BANK_QUERY_TEXT_RESULT", 0x75A3);
    DumpOpcodeLine(output, "SMSG_GUILD_CHALLENGE_COMPLETED", 0x39A3);
    DumpOpcodeLine(output, "SMSG_GUILD_CHALLENGE_UPDATED", 0x18B1);
    DumpOpcodeLine(output, "SMSG_GUILD_CHANGE_NAME_RESULT", 0x3CB1);
    DumpOpcodeLine(output, "SMSG_GUILD_COMMAND_RESULT", 0x7DB3);
    DumpOpcodeLine(output, "SMSG_GUILD_COMMAND_RESULT_2", 0x2707);
    DumpOpcodeLine(output, "SMSG_GUILD_CRITERIA_DATA", 0x14B4);
    DumpOpcodeLine(output, "SMSG_GUILD_CRITERIA_DELETED", 0x55B1);
    DumpOpcodeLine(output, "SMSG_GUILD_DECLINE", 0x2C07);
    DumpOpcodeLine(output, "SMSG_GUILD_EVENT", 0x0705);
    DumpOpcodeLine(output, "SMSG_GUILD_EVENT_LOG_QUERY_RESULT", 0x10B2);
    DumpOpcodeLine(output, "SMSG_GUILD_FLAGGED_FOR_RENAME", 0x30B6);
    DumpOpcodeLine(output, "SMSG_GUILD_INVITE", 0x14A2);
    DumpOpcodeLine(output, "SMSG_GUILD_INVITE_CANCEL", 0x0606);
    DumpOpcodeLine(output, "SMSG_GUILD_MAX_DAILY_XP", 0x79B5);
    DumpOpcodeLine(output, "SMSG_GUILD_MEMBERS_FOR_RECIPE", 0x1CB7);
    DumpOpcodeLine(output, "SMSG_GUILD_MEMBER_DAILY_RESET", 0x10A5);
    DumpOpcodeLine(output, "SMSG_GUILD_MEMBER_RECIPES", 0x1CB0);
    DumpOpcodeLine(output, "SMSG_GUILD_MEMBER_UPDATE_NOTE", 0x7CA0);
    DumpOpcodeLine(output, "SMSG_GUILD_MOVE_COMPLETE", 0x11B2);
    DumpOpcodeLine(output, "SMSG_GUILD_MOVE_STARTING", 0x70A4);
    DumpOpcodeLine(output, "SMSG_GUILD_NEWS_DELETED", 0x74A7);
    DumpOpcodeLine(output, "SMSG_GUILD_NEWS_UPDATE", 0x35A7);
    DumpOpcodeLine(output, "SMSG_GUILD_PARTY_STATE_RESPONSE", 0x50A6);
    DumpOpcodeLine(output, "SMSG_GUILD_PERMISSIONS_QUERY_RESULTS", 0x34A3);
    DumpOpcodeLine(output, "SMSG_GUILD_QUERY_RESPONSE", 0x0E06);
    DumpOpcodeLine(output, "SMSG_GUILD_RANK", 0x30B4);
    DumpOpcodeLine(output, "SMSG_GUILD_RANKS_UPDATE", 0x5DA0);
    DumpOpcodeLine(output, "SMSG_GUILD_RECIPES", 0x10B3);
    DumpOpcodeLine(output, "SMSG_GUILD_RENAMED", 0x74A6);
    DumpOpcodeLine(output, "SMSG_GUILD_REPUTATION_REACTION_CHANGED", 0x74B0);
    DumpOpcodeLine(output, "SMSG_GUILD_REPUTATION_WEEKLY_CAP", 0x30B7);
    DumpOpcodeLine(output, "SMSG_GUILD_RESET", 0x1CB5);
    DumpOpcodeLine(output, "SMSG_GUILD_REWARDS_LIST", 0x1DB0);
    DumpOpcodeLine(output, "SMSG_GUILD_ROSTER", 0x3DA3);
    DumpOpcodeLine(output, "SMSG_GUILD_UPDATE_ROSTER", 0x18B0);
    DumpOpcodeLine(output, "SMSG_GUILD_XP", 0x3DB0);
    DumpOpcodeLine(output, "SMSG_GUILD_XP_GAIN", 0x14A1);
    DumpOpcodeLine(output, "SMSG_HEALTH_UPDATE", 0x4734);
    DumpOpcodeLine(output, "SMSG_HIGHEST_THREAT_UPDATE", 0x4104);
    DumpOpcodeLine(output, "SMSG_HOTFIX_INFO", 0x19B5);
    DumpOpcodeLine(output, "SMSG_HOTFIX_NOTIFY", 0x55A7);
    DumpOpcodeLine(output, "SMSG_INITIALIZE_FACTIONS", 0x4634);
    DumpOpcodeLine(output, "SMSG_INITIAL_SPELLS", 0x0104);
    DumpOpcodeLine(output, "SMSG_INIT_CURRENCY", 0x15A5);
    DumpOpcodeLine(output, "SMSG_INIT_WORLD_STATES", 0x4C15);
    DumpOpcodeLine(output, "SMSG_INSPECT_HONOR_STATS", 0x79A5);
    DumpOpcodeLine(output, "SMSG_INSPECT_RATED_BG_STATS", 0x19A5);
    DumpOpcodeLine(output, "SMSG_INSPECT_RESULTS_UPDATE", 0x0C14);
    DumpOpcodeLine(output, "SMSG_INSPECT_TALENT", 0x4014);
    DumpOpcodeLine(output, "SMSG_INSTANCE_LOCK_WARNING_QUERY", 0x4F17);
    DumpOpcodeLine(output, "SMSG_INSTANCE_RESET", 0x6F05);
    DumpOpcodeLine(output, "SMSG_INSTANCE_RESET_FAILED", 0x4725);
    DumpOpcodeLine(output, "SMSG_INSTANCE_SAVE_CREATED", 0x0124);
    DumpOpcodeLine(output, "SMSG_INVALIDATE_DANCE", 0x0E27);
    DumpOpcodeLine(output, "SMSG_INVALIDATE_PLAYER", 0x6325);
    DumpOpcodeLine(output, "SMSG_INVALID_PROMOTION_CODE", 0x6F25);
    DumpOpcodeLine(output, "SMSG_INVENTORY_CHANGE_FAILURE", 0x2236);
    DumpOpcodeLine(output, "SMSG_ITEM_ADD_PASSIVE", 0x7CB4);
    DumpOpcodeLine(output, "SMSG_ITEM_COOLDOWN", 0x4D14);
    DumpOpcodeLine(output, "SMSG_ITEM_ENCHANT_TIME_UPDATE", 0x0F27);
    DumpOpcodeLine(output, "SMSG_ITEM_EXPIRE_PURCHASE_REFUND", 0x1CA0);
    DumpOpcodeLine(output, "SMSG_ITEM_PUSH_RESULT", 0x0E15);
    DumpOpcodeLine(output, "SMSG_ITEM_REFUND_INFO_RESPONSE", 0x15A3);
    DumpOpcodeLine(output, "SMSG_ITEM_REFUND_RESULT", 0x5DB1);
    DumpOpcodeLine(output, "SMSG_ITEM_REMOVE_PASSIVE", 0x39A1);
    DumpOpcodeLine(output, "SMSG_ITEM_SEND_PASSIVE", 0x70B1);
    DumpOpcodeLine(output, "SMSG_ITEM_TEXT_QUERY_RESPONSE", 0x2725);
    DumpOpcodeLine(output, "SMSG_ITEM_TIME_UPDATE", 0x2407);
    DumpOpcodeLine(output, "SMSG_LEARNED_DANCE_MOVES", 0x0E05);
    DumpOpcodeLine(output, "SMSG_LEARNED_SPELL", 0x58A2);
    DumpOpcodeLine(output, "SMSG_LEVELUP_INFO", 0x0435);
    DumpOpcodeLine(output, "SMSG_LFG_BOOT_PROPOSAL_UPDATE", 0x0F05);
    DumpOpcodeLine(output, "SMSG_LFG_DISABLED", 0x0815);
    DumpOpcodeLine(output, "SMSG_LFG_JOIN_RESULT", 0x38B6);
    DumpOpcodeLine(output, "SMSG_LFG_OFFER_CONTINUE", 0x6B27);
    DumpOpcodeLine(output, "SMSG_LFG_PARTY_INFO", 0x2325);
    DumpOpcodeLine(output, "SMSG_LFG_PLAYER_INFO", 0x4B36);
    DumpOpcodeLine(output, "SMSG_LFG_PLAYER_REWARD", 0x6834);
    DumpOpcodeLine(output, "SMSG_LFG_PROPOSAL_UPDATE", 0x7DA6);
    DumpOpcodeLine(output, "SMSG_LFG_QUEUE_STATUS", 0x78B4);
    DumpOpcodeLine(output, "SMSG_LFG_ROLE_CHECK_UPDATE", 0x0336);
    DumpOpcodeLine(output, "SMSG_LFG_ROLE_CHOSEN", 0x6A26);
    DumpOpcodeLine(output, "SMSG_LFG_SLOT_INVALID", 0x54B5);
    DumpOpcodeLine(output, "SMSG_LFG_TELEPORT_DENIED", 0x0E14);
    DumpOpcodeLine(output, "SMSG_LFG_UPDATE_SEARCH", 0x54A1);
    DumpOpcodeLine(output, "SMSG_LFG_UPDATE_STATUS", 0x31A4);
    DumpOpcodeLine(output, "SMSG_LFG_UPDATE_STATUS_NONE", 0x7CA1);
    DumpOpcodeLine(output, "SMSG_LF_GUILD_APPLICANT_LIST_UPDATED", 0x10A4);
    DumpOpcodeLine(output, "SMSG_LF_GUILD_APPLICATIONS_LIST_CHANGED", 0x71A5);
    DumpOpcodeLine(output, "SMSG_LF_GUILD_BROWSE_UPDATED", 0x1DA3);
    DumpOpcodeLine(output, "SMSG_LF_GUILD_COMMAND_RESULT", 0x54A6);
    DumpOpcodeLine(output, "SMSG_LF_GUILD_MEMBERSHIP_LIST_UPDATED", 0x1CA5);
    DumpOpcodeLine(output, "SMSG_LF_GUILD_POST_UPDATED", 0x35B7);
    DumpOpcodeLine(output, "SMSG_LF_GUILD_RECRUIT_LIST_UPDATED", 0x1CB2);
    DumpOpcodeLine(output, "SMSG_LIST_INVENTORY", 0x7CB0);
    DumpOpcodeLine(output, "SMSG_LOAD_CUF_PROFILES", 0x50B1);
    DumpOpcodeLine(output, "SMSG_LOGIN_SETTIMESPEED", 0x4D15);
    DumpOpcodeLine(output, "SMSG_LOGIN_VERIFY_WORLD", 0x2005);
    DumpOpcodeLine(output, "SMSG_LOGOUT_CANCEL_ACK", 0x6514);
    DumpOpcodeLine(output, "SMSG_LOGOUT_COMPLETE", 0x2137);
    DumpOpcodeLine(output, "SMSG_LOGOUT_RESPONSE", 0x0524);
    DumpOpcodeLine(output, "SMSG_LOG_XPGAIN", 0x4514);
    DumpOpcodeLine(output, "SMSG_LOOT_ALL_PASSED", 0x6237);
    DumpOpcodeLine(output, "SMSG_LOOT_CLEAR_MONEY", 0x2B37);
    DumpOpcodeLine(output, "SMSG_LOOT_CONTENTS", 0x11B1);
    DumpOpcodeLine(output, "SMSG_LOOT_ITEM_NOTIFY", 0x6D15);
    DumpOpcodeLine(output, "SMSG_LOOT_LIST", 0x6807);
    DumpOpcodeLine(output, "SMSG_LOOT_MASTER_LIST", 0x0325);
    DumpOpcodeLine(output, "SMSG_LOOT_MONEY_NOTIFY", 0x2836);
    DumpOpcodeLine(output, "SMSG_LOOT_RELEASE_RESPONSE", 0x6D25);
    DumpOpcodeLine(output, "SMSG_LOOT_REMOVED", 0x6817);
    DumpOpcodeLine(output, "SMSG_LOOT_RESPONSE", 0x4C16);
    DumpOpcodeLine(output, "SMSG_LOOT_ROLL", 0x6507);
    DumpOpcodeLine(output, "SMSG_LOOT_ROLL_WON", 0x6617);
    DumpOpcodeLine(output, "SMSG_LOOT_SLOT_CHANGED", 0x2935);
    DumpOpcodeLine(output, "SMSG_LOOT_START_ROLL", 0x2227);
    DumpOpcodeLine(output, "SMSG_MAIL_LIST_RESULT", 0x4217);
    DumpOpcodeLine(output, "SMSG_MAP_OBJ_EVENTS", 0x54B2);
    DumpOpcodeLine(output, "SMSG_MEETINGSTONE_IN_PROGRESS", 0x2D35);
    DumpOpcodeLine(output, "SMSG_MESSAGECHAT", 0x2026);
    DumpOpcodeLine(output, "SMSG_MESSAGE_BOX", 0x30A1);
    DumpOpcodeLine(output, "SMSG_MINIGAME_SETUP", 0x6727);
    DumpOpcodeLine(output, "SMSG_MINIGAME_STATE", 0x2E17);
    DumpOpcodeLine(output, "SMSG_MIRRORIMAGE_DATA", 0x2634);
    DumpOpcodeLine(output, "SMSG_MISSILE_CANCEL", 0x3DB4);
    DumpOpcodeLine(output, "SMSG_MODIFY_COOLDOWN", 0x6016);
    DumpOpcodeLine(output, "SMSG_MONEY_NOTIFY", 0x55B6);
    DumpOpcodeLine(output, "SMSG_MONSTER_MOVE", 0x6E17);
    DumpOpcodeLine(output, "SMSG_MONSTER_MOVE_TRANSPORT", 0x2004);
    DumpOpcodeLine(output, "SMSG_MOTD", 0x0A35);
    DumpOpcodeLine(output, "SMSG_MOUNTRESULT", 0x2225);
    DumpOpcodeLine(output, "SMSG_MOUNTSPECIAL_ANIM", 0x0217);
    DumpOpcodeLine(output, "SMSG_MOVE_COLLISION_DISABLE", 0x31B0);
    DumpOpcodeLine(output, "SMSG_MOVE_COLLISION_ENABLE", 0x11A7);
    DumpOpcodeLine(output, "SMSG_MOVE_FEATHER_FALL", 0x79B0);
    DumpOpcodeLine(output, "SMSG_MOVE_GRAVITY_DISABLE", 0x75B2);
    DumpOpcodeLine(output, "SMSG_MOVE_GRAVITY_ENABLE", 0x30B3);
    DumpOpcodeLine(output, "SMSG_MOVE_KNOCK_BACK", 0x5CB4);
    DumpOpcodeLine(output, "SMSG_MOVE_LAND_WALK", 0x34B7);
    DumpOpcodeLine(output, "SMSG_MOVE_NORMAL_FALL", 0x51B6);
    DumpOpcodeLine(output, "SMSG_MOVE_ROOT", 0x7DA0);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_ACTIVE_MOVER", 0x11B3);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_CAN_FLY", 0x3DA1);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY", 0x59A2);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_COLLISION_HEIGHT", 0x11B0);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_COMPOUND_STATE", 0x75A0);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_FLIGHT_BACK_SPEED", 0x30A2);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_FLIGHT_SPEED", 0x71A6);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_HOVER", 0x5CB3);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_PITCH_RATE", 0x75B0);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_RUN_BACK_SPEED", 0x71B1);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_RUN_SPEED", 0x3DB5);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_SWIM_BACK_SPEED", 0x5CA6);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_SWIM_SPEED", 0x15A7);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_TURN_RATE", 0x30A5);
    DumpOpcodeLine(output, "SMSG_MOVE_SET_WALK_SPEED", 0x1DA4);
    DumpOpcodeLine(output, "SMSG_MOVE_UNROOT", 0x7DB4);
    DumpOpcodeLine(output, "SMSG_MOVE_UNSET_CAN_FLY", 0x15A2);
    DumpOpcodeLine(output, "SMSG_MOVE_UNSET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY", 0x7DB2);
    DumpOpcodeLine(output, "SMSG_MOVE_UNSET_HOVER", 0x51B3);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_COLLISION_HEIGHT", 0x59A3);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_FLIGHT_BACK_SPEED", 0x74A0);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_FLIGHT_SPEED", 0x30B1);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_KNOCK_BACK", 0x3DB2);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_PITCH_RATE", 0x1DB5);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_RUN_BACK_SPEED", 0x3DA6);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_RUN_SPEED", 0x14A6);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_SWIM_BACK_SPEED", 0x30B5);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_SWIM_SPEED", 0x59B5);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_TELEPORT", 0x50B2);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_TURN_RATE", 0x5DA1);
    DumpOpcodeLine(output, "SMSG_MOVE_UPDATE_WALK_SPEED", 0x54A2);
    DumpOpcodeLine(output, "SMSG_MOVE_WATER_WALK", 0x75B1);
    DumpOpcodeLine(output, "SMSG_MULTIPLE_PACKETS", 0x6736);
    DumpOpcodeLine(output, "SMSG_NAME_QUERY_RESPONSE", 0x6E04);
    DumpOpcodeLine(output, "SMSG_NEW_TAXI_PATH", 0x4B35);
    DumpOpcodeLine(output, "SMSG_NEW_WORLD", 0x79B1);
    DumpOpcodeLine(output, "SMSG_NEW_WORLD_ABORT", 0x14B7);
    DumpOpcodeLine(output, "SMSG_NOTIFICATION", 0x14A0);
    DumpOpcodeLine(output, "SMSG_NOTIFY_DANCE", 0x4904);
    DumpOpcodeLine(output, "SMSG_NOTIFY_DEST_LOC_SPELL_CAST", 0x6204);
    DumpOpcodeLine(output, "SMSG_NPC_TEXT_UPDATE", 0x4436);
    DumpOpcodeLine(output, "SMSG_OFFER_PETITION_ERROR", 0x2716);
    DumpOpcodeLine(output, "SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA", 0x4D34);
    DumpOpcodeLine(output, "SMSG_OPEN_CONTAINER", 0x4714);
    DumpOpcodeLine(output, "SMSG_OPEN_LFG_DUNGEON_FINDER", 0x2C37);
    DumpOpcodeLine(output, "SMSG_OVERRIDE_LIGHT", 0x4225);
    DumpOpcodeLine(output, "SMSG_PAGE_TEXT_QUERY_RESPONSE", 0x2B14);
    DumpOpcodeLine(output, "SMSG_PARTYKILLLOG", 0x4937);
    DumpOpcodeLine(output, "SMSG_PARTY_COMMAND_RESULT", 0x6E07);
    DumpOpcodeLine(output, "SMSG_PARTY_MEMBER_STATS", 0x2104);
    DumpOpcodeLine(output, "SMSG_PARTY_MEMBER_STATS_FULL", 0x0215);
    DumpOpcodeLine(output, "SMSG_PAUSE_MIRROR_TIMER", 0x4015);
    DumpOpcodeLine(output, "SMSG_PERIODICAURALOG", 0x0416);
    DumpOpcodeLine(output, "SMSG_PETGODMODE", 0x2E36);
    DumpOpcodeLine(output, "SMSG_PETITION_ALREADY_SIGNED", 0x5DA3);
    DumpOpcodeLine(output, "SMSG_PETITION_QUERY_RESPONSE", 0x4B37);
    DumpOpcodeLine(output, "SMSG_PETITION_SHOWLIST", 0x6405);
    DumpOpcodeLine(output, "SMSG_PETITION_SHOW_SIGNATURES", 0x0716);
    DumpOpcodeLine(output, "SMSG_PETITION_SIGN_RESULTS", 0x6217);
    DumpOpcodeLine(output, "SMSG_PET_ACTION_FEEDBACK", 0x0807);
    DumpOpcodeLine(output, "SMSG_PET_ACTION_SOUND", 0x4324);
    DumpOpcodeLine(output, "SMSG_PET_ADDED", 0x3CA5);
    DumpOpcodeLine(output, "SMSG_PET_BROKEN", 0x2E27);
    DumpOpcodeLine(output, "SMSG_PET_CAST_FAILED", 0x2B15);
    DumpOpcodeLine(output, "SMSG_PET_DISMISS_SOUND", 0x2B05);
    DumpOpcodeLine(output, "SMSG_PET_GUIDS", 0x2D26);
    DumpOpcodeLine(output, "SMSG_PET_LEARNED_SPELL", 0x0507);
    DumpOpcodeLine(output, "SMSG_PET_MODE", 0x2235);
    DumpOpcodeLine(output, "SMSG_PET_NAME_INVALID", 0x6007);
    DumpOpcodeLine(output, "SMSG_PET_NAME_QUERY_RESPONSE", 0x4C37);
    DumpOpcodeLine(output, "SMSG_PET_REMOVED_SPELL", 0x6A04);
    DumpOpcodeLine(output, "SMSG_PET_RENAMEABLE", 0x2B27);
    DumpOpcodeLine(output, "SMSG_PET_SLOT_UPDATED", 0x51A3);
    DumpOpcodeLine(output, "SMSG_PET_SPELLS", 0x4114);
    DumpOpcodeLine(output, "SMSG_PET_TAME_FAILURE", 0x6B24);
    DumpOpcodeLine(output, "SMSG_PET_UPDATE_COMBO_POINTS", 0x4325);
    DumpOpcodeLine(output, "SMSG_PLAYED_TIME", 0x6037);
    DumpOpcodeLine(output, "SMSG_PLAYERBINDERROR", 0x6A24);
    DumpOpcodeLine(output, "SMSG_PLAYERBOUND", 0x2516);
    DumpOpcodeLine(output, "SMSG_PLAYER_DIFFICULTY_CHANGE", 0x2217);
    DumpOpcodeLine(output, "SMSG_PLAYER_MOVE", 0x79A2);
    DumpOpcodeLine(output, "SMSG_PLAYER_SKINNED", 0x0116);
    DumpOpcodeLine(output, "SMSG_PLAYER_VEHICLE_DATA", 0x4115);
    DumpOpcodeLine(output, "SMSG_PLAY_DANCE", 0x4704);
    DumpOpcodeLine(output, "SMSG_PLAY_MUSIC", 0x4B06);
    DumpOpcodeLine(output, "SMSG_PLAY_OBJECT_SOUND", 0x2635);
    DumpOpcodeLine(output, "SMSG_PLAY_ONE_SHOT_ANIM_KIT", 0x4A35);
    DumpOpcodeLine(output, "SMSG_PLAY_SOUND", 0x2134);
    DumpOpcodeLine(output, "SMSG_PLAY_SPELL_VISUAL", 0x10B1);
    DumpOpcodeLine(output, "SMSG_PLAY_SPELL_VISUAL_KIT", 0x55A5);
    DumpOpcodeLine(output, "SMSG_PLAY_TIME_WARNING", 0x4814);
    DumpOpcodeLine(output, "SMSG_PONG", 0x4D42);
    DumpOpcodeLine(output, "SMSG_POWER_UPDATE", 0x4A07);
    DumpOpcodeLine(output, "SMSG_PRE_RESURRECT", 0x6C36);
    DumpOpcodeLine(output, "SMSG_PROCRESIST", 0x0426);
    DumpOpcodeLine(output, "SMSG_PROPOSE_LEVEL_GRANT", 0x6114);
    DumpOpcodeLine(output, "SMSG_PVP_CREDIT", 0x6015);
    DumpOpcodeLine(output, "SMSG_PVP_LOG_DATA", 0x5CB2);
    DumpOpcodeLine(output, "SMSG_PVP_OPTIONS_ENABLED", 0x50A1);
    DumpOpcodeLine(output, "SMSG_QUERY_QUESTS_COMPLETED_RESPONSE", 0x6314);
    DumpOpcodeLine(output, "SMSG_QUERY_TIME_RESPONSE", 0x2124);
    DumpOpcodeLine(output, "SMSG_QUESTGIVER_OFFER_REWARD", 0x2427);
    DumpOpcodeLine(output, "SMSG_QUESTGIVER_QUEST_COMPLETE", 0x55A4);
    DumpOpcodeLine(output, "SMSG_QUESTGIVER_QUEST_DETAILS", 0x2425);
    DumpOpcodeLine(output, "SMSG_QUESTGIVER_QUEST_FAILED", 0x4236);
    DumpOpcodeLine(output, "SMSG_QUESTGIVER_QUEST_INVALID", 0x4016);
    DumpOpcodeLine(output, "SMSG_QUESTGIVER_QUEST_LIST", 0x0134);
    DumpOpcodeLine(output, "SMSG_QUESTGIVER_REQUEST_ITEMS", 0x6236);
    DumpOpcodeLine(output, "SMSG_QUESTGIVER_STATUS", 0x2115);
    DumpOpcodeLine(output, "SMSG_QUESTGIVER_STATUS_MULTIPLE", 0x4F25);
    DumpOpcodeLine(output, "SMSG_QUESTLOG_FULL", 0x0E36);
    DumpOpcodeLine(output, "SMSG_QUESTUPDATE_ADD_KILL", 0x0D27);
    DumpOpcodeLine(output, "SMSG_QUESTUPDATE_ADD_PVP_KILL", 0x4416);
    DumpOpcodeLine(output, "SMSG_QUESTUPDATE_COMPLETE", 0x2937);
    DumpOpcodeLine(output, "SMSG_QUESTUPDATE_FAILED", 0x6324);
    DumpOpcodeLine(output, "SMSG_QUESTUPDATE_FAILEDTIMER", 0x6427);
    DumpOpcodeLine(output, "SMSG_QUEST_NPC_QUERY_RESPONSE", 0x75A1);
    DumpOpcodeLine(output, "SMSG_QUEST_CONFIRM_ACCEPT", 0x6F07);
    DumpOpcodeLine(output, "SMSG_QUEST_FORCE_REMOVE", 0x6605);
    DumpOpcodeLine(output, "SMSG_QUEST_POI_QUERY_RESPONSE", 0x6304);
    DumpOpcodeLine(output, "SMSG_QUEST_QUERY_RESPONSE", 0x6936);
    DumpOpcodeLine(output, "SMSG_RAID_GROUP_ONLY", 0x0837);
    DumpOpcodeLine(output, "SMSG_RAID_INSTANCE_INFO", 0x6626);
    DumpOpcodeLine(output, "SMSG_RAID_INSTANCE_MESSAGE", 0x6E15);
    DumpOpcodeLine(output, "SMSG_RAID_MARKERS_CHANGED", 0x10A1);
    DumpOpcodeLine(output, "SMSG_RAID_READY_CHECK_THROTTLED_ERROR", 0x2607);
    DumpOpcodeLine(output, "SMSG_RAID_SUMMON_FAILED", 0x18B6);
    DumpOpcodeLine(output, "SMSG_RANDOMIZE_CHAR_NAME", 0x38B1);
    DumpOpcodeLine(output, "SMSG_RATED_BG_RATING", 0x15A1);
    DumpOpcodeLine(output, "SMSG_RATED_BG_STATS", 0x34A1);
    DumpOpcodeLine(output, "SMSG_READ_ITEM_FAILED", 0x0F16);
    DumpOpcodeLine(output, "SMSG_READ_ITEM_OK", 0x2605);
    DumpOpcodeLine(output, "SMSG_REALM_SPLIT", 0x2714);
    DumpOpcodeLine(output, "SMSG_REAL_GROUP_UPDATE", 0x0F34);
    DumpOpcodeLine(output, "SMSG_RECEIVED_MAIL", 0x2924);
    DumpOpcodeLine(output, "SMSG_REDIRECT_CLIENT", 0x0942);
    DumpOpcodeLine(output, "SMSG_REFER_A_FRIEND_EXPIRED", 0x4934);
    DumpOpcodeLine(output, "SMSG_REFER_A_FRIEND_FAILURE", 0x2037);
    DumpOpcodeLine(output, "SMSG_REFORGE_RESULT", 0x58A4);
    DumpOpcodeLine(output, "SMSG_REMOVED_SPELL", 0x4804);
    DumpOpcodeLine(output, "SMSG_REPORT_PVP_AFK_RESULT", 0x2D06);
    DumpOpcodeLine(output, "SMSG_REQUEST_CEMETERY_LIST_RESPONSE", 0x30A7);
    DumpOpcodeLine(output, "SMSG_REQUEST_PVP_REWARDS_RESPONSE", 0x5DA4);
    DumpOpcodeLine(output, "SMSG_RESEARCH_COMPLETE", 0x35A6);
    DumpOpcodeLine(output, "SMSG_RESEARCH_SETUP_HISTORY", 0x10B6);
    DumpOpcodeLine(output, "SMSG_RESET_COMPRESSION_CONTEXT", 0x0142);
    DumpOpcodeLine(output, "SMSG_RESET_FAILED_NOTIFY", 0x4616);
    DumpOpcodeLine(output, "SMSG_RESPOND_INSPECT_ACHIEVEMENTS", 0x15B0);
    DumpOpcodeLine(output, "SMSG_RESURRECT_REQUEST", 0x2905);
    DumpOpcodeLine(output, "SMSG_RESYNC_RUNES", 0x6224);
    DumpOpcodeLine(output, "SMSG_ROLE_POLL_BEGIN", 0x70B0);
    DumpOpcodeLine(output, "SMSG_RWHOIS", 0x2437);
    DumpOpcodeLine(output, "SMSG_SELL_ITEM", 0x6105);
    DumpOpcodeLine(output, "SMSG_SEND_MAIL_RESULT", 0x4927);
    DumpOpcodeLine(output, "SMSG_SEND_UNLEARN_SPELLS", 0x4E25);
    DumpOpcodeLine(output, "SMSG_SERVERTIME", 0x6327);
    DumpOpcodeLine(output, "SMSG_SERVER_FIRST_ACHIEVEMENT", 0x6424);
    DumpOpcodeLine(output, "SMSG_SERVER_INFO_RESPONSE", 0x74B5);
    DumpOpcodeLine(output, "SMSG_SERVER_MESSAGE", 0x6C04);
    DumpOpcodeLine(output, "SMSG_SERVER_PERF", 0x74B6);
    DumpOpcodeLine(output, "SMSG_SET_DF_FAST_LAUNCH_RESULT", 0x35B6);
    DumpOpcodeLine(output, "SMSG_SET_FACTION_ATWAR", 0x4216);
    DumpOpcodeLine(output, "SMSG_SET_FACTION_STANDING", 0x0126);
    DumpOpcodeLine(output, "SMSG_SET_FACTION_VISIBLE", 0x2525);
    DumpOpcodeLine(output, "SMSG_SET_FLAT_SPELL_MODIFIER", 0x2834);
    DumpOpcodeLine(output, "SMSG_SET_FORCED_REACTIONS", 0x4615);
    DumpOpcodeLine(output, "SMSG_SET_PCT_SPELL_MODIFIER", 0x0224);
    DumpOpcodeLine(output, "SMSG_SET_PHASE_SHIFT", 0x70A0);
    DumpOpcodeLine(output, "SMSG_SET_PLAYER_DECLINED_NAMES_RESULT", 0x2B25);
    DumpOpcodeLine(output, "SMSG_SET_PLAY_HOVER_ANIM", 0x30A6);
    DumpOpcodeLine(output, "SMSG_SET_PROFICIENCY", 0x6207);
    DumpOpcodeLine(output, "SMSG_SET_PROJECTILE_POSITION", 0x2616);
    DumpOpcodeLine(output, "SMSG_SHOWTAXINODES", 0x2A36);
    DumpOpcodeLine(output, "SMSG_SHOW_BANK", 0x2627);
    DumpOpcodeLine(output, "SMSG_SHOW_MAILBOX", 0x2524);
    DumpOpcodeLine(output, "SMSG_SHOW_RATINGS", 0x11B4);
    DumpOpcodeLine(output, "SMSG_SOCKET_GEMS_RESULT", 0x6014);
    DumpOpcodeLine(output, "SMSG_SOR_START_EXPERIENCE_INCOMPLETE", 0x7CA7);
    DumpOpcodeLine(output, "SMSG_SPELLBREAKLOG", 0x6B17);
    DumpOpcodeLine(output, "SMSG_SPELLDAMAGESHIELD", 0x2927);
    DumpOpcodeLine(output, "SMSG_SPELLDISPELLOG", 0x4516);
    DumpOpcodeLine(output, "SMSG_SPELLENERGIZELOG", 0x0414);
    DumpOpcodeLine(output, "SMSG_SPELLHEALLOG", 0x2816);
    DumpOpcodeLine(output, "SMSG_SPELLINSTAKILLLOG", 0x6216);
    DumpOpcodeLine(output, "SMSG_SPELLINTERRUPTLOG", 0x1DA7);
    DumpOpcodeLine(output, "SMSG_SPELLLOGEXECUTE", 0x0626);
    DumpOpcodeLine(output, "SMSG_SPELLLOGMISS", 0x0625);
    DumpOpcodeLine(output, "SMSG_SPELLNONMELEEDAMAGELOG", 0x4315);
    DumpOpcodeLine(output, "SMSG_SPELLORDAMAGE_IMMUNE", 0x4507);
    DumpOpcodeLine(output, "SMSG_SPELLSTEALLOG", 0x4E26);
    DumpOpcodeLine(output, "SMSG_SPELL_CATEGORY_COOLDOWN", 0x71B6);
    DumpOpcodeLine(output, "SMSG_SPELL_COOLDOWN", 0x4B16);
    DumpOpcodeLine(output, "SMSG_SPELL_DELAYED", 0x0715);
    DumpOpcodeLine(output, "SMSG_SPELL_FAILED_OTHER", 0x0C34);
    DumpOpcodeLine(output, "SMSG_SPELL_FAILURE", 0x4535);
    DumpOpcodeLine(output, "SMSG_SPELL_GO", 0x6E16);
    DumpOpcodeLine(output, "SMSG_SPELL_START", 0x6415);
    DumpOpcodeLine(output, "SMSG_SPELL_UPDATE_CHAIN_TARGETS", 0x6006);
    DumpOpcodeLine(output, "SMSG_SPIRIT_HEALER_CONFIRM", 0x4917);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_COLLISION_DISABLE", 0x35B1);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_COLLISION_ENABLE", 0x3CB0);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_GRAVITY_DISABLE", 0x5DB5);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_GRAVITY_ENABLE", 0x3CA6);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_ROOT", 0x51B4);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_ANIM", 0x4335);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_FEATHER_FALL", 0x3DA5);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_FLIGHT_BACK_SPEED", 0x38B3);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_FLIGHT_SPEED", 0x39A0);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_FLYING", 0x31B5);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_HOVER", 0x14B6);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_LAND_WALK", 0x3DA7);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_NORMAL_FALL", 0x38B2);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_PITCH_RATE", 0x14B0);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_RUN_BACK_SPEED", 0x3DB3);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_RUN_MODE", 0x75A7);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_RUN_SPEED", 0x51B7);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_SWIM_BACK_SPEED", 0x59A1);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_SWIM_SPEED", 0x39A4);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_TURN_RATE", 0x78B5);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_WALK_MODE", 0x54B6);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_WALK_SPEED", 0x34A5);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_SET_WATER_WALK", 0x50A2);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_START_SWIM", 0x31A5);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_STOP_SWIM", 0x1DA2);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_UNROOT", 0x75B6);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_UNSET_FLYING", 0x58A6);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_UNSET_HOVER", 0x7DA5);
    DumpOpcodeLine(output, "SMSG_SPLINE_MOVE_WATER_WALK", 0x50A2);
    DumpOpcodeLine(output, "SMSG_STABLE_RESULT", 0x2204);
    DumpOpcodeLine(output, "SMSG_STANDSTATE_UPDATE", 0x6F04);
    DumpOpcodeLine(output, "SMSG_START_MIRROR_TIMER", 0x6824);
    DumpOpcodeLine(output, "SMSG_START_TIMER", 0x59A5);
    DumpOpcodeLine(output, "SMSG_STOP_DANCE", 0x4637);
    DumpOpcodeLine(output, "SMSG_STOP_MIRROR_TIMER", 0x0B06);
    DumpOpcodeLine(output, "SMSG_STREAMING_MOVIE", 0x15B7);
    DumpOpcodeLine(output, "SMSG_SUMMON_CANCEL", 0x0B34);
    DumpOpcodeLine(output, "SMSG_SUMMON_REQUEST", 0x2A07);
    DumpOpcodeLine(output, "SMSG_SUPERCEDED_SPELL", 0x35B0);
    DumpOpcodeLine(output, "SMSG_SUPPRESS_NPC_GREETINGS", 0x74B1);
    DumpOpcodeLine(output, "SMSG_SUSPEND_COMMS", 0x4140);
    DumpOpcodeLine(output, "SMSG_SUSPEND_TOKEN_RESPONSE", 0x14B1);
    DumpOpcodeLine(output, "SMSG_TALENTS_ERROR", 0x0916);
    DumpOpcodeLine(output, "SMSG_TALENTS_INFO", 0x6F26);
    DumpOpcodeLine(output, "SMSG_TALENTS_INVOLUNTARILY_RESET", 0x2C27);
    DumpOpcodeLine(output, "SMSG_TAXINODE_STATUS", 0x2936);
    DumpOpcodeLine(output, "SMSG_TEST_DROP_RATE_RESULT", 0x6816);
    DumpOpcodeLine(output, "SMSG_TEXT_EMOTE", 0x0B05);
    DumpOpcodeLine(output, "SMSG_THREAT_CLEAR", 0x6437);
    DumpOpcodeLine(output, "SMSG_THREAT_REMOVE", 0x2E05);
    DumpOpcodeLine(output, "SMSG_THREAT_UPDATE", 0x4735);
    DumpOpcodeLine(output, "SMSG_TIME_ADJUSTMENT", 0x79B7);
    DumpOpcodeLine(output, "SMSG_TIME_SYNC_REQ", 0x3CA4);
    DumpOpcodeLine(output, "SMSG_TITLE_EARNED", 0x2426);
    DumpOpcodeLine(output, "SMSG_TOGGLE_XP_GAIN", 0x6704);
    DumpOpcodeLine(output, "SMSG_TOTEM_CREATED", 0x2414);
    DumpOpcodeLine(output, "SMSG_TRADE_STATUS", 0x5CA3);
    DumpOpcodeLine(output, "SMSG_TRADE_STATUS_EXTENDED", 0x70A2);
    DumpOpcodeLine(output, "SMSG_TRAINER_BUY_FAILED", 0x0004);
    DumpOpcodeLine(output, "SMSG_TRAINER_BUY_SUCCEEDED", 0x6A05);
    DumpOpcodeLine(output, "SMSG_TRAINER_LIST", 0x4414);
    DumpOpcodeLine(output, "SMSG_TRANSFER_ABORTED", 0x0537);
    DumpOpcodeLine(output, "SMSG_TRANSFER_PENDING", 0x18A6);
    DumpOpcodeLine(output, "SMSG_TRIGGER_CINEMATIC", 0x6C27);
    DumpOpcodeLine(output, "SMSG_TRIGGER_MOVIE", 0x4625);
    DumpOpcodeLine(output, "SMSG_TURN_IN_PETITION_RESULTS", 0x0F07);
    DumpOpcodeLine(output, "SMSG_TUTORIAL_FLAGS", 0x0B35);
    DumpOpcodeLine(output, "SMSG_UNIT_HEALTH_FREQUENT", 0x2C26);
    DumpOpcodeLine(output, "SMSG_UNIT_SPELLCAST_START", 0x2517);
    DumpOpcodeLine(output, "SMSG_UPDATE_ACCOUNT_DATA", 0x6837);
    DumpOpcodeLine(output, "SMSG_UPDATE_ACCOUNT_DATA_COMPLETE", 0x2015);
    DumpOpcodeLine(output, "SMSG_UPDATE_COMBO_POINTS", 0x6B34);
    DumpOpcodeLine(output, "SMSG_UPDATE_CURRENCY", 0x59B0);
    DumpOpcodeLine(output, "SMSG_UPDATE_CURRENCY_WEEK_LIMIT", 0x70A7);
    DumpOpcodeLine(output, "SMSG_UPDATE_DUNGEON_ENCOUNTER_FOR_LOOT", 0x3CB5);
    DumpOpcodeLine(output, "SMSG_UPDATE_INSTANCE_ENCOUNTER_UNIT", 0x4007);
    DumpOpcodeLine(output, "SMSG_UPDATE_INSTANCE_OWNERSHIP", 0x4915);
    DumpOpcodeLine(output, "SMSG_UPDATE_LAST_INSTANCE", 0x0437);
    DumpOpcodeLine(output, "SMSG_UPDATE_OBJECT", 0x4715);
    DumpOpcodeLine(output, "SMSG_UPDATE_SERVER_PLAYER_POSITION", 0x74A3);
    DumpOpcodeLine(output, "SMSG_UPDATE_WORLD_STATE", 0x4816);
    DumpOpcodeLine(output, "SMSG_USERLIST_ADD", 0x0F37);
    DumpOpcodeLine(output, "SMSG_USERLIST_REMOVE", 0x2006);
    DumpOpcodeLine(output, "SMSG_USERLIST_UPDATE", 0x0135);
    DumpOpcodeLine(output, "SMSG_VOICESESSION_FULL", 0x6225);
    DumpOpcodeLine(output, "SMSG_VOICE_CHAT_STATUS", 0x0F15);
    DumpOpcodeLine(output, "SMSG_VOICE_PARENTAL_CONTROLS", 0x0534);
    DumpOpcodeLine(output, "SMSG_VOICE_SESSION_LEAVE", 0x2A24);
    DumpOpcodeLine(output, "SMSG_VOICE_SESSION_ROSTER_UPDATE", 0x2A17);
    DumpOpcodeLine(output, "SMSG_VOICE_SET_TALKER_MUTED", 0x6E35);
    DumpOpcodeLine(output, "SMSG_VOID_ITEM_SWAP_RESPONSE", 0x78A2);
    DumpOpcodeLine(output, "SMSG_VOID_STORAGE_CONTENTS", 0x75B4);
    DumpOpcodeLine(output, "SMSG_VOID_STORAGE_FAILED", 0x18A7);
    DumpOpcodeLine(output, "SMSG_VOID_STORAGE_TRANSFER_CHANGES", 0x51A6);
    DumpOpcodeLine(output, "SMSG_VOID_TRANSFER_RESULT", 0x1DA6);
    DumpOpcodeLine(output, "SMSG_WAIT_QUEUE_FINISH", 0x75B7);
    DumpOpcodeLine(output, "SMSG_WAIT_QUEUE_UPDATE", 0x58A1);
    DumpOpcodeLine(output, "SMSG_WARDEN_DATA", 0x31A0);
    DumpOpcodeLine(output, "SMSG_WARGAME_CHECK_ENTRY", 0x3DA4);
    DumpOpcodeLine(output, "SMSG_WARGAME_REQUEST_SENT", 0x59B2);
    DumpOpcodeLine(output, "SMSG_WEATHER", 0x2904);
    DumpOpcodeLine(output, "SMSG_WEEKLY_LAST_RESET", 0x50A5);
    DumpOpcodeLine(output, "SMSG_WEEKLY_RESET_CURRENCY", 0x3CA1);
    DumpOpcodeLine(output, "SMSG_WEEKLY_SPELL_USAGE", 0x39B7);
    DumpOpcodeLine(output, "SMSG_WEEKLY_SPELL_USAGE_UPDATE", 0x11B5);
    DumpOpcodeLine(output, "SMSG_WHO", 0x6907);
    DumpOpcodeLine(output, "SMSG_WHOIS", 0x6917);
    DumpOpcodeLine(output, "SMSG_WORLD_SERVER_INFO", 0x31A2);
    DumpOpcodeLine(output, "SMSG_WORLD_STATE_UI_TIMER_UPDATE", 0x4A14);
    DumpOpcodeLine(output, "SMSG_XP_GAIN_ABORTED", 0x50B4);
    DumpOpcodeLine(output, "SMSG_ZONE_UNDER_ATTACK", 0x0A06);
    fclose(output);
}