/***
 * Demonstrike Core
 * Citric
 */

#pragma once

#include "DBCAchievementStores.h"

#pragma pack(push,1)

struct AreaGroupEntry
{
    uint32 AreaGroupId;
    uint32 AreaId[7];
};

struct AreaTableEntry
{
    uint32 AreaId;
    uint32 mapId;
    uint32 ZoneId;
    uint32 explorationFlag;
    uint32 AreaFlags;
    uint32 level;
    char* name;
    uint32 category;
};

struct AreaTriggerEntry
{
    uint32 Id;
    uint32 continentId;
    float base_x;
    float base_y;
    float base_z;
    float phaseShift;
    float size;
    // unknown
    float radius;
    float box_length;
    float box_width;
    float box_height;
    float box_yaw;
};

struct AuctionHouseDBC
{
    uint32 id;
    uint32 faction;
    uint32 fee;
    uint32 tax;
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
    uint32 holidayWSId;
    uint32 minLevel;
    uint32 maxLevel;
    uint32 maxRatedGroupSize;
    uint32 maxPlayers;
    uint32 rated;
    //uint32 unknown
};

struct CharClassEntry
{
    uint32 class_id;
    uint32 power_type;
    char* name;
};

struct CharRaceEntry
{
    uint32 RaceId;
    uint32 FactionId;
    uint32 maleModel;
    uint32 femaleModel;
    uint32 CinematicId;
    uint32 TeamId;
    char*  Name;
};

struct gtFloat
{
    uint32 id;
    float val;
};

struct ChatChannelDBC
{
    uint32 id;
    uint32 flags;
    const char* pattern;
};

struct CreatureDisplayInfo
{
    uint32 ID;
    //uint32 ModelData
    //uint32 Sound
    //uint32 ExtraDisplayInfo
    float Scale;
    //uint32 unk[7];
};

struct CreatureBoundData
{
    uint32 Entry; // Display ID
    float Low[3];
    float High[3];
    float BoundRadius;
};

struct CreatureFamilyEntry
{
    uint32 ID;
    float minsize;
    uint32 minlevel;
    float maxsize;
    uint32 maxlevel;
    uint32 skilline;
    uint32 tameable;        //second skill line - 270 Generic
    uint32 petdietflags;
    uint32 pettalenttype;
    char* name;
};

struct CreatureSpellDataEntry
{
    uint32 id;
    uint32 Spells[3];
    uint32 PHSpell;
    uint32 Cooldowns[3];
    uint32 PH;
};

struct CharTitlesEntry
{
    uint32 entry;
    //uint32 unk;
    char *name1;
    //char *name2;
    uint32 index;
    //uint32 unk0;
};

struct DBCTaxiNode
{
    uint32 id;
    uint32 mapid;
    float x;
    float y;
    float z;
    uint32 horde_mount;
    uint32 alliance_mount;
};

struct DBCTaxiPath
{
    uint32 id;
    uint32 from;
    uint32 to;
    uint32 price;
};

struct DBCTaxiPathNode
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

struct DestructibleModelDataEntry
{
    uint32 entry; //Unknown9 from gameobject_names
    uint32 displayId[5];

    uint32 GetDisplayId(uint8 state)
    {
        if(state > 5)
            return 0;

        if(!displayId[state])
        {
            for(int32 i = state-1; i > -1; --i)
            {
                if(displayId[i])
                    return displayId[i];
            }
        }

        return displayId[state];
    }
};

struct EmoteEntry
{
    uint32 Id;
    uint32 textid;
    uint32 textid2;
    uint32 textid3;
    uint32 textid4;
    uint32 textid5;
    uint32 textid6;
};

struct EnchantEntry
{
    uint32 Id;
    uint32 type[3];
    int32 min[3];//for compat, in practice min==max
    int32 max[3];
    uint32 spell[3];
    char* Name;
    uint32 visual;
    uint32 EnchantGroups;
    uint32 GemEntry;
};

struct FactionEntry
{
    uint32 ID;
    int32  RepListId;
    uint32 RaceMask[4];
    uint32 ClassMask[4];
    int32  baseRepValue[4];
    uint32 repFlags[4];
    uint32 parentFaction;
    char*  Name;
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
};

