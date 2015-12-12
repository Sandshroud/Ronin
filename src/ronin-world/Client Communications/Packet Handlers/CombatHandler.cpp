/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleAttackSwingOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    recv_data >> guid;
    if(guid.empty() || _player->bGMTagOn)
    {
        HandleAttackStopOpcode(recv_data);
        return;
    }

    // AttackSwing
    sLog.Debug( "WORLD"," Recvd CMSG_ATTACKSWING Message" );
    if(GetPlayer()->IsPacified() || GetPlayer()->IsStunned() || GetPlayer()->IsFeared())
        return;

    Unit* pEnemy = _player->GetMapMgr()->GetUnit(guid);
    if(!pEnemy)
    {
        sLog.outDebug("WORLD: "I64FMT" does not exist.", guid);
        return;
    }

    if(pEnemy->isDead() || _player->isDead())       // haxors :(
    {
        HandleAttackStopOpcode(recv_data);
        return;
    }

    // Implemented Hackfix for quest 1640
    if(!(pEnemy->GetEntry() == 6090 && _player->HasQuest(1640)) && sFactionSystem.AC_GetAttackableStatus(GetPlayer(), pEnemy))
    {
        sWorld.LogCheater(this, "Faction exploit detected. Damagetype: Melee.");
        GetPlayer()->BroadcastMessage("Faction exploit detected. You will be disconnected in 5 seconds.");
        GetPlayer()->Kick(5000);
        return;
    }

    _player->EventAttackStart(guid);
}

void WorldSession::HandleAttackStopOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    GetPlayer()->EventAttackStop();
}

