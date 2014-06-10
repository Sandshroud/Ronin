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

class SilvermoonGuard : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr, bool AutoSend)
    {
        GossipMenu* Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9316, plr);

        Menu->AddItem(0, "Auction House"        , 1);
        Menu->AddItem(0, "The Bank"             , 2);
        Menu->AddItem(0, "Dragonhawk Master"    , 3);
        Menu->AddItem(0, "Guild Master"         , 4);
        Menu->AddItem(0, "The Inn"              , 5);
        Menu->AddItem(0, "Mailbox"              , 6);
        Menu->AddItem(0, "Stable Master"        , 7);
        Menu->AddItem(0, "Weapon Master"        , 8);
        Menu->AddItem(0, "Battlemaster"         , 9);
        Menu->AddItem(0, "Class Trainer"        , 10);
        Menu->AddItem(0, "Profession Trainer"   , 11);
        Menu->AddItem(0, "Mana Loom"            , 12);
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
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9317, Plr);
            Menu->AddItem(0, "To the west."  , 13);
            Menu->AddItem(0, "To the east."    , 14);
            Menu->SendTo(Plr);
            break;

        case 2:     // The Bank
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9320, Plr);
            Menu->AddItem(0, "The west."  , 15);
            Menu->AddItem(0, "The east."    , 16);
            Menu->SendTo(Plr);
            break;

        case 3:     // Dragonhawk Master
            Plr->Gossip_SendPOI(9378.45f, -7163.94f, 7, 6, 0, "Silvermoon City, Flight Master");
            SendEmptyMenu(9323);
            break;

        case 4:     // Guild Master
            Plr->Gossip_SendPOI(9480.75f, -7345.587f, 7, 6, 0, "Silvermoon City, Guild Master");
            SendEmptyMenu(9324);
            break;

        case 5:     // The Inn
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9325, Plr);
            Menu->AddItem(0, "The Silvermoon City Inn."  , 17);
            Menu->AddItem(0, "The Wayfarer's Rest tavern."    , 18);
            Menu->SendTo(Plr);
            break;

        case 6:     // Mailbox
            Plr->Gossip_SendPOI(9743.078f, -7466.4f, 7, 6, 0, "Silvermoon City, Mailbox");
            SendEmptyMenu(9326);
            break;

        case 7:     // Stable Master
            Plr->Gossip_SendPOI(9904.95f, -7404.31f, 7, 6, 0, "Silvermoon City, Stable Master");
            SendEmptyMenu(9327);
            break;

        case 8:     // Weapon Master
            SendEmptyMenu(9328);
            Plr->Gossip_SendPOI(9841.17f, -7505.13f, 7, 6, 0, "Silvermoon City, Weapon Master");
            break;

        case 9:     // Battlemasters
            Plr->Gossip_SendPOI(9850.74f, -7563.84f, 7, 6, 0, "Silvermoon City, Battlemasters");
            SendEmptyMenu(9329);
            break;

        case 10:    // Class Trainers
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9331, Plr);
                Menu->AddItem(0, "Druid"       , 19);
                Menu->AddItem(0, "Hunter"      , 20);
                Menu->AddItem(0, "Mage"        , 21);
                Menu->AddItem(0, "Paladin"     , 22);
                Menu->AddItem(0, "Priest"      , 23);
                Menu->AddItem(0, "Rogue"       , 24);
                Menu->AddItem(0, "Warlock"     , 25);
                Menu->SendTo(Plr);
            }break;

        case 11:    // Profession Trainers
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 9338, Plr);
                Menu->AddItem(0, "Alchemy"           , 26);
                Menu->AddItem(0, "Blacksmithing"     , 27);
                Menu->AddItem(0, "Cooking"           , 28);
                Menu->AddItem(0, "Enchanting"        , 29);
                Menu->AddItem(0, "Engineering"       , 30);
                Menu->AddItem(0, "First Aid"         , 31);
                Menu->AddItem(0, "Fishing"           , 32);
                Menu->AddItem(0, "Herbalism"         , 33);
                Menu->AddItem(0, "Inscription"       , 34);
                Menu->AddItem(0, "Jewelcrafting"     , 35);
                Menu->AddItem(0, "Leatherworking"    , 36);
                Menu->AddItem(0, "Mining"            , 37);
                Menu->AddItem(0, "Skinning"          , 38);
                Menu->AddItem(0, "Tailoring"         , 39);
                Menu->SendTo(Plr);
            }break;

        case 12: //Mana Loom
            Plr->Gossip_SendPOI(9751.013f, -7074.85f, 7, 6, 0, "Silvermoon City, Mana Loom");
            SendQuickMenu(10502);
            break;

        case 13: //To the west - Auction House no. 1
            Plr->Gossip_SendPOI(9649.429f, -7134.027f, 7, 6, 0, "Silvermoon City, Auction House");
            SendQuickMenu(9318);
            break;

        case 14: //To the east - Auction House no. 2
            Plr->Gossip_SendPOI(9682.864f, -7515.786f, 7, 6, 0, "Silvermoon City, Auction House");
            SendQuickMenu(9319);
            break;

        case 15:     // The bank - The west.
            Plr->Gossip_SendPOI(9522.104f, -7208.878f, 7, 6, 0, "Silvermoon City, Bank");
            SendQuickMenu(9321);
            break;

        case 16:     // The bank - The east.
            Plr->Gossip_SendPOI(9791.07f, -7488.041f, 7, 6, 0, "Silvermoon City, Bank");
            SendQuickMenu(9322);
            break;

        case 17: //The Silvermoon City Inn
            Plr->Gossip_SendPOI(9677.113f, -7367.575f, 7, 6, 0, "Silvermoon City, Inn");
            SendQuickMenu(9325);
            break;

        case 18: //The Wayfarer's Rest tavern
            Plr->Gossip_SendPOI(9562.813f, -7218.63f, 7, 6, 0, "Silvermoon City, Inn");
            SendQuickMenu(9603);
            break;

        case 19: //Druid
            Plr->Gossip_SendPOI(9700.55f, -7262.57f, 7, 6, 0, "Silvermoon City, Druid Trainer");
            SendQuickMenu(9330);
            break;

        case 20: //Hunter
            Plr->Gossip_SendPOI(9930.568f, -7412.115f, 7, 6, 0, "Silvermoon City, Hunter Trainer");
            SendQuickMenu(9332);
            break;

        case 21: //Mage
            Plr->Gossip_SendPOI(9996.914f, -7104.803f, 7, 6, 0, "Silvermoon City, Mage Trainer");
            SendQuickMenu(9333);
            break;

        case 22: //Paladin
            Plr->Gossip_SendPOI(9850.22f, -7516.93f, 7, 6, 0, "Silvermoon City, Paladin Trainer");
            SendQuickMenu(9334);
            break;

        case 23: //Priest
            Plr->Gossip_SendPOI(9935.37f, -7131.14f, 7, 6, 0, "Silvermoon City, Priest Trainer");
            SendQuickMenu(9335);
            break;

        case 24: //Rogue
            Plr->Gossip_SendPOI(9739.88f, -7374.33f, 7, 6, 0, "Silvermoon City, Rogue Trainer");
            SendQuickMenu(9336);
            break;

        case 25: //Warlock
            Plr->Gossip_SendPOI(9803.052f, -7316.967f, 7, 6, 0, "Silvermoon City, Warlock Trainer");
            SendQuickMenu(9337);
            break;

        case 26: //Alchemy
            Plr->Gossip_SendPOI(10000.9f, -7216.63f, 7, 6, 0, "Silvermoon City, Alchemy");
            SendQuickMenu(9339);
            break;

        case 27: //Blacksmithing
            Plr->Gossip_SendPOI(9841.43f, -7361.53f, 7, 6, 0, "Silvermoon City, Blacksmithing");
            SendQuickMenu(9340);
            break;

        case 28: //Cooking
            Plr->Gossip_SendPOI(9577.26f, -7243.6f, 7, 6, 0, "Silvermoon City, Cooking");
            SendQuickMenu(9624);
            break;

        case 29: //Enchanting
            Plr->Gossip_SendPOI(9962.57f, -7246.18f, 7, 6, 0, "Silvermoon City, Enchanting");
            SendQuickMenu(9341);
            break;

        case 30: //Engineering
            Plr->Gossip_SendPOI(9808.85f, -7287.31f, 7, 6, 0, "Silvermoon City, Engineering");
            SendQuickMenu(9342);
            break;

        case 31: //First Aid
            Plr->Gossip_SendPOI(9588.61f, -7337.526f, 7, 6, 0, "Silvermoon City, First Aid");
            SendQuickMenu(9343);
            break;

        case 32: //Fishing
            Plr->Gossip_SendPOI(9601.97f, -7332.34f, 7, 6, 0, "Silvermoon City, Fishing");
            SendQuickMenu(9344);
            break;

        case 33: //Herbalism
            Plr->Gossip_SendPOI(9996.96f, -7208.39f, 7, 6, 0, "Silvermoon City, Herbalism");
            SendQuickMenu(9345);
            break;

        case 34: //Inscription
            Plr->Gossip_SendPOI(9957.12f, -7242.85f, 7, 6, 0, "Silvermoon City, Inscription");
            SendQuickMenu(13893);
            break;

        case 35: //Jewelcrafting
            Plr->Gossip_SendPOI(9552.8f, -7502.12f, 7, 6, 0, "Silvermoon City, Jewelcrafting");
            SendQuickMenu(9346);
            break;

        case 36: //Leatherworking
            Plr->Gossip_SendPOI(9502.486f, -7425.51f, 7, 6, 0, "Silvermoon City, Leatherworking");
            SendQuickMenu(9347);
            break;

        case 37: //Mining
            Plr->Gossip_SendPOI(9813.73f, -7360.19f, 7, 6, 0, "Silvermoon City, Mining");
            SendQuickMenu(9348);
            break;

        case 38: //Skinning
            Plr->Gossip_SendPOI(9513.37f, -7429.4f, 7, 6, 0, "Silvermoon City, Skinning");
            SendQuickMenu(9349);
            break;

        case 39: //Tailoring
            Plr->Gossip_SendPOI(9727.56f, -7086.65f, 7, 6, 0, "Silvermoon City, Tailoring");
            SendQuickMenu(9350);
            break;
        }
    }
};

