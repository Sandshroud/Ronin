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

#include "LogonStdAfx.h"
#include <openssl/md5.h>

enum _errors
{
    CE_SUCCESS              = 0x00,
    CE_IPBAN                = 0x01, // Unable to connect
    CE_ACCOUNT_CLOSED       = 0x03, // "This account has been closed and is no longer in service -- Please check the registered email address of this account for further information.";
    CE_NO_ACCOUNT           = 0x04, // The information you have entered is not valid. Please check the spelling of the account name and password. If you need help in retrieving a lost or stolen password, see <site> for more information
    CE_ACCOUNT_IN_USE       = 0x06, // This account is already logged in.  Please check the spelling and try again.
    CE_PREORDER_TIME_LIMIT  = 0x07,
    CE_SERVER_FULL          = 0x08, // Could not log in to <game> at this time. Please try again later.
    CE_WRONG_BUILD_NUMBER   = 0x09, // Unable to validate game version. This may be caused by file corruption or interference of another program. Please visit <site> for more information and possible solutions to this issue.
    CE_UPDATE_CLIENT        = 0x0a, // Downloading
    CE_ACCOUNT_FREEZED      = 0x0c, // This <game> account has been temporarily suspended. Please go to <site>/banned.html for further information
    CE_ACCOUNT_PARENTAL     = 0x0f, // Access to this account has been blocked by parental controls. Your settings may be changed in your account preferences at <site>
};

AuthSocket::AuthSocket(SOCKET fd, const sockaddr_in * peer) : TcpSocket(fd, 32768, 4096, false, peer)
{
    N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);
    s.SetRand(256);
    m_account = 0;
    last_recv = time(NULL);
    removedFromSet = false;
    m_patch=NULL;
    m_patchJob=NULL;
    _authSocketLock.Acquire();
    _authSockets.insert(this);
    _authSocketLock.Release();
    m_state = STATE_NEED_CHALLENGE;
}

AuthSocket::~AuthSocket()
{
    ASSERT(!m_patchJob);
}

void AuthSocket::OnDisconnect()
{
    if(!removedFromSet)
    {
        _authSocketLock.Acquire();
        _authSockets.erase(this);
        _authSocketLock.Release();
    }

    if(m_patchJob)
    {
        PatchMgr::getSingleton().AbortPatchJob(m_patchJob);
        m_patchJob=NULL;
    }
}

void AuthSocket::SendChallengeError(uint8 Error)
{
    uint8 buffer[3];
    buffer[0] = buffer[1] = 0;
    buffer[2] = Error;

    Send(buffer, 3);
}

void AuthSocket::SendProofError(uint8 Error, uint8 * M2)
{
    uint8 buffer[32];
    memset(buffer, 0, 32);

    buffer[0] = 1;
    buffer[1] = Error;
    if(M2 == 0)
    {
#ifdef USING_BIG_ENDIAN
        *(uint32*)&buffer[2] = swap32(3);
#else
        *(uint32*)&buffer[2] = 3;
#endif
        Send(buffer, 6);
        return;
    }
    
    memcpy(&buffer[2], M2, 20);
    Send(buffer, 32);
}

typedef bool (AuthSocket::*AuthPacketHandler)();
struct AuthHandler
{
    uint8 command;
    AuthPacketHandler func;
};

static AuthHandler Handlers[] = {
    { 0x00, &AuthSocket::HandleChallenge },
    { 0x01, &AuthSocket::HandleProof },
    { 0x02, &AuthSocket::HandleReconnectChallenge },
    { 0x03, &AuthSocket::HandleReconnectProof },
    { 0x10, &AuthSocket::HandleRealmlist },
    { 0x13, &AuthSocket::HandleCMD19 },
    // 0x30 Init Transfer
    // 0x31 Transfer data
    { 0x32, &AuthSocket::HandleTransferAccept },
    { 0x33, &AuthSocket::HandleTransferResume },
    { 0x34, &AuthSocket::HandleTransferCancel },
    { 0xFF, NULL },
};

bool AuthSocket::HandleCMD19()
{
    return false;
}

