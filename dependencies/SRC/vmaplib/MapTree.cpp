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

#include <g3dlite\G3D.h>
#include "VMapLib.h"
#include "VMapDefinitions.h"

#include <iomanip>

using G3D::Vector3;

namespace VMAP
{

    class MapRayCallback
    {
        public:
            MapRayCallback(ModelInstance* val): prims(val), hit(false) {}
            bool operator()(const G3D::Ray& ray, G3D::uint32 entry, float& distance, bool pStopAtFirstHit=true)
            {
                bool result = prims[entry].intersectRay(ray, distance, pStopAtFirstHit);
                if (result)
                    hit = true;
                return result;
            }
        bool didHit() { return hit; }
    protected:
        ModelInstance* prims;
        bool hit;
    };

    class WMODataCallback
    {
        public:
            WMODataCallback(ModelInstance* val, WMOData &data, G3D::int32 requiredFlags, G3D::int32 ignoreFlags): prims(val), wmoData(data), reqFlags(requiredFlags), ignFlags(ignoreFlags) {}
            void operator()(const Vector3& point, G3D::uint32 entry)
            {
                OUT_DEBUG("MapTree::WMODataCallback: trying to intersect '%s'", prims[entry].name.c_str());
                prims[entry].getWMOData(point, wmoData, reqFlags, ignFlags);
            }

            ModelInstance* prims;
            WMOData &wmoData;
            G3D::int32 reqFlags, ignFlags;
    };

    class AreaInfoCallback
    {
        public:
            AreaInfoCallback(ModelInstance* val): prims(val) {}
            void operator()(const Vector3& point, G3D::uint32 entry)
            {
                OUT_DEBUG("MapTree::AreaInfoCallback: trying to intersect '%s'", prims[entry].name.c_str());
                prims[entry].intersectPoint(point, aInfo);
            }

            ModelInstance* prims;
            AreaInfo aInfo;
    };

    class LocationInfoCallback
    {
        public:
            LocationInfoCallback(ModelInstance* val, LocationInfo &info): prims(val), locInfo(info), result(false) {}
            void operator()(const Vector3& point, G3D::uint32 entry)
            {
                OUT_DEBUG("MapTree::LocationInfoCallback: trying to intersect '%s'", prims[entry].name.c_str());
                if (prims[entry].GetLocationInfo(point, locInfo))
                    result = true;
            }

            ModelInstance* prims;
            LocationInfo &locInfo;
            bool result;
    };

    //=========================================================

    std::string StaticMapTree::getTileFileName(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY)
    {
        std::stringstream tilefilename;
        tilefilename.fill('0');
        tilefilename << std::setw(3) << mapID << '_';
        //tilefilename << std::setw(2) << tileX << '_' << std::setw(2) << tileY << ".vmtile";
        tilefilename << std::setw(2) << tileY << '_' << std::setw(2) << tileX << ".vmtile";
        return tilefilename.str();
    }

    void StaticMapTree::getWMOData(const G3D::Vector3 &pos, WMOData &data, G3D::int32 requiredFlags, G3D::int32 ignoreFlags) const
    {
        WMODataCallback intersectionCallBack(iTreeValues, data, requiredFlags, ignoreFlags);
        iTree.intersectPoint(pos, iTreeBounds, intersectionCallBack);
    }

    bool StaticMapTree::getAreaInfo(Vector3 &pos, G3D::uint32 &flags, G3D::int32 &adtId, G3D::int32 &rootId, G3D::int32 &groupId) const
    {
        AreaInfoCallback intersectionCallBack(iTreeValues);
        iTree.intersectPoint(pos, iTreeBounds, intersectionCallBack);
        if (intersectionCallBack.aInfo.result)
        {
            flags = intersectionCallBack.aInfo.flags;
            adtId = intersectionCallBack.aInfo.adtId;
            rootId = intersectionCallBack.aInfo.rootId;
            groupId = intersectionCallBack.aInfo.groupId;
            pos.z = intersectionCallBack.aInfo.ground_Z;
            return true;
        }
        return false;
    }

