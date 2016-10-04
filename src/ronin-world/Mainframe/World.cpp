/***
 * Demonstrike Core
 */

#include "StdAfx.h"

#define WORLD_UPDATE_DELAY 50

initialiseSingleton( World );

World::World() : m_eventHandler(NULL)
{
    SendServerData = 1;
    m_hashInfo = format("%s-%s-%s", RONIN_UTIL::TOUPPER_RETURN(BUILD_HASH_STR).c_str(), ARCH, CONFIG);

    m_StartTime = 0;
    m_playerLimit = 0;
    GmClientChannel = "";
    GuildsLoading = false;
    PeakSessionCount = 0;
    mInWorldPlayerCount = 0;
    mAcceptedConnections = 0;
    HordePlayers = 0;
    AlliancePlayers = 0;
    gm_force_robes = false;
    AHEnabled = true;
    IsPvPRealm = true;
    authSeed1.SetRand(16 * 8);
    authSeed2.SetRand(16 * 8);

    m_queueUpdateTimer = 180000;
    m_pushUpdateTimer = 0;

#ifdef WIN32
    m_bFirstTime = true;
    m_lnOldValue = 0;
    memset( &m_OldPerfTime100nSec, 0, sizeof( m_OldPerfTime100nSec ) );
    SYSTEM_INFO si;
    GetSystemInfo( &si );
    number_of_cpus = si.dwNumberOfProcessors;
    m_current_holiday_mask = 0;
#endif // WIN32
}

bool World::run()
{
    uint32 counter = 0, mstime = getMSTime(), lastUpdate = mstime; // Get our ms time
    while(GetThreadState() != THREADSTATE_TERMINATE)
    {
        // Provision for pausing this thread.
        if(GetThreadState() == THREADSTATE_PAUSED)
            while(GetThreadState() == THREADSTATE_PAUSED)
                Delay(200);
        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        mstime = getMSTime();
        uint32 diff = std::min<uint32>(500, mstime - lastUpdate);
        lastUpdate = mstime;

        Update( diff );
        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;

        diff = getMSTime()-lastUpdate;
        if(diff<WORLD_UPDATE_DELAY)
            Delay(WORLD_UPDATE_DELAY-diff);
        else sLog.outDebug("World thread running outside of update period");
    }

    return false;
}

uint32 World::GetMaxLevel(Player* plr)
{
    if(LevelCap_Custom_All && LevelCap_Custom_All != MAXIMUM_CEXPANSION_LEVEL)
        return LevelCap_Custom_All;

    uint32 level = MAXIMUM_ATTAINABLE_LEVEL;
    if( plr->GetSession()->HasFlag(ACCOUNT_FLAG_XPACK_03) )
        level = MAXIMUM_CEXPANSION_LEVEL;
    else if( plr->GetSession()->HasFlag(ACCOUNT_FLAG_XPACK_02) )
        level = 80;
    else if( plr->GetSession()->HasFlag(ACCOUNT_FLAG_XPACK_01) )
        level = 70;
    else level = 60;// Classic World of Warcraft
    return level;
}

uint32 World::GetMaxLevelStatCalc()
{
    if(MaxLevelCalc && MaxLevelCalc < MAXIMUM_ATTAINABLE_LEVEL)
        return MaxLevelCalc;
    return MAXIMUM_ATTAINABLE_LEVEL;
}

bool World::CompressPacketData(z_stream *stream, const void *data, uint32 len, ByteBuffer *output)
{
    uint32 destSize = compressBound(len);
    ByteBuffer *buff;
    m_compressionLock.Acquire();
    if(m_compressionBuffers.size())
    {
        buff = m_compressionBuffers.front();
        m_compressionBuffers.pop_front();
        if(buff->size() < destSize)
            buff->resize(destSize);
    } else if(buff = new ByteBuffer())
        buff->resize(destSize);
    m_compressionLock.Release();

    // set up stream pointers
    stream->avail_in  = (uInt)len;
    stream->avail_out = (uInt)destSize;
    stream->next_in   = (Bytef*)data;
    stream->next_out  = (Bytef*)buff->contents();

    bool res = false;
    if(deflate(stream, Z_SYNC_FLUSH) == Z_OK)
    {
        if(stream->avail_in == 0)
        {
            res = true;
            destSize -= stream->avail_out;
            output->append(buff->contents(), destSize);
        } else sLog.outDebug("deflate failed: did not end stream");
    } else sLog.outDebug("deflate failed.");
    // Compression failed, readd the buffer
    m_compressionLock.Acquire();
    m_compressionBuffers.push_back(buff);
    m_compressionLock.Release();
    return res;
}

void World::LogoutPlayers()
{
    sLog.Notice("World", "Logging out players...");
    for(SessionMap::iterator i = m_sessions.begin(); i != m_sessions.end(); i++)
        (i->second)->LogoutPlayer();

    sLog.Notice("World", "Deleting sessions...");
    WorldSession * Session;
    for(SessionMap::iterator i=m_sessions.begin();i!=m_sessions.end();)
    {
        Session = i->second;
        ++i;

        DeleteGlobalSession(Session);
    }
}

World::~World()
{

}

void World::Destruct()
{
    while(m_restedAreas.size())
    {
        delete m_restedAreas.begin()->second;
        m_restedAreas.erase(m_restedAreas.begin());
    }

    sLog.Notice("AchievementMgr", "~AchievementMgr()");
    delete AchievementMgr::getSingletonPtr();

    sLog.Notice("AddonMgr", "~AddonMgr()");
    delete AddonMgr::getSingletonPtr();

    sLog.Notice("StatSystem", "~StatSystem()");
    delete StatSystem::getSingletonPtr();

    sLog.Notice("GossipManager", "~GossipManager()");
    delete GossipManager::getSingletonPtr();

    sLog.Notice("Tracker", "~Tracker()");
    delete Tracker::getSingletonPtr();

    sLog.Notice("SpellManager", "~SpellManager()");
    delete SpellManager::getSingletonPtr();

    sLog.Notice("TicketMgr", "~TicketMgr()");
    delete TicketMgr::getSingletonPtr();

    sLog.Notice("WarnSys", "~WarnSystem()");
    delete WarnSystem::getSingletonPtr();

    sLog.Notice("QuestMgr", "~QuestMgr()");
    delete QuestMgr::getSingletonPtr();

    sLog.Notice("ObjectMgr", "~ObjectMgr()");
    delete ObjectMgr::getSingletonPtr();

    sLog.Notice("GuildMgr", "~GuildMgr()");
    delete GuildMgr::getSingletonPtr();

    sLog.Notice("LootMgr", "~LootMgr()");
    delete LootMgr::getSingletonPtr();

    sLog.Notice("LfgMgr", "~LfgMgr()");
    delete LfgMgr::getSingletonPtr();

    sLog.Notice("ChannelMgr", "~ChannelMgr()");
    delete ChannelMgr::getSingletonPtr();

    sLog.Notice("WeatherMgr", "~WeatherMgr()");
    delete WeatherMgr::getSingletonPtr();

    sLog.Notice("TaxiMgr", "~TaxiMgr()");
    delete TaxiMgr::getSingletonPtr();

    sLog.Notice("ChatHandler", "~ChatHandler()");
    delete ChatHandler::getSingletonPtr();

    sLog.Notice("CBattlegroundManager", "~CBattlegroundManager()");
    delete CBattlegroundManager::getSingletonPtr();

    sLog.Notice("AuctionMgr", "~AuctionMgr()");
    delete AuctionMgr::getSingletonPtr();

    sLog.Notice("WorldStateTemplateManager", "~WorldStateTemplateManager()");
    delete WorldStateTemplateManager::getSingletonPtr();

    sLog.Notice("ItemManager", "~ItemManager()");
    delete ItemManager::getSingletonPtr();

    sLog.Notice("PacketHandler", "~PacketHandler()");
    WorldSession::DeInitPacketHandlerTable();

    sLog.Notice("WorldManager", "Destruct()");
    sWorldMgr.Shutdown();
    sWorldMgr.Destruct();

    sLog.Notice("CreatureDataMgr", "~CreatureDataMgr()");
    delete CreatureDataManager::getSingletonPtr();
    Storage_Cleanup();

    sVMapInterface.DeInit();
    sNavMeshInterface.DeInit();

    delete this;
}