void AuthSocket::OnRecvData()
{
    if(!IsConnected())
        return;

    if(m_state == STATE_CLOSED)
    {
        Disconnect();
        return;
    }

    last_recv = UNIXTIME;
    AuthPacketHandler *Handler = NULL;
    uint8 Command = *(uint8*)GetReadBuffer()->GetBufferOffset();
    for(uint8 i = 0; Handlers[i].command != 0xFF; i++)
        if(Handlers[i].command == Command)
            Handler = &Handlers[i].func;
    if(Handler == NULL)
        sLog.Debug("AuthSocket", "Unknown cmd %u", Command);
    else if(!(this->*(*Handler))())
    {
        m_state = STATE_CLOSED;
        Disconnect();
    }
}

bool AuthSocket::HandleRealmlist()
{
    if(GetReadBuffer()->GetSize() < sizeof(sRealmlistChallenge_C))
        return true;

    // Check the rest of the packet is complete.
    uint8 * ReceiveBuffer = (uint8*)GetReadBuffer()->GetBufferOffset();
    uint32 full_size = 5;
    if((full_size += (*(uint32*)&ReceiveBuffer[1])) > 5)
        if(GetReadBuffer()->GetSize() < full_size)
            return true;
    GetReadBuffer()->Remove(full_size);

    // First realmlist request is for auto connect
    // Ignore, client pushes auth challenge after
    if(m_state == STATE_NEED_CHALLENGE)
        return true;

    // If we're not at either state of authentication(full or non) return an error
    if(m_state != STATE_AUTHENTICATED)
    {
        sLog.Notice("AuthSocket","Realmlist without auth\n");
        return false;
    }

    sLog.Debug("AuthSocket","HandleRealmlist.");
    sInfoCore.SendRealms(this);
    return true;
}

