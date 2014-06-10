/***
 * Demonstrike Core
 */

#ifndef __ACCOUNTCACHE_H
#define __ACCOUNTCACHE_H

enum RealmColours
{
    REALM_POP_MEDIUM        = 0,
    REALM_POP_NEW           = 200,
    REALM_POP_FULL          = 400,
    REALM_POP_NEW_PLAYERS   = 600,

    REALM_FLAG_NONE         = 0x00, // Green?
    REALM_FLAG_INVALID      = 0x01, // Red!
    REALM_FLAG_OFFLINE      = 0x02, // Grey!
    REALM_FLAG_SPECIFYBUILD = 0x04, // Red!
    REALM_FLAG_RECOMMENDED  = 0x20, // Green!
    REALM_FLAG_NEW          = 0x40,
    REALM_FLAG_FULL         = 0x80
};

struct Account
{
    uint32 AccountId;
    char * GMFlags;
    uint8 AccountFlags;
    uint32 Banned;
    uint8 SrpHash[20];
    uint8 * SessionKey;
    std::string * UsernamePtr;
    uint32 Muted;

    Account()
    {
        GMFlags = NULL;
        SessionKey = NULL;
    }

    ~Account()
    {
        if(GMFlags)
            delete [] GMFlags;
        if(SessionKey)
            delete [] SessionKey;
    }

    void SetGMFlags(const char * flags)
    {
        if(GMFlags)
            delete [] GMFlags;

        size_t len = strlen(flags);
        if(len == 0 || (len == 1 && flags[0] == '0'))
        {
            // no flags
            GMFlags = NULL;
            return;
        }
        
        GMFlags = new char[len+1];
        memcpy(GMFlags, flags, len);
        GMFlags[len] = 0;
    }

    void SetSessionKey(const uint8 *key)
    {
        if(SessionKey == NULL)
            SessionKey = new uint8[40];
        memcpy(SessionKey, key, 40);
    }

    char Locale[4];
    bool forcedLocale;

};

typedef struct IPBan
{
    unsigned int Mask;
    unsigned char Bytes;
    uint32 Expire;
    std::string db_ip;
} IPBan;

enum BAN_STATUS
{
    BAN_STATUS_NOT_BANNED = 0,
    BAN_STATUS_TIME_LEFT_ON_BAN = 1,
    BAN_STATUS_PERMANENT_BAN = 2,
};

class IPBanner : public Singleton< IPBanner >
{
public:
    void Reload();

    bool Add(const char * ip, uint32 dur);
    bool Remove(const char * ip);

    BAN_STATUS CalculateBanStatus(in_addr ip_address);

protected:
    Mutex listBusy;
    std::list<IPBan> banList;
};

class AccountMgr : public Singleton < AccountMgr >
{
public:
    ~AccountMgr()
    {

#ifdef WIN32
        for(HM_NAMESPACE::hash_map<std::string, Account*>::iterator itr = AccountDatabase.begin(); itr != AccountDatabase.end(); ++itr)
#else
        for(std::map<std::string,Account*>::iterator itr = AccountDatabase.begin(); itr != AccountDatabase.end(); ++itr)
#endif
        {
            delete itr->second;
        }
    }

    void AddAccount(Field* field);

    Account* GetAccount(std::string Name)
    {
        setBusy.Acquire();
        Account * pAccount = NULL;
        // this should already be uppercase!
#ifdef WIN32
        HM_NAMESPACE::hash_map<std::string, Account*>::iterator itr = AccountDatabase.find(Name);
#else
        map<string, Account*>::iterator itr = AccountDatabase.find(Name);
#endif

        if(itr == AccountDatabase.end())    pAccount = NULL;
        else                                pAccount = itr->second;

        setBusy.Release();
        return pAccount;
    }

    void UpdateAccount(Account * acct, Field * field);
    void ReloadAccounts(bool silent);
    void ReloadAccountsCallback();

    HEARTHSTONE_INLINE size_t GetCount() { return AccountDatabase.size(); }

private:
    Account* __GetAccount(std::string Name)
    {
        // this should already be uppercase!
#ifdef WIN32
        HM_NAMESPACE::hash_map<std::string, Account*>::iterator itr = AccountDatabase.find(Name);
#else
        map<string, Account*>::iterator itr = AccountDatabase.find(Name);
#endif

        if(itr == AccountDatabase.end())    return NULL;
        else                                return itr->second;
    }

#ifdef WIN32
    HM_NAMESPACE::hash_map<std::string, Account*> AccountDatabase;
#else
    std::map<std::string, Account*> AccountDatabase;
#endif

protected:
    Mutex setBusy;
};
class LogonCommServerSocket;
typedef struct Realm
{
    std::string Name;
    std::string Address;
    uint8 Flag;
    uint8 Icon;
    uint8 WorldRegion;
    uint32 RealPopulation;
    uint32 Population;
    uint32 RealmCap;

    uint8 RequiredCV[3];
    uint16 RequiredBuild;

    Mutex m_charMapLock;
    HM_NAMESPACE::hash_map<uint32, uint8> CharacterMap;
    LogonCommServerSocket *ServerSocket;
}Realm;

class AuthSocket;
class LogonCommServerSocket;

class InformationCore : public Singleton<InformationCore>
{
    std::map<uint32, Realm*>       m_realms;
    std::set<LogonCommServerSocket*> m_serverSockets;
    Mutex serverSocketLock;
    Mutex realmLock;

    uint32 realmhigh;
    bool usepings;

public:
    HEARTHSTONE_INLINE Mutex & getServerSocketLock() { return serverSocketLock; }
    HEARTHSTONE_INLINE Mutex & getRealmLock() { return realmLock; }

    InformationCore();

    // Packets
    void SendRealms(AuthSocket * Socket);
    
    // Realm management
    uint32 GenerateRealmID()
    {
        return ++realmhigh;
    }

    Realm* AddRealm(uint32 realm_id, Realm * rlm);
    Realm* GetRealm(uint32 realm_id);
    int32 GetRealmIdByName(std::string Name);
    std::map<uint32, Realm*> GetRealmMap() { return m_realms; }
    void RemoveRealm(uint32 realm_id);
    bool FindRealmWithAdress(std::string Address);
    void UpdateRealmPop(uint32 realm_id, uint32 population);
    void SetRealmOffline(uint32 realm_id, LogonCommServerSocket *ss);

    HEARTHSTONE_INLINE void AddServerSocket(LogonCommServerSocket * sock) { serverSocketLock.Acquire(); m_serverSockets.insert( sock ); serverSocketLock.Release(); }
    HEARTHSTONE_INLINE void RemoveServerSocket(LogonCommServerSocket * sock) { serverSocketLock.Acquire(); m_serverSockets.erase( sock ); serverSocketLock.Release(); }

    void TimeoutSockets();
    void CheckServers();
};

#define sIPBanner IPBanner::getSingleton()
#define sAccountMgr AccountMgr::getSingleton()
#define sInfoCore InformationCore::getSingleton()

#endif
