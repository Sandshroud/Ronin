/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton( TicketMgr );

TicketMgr::TicketMgr() : m_ticketid(0)
{

}

TicketMgr::~TicketMgr()
{
    sLog.Notice("TicketMgr", "Deleting GM Tickets...");
    for(GmTicketList::iterator itr = GM_TicketList.begin(); itr != GM_TicketList.end(); itr++)
        delete (*itr);
}

void TicketMgr::Load()
{
    QueryResult *result = CharacterDatabase.Query( "SELECT guid, playerGuid, name, level, map, posX, posY, posZ, message, timestamp, deleted, assignedto, comment FROM gm_tickets WHERE deleted = 0" );
    if(result)
    {
        GM_Ticket *ticket;
        do
        {
            Field *fields = result->Fetch();
            uint64 guid = fields[0].GetUInt64();
            if(m_ticketid < guid)
                m_ticketid = guid;

            ticket = new GM_Ticket(guid);
            ticket->playerGuid = fields[1].GetUInt64();
            ticket->name = fields[2].GetString();
            ticket->level = fields[3].GetUInt32();
            ticket->map = fields[4].GetUInt32();
            ticket->posX = fields[5].GetFloat();
            ticket->posY = fields[6].GetFloat();
            ticket->posZ = fields[7].GetFloat();
            ticket->message = fields[8].GetString();
            ticket->timestamp = fields[9].GetUInt32();
            ticket->deleted = false;

            ticket->assignedToPlayer = fields[11].GetUInt64();
            ticket->comment = fields[12].GetString();

            AddGMTicket(ticket, true);

        } while( result->NextRow() );
        sLog.Notice("ObjectMgr", "%u open GM Tickets loaded.", result->GetRowCount());
        delete result;
    }
}

void TicketMgr::SaveGMTicket(GM_Ticket* ticket, QueryBuffer * buf)
{
    std::stringstream ss;
    ss << "REPLACE INTO gm_tickets (`guid`, `playerGuid`, `name`, `level`, `map`, `posX`, `posY`, `posZ`, `message`, `timestamp`, `deleted`, `assignedto`, `comment`) VALUES(";
    ss << ticket->guid << ", ";
    ss << ticket->playerGuid << ", '";
    ss << CharacterDatabase.EscapeString(ticket->name) << "', ";
    ss << ticket->level << ", ";
    ss << ticket->map << ", ";
    ss << ticket->posX << ", ";
    ss << ticket->posY << ", ";
    ss << ticket->posZ << ", '";
    ss << CharacterDatabase.EscapeString(ticket->message) << "', ";
    ss << ticket->timestamp << ", ";
    ss << uint32( ticket->deleted ? 1 : 0);
    ss << ",";

    ss << ticket->assignedToPlayer << ", '";
    ss << CharacterDatabase.EscapeString(ticket->comment) << "');";

    if(buf) buf->AddQueryStr(ss.str());
    else CharacterDatabase.ExecuteNA(ss.str( ).c_str( ));
    
}

void TicketMgr::AddGMTicket(GM_Ticket *ticket, bool startup)
{
    ASSERT( ticket );
    GM_TicketList.push_back(ticket);

    // save
    if(!startup)
        SaveGMTicket(ticket, NULL);
}

void TicketMgr::UpdateGMTicket(GM_Ticket *ticket)
{
    SaveGMTicket(ticket, NULL);
}

void TicketMgr::DeleteGMTicketPermanently(uint64 ticketGuid)
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->guid == ticketGuid)
            i = GM_TicketList.erase(i);
        else
            ++i;
    }

    // kill from db
    CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE guid=%u", ticketGuid);
}

void TicketMgr::DeleteAllRemovedGMTickets()
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->deleted)
            i = GM_TicketList.erase(i);
        else
            ++i;
    }

    CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE deleted=1");
}

void TicketMgr::RemoveGMTicketByPlayer(uint64 playerGuid)
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->playerGuid == playerGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            SaveGMTicket((*i), NULL);
        }
        ++i;
    }
}

void TicketMgr::RemoveGMTicket(uint64 ticketGuid)
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->guid == ticketGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            SaveGMTicket((*i), NULL);
        }
        ++i;
    }
}

GM_Ticket* TicketMgr::GetGMTicketByPlayer(uint64 playerGuid)
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->playerGuid == playerGuid && !(*i)->deleted)
        {
            return (*i);
        }
        ++i;
    }
    return NULL;
}

GM_Ticket* TicketMgr::GetGMTicket(uint64 ticketGuid)
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->guid == ticketGuid)
        {
            return (*i);
        }
        ++i;
    }
    return NULL;
}
