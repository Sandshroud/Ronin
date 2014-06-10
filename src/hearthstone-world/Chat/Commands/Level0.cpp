/***
 * Demonstrike Core
 */

/////////////////////////////////////////////////
//  Normal User Chat Commands
//

#include "StdAfx.h"

bool ChatHandler::ShowHelpForCommand(WorldSession *m_session, ChatCommand *table, const char* cmd)
{
    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(!hasStringAbbr(table[i].Name, cmd))
            continue;

        if(m_session->CanUseCommand(table[i].CommandGroup))
            continue;

        if(table[i].ChildCommands != NULL)
        {
            cmd = strtok(NULL, " ");
            if(cmd && ShowHelpForCommand(m_session, table[i].ChildCommands, cmd))
                return true;
        }

        if(table[i].Help == "")
        {
            SystemMessage(m_session, "There is no help for that command");
            return true;
        }

        SendMultilineMessage(m_session, table[i].Help.c_str());

        return true;
    }

    return false;
}

bool ChatHandler::HandleHelpCommand(const char* args, WorldSession *m_session)
{
//  ChatCommand *table = getCommandTable();
    WorldPacket data;

    if(!*args)
        return false;

    char* cmd = strtok((char*)args, " ");
    if(!cmd)
        return false;

    if(!ShowHelpForCommand(m_session, sComTableStore.Get(), cmd))
    {
        RedSystemMessage(m_session, "Sorry, no help was found for this command, or that command does not exist.");
    }

    return true;
}


bool ChatHandler::HandleCommandsCommand(const char* args, WorldSession *m_session)
{
    ChatCommand *table = sComTableStore.Get();
    WorldPacket data;

    std::string output;
    uint32 count = 0;

    output = "Available commands: \n\n";

    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(*args && !hasStringAbbr(table[i].Name, (char*)args))
            continue;

        if(table[i].CommandGroup != '0' && !m_session->CanUseCommand(table[i].CommandGroup))
            continue;

        switch(table[i].CommandGroup)
        {
        case 'z':
            {
                output+="|cffff6060";
                output+=table[i].Name;
                output+="|r, ";
            }
            break;
        case 'm':
            {
                output+="|cff00ffff";
                output+=table[i].Name;
                output+=", ";
            }
            break;
        case 'c':
            {
                output += "|cff00ff00";
                output += table[i].Name;
                output += "|r, ";
            }break;
        default:
            {
                output+="|cff00ccff";
                output+=table[i].Name;
                output+="|r, ";
            }
            break;
        }

        count++;
        if(count == 5)  // 5 per line
        {
            output += "\n";
            count = 0;
        }
    }
    if(count)
        output += "\n";


        //FillSystemMessageData(&data, table[i].Name);
        //m_session->SendPacket(&data);
    //}

    SendMultilineMessage(m_session, output.c_str());

    return true;
}

bool ChatHandler::HandleStartCommand(const char* args, WorldSession *m_session)
{
    Player* m_plyr = TO_PLAYER(getSelectedChar(m_session, true));
    if( m_plyr == NULL)
        return false;

    uint32 raceid = m_plyr->getRace();
    uint32 classid = m_plyr->getClass();
    std::string argument = args;

    //No arguments given, get race from selected player
    if(m_plyr && args && strlen(args) < 2)
    {
        switch (raceid)
        {
        case 1: argument = "human";     break;
        case 2: argument = "orc";       break;
        case 3: argument = "dwarf";     break;
        case 4: argument = "nightelf";  break;
        case 5: argument = "undead";    break;
        case 6: argument = "tauren";    break;
        case 7: argument = "gnome";     break;
        case 8: argument = "troll";     break;
        case 10:argument = "bloodelf";  break;
        case 11:argument = "draenei";   break;
        default:
            {
                RedSystemMessage(m_session, "Could not extract race from slected character.");
                return true;
            }

        }
    } // Optional argument
    else if(m_plyr && args && strlen(args) > 2)
    {
        HEARTHSTONE_TOLOWER(argument);

        // Teleport to specific race
        if(argument == "human")             raceid = 1;
        else if(argument == "orc")          raceid = 2;
        else if(argument == "dwarf")        raceid = 3;
        else if(argument == "nightelf")     raceid = 4;
        else if(argument == "undead")       raceid = 5;
        else if(argument == "tauren")       raceid = 6;
        else if(argument == "gnome")        raceid = 7;
        else if(argument == "troll")        raceid = 8;
        else if(argument == "bloodelf")     raceid = 10;
        else if(argument == "draenei")      raceid = 11;
        else if(argument == "deathknight")  classid = 6;
        else
        {
            RedSystemMessage(m_session, "Invalid start location! Valid locations are: human, dwarf, gnome, nightelf, draenei, orc, troll, tauren, undead, bloodelf");
            return true;
        }
    }
    else
        return false;

    //GetPlayerCreateInfo
    PlayerCreateInfo *info = NULL;
    info = objmgr.GetPlayerCreateInfo(raceid, classid);
    if(info == NULL)
    {
        RedSystemMessage(m_session, "Internal error: Could not find create info.");
            return true;
    }


    GreenSystemMessage(m_session, "Telporting %s to %s starting location.", m_plyr->GetName(), argument.c_str());
    m_plyr->SafeTeleport(info->mapId, 0, LocationVector(info->positionX, info->positionY, info->positionZ, info->Orientation));
    return true;
}


