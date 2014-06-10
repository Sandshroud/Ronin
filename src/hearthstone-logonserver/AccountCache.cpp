/***
 * Demonstrike Core
 */

#include "LogonStdAfx.h"

initialiseSingleton(AccountMgr);
initialiseSingleton(IPBanner);
initialiseSingleton(InformationCore);

void AccountMgr::ReloadAccounts(bool silent)
{
    setBusy.Acquire();
    if(!silent) sLog.outString("[AccountMgr] Reloading Accounts...");

    // Load *all* accounts.
    QueryResult * result = sLogonSQL->Query("SELECT acct, login, password, gm, flags, banned, forceLanguage, muted FROM accounts");
    Field * field;
    std::string AccountName;
    std::set<std::string> account_list;
    Account * acct;

    if(result)
    {
        do 
        {
            field = result->Fetch();
            AccountName = field[1].GetString();

            // transform to uppercase
            HEARTHSTONE_TOUPPER(AccountName);

            //Use private __GetAccount, for locks
            acct = __GetAccount(AccountName);
            if(acct == 0)
            {
                // New account.
                AddAccount(field);
            }
            else
            {
                // Update the account with possible changed details.
                UpdateAccount(acct, field);
            }

            // add to our "known" list
            account_list.insert(AccountName);

        } while(result->NextRow());
        delete result;
    }

    // check for any purged/deleted accounts
#ifdef WIN32
    HM_NAMESPACE::hash_map<std::string, Account*>::iterator itr = AccountDatabase.begin();
    HM_NAMESPACE::hash_map<std::string, Account*>::iterator it2;
#else
    std::map<string, Account*>::iterator itr = AccountDatabase.begin();
    std::map<string, Account*>::iterator it2;
#endif

    for(; itr != AccountDatabase.end();)
    {
        it2 = itr;
        ++itr;

        if(account_list.find(it2->first) == account_list.end())
        {
            delete it2->second;
            AccountDatabase.erase(it2);
        }
        else
        {
            it2->second->UsernamePtr = (std::string*)&it2->first;
        }
    }

    if(!silent) sLog.outString("[AccountMgr] Found %u accounts.", AccountDatabase.size());
    setBusy.Release();

    IPBanner::getSingleton().Reload();
}

void AccountMgr::AddAccount(Field* field)
{
    Account * acct = new Account;
    Sha1Hash hash;
    std::string Username     = field[1].GetString();
    std::string Password     = field[2].GetString();
    //string EncryptedPassword = field[3].GetString();
    std::string GMFlags      = field[3].GetString();

    acct->AccountId             = field[0].GetUInt32();
    acct->AccountFlags          = field[4].GetUInt8();
    acct->Banned                = field[5].GetUInt32();
    if ( (uint32)UNIXTIME > acct->Banned && acct->Banned != 0 && acct->Banned != 1) //1 = perm ban?
    {
        //Accounts should be unbanned once the date is past their set expiry date.
        acct->Banned = 0;
        //me go boom :(
        //printf("Account %s's ban has expired.\n",acct->UsernamePtr->c_str());
        sLogonSQL->Execute("UPDATE accounts SET banned = 0 WHERE acct=%u",acct->AccountId);
    }
    acct->SetGMFlags(GMFlags.c_str());
    acct->Locale[0] = 'e';
    acct->Locale[1] = 'n';
    acct->Locale[2] = 'U';
    acct->Locale[3] = 'S';
    if(strcmp(field[6].GetString(), "enUS"))
    {
        // non-standard language forced
        memcpy(acct->Locale, field[6].GetString(), 4);
        acct->forcedLocale = true;
    }
    else
        acct->forcedLocale = false;

    acct->Muted = field[7].GetUInt32();
    if ( (uint32)UNIXTIME > acct->Muted && acct->Muted != 0 && acct->Muted != 1) //1 = perm ban?
    {
        //Accounts should be unbanned once the date is past their set expiry date.
        acct->Muted= 0;
        sLog.Debug("AccountMgr","Account %s's mute has expired.", Username.c_str());
        sLogonSQL->Execute("UPDATE accounts SET muted = 0 WHERE acct=%u",acct->AccountId);
    }
    // Convert username/password to uppercase. this is needed ;)
    HEARTHSTONE_TOUPPER(Username);
    HEARTHSTONE_TOUPPER(Password);
    
    if( m_encryptedPasswords )
    {
        // prefer encrypted passwords over nonencrypted
        BigNumber bn;
        bn.SetHexStr( Password.c_str() );
        if( bn.GetNumBytes() != 20 )
        {
            // Someone probably has non-encrypted passwords in a server that's set to encrypted pws.
            hash.UpdateData((Username + ":" + Password));
            hash.Finalize();
            memcpy(acct->SrpHash, hash.GetDigest(), 20);
            // Make sure this doesn't happen again.
            BigNumber cnSave;
            cnSave.SetBinary( acct->SrpHash, 20);
            std::string hash = cnSave.AsHexStr();
            sLog.Debug("AccountMgr", "Found account %s [%u] with invalid password format. Converting to encrypted password.", Username.c_str(), acct->AccountId);
            sLogonSQL->Execute("UPDATE accounts SET password = SHA1(CONCAT(UPPER(login), ':', UPPER(password))) WHERE acct = %u", acct->AccountId);
        }
        else
        {
            if ( Password.size() == 40 )
            {
                if( bn.GetNumBytes() < 20 )
                {
                    memcpy(acct->SrpHash, bn.AsByteArray(), bn.GetNumBytes());
                    for (int n=bn.GetNumBytes(); n<=19; n++)
                        acct->SrpHash[n] = (uint8)0;
                    reverse_array(acct->SrpHash, 20);
                }
                else
                {
                    memcpy(acct->SrpHash, bn.AsByteArray(), 20);
                    reverse_array(acct->SrpHash, 20);
                }
            }
        }
    }
    else
    {
        // Prehash the I value.
        hash.UpdateData((Username + ":" + Password));
        hash.Finalize();
        memcpy(acct->SrpHash, hash.GetDigest(), 20);
    }

    AccountDatabase[Username] = acct;
}

