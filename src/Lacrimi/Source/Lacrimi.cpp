/*
 * Lacrimi Scripts Copyright 2010 - 2011
 *
 * ############################################################
 * # ##            #       ####### ####### ##    #    #    ## #
 * # ##           ###      ##      ##   ## ##   ###  ###   ## #
 * # ##          ## ##     ##      ##   ## ##   ###  ###   ## #
 * # ##         #######    ##      ####### ##  ## #### ##  ## #
 * # ##        ##     ##   ##      #####   ##  ## #### ##  ## #
 * # ##       ##       ##  ##      ##  ##  ## ##   ##   ## ## #
 * # ####### ##         ## ####### ##   ## ## ##   ##   ## ## #
 * # :::::::.::.........::.:::::::.::...::.::.::...::...::.:: #
 * ############################################################
 *
 */

#include "LacrimiStdAfx.h"

Lacrimi::Lacrimi(ScriptMgr* mgr) : ThreadContext()
{
    first = true;
    config = true;
    LacrimiDB = NULL;
    database = false;
    dumpstats = false;
    LuaEngineIsStarting = false;
    sMgr = mgr;
}

Lacrimi::~Lacrimi()
{

}

bool Lacrimi::run()
{
    Delay(400);
    if(GetConfigBool("Features", "LuaEngine", true))
    {
        L_LuaEngineMgr = new LuaEngineMgr();
        L_LuaEngineMgr->Startup();
        Delay(100);
        while(LuaEngineIsStarting)
            Delay(100);
    }

    uint32 curTime = getMSTime();
    uint32 m_StatDumpTimer = curTime+15000, m_CleanupDelay = curTime+10000;
    while(GetThreadState() != THREADSTATE_SELF_TERMINATE)
    {
        curTime = getMSTime();
        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        if(dumpstats)
        {
            if(curTime > m_StatDumpTimer)
            {
                DumpStats();
                m_StatDumpTimer = curTime+60000;
            }
        }

        if(curTime > m_CleanupDelay)
            Cleanup();
        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;
        Delay(5);
    }
    sLog.Notice("Lacrimi", "Terminating...");

    FinalCleanup();
    if(database)
        _StopDB();
    OnShutdown();
    return true;
}

void Lacrimi::Cleanup()
{
    CleanupComponent* CC = NULL;
    uint32 current = getMSTime(), timer = 0;
    for(std::set<CleanupComponent*>::iterator itr = CleanupComponents.begin(); itr != CleanupComponents.end(); itr++)
    {
        timer = ((*itr)->LastCall+(*itr)->CallDelay);
        if(current >= timer)
        {
            (*itr)->LastCall = current;
            (*itr)->CleanupFunction((*itr)->CallDelay+(current-timer));
        }
    }
}

void Lacrimi::FinalCleanup()
{
    sLog.Notice("Lacrimi", "Running Final Cleanup!");
    uint32 current = getMSTime();
    for(std::set<CleanupComponent*>::iterator itr = CleanupComponents.begin(); itr != CleanupComponents.end(); itr++)
        (*itr)->CleanupFunction(current);
    for(std::set<CleanupComponent*>::iterator itr = CleanupComponents.begin(); itr != CleanupComponents.end(); itr++)
        (*itr)->Destruct();
    sLog.Notice("Lacrimi", "Final Cleanup Finished!");
}

