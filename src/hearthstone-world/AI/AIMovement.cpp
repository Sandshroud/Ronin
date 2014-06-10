/***
 * Demonstrike Core
 */

#include "StdAfx.h"

AI_Movement::AI_Movement()
{
    Nullify();
}

void AI_Movement::Nullify()
{
    m_canMove = true;
    m_moveRun = m_moveFly = false;
    m_moveJump = m_moveSprint = m_moveBackward = false;
    m_WayPointsShowing = m_WayPointsShowBackwards = false;

    m_pathfinding = m_ignorePathMap = false;

    m_Unit = NULL;
    PathMap = NULL;
    UnitToFear = NULL;
    m_waypoints = NULL;
    m_FormationLinkTarget = NULL;
    UnitToFollow = UnitToFollow_backup = NULL;

    m_CurrentWayPoint = 0;
    m_FormationLinkSqlId = 0;
    m_FearTimer = m_WanderTimer = 0;
    m_timeToMove = m_timeMoved = m_moveTimer = m_totalMoveTime = 0;

    m_lastFollowX = m_lastFollowY = 0.0f;
    m_sourceX = m_sourceY = m_sourceZ = 0.0f;
    m_walkSpeed = m_runSpeed = m_flySpeed = 0.0f;
    m_returnX = m_returnY = m_returnZ = m_returnO = 0.0f;
    m_nextPosX = m_nextPosY = m_nextPosZ = m_nextPosO = 0.0f;
    m_FormationFollowAngle = m_FormationFollowDistance = 0.0f;
    FollowDistance = FollowDistance_backup = m_FollowAngle = 0.0f;
    m_destinationX = m_destinationY = m_destinationZ = m_destinationO = 0.0f;
}

void AI_Movement::Initialize(AIInterface* AI, Unit* un, MovementType mt)
{
    Nullify();
    m_Unit = un;
    Interface = AI;
    m_MovementType = mt;
    m_MovementState = MOVEMENTSTATE_STOP;

    if(un->IsCreature())
        if(TO_CREATURE(un)->GetCanMove() & LIMIT_AIR)
            m_moveFly = true;

    m_walkSpeed = un->m_walkSpeed*0.001f;//move distance per ms time
    m_runSpeed = un->m_runSpeed*0.001f;//move/ms
    m_flySpeed = un->m_flySpeed*0.001f;
    m_sourceX = un->GetPositionX();
    m_sourceY = un->GetPositionY();
    m_sourceZ = un->GetPositionZ();
}

void AI_Movement::DeInitialize()
{
    Nullify();
    CurrentMoveRequired.clear();
}