void AccountMgr::UpdateAccount(Account * acct, Field * field)
{
    uint32 id = field[0].GetUInt32();
    Sha1Hash hash;
    std::string Username     = field[1].GetString();
    std::string Password     = field[2].GetString();
    //string EncryptedPassword = field[3].GetString();
    std::string GMFlags      = field[3].GetString();

    if(id != acct->AccountId)
    {
        //printf("Account %u `%s` is a duplicate.\n", id, acct->Username.c_str());
        sLog.outColor(TYELLOW, " >> deleting duplicate account %u [%s]...", id, Username.c_str());
        sLog.outColor(TNORMAL, "\n");
        sLogonSQL->Execute("DELETE FROM accounts WHERE acct=%u", id);
        return;
    }

    acct->AccountId             = field[0].GetUInt32();
    acct->AccountFlags          = field[4].GetUInt8();
    acct->Banned                = field[5].GetUInt32();
    if ((uint32)UNIXTIME > acct->Banned && acct->Banned != 0 && acct->Banned != 1) //1 = perm ban?
    {
        //Accounts should be unbanned once the date is past their set expiry date.
        acct->Banned = 0;
        sLog.Debug("AccountMgr","Account %s's ban has expired.",acct->UsernamePtr->c_str());
        sLogonSQL->Execute("UPDATE accounts SET banned = 0 WHERE acct=%u",acct->AccountId);
    }
    acct->SetGMFlags(GMFlags.c_str());
    if(strcmp(field[6].GetString(), "enUS"))
    {
        // non-standard language forced
        memcpy(acct->Locale, field[6].GetString(), 4);
        acct->forcedLocale = true;
    }
    else
        acct->forcedLocale = false;

    acct->Muted = field[7].GetUInt32();
    if ((uint32)UNIXTIME > acct->Muted && acct->Muted != 0 && acct->Muted != 1) //1 = perm ban?
    {
        //Accounts should be unbanned once the date is past their set expiry date.
        acct->Muted= 0;
        sLog.outDebug("Account %s's mute has expired.",acct->UsernamePtr->c_str());
        sLogonSQL->Execute("UPDATE accounts SET muted = 0 WHERE acct=%u",acct->AccountId);
    }

    // Convert username/password to uppercase. this is needed ;)
    HEARTHSTONE_TOUPPER(Username);
    HEARTHSTONE_TOUPPER(Password);

    if( m_encryptedPasswords )
    {
        // prefer encrypted passwords over nonencrypted
        BigNumber bn;
        bn.SetHexStr( Password.c_str() );
        if( bn.GetNumBytes() != 20 )
        {
            // Someone probably has non-encrypted passwords in a server that's set to encrypted pws.
            hash.UpdateData((Username + ":" + Password));
            hash.Finalize();
            memcpy(acct->SrpHash, hash.GetDigest(), 20);
            // Make sure this doesn't happen again.
            BigNumber cnSave;
            cnSave.SetBinary( acct->SrpHash, 20);
            std::string hash = cnSave.AsHexStr();
            sLog.Debug("AccountMgr", "Found account %s [%u] with invalid password format. Converting to encrypted password.", Username.c_str(), acct->AccountId);
            sLogonSQL->Execute("UPDATE accounts SET password = SHA1(CONCAT(UPPER(login), ':', UPPER(password))) WHERE acct = %u", acct->AccountId);
        }
        else
        {
            if ( Password.size() == 40 )
            {
                if( bn.GetNumBytes() < 20 )
                {
                    memcpy(acct->SrpHash, bn.AsByteArray(), bn.GetNumBytes());
                    for (int n=bn.GetNumBytes(); n<=19; n++)
                        acct->SrpHash[n] = (uint8)0;
                    reverse_array(acct->SrpHash, 20);
                }
                else
                {
                    memcpy(acct->SrpHash, bn.AsByteArray(), 20);
                    reverse_array(acct->SrpHash, 20);
                }
            }
        }
    }
    else
    {
        // Prehash the I value.
        hash.UpdateData((Username + ":" + Password));
        hash.Finalize();
        memcpy(acct->SrpHash, hash.GetDigest(), 20);
    }
}

