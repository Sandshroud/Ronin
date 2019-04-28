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

class Group;

#pragma pack(PRAGMA_PACK)

struct PlayerCreateInfo
{
    uint8  race;
    uint8  class_;
    uint32 factiontemplate;
    uint32 mapId;
    uint32 zoneId;
    float  positionX;
    float  positionY;
    float  positionZ;
    float  Orientation;
    uint32 displayId[2];
    uint32 requiredExpansion;

    std::set<uint32> spell_list;
    std::list<CreateInfo_ItemStruct> items;
    std::list<CreateInfo_ActionBarStruct> bars;
};

struct VendorItem
{
    uint32 itemid;
    uint32 max_amount;
    uint32 incrtime;
    uint32 vendormask;
    uint32 extendedCost;
    bool IsDependent;
};

struct PageText
{
    uint32 id;
    std::string text;
    uint32 next_page;
};

#pragma pack(PRAGMA_POP)

enum MONSTER_SAY_EVENTS
{
    MONSTER_SAY_EVENT_ENTER_COMBAT      = 0,
    MONSTER_SAY_EVENT_RANDOM_WAYPOINT   = 1,
    MONSTER_SAY_EVENT_CALL_HELP         = 2,
    MONSTER_SAY_EVENT_ON_COMBAT_STOP    = 3,
    MONSTER_SAY_EVENT_ON_DAMAGE_TAKEN   = 4,
    MONSTER_SAY_EVENT_ON_DIED           = 5,
    MONSTER_SAY_EVENT_ON_SPAWN          = 6,
    NUM_MONSTER_SAY_EVENTS,
};

enum AREATABLE_FLAGS
{
    AREA_SNOW                   = 0x00000001,   // Snow (only Dun Morogh, Naxxramas, Razorfen Downs and Winterspring)
    AREA_CAPITAL_SUB            = 0x00000008,   // City and city subsones
    AREA_CITY_AREA              = 0x00000020,   // Slave capital city flag?
    AREA_NEUTRAL_AREA           = 0x00000040,   // Many zones have this flag
    AREA_PVP_ARENA              = 0x00000080,   // Arena, both instanced and world arenas
    AREA_CAPITAL                = 0x00000100,   // Main capital city flag
    AREA_CITY                   = 0x00000200,   // Only for one zone named "City" (where it located?)
    AREA_FLYING_PERMITTED       = 0x00000400,   // Expansion zones? (only Eye of the Storm not have this flag, but have 0x00004000 flag)
    AREA_SANCTUARY              = 0x00000800,   // Sanctuary area (PvP disabled)
    AREA_ISLAND                 = 0x00001000,   // Only Netherwing Ledge, Socrethar's Seat, Tempest Keep, The Arcatraz, The Botanica, The Mechanar, Sorrow Wing Point, Dragonspine Ridge, Netherwing Mines, Dragonmaw Base Camp, Dragonmaw Skyway
    AREA_OUTLAND2               = 0x00004000,   // Expansion zones? (only Circle of Blood Arena not have this flag, but have 0x00000400 flag)
    AREA_PVP_OBJECTIVE_AREA     = 0x00008000,   // Pvp objective area? (Death's Door also has this flag although it's no pvp object area)
    AREA_ARENA_INSTANCE         = 0x00010000,   // Used by instanced arenas only
    AREA_LOWLEVEL               = 0x00100000,   // Used for some starting areas with area_level <=15
    AREA_TOWN                   = 0x00200000,   // Small towns with Inn
    AREA_OUTDOOR_PVP            = 0x01000000,   // Wintergrasp and it's subzones
    AREA_INSIDE                 = 0x02000000,   // Used for determinating spell related inside/outside questions in Map::IsOutdoors
    AREA_OUTSIDE                = 0x04000000,   // Used for determinating spell related inside/outside questions in Map::IsOutdoors
    AREA_CAN_HEARTH_AND_RES     = 0x08000000,   // Wintergrasp and it's subzones
    AREA_CANNOT_FLY             = 0x20000000    // Not allowed to fly, only used in Dalaran areas (zone 4395)
};

