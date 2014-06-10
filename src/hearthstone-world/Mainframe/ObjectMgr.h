/***
 * Demonstrike Core
 */

#pragma once

HEARTHSTONE_INLINE bool FindXinYString(std::string x, std::string y)
{
    return y.find(x) != std::string::npos;
}

struct GM_Ticket
{
    uint64 guid;
    uint64 playerGuid;
    std::string name;
    uint32 level;
    uint32 map;
    float posX;
    float posY;
    float posZ;
    std::string message;
    uint32 timestamp;
    bool deleted;
    uint64 assignedToPlayer;
    std::string comment;
};

enum
{
    GM_TICKET_CHAT_OPCODE_NEWTICKET         = 1,
    GM_TICKET_CHAT_OPCODE_LISTSTART         = 2,
    GM_TICKET_CHAT_OPCODE_LISTENTRY         = 3,
    GM_TICKET_CHAT_OPCODE_CONTENT           = 4,
    GM_TICKET_CHAT_OPCODE_APPENDCONTENT     = 5,
    GM_TICKET_CHAT_OPCODE_REMOVED           = 6,
    GM_TICKET_CHAT_OPCODE_UPDATED           = 7,
    GM_TICKET_CHAT_OPCODE_ASSIGNED          = 8,
    GM_TICKET_CHAT_OPCODE_RELEASED          = 9,
    GM_TICKET_CHAT_OPCODE_COMMENT           = 10,
    GM_TICKET_CHAT_OPCODE_ONLINESTATE       = 11
};

#pragma pack(PRAGMA_PACK)
struct FishingZoneEntry
{
    uint32 ZoneID;
    uint32 MinSkill;
    uint32 MaxSkill;
};

struct ZoneGuardEntry
{
    uint32 ZoneID;
    uint32 HordeEntry;
    uint32 AllianceEntry;
};

struct AchievementReward
{
    uint32 AchievementID;
    uint32 AllianceTitle;
    uint32 HordeTitle;
    uint32 ItemID;
    bool MailMessage;
    uint32 SenderEntry;
    char* MessageHeader;
    char* MessageBody;
};

struct ProfessionDiscovery
{
    uint32 SpellId;
    uint32 SpellToDiscover;
    uint32 SkillValue;
    float Chance;
};

struct RandomItemCreation
{
    uint32 SpellId;
    uint32 ItemToCreate;
    uint32 Skill;
    uint32 Chance;
};

struct RandomCardCreation
{
    uint32 SpellId;
    uint32 ItemId[32];
    uint32 itemcount;
};

struct ScrollCreation
{
    uint32 SpellId;
    uint32 ItemId;
};

struct ItemPage
{
    uint32 id;
    char * text;
    uint32 next_page;
};

struct ItemQuestRelation
{
    uint32 item_id;
    uint32 quest_id;
    uint32 itemcount;
};

#pragma pack(PRAGMA_POP)

struct SpellReplacement
{
    uint32 count;
    uint32 *spells;
};

class Group;

struct GossipMenuItem
{
    uint32      Id;
    uint32      IntId;
    uint8       Icon;
    bool        Coded;
    uint32      BoxMoney;
    string      Text;
    string      BoxMessage;
};

struct SpellEntry;
struct TrainerSpell
{
    SpellEntry * pCastSpell;
    SpellEntry * pLearnSpell;
    SpellEntry * pCastRealSpell;
    uint32  DeleteSpell;
    uint32  RequiredSpell;
    uint32  RequiredSkillLine;
    uint32  RequiredSkillLineValue;
    bool    IsProfession;
    uint32  Cost;
    uint32  RequiredLevel;
};

struct Trainer
{
    uint32 SpellCount;
    vector<TrainerSpell> Spells;
    char*   UIMessage;
    uint32 RequiredSkill;
    uint32 RequiredSkillLine;
    uint32 RequiredClass;
    uint32 TrainerType;
    uint32 Can_Train_Gossip_TextId;
    uint32 Cannot_Train_GossipTextId;
};

struct LevelInfo
{
    uint32 HP;
    uint32 Mana;
    uint32 BaseHP;
    uint32 BaseMana;
    uint32 Stat[5];
    uint32 XPToNextLevel;
};

struct ReputationMod
{
    uint32 faction[2];
    int32 value;
    uint32 replimit;
};

struct InstanceReputationMod
{
    uint32 mapid;
    uint32 mob_rep_reward;
    uint32 mob_rep_reward_heroic;
    uint32 mob_rep_limit;
    uint32 mob_rep_limit_heroic;
    uint32 boss_rep_reward;
    uint32 boss_rep_reward_heroic;
    uint32 boss_rep_limit;
    uint32 boss_rep_limit_heroic;
    uint32 faction[2];
};