struct GemPropertyEntry
{
    uint32 Entry;
    uint32 EnchantmentID;
    uint32 SocketMask;
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
    uint32 ID;                          // 0
    uint32 Quality;                     // 1
    uint32 Flags;                       // 2
    uint32 Flags2;                      // 3
    uint32 BuyPrice;                    // 4
    uint32 SellPrice;                   // 5
    uint32 InventoryType;               // 6
    int32  AllowableClass;              // 7
    int32  AllowableRace;               // 8
    uint32 ItemLevel;                   // 9
    int32  RequiredLevel;               // 10
    uint32 RequiredSkill;               // 11
    uint32 RequiredSkillRank;           // 12
    uint32 RequiredSpell;               // 13
    uint32 RequiredHonorRank;           // 14
    uint32 RequiredCityRank;            // 15
    uint32 RequiredReputationFaction;   // 16
    uint32 RequiredReputationRank;      // 17
    uint32 MaxCount;                    // 18
    uint32 Stackable;                   // 19
    uint32 ContainerSlots;              // 20
    int32  ItemStatType[10];            // 21 - 30
    uint32 ItemStatValue[10];           // 31 - 40
    int32  ItemStatUnk1[10];            // 41 - 50
    int32  ItemStatUnk2[10];            // 51 - 60
    uint32 ScalingStatDistribution;     // 61
    uint32 DamageType;                  // 62
    uint32 Delay;                       // 63
    float  RangedModRange;              // 64
    int32  SpellId[5];                  // 65 - 69
    int32  SpellTrigger[5];             // 70 - 74
    int32  SpellCharges[5];             // 75 - 79
    int32  SpellCooldown[5];            // 80 - 84
    int32  SpellCategory[5];            // 85 - 89
    int32  SpellCategoryCooldown[5];    // 90 - 94
    uint32 Bonding;                     // 95
    char*  Name;                        // 96
    char*  Name2;                       // 97
    char*  Name3;                       // 98
    char*  Name4;                       // 99
    char*  Description;                 // 100
    uint32 PageText;                    // 101
    uint32 LanguageID;                  // 102
    uint32 PageMaterial;                // 103
    uint32 StartQuest;                  // 104
    uint32 LockID;                      // 105
    int32  Material;                    // 106
    uint32 Sheath;                      // 107
    uint32 RandomProperty;              // 108
    uint32 RandomSuffix;                // 109
    uint32 ItemSet;                     // 110
    uint32 MaxDurability;               // 111
    uint32 Area;                        // 112
    uint32 Map;                         // 113
    uint32 BagFamily;                   // 114
    uint32 TotemCategory;               // 115
    uint32 SocketColor[3];              // 116 - 118
    uint32 SocketContent[3];            // 119 - 121
    int32  SocketBonus;                 // 122
    uint32 GemProperties;               // 123
    float  ArmorDamageModifier;         // 124
    uint32 Duration;                    // 125
    uint32 ItemLimitCategory;           // 126
    uint32 HolidayId;                   // 127
    float  StatScalingFactor;           // 128
    int32  Field130;                    // 129
    int32  Field131;                    // 130
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

struct ArmorLocationEntry
{
    uint32 InventoryType;   // 0
    float  Value[5];        // 1-5 multiplier for armor types (cloth...plate, no armor?)
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
    // unk[5];
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
    uint32 spells[3];
    //uint32 unk[2];
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

struct ItemSetEntry
{
    uint32 id;                  //1
    char* name;                //2
    uint32 flag;                //10 constant
    uint32 itemid[10];          //11 - 18
    uint32 SpellID[8];          //28 - 35
    uint32 itemscount[8];       //36 - 43
    uint32 RequiredSkillID;     //44
    uint32 RequiredSkillAmt;    //45
};

struct Lock
{
    uint32 Id;
    uint32 locktype[5]; // 0 - no lock, 1 - item needed for lock, 2 - min lockping skill needed
    uint32 lockmisc[5]; // if type is 1 here is a item to unlock, else is unknow for now
    uint32 minlockskill[5]; // min skill in lockpiking to unlock.
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
//  uint32 unk;
//  char *iconName;
    uint32 reqExpansion;
//  uint32 unk2;
    uint32 groupType;
//  char *description;

    // Get the dungeon entry
    uint32 GetUniqueID() const { return Id + (LFGType << 24); }
};

struct MapEntry
{
    uint32 id;
    char* name_internal;
    uint32 map_type;
    uint32 is_pvp_zone;
    char* real_name;
    uint32 linked_zone;        // common zone for instance and continent map
    char* hordeIntro;        // text for PvP Zones
    char* allianceIntro;    // text for PvP Zones
    uint32 multimap_id;