bool Lacrimi::_StartDB()
{
    if(!config)
        return false;

    string error;
    // Configure World Database...
    string hostname = lacrimiIni->ReadString("LacrimiDatabase", "Hostname", "ERROR");
    string username = lacrimiIni->ReadString("LacrimiDatabase", "Username", "ERROR");
    string password = lacrimiIni->ReadString("LacrimiDatabase", "Password", "ERROR");
    string database = lacrimiIni->ReadString("LacrimiDatabase", "Name", "ERROR");
    int port = lacrimiIni->ReadInteger("LacrimiDatabase", "Port", 0);
    int type = lacrimiIni->ReadInteger("LacrimiDatabase", "Type", 0);
    if(strcmp(hostname.c_str(), "ERROR") == 0)
        error.append("Hostname");
    else if(strcmp(username.c_str(), "ERROR") == 0)
        error.append("Username");
    else if(strcmp(password.c_str(), "ERROR") == 0)
        error.append("Password");
    else if(strcmp(database.c_str(), "ERROR") == 0)
        error.append("DatabaseName");
    else if(port == 0 || type == 0)
        error.append("Port/Type");
    if(error.length() || username == "user" || password == "pass" || hostname == "host")
    {
        sLog.outError("sql: Lacrimi database parameter not found for %s.", error.c_str());
        return false;
    }

    // Create the DB
    LacrimiDB = DirectDatabase::Create();

    // Initialize it
    if( !LacrimiDB->Initialize(hostname.c_str(), uint(port), username.c_str(),
        password.c_str(), database.c_str(), lacrimiIni->ReadInteger("LacrimiDatabase", "ConnectionCount", 2), 15000 ) )
    {
        sLog.outDebug( "sql: Main database initialization failed. Exiting." );
        _StopDB(); // Kekeke
        return false;
    }

    return true;
}

void Lacrimi::_StopDB()
{
    LacrimiDB->EndThreads();
    LacrimiDB->Shutdown();
    delete LacrimiDB;
    LacrimiDB = NULL;
}

std::string Lacrimi::GetConfigString(char* configfamily, char* configoption, char* cdefault)
{
    std::string creturn = cdefault;
    if(config) // Crow: Lets just use MAX_PATH for this, it's a reasonable number...
        creturn = lacrimiIni->ReadString(configfamily, configoption, cdefault);
    return creturn;
}

float Lacrimi::GetConfigfloat(char* configfamily, char* configoption, float fdefault)
{
    float freturn = fdefault;
    if(config)
        freturn = lacrimiIni->ReadFloat(configfamily, configoption, fdefault);
    return freturn;
}

bool Lacrimi::GetConfigBool(char* configfamily, char* configoption, bool bdefault)
{
    bool breturn = bdefault;
    if(config)
        breturn = lacrimiIni->ReadBoolean(configfamily, configoption, bdefault);
    return breturn;
}

int Lacrimi::GetConfigInt(char* configfamily, char* configoption, int intdefault)
{
    int ireturn = intdefault;
    if(config)
        ireturn = lacrimiIni->ReadInteger(configfamily, configoption, intdefault);
    return ireturn;
}

// Use sMgr for Script Mgr.
void Lacrimi::SetupScripts()
{
    sLog.Success("Lacrimi", "Lacrimi Engine Started");
    sLog.Success("","############################################################");
    sLog.Success("","# ##            #       ####### ####### ##    #    #    ## #");
    sLog.Success("","# ##           ###      ##      ##   ## ##   ###  ###   ## #");
    sLog.Success("","# ##          ## ##     ##      ##   ## ##   ###  ###   ## #");
    sLog.Success("","# ##         #######    ##      ####### ##  ## #### ##  ## #");
    sLog.Success("","# ##        ##     ##   ##      #####   ##  ## #### ##  ## #");
    sLog.Success("","# ##       ##       ##  ##      ##  ##  ## ##   ##   ## ## #");
    sLog.Success("","# ####### ##         ## ####### ##   ## ## ##   ##   ## ## #");
    sLog.Success("","# :::::::.::.........::.:::::::.::...::.::.::...::...::.:: #");
    sLog.Success("","############################################################");

    // Load our configs
    lacrimiIni = new CIniFile("./lacrimi.ini");
    if(lacrimiIni->ParseError())
        config = false;

    // Load our DBs
    if(_StartDB())
        database = true;

    dumpstats = GetConfigBool("StatDumper", "DumpStats", false);
    if(dumpstats)
    {
        sLog.Success("Lacrimi", "Stat Dumper Initialized");
        strcpy(Filename, GetConfigString("StatDumper", "Filename", "stats.xml").c_str());
    }

    sLog.Notice("Lacrimi", "C++ Loading scripts...");
    SetupCityScripts();
    SetupSpellScripts();
    SetupCustomScripts();

    SetupZoneScripts();
    SetupInstanceScripts();
}

