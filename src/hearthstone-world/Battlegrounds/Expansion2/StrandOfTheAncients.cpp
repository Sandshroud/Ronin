/***
 * Demonstrike Core
 */

#include "StdAfx.h"

#define ROUND_LENGTH 600 // This takes seconds.

const float m_gateSigilsLocations[5][5] = {
    {192687, 1414.054f, 106.72f, 41.442f, 5.441f},
    {192685, 1060.63f, -107.8f, 94.7f, 0.034f},
    {192689, 1433.383f, -216.4f, 43.642f, 0.9736f},
    {192690, 1230.75f, -210.724f, 67.611f, 0.5023f},
    {192691, 1217.8f, 79.532f, 66.58f, 5.745f},
};

const float m_gatesLocations[5][5] = {
    {190722, 1411.57f, 108.163f, 28.692f, 5.441f},
    {190727, 1055.452f, -108.1f, 82.134f, 0.034f},
    {190724, 1431.3413f, -219.437f, 30.893f, 0.9736f},
    {190726, 1227.667f, -212.555f, 55.372f, 0.5023f},
    {190723, 1214.681f, 81.21f,53.413f, 5.745f},
};

//Location : Cannons
const float m_cannonsLocations[10][4] = {
    {1436.429f, 110.05f, 41.407f, 5.4f},
    {1404.9023f, 84.758f, 41.183f, 5.46f},
    {1068.693f, -86.951f, 93.81f, 0.02f},
    {1068.83f, -127.56f, 96.45f, 0.0912f},
    {1422.115f, -196.433f, 42.1825f, 1.0222f},
    {1454.887f, -220.454f, 41.956f, 0.9627f},
    {1232.345f, -187.517f, 66.945f, 0.45f},
    {1236.213f, 92.287f, 64.965f, 5.751f},
    {1215.11f, 57.772f, 64.739f, 5.78f},
    {1249.56f, -223.966f, 66.72f, 0.52f},
};

//Location : Transporters ( Teleporters.)
const float m_gateTransportersLocations[5][4] = {
    {1394.0444f, 72.586f, 31.0535f, 0.0f},
    {1065.0f, -89.7f, 81.08f, 0.0f},
    {1467.95f, -225.67f, 30.9f, 0.0f},
    {1255.93f, -233.382f, 56.44f, 0.0f},
    {1215.679f, 47.47f, 54.281f, 0.0f},
};

// Defender Trans : (Teleporters.)
const float sotaTransporters[5][4] = {
    { 1394.0444f, 72.586f, 31.0535f, 0.0f },
    { 1065.0f, -89.7f, 81.08f, 0.0f },
    { 1467.95f, -225.67f, 30.9f, 0.0f },
    { 1255.93f, -233.382f, 56.44f, 0.0f },
    { 1215.679f, 47.47f, 54.281f, 0.0f },
};

// Defender Trans Dest: Locations you end up.
const float sotaTransporterDestination[5][4] = {
    { 1388.94f, 103.067f, 34.49f, 5.4571f },
    { 1043.69f, -87.95f, 87.12f, 0.003f },
    { 1441.0411f, -240.974f, 35.264f, 0.949f },
    { 1228.342f, -235.234f, 60.03f, 0.4584f },
    { 1193.857f, 69.9f, 58.046f, 5.7245f },
};

const float sotaRepop[2][4] = {
    { 1600.0f, 58.3f, 11.0f, 2.98f },
    { 1600.0f, 58.3f, 11.0f, 2.98f },
};

float CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    float dz = z1 - z2;
    return sqrt(dx*dx + dy*dy + dz*dz);
}

