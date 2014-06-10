/***
 * Demonstrike Core
 */

#pragma once

enum MailCMD
{
    MAIL_RES_MAIL_SENT = 0,
    MAIL_RES_MONEY_TAKEN = 1,
    MAIL_RES_ITEM_TAKEN = 2,
    MAIL_RES_RETURNED_TO_SENDER = 3,
    MAIL_RES_DELETED = 4,
    MAIL_RES_MADE_PERMANENT = 5
};

enum MailFlags
{
    MAIL_FLAG_NO_COST_FOR_GM                    = 1,
    MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION    = 2,
    MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION_GM   = 4,
    MAIL_FLAG_DISABLE_POSTAGE_COSTS          = 8,
    MAIL_FLAG_DISABLE_HOUR_DELAY_FOR_ITEMS    = 16,
    MAIL_FLAG_NO_EXPIRY                      = 32,
};

enum MailTypes
{
    MAILTYPE_NORMAL,
    MAILTYPE_COD,
    MAILTYPE_AUCTION,
    MAILTYPE_CREATURE,
    MAILTYPE_GAMEOBJECT,
    MAILTYPE_ITEM
};

enum MailStationery
{
    STATIONERY_UNK      =  1,
    STATIONERY_NORMAL   = 41, // Normal Stationary.
    STATIONERY_GM       = 61, // Sent from Gamemaster.
    STATIONERY_AUCTION  = 62, // Auctionhouse Stationary.
    STATIONERY_UNK2     = 64, // No clue.
    STATIONERY_UNK3     = 65, // No clue.
    STATIONERY_UNK4     = 67, // Sent from Orphan.
};

enum MailError
{
    MAIL_OK = 0,
    MAIL_ERR_BAG_FULL = 1,
    MAIL_ERR_CANNOT_SEND_TO_SELF = 2,
    MAIL_ERR_NOT_ENOUGH_MONEY = 3,
    MAIL_ERR_RECIPIENT_NOT_FOUND = 4,
    MAIL_ERR_NOT_YOUR_ALLIANCE = 5,
    MAIL_ERR_INTERNAL_ERROR = 6
};

struct MailMessage
{
    uint32 message_id;
    uint32 message_type;
    uint64 player_guid;
    uint64 sender_guid;
    string subject;
    string body;
    uint32 money;
    vector<uint64> items;
    uint32 cod;
    uint32 stationary;
    uint32 expire_time;
    uint32 delivery_time;
    uint32 copy_made;
    bool read_flag;
    bool deleted_flag;
    bool returned_flag;
    bool LoadFromDB(Field * fields);
    void SaveToDB();
    void SaveToDBCallBack(QueryResultVector & results);
    bool Expired();
};

typedef map<uint32, MailMessage> MessageMap;

class SERVER_DECL Mailbox
{
private:
    bool AddMessageToListingPacket(WorldPacket& data, MailMessage *msg);
    bool AddMessageToTimePacket(WorldPacket& data, MailMessage *msg);
protected:
    uint64 owner;
    MessageMap Messages;

public:
    Mailbox(uint64 owner_) : owner(owner_) {}

    void AddMessage(MailMessage* Message);
    void DeleteMessage(MailMessage* Message);
    MailMessage * GetMessage(uint32 message_id)
    {
        MessageMap::iterator iter = Messages.find(message_id);
        if(iter == Messages.end())
            return NULL;
        return &(iter->second);
    }
    string GetMessageBody(uint32 message_id);
    WorldPacket * MailboxListingPacket();
    WorldPacket * MailboxTimePacket();
    HEARTHSTONE_INLINE size_t MessageCount() { return Messages.size(); }
    HEARTHSTONE_INLINE uint64 GetOwner() { return owner; }
    void Load(QueryResult * result);
    void OnMessageCopyDeleted(uint32 msg_id);
};


class SERVER_DECL MailSystem : public Singleton<MailSystem>, public EventableObject
{
public:
    MailSystem() { update_timer = config_flags = 0; };

    void StartMailSystem();
    void UpdateMessages(uint32 diff);
    void ReturnToSender(MailMessage* message);
    void DeliverMessage(MailMessage* message);
    void DeliverMessage(uint32 type, uint64 sender, uint64 receiver, string subject, string body, uint32 money, uint32 cod, uint64 item_guid, uint32 stationary, bool returned);

    void SetConfigFlags(uint32 flags) { config_flags = flags; };
    HEARTHSTONE_INLINE bool MailOption(uint32 flag) { return (config_flags & flag) ? true : false; }

private:
    uint32 update_timer;
    uint32 config_flags;
};

#define sMailSystem MailSystem::getSingleton()
