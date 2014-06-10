/***
 * Demonstrike Core
 */

#pragma once

#define IS_INSTANCE(a) (a > 1 && a != 530 && a != 571 && a != 609)
#define IS_MAIN_MAP(a) (a == 0 || a == 1 || a == 530 || a == 571 || a == 609)

class Object;
class WorldPacket;
class WorldSession;
class Unit;
class Creature;
class GameObject;
class DynamicObject;
class Player;
class MapMgr;
class Battleground;
class Lacrimi;

enum Rates
{
    RATE_XP = 0,
    RATE_QUESTXP,
    RATE_RESTXP,
    RATE_DROP0, // separate rates for each quality level
    RATE_DROP1,
    RATE_DROP2,
    RATE_DROP3,
    RATE_DROP4,
    RATE_DROP5,
    RATE_DROP6,
    RATE_MONEY,
    RATE_QUESTMONEY,
    RATE_HONOR,
    RATE_SKILLRATE,
    RATE_SKILLCHANCE,
    RATE_QUESTREPUTATION,
    RATE_KILLREPUTATION,
    MAX_RATES
};

enum EventIdFlags
{
    EVENTID_FLAG_NONE = 0,
    EVENTID_FLAG_PHASE = 1,
    EVENTID_FLAG_MODELID = 2,
    EVENTID_FLAG_EQUIP = 4, //this obviously cannot be used for gameobjects
    EVENTID_FLAG_SPAWN = 8
};

enum EnviromentalDamage
{
    DAMAGE_EXHAUSTED = 0,
    DAMAGE_DROWNING = 1,
    DAMAGE_FALL = 2,
    DAMAGE_LAVA = 3,
    DAMAGE_SLIME = 4,
    DAMAGE_FIRE = 5
};

enum CharCreateErrors
{
    SUCCESS,
    FAILURE,
    CANCELLED,
    DISCONNECT_FROM_SERVER,
    FAILED_TO_CONNECT,
    CONNECTED,
    WRONG_CLIENT_VERSION,
    CONNECTING_TO_SERVER,
    NEGOTIATING_SECURITY,
    NEGOTIATING_SECURITY_COMPLETE,
    NEGOTIATING_SECURITY_FAILED,
    AUTHENTICATING,
    AUTHENTICATION_SUCCESSFUL,
    AUTHENTICATION_FAILED,
    LOGIN_UNAVAIBLE,
    SERVER_IS_NOT_VALID,
    SYSTEM_UNAVAIBLE,
    SYSTEM_ERROR,
    BILLING_SYSTEM_ERROR,
    ACCOUNT_BILLING_EXPIRED,
    WRONG_CLIENT_VERSION_2,
    UNKNOWN_ACCOUNT,
    INCORRECT_PASSWORD,
    SESSION_EXPIRED,
    SERVER_SHUTTING_DOWN,
    ALREADY_LOGGED_IN,
    INVALID_LOGIN_SERVER,
    POSITION_IN_QUEUE_0,
    THIS_ACCOUNT_HAS_BEEN_BANNED,
    THIS_CHARACTER_STILL_LOGGED_ON,
    YOUR_WOW_SUBSCRIPTION_IS_EXPIRED,
    THIS_SESSION_HAS_TIMED_OUT,
    THIS_ACCOUNT_TEMP_SUSPENDED,
    ACCOUNT_BLOCKED_BY_PARENTAL_CONTROL,
    RETRIEVING_REALMLIST,
    REALMLIST_RETRIEVED,
    UNABLE_TO_CONNECT_REALMLIST_SERVER,
    INVALID_REALMLIST,
    GAME_SERVER_DOWN,
    CREATING_ACCOUNT,
    ACCOUNT_CREATED,
    ACCOUNT_CREATION_FAIL,
    RETRIEVE_CHAR_LIST,
    CHARLIST_RETRIEVED,
    CHARLIST_ERROR,
    CREATING_CHARACTER,
    CHARACTER_CREATED,
    ERROR_CREATING_CHARACTER,
    CHARACTER_CREATION_FAIL,
    NAME_IS_IN_USE,
    CREATION_OF_RACE_DISABLED,
    ALL_CHARS_ON_PVP_REALM_MUST_AT_SAME_SIDE,
    ALREADY_HAVE_MAXIMUM_CHARACTERS,
    ALREADY_HAVE_MAXIMUM_CHARACTERS_2,
    SERVER_IS_CURRENTLY_QUEUED,
    ONLY_PLAYERS_WHO_HAVE_CHARACTERS_ON_THIS_REALM,
    NEED_EXPANSION_ACCOUNT,
    DELETING_CHARACTER,
    CHARACTER_DELETED,
    CHARACTER_DELETION_FAILED,
    ENTERING_WOW,
    LOGIN_SUCCESFUL,
    WORLD_SERVER_DOWN,
    A_CHARACTER_WITH_THAT_NAME_EXISTS,
    NO_INSTANCE_SERVER_AVAIBLE,
    LOGIN_FAILED,
    LOGIN_FOR_THAT_RACE_DISABLED,
    LOGIN_FOR_THAT_RACE_CLASS_DISABLED,//check
    ENTER_NAME_FOR_CHARACTER,
    NAME_AT_LEAST_TWO_CHARACTER,
    NAME_AT_MOST_12_CHARACTER,
    NAME_CAN_CONTAIN_ONLY_CHAR,
    NAME_CONTAIN_ONLY_ONE_LANG,
    NAME_CONTAIN_PROFANTY,
    NAME_IS_RESERVED,
    YOU_CANNOT_USE_APHOS,
    YOU_CAN_ONLY_HAVE_ONE_APHOS,
    YOU_CANNOT_USE_SAME_LETTER_3_TIMES,
    NO_SPACE_BEFORE_NAME,
    BLANK,
    INVALID_CHARACTER_NAME,
    BLANK_1
    //All further codes give the number in dec.
};