void AI_Movement::Update(uint32 p_time)
{
    ASSERT(m_Unit != NULL);

    if(!m_Unit->isAlive())
        return;

    if(m_moveTimer > 0)
    {
        if(p_time >= m_moveTimer)
            m_moveTimer = 0;
        else
            m_moveTimer -= p_time;
    }

    if(m_timeToMove > 0)
        m_timeMoved = m_timeToMove <= p_time + m_timeMoved ? m_timeToMove : p_time + m_timeMoved;

    if(getCreatureState() == MOVING)
    {
        if(PathMap != NULL)
        {
            if(!m_moveTimer)
            {   // This works, but maybe a 200ms update time would be better for pathfinding.
                if(m_timeMoved == m_timeToMove)
                {
                    delete PathMap->InternalMap;
                    PathMap->InternalMap = NULL;
                    delete PathMap;
                    PathMap = NULL;

                    setCreatureState(STOPPED);
                    m_moveSprint = false;

                    m_Unit->SetPosition(m_destinationX, m_destinationY, m_destinationZ, (m_destinationO == 0.0f ? m_Unit->GetOrientation() : m_destinationO));
                    m_destinationX = m_destinationY = m_destinationZ = m_destinationO = 0.0f;
                    m_timeMoved = 0;
                    m_timeToMove = 0;
                }
                else
                {
                    // Move Server Side Update
                    bool positionchanged = false;
                    int32 sexyteim[2] = { 0, m_timeMoved };
                    float x1 = m_sourceX, y1 = m_sourceY, z1 = m_sourceZ, x2 = m_destinationX, y2 = m_destinationY, z2 = m_destinationZ;
                    for(LocationVectorMap::iterator itr = PathMap->InternalMap->begin(), lastitr = PathMap->InternalMap->begin(); itr != PathMap->InternalMap->end();)
                    {
                        if(m_timeMoved == (*itr).first)
                        {
                            positionchanged = true;
                            m_Unit->SetPosition((*itr).second.x, (*itr).second.y, (*itr).second.z, m_Unit->GetOrientation());
                            break;
                        }

                        if(m_timeMoved < (*itr).first)
                        {
                            if(itr != PathMap->InternalMap->begin())
                            {   // If we're not the first itr, grab info, if we are, use starting position.
                                sexyteim[0] = (*lastitr).first;
                                x1 = (*lastitr).second.x;
                                y1 = (*lastitr).second.y;
                                z1 = (*lastitr).second.z;
                            }

                            sexyteim[1] = (*itr).first;
                            x2 = (*itr).second.x;
                            y2 = (*itr).second.y;
                            z2 = (*itr).second.z;
                            break;
                        }
                        lastitr = itr++;
                    }

                    uint32 timeleft = UNIT_MOVEMENT_INTERPOLATE_INTERVAL/2;
                    if(!positionchanged)
                    {
                        timeleft = (sexyteim[1]-sexyteim[0])-(m_timeMoved-sexyteim[0]);
                        float q = float(float(m_timeMoved)-float(sexyteim[0]))/float(float(sexyteim[1])-float(sexyteim[0]));

                        float x = x1 - ((x1 - x2) * q);
                        float y = y1 - ((y1 - y2) * q);
                        float z = z1 - ((z1 - z2) * q);
                        m_Unit->SetPosition(x, y, z, m_Unit->GetOrientation());
                    }

                    if(!timeleft || timeleft > UNIT_MOVEMENT_INTERPOLATE_INTERVAL/2)
                        timeleft = UNIT_MOVEMENT_INTERPOLATE_INTERVAL/2;
                    m_moveTimer = timeleft;

                    Player *update = NULL;
                    uint32 ms_Timer = getMSTime();
                    std::map<uint32, uint32>::iterator plritr, plritr2;
                    for(plritr = CurrentMoveRequired.begin();  plritr != CurrentMoveRequired.end();)
                    {
                        plritr2 = plritr++;
                        update = m_Unit->GetMapMgr()->GetPlayer(plritr2->first);
                        if(update != NULL)
                        {
                            SendMoveToPacket(update);
                            CurrentMoveRequired.erase(plritr2);
                            update = NULL;
                            continue;
                        }
                        update = NULL;

                        // 30 second limit.
                        if(plritr2->second+30 < ms_Timer)
                            CurrentMoveRequired.erase(plritr2);
                    }
                }
            }
        }
        else if(!m_moveTimer)
        {
            if(m_timeMoved == m_timeToMove) //reached destination
            {
                if(m_moveType == MOVEMENTTYPE_WANTEDWP)//We reached wanted wp stop now
                    m_moveType = MOVEMENTTYPE_DONTMOVEWP;

                float wayO = m_nextPosO;
                if((GetWayPointsCount() != 0) && (getAIState() == STATE_IDLE || getAIState() == STATE_SCRIPTMOVE)) //if we attacking don't use wps
                {
                    WayPoint* wp = getWayPoint(getCurrentWaypoint());
                    if(wp)
                    {
                        CALL_SCRIPT_EVENT(m_Unit, OnReachWP)(wp->id, !m_moveBackward);
                        if(TO_CREATURE(m_Unit)->has_waypoint_text)
                            objmgr.HandleMonsterSayEvent(TO_CREATURE(m_Unit), MONSTER_SAY_EVENT_RANDOM_WAYPOINT);

                        //Lets face to correct orientation
                        if(wp->flags & 128)
                            m_moveType = MOVEMENTTYPE_CIRCLEWP;
                        wayO = wp->orientation;
                        m_moveTimer = wp->waittime; //wait before next move
                        if(!m_moveBackward)
                        {
                            if(wp->forwardInfo)
                            {
                                if(wp->forwardInfo->EmoteOneShot)
                                    m_Unit->Emote(EmoteType(wp->forwardInfo->EmoteID));
                                else if(m_Unit->GetUInt32Value(UNIT_NPC_EMOTESTATE) != wp->forwardInfo->EmoteID)
                                    m_Unit->SetUInt32Value(UNIT_NPC_EMOTESTATE, wp->forwardInfo->EmoteID);

                                if(m_Unit->getStandState() != wp->forwardInfo->StandState )
                                    m_Unit->SetStandState(wp->forwardInfo->StandState);
                                if (wp->forwardInfo->SpellToCast)
                                    m_Unit->CastSpell(m_Unit, wp->forwardInfo->SpellToCast, false);
                                if (wp->forwardInfo->SayText != "")
                                    m_Unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, wp->forwardInfo->SayText.c_str() );
                            }
                        }
                        else
                        {
                            if(wp->backwardInfo)
                            {
                                if(wp->backwardInfo->EmoteOneShot)
                                    m_Unit->Emote(EmoteType(wp->backwardInfo->EmoteID));
                                else if(m_Unit->GetUInt32Value(UNIT_NPC_EMOTESTATE) != wp->backwardInfo->EmoteID)
                                    m_Unit->SetUInt32Value(UNIT_NPC_EMOTESTATE, wp->backwardInfo->EmoteID);

                                if(m_Unit->getStandState() != wp->backwardInfo->StandState )
                                    m_Unit->SetStandState(wp->backwardInfo->StandState);
                                if (wp->backwardInfo->SpellToCast)
                                    m_Unit->CastSpell(m_Unit, wp->backwardInfo->SpellToCast, false);
                                if (wp->backwardInfo->SayText != "")
                                    m_Unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, wp->backwardInfo->SayText.c_str() );
                            }
                        }
                    }
                    else
                        m_moveTimer = RandomUInt(m_moveRun ? 5000 : 10000); // wait before next move
                }

                setCreatureState(STOPPED);
                m_moveSprint = false;

                m_Unit->SetPosition(m_nextPosX, m_nextPosY, m_nextPosZ, (wayO == 0.0f ? m_Unit->GetOrientation() : wayO));
                m_nextPosX = m_nextPosY = m_nextPosZ = m_nextPosO = 0.0f;
                m_timeMoved = 0;
                m_timeToMove = 0;
            }
            else
            {
                // Move Server Side Update
                float q = (float)m_timeMoved / (float)m_timeToMove;
                float x = m_Unit->GetPositionX() + (m_nextPosX - m_Unit->GetPositionX()) * q;
                float y = m_Unit->GetPositionY() + (m_nextPosY - m_Unit->GetPositionY()) * q;
                float z = m_Unit->GetPositionZ() + (m_nextPosZ - m_Unit->GetPositionZ()) * q;

                m_Unit->SetPosition(x, y, z, m_Unit->GetOrientation());

                m_timeToMove -= m_timeMoved;
                m_timeMoved = 0;
                m_moveTimer = (UNIT_MOVEMENT_INTERPOLATE_INTERVAL < m_timeToMove) ? UNIT_MOVEMENT_INTERPOLATE_INTERVAL : m_timeToMove;
            }

            //**** Movement related stuff that should be done after a move update (Keeps Client and Server Synced) ****//
            //**** Process the Pending Move ****//
            if(m_destinationX != 0.0f && m_destinationY != 0.0f)
                UpdateMove();
        }
    }
    else if(getCreatureState() == STOPPED && (getAIState() == STATE_IDLE || getAIState() == STATE_SCRIPTMOVE) && !m_moveTimer && !m_timeToMove && UnitToFollow == NULL) //creature is stopped and out of Combat
    {
        if(m_Unit->GetUInt32Value(UNIT_FIELD_DISPLAYID) == 5233) //if Spirit Healer don't move
            return;

        // do we have a formation?
        if(m_FormationLinkSqlId != 0)
        {
            if(m_FormationLinkTarget == NULL)
            {
                // haven't found our target yet
                Creature* c = TO_CREATURE(m_Unit);
                if(!c->haslinkupevent)
                {
                    // register linkup event
                    c->haslinkupevent = true;
                    sEventMgr.AddEvent(c, &Creature::FormationLinkUp, m_FormationLinkSqlId, EVENT_CREATURE_FORMATION_LINKUP, 1000, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                }
            }
            else
            {
                // we've got a formation target, set unittofollow to this
                UnitToFollow = m_FormationLinkTarget;
                FollowDistance = m_FormationFollowDistance;
                m_FollowAngle = m_FormationFollowAngle;
            }
        }

        if(UnitToFollow == NULL)
        {
            // no formation, use waypoints
            int destpoint = -1;

            // If creature has no waypoints just wander aimlessly around spawnpoint
            if(GetWayPointsCount() == 0) //no waypoints
                return;
            else //we do have waypoints
            {
                uint32 wpcount = uint32(GetWayPointsCount());
                if(m_moveType == MOVEMENTTYPE_RANDOMWP) //is random move on if so move to a random waypoint
                {
                    if(wpcount > 1)
                        destpoint = RandomUInt(wpcount-1)+1;
                }
                else if (m_moveType == MOVEMENTTYPE_CIRCLEWP) //random move is not on lets follow the path in circles
                {
                    // 1 -> 10 then 1 -> 10
                    m_CurrentWayPoint++;
                    if (m_CurrentWayPoint > wpcount)
                        m_CurrentWayPoint = 1; //Happens when you delete last wp seems to continue ticking
                    destpoint = m_CurrentWayPoint;
                    m_moveBackward = false;
                }
                else if(m_moveType == MOVEMENTTYPE_WANTEDWP)//Move to wanted wp
                {
                    if(m_CurrentWayPoint)
                    {
                        if( wpcount > 0)
                        {
                            destpoint = m_CurrentWayPoint;
                        }
                        else
                            destpoint = -1;
                    }
                }
                else if(m_moveType == MOVEMENTTYPE_FORWARDTHANSTOP)// move to end, then stop
                {
                    ++m_CurrentWayPoint;
                    if(m_CurrentWayPoint > wpcount)
                    {
                        //hmm maybe we should stop being path walker since we are waiting here anyway
                        destpoint = -1;
                    }
                    else
                        destpoint = m_CurrentWayPoint;
                }
                else if(m_moveType != MOVEMENTTYPE_QUEST && m_moveType != MOVEMENTTYPE_DONTMOVEWP)//4 Unused
                {
                    // 1 -> 10 then 10 -> 1
                    if (m_CurrentWayPoint > wpcount)
                        m_CurrentWayPoint = 1; //Happens when you delete last wp seems to continue ticking
                    if (m_CurrentWayPoint == wpcount) // Are we on the last waypoint? if so walk back
                        m_moveBackward = true;
                    if (m_CurrentWayPoint == 1) // Are we on the first waypoint? if so lets goto the second waypoint
                        m_moveBackward = false;
                    if (!m_moveBackward) // going 1..n
                        destpoint = ++m_CurrentWayPoint;
                    else                // going n..1
                        destpoint = --m_CurrentWayPoint;
                }

                if(destpoint != -1)
                {
                    WayPoint* wp = getWayPoint(destpoint);
                    if(wp)
                    {
                        if(!m_moveBackward)
                        {
                            if(wp->forwardInfo)
                            {
                                if((wp->forwardInfo->SkinID != 0) && (m_Unit->GetUInt32Value(UNIT_FIELD_DISPLAYID) != wp->forwardInfo->SkinID))
                                {
                                    m_Unit->SetUInt32Value(UNIT_FIELD_DISPLAYID, wp->forwardInfo->SkinID);
                                    m_Unit->EventModelChange();
                                }
                            }
                        }
                        else
                        {
                            if(wp->backwardInfo)
                            {
                                if((wp->backwardInfo->SkinID != 0) && (m_Unit->GetUInt32Value(UNIT_FIELD_DISPLAYID) != wp->backwardInfo->SkinID))
                                {
                                    m_Unit->SetUInt32Value(UNIT_FIELD_DISPLAYID, wp->backwardInfo->SkinID);
                                    m_Unit->EventModelChange();
                                }
                            }
                        }
                        if(wp->flags & 128)
                            m_moveType = MOVEMENTTYPE_CIRCLEWP;
                        m_moveRun = (wp->flags & 256) ? 1 : 0;
                        m_moveFly = (wp->flags & 512) ? true : false;
                        MoveTo(wp->x, wp->y, wp->z, wp->orientation);
                    }
                }
            }
        }
    }

    // Fear Code
    if(getAIState() == STATE_FEAR && UnitToFear != NULL && getCreatureState() == STOPPED)
    {
        if(p_time > m_FearTimer)   // Wait at point for x ms ;)
        {
            float Fx;
            float Fy;
            float Fz;
            // Calculate new angle to target.
            float Fo = m_Unit->calcRadAngle(UnitToFear->GetPositionX(), UnitToFear->GetPositionY(), m_Unit->GetPositionX(), m_Unit->GetPositionY());
            double fAngleAdd = RandomDouble(((M_PI/2) * 2)) - (M_PI/2);
            Fo += (float)fAngleAdd;

            float dist = m_Unit->CalcDistance(UnitToFear);
            if(dist > 30.0f || (Rand(25) && dist > 10.0f))  // not too far or too close
            {
                Fx = m_Unit->GetPositionX() - (RandomFloat(15.f)+5.0f)*cosf(Fo);
                Fy = m_Unit->GetPositionY() - (RandomFloat(15.f)+5.0f)*sinf(Fo);
            }
            else
            {
                Fx = m_Unit->GetPositionX() + (RandomFloat(20.f)+5.0f)*cosf(Fo);
                Fy = m_Unit->GetPositionY() + (RandomFloat(20.f)+5.0f)*sinf(Fo);
            }
            Fz = m_Unit->GetCHeightForPosition(false); // We COULD check water height, but nah.

            // Check if this point is in water.
            float wl = m_Unit->GetMapMgr()->GetWaterHeight(Fx, Fy, Fz);
            if( fabs( m_Unit->GetPositionZ() - Fz ) > 3.5f || ( wl != NO_WATER_HEIGHT && Fz < wl ) ) // in water
            {
                m_FearTimer = p_time + 500;
            }
            else
            {
                MoveTo(Fx, Fy, Fz);
                m_FearTimer = m_totalMoveTime + p_time + 400;
            }
        }
    }

    // Wander AI movement code
    if(getAIState() == STATE_WANDER && getCreatureState() == STOPPED)
    {
        if(p_time < m_WanderTimer) // is it time to move again?
            return;

        // calculate a random distance and angle to move
        float wanderD = RandomFloat(2.0f) + 2.0f;
        float wanderO = RandomFloat(6.283f);
        float Fx = m_Unit->GetPositionX() + wanderD * cosf(wanderO);
        float Fy = m_Unit->GetPositionY() + wanderD * sinf(wanderO);
        float Fz;

        if (m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->CanUseCollision(m_Unit))
        {
            if( !sVMapInterface.GetFirstPoint( m_Unit->GetMapId(), m_Unit->GetInstanceID(), m_Unit->GetPhaseMask(), m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ(),
                Fx, Fy, m_Unit->GetPositionZ() + 1.5f, Fx, Fy, Fz, -3.5f ) )
            {
                // clear path?
                Fz = sVMapInterface.GetHeight( m_Unit->GetMapId(), m_Unit->GetInstanceID(), m_Unit->GetPhaseMask(), Fx, Fy, m_Unit->GetPositionZ() );
                if( Fz == NO_WMO_HEIGHT )
                    Fz = m_Unit->GetMapMgr()->GetLandHeight(Fx, Fy);
            }
            else
            {
                // obstruction in the way.
                // the distmod will fuck up the Z, so get a new height.
                float fz2 = sVMapInterface.GetHeight(m_Unit->GetMapId(), m_Unit->GetInstanceID(), m_Unit->GetPhaseMask(), Fx, Fy, Fz);
                if( fz2 != NO_WMO_HEIGHT )
                    Fz = fz2;
            }

            if( fabs( m_Unit->GetPositionZ() - Fz ) > 3.5f )
            {
                m_WanderTimer = p_time + 1000;
            }
            else
            {
                m_Unit->SetOrientation(wanderO);
                MoveTo(Fx, Fy, Fz);
                m_WanderTimer = p_time + m_totalMoveTime + 300; // time till next move (+ pause)
            }
        }
        else
        {
            Fz = m_Unit->GetMapMgr()->GetLandHeight(Fx, Fy);

            // without these next checks we could fall through the "ground" (WMO) and get stuck
            // wander won't work correctly in cities until we get some way to fix this and remove these checks
            float currentZ = m_Unit->GetPositionZ();
            float landZ = m_Unit->GetMapMgr()->GetLandHeight(m_Unit->GetPositionX(), m_Unit->GetPositionY());

            if( currentZ > landZ + 1.0f // are we more than 1yd above ground? (possible WMO)
             || Fz < currentZ - 5.0f // is our destination land height too low? (possible WMO)
             || Fz > currentZ + wanderD) // is our destination too high to climb?
            {
                m_WanderTimer = p_time + 1000; // wait 1 second before we try again
                return;
            }

            m_Unit->SetOrientation(wanderO);
            MoveTo(Fx, Fy, Fz);
            m_WanderTimer = p_time + m_totalMoveTime + 300; // time till next move (+ pause)
        }
    }

    //Unit Follow Code
    if(UnitToFollow != NULL)
    {
        if( UnitToFollow->GetInstanceID() != m_Unit->GetInstanceID() || !UnitToFollow->IsInWorld() )
            UnitToFollow = NULLUNIT;
        else
        {
            if( getAIState() == STATE_IDLE || getAIState() == STATE_FOLLOWING)
            {
                float dist = m_Unit->GetDistanceSq(UnitToFollow);

                // re-calculate orientation based on target's movement
                if(m_lastFollowX != UnitToFollow->GetPositionX() ||
                    m_lastFollowY != UnitToFollow->GetPositionY())
                {
                    float dx = UnitToFollow->GetPositionX() - m_Unit->GetPositionX();
                    float dy = UnitToFollow->GetPositionY() - m_Unit->GetPositionY();
                    if(dy != 0.0f)
                    {
                        float angle = atan2(dx,dy);
                        m_Unit->SetOrientation(angle);
                    }
                    m_lastFollowX = UnitToFollow->GetPositionX();
                    m_lastFollowY = UnitToFollow->GetPositionY();
                }

                if (dist > (FollowDistance*FollowDistance)) //if out of range
                {
                    SetAIState(STATE_FOLLOWING);

                    if(dist > 25.0f) //25 yard away lets run else we will loose the them
                        m_moveRun = true;
                    else
                        m_moveRun = false;

                    if(getAIType() == AITYPE_PET || UnitToFollow == GetFormationLinkTarget()) //Unit is Pet/formation
                    {
                        if(dist > 900.0f/*30*/)
                            m_moveSprint = true;

                        float delta_x = UnitToFollow->GetPositionX();
                        float delta_y = UnitToFollow->GetPositionY();
                        float d = 3;
                        if(GetFormationLinkTarget() != NULL)
                            d = GetFormationFollowDistance();

                        MoveTo(delta_x+(d*(cosf(m_FollowAngle+UnitToFollow->GetOrientation()))),
                            delta_y+(d*(sinf(m_FollowAngle+UnitToFollow->GetOrientation()))),
                            UnitToFollow->GetPositionZ());
                    }
                    else
                    {
                        _CalcDestinationAndMove(UnitToFollow, FollowDistance);
                    }
                }
            }
        }
    }
}

