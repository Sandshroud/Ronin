/***
 * Demonstrike Core
 */

#pragma once

enum VMAP_gWMOFlags
{
    // WMO exists at this location
    WMO_FLAG_WMO_EXISTS             = 0x0001,
    // Inside, WMO minimap will be set
    WMO_FLAG_INSIDE_WMO_BOUNDS      = 0x0004,
    // Outside, WMO minimap is ignored
    WMO_FLAG_OUTSIDE_WMO_BOUNDS     = 0x0008,
    // Inside of a WMO flagged as a city
    WMO_FLAG_INSIDE_CITY_WMO        = 0x0040,
    // Inside of a WMO that is inside of another WMO
    WMO_FLAG_INSIDE_SUB_WMO         = 0x0100,
    // Seems constant when inside of WMO
    WMO_FLAG_UNK800                 = 0x0800,
    // Liquid exists for this WMO object
    WMO_FLAG_HAS_WMO_LIQUID         = 0x1000,
    // Seems constant when inside of WMO
    WMO_FLAG_UNK2000                = 0x2000,
    // No clue yet
    WMO_FLAG_WMO_NO_INSIDE          = 0x8000,
    // Seems to be Worldspawn WMO, client loads on map load, always in render etc
    WMO_FLAG_HARDCODED_WORLDSPAWN   = 0x80000000,
};

namespace VMAP
{
    class VMapManagerExt
    {
    public:
        virtual void updateDynamicMapTree(unsigned int t_diff, int mapid = -1) = 0;
        virtual void LoadGameObjectModelList() = 0;

        // Static collision
        virtual bool loadMap(unsigned int mapId) = 0;
        virtual void unloadMap(unsigned int mapId) = 0;
        virtual bool loadMap(unsigned int mapId, int x, int y) = 0;
        virtual void unloadMap(unsigned int mapId, int x, int y) = 0;

        // Dynamic collision
        virtual bool loadObject(unsigned long long guid, unsigned int mapId, unsigned int DisplayID, float scale, float x, float y, float z, float o, unsigned int m_instance, int m_phase) = 0;
        virtual bool changeObjectModel(unsigned long long guid, unsigned int mapId, unsigned int m_instance, unsigned int DisplayID) = 0;
        virtual void unloadObject(unsigned int mapId, unsigned int m_instance, unsigned long long guid) = 0;

        // Functionality
        virtual bool isInLineOfSight(unsigned int mapId, unsigned int m_instance, int m_phase, float x1, float y1, float z1, float x2, float y2, float z2) = 0;

        /** fill the hit pos and return true, if an object was hit */
        virtual bool getObjectHitPos(unsigned int mapId, unsigned int m_instance, int m_phase, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float& ry, float& rz, float modifyDist) = 0;
        virtual float getHeight(unsigned int mapId, unsigned int m_instance, int m_phase, float x, float y, float z, float maxSearchDist) = 0;

        virtual bool getAreaInfo(unsigned int pMapId, float x, float y, float& z, unsigned int& flags, int& adtId, int& rootId, int& groupId) const = 0;
        virtual void GetLiquidData(unsigned int pMapId, float x, float y, float z, unsigned short &type, float &level) const = 0;
    };
};