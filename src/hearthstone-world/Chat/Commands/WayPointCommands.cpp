/***
 * Demonstrike Core
 */

///////////////////////////////////////////////
//  Waypoint Commands
//

#include "StdAfx.h"

bool ChatHandler::HandleWPAddCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No Selection");
        return true;
    }
    AIInterface* ai = NULL;
    Creature* pCreature = NULLCREATURE;
    Player* p = m_session->GetPlayer();
    if(p->waypointunit != NULL)
    {
        SystemMessage(m_session, "Using Previous Unit.");
        ai = p->waypointunit;
        if(!ai)
        {
            SystemMessage(m_session, "Invalid Creature, please select another one.");
            return true;
        }

        pCreature = TO_CREATURE(ai->GetUnit());
        if(!pCreature)
        {
            SystemMessage(m_session, "Invalid Creature, please select another one.");
            return true;
        }
    }
    else
    {
        pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
        if(!pCreature)
        {
            SystemMessage(m_session, "You should select a creature.");
            return true;
        }

        if( pCreature->m_spawn == NULL )
        {
            SystemMessage(m_session, "You cannot add waypoints to a creature that is not saved.");
            return true;
        }

        ai = pCreature->GetAIInterface();
    }

    char* pWaitTime = strtok((char*)args, " ");
    uint32 WaitTime = (pWaitTime)? atoi(pWaitTime) : 10000;
    char* pFlags = strtok(NULL, " ");
    uint32 Flags = (pFlags)? atoi(pFlags) : 0 ;
    char* pForwardEmoteId = strtok(NULL, " ");
    uint32 ForwardEmoteId = (pForwardEmoteId)? atoi(pForwardEmoteId) : 0;
    char* pBackwardEmoteId = strtok(NULL, " ");
    uint32 BackwardEmoteId = (pBackwardEmoteId)? atoi(pBackwardEmoteId) : 0;
    char* pForwardSkinId = strtok(NULL, " ");
    uint32 ForwardSkinId = (pForwardSkinId)? atoi(pForwardSkinId) : pCreature->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    char* pBackwardSkinId = strtok(NULL, " ");
    uint32 BackwardSkinId = (pBackwardSkinId)? atoi(pBackwardSkinId) : pCreature->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    char* pForwardEmoteOneShot = strtok(NULL, " ");
    uint32 ForwardEmoteOneShot = (pForwardEmoteOneShot)? atoi(pForwardEmoteOneShot) : 1;
    char* pBackwardEmoteOneShot = strtok(NULL, " ");
    uint32 BackwardEmoteOneShot = (pBackwardEmoteOneShot)? atoi(pBackwardEmoteOneShot) : 1;
    char* pForwardStandState = strtok(NULL, " ");
    uint32 ForwardStandState = (pForwardStandState)? atoi(pForwardStandState) : 0;
    char* pBackwardStandState = strtok(NULL, " ");
    uint32 BackwardStandState = (pBackwardStandState)? atoi(pBackwardStandState) : 0;
    char* pForwardSpellToCast = strtok(NULL, " ");
    uint32 ForwardSpellToCast = (pForwardSpellToCast)? atoi(pForwardSpellToCast) : 0;
    char* pBackwardSpellToCast = strtok(NULL, " ");
    uint32 BackwardSpellToCast = (pBackwardSpellToCast)? atoi(pBackwardSpellToCast) : 0;

    WayPoint* wp = new WayPoint;
    bool showing = ai->WayPointsShowing();
    wp->id = (uint32)ai->GetWayPointsCount()+1;
    wp->x = p->GetPositionX();
    wp->y = p->GetPositionY();
    wp->z = p->GetPositionZ();
    wp->orientation = p->GetOrientation();
    wp->waittime = WaitTime;
    wp->flags = Flags;

    wp->forwardInfo = new ConditionalData(((ForwardEmoteOneShot > 0) ? true : false), ForwardEmoteId, ForwardSkinId, (ForwardStandState > 8 ? 0 : ForwardStandState), ForwardSpellToCast, "");
    if(wp->forwardInfo->EmoteID == 0
        && wp->forwardInfo->SkinID == 0
        && wp->forwardInfo->StandState == 0
        && wp->forwardInfo->SpellToCast == 0
        && wp->forwardInfo->SayText.length() == 0)
    {
        delete wp->forwardInfo;
        wp->forwardInfo = NULL;
    }

    wp->backwardInfo = new ConditionalData(((BackwardEmoteOneShot > 0) ? true : false), BackwardEmoteId, BackwardSkinId, (BackwardStandState > 8 ? 0 : BackwardStandState), BackwardSpellToCast, "");
    if(wp->backwardInfo->EmoteID == 0
        && wp->backwardInfo->SkinID == 0
        && wp->backwardInfo->StandState == 0
        && wp->backwardInfo->SpellToCast == 0
        && wp->backwardInfo->SayText.length() == 0)
    {
        delete wp->backwardInfo;
        wp->backwardInfo = NULL;
    }

    if(showing)
        ai->hideWayPoints(p);
    ai->addWayPoint(wp);
    ai->saveWayPoints();

    if(showing)
        ai->showWayPoints(p, ai->WayPointsShowingBackwards());

    SystemMessage(m_session, "Waypoint %u added.", wp->id);
    return true;
}

