/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton( InsertQueueLoader );
initialiseSingleton( World );

World::World()
{
    m_StartTime = 0;
    m_playerLimit = 0;
    GmClientChannel = "";
    GuildsLoading = false;
    NetworkStressIn = 0;
    NetworkStressOut = 0;
    mQueueUpdateInterval = 180000;
    PeakSessionCount = 0;
    mInWorldPlayerCount = 0;
    mAcceptedConnections = 0;
    HordePlayers = 0;
    AlliancePlayers = 0;
    gm_force_robes = false;
    AHEnabled = true;
    HallowsEnd = false;
    WintersVeil = false;
    IsPvPRealm = true;
    m_speedHackThreshold = -500.0f;
    m_speedHackLatencyMultiplier = 0.0f;
    m_CEThreshold = 10000;
    LacrimiThread = NULL;
    LacrimiPtr = NULL;
    authSeed1.SetRand(16 * 8);
    authSeed2.SetRand(16 * 8);

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
    else // Classic World of Warcraft
        level = 60;
    return level;
}

uint32 World::GetMaxLevelStatCalc()
{
    if(MaxLevelCalc && MaxLevelCalc < MAXIMUM_ATTAINABLE_LEVEL)
        return MaxLevelCalc;

    return MAXIMUM_ATTAINABLE_LEVEL;
}

