/***
 * Demonstrike Core
 */

#pragma once

namespace VMAP
{
    class ModelInstance;
    class GroupModel;
    class VMapManager;

    struct LocationInfo
    {
        LocationInfo(): hitInstance(0), hitModel(0), ground_Z(-G3D::inf()) {}
        const ModelInstance* hitInstance;
        const GroupModel* hitModel;
        float ground_Z;
    };

    class StaticMapTree
    {
        typedef std::map<G3D::g3d_uint32, bool> loadedTileMap;
        typedef std::map<G3D::g3d_uint32, G3D::g3d_uint32> loadedSpawnMap;
        private:
            G3D::g3d_uint32 iMapID;
            bool iIsTiled;
            BIH iTree;
            ModelInstance* iTreeValues; // the tree entries
            G3D::g3d_uint32 iNTreeValues;

            // Store all the map tile idents that are loaded for that map
            // some maps are not splitted into tiles and we have to make sure, not removing the map before all tiles are removed
            // empty tiles have no tile file, hence map with bool instead of just a set (consistency check)
            loadedTileMap iLoadedTiles;
            // stores <tree_index, reference_count> to invalidate tree values, unload map, and to be able to report errors
            loadedSpawnMap iLoadedSpawns;
            std::string iBasePath;

        private:
            bool getIntersectionTime(const G3D::Ray& pRay, float &pMaxDist, bool pStopAtFirstHit) const;
            //bool containsLoadedMapTile(unsigned int pTileIdent) const { return(iLoadedMapTiles.containsKey(pTileIdent)); }
        public:
            static std::string getTileFileName(G3D::g3d_uint32 mapID, G3D::g3d_uint32 tileX, G3D::g3d_uint32 tileY);
            static G3D::g3d_uint32 packTileID(G3D::g3d_uint32 tileX, G3D::g3d_uint32 tileY) { return tileX<<16 | tileY; }
            static void unpackTileID(G3D::g3d_uint32 ID, G3D::g3d_uint32 &tileX, G3D::g3d_uint32 &tileY) { tileX = ID>>16; tileY = ID&0xFF; }
            static bool CanLoadMap(const std::string &basePath, G3D::g3d_uint32 mapID, G3D::g3d_uint32 tileX, G3D::g3d_uint32 tileY);

            StaticMapTree(G3D::g3d_uint32 mapID, const std::string &basePath);
            ~StaticMapTree();

            bool isInLineOfSight(const G3D::Vector3& pos1, const G3D::Vector3& pos2) const;
            bool getObjectHitPos(const G3D::Vector3& pos1, const G3D::Vector3& pos2, G3D::Vector3& pResultHitPos, float pModifyDist) const;
            float getHeight(const G3D::Vector3& pPos, float maxSearchDist) const;
            G3D::g3d_uint32 GetVmapFlags(G3D::Vector3& pos);
            bool getAreaInfo(G3D::Vector3 &pos, G3D::g3d_uint32 &flags, G3D::g3d_int32 &adtId, G3D::g3d_int32 &rootId, G3D::g3d_int32 &groupId) const;
            bool GetLocationInfo(const G3D::Vector3 &pos, LocationInfo &info) const;

            bool InitMap(const std::string &fname, VMapManager* vm);
            void UnloadMap(VMapManager* vm);
            bool LoadMapTile(G3D::g3d_uint32 tileX, G3D::g3d_uint32 tileY, VMapManager* vm);
            void UnloadMapTile(G3D::g3d_uint32 tileX, G3D::g3d_uint32 tileY, VMapManager* vm);
            bool isTiled() const { return iIsTiled; }
            G3D::g3d_uint32 numLoadedTiles() const { return iLoadedTiles.size(); }
            void getModelInstances(ModelInstance* &models, G3D::g3d_uint32 &count);
    };

    struct AreaInfo
    {
        AreaInfo(): result(false), ground_Z(-G3D::inf()) {}
        bool result;
        float ground_Z;
        G3D::g3d_uint32 flags;
        G3D::g3d_int32 adtId;
        G3D::g3d_int32 rootId;
        G3D::g3d_int32 groupId;
    };
}                                                           // VMAP
