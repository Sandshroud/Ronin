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

#include "StdAfx.h"

initialiseSingleton(GossipManager);

GossipManager::GossipManager() : Singleton<GossipManager>(), m_gossipOptGuidCounter(GOSSIP_OPT_START)
{

}

GossipManager::~GossipManager()
{
    while(m_dbGossipPOIs.size())
    {
        DatabaseGossipPOI *gossipPOI = m_dbGossipPOIs.begin()->second;
        m_dbGossipPOIs.erase(m_dbGossipPOIs.begin());
        delete gossipPOI;
    }

    while(m_dbGossipMenus.size())
    {
        DatabaseGossipMenu *gossipMenu = m_dbGossipMenus.begin()->second;
        m_dbGossipMenus.erase(m_dbGossipMenus.begin());
        while(gossipMenu->gossipOptions.size())
        {
            DatabaseGossipOptions *gossipOption = gossipMenu->gossipOptions.begin()->second;
            gossipMenu->gossipOptions.erase(gossipMenu->gossipOptions.begin());
            delete gossipOption;
        }
        delete gossipMenu;
    }
}

void GossipManager::LoadGossipData()
{
    QueryResult *result = NULL;
    if(result = WorldDatabase.Query("SELECT * FROM gossip_poi_points"))
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 entryId = fields[0].GetUInt32();
            if(m_dbGossipPOIs.find(entryId) != m_dbGossipPOIs.end())
                return;

            DatabaseGossipPOI *gossipPOI = new DatabaseGossipPOI();
            gossipPOI->poiId = entryId;
            gossipPOI->poiName.append(fields[1].GetString());
            gossipPOI->x = fields[2].GetFloat();
            gossipPOI->y = fields[3].GetFloat();
            gossipPOI->flags = fields[3].GetUInt16();
            m_dbGossipPOIs.insert(std::make_pair(entryId, gossipPOI));
        }
        while(result->NextRow());
        delete result;
    }

    if(result = WorldDatabase.Query("SELECT * FROM gossip_menu_data ORDER BY menuId, menuText"))
    {
        DatabaseGossipMenu *gossipMenu = NULL;
        std::map<uint32, DatabaseGossipMenu*>::iterator menu_itr;
        do
        {
            Field *fields = result->Fetch();
            uint32 MenuID = fields[0].GetUInt32();
            if((menu_itr = m_dbGossipMenus.find(MenuID)) == m_dbGossipMenus.end())
            {
                gossipMenu = new DatabaseGossipMenu();
                gossipMenu->menuEntry = MenuID;
                m_dbGossipMenus.insert(std::make_pair(MenuID, gossipMenu));
            } else gossipMenu = menu_itr->second;

            gossipMenu->text_ids.insert(fields[1].GetUInt32());
        }
        while(result->NextRow());
        delete result;
    }

    if(result = WorldDatabase.Query("SELECT * FROM gossip_menu_options ORDER BY menuId, orderId"))
    {
        std::map<uint32, DatabaseGossipMenu*>::iterator menu_itr;
        do
        {
            Field *fields = result->Fetch();
            uint32 MenuID = fields[0].GetUInt32();
            uint32 OrderId = fields[1].GetUInt32();
            uint8 optionId = fields[4].GetUInt8();
            if(optionId == 0)
                continue;
            if((menu_itr = m_dbGossipMenus.find(MenuID)) == m_dbGossipMenus.end())
                continue;

            DatabaseGossipOptions *option = new DatabaseGossipOptions();
            option->menuId = MenuID;
            option->orderId = OrderId;
            option->optionIcon = fields[2].GetUInt16();
            option->gossipText = fields[3].GetString();
            option->optionId = optionId;
            option->optionNPCFlag = fields[5].GetUInt32();
            option->actionMenuId = fields[6].GetInt32();
            option->menuPoiID = fields[7].GetUInt32();
            menu_itr->second->gossipOptions.insert(std::make_pair(OrderId, option));
        }
        while(result->NextRow());
        delete result;
    }
}

