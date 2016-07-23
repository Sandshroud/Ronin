/***
 * Demonstrike Core
 */

#pragma once

#define TAXI_TRAVEL_SPEED 32

class Player;

class SERVER_DECL TaxiPath
{
    friend class TaxiMgr;
public:
    TaxiPath(uint32 id, uint32 to, uint32 from, uint32 price) : _Id(id), _To(to), _From(from), _Price(price), endX(0.f), endY(0.f), endZ(0.f), mapStartX(0.f), mapStartY(0.f), mapStartZ(0.f)
    {
        mapData[0].mapId = mapData[1].mapId = -1;
        mapData[0].length = mapData[1].length = 0.f;
    }

    ~TaxiPath() { m_pathNodes.clear(); }

    void ComputeLen();
    void GetPosForTime(uint32 mapid, float &x, float &y, float &z, uint32 time);
    RONIN_INLINE uint32 GetID() { return _Id; }
    void SendMoveForTime(Player* riding, Player* to, uint32 time, uint32 maxTime);
    void AddPathNode(uint32 index, TaxiPathNodeEntry* pn) { m_pathNodes.insert(std::make_pair(index, pn)); }
    RONIN_INLINE size_t GetNodeCount() { return m_pathNodes.size(); }
    TaxiPathNodeEntry* GetPathNode(uint32 i);
    RONIN_INLINE uint32 GetPrice() { return _Price; }
    RONIN_INLINE uint32 GetSourceNode() { return _From; }
    RONIN_INLINE uint32 GetEndNode() { return _To; }
    RONIN_INLINE uint32 GetLength(uint32 mapId)
    {
        if(mapId == mapData[0].mapId)
            return mapData[0].length;
        else if(mapId == mapData[1].mapId)
            return mapData[1].length;
        return 0;
    }

    void GetMapTargetPos(float &x, float &y, float &z, uint32 *mapId = NULL) { x = mapStartX, y = mapStartY, z = mapStartZ; if(mapId) *mapId = mapData[1].mapId; }
    void GetEndPos(float &x, float &y, float &z, uint32 *mapId = NULL)
    {
        x = endX, y = endY, z = endZ;
        if(mapId) *mapId = mapData[1].mapId != -1 ? mapData[1].mapId : mapData[0].mapId;
    }

    bool HasMapChange(uint32 mapId)
    {
        if(mapId != mapData[0].mapId)
            return false;
        return mapData[1].mapId != -1;
    }

    struct posPoint
    {
        posPoint(float _x, float _y, float _z) : x(_x), y(_y), z(_z), length(0.f) {}
        float x, y, z;
        float length;
    };

    RONIN_INLINE std::vector<posPoint> *GetPath(uint32 mapId)
    {
        if(mapId == mapData[0].mapId)
            return &mapData[0].m_pathData;
        else if(mapId == mapData[1].mapId)
            return &mapData[1].m_pathData;
        return NULL;
    }

protected:
    std::map<uint32, TaxiPathNodeEntry*> m_pathNodes;

    struct MapPointStorage
    {
        int32 mapId;
        float length;
        std::vector<posPoint> m_pathData;
    } mapData[2];

    uint32 _Id, _To, _From, _Price;
    float endX, endY, endZ;
    float mapStartX, mapStartY, mapStartZ;

    // Not for calling during runtime
    static float dist(posPoint a, posPoint b);
    size_t GetNodeForTime(uint32 mapId, uint32 time);
};

class SERVER_DECL TaxiMgr :  public Singleton < TaxiMgr >
{
public:
    TaxiMgr() {}
    ~TaxiMgr()
    {
        while(m_taxiPaths.size())
        {
            TaxiPath *p = m_taxiPaths.begin()->second;
            m_taxiPaths.erase(m_taxiPaths.begin());
            delete p;
        }

        m_taxiMasks.CleanupMask();
        m_hordeTaxiMasks.CleanupMask();
        m_allianceTaxiMasks.CleanupMask();
        m_DKTaxiMasks.CleanupMask();
    }

    void Initialize();
    TaxiPath *GetTaxiPath(uint32 path);
    TaxiPath *GetTaxiPath(uint32 from, uint32 to);
    std::vector<uint32> *GetPathTargets(uint32 from) { if(m_taxiPathTargets.find(from) == m_taxiPathTargets.end()) return NULL; return &m_taxiPathTargets.at(from); }

    void GetNearestTaxiNodes( uint32 mapid, float x, float y, float z, uint32 *taxiNodeOut );

    UpdateMask *GetAllTaxiMasks() { return &m_taxiMasks; }
    UpdateMask *GetHordeTaxiMasks() { return &m_hordeTaxiMasks; }
    UpdateMask *GetAllianceTaxiMasks() { return &m_allianceTaxiMasks; }
    UpdateMask *GetDeathKnightTaxiMasks() { return &m_DKTaxiMasks; }
private:
    std::map<uint32, TaxiPath*> m_taxiPaths;
    std::map<std::pair<uint32, uint32>, uint32> m_taxiTargetsToIds;

    std::map<uint32, std::vector<uint32>> m_taxiPathTargets;
    UpdateMask m_taxiMasks, m_hordeTaxiMasks, m_allianceTaxiMasks, m_DKTaxiMasks;
};

#define sTaxiMgr TaxiMgr::getSingleton()
