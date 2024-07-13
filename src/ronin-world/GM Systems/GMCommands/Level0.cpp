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

/////////////////////////////////////////////////
//  Normal User Chat Commands
//

#include "StdAfx.h"

bool GMWarden::ShowHelpForCommand(WorldSession *m_session, ChatCommand *table, const char* cmd)
{
    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(!ChatHandler::hasStringAbbr(table[i].Name, cmd))
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
            sChatHandler.SystemMessage(m_session, "There is no help for that command");
            return true;
        }

        sChatHandler.sChatHandler.SendMultilineMessage(m_session, table[i].Help.c_str());

        return true;
    }

    return false;
}

bool GMWarden::HandleHelpCommand(const char* args, WorldSession *m_session)
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
        sChatHandler.RedSystemMessage(m_session, "Sorry, no help was found for this command, or that command does not exist.");
    }

    return true;
}


bool GMWarden::HandleCommandsCommand(const char* args, WorldSession *m_session)
{
    ChatCommand *table = sComTableStore.Get();
    WorldPacket data;

    std::string output;
    uint32 count = 0;

    output = "Available commands: \n\n";

    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(*args && !ChatHandler::hasStringAbbr(table[i].Name, (char*)args))
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


        //FillsChatHandler.SystemMessageData(&data, table[i].Name);
        //m_session->SendPacket(&data);
    //}

    sChatHandler.sChatHandler.SendMultilineMessage(m_session, output.c_str());

    return true;
}

bool GMWarden::HandleStartCommand(const char* args, WorldSession *m_session)
{
    Player* m_plyr = castPtr<Player>(getSelectedChar(m_session, true));
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
                sChatHandler.RedSystemMessage(m_session, "Could not extract race from slected character.");
                return true;
            }

        }
    } // Optional argument
    else if(m_plyr && args && strlen(args) > 2)
    {
        RONIN_UTIL::TOLOWER(argument);

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
            sChatHandler.RedSystemMessage(m_session, "Invalid start location! Valid locations are: human, dwarf, gnome, nightelf, draenei, orc, troll, tauren, undead, bloodelf");
            return true;
        }
    } else return false;

    //GetPlayerCreateInfo
    PlayerCreateInfo *info = NULL;
    info = objmgr.GetPlayerCreateInfo(raceid, classid);
    if(info == NULL)
    {
        sChatHandler.RedSystemMessage(m_session, "Internal error: Could not find create info.");
            return true;
    }


    sChatHandler.GreenSystemMessage(m_session, "Telporting %s to %s starting location.", m_plyr->GetName(), argument.c_str());
    m_plyr->SafeTeleport(info->mapId, 0, LocationVector(info->positionX, info->positionY, info->positionZ, info->Orientation));
    return true;
}


bool GMWarden::HandleInfoCommand(const char* args, WorldSession *m_session)
{
    int gm = 0;
    int avg = 0;
    int count = 0;
    ObjectMgr::PlayerStorageMap::const_iterator itr;

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

    sChatHandler.GreenSystemMessage(m_session, "Server Revision: SS Ronin(%s::%s) r%u/%s-%s-%s", BUILD_TAG, BUILD_HASH_STR, BUILD_REVISION, CONFIG, PLATFORM_TEXT, ARCH);
    sChatHandler.GreenSystemMessage(m_session, "Server Uptime: |r%s", sWorld.GetUptimeString().c_str());
    if(m_session->CanUseCommand('z'))
        sChatHandler.GreenSystemMessage(m_session, "Useage(Win only): RAM:(%f), CPU:(%f)", sWorld.GetRAMUsage(), sWorld.GetAverageCPUUsage());
    sChatHandler.GreenSystemMessage(m_session, "Players: (%u Alliance/%u Horde/%u GMs)",sWorld.AlliancePlayers, sWorld.HordePlayers, gm);
    sChatHandler.GreenSystemMessage(m_session, "Average Latency: |r%.3fms", (float)((float)avg / (float)count));
    return true;
}


bool GMWarden::HandleNYICommand(const char* args, WorldSession *m_session)
{
    sChatHandler.RedSystemMessage(m_session, "Not yet implemented.");
    return true;
}

bool GMWarden::HandleDismountCommand(const char* args, WorldSession *m_session)
{
    Unit* m_target = NULL;

    Player* p_target = getSelectedChar(m_session, false);

    if(p_target)
        m_target = castPtr<Unit>(p_target);
    else
    {
        Creature* m_crt = getSelectedCreature(m_session, false);
        if(m_crt)
            m_target = m_crt;
    }

    if(!m_target)
        sChatHandler.RedSystemMessage(m_session, "No target found.");
    else if( !m_target->GetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID) )
        sChatHandler.RedSystemMessage(m_session, "Target is not mounted.");
    else
    {
        m_target->Dismount();
        sChatHandler.BlueSystemMessage(m_session, "Unit has been dismounted.");
    }
    return true;

}

