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

#pragma once

class ChatHandler;
class WorldSession;
class Player;
class Unit;

enum ChatMsg
{
    CHAT_MSG_ADDON                              = 0xFFFFFFFF,
    CHAT_MSG_SYSTEM                             = 0x00,
    CHAT_MSG_SAY                                = 0x01,
    CHAT_MSG_PARTY                              = 0x02,
    CHAT_MSG_RAID                               = 0x03,
    CHAT_MSG_GUILD                              = 0x04,
    CHAT_MSG_OFFICER                            = 0x05,
    CHAT_MSG_YELL                               = 0x06,
    CHAT_MSG_WHISPER                            = 0x07,
    CHAT_MSG_WHISPER_FOREIGN                    = 0x08,
    CHAT_MSG_WHISPER_INFORM                     = 0x09,
    CHAT_MSG_EMOTE                              = 0x0A,
    CHAT_MSG_TEXT_EMOTE                         = 0x0B,
    CHAT_MSG_MONSTER_SAY                        = 0x0C,
    CHAT_MSG_MONSTER_PARTY                      = 0x0D,
    CHAT_MSG_MONSTER_YELL                       = 0x0E,
    CHAT_MSG_MONSTER_WHISPER                    = 0x0F,
    CHAT_MSG_MONSTER_EMOTE                      = 0x10,
    CHAT_MSG_CHANNEL                            = 0x11,
    CHAT_MSG_CHANNEL_JOIN                       = 0x12,
    CHAT_MSG_CHANNEL_LEAVE                      = 0x13,
    CHAT_MSG_CHANNEL_LIST                       = 0x14,
    CHAT_MSG_CHANNEL_NOTICE                     = 0x15,
    CHAT_MSG_CHANNEL_NOTICE_USER                = 0x16,
    CHAT_MSG_AFK                                = 0x17,
    CHAT_MSG_DND                                = 0x18,
    CHAT_MSG_IGNORED                            = 0x19,
    CHAT_MSG_SKILL                              = 0x1A,
    CHAT_MSG_LOOT                               = 0x1B,
    CHAT_MSG_MONEY                              = 0x1C,
    CHAT_MSG_OPENING                            = 0x1D,
    CHAT_MSG_TRADESKILLS                        = 0x1E,
    CHAT_MSG_PET_INFO                           = 0x1F,
    CHAT_MSG_COMBAT_MISC_INFO                   = 0x20,
    CHAT_MSG_COMBAT_XP_GAIN                     = 0x21,
    CHAT_MSG_COMBAT_HONOR_GAIN                  = 0x22,
    CHAT_MSG_COMBAT_FACTION_CHANGE              = 0x23,
    CHAT_MSG_BG_SYSTEM_NEUTRAL                  = 0x24,
    CHAT_MSG_BG_SYSTEM_ALLIANCE                 = 0x25,
    CHAT_MSG_BG_SYSTEM_HORDE                    = 0x26,
    CHAT_MSG_RAID_LEADER                        = 0x27,
    CHAT_MSG_RAID_WARNING                       = 0x28,
    CHAT_MSG_RAID_BOSS_EMOTE                    = 0x29,
    CHAT_MSG_RAID_BOSS_WHISPER                  = 0x2A,
    CHAT_MSG_FILTERED                           = 0x2B,
    CHAT_MSG_BATTLEGROUND                       = 0x2C,
    CHAT_MSG_BATTLEGROUND_LEADER                = 0x2D,
    CHAT_MSG_RESTRICTED                         = 0x2E,
    CHAT_MSG_BATTLENET                          = 0x2F,
    CHAT_MSG_ACHIEVEMENT                        = 0x30,
    CHAT_MSG_GUILD_ACHIEVEMENT                  = 0x31,
    CHAT_MSG_ARENA_POINTS                       = 0x32,
    CHAT_MSG_PARTY_LEADER                       = 0x33,
    CHAT_MSG_TARGETICONS                        = 0x34,
    CHAT_MSG_BN_WHISPER                         = 0x35,
    CHAT_MSG_BN_WHISPER_INFORM                  = 0x36,
    CHAT_MSG_BN_CONVERSATION                    = 0x37,
    CHAT_MSG_BN_CONVERSATION_NOTICE             = 0x38,
    CHAT_MSG_BN_CONVERSATION_LIST               = 0x39,
    CHAT_MSG_BN_INLINE_TOAST_ALERT              = 0x3A,
    CHAT_MSG_BN_INLINE_TOAST_BROADCAST          = 0x3B,
    CHAT_MSG_BN_INLINE_TOAST_BROADCAST_INFORM   = 0x3C,
    CHAT_MSG_BN_INLINE_TOAST_CONVERSATION       = 0x3D,
    CHAT_MSG_BN_WHISPER_PLAYER_OFFLINE          = 0x3E,
    CHAT_MSG_COMBAT_GUILD_XP_GAIN               = 0x3F,
    CHAT_MSG_CURRENCY                           = 0x40
};