// ServerMessages.dbc
enum ServerMessageType
{
    SERVER_MSG_SHUTDOWN_TIME        = 1,
    SERVER_MSG_RESTART_TIME         = 2,
    SERVER_MSG_STRING               = 3,
    SERVER_MSG_SHUTDOWN_CANCELLED   = 4,
    SERVER_MSG_RESTART_CANCELLED    = 5
};

enum ServerShutdownType
{
    SERVER_SHUTDOWN_TYPE_SHUTDOWN   = 1,
    SERVER_SHUTDOWN_TYPE_RESTART    = 2,
};

enum WorldMapInfoFlag
{
    WMI_INSTANCE_ENABLED        = 0x1,
    WMI_INSTANCE_WELCOME        = 0x2,
    WMI_INSTANCE_MULTIMODE      = 0x4,
    WMI_INSTANCE_XPACK_01       = 0x8, //The Burning Crusade expansion
    WMI_INSTANCE_XPACK_02       = 0x10 //Wrath of the Lich King expansion
};

enum AccountFlags
{
    ACCOUNT_FLAG_VIP            = 0x01,
    ACCOUNT_FLAG_SPONSOR        = 0x02,
    ACCOUNT_FLAG_SUPER_SPONSOR  = 0x04,
    ACCOUNT_FLAG_XPACK_01       = 0x08,
    ACCOUNT_FLAG_XPACK_02       = 0x10,
    ACCOUNT_FLAG_XPACK_03       = 0x20
};

enum HolidayMasks
{
    HOLIDAY_DARKMOON_FAIRE      = 0x1,
    HOLIDAY_BREWFEST            = 0x2,
    HOLIDAY_LOVE_IS_IN_AIR      = 0x4,
    HOLIDAY_MIDSUMMER           = 0x8,
    HOLIDAY_CHILD_WEEK          = 0x10,
    HOLIDAY_WINTER_VIEL         = 0x20,
    HOLIDAY_NOBLEGARDEN         = 0x40,
    HOLIDAY_HALLOWS_END         = 0x80,
    HOLIDAY_HARVEST_FEST        = 0x100,
    HOLIDAY_LUNAR_FEST          = 0x200,
    HOLIDAY_DAY_OF_DEAD         = 0x400,
    HOLIDAY_PILGRIM_BOUNTY      = 0x800,