uint32 AI_Movement::getMoveFlags(bool ignorejump)
{
    ASSERT(m_Unit != NULL);

    uint32 MoveFlags = MONSTER_MOVE_FLAG_WALK;
    if(!ignorejump && m_moveJump == true)
    {
        m_flySpeed = m_Unit->m_flySpeed*0.001f;
        MoveFlags = MONSTER_MOVE_FLAG_JUMP;
    }
    else if(m_moveFly == true) //Fly
    {
        m_flySpeed = m_Unit->m_flySpeed*0.001f;
        MoveFlags = MONSTER_MOVE_FLAG_FLY;
    }
    else if(m_moveSprint == true) //Sprint
    {
        m_runSpeed = (m_Unit->m_runSpeed+5.0f)*0.001f;
    }
    else if(m_moveRun == true) //Run
    {
        m_runSpeed = m_Unit->m_runSpeed*0.001f;
    }
    m_walkSpeed = m_Unit->m_walkSpeed*0.001f;//move distance per ms time
    return MoveFlags;
}

//this should NEVER be called directly !!!!!!
//use MoveTo()
void AI_Movement::SendMoveToPacket(float toX, float toY, float toZ, float toO, uint32 time, uint32 MoveFlags, Player* playerTarget)
{
    ASSERT(m_Unit != NULL);
    bool orientation = (toO != 0.0f);

    float posX = m_Unit->GetPositionX(), posY = m_Unit->GetPositionY(), posZ = m_Unit->GetPositionZ();
    WorldPacket data(SMSG_MONSTER_MOVE, 100);
    data << m_Unit->GetNewGUID();
    data << uint8(0);
    data << posX << posY << posZ;
    data << getMSTime();
    if(time == 0 && posX == toX && posY == toY && posZ == toZ)
        data << uint8(1); //stop
    else
    {
        data << uint8(orientation ? 4 : 0);
        if(orientation)
            data << float( toO );
        data << MoveFlags;
        data << time;
        if(MoveFlags & MONSTER_MOVE_FLAG_JUMP)
            data << float((toZ - m_Unit->GetPositionZ()) + 4.0f) << uint32(0);
        data << uint32(1);  // 1 waypoint
        data << toX << toY << toZ;
    }

    if(playerTarget == NULL)
        m_Unit->SendMessageToSet( &data, m_Unit->IsPlayer() ? true : false );
    else
    {
        if(playerTarget->IsInWorld())
            playerTarget->GetSession()->SendPacket(&data);
        else
        {
            playerTarget->CopyAndSendDelayedPacket(&data);
        }
    }
}

