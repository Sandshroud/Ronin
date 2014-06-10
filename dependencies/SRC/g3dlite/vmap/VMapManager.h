/***
 * Demonstrike Core
 */

#pragma once

//===========================================================

#define MAP_FILENAME_EXTENSION2 ".vmtree"

#define FILENAMEBUFFER_SIZE 500

#define VMAP_MANAGER_VERSION "VMapManagerV2"

#define VMAP_INVALID_HEIGHT        -100000.0f            // for check
#define VMAP_MAX_HEIGHT             100000.0f            // for other checks

#ifdef DEBUG
#define OUT_DEBUG bLog.outDebug
#else
#define OUT_DEBUG
#endif

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
            ManagedModel() : iModel(0), iRefCount(0) {}
            void setModel(WorldModel *model) { iModel = model; }
            WorldModel *getModel() { return iModel; }
            void incRefCount() { ++iRefCount; }
            int decRefCount() { return --iRefCount; }
        protected:
            WorldModel *iModel;
            int iRefCount;
    };

    typedef std::map< G3D::g3d_uint64, GameobjectModelInstance*> ModelGUIDEs;
    struct GOMapGuides
    {
        ModelGUIDEs ModelsByGuid;
    };

    typedef std::map<G3D::g3d_uint32, GOMapGuides* > GOModelInstanceByGUID;
    typedef std::map<G3D::g3d_uint32, GameobjectModelSpawn> GOModelSpawnList;
    typedef std::map<std::pair<G3D::g3d_uint32, G3D::g3d_uint32>, DynamicMapTree*> DynamicTreeMap;
    typedef std::map<G3D::g3d_uint32, StaticMapTree*> InstanceTreeMap;
    typedef std::map<std::string, ManagedModel> ModelFileMap;

    class VMapManager
    {
        protected:
            std::string vmapDir;
            GOModelInstanceByGUID GOModelTracker;

            // Tree to check collision
            ModelFileMap iLoadedModelFiles;
            DynamicTreeMap iDynamicMapTrees;
            InstanceTreeMap iInstanceMapTrees;
            GOModelSpawnList GOModelList;

            // Mutex for iLoadedModelFiles
            G3D::GMutex LoadedModelFilesLock;

            bool _loadObject(DynamicMapTree* tree, G3D::g3d_uint64 guid, unsigned int mapId, G3D::g3d_uint32 DisplayID, float scale, float x, float y, float z, float o, G3D::g3d_int32 m_phase);

        public:
            // public for debug
            static G3D::Vector3 convertPositionToInternalRep(float x, float y, float z);
            static std::string getMapFileName(unsigned int mapId);

            VMapManager(std::string vMapDirectory);
            ~VMapManager(void);

            const char *GetManagerVersion() { return VMAP_MANAGER_VERSION; }

            void updateDynamicMapTree(G3D::g3d_uint32 t_diff, G3D::g3d_int32 mapid = -1, G3D::g3d_uint32 instanceId = 0);
            void LoadGameObjectModelList();

            // Static collision
            bool loadMap(unsigned int mapId);
            void unloadMap(unsigned int mapId);
            bool loadMap(unsigned int mapId, int x, int y);
            void unloadMap(unsigned int mapId, int x, int y);

            // Dynamic collision
            bool loadObject(G3D::g3d_uint64 guid, unsigned int mapId, G3D::g3d_uint32 DisplayID, float scale, float x, float y, float z, float o, G3D::g3d_uint32 m_instance, G3D::g3d_int32 m_phase);
            bool changeObjectModel(G3D::g3d_uint64 guid, unsigned int mapId, G3D::g3d_uint32 m_instance, G3D::g3d_uint32 DisplayID);
            void unloadObject(unsigned int mapId, G3D::g3d_uint32 m_instance, G3D::g3d_uint64 guid);

            // Functionality
            bool isInLineOfSight(unsigned int mapId, G3D::g3d_uint32 m_instance, G3D::g3d_int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2);
            /**
            fill the hit pos and return true, if an object was hit
            */
            bool getObjectHitPos(unsigned int mapId, G3D::g3d_uint32 m_instance, G3D::g3d_int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float& ry, float& rz, float modifyDist);
            float getHeight(unsigned int mapId, G3D::g3d_uint32 m_instance, G3D::g3d_int32 m_phase, float x, float y, float z, float maxSearchDist);

            bool processCommand(char *) { return false; } // for debug and extensions

            bool getAreaInfo(unsigned int pMapId, float x, float y, float& z, G3D::g3d_uint32& flags, G3D::g3d_int32& adtId, G3D::g3d_int32& rootId, G3D::g3d_int32& groupId) const;
            void GetLiquidData(G3D::g3d_uint32 pMapId, float x, float y, float z, G3D::g3d_uint16 &type, float &level) const;

            WorldModel* acquireModelInstance(const std::string& filename);
            void releaseModelInstance(const std::string& filename);

            // what's the use of this? o.O
            virtual std::string getDirFileName(unsigned int mapId, int /*x*/, int /*y*/) const
            {
                return getMapFileName(mapId);
            }
            virtual bool existsMap(const char* basePath, unsigned int mapId, int x, int y);
        public:
            void getInstanceMapTree(InstanceTreeMap &instanceMapTree);
    };
}