WorldSession* World::FindSession(uint32 id)
{
    m_sessionlock.AcquireReadLock();
    WorldSession * ret = 0;
    SessionMap::const_iterator itr = m_sessions.find(id);

    if(itr != m_sessions.end())
        ret = m_sessions[id];

    m_sessionlock.ReleaseReadLock();

    return ret;
}

void World::RemoveSession(uint32 id)
{
    SessionMap::iterator itr = m_sessions.find(id);

    m_sessionlock.AcquireWriteLock();
    if(itr != m_sessions.end())
    {
        //If it's a GM, remove him from GM session map
        if(itr->second->HasGMPermissions())
        {
            gmList_lock.AcquireWriteLock();
            gmList.erase(itr->second);
            gmList_lock.ReleaseWriteLock();
        }
        delete itr->second;
        m_sessions.erase(itr);
    }
    m_sessionlock.ReleaseWriteLock();
}

void World::AddSession(WorldSession* s)
{
    ASSERT(s);

    //add this session to the big session map
    m_sessionlock.AcquireWriteLock();
    m_sessions[s->GetAccountId()] = s;
    m_sessionlock.ReleaseWriteLock();

    //check max online counter, update when necessary
    if(m_sessions.size() >  PeakSessionCount)
        PeakSessionCount = (uint32)m_sessions.size();

    //If it's a GM, add to GM session map
    if(s->HasGMPermissions())
    {
        gmList_lock.AcquireWriteLock();
        gmList.insert(s);
        gmList_lock.ReleaseWriteLock();
    }
}

void World::AddGlobalSession(WorldSession *GlobalSession)
{
    if(!GlobalSession)
        return;

    SessionsMutex.Acquire();
    GlobalSessions.insert(GlobalSession);
    SessionsMutex.Release();
}

void World::RemoveGlobalSession(WorldSession *GlobalSession)
{
    SessionsMutex.Acquire();
    GlobalSessions.erase(GlobalSession);
    SessionsMutex.Release();
}

bool BasicTaskExecutor::run()
{
    /* Set thread priority, this is a bitch for multiplatform :P */
#ifdef WIN32
    switch(priority)
    {
    case BTE_PRIORITY_LOW:
        ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_LOWEST );
        break;

    case BTW_PRIORITY_HIGH:
        ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
        break;

    default:        // BTW_PRIORITY_MED
        ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_NORMAL );
        break;
    }
#else
    struct sched_param param;
    switch(priority)
    {
    case BTE_PRIORITY_LOW:
        param.sched_priority = 0;
        break;

    case BTW_PRIORITY_HIGH:
        param.sched_priority = 10;
        break;

    default:        // BTW_PRIORITY_MED
        param.sched_priority = 5;
        break;
    }
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif

    // Execute the task in our new context.
    cb->execute();
#ifdef WIN32
    ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#else
    param.sched_priority = 5;
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif

    return true;
}

uint32 World::GetWeekDay()
{
    uint32 ret = 0;
    m_timeDataLock.Acquire();
    ret = m_currentTimeData.tm_wday;
    m_timeDataLock.Release();
    return ret;
}

bool World::SetInitialWorldSettings()
{
    CharacterDatabase.WaitExecute("UPDATE character_data SET online = 0 WHERE online = 1");

    sLog.Notice("World", "Starting up...");

    m_lastTick = UNIXTIME;

    // TODO: clean this
    localtime_s(&m_currentTimeData, &m_lastTick);
    m_gameTime = (3600*m_currentTimeData.tm_hour)+(60*m_currentTimeData.tm_min)+m_currentTimeData.tm_sec; // server starts at noon
    m_weekStart = m_lastTick-((86400*m_currentTimeData.tm_wday)+m_gameTime);
    uint32 start_time = getMSTime();

    new DBCLoader();

    // Fill the task list with jobs to do.
    TaskList tl; bool dbcResult = true;

    // Fill tasklist with load tasks
    sDBCLoader.FillDBCLoadList(tl, DBCPath.c_str(), &dbcResult);

    // spawn worker threads (2 * number of cpus)
    tl.spawn();

    // Wait for our DBCs to be finished loading
    tl.wait();

    if( !dbcResult)
    {
        sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "One or more of the DBC files are missing.", "These are absolutely necessary for the server to function.", "The server will not start without them.", NULL);
        return false;
    }

    sLog.Success("World", "DBC Files Loaded successfully");

    // Unload the DBC loader
    delete DBCLoader::getSingletonPtr();

    new StatSystem();
    if(!sStatSystem.Load())
    {
        sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "Unit stat system failed to construct.", "Server cannnot properly function without these stats.", "The server will not start because of this.", NULL);
        return false;
    }

    new AchievementMgr();
    new SpellManager();
    new ObjectMgr();
    new QuestMgr();
    new GossipManager();
    new TicketMgr();
    new LootMgr();
    new LfgMgr();
    new WeatherMgr();
    new TaxiMgr();
    new AddonMgr();
    new ChatHandler();
    new WarnSystem();
    new Tracker();
    new GuildMgr();
    new ItemManager();
    new CreatureDataManager();
    new WorldManager();

    Storage_FillTaskList(tl);

#define MAKE_TASK(sp, ptr) tl.AddTask(new Task(new CallbackP0<sp>(sp::getSingletonPtr(), &sp::ptr)))
    MAKE_TASK(TaxiMgr, Initialize);
    MAKE_TASK(ItemManager, InitializeItemPrototypes);
    MAKE_TASK(CreatureDataManager, LoadFromDB);
    MAKE_TASK(FactionSystem, LoadFactionInteractionData);
    MAKE_TASK(SpellManager, ParseSpellDBC);

    tl.wait(); // Load all the storage first
    MAKE_TASK(SpellManager, PoolSpellData);

    MAKE_TASK(QuestMgr, LoadQuests);
    MAKE_TASK(LootMgr, LoadLoot);
    MAKE_TASK(WorldManager, ParseMapDBC);

    Storage_LoadAdditionalTables();

    ThreadPool.ExecuteTask("TaskExecutor", new BasicTaskExecutor(new CallbackP0<ObjectMgr>(ObjectMgr::getSingletonPtr(), &ObjectMgr::LoadPlayersInfo), BTE_PRIORITY_MED));

    MAKE_TASK(LfgMgr, LoadRandomDungeonRewards);
    MAKE_TASK(ObjectMgr, LoadPlayerCreateInfo);
    MAKE_TASK(ObjectMgr, LoadSpellSkills);
    MAKE_TASK(ObjectMgr, ProcessTitles);
    MAKE_TASK(ObjectMgr, ProcessCreatureFamilies);
    tl.wait();

    MAKE_TASK(SpellManager, LoadSpellFixes);
    MAKE_TASK(GuildMgr, LoadAllGuilds);
    MAKE_TASK(GuildMgr, LoadGuildCharters);
    MAKE_TASK(AchievementMgr, ParseAchievements);
    MAKE_TASK(ObjectMgr, LoadVendors);
    MAKE_TASK(ObjectMgr, LoadTrainers);
    MAKE_TASK(TicketMgr, Load);
    MAKE_TASK(AddonMgr,  LoadFromDB);
    MAKE_TASK(ObjectMgr, SetHighestGuids);
    MAKE_TASK(ObjectMgr, ListGuidAmounts);
    MAKE_TASK(ObjectMgr, HashWMOAreaTables);
    MAKE_TASK(WeatherMgr,LoadFromDB);
    MAKE_TASK(ObjectMgr, LoadGroups);
    MAKE_TASK(Tracker,   LoadFromDB);
    MAKE_TASK(ObjectMgr, LoadExtraItemStuff);
    MAKE_TASK(ObjectMgr, LoadArenaTeams);
    MAKE_TASK(GossipManager, LoadGossipData);
    MAKE_TASK(WorldManager, LoadSpawnData);

