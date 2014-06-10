/***
 * Demonstrike Core
 */

// Class WorldSocket - Main network code functions, handles
// reading/writing of all packets.

#pragma once

#define WORLDSOCKET_SENDBUF_SIZE 131078
#define WORLDSOCKET_RECVBUF_SIZE 16384

class WorldPacket;
class SocketHandler;
class WorldSession;

enum OUTPACKET_RESULT
{
    OUTPACKET_RESULT_SUCCESS = 1,
    OUTPACKET_RESULT_NO_ROOM_IN_BUFFER = 2,
    OUTPACKET_RESULT_NOT_CONNECTED = 3,
    OUTPACKET_RESULT_SOCKET_ERROR = 4,
    OUTPACKET_RESULT_PACKET_ERROR = 5
};

class SERVER_DECL WorldSocket : public TcpSocket
{
public:
    WorldSocket(SOCKET fd, const sockaddr_in * peer);
    ~WorldSocket();

    // vs8 fix - send null on empty buffer
    HEARTHSTONE_INLINE void SendPacket(WorldPacket* packet, bool inWorld = false) { if(!packet) return; OutPacket(packet->GetOpcode(), packet->size(), (packet->size() ? (const void*)packet->contents() : NULL), inWorld); }

    void __fastcall OutPacket(uint16 opcode, size_t len, const void* data, bool InWorld = false);
    OUTPACKET_RESULT __fastcall _OutPacket(uint16 opcode, size_t len, const void* data, bool InWorld = false);

    HEARTHSTONE_INLINE uint32 GetLatency() { return _latency; }
    HEARTHSTONE_INLINE bool isAuthed() { return m_authed; }

    void Authenticate();
    void InformationRetreiveCallback(WorldPacket & recvData, uint32 requestid);

    void __fastcall SendAuthResponse(uint8 code, bool holdsPosition, uint32 position = 0);

    void OnRecvData();
    void OnConnect();
    void OnDisconnect();
    void UpdateQueuedPackets();

    HEARTHSTONE_INLINE void SetSession(WorldSession * session) { mSession = session; }
    HEARTHSTONE_INLINE WorldSession * GetSession() { return mSession; }

protected:
    void _HandleAuthSession(WorldPacket* recvPacket);
    void _HandlePing(WorldPacket* recvPacket);

private:
    uint32 mOpcode, mRemaining, mUnaltered;

    uint32 mSeed;
    uint32 mRequestID;
    uint8 hashDigest[20];
    uint32 mClientSeed;
    uint16 mClientBuild;
    WorldPacket *addonPacket;

    WorldSession *mSession;
    FastQueue<WorldPacket*, DummyLock> _queue;
    Mutex queueLock;

    WowCrypt _crypt;
    uint32 _latency;
    bool m_authed;
    bool mQueued;
    bool m_nagleEanbled;
    string * m_fullAccountName;
};