bool ChatHandler::HandleWPMoveTypeCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    uint32 option = atoi((char*)args);

    if (option != 0 && option != 1 && option != 2)
    {
        std::stringstream ss;
        ss << "Incorrect value." << endl;
        ss << "0 is Move from WP 1 ->  10 then 10 -> 1." << endl;
        ss << "1 is Move from WP to a random WP." << endl;
        ss << "2 is Move from WP 1 -> 10 then 1 -> 10." << endl;
        SendMultilineMessage(m_session, ss.str().c_str());
        return true;
    }

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    if( pCreature->m_spawn == NULL )
    {
        SystemMessage(m_session, "You cannot add waypoints to a creature that is not saved.");
        return true;
    }

    char sql[512];
    snprintf(sql, 512, "UPDATE creature_spawns SET movetype = '%u' WHERE id = '%u'", (unsigned int)option, (unsigned int)pCreature-> GetSQL_id());
    WorldDatabase.Execute( sql );

    pCreature->GetAIInterface()->setMoveType(option);

    SystemMessage(m_session, "Value saved to database.");
    return true;
}


bool ChatHandler::HandleWPShowCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a Creature.");
        return true;
    }

    if( pCreature->m_spawn == NULL )
    {
        SystemMessage(m_session, "You cannot add waypoints to a creature that is not saved.");
        return true;
    }

    char* pBackwards = strtok((char*)args, " ");
    bool Backwards = (pBackwards)? ((atoi(pBackwards)>0)?true:false) : false;

    AIInterface* ai = pCreature->GetAIInterface();
    Player* pPlayer = m_session->GetPlayer();


    if(pPlayer->waypointunit != ai)
    {
        if(ai->WayPointsShowing() == true)
        {
            SystemMessage(m_session, "Some one else is also Viewing this Creatures WayPoints.");
            SystemMessage(m_session, "Viewing WayPoints at the same time as some one else can cause undesireble results.");
            return true;
        }

        if(pPlayer->waypointunit != NULL)
        {
            pPlayer->waypointunit->hideWayPoints(pPlayer);
        }
        pPlayer->waypointunit = ai;
        ai->showWayPoints(pPlayer,Backwards);
        ai->SetWayPointsShowingBackwards(Backwards);
    }
    else
    {
        if(ai->WayPointsShowing() == true)
        {
            SystemMessage(m_session, "Waypoints Already Showing.");
        }
        else
            ai->showWayPoints(m_session->GetPlayer(),Backwards);
    }

    SystemMessage(m_session, "Showing waypoints for creature %u", pCreature->GetSQL_id());
    return true;
}

