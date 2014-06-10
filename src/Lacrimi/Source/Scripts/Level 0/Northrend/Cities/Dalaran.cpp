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

/*****************************************/
/* Dalaran Guards - Created by Saerothan */
/*****************************************/

class DalaranGuard : public GossipScript
{
public:
    void GossipHello(Object *pObject, Player *Plr, bool AutoSend)
    {
        GossipMenu *Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50000, Plr);
        if(Menu == NULL)
            return;

        Menu->AddItem(0, "Arena",                 1);
        Menu->AddItem(0, "Auction House",         2);
        Menu->AddItem(0, "Bank",                  3);
        Menu->AddItem(0, "Barber",                4);
        Menu->AddItem(0, "Battlemasters",         5);
        Menu->AddItem(0, "Capital Portals",       6);
        Menu->AddItem(0, "Flight Master",         7);
        Menu->AddItem(0, "Guild Master",          8);
        Menu->AddItem(0, "Inn",                   9);
        Menu->AddItem(0, "Locksmith ",           77);
        Menu->AddItem(0, "Mailbox",              10);
        Menu->AddItem(0, "Points of Interest",   11);
        Menu->AddItem(0, "Stable Master",        12);
        Menu->AddItem(0, "Trainers",             13);
        Menu->AddItem(0, "Vendors",              14);

