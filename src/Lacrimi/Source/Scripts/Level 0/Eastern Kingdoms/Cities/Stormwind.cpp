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

class StormwindCityGuard : public GossipScript
{
public:
    void GossipHello(Object *pObject, Player *Plr, bool AutoSend)
    {
        GossipMenu *Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 933, Plr);
        Menu->AddItem(0, "Auction House"        , 1);
        Menu->AddItem(0, "Bank of Stormwind"    , 2);
        Menu->AddItem(0, "Stormwind Harbor"     , 3);
        Menu->AddItem(0, "Deeprun Tram"         , 4);
        Menu->AddItem(0, "The Inn"              , 5);
        Menu->AddItem(0, "Gryphon Master"       , 6);
        Menu->AddItem(0, "Guild Master"         , 7);
        Menu->AddItem(0, "Mailbox"              , 8);
        Menu->AddItem(0, "Locksmith"            , 38);
        Menu->AddItem(0, "Stable Master"        , 9);
        Menu->AddItem(0, "Weapons Trainer"      , 10);
        Menu->AddItem(0, "Officers' Lounge"     , 11);
        Menu->AddItem(0, "Battlemaster"         , 12);
        Menu->AddItem(0, "Barber"               , 13);
        Menu->AddItem(0, "Lexicon of Power"     , 39);
        Menu->AddItem(0, "Class Trainer"        , 14);
        Menu->AddItem(0, "Profession Trainer"   , 15);
        Menu->SendTo(Plr);
    }

    void GossipSelectOption(Object *pObject, Player *Plr, uint32 Id, uint32 IntId, const char * Code)
    {
        GossipMenu * Menu;
        switch(IntId)
        {
        case 0: GossipHello(pObject, Plr, true); break;
        case 1:     // Auction House
            {
                Plr->Gossip_SendPOI(-8811.46f, 667.46f, 7, 99, 0, "Stormwind Auction House");
                SendEmptyMenu(3834);
            }break;
        case 2:     // Bank of Stormwind
            {
                Plr->Gossip_SendPOI(-8916.87f, 622.87f, 7, 99, 0, "Stormwind Bank");
                SendEmptyMenu(764);
            }break;
        case 3:     // Stormwind Harbor
            {
                Plr->Gossip_SendPOI(-8634.77f, 949.64f, 7, 99, 0, "Stormwind Harbor");
                SendEmptyMenu(13439);
            }break;
        case 4:     // Deeprun Tram
            {
                Plr->Gossip_SendPOI(-8378.88f, 554.23f, 7, 99, 0, "The Deeprun Tram");
                SendEmptyMenu(3813);
            }break;
        case 5:     // The Inn
            {
                Plr->Gossip_SendPOI(-8869.0f, 675.4f, 7, 99, 0, "The Gilded Rose");
                SendEmptyMenu(3860);
            }break;
        case 6:     // Gryphon Master
            {
                Plr->Gossip_SendPOI(-8837.0f, 493.5f, 7, 99, 0, "Stormwind Gryphon Master");
                SendEmptyMenu(879);
            }break;
        case 7:     // Guild Master
            {
                Plr->Gossip_SendPOI(-8894.0f, 611.2f, 7, 99, 0, "Stormwind Vistor`s Center");
                SendEmptyMenu(882);
            }break;
        case 8:     // Mailbox
            {
                Plr->Gossip_SendPOI(-8876.48f, 649.18f, 7, 99, 0, "Stormwind Mailbox");
                SendEmptyMenu(3861);
            }break;
        case 9:     // Stable Master
            {
                Plr->Gossip_SendPOI(-8433.0f, 554.7f, 7, 99, 0, "Jenova Stoneshield");
                SendEmptyMenu(5984);
            }break;
        case 10:    // Weapons Master
            {
                Plr->Gossip_SendPOI(-8797.0f, 612.8f, 7, 99, 0, "Woo Ping");
                SendEmptyMenu(4516);
            }break;
        case 11:    // Officers' Lounge
            {
                Plr->Gossip_SendPOI(-8759.92f, 399.69f, 7, 99, 0, "Champions` Hall");
                SendEmptyMenu(7047);
            }break;
        case 12:    // Battlemaster
            {
                Plr->Gossip_SendPOI(-8393.62f, 274.21f, 7, 99, 0, "Battlemasters Stormwind");
                SendEmptyMenu(10218);
            }break;
        case 13:    // Barber
            {
                Plr->Gossip_SendPOI(-8743.15f, 660.36f, 7, 99, 0, "Stormwind Barber");
                SendEmptyMenu(13882);
            }break;
        case 14:    // Class Trainers
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 898, Plr);
                Menu->AddItem( 0, "Druid"   , 16);
                Menu->AddItem( 0, "Hunter"  , 17);
                Menu->AddItem( 0, "Mage"    , 18);
                Menu->AddItem( 0, "Paladin" , 19);
                Menu->AddItem( 0, "Priest"  , 20);
                Menu->AddItem( 0, "Rogue"   , 21);
                Menu->AddItem( 0, "Shaman"  , 22);
                Menu->AddItem( 0, "Warlock" , 23);
                Menu->AddItem( 0, "Warrior" , 24);
                Menu->SendTo(Plr);
            }break;
        case 15:    // Profession Trainers
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 918, Plr);
                Menu->AddItem( 0, "Alchemy"         , 25);
                Menu->AddItem( 0, "Blacksmithing"   , 26);
                Menu->AddItem( 0, "Cooking"         , 27);
                Menu->AddItem( 0, "Enchanting"      , 28);
                Menu->AddItem( 0, "Engineering"     , 29);
                Menu->AddItem( 0, "First Aid"       , 30);
                Menu->AddItem( 0, "Fishing"         , 31);
                Menu->AddItem( 0, "Herbalism"       , 32);
                Menu->AddItem( 0, "Inscription"     , 33);
                Menu->AddItem( 0, "Leatherworking"  , 34);
                Menu->AddItem( 0, "Mining"          , 35);
                Menu->AddItem( 0, "Skinning"        , 36);
                Menu->AddItem( 0, "Tailoring"       , 37);
                Menu->SendTo(Plr);
            }break;
        case 16:    // Druid
            {
                Plr->Gossip_SendPOI(-8751.0f, 1124.5f, 7, 99, 0, "The Park");
                SendEmptyMenu(902);
            }break;
        case 17:    // Hunter
            {
                Plr->Gossip_SendPOI(-8413.0f, 541.5f, 7, 99, 0, "Hunter Lodge");
                SendEmptyMenu(905);
            }break;
        case 18:    // Mage
            {
                Plr->Gossip_SendPOI(-9012.0f, 867.6f, 7, 99, 0, "Wizard`s Sanctum");
                SendEmptyMenu(899);
            }break;
        case 19:    // Paladin
            {
                Plr->Gossip_SendPOI(-8577.0f, 881.7f, 7, 99, 0, "Cathedral Of Light");
                SendEmptyMenu(904);
            }break;
        case 20:    // Priest
            {
                Plr->Gossip_SendPOI(-8512.0f, 862.4f, 7, 99, 0, "Cathedral Of Light");
                SendEmptyMenu(903);
            }break;
        case 21:    // Rogue
            {
                Plr->Gossip_SendPOI(-8753.0f, 367.8f, 7, 99, 0, "Stormwind - Rogue House");
                SendEmptyMenu(900);
            }break;
        case 22:    // Shaman
            {
                Plr->Gossip_SendPOI(-9031.54f, 549.87f, 7, 99, 0, "Farseer Umbrua");
                SendEmptyMenu(10106);
            }break;
        case 23:    // Warlock
            {
                Plr->Gossip_SendPOI(-8948.91f, 998.35f, 7, 99, 0, "The Slaughtered Lamb");
                SendEmptyMenu(906);
            }break;
        case 24:    // Warrior
            {
                Plr->Gossip_SendPOI(-8714.14f, 334.96f, 7, 99, 0, "Stormwind Barracks");
                SendEmptyMenu(901);
            }break;
        case 25:    // Alchemy
            {
                Plr->Gossip_SendPOI(-8988.0f, 759.60f, 7, 99, 0, "Alchemy Needs");
                SendEmptyMenu(919);
            }break;
        case 26:    // Blacksmithing
            {
                Plr->Gossip_SendPOI(-8424.0f, 616.9f, 7, 99, 0, "Therum Deepforge");
                SendEmptyMenu(920);
            }break;
        case 27:    // Cooking
            {
                Plr->Gossip_SendPOI(-8611.0f, 364.6f, 7, 99, 0, "Pig and Whistle Tavern");
                SendEmptyMenu(921);
            }break;
        case 28:    // Enchanting
            {
                Plr->Gossip_SendPOI(-8858.0f, 803.7f, 7, 99, 0, "Lucan Cordell");
                SendEmptyMenu(941);
            }break;
        case 29:    // Engineering
            {
                Plr->Gossip_SendPOI(-8347.0f, 644.1f, 7, 99, 0, "Lilliam Sparkspindle");
                SendEmptyMenu(922);
            }break;
        case 30:    // First Aid
            {
                Plr->Gossip_SendPOI(-8513.0f, 801.8f, 7, 99, 0, "Shaina Fuller");
                SendEmptyMenu(923);
            }break;
        case 31:    // Fishing
            {
                Plr->Gossip_SendPOI(-8803.0f, 767.5f, 7, 99, 0, "Arnold Leland");
                SendEmptyMenu(940);
            }break;
        case 32:    // Herbalism
            {
                Plr->Gossip_SendPOI(-8967.0f, 779.5f, 7, 99, 0, "Alchemy Needs");
                SendEmptyMenu(924);
            }break;
        case 33:    // Inscription
            {
                Plr->Gossip_SendPOI(-8853.33f, 857.66f, 7, 99, 0, "Stormwind Inscription");
                SendEmptyMenu(13881);
            }break;
        case 34:    // Leatherworking
            {
                Plr->Gossip_SendPOI(-8726.0f, 477.4f, 7, 99, 0, "The Protective Hide");
                SendEmptyMenu(925);
            }break;
        case 35:    // Mining
            {
                Plr->Gossip_SendPOI(-8434.0f, 692.8f, 7, 99, 0, "Gelman Stonehand");
                SendEmptyMenu(927);
            }break;
        case 36:    // Skinning
            {
                Plr->Gossip_SendPOI(-8716.0f, 469.4f, 7, 99, 0, "The Protective Hide");
                SendEmptyMenu(928);
            }break;
        case 37:    // Tailoring
            {
                Plr->Gossip_SendPOI(-8938.0f, 800.7f, 7, 99, 0, "Duncan`s Textiles");
                SendEmptyMenu(929);
            }break;
        case 38:    // Locksmith
            {
                Plr->Gossip_SendPOI(-8425.0f, 627.621f, 7, 99, 0, "Stormwind Locksmith");
                SendEmptyMenu(14249);
            }break;
        case 39:    // Lexicon of Power
            {
                Plr->Gossip_SendPOI(-8851.0f, 856.599f, 7, 99, 0, "Stormwind Inscription");
                SendEmptyMenu(14174);
            }break;
        }
    }
};

