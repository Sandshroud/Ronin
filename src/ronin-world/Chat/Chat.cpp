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

initialiseSingleton( ChatHandler );

ChatHandler::ChatHandler() { }

ChatHandler::~ChatHandler() { }

bool ChatHandler::hasStringAbbr(const char* s1, const char* s2)
{
    for(;;)
    {
        if( !*s2 )
            return true;
        else if( !*s1 )
            return false;
        else if( tolower( *s1 ) != tolower( *s2 ) )
            return false;
        s1++; s2++;
    }
}

void ChatHandler::SendMultilineMessage(WorldSession *m_session, const char *str)
{
    char * start = (char*)str, *end;
    for(;;)
    {
        end = strchr(start, '\n');
        if(!end)
            break;

        *end = '\0';
        SystemMessage(m_session, start);
        start = end + 1;
    }
    if(*start != '\0')
        SystemMessage(m_session, start);
}

int ChatHandler::ParseCommands(uint32 type, const char* text, Player *plr)
{
    if (*text == NULL || !plr->hasGMTag())
        return 0;

    bool result = false;
    switch (text[0])
    {   // Allow . and #
    case '.':
    case '#':
        result = true;
        size_t len = strlen(text);
        // Parse out anything from the front of the input command
        while (len && (text[0] == '.' || text[0] == '#'))
            text++, len--;
        break;
    }

    if(result)
        sGMWarden.ProcessGMCommand(plr, text);
    return 1;
}

size_t ChatHandler::_FillBasicStructure(WorldPacket *data, bool gmMessage, uint8 type, int32 lang, WoWGuid sender, uint32 unk/* = 0*/)
{
    data->Initialize(gmMessage ? SMSG_GM_MESSAGECHAT : SMSG_MESSAGECHAT, 500);
    *data << type;
    size_t langPos = data->wpos();
    *data << lang << sender;
    *data << uint32(unk);
    return langPos;
}

void ChatHandler::_FillMessageStructure(WorldPacket *data, int32 lang, std::string message, uint8 chatTag)
{
    if (lang == LANG_ADDON)
        *data << uint8(0);
    *data << uint32(message.length()+1) << message << uint8(chatTag);
}

size_t ChatHandler::_FillMonsterMessageStructure(WorldPacket *data, std::string senderName, WoWGuid targetGuid, std::string receiverName)
{
    *data << uint32(senderName.length() + 1) << senderName;
    size_t guidPos = data->wpos();
    *data << targetGuid;
    if (!targetGuid.empty() && (targetGuid.getHigh() != HIGHGUID_TYPE_PLAYER && targetGuid.getHigh() != HIGHGUID_TYPE_PET))
    {
        *data << uint32(receiverName.length() + 1);
        *data << receiverName;
    }
    return guidPos;
}

void ChatHandler::FillPlayerMessage(WorldPacket *data, WoWGuid from, uint8 type, int32 language, std::string message, uint8 chatTag, const char *header, bool gmMessage)
{
    data->Initialize(gmMessage ? SMSG_GM_MESSAGECHAT : SMSG_MESSAGECHAT, 500);
    _FillBasicStructure(data, gmMessage, type, language, from);
    if(header)
        *data << header;
    else *data << from;
    _FillMessageStructure(data, language, message, chatTag);
}

void ChatHandler::FillMessageData(WorldPacket *data, bool gmMessage, uint8 type, int32 language, WoWGuid senderGuid, WoWGuid receiverGuid, std::string senderName, std::string message, std::string receiverName, uint8 chatTag)
{
    data->Initialize(gmMessage ? SMSG_GM_MESSAGECHAT : SMSG_MESSAGECHAT, 500);
    _FillBasicStructure(data, gmMessage, type, language, senderGuid);
    switch(type)
    {
    case CHAT_MSG_MONSTER_SAY:
    case CHAT_MSG_MONSTER_PARTY:
    case CHAT_MSG_MONSTER_YELL:
    case CHAT_MSG_MONSTER_WHISPER:
    case CHAT_MSG_MONSTER_EMOTE:
    case CHAT_MSG_RAID_BOSS_EMOTE:
    case CHAT_MSG_RAID_BOSS_WHISPER:
    case CHAT_MSG_BATTLENET:
        _FillMonsterMessageStructure(data, senderName, receiverGuid, receiverName);
        break;
    case CHAT_MSG_WHISPER_FOREIGN:
        {
            *data << uint32(senderName.length() + 1);
            *data << senderName;
            *data << uint64(receiverGuid);
        }break;
    case CHAT_MSG_BG_SYSTEM_NEUTRAL:
    case CHAT_MSG_BG_SYSTEM_ALLIANCE:
    case CHAT_MSG_BG_SYSTEM_HORDE:
        {
            *data << uint64(receiverGuid);
            if (!receiverGuid.empty() && receiverGuid.getHigh() != HIGHGUID_TYPE_PLAYER)
            {
                *data << uint32(receiverName.length() + 1);
                *data << receiverName;
            }
        }break;
    case CHAT_MSG_ACHIEVEMENT:
    case CHAT_MSG_GUILD_ACHIEVEMENT:
        *data << uint64(receiverGuid);
        break;
    default:
        {
            if (type == CHAT_MSG_CHANNEL)
            {
                ASSERT(senderName.length() > 0);
                *data << senderName;
            }
            *data << uint64(receiverGuid);
        }break;
    }

    _FillMessageStructure(data, language, message, chatTag);
}