bool ChatHandler::HandleInfoCommand(const char* args, WorldSession *m_session)
{
    int gm = 0;
    int avg = 0;
    int count = 0;
    PlayerStorageMap::const_iterator itr;

    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        if(itr->second->GetSession())
        {
            count++;
            avg += itr->second->GetSession()->GetLatency();
            if(itr->second->GetSession()->GetPermissionCount())
                gm++;
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    GreenSystemMessage(m_session, "Server Revision: SS Hearthstone(%s::%s) r%u/%s-%s-%s", BUILD_TAG, BUILD_HASH_STR, BUILD_REVISION, CONFIG, PLATFORM_TEXT, ARCH);
    GreenSystemMessage(m_session, "Server Uptime: |r%s", sWorld.GetUptimeString().c_str());
    if(m_session->CanUseCommand('z'))
        GreenSystemMessage(m_session, "Useage(Win only): RAM:(%f), CPU:(%f)", sWorld.GetRAMUsage(), sWorld.GetCPUUsage());
    GreenSystemMessage(m_session, "Players: (%u Alliance/%u Horde/%u GMs)",sWorld.AlliancePlayers, sWorld.HordePlayers, gm);
    GreenSystemMessage(m_session, "Average Latency: |r%.3fms", (float)((float)avg / (float)count));
    return true;
}


bool ChatHandler::HandleNYICommand(const char* args, WorldSession *m_session)
{
    RedSystemMessage(m_session, "Not yet implemented.");
    return true;
}

bool ChatHandler::HandleDismountCommand(const char* args, WorldSession *m_session)
{
    Unit* m_target = NULLUNIT;

    Player* p_target = getSelectedChar(m_session, false);

    if(p_target)
        m_target = TO_UNIT(p_target);
    else
    {
        Creature* m_crt = getSelectedCreature(m_session, false);
        if(m_crt)
            m_target = m_crt;
    }

    if(!m_target)
        RedSystemMessage(m_session, "No target found.");
    else if( !m_target->GetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID) )
        RedSystemMessage(m_session, "Target is not mounted.");
    else
    {
        m_target->Dismount();
        BlueSystemMessage(m_session, "Unit has been dismounted.");
    }
    return true;

}

bool ChatHandler::HandleFullDismountCommand(const char * args, WorldSession *m_session)
{
    Player* p_target = getSelectedChar(m_session, false);
    if(!p_target)
    {
        SystemMessage(m_session, "Select a player or yourself first.");
        return false;
    }

   if(!p_target->IsInWorld())
       return false;

    WorldSession* sess = p_target->GetSession();

    if(!sess || !sess->GetSocket())
    {
       RedSystemMessage(m_session, "Not able to locate player %s.", sess->GetPlayer()->GetName());
       return false;
    }

    if(!p_target->m_taxiPaths.size())
        p_target->SetTaxiState(false);

    p_target->SetTaxiPath(NULL);
    p_target->UnSetTaxiPos();
    p_target->m_taxi_ride_time = 0;

    p_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
    p_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
    p_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);
    p_target->Dismount();
    sEventMgr.RemoveEvents(p_target, EVENT_PLAYER_TAXI_INTERPOLATE);

    if( p_target->m_taxiPaths.size() )
        p_target->m_taxiPaths.clear();
    return true;
}

bool ChatHandler::HandleSaveCommand(const char* args, WorldSession *m_session)
{
    Player* plr = getSelectedChar(m_session, true);

    uint32 timeLeft = plr->m_nextSave;
    if(timeLeft > 10000 && timeLeft < 100000 && !plr->ForceSaved)
    {
        plr->SaveToDB(false);
        plr->m_nextSave = timeLeft;
        plr->ForceSaved = true;
        GreenSystemMessage(m_session, "Player saved to DB");
    }
    else
        RedSystemMessage(m_session, "You can only save one extra time every 2 minutes.");
    return true;
}


