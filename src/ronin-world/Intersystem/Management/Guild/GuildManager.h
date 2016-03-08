/***
 * Demonstrike Core
 */

#pragma once

#pragma pack(PRAGMA_PACK)

// Guild Challenge
#define GUILD_CHALLENGES_TYPES 4

static const uint32 GuildChallengeGoldReward[GUILD_CHALLENGES_TYPES]         = { 0, 250,    1000,    500 };
static const uint32 GuildChallengeMaxLevelGoldReward[GUILD_CHALLENGES_TYPES] = { 0, 125,    500,     250 };
static const uint32 GuildChallengeXPReward[GUILD_CHALLENGES_TYPES]           = { 0, 300000, 3000000, 1500000 };
static const uint32 GuildChallengesPerWeek[GUILD_CHALLENGES_TYPES]           = { 0, 7,      1,       3 };
static const uint64 GuildXPPerLevel[24] = { 1658, 1824, 1990, 2155, 2322, 2488, 2653, 2819, 2985, 3151, 3317, 3482, 3649, 3814, 3980, 4145, 4312, 4478, 4643, 4809, 4975, 5141, 5306, 5473 };
static const uint32 GuildPerks[24] = { 78631, 78633, 78634, 83940, 78632, 83942, 83944, 83943, 83945, 83958, 78635, 83959, 83949, 83950, 83941, 83951, 83953, 83960, 83963, 83967, 83961, 83966, 83964, 83968 };

struct GuildInfo
{
    uint32 m_GuildStatus;
    Mutex m_GuildLock;

    /** Internal variables
     */
    uint32 m_guildId;
    uint32 m_guildLevel;
    uint32 m_emblemStyle;
    uint32 m_emblemColor;
    uint32 m_borderStyle;
    uint32 m_borderColor;
    uint32 m_backgroundColor;

    WoWGuid m_guildLeader;
    uint32 m_creationTimeStamp;
    uint64 m_bankBalance;
    bool m_commandLogging;

    std::string m_guildName;
    std::string m_guildInfo;
    std::string m_motd;

    Mutex guildXPLock;
    uint64 m_xpGainedToday;
    uint64 m_guildExperience;
};

struct GuildRankTabPermissions
{
    uint32 iFlags;
    int32 iStacksPerDay;
};

struct GuildRank
{
    GuildRank(uint32 RankId, uint32 RankRights, const char* RankName, bool FullPermissions)
    {
        iId = RankId;
        iRights = RankRights;
        DailyXPCap = 0x77220C;
        szRankName = RankName;
        iGoldLimitPerDay = FullPermissions ? -1 : 0;
        for(uint32 j = 0; j < MAX_GUILD_BANK_TABS; ++j)
        {
            iTabPermissions[j].iFlags = FullPermissions ? GR_RIGHT_GUILD_BANK_ALL : 0;
            iTabPermissions[j].iStacksPerDay = FullPermissions ? -1 : 0;
        }
    }

    uint32 iId;
    uint32 iRights;
    uint32 DailyXPCap;
    std::string szRankName;
    int32 iGoldLimitPerDay;
    GuildRankTabPermissions iTabPermissions[MAX_GUILD_BANK_TABS];
};

struct GuildRankStorage
{
    GuildRankStorage(uint32 guildid)
    {
        ssid = 0;
        GuildId = guildid;
        RankLock.Acquire();
        for(uint8 i = 0; i < MAX_GUILD_RANKS; i++)
            m_ranks[i] = NULL;
        RankLock.Release();
    };

    uint32 ssid;
    uint32 GuildId;
    Mutex RankLock;
    GuildRank* m_ranks[MAX_GUILD_RANKS];
};

struct GuildLogEvent
{
    uint32 iLogId;
    uint8 iEvent;
    uint32 iTimeStamp;
    uint32 iEventData[3];
};

struct GuildLogStorage
{
    GuildLogStorage(uint32 guildid)
    {
        GuildId = guildid; m_log_High = 0;
    };

