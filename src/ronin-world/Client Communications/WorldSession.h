/***
 * Demonstrike Core
 */

//
//
// WorldSession.h
//

#pragma once

class Player;
class WorldPacket;
class WorldSocket;
class WorldSession;
class MapInstance;
class Creature;

//#define SESSION_CAP 5

enum AreaTriggerFailures
{
    AREA_TRIGGER_FAILURE_OK             = 0,
    AREA_TRIGGER_FAILURE_UNAVAILABLE    = 1,
    AREA_TRIGGER_FAILURE_NO_BC          = 2,
    AREA_TRIGGER_FAILURE_NO_HEROIC      = 3,
    AREA_TRIGGER_FAILURE_NO_RAID        = 4,
    AREA_TRIGGER_FAILURE_NO_ATTUNE_Q    = 5,
    AREA_TRIGGER_FAILURE_NO_ATTUNE_I    = 6,
    AREA_TRIGGER_FAILURE_LEVEL          = 7,
    AREA_TRIGGER_FAILURE_NO_GROUP       = 8,
    AREA_TRIGGER_FAILURE_NO_KEY         = 9,
    AREA_TRIGGER_FAILURE_LEVEL_HEROIC   = 10,
    AREA_TRIGGER_FAILURE_NO_CHECK       = 11,
    AREA_TRIGGER_FAILURE_NO_WOTLK       = 12,
    AREA_TRIGGER_FAILURE_IN_QUEUE       = 13,
    AREA_TRIGGER_FAILURE_WRONG_GROUP    = 14,
};

struct OpcodeHandler
{
    uint16 status;
    void (WorldSession::*handler)(WorldPacket& recvPacket);
};

enum SessionStatus
{
    STATUS_AUTHED = 0,
    STATUS_LOGGEDIN,
    STATUS_IN_OR_LOGGINGOUT,
    STATUS_IGNORED,
    STATUS_WHENEVER
};

struct AccountDataEntry
{
    AccountDataEntry() : timeStamp(0), data(NULL), sz(0) { }

    uint32 timeStamp;
    char * data;
    uint32 sz;
};

// ? New 3.2.2 Account DataType Enums
enum AccountDataTypes
{
    GLOBAL_CONFIG_CACHE             = 0,    // 0x01
    PER_CHARACTER_CONFIG_CACHE      = 1,    // 0x02
    GLOBAL_BINDINGS_CACHE           = 2,    // 0x04
    PER_CHARACTER_BINDINGS_CACHE    = 3,    // 0x08
    GLOBAL_MACROS_CACHE             = 4,    // 0x10
    PER_CHARACTER_MACROS_CACHE      = 5,    // 0x20
    PER_CHARACTER_LAYOUT_CACHE      = 6,    // 0x40
    PER_CHARACTER_CHAT_CACHE        = 7,    // 0x80
    NUM_ACCOUNT_DATA_TYPES          = 8
};

#define GLOBAL_CACHE_MASK           0x15
#define PER_CHARACTER_CACHE_MASK    0xEA

typedef struct Cords {
    float x,y,z;
}Cords;

#define PLAYER_LOGOUT_DELAY (sWorld.LogoutDelay*1000)

#define CHECK_INWORLD_RETURN() if(_player == NULL || !_player->IsInWorld()) { return; }
#define SKIP_READ_PACKET(pckt) pckt.rpos(pckt.wpos())

#define NOTIFICATION_MESSAGE_NO_PERMISSION "You do not have permission to perform that function."
#define NOTIFICATION_MESSAGE_FAILURE "The requested action could not be performed."

extern OpcodeHandler *WorldPacketHandlers;

void CapitalizeString(std::string& arg);

class SERVER_DECL WorldSession
{
    friend class WorldSocket;
public:
    WorldSession(uint32 id, std::string Name, WorldSocket *sock);
    ~WorldSession();

    void Init();

    bool InitializeZLibCompression();
    void CharEnumDisplayData(QueryResultVector& results);

    RONIN_INLINE bool IsLoggingIn() { return (m_loggingInPlayer != NULL); };
    Player* m_loggingInPlayer;

    RONIN_INLINE void SendPacket(WorldPacket* packet);
    void OutPacket(uint16 opcode, uint16 len = 0, const void* data = NULL);

    void SendChatPacket(WorldPacket * data, uint32 langpos, uint32 guidPos, int32 lang, WorldSession * originator);

    // Process Logs
    void LogUnprocessedTail(WorldPacket *packet);

    uint32 m_currMsTime;
    time_t m_lastPing;

    RONIN_INLINE uint32 GetAccountId() const { return _accountId; }
    RONIN_INLINE Player* GetPlayer() { return _player; }

