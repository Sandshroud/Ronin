/***
 * Demonstrike Core
 * Citric
 */

#pragma once

#include "DBCAchievementStores.h"

#pragma pack(PRAGMA_PACK)

struct AreaGroupEntry
{
    uint32 AreaGroupId;
    uint32 AreaId[6];
    uint32 nextAreaGroupId;
};

struct AreaTableEntry
{
    uint32 AreaId;
    uint32 mapId;
    uint32 ZoneId;
    uint32 explorationFlag;
    uint32 AreaFlags;
    //uint32 unk[5];
    uint32 level;
    char* name;
    uint32 category;
    uint32 liquidOverrides[4];
    //float unk[2];
    //uint32 unk[7]; // 4.x
};

struct AreaTriggerEntry
{
    uint32 Id;
    uint32 continentId;
    float base_x;
    float base_y;
    float base_z;
    uint32 phaseShift;
    uint32 size;
    //uint32 unk;
    float radius;
    float box_length;
    float box_width;
    float box_height;
    float box_yaw;
};

struct AuctionHouseEntry
{
    uint32 id;
    uint32 faction;
    uint32 fee;
    uint32 tax;
    //char *name;
};

struct BankSlotPriceEntry
{
    uint32 Id;
    uint32 Price;
};

struct BarberShopStyleEntry
{
    uint32 id;                  // 0
    uint32 type;                // 1 value 0 -> hair, value 2 -> facialhair
    char* name;                 // 2 string hairstyle name
//  uint32 name_flags;          // 3
//  float costMultiplier;       // 4
    uint32 race;                // 5 race
    uint32 gender;              // 6 0 male, 1 female
    uint32 hair_id;             // 7 Hair ID
};

struct BattleMasterListEntry
{
    uint32 entry;       // 0
    int32 mapids[8];    // 1-8 Map ids
    uint32 maptype;     // 9 Map flags
//  uint32 queueAsGroup;
    char *name;
    uint32 maxMembers;
    uint32 holidayWSID;
    uint32 minLevel;
    uint32 maxLevel;
    uint32 maxRatedGroupSize;
    uint32 minPlayers;
    uint32 maxPlayers;
    uint32 rated;
};

struct CharClassEntry
{
    uint32 classId;
    uint32 powerType;
    //char *unkname
    char* name;
    //char *name2;
    //char *name3;
    //char *name4;
    uint32 spellFamily;
    //uint32 rangedFlags;
    uint32 cinematicID;
    uint32 requiredExpansion;
    uint32 apPerSTr;
    uint32 apPerAgil;
    uint32 rapPerAgil;
};

struct CharPowerTypeEntry
{
    uint32 entry;   // 0
    uint32 classId; // 1
    uint32 power;   // 2
};

struct CharRaceEntry
{
    uint32 RaceId;
    //uint32 unkFlag;
    uint32 FactionId;
    //uint32 explorationSound;
    uint32 maleModel;
    uint32 femaleModel;
    //char *clientPrefix;
    //uint32 teamOffset;
    //uint32 m_creatureType;
    //uint32 resSicknessSpellId;
    //uint32 splashSoundId;
    //char *clientFileString;
    uint32 CinematicId;
    uint32 TeamId;
    char*  Name;
    //char *name2;
    //char *name3;
    //uint32 facialHair[2];
    //uint32 hairCustomization;
    uint32 requiredExpansion;
    //uint32 unk[3]; // 4.x
};

struct CharStartOutfitEntry
{
    uint32 index;
    uint8 Race, Class, Gender, padding;
    int32 itemId[24];
    //uint32 itemDisplayId[24];
    //uint32 itemInventorySlot[24];
    //uint32 unk[2];
};

struct CharTitleEntry
{
    uint32 Id;
    //uint32 unk1;
    char *titleFormat;
    //char *title2;
    uint32 bit_index;
    //uint32 unk3;
    std::string titleName;
};

struct ChatChannelEntry
{
    uint32 id;
    uint32 flags;
    //uint32 factionGroup;
    char* pattern;
    //char *shortName;
};

struct CreatureBoundDataEntry
{
    uint32 Entry; // Display ID
    float Low[3];
    float High[3];
    float BoundRadius;
};

struct CreatureDisplayInfoEntry
{
    uint32 ID;
    //uint32 ModelData;
    //uint32 Sound;
    uint32 ExtraDisplayInfoEntry;
    float Scale;
    //uint32 unk[12];
};

struct CreatureDisplayInfoExtraEntry
{
    uint32 DisplayExtraId;
    uint32 Race;
    //uint32 Gender;
    //uint32 SkinColor;
    //uint32 FaceType;
    //uint32 HairType;
    //uint32 HairStyle;
    //uint32 FacialHair;
    //uint32 Equipment[11];
    //uint32 canEquip;
    //char *modelName;
};

struct CreatureFamilyEntry
{
    uint32 ID;
    float minsize;
    uint32 minlevel;
    float maxsize;
    uint32 maxlevel;
    uint32 skillLine[2];
    uint32 petDietFlags;
    uint32 petTalentType;
    //uint32 categoryEnumID;
    char* name;
    //char *m_iconFile;
};

struct CreatureSpellDataEntry
{
    uint32 id;
    uint32 Spells[4];
    //uint32 Cooldowns[4];
};

struct CurrencyTypeEntry
{
    uint32 Id;              // 0        not used
    uint32 Category;        // 1        may be category
    char* name;             // 2
    //char* iconName;       // 3
    //uint32 unk4;          // 4        all 0
    uint32 HasSubstitution; // 5        archaeology-related (?)
    uint32 SubstitutionId;  // 6
    uint32 TotalCap;        // 7
    uint32 WeekCap;         // 8
    uint32 Flags;           // 9
    //char* description;    // 10
};

struct DestructibleModelDataEntry
{
    uint32 m_id;
    uint32 normalDisplayId;
    //uint32 unk[3];
    uint32 damagedDisplayId;
    //uint32 unk[4];
    uint32 destroyedDisplayId;
    //uint32 unk[4];
    uint32 rebuildingDisplayId;
    //uint32 unk[4];
    uint32 smokeDisplayId;
    //uint32 unk[3];

    uint32 GetDisplayId(uint8 state)
    {
        switch(state)
        {
        case 4: return smokeDisplayId;
        case 3: return rebuildingDisplayId;
        case 2: return destroyedDisplayId;
        case 1: return damagedDisplayId;
        }
        return normalDisplayId;
    }
};

struct DungeonEncounterEntry
{
    uint32 ID;
    uint32 mapId;
    uint32 difficulty;
    uint32 encounterData;
    uint32 encounterIndex;
    char *encounterName;
    //uint32 nameFlag;
    //uint32 spellIconID;
};

struct DurabilityCostsEntry
{
    uint32 itemlevel;
    uint32 modifier[29];
};

struct DurabilityQualityEntry
{
    uint32 id;
    float quality_modifier;
};

struct EmoteEntry
{
    uint32 Id;
    //char *name;
    //uint32 animationId;
    uint32 flags;
    uint32 emoteType;
    uint32 unitStandState;
    uint32 soundId;
    //uint32 unk; //4.x
};

struct EmoteTextEntry
{
    uint32 Id;
    //char *name;
    uint32 textId;
    //char *textEmote;
    //uint32 unk[15];
};

struct FactionEntry
{
    uint32 ID;
    int32  RepListIndex;
    uint32 RaceMask[4];
    uint32 ClassMask[4];
    int32  baseRepValue[4];
    uint32 repFlags[4];
    uint32 parentFaction;
    //float spilloverRateIn;
    //float spilloverRateOut;
    //uint32 spilloverMaxRankIn;
    //uint32 spilloverRank_unk;
    char*  Name;
    //char *description;
    //uint32 unk;
};

struct FactionTemplateEntry
{
    uint32 ID;
    uint32 Faction;
    uint32 FactionFlags;
    uint32 FactionMask;
    uint32 FriendlyMask;
    uint32 HostileMask;
    uint32 EnemyFactions[4];
    uint32 FriendlyFactions[4];
    uint32 FactionTeam;

    FactionEntry *m_faction;
    FactionEntry *GetFaction() { return m_faction; }
};

struct GameObjectDisplayInfoEntry
{
    uint32 displayId;
    char *fileName;
    //uint32 unk[10];
    float geoBoxData[6];
    //uint32 transportId;
    //uint32 unk[2];
};

struct GemPropertyEntry
{
    uint32 Entry;
    uint32 EnchantmentID;
    //uint32 maxCountInv;
    //uint32 maxCountItem;
    uint32 SocketMask;
    //uint32 unk;
};

struct GlyphProperties
{
    uint32 Id;
    uint32 SpellId;
    uint32 TypeFlags;
    uint32 SpellIcon;
};

struct ItemDataEntry
{
    uint32 ID;
    uint32 Class;
    uint32 SubClass;
    int32 SoundOverrideSubclass;
    int32 Material;
    uint32 DisplayId;
    uint32 InventoryType;
    uint32 Sheath;
};

