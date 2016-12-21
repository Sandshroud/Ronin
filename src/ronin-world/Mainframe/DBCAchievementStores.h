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

#pragma once

#pragma pack(PRAGMA_PACK)

struct AchievementEntry
{
    uint32 ID;                      // 0
    uint32 factionFlag;             // 1 -1=all, 0=horde, 1=alliance
    uint32 mapID;                   // 2 -1=none
    uint32 Previous_achievement;    // 3 Lots of 0's
    char* name;                     // 4
    char* description;              // 5
    uint32 categoryId;              // 6 Category
    uint32 points;                  // 7 reward points
//  uint32 OrderInCategory;         // 8
    uint32 flags;                   // 9
//  uint32 icon;                    // 10 - SpellIcon.dbc
//  char* RewardTitle;              // 11 - Rewarded Title

//  Used for counting criteria.
//  Example: http://www.wowhead.com/achievement=1872
    uint32 count;                   // 12
    uint32 refAchievement;          // 13

    // Custom fields set when parsing
    uint32 reqRaceMask, reqClassMask;
};

enum AchievementFactionFlags : int8
{
    ACHIEVEMENT_FACTION_FLAG_ANY      = -0x1,
    ACHIEVEMENT_FACTION_FLAG_HORDE    = 0x00,
    ACHIEVEMENT_FACTION_FLAG_ALLIANCE = 0x01,
};

enum AchievementFlags : uint32
{
    ACHIEVEMENT_FLAG_NONE                   = 0x00000000,
    ACHIEVEMENT_FLAG_COUNTER                = 0x00000001,   // ACHIEVEMENT_FLAG_STATISTIC Just count statistic (never stop and complete)
    ACHIEVEMENT_FLAG_UNK2                   = 0x00000002,   // ACHIEVEMENT_FLAG_HIDDEN not used
    ACHIEVEMENT_FLAG_STORE_MAX_VALUE        = 0x00000004,   // ACHIEVEMENT_FLAG_HIDDEN_TILL_AWARDED Store only max value? used only in "Reach level xx"
    ACHIEVEMENT_FLAG_SUMM                   = 0x00000008,   // ACHIEVEMENT_FLAG_CUMULATIVE Use summ criteria value from all requirements (and calculate max value)
    ACHIEVEMENT_FLAG_MAX_USED               = 0x00000010,   // ACHIEVEMENT_FLAG_DISPLAY_HIGHEST Show max criteria (and calculate max value ??)
    ACHIEVEMENT_FLAG_REQ_COUNT              = 0x00000020,   // ACHIEVEMENT_FLAG_CRITERIA_COUNT Use not zero req count (and calculate max value)
    ACHIEVEMENT_FLAG_AVERAGE                = 0x00000040,   // ACHIEVEMENT_FLAG_AVG_PER_DAY Show as average value (value / time_in_days) depend from other flag (by def use last criteria value)
    ACHIEVEMENT_FLAG_BAR                    = 0x00000080,   // ACHIEVEMENT_FLAG_HAS_PROGRESS_BAR Show as progress bar (value / max vale) depend from other flag (by def use last criteria value)
    ACHIEVEMENT_FLAG_REALM_FIRST_REACH      = 0x00000100,   // ACHIEVEMENT_FLAG_SERVER_FIRST
    ACHIEVEMENT_FLAG_REALM_FIRST_KILL       = 0x00000200,   //
    ACHIEVEMENT_FLAG_UNK3                   = 0x00000400,   // ACHIEVEMENT_FLAG_HIDE_NAME_IN_TIE
    ACHIEVEMENT_FLAG_REALM_FIRST_GUILD      = 0x00000800,   // first guild on realm done something
    ACHIEVEMENT_FLAG_UNK4                   = 0x00001000,   // as guild group?
    ACHIEVEMENT_FLAG_UNK5                   = 0x00002000,   // as guild group?
    ACHIEVEMENT_FLAG_GUILD                  = 0x00004000,   //
    ACHIEVEMENT_FLAG_SHOW_GUILD_MEMBERS     = 0x00008000,   //
    ACHIEVEMENT_FLAG_SHOW_CRITERIA_MEMBERS  = 0x00010000,   //
};