    /* Acct flags */
    void SetAccountFlags(uint32 flags) { _accountFlags = flags; }
    bool HasFlag(uint32 flag) { return (_accountFlags & flag) != 0; }
    uint32 GetHighestExpansion() 
    {
        if(HasFlag(ACCOUNT_FLAG_XPACK_03))
            return 3;
        else if(HasFlag(ACCOUNT_FLAG_XPACK_02))
            return 2;
        else if(HasFlag(ACCOUNT_FLAG_XPACK_01))
            return 1;
        return 0;
    };

    /* GM Permission System */
    void LoadSecurity(std::string securitystring);
    void SetSecurity(std::string securitystring);
    RONIN_INLINE uint32 GetAccountFlags() { return _accountFlags; }
    RONIN_INLINE const char* GetPermissions() { return permissions.c_str(); }
    RONIN_INLINE int GetPermissionCount() { return permissioncount; }
    RONIN_INLINE bool HasPermissions() { return (permissioncount > 0) ? true : false; }
    RONIN_INLINE bool HasGMPermissions()
    {
        if(!permissioncount)
            return false;

        return (strchr(permissions.c_str(),'a')!=NULL) ? true : false;
    }

    bool CanUseCommand(char cmdstr);

    RONIN_INLINE void SetSocket(WorldSocket *sock)
    {
        _socket = sock;
    }
    RONIN_INLINE void SetPlayer(Player* plr) { _player = plr; }

    RONIN_INLINE void SetAccountData(uint32 index, const char* data, uint32 sz, uint32 time = UNIXTIME)
    {
        ASSERT(index < 8);
        if(data == NULL || sz == 0)
        {
            if(m_accountData[index])
            {
                if(m_accountData[index]->data)
                    free(m_accountData[index]->data);
                m_accountData[index]->data = NULL;
                delete m_accountData[index];
                m_accountData[index] = NULL;
            }
        }
        else
        {
            if(m_accountData[index] == NULL)
                m_accountData[index] = new AccountDataEntry();
            m_accountData[index]->data = strdup(data);
            m_accountData[index]->sz = sz;
            m_accountData[index]->timeStamp = time;
        }
    }

    RONIN_INLINE AccountDataEntry* GetAccountData(uint32 index)
    {
        ASSERT(index < 8);
        return m_accountData[index];
    }

    void SetLogoutTimer(uint32 ms)
    {
        if(ms)  _logoutTime = m_currMsTime+ms;
        else    _logoutTime = 0;
    }

    void LogoutPlayer();

    RONIN_INLINE void QueuePacket(WorldPacket* packet)
    {
        m_lastPing = (uint32)UNIXTIME;
        _recvQueue.Push(packet);
    }

    RONIN_INLINE WorldSocket* GetSocket() { return _socket; }

    void Disconnect()
    {
        if(_socket && _socket->IsConnected())
            _socket->Disconnect();
        _socket = NULL;
    }

    int __fastcall Update(int32 InstanceID);
    RONIN_INLINE int32 GetEventInstanceId() { return m_eventInstanceId; }
    RONIN_INLINE void SetEventInstanceId(int32 instanceId) { m_eventInstanceId = instanceId; }

    bool IsHighPriority();
    void SendBuyFailed(uint64 guid, uint32 itemid, uint8 error);
    void SendSellItem(uint64 vendorguid, uint64 itemid, uint8 error);
    void SendNotification(const char *message, ...);

    RONIN_INLINE uint32 GetLatency() { return _latency; }
    RONIN_INLINE std::string GetAccountName() { return _accountName; }
    RONIN_INLINE const char * GetAccountNameS() { return _accountName.c_str(); }

    RONIN_INLINE uint32 GetClientBuild() { return client_build; }
    RONIN_INLINE void SetClientBuild(uint32 build) { client_build = build; }
    bool bDeleted;
    int32 m_moveDelayTime;
    int32 m_clientTimeDelay;

    void PlayerLoginProc(PlayerInfo *info);

    RONIN_INLINE bool IsLoggingOut() { return _loggingOut; }

    bool CanCreateDeathKnight()
    {
        if( sWorld.m_deathKnightOnePerAccount && m_hasDeathKnight )
            return false;

        if( sWorld.m_deathKnightReqLevel > m_highestLevel )
            return false;

        return true;
    }

    void AppendTutorialData(WorldPacket *packet) { packet->append(m_tutorials.GetMask(), m_tutorials.GetLength()); }

protected:
    bool m_hasDeathKnight;
    uint8 m_highestLevel;
    uint8 DeleteCharacter(WoWGuid guid);

    /// Login screen opcodes (PlayerHandler.cpp):
    void HandleCharEnumOpcode(WorldPacket& recvPacket);
    void HandleCharReorderOpcode(WorldPacket& recvPacket);
    void HandleCharDeleteOpcode(WorldPacket& recvPacket);
    void HandleCharCreateOpcode(WorldPacket& recvPacket);
    void HandleCharCustomizeOpcode(WorldPacket& recvPacket);
    void HandleRandomizeCharNameOpcode(WorldPacket& recvPacket);
    void HandlePlayerLoginOpcode(WorldPacket& recvPacket);
    void HandleWorldLoginOpcode(WorldPacket& recvPacket);
    void HandleObjectUpdateRequest(WorldPacket& recvPacket);