        if(AutoSend)
            Menu->SendTo(Plr);
    }

    void GossipSelectOption(Object *pObject, Player *Plr, uint32 Id, uint32 IntId, const char * Code)
    {
        GossipMenu * Menu;
        switch(IntId)
        {
        case 1: // Arena
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50001, Plr);
                Menu->AddItem( 0, "Eastern Sewer Entrance", 15);
                Menu->AddItem( 0, "Western Sewer Entrance", 16);
                Menu->AddItem( 0, "Well Entrance", 17);
                Menu->SendTo(Plr);
            }break;
        case 2: // Auction House
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50002, Plr);
                Menu->AddItem( 0, "The Alliance Quarter", 18);
                Menu->AddItem( 0, "The Horde Quarter", 19);
                Menu->SendTo(Plr);
            }break;
        case 3: // Bank
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50003, Plr);
                Menu->AddItem( 0, "Northern Bank", 20);
                Menu->AddItem( 0, "Southern Bank", 21);
                Menu->AddItem( 0, "Sewers", 22); // Sewers 1
                Menu->SendTo(Plr);
            }break;
        case 4: // Barber
            {
                Plr->Gossip_SendPOI(5889.67f, 621.21f, 7, 6, 0, "Barber");
                SendQuickMenu(60000);
            }break;
        case 5: // Battlemasters
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50004, Plr);
                Menu->AddItem( 0, "The Alliance Quarter", 18);
                Menu->AddItem( 0, "The Horde Quarter", 19);
                Menu->SendTo(Plr);
            }break;
        case 6: // Capital Portals
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50005, Plr);
                Menu->AddItem( 0, "The Alliance Quarter", 18);
                Menu->AddItem( 0, "The Horde Quarter", 19);
                Menu->SendTo(Plr);
            }break;
        case 7: // Flight Master
            {
                Plr->Gossip_SendPOI(5815.91f, 453.64f, 7, 6, 0, "Flight Master");
                SendQuickMenu(60001);
            }break;

        case 8: // Guild Master
            {
                Plr->Gossip_SendPOI(5762.21f, 622.84f, 7, 6, 0, "Guild Master");
                SendQuickMenu(60002);
            }break;

        case 9: // Inn
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50006, Plr);
                Menu->AddItem( 0, "Alliance Inn", 24);
                Menu->AddItem( 0, "Horde Inn", 25);
                Menu->AddItem( 0, "Sewers", 26); // Sewers 2
                Menu->SendTo(Plr);
            }break;
        case 10: // Mailbox
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50007, Plr);
                Menu->AddItem( 0, "Inn", 9);
                Menu->AddItem( 0, "Bank", 3);
                Menu->AddItem( 0, "Krasus' Landing", 74);
                Menu->SendTo(Plr);
            }break;

        case 11: // Points of Interest
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50008, Plr);
                Menu->AddItem( 0, "The Alliance Quarter", 18);
                Menu->AddItem( 0, "The Horde Quarter", 19);
                Menu->AddItem( 0, "The Violet Citadel", 27);
                Menu->AddItem( 0, "The Violet Hold", 28);
                Menu->AddItem( 0, "Sewers", 22); // Sewers 1
                Menu->AddItem( 0, "Trade District", 29);
                Menu->AddItem( 0, "Krasus'Landing", 75);
                Menu->AddItem( 0, "Antonidas Memorial", 30);
                Menu->AddItem( 0, "Runeweaver Square", 31);
                Menu->AddItem( 0, "The Eventide", 32);
                Menu->AddItem( 0, "Cemetary", 33);
                Menu->AddItem( 0, "Lexicon of Power", 34);
                Menu->SendTo(Plr);
            }break;

        case 12: // Stable Master
            {
                Plr->Gossip_SendPOI(5859.68f, 557.57f, 7, 6, 0, "Stable Master");
                SendQuickMenu(60003);
            }break;

        case 13: // Trainers
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50009, Plr);
                Menu->AddItem( 0, "Class Trainer", 35);
                Menu->AddItem( 0, "Cold Weather Flying Trainer", 76);
                Menu->AddItem( 0, "Portal Trainer", 36);
                Menu->AddItem( 0, "Profession Trainer", 37);
                Menu->SendTo(Plr);
            }break;

        case 14: // Vendors
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50010, Plr);
                Menu->AddItem( 0, "Armor", 38);
                Menu->AddItem( 0, "Clothing", 39);
                Menu->AddItem( 0, "Emblem Gear", 40);
                Menu->AddItem( 0, "Flowers", 41);
                Menu->AddItem( 0, "Fruit", 42);
                Menu->AddItem( 0, "General Goods", 43);
                Menu->AddItem( 0, "Jewelry", 44);
                Menu->AddItem( 0, "Pet Supplies & Exotic Mounts", 45);
                Menu->AddItem( 0, "Pie, Pastry & Cakes", 46);
                Menu->AddItem( 0, "Reagents & Magical Goods", 47);
                Menu->AddItem( 0, "Toys", 48);
                Menu->AddItem( 0, "Trade Supplies", 43);
                Menu->AddItem( 0, "Trinkets. Relics & Off-hand items", 49);
                Menu->AddItem( 0, "Weapons", 50);
                Menu->AddItem( 0, "Wine & Cheese", 51);
                Menu->SendTo(Plr);
            }break;

        case 15: // Eastern Sewer Entrance
            {
                Plr->Gossip_SendPOI(5801.77f, 551.07f, 7, 6, 0, "Eastern Sewer Entrance");
                SendQuickMenu(60004);
            }break;

        case 16: // Western Sewer Entrance
            {
                Plr->Gossip_SendPOI(5815.21f, 762.65f, 7, 6, 0, "Western Sewer Entrance");
                SendQuickMenu(60005);
            }break;

        case 17: // Well Entrance
            {
                Plr->Gossip_SendPOI(5791.60f, 561.82f, 7, 6, 0, "Well Entrance");
                SendQuickMenu(60006);
            }break;

        case 18: // The Alliance Quarter
            {
                Plr->Gossip_SendPOI(5763.20f, 711.77f, 7, 6, 0, "The Alliance Quarter");
                SendQuickMenu(60007);
            }break;

        case 19: // The Horde Quarter
            {
                Plr->Gossip_SendPOI(5860.35f, 592.97f, 7, 6, 0, "The Horde Quarter");
                SendQuickMenu(60008);
            }break;

        case 20: // Northern Bank
            {
                Plr->Gossip_SendPOI(5979.58f, 608.16f, 7, 6, 0, "Northern Bank");
                SendQuickMenu(60009);
            }break;

        case 21: // Southern Bank
            {
                Plr->Gossip_SendPOI(5631.20f, 694.15f, 7, 6, 0, "Southern Bank");
                SendQuickMenu(60010);
            }break;

        case 22: // Sewers 1
            {
                Plr->Gossip_SendPOI(5766.31f, 731.66f, 7, 6, 0, "Sewers");
                SendQuickMenu(60011);
            }break;
        case 23: // Legerdemain Lounge
            {
                Plr->Gossip_SendPOI(5845.40f, 647.37f, 7, 6, 0, "Legerdemain Lounge");
                SendQuickMenu(60012);
            }break; 

        case 24: // Alliance Inn
            {
                Plr->Gossip_SendPOI(5718.01f, 689.28f, 7, 6, 0, "Alliance Inn");
                SendQuickMenu(60013);
            }break; 

        case 25: // Horde Inn
            {
                Plr->Gossip_SendPOI(5890.22f, 500.58f, 7, 6, 0, "Horde Inn");
                SendQuickMenu(60014);
            }break; 

        case 26: // Sewers 2
            {
                Plr->Gossip_SendPOI(5761.06f, 714.45f, 7, 6, 0, "Sewers");
                SendQuickMenu(60015);
            }break; 
        case 27: // The Violet Citadel
            {
                Plr->Gossip_SendPOI(5795.11f, 769.15f, 7, 6, 0, "The Violet Citadel");
                SendQuickMenu(60016);
            }break; 

        case 28: // The Violet Hold
            {
                Plr->Gossip_SendPOI(5720.40f, 537.46f, 7, 6, 0, "The Violet Hold");
                SendQuickMenu(60017);
            }break; 

        case 29: // Trade District
            {
                Plr->Gossip_SendPOI(5900.88f, 726.65f, 7, 6, 0, "Trade District");
                SendQuickMenu(60018);
            }break; 

        case 30: // Antonidas Memorial
            {
                Plr->Gossip_SendPOI(5951.74f, 683.53f, 7, 6, 0, "Antonidas Memorial");
                SendQuickMenu(60019);
            }break; 

        case 31: // Runeweaver Square
            {
                Plr->Gossip_SendPOI(5810.96f, 632.72f, 7, 6, 0, "Runeweaver Square");
                SendQuickMenu(60020);
            }break; 

        case 32: // The Eventide
            {
                Plr->Gossip_SendPOI(5694.66f, 650.82f, 7, 6, 0, "The Eventide");
                SendQuickMenu(60021);
            }break; 

        case 33: // Cemetary
            {
                Plr->Gossip_SendPOI(5853.35f, 769.06f, 7, 6, 0, "Cemetary");
                SendQuickMenu(60022);
            }break; 

        case 34: // Lexicon of Power
            {
                Plr->Gossip_SendPOI(5860.99f, 707.23f, 7, 6, 0, "Lexicon of Power");
                SendQuickMenu(60023);
            }break; 

        case 35: // Class Trainers
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50011, Plr);
                Menu->AddItem( 0, "The Alliance Quarter", 18);
                Menu->AddItem( 0, "The Horde Quarter", 19);
                Menu->SendTo(Plr);
            }break;

        case 36: // Portal Trainer
            {
                Plr->Gossip_SendPOI(5810.07f, 581.00f, 7, 6, 0, "Portal Trainer");
                SendQuickMenu(60024);
            }break;

        case 37: // Profession Trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50012, Plr);
                Menu->AddItem( 0, "Alchemy", 52);
                Menu->AddItem( 0, "Blacksmithing", 53);
                Menu->AddItem( 0, "Cooking", 54);
                Menu->AddItem( 0, "Enchanting", 55);
                Menu->AddItem( 0, "engineering", 56);
                Menu->AddItem( 0, "First Aid", 57);
                Menu->AddItem( 0, "Fishing", 58);
                Menu->AddItem( 0, "Herbalism", 59);
                Menu->AddItem( 0, "Inscription", 60);
                Menu->AddItem( 0, "Jewelcrafting", 61);
                Menu->AddItem( 0, "Leatherworking", 62);
                Menu->AddItem( 0, "Mining", 63);
                Menu->AddItem( 0, "Skinning", 64);
                Menu->AddItem( 0, "Tailoring", 65);
                Menu->SendTo(Plr);
            }break;

        case 38: // Armor
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50013, Plr);
                Menu->AddItem( 0, "Cloth Armor", 66);
                Menu->AddItem( 0, "Leather Armor", 67);
                Menu->AddItem( 0, "Mail Armor", 68);
                Menu->AddItem( 0, "Plate Armor", 69);
                Menu->AddItem( 0, "Shields", 70);
                Menu->SendTo(Plr);
            }break;

        case 39: // Clothing
            {
                Plr->Gossip_SendPOI(5793.11f, 686.44f, 7, 6, 0, "Clothing");
                SendQuickMenu(60025);
            }break;

        case 40: // Emblem Gear
            {
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50014, Plr);
                Menu->AddItem( 0, "The Alliance Quarter", 18);
                Menu->AddItem( 0, "The Horde Quarter", 19);
                Menu->SendTo(Plr);
            }break;

        case 41: // Flowers
            {
                Plr->Gossip_SendPOI(5772.27f, 576.61f, 7, 6, 0, "Flowers");
                SendQuickMenu(60026);
            }break;

        case 42: // Fruit
            {
                Plr->Gossip_SendPOI(5754.02f, 694.18f, 7, 6, 0, "Fruit");
                SendQuickMenu(60027);
            }break;

        case 43: // General Goods
            {
                Plr->Gossip_SendPOI(5681.22f, 616.57f, 7, 6, 0, "General Goods");
                SendQuickMenu(60028);
            }break;

        case 44: // Jewelry
            {
                Plr->Gossip_SendPOI(5874.27f, 719.18f, 7, 6, 0, "Portal Trainer");
                SendQuickMenu(60029);
            }break;

        case 45: // Pet Supplies & Exotic Mounts
            {
                Plr->Gossip_SendPOI(5833.13f, 572.72f, 7, 6, 0, "Pet Supplies & Exotic Mounts");
                SendQuickMenu(60030);
            }break;

        case 46: // Pie, Pastry & Cakes
            {
                Plr->Gossip_SendPOI(5905.86f, 629.29f, 7, 6, 0, "Pie, Pastry & Cakes");
                SendQuickMenu(60031);
            }break;

        case 47: // Reagents & Magical Goods
            {
                Plr->Gossip_SendPOI(5758.70f, 737.24f, 7, 6, 0, "Reagents & Magical Goods");
                SendQuickMenu(60032);
            }break;

        case 48: // Toys
            {
                Plr->Gossip_SendPOI(5813.48f, 688.49f, 7, 6, 0, "Toys");
                SendQuickMenu(60033);
            }break;

        case 49: // Trinkets. Relics & Off-hand items
            {
                Plr->Gossip_SendPOI(5755.53f, 642.03f, 7, 6, 0, "Trinkets. Relics & Off-hand items");
                SendQuickMenu(60034);
            }break;

        case 50: // Weapons
            {
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50015, Plr);
                Menu->AddItem( 0, "Melee Weapons", 71);
                Menu->AddItem( 0, "Ranged & Thrown Weapons", 72);
                Menu->AddItem( 0, "Staves & Wands", 73);
                Menu->SendTo(Plr);
            }break;

        case 51: // Wine & Cheese
            {
                Plr->Gossip_SendPOI(5885.66f, 606.89f, 7, 6, 0, "Wine & Cheese");
                SendQuickMenu(60035);
            }break;

        case 52: // Alchemy
            {
                Plr->Gossip_SendPOI(5888.05f, 702.44f, 7, 6, 0, "Alchemy");
                SendQuickMenu(60036);
            }break;

        case 53: // Blacksmithing
            {
                Plr->Gossip_SendPOI(5904.86f, 680.12f, 7, 6, 0, "Blacksmithing");
                SendQuickMenu(60037);
            }break;

        case 54: // Cooking
            {
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50016, Plr);
                Menu->AddItem( 0, "Alliance Inn", 24);
                Menu->AddItem( 0, "Horde Inn", 25);
                Menu->SendTo(Plr);
            }break;

        case 55: // Enchanting
            {
                Plr->Gossip_SendPOI(5840.01f, 726.53f, 7, 6, 0, "Enchanting");
                SendQuickMenu(60038);
            }break;

        case 56: // Engineering
            {
                Plr->Gossip_SendPOI(5922.55f, 727.05f, 7, 6, 0, "Engineering");
                SendQuickMenu(60039);
            }break;

        case 57: // First Aid
            {
                Plr->Gossip_SendPOI(5862.76f, 743.71f, 7, 6, 0, "First Aid");
                SendQuickMenu(60040);
            }break;

        case 58: // Fishing
            {
                Plr->Gossip_SendPOI(5707.45f, 614.57f, 7, 6, 0, "Fishing");
                SendQuickMenu(60041);
            }break;

        case 59: // Herbalism
            {
                Plr->Gossip_SendPOI(5873.99f, 689.44f, 7, 6, 0, "Herbalism");
                SendQuickMenu(60042);
            }break;

        case 60: // Inscription
            {
                Plr->Gossip_SendPOI(5861.80f, 704.30f, 7, 6, 0, "Inscription");
                SendQuickMenu(60043);
            }break;

        case 61: // Jewelcrafting
            {
                Plr->Gossip_SendPOI(5874.27f, 719.18f, 7, 6, 0, "Jewelcrafting");
                SendQuickMenu(60044);
            }break;

        case 62: // Leatherworking
            {
                Plr->Gossip_SendPOI(5903.90f, 751.97f, 7, 6, 0, "Leatherworking");
                SendQuickMenu(60045);
            }break;

        case 63: // Mining
            {
                Plr->Gossip_SendPOI(5923.27f, 709.86f, 7, 6, 0, "Mining");
                SendQuickMenu(60046);
            }break;

        case 64: // Skinning
            {
                Plr->Gossip_SendPOI(5903.90f, 751.97f, 7, 6, 0, "Skinning");
                SendQuickMenu(60047);
            }break;

        case 65: // Tailoring
            {
                Plr->Gossip_SendPOI(5881.78f, 746.64f, 7, 6, 0, "Tailoring");
                SendQuickMenu(60048);
            }break;

        case 66: // Cloth Armor
            {
                Plr->Gossip_SendPOI(5793.11f, 686.44f, 7, 6, 0, "Cloth Armor");
                SendQuickMenu(60049);
            }break;

        case 67: // Leather Armor
            {
                Plr->Gossip_SendPOI(5667.76f, 627.16f, 7, 6, 0, "Leather Armor");
                SendQuickMenu(60050);
            }break;

        case 68: // Mail Armor
            {
                Plr->Gossip_SendPOI(5667.76f, 627.16f, 7, 6, 0, "Mail Armor");
                SendQuickMenu(60051);
            }break;

        case 69: // Plate Armor
            {
                Plr->Gossip_SendPOI(5914.85f, 667.22f, 7, 6, 0, "Plate Armor");
                SendQuickMenu(60052);
            }break;

        case 70: // Shields
            {
                Plr->Gossip_SendPOI(5914.85f, 667.22f, 7, 6, 0, "Shields");
                SendQuickMenu(60053);
            }break;
        case 71: // Melee Weapons
            {
                Plr->Gossip_SendPOI(5725.11f, 602.57f, 7, 6, 0, "Melee Weapons");
                SendQuickMenu(60054);
            }break;
        case 72: // Ranged & Thrown Weapons
            {
                Plr->Gossip_SendPOI(5778.50f, 556.18f, 7, 6, 0, "Ranged & Thrown Weapons");
                SendQuickMenu(60055);
            }break;
        case 73: // Staves & Wands
            {
                Plr->Gossip_SendPOI(5665.40f, 644.91f, 7, 6, 0, "Staves & Wands");
                SendQuickMenu(60056);
            }break;
        case 74: // Krasus' Landing (mailbox) 
            {
                Plr->Gossip_SendPOI(5815.91f, 453.64f, 7, 6, 0, "Krasus' Landing");
                SendQuickMenu(60057);
            }break;
        case 75: // Krasus' Landing (points of interest)
            {
                Plr->Gossip_SendPOI(5815.91f, 453.64f, 7, 6, 0, "Krasus' Landing");
                SendQuickMenu(60058);
            }break;
        case 76: // Cold weather flying trainer
            {
                Plr->Gossip_SendPOI(5815.91f, 453.64f, 7, 6, 0, "Flight Master");
                SendQuickMenu(60059);
            }break;
        case 77: // Locksmith
            {
                Plr->Gossip_SendPOI(5934.0f, 575.626f, 7, 99, 0, "Dalaran Locksmith");
                SendQuickMenu(14004);
            }break;
        }
    }
};