    bool StaticMapTree::GetLocationInfo(const Vector3 &pos, LocationInfo &info) const
    {
        LocationInfoCallback intersectionCallBack(iTreeValues, info);
        iTree.intersectPoint(pos, iTreeBounds, intersectionCallBack);
        return intersectionCallBack.result;
    }

    StaticMapTree::StaticMapTree(G3D::uint32 mapID) : iMapID(mapID), iTreeValues(0), iNTreeValues(0) { }

    //=========================================================
    //! Make sure to call unloadMap() to unregister acquired model references before destroying
    StaticMapTree::~StaticMapTree()
    {
        delete[] iTreeValues;
    }

    //=========================================================
    /**
    If intersection is found within pMaxDist, sets pMaxDist to intersection distance and returns true.
    Else, pMaxDist is not modified and returns false;
    */

    bool StaticMapTree::getIntersectionTime(const G3D::Ray& pRay, float &pMaxDist, bool pStopAtFirstHit) const
    {
        float distance = pMaxDist;
        MapRayCallback intersectionCallBack(iTreeValues);
        iTree.intersectRay(pRay, intersectionCallBack, distance, pStopAtFirstHit);
        if (intersectionCallBack.didHit())
            pMaxDist = distance;
        return intersectionCallBack.didHit();
    }
    //=========================================================

    bool StaticMapTree::isInLineOfSight(const Vector3& pos1, const Vector3& pos2) const
    {
        float maxDist = (pos2 - pos1).magnitude();
        // return false if distance is over max float, in case of cheater teleporting to the end of the universe
        if (maxDist == std::numeric_limits<float>::max() ||
            maxDist == std::numeric_limits<float>::infinity())
            return false;

        // valid map coords should *never ever* produce float overflow, but this would produce NaNs too
        assert(maxDist < std::numeric_limits<float>::max());
        // prevent NaN values which can cause BIH intersection to enter infinite loop
        if (maxDist < 1e-10f)
            return true;
        // direction with length of 1
        G3D::Ray ray = G3D::Ray::fromOriginAndDirection(pos1, (pos2 - pos1)/maxDist);
        if (getIntersectionTime(ray, maxDist, true))
            return false;

        return true;
    }
    //=========================================================
    /**
    When moving from pos1 to pos2 check if we hit an object. Return true and the position if we hit one
    Return the hit pos or the original dest pos
    */

    bool StaticMapTree::getObjectHitPos(const Vector3& pPos1, const Vector3& pPos2, Vector3& pResultHitPos, float pModifyDist) const
    {
        bool result=false;
        float maxDist = (pPos2 - pPos1).magnitude();
        // valid map coords should *never ever* produce float overflow, but this would produce NaNs too
        assert(maxDist < std::numeric_limits<float>::max());
        // prevent NaN values which can cause BIH intersection to enter infinite loop
        if (maxDist < 1e-10f)
        {
            pResultHitPos = pPos2;
            return false;
        }
        Vector3 dir = (pPos2 - pPos1)/maxDist;              // direction with length of 1
        G3D::Ray ray(pPos1, dir);
        float dist = maxDist;
        if (getIntersectionTime(ray, dist, false))
        {
            pResultHitPos = pPos1 + dir * dist;
            if (pModifyDist < 0)
            {
                if ((pResultHitPos - pPos1).magnitude() > -pModifyDist)
                {
                    pResultHitPos = pResultHitPos + dir*pModifyDist;
                }
                else
                {
                    pResultHitPos = pPos1;
                }
            }
            else
            {
                pResultHitPos = pResultHitPos + dir*pModifyDist;
            }
            result = true;
        }
        else
        {
            pResultHitPos = pPos2;
            result = false;
        }
        return result;
    }

    //=========================================================

    float StaticMapTree::getHeight(const Vector3& pPos, float maxSearchDist) const
    {
        float height = G3D::inf();
        Vector3 dir = Vector3(0, 0, -1);
        G3D::Ray ray(pPos, dir);   // direction with length of 1
        float maxDist = maxSearchDist;
        if (getIntersectionTime(ray, maxDist, false))
        {
            height = pPos.z - maxDist;
        }
        return(height);
    }

    //=========================================================