void AI_Movement::SendMoveToPacket(Player* playerTarget)
{
    ASSERT(m_Unit != NULL);
    if(PathMap == NULL)
        return;

    LocationVectorMap MovementMap = *PathMap->InternalMap;
    if(MovementMap.size() < 4)
    {
        SendMoveToPacket(m_destinationX, m_destinationY, m_destinationZ, m_destinationO, GetMovementTime(m_Unit->CalcDistance(m_destinationX, m_destinationY, m_destinationZ)), getMoveFlags(), playerTarget);
        return;
    }

    LocationVectorMap::iterator itr = MovementMap.begin();
    float startx = (*itr).second.x, starty = (*itr).second.y, startz = (*itr).second.z;
    float mid_X = (startx + m_destinationX) * 0.5f;
    float mid_Y = (starty + m_destinationY) * 0.5f;
    float mid_Z = (startz + m_destinationZ) * 0.5f;

    WorldPacket data(SMSG_MONSTER_MOVE, 8+1+4+4+4+4+1+4+4+4+(MovementMap.size()*4));
    data << m_Unit->GetNewGUID();
    data << uint8(0);
    data << startx << starty << startz;
    data << PathMap->StartTime;
    if(m_destinationO)
        data << uint8(4) << float( m_destinationO );
    else
        data << uint8(0);
    data << getMoveFlags();
    data << PathMap->TotalMoveTime;
    data << uint32(MovementMap.size()-1);   // waypoint count
    data << m_destinationX << m_destinationY << m_destinationZ;

    int32 packedlocationshit = 0;
    while(itr != MovementMap.end())
    {
        packedlocationshit = 0;
        packedlocationshit |= (((int)((mid_X - (*itr).second.x)/0.25f)) & 0x7FF);
        packedlocationshit |= (((int)((mid_Y - (*itr).second.y)/0.25f)) & 0x7FF) << 11;
        packedlocationshit |= (((int)((mid_Z - (*itr).second.z)/0.25f)) & 0x3FF) << 22;
        data << packedlocationshit;
        ++itr;
    }

    if(playerTarget == NULL)
        m_Unit->SendMessageToSet( &data, m_Unit->IsPlayer());
    else
    {
        if(playerTarget->IsInWorld())
            playerTarget->GetSession()->SendPacket(&data);
        else
        {
            playerTarget->CopyAndSendDelayedPacket(&data);
            delete &data;
        }
    }
}

