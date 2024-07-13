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

void GMWarden::SendGMSurvey()
{

}

bool GMWarden::HandleGMTicketListCommand(const char* args, WorldSession *m_session)
{
    Player *cplr = m_session->GetPlayer();

    Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), cplr);
    if(!chn)
        return false;

    std::stringstream ss0;
    ss0 << "GmTicket:" << GM_TICKET_CHAT_OPCODE_LISTSTART;
    chn->Say(cplr, ss0.str().c_str(), cplr, true);

    for(GmTicketList::const_iterator itr = sTicketMgr.TicketBegin(); itr != sTicketMgr.TicketEnd(); itr++)
    {
        if((*itr)->deleted)
            continue;

        Player* plr = objmgr.GetPlayer((*itr)->playerGuid);
        PlayerInfo* aplri = NULL;
        if((*itr)->assignedToPlayer)
            aplri = objmgr.GetPlayerInfo((*itr)->assignedToPlayer);

        std::stringstream ss;
        ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_LISTENTRY;
        ss << ":" << (*itr)->guid.getLow();
        ss << ":" << (plr == NULL ? (*itr)->level : plr->getLevel());
        ss << ":" << (plr == NULL ? 0 : plr->IsInWorld());
        ss << ":" << (aplri == NULL ? "" : aplri->charName.c_str());
        ss << ":" << (plr == NULL ? (*itr)->name : plr->GetName());
        ss << ":" << (*itr)->comment;
        chn->Say(cplr, ss.str().c_str(), cplr, true);
    }

    return true;
}

bool GMWarden::HandleGMTicketGetByIdCommand(const char* args, WorldSession *m_session)
{
    uint64 ticketGuid = (args ? atoi(args) : 0);
    if(!ticketGuid)
    {
        sChatHandler.RedSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player *cplr = m_session->GetPlayer();
    Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), cplr);
    if(!chn)
        return false;

    GM_Ticket *ticket = sTicketMgr.GetGMTicket(ticketGuid);
    if(ticket == NULL || ticket->deleted)
    {
        chn->Say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    char* msg = new char[ticket->message.size() + 1];
    strcpy(msg, ticket->message.c_str());
    char * start = msg, *end;
    bool firstLine = true;
    for(;;)
    {
        end = strchr(start, '\n');
        if(!end)
            break;

        *end = '\0';

        std::stringstream ss;
        ss << "GmTicket:" << (firstLine ? GM_TICKET_CHAT_OPCODE_CONTENT : GM_TICKET_CHAT_OPCODE_APPENDCONTENT);
        ss << ":" << ticket->guid.getLow();
        ss << ":" << start;
        chn->Say(cplr, ss.str().c_str(), cplr, true);

        firstLine = false;

        start = end + 1;
    }
    if(*start != '\0')
    {
        std::stringstream ss;
        ss << "GmTicket:" << (firstLine ? GM_TICKET_CHAT_OPCODE_CONTENT : GM_TICKET_CHAT_OPCODE_APPENDCONTENT);
        ss << ":" << ticket->guid.getLow();
        ss << ":" << start;
        chn->Say(cplr, ss.str().c_str(), cplr, true);
    }
    delete[] msg;

    return true;
}

bool GMWarden::HandleGMTicketRemoveByIdCommand(const char* args, WorldSession *m_session)
{
    uint64 ticketGuid = (args ? atoi(args) : 0);
    if(!ticketGuid)
    {
        sChatHandler.RedSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player *cplr = m_session->GetPlayer();
    Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), cplr);
    if(!chn)
        return false;

    GM_Ticket *ticket = sTicketMgr.GetGMTicket(ticketGuid);
    if(ticket == NULL || ticket->deleted)
    {
        chn->Say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    if(!ticket->assignedToPlayer.empty() && ticket->assignedToPlayer != cplr->GetGUID() && !cplr->GetSession()->CanUseCommand('z'))
    {
        chn->Say(cplr, "GmTicket:0:Ticket is assigned to another GM.", cplr, true);
        return true;
    }

    std::stringstream ss;
    ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_REMOVED;
    ss << ":" << ticket->guid.getLow();
    chn->Say(cplr, ss.str().c_str(), NULL, true);

    sTicketMgr.RemoveGMTicket(ticket->guid);

    Player* plr = objmgr.GetPlayer(ticket->playerGuid);
    if(plr == NULL || !plr->IsInWorld())
        return true;

    // Notify player about removing ticket
    WorldPacket data( SMSG_GMTICKET_DELETETICKET, 4 );
    data << uint32(9);
    plr->PushPacket( &data );
    // Response - Send GM Survey
    WorldPacket datab( SMSG_GM_TICKET_STATUS_UPDATE, 1 );
    datab << uint32(3);
    plr->PushPacket( &datab );
    return true;
}


