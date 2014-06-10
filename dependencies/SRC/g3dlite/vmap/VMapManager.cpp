/***
 * Demonstrike Core
 */

#include "../G3DAll.h"
#include "VMapDefinitions.h"

#include <iomanip>

using G3D::Vector3;

namespace VMAP
{
    VMapManager::VMapManager(std::string vMapDir) : vmapDir(vMapDir)
    {
        if(vmapDir.length() > 0 && vmapDir.at(vmapDir.length()-1) != '\\' && vmapDir.at(vmapDir.length()-1) != '/')
            vmapDir.push_back('/');
    }

    VMapManager::~VMapManager(void)
    {
        for (InstanceTreeMap::iterator i = iInstanceMapTrees.begin(); i != iInstanceMapTrees.end(); ++i)
        {
            delete i->second;
        }
        for (DynamicTreeMap::iterator i = iDynamicMapTrees.begin(); i != iDynamicMapTrees.end(); ++i)
        {
            delete i->second;
        }
        for (ModelFileMap::iterator i = iLoadedModelFiles.begin(); i != iLoadedModelFiles.end(); ++i)
        {
            delete i->second.getModel();
        }
    }

    Vector3 VMapManager::convertPositionToInternalRep(float x, float y, float z)
    {
        static float mid = 0.5f * 64.0f * 533.33333333f;
        Vector3 pos(mid - x, mid - y, z);
        return pos;
    }

    // move to MapTree too?
    std::string VMapManager::getMapFileName(unsigned int mapId)
    {
        std::stringstream fname;
        fname.width(3);
        fname << std::setfill('0') << mapId << std::string(MAP_FILENAME_EXTENSION2);

        return fname.str();
    }