bool AuthSocket::HandleChallenge()
{
    if(m_state != STATE_NEED_CHALLENGE)
        return false;

    // No header
    if(GetReadBuffer()->GetSize() < 4)
        return true;

    if(sInfoCore.GetRealmMap().empty())
    {   // If we lack a realm to connect to, block em, it's better then making them sit and get locked into an empty list.
        SendChallengeError(CE_IPBAN);
        m_state = STATE_CLOSED;
        return true;
    }

    // Check the rest of the packet is complete.
    uint8 * ReceiveBuffer = (uint8*)GetReadBuffer()->GetBufferOffset();
    uint16 full_size = *(uint16*)&ReceiveBuffer[2];

    // Make sure it's not a bloated packet
    if(full_size > sizeof(sAuthLogonChallenge_C))
        return false;

    sLog.Debug("AuthChallenge","got header, body is 0x%02X bytes", full_size);
    if(GetReadBuffer()->GetSize() < uint32(full_size+4))
        return true;

    sLog.Debug("AuthChallenge","got full packet.");

    // Copy the data into our cached challenge structure
    GetReadBuffer()->Read(&m_challenge, full_size + 4);

    // Check client build.
    if(GetBuild() > LogonServer::getSingleton().max_build)
    {
        SendChallengeError(CE_WRONG_BUILD_NUMBER);
        m_state = STATE_CLOSED;
        return true;
    }

    if(GetBuild() < LogonServer::getSingleton().min_build)
    {
        // can we patch?
        char flippedloc[5] = {0,0,0,0,0};
        flippedloc[0] = m_challenge.country[3];
        flippedloc[1] = m_challenge.country[2];
        flippedloc[2] = m_challenge.country[1];
        flippedloc[3] = m_challenge.country[0];

        m_patch = PatchMgr::getSingleton().FindPatchForClient(GetBuild(), flippedloc);
        if(m_patch == NULL)
        {
            // could not find a valid patch
            SendChallengeError(CE_WRONG_BUILD_NUMBER);
            m_state = STATE_CLOSED;
            return true;
        }

        sLog.Debug("Patch", "Selected patch %u%s for client.", m_patch->Version,m_patch->Locality);

        BigNumber unk;
        unk.SetRand(128);

        uint8 response[119] = {
            0x00, 0x00, 0x00, 0x72, 0x50, 0xa7, 0xc9, 0x27, 0x4a, 0xfa, 0xb8, 0x77, 0x80, 0x70, 0x22,
            0xda, 0xb8, 0x3b, 0x06, 0x50, 0x53, 0x4a, 0x16, 0xe2, 0x65, 0xba, 0xe4, 0x43, 0x6f, 0xe3,
            0x29, 0x36, 0x18, 0xe3, 0x45, 0x01, 0x07, 0x20, 0x89, 0x4b, 0x64, 0x5e, 0x89, 0xe1, 0x53,
            0x5b, 0xbd, 0xad, 0x5b, 0x8b, 0x29, 0x06, 0x50, 0x53, 0x08, 0x01, 0xb1, 0x8e, 0xbf, 0xbf,
            0x5e, 0x8f, 0xab, 0x3c, 0x82, 0x87, 0x2a, 0x3e, 0x9b, 0xb7, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe1, 0x32, 0xa3,
            0x49, 0x76, 0x5c, 0x5b, 0x35, 0x9a, 0x93, 0x3c, 0x6f, 0x3c, 0x63, 0x6d, 0xc0, 0x00
        };
        Send(response, 119);

        // Wait for proof
        m_state = STATE_NEED_PROOF;
        return true;
    }

    // Check for a possible IP ban on this client.
    BAN_STATUS ipb = IPBanner::getSingleton().CalculateBanStatus(GetRemoteAddress());

    switch(ipb)
    {
    case BAN_STATUS_PERMANENT_BAN:
        SendChallengeError(CE_ACCOUNT_CLOSED);
        m_state = STATE_CLOSED;
        return true;
    case BAN_STATUS_TIME_LEFT_ON_BAN:
        SendChallengeError(CE_ACCOUNT_FREEZED);
        m_state = STATE_CLOSED;
        return true;
    }

    // Null-terminate the account string
    if(m_challenge.I_len >= 0x50)
        return false;

    m_challenge.I[m_challenge.I_len] = 0;
    AccountName = (char*)&m_challenge.I;
    std::string::size_type i = AccountName.rfind("#");
    if( i != std::string::npos )
    {
        printf("# ACCOUNTNAME!\n");
        return false;
    }

    // Look up the account information
    m_account = AccountMgr::getSingleton().GetAccount(AccountName);
    if(m_account == 0)
    {
        sLog.Debug("AuthChallenge","Account Name: \"%s\" - Account state: INVALID", AccountName.c_str());

        // Non-existant account
        SendChallengeError(CE_NO_ACCOUNT);
        m_state = STATE_CLOSED;
        return true;
    }

    // Check that the account isn't banned.
    if(m_account->Banned == 1)
    {
        sLog.Notice("AuthChallenge","Account Name: \"%s\" - Account state: CLOSED", AccountName.c_str());
        SendChallengeError(CE_ACCOUNT_CLOSED);
        m_state = STATE_CLOSED;
        return true;
    }
    else if(m_account->Banned > 0)
    {
        sLog.Notice("AuthChallenge","Account Name: \"%s\" - Account state: FROZEN (%u)", AccountName.c_str(), m_account->Banned);
        SendChallengeError(CE_ACCOUNT_FREEZED);
        m_state = STATE_CLOSED;
        return true;
    } else sLog.Notice("AuthChallenge","Account Name: \"%s\" - Account state: OK", AccountName.c_str());

    // update cached locale
    if(!m_account->forcedLocale)
    {
        char temp[4];
        temp[0] = m_challenge.country[3];
        temp[1] = m_challenge.country[2];
        temp[2] = m_challenge.country[1];
        temp[3] = m_challenge.country[0];

        *(uint32*)&m_account->Locale[0] = *(uint32*)temp;
    }

    Sha1Hash sha;
    //uint32 tc = s.GetNumBytes();
    sha.UpdateData( s.AsByteArray(), 32 );
    sha.UpdateData( m_account->SrpHash, 20 );
    sha.Finalize();

    BigNumber x;
    x.SetBinary( sha.GetDigest(), sha.GetLength() );
    v = g.ModExp(x, N);
    b.SetRand(152);

    BigNumber gmod = g.ModExp(b, N);
    B = ((v * 3) + gmod) % N;
    ASSERT(gmod.GetNumBytes() <= 32);

    BigNumber unk;
    unk.SetRand(128);
    uint8 response[200];
    uint32 c = 0;
    response[c] = 0;                                        c += 1;
    response[c] = 0;                                        c += 1;
    response[c] = CE_SUCCESS;                               c += 1;
    memcpy(&response[c], B.AsByteArray(), 32);              c += 32;
    response[c] = 1;                                        c += 1;
    response[c] = g.AsByteArray()[0];                       c += 1;
    response[c] = 32;                                       c += 1;
    memcpy(&response[c], N.AsByteArray(), 32);              c += 32;
    memcpy(&response[c], s.AsByteArray(), s.GetNumBytes()); c += s.GetNumBytes();
    memcpy(&response[c], unk.AsByteArray(), 16);            c += 16;
    response[c] = 0;                                        c += 1;

    sLog.Debug("AuthSocket","Sending Success Response");
    // Send our challenge response
    Send(response, c);

    // Wait for proof
    m_state = STATE_NEED_PROOF;
    return true;
}

