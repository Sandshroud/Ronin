/***
 * Demonstrike Core
 */

#include "StdAfx.h"

createFileSingleton(VMapInterface);

void VMapInterface::Init()
{
    sLog.Notice("CollideInterface", "Init");
    vMapMgr = new VMAP::VMapManager(sWorld.vMapPath);
    memset(&m_mapLocks, NULL, sizeof(MapLoadData*)*NUM_MAPS);
    vMapMgr->LoadGameObjectModelList();
}

void VMapInterface::DeInit()
{
    for(uint16 i = 0; i < NUM_MAPS; i++)
    {
        if(m_mapLocks[i])
        {
            vMapMgr->unloadMap(i);
            delete m_mapLocks[i];
            m_mapLocks[i] = NULL;
        }
    }
    delete vMapMgr;
    // bleh.
}

void VMapInterface::UpdateAllMaps(uint32 p_time)
{
    if( vMapMgr == NULL )
        return;

    vMapMgr->updateDynamicMapTree(p_time);
}

void VMapInterface::UpdateSingleMap(uint32 mapId, uint32 instanceId, uint32 p_time)
{
    if( vMapMgr == NULL )
        return;

    vMapMgr->updateDynamicMapTree(p_time, mapId, instanceId);
}

bool VMapInterface::ActivateMap(uint32 mapId)
{
    if( vMapMgr == NULL )
        return false;
    if(m_mapLocks[mapId] != NULL)
        return true;
    bool result;
    m_mapDataLock.Acquire();
    if(result = vMapMgr->loadMap(mapId))
        m_mapLocks[mapId] = new MapLoadData();
    m_mapDataLock.Release();
    return result;
}

void VMapInterface::DeactivateMap(uint32 mapId)
{
    if( vMapMgr == NULL )
        return;
    if(m_mapLocks[mapId] == NULL)
        return;

    m_mapDataLock.Acquire();
    // no instances using this anymore
    delete m_mapLocks[mapId];
    m_mapLocks[mapId] = NULL;
    vMapMgr->unloadMap(mapId);
    m_mapDataLock.Release();
}

bool VMapInterface::ActivateTile(uint32 mapId, uint32 tileX, uint32 tileY)
{
    if( vMapMgr == NULL )
        return false;
    if(m_mapLocks[mapId] == NULL)
        return false;

    // acquire write lock
    m_mapLocks[mapId]->m_lock.Acquire();
    if( m_mapLocks[mapId]->m_tileLoadCount[tileX][tileY] == 0 )
    {
        if(vMapMgr->loadMap(mapId, tileX, tileY))
            sLog.outDebug("Loading VMap [%u/%u] successful", tileX, tileY);
        else
        {
            sLog.outDebug("Loading VMap [%u/%u] unsuccessful", tileX, tileY);
            m_mapLocks[mapId]->m_lock.Release();
            return false;
        }
    }

    // increment count
    m_mapLocks[mapId]->m_tileLoadCount[tileX][tileY]++;

    // release lock
    m_mapLocks[mapId]->m_lock.Release();
    return true;
}

void VMapInterface::DeactivateTile(uint32 mapId, uint32 tileX, uint32 tileY)
{
    if( vMapMgr == NULL )
        return;
    if(m_mapLocks[mapId] == NULL)
        return;

    // get write lock
    m_mapLocks[mapId]->m_lock.Acquire();
    if( (--m_mapLocks[mapId]->m_tileLoadCount[tileX][tileY]) == 0 )
        vMapMgr->unloadMap(mapId, tileX, tileY);

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();
}

bool VMapInterface::IsActiveTile(uint32 mapId, uint32 tileX, uint32 tileY)
{
    if( vMapMgr == NULL )
        return false;
    if(m_mapLocks[mapId] == NULL)
        return false;

    bool isactive = false;

    // acquire write lock
    m_mapLocks[mapId]->m_lock.Acquire();
    if(m_mapLocks[mapId]->m_tileLoadCount[tileX][tileY])
        isactive = true;
    m_mapLocks[mapId]->m_lock.Release(); // release lock

    return isactive;
}

