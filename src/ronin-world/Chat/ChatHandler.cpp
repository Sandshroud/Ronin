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

uint32 GetChatTypeForOpcode(uint32 opcode)
{
    switch(opcode)
    {
    case CMSG_MESSAGECHAT_SAY: return CHAT_MSG_SAY;
    case CMSG_MESSAGECHAT_YELL: return CHAT_MSG_YELL;
    case CMSG_MESSAGECHAT_CHANNEL: return CHAT_MSG_CHANNEL;
    case CMSG_MESSAGECHAT_WHISPER: return CHAT_MSG_WHISPER;
    case CMSG_MESSAGECHAT_GUILD: return CHAT_MSG_GUILD;
    case CMSG_MESSAGECHAT_OFFICER: return CHAT_MSG_OFFICER;
    case CMSG_MESSAGECHAT_AFK: return CHAT_MSG_AFK;
    case CMSG_MESSAGECHAT_DND: return CHAT_MSG_DND;
    case CMSG_MESSAGECHAT_EMOTE: return CHAT_MSG_EMOTE;
    case CMSG_MESSAGECHAT_PARTY: return CHAT_MSG_PARTY;
    case CMSG_MESSAGECHAT_RAID: return CHAT_MSG_RAID;
    case CMSG_MESSAGECHAT_BATTLEGROUND: return CHAT_MSG_BATTLEGROUND;
    case CMSG_MESSAGECHAT_RAID_WARNING: return CHAT_MSG_RAID_WARNING;
    }
    return 0;
};

