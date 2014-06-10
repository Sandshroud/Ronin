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

void WorldSocket::OutPacket(uint16 opcode, size_t len, const void* data, bool InWorld)
{
    OUTPACKET_RESULT res;
    if( (len + 10) > WORLDSOCKET_SENDBUF_SIZE )
    {
        printf("WARNING: Tried to send a packet of %u bytes (which is too large) to a socket. Opcode was: %u (0x%04X)\n", uint(len), uint(opcode), uint(opcode));
        return;
    }

    res = _OutPacket(opcode, len, data, InWorld);
    if(res == OUTPACKET_RESULT_SUCCESS)
        return;
    if(res == OUTPACKET_RESULT_PACKET_ERROR)
    { // Track packets that cause packet errors
        if(!sLog.isOutDevelopment())
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
        /* queue the packet */
        queueLock.Acquire();
        WorldPacket *pck = new WorldPacket(opcode, len);
        pck->SetOpcode(opcode);
        if(len)
            pck->append((const uint8*)data, len);
        _queue.Push(pck);
        queueLock.Release();
    }
}

OUTPACKET_RESULT WorldSocket::_OutPacket(uint16 opcode, size_t len, const void* data, bool InWorld)
{
    bool rv;
    if(!IsConnected())
        return OUTPACKET_RESULT_NOT_CONNECTED;

    uint16 newOpcode = sOpcodeMgr.ConvertOpcodeForOutput(opcode);
    if(newOpcode == MSG_NULL_ACTION)
        return OUTPACKET_RESULT_PACKET_ERROR;
    else if( GetWriteBuffer()->GetSpace() < (len+4) )
        return OUTPACKET_RESULT_NO_ROOM_IN_BUFFER;

    LockWriteBuffer();
    // Encrypt the packet
    // First, create the header.
    ServerPktHeader Header(len + 2, newOpcode);
    _crypt.EncryptSend(((uint8*)Header.header), Header.getHeaderLength());

    // Pass the header to our send buffer
    rv = WriteButHold(((const uint8*)Header.header), Header.getHeaderLength());

    // Pass the rest of the packet to our send buffer (if there is any)
    if(len > 0 && rv)
        rv = Write((const uint8*)data, (uint32)len);
    else if(rv)
        rv = ForceSend();

    UnlockWriteBuffer();
    if(len > 0 && rv && !bServerShutdown)
        sWorld.NetworkStressOut += float(float(len+4)/1024);
    return rv ? OUTPACKET_RESULT_SUCCESS : OUTPACKET_RESULT_SOCKET_ERROR;
}

void WorldSocket::OnConnect()
{
    sWorld.mAcceptedConnections++;
    _latency = getMSTime();

    WorldPacket data (SMSG_AUTH_CHALLENGE, 37);
    data.append(sWorld.authSeed1.AsByteArray(), 16);
    data << uint8(1);
    data << mSeed;
    data.append(sWorld.authSeed2.AsByteArray(), 16);
    SendPacket(&data);
}