StrandOfTheAncients::StrandOfTheAncients(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t) : CBattleground(mgr,id,lgroup,t)
{
    Attackers = RandomUInt(2)-1;
    BattleRound = 1;
    hordewins = 0;
    allywins = 0;

    //Begin Gate Spawning.
    for(uint32 gates = 0; gates < 5; gates++)
    {
        m_gates[gates] = m_mapMgr->CreateGameObject(m_gatesLocations[gates][0]);
        if(m_gates[gates] == NULL || !m_gates[gates]->CreateFromProto(m_gatesLocations[gates][0], m_mapMgr->GetMapId() , m_gatesLocations[gates][1] ,m_gatesLocations[gates][2], m_gatesLocations[gates][3], m_gatesLocations[gates][4]))
        {
            sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "SOTA is being created and you are missing gameobjects. Terminating.");
            abort();
            return;
        }

        if(Attackers == HORDE)
            m_gates[gates]->SetUInt32Value(GAMEOBJECT_FACTION, 951);
        else if(Attackers == ALLIANCE)
            m_gates[gates]->SetUInt32Value(GAMEOBJECT_FACTION, 954);

        m_gates[gates]->PushToWorld(m_mapMgr);
    }

    m_endgate = m_mapMgr->CreateGameObject(192549);
    if(m_endgate == NULL || !m_endgate->CreateFromProto(192549, m_mapMgr->GetMapId() ,878.555f, -108.989f, 119.835f, 0.0565f))
    {
        sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "SOTA is being created and you are missing gameobjects. Terminating.");
        abort();
        return;
    }
    m_endgate->PushToWorld(m_mapMgr);

    //Spawn Gate Sigils
    for(uint32 sigils = 0; sigils < 5; sigils++)
    {
        m_gateSigils[sigils] = m_mapMgr->CreateGameObject(m_gateSigilsLocations[sigils][0]);
        if(m_gateSigils[sigils] == NULL || !m_gateSigils[sigils]->CreateFromProto(m_gateSigilsLocations[sigils][0], m_mapMgr->GetMapId() , m_gateSigilsLocations[sigils][1] , m_gateSigilsLocations[sigils][2] , m_gateSigilsLocations[sigils][3], m_gateSigilsLocations[sigils][4]))
        {
            sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "SOTA is being created and you are missing gameobjects. Terminating.");
            abort();
            return;
        }
        m_gateSigils[sigils]->PushToWorld(m_mapMgr);
    }

    //Spawn the Relic
    m_relic = m_mapMgr->CreateGameObject(192834);
    if(m_relic == NULL || !m_relic->CreateFromProto(192834, m_mapMgr->GetMapId() , 836.5f , -108.8f , 150.0f ,0.0f))
    {
        sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "SOTA is being created and you are missing gameobjects. Terminating.");
        abort();
        return;
    }
    // No suicide
    if(Attackers == HORDE)
        m_relic->SetUInt32Value(GAMEOBJECT_FACTION, 951);
    else
        m_relic->SetUInt32Value(GAMEOBJECT_FACTION, 954);

    m_relic->PushToWorld(m_mapMgr);

    CreatureProto* prto = CreatureProtoStorage.LookupEntry(27894);
    if(prto != NULL)
    {
        //Spawn the Cannons
        for(uint32 x = 0; x < 10; x++)
        {
            m_cannons[x] = m_mapMgr->CreateCreature(27894);
            if(m_cannons[x] != NULL)
            {
                m_cannons[x]->Load(prto, m_mapMgr->iInstanceMode, m_cannonsLocations[x][0], m_cannonsLocations[x][1], m_cannonsLocations[x][2], m_cannonsLocations[x][3]);
                m_cannons[x]->PushToWorld(m_mapMgr);

                //Change Cannon Factions
                if(Attackers == HORDE)
                    m_cannons[x]->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, 1981);
                else if(Attackers == ALLIANCE)
                    m_cannons[x]->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, 1892);
            }
        }
    }

    //Spawn Teleporters
    for(uint32 i = 0; i < 5; i++)
    {
        m_gateTransporters[i] = m_mapMgr->CreateGameObject(192819);
        if(m_gateTransporters[i] == NULL || !m_gateTransporters[i]->CreateFromProto(192819, m_mapMgr->GetMapId() , m_gateTransportersLocations[i][0] ,m_gateTransportersLocations[i][1], m_gateTransportersLocations[i][2], m_gateTransportersLocations[i][3]))
        {
            sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "SOTA is being created and you are missing gameobjects. Terminating.");
            abort();
            return;
        }

        // Faction Change Two.
        if(Attackers == HORDE)
            m_gateTransporters[i]->SetUInt32Value(GAMEOBJECT_FACTION, 951);
        else
            m_gateTransporters[i]->SetUInt32Value(GAMEOBJECT_FACTION, 954);

        m_gateTransporters[i]->PushToWorld(m_mapMgr);
    }
}

