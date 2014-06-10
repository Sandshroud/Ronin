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
    bool m_busy;
    bool m_dirty;
    bool _loaded;
    bool _firstrun[2];
    bool m_heroic_reset;

    static const time_t timeout = 120;      /* check every 2 minutes */
    time_t currenttime;
    tm local_currenttime;
    time_t last_arena_time;
    tm local_last_arena_time;
    time_t last_daily_reset_time;
    tm local_last_daily_reset_time;

public:
    DayWatcherThread();
    ~DayWatcherThread();

    bool run();
    void maintenence();
    void dupe_tm_pointer(tm * returnvalue, tm * mypointer);
    void load_settings();
    void update_settings();
    void set_tm_pointers();
    bool has_timeout_expired(tm * now_time, tm * last_time, uint32 timeoutval);
    void update_arena();
    void update_daily();
    void Reset_Heroic_Instances();
    void CheckSpecialTimes(tm*time);
};

#define sDayWatcher DayWatcherThread::getSingleton()
