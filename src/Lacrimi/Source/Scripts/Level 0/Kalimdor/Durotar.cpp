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

// Razor Hill
class RazorHillGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr, bool AutoSend)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4037, plr);
            Menu->AddItem(0, "The bank",              1);
            Menu->AddItem(0, "The wind rider master", 2);
            Menu->AddItem(0, "The inn",               3);
            Menu->AddItem(0, "The stable master",     4);
            Menu->AddItem(0, "A class trainer",       5);
            Menu->AddItem(0, "A profession trainer",  6);

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
                    SendEmptyMenu(4032);
                    break;

                case 2:     // The wind rider master
                    SendEmptyMenu(4033);
                    break;

                case 3:     // The inn
                    Plr->Gossip_SendPOI(338.7f, -4688.87f, 7, 6, 0, "Razor Hill Inn");
                    SendEmptyMenu(4034);
                    break;

                case 4:     // The stable master
                    Plr->Gossip_SendPOI(330.31f, -4710.66f, 7, 6, 0, "Shoja'my");
                    SendEmptyMenu(5973);
                    break;

                case 5:     // A class trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4035, Plr);
                        Menu->AddItem(0, "Hunter"       , 7);
                        Menu->AddItem(0, "Mage"         , 8);
                        Menu->AddItem(0, "Priest"       , 9);
                        Menu->AddItem(0, "Rogue"       , 10);
                        Menu->AddItem(0, "Shaman"      , 11);
                        Menu->AddItem(0, "Warlock"     , 12);
                        Menu->AddItem(0, "Warrior"     , 13);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 6:     // A profession trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3541, Plr);
                        Menu->AddItem(0, "Alchemy"           , 14);
                        Menu->AddItem(0, "Blacksmithing"     , 15);
                        Menu->AddItem(0, "Cooking"           , 16);
                        Menu->AddItem(0, "Enchanting"        , 17);
                        Menu->AddItem(0, "Engineering"       , 18);
                        Menu->AddItem(0, "First Aid"         , 19);
                        Menu->AddItem(0, "Fishing"           , 20);
                        Menu->AddItem(0, "Herbalism"         , 21);
                        Menu->AddItem(0, "Leatherworking"    , 22);
                        Menu->AddItem(0, "Mining"            , 23);
                        Menu->AddItem(0, "Skinning"          , 24);
                        Menu->AddItem(0, "Tailoring"         , 25);
                        Menu->SendTo(Plr);
                    }
                    break;

                    ////////////////
                    // Class trainer submenu
                    ////////
                case 7: //Hunter
                    {
                        Plr->Gossip_SendPOI(276, -4706.72f, 7, 6, 0, "Thotar");
                        SendQuickMenu(4013);
                    }
                    break;

                case 8: //Mage
                    {
                        Plr->Gossip_SendPOI(-839.33f, -4935.6f, 7, 6, 0, "Un'Thuwa");
                        SendQuickMenu(4014);
                    }
                    break;

                case 9: //Priest
                    {
                        Plr->Gossip_SendPOI(296.22f, -4828.1f, 7, 6, 0, "Tai'jin");
                        SendQuickMenu(4015);
                    }
                    break;

                case 10: //Rogue
                    {
                        Plr->Gossip_SendPOI(265.76f, -4709, 7, 6, 0, "Kaplak");
                        SendQuickMenu(4016);
                    }
                    break;

                case 11: //Shaman
                    {
                        Plr->Gossip_SendPOI(307.79f, -4836.97f, 7, 6, 0, "Swart");
                        SendQuickMenu(4017);
                    }
                    break;

                case 12: //Warlock
                    {
                        Plr->Gossip_SendPOI(355.88f, -4836.45f, 7, 6, 0, "Dhugru Gorelust");
                        SendQuickMenu(4018);
                    }
                    break;

                case 13: //Warrior
                    {
                        Plr->Gossip_SendPOI(312.3f, -4824.66f, 7, 6, 0, "Tarshaw Jaggedscar");
                        SendQuickMenu(4019);
                    }
                    break;

                case 14: //Alchemy
                    {
                        Plr->Gossip_SendPOI(-800.25f, -4894.33f, 7, 6, 0, "Miao'zan");
                        SendQuickMenu(4020);
                    }
                    break;

                case 15: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(373.24f, -4716.45f, 7, 6, 0, "Dwukk");
                        SendQuickMenu(4021);
                    }
                    break;

                case 16: //Cooking
                    {
                        SendQuickMenu(4022);
                    }
                    break;

                case 17: //Enchanting
                    {
                        SendQuickMenu(4023);
                    }
                    break;

                case 18: //Engineering
                    {
                        Plr->Gossip_SendPOI(368.95f, -4723.95f, 7, 6, 0, "Mukdrak");
                        SendQuickMenu(4024);
                    }
                    break;

                case 19: //First Aid
                    {
                        Plr->Gossip_SendPOI(327.17f, -4825.62f, 7, 6, 0, "Rawrk");
                        SendQuickMenu(4025);
                    }
                    break;

                case 20: //Fishing
                    {
                        Plr->Gossip_SendPOI(-1065.48f, -4777.43f, 7, 6, 0, "Lau'Tiki");
                        SendQuickMenu(4026);
                    }
                    break;

                case 21: //Herbalism
                    {
                        Plr->Gossip_SendPOI(-836.25f, -4896.89f, 7, 6, 0, "Mishiki");
                        SendQuickMenu(4027);
                    }
                    break;

                case 22: //Leatherworking
                    {
                        SendQuickMenu(4028);
                    }
                    break;

                case 23: //Mining
                    {
                        Plr->Gossip_SendPOI(366.94f, -4705, 7, 6, 0, "Krunn");
                        SendQuickMenu(4029);
                    }
                    break;

                case 24: //Skinning
                    {
                        Plr->Gossip_SendPOI(-2252.94f, -291.32f, 7, 6, 0, "Yonn Deepcut");
                        SendQuickMenu(4030);
                    }
                    break;

                case 25: //Tailoring
                    {
                        SendQuickMenu(4031);
                    }
                    break;
            }
        }
};

void Lacrimi::SetupDurotar()
{
    RegisterCtrGossipScript(5953,    RazorHillGuard);    // Razor Hill Grunt
}