void GossipManager::BuildGossipMessage(WorldPacket *packet, Player *plr, Object *obj)
{
    uint32 count = 0, menuId = obj->GetEntry(), textId = 68;
    *packet << uint64(obj->GetGUID()); // NPC guid
    size_t sizePos = packet->wpos();
    *packet << uint32(menuId); // Menu Id
    *packet << uint32(textId); // Text ID
    *packet << uint32(count); // Gossip counter
    // Custom gossip menu or basic gossip menu appending
    GossipMenu *menu = obj->IsCreature() ? _getCreatureGossipOverride(obj->GetEntry()) : obj->IsGameObject() ? _getGoGossipOverride(obj->GetEntry()) : NULL;
    if(menu == NULL || !menu->BlocksBasicMenu())
        count += _BuildBasicGossipMenu(packet, menuId, textId, plr, obj);
    if (menu != NULL)
        count += menu->BuildGossipMenu(packet, menuId, textId, plr, obj);
    packet->put<uint32>(sizePos, menuId);
    packet->put<uint32>(sizePos+4, textId);
    packet->put<uint32>(sizePos+8, count);

    // Append our questgiver data from here on
    sizePos = packet->wpos();
    *packet << uint32(count = 0); // Quest counter
    sQuestMgr.AppendQuestList(obj, plr, count, packet);
    packet->put<uint32>(sizePos, count);
}

void GossipManager::HandleGossipOptionSelect(uint32 menuId, uint32 optionGuid, Player *plr, Object *obj, const char *coded)
{
    uint32 newMenuId = menuId, textId = 68;
    DatabaseGossipOptions *gossipOptions = NULL;
    Creature *ctrObj = obj->IsCreature() ? castPtr<Creature>(obj) : NULL;
    if(ctrObj && _hasDatabaseGossipMenu(ctrObj->GetGossipId()))
    {
        if((gossipOptions = _acquireDatabaseGossipInfo(newMenuId, textId, optionGuid, ctrObj, plr)) == NULL || menuId == -1)
        {
            if(WorldSession *session = plr->GetSession())
                session->OutPacket(SMSG_GOSSIP_COMPLETE);
            return;
        }
    }

    bool switchedMenus = menuId != newMenuId;
    switch (optionGuid)
    {
    case GOSSIP_OPT_NEXT_MENU:
        if(newMenuId)
            _BuildSecondaryGossipMenu(obj, switchedMenus ? newMenuId : menuId, textId, plr);
        else
        {
            if(WorldSession *session = plr->GetSession())
                session->OutPacket(SMSG_GOSSIP_COMPLETE);
            return;
        }
        break;

    case GOSSIP_OPT_VENDOR: if(ctrObj) ctrObj->SendInventoryList(plr); break;
    case GOSSIP_OPT_FLIGHT: if(ctrObj) ctrObj->SendTaxiList(plr); break;
    case GOSSIP_OPT_INNKEEPER: if (ctrObj) plr->GetSession()->SendInnkeeperBind(ctrObj); break;
    case GOSSIP_OPT_TOGGLE_XPGAIN: { plr->SendXPToggleConfirm(); }break;

    case GOSSIP_OPT_AUCTION:
        if(AuctionHouse *auctionHouse = ctrObj ? ctrObj->auctionHouse : NULL)
            auctionHouse->SendAuctionHello(ctrObj->GetGUID(), plr);
        break;
    case GOSSIP_OPT_BANKER:
        {
            WorldPacket data(SMSG_SHOW_BANK, 8);
            data << obj->GetGUID();
            plr->PushPacket(&data);
        }break;
    case GOSSIP_OPT_SPIRITHEALER:
        // Make menu
        break;
    case GOSSIP_OPT_CHARTER:
        //plr->GetSession()->SendPetitionShowList(guid);
        break;
    case GOSSIP_OPT_TABARD:
        //plr->GetSession()->SendTabardVendorActivate(guid);
        break;
    case GOSSIP_OPT_BATTLEMASTER:
        //plr->GetSession()->SendBattlegGroundList(guid, bgTypeId);
        break;
    case GOSSIP_OPT_TRAINER:
        {
            if (ctrObj)
            {
                WorldPacket data(SMSG_TRAINER_LIST, 5000);
                ctrObj->BuildTrainerData(&data, plr);
                plr->PushPacket(&data);
            }
        }break;
    case GOSSIP_OPT_TALENT_RESET:
        {
            // Send talent reset confirmation
            if(!plr->SendTalentResetConfirm())
            {
                // Send gossip complete opcode
                WorldPacket data(SMSG_GOSSIP_COMPLETE, 8);
                plr->PushPacket(&data);
            }
        }break;
    case GOSSIP_OPT_DUAL_TALENT_CONFIRM:
        {
            uint32 playerGold = plr->GetUInt32Value(PLAYER_FIELD_COINAGE), price = 10000000;
            if (playerGold >= price)
            {
                SpellEntry* learn = dbcSpell.LookupEntry(63680), * visual = dbcSpell.LookupEntry(63624);
                if (learn && visual)
                {
                    // Remove gold, confirm.
                    plr->SetUInt32Value(PLAYER_FIELD_COINAGE, playerGold - price);

                    plr->GetSpellInterface()->TriggerSpell(learn, plr);
                    plr->GetSpellInterface()->TriggerSpell(visual, plr);
                }

                _BuildSecondaryGossipMenu(obj, GOSSIP_OPT_DUAL_TALENT_CONFIRM, textId, plr);
            }
            else
            {
                // Send gossip complete opcode
                WorldPacket data(SMSG_GOSSIP_COMPLETE, 8);
                plr->PushPacket(&data);
            }
        }break;
    case GOSSIP_OPT_SEND_POI:
        {
            std::map<uint32, DatabaseGossipPOI*>::iterator itr;
            if(gossipOptions && (itr = m_dbGossipPOIs.find(gossipOptions->menuPoiID)) != m_dbGossipPOIs.end())
            {
                WorldPacket data(SMSG_GOSSIP_POI, 50);
                data << uint32(itr->second->flags);
                data << float(itr->second->x);
                data << float(itr->second->y);
                data << uint32(7) << uint32(0);
                data << itr->second->poiName;
                plr->PushPacket(&data);
            }

            if(switchedMenus && newMenuId && newMenuId != 68)
                _BuildSecondaryGossipMenu(obj, newMenuId, textId, plr);
            else
            {
                // Send gossip complete opcode
                WorldPacket data(SMSG_GOSSIP_COMPLETE, 8);
                plr->PushPacket(&data);
            }
        }break;
    }
}