void WorldSession::HandleMessagechatOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    bool packetLang = false;
    int32 lang = LANG_UNIVERSAL;
    uint32 type = GetChatTypeForOpcode(recv_data.GetOpcode());
    bool step1 = false, step2 = false;
    uint8 flags = 0;
    std::string message, miscName;
    switch(type)
    {
    case 0: return; // Skip chat types not supported
    case CHAT_MSG_EMOTE:
    case CHAT_MSG_AFK:
    case CHAT_MSG_DND:
        {
            uint32 textLength = recv_data.ReadBits(9);
            message = recv_data.ReadString(textLength);
        }break; // No lang
    case CHAT_MSG_WHISPER: step1 = true;
    case CHAT_MSG_CHANNEL: step2 = true;
    case CHAT_MSG_PARTY:
    case CHAT_MSG_RAID_LEADER:
    case CHAT_MSG_BATTLEGROUND:
        {
            if(Group *grp = _player->GetGroup())
            {
                if(grp->GetLeader() == _player->getPlayerInfo())
                {
                    if(type == CHAT_MSG_PARTY)
                        type = CHAT_MSG_PARTY_LEADER;
                    else if(type == CHAT_MSG_RAID)
                        type = CHAT_MSG_RAID_LEADER;
                    else if(type == CHAT_MSG_BATTLEGROUND)
                        type = CHAT_MSG_BATTLEGROUND_LEADER;
                }
            }
        }
    default:
        {
            recv_data >> lang;
            packetLang = true;
            if(step1 || step2)
            {
                uint32 len1 = recv_data.ReadBits(10), len2 = recv_data.ReadBits(9);
                if(step1) miscName = recv_data.ReadString(len1), message = recv_data.ReadString(len2);
                else message = recv_data.ReadString(len2), miscName = recv_data.ReadString(len1);
            }
            else
            {
                uint32 textLength = recv_data.ReadBits(9);
                message = recv_data.ReadString(textLength);
            }
        }break;
    }

    if(GetPlayer()->IsBanned())
    {
        GetPlayer()->BroadcastMessage("You cannot do that when banned.");
        return;
    }

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
                SystemMessage("Your voice is currently muted by a moderator. This will expire in %s.", RONIN_UTIL::ConvertTimeStampToString(m_muted - (uint32)UNIXTIME).c_str());
                return;
            }
        }break;
    }

    if(!_player->hasGMTag() && !ValidateText2(message))
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
    }

    WorldPacket broadcast(type, 50);
    if(packetLang)
        lang = CanUseCommand('c') ? LANG_UNIVERSAL : lang;

    switch(type)
    {
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
                        if( HasGMPermissions() && _player->hasGMTag() )
                        {
                            if( CanUseCommand('z') )
                            {
                                if(_player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER))
                                    snprintf( Message, 512, "[DEV][%s]: %s%s|r", GetPlayer()->GetName(), MSG_COLOR_ORANGEY, message.c_str() );
                                else snprintf( Message, 512, "[ADMIN][%s]: %s%s|r", GetPlayer()->GetName(), MSG_COLOR_ORANGEY, message.c_str() );
                            } else snprintf( Message, 512, "[GM][%s]: %s%s|r", GetPlayer()->GetName(), MSG_COLOR_CYAN, message.c_str() );
                        } else snprintf( Message, 512, "[%s]: %s%s|r", GetPlayer()->GetName(), MSG_COLOR_GREEN, message.c_str() );

                        // Send message to world
                        sWorld.SendWorldText( Message );
                    } else channelmgr.BroadcastToDBCChannels(chn->pDBC->id, GetPlayer(), message.c_str());
                } else chn->Say(GetPlayer(), message.c_str(), NULL, false);
            }

            if(sWorld.bLogChat && message.c_str()[0] != '.')
                sWorld.LogChat(this, "[%s] %s: %s", miscName.c_str(), _player->GetName(), message.c_str());
        } break;
        case CHAT_MSG_WHISPER:
            {
                if(Player* target = objmgr.GetPlayer(miscName.c_str(), false))
                {
                    if( target->Social_IsIgnoring( _player->GetGUID() ) && !_player->GetSession()->HasGMPermissions() )
                    {
                        sChatHandler.FillMessageData(&broadcast, false, CHAT_MSG_IGNORED, LANG_UNIVERSAL, _player->GetGUID(), 0, _player->GetName(),  message.c_str(), "", _player->GetChatTag() );
                        SendPacket(&broadcast);
                    }
                    else
                    {
                        sChatHandler.FillMessageData(&broadcast, false, CHAT_MSG_WHISPER, lang, _player->GetGUID(), target->GetGUID(), _player->GetName(),  message.c_str(), target->GetName(), _player->GetChatTag() );
                        target->PushPacket(&broadcast);
                    }

                    //Sent the to Users id as the channel, this should be fine as it's not used for wisper
                    sChatHandler.FillPlayerMessage(&broadcast, target->GetGUID(), CHAT_MSG_WHISPER_INFORM, LANG_UNIVERSAL, message.c_str());
                    _player->PushPacket(&broadcast);

                    if(target->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
                    {
                        // Has AFK flag, autorespond.
                        sChatHandler.FillMessageData(&broadcast, false, CHAT_MSG_AFK, LANG_UNIVERSAL, _player->GetGUID(), 0, _player->GetName(),  _player->m_afk_reason.c_str(), "", _player->GetChatTag());
                        SendPacket(&broadcast);
                    }
                    else if(target->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
                    {
                        // Has AFK flag, autorespond.
                        sChatHandler.FillMessageData(&broadcast, false, CHAT_MSG_DND, LANG_UNIVERSAL, _player->GetGUID(), 0, _player->GetName(),  _player->m_afk_reason.c_str(), "", _player->GetChatTag());
                        SendPacket(&broadcast);
                    }

                    if(sWorld.bLogChat)
                        sWorld.LogChat(this, "[whisper] %s to %s: %s", _player->GetName(), target->GetName(), message.c_str());
                }
                else
                {
                    if( miscName == "Console" ||  miscName == "console" )
                    {
                        std::string ConsoleMessage = format("%s To Console: %s", _player->GetName(), message.c_str());
                        if(HasGMPermissions()) sLog.Notice("Whisper", ConsoleMessage.c_str());
                        sChatHandler.FillSystemMessageData(&broadcast, ConsoleMessage.c_str());
                    }
                    else
                    {
                        broadcast.Initialize(SMSG_CHAT_PLAYER_NOT_FOUND, miscName.length() + 1);
                        broadcast << miscName;
                    }
                    SendPacket(&broadcast);
                }
            } break;
    case CHAT_MSG_AFK:
        {
            GetPlayer()->SetAFKReason(message);

            /* WorldPacket *data, WorldSession* session, uint32 type, uint32 language, const char *channelName, const char *message*/
            if(GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
            {
                GetPlayer()->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK);
            }
            else
            {
                GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK);
            }
        } break;
    case CHAT_MSG_DND:
        {
            GetPlayer()->SetAFKReason(message);

            if(GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
                GetPlayer()->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DND);
            else GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DND);
        } break;
    default:
        {
            size_t pos = sChatHandler.FillMessageData(&broadcast, false, type, lang, _player->GetGUID(), 0, _player->GetName(), message.c_str(), "", _player->GetChatTag());
            switch(type)
            {
            case CHAT_MSG_EMOTE:
                {
                    _player->SendMessageToSet(&broadcast, true ,true );
                    if(sWorld.bLogChat) sWorld.LogChat(this, "[emote] %s: %s", _player->GetName(), message.c_str());
                }break;
            case CHAT_MSG_SAY:
                {
                    Player *target = NULL;
                    SendChatPacket(&broadcast, 1, pos, lang, this);
                    for(WorldObject::InRangeArray::iterator itr = _player->GetInRangeUnitSetBegin(); itr != _player->GetInRangeUnitSetEnd(); itr++)
                        if((target = _player->GetInRangeObject<Player>(*itr)) && target->GetDistanceSq(_player) < 1600)
                            target->GetSession()->SendChatPacket(&broadcast, 1, pos, lang, this);
                    if(sWorld.bLogChat && message.c_str()[0] != '.') sWorld.LogChat(this, "[say] %s: %s", _player->GetName(), message.c_str());
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

                    SubGroup *sgr = NULL;
                    if((type == CHAT_MSG_PARTY || type == CHAT_MSG_PARTY_LEADER) && pGroup->GetGroupType() == GROUP_TYPE_RAID)
                    {
                        // only send to that subgroup
                        if(sgr = pGroup->GetSubGroup(_player->GetSubGroup()))
                        {
                            _player->GetGroup()->Lock();
                            for(GroupMembersSet::iterator itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); itr++)
                            {
                                if((*itr)->m_loggedInPlayer == NULL)
                                    continue;
                                (*itr)->m_loggedInPlayer->GetSession()->SendChatPacket(&broadcast, 1, pos, lang, this);
                            }
                            _player->GetGroup()->Unlock();
                        }
                    }
                    else
                    {
                        for(uint32 i = 0; i < pGroup->GetSubGroupCount(); i++)
                        {
                            sgr = pGroup->GetSubGroup(i);
                            _player->GetGroup()->Lock();
                            for(GroupMembersSet::iterator itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); itr++)
                            {
                                if((*itr)->m_loggedInPlayer == NULL)
                                    continue;
                                (*itr)->m_loggedInPlayer->GetSession()->SendChatPacket(&broadcast, 1, pos, lang, this);
                            }
                            _player->GetGroup()->Unlock();
                        }
                    }
                    if(sWorld.bLogChat && message.c_str()[0] != '.') sWorld.LogChat(this, "[Party/Raid/Battleground] %s: %s", _player->GetName(), message.c_str());
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
                    _player->GetMapInstance()->SendChatMessageToCellPlayers(_player, &broadcast, 2, 1, pos, lang, this);
                    if(sWorld.bLogChat && message.c_str()[0] != '.') sWorld.LogChat(this, "[Yell] %s: %s", _player->GetName(), message.c_str());
                } break;
            default:
                {
                    sLog.outDebug("CHAT: unknown msg type %u, lang: %u", type, lang);
                }break;
            }
        }break;
    }
}