struct ReputationModifier
{
    uint32 entry;
    vector<ReputationMod> mods;
};

struct InstanceReputationModifier
{
    uint32 mapid;
    vector<InstanceReputationMod> mods;
};

struct NpcMonsterSay
{
    float Chance;
    uint32 Language;
    uint32 Type;
    const char * MonsterName;

    uint32 TextCount;
    const char ** Texts;
};

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

class SERVER_DECL GossipMenu
{
public:
    GossipMenu(uint64 Creature_Guid, uint32 Text_Id);
    void AddItem(GossipMenuItem* GossipItem);
    void AddItem(uint8 Icon, const char* Text, int32 Id = -1, bool Coded = false, uint32 BoxMoney = 0, const char* BoxMessage = "");
    void BuildPacket(WorldPacket& Packet);
    void SendTo(Player* Plr);
    GossipMenuItem GetItem(uint32 Id);
    HEARTHSTONE_INLINE void SetTextID(uint32 TID) { TextId = TID; }
    uint32 GetMenuSize() { return uint32(Menu.size()); }

protected:
    uint32 TextId;
    uint64 CreatureGuid;
    std::vector<GossipMenuItem> Menu;
};

typedef std::map<uint32, std::list<SpellEntry*>* >                  OverrideIdMap;
typedef HM_NAMESPACE::hash_map<uint32, Player* >                    PlayerStorageMap;
typedef std::list<GM_Ticket*>                                       GmTicketList;

#ifndef WIN32
#ifndef TRHAX
// gcc has no default hash for string type,
// so we have to make an explicit hash template here
namespace __gnu_cxx
{
    template<> struct hash<string>
    {
        size_t operator()(string& tbh) const
        {
            // simple crc32 hash for now, we may need to change this later however
            return size_t( crc32( (const unsigned char*)tbh.c_str(), tbh.length() ) );
        }
    }
}
#endif
#endif
// vc++ has the type for a string hash already, so we don't need to do anything special
typedef HM_NAMESPACE::hash_map<string, PlayerInfo*> PlayerNameStringIndexMap;

typedef std::map<uint32, uint32> PetLevelupSpellSet;
typedef std::map<uint32, PetLevelupSpellSet> PetLevelupSpellMap;

struct QuestPOIPoint
{
    int32 x;
    int32 y;
    QuestPOIPoint() : x(0), y(0) {}
    QuestPOIPoint(int32 _x, int32 _y) : x(_x), y(_y) {}
};

/* Some Research:
**
** Id: Quest Id
** ObjectiveIndex: Should be ObjectiveType
****************** -1 for turning in, 0, 1, 2 and 3 are for creatures, 4, 5, 6, 7, 8, and 9 are for items from creatures
****************** 10, 11, 12, 13, 14 and 15 are for items from gameobjects, 16, 17, 18, and 19 are for gameobjects.
** MapId: Easy enough to understand, map id for objective.
** AreaId: This is the WorldMap Area id, not the normal area id.
** MapFloorId: For maps which use multiple floors or areas(Instances), it's used to designate which floor.
** Unk3: No clue
** Unk4: No clue, set it to 1 when I use it.
**
******/
struct QuestPOI
{
    uint32 Id;
    int32 ObjectiveIndex;
    uint32 MapId;
    uint32 AreaId;
    uint32 MapFloorId;
    uint32 Unk3;
    uint32 Unk4;
    std::vector<QuestPOIPoint> points;

    QuestPOI() : Id(0), ObjectiveIndex(0), MapId(0), AreaId(0), MapFloorId(0), Unk3(0), Unk4(1) {}

    QuestPOI(uint32 id, int32 objIndex, uint32 mapId, uint32 areaId, uint32 floorId, uint32 unk3, uint32 unk4) : Id(id), ObjectiveIndex(objIndex), MapId(mapId), AreaId(areaId), MapFloorId(floorId), Unk3(unk3), Unk4(unk4) {}
};

typedef std::vector<QuestPOI> QuestPOIVector;
typedef std::tr1::unordered_map<uint32, QuestPOIVector> QuestPOIMap;

struct RecallLocation
{
    uint32 mapId;
    float x, y, z;
    float orient;
    std::string lowercase_name, RealName;
};

class SERVER_DECL ObjectMgr : public Singleton < ObjectMgr >
{
public:
    ObjectMgr();
    ~ObjectMgr();
    void LoadCreatureWaypoints();

