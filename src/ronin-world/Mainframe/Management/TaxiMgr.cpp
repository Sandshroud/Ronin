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

initialiseSingleton( TaxiMgr );

/************************
 *     TaxiPath    *
 ************************/

TaxiPath::posPoint interpolatedPosition( TaxiPath::posPoint P0, TaxiPath::posPoint P1, TaxiPath::posPoint P2, TaxiPath::posPoint P3, uint8 u ) //Catmull-Rom interpolation
{
    static double pathPoints[5] = { 0.1f, 0.25f, 0.5f, 0.75f, 0.9f };
    double u3 = pathPoints[u] * pathPoints[u] * pathPoints[u], u2 = pathPoints[u] * pathPoints[u];
    double f1 = -0.5 * u3 + u2 - 0.5 * pathPoints[u];
    double f2 =  1.5 * u3 - 2.5 * u2 + 1.0;
    double f3 = -1.5 * u3 + 2.0 * u2 + 0.5 * pathPoints[u];
    double f4 =  0.5 * u3 - 0.5 * u2;
    return TaxiPath::posPoint(P0.x * f1 + P1.x * f2 + P2.x * f3 + P3.x * f4, P0.y * f1 + P1.y * f2 + P2.y * f3 + P3.y * f4, P1.z+(u*((P2.z-P1.z)/5)));
}

void savePathCatmullRom( std::vector<TaxiPath::posPoint> *path, std::vector<TaxiPath::posPoint> *pathOut ) //main function to calculate the Path
{
    if ( path == NULL || path->empty() )
        return;

    int32 length = path->size();
    for ( int32 i = 0; i < length-1; i++ )
    {
        pathOut->push_back(path->at(i));
        for(uint8 u = 0; u < 5; u++)
        {
            int32 i0 = std::max(0, i-1), i1 = i, i2 = std::min(i+1, length-1), i3 = std::min(i+2, length-1);
            TaxiPath::posPoint vec = interpolatedPosition((*path)[i0], (*path)[i1], (*path)[i2], (*path)[i3], u);
            pathOut->push_back(vec);   //store each value
        }
    }
}

float TaxiPath::dist(posPoint a, posPoint b)
{
    float delta_x = fabs(a.x - b.x), delta_y = fabs(a.y - b.y), delta_z = fabs(a.z - b.z);
    return sqrtf(delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);
}

size_t TaxiPath::GetNodeForTime(uint32 mapId, uint32 time)
{
    std::vector<posPoint> *path = GetPath(mapId);
    size_t i = GetStartNode(mapId);
    if(time == 0 || path == NULL)
        return i;

    float dist = (float(time)/1000.f)*TAXI_TRAVEL_SPEED, len = 0.f;
    std::vector<posPoint>::iterator itr;
    for(itr = path->begin(); itr != path->end(); itr++)
    {
        if(len + (*itr).length >= dist)
            break;
        len += (*itr).length;
        i++;
    }
    return i;
}

void TaxiPath::ComputeLen()
{
    std::map<uint32, TaxiPathNodeEntry*>::iterator itr = m_pathNodes.begin();
    if(itr == m_pathNodes.end())
        return;

    uint8 index = 0;
    posPoint lastPos(itr->second->LocX, itr->second->LocY, itr->second->LocZ);
    mapData[index].m_pathData.push_back(lastPos);
    mapData[index].mapId = itr->second->ContinentID;
    mapData[index].startNode = itr->second->NodeIndex;
    itr++;

    for(; itr != m_pathNodes.end(); itr++)
    {
        TaxiPathNodeEntry *nodeEntry = itr->second;
        if(mapData[index].mapId != nodeEntry->ContinentID)
        {
            // only support 2 map changes
            if(index == 1)
                break;
            index++;

            mapData[index].startNode = nodeEntry->NodeIndex;
            mapData[index].mapId = nodeEntry->ContinentID;
            mapStartX = nodeEntry->LocX;
            mapStartY = nodeEntry->LocY;
            mapStartZ = nodeEntry->LocZ;
            posPoint pos(nodeEntry->LocX, nodeEntry->LocY, nodeEntry->LocZ);
            mapData[index].m_pathData.push_back(pos);
            lastPos = pos;
            continue;
        }

        posPoint pos(nodeEntry->LocX, nodeEntry->LocY, nodeEntry->LocZ);
        pos.length = dist(pos, lastPos);
        mapData[index].m_pathData.push_back(pos);
        mapData[index].length += pos.length;
        lastPos = pos;
    }

    endX = lastPos.x;
    endY = lastPos.y;
    endZ = lastPos.z;
}

