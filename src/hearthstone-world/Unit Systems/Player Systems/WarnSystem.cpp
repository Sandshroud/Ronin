/*
 * Sandshroud Hearthstone
 * Copyright (C) 2010 - 2011 Sandshroud <http://www.sandshroud.org/>
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
 * Crow: Credits to Egari from Feathermoon, you kick ass dude. Seriously.
 *
 */

#include "StdAfx.h"

initialiseSingleton( WarnSystem );

void PlrSendMessageToSet(Player * Plr, const char* msg)
{
    WorldPacket *data = sChatHandler.FillMessageData( CHAT_MSG_SAY, LANG_UNIVERSAL,  msg, Plr->GetGUID(), Plr->bGMTagOn ? 4 : 0 );
    Plr->SendMessageToSet( data, true );
    delete data;
}

bool WarnSystem::Punishment_TakeGold(Player *Warned, uint32 NewWarnCnt)
{
    if( NewWarnCnt <= 1 )
        return true;

    bool enoughGold = true;

    int32 CopperPerGold = 10000;
    int32 CoinageLevel = Warned->GetUInt32Value(PLAYER_FIELD_COINAGE);
    int32 CoinageToLose = ( (NewWarnCnt - 1) * 1000 * CopperPerGold );
    int32 NewCoinageLevel = CoinageLevel - CoinageToLose;
    if( NewCoinageLevel < 0 )
    {
        CoinageToLose = CoinageLevel;

        enoughGold = false;
        // If user has not enough gold he's either trying to get around it or
        // being a complete asshole while just having joined, so let's teach him a lesson! :D
        // Apply this punishment AND the punishment for the next warning.

    }

    Warned->ModUnsigned32Value( PLAYER_FIELD_COINAGE, -CoinageToLose );

    if( CoinageToLose > CopperPerGold )
    {
        char msg[1024];
        sprintf(msg, "[WarnSys] I got warned and I just lost %u gold!", (int)(CoinageToLose/CopperPerGold));
        PlrSendMessageToSet(Warned, msg);
    }

    return enoughGold;
}


void WarnSystem::ReachedPunishmentLevel(Player * Warner, Player *Warned, uint32 NewWarnCnt, const char* reason)
{
    // Gold punishment, on by default. It takes gold away depending on their New Warn Count.
    // 1000 gold is taken for each warning received after a first warning.
    // So warning 3 gets a 2000 gold fine. Warning 4 a 3000 gold fine. Etc.
    if( !Warned || !Warned->GetSession() || !NewWarnCnt )
        return;

//  if( !Punishment_TakeGold(Warned, NewWarnCnt) ) {
//      if( NewWarnCnt == 2 )
//          NewWarnCnt = 3;
//  }
    Warned->SaveToDB(false);

    if( reason == NULL )
        reason = "No reason given.";

    string pReason = string(reason);
    switch( NewWarnCnt )
    {
    case 1:
    case 2:
        {
            Punishment_None();
        }break;
    case 3:
    case 4:
        {
            Punishment_Kick(Warner, Warned, NewWarnCnt, pReason);
        }break;
    case 5:
    case 6:
        {
            Punishment_BanCharacter(Warner, Warned, NewWarnCnt, pReason);
        }break;
    default:
        {
            Punishment_PermBanAccount(Warner, Warned, NewWarnCnt, pReason);
        }break;
    }
}

