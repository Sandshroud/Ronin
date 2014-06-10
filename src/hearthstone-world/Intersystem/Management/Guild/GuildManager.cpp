/***
 * Demonstrike Core
 */

#include "StdAfx.h"

#define USE_QMGR_QUERYBUFFER
#ifdef USE_QMGR_QUERYBUFFER
//#define MASS_SAVE_QUERYBUFFER
#define QMGR_EXECUTE qb->AddQuery
#else
#define QMGR_EXECUTE CharacterDatabase.Execute
#endif

initialiseSingleton( GuildMgr );

GuildMgr::GuildMgr()
{
    m_hiGuildId = 0;
    m_updateTimer = 0;
    m_GuildsLoading = true;

    QueryResult *result = CharacterDatabase.Query("SELECT MAX(guildId) FROM guilds");
    if(result)
    {
        m_hiGuildId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(charterId) FROM charters");
    if(result)
    {
        m_hiCharterId = result->Fetch()[0].GetUInt32();
        delete result;
    }
}

GuildMgr::~GuildMgr()
{
    for(GuildMemberMaps::iterator itr = m_GuildMemberMaps.begin(); itr != m_GuildMemberMaps.end(); itr++ )
    {
        itr->second->MemberMap.clear();
        delete itr->second;
    }

    for(GuildInfoMap::iterator itr = m_Guilds.begin(); itr != m_Guilds.end(); itr++)
        delete itr->second;

    for(GuildMemberMap::iterator itr = m_GuildMembers.begin(); itr != m_GuildMembers.end(); itr++)
    {
        itr->second->pRank = NULL;
        itr->second->pPlayer = NULL;
        delete itr->second;
    }

    for(GuildLogMap::iterator itr = m_GuildLogs.begin(); itr != m_GuildLogs.end(); itr++)
    {
        itr->second->m_logs.clear();
        delete itr->second;
    }

    for(GuildRankMap::iterator itr = m_GuildRanks.begin(); itr != m_GuildRanks.end(); itr++)
    {
        for(uint8 i = 0; i < MAX_GUILD_RANKS; i++)
        {
            if(itr->second->m_ranks[i] != NULL)
            {
                delete itr->second->m_ranks[i];
                itr->second->m_ranks[i] = NULL;
            }
        }
        delete itr->second;
    }

    for(GuildBankTabMap::iterator itr = m_GuildTabs.begin(); itr != m_GuildTabs.end(); itr++)
    {
        for(uint8 i = 0; i < MAX_GUILD_BANK_TABS; i++)
        {
            if(itr->second->m_Tabs[i] != NULL)
            {
                memset(&itr->second->m_Tabs[i]->pSlots, 0, 0);
                delete itr->second->m_Tabs[i];
                itr->second->m_Tabs[i] = NULL;
            }

            if(itr->second->m_TabLogs[i] != NULL)
            {
                itr->second->m_TabLogs[i]->m_logs.clear();
                delete itr->second->m_TabLogs[i];
                itr->second->m_TabLogs[i] = NULL;
            }
        }
        delete itr->second;
    }
}

void GuildMgr::Update(uint32 p_time)
{
    m_updateTimer += p_time;
    if(m_updateTimer > 10000)
    {
        m_updateTimer -= 10000;
        SaveAllGuilds();
    }
}

void GuildMgr::LoadAllGuilds()
{
    sWorld.GuildsLoading = true;
    sLog.Notice("GuildMgr", "Loading Guilds.");

    QueryResult *result = CharacterDatabase.Query( "SELECT * FROM guilds" );
    if(result)
    {
        uint32 c = 0;
        const char* temp = "";
        uint32 period = (result->GetRowCount() / 20) + 1;
        do
        {
            Field* f = result->Fetch();
            GuildInfo* gInfo = new GuildInfo();
            gInfo->m_GuildStatus = GUILD_STATUS_JUST_LOADED;
            gInfo->m_guildId = f[0].GetUInt32();
            gInfo->m_guildName = string(strlen(temp = f[1].GetString()) ? strdup(temp) : "");
            gInfo->m_guildLeader = f[2].GetUInt32();
            gInfo->m_emblemStyle = f[3].GetUInt32();
            gInfo->m_emblemColor = f[4].GetUInt32();
            gInfo->m_borderStyle = f[5].GetUInt32();
            gInfo->m_borderColor = f[6].GetUInt32();
            gInfo->m_backgroundColor = f[7].GetUInt32();
            gInfo->m_guildInfo = string(strlen(temp = f[8].GetString()) ? strdup(temp) : "");
            gInfo->m_motd = string(strlen(temp = f[9].GetString()) ? strdup(temp) : "");
            gInfo->m_creationTimeStamp = f[10].GetUInt32();
            uint32 high = f[11].GetUInt32(), low = f[12].GetUInt64();
            gInfo->m_bankBalance = uint64(uint64(low) | (uint64(high)<<32));
            gInfo->m_commandLogging = true;
            m_Guilds.insert(make_pair(gInfo->m_guildId, gInfo));
            m_GuildNames.insert(make_pair(gInfo->m_guildName, gInfo));
            if( !((++c) % period) )
                sLog.Notice("GuildMgr", "Done %u/%u, %u%% complete.", c, result->GetRowCount(), float2int32( (float(c) / float(result->GetRowCount()))*100.0f ));
        } while(result->NextRow());
        delete result;
        result = NULL;
    }

    result = CharacterDatabase.Query("SELECT * FROM guild_banktabs ORDER BY tabId ASC");
    if(result != NULL)
    {
        Field* f = NULL;
        const char* temp = "";
        uint32 GuildId = 0, tabid = 0;
        GuildBankTabStorage* BankTabStorage = NULL;
        do
        {
            f = result->Fetch();
            GuildId = f[0].GetUInt32();
            if(m_Guilds.find(GuildId) == m_Guilds.end())
                continue;

            tabid = f[1].GetUInt32();
            BankTabStorage = GetGuildBankTabStorage(GuildId);
            if(BankTabStorage == NULL)
            {
                BankTabStorage = new GuildBankTabStorage(GuildId);
                m_GuildTabs.insert(make_pair(GuildId, BankTabStorage));
            }

            if((BankTabStorage->ssid++) != tabid)
            {
                sLog.outError("Guild bank tabs are out of order!");
#ifdef WIN32
                TerminateProcess(GetCurrentProcess(), 0);
                return;
#else
                exit(0);
                return;
#endif
            }

            if(BankTabStorage->m_Tabs[tabid] == NULL)
                BankTabStorage->m_Tabs[tabid] = new GuildBankTab(tabid);
            BankTabStorage->m_Tabs[tabid]->szTabName = string(strlen(temp = f[2].GetString()) ? strdup(temp) : "");
            BankTabStorage->m_Tabs[tabid]->szTabIcon = string(strlen(temp = f[3].GetString()) ? strdup(temp) : "");
            BankTabStorage->m_Tabs[tabid]->szTabInfo = string(strlen(temp = f[4].GetString()) ? strdup(temp) : "");
            BankTabStorage = NULL;
        }while(result->NextRow());
        delete result;
        result = NULL;
    }

    result = CharacterDatabase.Query("SELECT * FROM guild_ranks ORDER BY rankId ASC");
    if(result != NULL)
    {
        Field* f = NULL;
        uint32 GuildId = 0;
        const char* temp = "";
        GuildInfo* gInfo = NULL;
        do
        {
            f = result->Fetch();
            GuildId = f[0].GetUInt32();
            if((gInfo = GetGuildInfo(GuildId)) == NULL)
                continue;

            GuildRank* r = new GuildRank(f[1].GetUInt32(), f[3].GetUInt32(), string(strlen(temp = f[2].GetString()) ? strdup(temp) : "").c_str(), false);
            r->iGoldLimitPerDay = f[4].GetInt32();
            for(uint32 j = 0; j < MAX_GUILD_BANK_TABS; ++j)
            {
                r->iTabPermissions[j].iFlags = f[5 + (j * 2)].GetUInt32();
                r->iTabPermissions[j].iStacksPerDay = f[6 + (j * 2)].GetInt32();
            }

            m_RankLocks.Acquire();
            GuildRankStorage* storage = NULL;
            if(m_GuildRanks.find(GuildId) == m_GuildRanks.end())
            {
                storage = new GuildRankStorage(GuildId);
                m_GuildRanks.insert(make_pair(GuildId, storage));
                m_RankLocks.Release();
            }
            else { m_RankLocks.Release(); storage = m_GuildRanks[GuildId]; }

            if(r->iId != storage->ssid)
            {
                sLog.Notice("GuildMgr", "Renaming rank %u of guild %s to %u.", r->iId, gInfo->m_guildName.c_str(), storage->ssid);
                r->iId = storage->ssid;
            }
            storage->ssid++;

            storage->m_ranks[r->iId] = r;
            gInfo = NULL;
        }
        while(result->NextRow());
        delete result;
        result = NULL;
    }

    result = CharacterDatabase.Query("SELECT * FROM guild_data");
    if(result != NULL)
    {
        Field* f = NULL;
        uint32 GuildId = 0;
        const char* temp = "";
        GuildInfo* gInfo = NULL;
        GuildRankStorage* rankstorage = NULL;
        GuildMemberMapStorage* MemberMapStorage = NULL;
        do
        {
            f = result->Fetch();
            GuildId = f[0].GetUInt32();
            if((gInfo = GetGuildInfo(GuildId)) == NULL)
                continue;
            rankstorage = GetGuildRankStorage(GuildId);
            if(rankstorage == NULL)
                rankstorage = ConstructRankStorage(GuildId);

            uint32 guid = f[1].GetUInt32();
            GuildMember* gm = new GuildMember(guid, objmgr.GetPlayerInfo(guid), NULL);
            if(gm->pPlayer == NULL)
            {
                delete gm;
                continue;
            }
            gm->pPlayer->GuildId = GuildId;

            uint32 rankid = f[2].GetUInt32();
            if(rankid >= rankstorage->ssid)
                rankid = rankstorage->ssid-1;

            gm->pPlayer->GuildRank = rankid;
            gm->pRank = rankstorage->m_ranks[rankid];
            if(gm->pRank == NULL)
                gm->pRank = FindLowestRank(rankstorage);

            gm->szPublicNote = string(strlen(temp = f[3].GetString()) ? strdup(temp) : "");
            gm->szOfficerNote = string(strlen(temp = f[4].GetString()) ? strdup(temp) : "");
            gm->uLastWithdrawReset = f[5].GetUInt32();
            gm->uWithdrawlsSinceLastReset = f[6].GetUInt32();
            for(uint32 j = 0; j < MAX_GUILD_BANK_TABS; j++)
            {
                gm->uLastItemWithdrawReset[j] = f[7 + (j * 2)].GetUInt32();
                gm->uItemWithdrawlsSinceLastReset[j] = f[8 + (j * 2)].GetUInt32();
            }

            if(m_GuildMemberMaps.find(GuildId) == m_GuildMemberMaps.end())
            {
                MemberMapStorage = new GuildMemberMapStorage(GuildId);
                m_GuildMemberMaps.insert(make_pair(GuildId, MemberMapStorage));
            }
            else
                MemberMapStorage = m_GuildMemberMaps[GuildId];

            MemberMapStorage->MemberMap.insert(make_pair(gm->pPlayer->guid, gm));
            m_GuildMembers.insert(make_pair(gm->pPlayer->guid, gm));

            gInfo = NULL;
            rankstorage = NULL;
        }while(result->NextRow());
        delete result;
        result = NULL;
    }

    result = CharacterDatabase.Query("SELECT * FROM guild_logs ORDER BY timestamp ASC");
    if(result != NULL)
    {
        Field* f = NULL;
        uint32 GuildId = 0;
        GuildLogStorage* LogStorage = NULL;
        do
        {
            f = result->Fetch();
            GuildId = f[1].GetUInt32();
            LogStorage = GetGuildLogStorage(GuildId);
            if(LogStorage == NULL)
            {
                LogStorage = new GuildLogStorage(GuildId);
                m_GuildLogs.insert(make_pair(GuildId, LogStorage));
            }

            GuildLogEvent* li = new GuildLogEvent();
            li->iLogId = f[0].GetUInt32();
            li->iTimeStamp = f[2].GetUInt32();
            li->iEvent = f[3].GetUInt8();
            li->iEventData[0] = f[4].GetUInt32();
            li->iEventData[1] = f[5].GetUInt32();
            li->iEventData[2] = f[6].GetUInt32();
            if(LogStorage->m_log_High <= li->iLogId)
                LogStorage->m_log_High = li->iLogId+1;
            LogStorage->m_logs.push_back(li);
            LogStorage = NULL;
        }while(result->NextRow());
        delete result;
        result = NULL;
    }

    result = CharacterDatabase.Query("SELECT * FROM guild_banklogs ORDER BY timestamp ASC");
    if(result != NULL)
    {
        Field* f = NULL;
        uint32 GuildId = 0, tabid = 0;
        GuildBankTabStorage* BankTabStorage = NULL;
        do
        {
            f = result->Fetch();
            GuildId = f[1].GetUInt32();
            tabid = f[2].GetUInt32();
            if(tabid > MAX_GUILD_BANK_TABS)
                continue;

            BankTabStorage = GetGuildBankTabStorage(GuildId);
            if(BankTabStorage == NULL)
                continue; // Shouldn't happen.

            if(tabid == 6)
            {
                GuildBankEvent* ev = new GuildBankEvent();
                ev->iLogId = f[0].GetUInt32();
                ev->iAction = f[3].GetUInt8();
                ev->uPlayer = f[4].GetUInt32();
                ev->uEntry = f[5].GetUInt32();
                ev->iStack = f[6].GetUInt8();
                ev->uTimeStamp = f[7].GetUInt32();
                BankTabStorage->m_money_logs.push_back(ev);
                BankTabStorage = NULL;
                continue;
            }

            if(BankTabStorage->m_TabLogs[tabid] == NULL)
                BankTabStorage->m_TabLogs[tabid] = new BankLogInternalStorage();

            GuildBankEvent* ev = new GuildBankEvent();
            ev->iLogId = f[0].GetUInt32();
            ev->iAction = f[3].GetUInt8();
            ev->uPlayer = f[4].GetUInt32();
            ev->uEntry = f[5].GetUInt32();
            ev->iStack = f[6].GetUInt8();
            ev->uTimeStamp = f[7].GetUInt32();
            BankTabStorage->m_TabLogs[tabid]->m_logs.push_back(ev);
            BankTabStorage = NULL;
        }while(result->NextRow());
        delete result;
        result = NULL;
    }

    result = CharacterDatabase.Query("SELECT * FROM guild_bankitems");
    if(result != NULL)
    {
        Field* f = NULL;
        uint32 GuildId = 0, tabid = 0;
        GuildBankTabStorage* BankTabStorage = NULL;
        do
        {
            f = result->Fetch();
            GuildId = f[0].GetUInt32();
            tabid = f[1].GetUInt32();
            if(tabid >= MAX_GUILD_BANK_TABS)
                continue;
            BankTabStorage = GetGuildBankTabStorage(GuildId);
            if(BankTabStorage == NULL)
                continue;

            uint32 slot = f[2].GetUInt32();
            if(slot >= MAX_GUILD_BANK_SLOTS)
                continue;

            uint32 itemGuid = f[3].GetUInt32();
            Item* pItem = objmgr.LoadItem(itemGuid);
            if(pItem == NULL)
                continue;

            BankTabStorage->m_Tabs[tabid]->pSlots[slot] = pItem;
        }
        while(result->NextRow());
        delete result;
        result = NULL;
    }

    for(GuildInfoMap::iterator itr, itr2 = m_Guilds.begin(); itr2 != m_Guilds.end();)
    {
        itr = itr2++;
        if(itr->second == NULL)
        {
            m_Guilds.erase(itr->first);
            continue;
        }

        if(m_GuildRanks.find(itr->first) == m_GuildRanks.end())
            ConstructRankStorage(itr->first);
        if(m_GuildMemberMaps.find(itr->first) == m_GuildMemberMaps.end())
            m_GuildMemberMaps.insert(make_pair(itr->first, new GuildMemberMapStorage(itr->first)));
        if(m_GuildLogs.find(itr->first) == m_GuildLogs.end())
            m_GuildLogs.insert(make_pair(itr->first, new GuildLogStorage(itr->first)));
        if(m_GuildTabs.find(itr->first) == m_GuildTabs.end())
            m_GuildTabs.insert(make_pair(itr->first, new GuildBankTabStorage(itr->first)));
    }

    sLog.Notice("GuildMgr", "%u guilds loaded.", m_Guilds.size());
    sWorld.GuildsLoading = false;
    m_GuildsLoading = false;
}

void GuildMgr::AddDestructionQueries(uint32 guildid)
{
#ifdef USE_QMGR_QUERYBUFFER
    QueryBuffer *qb = new QueryBuffer();
#endif
    QMGR_EXECUTE("DELETE FROM `guilds` WHERE guildId = '%u';", guildid);
    QMGR_EXECUTE("DELETE FROM `guild_data` WHERE guildId = '%u';", guildid);
    QMGR_EXECUTE("DELETE FROM `guild_logs` WHERE guildId = '%u';", guildid);
    QMGR_EXECUTE("DELETE FROM `guild_ranks` WHERE guildId = '%u';", guildid);
    QMGR_EXECUTE("DELETE FROM `guild_banklogs` WHERE guildId = '%u';", guildid);
    QMGR_EXECUTE("DELETE FROM `guild_banktabs` WHERE guildId = '%u';", guildid);
    QMGR_EXECUTE("DELETE FROM `guild_bankitems` WHERE guildId = '%u';", guildid);
#ifdef USE_QMGR_QUERYBUFFER
    CharacterDatabase.AddQueryBuffer(qb);
#endif
}

void GuildMgr::SetNote(PlayerInfo* pInfo, std::string Note, bool Officer)
{
    GuildInfo* gInfo = GetGuildInfo(pInfo->GuildId);
    if(gInfo == NULL)
        return;

    GuildMember* gMember = GetGuildMember(pInfo->guid);
    if(gMember == NULL)
        return;

    if(Officer)
        gMember->szOfficerNote = Note;
    else
        gMember->szPublicNote = Note;
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
}

void GuildMgr::AddGuildLogEntry(uint32 GuildId, uint8 iEvent, uint32 arguement1, uint32 arguement2, uint32 arguement3)
{
    GuildInfo* gInfo = GetGuildInfo(GuildId);
    if(gInfo == NULL || !gInfo->m_commandLogging)
        return;
    GuildLogStorage* LogStorage = GetGuildLogStorage(GuildId);
    if(LogStorage == NULL)
        return;

    uint32 count = 1;
    if(arguement2)
        count++;
    if(arguement3)
        count++;

    LogStorage->Locks.Acquire();
    GuildLogEvent* ev = new GuildLogEvent();
    ev->iLogId = LogStorage->m_log_High++;
    ev->iEvent = iEvent;
    ev->iTimeStamp = (uint32)UNIXTIME;
    ev->iEventData[0] = arguement1;
    ev->iEventData[1] = arguement2;
    ev->iEventData[2] = arguement3;
    LogStorage->m_logs.push_back(ev);
    LogStorage->Locks.Release();
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
}

void GuildMgr::LogGuildBankAction(uint64 GuildId, uint8 iAction, uint32 uGuid, uint32 uEntry, uint8 iStack, uint32 tabId)
{
    GuildInfo* gInfo = GetGuildInfo(GuildId);
    if(gInfo == NULL || !gInfo->m_commandLogging)
        return;
    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(GuildId);
    if(BankTabStorage == NULL)
        return;

    if(tabId == 6)
    {
        GuildBankEvent * ev = new GuildBankEvent();
        ev->iAction = iAction;
        ev->iStack = 0;
        ev->uEntry = uEntry;
        ev->uPlayer = uGuid;
        ev->uTimeStamp = (uint32)UNIXTIME;
        gInfo->m_GuildLock.Acquire();
        if(BankTabStorage->m_money_logs.size() >= 25)
        {
            // pop one off the end
            GuildBankEvent * ev2 = *(BankTabStorage->m_money_logs.begin());
            BankTabStorage->m_money_logs.pop_front();
            delete ev2;
        }
        ev->iLogId = BankTabStorage->money_high_guid++;
        BankTabStorage->m_money_logs.push_back(ev);
        gInfo->m_GuildLock.Release();
        return;
    }

    GuildBankEvent * ev = new GuildBankEvent();
    uint32 timest = (uint32)UNIXTIME;
    ev->iAction = iAction;
    ev->iStack = iStack;
    ev->uEntry = uEntry;
    ev->uPlayer = uGuid;
    ev->uTimeStamp = timest;

    gInfo->m_GuildLock.Acquire();

    if(BankTabStorage->m_TabLogs[tabId] == NULL)
    {
        BankTabStorage->m_TabLogs[tabId] = new BankLogInternalStorage();
        BankTabStorage->m_TabLogs[tabId]->log_high_guid = 1;
    }

    if(BankTabStorage->m_TabLogs[tabId]->m_logs.size() >= 25)
    {
        // pop one off the end
        GuildBankEvent * ev2 = *(BankTabStorage->m_TabLogs[tabId]->m_logs.begin());
        BankTabStorage->m_TabLogs[tabId]->m_logs.pop_front();
        delete ev2;
    }

    ev->iLogId = BankTabStorage->m_TabLogs[tabId]->log_high_guid++;
    BankTabStorage->m_TabLogs[tabId]->m_logs.push_back(ev);
    gInfo->m_GuildLock.Release();
}

void GuildMgr::LogGuildEvent(Player* plr, uint32 GuildId, uint8 iEvent, const char* arguement1, const char* arguement2, const char* arguement3, const char* arguement4)
{
    if(plr != NULL)
        GuildId = plr->GetGuildId();
    if(GuildId < 1)
        return;

    GuildInfo* gInfo = GetGuildInfo(GuildId);
    if(gInfo == NULL || !gInfo->m_commandLogging)
        return;

    size_t size = strlen(arguement1), count = 1;
    if(arguement2 != NULL)
    {
        count++;
        size += strlen(arguement2);
    }
    if(arguement3 != NULL)
    {
        count++;
        size += strlen(arguement3);
    }
    if(arguement4 != NULL)
    {
        count++;
        size += strlen(arguement4);
    }

    WorldPacket data(SMSG_GUILD_EVENT, 2+uint32(size));
    data << uint8(iEvent);
    data << uint8(count);
    data << strdup(arguement1);
    if(count > 1)
        data << strdup(arguement2);
    if(count > 2)
        data << strdup(arguement3);
    if(count > 3)
        data << strdup(arguement4);
    if(plr != NULL)
    {
        if(plr->IsInWorld())
            plr->GetSession()->SendPacket(&data);
        else
        {
            WorldPacket* data2 = new WorldPacket(data);
            plr->SendDelayedPacket(data2);
        }
    }
    else
    {
        GuildMemberMapStorage* MemberMapStorage = GetGuildMemberMapStorage(GuildId);
        if(MemberMapStorage != NULL)
        {
            Player* target = NULL;
            MemberMapStorage->MemberMapLock.Acquire();
            for(GuildMemberMap::iterator itr = MemberMapStorage->MemberMap.begin(); itr != MemberMapStorage->MemberMap.end(); itr++)
            {
                target = itr->second->pPlayer->m_loggedInPlayer;
                if(target != NULL)
                {
                    WorldPacket* data2 = new WorldPacket(data);
                    target->SendPacket(data2);
                }
            }
            MemberMapStorage->MemberMapLock.Release();
        }
    }
}

void GuildMgr::SaveGuild(QueryBuffer* qb, GuildInfo* guildInfo)
{
#ifdef USE_QMGR_QUERYBUFFER
    if(qb == NULL)
        qb = new QueryBuffer();
#endif

    uint32 GuildId = guildInfo->m_guildId;
    QMGR_EXECUTE("REPLACE INTO guilds VALUES(%u, \'%s\', %u, %u, %u, %u, %u, %u, \'%s\', \'%s\', %u, %u, %u);", GuildId, CharacterDatabase.EscapeString(guildInfo->m_guildName).c_str(), guildInfo->m_guildLeader,
        guildInfo->m_emblemStyle, guildInfo->m_emblemColor, guildInfo->m_borderColor, guildInfo->m_borderStyle, guildInfo->m_backgroundColor, CharacterDatabase.EscapeString(guildInfo->m_guildInfo).c_str(), 
        CharacterDatabase.EscapeString(guildInfo->m_motd).c_str(), guildInfo->m_creationTimeStamp, GUID_HIPART(guildInfo->m_bankBalance), GUID_LOPART(guildInfo->m_bankBalance));

    guildInfo->m_GuildStatus = GUILD_STATUS_NORMAL;
    uint32 count = 0;
    bool first = true;

    GuildRankStorage* RankStorage = GetGuildRankStorage(GuildId);
    if(RankStorage != NULL)
    {
        stringstream GuildRanks;
        RankStorage->RankLock.Acquire();
        GuildRanks << "REPLACE INTO guild_ranks VALUES";
        for(uint32 i = 0; i < RankStorage->ssid; i++)
        {
            if(RankStorage->m_ranks[i] != NULL)
            {
                if(first)
                    first = false;
                else
                    GuildRanks << ", ";
                GuildRanks << "('" << GuildId << "', '" << RankStorage->m_ranks[i]->iId << "', '" << CharacterDatabase.EscapeString(RankStorage->m_ranks[i]->szRankName).c_str();
                GuildRanks << "', '" << RankStorage->m_ranks[i]->iRights << "', '" << RankStorage->m_ranks[i]->iGoldLimitPerDay << "'";
                for(uint32 j = 0; j < MAX_GUILD_BANK_TABS; j++)
                    GuildRanks << ", '" << RankStorage->m_ranks[i]->iTabPermissions[j].iFlags << "', '" << RankStorage->m_ranks[i]->iTabPermissions[j].iStacksPerDay << "'";
                GuildRanks << ")";
                count++;
            }
        }

        if(count)
        {
            GuildRanks << ";";
            QMGR_EXECUTE("DELETE FROM `guild_ranks` WHERE `guildid` = '%u' AND `rankId` >= '%u';", GuildId, RankStorage->ssid);
            QMGR_EXECUTE(GuildRanks.str().c_str());
        }
        RankStorage->RankLock.Release();
    }
    else
        QMGR_EXECUTE("DELETE FROM `guild_ranks` WHERE `guildid` = '%u';", GuildId);

    // Clear our count and stringstream
    count = 0;
    first = true;

    GuildMemberMapStorage* MapStorage = GetGuildMemberMapStorage(GuildId);
    if(MapStorage)
    {
        MapStorage->MemberMapLock.Acquire();
        stringstream guildData;
        guildData << "DELETE FROM `guild_data` WHERE `guildid` = '%u' AND `playerid` NOT IN(";
        for(GuildMemberMap::iterator itr = MapStorage->MemberMap.begin(); itr != MapStorage->MemberMap.end(); itr++)
        {
            if(first)
                first = false;
            else
                guildData << ", ";
            guildData << "'" << itr->first << "'";
            count++;
        }

        if(count)
        {
            guildData << ");";
            QMGR_EXECUTE(guildData.str().c_str());
        }
        else
            QMGR_EXECUTE("DELETE FROM `guild_data` WHERE `guildid` = '%u';");

        // Clear for next part.
        count = 0;
        first = true;

        stringstream guildData2;
        guildData2 << "REPLACE INTO `guild_data` VALUES";
        for(GuildMemberMap::iterator itr = MapStorage->MemberMap.begin(); itr != MapStorage->MemberMap.end(); itr++)
        {
            if(first)
                first = false;
            else
                guildData2 << ", ";
            guildData2 << "('" << GuildId << "', '" << itr->first << "', '" << itr->second->pRank->iId << "', '" << CharacterDatabase.EscapeString(itr->second->szPublicNote).c_str();
            guildData2 << "', '" << CharacterDatabase.EscapeString(itr->second->szOfficerNote).c_str() << "', '" << itr->second->uLastWithdrawReset << "', '" << itr->second->uWithdrawlsSinceLastReset << "'";
            for(uint32 j = 0; j < MAX_GUILD_BANK_TABS; j++)
                guildData2 << ", '" << itr->second->uLastItemWithdrawReset[j] << "', '" << itr->second->uItemWithdrawlsSinceLastReset[j] << "'";
            guildData2 << ")";
            count++;
        }

        if(count)
        {
            guildData2 << ";";
            QMGR_EXECUTE(guildData2.str().c_str());
        }
        MapStorage->MemberMapLock.Release();
    }
    else
        QMGR_EXECUTE("DELETE FROM `guild_data` WHERE `guildid` = '%u';", GuildId);

    count = 0;
    first = true;

    //guild_logs
    QMGR_EXECUTE("DELETE FROM `guild_logs` WHERE `guildid` = '%u';", GuildId);
    GuildLogStorage* LogStorage = GetGuildLogStorage(GuildId);
    if(LogStorage != NULL)
    {
        stringstream GuildLogs;
        LogStorage->Locks.Acquire();
        GuildLogs << "REPLACE INTO `guild_logs` VALUES";
        for(std::vector<GuildLogEvent*>::iterator itr = LogStorage->m_logs.begin(); itr != LogStorage->m_logs.end(); itr++)
        {
            if(first)
                first = false;
            else
                GuildLogs << ", ";
            GuildLogs << "('" << (*itr)->iLogId << "', '" << GuildId << "', '" << (*itr)->iTimeStamp << "', '" << uint32((*itr)->iEvent) << "'";
            for(uint8 e = 0; e < 3; e++)
                GuildLogs << ", '" << (*itr)->iEventData[e] << "'";
            GuildLogs << ")";
            count++;
        }

        if(count)
        {
            GuildLogs << ";";
            QMGR_EXECUTE(GuildLogs.str().c_str());
        }
        LogStorage->Locks.Release();
    }

    count = 0;
    first = true;

    QMGR_EXECUTE("DELETE FROM `guild_banklogs` WHERE `guildid` = '%u';", GuildId);
    GuildBankTabStorage* BankTabStorage = GetGuildBankTabStorage(GuildId);
    if(BankTabStorage != NULL)
    {
        stringstream GuildBankLogs;
        GuildBankLogs << "REPLACE INTO `guild_banklogs` VALUES";
        for(uint32 t = 0; t < 6; t++)
        {
            if(BankTabStorage->m_TabLogs[t] == NULL)
                continue;

            for(std::list<GuildBankEvent*>::iterator itr = BankTabStorage->m_TabLogs[t]->m_logs.begin(); itr != BankTabStorage->m_TabLogs[t]->m_logs.end(); itr++)
            {
                if(first)
                    first = false;
                else
                    GuildBankLogs << ", ";
                GuildBankLogs << "('" << (*itr)->iLogId << "', '" << GuildId << "', '" << uint32(t) << "', '" << uint32((*itr)->iAction);
                GuildBankLogs << "', '" << (*itr)->uPlayer << "', '" << (*itr)->uEntry << "', '" << uint32((*itr)->iStack) << "', '";
                GuildBankLogs << (*itr)->uTimeStamp << "')";
                count++;
            }
        }

        for(std::list<GuildBankEvent*>::iterator itr = BankTabStorage->m_money_logs.begin(); itr != BankTabStorage->m_money_logs.end(); itr++)
        {
            if(first)
                first = false;
            else
                GuildBankLogs << ", ";
            GuildBankLogs << "('" << (*itr)->iLogId << "', '" << GuildId << "', '" << uint32(6) << "', '" << uint32((*itr)->iAction);
            GuildBankLogs << "', '" << (*itr)->uPlayer << "', '" << (*itr)->uEntry << "', '" << uint32((*itr)->iStack) << "', '";
            GuildBankLogs << (*itr)->uTimeStamp << "')";
            count++;
        }

        if(count)
        {
            GuildBankLogs << ";";
            QMGR_EXECUTE(GuildBankLogs.str().c_str());
        }
    }

    count = 0;
    first = true;

    if(BankTabStorage != NULL)
    {
        stringstream GuildBankTabs;
        GuildBankTabs << "REPLACE INTO `guild_banktabs` VALUES";
        for(uint32 t = 0; t < 6; t++)
        {
            if(BankTabStorage->m_Tabs[t] == NULL)
                continue;

            if(first)
                first = false;
            else
                GuildBankTabs << ", ";
            GuildBankTabs << "('" << GuildId << "', '" << uint32(BankTabStorage->m_Tabs[t]->iTabId) << "', '" << CharacterDatabase.EscapeString(BankTabStorage->m_Tabs[t]->szTabName).c_str();
            GuildBankTabs << "', '" << CharacterDatabase.EscapeString(BankTabStorage->m_Tabs[t]->szTabIcon).c_str() << "', '" << CharacterDatabase.EscapeString(BankTabStorage->m_Tabs[t]->szTabInfo).c_str() << "')";
            count++;
        }

        if(count)
        {
            GuildBankTabs << ";";
            QMGR_EXECUTE(GuildBankTabs.str().c_str());
        }

        count = 0;
        first = true;
        stringstream GuildBankItems;

        // Step 1: Clean out any items that were removed.
        GuildBankItems << format("DELETE FROM `guild_bankitems` WHERE `guildId` = '%u' AND `itemGuid` NOT IN(", GuildId).c_str();
        for(uint32 t = 0; t < 6; t++)
        {
            if(BankTabStorage->m_Tabs[t] == NULL)
                continue;

            for(uint32 s = 0; s < MAX_GUILD_BANK_SLOTS; s++)
            {
                if(BankTabStorage->m_Tabs[t]->pSlots[s] == NULL)
                    continue;

                if(first)
                    first = false;
                else
                    GuildBankItems << ", ";
                GuildBankItems << "'" << BankTabStorage->m_Tabs[t]->pSlots[s]->GetLowGUID() << "'";
                count++;
            }
        }

        if(count)
        {
            GuildBankItems << ");";
            QMGR_EXECUTE(GuildBankItems.str().c_str());
        }
        else
            QMGR_EXECUTE(format("DELETE FROM `guild_bankitems` WHERE `guildId` = '%u'", GuildId).c_str());

        // Step 2: Empty any slots that aren't used.
        for(uint32 t = 0; t < 6; t++)
        {
            if(BankTabStorage->m_Tabs[t] == NULL)
                continue;

            count = 0;
            first = true;
            stringstream GuildBankItemsSlot;
            GuildBankItemsSlot << format("DELETE FROM `guild_bankitems` WHERE `guildId` = '%u' AND `tabId` = '%u' AND `slotId` NOT IN(", GuildId, t).c_str();
            for(uint32 s = 0; s < MAX_GUILD_BANK_SLOTS; s++)
            {
                if(BankTabStorage->m_Tabs[t]->pSlots[s] == NULL)
                    continue;

                if(first)
                    first = false;
                else
                    GuildBankItemsSlot << ", ";
                GuildBankItemsSlot << "'" << uint32(s) << "'";
                count++;
            }

            if(count)
            {
                GuildBankItemsSlot << ");";
                QMGR_EXECUTE(GuildBankItemsSlot.str().c_str());
            }
        }

        count = 0;
        first = true;
        stringstream GuildBankItemsBuild;

        // Step 3: Replace any existing values with the newer ones.
        GuildBankItemsBuild << "REPLACE INTO `guild_bankitems` VALUES";
        for(uint32 t = 0; t < 6; t++)
        {
            if(BankTabStorage->m_Tabs[t] == NULL)
                continue;

            for(uint32 s = 0; s < MAX_GUILD_BANK_SLOTS; s++)
            {
                if(BankTabStorage->m_Tabs[t]->pSlots[s] == NULL)
                    continue;

                if(first)
                    first = false;
                else
                    GuildBankItemsBuild << ", ";
                GuildBankItemsBuild << "('" << GuildId << "', '" << uint32(t) << "', '" << uint32(s) << "', '" << BankTabStorage->m_Tabs[t]->pSlots[s]->GetLowGUID() << "')";
                count++;
            }
        }

        if(count)
        {
            GuildBankItemsBuild << ";";
            QMGR_EXECUTE(GuildBankItemsBuild.str().c_str());
        }
    }
    else
    {
        QMGR_EXECUTE("DELETE FROM `guild_banktabs` WHERE `guildid` = '%u';", GuildId);
        QMGR_EXECUTE("DELETE FROM `guild_bankitems` WHERE `guildid` = '%u';", GuildId);
    }

#ifndef MASS_SAVE_QUERYBUFFER
    CharacterDatabase.AddQueryBuffer(qb);
#endif
}

void GuildMgr::CreateGuildFromCharter(Charter* charter)
{
    m_GuildLocks.Acquire();
    GuildInfo* gInfo = new GuildInfo();
    gInfo->m_GuildLock.Acquire();
    gInfo->m_guildId = guildmgr.GenerateGuildId();
    gInfo->m_guildName = charter->GuildName;
    gInfo->m_guildLeader = charter->LeaderGuid;
    gInfo->m_creationTimeStamp = (uint32)UNIXTIME;
    gInfo->m_emblemStyle = 0;
    gInfo->m_emblemColor = 0;
    gInfo->m_borderStyle = 0;
    gInfo->m_borderColor = 0;
    gInfo->m_backgroundColor = 0;
    gInfo->m_bankBalance = 0;
    gInfo->m_guildInfo = "";
    gInfo->m_motd = "";
    gInfo->m_GuildStatus = GUILD_STATUS_NEW;
    m_Guilds.insert(make_pair(gInfo->m_guildId, gInfo));
    m_GuildNames.insert(make_pair(gInfo->m_guildName, gInfo));
    gInfo->m_GuildLock.Release();

    // rest of the fields have been nulled out, create some default ranks.
    GuildRankStorage* RankStorage = ConstructRankStorage(gInfo->m_guildId);
    m_GuildMemberMaps.insert(make_pair(gInfo->m_guildId, new GuildMemberMapStorage(gInfo->m_guildId)));
    m_GuildLogs.insert(make_pair(gInfo->m_guildId, new GuildLogStorage(gInfo->m_guildId)));
    m_GuildTabs.insert(make_pair(gInfo->m_guildId, new GuildBankTabStorage(gInfo->m_guildId)));

    // turn off command logging, we don't wanna spam the logs
    gInfo->m_commandLogging = false;

    // add the leader to the guild
    PlayerInfo* pi = objmgr.GetPlayerInfo(charter->GetLeader());
    AddGuildMember(gInfo, pi, NULL, 0);

    // add all the other people
    for(uint32 i = 0; i < charter->SignatureCount; ++i)
    {
        pi = objmgr.GetPlayerInfo(charter->Signatures[i]);
        if(pi)
            AddGuildMember(gInfo, pi, NULL, RankStorage->ssid-1);
    }

    // re-enable command logging
    gInfo->m_commandLogging = true;
    SaveGuild(NULL, gInfo);
    m_GuildLocks.Release();
}

void GuildMgr::CreateGuildFromCommand(string name, uint32 gLeader)
{
    m_GuildLocks.Acquire();
    GuildInfo* gInfo = new GuildInfo();
    gInfo->m_GuildLock.Acquire();
    gInfo->m_guildId = guildmgr.GenerateGuildId();
    gInfo->m_guildName = name;
    gInfo->m_guildLeader = gLeader;
    gInfo->m_creationTimeStamp = (uint32)UNIXTIME;
    gInfo->m_emblemStyle = 0;
    gInfo->m_emblemColor = 0;
    gInfo->m_borderStyle = 0;
    gInfo->m_borderColor = 0;
    gInfo->m_backgroundColor = 0;
    gInfo->m_bankBalance = 0;
    gInfo->m_guildInfo = "";
    gInfo->m_motd = "";
    gInfo->m_commandLogging = true;
    gInfo->m_GuildStatus = GUILD_STATUS_NEW;
    m_Guilds.insert(make_pair(gInfo->m_guildId, gInfo));
    m_GuildNames.insert(make_pair(gInfo->m_guildName, gInfo));
    gInfo->m_GuildLock.Release();

    // rest of the fields have been nulled out, create some default ranks.
    ConstructRankStorage(gInfo->m_guildId);
    m_GuildMemberMaps.insert(make_pair(gInfo->m_guildId, new GuildMemberMapStorage(gInfo->m_guildId)));
    m_GuildLogs.insert(make_pair(gInfo->m_guildId, new GuildLogStorage(gInfo->m_guildId)));
    m_GuildTabs.insert(make_pair(gInfo->m_guildId, new GuildBankTabStorage(gInfo->m_guildId)));

    // add the leader to the guild
    PlayerInfo* pi = objmgr.GetPlayerInfo(gLeader);
    AddGuildMember(gInfo, pi, NULL, 0);

    SaveGuild(NULL, gInfo);
    m_GuildLocks.Release();
}

void GuildMgr::DestroyGuild(GuildInfo* guildInfo)
{
    uint32 GuildId = guildInfo->m_guildId;

    // Ranks.
    GuildRankStorage* RankStorage = m_GuildRanks.at(GuildId);
    if(RankStorage != NULL)
    {
        m_RankLocks.Acquire();
        delete RankStorage;
        RankStorage = NULL;
        m_RankLocks.Release();
    }
    RankStorage = NULL;
    m_GuildRanks.erase(GuildId);

    // Members
    GuildMemberMapStorage* MemberMapStorage = m_GuildMemberMaps.at(GuildId);
    if(MemberMapStorage != NULL)
    {
        GuildMember* GMember = NULL;
        MemberMapStorage->MemberMapLock.Acquire();
        for(GuildMemberMap::iterator itr = MemberMapStorage->MemberMap.begin(), itr2; itr != MemberMapStorage->MemberMap.end(); itr++)
        {
            GMember = NULL;
            if((itr2 = m_GuildMembers.find(itr->first)) != m_GuildMembers.end())
            {
                GMember = itr2->second;
                m_GuildMembers.erase(itr2);
            }
            if(GMember != NULL)
                delete GMember;
        }

        MemberMapStorage->MemberMap.clear();
        MemberMapStorage->MemberMapLock.Release();
        delete MemberMapStorage;
        MemberMapStorage = NULL;
    }
    MemberMapStorage = NULL;
    m_GuildMemberMaps.erase(GuildId);

    // Logs
    GuildLogStorage *LogStorage = m_GuildLogs.at(GuildId);
    if(LogStorage != NULL)
        delete LogStorage;
    LogStorage = NULL;
    m_GuildLogs.erase(GuildId);

    // BankTabs
    GuildBankTabStorage *BankTabStorage = m_GuildTabs.at(GuildId);
    if(BankTabStorage != NULL)
        delete BankTabStorage;
    BankTabStorage = NULL;
    m_GuildTabs.erase(GuildId);

    guildInfo->m_GuildLock.Release();
    delete guildInfo;
    guildInfo = NULL;
}

void GuildMgr::SaveAllGuilds()
{
    sLog.Debug("GuildMgr", "Saving all guilds...");

#ifdef MASS_SAVE_QUERYBUFFER
    QueryBuffer* qb = NULL;
#endif

    GuildInfo* gInfo = NULL;
    for(GuildInfoMap::iterator itr = m_Guilds.begin(), itr2; itr != m_Guilds.end();)
    {
        itr2 = itr++;
        gInfo = itr2->second;
        if(gInfo == NULL) // ??
        {
            itr2->second = NULL;
            m_Guilds.erase(itr2->first);
            continue;
        }

        gInfo->m_GuildLock.Acquire();
        if(!bServerShutdown)
        {
            if(gInfo->m_GuildStatus == GUILD_STATUS_NORMAL)
            {
                gInfo->m_GuildLock.Release();
                continue;
            }

            if(gInfo->m_GuildStatus == GUILD_STATUS_JUST_LOADED)
            {
                gInfo->m_GuildStatus = GUILD_STATUS_NORMAL;
                gInfo->m_GuildLock.Release();
                continue;
            }
        }

        if(gInfo->m_GuildStatus == GUILD_STATUS_DISBANDED)
        {
            itr2->second = NULL;
            m_Guilds.erase(itr2->first);
            m_GuildNames.erase(gInfo->m_guildName);
            DestroyGuild(gInfo);
            continue;
        }

        if(gInfo->m_GuildStatus == GUILD_STATUS_DIRTY || gInfo->m_GuildStatus == GUILD_STATUS_NEW || bServerShutdown)
        {
            sLog.Debug("GuildMgr", "Saving Guild %s", gInfo->m_guildName.c_str());
#ifdef MASS_SAVE_QUERYBUFFER
            SaveGuild(qb, gInfo);
#else
            SaveGuild(NULL, gInfo);
#endif
            gInfo->m_GuildLock.Release();
        }
        gInfo = NULL;
    }

#ifdef MASS_SAVE_QUERYBUFFER
    if(qb != NULL)
        CharacterDatabase.AddQueryBuffer(qb);
#endif
}

GuildRank* GuildMgr::FindHighestRank(GuildRankStorage* Ranks)
{
    Ranks->RankLock.Acquire();
    for(uint32 i = 1; i < MAX_GUILD_RANKS; ++i)
    {
        if(Ranks->m_ranks[i] != NULL)
        {
            Ranks->RankLock.Release();
            return Ranks->m_ranks[i];
        }
    }

    Ranks->RankLock.Release();
    return NULL;
}

GuildRank* GuildMgr::FindLowestRank(GuildRankStorage* Ranks)
{
    uint32 i = Ranks->ssid;
    Ranks->RankLock.Acquire();
    while(i > 0 && Ranks->m_ranks[i] == NULL)
        i--;
    Ranks->RankLock.Release();
    return Ranks->m_ranks[i];
}

GuildRankStorage* GuildMgr::ConstructRankStorage(uint32 GuildId)
{
    m_RankLocks.Acquire();
    GuildRankStorage* storage = new GuildRankStorage(GuildId);
    CreateGuildRank(storage, "Guild Master", GR_RIGHT_ALL, true);
    CreateGuildRank(storage, "Officer", GR_RIGHT_ALL, true);
    CreateGuildRank(storage, "Veteran", GR_RIGHT_DEFAULT, false);
    CreateGuildRank(storage, "Member", GR_RIGHT_DEFAULT, false);
    CreateGuildRank(storage, "Initiate", GR_RIGHT_DEFAULT, false);
    m_GuildRanks.insert(make_pair(GuildId, storage));
    m_RankLocks.Release();
    return storage;
}

uint32 GuildMgr::CalculateAllowedItemWithdraws(GuildMember* gMember, uint32 tab)
{
    if(gMember->pRank->iTabPermissions[tab].iStacksPerDay == -1)        // Unlimited
        return 0xFFFFFFFF;
    if(gMember->pRank->iTabPermissions[tab].iStacksPerDay == 0)     // none
        return 0;

    if((UNIXTIME - gMember->uLastItemWithdrawReset[tab]) >= TIME_DAY)
        return gMember->pRank->iTabPermissions[tab].iStacksPerDay;
    else
        return (gMember->pRank->iTabPermissions[tab].iStacksPerDay - gMember->uItemWithdrawlsSinceLastReset[tab]);
}

uint32 GuildMgr::CalculateAvailableAmount(GuildMember* gMember)
{
    if(gMember->pRank->iGoldLimitPerDay == -1)      // Unlimited
        return 0xFFFFFFFF;

    if(gMember->pRank->iGoldLimitPerDay == 0)
        return 0;

    if((UNIXTIME - gMember->uLastWithdrawReset) >= TIME_DAY)
        return gMember->pRank->iGoldLimitPerDay;
    else
        return (gMember->pRank->iGoldLimitPerDay - gMember->uWithdrawlsSinceLastReset);
}

void GuildMgr::OnMoneyWithdraw(GuildMember* gMember, uint32 amount)
{
    if(gMember->pRank->iGoldLimitPerDay <= 0)       // Unlimited
        return;

    // reset the counter if a day has passed
    if(((uint32)UNIXTIME - gMember->uLastWithdrawReset) >= TIME_DAY)
        gMember->uLastWithdrawReset = (uint32)UNIXTIME;
    gMember->uWithdrawlsSinceLastReset = amount;
}

void GuildMgr::OnItemWithdraw(GuildMember* gMember, uint32 tab)
{
    if(gMember->pRank->iTabPermissions[tab].iStacksPerDay <= 0)     // Unlimited
        return;

    // reset the counter if a day has passed
    if(((uint32)UNIXTIME - gMember->uLastItemWithdrawReset[tab]) >= TIME_DAY)
    {
        gMember->uLastItemWithdrawReset[tab] = (uint32)UNIXTIME;
        gMember->uItemWithdrawlsSinceLastReset[tab] = 1;
    }
    else
    {
        // increment counter
        gMember->uItemWithdrawlsSinceLastReset[tab]++;
    }
}

bool GuildMgr::HasGuildRights(Player* plr, uint32 Rights)
{
    uint32 GuildId = plr->GetGuildId();
    if(!GuildId)
        return false;

    GuildRankStorage* storage = GetGuildRankStorage(GuildId);
    if(storage == NULL)
        return false;

    uint8 rank = plr->GetGuildRank();
    if(rank >= MAX_GUILD_RANKS)
        return false;

    if(storage->m_ranks[rank]->iRights & Rights)
        return true;
    return false;
}

bool GuildMgr::HasGuildBankRights(Player* plr, uint8 tabId, uint32 Rights)
{
    uint32 GuildId = plr->GetGuildId();
    if(!GuildId)
        return false;

    GuildRankStorage* storage = GetGuildRankStorage(GuildId);
    if(storage == NULL)
        return false;

    uint8 rank = plr->GetGuildRank();
    if(rank >= MAX_GUILD_RANKS)
        return false;

    if(storage->m_ranks[rank]->iTabPermissions[tabId].iFlags & Rights)
        return true;
    return false;
}

void GuildMgr::CreateGuildRank(GuildRankStorage* storage, const char* szRankName, uint32 iPermissions, bool bFullGuildBankPermissions)
{
    // find a free id
    GuildInfo* gInfo = GetGuildInfo(storage->GuildId);
    storage->RankLock.Acquire();
    for(uint32 i = storage->ssid; i < MAX_GUILD_RANKS; i++)
    {
        if(storage->m_ranks[i] == NULL)
        {
            gInfo->m_GuildLock.Acquire();
            storage->ssid = i+1;
            storage->m_ranks[i] = new GuildRank(i, iPermissions, szRankName, bFullGuildBankPermissions);
            storage->RankLock.Release();
            gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
            gInfo->m_GuildLock.Release();
//          sLog.Notice("Guild", "Created rank %u on guild %u (%s)", i, storage->GuildId, szRankName);
            return;
        }
    }
    sLog.Error("GuildMgr", "Failed to create rank %s", szRankName);
    storage->RankLock.Release();
}

uint32 GuildMgr::RemoveGuildRank(uint32 GuildId)
{
    GuildInfo* gInfo = GetGuildInfo(GuildId);
    GuildRankStorage* storage = GetGuildRankStorage(GuildId);
    storage->RankLock.Acquire();
    GuildRank* pLowestRank = FindLowestRank(storage);
    if(pLowestRank == NULL || pLowestRank->iId < 5)     // cannot delete default ranks.
    {
        storage->RankLock.Release();
        return 1;
    }

    if(m_GuildMemberMaps.find(GuildId) == m_GuildMemberMaps.end())
        return 2; // No member list.

    // check for players that need to be promoted
    GuildMemberMapStorage* MemberMapStorage = m_GuildMemberMaps[GuildId];
    GuildMemberMap::iterator itr = MemberMapStorage->MemberMap.begin();
    MemberMapStorage->MemberMapLock.Acquire();
    for(; itr != MemberMapStorage->MemberMap.end(); ++itr)
    {
        if(itr->second->pRank == pLowestRank)
        {
            MemberMapStorage->MemberMapLock.Release();
            storage->RankLock.Release();
            return 3;
        }
    }
    MemberMapStorage->MemberMapLock.Release();

    gInfo->m_GuildLock.Acquire();
    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    storage->ssid--; // Decremention.
    storage->m_ranks[pLowestRank->iId] = NULL;
    delete pLowestRank;
    gInfo->m_GuildLock.Release();
    storage->RankLock.Release();
    return 0;
}

bool GuildMgr::Disband(uint32 GuildId)
{
    GuildInfo* gInfo = GetGuildInfo(GuildId);
    if(gInfo == NULL)
        return false;

    gInfo->m_commandLogging = false;
    GuildMemberMapStorage* MemberMapStorage = GetGuildMemberMapStorage(gInfo->m_guildId);
    if(MemberMapStorage != NULL)
    {
        for(GuildMemberMap::iterator itr = MemberMapStorage->MemberMap.begin(), itr2; itr != MemberMapStorage->MemberMap.end();)
        {
            itr2 = itr++;
            guildmgr.ForceRemoveMember(NULL, itr2->second->pPlayer);
        }
    }
    gInfo->m_GuildStatus = GUILD_STATUS_DISBANDED;
    AddDestructionQueries(GuildId);
    return true;
}

void GuildMgr::PlayerLoggedIn(PlayerInfo* plr)
{
    SendMotd(plr);
    LogGuildEvent(NULL, plr->GuildId, GUILD_EVENT_HASCOMEONLINE, plr->name);
}

void GuildMgr::PlayerLoggedOff(PlayerInfo* plr)
{
    LogGuildEvent(NULL, plr->GuildId, GUILD_EVENT_HASGONEOFFLINE, plr->name);
}

void GuildMgr::SendMotd(PlayerInfo* plr, uint32 guildid)
{
    if(plr != NULL)
        guildid = plr->GuildId;
    if(guildid < 1)
        return;
    GuildInfo* gInfo = GetGuildInfo(guildid);
    if(gInfo == NULL)
        return;
    if(!gInfo->m_commandLogging)
        return;

    Player* player = ((plr != NULL) ? plr->m_loggedInPlayer : NULL);
    LogGuildEvent(player, ((player != NULL) ? 0 : guildid), GUILD_EVENT_MOTD, gInfo->m_motd.c_str());
}

void GuildMgr::RemoveMember(Player* remover, PlayerInfo* removee)
{
    if(remover && remover->GetGuildId() != removee->GuildId)
        return;

    uint32 GuildId = removee->GuildId;
    GuildInfo* gInfo = GetGuildInfo(GuildId);
    if(gInfo == NULL)
        return;

    if(remover)
    {
        int RDiff = remover->GetGuildRank() - removee->GuildRank;

        if((!HasGuildRights(remover, GR_RIGHT_REMOVE) && remover->getPlayerInfo() != removee)
            || (RDiff >= 0 && remover->getPlayerInfo() != removee))
        {
            SendGuildCommandResult(remover->GetSession(), GUILD_MEMBER_S, "", GUILD_PERMISSIONS);
            return;
        }
    }

    if(removee->GuildRank == 0)
    {
        if(remover)
            remover->GetSession()->SystemMessage("You cannot remove the guild master.");
        return;
    }

    GuildMember* ToRemove = NULL;
    GuildMemberMap::iterator itr = m_GuildMembers.find(removee->guid);
    if(itr != m_GuildMembers.end())
    {
        ToRemove = itr->second;
        m_GuildMembers.erase(itr);
    }

    GuildMemberMapStorage* MemberMapStorage = GetGuildMemberMapStorage(GuildId);
    if(MemberMapStorage != NULL)
    {
        MemberMapStorage->MemberMapLock.Acquire();
        GuildMemberMap::iterator itr = MemberMapStorage->MemberMap.find(removee->guid);
        if(itr != MemberMapStorage->MemberMap.end())
            MemberMapStorage->MemberMap.erase(itr);
        MemberMapStorage->MemberMapLock.Release();
    }

    if(ToRemove != NULL)
    {
        delete ToRemove;
        ToRemove = NULL;
    }

    if(remover && remover->getPlayerInfo() != removee)
    {
        if(removee->m_loggedInPlayer)
        {
            Player* plr = objmgr.GetPlayer(removee->guid);
            if(plr)
                sChatHandler.SystemMessageToPlr(plr, "You have been kicked from the guild by %s", remover->GetName());
        }

        LogGuildEvent(NULL, GuildId, GUILD_EVENT_REMOVED, removee->name, remover->GetName());
        AddGuildLogEntry(GuildId, GUILD_LOG_EVENT_REMOVAL, remover->GetLowGUID(), removee->guid);
    }
    else
    {
        LogGuildEvent(NULL, GuildId, GUILD_EVENT_LEFT, removee->name);
        AddGuildLogEntry(GuildId, GUILD_LOG_EVENT_LEFT, removee->guid);
    }

    removee->GuildId = 0;
    removee->GuildRank = 0;
    if(removee->m_loggedInPlayer)
    {
        removee->m_loggedInPlayer->SetGuildRank(0);
        removee->m_loggedInPlayer->SetGuildId(0);
    }

    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
}

void GuildMgr::ForceRemoveMember(Player* remover, PlayerInfo* removee)
{
    uint32 GuildId = removee->GuildId;
    GuildInfo* gInfo = GetGuildInfo(GuildId);
    if(gInfo == NULL)
        return;

    GuildMember* ToRemove = NULL;
    GuildMemberMap::iterator itr = m_GuildMembers.find(removee->guid);
    if(itr != m_GuildMembers.end())
    {
        ToRemove = itr->second;
        m_GuildMembers.erase(itr);
    }

    GuildMemberMapStorage* MemberMapStorage = GetGuildMemberMapStorage(GuildId);
    if(MemberMapStorage != NULL)
    {
        MemberMapStorage->MemberMapLock.Acquire();
        GuildMemberMap::iterator itr = MemberMapStorage->MemberMap.find(removee->guid);
        if(itr != MemberMapStorage->MemberMap.end())
            MemberMapStorage->MemberMap.erase(itr);
        MemberMapStorage->MemberMapLock.Release();
    }

    if(ToRemove != NULL)
    {
        delete ToRemove;
        ToRemove = NULL;
    }

    if(remover && remover->getPlayerInfo() != removee)
    {
        if(removee->m_loggedInPlayer)
        {
            Player* plr = objmgr.GetPlayer(removee->guid);
            if(plr)
                sChatHandler.SystemMessageToPlr(plr, "You have been kicked from the guild by %s", remover->GetName());
        }

        LogGuildEvent(NULL, GuildId, GUILD_EVENT_REMOVED, removee->name, remover->GetName());
        AddGuildLogEntry(GuildId, GUILD_LOG_EVENT_REMOVAL, remover->GetLowGUID(), removee->guid);
    }
    else
    {
        LogGuildEvent(NULL, GuildId, GUILD_EVENT_LEFT, removee->name);
        AddGuildLogEntry(GuildId, GUILD_LOG_EVENT_LEFT, removee->guid);
    }

    removee->GuildId = 0;
    removee->GuildRank = 0;
    if(removee->m_loggedInPlayer)
    {
        removee->m_loggedInPlayer->SetGuildRank(0);
        removee->m_loggedInPlayer->SetGuildId(0);
    }

    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
}

// adding a member
void GuildMgr::AddGuildMember(GuildInfo* gInfo, PlayerInfo* newmember, WorldSession* pClient, int32 ForcedRank /* = -1 */)
{
    //we don't need useless paranoia checks.
    if(newmember->GuildId != 0)
        return;

    GuildRankStorage* RankStorage = GetGuildRankStorage(gInfo->m_guildId);
    RankStorage->RankLock.Acquire();
    GuildRank* r = NULL;
    if(ForcedRank >= 0)
    {
        r = RankStorage->m_ranks[ForcedRank];
        RankStorage->RankLock.Release();
    }
    else if(ForcedRank == -2)
    {
        RankStorage->RankLock.Release();
        r = FindHighestRank(RankStorage);
    }
    else
    {
        RankStorage->RankLock.Release();
        r = FindLowestRank(RankStorage);
    }

    if(r == NULL)
        r = FindLowestRank(RankStorage);

    if(r == NULL) // shouldn't happen
        return;
    uint32 rank = r->iId;

    GuildMember* pm = new GuildMember(newmember->guid, newmember, r);
    GuildMemberMapStorage* MemberList = GetGuildMemberMapStorage(gInfo->m_guildId);
    m_GuildMembers.insert(make_pair(pm->PlrGuid, pm));
    MemberList->MemberMapLock.Acquire();
    MemberList->MemberMap.insert(make_pair(pm->PlrGuid, pm));
    MemberList->MemberMapLock.Release();

    newmember->GuildRank = rank;
    newmember->GuildId = gInfo->m_guildId;

    if(newmember->m_loggedInPlayer)
    {
        newmember->m_loggedInPlayer->SetGuildId(gInfo->m_guildId);
        newmember->m_loggedInPlayer->SetGuildRank(r->iId);
        SendMotd(newmember);
    }

    gInfo->m_GuildStatus = GUILD_STATUS_DIRTY;
    LogGuildEvent(NULL, gInfo->m_guildId, GUILD_EVENT_JOINED, newmember->name);
    AddGuildLogEntry(gInfo->m_guildId, GUILD_LOG_EVENT_JOIN, newmember->guid);
}

void GuildMgr::GuildChat(WorldSession* m_session, uint32 Language, const char* message)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildMemberMapStorage* MemberMapStorage = GetGuildMemberMapStorage(plr->GetGuildId());
    if(MemberMapStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(!HasGuildRights(plr, GR_RIGHT_GCHATSPEAK))
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PERMISSIONS);
        return;
    }

    Player* Target = NULL;
    WorldPacket * data = sChatHandler.FillMessageData( CHAT_MSG_GUILD, LANG_UNIVERSAL, message, plr->GetGUID(), plr->GetChatTag());
    MemberMapStorage->MemberMapLock.Acquire();
    for(GuildMemberMap::iterator itr = MemberMapStorage->MemberMap.begin(); itr != MemberMapStorage->MemberMap.end(); itr++)
    {
        Target = itr->second->pPlayer->m_loggedInPlayer;
        if(Target == NULL)
            continue;

        if(HasGuildRights(Target, GR_RIGHT_GCHATLISTEN))
            Target->SendPacket(data);
    }
    MemberMapStorage->MemberMapLock.Release();

    delete data;
}

