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

#define SunkenTreauseWPCount 15

static Coords SunkenTreauseWP[SunkenTreauseWPCount] =
{
    {-2078.054443f, -2091.207764f, 9.526212f, 0},
    {-2076.626465f, -2109.960449f, 14.320494f, 0},
    {-2072.851074f, -2123.574219f, 18.482662f, 0},
    {-2063.878906f, -2132.617920f, 21.430487f, 0},
    {-2051.495117f, -2145.205811f, 20.500065f, 0},
    {-2044.748291f, -2152.411377f, 20.158432f, 0},
    {-2044.748291f, -2152.411377f, 20.158432f, 0},
    {-2044.748291f, -2152.411377f, 20.158432f, 0},
    {-2044.748291f, -2152.411377f, 20.158432f, 0},
    {-2044.748291f, -2152.411377f, 20.158432f, 0},
    {-2051.495117f, -2145.205811f, 20.500065f, 0},
    {-2063.878906f, -2132.617920f, 21.430487f, 0},
    {-2072.851074f, -2123.574219f, 18.482662f, 0},
    {-2076.626465f, -2109.960449f, 14.320494f, 0},
    {-2078.054443f, -2091.207764f, 9.526212f, 0}
};

class SunkenTreasure : public QuestScript 
{ 
public:
    void OnQuestStart( Player *mTarget, QuestLogEntry * qLogEntry)
    {
        if( mTarget == NULL || mTarget->GetMapMgr() == NULL || mTarget->GetMapMgr()->GetMapScript() == NULL )
            return;
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature *creat = mTarget->GetMapMgr()->GetMapScript()->FindClosestCreature(2768, SSX, SSY, SSZ);
        if(creat == NULL)
            return;
        creat->m_escorter = mTarget;   
        creat->GetAIInterface()->setMoveType(11);
        creat->GetAIInterface()->StopMovement(3000);
        creat->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Defens Me!");
        creat->SetUInt32Value(UNIT_NPC_FLAGS, 0);

        CreateCustomWaypointMap(creat);
        for(uint32 i = 0; i < SunkenTreauseWPCount; i++)
        {
            WaypointCreate(creat, SunkenTreauseWP[i].mX, SunkenTreauseWP[i].mY, SunkenTreauseWP[i].mZ, 0.0f, 0, 256, 4049);
        }
        EnableWaypoints(creat);
    }
};

class Professor_Phizzlethorpe : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Professor_Phizzlethorpe(c); }
    Professor_Phizzlethorpe(Creature *pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32 iWaypointId, bool bForwards)
    {
        if(iWaypointId == 15)
        {
            _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thanks, I found the fact that, it searched");
            _unit->Despawn(5000,1000);
            DeleteWaypoints(_unit);
            if(_unit->m_escorter == NULL)
                return;
            Player *plr = _unit->m_escorter;
            _unit->m_escorter = NULLPLR;
            plr->GetQuestLogForEntry(665)->SendQuestComplete();
        }
    }
};

void Lacrimi::SetupArathiHighlands()
{
    RegisterCtrAIScript(2768, Professor_Phizzlethorpe);
    RegisterQuestScript(665, SunkenTreasure);
}
