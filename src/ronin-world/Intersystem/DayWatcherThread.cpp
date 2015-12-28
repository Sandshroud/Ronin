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

}

DayWatcherThread::~DayWatcherThread()
{

}

void DayWatcherThread::load_settings()
{
    QueryResult *result = NULL;
    if(result = CharacterDatabase.Query("SELECT setting_value FROM server_settings WHERE setting_id = \"last_dailies_reset_time\""))
    {
        last_daily_reset_time = result->Fetch()[0].GetUInt64();
        local_last_daily_reset_time = *localtime(&last_daily_reset_time);
        delete result;
    }
    else
    {
        tm *now_time = localtime(&UNIXTIME);
        now_time->tm_hour = 0;
        last_daily_reset_time = mktime(now_time);
        local_last_daily_reset_time = *now_time;
    }
}

bool DayWatcherThread::has_timeout_expired(tm *now_time, tm *last_time, uint32 timeoutval)
{
    switch(timeoutval)
    {
    case MONTHLY: return (now_time->tm_mon != last_time->tm_mon);
    case WEEKLY: return ( (now_time->tm_mday / 7) != (last_time->tm_mday / 7) || (now_time->tm_mon != last_time->tm_mon) );
    case DAILY: return ((now_time->tm_mday != last_time->tm_mday) || (now_time->tm_mon != last_time->tm_mon));
    case HOURLY: return ((now_time->tm_hour != last_time->tm_hour) || (now_time->tm_mday != last_time->tm_mday) || (now_time->tm_mon != last_time->tm_mon));
    case MINUTELY: return ((now_time->tm_min != last_time->tm_min) || (now_time->tm_hour != last_time->tm_hour) || (now_time->tm_mday != last_time->tm_mday) || (now_time->tm_mon != last_time->tm_mon));
    }
    return false;
}

bool DayWatcherThread::run()
{
    sLog.Notice("DayWatcherThread", "Started.");
    load_settings();
    bool bheroic_reset = false;

    uint32 interv = 120000;//Daywatcher check interval (in ms), must be >> 30secs !
    while(GetThreadState() != THREADSTATE_TERMINATE)
    {
        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        if(has_timeout_expired(&g_localTime, &local_last_daily_reset_time, DAILY))
            update_daily();

        // reset will occur daily between 07:59:00 CET and 08:01:30 CET (players inside will get 60 sec countdown)
        // 8AM = 25200s
        uint32 umod = uint32(currenttime + 3600) % 86400;
        if(bheroic_reset == false && umod >= 25140 && umod <= 25140 + (interv/1000) + 30 )
        {
            //It's approx 8AM, let's reset (if not done so already)
            Reset_Heroic_Instances();
            bheroic_reset = true;
        }

        if(bheroic_reset && umod > 25140 + (interv/1000) + 30 )
            bheroic_reset = false;

        if(!SetThreadState(THREADSTATE_SLEEPING))
            break;

        Delay(interv);
    }

    return true;
}

void DayWatcherThread::update_daily()
{
    sLog.Notice("DayWatcherThread", "Running Daily Quest Reset...");
    objmgr.ResetDailies();
    last_daily_reset_time = UNIXTIME;
    local_last_daily_reset_time = g_localTime;
    CharacterDatabase.Execute("REPLACE INTO server_settings VALUES(\"last_dailies_reset_time\", %u)", last_daily_reset_time);
}

void DayWatcherThread::Reset_Heroic_Instances()
{
    sLog.Notice("DayWatcherThread", "Reseting heroic instances...");
    sWorldMgr.ResetHeroicInstances();
}
