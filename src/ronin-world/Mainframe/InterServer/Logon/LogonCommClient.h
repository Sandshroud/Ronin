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

class LogonCommClientSocket : public TcpSocket
{
    uint32 remaining;
    uint16 opcode;
    RC4Engine _recv, _send;
public:
    LogonCommClientSocket(SOCKET fd, const sockaddr_in * peer);
    ~LogonCommClientSocket();

    void OnRecvData();
    bool SendPacket(WorldPacket * data);
    void HandlePacket(WorldPacket & recvData);
    void SendChallenge(std::string challenger);
    void HandleAuthResponse(WorldPacket & recvData);

    void HandleRegister(WorldPacket & recvData);
    void HandlePing(WorldPacket & recvData);
    void HandleLatency(WorldPacket & recvData);
    void HandleSessionInfo(WorldPacket & recvData);
    void HandleRequestAccountMapping(WorldPacket & recvData);
    void UpdateAccountCount(uint32 account_id, int8 add);
    void HandleDisconnectAccount(WorldPacket & recvData);
    void HandleConsoleAuthResult(WorldPacket & recvData);

    void OnDisconnect();
    void CompressAndSend(ByteBuffer & uncompressed);

    uint32 _id;
    uint32 latency;
    uint32 last_ping;
    bool authenticated, rejected;

    uint32 realmID;
    std::string realmName;
};

typedef void (LogonCommClientSocket::*logonpacket_handler)(WorldPacket&);