bool GMWarden::HandleFullDismountCommand(const char * args, WorldSession *m_session)
{
    Player* p_target = getSelectedChar(m_session, false);
    if(p_target == NULL || !p_target->IsInWorld())
    {
        sChatHandler.SystemMessage(m_session, "Select a player or yourself first.");
        return true;
    }

    WorldSession* sess = p_target->GetSession();
    if(!sess || !sess->GetSocket())
    {
       sChatHandler.RedSystemMessage(m_session, "Not able to locate player %s.", sess->GetPlayer()->GetName());
       return true;
    }

    TaxiPath *path = p_target->GetTaxiPath();
    if(path == NULL)
        return false;
    p_target->JumpToEndTaxiNode(p_target->GetTaxiPath());
    return true;
}

bool GMWarden::HandleSaveCommand(const char* args, WorldSession *m_session)
{
    if(Player* plr = getSelectedChar(m_session, true))
    {
        plr->SaveToDB(false);
        sChatHandler.GreenSystemMessage(m_session, "Player saved to DB");
    } else sChatHandler.RedSystemMessage(m_session, "You can only save one extra time every 2 minutes.");
    return true;
}


bool GMWarden::HandleGMListCommand(const char* args, WorldSession *m_session)
{
    //sWorld.SendGMList(m_session);
    /*WorldSession *gm_session;
    SessionSet::iterator itr;
    sWorld.gmList_lock.AcquireReadLock();
    bool first = true;
    for (itr = sWorld.gmList.begin(); itr != sWorld.gmList.end();)
    {
        gm_session = (*itr);
        ++itr;
        if(first)
            sChatHandler.GreenSystemMessage(m_session, "There are following active GMs on this server:");
        first = false;

        if(gm_session->GetPlayer() && m_session != gm_session)
            sChatHandler.SystemMessage(m_session, "%s [%s]", gm_session->GetPlayer()->GetName(), gm_session->GetPermissions());
    }
    sWorld.gmList_lock.ReleaseReadLock();
    if(first)
        sChatHandler.SystemMessage(m_session, "There are no GMs currently logged in on this server.");*/

    return true;
}

bool GMWarden::HandleRangeCheckCommand( const char *args , WorldSession *m_session )
{
    WorldPacket data;
    WoWGuid guid = m_session->GetPlayer()->GetSelection();
    m_session->SystemMessage( "=== RANGE CHECK ===" );
    if (!guid)
    {
        m_session->SystemMessage("No selection");
        return true;
    }

    Unit* unit = m_session->GetPlayer()->GetMapInstance()->GetUnit( guid );
    if(!unit)
    {
        m_session->SystemMessage("Invalid selection");
        return true;
    }
    float DistSq = unit->GetDistanceSq( m_session->GetPlayer() );
    m_session->SystemMessage( "GetDistanceSq  :   %u" , float2int32( DistSq ) );
    LocationVector locvec( m_session->GetPlayer()->GetPositionX() , m_session->GetPlayer()->GetPositionY() , m_session->GetPlayer()->GetPositionZ() );
    float DistReal = sqrtf(unit->GetDistanceSq( locvec ));
    m_session->SystemMessage( "CalcDistance   :   %u" , float2int32( DistReal ) );
    float Dist2DSq = unit->GetDistance2dSq( m_session->GetPlayer() );
    m_session->SystemMessage( "GetDistance2dSq:   %u" , float2int32( Dist2DSq ) );
    return true;
}

bool GMWarden::HandleGmLogCommentCommand( const char *args , WorldSession *m_session )
{
    if(!args || !strlen(args)) return false;
    sChatHandler.BlueSystemMessage(m_session, "Added Logcomment: %s",args);
    sWorld.LogGM(m_session,"Logcomment: %s", args);
    return true;
}

bool GMWarden::HandleRatingsCommand( const char *args , WorldSession *m_session )
{
    m_session->SystemMessage("Ratings!!!");
    Player* m_plyr = getSelectedChar(m_session, false);

    return true;
}

bool GMWarden::HandleModifyPlayerFlagsCommand(const char *args, WorldSession *m_session)
{
    Player* player = getSelectedChar(m_session);
    if(player == NULL || !args)
        return true;
    uint32 flags = atol(args);
    player->SetFlag(PLAYER_FLAGS, flags);
    return true;
}

bool GMWarden::HandleModifyAuraStateCommand(const char *args, WorldSession *m_session)
{
    Player* player = getSelectedChar(m_session);
    if(player == NULL || !args)
        return true;

    uint32 flags = uint32(uint32(1) << (atol(args) - 1));
    player->SetFlag(UNIT_FIELD_AURASTATE, flags);
    return true;
}

bool GMWarden::HandleMirrorTimerCommand( const char *args , WorldSession *m_session )
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
