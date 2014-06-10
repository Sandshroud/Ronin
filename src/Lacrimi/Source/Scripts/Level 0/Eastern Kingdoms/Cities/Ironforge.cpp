/*
 * Lacrimi Scripts Copyright 2010 - 2011
 *
 * ############################################################
 * # ##            #       ####### ####### ##    #    #    ## #
 * # ##           ###      ##      ##   ## ##   ###  ###   ## #
 * # ##          ## ##     ##      ##   ## ##   ###  ###   ## #
 * # ##         #######    ##      ####### ##  ## #### ##  ## #
 * # ##        ##     ##   ##      #####   ##  ## #### ##  ## #
 * # ##       ##       ##  ##      ##  ##  ## ##   ##   ## ## #
 * # ####### ##         ## ####### ##   ## ## ##   ##   ## ## #
 * # :::::::.::.........::.:::::::.::...::.::.::...::...::.:: #
 * ############################################################
 *
 */

#include "LacrimiStdAfx.h"

#define RHA_GOSSIP_ITEM_1 "I am ready to listen"
#define RHA_GOSSIP_ITEM_2 "That is tragic. How did this happen?"
#define RHA_GOSSIP_ITEM_3 "Interesting, continue please."
#define RHA_GOSSIP_ITEM_4 "Unbelievable! How dare they??"
#define RHA_GOSSIP_ITEM_5 "Of course I will help!"

