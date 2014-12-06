/***
 * Demonstrike Core
 * Citric
 */

#include "StdAfx.h"

initialiseSingleton(DBCLoader);

#define DECLARE_CLASS_INTERNAL_DBC_MACRO(EntryClass, DeclaredClass) SERVER_DECL DBStorage<EntryClass, DBC<EntryClass>> DeclaredClass
#define DECLARE_CLASS_INTERNAL_DB2_MACRO(EntryClass, DeclaredClass) SERVER_DECL DBStorage<EntryClass, DB2<EntryClass>> DeclaredClass

DECLARE_CLASS_INTERNAL_DBC_MACRO(AchievementEntry, dbcAchievement);
const char* achievementFormat = "nuuussuuxuxxuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(AchievementCriteriaEntry, dbcAchievementCriteria);
const char* achievementcriteriaFormat = "niiiiiiiisiiiiixxiiiiii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(AreaGroupEntry, dbcAreaGroup);
const char* areagroupFormat = "niiiiiii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(AreaTableEntry, dbcAreaTable);
const char* areatableFormat = "uuuuuxxxxxusuxxxxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(AreaTriggerEntry, dbcAreaTrigger);
const char* AreaTriggerFormat = "uufffffxfffff";

DECLARE_CLASS_INTERNAL_DBC_MACRO(AuctionHouseDBC, dbcAuctionHouse);
const char* auctionhousedbcFormat = "uuuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(BankSlotPriceEntry, dbcBankSlotPrices);
const char* bankslotpriceformat = "uu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(BarberShopStyleEntry, dbcBarberShopStyle);
const char* barbershopstyleFormat = "nusxxuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(BattleMasterListEntry, dbcBattleMasterList);
const char* battlemasterlistFormat = "uiiiiiiiiuxsuuuuuuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CharClassEntry, dbcCharClass);
const char* charclassFormat = "uuxsxxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CharRaceEntry, dbcCharRace);
const char* charraceFormat = "uxuxuuxxxxxxuusxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ChatChannelDBC, dbcChatChannels);
const char* chatchannelformat = "uuxsx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CreatureBoundData, dbcCreatureBoundData);
const char* creatureboundFormat = "ufffffff";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CreatureDisplayInfo, dbcCreatureDisplayInfo);
const char* creaturedisplayFormat = "uxxxfxxxxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CreatureFamilyEntry, dbcCreatureFamily);
const char* creaturefamilyFormat = "ufufuuuuuxsx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CreatureSpellDataEntry, dbcCreatureSpellData);
const char* creaturespelldataFormat = "uuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CharTitleEntry, dbcCharTitles);
const char* chartitlesFormat = "uxsxux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TaxiNodeEntry, dbcTaxiNode);
const char* dbctaxinodeFormat = "uufffxuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TaxiPathEntry, dbcTaxiPath);
const char* dbctaxipathFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TaxiPathNodeEntry, dbcTaxiPathNode);
const char* dbctaxipathnodeFormat = "uuuufffuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DurabilityCostsEntry, dbcDurabilityCosts);
const char * durabilitycostsFormat = "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DurabilityQualityEntry, dbcDurabilityQuality);
const char * durabilityqualityFormat = "uf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(EmoteEntry, dbcEmoteEntry);
const char* EmoteEntryFormat = "uxuuuuxuxuxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellItemEnchantEntry, dbcEnchant);
const char* EnchantEntryFormat = "uxuuuuuuuuuuuusxuuuxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(FactionEntry, dbcFaction);
const char* factionFormat = "uiuuuuuuuuiiiiuuuuuxxxxsxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(FactionTemplateEntry, dbcFactionTemplate);
const char* factiontemplateFormat = "uuuuuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(GemPropertyEntry, dbcGemProperty);
const char* gempropertyFormat = "uuxxux";

DECLARE_CLASS_INTERNAL_DB2_MACRO(ItemDataEntry, db2Item);
const char* ItemFormat = "uuuiiuuu";