void AccountMgr::ReloadAccountsCallback()
{
    ReloadAccounts(true);
}
BAN_STATUS IPBanner::CalculateBanStatus(in_addr ip_address)
{
    Guard lguard(listBusy);
    std::list<IPBan>::iterator itr;
    std::list<IPBan>::iterator itr2 = banList.begin();
    for(; itr2 != banList.end();)
    {
        itr = itr2;
        ++itr2;

        if( ParseCIDRBan(ip_address.s_addr, itr->Mask, itr->Bytes) )
        {
            // ban hit
            if( itr->Expire == 0 )
                return BAN_STATUS_PERMANENT_BAN;
            
            if( (uint32)UNIXTIME >= itr->Expire )
            {
                sLogonSQL->Execute("DELETE FROM ipbans WHERE expire = %u AND ip = \"%s\"", itr->Expire, sLogonSQL->EscapeString(itr->db_ip).c_str());
                banList.erase(itr);
            }
            else
            {
                return BAN_STATUS_TIME_LEFT_ON_BAN;
            }
        }
    }

    return BAN_STATUS_NOT_BANNED;
}

bool IPBanner::Add(const char * ip, uint32 dur)
{
    std::string sip = std::string(ip);
    std::string::size_type i = sip.find("/");
    if( i == std::string::npos )
        return false;

    std::string stmp = sip.substr(0, i);
    std::string smask = sip.substr(i+1);

    unsigned int ipraw = MakeIP(stmp.c_str());
    unsigned int ipmask = atoi(smask.c_str());
    if( ipraw == 0 || ipmask == 0 )
        return false;

    IPBan ipb;
    ipb.db_ip = sip;
    ipb.Bytes = ipmask;
    ipb.Mask = ipraw;
    
    listBusy.Acquire();
    banList.push_back(ipb);
    listBusy.Release();

    return true;
}

bool IPBanner::Remove(const char * ip)
{
    listBusy.Acquire();
    for(std::list<IPBan>::iterator itr = banList.begin(); itr != banList.end(); ++itr)
    {
        if( !strcmp(ip, itr->db_ip.c_str()) )
        {
            banList.erase(itr);
            listBusy.Release();
            return true;
        }
    }
    listBusy.Release();
    return false;
}

void IPBanner::Reload()
{

    listBusy.Acquire();
    banList.clear();
    QueryResult * result = sLogonSQL->Query("SELECT ip, expire FROM ipbans");
    if( result != NULL )
    {
        do 
        {
            IPBan ipb;
            std::string smask= "32";
            std::string ip = result->Fetch()[0].GetString();
            std::string::size_type i = ip.find("/");
            std::string stmp = ip.substr(0, i);
            if( i == std::string::npos )
            {
                printf("IP ban \"%s\" netmask not specified. assuming /32 \n", ip.c_str());
            }
            else
                smask = ip.substr(i+1);

            unsigned int ipraw = MakeIP(stmp.c_str());
            unsigned int ipmask = atoi(smask.c_str());
            if( ipraw == 0 || ipmask == 0 )
            {
                printf("IP ban \"%s\" could not be parsed. Ignoring\n", ip.c_str());
                continue;
            }

            ipb.Bytes = ipmask;
            ipb.Mask = ipraw;
            ipb.Expire = result->Fetch()[1].GetUInt32();
            ipb.db_ip = ip;
            banList.push_back(ipb);

        } while (result->NextRow());
        delete result;
    }
    listBusy.Release();
}