struct ItemSparseEntry
{
    uint32 ID;
    uint32 Quality;
    uint32 Flags;
    uint32 Flags2;
    float f_unk[2]; // 4.3.x
    uint32 BuyCount;
    uint32 BuyPrice;
    uint32 SellPrice;
    uint32 InventoryType;
    int32  AllowableClass;
    int32  AllowableRace;
    uint32 ItemLevel;
    int32  RequiredLevel;
    uint32 RequiredSkill;
    uint32 RequiredSkillRank;
    uint32 RequiredSpell;
    uint32 RequiredHonorRank;
    uint32 RequiredCityRank;
    uint32 RequiredReputationFaction;
    uint32 RequiredReputationRank;
    uint32 MaxCount;
    uint32 Stackable;
    uint32 ContainerSlots;
    int32  ItemStatType[10];
    uint32 ItemStatValue[10];
    int32  ItemStatType2[10];
    int32  ItemStatValue2[10];
    uint32 ScalingStatDistribution;
    uint32 DamageType;
    uint32 Delay;
    float  RangedModRange;
    int32  SpellId[5];
    int32  SpellTrigger[5];
    int32  SpellCharges[5];
    int32  SpellCooldown[5];
    int32  SpellCategory[5];
    int32  SpellCategoryCooldown[5];
    uint32 Bonding;
    char*  Name;
    char*  Name2;
    char*  Name3;
    char*  Name4;
    char*  Description;
    uint32 PageText;
    uint32 LanguageID;
    uint32 PageMaterial;
    uint32 StartQuest;
    uint32 LockID;
    int32  Material;
    uint32 Sheath;
    uint32 RandomProperty;
    uint32 RandomSuffix;
    uint32 ItemSet;
    uint32 Area;
    uint32 Map;
    uint32 BagFamily;
    uint32 TotemCategory;
    uint32 SocketColor[3];
    uint32 SocketContent[3];
    int32  SocketBonus;
    uint32 GemProperties;
    float  ArmorDamageModifier;
    uint32 Duration;
    uint32 ItemLimitCategory;
    uint32 HolidayId;
    float  StatScalingFactor;
    int32  CurrencySubstitutionId;
    int32  CurrencySubstitutionCount;
};

struct ArmorLocationEntry
{
    uint32 InventoryType;   // 0
    float  Value[5];        // 1-5 multiplier for armor types (cloth...plate, no armor?)
};

struct ItemDamageEntry
{
    uint32 Id;          // 0 item level
    float  mod_DPS[7];  // 1-7 multiplier for item quality
    uint32 Id2;         // 8 item level
};

struct ItemArmorQuality
{
    uint32 Id;              // 0 item level
    float  mod_Resist[7];   // 1-7 multiplier for item quality
    uint32 Id2;             // 8 item level
};

struct ItemArmorShield
{
    uint32 Id;              // 0 item level
    uint32 Id2;             // 1 item level
    uint32 mod_Resist[7];   // 2-8 multiplier for item quality
};

struct ItemArmorTotal
{
    uint32 Id;              // 0 item level
    uint32 Id2;             // 1 item level
    float  mod_Resist[4];   // 2-5 multiplier for armor types (cloth...plate)
};

struct ItemExtendedCostEntry
{
    uint32 Id;
    //uint32 reqHonor;
    //uint32 reqArena;
    uint32 reqArenaSlot;
    uint32 reqItem[5];
    uint32 reqItemCount[5];
    uint32 reqPersonalRating;
    //uint32 itemGroup;
    uint32 reqCurrency[5];
    uint32 reqCurrencyCount[5];
    //uint32 unkRep;
    //uint32 unkRep2;
    uint32 flags;
    //uint32 unk1;
    //uint32 unk2;
};

struct ItemLimitCategoryEntry
{
    uint32 Id;                  // 0
    //char *name;
    uint32 MaxAmount;           // 2
    uint32 EquippedFlag;        // 3
};

struct ItemRandomPropertiesEntry
{
    uint32 ID;
    //char *i_name;
    uint32 enchant_id[5];
    char *rpname;
};

struct ItemRandomSuffixEntry
{
    uint32 id;
    char *name;
    // char* i_name;
    uint32 enchantments[5];
    uint32 prefixes[5];
};

struct ItemReforgeEntry
{
    uint32 Id;
    uint32 sourceStat;
    float sourceMultiplier;
    uint32 finalStat;
    float finalMultiplier;
};

struct ItemSetEntry
{
    uint32 id;
    char *name;
    uint32 setItems[9];
    //uint32 unk_0[8];
    uint32 setBonusSpellIds[8];
    uint32 spellRequiredItemCount[8];
    uint32 requiredSkill[2];
};

struct LFGDungeonsEntry
{
    uint32 Id;
    char *dungeonName;
    uint32 minLevel;
    uint32 maxLevel;
    uint32 recomLevel;
    uint32 recomMinLevel;
    uint32 recomMaxLevel;
    int32 mapId;
    uint32 mapDifficulty;
    uint32 LFGFlags;
    uint32 LFGType;
    uint32 LFGFaction;
//  char *iconName;
    uint32 reqExpansion;
//  uint32 unk2;
    uint32 groupType;
//  char *description;
    uint32 randomCategoryId;
    //uint32 unk3[3];

    // Get the dungeon entry
    uint32 GetUniqueID() const { return Id + (LFGType << 24); }
};

struct LockEntry
{
    uint32 Id;
    uint32 locktype[8]; // 0 - no lock, 1 - item needed for lock, 2 - min lockping skill needed
    uint32 lockmisc[8]; // if type is 1 here is a item to unlock, else is unknow for now
    uint32 minlockskill[8]; // min skill in lockpiking to unlock.
    //uint32 unk[8];
};

struct MapEntry
{
    uint32 MapID;               // 0 m_ID
    char  *internalname;        // 1 m_Directory
    uint32 map_type;            // 2 m_InstanceType
    uint32 mapFlags;            // 3 m_Flags (0x100 - CAN_CHANGE_PLAYER_DIFFICULTY)
    //uint32 unk4;              // 4 4.0.1
    //uint32 isPvP;             // 5 m_PVP 0 or 1 for battlegrounds (not arenas)
    char  *name;                // 6 m_MapName_lang
    uint32 linked_zone;         // 7 m_areaTableID
    char  *hordeIntro;          // 8 m_MapDescription0_lang
    char  *allianceIntro;       // 9 m_MapDescription1_lang
    //uint32 multimap_id;       // 10 m_LoadingScreenID (LoadingScreens.dbc)
    //float  BGMapIconScale;    // 11 m_minimapIconScale
    int32  ghost_entrance_map;  // 12 m_corpseMapID map_id of entrance map in ghost mode (continent always and in most cases = normal entrance)
    float  ghost_entrance_x;    // 13 m_corpseX entrance x coordinate in ghost mode  (in most cases = normal entrance)
    float  ghost_entrance_y;    // 14 m_corpseY entrance y coordinate in ghost mode  (in most cases = normal entrance)
    //uint32 timeOfDayOverride; // 15 m_timeOfDayOverride
    uint32 addon;               // 16 m_expansionID
    //uint32 unkTime;           // 17 m_raidOffset
    uint32 maxPlayers;          // 18 m_maxPlayers
    int32  rootPhaseMap;        // 19 map with base phasing

    bool IsMultiDifficulty() const { return mapFlags & 0x100; }
    bool IsDungeon() const { return map_type == 1; }
    bool IsRaid() const { return map_type == 2; }
    bool IsBattleGround() const { return map_type == 3; }
    bool IsBattleArena() const { return map_type == 4; }
    bool Instanceable() const { return map_type == 1 || map_type == 2 || map_type == 3 || map_type == 4; }
    bool IsContinent() const { return MapID == 0 || MapID == 1 || MapID == 530 || MapID == 571 || MapID == 609 || MapID == 646; }
};

struct MapDifficultyEntry
{
    uint32 Id;
    uint32 mapId;
    uint32 difficulty;
    //char *areaTriggerText;
    uint32 resetTime;
    uint32 maxPlayers;
    //char *difficultyStr;
};

struct MountCapabilityEntry
{
    uint32 Id;
    uint32 flags;
    uint32 requiredRidingSkill;
    uint32 requiredArea;
    uint32 requiredAura;
    uint32 requiredSpell;
    uint32 speedModSpell;
    int32  requiredMap;
};

struct MountTypeEntry
{
    uint32 Id;
    uint32 MountCapability[24];
    uint32 maxCapability[2];
};

struct NumTalentsAtLevelEntry
{
    uint32 Level;
    float talentPoints; // I think this is a float?
};

struct OverrideSpellDataEntry
{
    uint32 Id;
    uint32 spellIds[10];
    //uint32 flags;
    //uint32 unk;
};

struct PhaseEntry
{
    uint32 Id;
    uint32 PhaseShift;
    uint32 Flags;
};

