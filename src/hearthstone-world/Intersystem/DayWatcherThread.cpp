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

void DayWatcherThread::dupe_tm_pointer(tm value, time_t currentTime)
{
    value = *localtime(&currentTime);
}

void DayWatcherThread::update_settings()
{
    if(m_dirty == false)
        return;

    CharacterDatabase.Execute("REPLACE INTO server_settings VALUES(\"last_dailies_reset_time\", %u)", last_daily_reset_time);
}

void DayWatcherThread::load_settings()
{
    QueryResult *result = NULL;
    if(result = CharacterDatabase.Query("SELECT setting_value FROM server_settings WHERE setting_id = \"last_dailies_reset_time\""))
    {
        last_daily_reset_time = result->Fetch()[0].GetUInt64();
        delete result;
    }
    else
    {
        tm *now_time = localtime(&UNIXTIME);
        now_time->tm_hour = 0;
        last_daily_reset_time = mktime(now_time);
    }
    dupe_tm_pointer(local_last_daily_reset_time, last_daily_reset_time);
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
    m_heroic_reset = false;

    uint32 interv = 120000;//Daywatcher check interval (in ms), must be >> 30secs !

    while(GetThreadState() != THREADSTATE_TERMINATE)
    {
        if(!SetThreadState(THREADSTATE_BUSY))
            break;

        dupe_tm_pointer(local_currenttime, (currenttime = UNIXTIME));
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

        update_settings();

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
    dupe_tm_pointer(local_last_daily_reset_time, (last_daily_reset_time = UNIXTIME));
    m_dirty = true;
}

void DayWatcherThread::Reset_Heroic_Instances()
{
    sLog.Notice("DayWatcherThread", "Reseting heroic instances...");
    sInstanceMgr.ResetHeroicInstances();
}
