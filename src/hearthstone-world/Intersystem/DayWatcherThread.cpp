/***
 * Demonstrike Core
 */

/* Arena and Honor Point Calculation System
 *    Copyright (c) 2007 Burlex
 */

#include "StdAfx.h"

initialiseSingleton( DayWatcherThread );

#ifdef WIN32
static HANDLE m_abortEvent = INVALID_HANDLE_VALUE;
#else
static pthread_cond_t abortcond;
static pthread_mutex_t abortmutex;
#endif

DayWatcherThread::DayWatcherThread() : ThreadContext()
{
    m_dirty = false;
}

DayWatcherThread::~DayWatcherThread()
{

}

void DayWatcherThread::dupe_tm_pointer(tm * returnvalue, tm * mypointer)
{
    memcpy(mypointer, returnvalue, sizeof(tm));
}

void DayWatcherThread::update_settings()
{
    CharacterDatabase.Execute("REPLACE INTO server_settings VALUES(\"last_arena_update_time\", %u)", last_arena_time);
    CharacterDatabase.Execute("REPLACE INTO server_settings VALUES(\"last_dailies_reset_time\", %u)", last_daily_reset_time);
}

void DayWatcherThread::load_settings()
{
    QueryResult * result = CharacterDatabase.Query("SELECT setting_value FROM server_settings WHERE setting_id = \"last_arena_update_time\"");
    if(result)
    {
        last_arena_time = result->Fetch()[0].GetUInt32();
        delete result;
    }
    else
    {
        sLog.Debug("DayWatcherThread", "Initialized Arena Updates.");
        last_arena_time = 0;
    }

    result = CharacterDatabase.Query("SELECT setting_value FROM server_settings WHERE setting_id = \"last_dailies_reset_time\"");
    if(result)
    {
        last_daily_reset_time = result->Fetch()[0].GetUInt32();
        delete result;
    }
    else
    {
        sLog.Debug("DayWatcherThread", "Initialized Daily Updates.");
        last_daily_reset_time = 0;
    }
}

void DayWatcherThread::set_tm_pointers()
{
    dupe_tm_pointer(localtime(&last_arena_time), &local_last_arena_time);
    dupe_tm_pointer(localtime(&last_daily_reset_time), &local_last_daily_reset_time);
}

bool DayWatcherThread::has_timeout_expired(tm * now_time, tm * last_time, uint32 timeoutval)
{
    switch(timeoutval)
    {
    case MONTHLY:
        return (now_time->tm_mon != last_time->tm_mon);

    case WEEKLY:
        return ( (now_time->tm_mday / 7) != (last_time->tm_mday / 7) || (now_time->tm_mon != last_time->tm_mon) );

    case DAILY:
        return ((now_time->tm_mday != last_time->tm_mday) || (now_time->tm_mon != last_time->tm_mon));

    case HOURLY:
        return ((now_time->tm_hour != last_time->tm_hour) || (now_time->tm_mday != last_time->tm_mday) || (now_time->tm_mon != last_time->tm_mon));

    case MINUTELY:
        return ((now_time->tm_min != last_time->tm_min) || (now_time->tm_hour != last_time->tm_hour) || (now_time->tm_mday != last_time->tm_mday) || (now_time->tm_mon != last_time->tm_mon));
    }
    return false;
}

bool DayWatcherThread::run()
{
    sLog.Notice("DayWatcherThread", "Started.");
    _loaded = false;
    currenttime = UNIXTIME;
    dupe_tm_pointer(localtime(&currenttime), &local_currenttime);
    load_settings();
    set_tm_pointers();
    m_busy = false;
    _firstrun[0] = true;
    _firstrun[1] = true;
    m_heroic_reset = false;

    uint32 interv = 120000;//Daywatcher check interval (in ms), must be >> 30secs !

    while(GetThreadState() != THREADSTATE_TERMINATE)
    {
        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        currenttime = UNIXTIME;
        dupe_tm_pointer(localtime(&currenttime), &local_currenttime);

        if(has_timeout_expired(&local_currenttime, &local_last_arena_time, WEEKLY))
            update_arena();

        if(has_timeout_expired(&local_currenttime, &local_last_daily_reset_time, DAILY))
            update_daily();

        // reset will occur daily between 07:59:00 CET and 08:01:30 CET (players inside will get 60 sec countdown)
        // 8AM = 25200s
        uint32 umod = uint32(currenttime + 3600) % 86400;
        if(!m_heroic_reset && umod >= 25140 && umod <= 25140 + (interv/1000) + 30 )
        {
            //It's approx 8AM, let's reset (if not done so already)
            Reset_Heroic_Instances();
            m_heroic_reset = true;
        }
        if(m_heroic_reset && umod > 25140 + (interv/1000) + 30 )
            m_heroic_reset = false;

        if(m_dirty)
            update_settings();

        CheckSpecialTimes(&local_currenttime);

        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;

        Delay(interv);
    }

    return true;
}