struct PvPDifficultyEntry
{
    //uint32 Id;
    uint32 mapId;
    uint32 bracketId;
    uint32 minLevel;
    uint32 maxLevel;
    uint32 difficulty;
};

struct QuestFactionRewardEntry
{
    uint32 Id;
    int32 rewardValue[10];
};

struct QuestXPLevelEntry
{
    uint32 questLevel;
    int32 xpIndex[10];
};

struct ScalingStatDistributionEntry
{
    uint32 Id;                  // 0
    int32  StatMod[10];         // 1-10
    uint32 Modifier[10];        // 11-20
    //uint32 unk1;
    uint32 MaxLevel;            // 22
};

struct ScalingStatValuesEntry
{
    //uint32  Id;
    uint32 Level;
    uint32 dpsMod[6];           // DPS mod for level
    uint32 spellBonus;          // spell power for level
    uint32 ssdMultiplier[5];    // Multiplier for ScalingStatDistribution
    uint32 armorMod[4];         // Armor for level
    uint32 armorMod2[4];        // Low Armor for level
    //uint32 unk[24];
    //uint32 unk2;                // unk, probably also Armor for level (flag 0x80000?)
};

struct SkillRaceClassInfoEntry
{
    //uint32 Id;
    uint32 skillId;
    uint32 raceMask;
    uint32 classMask;
    uint32 flags;
    uint32 reqLevel;
    //uint32 skillTierId;
    //uint32 skillCostId;
    //uint32 unk;
};

struct SkillLineEntry
{
    uint32 id;
    uint32 categoryId;
    char *name;
    //char *description;
    uint32 spellIcon;
    //char *alternateName;
    //uint32 canLink;
};

struct SkillLineAbilityEntry
{
    uint32 Id;
    uint32 skilline;
    uint32 spell;
    uint32 raceMask;
    uint32 classMask;
    //uint32 excludeRace;
    //uint32 excludeClass;
    uint32 minSkillLineRank;
    uint32 supercededBySpell;
    uint32 acquireMethod;
    uint32 RankMax;
    uint32 RankMin;
    //uint32 unk;
    //uint32 unk2; // 4.x
};

struct SpellEntry
{
    uint32 Id;
    uint32 Attributes[11];
    uint32 CastingTimeIndex;
    uint32 DurationIndex;
    uint32 powerType;
    uint32 rangeIndex;
    float  speed;
    uint32 SpellVisual[2];
    uint32 SpellIconID;
    uint32 ActiveIconID;
    char*  Name;
    char*  Rank;
    char*  Description;
    char*  BuffDescription;
    uint32 SchoolMask;
    uint32 SpellRuneCostID;
    //uint32 SpellMissileID;
    //uint32 spellDescriptionVariableID;
    uint32 SpellDifficulty;
    //float Unk400;
    uint32 SpellScalingId;
    uint32 SpellAuraOptionsId;
    uint32 SpellAuraRestrictionsId;
    uint32 SpellCastingRequirementsId;
    uint32 SpellCategoriesId;
    uint32 SpellClassOptionsId;
    uint32 SpellCooldownsId;
    //uint32 Unk400;
    uint32 SpellEquippedItemsId;
    uint32 SpellInterruptsId;
    uint32 SpellLevelsId;
    uint32 SpellPowerId;
    uint32 SpellReagentsId;
    uint32 SpellShapeshiftId;
    uint32 SpellTargetRestrictionsId;
    uint32 SpellTotemsId;
    //uint32 ResearchProject;

    //SpellAuraOptionsEntry
    uint32 maxstack;
    uint32 procChance;
    uint32 procCharges;
    uint32 procFlags;

    //SpellAuraRestrictionsEntry
    uint32 CasterAuraState;
    uint32 TargetAuraState;
    uint32 CasterAuraStateNot;
    uint32 TargetAuraStateNot;
    //uint32 CasterAuraSpell;
    //uint32 TargetAuraSpell;
    //uint32 ExcludeCasterAuraSpell;
    //uint32 ExcludeTargetAuraSpell;

    // SpellCastingRequirementsEntry
    uint32 FacingCasterFlags;
    //uint32 minFactionId;
    //uint32 minFactionStanding;
    int32 AreaGroupId;
    //uint32 requiredAuraVision;
    uint32 RequiresSpellFocus;

    // SpellCategoriesEntry
    uint32 Category;
    uint32 Spell_Dmg_Type;
    uint32 DispelType;
    uint32 MechanicsType;
    uint32 PreventionType;
    uint32 StartRecoveryCategory;

    // SpellCastTimeEntry
    uint32 castTime;
    float castTimePerLevel;
    uint32 baseCastTime;

    // SpellClassOptionsEntry
    //uint32 modalNextSpell;
    uint32 SpellGroupType[3];
    uint32 SpellFamilyName;
    std::string spellBookDescription;

    // SpellCooldownsEntry
    uint32 CategoryRecoveryTime;
    uint32 RecoveryTime;
    uint32 StartRecoveryTime;

    // SpellEffectEntry
    uint32 Effect[3];
    float EffectValueMultiplier[3];
    uint32 EffectApplyAuraName[3];
    uint32 EffectAmplitude[3];
    int32 EffectBasePoints[3];
    float EffectBonusCoefficient[3];
    float EffectDamageMultiplier[3];
    uint32 EffectChainTarget[3];
    int32 EffectDieSides[3];
    uint32 EffectItemType[3];
    uint32 EffectMechanic[3];
    int32 EffectMiscValue[3];
    int32 EffectMiscValueB[3];
    float EffectPointsPerComboPoint[3];
    uint32 EffectRadiusIndex[3];
    uint32 EffectRadiusMaxIndex[3];
    float EffectRealPointsPerLevel[3];
    uint32 EffectSpellClassMask[3][3];
    uint32 EffectTriggerSpell[3];
    uint32 EffectImplicitTargetA[3];
    uint32 EffectImplicitTargetB[3];

    // SpellEquippedItemsEntry
    int32 EquippedItemClass;
    int32 EquippedItemInventoryTypeMask;
    int32 EquippedItemSubClassMask;

    // SpellInterruptsEntry
    uint32 AuraInterruptFlags;
    //uint32 unk;
    uint32 ChannelInterruptFlags;
    //uint32 unk;
    uint32 InterruptFlags;

    // SpellLevelsEntry
    uint32 spellLevelBaseLevel;
    uint32 spellLevelMaxLevel;
    uint32 spellLevelSpellLevel;

    // SpellPowerEntry
    uint32 ManaCost;
    uint32 ManaCostPerlevel;
    uint32 ManaCostPercentage;
    uint32 ManaPerSecond;
    //uint32 PowerDisplayId;
    //uint32 unk1; // 4.x
    //uint32 unk2; // 4.3.x

    // SpellReagentsEntry
    int32 Reagent[8];
    uint32 ReagentCount[8];

    // SpellRuneCostEntry
    uint32 runeCost[3];
    uint32 runicGain;

    // SpellScalingEntry
    uint32 castTimeMin;
    uint32 castTimeMax;
    uint32 castScalingMaxLevel;
    uint32 playerClass;
    float coeff[3][3];
    float coefBase;
    uint32 coefLevelBase;

    // SpellShapeshiftEntry
    uint32 RequiredShapeShift;
    //uint32 unk1; // 3.2.0
    uint32 ShapeshiftExclude;
    //uint32 unk2; // 3.2.0
    //uint32 stanceBarOrder;

    // SpellTargetRestrictionsEntry
    float  MaxTargetRadius;
    uint32 MaxTargets;
    uint32 MaxTargetLevel;
    uint32 TargetCreatureType;
    uint32 Targets;

    // SpellTotemsEntry
    //uint32 TotemCategory[2];
    uint32 Totem[2];

    // SpellDurationEntry
    uint32 Duration[3];

    // SpellRangeEntry
    float minRange[2]; // minEnemy, minFriend
    float maxRange[2]; // maxEnemy, maxFriend

    // SpellRadiusEntry
    float radiusHostile[3];
    float radiusFriend[3];

    // Custom
    uint8 Class;
    uint8 buffIndex;
    uint8 spellType;

    uint32 School;
    uint32 NameHash;
    uint32 RankNumber;
    uint32 GeneratedThreat;
    uint32 SpellSkillLine;
    uint32 CustomAttributes[2];
    bool isUnique, always_apply;

    // Calculations
    int32 CalculateSpellPoints(uint8 effIndex, int32 level, int32 comboPoints);

    int32 CalculateSpellDuration(int32 level, int32 comboPoints)
    {
        int32 retVal = -1;
        if(DurationIndex)
        {
            retVal = Duration[0];
            if(Duration[1] > 0 && Duration[0] <= 0)
            {
                retVal += Duration[1] * level;
                if(Duration[2] > 0 && retVal > 0 && retVal > Duration[2])
                    retVal = Duration[2];
                else if(retVal < 0)
                    retVal = 0;
            }

            if(comboPoints >= 0)
                retVal += (comboPoints*(Duration[2]-Duration[0]))/5;
        }
        return retVal;
    }

