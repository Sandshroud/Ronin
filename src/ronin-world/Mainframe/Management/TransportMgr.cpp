/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

initialiseSingleton( TransportMgr );

TransportMgr::TransportMgr() : m_transportUpdateTimer(0)
{

}

TransportMgr::~TransportMgr()
{
    while(m_transportDataStorage.size())
    {
        TransportData *tData = m_transportDataStorage.begin()->second;
        m_transportDataStorage.erase(m_transportDataStorage.begin());
        delete tData->transportPath;
        delete tData;
    }
    m_transportDataStorage.clear();

    while(m_transportStatusStorage.size())
    {
        TransportStatus *status = m_transportStatusStorage.begin()->second;
        m_transportStatusStorage.erase(m_transportStatusStorage.begin());
        delete status;
    }
    m_transportStatusStorage.clear();
}

TransportStatus::TransportStatus(uint32 entry) : _mapId(-1), _pendingMapId(-1), transportEntry(entry)
{

}

TransportStatus::~TransportStatus()
{

}

void TransportStatus::Initialize(uint32 mapId)
{
    _mapId = mapId;

}

void TransportStatus::Update()
{
    dataLock.Acquire();
    if(_mapId == -1 && _pendingMapId != -1)
    {
        _mapId = _pendingMapId;
        _pendingMapId = -1;
    }
    dataLock.Release();
}

void TransportStatus::QueueMapTransfer(uint32 oldMap, uint32 newMapId)
{
    Guard guard(dataLock);
    // Don't queue any incorrect transfers
    if(oldMap != _mapId)
        return;

    // Disable our current map
    _mapId = -1;
    // Set our pending map
    _pendingMapId = newMapId;
}

void TransportMgr::LoadTransportData()
{
    QueryResult *result = WorldDatabase.Query("SELECT entry, data0 FROM gameobject_names WHERE type = 15");
    if(result == NULL)
        return; // No usable transport data in gameobject names

    uint32 creationCount = 0;
    do
    {
        GameObjectInfo *info = NULL;
        uint32 objectInfoEntry = result->Fetch()[0].GetUInt32();
        if((info = GameObjectNameStorage.LookupEntry(objectInfoEntry)) == NULL)
            continue; // Not possible but check anyway
        if(info->data.moTransport.moveSpeed != 30.f)
            continue; // We have no movement speed so do not initialize

        TaxiPath *transportPath = NULL;
        uint32 taxiPath = result->Fetch()[1].GetUInt32();
        if((transportPath = sTaxiMgr.GetTaxiPath(taxiPath)) == NULL)
            continue; // Possible as we might have deprecated transports

        // Create our transport data
        _CreateTransportData(info, transportPath);
        ++creationCount;
    }while(result->NextRow());
    delete result;
    result = NULL;

    sLog.Notice("TransportMgr", "%u transports loaded from database", creationCount);
}

void TransportMgr::ProcessTransports(uint32 uiDiff)
{
    m_transportUpdateTimer += uiDiff;
    if(m_transportUpdateTimer < 5000)
        return;
    m_transportUpdateTimer = 0;

    m_transportStatusLock.Acquire();
    for(std::map<WoWGuid, TransportStatus*>::iterator itr = m_transportStatusStorage.begin(); itr != m_transportStatusStorage.end(); ++itr)
        itr->second->Update();
    m_transportStatusLock.Release();
}

void TransportMgr::PreloadMapInstance(uint32 msTime, MapInstance *instance, uint32 mapId)
{
    return;

    for(auto itr = m_transportDataStorage.begin(); itr != m_transportDataStorage.end(); ++itr)
    {
        TransportData *transData = itr->second;
        if(mapId != transData->mapIds[0] && mapId != transData->mapIds[1])
            continue;

        TaxiPathNodeEntry *taxiPoint = transData->transportPath->GetPathNode(transData->transportPath->GetStartNode(mapId));
        if(taxiPoint == NULL)
            continue;

        GameObject *gobj = new GameObject();
        gobj->Construct(transData->transportTemplate, MAKE_NEW_GUID(transData->transportTemplate->ID, transData->transportTemplate->ID, HIGHGUID_TYPE_MO_TRANSPORT));

        // Finish loading our allocation
        gobj->Load(mapId, taxiPoint->LocX, taxiPoint->LocY, taxiPoint->LocZ);
        gobj->SetInstanceID(instance->GetInstanceID());

        // Push to our map instance
        if(instance->IsGameObjectPoolUpdating())
            instance->AddObject(gobj);
        else gobj->PushToWorld(instance, msTime);

        // Second map, deactivate till we're in timer
        if(mapId != transData->mapIds[0])
            gobj->Deactivate(0);
    }
}