    /// Authentification and misc opcodes (MiscHandler.cpp):
    void HandlePingOpcode(WorldPacket& recvPacket);
    void HandleAuthSessionOpcode(WorldPacket& recvPacket);
    void HandleRepopRequestOpcode(WorldPacket& recvPacket);
    void HandleAutostoreLootItemOpcode(WorldPacket& recvPacket);
    void HandleLootMoneyOpcode(WorldPacket& recvPacket);
    void HandleLootOpcode(WorldPacket& recvPacket);
    void HandleLootReleaseOpcode(WorldPacket& recvPacket);
    void HandleLootMasterGiveOpcode(WorldPacket& recv_data);
    void HandleLootRollOpcode(WorldPacket& recv_data);
    void HandleWhoOpcode(WorldPacket& recvPacket);
    void HandleLogoutRequestOpcode(WorldPacket& recvPacket);
    void HandlePlayerLogoutOpcode(WorldPacket& recvPacket);
    void HandleLogoutCancelOpcode(WorldPacket& recvPacket);
    void HandleZoneUpdateOpcode(WorldPacket& recvPacket);
    void HandleSetTargetOpcode(WorldPacket& recvPacket);
    void HandleSetSelectionOpcode(WorldPacket& recvPacket);
    void HandleStandStateChangeOpcode(WorldPacket& recvPacket);
    void HandleDismountOpcode(WorldPacket & recvPacket);
    void HandleFriendListOpcode(WorldPacket& recvPacket);
    void HandleAddFriendOpcode(WorldPacket& recvPacket);
    void HandleDelFriendOpcode(WorldPacket& recvPacket);
    void HandleAddIgnoreOpcode(WorldPacket& recvPacket);
    void HandleDelIgnoreOpcode(WorldPacket& recvPacket);
    void HandleBugOpcode(WorldPacket& recvPacket);
    void HandleUpdateAccountData(WorldPacket& recvPacket);
    void HandleRequestAccountData(WorldPacket& recvPacket);
    void HandleSetActionButtonOpcode(WorldPacket& recvPacket);
    void HandleSetAtWarOpcode(WorldPacket& recvPacket);
    void HandleSetWatchedFactionIndexOpcode(WorldPacket& recvPacket);
    void HandleTogglePVPOpcode(WorldPacket& recvPacket);
    void HandleSetCurrencyFlags(WorldPacket &recvPacket);
    void HandleGameObjectUse(WorldPacket& recvPacket);
    //void HandleJoinChannelOpcode(WorldPacket& recvPacket);
    //void HandleLeaveChannelOpcode(WorldPacket& recvPacket);
    void HandlePlayedTimeOpcode(WorldPacket & recv_data);
    void HandleSetSheathedOpcode(WorldPacket & recv_data);
    void HandleCompleteCinematic(WorldPacket & recv_data);
    void HandleInspectOpcode( WorldPacket & recv_data );
    void HandleGameobjReportUseOpCode( WorldPacket& recv_data );
    void HandleTimeSyncResp(WorldPacket& recv_data);
    void HandleAreaTriggerOpcode(WorldPacket& recv_data);
    void _HandleAreaTriggerOpcode(uint32 id);

    /// Gm Ticket System in GMTicket.cpp:
    void HandleGMTicketCreateOpcode(WorldPacket& recvPacket);
    void HandleGMTicketUpdateOpcode(WorldPacket& recvPacket);
    void HandleGMTicketDeleteOpcode(WorldPacket& recvPacket);
    void HandleGMTicketGetTicketOpcode(WorldPacket& recvPacket);
    void HandleGMTicketSystemStatusOpcode(WorldPacket& recvPacket);
    void HandleGMTicketSurveySubmitOpcode(WorldPacket& recvPacket);

    /// Opcodes implemented in QueryHandler.cpp:
    void HandleNameQueryOpcode(WorldPacket& recvPacket);
    void HandleQueryTimeOpcode(WorldPacket& recvPacket);
    void HandleCreatureQueryOpcode(WorldPacket& recvPacket);
    void HandleGameObjectQueryOpcode(WorldPacket& recvPacket);
    void HandlePageTextQueryOpcode(WorldPacket & recvPacket);
    void HandleItemHotfixQueryOpcode(WorldPacket & recvPacket);