class BloodKnightStillblade : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(BloodKnightStillblade);
    BloodKnightStillblade(Creature *pCreature) : CreatureAIScript(pCreature) {}

    void Reset()
    {
        if(active)
        {
            _unit->SetPosition(_unit->GetSpawnX(), _unit->GetSpawnY(), _unit->GetSpawnZ(), _unit->GetSpawnO());
            lifetimer = 120000;
            timediff = UNIXTIME;
            _unit->SetUInt32Value(UNIT_NPC_FLAGS, 1);
            _unit->SetStandState(STANDSTATE_DEAD);
            _unit->setDeathState(CORPSE);
            _unit->GetAIInterface()->setCanMove(false);
        }
    }

    void OnLoad()
    {
        if(_unit->m_spawn)
        {
            active = true;
            Reset();
        }
    }

    void AIUpdate(MapManagerScript* MMScript, uint32 p_time)
    {
        if(active)
        {
            uint32 diff = UNIXTIME - timediff;
            if(_unit->GetStandState() == STANDSTATE_STAND)
            {
                if (lifetimer < diff)
                    Reset();
            }
        }
    }

protected:
    Creature* BKStillblade;
    uint32 lifetimer;
    uint32 timediff;
    bool active;
};

void Lacrimi::SetupSilvermoon()
{
    RegisterCtrGossipScript(16222,    SilvermoonGuard);    //Silvermoon City Guardian
    RegisterCtrAIScript(17768,        BloodKnightStillblade);
}