void DayWatcherThread::update_arena()
{
    sLog.Notice("DayWatcherThread", "Running Weekly Arena Point Maintenance...");
    QueryResult * result = CharacterDatabase.Query("SELECT guid, arenaPoints FROM characters");     /* this one is a little more intensive. */
    Player* plr;
    uint32 guid, arenapoints, orig_arenapoints;
    ArenaTeam * team;
    PlayerInfo * inf;
    uint32 arenapointsPerTeam[3] = {0};
    double X, Y;
    if(result)
    {
        do
        {
            Field * f = result->Fetch();
            guid = f[0].GetUInt32();

            inf = objmgr.GetPlayerInfo(guid);
            if( inf == NULL )
                continue;

            arenapoints = f[1].GetUInt32();
            orig_arenapoints = arenapoints;

            for(uint32 i = 0; i < 3; i++)
                arenapointsPerTeam[i] = 0;

            /* are we in any arena teams? */
            for(uint32 i = 0; i < 3; i++)           // 3 arena team types
            {
                team = inf->arenaTeam[i];
                if(team)
                {
                    ArenaTeamMember *member = team->GetMemberByGuid(guid);
                    if(member == NULL || team->m_stat_gamesplayedweek < 10 || ((member->Played_ThisWeek * 100) / team->m_stat_gamesplayedweek < 30))
                        continue;

                    /* we're in an arena team of this type! */
                    /* Source: http://www.wowwiki.com/Arena_point */
                    X = (double)team->m_stat_rating;
                    if(X <= 510.0)  // "if X<=510"
                        continue;       // no change
                    else if(X > 510.0 && X <= 1500.0)       // "if 510 < X <= 1500"
                    {
                        Y = (0.22 * X) + 14.0;
                    }
                    else            // "if X > 1500"
                    {
                        // http://eu.wowarmory.com/arena-calculator.xml
                        //              1511.26
                        //   ---------------------------
                        //                   -0.00412*X
                        //    1+1639.28*2.71828

                        double power = ((-0.00412) * X);
                        //if(power < 1.0)
                        //  power = 1.0;

                        double divisor = pow(((double)(2.71828)), power);
                        divisor *= 1639.28;
                        divisor += 1.0;
                        //if(divisor < 1.0)
                        //  divisor = 1.0;

                        Y = 1511.26 / divisor;
                    }

                    // 2v2 teams only earn 70% (Was 60% until 13th March 07) of the arena points, 3v3 teams get 80%, while 5v5 teams get 100% of the arena points.
                    // 2v2 - 76%, 3v3 - 88% as of patch 2.2
                    if(team->m_type == ARENA_TEAM_TYPE_2V2)
                        Y *= 0.76;
                    else if(team->m_type == ARENA_TEAM_TYPE_3V3)
                        Y *= 0.88;

                    if(Y > 1.0)
                        arenapointsPerTeam[i] += long2int32(double(ceil(Y)));
                }
            }

            arenapointsPerTeam[0] = (uint32)max(arenapointsPerTeam[0],arenapointsPerTeam[1]);
            arenapoints += (uint32)max(arenapointsPerTeam[0],arenapointsPerTeam[2]);

            if(orig_arenapoints != arenapoints)
            {
                plr = objmgr.GetPlayer(guid);
                if(plr != NULL)
                {
                    plr->m_arenaPoints = arenapoints;

                    /* update visible fields (must be done through an event because of no uint lock */
                    sEventMgr.AddEvent(plr, &Player::RecalculateHonor, EVENT_PLAYER_UPDATE, 100, 1, 0);

                    /* send a little message :> */
                    sChatHandler.SystemMessage(plr->GetSession(), "Your arena points have been updated! Check your PvP tab!");
                }

                /* update in sql */
                CharacterDatabase.Execute("UPDATE characters SET arenaPoints = %u WHERE guid = %u", arenapoints, guid);
            }
        }while(result->NextRow());
        delete result;
    }

    objmgr.UpdateArenaTeamWeekly();

    //===========================================================================
    last_arena_time = UNIXTIME;
    dupe_tm_pointer(localtime(&last_arena_time), &local_last_arena_time);
    m_dirty = true;
}

void DayWatcherThread::update_daily()
{
    sLog.Notice("DayWatcherThread", "Running Daily Quest Reset...");
    CharacterDatabase.WaitExecute("UPDATE characters SET finished_daily_quests = ''");
    objmgr.ResetDailies();
    last_daily_reset_time = UNIXTIME;
    dupe_tm_pointer(localtime(&last_daily_reset_time), &local_last_daily_reset_time);
    m_dirty = true;
}

void DayWatcherThread::Reset_Heroic_Instances()
{
    sLog.Notice("DayWatcherThread", "Reseting heroic instances...");
    sInstanceMgr.ResetHeroicInstances();
}

void DayWatcherThread::CheckSpecialTimes(tm*time)
{
    switch(time->tm_mon)
    {
    case 0:
        {
            if(sWorld.WintersVeil)
                if(time->tm_mday >= 2)
                    sWorld.SpawnWintersVeil(false);
        }break;
    case 9:
        {
            if(!sWorld.HallowsEnd)
                if(time->tm_mday >= 18)
                    sWorld.SpawnHallowsEnd(true);
        }break;
    case 10:
        {
            if(sWorld.HallowsEnd)
                if(time->tm_mday <= 3)
                    sWorld.SpawnHallowsEnd(false);
        }break;
    case 11:
        {
            if(!sWorld.WintersVeil)
                sWorld.SpawnWintersVeil(true);
        }break;
    }

    switch(time->tm_year)
    {
        // Crow: TODO: Date requirements... Oh well.
    case 108: // 4th Anniversary
        {
            sWorld.SetAnniversary(4);
        }break;
    case 109: // 5th Anniversary
        {
            sWorld.SetAnniversary(5);
        }break;
    case 110: // 6th Anniversary
        {
            sWorld.SetAnniversary(6);
        }break;
    }
}
