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

#include "StdAfx.h"

initialiseSingleton( Tracker );

Tracker::Tracker( )
{
    m_IdCount = 0;
    m_maxId = 0;
}

Tracker::~Tracker( )
{
    Destroy();
}

void Tracker::CheckPlayerForTracker(Player * plr, bool online)
{
    if( !IsValidPlayer(plr) || !IsBeingTracked(plr) )   // No tracker found on user!
        return;

    TrackedPlr * tracker = GetTrackerByIP(plr->GetSession()->GetSocket()->GetIP());
    if ( !tracker )
        return; // No tracker found on user!

    char text[1024];
    sprintf(text, "%sTracker:|r %s has %s on character %s", MSG_COLOR_CYAN, tracker->Name.c_str(),
        (online?"come online":"gone offline"), plr->GetName());

    WorldPacket data;
    sChatHandler.FillSystemMessageData(&data, text);
    sWorld.SendAdministratorMessage(&data);
}

bool Tracker::IsBeingTracked(Player *plr)
{
    if( !IsValidPlayer(plr) )   // No tracker found on user!
        return false;

    TrackedPlr * tracker = GetTrackerByIP(plr->GetSession()->GetSocket()->GetIP());
    return ( tracker != NULL ); // No tracker found on user!
}

void Tracker::GetGUIDCount()
{
    sLog.Notice("ObjectMgr", "HighGuid(Trackers) = %u - Size(Trackers) = %u ", m_maxId, m_IdCount);
}

uint64 Tracker::GenerateID()
{
    return ++m_maxId;
}

void Tracker::LoadFromDB()
{
    QueryResult *result = CharacterDatabase.Query( "SELECT `Id`, `AcctId`, `Name`, `IP_Address` FROM tracker" );

    TrackedPlr * user;
    if( result != NULL )
    {
        do
        {
            Field *fields       = result->Fetch();

            user                = new TrackedPlr;
            user->Id            = fields[0].GetUInt64();
            user->AcctId        = fields[1].GetUInt32();
            user->Name          = fields[2].GetString();
            user->IP_Address    = fields[3].GetString();

            AddTracker(user, true);

        } while( result->NextRow() );

        sLog.Notice("Tracker", "%u trackers identified.", result->GetRowCount());

        delete result;
    }
}

void Tracker::SaveToDB(TrackedPlr* user, QueryBuffer * buf)
{
    std::stringstream ss;
    ss << "REPLACE INTO tracker (`Id`, `AcctId`, `Name`, `IP_Address`) VALUES(";
    ss << user->Id                                      << ", ";
    ss << user->AcctId                                  << ", \"";
    ss << WorldDatabase.EscapeString(user->Name)        << "\", \"";
    ss << WorldDatabase.EscapeString(user->IP_Address)  << "\");";

    if(buf == NULL)
        CharacterDatabase.ExecuteNA(ss.str( ).c_str( ));
    else
        buf->AddQueryStr(ss.str());
}

void Tracker::AddTracker(TrackedPlr *user, bool startup)
{
    ASSERT( user != NULL );
    Tracked_List.push_back(user);
    ++m_IdCount;

    if( user->Id > m_maxId )
        m_maxId = user->Id;

    // save
    if(!startup)
        SaveToDB(user, NULL);
}

void Tracker::DelTracker(uint64 trackerId)
{
    for(TrackerTable::iterator i = Tracked_List.begin(); i != Tracked_List.end();)
    {
        if( (*i)->Id == trackerId )
        {
            std::stringstream ss;
            ss << "DELETE FROM tracker WHERE `Id` = " << (*i)->Id;
            ss << " AND `Name` = \"" << CharacterDatabase.EscapeString((*i)->Name) << "\"";
            ss << " AND `IP_Address` = \"" << CharacterDatabase.EscapeString((*i)->IP_Address) << "\"";
            ss << " AND `AcctId` = '" << (*i)->AcctId << "'";
            CharacterDatabase.Execute(ss.str().c_str());

            char msg[128];
            sprintf(msg, "%sTracker:|r %s is no longer being tracked.", MSG_COLOR_CYAN, (*i)->Name.c_str());
            sWorld.SendGMWorldText( msg, true );

            Tracked_List.erase(i);
            break;
        }
        ++i;
    }
    if( trackerId == m_maxId )
        --m_maxId;

    --m_IdCount;
}

TrackedPlr* Tracker::GetTrackerByID(uint64 trackerId)
{
    for(TrackerTable::iterator i = Tracked_List.begin(); i != Tracked_List.end();)
    {
        if((*i)->Id == trackerId)
        {
            return (*i);
        }
        ++i;
    }
    return NULL;
}

uint64 Tracker::GetTrackerId(std::string Name)
{
    for(TrackerTable::iterator i = Tracked_List.begin(); i != Tracked_List.end();)
    {
        if( strcmp((*i)->Name.c_str(),Name.c_str()) == 0 )
        {
            return (*i)->Id;
        }
        ++i;
    }
    return NULL;
}