bool VMapInterface::CheckLOS(uint32 mapId, uint32 instanceId, int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2)
{
    if( vMapMgr == NULL )
        return true;
    if(m_mapLocks[mapId] == NULL)
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
    if( vMapMgr == NULL )
        return false;
    if(m_mapLocks[mapId] == NULL)
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

float VMapInterface::GetHeight(uint32 mapId, uint32 instanceId, int32 m_phase, float x, float y, float z)
{
    if( vMapMgr == NULL )
        return NO_WMO_HEIGHT;
    if(m_mapLocks[mapId] == NULL)
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

// DBC ids are stored in vmaps currently, so convert to terrain water flags
// We could use DBC file, but this workaround is sufficient.
uint16 convertWaterIDToFlags(uint16 wmoType)
{
    switch(wmoType)
    {
        // Mask these to Regular Water
    case 1: case 5: case 9: case 13: case 17:
    case 41: case 61: case 81: case 181:
        return 0x01;
        // Mask these to Ocean Water
    case 2: case 6: case 10:
    case 14: case 100:
        return 0x02;
        // Mask these to Regular Magma
    case 3: case 7: case 11: case 15:
    case 19: case 121: case 141:
        return 0x04;
        // Mask these to Regular Slime
    case 4: case 8: case 12:
    case 20: case 21:
        return 0x08;
    }
    return 0;
}

float VMapInterface::GetWaterHeight(uint32 mapId, float x, float y, float z, uint16 &outType)
{
    if( vMapMgr == NULL )
        return NO_WMO_HEIGHT;
    if(m_mapLocks[mapId] == NULL)
        return NO_WMO_HEIGHT;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    // get data
    uint16 waterDBCId = 0;
    float res = NO_WMO_HEIGHT;
    vMapMgr->GetLiquidData(mapId, x, y, z, waterDBCId, res);
    if(waterDBCId)
        outType = convertWaterIDToFlags(waterDBCId);

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();

    // return
    return res;
}

bool VMapInterface::IsIndoor(uint32 mapId, float x, float y, float z)
{
    if(vMapMgr == NULL)
        return false;
    if(m_mapLocks[mapId] == NULL)
        return false;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    bool res = false;
    uint32 flags = 0;
    int32 adtId = 0, rootId = 0, groupid = 0;
    if(vMapMgr->getAreaInfo(mapId, x, y, z, flags, adtId, rootId, groupid))
    {
        if(flags & WMO_FLAG_INSIDE_WMO_BOUNDS
            && !(flags & WMO_FLAG_OUTSIDE_WMO_BOUNDS)
            && !(flags & WMO_FLAG_WMO_NO_INSIDE))
        {
            WMOAreaTableEntry * WMOEntry = objmgr.GetWMOAreaTable(adtId, rootId, groupid);
            if(WMOEntry == NULL || !(WMOEntry->Flags & 0x4))
                res = true;
        }
    }

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();

    return res;
}

bool VMapInterface::IsIncity(uint32 mapId, float x, float y, float z)
{
    if(vMapMgr == NULL)
        return false;
    if(m_mapLocks[mapId] == NULL)
        return false;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    bool res = false;
    uint32 flags = 0;
    int32 adtId = 0, rootId = 0, groupid = 0;
    if(vMapMgr->getAreaInfo(mapId, x, y, z, flags, adtId, rootId, groupid))
    {
        if(flags & WMO_FLAG_INSIDE_CITY_WMO
            //&& (flags & WMO_FLAG_INSIDE_WMO_BOUNDS)
            && !(flags & WMO_FLAG_OUTSIDE_WMO_BOUNDS))
            res = true;
        else if(flags & (WMO_FLAG_INSIDE_WMO_BOUNDS|WMO_FLAG_INSIDE_SUB_WMO))
        {
            WMOAreaTableEntry * WMOEntry = objmgr.GetWMOAreaTable(adtId, rootId, groupid);
            if(WMOEntry != NULL)
            {
                AreaTableEntry* ate = dbcAreaTable.LookupEntry(WMOEntry->areaId);
                if(ate != NULL)
                {
                    if(ate->AreaFlags & AREA_CITY_AREA || ate->AreaFlags & AREA_CITY)
                        res = true;
                }
            }
        }
    }

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();

    return res;
}

bool VMapInterface::GetAreaInfo(uint32 mapId, float x, float y, float z, uint16 &areaId, uint32 &flags, int32 &adtId, int32 &rootId, int32 &groupid)
{
    if(vMapMgr == NULL)
        return false;
    if(m_mapLocks[mapId] == NULL)
        return false;

    bool res = false;
    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();
    if(vMapMgr->getAreaInfo(mapId, x, y, z, flags, adtId, rootId, groupid))
    {
        if(flags & WMO_FLAG_WMO_EXISTS)
        {
            res = true;
            WMOAreaTableEntry * WMOEntry = objmgr.GetWMOAreaTable(adtId, rootId, groupid);
            if(WMOEntry != NULL)
                areaId = WMOEntry->areaId;
        }
    }

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();
    return res;
}

void VMapInterface::LoadGameobjectModel(uint64 Guid, uint32 mapId, uint32 displayID, float scale, float posX, float posY, float posZ, float orientation, uint32 instanceId, int32 phasemask)
{
    if( vMapMgr == NULL )
        return;
    if(m_mapLocks[mapId] == NULL)
        return;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    vMapMgr->loadObject(Guid, mapId, displayID, scale, posX, posY, posZ, orientation, instanceId, phasemask);

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();
}

void VMapInterface::UpdateObjectModel(uint64 Guid, uint32 mapId, uint32 instanceId, uint32 displayID)
{
    if( vMapMgr == NULL )
        return;
    if(m_mapLocks[mapId] == NULL)
        return;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    vMapMgr->changeObjectModel(Guid, mapId, instanceId, displayID);

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();
}

void VMapInterface::UnLoadGameobjectModel(uint64 Guid, uint32 instanceId, uint32 mapId)
{
    if( vMapMgr == NULL )
        return;
    if(m_mapLocks[mapId] == NULL)
        return;

    // get read lock
    m_mapLocks[mapId]->m_lock.Acquire();

    vMapMgr->unloadObject(mapId, instanceId, Guid);

    // release write lock
    m_mapLocks[mapId]->m_lock.Release();
}