StrandOfTheAncients::~StrandOfTheAncients()
{
    delete m_mapMgr;
}

void StrandOfTheAncients::Init()
{
    CBattleground::Init();
}

bool StrandOfTheAncients::HookHandleRepop(Player* plr)
{
    LocationVector dest;

    dest.ChangeCoords(sotaRepop[plr->GetTeam()][0], sotaRepop[plr->GetTeam()][1],
        sotaRepop[plr->GetTeam()][2], sotaRepop[plr->GetTeam()][3]);

    plr->SafeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
    return true;
}

void StrandOfTheAncients::HookOnAreaTrigger(Player* plr, uint32 id)
{

}

void StrandOfTheAncients::HookOnPlayerDeath(Player* plr)
{
    plr->m_bgScore.Deaths++;
    UpdatePvPData();
}
void StrandOfTheAncients::OnPlatformTeleport(Player* plr)
{
    LocationVector dest;
    uint32 closest_platform = 0;

    for (uint32 i = 0; i < 5; i++)
    {
        float distance = CalculateDistance(plr->GetPositionX(),
            plr->GetPositionY(), plr->GetPositionZ(),
            sotaTransporterDestination[i][0],
            sotaTransporterDestination[i][1],
            sotaTransporterDestination[i][2]);
        if (distance < 75)
        {
            closest_platform = i;
            break;
        }
    }
    dest.ChangeCoords(sotaTransporterDestination[closest_platform][0],
        sotaTransporterDestination[closest_platform][1],
        sotaTransporterDestination[closest_platform][2],
        sotaTransporterDestination[closest_platform][3]);

    plr->SafeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
}

void StrandOfTheAncients::HookFlagDrop(Player* plr, GameObject* obj)
{

}

void StrandOfTheAncients::HookFlagStand(Player* plr, GameObject* obj)
{

}

bool StrandOfTheAncients::HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* pSpell)
{
    Respawn();
    OnStart();
    if(BattleRound == 1)
    {
        sEventMgr.RemoveEvents(this, EVENT_SOTA_TIMER);
        PlaySoundToAll( 8212 );
        pPlayer->CastSpell(pPlayer,52459,true);
        if(pPlayer->GetTeam() == Attackers)
        {
            pPlayer->SetTeam(1);
            pPlayer->SafeTeleport(pPlayer->GetMapId(),pPlayer->GetInstanceID(),SOTAStartLocations[0][0],SOTAStartLocations[0][1],SOTAStartLocations[0][2],0.0f);
        }
        else
        {
            pPlayer->SetTeam(0);
            pPlayer->SafeTeleport(pPlayer->GetMapId(),pPlayer->GetInstanceID(),SOTAStartLocations[1][0],SOTAStartLocations[1][1],SOTAStartLocations[1][2],0.0f);
        }
        if(Attackers == HORDE)
        {
            m_mapMgr->GetStateManager().UpdateWorldState(WORLDSTATE_SOTA_ALLIANCE_ATTACKER, 1 );
            m_mapMgr->GetStateManager().UpdateWorldState(WORLDSTATE_SOTA_ALLIANCE_DEFENDER, 0 );
            hordewins++;
            Attackers = ALLIANCE;
        }
        else if(Attackers == ALLIANCE)
        {
            m_mapMgr->GetStateManager().UpdateWorldState(WORLDSTATE_SOTA_ALLIANCE_DEFENDER, 1 );
            m_mapMgr->GetStateManager().UpdateWorldState(WORLDSTATE_SOTA_ALLIANCE_ATTACKER, 0 );
            allywins++;
            Attackers = HORDE;
        }
        BattleRound = 2;
        OnStart();
    }
    else if(BattleRound == 2)
    {
        EndGame();
    }
    return true;
}