void Lacrimi::SetupZoneScripts()
{
    if(GetConfigBool("ZoneScripts", "EnableOutlandScripts", true))
        SetupOutlandScripts();
    if(GetConfigBool("ZoneScripts", "EnableKalimdorScripts", true))
        SetupKalimdorScripts();
    if(GetConfigBool("ZoneScripts", "EnableEbonHoldScripts", true))
        SetupEbonHoldScripts();
    if(GetConfigBool("ZoneScripts", "EnableNorthrendScripts", true))
        SetupNorthrendScripts();
    if(GetConfigBool("ZoneScripts", "EnableEasternKingdomScripts", true))
        SetupEasternKingdomScripts();
}

void Lacrimi::SetupEasternKingdomScripts()
{
    SetupAlteracMountains();
    SetupArathiHighlands();
    SetupBlastedLands();
    SetupBurningSteppes();
    SetupDunMorogh();
    SetupDuskwood();
    SetupEasternPlaguelands();
    SetupElwynnForest();
    SetupEversongWoods();
    SetupGhostlands();
    SetupHinterlands();
    SetupIsleOfQuelDanas();
    SetupLochModan();
    SetupSearingGorge();
    SetupSilverpineForest();
    SetupStranglethornVale();
    SetupTrisfalGlades();
    SetupWesternPlaguelands();
    SetupWestfall();
    SetupWetlands();
}

void Lacrimi::SetupKalimdorScripts()
{
    SetupAshenvale();
    SetupAzshara();
    SetupAzuremystIsle();
    SetupBloodmystIsle();
    SetupDarkshore();
    SetupDesolace();
    SetupDurotar();
    SetupDustwallowMarsh();
    SetupFelwood();
    SetupFeralas();
    SetupMoonglade();
    SetupMulgore();
    SetupSilithus();
    SetupStonetalonMountains();
    SetupTanaris();
    SetupTeldrassil();
    SetupTheBarrens();
    SetupThousandNeedles();
    SetupUngoroCrater();
    SetupWinterspring();
}

void Lacrimi::SetupOutlandScripts()
{
    SetupBladesEdgeMountains();
    SetupHellfirePeninsula();
    SetupNagrand();
    SetupNetherstorm();
    SetupShadowmoonValley();
    SetupTerrokarForest();
    SetupZangarmarsh();
}

void Lacrimi::SetupNorthrendScripts()
{
    SetupWintergrasp();
    SetupBoreanTundra();
    SetupDragonblight();
    SetupGrizzlyHills();
    SetupHowlingFjord();
    SetupIcecrown();
    SetupSholazarBasin();
    SetupStormPeaks();
    SetupZulDrak();
}

void Lacrimi::SetupCityScripts()
{
    SetupIronforge();
    SetupSilvermoon();
    SetupStormwind();
    SetupUndercity();
    SetupDarnassus();
    SetupExodar();
    SetupOrgrimmar();
    SetupThunderbluff();
    SetupShattrath();
    SetupDalaran();
}

void Lacrimi::SetupSpellScripts()
{
    SetupDeathKnightSpells();
    SetupDruidSpells();
    SetupHunterSpells();
    SetupMageSpells();
    SetupPaladinSpells();
    SetupPriestSpells();
    SetupRogueSpells();
    SetupShamanSpells();
    SetupWarlockSpells();
    SetupWarriorSpells();
    SetupMiscSpells();
}

void Lacrimi::SetupInstanceScripts()
{
    // Level 2: Classic Instance Scripts
    SetupBlackfathomDeeps();
}