    HOLIDAY_ALL                 = HOLIDAY_DARKMOON_FAIRE | HOLIDAY_BREWFEST |
    HOLIDAY_LOVE_IS_IN_AIR | HOLIDAY_MIDSUMMER | HOLIDAY_CHILD_WEEK |
    HOLIDAY_WINTER_VIEL | HOLIDAY_NOBLEGARDEN | HOLIDAY_HALLOWS_END |
    HOLIDAY_HARVEST_FEST | HOLIDAY_LUNAR_FEST | HOLIDAY_DAY_OF_DEAD |
    HOLIDAY_PILGRIM_BOUNTY
};

#pragma pack(PRAGMA_PACK)
struct MapInfo
{
    uint32 mapid;
    char * name;
    bool load;
    uint32 type;
    uint32 playerlimit;
    uint32 minlevel;
    uint32 LinkedAreaTrigger;
    float repopx;
    float repopy;
    float repopz;
    int32 repopmapid;
    uint32 flags;
    uint32 cooldown;
    uint32 required_quest;
    uint32 required_item;
    uint32 heroic_key[2];
    uint32 phasehorde;
    uint32 phasealliance;

    bool HasFlag(uint32 flag)
    {
        return (flags & flag) != 0;
    }
};

#pragma pack(PRAGMA_POP)

struct insert_playeritem
{
    uint32 ownerguid;
    uint32 entry;
    uint32 wrapped_item_id;
    uint32 wrapped_creator;
    uint32 creator;
    uint32 count;
    uint32 charges;
    uint32 flags;
    uint32 randomprop;
    uint32 randomsuffix;
    uint32 itemtext;
    uint32 durability;
    int32 containerslot;
    int32 slot;
    string enchantments;
};

struct insert_playerskill
{
    uint32 player_guid;
    uint32 skill_id;
    uint32 type;
    uint32 currentlvl;
    uint32 maxlvl;
};

struct insert_playerquest
{
    uint32 player_guid;
    uint32 quest_id;
    uint32 slot;
    uint32 time_left;
    uint32 explored_area1;
    uint32 explored_area2;
    uint32 explored_area3;
    uint32 explored_area4;
    uint32 mob_kill1;
    uint32 mob_kill2;
    uint32 mob_kill3;
    uint32 mob_kill4;
    uint32 slain;
};

struct insert_playerglyph
{
    uint32 player_guid;
    uint32 spec;
    uint32 glyph1;
    uint32 glyph2;
    uint32 glyph3;
    uint32 glyph4;
    uint32 glyph5;
    uint32 glyph6;
};

struct insert_playertalent
{
    uint32 player_guid;
    uint32 spec;
    uint32 tid;
    uint32 rank;
};

struct insert_playerspell
{
    uint32 player_guid;
    uint32 spellid;
};

class BasicTaskExecutor : public ThreadContext
{
    CallbackBase * cb;
    uint32 priority;
public:
    BasicTaskExecutor(CallbackBase * Callback, uint32 Priority) : cb(Callback), priority(Priority) {}
    ~BasicTaskExecutor() { delete cb; }
    bool run();
};

class Task
{
    CallbackBase * _cb;
public:
    Task(CallbackBase * cb) : _cb(cb), completed(false), in_progress(false) {}
    ~Task() { delete _cb; }
    bool completed;
    bool in_progress;
    void execute();
};

struct InsertQueueLoader : public Singleton<InsertQueueLoader>
{
private:
    uint32 m_UpdateTimer;

public:
    InsertQueueLoader();
    ~InsertQueueLoader();
    void Update(uint32 timeDiff);
};

class TaskList
{
    set<Task*> tasks;
    Mutex queueLock;
public:
    Task * GetTask();
    void AddTask(Task* task);
    void RemoveTask(Task * task)
    {
        queueLock.Acquire();
        tasks.erase(task);
        queueLock.Release();
    }

    void spawn();
    void kill();

    void wait();
    void waitForThreadsToExit();
    uint32 thread_count;
    bool running;