void GuildMgr::OfficerChat(WorldSession* m_session, uint32 Language, const char* message)
{
    Player* plr = m_session->GetPlayer();
    if(!plr->IsInGuild())
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    GuildMemberMapStorage* MemberMapStorage = GetGuildMemberMapStorage(plr->GetGuildId());
    if(MemberMapStorage == NULL)
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if(!HasGuildRights(plr, GR_RIGHT_OFFCHATSPEAK))
    {
        SendGuildCommandResult(m_session, GUILD_MEMBER_S, "", GUILD_PERMISSIONS);
        return;
    }

    Player* Target = NULL;
    WorldPacket * data = sChatHandler.FillMessageData( CHAT_MSG_OFFICER, Language, message, plr->GetGUID(), plr->GetChatTag());
    MemberMapStorage->MemberMapLock.Acquire();
    for(GuildMemberMap::iterator itr = MemberMapStorage->MemberMap.begin(); itr != MemberMapStorage->MemberMap.end(); itr++)
    {
        Target = itr->second->pPlayer->m_loggedInPlayer;
        if(Target == NULL)
            continue;

        if(HasGuildRights(Target, GR_RIGHT_OFFCHATLISTEN))
            Target->SendPacket(data);
    }
    MemberMapStorage->MemberMapLock.Release();

    delete data;
}