    float CalculateRange(bool friendly)
    {

    }

    // Functions
    bool HasAttribute(uint8 index, uint32 attributeFlag) { if(index >= 11) return false; return (Attributes[index] & attributeFlag); }
    bool HasCustomAttribute(uint8 index, uint32 attributeFlag) { if(index >= 2) return false; return (CustomAttributes[index] & attributeFlag); }
    bool AppliesAura(uint32 auraName) { return (EffectApplyAuraName[0] == auraName || EffectApplyAuraName[1] == auraName || EffectApplyAuraName[2] == auraName); }
    bool HasEffect(uint32 spellEffect, uint8 effectMask = 0xFF) { return ((Effect[0] == spellEffect && effectMask&0x01) || (Effect[1] == spellEffect && effectMask&0x02) || (Effect[2] == spellEffect && effectMask&0x04)); }
    bool GetEffectIndex(uint32 spellEffect, uint8 &index)
    {
        if(Effect[0] == spellEffect) { index = 0; return true; }
        if(Effect[1] == spellEffect) { index = 1; return true; }
        if(Effect[2] == spellEffect) { index = 2; return true; }
        return false;
    }

    // Attribute parsing
    bool reqAmmoForSpell() { return HasAttribute(0, 0x02); }
    bool isNextMeleeAttack1() { return HasAttribute(0, 0x04); }
    bool isAbilitySpell() { return HasAttribute(0, 0x10); }
    bool isTradeSpell() { return HasAttribute(0, 0x20); }
    bool isPassiveSpell() { return HasAttribute(0, 0x40); }
    bool isHiddenSpell() { return HasAttribute(0, 0x80); }
    bool isCombatLogInvisible() { return HasAttribute(0, 0x100); }
    bool isautoTargetMainHand() { return HasAttribute(0, 0x200); }
    bool isNextMeleeAttack2() { return HasAttribute(0, 0x400); }
    bool isOutdoorOnly() { return HasAttribute(0, 0x8000); }
    bool isAntiShapeshift() { return HasAttribute(0, 0x10000); }
    bool isStealthOnly() { return HasAttribute(0, 0x20000); }
    bool isStealthStateUnaffecting() { return HasAttribute(0, 0x40000); }
    bool isDamageLevelBased() { return HasAttribute(0, 0x80000); }
    bool isAttackInterrupting() { return HasAttribute(0, 0x100000); }
    bool isUnstoppableForce() { return HasAttribute(0, 0x200000); }
    bool isForcedTargetTracking() { return HasAttribute(0, 0x400000); }
    bool isCastableWhileDead() { return HasAttribute(0, 0x800000); }
    bool isCastableWhileMounted() { return HasAttribute(0, 0x1000000); }
    bool isDisabledWhileActive() { return HasAttribute(0, 0x2000000); }
    bool isNegativeSpell1() { return HasAttribute(0, 0x4000000); }
    bool isCastableWhileSitting() { return HasAttribute(0, 0x8000000); }
    bool isUnavailableInCombat() { return HasAttribute(0, 0x10000000); }
    bool isUnstoppableForce2() { return HasAttribute(0, 0x20000000); }
    bool isBreakableByDamage() { return HasAttribute(0, 0x40000000); }
    bool isForcedApplication() { return HasAttribute(0, 0x80000000); }
    bool isExhaustingPowerSpell() { return HasAttribute(1, 0x02); }
    bool isChanneledSpell() { return HasAttribute(1, 0x04); }
    bool isNotRedirectable() { return HasAttribute(1, 0x08); }
    bool isStealthUnaffecting() { return HasAttribute(1, 0x20); }
    bool isChanneledSpell2() { return HasAttribute(1, 0x40); }
    bool isUnreflectable() { return HasAttribute(1, 0x80); }
    bool isNonCombatTarget() { return HasAttribute(1, 0x100); }
    bool isMeleeCombatTrigger() { return HasAttribute(1, 0x200); }
    bool isThreatlessSpell() { return HasAttribute(1, 0x400); }
    bool isFarsightDetrigger() { return HasAttribute(1, 0x2000); }
    bool isChannelTrackTarget() { return HasAttribute(1, 0x4000); }
    bool isDispelOnImmunityAura() { return HasAttribute(1, 0x8000); }
    bool isImmunityPiercing() { return HasAttribute(1, 0x10000); }
    bool isManualCastPetSpell() { return HasAttribute(1, 0x20000); }
    bool isNotSelfTargettable() { return HasAttribute(1, 0x80000); }
    bool isComboPointReqSpell() { return HasAttribute(1, 0x100000); }
    bool isComboPointUsingSpell() { return HasAttribute(1, 0x200000); }
    bool isSpellEnabledAtDodge() { return HasAttribute(1, 0x400000); }
    bool isUncastableWhileDead() { return HasAttribute(2, 0x01); }
    bool isLineOfSiteIngorant() { return HasAttribute(2, 0x04); }
    bool isStanceBarSpell() { return HasAttribute(2, 0x10); }
    bool isAutoRepeatSpell() { return HasAttribute(2, 0x20); }
    bool reqHonorableTarget() { return HasAttribute(2, 0x40); }
    bool isHealthFunnelSpell() { return HasAttribute(2, 0x800); }
    bool isTargetOwnItemsOnly() { return HasAttribute(2, 0x2000); }
    bool isTameBeastSpell() { return HasAttribute(2, 0x10000); }
    bool isMeleeTimerUnaffecting() { return HasAttribute(2, 0x20000); }
    bool isShapeShiftIgnorant() { return HasAttribute(2, 0x40000); }
    bool reqCasterIsBehindTarget() { return HasAttribute(2, 0x100000); }
    bool isDamageAbsorbingAura() { return HasAttribute(2, 0x200000); }
    bool isUncrittableSpell() { return HasAttribute(2, 0x20000000); }
    bool isTriggerableOnTrigger() { return HasAttribute(2, 0x40000000); }
    bool isFoodBuffAura() { return HasAttribute(2, 0x80000000); }
    bool isBlockableSpell() { return HasAttribute(3, 0x08); }
    bool isResurrectionTimerIgnorant() { return HasAttribute(3, 0x10); };
    bool isMultiCasterStackable() { return HasAttribute(3, 0x80); }
    bool isPlayerTargetRequired() { return HasAttribute(3, 0x100); }
    bool isTriggerableOnTrigger2() { return HasAttribute(3, 0x200); }
    bool reqMainHandWeapon() { return HasAttribute(3, 0x400); }
    bool reqInBattleground() { return HasAttribute(3, 0x800); }
    bool isGhostTargetRequired() { return HasAttribute(3, 0x1000); }
    bool isUnableToTriggerProcs() { return HasAttribute(3, 0x10000); }
    bool isNonInitialAggroSpell() { return HasAttribute(3, 0x20000); }
    bool isIgnorantOfHitResult() { return HasAttribute(3, 0x40000); }
    bool isProcPausingSpell() { return HasAttribute(3, 0x80000); }
    bool isDeathPersistentAura() { return HasAttribute(3, 0x100000); }
    bool reqWandForSpell() { return HasAttribute(3, 0x400000); }
    bool reqOffHandWeapon() { return HasAttribute(3, 0x1000000); }
    bool isTriggerableOnTrigger3() { return HasAttribute(3, 0x4000000); }
    bool isDrainSoulSpell() { return HasAttribute(3, 0x8000000); }
    bool isSpellPowerIgnorant() { return HasAttribute(3, 0x20000000); }
    bool isSpellResistanceIgnorant() { return HasAttribute(4, 0x01); }
    bool isProcLimitedToDummyEffect() { return HasAttribute(4, 0x02); }
    bool isSpellAuraNotStealable() { return HasAttribute(4, 0x40); }
    bool isSpellForceProcTriggered() { return HasAttribute(4, 0x80); }
    bool isSpellDamageNotAuraBreaking() { return HasAttribute(4, 04000); }
    bool isSpellNotAvailableInArena() { return HasAttribute(4, 0x10000); }
    bool isSpellAvailableInArena() { return HasAttribute(4, 0x20000); }
    bool isDelayedSpellChainProcing() { return HasAttribute(4, 0x40000); }
    bool isSelfCastIgnorantOfExistingRank() { return HasAttribute(4, 0x100000); }
    bool isSpellCastableOnlyInOutlands() { return HasAttribute(4, 0x4000000); }
    bool isAwareOfPreparation() { return HasAttribute(5, 0x02); }
    bool isUsableWhileStunned() { return HasAttribute(5, 0x08); }
    bool isSingleTargetSpell() { return HasAttribute(5, 0x20); }
    bool isPeriodicAtApplication() { return HasAttribute(5, 0x200); }
    bool isSpellDurationHidden() { return HasAttribute(5, 0x400); }
    bool isTargetOfTargetCapable() { return HasAttribute(5, 0x800); }
    bool isDurationAffectedByHaste() { return HasAttribute(5, 0x2000); }
    bool reqSpecialItemClassCheck() { return HasAttribute(5, 0x10000); }
    bool isAvailableWhileFeared() { return HasAttribute(5, 0x20000); }
    bool isAvailableWhileConfused() { return HasAttribute(5, 0x40000); }
    bool isSpellIgnorantOfcasterAuras() { return HasAttribute(6, 0x04); }
    bool reqNonCrowdControlledTarget() { return HasAttribute(6, 0x100); }
    bool reqCasterInNonRaidInstance() { return HasAttribute(6, 0x800); }
    bool isCastableWhileMountedInVehicle() { return HasAttribute(6, 0x1000); }
    bool isCastableOnInivsible() { return HasAttribute(6, 0x2000); }
    bool isSpellCastedByCharmOwner() { return HasAttribute(6, 0x4000); }
    bool isUntargettableTargetCapable() { return HasAttribute(6, 0x1000000); }
    bool isIgnorantOfDamagePctMods() { return HasAttribute(6, 0x20000000); }
    bool isSpellCheatSpell() { return HasAttribute(7, 0x08); }
    bool isSpellIgnorantOfPushback() { return HasAttribute(7, 0x40); }
    bool isSpellHordeRacesOnly() { return HasAttribute(7, 0x100); }
    bool isSpellAllianceRacesOnly() { return HasAttribute(7, 0x200); }
    bool isSpellChargeDispelling() { return HasAttribute(7, 0x400); }
    bool isSpellIgnorantOfPeriodicTimer() { return HasAttribute(8, 0x200); }
    bool isEffectAmountSentFromAura() { return HasAttribute(8, 0x1000); }
    bool isMountAuraUsableInWater() { return HasAttribute(8, 0x8000); }
    bool isSpellHealingSpell() { return HasAttribute(8, 0x1000000); }
    bool isSpellGuildPerk() { return HasAttribute(8, 0x10000000); }
    bool isSpellMasterySpecializationBased() { return HasAttribute(8, 0x20000000); }
    //Custom
    bool isSpellStealthTargetCapable() { return HasCustomAttribute(0, 0x01); }
    bool isSpellBackAttackCapable() { return HasCustomAttribute(0, 0x02); }
    bool isSpellExpiringWithPet() { return HasCustomAttribute(0, 0x04); }
    bool isSpellAppliedOnShapeshift() { return HasCustomAttribute(0, 0x08); }
    bool isSpellCastOnPetOwnerOnSummon() { return HasCustomAttribute(0, 0x10); }
    bool isSpellCastOnPetOnSummon() { return HasCustomAttribute(0, 0x20); }
    bool isSpellSelfCastOnly() { return HasCustomAttribute(0, 0x40); }