    uint32 GetMapType() { return map_type; }
    bool IsRaid() { return map_type == 2; }
    bool IsInstancedMap()
    {
        if(GetMapType() == 1 || GetMapType() == 2)
            return true;

        return false;
    }
};

struct NumTalentsAtLevelEntry
{
    uint32 Level;
    float talentPoints; // I think this is a float?
};

struct QuestXPEntry
{
    uint32 questLevel;
    uint32 xpIndex[10];
};

struct ScalingStatDistributionEntry
{
    uint32 Id;                  // 0
    int32  StatMod[10];         // 1-10
    uint32 Modifier[10];        // 11-20
    uint32 MaxLevel;            // 21
};

struct ScalingStatValuesEntry
{
//  uint32  Id;                         // 0
    uint32 Level;                       // 1
    uint32 ssdMultiplier[4];            // 2-5 Multiplier for ScalingStatDistribution
    uint32 armorMod[4];                 // 6-9 Armor for level
    uint32 dpsMod[6];                   // 10-15 DPS mod for level
    uint32 spellBonus;                  // 16 spell power for level
    uint32 ssdMultiplier2;              // 17 there's data from 3.1 dbc ssdMultiplier[3]
    uint32 ssdMultiplier3;              // 18 3.3
//  uint32 unk2;                        // 19 unk, probably also Armor for level (flag 0x80000?)
    uint32 armorMod2[4];                // 20-23 Low Armor for level
};

struct SkillLineEntry //SkillLine.dbc
{
    uint32 id;
    uint32 type;
    uint32 unk1;
    char* Name;
};

struct SkillLineSpell //SkillLineAbility.dbc
{
    uint32 Id;
    uint32 skilline;
    uint32 spell;
    uint32 raceMask;
    uint32 classMask;
    uint32 excludeRace;
    uint32 excludeClass;
    uint32 minSkillLineRank;
    uint32 supercededBySpell;
    uint32 acquireMethod;
    uint32 RankMax;
    uint32 RankMin;
};

struct SpellEntry
{
    uint32 Id;
    uint32 Attributes;
    uint32 AttributesEx;
    uint32 Flags3;
    uint32 Flags4;
    uint32 Flags5;
    uint32 Flags6;
    uint32 Flags7;
    uint32 Flags8;
    uint32 Flags9;
    //uint32 Unk400
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
    uint32 RuneCostID;
    //uint32 SpellMissileID
    //uint32 SpellDescriptionsomethingorother
    uint32 SpellDifficulty;
    //float Unk400
    uint32 SpellScalingId;
    uint32 SpellAuraOptionsId;
    uint32 SpellAuraRestrictionsId;
    uint32 SpellCastingRequirementsId;
    uint32 SpellCategoriesId;
    uint32 SpellClassOptionsId;
    uint32 SpellCooldownsId;
    //uint32 Unk400
    uint32 SpellEquippedItemsId;
    uint32 SpellInterruptsId;
    uint32 SpellLevelsId;
    uint32 SpellPowerId;
    uint32 SpellReagentsId;
    uint32 SpellShapeshiftId;
    uint32 SpellTargetRestrictionsId;
    uint32 SpellTotemsId;
    //uint32 Unk400

    //SpellAuraOptionsEntry
        uint32    maxstack;
        uint32    procChance;
        uint32    procCharges;
        uint32    procFlags;

    //SpellAuraRestrictionsEntry
        uint32    CasterAuraState;
        uint32    TargetAuraState;
        uint32    CasterAuraStateNot;
        uint32    TargetAuraStateNot;
        //uint32    CasterAuraSpell;
        //uint32    TargetAuraSpell;
        //uint32    ExcludeCasterAuraSpell;
        //uint32    ExcludeTargetAuraSpell;

    // SpellCastingRequirementsEntry
        uint32    FacingCasterFlags;
        int32     AreaGroupId;
        uint32    RequiresSpellFocus;

    // SpellCategoriesEntry
        uint32    Category;
        uint32    Spell_Dmg_Type;
        uint32    DispelType;
        uint32    MechanicsType;
        uint32    PreventionType;
        uint32    StartRecoveryCategory;

    // SpellClassOptionsEntry
        uint32    SpellGroupType[3];
        uint32    SpellFamilyName;

