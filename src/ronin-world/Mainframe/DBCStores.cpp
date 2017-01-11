/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

initialiseSingleton(DBCLoader);

#define DECLARE_CLASS_INTERNAL_DBC_MACRO(EntryClass, DeclaredClass) SERVER_DECL DBStorage<EntryClass, DBC<EntryClass>> DeclaredClass
#define DECLARE_CLASS_INTERNAL_DB2_MACRO(EntryClass, DeclaredClass) SERVER_DECL DBStorage<EntryClass, DB2<EntryClass>> DeclaredClass

DECLARE_CLASS_INTERNAL_DBC_MACRO(AchievementEntry, dbcAchievement);
static const char *achievementFormat = "uuuussuuxuxxuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(AchievementCriteriaEntry, dbcAchievementCriteria);
static const char *achievementcriteriaFormat = "iiiiliixxsiiiiixxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(AreaGroupEntry, dbcAreaGroup);
static const char *areagroupFormat = "iiiiiiii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(AreaTableEntry, dbcAreaTable);
static const char *areatableFormat = "uuuuuxxxxxusuuuuuxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(AreaTriggerEntry, dbcAreaTrigger);
static const char *areatriggerFormat = "uufffuuxfffff";

DECLARE_CLASS_INTERNAL_DBC_MACRO(AuctionHouseEntry, dbcAuctionHouse);
static const char *auctionhousedbcFormat = "uuuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(BankSlotPriceEntry, dbcBankSlotPrices);
static const char *bankslotpriceformat = "uu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(BarberShopStyleEntry, dbcBarberShopStyle);
static const char *barbershopstyleFormat = "iusxxuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(BattleMasterListEntry, dbcBattleMasterList);
static const char *battlemasterlistFormat = "uiiiiiiiiuxsuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CharClassEntry, dbcCharClass);
static const char *charclassFormat = "uuxsxxxuxuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CharPowerTypeEntry, dbcCharPowerType);
static const char *charpowertypeFormat = "uuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CharRaceEntry, dbcCharRace);
static const char *charraceFormat = "uxuxuuxxxxxxuusxxxxxuxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CharStartOutfitEntry, dbcCharStartOutfit);
static const char *charstartoutfitFormat = "ubbbbuuuuuuuuuuuuuuuuuuuuuuuuxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CharTitleEntry, dbcCharTitle);
static const char *chartitlesFormat = "uxsxux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ChatChannelEntry, dbcChatChannels);
static const char *chatchannelformat = "uuxsx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CreatureBoundDataEntry, dbcCreatureBoundData);
static const char *creatureboundFormat = "ufffffff";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CreatureDisplayInfoEntry, dbcCreatureDisplayInfo);
static const char *creaturedisplayinfoFormat = "uxxufxxxxxixxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CreatureDisplayInfoExtraEntry, dbcCreatureDisplayInfoExtra);
static const char *creaturedisplayinfoextraFormat = "uuxxxxxxxxxxxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CreatureFamilyEntry, dbcCreatureFamily);
static const char *creaturefamilyFormat = "ufufuuuuuxsx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CreatureSpellDataEntry, dbcCreatureSpellData);
static const char *creaturespelldataFormat = "uuuuuxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(CurrencyTypeEntry, dbcCurrencyType);
static const char *currencytypeFormat = "iisxxiiiiix";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DestructibleModelDataEntry, dbcDestructibleModelData);
static const char *destructiblemodeldataFormat = "uuxxxuxxxxuxxxxuxxxxuxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DungeonEncounterEntry, dbcDungeonEncounter);
static const char *dungeonencounterFormat = "uuuuusxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DurabilityCostsEntry, dbcDurabilityCosts);
static const char *durabilitycostsFormat = "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(DurabilityQualityEntry, dbcDurabilityQuality);
static const char *durabilityqualityFormat = "uf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(EmoteEntry, dbcEmote);
static const char *emoteFormat = "uxxuuuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(EmoteTextEntry, dbcEmoteText);
static const char *emotetextFormat = "uxuxxxxxxxxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(FactionEntry, dbcFaction);
static const char *factionFormat = "uiuuuuuuuuiiiiuuuuuxxxxsxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(FactionTemplateEntry, dbcFactionTemplate);
static const char *factiontemplateFormat = "uuuuuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(GameObjectDisplayInfoEntry, dbcGameObjectDisplayInfo);
static const char *gameobjectdisplayinfoFormat = "usxxxxxxxxxxffffffxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(GemPropertyEntry, dbcGemProperty);
static const char *gempropertyFormat = "uuxxux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(GlyphProperties, dbcGlyphProperties);
static const char *glypepropertiesFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DB2_MACRO(ItemDataEntry, db2Item);
static const char *itemFormat = "uuuiiuuu";