    /// Opcodes implemented in MovementHandler.cpp
    void HandleMovementOpcodes( WorldPacket& recv_data );
    void HandleMovementInputOpcodes( WorldPacket & recv_data );
    void HandleAcknowledgementOpcodes( WorldPacket & recv_data );
    void HandleMoveWorldPortAck( WorldPacket & recv_data );
    void HandleSetActiveMoverOpcode( WorldPacket & recv_data );
    void HandleMoveTimeSkippedOpcode( WorldPacket & recv_data );
    void HandleMoveSplineCompleteOpcode(WorldPacket &recvPacket);
    void HandleMoveFallResetOpcode( WorldPacket & recv_data);

    /// Opcodes implemented in GroupHandler.cpp:
    void HandleGroupInviteOpcode(WorldPacket& recvPacket);
    void HandleGroupInviteResponseOpcode(WorldPacket& recvPacket);
    void HandleGroupCancelOpcode(WorldPacket& recvPacket);
    void HandleGroupAcceptOpcode(WorldPacket& recvPacket);
    void HandleGroupDeclineOpcode(WorldPacket& recvPacket);
    void HandleGroupUninviteOpcode(WorldPacket& recvPacket);
    void HandleGroupUninviteGUIDOpcode(WorldPacket& recvPacket);
    void HandleGroupSetLeaderOpcode(WorldPacket& recvPacket);
    void HandleGroupDisbandOpcode(WorldPacket& recvPacket);
    void HandleLootMethodOpcode(WorldPacket& recvPacket);
    void HandleMinimapPingOpcode(WorldPacket& recvPacket);
    void HandleSetPlayerIconOpcode(WorldPacket& recv_data);
    void SendPartyCommandResult(Player* pPlayer, uint32 p1, std::string name, uint32 err);

    // Raid
    void HandleConvertGroupToRaidOpcode(WorldPacket& recvPacket);
    void HandleGroupChangeSubGroup(WorldPacket& recvPacket);
    void HandleGroupAssistantLeader(WorldPacket& recvPacket);
    void HandleRequestRaidInfoOpcode(WorldPacket& recvPacket);
    void HandleReadyCheckOpcode(WorldPacket& recv_data);
    void HandleGroupPromote(WorldPacket& recv_data);

    // LFG opcodes
    void HandleLFDPlrLockOpcode(WorldPacket& recvPacket);
    void HandleLFDPartyLockOpcode(WorldPacket& recvPacket);