enum AchievementCriteriaCondition : uint8
{
    ACHIEVEMENT_CRITERIA_CONDITION_NONE      = 0,
    ACHIEVEMENT_CRITERIA_CONDITION_NO_DEATH  = 1,
    ACHIEVEMENT_CRITERIA_CONDITION_UNK1      = 2,           // only used in "Complete a daily quest every day for five consecutive days"
    ACHIEVEMENT_CRITERIA_CONDITION_MAP       = 3,           // requires you to be on specific map
    ACHIEVEMENT_CRITERIA_CONDITION_NO_LOSE   = 4,           // only used in "Win 10 arenas without losing"
    ACHIEVEMENT_CRITERIA_CONDITION_UNK2      = 9,           // unk
    ACHIEVEMENT_CRITERIA_CONDITION_UNK3      = 13,          // unk
};

enum AchievementCriteriaCompletionFlags : uint8
{
    ACHIEVEMENT_CRITERIA_FLAG_PROGRESS_BAR      = 0x00000001,         // Show progress as bar
    ACHIEVEMENT_CRITERIA_FLAG_HIDDEN            = 0x00000002,         // Not show criteria in client
    ACHIEVEMENT_CRITERIA_FLAG_FAIL_ACHIEVEMENT  = 0x00000004,         // BG related??
    ACHIEVEMENT_CRITERIA_FLAG_RESET_ON_START    = 0x00000008,         //
    ACHIEVEMENT_CRITERIA_FLAG_IS_DATE           = 0x00000010,         // not used
    ACHIEVEMENT_CRITERIA_FLAG_IS_MONEY          = 0x00000020,         // Displays counter as money
    ACHIEVEMENT_CRITERIA_FLAG_IS_ACHIEVEMENT_ID = 0x00000040,
};