    Mutex Locks;
    uint32 GuildId;
    uint32 m_log_High;
    std::vector<GuildLogEvent*> m_logs;
};

struct GuildBankEvent
{
    uint32 iLogId;
    uint8 iAction;
    uint32 uPlayer;
    uint32 uEntry;
    uint8 iStack;
    uint32 uTimeStamp;
};

struct BankLogInternalStorage
{
    uint32 log_high_guid;
    std::list<GuildBankEvent*> m_logs;
};

struct GuildBankTab
{
    GuildBankTab(uint32 tabId)
    {
        iTabId = tabId;
        szTabName = szTabIcon = szTabInfo = "";
        for(uint32 i = 0; i < MAX_GUILD_BANK_SLOTS; i++)
            pSlots[i] = NULL;
    }

    uint8 iTabId;
    std::string szTabName;
    std::string szTabIcon;
    std::string szTabInfo;
    Item *pSlots[MAX_GUILD_BANK_SLOTS];
};

struct GuildBankTabStorage
{
    GuildBankTabStorage(uint32 guildid)
    {
        ssid = 0;
        GuildId = guildid;
        money_high_guid = 1;
        for(uint8 i = 0; i < MAX_GUILD_BANK_TABS; i++)
            m_Tabs[i] = NULL;
        for(uint8 i = 0; i < MAX_GUILD_BANK_TABS; i++)
            m_TabLogs[i] = NULL;
    };

    uint32 ssid;
    uint32 GuildId;
    uint32 money_high_guid;
    GuildBankTab* m_Tabs[MAX_GUILD_BANK_TABS];
    BankLogInternalStorage* m_TabLogs[MAX_GUILD_BANK_TABS];
    std::list<GuildBankEvent*> m_money_logs;
};

struct GuildMember
{
    GuildMember(uint32 _guildId, WoWGuid PlrLowGuid, PlayerInfo *PlrI, GuildRank *GRank)
    {
        guildId = _guildId;
        pRank = GRank;
        pPlayer = PlrI;
        PlrGuid = PlrLowGuid;
        szPublicNote = szOfficerNote = "";
        uLastWithdrawReset = 0;
        uWithdrawlsSinceLastReset = 0;
        for(uint8 i = 0; i < MAX_GUILD_BANK_TABS; i++)
        {
            uLastItemWithdrawReset[i] = 0;
            uItemWithdrawlsSinceLastReset[i] = 0;
        }
        guildRepCapModifier = 1;
        guildReputation = 0;
        weekReputation = 0;
        weeklyActivity = 0;
        totalActivity = 0;

        guildXPCapModifier = 1;
        guildXPToday = 0;
        guildWeekXP = 0;
    }

    uint32 guildId;
    WoWGuid PlrGuid;
    GuildRank *pRank;
    PlayerInfo *pPlayer;
    std::string szPublicNote;
    std::string szOfficerNote;

    uint32 uLastWithdrawReset;
    uint32 uWithdrawlsSinceLastReset;
    uint32 uLastItemWithdrawReset[MAX_GUILD_BANK_TABS];
    uint32 uItemWithdrawlsSinceLastReset[MAX_GUILD_BANK_TABS];

    uint32 guildRepCapModifier;
    uint32 guildReputation;
    uint32 weekReputation;
    uint32 weeklyActivity;
    uint32 totalActivity;

    uint32 guildXPCapModifier;
    uint64 guildXPToday;
    uint64 guildWeekXP;
};

typedef std::map<WoWGuid, GuildMember*> GuildMemberMap;

struct GuildMemberMapStorage
{
    GuildMemberMapStorage(uint32 guildid) { GuildId = guildid; };

    uint32 GuildId;
    Mutex MemberMapLock;
    GuildMemberMap MemberMap;
};

#pragma pack(PRAGMA_POP)

