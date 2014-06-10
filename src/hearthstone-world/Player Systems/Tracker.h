/***
 * Demonstrike Core
 */

#pragma once

struct TrackedPlr
{
    uint64 Id;
    uint32 AcctId;
    string Name;
    string IP_Address;
};

//typedef std::set<TrackedGM*> GMTrackerTable;
typedef std::list<TrackedPlr*> TrackerTable;

class SERVER_DECL Tracker :  public Singleton < Tracker >
{
    friend class ChatHandler;

public:
    // Constructor
    Tracker( );

    // Destructor
    ~Tracker( );

    // Variables
    uint64 m_IdCount;
    uint64 m_maxId;
    TrackerTable    Tracked_List;

    // Functions
    uint64 GenerateID();
    uint64 GetTrackerId(string Name);
    TrackedPlr* GetTrackerByID(uint64 Id);
    TrackedPlr* GetTrackerByIP(string RemoteIP);

    // Methods
    void CheckPlayerForTracker(Player * plr, bool online);
    bool IsBeingTracked(Player *plr);
    void LoadFromDB();
    void SaveToDB(TrackedPlr* report, QueryBuffer * buf);
    void Destroy();
    void GetGUIDCount();
    void AddTracker(TrackedPlr *plr, bool startup);
    void DelTracker(uint64 trackerId);
    void List(WorldSession *m_session, uint32 listStart);
    void CreateTracker(WorldSession *m_session, const char * args);
    void DestroyTracker(WorldSession *m_session, const char * args);

    bool IsValidPlayer(Player * plr)
    {
        return !( plr == NULL || plr->GetSession() == NULL || plr->GetSession()->GetSocket() == NULL || !plr->IsInWorld() );
    }
};

#define sTracker Tracker::getSingleton()