enum AchievementCriteriaTypes : uint8
{
    ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE = 0,
    ACHIEVEMENT_CRITERIA_TYPE_WIN_BG = 1,
    ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL = 5,
    ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL = 7,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT = 8,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT = 9,
    // you have to complete a daily quest x times in a row
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY = 10,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE = 11,
    ACHIEVEMENT_CRITERIA_TYPE_CURRENCY_EARNED = 12,
    ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE = 13,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST = 14,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND = 15,
    ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP = 16,
    ACHIEVEMENT_CRITERIA_TYPE_DEATH = 17,
    ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON = 18,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_RAID = 19,
    ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE = 20,
    ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER = 23,
    ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING = 24,
    ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM = 26,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST = 27,
    ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET = 28,
    ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL = 29,
    ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE = 30,
    ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA = 31,
    ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA = 32,
    ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA = 33,
    ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL = 34,
    // TODO: this criteria has additional conditions which can not be found in the dbcs
    ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL = 35,
    ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM = 36,
    // TODO: the archievements 1162 and 1163 requires a special rating which can't be found in the dbc
    ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA = 37,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING = 38,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING = 39,
    ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL = 40,
    ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM = 41,
    ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM = 42,
    ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA = 43,
    ACHIEVEMENT_CRITERIA_TYPE_OWN_RANK = 44,
    ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT = 45,
    ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION = 46,
    ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION = 47,
    // noted: rewarded as soon as the player payed, not at taking place at the seat
    ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP = 48,
    ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM = 49,
    // TODO: itemlevel is mentioned in text but not present in dbc
    ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT = 50,
    ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT = 51,
    ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS = 52,
    ACHIEVEMENT_CRITERIA_TYPE_HK_RACE = 53,
    ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE = 54,
    ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE = 55,
    // TODO: in some cases map not present, and in some cases need do without die
    ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS = 56,
    ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM = 57,
    ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS = 59,
    ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS = 60,
    ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS = 61,
    ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD = 62,
    ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING = 63,
    ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER = 65,
    ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL = 66,
    ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY = 67,
    ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT = 68,
    ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2 = 69,
    ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL = 70,
    // 71 does not exist in DBC
    ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT = 72,
    // 73 is wotlk related
    ACHIEVEMENT_CRITERIA_TYPE_HAS_TITLE = 74,
    ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS = 75,
    ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL = 76,
    ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL = 77,
    // TODO: creature type (demon, undead etc.) is not stored in dbc
    ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE = 78,
    ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS = 80,
    ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION = 82,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID = 83,
    ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS = 84,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD = 85,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED = 86,
    ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION = 87,
    ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION = 88,
    ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS = 89,
    ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM = 90,
    ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM = 91,
    ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED = 93,
    ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED = 94,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALTH = 95,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_POWER = 96,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_STAT = 97,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_SPELLPOWER = 98,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_ARMOR = 99,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_RATING = 100,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_DEALT = 101,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_RECEIVED = 102,
    ACHIEVEMENT_CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED = 103,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEAL_CASTED = 104,
    ACHIEVEMENT_CRITERIA_TYPE_TOTAL_HEALING_RECEIVED = 105,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED = 106,
    ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED = 107,
    ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN = 108,
    ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE = 109,
    // TODO: target entry is missing
    ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2 = 110,
    ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE = 112,
    ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL = 113,
    ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS = 114,
    ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS = 115,
    ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS = 119,
    // 120 unused
    // 121 unused
    // 122 unused
    // 123 unused
    ACHIEVEMENT_CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS = 124,
    ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL = 125,
    ACHIEVEMENT_CRITERIA_TYPE_CRAFT_ITEMS_GUILD = 126,
    ACHIEVEMENT_CRITERIA_TYPE_CATCH_FROM_POOL = 127,
    ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS = 128,
    ACHIEVEMENT_CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS = 129,
    ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND = 130,
    // 131 unused
    ACHIEVEMENT_CRITERIA_TYPE_REACH_BG_RATING = 132,
    ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_TABARD = 133,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_GUILD = 134,
    ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILLS_GUILD = 135,
    ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD = 136,
    // 137 no achievements
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE = 138,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE_2 = 139,
    // 140 no achievements
    // 141 no achievements
    // 142 no achievements
    // 143 no achievements
    // 144 no achievements
    // 145 no achievements
    // 146 no achievements
    // 147 no achievements
    // 148 no achievements
    // 149 no achievements
    // 150 no achievements
    // 0..150 => 151 criteria types total
};

#define ACHIEVEMENT_CRITERIA_TYPE_TOTAL 151

enum AchievementCriteriaMoreReqType : uint8
{
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_ITEM_LEVEL             = 3,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_CREATURE_ID            = 4,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_SPELL                  = 8,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_SPELL_ON_TARGET        = 10,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_ITEM_QUALITY_EQUIPPED  = 14,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_ITEM_QUALITY_LOOTED    = 15,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_AREA_ID                = 17,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_AREA_ID2               = 18,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_RAID_DIFFICULTY        = 20,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_ARENA_TYPE             = 24,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_PLAYER_CLASS           = 26,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_PLAYER_RACE            = 27,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_PLAYER_CLASS2          = 28,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_CREATURE_TYPE          = 30,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_MAP_ID                 = 32,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_TIMED_QUEST            = 35,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_PLAYER_TITLE           = 38,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_PLAYER_LEVEL           = 39,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_PLAYER_LEVEL2          = 40,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_AREA_ID3               = 41,
    ACHIEVEMENT_CRITERIA_MORE_REQ_TYPE_GUILD_REP              = 62,
};

struct AchievementCriteriaEntry
{
    uint32 ID;                      // 0
    uint32 referredAchievement;     // 1
    uint32 requiredType;            // 2
    union
    {
        // ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE = 0
        // TODO: also used for player deaths..
        struct
        {
            uint32 creatureID;                          // 3
            uint64 creatureCount;                       // 4
        } kill_creature;