typedef std::vector<GuildLogEvent*> GuildLogList;
typedef std::map<uint32, GuildInfo*> GuildInfoMap;
typedef std::map<uint32, GuildLogStorage*> GuildLogMap;
typedef std::map<uint32, GuildRankStorage*> GuildRankMap;
typedef std::map<std::string, GuildInfo*> GuildInfoNameMap;
typedef std::map<uint32, GuildBankTabStorage*> GuildBankTabMap;
typedef std::map<uint32, GuildMemberMapStorage* > GuildMemberMaps;

class SERVER_DECL GuildMgr : public Singleton < GuildMgr >
{
private:
    uint32 m_hiGuildId;
    uint32 m_hiCharterId;
    Mutex m_guidGenMutex;

    // Generation locks.
    Mutex m_TabLocks;
    Mutex m_RankLocks;
    Mutex m_GuildLocks;

    // By guild guid.
    GuildInfoMap m_Guilds;
    GuildLogMap m_GuildLogs;
    GuildRankMap m_GuildRanks;
    GuildBankTabMap m_GuildTabs;
    GuildInfoNameMap m_GuildNames;
    GuildMemberMaps m_GuildMemberMaps;

    // By player guid.
    GuildMemberMap m_GuildMembers;

    uint32 m_updateTimer;
public:
    GuildMgr();
    ~GuildMgr();

    void Update(uint32 p_time);

public:
    void LoadAllGuilds();
    void SaveAllGuilds();

    void DestroyGuild(GuildInfo* guildInfo);
    void AddDestructionQueries(uint32 guildid);
    void SaveGuild(QueryBuffer* qb, GuildInfo* guildInfo);

    bool IsGuildPerk(SpellEntry *sp);
    void ModifyGuildLevel(GuildInfo *gInfo, int32 mod);
    void AddGuildPerks(Player *plr, GuildInfo *info = NULL);
    void RemoveGuildPerks(Player *plr, GuildInfo *info = NULL);
    void GuildGainXP(Player *plr, uint32 xpGain);
    void SetNote(PlayerInfo* pInfo, std::string Note, bool Officer);
    void LogGuildBankAction(uint64 GuildId, uint8 iAction, uint32 uGuid, uint32 uEntry, uint8 iStack, uint32 tabId);
    void AddGuildLogEntry(uint32 GuildId, uint8 iEvent, uint32 arguement1, uint32 arguement2 = 0, uint32 arguement3 = 0);
    void LogGuildEvent(Player* plr, uint32 GuildId, uint8 iEvent, std::string arguement1, std::string arguement2 = "", std::string arguement3 = "", std::string arguement4 = "");

    void CreateGuildFromCharter(Charter* charter);
    void CreateGuildFromCommand(std::string name, uint32 gLeader);

    GuildRank* FindLowestRank(GuildRankStorage* Ranks);
    GuildRank* FindHighestRank(GuildRankStorage* Ranks);
    GuildRankStorage* ConstructRankStorage(uint32 GuildId);

    uint32 GetWeeklyRepCap() { return m_weeklyRepCap; }
    static uint64 GetXPForNextGuildLevel(uint32 level) { ASSERT(level); return level >= 25 ? 0 : (GuildXPPerLevel[level-1]*1000); }

    uint32 RemoveGuildRank(uint32 GuildId);
    bool HasGuildRights(Player* plr, uint32 Rights);
    bool HasGuildBankRights(Player* plr, uint8 tabId, uint32 Rights);
    uint64 CalculateAvailableAmount(GuildMember* gMember);
    void OnMoneyWithdraw(GuildMember* gMember, uint32 amount);
    void OnItemWithdraw(GuildMember* gMember, uint32 tab);
    uint32 CalculateAllowedItemWithdraws(GuildMember* gMember, uint32 tab);
    void CreateGuildRank(GuildRankStorage* storage, const char* szRankName, uint32 iPermissions, bool bFullGuildBankPermissions);
    uint32 GetMemberCount(uint32 GuildId) { GuildMemberMapStorage* storagemap = GetGuildMemberMapStorage(GuildId); if(storagemap == NULL) return uint32(NULL); return uint32(storagemap->MemberMap.size()); };
    uint32 GetGuildBankTabCount(uint32 GuildId)
    {
        m_TabLocks.Acquire();
        if(m_GuildTabs.find(GuildId) != m_GuildTabs.end())
        {
            m_TabLocks.Release();
            return m_GuildTabs[GuildId]->ssid;
        }
        m_TabLocks.Release();
        return 0;
    };

