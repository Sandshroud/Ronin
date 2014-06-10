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
    if(m_session->GetPlayer()->m_bg && sEventMgr.HasEvent(m_session->GetPlayer()->m_bg, EVENT_BATTLEGROUND_COUNTDOWN))
    {
        sEventMgr.RemoveEvents(m_session->GetPlayer()->m_bg, EVENT_BATTLEGROUND_COUNTDOWN);
        m_session->GetPlayer()->m_bg->Start();
    }
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
        m_session->GetPlayer()->GetMapMgr()->GetStateManager().CreateWorldState(id, val);
        m_session->GetPlayer()->GetMapMgr()->GetStateManager().UpdateWorldState(id, val);
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
    uint32 type = atoi(args);
    BattlegroundManager.SendBattlegroundQueueStatus(m_session->GetPlayer(), type);
    return true;
}

bool ChatHandler::HandleBattlegroundExitCommand(const char* args, WorldSession* m_session)
{
    Player* plr = getSelectedChar(m_session, false);
    if(plr==NULL)
    {
        if(m_session->GetPlayer()!= NULL)
            plr = m_session->GetPlayer();
    }
    if(plr==NULL)
        return true;

    if(plr->m_bg && plr->IsInWorld())
        plr->m_bg->RemovePlayer(plr, false);
    return true;
}

bool ChatHandler::HandleBattlegroundForcestartCommand(const char* args, WorldSession* m_session)
{
    BattlegroundManager.EventQueueUpdate(true);
    return true;
}