size_t GossipManager::_BuildBasicGossipMenu(WorldPacket *packet, uint32 &menuId, uint32 &textId, Player *plr, Object *obj)
{
    size_t result = 0;
    switch(obj->GetTypeId())
    {
    case TYPEID_UNIT:
        {
            if(Creature* pCreature = castPtr<Creature>(obj))
            {
                // Set our gossip menu ID
                if(uint32 MenuId = pCreature->GetGossipId())
                    menuId = MenuId;

                if(_hasDatabaseGossipMenu(menuId) && result == 0)
                {
                    result += _buildDatabaseGossipMenu(menuId, packet, textId, pCreature, plr);
                    return result;
                }

                uint32 flags = pCreature->GetUInt32Value(UNIT_NPC_FLAGS);
                if( flags & UNIT_NPC_FLAG_VENDOR && !pCreature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE))
                    _AddMenuItem(packet, result, GOSSIP_OPT_VENDOR, GOSSIP_ICON_GOSSIP_VENDOR, "I would like to browse your goods");

                if(flags & UNIT_NPC_FLAG_TAXIVENDOR)
                    _AddMenuItem(packet, result, GOSSIP_OPT_FLIGHT, GOSSIP_ICON_GOSSIP_FLIGHT, "Give me a ride.");

                if(flags & UNIT_NPC_FLAG_AUCTIONEER)
                    _AddMenuItem(packet, result, GOSSIP_OPT_AUCTION, GOSSIP_ICON_GOSSIP_AUCTION, "I would like to make a bid.");

                if(flags & UNIT_NPC_FLAG_BANKER)
                    _AddMenuItem(packet, result, GOSSIP_OPT_BANKER, GOSSIP_ICON_GOSSIP_COIN, "I would like to check my deposit box.");

                if(flags & UNIT_NPC_FLAG_SPIRITHEALER || sCreatureDataMgr.IsSpiritHealer(pCreature->GetCreatureData()))
                    _AddMenuItem(packet, result, GOSSIP_OPT_SPIRITHEALER, GOSSIP_ICON_GOSSIP_NORMAL, "Bring me back to life.");

                if(flags & UNIT_NPC_FLAG_ARENACHARTER)
                    _AddMenuItem(packet, result, GOSSIP_OPT_CHARTER, GOSSIP_ICON_GOSSIP_ARENA, "How do I create a guild/arena team?");

                if(flags & UNIT_NPC_FLAG_TABARDCHANGER)
                    _AddMenuItem(packet, result, GOSSIP_OPT_TABARD, GOSSIP_ICON_GOSSIP_TABARD, "I want to create a guild crest.");

                if(flags & UNIT_NPC_FLAG_BATTLEFIELDPERSON && false)//pCreature->CanBattleMasterPlayer(plr))
                    _AddMenuItem(packet, result, GOSSIP_OPT_BATTLEMASTER, GOSSIP_ICON_GOSSIP_ARENA, "I would like to enter the battleground.");

                if(flags & UNIT_NPC_FLAG_INNKEEPER)
                {
                    _AddMenuItem(packet, result, GOSSIP_OPT_INNKEEPER, GOSSIP_ICON_GOSSIP_ENGINEER2, "Make this inn your home.");
                    _AddMenuItem(packet, result, GOSSIP_OPT_NEXT_MENU, GOSSIP_ICON_GOSSIP_NORMAL, "What can I do at an Inn.");
                }

                if((flags & (UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_TRAINER_PROF)) && pCreature->CanTrainPlayer(plr))
                {
                    std::string trainingType = pCreature->GetTrainerType();

                    // If our training type is not empty, append a space
                    if(trainingType.empty() == false)
                        trainingType.append(" ");

                    _AddMenuItem(packet, result, GOSSIP_OPT_TRAINER, GOSSIP_ICON_GOSSIP_TRAINER, format("I seek %straining %s.", trainingType.c_str(), pCreature->GetName()).c_str());
                    if(pCreature->IsClassTrainer())
                    {
                        if(pCreature->getLevel() > 10 && plr->getLevel() >= 10)
                        {
                            _AddMenuItem(packet, result, GOSSIP_OPT_TALENT_RESET, GOSSIP_ICON_GOSSIP_NORMAL, "I would like to reset my talents.");
                            if( plr->getLevel() >= 40 && plr->GetTalentInterface()->GetSpecCount() < 2)
                                _AddMenuItem(packet, result, GOSSIP_OPT_NEXT_MENU, GOSSIP_ICON_GOSSIP_NORMAL, "Learn about Dual Talent Specialization.");
                        }
                    } else if(pCreature->IsPetTrainer())
                        _AddMenuItem(packet, result, GOSSIP_OPT_TALENT_RESET, GOSSIP_ICON_GOSSIP_NORMAL, "I would like to untrain my pet.");
                }

                if( pCreature->GetEntry() == 35364 || pCreature->GetEntry() == 35365 )
                {
                    if(plr->getLevel() >= 10 && plr->getLevel() < plr->GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
                    {
                        if(plr->m_XPoff)
                            _AddMenuItem(packet, result, GOSSIP_OPT_TOGGLE_XPGAIN, GOSSIP_ICON_GOSSIP_NORMAL, "I wish to start gaining experience again.", false, 10000, "Are you certain you wish to start gaining experience?");
                        else _AddMenuItem(packet, result, GOSSIP_OPT_TOGGLE_XPGAIN, GOSSIP_ICON_GOSSIP_NORMAL, "I no longer wish to gain experience.", false, 10000, "Are you certain you wish to stop gaining experience?");
                    }
                }
            }
        }break;
    case TYPEID_GAMEOBJECT:
        {
            if(GameObject *gObj = castPtr<GameObject>(obj))
            {
                switch(gObj->GetType())
                {
                case GAMEOBJECT_TYPE_GOOBER:
                    {
                        if(uint32 gossipId = gObj->GetInfo()->data.goober.gossipID)
                            textId = gossipId;
                    }break;
                }
            }
        }break;
    }

    return result;
}

