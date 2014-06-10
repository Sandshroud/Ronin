/***
 * Demonstrike Core
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
    string realmName;
};

typedef void (LogonCommClientSocket::*logonpacket_handler)(WorldPacket&);
