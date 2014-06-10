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
class MapMgr;
class Creature;
struct TrainerSpell;

//#define SESSION_CAP 5

// MovementFlags Contribution by Tenshi
enum MovementFlags
{
    // Byte 1 (Resets on Movement Key Press)
    MOVEFLAG_MOVE_STOP                  = 0x00,         //verified
    MOVEFLAG_MOVE_FORWARD               = 0x01,         //verified
    MOVEFLAG_MOVE_BACKWARD              = 0x02,         //verified
    MOVEFLAG_STRAFE_LEFT                = 0x04,         //verified
    MOVEFLAG_STRAFE_RIGHT               = 0x08,         //verified
    MOVEFLAG_TURN_LEFT                  = 0x10,         //verified
    MOVEFLAG_TURN_RIGHT                 = 0x20,         //verified
    MOVEFLAG_PITCH_DOWN                 = 0x40,         //Unconfirmed
    MOVEFLAG_PITCH_UP                   = 0x80,         //Unconfirmed

    // Byte 2 (Resets on Situation Change)
    MOVEFLAG_WALK                       = 0x100,        //verified
    MOVEFLAG_TAXI                       = 0x200,
    MOVEFLAG_NO_COLLISION               = 0x400,
    MOVEFLAG_FLYING                     = 0x800,        //verified
    MOVEFLAG_REDIRECTED                 = 0x1000,       //Jumping
    MOVEFLAG_FALLING                    = 0x2000,       //verified
    MOVEFLAG_FALLING_FAR                = 0x4000,       //verified
    MOVEFLAG_FREE_FALLING               = 0x8000,       //half verified

    // Byte 3 (Set by server. TB = Third Byte. Completely unconfirmed.)
    MOVEFLAG_TB_PENDING_STOP            = 0x10000,      // (MOVEFLAG_PENDING_STOP)
    MOVEFLAG_TB_PENDING_UNSTRAFE        = 0x20000,      // (MOVEFLAG_PENDING_UNSTRAFE)
    MOVEFLAG_TB_PENDING_FALL            = 0x40000,      // (MOVEFLAG_PENDING_FALL)
    MOVEFLAG_TB_PENDING_FORWARD         = 0x80000,      // (MOVEFLAG_PENDING_FORWARD)
    MOVEFLAG_TB_PENDING_BACKWARD        = 0x100000,     // (MOVEFLAG_PENDING_BACKWARD)
    MOVEFLAG_SWIMMING                   = 0x200000,     //  verified
    MOVEFLAG_FLYING_PITCH_UP            = 0x400000,     // (half confirmed)(MOVEFLAG_PENDING_STR_RGHT)
    MOVEFLAG_TB_MOVED                   = 0x800000,     // Send to client on entervehicle

    // Byte 4 (Script Based Flags. Never reset, only turned on or off.)
    MOVEFLAG_AIR_SUSPENSION             = 0x1000000,    // confirmed allow body air suspension(good name? lol).
    MOVEFLAG_AIR_SWIMMING               = 0x2000000,    // confirmed while flying.
    MOVEFLAG_SPLINE_MOVER               = 0x4000000,    // Unconfirmed
    MOVEFLAG_SPLINE_ENABLED             = 0x8000000,
    MOVEFLAG_WATER_WALK                 = 0x10000000,
    MOVEFLAG_FEATHER_FALL               = 0x20000000,   // Does not negate fall damage.
    MOVEFLAG_LEVITATE                   = 0x40000000,
    MOVEFLAG_LOCAL                      = 0x80000000,   // This flag defaults to on. (Assumption)

    // Masks
    MOVEFLAG_MOVING_MASK                = 0x03,
    MOVEFLAG_STRAFING_MASK              = 0x0C,
    MOVEFLAG_TURNING_MASK               = 0x30,
    MOVEFLAG_FALLING_MASK               = 0x6000,
    MOVEFLAG_MOTION_MASK                = 0xE00F,       // Forwards, Backwards, Strafing, Falling
    MOVEFLAG_PENDING_MASK               = 0x7F0000,
    MOVEFLAG_PENDING_STRAFE_MASK        = 0x600000,
    MOVEFLAG_PENDING_MOVE_MASK          = 0x180000,
    MOVEFLAG_FULL_FALLING_MASK          = 0xE000,
};