class IronforgeGuard : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr, bool AutoSend)
    {
        GossipMenu* Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2760, plr);
        Menu->AddItem(0, "Auction House"            , 1);
        Menu->AddItem(0, "Bank of Ironforge"        , 2);
        Menu->AddItem(0, "Deeprun Tram"             , 3);
        Menu->AddItem(0, "Gryphon Master"           , 4);
        Menu->AddItem(0, "Guild Master"             , 5);
        Menu->AddItem(0, "The Inn"                  , 6);
        Menu->AddItem(0, "Mailbox"                  , 7);
        Menu->AddItem(0, "Stable Master"            , 8);
        Menu->AddItem(0, "Weapons Trainer"          , 9);
        Menu->AddItem(0, "Battlemaster"             , 10);
        Menu->AddItem(0, "Barber"                   , 11);
        Menu->AddItem(0, "Class Trainer"            , 12);
        Menu->AddItem(0, "Profession Trainer"       , 13);
        Menu->SendTo(plr);
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        GossipMenu* Menu;
        switch(IntId)
        {
        case 0:     // Return to start
            GossipHello(pObject, Plr, true);
            break;

            //////////////////////
            // Main menu handlers
            /////
        case 1:     // Auction House
            Plr->Gossip_SendPOI(-4957.39f, -911.6f, 7, 6, 0, "Ironforge Auction House");
            SendEmptyMenu(3014);
            break;

        case 2:     // Bank of Ironforge
            Plr->Gossip_SendPOI(-4891.91f, -991.47f, 7, 6, 0, "The Vault");
            SendEmptyMenu(2761);
            break;

        case 3:     // Deeprun Tram
            Plr->Gossip_SendPOI(-4835.27f, -1294.69f, 7, 6, 0, "Deeprun Tram");
            SendEmptyMenu(3814);
            break;

        case 4:     // Gryphon Master
            Plr->Gossip_SendPOI(-4821.52f, -1152.3f, 7, 6, 0, "Ironforge Gryphon Master");
            SendEmptyMenu(2762);
            break;

        case 5:     // Guild Master
            Plr->Gossip_SendPOI(-5021, -996.45f, 7, 6, 0, "Ironforge Visitor's Center");
            SendEmptyMenu(2764);
            break;

        case 6:     // The Inn
            Plr->Gossip_SendPOI(-4850.47f, -872.57f, 7, 6, 0, "Stonefire Tavern");
            SendEmptyMenu(2768);
            break;

        case 7:     // Mailbox
            Plr->Gossip_SendPOI(-4845.7f, -880.55f, 7, 6, 0, "Ironforge Mailbox");
            SendEmptyMenu(2769);
            break;

        case 8:     // Stable Master
            Plr->Gossip_SendPOI(-5010.2f, -1262, 7, 6, 0, "Ulbrek Firehand");
            SendEmptyMenu(5986);
            break;

        case 9:    // Weapon Trainer
            Plr->Gossip_SendPOI(-5040, -1201.88f, 7, 6, 0, "Bixi and Buliwyf");
            SendEmptyMenu(4518);
            break;

        case 10:    // Battlemaster
            Plr->Gossip_SendPOI(-5038.54f, -1266.44f, 7, 6, 0, "Battlemasters Ironforge");
            SendEmptyMenu(10216);
            break;

        case 11:    // Barber
            Plr->Gossip_SendPOI(-4838.49f, -919.18f, 7, 6, 0, "Ironforge Barber");
            SendEmptyMenu(13885);
            break;

        case 12:    // A class trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2766, Plr);
                Menu->AddItem(0, "Hunter"      , 14);
                Menu->AddItem(0, "Mage"        , 15);
                Menu->AddItem(0, "Paladin"     , 16);
                Menu->AddItem(0, "Priest"      , 17);
                Menu->AddItem(0, "Rogue"       , 18);
                Menu->AddItem(0, "Warlock"     , 19);
                Menu->AddItem(0, "Warrior"     , 20);
                Menu->AddItem(0, "Shaman"      , 21);
                Menu->SendTo(Plr);
            }break;

        case 13:    // A profession trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2793, Plr);
                Menu->AddItem(0, "Alchemy"           , 22);
                Menu->AddItem(0, "Blacksmithing"     , 23);
                Menu->AddItem(0, "Cooking"           , 24);
                Menu->AddItem(0, "Enchanting"        , 25);
                Menu->AddItem(0, "Engineering"       , 26);
                Menu->AddItem(0, "First Aid"         , 27);
                Menu->AddItem(0, "Fishing"           , 28);
                Menu->AddItem(0, "Herbalism"         , 29);
                Menu->AddItem(0, "Inscription"       , 30);
                Menu->AddItem(0, "Leatherworking"    , 31);
                Menu->AddItem(0, "Mining"            , 32);
                Menu->AddItem(0, "Skinning"          , 33);
                Menu->AddItem(0, "Tailoring"         , 34);
                Menu->SendTo(Plr);
            }break;

            ////////////////
            // Class trainer submenu
            ////////
        case 14: //Hunter
            Plr->Gossip_SendPOI(-5023, -1253.68f, 7, 6, 0, "Hall of Arms");
            SendQuickMenu(2770);
            break;

        case 15: //Mage
            Plr->Gossip_SendPOI(-4627, -926.45f, 7, 6, 0, "Hall of Mysteries");
            SendQuickMenu(2771);
            break;

        case 16: //Paladin
            Plr->Gossip_SendPOI(-4627.02f, -926.45f, 7, 6, 0, "Hall of Mysteries");
            SendQuickMenu(2773);
            break;

        case 17: //Priest
            Plr->Gossip_SendPOI(-4627, -926.45f, 7, 6, 0, "Hall of Mysteries");
            SendQuickMenu(2772);
            break;

        case 18: //Rogue
            Plr->Gossip_SendPOI(-4647.83f, -1124, 7, 6, 0, "Ironforge Rogue Trainer");
            SendQuickMenu(2774);
            break;

        case 19: //Warlock
            Plr->Gossip_SendPOI(-4605, -1110.45f, 7, 6, 0, "Ironforge Warlock Trainer");
            SendQuickMenu(2775);
            break;

        case 20: //Warrior
            Plr->Gossip_SendPOI(-5023.08f, -1253.68f, 7, 6, 0, "Hall of Arms");
            SendQuickMenu(2776);
            break;

        case 21: //Shaman
            Plr->Gossip_SendPOI(-4722.02f, -1150.66f, 7, 6, 0, "Ironforge Shaman Trainer");
            SendQuickMenu(10842);
            break;

        case 22: //Alchemy
            Plr->Gossip_SendPOI(-4858.5f, -1241.83f, 7, 6, 0, "Berryfizz's Potions and Mixed Drinks");
            SendQuickMenu(2794);
            break;

        case 23: //Blacksmithing
            Plr->Gossip_SendPOI(-4796.97f, -1110.17f, 7, 6, 0, "The Great Forge");
            SendQuickMenu(2795);
            break;

        case 24: //Cooking
            Plr->Gossip_SendPOI(-4767.83f, -1184.59f, 7, 6, 0, "The Bronze Kettle");
            SendQuickMenu(2796);
            break;

        case 25: //Enchanting
            Plr->Gossip_SendPOI(-4803.72f, -1196.53f, 7, 6, 0, "Thistlefuzz Arcanery");
            SendQuickMenu(2797);
            break;

        case 26: //Engineering
            Plr->Gossip_SendPOI(-4799.56f, -1250.23f, 7, 6, 0, "Springspindle's Gadgets");
            SendQuickMenu(2798);
            break;

        case 27: //First Aid
            Plr->Gossip_SendPOI(-4881.6f, -1153.13f, 7, 6, 0, "Ironforge Physician");
            SendQuickMenu(2799);
            break;

        case 28: //Fishing
            Plr->Gossip_SendPOI(-4597.91f, -1091.93f, 7, 6, 0, "Traveling Fisherman");
            SendQuickMenu(2800);
            break;

        case 29: //Herbalism
            Plr->Gossip_SendPOI(-4876.9f, -1151.92f, 7, 6, 0, "Ironforge Physician");
            SendQuickMenu(2801);
            break;

        case 30: //Inscription
            Plr->Gossip_SendPOI(-4801.72f, -1189.41f, 7, 6, 0, "Ironforge Inscription");
            SendQuickMenu(13884);
            break;

        case 31: //Leatherworking
            Plr->Gossip_SendPOI(-4745, -1027.57f, 7, 6, 0, "Finespindle's Leather Goods");
            SendQuickMenu(2802);
            break;

        case 32: //Mining
            Plr->Gossip_SendPOI(-4705.06f, -1116.43f, 7, 6, 0, "Deepmountain Mining Guild");
            SendQuickMenu(2804);
            break;

        case 33: //Skinning
            Plr->Gossip_SendPOI(-4745, -1027.57f, 7, 6, 0, "Finespindle's Leather Goods");
            SendQuickMenu(2805);
            break;

        case 34: //Tailoring
            Plr->Gossip_SendPOI(-4719.60f, -1056.96f, 7, 6, 0, "Stonebrow's Clothier");
            SendQuickMenu(2807);
            break;
        }
    }
};

