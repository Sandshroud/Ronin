/***
 * Demonstrike Core
 */

#include "StdAfx.h"

extern std::string LogFileName;

static const uint32 LanguageSkills[NUM_LANGUAGES] = {
    0,              // UNIVERSAL        0x00
    109,            // ORCISH           0x01
    113,            // DARNASSIAN       0x02
    115,            // TAURAHE          0x03
    0,              // -                0x04
    0,              // -                0x05
    111,            // DWARVISH         0x06
    98,             // COMMON           0x07
    139,            // DEMON TONGUE     0x08
    140,            // TITAN            0x09
    137,            // THALSSIAN        0x0A
    138,            // DRACONIC         0x0B
    0,              // KALIMAG          0x0C
    313,            // GNOMISH          0x0D
    315,            // TROLL            0x0E
    0,              // -                0x0F
    0,              // -                0x10
    0,              // -                0x11
    0,              // -                0x12
    0,              // -                0x13
    0,              // -                0x14
    0,              // -                0x15
    0,              // -                0x16
    0,              // -                0x17
    0,              // -                0x18
    0,              // -                0x19
    0,              // -                0x1A
    0,              // -                0x1B
    0,              // -                0x1C
    0,              // -                0x1D
    0,              // -                0x1E
    0,              // -                0x1F
    0,              // -                0x20
    673,            // -                0x21
    0,              // -                0x22
    759,            // -                0x23
};

static const uint32 opcodeToChatType[16] = 
{
    CHAT_MSG_SAY,
    CHAT_MSG_YELL,
    CHAT_MSG_CHANNEL,
    CHAT_MSG_WHISPER,
    CHAT_MSG_GUILD,
    CHAT_MSG_OFFICER,
    CHAT_MSG_AFK,
    CHAT_MSG_DND,
    CHAT_MSG_EMOTE,
    CHAT_MSG_PARTY,
    CHAT_MSG_PARTY_LEADER,
    CHAT_MSG_RAID,
    CHAT_MSG_RAID_LEADER,
    CHAT_MSG_BATTLEGROUND,
    CHAT_MSG_BATTLEGROUND_LEADER,
    CHAT_MSG_RAID_WARNING
};