bool TransportMgr::RegisterTransport(GameObject *gobj, uint32 mapId, GameObject::TransportTaxiData *dataOut)
{
    if(gobj->GetGUID().getHigh() != HIGHGUID_TYPE_MO_TRANSPORT)
        return false; // Not initialized correctly

    std::map<uint32, TransportData*>::iterator itr;
    if((itr = m_transportDataStorage.find(gobj->GetEntry())) == m_transportDataStorage.end())
        return false;

    // Get our map index
    uint8 index = (mapId == itr->second->mapIds[0]) ? 0 : 1;

    // Set activity state to match our internal data
    dataOut->isActive = (index == 0);

    // We spawn at our start node for this map, so initialize position here
    dataOut->currentPos.ChangeCoords(gobj->GetPositionX(), gobj->GetPositionY(), gobj->GetPositionZ(), gobj->GetOrientation());

    // Grab our map specific storage
    dataOut->movementPath = itr->second->transportPath->GetMapPointStorage(mapId);

    // Check if we change maps
    dataOut->changesMaps = itr->second->transportPath->HasMapChange(mapId);

    // Grab our movement speed from our transport data
    dataOut->moveSpeed = itr->second->transportTemplate->data.moTransport.moveSpeed;

    // Transfer our path point
    dataOut->pathTimers.insert(itr->second->pathPointTimes[index].begin(), itr->second->pathPointTimes[index].end());

    // Transfer our delay timers
    dataOut->taxiDelayTimers.insert(itr->second->taxiDelayTimers[index].begin(), itr->second->taxiDelayTimers[index].end());

    // Grab our calc start time but also set our tick to match
    dataOut->transportTick = dataOut->pathStartTime = itr->second->timerStart[index];

    // Grab our path travel time
    dataOut->pathTravelTime = itr->second->pathTravelTime[index];

    // Grab our calculated full path length
    dataOut->calculatedPathTimer = itr->second->calculatedPathTimer;
    return true;
}

void TransportMgr::ChangeTransportActiveMap(WoWGuid transport, uint32 oldMapId)
{
    Guard guard(m_transportStatusLock);
    std::map<WoWGuid, TransportStatus*>::iterator itr;
    if((itr = m_transportStatusStorage.find(transport)) == m_transportStatusStorage.end())
        return;
    std::map<uint32, TransportData*>::iterator itr2;
    if((itr2 = m_transportDataStorage.find(transport.getLow())) == m_transportDataStorage.end())
        return;
    uint32 newMapId = itr2->second->transportPath->GetNextMap(oldMapId);


    itr->second->QueueMapTransfer(oldMapId, newMapId);
}

bool TransportMgr::CheckTransportPosition(WoWGuid transport, uint32 mapId)
{
    Guard guard(m_transportStatusLock);
    std::map<WoWGuid, TransportStatus*>::iterator itr;
    if((itr = m_transportStatusStorage.find(transport)) == m_transportStatusStorage.end())
        return false;

    return mapId == itr->second->getMapId();
}

void TransportMgr::UpdateTransportData(Player *plr)
{

}

void TransportMgr::ClearPlayerData(Player *plr)
{

}

