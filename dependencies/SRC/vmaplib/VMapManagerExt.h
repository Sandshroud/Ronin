/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2014-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "VMapDefinitions.h"

namespace VMAP
{
    class DataPointCallback;

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

        virtual unsigned int getWMOData(unsigned int pMapId, float x, float y, float z, unsigned int &wmoFlags, unsigned int &areaResultFlags, unsigned int &adtFlags, int& adtId, int& rootId, int& groupId, float &groundHeight, unsigned short &liquidFlags, float &liquidHeight, DataPointCallback* callback) const = 0;
        virtual bool getAreaInfo(unsigned int pMapId, float x, float y, float& z, unsigned int& flags, int& adtId, int& rootId, int& groupId) const = 0;
        virtual void getLiquidData(unsigned int pMapId, float x, float y, float z, unsigned short &typeFlags, float &level) const = 0;
    };

    class DataPointCallback
    {
    public:
        virtual void point(unsigned int id0, unsigned int id1, unsigned int id2, float x, float y, float z) = 0;
        virtual void post(unsigned int id0, unsigned int id1, unsigned int id2) = 0;
    };
};