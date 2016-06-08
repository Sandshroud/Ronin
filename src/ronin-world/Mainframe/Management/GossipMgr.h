/***
 * Demonstrike Core
 */

#pragma once

enum GossipOptionGuids
{
    GOSSIP_OPT_NEXT_MENU    = 1,
    GOSSIP_OPT_VENDOR       = 2,
    GOSSIP_OPT_FLIGHT       = 3,
    GOSSIP_OPT_AUCTION      = 4,
    GOSSIP_OPT_BANKER       = 5,
    GOSSIP_OPT_SPIRITHEALER = 6,
    GOSSIP_OPT_CHARTER      = 7,
    GOSSIP_OPT_TABARD       = 8,
    GOSSIP_OPT_BATTLEMASTER = 9,
    GOSSIP_OPT_INNKEEPER    = 10,
    GOSSIP_OPT_TRAINER      = 11,
    GOSSIP_OPT_TALENT_RESET = 12,
    GOSSIP_OPT_START
};

class SERVER_DECL GossipManager : public Singleton < GossipManager >
{
public:
    GossipManager();
    ~GossipManager();

    void LoadGossipData();

    void BuildGossipMessage(WorldPacket *packet, Player *plr, Object *obj);
    void HandleGossipOptionSelect(uint32 menuId, uint32 optionGuid, Player *plr, Object *obj, const char *coded);

    uint32 GenerateGossipOptionGuid()
    {
        gossipOptLock.Acquire();
        uint32 gossipOptGuid = m_gossipOptGuidCounter++;
        gossipOptLock.Release();
        return gossipOptGuid;
    }

private:
    size_t _BuildBasicGossipMenu(WorldPacket *packet, uint32 &textId, Player *plr, Object *obj);

    void _AddMenuItem(WorldPacket *packet, size_t &counter, uint32 guid, uint8 Icon, std::string text, bool codeBox = false, uint32 boxMoney = 0, std::string boxMessage = "");

protected:
    Mutex gossipOptLock;
    uint32 m_gossipOptGuidCounter;
};

#define sGossipMgr GossipManager::getSingleton()

class SERVER_DECL GossipMenu
{
public:
    bool BlocksBasicMenu() { return false; }

};
