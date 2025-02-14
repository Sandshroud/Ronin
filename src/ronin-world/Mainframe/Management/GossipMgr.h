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
    GOSSIP_OPT_VENDOR               = 2,
    GOSSIP_OPT_FLIGHT               = 3,
    GOSSIP_OPT_AUCTION              = 4,
    GOSSIP_OPT_BANKER               = 5,
    GOSSIP_OPT_SPIRITHEALER         = 6,
    GOSSIP_OPT_CHARTER              = 7,
    GOSSIP_OPT_TABARD               = 8,
    GOSSIP_OPT_BATTLEMASTER         = 9,
    GOSSIP_OPT_INNKEEPER            = 10,
    GOSSIP_OPT_TRAINER              = 11,
    GOSSIP_OPT_TALENT_RESET         = 12,
    GOSSIP_OPT_DUAL_TALENT_CONFIRM  = 13,
    GOSSIP_OPT_TOGGLE_XPGAIN        = 14,
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

    void _BuildSecondaryGossipMenu(Object *obj, uint32 menuId, Player* plr);
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