enum AREATABLE_CATEGORY
{
    AREAC_CONTESTED             = 0,
    AREAC_ALLIANCE_TERRITORY    = 2,
    AREAC_HORDE_TERRITORY       = 4,
    AREAC_SANCTUARY             = 6,
};

#define MAX_PREDEFINED_NEXTLEVELXP 85
static const uint32 NextLevelXp[MAX_PREDEFINED_NEXTLEVELXP]=
{
    0,          400,        900,        1400,       2100,       2800,       3600,       4500,       5400,       6500,
    7600,       8700,       9800,       11000,      12300,      13600,      15000,      16400,      17800,      19300,
    20800,      22400,      24000,      25500,      27200,      28900,      30500,      32200,      33900,      36300,
    38800,      41600,      44600,      48000,      51400,      55000,      58700,      62400,      66200,      70200,
    74300,      78500,      82800,      87100,      91600,      95300,      101000,     105800,     110700,     115700,
    120900,     126100,     131500,     137000,     142500,     148200,     154000,     159900,     165800,     172000,
    290000,     317000,     349000,     386000,     428000,     475000,     527000,     585000,     648000,     717000,
    1219040,    1231680,    1244560,    1257440,    1270320,    1283360,    1296560,    1309920,    1323120,    1336640,
    1686300,    2121500,    4004000,    5203400,    9165100};

typedef std::map<uint32, std::list<SpellEntry*>* > OverrideIdMap;
typedef std::map<std::string, PlayerInfo*> PlayerNameStringIndexMap;

struct RecallLocation
{
    uint32 mapId;
    float x, y, z;
    float orient;
    std::string lowercase_name, RealName;
};

class SERVER_DECL ObjectMgr : public Singleton < ObjectMgr >
{
    struct AreaTriggerData;

public:
    ObjectMgr();
    ~ObjectMgr();

    // other objects

    // Set typedef's
    typedef std::set<AchievementCriteriaEntry*>                  AchievementCriteriaSet;
    typedef std::set<RecallLocation*>                            RecallSet;
    typedef std::map<uint32, TrainerSpell>                       TrainerSpellMap;

    // HashMap typedef's
    typedef std::map<uint64, Item* >                             ItemMap;
    typedef std::map<uint32, CorpseData*>                        CorpseCollectorMap;
    typedef std::map<uint32, PlayerInfo*>                        PlayerNameMap;
    typedef std::map<uint16, PlayerCreateInfo*>                  PlayerCreateInfoMap;
    typedef std::map<uint32, SkillLineAbilityEntry*>             SLMap;
    typedef std::map<uint32, std::vector<VendorItem> >           VendorMap;
    typedef std::map<uint32, TrainerData >                       TrainerDataMap;
    typedef std::map<std::pair<uint8, uint16>, TrainerSpellMap>  TrainerSpellStorage;
    typedef std::map<uint32, Corpse* >                           CorpseMap;
    typedef std::map<uint32, Group*>                             GroupMap;

    // Map typedef's
    typedef std::map<uint32, std::list<ItemPrototype*>* >        ItemSetContentMap;
    typedef std::map<uint32, uint32>                             NpcToGossipTextMap;
    typedef std::map<uint32, AchievementCriteriaSet*>            AchievementCriteriaMap;

    // WMO tables
    typedef std::map<std::pair<uint32, std::pair<uint32, uint32> >, WMOAreaTableEntry*> WMOAreaTableMap;

    // object holder
    OverrideIdMap       mOverrideIdMap;

    Player* GetPlayer(const char* name, bool caseSensitive = true);
    Player* GetPlayer(WoWGuid guid);