void ChatHandler::FillBroadcastMessage(WorldPacket *data, size_t &guidPos, size_t &langPos, bool gmMessage, uint8 type, int32 language, WoWGuid senderGuid, WoWGuid receiverGuid, std::string senderName, std::string message, std::string receiverName, uint8 chatTag)
{
    data->Initialize(gmMessage ? SMSG_GM_MESSAGECHAT : SMSG_MESSAGECHAT, 500);
    langPos = _FillBasicStructure(data, gmMessage, type, language, senderGuid);

    switch(type)
    {
    case CHAT_MSG_MONSTER_SAY:
    case CHAT_MSG_MONSTER_PARTY:
    case CHAT_MSG_MONSTER_YELL:
    case CHAT_MSG_MONSTER_WHISPER:
    case CHAT_MSG_MONSTER_EMOTE:
    case CHAT_MSG_RAID_BOSS_EMOTE:
    case CHAT_MSG_RAID_BOSS_WHISPER:
    case CHAT_MSG_BATTLENET:
        guidPos = _FillMonsterMessageStructure(data, senderName, receiverGuid, receiverName);
        break;
    case CHAT_MSG_WHISPER_FOREIGN:
        {
            *data << uint32(senderName.length() + 1);
            *data << senderName;
            guidPos = data->wpos();
            *data << uint64(receiverGuid);
        }break;
    case CHAT_MSG_BG_SYSTEM_NEUTRAL:
    case CHAT_MSG_BG_SYSTEM_ALLIANCE:
    case CHAT_MSG_BG_SYSTEM_HORDE:
        {
            guidPos = data->wpos();
            *data << uint64(receiverGuid);
            if (!receiverGuid.empty() && receiverGuid.getHigh() != HIGHGUID_TYPE_PLAYER)
            {
                *data << uint32(receiverName.length() + 1);
                *data << receiverName;
            }
        }break;
    case CHAT_MSG_ACHIEVEMENT:
    case CHAT_MSG_GUILD_ACHIEVEMENT:
        *data << uint64(receiverGuid);
        break;
    default:
        {
            if (type == CHAT_MSG_CHANNEL)
            {
                ASSERT(senderName.length() > 0);
                *data << senderName;
            }
            guidPos = data->wpos();
            *data << uint64(receiverGuid);
        }break;
    }

    _FillMessageStructure(data, language, message, chatTag);
}

void ChatHandler::FillSystemMessageData(WorldPacket *data, const char *message)
{
    uint32 messageLength = (uint32)strlen((char*)message) + 1;
    data->Initialize(SMSG_MESSAGECHAT, 50+messageLength);
    *data << (uint8)CHAT_MSG_SYSTEM;
    *data << (uint32)LANG_UNIVERSAL;
    *data << (uint64)0;
    *data << (uint32)0;
    *data << (uint64)0;
    *data << messageLength;
    *data << message;
    *data << uint8(0);
}

void ChatHandler::SystemMessage(WorldSession *m_session, const char* message, ...)
{
    if( !message )
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    WorldPacket data;
    FillSystemMessageData(&data, msg1);
    if(m_session != NULL)
        m_session->SendPacket(&data);
}

void ChatHandler::ColorSystemMessage(WorldSession *m_session, const char* colorcode, const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", colorcode, msg1);
    WorldPacket data;
    FillSystemMessageData(&data, msg1);
    if(m_session != NULL)
        m_session->SendPacket(&data);
}

void ChatHandler::RedSystemMessage(WorldSession *m_session, const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024,message,ap);
    char msg[1024];
    snprintf(msg, 1024,"%s%s|r", MSG_COLOR_LIGHTRED/*MSG_COLOR_RED*/, msg1);
    WorldPacket data;
    FillSystemMessageData(&data, msg1);
    if(m_session != NULL)
        m_session->SendPacket(&data);
}

void ChatHandler::GreenSystemMessage(WorldSession *m_session, const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", MSG_COLOR_GREEN, msg1);
    WorldPacket data;
    FillSystemMessageData(&data, msg1);
    if(m_session != NULL)
        m_session->SendPacket(&data);
}

void ChatHandler::WhiteSystemMessage(WorldSession *m_session, const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", MSG_COLOR_WHITE, msg1);
    WorldPacket data;
    FillSystemMessageData(&data, msg1);
    if(m_session != NULL)
        m_session->SendPacket(&data);
}

void ChatHandler::BlueSystemMessage(WorldSession *m_session, const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    char msg[1024];
    snprintf(msg, 1024,"%s%s|r", MSG_COLOR_LIGHTBLUE, msg1);
    WorldPacket data;
    FillSystemMessageData(&data, msg1);
    if(m_session != NULL)
        m_session->SendPacket(&data);
}

void ChatHandler::RedSystemMessageToPlr(Player* plr, const char *message, ...)
{
    if( !message || !plr->GetSession() ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024,message,ap);
    RedSystemMessage(plr->GetSession(), (const char*)msg1);
}

void ChatHandler::GreenSystemMessageToPlr(Player* plr, const char *message, ...)
{
    if( !message || !plr->GetSession() ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024,message,ap);
    GreenSystemMessage(plr->GetSession(), (const char*)msg1);
}

void ChatHandler::BlueSystemMessageToPlr(Player* plr, const char *message, ...)
{
    if( !message || !plr->GetSession() ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024,message,ap);
    BlueSystemMessage(plr->GetSession(), (const char*)msg1);
}

void ChatHandler::SystemMessageToPlr(Player* plr, const char* message, ...)
{
    if( !message || !plr->GetSession() ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024,message,ap);
    SystemMessage(plr->GetSession(), msg1);
}
