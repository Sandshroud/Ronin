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

#ifndef __LOGON_COMM_SERVER_H
#define __LOGON_COMM_SERVER_H

#include <RC4Engine.h>
#include "WorldPacket.h"

class LogonCommServerSocket : public TcpSocket
{
    uint32 remaining;
    uint16 opcode;
    uint32 seed;
    RC4Engine _recv, _send;
public:
    bool authenticated;

    LogonCommServerSocket(SOCKET fd, const sockaddr_in * peer);
    ~LogonCommServerSocket();

    void OnRecvData();
    void OnDisconnect();
    void OnConnect();
    void SendDataPing();
    void SendPacket(WorldPacket * data);
    void HandlePacket(WorldPacket & recvData);

    void HandleRegister(WorldPacket & recvData);
    void HandlePong(WorldPacket & recvData);
    void HandleSessionRequest(WorldPacket & recvData);
    void HandleSQLExecute(WorldPacket & recvData);
    void HandleReloadAccounts(WorldPacket & recvData);
    void HandleAuthChallenge(WorldPacket & recvData);
    void HandleMappingReply(WorldPacket & recvData);
    void HandleUpdateMapping(WorldPacket & recvData);
    void HandleTestConsoleLogin(WorldPacket & recvData);
    void HandleDatabaseModify(WorldPacket& recvData);

    uint32 latency;
    uint32 last_ping;
    uint32 last_pong;
    bool removed;
    uint32 realmID;
};

typedef void (LogonCommServerSocket::*logonpacket_handler)(WorldPacket&);

#endif