void WorldSession::HandleMessagechatOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    uint32 type = opcodeToChatType[recv_data.GetOpcode()-CMSG_MESSAGECHAT_SAY];
    int32 lang = LANG_UNIVERSAL;
    if (type != CHAT_MSG_EMOTE && type != CHAT_MSG_AFK && type != CHAT_MSG_DND)
        recv_data >> lang;
    if( lang >= NUM_LANGUAGES )
        return;
    if( sWorld.cross_faction_world && lang > 0)
        lang = LANG_UNIVERSAL;
    if(GetPlayer()->IsBanned())
    {
        GetPlayer()->BroadcastMessage("You cannot do that when banned.");
        return;
    }

    std::string message, miscName;
    recv_data >> message;
    if( type == CHAT_MSG_CHANNEL || type == CHAT_MSG_WHISPER )
        recv_data >> miscName;
    if (sChatHandler.ParseCommands(message.c_str(), this) > 0)
        return;

    //arghhh STFU. I'm not giving you gold or items NOOB
    switch(type)
    {
    case CHAT_MSG_EMOTE:
    case CHAT_MSG_SAY:
    case CHAT_MSG_YELL:
    case CHAT_MSG_WHISPER:
    case CHAT_MSG_CHANNEL:
    case CHAT_MSG_PARTY:
    case CHAT_MSG_PARTY_LEADER:
    case CHAT_MSG_BATTLEGROUND:
    case CHAT_MSG_BATTLEGROUND_LEADER:
    case CHAT_MSG_RAID:
    case CHAT_MSG_RAID_WARNING:
    case CHAT_MSG_RAID_LEADER:
    case CHAT_MSG_GUILD:
    case CHAT_MSG_OFFICER:
        {
            if( m_muted && m_muted >= (uint32)UNIXTIME )
            {
                SystemMessage("Your voice is currently muted by a moderator. This will expire in %s.", ConvertTimeStampToString(m_muted - (uint32)UNIXTIME).c_str());
                return;
            }
        }break;
    }

    if(!_player->bGMTagOn && !ValidateText2(message))
        return;

    if(!sHookInterface.OnChat(_player, type, lang, message, miscName))
        return;

    if( lang != -1 && !GetPermissionCount() )
    {
        if(sWorld.flood_lines)
        {
            /* flood detection, wheeee! */
            if(UNIXTIME >= floodTime)
            {
                floodLines = 0;
                floodTime = UNIXTIME + sWorld.flood_seconds;
            }

            if((++floodLines) > sWorld.flood_lines)
            {
                if( sWorld.flood_mute_after_flood )
                {
                    _player->BroadcastMessage("You have been muted for %u seconds for spamming.", sWorld.flood_mute_after_flood );
                    m_muted = (uint32)UNIXTIME + sWorld.flood_mute_after_flood;
                    return;
                }

                if(sWorld.flood_message)
                    _player->BroadcastMessage("Your message has triggered serverside flood protection. You can speak again in %u seconds.", floodTime - UNIXTIME);
                return;
            }
        }

        if( sWorld.flood_message_time )
        {
            if( !stricmp(m_repeatMessage.c_str(), message.c_str()) )
            {
                if( ((uint32)UNIXTIME - m_repeatTime) < sWorld.flood_message_time )
                {
                    _player->BroadcastMessage("Your message has triggered serverside flood protection. Please don't repeat yourself.");
                    return;
                }
            }

            m_repeatMessage = message;
            m_repeatTime = (uint32)UNIXTIME;
        }

        if( sWorld.flood_caps_pct > 0.0f )
        {
            if( message.length() >= sWorld.flood_caps_min_len )
            {
                uint32 fc = 0;
                uint32 slen = uint32(message.length());
                uint32 clen = 0;
                for(; fc < slen; ++fc)
                {
                    if( message[fc] >= 'A' && message[fc] <= 'Z' )
                        ++clen;
                }

                float pct = (float(clen) / float(slen)) * 100.0f;
                if( pct >= sWorld.flood_caps_pct )
                {
                    SystemMessage("Message blocked. Please don't speak in caps. You are are allowed %.2f%% caps in a message of %u characters.", sWorld.flood_caps_pct, sWorld.flood_caps_min_len);
                    return;
                }
            }
        }

        // filter
        if(g_chatFilter->Parse(message) == true)
        {
            SystemMessage("Your chat message was blocked by a server-side filter.");
            return;
        }
    }

    WorldPacket *data;
    switch(type)
    {
    case CHAT_MSG_EMOTE:
        {
            if(GetPlayer()->m_modlanguage >=0)
                data = sChatHandler.FillMessageData( CHAT_MSG_EMOTE, GetPlayer()->m_modlanguage,  message.c_str(), _player->GetGUID(), _player->GetChatTag());
            else
                data = sChatHandler.FillMessageData( CHAT_MSG_EMOTE, CanUseCommand('c') ? LANG_UNIVERSAL : lang,  message.c_str(), _player->GetGUID(), _player->GetChatTag());

            GetPlayer()->SendMessageToSet( data, true ,true );
            if(sWorld.bLogChat)
                sWorld.LogChat(this, "[emote] %s: %s", _player->GetName(), message.c_str());
            delete data;
        }break;
    case CHAT_MSG_SAY:
        {
            if(GetPlayer()->m_modlanguage >=0)
            {
                data = sChatHandler.FillMessageData( CHAT_MSG_SAY, GetPlayer()->m_modlanguage,  message.c_str(), _player->GetGUID(), _player->GetChatTag());
                GetPlayer()->SendMessageToSet( data, true );
            }
            else
            {
                if(lang > 0 && LanguageSkills[lang] && _player->_HasSkillLine(LanguageSkills[lang]) == false)
                    return;

                if(!sWorld.cross_faction_world && lang == 0 && !CanUseCommand('c'))
                    return;

                data = sChatHandler.FillMessageData( CHAT_MSG_SAY, lang, message.c_str(), _player->GetGUID(), _player->GetChatTag());
                SendChatPacket(data, 1, lang, this);
                for(unordered_set<Player*  >::iterator itr = _player->m_inRangePlayers.begin(); itr != _player->m_inRangePlayers.end(); itr++)
                {
                    if(_player->PhasedCanInteract((*itr))) // Matching phases.
                        (*itr)->GetSession()->SendChatPacket(data, 1, lang, this);
                }
            }
            if(sWorld.bLogChat && message.c_str()[0] != '.')
                sWorld.LogChat(this, "[say] %s: %s", _player->GetName(), message.c_str());
            delete data;
        } break;
    case CHAT_MSG_PARTY:
    case CHAT_MSG_PARTY_LEADER:
    case CHAT_MSG_RAID:
    case CHAT_MSG_RAID_LEADER:
    case CHAT_MSG_RAID_WARNING:
    case CHAT_MSG_BATTLEGROUND:
        {
            Group *pGroup = _player->GetGroup();
            if(pGroup == NULL)
                break;

            if(GetPlayer()->m_modlanguage >= 0)
                data = sChatHandler.FillMessageData( type, GetPlayer()->m_modlanguage,  message.c_str(), _player->GetGUID(), _player->GetChatTag());
            else
                data = sChatHandler.FillMessageData( type, (CanUseCommand('c') && lang != -1) ? LANG_UNIVERSAL : lang, message.c_str(), _player->GetGUID(), _player->GetChatTag());

            if((type == CHAT_MSG_PARTY || type == CHAT_MSG_PARTY_LEADER) && pGroup->GetGroupType() == GROUP_TYPE_RAID)
            {
                // only send to that subgroup
                SubGroup * sgr = _player->GetGroup() ? _player->GetGroup()->GetSubGroup(_player->GetSubGroup()) : 0;

                if(sgr)
                {
                    _player->GetGroup()->Lock();
                    for(GroupMembersSet::iterator itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); itr++)
                    {
                        if((*itr)->m_loggedInPlayer)
                            (*itr)->m_loggedInPlayer->GetSession()->SendChatPacket(data, 1, lang, this);
                    }
                    _player->GetGroup()->Unlock();
                }
            }
            else
            {
                SubGroup * sgr;
                for(uint32 i = 0; i < _player->GetGroup()->GetSubGroupCount(); i++)
                {
                    sgr = _player->GetGroup()->GetSubGroup(i);
                    _player->GetGroup()->Lock();
                    for(GroupMembersSet::iterator itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); itr++)
                    {
                        if((*itr)->m_loggedInPlayer)
                            (*itr)->m_loggedInPlayer->GetSession()->SendChatPacket(data, 1, lang, this);
                    }
                    _player->GetGroup()->Unlock();
                }
            }
            if(sWorld.bLogChat && message.c_str()[0] != '.')
                sWorld.LogChat(this, "[Party/Raid/Battleground] %s: %s", _player->GetName(), message.c_str());
            delete data;
        } break;
    case CHAT_MSG_GUILD:
        {
            if(_player->m_playerInfo == NULL)
                break;

            guildmgr.GuildChat(this, lang, message.c_str());
        } break;
    case CHAT_MSG_OFFICER:
        {
            if(_player->m_playerInfo == NULL)
                break;

            guildmgr.OfficerChat(this, lang, message.c_str());
        } break;
    case CHAT_MSG_YELL:
        {
            if(lang > 0 && LanguageSkills[lang] && _player->_HasSkillLine(LanguageSkills[lang]) == false)
                return;

            if(!sWorld.cross_faction_world && lang == 0 && !CanUseCommand('c'))
                return;

            if(GetPlayer()->m_modlanguage >= 0)
                data = sChatHandler.FillMessageData( CHAT_MSG_YELL, GetPlayer()->m_modlanguage,  message.c_str(), _player->GetGUID(), _player->GetChatTag() );
            else
                data = sChatHandler.FillMessageData( CHAT_MSG_YELL, (CanUseCommand('c') && lang != -1) ? LANG_UNIVERSAL : lang, message.c_str(), _player->GetGUID(), _player->GetChatTag() );

            _player->GetMapMgr()->SendChatMessageToCellPlayers(_player, data, 2, 1, lang, this);
            delete data;
            if(sWorld.bLogChat && message.c_str()[0] != '.')
                sWorld.LogChat(this, "[Yell] %s: %s", _player->GetName(), message.c_str());
        } break;
    case CHAT_MSG_WHISPER:
        {
            Player* player = objmgr.GetPlayer(message.c_str(), false);
            if(!player)
            {
                if( miscName == "Console" ||  miscName == "console" )
                {
                    string ConsoleMessage = format("%s To Console: %s", _player->GetName(), message.c_str());
                    sLog.Notice("Whisper", ConsoleMessage.c_str());
                    data = sChatHandler.FillSystemMessageData(ConsoleMessage.c_str());
                    SendPacket(data);
                    delete data;
                    return;
                }
                else
                {
                    data = new WorldPacket(SMSG_CHAT_PLAYER_NOT_FOUND, miscName.length() + 1);
                    *data << miscName;
                    SendPacket(data);
                    delete data;
                }
                break;
            }

            // Check that the player isn't a gm with his status on
            if(!_player->GetSession()->GetPermissionCount() && player->bGMTagOn && player->gmTargets.count(_player) == 0)
            {
                // Build automated reply
                string Reply = "This Game Master does not currently have an open ticket from you and did not receive your whisper. Please submit a new GM Ticket request if you need to speak to a GM. This is an automatic message.";
                data = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.c_str(), player->GetGUID(), 3);
                SendPacket(data);
                delete data;
                break;
            }

            if(lang > 0 && LanguageSkills[lang] && _player->_HasSkillLine(LanguageSkills[lang]) == false)
                return;

            if(!sWorld.cross_faction_world && lang == 0 && !CanUseCommand('c'))
                return;

            if( player->Social_IsIgnoring( _player->GetLowGUID() ) && !_player->GetSession()->HasGMPermissions() )
            {
                data = sChatHandler.FillMessageData( CHAT_MSG_IGNORED, LANG_UNIVERSAL,  message.c_str(), _player->GetGUID(), _player->GetChatTag() );
                SendPacket(data);
                delete data;
            }
            else
            {
                if(GetPlayer()->m_modlanguage >=0)
                    data = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, GetPlayer()->m_modlanguage,  message.c_str(), _player->GetGUID(), _player->GetChatTag() );
                else
                    data = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, ((CanUseCommand('c') || player->GetSession()->CanUseCommand('c')) && lang != -1) ? LANG_UNIVERSAL : lang,  message.c_str(), _player->GetGUID(), _player->GetChatTag() );

                player->GetSession()->SendPacket(data);
                delete data;
            }

            //Sent the to Users id as the channel, this should be fine as it's not used for wisper
            data = sChatHandler.FillMessageData(CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL,message.c_str(), player->GetGUID(), player->GetChatTag()  );
            SendPacket(data);
            delete data;

            if(player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
            {
                // Has AFK flag, autorespond.
                data = sChatHandler.FillMessageData(CHAT_MSG_AFK, LANG_UNIVERSAL,  player->m_afk_reason.c_str(),player->GetGUID(), _player->GetChatTag());
                SendPacket(data);
                delete data;
            }
            else if(player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
            {
                // Has AFK flag, autorespond.
                data = sChatHandler.FillMessageData(CHAT_MSG_DND, LANG_UNIVERSAL, player->m_afk_reason.c_str(),player->GetGUID(), _player->GetChatTag());
                SendPacket(data);
                delete data;
            }
            if(sWorld.bLogChat)
                sWorld.LogChat(this, "[whisper] %s to %s: %s", _player->GetName(), player->GetName(), message.c_str());
        } break;
    case CHAT_MSG_CHANNEL:
        {
            Channel *chn = channelmgr.GetChannel(miscName.c_str(), GetPlayer(), true);
            if(chn != NULL)
            {
                if(chn->m_general == true && chn->pDBC && chn->pDBC->id == 2)
                {
                    if(sWorld.trade_world_chat == 2)
                    {
                        char Message[512];
                        if( HasGMPermissions() && _player->bGMTagOn )
                        {
                            if( CanUseCommand('z') )
                            {
                                if(_player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER))
                                    snprintf( Message, 512, "[DEV][%s]: %s%s|r", GetPlayer()->GetName(), MSG_COLOR_ORANGEY, message.c_str() );
                                else
                                    snprintf( Message, 512, "[ADMIN][%s]: %s%s|r", GetPlayer()->GetName(), MSG_COLOR_ORANGEY, message.c_str() );
                            }
                            else
                                snprintf( Message, 512, "[GM][%s]: %s%s|r", GetPlayer()->GetName(), MSG_COLOR_CYAN, message.c_str() );
                        }
                        else
                        { snprintf( Message, 512, "[%s]: %s%s|r", GetPlayer()->GetName(), MSG_COLOR_GREEN, message.c_str() ); }

                        // Send message to world
                        sWorld.SendWorldText( Message );
                    }
                    else
                    {
                        channelmgr.BroadcastToDBCChannels(chn->pDBC->id, GetPlayer(), message.c_str());
                    }
                }
                else chn->Say(GetPlayer(), message.c_str(), NULL, false);
            }

            if(sWorld.bLogChat && message.c_str()[0] != '.')
                sWorld.LogChat(this, "[%s] %s: %s", miscName.c_str(), _player->GetName(), message.c_str());
        } break;
    case CHAT_MSG_AFK:
        {
            GetPlayer()->SetAFKReason(message);

            /* WorldPacket *data, WorldSession* session, uint32 type, uint32 language, const char *channelName, const char *message*/
            if(GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
            {
                GetPlayer()->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK);
                sEventMgr.RemoveEvents(GetPlayer(), EVENT_PLAYER_FORCE_LOGOUT);
            }
            else
            {
                GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK);
                if(GetPlayer()->m_bg)
                    sEventMgr.AddEvent(GetPlayer(), &Player::BattlegroundKick, EVENT_PLAYER_BG_KICK, 30000, 1, 0);
                sEventMgr.AddEvent(GetPlayer(), &Player::ForceLogout, true, EVENT_PLAYER_FORCE_LOGOUT, 1800000, 1, 0);
            }
            GetPlayer()->Social_TellFriendsStatus();
        } break;
    case CHAT_MSG_DND:
        {
            GetPlayer()->SetAFKReason(message);

            if(GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
                GetPlayer()->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DND);
            else
                GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DND);
            GetPlayer()->Social_TellFriendsStatus();
        } break;
    default:
        sLog.outDebug("CHAT: unknown msg type %u, lang: %u", type, lang);
        break;
    }
}