class UnderbellyGuard : public GossipScript
{
public:
    void GossipHello(Object *pObject, Player *Plr, bool AutoSend)
    {
        GossipMenu* Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14027, Plr);
        Menu->AddItem(0, "Arena"        , 1);
        Menu->AddItem(0, "Bank"         , 2);
        Menu->AddItem(0, "Inn"          , 3);
        Menu->AddItem(0, "Sewer Exits"  , 4);
        Menu->AddItem(0, "Vendors"      , 5);
        Menu->SendTo(Plr);
    }

    void GossipSelectOption(Object *pObject, Player *Plr, uint32 Id, uint32 IntId, const char* Code)
    {
        GossipMenu* Menu;
        switch (IntId)
        {
        case 0:
            GossipHello(pObject, Plr, true);
            break;

            //////////////////////
            // Main menu handlers
            /////
        case 1:     // Arena
            Plr->Gossip_SendPOI(5777.62f, 604.66f, 7, 6, 0, "Dalaran: Sewer Arena");
            SendEmptyMenu(14023);
            break;

        case 2:     // Bank
            Plr->Gossip_SendPOI(5766.77f, 733.89f, 7, 6, 0, "Dalaran: Sewer Bank");
            SendEmptyMenu(14022);
            break;

        case 3:     // Inn
            Plr->Gossip_SendPOI(5760.82f, 715.31f, 7, 6, 0, "Dalaran: Sewer Inn");
            SendEmptyMenu(14021);
            break;

        case 4:     // Sewer Exits
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 14026, Plr);
                Menu->AddItem( 0, "Eastern Sewer Entrance", 6);
                Menu->AddItem( 0, "Western Sewer Entrance", 7);
                Menu->AddItem( 0, "Sewer Tunnel", 8);
                Menu->SendTo(Plr);
            }break;

        case 5:     // Vendors
            Plr->Gossip_SendPOI(5931.57f, 558.50f, 7, 6, 0, "Dalaran: Sewer Vendors");
            SendEmptyMenu(14019);
            break;
                    
        case 6:     // Eastern Sewer Entrance
            Plr->Gossip_SendPOI(5795.98f, 555.54f, 7, 6, 0, "Dalaran: Eastern Sewer Entrance");
            SendEmptyMenu(14024);
            break;

        case 7:     // Western Sewer Entrance
            Plr->Gossip_SendPOI(5812.79f, 722.58f, 7, 6, 0, "Dalaran: Western Sewer Entrance");
            SendEmptyMenu(14025);
            break;

        case 8:     // Sewer Tunnel
            Plr->Gossip_SendPOI(5778.03f, 743.20f, 7, 6, 0, "Dalaran: Sewer Tunnel");
            SendEmptyMenu(14020);
            break;
        }
    }
};

