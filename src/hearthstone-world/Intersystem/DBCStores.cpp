/***
 * Demonstrike Core
 * Citric
 */

#include "StdAfx.h"

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

DECLARE_CLASS_INTERNAL_DBC_MACRO(CharTitlesEntry, dbcCharTitles);
const char* chartitlesFormat = "uxsxux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DBCTaxiNode, dbcTaxiNode);
const char* dbctaxinodeFormat = "uufffxuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DBCTaxiPath, dbcTaxiPath);
const char* dbctaxipathFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DBCTaxiPathNode, dbcTaxiPathNode);
const char* dbctaxipathnodeFormat = "uuuufffuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DurabilityCostsEntry, dbcDurabilityCosts);
const char * durabilitycostsFormat = "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DurabilityQualityEntry, dbcDurabilityQuality);
const char * durabilityqualityFormat = "uf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(EmoteEntry, dbcEmoteEntry);
const char* EmoteEntryFormat = "uxuuuuxuxuxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(EnchantEntry, dbcEnchant);
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

DECLARE_CLASS_INTERNAL_DBC_MACRO(Lock, dbcLock);
const char* LockFormat = "uuuuuuxxxuuuuuxxxuuuuuxxxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(LFGDungeonsEntry, dbcLFGDungeons);
const char* lfgdungeonsFormat = "nsuuuuuiuuuxxuxux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(MapEntry, dbcMap);
const char* mapentryFormat = "usuxxusussuxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(GlyphProperties, dbcGlyphProperties);
const char* glypepropertiesFormat="uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(NumTalentsAtLevelEntry, dbcNumTalents);
const char* numtalentsFormat="uf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(QuestXPEntry, dbcQuestXP);
const char* questxpFormat="niiiiiiiiii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SkillLineEntry, dbcSkillLine);
const char* SkillLineEntryFormat = "uuusxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SkillLineSpell, dbcSkillLineSpell);
const char* SkillLineSpellFormat = "uuuuuuuuuuuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcCombatRating);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcBarberShopPrices);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcMeleeCrit);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcMeleeCritBase);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcSpellCrit);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcSpellCritBase);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcManaRegen);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcManaRegenBase);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcHPRegen);
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

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCastTime, dbcSpellCastTime);
const char* spellcasttimeFormat = "uuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCategoriesEntry, dbcSpellCategories);
const char* spellcategoryFormat = "uuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellClassOptionsEntry, dbcSpellClassOptions);
const char* spellclassoptionFormat = "uxuuuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCooldownsEntry, dbcSpellCooldowns);
const char* spellcooldownFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellDifficultyEntry, dbcSpellDifficulty);
const char* spelldifficultyFormat = "uuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellDuration, dbcSpellDuration);
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

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellRadius, dbcSpellRadius);
const char* spellradiusFormat = "ufxf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellRange, dbcSpellRange);
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

template<class T>
bool loader_stub(const char * filename, const char * format, T& l)
{
    return l.Load(filename, format);
}

#define LOAD_DB(filename, format, stor) if(!loader_stub(filename, format, stor)) { return false; } 