TrackedPlr* Tracker::GetTrackerByIP(std::string RemoteIP)
{
    for(TrackerTable::iterator i = Tracked_List.begin(); i != Tracked_List.end();)
    {
        if(strcmp((*i)->IP_Address.c_str(), RemoteIP.c_str()) == 0 )
        {
            return (*i);
        }
        ++i;
    }
    return NULL;
}

void Tracker::Destroy()
{
    sLog.Notice("ObjectMgr", "Deleting list of trackers from memory...");
    for(TrackerTable::iterator itr = Tracked_List.begin(); itr != Tracked_List.end(); ++itr)
        delete (*itr);
}

void Tracker::CreateTracker(WorldSession * adder, const char * args)
{
    if(!*args)
        return;

    char *pname = strtok((char*)args, " ");
    if(!pname)
    {
        if( adder != NULL )
            sChatHandler.RedSystemMessage(adder, "No name specified.");

        return;
    }
    Player *toAdd = objmgr.GetPlayer((const char*)pname, false);
    if( toAdd && toAdd->GetSession() && toAdd->GetSession()->GetSocket() )
    {
        WorldSession * toAddSession = toAdd->GetSession();
        TrackedPlr *user            = new TrackedPlr;
        user->Id                    = sTracker.GenerateID();
        user->AcctId                = toAddSession->GetAccountId();
        user->Name                  = toAddSession->GetAccountName();
        user->IP_Address            = toAddSession->GetSocket()->GetIP();

        AddTracker(user, false);

        char msg[128];
        sprintf(msg, "%sTracker:|r User [%s] is now being tracked (Tracker ID: %u)", MSG_COLOR_CYAN, toAddSession->GetAccountName().c_str(), (uint32)user->Id);
        sWorld.SendGMWorldText( msg );
    }
    else
    {
        if( adder != NULL )
            sChatHandler.RedSystemMessage(adder, "Server was unable to handle your request, please check your syntax.");
    }
}

void Tracker::DestroyTracker(WorldSession * deleter, const char * args)
{
    if(!*args)
        return;

    uint32 trackerId = atoi((char*)args);
    if( trackerId > 0 )
    {
        DelTracker(trackerId);
    }
    else
    {
        sChatHandler.RedSystemMessage(deleter, "Server was unable to handle your request, please check your syntax.");
    }
}

void Tracker::List(WorldSession *m_session, uint32 listStart)
{
    if( m_session == NULL ) // No clue how this would happen, but let's check for it anyway.....
        return;

    char msg[128];
    sprintf(msg, "%sTracker|r - tracked users:", MSG_COLOR_CYAN);
    sChatHandler.SystemMessage(m_session, msg);

    Tracked_List.sort();
    for(TrackerTable::iterator itr = Tracked_List.begin(); itr != Tracked_List.end(); ++itr)
    {
        if( (*itr) == NULL || (*itr)->Id < listStart )
            continue;

        bool isOnline = false;
        std::string IP_Address = (*itr)->IP_Address;

        ObjectMgr::PlayerStorageMap::const_iterator pItr;
        objmgr._playerslock.AcquireReadLock();
        for (pItr = objmgr._players.begin(); pItr != objmgr._players.end(); pItr++)
        {
            if(pItr->second->GetSession() && pItr->second->GetSession()->GetSocket())
            {
                std::string RemoteIP = pItr->second->GetSession()->GetSocket()->GetIP();
                if( strcmp( RemoteIP.c_str(), IP_Address.c_str() ) == 0 )
                {
                    isOnline = true;
                    std::stringstream ss;
                    ss << " * " << pItr->second->GetName() << " - TrackerID: " << (*itr)->Id << " - [" << (*itr)->Name << "]";
                    sChatHandler.SystemMessage(m_session, ss.str().c_str());
                }
            }
        }
        objmgr._playerslock.ReleaseReadLock();

        if(!isOnline)
        {
            std::stringstream ss;
            ss << " * <Offline>[" << (*itr)->Name << "] - TrackerID: " << (*itr)->Id;
            sChatHandler.SystemMessage(m_session, ss.str().c_str());
        }

        if( (*itr)->Id > listStart + 50 )
            break;
    }
}

bool ChatHandler::HandleTrackerListCommand(const char* args, WorldSession *m_session)
{
    uint32 listStart = 0;
    if( args )
        listStart = atoi(args);

    sTracker.List(m_session, listStart);
    return true;
}

bool ChatHandler::HandleAddTrackerCommand(const char *args, WorldSession * m_session)
{
    if(!*args) return false;

    sTracker.CreateTracker(m_session, args);
    return true;
}

bool ChatHandler::HandleDelTrackerCommand(const char *args, WorldSession * m_session)
{
    if(!*args) return false;

    sTracker.DestroyTracker(m_session, args);
    return true;
}