void World::LogoutPlayers()
{
    sLog.Notice("World", "Logging out players...");
    for(SessionMap::iterator i = m_sessions.begin(); i != m_sessions.end(); i++)
        (i->second)->LogoutPlayer(true);

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
    if(Collision)
    {
        sVMapInterface.DeInit();
        if(PathFinding)
            NavMeshInterface.DeInit();
    }

    dummyspells.clear();

    sLog.Notice("Tracker", "~Tracker()");
    delete Tracker::getSingletonPtr();

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

    sLog.Notice("DayWatcherThread", "~DayWatcherThread()");
    delete DayWatcherThread::getSingletonPtr();

    sLog.Notice("InstanceMgr", "~InstanceMgr()");
    sInstanceMgr.Shutdown();

    sLog.Notice("WordFilter", "~WordFilter()");
    delete g_characterNameFilter;
    g_characterNameFilter = NULL;
    delete g_chatFilter;
    g_chatFilter = NULL;

    for( AreaTriggerMap::iterator i = m_AreaTrigger.begin( ); i != m_AreaTrigger.end( ); i++ )
        delete i->second;
    m_AreaTrigger.clear();

    Storage_Cleanup();
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

void ApplyNormalFixes();

void PreStartQueries()
{
    QueryResult * result;

    result = WorldDatabase.Query("SELECT * FROM prestartqueries ORDER BY seq ASC");
    if(result)
    {
        sLog.Notice("DataBase","Found and executing %u prestart queries on World tables.",result->GetRowCount());
        do
        {
            Field * f = result->Fetch();
            string e_query =  f[0].GetString();
            WorldDatabase.Execute(e_query.c_str());
        }while(result->NextRow());

        delete result;
        WorldDatabase.Execute("DELETE FROM prestartqueries WHERE SingleShot = 1;");
    }

    result = CharacterDatabase.Query("SELECT * FROM prestartqueries ORDER BY seq ASC");
    if(result)
    {
        sLog.Notice("DataBase","Found and executing %u prestart queries on Character tables.",result->GetRowCount());
        do
        {
            Field * f = result->Fetch();
            string e_query =  f[0].GetString();
            CharacterDatabase.Execute(e_query.c_str());
        }while(result->NextRow());

        delete result;
        CharacterDatabase.Execute("DELETE FROM prestartqueries WHERE SingleShot = 1;");
    }
}

bool World::SetInitialWorldSettings()
{
    //Perform pre-starting queries on World- and Character-DataBase
    PreStartQueries();
    CharacterDatabase.WaitExecute("UPDATE characters SET online = 0 WHERE online = 1");

    sLog.Notice("World", "Starting up...");

    Player::InitVisibleUpdateBits();

    m_lastTick = UNIXTIME;

    // TODO: clean this
    time_t tiempo;
    char hour[3];
    char minute[3];
    char second[3];
    struct tm *tmPtr;
    tiempo = UNIXTIME;
    tmPtr = localtime(&tiempo);
    strftime( hour, 3, "%H", tmPtr );
    strftime( minute, 3, "%M", tmPtr );
    strftime( second, 3, "%S", tmPtr );
    m_gameTime = (3600*atoi(hour))+(atoi(minute)*60)+(atoi(second)); // server starts at noon

    // TODO: clean this
    // fill in emotes table
    // it appears not every emote has an animation
    mPrices[1] = 10;
    mPrices[4] = 80;
    mPrices[6] = 150;
    mPrices[8] = 200;
    mPrices[10] = 300;
    mPrices[12] = 800;
    mPrices[14] = 900;
    mPrices[16] = 1800;
    mPrices[18] = 2200;
    mPrices[20] = 2300;
    mPrices[22] = 3600;
    mPrices[24] = 4200;
    mPrices[26] = 6700;
    mPrices[28] = 7200;
    mPrices[30] = 8000;
    mPrices[32] = 11000;
    mPrices[34] = 14000;
    mPrices[36] = 16000;
    mPrices[38] = 18000;
    mPrices[40] = 20000;
    mPrices[42] = 27000;
    mPrices[44] = 32000;
    mPrices[46] = 37000;
    mPrices[48] = 42000;
    mPrices[50] = 47000;
    mPrices[52] = 52000;
    mPrices[54] = 57000;
    mPrices[56] = 62000;
    mPrices[58] = 67000;
    mPrices[60] = 72000;

    // Start
    uint32 start_time = getMSTime();
    if( !LoadDBCs(sWorld.DBCPath.c_str()) )
    {
        sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "One or more of the DBC files are missing.", "These are absolutely necessary for the server to function.", "The server will not start without them.", NULL);
        return false;
    }

    new ObjectMgr();
    new QuestMgr();
    new LootMgr();
    (new LfgMgr())->LoadRandomDungeonRewards();
    new WeatherMgr();
    new TaxiMgr();
    new AddonMgr();
    new ChatHandler();
    new WarnSystem();
    new Tracker();
    new GuildMgr();
    new ItemPrototypeSystem();

    // Fill the task list with jobs to do.
    TaskList tl;

    Storage_FillTaskList(tl);

#define MAKE_TASK(sp, ptr) tl.AddTask(new Task(new CallbackP0<sp>(sp::getSingletonPtr(), &sp::ptr)))

    // spawn worker threads (2 * number of cpus)
    tl.spawn();

    /* storage stuff has to be loaded first */
    tl.wait();

    MAKE_TASK(QuestMgr, LoadQuests);

    Storage_LoadAdditionalTables();

    ThreadPool.ExecuteTask("TaskExecutor", new BasicTaskExecutor(new CallbackP0<ObjectMgr>(ObjectMgr::getSingletonPtr(),
        &ObjectMgr::LoadPlayersInfo), BTE_PRIORITY_MED));

    MAKE_TASK(ObjectMgr, LoadPlayerCreateInfo);
    MAKE_TASK(ObjectMgr, LoadSpellSkills);
    tl.wait();

    ApplyNormalFixes();
    ItemPrototypeStorage.Init();
    MAKE_TASK(GuildMgr, LoadAllGuilds);
    MAKE_TASK(GuildMgr, LoadGuildCharters);
    MAKE_TASK(ObjectMgr, LoadQuestPOI);
    MAKE_TASK(ObjectMgr, LoadRecallPoints);
    MAKE_TASK(ObjectMgr, LoadAchievements);
    MAKE_TASK(ObjectMgr, LoadCreatureWaypoints);
    MAKE_TASK(ObjectMgr, LoadTrainers);
    MAKE_TASK(ObjectMgr, LoadTotemSpells);
    MAKE_TASK(ObjectMgr, LoadSpellOverride);
    MAKE_TASK(ObjectMgr, LoadVendors);
    MAKE_TASK(ObjectMgr, LoadAIThreatToSpellId);
    MAKE_TASK(ObjectMgr, LoadSpellFixes);
    MAKE_TASK(ObjectMgr, LoadGMTickets);
    MAKE_TASK(ObjectMgr, LoadPetLevelupSpellMap);
    MAKE_TASK(AddonMgr,  LoadFromDB);
    MAKE_TASK(ObjectMgr, SetHighestGuids);
    MAKE_TASK(ObjectMgr, ListGuidAmounts);
    MAKE_TASK(ObjectMgr, HashWMOAreaTables);
    MAKE_TASK(ObjectMgr, LoadReputationModifiers);
    MAKE_TASK(ObjectMgr, LoadMonsterSay);
    MAKE_TASK(WeatherMgr,LoadFromDB);
    MAKE_TASK(ObjectMgr, LoadGroups);
    MAKE_TASK(Tracker,   LoadFromDB);
    MAKE_TASK(ObjectMgr, LoadExtraGameObjectStuff);
    MAKE_TASK(ObjectMgr, LoadExtraCreatureProtoStuff);
    MAKE_TASK(ObjectMgr, LoadExtraItemStuff);
    MAKE_TASK(ObjectMgr, LoadArenaTeams);
    MAKE_TASK(ObjectMgr, LoadProfessionDiscoveries);

#undef MAKE_TASK

    // wait for all loading to complete.
    tl.wait();

    // start mail system
    sLog.Notice("World","Starting Mail System...");
    MailSystem::getSingleton().StartMailSystem();

    sLog.Notice("World", "Starting Auction System...");
    new AuctionMgr;
    sAuctionMgr.LoadAuctionHouses();

    sComTableStore.Load();
    sLog.Notice("WordFilter", "Loading...");

    g_characterNameFilter = new WordFilter();
    g_chatFilter = new WordFilter();
    g_characterNameFilter->Load("wordfilter_character_names");
    g_chatFilter->Load("wordfilter_chat");
    sLog.Success("World", "Database loaded in %ums.", getMSTime() - start_time);

    if(Collision)
    {
        sVMapInterface.Init();
        if(PathFinding)
            NavMeshInterface.Init();
    }

    sScriptMgr.LoadScripts();

    // calling this puts all maps into our task list.
    sInstanceMgr.Load(&tl);

    // wait for the events to complete.
    tl.wait();

    // wait for them to exit, now.
    tl.kill();
    tl.waitForThreadsToExit();

    LoadNameGenData();
    sLog.Notice("World", "Object size: %u bytes", sizeof(Object));
    sLog.Notice("World", "GameObject size: %u bytes", sizeof(GameObject));
    sLog.Notice("World", "AI size: %u bytes", sizeof(AIInterface));
    sLog.Notice("World", "Unit size: %u bytes", sizeof(Unit) + sizeof(AIInterface));
    sLog.Notice("World", "Creature size: %u bytes", sizeof(Creature) + sizeof(AIInterface));
    sLog.Notice("World", "Vehicle size: %u bytes", sizeof(Vehicle) + sizeof(AIInterface));
    sLog.Notice("World", "Player size: %u bytes", sizeof(Player) + sizeof(AIInterface) + sizeof(ItemInterface) + sizeof(WorldSession) + sizeof(MapMgr));
    sLog.Notice("World", "Single Player size: %u bytes", sizeof(Player) + sizeof(AIInterface));
    sLog.Notice("World", "MapMgr size: %u bytes", sizeof(MapMgr));
    sLog.Notice("World", "ItemInterface size: %u bytes", sizeof(ItemInterface));
    sLog.Notice("World", "WorldSession size: %u bytes", sizeof(WorldSession));
    sLog.Notice("World","Starting Transport System...");
    objmgr.LoadTransporters();

    ThreadPool.ExecuteTask("DayWatcherThread", new DayWatcherThread());

    if(mainIni->ReadBoolean("Startup", "BackgroundLootLoading", true))
    {
        sLog.Notice("World", "Background loot loading...");

        // loot background loading in a lower priority thread.
        ThreadPool.ExecuteTask("LootLoader", new BasicTaskExecutor(new CallbackP0<LootMgr>(LootMgr::getSingletonPtr(), &LootMgr::LoadDelayedLoot),
            BTE_PRIORITY_LOW));
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

    sLog.Notice("World", "Starting InsertQueueLoader...");
    new InsertQueueLoader();

    if(GuildsLoading)
    {
        sLog.Notice( "World", "Waiting for groups and players to finish loading..." );
        while(GuildsLoading)
            Sleep( 100 );
    }

    int8 team = -1;
    AreaTableEntry *areaentry = NULL;
    for( ConstructDBCStorageIterator(AreaTableEntry) area_itr = dbcAreaTable.begin(); area_itr != dbcAreaTable.end(); ++area_itr )
    {
        areaentry = (*area_itr);
        if(areaentry == NULL)
            continue;

        if(Sanctuaries.find(areaentry->AreaId) == Sanctuaries.end())
        {
            if(areaentry->category == AREAC_SANCTUARY || areaentry->AreaFlags & AREA_SANCTUARY)
                Sanctuaries.insert(areaentry->AreaId);
        }

        if(RestedAreas.find(areaentry->AreaId) == RestedAreas.end())
        {
            if(areaentry->AreaFlags & AREA_CITY_AREA || areaentry->AreaFlags & AREA_CITY || areaentry->AreaFlags & AREA_CAPITAL_SUB || areaentry->AreaFlags & AREA_CAPITAL)
            {
                team = -1;
                if(areaentry->category == AREAC_ALLIANCE_TERRITORY)
                    team = ALLIANCE;
                if(areaentry->category == AREAC_HORDE_TERRITORY)
                    team = HORDE;
                SetRestedArea(areaentry->AreaId, team);
            }

        }
        areaentry = NULL;
    }
    sLog.Notice("World", "Hashed %u sanctuaries", Sanctuaries.size());

    sEventMgr.AddEvent(CAST(World,this), &World::CheckForExpiredInstances, EVENT_WORLD_UPDATEAUCTIONS, 120000, 0, 0);
    return true;
}

void World::Update(time_t diff)
{
    uint32 pDiff = uint32(diff);

    _UpdateGameTime();

    UpdateQueuedSessions(pDiff);

    if(AuctionMgr::getSingletonPtr() != NULL)
        sAuctionMgr.Update();

    if(InsertQueueLoader::getSingletonPtr() != NULL)
        sIQL.Update(pDiff);

    if(MailSystem::getSingletonPtr() != NULL)
        sMailSystem.UpdateMessages(pDiff);

    if(GuildMgr::getSingletonPtr() != NULL)
        guildmgr.Update(pDiff);
}

void World::SendMessageToGMs(WorldSession *self, const char * text, ...)
{
    char buf[500];
    va_list ap;
    va_start(ap, text);
    vsnprintf(buf, 2000, text, ap);
    va_end(ap);
    WorldSession *gm_session;

    WorldPacket *data = sChatHandler.FillSystemMessageData(buf);
    gmList_lock.AcquireReadLock();
    SessionSet::iterator itr;
    for (itr = gmList.begin(); itr != gmList.end();itr++)
    {
        gm_session = (*itr);
        if(gm_session->GetPlayer() != NULL && gm_session != self)  // dont send to self!)
            gm_session->SendPacket(data);
    }
    gmList_lock.ReleaseReadLock();
    delete data;
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
    uint32 textLen = (uint32)strlen((char*)text) + 1;

    WorldPacket data(textLen + 40);

    data.Initialize(SMSG_MESSAGECHAT);
    data << uint8(CHAT_MSG_SYSTEM);
    data << uint32(LANG_UNIVERSAL);

    data << (uint64)0; // Who cares about guid when there's no nickname displayed heh ?
    data << (uint32)0;
    data << (uint64)0;

    data << textLen;
    data << text;
    data << uint8(0);

    SendGlobalMessage(&data, self);
}

void World::SendGMWorldText(const char* text, bool admin)
{
    uint32 textLen = (uint32)strlen((char*)text) + 1;

    WorldPacket data(textLen + 40);
    data.Initialize(SMSG_MESSAGECHAT);
    data << uint8(CHAT_MSG_SYSTEM);
    data << uint32(LANG_UNIVERSAL);
    data << (uint64)0;
    data << (uint32)0;
    data << (uint64)0;
    data << textLen;
    data << text;
    data << uint8(0);
    if(admin)
        SendAdministratorMessage(&data);
    else
        SendGamemasterMessage(&data);
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
        if( GlobalSession->GetInstance() != 0 )
        {
            RemoveGlobalSession(GlobalSession);
            continue;
        }

        result = GlobalSession->Update(0);
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

void World::DeleteGlobalSession(WorldSession *GlobalSession)
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
    m_sessions.erase(GlobalSession->GetAccountId());
    m_sessionlock.ReleaseWriteLock();

    // Remove us from the map
    RemoveGlobalSession(GlobalSession);

    // delete us
    GlobalSession->Delete();
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
        m_queueUpdateTimer = 180000;
        queueMutex.Acquire();

        if(mQueuedSessions.size() == 0)
        {
            queueMutex.Release();
            return;
        }

        while(m_sessions.size() < m_playerLimit && mQueuedSessions.size())
        {
            // Yay. We can let another player in now.
            // Grab the first fucker from the queue, but guard of course, since
            // this is in a different thread again.

            QueueSet::iterator iter = mQueuedSessions.begin();
            WorldSocket * QueuedSocket = *iter;
            mQueuedSessions.erase(iter);

            // Welcome, sucker.
            if(QueuedSocket->GetSession())
            {
                QueuedSocket->GetSession()->deleteMutex.Acquire();
                QueuedSocket->Authenticate();
                QueuedSocket->GetSession()->deleteMutex.Release();
            }
        }

        if(mQueuedSessions.size() == 0)
        {
            queueMutex.Release();
            return;
        }

        // Update the remaining queue members.
        QueueSet::iterator iter = mQueuedSessions.begin();
        uint32 Position = 1;
        while(iter != mQueuedSessions.end())
        {
            (*iter)->SendAuthResponse(AUTH_WAIT_QUEUE, true, Position++);
            ++iter;
        }
        queueMutex.Release();
    }
    else
    {
        m_queueUpdateTimer -= diff;
    }
}