#undef MAKE_TASK

    // wait for all loading to complete.
    tl.wait();

    // start mail system
    sLog.Notice("World","Starting Mail System...");
    MailSystem::getSingleton().StartMailSystem();

    sLog.Notice("World", "Starting Auction System...");
    new AuctionMgr;
    sAuctionMgr.LoadAuctionHouses();

    sLog.Success("World", "Database loaded in %ums.", getMSTime() - start_time);

    sVMapInterface.Init();
    sNavMeshInterface.Init();

    // calling this puts all maps into our task list.
    sWorldMgr.Load(&tl);

    // wait for the events to complete.
    tl.wait();

    // wait for them to exit, now.
    tl.kill();
    tl.waitForThreadsToExit();

    LoadNameGenData();

    sLog.Notice("World","Starting Transport System...");
    objmgr.LoadTransporters();

    if(mainIni->ReadBoolean("Startup", "BackgroundLootLoading", true))
    {
        sLog.Notice("World", "Background loot loading...");

        // loot background loading in a lower priority thread.
        ThreadPool.ExecuteTask("LootLoader", new BasicTaskExecutor(new CallbackP0<LootMgr>(LootMgr::getSingletonPtr(), &LootMgr::LoadDelayedLoot), BTE_PRIORITY_LOW));
    }
    else
    {
        sLog.Notice("World", "Loading loot in foreground...");
        lootmgr.LoadDelayedLoot();
    }

    sLog.Notice("World", "Loading Channel config...");
    Channel::LoadConfSettings();

    sLog.Notice("World", "Starting BattlegroundManager...");
    new CBattlegroundManager;
    BattlegroundManager.Init();

    if(GuildsLoading)
    {
        sLog.Notice( "World", "Waiting for groups and players to finish loading..." );
        while(GuildsLoading)
            Sleep( 100 );
    }

    for(uint32 i = 0; i < dbcCharStartOutfit.GetNumRows(); i++)
        if(CharStartOutfitEntry *startOutfit = dbcCharStartOutfit.LookupRow(i))
            StoreCharacterStartingOutfit(startOutfit);
    sLog.Notice("World", "Hashed %u/%u starting outfits", m_startingOutfits.size(), dbcCharStartOutfit.GetNumRows());

    // Begin preprocessing max capacities for mount types to cut down on array scanning during runtime
    sLog.Notice("World", "Processing %u mount types with %u capabilities...", dbcMountType.GetNumRows(), dbcMountCapability.GetNumRows());
    for(uint32 x = 0; x < dbcMountType.GetNumRows(); x++)
    {
        if(MountTypeEntry *mountType = dbcMountType.LookupRow(x))
        {
            mountType->maxCapability[0] = mountType->maxCapability[1] = 24;
            uint32 i = 24;
            while(i > 0)
            {
                i--;
                if(mountType->maxCapability[0] != 24 && mountType->maxCapability[1] != 24)
                    break;

                MountCapabilityEntry *entry = dbcMountCapability.LookupEntry(mountType->MountCapability[i]);
                if(entry == NULL)
                    continue;
                if(mountType->maxCapability[0] == 24)
                    mountType->maxCapability[0] = i;
                if(entry->requiredRidingSkill > 150)
                    continue;
                if(mountType->maxCapability[1] == 24)
                    mountType->maxCapability[1] = i;
            }

            // Increment changed max capacities
            if(mountType->maxCapability[0] != 24)
                mountType->maxCapability[0]++;
            if(mountType->maxCapability[1] != 24)
                mountType->maxCapability[1]++;
            // Set our secondary max capacity to be less than or equal to our regular max capacity
            if(mountType->maxCapability[1] > mountType->maxCapability[0])
                mountType->maxCapability[1] = mountType->maxCapability[0];
        }
    }

    sLog.Notice("World", "Processing %u area table entries...", dbcAreaTable.GetNumRows());
    for(uint32 i = 0; i < dbcAreaTable.GetNumRows(); i++)
    {
        AreaTableEntry *areaentry = dbcAreaTable.LookupRow(i);
        if(m_sanctuaries.find(areaentry->AreaId) == m_sanctuaries.end())
        {
            if(areaentry->category == AREAC_SANCTUARY || areaentry->AreaFlags & AREA_SANCTUARY)
                m_sanctuaries.insert(areaentry->AreaId);
        }

        if(m_sanctuaries.find(areaentry->AreaId) == m_sanctuaries.end())
        {
            if(areaentry->AreaFlags & AREA_CITY_AREA || areaentry->AreaFlags & AREA_CITY || areaentry->AreaFlags & AREA_CAPITAL_SUB || areaentry->AreaFlags & AREA_CAPITAL)
            {
                int8 team = -1;
                if(areaentry->category == AREAC_ALLIANCE_TERRITORY)
                    team = TEAM_ALLIANCE;
                if(areaentry->category == AREAC_HORDE_TERRITORY)
                    team = TEAM_HORDE;
                SetRestedArea(areaentry->AreaId, team);
            }
        }
    }

    return true;
}

void World::Update(uint32 diff)
{
    // Through main thread, we calculate our timers for weekday and event timers etc
    UpdateServerTimers(diff);

    // Update our queued sessions
    UpdateQueuedSessions(diff);

    // Auction updates
    if(AuctionMgr::getSingletonPtr() != NULL)
        sAuctionMgr.Update();

    // Mail updates
    if(MailSystem::getSingletonPtr() != NULL)
        sMailSystem.UpdateMessages(diff);

    // Guild updates
    if(GuildMgr::getSingletonPtr() != NULL)
        guildmgr.Update(diff);

    // Update sessions
    UpdateSessions(diff);

    UpdateShutdownStatus();
}

void World::UpdateServerTimers(uint32 diff)
{
    // Grab our current time
    time_t thisTime = UNIXTIME;
    localtime_s(&m_currentTimeData, &thisTime);

    // Update weekly start timer
    if(thisTime-m_weekStart >= 604800)      // One week has passed
        m_weekStart += 604800;

    // Update Server time
    m_gameTime += thisTime - m_lastTick;    //in seconds
    if(m_gameTime >= 86400) // One day has passed
        m_gameTime -= 86400;
    m_lastTick = thisTime;

    // Update our daily reset timers
    if(m_gameTime >= 12600 && m_dailyReset == true)
        m_dailyReset = false;
    else if(m_gameTime >= 10800 && m_gameTime < 12600 && m_dailyReset == false)
    {
        m_lastDailyReset = thisTime;
        CharacterDatabase.Execute("REPLACE INTO server_settings VALUES(\"last_dailies_reset_time\", \'%ull\')", thisTime);
        sLog.Notice("World", "Running Daily Quest Reset...");
        objmgr.ResetDailies();
        m_dailyReset = true;
    }

    // Update timers for heroic instance resets
    if(m_gameTime >= 34200 && m_heroicReset == true)
        m_heroicReset = false;
    else if(m_gameTime >= 32400 && m_gameTime < 34200 && m_heroicReset == false)
    {
        m_lastHeroicReset = thisTime;
        CharacterDatabase.Execute("REPLACE INTO server_settings VALUES(\"last_heroic_reset_time\", \'%ull\')", thisTime);
        sLog.Notice("World", "Reseting heroic instances...");
        sWorldMgr.ResetHeroicInstances();
        m_heroicReset = true;
    }

    // Update world events
    //sWorldEvents.Update(1900+m_currentTimeData.tm_year, 1+m_currentTimeData.tm_mon, m_currentTimeData.tm_mday, 1+m_currentTimeData.tm_wday, m_currentTimeData.tm_hour);
}

bool World::HasActiveEvents(WorldObject *obj)
{
    return false;
}

void World::SendMessageToGMs(WorldSession *self, const char * text, ...)
{
    char buf[500];
    va_list ap;
    va_start(ap, text);
    vsnprintf(buf, 2000, text, ap);
    va_end(ap);
    WorldSession *gm_session;

    WorldPacket data;
    sChatHandler.FillSystemMessageData(&data, buf);
    gmList_lock.AcquireReadLock();
    SessionSet::iterator itr;
    for (itr = gmList.begin(); itr != gmList.end();itr++)
    {
        gm_session = (*itr);
        if(gm_session->GetPlayer() != NULL && gm_session != self)  // dont send to self!)
            gm_session->SendPacket(&data);
    }
    gmList_lock.ReleaseReadLock();
}

