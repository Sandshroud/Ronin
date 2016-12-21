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
    HMODULE hModule;
    VMAP::VMapManagerExt* vMapMgr;

protected:
    struct MapLoadData
    {
        MapLoadData() : m_lock() { memset(&m_tileLoadCount, 0, sizeof(uint32)*64*64); }
        uint32 m_tileLoadCount[64][64];
        Mutex m_lock;
    };
    Mutex m_mapDataLock;

    std::map<uint32, MapLoadData*> m_mapLocks;

public:
    void Init();
    void DeInit();

    void UpdateSingleMap(uint32 mapId, uint32 p_time);

    int ActivateTile(uint32 mapId, uint32 tileX, uint32 tileY, FILE *file);
    void DeactivateTile(uint32 mapId, uint32 tileX, uint32 tileY);
    bool IsActiveTile(uint32 mapId, uint32 tileX, uint32 tileY);

    bool ActivateMap(uint32 mapId, FILE *mapFile);
    void DeactivateMap(uint32 mapId);

    void GetWMOData(uint32 mapId, float x, float y, float z, uint32 &wmoId, uint32 &areaId, uint16 &areaFlags, float &groundLevel, uint16 &liquidFlags, float &liquidLevel);

    float GetHeight(uint32 mapId, uint32 instanceId, int32 m_phase, float x, float y, float z);
    bool CheckLOS(uint32 mapId, uint32 instanceId, int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2);
    bool GetFirstPoint(uint32 mapId, uint32 instanceId, int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2, float & outx, float & outy, float & outz, float distmod);

    void LoadGameobjectModel(uint64 Guid, uint32 mapId, uint32 displayID, float scale, float posX, float posY, float posZ, float orientation, uint32 instanceId, int32 phasemask);
    void UpdateObjectModel(uint64 Guid, uint32 mapId, uint32 instanceId, uint32 displayID);
    void UnLoadGameobjectModel(uint64 Guid, uint32 mapId, uint32 instanceId);

    void LoadCallback(VMAP::VMapManagerExt *mgr) { vMapMgr = mgr; };
};

typedef VMAP::VMapManagerExt *(*vmap_manager_construction)(const char *vmapDir);

#define sVMapInterface VMapInterface::getSingleton()