    CorpseMap m_corpses;
    Mutex _corpseslock;
    Mutex m_corpseguidlock;
    uint32 m_hiCorpseGuid;

    Mutex m_achievementLock;
    AchievementCriteriaMap m_achievementCriteriaMap;

    Mutex _recallLock;
    RecallSet m_recallLocations;

    void ResetDailies();

    Item* CreateItem(uint32 entry,Player* owner, uint32 count = 1);
    Item* LoadItem(uint64 guid);

    // Groups
    Group * GetGroupByLeader(Player* pPlayer);
    Group * GetGroupById(uint32 id);
    uint32 GenerateGroupId()
    {
        uint32 r;
        m_guidGenMutex.Acquire();
        r = ++m_hiGroupId;
        m_guidGenMutex.Release();
        return r;
    }

    void AddGroup(Group* group)
    {
        m_groupLock.AcquireWriteLock();
        m_groups.insert(std::make_pair(group->GetID(), group));
        m_groupLock.ReleaseWriteLock();
    }

    void RemoveGroup(Group* group)
    {
        m_groupLock.AcquireWriteLock();
        m_groups.erase(group->GetID());
        m_groupLock.ReleaseWriteLock();
    }

    void GroupVoiceReconnected();

    void LoadGroups();

    // player names
    void AddPlayerInfo(PlayerInfo *pn);
    PlayerInfo *LoadPlayerInfo(WoWGuid guid);
    void UpdatePlayerData(WoWGuid guid, QueryResult *result);

    PlayerInfo *GetPlayerInfo(WoWGuid guid );
    PlayerInfo *GetPlayerInfoByName(const char * name);
    void RenamePlayerInfo(PlayerInfo * pn, const char * oldname, const char * newname);
    void DeletePlayerInfo(uint32 guid);
    PlayerCreateInfo* GetPlayerCreateInfo(uint8 race, uint8 class_) const;
    bool CheckPlayerCreateInfo(uint8 race, uint8 class_) { return mPlayerCreateInfo.find(((uint16)(((uint16)class_)|(((uint16)race)<<8)))) != mPlayerCreateInfo.end(); }

    // Gameobject Stuff
    std::map<uint32, std::set<uint32> > GameObjectInvolvedQuestIds;
    void AddInvolvedQuestIds(uint32 entry, std::set<uint32> set) { GameObjectInvolvedQuestIds.insert(std::make_pair(entry, set)); };
    void RemoveInvolvedQuestIds(uint32 entry) { std::map<uint32, std::set<uint32> >::iterator itr = GameObjectInvolvedQuestIds.find(entry); if(itr != GameObjectInvolvedQuestIds.end()) GameObjectInvolvedQuestIds.erase(itr); };
    std::set<uint32>* GetInvolvedQuestIds(uint32 entry)
    {
        if(GameObjectInvolvedQuestIds.size())
        {
            std::map<uint32, std::set<uint32> >::iterator itr = GameObjectInvolvedQuestIds.find(entry);
            if(itr != GameObjectInvolvedQuestIds.end())
                return &itr->second;
        }
        return NULL;
    };

    //Corpse Stuff
    Corpse* GetCorpseByOwner(uint32 ownerguid);
    void CorpseCollectorUnload(bool saveOnly = false);
    void DespawnCorpse(uint64 Guid);
    void CorpseAddEventDespawn(Corpse* pCorpse);
    void DelinkPlayerCorpses(Player* pOwner);
    Corpse* CreateCorpse();
    void AddCorpse(Corpse* p);
    void RemoveCorpse(Corpse* p);
    void RemoveCorpse(uint32 corpseguid);
    Corpse* GetCorpse(uint32 corpseguid);

    //Vendors
    void FillVendorList(uint32 entry, uint32 vendorMask, std::vector<AvailableCreatureItem> &toFill);
    std::list<ItemPrototype*>* GetListForItemSet(uint32 setid);

