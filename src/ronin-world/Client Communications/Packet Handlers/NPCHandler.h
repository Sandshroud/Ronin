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

enum GOSSIP_SPECIAL
{
    GOSSIP_NO_SPECIAL          = 0x00,
    GOSSIP_POI                = 0x01,
    GOSSIP_SPIRIT_HEALER_ACTIVE = 0x02,
    GOSSIP_VENDOR              = 0x03,
    GOSSIP_TRAINER            = 0x04,
    GOSSIP_TABARD_VENDOR        = 0x05,
    GOSSIP_INNKEEPER            = 0x06,
    GOSSIP_PETITIONER          = 0x07,
    GOSSIP_TAXI              = 0x08,
    GOSSIP_TEXT              = 0x09,
    GOSSIP_MENU              = 0x0A,
    GOSSIP_BATTLEMASTER      = 0x0B,
    GOSSIP_BANKEER            = 0x0C,
};

#pragma pack(PRAGMA_PACK)
struct GossipText_Text
{
    char * Text[2];
};

struct GossipText_Info
{
    float Prob;
    uint32 Lang;
    uint32 Delay[3];
    uint32 Emote[3];
};

struct GossipText
{
    uint32 ID;
    GossipText_Text Texts[8];
    GossipText_Info Infos[8];
};
#pragma pack(PRAGMA_POP)

#define TRAINER_STATUS_LEARNABLE        0
#define TRAINER_STATUS_NOT_LEARNABLE    1
#define TRAINER_STATUS_ALREADY_HAVE     2
#define TRAINER_STATUS_NOT_AVAILABLE    3