InformationCore::InformationCore() : realmhigh(0)
{
    usepings = !mainIni->ReadBoolean("LogonServer", "DisablePings", false);
}

Realm * InformationCore::AddRealm(uint32 realm_id, Realm * rlm)
{
    realmLock.Acquire();
    m_realms.insert( std::make_pair( realm_id, rlm ) );
    realmLock.Release();
    return rlm;
}

Realm * InformationCore::GetRealm(uint32 realm_id)
{
    Realm * ret = NULL;
    realmLock.Acquire();
    std::map<uint32, Realm*>::iterator itr = m_realms.find(realm_id);
    if(itr != m_realms.end())
        ret = itr->second;
    realmLock.Release();
    return ret;
}

int32 InformationCore::GetRealmIdByName(std::string Name)
{
    std::map<uint32, Realm*>::iterator itr = m_realms.begin();
    for(; itr != m_realms.end(); ++itr)
    {
        if (itr->second->Name == Name)
        {
            return itr->first;
        }
    }
    return -1;
}

void InformationCore::RemoveRealm(uint32 realm_id)
{
    realmLock.Acquire();
    std::map<uint32, Realm*>::iterator itr = m_realms.find(realm_id);
    if(itr != m_realms.end())
    {
        delete itr->second;
        m_realms.erase(itr);
    }
    realmLock.Release();
}

bool InformationCore::FindRealmWithAdress(std::string Address)
{
    std::map<uint32, Realm*>::iterator itr = m_realms.begin();
    for(; itr != m_realms.end(); itr++)
    {
        if (itr->second->Address == Address)
            return true;
    }
    return false;
}

void InformationCore::UpdateRealmPop(uint32 realm_id, uint32 population)
{
    realmLock.Acquire();
    std::map<uint32, Realm*>::iterator itr = m_realms.find(realm_id);
    if(itr != m_realms.end())
    {
        uint8 temp;
        uint32 color;
        float pop = float(population)/float(itr->second->RealmCap);
        if(pop == 1)
        {
            color = REALM_POP_FULL;
            temp = REALM_FLAG_FULL|REALM_FLAG_INVALID;
        }
        else if ( pop >= 0.6 )
        {
            color = REALM_POP_MEDIUM;
            temp = REALM_FLAG_NONE;
        }
        else if ( pop >= 0.2 )
        {
            color = REALM_POP_NEW;
            temp = REALM_FLAG_NEW;
        }
        else
        {
            color = REALM_POP_NEW_PLAYERS;
            temp = REALM_FLAG_RECOMMENDED;
        }

        itr->second->RealPopulation = population;
        itr->second->Population = color;
        itr->second->Flag = temp;
    }
    realmLock.Release();
}