    /// Taxi opcodes (TaxiHandler.cpp)
    void HandleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket);
    void HandleTaxiQueryAvaibleNodesOpcode(WorldPacket& recvPacket);
    void HandleActivateTaxiOpcode(WorldPacket& recvPacket);
    void HandleMultipleActivateTaxiOpcode(WorldPacket & recvPacket);

    /// NPC opcodes (NPCHandler.cpp)
    void HandleTabardVendorActivateOpcode(WorldPacket& recvPacket);
    void HandleBankerActivateOpcode(WorldPacket& recvPacket);
    void HandleBuyBankSlotOpcode(WorldPacket& recvPacket);
    void HandleTrainerListOpcode(WorldPacket& recvPacket);
    void HandleTrainerBuySpellOpcode(WorldPacket& recvPacket);
    void HandleCharterShowListOpcode(WorldPacket& recvPacket);
    void HandleGossipHelloOpcode(WorldPacket& recvPacket);
    void HandleGossipSelectOptionOpcode(WorldPacket& recvPacket);
    void HandleSpiritHealerActivateOpcode(WorldPacket& recvPacket);
    void HandleNpcTextQueryOpcode(WorldPacket& recvPacket);
    void HandleBinderActivateOpcode(WorldPacket& recvPacket);

    // Auction House opcodes
    void HandleAuctionHelloOpcode(WorldPacket& recvPacket);
    void HandleAuctionListItems( WorldPacket & recv_data );
    void HandleAuctionListBidderItems( WorldPacket & recv_data );
    void HandleAuctionSellItem( WorldPacket & recv_data );
    void HandleAuctionListOwnerItems( WorldPacket & recv_data );
    void HandleAuctionPlaceBid( WorldPacket & recv_data );
    void HandleCancelAuction( WorldPacket & recv_data);

    // Mail opcodes
    void HandleGetMail( WorldPacket & recv_data );
    void HandleSendMail( WorldPacket & recv_data );
    void HandleTakeMoney( WorldPacket & recv_data );
    void HandleTakeItem( WorldPacket & recv_data );
    void HandleMarkAsRead( WorldPacket & recv_data );
    void HandleReturnToSender( WorldPacket & recv_data );
    void HandleMailDelete( WorldPacket & recv_data );
    void HandleItemTextQuery( WorldPacket & recv_data);
    void HandleMailTime(WorldPacket & recv_data);
    void HandleMailCreateTextItem(WorldPacket & recv_data );

    /// Item opcodes (ItemHandler.cpp)
    void HandleSwapInvItemOpcode(WorldPacket& recvPacket);
    void HandleSwapItemOpcode(WorldPacket& recvPacket);
    void HandleDestroyItemOpcode(WorldPacket& recvPacket);
    void HandleAutoEquipItemOpcode(WorldPacket& recvPacket);
    void HandleSellItemOpcode(WorldPacket& recvPacket);
    void HandleBuyItemOpcode(WorldPacket& recvPacket);
    void HandleListInventoryOpcode(WorldPacket& recvPacket);
    void HandleAutoStoreBagItemOpcode(WorldPacket& recvPacket);
    void HandleBuyBackOpcode(WorldPacket& recvPacket);
    void HandleSplitOpcode(WorldPacket& recvPacket);
    void HandleReadItemOpcode(WorldPacket& recvPacket);
    void HandleRepairItemOpcode(WorldPacket &recvPacket);
    void HandleAutoBankItemOpcode(WorldPacket &recvPacket);
    void HandleAutoStoreBankItemOpcode(WorldPacket &recvPacket);
    void HandleCancelTemporaryEnchantmentOpcode(WorldPacket &recvPacket);
    void HandleInsertGemOpcode(WorldPacket &recvPacket);
    void HandleItemRefundInfoOpcode( WorldPacket& recvPacket );
    void HandleItemRefundRequestOpcode( WorldPacket& recvPacket );
    void HandleTransmogrifyItemsOpcode( WorldPacket& recvPacket );

    /// Combat opcodes (CombatHandler.cpp)
    void HandleAttackSwingOpcode(WorldPacket& recvPacket);
    void HandleAttackStopOpcode(WorldPacket& recvPacket);

    /// Spell opcodes (SpellHandler.cpp)
    void HandleUseItemOpcode(WorldPacket& recvPacket);
    void HandleCastSpellOpcode(WorldPacket& recvPacket);
    void HandleCancelCastOpcode(WorldPacket& recvPacket);
    void HandleCancelAuraOpcode(WorldPacket& recvPacket);
    void HandleCancelChannellingOpcode(WorldPacket& recvPacket);
    void HandleCancelAutoRepeatSpellOpcode(WorldPacket& recv_data);
    void HandleCharmForceCastSpell(WorldPacket & recvPacket);

    /// Skill opcodes (SkillHandler.spp)
    //void HandleSkillLevelUpOpcode(WorldPacket& recvPacket);
    void HandleLearnTalentOpcode(WorldPacket& recvPacket);
    void HandleLearnPreviewTalents(WorldPacket& recv_data);
    void HandleUnlearnTalents( WorldPacket & recv_data );
    void HandleTalentWipeConfirmOpcode(WorldPacket &recv_data);

    /// Quest opcodes (QuestHandler.cpp)
    void HandleQuestgiverStatusQueryOpcode(WorldPacket& recvPacket);
    void HandleQuestgiverHelloOpcode(WorldPacket& recvPacket);
    void HandleQuestgiverAcceptQuestOpcode(WorldPacket& recvPacket);
    void HandleQuestgiverChooseRewardOpcode(WorldPacket& recvPacket);
    void HandleQuestgiverRequestRewardOpcode(WorldPacket& recvPacket);
    void HandleQuestGiverQueryQuestOpcode( WorldPacket& recvPacket );
    void HandleQuestQueryOpcode(WorldPacket& recvPacket);
    void HandleQuestgiverCompleteQuestOpcode( WorldPacket & recvPacket );
    void HandleQuestlogRemoveQuestOpcode(WorldPacket& recvPacket);
    void HandlePushQuestToPartyOpcode(WorldPacket &recvPacket);
    void HandleQuestPushResult(WorldPacket &recvPacket);
    void HandleQuestPOI(WorldPacket &recvPacket);


    /// Chat opcodes (Chat.cpp)
    void HandleMessagechatOpcode(WorldPacket& recvPacket);
    void HandleTextEmoteOpcode(WorldPacket& recvPacket);
    void HandleReportSpamOpcode(WorldPacket& recvPacket);

    /// Corpse opcodes (Corpse.cpp)
    void HandleCorpseReclaimOpcode( WorldPacket& recvPacket );
    void HandleCorpseQueryOpcode( WorldPacket& recvPacket );
    void HandleResurrectResponseOpcode(WorldPacket& recvPacket);

    /// Channel Opcodes (ChannelHandler.cpp)
    void HandleChannelJoin(WorldPacket& recvPacket);
    void HandleChannelLeave(WorldPacket& recvPacket);
    void HandleChannelList(WorldPacket& recvPacket);
    void HandleChannelPassword(WorldPacket& recvPacket);
    void HandleChannelSetOwner(WorldPacket& recvPacket);
    void HandleChannelOwner(WorldPacket& recvPacket);
    void HandleChannelModerator(WorldPacket& recvPacket);
    void HandleChannelUnmoderator(WorldPacket& recvPacket);
    void HandleChannelMute(WorldPacket& recvPacket);
    void HandleChannelUnmute(WorldPacket& recvPacket);
    void HandleChannelInvite(WorldPacket& recvPacket);
    void HandleChannelKick(WorldPacket& recvPacket);
    void HandleChannelBan(WorldPacket& recvPacket);
    void HandleChannelUnban(WorldPacket& recvPacket);
    void HandleChannelAnnounce(WorldPacket& recvPacket);
    void HandleChannelModerate(WorldPacket& recvPacket);
    void HandleChannelNumMembersQuery(WorldPacket & recvPacket);
    void HandleChannelRosterQuery(WorldPacket & recvPacket);

    // Duel
    void HandleDuelAccepted(WorldPacket & recv_data);
    void HandleDuelCancelled(WorldPacket & recv_data);

    // Trade
    void HandleInitiateTrade(WorldPacket & recv_data);
    void HandleBeginTrade(WorldPacket & recv_data);
    void HandleBusyTrade(WorldPacket & recv_data);
    void HandleIgnoreTrade(WorldPacket & recv_data);
    void HandleAcceptTrade(WorldPacket & recv_data);
    void HandleUnacceptTrade(WorldPacket & recv_data);
    void HandleCancelTrade(WorldPacket & recv_data);
    void HandleSetTradeItem(WorldPacket & recv_data);
    void HandleClearTradeItem(WorldPacket & recv_data);
    void HandleSetTradeGold(WorldPacket & recv_data);

    // Guild
    void HandleGuildQuery(WorldPacket & recv_data);
    void HandleGuildXP(WorldPacket & recv_data);
    void HandleGuildMaxDailyXP(WorldPacket & recv_data);
    void HandleGuildNews(WorldPacket & recv_data);
    void HandleGuildRanks(WorldPacket & recv_data);
    void HandleGuildRoster(WorldPacket & recv_data);
    void HandleGuildRewards(WorldPacket & recv_data);
    void HandleGuildPermissions(WorldPacket & recv_data);
    void HandleGuildPartyState(WorldPacket & recv_data);
    void HandleGuildChallengeUpdate(WorldPacket & recv_data);
    void HandleInviteToGuild(WorldPacket & recv_data);
    void HandleGuildAccept(WorldPacket & recv_data);
    void HandleGuildDecline(WorldPacket & recv_data);
    void HandleGuildPromote(WorldPacket & recv_data);
    void HandleGuildDemote(WorldPacket & recv_data);
    void HandleGuildLeave(WorldPacket & recv_data);
    void HandleGuildRemove(WorldPacket & recv_data);
    void HandleGuildDisband(WorldPacket & recv_data);
    void HandleGuildMotd(WorldPacket & recv_data);
    void HandleGuildAddRank(WorldPacket & recv_data);
    void HandleGuildDelRank(WorldPacket & recv_data);
    void HandleGuildSetNote(WorldPacket & recv_data);
    void HandleSaveGuildEmblem(WorldPacket & recv_data);
    void HandleCharterBuy(WorldPacket & recv_data);
    void HandleCharterShowSignatures(WorldPacket & recv_data);
    void HandleCharterTurnInCharter(WorldPacket & recv_data);
    void HandleCharterQuery(WorldPacket & recv_data);
    void HandleCharterOffer(WorldPacket & recv_data);
    void HandleCharterSign(WorldPacket &recv_data);
    void HandleCharterRename(WorldPacket & recv_data);
    void HandleSetGuildInformation(WorldPacket & recv_data);
    void HandleGuildBankQueryText(WorldPacket & recv_data);
    void HandleSetGuildBankText(WorldPacket & recv_data);
    void HandleGuildLog(WorldPacket & recv_data);
    void HandleGuildBankViewTab(WorldPacket & recv_data);
    void HandleGuildBankViewLog(WorldPacket & recv_data);
    void HandleGuildBankOpenVault(WorldPacket & recv_data);
    void HandleGuildBankBuyTab(WorldPacket & recv_data);
    void HandleGuildBankDepositMoney(WorldPacket & recv_data);
    void HandleGuildBankWithdrawMoney(WorldPacket & recv_data);
    void HandleGuildBankSwapItem(WorldPacket & recv_data);
    void HandleGuildBankWithdrawItem(WorldPacket & recv_data);
    void HandleGuildBankGetAvailableAmount(WorldPacket & recv_data);
    void HandleGuildBankModifyTab(WorldPacket & recv_data);

    // Pet
    void HandlePetAction(WorldPacket & recv_data);
    void HandlePetInfo(WorldPacket & recv_data);
    void HandlePetNameQuery(WorldPacket & recv_data);
    void HandleBuyStableSlot(WorldPacket & recv_data);
    void HandleStablePet(WorldPacket & recv_data);
    void HandleUnstablePet(WorldPacket & recv_data);
    void HandleStabledPetList(WorldPacket & recv_data);
    void HandleStableSwapPet(WorldPacket & recv_data);
    void HandlePetRename(WorldPacket & recv_data);
    void HandlePetAbandon(WorldPacket & recv_data);
    void HandlePetUnlearn(WorldPacket & recv_data);
    void HandlePetLearnTalent(WorldPacket & recv_data);
    void HandleCancelPetAura(WorldPacket & recv_data);

    // Totems
    void HandleTotemDestroyed(WorldPacket & recv_data);

    // Battleground
    void HandleBattlefieldPortOpcode(WorldPacket &recv_data);
    void HandleBattlefieldStatusOpcode(WorldPacket &recv_data);
    void HandleBattleMasterHelloOpcode(WorldPacket &recv_data);
    void HandleLeaveBattlefieldOpcode(WorldPacket &recv_data);
    void HandleAreaSpiritHealerQueryOpcode(WorldPacket &recv_data);
    void HandleAreaSpiritHealerQueueOpcode(WorldPacket &recv_data);
    void HandleBattlegroundPlayerPositionsOpcode(WorldPacket &recv_data);
    void HandleArenaJoinOpcode(WorldPacket &recv_data);
    void HandleBattleMasterJoinOpcode(WorldPacket &recv_data);
    void HandleInspectHonorStatsOpcode(WorldPacket &recv_data);
    void HandlePVPLogDataOpcode(WorldPacket &recv_data);
    void HandleBattlefieldListOpcode(WorldPacket &recv_data);

    void HandleSetActionBarTogglesOpcode(WorldPacket &recvPacket);

    /// Helper functions
    void SetNpcFlagsForTalkToQuest(const uint64& guid, const uint64& targetGuid);

    //Tutorials
    void HandleTutorialFlag ( WorldPacket & recv_data );
    void HandleTutorialClear( WorldPacket & recv_data );
    void HandleTutorialReset( WorldPacket & recv_data );

    // Other
    void HandleMountSpecialAnimOpcode(WorldPacket& recv_data);
    void HandleSelfResurrectOpcode(WorldPacket& recv_data);
    void HandleUnlearnSkillOpcode(WorldPacket &recv_data);
    void HandleRandomRollOpcode(WorldPacket &recv_data);
    void HandleOpenItemOpcode(WorldPacket &recv_data);
    void HandleToggleHelmOpcode(WorldPacket &recv_data);
    void HandleToggleCloakOpcode(WorldPacket &recv_data);
    void HandleSetVisibleRankOpcode(WorldPacket& recv_data);
    void HandlePetSetActionOpcode(WorldPacket& recv_data);

    //instances
    void HandleResetInstanceOpcode(WorldPacket& recv_data);
    void HandleDungeonDifficultyOpcode(WorldPacket& recv_data);
    void HandleRaidDifficultyOpcode(WorldPacket& recv_data);

    void SendMailError(uint32 error, uint32 extra=0);

    void HandleCharRenameOpcode(WorldPacket & recv_data);
    void HandlePartyMemberStatsOpcode(WorldPacket & recv_data);
    void HandleSummonResponseOpcode(WorldPacket & recv_data);
    void HandleMeetingStoneInfo(WorldPacket & recv_data);

    void HandleArenaTeamAddMemberOpcode(WorldPacket & recv_data);
    void HandleArenaTeamRemoveMemberOpcode(WorldPacket & recv_data);
    void HandleArenaTeamInviteAcceptOpcode(WorldPacket & recv_data);
    void HandleArenaTeamInviteDenyOpcode(WorldPacket & recv_data);
    void HandleArenaTeamLeaveOpcode(WorldPacket & recv_data);
    void HandleArenaTeamDisbandOpcode(WorldPacket & recv_data);
    void HandleArenaTeamPromoteOpcode(WorldPacket & recv_data);
    void HandleArenaTeamQueryOpcode(WorldPacket & recv_data);
    void HandleArenaTeamRosterOpcode(WorldPacket & recv_data);
    void HandleInspectArenaStatsOpcode(WorldPacket & recv_data);
    void HandleWrapItemOpcode(WorldPacket& recv_data);

    // VOICECHAT
    void HandleEnableMicrophoneOpcode(WorldPacket & recv_data);
    void HandleVoiceChatQueryOpcode(WorldPacket & recv_data);
    void HandleChannelVoiceOnOpcode(WorldPacket & recv_data);
    void HandleChannelWatchOpcode(WorldPacket & recv_data);

    // Auto Loot Pass
    void HandleSetAutoLootPassOpcode(WorldPacket & recv_data);

    void HandleSetFriendNote(WorldPacket & recv_data);
    void HandleRealmSplit(WorldPacket & recv_data);
    void HandleInrangeQuestgiverQuery(WorldPacket & recv_data);

    // Misc Opcodes
    void HandleAlterAppearance(WorldPacket & recv_data);
    void HandleAchievementInspect(WorldPacket & recv_data);
    void HandleRemoveGlyph(WorldPacket & recv_data);

    //MISC
    void HandleReadyForAccountDataTimes(WorldPacket &recv_data);
    void HandleWorldStateUITimerUpdate( WorldPacket & recv_data );
    void HandleFarsightOpcode(WorldPacket &recv_data);
    void HandleEquipmentSetSave(WorldPacket &recv_data);
    void HandleEquipmentSetDelete(WorldPacket &recv_data);
    void HandleEquipmentSetUse(WorldPacket &recv_data);
    void HandleHearthandResurrect(WorldPacket &recv_data);

    //Calendar
    void HandleCalendarGetCalendar( WorldPacket & recv_data );
    void HandleCalendarGetEvent( WorldPacket & recv_data );
    void HandleCalendarGuildFilter( WorldPacket & recv_data );
    void HandleCalendarArenaTeam( WorldPacket & recv_data );
    void HandleCalendarAddEvent( WorldPacket & recv_data );
    void HandleCalendarUpdateEvent( WorldPacket & recv_data );
    void HandleCalendarRemoveEvent( WorldPacket & recv_data );
    void HandleCalendarCopyEvent( WorldPacket & recv_data );
    void HandleCalendarEventInvite( WorldPacket & recv_data );
    void HandleCalendarEventRsvp( WorldPacket & recv_data );
    void HandleCalendarEventRemoveInvite( WorldPacket & recv_data );
    void HandleCalendarEventStatus( WorldPacket & recv_data );
    void HandleCalendarEventModeratorStatus( WorldPacket & recv_data );
    void HandleCalendarComplain( WorldPacket & recv_data );
    void HandleCalendarGetNumPending( WorldPacket & recv_data );

    /// Empty packets
    void EmptyPacket(WorldPacket &recv_data);