void GuildMgr::LoadGuildCharters()
{
    m_hiCharterId = 0;
    QueryResult * result = CharacterDatabase.Query("SELECT * FROM charters");
    if(result == NULL)
        return;

    do
    {
        Charter * c = new Charter(result->Fetch());
        m_charters[c->CharterType].insert(make_pair(c->GetID(), c));
        if(c->GetID() > m_hiCharterId)
            m_hiCharterId = c->GetID();
    } while(result->NextRow());
    delete result;
}

Charter * GuildMgr::GetCharterByItemGuid(uint32 guid)
{
    m_charterLock.AcquireReadLock();
    for(int i = 0; i < NUM_CHARTER_TYPES; i++)
    {
        map<uint32, Charter*>::iterator itr = m_charters[i].begin();
        for(; itr != m_charters[i].end(); itr++)
        {
            if(itr->second->ItemGuid == guid)
            {
                m_charterLock.ReleaseReadLock();
                return itr->second;
            }
        }
    }
    m_charterLock.ReleaseReadLock();
    return NULL;
}

Charter * GuildMgr::GetCharterByGuid(uint64 playerguid, CharterTypes type)
{
    m_charterLock.AcquireReadLock();
    for(int i = 0; i < NUM_CHARTER_TYPES; i++)
    {
        map<uint32, Charter*>::iterator itr = m_charters[i].begin();
        for(; itr != m_charters[i].end(); itr++)
        {
            if(playerguid == itr->second->LeaderGuid)
            {
                m_charterLock.ReleaseReadLock();
                return itr->second;
            }

            for(uint32 j = 0; j < itr->second->SignatureCount; ++j)
            {
                if(itr->second->Signatures[j] == playerguid)
                {
                    m_charterLock.ReleaseReadLock();
                    return itr->second;
                }
            }
        }
    }
    m_charterLock.ReleaseReadLock();
    return NULL;
}

