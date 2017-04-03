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

void Player::SendWorldStateUpdate(uint32 WorldState, uint32 Value)
{
    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 15);
    data << uint32(WorldState) << uint32(Value);
    PushPacket(&data);
}

void Player::Gossip_SendPOI(float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, const char* Name)
{
    WorldPacket data(SMSG_GOSSIP_POI, 50);
    data << Flags << X << Y << Icon << Data;
    data << Name;
    PushPacket(&data);
}

void Player::SendLevelupInfo(uint32 level, uint32 Hp, uint32 Mana, uint32 *Stats)
{
    WorldPacket data(SMSG_LEVELUP_INFO, 48);
    data << uint32(level) << uint32(Hp) << uint32(Mana);
    data << uint32(0) << uint32(0) << uint32(0) << uint32(0);
    for(uint8 i = 0; i < 5; i++)
        data << uint32(Stats[i]);
    PushPacket(&data);
}

void Player::SendLogXPGain(WoWGuid victimGuid, uint32 totalXP, uint32 RestedXP)
{
    WorldPacket data(SMSG_LOG_XPGAIN, 50);
    data << victimGuid << uint32(totalXP) << uint8(victimGuid.empty() ? 0 : 1);
    if(!victimGuid.empty()) data << uint32(RestedXP) << float(1.f);
    data << uint8(0);
    PushPacket(&data);
}

// this one needs to be send inrange...
void Player::SendEnvironmentalDamageLog(const WoWGuid &guid, uint8 type, uint32 damage)
{
    WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, 20);
    data << guid << type << damage << uint32(0) << uint32(0);
    SendMessageToSet(&data, true);
}

void Player::SendCastResult(uint32 SpellId, uint8 ErrorMessage, uint8 MultiCast, uint32 Extra)
{
    WorldPacket data(SMSG_CAST_FAILED, 20);
    data << MultiCast << SpellId;
    data << ErrorMessage;
    if(Extra) data << Extra;
    PushPacket(&data);
}