void GossipManager::_BuildSecondaryGossipMenu(Object* obj, uint32 menuId, uint32 textId, Player* plr)
{
    WorldPacket data(SMSG_GOSSIP_MESSAGE, 500);

    size_t count = 0;

    data << uint64(obj->GetGUID()); // NPC guid
    data << uint32(menuId); // Menu Guid
    size_t sizePos = data.wpos();
    data << uint32(textId); // Text ID
    data << uint32(count); // Gossip counter

    switch (obj->GetTypeId())
    {
    case TYPEID_UNIT:
        {
            if (Creature* pCreature = castPtr<Creature>(obj))
            {
                if(_hasDatabaseGossipMenu(menuId))
                    count += _buildDatabaseGossipMenu(menuId, &data, textId, pCreature, plr);
                else
                {
                    uint32 flags = pCreature->GetUInt32Value(UNIT_NPC_FLAGS);
                    if (menuId == GOSSIP_OPT_DUAL_TALENT_CONFIRM)
                        textId = 14393; // Dual talent confirmation menu
                    else if ((flags & (UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_TRAINER_PROF)) && pCreature->CanTrainPlayer(plr) && plr->getLevel() >= 40 && plr->GetTalentInterface()->GetSpecCount() < 2)
                    {
                        textId = 14391; // Dual talent information
                        _AddMenuItem(&data, count, GOSSIP_OPT_DUAL_TALENT_CONFIRM, GOSSIP_ICON_GOSSIP_NORMAL, "Purchase a Dual Talent Specialization.", false, 10000000, "Are you sure you would like to purchase your second talent specialization?");
                    }
                }
            }
        }break;
    case TYPEID_GAMEOBJECT:
        {
            if (GameObject* gObj = castPtr<GameObject>(obj))
            {
                switch (gObj->GetType())
                {
                case GAMEOBJECT_TYPE_GOOBER:
                    {
                        if (uint32 gossipId = 0)//GetGossipIdChain(menuId, optionId, gObj->GetInfo()->data.goober.gossipID))
                            textId = gossipId;
                    }break;
                }
            }
        }break;
    }

    data.put<uint32>(sizePos, textId);
    data.put<uint32>(sizePos + 4, count);
    // Append our questgiver data from here on
    sizePos = data.wpos();
    data << uint32(0); // Quest counter
    plr->PushPacket(&data);
}

