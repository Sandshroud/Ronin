/***
 * Demonstrike Core
 */

#include "StdAfx.h"

bool AI_Movement::addWayPoint(WayPoint* wp)
{
    ASSERT(m_Unit != NULL);

    if(m_waypoints == NULL)
        m_waypoints = new WayPointMap;

    if(!wp)
        return false;
    if(wp->id <= 0)
        return false; //not valid id

    if(m_waypoints->size() < wp->id)
        m_waypoints->resize(wp->id);

    if((*m_waypoints)[wp->id-1] == NULL)
    {
        (*m_waypoints)[wp->id-1] = wp;
        return true;
    }
    return false;
}

void AI_Movement::changeWayPointID(uint32 oldwpid, uint32 newwpid)
{
    ASSERT(m_Unit != NULL);

    if(!m_waypoints)return;
    if(newwpid <= 0)
        return; //not valid id
    if(newwpid > m_waypoints->size())
        return; //not valid id
    if(oldwpid > m_waypoints->size())
        return;

    if(newwpid == oldwpid)
        return; //same spot

    //already wp with that id ?
    WayPoint* originalwp = getWayPoint(newwpid);
    if(!originalwp)
        return;
    WayPoint* oldwp = getWayPoint(oldwpid);
    if(!oldwp)
        return;

    oldwp->id = newwpid;
    originalwp->id = oldwpid;
    (*m_waypoints)[oldwp->id-1] = oldwp;
    (*m_waypoints)[originalwp->id-1] = originalwp;

    //SaveAll to db
    saveWayPoints();
}

void AI_Movement::deleteWayPoint(uint32 wpid)
{
    ASSERT(m_Unit != NULL);

    if(!m_waypoints || wpid <= 0 || wpid > m_waypoints->size())
        return; //not valid id

    uint32 newpid = 1;
    WayPointMap new_waypoints;
    WayPointMap::iterator itr,it2;
    for(itr = m_waypoints->begin(); itr != m_waypoints->end();)
    {
        it2 = itr++;
        if((*it2) == NULL || (*it2)->id == wpid)
        {
            delete (*it2);
            continue;
        }
        new_waypoints.push_back(*it2);
    }

    m_waypoints->clear();
    for(WayPointMap::iterator itr = new_waypoints.begin(); itr != new_waypoints.end(); itr++)
    {
        (*itr)->id = newpid++;
        m_waypoints->push_back(*itr);
    }

    saveWayPoints();
}

bool AI_Movement::showWayPoints(Player* pPlayer, bool Backwards)
{
    ASSERT(m_Unit != NULL);

    if(!m_waypoints)
        return false;

    //wpid of 0 == all
    WayPointMap::const_iterator itr;
    if(m_WayPointsShowing == true)
        return false;

    m_WayPointsShowing = true;

    WayPoint* wp = NULL;
    for (itr = m_waypoints->begin(); itr != m_waypoints->end(); itr++)
    {
        if( (*itr) != NULL )
        {
            wp = *itr;

            //Create
            Creature* pWayPoint = NULLCREATURE;
            pWayPoint = new Creature((uint64)HIGHGUID_TYPE_WAYPOINT << 32 | wp->id);
            pWayPoint->Init();
            pWayPoint->CreateWayPoint(wp->id, pPlayer->GetMapId(), wp->x, wp->y, wp->z, wp->orientation);
            pWayPoint->SetUInt32Value(OBJECT_FIELD_ENTRY, 300000);
            pWayPoint->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5f);

            bool ModelChange = false;
            if(!Backwards)
            {
                if(wp->forwardInfo)
                {
                    uint32 DisplayID = (wp->forwardInfo->SkinID == 0) ? m_Unit->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID) : wp->forwardInfo->SkinID;
                    if(DisplayID != m_Unit->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID))
                        ModelChange = true;

                    pWayPoint->SetUInt32Value(UNIT_FIELD_DISPLAYID, DisplayID);
                    pWayPoint->SetUInt32Value(UNIT_NPC_EMOTESTATE, wp->forwardInfo->EmoteID);
                    pWayPoint->SetStandState(wp->forwardInfo->StandState);
                }
            }
            else
            {
                if(wp->backwardInfo)
                {
                    uint32 DisplayID = (wp->backwardInfo->SkinID == 0) ? m_Unit->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID) : wp->backwardInfo->SkinID;
                    if(DisplayID != m_Unit->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID))
                        ModelChange = true;

                    pWayPoint->SetUInt32Value(UNIT_FIELD_DISPLAYID, DisplayID);
                    pWayPoint->SetUInt32Value(UNIT_NPC_EMOTESTATE, wp->backwardInfo->EmoteID);
                    pWayPoint->SetStandState(wp->backwardInfo->StandState);
                }
            }
            if(ModelChange)
                pWayPoint->EventModelChange();

            pWayPoint->SetUInt32Value(UNIT_FIELD_LEVEL, wp->id);
            pWayPoint->SetUInt32Value(UNIT_NPC_FLAGS, 0);