bool AuthSocket::HandleProof()
{
    if( m_state != STATE_NEED_PROOF || m_account == NULL )
        return false;

    if(GetReadBuffer()->GetSize() < sizeof(sAuthLogonProof_C))
        return true;

    // patch
    if(m_patch && !m_account)
    {
        // Clear out the proof
        GetReadBuffer()->Remove(sizeof(sAuthLogonProof_C));

        // Send patch start
        sLog.Debug("AuthLogonProof","Intitiating PatchJob");
        uint8 bytes[2] = {0x01,0x0a};
        Send(bytes,2);
        PatchMgr::getSingleton().InitiatePatch(m_patch, this);

        // Wait for patch data
        m_state = STATE_PATCHING;
        return true;
    }

    sLog.Debug("AuthLogonProof","Interleaving and checking proof...");

    sAuthLogonProof_C lp;
    GetReadBuffer()->Read(&lp, sizeof(sAuthLogonProof_C));

    BigNumber A;
    A.SetBinary(lp.A, 32);

    Sha1Hash sha;
    sha.UpdateBigNumbers(&A, &B, 0);
    sha.Finalize();

    BigNumber u;
    u.SetBinary(sha.GetDigest(), 20);
    
    BigNumber S = (A * (v.ModExp(u, N))).ModExp(b, N);
    uint8 t[32];
    uint8 t1[16];
    uint8 vK[40];
    memcpy(t, S.AsByteArray(), 32);
    for (int i = 0; i < 16; i++)
    {
        t1[i] = t[i*2];
    }
    sha.Initialize();
    sha.UpdateData(t1, 16);
    sha.Finalize();
    for (int i = 0; i < 20; i++)
    {
        vK[i*2] = sha.GetDigest()[i];
    }
    for (int i = 0; i < 16; i++)
    {
        t1[i] = t[i*2+1];
    }
    sha.Initialize();
    sha.UpdateData(t1, 16);
    sha.Finalize();
    for (int i = 0; i < 20; i++)
    {
        vK[i*2+1] = sha.GetDigest()[i];
    }
    m_sessionkey.SetBinary(vK, 40);

    uint8 hash[20];

    sha.Initialize();
    sha.UpdateBigNumbers(&N, NULL);
    sha.Finalize();
    memcpy(hash, sha.GetDigest(), 20);
    sha.Initialize();
    sha.UpdateBigNumbers(&g, NULL);
    sha.Finalize();
    for (int i = 0; i < 20; i++)
    {
        hash[i] ^= sha.GetDigest()[i];
    }
    BigNumber t3;
    t3.SetBinary(hash, 20);

    sha.Initialize();
    sha.UpdateData((const uint8*)m_account->UsernamePtr->c_str(), (int)m_account->UsernamePtr->size());
    sha.Finalize();

    BigNumber t4;
    t4.SetBinary(sha.GetDigest(), 20);

    sha.Initialize();
    sha.UpdateBigNumbers(&t3, &t4, &s, &A, &B, &m_sessionkey, NULL);
    sha.Finalize();

    BigNumber M;
    M.SetBinary(sha.GetDigest(), 20);

    // Compare M1 values.
    if(memcmp(lp.M1, M.AsByteArray(), 20) != 0)
    {
        // Authentication failed.
        //SendProofError(4, 0);
        sLog.Debug("AuthLogonProof","M1 values don't match.");
        SendChallengeError(CE_NO_ACCOUNT);
        m_state = STATE_CLOSED;
        return true;
    }

    // Store sessionkey
    m_account->SetSessionKey(m_sessionkey.AsByteArray());

//  sLog.outDebug("========================\nSession key: ");
//  for(uint32 z = 0; z < 40; ++z)
//      sLog.outDebug("%.2X ", m_account->SessionKey[z]);
//  sLog.outDebug("\n========================\n");

    // let the client know
    sha.Initialize();
    sha.UpdateBigNumbers(&A, &M, &m_sessionkey, 0);
    sha.Finalize();
    if(GetBuild() <= 6005)
    {
        sAuthLogonProof_S proof;
        proof.cmd = 0x01;
        proof.error = 0;
        memcpy(proof.M2, sha.GetDigest(), 20);
        proof.unk2 = 0;
        SendPacket( (uint8*) &proof, sizeof(proof) );
    } else SendProofError(0, sha.GetDigest());

    sLog.Debug("AuthLogonProof","Authentication Success.");

    // Don't update when IP banned, but update anyway if it's an account ban
    const char* m_sessionkey_hex = m_sessionkey.AsHexStr();
    sLogonSQL->Execute("UPDATE accounts SET lastlogin=NOW(), SessionKey = '%s', lastip='%s' WHERE acct=%u;", m_sessionkey_hex, GetIP(), m_account->AccountId);

    // we're authenticated now :)
    m_state = STATE_AUTHENTICATED;
    return true;
}