enum MovementFlags2
{
    MOVEMENTFLAG2_NONE                      = 0x00000000,
    MOVEMENTFLAG2_NO_STRAFE                 = 0x00000001,
    MOVEMENTFLAG2_NO_JUMPING                = 0x00000002,
    MOVEMENTFLAG2_UNK3                      = 0x00000004, // Overrides various clientside checks
    MOVEMENTFLAG2_FULL_SPEED_TURNING        = 0x00000008,
    MOVEMENTFLAG2_FULL_SPEED_PITCHING       = 0x00000010,
    MOVEMENTFLAG2_ALWAYS_ALLOW_PITCHING     = 0x00000020,
    MOVEMENTFLAG2_UNK7                      = 0x00000040,
    MOVEMENTFLAG2_UNK8                      = 0x00000080,
    MOVEMENTFLAG2_UNK9                      = 0x00000100,
    MOVEMENTFLAG2_UNK10                     = 0x00000200,
    MOVEMENTFLAG2_INTERPOLATED_MOVEMENT     = 0x00000400,
    MOVEMENTFLAG2_INTERPOLATED_TURNING      = 0x00000800,
    MOVEMENTFLAG2_INTERPOLATED_PITCHING     = 0x00001000,
    MOVEMENTFLAG2_UNK14                     = 0x00002000,
    MOVEMENTFLAG2_UNK15                     = 0x00004000,
    MOVEMENTFLAG2_UNK16                     = 0x00008000,
};

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

enum ActionButtonType
{
    ACTION_BUTTON_SPELL     = 0x00,
    ACTION_BUTTON_C         = 0x01,
    ACTION_BUTTON_EQSET     = 0x20,
    ACTION_BUTTON_MACRO     = 0x40,
    ACTION_BUTTON_CMACRO    = ACTION_BUTTON_C | ACTION_BUTTON_MACRO,
    ACTION_BUTTON_ITEM      = 0x80
};

#define ACTION_BUTTON_ACTION(X) (uint32(X) & 0x00FFFFFF)
#define ACTION_BUTTON_TYPE(X) ((uint32(X) & 0xFF000000) >> 24)
#define ACTION_PACK_BUTTION(action, type) uint32(action | (uint32(type) << 24))
#define MAX_ACTION_BUTTON_ACTION_VALUE (0x00FFFFFF+1)

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
#define CHECK_GUID_EXISTS(guidx) if(_player->GetMapMgr()->GetUnit((guidx)) == NULL) { return; }
#define CHECK_PACKET_SIZE(pckp, ssize) if(ssize && pckp.size() < ssize) { Disconnect(); return; }
#define SKIP_READ_PACKET(pckt) pckt.rpos(pckt.wpos())

#define NOTIFICATION_MESSAGE_NO_PERMISSION "You do not have permission to perform that function."
#define NOTIFICATION_MESSAGE_FAILURE "The requested action could not be performed."

extern OpcodeHandler WorldPacketHandlers[NUM_MSG_TYPES];
void CapitalizeString(string& arg);

class SERVER_DECL WorldSession : public EventableObject
{
    friend class WorldSocket;
public:
    WorldSession(uint32 id, string Name, WorldSocket *sock);
    ~WorldSession();

    HEARTHSTONE_INLINE bool IsLoggingIn() { return (m_loggingInPlayer != NULL); };
    Player* m_loggingInPlayer;

    HEARTHSTONE_INLINE void SendPacket(WorldPacket* packet);
    void OutPacket(uint16 opcode, uint16 len = 0, const void* data = NULL);

    void Delete();

    void SendChatPacket(WorldPacket * data, uint32 langpos, int32 lang, WorldSession * originator);

    // Process Logs
    void LogUnprocessedTail(WorldPacket *packet);

    uint32 m_currMsTime;
    uint32 m_lastPing;

    HEARTHSTONE_INLINE uint32 GetAccountId() const { return _accountId; }
    HEARTHSTONE_INLINE Player* GetPlayer() { return _player; }

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
    HEARTHSTONE_INLINE uint32 GetAccountFlags() { return _accountFlags; }
    HEARTHSTONE_INLINE char* GetPermissions() { return permissions; }
    HEARTHSTONE_INLINE int GetPermissionCount() { return permissioncount; }
    HEARTHSTONE_INLINE bool HasPermissions() { return (permissioncount > 0) ? true : false; }
    HEARTHSTONE_INLINE bool HasGMPermissions()
    {
        if(!permissioncount)
            return false;

        return (strchr(permissions,'a')!=NULL) ? true : false;
    }

    bool CanUseCommand(char cmdstr);

    HEARTHSTONE_INLINE void SetSocket(WorldSocket *sock)
    {
        _socket = sock;
    }
    HEARTHSTONE_INLINE void SetPlayer(Player* plr) { _player = plr; }