void StrandOfTheAncients::Respawn()
{
    for(uint32 y = 0; y < 4; y++)
    {
        if(y == 0)
            for(uint32 z = 0; z < 9; z++)
                m_cannons[z]->Destruct();

        m_gates[y]->Destruct();
        m_gateSigils[y]->Destruct();
        m_gateTransporters[y]->Destruct();
    }

    m_endgate->Destruct();

    CreatureProto* prto = CreatureProtoStorage.LookupEntry(27894);
    if(prto != NULL)
    {
        //Spawn the Cannons
        for(uint32 x = 0; x < 10; x++)
        {
            m_cannons[x] = m_mapMgr->CreateCreature(27894);
            if(m_cannons[x] != NULL)
            {
                m_cannons[x]->Load(prto, m_mapMgr->iInstanceMode, m_cannonsLocations[x][0], m_cannonsLocations[x][1], m_cannonsLocations[x][2], m_cannonsLocations[x][3]);
                m_cannons[x]->PushToWorld(m_mapMgr);

                //Change Cannon Factions
                if(Attackers == HORDE)
                    m_cannons[x]->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, 1981);
                else if(Attackers == ALLIANCE)
                    m_cannons[x]->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, 1892);
            }
        }
    }

    // Spawn Transporters
    for(uint32 i = 0; i < 5; i++)
    {
        m_gateTransporters[i] = m_mapMgr->CreateGameObject(192819);
        if(m_gateTransporters[i] == NULL || !m_gateTransporters[i]->CreateFromProto(192819, m_mapMgr->GetMapId() , m_gateTransportersLocations[i][0] ,m_gateTransportersLocations[i][1], m_gateTransportersLocations[i][2], m_gateTransportersLocations[i][3]))
        {
            sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "SOTA is being created and you are missing gameobjects. Terminating.");
            abort();
            return;
        }

        // Set Faction Two.
        if(Attackers == HORDE)
            m_gateTransporters[i]->SetUInt32Value(GAMEOBJECT_FACTION, 951);
        else if(Attackers == ALLIANCE)
            m_gateTransporters[i]->SetUInt32Value(GAMEOBJECT_FACTION, 954);

        m_gateTransporters[i]->PushToWorld(m_mapMgr);
    }

    //Spawn Gates
    for(uint32 gates = 0; gates < 5; gates++)
    {
        m_gates[gates] = m_mapMgr->CreateGameObject(m_gatesLocations[gates][0]);
        if(m_gates[gates] == NULL || !m_gates[gates]->CreateFromProto(m_gatesLocations[gates][0], m_mapMgr->GetMapId() , m_gatesLocations[gates][1] ,m_gatesLocations[gates][2], m_gatesLocations[gates][3], m_gatesLocations[gates][4]))
        {
            sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "SOTA is being created and you are missing gameobjects. Terminating.");
            abort();
            return;
        }

        if(Attackers == HORDE)
            m_gates[gates]->SetUInt32Value(GAMEOBJECT_FACTION, 951);
        else if(Attackers == ALLIANCE)
            m_gates[gates]->SetUInt32Value(GAMEOBJECT_FACTION, 954);

        m_gates[gates]->PushToWorld(m_mapMgr);
    }

    m_endgate = m_mapMgr->CreateGameObject(192549);
    if(m_endgate == NULL || !m_endgate->CreateFromProto(192549, m_mapMgr->GetMapId() ,878.555f, -108.989f, 119.835f, 0.0565f))
    {
        sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "SOTA is being created and you are missing gameobjects. Terminating.");
        abort();
        return;
    }

    if(Attackers == HORDE)
        m_endgate->SetUInt32Value(GAMEOBJECT_FACTION, 951);
    else if(Attackers == ALLIANCE)
        m_endgate->SetUInt32Value(GAMEOBJECT_FACTION, 954);
    m_endgate->PushToWorld(m_mapMgr);

    //Spawn Gate Sigils
    for(uint32 sigils = 0; sigils < 5; sigils++)
    {
        m_gateSigils[sigils] = m_mapMgr->CreateGameObject(m_gateSigilsLocations[sigils][0]);
        if(m_gateSigils[sigils] == NULL || !m_gateSigils[sigils]->CreateFromProto(m_gateSigilsLocations[sigils][0], m_mapMgr->GetMapId() , m_gateSigilsLocations[sigils][1] , m_gateSigilsLocations[sigils][2] , m_gateSigilsLocations[sigils][3], m_gateSigilsLocations[sigils][4]))
        {
            sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "SOTA is being created and you are missing gameobjects. Terminating.");
            abort();
            return;
        }
        m_gateSigils[sigils]->PushToWorld(m_mapMgr);
    }
}