    bool isSpellDamagingEffect() { return HasCustomAttribute(1, 0x01); }
    bool isSpellHealingEffect() { return HasCustomAttribute(1, 0x02); }
    bool isSpellEnergizingEffect() { return HasCustomAttribute(1, 0x04); }
    bool isSpellAreaOfEffect() { return HasCustomAttribute(1, 0x08); }
    bool isSpellRangedSpell() { return HasCustomAttribute(1, 0x10); }
    bool isSpellFinishingMove() { return HasCustomAttribute(1, 0x20); }
    bool isSpellFlyingSpell() { return HasCustomAttribute(1, 0x40); }

    // Combo
    bool IsSpellChannelSpell() { return (isChanneledSpell() || isChanneledSpell2() || ChannelInterruptFlags); }
    bool IsSpellMeleeSpell() { return (reqMainHandWeapon() || reqOffHandWeapon()); }
    bool IsSpellWeaponSpell() { return spellType <= 3; }

    bool isSpellAuraApplicator() { return (EffectApplyAuraName[0] || EffectApplyAuraName[1] || EffectApplyAuraName[2]); }
    bool isSpellInterruptOnMovement() { return (InterruptFlags & 0x08) || (IsSpellChannelSpell() && (ChannelInterruptFlags & 0x08)); }
    bool isSpellAttackInterrupting() { return !IsSpellChannelSpell() || (IsSpellChannelSpell() && isAttackInterrupting()); }

};

// SpellAuraOptions.dbc
struct SpellAuraOptionsEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    StackAmount;                                  // 1        m_cumulativeAura
    uint32    procChance;                                   // 2        m_procChance
    uint32    procCharges;                                  // 3        m_procCharges
    uint32    procFlags;                                    // 4        m_procTypeMask
};

// SpellAuraRestrictions.dbc
struct SpellAuraRestrictionsEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    CasterAuraState;                              // 1        m_casterAuraState
    uint32    TargetAuraState;                              // 2        m_targetAuraState
    uint32    CasterAuraStateNot;                           // 3        m_excludeCasterAuraState
    uint32    TargetAuraStateNot;                           // 4        m_excludeTargetAuraState
    uint32    casterAuraSpell;                              // 5        m_casterAuraSpell
    uint32    targetAuraSpell;                              // 6        m_targetAuraSpell
    uint32    excludeCasterAuraSpell;                       // 7        m_excludeCasterAuraSpell
    uint32    excludeTargetAuraSpell;                       // 8        m_excludeTargetAuraSpell
};

// SpellCastingRequirements.dbc
struct SpellCastingRequirementsEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    FacingCasterFlags;                            // 1        m_facingCasterFlags
    //uint32    MinFactionId;                               // 2        m_minFactionID not used
    //uint32    MinReputation;                              // 3        m_minReputation not used
    int32     AreaGroupId;                                  // 4        m_requiredAreaGroupId
    //uint32    RequiredAuraVision;                         // 5        m_requiredAuraVision not used
    uint32    RequiresSpellFocus;                           // 6        m_requiresSpellFocus
};

struct SpellCastTimeEntry
{
    uint32 Id;
    uint32 castTime;
    float castTimePerLevel;
    uint32 baseCastTime;
};

// SpellCategories.dbc
struct SpellCategoriesEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    Category;                                     // 1        m_category
    uint32    DmgClass;                                     // 2        m_defenseType
    uint32    Dispel;                                       // 3        m_dispelType
    uint32    Mechanic;                                     // 4        m_mechanic
    uint32    PreventionType;                               // 5        m_preventionType
    uint32    StartRecoveryCategory;                        // 6        m_startRecoveryCategory
};

// SpellClassOptions.dbc
struct SpellClassOptionsEntry
{
    uint32    Id;                   // 0        m_ID
    //uint32    modalNextSpell;     // 1        m_modalNextSpell not used
    uint32    SpellFamilyFlags[3];  // 2-4
    uint32    SpellFamilyName;      // 5      m_spellClassSet
    char     *Description;            // 6 4.0.0
};

// SpellCooldowns.dbc
struct SpellCooldownsEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    CategoryRecoveryTime;                         // 1        m_categoryRecoveryTime
    uint32    RecoveryTime;                                 // 2        m_recoveryTime
    uint32    StartRecoveryTime;                            // 3        m_startRecoveryTime
};

struct SpellDifficultyEntry
{
    uint32 DifficultyID; // id from spell.dbc
    // first is man 10 normal
    // second is man 25 normal
    // third is man 10 heroic
    // fourth is man 25 heroic
    uint32 SpellId[4];
};

struct SpellDurationEntry
{
    uint32 ID;
    int32 Duration1;
    uint32 Duration2;
    uint32 Duration3;
};

// SpellEffect.dbc
struct SpellEffectEntry
{
    uint32    Id;                           // 0    m_ID
    uint32    Effect;                       // 1    m_effect
    float     EffectValueMultiplier;        // 2    m_effectAmplitude
    uint32    EffectApplyAuraName;          // 3    m_effectAura
    uint32    EffectAmplitude;              // 4    m_effectAuraPeriod
    int32     EffectBasePoints;             // 5    m_effectBasePoints (don't must be used in spell/auras explicitly, must be used cached Spell::m_currentBasePoints)
    float     EffectBonusCoefficient;       // 6    m_effectBonusCoefficient
    float     EffectDamageMultiplier;       // 7    m_effectChainAmplitude
    uint32    EffectChainTarget;            // 8    m_effectChainTargets
    int32     EffectDieSides;               // 9    m_effectDieSides
    uint32    EffectItemType;               // 10   m_effectItemType
    uint32    EffectMechanic;               // 11   m_effectMechanic
    int32     EffectMiscValue;              // 12   m_effectMiscValue
    int32     EffectMiscValueB;             // 13   m_effectMiscValueB
    float     EffectPointsPerComboPoint;    // 14   m_effectPointsPerCombo
    uint32    EffectRadiusIndex;            // 15   m_effectRadiusIndex - spellradius.dbc
    uint32    EffectRadiusMaxIndex;         // 16   4.0.0
    float     EffectRealPointsPerLevel;     // 17   m_effectRealPointsPerLevel
    uint32    EffectSpellClassMask[3];      // 18   m_effectSpellClassMask, effect 0
    uint32    EffectTriggerSpell;           // 19   m_effectTriggerSpell
    uint32    EffectImplicitTargetA;        // 20   m_implicitTargetA
    uint32    EffectImplicitTargetB;        // 21   m_implicitTargetB
    uint32    EffectSpellId;                // 22   new 4.0.0
    uint32    EffectIndex;                  // 23   new 4.0.0
    //uint32    unk;                        // 24   new 4.2.0
};