bool ChatHandler::HandleWPDeleteCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        //Refresh client
        //Hide all
        bool show = ai->WayPointsShowing();
        if(show == true)
            ai->hideWayPoints(pPlayer);

        ai->deleteWayPoint(wpid);

        //Show All again after delete
        if(show == true)
            ai->showWayPoints(pPlayer, ai->WayPointsShowingBackwards());

        SystemMessage(m_session, "Waypoint %u deleted.", wpid);
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWPChangeNoCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    std::stringstream ss;

    //get newid
    char* pNewID = strtok((char*)args, " ");
    uint32 NewID = (pNewID)? atoi(pNewID) : 0;

    uint32 wpid = GUID_LOPART(guid);
    if(NewID == wpid) return false;
    if(wpid)
    {
        //Refresh client
        //Hide all

        bool show = ai->WayPointsShowing();
        if(show == true)
            ai->hideWayPoints(pPlayer);

        //update to new id
        ai->changeWayPointID(wpid, NewID);

        //Show All again after update
        if(show == true)
            ai->showWayPoints(pPlayer,ai->WayPointsShowingBackwards());

        ss << "Waypoint " << wpid << " changed to Waypoint " << NewID << ".";
        SystemMessage(m_session, ss.str().c_str());
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWPFlagsCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        uint32 flags = wp->flags;

        char* pNewFlags = strtok((char*)args, " ");
        uint32 NewFlags = (pNewFlags)? atoi(pNewFlags) : 0;

        wp->flags = NewFlags;

        //save wp
        ai->saveWayPoints();

        ss << "Waypoint " << wpid << " flags changed from " << flags << " to " << NewFlags;
        SystemMessage(m_session, ss.str().c_str());
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWPMoveHereCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            wp->x = pPlayer->GetPositionX();
            wp->y = pPlayer->GetPositionY();
            wp->z = pPlayer->GetPositionZ();
            wp->orientation = pPlayer->GetOrientation();

            //save wp
            ai->saveWayPoints();
        }
        //Refresh client
        if(ai->WayPointsShowing() == true)
        {
            ai->hideWayPoints(pPlayer);
            ai->showWayPoints(pPlayer,ai->WayPointsShowingBackwards());
        }

        ss << "Waypoint " << wpid << " has been moved.";
        SystemMessage(m_session, ss.str().c_str());
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWPWaitCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }

    uint32 Wait = 10000;
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            char* pWait = strtok((char*)args, " ");
            Wait = (pWait)? atoi(pWait) : 10000;

            if(Wait < 5000)
            {
                SystemMessage(m_session, "A Wait Time of less then 5000ms can cause lag, consider extending it.");
            }
            wp->waittime = Wait;

            //save wp
            ai->saveWayPoints();
        }

        ss << "Wait Time for Waypoint " << wpid << " is now " << Wait << "ms.";
        SystemMessage(m_session, ss.str().c_str());
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWaypointGettextCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if((wpid > 0) && (wpid <= ai->GetWayPointsCount()))
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            ss << "Waypoint Number " << wp->id << ":\n";
            if(wp->backwardInfo)
            {
                ss << "Backward:\n";
                ss << wp->backwardInfo->SayText << "\n";
            }
            if(wp->forwardInfo)
            {
                ss << "Forward:\n";
                ss << wp->forwardInfo->SayText << "\n";
            }
            SendMultilineMessage(m_session, ss.str().c_str());
        }
    }
    else
    {
       SystemMessage(m_session,  "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWaypointForwardTextCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            char pAnnounce[1024];
            snprintf(pAnnounce, 1024, "%s", args);

            if(wp->forwardInfo == NULL)
                wp->forwardInfo = new ConditionalData();
            wp->forwardInfo->SayText = ((const char*)(pAnnounce));
            ss << "Forward SayText for Waypoint " << wpid << " is now " << string(pAnnounce);

            //save wp
            ai->saveWayPoints();
        }

        SystemMessage(m_session,  ss.str().c_str());
    }   else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWaypointSetOrientationCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }

    bool showing = ai->WayPointsShowing();
    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            if(showing)
                ai->hideWayPoints(pPlayer);

            wp->orientation = pPlayer->GetOrientation();
            //save wp
            ai->saveWayPoints();
        }
        else return false;
    }
    else return false;

    if(showing)
        ai->showWayPoints(pPlayer, ai->WayPointsShowingBackwards());
    return true;
}