enum Languages
{
    LANG_ADDON          = -1,
    LANG_UNIVERSAL      = 0x00,
    LANG_ORCISH         = 0x01,
    LANG_DARNASSIAN     = 0x02,
    LANG_TAURAHE        = 0x03,
    LANG_DWARVISH       = 0x06,
    LANG_COMMON         = 0x07,
    LANG_DEMONIC        = 0x08,
    LANG_TITAN          = 0x09,
    LANG_THALASSIAN     = 0x0A,
    LANG_DRACONIC       = 0x0B,
    LANG_KALIMAG        = 0x0C,
    LANG_GNOMISH        = 0x0D,
    LANG_TROLL          = 0x0E,
    LANG_GUTTERSPEAK    = 0x21,
    LANG_DRAENEI        = 0x23,
    LANG_ZOMBIE         = 0x24,
    LANG_GNOMISHBINARY  = 0x25,
    LANG_GOBLINBINARY   = 0x26,
    LANG_WORGEN         = 0x27,
    LANG_GOBLIN         = 0x28,
    NUM_LANGUAGES       = 0x29
};

/*#define MSG_COLOR_YELLOW      "|r"
#define MSG_COLOR_RED           "|cffff2020"
#define MSG_COLOR_GREEN         "|c1f40af20"
#define MSG_COLOR_LIGHTRED      "|cffff6060"*/

#define MSG_COLOR_LIGHTRED      "|cffff6060"
#define MSG_COLOR_LIGHTBLUE     "|cff00ccff"
#define MSG_COLOR_BLUE          "|cff0000ff"
#define MSG_COLOR_GREEN         "|cff00ff00"
#define MSG_COLOR_RED           "|cffff0000"
#define MSG_COLOR_CRIMSON       "|cffdc143c"
#define MSG_COLOR_GOLD          "|cffffcc00"
#define MSG_COLOR_GREY          "|cff888888"
#define MSG_COLOR_WHITE         "|cffffffff"
#define MSG_COLOR_SUBWHITE      "|cffbbbbbb"
#define MSG_COLOR_MAGENTA       "|cffff00ff"
#define MSG_COLOR_YELLOW        "|cffffff00"
#define MSG_COLOR_CYAN          "|cff00ffff"
#define MSG_COLOR_TORQUISEBLUE  "|cff00C78C"
#define MSG_COLOR_GREENYELLOW   "|cffADFF2F"
#define MSG_COLOR_PURPLE        "|cffDA70D6"
#define MSG_COLOR_ORANGEY       "|cffFF4500"
#define MSG_COLOR_CHOCOLATE     "|cffCD661D"
#define MSG_COLOR_IVORY         "|cff8B8B83"
#define MSG_COLOR_LIGHTYELLOW   "|cffFFFFE0"
#define MSG_COLOR_SEXGREEN      "|cff71C671"
#define MSG_COLOR_SEXTEAL       "|cff388E8E"
#define MSG_COLOR_SEXPINK       "|cffC67171"
#define MSG_COLOR_SEXBLUE       "|cff00E5EE"
#define MSG_COLOR_SEXHOTPINK    "|cffFF6EB4"

#define CHECKSESSION if(m_session == NULL) return NULL; \
    if(m_session->GetPlayer() == NULL) return NULL;

class SERVER_DECL ChatHandler : public Singleton<ChatHandler>
{
public:
    ChatHandler();
    ~ChatHandler();

protected:
    size_t _FillBasicStructure(WorldPacket *data, bool gmMessage, uint8 type, int32 lang, WoWGuid sender, uint32 unk = 0);

    size_t _FillMonsterMessageStructure(WorldPacket *data, std::string senderName, WoWGuid targetGuid, std::string receiverName);
    void _FillMessageStructure(WorldPacket *data, int32 lang, std::string message, uint8 chatTag);

public:
    void FillPlayerMessage(WorldPacket *data, WoWGuid from, uint8 type, int32 language, std::string message, uint8 chatTag = 0, const char *header = NULL, bool gmMessage = false);
    void FillMessageData(WorldPacket *data, bool gmMessage, uint8 type, int32 language, WoWGuid senderGuid = 0, WoWGuid receiverGuid = 0, std::string senderName = "", std::string message = "", std::string receiverName = "", uint8 chatTag = 0);
    void FillBroadcastMessage(WorldPacket *data, size_t &guidPos, size_t &langPos, bool gmMessage, uint8 type, int32 language, WoWGuid senderGuid = 0, WoWGuid receiverGuid = 0, std::string senderName = "", std::string message = "", std::string receiverName = "", uint8 chatTag = 0);
    static void FillSystemMessageData(WorldPacket *data, const char* message );

    int ParseCommands(uint32 type, const char* text, Player *plr);

    static void SystemMessage(WorldSession *m_session, const char *message, ...);
    void ColorSystemMessage(WorldSession *m_session, const char *colorcode, const char *message, ...);
    void RedSystemMessage(WorldSession *m_session, const char *message, ...);
    void GreenSystemMessage(WorldSession *m_session, const char *message, ...);
    void WhiteSystemMessage(WorldSession *m_session, const char *message, ...);
    void BlueSystemMessage(WorldSession *m_session, const char *message, ...);
    void RedSystemMessageToPlr(Player* plr, const char *message, ...);
    void GreenSystemMessageToPlr(Player* plr, const char *message, ...);
    void BlueSystemMessageToPlr(Player* plr, const char *message, ...);
    void SystemMessageToPlr(Player* plr, const char *message, ...);

    static bool hasStringAbbr(const char* s1, const char* s2);

    void SendMultilineMessage(WorldSession *m_session, const char *str);

private:
    Mutex m_lock;
};

#define sChatHandler ChatHandler::getSingleton()
