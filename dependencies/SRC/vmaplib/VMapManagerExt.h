/***
 * Demonstrike Core
 */

#pragma once

#include "VMapDefinitions.h"

namespace VMAP
{
    class VMapManagerExt
    {
    public:
        virtual void updateDynamicMapTree(unsigned int t_diff, int mapid = -1) = 0;
        virtual void LoadGameObjectModelList() = 0;

        // Static collision
        virtual bool loadMap(unsigned int mapId, FILE *file) = 0;
        virtual void unloadMap(unsigned int mapId) = 0;
        virtual bool loadMap(unsigned int mapId, int x, int y, FILE *file) = 0;
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

        virtual unsigned int getWMOData(unsigned int pMapId, float x, float y, float z, unsigned int &wmoFlags, bool &areaResult, unsigned int &adtFlags, int& adtId, int& rootId, int& groupId, float &groundHeight, unsigned short &liquidFlags, float &liquidHeight) const = 0;
        virtual bool getAreaInfo(unsigned int pMapId, float x, float y, float& z, unsigned int& flags, int& adtId, int& rootId, int& groupId) const = 0;
        virtual void getLiquidData(unsigned int pMapId, float x, float y, float z, unsigned short &typeFlags, float &level) const = 0;
    };
};