bool AuthSocket::HandleReconnectChallenge()
{
    // Only when we are looking for it
    if(m_state != STATE_NEED_CHALLENGE)
        return false;

    // No header
    if(GetReadBuffer()->GetSize() < 4)
        return true;

    // Check the rest of the packet is complete.
    uint8 * ReceiveBuffer = (uint8*)GetReadBuffer()->GetBufferOffset();
    uint16 full_size = *(uint16*)&ReceiveBuffer[2];

    // Make sure the packet is not bloated
    if((size_t)(full_size+4) > sizeof(sAuthLogonChallenge_C))
        return false;

    sLog.Debug("ReconnectChallenge","got header, body is 0x%02X bytes", full_size);
    if(GetReadBuffer()->GetSize() < (uint32)full_size+4)
        return true;

    sLog.Debug("ReconnectChallenge", "got full packet.");

    // Copy the data into our cached challenge structure
    memcpy(&m_challenge, ReceiveBuffer, full_size + 4);
    GetReadBuffer()->Read(&m_challenge, full_size + 4);

    // Check client build.
    if(m_challenge.build > LogonServer::getSingleton().max_build ||
        m_challenge.build < LogonServer::getSingleton().min_build)
    {
        SendChallengeError(CE_WRONG_BUILD_NUMBER);
        m_state = STATE_CLOSED;
        return true;
    }

    // Check for a possible IP ban on this client.
    BAN_STATUS ipb = IPBanner::getSingleton().CalculateBanStatus(GetRemoteAddress());

    switch(ipb)
    {
    case BAN_STATUS_PERMANENT_BAN:
        SendChallengeError(CE_ACCOUNT_CLOSED);
        m_state = STATE_CLOSED;
        return true;

    case BAN_STATUS_TIME_LEFT_ON_BAN:
        SendChallengeError(CE_ACCOUNT_FREEZED);
        m_state = STATE_CLOSED;
        return true;
    }

    // Null-terminate the account string
    if( m_challenge.I_len >= 50 )
        return false;

    m_challenge.I[m_challenge.I_len] = 0;

    // Look up the account information
    AccountName = (char*)&m_challenge.I;
    sLog.Notice("ReconnectChallenge","Account Name: \"%s\"", AccountName.c_str());

    m_account = AccountMgr::getSingleton().GetAccount(AccountName);
    if(m_account == 0)
    {
        sLog.Debug("ReconnectChallenge","Invalid account.");

        // Non-existant account
        SendChallengeError(CE_NO_ACCOUNT);
        m_state = STATE_CLOSED;
        return true;
    }


    // Check that the account isn't banned.
    if(m_account->Banned == 1)
    {
        sLog.Notice("ReconnectChallenge","Account banned state = %u", m_account->Banned);
        SendChallengeError(CE_ACCOUNT_CLOSED);
        m_state = STATE_CLOSED;
        return true;
    }
    else if(m_account->Banned > 0)
    {
        sLog.Notice("ReconnectChallenge","Account banned state = %u", m_account->Banned);
        SendChallengeError(CE_ACCOUNT_FREEZED);
        m_state = STATE_CLOSED;
        return true;
    } else sLog.Debug("ReconnectChallenge","Account banned state = %u", m_account->Banned);

    if(!m_account->SessionKey)
    {
        SendChallengeError(CE_SERVER_FULL);
        m_state = STATE_CLOSED;
        return true;
    }

    // Mangos is original source for both version builds, many thanks
    ///- Sending response
    if(GetBuild() <= 6005)
    {
        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, m_account->SessionKey, 40);
        uint8 buffer[20];
        MD5_Final(buffer, &ctx);
        ByteBuffer buf;
        buf << uint16(2);
        buf.append(buffer, 20);
        buf << uint64(0);
        buf << uint64(0);
        Send(buf.contents(), 34);
    }
    else
    {
        ByteBuffer pkt;
        pkt << (uint8)  0x02;   //ReconnectChallenge
        pkt << (uint8)  0x00;
        rs.SetRand(16*8);
        pkt.append(rs.AsByteArray(), 16);    // 16 bytes random
        pkt << uint64(0x00) << uint64(0x00);    // 16 bytes zeros
        Send(pkt.contents(), uint32(pkt.size()));
    }

    // Now we wait for reconnect proof
    m_state = STATE_NEED_REPROOF;
    return true;
}

