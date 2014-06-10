/***
 * Demonstrike Core
 */

//
// WorldSession.cpp
//

#include "StdAfx.h"

extern bool bServerShutdown;
#define WORLDSOCKET_TIMEOUT 80
OpcodeHandler WorldPacketHandlers[NUM_MSG_TYPES];

WorldSession::WorldSession(uint32 id, string Name, WorldSocket *sock) : EventableObject(), _socket(sock), _accountId(id), _accountName(Name),
_logoutTime(0), permissions(NULL), permissioncount(0), _loggingOut(false), instanceId(0), _recentlogout(false)
{
    _player = NULLPLR;
    m_hasDeathKnight = false;
    m_highestLevel = sWorld.StartLevel;
    m_asyncQuery = false;
    m_currMsTime = getMSTime();
    bDeleted = false;
    m_bIsWLevelSet = false;
    floodLines = 0;
    floodTime = UNIXTIME;
    _updatecount = 0;
    m_moveDelayTime=0;
    m_clientTimeDelay =0;
    m_loggingInPlayer=NULLPLR;
    m_muted = 0;
    m_repeatTime = 0;
    m_repeatEmoteTime = 0;
    m_repeatEmoteId = 0;
    m_lastWhoTime = 0;
    m_isFalling = false;
    m_isJumping = false;
    m_isKnockedback = false;
    m_jumpHackChances = 5;

    for(uint32 x = 0; x < 8; x++)
        m_accountData[x] = NULL;
}

WorldSession::~WorldSession()
{
    deleteMutex.Acquire();

    if(_player != NULL)
    {
        printf("warning: logged out player in worldsession destructor");
        LogoutPlayer(true);
    }

    if(permissions)
        delete [] permissions;

    WorldPacket *packet;

    while((packet = _recvQueue.Pop()))
        delete packet;

    for(uint32 x= 0;x<8;x++)
        if(m_accountData[x]) delete m_accountData[x];

    if(_socket)
        _socket->SetSession(0);

    if(m_loggingInPlayer)
    {
        m_loggingInPlayer->SetSession(NULL);
        m_loggingInPlayer = NULLPLR;
    }

    deleteMutex.Release();
}

int WorldSession::Update(uint32 InstanceID)
{
    m_currMsTime = getMSTime();

    // If we have no player and no socket, delete us
    if(_player == NULL && _socket == NULL)
        return 1;

    // We're being updated by the wrong thread, remove us
    if(InstanceID != instanceId)
        return 2;

    // Update our queued packets
    if(!((++_updatecount) % 2) && _socket)
        _socket->UpdateQueuedPackets();

    // Socket disconnection.
    if(!_socket)
    {
        // Check if the player is in the process of being moved. We can't delete him
        // if we are.
        if(_player && _player->m_beingPushed) // check timeout
        {
            //Timeout client after 2 minutes, in case AddToWorld failed (f.e. client crash)
            if(  (uint32)UNIXTIME - m_lastPing > 120 )
            {
                sLog.Debug("WorldSession","Removing InQueue player due to socket timeout.");
                LogoutPlayer(true);
                bDeleted = true;
                return 1;
            }
            return 0;
        }
        if(!_logoutTime)
            SetLogoutTimer(PLAYER_LOGOUT_DELAY);
    }

    if(_recvQueue.HasItems())
    {
        WorldPacket *packet;
        OpcodeHandler * Handler;
        while (_socket && (packet = _recvQueue.Pop()))
        {
            ASSERT(packet);

            if(packet->GetOpcode() >= NUM_MSG_TYPES)
                sLog.Error("WorldSession", "Received out of range packet with opcode 0x%.4X", packet->GetOpcode());
            else
            {
                Handler = &WorldPacketHandlers[packet->GetOpcode()];
                if(Handler->status != STATUS_IGNORED)
                {
                    if(Handler->status == STATUS_LOGGEDIN && !_player && Handler->handler != 0)
                    {
                        sLog.Warning("WorldSession", "Received unexpected/wrong state packet(Logged In) with opcode %s (0x%.4X)",
                            sOpcodeMgr.GetOpcodeName(packet->GetOpcode()), packet->GetOpcode());
                    }
                    else if(Handler->status == STATUS_IN_OR_LOGGINGOUT && !_player && !_recentlogout && Handler->handler != 0)
                    {
                        sLog.Warning("WorldSession", "Received unexpected/wrong state packet(In or Out) with opcode %s (0x%.4X)",
                            sOpcodeMgr.GetOpcodeName(packet->GetOpcode()), packet->GetOpcode());
                    }
                    else
                    {
                        // Valid Packet :>
                        if(Handler->handler == 0)
                            sLog.Warning("WorldSession", "Received unhandled packet with opcode %s (0x%.4X)", sOpcodeMgr.GetOpcodeName(packet->GetOpcode()), packet->GetOpcode());
                        else
                        {
                            bool fail = false;
                            packet->opcodename = sOpcodeMgr.GetOpcodeName(packet->GetOpcode()); // Needed for ByteBuffer
                            try
                            {
                                (this->*Handler->handler)(*packet);
                            }
                            catch (ByteBufferException &)
                            {
                                fail = true;
                                sLog.Error("WorldSession", "Incorrect handling of opcode %s (0x%.4X) REPORT TO DEVS", sOpcodeMgr.GetOpcodeName(packet->GetOpcode()), packet->GetOpcode());
                            }

                            if(!fail && sLog.isOutProcess() && (packet->rpos() < packet->wpos()))
                                LogUnprocessedTail(packet);

                            if(Handler->status == STATUS_AUTHED)
                                if(packet->GetOpcode() != CMSG_SET_ACTIVE_VOICE_CHANNEL)
                                    _recentlogout = false;
                        }
                    }
                }
            }

            delete packet;
            packet = NULL;
            if(InstanceID != instanceId)
                return 2; // If we hit this it means that an opcode has changed our map.

            if( bDeleted )
                return 1;
        }
    }

    if(InstanceID != instanceId)
        return 2; // If we hit this it means that an opcode has changed our map.

    if( _logoutTime && (m_currMsTime >= _logoutTime) && instanceId == InstanceID)
    {
        // Check if the player is in the process of being moved. We can't delete him
        // if we are.
        if(_player && _player->m_beingPushed)
        {
            //Timeout client after 2 minutes, in case AddToWorld failed (f.e. client crash)
            if(  (uint32)UNIXTIME - m_lastPing > 120 )
            {
                sLog.Debug("WorldSession","Removing InQueue player due to socket timeout.");
                LogoutPlayer(true);
                bDeleted = true;
                return 1;
            }
            // Abort..
            return 0;
        }

        LogoutPlayer(true);
        if( _socket == NULL )
        {
            bDeleted = true;
            return 1;
        }
    }

    if(m_lastPing + WORLDSOCKET_TIMEOUT < (uint32)UNIXTIME)
    {
        // Check if the player is in the process of being moved. We can't delete him
        // if we are.
        if(_player && _player->m_beingPushed)
            return 0; // Abort

        // ping timeout!
        if( _socket != NULL )
        {
            Disconnect();
            _socket = NULL;
        }

        m_lastPing = (uint32)UNIXTIME;      // Prevent calling this code over and over.
        if(!_logoutTime)
            SetLogoutTimer(PLAYER_LOGOUT_DELAY);
    }

    return 0;
}

bool WorldSession::IsHighPriority()
{
    bool res = false;
    if(m_loggingInPlayer)
        res = true;
    else if(_player)
    {
        if(_player->m_beingPushed)
            res = true;
        else if(_player->m_changingMaps)
            res = true;
        else if(!_player->IsInWorld())
            res = true;
    }
    return res;
}

void WorldSession::LogoutPlayer(bool Save)
{
    _updatecount = 0;
    if( _loggingOut )
        return;

    _loggingOut = true;
    _recentlogout = true;

    if( _player != NULL )
    {
        _player->ObjLock();
        if(_player == NULL)
        {
            _player->ObjUnlock();
            return;
        }

        sHookInterface.OnLogout( _player );

        sEventMgr.RemoveEvents(_player);

        //Duel Cancel on Leave
        if( _player->DuelingWith != NULL )
            _player->EndDuel( DUEL_WINNER_RETREAT );

        if( _player->m_currentLoot && _player->IsInWorld() )
        {
            Object* obj = _player->GetMapMgr()->_GetObject( _player->m_currentLoot );
            if( obj != NULL )
                obj->m_loot.looters.erase(_player->GetLowGUID());
            obj = NULLOBJ;
        }

#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
        GM_Ticket * ticket = objmgr.GetGMTicketByPlayer(_player->GetGUID());
        if(ticket != NULL)
        {
            //Send status change to gm_sync_channel
            Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), _player);
            if(chn)
            {
                std::stringstream ss;
                ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_ONLINESTATE;
                ss << ":" << ticket->guid;
                ss << ":0";
                chn->Say(_player, ss.str().c_str(), NULLPLR, true);
            }
        }
