/***
 * Demonstrike Core
 */
// Class WorldSocket - Main network code functions, handles
// reading/writing of all packets.

#include "StdAfx.h"

/* echo send/received packets to console */
//#define ECHO_PACKET_LOG_TO_CONSOLE 1

extern bool bServerShutdown;

#pragma pack(push, 1)

struct ClientPktHeader
{
    uint16 size;
    uint32 cmd;
};

struct ServerPktHeader
{
    ServerPktHeader(uint32 _size, uint16 _cmd) : size(_size), headerLength(0)
    {
        if (size > 0x7FFF)
            header[headerLength++] = 0x80 | (0xFF & (_size >> 16));
        header[headerLength++] = 0xFF & (_size >> 8);
        header[headerLength++] = 0xFF & _size;
        header[headerLength++] = 0xFF & _cmd;
        header[headerLength++] = 0xFF & (_cmd >> 8);
    }

    uint8 getHeaderLength() { return headerLength; }
    const uint32 size;
    uint8 header[5];
    uint8 headerLength;
};

#pragma pack(PRAGMA_POP)

WorldSocket::WorldSocket(SOCKET fd, const sockaddr_in * peer) : TcpSocket(fd, WORLDSOCKET_RECVBUF_SIZE, WORLDSOCKET_SENDBUF_SIZE, false, peer)
{
    m_authed = false;
    mOpcode = mRemaining = mUnaltered = 0;
    _latency = 0;
    mSession = NULL;
    mSeed = RandomUInt();
    addonPacket = NULL;
    mQueued = false;
    mRequestID = 0;
    m_nagleEanbled = false;
    m_fullAccountName = NULL;
}

WorldSocket::~WorldSocket()
{
    WorldPacket * pck;
    queueLock.Acquire();
    while((pck = _queue.Pop()))
        delete pck;
    queueLock.Release();

    if(addonPacket)
        delete addonPacket;
    addonPacket = NULL;

    if(mSession)
    {
        mSession->SetSocket(NULL);
        mSession=NULL;
    }

    if( m_fullAccountName != NULL )
    {
        delete m_fullAccountName;
        m_fullAccountName = NULL;
    }
}

void WorldSocket::OnDisconnect()
{
    if(mSession)
    {
        mSession->SetSocket(0);
        mSession = NULL;
    }

    if(mRequestID != 0)
    {
        sLogonCommHandler.UnauthedSocketClose(mRequestID);
        mRequestID = 0;
    }

    if(mQueued)
    {
        sWorld.RemoveQueuedSocket(this);    // Remove from queued sockets.
        mQueued = false;
    }

    // clear buffer
    queueLock.Acquire();
    WorldPacket *pck;
    while((pck = _queue.Pop()))
        delete pck;
    queueLock.Release();
}

void WorldSocket::UpdateQueuedPackets()
{
    queueLock.Acquire();
    if(!_queue.HasItems())
    {
        queueLock.Release();
        return;
    }

    WorldPacket * pck;
    while((pck = _queue.front()))
    {
        /* try to push out as many as you can */
        switch(_OutPacket(pck->GetOpcode(), pck->size(), pck->size() ? pck->contents() : NULL))
        {
        case OUTPACKET_RESULT_SUCCESS:
            {
                delete pck;
                _queue.pop_front();
            }break;

        case OUTPACKET_RESULT_NO_ROOM_IN_BUFFER:
            {
                /* still connected */
                queueLock.Release();
                return;
            }break;

        default:
            {
                /* kill everything in the buffer */
                while((pck = _queue.Pop()))
                    delete pck;
                queueLock.Release();
                return;
            }break;
        }
    }
    queueLock.Release();
}