void Lacrimi::SetupDalaran()
{
    /* Guard List */
    RegisterCtrGossipScript(32675, DalaranGuard);
    RegisterCtrGossipScript(32676, DalaranGuard);
    RegisterCtrGossipScript(32677, DalaranGuard);
    RegisterCtrGossipScript(32678, DalaranGuard);
    RegisterCtrGossipScript(32679, DalaranGuard);
    RegisterCtrGossipScript(32680, DalaranGuard);
    RegisterCtrGossipScript(32681, DalaranGuard);
    RegisterCtrGossipScript(32683, DalaranGuard);
    RegisterCtrGossipScript(32684, DalaranGuard);
    RegisterCtrGossipScript(32685, DalaranGuard);
    RegisterCtrGossipScript(32686, DalaranGuard);
    RegisterCtrGossipScript(32687, DalaranGuard);
    RegisterCtrGossipScript(32688, DalaranGuard);
    RegisterCtrGossipScript(32689, DalaranGuard);
    RegisterCtrGossipScript(32690, DalaranGuard);
    RegisterCtrGossipScript(32691, DalaranGuard);
    RegisterCtrGossipScript(32692, DalaranGuard);
    RegisterCtrGossipScript(32693, DalaranGuard);

    RegisterCtrGossipScript(32726, UnderbellyGuard);
    RegisterCtrGossipScript(32727, UnderbellyGuard);
    RegisterCtrGossipScript(32728, UnderbellyGuard);
    RegisterCtrGossipScript(32729, UnderbellyGuard);
    RegisterCtrGossipScript(32730, UnderbellyGuard);
    RegisterCtrGossipScript(32731, UnderbellyGuard);
    RegisterCtrGossipScript(32732, UnderbellyGuard);
    RegisterCtrGossipScript(32733, UnderbellyGuard);
}
