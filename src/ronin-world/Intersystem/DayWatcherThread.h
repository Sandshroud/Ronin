/***
 * Demonstrike Core
 */

#pragma once

enum DAYWATCHERSETTINGS
{
    WEEKLY      = 1,
    DAILY       = 2,
    MONTHLY     = 3,
    HOURLY      = 4,
    MINUTELY    = 5,
};

class SERVER_DECL DayWatcherThread : public Singleton<DayWatcherThread>, public ThreadContext
{
public:
    DayWatcherThread();
    ~DayWatcherThread();

    bool run();
    time_t GetLastDailyResetTime() { return last_daily_reset_time; }

protected:
    void load_settings();
    void update_settings();
    void set_tm_pointers();

    void update_daily();

    void Reset_Heroic_Instances();
    void dupe_tm_pointer(tm value, time_t currentTime);
    bool has_timeout_expired(tm *now_time, tm *last_time, uint32 timeoutval);

private:
    bool m_dirty, m_heroic_reset;

    static const time_t timeout = 120;      /* check every 2 minutes */
    time_t currenttime, last_daily_reset_time;
    tm local_currenttime, local_last_daily_reset_time;
};

#define sDayWatcher DayWatcherThread::getSingleton()