// SpellEquippedItems.dbc
struct SpellEquippedItemsEntry
{
    uint32    Id;                                           // 0        m_ID
    int32     EquippedItemClass;                            // 1        m_equippedItemClass (value)
    int32     EquippedItemInventoryTypeMask;                // 2        m_equippedItemInvTypes (mask)
    int32     EquippedItemSubClassMask;                     // 3        m_equippedItemSubclass (mask)
};

// SpellInterrupts.dbc
struct SpellInterruptsEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    AuraInterruptFlags;                           // 1        m_auraInterruptFlags
    //uint32                                                // 2        4.0.0
    uint32    ChannelInterruptFlags;                        // 3        m_channelInterruptFlags
    //uint32                                                // 4        4.0.0
    uint32    InterruptFlags;                               // 5        m_interruptFlags
};

// SpellLevels.dbc
struct SpellLevelsEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    baseLevel;                                    // 1        m_baseLevel
    uint32    maxLevel;                                     // 2        m_maxLevel
    uint32    spellLevel;                                   // 3        m_spellLevel
};

// SpellPower.dbc
struct SpellPowerEntry
{
    uint32    Id;                   // 0    m_ID
    uint32    manaCost;             // 1    m_manaCost
    uint32    manaCostPerlevel;     // 2    m_manaCostPerLevel
    uint32    manaCostPercentage;   // 3    m_manaCostPct
    uint32    manaPerSecond;        // 4    m_manaPerSecond
    //uint32  PowerDisplayId;       // 5    PowerDisplay.dbc, new in 3.1
    //uint32  unk1;                 // 6    4.0.0
    //uint32  unk4;                 // 7    4.3.x
};

// SpellReagents.dbc
struct SpellReagentsEntry
{
    uint32    Id;                                           // 0        m_ID
    int32     Reagent[8];                                   // 1-8      m_reagent
    uint32    ReagentCount[8];                              // 9-16     m_reagentCount
};

struct SpellRuneCostEntry
{
    uint32 ID;
    uint32 bloodRuneCost;
    uint32 frostRuneCost;
    uint32 unholyRuneCost;
    uint32 runicPowerGain;
};

// SpellScaling.dbc
struct SpellScalingEntry
{
    uint32    Id;                                           // 0        m_ID
    int32     castTimeMin;                                  // 1        minimum cast time
    int32     castTimeMax;                                  // 2        maximum cast time
    uint32    castScalingMaxLevel;                          // 3        first level with maximum cast time
    int32     playerClass;                                  // 4        (index * 100) + charLevel => gtSpellScaling.dbc
    float     coeff[3][3];                                  // 5-13
    float     CoefBase;                                     // 14        some coefficient, mostly 1.0f
    uint32    CoefLevelBase;                                // 15        under this level, the final base coef will be < 1.
};

// SpellShapeshift.dbc
struct SpellShapeshiftEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    Stances;                                      // 1        m_shapeshiftMask
    // uint32 unk_320_2;                                    // 2        3.2.0
    uint32    StancesNot;                                   // 3        m_shapeshiftExclude
    // uint32 unk_320_3;                                    // 4        3.2.0
    // uint32    StanceBarOrder;                            // 5        m_stanceBarOrder not used
};

// SpellShapeshiftForm.dbc
struct SpellShapeshiftFormEntry
{
    uint32 id;                                              // 0 id
    //uint32 buttonPosition;                                // 1 unused
    //char* name;                                           // 2 unused
    uint32 flags1;                                          // 3
    int32  creatureType;                                    // 4 <= 0 humanoid, other normal creature types
    //uint32 unk1;                                          // 5 unused
    uint32 attackSpeed;                                     // 6
    uint32 modelID_A;                                       // 7 Alliance, 0 means use Horde
    uint32 modelID_H;                                       // 8 unused
    //uint32 unk3;                                          // 9 unused
    //uint32 unk4;                                          // 10 unused
    uint32 spells[8];
    //uint32 unk5;                                          // 19 unused
    //uint32 unk6;                                          // 20 unused

    uint32 GetModel(uint8 team)
    {
        if(modelID_A && team)
            return modelID_A;
        return modelID_H;
    }
};

// SpellTargetRestrictions.dbc
struct SpellTargetRestrictionsEntry
{
    uint32  Id;                 // 0        m_ID
    float   maxTargetRadius;    // 1
    uint32  MaxAffectedTargets; // 2        m_maxTargets
    uint32  MaxTargetLevel;     // 3        m_maxTargetLevel
    uint32  TargetCreatureType; // 4        m_targetCreatureType
    uint32  Targets;            // 5        m_targets
};

// SpellTotems.dbc
struct SpellTotemsEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    TotemCategory[2];                             // 1-2      m_requiredTotemCategoryID
    uint32    Totem[2];                                     // 3-4      m_totem
};

struct SpellItemEnchantEntry
{
    uint32 Id;
    //uint32 charges;
    uint32 type[3];
    int32 minPoints[3]; // min always equals max
    //int32 maxPoints[3];
    uint32 spell[3];
    char *description;
    uint32 visualAura;
    uint32 EnchantGroups;
    uint32 GemEntry;
    uint32 enchantCondition;
    uint32 requiredSkill[2];
    //uint32 unk;
    //uint32 unk2;
};

struct SpellRadiusEntry
{
    uint32 ID;
    float radiusHostile;
    //float unk1;
    float radiusFriend;
};

struct SpellRangeEntry
{
    uint32 ID;
    float minRangeHostile;
    float minRangeFriend;
    float maxRangeHostile;
    float maxRangeFriend;
    uint32 flags;
    char *name;
    char *shortName;
};

struct SummonPropertiesEntry
{
    uint32 Id;
    uint32 controltype;
    uint32 factionId;
    uint32 type;
    uint32 slot;
    uint32 summonPropFlags;
};

struct TalentEntry
{
    uint32  TalentID;
    uint32  TalentTree;
    uint32  Row;
    uint32  Col;
    uint32  RankID[5];
    uint32  DependsOn;
    //uint64 unk;
    uint32  DependsOnRank;
    //uint64 unk2;
    //uint32 unkFlag;
    //uint32 unkSpellId;
    //uint64 unkPetFlag;
};

struct TalentTabEntry
{
    uint32 TalentTabID;
    //char *name;
    //uint32 spellIcon;
    uint32 ClassMask;
    uint32 petTalentMask;
    uint32 TabPage;
    //char *internalName;
    //char *description;
    uint32 roleMask;
    uint32 masterySpells[2];

    uint32 affectedClass;
};

struct TalentTreePrimarySpellsEntry
{
    uint32 Id;
    uint32 TalentTabID;
    uint32 SpellID;
    //uint32 flags;
};

struct TaxiNodeEntry
{
    uint32 id;
    uint32 mapid;
    float x, y, z;
    //char *name;
    uint32 mountIdHorde;
    uint32 mountIdAlliance;
    //uint32 unk;
    //float f_unk[2];
};

struct TaxiPathEntry
{
    uint32 id;
    uint32 from;
    uint32 to;
    uint32 price;
};

struct TaxiPathNodeEntry
{
    uint32 Id;
    uint32 PathId;
    uint32 NodeIndex;
    uint32 ContinentID;
    float LocX;
    float LocY;
    float LocZ;
    uint32 flags;
    uint32 delay;
    uint32 arrivalEventId;
    uint32 departureEventId;
};

struct Trainerspell
{
    uint32 Id;
    uint32 skilline1;
    uint32 skilline2;
    uint32 skilline3;
    uint32 maxlvl;
    uint32 charclass;
};

struct VehicleEntry
{
    uint32  m_ID;                                           // 0
    uint32  m_flags;                                        // 1
    float   m_turnSpeed;                                    // 2
    float   m_pitchSpeed;                                   // 3
    float   m_pitchMin;                                     // 4
    float   m_pitchMax;                                     // 5
    uint32  m_seatID[8];                                    // 6-13
    float   m_mouseLookOffsetPitch;                         // 14
    float   m_cameraFadeDistScalarMin;                      // 15
    float   m_cameraFadeDistScalarMax;                      // 16
    float   m_cameraPitchOffset;                            // 17
    float   m_facingLimitRight;                             // 18
    float   m_facingLimitLeft;                              // 19
    float   m_msslTrgtTurnLingering;                        // 20
    float   m_msslTrgtPitchLingering;                       // 21
    float   m_msslTrgtMouseLingering;                       // 22
    float   m_msslTrgtEndOpacity;                           // 23
    float   m_msslTrgtArcSpeed;                             // 24
    float   m_msslTrgtArcRepeat;                            // 25
    float   m_msslTrgtArcWidth;                             // 26
    float   m_msslTrgtImpactRadius[2];                      // 27-28
    char*   m_msslTrgtArcTexture;                           // 29
    char*   m_msslTrgtImpactTexture;                        // 30
    char*   m_msslTrgtImpactModel[2];                       // 31-32
    float   m_cameraYawOffset;                              // 33
    uint32  m_uiLocomotionType;                             // 34
    float   m_msslTrgtImpactTexRadius;                      // 35
    uint32  m_uiSeatIndicatorType;                          // 36
    uint32  m_powerType;                                    // 37, new in 3.1
                                                            // 38, new in 3.1
                                                            // 39, new in 3.1
};