bool AuthSocket::HandleReconnectProof()
{
    if( m_state != STATE_NEED_REPROOF || m_account == NULL )
        return false;

    if(GetReadBuffer()->GetSize() < sizeof(sAuthLogonProofKey_C))
        return true;

    // Load sessionkey from account database.
    if(QueryResult *result = sLogonSQL->Query ("SELECT SessionKey FROM accounts WHERE acct = %u", m_account->AccountId))
    {
        Field * field = result->Fetch();
        K.SetHexStr(field[0].GetString());
        delete result;
    }
    else
    {
        // Disconnect if the sessionkey invalid or not found
        sLog.Debug("AuthReConnectProof","No matching SessionKey found while user %s tried to login.", AccountName.c_str());
        return false;
    }

    sAuthLogonProofKey_C lp;
    GetReadBuffer()->Read(&lp, sizeof(sAuthLogonProofKey_C));

    BigNumber A;
    A.SetBinary(lp.R1, 16);

    Sha1Hash sha;
    sha.Initialize();
    sha.UpdateData(AccountName);
    sha.UpdateBigNumbers(&A, &rs, &K, 0);
    sha.Finalize();

    if (memcmp(sha.GetDigest(), lp.R2, SHA_DIGEST_LENGTH))
    {
        sLog.Debug("AuthReConnectProof","Authentication Failed.");
        return false;
    }

    sLog.Debug("AuthReConnectProof","Authentication Success.");

    ///- Sending response
    ByteBuffer pkt;
    pkt << (uint8)  0x03;   //ReconnectProof
    pkt << (uint8)  0x00;
    if(GetBuild() > 6141)
        pkt << (uint16) 0x0000;   // 2 bytes zeros
    Send(pkt.contents(), uint32(pkt.size()));

    // we're authenticated now :)
    m_state = STATE_AUTHENTICATED;
    return true;
}

bool AuthSocket::HandleTransferAccept()
{
    sLog.Debug("AuthSocket","Accepted transfer");
    if(m_state != STATE_PATCHING || m_patch == NULL)
        return false;

    GetReadBuffer()->Remove(1);
    PatchMgr::getSingleton().BeginPatchJob(m_patch,this,0);
    return true;
}

bool AuthSocket::HandleTransferResume()
{
    sLog.Debug("AuthSocket","Resuming transfer");
    if(m_state != STATE_PATCHING || m_patch == NULL)
        return false;

    GetReadBuffer()->Remove(1);
    uint64 size;
    GetReadBuffer()->Read(&size, 8);
    if(size >= m_patch->FileSize)
        return false;

    PatchMgr::getSingleton().BeginPatchJob(m_patch,this,(uint32)size);
    return true;
}

bool AuthSocket::HandleTransferCancel()
{
    GetReadBuffer()->Remove(1);
    return false;
}
