/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2013-2017 Sandshroud <https://github.com/Sandshroud>
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

#include "PathCommon.h"
#include "MapBuilder.h"

#include <vmaplib/VMapLib.h>
#include <threading/Threading.h>

#include "DetourNavMeshBuilder.h"
#include "DetourNavMesh.h"
#include "DetourCommon.h"

#define MMAP_MAGIC 0x4d4d4150   // 'MMAP'
#define MMAP_VERSION 5

time_t UNIXTIME;
tm g_localTime;

const char *heightMapHeader = "HMAP434_2";

struct MmapTileHeader
{
    G3D::uint32 mmapMagic;
    G3D::uint32 dtVersion;
    G3D::uint32 mmapVersion;
    G3D::uint32 size;
    G3D::uint32 hasNextLayer;

    MmapTileHeader() : mmapMagic(MMAP_MAGIC), dtVersion(DT_NAVMESH_VERSION),
        mmapVersion(MMAP_VERSION), size(0), hasNextLayer(0) {}
};

namespace MMAP
{
    MapBuilder::MapBuilder(float maxWalkableAngle, bool skipLiquid,
        bool skipContinents, bool skipJunkMaps, bool skipBattlegrounds,
        bool debugOutput, bool bigBaseUnit, const char* offMeshFilePath) :
        m_terrainBuilder     (NULL),
        m_debugOutput        (debugOutput),
        m_offMeshFilePath    (offMeshFilePath),
        m_skipContinents     (skipContinents),
        m_skipJunkMaps       (skipJunkMaps),
        m_skipBattlegrounds  (skipBattlegrounds),
        m_maxWalkableAngle   (maxWalkableAngle),
        m_bigBaseUnit        (bigBaseUnit),
        m_rcContext          (NULL)
    {
        m_terrainBuilder = new TerrainBuilder(skipLiquid);

        m_rcContext = new rcContext(false);

        discoverTiles();
    }