    HEARTHSTONE_INLINE void SetAccountData(uint32 index, char* data, uint32 sz, uint32 time = UNIXTIME)
    {
        ASSERT(index < 8);
        if(data == NULL || sz == 0)
        {
            if(m_accountData[index])
            {
                delete [] m_accountData[index]->data;
                m_accountData[index]->data = NULL;
                delete m_accountData[index];
                m_accountData[index] = NULL;
            }
        }
        else
        {
            if(m_accountData[index] == NULL)
                m_accountData[index] = new AccountDataEntry();
            m_accountData[index]->data = data;
            m_accountData[index]->sz = sz;
            m_accountData[index]->timeStamp = time;
        }
    }

    HEARTHSTONE_INLINE AccountDataEntry* GetAccountData(uint32 index)
    {
        ASSERT(index < 8);
        return m_accountData[index];
    }

    void SetLogoutTimer(uint32 ms)
    {
        if(ms)  _logoutTime = m_currMsTime+ms;
        else    _logoutTime = 0;
    }

    void LogoutPlayer(bool Save);

    HEARTHSTONE_INLINE void QueuePacket(WorldPacket* packet)
    {
        m_lastPing = (uint32)UNIXTIME;
        _recvQueue.Push(packet);
    }

    HEARTHSTONE_INLINE WorldSocket* GetSocket() { return _socket; }

    void Disconnect()
    {
        if(_socket && _socket->IsConnected())
            _socket->Disconnect();
    }

    int __fastcall Update(uint32 InstanceID);

    bool IsHighPriority();
    void SendItemPushResult(Item* pItem, bool Created, bool Received, bool SendToSet, bool NewItem, uint8 DestBagSlot, uint32 DestSlot, uint32 AddCount);
    void SendBuyFailed(uint64 guid, uint32 itemid, uint8 error);
    void SendSellItem(uint64 vendorguid, uint64 itemid, uint8 error);
    void SendNotification(const char *message, ...);

    HEARTHSTONE_INLINE void SetInstance(uint32 Instance) { instanceId = Instance; }
    HEARTHSTONE_INLINE uint32 GetLatency() { return _latency; }
    HEARTHSTONE_INLINE string GetAccountName() { return _accountName; }
    HEARTHSTONE_INLINE const char * GetAccountNameS() { return _accountName.c_str(); }

    HEARTHSTONE_INLINE uint32 GetClientBuild() { return client_build; }
    HEARTHSTONE_INLINE void SetClientBuild(uint32 build) { client_build = build; }
    bool bDeleted;
    HEARTHSTONE_INLINE uint32 GetInstance() { return instanceId; }
    Mutex deleteMutex;
    void _HandleAreaTriggerOpcode(uint32 id);//real handle
    int32 m_moveDelayTime;
    int32 m_clientTimeDelay;

    void CharacterEnumProc(QueryResult * result);
    HEARTHSTONE_INLINE bool IsLoggingOut() { return _loggingOut; }

    bool CanCreateDeathKnight()
    {
        if( sWorld.m_deathKnightOnePerAccount && m_hasDeathKnight )
            return false;

        if( sWorld.m_deathKnightReqLevel > m_highestLevel )
            return false;

        return true;
    }

    uint32 GetTutorialFlag(uint8 i)
    {
        ASSERT(i < 8);
        return m_tutorials[i];
    }

protected:
    bool m_hasDeathKnight;
    uint8 m_highestLevel;
    uint8 DeleteCharacter(uint32 guid);

    /// Login screen opcodes (PlayerHandler.cpp):
    void HandleCharEnumOpcode(WorldPacket& recvPacket);
    void HandleCharDeleteOpcode(WorldPacket& recvPacket);
    void HandleCharCreateOpcode(WorldPacket& recvPacket);
    void HandleCharCustomizeOpcode(WorldPacket& recvPacket);
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
    void HandleAreaTriggerOpcode(WorldPacket& recvPacket);
    void HandleUpdateAccountData(WorldPacket& recvPacket);
    void HandleRequestAccountData(WorldPacket& recvPacket);
    void HandleSetActionButtonOpcode(WorldPacket& recvPacket);
    void HandleSetAtWarOpcode(WorldPacket& recvPacket);
    void HandleSetWatchedFactionIndexOpcode(WorldPacket& recvPacket);
    void HandleTogglePVPOpcode(WorldPacket& recvPacket);
    void HandleGameObjectUse(WorldPacket& recvPacket);
    //void HandleJoinChannelOpcode(WorldPacket& recvPacket);
    //void HandleLeaveChannelOpcode(WorldPacket& recvPacket);
    void HandlePlayedTimeOpcode(WorldPacket & recv_data);
    void HandleSetSheathedOpcode(WorldPacket & recv_data);
    void HandleCompleteCinematic(WorldPacket & recv_data);
    void HandleInspectOpcode( WorldPacket & recv_data );
    void HandleGameobjReportUseOpCode( WorldPacket& recv_data );
    void HandleTimeSyncResp(WorldPacket& recv_data);

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
    void HandleMoveWorldportAckOpcode( WorldPacket& recvPacket );
    void HandleMovementOpcodes( WorldPacket& recvPacket );
    void HandleMoveFallResetOpcode( WorldPacket & recvPacket);
    void HandleMoveTimeSkippedOpcode( WorldPacket & recv_data );
    void HandleSetActiveMoverOpcode( WorldPacket & recv_data );
    void HandleMoveTeleportAckOpcode( WorldPacket & recv_data );
    void HandleMoveKnockbackAckOpcode( WorldPacket & recv_data );
    void HandleMoveHoverWaterFlyAckOpcode( WorldPacket & recv_data );
    void HandleAcknowledgementOpcodes( WorldPacket & recv_data );
    void HandleForceSpeedChangeOpcodes( WorldPacket & recv_data );