    bool StaticMapTree::InitMap(VMapManager* vm, FILE *rf)
    {
        OUT_DEBUG("StaticMapTree::InitMap() : initializing StaticMapTree '%03u'", iMapID);
        char chunk[10];
        if(!readChunk(rf, chunk, VMAP_MAGIC, 10))
        {
            OUT_ERROR("VMap magic does not match!");
            return false;
        }

        uint32 tileSize = 0;
        bool success = true;
        if(fread(&tileSize, sizeof(uint32), 1, rf) != 1)
            success = false;
        if(success && tileSize)
        {
            std::vector<uint32> availableTiles, tileOffsets;
            availableTiles.resize(tileSize/4), tileOffsets.resize(availableTiles.size());
            if(success && (fread(&availableTiles[0], 1, tileSize, rf) != tileSize))
                success = false;
            if(success && (fread(&tileOffsets[0], 1, tileSize, rf) != tileSize))
                success = false;
            for(uint32 i = 0; i < availableTiles.size(); i++)
                iFileOffsets.insert(std::make_pair(availableTiles[i], tileOffsets[i]));
        }

        G3D::uint32 spawnSize = 0;
        if(success && !readChunk(rf, chunk, "GOBJ", 4))
            success = false;
        if(success && fread(&spawnSize, sizeof(uint32), 1, rf) != 1)
            success = false;
        else
        {
            std::map<uint32, ModelSpawn> spawnMap;
            if(success)
            {
                OUT_DEBUG("StaticMapTree::InitMap() : global spawns: %u", spawnSize);
                // global model spawns
                // only non-tiled maps have them, and if so exactly one (so far at least...)
                for(uint32 i = 0; i < spawnSize; i++)
                {
                    ModelSpawn spawn;
                    if(success && !ModelSpawn::readFromFile(rf, spawn))
                    {
                        success = false;
                        break;
                    } else spawnMap.insert(std::make_pair(i, spawn));
                }
            }

            if(success && !readChunk(rf, chunk, "NODE", 4))
                success = false;
            if(success && !iTree.readFromFile(rf))
                success = false;
            else
            {
                iNTreeValues = iTree.primCount();
                iTreeValues = new ModelInstance[iNTreeValues];
                for(std::map<uint32, ModelSpawn>::iterator it = spawnMap.begin(); it != spawnMap.end(); it++)
                {
                    OUT_DEBUG("StaticMapTree::InitMap() : loading %s", it->second.name.c_str());
                    if (WorldModel* model = vm->acquireModelInstance(it->second.name))
                    {
                        // assume that global model always is the first and only tree value (could be improved...)
                        iTreeValues[it->first] = ModelInstance(it->second, model);
                        iLoadedSpawns[it->first] = 1;
                    }
                    else
                    {
                        success = false;
                        OUT_ERROR("StaticMapTree::InitMap() : could not acquire WorldModel pointer for '%s'", it->second.name.c_str());
                    }
                }
            }
        }

        if(success == false)
        {
            OUT_ERROR("StaticMapTree::InitMap() : failed reading data!");
            return false;
        }

        iTree.getBounds(iTreeBounds);
        if(iTreeBounds.volume() == 0.f)
            iTree.getMergeBounds(iTreeBounds);
        return true;
    }

    //=========================================================

    void StaticMapTree::UnloadMap(VMapManager* vm)
    {
        for (loadedSpawnMap::iterator i = iLoadedSpawns.begin(); i != iLoadedSpawns.end(); ++i)
        {
            iTreeValues[i->first].setUnloaded();
            for (G3D::uint32 refCount = 0; refCount < i->second; ++refCount)
                vm->releaseModelInstance(iTreeValues[i->first].name);
        }
        iLoadedSpawns.clear();
        iLoadedTiles.clear();
    }

    //=========================================================