DECLARE_CLASS_INTERNAL_DB2_MACRO(ItemSparseEntry, db2ItemSparse);
static const char *itemsparseFormat = "uuuuffuuuuiiuiuuuuuuuuuuiiiiiiiiiiuuuuuuuuuuiiiiiiiiiiiiiiiiiiiiuuufiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiusssssuuuuuiuuuuuuuuuuuuuuiufuuufii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ArmorLocationEntry, dbcArmorLocation);
static const char *armorlocationFormat = "ufffff";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemArmorQuality, dbcArmorQuality);
static const char *armorqualityFormat = "ufffffffu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemArmorShield, dbcArmorShield);
static const char *armorshieldFormat = "uuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemArmorTotal, dbcArmorTotal);
static const char *armortotalFormat = "uuffff";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageAmmo);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageOneHand);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageOneHandCaster);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageRanged);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageThrown);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageTwoHand);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageTwoHandCaster);
DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemDamageEntry, dbcDamageDamageWand);
static const char *itemdamageFormat = "ufffffffu";

DECLARE_CLASS_INTERNAL_DB2_MACRO(ItemExtendedCostEntry, dbcItemExtendedCost);
static const char *itemextendedcostFormat = "uxxuuuuuuuuuuuuxuuuuuuuuuuxxuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemLimitCategoryEntry, dbcItemLimitCategory);
static const char *itemlimitcategoryFormat = "ixuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemRandomPropertiesEntry, dbcItemRandomProperties);
static const char *itemrandompropertiesFormat = "uxuuuuus";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemRandomSuffixEntry, dbcItemRandomSuffix);
static const char *itemrandomsuffixFormat = "usxuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemReforgeEntry, dbcItemReforge);
static const char *itemreforgeFormat = "uufuf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ItemSetEntry, dbcItemSet);
static const char *itemsetFormat = "usuuuuuuuuuxxxxxxxxuuuuuuuuuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(LFGDungeonsEntry, dbcLFGDungeons);
static const char *lfgdungeonsFormat = "usuuuuuiuuuuxuxuxuxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(LockEntry, dbcLock);
static const char *lockFormat = "uuuuuuuuuuuuuuuuuuuuuuuuuxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(MapEntry, dbcMap);
static const char *mapentryFormat = "usuuxxsussxxiffxuxui";

DECLARE_CLASS_INTERNAL_DBC_MACRO(MapDifficultyEntry, dbcMapDifficulty);
static const char *mapdifficultyFormat = "uuuxuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(MountCapabilityEntry, dbcMountCapability);
static const char *mountcapabilityFormat = "uuuuuuui";

