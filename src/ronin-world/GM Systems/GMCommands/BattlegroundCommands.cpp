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

bool GMWarden::HandleSetBGScoreCommand(const char* args, WorldSession *m_session)
{
    sChatHandler.SystemMessage(m_session, "Command not yet implemented.");
    return true;
}

bool GMWarden::HandleStartBGCommand(const char *args, WorldSession *m_session)
{
    return true;
}

bool GMWarden::HandlePauseBGCommand(const char *args, WorldSession *m_session)
{
    sChatHandler.SystemMessage(m_session, "Command not yet implemented.");
    return true;
}

bool GMWarden::HandleBGInfoCommnad(const char *args, WorldSession *m_session)
{
    sChatHandler.SystemMessage(m_session, "Command not yet implemented.");
    return true;
}

bool GMWarden::HandleSetWorldStateCommand(const char* args, WorldSession *m_session)
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

bool GMWarden::HandlePlaySoundCommand(const char* args, WorldSession *m_session)
{
    sChatHandler.SystemMessage(m_session, "Command not yet implemented.");
    return true;
}

bool GMWarden::HandleSetBattlefieldStatusCommand(const char* args, WorldSession *m_session)
{
    return true;
}

bool GMWarden::HandleBattlegroundExitCommand(const char* args, WorldSession* m_session)
{

    return true;
}

bool GMWarden::HandleBattlegroundForcestartCommand(const char* args, WorldSession* m_session)
{
    return true;
}
