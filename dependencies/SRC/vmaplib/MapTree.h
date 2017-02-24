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

namespace VMAP
{
    class ModelInstance;
    class GroupModel;
    class VMapManager;
    class DataPointCallback;

    struct WMOData
    {   // Quick grab of all needed WMO info from vmap data
        WMOData() : groundResult(false), hitResult(false), ground_CalcZ(-G3D::inf()), offset_z(-G3D::inf()), hitInstance(NULL), hitModel(NULL), hitFlags(0), flags(0), wmoId(0), adtId(0), rootId(0), groupId(0), liqTypeSearch(0), LiquidHeightSearch(-G3D::inf()) {}

        // We have data
        bool groundResult, hitResult;

        // GroundZ
        float ground_CalcZ;

        // Location info
        const ModelInstance* hitInstance;
        const GroupModel* hitModel;
        G3D::uint16 hitFlags;
        float offset_z;

        // Area data
        G3D::uint32 flags;
        G3D::int32 wmoId, adtId, rootId, groupId;

        // Liquid data
        G3D::uint16 liqTypeSearch;
        float LiquidHeightSearch;
    };

    struct LocationInfo
    {
        LocationInfo(): hitInstance(0), hitModel(0), ground_Z(-G3D::inf()) {}
        const ModelInstance* hitInstance;
        const GroupModel* hitModel;
        float ground_Z;
    };

    class StaticMapTree
    {
        typedef std::map<G3D::uint32, bool> loadedTileMap;
        typedef std::map<G3D::uint32, G3D::uint32> loadedSpawnMap;
        typedef std::map<G3D::uint32, G3D::uint32> fileOffsetMap;
        private:
            G3D::uint32 iMapID;
            BIH iTree;
            G3D::AABox iTreeBounds;
            ModelInstance* iTreeValues; // the tree entries
            G3D::uint32 iNTreeValues;

            // Store all the map tile idents that are loaded for that map
            // some maps are not splitted into tiles and we have to make sure, not removing the map before all tiles are removed
            // empty tiles have no tile file, hence map with bool instead of just a set (consistency check)
            loadedTileMap iLoadedTiles;
            // stores <tree_index, reference_count> to invalidate tree values, unload map, and to be able to report errors
            loadedSpawnMap iLoadedSpawns;
            // Sotred offsets for main map file
            fileOffsetMap iFileOffsets;
            // Load mapping
            std::map<G3D::uint32, std::vector<std::pair<uint32, std::string>>> loadedData;

        private:
            bool getIntersectionTime(const G3D::Ray& pRay, float &pMaxDist, bool pStopAtFirstHit) const;
            //bool containsLoadedMapTile(unsigned int pTileIdent) const { return(iLoadedMapTiles.containsKey(pTileIdent)); }
        public:
            static std::string getTileFileName(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY);

            StaticMapTree(G3D::uint32 mapID);
            ~StaticMapTree();

            bool isInLineOfSight(const G3D::Vector3& pos1, const G3D::Vector3& pos2) const;
            bool getObjectHitPos(const G3D::Vector3& pos1, const G3D::Vector3& pos2, G3D::Vector3& pResultHitPos, float pModifyDist) const;
            float getHeight(const G3D::Vector3& pPos, float maxSearchDist) const;

            void getWMOData(const G3D::Vector3 &pos, WMOData &data, G3D::int32 requiredFlags = 0, G3D::int32 ignoreFlags = 0) const;
            bool getAreaInfo(G3D::Vector3 &pos, G3D::uint32 &flags, G3D::int32 &adtId, G3D::int32 &rootId, G3D::int32 &groupId) const;
            bool GetLocationInfo(const G3D::Vector3 &pos, LocationInfo &info) const;

            bool InitMap(VMapManager* vm, FILE *file);
            void UnloadMap(VMapManager* vm);
            bool LoadMapTile(G3D::uint32 tileX, G3D::uint32 tileY, FILE *file, VMapManager* vm);
            void UnloadMapTile(G3D::uint32 tileX, G3D::uint32 tileY, VMapManager* vm);
            G3D::uint32 numLoadedTiles() const { return iLoadedTiles.size(); }
            void getModelInstances(ModelInstance* &models, G3D::uint32 &count);
    };

    struct AreaInfo
    {
        AreaInfo(): result(false), ground_Z(-G3D::inf()) {}
        bool result;
        float ground_Z;
        G3D::uint32 flags;
        G3D::int32 adtId;
        G3D::int32 rootId;
        G3D::int32 groupId;
    };
}                                                           // VMAP