    // other objects

    // Set typedef's
    typedef std::set<AchievementCriteriaEntry*>                                 AchievementCriteriaSet;
    typedef std::set<RecallLocation*>                                           RecallSet;

    // HashMap typedef's
    typedef HM_NAMESPACE::hash_map<uint64, Item* >                              ItemMap;
    typedef HM_NAMESPACE::hash_map<uint32, CorpseData*>                         CorpseCollectorMap;
    typedef HM_NAMESPACE::hash_map<uint32, PlayerInfo*>                         PlayerNameMap;
    typedef HM_NAMESPACE::hash_map<uint32, PlayerCreateInfo*>                   PlayerCreateInfoMap;
    typedef HM_NAMESPACE::hash_map<uint32, SkillLineSpell*>                     SLMap;
    typedef HM_NAMESPACE::hash_map<uint32, std::map<uint32, CreatureItem>* >    VendorMap;
    typedef HM_NAMESPACE::hash_map<uint32, Transporter* >                       TransportMap;
    typedef HM_NAMESPACE::hash_map<uint32, Trainer*>                            TrainerMap;
    typedef HM_NAMESPACE::hash_map<uint32, std::vector<TrainerSpell*> >         TrainerSpellMap;
    typedef HM_NAMESPACE::hash_map<uint32, ReputationModifier*>                 ReputationModMap;
    typedef HM_NAMESPACE::hash_map<uint32, Corpse* >                            CorpseMap;
    typedef HM_NAMESPACE::hash_map<uint32, Group*>                              GroupMap;

    // Map typedef's
    typedef std::map<uint32, LevelInfo*>                                        LevelMap;
    typedef std::map<pair<uint32, uint32>, LevelMap* >                          LevelInfoMap;
    typedef std::map<uint32, std::list<ItemPrototype*>* >                       ItemSetContentMap;
    typedef std::map<uint32, uint32>                                            NpcToGossipTextMap;
    typedef std::map<uint32, uint32>                                            PetSpellCooldownMap;
    typedef std::map<uint32, SpellEntry*>                                       TotemSpellMap;
    typedef std::map<uint32, AchievementCriteriaSet*>                           AchievementCriteriaMap;

    // WMO tables
    typedef std::map<std::pair<uint32, std::pair<uint32, uint32> >, WMOAreaTableEntry*> WMOAreaTableMap;

    // object holders
    GmTicketList        GM_TicketList;
    TotemSpellMap       m_totemSpells;
    OverrideIdMap       mOverrideIdMap;
    QuestPOIMap         mQuestPOIMap;

    Player* GetPlayer(const char* name, bool caseSensitive = true);
    Player* GetPlayer(uint32 guid);

    CorpseMap m_corpses;
    Mutex _corpseslock;
    Mutex m_corpseguidlock;
    Mutex _TransportLock;
    uint32 m_hiCorpseGuid;

    Mutex m_achievementLock;
    AchievementCriteriaMap m_achievementCriteriaMap;

    Mutex _recallLock;
    RecallSet m_recallLocations;

    Item* CreateItem(uint32 entry,Player* owner);
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
        m_groups.insert(make_pair(group->GetID(), group));
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
    PlayerInfo *GetPlayerInfo(uint32 guid );
    PlayerInfo *GetPlayerInfoByName(const char * name);
    void RenamePlayerInfo(PlayerInfo * pn, const char * oldname, const char * newname);
    void DeletePlayerInfo(uint32 guid);
    PlayerCreateInfo* GetPlayerCreateInfo(uint8 race, uint8 class_) const;

    void LoadAchievements();