    bool VMapManager::loadMap(unsigned int mapId)
    {
        InstanceTreeMap::iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree == iInstanceMapTrees.end())
        {
            StaticMapTree* newTree = new StaticMapTree(mapId, vmapDir);
            if (!newTree->InitMap(getMapFileName(mapId), this))
            {
                delete newTree;
                return false;
            }

            iInstanceMapTrees.insert(InstanceTreeMap::value_type(mapId, newTree));
        }
        return true;
    }

    void VMapManager::unloadMap(unsigned int mapId)
    {
        InstanceTreeMap::iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree != iInstanceMapTrees.end())
        {
            instanceTree->second->UnloadMap(this);
            if (instanceTree->second->numLoadedTiles() == 0)
            {
                delete instanceTree->second;
                iInstanceMapTrees.erase(mapId);
            }
        }
    }

    void VMapManager::unloadMap(unsigned int mapId, int x, int y)
    {
        InstanceTreeMap::iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree == iInstanceMapTrees.end())
            return;
        instanceTree->second->UnloadMapTile(x, y, this);
    }

    // load one tile (internal use only)
    bool VMapManager::loadMap(unsigned int mapId, int x, int y)
    {
        InstanceTreeMap::iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree == iInstanceMapTrees.end())
            return false;

        return instanceTree->second->LoadMapTile(x, y, this);
    }

    // load one tile (internal use only)
    bool VMapManager::loadObject(G3D::g3d_uint64 guid, unsigned int mapId, G3D::g3d_uint32 DisplayID, float scale, float x, float y, float z, float o, G3D::g3d_uint32 instanceId, G3D::g3d_int32 m_phase)
    {
        DynamicTreeMap::iterator DIT = iDynamicMapTrees.find(std::make_pair(mapId, instanceId));
        if (DIT == iDynamicMapTrees.end())
        {
            DynamicMapTree* Tree = new DynamicMapTree();
            DIT = iDynamicMapTrees.insert(DynamicTreeMap::value_type(std::make_pair(mapId, instanceId), Tree)).first;
        }
        if(DIT->second == NULL)
            return false; // Shouldn't happen.

        return _loadObject(DIT->second, guid, mapId, DisplayID, scale, x, y, z, o, m_phase);
    }

    // Load our object into our dynamic tree(Internal only please!)
    bool VMapManager::_loadObject(DynamicMapTree* tree, G3D::g3d_uint64 guid, unsigned int mapId, G3D::g3d_uint32 DisplayID, float scale, float x, float y, float z, float o, G3D::g3d_int32 m_phase)
    {
        GOModelSpawnList::const_iterator it = GOModelList.find(DisplayID);
        if (it == GOModelList.end())
            return false;

        G3D::AABox mdl_box(it->second.BoundBase);
        if (mdl_box == G3D::AABox::zero())
        {
            // ignore models with no bounds
            OUT_DEBUG("GameObject model %s has zero bounds, loading skipped", it->second.name.c_str());
            return false;
        }

        WorldModel* model = acquireModelInstance(it->second.name);
        if(model == NULL)
            return false; // Shouldn't happen.
        GameobjectModelInstance* Instance = new GameobjectModelInstance(it->second, model, m_phase);
        if(Instance == NULL)
        {
            releaseModelInstance(it->second.name);
            return false; // Shouldn't happen.
        }

        Instance->SetData(x, y, z, o, scale);
        GOMapGuides* guides = NULL;
        GOModelInstanceByGUID::iterator itr = GOModelTracker.find(mapId);
        if(GOModelTracker.find(mapId) == GOModelTracker.end())
        {
            guides = new GOMapGuides();
            GOModelTracker.insert(GOModelInstanceByGUID::value_type(mapId, guides));
        }
        else
            guides = GOModelTracker.at(mapId);

        guides->ModelsByGuid.insert(ModelGUIDEs::value_type(guid, Instance));
        tree->insert(*Instance);
        return true;
    }

    bool VMapManager::changeObjectModel(G3D::g3d_uint64 guid, unsigned int mapId, G3D::g3d_uint32 instanceId, G3D::g3d_uint32 DisplayID)
    {
        DynamicTreeMap::iterator DIT = iDynamicMapTrees.find(std::make_pair(mapId, instanceId));
        if (DIT == iDynamicMapTrees.end())
            return false;
        GOModelInstanceByGUID::iterator itr = GOModelTracker.find(mapId);
        if(itr == GOModelTracker.end())
            return false;

        GOMapGuides* guides = GOModelTracker.at(mapId);
        ModelGUIDEs::iterator itr2 = guides->ModelsByGuid.find(guid);
        if(itr2 == guides->ModelsByGuid.end())
            return false;

        GameobjectModelInstance *Instance = itr2->second;
        GOModelSpawnList::const_iterator it = GOModelList.find(DisplayID);
        if (it == GOModelList.end())
        {
            DIT->second->remove(*Instance);
            Instance->setUnloaded();
            DIT->second->insert(*Instance);
            return false;
        }
        else
        {
            G3D::AABox mdl_box(it->second.BoundBase);
            if (mdl_box == G3D::AABox::zero())
            {
                DIT->second->remove(*Instance);
                Instance->setUnloaded();
                DIT->second->insert(*Instance);
                return false;
            }

            WorldModel* model = acquireModelInstance(it->second.name);
            if(model == NULL)
            {
                DIT->second->remove(*Instance);
                Instance->setUnloaded();
                DIT->second->insert(*Instance);
                return false;
            }

            DIT->second->remove(*Instance);
            G3D::Vector3 pos = Instance->getPosition();
            Instance->LoadModel(model, mdl_box);
            Instance->SetData(pos.x, pos.y, pos.z, Instance->GetOrientation(), Instance->GetScale());
            DIT->second->insert(*Instance);
            return true;
        }
    }

    void VMapManager::unloadObject(unsigned int mapId, G3D::g3d_uint32 m_instance, G3D::g3d_uint64 guid)
    {
        GOModelInstanceByGUID::iterator itr = GOModelTracker.find(mapId);
        if(itr == GOModelTracker.end())
            return;
        GOMapGuides* guides = GOModelTracker.at(mapId);
        DynamicTreeMap::iterator DynamicTree = iDynamicMapTrees.find(std::make_pair(mapId, m_instance));
        if (DynamicTree != iDynamicMapTrees.end())
        {
            if(guides->ModelsByGuid.find(guid) != guides->ModelsByGuid.end())
            {
                GameobjectModelInstance* Instance = guides->ModelsByGuid.at(guid);
                DynamicTree->second->remove(*Instance);
                releaseModelInstance(Instance->name);
                guides->ModelsByGuid.erase(guid);
                delete Instance;
            }
        }

        if(guides->ModelsByGuid.size() == 0)
        {
            GOModelTracker.erase(mapId);
            delete guides;
        }
    }

    bool VMapManager::isInLineOfSight(unsigned int mapId, G3D::g3d_uint32 instanceId, G3D::g3d_int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2)
    {
        bool result = true;
        if(fuzzyEq(x1, x2) && fuzzyEq(y1, y2) && fuzzyEq(z1, z2))
            return result; // Save us some time.

        Vector3 dest = convertPositionToInternalRep(x2, y2, z2);
        InstanceTreeMap::iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree != iInstanceMapTrees.end())
        {
            Vector3 pos1 = convertPositionToInternalRep(x1, y1, z1);
            result = instanceTree->second->isInLineOfSight(pos1, dest);
        }
        dest = convertPositionToInternalRep(dest.x, dest.y, dest.z);

        // This is only a check for true or false so we only need to check when still in line of sight.
        if(result)
        {
            DynamicTreeMap::iterator DynamicTree = iDynamicMapTrees.find(std::make_pair(mapId, instanceId));
            if (DynamicTree != iDynamicMapTrees.end())
            {
                if(!DynamicTree->second->isInLineOfSight(x1, y1, z1, dest.x, dest.y, dest.z, m_phase))
                    result = false;
            }
        }
        return result;
    }

    /**
    get the hit position and return true if we hit something
    otherwise the result pos will be the dest pos
    */
    bool VMapManager::getObjectHitPos(unsigned int mapId, G3D::g3d_uint32 instanceId, G3D::g3d_int32 m_phase, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float &ry, float& rz, float modifyDist)
    {
        bool result = false;
        rx = x2; ry = y2; rz = z2;
        if(fuzzyEq(x1, x2) && fuzzyEq(y1, y2) && fuzzyEq(z1, z2))
            return result; // Save us some time.

        Vector3 start = convertPositionToInternalRep(x1, y1, z1);
        Vector3 dest = convertPositionToInternalRep(x2, y2, z2);
        InstanceTreeMap::iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree != iInstanceMapTrees.end())
            result = instanceTree->second->getObjectHitPos(start, dest, dest, modifyDist);

        dest = convertPositionToInternalRep(dest.x, dest.y, dest.z);
        DynamicTreeMap::iterator DynamicTree = iDynamicMapTrees.find(std::make_pair(mapId, instanceId));
        if (DynamicTree != iDynamicMapTrees.end())
        {
            start = convertPositionToInternalRep(start.x, start.y, start.z);
            result = DynamicTree->second->getObjectHitPos(start, dest, dest, modifyDist, m_phase);
            // No conversion needed for dynamic result
        }

        rx = dest.x;
        ry = dest.y;
        rz = dest.z;
        return result;
    }

    /**
    get height or INVALID_HEIGHT if no height available
    */

    float VMapManager::getHeight(unsigned int mapId, G3D::g3d_uint32 instanceId, G3D::g3d_int32 m_phase, float x, float y, float z, float maxSearchDist)
    {
        float height = VMAP_INVALID_HEIGHT;
        InstanceTreeMap::iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree != iInstanceMapTrees.end())
        {
            Vector3 pos = convertPositionToInternalRep(x, y, z);
            float height2 = instanceTree->second->getHeight(pos, maxSearchDist);
            if (!G3D::fuzzyEq(height2, G3D::inf()))
                height = height2; // No height
        }

        DynamicTreeMap::iterator DynamicTree = iDynamicMapTrees.find(std::make_pair(mapId, instanceId));
        if (DynamicTree != iDynamicMapTrees.end())
        {
            float height2 = DynamicTree->second->getHeight(x, y, z, maxSearchDist, m_phase);
            if (!G3D::fuzzyEq(height2, G3D::inf()))
                height = height2; // No height
        }

        return height;
    }

    bool VMapManager::getAreaInfo(unsigned int mapId, float x, float y, float& z, G3D::g3d_uint32& flags, G3D::g3d_int32& adtId, G3D::g3d_int32& rootId, G3D::g3d_int32& groupId) const
    {
        InstanceTreeMap::const_iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree != iInstanceMapTrees.end())
        {
            Vector3 pos = convertPositionToInternalRep(x, y, z);
            bool result = instanceTree->second->getAreaInfo(pos, flags, adtId, rootId, groupId);
            // z is not touched by convertPositionToInternalRep(), so just copy
            z = pos.z;
            return result;
        }

        return false;
    }

    void VMapManager::GetLiquidData(G3D::g3d_uint32 mapId, float x, float y, float z, G3D::g3d_uint16 &type, float &level) const
    {
        InstanceTreeMap::const_iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree != iInstanceMapTrees.end())
        {
            LocationInfo info;
            Vector3 pos = convertPositionToInternalRep(x, y, z);
            if (instanceTree->second->GetLocationInfo(pos, info))
            {
                if(info.hitInstance->GetLiquidLevel(pos, info, level))
                    type = info.hitModel->GetLiquidType();
            }
        }
    }

    WorldModel* VMapManager::acquireModelInstance(const std::string& filename)
    {
        //! Critical section, thread safe access to iLoadedModelFiles
        LoadedModelFilesLock.lock();

        ModelFileMap::iterator model = iLoadedModelFiles.find(filename);
        if (model == iLoadedModelFiles.end())
        {
            WorldModel* worldmodel = new WorldModel();
            if (!worldmodel->readFile(vmapDir + filename + ".vmo"))
            {
                LoadedModelFilesLock.unlock();
                OUT_DEBUG("VMapManager: could not load '%s%s.vmo'!", vmapDir.c_str(), filename.c_str());
                delete worldmodel;
                return NULL;
            }

            OUT_DEBUG("VMapManager: loading file '%s%s'", vmapDir.c_str(), filename.c_str());
            model = iLoadedModelFiles.insert(std::pair<std::string, ManagedModel>(filename, ManagedModel())).first;
            model->second.setModel(worldmodel);
        }
        model->second.incRefCount();
        LoadedModelFilesLock.unlock();
        return model->second.getModel();
    }

    void VMapManager::releaseModelInstance(const std::string &filename)
    {
        //! Critical section, thread safe access to iLoadedModelFiles
        LoadedModelFilesLock.lock();

        ModelFileMap::iterator model = iLoadedModelFiles.find(filename);
        if (model == iLoadedModelFiles.end())
        {
            LoadedModelFilesLock.unlock();
            OUT_DEBUG("VMapManager: trying to unload non-loaded file '%s'", filename.c_str());
            return;
        }

        if (model->second.decRefCount() == 0)
        {
            OUT_DEBUG("VMapManager: unloading file '%s'", filename.c_str());
            delete model->second.getModel();
            iLoadedModelFiles.erase(model);
        }
        LoadedModelFilesLock.unlock();
    }

    bool VMapManager::existsMap(const char* basePath, unsigned int mapId, int x, int y)
    {
        return StaticMapTree::CanLoadMap(std::string(basePath), mapId, x, y);
    }

    void VMapManager::updateDynamicMapTree(G3D::g3d_uint32 t_diff, G3D::g3d_int32 mapid, G3D::g3d_uint32 instanceId)
    {
        if(mapid == -1)
        {
            for(DynamicTreeMap::iterator itr = iDynamicMapTrees.begin(); itr != iDynamicMapTrees.end(); itr++)
                itr->second->update(t_diff);
        }
        else
        {
            DynamicTreeMap::iterator DynamicTree = iDynamicMapTrees.find(std::make_pair(mapid, instanceId));
            if (DynamicTree != iDynamicMapTrees.end())
                DynamicTree->second->update(t_diff);
        }
    }

    void VMapManager::LoadGameObjectModelList()
    {
        FILE* model_list_file = fopen((vmapDir + VMAP::GAMEOBJECT_MODELS).c_str(), "rb");
        if (!model_list_file)
        {
            OUT_DEBUG("Unable to open '%s' file.", VMAP::GAMEOBJECT_MODELS);
            return;
        }

        G3D::g3d_uint32 name_length, displayId;
        char buff[500];
        while (true)
        {
            Vector3 v1, v2;
            if (fread(&displayId, sizeof(G3D::g3d_uint32), 1, model_list_file) != 1)
                if (feof(model_list_file))  // EOF flag is only set after failed reading attempt
                    break;

            if (fread(&name_length, sizeof(G3D::g3d_uint32), 1, model_list_file) != 1
                || name_length >= sizeof(buff)
                || fread(&buff, sizeof(char), name_length, model_list_file) != name_length
                || fread(&v1, sizeof(Vector3), 1, model_list_file) != 1
                || fread(&v2, sizeof(Vector3), 1, model_list_file) != 1)
            {
                OUT_DEBUG("File '%s' seems to be corrupted!", VMAP::GAMEOBJECT_MODELS);
                break;
            }

            GameobjectModelSpawn iModel;
            iModel.name = std::string(buff, name_length);
            iModel.BoundBase = AABox(v1, v2);
            GOModelList.insert(GOModelSpawnList::value_type(displayId, iModel));
        }

        fclose(model_list_file);
        bLog.outDetail("Loaded %u GameObject models", G3D::g3d_uint32(GOModelList.size()));
    }

} // namespace VMAP