void TaxiPath::GetPosForTime(uint32 mapid, float &x, float &y, float &z, uint32 time)
{
    std::vector<posPoint> *path = GetPath(mapid);
    uint32 traveltime = 1000 * (GetLength(mapid)/TAXI_TRAVEL_SPEED);
    if(path == NULL || time >= traveltime)
        x = endX, y = endY, z = endZ;
    else
    {
        float len = 0, dist = (float(time)/1000.f)*TAXI_TRAVEL_SPEED;
        std::vector<posPoint>::iterator itr;
        for(itr = path->begin(); itr != path->end(); itr++)
        {
            if(len + (*itr).length > dist)
                break;
            else len += (*itr).length;
            x = (*itr).x, y = (*itr).y, z = (*itr).z;
        }

        if(itr != path->end() && (*itr).length)
        {
            posPoint next(*itr);
            float p = ((dist-len)/next.length);
            x -= ((x-next.x)*p);
            y -= ((y-next.y)*p);
            z -= ((z-next.z)*p);
        }
    }
}

TaxiPathNodeEntry* TaxiPath::GetPathNode(uint32 i)
{
    if (m_pathNodes.find(i) != m_pathNodes.end())
        return m_pathNodes.at(i);
    return NULL;
}

void TaxiPath::SendMoveForTime(Player* riding, Player* to, uint32 time, uint32 maxTime)
{
    if(time >= maxTime)
        return;

    uint32 mapId = riding->GetMapId();
    size_t startnode = 1+GetNodeForTime(mapId, time), endn = GetStartNode(mapId)+GetNodeCount(mapId);
    WorldPacket data(SMSG_MONSTER_MOVE, 38 + ( endn * 12 ) + 12 );
    data << riding->GetGUID().asPacked();
    data << uint8(0);
    data << riding->GetPositionX() << riding->GetPositionY() << riding->GetPositionZ();
    data << getMSTime();
    data << uint8( 0 );
    data << uint32( 0x00400000|0x00000800|0x00000200 );
    data << uint32( maxTime-time );
    data << uint32(endn-startnode);
    for(uint32 i = startnode; i < endn; i++)
    {
        TaxiPathNodeEntry *pn = GetPathNode(i);
        data << pn->LocX << pn->LocY << pn->LocZ;
    }

    if(riding != to)
        to->PushPacket(&data);
    else riding->SendMessageToSet(&data, true);
}

/***********************
 *     TaxiMgr     *
 ***********************/

struct TaxiPathBySourceAndDestination
{
    TaxiPathBySourceAndDestination() : ID(0), price(0) { }
    TaxiPathBySourceAndDestination(uint32 _id, uint32 _price) : ID(_id), price(_price) { }

    uint32 ID;
    uint32 price;
};
typedef std::map<uint32, TaxiPathBySourceAndDestination> TaxiPathSetForSource;
typedef std::map<uint32, TaxiPathSetForSource> TaxiPathSetBySource;