public:
    void SendTradeStatus(uint32 TradeStatus);
    void SendCharterRequest(Creature* pCreature);
    void SendInnkeeperBind(Creature* pCreature);
    void SendBattlegroundList(Creature* pCreature, uint32 type);
    void SendBankerList(Creature* pCreature);
    void SendTabardHelp(Creature* pCreature);
    void SendAuctionList(Creature* pCreature);
    void SendSpiritHealerRequest(Creature* pCreature);
    void FullLogin(Player* plr);

    float m_wLevel; // Level of water the player is currently in
    bool m_bIsWLevelSet; // Does the m_wLevel variable contain up-to-date information about water level?

    bool m_isFalling;
    bool m_isJumping;
    bool m_isKnockedback;
    uint32 m_jumpHackChances;

private:
    friend class Player;
    Player* _player;
    WorldSocket *_socket;

    uint32 _accountId;
    uint32 _accountFlags;
    std::string _accountName;
    uint32 _logoutTime; // time we received a logout request -- wait 20 seconds, and quit

    WoWGuid m_MoverWoWGuid;

    z_stream *_zlibStream;
    FastQueue<WorldPacket*, Mutex> _recvQueue;
    std::string permissions;
    int permissioncount;

    bool _loggingOut;
    bool _recentlogout;
    bool m_asyncQuery;
    uint32 _latency;
    uint32 client_build;
    uint8 _updatecount;

    int32 m_eventInstanceId;

    // Data
    void SendAccountDataTimes(uint8 mask);
    void LoadAccountData();
    void SaveAccountData();
    AccountDataEntry *m_accountData[8];

    // Characters
    void LoadCharacterData();
    bool HasCharacterData(WoWGuid lowGuid);

    Mutex charDataLock;
    // First: Char index, Second: Pair<First: Player guid, Second: Player Map>;
    Loki::AssocVector<uint8, PlayerInfo*> m_charData;
    std::set<WoWGuid> m_bannedCharacters;

    // Tutorials
    void LoadTutorials();
    void SaveTutorials();
    UpdateMask m_tutorials;

public:
    void ValidateText1(std::string text)
    {
        if(!ValidateText2(text))
        {

        }
    };

    bool ValidateText2(std::string text);
    uint8 CheckTeleportPrerequisites(WorldSession * pSession, Player* pPlayer, uint32 mapid);
    uint32 floodLines;
    time_t floodTime;
    void SystemMessage(const char * format, ...);
    WorldPacket* BuildQuestQueryResponse(Quest *qst);
    uint32 m_muted;
    uint32 m_lastWhoTime;
    uint32 m_maxLevel;

protected:
    uint32 m_repeatTime;
    std::string m_repeatMessage;
    uint32 m_repeatEmoteTime;
    uint32 m_repeatEmoteId;

public:
    static void InitPacketHandlerTable();
    static void DeInitPacketHandlerTable();
};

void BuildCorpseInfo(WorldPacket* data, Corpse* corpse);
typedef std::set<WorldSession*> SessionSet;