void GossipManager::_AddMenuItem(ByteBuffer *buffer, size_t &counter, uint32 optionId, uint8 Icon, std::string text, bool codeBox, uint32 boxMoney, std::string boxMessage)
{
    *buffer << uint32(optionId);
    *buffer << uint8(Icon);
    *buffer << uint8(codeBox);
    *buffer << uint32(boxMoney);
    *buffer << text << boxMessage;
    counter++;
}

bool GossipManager::_hasDatabaseGossipMenu(uint32 menuId)
{
    return m_dbGossipMenus.find(menuId) != m_dbGossipMenus.end();
}

size_t GossipManager::_buildDatabaseGossipMenu(uint32 menuId, WorldPacket *packet, uint32 &textId, Creature *ctr, Player *plr)
{
    std::map<uint32, DatabaseGossipMenu*>::iterator itr;
    if((itr = m_dbGossipMenus.find(menuId)) == m_dbGossipMenus.end())
        return 0;
    textId = itr->second->text_ids.size() ? (*itr->second->text_ids.begin()) : 68;

    size_t result = 0;
    std::map<uint8, DatabaseGossipOptions*>::iterator itr2 = itr->second->gossipOptions.begin();
    for(itr2; itr2 != itr->second->gossipOptions.end(); ++itr2)
    {
        // Check NPC flags
        if(itr2->second->optionNPCFlag && !ctr->HasNpcFlag(itr2->second->optionNPCFlag))
            continue;

        _AddMenuItem(packet, result, itr2->second->orderId, itr2->second->optionIcon, itr2->second->gossipText.c_str());
    }

    return result;
}

DatabaseGossipOptions *GossipManager::_acquireDatabaseGossipInfo(uint32 &menuId, uint32 &textId, uint32 &optionGuid, Creature *ctr, Player *plr)
{
    DatabaseGossipOptions *optionFound = NULL;
    std::map<uint32, DatabaseGossipMenu*>::iterator itr;
    if((itr = m_dbGossipMenus.find(menuId)) != m_dbGossipMenus.end())
    {
        std::map<uint8, DatabaseGossipOptions*>::iterator itr2;
        if((itr2 = itr->second->gossipOptions.find(optionGuid)) != itr->second->gossipOptions.end())
        {
            optionFound = itr2->second;
            if(optionFound->actionMenuId)
            {
                menuId = optionFound->actionMenuId;
                optionGuid = GOSSIP_OPT_NEXT_MENU;
            } else optionGuid = optionFound->optionId;

            if((itr = m_dbGossipMenus.find(menuId)) != m_dbGossipMenus.end())
                textId = itr->second->text_ids.size() ? (*itr->second->text_ids.begin()) : 68;

            if(optionFound->menuPoiID)
                optionGuid = GOSSIP_OPT_SEND_POI;

        }
    }
    return optionFound;
}