void AI_Movement::MoveTo(float x, float y, float z, float o, bool IgnorePathMap)
{
    ASSERT(m_Unit != NULL);
    m_sourceX = m_Unit->GetPositionX();
    m_sourceY = m_Unit->GetPositionY();
    m_sourceZ = m_Unit->GetPositionZ();

    if(!m_canMove || m_Unit->IsStunned()|| m_Unit->isCasting())
    {
        StopMovement(0); //Just Stop
        return;
    }

    m_destinationX = x;
    m_destinationY = y;
    m_destinationZ = z;
    m_destinationO = o;

    if(PathMap != NULL)
    {
        delete PathMap->InternalMap;
        PathMap->InternalMap = NULL;
        delete PathMap;
        PathMap = NULL;
    }

    m_ignorePathMap = IgnorePathMap;
    if(getCreatureState() != MOVING)
        UpdateMove();
}

void AI_Movement::SendJumpTo(float toX, float toY, float toZ, uint32 moveTime, float arc, uint32 unk)
{
    WorldPacket data(SMSG_MONSTER_MOVE, 100);
    data << m_Unit->GetNewGUID();
    data << uint8(0);
    data << m_Unit->GetPositionX() << m_Unit->GetPositionY() << m_Unit->GetPositionZ();
    data << getMSTime();
    data << uint8(0);
    data << uint32(MONSTER_MOVE_FLAG_JUMP); //Jump + walking for the run on land.
    data << moveTime;
    data << float(arc);
    data << uint32(0);
    data << uint32(1);
    data << toX << toY << toZ;
    m_Unit->SendMessageToSet( &data, m_Unit->IsPlayer() ? true : false );
}

