/***
 * Demonstrike Core
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

void TaxiPath::ComputeLen()
{
    mapData[0].mapId = mapData[1].mapId = -1;
    mapData[0].length = mapData[1].length = 0.f;
    mapData[0].catrumLength = mapData[1].catrumLength = 0.f;
    std::map<uint32, TaxiPathNodeEntry*>::iterator itr = m_pathNodes.begin();
    if(itr == m_pathNodes.end())
        return;

    uint8 index = 0;
    TaxiPath::posPoint lastPos(itr->second->LocX, itr->second->LocY, itr->second->LocZ);
    mapData[index].m_pathData.push_back(lastPos);
    mapData[index].mapId = itr->second->ContinentID;
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

            mapData[index].mapId = nodeEntry->ContinentID;
        }

        TaxiPath::posPoint pos(nodeEntry->LocX, nodeEntry->LocY, nodeEntry->LocZ);
        mapData[index].m_pathData.push_back(pos);
        mapData[index].length += pos.dist(lastPos);
        lastPos = pos;
    }

    mapData[0].length = mapData[0].length ? sqrt(mapData[0].length) : 0.f;
    mapData[1].length = mapData[1].length ? sqrt(mapData[1].length) : 0.f;
    savePathCatmullRom(&mapData[0].m_pathData, &mapData[0].m_catrumPathData);
    savePathCatmullRom(&mapData[1].m_pathData, &mapData[1].m_catrumPathData);

    if(!mapData[0].m_catrumPathData.empty())
    {
        std::vector<posPoint>::iterator posItr = mapData[0].m_catrumPathData.begin();
        lastPos = TaxiPath::posPoint((*posItr).x, (*posItr).y, (*posItr).z);
        posItr++;
        for( ; posItr != mapData[0].m_catrumPathData.end(); posItr++)
        {
            TaxiPath::posPoint pos((*posItr).x, (*posItr).y, (*posItr).z);
            mapData[0].catrumLength += pos.dist(lastPos);
            lastPos = pos;
        }
    }

    if(!mapData[1].m_catrumPathData.empty())
    {
        std::vector<posPoint>::iterator posItr = mapData[1].m_catrumPathData.begin();
        lastPos = TaxiPath::posPoint((*posItr).x, (*posItr).y, (*posItr).z);
        posItr++;
        for( ; posItr != mapData[1].m_catrumPathData.end(); posItr++)
        {
            TaxiPath::posPoint pos((*posItr).x, (*posItr).y, (*posItr).z);
            mapData[1].catrumLength += pos.dist(lastPos);
            lastPos = pos;
        }
    }
    mapData[0].catrumLength = mapData[0].catrumLength ? sqrt(mapData[0].catrumLength) : 0.f;
    mapData[1].catrumLength = mapData[1].catrumLength ? sqrt(mapData[1].catrumLength) : 0.f;
}

void TaxiPath::SetPosForTime(float &x, float &y, float &z, uint32 time, uint32 *last_node, uint32 mapid)
{
    if (!time)
        return;

    float length = GetLength(mapid, false);
    float traveled_len = (time/(length * TAXI_TRAVEL_SPEED))*length;
    uint32 len = 0;

    x = 0;
    y = 0;
    z = 0;

    if (!m_pathNodes.size())
        return;

    std::map<uint32, TaxiPathNodeEntry*>::iterator itr;
    itr = m_pathNodes.begin();

    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 0.0f;
    bool set = false;
    uint32 nodecounter = 0;

    while (itr != m_pathNodes.end())
    {
        if( itr->second->ContinentID != mapid )
        {
            ++itr;
            nodecounter++;
            continue;
        }

        if(!set)
        {
            nx = itr->second->LocX;
            ny = itr->second->LocY;
            nz = itr->second->LocZ;
            set = true;
            continue;
        }

        len = (uint32)sqrt((itr->second->LocX - nx)*(itr->second->LocX - nx) + (itr->second->LocY - ny)*(itr->second->LocY - ny) +
            (itr->second->LocZ - nz)*(itr->second->LocZ - nz));

        if (len >= traveled_len)
        {
            x = (itr->second->LocX - nx)*(traveled_len/len) + nx;
            y = (itr->second->LocY - ny)*(traveled_len/len) + ny;
            z = (itr->second->LocZ - nz)*(traveled_len/len) + nz;
            *last_node = nodecounter;
            return;
        }
        else
        {
            traveled_len -= len;
        }

        nx = itr->second->LocX;
        ny = itr->second->LocY;
        nz = itr->second->LocZ;
        ++itr;
        nodecounter++;
    }

    x = nx;
    y = ny;
    z = nz;
}

TaxiPathNodeEntry* TaxiPath::GetPathNode(uint32 i)
{
    if (m_pathNodes.find(i) != m_pathNodes.end())
        return m_pathNodes.at(i);
    return NULL;
}

void TaxiPath::SendMoveForTime(Player* riding, Player* to, uint32 time)
{
    if (!time)
        return;

    uint32 mapid = riding->GetMapId();
    float length = GetLength(mapid, false);
    float traveled_len = (time/(length * TAXI_TRAVEL_SPEED))*length;
    uint32 len = 0;
    float x = 0,y = 0,z = 0;

    if (!m_pathNodes.size())
        return;

    std::map<uint32, TaxiPathNodeEntry*>::iterator itr;
    itr = m_pathNodes.begin();

    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 0.0f;
    bool set = false;
    uint32 nodecounter = 1;

    while (itr != m_pathNodes.end())
    {
        if( itr->second->ContinentID != mapid )
        {
            ++itr;
            nodecounter++;
            continue;
        }

        if(!set)
        {
            nx = itr->second->LocX;
            ny = itr->second->LocY;
            nz = itr->second->LocZ;
            set = true;
            continue;
        }

        len = (uint32)sqrt((itr->second->LocX - nx)*(itr->second->LocX - nx) +
            (itr->second->LocY - ny)*(itr->second->LocY - ny) +
            (itr->second->LocZ - nz)*(itr->second->LocZ - nz));

        if (len >= traveled_len)
        {
            x = (itr->second->LocX - nx)*(traveled_len/len) + nx;
            y = (itr->second->LocY - ny)*(traveled_len/len) + ny;
            z = (itr->second->LocZ - nz)*(traveled_len/len) + nz;
            break;
        }
        else
        {
            traveled_len -= len;
        }

        nx = itr->second->LocX;
        ny = itr->second->LocY;
        nz = itr->second->LocZ;
        ++itr;
    }

    if (itr == m_pathNodes.end())
        return;

    WorldPacket data(SMSG_MONSTER_MOVE, 500);
    data << riding->GetGUID().asPacked();
    data << uint8(0);
    data << riding->GetPositionX() << riding->GetPositionY() << riding->GetPositionZ();
    data << getMSTime();
    data << uint8(0);
    data << uint32( 0x00400000|0x00000800|0x00000200 );
    data << uint32(uint32(length * TAXI_TRAVEL_SPEED) - time);
    size_t pos = data.wpos();
    data << uint32(nodecounter);
    data << nx << ny << nz;

    while (itr != m_pathNodes.end())
    {
        TaxiPathNodeEntry *pn = itr->second;
        ++itr;

        if( pn->ContinentID != mapid )
            break;

        ++nodecounter;
        data << pn->LocX << pn->LocY << pn->LocZ;
    }

    data.put<uint32>(pos, nodecounter);
    to->CopyAndSendDelayedPacket(&data);
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
                    path = new TaxiPath();
                    path->from = pathEntry->from;
                    path->to = pathEntry->to;
                    path->id = pathEntry->id;
                    path->price = pathEntry->price;
                    m_taxiPaths.insert(std::make_pair(pathnode->PathId, path));
                    m_taxiPathTargets[path->from].push_back(path->to);
                } else path = m_taxiPaths.at(pathnode->PathId);

                path->AddPathNode(pathnode->NodeIndex, pathnode);
                m_taxiTargetsToIds.insert(std::make_pair(std::make_pair(path->from, path->to), pathnode->PathId));
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