    Mutex tcMutex;
    void incrementThreadCount()
    {
        tcMutex.Acquire();
        ++thread_count;
        tcMutex.Release();
    }

    void decrementThreadCount()
    {
        tcMutex.Acquire();
        --thread_count;
        tcMutex.Release();
    }
};

enum BasicTaskExecutorPriorities
{
    BTE_PRIORITY_LOW        = 0,
    BTE_PRIORITY_MED        = 1,
    BTW_PRIORITY_HIGH       = 2,
};

class TaskExecutor : public ThreadContext
{
    TaskList * starter;
public:
    TaskExecutor(TaskList * l) : ThreadContext(), starter(l) { l->incrementThreadCount(); }
    ~TaskExecutor() { starter->decrementThreadCount(); }

    bool run();
};

class WorldSocket;

// Slow for remove in middle, oh well, wont get done much.
typedef std::list<WorldSocket*> QueueSet;
typedef set<WorldSession*> SessionSet;

#define MAXIMUM_CEXPANSION_LEVEL 85 // Current expansion's max level
#define MAXIMUM_ATTAINABLE_LEVEL 100 // Crow: Lets use 100, since its the highest the DBCs will support

class SERVER_DECL World : public Singleton<World>, public EventableObject
{
public:
    World();
    ~World();

    void Destruct();

    uint32 GetMaxLevel(Player* plr);
    uint32 GetMaxLevelStatCalc();

    /** Reloads the config and sets all of the setting variables
     */
    void Rehash(bool load);

    void CleanupCheaters();
    WorldSession* FindSession(uint32 id);
    WorldSession* FindSessionByName(const char *);
    void AddSession(WorldSession *s);
    void RemoveSession(uint32 id);

    void AddGlobalSession(WorldSession *GlobalSession);
    void RemoveGlobalSession(WorldSession *GlobalSession);
    void DeleteGlobalSession(WorldSession *GlobalSession);

    HEARTHSTONE_INLINE size_t GetSessionCount() const { return m_sessions.size(); }

    HEARTHSTONE_INLINE size_t GetQueueCount() { return mQueuedSessions.size(); }
    void GetStats(uint32 * GMCount, float * AverageLatency);

    HEARTHSTONE_INLINE uint32 GetPlayerLimit() const { return m_playerLimit; }
    void SetPlayerLimit(uint32 limit) { m_playerLimit = limit; }

    HEARTHSTONE_INLINE std::string getGmClientChannel() { return GmClientChannel; }

    // MOTD line 1
    void SetMotd(const char *motd) { m_motd = motd; }
    HEARTHSTONE_INLINE const char* GetMotd() const { return m_motd.c_str(); }
    HEARTHSTONE_INLINE time_t GetGameTime() const { return m_gameTime; }

    bool SetInitialWorldSettings();

    void SendWorldText(const char *text, WorldSession *self = 0);
    void SendWorldWideScreenText(const char *text, WorldSession *self = 0);
    void SendGlobalMessage(WorldPacket *packet, WorldSession *self = 0);
    void SendZoneMessage(WorldPacket *packet, uint32 zoneid, WorldSession *self = 0);
    void SendInstanceMessage(WorldPacket *packet, uint32 instanceid, WorldSession *self = 0);
    void SendFactionMessage(WorldPacket *packet, uint8 teamId);
    void SendGMWorldText(const char* text, bool admin = false);
    void SendAdministratorMessage(WorldPacket *packet);
    void SendGamemasterMessage(WorldPacket *packet);
    void SendMessageToGMs(WorldSession *self, const char * text, ...);

    HEARTHSTONE_INLINE void SetStartTime(uint32 val) { m_StartTime = val; }
    HEARTHSTONE_INLINE uint32 GetUptime(void) { return (uint32)UNIXTIME - m_StartTime; }
    HEARTHSTONE_INLINE uint32 GetStartTime(void) { return m_StartTime; }
    std::string GetUptimeString();

    // update the world server every frame
    void Update(time_t diff);
    void CheckForExpiredInstances();

    void UpdateSessions(uint32 diff);

