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

#pragma once

enum GossipOptionGuids
{
    GOSSIP_OPT_NEXT_MENU            = 1,
    GOSSIP_OPT_QUEST                = 2,
    GOSSIP_OPT_VENDOR               = 3,
    GOSSIP_OPT_FLIGHT               = 4,
    GOSSIP_OPT_TRAINER              = 5,
    GOSSIP_OPT_SPIRITHEALER         = 6,
    GOSSIP_OPT_SPIRITGUIDE          = 7,
    GOSSIP_OPT_INNKEEPER            = 8,
    GOSSIP_OPT_BANKER               = 9,
    GOSSIP_OPT_CHARTER              = 10,
    GOSSIP_OPT_TABARD               = 11,
    GOSSIP_OPT_BATTLEMASTER         = 12,
    GOSSIP_OPT_AUCTION              = 13,
    GOSSIP_OPT_PETSTABLES           = 14,
    GOSSIP_OPT_REPAIR_VENDOR        = 15,
    GOSSIP_OPT_TALENT_RESET         = 16,
    GOSSIP_OPT_PET_TALENT_RESET     = 17,
    GOSSIP_OPT_DUAL_TALENT_CONFIRM  = 18,
    GOSSIP_OPT_SEND_POI             = 19,
    GOSSIP_OPT_TOGGLE_XPGAIN        = 21,
    GOSSIP_OPT_START
};

struct DatabaseGossipOptions
{
    uint32 menuId;
    uint8 orderId;
    uint16 optionIcon;
    std::string gossipText;
    uint8 optionId;
    uint32 optionNPCFlag;
    int32 actionMenuId;
    uint32 menuPoiID;
};

struct DatabaseGossipMenu
{
    uint32 menuEntry;
    std::map<uint8, DatabaseGossipOptions*> gossipOptions;
    std::set<uint32> text_ids;
};

struct DatabaseGossipPOI
{
    uint32 poiId;
    std::string poiName;
    float x, y;
    uint16 flags;
};

class GossipMenu;

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

    void RegisterCreatureGossipOverride(uint32 entry, GossipMenu* menu)
    {
        if (m_creatureGossipOverride.find(entry) != m_creatureGossipOverride.end())
        {
            sLog.outError("Gossip script override registered for already existing entry %u", entry);
            return;
        }
        m_creatureGossipOverride.insert(std::make_pair(entry, menu));
    }

    void RegisterGOGossipOverride(uint32 entry, GossipMenu* menu)
    {
        if (m_goGossipOverride.find(entry) != m_goGossipOverride.end())
        {
            sLog.outError("Gossip script override registered for already existing entry %u", entry);
            return;
        }
        m_goGossipOverride.insert(std::make_pair(entry, menu));
    }

private:
    size_t _BuildBasicGossipMenu(WorldPacket *packet, uint32 &menuId, uint32 &textId, Player *plr, Object *obj);

    void _BuildSecondaryGossipMenu(Object *obj, uint32 menuId, uint32 textId, Player* plr);
    void _AddMenuItem(ByteBuffer *buffer, size_t &counter, uint32 optionId, uint8 Icon, std::string text, bool codeBox = false, uint32 boxMoney = 0, std::string boxMessage = "");

    // Database functions
    bool _hasDatabaseGossipMenu(uint32 menuId);
    size_t _buildDatabaseGossipMenu(uint32 menuId, WorldPacket *packet, uint32 &textId, Creature *ctr, Player *plr);
    DatabaseGossipOptions *_acquireDatabaseGossipInfo(uint32 &menuId, uint32 &textId, uint32 &optionGuid, Creature *ctr, Player *plr);

    // Scripted override functions
    GossipMenu* _getCreatureGossipOverride(uint32 entry)
    {
        std::map<uint32, GossipMenu*>::iterator itr;
        if ((itr = m_creatureGossipOverride.find(entry)) != m_creatureGossipOverride.end())
            return itr->second;
        return NULL;
    }

    GossipMenu* _getGoGossipOverride(uint32 entry)
    {
        std::map<uint32, GossipMenu*>::iterator itr;
        if ((itr = m_goGossipOverride.find(entry)) != m_goGossipOverride.end())
            return itr->second;
        return NULL;
    }

    // Directions gossip

protected:
    Mutex gossipOptLock;
    uint32 m_gossipOptGuidCounter;

protected:
    std::map<uint32, GossipMenu*> m_creatureGossipOverride, m_goGossipOverride;

    std::map<uint32, DatabaseGossipMenu*> m_dbGossipMenus;
    std::map<uint32, DatabaseGossipPOI*> m_dbGossipPOIs;
};

#define sGossipMgr GossipManager::getSingleton()

class SERVER_DECL GossipMenu
{
public:
    bool BlocksBasicMenu() { return false; }

    size_t BuildGossipMenu(WorldPacket* packet, uint32 &menuId, uint32& textId, Player* plr, Object* obj) { return 0; }
};