struct VehicleSeatEntry
{
    uint32  m_ID;                                           // 0
    uint32  m_flags;                                        // 1
    int32   m_attachmentID;                                 // 2
    float   m_attachmentOffsetX;                            // 3
    float   m_attachmentOffsetY;                            // 4
    float   m_attachmentOffsetZ;                            // 5
    float   m_enterPreDelay;                                // 6
    float   m_enterSpeed;                                   // 7
    float   m_enterGravity;                                 // 8
    float   m_enterMinDuration;                             // 9
    float   m_enterMaxDuration;                             // 10
    float   m_enterMinArcHeight;                            // 11
    float   m_enterMaxArcHeight;                            // 12
    int32   m_enterAnimStart;                               // 13
    int32   m_enterAnimLoop;                                // 14
    int32   m_rideAnimStart;                                // 15
    int32   m_rideAnimLoop;                                 // 16
    int32   m_rideUpperAnimStart;                           // 17
    int32   m_rideUpperAnimLoop;                            // 18
    float   m_exitPreDelay;                                 // 19
    float   m_exitSpeed;                                    // 20
    float   m_exitGravity;                                  // 21
    float   m_exitMinDuration;                              // 22
    float   m_exitMaxDuration;                              // 23
    float   m_exitMinArcHeight;                             // 24
    float   m_exitMaxArcHeight;                             // 25
    int32   m_exitAnimStart;                                // 26
    int32   m_exitAnimLoop;                                 // 27
    int32   m_exitAnimEnd;                                  // 28
    float   m_passengerYaw;                                 // 29
    float   m_passengerPitch;                               // 30
    float   m_passengerRoll;                                // 31
    int32   m_passengerAttachmentID;                        // 32
    int32   m_vehicleEnterAnim;                             // 33
    int32   m_vehicleExitAnim;                              // 34
    int32   m_vehicleRideAnimLoop;                          // 35
    int32   m_vehicleEnterAnimBone;                         // 36
    int32   m_vehicleExitAnimBone;                          // 37
    int32   m_vehicleRideAnimLoopBone;                      // 38
    float   m_vehicleEnterAnimDelay;                        // 39
    float   m_vehicleExitAnimDelay;                         // 40
    uint32  m_vehicleAbilityDisplay;                        // 41
    uint32  m_enterUISoundID;                               // 42
    uint32  m_exitUISoundID;                                // 43
    int32   m_uiSkin;                                       // 44
    uint32  m_flagsB;                                       // 45
                                                            // 46-57 added in 3.1, floats mostly
    //uint32 unk[6];                                        // 58-63
    //uint32 unk2;                                          // 64 4.0.0
    //uint32 unk3;                                          // 65 4.0.1

    bool IsUsable() const { return (m_flags & 0x2000000 ? true : false); }
    bool IsControllable() const { return (m_flags & 0x800 ? true : false); }
};

struct WMOAreaTableEntry
{
    uint32 Id;                // 0 index
    int32 rootId;            // 1 used in root WMO
    uint32 adtId;            // 2 used in adt file
    int32 groupId;            // 3 used in group WMO
    uint32 Flags;            // 9 used for indoor/outdoor determination
    uint32 areaId;            // 10 link to AreaTableEntry.ID
};

struct WorldMapOverlayEntry
{
    uint32 AreaReference;
    uint32 AreaTableID[4];
};

struct WorldSafeLocsEntry
{
    uint32 ID;                                           // 0
    uint32 map_id;                                       // 1
    float  x;                                            // 2
    float  y;                                            // 3
    float  z;                                            // 4
    //char Name;                                         // 5
    //char name[7]                                       // 6-12 name, unused
                                                         // 13 name flags, unused
};

struct gtFloat
{
    uint32 id;
    float val;
};

#pragma pack(PRAGMA_POP)

template<class T, class T2> class SERVER_DECL DBStorage
{
    T2 *m_file;
    std::string _fileName;
    std::string _fileFormat;

    std::map<uint32, T*> subEntries;
public:
    DBStorage() { m_file = NULL; }
    ~DBStorage() { Unload(); _fileName.clear(); _fileFormat.clear(); }

    // Initialize with the correct name and format
    void Init(const char *filename, const char *format) { _fileName = filename; _fileFormat = format; }

    // for load and unload
    bool Load() { return m_file ? true : (m_file = new T2())->Load(_fileName.c_str(), _fileFormat.c_str()); }
    void Unload()
    {
        if(m_file)
            delete m_file;
        m_file = NULL;
        if(!subEntries.empty())
        {
            for(auto itr = subEntries.begin(); itr != subEntries.end(); itr++)
                delete itr->second;
            subEntries.clear();
        }
    }

    uint32 GetNumRows() { return m_file ? m_file->GetNumRows() : 0; }
    uint32 GetMaxEntry() { return m_file ? m_file->GetMaxEntry() : 0; }

    T * LookupEntryTest(uint32 i)
    {
        if(subEntries.find(i) != subEntries.end())
            return subEntries.at(i);
        return m_file ? m_file->LookupEntryTest(i) : NULL;
    }

    T * LookupEntry(uint32 i)
    {
        if(subEntries.find(i) != subEntries.end())
            return subEntries.at(i);
        return m_file ? m_file->LookupEntry(i) : NULL;
    }

    T * LookupRow(uint32 i) { return m_file ? m_file->LookupRow(i) : NULL; }

    bool SetEntry(uint32 entry, T* block)
    {
        if(subEntries.find(entry) != subEntries.end())
            return false;
        subEntries.insert(std::make_pair(entry, block));
        return true;
    }
};

RONIN_INLINE float GetDBCScale(CreatureDisplayInfoEntry *Scale)
{
    if(Scale && Scale->Scale)
        return Scale->Scale;
    return 1.0f;
}

#define DBC_STORAGE_EXTERN_DBC_MACRO(EntryClass, DeclaredClass) extern SERVER_DECL DBStorage<EntryClass, DBC<EntryClass>> DeclaredClass
#define DBC_STORAGE_EXTERN_DB2_MACRO(EntryClass, DeclaredClass) extern SERVER_DECL DBStorage<EntryClass, DB2<EntryClass>> DeclaredClass

