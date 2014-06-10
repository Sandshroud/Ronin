/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleGMTicketCreateOpcode(WorldPacket & recv_data)
{
    uint32 map;
    float x, y, z;
    std::string message = "";
    std::string message2 = "";
    GM_Ticket *ticket = new GM_Ticket;
    WorldPacket data(SMSG_GMTICKET_CREATE, 4);

    // recv Data
    recv_data >> map;
    recv_data >> x;
    recv_data >> y;
    recv_data >> z;
    recv_data >> message;
    recv_data >> message2;

    // Crap cleanup.
    uint64 crap;
    uint16 crap2;
    uint8 crap3;
    recv_data >> crap >> crap2 >> crap3;

    // Remove pending tickets
    objmgr.RemoveGMTicketByPlayer(GetPlayer()->GetGUID());

    ticket->guid = objmgr.GenerateTicketID();
    ticket->playerGuid = GetPlayer()->GetGUID();
    ticket->map = map;
    ticket->posX = x;
    ticket->posY = y;
    ticket->posZ = z;
    ticket->message = message, message2;
    ticket->timestamp = (uint32)UNIXTIME;
    ticket->name = GetPlayer()->GetName();
    ticket->level = GetPlayer()->getLevel();
    ticket->deleted = false;
    ticket->assignedToPlayer = 0;
    ticket->comment = "";

    // Add a new one
    objmgr.AddGMTicket(ticket, false);

    // Response - no errors
    data << uint32(2);

    SendPacket(&data);

    // send message indicating new ticket
    Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(),GetPlayer());
    if(chn)
    {
        std::stringstream ss;
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        ss << "GmTicket 5, " << ticket->name;
#else
        ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_NEWTICKET;
        ss << ":" << ticket->guid;
        ss << ":" << ticket->level;
        ss << ":" << ticket->name;
#endif
        chn->Say(_player, ss.str().c_str(), NULLPLR, true);
    }
}

void WorldSession::HandleGMTicketUpdateOpcode(WorldPacket & recv_data)
{
    std::string message = "";
    WorldPacket data(SMSG_GMTICKET_UPDATETEXT, 4);

    // recv Data
    recv_data >> message;

    // Update Ticket
    GM_Ticket *ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->GetGUID());
    if(!ticket) // Player doesn't have a GM Ticket yet
    {
        // Response - error couldn't find existing Ticket
        data << uint32(1);

        SendPacket(&data);
        return;
    }
    ticket->message = message;
    ticket->timestamp = (uint32)UNIXTIME;

    objmgr.UpdateGMTicket(ticket);

    // Response - no errors
    data << uint32(2);

    SendPacket(&data);

#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
    Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(),GetPlayer());
    if(chn)
    {
        std::stringstream ss;
        ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_UPDATED;
        ss << ":" << ticket->guid;
        chn->Say(_player, ss.str().c_str(), NULLPLR, true);
    }
#endif
}

void WorldSession::HandleGMTicketDeleteOpcode(WorldPacket & recv_data)
{
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->GetGUID());

    // remove Ticket
    objmgr.RemoveGMTicketByPlayer(GetPlayer()->GetGUID());

    // Response - no errors
    WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
    data << uint32(9);
    SendPacket(&data);

    // send message to gm_sync_chan
    Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), GetPlayer());
    if(chn && ticket != NULL)
    {
        std::stringstream ss;
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        ss << "GmTicket 1," << ticket->name;
#else
        ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_REMOVED;
        ss << ":" << ticket->guid;
#endif
        chn->Say(_player, ss.str().c_str(), NULLPLR, true);
    }
}

void WorldSession::HandleGMTicketGetTicketOpcode(WorldPacket & recv_data)
{
    WorldPacket data(SMSG_GMTICKET_GETTICKET, 400);

    // get Current Ticket
    GM_Ticket *ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->GetGUID());
    if(!ticket) // no Current Ticket
    {
        data << uint32(10);
        data << uint32(0);
        SendPacket(&data);
        return;
    }

    // Send current Ticket
    data << uint32(6); // hasTicket
    data << uint32(1); // Id
    data << ticket->message.c_str();
    data << (uint8)ticket->map;

    SendPacket(&data);
}


void WorldSession::HandleGMTicketSystemStatusOpcode( WorldPacket & recv_data )
{
    WorldPacket data(SMSG_GMTICKET_SYSTEMSTATUS, 4);
    sWorld.gmList_lock.AcquireReadLock();
    data << uint32(sWorld.gmList.empty() ? 0 : 1);
    sWorld.gmList_lock.ReleaseReadLock();
    SendPacket(&data);
}

void WorldSession::HandleGMTicketSurveySubmitOpcode( WorldPacket & recv_data )
{
    std::stringstream ss;
    uint32 x;
    recv_data >> x;
    //sLog.outDebug("SURVEY: X = %u", x);

    ss << "INSERT INTO gm_surveys (playerguid, question1, answer1, question2, answer2, question3, answer3, question4, answer4, question5, answer5, question6, answer6, question7, answer7, comment, timestamp) VALUES (";
    ss << GetPlayer()->GetLowGUID() << ", ";

    for( int i = 0; i < 10; i++)
    {
        uint32 questionID;
        recv_data >> questionID;
        if( !questionID )
            break;

        uint8 value;
        std::string unk_text;
        recv_data >> value;
        recv_data >> unk_text;

        ss << questionID << ", ";
        ss << uint32(value) << ", ";

        //sLog.outDebug("SURVEY: ID %u, value %u, text %s", questionID, value, unk_text.c_str());
    }

    std::string comment;
    recv_data >> comment;
    //sLog.outDebug("SURVEY: comment %s", comment.c_str());

    ss << "'" << CharacterDatabase.EscapeString( comment ) << "', ";
    ss << uint32( UNIXTIME ) << ");";

    CharacterDatabase.Execute(ss.str( ).c_str( ));
}
