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

// Exarch Nasuun
#define GOSSIP_EXARCH_NASUUN_1 "Nasuun, do you know how long until we have an alchemy lab at the Sun's Reach Harbor?"
#define GOSSIP_EXARCH_NASUUN_2 "What do you know about the magical gates at the Sunwell Plateau being brought down?"
#define GOSSIP_EXARCH_NASUUN_3 "I have something else to ask you about."

//#define USE_THE_STATUS 1  // Decomment this is for the status

class ExarchNasuun_Gossip : public GossipScript
{
public:
    void GossipHello(Object *pObject, Player *plr, bool AutoSend)
    {
        GossipMenu *Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 12227, plr);

#ifdef USE_THE_STATUS
        Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_1, 1); // Status of the lab
        Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_2, 2);
#else
        Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_2, 3);
#endif
        if(AutoSend)
            Menu->SendTo(plr);
    }

    void GossipSelectOption(Object *pObject, Player *plr, uint32 Id, uint32 IntId, const char * Code)
    {
        if(pObject->GetTypeId() != TYPEID_UNIT)
            return;

        GossipMenu * Menu;
        switch(IntId)
        {
        case 0:
            GossipHello(pObject, plr, true);
            break;
        case 1:
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 12303, plr);
                Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_3, 0);
                Menu->SendTo(plr);
            }break;
        case 2:
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 12305, plr);
                Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_3, 0);
                Menu->SendTo(plr);
            }break;
        case 3:
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 12623, plr);
                Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_3, 0);
                Menu->SendTo(plr);
            }break;
        }
    }

    void Destroy()
    {
        delete this;
    }
};

class ZephyrGossipScript : public GossipScript
{
public:
    void GossipHello(Object *pObject, Player *Plr, bool AutoSend)
    {
        GossipMenu *Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 1, Plr);
        if(Plr->GetStanding(989) >= 21000)
            Menu->AddItem(0, "Bring me to Caverns of Time!", 1); 
        Menu->SendTo(Plr);
    }

    void GossipSelectOption(Object *pObject, Player *plr, uint32 Id, uint32 IntId, const char * EnteredCode)
    {
        Creature *Zephyr = pObject->IsCreature() ? TO_CREATURE(pObject) : NULL;
        if (Zephyr == NULLCREATURE)
            return;

        switch (IntId)
        {
        case 0:
            GossipHello(pObject, plr, true);
            break;
        case 1:
            plr->Gossip_Complete();
            Zephyr->CastSpell(plr, dbcSpell.LookupEntry(37778), true);
            break;
        }
    }

    void Destroy()
    {
        delete this;
    }
};

class ShattrathGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr, bool AutoSend)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10524, plr);

            Menu->AddItem(0, "World's End Tavern"  , 1);
            Menu->AddItem(0, "Bank"                , 2);
            Menu->AddItem(0, "Inn"                 , 3);
            Menu->AddItem(0, "Flight Master"       , 4);
            Menu->AddItem(0, "Mailbox"             , 5);
            Menu->AddItem(0, "Stable Master"       , 6);
            Menu->AddItem(0, "Battlemaster"        , 7);
            Menu->AddItem(0, "Profession Trainer"  , 8);
            Menu->AddItem(0, "Mana Loom"           , 9);
            Menu->AddItem(0, "Alchemy Lab"         , 10);
            Menu->AddItem(0, "Gem Merchant"        , 11);
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
                    // Menus
                    /////
                case 1:     // World's End Tavern
                    SendQuickMenu(10394);
                    Plr->Gossip_SendPOI(-1760.4f, 5166.9f, 7, 6, 0, "World's End Tavern");
                    break;

                case 2:     // Shattrath Banks
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10395, Plr);
                        Menu->AddItem(0, "Aldor Bank"          , 12);
                        Menu->AddItem(0, "Scryers Bank"        , 13);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 3:     // Inn's
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10398, Plr);
                        Menu->AddItem(0, "Aldor inn"          , 14);
                        Menu->AddItem(0, "Scryers inn"        , 15);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 4:     // Gryphon Master
                    SendQuickMenu(10402);
                    Plr->Gossip_SendPOI(-1831.9f, 5298.2f, 7, 6, 0, "Gryphon Master");
                    break;

                case 5:     // Mailboxes
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10403, Plr);
                        Menu->AddItem(0, "Aldor inn"          , 16);
                        Menu->AddItem(0, "Scryers inn"        , 17);
                        Menu->AddItem(0, "Aldor Bank"         , 18);
                        Menu->AddItem(0, "Scryers Bank"       , 19);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 6:     // Stable Masters
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10404, Plr);
                        Menu->AddItem(0, "Aldor Stable"         , 20);
                        Menu->AddItem(0, "Scryers Stable"       , 21);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 7:     // Battlemasters
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10405, Plr);
                        Menu->AddItem(0, "Alliance Battlemasters"          , 22);
                        Menu->AddItem(0, "Horde & Arena Battlemasters"     , 23);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 8:     // Proffesion Trainers
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10391, Plr);
                        Menu->AddItem(0, "Alchemy"              , 24);
                        Menu->AddItem(0, "Blacksmithing"        , 25);
                        Menu->AddItem(0, "Cooking"              , 26);
                        Menu->AddItem(0, "Enchanting"           , 27);
                        Menu->AddItem(0, "First Aid"            , 28);
                        Menu->AddItem(0, "Jewelcrafting"        , 29);
                        Menu->AddItem(0, "Leatherworking"       , 30);
                        Menu->AddItem(0, "Skinning"             , 31);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 9:     // Mana Loom
                    SendQuickMenu(10408);
                    Plr->Gossip_SendPOI(-2073.9f, 5265.7f, 7, 6, 0, "Mana Loom");
                    break;

                case 10:    // Alchemy Lab
                    SendQuickMenu(10409);
                    Plr->Gossip_SendPOI(-1648.1f, 5537.3f, 7, 6, 0, "Alchemy Lab");
                    break;

                case 11:    // Gem Merchants
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 10410, Plr);
                        Menu->AddItem(0, "Aldor Gem Merchant"          , 32);
                        Menu->AddItem(0, "Scryer Gem Merchant"         , 33);
                        Menu->SendTo(Plr);
                    }
                    break;

                    //////////////////////
                    // Banks
                    /////
                case 12: //Aldor Bank
                    {
                        Plr->Gossip_SendPOI(-1730.8f, 5496.2f, 7, 6, 0, "Aldor Bank");
                        SendQuickMenu(10396);
                    }
                    break;

                case 13: //Scryers Bank
                    {
                        Plr->Gossip_SendPOI(-1999.6f, 5362.0f, 7, 6, 0, "Scryers Bank");
                        SendQuickMenu(10397);
                    }
                    break;

                    //////////////////////
                    // Inns
                    /////
                case 14: //Aldor Inn
                    {
                        Plr->Gossip_SendPOI(-1897.5f, 5767.6f, 7, 6, 0, "Aldor inn");
                        SendQuickMenu(10399);
                    }
                    break;

                case 15: //Scryers Inn
                    {
                        Plr->Gossip_SendPOI(-2180.6f, 5403.9f, 7, 6, 0, "Scryers inn");
                        SendQuickMenu(10401);
                    }
                    break;

                    //////////////////////
                    // Mailboxes
                    /////
                case 16: //Aldor Inn
                    {
                        Plr->Gossip_SendPOI(-1886.9f, 5761.5f, 7, 6, 0, "Aldor Inn");
                        SendQuickMenu(10399);
                    }
                    break;

                case 17: //Scryers Bank
                    {
                        Plr->Gossip_SendPOI(-2175.0f, 5411.7f, 7, 6, 0, "Scryers Bank");
                        SendQuickMenu(10397);
                    }
                    break;

                case 18: //Aldor Bank
                    {
                        Plr->Gossip_SendPOI(-1695.5f, 5521.8f, 7, 6, 0, "Aldor Bank");
                        SendQuickMenu(10396);
                    }
                    break;

                case 19: //Scryers Inn
                    {
                        Plr->Gossip_SendPOI(-2033.0f, 5336.1f, 7, 6, 0, "Scryers Inn");
                        SendQuickMenu(10401);
                    }
                    break;

                    //////////////////////
                    // Stable Masters
                    /////
                case 20: //Aldor Stable Master
                    {
                        Plr->Gossip_SendPOI(-1889.6f, 5761.5f, 7, 6, 0, "Aldor Stable");
                        SendQuickMenu(10399);
                    }
                    break;

                case 21: //Scryers Stable Master
                    {
                        Plr->Gossip_SendPOI(-2170.0f, 5404.6f, 7, 6, 0, "Scryers Stable");
                        SendQuickMenu(10401);
                    }
                    break;

                    //////////////////////
                    // Battlemasters
                    /////
                case 22: //Alliance Battlemaster
                    {
                        Plr->Gossip_SendPOI(-1831.9f, 5298.2f, 7, 6, 0, "Alliance Battlemasters");
                        SendQuickMenu(10406);
                    }
                    break;

                case 23: //Horde Battle Master and Arena Battle Master
                    {
                        Plr->Gossip_SendPOI(-1968.7f, 5262.2f, 7, 6, 0, "Horde & Arena Battlemasters");
                        SendQuickMenu(10407);
                    }
                    break;

                    //////////////////////
                    // Profession Trainers
                    /////
                case 24: //Alchemy
                    {
                        Plr->Gossip_SendPOI(-1661.0f, 5538, 7, 6, 0, "Alchemy Trainer");
                        SendQuickMenu(10413);
                    }
                    break;

                case 25: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(-1847.7f, 5230.3f, 7, 6, 0, "Blacksmithing Trainer");
                        SendQuickMenu(10400);
                    }
                    break;

                case 26: //Cooking
                    {
                        Plr->Gossip_SendPOI(-2067.4f, 5316.5f, 7, 6, 0, "Cooking Trainer");
                        SendQuickMenu(10414);
                    }
                    break;

                case 27: //Enchanting
                    {
                        Plr->Gossip_SendPOI(-2278.5f, 5567.7f, 7, 6, 0, "Enchanting Trainer");
                        SendQuickMenu(10415);
                    }
                    break;

                case 28: //First Aid
                    {
                        Plr->Gossip_SendPOI(-1592.0f, 5263.7f, 7, 6, 0, "First Aid Trainer");
                        SendQuickMenu(10416);
                    }
                    break;

                case 29: //Jewelcrafting
                    {
                        Plr->Gossip_SendPOI(-1653.3f, 5665.1f, 7, 6, 0, "Jewelcrafting Trainer");
                        SendQuickMenu(10417);
                    }
                    break;

                case 30: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(-2060.9f, 5256.6f, 7, 6, 0, "Leatherworking Trainer");
                        SendQuickMenu(10418);
                    }
                    break;

                case 31: //Skinning
                    {
                        Plr->Gossip_SendPOI(-2047.9f, 5299.6f, 7, 6, 0, "Skinning Trainer");
                        SendQuickMenu(10419);
                    }
                    break;

                    //////////////////////
                    // Gem Merchants
                    /////
                case 32: //Aldor gem merchant
                    {
                        Plr->Gossip_SendPOI(-1649.3f, 5668.6f, 7, 6, 0, "Aldor gem merchant");
                        SendQuickMenu(10411);
                    }
                    break;

                case 33: //Scryers gem merchant
                    {
                        Plr->Gossip_SendPOI(-2193.9f, 5422.1f, 7, 6, 0, "Scryers gem merchant");
                        SendQuickMenu(10412);
                    }
                    break;
            }
        }
};

void Lacrimi::SetupShattrath()
{
    RegisterCtrGossipScript(25967,    ZephyrGossipScript);
    RegisterCtrGossipScript(24932,    ExarchNasuun_Gossip);
    RegisterCtrGossipScript(19687,    ShattrathGuard);            // Shattrath City Guard Peacekeeper
    RegisterCtrGossipScript(18568,    ShattrathGuard);            // Shattrath City Scryer Arcane Guardian
    RegisterCtrGossipScript(18549,    ShattrathGuard);            // Shattrath City Aldor Vindicator
}
