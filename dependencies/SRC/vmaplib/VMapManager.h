/***
 * Demonstrike Core
 */

#pragma once

//===========================================================

#define MAP_FILENAME_EXTENSION2 ".vmtiletree"

#define FILENAMEBUFFER_SIZE 500

#define VMAP_MANAGER_VERSION "VMapManagerV2"

#define VMAP_INVALID_HEIGHT        -100000.0f            // for check
#define VMAP_MAX_HEIGHT             100000.0f            // for other checks

/**
This is the main Class to manage loading and unloading of maps, line of sight, height calculation and so on.
For each map or map tile to load it reads a directory file that contains the ModelContainer files used by this map or map tile.
Each global map or instance has its own dynamic BSP-Tree.
The loaded ModelContainers are included in one of these BSP-Trees.
Additionally a table to match map ids and map names is used.
*/
//===========================================================
namespace VMAP
{
    class StaticMapTree;
    class WorldModel;

    class ManagedModel
    {
        public:
            ManagedModel(WorldModel *model) : iModel(model), iRefCount(0), iInvalid(false) {}
            ~ManagedModel() { delete iModel; }

            WorldModel *getModel() { return iModel; }
            void incRefCount() { ++iRefCount; }
            int decRefCount() { return --iRefCount; }
            bool invalid() { return iInvalid; }
            void invalidate() { iInvalid = true; }
        protected:
            WorldModel *iModel;
            bool iInvalid;
            int iRefCount;
    };

    typedef std::map< G3D::uint64, GameobjectModelInstance*> ModelGUIDEs;
    struct GOMapGuides
    {
        ModelGUIDEs ModelsByGuid;
    };

    typedef std::map<G3D::uint32, GOMapGuides* > GOModelInstanceByGUID;
    typedef std::map<G3D::uint32, GameobjectModelSpawn> GOModelSpawnList;
    typedef std::map<G3D::uint32, StaticMapTree*> InstanceTreeMap;
    typedef std::map<G3D::uint32, DynamicMapTree*> SubDynamicTreeMap;
    typedef std::map<G3D::uint32, SubDynamicTreeMap> DynamicTreeMap;
    typedef std::map<std::string, ManagedModel*> ModelFileMap;

    class VMapManager : public VMapManagerExt
    {
        protected:
            std::string vMapObjDir;
            std::set<std::string> DisabledModels;
            GOModelInstanceByGUID GOModelTracker;

            // Tree to check collision
            std::set<G3D::uint32> iDummyMaps;
            ModelFileMap iLoadedModelFiles;
            DynamicTreeMap iDynamicMapTrees;
            InstanceTreeMap iInstanceMapTrees;
            GOModelSpawnList GOModelList;

            // Mutex for iLoadedModelFiles
            G3D::GMutex LoadedModelFilesLock, filenameMutexlock;
            std::map<std::string, std::pair<uint32, G3D::GMutex*>> iModelNameLocks;

            bool _loadObject(DynamicMapTree* tree, G3D::uint64 guid, unsigned int mapId, G3D::uint32 DisplayID, float scale, float x, float y, float z, float o, G3D::int32 m_phase);

        public:
            // public for debug
            static G3D::Vector3 convertPositionToInternalRep(float x, float y, float z);

            VMapManager(std::string objDir);
            ~VMapManager(void);

            const char *GetManagerVersion() { return VMAP_MANAGER_VERSION; }

            void updateDynamicMapTree(unsigned int t_diff, int mapid = -1);
            void LoadGameObjectModelList();

            // Static collision
            bool loadMap(unsigned int mapId, FILE *file);
            void unloadMap(unsigned int mapId);
            bool loadMap(unsigned int mapId, int x, int y, FILE *file);
            void unloadMap(unsigned int mapId, int x, int y);

            // Dynamic collision
            bool loadObject(unsigned long long guid, unsigned int mapId, unsigned int DisplayID, float scale, float x, float y, float z, float o, unsigned int m_instance, int m_phase);
            bool changeObjectModel(unsigned long long guid, unsigned int mapId, unsigned int m_instance, unsigned int DisplayID);
            void unloadObject(unsigned int mapId, unsigned int m_instance, unsigned long long guid);

            // Functionality
            bool isInLineOfSight(unsigned int mapId, unsigned int m_instance, int m_phase, float x1, float y1, float z1, float x2, float y2, float z2);
            /**
            fill the hit pos and return true, if an object was hit
            */
            bool getObjectHitPos(unsigned int mapId, unsigned int m_instance, int m_phase, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float& ry, float& rz, float modifyDist);
            float getHeight(unsigned int mapId, unsigned int m_instance, int m_phase, float x, float y, float z, float maxSearchDist);

            bool processCommand(char *) { return false; } // for debug and extensions

            bool getAreaInfo(unsigned int pMapId, float x, float y, float& z, unsigned int& flags, int& adtId, int& rootId, int& groupId) const;
            void GetLiquidData(unsigned int pMapId, float x, float y, float z, unsigned short &type, float &level) const;

            WorldModel* acquireModelInstance(std::string filename);
            void releaseModelInstance(std::string filename);

        public:
            void getInstanceMapTree(InstanceTreeMap &instanceMapTree);
    };
}
