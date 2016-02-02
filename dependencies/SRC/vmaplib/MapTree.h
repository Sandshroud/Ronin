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
        typedef std::map<G3D::uint32, bool> loadedTileMap;
        typedef std::map<G3D::uint32, G3D::uint32> loadedSpawnMap;
        typedef std::map<G3D::uint32, G3D::uint32> fileOffsetMap;
        private:
            G3D::uint32 iMapID;
            BIH iTree;
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

            // 
            Mutex fileLock;
            std::string iFileName;

        private:
            bool getIntersectionTime(const G3D::Ray& pRay, float &pMaxDist, bool pStopAtFirstHit) const;
            //bool containsLoadedMapTile(unsigned int pTileIdent) const { return(iLoadedMapTiles.containsKey(pTileIdent)); }
        public:
            static std::string getTileFileName(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY);
            static G3D::uint32 packTileID(G3D::uint32 tileX, G3D::uint32 tileY) { return tileX<<16 | tileY; }
            static void unpackTileID(G3D::uint32 ID, G3D::uint32 &tileX, G3D::uint32 &tileY) { tileX = ID>>16; tileY = ID&0xFF; }

            StaticMapTree(G3D::uint32 mapID, const std::string &tilePath, const std::string fileName);
            ~StaticMapTree();

            bool isInLineOfSight(const G3D::Vector3& pos1, const G3D::Vector3& pos2) const;
            bool getObjectHitPos(const G3D::Vector3& pos1, const G3D::Vector3& pos2, G3D::Vector3& pResultHitPos, float pModifyDist) const;
            float getHeight(const G3D::Vector3& pPos, float maxSearchDist) const;
            G3D::uint32 GetVmapFlags(G3D::Vector3& pos);
            bool getAreaInfo(G3D::Vector3 &pos, G3D::uint32 &flags, G3D::int32 &adtId, G3D::int32 &rootId, G3D::int32 &groupId) const;
            bool GetLocationInfo(const G3D::Vector3 &pos, LocationInfo &info) const;

            bool InitMap(VMapManager* vm, bool loadAll);
            void UnloadMap(VMapManager* vm);
            bool LoadMapTile(G3D::uint32 tileX, G3D::uint32 tileY, VMapManager* vm, FILE *input = NULL);
            void UnloadMapTile(G3D::uint32 tileX, G3D::uint32 tileY, VMapManager* vm, FILE *input = NULL);
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