#endif

        // part channels
        _player->CleanupChannels();

        // Remove from vehicle for now.
        if(_player->GetVehicle(true))
            TO_VEHICLE(_player->GetVehicle())->RemovePassenger(_player);

        if( _player->m_CurrentTransporter != NULL )
        {
            _player->m_CurrentTransporter->RemovePlayer( _player );
            _player->m_CurrentTransporter = NULLTRANSPORT;
            _player->GetMovementInfo()->ClearTransportData();
        }

        // cancel current spell
        if( _player->m_currentSpell != NULL )
            _player->m_currentSpell->cancel();

        _player->Social_TellOnlineStatus(false);

        if( _player->GetTeam() == 1 )
        {
            if( sWorld.HordePlayers )
                sWorld.HordePlayers--;
        }
        else
        {
            if( sWorld.AlliancePlayers )
                sWorld.AlliancePlayers--;
        }

        if( _player->m_bg )
            _player->m_bg->RemovePlayer( _player, true );

        _player->RemoveFromBattlegroundQueue(0); // Pending BGs
        _player->RemoveFromBattlegroundQueue(1); // Pending BGs
        BattlegroundManager.RemovePlayerFromQueues( _player );

        //Issue a message telling all guild members that this player signed off
        guildmgr.PlayerLoggedOff(_player->getPlayerInfo());

        _player->GetItemInterface()->EmptyBuyBack();

        sLfgMgr.RemovePlayerFromLfgQueues( _player );

        // Save HP/Mana
        _player->load_health = _player->GetUInt32Value( UNIT_FIELD_HEALTH );
        _player->load_mana = _player->GetUInt32Value( UNIT_FIELD_POWER1 );

        objmgr.RemovePlayer( _player );
        _player->ok_to_remove = true;

        if( _player->GetSummon() != NULL )
            _player->GetSummon()->Remove( false, true, false );

        if( Save )
            _player->SaveToDB(false);

        // send to gms
        if( HasGMPermissions() && !bServerShutdown )
            sWorld.SendMessageToGMs(this, "GM %s (%s) is now offline. (Permissions: [%s])", _player->GetName(), GetAccountNameS(), GetPermissions());

        _player->m_AuraInterface.RemoveAllAuras();
        if( _player->IsInWorld() )
            _player->RemoveFromWorld();

        _player->m_playerInfo->m_loggedInPlayer = NULLPLR;

        if(!bServerShutdown) // Save our groups for the next startup.
        {
            if(_player->GetGroup()) // Init group logout checks.
            {
                // Remove player from the group if he is in a group and not in a raid.
                if(!(_player->GetGroup()->GetGroupType() & GROUP_TYPE_RAID) && _socket && (_player->GetGroup()->GetOnlineMemberCount() == 0))
                    _player->GetGroup()->Disband();
                else
                    _player->m_playerInfo->m_Group->Update();
            }
        }

        // Remove the "player locked" flag, to allow movement on next login
        GetPlayer()->RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER );

        // Update Tracker status
        sTracker.CheckPlayerForTracker(_player, false);

        // Save our tutorials
        SaveTutorials();

        // Save our account data, if we have any
        SaveAccountData();

        _player->ObjUnlock();

        _player->Destruct();
        _player = NULLPLR;

        OutPacket(SMSG_LOGOUT_COMPLETE, 0, NULL);
        sLog.Debug( "WorldSession","Sent SMSG_LOGOUT_COMPLETE Message" );
    }
    _loggingOut = false;

    SetLogoutTimer(0);
    if(_player != NULL)
        _player = NULLPLR;
}

void WorldSession::SendBuyFailed(uint64 guid, uint32 itemid, uint8 error)
{
    WorldPacket data(SMSG_BUY_FAILED, 13);
    data << guid << itemid << error;
    SendPacket(&data);
}

void WorldSession::SendSellItem(uint64 vendorguid, uint64 itemid, uint8 error)
{
    WorldPacket data(SMSG_SELL_ITEM, 17);
    data << vendorguid << itemid << error;
    SendPacket(&data);
}

void WorldSession::LoadSecurity(std::string securitystring)
{
    std::list<char> tmp;
    bool hasa = false;
    for(uint32 i = 0; i < securitystring.length(); i++)
    {
        char c = securitystring.at(i);
        c = tolower(c);
        if(c == '4' || c == '3')
            c = 'a';    // for the lazy people

        if(c == 'a')
        {
            // all permissions
            tmp.push_back('a');
            hasa = true;
        }
        else if(!hasa && (c == '0') && i == 0)
            break;
        else if(!hasa || (hasa && (c == 'z')))
        {
            tmp.push_back(c);
        }
    }

    permissions = new char[tmp.size()+1];
    memset(permissions, 0, tmp.size()+1);
    permissioncount = (uint32)tmp.size();
    int k = 0;
    for(std::list<char>::iterator itr = tmp.begin(); itr != tmp.end(); itr++)
        permissions[k++] = (*itr);

    if(permissions[tmp.size()] != 0)
        permissions[tmp.size()] = 0;

    sLog.Debug("WorldSession","Loaded permissions for %u. (%u) : [%s]", GetAccountId(), permissioncount, securitystring.c_str());
}

void WorldSession::SetSecurity(std::string securitystring)
{
    delete [] permissions;
    LoadSecurity(securitystring);

    // update db
    CharacterDatabase.Execute("UPDATE accounts SET gm=\'%s\' WHERE acct=%u", CharacterDatabase.EscapeString(string(permissions)).c_str(), _accountId);
}

bool WorldSession::CanUseCommand(char cmdstr)
{
    if(permissioncount == 0)
        return false;
    if(cmdstr == 0)
        return true;
    if(permissions[0] == 'a' && cmdstr != 'z')   // all
        return true;

    for(int i = 0; i < permissioncount; i++)
        if(permissions[i] == cmdstr)
            return true;

    return false;
}

void WorldSession::SendNotification(const char *message, ...)
{
    if( !message ) return;
    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1,1024, message,ap);
    WorldPacket data(SMSG_NOTIFICATION, strlen(msg1) + 1);
    data << msg1;
    SendPacket(&data);
}

