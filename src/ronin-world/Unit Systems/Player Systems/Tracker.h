/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

struct TrackedPlr
{
    uint64 Id;
    uint32 AcctId;
    std::string Name;
    std::string IP_Address;
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
    uint64 GetTrackerId(std::string Name);
    TrackedPlr* GetTrackerByID(uint64 Id);
    TrackedPlr* GetTrackerByIP(std::string RemoteIP);

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