    uint32 GetTotalGuildCount() { return uint32(m_Guilds.size()); };
    uint32 GenerateGuildId()
    {
        uint32 r;
        m_guidGenMutex.Acquire();
        r = ++m_hiGuildId;
        m_guidGenMutex.Release();
        return r;
    }

    GuildMember* GetGuildMember(WoWGuid PlrGuid)
    {
        if(m_GuildMembers.find(PlrGuid) != m_GuildMembers.end())
            return m_GuildMembers[PlrGuid];
        return NULL;
    };

    GuildLogStorage* GetGuildLogStorage(uint32 GuildId)
    {
        m_GuildLocks.Acquire();
        if(m_Guilds[GuildId] && m_Guilds[GuildId]->m_GuildStatus == GUILD_STATUS_DISBANDED)
        {
            m_GuildLocks.Release();
            return NULL;
        }
        m_GuildLocks.Release();

        if(m_GuildLogs.find(GuildId) != m_GuildLogs.end())
            return m_GuildLogs[GuildId];
        return NULL;
    };

    GuildInfo* GetGuildByGuildName(std::string name)
    {
        GuildInfoNameMap::iterator itr = m_GuildNames.find(name);
        if(itr != m_GuildNames.end())
        {
            if(itr->second->m_GuildStatus != GUILD_STATUS_DISBANDED)
                return itr->second;
        }
        return NULL;
    };

    GuildMemberMapStorage* GetGuildMemberMapStorage(uint32 GuildId)
    {
        m_GuildLocks.Acquire();
        if(m_Guilds[GuildId] && m_Guilds[GuildId]->m_GuildStatus == GUILD_STATUS_DISBANDED)
        {
            m_GuildLocks.Release();
            return NULL;
        }
        m_GuildLocks.Release();

        GuildMemberMaps::iterator itr = m_GuildMemberMaps.find(GuildId);
        if(itr != m_GuildMemberMaps.end())
            return itr->second;
        return NULL;
    };

    GuildInfo* GetGuildInfo(uint32 GuildId)
    {
        m_GuildLocks.Acquire();
        if(m_Guilds.find(GuildId) != m_Guilds.end())
        {
            if(m_Guilds[GuildId] == NULL)
            {
                m_Guilds.erase(GuildId);
                m_GuildLocks.Release();
                return NULL;
            }

            if(m_Guilds[GuildId]->m_GuildStatus != GUILD_STATUS_DISBANDED)
            {
                m_GuildLocks.Release();
                return m_Guilds[GuildId];
            }
        }
        m_GuildLocks.Release();
        return NULL;
    };

    GuildRankStorage* GetGuildRankStorage(uint32 GuildId)
    {
        m_GuildLocks.Acquire();
        if(m_Guilds[GuildId] && m_Guilds[GuildId]->m_GuildStatus == GUILD_STATUS_DISBANDED)
        {
            m_GuildLocks.Release();
            return NULL;
        }
        m_GuildLocks.Release();

        m_RankLocks.Acquire();
        if(m_GuildRanks.find(GuildId) != m_GuildRanks.end())
        {
            m_RankLocks.Release();
            return m_GuildRanks[GuildId];
        }
        m_RankLocks.Release();
        return NULL;
    };

