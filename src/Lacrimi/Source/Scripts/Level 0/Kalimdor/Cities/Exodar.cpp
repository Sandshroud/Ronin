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

class ExodarGuard : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr, bool AutoSend)
    {
        GossipMenu* Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9551, plr);
        Menu->AddItem(0, "Auction House"        , 1);
        Menu->AddItem(0, "The Bank"             , 2);
        Menu->AddItem(0, "Hippogryph Master"    , 3);
        Menu->AddItem(0, "Guild Master"         , 4);
        Menu->AddItem(0, "The Inn"              , 5);
        Menu->AddItem(0, "Mailbox"              , 6);
        Menu->AddItem(0, "Stable Master"        , 7);
        Menu->AddItem(0, "Weapon Master"        , 8);
        Menu->AddItem(0, "Battlemasters"        , 9);
        Menu->AddItem(0, "Class Trainer"        , 10);
        Menu->AddItem(0, "Profession Trainer"   , 11);
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
            Plr->Gossip_SendPOI(-4013.82f, -11729.64f, 7, 6, 0, "Exodar, Auctioneer");
            SendEmptyMenu(9528);
            break;

        case 2:     // The Bank
            Plr->Gossip_SendPOI(-3923.89f, -11544.5f, 7, 6, 0, "Exodar, bank");
            SendEmptyMenu(9529);
            break;

        case 3:     // Hippogryph Master
            Plr->Gossip_SendPOI(-4058.45f, -11789.7f, 7, 6, 0, "Exodar, Hippogryph Master");
            SendEmptyMenu(9530);
            break;

        case 4:     // Guild Master
            Plr->Gossip_SendPOI(-4093.38f, -11630.39f, 7, 6, 0, "Exodar, Guild Master");
            SendEmptyMenu(9539);
            break;

        case 5:     // The Inn
            Plr->Gossip_SendPOI(-3765.34f, -11695.8f, 7, 6, 0, "Exodar, Inn");
            SendEmptyMenu(9545);
            break;

        case 6:     // Mailbox
            Plr->Gossip_SendPOI(-3913.75f, -11606.83f, 7, 6, 0, "Exodar, Mailbox");
            SendEmptyMenu(10254);
            break;

        case 7:     // Stable Master
            Plr->Gossip_SendPOI(-3787.01f, -11702.7f, 7, 6, 0, "Exodar, Stable Master");
            SendEmptyMenu(9558);
            break;

        case 8:     // Weapon Master
            Plr->Gossip_SendPOI(-4215.68f, -11628.9f, 7, 6, 0, "Exodar, Weapon Master");
            SendEmptyMenu(9565);
            break;

        case 9:     // Battlemasters
            Plr->Gossip_SendPOI(-3999.82f, -11368.33f, 7, 6, 0, "Exodar, Battlemasters");
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9531, Plr);
            Menu->AddItem(0, "Arena Battlemaster"  , 12);
            Menu->SendTo(Plr);
            break;

        case 10:    // Class Trainers
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9533, Plr);
                Menu->AddItem(0, "Druid"       , 13);
                Menu->AddItem(0, "Hunter"      , 14);
                Menu->AddItem(0, "Mage"        , 15);
                Menu->AddItem(0, "Paladin"     , 16);
                Menu->AddItem(0, "Priest"      , 17);
                Menu->AddItem(0, "Shaman"      , 18);
                Menu->AddItem(0, "Warrior"     , 19);
                Menu->SendTo(Plr);
            }break;

        case 11:    // Profession Trainers
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9555, Plr);
                Menu->AddItem(0, "Alchemy"           , 20);
                Menu->AddItem(0, "Blacksmithing"     , 21);
                Menu->AddItem(0, "Enchanting"        , 22);
                Menu->AddItem(0, "Engineering"       , 23);
                Menu->AddItem(0, "First Aid"         , 24);
                Menu->AddItem(0, "Fishing"           , 25);
                Menu->AddItem(0, "Herbalism"         , 26);
                Menu->AddItem(0, "Inscription"       , 27);
                Menu->AddItem(0, "Jewelcrafting"     , 28);
                Menu->AddItem(0, "Leatherworking"    , 29);
                Menu->AddItem(0, "Mining"            , 30);
                Menu->AddItem(0, "Skinning"          , 31);
                Menu->AddItem(0, "Tailoring"         , 32);
                Menu->AddItem(0, "Cooking"           , 33);
                Menu->SendTo(Plr);
            }break;

            //////////////////
            // Battlemaster submenu
            ////////
        case 12://Arena Battlemaster Exodar
            Plr->Gossip_SendPOI(-3725.25f, -11688.3f, 7, 6, 0, "Arena Battlemaster Exodar");
            SendQuickMenu(10223);
            break;

        case 13: //Druid
            Plr->Gossip_SendPOI(-4274.81f, -11495.3f, 7, 6, 0, "Exodar, Druid Trainer");
            SendQuickMenu(9534);
            break;

        case 14: //Hunter
            Plr->Gossip_SendPOI(-4229.36f, -11563.36f, 7, 6, 0, "Exodar, Hunter Trainers");
            SendQuickMenu(9544);
            break;

        case 15: //Mage
            Plr->Gossip_SendPOI(-4048.8f, -11559.02f, 7, 6, 0, "Exodar, Mage Trainers");
            SendQuickMenu(9550);
            break;

        case 16: //Paladin
            Plr->Gossip_SendPOI(-4176.57f, -11476.46f, 7, 6, 0, "Exodar, Paladin Trainers");
            SendQuickMenu(9553);
            break;

        case 17: //Priest
            Plr->Gossip_SendPOI(-3972.38f, -11483.2f, 7, 6, 0, "Exodar, Priest Trainers");
            SendQuickMenu(9554);
            break;

        case 18: //Shaman
            Plr->Gossip_SendPOI(-3843.8f, -11390.75f, 7, 6, 0, "Exodar, Shaman Trainer");
            SendQuickMenu(9556);
            break;

        case 19: //Warrior
            Plr->Gossip_SendPOI(-4191.11f, -11650.45f, 7, 6, 0, "Exodar, Warrior Trainers");
            SendQuickMenu(9562);
            break;

        case 20: //Alchemy
            Plr->Gossip_SendPOI(-4042.37f, -11366.3f, 7, 6, 0, "Exodar, Alchemist Trainers");
            SendQuickMenu(9527);
            break;

        case 21: //Blacksmithing
            Plr->Gossip_SendPOI(-4232.4f, -11705.23f, 7, 6, 0, "Exodar, Blacksmithing Trainers");
            SendQuickMenu(9532);
            break;

        case 22: //Enchanting
            Plr->Gossip_SendPOI(-3889.3f, -11495, 7, 6, 0, "Exodar, Enchanters");
            SendQuickMenu(9535);
            break;

        case 23: //Engineering
            Plr->Gossip_SendPOI(-4257.93f, -11636.53f, 7, 6, 0, "Exodar, Engineering");
            SendQuickMenu(9536);
            break;

        case 24: //First Aid
            Plr->Gossip_SendPOI(-3766.05f, -11481.8f, 7, 6, 0, "Exodar, First Aid Trainer");
            SendQuickMenu(9537);
            break;

        case 25: //Fishing
            Plr->Gossip_SendPOI(-3726.64f, -11384.43f, 7, 6, 0, "Exodar, Fishing Trainer");
            SendQuickMenu(9538);
            break;

        case 26: //Herbalism
            Plr->Gossip_SendPOI(-4052.5f, -11356.6f, 7, 6, 0, "Exodar, Herbalism Trainer");
            SendQuickMenu(9543);
            break;

        case 27: //Inscription
            Plr->Gossip_SendPOI(-3889.3f, -11495, 7, 6, 0, "Exodar, Inscription");
            SendQuickMenu(13887);
            break;

        case 28: //Jewelcrafting
            Plr->Gossip_SendPOI(-3786.27f, -11541.33f, 7, 6, 0, "Exodar, Jewelcrafters");
            SendQuickMenu(9547);
            break;

        case 29: //Leatherworking
            Plr->Gossip_SendPOI(-4134.42f, -11772.93f, 7, 6, 0, "Exodar, Leatherworking");
            SendQuickMenu(9549);
            break;

        case 30: //Mining
            Plr->Gossip_SendPOI(-4220.31f, -11694.29f, 7, 6, 0, "Exodar, Mining Trainers");
            SendQuickMenu(9552);
            break;

        case 31: //Skinning
            Plr->Gossip_SendPOI(-4134.97f, -11760.5f, 7, 6, 0, "Exodar, Skinning Trainer");
            SendQuickMenu(9557);
            break;

        case 32: //Tailoring
            Plr->Gossip_SendPOI(-4095.78f, -11746.9f, 7, 6, 0, "Exodar, Tailors");
            SendQuickMenu(9350);
            break;

        case 33: //Cooking
            Plr->Gossip_SendPOI(-3799.69f, -11650.51f, 7, 6, 0, "Exodar, Cook");
            SendQuickMenu(9559);
            break;
        }
    }
};

void Lacrimi::SetupExodar()
{
    RegisterCtrGossipScript(16733,    ExodarGuard);        // Exodar Peacekeeper
    RegisterCtrGossipScript(20674,    ExodarGuard);        // Shield of Velen
}