void WorldSession::InitPacketHandlerTable()
{
    // Nullify Everything, default to STATUS_LOGGEDIN
    for(uint32 i = 0; i < NUM_MSG_TYPES; i++)
    {
        WorldPacketHandlers[i].status = STATUS_LOGGEDIN;
        WorldPacketHandlers[i].handler = NULL;
    }

    // Login
    WorldPacketHandlers[CMSG_CHARACTER_ENUM].handler                        = &WorldSession::HandleCharEnumOpcode;
    WorldPacketHandlers[CMSG_CHARACTER_ENUM].status                         = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHARACTER_CREATE].handler                      = &WorldSession::HandleCharCreateOpcode;
    WorldPacketHandlers[CMSG_CHARACTER_CREATE].status                       = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHARACTER_DELETE].handler                      = &WorldSession::HandleCharDeleteOpcode;
    WorldPacketHandlers[CMSG_CHARACTER_DELETE].status                       = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHARACTER_RENAME].handler                      = &WorldSession::HandleCharRenameOpcode;
    WorldPacketHandlers[CMSG_CHARACTER_RENAME].status                       = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHARACTER_CUSTOMIZE].handler                   = &WorldSession::HandleCharCustomizeOpcode;
    WorldPacketHandlers[CMSG_CHARACTER_CUSTOMIZE].status                    = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_PLAYER_LOGIN].handler                          = &WorldSession::HandlePlayerLoginOpcode;
    WorldPacketHandlers[CMSG_PLAYER_LOGIN].status                           = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_WORLD_LOGIN].handler                           = &WorldSession::HandleWorldLoginOpcode;
    WorldPacketHandlers[CMSG_WORLD_LOGIN].status                            = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_UPDATE_OBJECT_REQUEST].handler                 = &WorldSession::HandleObjectUpdateRequest;

    // Account Data
    WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].handler                   = &WorldSession::HandleUpdateAccountData;
    WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].status                    = STATUS_AUTHED;
    WorldPacketHandlers[CMSG_REQUEST_ACCOUNT_DATA].handler                  = &WorldSession::HandleRequestAccountData;
    WorldPacketHandlers[CMSG_REQUEST_ACCOUNT_DATA].status                   = STATUS_AUTHED;
    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler          = &WorldSession::HandleReadyForAccountDataTimes;
    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status           = STATUS_AUTHED;

    // Queries
    WorldPacketHandlers[MSG_CORPSE_QUERY].handler                           = &WorldSession::HandleCorpseQueryOpcode;
    WorldPacketHandlers[CMSG_NAME_QUERY].handler                            = &WorldSession::HandleNameQueryOpcode;
    WorldPacketHandlers[CMSG_QUERY_TIME].handler                            = &WorldSession::HandleQueryTimeOpcode;
    WorldPacketHandlers[CMSG_CREATURE_QUERY].handler                        = &WorldSession::HandleCreatureQueryOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJECT_QUERY].handler                      = &WorldSession::HandleGameObjectQueryOpcode;
    WorldPacketHandlers[CMSG_PAGE_TEXT_QUERY].handler                       = &WorldSession::HandlePageTextQueryOpcode;
    WorldPacketHandlers[CMSG_QUEST_QUERY].handler                           = &WorldSession::HandleQuestQueryOpcode;

    WorldPacketHandlers[CMSG_REQUEST_HOTFIX].handler                        = &WorldSession::HandleItemHotfixQueryOpcode;
    WorldPacketHandlers[CMSG_REQUEST_HOTFIX].status                         = STATUS_WHENEVER;

    // Movement
    WorldPacketHandlers[CMSG_MOVE_FALL_RESET].handler                       = &WorldSession::HandleMoveFallResetOpcode;
    WorldPacketHandlers[MSG_MOVE_HEARTBEAT].handler                         = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_WORLDPORT_ACK].handler                     = &WorldSession::HandleMoveWorldportAckOpcode;
    WorldPacketHandlers[MSG_MOVE_JUMP].handler                              = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_ASCEND].handler                      = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler                       = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_DESCEND].handler                     = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_FORWARD].handler                     = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_BACKWARD].handler                    = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_FACING].handler                        = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_STRAFE_LEFT].handler                 = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_STRAFE_RIGHT].handler                = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_STRAFE].handler                       = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_TURN_LEFT].handler                   = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_TURN_RIGHT].handler                  = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_TURN].handler                         = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_PITCH_UP].handler                    = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_PITCH_DOWN].handler                  = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_PITCH].handler                        = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_RUN_MODE].handler                      = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_WALK_MODE].handler                     = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_PITCH].handler                         = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_SWIM].handler                        = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_SWIM].handler                         = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_FALL_LAND].handler                         = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP].handler                              = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_SET_FLY].handler                          = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_NOT_ACTIVE_MOVER].handler                 = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_CHNG_TRANSPORT].handler                   = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_TIME_SKIPPED].handler                     = &WorldSession::HandleMoveTimeSkippedOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIVE_MOVER].handler                      = &WorldSession::HandleSetActiveMoverOpcode;

    // ACK
    WorldPacketHandlers[MSG_MOVE_TELEPORT_ACK].handler                      = &WorldSession::HandleMoveTeleportAckOpcode;
    WorldPacketHandlers[CMSG_MOVE_HOVER_ACK].handler                        = &WorldSession::HandleMoveHoverWaterFlyAckOpcode;
    WorldPacketHandlers[CMSG_MOVE_WATER_WALK_ACK].handler                   = &WorldSession::HandleMoveHoverWaterFlyAckOpcode;
    WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY_ACK].handler                  = &WorldSession::HandleMoveHoverWaterFlyAckOpcode;
    WorldPacketHandlers[CMSG_MOVE_KNOCK_BACK_ACK].handler                   = &WorldSession::HandleMoveKnockbackAckOpcode;
    WorldPacketHandlers[CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK].handler      = &WorldSession::HandleForceSpeedChangeOpcodes;
    WorldPacketHandlers[CMSG_FORCE_TURN_RATE_CHANGE_ACK].handler            = &WorldSession::HandleForceSpeedChangeOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_SPEED_CHANGE_ACK].handler            = &WorldSession::HandleForceSpeedChangeOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK].handler       = &WorldSession::HandleForceSpeedChangeOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_SPEED_CHANGE_ACK].handler           = &WorldSession::HandleForceSpeedChangeOpcodes;
    WorldPacketHandlers[CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK].handler         = &WorldSession::HandleForceSpeedChangeOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_ROOT_ACK].handler                   = &WorldSession::HandleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_UNROOT_ACK].handler                 = &WorldSession::HandleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_FEATHER_FALL_ACK].handler                 = &WorldSession::HandleAcknowledgementOpcodes;

    // Action Buttons
    WorldPacketHandlers[CMSG_SET_ACTION_BUTTON].handler                      = &WorldSession::HandleSetActionButtonOpcode;
    WorldPacketHandlers[CMSG_REPOP_REQUEST].handler                          = &WorldSession::HandleRepopRequestOpcode;

    // Loot
    WorldPacketHandlers[CMSG_AUTOSTORE_LOOT_ITEM].handler                   = &WorldSession::HandleAutostoreLootItemOpcode;
    WorldPacketHandlers[CMSG_LOOT_MONEY].handler                            = &WorldSession::HandleLootMoneyOpcode;
    WorldPacketHandlers[CMSG_LOOT].handler                                  = &WorldSession::HandleLootOpcode;
    WorldPacketHandlers[CMSG_LOOT_RELEASE].handler                          = &WorldSession::HandleLootReleaseOpcode;
    WorldPacketHandlers[CMSG_LOOT_ROLL].handler                             = &WorldSession::HandleLootRollOpcode;
    WorldPacketHandlers[CMSG_LOOT_MASTER_GIVE].handler                      = &WorldSession::HandleLootMasterGiveOpcode;

    // Player Interaction
    WorldPacketHandlers[CMSG_WHO].handler                                   = &WorldSession::HandleWhoOpcode;
    WorldPacketHandlers[CMSG_LOGOUT_REQUEST].handler                        = &WorldSession::HandleLogoutRequestOpcode;
    WorldPacketHandlers[CMSG_PLAYER_LOGOUT].handler                         = &WorldSession::HandlePlayerLogoutOpcode;
    WorldPacketHandlers[CMSG_LOGOUT_CANCEL].handler                         = &WorldSession::HandleLogoutCancelOpcode;
    WorldPacketHandlers[CMSG_ZONEUPDATE].handler                            = &WorldSession::HandleZoneUpdateOpcode;
    WorldPacketHandlers[CMSG_SET_SELECTION].handler                         = &WorldSession::HandleSetSelectionOpcode;
    WorldPacketHandlers[CMSG_STANDSTATECHANGE].handler                      = &WorldSession::HandleStandStateChangeOpcode;
    WorldPacketHandlers[CMSG_CANCEL_MOUNT_AURA].handler                     = &WorldSession::HandleDismountOpcode;

    // Friends
    WorldPacketHandlers[CMSG_CONTACT_LIST].handler                          = &WorldSession::HandleFriendListOpcode;
    WorldPacketHandlers[CMSG_ADD_FRIEND].handler                            = &WorldSession::HandleAddFriendOpcode;
    WorldPacketHandlers[CMSG_DEL_FRIEND].handler                            = &WorldSession::HandleDelFriendOpcode;
    WorldPacketHandlers[CMSG_ADD_IGNORE].handler                            = &WorldSession::HandleAddIgnoreOpcode;
    WorldPacketHandlers[CMSG_DEL_IGNORE].handler                            = &WorldSession::HandleDelIgnoreOpcode;
    WorldPacketHandlers[CMSG_BUG].handler                                   = &WorldSession::HandleBugOpcode;
    WorldPacketHandlers[CMSG_SET_CONTACT_NOTES].handler                     = &WorldSession::HandleSetFriendNote;

    // Misc
    WorldPacketHandlers[CMSG_AREATRIGGER].handler                           = &WorldSession::HandleAreaTriggerOpcode;
    WorldPacketHandlers[CMSG_SET_FACTION_ATWAR].handler                     = &WorldSession::HandleSetAtWarOpcode;
    WorldPacketHandlers[CMSG_SET_WATCHED_FACTION].handler                   = &WorldSession::HandleSetWatchedFactionIndexOpcode;
    WorldPacketHandlers[CMSG_TOGGLE_PVP].handler                            = &WorldSession::HandleTogglePVPOpcode;

    // Player Interaction
    WorldPacketHandlers[CMSG_GAMEOBJ_USE].handler                           = &WorldSession::HandleGameObjectUse;
    WorldPacketHandlers[CMSG_PLAYED_TIME].handler                           = &WorldSession::HandlePlayedTimeOpcode;
    WorldPacketHandlers[CMSG_SETSHEATHED].handler                           = &WorldSession::HandleSetSheathedOpcode;
    WorldPacketHandlers[CMSG_TEXT_EMOTE].handler                            = &WorldSession::HandleTextEmoteOpcode;
    WorldPacketHandlers[CMSG_INSPECT].handler                               = &WorldSession::HandleInspectOpcode;
    WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler                        = &WorldSession::HandleTimeSyncResp;
    WorldPacketHandlers[CMSG_GAMEOBJ_REPORT_USE].handler                    = &WorldSession::HandleGameobjReportUseOpCode;

    // Chat Interaction
    WorldPacketHandlers[CMSG_MESSAGECHAT_SAY].handler                       = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_YELL].handler                      = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_CHANNEL].handler                   = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_WHISPER].handler                   = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_GUILD].handler                     = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_OFFICER].handler                   = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_AFK].handler                       = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_DND].handler                       = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_EMOTE].handler                     = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_PARTY].handler                     = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_PARTY_LEADER].handler              = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_RAID].handler                      = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_RAID_LEADER].handler               = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_BATTLEGROUND].handler              = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_BATTLEGROUND_LEADER].handler       = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_RAID_WARNING].handler              = &WorldSession::HandleMessagechatOpcode;

    // Channels
    WorldPacketHandlers[CMSG_JOIN_CHANNEL].handler                          = &WorldSession::HandleChannelJoin;
    WorldPacketHandlers[CMSG_LEAVE_CHANNEL].handler                         = &WorldSession::HandleChannelLeave;
    WorldPacketHandlers[CMSG_CHANNEL_LIST].handler                          = &WorldSession::HandleChannelList;
    WorldPacketHandlers[CMSG_CHANNEL_PASSWORD].handler                      = &WorldSession::HandleChannelPassword;
    WorldPacketHandlers[CMSG_CHANNEL_SET_OWNER].handler                     = &WorldSession::HandleChannelSetOwner;
    WorldPacketHandlers[CMSG_CHANNEL_OWNER].handler                         = &WorldSession::HandleChannelOwner;
    WorldPacketHandlers[CMSG_CHANNEL_MODERATOR].handler                     = &WorldSession::HandleChannelModerator;
    WorldPacketHandlers[CMSG_CHANNEL_UNMODERATOR].handler                   = &WorldSession::HandleChannelUnmoderator;
    WorldPacketHandlers[CMSG_CHANNEL_MUTE].handler                          = &WorldSession::HandleChannelMute;
    WorldPacketHandlers[CMSG_CHANNEL_UNMUTE].handler                        = &WorldSession::HandleChannelUnmute;
    WorldPacketHandlers[CMSG_CHANNEL_INVITE].handler                        = &WorldSession::HandleChannelInvite;
    WorldPacketHandlers[CMSG_CHANNEL_KICK].handler                          = &WorldSession::HandleChannelKick;
    WorldPacketHandlers[CMSG_CHANNEL_BAN].handler                           = &WorldSession::HandleChannelBan;
    WorldPacketHandlers[CMSG_CHANNEL_UNBAN].handler                         = &WorldSession::HandleChannelUnban;
    WorldPacketHandlers[CMSG_CHANNEL_ANNOUNCEMENTS].handler                 = &WorldSession::HandleChannelAnnounce;
    WorldPacketHandlers[CMSG_CHANNEL_MODERATE].handler                      = &WorldSession::HandleChannelModerate;
    WorldPacketHandlers[CMSG_GET_CHANNEL_MEMBER_COUNT].handler              = &WorldSession::HandleChannelNumMembersQuery;
    WorldPacketHandlers[CMSG_CHANNEL_DISPLAY_LIST].handler                  = &WorldSession::HandleChannelRosterQuery;

    // Groups / Raids
    WorldPacketHandlers[CMSG_GROUP_INVITE].handler                          = &WorldSession::HandleGroupInviteOpcode;
    WorldPacketHandlers[CMSG_GROUP_CANCEL].handler                          = &WorldSession::HandleGroupCancelOpcode;
    WorldPacketHandlers[CMSG_GROUP_ACCEPT].handler                          = &WorldSession::HandleGroupAcceptOpcode;
    WorldPacketHandlers[CMSG_GROUP_DECLINE].handler                         = &WorldSession::HandleGroupDeclineOpcode;
    WorldPacketHandlers[CMSG_GROUP_UNINVITE].handler                        = &WorldSession::HandleGroupUninviteOpcode;
    WorldPacketHandlers[CMSG_GROUP_UNINVITE_GUID].handler                   = &WorldSession::HandleGroupUninviteGUIDOpcode;
    WorldPacketHandlers[CMSG_GROUP_SET_LEADER].handler                      = &WorldSession::HandleGroupSetLeaderOpcode;
    WorldPacketHandlers[CMSG_GROUP_DISBAND].handler                         = &WorldSession::HandleGroupDisbandOpcode;
    WorldPacketHandlers[CMSG_LOOT_METHOD].handler                           = &WorldSession::HandleLootMethodOpcode;
    WorldPacketHandlers[MSG_MINIMAP_PING].handler                           = &WorldSession::HandleMinimapPingOpcode;
    WorldPacketHandlers[CMSG_GROUP_RAID_CONVERT].handler                    = &WorldSession::HandleConvertGroupToRaidOpcode;
    WorldPacketHandlers[CMSG_GROUP_CHANGE_SUB_GROUP].handler                = &WorldSession::HandleGroupChangeSubGroup;
    WorldPacketHandlers[CMSG_GROUP_ASSISTANT_LEADER].handler                = &WorldSession::HandleGroupAssistantLeader;
    WorldPacketHandlers[CMSG_REQUEST_RAID_INFO].handler                     = &WorldSession::HandleRequestRaidInfoOpcode;
    WorldPacketHandlers[MSG_RAID_READY_CHECK].handler                       = &WorldSession::HandleReadyCheckOpcode;
    WorldPacketHandlers[MSG_RAID_TARGET_UPDATE].handler                     = &WorldSession::HandleSetPlayerIconOpcode;
    WorldPacketHandlers[CMSG_REQUEST_PARTY_MEMBER_STATS].handler            = &WorldSession::HandlePartyMemberStatsOpcode;
    WorldPacketHandlers[MSG_PARTY_ASSIGNMENT].handler                       = &WorldSession::HandleGroupPromote;

    // Grouping Search System
    WorldPacketHandlers[CMSG_LFD_PLAYER_LOCK_INFO_REQUEST].handler          = &WorldSession::HandleLFDPlrLockOpcode;
    WorldPacketHandlers[CMSG_LFD_PARTY_LOCK_INFO_REQUEST].handler           = &WorldSession::HandleLFDPartyLockOpcode;

    // Taxi / NPC Interaction
    WorldPacketHandlers[CMSG_ENABLETAXI].handler                            = &WorldSession::HandleTaxiQueryAvaibleNodesOpcode;
    WorldPacketHandlers[CMSG_TAXINODE_STATUS_QUERY].handler                 = &WorldSession::HandleTaxiNodeStatusQueryOpcode;
    WorldPacketHandlers[CMSG_TAXIQUERYAVAILABLENODES].handler               = &WorldSession::HandleTaxiQueryAvaibleNodesOpcode;
    WorldPacketHandlers[CMSG_ENABLETAXI].handler                            = &WorldSession::HandleTaxiQueryAvaibleNodesOpcode;
    WorldPacketHandlers[CMSG_ACTIVATETAXI].handler                          = &WorldSession::HandleActivateTaxiOpcode;
    WorldPacketHandlers[MSG_TABARDVENDOR_ACTIVATE].handler                  = &WorldSession::HandleTabardVendorActivateOpcode;
    WorldPacketHandlers[CMSG_BANKER_ACTIVATE].handler                       = &WorldSession::HandleBankerActivateOpcode;
    WorldPacketHandlers[CMSG_BUY_BANK_SLOT].handler                         = &WorldSession::HandleBuyBankSlotOpcode;
    WorldPacketHandlers[CMSG_TRAINER_LIST].handler                          = &WorldSession::HandleTrainerListOpcode;
    WorldPacketHandlers[CMSG_TRAINER_BUY_SPELL].handler                     = &WorldSession::HandleTrainerBuySpellOpcode;
    WorldPacketHandlers[CMSG_PETITION_SHOWLIST].handler                     = &WorldSession::HandleCharterShowListOpcode;
    WorldPacketHandlers[MSG_AUCTION_HELLO].handler                          = &WorldSession::HandleAuctionHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_HELLO].handler                          = &WorldSession::HandleGossipHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_SELECT_OPTION].handler                  = &WorldSession::HandleGossipSelectOptionOpcode;
    WorldPacketHandlers[CMSG_SPIRIT_HEALER_ACTIVATE].handler                = &WorldSession::HandleSpiritHealerActivateOpcode;
    WorldPacketHandlers[CMSG_NPC_TEXT_QUERY].handler                        = &WorldSession::HandleNpcTextQueryOpcode;
    WorldPacketHandlers[CMSG_BINDER_ACTIVATE].handler                       = &WorldSession::HandleBinderActivateOpcode;
    WorldPacketHandlers[CMSG_ACTIVATETAXIEXPRESS].handler                   = &WorldSession::HandleMultipleActivateTaxiOpcode;

    // Item / Vendors
    WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler                         = &WorldSession::HandleSwapInvItemOpcode;
    WorldPacketHandlers[CMSG_SWAP_ITEM].handler                             = &WorldSession::HandleSwapItemOpcode;
    WorldPacketHandlers[CMSG_DESTROYITEM].handler                           = &WorldSession::HandleDestroyItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler                        = &WorldSession::HandleAutoEquipItemOpcode;
    WorldPacketHandlers[CMSG_SELL_ITEM].handler                             = &WorldSession::HandleSellItemOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM].handler                              = &WorldSession::HandleBuyItemOpcode;
    WorldPacketHandlers[CMSG_LIST_INVENTORY].handler                        = &WorldSession::HandleListInventoryOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler                    = &WorldSession::HandleAutoStoreBagItemOpcode;
    WorldPacketHandlers[CMSG_BUYBACK_ITEM].handler                          = &WorldSession::HandleBuyBackOpcode;
    WorldPacketHandlers[CMSG_SPLIT_ITEM].handler                            = &WorldSession::HandleSplitOpcode;
    WorldPacketHandlers[CMSG_READ_ITEM].handler                             = &WorldSession::HandleReadItemOpcode;
    WorldPacketHandlers[CMSG_REPAIR_ITEM].handler                           = &WorldSession::HandleRepairItemOpcode;
    WorldPacketHandlers[CMSG_AUTOBANK_ITEM].handler                         = &WorldSession::HandleAutoBankItemOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BANK_ITEM].handler                   = &WorldSession::HandleAutoStoreBankItemOpcode;
    WorldPacketHandlers[CMSG_CANCEL_TEMP_ENCHANTMENT].handler               = &WorldSession::HandleCancelTemporaryEnchantmentOpcode;
    WorldPacketHandlers[CMSG_SOCKET_GEMS].handler                           = &WorldSession::HandleInsertGemOpcode;
    WorldPacketHandlers[CMSG_WRAP_ITEM].handler                             = &WorldSession::HandleWrapItemOpcode;

    // Spell System / Talent System
    WorldPacketHandlers[CMSG_USE_ITEM].handler                              = &WorldSession::HandleUseItemOpcode;
    WorldPacketHandlers[CMSG_CAST_SPELL].handler                            = &WorldSession::HandleCastSpellOpcode;
    WorldPacketHandlers[CMSG_CANCEL_CAST].handler                           = &WorldSession::HandleCancelCastOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AURA].handler                           = &WorldSession::HandleCancelAuraOpcode;
    WorldPacketHandlers[CMSG_CANCEL_CHANNELLING].handler                    = &WorldSession::HandleCancelChannellingOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AUTO_REPEAT_SPELL].handler              = &WorldSession::HandleCancelAutoRepeatSpellOpcode;
    WorldPacketHandlers[CMSG_LEARN_TALENT].handler                          = &WorldSession::HandleLearnTalentOpcode;
    WorldPacketHandlers[CMSG_LEARN_PREVIEW_TALENTS].handler                 = &WorldSession::HandleLearnPreviewTalents;
    WorldPacketHandlers[CMSG_UNLEARN_TALENTS].handler                       = &WorldSession::HandleUnlearnTalents;
    WorldPacketHandlers[MSG_TALENT_WIPE_CONFIRM].handler                    = &WorldSession::HandleTalentWipeConfirmOpcode;
    WorldPacketHandlers[CMSG_UNLEARN_SKILL].handler                         = &WorldSession::HandleUnlearnSkillOpcode;

    // Combat / Duel
    WorldPacketHandlers[CMSG_ATTACKSWING].handler                           = &WorldSession::HandleAttackSwingOpcode;
    WorldPacketHandlers[CMSG_ATTACKSTOP].handler                            = &WorldSession::HandleAttackStopOpcode;
    WorldPacketHandlers[CMSG_DUEL_ACCEPTED].handler                         = &WorldSession::HandleDuelAccepted;
    WorldPacketHandlers[CMSG_DUEL_CANCELLED].handler                        = &WorldSession::HandleDuelCancelled;

    // Trade
    WorldPacketHandlers[CMSG_INITIATE_TRADE].handler                        = &WorldSession::HandleInitiateTrade;
    WorldPacketHandlers[CMSG_BEGIN_TRADE].handler                           = &WorldSession::HandleBeginTrade;
    WorldPacketHandlers[CMSG_BUSY_TRADE].handler                            = &WorldSession::HandleBusyTrade;
    WorldPacketHandlers[CMSG_IGNORE_TRADE].handler                          = &WorldSession::HandleIgnoreTrade;
    WorldPacketHandlers[CMSG_ACCEPT_TRADE].handler                          = &WorldSession::HandleAcceptTrade;
    WorldPacketHandlers[CMSG_UNACCEPT_TRADE].handler                        = &WorldSession::HandleUnacceptTrade;
    WorldPacketHandlers[CMSG_CANCEL_TRADE].handler                          = &WorldSession::HandleCancelTrade;
    WorldPacketHandlers[CMSG_CANCEL_TRADE].status                           = STATUS_IN_OR_LOGGINGOUT;
    WorldPacketHandlers[CMSG_SET_TRADE_ITEM].handler                        = &WorldSession::HandleSetTradeItem;
    WorldPacketHandlers[CMSG_CLEAR_TRADE_ITEM].handler                      = &WorldSession::HandleClearTradeItem;
    WorldPacketHandlers[CMSG_SET_TRADE_GOLD].handler                        = &WorldSession::HandleSetTradeGold;

    // Quest System
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_QUERY].handler               = &WorldSession::HandleQuestgiverStatusQueryOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_HELLO].handler                      = &WorldSession::HandleQuestgiverHelloOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_ACCEPT_QUEST].handler               = &WorldSession::HandleQuestgiverAcceptQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_CHOOSE_REWARD].handler              = &WorldSession::HandleQuestgiverChooseRewardOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_REQUEST_REWARD].handler             = &WorldSession::HandleQuestgiverRequestRewardOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_QUERY_QUEST].handler                = &WorldSession::HandleQuestGiverQueryQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_COMPLETE_QUEST].handler             = &WorldSession::HandleQuestgiverCompleteQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTLOG_REMOVE_QUEST].handler                 = &WorldSession::HandleQuestlogRemoveQuestOpcode;
    WorldPacketHandlers[CMSG_RECLAIM_CORPSE].handler                        = &WorldSession::HandleCorpseReclaimOpcode;
    WorldPacketHandlers[CMSG_RESURRECT_RESPONSE].handler                    = &WorldSession::HandleResurrectResponseOpcode;
    WorldPacketHandlers[CMSG_PUSHQUESTTOPARTY].handler                      = &WorldSession::HandlePushQuestToPartyOpcode;
    WorldPacketHandlers[MSG_QUEST_PUSH_RESULT].handler                      = &WorldSession::HandleQuestPushResult;
    WorldPacketHandlers[CMSG_QUEST_POI_QUERY].handler                       = &WorldSession::HandleQuestPOI;

    // Auction System
    WorldPacketHandlers[CMSG_AUCTION_LIST_ITEMS].handler                    = &WorldSession::HandleAuctionListItems;
    WorldPacketHandlers[CMSG_AUCTION_LIST_BIDDER_ITEMS].handler             = &WorldSession::HandleAuctionListBidderItems;
    WorldPacketHandlers[CMSG_AUCTION_SELL_ITEM].handler                     = &WorldSession::HandleAuctionSellItem;
    WorldPacketHandlers[CMSG_AUCTION_LIST_OWNER_ITEMS].handler              = &WorldSession::HandleAuctionListOwnerItems;
    WorldPacketHandlers[CMSG_AUCTION_PLACE_BID].handler                     = &WorldSession::HandleAuctionPlaceBid;
    WorldPacketHandlers[CMSG_AUCTION_REMOVE_ITEM].handler                   = &WorldSession::HandleCancelAuction;

    // Mail System
    WorldPacketHandlers[CMSG_GET_MAIL_LIST].handler                         = &WorldSession::HandleGetMail;
    WorldPacketHandlers[CMSG_ITEM_TEXT_QUERY].handler                       = &WorldSession::HandleItemTextQuery;
    WorldPacketHandlers[CMSG_SEND_MAIL].handler                             = &WorldSession::HandleSendMail;
    WorldPacketHandlers[CMSG_MAIL_TAKE_MONEY].handler                       = &WorldSession::HandleTakeMoney;
    WorldPacketHandlers[CMSG_MAIL_TAKE_ITEM].handler                        = &WorldSession::HandleTakeItem;
    WorldPacketHandlers[CMSG_MAIL_MARK_AS_READ].handler                     = &WorldSession::HandleMarkAsRead;
    WorldPacketHandlers[CMSG_MAIL_RETURN_TO_SENDER].handler                 = &WorldSession::HandleReturnToSender;
    WorldPacketHandlers[CMSG_MAIL_DELETE].handler                           = &WorldSession::HandleMailDelete;
    WorldPacketHandlers[MSG_QUERY_NEXT_MAIL_TIME].handler                   = &WorldSession::HandleMailTime;
    WorldPacketHandlers[CMSG_MAIL_CREATE_TEXT_ITEM].handler                 = &WorldSession::HandleMailCreateTextItem;

    // Guild System
    WorldPacketHandlers[CMSG_GUILD_QUERY].handler                           = &WorldSession::HandleGuildQuery;
    WorldPacketHandlers[CMSG_GUILD_QUERY].status                            = STATUS_AUTHED;
    WorldPacketHandlers[CMSG_GUILD_INVITE].handler                          = &WorldSession::HandleInviteToGuild;
    WorldPacketHandlers[CMSG_GUILD_ACCEPT].handler                          = &WorldSession::HandleGuildAccept;
    WorldPacketHandlers[CMSG_GUILD_DECLINE].handler                         = &WorldSession::HandleGuildDecline;
    WorldPacketHandlers[CMSG_GUILD_INFO].handler                            = &WorldSession::HandleGuildInfo;
    WorldPacketHandlers[CMSG_GUILD_ROSTER].handler                          = &WorldSession::HandleGuildRoster;
    WorldPacketHandlers[CMSG_GUILD_PROMOTE].handler                         = &WorldSession::HandleGuildPromote;
    WorldPacketHandlers[CMSG_GUILD_DEMOTE].handler                          = &WorldSession::HandleGuildDemote;
    WorldPacketHandlers[CMSG_GUILD_LEAVE].handler                           = &WorldSession::HandleGuildLeave;
    WorldPacketHandlers[CMSG_GUILD_REMOVE].handler                          = &WorldSession::HandleGuildRemove;
    WorldPacketHandlers[CMSG_GUILD_DISBAND].handler                         = &WorldSession::HandleGuildDisband;
    WorldPacketHandlers[CMSG_GUILD_LEADER].handler                          = &WorldSession::HandleGuildLeader;
    WorldPacketHandlers[CMSG_GUILD_MOTD].handler                            = &WorldSession::HandleGuildMotd;
    WorldPacketHandlers[CMSG_GUILD_RANK].handler                            = &WorldSession::HandleGuildEditRank;
    WorldPacketHandlers[CMSG_GUILD_ADD_RANK].handler                        = &WorldSession::HandleGuildAddRank;
    WorldPacketHandlers[CMSG_GUILD_DEL_RANK].handler                        = &WorldSession::HandleGuildDelRank;
    //WorldPacketHandlers[CMSG_GUILD_SET_PUBLIC_NOTE].handler                 = &WorldSession::HandleGuildSetPublicNote;
    //WorldPacketHandlers[CMSG_GUILD_SET_OFFICER_NOTE].handler                = &WorldSession::HandleGuildSetOfficerNote;
    WorldPacketHandlers[CMSG_PETITION_BUY].handler                          = &WorldSession::HandleCharterBuy;
    WorldPacketHandlers[CMSG_PETITION_SHOW_SIGNATURES].handler              = &WorldSession::HandleCharterShowSignatures;
    WorldPacketHandlers[CMSG_TURN_IN_PETITION].handler                      = &WorldSession::HandleCharterTurnInCharter;
    WorldPacketHandlers[CMSG_PETITION_QUERY].handler                        = &WorldSession::HandleCharterQuery;
    WorldPacketHandlers[CMSG_OFFER_PETITION].handler                        = &WorldSession::HandleCharterOffer;
    WorldPacketHandlers[CMSG_PETITION_SIGN].handler                         = &WorldSession::HandleCharterSign;
    WorldPacketHandlers[MSG_PETITION_RENAME].handler                        = &WorldSession::HandleCharterRename;
    WorldPacketHandlers[MSG_SAVE_GUILD_EMBLEM].handler                      = &WorldSession::HandleSaveGuildEmblem;
    WorldPacketHandlers[CMSG_GUILD_INFO_TEXT].handler                       = &WorldSession::HandleSetGuildInformation;
    WorldPacketHandlers[MSG_GUILD_PERMISSIONS].handler                      = &WorldSession::HandleGuildGetFullPermissions;
    WorldPacketHandlers[MSG_GUILD_EVENT_LOG_QUERY].handler                  = &WorldSession::HandleGuildLog;
    WorldPacketHandlers[MSG_QUERY_GUILD_BANK_TEXT].handler                  = &WorldSession::HandleGuildBankQueryText;
    WorldPacketHandlers[CMSG_SET_GUILD_BANK_TEXT].handler                   = &WorldSession::HandleSetGuildBankText;
    WorldPacketHandlers[CMSG_GUILD_BANKER_ACTIVATE].handler                 = &WorldSession::HandleGuildBankOpenVault;
    WorldPacketHandlers[CMSG_GUILD_BANK_BUY_TAB].handler                    = &WorldSession::HandleGuildBankBuyTab;
    WorldPacketHandlers[MSG_GUILD_BANK_MONEY_WITHDRAWN].handler             = &WorldSession::HandleGuildBankGetAvailableAmount;
    WorldPacketHandlers[CMSG_GUILD_BANK_UPDATE_TAB].handler                 = &WorldSession::HandleGuildBankModifyTab;
    WorldPacketHandlers[CMSG_GUILD_BANK_SWAP_ITEMS].handler                 = &WorldSession::HandleGuildBankSwapItem;
    WorldPacketHandlers[CMSG_GUILD_BANK_WITHDRAW_MONEY].handler             = &WorldSession::HandleGuildBankWithdrawMoney;
    WorldPacketHandlers[CMSG_GUILD_BANK_DEPOSIT_MONEY].handler              = &WorldSession::HandleGuildBankDepositMoney;
    WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TAB].handler                  = &WorldSession::HandleGuildBankViewTab;
    WorldPacketHandlers[MSG_GUILD_BANK_LOG_QUERY].handler                   = &WorldSession::HandleGuildBankViewLog;

    // Tutorials
    WorldPacketHandlers[CMSG_TUTORIAL_FLAG].handler                         = &WorldSession::HandleTutorialFlag;
    WorldPacketHandlers[CMSG_TUTORIAL_CLEAR].handler                        = &WorldSession::HandleTutorialClear;
    WorldPacketHandlers[CMSG_TUTORIAL_RESET].handler                        = &WorldSession::HandleTutorialReset;

    // Pets
    WorldPacketHandlers[CMSG_PET_ACTION].handler                            = &WorldSession::HandlePetAction;
    WorldPacketHandlers[CMSG_REQUEST_PET_INFO].handler                      = &WorldSession::HandlePetInfo;
    WorldPacketHandlers[CMSG_PET_NAME_QUERY].handler                        = &WorldSession::HandlePetNameQuery;
    //WorldPacketHandlers[CMSG_BUY_STABLE_SLOT].handler                       = &WorldSession::HandleBuyStableSlot;
    //WorldPacketHandlers[CMSG_STABLE_PET].handler                            = &WorldSession::HandleStablePet;
    //WorldPacketHandlers[CMSG_UNSTABLE_PET].handler                          = &WorldSession::HandleUnstablePet;
    //WorldPacketHandlers[CMSG_STABLE_SWAP_PET].handler                       = &WorldSession::HandleStableSwapPet;
    WorldPacketHandlers[MSG_LIST_STABLED_PETS].handler                      = &WorldSession::HandleStabledPetList;
    WorldPacketHandlers[CMSG_PET_SET_ACTION].handler                        = &WorldSession::HandlePetSetActionOpcode;
    WorldPacketHandlers[CMSG_PET_RENAME].handler                            = &WorldSession::HandlePetRename;
    WorldPacketHandlers[CMSG_PET_ABANDON].handler                           = &WorldSession::HandlePetAbandon;
    WorldPacketHandlers[CMSG_PET_UNLEARN].handler                           = &WorldSession::HandlePetUnlearn;
    WorldPacketHandlers[CMSG_PET_LEARN_TALENT].handler                      = &WorldSession::HandlePetLearnTalent;
    WorldPacketHandlers[CMSG_PET_CANCEL_AURA].handler                       = &WorldSession::HandleCancelPetAura;

    // Totems
    WorldPacketHandlers[CMSG_TOTEM_DESTROYED].handler                       = &WorldSession::HandleTotemDestroyed;

    // Battlegrounds
    WorldPacketHandlers[CMSG_BATTLEFIELD_PORT].handler                      = &WorldSession::HandleBattlefieldPortOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_STATUS].handler                    = &WorldSession::HandleBattlefieldStatusOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_LIST].handler                      = &WorldSession::HandleBattlefieldListOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_HELLO].handler                    = &WorldSession::HandleBattleMasterHelloOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN_ARENA].handler               = &WorldSession::HandleArenaJoinOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN].handler                     = &WorldSession::HandleBattleMasterJoinOpcode;
    WorldPacketHandlers[CMSG_LEAVE_BATTLEFIELD].handler                     = &WorldSession::HandleLeaveBattlefieldOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUERY].handler              = &WorldSession::HandleAreaSpiritHealerQueryOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUEUE].handler              = &WorldSession::HandleAreaSpiritHealerQueueOpcode;
    //WorldPacketHandlers[MSG_BATTLEGROUND_PLAYER_POSITIONS].handler          = &WorldSession::HandleBattlegroundPlayerPositionsOpcode;
    WorldPacketHandlers[MSG_PVP_LOG_DATA].handler                           = &WorldSession::HandlePVPLogDataOpcode;
    //WorldPacketHandlers[MSG_INSPECT_HONOR_STATS].handler                    = &WorldSession::HandleInspectHonorStatsOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIONBAR_TOGGLES].handler                 = &WorldSession::HandleSetActionBarTogglesOpcode;
    WorldPacketHandlers[CMSG_MOVE_SPLINE_DONE].handler                      = &WorldSession::HandleMoveSplineCompleteOpcode;

    // GM Ticket System
    WorldPacketHandlers[CMSG_GMTICKET_CREATE].handler                       = &WorldSession::HandleGMTicketCreateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_UPDATETEXT].handler                   = &WorldSession::HandleGMTicketUpdateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_DELETETICKET].handler                 = &WorldSession::HandleGMTicketDeleteOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_GETTICKET].handler                    = &WorldSession::HandleGMTicketGetTicketOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_SYSTEMSTATUS].handler                 = &WorldSession::HandleGMTicketSystemStatusOpcode;
    WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].handler                       = &WorldSession::HandleGMTicketSurveySubmitOpcode;

    // Meeting Stone / Instances
    WorldPacketHandlers[CMSG_SUMMON_RESPONSE].handler                       = &WorldSession::HandleSummonResponseOpcode;
    WorldPacketHandlers[CMSG_RESET_INSTANCES].handler                       = &WorldSession::HandleResetInstanceOpcode;
    WorldPacketHandlers[CMSG_SELF_RES].handler                              = &WorldSession::HandleSelfResurrectOpcode;
    WorldPacketHandlers[CMSG_MEETINGSTONE_INFO].handler                     = &WorldSession::HandleSelfResurrectOpcode;
    WorldPacketHandlers[MSG_RANDOM_ROLL].handler                            = &WorldSession::HandleRandomRollOpcode;
    WorldPacketHandlers[MSG_SET_DUNGEON_DIFFICULTY].handler                 = &WorldSession::HandleDungeonDifficultyOpcode;
    WorldPacketHandlers[MSG_SET_RAID_DIFFICULTY].handler                    = &WorldSession::HandleRaidDifficultyOpcode;

    // Misc
    WorldPacketHandlers[CMSG_OPEN_ITEM].handler                             = &WorldSession::HandleOpenItemOpcode;
    WorldPacketHandlers[CMSG_COMPLETE_CINEMATIC].handler                    = &WorldSession::HandleCompleteCinematic;
    WorldPacketHandlers[CMSG_MOUNTSPECIAL_ANIM].handler                     = &WorldSession::HandleMountSpecialAnimOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_CLOAK].handler                          = &WorldSession::HandleToggleCloakOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_HELM].handler                           = &WorldSession::HandleToggleHelmOpcode;
    WorldPacketHandlers[CMSG_SET_TITLE].handler                             = &WorldSession::HandleSetVisibleRankOpcode;
    WorldPacketHandlers[CMSG_COMPLAIN].handler                              = &WorldSession::HandleReportSpamOpcode;
    WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler           = &WorldSession::HandleWorldStateUITimerUpdate;
    WorldPacketHandlers[CMSG_EQUIPMENT_SET_SAVE].handler                    = &WorldSession::HandleEquipmentSetSave;
    WorldPacketHandlers[CMSG_EQUIPMENT_SET_DELETE].handler                  = &WorldSession::HandleEquipmentSetDelete;
    WorldPacketHandlers[CMSG_EQUIPMENT_SET_USE].handler                     = &WorldSession::HandleEquipmentSetUse;
    WorldPacketHandlers[CMSG_HEARTH_AND_RESURRECT].handler                  = &WorldSession::HandleHearthandResurrect;

    // Arenas
    WorldPacketHandlers[CMSG_ARENA_TEAM_QUERY].handler                      = &WorldSession::HandleArenaTeamQueryOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_ROSTER].handler                     = &WorldSession::HandleArenaTeamRosterOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_INVITE].handler                     = &WorldSession::HandleArenaTeamAddMemberOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_ACCEPT].handler                     = &WorldSession::HandleArenaTeamInviteAcceptOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_DECLINE].handler                    = &WorldSession::HandleArenaTeamInviteDenyOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_LEAVE].handler                      = &WorldSession::HandleArenaTeamLeaveOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_REMOVE].handler                     = &WorldSession::HandleArenaTeamRemoveMemberOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_DISBAND].handler                    = &WorldSession::HandleArenaTeamDisbandOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_LEADER].handler                     = &WorldSession::HandleArenaTeamPromoteOpcode;
    WorldPacketHandlers[MSG_INSPECT_ARENA_TEAMS].handler                    = &WorldSession::HandleInspectArenaStatsOpcode;

    // voicechat
    WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler                  = &WorldSession::HandleEnableMicrophoneOpcode;
    WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].status                   = STATUS_AUTHED;
    WorldPacketHandlers[CMSG_CHANNEL_VOICE_ON].handler                      = &WorldSession::HandleChannelVoiceOnOpcode;
    WorldPacketHandlers[CMSG_SET_CHANNEL_WATCH].handler                     = &WorldSession::HandleChannelWatchOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler              = &WorldSession::HandleVoiceChatQueryOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].status               = STATUS_AUTHED;

    // Opt out of loot!
    WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler                       = &WorldSession::HandleSetAutoLootPassOpcode;

    WorldPacketHandlers[CMSG_REALM_SPLIT].handler                           = &WorldSession::HandleRealmSplit;
    WorldPacketHandlers[CMSG_REALM_SPLIT].status                            = STATUS_AUTHED;
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler      = &WorldSession::HandleInrangeQuestgiverQuery;
    WorldPacketHandlers[CMSG_ALTER_APPEARANCE].handler                      = &WorldSession::HandleAlterAppearance;
    WorldPacketHandlers[CMSG_REMOVE_GLYPH].handler                          = &WorldSession::HandleRemoveGlyph;
    WorldPacketHandlers[CMSG_QUERY_INSPECT_ACHIEVEMENTS].handler            = &WorldSession::HandleAchievementInspect;
    WorldPacketHandlers[CMSG_FAR_SIGHT].handler                             = &WorldSession::HandleFarsightOpcode;

    // Calendar
    WorldPacketHandlers[CMSG_CALENDAR_GET_CALENDAR].handler                 = &WorldSession::HandleCalendarGetCalendar;
    WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].handler                    = &WorldSession::HandleCalendarGetEvent;
    WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].handler                 = &WorldSession::HandleCalendarGuildFilter;
    WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].handler                   = &WorldSession::HandleCalendarArenaTeam;
    WorldPacketHandlers[CMSG_CALENDAR_ADD_EVENT].handler                    = &WorldSession::HandleCalendarAddEvent;
    WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].handler                 = &WorldSession::HandleCalendarUpdateEvent;
    WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].handler                 = &WorldSession::HandleCalendarRemoveEvent;
    WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].handler                   = &WorldSession::HandleCalendarCopyEvent;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].handler                 = &WorldSession::HandleCalendarEventInvite;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].handler                   = &WorldSession::HandleCalendarEventRsvp;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].handler          = &WorldSession::HandleCalendarEventRemoveInvite;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].handler                 = &WorldSession::HandleCalendarEventStatus;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].handler       = &WorldSession::HandleCalendarEventModeratorStatus;
    WorldPacketHandlers[CMSG_CALENDAR_COMPLAIN].handler                     = &WorldSession::HandleCalendarComplain;
    WorldPacketHandlers[CMSG_CALENDAR_GET_NUM_PENDING].handler              = &WorldSession::HandleCalendarGetNumPending;

    // Vehicles
    WorldPacketHandlers[CMSG_SPELLCLICK].handler                            = &WorldSession::HandleSpellClick;
    WorldPacketHandlers[CMSG_DISMISS_CONTROLLED_VEHICLE].handler            = &WorldSession::HandleVehicleDismiss;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_EXIT].handler                  = &WorldSession::HandleVehicleDismiss;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_PREV_SEAT].handler             = &WorldSession::HandleRequestSeatChange;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_NEXT_SEAT].handler             = &WorldSession::HandleRequestSeatChange;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_SWITCH_SEAT].handler           = &WorldSession::HandleRequestSeatChange;
    WorldPacketHandlers[CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE].handler    = &WorldSession::HandleRequestSeatChange;
    WorldPacketHandlers[CMSG_EJECT_PASSENGER].handler                       = &WorldSession::HandleEjectPassenger;

    WorldPacketHandlers[CMSG_PLAYER_VEHICLE_ENTER].handler                  = &WorldSession::HandleVehicleMountEnter;

    // Minion Cast Spell
    WorldPacketHandlers[CMSG_PET_CAST_SPELL].handler                        = &WorldSession::HandleCharmForceCastSpell;

    /// Empty packets
}
///
void WorldSession::EmptyPacket(WorldPacket &recv_data)
{
    // This is for collecting packet information before we start working on it.

}