bool ChatHandler::HandleWaypointBackwardTextCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            char pAnnounce[1024];
            snprintf(pAnnounce, 1024, "%s", args);

            if(wp->backwardInfo == NULL)
                wp->backwardInfo = new ConditionalData();
            wp->backwardInfo->SayText = ((const char*)(pAnnounce));
            ss << "Backward SayText for Waypoint " << wpid << " is now " << string(pAnnounce);
            //save wp
            ai->saveWayPoints();
        }

        SystemMessage(m_session,  ss.str().c_str());
    }   else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }
    return true;
}
bool ChatHandler::HandleWPSpellToCastCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        uint32 SpellToCast = 0;
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            char* pBackwards = strtok((char*)args, " ");
            uint32 Backwards = (pBackwards)? atoi(pBackwards) : 0;
            char* pSpellToCast = strtok((char*)args, " ");
            SpellToCast = (pSpellToCast)? atoi(pSpellToCast) : 0;

            if(Backwards)
            {
                if(wp->backwardInfo == NULL)
                    wp->backwardInfo = new ConditionalData();
                wp->backwardInfo->SpellToCast = SpellToCast;
                ss << "Backward SpellToCast for Waypoint " << wpid << " is now " << SpellToCast;
            }
            else
            {
                if(wp->forwardInfo == NULL)
                    wp->forwardInfo = new ConditionalData();
                wp->forwardInfo->SpellToCast = SpellToCast;
                ss << "Forward SpellToCast for Waypoint " << wpid << " is now " << SpellToCast;
            }
            //save wp
            ai->saveWayPoints();
        }

        SystemMessage(m_session,  ss.str().c_str());
    }   else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWPStandStateCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    uint32 StandState = 0;
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            char* pBackwards = strtok((char*)args, " ");
            uint32 Backwards = (pBackwards)? atoi(pBackwards) : 0;
            char* pStandState = strtok(NULL, " ");
            StandState = (pStandState)? atoi(pStandState) : 0;
            StandState = StandState > 8 ? 0 : StandState;
            if(Backwards)
            {
                if(wp->backwardInfo == NULL)
                    wp->backwardInfo = new ConditionalData();
                wp->backwardInfo->StandState = StandState;
                ss << "Backward StandState for Waypoint " << wpid << " is now " << StandState;
            }
            else
            {
                if(wp->forwardInfo == NULL)
                    wp->forwardInfo = new ConditionalData();
                wp->forwardInfo->StandState = StandState;
                ss << "Forward StandState for Waypoint " << wpid << " is now " << StandState;
            }
            //save wp
            ai->saveWayPoints();
        }

        SystemMessage(m_session,  ss.str().c_str());
    }
    else
        SystemMessage(m_session, "Invalid Waypoint.");
    return true;
}