void WorldSession::HandleTextEmoteOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data, 16);
    if(!_player->IsInWorld() || !_player->isAlive())
        return;

    uint64 guid;
    uint32
        text_emote,
        unk,
        namelen =1;
    const char* name =" ";

    recv_data >> text_emote;
    recv_data >> unk;
    recv_data >> guid;

    if( m_muted && m_muted >= (uint32)UNIXTIME )
    {
        SystemMessage("Your voice is currently muted by a moderator. This will expire in %s.", ConvertTimeStampToString(m_muted - (uint32)UNIXTIME).c_str());
        return;
    }

    if( !GetPermissionCount() )
    {
        if( sWorld.flood_lines)
        {
            /* flood detection, wheeee! */
            if(UNIXTIME >= floodTime)
            {
                floodLines = 0;
                floodTime = UNIXTIME + sWorld.flood_seconds;
            }

            if((++floodLines) > sWorld.flood_lines)
            {
                if( sWorld.flood_mute_after_flood )
                {
                    _player->BroadcastMessage("You have been muted for %u seconds for spamming.", sWorld.flood_mute_after_flood );
                    m_muted = (uint32)UNIXTIME + sWorld.flood_mute_after_flood;
                    return;
                }

                if(sWorld.flood_message)
                    _player->BroadcastMessage("Your message has triggered serverside flood protection. You can speak again in %u seconds.", floodTime - UNIXTIME);

                return;
            }
        }

        if( sWorld.flood_message_time )
        {
            // fuck you /crying newbs, go QQ somewhere else
            if( m_repeatEmoteId == text_emote )
            {
                if( ((uint32)UNIXTIME - m_repeatEmoteTime) < sWorld.flood_message_time )
                {
                    _player->BroadcastMessage("Your message has triggered serverside flood protection. Please don't repeat yourself.");
                    return;
                }
            }

            m_repeatEmoteTime = (uint32)UNIXTIME;
            m_repeatEmoteId = text_emote;
        }
    }

    Unit* pUnit = _player->GetMapMgr()->GetUnit(guid);
    if(pUnit)
    {
        if(pUnit->IsPlayer())
        {
            name = TO_PLAYER( pUnit )->GetName();
            namelen = (uint32)strlen(name) + 1;
        }
        else if(pUnit->GetTypeId() == TYPEID_UNIT)
        {
            Creature* p = TO_CREATURE(pUnit);
            if(p->GetCreatureInfo())
            {
                name = p->GetCreatureInfo()->Name;
                namelen = (uint32)strlen(name) + 1;

                if( p->IsPet() )
                {
                    name = TO_PET(p)->GetName();
                    namelen = (uint32)strlen(name)+1;
                }
            }
            else
            {
                name = 0;
                namelen = 0;
            }
        }
    }

    EmoteEntry *em = dbcEmoteEntry.LookupEntry(text_emote);
    if(em)
    {
        WorldPacket data(SMSG_EMOTE, 28 + namelen);

        sHookInterface.OnEmote(_player, (EmoteType)em->textid, pUnit);
        if(pUnit)
            CALL_SCRIPT_EVENT(pUnit,OnEmote)(_player,(EmoteType)em->textid);

        switch(em->textid)
        {
        case EMOTE_STATE_SLEEP:
        case EMOTE_STATE_SIT:
        case EMOTE_STATE_KNEEL:
        case EMOTE_STATE_DANCE:
            {
                _player->SetUInt32Value(UNIT_NPC_EMOTESTATE, em->textid);
            }break;
        }

        data << (uint32)em->textid;
        data << (uint64)GetPlayer()->GetGUID();
        GetPlayer()->SendMessageToSet(&data, true);

        data.Initialize(SMSG_TEXT_EMOTE);
        data << (uint64)GetPlayer()->GetGUID();
        data << (uint32)text_emote;
        data << unk;
        data << (uint32)namelen;
        if( namelen > 1 )
            data.append(name, namelen);
        else
            data << (uint8)0x00;
        GetPlayer()->SendMessageToSet(&data, true);

        GetPlayer()->GetAchievementInterface()->HandleAchievementCriteriaDoEmote(em->Id, pUnit);
    }
}

