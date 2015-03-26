/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void Player::SendWorldStateUpdate(uint32 WorldState, uint32 Value)
{
    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 15);
    data << uint32(WorldState) << uint32(Value);
    GetSession()->SendPacket(&data);
}

void Player::Gossip_SendPOI(float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, const char* Name)
{
    WorldPacket data(SMSG_GOSSIP_POI, 50);
    data << Flags << X << Y << Icon << Data;
    data << Name;
    GetSession()->SendPacket(&data);
}

void Player::SendLevelupInfo(uint32 level, uint32 Hp, uint32 Mana, uint32 *Stats)
{
    WorldPacket data(SMSG_LEVELUP_INFO, 48);
    data << uint32(level) << uint32(Hp) << uint32(Mana);
    data << uint32(0) << uint32(0) << uint32(0) << uint32(0);
    for(uint8 i = 0; i < 5; i++)
        data << uint32(Stats[i]);
    GetSession()->SendPacket(&data);
}

void Player::SendLogXPGain(uint64 guid, uint32 NormalXP, uint32 RestedXP, bool type)
{
    WorldPacket data(SMSG_LOG_XPGAIN, 50);
    data << guid << NormalXP << uint8(type);
    if(type) data << uint32(RestedXP) << float(1.f);
    data << uint8(0);
    GetSession()->SendPacket(&data);
}

// this one needs to be send inrange...
void Player::SendEnvironmentalDamageLog(const uint64 & guid, uint8 type, uint32 damage)
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
    GetSession()->SendPacket(&data);
}