void WorldSocket::OutPacket(uint16 opcode, size_t len, const void* data, bool compressed)
{
    OUTPACKET_RESULT res;
    if( (len + 10) > WORLDSOCKET_SENDBUF_SIZE )
    {
        printf("WARNING: Tried to send a packet of %u bytes (which is too large) to a socket. Opcode was: %u (0x%04X)\n", uint(len), uint(opcode), uint(opcode));
        return;
    }

    res = _OutPacket(opcode, len, data, compressed);
    if(res == OUTPACKET_RESULT_SUCCESS)
        return;
    if(res == OUTPACKET_RESULT_PACKET_ERROR)
    { // Track packets that cause packet errors
        if(sLog.GetLogLevel() < 4)
            return;

        FILE *codeLog = NULL;
        fopen_s(&codeLog, "rejectedOpcodes.txt", "a+b");
        if(codeLog)
        {
            fprintf(codeLog, "Rejecting unset packet %u with size %u\r\n", opcode, len);
            WorldPacket packet(opcode, len);
            packet.append((uint8*)data, len);
            packet.hexlike(codeLog);
            fclose(codeLog);
        }
    }
    else if(res == OUTPACKET_RESULT_NO_ROOM_IN_BUFFER)
    {
        if(compressed)
            Disconnect();
        else
        {
            /* queue the packet */
            queueLock.Acquire();
            WorldPacket *pck = new WorldPacket(opcode, len);
            pck->SetOpcode(opcode);
            if(len) pck->append((const uint8*)data, len);
            _queue.Push(pck);
            queueLock.Release();
        }
    }
}

OUTPACKET_RESULT WorldSocket::_OutPacket(uint16 opcode, size_t len, const void* data, bool compressed)
{
    bool rv;
    if(!IsConnected())
        return OUTPACKET_RESULT_NOT_CONNECTED;
    if(GetWriteBuffer()->GetSpace() < (len+5))
        return OUTPACKET_RESULT_NO_ROOM_IN_BUFFER;
    uint16 newOpcode = sOpcodeMgr.ConvertOpcodeForOutput(opcode&0x7FFF);
    if(newOpcode == MSG_NULL_ACTION)
        return OUTPACKET_RESULT_PACKET_ERROR;
    if(compressed) newOpcode |= OPCODE_COMPRESSION_MASK;
    LockWriteBuffer();

    //printf("Sending opcode %s%s (0x%.4X)\n", compressed ? "COMPRESSED_" : "", sOpcodeMgr.GetOpcodeName(opcode), opcode);
    // Encrypt the packet
    // First, create the header.
    ServerPktHeader Header(len + 2, newOpcode);
    _crypt.EncryptSend(((uint8*)Header.header), Header.getHeaderLength());

    // Pass the header to our send buffer
    rv = WriteButHold(((const uint8*)Header.header), Header.getHeaderLength());

    // Pass the rest of the packet to our send buffer (if there is any)
    if(len > 0 && rv)
        rv = Write((const uint8*)data, (uint32)len);
    else if(rv) rv = ForceSend();
    UnlockWriteBuffer();
    return rv ? OUTPACKET_RESULT_SUCCESS : OUTPACKET_RESULT_SOCKET_ERROR;
}

void WorldSocket::OnConnect()
{
    sWorld.mAcceptedConnections++;
    _latency = getMSTime();

    WorldPacket data (MSG_VERIFY_CONNECTIVITY, 37);
    data << "RLD OF WARCRAFT CONNECTION - SERVER TO CLIENT";
    SendPacket(&data);
}

