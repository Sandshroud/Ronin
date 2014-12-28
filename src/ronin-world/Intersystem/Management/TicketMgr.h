/***
 * Demonstrike Core
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
