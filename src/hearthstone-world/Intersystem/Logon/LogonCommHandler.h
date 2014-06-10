/***
 * Demonstrike Core
 */

#pragma once

extern bool bServerShutdown;

typedef struct
{
    uint32 ID;
    string Name;
    string Address;
    uint32 Port;
    uint32 ServerID;
    uint32 RetryTime;
    bool   Registered;
}LogonServer;

typedef struct
{
    string Name;
    string Address;
    uint8 Icon;
    uint8 WorldRegion;
}Realm;

enum RealmType
{
    REALMTYPE_NORMAL    = 0,
    REALMTYPE_PVP       = 3,
    REALMTYPE_RP        = 6,
    REALMTYPE_RPPVP     = 8,
};

class SocketLoadBalancer;

class LogonCommHandler : public Singleton<LogonCommHandler>
{
#ifdef WIN32
    typedef HM_NAMESPACE::hash_map<string, string> ForcedPermissionMap;
#else
    typedef map<string, string> ForcedPermissionMap;
#endif

    ForcedPermissionMap forced_permissions;
    map<uint32, WorldSocket*> pending_logons;
    LogonCommClientSocket* logon;
    LogonServer* server;
    Realm* realm;
    uint32 idhigh;
    uint32 next_request;
    Mutex mapLock;
    Mutex pendingLock;
    bool pings;
    int8 ReConCounter;
    uint32 plrLimit;
    float server_population;

public:
    uint8 sql_passhash[20];

    LogonCommHandler();
    ~LogonCommHandler();

    void UpdateSockets(uint32 diff);

    uint32 GetLatency() { return (logon != NULL ? logon->latency : 0); }
    LogonCommClientSocket * ConnectToLogon(string Address, uint32 Port);
    void UpdateAccountCount(uint32 account_id, int8 add);
    void RequestAddition(LogonCommClientSocket * Socket);
    void ConnectionDropped();
    void AdditionAck(uint32 ServID);
    void CheckAllServers();
    void Startup();
    void Connect();

    //void LogonDatabaseSQLExecute(const char* str, ...);
    //void LogonDatabaseReloadAccounts();

    void Account_SetBanned(const char * account, uint32 banned, const char* reason);
    void Account_SetGM(const char * account, const char * flags);
    void Account_SetMute(const char * account, uint32 muted);
    void IPBan_Add(const char * ip, uint32 duration, const char* reason);
    void IPBan_Remove(const char * ip);
    void SendCreateAccountRequest(const char *accountname, const char *password, const char *email, uint32 accountFlags);

    void LoadRealmConfiguration();
    void AddServer(string Name, string Address, uint32 Port);

    /////////////////////////////
    // Worldsocket stuff
    ///////

    uint32 ClientConnected(string AccountName, WorldSocket * Socket);
    void UnauthedSocketClose(uint32 id);
    void RemoveUnauthedSocket(uint32 id);
    WorldSocket* GetSocketByRequest(uint32 id)
    {
        //pendingLock.Acquire();

        WorldSocket * sock;
        map<uint32, WorldSocket*>::iterator itr = pending_logons.find(id);
        sock = (itr == pending_logons.end()) ? 0 : itr->second;

        //pendingLock.Release();
        return sock;
    }
    HEARTHSTONE_INLINE Mutex & GetPendingLock() { return pendingLock; }
    const string* GetForcedPermissions(string& username);

    void TestConsoleLogon(string& username, string& password, uint32 requestnum);

    ///// From our thread system to allow multithread save pausing
    void Delay(uint32 etimeMS)
    {
        if(etimeMS == 0)
            return;

#if PLATFORM == PLATFORM_WIN
        WaitForSingleObject(hEvent, etimeMS);
#else
        unsigned long times = etimeMS / 1000;
        etimeMS = etimeMS - times * 1000;

        timeval now;
        timespec tv;

        gettimeofday(&now, NULL);

        tv.tv_sec = now.tv_sec;
        tv.tv_nsec = now.tv_usec * 1000;
        tv.tv_sec += times;
        tv.tv_nsec += (etimeMS * 1000 * 1000);

        pthread_mutex_lock(&mutex);
        pthread_cond_timedwait(&cond, &mutex, &tv);
        pthread_mutex_unlock(&mutex);
#endif
    }

private:
#if PLATFORM == PLATFORM_WIN
    HANDLE hEvent;
#else
    pthread_cond_t cond;
    pthread_mutex_t mutex;
#endif
};

#define sLogonCommHandler LogonCommHandler::getSingleton()