Charter * GuildMgr::GetCharterByName(string &charter_name, CharterTypes Type)
{
    Charter * rv = 0;
    m_charterLock.AcquireReadLock();
    map<uint32, Charter*>::iterator itr = m_charters[Type].begin();
    for(; itr != m_charters[Type].end(); itr++)
    {
        if(itr->second->GuildName == charter_name)
        {
            rv = itr->second;
            break;
        }
    }

    m_charterLock.ReleaseReadLock();
    return rv;
}

void GuildMgr::RemoveCharter(Charter * c)
{
    if( c == NULL )
        return;
    if( c->CharterType > NUM_CHARTER_TYPES )
    {
        sLog.Notice("GuildMgr", "Charter %u cannot be destroyed as type %u is not a sane type value.", c->CharterId, c->CharterType );
        return;
    }

    m_charterLock.AcquireWriteLock();
    m_charters[c->CharterType].erase(c->CharterId);
    m_charterLock.ReleaseWriteLock();
}

Charter * GuildMgr::GetCharter(uint32 CharterId, CharterTypes Type)
{
    Charter * rv;
    map<uint32,Charter*>::iterator itr;
    m_charterLock.AcquireReadLock();
    itr = m_charters[Type].find(CharterId);
    rv = (itr == m_charters[Type].end()) ? 0 : itr->second;
    m_charterLock.ReleaseReadLock();
    return rv;
}

