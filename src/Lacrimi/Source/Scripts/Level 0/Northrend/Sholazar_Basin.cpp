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

bool ChickenNet( uint32 i, Spell *pSpell )
{
    if( !pSpell->p_caster != NULL )
        return false;

    Player *pPlayer = pSpell->p_caster;
    QuestLogEntry *qle = pPlayer->GetQuestLogForEntry( 12532 );
    if( qle == NULL )
        qle = pPlayer->GetQuestLogForEntry( 12702 );

    if( qle == NULL )
        return false;

    if( !pSpell->GetUnitTarget()->IsCreature() )
        return false;

    Creature *Chicken = TO_CREATURE( pSpell->GetUnitTarget() );
    if( Chicken != NULL && Chicken->GetEntry() == 28161 )
    {
        Chicken->Despawn( 5000, 360000 );
        pPlayer->CastSpell( pSpell->u_caster, dbcSpell.LookupEntry( 51037 ), true );
        return true;
    }
    return false;
}

void Lacrimi::SetupSholazarBasin()
{
    RegisterDummySpell(51959, ChickenNet);
}