DECLARE_CLASS_INTERNAL_DBC_MACRO(MountTypeEntry, dbcMountType);
static const char *mounttypeFormat = "uuuuuuuuuuuuuuuuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(NumTalentsAtLevelEntry, dbcNumTalents);
static const char *numtalentsFormat="uf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(OverrideSpellDataEntry, dbcOverrideSpellData);
static const char *overridespelldataFormat="uuuuuuuuuuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(PhaseEntry, dbcPhase);
static const char *phaseFormat="uuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(PvPDifficultyEntry, dbcPvPDifficulty);
static const char *pvpdifficultyFormat="xuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(QuestFactionRewardEntry, dbcQuestFactionReward);
static const char *questfactionrewardFormat="uiiiiiiiiii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(QuestXPLevelEntry, dbcQuestXP);
static const char *questxpFormat="uiiiiiiiiii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ScalingStatDistributionEntry, dbcScalingStatDistribution);
static const char *scalingstatdistributionFormat = "uiiiiiiiiiiuuuuuuuuuuxu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(ScalingStatValuesEntry, dbcScalingStatValues);
static const char *scalingstatvaluesFormat = "xuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SkillRaceClassInfoEntry, dbcSkillRaceClassInfo);
static const char *skillraceclassinfoFormat = "xuuuuuxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SkillLineEntry, dbcSkillLine);
static const char *skilllineFormat = "uusxuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SkillLineAbilityEntry, dbcSkillLineSpell);
static const char *skilllinespellFormat = "uuuuuxxuuuuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellEntry, dbcSpell);
static const char *spellentryFormat = "uuuuuuuuuuuuuuuufuuuussssuuxxuxuuuuuuuxuuuuuuuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellAuraOptionsEntry, dbcSpellAuraOptions);
static const char *spellauraoptionFormat = "uuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellAuraRestrictionsEntry, dbcSpellAuraRestrictions);
static const char *spellaurarestrictionFormat = "uuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCastingRequirementsEntry, dbcSpellCastingRequirements);
static const char *spellcastingrequirementFormat = "uuxxixu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCastTimeEntry, dbcSpellCastTime);
static const char *spellcasttimeFormat = "uuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCategoriesEntry, dbcSpellCategories);
static const char *spellcategoryFormat = "uuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellClassOptionsEntry, dbcSpellClassOptions);
static const char *spellclassoptionFormat = "uxuuuus";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellCooldownsEntry, dbcSpellCooldowns);
static const char *spellcooldownFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellDifficultyEntry, dbcSpellDifficulty);
static const char *spelldifficultyFormat = "uuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellDurationEntry, dbcSpellDuration);
static const char *spelldurationFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellEffectEntry, dbcSpellEffect);
static const char *spelleffectFormat = "uufuuiffuiuuiifuufuuuuuuuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellEquippedItemsEntry, dbcSpellEquippedItems);
static const char *spellequippeditemFormat = "uiii";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellInterruptsEntry, dbcSpellInterrupts);
static const char *spellinterruptFormat = "uuxuxu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellItemEnchantEntry, dbcSpellItemEnchant);
static const char *spellitemenchantFormat = "uxuuuuuuxxxuuusuuuuuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellItemEnchantConditionEntry, dbcSpellItemEnchantCondition);
static const char *spellitemenchantconditionFormat = "ubbbpppppxxxxxbbbppbbbPPuuuxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellLevelsEntry, dbcSpellLevels);
static const char *spelllevelFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellPowerEntry, dbcSpellPower);
static const char *spellpowerFormat = "uuuuuxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellRadiusEntry, dbcSpellRadius);
static const char *spellradiusFormat = "ufxf";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellRangeEntry, dbcSpellRange);
static const char *spellrangeFormat = "uffffxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellReagentsEntry, dbcSpellReagents);
static const char *spellreagentFormat = "uiiiiiiiiuuuuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellRuneCostEntry, dbcSpellRuneCost);
static const char *spellrunecostFormat = "uuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellScalingEntry, dbcSpellScaling);
static const char *spellscalingFormat = "uiiuiffffffffffu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellShapeshiftEntry, dbcSpellShapeshift);
static const char *spellshapeshiftFormat = "uuxuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellShapeshiftFormEntry, dbcSpellShapeshiftForm);
static const char *spellshapeshiftformFormat = "uxxuixuuuxxuuuuuuuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellTargetRestrictionsEntry, dbcSpellTargetRestrictions);
static const char *spelltargetrestrictionFormat = "uuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SpellTotemsEntry, dbcSpellTotems);
static const char *spelltotemFormat = "uuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(SummonPropertiesEntry, dbcSummonProperties);
static const char *summonpropertiesFormat = "uuuuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TalentEntry, dbcTalent);
static const char *talentFormat = "uuuuuuuuuuxxuxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TalentTabEntry, dbcTalentTab);
static const char *talenttabFormat = "uxxuuuxxuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TalentTreePrimarySpellsEntry, dbcTreePrimarySpells);
static const char *talenttreeFormat = "uuux";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TaxiNodeEntry, dbcTaxiNode);
static const char *dbctaxinodeFormat = "uufffxuuxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TaxiPathEntry, dbcTaxiPath);
static const char *dbctaxipathFormat = "uuuu";

DECLARE_CLASS_INTERNAL_DBC_MACRO(TaxiPathNodeEntry, dbcTaxiPathNode);
static const char *dbctaxipathnodeFormat = "uuuufffuuxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(VehicleEntry, dbcVehicle);
static const char *vehicleFormat = "uuffffuuuuuuuufffffffffffffffssssfifiixx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(VehicleSeatEntry, dbcVehicleSeat);
static const char *vehicleseatFormat = "uuiffffffffffiiiiiifffffffiiifffiiiiiiiffuuuiuxxxxxxxxxxxxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(WMOAreaTableEntry, dbcWMOAreaTable);
static const char *wmoareatableFormat="iiiixxxxxiixxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(WorldMapOverlayEntry, dbcWorldMapOverlay);
static const char *worldmapoverlayFormat="uxuuuuxxxxxxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(WorldMapAreaOverlayEntry, dbcWorldAreaOverlay);
static const char *worldareaoverlayFormat="iinsffffixxxxx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(WorldSafeLocsEntry, dbcWorldSafeLocs);
static const char *worldsafeLocationsFormat="iifffx";

DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcCombatRating);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcBarberShopPrices);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcMeleeCrit);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcMeleeCritBase);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcSpellCrit);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcSpellCritBase);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcManaRegen);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcManaRegenBase);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcSpellScalar);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcHPPerStam);
DECLARE_CLASS_INTERNAL_DBC_MACRO(gtFloat, dbcCombatRatingScaling);
static const char *gtFloatFormat = "uf";

int32 SpellEntry::CalculateSpellPoints(uint8 effIndex, int32 level, int32 comboPoints)
{
    int32 basePoints = 0;
    float comboDamage = 0.0f;
    gtFloat *gtScalingEntry = NULL;
    if (coeff[0][effIndex] > 0.f)
    {
        uint32 gtSpellScalingId = level - 1;
        if (playerClass == -1)
            gtSpellScalingId += 1100;
        else gtSpellScalingId += (playerClass - 1) * 100;
        gtScalingEntry = dbcSpellScalar.LookupEntry(gtSpellScalingId);
    }

    if (gtScalingEntry)
    {
        float scale = gtScalingEntry->val;
        if (castTimeMax > 0 && castScalingMaxLevel > level)
            scale *= float(castTimeMin + float(level - 1) * (castTimeMax - castTimeMin) / (castScalingMaxLevel - 1)) / float(castTimeMax);
        if (coefLevelBase > level) scale *= (1.0f - coefBase) * (level - 1) / (coefLevelBase - 1) + coefBase;

        basePoints = int32(coeff[0][effIndex] * scale);
        int32 randomPoints = int32(coeff[0][effIndex] * scale * coeff[1][effIndex]);
        basePoints += ((RandomFloat(2.f)-1.f)*randomPoints) / 2;
        comboDamage = uint32(coeff[2][effIndex] * scale);
    }
    else
    {
        if(spellLevelMaxLevel == 0 && spellLevelBaseLevel == spellLevelSpellLevel)
            level = spellLevelSpellLevel-1;
        else
        {
            if (spellLevelMaxLevel)
                level = std::min<int32>(level, spellLevelMaxLevel);
            level = std::max<int32>(level, spellLevelBaseLevel);
            level = std::max<int32>(level, spellLevelSpellLevel) - spellLevelSpellLevel;
        }

        basePoints = EffectBasePoints[effIndex];
        basePoints += int32(level * EffectRealPointsPerLevel[effIndex]);
        comboDamage = EffectPointsPerComboPoint[effIndex];
        if(int32 randomPoints = int32(EffectDieSides[effIndex]))
        {
            switch (randomPoints)
            {
            case 1: basePoints += 1; break;                     // range 1..1
            default:
                {
                    // range can have positive (1..rand) and negative (rand..1) values, so order its for irand
                    int32 randvalue = (randomPoints >= 1) ? (rand() % randomPoints) : -(rand() % -randomPoints);
                    basePoints += randvalue;
                }break;
            }
        }
    }

    basePoints += int32(comboDamage * comboPoints);

    if (!gtScalingEntry && HasAttribute(0, 0x00080000) && spellLevelSpellLevel && Effect[effIndex] != SPELL_EFFECT_WEAPON_PERCENT_DAMAGE && Effect[effIndex] != SPELL_EFFECT_KNOCK_BACK
        && (Effect[effIndex] != SPELL_EFFECT_APPLY_AURA || EffectApplyAuraName[effIndex] != SPELL_AURA_MOD_DECREASE_SPEED))
        basePoints = int32(basePoints * 0.25f * exp(level * (70 - spellLevelSpellLevel) / 1000.0f));
    return basePoints;
}

template<class T> void DBCLoader::LoadDBC(bool *result, std::string filename, const char * format, T *l)
{
    l->Init(filename.c_str(), format);
    // Result must only be set to false
    if(!l->Load()) *result = false;
}

//#define DISABLE_DBC_MULTILOADING
#ifdef DISABLE_DBC_MULTILOADING
#define ADD_LOAD_DB(filename, format, stor) LoadDBC(result, filename, format, &stor)
#else
#define ADD_LOAD_DB(filename, format, stor) tl.AddTask(new Task(new CallbackP4<DBCLoader, bool*, std::string, const char*, decltype(stor)*>(this, &DBCLoader::LoadDBC, result, filename, format, &stor)))
#endif