bool ChatHandler::HandleWPEmoteCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    uint32 EmoteId = 0;
    bool OneShot = true;
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            char* pBackwards = strtok((char*)args, " ");
            uint32 Backwards = (pBackwards)? atoi(pBackwards) : 0;
            char* pEmoteId = strtok(NULL, " ");
            EmoteId = (pEmoteId)? atoi(pEmoteId) : 0;
            char* pOneShot = strtok(NULL, " ");
            OneShot = (pOneShot)? ((atoi(pOneShot)>0)?true:false) : 1;
            if(Backwards)
            {
                if(wp->backwardInfo == NULL)
                    wp->backwardInfo = new ConditionalData();
                wp->backwardInfo->EmoteID = EmoteId;
                wp->backwardInfo->EmoteOneShot = OneShot;
            }
            else
            {
                if(wp->forwardInfo == NULL)
                    wp->forwardInfo = new ConditionalData();
                wp->forwardInfo->EmoteID = EmoteId;
                wp->forwardInfo->EmoteOneShot = OneShot;
            }

            //save wp
            ai->saveWayPoints();
        }

        ss << "EmoteID for Waypoint " << wpid << " is now " << EmoteId << " and oneshot is " << ((OneShot == true)? "Enabled." : "Disabled.");
        SystemMessage(m_session,  ss.str().c_str());
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWPSkinCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session,  "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session,  "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    uint32 SkinId = 0;
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if(wpid)
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            char* pBackwards = strtok((char*)args, " ");
            uint32 Backwards = (pBackwards)? atoi(pBackwards) : 0;
            char* pSkinId = strtok(NULL, " ");
            SkinId = (pSkinId)? atoi(pSkinId) : 0;
            if(Backwards)
            {
                if(wp->backwardInfo == NULL)
                    wp->backwardInfo = new ConditionalData();
                wp->backwardInfo->SkinID = SkinId;
            }
            else
            {
                if(wp->forwardInfo == NULL)
                    wp->forwardInfo = new ConditionalData();
                wp->forwardInfo->SkinID = SkinId;
            }

            //save wp
            ai->saveWayPoints();
        }

        ss << "SkinID for Waypoint " << wpid << " is now " << SkinId;
        SystemMessage(m_session, ss.str().c_str());
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWPInfoCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    if(GUID_HIPART(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return true;
    }

    Player* pPlayer = m_session->GetPlayer();
    AIInterface* ai = pPlayer->waypointunit;
    if(!ai || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    std::stringstream ss;

    uint32 wpid = GUID_LOPART(guid);
    if((wpid > 0) && (wpid <= ai->GetWayPointsCount()))
    {
        WayPoint* wp = ai->getWayPoint(wpid);
        if(wp)
        {
            ss << "Waypoint Number " << wp->id << ":\n";
            ss << "WaitTime: " << wp->waittime << "\n";
            ss << "Flags: " << wp->flags;
            if(wp->flags == 768)
                ss << " (Fly)\n";
            else if(wp->flags == 256)
                ss << " (Run)\n";
            else
                ss << " (Walk)\n";

            if(wp->forwardInfo)
            {
                ss << "Forward\n";
                ss << "EmoteId    : " << wp->forwardInfo->EmoteID << "\n";
                ss << "OneShot    : " << (wp->forwardInfo->EmoteOneShot ? "Yes" : "No") << "\n";
                ss << "SkinId     : " << wp->forwardInfo->SkinID << "\n";
                ss << "StandState : " << wp->forwardInfo->StandState << "\n";
                ss << "SpellToCast: " << wp->forwardInfo->SpellToCast << "\n";
            }

            if(wp->backwardInfo)
            {
                ss << "Backward\n";
                ss << "EmoteId    : " << wp->backwardInfo->EmoteID << "\n";
                ss << "OneShot    : " << (wp->backwardInfo->EmoteOneShot ? "Yes" : "No") << "\n";
                ss << "SkinId     : " << wp->backwardInfo->SkinID << "\n";
                ss << "StandState : " << wp->backwardInfo->StandState << "\n";
                ss << "SpellToCast: " << wp->backwardInfo->SpellToCast << "\n";
            }
            SendMultilineMessage(m_session, ss.str().c_str());
        }
    }
    else
    {
       SystemMessage(m_session,  "Invalid Waypoint.");
        return true;
    }
    return true;
}

bool ChatHandler::HandleWPHideCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a Creature.");
        return true;
    }

    if( pCreature->m_spawn == NULL )
    {
        SystemMessage(m_session, "You cannot add waypoints to a creature that is not saved.");
        return true;
    }

    AIInterface* ai = pCreature->GetAIInterface();
    Player* pPlayer = m_session->GetPlayer();


    if(pPlayer->waypointunit == ai)
    {
        if(ai->WayPointsShowing() == true)
            pPlayer->waypointunit->hideWayPoints(pPlayer);

        pPlayer->waypointunit = NULL;
    }
    else
    {
        SystemMessage(m_session, "Waypoints for that Unit are not Visible.");
        return true;
    }

    std::stringstream ss;
    ss << "Hiding Waypoints for " << pCreature->GetSQL_id();
    SystemMessage(m_session, ss.str().c_str());

    return true;
}