//          pWayPoint->SetUInt32Value(UNIT_FIELD_AURA+32, 8326); //invisable & deathworld look
            pWayPoint->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE , pPlayer->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
            pWayPoint->SetUInt32Value(UNIT_FIELD_HEALTH, 1);
            pWayPoint->SetUInt32Value(UNIT_FIELD_MAXHEALTH, 1);
            pWayPoint->SetUInt32Value(UNIT_FIELD_STAT0, wp->flags);

            //Create on client
            ByteBuffer buf(2500);
            uint32 count = pWayPoint->BuildCreateUpdateBlockForPlayer(&buf, pPlayer);
            pPlayer->PushUpdateData(&buf, count);

            //root the object
            WorldPacket data1;
            data1.Initialize(SMSG_FORCE_MOVE_ROOT);
            data1 << pWayPoint->GetNewGUID();
            pPlayer->GetSession()->SendPacket( &data1 );

            //Cleanup
            delete pWayPoint;
            pWayPoint = NULL;
        }
    }
    return true;
}

bool AI_Movement::hideWayPoints(Player* pPlayer)
{
    ASSERT(m_Unit != NULL);

    if(!m_waypoints)
        return false;

    //wpid of 0 == all
    if(m_WayPointsShowing != true) return false;
    m_WayPointsShowing = false;
    WayPointMap::const_iterator itr;

    // slightly better way to do this
    uint64 guid;

    for (itr = m_waypoints->begin(); itr != m_waypoints->end(); itr++)
    {
        if( (*itr) != NULL )
        {
            // avoid C4293
            guid = ((uint64)HIGHGUID_TYPE_WAYPOINT << 32) | (*itr)->id;
            WoWGuid wowguid(guid);
            pPlayer->PushOutOfRange(wowguid);
        }
    }
    return true;
}

bool AI_Movement::saveWayPoints()
{
    ASSERT(m_Unit != NULL);
    if(m_waypoints == NULL)
        return false;
    if(!m_Unit->IsCreature())
        return false;
    if(TO_CREATURE(m_Unit)->m_spawn == NULL)
        return false;

    WorldDatabase.Execute("DELETE FROM creature_waypoints WHERE spawnid = %u", TO_CREATURE(m_Unit)->GetSQL_id());
    WayPointMap::const_iterator itr;
    WayPoint* wp = NULL;
    std::stringstream ss;

    for (itr = m_waypoints->begin(); itr != m_waypoints->end();)
    {
        wp = (*itr);
        ++itr;
        //Save
        ss.str("");
        ss << "REPLACE INTO creature_waypoints ";
        ss << "(spawnid,waypointid,position_x,position_y,position_z,orientation,waittime,flags,forwardemoteoneshot,forwardemoteid,backwardemoteoneshot,backwardemoteid,forwardskinid,backwardskinid,forwardStandState,backwardStandState,forwardSpellToCast,backwardSpellToCast,forwardSayText,backwardSayText) VALUES (";
        ss << TO_CREATURE(m_Unit)->GetSQL_id() << ", ";
        ss << wp->id << ", ";
        ss << wp->x << ", ";
        ss << wp->y << ", ";
        ss << wp->z << ", ";
        ss << wp->orientation << ", ";
        ss << wp->waittime << ", ";
        ss << wp->flags << ", ";
        ss << uint32(wp->forwardInfo ? (wp->forwardInfo->EmoteOneShot ? 1 : 0) : 0) << ", ";
        ss << uint32(wp->forwardInfo ? wp->forwardInfo->EmoteID : 0) << ", ";
        ss << uint32(wp->backwardInfo ? (wp->backwardInfo->EmoteOneShot ? 1 : 0) : 0) << ", ";
        ss << uint32(wp->backwardInfo ? wp->backwardInfo->EmoteID : 0) << ", ";
        ss << uint32(wp->forwardInfo ? wp->forwardInfo->SkinID : 0) << ", ";
        ss << uint32(wp->backwardInfo ? wp->backwardInfo->SkinID : 0) << ", ";
        ss << uint32(wp->forwardInfo ? wp->forwardInfo->StandState : 0) << ", ";
        ss << uint32(wp->backwardInfo ? wp->backwardInfo->StandState : 0) << ", ";
        ss << uint32(wp->forwardInfo ? wp->forwardInfo->SpellToCast : 0) << ", ";
        ss << uint32(wp->backwardInfo ? wp->backwardInfo->SpellToCast : 0) << ",'";
        ss << WorldDatabase.EscapeString(wp->forwardInfo ? wp->forwardInfo->SayText : "") << "','";
        ss << WorldDatabase.EscapeString(wp->backwardInfo ? wp->backwardInfo->SayText : "") << "')\0";
        WorldDatabase.Execute(ss.str().c_str());
    }
    return true;
}

void AI_Movement::deleteAllWaypoints()
{
    if(!m_waypoints)
        return;

    for(WayPointMap::iterator itr = m_waypoints->begin(); itr != m_waypoints->end(); itr++)
    {
        if((*itr) != NULL)
            delete (*itr);
    }
    m_waypoints->clear();
}

WayPoint* AI_Movement::getWayPoint(uint32 wpid)
{
    if(!m_waypoints)
        return NULL;

    //make sure wpid is valid, return 1st/last if out of boundaries.
    wpid = uint32( wpid > m_waypoints->size() ? m_waypoints->size() : ( !wpid?  1 : wpid ));

    return m_waypoints->at(wpid-1);
}
