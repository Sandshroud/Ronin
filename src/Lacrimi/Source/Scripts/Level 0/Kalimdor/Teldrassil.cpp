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

class TeldrassilGuard : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr, bool AutoSend)
    {
        GossipMenu* Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4316, plr);

        Menu->AddItem(0, "The Bank"           , 1);
        Menu->AddItem(0, "Rut'theran Ferry"   , 2);
        Menu->AddItem(0, "The Guild Master"   , 3);
        Menu->AddItem(0, "The Inn"            , 4);
        Menu->AddItem(0, "Stable Master"      , 5);
        Menu->AddItem(0, "Class Trainer"      , 6);
        Menu->AddItem(0, "Profession Trainer" , 7);
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

        case 1:     // The Bank
            SendEmptyMenu(4317);
            break;

        case 2:     // Rut'theran Ferry
            SendEmptyMenu(4318);
            break;

        case 3:     // The Guild Master
            SendEmptyMenu(4319);
            break;

        case 4:     // The Inn
            Plr->Gossip_SendPOI(9802.2f, 982.6f, 7, 6, 0, "Dolanaar Inn");
            SendEmptyMenu(4320);
            break;

        case 5:     // Stable Master
            Plr->Gossip_SendPOI(9807.78f, 931.3f, 7, 6, 0, "Seriadne");
            SendEmptyMenu(5982);
            break;

        case 6:    // Class Trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4322, Plr);
                Menu->AddItem(0, "Druid"       , 8);
                Menu->AddItem(0, "Hunter"      , 9);
                Menu->AddItem(0, "Priest"      , 10);
                Menu->AddItem(0, "Rogue"       , 11);
                Menu->AddItem(0, "Warrior"     , 12);
                Menu->SendTo(Plr);
            }break;

        case 7:    // Profession Trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4273, Plr);
                Menu->AddItem(0, "Alchemy"           , 13);
                Menu->AddItem(0, "Cooking"           , 14);
                Menu->AddItem(0, "Enchanting"        , 15);
                Menu->AddItem(0, "First Aid"         , 16);
                Menu->AddItem(0, "Fishing"           , 17);
                Menu->AddItem(0, "Herbalism"         , 18);
                Menu->AddItem(0, "Inscription"       , 19);
                Menu->AddItem(0, "Leatherworking"    , 20);
                Menu->AddItem(0, "Skinning"          , 21);
                Menu->AddItem(0, "Tailoring"         , 22);
                Menu->SendTo(Plr);
            }break;

        case 8:    // Druid
            Plr->Gossip_SendPOI(9741.91f, 966.0f, 7, 6, 0, "Kal");
            SendQuickMenu(4323);
            break;

        case 9:    // Hunter
            Plr->Gossip_SendPOI(9812.33f, 928.8f, 7, 6, 0, "Dazalar");
            SendQuickMenu(4324);
            break;

        case 10:    // Priest
            Plr->Gossip_SendPOI(9905.44f, 985.43f, 7, 6, 0, "Laurana Morninglight");
            SendQuickMenu(4325);
            break;

        case 11:    // Rogue
            Plr->Gossip_SendPOI(9790.3f, 943.86f, 7, 6, 0, "Jannok Breezesong");
            SendQuickMenu(4326);
            break;

        case 12:    // Warrior
            Plr->Gossip_SendPOI(9822.23f, 952.0f, 7, 6, 0, "Kyra Windblade");
            SendQuickMenu(4327);
            break;

        case 13: //Alchemy
            Plr->Gossip_SendPOI(9767.75f, 879.8f, 7, 6, 0, "Cyndra Kindwhisper");
            SendQuickMenu(4329);
            break;

        case 14: //Cooking
            Plr->Gossip_SendPOI(9751.01f, 9061.19f, 7, 6, 0, "Zarrin");
            SendQuickMenu(4330);
            break;

        case 15: //Enchanting
            Plr->Gossip_SendPOI(10671.9f, 1945.48f, 7, 6, 0, "Alanna Raveneye");
            SendQuickMenu(4331);
            break;

        case 16: //First Aid
            Plr->Gossip_SendPOI(9902.94f, 999.57f, 7, 6, 0, "Byancie");
            SendQuickMenu(4332);
            break;

        case 17: //Fishing
            SendQuickMenu(4333);
            break;

        case 18: //Herbalism
            Plr->Gossip_SendPOI(9773.23f, 875.64f, 7, 6, 0, "Malorne Bladeleaf");
            SendQuickMenu(4334);
            break;

        case 19: //Inscription
            Plr->Gossip_SendPOI(10146.09f, 2313.42f, 7, 6, 0, "Darnassus Inscription");
            SendQuickMenu(13886);
            break;

        case 20: //Leatherworking
            Plr->Gossip_SendPOI(10153.29f, 1682.3f, 7, 6, 0, "Nadyia Maneweaver");
            SendQuickMenu(4335);
            break;

        case 21: //Skinning
            Plr->Gossip_SendPOI(10135.7f, 1671.47f, 7, 6, 0, "Radnaal Maneweaver");
            SendQuickMenu(4336);
            break;

        case 22: //Tailoring
            SendQuickMenu(4337);
            break;
        }
    }
};

void Lacrimi::SetupTeldrassil()
{
    RegisterCtrGossipScript(3571,    TeldrassilGuard);    // Teldrassil Sentinel
}