/// Logging helper for unexpected opcodes
void WorldSession::LogUnprocessedTail(WorldPacket *packet)
{
    sLog.outError( "SESSION: opcode %s (0x%.4X) has unprocessed tail data \n"
        "The size recieved is %u while the packet size is %u\n",
        sOpcodeMgr.GetOpcodeName(packet->GetOpcode()),
        packet->GetOpcode(),
        packet->rpos(), packet->wpos());

    packet->print_storage();
}

void WorldSession::SystemMessage(const char * format, ...)
{
    WorldPacket * data;
    char buffer[1024];
    va_list ap;
    va_start(ap,format);
    vsnprintf(buffer,1024,format,ap);
    va_end(ap);

    data = sChatHandler.FillSystemMessageData(buffer);
    SendPacket(data);
    delete data;
}

void WorldSession::SendChatPacket(WorldPacket * data, uint32 langpos, int32 lang, WorldSession * originator)
{
    if(lang == -1)
        *(uint32*)&data->contents()[langpos] = lang;
    else
    {
        if(CanUseCommand('c') || (originator && originator->CanUseCommand('c')))
            *(uint32*)&data->contents()[langpos] = LANG_UNIVERSAL;
        else
            *(uint32*)&data->contents()[langpos] = lang;
    }

    SendPacket(data);
}