bool GMWarden::HandleGMTicketAssignToCommand(const char* args, WorldSession *m_session)
{
    uint64 ticketGuid;
    char guidstring[100], name[100];
    int argc = sscanf(args,"%s %s", guidstring, name);
    if(argc < 1 || argc > 2)
        return false;

    ticketGuid = atoi(guidstring);
    if(!ticketGuid)
    {
        sChatHandler.RedSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player *cplr = m_session->GetPlayer();
    Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), cplr);
    if(!chn)
        return false;

    GM_Ticket *ticket = sTicketMgr.GetGMTicket(ticketGuid);
    if(ticket == NULL || ticket->deleted)
    {
        chn->Say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    Player* mplr = objmgr.GetPlayer(ticket->playerGuid), *plr = (argc == 1 ? cplr : objmgr.GetPlayer(name, false));
    if(plr == NULL)
    {
        chn->Say(cplr, "GmTicket:0:Player not found.", cplr, true);
        return true;
    }

    if(!plr->IsInWorld())
    {
        chn->Say(cplr, "GmTicket:0:Player isn't online.", cplr, true);
        return true;
    }

    if(plr->GetSession()->GetPermissionCount() == 0)
    {
        chn->Say(cplr, "GmTicket:0:Player is not a GM.", cplr, true);
        return true;
    }

    if(ticket->assignedToPlayer == plr->GetGUID())
    {
        chn->Say(cplr, "GmTicket:0:Ticket already assigned to this GM.", cplr, true);
        return true;
    }

    if(!ticket->assignedToPlayer.empty() && ticket->assignedToPlayer != cplr->GetGUID())
    {
        Player *aplr = objmgr.GetPlayer(ticket->assignedToPlayer);
        if(aplr != NULL && aplr->IsInWorld() && !cplr->GetSession()->CanUseCommand('z'))
        {
            chn->Say(cplr, "GmTicket:0:Ticket already assigned to another GM.", cplr, true);
            return true;
        }
    }

    ticket->assignedToPlayer = plr->GetLowGUID();
    sTicketMgr.UpdateGMTicket(ticket);

    std::stringstream ss;
    ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_ASSIGNED;
    ss << ":" << ticket->guid.getLow();
    ss << ":" << plr->GetName();
    chn->Say(cplr, ss.str().c_str(), NULL, true);
    //Send Response Packet to update Ticket
    //WorldPacket data(SMSG_GMTICKET_GETTICKET, 400);
    //data << uint32(6); // Packet Status
    //data << uint8(0x7);//static Category
    //data << ticket->message.c_str();//ticketDescription
    //data << float(0.0);//ticketAge - days //update time =  last time ticket was modified?
    //data << float(0.0);//oldestTicketTime - days
    //data << float(0.0);//updateTime - days | How recent is the data for oldest ticket time, measured in days.  If this number 1 hour, we have bad data.
    //data << unit64(2);//assignedToGM |0 - ticket is not currently assigned to a gm | 1 - ticket is assigned to a normal gm |  2 - ticket is in the escalation queue
    //data << uint64(1);//openedByGM | 0 - ticket has never been opened by a gm | 1 - ticket has been opened by a gm
    //mplr->PushPacket( &data );
    sChatHandler.SystemMessageToPlr(mplr, "SYSTEM: Your ticket has been escalated. A Senior Game Master will be with you shortly!");
    return true;
}