void AI_Movement::StopMovement(uint32 time, bool stopatcurrent)
{
    ASSERT(m_Unit != NULL);
    if(m_Unit->GetVehicle())
        return;

    if(PathMap != NULL)
    {
        delete PathMap->InternalMap;
        PathMap->InternalMap = NULL;
        delete PathMap;
        PathMap = NULL;
    }

    m_moveTimer = time; //set pause after stopping
    setCreatureState(STOPPED);

    if(m_nextPosY && m_nextPosX)
    {
        float orient = atan2(m_nextPosY - m_Unit->GetPositionY(), m_nextPosX - m_Unit->GetPositionX());
        m_Unit->SetOrientation(orient);
    }

    m_nextPosX = m_nextPosY = m_nextPosZ = 0;
    m_destinationX = m_destinationY = m_destinationZ = 0;
    m_timeMoved = 0;
    m_timeToMove = 0;
    if(stopatcurrent)
        SendMoveToPacket(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ(), m_Unit->GetOrientation(), 0, getMoveFlags());
}

void AI_Movement::UpdateMove()
{
    ASSERT(m_Unit != NULL && m_Unit->IsInWorld());
    //this should NEVER be called directly !!!!!!
    //use MoveTo()

    if(!m_ignorePathMap && !m_moveJump && sWorld.PathFinding && NavMeshInterface.IsNavmeshLoadedAtPosition(m_Unit->GetMapId(), m_Unit->GetPositionX(), m_Unit->GetPositionY()))
    {
        if(PathMap != NULL)
        {
            SendMoveToPacket();
            return;
        }

        PathMap = NavMeshInterface.BuildFullPath(m_Unit, m_Unit->GetMapId(), m_sourceX, m_sourceY, m_sourceZ, m_destinationX, m_destinationY, m_destinationZ);

        // Found a path!
        if(PathMap != NULL && (PathMap->InternalMap->size() > 1)) // We should always have more than 1 point, since build adds start.
        {
            m_totalMoveTime = PathMap->TotalMoveTime;

            if(m_destinationO == 0.0f)
                m_destinationO = m_Unit->GetOrientation();
            SendMoveToPacket();

            m_timeMoved = 0;
            m_timeToMove = PathMap->TotalMoveTime;
            setCreatureState(MOVING);
            m_moveTimer = UNIT_MOVEMENT_INTERPOLATE_INTERVAL/2; // update every few msecs
            m_moveJump = false;
            return;
        }
    }

    if(PathMap != NULL)
    {
        delete PathMap->InternalMap;
        PathMap->InternalMap = NULL;
        delete PathMap;
        PathMap = NULL;
    }

    if(!m_pathfinding && sWorld.PathFinding)
    {
        if(NavMeshInterface.IsNavmeshLoadedAtPosition(m_Unit->GetMapId(), m_Unit->GetPositionX(), m_Unit->GetPositionY()))
        {
            // Reset our source position
            m_sourceX = m_Unit->GetPositionX();
            m_sourceY = m_Unit->GetPositionY();
            m_sourceZ = m_Unit->GetPositionZ();

            m_pathfinding = true;
        }
    }

    if(m_pathfinding)
    {
        LocationVector PathLocation = NavMeshInterface.BuildPath(m_Unit->GetMapId(), m_sourceX, m_sourceY, m_sourceZ, m_destinationX, m_destinationY, m_destinationZ, true);
        m_nextPosX = PathLocation.x;
        m_nextPosY = PathLocation.y;
        m_nextPosZ = PathLocation.z;

        // Found a path!
        if(m_nextPosX != m_sourceX && m_nextPosY != m_sourceY && m_nextPosX != 0.0f && m_nextPosY != 0.0f)
        {
            if(m_nextPosX == m_destinationX && m_nextPosY == m_destinationY && m_nextPosZ == m_destinationZ)
            {
                m_nextPosO = m_destinationO;
                m_moveJump = false;
                m_pathfinding = false;
                m_destinationX = m_destinationY = m_destinationZ = m_destinationO = 0.0f; // Pathfinding requires we keep our destination.
            }

            float distance = m_Unit->CalcDistance(m_nextPosX, m_nextPosY, m_nextPosZ);

            uint32 moveTime = float2int32(GetMovementTime(distance));

            m_totalMoveTime = moveTime;

            if(m_Unit->GetTypeId() == TYPEID_UNIT)
            {
                float angle = 0.0f;
                Creature* creature = TO_CREATURE(m_Unit);

                // don't move if we're well within combat range; rooted can't move neither
                if( distance < DISTANCE_TO_SMALL_TO_WALK || (creature->GetCanMove() == LIMIT_ROOT ) )
                    return; 

                // check if we're returning to our respawn location. if so, reset back to default
                // orientation
                if(creature->GetSpawnX() == m_nextPosX && creature->GetSpawnY() == m_nextPosY)
                {
                    angle = creature->GetSpawnO();
                    creature->SetOrientation(angle);
                }
                else
                {
                    // Calculate the angle to our next position
                    float dx = (float)m_nextPosX - m_Unit->GetPositionX();
                    float dy = (float)m_nextPosY - m_Unit->GetPositionY();
                    if(dy != 0.0f)
                    {
                        angle = atan2(dy, dx);
                        m_Unit->SetOrientation(angle);
                    }
                }
            }

            if(m_nextPosO == 0.0f)
                m_nextPosO = m_Unit->GetOrientation();
            SendMoveToPacket(m_nextPosX, m_nextPosY, m_nextPosZ, m_nextPosO, moveTime + UNIT_MOVEMENT_INTERPOLATE_INTERVAL, getMoveFlags());

            m_timeToMove = moveTime;
            m_timeMoved = 0;
            if(m_moveTimer == 0)
                m_moveTimer =  UNIT_MOVEMENT_INTERPOLATE_INTERVAL; // update every few msecs
            setCreatureState(MOVING);
            return;
        }
    }

    m_pathfinding = false;
    float distance = m_Unit->CalcDistance(m_destinationX, m_destinationY, m_destinationZ);

    m_nextPosX = m_destinationX;
    m_nextPosY = m_destinationY;
    m_nextPosZ = m_destinationZ;
    m_nextPosO = m_destinationO;
    m_destinationX = m_destinationY = m_destinationZ = m_destinationO = 0.0f;

    uint32 moveTime = float2int32(GetMovementTime(distance));

    m_totalMoveTime = moveTime;

    if(m_Unit->GetTypeId() == TYPEID_UNIT)
    {
        Creature* creature = TO_CREATURE(m_Unit);

        float angle = 0.0f;

        // don't move if we're well within combat range; rooted can't move neither
        if( distance < DISTANCE_TO_SMALL_TO_WALK || creature->GetCanMove() == LIMIT_ROOT )
            return;

        // check if we're returning to our respawn location. if so, reset back to default
        // orientation
        if(creature->GetSpawnX() == m_nextPosX && creature->GetSpawnY() == m_nextPosY)
        {
            angle = creature->GetSpawnO();
            creature->SetOrientation(angle);
        }
        else
        {
            // Calculate the angle to our next position
            float dx = (float)m_nextPosX - m_Unit->GetPositionX();
            float dy = (float)m_nextPosY - m_Unit->GetPositionY();
            if(dy != 0.0f)
            {
                angle = atan2(dy, dx);
                m_Unit->SetOrientation(angle);
            }
        }
    }
    if(m_nextPosO == 0.0f)
        m_nextPosO = m_Unit->GetOrientation();
    SendMoveToPacket(m_nextPosX, m_nextPosY, m_nextPosZ, m_nextPosO, moveTime + UNIT_MOVEMENT_INTERPOLATE_INTERVAL, getMoveFlags());

    m_timeMoved = 0;
    m_moveJump = false;
    m_timeToMove = moveTime;
    m_moveTimer = UNIT_MOVEMENT_INTERPOLATE_INTERVAL; // update every few msecs
    setCreatureState(MOVING);
}

