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

class UndercityGuard : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr, bool AutoSend)
    {
        GossipMenu* Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3543, plr);

        Menu->AddItem(0, "The bank"              , 1);
        Menu->AddItem(0, "The bat handler"       , 2);
        Menu->AddItem(0, "The guild master"      , 3);
        Menu->AddItem(0, "The inn"               , 4);
        Menu->AddItem(0, "The mailbox"           , 5);
        Menu->AddItem(0, "The auction house"     , 6);
        Menu->AddItem(0, "The zeppelin master"   , 7);
        Menu->AddItem(0, "The weapon master"     , 8);
        Menu->AddItem(0, "The stable master"     , 9);
        Menu->AddItem(0, "The battlemaster"      , 10);
        Menu->AddItem(0, "A class trainer"       , 11);
        Menu->AddItem(0, "A profession trainer"  , 12);
        Menu->AddItem(0, "Locksmith"             , 32);
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
            Plr->Gossip_SendPOI(1595.64f, 232.45f, 7, 6, 0, "Undercity Bank");
            SendEmptyMenu(3514);
            break;

        case 2:     // The bat handler
            Plr->Gossip_SendPOI(1565.9f, 271.43f, 7, 6, 0, "Undercity Bat Handler");
            SendEmptyMenu(3515);
            break;

        case 3:     // The guild master
            Plr->Gossip_SendPOI(1594.17f, 205.57f, 7, 6, 0, "Undercity Guild Master");
            SendEmptyMenu(3516);
            break;

        case 4:     // The inn
            Plr->Gossip_SendPOI(1639.43f, 220.99f, 7, 6, 0, "Undercity Inn");
            SendEmptyMenu(3517);
            break;

        case 5:     // The mailbox
            Plr->Gossip_SendPOI(1632.68f, 219.4f, 7, 6, 0, "Undercity Mailbox");
            SendEmptyMenu(3518);
            break;

        case 6:     // The auction house
            Plr->Gossip_SendPOI(1647.9f, 258.49f, 7, 6, 0, "Undercity Auction House");
            SendEmptyMenu(3520);
            break;

        case 7:     // The zeppelin master
            Plr->Gossip_SendPOI(2059, 274.86f, 7, 6, 0, "Undercity Zeppelin");
            SendEmptyMenu(3519);
            break;

        case 8:     // The weapon master
            Plr->Gossip_SendPOI(1670.31f, 324.66f, 7, 6, 0, "Archibald");
            SendEmptyMenu(4521);
            break;

        case 9:     // The stable master
            Plr->Gossip_SendPOI(1634.18f, 226.76f, 7, 6, 0, "Anya Maulray");
            SendEmptyMenu(5979);
            break;

        case 10:    // The battlemaster
            Plr->Gossip_SendPOI(1300.33f, 350.92f, 7, 6, 0, "Battlemasters Undercity");
            SendEmptyMenu(7527);
            break;

        case 11:    // A class trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3542, Plr);
                Menu->AddItem(0, "Mage"         , 13);
                Menu->AddItem(0, "Paladin"      , 14);
                Menu->AddItem(0, "Priest"       , 15);
                Menu->AddItem(0, "Rogue"        , 16);
                Menu->AddItem(0, "Warlock"      , 17);
                Menu->AddItem(0, "Warrior"      , 18);
                Menu->SendTo(Plr);
            }
            break;

        case 12:    // A profession trainer
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3541, Plr);
                Menu->AddItem(0, "Alchemy"           , 19);
                Menu->AddItem(0, "Blacksmithing"     , 20);
                Menu->AddItem(0, "Cooking"           , 21);
                Menu->AddItem(0, "Enchanting"        , 22);
                Menu->AddItem(0, "Engineering"       , 23);
                Menu->AddItem(0, "First Aid"         , 24);
                Menu->AddItem(0, "Fishing"           , 25);
                Menu->AddItem(0, "Herbalism"         , 26);
                Menu->AddItem(0, "Leatherworking"    , 27);
                Menu->AddItem(0, "Mining"            , 28);
                Menu->AddItem(0, "Skinning"          , 29);
                Menu->AddItem(0, "Tailoring"         , 30);
                Menu->SendTo(Plr);
            }
            break;

            ////////////////
            // Class trainer submenu
            ////////

        case 13: //Mage
            Plr->Gossip_SendPOI(1781, 53, 7, 6, 0, "Undercity Mage Trainers");
            SendQuickMenu(3513);
            break;

        case 14: //Paladin
            Plr->Gossip_SendPOI(1298.98f, 316.51f, 7, 6, 0, "Champion Cyssa Downrose");
            SendQuickMenu(3521);
            break;

        case 15: //Priest
            Plr->Gossip_SendPOI(1758.33f, 401.5f, 7, 6, 0, "Undercity Priest Trainers");
            SendQuickMenu(3521);
            break;

        case 16: //Rogue
            Plr->Gossip_SendPOI(1418.56f, 65, 7, 6, 0, "Undercity Rogue Trainers");
            SendQuickMenu(3526);
            break;

        case 17: //Warlock
            Plr->Gossip_SendPOI(1780.92f, 53.16f, 7, 6, 0, "Undercity Warlock Trainers");
            SendQuickMenu(3526);
            break;

        case 18: //Warrior
            Plr->Gossip_SendPOI(1775.59f, 418.19f, 7, 6, 0, "Undercity Warrior Trainers");
            SendQuickMenu(3527);
            break;

        case 19: //Alchemy
            Plr->Gossip_SendPOI(1419.82f, 417.19f, 7, 6, 0, "The Apothecarium");
            SendQuickMenu(3528);
            break;

        case 20: //Blacksmithing
            Plr->Gossip_SendPOI(1696, 285, 7, 6, 0, "Undercity Blacksmithing Trainer");
            SendQuickMenu(3529);
            break;

        case 21: //Cooking
            Plr->Gossip_SendPOI(1596.34f, 274.68f, 7, 6, 0, "Undercity Cooking Trainer");
            SendQuickMenu(3530);
            break;

        case 22: //Enchanting
            Plr->Gossip_SendPOI(1488.54f, 280.19f, 7, 6, 0, "Undercity Enchanting Trainer");
            SendQuickMenu(3531);
            break;

        case 23: //Engineering
            Plr->Gossip_SendPOI(1408.58f, 143.43f, 7, 6, 0, "Undercity Engineering Trainer");
            SendQuickMenu(3532);
            break;

        case 24: //First Aid
            Plr->Gossip_SendPOI(1519.65f, 167.19f, 7, 6, 0, "Undercity First Aid Trainer");
            SendQuickMenu(3533);
            break;

        case 25: //Fishing
            Plr->Gossip_SendPOI(1679.9f, 89, 7, 6, 0, "Undercity Fishing Trainer");
            SendQuickMenu(3534);
            break;

        case 26: //Herbalism
            Plr->Gossip_SendPOI(1558, 349.36f, 7, 6, 0, "Undercity Herbalism Trainer");
            SendQuickMenu(3535);
            break;

        case 27: //Leatherworking
            Plr->Gossip_SendPOI(1498.76f, 196.43f, 7, 6, 0, "Undercity Leatherworking Trainer");
            SendQuickMenu(3536);
            break;

        case 28: //Mining
            Plr->Gossip_SendPOI(1642.88f, 335.58f, 7, 6, 0, "Undercity Mining Trainer");
            SendQuickMenu(3537);
            break;

        case 29: //Skinning
            Plr->Gossip_SendPOI(1498.6f, 196.46f, 7, 6, 0, "Undercity Skinning Trainer");
            SendQuickMenu(3538);
            break;

        case 30: //Tailoring
            Plr->Gossip_SendPOI(1689.55f, 193, 7, 6, 0, "Undercity Tailoring Trainer");
            SendQuickMenu(3539);
            break;

        case 32:     // Locksmith
            Plr->Gossip_SendPOI(1632.68f, 219.4f, 7, 99, 0, "Undercity Locksmith");
            SendQuickMenu(14916);
            break;
        }
    }
};