    // SpellCooldownsEntry
        uint32    CategoryRecoveryTime;
        uint32    RecoveryTime;
        uint32    StartRecoveryTime;

    // SpellEffectEntry
        uint32    Effect[3];
        float     EffectValueMultiplier[3];
        uint32    EffectApplyAuraName[3];
        uint32    EffectAmplitude[3];
        int32     EffectBasePoints[3];
        float     EffectBonusCoefficient[3];
        float     EffectDamageMultiplier[3];
        uint32    EffectChainTarget[3];
        int32     EffectDieSides[3];
        uint32    EffectItemType[3];
        uint32    EffectMechanic[3];
        int32     EffectMiscValue[3];
        int32     EffectMiscValueB[3];
        float     EffectPointsPerComboPoint[3];
        uint32    EffectRadiusIndex[3];
        float     EffectRealPointsPerLevel[3];
        uint32    EffectSpellClassMask[3][3];
        uint32    EffectTriggerSpell[3];
        uint32    EffectImplicitTargetA[3];
        uint32    EffectImplicitTargetB[3];

    // SpellEquippedItemsEntry
        int32     EquippedItemClass;
        int32     RequiredItemFlags;
        int32     EquippedItemSubClass;

    // SpellInterruptsEntry
        uint32    AuraInterruptFlags;
        uint32    ChannelInterruptFlags;
        uint32    InterruptFlags;

    // SpellLevelsEntry
        uint32    baseLevel;
        uint32    maxLevel;
        uint32    spellLevel;

    // SpellPowerEntry
        uint32    ManaCost;
        uint32    ManaCostPerlevel;
        uint32    ManaCostPercentage;
        uint32    ManaPerSecond;

    // SpellReagentsEntry
        int32     Reagent[8];
        uint32    ReagentCount[8];

    // SpellShapeshiftEntry
        uint32    RequiredShapeShift;
        uint32    ShapeshiftExclude;

    // SpellTargetRestrictionsEntry
        uint32    MaxTargets;
        uint32    MaxTargetLevel;
        uint32    TargetCreatureType;
        uint32    Targets;

    // SpellTotemsEntry
        //uint32    TotemCategory[2];    // 162-163  m_requiredTotemCategoryID
        uint32    Totem[2];                      // 52-53    m_totem

    /// CUSTOM: these fields are used for the modifications made in the world.cpp
    uint32 School;
    uint32 proc_interval;               //!!! CUSTOM, Time(In MS) between proc's.
    float ProcsPerMinute;               //!!! CUSTOM, number of procs per minute
    uint32 buffIndexType;               //!!! CUSTOM, Tells us what type of buff it is, so we can limit the amount of them.
    uint32 c_is_flags;                  //!!! CUSTOM, store spell checks in a static way : isdamageind,ishealing
    uint32 buffType;                    //!!! CUSTOM, these are related to creating a item through a spell
    uint32 RankNumber;                  //!!! CUSTOM, this protects players from having >1 rank of a spell
    uint32 NameHash;                    //!!! CUSTOM, related to custom spells, summon spell quest related spells
    float base_range_or_radius;         //!!! CUSTOM, needed for aoe spells most of the time
    float base_range_or_radius_sqr;     //!!! CUSTOM, needed for aoe spells most of the time
    float base_range_or_radius_friendly;//!!! CUSTOM, needed for aoe spells most of the time
    float base_range_or_radius_sqr_friendly;//!!! CUSTOM, needed for aoe spells most of the time
    uint32 talent_tree;                 //!!! CUSTOM, Used for dumping class spells.
    bool is_melee_spell;                //!!! CUSTOM, Obvious.
    bool is_ranged_spell;               //!!! CUSTOM, Obvious.
    bool spell_can_crit;                //!!! CUSTOM, Obvious.
    uint32 trnr_req_clsmsk;             //!!! CUSTOM, Required class mask to learn at a trainer.