void WorldSession::SendItemPushResult(Item* pItem, bool Created, bool Received, bool SendToSet, bool NewItem, uint8 DestBagSlot, uint32 DestSlot, uint32 AddCount)
{
    packetSMSG_ITEM_PUSH_RESULT data;
    data.guid = _player->GetGUID();
    data.received = Received;
    data.created = Created;
    data.unk1 = 1;
    data.destbagslot = DestBagSlot;
    data.destslot = NewItem ? DestSlot : 0xFFFFFFFF;
    data.entry = pItem->GetEntry();
    data.suffix = pItem->GetItemRandomSuffixFactor();
    data.randomprop = pItem->GetUInt32Value( ITEM_FIELD_RANDOM_PROPERTIES_ID );
    data.count = AddCount;
    data.stackcount = pItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT);

    if(SendToSet)
    {
        if( _player->GetGroup() )
            _player->GetGroup()->OutPacketToAll( SMSG_ITEM_PUSH_RESULT, sizeof( packetSMSG_ITEM_PUSH_RESULT ), &data );
        else
            OutPacket( SMSG_ITEM_PUSH_RESULT, sizeof( packetSMSG_ITEM_PUSH_RESULT ), &data );
    }
    else
        OutPacket( SMSG_ITEM_PUSH_RESULT, sizeof( packetSMSG_ITEM_PUSH_RESULT ), &data );
}