    GuildBankTabStorage* GetGuildBankTabStorage(uint32 GuildId)
    {
        m_GuildLocks.Acquire();
        if(m_Guilds[GuildId] && m_Guilds[GuildId]->m_GuildStatus == GUILD_STATUS_DISBANDED)
        {
            m_GuildLocks.Release();
            return NULL;
        }
        m_GuildLocks.Release();

        m_TabLocks.Acquire();
        if(m_GuildTabs.find(GuildId) != m_GuildTabs.end())
        {
            m_TabLocks.Release();
            return m_GuildTabs[GuildId];
        }
        m_TabLocks.Release();
        return NULL;
    };

public: // Guild Functions and calls
    bool Disband(uint32 guildId);
    void PlayerLoggedIn(PlayerInfo* plr);
    void PlayerLoggedOff(PlayerInfo* plr);
    void SendMotd(PlayerInfo* plr, uint32 guildid = 0);
    void RemoveMember(Player* remover, PlayerInfo* removee);
    void ForceRemoveMember(Player* remover, PlayerInfo* removee);
    void AddGuildMember(GuildInfo* gInfo, PlayerInfo* newmember, WorldSession* pClient, int32 ForcedRank = -1);

public: // Direct Packet Handlers
    void Packet_DisbandGuild(WorldSession* m_session);
    void Packet_SetMotd(WorldSession* m_session, std::string motd);

    // Chat!
    void GuildChat(WorldSession* m_session, uint32 Language, const char* message);
    void OfficerChat(WorldSession* m_session, uint32 Language, const char* message);

    // Invite Handlers
    void Packet_GuildInviteAccept(WorldSession* m_session);
    void Packet_GuildInviteDecline(WorldSession* m_session);
    void Packet_HandleGuildInvite(WorldSession* m_session, std::string inviteeName);

    // Requests
    void Packet_SendGuildLog(WorldSession* m_session);
    void Packet_SendGuildXP(WorldSession *m_session);
    void Packet_SendGuildMaxDailyXP(WorldSession *m_session, WoWGuid guid);
    void Packet_SendGuildNews(WorldSession *m_session);
    void Packet_SendGuildRankInfo(WorldSession *m_session);
    void Packet_SendGuildRoster(WorldSession* m_session);
    void Packet_SendGuildRewards(WorldSession* m_session);
    void Packet_SendGuildPermissions(WorldSession* m_session);
    void Packet_SendGuildPartyState(WorldSession* m_session);
    void Packet_SendGuildChallengeUpdate(WorldSession* m_session);
    void Packet_SendGuildQuery(WorldSession* m_session, uint32 guildid, WoWGuid targetPlayer = 0);
    void Packet_SendGuildBankLog(WorldSession* m_session, uint32 slotid);
    void Packet_SendGuildBankText(WorldSession* m_session, uint8 TabSlot);

    // Information
    void Packet_SetGuildInformation(WorldSession* m_session, std::string guildInfo);
    void Packet_SetGuildBankText(WorldSession* m_session, uint32 tabid, std::string tabtext);
    void Packet_SetMemberNote(WorldSession* m_session, PlayerInfo* pInfo, bool officer, std::string note);
    void Packet_SaveGuildEmblem(WorldSession* m_session, uint32 emblemStyle, uint32 emblemColor, uint32 borderStyle, uint32 borderColor, uint32 backgroundColor);

    // Rank Handlers
    void Packet_HandleDeleteRank(WorldSession* m_session);
    void Packet_HandleAddRank(WorldSession* m_session, std::string name);
    void Packet_HandleEditRank(WorldSession* m_session, std::string name, uint32 rankid, uint32 rankrights, int32 DailyGoldLimit, uint32* iflags, int32* istacksperday);

    // Member Handlers
    void Packet_ChangeGuildLeader(WorldSession* m_session, PlayerInfo* newLeader);
    void Packet_DemoteGuildMember(WorldSession* m_session, PlayerInfo *demoteeInfo);
    void Packet_PromoteGuildMember(WorldSession* m_session, PlayerInfo *promoteeInfo);