Charter * GuildMgr::CreateCharter(uint32 LeaderGuid, CharterTypes Type)
{
    m_charterLock.AcquireWriteLock();
    Charter * c = new Charter(++m_hiCharterId, LeaderGuid, Type);
    m_charters[c->CharterType].insert(make_pair(c->GetID(), c));
    m_charterLock.ReleaseWriteLock();
    return c;
}

Charter::Charter(Field * fields)
{
    uint32 f = 0;
    const char* temp = "";
    CharterId = fields[f++].GetUInt32();
    CharterType = fields[f++].GetUInt32();
    LeaderGuid = fields[f++].GetUInt32();
    GuildName = string(strlen(temp = fields[f++].GetString()) ? strdup(temp) : "");
    ItemGuid = fields[f++].GetUInt32();
    SignatureCount = 0;
    Slots = GetNumberOfSlotsByType();
    Signatures = new uint32[Slots];

    for(uint32 i = 0; i < Slots; i++)
    {
        Signatures[i] = fields[f++].GetUInt32();
        if(Signatures[i])
        {
            PlayerInfo * inf = objmgr.GetPlayerInfo(Signatures[i]);
            if( inf == NULL )
            {
                Signatures[i] = 0;
                continue;
            }

            inf->charterId[CharterType] = CharterId;
            ++SignatureCount;
        }
    }
}