void WorldSocket::_HandleAuthSession(WorldPacket* recvPacket)
{
    uint32 addonSize;
    std::string account;
    _latency = getMSTime() - _latency;

    try
    {
        recvPacket->read_skip<uint32>();
        recvPacket->read_skip<uint32>();
        recvPacket->read_skip<uint8>();
        *recvPacket >> hashDigest[10] >> hashDigest[18] >> hashDigest[12] >> hashDigest[5];
        recvPacket->read_skip<uint64>();
        *recvPacket >> hashDigest[15] >> hashDigest[9] >> hashDigest[19] >> hashDigest[4];
        *recvPacket >> hashDigest[7] >> hashDigest[16] >> hashDigest[3];
        *recvPacket >> mClientBuild >> hashDigest[8];
        recvPacket->read_skip<uint32>();
        recvPacket->read_skip<uint8>();
        *recvPacket >> hashDigest[17] >> hashDigest[6] >> hashDigest[0];
        *recvPacket >> hashDigest[1] >> hashDigest[11];
        *recvPacket >> mClientSeed >> hashDigest[2];
        recvPacket->read_skip<uint32>();
        *recvPacket >> hashDigest[14] >> hashDigest[13];

        *recvPacket >> addonSize;
        if(addonSize)
        {
            uint8 *addonBytes = new uint8[addonSize];
            recvPacket->read(addonBytes, addonSize);
            addonPacket = new WorldPacket(CMSG_AUTH_SESSION, addonSize);
            addonPacket->append(addonBytes, addonSize);
            delete addonBytes;
        }

        isBattleNetAccount = recvPacket->ReadBit();
        uint16 accountLen = recvPacket->ReadBits(12);
        account = recvPacket->ReadString(accountLen);
    }
    catch(ByteBufferException &)
    {
        sLog.outDebug("Incomplete copy of AUTH_SESSION Received.");
        return;
    }

    if(mClientBuild != CL_BUILD_SUPPORT)
    {
        SendAuthResponse(AUTH_VERSION_MISMATCH, false);
        return;
    }

    // Send out a request for this account.
    mRequestID = sLogonCommHandler.ClientConnected(account, this);
    if(mRequestID == 0xFFFFFFFF)
    {
        Disconnect();
        return;
    }

    // shitty hash !
    m_fullAccountName = new std::string( account );
}

void WorldSocket::InformationRetreiveCallback(WorldPacket & recvData, uint32 requestid)
{
    uint32 error;
    recvData >> error;
    if(error != 0 || requestid != mRequestID)
    {
        // something happened wrong @ the logon server
        SendAuthResponse(AUTH_UNKNOWN_ACCOUNT, false);
        return;
    }

    // Extract account information from the packet.
    std::string AccountName, GMFlags, lang = "enUS";
    uint32 AccountID;
    uint8 AccountFlags;

    recvData >> AccountID >> AccountName >> GMFlags >> AccountFlags;
    if( const std::string *ForcedPermissions = sLogonCommHandler.GetForcedPermissions(AccountName))
        GMFlags.assign(ForcedPermissions->c_str());

    sLog.Debug( "WorldSocket","Received information packet from logon: `%s` ID %u (request %u)", AccountName.c_str(), AccountID, mRequestID);

    mRequestID = 0;
    uint8 K[40];
    recvData.read(K, 40);
    _crypt.Init(K);

    recvData.read((uint8*)lang.data(), 4);

    //checking if player is already connected
    //disconnect current player and login this one(blizzlike)
    WorldSession *session = sWorld.FindSession( AccountID );
    if( session != NULL )
    {
        if(session->IsHighPriority())
        {
            // Fail authentification until the player is finally added to world
            SendAuthResponse(AUTH_ALREADY_LOGGING_IN, false);
            return;
        }
        else if(session->GetPlayer())
        {
            // Disconnect the target player from the session
            session->Disconnect();

            // clear the logout timer so he times out straight away
            session->LogoutPlayer(false);
        }
        else 
        {
            // Disconnect the target player from the session
            session->Disconnect();

            if(session->GetInstance() == 0)
                sWorld.DeleteGlobalSession(session);
        }
    }

    Sha1Hash sha;

    uint32 t = 0;
    if( m_fullAccountName == NULL )             // should never happen !
        sha.UpdateData(AccountName);
    else
    {
        sha.UpdateData(*m_fullAccountName);

        // this is unused now. we may as well free up the memory.
        delete m_fullAccountName;
        m_fullAccountName = NULL;
    }

    sha.UpdateData((uint8 *)&t, 4);
    sha.UpdateData((uint8 *)&mClientSeed, 4);
    sha.UpdateData((uint8 *)&mSeed, 4);
    sha.UpdateData((uint8 *)&K, 40);
    sha.Finalize();

    if (memcmp(sha.GetDigest(), hashDigest, 20))
    {
        SendAuthResponse(AUTH_UNKNOWN_ACCOUNT, false);
        return;
    }

    m_authed = true;

    // Allocate session
    WorldSession *pSession = new WorldSession(AccountID, AccountName, this);
    ASSERT(pSession);
    // Set session properties
    pSession->permissioncount = 0;//just to make sure it's 0
    pSession->SetClientBuild(mClientBuild);
    pSession->LoadSecurity(GMFlags);
    pSession->SetAccountFlags(AccountFlags);
    pSession->m_lastPing = UNIXTIME;
    recvData >> pSession->m_muted;
    if(!pSession->InitializeZLibCompression())
    {
        delete pSession;
        SendAuthResponse(AUTH_SYSTEM_ERROR, false);
        return;
    }

    mSession = pSession;
    pSession->deleteMutex.Acquire();

    pSession->LoadTutorials();
    pSession->LoadAccountData();

    sLog.Debug("Auth", "%s from %s:%u [%ums]", AccountName.c_str(), GetIP(), GetPort(), _latency);

    // Check for queue.
    if( (sWorld.GetSessionCount() < sWorld.GetPlayerLimit()) || pSession->HasGMPermissions() )
        Authenticate();
    else
    {
        // Queued, sucker.
        uint32 Position = sWorld.AddQueuedSocket(this);
        mQueued = true;
        sLog.Debug("Queue", "%s added to queue in position %u", AccountName.c_str(), Position);

        // Send packet so we know what we're doing
        SendAuthResponse(AUTH_WAIT_QUEUE, true, Position);
    }

    pSession->deleteMutex.Release();
}

