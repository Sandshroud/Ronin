/***
 * Demonstrike Core
 */

#pragma once

/* imports */
#define NO_WMO_HEIGHT -100000.0f
#define WMO_MAX_HEIGHT 100000.0f

class SERVER_DECL VMapInterface : public Singleton<VMapInterface>
{
private:
    VMAP::VMapManager* vMapMgr;

protected:
    struct MapLoadData
    {
        MapLoadData() : m_lock() { memset(&m_tileLoadCount, 0, sizeof(uint32)*64*64); }
        uint32 m_tileLoadCount[64][64];
        Mutex m_lock;
    }*m_mapLocks[NUM_MAPS];
    Mutex m_mapDataLock;

public:
    void Init();
    void DeInit();
    void UpdateAllMaps(uint32 p_time);
    void UpdateSingleMap(uint32 mapId, uint32 p_time);

    bool ActivateTile(uint32 mapId, uint32 tileX, uint32 tileY);
    void DeactivateTile(uint32 mapId, uint32 tileX, uint32 tileY);
    bool IsActiveTile(uint32 mapId, uint32 tileX, uint32 tileY);

    bool ActivateMap(uint32 mapId);
    void DeactivateMap(uint32 mapId);

    bool GetAreaInfo(uint32 mapId, float x, float y, float z, uint16 &areaId, uint32 &flags, int32 &adtId, int32 &rootId, int32 &groupid);
    bool CheckLOS(uint32 mapId, uint32 instanceId, int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2);
    bool GetFirstPoint(uint32 mapId, uint32 instanceId, int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2, float & outx, float & outy, float & outz, float distmod);
    bool IsIndoor(uint32 mapId, float x, float y, float z);
    bool IsIncity(uint32 mapid, float x, float y, float z);
    uint16 GetAreaID(uint32 mapId, float x, float y, float z);
    float GetHeight(uint32 mapId, uint32 instanceId, int32 m_phase, float x, float y, float z);
    float GetWaterHeight(uint32 mapId, float x, float y, float z, uint16 &outType);

    void LoadGameobjectModel(uint64 Guid, uint32 mapId, uint32 displayID, float scale, float posX, float posY, float posZ, float orientation, uint32 instanceId, int32 phasemask);
    void UpdateObjectModel(uint64 Guid, uint32 mapId, uint32 instanceId, uint32 displayID);
    void UnLoadGameobjectModel(uint64 Guid, uint32 mapId, uint32 instanceId);
};

#define sVMapInterface VMapInterface::getSingleton()
