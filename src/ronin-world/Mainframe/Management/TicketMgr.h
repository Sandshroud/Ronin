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

enum
{
    GM_TICKET_CHAT_OPCODE_NEWTICKET         = 1,
    GM_TICKET_CHAT_OPCODE_LISTSTART         = 2,
    GM_TICKET_CHAT_OPCODE_LISTENTRY         = 3,
    GM_TICKET_CHAT_OPCODE_CONTENT           = 4,
    GM_TICKET_CHAT_OPCODE_APPENDCONTENT     = 5,
    GM_TICKET_CHAT_OPCODE_REMOVED           = 6,
    GM_TICKET_CHAT_OPCODE_UPDATED           = 7,
    GM_TICKET_CHAT_OPCODE_ASSIGNED          = 8,
    GM_TICKET_CHAT_OPCODE_RELEASED          = 9,
    GM_TICKET_CHAT_OPCODE_COMMENT           = 10,
    GM_TICKET_CHAT_OPCODE_ONLINESTATE       = 11
};

struct GM_Ticket
{
    GM_Ticket(WoWGuid _guid) : guid(_guid) {};

    WoWGuid guid;
    WoWGuid playerGuid;
    std::string name;
    uint32 level;
    uint32 map;
    float posX;
    float posY;
    float posZ;
    std::string message;
    uint32 timestamp;
    bool deleted;
    WoWGuid assignedToPlayer;
    std::string comment;
};

typedef std::list<GM_Ticket*>                                       GmTicketList;

class TicketMgr : public Singleton<TicketMgr>
{
public:
    TicketMgr();
    ~TicketMgr();

    void Load();
    void SaveGMTicket(GM_Ticket* ticket, QueryBuffer * buf);

    // Gm Tickets
    void AddGMTicket(GM_Ticket *ticket, bool startup = false);
    void UpdateGMTicket(GM_Ticket *ticket);
    void RemoveGMTicketByPlayer(uint64 playerGuid);
    void RemoveGMTicket(uint64 ticketGuid);
    void DeleteGMTicketPermanently(uint64 ticketGuid);
    void DeleteAllRemovedGMTickets();
    GM_Ticket* GetGMTicket(uint64 ticketGuid);
    GM_Ticket* GetGMTicketByPlayer(uint64 playerGuid);

    // List operators
    GmTicketList::const_iterator TicketBegin() { return GM_TicketList.cbegin(); };
    GmTicketList::const_iterator TicketEnd() { return GM_TicketList.cend(); };

    uint64 GenerateTicketID() { return ++m_ticketid; }

private:
    uint64 m_ticketid;
    GmTicketList GM_TicketList;
};

#define sTicketMgr TicketMgr::getSingleton()