    // Bank Handlers
    void Packet_WithdrawItem(WorldSession* m_session, uint8 dest_bank, uint8 dest_bankslot, uint8 source_bank, uint8 source_bankslot, uint32 itementry, uint8 autostore, int32 splitted_count);
    void Packet_DepositItem(WorldSession* m_session, uint8 dest_bank, uint8 dest_bankslot, uint32 itementry, uint8 autostore, uint8 withdraw_stack, int8 source_bagslot, int8 source_slot, uint8 tochar, uint8 deposit_stack);

    void Packet_SendAvailableBankFunds(WorldSession* m_session);
    void Packet_BuyBankTab(WorldSession* m_session, uint64 BankGuid);
    void Packet_SendGuildBankTab(WorldSession* m_session, uint8 TabSlot, bool withTabInfo);
    void Packet_SendGuildBankTabUpdate(WorldSession* m_session, uint8 TabSlot, std::set<uint8> slots);
    void Packet_SendGuildBankInfo(WorldSession* m_session, uint64 BankGuid);
    void Packet_DepositMoney(WorldSession* m_session, uint64 BankGuid, uint32 Amount);
    void Packet_WithdrawMoney(WorldSession* m_session, uint64 BankGuid, uint32 Amount);
    void Packet_SetBankTabInfo(WorldSession* m_session, uint64 BankGuid, uint8 TabSlot, std::string TabName, std::string TabIcon);

private:
    bool m_GuildsLoaded;
    bool m_GuildsLoading;
    uint32 m_weeklyRepCap;

    /***************
    ** Charter section.
    *****/
public:
    void RemoveCharter(Charter *);
    void LoadGuildCharters();

    Charter *CreateCharter(uint32 LeaderGuid, CharterTypes Type);
    Charter *GetCharter(uint32 CharterId, CharterTypes Type);
    Charter *GetCharterByName(std::string &charter_name, CharterTypes Type);
    Charter *GetCharterByItemGuid(uint32 guid);
    Charter *GetCharterByGuid(uint64 playerguid, CharterTypes type);

private:
    RWLock m_charterLock;
    std::map<uint32, Charter*> m_charters[NUM_CHARTER_TYPES];

public: // Direct Packet Handlers
    void CharterBuy(WorldSession* m_session, uint64 SellerGuid, std::string name, uint32 petitionCount, uint32 ArenaIndex);
};

class SERVER_DECL Charter
{
public:
    uint32 GetNumberOfSlotsByType()
    {
        switch(CharterType)
        {
        case CHARTER_TYPE_GUILD:
            return 9;

        case CHARTER_TYPE_ARENA_2V2:
            return 1;

        case CHARTER_TYPE_ARENA_3V3:
            return 2;

        case CHARTER_TYPE_ARENA_5V5:
            return 4;
        default:
            return 9;
        }
    }

    uint32 SignatureCount;
    WoWGuid *Signatures;
    uint32 CharterType;
    uint32 Slots;
    uint32 LeaderGuid;
    uint32 ItemGuid;
    uint32 CharterId;
    std::string GuildName;

    Charter(Field * fields);
    Charter(uint32 id, uint32 leader, uint32 type) : CharterType(type), LeaderGuid(leader), CharterId(id)
    {
        SignatureCount = 0;
        ItemGuid = 0;
        Slots = GetNumberOfSlotsByType();
        Signatures = new WoWGuid[Slots];
        memset(Signatures, 0, sizeof(WoWGuid)*Slots);
    }

    ~Charter()
    {
        delete [] Signatures;
    }

    void SaveToDB();
    void Destroy();      // When item is deleted.

    void AddSignature(WoWGuid PlayerGuid);
    void RemoveSignature(WoWGuid PlayerGuid);

    RONIN_INLINE uint32 GetLeader() { return LeaderGuid; }
    RONIN_INLINE uint32 GetID() { return CharterId; }

    RONIN_INLINE bool IsFull() { return (SignatureCount == Slots); }
};

#define guildmgr GuildMgr::getSingleton()