void Charter::AddSignature(uint32 PlayerGuid)
{
    if(SignatureCount >= Slots)
        return;

    ++SignatureCount;
    uint32 i;
    for(i = 0; i < Slots; i++)
    {
        if(Signatures[i] == 0)
        {
            Signatures[i] = PlayerGuid;
            break;
        }
    }

    assert(i != Slots);
}

void Charter::RemoveSignature(uint32 PlayerGuid)
{
    for(uint32 i = 0; i < Slots; i++)
    {
        if(Signatures[i] == PlayerGuid)
        {
            Signatures[i] = 0;
            --SignatureCount;
            SaveToDB();
            break;
        }
    }
}


void Charter::Destroy()
{
    if( Slots == 0 )            // ugly hack because of f*cked memory
        return;

    //meh remove from objmgr
    guildmgr.RemoveCharter(this);

    // Kill the players with this (in db/offline)
    CharacterDatabase.Execute( "DELETE FROM charters WHERE charterId = %u", CharterId );
    PlayerInfo * p;
    for( uint32 i = 0; i < Slots; i++ )
    {
        if(!Signatures[i])
            continue;

        p = objmgr.GetPlayerInfo(Signatures[i]);
        if( p != NULL )
            p->charterId[CharterType] = 0;
    }

    // click, click, boom!
    // Crow: A Saliva reference?
    delete this;
}

void Charter::SaveToDB()
{
    std::stringstream ss;
    uint32 i;

    ss << "REPLACE INTO charters VALUES(" << CharterId << "," << CharterType << "," << LeaderGuid << ",'" << GuildName << "'," << ItemGuid;

    for(i = 0; i < Slots; i++)
        ss << "," << Signatures[i];

    for(; i < 9; i++)
        ss << ",0";

    ss << ")";
    CharacterDatabase.Execute(ss.str().c_str());
}