bool ChatHandler::HandleGenerateWaypoints(const char* args, WorldSession * m_session)
{
    Creature* cr = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(m_session->GetPlayer()->GetSelection()));
    if(!cr)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    if( cr->m_spawn == NULL )
    {
        SystemMessage(m_session, "You cannot add waypoints to a creature that is not saved.");
        return true;
    }

    if(cr->GetAIInterface() == NULL)
    {
        SystemMessage(m_session, "Creature was not initialized correctly.");
        return true;
    }

    if(cr->GetAIInterface()->GetWayPointsCount())//ALREADY HAVE WAYPOINTS
    {
        SystemMessage(m_session, "The creature already has waypoints");
        return false;
    }

    if(m_session->GetPlayer()->waypointunit != NULL)
    {
        SystemMessage(m_session, "You are already showing waypoints, hide them first.");
        return true;
    }

    if(!cr->GetSQL_id())
    {
        SystemMessage(m_session, "This creature did not get a valid spawn_id.");
        return true;
    }

    uint32 r;
    uint32 n;
    if(sscanf(args, "%u %u", &r, &n) != 2)
    {
        SystemMessage(m_session, "Randomly generate wps params: range count");
        return true;
    }

    for(uint32 i = 0; i < n; i++)
    {
        float ang = rand()/100.0f;
        float ran = (RandomUInt(r*10))/10.0f;
        while(ran < 1)
        {
            ran = (RandomUInt(r*10))/10.0f;
        }

        float x = cr->GetPositionX()+ran*sin(ang);
        float y = cr->GetPositionY()+ran*cos(ang);
        float z = cr->GetCHeightForPosition(true, x, y, cr->GetPositionZ());

        WayPoint* wp = new WayPoint;
        wp->id = (uint32)cr->GetAIInterface()->GetWayPointsCount()+1;
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = 0.0f;
        wp->waittime = 5000;
        wp->flags = 0;
        wp->forwardInfo = NULL;
        wp->backwardInfo = NULL;
        cr->GetAIInterface()->addWayPoint(wp);
    }

    cr->GetAIInterface()->setMoveType(1);
    m_session->GetPlayer()->waypointunit = cr->GetAIInterface();
    cr->GetAIInterface()->showWayPoints(m_session->GetPlayer(),cr->GetAIInterface()->WayPointsShowingBackwards());
    return true;
}

bool ChatHandler::HandleSaveWaypoints(const char* args, WorldSession * m_session)
{
    Creature* cr = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(m_session->GetPlayer()->GetSelection()));
    if(cr == NULL)
        return false;
    if(!cr->GetSQL_id())
        return false;

    if(cr->GetAIInterface()->saveWayPoints())
    {
        SystemMessage(m_session, "Waypoints saved!");
        return true;
    }
    return false;
}


bool ChatHandler::HandleDeleteWaypoints(const char* args, WorldSession * m_session)
{
    Creature* cr =
        m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(m_session->GetPlayer()->GetSelection()));
    if(!cr)return false;
    if(!cr->GetSQL_id())
        return false;

    if(cr->GetAIInterface()->WayPointsShowing())
    {
        SystemMessage(m_session, "Waypoints are showing, hide them first.");
        return true;
    }

    WorldDatabase.Execute("DELETE FROM creature_waypoints WHERE spawnid=%u",cr->GetSQL_id());

    cr->GetAIInterface()->deleteAllWaypoints();
    SystemMessage(m_session, "Deleted waypoints for %u", cr->GetSQL_id());
    return true;
}