class UndercityGuardOverseer : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* Plr, bool AutoSend)
        {
            GossipMenu *Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 15321, Plr);
            Menu->AddItem(0, "The auction house"     , 1);
            Menu->AddItem(0, "The bank"              , 2);
            Menu->AddItem(0, "Barber"                , 3);
            Menu->AddItem(0, "The bat handler"       , 4);
            Menu->AddItem(0, "The battlemaster"      , 5);
            Menu->AddItem(0, "The guild master"      , 6);
            Menu->AddItem(0, "The inn"               , 7);
            Menu->AddItem(0, "Locksmith"             , 8);
            Menu->AddItem(0, "The mailbox"           , 9);
            Menu->AddItem(0, "The stable master"     , 10);
            Menu->AddItem(0, "The weapon master"     , 11);
            Menu->AddItem(0, "The zeppelin master"   , 12);
            Menu->AddItem(0, "A class trainer"       , 13);
            Menu->AddItem(0, "A profession trainer"  , 14);
            Menu->SendTo(Plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch(IntId)
            {
            case 0: 
                GossipHello(pObject, Plr,true);
                break;
            case 1:     // The auction house
                {
                    Plr->Gossip_SendPOI(1647.9f, 258.49f, 7, 99, 0, "Undercity Auction House");
                    SendEmptyMenu(14900);
                }break;
            case 2:     // The bank
                {
                    Plr->Gossip_SendPOI(1595.64f, 232.45f, 7, 99, 0, "Undercity Bank");
                    SendEmptyMenu(14901);
                }break;
            case 3:     // Barber
                {
                    Plr->Gossip_SendPOI(1576.20f, 193.14f, 7, 99, 0, "Undercity Barber");
                    SendEmptyMenu(14902);
                }break;
            case 4:     // The bat handler
                {
                    SendEmptyMenu(14903);
                    Plr->Gossip_SendPOI(1565.9f, 271.43f, 7, 99, 0, "Undercity Bat Handler");
                }break;
            case 5:    // The battlemaster
                {
                    SendEmptyMenu(14904);
                    Plr->Gossip_SendPOI(1300.33f, 350.92f, 7, 99, 0, "Battlemasters Undercity");
                }break;
            case 6:     // The guild master
                {
                    SendEmptyMenu(14911);
                    Plr->Gossip_SendPOI(1594.17f, 205.57f, 7, 99, 0, "Undercity Guild Master");
                }break;
            case 7:     // The inn
                {
                    SendEmptyMenu(14913);
                    Plr->Gossip_SendPOI(1639.43f, 220.99f, 7, 99, 0, "Undercity Inn");
                }break;
            case 8:     // Locksmith
                {
                    SendEmptyMenu(14916);
                    Plr->Gossip_SendPOI(1499.68f, 53.28f, 7, 99, 0, "Undercity Locksmith");
                }break;
            case 9:     // The mailbox
                {
                    SendEmptyMenu(14918);
                    Plr->Gossip_SendPOI(1632.68f, 219.4f, 7, 99, 0, "Undercity Mailbox");
                }break;
            case 10:     // The stable master
                {
                    SendEmptyMenu(14924);
                    Plr->Gossip_SendPOI(1634.18f, 226.76f, 7, 99, 0, "Anya Maulray");
                }break;
            case 11:     // The weapon master
                {
                    SendEmptyMenu(14928);
                    Plr->Gossip_SendPOI(1670.31f, 324.66f, 7, 99, 0, "Archibald");
                }break;
            case 12:     // The zeppelin master
                {
                    SendEmptyMenu(14929);
                    Plr->Gossip_SendPOI(2059.0f, 274.86f, 7, 99, 0, "Undercity Zeppelin");
                }break;
            case 13:    // A class trainer
                {
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3542, Plr);
                    Menu->AddItem(0, "Mage"         , 15);
                    Menu->AddItem(0, "Paladin"      , 16);
                    Menu->AddItem(0, "Priest"       , 17);
                    Menu->AddItem(0, "Rogue"        , 18);
                    Menu->AddItem(0, "Warlock"      , 19);
                    Menu->AddItem(0, "Warrior"      , 20);
                    Menu->SendTo(Plr);
                }break;
            case 14:    // A profession trainer
                {
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 3541, Plr);
                    Menu->AddItem(0, "Alchemy"           , 21);
                    Menu->AddItem(0, "Blacksmithing"     , 22);
                    Menu->AddItem(0, "Cooking"           , 23);
                    Menu->AddItem(0, "Enchanting"        , 24);
                    Menu->AddItem(0, "Engineering"       , 25);
                    Menu->AddItem(0, "First Aid"         , 26);
                    Menu->AddItem(0, "Fishing"           , 27);
                    Menu->AddItem(0, "Herbalism"         , 28);
                    Menu->AddItem(0, "Inscription"       , 29);
                    Menu->AddItem(0, "Leatherworking"    , 30);
                    Menu->AddItem(0, "Mining"            , 31);
                    Menu->AddItem(0, "Skinning"          , 32);
                    Menu->AddItem(0, "Tailoring"         , 33);
                    Menu->SendTo(Plr);
                }break;
            case 15: //Mage
                {
                    Plr->Gossip_SendPOI(1781.0f, 53.0f, 7, 99, 0, "Undercity Mage Trainers");
                    SendEmptyMenu(3513);
                }break;
            case 16: //Paladin
                {
                    Plr->Gossip_SendPOI(1298.98f, 316.51f, 7, 99, 0, "Champion Cyssa Downrose");
                    SendEmptyMenu(3521);
                }break;
            case 17: //Priest
                {
                    Plr->Gossip_SendPOI(1758.33f, 401.5f, 7, 99, 0, "Undercity Priest Trainers");
                    SendEmptyMenu(3521);
                }break;
            case 18: //Rogue
                {
                    Plr->Gossip_SendPOI(1418.56f, 65.0f, 7, 99, 0, "Undercity Rogue Trainers");
                    SendEmptyMenu(3526);
                }break;
            case 19: //Warlock
                {
                    Plr->Gossip_SendPOI(1780.92f, 53.16f, 7, 99, 0, "Undercity Warlock Trainers");
                    SendEmptyMenu(3526);
                }break;
            case 20: //Warrior
                {
                    Plr->Gossip_SendPOI(1775.59f, 418.19f, 7, 99, 0, "Undercity Warrior Trainers");
                    SendEmptyMenu(3527);
                }break;
            case 21: //Alchemy
                {
                    Plr->Gossip_SendPOI(1419.82f, 417.19f, 7, 99, 0, "The Apothecarium");
                    SendEmptyMenu(3528);
                }break;
            case 22: //Blacksmithing
                {
                    Plr->Gossip_SendPOI(1696.0f, 285.0f, 7, 99, 0, "Undercity Blacksmithing Trainer");
                    SendEmptyMenu(3529);
                }break;
            case 23: //Cooking
                {
                    Plr->Gossip_SendPOI(1596.34f, 274.68f, 7, 99, 0, "Undercity Cooking Trainer");
                    SendEmptyMenu(3530);
                }break;
            case 24: //Enchanting
                {
                    Plr->Gossip_SendPOI(1488.54f, 280.19f, 7, 99, 0, "Undercity Enchanting Trainer");
                    SendEmptyMenu(3531);
                }break;
            case 25: //Engineering
                {
                    Plr->Gossip_SendPOI(1408.58f, 143.43f, 7, 99, 0, "Undercity Engineering Trainer");
                    SendEmptyMenu(3532);
                }break;
            case 26: //First Aid
                {
                    Plr->Gossip_SendPOI(1519.65f, 167.19f, 7, 99, 0, "Undercity First Aid Trainer");
                    SendEmptyMenu(3533);
                }break;
            case 27: //Fishing
                {
                    Plr->Gossip_SendPOI(1679.9f, 89.0f, 7, 99, 0, "Undercity Fishing Trainer");
                    SendEmptyMenu(3534);
                }break;
            case 28: //Herbalism
                {
                    Plr->Gossip_SendPOI(1558.0f, 349.36f, 7, 99, 0, "Undercity Herbalism Trainer");
                    SendEmptyMenu(3535);
                }break;
            case 29: //Inscription
                {
                    Plr->Gossip_SendPOI(1558.0f, 349.36f, 7, 99, 0, "Undercity Inscription");
                    SendEmptyMenu(14914);
                }break;
            case 30: //Leatherworking
                {
                    Plr->Gossip_SendPOI(1498.76f, 196.43f, 7, 99, 0, "Undercity Leatherworking Trainer");
                    SendEmptyMenu(3536);
                }break;
            case 31: //Mining
                {
                    Plr->Gossip_SendPOI(1642.88f, 335.58f, 7, 99, 0, "Undercity Mining Trainer");
                    SendEmptyMenu(3537);
                }break;
            case 32: //Skinning
                {
                    Plr->Gossip_SendPOI(1498.6f, 196.46f, 7, 99, 0, "Undercity Skinning Trainer");
                    SendEmptyMenu(3538);
                }break;
            case 33: //Tailoring
                {
                    Plr->Gossip_SendPOI(1689.55f, 193.0f, 7, 99, 0, "Undercity Tailoring Trainer");
                    SendEmptyMenu(3539);
                }break;
            }
        }
};

void Lacrimi::SetupUndercity()
{
    RegisterCtrGossipScript(5624,     UndercityGuard);         // Undercity Guardian
    RegisterCtrGossipScript(36213,    UndercityGuardOverseer); // Kor'kron Overseer
}