    // Gameobject Stuff
    std::map<uint32, set<uint32> > GameObjectInvolvedQuestIds;
    void AddInvolvedQuestIds(uint32 entry, set<uint32> set) { GameObjectInvolvedQuestIds.insert(make_pair(entry, set)); };
    void RemoveInvolvedQuestIds(uint32 entry) { std::map<uint32, set<uint32> >::iterator itr = GameObjectInvolvedQuestIds.find(entry); if(itr != GameObjectInvolvedQuestIds.end()) GameObjectInvolvedQuestIds.erase(itr); };
    set<uint32>* GetInvolvedQuestIds(uint32 entry)
    {
        if(GameObjectInvolvedQuestIds.size())
        {
            std::map<uint32, set<uint32> >::iterator itr = GameObjectInvolvedQuestIds.find(entry);
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

    uint32 GetGossipTextForNpc(uint32 ID);

    // Gm Tickets
    void AddGMTicket(GM_Ticket *ticket, bool startup = false);
    void UpdateGMTicket(GM_Ticket *ticket);
    void RemoveGMTicketByPlayer(uint64 playerGuid);
    void RemoveGMTicket(uint64 ticketGuid);
    void DeleteGMTicketPermanently(uint64 ticketGuid);
    void DeleteAllRemovedGMTickets();
    GM_Ticket* GetGMTicket(uint64 ticketGuid);
    GM_Ticket* GetGMTicketByPlayer(uint64 playerGuid);

    SkillLineSpell* GetSpellSkill(uint32 id);

    //Vendors
    std::map<uint32, CreatureItem> *GetVendorList(uint32 entry);
    void SetVendorList(uint32 Entry, std::map<uint32, CreatureItem>* list_);

    //Totem
    SpellEntry* GetTotemSpell(uint32 spellId);

    std::list<ItemPrototype*>* GetListForItemSet(uint32 setid);

    Pet* CreatePet();
    uint32 m_hiPetGuid;
    uint32 m_hiArenaTeamId;
    uint32 GenerateArenaTeamId()
    {
        uint32 ret;
        m_arenaTeamLock.Acquire();
        ret = ++m_hiArenaTeamId;
        m_arenaTeamLock.Release();
        return ret;
    }

    Mutex m_petlock;

    Player* CreatePlayer();
    Mutex m_playerguidlock;
    PlayerStorageMap _players;
    RWLock _playerslock;
    uint32 m_hiPlayerGuid;

    void AddPlayer(Player* p);//add it to global storage
    void RemovePlayer(Player* p);

    QuestPOIVector const* GetQuestPOIVector(uint32 questId)
    {
        QuestPOIMap::const_iterator itr = mQuestPOIMap.find(questId);
        if (itr != mQuestPOIMap.end())
            return &itr->second;
        return NULL;
    }

    // Serialization
    void LoadPlayersInfo();
    void LoadPlayerCreateInfo();
    Corpse* LoadCorpse(uint32 guid);
    void LoadCorpses(MapMgr* mgr);
    void LoadGMTickets();
    void SaveGMTicket(GM_Ticket* ticket, QueryBuffer * buf);
    void LoadAuctions();
    void LoadAuctionItems();
    void LoadSpellSkills();
    void LoadVendors();
    void ReloadVendors();
    void LoadTotemSpells();
    void LoadAIThreatToSpellId();
    void LoadSpellFixes();
    void LoadReputationModifierTable(const char * tablename, ReputationModMap * dmap);
    void LoadReputationModifiers();
    void LoadQuestPOI();
    void LoadRecallPoints();
    ReputationModifier * GetReputationModifier(uint32 entry_id, uint32 faction_id);

    void SetHighestGuids();
    void ListGuidAmounts();
    uint32 GenerateLowGuid(uint32 guidhigh);
    uint32 GenerateMailID();
    uint64 GenerateTicketID();

    uint64 GenerateEquipmentSetGuid();

    void LoadTransporters();
    void ProcessGameobjectQuests();
    void AddTransport(Transporter* pTransporter);

    void LoadTrainers();
    Trainer* GetTrainer(uint32 Entry);

    void LoadExtraItemStuff();
    void LoadExtraGameObjectStuff();
    void LoadExtraCreatureProtoStuff();
    void LoadProfessionDiscoveries();
    void LoadRandomItemCreation();
    void LoadRandomCardCreation();
    void LoadScrollCreation();
    void CreateGossipMenuForPlayer(GossipMenu** Location, uint64 Guid, uint32 TextID, Player* Plr);

    QueryResult* SQLCheckExists(const char* tablename, const char* columnname, uint64 columnvalue);
    LevelInfo * GetLevelInfo(uint32 Race, uint32 Class, uint32 Level);
    void GenerateLevelUpInfo();
    uint32 GetPetSpellCooldown(uint32 SpellId);
    void LoadPetSpellCooldowns();
    WayPointMap * GetWayPointMap(uint32 spawnid);
    void LoadSpellOverride();
    void LoadPetLevelupSpellMap();
    PetLevelupSpellSet const* GetPetLevelupSpellList(uint32 petFamily) const;

    uint32 GenerateCreatureSpawnID()
    {
        m_CreatureSpawnIdMutex.Acquire();
        uint32 r = ++m_hiCreatureSpawnId;
        m_CreatureSpawnIdMutex.Release();
        return r;
    }

    uint32 GenerateGameObjectSpawnID()
    {
        m_GOSpawnIdMutex.Acquire();
        uint32 r = ++m_hiGameObjectSpawnId;
        m_GOSpawnIdMutex.Release();
        return r;
    }

    Transporter* GetTransporter(uint32 guid);
    Transporter* GetTransporterByEntry(uint32 entry);

    ArenaTeam * GetArenaTeamByName(string & name, uint32 Type);
    ArenaTeam * GetArenaTeamById(uint32 id);
    void UpdateArenaTeamRankings();
    void UpdateArenaTeamWeekly();
    void ResetDailies();
    void LoadArenaTeams();
    HM_NAMESPACE::hash_map<uint32, ArenaTeam*> m_arenaTeamMap[3];
    HM_NAMESPACE::hash_map<uint32, ArenaTeam*> m_arenaTeamPlayerMap[3];
    HM_NAMESPACE::hash_map<uint32, ArenaTeam*> m_arenaTeams;
    void RemoveArenaTeam(ArenaTeam * team);
    void AddArenaTeam(ArenaTeam * team);
    Mutex m_arenaTeamLock;

    typedef HM_NAMESPACE::hash_map<uint32, NpcMonsterSay*> MonsterSayMap;
    MonsterSayMap mMonsterSays[NUM_MONSTER_SAY_EVENTS];

    void HandleMonsterSayEvent(Creature* pCreature, MONSTER_SAY_EVENTS Event);
    bool HasMonsterSay(uint32 Entry, MONSTER_SAY_EVENTS Event);
    void LoadMonsterSay();

    bool HandleInstanceReputationModifiers(Player* pPlayer, Unit* pVictim);
    void LoadInstanceReputationModifiers();

    HEARTHSTONE_INLINE bool IsSpellDisabled(uint32 spellid)
    {
        if(m_disabled_spells.find(spellid) != m_disabled_spells.end())
            return true;
        return false;
    }

    void LoadDisabledSpells();
    void ReloadDisabledSpells();
    std::set<ProfessionDiscovery*> ProfessionDiscoveryTable;
    map<uint32, uint32> ItemsInSets;

    void HashWMOAreaTables();
    WMOAreaTableEntry* GetWMOAreaTable(int32 adtid, int32 rootid, int32 groupid);

    RecallLocation *GetRecallLocByName(std::string name);
    bool AddRecallLocation(std::string name, uint32 mapId, float x, float y, float z, float o);
    bool FillRecallNames(std::string match, std::set<RecallLocation*> &output);
    bool DeleteRecallLocation(std::string name);

protected:
    WMOAreaTableMap WMOAreaTables;
    RWLock playernamelock;
    uint32 m_mailid;
    uint64 m_ticketid;
    uint64 m_equipmentSetGuid;
    // highest GUIDs, used for creating new objects
    Mutex m_guidGenMutex;
    union
    {
        uint32 m_hiItemGuid;
        uint32 m_hiContainerGuid;
    };
    uint32 m_hiGroupId;

    ReputationModMap m_reputation_faction;
    ReputationModMap m_reputation_creature;
    HM_NAMESPACE::hash_map<uint32, InstanceReputationModifier*> m_reputation_instance;

    set<uint32> m_disabled_spells;

    uint64 TransportersCount;
    HM_NAMESPACE::hash_map<uint32,PlayerInfo*> m_playersinfo;
    PlayerNameStringIndexMap m_playersInfoByName;

    HM_NAMESPACE::hash_map<uint32,WayPointMap*> m_waypoints;//stored by spawnid
    uint32 m_hiCreatureSpawnId;

    Mutex m_CreatureSpawnIdMutex;
    Mutex m_GOSpawnIdMutex;

    uint32 m_hiGameObjectSpawnId;

    ///// Object Tables ////
    // These tables are modified as creatures are created and destroyed in the world

    // Group List
    RWLock m_groupLock;
    GroupMap m_groups;

    // Map of all starting infos needed for player creation
    PlayerCreateInfoMap mPlayerCreateInfo;

    // Map of all vendor goods
    VendorMap mVendors;

    // Maps for Gossip stuff
    NpcToGossipTextMap  mNpcToGossipText;

    SLMap               mSpellSkills;

    //Corpse Collector
    CorpseCollectorMap mCorpseCollector;

    TransportMap mTransports;

    ItemSetContentMap mItemSets;

    TrainerMap mTrainers;
    LevelInfoMap mLevelInfo;
    PetSpellCooldownMap mPetSpellCooldowns;
    PetLevelupSpellMap  mPetLevelupSpellMap;
};


#define objmgr ObjectMgr::getSingleton()

//void SetProgressBar(int, int, const char*);
//void ClearProgressBar();