void Lacrimi::SetupCustomScripts()
{

}

void Lacrimi::GenerateUptimeString(char * Dest)
{
    if(first)
    {
        sprintf(Dest, "0 days, 0 hours, 0 minutes, 0 seconds");
        first = false;
        return;
    }

    uint32 seconds = sWorld.GetUptime();
    uint32 mins = 0;
    uint32 hours = 0;
    uint32 days = 0;
    if(seconds >= 60)
    {
        mins = seconds / 60;
        if(mins)
        {
            seconds -= mins*60;
            if(mins >= 60)
            {
                hours = mins / 60;
                if(hours)
                {
                    mins -= hours*60;
                    if(hours >= 24)
                    {
                        days = hours/24;
                        if(days)
                            hours -= days*24;
                    }
                }
            }
        }
    }

    sprintf(Dest, "%d days, %d hours, %d minutes, %d seconds", (int)days, (int)hours, (int)mins, (int)seconds);
}

void Lacrimi::FillOnlineTime(uint32 Time, char * Dest)
{
    uint32 seconds = Time;
    uint32 mins=0;
    uint32 hours=0;
    uint32 days=0;
    if(seconds >= 60)
    {
        mins = seconds / 60;
        if(mins)
        {
            seconds -= mins*60;
            if(mins >= 60)
            {
                hours = mins / 60;
                if(hours)
                {
                    mins -= hours*60;
                    if(hours >= 24)
                    {
                        days = hours/24;
                        if(days)
                            hours -= days*24;
                    }
                }
            }
        }
    }
    sprintf(Dest, "%d hours, %d minutes, %d seconds", (int)hours, (int)mins, (int)seconds);
}

static const char * race_names[RACE_DRAENEI+1] = {
    NULL,
    "human",
    "orc",
    "dwarf",
    "nightelf",
    "undead",
    "tauren",
    "gnome",
    "troll",
    NULL,
    "bloodelf",
    "draenei",
};

static const char * class_names[DRUID+1] = {
    NULL,
    "warrior",
    "paladin",
    "hunter",
    "rogue",
    "priest",
    "deathknight",
    "shaman",
    "mage",
    "warlock",
    NULL,
    "druid",
};

