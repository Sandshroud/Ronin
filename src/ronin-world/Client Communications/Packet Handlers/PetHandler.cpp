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

void WorldSession::HandlePetAction(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 petGuid = 0;
    uint16 misc = 0;
    uint16 action = 0;
    uint64 targetguid = 0;
    recv_data >> petGuid;
    recv_data >> misc;
    recv_data >> action;

}

void WorldSession::HandlePetInfo(WorldPacket & recv_data)
{
    //nothing
    sLog.Debug("WorldSession","HandlePetInfo is called");
}

void WorldSession::HandlePetNameQuery(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint32 reqNumber = 0;
    uint64 petGuid = 0;

    recv_data >> reqNumber >> petGuid;
}

void WorldSession::HandleStablePet(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
}

void WorldSession::HandleUnstablePet(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint64 npcguid = 0;
    uint32 petnumber = 0;

    recv_data >> npcguid >> petnumber;
}

void WorldSession::HandleStableSwapPet(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint64 npcguid = 0;
    uint32 petnumber = 0;
    recv_data >> npcguid >> petnumber;
}

void WorldSession::HandleStabledPetList(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 npcguid;
    recv_data >> npcguid;

}

void WorldSession::HandleBuyStableSlot(WorldPacket &recv_data)
{

}

void WorldSession::HandlePetSetActionOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 guid; uint32 slot;
    uint16 spell, state;
    
    recv_data >> guid >> slot >> spell >> state;
}

void WorldSession::HandlePetRename(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    std::string name;
    recv_data >> guid >> name;
}

void WorldSession::HandlePetAbandon(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
}

void WorldSession::HandlePetUnlearn(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint64 guid;
    recv_data >> guid;
}

void WorldSession::HandleTotemDestroyed(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint8 slot;
    recv_data >> slot;
}

void WorldSession::HandlePetLearnTalent( WorldPacket & recvPacket )
{
    CHECK_INWORLD_RETURN();

    uint64 guid = 0;
    uint32 talentid, rank = 0;

    recvPacket >> guid >> talentid >> rank;

}

void WorldSession::HandleCancelPetAura(WorldPacket& recvPacket)
{
    uint64 guid;
    uint32 spellId;

    recvPacket >> guid;
    recvPacket >> spellId;

}