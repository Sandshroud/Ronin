/***
 * Demonstrike Core
 */

#include "StdAfx.h"

bool ChatHandler::HandleSetBGScoreCommand(const char* args, WorldSession *m_session)
{
    SystemMessage(m_session, "Command not yet implemented.");
    return true;
}

bool ChatHandler::HandleStartBGCommand(const char *args, WorldSession *m_session)
{
    return true;
}

bool ChatHandler::HandlePauseBGCommand(const char *args, WorldSession *m_session)
{
    SystemMessage(m_session, "Command not yet implemented.");
    return true;
}

bool ChatHandler::HandleBGInfoCommnad(const char *args, WorldSession *m_session)
{
    SystemMessage(m_session, "Command not yet implemented.");
    return true;
}

bool ChatHandler::HandleSetWorldStateCommand(const char* args, WorldSession *m_session)
{
    uint32 id,val;
    if(sscanf(args,"%u %u", &id,&val)!=2)
        return false;

    //if(m_session->GetPlayer()->m_bg)
    //{
        m_session->GetPlayer()->GetMapInstance()->GetStateManager().CreateWorldState(id, val);
        m_session->GetPlayer()->GetMapInstance()->GetStateManager().UpdateWorldState(id, val);
    //}

    return true;
}

bool ChatHandler::HandlePlaySoundCommand(const char* args, WorldSession *m_session)
{
    SystemMessage(m_session, "Command not yet implemented.");
    return true;
}

bool ChatHandler::HandleSetBattlefieldStatusCommand(const char* args, WorldSession *m_session)
{
    return true;
}

bool ChatHandler::HandleBattlegroundExitCommand(const char* args, WorldSession* m_session)
{

    return true;
}

bool ChatHandler::HandleBattlegroundForcestartCommand(const char* args, WorldSession* m_session)
{
    return true;
}