    // Trainers
    TrainerData *GetTrainerData(uint32 entry);
    TrainerSpellMap *GetTrainerSpells(uint8 category, uint16 subcategory);

    uint32 GenerateArenaTeamId()
    {
        uint32 ret = 0;
        return ret;
    }

    Mutex m_petlock;

    PlayerInfo* CreatePlayer();
    Mutex m_playerguidlock;
    typedef std::map<uint64, Player*> PlayerStorageMap;
    PlayerStorageMap _players;
    RWLock _playerslock;
    uint32 m_hiPlayerGuid;

    void AddPlayer(Player* p);//add it to global storage
    void RemovePlayer(Player* p);

    // Serialization
    void LoadPlayersInfo();
    void LoadPlayerCreateInfo();
    Corpse* LoadCorpse(uint32 guid);
    void LoadCorpses(MapInstance* instance);
    void LoadVendors();
    void ReloadVendors();
    void ProcessTitles();
    void ProcessCreatureFamilies();

    void LoadTrainers();

    ObjectMgr::AreaTriggerData *GetAreaTriggerData(uint32 entry);
    bool GetDungeonEntrance(uint32 mapId, LocationVector *entrance);
    void LoadAreaTriggerData();

    void SetHighestGuids();
    void ListGuidAmounts();
    uint32 GeneratePlayerGuid();
    uint32 GenerateMailID();

    uint64 GenerateEquipmentSetGuid();
    uint32 GenerateItemGuid();

    void LoadExtraItemStuff();

    bool GetCreatureFamilyIDFromName(const char *name, uint32 &out);

    QueryResult* SQLCheckExists(const char* tablename, const char* columnname, uint64 columnvalue);

    std::map<uint32, uint32> ItemsInSets;

    void HashWMOAreaTables();
    WMOAreaTableEntry* GetWMOAreaTable(int32 adtid, int32 rootid, int32 groupid);

protected:
    WMOAreaTableMap WMOAreaTables;
    RWLock playernamelock;
    uint32 m_mailid;
    uint64 m_equipmentSetGuid;
    // highest GUIDs, used for creating new objects
    Mutex m_guidGenMutex;
    uint32 m_hiItemGuid;
    uint32 m_hiGroupId;

    std::map<WoWGuid,PlayerInfo*> m_playersinfo;
    PlayerNameStringIndexMap m_playersInfoByName;

    ///// WorldObject Tables ////
    // These tables are modified as creatures are created and destroyed in the world

    // Group List
    RWLock m_groupLock;
    GroupMap m_groups;

    // Map of all starting infos needed for player creation
    PlayerCreateInfoMap mPlayerCreateInfo;

    // Map of all vendor goods
    VendorMap mVendors;

    // Map of all trainer data
    TrainerDataMap mTrainerData;
    TrainerSpellStorage mTrainerSpellStorage;

    std::map<std::string, uint32> m_creatureFamilyNameToID;

    //Corpse Collector
    CorpseCollectorMap mCorpseCollector;

    ItemSetContentMap mItemSets;

public:
    enum AreaTriggerType
    {
        AREATRIGGER_TYPE_BASIC = 0,
        AREATRIGGER_TYPE_DUNGEON,
        AREATRIGGER_TYPE_UNK,
        AREATRIGGER_TYPE_INN,
        AREATRIGGER_TYPE_TELEPORT,
    };

    struct AreaTriggerData
    {
        uint32 Id;
        uint8 type;
        uint8 reqTeam;
        uint32 reqLevel;
        std::string name;
        struct TeleportDest
        {
            uint32 mapId;
            float x, y, z, o;
        } *destination;
    };

private:
    std::map<uint32, AreaTriggerData*> m_areaTriggerData;
    std::multimap<uint32, AreaTriggerData*> m_areaTriggerDungeonEntrances;

};


#define objmgr ObjectMgr::getSingleton()

//void SetProgressBar(int, int, const char*);
//void ClearProgressBar();