void WorldSession::SendPacket(WorldPacket* packet)
{
    bool World = false;
    if(_player && _player->IsInWorld())
        World = true;
    if(_socket && _socket->IsConnected())
        _socket->SendPacket(packet, World);
}

void WorldSession::OutPacket(uint16 opcode, uint16 len, const void* data)
{
    bool World = false;
    if(_player && _player->IsInWorld())
        World = true;
    if(_socket && _socket->IsConnected())
        _socket->OutPacket(opcode, (data == NULL ? 0 : len), data, World);
}

void WorldSession::Delete()
{
    //deleteMutex.Acquire();
    delete this;
}

void WorldSession::HandleRealmSplit(WorldPacket & recv_data)
{
    WorldPacket data(SMSG_REALM_SPLIT, 17);
    data << recv_data.read<uint32>() << uint32(0);
    data << "01/01/01";
    SendPacket(&data);
}

void WorldSession::HandleWorldLoginOpcode(WorldPacket & recv_data)
{
    uint8 unk;
    uint32 mapId;
    recv_data >> unk >> mapId;

    if(_player == NULL)
        return;

    //_player->OnWorldLogin();
}

void WorldSession::HandleObjectUpdateRequest(WorldPacket & recv_data)
{
    if(_player == NULL)
        return;

    uint64 guid;
    recv_data >> guid;

    _player->SendObjectUpdate(guid);
}