bool ChatHandler::HandleWaypointAddFlyCommand(const char * args, WorldSession * m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No Selection");
        return true;
    }
    AIInterface* ai = NULL;
    Creature* pCreature = NULLCREATURE;
    Player* p = m_session->GetPlayer();
    if(p->waypointunit != NULL)
    {
        SystemMessage(m_session, "Using Previous Unit.");
        ai = p->waypointunit;
        if(!ai)
        {
            SystemMessage(m_session, "Invalid Creature, please select another one.");
            return true;
        }

        pCreature = TO_CREATURE(ai->GetUnit());
        if(!pCreature)
        {
            SystemMessage(m_session, "Invalid Creature, please select another one.");
            return true;
        }
    }
    else
    {
        pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
        if(!pCreature)
        {
            SystemMessage(m_session, "You should select a creature.");
            return true;
        }
        ai = pCreature->GetAIInterface();
    }

    char* pWaitTime = strtok((char*)args, " ");
    uint32 WaitTime = (pWaitTime)? atoi(pWaitTime) : 0;
    char* pForwardEmoteId = strtok(NULL, " ");
    uint32 ForwardEmoteId = (pForwardEmoteId)? atoi(pForwardEmoteId) : 0;
    char* pBackwardEmoteId = strtok(NULL, " ");
    uint32 BackwardEmoteId = (pBackwardEmoteId)? atoi(pBackwardEmoteId) : 0;
    char* pForwardSkinId = strtok(NULL, " ");
    uint32 ForwardSkinId = (pForwardSkinId)? atoi(pForwardSkinId) : pCreature->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    char* pBackwardSkinId = strtok(NULL, " ");
    uint32 BackwardSkinId = (pBackwardSkinId)? atoi(pBackwardSkinId) : pCreature->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    char* pForwardEmoteOneShot = strtok(NULL, " ");
    uint32 ForwardEmoteOneShot = (pForwardEmoteOneShot)? atoi(pForwardEmoteOneShot) : 1;
    char* pBackwardEmoteOneShot = strtok(NULL, " ");
    uint32 BackwardEmoteOneShot = (pBackwardEmoteOneShot)? atoi(pBackwardEmoteOneShot) : 1;
    char* pForwardStandState = strtok(NULL, " ");
    uint32 ForwardStandState = (pForwardStandState)? atoi(pForwardStandState) : 0;
    char* pBackwardStandState = strtok(NULL, " ");
    uint32 BackwardStandState = (pBackwardStandState)? atoi(pBackwardStandState) : 0;
    char* pForwardSpellToCast = strtok(NULL, " ");
    uint32 ForwardSpellToCast = (pForwardSpellToCast)? atoi(pForwardSpellToCast) : 0;
    char* pBackwardSpellToCast = strtok(NULL, " ");
    uint32 BackwardSpellToCast = (pBackwardSpellToCast)? atoi(pBackwardSpellToCast) : 0;

    WayPoint* wp = new WayPoint;
    bool showing = ai->WayPointsShowing();
    wp->id = (uint32)ai->GetWayPointsCount()+1;
    wp->x = p->GetPositionX();
    wp->y = p->GetPositionY();
    wp->z = p->GetPositionZ();
    wp->orientation = p->GetOrientation();
    wp->waittime = WaitTime;
    wp->flags = 768;
    wp->forwardInfo = new ConditionalData(((ForwardEmoteOneShot > 0) ? true : false), ForwardEmoteId, ForwardSkinId, (ForwardStandState > 8 ? 0 : ForwardStandState), ForwardSpellToCast, "");
    if(wp->forwardInfo->EmoteID == 0
        && wp->forwardInfo->SkinID == 0
        && wp->forwardInfo->StandState == 0
        && wp->forwardInfo->SpellToCast == 0
        && wp->forwardInfo->SayText.length() == 0)
    {
        delete wp->forwardInfo;
        wp->forwardInfo = NULL;
    }

    wp->backwardInfo = new ConditionalData(((BackwardEmoteOneShot > 0) ? true : false), BackwardEmoteId, BackwardSkinId, (BackwardStandState > 8 ? 0 : BackwardStandState), BackwardSpellToCast, "");
    if(wp->backwardInfo->EmoteID == 0
        && wp->backwardInfo->SkinID == 0
        && wp->backwardInfo->StandState == 0
        && wp->backwardInfo->SpellToCast == 0
        && wp->backwardInfo->SayText.length() == 0)
    {
        delete wp->backwardInfo;
        wp->backwardInfo = NULL;
    }

    if(showing)
        ai->hideWayPoints(p);

    ai->addWayPoint(wp);
    ai->saveWayPoints();

    if(showing)
        ai->showWayPoints(p, ai->WayPointsShowingBackwards());

    SystemMessage(m_session, "Waypoint %u added.", wp->id);
    return true;
}

bool ChatHandler::HandleNpcSelectCommand(const char * args, WorldSession * m_session)
{
    Creature* un = NULLCREATURE;
    float dist = 999999.0f;
    float dist2;
    Player* plr = m_session->GetPlayer();
    unordered_set<Object* >::iterator itr;
    for(itr = plr->GetInRangeSetBegin(); itr != plr->GetInRangeSetEnd(); itr++)
    {
        if( (dist2 = plr->GetDistance2dSq(*itr)) < dist && (*itr)->GetTypeId() == TYPEID_UNIT )
        {
            un = TO_CREATURE(*itr);
            dist = dist2;
        }
    }

    if(!un)
    {
        SystemMessage(m_session, "No inrange creatures found.");
        return true;
    }

    plr->SetSelection(un->GetGUID());
    SystemMessage(m_session, "Set selection to "I64FMT" (%s)", un->GetGUID(), un->GetCreatureInfo() ? un->GetCreatureInfo()->Name : "Unknown");
    return true;
}