void WorldSession::HandleTextEmoteOpcode( WorldPacket & recv_data )
{
    if(!_player->IsInWorld() || !_player->isAlive())
        return;

    WoWGuid guid;
    uint32 text_emote, emoteNum;
    recv_data >> text_emote;
    recv_data >> emoteNum;
    recv_data >> guid;

    if( m_muted && m_muted >= (uint32)UNIXTIME )
    {
        SystemMessage("Your voice is currently muted by a moderator. This will expire in %s.", RONIN_UTIL::ConvertTimeStampToString(m_muted - (uint32)UNIXTIME).c_str());
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

    if(EmoteTextEntry *emText = dbcEmoteText.LookupEntry(text_emote))
    {
        WorldPacket data;
        Unit* pUnit = NULL;
        std::string unitName;
        if(pUnit = _player->GetMapInstance()->GetUnit(guid))
            unitName = pUnit->GetName();

        switch(emText->textId)
        {
        case EMOTE_STATE_SLEEP:
        case EMOTE_STATE_SIT:
        case EMOTE_STATE_KNEEL:
        case EMOTE_ONESHOT_NONE:
            break;
        case EMOTE_STATE_DANCE:
        case EMOTE_STATE_READ:
            {
                _player->SetUInt32Value(UNIT_NPC_EMOTESTATE, emText->textId);
            }break;
        default:
            {
                WorldPacket data(SMSG_EMOTE, 12);
                data << uint32(emText->textId);
                data << GetPlayer()->GetGUID();
                GetPlayer()->SendMessageToSet(&data, true);
            }break;
        }

        if(pUnit) TRIGGER_AI_EVENT(pUnit,OnEmote)(_player, EmoteType(emText->textId));

        data.Initialize(SMSG_TEXT_EMOTE);
        data << GetPlayer()->GetGUID();
        data << uint32(text_emote);
        data << uint32(emoteNum);
        data << uint32(unitName.length());
        if(!unitName.length())
            data << uint8(0x00);
        else data << unitName;
        GetPlayer()->SendMessageToSet(&data, true);
    }
}

void WorldSession::HandleSetVisibleRankOpcode(WorldPacket &recvPacket)
{

}

void WorldSession::HandleReportSpamOpcode(WorldPacket & recvPacket)
{
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
    if( text.find("|TInterface") != std::string::npos)
        return false;
    if( text.find("\n") != std::string::npos )
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
    if((stringpos = text.find("|Hquest:")) != std::string::npos)
    { //Hquest:2278:47|h[The Platinum Discs]|h|r
        ///////////////////////////////////////////////////////////////////
        size_t length = stringpos+8;
        if(text.size() < length)
            return false;

        std::string newstring = text.substr(stringpos+8, text.size());
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
    if((stringpos = text.find("|Htrade:")) != std::string::npos)
    { //|Htrade:4037:1:150:1:6AAAAAAAAAAAAAAAAAAAAAAOAADAAAAAAAAAAAAAAAAIAAAAAAAAA|h[Engineering]|h|r
        ///////////////////////////////////////////////////////////////////
        size_t length = stringpos+8;
        if(text.size() < length)
            return false;

        std::string newstring = text.substr(stringpos+8, text.size());
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
    if((stringpos = text.find("|Htalent:")) != std::string::npos)
    { //Htalent:2232:-1|h[Taste for Blood]|h|r
        ///////////////////////////////////////////////////////////////////
        size_t length = stringpos+9;
        if(text.size() < length)
            return false;

        std::string newstring = text.substr(stringpos+9, text.size());
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
    if((stringpos = text.find("|Hachievement:")) != std::string::npos)
    { //Hachievement:546:0000000000000001:0:0:0:-1:0:0:0:0|h[Safe Deposit]|h|r
        return true;
    }

    // Glyphs
    if((stringpos = text.find("|Hglyph:")) != std::string::npos)
    { //Hglyph:21:762|h[Glyph of Bladestorm]|h|r
        return true;
    }

    // Enchants
    if((stringpos = text.find("|Henchant:")) != std::string::npos)
    { //Henchant:3919|h[Engineering: Rough Dynamite]|h|r
        return true;
    }

    // Spells
    if((stringpos = text.find("|Hspell:")) != std::string::npos)
    { //|cff71d5ff|Hspell:21563|h[Command]|h|r
        ///////////////////////////////////////////////////////////////////
        size_t length = stringpos+8;
        if(text.size() < length)
            return false;

        std::string newstring = text.substr(stringpos+8, text.size());
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
    if((stringpos = text.find("Hitem:")) != std::string::npos)
    { //|cffa335ee|Hitem:812:0:0:0:0:0:0:0:70|h[Glowing Brightwood Staff]|h|r
        ///////////////////////////////////////////////////////////////////
        size_t length = stringpos+6;
        if(text.size() < length)
            return false;

        std::string newstring = text.substr(stringpos+6, text.size());
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

        ItemPrototype* proto = sItemMgr.LookupEntry(itemid);
        if(proto == NULL)
            return false;
        if(proto->Name.length() != strlen(itemname))
        {
            if(std::string(itemname).find("of") != std::string::npos)
            {
                length = proto->Name.length();
                if(newstring.size() < length)
                    return false;

                newstring = std::string(itemname).substr(length, strlen(itemname));
                if(!newstring.size())
                    return false; // Their fault

                scannedtext = (char*)newstring.c_str();
                if(std::string(scannedtext).find("of") != std::string::npos)
                    return true; // We have a suffix
            }
            return false;
        }
        // Return true here, no need to continue.
        return true;
    }

    // Safe to search, since we're done with items
    if(text.find("|c") != std::string::npos && text.find("|r") != std::string::npos)
        return false;
    if(text.find("|c") != std::string::npos && text.find("|h") != std::string::npos)
        return false;

    return true;
}