void WorldSocket::Authenticate()
{
    mQueued = false;
    WorldSession *pSession = mSession;
    if(!pSession)
    {
        sLog.Debug( "WorldSocket","Lost Session");
        return;
    }

    SendAuthResponse(AUTH_OK, false);
    SendAddonPacket(pSession);

    WorldPacket data(SMSG_CLIENTCACHE_VERSION, 4);
    data << uint32(CL_BUILD_SUPPORT);
    SendPacket(&data);

    data.Initialize(SMSG_HOTFIX_NOTIFY, 12); // Blop or not, client will accept the info
    if(size_t count = sItemMgr.HotfixOverridesSize())
    {
        data << uint32(count); // count
        data << uint32(true ? 0x919BE54E : 0x50238EC2); // This can be either, the client will ask for both if no current db2 info is found
        for(std::map<uint32, uint8>::iterator itr = sItemMgr.HotfixOverridesBegin(); itr != sItemMgr.HotfixOverridesEnd(); itr++)
        {
            data << uint32(((itr->second & 0x02) ? 0x50238EC2 : 0x919BE54E));
            data << uint32(sWorld.GetStartTime());
            data << itr->first;
        }
    } else data << uint32(0);
    SendPacket(&data);

    data.Initialize(SMSG_TUTORIAL_FLAGS, 4 * 8);
    for (uint32 i = 0; i < 8; ++i)
        data << uint32(mSession->GetTutorialFlag(i));
    SendPacket(&data);

    pSession->_latency = _latency;
    sWorld.AddSession(pSession);
    sWorld.AddGlobalSession(pSession);
}

void WorldSocket::SendAuthResponse(uint8 code, bool holdsPosition, uint32 position)
{
    uint8 expansion = mSession ? mSession->GetHighestExpansion() : 0;
    WorldPacket data(SMSG_AUTH_RESPONSE, 15);
    if(holdsPosition) data.WriteBitString(3, 1, 0, 1);
    else data.WriteBitString(2, 0, 1);
    data << uint32(0) << uint8(expansion) << uint32(0) << uint8(expansion);
    data << uint32(0) << uint8(0) << uint8(code);
    if(holdsPosition) data << uint32(position);
    SendPacket(&data);
}

void WorldSocket::SendAddonPacket(WorldSession *pSession)
{
    if(addonPacket == NULL)
        return;

    sAddonMgr.SendAddonInfoPacket(addonPacket, pSession);
    delete addonPacket;
    addonPacket = NULL;
}