void World::SendGlobalMessage(WorldPacket *packet, WorldSession *self)
{
    m_sessionlock.AcquireReadLock();

    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        if (itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld()
            && itr->second != self)  // dont send to self!
        {
            itr->second->SendPacket(packet);
        }
    }

    m_sessionlock.ReleaseReadLock();
}

void World::SendFactionMessage(WorldPacket *packet, uint8 teamId)
{
    m_sessionlock.AcquireReadLock();
    SessionMap::iterator itr;
    Player* plr;
    for(itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        plr = itr->second->GetPlayer();
        if(!plr || !plr->IsInWorld())
            continue;

        if(plr->GetTeam() == teamId)
            itr->second->SendPacket(packet);
    }
    m_sessionlock.ReleaseReadLock();
}

void World::SendZoneMessage(WorldPacket *packet, uint32 zoneid, WorldSession *self)
{
    m_sessionlock.AcquireReadLock();

    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld() && itr->second != self)  // dont send to self!
        {
            if (itr->second->GetPlayer()->GetZoneId() == zoneid)
                itr->second->SendPacket(packet);
        }
    }

    m_sessionlock.ReleaseReadLock();
}

void World::SendInstanceMessage(WorldPacket *packet, uint32 instanceid, WorldSession *self)
{
    m_sessionlock.AcquireReadLock();

    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        if (itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld()
            && itr->second != self)  // dont send to self!
        {
            if (itr->second->GetPlayer()->GetInstanceID() == (int32)instanceid)
                itr->second->SendPacket(packet);
        }
    }

    m_sessionlock.ReleaseReadLock();
}

void World::SendWorldText(const char* text, WorldSession *self)
{
    WorldPacket data;
    sChatHandler.FillSystemMessageData(&data, text);
    SendGlobalMessage(&data, self);
}

void World::SendGMWorldText(const char* text, bool admin)
{
    WorldPacket data;
    data.Initialize(SMSG_MESSAGECHAT);
    sChatHandler.FillSystemMessageData(&data, text);
    if(admin) SendAdministratorMessage(&data);
    else SendGamemasterMessage(&data);
}

void World::SendAdministratorMessage(WorldPacket *packet)
{
    m_sessionlock.AcquireReadLock();
    SessionMap::iterator itr;
    for(itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld())
        {
            if(itr->second->CanUseCommand('z'))
                itr->second->SendPacket(packet);
        }
    }
    m_sessionlock.ReleaseReadLock();
}

void World::SendGamemasterMessage(WorldPacket *packet)
{
    m_sessionlock.AcquireReadLock();
    SessionMap::iterator itr;
    for(itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld())
        {
            if(itr->second->GetPermissions())
                itr->second->SendPacket(packet);
        }
    }
    m_sessionlock.ReleaseReadLock();
}

void World::SendWorldWideScreenText(const char *text, WorldSession *self)
{
    WorldPacket data(256);
    data.Initialize(SMSG_AREA_TRIGGER_MESSAGE);
    data << (uint32)0 << text << (uint8)0x00;
    SendGlobalMessage(&data, self);
}

extern bool bServerShutdown;

void World::UpdateSessions(uint32 diff)
{
    int result;
    WorldSession *GlobalSession;
    SessionsMutex.Acquire();
    for(SessionSet::iterator itr = GlobalSessions.begin(); itr != GlobalSessions.end();)
    {
        GlobalSession = (*itr);
        ++itr;

        //We have been moved to mapmgr, remove us here.
        if( GlobalSession->GetEventInstanceId() != -1 )
        {
            RemoveGlobalSession(GlobalSession);
            continue;
        }

        result = GlobalSession->Update(-1);
        if(result)
        {
            if(result == 1)//socket don't exist anymore, delete from worldsessions.
                DeleteGlobalSession(GlobalSession);
            else //We have been (re-)moved to mapmgr, remove us here.
                RemoveGlobalSession(GlobalSession);
        }

        if(bServerShutdown)
            break;
    }
    SessionsMutex.Release();
}

std::string World::GenerateName(uint32 type)
{
    if(_namegendata[type].size() == 0)
        return "ERR";

    uint32 ent = RandomUInt((uint32)_namegendata[type].size()-1);
    return _namegendata[type].at(ent).name;
}

void World::DeleteGlobalSession(WorldSession *GlobalSession, bool destruct)
{
    SessionsMutex.Acquire();
    //If it's a GM, remove him from GM session map
    if(GlobalSession->HasGMPermissions())
    {
        gmList_lock.AcquireWriteLock();
        gmList.erase(GlobalSession);
        gmList_lock.ReleaseWriteLock();
    }

    // remove from big map
    m_sessionlock.AcquireWriteLock();
    if(m_sessions.find(GlobalSession->GetAccountId()) != m_sessions.end() && m_sessions.at(GlobalSession->GetAccountId()) == GlobalSession)
        m_sessions.erase(GlobalSession->GetAccountId());
    m_sessionlock.ReleaseWriteLock();

    // Remove us from the map
    RemoveGlobalSession(GlobalSession);

    // delete us
    if(destruct)
        delete GlobalSession;
    SessionsMutex.Release();
}

uint32 World::AddQueuedSocket(WorldSocket* Socket)
{
    // Since we have multiple socket threads, better guard for this one,
    // we don't want heap corruption ;)
    queueMutex.Acquire();

    // Add socket to list
    mQueuedSessions.push_back(Socket);
    queueMutex.Release();
    // Return queue position
    return (uint32)mQueuedSessions.size();
}

void World::RemoveQueuedSocket(WorldSocket* Socket)
{
    // Since we have multiple socket threads, better guard for this one,
    // we don't want heap corruption ;)
    queueMutex.Acquire();

    // Find socket in list
    QueueSet::iterator iter = mQueuedSessions.begin();
    for(; iter != mQueuedSessions.end(); iter++)
    {
        if((*iter) == Socket)
        {
            // Remove from the queue and abort.
            // This will be slow (Removing from middle of a vector!) but it won't
            // get called very much, so it's not really a big deal.

            mQueuedSessions.erase(iter);
            queueMutex.Release();
            return;
        }
    }
    queueMutex.Release();
}

uint32 World::GetQueuePos(WorldSocket* Socket)
{
    // Since we have multiple socket threads, better guard for this one,
    // we don't want heap corruption ;)
    queueMutex.Acquire();

    // Find socket in list
    QueueSet::iterator iter = mQueuedSessions.begin();
    uint32 QueuePos = 1;
    for(; iter != mQueuedSessions.end(); iter++, ++QueuePos)
    {
        if((*iter) == Socket)
        {
            queueMutex.Release();
            // Return our queue position.
            return QueuePos;
        }
    }
    queueMutex.Release();
    // We shouldn't get here..
    return 1;
}

