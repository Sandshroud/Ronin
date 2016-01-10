/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton( TaxiMgr );

/************************
 *     TaxiPath    *
 ************************/

void TaxiPath::ComputeLen()
{
    m_length1 = m_length1 = 0;
    m_map1 = m_map2 = 0;
    float * curptr = &m_length1;

    if (!m_pathNodes.size())
        return;

    std::map<uint32, TaxiPathNodeEntry*>::iterator itr;
    itr = m_pathNodes.begin();

    float x = itr->second->LocX;
    float y = itr->second->LocY;
    float z = itr->second->LocZ;
    uint32 curmap = itr->second->ContinentID;
    m_map1 = curmap;

    ++itr;

    while (itr != m_pathNodes.end())
    {
        if( itr->second->ContinentID != curmap )
        {
            curptr = &m_length2;
            m_map2 = itr->second->ContinentID;
            curmap = itr->second->ContinentID;
        }

        *curptr += sqrt((itr->second->LocX - x)*(itr->second->LocX - x) +
            (itr->second->LocY - y)*(itr->second->LocY - y) +
            (itr->second->LocZ - z)*(itr->second->LocZ - z));

        x = itr->second->LocX;
        y = itr->second->LocY;
        z = itr->second->LocZ;
        ++itr;
    }
}

void TaxiPath::SetPosForTime(float &x, float &y, float &z, uint32 time, uint32 *last_node, uint32 mapid)
{
    if (!time)
        return;

    float length;
    if( mapid == m_map1 )
        length = m_length1;
    else
        length = m_length2;

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

    float length;
    uint32 mapid = riding->GetMapId();
    if( mapid == m_map1 )
        length = m_length1;
    else
        length = m_length2;

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
    data << uint32( MONSTER_MOVE_FLAG_FLY|0x00400000 );
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
                } else path = m_taxiPaths.at(pathnode->PathId);

                path->AddPathNode(pathnode->Id, pathnode);
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
    std::map<uint32, TaxiPath*>::iterator itr;
    for (itr = m_taxiPaths.begin(); itr != m_taxiPaths.end(); itr++)
        if ((itr->second->to == to) && (itr->second->from == from))
            return itr->second;
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

        uint8 team = ALLIANCE;
        if(node->mountIdHorde && node->mountIdAlliance == 0)
            team = HORDE;
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