#define GOSSIP_ARCHMAGE_MALIN    "Can you send me to Theramore? I have an urgent message for Lady Jaina from Highlord Bolvar."

class ArchmageMalin_Gossip : public GossipScript
{
public:
    void GossipHello(Object *pObject, Player *plr, bool AutoSend)
    {
        GossipMenu *Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 11469, plr);

        if(plr->GetQuestLogForEntry(11223))
            Menu->AddItem( 0, GOSSIP_ARCHMAGE_MALIN, 1);

        Menu->SendTo(plr);
    }

    void GossipSelectOption(Object *pObject, Player *plr, uint32 Id, uint32 IntId, const char * Code)
    {
        if(IntId == 1)
        {
            plr->Gossip_Complete();
            pCreature->CastSpell(plr, 42711, true);
        }
    }

protected:
    Creature *pCreature;
};

void Lacrimi::SetupStormwind()
{
    RegisterCtrGossipScript(68,     StormwindCityGuard);    // Stormwind City Guard
    RegisterCtrGossipScript(1976,   StormwindCityGuard);    // Stormwind City Patroller
    RegisterCtrGossipScript(29712,  StormwindCityGuard);    // Stormwind Harbor Guard
    RegisterCtrGossipScript(2708,   ArchmageMalin_Gossip);  // Archmage Malin
}
