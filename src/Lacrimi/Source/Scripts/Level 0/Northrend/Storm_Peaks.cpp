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

// The Gifts of Loken
class LokensFury : public GameObjectAIScript
{
public:
    ADD_GAMEOBJECT_FACTORY_FUNCTION(LokensFury);
    LokensFury(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

    void OnActivate(Player* pPlayer)
    {
        if( GetQuest( pPlayer, 12965) )
            KillMobForQuest( pPlayer, 12965, 0 );
    };
};

class LokensPower : public GameObjectAIScript
{
public:
    ADD_GAMEOBJECT_FACTORY_FUNCTION(LokensPower);
    LokensPower(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

    void OnActivate(Player* pPlayer)
    {
        if ( GetQuest( pPlayer, 12965) )
            KillMobForQuest( pPlayer, 12965, 1 );
    };
};

class LokensFavor : public GameObjectAIScript
{
public:
    ADD_GAMEOBJECT_FACTORY_FUNCTION(LokensFavor);
    LokensFavor(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

    void OnActivate(Player* pPlayer)
    {
        if ( GetQuest( pPlayer, 12965) )
            KillMobForQuest( pPlayer, 12965, 2 );
    };
};

#define GOSSIP_SCOUTMENU1 "Are you okay? I've come to take you back to Frosthold if you can stand."
#define GOSSIP_SCOUTMENU2 "I'm sorry that I didn't get here sooner. What happened?"
#define GOSSIP_SCOUTMENU3 "I'll go get some help. Hang in there."

class SCRIPT_DECL MissingScout_Gossip : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr, bool AutoSend)
    {
        GossipMenu *Menu;
        objmgr.CreateGossipMenuForPlayer( &Menu, pObject->GetGUID(), 13611, plr );
        if( plr->HasQuest(12864) )
            Menu->AddItem( 0, GOSSIP_SCOUTMENU1, 1 );

        if( AutoSend )
            Menu->SendTo(plr);
    }

    void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char * Code)
    {
        Creature* pCreature = (pObject->GetTypeId()==TYPEID_UNIT) ? (TO_CREATURE(pObject)) : NULL;
        if(pCreature == NULL)
            return;

        GossipMenu *Menu;
        switch(IntId)
        {
        case 1:
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13612, plr);
                Menu->AddItem( 0, GOSSIP_SCOUTMENU2, 2);
                Menu->SendTo(plr);
            }break;
        case 2:
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13613, plr);
                Menu->AddItem( 0, GOSSIP_SCOUTMENU3, 3);
                Menu->SendTo(plr);
            }break;
        case 3:
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 13614, plr);
                Menu->SendTo(plr);
                QuestLogEntry *qle = plr->GetQuestLogForEntry(12864);
                if(qle == NULL || qle->GetMobCount(0) != 0)
                    return;

                qle->SetMobCount(0, 1);
                qle->SendUpdateAddKill(0);
                qle->UpdatePlayerFields();
            }break;
        }
    }
};

void Lacrimi::SetupStormPeaks()
{
    // The Gifts of Loken
    RegisterGOAIScript(192120, LokensFury);
    RegisterGOAIScript(192121, LokensPower);
    RegisterGOAIScript(192122, LokensFavor);
    RegisterGOGossipScript(29811, MissingScout_Gossip);
}