        // ACHIEVEMENT_CRITERIA_TYPE_WIN_BG = 1
        // TODO: there are further criterias instead just winning
        struct
        {
            uint32 bgMapID;                             // 3
            uint64 winCount;                            // 4
        } win_bg;

        // ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL = 5
        struct
        {
            uint32 unused;                              // 3
            uint64 level;                               // 4
        } reach_level;

        // ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL = 7
        struct
        {
            uint32 skillID;                         // 3
            uint64 skillLevel;                          // 4
        } reach_skill_level;

        // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT = 8
        struct
        {
            uint32 linkedAchievement;                   // 3
        } complete_achievement;

        // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT = 9
        struct
        {
            uint32 unused;                              // 3
            uint64 totalQuestCount;                     // 4
        } complete_quest_count;

        // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY = 10
        struct
        {
            uint32 unused;                              // 3
            uint64 numberOfDays;                        // 4
        } complete_daily_quest_daily;

        // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE = 11
        struct
        {
            uint32 zoneID;                              // 3
            uint64 questCount;                          // 4
        } complete_quests_in_zone;

        // ACHIEVEMENT_CRITERIA_TYPE_CURRENCY_EARNED = 12
        struct
        {
            uint32 currencyId;                          // 3
            uint64 count;                               // 4
        } currencyEarned;

        // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST = 14
        struct
        {
            uint32 unused;                              // 3
            uint64 questCount;                          // 4
        } complete_daily_quest;

        // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND= 15
        struct
        {
            uint32 mapID;                               // 3
        } complete_battleground;

        // ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP= 16
        struct
        {
            uint32 mapID;                               // 3
        } death_at_map;

        // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_RAID = 19
        struct
        {
            uint32 groupSize;                           // 3 can be 5, 10 or 25
        } complete_raid;

        // ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE = 20
        struct
        {
            uint32 creatureEntry;                       // 3
        } killed_by_creature;

        // ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING = 24
        struct
        {
            uint32 unused;                              // 3
            uint64 fallHeight;                          // 4
        } fall_without_dying;

        // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST = 27
        struct
        {
            uint32 questID;                             // 3
            uint64 questCount;                          // 4
        } complete_quest;

        // ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET = 28
        // ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2= 69
        struct
        {
            uint32 spellID;                             // 3
            uint64 spellCount;                          // 4
        } be_spell_target;

        // ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL= 29
        struct
        {
            uint32 spellID;                             // 3
            uint64 castCount;                           // 4
        } cast_spell;

        // ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA = 31
        struct
        {
            uint32 areaID;                              // 3 Reference to AreaTable.dbc
            uint64 killCount;                           // 4
        } honorable_kill_at_area;

        // ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA = 32
        struct
        {
            uint32 mapID;                               // 3 Reference to Map.dbc
        } win_arena;

        // ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA = 33
        struct
        {
            uint32 mapID;                               // 3 Reference to Map.dbc
        } play_arena;

        // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL = 34
        struct
        {
            uint32 spellID;                         // 3 Reference to Map.dbc
        } learn_spell;

        // ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM = 36
        struct
        {
            uint32 itemID;                              // 3
            uint64 itemCount;                           // 4
        } own_item;

        // ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA = 37
        struct
        {
            uint32 unused;                              // 3
            uint64 count;                               // 4
        } win_rated_arena;

        // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING = 38
        struct
        {
            uint32 teamtype;                            // 3 {2,3,5}
        } highest_team_rating;

        // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING = 39
        struct
        {
            uint32 teamtype;                            // 3 {2,3,5}
            uint64 teamrating;                          // 4
        } highest_personal_rating;

        // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL = 40
        struct
        {
            uint32 skillID;                         // 3
            uint64 skillLevel;                          // 4 apprentice=1, journeyman=2, expert=3, artisan=4, master=5, grand master=6
        } learn_skill_level;

        // ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM = 41
        struct
        {
            uint32 itemID;                              // 3
            uint64 itemCount;                           // 4
        } use_item;

        // ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM = 42
        struct
        {
            uint32 itemID;                              // 3
            uint64 itemCount;                           // 4
        } loot_item;

        // ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA = 43
        struct
        {
            // TODO: This rank is _NOT_ the index from AreaTable.dbc
            uint32 areaReference;                       // 3
        } explore_area;

        // ACHIEVEMENT_CRITERIA_TYPE_OWN_RANK= 44
        struct
        {
            // TODO: This rank is _NOT_ the index from CharTitles.dbc
            uint32 rank;                                // 3
        } own_rank;

        // ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT= 45
        struct
        {
            uint32 unused;                              // 3
            uint64 numberOfSlots;                       // 4
        } buy_bank_slot;

        // ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION= 46
        struct
        {
            uint32 factionID;                           // 3
            uint64 reputationAmount;                    // 4 Total reputation amount, so 42000 = exalted
        } gain_reputation;

        // ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION= 47
        struct
        {
            uint32 unused;                              // 3
            uint64 numberOfExaltedFactions;             // 4
        } gain_exalted_reputation;

        // ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP = 48
        struct
        {
            uint32 unused;                              // 3
            uint64 numberOfVisits;                      // 4
        } visit_barber;

        // ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM = 49
        // TODO: where is the required itemlevel stored?
        struct
        {
            uint32 itemSlot;                            // 3
            uint64 count;                               // 4
        } equip_epic_item;

        // ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT = 50
        struct
        {
            uint32 rollValue;                           // 3
            uint64 count;                               // 4
        } roll_need_on_loot;

        // ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT = 51
        struct
        {
            uint32 rollValue;                           // 3
            uint64 count;                               // 4
        } roll_greed_on_loot;

        // ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS = 52
        struct
        {
            uint32 classID;                             // 3
            uint64 count;                               // 4
        } hk_class;

        // ACHIEVEMENT_CRITERIA_TYPE_HK_RACE = 53
        struct
        {
            uint32 raceID;                              // 3
            uint64 count;                               // 4
        } hk_race;

        // ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE = 54
        // TODO: where is the information about the target stored?
        struct
        {
            uint32 emoteID;                             // 3
            uint32 count;                               // 4
        } do_emote;

        // ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE = 55
        struct
        {
            uint32 unused;                              // 3
            uint64 count;                               // 4
            uint32 flag;                                // 5 =3 for battleground healing
            uint32 mapid;                               // 6
        } healing_done;

        // ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM = 57
        struct
        {
            uint32 itemID;                              // 3
            uint64 count;                               // 4
        } equip_item;

        // ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD= 62
        struct
        {
            uint32 unused;                              // 3
            uint64 goldInCopper;                        // 4
        } quest_reward_money;

        // ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY = 67
        struct
        {
            uint32 unused;                              // 3
            uint64 goldInCopper;                        // 4
        } loot_money;

        // ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT = 68
        struct
        {
            uint32 goEntry;                         // 3
            uint64 useCount;                            // 4
        } use_gameobject;

        // ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL= 70
        // TODO: are those special criteria stored in the dbc or do we have to add another sql table?
        struct
        {
            uint32 unused;                              // 3
            uint64 killCount;                           // 4
        } special_pvp_kill;

        // ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT = 72
        struct
        {
            uint32 goEntry;                         // 3
            uint64 lootCount;                           // 4
        } fish_in_gameobject;

        // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS= 75
        struct
        {
            uint32 skillLine;                           // 3
            uint64 spellCount;                          // 4
        } learn_skillline_spell;

        // ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL = 76
        struct
        {
            uint32 unused;                              // 3
            uint64 duelCount;                           // 4
        } win_duel;

        // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_POWER = 96
        struct
        {
            uint32 powerType;                           // 3 mana=0, 1=rage, 3=energy, 6=runic power
        } highest_power;

        // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_STAT = 97
        struct
        {
            uint32 statType;                            // 3 4=spirit, 3=int, 2=stamina, 1=agi, 0=strength
        } highest_stat;

        // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_SPELLPOWER = 98
        struct
        {
            uint32 spellSchool;                     // 3
        } highest_spellpower;

        // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_RATING = 100
        struct
        {
            uint32 ratingType;                          // 3
        } highest_rating;

        // ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE = 109
        struct
        {
            uint32 lootType;                            // 3 3=fishing, 2=pickpocket, 4=disentchant
            uint64 lootTypeCount;                       // 4
        } loot_type;

        // ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2 = 110
        struct
        {
            uint32 skillLine;                           // 3
            uint64 spellCount;                          // 4
        } cast_spell2;

        // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE= 112
        struct
        {
            uint32 skillLine;                           // 3
            uint64 spellCount;                          // 4
        } learn_skill_line;

        // ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL = 113
        struct
        {
            uint32 unused;                              // 3
            uint64 killCount;                           // 4
        } honorable_kill;

        // ACHIEVEMENT_CRITERIA_TYPE_LFG_USED = 114
        struct
        {
            uint32  unused;
            uint64  dungeonsComplete;
        } use_lfg;

        struct
        {
            uint32 value;                               // 3 main requirement
            uint64 count;                               // 4 main requirement count
            uint32 additionalRequirement_type1;
            uint32 additionalRequirement_value1;
        } raw;
    };

    //unk1[2];                                          // 7,8
    char* name;                                         // 9
    uint32 completionFlag;                              // 10
    uint32 timedData[3];                                // 11,12,13
    uint32 showOrder;                                   // 14
    //uint32 unk[3];                                    // 15,16,17
    //uint32 moreRequirement[3];                        // 18,19,20
    //uint32 moreRequirementValue[3];                   // 21,22,23

    uint32 getMaxCounter()
    {
        switch (requiredType)
        {
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG: return win_bg.winCount;
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE: return kill_creature.creatureCount;
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL: return reach_level.level;
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL: return reach_skill_level.skillLevel;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT: return 1;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT: return complete_quest_count.totalQuestCount;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE: return complete_quests_in_zone.questCount;
        case ACHIEVEMENT_CRITERIA_TYPE_CURRENCY_EARNED: return currencyEarned.count;
        case ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE: case ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE: return healing_done.count;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST: return complete_daily_quest.questCount;
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING: return fall_without_dying.fallHeight;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST: return 1;
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET: case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2: return be_spell_target.spellCount;
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL: case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2: return cast_spell.castCount;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL: return 1;
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM: return own_item.itemCount;
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA: return win_rated_arena.count;
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING: return highest_personal_rating.teamrating;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL: return learn_skill_level.skillLevel * 75;
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM: return use_item.itemCount;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM: return loot_item.itemCount;
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA: return 1;
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT: return buy_bank_slot.numberOfSlots;
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION: return gain_reputation.reputationAmount;
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION: return gain_exalted_reputation.numberOfExaltedFactions;
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP: return visit_barber.numberOfVisits;
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM: return equip_epic_item.count;
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT: case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT: return roll_greed_on_loot.count;
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS: return hk_class.count;
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE: return hk_race.count;
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE: return do_emote.count;
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM: return equip_item.count;
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD: return quest_reward_money.goldInCopper;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY: return loot_money.goldInCopper;
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT: return use_gameobject.useCount;
        case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL: return special_pvp_kill.killCount;
        case ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT: return fish_in_gameobject.lootCount;
        case ACHIEVEMENT_CRITERIA_TYPE_HAS_TITLE: return 1;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS: return learn_skillline_spell.spellCount;
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL: return win_duel.duelCount;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE: return loot_type.lootTypeCount;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE: return learn_skill_line.spellCount;
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL: return honorable_kill.killCount;
            // these achievements are all statistic-only criteria
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM:
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
        case ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:
        case ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID:
        case ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALTH:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_SPELLPOWER:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_ARMOR:
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED:
        case ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
        case ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
            return 0;
        }
        return 0xFFFFFFFF;
    }
};

#pragma pack(PRAGMA_POP)