    HEARTHSTONE_INLINE void setRate(int index,float value)
    {
        regen_values[index]=value;
    }

    HEARTHSTONE_INLINE float getRate(int index)
    {
        return regen_values[index];
    }

    // map text emote to spell prices
    typedef std::map< uint32, uint32> SpellPricesMap;
    SpellPricesMap mPrices;
    uint32 MaxLevelCalc;

    struct NameGenData
    {
        string name;
        uint32 type;
    };
    vector<NameGenData> _namegendata[3];
    void LoadNameGenData();
    std::string GenerateName(uint32 type = 0);

    std::map<uint32,uint32> TeachingSpellMap;
    uint32 GetTeachingSpell(uint32 NormalSpellId)
    {
        map<uint32,uint32>::iterator i = TeachingSpellMap.find(NormalSpellId);
        if(i!=TeachingSpellMap.end())
            return i->second;
        return 0;
    }

    uint32 AddQueuedSocket(WorldSocket* Socket);
    void RemoveQueuedSocket(WorldSocket* Socket);
    uint32 GetQueuePos(WorldSocket* Socket);
    void UpdateQueuedSessions(uint32 diff);

    // Auth seeds
    BigNumber authSeed1, authSeed2;

    Mutex queueMutex;
    string LuaScriptPath, GameMonkeyScriptPath;
    string DBCPath, MapPath, vMapPath, MMapPath;
    bool AHEnabled, DisableBufferSaving;
    bool Collision, PathFinding, CalculatedHeightChecks;
    bool LogCheaters, LogCommands, LogPlayers, bLogChat;
    float AreaUpdateDistance, NetworkStressIn, NetworkStressOut;
    uint32 ServerPreloading, mInWorldPlayerCount, mAcceptedConnections;
    uint32 mQueueUpdateInterval, trade_world_chat, m_deathKnightReqLevel;
    bool cross_faction_world, m_deathKnightOnePerAccount, EnableFatigue, NumericCommandGroups;

    void SaveAllPlayers();

    bool CheckSanctuary(uint32 MapId, uint32 zone, uint32 area)
    {
        if(FunServerMall != -1 && (zone == FunServerMall || area == FunServerMall))
            return true;
        if(Sanctuaries.find(zone) != Sanctuaries.end()
            || Sanctuaries.find(area) != Sanctuaries.end())
            return true;
        return SanctuaryMaps.find(MapId) != SanctuaryMaps.end();
    }

    std::set<uint32> Sanctuaries;
    void SetSanctuaryArea(uint32 areaid) { Sanctuaries.insert(areaid); }
    void RemoveSanctuaryArea(uint32 areaid) { Sanctuaries.erase(areaid); }
    bool IsSanctuaryArea(uint32 areaid) { return Sanctuaries.find(areaid) != Sanctuaries.end(); }

    struct RestedAreaInfo { RestedAreaInfo(int8 team){ReqTeam = team;}; int8 ReqTeam; };
    std::map<uint32, RestedAreaInfo*> RestedAreas;
    void SetRestedArea(uint32 areaid, int8 reqteam = -1) { if(RestedAreas[areaid] == NULL) RestedAreas[areaid] = new RestedAreaInfo(reqteam); else RestedAreas[areaid]->ReqTeam = reqteam; }
    void RemoveRestedArea(uint32 areaid)
    {
        RestedAreaInfo* info = RestedAreas[areaid];
        if(info != NULL)
        {
            RestedAreas.erase(areaid);
            delete info;
            info = NULL;
        }
    }
    RestedAreaInfo* GetRestedAreaInfo(uint32 areaid) { return RestedAreas[areaid]; }

    std::set<uint32> SanctuaryMaps;
    void SetSanctuaryMap(uint32 mapid) { SanctuaryMaps.insert(mapid); }
    void RemoveSanctuaryMap(uint32 mapid) { SanctuaryMaps.erase(mapid); }
    bool IsSanctuaryMap(uint32 mapid) { return SanctuaryMaps.find(mapid) != SanctuaryMaps.end(); }