void DBCLoader::FillDBCLoadList(TaskList &tl, const char* datapath, bool *result)
{
    ADD_LOAD_DB(format("%s/Achievement.dbc", datapath), achievementFormat, dbcAchievement);
    ADD_LOAD_DB(format("%s/Achievement_Criteria.dbc", datapath), achievementcriteriaFormat, dbcAchievementCriteria);
    ADD_LOAD_DB(format("%s/AreaGroup.dbc", datapath), areagroupFormat, dbcAreaGroup);
    ADD_LOAD_DB(format("%s/AreaTable.dbc", datapath), areatableFormat, dbcAreaTable);
    ADD_LOAD_DB(format("%s/AreaTrigger.dbc", datapath), areatriggerFormat, dbcAreaTrigger);
    ADD_LOAD_DB(format("%s/AuctionHouse.dbc", datapath), auctionhousedbcFormat, dbcAuctionHouse);
    ADD_LOAD_DB(format("%s/BankBagSlotPrices.dbc", datapath), bankslotpriceformat, dbcBankSlotPrices);
    ADD_LOAD_DB(format("%s/BarberShopStyle.dbc", datapath), barbershopstyleFormat, dbcBarberShopStyle);
    ADD_LOAD_DB(format("%s/BattlemasterList.dbc", datapath), battlemasterlistFormat, dbcBattleMasterList);
    ADD_LOAD_DB(format("%s/ChatChannels.dbc", datapath), chatchannelformat, dbcChatChannels);
    ADD_LOAD_DB(format("%s/CharStartOutfit.dbc", datapath), charstartoutfitFormat, dbcCharStartOutfit);
    ADD_LOAD_DB(format("%s/CharTitles.dbc", datapath), chartitlesFormat, dbcCharTitle);
    ADD_LOAD_DB(format("%s/ChrClasses.dbc", datapath), charclassFormat, dbcCharClass);
    ADD_LOAD_DB(format("%s/ChrClassesXPowerTypes.dbc", datapath), charpowertypeFormat, dbcCharPowerType);
    ADD_LOAD_DB(format("%s/ChrRaces.dbc", datapath), charraceFormat, dbcCharRace);
    ADD_LOAD_DB(format("%s/CreatureBoundInformation.dbc", datapath), creatureboundFormat, dbcCreatureBoundData);
    ADD_LOAD_DB(format("%s/CreatureDisplayInfo.dbc", datapath), creaturedisplayinfoFormat, dbcCreatureDisplayInfo);
    ADD_LOAD_DB(format("%s/CreatureDisplayInfoExtra.dbc", datapath), creaturedisplayinfoextraFormat, dbcCreatureDisplayInfoExtra);
    ADD_LOAD_DB(format("%s/CreatureFamily.dbc", datapath), creaturefamilyFormat, dbcCreatureFamily);
    ADD_LOAD_DB(format("%s/CreatureSpellData.dbc", datapath), creaturespelldataFormat, dbcCreatureSpellData);
    ADD_LOAD_DB(format("%s/CurrencyTypes.dbc", datapath), currencytypeFormat, dbcCurrencyType);
    ADD_LOAD_DB(format("%s/DestructibleModelData.dbc", datapath), destructiblemodeldataFormat, dbcDestructibleModelData);
    ADD_LOAD_DB(format("%s/DungeonEncounter.dbc", datapath), dungeonencounterFormat, dbcDungeonEncounter);
    ADD_LOAD_DB(format("%s/DurabilityCosts.dbc", datapath), durabilitycostsFormat, dbcDurabilityCosts);
    ADD_LOAD_DB(format("%s/DurabilityQuality.dbc", datapath), durabilityqualityFormat, dbcDurabilityQuality);
    ADD_LOAD_DB(format("%s/Emotes.dbc", datapath), emoteFormat, dbcEmote);
    ADD_LOAD_DB(format("%s/EmotesText.dbc", datapath), emotetextFormat, dbcEmoteText);
    ADD_LOAD_DB(format("%s/Faction.dbc", datapath), factionFormat, dbcFaction);
    ADD_LOAD_DB(format("%s/FactionTemplate.dbc", datapath), factiontemplateFormat, dbcFactionTemplate);
    ADD_LOAD_DB(format("%s/GameObjectDisplayInfo.dbc", datapath), gameobjectdisplayinfoFormat, dbcGameObjectDisplayInfo);
    ADD_LOAD_DB(format("%s/GemProperties.dbc", datapath), gempropertyFormat, dbcGemProperty);
    ADD_LOAD_DB(format("%s/GlyphProperties.dbc", datapath), glypepropertiesFormat, dbcGlyphProperties);
    ADD_LOAD_DB(format("%s/Item.db2", datapath), itemFormat, db2Item);
    ADD_LOAD_DB(format("%s/Item-sparse.db2", datapath), itemsparseFormat, db2ItemSparse);
    ADD_LOAD_DB(format("%s/ArmorLocation.dbc", datapath), armorlocationFormat, dbcArmorLocation);
    ADD_LOAD_DB(format("%s/ItemArmorQuality.dbc", datapath), armorqualityFormat, dbcArmorQuality);
    ADD_LOAD_DB(format("%s/ItemArmorShield.dbc", datapath), armorshieldFormat, dbcArmorShield);
    ADD_LOAD_DB(format("%s/ItemArmorTotal.dbc", datapath), armortotalFormat, dbcArmorTotal);
    ADD_LOAD_DB(format("%s/ItemDamageAmmo.dbc", datapath), itemdamageFormat, dbcDamageAmmo);
    ADD_LOAD_DB(format("%s/ItemDamageOneHand.dbc", datapath), itemdamageFormat, dbcDamageOneHand);
    ADD_LOAD_DB(format("%s/ItemDamageOneHandCaster.dbc", datapath), itemdamageFormat, dbcDamageOneHandCaster);
    ADD_LOAD_DB(format("%s/ItemDamageRanged.dbc", datapath), itemdamageFormat, dbcDamageRanged);
    ADD_LOAD_DB(format("%s/ItemDamageThrown.dbc", datapath), itemdamageFormat, dbcDamageThrown);
    ADD_LOAD_DB(format("%s/ItemDamageTwoHand.dbc", datapath), itemdamageFormat, dbcDamageTwoHand);
    ADD_LOAD_DB(format("%s/ItemDamageTwoHandCaster.dbc", datapath), itemdamageFormat, dbcDamageTwoHandCaster);
    ADD_LOAD_DB(format("%s/ItemDamageWand.dbc", datapath), itemdamageFormat, dbcDamageDamageWand);
    ADD_LOAD_DB(format("%s/ItemExtendedCost.db2", datapath), itemextendedcostFormat, dbcItemExtendedCost);
    ADD_LOAD_DB(format("%s/ItemLimitCategory.dbc", datapath), itemlimitcategoryFormat, dbcItemLimitCategory);
    ADD_LOAD_DB(format("%s/ItemRandomProperties.dbc", datapath), itemrandompropertiesFormat, dbcItemRandomProperties);
    ADD_LOAD_DB(format("%s/ItemRandomSuffix.dbc", datapath), itemrandomsuffixFormat, dbcItemRandomSuffix);
    ADD_LOAD_DB(format("%s/ItemReforge.dbc", datapath), itemreforgeFormat, dbcItemReforge);
    ADD_LOAD_DB(format("%s/ItemSet.dbc", datapath), itemsetFormat, dbcItemSet);
    ADD_LOAD_DB(format("%s/LFGDungeons.dbc", datapath), lfgdungeonsFormat, dbcLFGDungeons);
    ADD_LOAD_DB(format("%s/Lock.dbc", datapath), lockFormat, dbcLock);
    ADD_LOAD_DB(format("%s/Map.dbc", datapath), mapentryFormat, dbcMap);
    ADD_LOAD_DB(format("%s/MapDifficulty.dbc", datapath), mapdifficultyFormat, dbcMapDifficulty);
    ADD_LOAD_DB(format("%s/MountCapability.dbc", datapath), mountcapabilityFormat, dbcMountCapability);
    ADD_LOAD_DB(format("%s/MountType.dbc", datapath), mounttypeFormat, dbcMountType);
    ADD_LOAD_DB(format("%s/NumTalentsAtLevel.dbc", datapath), numtalentsFormat, dbcNumTalents);
    ADD_LOAD_DB(format("%s/OverrideSpellData.dbc", datapath), overridespelldataFormat, dbcOverrideSpellData);
    ADD_LOAD_DB(format("%s/Phase.dbc", datapath), phaseFormat, dbcPhase);
    ADD_LOAD_DB(format("%s/PvpDifficulty.dbc", datapath), pvpdifficultyFormat, dbcPvPDifficulty);
    ADD_LOAD_DB(format("%s/QuestFactionReward.dbc", datapath), questfactionrewardFormat, dbcQuestFactionReward);
    ADD_LOAD_DB(format("%s/QuestXP.dbc", datapath), questxpFormat, dbcQuestXP);
    ADD_LOAD_DB(format("%s/ScalingStatDistribution.dbc", datapath), scalingstatdistributionFormat, dbcScalingStatDistribution);
    ADD_LOAD_DB(format("%s/ScalingStatValues.dbc", datapath), scalingstatvaluesFormat, dbcScalingStatValues);
    ADD_LOAD_DB(format("%s/SkillRaceClassInfo.dbc", datapath), skillraceclassinfoFormat, dbcSkillRaceClassInfo);
    ADD_LOAD_DB(format("%s/SkillLine.dbc", datapath), skilllineFormat, dbcSkillLine);
    ADD_LOAD_DB(format("%s/SkillLineAbility.dbc", datapath), skilllinespellFormat, dbcSkillLineSpell);
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
    ADD_LOAD_DB(format("%s/SpellItemEnchantment.dbc", datapath), spellitemenchantFormat, dbcSpellItemEnchant);
    ADD_LOAD_DB(format("%s/SpellItemEnchantmentCondition.dbc", datapath), spellitemenchantconditionFormat, dbcSpellItemEnchantCondition);
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
    ADD_LOAD_DB(format("%s/Talent.dbc", datapath), talentFormat, dbcTalent);
    ADD_LOAD_DB(format("%s/TalentTab.dbc", datapath), talenttabFormat, dbcTalentTab);
    ADD_LOAD_DB(format("%s/TalentTreePrimarySpells.dbc", datapath), talenttreeFormat, dbcTreePrimarySpells);
    ADD_LOAD_DB(format("%s/TaxiNodes.dbc", datapath), dbctaxinodeFormat, dbcTaxiNode);
    ADD_LOAD_DB(format("%s/TaxiPath.dbc", datapath), dbctaxipathFormat, dbcTaxiPath);
    ADD_LOAD_DB(format("%s/TaxiPathNode.dbc", datapath), dbctaxipathnodeFormat, dbcTaxiPathNode);
    ADD_LOAD_DB(format("%s/Vehicle.dbc", datapath), vehicleFormat, dbcVehicle);
    ADD_LOAD_DB(format("%s/VehicleSeat.dbc", datapath), vehicleseatFormat, dbcVehicleSeat);
    ADD_LOAD_DB(format("%s/WMOAreaTable.dbc", datapath), wmoareatableFormat, dbcWMOAreaTable);
    ADD_LOAD_DB(format("%s/WorldMapOverlay.dbc", datapath), worldmapoverlayFormat, dbcWorldMapOverlay);
    ADD_LOAD_DB(format("%s/WorldMapArea.dbc", datapath), worldareaoverlayFormat, dbcWorldAreaOverlay);
    ADD_LOAD_DB(format("%s/WorldSafeLocs.dbc", datapath), worldsafeLocationsFormat, dbcWorldSafeLocs);

    ADD_LOAD_DB(format("%s/gtBarberShopCostBase.dbc", datapath), gtFloatFormat, dbcBarberShopPrices);
    ADD_LOAD_DB(format("%s/gtChanceToMeleeCrit.dbc", datapath), gtFloatFormat, dbcMeleeCrit);
    ADD_LOAD_DB(format("%s/gtChanceToMeleeCritBase.dbc", datapath), gtFloatFormat, dbcMeleeCritBase);
    ADD_LOAD_DB(format("%s/gtChanceToSpellCrit.dbc", datapath), gtFloatFormat, dbcSpellCrit);
    ADD_LOAD_DB(format("%s/gtChanceToSpellCritBase.dbc", datapath), gtFloatFormat, dbcSpellCritBase);
    ADD_LOAD_DB(format("%s/gtCombatRatings.dbc", datapath), gtFloatFormat, dbcCombatRating);
    ADD_LOAD_DB(format("%s/gtOCTHpPerStamina.dbc", datapath), gtFloatFormat, dbcHPPerStam);
    ADD_LOAD_DB(format("%s/gtOCTRegenMP.dbc", datapath), gtFloatFormat, dbcManaRegen);
    ADD_LOAD_DB(format("%s/gtRegenMPPerSpt.dbc", datapath), gtFloatFormat, dbcManaRegenBase);
    ADD_LOAD_DB(format("%s/gtSpellScaling.dbc", datapath), gtFloatFormat, dbcSpellScalar);
    ADD_LOAD_DB(format("%s/gtOCTClassCombatRatingScalar.dbc", datapath), gtFloatFormat, dbcCombatRatingScaling);
}

