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

class GoldshireGuard : public GossipScript
{
public:
    void Destroy()
    {
        delete this;
    }

    void GossipHello(Object *pObject, Player *Plr, bool AutoSend)
    {
        GossipMenu *Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4259, Plr);
        Menu->AddItem( 0, "Bank"                , 1);
        Menu->AddItem( 0, "Gryphon Master"      , 2);
        Menu->AddItem( 0, "Guild Master"        , 3);
        Menu->AddItem( 0, "Inn"                 , 4);
        Menu->AddItem( 0, "Stable Master"       , 5);
        Menu->AddItem( 0, "Class Trainer"       , 6);
        Menu->AddItem( 0, "Profession Trainer"  , 7);
        if(AutoSend)
            Menu->SendTo(Plr);
    }

    void GossipSelectOption(Object *pObject, Player *Plr, uint32 Id, uint32 IntId, const char * Code)
    {
        GossipMenu * Menu;
        switch(IntId)
        {
        case 0: // Return to start
            GossipHello(pObject, Plr, true);
            break;

            //////////////////////
            // Main menu handlers
            /////

        case 1:     // Bank
            SendQuickMenu(4260);
            break;

        case 2:     // Gryphon Master
            SendQuickMenu(4261);
            break;

        case 3:     // Guild Master
            SendQuickMenu(4262);
            break;

        case 4:     // Inn
            SendQuickMenu(4263);
            Plr->Gossip_SendPOI(-9459.34f, 42.08f, 7, 6, 0, "Lion's Pride Inn");
            break;

        case 5:     // Stable Master
            SendQuickMenu(5983);
            Plr->Gossip_SendPOI(-9466.62f, 45.87f, 7, 6, 0, "Erma");
            break;

        case 6:     // Class Trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4264, Plr);
                Menu->AddItem( 0, "Druid",  8);
                Menu->AddItem( 0, "Hunter", 9);
                Menu->AddItem( 0, "Mage",   10);
                Menu->AddItem( 0, "Paladin",11);
                Menu->AddItem( 0, "Priest", 12);
                Menu->AddItem( 0, "Rogue",  13);
                Menu->AddItem( 0, "Shaman", 14);
                Menu->AddItem( 0, "Warlock",15);
                Menu->AddItem( 0, "Warrior",16);
                Menu->SendTo(Plr);
            }break;

        case 7:     // Profession Trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4273, Plr);
                Menu->AddItem( 0, "Alchemy",        17);
                Menu->AddItem( 0, "Blacksmithing",  18);
                Menu->AddItem( 0, "Cooking",        19);
                Menu->AddItem( 0, "Enchanting",     20);
                Menu->AddItem( 0, "Engineering",    21);
                Menu->AddItem( 0, "First Aid",      22);
                Menu->AddItem( 0, "Fishing",        23);
                Menu->AddItem( 0, "Herbalism",      24);
                Menu->AddItem( 0, "Inscription",    25);
                Menu->AddItem( 0, "Leatherworking", 26);
                Menu->AddItem( 0, "Mining",         27);
                Menu->AddItem( 0, "Skinning",       28);
                Menu->AddItem( 0, "Tailoring",      29);
                Menu->SendTo(Plr);
            }break;

        case 8:     // Druid
            {
                SendQuickMenu(4265);
            }break;

        case 9:     // Hunter
            {
                SendQuickMenu(4266);
            }break;

        case 10:    // Mage
            {
                Plr->Gossip_SendPOI(-9471.12f, 33.44f, 7, 6, 0, "Zaldimar Wefhellt");
                SendQuickMenu(4268);
            }break;

        case 11:    // Paladin
            {
                Plr->Gossip_SendPOI(-9469.0f, 108.05f, 7, 6, 0, "Brother Wilhelm");
                SendQuickMenu(4269);
            }break;

        case 12:    // Priest
            {
                Plr->Gossip_SendPOI(-9461.07f, 32.6f, 7, 6, 0, "Priestess Josetta");
                SendQuickMenu(4267);
            }break;

        case 13:    // Rogue
            {
                Plr->Gossip_SendPOI(-9465.13f, 13.29f, 7, 6, 0, "Keryn Sylvius");
                SendQuickMenu(4270);
            }break;

        case 14:    // Shaman
            {
                SendQuickMenu(10101);
            }break;

        case 15:    // Warlock
            {
                Plr->Gossip_SendPOI(-9473.21f, -4.08f, 7, 6, 0, "Maximillian Crowe");
                SendQuickMenu(4272);
            }break;

        case 16:    // Warrior
            {
                Plr->Gossip_SendPOI(-9461.82f, 109.50f, 7, 6, 0, "Lyria Du Lac");
                SendQuickMenu(4271);
            }break;

        case 17:    // Alchemy
            {
                Plr->Gossip_SendPOI(-9057.04f, 153.63f, 7, 6, 0, "Alchemist Mallory");
                SendQuickMenu(4274);
            }break;

        case 18:    // Blacksmithing
            {
                Plr->Gossip_SendPOI(-9456.58f, 87.90f, 7, 6, 0, "Smith Argus");
                SendQuickMenu(4275);
            }break;

        case 19:    // Cooking
            {
                Plr->Gossip_SendPOI(-9467.54f, -3.16f, 7, 6, 0, "Tomas");
                SendQuickMenu(4276);
            }break;

        case 20:    // Enchanting
            {
                SendQuickMenu(4277);
            }break;

        case 21:    // Engineering
            {
                SendQuickMenu(4278);
            }break;

        case 22:    // First Aid
            {
                Plr->Gossip_SendPOI(-9456.82f, 30.49f, 7, 6, 0, "Michelle Belle");
                SendQuickMenu(4279);
            }break;

        case 23:    // Fishing
            {
                Plr->Gossip_SendPOI(-9386.54f, -118.73f, 7, 6, 0, "Lee Brown");
                SendQuickMenu(4280);
            }break;

        case 24:    // Herbalism
            {
                Plr->Gossip_SendPOI(-9060.70f, 149.23f, 7, 6, 0, "Herbalist Pomeroy");
                SendQuickMenu(4281);
            }break;

        case 25:    // Inscription
            {
                Plr->Gossip_SendPOI(-8853.33f, 857.66f, 7, 6, 0, "Stormwind Inscription");
                SendQuickMenu(13881);
            }break;

        case 26:    // Leatherworking
            {
                Plr->Gossip_SendPOI(-9376.12f, -75.23f, 7, 6, 0, "Adele Fielder");
                SendQuickMenu(4282);
            }break;

        case 27:    // Mining
            {
                SendQuickMenu(4283);
            }break;

        case 28:    // Skinning
            {
                Plr->Gossip_SendPOI(-9536.91f, -1212.76f, 7, 6, 0, "Helene Peltskinner");
                SendQuickMenu(4284);
            }break;

        case 29:    // Tailoring
            {
                Plr->Gossip_SendPOI(-9376.12f, -75.23f, 7, 6, 0, "Eldrin");
                SendQuickMenu(4285);
            }break;
        }
    }
};

void Lacrimi::SetupElwynnForest()
{
    RegisterCtrGossipScript(1423,    GoldshireGuard);        // Stormwind Guard
}