    std::map<uint32, RestedAreaInfo*> RestedMapIds;
    void SetRestedMap(uint32 mapid, int8 reqteam = -1) { if(RestedMapIds[mapid] == NULL) RestedMapIds[mapid] = new RestedAreaInfo(reqteam); else RestedMapIds[mapid]->ReqTeam = reqteam; }
    void RemoveRestedMap(uint32 mapid)
    {
        RestedAreaInfo* info = RestedMapIds[mapid];
        if(info != NULL)
        {
            RestedMapIds.erase(mapid);
            delete info;
            info = NULL;
        }
    }
    RestedAreaInfo* GetRestedMapInfo(uint32 mapid) { return RestedMapIds[mapid]; }

    uint32 HordePlayers, AlliancePlayers, PeakSessionCount;
    bool IsPvPRealm, SendMovieOnJoin;
    int32 FunServerMall, LogoutDelay;
    uint32 expansionUpdateTime;
    SessionSet gmList;
    RWLock gmList_lock;

    void DeleteObject(Object* obj);

    bool gm_force_robes;
    uint8 StartLevel;
    uint32 StartGold;

    uint32 flood_lines, flood_seconds, flood_message_time, flood_mute_after_flood, flood_caps_min_len;
    float flood_caps_pct;
    bool flood_message;

    bool no_antihack_on_gm, antihack_teleport, antihack_speed, antihack_flight, antihack_cheatengine;

    // Force start/end Wintergrasp
    bool ForceStart, ForceEnd;
    //Enable/Disable specific battlegrounds/arenas
    bool wg_enabled, av_enabled, ab_enabled, wsg_enabled, eots_enabled, sota_enabled, ioc_enabled;
    uint32 av_minplrs, ab_minplrs, wsg_minplrs, eots_minplrs, sota_minplrs, ioc_minplrs;

    // Level Caps
    uint32 LevelCap_Custom_All;
    bool Start_With_All_Taximasks;

    // could add configs for every expansion..

    void CharacterEnumProc(QueryResultVector& results, uint32 AccountId);

    void PollCharacterInsertQueue(DatabaseConnection * con);
    void PollMailboxInsertQueue(DatabaseConnection * con);
    void DisconnectUsersWithAccount(const char * account, WorldSession * session);
    void DisconnectUsersWithIP(const char * ip, WorldSession * session);
    void DisconnectUsersWithPlayerName(const char * plr, WorldSession * session);

    void LogoutPlayers();

    void BackupDB();

    void LogGM(WorldSession* session, string message, ...);
    void LogCheater(WorldSession* session, string message, ...);
    void LogPlayer(WorldSession* session, string message, ...);
    void LogChat(WorldSession* session, string message, ...);

protected:
    // update Stuff, FIXME: use diff
    time_t _UpdateGameTime()
    {
        // Update Server time
        time_t thisTime = UNIXTIME;
        m_gameTime += thisTime - m_lastTick; //in seconds

        if(m_gameTime >= 86400)         // One day has passed
            m_gameTime -= 86400;

        m_lastTick = thisTime;

        return m_gameTime;
    }
    void FillSpellReplacementsTable();

private:
    //! Timers
    typedef HM_NAMESPACE::hash_map<uint32, WorldSession*> SessionMap;
    SessionMap m_sessions;
    RWLock m_sessionlock;

    typedef HM_NAMESPACE::hash_map<uint32, AreaTrigger*> AreaTriggerMap;
    AreaTriggerMap m_AreaTrigger;

protected:
    Mutex SessionsMutex;//FOR GLOBAL !
    SessionSet GlobalSessions;

    float regen_values[MAX_RATES];

    uint32 m_playerLimit;
    std::string m_motd;

    time_t m_gameTime, m_lastTick;
    uint32 m_StartTime, m_queueUpdateTimer;

    QueueSet mQueuedSessions;

public:
    ThreadContext* LacrimiThread;
    Lacrimi* LacrimiPtr;

    bool GuildsLoading;

    std::string GmClientChannel;
    bool m_reqGmForCommands;
    list<SpellEntry*> dummyspells;
    bool m_limitedNames;
    bool m_useAccountData;
    bool m_blockgmachievements;

