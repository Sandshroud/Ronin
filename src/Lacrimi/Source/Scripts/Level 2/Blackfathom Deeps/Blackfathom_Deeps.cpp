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
class SCRIPT_DECL LadySarevessAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(LadySarevessAI, MoonScriptCreatureAI);
    LadySarevessAI(Creature *pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddEmote(Event_OnCombatStart, "You should not be here! Slay them!", Text_Yell, 5799);
        AddSpell(8435, Target_Current, 10, 2, 0);   // Forked Lightning
        AddSpell(865, Target_Self, 15, 0, 25);      // Frost Nova
        AddSpell(246, Target_Current, 15, 0, 10);   // Slow
    }
};

class SCRIPT_DECL BaronAquanisAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(BaronAquanisAI, MoonScriptCreatureAI);
    BaronAquanisAI(Creature *pCreature) : MoonScriptCreatureAI(pCreature) 
    {
        // Frostbolt
        AddSpell(15043, Target_Current, 20, 3, 0);
        AggroNearestPlayer();
        SetDespawnWhenInactive(true);
    }
};

class SCRIPT_DECL TwilightLordKelrisAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(TwilightLordKelrisAI, MoonScriptCreatureAI);
    TwilightLordKelrisAI(Creature *pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddEmote(Event_OnCombatStart, "Who dares disturb my meditation?!", Text_Yell, 5802);
        AddEmote(Event_OnTargetDied, "Dust to dust.", Text_Yell, 5803);

        // Sleep
        AddSpell(8399, Target_RandomPlayer, 12, 1.3f, 0, 0, 0, false, "Sleep...", Text_Yell, 5804);

        // Mind Blast
        AddSpell(15587, Target_Current, 16, 1.5f, 0);
    }
};

class SCRIPT_DECL MorriduneGossip : public GossipScript
{
public:
    void GossipHello(Object *pObject, Player * plr, bool AutoSend)
    {
        GossipMenu *Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 7247, plr);
        
        if(plr->GetTeam() == 0)
            Menu->AddItem( 0, "Please Teleport me to Darnassus.", 1);
        Menu->AddItem( 0, "I wish to leave this horrible place", 2);
        Menu->SendTo(plr);
    }
    
    void GossipSelectOption(Object *pObject, Player * plr, uint32 Id, uint32 IntId, const char * Code)
    {
        switch (IntId)
        {
        case 0: GossipHello(pObject, plr, true); break;
        case 1: //Port to Darnassus
            {
                plr->Gossip_Complete();
                plr->SafeTeleport(1, 0, 9951.52f, 2280.32f, 1341.39f, 0);
            }break;
        case 2: //Port to entrance
            {
                plr->Gossip_Complete();
                plr->SafeTeleport(1, 0, 4247.74f, 745.879f, -24.2967f, 4.36996f);
            }break;
        }
    }
};

class SCRIPT_DECL FathomStone : public GameObjectAIScript
{
public:
    FathomStone(GameObject * goinstance) : GameObjectAIScript(goinstance){ BaronAquanis = false; }
    static GameObjectAIScript* Create(GameObject*  GO) { return new FathomStone(GO); }

    void OnActivate(Player * pPlayer)
    {
        if(pPlayer->GetTeam() == 1 && !BaronAquanis) // Horde
        {
            // Spawn Baron Aquanis
            _gameobject->GetMapMgr()->GetMapScript()->SpawnCreature(12876, -782.021f, -63.5876f, -45.0935f, -2.44346f);
            BaronAquanis = true;
        }
    }

protected:
    bool BaronAquanis;
};

void Lacrimi::SetupBlackfathomDeeps()
{
    RegisterCtrAIScript(4831, LadySarevessAI);
    RegisterCtrAIScript(12876, BaronAquanisAI);
    RegisterCtrAIScript(4832, TwilightLordKelrisAI);
    RegisterCtrGossipScript(6729, MorriduneGossip);
    RegisterGOAIScript(177964, FathomStone);
}
