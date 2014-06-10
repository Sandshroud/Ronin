/***
 * Demonstrike Core
 */

#pragma once

#include "../hearthstone-shared/Client/OpcodeList.h"

class OpcodeManager : public Singleton<OpcodeManager>
{
public:
    OpcodeManager();
    ~OpcodeManager();

    void SetOpcodeListData();

    uint32 ConvertOpcodeForInput(uint32 opcode);
    uint32 ConvertOpcodeForOutput(uint32 opcode);
    const char* GetOpcodeName(uint32 opcode);

private:
    void SetOpcodeData(uint32 opcode, char* name, uint32 newOpcode);

    std::map<uint32, uint32> outputMap, inputMap;
    std::map<uint32, char*> opcodeNames;
};

#define sOpcodeMgr OpcodeManager::getSingleton()

enum FriendsResult {
    FRIEND_DB_ERROR             = 0x00,
    FRIEND_LIST_FULL            = 0x01,
    FRIEND_ONLINE               = 0x02,
    FRIEND_OFFLINE              = 0x03,
    FRIEND_NOT_FOUND            = 0x04,
    FRIEND_REMOVED              = 0x05,
    FRIEND_ADDED_ONLINE         = 0x06,
    FRIEND_ADDED_OFFLINE        = 0x07,
    FRIEND_ALREADY              = 0x08,
    FRIEND_SELF                 = 0x09,
    FRIEND_ENEMY                = 0x0A,
    FRIEND_IGNORE_FULL          = 0x0B,
    FRIEND_IGNORE_SELF          = 0x0C,
    FRIEND_IGNORE_NOT_FOUND     = 0x0D,
    FRIEND_IGNORE_ALREADY       = 0x0E,
    FRIEND_IGNORE_ADDED         = 0x0F,
    FRIEND_IGNORE_REMOVED       = 0x10,
    FRIEND_IGNORE_AMBIGUOUS     = 0x11,
    FRIEND_MUTE_FULL            = 0x12,
    FRIEND_MUTE_SELF            = 0x13,
    FRIEND_MUTE_NOT_FOUND       = 0x14,
    FRIEND_MUTE_ALREADY         = 0x15,
    FRIEND_MUTE_ADDED           = 0x16,
    FRIEND_MUTE_REMOVED         = 0x17,
    FRIEND_MUTE_AMBIGUOUS       = 0x18,
    FRIEND_UNK7                 = 0x19,
    FRIEND_UNKNOWN              = 0x1A
};

enum NPCFlags
{
    UNIT_NPC_FLAG_NONE              = 0x00000000,
    UNIT_NPC_FLAG_GOSSIP            = 0x00000001,
    UNIT_NPC_FLAG_QUESTGIVER        = 0x00000002,
    UNIT_NPC_FLAG_TRAINER           = 0x00000010,
    UNIT_NPC_FLAG_VENDOR            = 0x00000080,
    UNIT_NPC_FLAG_ARMORER           = 0x00001000,
    UNIT_NPC_FLAG_TAXIVENDOR        = 0x00002000,
    UNIT_NPC_FLAG_TRAINER_PROF      = 0x00004000,
    UNIT_NPC_FLAG_SPIRITHEALER      = 0x00008000,
    UNIT_NPC_FLAG_INNKEEPER         = 0x00010000,
    UNIT_NPC_FLAG_BANKER            = 0x00020000,
    UNIT_NPC_FLAG_ARENACHARTER      = 0x00040000,
    UNIT_NPC_FLAG_TABARDCHANGER     = 0x00080000,
    UNIT_NPC_FLAG_BATTLEFIELDPERSON = 0x00100000,
    UNIT_NPC_FLAG_AUCTIONEER        = 0x00200000,
    UNIT_NPC_FLAG_STABLEMASTER      = 0x00400000,
    UNIT_NPC_FLAG_GUILD_BANK        = 0x00800000,
    UNIT_NPC_FLAG_SPELLCLICK        = 0x01000000,
    UNIT_NPC_FLAG_VEHICLE_MOUNT     = 0x02000000,
};

enum GossipIcons 
{
    GOSSIP_ICON_GOSSIP_NORMAL       = 0,
    GOSSIP_ICON_GOSSIP_VENDOR       = 1,
    GOSSIP_ICON_GOSSIP_FLIGHT       = 2,
    GOSSIP_ICON_GOSSIP_TRAINER      = 3,
    GOSSIP_ICON_GOSSIP_ENGINEER1    = 4,
    GOSSIP_ICON_GOSSIP_ENGINEER2    = 5,
    GOSSIP_ICON_GOSSIP_AUCTION      = 6,
    GOSSIP_ICON_GOSSIP_EXTRA        = 7,
    GOSSIP_ICON_GOSSIP_TABARD       = 8,
    GOSSIP_ICON_GOSSIP_ARENA        = 9,
    GOSSIP_ICON_GOSSIP_COIN         = 10,
    GOSSIP_ICON_GOSSIP_NORMAL_11    = 11,
    GOSSIP_ICON_GOSSIP_NORMAL_12    = 12,
    GOSSIP_ICON_GOSSIP_NORMAL_13    = 13,
    GOSSIP_ICON_DONT_USE            = 14, // Reported to crash client?
    GOSSIP_ICON_DONT_USE2           = 15, // Reported to crash client?
    GOSSIP_ICON_GOSSIP_NORMAL_16    = 16,
    GOSSIP_ICON_GOSSIP_NORMAL_17    = 17,
    GOSSIP_ICON_GOSSIP_NORMAL_18    = 18,
    GOSSIP_ICON_GOSSIP_NORMAL_19    = 19,
    GOSSIP_ICON_GOSSIP_NORMAL_20    = 20,
};