    /* Crow:
    Custom: The amount of threat the spell will generate.
    This is loaded from a DB table, and if it isn't there, threat is always damage. */
    uint32 ThreatForSpell;
    float cone_width; // love me or hate me, all "In a cone in front of the caster" spells don't necessarily mean "in front"
    //Spell Coefficient
    bool isAOE;                         //!!! CUSTOM, Obvious.
    float SP_coef_override;             //!!! CUSTOM, overrides any spell coefficient calculation and use this value
    float AP_coef_override;             //!!! CUSTOM, Additional coef from ap
    float RAP_coef_override;            //!!! CUSTOM, Additional coef from RAP
    bool self_cast_only;                //!!! CUSTOM, Obvious.
    bool apply_on_shapeshift_change;    //!!! CUSTOM, Obvious.
    bool always_apply;                  //!!! CUSTOM, Obvious.
    uint32 auraimmune_flag;             //!!! CUSTOM, this var keeps aura effects in binary format.
    bool Unique;                        //!!! CUSTOM, Is this a unique effect? ex: Mortal Strike -50% healing.

    uint32 area_aura_update_interval;
    uint32 skilline;
    /* Crow:
    SpellId used to send log to client for this spell
    This is overwritten sometimes with proc's */
    uint32 logsId;
    uint32 AdditionalAura;
    uint32 forced_creature_target;
    uint32 AreaAuraTarget;

    //poisons type...
    uint32 poison_type;                 //!!! CUSTOM, Type of poison it is.

    //backattack
    bool AllowBackAttack;               //!!! CUSTOM, Obvious.

    // Crow: The following are customs made by me, mostly duplicate fields for handling more information.
    uint32 procflags2; // We get two now, hurray. One does not take the place of the other.

