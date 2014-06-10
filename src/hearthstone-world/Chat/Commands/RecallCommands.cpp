/***
 * Demonstrike Core
 */

#include "StdAfx.h"

bool ChatHandler::HandleRecallAddCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;
    Player* plr = m_session->GetPlayer();
    if(plr == NULL || !plr->IsInWorld())
        return false;

    string rc_locname = string(args);
    if(!objmgr.AddRecallLocation(string(args), plr->GetMapId(), plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetOrientation()))
        return false;

    char buf[256];
    snprintf((char*)buf, 256, "Added location to DB with MapID: %d, X: %f, Y: %f, Z: %f",
        (unsigned int)plr->GetMapId(), plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ());
    GreenSystemMessage(m_session, buf);
    sWorld.LogGM(m_session, "used recall add, added \"%s\" location to database.", rc_locname.c_str());
    return true;
}

bool ChatHandler::HandleRecallDelCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    if(!objmgr.DeleteRecallLocation(std::string(args)))
        return false;

    GreenSystemMessage(m_session, "Removed location %s", args);
    sWorld.LogGM(m_session, "used recall add, added \"%s\" location to database.", args);
    return true;
}

bool ChatHandler::HandleRecallListCommand(const char* args, WorldSession *m_session)
{
    if(!args || *args == NULL)
        return false;

    std::set<RecallLocation*> locations;
    if(!objmgr.FillRecallNames(std::string(args), locations))
        return false;

    uint32 count = 0;
    std::string recout;
    recout = "|cff00ff00Recall locations|r:\n\n";
    for(std::set<RecallLocation*>::iterator itr = locations.begin(); itr != locations.end(); itr++)
    {
        recout += "|cff00ccff";
        recout += (*itr)->RealName;
        recout += "|r, ";
        if(count%5 == 5)
            recout += "\n";
    }
    SendMultilineMessage(m_session, recout.c_str());
    return true;
}

bool ChatHandler::HandleRecallGoCommand(const char* args, WorldSession *m_session)
{
    if( args == NULL || !*args)
        return false;
    if( m_session == NULL || m_session->GetPlayer() == NULL || !m_session->GetPlayer()->IsInWorld())
        return false;

    RecallLocation *loc = objmgr.GetRecallLocByName(std::string(args));
    if( loc == NULL)
    {
        GreenSystemMessage(m_session, "Recall information not found.");
        return true;
    }

    m_session->GetPlayer()->SafeTeleport(loc->mapId, 0, loc->x, loc->y, loc->z, loc->orient);
    return true;
}

bool ChatHandler::HandleRecallPortPlayerCommand(const char* args, WorldSession * m_session)
{
    char player[255], location[255];
    if(sscanf(args, "%s %s", &player, &location) != 2)
        return false;

    Player* plr = objmgr.GetPlayer((char*)player, false);
    if(plr == NULL)
        return false;

    RecallLocation *loc = objmgr.GetRecallLocByName(std::string(args));
    if( loc == NULL)
    {
        GreenSystemMessage(m_session, "Recall information not found.");
        return true;
    }

    plr->SafeTeleport(loc->mapId, 0, loc->x, loc->y, loc->z, loc->orient);
    return true;
}
