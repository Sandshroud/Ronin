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
    TaxiPath() {}
    ~TaxiPath() { m_pathNodes.clear(); }

    void ComputeLen();
    void SetPosForTime(float &x, float &y, float &z, uint32 time, uint32* lastnode, uint32 mapid);
    RONIN_INLINE uint32 GetID() { return id; }
    void SendMoveForTime(Player* riding, Player* to, uint32 time);
    void AddPathNode(uint32 index, TaxiPathNodeEntry* pn) { m_pathNodes.insert(std::make_pair(index, pn)); }
    RONIN_INLINE size_t GetNodeCount() { return m_pathNodes.size(); }
    TaxiPathNodeEntry* GetPathNode(uint32 i);
    RONIN_INLINE uint32 GetPrice() { return price; }
    RONIN_INLINE uint32 GetSourceNode() { return from; }
    RONIN_INLINE uint32 GetLength(uint32 mapId, bool catrum)
    {
        if(mapId == mapData[0].mapId)
            return catrum ? mapData[0].catrumLength : mapData[0].length;
        else if(mapId == mapData[1].mapId)
            return catrum ? mapData[1].catrumLength : mapData[1].length;
        return 0;
    }

    struct posPoint
    {
        posPoint(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
        float x, y, z;

        float dist(posPoint point)
        {
            float delta_x = fabs(x - point.x);
            float delta_y = fabs(y - point.y);
            float delta_z = fabs(z - point.z);
            return (delta_x*delta_x + delta_y*delta_y + delta_z*delta_z);
        }
    };

    RONIN_INLINE std::vector<posPoint> *GetPath(uint32 mapId, bool catrum)
    {
        if(mapId == mapData[0].mapId)
            return catrum ? &mapData[0].m_catrumPathData : &mapData[0].m_pathData;
        else if(mapId == mapData[1].mapId)
            return catrum ? &mapData[1].m_catrumPathData : &mapData[1].m_pathData;
        return NULL;
    }

protected:

    std::map<uint32, TaxiPathNodeEntry*> m_pathNodes;

    struct MapPointStorage
    {
        int32 mapId;
        float length, catrumLength;
        std::vector<posPoint> m_pathData, m_catrumPathData;
    } mapData[2];

    uint32 id, to, from, price;
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