    bool StaticMapTree::LoadMapTile(G3D::uint32 tileX, G3D::uint32 tileY, FILE *input, VMapManager* vm)
    {
        G3D::uint32 packedTile = packTileID(tileX, tileY);
        if(iFileOffsets.find(packedTile) == iFileOffsets.end())
            return false;
        if (!iTreeValues)
        {
            OUT_DEBUG("StaticMapTree::LoadMapTile() : tree has not been initialized [%u, %u]", tileX, tileY);
            return false;
        }

        if(iLoadedTiles.find(packedTile) != iLoadedTiles.end())
            return true;

        if(fseek(input, iFileOffsets.at(packedTile), SEEK_SET) != 0)
        {
            OUT_ERROR("StaticMapTree::LoadMapTile() : could not seek to tile offset [%u, %u]", tileX, tileY);
            return false;
        }

        bool result = true;
        G3D::uint32 numSpawns = 0;
        if (fread(&numSpawns, sizeof(G3D::uint32), 1, input) != 1)
            result = false;

        std::vector<std::pair<uint32, ModelSpawn>> modelSpawns;
        for (G3D::uint32 i=0; i<numSpawns && result; ++i)
        {
            // read model spawns
            ModelSpawn spawn;
            G3D::uint32 referencedVal;
            result = ModelSpawn::readFromFile(input, spawn);
            if(result == false)
            {
                OUT_DEBUG("StaticMapTree::LoadMapTile() : could not acquire WorldModel file [%u, %u]", tileX, tileY);
                break;
            }
            else if (fread(&referencedVal, sizeof(G3D::uint32), 1, input) != 1)
            {
                result = false;
                break;
            }
            modelSpawns.push_back(std::make_pair(referencedVal, spawn));
        }
        if(result)
            iLoadedTiles[packedTile] = true;

        for (auto itr = modelSpawns.begin(); itr != modelSpawns.end(); itr++)
        {   // acquire model instance
            if(WorldModel* model = vm->acquireModelInstance((*itr).second.name))
            {
                if (iLoadedSpawns.find((*itr).first) == iLoadedSpawns.end())
                {
                    if ((*itr).first > iNTreeValues)
                    {
                        OUT_DEBUG("StaticMapTree::LoadMapTile() : invalid tree element (%u/%u)", (*itr).first, iNTreeValues);
                        continue;
                    }

                    iTreeValues[(*itr).first] = ModelInstance((*itr).second, model);
                    iLoadedSpawns[(*itr).first] = 1;
                }
                else
                {
                    ++iLoadedSpawns[(*itr).first];
                    if (iTreeValues[(*itr).first].ID != (*itr).second.ID)
                        OUT_DEBUG("StaticMapTree::LoadMapTile() : trying to load wrong spawn in node");
                    else if (iTreeValues[(*itr).first].name != (*itr).second.name)
                        OUT_DEBUG("StaticMapTree::LoadMapTile() : name collision on GUID=%u", (*itr).second.ID);
                }

                loadedData[packedTile].push_back(std::make_pair((*itr).first, (*itr).second.name));
            } else OUT_DEBUG("StaticMapTree::LoadMapTile() : could not acquire WorldModel pointer [%u, %u]", tileX, tileY);
        }

        modelSpawns.clear();
        return result;
    }

    //=========================================================

    void StaticMapTree::UnloadMapTile(G3D::uint32 tileX, G3D::uint32 tileY, VMapManager* vm)
    {
        G3D::uint32 packedTile = packTileID(tileX, tileY);
        loadedTileMap::iterator tile = iLoadedTiles.find(packedTile);
        if (tile == iLoadedTiles.end())
        {
            OUT_DEBUG("StaticMapTree::UnloadMapTile() : trying to unload non-loaded tile - Map:%u X:%u Y:%u", iMapID, tileX, tileY);
            return;
        }

        if(loadedData[packedTile].empty())
            return; /// Empty tile

        while(!loadedData[packedTile].empty())
        {
            uint32 ref = loadedData[packedTile].begin()->first;
            std::string spawnName = loadedData[packedTile].begin()->second;
            // release model instance
            vm->releaseModelInstance(spawnName);
            if (!iLoadedSpawns.count(ref))
                OUT_DEBUG("StaticMapTree::UnloadMapTile() : trying to unload non-referenced model '%s'", spawnName.c_str());
            else if (--iLoadedSpawns[ref] == 0)
            {
                iTreeValues[ref].setUnloaded();
                iLoadedSpawns.erase(ref);
            }

            loadedData[packedTile].erase(loadedData[packedTile].begin());
        }
    }
}
