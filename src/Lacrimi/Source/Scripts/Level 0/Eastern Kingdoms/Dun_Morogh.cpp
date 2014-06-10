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

class IronforgeMountaineer : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr, bool AutoSend)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4287, plr);
            Menu->AddItem(0, "Bank",                1);
            Menu->AddItem(0, "Gryphon Master",      2);
            Menu->AddItem(0, "Guild Master",        3);
            Menu->AddItem(0, "The Inn",             4);
            Menu->AddItem(0, "Stable Master",       5);
            Menu->AddItem(0, "Class Trainer",       6);
            Menu->AddItem(0, "Profession Trainer",  7);
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
                case 1:     //Bank
                    SendQuickMenu(4288);
                    break;

                case 2:     //Gryphon Master
                    SendQuickMenu(4289);
                    break;

                case 3:     //Guild Master
                    SendQuickMenu(4290);
                    break;

                case 4:     //The Inn
                    SendQuickMenu(4291);
                    Plr->Gossip_SendPOI(-5582.66f, -525.89f, 7, 6, 0, "Thunderbrew Distillery");
                    break;

                case 5:     //Stable Master
                    SendQuickMenu(5985);
                    Plr->Gossip_SendPOI(-5604, -509.58f, 7, 6, 0, "Shelby Stoneflint");
                    break;

                case 6:     //Class Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4292, Plr);
                        Menu->AddItem(0, "Hunter"      , 8);
                        Menu->AddItem(0, "Mage"        , 9);
                        Menu->AddItem(0, "Paladin"     , 10);
                        Menu->AddItem(0, "Priest"      , 11);
                        Menu->AddItem(0, "Rogue"       , 12);
                        Menu->AddItem(0, "Warlock"     , 13);
                        Menu->AddItem(0, "Warrior"     , 14);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 7:     // Profession Trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4300, Plr);
                        Menu->AddItem(0, "Alchemy"           , 15);
                        Menu->AddItem(0, "Blacksmithing"     , 16);
                        Menu->AddItem(0, "Cooking"           , 17);
                        Menu->AddItem(0, "Enchanting"        , 18);
                        Menu->AddItem(0, "Engineering"       , 19);
                        Menu->AddItem(0, "First Aid"         , 20);
                        Menu->AddItem(0, "Fishing"           , 21);
                        Menu->AddItem(0, "Herbalism"         , 22);
                        Menu->AddItem(0, "Inscription"       , 23);
                        Menu->AddItem(0, "Leatherworking"    , 24);
                        Menu->AddItem(0, "Mining"            , 25);
                        Menu->AddItem(0, "Skinning"          , 26);
                        Menu->AddItem(0, "Tailoring"         , 27);
                        Menu->SendTo(Plr);
                    }
                    break;

                    ////////////////
                    // Class trainer submenu
                    ////////
                case 8: //Hunter
                    {
                        Plr->Gossip_SendPOI(-5618.29f, -454.25f, 7, 6, 0, "Grif Wildheart");
                        SendQuickMenu(4293);
                    }
                    break;

                case 9: //Mage
                    {
                        Plr->Gossip_SendPOI(-5585.6f, -539.99f, 7, 6, 0, "Magis Sparkmantle");
                        SendQuickMenu(4294);
                    }
                    break;

                case 10: //Paladin
                    {
                        Plr->Gossip_SendPOI(-5585.6f, -539.99f, 7, 6, 0, "Azar Stronghammer");
                        SendQuickMenu(4295);
                    }
                    break;

                case 11: //Priest
                    {
                        Plr->Gossip_SendPOI(-5591.74f, -525.61f, 7, 6, 0, "Maxan Anvol");
                        SendQuickMenu(4296);
                    }
                    break;

                case 12: //Rogue
                    {
                        Plr->Gossip_SendPOI(-5602.75f, -542.4f, 7, 6, 0, "Hogral Bakkan");
                        SendQuickMenu(4297);
                    }
                    break;

                case 13: //Warlock
                    {
                        Plr->Gossip_SendPOI(-5641.97f, -523.76f, 7, 6, 0, "Gimrizz Shadowcog");
                        SendQuickMenu(4298);
                    }
                    break;

                case 14: //Warrior
                    {
                        Plr->Gossip_SendPOI(-5604.79f, -529.38f, 7, 6, 0, "Granis Swiftaxe");
                        SendQuickMenu(4299);
                    }
                    break;

                case 15: //Alchemy
                    {
                        SendQuickMenu(4301);
                    }
                    break;

                case 16: //Blacksmithing
                    {
                        Plr->Gossip_SendPOI(-5584.72f, -428.41f, 7, 6, 0, "Tognus Flintfire");
                        SendQuickMenu(4302);
                    }
                    break;

                case 17: //Cooking
                    {
                        Plr->Gossip_SendPOI(-5596.85f, -541.43f, 7, 6, 0, "Gremlock Pilsnor");
                        SendQuickMenu(4303);
                    }
                    break;

                case 18: //Enchanting
                    {
                        SendQuickMenu(4304);
                    }
                    break;

                case 19: //Engineering
                    {
                        SendQuickMenu(4305);
                    }
                    break;

                case 20: //First Aid
                    {
                        Plr->Gossip_SendPOI(-5603.67f, -523.57f, 7, 6, 0, "Thamner Pol");
                        SendQuickMenu(4306);
                    }
                    break;

                case 21: //Fishing
                    {
                        Plr->Gossip_SendPOI(-5202.39f, -51.36f, 7, 6, 0, "Paxton Ganter");
                        SendQuickMenu(4307);
                    }
                    break;

                case 22: //Herbalism
                    {
                        SendQuickMenu(4308);
                    }
                    break;

                case 23: //Inscription
                    {
                        Plr->Gossip_SendPOI(-4801.72f, -1189.41f, 7, 6, 0, "Ironforge Inscription");
                        SendQuickMenu(13884);
                    }
                    break;

                case 24: //Leatherworking
                    {
                        SendQuickMenu(4310);
                    }
                    break;

                case 25: //Mining
                    {
                        Plr->Gossip_SendPOI(-5531, -666.53f, 7, 6, 0, "Yarr Hamerstone");
                        SendQuickMenu(4311);
                    }
                    break;

                case 26: //Skinning
                    {
                        SendQuickMenu(4312);
                    }
                    break;

                case 27: //Tailoring
                    {
                        SendQuickMenu(4313);
                    }
                    break;
            }
        }
};

void Lacrimi::SetupDunMorogh()
{
    RegisterCtrGossipScript(727,     IronforgeMountaineer);     // Ironforge Mountaineer
}