void World::SaveAllPlayers()
{
    if(!(ObjectMgr::getSingletonPtr()))
        return;

    sLog.outString("Saving all players to database...");
    uint32 count = 0;
    PlayerStorageMap::const_iterator itr;
        // Servers started and obviously runing. lets save all players.
    uint32 mt;
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
    int gm = 0;
    int count = 0;
    int avg = 0;
    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        if(itr->second->GetSession())
        {
            count++;
            avg += itr->second->GetSession()->GetLatency();
            if(itr->second->GetSession()->GetPermissionCount())
                gm++;
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    *AverageLatency = count ? (float)((float)avg / (float)count) : 0;
    *GMCount = gm;
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
    for(set<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
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
    ThreadContext();
    running = true;
    thread_count = 0;

    uint32 threadcount;
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
    else
        threadcount = 1;

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
        for(set<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
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
        t = starter->GetTask();
        if(t)
        {
            t->execute();
            t->completed = true;
            starter->RemoveTask(t);
            delete t;
        }
        else
            Delay(20);
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

void World::DeleteObject(Object* obj)
{
    obj->Destruct();
    obj = NULLOBJ;
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

    // Script configs
    LuaScriptPath = mainIni->ReadString("Script", "LuaScriptsLocation", "scripts");
    GameMonkeyScriptPath = mainIni->ReadString("Script", "GameMonkeyScriptsLocation", "scripts");

    // Data configs
    DBCPath = mainIni->ReadString("Data", "DBCPath", "dbc");
    MapPath = mainIni->ReadString("Data", "MapPath", "maps");
    vMapPath = mainIni->ReadString("Data", "vMapPath", "vmaps");
    MMapPath = mainIni->ReadString("Data", "MMapPath", "mmaps");

    // Performance configs
    Collision = mainIni->ReadBoolean("PerformanceSettings", "Collision", false);
    PathFinding = mainIni->ReadBoolean("PerformanceSettings", "Pathfinding", false);
    CalculatedHeightChecks = mainIni->ReadBoolean("PerformanceSettings", "CHeightChecks", false);
    AreaUpdateDistance = mainIni->ReadFloat("PerformanceSettings", "AreaUpdateDistance", false);

    // Server Configs
    StartGold = mainIni->ReadInteger("ServerSettings", "StartGold", 1);
    StartLevel = mainIni->ReadInteger("ServerSettings", "StartLevel", 1);
    MaxLevelCalc = mainIni->ReadInteger("ServerSettings", "MaxLevelCalc", MAXIMUM_ATTAINABLE_LEVEL);
    m_useAccountData = mainIni->ReadBoolean("ServerSettings", "UseAccountData", false);
    SetMotd(mainIni->ReadString("ServerSettings", "Motd", "Hearthstone Default MOTD").c_str());
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
    ApplyHolidayConfigMaskOverride();

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

    antihack_teleport = mainIni->ReadBoolean("AntiHack", "Teleport", true);
    antihack_speed = mainIni->ReadBoolean("AntiHack", "Speed", true);
    antihack_flight = mainIni->ReadBoolean("AntiHack", "Flight", true);
    no_antihack_on_gm = mainIni->ReadBoolean("AntiHack", "DisableOnGM", false);

    m_speedHackThreshold = mainIni->ReadFloat("AntiHack", "SpeedThreshold", -500.0f);
    m_speedHackLatencyMultiplier = mainIni->ReadFloat("AntiHack", "SpeedLatencyCompensation", 0.25f);
    antihack_cheatengine = mainIni->ReadBoolean("AntiHack", "CheatEngine", false);
    m_CEThreshold = mainIni->ReadInteger("AntiHack", "CheatEngineTimeDiff", 10000);
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
        if(dbc.getRecord(i).getString(1)==NULL)
            continue;

        d.name = string(dbc.getRecord(i).getString(1));
        d.type = dbc.getRecord(i).getUInt(3);
        _namegendata[d.type].push_back(d);
    }
}

void World::CharacterEnumProc(QueryResultVector& results, uint32 AccountId)
{
    WorldSession * s = FindSession(AccountId);
    if(s == NULL)
        return;

    s->CharacterEnumProc(results[0].result);
}

void World::CheckForExpiredInstances()
{
    sInstanceMgr.CheckForExpiredInstances();
}

InsertQueueLoader::InsertQueueLoader()
{
    m_UpdateTimer = 0;
}

InsertQueueLoader::~InsertQueueLoader()
{

}

void InsertQueueLoader::Update(uint32 timeDiff)
{
    m_UpdateTimer += timeDiff;
    if(m_UpdateTimer > 60000)
    {
        m_UpdateTimer -= 60000;

        // Get a single connection to maintain for the whole process.
        DatabaseConnection * con = CharacterDatabase.GetFreeConnection();

        // External Character Import. Updated for character structure r1590
        sWorld.PollCharacterInsertQueue(con);

        // External mail Import.
        sWorld.PollMailboxInsertQueue(con);

        //release the lock obtained in GetFreeConnection
        con->Busy.Release();
    }
}

void World::PollMailboxInsertQueue(DatabaseConnection * con)
{
    QueryResult * result;
    Field * f;
    Item* pItem;
    uint32 itemid;
    uint32 stackcount;

    result = CharacterDatabase.FQuery("SELECT * FROM mailbox_insert_queue", con);
    if( result != NULL )
    {
        sLog.Debug("MailboxQueue", "Sending queued messages....");
        do
        {
            f = result->Fetch();
            itemid = f[6].GetUInt32();
            stackcount = f[7].GetUInt32();

            if( itemid != 0 )
            {
                pItem = objmgr.CreateItem( itemid, NULLPLR );
                if( pItem != NULL )
                {
                    pItem->SetUInt32Value( ITEM_FIELD_STACK_COUNT, stackcount );
                    pItem->SaveToDB( 0, 0, true, NULL );
                }
            }
            else
                pItem = NULLITEM;

            sLog.Debug("MailboxQueue", "Sending message to %u (item: %u)...", f[1].GetUInt32(), itemid);
            sMailSystem.DeliverMessage( 0, f[0].GetUInt64(), f[1].GetUInt64(), f[2].GetString(), f[3].GetString(), f[5].GetUInt32(),
                0, pItem ? pItem->GetGUID() : 0, f[4].GetUInt32(), true );

            if( pItem != NULL )
            {
                pItem->DeleteMe();
            }

        } while ( result->NextRow() );
        delete result;
        sLog.Debug("MailboxQueue", "Done.");
        CharacterDatabase.FWaitExecute("DELETE FROM mailbox_insert_queue", con);
    }
}

void World::PollCharacterInsertQueue(DatabaseConnection * con)
{
    QueryResult * result = CharacterDatabase.FQuery("SELECT guid FROM characters_insert_queue", con);
    if(!result)
        return;
    else
        delete result;

    // Our local stuff..
    Field * f;
    map<uint32, vector<insert_playeritem> > itemMap;
    map<uint32, vector<insert_playeritem> >::iterator itr;
    map<uint32, vector<insert_playerskill> > skillMap;
    map<uint32, vector<insert_playerskill> >::iterator itr1;
    map<uint32, vector<insert_playerquest> > questMap;
    map<uint32, vector<insert_playerquest> >::iterator itr2;
    map<uint32, vector<insert_playerglyph> > glyphMap;
    map<uint32, vector<insert_playerglyph> >::iterator itr3;
    map<uint32, vector<insert_playertalent> > talentMap;
    map<uint32, vector<insert_playertalent> >::iterator itr4;
    map<uint32, vector<insert_playerspell> > spellMap;
    map<uint32, vector<insert_playerspell> >::iterator itr5;
    insert_playeritem ipi;
    insert_playerskill ips;
    insert_playerquest ipq;
    insert_playerglyph ipg;
    insert_playertalent ipt;
    insert_playerspell ipsp;

    bool has_results = false;

    //The format of our character database (applies to revision 1740)
    static const char * characterTableFormat = "xuSuuuuuussuuuuuuuuuuuuuuuffffuususuufffuuuuusuuuUuuuuffffuuuuufffssssssuuuuuuuuuuu";

    // Lock the tables to prevent any more inserts
    CharacterDatabase.FWaitExecute("LOCK TABLES characters_insert_queue WRITE, playeritems_insert_queue WRITE, playerskills_insert_queue WRITE, questlog_insert_queue WRITE, playerglyphs_insert_queue WRITE, playerspells_insert_queue WRITE, playertalents_insert_queue WRITE", con);

    // Caching will save us doing additional queries and slowing down the db.
    // Cache all items in memory.
    result = CharacterDatabase.FQuery("SELECT * FROM playeritems_insert_queue", con);
    if(result)
    {
        do
        {
            f = result->Fetch();

            ipi.ownerguid = f[0].GetUInt32();
            //skip itemguid, we'll generate a new one.
            ipi.entry = f[2].GetUInt32();
            ipi.wrapped_item_id = f[3].GetUInt32();
            ipi.wrapped_creator = f[4].GetUInt32();
            ipi.creator = f[5].GetUInt32();
            ipi.count = f[6].GetUInt32();
            ipi.charges = f[7].GetUInt32();
            ipi.flags = f[8].GetUInt32();
            ipi.randomprop = f[9].GetUInt32();
            ipi.randomsuffix = f[10].GetUInt32();
            ipi.itemtext = f[11].GetUInt32();
            ipi.durability = f[12].GetUInt32();
            ipi.containerslot = f[13].GetInt32();
            ipi.slot = f[14].GetInt32();
            ipi.enchantments = string(f[15].GetString());

            itr = itemMap.find(ipi.ownerguid);
            if(itr == itemMap.end())
            {
                vector<insert_playeritem> to_insert;
                to_insert.push_back(ipi);
                itemMap.insert(make_pair(ipi.ownerguid,to_insert));
            }
            else
            {
                itr->second.push_back(ipi);
            }

        } while(result->NextRow());
        delete result;
    }

    // Cache all skills in memory.
    result = CharacterDatabase.FQuery("SELECT * FROM playerskills_insert_queue", con);
    if(result)
    {
        do
        {
            f = result->Fetch();

            ips.player_guid = f[0].GetUInt32();
            ips.skill_id = f[1].GetUInt32();
            ips.type = f[2].GetUInt32();
            ips.currentlvl = f[3].GetUInt32();
            ips.maxlvl = f[4].GetUInt32();

            itr1 = skillMap.find(ips.player_guid);
            if(itr1 == skillMap.end())
            {
                vector<insert_playerskill> to_insert;
                to_insert.push_back(ips);
                skillMap.insert(make_pair(ips.player_guid,to_insert));
            }
            else
            {
                itr1->second.push_back(ips);
            }

        } while(result->NextRow());
        delete result;
    }

    // Cache all questlogs in memory.
    result = CharacterDatabase.FQuery("SELECT * FROM questlog_insert_queue", con);
    if(result)
    {
        do
        {
            f = result->Fetch();

            ipq.player_guid = f[0].GetUInt32();
            ipq.quest_id = f[1].GetUInt32();
            ipq.slot = f[2].GetUInt32();
            ipq.time_left = f[3].GetUInt32();
            ipq.explored_area1 = f[4].GetUInt32();
            ipq.explored_area2 = f[5].GetUInt32();
            ipq.explored_area3 = f[6].GetUInt32();
            ipq.explored_area4 = f[7].GetUInt32();
            ipq.mob_kill1 = f[8].GetUInt32();
            ipq.mob_kill2 = f[9].GetUInt32();
            ipq.mob_kill3 = f[10].GetUInt32();
            ipq.mob_kill4 = f[11].GetUInt32();
            ipq.slain = f[12].GetUInt32();

            itr2 = questMap.find(ipq.player_guid);
            if(itr2 == questMap.end())
            {
                vector<insert_playerquest> to_insert;
                to_insert.push_back(ipq);
                questMap.insert(make_pair(ipq.player_guid,to_insert));
            }
            else
            {
                itr2->second.push_back(ipq);
            }

        } while(result->NextRow());
        delete result;
    }

    // Cache all glyphs in memory.
    result = CharacterDatabase.FQuery("SELECT * FROM playerglyphs_insert_queue", con);
    if(result)
    {
        do
        {
            f = result->Fetch();

            ipg.player_guid = f[0].GetUInt32();
            ipg.spec = f[1].GetUInt32();
            ipg.glyph1 = f[2].GetUInt32();
            ipg.glyph2 = f[3].GetUInt32();
            ipg.glyph3 = f[4].GetUInt32();
            ipg.glyph4 = f[5].GetUInt32();
            ipg.glyph5 = f[6].GetUInt32();
            ipg.glyph6 = f[7].GetUInt32();

            itr3 = glyphMap.find(ipg.player_guid);
            if(itr3 == glyphMap.end())
            {
                vector<insert_playerglyph> to_insert;
                to_insert.push_back(ipg);
                glyphMap.insert(make_pair(ipg.player_guid,to_insert));
            }
            else
            {
                itr3->second.push_back(ipg);
            }

        } while(result->NextRow());
        delete result;
    }

    // Cache all talents in memory.
    result = CharacterDatabase.FQuery("SELECT * FROM playertalents_insert_queue", con);
    if(result)
    {
        do
        {
            f = result->Fetch();

            ipt.player_guid = f[0].GetUInt32();
            ipt.spec = f[1].GetUInt32();
            ipt.tid = f[2].GetUInt32();
            ipt.rank = f[3].GetUInt32();

            itr4 = talentMap.find(ipt.player_guid);
            if(itr4 == talentMap.end())
            {
                vector<insert_playertalent> to_insert;
                to_insert.push_back(ipt);
                talentMap.insert(make_pair(ipt.player_guid,to_insert));
            }
            else
            {
                itr4->second.push_back(ipt);
            }

        } while(result->NextRow());
        delete result;
    }

    // Cache all spells in memory.
    result = CharacterDatabase.FQuery("SELECT * FROM playerspells_insert_queue", con);
    if(result)
    {
        do
        {
            f = result->Fetch();

            ipsp.player_guid = f[0].GetUInt32();
            ipsp.spellid = f[1].GetUInt32();

            itr5 = spellMap.find(ipsp.player_guid);
            if(itr5 == spellMap.end())
            {
                vector<insert_playerspell> to_insert;
                to_insert.push_back(ipsp);
                spellMap.insert(make_pair(ipsp.player_guid,to_insert));
            }
            else
            {
                itr5->second.push_back(ipsp);
            }

        } while(result->NextRow());
        delete result;
    }

    // Load the characters, and assign them their new guids, and insert them into the main db.
    result = CharacterDatabase.FQuery("SELECT * FROM characters_insert_queue", con);

    // Can be unlocked now.
    CharacterDatabase.FWaitExecute("UNLOCK TABLES", con);

    if(result)
    {
        uint32 guid;
        std::stringstream ss;
        uint32 queuesize = result->GetRowCount();
        do
        {
            f = result->Fetch();
            char * p = (char*)characterTableFormat;
            uint32 i = 0;
            guid = f[0].GetUInt32();

            //Generate a new player guid
            uint32 new_guid = objmgr.GenerateLowGuid(HIGHGUID_TYPE_PLAYER);

            // Create his playerinfo in the server
            PlayerInfo * inf = new PlayerInfo();
            memset(inf, 0, sizeof(PlayerInfo));
            inf->guid = new_guid;
            inf->acct = f[1].GetUInt32();
            inf->_class = f[4].GetUInt32();
            inf->race=f[3].GetUInt32();
            inf->gender = f[5].GetUInt32();
            inf->lastLevel = f[7].GetUInt32();
            inf->lastOnline = UNIXTIME;
            switch(inf->race)
            {
                case RACE_HUMAN:
                case RACE_GNOME:
                case RACE_DWARF:
                case RACE_NIGHTELF:
                case RACE_DRAENEI:
                    {
                        inf->team=0;
                    }break;
                default:
                    {
                        inf->team=1;
                    }break;
            }

            // Build our query
            ss << "INSERT INTO characters VALUES(" << new_guid;
            while(*p != 0)
            {
                switch(*p)
                {
                case 's':
                    ss << ",'" << CharacterDatabase.EscapeString(f[i].GetString(), con) << "'";
                    break;

                case 'f':
                    ss << ",'" << f[i].GetFloat() << "'";
                    break;

                case 'S':
                    {
                        // this is the character name, append a hex version of the guid to it to prevent name clashes.
                        char newname[100];
                        snprintf(newname,20,"%5s%X",f[i].GetString(),new_guid);
                        ss << ",'" << CharacterDatabase.EscapeString(newname,con) << "'";
                        inf->name = strdup(newname);
                    }break;

                case 'U':
                    {
                        // this is our forced rename field. force it to one.
                        ss << ",1";
                    }break;

                case 'x':
                    {
                        // players guid (we generate a new one)
                    }break;
                default:
                    ss << "," << f[i].GetUInt32();
                    break;
                }

                ++i;
                ++p;
            }

            ss << ")";
            //Execute the query
            CharacterDatabase.FWaitExecute(ss.str().c_str(),con);

            // Add playerinfo to the objectmgr
            objmgr.AddPlayerInfo(inf);

            // grab all his items, assign them their new guids and insert them
            itr = itemMap.find(guid);
            if(itr != itemMap.end())
            {
                for(vector<insert_playeritem>::iterator vtr = itr->second.begin(); vtr != itr->second.end(); ++vtr)
                {
                    //Generate a new item guid
                    uint32 new_item_guid = objmgr.GenerateLowGuid(HIGHGUID_TYPE_ITEM);

                    //empty our query string
                    ss.rdbuf()->str("");

                    // Build query
                    ss << "INSERT INTO playeritems VALUES(";
                    ss << new_guid << ","
                        << new_item_guid << ","
                        << (*vtr).entry << ","
                        << (*vtr).wrapped_item_id << ","
                        << (*vtr).wrapped_creator << ","
                        << (*vtr).creator << ","
                        << (*vtr).count << ","
                        << (*vtr).charges << ","
                        << (*vtr).flags << ","
                        << (*vtr).randomprop << ","
                        << (*vtr).randomsuffix << ","
                        << (*vtr).itemtext << ","
                        << (*vtr).durability << ","
                        << (*vtr).containerslot << ","
                        << (*vtr).slot << ",'"
                        << (*vtr).enchantments << "')";
                    //Execute query
                    CharacterDatabase.FWaitExecute(ss.str().c_str(),con);
                }
            }

            // grab all his skills, assign them their new guids and insert them
            itr1 = skillMap.find(guid);
            if(itr1 != skillMap.end())
            {
                for(vector<insert_playerskill>::iterator vtr1 = itr1->second.begin(); vtr1 != itr1->second.end(); ++vtr1)
                {
                    //empty our query string
                    ss.rdbuf()->str("");

                    // Build query
                    ss << "INSERT INTO playerskills VALUES(";
                    ss << new_guid << ","
                        << (*vtr1).skill_id << ","
                        << (*vtr1).type << ","
                        << (*vtr1).currentlvl << ","
                        << (*vtr1).maxlvl << " )";
                    //Execute query
                    CharacterDatabase.FWaitExecute(ss.str().c_str(),con);
                }
            }

            // grab all his quests, assign them their new guids and insert them
            itr2 = questMap.find(guid);
            if(itr2 != questMap.end())
            {
                for(vector<insert_playerquest>::iterator vtr2 = itr2->second.begin(); vtr2 != itr2->second.end(); ++vtr2)
                {
                    //empty our query string
                    ss.rdbuf()->str("");

                    // Build query
                    ss << "INSERT INTO questlog VALUES(";
                    ss << new_guid << ","
                        << (*vtr2).quest_id << ","
                        << (*vtr2).slot << ","
                        << (*vtr2).time_left << ","
                        << (*vtr2).explored_area1 << ","
                        << (*vtr2).explored_area2 << ","
                        << (*vtr2).explored_area3 << ","
                        << (*vtr2).explored_area4 << ","
                        << (*vtr2).mob_kill1 << ","
                        << (*vtr2).mob_kill2 << ","
                        << (*vtr2).mob_kill3 << ","
                        << (*vtr2).mob_kill4 << ","
                        << (*vtr2).slain << ")";
                    //Execute query
                    CharacterDatabase.FWaitExecute(ss.str().c_str(),con);
                }
            }

            // grab all his glyphs, assign them their new guids and insert them
            itr3 = glyphMap.find(guid);
            if(itr3 != glyphMap.end())
            {
                for(vector<insert_playerglyph>::iterator vtr3 = itr3->second.begin(); vtr3 != itr3->second.end(); ++vtr3)
                {
                    //empty our query string
                    ss.rdbuf()->str("");

                    // Build query
                    ss << "INSERT INTO playerglyphs VALUES(";
                    ss << new_guid << ","
                        << (*vtr3).spec << ","
                        << (*vtr3).glyph1 << ","
                        << (*vtr3).glyph2 << ","
                        << (*vtr3).glyph3 << ","
                        << (*vtr3).glyph4 << ","
                        << (*vtr3).glyph5 << ","
                        << (*vtr3).glyph6 << " )";
                    //Execute query
                    CharacterDatabase.FWaitExecute(ss.str().c_str(),con);
                }
            }

            // grab all his talents, assign them their new guids and insert them
            itr4 = talentMap.find(guid);
            if(itr4 != talentMap.end())
            {
                for(vector<insert_playertalent>::iterator vtr4 = itr4->second.begin(); vtr4 != itr4->second.end(); ++vtr4)
                {
                    //empty our query string
                    ss.rdbuf()->str("");

                    // Build query
                    ss << "INSERT INTO playertalents VALUES(";
                    ss << new_guid << ","
                        << (*vtr4).spec << ","
                        << (*vtr4).tid << ","
                        << (*vtr4).rank << " )";
                    //Execute query
                    CharacterDatabase.FWaitExecute(ss.str().c_str(),con);
                }
            }

            // grab all his spells, assign them their new guids and insert them
            itr5 = spellMap.find(guid);
            if(itr5 != spellMap.end())
            {
                for(vector<insert_playerspell>::iterator vtr5 = itr5->second.begin(); vtr5 != itr5->second.end(); ++vtr5)
                {
                    //empty our query string
                    ss.rdbuf()->str("");

                    // Build query
                    ss << "INSERT INTO playerspells VALUES(";
                    ss << new_guid << ","
                        << (*vtr5).spellid << " )";
                    //Execute query
                    CharacterDatabase.FWaitExecute(ss.str().c_str(),con);
                }
            }

            //empty our query string
            ss.rdbuf()->str("");
        } while(result->NextRow());
        has_results = true;
        sLog.Debug("CharacterLoader","Imported %u character(s) from external queue",queuesize);
        delete result;
    }

    // Clear all the data in the tables.
    if(has_results)
    {
        CharacterDatabase.FWaitExecute("DELETE FROM characters_insert_queue", con);
        CharacterDatabase.FWaitExecute("DELETE FROM playeritems_insert_queue", con);
        CharacterDatabase.FWaitExecute("DELETE FROM playerskills_insert_queue", con);
        CharacterDatabase.FWaitExecute("DELETE FROM questlog_insert_queue", con);
        CharacterDatabase.FWaitExecute("DELETE FROM playerglyphs_insert_queue", con);
        CharacterDatabase.FWaitExecute("DELETE FROM playertalents_insert_queue", con);
        CharacterDatabase.FWaitExecute("DELETE FROM playerspells_insert_queue", con);
    }
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

        string ip2 = worldsession->GetSocket()->GetIP();
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

string World::GetUptimeString()
{
    char str[300];
    time_t pTime = (time_t)UNIXTIME - m_StartTime;
    tm * tmv = gmtime(&pTime);

    snprintf(str, 300, "%u days, %u hours, %u minutes, %u seconds.", tmv->tm_yday, tmv->tm_hour, tmv->tm_min, tmv->tm_sec);
    return string(str);
}

void World::UpdateShutdownStatus()
{
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
        sEventMgr.RemoveEvents(CAST(World,this), EVENT_WORLD_SHUTDOWN);
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

    // add event
    sEventMgr.AddEvent(this, &World::UpdateShutdownStatus, EVENT_WORLD_SHUTDOWN, 50, 0, 0);

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
      "banned_names", "characters", "characters_insert_queue", "charters", "corpses", "gm_tickets",
      "groups", "guild_bankitems", "guild_banklogs", "guild_banktabs",
      "guild_data", "guild_logs", "guild_ranks", "guilds",
      "instances", "mailbox", "mailbox_insert_queue", "news_timers",
      "playercooldowns", "playeritems", "playeritems_insert_queue", "playerpets",
      "playerpetspells", "playerpettalents", "playersummons", "playersummonspells", "questlog",
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

void World::LogGM(WorldSession* session, string message, ...)
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

void World::LogCheater(WorldSession* session, string message, ...)
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

void World::LogPlayer(WorldSession* session, string message, ...)
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

void World::LogChat(WorldSession* session, string message, ...)
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

void World::SetAnniversary(uint32 anniversarynumber)
{
    // Set these here.
    RealAchievement = false;

    // Crow: The rest is handled via the achievement system, so just set achievements.
    switch(anniversarynumber)
    {
    case 4:
        {
            AnniversaryAchievement = 2398;
        }break;
    case 5:
        {
            AnniversaryAchievement = 4400;
        }break;
    case 6:
        {
            // WHAT?! NO PET?!!? RAGE!!!!!!!!!
            // Note, this won't work with 3.3.5, its just here for flavor.
            AnniversaryAchievement = 5512;
        }break;
    }

    if(AnniversaryAchievement)
        RealAchievement = (dbcAchievement.LookupEntry(AnniversaryAchievement) != NULL);
}

void World::OnHolidayChange(uint32 IgnoreHolidayId)
{
    sLog.Notice("World", "Cleaning holiday items...");
    m_sessionlock.AcquireReadLock();
    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
        if(itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld())
            itr->second->GetPlayer()->GetItemInterface()->RemoveItemsWithHolidayId(IgnoreHolidayId);

    m_sessionlock.ReleaseReadLock();
    std::stringstream ss;
    ss << "DELETE FROM `playeritems` WHERE `entry` IN(";
    ItemPrototypeSystem::iterator iter = ItemPrototypeStorage.begin();
    ItemPrototype * pItemPrototype;
    bool first = true;
    while(iter != ItemPrototypeStorage.end())
    {
        pItemPrototype = (*iter)->second;
        if(pItemPrototype->HolidayId != 0 && pItemPrototype->HolidayId != IgnoreHolidayId)
        {
            if(first)
                first = false;
            else
                ss << ", ";
            ss << "'" << uint32(pItemPrototype->ItemId) << "'";
        }

        ++iter;
    }

    if(!first)
    {
        CharacterDatabase.Execute(ss.str().c_str());
        sLog.Notice("World", "Holiday Items Cleaned!");
    }
    sLog.Notice("World", "Done cleaning Holiday Items.");
}
