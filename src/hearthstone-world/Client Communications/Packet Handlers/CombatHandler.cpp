/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleAttackSwingOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 8);
    uint64 guid;
    recv_data >> guid;

    if(!guid)
    {
        // does this mean cancel combat?
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

    // Faction "Hack" by Deathshit
    // Implemented Hackfix for quest 1640
    if(sWorld.antihack_cheatengine)
    {
        if(!HasGMPermissions() || !sWorld.no_antihack_on_gm)
        {
            if(pEnemy->GetEntry() == 6090 && _player->HasQuest(1640))
            {
                GetPlayer()->smsg_AttackStart(pEnemy);
                GetPlayer()->EventAttackStart();
                return;
            }

            if(sFactionSystem.AC_GetAttackableStatus( GetPlayer(), pEnemy ))
            {
                sWorld.LogCheater(this, "Faction exploit detected. Damagetype: Melee.");
                GetPlayer()->BroadcastMessage("Faction exploit detected. You will be disconnected in 5 seconds.");
                GetPlayer()->Kick(5000);
                return;
            }
        }
    }

    _player->smsg_AttackStart(pEnemy);
    _player->EventAttackStart();
}

void WorldSession::HandleAttackStopOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid = GetPlayer()->GetSelection();
    Unit* pEnemy = NULLUNIT;

    if(guid)
    {
        pEnemy = _player->GetMapMgr()->GetUnit(guid);
        if(pEnemy)
        {
            GetPlayer()->EventAttackStop();
            GetPlayer()->smsg_AttackStop(pEnemy);
        }
    }
}