void StrandOfTheAncients::HookOnMount(Player* plr)
{
}

void StrandOfTheAncients::OnAddPlayer(Player* plr)
{
    if(!m_started)
        plr->CastSpell(plr, BG_PREPARATION, true);
}

void StrandOfTheAncients::OnRemovePlayer(Player* plr)
{
    plr->RemoveAura(BG_PREPARATION);
}

void StrandOfTheAncients::OnCreate()
{
    WorldStateManager& sm = m_mapMgr->GetStateManager();
    sm.CreateWorldState( WORLDSTATE_SOTA_CAPTURE_BAR_DISPLAY, 0 );
    sm.CreateWorldState( WORLDSTATE_SOTA_CAPTURE_BAR_VALUE, 0 );
    PrepareRound();
    sm.CreateWorldState( WORLDSTATE_SOTA_BONUS_TIME, 0 );
    sm.CreateWorldState( WORLDSTATE_SOTA_TIMER_1, 0 );
    sm.CreateWorldState( WORLDSTATE_SOTA_TIMER_2, 0 );
    sm.CreateWorldState( WORLDSTATE_SOTA_TIMER_3, 0 );
}

void StrandOfTheAncients::HookOnPlayerKill(Player* plr, Unit* pVictim)
{
    plr->m_bgScore.KillingBlows++;
    UpdatePvPData();
}

void StrandOfTheAncients::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    UpdatePvPData();
}

LocationVector StrandOfTheAncients::GetStartingCoords(uint32 Team)
{
    uint32 sTeam = ( Team == Attackers ? 1 : 0 );
    return LocationVector( SOTAStartLocations[sTeam][0],
        SOTAStartLocations[sTeam][1],
        SOTAStartLocations[sTeam][2] );
}

void StrandOfTheAncients::OnStart()
{
    m_started = true;
    SetTime(ROUND_LENGTH, 0);
    sEventMgr.AddEvent( this, &StrandOfTheAncients::TimeTick, EVENT_SOTA_TIMER, MSTIME_SECOND * 5, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );
}

void StrandOfTheAncients::HookGenerateLoot(Player* plr, Corpse* pCorpse)
{
}

void StrandOfTheAncients::HookOnShadowSight()
{
}

void StrandOfTheAncients::SetIsWeekend(bool isweekend)
{
    m_isWeekend = isweekend;
}

// Time Goes Through Seconds
void StrandOfTheAncients::SetTime(uint32 secs, uint32 WorldState)
{
    uint32 minutes = secs / TIME_MINUTE;
    uint32 seconds = secs % TIME_MINUTE;
    uint32 digits[3];
    digits[0] = minutes;
    digits[1] = seconds / 10;
    digits[2] = seconds % 10;

    //m_mapMgr->GetStateManager().UpdateWorldState( WorldState, 1 ); // This can be good.
    m_mapMgr->GetStateManager().UpdateWorldState( WORLDSTATE_SOTA_TIMER_1, digits[0] );
    m_mapMgr->GetStateManager().UpdateWorldState( WORLDSTATE_SOTA_TIMER_2, digits[1] );
    m_mapMgr->GetStateManager().UpdateWorldState( WORLDSTATE_SOTA_TIMER_3, digits[2] );
    SetRoundTime( secs );
}

void StrandOfTheAncients::PrepareRound()
{
    if(Attackers == HORDE)
    {
        m_mapMgr->GetStateManager().CreateWorldState( WORLDSTATE_SOTA_ALLIANCE_DEFENDER, 1 );
        m_mapMgr->GetStateManager().CreateWorldState( WORLDSTATE_SOTA_ALLIANCE_ATTACKER, 0 );
    }
    else if(Attackers == ALLIANCE)
    {
        m_mapMgr->GetStateManager().CreateWorldState( WORLDSTATE_SOTA_ALLIANCE_DEFENDER, 0 );
        m_mapMgr->GetStateManager().CreateWorldState( WORLDSTATE_SOTA_ALLIANCE_ATTACKER, 1 );
    }
    else
    {
        Attackers = RandomUInt(2)-1;
        PrepareRound();
    }
};