void AI_Movement::SendCurrentMove(Player* plyr/*uint64 guid*/)
{
    ASSERT(m_Unit != NULL);

    if(PathMap == NULL || m_timeMoved == m_timeToMove)
        return;
    if(CurrentMoveRequired.find(plyr->GetLowGUID()) != CurrentMoveRequired.end())
        return;

    CurrentMoveRequired.insert(make_pair(plyr->GetLowGUID(), getMSTime()));
}

void AI_Movement::HandleEvade()
{
    float tdist = m_Unit->GetDistanceSq(m_returnX, m_returnY, m_returnZ);
    if(tdist <= 4.0f/*2.0*/)
    {
        SetAIState(STATE_IDLE);
        m_returnX = m_returnY = m_returnZ = m_returnO = 0.0f;
        m_moveRun = false;

        // Set health to full if they at there last location before attacking
        if(getAIType() != AITYPE_PET && !Interface->skip_reset_hp)
            m_Unit->SetUInt32Value(UNIT_FIELD_HEALTH, m_Unit->GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    }
    else
    {
        if( getCreatureState() == STOPPED )
        {
            // return to the home
            if( m_returnX == 0.0f && m_returnY == 0.0f )
            {
                m_returnX = m_Unit->GetSpawnX();
                m_returnY = m_Unit->GetSpawnY();
                m_returnZ = m_Unit->GetSpawnZ();
                m_returnO = m_Unit->GetSpawnO();
            }

            MoveTo(m_returnX, m_returnY, m_returnZ, m_returnO);
        }
    }
}

void AI_Movement::EventEnterCombat(uint32 misc1)
{
    if(misc1 == 0)
    {
        if(m_destinationX != 0.0f && m_destinationY != 0.0f && m_destinationZ != 0.0f)
            SetReturnPos(m_destinationX, m_destinationY, m_destinationZ, m_destinationO);
        else
            SetReturnPos(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ(), m_Unit->GetOrientation());
    }

    m_moveRun = true; //run to the target
}

void AI_Movement::_CalcDestinationAndMove(Unit* target, float dist)
{
    ASSERT(m_Unit != NULL);
    if(!getCanMove() || m_Unit->IsStunned())
    {
        StopMovement(0); //Just Stop
        return;
    }

    if(PathMap != NULL)
    {
        delete PathMap->InternalMap;
        PathMap->InternalMap = NULL;
        delete PathMap;
        PathMap = NULL;
    }

    if(target && target->IsUnit())
    {
        float ResX = target->GetPositionX();
        float ResY = target->GetPositionY();
        float ResZ = target->GetPositionZ();

        float angle = m_FollowAngle == 0.0f ? m_Unit->calcAngle(m_Unit->GetPositionX(), m_Unit->GetPositionY(), ResX, ResY) * float(M_PI) / 180.0f : m_FollowAngle;
        float x = dist * cosf(angle);
        float y = dist * sinf(angle);

        if(target->IsPlayer() && TO_PLAYER( target )->m_isMoving )
        {
            // cater for moving player vector based on orientation
            x -= cosf(target->GetOrientation());
            y -= sinf(target->GetOrientation());
        }

        SetDestPos(ResX - x, ResY - y, ResZ);
    }
    else
    {
        target = NULLUNIT;
        SetDestPos(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ());
    }

    float dx = m_destinationX - m_Unit->GetPositionX();
    float dy = m_destinationY - m_Unit->GetPositionY();
    if(dx != 0.0f || dy != 0.0f)
    {
        float angle = atan2(dx, dy);
        m_Unit->SetOrientation(angle);
    }

    m_ignorePathMap = true;
    if(getCreatureState() != MOVING)
        UpdateMove();
}

float AI_Movement::GetMovementTime(float distance)
{
    if(m_moveJump)
        return m_flySpeed ? (distance/m_flySpeed) : 0xFFFFFFFF;
    else if (m_moveFly)
        return m_flySpeed ? (distance/m_flySpeed) : 0xFFFFFFFF;
    else if (m_moveRun)
        return m_runSpeed ? (distance/m_runSpeed) : 0xFFFFFFFF;
    else
        return m_walkSpeed ? (distance/m_walkSpeed) : 0xFFFFFFFF;
}

bool AI_Movement::IsFlying()
{
    ASSERT(m_Unit != NULL);

    if(m_moveFly)
        return true;

    if( m_Unit->IsPlayer() )
        return TO_PLAYER( m_Unit )->FlyCheat;

    return false;
}

AIType AI_Movement::getAIType()
{
    return Interface->getAIType();
}

void AI_Movement::SetAIType(AIType newtype)
{
    Interface->SetAIType(newtype);
}

uint8 AI_Movement::getAIState()
{
    return Interface->getAIState();
}

void AI_Movement::SetAIState(AI_State newstate)
{
    Interface->SetAIState(newstate);
}

CreatureState AI_Movement::getCreatureState()
{
    return Interface->getCreatureState();
}

void AI_Movement::setCreatureState(CreatureState state)
{
    Interface->setCreatureState(state);
}