void InformationCore::SendRealms(AuthSocket * Socket)
{
    Realm* realm = NULL;
    realmLock.Acquire();
    HM_NAMESPACE::hash_map<uint32, uint8>::iterator it;
    std::map<uint32, Realm*>::iterator itr = m_realms.begin();
    if(Socket->GetBuild() <= 6005) // PreBC
    {
        // packet header
        ByteBuffer data(m_realms.size() * 150 + 20);
        data << uint8(0x10);
        data << uint16(0);    // Size Placeholder

        // dunno what this is..
        data << uint32(0);

        // loop realms :/
        data << uint8(m_realms.size());
        for(; itr != m_realms.end(); ++itr)
        {
            realm = itr->second;

            data << uint32(realm->Icon);
            uint8 realmflags = realm->Flag;
            if(realm->RequiredBuild)
            {
                if(realm->RequiredBuild != Socket->GetBuild())
                    realmflags = REALM_FLAG_OFFLINE;
            }

            // This part is the same for all.
            data << realmflags;
            data << realm->Name;
            data << realm->Address;
            data << realm->Population;

            /* Get our character count */
            it = realm->CharacterMap.find(Socket->GetAccountID());
            data << uint8( (it == realm->CharacterMap.end()) ? 0 : it->second ); //We can fix this later, character count. 
            data << uint8(1);
            data << uint8(0);
        }
        realmLock.Release();

        data << uint8(0x15);
        data << uint8(0);

        // Re-calculate size.
#ifdef USING_BIG_ENDIAN
        *(uint16*)&data.contents()[1] = swap16(uint16(data.size() - 3));
#else
        *(uint16*)&data.contents()[1] = uint16(data.size() - 3);
#endif

        // Send to the socket.
        Socket->Send((const uint8*)data.contents(), uint32(data.size()));
        return;
    }

    // packet header
    ByteBuffer data(m_realms.size() * 150 + 20);
    data << uint8(0x10);
    data << uint16(0);    // Size Placeholder

    // dunno what this is..
    data << uint32(0);
    size_t count_pos = data.wpos();
    uint16 count = uint16(m_realms.size());
    data << uint16(count);

    // loop realms :/
    for(; itr != m_realms.end(); ++itr)
    {
        realm = itr->second;

        data << realm->Icon;
        uint8 flag = realm->Flag;
        if(realm->RequiredBuild && realm->RequiredBuild != Socket->GetBuild())
            flag = (REALM_FLAG_SPECIFYBUILD|REALM_FLAG_OFFLINE);

        data << uint8(0);
        data << flag;
        data << realm->Name;
        data << realm->Address;
        data << realm->Population;

        /* Get our character count */
        it = realm->CharacterMap.find(Socket->GetAccountID());
        data << uint8( (it == realm->CharacterMap.end()) ? 0 : it->second );
        data << realm->WorldRegion;
        data << uint8(GetRealmIdByName(realm->Name));       //Realm ID
        if(flag & REALM_FLAG_SPECIFYBUILD)
        {
            data << uint8(realm->RequiredCV[0]);
            data << uint8(realm->RequiredCV[1]);
            data << uint8(realm->RequiredCV[2]);
            data << uint16(realm->RequiredBuild);
        }
        realm = NULL;
    }
    realmLock.Release();

    data.put<uint16>(count_pos, count);

    data << uint8(0x17);
    data << uint8(0);

    // Re-calculate size.
    *(uint16*)&data.contents()[1] = uint16(data.size() - 3);

    // Send to the socket.
    Socket->Send((const uint8*)data.contents(), uint32(data.size()));
}

void InformationCore::TimeoutSockets()
{
    if(usepings == false)
        return;

    /* burlex: this is vulnerable to race conditions, adding a mutex to it. */
    serverSocketLock.Acquire();

    // check the ping time
    uint32 t = getMSTime();

    std::set<LogonCommServerSocket*>::iterator itr, it2;
    LogonCommServerSocket * s;
    for(itr = m_serverSockets.begin(); itr != m_serverSockets.end();)
    {
        s = *itr;
        it2 = itr++;
        if(s->removed || !s->authenticated)
            continue;

        if(t - s->last_pong > (15000+s->latency))
        {
            // ping timeout
            s->removed = true;
            SetRealmOffline(s->realmID, s);
            m_serverSockets.erase(it2);
            s->Disconnect();
            continue;
        }

        // Send ping and refresh pop
        s->SendDataPing();
    }

    serverSocketLock.Release();
}
void InformationCore::SetRealmOffline(uint32 realm_id, LogonCommServerSocket *ss)
{
    realmLock.Acquire();
    std::map<uint32,Realm*>::iterator itr = m_realms.find( realm_id );
    if( itr != m_realms.end() )
    {
        Realm * pr = itr->second;
        if( pr->ServerSocket == ss )
        {
            sLog.Debug("LogonServer","Realm: %s is now offline.\n", pr->Name.c_str());
            pr->Flag = REALM_FLAG_OFFLINE;

            // clear the mapping, when its re-registered it will send it again
            pr->m_charMapLock.Acquire();
            pr->CharacterMap.clear();
            pr->m_charMapLock.Release();
            pr->ServerSocket = NULL;
        }
    }
    realmLock.Release();
}

void InformationCore::CheckServers()
{
    serverSocketLock.Acquire();

    std::set<LogonCommServerSocket*>::iterator itr, it2;
    LogonCommServerSocket * s;
    for(itr = m_serverSockets.begin(); itr != m_serverSockets.end();)
    {
        s = *itr;
        it2 = itr;
        ++itr;

        if(!IsServerAllowed(s->GetRemoteAddress().s_addr))
        {
            sLog.Debug("LogonServer","Disconnecting socket: %s due to it no longer being on an allowed IP.\n", s->GetIP());
            s->Disconnect();
        }
    }

    serverSocketLock.Release();
}