    float m_speedHackThreshold;
    float m_speedHackLatencyMultiplier;
    uint32 m_CEThreshold;

    // shutdown
    uint32 m_shutdownTime, m_shutdownType, m_shutdownLastTime;

    void QueueShutdown(uint32 delay, uint32 type);
    void CancelShutdown();
    void UpdateShutdownStatus();

    bool VerifyName(const char * name, size_t nlen)
    {
        const char * p;
        size_t i;

        static const char * bannedCharacters = "\t\v\b\f\a\n\r\\\"\'\?<>[](){}_=+-|/!@#$%^&*~`.,0123456789\0";
        for(i = 0; i < nlen; i++)
        {
            p = bannedCharacters;
            while(*p != 0 && name[i] != *p && name[i] != 0)
                ++p;

            if(*p != 0)
                return false;
        }

        return true;
    }
    bool QueryLog;

public:
    float GetCPUUsage(bool external = false);
    float GetRAMUsage(bool external = false);

#ifdef WIN32

private:
    bool m_bFirstTime;
    __int64 m_lnOldValue;
    LARGE_INTEGER m_OldPerfTime100nSec;
    uint32 number_of_cpus;
    uint32 m_current_holiday_mask;

#endif // WIN32

public: // Events! :D
    void SetAnniversary(uint32 anniversarynumber);
    uint32 AnniversaryAchievement;
    bool RealAchievement;

    void SpawnHallowsEnd(bool apply) 
    { 
        HallowsEnd = apply;
        if(apply)
            AddHolidayMask(HOLIDAY_HALLOWS_END);
        else
            RemoveHolidayMask(HOLIDAY_HALLOWS_END);
        OnHolidayChange(16);
    };

    bool HallowsEnd;

    void SpawnWintersVeil(bool apply) 
    { 
        WintersVeil = apply; 
        if(apply)
            AddHolidayMask(HOLIDAY_WINTER_VIEL);
        else
            RemoveHolidayMask(HOLIDAY_WINTER_VIEL);
        OnHolidayChange(14);
    };

    bool WintersVeil;

    void OnHolidayChange(uint32 IgnoreHolidayId);
    uint32 GetCurrentHolidayMask() { return m_current_holiday_mask; };
    void AddHolidayMask(uint32 mask) 
    {
        if(GetCurrentHolidayMask() & mask)
            return;
        m_current_holiday_mask &= mask;
        ApplyHolidayConfigMaskOverride();
    }

    void RemoveHolidayMask(uint32 mask) 
    {
        if((GetCurrentHolidayMask() & mask) == 0)
            return;
        m_current_holiday_mask &= ~mask;
        ApplyHolidayConfigMaskOverride();
    }

    void ApplyHolidayConfigMaskOverride()
    {
        uint32 config_holiday_mask = mainIni->ReadInteger("Server", "HolidayMaskOverride", 4095);
        if(config_holiday_mask > 0)
            m_current_holiday_mask = config_holiday_mask;
    }

    uint32 ConverHolidayIdToMask(uint32 id)
    {
        switch(id)
        {
        case 1:
            return HOLIDAY_DARKMOON_FAIRE;
        case 7:
        case 19:
            return HOLIDAY_BREWFEST;
        case 9:
            return HOLIDAY_LOVE_IS_IN_AIR;
        case 11:
            return HOLIDAY_MIDSUMMER;
        case 13:
            return HOLIDAY_CHILD_WEEK;
        case 14:
            return HOLIDAY_WINTER_VIEL;
        case 15:
            return HOLIDAY_NOBLEGARDEN;
        case 16:
            return HOLIDAY_LUNAR_FEST;
        case 17:
            return HOLIDAY_HARVEST_FEST;
        case 81:
            return HOLIDAY_DAY_OF_DEAD;
        case 101:
            return HOLIDAY_PILGRIM_BOUNTY;
        }
        return 0;
    };

};

#define sIQL InsertQueueLoader::getSingleton()
#define sWorld World::getSingleton()
