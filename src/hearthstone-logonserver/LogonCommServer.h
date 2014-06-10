/***
 * Demonstrike Core
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