    /**************************************************************************/
    MapBuilder::~MapBuilder()
    {
        for (TileList::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
        {
            (*it).second->clear();
            delete (*it).second;
        }

        delete m_terrainBuilder;
        delete m_rcContext;
    }

    /**************************************************************************/
    void MapBuilder::discoverTiles()
    {
        std::vector<std::string> files;
        G3D::uint32 mapID, tileX, tileY, tileID, count = 0;
        char filter[12];

        printf("Discovering maps... ");
        getDirContents(files, "Tiles", "*.tiletree");
        for (G3D::uint32 i = 0; i < files.size(); ++i)
        {
            mapID = G3D::uint32(atoi(files[i].substr(0,3).c_str()));
            if (m_tiles.find(mapID) == m_tiles.end())
            {
                m_tiles.insert(std::make_pair(mapID, new std::set<G3D::uint32>));
                count++;
            }
        }

        files.clear();
        printf("found %u.\n", count);

        count = 0;
        printf("Discovering tiles... ");
        for (TileList::iterator itr = m_tiles.begin(); itr != m_tiles.end(); ++itr)
        {
            mapID = (*itr).first;
            std::set<G3D::uint32>* tiles = itr->second;

            char FileName[255];
            sprintf(FileName, "Tiles/%03u.tiletree", mapID);

            FILE* mapFile = NULL;
            fopen_s(&mapFile, FileName, "rb");
            if(mapFile == NULL)
                continue;

            char identifier[10];
            if(fread(identifier, 10, 1, mapFile) != 1)
                continue;
            if(strcmp(identifier, heightMapHeader))
                continue;

            G3D::uint32 offsets[64][64];
            if(fread(offsets, 1, 16384, mapFile) != 16384)
                continue;

            for(G3D::uint8 tileY = 0; tileY < 64; tileY++)
            {
                for(G3D::uint8 tileX = 0; tileX < 64; tileX++)
                {
                    if(!offsets[tileX][tileY])
                        continue;

                    tileID = packTileID(tileX, tileY);
                    if (tiles->insert(tileID).second)
                        count++;
                }
            }
        }
        printf("found %u.\n\n", count);
    }

    /**************************************************************************/
    std::set<G3D::uint32>* MapBuilder::getTileList(G3D::uint32 mapID)
    {
        TileList::iterator itr = m_tiles.find(mapID);
        if (itr != m_tiles.end())
            return (*itr).second;

        std::set<G3D::uint32>* tiles = new std::set<G3D::uint32>();
        m_tiles.insert(std::pair<G3D::uint32, std::set<G3D::uint32>*>(mapID, tiles));
        return tiles;
    }

    /**************************************************************************/
    void MapBuilder::buildAllMaps(ThreadTaskList &threadList)
    {
        for (TileList::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
        {
            G3D::uint32 mapID = it->first;
            if (!shouldSkipMap(mapID))
            {
                buildMap(mapID, threadList);
            }
        }

        threadList.wait(UNIXTIME, g_localTime);
        printf("Job Complete!                                    \n");
    }

    /**************************************************************************/
    void MapBuilder::getGridBounds(G3D::uint32 mapID, G3D::uint32 &minX, G3D::uint32 &minY, G3D::uint32 &maxX, G3D::uint32 &maxY)
    {
        maxX = INT_MAX;
        maxY = INT_MAX;
        minX = INT_MIN;
        minY = INT_MIN;

        float bmin[3] = { 0, 0, 0 };
        float bmax[3] = { 0, 0, 0 };
        MapTileLoadData loadData;

        // make sure we process maps which don't have tiles
        // initialize the static tree, which loads WDT models
        if(!m_terrainBuilder->loadVMap(mapID, 64, 64, loadData))
            return;

        // get the coord bounds of the model data
        if (loadData.allVerts.size() == 0)
            return;

        rcCalcBounds(loadData.allVerts.getCArray(), loadData.allVerts.size() / 3, bmin, bmax);

        // convert coord bounds to grid bounds
        maxX = 32 - bmin[0] / GRID_SIZE;
        maxY = 32 - bmin[2] / GRID_SIZE;
        minX = 32 - bmax[0] / GRID_SIZE;
        minY = 32 - bmax[2] / GRID_SIZE;
    }

    void MapBuilder::buildMeshFromFile(char* name, ThreadTaskList &threadList)
    {
        FILE* file = fopen(name, "rb");
        if (!file)
            return;

        printf("Building mesh from file\n");
        int tileX, tileY, mapId;
        if (fread(&mapId, sizeof(int), 1, file) != 1)
        {
            fclose(file);
            return;
        }
        if (fread(&tileX, sizeof(int), 1, file) != 1)
        {
            fclose(file);
            return;
        }
        if (fread(&tileY, sizeof(int), 1, file) != 1)
        {
            fclose(file);
            return;
        }

        dtNavMesh* navMesh = NULL;
        buildNavMesh(mapId, navMesh);
        if (!navMesh)
        {
            printf("Failed creating navmesh!              \n");
            fclose(file);
            return;
        }

        G3D::uint32 verticesCount, indicesCount;
        if (fread(&verticesCount, sizeof(G3D::uint32), 1, file) != 1)
        {
            fclose(file);
            return;
        }

        if (fread(&indicesCount, sizeof(G3D::uint32), 1, file) != 1)
        {
            fclose(file);
            return;
        }

        float* verts = new float[verticesCount];
        int* inds = new int[indicesCount];

        if (fread(verts, sizeof(float), verticesCount, file) != verticesCount)
        {
            fclose(file);
            delete[] verts;
            delete[] inds;
            return;
        }

        if (fread(inds, sizeof(int), indicesCount, file) != indicesCount)
        {
            fclose(file);
            delete[] verts;
            delete[] inds;
            return;
        }

        MapTileLoadData loadData;

        for (G3D::uint32 i = 0; i < verticesCount; ++i)
            loadData.allVerts.append(verts[i]);
        delete[] verts;

        for (G3D::uint32 i = 0; i < indicesCount; ++i)
            loadData.allTris.append(inds[i]);
        delete[] inds;

        TerrainBuilder::cleanVertices(loadData.allVerts, loadData.allTris);
        // get bounds of current tile
        float bmin[3], bmax[3];
        getTileBounds(tileX, tileY, loadData.allVerts.getCArray(), loadData.allVerts.size() / 3, bmin, bmax);

        // build navmesh tile
        buildMoveMapTerrainTile(mapId, tileX, tileY, loadData, bmin, bmax, navMesh, threadList);
        fclose(file);
    }

    /**************************************************************************/
    void MapBuilder::buildSingleTile(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, ThreadTaskList &threadList)
    {
        dtNavMesh* navMesh = NULL;
        buildNavMesh(mapID, navMesh);
        if (!navMesh)
        {
            printf("Failed creating navmesh!              \n");
            return;
        }

        m_terrainBuilder->InitializeMap(mapID);
        buildTile(mapID, tileX, tileY, navMesh, threadList);
        m_terrainBuilder->UnloadVMap(mapID);
        dtFreeNavMesh(navMesh);
    }

    /**************************************************************************/
    void MapBuilder::buildMap(G3D::uint32 mapID, ThreadTaskList &threadList)
    {
        printf("Building map %03u:                                          \n\n", mapID);
        m_terrainBuilder->InitializeMap(mapID);

        std::set<G3D::uint32>* tiles = getTileList(mapID);

        // make sure we process maps which don't have tiles
        if (!tiles->size())
        {
            // convert coord bounds to grid bounds
            G3D::uint32 minX, minY, maxX, maxY;
            getGridBounds(mapID, minX, minY, maxX, maxY);

            // add all tiles within bounds to tile list.
            for (G3D::uint32 i = minX; i <= maxX; ++i)
                for (G3D::uint32 j = minY; j <= maxY; ++j)
                    tiles->insert(packTileID(i, j));
        }

        if (!tiles->empty())
        {
            // build navMesh
            dtNavMesh* navMesh = NULL;
            buildNavMesh(mapID, navMesh);
            if(navMesh == NULL)
                printf("Failed creating navmesh for %u!      \n\n", mapID);
            else
            {
                // now start building mmtiles for each tile
                for (std::set<G3D::uint32>::iterator it = tiles->begin(); it != tiles->end(); ++it)
                {
                    G3D::uint32 tileX, tileY;

                    // unpack tile coords
                    unpackTileID((*it), tileX, tileY);
                    if (shouldSkipTile(mapID, tileX, tileY))
                        continue;

                    buildTile(mapID, tileX, tileY, navMesh, threadList);
                }

                dtFreeNavMesh(navMesh);
            }
        }

        m_terrainBuilder->UnloadVMap(mapID);
    }

    static std::string dialogBox;

    /**************************************************************************/
    void MapBuilder::buildTile(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, dtNavMesh* navMesh, ThreadTaskList &threadList)
    {
        MapTileLoadData loadData;

        bool res = true;

        // get heightmap data
        res = m_terrainBuilder->loadMap(mapID, tileX, tileY, loadData);

        // get model data
        if(m_terrainBuilder->loadVMap(mapID, tileX, tileY, loadData) == false && res == false)
            return;

        // get bounds of current tile
        float bmin[3], bmax[3];
        getTileBounds(tileX, tileY, loadData, bmin, bmax);

        // build navmesh tile
        buildMoveMapTerrainTile(mapID, tileX, tileY, loadData, bmin, bmax, navMesh, threadList);
    }

    /**************************************************************************/
    void MapBuilder::buildNavMesh(G3D::uint32 mapID, dtNavMesh* &navMesh)
    {
        std::set<G3D::uint32>* tiles = getTileList(mapID);

        // old code for non-statically assigned bitmask sizes:
        ///*** calculate number of bits needed to store tiles & polys ***/
        //int tileBits = dtIlog2(dtNextPow2(tiles->size()));
        //if (tileBits < 1) tileBits = 1;                                     // need at least one bit!
        //int polyBits = sizeof(dtPolyRef)*8 - SALT_MIN_BITS - tileBits;

        int polyBits = DT_POLY_BITS;

        int maxTiles = tiles->size();
        int maxPolysPerTile = 1 << polyBits;

        /***          calculate bounds of map         ***/

        G3D::uint32 tileXMin = 64, tileYMin = 64, tileXMax = 0, tileYMax = 0, tileX, tileY;
        for (std::set<G3D::uint32>::iterator it = tiles->begin(); it != tiles->end(); ++it)
        {
            unpackTileID(*it, tileX, tileY);

            if (tileX > tileXMax)
                tileXMax = tileX;
            else if (tileX < tileXMin)
                tileXMin = tileX;

            if (tileY > tileYMax)
                tileYMax = tileY;
            else if (tileY < tileYMin)
                tileYMin = tileY;
        }

        // use Max because '32 - tileX' is negative for values over 32
        float bmin[3], bmax[3];
        getTileBounds(tileXMax, tileYMax, NULL, 0, bmin, bmax);

        /***       now create the navmesh       ***/

        // navmesh creation params
        dtNavMeshParams navMeshParams;
        memset(&navMeshParams, 0, sizeof(dtNavMeshParams));
        navMeshParams.tileWidth = GRID_SIZE;
        navMeshParams.tileHeight = GRID_SIZE;
        rcVcopy(navMeshParams.orig, bmin);
        navMeshParams.maxTiles = maxTiles;
        navMeshParams.maxPolys = maxPolysPerTile;

        navMesh = dtAllocNavMesh();
        //printf("Creating navMesh...                     \r");
        if (!navMesh->init(&navMeshParams))
        {
            printf("Failed creating navmesh!                \n");
            return;
        }

        char fileName[25];
        sprintf(fileName, "mmaps/%03u.mmap", mapID);

        FILE* file = fopen(fileName, "wb");
        if (!file)
        {
            dtFreeNavMesh(navMesh);
            char message[1024];
            sprintf(message, "Failed to open %s for writing!\n", fileName);
            perror(message);
            return;
        }

        // now that we know navMesh params are valid, we can write them to file
        fwrite(&navMeshParams, sizeof(dtNavMeshParams), 1, file);
        fclose(file);
    }

    inline bool overlapBounds(const float* amin, const float* amax, const float* bmin, const float* bmax)
    {
        bool overlap = true;
        overlap = (amin[0] > bmax[0] || amax[0] < bmin[0]) ? false : overlap;
        overlap = (amin[1] > bmax[1] || amax[1] < bmin[1]) ? false : overlap;
        overlap = (amin[2] > bmax[2] || amax[2] < bmin[2]) ? false : overlap;
        return overlap;
    }

    // Simple, we input our terrain tris/verts and vmap tris/verts and filter the terrain areas where vmap exists
    bool MapBuilder::filterObjectsFromTriangles(G3D::Array<float> &tVertArray, G3D::Array<int> &tTriArray, G3D::Array<int> &tTriFlags, G3D::Array<G3D::AABox*> &vmapData)
    {
        if(vmapData.size() == 0)
            return true;

        int numTris = tTriArray.size()/3;

        bool ret = false;
        for(int i = 0; i < numTris; i++)
        {
            const float* v0 = &tVertArray[tTriArray[i*3+0]*3];
            const float* v1 = &tVertArray[tTriArray[i*3+1]*3];
            const float* v2 = &tVertArray[tTriArray[i*3+2]*3];
            // Bound boxes are in X,Z,Y
            G3D::Vector3 point0 = G3D::Vector3(v0[0], v0[1], v0[2]),
                point1 = G3D::Vector3(v1[0], v1[1], v1[2]),
                point2 = G3D::Vector3(v2[0], v2[1], v2[2]);

            // Scan our vmap mesh bounding boxes to see if any terrain is contained within
            for(int j = 0; j < vmapData.size(); j++)
            {
                const G3D::AABox *objBox = vmapData[j];
                if(objBox->contains(point0) || objBox->contains(point1) || objBox->contains(point2))
                {
                    tTriFlags[i] = NAV_EMPTY; // Mark triangle as empty, should be marking as offmesh connection
                }
            }

            // We haven't removed this triangle from mesh so we're returning with valid terrain mesh
            if(tTriFlags[i] != NAV_EMPTY)
                ret = true;
        }

        return ret;
    }

    static Mutex fileLock;

    enum layerTypes
    {   // Just differentiate
        LAYER_TYPE_TERRAIN = 0,
        LAYER_TYPE_VMAP,
        MAX_LAYER_TYPE
    };

    void buildTrisVerticesFlagsFromLoadData(MapTileLoadData &loadData, int layer, int layerType, G3D::Array<float> &tVertArray, G3D::Array<int> &tTriArray, G3D::Array<int> &tTriFlags, G3D::Array<G3D::AABox*> &vmapData)
    {
        switch(layerType)
        {
        case LAYER_TYPE_TERRAIN:
            {
                switch(layer)
                {
                case 0:
                    {
                        tVertArray.append(loadData.terrainData[0].objVerts);
                        tTriArray.append(loadData.terrainData[0].objTris);
                        tTriFlags.append(loadData.terrainData[0].objFlags);
                        for(auto itr = loadData.vmapData.begin(); itr != loadData.vmapData.end(); itr++)
                            vmapData.append(&(*itr)->boundBox);
                    }break;
                case 1:
                    {
                        tVertArray.append(loadData.terrainData[1].objVerts);
                        tTriArray.append(loadData.terrainData[1].objTris);
                        tTriFlags.append(loadData.terrainData[1].objFlags);
                        for(auto itr = loadData.vmapData.begin(); itr != loadData.vmapData.end(); itr++)
                            vmapData.append(&(*itr)->boundBox);
                    }break;
                }break;
            }break;
        case LAYER_TYPE_VMAP:
            {
                for(auto itr = loadData.vmapData.begin(); itr != loadData.vmapData.end(); itr++)
                {
                    int offset = tVertArray.size()/3;
                    tVertArray.append((*itr)->objVerts);
                    TerrainBuilder::copyIndices((*itr)->objTris, tTriArray, offset);
                    tTriFlags.append((*itr)->objFlags);
                }
            }break;
        }
    }

    void getTileBoundingBoxInfoforTileConfig(MapTileLoadData &loadData, rcConfig &config, rcConfig &tileConfig, int layer, int layerType)
    {
        switch(layerType)
        {
        case LAYER_TYPE_TERRAIN:
            {
                switch(layer)
                {
                case 0:
                    {
                        tileConfig.bmin[1] = loadData.terrainData[0].boundBox.low()[1];
                        tileConfig.bmax[1] = loadData.terrainData[0].boundBox.high()[1];
                    }break;
                case 1:
                    {
                        tileConfig.bmin[1] = loadData.terrainData[0].boundBox.low()[1];
                        tileConfig.bmax[1] = loadData.terrainData[0].boundBox.high()[1];
                    }break;
                }break;
            }break;
        case LAYER_TYPE_VMAP:
            {
                if(loadData.vmapData.size())
                {   // This function only really uses our height from the bound boxes, need to eventually utilize our assumed bound box to only base height off of intersected bound boxes for a reduced heightfield
                    G3D::AABox stackedBoundBox(G3D::Vector3(tileConfig.bmin[0], tileConfig.bmin[1], tileConfig.bmin[2]), G3D::Vector3(tileConfig.bmax[0], tileConfig.bmax[1], tileConfig.bmax[2]));; // Grab all our vmap bound boxes and merge them into place, start with existing boundbox
                    G3D::AABox tileBounds(G3D::Vector3(tileConfig.bmin[0], VMAP_INVALID_HEIGHT, tileConfig.bmin[2]), G3D::Vector3(tileConfig.bmax[0], VMAP_MAX_HEIGHT, tileConfig.bmax[2]));
                    for(auto itr = loadData.vmapData.begin(); itr != loadData.vmapData.end(); itr++)
                    {
                        // Check if our vmap bound box intersects with our current tile
                        if(!(*itr)->boundBox.intersects(tileBounds))
                            continue;
                        stackedBoundBox.merge((*itr)->boundBox);
                    }
                    tileConfig.bmin[1] = stackedBoundBox.low()[1];
                    tileConfig.bmax[1] = stackedBoundBox.high()[1];
                }
            }break;
        }
    }

    void MapBuilder::buildMoveMapSubTileFromContext(Tile *tileToBuild, MapSubtileBuildContext *context)
    {
        Tile &tile = *tileToBuild;
        // Tile defaults to failed
        context->builtHeightField = context->builtCompactHeightField = context->builtContourSet = context->builtPolyMesh = context->builtDetailMesh = false;

        // build heightfield
        tile.solid = rcAllocHeightfield();
        if (!tile.solid || !rcCreateHeightfield(m_rcContext, *tile.solid, context->tileCfg.width, context->tileCfg.height, context->tileCfg.bmin, context->tileCfg.bmax, context->tileCfg.cs, context->tileCfg.ch))
        {
            printf("%s Failed building heightfield!            \n%s\r", context->tileString, dialogBox.c_str());
            return;
        }
        context->builtHeightField = true;

        // Grab our loaded verts and tris
        float *tVerts = context->tVertArray->getCArray();
        int tVertCount = context->tVertArray->size() / 3;
        int *tTris = context->tTriArray->getCArray();
        int tTriCount = context->tTriArray->size() / 3;

        // Rasterize our terrain triangles
        rcRasterizeTriangles(m_rcContext, tVerts, tVertCount, tTris, context->triFlags, tTriCount, *tile.solid, context->tileCfg.walkableClimb);

        // Filter our tile
        rcFilterLowHangingWalkableObstacles(m_rcContext, context->tileCfg.walkableClimb, *tile.solid);
        rcFilterLedgeSpans(m_rcContext, context->tileCfg.walkableHeight, context->tileCfg.walkableClimb, *tile.solid);
        rcFilterWalkableLowHeightSpans(m_rcContext, context->tileCfg.walkableHeight, *tile.solid);

        // compact heightfield spans
        tile.chf = rcAllocCompactHeightfield();
        if (!tile.chf || !rcBuildCompactHeightfield(m_rcContext, context->tileCfg.walkableHeight, context->tileCfg.walkableClimb, *tile.solid, *tile.chf))
        {
            printf("%s Failed compacting heightfield!            \n%s\r", context->tileString, dialogBox.c_str());
            return;
        }
        context->builtCompactHeightField = true;

        // build polymesh intermediates
        if (!rcErodeWalkableArea(m_rcContext, context->tileCfg.walkableRadius, *tile.chf))
        {
            printf("%s Failed eroding area!                    \n%s\r", context->tileString, dialogBox.c_str());
            return;
        }

        if (!rcMedianFilterWalkableArea(m_rcContext, *tile.chf))
        {
            printf("%s Failed filtering walkable area!         \n%s\r", context->tileString, dialogBox.c_str());
            return;
        }

        if (!rcBuildDistanceField(m_rcContext, *tile.chf))
        {
            printf("%s Failed building distance field!         \n%s\r", context->tileString, dialogBox.c_str());
            return;
        }

        if (!rcBuildRegions(m_rcContext, *tile.chf, context->tileCfg.borderSize, context->tileCfg.minRegionArea, context->tileCfg.mergeRegionArea))
        {
            printf("%s Failed building regions!                \n%s\r", context->tileString, dialogBox.c_str());
            return;
        }

        tile.cset = rcAllocContourSet();
        if (!tile.cset || !rcBuildContours(m_rcContext, *tile.chf, context->tileCfg.maxSimplificationError, context->tileCfg.maxEdgeLen, *tile.cset))
        {
            printf("%s Failed building contours!               \n%s\r", context->tileString, dialogBox.c_str());
            return;
        }
        context->builtContourSet = true;

        // build polymesh
        tile.pmesh = rcAllocPolyMesh();
        if (!tile.pmesh || !rcBuildPolyMesh(m_rcContext, *tile.cset, context->tileCfg.maxVertsPerPoly, *tile.pmesh))
        {
            printf("%s Failed building polymesh!               \n%s\r", context->tileString, dialogBox.c_str());
            return;
        }
        context->builtPolyMesh = true;

        tile.dmesh = rcAllocPolyMeshDetail();
        if (!tile.dmesh || !rcBuildPolyMeshDetail(m_rcContext, *tile.pmesh, *tile.chf, context->tileCfg.detailSampleDist, context->tileCfg.detailSampleMaxError, *tile.dmesh))
        {
            printf("%s Failed building polymesh detail!        \n%s\r", context->tileString, dialogBox.c_str());
            return;
        }
        context->builtDetailMesh = true;
    }

    const static float BASE_UNIT_SIZE = 0.2666666f;

    const static float buildConfig_moveSize[MAX_LAYER_TYPE] = { BASE_UNIT_SIZE*4, BASE_UNIT_SIZE };
    const static float buildConfig_VERTEX_PER_MAP[MAX_LAYER_TYPE] = { (GRID_SIZE / buildConfig_moveSize[0] + 0.5f), (GRID_SIZE / buildConfig_moveSize[1] + 0.5f) };
    const static int buildConfig_VERTEX_PER_TILE[MAX_LAYER_TYPE] = { 32, 128 };
    const static int buildConfig_TILES_PER_MAP[MAX_LAYER_TYPE] = { int(buildConfig_VERTEX_PER_MAP[0]/buildConfig_VERTEX_PER_TILE[0]), int(buildConfig_VERTEX_PER_MAP[1]/buildConfig_VERTEX_PER_TILE[1]) };
    const static int buildConfig_MinRegionArea[MAX_LAYER_TYPE] = { rcSqr(15), rcSqr(60) };
    const static int buildConfig_MergeRegionArea[MAX_LAYER_TYPE] = { rcSqr(12), rcSqr(50) };

    /**************************************************************************/
    void MapBuilder::buildMoveMapTerrainTile(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapTileLoadData &loadData, float bmin[3], float bmax[3], dtNavMesh* navMesh, ThreadTaskList &threadList)
    {   // 35,39 | -1821.15 -4208.10 3.15 | -1821.15 3.15 -4208.10
        // value have to divide GRID_SIZE(533.3333f) ( aka: 0.5333, 0.2666, 0.3333, 0.1333, etc )
        rcConfig config[MAX_LAYER_TYPE];
        memset(&config, 0, sizeof(rcConfig)*MAX_LAYER_TYPE);

        rcVcopy(config[LAYER_TYPE_TERRAIN].bmin, bmin);
        rcVcopy(config[LAYER_TYPE_TERRAIN].bmax, bmax);
        rcVcopy(config[LAYER_TYPE_VMAP].bmin, bmin);
        rcVcopy(config[LAYER_TYPE_VMAP].bmax, bmax);

        for(int laterType = LAYER_TYPE_TERRAIN; laterType < MAX_LAYER_TYPE; laterType++)
        {
            config[laterType].maxVertsPerPoly = DT_VERTS_PER_POLYGON;
            config[laterType].cs = buildConfig_moveSize[laterType];
            config[laterType].ch = buildConfig_moveSize[laterType];
            config[laterType].walkableSlopeAngle = m_maxWalkableAngle;
            config[laterType].walkableSlopeAngleNotSteep = 55;
            config[laterType].tileSize = buildConfig_VERTEX_PER_TILE[laterType];
            config[laterType].walkableRadius = m_bigBaseUnit ? 1 : 2;
            config[laterType].borderSize = config[laterType].walkableRadius + 3;
            config[laterType].maxEdgeLen = buildConfig_VERTEX_PER_TILE[laterType] + 1;   //anything bigger than tileSize
            config[laterType].walkableHeight = m_bigBaseUnit ? 3 : 6;
            config[laterType].walkableClimb = m_bigBaseUnit ? 3 : 6;
            config[laterType].minRegionArea = buildConfig_MinRegionArea[laterType];
            config[laterType].mergeRegionArea = buildConfig_MergeRegionArea[laterType];
            config[laterType].maxSimplificationError = 1.8f;             // eliminates most jagged edges (tiny polygons)
            config[laterType].detailSampleDist = config[laterType].cs * 16;
            config[laterType].detailSampleMaxError = config[laterType].ch * 1;

            switch (mapID)
            {
            // Blade's Edge Arena
            case 562: // This allows to walk on the ropes to the pillars
                config[laterType].walkableRadius = 0;
                break;
            // Blackfathom Deeps
            case 48: // Reduce the chance to have underground levels
                config[laterType].ch *= 2;
                break;
            }

            // this sets the dimensions of the heightfield - should maybe happen before border padding
            rcCalcGridSize(config[laterType].bmin, config[laterType].bmax, config[laterType].cs, &config[laterType].width, &config[laterType].height);
        }

        const int MAX_LAYER_PER_NAVMESH = 3;
        const int LayerToType[MAX_LAYER_PER_NAVMESH] = {LAYER_TYPE_TERRAIN, LAYER_TYPE_TERRAIN, LAYER_TYPE_VMAP}; // [Land][Water][VMAP]

        // console output
        char tileString[20];
        IntermediateValues iv[MAX_LAYER_PER_NAVMESH];
        for(int layer = 0; layer < MAX_LAYER_PER_NAVMESH; layer++)
        {
            sprintf(tileString, "[%02i,%02i|%i]: ", tileX, tileY, layer);

            int nmerge = 0;
            G3D::Array<float> tVertArray;
            G3D::Array<int> tTriArray;
            G3D::Array<int> tTriFlags;
            G3D::Array<G3D::AABox*> vmapData;
            buildTrisVerticesFlagsFromLoadData(loadData, layer, LayerToType[layer], tVertArray, tTriArray, tTriFlags, vmapData);
            if(!tVertArray.size())
                continue; // We have no verts available
            if(LayerToType[layer] != LAYER_TYPE_VMAP)
            {    // Cull terrain that interacts with vmap objects
                if(!filterObjectsFromTriangles(tVertArray, tTriArray, tTriFlags, vmapData))
                    continue; // Vmapobj has culled all our terrain, end this layer here
            }

            // allocate subregions : tiles
            Tile* tiles = new Tile[buildConfig_TILES_PER_MAP[LayerToType[layer]] * buildConfig_TILES_PER_MAP[LayerToType[layer]]];

            // merge per tile poly and detail meshes
            rcPolyMesh** pmmerge = new rcPolyMesh*[buildConfig_TILES_PER_MAP[LayerToType[layer]] * buildConfig_TILES_PER_MAP[LayerToType[layer]]];
            if (!pmmerge)
            {
                printf("%s alloc pmmerge FAILED!             \n%s\r", tileString, dialogBox.c_str());
                return;
            }

            rcPolyMeshDetail** dmmerge = new rcPolyMeshDetail*[buildConfig_TILES_PER_MAP[LayerToType[layer]] * buildConfig_TILES_PER_MAP[LayerToType[layer]]];
            if (!dmmerge)
            {
                printf("%s alloc dmmerge FAILED!             \n%s\r", tileString, dialogBox.c_str());
                return;
            }

            // Grab our flags
            int *cTriFlags = tTriFlags.getCArray();
            int cTriFlagCount = tTriFlags.size();

            // Initialize flags for our terrain and copy in our existing terrain flags
            unsigned char* triFlags = new unsigned char[cTriFlagCount];
            memcpy_s(triFlags, cTriFlagCount, cTriFlags, cTriFlagCount);
            // Process our tris and vertices to fill our tri flags with appropriate data
            rcClearUnwalkableTriangles(m_rcContext, config[LayerToType[layer]].walkableSlopeAngle, tVertArray.getCArray(), tVertArray.size() / 3, tTriArray.getCArray(), tTriArray.size() / 3, triFlags, NAV_COLLISION);
            rcMarkWalkableTriangles(m_rcContext, config[LayerToType[layer]].walkableSlopeAngleNotSteep, tVertArray.getCArray(), tVertArray.size() / 3, tTriArray.getCArray(), tTriArray.size() / 3, triFlags, NAV_GROUND);

            printf("%s Building subtile context...           \r", tileString);
            std::map<uint32, MapSubtileBuildContext*> m_tileContextMap;
            // build all tiles
            for (int x = 0; x < buildConfig_TILES_PER_MAP[LayerToType[layer]]; ++x)
            {
                for (int y = 0; y < buildConfig_TILES_PER_MAP[LayerToType[layer]]; ++y)
                {
                    Tile& tile = tiles[x + y * buildConfig_TILES_PER_MAP[LayerToType[layer]]];
                    MapSubtileBuildContext *context = new MapSubtileBuildContext();
                    m_tileContextMap.insert(std::make_pair(packTileID(x, y), context));

                    context->tileCfg = config[LayerToType[layer]];
                    sprintf(context->tileString, "[%02i,%02i|%i]: ", tileX, tileY, layer);

                    // Padd our tile with a buffer
                    context->tileCfg.bmin[0] = config[LayerToType[layer]].bmin[0] + x * float(context->tileCfg.tileSize * context->tileCfg.cs);
                    context->tileCfg.bmin[2] = config[LayerToType[layer]].bmin[2] + y * float(context->tileCfg.tileSize * context->tileCfg.cs);
                    context->tileCfg.bmax[0] = config[LayerToType[layer]].bmin[0] + (x + 1) * float(context->tileCfg.tileSize * context->tileCfg.cs);
                    context->tileCfg.bmax[2] = config[LayerToType[layer]].bmin[2] + (y + 1) * float(context->tileCfg.tileSize * context->tileCfg.cs);

                    context->tileCfg.bmin[0] -= context->tileCfg.borderSize * context->tileCfg.cs;
                    context->tileCfg.bmin[2] -= context->tileCfg.borderSize * context->tileCfg.cs;
                    context->tileCfg.bmax[0] += context->tileCfg.borderSize * context->tileCfg.cs;
                    context->tileCfg.bmax[2] += context->tileCfg.borderSize * context->tileCfg.cs;

                    // Initialize per tile config.
                    getTileBoundingBoxInfoforTileConfig(loadData, config[LayerToType[layer]], context->tileCfg, layer, LayerToType[layer]);

                    context->tileCfg.height /= buildConfig_TILES_PER_MAP[LayerToType[layer]];
                    context->tileCfg.width /= buildConfig_TILES_PER_MAP[LayerToType[layer]];

                    // Set our context with our verts, tris, and flags
                    context->tVertArray = &tVertArray;
                    context->tTriArray = &tTriArray;
                    context->triFlags = triFlags;
                    context->cTriFlagCount = cTriFlagCount;

                    // Queue into threadlist
                    threadList.AddTask(new CallbackP2 <MapBuilder, Tile*, MapSubtileBuildContext*>(this, &MapBuilder::buildMoveMapSubTileFromContext, &tile, context));
                }
            }

            threadList.wait(UNIXTIME, g_localTime);
            printf("%s Merging polymesh...           \r", tileString);

            // Clean up our flags
            delete triFlags;

            for (int x = 0; x < buildConfig_TILES_PER_MAP[LayerToType[layer]]; ++x)
            {
                for (int y = 0; y < buildConfig_TILES_PER_MAP[LayerToType[layer]]; ++y)
                {
                    Tile& tile = tiles[x + y * buildConfig_TILES_PER_MAP[LayerToType[layer]]];
                    // Tile context is always created, safe to use at instead of search
                    MapSubtileBuildContext *tileContext = m_tileContextMap.at(packTileID(x, y));
                    m_tileContextMap.erase(packTileID(x, y));

                    /// Free up a lot of data and check our allocation steps
                    // we may want to keep them in the future for debug
                    if(!tileContext->builtHeightField)
                    {
                        delete tileContext;
                        continue;
                    }

                    rcFreeHeightField(tile.solid);
                    tile.solid = NULL;
                    if(!tileContext->builtCompactHeightField)
                    {
                        delete tileContext;
                        continue;
                    }

                    rcFreeCompactHeightfield(tile.chf);
                    tile.chf = NULL;
                    if(!tileContext->builtContourSet)
                    {
                        delete tileContext;
                        continue;
                    }

                    rcFreeContourSet(tile.cset);
                    tile.cset = NULL;
                    // tile dealloc will clean these up?
                    if(!tileContext->builtPolyMesh || !tileContext->builtDetailMesh)
                    {
                        if(tile.pmesh)
                            delete tile.pmesh;
                        if(tile.dmesh)
                            delete tile.dmesh;
                        delete tileContext;
                        continue;
                    }

                    delete tileContext;
                    pmmerge[nmerge] = tile.pmesh;
                    dmmerge[nmerge] = tile.dmesh;
                    nmerge++;
                }
            }

            iv[layer].polyMesh = rcAllocPolyMesh();
            if (!iv[layer].polyMesh)
            {
                printf("%s alloc iv.polyMesh FIALED!             \n%s\r", tileString, dialogBox.c_str());
                delete[] pmmerge;
                delete[] dmmerge;
                delete[] tiles;
                return;
            }
            rcMergePolyMeshes(m_rcContext, pmmerge, nmerge, *iv[layer].polyMesh);

            iv[layer].polyMeshDetail = rcAllocPolyMeshDetail();
            if (!iv[layer].polyMeshDetail)
            {
                printf("%s alloc m_dmesh FIALED!             \n%s\r", tileString, dialogBox.c_str());
                delete[] pmmerge;
                delete[] dmmerge;
                delete[] tiles;
                return;
            }
            rcMergePolyMeshDetails(m_rcContext, dmmerge, nmerge, *iv[layer].polyMeshDetail);

            // free things up
            delete[] pmmerge;
            delete[] dmmerge;
            delete[] tiles;
        }

        // Push our intermediate values into our navmesh layers
        dtNavMeshCreateParams params[MAX_LAYER_PER_NAVMESH];
        for(int layer = 0; layer < MAX_LAYER_PER_NAVMESH; layer++)
        {
            memset(&params[layer], 0, sizeof(params));
            if(!iv[layer].polyMesh)
                continue;

            // set polygons as walkable
            // TODO: special flags for DYNAMIC polygons, ie surfaces that can be turned on and off
            for (int i = 0; i < iv[layer].polyMesh->npolys; ++i)
                if (uint8 area = iv[layer].polyMesh->areas[i] & NAV_ALL_MASK)
                    iv[layer].polyMesh->flags[i] = (area >= NAV_MIN_VALUE) ? (1 << (NAV_MAX_VALUE - iv[layer].polyMesh->areas[i])) : NAV_GROUND;

            // setup mesh parameters
            params[layer].verts = iv[layer].polyMesh->verts;
            params[layer].vertCount = iv[layer].polyMesh->nverts;
            params[layer].polys = iv[layer].polyMesh->polys;
            params[layer].polyAreas = iv[layer].polyMesh->areas;
            params[layer].polyFlags = iv[layer].polyMesh->flags;
            params[layer].polyCount = iv[layer].polyMesh->npolys;
            params[layer].nvp = iv[layer].polyMesh->nvp;
            params[layer].detailMeshes = iv[layer].polyMeshDetail->meshes;
            params[layer].detailVerts = iv[layer].polyMeshDetail->verts;
            params[layer].detailVertsCount = iv[layer].polyMeshDetail->nverts;
            params[layer].detailTris = iv[layer].polyMeshDetail->tris;
            params[layer].detailTriCount = iv[layer].polyMeshDetail->ntris;

            params[layer].offMeshConVerts = loadData.offMeshConnections.getCArray();
            params[layer].offMeshConCount = loadData.offMeshConnections.size()/6;
            params[layer].offMeshConRad = loadData.offMeshConnectionRads.getCArray();
            params[layer].offMeshConDir = loadData.offMeshConnectionDirs.getCArray();
            params[layer].offMeshConAreas = loadData.offMeshConnectionsAreas.getCArray();
            params[layer].offMeshConFlags = loadData.offMeshConnectionsFlags.getCArray();

            params[layer].walkableHeight = BASE_UNIT_SIZE*config[layer].walkableHeight;    // agent height
            params[layer].walkableRadius = BASE_UNIT_SIZE*config[layer].walkableRadius;    // agent radius
            params[layer].walkableClimb = BASE_UNIT_SIZE*config[layer].walkableClimb;      // keep less that walkableHeight (aka agent height)!
            params[layer].tileX = (((bmin[0] + bmax[0]) / 2) - navMesh->getParams()->orig[0]) / GRID_SIZE;
            params[layer].tileY = (((bmin[2] + bmax[2]) / 2) - navMesh->getParams()->orig[2]) / GRID_SIZE;
            rcVcopy(params[layer].bmin, bmin);
            rcVcopy(params[layer].bmax, bmax);
            params[layer].cs = config[layer].cs;
            params[layer].ch = config[layer].ch;
            params[layer].tileLayer = layer;
            params[layer].buildBvTree = true;
        }

        // will hold final navmesh
        unsigned char* navData[MAX_LAYER_PER_NAVMESH] = {NULL, NULL, NULL};
        int navDataSize[MAX_LAYER_PER_NAVMESH] = { 0, 0, 0 };
        int navlayerCount = 0;

        do
        {
            for(int layer = 0; layer < MAX_LAYER_PER_NAVMESH; layer++)
            {
                sprintf(tileString, "[%02i,%02i|%i]: ", tileX, tileY, layer);
                printf("%s Building navmesh tile...           \r", tileString);

                // these values are checked within dtCreateNavMeshData - handle them here
                // so we have a clear error message
                if (!params[layer].vertCount || !params[layer].verts)
                {
                    // message is an annoyance
                    if(layer == 0) // Only output error on layer 0
                        printf("%sNo vertices to build tile!              \n%s\r", tileString, dialogBox.c_str());
                    continue;
                }
                if (params[layer].nvp > DT_VERTS_PER_POLYGON)
                {
                    printf("%s Invalid verts-per-polygon value!        \n%s\r", tileString, dialogBox.c_str());
                    continue;
                }
                if (params[layer].vertCount >= 0xffff)
                {
                    printf("%s Too many vertices!                      \n%s\r", tileString, dialogBox.c_str());
                    continue;
                }
                if (!params[layer].polyCount || !params[layer].polys)
                {
                    // we have flat tiles with no actual geometry - don't build those, its useless
                    // keep in mind that we do output those into debug info
                    // drop tiles with only exact count - some tiles may have geometry while having less tiles
                    printf("%s No polygons to build on tile!              \n%s\r", tileString, dialogBox.c_str());
                    continue;
                }
                if (!params[layer].detailMeshes || !params[layer].detailVerts || !params[layer].detailTris)
                {
                    printf("%s No detail mesh to build tile!           \n%s\r", tileString, dialogBox.c_str());
                    continue;
                }

                if (!dtCreateNavMeshData(&params[layer], &navData[layer], &navDataSize[layer]))
                {
                    printf("%s Failed building navmesh tile!           \n%s\r", tileString, dialogBox.c_str());
                    continue;
                }
                navlayerCount++;
            }

            if(navlayerCount == 0)
            {
                sprintf(tileString, "[%02i,%02i|ALL]: ", tileX, tileY);
                printf("%s Failed building navmesh tile!           \n%s\r", tileString, dialogBox.c_str());
                continue;
            }

            bool result = false;
            dtTileRef tileRef[MAX_LAYER_PER_NAVMESH] = { 0, 0, 0 };
            for(int layer = 0; layer < MAX_LAYER_PER_NAVMESH; layer++)
            {
                if(navData[layer] == NULL)
                    continue;

                //printf("%s Adding tile to navmesh...           \r", tileString);
                // DT_TILE_FREE_DATA tells detour to unallocate memory when the tile
                // is removed via removeTile()
                dtStatus dtResult = navMesh->addTile(navData[layer], navDataSize[layer], DT_TILE_FREE_DATA, 0, &tileRef[layer]);
                if (!tileRef[layer] || dtResult != DT_SUCCESS)
                    continue;
                result = true;
            }

            if(result == false)
            {
                sprintf(tileString, "[%02i,%02i|ALL]: ", tileX, tileY);
                printf("%s Failed adding tile to navmesh!           \n%s\r", tileString, dialogBox.c_str());
                continue;
            }

            fileLock.Acquire();
            // file output
            char fileName[255];
            sprintf(fileName, "mmaps/%03u.mmtiles", mapID);
            FILE* file = fopen(fileName, "r+b");
            if (!file && !(file = fopen(fileName, "w+b")))
            {
                char message[1024];
                sprintf(message, "Failed to open %s for writing!             \n%s\r", fileName, dialogBox.c_str());
                perror(message);
                for(int layer = 0; layer < MAX_LAYER_PER_NAVMESH; layer++)
                    navMesh->removeTile(tileRef[layer], NULL, NULL);
                fileLock.Release();
                continue;
            }

            uint32 offsets[64][64];
            memset(offsets, 0, sizeof(uint32)*64*64);
            // Go to end of file.
            fseek(file, 0, SEEK_END);
            uint32 fileSize = 0;
            if((fileSize = ftell(file)) == 0)
            {
                fwrite(offsets, sizeof(uint32)*64*64, 1, file);
                fseek(file, 0, SEEK_END);
            }
            fseek(file, 0, SEEK_SET);
            fread(offsets, sizeof(uint32)*64*64, 1, file);
            ASSERT(offsets[tileX][tileY] == 0);
            offsets[tileX][tileY] = fileSize;
            fseek(file, 0, SEEK_SET);
            fwrite(offsets, sizeof(uint32)*64*64, 1, file);
            fseek(file, 0, SEEK_END);

            //printf("%s Writing to file...             \r", tileString);

            // write header for layer 0
            MmapTileHeader header;
            for(uint8 i = 0; i < MAX_LAYER_PER_NAVMESH; i++)
            {
                header.size = G3D::uint32(navDataSize[i]);
                header.hasNextLayer = G3D::uint32(i == 2 ? 0 : 1);
                fwrite(&header, sizeof(MmapTileHeader), 1, file);
                // write data
                if(header.size) fwrite(navData[i], sizeof(unsigned char), navDataSize[i], file);
            }
            fclose(file);

            // now that tile is written to disk, we can unload it
            for(int layer = 0; layer < MAX_LAYER_PER_NAVMESH; layer++)
                navMesh->removeTile(tileRef[layer], NULL, NULL);
            fileLock.Release();
        }while (0);
    }

    /**************************************************************************/
    void MapBuilder::getTileBounds(G3D::uint32 tileX, G3D::uint32 tileY, float* verts, int vertCount, float* bmin, float* bmax)
    {
        // this is for elevation
        if (verts && vertCount)
            rcCalcBounds(verts, vertCount, bmin, bmax);
        else
        {
            bmin[1] = VMAP_INVALID_HEIGHT;
            bmax[1] = VMAP_MAX_HEIGHT;
        }

        // this is for width and depth
        bmax[0] = (32 - int(tileX)) * GRID_SIZE;
        bmax[2] = (32 - int(tileY)) * GRID_SIZE;
        bmin[0] = bmax[0] - GRID_SIZE;
        bmin[2] = bmax[2] - GRID_SIZE;
    }

    /**************************************************************************/
    void MapBuilder::getTileBounds(G3D::uint32 tileX, G3D::uint32 tileY, MapTileLoadData &loadData, float* bmin, float* bmax)
    {
        // this is for width and depth
        bmin[0] = bmin[2] = bmax[0] = bmax[2] = 0.f;
        bmin[1] = VMAP_MAX_HEIGHT;
        bmax[1] = VMAP_INVALID_HEIGHT;

        size_t length = 0;
        bool hasVert = false;
        // Check vmap verts for bounding box height
        if(hasVert = (length = loadData.allVerts.length()))
            rcCalcBounds(loadData.allVerts.getCArray(), length/3, bmin, bmax);

        if(!hasVert)
        {
            bmin[1] = VMAP_INVALID_HEIGHT;
            bmax[1] = VMAP_MAX_HEIGHT;
        }

        // this is for width and depth
        bmax[0] = (32 - int(tileX)) * GRID_SIZE;
        bmax[2] = (32 - int(tileY)) * GRID_SIZE;
        bmin[0] = bmax[0] - GRID_SIZE;
        bmin[2] = bmax[2] - GRID_SIZE;
    }

    /**************************************************************************/
    bool MapBuilder::shouldSkipMap(G3D::uint32 mapID)
    {
        if (m_skipContinents)
            switch (mapID)
        {
            case 0:
            case 1:
            case 530:
            case 571:
                return true;
            default:
                break;
        }

        if (m_skipJunkMaps)
            switch (mapID)
        {
            case 13:    // test.wdt
            case 25:    // ScottTest.wdt
            case 29:    // Test.wdt
            case 42:    // Colin.wdt
            case 169:   // EmeraldDream.wdt (unused, and very large)
            case 451:   // development.wdt
            case 573:   // ExteriorTest.wdt
            case 597:   // CraigTest.wdt
            case 605:   // development_nonweighted.wdt
            case 606:   // QA_DVD.wdt
            case 651:   // ElevatorSpawnTest.wdt
                return true;
            default:
                if (isTransportMap(mapID))
                    return true;
                break;
        }

        if (m_skipBattlegrounds)
            switch (mapID)
        {
            case 30:    // AV
            case 37:    // ?
            case 489:   // WSG
            case 529:   // AB
            case 566:   // EotS
            case 607:   // SotA
            case 628:   // IoC
            case 726:   // Twin Peaks
            case 727:   // Silvershard Mines
            case 761:   // The Battle for Gilneas
            case 968:   // Rated Eye of the Storm
                return true;
            default:
                break;
        }

        return false;
    }

    /**************************************************************************/
    bool MapBuilder::isTransportMap(G3D::uint32 mapID)
    {
        switch (mapID)
        {
            // transport maps
        case 582:
        case 584:
        case 586:
        case 587:
        case 588:
        case 589:
        case 590:
        case 591:
        case 592:
        case 593:
        case 594:
        case 596:
        case 610:
        case 612:
        case 613:
        case 614:
        case 620:
        case 621:
        case 622:
        case 623:
        case 641:
        case 642:
        case 647:
        case 662:
        case 672:
        case 673:
        case 674:
        case 712:
        case 713:
        case 718:
        case 738:
        case 739:
        case 740:
        case 741:
        case 742:
        case 743:
        case 747:
        case 748:
        case 749:
        case 750:
        case 762:
        case 763:
        case 765:
        case 766:
        case 767:
            return true;
        default:
            return false;
        }
    }

    /**************************************************************************/
    bool MapBuilder::shouldSkipTile(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY)
    {
        char fileName[255];
        sprintf(fileName, "mmaps/%03u%02i%02i.mmtile", mapID, tileY, tileX);
        FILE* file = fopen(fileName, "rb");
        if (!file)
            return false;

        MmapTileHeader header;
        int count = fread(&header, sizeof(MmapTileHeader), 1, file);
        fclose(file);
        if (count != 1)
            return false;

        if (header.mmapMagic != MMAP_MAGIC || header.dtVersion != G3D::uint32(DT_NAVMESH_VERSION))
            return false;

        if (header.mmapVersion != MMAP_VERSION)
            return false;

        return true;
    }

}