void WorldSocket::_HandlePing(WorldPacket* recvPacket)
{
    uint32 ping;
    *recvPacket >> _latency;
    *recvPacket >> ping;

    if(mSession)
    {
        mSession->_latency = _latency;
        mSession->m_lastPing = UNIXTIME;

        // reset the move time diff calculator, don't worry it will be re-calculated next movement packet.
        mSession->m_clientTimeDelay = 0;
    }

    OutPacket(SMSG_PONG, 4, &ping);

#ifdef WIN32
    // Dynamically change nagle buffering status based on latency.
    if(_latency >= 250)
    {
        if(!m_nagleEanbled)
        {
            u_long arg = 0;
            setsockopt(GetFd(), 0x6, 0x1, (const char*)&arg, sizeof(arg));
            m_nagleEanbled = true;
        }
    }
    else
    {
        if(m_nagleEanbled)
        {
            u_long arg = 1;
            setsockopt(GetFd(), 0x6, 0x1, (const char*)&arg, sizeof(arg));
            m_nagleEanbled = false;
        }
    }
#endif
}

void WorldSocket::OnRecvData()
{
    for(;;)
    {
        // Check for the header if we don't have any bytes to wait for.
        if(mRemaining == 0)
        {
            if(GetReadBuffer()->GetSize() < 6)
            {
                // No header in the packet, let's wait.
                return;
            }

            // Copy from packet buffer into header local var
            ClientPktHeader Header;
            Read(&Header, 6);

            // Decrypt the header
            _crypt.DecryptRecv((uint8*)&Header, sizeof (ClientPktHeader));
            mRemaining = ntohs(Header.size) - 4;
            mUnaltered = Header.cmd;
            mOpcode = sOpcodeMgr.ConvertOpcodeForInput(mUnaltered);
        }

        if(mRemaining > 0)
        {
            if( GetReadBuffer()->GetSize() < mRemaining )
            {
                // We have a fragmented packet. Wait for the complete one before proceeding.
                return;
            }
        }

        WorldPacket *Packet = new WorldPacket(mOpcode, mRemaining);
        if(mRemaining > 0)
        {
            Packet->resize(mRemaining);
            Read((uint8*)Packet->contents(), mRemaining);
        }
        mRemaining = mOpcode = 0;

        // Check for packets that we handle
        switch(Packet->GetOpcode())
        {
        case CMSG_PING:
            {
                _HandlePing(Packet);
            }break;
        case MSG_VERIFY_CONNECTIVITY_RESPONSE:
            {
                if(strcmp(((char*)Packet->contents()), "D OF WARCRAFT CONNECTION - CLIENT TO SERVER"))
                    Disconnect();
                else
                {
                    WorldPacket data (SMSG_AUTH_CHALLENGE, 37);
                    data.append(sWorld.authSeed1.AsByteArray(), 16);
                    data.append(sWorld.authSeed2.AsByteArray(), 16);
                    data << mSeed;
                    data << uint8(1);
                    SendPacket(&data);
                }
            }break;
        case CMSG_AUTH_SESSION:
            {
                _HandleAuthSession(Packet);
            }break;
        case MSG_NULL_ACTION:
            { // We need to log opcodes that are non existent
                if(sLog.GetLogLevel() >= 4)
                {
                    FILE *codeLog = NULL;
                    fopen_s(&codeLog, "RecvOpcodeLog.txt", "a+b");
                    if(codeLog)
                    {
                        fprintf(codeLog, "Received unhandled packet %u(0x%.4X) with size %u\r\n", mUnaltered, mUnaltered, Packet->size());
                        Packet->hexlike(codeLog);
                        fclose(codeLog);
                    }
                }
            }break;
        case CMSG_LOG_DISCONNECT:
            {
                printf("Disconnect reason %u\n", Packet->read<uint32>());
            }break;
        default:
            {
                //printf("Queuing packet %s(0x%.4X)\n", sOpcodeMgr.GetOpcodeName(Packet->GetOpcode()), Packet->GetOpcode());
                if(mSession)
                {
                    mSession->QueuePacket(Packet);
                    continue;
                }
            }break;
        }
        delete Packet;
    }
}