void WarnSystem::AddWarn(Player * Warner, Player * Warned, const char  *reason)
{
    if( !Warner || !Warned || !Warner->GetSession() || !Warned->GetSession() )
        return;

    std::string GMChar = Warner->GetName();
    std::string WarnedChar = Warned->GetName();
    std::string GMAcct = Warner->GetSession()->GetAccountName();
    std::string WarnedAcct = Warned->GetSession()->GetAccountName();
    std::string WarnedIP = Warned->GetSession()->GetSocket()->GetIP();

    uint32 newWarnID = GetWarnCountForAccount( WarnedAcct ) + 1;

    BroadcastToWorld("%s has been warned( #%u ) for: %s", Warned->GetName(), newWarnID, reason);    // Adds BROADCAST:

    sChatHandler.GreenSystemMessage(Warner->GetSession(), "You have warned %s for: %s", WarnedChar.c_str(), reason );
    sChatHandler.BlueSystemMessage(Warned->GetSession(), "You have been warned by %s for: %s", GMChar.c_str(), reason );
    sLog.Notice("WarnSystem", "Player %s has been warned by %s for: %s", WarnedChar.c_str(), GMAcct.c_str(), reason );
    sWorld.LogGM(Warner->GetSession(), "warned(%u) player %s(%s) for %s", newWarnID, WarnedChar.c_str(), WarnedAcct.c_str(), reason);

    std::stringstream mySql;
    mySql << "INSERT INTO WarnSystem (`WarnID`,`GMAcct`,`WarnedChar`,`WarnedAcct`,`WarnedIP`, `Reason`, `time`) VALUES (";
    mySql << "'" << newWarnID << "', ";
    mySql << "'" << CharacterDatabase.EscapeString(GMAcct.c_str()) << "', ";
    mySql << "'" << CharacterDatabase.EscapeString(WarnedChar.c_str()) << "', ";
    mySql << "'" << CharacterDatabase.EscapeString(WarnedAcct.c_str()) << "', ";
    mySql << "'" << CharacterDatabase.EscapeString(WarnedIP.c_str()) << "', ";
    mySql << "'" << CharacterDatabase.EscapeString(reason) << "', ";
    mySql << "NOW())";
    CharacterDatabase.Execute( mySql.str().c_str() );

    ReachedPunishmentLevel(Warner, Warned, newWarnID, reason);
}

void WarnSystem::AddWarn_Internal(std::string Warner, Player * Warned, const char  *reason)
{
    if( Warner.empty() || !Warned || !Warned->GetSession() )
        return;

    uint32 now = (uint32)getMSTime();
    if( now - Warned->m_lastWarnCounter <= 5000 )
        return;

    Warned->m_lastWarnCounter = now;

    std::string WarnedChar = Warned->GetName();
    std::string WarnedAcct = Warned->GetSession()->GetAccountName();
    std::string WarnedIP = Warned->GetSession()->GetSocket()->GetIP();

    uint32 newWarnID = GetWarnCountForAccount( WarnedAcct ) + 1;

    BroadcastToWorld("%s has been warned( #%u ) for: %s", Warned->GetName(), newWarnID, reason);
    BroadcastToGMs("Player %s has been warned by %s for: %s", WarnedChar.c_str(), Warner.c_str(), reason );

    sChatHandler.BlueSystemMessage(Warned->GetSession(), "You have been warned by %s for: %s", Warner.c_str(), reason );
    sLog.Notice("WarnSystem", "Player %s has been warned by %s for: %s", WarnedChar.c_str(), Warner.c_str(), reason );

    ReachedPunishmentLevel(NULL, Warned, newWarnID, reason);

    std::stringstream mySql;
    mySql << "INSERT INTO WarnSystem (`WarnID`,`GMAcct`,`WarnedChar`,`WarnedAcct`,`WarnedIP`, `Reason`, `time`) VALUES (";
    mySql << "'" << newWarnID << "', ";
    mySql << "'" << CharacterDatabase.EscapeString(Warner.c_str()) << "', ";
    mySql << "'" << CharacterDatabase.EscapeString(WarnedChar.c_str()) << "', ";
    mySql << "'" << CharacterDatabase.EscapeString(WarnedAcct.c_str()) << "', ";
    mySql << "'" << CharacterDatabase.EscapeString(WarnedIP.c_str()) << "', ";
    mySql << "'" << CharacterDatabase.EscapeString(reason) << "', ";
    mySql << "NOW())";
    CharacterDatabase.Execute( mySql.str().c_str() );
}

