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

createFileSingleton(VMapInterface);

void VMapInterface::Init()
{
    vMapMgr = NULL;
    if(sWorld.Collision == false)
        return;

    if( hModule = LoadLibrary("./Modules/Collision.dll") )
    {
        // find version import
        if(vmap_manager_construction rcall = (vmap_manager_construction)GetProcAddress(hModule, "vmap_manager_construction"))
            vMapMgr = rcall(sWorld.VObjPath.c_str());
        else FreeLibrary(hModule);
    }

    if(vMapMgr)
    {
        sLog.Success("VMapInterface", "Collision linked successfully");
        vMapMgr->LoadGameObjectModelList();
    }
}

void VMapInterface::DeInit()
{
    if(vMapMgr)
        delete vMapMgr;
    vMapMgr = NULL;
    FreeLibrary(hModule);

    for (auto it : m_mapLocks)
        delete it.second;
}

void VMapInterface::UpdateSingleMap(uint32 mapId, uint32 p_time)
{
    if( vMapMgr == NULL )
        return;

    vMapMgr->updateDynamicMapTree(p_time, mapId);
}

bool VMapInterface::ActivateMap(uint32 mapId, FILE *mapFile)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) != m_mapLocks.end())
        return false;
    bool result;
    m_mapDataLock.Acquire();
    if(result = vMapMgr->loadMap(mapId, mapFile))
        m_mapLocks.insert(std::make_pair(mapId, new MapLoadData()));
    m_mapDataLock.Release();
    return result;
}

void VMapInterface::DeactivateMap(uint32 mapId)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return;

    m_mapDataLock.Acquire();
    // no instances using this anymore
    delete m_mapLocks[mapId];
    m_mapLocks.erase(mapId);
    vMapMgr->unloadMap(mapId);
    m_mapDataLock.Release();
}

int VMapInterface::ActivateTile(uint32 mapId, uint32 tileX, uint32 tileY, FILE *file)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return 0;
    MapLoadData *loadData = m_mapLocks[mapId];

    // acquire write lock
    loadData->m_lock.Acquire();
    if( loadData->m_tileLoadCount[tileX][tileY] == 0 )
    {
        if(vMapMgr->loadMap(mapId, tileX, tileY, file))
            sLog.outDebug("Loading VMap [%u/%u] successful", tileX, tileY);
        else
        {
            sLog.outDebug("Loading VMap [%u/%u] unsuccessful", tileX, tileY);
            loadData->m_lock.Release();
            return 0;
        }
    }

    // increment count
    loadData->m_tileLoadCount[tileX][tileY]++;

    // release lock
    loadData->m_lock.Release();
    return 1;
}

void VMapInterface::DeactivateTile(uint32 mapId, uint32 tileX, uint32 tileY)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return;
    MapLoadData *loadData = m_mapLocks[mapId];

    // get write lock
    loadData->m_lock.Acquire();
    if(loadData->m_tileLoadCount[tileX][tileY] == 1 )
        vMapMgr->unloadMap(mapId, tileX, tileY);

    if(loadData->m_tileLoadCount[tileX][tileY])
        --loadData->m_tileLoadCount[tileX][tileY];
    // release write lock
    loadData->m_lock.Release();
}

bool VMapInterface::IsActiveTile(uint32 mapId, uint32 tileX, uint32 tileY)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return false;

    bool isactive = false;

    // acquire write lock
    m_mapLocks[mapId]->m_lock.Acquire();
    if(m_mapLocks[mapId]->m_tileLoadCount[tileX][tileY])
        isactive = true;
    m_mapLocks[mapId]->m_lock.Release(); // release lock

    return isactive;
}

