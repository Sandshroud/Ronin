/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton(GossipManager);

GossipManager::GossipManager() : Singleton<GossipManager>(), m_gossipOptGuidCounter(GOSSIP_OPT_START)
{

}

GossipManager::~GossipManager()
{

}

void GossipManager::LoadGossipData()
{

}

void GossipManager::BuildGossipMessage(WorldPacket *packet, Player *plr, Object *obj)
{
    uint32 count = 0, textId = 68;
    *packet << uint64(obj->GetGUID()); // NPC guid
    *packet << uint32(obj->GetEntry()); // Menu Guid
    size_t sizePos = packet->wpos();
    *packet << uint32(textId); // Text ID
    *packet << uint32(count); // Gossip counter
    GossipMenu *menu = NULL;//GetScriptedGossipMenu(obj);
    if(menu == NULL || !menu->BlocksBasicMenu())
        count += _BuildBasicGossipMenu(packet, textId, plr, obj);
    //if(menu) count += menu->Append(packet, textId, plr);
    packet->put<uint32>(sizePos, textId);
    packet->put<uint32>(sizePos+4, count);

    // Append our questgiver data from here on
    sizePos = packet->wpos();
    *packet << uint32(count = 0); // Quest counter
    //count = sQuestMgr.AppendQuestList(packet, obj->GetGUID());
    packet->put<uint32>(sizePos, count);
}

void GossipManager::HandleGossipOptionSelect(uint32 menuId, uint32 optionGuid, Player *plr, Object *obj, const char *coded)
{
    Creature *ctrObj = obj->IsCreature() ? castPtr<Creature>(obj) : NULL;
    switch (optionGuid)
    {
    case GOSSIP_OPT_NEXT_MENU:
        break;
    case GOSSIP_OPT_VENDOR:
        ctrObj->SendInventoryList(plr);
        break;
    case GOSSIP_OPT_FLIGHT:
        ctrObj->SendTaxiList(plr);
        break;
    case GOSSIP_OPT_AUCTION:
        if(AuctionHouse *auctionHouse = ctrObj->auctionHouse)
            auctionHouse->SendAuctionHello(ctrObj->GetGUID(), plr);
        break;
    case GOSSIP_OPT_BANKER:
        {
            WorldPacket data(SMSG_SHOW_BANK, 8);
            data << obj->GetGUID();
            plr->SendPacket(&data);
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
    case GOSSIP_OPT_INNKEEPER:
        break;
    }
}

size_t GossipManager::_BuildBasicGossipMenu(WorldPacket *packet, uint32 &textId, Player *plr, Object *obj)
{
    size_t result = 0;
    switch(obj->GetTypeId())
    {
    case TYPEID_UNIT:
        {
            if(Creature* pCreature = castPtr<Creature>(obj))
            {
                /*if(uint32 textID = GetTextID<Creature>(pCreature->GetEntry()))
                    textId = textID;*/

                uint32 flags = pCreature->GetUInt32Value(UNIT_NPC_FLAGS);
                if( flags & UNIT_NPC_FLAG_VENDOR && !pCreature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE))
                    _AddMenuItem(packet, result, GOSSIP_OPT_VENDOR, GOSSIP_ICON_GOSSIP_VENDOR, "I would like to browse your goods");

                if(flags & UNIT_NPC_FLAG_TAXIVENDOR)
                    _AddMenuItem(packet, result, GOSSIP_OPT_FLIGHT, GOSSIP_ICON_GOSSIP_FLIGHT, "Give me a ride.");

                if(flags & UNIT_NPC_FLAG_AUCTIONEER)
                    _AddMenuItem(packet, result, GOSSIP_OPT_AUCTION, GOSSIP_ICON_GOSSIP_AUCTION, "I would like to make a bid.");

                if(flags & UNIT_NPC_FLAG_BANKER)
                    _AddMenuItem(packet, result, GOSSIP_OPT_BANKER, GOSSIP_ICON_GOSSIP_COIN, "I would like to check my deposit box.");

                if(flags & UNIT_NPC_FLAG_SPIRITHEALER || (pCreature->GetEntry() == 6491 || pCreature->GetEntry() == 32537))
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

                /*if((flags & (UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_TRAINER_PROF)) && pCreature->CanTrainPlayer(plr))
                {
                    _AddMenuItem(packet, result, GOSSIP_OPT_TRAINER, GOSSIP_ICON_GOSSIP_TRAINER, format("I seek %s training %s.", plr->getClassName().c_str(), pCreature->GetName()).c_str());
                    if(pCreature->IsClassTrainer())
                    {
                        _AddMenuItem(packet, result, GOSSIP_OPT_TALENT_RESET, GOSSIP_ICON_GOSSIP_NORMAL, "I would like to reset my talents.");
                        if( plr->getLevel() >= 40 && plr->m_talentInterface.GetSpecCount() < 2)
                            _AddMenuItem(packet, result, GOSSIP_OPT_NEXT_MENU, GOSSIP_ICON_GOSSIP_NORMAL, "Learn about Dual Talent Specialization.");
                    } else if(pCreature->IsPetTrainer())
                        _AddMenuItem(packet, result, GOSSIP_OPT_TALENT_RESET, GOSSIP_ICON_GOSSIP_NORMAL, "I would like to untrain my pet.");
                }*/
            }
        }break;
    }

    return result;
}

void GossipManager::_AddMenuItem(WorldPacket *packet, size_t &counter, uint32 guid, uint8 Icon, std::string text, bool codeBox, uint32 boxMoney, std::string boxMessage)
{
    *packet << uint32(guid);
    *packet << uint8(Icon);
    *packet << uint8(codeBox);
    *packet << uint32(boxMoney);
    *packet << text << boxMessage;
    counter++;
}