DBC_STORAGE_EXTERN_DBC_MACRO(AchievementEntry, dbcAchievement);
DBC_STORAGE_EXTERN_DBC_MACRO(AchievementCriteriaEntry, dbcAchievementCriteria);
DBC_STORAGE_EXTERN_DBC_MACRO(AreaGroupEntry, dbcAreaGroup);
DBC_STORAGE_EXTERN_DBC_MACRO(AreaTableEntry, dbcAreaTable);
DBC_STORAGE_EXTERN_DBC_MACRO(AreaTriggerEntry, dbcAreaTrigger);
DBC_STORAGE_EXTERN_DBC_MACRO(AuctionHouseEntry, dbcAuctionHouse);
DBC_STORAGE_EXTERN_DBC_MACRO(BankSlotPriceEntry, dbcBankSlotPrices);
DBC_STORAGE_EXTERN_DBC_MACRO(BarberShopStyleEntry, dbcBarberShopStyle);
DBC_STORAGE_EXTERN_DBC_MACRO(BattleMasterListEntry, dbcBattleMasterList);
DBC_STORAGE_EXTERN_DBC_MACRO(CharClassEntry, dbcCharClass);
DBC_STORAGE_EXTERN_DBC_MACRO(CharPowerTypeEntry, dbcCharPowerType);
DBC_STORAGE_EXTERN_DBC_MACRO(CharRaceEntry, dbcCharRace);
DBC_STORAGE_EXTERN_DBC_MACRO(CharStartOutfitEntry, dbcCharStartOutfit);
DBC_STORAGE_EXTERN_DBC_MACRO(CharTitleEntry, dbcCharTitle);
DBC_STORAGE_EXTERN_DBC_MACRO(ChatChannelEntry, dbcChatChannels);
DBC_STORAGE_EXTERN_DBC_MACRO(CreatureBoundDataEntry, dbcCreatureBoundData);
DBC_STORAGE_EXTERN_DBC_MACRO(CreatureDisplayInfoEntry, dbcCreatureDisplayInfo);
DBC_STORAGE_EXTERN_DBC_MACRO(CreatureDisplayInfoExtraEntry, dbcCreatureDisplayInfoExtra);
DBC_STORAGE_EXTERN_DBC_MACRO(CreatureFamilyEntry, dbcCreatureFamily);
DBC_STORAGE_EXTERN_DBC_MACRO(CreatureSpellDataEntry, dbcCreatureSpellData);
DBC_STORAGE_EXTERN_DBC_MACRO(CurrencyTypeEntry, dbcCurrencyType);
DBC_STORAGE_EXTERN_DBC_MACRO(DestructibleModelDataEntry, dbcDestructibleModelData);
DBC_STORAGE_EXTERN_DBC_MACRO(DungeonEncounterEntry, dbcDungeonEncounter);
DBC_STORAGE_EXTERN_DBC_MACRO(DurabilityCostsEntry, dbcDurabilityCosts);
DBC_STORAGE_EXTERN_DBC_MACRO(DurabilityQualityEntry, dbcDurabilityQuality);
DBC_STORAGE_EXTERN_DBC_MACRO(EmoteEntry, dbcEmote);
DBC_STORAGE_EXTERN_DBC_MACRO(EmoteTextEntry, dbcEmoteText);
DBC_STORAGE_EXTERN_DBC_MACRO(FactionEntry, dbcFaction);
DBC_STORAGE_EXTERN_DBC_MACRO(FactionTemplateEntry, dbcFactionTemplate);
DBC_STORAGE_EXTERN_DBC_MACRO(GameObjectDisplayInfoEntry, dbcGameObjectDisplayInfo);
DBC_STORAGE_EXTERN_DBC_MACRO(GemPropertyEntry, dbcGemProperty);
DBC_STORAGE_EXTERN_DBC_MACRO(GlyphProperties, dbcGlyphProperties);
DBC_STORAGE_EXTERN_DB2_MACRO(ItemDataEntry, db2Item);
DBC_STORAGE_EXTERN_DB2_MACRO(ItemSparseEntry, db2ItemSparse);
DBC_STORAGE_EXTERN_DBC_MACRO(ArmorLocationEntry, dbcArmorLocation);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemArmorQuality, dbcArmorQuality);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemArmorShield, dbcArmorShield);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemArmorTotal, dbcArmorTotal);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemDamageEntry, dbcDamageAmmo);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemDamageEntry, dbcDamageOneHand);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemDamageEntry, dbcDamageOneHandCaster);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemDamageEntry, dbcDamageRanged);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemDamageEntry, dbcDamageThrown);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemDamageEntry, dbcDamageTwoHand);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemDamageEntry, dbcDamageTwoHandCaster);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemDamageEntry, dbcDamageDamageWand);
DBC_STORAGE_EXTERN_DB2_MACRO(ItemExtendedCostEntry, dbcItemExtendedCost);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemLimitCategoryEntry, dbcItemLimitCategory);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemRandomPropertiesEntry, dbcItemRandomProperties);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemRandomSuffixEntry, dbcItemRandomSuffix);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemReforgeEntry, dbcItemReforge);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemSetEntry, dbcItemSet);
DBC_STORAGE_EXTERN_DBC_MACRO(LFGDungeonsEntry, dbcLFGDungeons);
DBC_STORAGE_EXTERN_DBC_MACRO(LockEntry, dbcLock);
DBC_STORAGE_EXTERN_DBC_MACRO(MapEntry, dbcMap);
DBC_STORAGE_EXTERN_DBC_MACRO(MapDifficultyEntry, dbcMapDifficulty);
DBC_STORAGE_EXTERN_DBC_MACRO(MountCapabilityEntry, dbcMountCapability);
DBC_STORAGE_EXTERN_DBC_MACRO(MountTypeEntry, dbcMountType);
DBC_STORAGE_EXTERN_DBC_MACRO(NumTalentsAtLevelEntry, dbcNumTalents);
DBC_STORAGE_EXTERN_DBC_MACRO(OverrideSpellDataEntry, dbcOverrideSpellData);
DBC_STORAGE_EXTERN_DBC_MACRO(PhaseEntry, dbcPhase);
DBC_STORAGE_EXTERN_DBC_MACRO(PvPDifficultyEntry, dbcPvPDifficulty);
DBC_STORAGE_EXTERN_DBC_MACRO(QuestFactionRewardEntry, dbcQuestFactionReward);
DBC_STORAGE_EXTERN_DBC_MACRO(QuestXPLevelEntry, dbcQuestXP);
DBC_STORAGE_EXTERN_DBC_MACRO(ScalingStatDistributionEntry, dbcScalingStatDistribution);
DBC_STORAGE_EXTERN_DBC_MACRO(ScalingStatValuesEntry, dbcScalingStatValues);
DBC_STORAGE_EXTERN_DBC_MACRO(SkillRaceClassInfoEntry, dbcSkillRaceClassInfo);
DBC_STORAGE_EXTERN_DBC_MACRO(SkillLineEntry, dbcSkillLine);
DBC_STORAGE_EXTERN_DBC_MACRO(SkillLineAbilityEntry, dbcSkillLineSpell);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellEntry, dbcSpell);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellAuraOptionsEntry, dbcSpellAuraOptions);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellAuraRestrictionsEntry, dbcSpellAuraRestrictions);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellCastingRequirementsEntry, dbcSpellCastingRequirements);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellCastTimeEntry, dbcSpellCastTime);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellCategoriesEntry, dbcSpellCategories);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellClassOptionsEntry, dbcSpellClassOptions);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellCooldownsEntry, dbcSpellCooldowns);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellDifficultyEntry, dbcSpellDifficulty);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellDurationEntry, dbcSpellDuration);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellEffectEntry, dbcSpellEffect);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellEquippedItemsEntry, dbcSpellEquippedItems);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellInterruptsEntry, dbcSpellInterrupts);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellItemEnchantEntry, dbcSpellItemEnchant);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellLevelsEntry, dbcSpellLevels);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellPowerEntry, dbcSpellPower);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellRadiusEntry, dbcSpellRadius);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellRangeEntry, dbcSpellRange);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellReagentsEntry, dbcSpellReagents);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellRuneCostEntry, dbcSpellRuneCost);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellScalingEntry, dbcSpellScaling);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellShapeshiftEntry, dbcSpellShapeshift);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellShapeshiftFormEntry, dbcSpellShapeshiftForm);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellTargetRestrictionsEntry, dbcSpellTargetRestrictions);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellTotemsEntry, dbcSpellTotems);
DBC_STORAGE_EXTERN_DBC_MACRO(SummonPropertiesEntry, dbcSummonProperties);
DBC_STORAGE_EXTERN_DBC_MACRO(TalentEntry, dbcTalent);
DBC_STORAGE_EXTERN_DBC_MACRO(TalentTabEntry, dbcTalentTab);
DBC_STORAGE_EXTERN_DBC_MACRO(TalentTreePrimarySpellsEntry, dbcTreePrimarySpells);
DBC_STORAGE_EXTERN_DBC_MACRO(TaxiNodeEntry, dbcTaxiNode);
DBC_STORAGE_EXTERN_DBC_MACRO(TaxiPathEntry, dbcTaxiPath);
DBC_STORAGE_EXTERN_DBC_MACRO(TaxiPathNodeEntry, dbcTaxiPathNode);
DBC_STORAGE_EXTERN_DBC_MACRO(VehicleEntry, dbcVehicle);
DBC_STORAGE_EXTERN_DBC_MACRO(VehicleSeatEntry, dbcVehicleSeat);
DBC_STORAGE_EXTERN_DBC_MACRO(WMOAreaTableEntry, dbcWMOAreaTable);
DBC_STORAGE_EXTERN_DBC_MACRO(WorldMapOverlayEntry, dbcWorldMapOverlay);
DBC_STORAGE_EXTERN_DBC_MACRO(WorldSafeLocsEntry, dbcWorldSafeLocs);

DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcCombatRating);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcBarberShopPrices);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcMeleeCrit);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcMeleeCritBase);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcSpellCrit);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcSpellCritBase);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcManaRegen);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcManaRegenBase);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcSpellScalar);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcHPPerStam);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcCombatRatingScaling);

class TaskList;

class DBCUnloader : public ThreadContext { public: bool run(); };
class DBCLoader : public Singleton<DBCLoader>
{
    friend class DBCUnloader;
public:
    DBCLoader() {};
    ~DBCLoader() {};

    void FillDBCLoadList(TaskList &tl, const char* datapath, bool *result);
    static void StartCleanup() { ThreadPool.ExecuteTask("DBCCleanup", new DBCUnloader()); }

private:
    template<class T> void LoadDBC(bool *result, std::string filename, const char * format, T *l);

    static void UnloadAllDBCFiles();
};

#define sDBCLoader DBCLoader::getSingleton()
