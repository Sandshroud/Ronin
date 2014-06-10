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

class DarnassusGuard : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr, bool AutoSend)
    {
        GossipMenu* Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3016, plr);

        Menu->AddItem(0, "Auction House"      , 1);
        Menu->AddItem(0, "The Bank"           , 2);
        Menu->AddItem(0, "Hippogryph Master"  , 3);
        Menu->AddItem(0, "Guild Master"       , 4);
        Menu->AddItem(0, "The Inn"            , 5);
        Menu->AddItem(0, "Mailbox"            , 6);
        Menu->AddItem(0, "Stable Master"      , 7);
        Menu->AddItem(0, "Weapons Trainer"    , 8);
        Menu->AddItem(0, "Battlemaster"       , 9);
        Menu->AddItem(0, "Class Trainer"      , 10);
        Menu->AddItem(0, "Profession Trainer" , 11);
        Menu->AddItem(0, "Lexicon of Power"   , 27);
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
            // Main menu handlers (Most/All 'borrowed' from scriptdev)
            /////
        case 1:     // Auction House
            Plr->Gossip_SendPOI(9861.23f, 2334.55f, 7, 6, 0, "Darnassus Auction House");
            SendEmptyMenu(3833);
            break;

        case 2:     // The Bank
            Plr->Gossip_SendPOI(9938.45f, 2512.35f, 7, 6, 0, "Darnassus Bank");
            SendEmptyMenu(3017);
            break;

        case 3:     // Hippogryph Master
            Plr->Gossip_SendPOI(9945.65f, 2618.94f, 7, 6, 0, "Rut'theran Village");
            SendEmptyMenu(3018);
            break;

        case 4:     // Guild Master
            Plr->Gossip_SendPOI(10076.40f, 2199.59f, 7, 6, 0, "Darnassus Guild Master");
            SendEmptyMenu(3019);
            break;

        case 5:     // The Inn
            Plr->Gossip_SendPOI(10133.29f, 2222.52f, 7, 6, 0, "Darnassus Inn");
            SendEmptyMenu(3020);
            break;

        case 6:     // Mailbox
            Plr->Gossip_SendPOI(9942.17f, 2495.48f, 7, 6, 0, "Darnassus Mailbox");
            SendEmptyMenu(3021);
            break;

        case 7:     // Stable Master
            Plr->Gossip_SendPOI(10167.20f, 2522.66f, 7, 6, 0, "Alassin");
            SendEmptyMenu(5980);
            break;

        case 8:     // Weapons Trainer
            Plr->Gossip_SendPOI(9907.11f, 2329.70f, 7, 6, 0, "Ilyenia Moonfire");
            SendEmptyMenu(4517);
            break;

        case 9:    // Battlemaster
            SendQuickMenu(7519);
            Plr->Gossip_SendPOI(9981.9f, 2325.9f, 7, 6, 0, "Battlemasters Darnassus");
            break;

        case 10:    // Class Trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4264, Plr);
                Menu->AddItem(0, "Druid"       , 12);
                Menu->AddItem(0, "Hunter"      , 13);
                Menu->AddItem(0, "Priest"      , 14);
                Menu->AddItem(0, "Rogue"       , 15);
                Menu->AddItem(0, "Warrior"     , 16);
                Menu->SendTo(Plr);
            }break;

        case 11:    // Profession Trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4273, Plr);
                Menu->AddItem(0, "Alchemy"           , 17);
                Menu->AddItem(0, "Cooking"           , 18);
                Menu->AddItem(0, "Enchanting"        , 19);
                Menu->AddItem(0, "First Aid"         , 20);
                Menu->AddItem(0, "Fishing"           , 21);
                Menu->AddItem(0, "Herbalism"         , 22);
                Menu->AddItem(0, "Inscription"       , 23);
                Menu->AddItem(0, "Leatherworking"    , 24);
                Menu->AddItem(0, "Skinning"          , 25);
                Menu->AddItem(0, "Tailoring"         , 26);
                Menu->SendTo(Plr);
            }break;

        case 12:    // Druid
            Plr->Gossip_SendPOI(10186, 2570.46f, 7, 6, 0, "Darnassus Druid Trainer");
            SendQuickMenu(3024);
            break;

        case 13:    // Hunter
            Plr->Gossip_SendPOI(10177.29f, 2511.10f, 7, 6, 0, "Darnassus Hunter Trainer");
            SendQuickMenu(3023);
            break;

        case 14:    // Priest
            Plr->Gossip_SendPOI(9659.12f, 2524.88f, 7, 6, 0, "Temple of the Moon");
            SendQuickMenu(3025);
            break;

        case 15:    // Rogue
            Plr->Gossip_SendPOI(10122, 2599.12f, 7, 6, 0, "Darnassus Rogue Trainer");
            SendQuickMenu(3026);
            break;

        case 16:    // Warrior
            Plr->Gossip_SendPOI(9951.91f, 2280.38f, 7, 6, 0, "Warrior's Terrace");
            SendQuickMenu(3033);
            break;

        case 17: //Alchemy
            Plr->Gossip_SendPOI(10075.90f, 2356.76f, 7, 6, 0, "Darnassus Alchemy Trainer");
            SendQuickMenu(3035);
            break;

        case 18: //Cooking
            Plr->Gossip_SendPOI(10088.59f, 2419.21f, 7, 6, 0, "Darnassus Cooking Trainer");
            SendQuickMenu(3036);
            break;

        case 19: //Enchanting
            Plr->Gossip_SendPOI(10146.09f, 2313.42f, 7, 6, 0, "Darnassus Enchanting Trainer");
            SendQuickMenu(3337);
            break;

        case 20: //First Aid
            Plr->Gossip_SendPOI(10150.09f, 2390.43f, 7, 6, 0, "Darnassus First Aid Trainer");
            SendQuickMenu(3037);
            break;

        case 21: //Fishing
            Plr->Gossip_SendPOI(9836.20f, 2432.17f, 7, 6, 0, "Darnassus Fishing Trainer");
            SendQuickMenu(3038);
            break;

        case 22: //Herbalism
            Plr->Gossip_SendPOI(9757.17f, 2430.16f, 7, 6, 0, "Darnassus Herbalism Trainer");
            SendQuickMenu(3039);
            break;

        case 23: //Inscription
            Plr->Gossip_SendPOI(10146.09f, 2313.42f, 7, 6, 0, "Darnassus Inscription Trainer");
            SendQuickMenu(13886);
            break;

        case 24: //Leatherworking
            Plr->Gossip_SendPOI(10086.59f, 2255.77f, 7, 6, 0, "Darnassus Leatherworking Trainer");
            SendQuickMenu(3040);
            break;

        case 25: //Skinning
            Plr->Gossip_SendPOI(10081.40f, 2257.18f, 7, 6, 0, "Darnassus Skinning Trainer");
            SendQuickMenu(3042);
            break;

        case 26: //Tailoring
            Plr->Gossip_SendPOI(10079.70f, 2268.19f, 7, 6, 0, "Darnassus Tailor");
            SendQuickMenu(3044);
            break;

        case 27: //Lexicon of Power
            Plr->Gossip_SendPOI(10146.09f, 2313.42f, 7, 6, 0, "Darnassus Inscription Trainer");
            SendQuickMenu(14174);
            break;
        }
    }
};

void Lacrimi::SetupDarnassus()
{
    RegisterCtrGossipScript(4262,    DarnassusGuard);    // Darnassus Sentinel
}