void StrandOfTheAncients::TimeTick()
{
    SetTime(GetRoundTime() - 1,0);
    if(GetRoundTime() == 0)
    {
        sEventMgr.RemoveEvents(this, EVENT_SOTA_TIMER);
        if(BattleRound == 1)
        {
            Player* pPlayer;
            PlaySoundToAll( 8212 );
            for(uint8 i = 0; i < 2; i++)
            {
                for(set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); itr++)
                {
                    if((pPlayer = (*itr)) != NULL)
                    {
                        pPlayer->CastSpell(pPlayer,52459,true);
                        if(pPlayer->GetTeam() == Attackers)
                        {
                            pPlayer->SetTeam(1);
                            pPlayer->SafeTeleport(pPlayer->GetMapId(),pPlayer->GetInstanceID(),SOTAStartLocations[0][0],SOTAStartLocations[0][1],SOTAStartLocations[0][2],0.0f);
                        }
                        else
                        {
                            pPlayer->SetTeam(0);
                            pPlayer->SafeTeleport(pPlayer->GetMapId(),pPlayer->GetInstanceID(),SOTAStartLocations[1][0],SOTAStartLocations[1][1],SOTAStartLocations[1][2],0.0f);
                        }
                    }
                }
            }

            Respawn();
            OnStart();
            if(Attackers == HORDE)
            {
                m_mapMgr->GetStateManager().UpdateWorldState(WORLDSTATE_SOTA_ALLIANCE_ATTACKER, 1 );
                m_mapMgr->GetStateManager().UpdateWorldState(WORLDSTATE_SOTA_ALLIANCE_DEFENDER, 0 );
                Attackers = ALLIANCE;
            }
            else if(Attackers == ALLIANCE)
            {
                m_mapMgr->GetStateManager().UpdateWorldState(WORLDSTATE_SOTA_ALLIANCE_DEFENDER, 1 );
                m_mapMgr->GetStateManager().UpdateWorldState(WORLDSTATE_SOTA_ALLIANCE_ATTACKER, 0 );
                Attackers = HORDE;
            }
            BattleRound = 2;
            OnStart();
        }
        else if(BattleRound == 2)
        {
            EndGame();
        }
    }
};

void StrandOfTheAncients::EndGame()
{
    m_ended = true;
    if(hordewins > allywins)
        m_losingteam = ALLIANCE;
    else if(allywins > hordewins)
        m_losingteam = HORDE;
    else if(allywins == hordewins)
        m_losingteam = 2;
    m_nextPvPUpdateTime = 0;

    sEventMgr.RemoveEvents(this);
    sEventMgr.AddEvent( (CBattleground*)this, &CBattleground::Close, EVENT_BATTLEGROUND_CLOSE, 120000, 1,0);

    m_mainLock.Acquire();
    // Honor is added?
    SpellEntry * winner_spell = dbcSpell.LookupEntry(61160);
    SpellEntry * loser_spell = dbcSpell.LookupEntry(61159);
    for(uint32 i = 0; i < 2; i++)
    {
        for(set<Player*  >::iterator itr = m_players[i].begin(); itr != m_players[i].end(); itr++)
        {
            (*itr)->Root();

            if( (*itr)->HasFlag(PLAYER_FLAGS, 0x2) )
                continue;

            if(i == m_losingteam)
            {
                (*itr)->CastSpell((*itr), loser_spell, true);
                if((*itr)->fromrandombg)
                {
                    (*itr)->m_honorToday += (*itr)->GenerateRBGReward((*itr)->getLevel(),5);
                    HonorHandler::RecalculateHonorFields((*itr));
                    (*itr)->fromrandombg = false;
                }
            }
            else
            {
                (*itr)->CastSpell((*itr), winner_spell, true);
                if((*itr)->fromrandombg)
                {
                    Player * p = (*itr);
                    p->AddArenaPoints(p->randombgwinner == false ? p->GenerateRBGReward(p->getLevel(),25) : p->GenerateRBGReward(p->getLevel(),0));
                    p->m_honorToday += p->randombgwinner == false ? p->GenerateRBGReward(p->getLevel(),30) : p->GenerateRBGReward(p->getLevel(),15);
                    HonorHandler::RecalculateHonorFields(p);
                    p->randombgwinner = true;
                    p->fromrandombg = false;
                }
            }
        }
    }

    UpdatePvPData();
    m_mainLock.Release();
};