void World::UpdateQueuedSessions(uint32 diff)
{
    if(diff >= m_queueUpdateTimer)
    {
        m_queueUpdateTimer = 60000;
        queueMutex.Acquire();
        if (!mQueuedSessions.empty())
        {
            while (m_sessions.size() < m_playerLimit && mQueuedSessions.size())
            {
                // Yay. We can let another player in now.
                // Grab the first fucker from the queue, but guard of course, since
                // this is in a different thread again.

                QueueSet::iterator iter = mQueuedSessions.begin();
                WorldSocket * QueuedSocket = *iter;
                mQueuedSessions.erase(iter);

                // Welcome, sucker.
                if (QueuedSocket->GetSession())
                    QueuedSocket->Authenticate();
            }

            if (!mQueuedSessions.empty())
            {
                // Update the remaining queue members.
                QueueSet::iterator iter = mQueuedSessions.begin();
                uint32 Position = 1;
                while (iter != mQueuedSessions.end())
                {
                    (*iter)->SendAuthResponse(AUTH_WAIT_QUEUE, true, Position++);
                    ++iter;
                }
            }
        }
        queueMutex.Release();
    } else m_queueUpdateTimer -= diff;

    m_pushUpdateTimer += diff;
    if(m_pushUpdateTimer > 15000)
    {
        // Reset our update timer
        m_pushUpdateTimer = 0;
        // Now we process each queued world push and do a delayed removal from map inside our wrapped mutex
        uint8 vError;
        std::set<std::pair<uint8, WorldSession*>> sessions;
        m_worldPushLock.Acquire();
        for(std::map<WorldSession*, std::pair<WoWGuid, uint32> >::iterator itr = m_worldPushQueue.begin(); itr != m_worldPushQueue.end(); itr++)
        {
            if((vError = sWorldMgr.ValidateMapId(itr->second.second)) == 2)
                continue;
            sessions.insert(std::make_pair(vError, itr->first));
        }

        // Process delayed removals and process calls
        for(std::set<std::pair<uint8, WorldSession*>>::iterator itr = sessions.begin(); itr != sessions.end(); itr++)
        {
            std::map<WorldSession*, std::pair<WoWGuid, uint32> >::iterator sessItr = m_worldPushQueue.find((*itr).second);
            ASSERT("FUCK" && sessItr != m_worldPushQueue.end());
            WorldSession *sess = sessItr->first;
            WoWGuid guid = sessItr->second.first;
            m_worldPushQueue.erase(sessItr);
            if((*itr).first)
            {
                sess->Disconnect();
                continue;
            }

            sess->PlayerLoginProc(guid);
        }
        m_worldPushLock.Release();
    }
}

void World::SaveAllPlayers()
{
    if(!(ObjectMgr::getSingletonPtr()))
        return;

    sLog.outString("Saving all players to database...");
    uint32 count = 0, mt;
    ObjectMgr::PlayerStorageMap::const_iterator itr;
    // Servers started and obviously runing. lets save all players.
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        if(itr->second->GetSession())
        {
            mt = getMSTime();
            itr->second->SaveToDB(false);
            sLog.outString("Saved player `%s` (level %u) in %ums.", itr->second->GetName(), itr->second->GetUInt32Value(UNIT_FIELD_LEVEL), getMSTime() - mt);
            ++count;
        }
    }
    objmgr._playerslock.ReleaseReadLock();
    sLog.outString("Saved %u players.", count);
}

float World::GetCPUUsage(bool external)
{
#ifdef WIN32
    CPerfCounters<LONGLONG> PerfCounters;
    DWORD dwObjectIndex = PROCESS_OBJECT_INDEX;
    DWORD dwCpuUsageIndex = PROCESSOR_TIME_COUNTER_INDEX;

    LONGLONG lnNewValue = 0;
    PPERF_DATA_BLOCK pPerfData = NULL;
    LARGE_INTEGER NewPerfTime100nSec = {0};

    lnNewValue = PerfCounters.GetCounterValueForProcessID(&pPerfData, dwObjectIndex, dwCpuUsageIndex, GetCurrentProcessId());
    NewPerfTime100nSec = pPerfData->PerfTime100nSec;
    if (external && m_bFirstTime)
    {
        m_bFirstTime = false;
        m_lnOldValue = lnNewValue;
        m_OldPerfTime100nSec = NewPerfTime100nSec;
        return 0.0f;
    }

    LONGLONG lnValueDelta = lnNewValue - m_lnOldValue;
    double DeltaPerfTime100nSec = (double)NewPerfTime100nSec.QuadPart - (double)m_OldPerfTime100nSec.QuadPart;

    m_lnOldValue = lnNewValue;
    m_OldPerfTime100nSec = NewPerfTime100nSec;

    double a = (double)lnValueDelta / DeltaPerfTime100nSec;
    a /= double(number_of_cpus);
    return float(a * 100.0);
#else
    return 0.0f;
#endif
}

float World::GetRAMUsage(bool external)
{
    float RAMUsage = 0.0f;
#ifdef WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    RAMUsage = (float)pmc.PagefileUsage;
    RAMUsage /= 1024.0f;
    RAMUsage /= 1024.0f;
#else
#ifdef UNTESTED_CODE
    FILE* file = fopen("/proc/self/status", "r");
    char line[128];
    while (fgets(line, 128, file) != NULL)
    {
        if (strncmp(line, "VmSize:", 7) == 0)
        {
            RAMUsage = parseLine(line);
            break;
        }
    }
    fclose(file);
#endif
#endif
    return RAMUsage;
}

WorldSession* World::FindSessionByName(const char * Name)//case insensetive
{
    m_sessionlock.AcquireReadLock();

    // loop sessions, see if we can find him
    SessionMap::iterator itr = m_sessions.begin();
    for(; itr != m_sessions.end(); itr++)
    {
        if(!stricmp(itr->second->GetAccountName().c_str(),Name))
        {
            m_sessionlock.ReleaseReadLock();
            return itr->second;
        }
    }
    m_sessionlock.ReleaseReadLock();
    return 0;
}