void Lacrimi::DumpStats()
{
    if( Filename[0] == '\0' )
        return;
    FILE* f = fopen( Filename, "w" );
    if( !f )
        return;

    sLog.Debug("Lacrimi", "Dumping stats...");

    // Dump Header
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<?xml-stylesheet type=\"text/xsl\" href=\"server_stats.xsl\"?>\n");
    fprintf(f, "<serverpage>\n");
    fprintf(f, "  <status>\n");

    uint32* races = new uint32[RACE_DRAENEI+1];
    uint32* classes = new uint32[DRUID+1];
    memset(races, 0, sizeof(uint32)*(RACE_DRAENEI+1));
    memset(classes, 0, sizeof(uint32)*(DRUID+1));
    std::deque<Player*> gms;
    {
        // Dump server information.
        fprintf(f, "    <platform>Sandshroud Hearthstone(%s::%s) r%u %s-%s</platform>\n", BUILD_TAG, BUILD_HASH_STR, BUILD_REVISION, CONFIG, ARCH);

        char *uptime = new char[80];
        GenerateUptimeString(uptime);
        time_t t = (time_t)UNIXTIME;
        float AvgLat;
        uint32 GMCount;
        int gm = 0;
        int count = 0;
        int avg = 0;

        // lock players reader
        objmgr._playerslock.AcquireReadLock();

        HM_NAMESPACE::hash_map<uint32, Player*>::const_iterator itr;
        for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
        {
            if(itr->second->GetSession() && itr->second->IsInWorld())
            {
                count++;
                avg += itr->second->GetSession()->GetLatency();
                if(itr->second->GetSession()->GetPermissionCount())
                {
                    gm++;
                    gms.push_back(itr->second);
                }
                classes[itr->second->getClass()]++;
                races[itr->second->getRace()]++;
            }           
        }
        objmgr._playerslock.ReleaseReadLock();

        AvgLat = count ? (float)((float)avg / (float)count) : 0;
        GMCount = gm;

        fprintf(f, "    <servername>%s</servername>\n", mainIni->ReadString("RealmData", "RealmName", "Test Realm").c_str());
        fprintf(f, "    <uptime>%s</uptime>\n", uptime);
        fprintf(f, "    <oplayers>%u</oplayers>\n", (unsigned int)sWorld.GetSessionCount());
        fprintf(f, "    <cpu>%2.2f</cpu>\n", sWorld.GetCPUUsage(true));
        fprintf(f, "    <qplayers>%u</qplayers>\n", (unsigned int)sWorld.GetQueueCount());
        fprintf(f, "    <ram>%.3f</ram>\n", sWorld.GetRAMUsage(true));
        fprintf(f, "    <avglat>%.3f</avglat>\n", AvgLat);
        fprintf(f, "    <threads>%u</threads>\n", (unsigned int)ThreadPool.GetActiveThreadCount());
        fprintf(f, "    <gmcount>%u</gmcount>\n", (unsigned int)GMCount);
        fprintf(f, "    <lastupdate>%s</lastupdate>\n", asctime(localtime(&t)));
        fprintf(f, "    <alliance>%u</alliance>\n", (unsigned int)sWorld.AlliancePlayers);
        fprintf(f, "    <horde>%u</horde>\n", (unsigned int)sWorld.HordePlayers);
        fprintf(f, "    <acceptedconns>%u</acceptedconns>\n", (unsigned int)sWorld.mAcceptedConnections);
        fprintf(f, "    <peakcount>%u</peakcount>\n", (unsigned int)sWorld.PeakSessionCount);
        fprintf(f, "    <wdbquerysize>%u</wdbquerysize>\n", WorldDatabase.GetQueueSize());
        fprintf(f, "    <cdbquerysize>%u</cdbquerysize>\n", CharacterDatabase.GetQueueSize());
        delete [] uptime;
    }

    fprintf(f, "  </status>\n");
    
    fprintf(f, "  <statsummary>\n");
    uint32 i;
    for(i = 0; i <= RACE_DRAENEI; ++i)
    {
        if( race_names[i] != NULL )
            fprintf(f, "    <%s>%u</%s>\n", race_names[i], races[i], race_names[i]);
    }

    for(i = 0; i <= DRUID; ++i)
    {
        if( class_names[i] != NULL )
            fprintf(f, "    <%s>%u</%s>\n", class_names[i], classes[i], class_names[i]);
    }
    fprintf(f, "  </statsummary>\n");

    Player*  plr;
    uint32 t = (uint32)time(NULL);

    char otime[100];
    {
        fprintf(f, "  <instances>\n");
        
        // need a big buffer..
        char *buf = new char[500000];
        memset(buf, 0, 500000);

        // Dump Instance Information
        //sWorldCreator.BuildXMLStats(buf);
        sInstanceMgr.BuildXMLStats(buf);
        fprintf(f, buf);
        fprintf(f, "  </instances>\n");
        delete [] buf;
    }

    {
        // GM Information
        fprintf(f, "  <gms>\n");
        while(!gms.empty())
        {
            plr = gms.front();
            gms.pop_front();

            if(plr->bGMTagOn)
            {
                FillOnlineTime(t - plr->OnlineTime, otime);
                fprintf(f, "    <gmplr>\n");
                fprintf(f, "      <name>%s%s</name>\n", (plr->bGMTagOn ? "{GM}" : ""), plr->GetName());
                fprintf(f, "      <race>%u</race>\n", plr->getRace());
                fprintf(f, "      <class>%u</class>\n", (unsigned int)plr->getClass());
                fprintf(f, "      <gender>%u</gender>\n", (unsigned int)plr->getGender());
                fprintf(f, "      <pvprank>%u</pvprank>\n", (unsigned int)plr->GetPVPRank());
                fprintf(f, "      <level>%u</level>\n", (unsigned int)plr->GetUInt32Value(UNIT_FIELD_LEVEL));
                fprintf(f, "      <map>%u</map>\n", (unsigned int)plr->GetMapId());
                fprintf(f, "      <areaid>%u</areaid>\n", (unsigned int)plr->GetAreaId());
                fprintf(f, "      <ontime>%s</ontime>\n", otime);
                fprintf(f, "      <latency>%u</latency>\n", (unsigned int)plr->GetSession()->GetLatency());
                fprintf(f, "      <permissions>%s</permissions>\n", plr->GetSession()->GetPermissions());
                fprintf(f, "    </gmplr>\n");
            }
        }

        fprintf(f, "  </gms>\n");
    }

    {
        fprintf(f, "  <sessions>\n");

        // Dump Player Information
        objmgr._playerslock.AcquireReadLock();
        HM_NAMESPACE::hash_map<uint32, Player*>::const_iterator itr;

        for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
        {
            plr = itr->second;
            if(itr->second->GetSession() && itr->second->IsInWorld())
            {
                FillOnlineTime(t - plr->OnlineTime, otime);

                fprintf(f, "    <plr>\n");
                fprintf(f, "      <name>%s</name>\n", plr->GetName());
                fprintf(f, "      <race>%u</race>\n", (unsigned int)plr->getRace());
                fprintf(f, "      <class>%u</class>\n", (unsigned int)plr->getClass());
                fprintf(f, "      <gender>%u</gender>\n", (unsigned int)plr->getGender());
                fprintf(f, "      <pvprank>%u</pvprank>\n", (unsigned int)plr->GetPVPRank());
                fprintf(f, "      <level>%u</level>\n", (unsigned int)plr->GetUInt32Value(UNIT_FIELD_LEVEL));
                fprintf(f, "      <map>%u</map>\n", (unsigned int)plr->GetMapId());
                fprintf(f, "      <areaid>%u</areaid>\n", (unsigned int)plr->GetAreaId());
                //requested by Zdarkside for he's online map. I hope it does not screw up any parser. If so, then make a better one :P
                fprintf(f, "      <xpos>%f</xpos>\n", plr->GetPositionX ());
                fprintf(f, "      <ypos>%f</ypos>\n", plr->GetPositionY());
                fprintf(f, "      <ontime>%s</ontime>\n", otime);
                fprintf(f, "      <latency>%u</latency>\n", (unsigned int)plr->GetSession()->GetLatency());
                fprintf(f, "    </plr>\n");
                if(plr->GetSession()->GetPermissionCount() > 0)
                    gms.push_back(plr);
            }
        }

        objmgr._playerslock.ReleaseReadLock();
        fprintf(f, "  </sessions>\n");

        
    }

    delete [] races;
    delete [] classes;
    fprintf(f, "</serverpage>\n");
    fclose(f);
}

bool isTargetDummy(uint32 id)
{
    switch(id)
    {
    case 1921:
    case 2673:
    case 2674:
    case 4952:
    case 5202:
    case 5652:
    case 5723:
    case 11875:
    case 12385:
    case 12426:
    case 16211:
    case 16897:
    case 17059:
    case 17060:
    case 17578:
    case 18215:
    case 18504:
    case 19139:
    case 21157:
    case 24792:
    case 25225:
    case 25297:
    case 30527:
    case 31143:
    case 31144:
    case 31146:
    case 32541:
    case 32542:
    case 32543:
    case 32545:
    case 32546:
    case 32547:
    case 32666:
    case 32667:
    case 33272:
    case 33243:
    case 33229:
        {
            return true;
        }break;
    }
    return false;
}
