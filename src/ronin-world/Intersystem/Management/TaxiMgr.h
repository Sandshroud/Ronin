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
    void AddPathNode(uint32 index, TaxiPathNodeEntry* pn) { m_pathNodes[index] = pn; }
    RONIN_INLINE size_t GetNodeCount() { return m_pathNodes.size(); }
    TaxiPathNodeEntry* GetPathNode(uint32 i);
    RONIN_INLINE uint32 GetPrice() { return price; }
    RONIN_INLINE uint32 GetSourceNode() { return from; }

protected:

    std::map<uint32, TaxiPathNodeEntry*> m_pathNodes;

    float m_length1;
    uint32 m_map1;

    float m_length2;
    uint32 m_map2;
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

    void GetNearestTaxiNodes( uint32 mapid, float x, float y, float z, uint32 *taxiNodeOut );

    UpdateMask *GetAllTaxiMasks() { return &m_taxiMasks; }
    UpdateMask *GetHordeTaxiMasks() { return &m_hordeTaxiMasks; }
    UpdateMask *GetAllianceTaxiMasks() { return &m_allianceTaxiMasks; }
    UpdateMask *GetDeathKnightTaxiMasks() { return &m_DKTaxiMasks; }
private:
    std::map<uint32, TaxiPath*> m_taxiPaths;
    UpdateMask m_taxiMasks, m_hordeTaxiMasks, m_allianceTaxiMasks, m_DKTaxiMasks;
};

#define sTaxiMgr TaxiMgr::getSingleton()