class Royal_Historian_Archesonus : public GossipScript
{
public:
    void GossipHello(Object * pObject, Player* Plr, bool AutoSend)
    {
        if(Plr->GetQuestStatusForQuest(3702) == QMGR_QUEST_NOT_FINISHED)
        {
            GossipMenu * Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2235, Plr);
            Menu->AddItem(2, RHA_GOSSIP_ITEM_1, 1);
            Menu->SendTo(Plr);
        }
    }

    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char * EnteredCode)
    {
        GossipMenu* Menu;
        switch(IntId)
        {
        case 1:
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2236, Plr);
                Menu->AddItem(2, RHA_GOSSIP_ITEM_2, 2);
                Menu->SendTo(Plr);
            }break;
        case 2:
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2237, Plr);
                Menu->AddItem(2, RHA_GOSSIP_ITEM_3, 3);
                Menu->SendTo(Plr);
            }break;
        case 3:
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2238, Plr);
                Menu->AddItem(2, RHA_GOSSIP_ITEM_4, 4);
                Menu->SendTo(Plr);
            }break;
        case 4:
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2239, Plr);
                Menu->AddItem(2, RHA_GOSSIP_ITEM_5, 5);
                Menu->SendTo(Plr);
            }break;
        case 5:
            {
                Plr->Gossip_Complete();
                sQuestMgr.OnPlayerExploreArea(Plr, 3702);
            }break;
        }
    }
};

void Lacrimi::SetupIronforge()
{
    RegisterCtrGossipScript(5595,    IronforgeGuard);    // Ironforge Guard
    RegisterCtrGossipScript(8879,    Royal_Historian_Archesonus);
}