void World::GetStats(uint32 * GMCount, float * AverageLatency)
{
    uint32 gm = 0, count = 0, total = 0;
    ObjectMgr::PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        if(itr->second->GetSession())
        {
            count++;
            total += itr->second->GetSession()->GetLatency();
            if(itr->second->GetSession()->GetPermissionCount())
                gm++;
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    *AverageLatency = count ? (float)((float)total / (float)count) : 0;
    *GMCount = gm;
}

bool World::BuildMoTDPacket(WorldSession *session, WorldPacket *packet)
{
    uint32 linecount = 0;
    *packet << uint32(linecount);
    if(m_motd.length())
    {
        std::string str_motd = sWorld.GetMotd();
        std::string::size_type pos = 0, nextpos;
        while ((nextpos = str_motd.find('@', pos)) != std::string::npos)
        {
            if (nextpos != pos)
            {
                *packet << str_motd.substr(pos, nextpos - pos);
                ++linecount;
            }
            pos = nextpos + 1;
        }

        if (pos < str_motd.length())
        {
            *packet << str_motd.substr(pos);
            ++linecount;
        }
    }

    if(session->HasGMPermissions())
    {
        // Send revision
        *packet << format("Server: %sRonin v%u|r%s | %s|r", MSG_COLOR_CRIMSON, BUILD_REVISION, MSG_COLOR_TORQUISEBLUE, m_hashInfo.c_str());
        *packet << format("Online Players: %s%u|r Peak: %s%u|r Connections: %s%u|r", MSG_COLOR_TORQUISEBLUE, GetSessionCount(), MSG_COLOR_TORQUISEBLUE, PeakSessionCount, MSG_COLOR_TORQUISEBLUE, mAcceptedConnections);
        *packet << format("Server Uptime: %s%s|r", MSG_COLOR_TORQUISEBLUE, GetUptimeString().c_str());
        linecount += 3;
    }

    packet->put(0, linecount);
    return linecount > 0;
}

void TaskList::AddTask(Task * task)
{
    queueLock.Acquire();
    tasks.insert(task);
    queueLock.Release();
}

Task * TaskList::GetTask()
{
    queueLock.Acquire();

    Task* t = 0;
    for(std::set<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
    {
        if(!(*itr)->in_progress)
        {
            t = (*itr);
            t->in_progress = true;
            break;
        }
    }
    queueLock.Release();
    return t;
}

void TaskList::spawn()
{
    running = true;
    thread_count = 0;

    uint32 threadcount = 1;
    if(mainIni->ReadBoolean("Startup", "EnableMultithreadedLoading", true))
    {
        // get processor count
#ifndef WIN32
#if UNIX_FLAVOUR == UNIX_FLAVOUR_LINUX
#ifdef X64
        threadcount = 2;
#else
        long affmask;
        sched_getaffinity(0, 4, (cpu_set_t*)&affmask);
        threadcount = (BitCount8(affmask)) * 2;
        if(threadcount > 8) threadcount = 8;
        else if(threadcount <= 0) threadcount = 1;
#endif
#else
        threadcount = 2;
#endif
#else
        SYSTEM_INFO s;
        GetSystemInfo(&s);
        threadcount = s.dwNumberOfProcessors * 2;
        if(threadcount > 8)
            threadcount = 8;
#endif
    }

    sLog.Notice("World", "Beginning %s server startup with %u thread(s).", (threadcount == 1) ? "progressive" : "parallel", threadcount);
    for(uint32 x = 0; x < threadcount; ++x)
        ThreadPool.ExecuteTask(format("TaskExecutor|%u", x).c_str(), new TaskExecutor(this));
}

void TaskList::wait()
{
    bool has_tasks = true;
    time_t t;
    while(has_tasks)
    {
        queueLock.Acquire();
        has_tasks = false;
        for(std::set<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
        {
            if(!(*itr)->completed)
            {
                has_tasks = true;
                break;
            }
        }
        queueLock.Release();

        // keep updating time lol
        t = time(NULL);
        if( UNIXTIME != t )
        {
            UNIXTIME = t;
            g_localTime = *localtime(&t);
        }

        Sleep(20);
    }
}

void TaskList::kill()
{
    running = false;
}

void Task::execute()
{
    _cb->execute();
}

bool TaskExecutor::run()
{
    Task * t;
    while(starter->running)
    {
        if(t = starter->GetTask())
        {
            t->execute();
            t->completed = true;
            starter->RemoveTask(t);
            delete t;
        } else Delay(20);
    }
    return true;
}

void TaskList::waitForThreadsToExit()
{
    while(thread_count)
    {
        Sleep(20);
    }
}

void World::DeleteObject(WorldObject* obj)
{
    obj->Destruct();
    obj = NULL;
}

void World::Rehash(bool load)
{
    mainIni->Reload();
    if(!ChannelMgr::getSingletonPtr())
        new ChannelMgr;

    if(!MailSystem::getSingletonPtr())
        new MailSystem;

    sLog.Init(mainIni->ReadInteger("LogLevel", "Screen", 1));
    QueryLog = mainIni->ReadBoolean("LogLevel", "Query", false);

    // Data configs
    DBCPath = mainIni->ReadString("Data", "DBCPath", "dbc");
    MapPath = mainIni->ReadString("Data", "MapPath", "Tiles");
    VObjPath = mainIni->ReadString("Data", "VObjPath", "Tiles/Obj");
    MNavPath = mainIni->ReadString("Data", "MNavPath", "Tiles/Nav");

    // Performance configs
    Collision = mainIni->ReadBoolean("PerformanceSettings", "Collision", false);
    PathFinding = mainIni->ReadBoolean("PerformanceSettings", "Pathfinding", false);
    AreaUpdateDistance = mainIni->ReadFloat("PerformanceSettings", "AreaUpdateDistance", false);
    AreaUpdateDistance *= AreaUpdateDistance;

    // Server Configs
    StartGold = mainIni->ReadInteger("ServerSettings", "StartGold", 1);
    StartLevel = mainIni->ReadInteger("ServerSettings", "StartLevel", 1);
    MaxLevelCalc = mainIni->ReadInteger("ServerSettings", "MaxLevelCalc", MAXIMUM_ATTAINABLE_LEVEL);
    m_useAccountData = mainIni->ReadBoolean("ServerSettings", "UseAccountData", false);
    SetMotd(mainIni->ReadString("ServerSettings", "Motd", "Ronin Default MOTD").c_str());
    cross_faction_world = mainIni->ReadBoolean("ServerSettings", "CrossFactionInteraction", false);
    SendMovieOnJoin = mainIni->ReadBoolean("ServerSettings", "SendMovieOnJoin", true);
    m_blockgmachievements = mainIni->ReadBoolean("ServerSettings", "DisableAchievementsForGM", true);
    channelmgr.seperatechannels = mainIni->ReadBoolean("ServerSettings", "SeperateChatChannels", true);
    gm_force_robes = mainIni->ReadBoolean("ServerSettings", "ForceRobesForGM", false);
    trade_world_chat = mainIni->ReadInteger("ServerSettings", "TradeWorldChat", 0);
    SetPlayerLimit(mainIni->ReadInteger("ServerSettings", "PlayerLimit", 1000));
    FunServerMall = mainIni->ReadInteger("ServerSettings", "MallAreaID", -1);
    LogoutDelay = mainIni->ReadInteger("ServerSettings", "Logout_Delay", 20);
    EnableFatigue = mainIni->ReadBoolean("ServerSettings", "EnableFatigue", true);
    ServerPreloading = mainIni->ReadInteger("Startup", "Preloading", 0);
    if(LogoutDelay <= 0)
        LogoutDelay = 1;

    // Battlegrounds
    // Wintergrasp
    wg_enabled = mainIni->ReadBoolean("Battlegrounds", "EnableWG", false);

    // Alterac Valley
    av_enabled = mainIni->ReadBoolean("Battlegrounds", "EnableAV", true);
    av_minplrs = mainIni->ReadInteger("Battlegrounds", "AVMinPlayers", 20);

    // Warsong Gulch
    wsg_enabled = mainIni->ReadBoolean("Battlegrounds", "EnableWSG", true);
    wsg_minplrs = mainIni->ReadInteger("Battlegrounds", "WSGMinPlayers", 5);

    // Arathi Basin
    ab_enabled = mainIni->ReadBoolean("Battlegrounds", "EnableAB", true);
    ab_minplrs = mainIni->ReadInteger("Battlegrounds", "ABMinPlayers", 7);

    // Eye of the Storm
    eots_enabled = mainIni->ReadBoolean("Battlegrounds", "EnableEOTS", true);
    eots_minplrs = mainIni->ReadInteger("Battlegrounds", "EOTSMinPlayers", 7);

    // Strand of the Ancients
    sota_enabled = mainIni->ReadBoolean("Battlegrounds", "EnableSOTA", true);
    sota_minplrs = mainIni->ReadInteger("Battlegrounds", "SOTAMinPlayers", 15);

    // Isle of Conquest
    ioc_enabled = mainIni->ReadBoolean("Battlegrounds", "EnableIOC", true);
    ioc_minplrs = mainIni->ReadInteger("Battlegrounds", "IOCMinPlayers", 15);

    // load regeneration rates.
    setRate(RATE_XP, mainIni->ReadFloat("Rates", "XP", 1.0f));
    setRate(RATE_QUESTXP, mainIni->ReadFloat("Rates", "QuestXP", 1.0f));
    setRate(RATE_RESTXP, mainIni->ReadFloat("Rates", "RestXP", 1.0f));
    setRate(RATE_DROP0, mainIni->ReadFloat("Rates", "DropGrey", 1.0f));
    setRate(RATE_DROP1, mainIni->ReadFloat("Rates", "DropWhite", 1.0f));
    setRate(RATE_DROP2, mainIni->ReadFloat("Rates", "DropGreen", 1.0f));
    setRate(RATE_DROP3, mainIni->ReadFloat("Rates", "DropBlue", 1.0f));
    setRate(RATE_DROP4, mainIni->ReadFloat("Rates", "DropPurple", 1.0f));
    setRate(RATE_DROP5, mainIni->ReadFloat("Rates", "DropOrange", 1.0f));
    setRate(RATE_DROP6, mainIni->ReadFloat("Rates", "DropArtifact", 1.0f));
    setRate(RATE_MONEY, mainIni->ReadFloat("Rates", "DropMoney", 1.0f));
    setRate(RATE_QUESTMONEY, mainIni->ReadFloat("Rates", "QuestMoney", 1.0f));
    setRate(RATE_HONOR, mainIni->ReadFloat("Rates", "Honor", 1.0f));
    setRate(RATE_SKILLRATE, mainIni->ReadFloat("Rates", "SkillRate", 1.0f));
    setRate(RATE_SKILLCHANCE, mainIni->ReadFloat("Rates", "SkillChance", 1.0f));
    setRate(RATE_QUESTREPUTATION, mainIni->ReadFloat("Rates", "QuestReputation", 1.0f));
    setRate(RATE_KILLREPUTATION, mainIni->ReadFloat("Rates", "KillReputation", 1.0f));

    GmClientChannel = mainIni->ReadString("GMClient", "GmClientChannel", "");
    m_reqGmForCommands = !mainIni->ReadBoolean("ServerSettings", "AllowPlayerCommands", false);

    uint32 config_flags = 0;
    if(mainIni->ReadBoolean("Mail", "DisablePostageCostsForGM", true))
        config_flags |= MAIL_FLAG_NO_COST_FOR_GM;

    if(mainIni->ReadBoolean("Mail", "DisablePostageCosts", false))
        config_flags |= MAIL_FLAG_DISABLE_POSTAGE_COSTS;

    if(mainIni->ReadBoolean("Mail", "DisablePostageDelayItems", true))
        config_flags |= MAIL_FLAG_DISABLE_HOUR_DELAY_FOR_ITEMS;

    if(mainIni->ReadBoolean("Mail", "DisableMessageExpiry", false))
        config_flags |= MAIL_FLAG_NO_EXPIRY;

    if(mainIni->ReadBoolean("Mail", "EnableInterfactionMail", true))
        config_flags |= MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION;

    if(mainIni->ReadBoolean("Mail", "EnableInterfactionForGM", true))
        config_flags |= MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION_GM;

    sMailSystem.SetConfigFlags(config_flags);
    flood_lines = mainIni->ReadInteger("FloodProtection", "Lines", 0);
    flood_seconds = mainIni->ReadInteger("FloodProtection", "Seconds", 0);
    flood_message = mainIni->ReadBoolean("FloodProtection", "SendMessage", false);
    flood_message_time = mainIni->ReadInteger("FloodProtection", "FloodMessageTime", 0);
    flood_mute_after_flood = mainIni->ReadInteger("FloodProtection", "MuteAfterFlood", 0);
    flood_caps_min_len = mainIni->ReadInteger("FloodProtection", "CapsMinLen", 0);
    flood_caps_pct = mainIni->ReadFloat("FloodProtection", "CapsPct", 0.0f);

    if(!flood_lines || !flood_seconds)
        flood_lines = flood_seconds = 0;
    // ======================================

    m_deathKnightOnePerAccount = mainIni->ReadBoolean("DeathKnight", "OnePerRealm", true);
    m_deathKnightReqLevel = mainIni->ReadInteger("DeathKnight", "RequiredLevel", 55);

    NumericCommandGroups = mainIni->ReadBoolean("ServerSettings", "NumericCommandGroups", false);
    Start_With_All_Taximasks = mainIni->ReadBoolean("ServerSettings", "StartWithAll_Taximasks", false);

    // LevelCaps
    LevelCap_Custom_All = mainIni->ReadInteger("ServerSettings", "LevelCap_Custom_All", 80);
    if(LevelCap_Custom_All < 1)
        LevelCap_Custom_All = 80;

    if( load )
        Channel::LoadConfSettings();
}

void World::LoadNameGenData()
{
    DBCFile dbc;

    if( !dbc.open( format("%s/NameGen.dbc", sWorld.DBCPath.c_str()).c_str() ) )
    {
        sLog.Error( "World", "Cannot find file %s/NameGen.dbc", sWorld.DBCPath.c_str() );
        return;
    }

    for(uint32 i = 0; i < dbc.getRecordCount(); i++)
    {
        NameGenData d;
        if(dbc.getRecord(i).getString(1) == NULL)
            continue;

        d.name = std::string(dbc.getRecord(i).getString(1));
        d.type = dbc.getRecord(i).getUInt(3);
        _namegendata[d.type].push_back(d);
    }
}

bool World::HasPendingWorldPush(WorldSession *session)
{
    m_worldPushLock.Acquire();
    bool res = m_worldPushQueue.find(session) != m_worldPushQueue.end();
    m_worldPushLock.Release();
    return res;
}

void World::QueueWorldPush(WorldSession *session, WoWGuid guid, uint32 mapId)
{
    m_worldPushLock.Acquire();
    m_worldPushQueue.insert(std::make_pair(session, std::make_pair(guid, mapId)));
    m_worldPushLock.Release();
}

void World::CancelWorldPush(WorldSession *session)
{
    std::map<WorldSession*, std::pair<WoWGuid, uint32>>::iterator itr;
    m_worldPushLock.Acquire();
    if((itr = m_worldPushQueue.find(session)) != m_worldPushQueue.end())
        m_worldPushQueue.erase(itr);
    m_worldPushLock.Release();
}

void World::DisconnectUsersWithAccount(const char * account, WorldSession * m_session)
{
    SessionMap::iterator itr;
    WorldSession * worldsession;
    m_sessionlock.AcquireReadLock();
    for(itr = m_sessions.begin(); itr != m_sessions.end();)
    {
        worldsession = (itr->second);
        ++itr;

        if(!stricmp(account, worldsession->GetAccountNameS()))
        {
            m_session->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", worldsession->GetAccountNameS(),
                worldsession->GetSocket() ? worldsession->GetSocket()->GetIP() : "noip", worldsession->GetPlayer() ? worldsession->GetPlayer()->GetName() : "noplayer");

            worldsession->Disconnect();
        }
    }
    m_sessionlock.ReleaseReadLock();
}

void World::DisconnectUsersWithIP(const char * ip, WorldSession * m_session)
{
    SessionMap::iterator itr;
    WorldSession * worldsession;
    m_sessionlock.AcquireReadLock();
    for(itr = m_sessions.begin(); itr != m_sessions.end();)
    {
        worldsession = (itr->second);
        ++itr;

        if(!worldsession->GetSocket())
            continue;

        std::string ip2 = worldsession->GetSocket()->GetIP();
        if(!stricmp(ip, ip2.c_str()))
        {
            m_session->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", worldsession->GetAccountNameS(),
                ip2.c_str(), worldsession->GetPlayer() ? worldsession->GetPlayer()->GetName() : "noplayer");

            worldsession->Disconnect();
        }
    }
    m_sessionlock.ReleaseReadLock();
}

void World::DisconnectUsersWithPlayerName(const char * plr, WorldSession * m_session)
{
    SessionMap::iterator itr;
    WorldSession * worldsession;
    m_sessionlock.AcquireReadLock();
    for(itr = m_sessions.begin(); itr != m_sessions.end();)
    {
        worldsession = (itr->second);
        ++itr;

        if(!worldsession->GetPlayer())
            continue;

        if(!stricmp(plr, worldsession->GetPlayer()->GetName()))
        {
            m_session->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", worldsession->GetAccountNameS(),
                worldsession->GetSocket() ? worldsession->GetSocket()->GetIP() : "noip", worldsession->GetPlayer() ? worldsession->GetPlayer()->GetName() : "noplayer");

            worldsession->Disconnect();
        }
    }
    m_sessionlock.ReleaseReadLock();
}