    /// Opcodes implemented in GroupHandler.cpp:
    void HandleGroupInviteOpcode(WorldPacket& recvPacket);
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
    void HandleInviteToGuild(WorldPacket & recv_data);
    void HandleGuildAccept(WorldPacket & recv_data);
    void HandleGuildDecline(WorldPacket & recv_data);
    void HandleGuildInfo(WorldPacket & recv_data);
    void HandleGuildRoster(WorldPacket & recv_data);
    void HandleGuildPromote(WorldPacket & recv_data);
    void HandleGuildDemote(WorldPacket & recv_data);
    void HandleGuildLeave(WorldPacket & recv_data);
    void HandleGuildRemove(WorldPacket & recv_data);
    void HandleGuildDisband(WorldPacket & recv_data);
    void HandleGuildLeader(WorldPacket & recv_data);
    void HandleGuildMotd(WorldPacket & recv_data);
    void HandleGuildEditRank(WorldPacket & recv_data);
    void HandleGuildAddRank(WorldPacket & recv_data);
    void HandleGuildDelRank(WorldPacket & recv_data);
    void HandleGuildSetPublicNote(WorldPacket & recv_data);
    void HandleGuildSetOfficerNote(WorldPacket & recv_data);
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
    void HandleGuildGetFullPermissions(WorldPacket & recv_data);

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
    void HandleMoveSplineCompleteOpcode(WorldPacket &recvPacket);

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

    uint8 TrainerGetSpellStatus(TrainerSpell* pSpell);
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

    //Vehicles
    void HandleVehicleDismiss(WorldPacket & recv_data);
    void HandleSpellClick( WorldPacket & recv_data );
    void HandleRequestSeatChange( WorldPacket & recv_data );
    void HandleEjectPassenger( WorldPacket & recv_data );
    void HandleVehicleMountEnter( WorldPacket & recv_data );

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
    void SendInventoryList(Creature* pCreature);
    void SendTrainerList(Creature* pCreature);
    void SendCharterRequest(Creature* pCreature);
    void SendTaxiList(Creature* pCreature);
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
    string _accountName;
    uint32 _logoutTime; // time we received a logout request -- wait 20 seconds, and quit

    WoWGuid m_MoverWoWGuid;

    FastQueue<WorldPacket*, Mutex> _recvQueue;
    char *permissions;
    int permissioncount;

    bool _loggingOut;
    bool _recentlogout;
    uint32 _latency;
    uint32 client_build;
    uint32 instanceId;
    uint8 _updatecount;

    // Data
    void SendAccountDataTimes(uint8 mask);
    void LoadAccountData();
    void SaveAccountData();
    AccountDataEntry *m_accountData[8];

    // Tutorials
    void LoadTutorials();
    void SaveTutorials();
    uint32 m_tutorials[8];

public:
    void ValidateText1(std::string text)
    {
        if(!ValidateText2(text))
        {

        }
    };

    bool ValidateText2(std::string text);
    uint8 CheckTeleportPrerequisites(AreaTrigger * pAreaTrigger, WorldSession * pSession, Player* pPlayer, uint32 mapid);
    static void InitPacketHandlerTable();
    uint32 floodLines;
    time_t floodTime;
    void SystemMessage(const char * format, ...);
    WorldPacket* BuildQuestQueryResponse(Quest *qst);
    uint32 m_muted;
    uint32 m_lastWhoTime;
    bool m_asyncQuery;

    void SendGossipForObject(Object* pObject);

protected:
    uint32 m_repeatTime;
    string m_repeatMessage;
    uint32 m_repeatEmoteTime;
    uint32 m_repeatEmoteId;
};

void BuildCorpseInfo(WorldPacket* data, Corpse* corpse);
typedef std::set<WorldSession*> SessionSet;