void TaxiMgr::Initialize()
{
    m_taxiMasks.SetCount(114*8);
    m_hordeTaxiMasks.SetCount(114*8);
    m_allianceTaxiMasks.SetCount(114*8);
    m_DKTaxiMasks.SetCount(114*8);

    TaxiPathSetBySource sTaxiPathSetBySource;
    for(uint32 j = 0; j < dbcTaxiPathNode.GetNumRows(); j++)
        if(TaxiPathEntry *entry = dbcTaxiPath.LookupRow(j))
            sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->id, entry->price);

    std::set<uint32> spellPaths;
    for (uint32 i = 0; i < dbcSpellEffect.GetNumRows(); ++i)
        if (SpellEffectEntry *sInfo = dbcSpellEffect.LookupRow(i))
            if (sInfo->Effect == SPELL_EFFECT_SEND_TAXI)
                spellPaths.insert(sInfo->EffectMiscValue);

    for(uint32 j = 0; j < dbcTaxiPathNode.GetNumRows(); j++)
    {
        if(TaxiPathNodeEntry *pathnode = dbcTaxiPathNode.LookupRow(j))
        {
            if(TaxiPathEntry *pathEntry = dbcTaxiPath.LookupEntry(pathnode->PathId))
            {
                TaxiPath *path;
                if(m_taxiPaths.find(pathnode->PathId) == m_taxiPaths.end())
                {
                    path = new TaxiPath(pathEntry->id, pathEntry->to, pathEntry->from, pathEntry->price);
                    m_taxiPaths.insert(std::make_pair(pathnode->PathId, path));
                    m_taxiPathTargets[path->_From].push_back(path->_To);
                } else path = m_taxiPaths.at(pathnode->PathId);

                path->AddPathNode(pathnode->NodeIndex, pathnode);
                m_taxiTargetsToIds.insert(std::make_pair(std::make_pair(path->_From, path->_To), pathnode->PathId));
            }
        }
    }

    for(uint32 i = 0; i < dbcTaxiNode.GetNumRows(); i++)
    {
        TaxiNodeEntry *node = dbcTaxiNode.LookupRow(i);
        if(node == NULL)
            continue;

        TaxiPathSetBySource::const_iterator src_i = sTaxiPathSetBySource.find(node->id);
        if (src_i != sTaxiPathSetBySource.end() && !src_i->second.empty())
        {
            bool ok = false;
            for (TaxiPathSetForSource::const_iterator dest_i = src_i->second.begin(); dest_i != src_i->second.end(); ++dest_i)
            {
                // not spell path
                if (spellPaths.find(dest_i->second.ID) == spellPaths.end())
                {
                    ok = true;
                    break;
                }
            }

            if (ok == false)
                continue;
        }

        m_taxiMasks.SetBit(node->id);
        if (node->mountIdHorde && node->mountIdHorde != 32981)
            m_hordeTaxiMasks.SetBit(node->id);
        if (node->mountIdAlliance && node->mountIdAlliance != 32981)
            m_allianceTaxiMasks.SetBit(node->id);
        if (node->mountIdHorde == 32981 || node->mountIdAlliance == 32981)
            m_DKTaxiMasks.SetBit(node->id);

        // old continent node (+ nodes virtually at old continents, check explicitly to avoid loading map files for zone info)
        if (node->mapid < 2 || node->id == 82 || node->id == 83 || node->id == 93 || node->id == 94)
            m_DKTaxiMasks.SetBit(node->id);
    }

    for(auto itr = m_taxiPaths.begin(); itr != m_taxiPaths.end(); itr++)
        itr->second->ComputeLen();
}

TaxiPath* TaxiMgr::GetTaxiPath(uint32 path)
{
    std::map<uint32, TaxiPath*>::iterator itr;
    if ((itr = m_taxiPaths.find(path)) != m_taxiPaths.end())
        return itr->second;
    return NULL;
}

TaxiPath* TaxiMgr::GetTaxiPath(uint32 from, uint32 to)
{
    std::pair<uint32, uint32> pathPair = std::make_pair(from, to);
    if(m_taxiTargetsToIds.find(pathPair) != m_taxiTargetsToIds.end())
        return GetTaxiPath(m_taxiTargetsToIds[pathPair]);
    return NULL;
}

void TaxiMgr::GetNearestTaxiNodes( uint32 mapid, float x, float y, float z, uint32 *taxiNodeOut )
{
    if(taxiNodeOut == NULL)
        return;

    float distance[2];
    taxiNodeOut[0] = taxiNodeOut[1] = 0;
    for (uint32 i = 0; i < dbcTaxiNode.GetNumRows(); i++)
    {
        TaxiNodeEntry const* node = dbcTaxiNode.LookupRow(i);
        if (!node || node->mapid != mapid)
            continue;

        // skip not taxi network nodes
        if(!m_taxiMasks.GetBit(node->id))
            continue;
        float delta_x = fabs(node->x - x), delta_y = fabs(node->y - y), delta_z = fabs(node->z - z), dist = (delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);
        if(node->mountIdHorde && node->mountIdAlliance)
        {
            for(uint8 team = 0; team < 2; team++)
            {
                if (taxiNodeOut[team])
                {
                    if (dist < distance[team])
                    {
                        distance[team] = dist;
                        taxiNodeOut[team] = node->id;
                    }
                }
                else
                {
                    distance[team] = dist;
                    taxiNodeOut[team] = node->id;
                }
            }
            continue;
        }

        uint8 team = (node->mountIdHorde && node->mountIdAlliance == 0) ? TEAM_HORDE : TEAM_ALLIANCE;
        if (taxiNodeOut[team])
        {
            if (dist < distance[team])
            {
                distance[team] = dist;
                taxiNodeOut[team] = node->id;
            }
        }
        else
        {
            distance[team] = dist;
            taxiNodeOut[team] = node->id;
        }
    }
}