std::string World::GetUptimeString()
{
    char str[300];
    time_t pTime = (time_t)UNIXTIME - m_StartTime;
    tm * tmv = gmtime(&pTime);

    snprintf(str, 300, "%u days, %u hours, %u minutes, %u seconds.", tmv->tm_yday, tmv->tm_hour, tmv->tm_min, tmv->tm_sec);
    return std::string(str);
}

void World::UpdateShutdownStatus()
{
    if(m_shutdownTime == 0)
        return;

    uint32 time_left = ((uint32)UNIXTIME > m_shutdownTime) ? 0 : m_shutdownTime - (uint32)UNIXTIME;
    uint32 time_period = 1;

    if( time_left && m_shutdownTime )
    {
        // determine period
        if( time_left <= 30 )
        {
            // every 1 sec
            time_period = 1;
        }
        else if( time_left <= (TIME_MINUTE * 2) )
        {
            // every 30 secs
            time_period = 30;
        }
        else
        {
            // every minute
            time_period = 60;
        }

        // time to send a new packet?
        if( ( (uint32)UNIXTIME - m_shutdownLastTime ) >= time_period )
        {
            // send message
            m_shutdownLastTime = (uint32)UNIXTIME;

            WorldPacket data(SMSG_SERVER_MESSAGE, 200);
            if( m_shutdownType == SERVER_SHUTDOWN_TYPE_RESTART )
                data << uint32(SERVER_MSG_RESTART_TIME);
            else
                data << uint32(SERVER_MSG_SHUTDOWN_TIME);

            char tbuf[100];
            snprintf(tbuf, 100, "%02u:%02u", (time_left / 60), (time_left % 60));
            data << tbuf;
            SendGlobalMessage(&data, NULL);

            printf("Server shutdown in %s.\n", tbuf);
        }
    }
    else
    {
        // shutting down?
        if( m_shutdownTime )
        {
            SendWorldText("Server is saving and shutting down. You will be disconnected shortly.", NULL);
            Master::m_stopEvent = true;
        }
        else
        {
            WorldPacket data(SMSG_SERVER_MESSAGE, 200);
            if( m_shutdownTime == SERVER_SHUTDOWN_TYPE_RESTART )
                data << uint32(SERVER_MSG_RESTART_CANCELLED);
            else
                data << uint32(SERVER_MSG_SHUTDOWN_CANCELLED);

            data << uint8(0);
            SendGlobalMessage(&data, NULL);
        }
    }
}

