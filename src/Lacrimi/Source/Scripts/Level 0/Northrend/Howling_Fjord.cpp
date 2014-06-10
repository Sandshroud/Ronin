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

// War is Hell
bool WarIsHell(uint32 i, Spell *pSpell)
{
    if( !pSpell->u_caster->IsPlayer() )
        return true;

    Player *plr = TO_PLAYER(pSpell->u_caster);
    if( plr == NULL )
        return true;

    Creature *target = TO_CREATURE(plr->GetMapMgr()->GetMapScript()->FindClosestCreature( 24008, plr->GetPositionX(), plr->GetPositionY() , plr->GetPositionZ() ));
    if( target == NULL )
        return true;

    QuestLogEntry *qle = plr->GetQuestLogForEntry(11270);

    if( qle == NULL || qle->GetQuest() == NULL )
        return true;

    GameObject *obj = NULLGOB;

    if( qle && qle->GetMobCount(0) < qle->GetQuest()->required_mobcount[0] )
    {
        qle->SetMobCount( 0, qle->GetMobCount( 0 ) + 1 );
        qle->SendUpdateAddKill( 0 );
        
        obj = CreateAndLoadGameObject(plr->GetMapMgr(), 183816, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), 1, true);
        GameobjectDelete(obj, 1*30*1000);
    }
    target->Despawn( 2000, 60*1000 );
    plr->UpdateNearbyGameObjects();
    qle->UpdatePlayerFields();
    return true;
}

// A Lesson in Fear
bool PlantForsakenBanner(uint32 i, Spell *pSpell)
{
    if(pSpell->p_caster == NULL)
        return true;

    Player *pPlayer = pSpell->p_caster;
    if( pPlayer == NULL )
        return true;

    QuestLogEntry *pQuest = pPlayer->GetQuestLogForEntry(11282);
    if( pQuest == NULL || pQuest->GetQuest() == NULL )
        return true;

    Creature *target = TO_CREATURE(pSpell->GetUnitTarget());
    if (target == NULL)
        return true;

    uint32 cit = target->GetEntry();
    switch(cit)
    {
    case 24161:
        {
            if( pQuest->GetMobCount( 0 ) < pQuest->GetQuest()->required_mobcount[ 0 ] )
            {
                uint32 newcount = pQuest->GetMobCount( 0 ) + 1;
                pQuest->SetMobCount( 0, newcount );
                pQuest->SendUpdateAddKill( 0 );
                pQuest->UpdatePlayerFields();
                target->Despawn(0, 3*60*1000);
            }   
        }
        break;
    case 24016:
        {
            if( pQuest->GetMobCount( 0 ) < pQuest->GetQuest()->required_mobcount[ 1 ] )
            {
                uint32 newcount = pQuest->GetMobCount( 0 ) + 1;
                pQuest->SetMobCount( 0, newcount );
                pQuest->SendUpdateAddKill( 0 );
                pQuest->UpdatePlayerFields();
                target->Despawn(0, 3*60*1000);
            }   
        }
        break;
    case 24162:
        {
            if( pQuest->GetMobCount( 0 ) < pQuest->GetQuest()->required_mobcount[ 2 ] )
            {
                uint32 newcount = pQuest->GetMobCount( 0 ) + 1;
                pQuest->SetMobCount( 0, newcount );
                pQuest->SendUpdateAddKill( 0 );
                pQuest->UpdatePlayerFields();
                target->Despawn(0, 3*60*1000);
            }   
        }
        break;
    }
    return true;
}

void Lacrimi::SetupHowlingFjord()
{
    RegisterDummySpell(42793, WarIsHell); 
    RegisterDummySpell(43178, PlantForsakenBanner);
}