void DBCLoader::UnloadAllDBCFiles()
{
    dbcAchievement.Unload();
    dbcAchievementCriteria.Unload();
    dbcAreaGroup.Unload();
    dbcAreaTable.Unload();
    dbcAreaTrigger.Unload();
    dbcAuctionHouse.Unload();
    dbcBankSlotPrices.Unload();
    dbcBarberShopStyle.Unload();
    dbcBattleMasterList.Unload();
    dbcChatChannels.Unload();
    dbcCharStartOutfit.Unload();
    dbcCharTitle.Unload();
    dbcCharClass.Unload();
    dbcCharPowerType.Unload();
    dbcCharRace.Unload();
    dbcCreatureBoundData.Unload();
    dbcCreatureDisplayInfo.Unload();
    dbcCreatureDisplayInfoExtra.Unload();
    dbcCreatureFamily.Unload();
    dbcCreatureSpellData.Unload();
    dbcDestructibleModelData.Unload();
    dbcDungeonEncounter.Unload();
    dbcDurabilityCosts.Unload();
    dbcDurabilityQuality.Unload();
    dbcEmote.Unload();
    dbcEmoteText.Unload();
    dbcFaction.Unload();
    dbcFactionTemplate.Unload();
    dbcGameObjectDisplayInfo.Unload();
    dbcGemProperty.Unload();
    dbcGlyphProperties.Unload();
    db2Item.Unload();
    db2ItemSparse.Unload();
    dbcArmorLocation.Unload();
    dbcArmorQuality.Unload();
    dbcArmorShield.Unload();
    dbcArmorTotal.Unload();
    dbcDamageAmmo.Unload();
    dbcDamageOneHand.Unload();
    dbcDamageOneHandCaster.Unload();
    dbcDamageRanged.Unload();
    dbcDamageThrown.Unload();
    dbcDamageTwoHand.Unload();
    dbcDamageTwoHandCaster.Unload();
    dbcDamageDamageWand.Unload();
    dbcItemExtendedCost.Unload();
    dbcItemLimitCategory.Unload();
    dbcItemRandomProperties.Unload();
    dbcItemRandomSuffix.Unload();
    dbcItemSet.Unload();
    dbcLock.Unload();
    dbcMap.Unload();
    dbcMapDifficulty.Unload();
    dbcMountCapability.Unload();
    dbcMountType.Unload();
    dbcNumTalents.Unload();
    dbcOverrideSpellData.Unload();
    dbcPhase.Unload();
    dbcPvPDifficulty.Unload();
    dbcQuestFactionReward.Unload();
    dbcQuestXP.Unload();
    dbcScalingStatDistribution.Unload();
    dbcScalingStatValues.Unload();
    dbcSkillRaceClassInfo.Unload();
    dbcSkillLine.Unload();
    dbcSkillLineSpell.Unload();
    dbcSpell.Unload();
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
    dbcSpellItemEnchant.Unload();
    dbcSpellLevels.Unload();
    dbcSpellPower.Unload();
    dbcSpellRadius.Unload();
    dbcSpellReagents.Unload();
    dbcSpellRuneCost.Unload();
    dbcSpellScaling.Unload();
    dbcSpellShapeshift.Unload();
    dbcSpellShapeshiftForm.Unload();
    dbcSpellTargetRestrictions.Unload();
    dbcSpellTotems.Unload();
    dbcSummonProperties.Unload();
    dbcTalent.Unload();
    dbcTalentTab.Unload();
    dbcTreePrimarySpells.Unload();
    dbcTaxiNode.Unload();
    dbcTaxiPath.Unload();
    dbcTaxiPathNode.Unload();
    dbcVehicle.Unload();
    dbcVehicleSeat.Unload();
    dbcWMOAreaTable.Unload();
    dbcWorldMapOverlay.Unload();
    dbcWorldAreaOverlay.Unload();
    dbcWorldSafeLocs.Unload();
    dbcBarberShopPrices.Unload();
    dbcMeleeCrit.Unload();
    dbcMeleeCritBase.Unload();
    dbcSpellCrit.Unload();
    dbcSpellCritBase.Unload();
    dbcCombatRating.Unload();
    dbcHPPerStam.Unload();
    dbcManaRegen.Unload();
    dbcManaRegenBase.Unload();
    dbcSpellScalar.Unload();
    dbcCombatRatingScaling.Unload();
}

bool DBCUnloader::run()
{
    DBCLoader::UnloadAllDBCFiles();
    return true;
}