bool LoadDBCs(const char* datapath)
{
    LOAD_DB(format("%s/Achievement.dbc", datapath).c_str(), achievementFormat, dbcAchievement);
    LOAD_DB(format("%s/Achievement_Criteria.dbc", datapath).c_str(), achievementcriteriaFormat, dbcAchievementCriteria);
    LOAD_DB(format("%s/AreaGroup.dbc", datapath).c_str(), areagroupFormat, dbcAreaGroup);
    LOAD_DB(format("%s/AreaTable.dbc", datapath).c_str(), areatableFormat, dbcAreaTable);
    LOAD_DB(format("%s/AreaTrigger.dbc", datapath).c_str(), AreaTriggerFormat, dbcAreaTrigger);
    LOAD_DB(format("%s/ArmorLocation.dbc", datapath).c_str(), ArmorLocationFormat, dbcArmorLocation);
    LOAD_DB(format("%s/AuctionHouse.dbc", datapath).c_str(), auctionhousedbcFormat, dbcAuctionHouse);
    LOAD_DB(format("%s/BankBagSlotPrices.dbc", datapath).c_str(), bankslotpriceformat, dbcBankSlotPrices);
    LOAD_DB(format("%s/BarberShopStyle.dbc", datapath).c_str(), barbershopstyleFormat, dbcBarberShopStyle);
    LOAD_DB(format("%s/BattlemasterList.dbc", datapath).c_str(), battlemasterlistFormat, dbcBattleMasterList);
    LOAD_DB(format("%s/ChatChannels.dbc", datapath).c_str(), chatchannelformat, dbcChatChannels);
    LOAD_DB(format("%s/ChrClasses.dbc", datapath).c_str(), charclassFormat, dbcCharClass);
    LOAD_DB(format("%s/ChrRaces.dbc", datapath).c_str(), charraceFormat, dbcCharRace);
    LOAD_DB(format("%s/CreatureBoundInformation.dbc", datapath).c_str(), creatureboundFormat, dbcCreatureBoundData);
    LOAD_DB(format("%s/CreatureDisplayInfo.dbc", datapath).c_str(), creaturedisplayFormat, dbcCreatureDisplayInfo);
    LOAD_DB(format("%s/CreatureFamily.dbc", datapath).c_str(), creaturefamilyFormat, dbcCreatureFamily);
    LOAD_DB(format("%s/CreatureSpellData.dbc", datapath).c_str(), creaturespelldataFormat, dbcCreatureSpellData);
    LOAD_DB(format("%s/CharTitles.dbc", datapath).c_str(), chartitlesFormat, dbcCharTitles);
    LOAD_DB(format("%s/DurabilityCosts.dbc", datapath).c_str(), durabilitycostsFormat, dbcDurabilityCosts);
    LOAD_DB(format("%s/DurabilityQuality.dbc", datapath).c_str(), durabilityqualityFormat, dbcDurabilityQuality);
    LOAD_DB(format("%s/EmotesText.dbc", datapath).c_str(), EmoteEntryFormat, dbcEmoteEntry);
    LOAD_DB(format("%s/Faction.dbc", datapath).c_str(), factionFormat, dbcFaction);
    LOAD_DB(format("%s/FactionTemplate.dbc", datapath).c_str(), factiontemplateFormat, dbcFactionTemplate);
    LOAD_DB(format("%s/GemProperties.dbc", datapath).c_str(), gempropertyFormat, dbcGemProperty);
    LOAD_DB(format("%s/Item.db2", datapath).c_str(), ItemFormat, db2Item);
    LOAD_DB(format("%s/Item-sparse.db2", datapath).c_str(), ItemSparseformat, db2ItemSparse);
    LOAD_DB(format("%s/ItemArmorQuality.dbc", datapath).c_str(), ArmorQualityFormat, dbcArmorQuality);
    LOAD_DB(format("%s/ItemArmorShield.dbc", datapath).c_str(), ArmorShieldFormat, dbcArmorShield);
    LOAD_DB(format("%s/ItemArmorTotal.dbc", datapath).c_str(), ArmorTotalFormat, dbcArmorTotal);
    LOAD_DB(format("%s/ItemDamageAmmo.dbc", datapath).c_str(), ItemDamageFormat, dbcDamageAmmo);
    LOAD_DB(format("%s/ItemDamageOneHand.dbc", datapath).c_str(), ItemDamageFormat, dbcDamageOneHand);
    LOAD_DB(format("%s/ItemDamageOneHandCaster.dbc", datapath).c_str(), ItemDamageFormat, dbcDamageOneHandCaster);
    LOAD_DB(format("%s/ItemDamageRanged.dbc", datapath).c_str(), ItemDamageFormat, dbcDamageRanged);
    LOAD_DB(format("%s/ItemDamageThrown.dbc", datapath).c_str(), ItemDamageFormat, dbcDamageThrown);
    LOAD_DB(format("%s/ItemDamageTwoHand.dbc", datapath).c_str(), ItemDamageFormat, dbcDamageTwoHand);
    LOAD_DB(format("%s/ItemDamageTwoHandCaster.dbc", datapath).c_str(), ItemDamageFormat, dbcDamageTwoHandCaster);
    LOAD_DB(format("%s/ItemDamageWand.dbc", datapath).c_str(), ItemDamageFormat, dbcDamageDamageWand);
    LOAD_DB(format("%s/ItemExtendedCost.dbc", datapath).c_str(), itemextendedcostFormat, dbcItemExtendedCost);
    LOAD_DB(format("%s/ItemLimitCategory.dbc", datapath).c_str(), itemlimitcategoryFormat, dbcItemLimitCategory);
    LOAD_DB(format("%s/ItemRandomProperties.dbc", datapath).c_str(), itemrandompropertiesFormat, dbcItemRandomProperties);
    LOAD_DB(format("%s/ItemRandomSuffix.dbc", datapath).c_str(), itemrandomsuffixFormat, dbcItemRandomSuffix);
    LOAD_DB(format("%s/ItemSet.dbc", datapath).c_str(), ItemSetFormat, dbcItemSet);
    LOAD_DB(format("%s/Lock.dbc", datapath).c_str(), LockFormat, dbcLock);
    LOAD_DB(format("%s/LFGDungeons.dbc", datapath).c_str(), lfgdungeonsFormat, dbcLFGDungeons);
    LOAD_DB(format("%s/Map.dbc", datapath).c_str(), mapentryFormat, dbcMap);
    LOAD_DB(format("%s/GlyphProperties.dbc", datapath).c_str(), glypepropertiesFormat, dbcGlyphProperties);
    LOAD_DB(format("%s/NumTalentsAtLevel.dbc", datapath).c_str(), numtalentsFormat, dbcNumTalents);
    LOAD_DB(format("%s/SkillLine.dbc", datapath).c_str(), SkillLineEntryFormat, dbcSkillLine);
    LOAD_DB(format("%s/SkillLineAbility.dbc", datapath).c_str(), SkillLineSpellFormat, dbcSkillLineSpell);
    LOAD_DB(format("%s/gtBarberShopCostBase.dbc", datapath).c_str(), gtFloatFormat, dbcBarberShopPrices);
    LOAD_DB(format("%s/gtChanceToMeleeCrit.dbc", datapath).c_str(), gtFloatFormat, dbcMeleeCrit);
    LOAD_DB(format("%s/gtChanceToMeleeCritBase.dbc", datapath).c_str(), gtFloatFormat, dbcMeleeCritBase);
    LOAD_DB(format("%s/gtChanceToSpellCrit.dbc", datapath).c_str(), gtFloatFormat, dbcSpellCrit);
    LOAD_DB(format("%s/gtChanceToSpellCritBase.dbc", datapath).c_str(), gtFloatFormat, dbcSpellCritBase);
    LOAD_DB(format("%s/gtCombatRatings.dbc", datapath).c_str(), gtFloatFormat, dbcCombatRating);
    LOAD_DB(format("%s/gtOCTHpPerStamina.dbc", datapath).c_str(), gtFloatFormat, dbcHPRegen);
    LOAD_DB(format("%s/gtOCTRegenMP.dbc", datapath).c_str(), gtFloatFormat, dbcManaRegen);
    LOAD_DB(format("%s/gtRegenMPPerSpt.dbc", datapath).c_str(), gtFloatFormat, dbcManaRegenBase);
    LOAD_DB(format("%s/gtOCTClassCombatRatingScalar.dbc", datapath).c_str(), gtFloatFormat, dbcCombatRatingScaling);
    LOAD_DB(format("%s/Spell.dbc", datapath).c_str(), spellentryFormat, dbcSpell);
    LOAD_DB(format("%s/SpellAuraOptions.dbc", datapath).c_str(), spellauraoptionFormat, dbcSpellAuraOptions);
    LOAD_DB(format("%s/SpellAuraRestrictions.dbc", datapath).c_str(), spellaurarestrictionFormat, dbcSpellAuraRestrictions);
    LOAD_DB(format("%s/SpellCastingRequirements.dbc", datapath).c_str(), spellcastingrequirementFormat, dbcSpellCastingRequirements);
    LOAD_DB(format("%s/SpellCastTimes.dbc", datapath).c_str(), spellcasttimeFormat, dbcSpellCastTime);
    LOAD_DB(format("%s/SpellCategories.dbc", datapath).c_str(), spellcategoryFormat, dbcSpellCategories);
    LOAD_DB(format("%s/SpellClassOptions.dbc", datapath).c_str(), spellclassoptionFormat, dbcSpellClassOptions);
    LOAD_DB(format("%s/SpellCooldowns.dbc", datapath).c_str(), spellcooldownFormat, dbcSpellCooldowns);
    LOAD_DB(format("%s/SpellDifficulty.dbc", datapath).c_str(), spelldifficultyFormat, dbcSpellDifficulty);
    LOAD_DB(format("%s/SpellDuration.dbc", datapath).c_str(), spelldurationFormat, dbcSpellDuration);
    LOAD_DB(format("%s/SpellEffect.dbc", datapath).c_str(), spelleffectFormat, dbcSpellEffect);
    LOAD_DB(format("%s/SpellEquippedItems.dbc", datapath).c_str(), spellequippeditemFormat, dbcSpellEquippedItems);
    LOAD_DB(format("%s/SpellInterrupts.dbc", datapath).c_str(), spellinterruptFormat, dbcSpellInterrupts);
    LOAD_DB(format("%s/SpellItemEnchantment.dbc", datapath).c_str(), EnchantEntryFormat, dbcEnchant);
    LOAD_DB(format("%s/SpellLevels.dbc", datapath).c_str(), spelllevelFormat, dbcSpellLevels);
    LOAD_DB(format("%s/SpellPower.dbc", datapath).c_str(), spellpowerFormat, dbcSpellPower);
    LOAD_DB(format("%s/SpellRadius.dbc", datapath).c_str(), spellradiusFormat, dbcSpellRadius);
    LOAD_DB(format("%s/SpellRange.dbc", datapath).c_str(), spellrangeFormat, dbcSpellRange);
    LOAD_DB(format("%s/SpellReagents.dbc", datapath).c_str(), spellreagentFormat, dbcSpellReagents);
    LOAD_DB(format("%s/SpellRunecost.dbc", datapath).c_str(), spellrunecostFormat, dbcSpellRuneCost);
    LOAD_DB(format("%s/SpellScaling.dbc", datapath).c_str(), spellscalingFormat, dbcSpellScaling);
    LOAD_DB(format("%s/SpellShapeshift.dbc", datapath).c_str(), spellshapeshiftFormat, dbcSpellShapeshift);
    LOAD_DB(format("%s/SpellShapeshiftForm.dbc", datapath).c_str(), spellshapeshiftformFormat, dbcSpellShapeshiftForm);
    LOAD_DB(format("%s/SpellTargetRestrictions.dbc", datapath).c_str(), spelltargetrestrictionFormat, dbcSpellTargetRestrictions);
    LOAD_DB(format("%s/SpellTotems.dbc", datapath).c_str(), spelltotemFormat, dbcSpellTotems);
    LOAD_DB(format("%s/SummonProperties.dbc", datapath).c_str(), summonpropertiesFormat, dbcSummonProperties);
    LOAD_DB(format("%s/Talent.dbc", datapath).c_str(), talententryFormat, dbcTalent);
    LOAD_DB(format("%s/TalentTab.dbc", datapath).c_str(), talenttabentryFormat, dbcTalentTab);
    LOAD_DB(format("%s/TalentTreePrimarySpells.dbc", datapath).c_str(), talenttreeFormat, dbcTreePrimarySpells);
    LOAD_DB(format("%s/TaxiNodes.dbc", datapath).c_str(), dbctaxinodeFormat, dbcTaxiNode);
    LOAD_DB(format("%s/TaxiPath.dbc", datapath).c_str(), dbctaxipathFormat, dbcTaxiPath);
    LOAD_DB(format("%s/TaxiPathNode.dbc", datapath).c_str(), dbctaxipathnodeFormat, dbcTaxiPathNode);
    LOAD_DB(format("%s/Vehicle.dbc", datapath).c_str(), vehicleentryFormat, dbcVehicle);
    LOAD_DB(format("%s/VehicleSeat.dbc", datapath).c_str(), vehicleseatentryFormat, dbcVehicleSeat);
    LOAD_DB(format("%s/WMOAreaTable.dbc", datapath).c_str(), wmoareatableFormat, dbcWMOAreaTable);
    LOAD_DB(format("%s/WorldMapOverlay.dbc", datapath).c_str(), worldmapoverlayFormat, dbcWorldMapOverlay);
    LOAD_DB(format("%s/WorldSafeLocs.dbc", datapath).c_str(), worldsafeLocationsFormat, dbcWorldSafeLocs);
    return true;
}