void WorldSession::HandleAchievementInspect(WorldPacket &recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 2);
    CHECK_INWORLD_RETURN();
    WoWGuid guid;
    recv_data >> guid;

    uint64 rguid = guid.GetOldGuid();
    Unit* pUnit = GetPlayer()->GetMapMgr()->GetPlayer( GUID_LOPART(rguid) );
    if( pUnit && pUnit->IsPlayer() && TO_PLAYER(pUnit)->GetAchievementInterface()->HasAchievements() )
    {
        SendPacket(TO_PLAYER(pUnit)->GetAchievementInterface()->BuildAchievementData(true));
    }
}

void WorldSession::HandleTimeSyncResp( WorldPacket & recv_data )
{
    uint32 counter, time_;
    recv_data >> counter >> time_;

    // This is just a response, no need to do anything... Yet.
}

void WorldSession::SendAccountDataTimes(uint8 mask)
{
    WorldPacket data(SMSG_ACCOUNT_DATA_TIMES, 4+1+4+8*4);
    data << uint32(UNIXTIME) << uint8(1) << uint32(mask);
    for (int i = 0; i < 8; i++)
    {
        AccountDataEntry* acct_data = GetAccountData(i);
        if(mask & (1 << i))
            data << uint32(acct_data ? acct_data->timeStamp : 0);
    }
    SendPacket(&data);
}