void WorldSession::HandleReportSpamOpcode(WorldPacket & recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 29);

    // the 0 in the out packet is unknown
    GetPlayer()->GetSession()->OutPacket(SMSG_COMPLAIN_RESULT, 1, 0 );

    /* This whole thing is guess-work */
    /*uint8 unk1;
    uint64 reportedGuid;
    uint32 unk2;
    uint32 messagetype;
    uint32 unk3;
    uint32 unk4;
    std::string message;
    recvPacket >> unk1 >> reportedGuid >> unk2 >> messagetype >> unk3 >> unk4 >> message;

    Player* rPlayer = objmgr.GetPlayer((uint32)reportedGuid);
    if(!rPlayer)
        return;*/

}

/* Crow: This will verify text that is able to be said ingame.
    Note that spaces are handled differently in DBC and storage
    than they are ingame, so we use string length.
*/
bool WorldSession::ValidateText2(std::string text)
{
    size_t stringpos;

    // Idiots spamming giant pictures through the chat system
    if( text.find("|TInterface") != string::npos)
        return false;
    if( text.find("\n") != string::npos )
        return false;

    /* Crow
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Crow: Die color text! You don't belong in this world!
    ColorTxt: It was not by my hand that I am once again given flesh.
    ColorTxt: I was called here by, Humans, who wish to pay me Tribute.
    Crow: Tribute? You steal mens souls! And make them your slaves!
    ColorTxt: Perhaps the same could be said of all Religions...
    Crow: Your words are as empty as your soul...
    Crow: Mankind ill needs a savor such as you!
    ~ColorTxt breaks wine glass~
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    */

    // Quests
    if((stringpos = text.find("|Hquest:")) != string::npos)
    { //Hquest:2278:47|h[The Platinum Discs]|h|r
        ///////////////////////////////////////////////////////////////////
        size_t length = stringpos+8;
        if(text.size() < length)
            return false;

        string newstring = text.substr(stringpos+8, text.size());
        if(!newstring.size())
            return false; // Their fault

        char *scannedtext = (char*)newstring.c_str();
        char* cquestid = strtok(scannedtext, "|");
        if(!cquestid)
            return false;
        uint32 questid = atol(cquestid);
        if(!questid)
            return false;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 1+strlen(cquestid);
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(1+strlen(cquestid), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        char* clevel = strtok(scannedtext, "|");
        if(!clevel)
            return false;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 3+strlen(clevel);
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(3+strlen(clevel), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        char* questname = strtok(scannedtext, "]");
        if(!questname)
            return false;
        ///////////////////////////////////////////////////////////////////

        Quest* qst = sQuestMgr.GetQuestPointer(questid);
        if(qst == NULL)
            return false;
        if(strlen(qst->qst_title) != strlen(questname))
            return false;

        // Return true here, no need to continue.
        return true;
    }

    // Professions
    if((stringpos = text.find("|Htrade:")) != string::npos)
    { //|Htrade:4037:1:150:1:6AAAAAAAAAAAAAAAAAAAAAAOAADAAAAAAAAAAAAAAAAIAAAAAAAAA|h[Engineering]|h|r
        ///////////////////////////////////////////////////////////////////
        size_t length = stringpos+8;
        if(text.size() < length)
            return false;

        string newstring = text.substr(stringpos+8, text.size());
        if(!newstring.size())
            return false; // Their fault

        char *scannedtext = (char*)newstring.c_str();
        char* tSpellId = strtok(scannedtext, ":");
        if(!tSpellId)
            return false;
        uint32 SpellId = atol(tSpellId);
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 1+strlen(tSpellId);
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(1+strlen(tSpellId), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        char* cminimum = strtok(scannedtext, ":");
        if(!cminimum)
            return false;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 1+strlen(cminimum);
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(1+strlen(cminimum), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        char* cmaximum = strtok(scannedtext, ":");
        if(!cmaximum)
            return false;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 1+strlen(cmaximum);
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(1+strlen(cmaximum), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        char* cunk = strtok(scannedtext, ":");
        if(!cunk)
            return false;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 1+strlen(cunk);
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(1+strlen(cunk), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        char* cguid = strtok(scannedtext, "|");
        if(!cguid)
            return false;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 3+strlen(scannedtext);
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(3+strlen(scannedtext), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        char* tradename = strtok(scannedtext, "]");
        if(!tradename)
            return false;
        ///////////////////////////////////////////////////////////////////

        SpellEntry* sp = dbcSpell.LookupEntry(SpellId);
        if(sp == NULL)
            return false;
        if(strlen(sp->Name) != strlen(tradename))
            return false;

        // Return true here, no need to continue.
        return true;
    }

    // Talents
    if((stringpos = text.find("|Htalent:")) != string::npos)
    { //Htalent:2232:-1|h[Taste for Blood]|h|r
        ///////////////////////////////////////////////////////////////////
        size_t length = stringpos+9;
        if(text.size() < length)
            return false;

        string newstring = text.substr(stringpos+9, text.size());
        if(!newstring.size())
            return false; // Their fault

        char *scannedtext = (char*)newstring.c_str();
        char* ctalentid = strtok(scannedtext, ":");
        if(!ctalentid)
            return false;

        uint32 talentid = atol(ctalentid);
        if(!talentid)
            return false;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 3+strlen(ctalentid);
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(1+strlen(ctalentid), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        char* cTalentPoints = strtok(scannedtext, "|");
        if(!cTalentPoints) // Apparently, we can have -1, but not 0
            return false;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 3+strlen(cTalentPoints);
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(3+strlen(cTalentPoints), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        char* TalentName = strtok(scannedtext, "]");
        if(!TalentName)
            return false;
        ///////////////////////////////////////////////////////////////////

        TalentEntry* TE = dbcTalent.LookupEntry(talentid);
        if(TE == NULL)
            return false;

        return true;
    }

    // Achievements
    if((stringpos = text.find("|Hachievement:")) != string::npos)
    { //Hachievement:546:0000000000000001:0:0:0:-1:0:0:0:0|h[Safe Deposit]|h|r
        return true;
    }

    // Glyphs
    if((stringpos = text.find("|Hglyph:")) != string::npos)
    { //Hglyph:21:762|h[Glyph of Bladestorm]|h|r
        return true;
    }

    // Enchants
    if((stringpos = text.find("|Henchant:")) != string::npos)
    { //Henchant:3919|h[Engineering: Rough Dynamite]|h|r
        return true;
    }

    // Spells
    if((stringpos = text.find("|Hspell:")) != string::npos)
    { //|cff71d5ff|Hspell:21563|h[Command]|h|r
        ///////////////////////////////////////////////////////////////////
        size_t length = stringpos+8;
        if(text.size() < length)
            return false;

        string newstring = text.substr(stringpos+8, text.size());
        if(!newstring.size())
            return false; // Their fault

        char *scannedtext = (char*)newstring.c_str();
        char* cspellid = strtok(scannedtext, "|");
        if(!cspellid)
            return false;

        uint32 spellid = atol(cspellid);
        if(!spellid)
            return false;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 3+strlen(cspellid);
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(3+strlen(cspellid), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        char* spellname = strtok(scannedtext, "]");
        if(!spellname)
            return false;
        ///////////////////////////////////////////////////////////////////

        SpellEntry* sp = dbcSpell.LookupEntry(spellid);
        if(sp == NULL)
            return false;
        if(strlen(sp->Name) != strlen(spellname))
            return false;
        // Return true here, no need to continue.
        return true;
    }

    // Items
    if((stringpos = text.find("Hitem:")) != string::npos)
    { //|cffa335ee|Hitem:812:0:0:0:0:0:0:0:70|h[Glowing Brightwood Staff]|h|r
        ///////////////////////////////////////////////////////////////////
        size_t length = stringpos+6;
        if(text.size() < length)
            return false;

        string newstring = text.substr(stringpos+6, text.size());
        if(!newstring.size())
            return false; // Their fault

        char *scannedtext = (char*)newstring.c_str();
        char* citemid = strtok(scannedtext, ":");
        if(!citemid)
            return false;

        uint32 itemid = atol(citemid);
        if(!itemid)
            return false;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = strlen(citemid);
        if(newstring.size() < length)
            return false;

        char* end = ":";
        char* buffer[8]; // Random suffix and shit, also last one is level.
        uint8 visuals[8];
        newstring = newstring.substr(strlen(citemid), newstring.size());
        if(!newstring.size())
            return false; // Their fault

        scannedtext = (char*)newstring.c_str();
        for(uint8 i = 0; i < 8; i++)
        {
            if(i == 7)
                end = "|";

            length = 1;
            if(newstring.size() < length)
                return false;

            newstring = newstring.substr(1, newstring.size());
            if(!newstring.size())
                return true; // Our fault

            scannedtext = (char*)newstring.c_str();
            buffer[i] = strtok(scannedtext, end);
            visuals[i] = buffer[i] ? atol(buffer[i]) : 0;
            if(buffer[i])
            {
                length = strlen(buffer[i]);
                if(newstring.size() < length)
                    return false;

                newstring = newstring.substr(strlen(buffer[i]), newstring.size());
                if(!newstring.size())
                    return true; // Our fault
            }
            else
            {
                length = 1;
                if(newstring.size() < length)
                    return false;

                newstring = newstring.substr(1, newstring.size());
                if(!newstring.size())
                    return true; // Our fault
            }
        }
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        length = 3;
        if(newstring.size() < length)
            return false;

        newstring = newstring.substr(3, newstring.size());
        if(!newstring.size())
            return true; // Our fault
        scannedtext = (char*)newstring.c_str();
        char* itemname = strtok(scannedtext, "]");
        if(!itemname)
            return false;
        ///////////////////////////////////////////////////////////////////

        ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(itemid);
        if(proto == NULL)
            return false;
        if(strlen(proto->Name1) != strlen(itemname))
        {
            if(string(itemname).find("of") != string::npos)
            {
                length = strlen(proto->Name1);
                if(newstring.size() < length)
                    return false;

                newstring = string(itemname).substr(strlen(proto->Name1), strlen(itemname));
                if(!newstring.size())
                    return false; // Their fault

                scannedtext = (char*)newstring.c_str();
                if(string(scannedtext).find("of") != string::npos)
                    return true; // We have a suffix
            }
            return false;
        }
        // Return true here, no need to continue.
        return true;
    }

    // Safe to search, since we're done with items
    if(text.find("|c") != string::npos && text.find("|r") != string::npos)
        return false;
    if(text.find("|c") != string::npos && text.find("|h") != string::npos)
        return false;

    return true;
}
