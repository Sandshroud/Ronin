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

class BloodhoofGuard : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr, bool AutoSend)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3543, plr);
            Menu->AddItem(0, "The bank",               1);
            Menu->AddItem(0, "The wind rider master",  2);
            Menu->AddItem(0, "The inn",                3);
            Menu->AddItem(0, "The stable master",      4);
            Menu->AddItem(0, "A class trainer",        5);
            Menu->AddItem(0, "A profession trainer",   6);
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
                    SendQuickMenu(4051);
                    break;

                case 2:     // The wind rider master
                    SendQuickMenu(4052);
                    break;

                case 3:     // The inn
                    SendQuickMenu(4053);
                    Plr->Gossip_SendPOI(-2361.38f, -349.19f, 7, 6, 0, "Bloodhoof Village Inn");
                    break;

                case 4:     // The stable master
                    SendQuickMenu(5976);
                    Plr->Gossip_SendPOI(-2338.86f, -357.56f, 7, 6, 0, "Seikwa");
                    break;

                case 5:     // A class trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4069, Plr);
                        Menu->AddItem(0, "Druid"       , 7);
                        Menu->AddItem(0, "Hunter"      , 8);
                        Menu->AddItem(0, "Shaman"      , 9);
                        Menu->AddItem(0, "Warrior"    , 10);
                        Menu->SendTo(Plr);
                    }
                    break;

                case 6:     // A profession trainer
                    {
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3541, Plr);
                        Menu->AddItem(0, "Alchemy"           , 11);
                        Menu->AddItem(0, "Blacksmithing"     , 12);
                        Menu->AddItem(0, "Cooking"           , 13);
                        Menu->AddItem(0, "Enchanting"        , 14);
                        Menu->AddItem(0, "First Aid"         , 15);
                        Menu->AddItem(0, "Fishing"           , 16);
                        Menu->AddItem(0, "Herbalism"         , 17);
                        Menu->AddItem(0, "Leatherworking"    , 18);
                        Menu->AddItem(0, "Mining"            , 19);
                        Menu->AddItem(0, "Skinning"          , 20);
                        Menu->AddItem(0, "Tailoring"         , 21);
                        Menu->SendTo(Plr);
                    }
                    break;

                    ////////////////
                    // Class trainer submenu
                    ////////
                case 7: //Druid
                    {
                        Plr->Gossip_SendPOI(-2312.15f, -443.69f, 7, 6, 0, "Gennia Runetotem");
                        SendQuickMenu(4054);
                    }
                    break;

                case 8: //Hunter
                    {
                        Plr->Gossip_SendPOI(-2178.14f, -406.14f, 7, 6, 0, "Yaw Sharpmane");
                        SendQuickMenu(4055);
                    }
                    break;

                case 9: //Shaman
                    {
                        Plr->Gossip_SendPOI(-2301.5f, -439.87f, 7, 6, 0, "Narm Skychaser");
                        SendQuickMenu(4056);
                    }
                    break;

                case 10: //Warrior
                    {
                        Plr->Gossip_SendPOI(-2345.43f, -494.11f, 7, 6, 0, "Krang Stonehoof");
                        SendQuickMenu(4057);
                    }
                    break;

                case 11: //Alchemy
                    {
                        SendQuickMenu(4058);
                    }
                    break;

                case 12: //Blacksmithing
                    {
                        SendQuickMenu(4059);
                    }
                    break;

                case 13: //Cooking
                    {
                        Plr->Gossip_SendPOI(-2263.34f, -287.91f, 7, 6, 0, "Pyall Silentstride");
                        SendQuickMenu(4060);
                    }
                    break;

                case 14: //Enchanting
                    {
                        SendQuickMenu(4061);
                    }
                    break;

                case 15: //First Aid
                    {
                        Plr->Gossip_SendPOI(-2353.52f, -355.82f, 7, 6, 0, "Vira Younghoof");
                        SendQuickMenu(4062);
                    }
                    break;

                case 16: //Fishing
                    {
                        Plr->Gossip_SendPOI(-2349.21f, -241.37f, 7, 6, 0, "Uthan Stillwater");
                        SendQuickMenu(4063);
                    }
                    break;

                case 17: //Herbalism
                    {
                        SendQuickMenu(4064);
                    }
                    break;

                case 18: //Leatherworking
                    {
                        Plr->Gossip_SendPOI(-2257.12f, -288.63f, 7, 6, 0, "Chaw Stronghide");
                        SendQuickMenu(4065);
                    }
                    break;

                case 19: //Mining
                    {
                        SendQuickMenu(4066);
                    }
                    break;

                case 20: //Skinning
                    {
                        Plr->Gossip_SendPOI(-2252.94f, -291.32f, 7, 6, 0, "Yonn Deepcut");
                        SendQuickMenu(4067);
                    }
                    break;

                case 21: //Tailoring
                    {
                        SendQuickMenu(4068);
                    }
                    break;
            }
        }
};

void Lacrimi::SetupMulgore()
{
    RegisterCtrGossipScript(3222,    BloodhoofGuard);       // Brave Wildrunner
    RegisterCtrGossipScript(3224,    BloodhoofGuard);       // Brave Cloudmane
    RegisterCtrGossipScript(3220,    BloodhoofGuard);       // Brave Darksky
    RegisterCtrGossipScript(3219,    BloodhoofGuard);       // Brave Leaping Deer
    RegisterCtrGossipScript(3217,    BloodhoofGuard);       // Brave Dawneagle
    RegisterCtrGossipScript(3215,    BloodhoofGuard);       // Brave Strongbash
    RegisterCtrGossipScript(3218,    BloodhoofGuard);       // Brave Swiftwind
    RegisterCtrGossipScript(3221,    BloodhoofGuard);       // Brave Rockhorn
    RegisterCtrGossipScript(3223,    BloodhoofGuard);       // Brave Rainchaser
    RegisterCtrGossipScript(3212,    BloodhoofGuard);       // Brave Ironhorn
}