bool ChatHandler::HandleGMListCommand(const char* args, WorldSession *m_session)
{
    WorldSession *gm_session;
    SessionSet::iterator itr;

    sWorld.gmList_lock.AcquireReadLock();
    bool first = true;
    for (itr = sWorld.gmList.begin(); itr != sWorld.gmList.end();)
    {
        gm_session = (*itr);
        ++itr;
        if(first)
            GreenSystemMessage(m_session, "There are following active GMs on this server:");
        first = false;

        if(gm_session->GetPlayer() && m_session != gm_session)
            SystemMessage(m_session, "%s [%s]", gm_session->GetPlayer()->GetName(), gm_session->GetPermissions());
    }
    sWorld.gmList_lock.ReleaseReadLock();
    if(first)
        SystemMessage(m_session, "There are no GMs currently logged in on this server.");

    return true;
}

bool ChatHandler::HandleRangeCheckCommand( const char *args , WorldSession *m_session )
{
    WorldPacket data;
    uint64 guid = m_session->GetPlayer()->GetSelection();
    m_session->SystemMessage( "=== RANGE CHECK ===" );
    if (guid == 0)
    {
        m_session->SystemMessage("No selection imo.");
        return true;
    }

    Unit* unit = m_session->GetPlayer()->GetMapMgr()->GetUnit( guid );
    if(!unit)
    {
        m_session->SystemMessage("Invalid selection imo.");
        return true;
    }
    float DistSq = unit->GetDistanceSq( TO_OBJECT(m_session->GetPlayer()) );
    m_session->SystemMessage( "GetDistanceSq  :   %u" , FL2UINT( DistSq ) );
    LocationVector locvec( m_session->GetPlayer()->GetPositionX() , m_session->GetPlayer()->GetPositionY() , m_session->GetPlayer()->GetPositionZ() );
    float DistReal = unit->CalcDistance( locvec );
    m_session->SystemMessage( "CalcDistance   :   %u" , FL2UINT( DistReal ) );
    float Dist2DSq = unit->GetDistance2dSq( TO_OBJECT(m_session->GetPlayer()) );
    m_session->SystemMessage( "GetDistance2dSq:   %u" , FL2UINT( Dist2DSq ) );
    return true;
}

bool ChatHandler::HandleGmLogCommentCommand( const char *args , WorldSession *m_session )
{
    if(!args || !strlen(args)) return false;
    BlueSystemMessage(m_session, "Added Logcomment: %s",args);
    sWorld.LogGM(m_session,"Logcomment: %s", args);
    return true;
}

bool ChatHandler::HandleRatingsCommand( const char *args , WorldSession *m_session )
{
    m_session->SystemMessage("Ratings!!!");
    Player* m_plyr = getSelectedChar(m_session, false);
    for( uint32 i = 0; i < 26; i++ )
    {
        m_plyr->ModUnsigned32Value( PLAYER_FIELD_COMBAT_RATING_1 + i, i );
    }
    m_plyr->UpdateStats();
    return true;
}

bool ChatHandler::HandleModifyPlayerFlagsCommand(const char *args, WorldSession *m_session)
{
    Player* player = getSelectedChar(m_session);
    if(player == NULL || !args)
        return true;
    uint32 flags = atol(args);
    player->SetFlag(PLAYER_FLAGS, flags);
    return true;
}

bool ChatHandler::HandleModifyAuraStateCommand(const char *args, WorldSession *m_session)
{
    Player* player = getSelectedChar(m_session);
    if(player == NULL || !args)
        return true;

    uint32 flags = uint32(uint32(1) << (atol(args) - 1));
    player->SetFlag(UNIT_FIELD_AURASTATE, flags);
    return true;
}

bool ChatHandler::HandleMirrorTimerCommand( const char *args , WorldSession *m_session )
{
    uint32 type = 0, spellid = 0;
    sscanf(args, "%u %u", &type, &spellid);

    WorldPacket data(SMSG_START_MIRROR_TIMER, 20);
    data << uint32(atol(args));
    data << int32(60000);
    data << int32(60000);
    data << uint32(0xFFFFFFFF);
    data << uint8(0);
    data << uint32(spellid);
    m_session->SendPacket(&data);
    return true;
}
