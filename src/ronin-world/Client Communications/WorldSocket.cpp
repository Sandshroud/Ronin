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
// Class WorldSocket - Main network code functions, handles
// reading/writing of all packets.

#include "StdAfx.h"

/* echo send/received packets to console */
//#define ECHO_PACKET_LOG_TO_CONSOLE 1

extern bool bServerShutdown;

WorldSocket::WorldSocket(SOCKET fd, const sockaddr_in * peer) : TcpSocket(fd, WORLDSOCKET_RECVBUF_SIZE, WORLDSOCKET_SENDBUF_SIZE, false, peer), _recvHeader(), _sendHeader()
{
    m_authed = false;
    mOpcode = mRemaining = mUnaltered = 0;
    _latency = 0;
    mSession = NULL;
    mSeed = RandomUInt();
    addonPacket = NULL;
    mQueued = false;
    mRequestID = 0;
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
        sWorld.CancelWorldPush(mSession);
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
        sWorld.CancelWorldPush(mSession);
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
        sLog.printf("WARNING: Tried to send a packet of %u bytes (which is too large) to a socket. Opcode was: %u (0x%04X)\n", uint(len), uint(opcode), uint(opcode));
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
            fprintf(codeLog, "Rejecting unset packet %u with size %zu\r\n", opcode, len);
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

extern void recordPacketStruct(uint16 opcode, size_t len, const void *data);

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
    else recordPacketStruct(opcode, len, data);
    LockWriteBuffer();

    //sLog.printf("Sending opcode %s%s (0x%.4X)\n", compressed ? "COMPRESSED_" : "", sOpcodeMgr.GetOpcodeName(opcode), opcode);
    // Encrypt the packet
    // First, create the header.
    _sendHeader.SetData(len+2, newOpcode);
    _crypt.EncryptSend(((uint8*)_sendHeader.header), _sendHeader.getHeaderLength());

    // Pass the header to our send buffer
    rv = WriteButHold(((const uint8*)_sendHeader.header), _sendHeader.getHeaderLength());

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
    if( WorldSession *session = sWorld.FindSession( AccountID ) )
    {
        if(session->IsHighPriority())
        {
            // Fail authentification until the player is finally added to world
            SendAuthResponse(AUTH_ALREADY_LOGGING_IN, false);
            return;
        }

        // Disconnect the target player from the session
        session->Disconnect();

        if(session->GetPlayer()) // Set our logout timer to 1 to queue a logout on session update
        {
            session->SetLogoutTimer(1);
            sWorld.SilentRemoveSession(session);
        } else sWorld.RemoveSession(AccountID);
    }

    Sha1Hash sha;
    if( m_fullAccountName == NULL )             // should never happen !
        sha.UpdateData(AccountName);
    else
    {
        sha.UpdateData(*m_fullAccountName);

        // this is unused now. we may as well free up the memory.
        delete m_fullAccountName;
        m_fullAccountName = NULL;
    }

    uint32 t = 0;
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
    pSession->Init();

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
        data.WriteBits(count, 22);
        data.FlushBits();

        for(std::map<uint32, uint8>::iterator itr = sItemMgr.HotfixOverridesBegin(); itr != sItemMgr.HotfixOverridesEnd(); itr++)
        {
            data << uint32(((itr->second & 0x02) ? 0x50238EC2 : 0x919BE54E));
            data << uint32(sWorld.GetStartTime());
            data << itr->first;
        }
    }
    else
    {
        // Write a null count
        data.WriteBits<uint32>(0, 22);
        data.FlushBits();
    }
    SendPacket(&data);

    data.Initialize(SMSG_TUTORIAL_FLAGS, 4 * 8);
    mSession->AppendTutorialData(&data);
    SendPacket(&data);

    pSession->_latency = _latency;
    sWorld.AddSession(pSession, true);
}

void WorldSocket::SendAuthResponse(uint8 code, bool holdsPosition, uint32 position)
{
    uint8 expansion = mSession ? mSession->GetHighestExpansion() : 0;
    WorldPacket data(SMSG_AUTH_RESPONSE, 15);
    if(holdsPosition) data.WriteBitString(3, 1, 0, 1);
    else data.WriteBitString(2, 0, 1);
    data.FlushBits();

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

    // Check if we should disable nagle to reduce overhead on slower sockets
    ToggleNagleAlgorithm(_latency > 500);
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
            memset(&_recvHeader, 0, sizeof(ClientPktHeader));
            Read(&_recvHeader, 6);

            // Decrypt the header
            _crypt.DecryptRecv((uint8*)&_recvHeader, sizeof (ClientPktHeader));
            mRemaining = ntohs(_recvHeader.size) - 4;
            mUnaltered = _recvHeader.cmd;
            mOpcode = sOpcodeMgr.ConvertOpcodeForInput(mUnaltered);
        }

        if(mRemaining > 0 && GetReadBuffer()->GetSize() < mRemaining )
            return; // We have a fragmented packet. Wait for the complete one before proceeding.

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
                        fprintf(codeLog, "Received unhandled packet %u(0x%.4X) with size %zu\r\n", mUnaltered, mUnaltered, Packet->size());
                        Packet->hexlike(codeLog);
                        fclose(codeLog);
                    }
                }
            }break;
        default:
            {
                //sLog.printf("Queuing packet %s(0x%.4X)\n", sOpcodeMgr.GetOpcodeName(Packet->GetOpcode()), Packet->GetOpcode());
                if(mSession)
                {
                    mSession->QueuePacket(Packet);
                    continue;
                }
            }break;
        case CMSG_LOG_DISCONNECT:
            break;
        }
        delete Packet;
    }
}