void WorldSocket::_HandleAuthSession(WorldPacket* recvPacket)
{
    uint32 addonSize;
    std::string account;
    _latency = getMSTime() - _latency;

    try
    {
        *recvPacket >> hashDigest[14] >> hashDigest[7] >> hashDigest[16];
        *recvPacket >> hashDigest[9] >> hashDigest[4] >> hashDigest[5] >> hashDigest[15];
        recvPacket->read_skip<uint32>();
        *recvPacket >> hashDigest[18];
        recvPacket->read_skip<uint64>();
        recvPacket->read_skip<uint32>();
        *recvPacket >> hashDigest[13];
        recvPacket->read_skip<uint8>();
        *recvPacket >> hashDigest[10] >> hashDigest[6];
        *recvPacket >> mClientSeed;
        recvPacket->read_skip<uint32>();
        *recvPacket >> hashDigest[19] >> hashDigest[11] >> hashDigest[17];
        *recvPacket >> hashDigest[8] >> hashDigest[12] >> hashDigest[0];
        *recvPacket >> mClientBuild;
        *recvPacket >> hashDigest[3];
        recvPacket->read_skip<uint8>();
        recvPacket->read_skip<uint32>();
        *recvPacket >> hashDigest[1] >> hashDigest[2];

        *recvPacket >> addonSize;
        if(addonSize)
        {
            uint8 *addonBytes = new uint8[addonSize];
            recvPacket->read(addonBytes, addonSize);
            addonPacket = new WorldPacket(CMSG_AUTH_SESSION, addonSize);
            addonPacket->append(addonBytes, addonSize);
            delete addonBytes;
        }

        *recvPacket >> account;
    }
    catch(ByteBufferException &)
    {
        sLog.outDebug("Incomplete copy of AUTH_SESSION Received.");
        return;
    }

    if(mClientBuild != CL_BUILD_SUPPORT)
    {
        OutPacket(SMSG_AUTH_RESPONSE, 1, "\x14");
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
    m_fullAccountName = new string( account );
}

void WorldSocket::InformationRetreiveCallback(WorldPacket & recvData, uint32 requestid)
{
    uint32 error;
    recvData >> error;
    if(error != 0 || requestid != mRequestID)
    {
        // something happened wrong @ the logon server
        OutPacket(SMSG_AUTH_RESPONSE, 1, "\x0D");
        return;
    }

    // Extract account information from the packet.
    string AccountName, GMFlags;
    const string * ForcedPermissions;
    uint32 AccountID;
    uint8 AccountFlags;
    string lang = "enUS";

    recvData >> AccountID >> AccountName >> GMFlags >> AccountFlags;
    ForcedPermissions = sLogonCommHandler.GetForcedPermissions(AccountName);
    if( ForcedPermissions != NULL )
        GMFlags.assign(ForcedPermissions->c_str());

    sLog.Debug( "WorldSocket","Received information packet from logon: `%s` ID %u (request %u)", AccountName.c_str(), AccountID, mRequestID);

    mRequestID = 0;
    uint8 K[40];
    recvData.read(K, 40);
    _crypt.Init(K);

    BigNumber BNK;
    BNK.SetBinary(K, 40);

    recvData.read((uint8*)lang.data(), 4);

    //checking if player is already connected
    //disconnect current player and login this one(blizzlike)
    WorldSession *session = sWorld.FindSession( AccountID );
    if( session != NULL )
    {
        if(session->IsHighPriority())
        {
            // Fail authentification until the player is finally added to world
            OutPacket(SMSG_AUTH_RESPONSE, 1, "\x15");
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
    sha.UpdateBigNumbers(&BNK, NULL);
    sha.Finalize();

    if (memcmp(sha.GetDigest(), hashDigest, 20))
    {
        // AUTH_UNKNOWN_ACCOUNT = 21
        OutPacket(SMSG_AUTH_RESPONSE, 1, "\x15");
        return;
    }

    m_authed = true;

    // Allocate session
    WorldSession *pSession = (mSession = new WorldSession(AccountID, AccountName, this));
    ASSERT(mSession);
    pSession->deleteMutex.Acquire();

    // Set session properties
    pSession->permissioncount = 0;//just to make sure it's 0
    pSession->SetClientBuild(mClientBuild);
    pSession->LoadSecurity(GMFlags);
    pSession->SetAccountFlags(AccountFlags);
    pSession->m_lastPing = (uint32)UNIXTIME;
    recvData >> pSession->m_muted;

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

    if(addonPacket)
    {
        sAddonMgr.SendAddonInfoPacket(addonPacket, pSession);
        delete addonPacket;
        addonPacket = NULL;
    }

    WorldPacket data(SMSG_CLIENTCACHE_VERSION, 4);
    data << uint32(CL_BUILD_SUPPORT);
    SendPacket(&data);

    if(ItemPrototypeStorage.HotfixBegin() != ItemPrototypeStorage.HotfixEnd())
    {
        WorldPacket hotFix(true ? SMSG_HOTFIX_NOTIFY : SMSG_HOTFIX_NOTIFY_BLOP, 100); // Blop or not, client will accept the info
        hotFix << uint32(0); // count
        hotFix << uint32(true ? 0x919BE54E : 0x50238EC2); // This can be either, the client will ask for both if no current db2 info is found
        uint32 count = 0;
        for(std::map<uint32, uint8>::iterator itr = ItemPrototypeStorage.HotfixBegin(); itr != ItemPrototypeStorage.HotfixEnd(); itr++)
        {
            hotFix << uint32(((itr->second & 0x02) ? 0x50238EC2 : 0x919BE54E));
            hotFix << uint32(UNIXTIME);
            hotFix << itr->first;
            count++;
        }
        hotFix.put<uint32>(0, count);
        SendPacket(&hotFix);
    }

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
    data << uint8(code) << uint32(0) << uint8(0) << uint32(0);
    data << uint8(expansion) << uint8(expansion);
    if(holdsPosition)
        data << position << uint8(0);
    SendPacket(&data);
}

void WorldSocket::_HandlePing(WorldPacket* recvPacket)
{
    uint32 ping;
    *recvPacket >> _latency;
    *recvPacket >> ping;

    if(mSession)
    {
        mSession->_latency = _latency;
        mSession->m_lastPing = (uint32)UNIXTIME;

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

            if(!bServerShutdown)
                sWorld.NetworkStressIn += float(float(mRemaining+6)/1024);
        }
        mRemaining = mOpcode = 0;

        // Check for packets that we handle
        switch(Packet->GetOpcode())
        {
        case CMSG_PING:
            {
                _HandlePing(Packet);
                delete Packet;
            }break;
        case CMSG_AUTH_SESSION:
            {
                _HandleAuthSession(Packet);
            }break;
        case MSG_NULL_ACTION:
            { // We need to log opcodes that are non existent
                if(!sLog.isOutDevelopment())
                    return;

                FILE *codeLog = NULL;
                fopen_s(&codeLog, "RecvOpcodeLog.txt", "a+b");
                if(codeLog)
                {
                    fprintf(codeLog, "Received unhandled packet %u(0x%.4X) with size %u\r\n", mUnaltered, mUnaltered, Packet->size());
                    Packet->hexlike(codeLog);
                    fclose(codeLog);
                }
                delete Packet;
                Packet = NULL;
            }break;
        default:
            {
                if(mSession)
                    mSession->QueuePacket(Packet);
                else
                {
                    delete Packet;
                    Packet = NULL;
                }
            }break;
        }
        mUnaltered = 0;
    }
}