bool GMWarden::HandleGMTicketReleaseCommand(const char* args, WorldSession *m_session)
{
    uint64 ticketGuid = (args ? atoi(args) : 0);
    if(!ticketGuid)
    {
        sChatHandler.RedSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player *cplr = m_session->GetPlayer();
    Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), cplr);
    if(!chn)
        return false;

    GM_Ticket *ticket = sTicketMgr.GetGMTicket(ticketGuid);
    if(ticket == NULL || ticket->deleted)
    {
        chn->Say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    if(ticket->assignedToPlayer.empty())
    {
        chn->Say(cplr, "GmTicket:0:Ticket not assigned to a GM.", cplr, true);
        return true;
    }

    Player *plr = objmgr.GetPlayer(ticket->assignedToPlayer);
    if(!cplr->GetSession()->CanUseCommand('z') && plr != NULL && plr->IsInWorld() && plr->GetSession()->CanUseCommand('z'))
    {
        chn->Say(cplr, "GmTicket:0:You can not release tickets from Senior Game Masters.", cplr, true);
        return true;
    }

    ticket->assignedToPlayer = 0;
    sTicketMgr.UpdateGMTicket(ticket);

    std::stringstream ss;
    ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_RELEASED;
    ss << ":" << ticket->guid.getLow();
    chn->Say(cplr, ss.str().c_str(), NULL, true);

    return true;
}

bool GMWarden::HandleGMTicketCommentCommand(const char* args, WorldSession *m_session)
{
    uint64 ticketGuid;
    int argc = 1;
    char* comment = NULL;
    char* guidstring = (char*)args;

    // Parse arguments
    char* space = (char*)strchr(args, ' ');
    if(space)
    {
        *space = '\0';
        comment = space + 1;
        argc = 2;
    }

    ticketGuid = atoi(guidstring);
    if(!ticketGuid)
    {
        sChatHandler.RedSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player *cplr = m_session->GetPlayer();
    Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), cplr);
    if(!chn)
        return false;

    GM_Ticket *ticket = sTicketMgr.GetGMTicket(ticketGuid);
    if(ticket == NULL || ticket->deleted)
    {
        chn->Say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    if(!ticket->assignedToPlayer.empty() && ticket->assignedToPlayer != cplr->GetGUID() && !cplr->GetSession()->CanUseCommand('z'))
    {
        chn->Say(cplr, "GmTicket:0:Ticket is assigned to another GM.", cplr, true);
        return true;
    }

    ticket->comment = (argc == 1 ? "" : comment);
    sTicketMgr.UpdateGMTicket(ticket);

    std::stringstream ss;
    ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_COMMENT;
    ss << ":" << ticket->guid.getLow();
    ss << ":" << cplr->GetName();
    ss << ":" << ticket->comment;
    chn->Say(cplr, ss.str().c_str(), NULL, true);

    return true;
}

bool GMWarden::HandleGMTicketDeletePermanentCommand(const char* args, WorldSession *m_session)
{
    uint64 ticketGuid = (args ? atoi(args) : 0);
    if(!ticketGuid)
    {
        sChatHandler.RedSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player *cplr = m_session->GetPlayer();
    Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), cplr);
    if(!chn)
        return false;

    GM_Ticket *ticket = sTicketMgr.GetGMTicket(ticketGuid);
    if(ticket == NULL)
    {
        chn->Say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    Player* plr = NULL;

    if(!ticket->deleted)
    {
        plr = objmgr.GetPlayer(ticket->playerGuid);

        std::stringstream ss;
        ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_REMOVED;
        ss << ":" << ticket->guid.getLow();
        chn->Say(cplr, ss.str().c_str(), NULL, true);

        sTicketMgr.RemoveGMTicket(ticket->guid);
    }

    sTicketMgr.DeleteGMTicketPermanently(ticket->guid);
    ticket = NULL;
    if(plr != NULL && plr->IsInWorld())
    {
        // Notify player about removing ticket
        WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
        data << uint32(9);
        plr->PushPacket( &data );

        // Response - Send GM Survey
        WorldPacket datab(SMSG_GM_TICKET_STATUS_UPDATE, 1);
        datab << uint32(3);
        plr->PushPacket( &datab );
    }

    return true;
}