void WarnSystem::DelWarn(Player * Warner, Player * Warned, uint32 WarnID)
{
    if(!Warner || !Warner->GetSession() || !Warned || !Warned->GetSession())
        return;

    WorldSession *gmSession = Warner->GetSession();
    WorldSession *plrSession = Warned->GetSession();

    std::string GMAcct      = Warner->GetName();
    std::string GMSession   = gmSession->GetAccountName();
    std::string WarnedChar  = Warned->GetName();
    std::string WarnedAcct  = plrSession->GetAccountName();

    std::stringstream mySql;
    mySql << "DELETE FROM WarnSystem WHERE `WarnedAcct` = '" << CharacterDatabase.EscapeString(WarnedAcct.c_str()) << "' AND WarnID = '" << WarnID << "'";
    CharacterDatabase.Execute( mySql.str().c_str() );

    sChatHandler.SystemMessage(gmSession, "Warn Deleted.");
    sChatHandler.SystemMessage(plrSession, "Warn Deleted.");

    sLog.Notice("WarnSystem", "GM %s[%s] has cleared warn %u from player %s(%s)", GMAcct.c_str(), GMSession.c_str(), WarnID, WarnedChar.c_str(), WarnedAcct.c_str());
    sWorld.LogGM(gmSession ,"deleted warn %u from player %s(%s)", WarnID, WarnedChar.c_str(), WarnedAcct.c_str());
    return;
}

void WarnSystem::ListWarns(Player * Requester, Player * Warned)
{
    if( !Requester || !Requester->GetSession() )
        return;

    WorldSession * req_session = Requester->GetSession();
    if( !Warned )
    {
        sChatHandler.SystemMessage(req_session, "You have to select an online player!");
        return; // If character doesn't exist or isn't online, return error.
    }
    bool IsGameMaster = req_session->CanUseCommand('a');

    /* MySql Query Building */
    std::string WarnedChar = Warned->GetName(); // Getting Player Name
    std::string WarnedAcct = Warned->GetSession()->GetAccountName(); // Getting Player Account.
    std::stringstream mySql;
    mySql << "SELECT `WarnID`, `GMAcct`, `WarnedChar`, `WarnedAcct`, `WarnedIP`, `Reason`, `time` FROM WarnSystem WHERE `WarnedAcct` = '" << CharacterDatabase.EscapeString(WarnedAcct.c_str()) << "'";
    QueryResult * result = CharacterDatabase.Query( mySql.str().c_str() );

    /* Query Result Handling */
    if( result )
    {
        do
        {
            /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
            Field           *fields     = result->Fetch();
            uint32          WarnID      = fields[0].GetUInt32();
            string          GMAcct      = fields[1].GetString();
            string          WarnedChar  = fields[2].GetString();
            string          WarnedAcct  = fields[3].GetString();
            string          WarnedIP    = fields[4].GetString();
            string          pReason     = fields[5].GetString();
            string          time        = fields[6].GetString();
            /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

            sChatHandler.GreenSystemMessage(req_session, "--------------------------"); // Display Warn number and reasons.
            sChatHandler.GreenSystemMessage(req_session, "WarnID: %u", WarnID );
            sChatHandler.GreenSystemMessage(req_session, "Warned on: %s", WarnedChar.c_str());
            if( IsGameMaster )
            {
                sChatHandler.GreenSystemMessage(req_session, "Warned by: %s", GMAcct.c_str() );
            }
            sChatHandler.GreenSystemMessage(req_session, "Time: %s", time.c_str());
            sChatHandler.GreenSystemMessage(req_session, "Reason: %s", pReason.c_str() );
        } while(result->NextRow());
        delete result;
    }
    else
    {
        sChatHandler.GreenSystemMessage(req_session, "No warns found.");
    }
    return;
}

void WarnSystem::ClearWarns(Player * Warner, Player *  Warned)
{
    if( !Warner || !Warner->GetSession() )
        return;

    WorldSession * gm_session = Warner->GetSession();
    // Getting Player Info
    if( !Warned )
    {
        sChatHandler.SystemMessage(gm_session, "No player found!");
        return; // If character doesn't exist or isn't online, return error.
    }

    // Get required info from both GM and player.
    std::string WarnedChar = Warned->GetName();
    std::string WarnedAcct = Warned->GetSession()->GetAccountName();
    std::string GMAcct = gm_session->GetAccountName();
    std::string GM_IP = gm_session->GetSocket()->GetIP();

    /* MySql Query Building */
    std::stringstream mySql;
    mySql << "DELETE FROM WarnSystem WHERE WarnedAcct = '" << CharacterDatabase.EscapeString(WarnedAcct.c_str()) << "'";
    CharacterDatabase.Execute( mySql.str().c_str() );

    sChatHandler.GreenSystemMessage(gm_session, "Warns cleared!");
    sChatHandler.GreenSystemMessage(Warned->GetSession(), "Your warns were cleared!");
    sLog.Notice("WarnSystem", "GM %s[%s] has cleared the warns from player %s(%s)", GMAcct.c_str(), GM_IP.c_str(), WarnedChar.c_str(), WarnedAcct.c_str());
}