DECLARE_CLASS_INTERNAL_DB2_MACRO(ItemSparseEntry, db2ItemSparse);
const char* ItemSparseformat = "uuuuuuuiiuiuuuuuuuuuuiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiisssssiiiiiiiiiiiiiiiiiiiiiiifiiifii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ArmorLocationEntry, dbcArmorLocation);
const char* ArmorLocationFormat = "ufffff";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemArmorQuality, dbcArmorQuality);
const char* ArmorQualityFormat = "ufffffffu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemArmorShield, dbcArmorShield);
const char* ArmorShieldFormat = "uuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemArmorTotal, dbcArmorTotal);
const char* ArmorTotalFormat = "uuffff";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageAmmo);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageOneHand);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageOneHandCaster);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageRanged);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageThrown);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageTwoHand);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageTwoHandCaster);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageDamageWand);
const char* ItemDamageFormat = "ufffffffu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemExtendedCostEntry, dbcItemExtendedCost);
const char* itemextendedcostFormat = "uxxuuuuuuuuuuuixuuuuuuuuuuxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemLimitCategoryEntry, dbcItemLimitCategory);
const char* itemlimitcategoryFormat = "nxuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemRandomPropertiesEntry, dbcItemRandomProperties);
const char* itemrandompropertiesFormat = "uxuuuxxs";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemRandomSuffixEntry, dbcItemRandomSuffix);
const char* itemrandomsuffixFormat = "usxuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemSetEntry, dbcItemSet);
const char* ItemSetFormat = "dsuuuuuuuuuuxxxxxxxuuuuuuuuuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(LockEntry, dbcLock);
const char* LockFormat = "uuuuuuxxxuuuuuxxxuuuuuxxxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(LFGDungeonsEntry, dbcLFGDungeons);
const char* lfgdungeonsFormat = "nsuuuuuiuuuxxuxux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(MapEntry, dbcMap);
const char* mapentryFormat = "usuxxusussuxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(GlyphProperties, dbcGlyphProperties);
const char* glypepropertiesFormat="uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(NumTalentsAtLevelEntry, dbcNumTalents);
const char* numtalentsFormat="uf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(QuestXPLevelEntry, dbcQuestXP);
const char* questxpFormat="niiiiiiiiii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SkillLineEntry, dbcSkillLine);
const char* SkillLineEntryFormat = "uuusxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SkillLineAbilityEntry, dbcSkillLineSpell);
const char* SkillLineSpellFormat = "uuuuuuuuuuuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcCombatRating);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcBarberShopPrices);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcMeleeCrit);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcMeleeCritBase);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcSpellCrit);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcSpellCritBase);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcManaRegen);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcManaRegenBase);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcHPPerStam);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcCombatRatingScaling);
const char* gtFloatFormat = "uf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellEntry, dbcSpell);
const char* spellentryFormat = "uuuuuuuuuuxuuuufuuuussssuuxxuxuuuuuuuxuuuuuuuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellAuraOptionsEntry, dbcSpellAuraOptions);
const char* spellauraoptionFormat = "uuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellAuraRestrictionsEntry, dbcSpellAuraRestrictions);
const char* spellaurarestrictionFormat = "uuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCastingRequirementsEntry, dbcSpellCastingRequirements);
const char* spellcastingrequirementFormat = "uuxxixu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCastTimeEntry, dbcSpellCastTime);
const char* spellcasttimeFormat = "uuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCategoriesEntry, dbcSpellCategories);
const char* spellcategoryFormat = "uuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellClassOptionsEntry, dbcSpellClassOptions);
const char* spellclassoptionFormat = "uxuuuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCooldownsEntry, dbcSpellCooldowns);
const char* spellcooldownFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellDifficultyEntry, dbcSpellDifficulty);
const char* spelldifficultyFormat = "uuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellDurationEntry, dbcSpellDuration);
const char* spelldurationFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellEffectEntry, dbcSpellEffect);
const char* spelleffectFormat = "uufuuiffuiuuiifuxfuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellEquippedItemsEntry, dbcSpellEquippedItems);
const char* spellequippeditemFormat = "uiii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellInterruptsEntry, dbcSpellInterrupts);
const char* spellinterruptFormat = "uuxuxu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellLevelsEntry, dbcSpellLevels);
const char* spelllevelFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellPowerEntry, dbcSpellPower);
const char* spellpowerFormat = "uuuuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellRadiusEntry, dbcSpellRadius);
const char* spellradiusFormat = "ufxf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellRangeEntry, dbcSpellRange);
const char* spellrangeFormat = "uffffxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellReagentsEntry, dbcSpellReagents);
const char* spellreagentFormat = "uiiiiiiiiuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellRuneCostEntry, dbcSpellRuneCost);
const char* spellrunecostFormat = "uuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellScalingEntry, dbcSpellScaling);
const char* spellscalingFormat = "uiiuiffffffffffu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellShapeshiftEntry, dbcSpellShapeshift);
const char* spellshapeshiftFormat = "uuxuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellShapeshiftFormEntry, dbcSpellShapeshiftForm);
const char* spellshapeshiftformFormat = "uxxuixuuuxxuuuuuuuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellTargetRestrictionsEntry, dbcSpellTargetRestrictions);
const char* spelltargetrestrictionFormat = "uuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellTotemsEntry, dbcSpellTotems);
const char* spelltotemFormat = "uuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SummonPropertiesEntry, dbcSummonProperties);
const char* summonpropertiesFormat = "uuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TalentEntry, dbcTalent);
const char* talententryFormat = "uuuuuuuuuuxxuxxuxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TalentTreePrimarySpellsEntry, dbcTreePrimarySpells);
const char* talenttreeFormat = "uuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TalentTabEntry, dbcTalentTab);
const char* talenttabentryFormat = "uxxuxuxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(VehicleEntry, dbcVehicle);
const char* vehicleentryFormat = "uuffffuuuuuuuufffffffffffffffssssfifiixx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(VehicleSeatEntry, dbcVehicleSeat);
const char* vehicleseatentryFormat = "uuiffffffffffiiiiiifffffffiiifffiiiiiiiffuuuiuxxxxxxxxxxxxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(WMOAreaTableEntry, dbcWMOAreaTable);
const char* wmoareatableFormat="niiixxxxxiixxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(WorldMapOverlayEntry, dbcWorldMapOverlay);
const char* worldmapoverlayFormat="uxuuuuxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(WorldSafeLocsEntry, dbcWorldSafeLocs);
const char* worldsafeLocationsFormat="nifffx";