void VMapInterface::GetWMOData(MapInstance *instance, uint32 mapId, float x, float y, float z, uint32 &wmoId, uint32 &areaId, uint16 &areaFlags, float &groundLevel, uint16 &liquidFlags, float &liquidLevel)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    groundLevel = liquidLevel = NO_WMO_HEIGHT;
    uint32 wmoFlags = 0, adtFlags = 0;
    int32 adtId = 0, rootId = 0, groupId = 0;
    bool areaRes = false;

    // Grab as much WMO data as we can in a single check
    wmoId = vMapMgr->getWMOData(mapId, x, y, z, wmoFlags, areaRes, adtFlags, adtId, rootId, groupId, groundLevel, liquidFlags, liquidLevel, NULL);

    // We can just check for liquid data, but currently there is no real fix for caves etc so just accept any WMO with inherit data
    if(wmoFlags & VMAP::WMO_FLAG_HAS_WMO_LIQUID)
        areaFlags |= OBJECT_AREA_FLAG_USE_WMO_WATER;
    else if(wmoFlags & (VMAP::WMO_FLAG_INSIDE_MASTER_WMO|VMAP::WMO_FLAG_INSIDE_SLAVE_WMO))
        areaFlags;// |= OBJECT_AREA_FLAG_IGNORE_ADT_WATER;

    // Next check if we've got any area info from our callback
    if(areaRes)
    {   // We precached our WMO table data, so grab that from our manager
        WMOAreaTableEntry *WMOEntry = NULL;
        // Set our area Id
        if(WMOEntry = objmgr.GetWMOAreaTable(adtId, rootId, groupId))
            areaId = WMOEntry->areaId;

        // Indoor checks
        if(adtFlags & VMAP::WMO_FLAG_INSIDE_WMO_BOUNDS && !(adtFlags & VMAP::WMO_FLAG_OUTSIDE_WMO_BOUNDS || adtFlags & VMAP::WMO_FLAG_WMO_NO_INSIDE))
            if(WMOEntry == NULL || !(WMOEntry->Flags & 0x4))
                areaFlags |= OBJECT_AREA_FLAG_INDOORS;

        // City flag checks
        if(adtFlags & VMAP::WMO_FLAG_INSIDE_CITY_WMO
            //&& (flags & VMAP::WMO_FLAG_INSIDE_WMO_BOUNDS)
            && !(adtFlags & VMAP::WMO_FLAG_OUTSIDE_WMO_BOUNDS))
            areaFlags |= OBJECT_AREA_FLAG_INCITY;
        else if(WMOEntry && (adtFlags & (VMAP::WMO_FLAG_INSIDE_WMO_BOUNDS|VMAP::WMO_FLAG_INSIDE_SLAVE_WMO)))
            if(AreaTableEntry* ate = dbcAreaTable.LookupEntry(WMOEntry->areaId))
                if(ate->AreaFlags & AREA_CITY_AREA || ate->AreaFlags & AREA_CITY)
                    areaFlags |= OBJECT_AREA_FLAG_INCITY;
    }

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();
}

float VMapInterface::GetHeight(uint32 mapId, uint32 instanceId, int32 m_phase, float x, float y, float z)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return NO_WMO_HEIGHT;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    // get data
    float res = vMapMgr ? vMapMgr->getHeight(mapId, instanceId, m_phase, x, y, z, 10.0f) : NO_WMO_HEIGHT;

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();

    // return
    return res;
}

bool VMapInterface::CheckLOS(uint32 mapId, uint32 instanceId, int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return true;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    // get data
    bool res = vMapMgr ? vMapMgr->isInLineOfSight(mapId, instanceId, m_phase, x1, y1, z1, x2, y2, z2) : true;

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();

    // return
    return res;
}

bool VMapInterface::GetFirstPoint(uint32 mapId, uint32 instanceId, int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2, float & outx, float & outy, float & outz, float distmod)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return false;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    // get data
    bool res = (vMapMgr ? vMapMgr->getObjectHitPos(mapId, instanceId, m_phase, x1, y1, z1, x2, y2, z2, outx, outy, outz, distmod) : false);

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();

    // return
    return res;
}

void VMapInterface::LoadGameobjectModel(uint64 Guid, uint32 mapId, uint32 displayID, float scale, float posX, float posY, float posZ, float orientation, uint32 instanceId, int32 phasemask)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    vMapMgr->loadObject(Guid, mapId, displayID, scale, posX, posY, posZ, orientation, instanceId, phasemask);

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();
}

void VMapInterface::UpdateObjectModel(uint64 Guid, uint32 mapId, uint32 instanceId, uint32 displayID)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    vMapMgr->changeObjectModel(Guid, mapId, instanceId, displayID);

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();
}

void VMapInterface::UnLoadGameobjectModel(uint64 Guid, uint32 instanceId, uint32 mapId)
{
    if( vMapMgr == NULL || m_mapLocks.find(mapId) == m_mapLocks.end())
        return;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    vMapMgr->unloadObject(mapId, instanceId, Guid);

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();
}
