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

class ThunderbluffGuard : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr, bool AutoSend)
    {
        GossipMenu* Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3543, plr);
        Menu->AddItem(0, "The bank",                1);
        Menu->AddItem(0, "The wind rider master",   2);
        Menu->AddItem(0, "The guild master",        3);
        Menu->AddItem(0, "The inn",                 4);
        Menu->AddItem(0, "The mailbox",             5);
        Menu->AddItem(0, "The auction house",       6);
        Menu->AddItem(0, "The weapon master",       7);
        Menu->AddItem(0, "The stable master",       8);
        Menu->AddItem(0, "The battlemaster",        9);
        Menu->AddItem(0, "A class trainer",         10);
        Menu->AddItem(0, "A profession trainer",    11);
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
        case 1:     // The bank
            Plr->Gossip_SendPOI(-1257.8f, 24.14f, 7, 6, 0, "Thunder Bluff Bank");
            SendEmptyMenu(1292);
            break;

        case 2:     // The wind rider master
            Plr->Gossip_SendPOI(-1196.43f, 28.26f, 7, 6, 0, "Wind Rider Roost");
            SendEmptyMenu(1293);
            break;

        case 3:     // The guild master
            Plr->Gossip_SendPOI(-1296.5f, 127.57f, 7, 6, 0, "Thunder Bluff Civic Information");
            SendEmptyMenu(1291);
            break;

        case 4:     // The inn
            Plr->Gossip_SendPOI(-1296, 39.7f, 7, 6, 0, "Thunder Bluff Inn");
            SendEmptyMenu(3153);
            break;

        case 5:     // The mailbox
            Plr->Gossip_SendPOI(-1263.59f, 44.36f, 7, 6, 0, "Thunder Bluff Mailbox");
            SendEmptyMenu(3154);
            break;

        case 6:     // The auction house
            Plr->Gossip_SendPOI(-1205.51f, 105.74f, 7, 6, 0, "Thunder Bluff Auction house");
            SendEmptyMenu(3155);
            break;

        case 7:     // The weapon master
            Plr->Gossip_SendPOI(-1282.31f, 89.56f, 7, 6, 0, "Ansekhwa");
            SendEmptyMenu(4520);
            break;

        case 8:     // The stable master
            Plr->Gossip_SendPOI(-1270.19f, 48.84f, 7, 6, 0, "Bulrug");
            SendEmptyMenu(5977);
            break;

        case 9:    // The battlemaster
            Plr->Gossip_SendPOI(-1391.22f, -81.33f, 7, 6, 0, "Battlemasters Thunder Bluff");
            SendEmptyMenu(7527);
            break;

        case 10:    // A class trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3542, Plr);
                Menu->AddItem(0, "Druid"       , 12);
                Menu->AddItem(0, "Hunter"      , 13);
                Menu->AddItem(0, "Mage"        , 14);
                Menu->AddItem(0, "Priest"      , 15);
                Menu->AddItem(0, "Shaman"      , 16);
                Menu->AddItem(0, "Warrior"     , 17);
                Menu->SendTo(Plr);
            }break;

        case 11:    // A profession trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3541, Plr);
                Menu->AddItem(0, "Alchemy"           , 18);
                Menu->AddItem(0, "Blacksmithing"     , 19);
                Menu->AddItem(0, "Cooking"           , 20);
                Menu->AddItem(0, "Enchanting"        , 21);
                Menu->AddItem(0, "First Aid"         , 22);
                Menu->AddItem(0, "Fishing"           , 23);
                Menu->AddItem(0, "Herbalism"         , 24);
                Menu->AddItem(0, "Leatherworking"    , 25);
                Menu->AddItem(0, "Mining"            , 26);
                Menu->AddItem(0, "Skinning"          , 27);
                Menu->AddItem(0, "Tailoring"         , 28);
                Menu->SendTo(Plr);
            }break;

            ////////////////
            // Class trainer submenu
            ////////

        case 12: //Druid
            Plr->Gossip_SendPOI(-1054.47f, -285, 7, 6, 0, "Hall of Elders");
            SendQuickMenu(1294);
            break;

        case 13: //Hunter
            Plr->Gossip_SendPOI(-1416.32f, -114.28f, 7, 6, 0, "Hunter's Hall");
            SendQuickMenu(1295);
            break;

        case 14: //Mage
            Plr->Gossip_SendPOI(-1061.2f, 195.5f, 7, 6, 0, "Pools of Vision");
            SendQuickMenu(1296);
            break;

        case 15: //Priest
            Plr->Gossip_SendPOI(-1061.2f, 195.5f, 7, 6, 0, "Pools of Vision");
            SendQuickMenu(1297);
            break;

        case 16: //Shaman
            Plr->Gossip_SendPOI(-989.54f, 278.25f, 7, 6, 0, "Hall of Spirits");
            SendQuickMenu(1298);
            break;

        case 17: //Warrior
            Plr->Gossip_SendPOI(-1416.32f, -114.28f, 7, 6, 0, "Hunter's Hall");
            SendQuickMenu(1299);
            break;

        case 18: //Alchemy
            Plr->Gossip_SendPOI(-1085.56f, 27.29f, 7, 6, 0, "Bena's Alchemy");
            SendQuickMenu(1332);
            break;

        case 19: //Blacksmithing
            Plr->Gossip_SendPOI(-1239.75f, 104.88f, 7, 6, 0, "Karn's Smithy");
            SendQuickMenu(1333);
            break;

        case 20: //Cooking
            Plr->Gossip_SendPOI(-1214.5f, -21.23f, 7, 6, 0, "Aska's Kitchen");
            SendQuickMenu(1334);
            break;

        case 21: //Enchanting
            Plr->Gossip_SendPOI(-1112.65f, 48.26f, 7, 6, 0, "Dawnstrider Enchanters");
            SendQuickMenu(1335);
            break;

        case 22: //First Aid
            Plr->Gossip_SendPOI(-996.58f, 200.5f, 7, 6, 0, "Spiritual Healing");
            SendQuickMenu(1336);
            break;

        case 23: //Fishing
            Plr->Gossip_SendPOI(-1169.35f, -68.87f, 7, 6, 0, "Mountaintop Bait & Tackle");
            SendQuickMenu(1337);
            break;

        case 24: //Herbalism
            Plr->Gossip_SendPOI(-1137.7f, -1.51f, 7, 6, 0, "Holistic Herbalism");
            SendQuickMenu(1338);
            break;

        case 25: //Leatherworking
            Plr->Gossip_SendPOI(-1156.22f, 66.86f, 7, 6, 0, "Thunder Bluff Armorers");
            SendQuickMenu(1339);
            break;

        case 26: //Mining
            Plr->Gossip_SendPOI(-1249.17f, 155, 7, 6, 0, "Stonehoof Geology");
            SendQuickMenu(1340);
            break;

        case 27: //Skinning
            Plr->Gossip_SendPOI(-1148.56f, 51.18f, 7, 6, 0, "Mooranta");
            SendQuickMenu(1343);
            break;

        case 28: //Tailoring
            Plr->Gossip_SendPOI(-1156.22f, 66.86f, 7, 6, 0, "Thunder Bluff Armorers");
            SendQuickMenu(1341);
            break;
        }
    }
};

void Lacrimi::SetupThunderbluff()
{
    RegisterCtrGossipScript(3084,    ThunderbluffGuard);    // Bluffwatcher
}