void TransportMgr::_CreateTransportData(GameObjectInfo *info, TaxiPath *inputPath)
{
    TransportData *data = new TransportData();
    data->transportTemplate = info;
    // Grab a catmull rom path clone from our source
    data->transportPath = inputPath->CloneAsCatmullRom();
    data->timerStart[0] = data->timerStart[1] = 0;

    TaxiPath *ourPath = data->transportPath;
    // Use double precision to calculate traversal time between points
    double traversedTime[2] = { 0., 0. };
    double moveSpeed = info->data.moTransport.moveSpeed, accelerate = info->data.moTransport.accelRate;
    // Find our acceleration distance
    float currDecelSpeed = moveSpeed, decelerateDistance = 0.f, currAccSpeed = accelerate, accelerateDistance = 0.f;
    while(true)
    {
        bool processed = true;
        if(currDecelSpeed > 0.f)
        {
            processed = false;
            // We've traveled this distance
            decelerateDistance += currDecelSpeed;
            // We've decelerated this much over that difference
            if(currDecelSpeed >= accelerate)
                currDecelSpeed -= accelerate;
            else // We actually hit our stop point somewhere while decelerating
            {
                // Remove our decelerate speed
                decelerateDistance -= currDecelSpeed;
                // Temp: Just add our accelerate distance, it'll be close enough
                decelerateDistance += accelerate;
                // We're done decelerating
                currDecelSpeed = 0.f;
            }
        }

        if(currAccSpeed < moveSpeed)
        {
            processed = false;
            // We've traveled this distance
            accelerateDistance += currAccSpeed;
            // We've accelerated this much over that difference
            if(currAccSpeed+accelerate <= moveSpeed)
                currAccSpeed += accelerate;
            else // We actually hit our stop point somewhere while decelerating
            {
                // Remove our decelerate speed
                accelerateDistance -= currAccSpeed;
                // Temp: Just add our accelerate distance, it'll be close enough
                accelerateDistance += (moveSpeed * ((moveSpeed-currAccSpeed)/accelerate));
                // We're done decelerating
                currAccSpeed = moveSpeed;
            }
        }

        if(processed)
            break;
    }

    TaxiPathNodeEntry *node = NULL;
    for(uint8 m = 0; m < 2; ++m)
    {
        data->pathTravelTime[m] = 0;
        data->mapIds[m] = ourPath->mapData[m].mapId;
        if(ourPath->mapData[m].m_pathData.empty())
            continue;

        std::set<size_t> delayPoints;
        std::map<size_t, double> pointSpeedMap;
        for(size_t i = 0; i < ourPath->mapData[m].m_pathData.size(); ++i)
        {
            // Point length is actually distance between us and our previous point!
            if(double moveLength = ourPath->mapData[m].m_pathData[i].length)
                pointSpeedMap.insert(std::make_pair(i, (moveLength/moveSpeed)*1000.));
            if((node = ourPath->GetPathNode(ourPath->mapData[m].m_pathData[i].matchedNode)) && node->delay > 5)
                delayPoints.insert(i);
        }

        for(auto itr = delayPoints.begin(); itr != delayPoints.end(); ++itr)
        {
            size_t index = *itr, index2 = 0;
            std::map<size_t, float> pointsToDecelerate, pointsToAccelerate;
            float distance = ourPath->mapData[m].m_pathData[index].length;
            pointsToDecelerate.insert(std::make_pair(index, std::min(decelerateDistance, distance)));
            while(distance < decelerateDistance)
            {
                ++index2;
                if(index2 == index)
                    break;
                TaxiPath::posPoint *currPoint = &ourPath->mapData[m].m_pathData[index-index2];
                if(currPoint->length == 0.f)
                    break;

                distance += currPoint->length;
                pointsToDecelerate.insert(std::make_pair(index-index2, std::min(decelerateDistance, std::min((currPoint->length+(decelerateDistance-distance)), currPoint->length))));
            }

            index2 = 0;
            distance = 0.f;
            while(distance < accelerateDistance)
            {
                ++index2;
                if(index+index2 == (ourPath->mapData[m].m_pathData.size()))
                    break;
                TaxiPath::posPoint *currPoint = &ourPath->mapData[m].m_pathData[index+index2];
                if(currPoint->length == 0.f)
                    break;

                distance += currPoint->length;
                pointsToAccelerate.insert(std::make_pair(index+index2, std::min(accelerateDistance, std::min((currPoint->length+(accelerateDistance-distance)), currPoint->length))));
            }
            index2 = 0;

            // We've already started decelerating
            double currMoveSpeed = moveSpeed, totalMoved = 0.;
            for(auto itr = pointsToDecelerate.begin(); itr != pointsToDecelerate.end(); ++itr)
            {
                double movedTime = 0., neededMove = (currMoveSpeed-totalMoved);
                totalMoved += itr->second;
                while(totalMoved > currMoveSpeed)
                {
                    totalMoved -= currMoveSpeed;
                    // Our movetime is 1 second times how much distance we moved over our speed at the time
                    movedTime += (1000. * (neededMove/currMoveSpeed));
                    // Tick our decelerate
                    currMoveSpeed -= accelerate;
                    if(currMoveSpeed < accelerate)
                        currMoveSpeed = accelerate;
                    // Change needed movement speed to match new movement speed
                    neededMove = currMoveSpeed;
                }

                movedTime += (1000.*(std::min<float>(totalMoved, itr->second)/currMoveSpeed));

                // Check if we can just set our move time to match our new calculated move time
                if(ourPath->mapData[m].m_pathData[itr->first].length == itr->second)
                    pointSpeedMap[itr->first] = movedTime;
                else // We only used a certain amount of length, so just modify the existing value
                {
                    // Remove the move time of the distance we've calculated
                    pointSpeedMap[itr->first] -= ((itr->second/moveSpeed)*1000.);
                    pointSpeedMap[itr->first] += movedTime;
                }
            }

            /*// Start at our acceleration speed
            currMoveSpeed = accelerate; totalMoved = 0.;
            for(auto itr = pointsToAccelerate.begin(); itr != pointsToAccelerate.end(); ++itr)
            {
                double movedTime = 0., neededMove = (currMoveSpeed-totalMoved);
                totalMoved += itr->second;
                while(totalMoved > currMoveSpeed)
                {
                    totalMoved -= currMoveSpeed;
                    // Our movetime is 1 second times how much distance we moved over our speed at the time
                    movedTime += (1000. * (neededMove/currMoveSpeed));
                    currMoveSpeed += accelerate;
                    if(currMoveSpeed > moveSpeed)
                        currMoveSpeed = moveSpeed;
                    // Change needed movement speed to match new movement speed
                    neededMove = currMoveSpeed;
                }

                movedTime += float2int32(1000.*(std::min<float>(totalMoved, itr->second)/currMoveSpeed));

                // Check if we can just set our move time to match our new calculated move time
                if(ourPath->mapData[m].m_pathData[itr->first].length == itr->second)
                    pointSpeedMap[itr->first] = movedTime;
                else // We only used a certain amount of length, so just modify the existing value
                {
                    // Remove the move time of the distance we've calculated
                    pointSpeedMap[itr->first] -= ((itr->second/moveSpeed)*1000.);
                    pointSpeedMap[itr->first] += movedTime;
                }
                printf("");
            }*/
        }

        uint32 totalDelayTimer = 0;
        for(size_t i = 0; i < ourPath->mapData[m].m_pathData.size(); ++i)
        {
            // Point length is actually distance between us and our previous point!
            traversedTime[0] += pointSpeedMap[i];
            traversedTime[1] += pointSpeedMap[i];

            // Push traverse time after our updated time is added
            data->pathPointTimes[m].insert(std::make_pair(i, float2int32(traversedTime[1])));

            if(delayPoints.find(i) != delayPoints.end() && (node = ourPath->GetPathNode(ourPath->mapData[m].m_pathData[i].matchedNode)))
            {
                data->taxiDelayTimers[m].insert(std::make_pair(float2int32(traversedTime[1]), node->delay*1000));
                totalDelayTimer += node->delay;
            }
        }

        // Accuracy is within 1ms, works for me
        data->pathTravelTime[m] = float2int32(traversedTime[1]);
        // Add our total delay timer to our overall time for this map
        traversedTime[0] += (totalDelayTimer*1000);

        if(m == 0 && ourPath->mapData[1].m_pathData.size()) // We need our start time for path 2 to be the timer for our last path end
            data->timerStart[1] = data->pathTravelTime[m] + (totalDelayTimer*1000);
    }

    // Now add in our delay timers
    data->calculatedPathTimer = traversedTime[0];

    m_transportDataStorage.insert(std::make_pair(info->ID, data));

    // ID: 164871 Timer: 255895
    TransportStatus *status = new TransportStatus(info->ID);
    status->Initialize(data->mapIds[0]);
    m_transportStatusStorage.insert(std::make_pair(WoWGuid(MAKE_NEW_GUID(info->ID, info->ID, HIGHGUID_TYPE_MO_TRANSPORT)), status));
}