    // Queries/Commands:
    bool IsChannelSpell() { return ((AttributesEx & (0x04|0x40)) ? true : (ChannelInterruptFlags != 0 ? true : false)); }
    bool HasEffect(uint32 spellEffect) { return (Effect[0] == spellEffect || Effect[1] == spellEffect || Effect[2] == spellEffect); };
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

struct SpellCastTime
{
    uint32 ID;
    uint32 CastTime;
    //uint32 unk1;
    //uint32 unk2;
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
    uint32    Id;                                           // 0        m_ID
    //uint32    modalNextSpell;                             // 1        m_modalNextSpell not used
    uint32    SpellFamilyFlags[3];                             // 2-4
    uint32    SpellFamilyName;                              // 5      m_spellClassSet
    //DBCString Description;                                // 6 4.0.0
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

struct SpellDuration
{
    uint32 ID;
    uint32 Duration1;
    uint32 Duration2;
    uint32 Duration3;
};

// SpellEffect.dbc
struct SpellEffectEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    Effect;                                       // 1        m_effect
    float     EffectValueMultiplier;                        // 2        m_effectAmplitude
    uint32    EffectApplyAuraName;                          // 3        m_effectAura
    uint32    EffectAmplitude;                              // 4        m_effectAuraPeriod
    int32     EffectBasePoints;                             // 5        m_effectBasePoints (don't must be used in spell/auras explicitly, must be used cached Spell::m_currentBasePoints)
    float     EffectBonusCoefficient;                       // 6        m_effectBonusCoefficient
    float     EffectDamageMultiplier;                       // 7        m_effectChainAmplitude
    uint32    EffectChainTarget;                            // 8        m_effectChainTargets
    int32     EffectDieSides;                               // 9        m_effectDieSides
    uint32    EffectItemType;                               // 10       m_effectItemType
    uint32    EffectMechanic;                               // 11       m_effectMechanic
    int32     EffectMiscValue;                              // 12       m_effectMiscValue
    int32     EffectMiscValueB;                             // 13       m_effectMiscValueB
    float     EffectPointsPerComboPoint;                    // 14       m_effectPointsPerCombo
    uint32    EffectRadiusIndex;                            // 15       m_effectRadiusIndex - spellradius.dbc
    //uint32   EffectRadiusMaxIndex;                        // 16       4.0.0
    float     EffectRealPointsPerLevel;                     // 17       m_effectRealPointsPerLevel
    uint32    EffectSpellClassMask[3];                      // 18       m_effectSpellClassMask, effect 0
    uint32    EffectTriggerSpell;                           // 19       m_effectTriggerSpell
    uint32    EffectImplicitTargetA;                        // 20       m_implicitTargetA
    uint32    EffectImplicitTargetB;                        // 21       m_implicitTargetB
    uint32    EffectSpellId;                                // 22       new 4.0.0
    uint32    EffectIndex;                                  // 23       new 4.0.0
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
    uint32    Id;                                           // 0        m_ID
    uint32    manaCost;                                     // 1        m_manaCost
    uint32    manaCostPerlevel;                             // 2        m_manaCostPerLevel
    uint32    manaCostPercentage;                           // 3        m_manaCostPct
    uint32    manaPerSecond;                                // 4        m_manaPerSecond
    //uint32  PowerDisplayId;                               // 5        PowerDisplay.dbc, new in 3.1
    //uint32  unk1;                                         // 6        4.0.0
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
    uint32 runePowerGain;
};

// SpellScaling.dbc
struct SpellScalingEntry
{
    uint32    Id;                                           // 0        m_ID
    int32     castTimeMin;                                  // 1        minimum cast time
    int32     castTimeMax;                                  // 2        maximum cast time
    uint32    castScalingMaxLevel;                          // 3        first level with maximum cast time
    int32     playerClass;                                  // 4        (index * 100) + charLevel => gtSpellScaling.dbc
    float     Multiplier[3];                                // 5-7
    float     RandomMultiplier[3];                          // 8-10
    float     OtherMultiplier[3];                           // 11-13
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
    uint32    Id;                                           // 0        m_ID
    uint32    MaxAffectedTargets;                           // 1        m_maxTargets
    uint32    MaxTargetLevel;                               // 2        m_maxTargetLevel
    uint32    TargetCreatureType;                           // 3        m_targetCreatureType
    uint32    Targets;                                      // 4        m_targets
};

// SpellTotems.dbc
struct SpellTotemsEntry
{
    uint32    Id;                                           // 0        m_ID
    uint32    TotemCategory[2];                             // 1-2      m_requiredTotemCategoryID
    uint32    Totem[2];                                     // 3-4      m_totem
};

struct SpellRadius
{
    uint32 ID;
    float radiusHostile;
    //float unk1;
    float radiusFriend;
};

struct SpellRange
{
    uint32 ID;
    float minRangeHostile;
    float minRangeFriend;
    float maxRangeHostile;
    float maxRangeFriend;
};

struct SummonPropertiesEntry
{
    uint32 Id;
    uint32 controltype;
    uint32 factionId;
    uint32 type;
    uint32 slot;
    uint32 unk2;
};

struct TalentEntry
{
    uint32  TalentID;
    uint32  TalentTree;
    uint32  Row;
    uint32  Col;
    uint32  RankID[5];
    uint32  DependsOn;
    uint32  DependsOnRank;
};

struct TalentTabEntry
{
    uint32    TalentTabID;
    uint32    ClassMask;
    uint32    TabPage;
};

struct TalentTreePrimarySpellsEntry
{
    uint32 Id;
    uint32 TalentTabID;
    uint32 SpellID;
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

#pragma pack(pop)

#define ConstructDBCStorageIterator(_class) DBStorage<_class, DBC<_class> >::iterator
#define ConstructDB2StorageIterator(_class) DBStorage<_class, DB2<_class> >::iterator

template<class T, class T2> class SERVER_DECL DBStorage
{
    T2 *m_file;

public:
    class iterator
    {
    private:
        T* p;
    public:
        iterator(T* ip = 0) : p(ip) { };
        iterator& operator++() { ++p; return *this; };
        bool operator != (const iterator &i) { return (p != i.p); };
        bool operator == (const iterator &i) { return (p == i.p); };
        T* operator*() { return p; };
    };

    iterator begin() { return iterator(m_file->GetFirstBlock()); }
    iterator end() { return iterator(m_file->GetLastBlock()); }

    DBStorage() { m_file = new T2(); }
    ~DBStorage() { delete m_file; }

    bool Load(const char * filename, const char * format) { return m_file->Load(filename, format); }

    uint32 GetNumRows() { return m_file->GetNumRows(); }
    uint32 GetMaxRow() { return m_file->GetMaxRow(); }

    T * LookupEntryTest(uint32 i) { return m_file->LookupEntryTest(i); }
    T * LookupEntry(uint32 i) { return m_file->LookupEntry(i); }
    T * LookupRow(uint32 i) { return m_file->LookupRow(i); }

    void SetRow(uint32 i, T * t) { return m_file->SetRow(i, t); }
};

HEARTHSTONE_INLINE float GetDBCScale(CreatureDisplayInfo *Scale)
{
    if(Scale && Scale->Scale)
        return Scale->Scale;
    return 1.0f;
}

HEARTHSTONE_INLINE float GetDBCRadius(SpellRadius *radius)
{
    if(radius)
    {
        if(radius->radiusHostile)
            return radius->radiusHostile;
        else
            return radius->radiusFriend;
    }

    return 0.0f;
}

HEARTHSTONE_INLINE uint32 GetDBCCastTime(SpellCastTime *time)
{
    if(time && time->CastTime)
        return time->CastTime;
    return 0;
}

HEARTHSTONE_INLINE float GetDBCMaxRange(SpellRange *range)
{
    if(range)
    {
        if(range->maxRangeHostile)
            return range->maxRangeHostile;
        else
            return range->maxRangeFriend;
    }

    return 0.0f;
}

HEARTHSTONE_INLINE float GetDBCMinRange(SpellRange *range)
{
    if(range)
    {
        if(range->minRangeHostile)
            return range->minRangeHostile;
        else
            return range->minRangeFriend;
    }

    return 0.0f;
}

HEARTHSTONE_INLINE int32 GetDBCDuration(SpellDuration *dur)
{
    if(dur && dur->Duration1)
        return dur->Duration1;
    return -1;
}

HEARTHSTONE_INLINE float GetDBCFriendlyRadius(SpellRadius *radius)
{
    if(radius == NULL)
        return 0.0f;

    if(radius->radiusFriend)
        return radius->radiusFriend;

    return GetDBCRadius(radius);
}

HEARTHSTONE_INLINE float GetDBCFriendlyMaxRange(SpellRange *range)
{
    if(range == NULL)
        return 0.0f;

    if(range->maxRangeFriend)
        return range->maxRangeFriend;

    return GetDBCMaxRange(range);
}

HEARTHSTONE_INLINE float GetDBCFriendlyMinRange(SpellRange *range)
{
    if(range == NULL)
        return 0.0f;

    if(range->minRangeFriend)
        return range->minRangeFriend;

    return GetDBCMinRange(range);
}

#define DBC_STORAGE_EXTERN_DBC_MACRO(EntryClass, DeclaredClass) extern SERVER_DECL DBStorage<EntryClass, DBC<EntryClass>> DeclaredClass
#define DBC_STORAGE_EXTERN_DB2_MACRO(EntryClass, DeclaredClass) extern SERVER_DECL DBStorage<EntryClass, DB2<EntryClass>> DeclaredClass

DBC_STORAGE_EXTERN_DBC_MACRO(AchievementEntry, dbcAchievement);
DBC_STORAGE_EXTERN_DBC_MACRO(AchievementCriteriaEntry, dbcAchievementCriteria);
DBC_STORAGE_EXTERN_DBC_MACRO(AreaGroupEntry, dbcAreaGroup);
DBC_STORAGE_EXTERN_DBC_MACRO(AreaTableEntry, dbcAreaTable);
DBC_STORAGE_EXTERN_DBC_MACRO(AreaTriggerEntry, dbcAreaTrigger);
DBC_STORAGE_EXTERN_DBC_MACRO(AuctionHouseDBC, dbcAuctionHouse);
DBC_STORAGE_EXTERN_DBC_MACRO(BankSlotPriceEntry, dbcBankSlotPrices);
DBC_STORAGE_EXTERN_DBC_MACRO(BarberShopStyleEntry, dbcBarberShopStyle);
DBC_STORAGE_EXTERN_DBC_MACRO(BattleMasterListEntry, dbcBattleMasterList);
DBC_STORAGE_EXTERN_DBC_MACRO(CharClassEntry, dbcCharClass);
DBC_STORAGE_EXTERN_DBC_MACRO(CharRaceEntry, dbcCharRace);
DBC_STORAGE_EXTERN_DBC_MACRO(ChatChannelDBC, dbcChatChannels);
DBC_STORAGE_EXTERN_DBC_MACRO(CreatureBoundData, dbcCreatureBoundData);
DBC_STORAGE_EXTERN_DBC_MACRO(CreatureDisplayInfo, dbcCreatureDisplayInfo);
DBC_STORAGE_EXTERN_DBC_MACRO(CreatureFamilyEntry, dbcCreatureFamily);
DBC_STORAGE_EXTERN_DBC_MACRO(CreatureSpellDataEntry, dbcCreatureSpellData);
DBC_STORAGE_EXTERN_DBC_MACRO(CharTitlesEntry, dbcCharTitles);
DBC_STORAGE_EXTERN_DBC_MACRO(DBCTaxiNode, dbcTaxiNode);
DBC_STORAGE_EXTERN_DBC_MACRO(DBCTaxiPath, dbcTaxiPath);
DBC_STORAGE_EXTERN_DBC_MACRO(DBCTaxiPathNode, dbcTaxiPathNode);
DBC_STORAGE_EXTERN_DBC_MACRO(DurabilityCostsEntry, dbcDurabilityCosts);
DBC_STORAGE_EXTERN_DBC_MACRO(DurabilityQualityEntry, dbcDurabilityQuality);
DBC_STORAGE_EXTERN_DBC_MACRO(EmoteEntry, dbcEmoteEntry);
DBC_STORAGE_EXTERN_DBC_MACRO(EnchantEntry, dbcEnchant);
DBC_STORAGE_EXTERN_DBC_MACRO(FactionEntry, dbcFaction);
DBC_STORAGE_EXTERN_DBC_MACRO(FactionTemplateEntry, dbcFactionTemplate);
DBC_STORAGE_EXTERN_DBC_MACRO(GemPropertyEntry, dbcGemProperty);
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
DBC_STORAGE_EXTERN_DBC_MACRO(ItemExtendedCostEntry, dbcItemExtendedCost);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemLimitCategoryEntry, dbcItemLimitCategory);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemRandomPropertiesEntry, dbcItemRandomProperties);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemRandomSuffixEntry, dbcItemRandomSuffix);
DBC_STORAGE_EXTERN_DBC_MACRO(ItemSetEntry, dbcItemSet);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcCombatRating);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcBarberShopPrices);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcMeleeCrit);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcMeleeCritBase);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcSpellCrit);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcSpellCritBase);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcManaRegen);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcManaRegenBase);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcHPRegen);
DBC_STORAGE_EXTERN_DBC_MACRO(gtFloat, dbcCombatRatingScaling);
DBC_STORAGE_EXTERN_DBC_MACRO(Lock, dbcLock);
DBC_STORAGE_EXTERN_DBC_MACRO(LFGDungeonsEntry, dbcLFGDungeons);
DBC_STORAGE_EXTERN_DBC_MACRO(MapEntry, dbcMap);
DBC_STORAGE_EXTERN_DBC_MACRO(GlyphProperties, dbcGlyphProperties);
DBC_STORAGE_EXTERN_DBC_MACRO(NumTalentsAtLevelEntry, dbcNumTalents);
DBC_STORAGE_EXTERN_DBC_MACRO(QuestXPEntry, dbcQuestXP);
DBC_STORAGE_EXTERN_DBC_MACRO(SkillLineEntry, dbcSkillLine);
DBC_STORAGE_EXTERN_DBC_MACRO(SkillLineSpell, dbcSkillLineSpell);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellAuraOptionsEntry, dbcSpellAuraOptions);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellAuraRestrictionsEntry, dbcSpellAuraRestrictions);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellCastingRequirementsEntry, dbcSpellCastingRequirements);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellCastTime, dbcSpellCastTime);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellCategoriesEntry, dbcSpellCategories);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellClassOptionsEntry, dbcSpellClassOptions);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellCooldownsEntry, dbcSpellCooldowns);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellDifficultyEntry, dbcSpellDifficulty);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellDuration, dbcSpellDuration);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellEffectEntry, dbcSpellEffect);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellEntry, dbcSpell);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellEquippedItemsEntry, dbcSpellEquippedItems);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellInterruptsEntry, dbcSpellInterrupts);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellLevelsEntry, dbcSpellLevels);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellPowerEntry, dbcSpellPower);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellRadius, dbcSpellRadius);
DBC_STORAGE_EXTERN_DBC_MACRO(SpellRange, dbcSpellRange);
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
DBC_STORAGE_EXTERN_DBC_MACRO(VehicleEntry, dbcVehicle);
DBC_STORAGE_EXTERN_DBC_MACRO(VehicleSeatEntry, dbcVehicleSeat);
DBC_STORAGE_EXTERN_DBC_MACRO(WMOAreaTableEntry, dbcWMOAreaTable);
DBC_STORAGE_EXTERN_DBC_MACRO(WorldMapOverlayEntry, dbcWorldMapOverlay);
DBC_STORAGE_EXTERN_DBC_MACRO(WorldSafeLocsEntry, dbcWorldSafeLocs);

bool LoadDBCs(const char* datapath);