void World::CancelShutdown()
{
    m_shutdownTime = 0;
    m_shutdownType = 0;
    m_shutdownLastTime = 0;
}

void World::QueueShutdown(uint32 delay, uint32 type)
{
    // set parameters
    m_shutdownLastTime = 0;
    m_shutdownTime = (uint32)UNIXTIME + delay;
    m_shutdownType = type;

    // send message
    char buf[1000];
    snprintf(buf, 1000, "Server %s initiated. Server will save and shut down in approx. %u seconds.", type == SERVER_SHUTDOWN_TYPE_RESTART ? "restart" : "shutdown", delay);
    SendWorldText(buf, NULL);
}

void World::BackupDB()
{
#ifndef WIN32
    const char *tables[] =
    { "account_data", "account_forced_permissions", "achievements", "arenateams", "auctions",
      "banned_names", "character_data", "character_data_insert_queue", "charters", "corpses", "gm_tickets",
      "groups", "guild_bankitems", "guild_banklogs", "guild_banktabs",
      "guild_data", "guild_logs", "guild_ranks", "guilds",
      "instances", "mailbox", "mailbox_insert_queue", "news_timers",
      "playercooldowns", "item_data", "character_inventory", "pet_data",
      "pet_spells", "pet_talents", "playersummons", "playersummonspells", "questlog",
      "server_settings", "social_friends", "social_ignores", "tutorials",
      "worldstate_save_data", NULL };
    char cmd[1024];
    char datestr[256];
    char path[256];
    std::string user, pass;
    std::string host, name;
    struct tm tm;
    time_t t;
    int i;

    user = mainIni->ReadString("CharacterDatabase", "Username", "");
    pass = mainIni->ReadString("CharacterDatabase", "Password", "");
    host = mainIni->ReadString("CharacterDatabase", "Hostname", "");
    name = mainIni->ReadString("CharacterDatabase", "Name", "");
    t = time(NULL);
    localtime_r(&t, &tm);
    strftime(datestr, 256, "%Y.%m.%d", &tm);

    snprintf(path, 256, "bk/%s", datestr);
    snprintf(cmd, 1024, "mkdir -p %s", path);

    sLog.outString("Backing up character db into %s", path);

    for (i=0; tables[i] != NULL; i++)
    {
        snprintf(cmd, 1024, "mkdir -p %s", path);
        system(cmd);

        snprintf(cmd, 1024, "mysqldump -u\"%s\" -p\"%s\" -h\"%s\" --result-file=\"%s/%s.sql\" \"%s\" \"%s\"", user.c_str(), pass.c_str(), host.c_str(), path, tables[i], name.c_str(), tables[i]);
        system(cmd);
    }

    sLog.outString("Done!");
#endif
}

void World::LogGM(WorldSession* session, std::string message, ...)
{
    if(LogCommands)
    {
        va_list ap;
        va_start(ap, message);
        char msg0[1024];
        char msg1[1024];
        vsnprintf(msg0, 1024, message.c_str(), ap);
        va_end(ap);
        uint32 size = (uint32)strlen(msg0);

        uint32 j = 0;
        for(uint32 i = 0; i < size; i++)
        {
            if(msg0[i] == '"')
            {
                msg1[j++] = '\\';
                msg1[j++] = '"';
            }
            else
                msg1[j++] = msg0[i];
        }

        const char* execute = format("INSERT INTO gmlog VALUES( %u, %u, \"%s\", \"%s\", \"%s\", \"%s\")", uint32(UNIXTIME),
            session->GetAccountId(), session->GetAccountName().c_str(), (session->GetSocket() ? session->GetSocket()->GetIP() : "NOIP"),
            (session->GetPlayer() ? session->GetPlayer()->GetName() : "nologin"), ((char*)(msg1))).c_str();

        LogDatabase.Execute(LogDatabase.EscapeString(execute).c_str());
    }
}

void World::LogCheater(WorldSession* session, std::string message, ...)
{
    if(LogCheaters)
    {
        va_list ap;
        va_start(ap, message);
        char msg0[1024];
        char msg1[1024];
        vsnprintf(msg0, 1024, message.c_str(), ap);
        va_end(ap);
        uint32 size = (uint32)strlen(msg0);

        uint32 j = 0;
        for(uint32 i = 0; i < size; i++)
        {
            if(msg0[i] == '"')
            {
                msg1[j++] = '\\';
                msg1[j++] = '"';
            }
            else
                msg1[j++] = msg0[i];
        }

        const char* execute = format("INSERT INTO cheaterlog VALUES( %u, %u, \"%s\", \"%s\", \"%s\", \"%s\")", uint32(UNIXTIME),
            session->GetAccountId(), session->GetAccountName().c_str(), (session->GetSocket() ? session->GetSocket()->GetIP() : "NOIP"),
            (session->GetPlayer() ? session->GetPlayer()->GetName() : "nologin"), ((char*)(msg1))).c_str();

        LogDatabase.Execute(LogDatabase.EscapeString(execute).c_str());
    }
}

void World::LogPlayer(WorldSession* session, std::string message, ...)
{
    if(LogPlayers)
    {
        va_list ap;
        va_start(ap, message);
        char msg0[1024];
        char msg1[1024];
        vsnprintf(msg0, 1024, message.c_str(), ap);
        va_end(ap);
        uint32 size = (uint32)strlen(msg0);

        uint32 j = 0;
        for(uint32 i = 0; i < size; i++)
        {
            if(msg0[i] == '"')
            {
                msg1[j++] = '\\';
                msg1[j++] = '"';
            }
            else
                msg1[j++] = msg0[i];
        }

        const char* execute = format("INSERT INTO playerlog VALUES( %u, %u, \"%s\", \"%s\", \"%s\", \"%s\")", uint32(UNIXTIME),
            session->GetAccountId(), session->GetAccountName().c_str(), (session->GetSocket() ? session->GetSocket()->GetIP() : "NOIP"),
            (session->GetPlayer() ? session->GetPlayer()->GetName() : "nologin"), ((char*)(msg1))).c_str();

        LogDatabase.Execute(LogDatabase.EscapeString(execute).c_str());
    }
}

void World::LogChat(WorldSession* session, std::string message, ...)
{
    if(bLogChat)
    {
        va_list ap;
        va_start(ap, message);
        char msg0[1024];
        char msg1[1024];
        vsnprintf(msg0, 1024, message.c_str(), ap);
        va_end(ap);
        uint32 size = (uint32)strlen(msg0);

        uint32 j = 0;
        for(uint32 i = 0; i < size; i++)
        {
            if(msg0[i] == '"')
            {
                msg1[j++] = '\\';
                msg1[j++] = '"';
            }
            else
                msg1[j++] = msg0[i];
        }

        const char* execute = format("INSERT INTO chatlog VALUES( %u, %u, \"%s\", \"%s\", \"%s\", \"%s\")", uint32(UNIXTIME),
            session->GetAccountId(), session->GetAccountName().c_str(), (session->GetSocket() ? session->GetSocket()->GetIP() : "NOIP"),
            (session->GetPlayer() ? session->GetPlayer()->GetName() : "nologin"), ((char*)(msg1))).c_str();

        LogDatabase.Execute(LogDatabase.EscapeString(execute).c_str());
    }
}