template<class T> void DBCLoader::LoadDBC(bool *result, std::string filename, const char * format, T *l)
{
    if(!l->Load(filename.c_str(), format))
        *result = false;
}

#define ADD_LOAD_DB(filename, format, stor) tl.AddTask(new Task(new CallbackP4<DBCLoader, bool*, std::string, const char*, decltype(stor)*>(this, &DBCLoader::LoadDBC, result, filename, format, &stor)))

void DBCLoader::FillDBCLoadList(TaskList &tl, const char* datapath, bool *result)
{
    ADD_LOAD_DB(format("%s/Achievement.dbc", datapath), achievementFormat, dbcAchievement);
    ADD_LOAD_DB(format("%s/Achievement_Criteria.dbc", datapath), achievementcriteriaFormat, dbcAchievementCriteria);
    ADD_LOAD_DB(format("%s/AreaGroup.dbc", datapath), areagroupFormat, dbcAreaGroup);
    ADD_LOAD_DB(format("%s/AreaTable.dbc", datapath), areatableFormat, dbcAreaTable);
    ADD_LOAD_DB(format("%s/AreaTrigger.dbc", datapath), AreaTriggerFormat, dbcAreaTrigger);
    ADD_LOAD_DB(format("%s/ArmorLocation.dbc", datapath), ArmorLocationFormat, dbcArmorLocation);
    ADD_LOAD_DB(format("%s/AuctionHouse.dbc", datapath), auctionhousedbcFormat, dbcAuctionHouse);
    ADD_LOAD_DB(format("%s/BankBagSlotPrices.dbc", datapath), bankslotpriceformat, dbcBankSlotPrices);
    ADD_LOAD_DB(format("%s/BarberShopStyle.dbc", datapath), barbershopstyleFormat, dbcBarberShopStyle);
    ADD_LOAD_DB(format("%s/BattlemasterList.dbc", datapath), battlemasterlistFormat, dbcBattleMasterList);
    ADD_LOAD_DB(format("%s/ChatChannels.dbc", datapath), chatchannelformat, dbcChatChannels);
    ADD_LOAD_DB(format("%s/ChrClasses.dbc", datapath), charclassFormat, dbcCharClass);
    ADD_LOAD_DB(format("%s/ChrRaces.dbc", datapath), charraceFormat, dbcCharRace);
    ADD_LOAD_DB(format("%s/CreatureBoundInformation.dbc", datapath), creatureboundFormat, dbcCreatureBoundData);
    ADD_LOAD_DB(format("%s/CreatureDisplayInfo.dbc", datapath), creaturedisplayFormat, dbcCreatureDisplayInfo);
    ADD_LOAD_DB(format("%s/CreatureFamily.dbc", datapath), creaturefamilyFormat, dbcCreatureFamily);
    ADD_LOAD_DB(format("%s/CreatureSpellData.dbc", datapath), creaturespelldataFormat, dbcCreatureSpellData);
    ADD_LOAD_DB(format("%s/CharTitles.dbc", datapath), chartitlesFormat, dbcCharTitles);
    ADD_LOAD_DB(format("%s/DurabilityCosts.dbc", datapath), durabilitycostsFormat, dbcDurabilityCosts);
    ADD_LOAD_DB(format("%s/DurabilityQuality.dbc", datapath), durabilityqualityFormat, dbcDurabilityQuality);
    ADD_LOAD_DB(format("%s/EmotesText.dbc", datapath), EmoteEntryFormat, dbcEmoteEntry);
    ADD_LOAD_DB(format("%s/Faction.dbc", datapath), factionFormat, dbcFaction);
    ADD_LOAD_DB(format("%s/FactionTemplate.dbc", datapath), factiontemplateFormat, dbcFactionTemplate);
    ADD_LOAD_DB(format("%s/GemProperties.dbc", datapath), gempropertyFormat, dbcGemProperty);
    ADD_LOAD_DB(format("%s/Item.db2", datapath), ItemFormat, db2Item);
    ADD_LOAD_DB(format("%s/Item-sparse.db2", datapath), ItemSparseformat, db2ItemSparse);
    ADD_LOAD_DB(format("%s/ItemArmorQuality.dbc", datapath), ArmorQualityFormat, dbcArmorQuality);
    ADD_LOAD_DB(format("%s/ItemArmorShield.dbc", datapath), ArmorShieldFormat, dbcArmorShield);
    ADD_LOAD_DB(format("%s/ItemArmorTotal.dbc", datapath), ArmorTotalFormat, dbcArmorTotal);
    ADD_LOAD_DB(format("%s/ItemDamageAmmo.dbc", datapath), ItemDamageFormat, dbcDamageAmmo);
    ADD_LOAD_DB(format("%s/ItemDamageOneHand.dbc", datapath), ItemDamageFormat, dbcDamageOneHand);
    ADD_LOAD_DB(format("%s/ItemDamageOneHandCaster.dbc", datapath), ItemDamageFormat, dbcDamageOneHandCaster);
    ADD_LOAD_DB(format("%s/ItemDamageRanged.dbc", datapath), ItemDamageFormat, dbcDamageRanged);
    ADD_LOAD_DB(format("%s/ItemDamageThrown.dbc", datapath), ItemDamageFormat, dbcDamageThrown);
    ADD_LOAD_DB(format("%s/ItemDamageTwoHand.dbc", datapath), ItemDamageFormat, dbcDamageTwoHand);
    ADD_LOAD_DB(format("%s/ItemDamageTwoHandCaster.dbc", datapath), ItemDamageFormat, dbcDamageTwoHandCaster);
    ADD_LOAD_DB(format("%s/ItemDamageWand.dbc", datapath), ItemDamageFormat, dbcDamageDamageWand);
    ADD_LOAD_DB(format("%s/ItemExtendedCost.dbc", datapath), itemextendedcostFormat, dbcItemExtendedCost);
    ADD_LOAD_DB(format("%s/ItemLimitCategory.dbc", datapath), itemlimitcategoryFormat, dbcItemLimitCategory);
    ADD_LOAD_DB(format("%s/ItemRandomProperties.dbc", datapath), itemrandompropertiesFormat, dbcItemRandomProperties);
    ADD_LOAD_DB(format("%s/ItemRandomSuffix.dbc", datapath), itemrandomsuffixFormat, dbcItemRandomSuffix);
    ADD_LOAD_DB(format("%s/ItemSet.dbc", datapath), ItemSetFormat, dbcItemSet);
    ADD_LOAD_DB(format("%s/Lock.dbc", datapath), LockFormat, dbcLock);
    ADD_LOAD_DB(format("%s/LFGDungeons.dbc", datapath), lfgdungeonsFormat, dbcLFGDungeons);
    ADD_LOAD_DB(format("%s/Map.dbc", datapath), mapentryFormat, dbcMap);
    ADD_LOAD_DB(format("%s/GlyphProperties.dbc", datapath), glypepropertiesFormat, dbcGlyphProperties);
    ADD_LOAD_DB(format("%s/NumTalentsAtLevel.dbc", datapath), numtalentsFormat, dbcNumTalents);
    ADD_LOAD_DB(format("%s/SkillLine.dbc", datapath), SkillLineEntryFormat, dbcSkillLine);
    ADD_LOAD_DB(format("%s/SkillLineAbility.dbc", datapath), SkillLineSpellFormat, dbcSkillLineSpell);
    ADD_LOAD_DB(format("%s/gtBarberShopCostBase.dbc", datapath), gtFloatFormat, dbcBarberShopPrices);
    ADD_LOAD_DB(format("%s/gtChanceToMeleeCrit.dbc", datapath), gtFloatFormat, dbcMeleeCrit);
    ADD_LOAD_DB(format("%s/gtChanceToMeleeCritBase.dbc", datapath), gtFloatFormat, dbcMeleeCritBase);
    ADD_LOAD_DB(format("%s/gtChanceToSpellCrit.dbc", datapath), gtFloatFormat, dbcSpellCrit);
    ADD_LOAD_DB(format("%s/gtChanceToSpellCritBase.dbc", datapath), gtFloatFormat, dbcSpellCritBase);
    ADD_LOAD_DB(format("%s/gtCombatRatings.dbc", datapath), gtFloatFormat, dbcCombatRating);
    ADD_LOAD_DB(format("%s/gtOCTHpPerStamina.dbc", datapath), gtFloatFormat, dbcHPPerStam);
    ADD_LOAD_DB(format("%s/gtOCTRegenMP.dbc", datapath), gtFloatFormat, dbcManaRegen);
    ADD_LOAD_DB(format("%s/gtRegenMPPerSpt.dbc", datapath), gtFloatFormat, dbcManaRegenBase);
    ADD_LOAD_DB(format("%s/gtOCTClassCombatRatingScalar.dbc", datapath), gtFloatFormat, dbcCombatRatingScaling);
    ADD_LOAD_DB(format("%s/Spell.dbc", datapath), spellentryFormat, dbcSpell);
    ADD_LOAD_DB(format("%s/SpellAuraOptions.dbc", datapath), spellauraoptionFormat, dbcSpellAuraOptions);
    ADD_LOAD_DB(format("%s/SpellAuraRestrictions.dbc", datapath), spellaurarestrictionFormat, dbcSpellAuraRestrictions);
    ADD_LOAD_DB(format("%s/SpellCastingRequirements.dbc", datapath), spellcastingrequirementFormat, dbcSpellCastingRequirements);
    ADD_LOAD_DB(format("%s/SpellCastTimes.dbc", datapath), spellcasttimeFormat, dbcSpellCastTime);
    ADD_LOAD_DB(format("%s/SpellCategories.dbc", datapath), spellcategoryFormat, dbcSpellCategories);
    ADD_LOAD_DB(format("%s/SpellClassOptions.dbc", datapath), spellclassoptionFormat, dbcSpellClassOptions);
    ADD_LOAD_DB(format("%s/SpellCooldowns.dbc", datapath), spellcooldownFormat, dbcSpellCooldowns);
    ADD_LOAD_DB(format("%s/SpellDifficulty.dbc", datapath), spelldifficultyFormat, dbcSpellDifficulty);
    ADD_LOAD_DB(format("%s/SpellDuration.dbc", datapath), spelldurationFormat, dbcSpellDuration);
    ADD_LOAD_DB(format("%s/SpellEffect.dbc", datapath), spelleffectFormat, dbcSpellEffect);
    ADD_LOAD_DB(format("%s/SpellEquippedItems.dbc", datapath), spellequippeditemFormat, dbcSpellEquippedItems);
    ADD_LOAD_DB(format("%s/SpellInterrupts.dbc", datapath), spellinterruptFormat, dbcSpellInterrupts);
    ADD_LOAD_DB(format("%s/SpellItemEnchantment.dbc", datapath), EnchantEntryFormat, dbcEnchant);
    ADD_LOAD_DB(format("%s/SpellLevels.dbc", datapath), spelllevelFormat, dbcSpellLevels);
    ADD_LOAD_DB(format("%s/SpellPower.dbc", datapath), spellpowerFormat, dbcSpellPower);
    ADD_LOAD_DB(format("%s/SpellRadius.dbc", datapath), spellradiusFormat, dbcSpellRadius);
    ADD_LOAD_DB(format("%s/SpellRange.dbc", datapath), spellrangeFormat, dbcSpellRange);
    ADD_LOAD_DB(format("%s/SpellReagents.dbc", datapath), spellreagentFormat, dbcSpellReagents);
    ADD_LOAD_DB(format("%s/SpellRunecost.dbc", datapath), spellrunecostFormat, dbcSpellRuneCost);
    ADD_LOAD_DB(format("%s/SpellScaling.dbc", datapath), spellscalingFormat, dbcSpellScaling);
    ADD_LOAD_DB(format("%s/SpellShapeshift.dbc", datapath), spellshapeshiftFormat, dbcSpellShapeshift);
    ADD_LOAD_DB(format("%s/SpellShapeshiftForm.dbc", datapath), spellshapeshiftformFormat, dbcSpellShapeshiftForm);
    ADD_LOAD_DB(format("%s/SpellTargetRestrictions.dbc", datapath), spelltargetrestrictionFormat, dbcSpellTargetRestrictions);
    ADD_LOAD_DB(format("%s/SpellTotems.dbc", datapath), spelltotemFormat, dbcSpellTotems);
    ADD_LOAD_DB(format("%s/SummonProperties.dbc", datapath), summonpropertiesFormat, dbcSummonProperties);
    ADD_LOAD_DB(format("%s/Talent.dbc", datapath), talententryFormat, dbcTalent);
    ADD_LOAD_DB(format("%s/TalentTab.dbc", datapath), talenttabentryFormat, dbcTalentTab);
    ADD_LOAD_DB(format("%s/TalentTreePrimarySpells.dbc", datapath), talenttreeFormat, dbcTreePrimarySpells);
    ADD_LOAD_DB(format("%s/TaxiNodes.dbc", datapath), dbctaxinodeFormat, dbcTaxiNode);
    ADD_LOAD_DB(format("%s/TaxiPath.dbc", datapath), dbctaxipathFormat, dbcTaxiPath);
    ADD_LOAD_DB(format("%s/TaxiPathNode.dbc", datapath), dbctaxipathnodeFormat, dbcTaxiPathNode);
    ADD_LOAD_DB(format("%s/Vehicle.dbc", datapath), vehicleentryFormat, dbcVehicle);
    ADD_LOAD_DB(format("%s/VehicleSeat.dbc", datapath), vehicleseatentryFormat, dbcVehicleSeat);
    ADD_LOAD_DB(format("%s/WMOAreaTable.dbc", datapath), wmoareatableFormat, dbcWMOAreaTable);
    ADD_LOAD_DB(format("%s/WorldMapOverlay.dbc", datapath), worldmapoverlayFormat, dbcWorldMapOverlay);
    ADD_LOAD_DB(format("%s/WorldSafeLocs.dbc", datapath), worldsafeLocationsFormat, dbcWorldSafeLocs);
}
