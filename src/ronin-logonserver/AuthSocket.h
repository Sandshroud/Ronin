/***
 * Demonstrike Core
 */

#ifndef AUTHSOCKET_H
#define AUTHSOCKET_H

#include "AccountCache.h"
#include "AuthStructs.h"

class LogonCommServerSocket;
struct Patch;
class PatchJob;

enum SocketState
{
    STATE_CLOSED = 0,
    STATE_NEED_CHALLENGE,
    STATE_NEED_PROOF,
    STATE_NEED_REPROOF,
    STATE_AUTHENTICATED,
    STATE_PATCHING
};

class AuthSocket : public TcpSocket
{
    friend class LogonCommServerSocket;
public:

    ///////////////////////////////////////////////////
    // Netcore shit
    //////////////////////////
    AuthSocket(SOCKET fd, const sockaddr_in * peer);
    ~AuthSocket();

    void OnRecvData();

    ///////////////////////////////////////////////////
    // Client Packet Handlers
    //////////////////////////

    bool HandleChallenge();
    bool HandleProof();
    bool HandleRealmlist();
    bool HandleReconnectChallenge();
    bool HandleReconnectProof();
    bool HandleTransferAccept();
    bool HandleTransferResume();
    bool HandleTransferCancel();
    bool HandleCMD19();

    ///////////////////////////////////////////////////
    // Server Packet Builders
    //////////////////////////

    void SendChallengeError(uint8 Error);
    void SendProofError(uint8 Error, uint8 * M2);
    RONIN_INLINE sAuthLogonChallenge_C * GetChallenge() { return &m_challenge; }
    RONIN_INLINE void Send(const uint8* data, const uint16 len) { SendPacket(data, len); };
    RONIN_INLINE void SendPacket(const uint8* data, const uint16 len)
    {
        LockWriteBuffer();
        Write(data, len);
        UnlockWriteBuffer();
    }

    void OnDisconnect();
    RONIN_INLINE time_t GetLastRecv() { return last_recv; }
    bool removedFromSet;
    RONIN_INLINE uint32 GetAccountID() { return m_account ? m_account->AccountId : 0; }
    RONIN_INLINE std::string GetAccountName() { return AccountName; }

    uint8 GetClientExpansion() { return m_challenge.version[0]; };
    uint8 GetClientPatch() { return m_challenge.version[1]; };
    uint8 GetClientVersion() { return m_challenge.version[2]; };
    uint16 GetBuild() { return m_challenge.build; };

protected:

    sAuthLogonChallenge_C m_challenge;
    Account * m_account;
    std::string AccountName;
    SocketState m_state;

    //////////////////////////////////////////////////
    // Authentication BigNumbers
    /////////////////////////
    BigNumber N, s, g, v;
    BigNumber b, B;
    BigNumber K;
    BigNumber rs;

    //////////////////////////////////////////////////
    // Session Key
    /////////////////////////

    BigNumber m_sessionkey;
    time_t last_recv;

    //////////////////////////////////////////////////////////////////////////
    // Patching stuff
    //////////////////////////////////////////////////////////////////////////
public:
    Patch * m_patch;
    PatchJob * m_patchJob;
};

#endif