void WarnSystem::BroadcastToWorld(const char* Format, ...)
{
    va_list l;
    va_start(l, Format);
    char msg1[1024];
    vsnprintf(msg1, 1024, Format, l);
    va_end(l);

    char msg2[1024];
    snprintf(msg2, 1024, "[WarnSystem] %s", msg1);
    sWorld.SendWorldText( msg2 );
}

void WarnSystem::BroadcastToGMs(const char* Format, ...)
{
    va_list l;
    va_start(l, Format);
    char msg1[1024];
    vsnprintf(msg1, 1024, Format, l);
    va_end(l);

    char msg2[1024];
    snprintf(msg2, 1024, "[WarnSystem] %s", msg1);
    sWorld.SendGMWorldText( msg2 );
}

uint32 WarnSystem::GetWarnCountForAccount(std::string WarnedAcct)
{
    uint32 cnt = 0;
    std::stringstream GetWarnCount;
    GetWarnCount << "SELECT `WarnID` FROM warnsystem WHERE `WarnedAcct` = \"" << CharacterDatabase.EscapeString(WarnedAcct.c_str()) << "\"";
    sLog.outString(GetWarnCount.str().c_str());
    QueryResult * GWCresult = CharacterDatabase.Query( GetWarnCount.str().c_str() );
    if( GWCresult )
    {
        do{
            cnt++;
        } while(GWCresult->NextRow());
        delete GWCresult;
    }
    return cnt;
}

bool ChatHandler::HandleWarnPlayerCommand(const char *args, WorldSession * m_session)
{
    if(!*args) return false;
    char *pname = strtok((char*)args, " ");
    if(!pname)
    {
        RedSystemMessage(m_session, "No name specified.");
        return false;
    }

    Player *warned = objmgr.GetPlayer((const char*)pname, false);
    if (warned)
    {
        char* pReason = strtok(NULL, "\n");
        if( !pReason || strlen(pReason) < 3 )
        {
            RedSystemMessage(m_session, "You need to specify a valid reason!");
            return false;
        }
        else
        {
            sWarnSystem.AddWarn(m_session->GetPlayer(), warned, pReason);
            return true;
        }
    }

    return false;
}

bool ChatHandler::HandleWarnListCommand(const char *args, WorldSession * m_session)
{
    if( *args )
    {
        SystemMessage(m_session, "This command does not accept arguments, player selection is required.");
        return true;
    }

    Player * Target = NULL;
    Player * SessionPlr = m_session->GetPlayer();
    if( SessionPlr && SessionPlr->GetSession() )
    {
        if( !SessionPlr->GetSession()->CanUseCommand('a') )
            Target = SessionPlr;
        else
            Target = getSelectedChar(m_session, true);
    }

    if(!Target)
        return false;

    sWarnSystem.ListWarns(SessionPlr, Target);
    return true;
}

bool ChatHandler::HandleWarnClearCommand(const char *args, WorldSession * m_session)
{
    if(!*args) return false;
    char *pname = strtok((char*)args, " ");
    if( !pname )
    {
        RedSystemMessage(m_session, "No name specified.");
        return false;
    }

    Player *chr = objmgr.GetPlayer(pname, false);
    sWarnSystem.ClearWarns(m_session->GetPlayer(), chr);
    return true;
}

bool ChatHandler::HandleWarnSingleDelCommand(const char *args, WorldSession * m_session)
{
    if( !*args )
    {
        RedSystemMessage(m_session, "No name specified.");
        return false;
    }

    char * pCharacter = strtok((char*)args, " ");
    if( !pCharacter )
    {
        RedSystemMessage(m_session, "No name specified.");
        return false;
    }

    char * pWarnID =  strtok(NULL, "\n");
    Player * pPlayer = objmgr.GetPlayer(pCharacter, false);
    if(!pWarnID || !pPlayer)
    {
        SystemMessage(m_session, "This function requires 2 arguments.");
        return false;
    }
    uint32 WarnID = atol(pWarnID);
    sWarnSystem.DelWarn(m_session->GetPlayer(), pPlayer, WarnID);
    return true;
}