void WorldSession::LoadAccountData()
{
    if(sWorld.m_useAccountData)
    {
        QueryResult *pResult = CharacterDatabase.Query("SELECT * FROM account_data WHERE acct = %u", GetAccountId());
        if( pResult == NULL )
            return;

        for(uint8 i = 0; i < 8; i++)
        {
            const char *data = pResult->Fetch()[1+i].GetString();
            uint32 len = data ? strlen(data) : 0;
            if(len == 0)
                continue;
            SetAccountData(i, strdup(data), true, len);
        }
        delete pResult;
    }
}

void WorldSession::SaveAccountData()
{
    if( sWorld.m_useAccountData )
    {
        std::stringstream fieldName, dataString;
        fieldName << "acct";
        dataString << "'" << _accountId << "'";
        for(uint32 ui = 0; ui < 8; ++ui)
        {
            if(m_accountData[ui] == NULL || m_accountData[ui]->data == NULL
                || m_accountData[ui]->sz == 0)
                continue;
            fieldName << format(",uiconfig%u", ui);
            dataString << ", \"";
            CharacterDatabase.EscapeLongString(m_accountData[ui]->data, m_accountData[ui]->sz, dataString);
            dataString << "\"";
        }

        CharacterDatabase.Execute("REPLACE INTO account_data(%s) VALUES(%s);", fieldName.str().c_str(), dataString.str().c_str());
    }
}

void WorldSession::LoadTutorials()
{
    QueryResult *result = CharacterDatabase.Query("SELECT * FROM account_tutorials WHERE acct = '%u';", GetAccountId());
    if(result == NULL)
        return;
    for(uint32 ui = 0; ui < 8; ++ui)
        m_tutorials[ui] = result->Fetch()[1+ui].GetUInt32();
}

void WorldSession::SaveTutorials()
{
    std::stringstream ss;
    ss << "REPLACE INTO account_tutorials VALUES(";
    ss << "'" << GetAccountId() << "', ";
    for(uint32 ui = 0; ui < 8; ++ui)
    {
        if(ui != 0)
            ss << ", ";
        ss << "'" << m_tutorials[ui] << "'";
    }
    ss << ");";
    CharacterDatabase.ExecuteNA(ss.str().c_str());
}
