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
#define MMAP_VERSION 4

time_t UNIXTIME;
tm g_localTime;

const char *heightMapHeader = "HMAP434_2";

struct MmapTileHeader
{
    G3D::uint32 mmapMagic;
    G3D::uint32 dtVersion;
    G3D::uint32 mmapVersion;
    G3D::uint32 size;
    bool usesLiquids : 1;

    MmapTileHeader() : mmapMagic(MMAP_MAGIC), dtVersion(DT_NAVMESH_VERSION),
        mmapVersion(MMAP_VERSION), size(0), usesLiquids(true) {}
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
    Mutex activeMapIdLock;
    std::map<uint32, std::string> activeMapIDs;
    void MapBuilder::buildAllMaps(int threads)
    {
        if(false)//threads)
        {
            ThreadTaskList taskList(threads);
            taskList.spawn("MapBuilder", "Initializing", threads > 0);

            for (TileList::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
            {
                G3D::uint32 mapID = it->first;
                if (shouldSkipMap(mapID))
                    continue;

                taskList.AddTask(new CallbackP1 <MapBuilder, G3D::uint32>(this, &MapBuilder::buildMap, mapID));
            }

            taskList.wait(UNIXTIME, g_localTime);
            printf("Job Complete!                                    \n");
            taskList.kill();
            taskList.waitForThreadsToExit();
        }
        else
        {
            for (TileList::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
            {
                G3D::uint32 mapID = it->first;
                if (!shouldSkipMap(mapID))
                {
                    buildMap(mapID);
                }
            }
        }
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
        float lmin[3] = { 0, 0, 0 };
        float lmax[3] = { 0, 0, 0 };
        MeshData meshData;

        // make sure we process maps which don't have tiles
        // initialize the static tree, which loads WDT models
        if(!m_terrainBuilder->loadVMap(mapID, 64, 64, meshData))
            return;

        // get the coord bounds of the model data
        if (meshData.solidVerts.size() + meshData.liquidVerts.size() == 0)
            return;

        // get the coord bounds of the model data
        if (meshData.solidVerts.size() && meshData.liquidVerts.size())
        {
            rcCalcBounds(meshData.solidVerts.getCArray(), meshData.solidVerts.size() / 3, bmin, bmax);
            rcCalcBounds(meshData.liquidVerts.getCArray(), meshData.liquidVerts.size() / 3, lmin, lmax);
            rcVmin(bmin, lmin);
            rcVmax(bmax, lmax);
        }
        else if (meshData.solidVerts.size())
            rcCalcBounds(meshData.solidVerts.getCArray(), meshData.solidVerts.size() / 3, bmin, bmax);
        else
            rcCalcBounds(meshData.liquidVerts.getCArray(), meshData.liquidVerts.size() / 3, lmin, lmax);

        // convert coord bounds to grid bounds
        maxX = 32 - bmin[0] / GRID_SIZE;
        maxY = 32 - bmin[2] / GRID_SIZE;
        minX = 32 - bmax[0] / GRID_SIZE;
        minY = 32 - bmax[2] / GRID_SIZE;
    }

    void MapBuilder::buildMeshFromFile(char* name)
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

        MeshData data;

        for (G3D::uint32 i = 0; i < verticesCount; ++i)
            data.solidVerts.append(verts[i]);
        delete[] verts;

        for (G3D::uint32 i = 0; i < indicesCount; ++i)
            data.solidTris.append(inds[i]);
        delete[] inds;

        TerrainBuilder::cleanVertices(data.solidVerts, data.solidTris);
        // get bounds of current tile
        float bmin[3], bmax[3];
        getTileBounds(tileX, tileY, data.solidVerts.getCArray(), data.solidVerts.size() / 3, bmin, bmax);

        // build navmesh tile
        buildMoveMapTile(mapId, tileX, tileY, data, bmin, bmax, navMesh);
        fclose(file);
    }

    /**************************************************************************/
    void MapBuilder::buildSingleTile(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY)
    {
        dtNavMesh* navMesh = NULL;
        buildNavMesh(mapID, navMesh);
        if (!navMesh)
        {
            printf("Failed creating navmesh!              \n");
            return;
        }

        m_terrainBuilder->InitializeMap(mapID);
        buildTile(mapID, tileX, tileY, navMesh);
        m_terrainBuilder->UnloadVMap(mapID);
        dtFreeNavMesh(navMesh);
    }

    /**************************************************************************/
    void MapBuilder::buildMap(G3D::uint32 mapID)
    {
        activeMapIdLock.Acquire();
        activeMapIDs.insert(std::make_pair(mapID, ThreadTaskList::format_name("[%u][64,64]").c_str()));
        printf("Building map %03u:                                          \n\n", mapID);
        std::string mapIds = "ActiveMapIds:";
        for(std::map<uint32, std::string>::iterator itr = activeMapIDs.begin(); itr != activeMapIDs.end(); itr++)
        {
            mapIds += " ";
            mapIds += itr->second.c_str();
        }

        printf("%s                        \r", mapIds.c_str());
        activeMapIdLock.Release();

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

                    buildTile(mapID, tileX, tileY, navMesh);
                }

                dtFreeNavMesh(navMesh);
            }
        }

        m_terrainBuilder->UnloadVMap(mapID);

        activeMapIdLock.Acquire();
        activeMapIDs.erase(mapID);
        mapIds = "ActiveMapIds:";
        for(std::map<uint32, std::string>::iterator itr = activeMapIDs.begin(); itr != activeMapIDs.end(); itr++)
        {
            mapIds += " ";
            mapIds += itr->second.c_str();
        }

        printf("%s                        \r", mapIds.c_str());

        activeMapIdLock.Release();
    }

    /**************************************************************************/
    void MapBuilder::buildTile(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, dtNavMesh* navMesh)
    {
        activeMapIdLock.Acquire();
        std::string mapIds = "ActiveMapIds:";
        mapIds = "ActiveMapIds:";
        activeMapIDs[mapID] = ThreadTaskList::format_name("[%u][%02u,%02u]", mapID, tileX, tileY).c_str();
        for(std::map<uint32, std::string>::iterator itr = activeMapIDs.begin(); itr != activeMapIDs.end(); itr++)
        {
            mapIds += " ";
            mapIds += itr->second.c_str();
        }

        printf("%s                        \r", mapIds.c_str());
        activeMapIdLock.Release();

        //MeshData meshData;
        MapLoadData *tileMap = new MapLoadData;
        tileMap->loadInfoCount = 0;
        for(int x = 0; x < 18; x++)
        {
            for(int y = 0; y < 18; y++)
            {
                tileMap->_chunkData[x][y].areaInfo = tileMap->_chunkData[x][y].liquidType = 0;
                tileMap->_chunkData[x][y].xOffset = tileMap->_chunkData[x][y].yOffset = 0xFF;
                tileMap->_chunkData[x][y].isHole = false;
            }
        }

        bool res = true;

        // get heightmap data
        res = m_terrainBuilder->loadMap(mapID, tileX, tileY, *tileMap);

        // get model data
        if(m_terrainBuilder->loadVMap(mapID, tileY, tileX, *tileMap) == false && res == false)
            return;

        // get bounds of current tile
        float bmin[3], bmax[3];
        getTileBounds(tileX, tileY, *tileMap, bmin, bmax);

        // build navmesh tile
        buildMoveMapTile(mapID, tileX, tileY, *tileMap, bmin, bmax, navMesh);
        delete tileMap;
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

    /**************************************************************************/
    void MapBuilder::buildMoveMapTile(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY,
        MeshData &meshData, float bmin[3], float bmax[3],
        dtNavMesh* navMesh)
    {   // 35,39 | -1821.15 -4208.10 3.15 | -4208.10 3.15 -1821.15
        // console output
        char tileString[20];
        sprintf(tileString, "[%02i,%02i]: ", tileX, tileY);

        IntermediateValues iv;

        float* tVerts = meshData.solidVerts.getCArray();
        int tVertCount = meshData.solidVerts.size() / 3;
        int* tTris = meshData.solidTris.getCArray();
        int tTriCount = meshData.solidTris.size() / 3;

        float* lVerts = meshData.liquidVerts.getCArray();
        int lVertCount = meshData.liquidVerts.size() / 3;
        int* lTris = meshData.liquidTris.getCArray();
        int lTriCount = meshData.liquidTris.size() / 3;
        G3D::uint8* lTriFlags = meshData.liquidType.getCArray();

        // these are WORLD UNIT based metrics
        // this are basic unit dimentions
        // value have to divide GRID_SIZE(533.33333f) ( aka: 0.5333, 0.2666, 0.3333, 0.1333, etc )
        const static float BASE_UNIT_DIM = m_bigBaseUnit ? 0.533333f : 0.266666f;

        // All are in UNIT metrics!
        const static int VERTEX_PER_MAP = int(GRID_SIZE/BASE_UNIT_DIM + 0.5f);
        const static int VERTEX_PER_TILE = m_bigBaseUnit ? 40 : 80; // must divide VERTEX_PER_MAP
        const static int TILES_PER_MAP = VERTEX_PER_MAP/VERTEX_PER_TILE;

        rcConfig config;
        memset(&config, 0, sizeof(rcConfig));

        rcVcopy(config.bmin, bmin);
        rcVcopy(config.bmax, bmax);

        config.maxVertsPerPoly = DT_VERTS_PER_POLYGON;
        config.cs = BASE_UNIT_DIM;
        config.ch = BASE_UNIT_DIM;
        config.walkableSlopeAngle = m_maxWalkableAngle;
        config.walkableSlopeAngleNotNavigatable = 55;
        config.tileSize = VERTEX_PER_TILE;
        config.walkableRadius = m_bigBaseUnit ? 1 : 2;
        config.borderSize = config.walkableRadius + 3;
        config.maxEdgeLen = VERTEX_PER_TILE + 1;        //anything bigger than tileSize
        config.walkableHeight = m_bigBaseUnit ? 3 : 6;
        config.walkableClimb = m_bigBaseUnit ? 3 : 6;
        config.minRegionArea = rcSqr(60);
        config.mergeRegionArea = rcSqr(50);
        config.maxSimplificationError = 1.8f;       // eliminates most jagged edges (tinny polygons)
        config.detailSampleDist = config.cs * 16;
        config.detailSampleMaxError = config.ch * 1;

        switch (mapID)
        {
        // Blade's Edge Arena
        case 562: // This allows to walk on the ropes to the pillars
            config.walkableRadius = 0;
            break;
        // Blackfathom Deeps
        case 48: // Reduce the chance to have underground levels
            config.ch *= 2;
            break;
        }

        // this sets the dimensions of the heightfield - should maybe happen before border padding
        rcCalcGridSize(config.bmin, config.bmax, config.cs, &config.width, &config.height);

        // allocate subregions : tiles
        Tile* tiles = new Tile[TILES_PER_MAP * TILES_PER_MAP];

        // Initialize per tile config.
        rcConfig tileCfg = config;
        tileCfg.width = config.tileSize + config.borderSize*2;
        tileCfg.height = config.tileSize + config.borderSize*2;

        // merge per tile poly and detail meshes
        rcPolyMesh** pmmerge = new rcPolyMesh*[TILES_PER_MAP * TILES_PER_MAP];
        if (!pmmerge)
        {
            printf("%s alloc pmmerge FAILED!             \n", tileString);
            return;
        }

        rcPolyMeshDetail** dmmerge = new rcPolyMeshDetail*[TILES_PER_MAP * TILES_PER_MAP];
        if (!dmmerge)
        {
            printf("%s alloc dmmerge FAILED!             \n", tileString);
            return;
        }

        int nmerge = 0;
        // build all tiles
        for (int y = 0; y < TILES_PER_MAP; ++y)
        {
            for (int x = 0; x < TILES_PER_MAP; ++x)
            {
                Tile& tile = tiles[x + y * TILES_PER_MAP];

                // Calculate the per tile bounding box.
                tileCfg.bmin[0] = config.bmin[0] + x * float(config.tileSize * config.cs);
                tileCfg.bmin[2] = config.bmin[2] + y * float(config.tileSize * config.cs);
                tileCfg.bmax[0] = config.bmin[0] + (x + 1) * float(config.tileSize * config.cs);
                tileCfg.bmax[2] = config.bmin[2] + (y + 1) * float(config.tileSize * config.cs);

                tileCfg.bmin[0] -= tileCfg.borderSize * tileCfg.cs;
                tileCfg.bmin[2] -= tileCfg.borderSize * tileCfg.cs;
                tileCfg.bmax[0] += tileCfg.borderSize * tileCfg.cs;
                tileCfg.bmax[2] += tileCfg.borderSize * tileCfg.cs;

                // build heightfield
                tile.solid = rcAllocHeightfield();
                if (!tile.solid || !rcCreateHeightfield(m_rcContext, *tile.solid, tileCfg.width, tileCfg.height, tileCfg.bmin, tileCfg.bmax, tileCfg.cs, tileCfg.ch))
                {
                    printf("%s Failed building heightfield!            \n", tileString);
                    continue;
                }

                // mark all walkable tiles, both liquids and solids

                /* we want to have triangles with slope less than walkableSlopeAngleNotNavigatable (<= 90) to have NAV_AREA_GROUND
                 * and with slope between walkableSlopeAngleNotNavigatable and walkableSlopeAngle (55 < .. <= 70) to have NAV_AREA_GROUND_STEEP.
                 * we achieve this using recast API: memset everything to NAV_AREA_GROUND_STEEP, call rcClearUnwalkableTriangles with 70 so
                 * any area above that will get RC_NULL_AREA (unwalkable), then call rcMarkWalkableTriangles with 55 to set NAV_AREA_GROUND
                 * on anything below 55 . Players and idle Creatures can use NAV_AREA_GROUND, while Creatures in combat can use NAV_AREA_GROUND_STEEP.
                 */
                unsigned char* triFlags = new unsigned char[tTriCount];
                memset(triFlags, NAV_TOO_STEEP, tTriCount*sizeof(unsigned char));
                rcClearUnwalkableTriangles(m_rcContext, tileCfg.walkableSlopeAngleNotNavigatable, tVerts, tVertCount, tTris, tTriCount, triFlags);
                rcMarkWalkableTriangles(m_rcContext, tileCfg.walkableSlopeAngle, tVerts, tVertCount, tTris, tTriCount, triFlags, NAV_GROUND);
                rcRasterizeTriangles(m_rcContext, tVerts, tVertCount, tTris, triFlags, tTriCount, *tile.solid, config.walkableClimb);
                delete[] triFlags;

                rcFilterLowHangingWalkableObstacles(m_rcContext, config.walkableClimb, *tile.solid);
                rcFilterLedgeSpans(m_rcContext, tileCfg.walkableHeight, tileCfg.walkableClimb, *tile.solid);
                rcFilterWalkableLowHeightSpans(m_rcContext, tileCfg.walkableHeight, *tile.solid);

                // add liquid triangles
                rcRasterizeTriangles(m_rcContext, lVerts, lVertCount, lTris, lTriFlags, lTriCount, *tile.solid, config.walkableClimb);

                // compact heightfield spans
                tile.chf = rcAllocCompactHeightfield();
                if (!tile.chf || !rcBuildCompactHeightfield(m_rcContext, tileCfg.walkableHeight, tileCfg.walkableClimb, *tile.solid, *tile.chf))
                {
                    printf("%s Failed compacting heightfield!            \n", tileString);
                    continue;
                }

                // build polymesh intermediates
                if (!rcErodeWalkableArea(m_rcContext, config.walkableRadius, *tile.chf))
                {
                    printf("%s Failed eroding area!                    \n", tileString);
                    continue;
                }

                if (!rcMedianFilterWalkableArea(m_rcContext, *tile.chf))
                {
                    printf("%s Failed filtering walkable area!         \n", tileString);
                    continue;
                }

                if (!rcBuildDistanceField(m_rcContext, *tile.chf))
                {
                    printf("%s Failed building distance field!         \n", tileString);
                    continue;
                }

                if (!rcBuildRegions(m_rcContext, *tile.chf, tileCfg.borderSize, tileCfg.minRegionArea, tileCfg.mergeRegionArea))
                {
                    printf("%s Failed building regions!                \n", tileString);
                    continue;
                }

                tile.cset = rcAllocContourSet();
                if (!tile.cset || !rcBuildContours(m_rcContext, *tile.chf, tileCfg.maxSimplificationError, tileCfg.maxEdgeLen, *tile.cset))
                {
                    printf("%s Failed building contours!               \n", tileString);
                    continue;
                }

                // build polymesh
                tile.pmesh = rcAllocPolyMesh();
                if (!tile.pmesh || !rcBuildPolyMesh(m_rcContext, *tile.cset, tileCfg.maxVertsPerPoly, *tile.pmesh))
                {
                    printf("%s Failed building polymesh!               \n", tileString);
                    continue;
                }

                tile.dmesh = rcAllocPolyMeshDetail();
                if (!tile.dmesh || !rcBuildPolyMeshDetail(m_rcContext, *tile.pmesh, *tile.chf, tileCfg.detailSampleDist, tileCfg.detailSampleMaxError, *tile.dmesh))
                {
                    printf("%s Failed building polymesh detail!        \n", tileString);
                    continue;
                }

                // free those up
                // we may want to keep them in the future for debug
                // but right now, we don't have the code to merge them
                rcFreeHeightField(tile.solid);
                tile.solid = NULL;
                rcFreeCompactHeightfield(tile.chf);
                tile.chf = NULL;
                rcFreeContourSet(tile.cset);
                tile.cset = NULL;

                pmmerge[nmerge] = tile.pmesh;
                dmmerge[nmerge] = tile.dmesh;
                nmerge++;
            }
        }

        iv.polyMesh = rcAllocPolyMesh();
        if (!iv.polyMesh)
        {
            printf("%s alloc iv.polyMesh FIALED!             \n", tileString);
            delete[] pmmerge;
            delete[] dmmerge;
            delete[] tiles;
            return;
        }
        rcMergePolyMeshes(m_rcContext, pmmerge, nmerge, *iv.polyMesh);

        iv.polyMeshDetail = rcAllocPolyMeshDetail();
        if (!iv.polyMeshDetail)
        {
            printf("%s alloc m_dmesh FIALED!             \n", tileString);
            delete[] pmmerge;
            delete[] dmmerge;
            delete[] tiles;
            return;
        }
        rcMergePolyMeshDetails(m_rcContext, dmmerge, nmerge, *iv.polyMeshDetail);

        // free things up
        delete[] pmmerge;
        delete[] dmmerge;
        delete[] tiles;

        // set polygons as walkable
        // TODO: special flags for DYNAMIC polygons, ie surfaces that can be turned on and off
        for (int i = 0; i < iv.polyMesh->npolys; ++i)
            if (uint8 area = iv.polyMesh->areas[i] & NAV_ALL_MASK)
                iv.polyMesh->flags[i] = (area >= NAV_MIN_VALUE) ? (1 << (NAV_MAX_VALUE - iv.polyMesh->areas[i])) : NAV_GROUND;

        // setup mesh parameters
        dtNavMeshCreateParams params;
        memset(&params, 0, sizeof(params));
        params.verts = iv.polyMesh->verts;
        params.vertCount = iv.polyMesh->nverts;
        params.polys = iv.polyMesh->polys;
        params.polyAreas = iv.polyMesh->areas;
        params.polyFlags = iv.polyMesh->flags;
        params.polyCount = iv.polyMesh->npolys;
        params.nvp = iv.polyMesh->nvp;
        params.detailMeshes = iv.polyMeshDetail->meshes;
        params.detailVerts = iv.polyMeshDetail->verts;
        params.detailVertsCount = iv.polyMeshDetail->nverts;
        params.detailTris = iv.polyMeshDetail->tris;
        params.detailTriCount = iv.polyMeshDetail->ntris;

        params.offMeshConVerts = meshData.offMeshConnections.getCArray();
        params.offMeshConCount = meshData.offMeshConnections.size()/6;
        params.offMeshConRad = meshData.offMeshConnectionRads.getCArray();
        params.offMeshConDir = meshData.offMeshConnectionDirs.getCArray();
        params.offMeshConAreas = meshData.offMeshConnectionsAreas.getCArray();
        params.offMeshConFlags = meshData.offMeshConnectionsFlags.getCArray();

        params.walkableHeight = BASE_UNIT_DIM*config.walkableHeight;    // agent height
        params.walkableRadius = BASE_UNIT_DIM*config.walkableRadius;    // agent radius
        params.walkableClimb = BASE_UNIT_DIM*config.walkableClimb;      // keep less that walkableHeight (aka agent height)!
        params.tileX = (((bmin[0] + bmax[0]) / 2) - navMesh->getParams()->orig[0]) / GRID_SIZE;
        params.tileY = (((bmin[2] + bmax[2]) / 2) - navMesh->getParams()->orig[2]) / GRID_SIZE;
        rcVcopy(params.bmin, bmin);
        rcVcopy(params.bmax, bmax);
        params.cs = config.cs;
        params.ch = config.ch;
        params.tileLayer = 0;
        params.buildBvTree = true;

        // 35,39 | -1821.15 -4208.10 3.15 | -4208.10 3.15 -1821.15
        // will hold final navmesh
        unsigned char* navData = NULL;
        int navDataSize = 0;

        do
        {
            // these values are checked within dtCreateNavMeshData - handle them here
            // so we have a clear error message
            if (params.nvp > DT_VERTS_PER_POLYGON)
            {
                printf("%s Invalid verts-per-polygon value!        \n", tileString);
                continue;
            }
            if (params.vertCount >= 0xffff)
            {
                printf("%s Too many vertices!                      \n", tileString);
                continue;
            }
            if (!params.vertCount || !params.verts)
            {
                // occurs mostly when adjacent tiles have models
                // loaded but those models don't span into this tile

                // message is an annoyance
                //printf("%sNo vertices to build tile!              \n", tileString);
                continue;
            }
            if (!params.polyCount || !params.polys ||
                TILES_PER_MAP*TILES_PER_MAP == params.polyCount)
            {
                // we have flat tiles with no actual geometry - don't build those, its useless
                // keep in mind that we do output those into debug info
                // drop tiles with only exact count - some tiles may have geometry while having less tiles
                printf("%s No polygons to build on tile!              \n", tileString);
                continue;
            }
            if (!params.detailMeshes || !params.detailVerts || !params.detailTris)
            {
                printf("%s No detail mesh to build tile!           \n", tileString);
                continue;
            }

            //printf("%s Building navmesh tile...           \r", tileString);
            if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
            {
                printf("%s Failed building navmesh tile!           \n", tileString);
                continue;
            }

            dtTileRef tileRef = 0;
            //printf("%s Adding tile to navmesh...           \r", tileString);
            // DT_TILE_FREE_DATA tells detour to unallocate memory when the tile
            // is removed via removeTile()
            dtStatus dtResult = navMesh->addTile(navData, navDataSize, DT_TILE_FREE_DATA, 0, &tileRef);
            if (!tileRef || dtResult != DT_SUCCESS)
            {
                printf("%s Failed adding tile to navmesh!           \n", tileString);
                continue;
            }

            // file output
            char fileName[255];
            sprintf(fileName, "mmaps/%03u%02i%02i.mmtile", mapID, tileX, tileY);
            FILE* file = fopen(fileName, "wb");
            if (!file)
            {
                char message[1024];
                sprintf(message, "Failed to open %s for writing!             \n", fileName);
                perror(message);
                navMesh->removeTile(tileRef, NULL, NULL);
                continue;
            }

            //printf("%s Writing to file...             \r", tileString);

            // write header
            MmapTileHeader header;
            header.usesLiquids = m_terrainBuilder->usesLiquids();
            header.size = G3D::uint32(navDataSize);
            fwrite(&header, sizeof(MmapTileHeader), 1, file);

            // write data
            fwrite(navData, sizeof(unsigned char), navDataSize, file);
            fclose(file);

            // now that tile is written to disk, we can unload it
            navMesh->removeTile(tileRef, NULL, NULL);
        }
        while (0);

        if (m_debugOutput)
        {
            // restore padding so that the debug visualization is correct
            for (int i = 0; i < iv.polyMesh->nverts; ++i)
            {
                unsigned short* v = &iv.polyMesh->verts[i*3];
                v[0] += (unsigned short)config.borderSize;
                v[2] += (unsigned short)config.borderSize;
            }

            iv.generateObjFile(mapID, tileX, tileY, meshData);
            iv.writeIV(mapID, tileX, tileY);
        }
    }

    static Mutex fileLock;
    /**************************************************************************/
    void MapBuilder::buildMoveMapTile(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapLoadData &loadData, float bmin[3], float bmax[3], dtNavMesh* navMesh)
    {   // 35,39 | -1821.15 -4208.10 3.15 | -1821.15 3.15 -4208.10
        // console output
        char tileString[20];
        sprintf(tileString, "[%02i,%02i]: ", tileX, tileY);

        IntermediateValues iv;

        // Base everything off of chunk counts for our tile
        const static int TILES_PER_MAP = CHUNK_SIZE+2;
        const static int VERTEX_PER_TILE = V8_CH_SIZE_SQ+V9_CH_SIZE_SQ; // must divide VERTEX_PER_MAP
        const static int VERTEX_PER_MAP = TILES_PER_MAP*VERTEX_PER_TILE;

        // value have to divide GRID_SIZE(533.33333f) ( aka: 0.5333, 0.2666, 0.3333, 0.1333, etc )
        static float BASE_UNIT_MOVE_SIZE = (GRID_SIZE + (GRID_CHUNK_SIZE*2)) / VERTEX_PER_MAP;
        // Current formula is (533.3333+((533.3333/16)*2))/2610 = 0.22988504313
        // Recast will interpolate our supplied triangles with values based on this walk distance, so file size bloats based on how small this value is

        rcConfig config;
        memset(&config, 0, sizeof(rcConfig));

        rcVcopy(config.bmin, bmin);
        rcVcopy(config.bmax, bmax);

        config.maxVertsPerPoly = DT_VERTS_PER_POLYGON;
        config.cs = BASE_UNIT_MOVE_SIZE;
        config.ch = BASE_UNIT_MOVE_SIZE;
        config.walkableSlopeAngle = m_maxWalkableAngle;
        config.walkableSlopeAngleNotNavigatable = 80;
        config.tileSize = VERTEX_PER_TILE;
        config.walkableRadius = m_bigBaseUnit ? 1 : 2;
        config.borderSize = config.walkableRadius + 3;
        config.maxEdgeLen = VERTEX_PER_TILE + 1;        //anything bigger than tileSize
        config.walkableHeight = m_bigBaseUnit ? 3 : 6;
        config.walkableClimb = m_bigBaseUnit ? 3 : 6;
        config.minRegionArea = rcSqr(60);
        config.mergeRegionArea = rcSqr(50);
        config.maxSimplificationError = 1.8f;       // eliminates most jagged edges (tinny polygons)
        config.detailSampleDist = config.cs * 16;
        config.detailSampleMaxError = config.ch * 1;
        if(mapID == 0 && tileX == 35 && tileY == 39)
            printf("");

        switch (mapID)
        {
        // Blade's Edge Arena
        case 562: // This allows to walk on the ropes to the pillars
            config.walkableRadius = 0;
            break;
        // Blackfathom Deeps
        case 48: // Reduce the chance to have underground levels
            config.ch *= 2;
            break;
        }

        // this sets the dimensions of the heightfield - should maybe happen before border padding
        rcCalcGridSize(config.bmin, config.bmax, config.cs, &config.width, &config.height);

        // allocate subregions : tiles
        Tile* tiles = new Tile[TILES_PER_MAP * TILES_PER_MAP];

        // Initialize per tile config.
        rcConfig tileCfg = config;
        tileCfg.width = config.tileSize + config.borderSize*2;
        tileCfg.height = config.tileSize + config.borderSize*2;

        // merge per tile poly and detail meshes
        rcPolyMesh** pmmerge = new rcPolyMesh*[TILES_PER_MAP * TILES_PER_MAP];
        if (!pmmerge)
        {
            printf("%s alloc pmmerge FAILED!             \n", tileString);
            return;
        }

        rcPolyMeshDetail** dmmerge = new rcPolyMeshDetail*[TILES_PER_MAP * TILES_PER_MAP];
        if (!dmmerge)
        {
            printf("%s alloc dmmerge FAILED!             \n", tileString);
            return;
        }

        int nmerge = 0;

        float *cVMapVerts = loadData.vmapSolidVerts.getCArray();
        int cVMapVertCount = loadData.vmapSolidVerts.size() / 3;

        int *cVMapTris = loadData.vmapSolidTris.getCArray();
        int cVMapTriCount = loadData.vmapSolidTris.size() / 3;

        // Add Vmap Triangles
        unsigned char *cVMapFlags = new unsigned char[cVMapTriCount];
        memset(cVMapFlags, NAV_GROUND, cVMapTriCount*sizeof(unsigned char));
        {   //rcMarkWalkableTriangles(m_rcContext, tileCfg.walkableSlopeAngleNotSteep, cVerts, cVertCount, cTris, cTriCount, cFlags, NAV_GROUND);
            float notWalkableThr = cosf(tileCfg.walkableSlopeAngle/180.0f*RC_PI), notNavigatableThr = cosf(tileCfg.walkableSlopeAngleNotNavigatable/180.0f*RC_PI);

            float norm[3];

            for (int i = 0; i < cVMapTriCount; ++i)
            {
                float grade = 0.f;
                const int* tri = &cVMapTris[i*3];
                float e0[3];//, e1[3];
                float* v0 = &cVMapVerts[tri[0]*3], *v1 = &cVMapVerts[tri[1]*3], *v2 = &cVMapVerts[tri[2]*3];
                //static void calcTriNormal(const float* v0, const float* v1, const float* v2, float* norm)
                {   //calcTriNormal(&verts[tri[0]*3], &verts[tri[1]*3], &verts[tri[2]*3], norm);
                    //float* v0 = &cVerts[tri[0]*3], *v1 = &cVerts[tri[1]*3], *v2 = &cVerts[tri[2]*3];
                    //float e0[3], e1[3];
                    e0[0] = std::min<float>(v0[0], std::min<float>(v1[0], v2[0]));
                    e0[1] = std::min<float>(v0[1], std::min<float>(v1[1], v2[1]));
                    e0[2] = std::min<float>(v0[2], std::min<float>(v1[2], v2[2]));

                    float mid[3] = { (v0[0]+v1[0]+v2[0])/3.f, (v0[1]+v1[1]+v2[1])/3.f, (v0[2]+v1[2]+v2[2])/3.f };
                    float dist = sqrtf(((mid[0]-e0[0])*(mid[0]-e0[0])) + ((mid[2]-e0[2])*(mid[2]-e0[2])));
                    grade = ((mid[1] - e0[1])/dist)*100.f;

                    /*rcVsub(e0, v1, v0);
                    rcVsub(e1, v2, v0);
                    rcVcross(norm, e0, e1);
                    rcVnormalize(norm);*/
                }
                float gradeCosine = cosf((grade / 180.0f) * RC_PI);

                // Mark flags of areas either too steep to include or not walkable
                if (gradeCosine < notNavigatableThr)
                    cVMapFlags[i] = NAV_EMPTY;
                else if (gradeCosine < notWalkableThr)
                    cVMapFlags[i] = NAV_TOO_STEEP;
            }
        }

        // build all tiles
        for (int y = 0; y < TILES_PER_MAP; ++y)
        {
            for (int x = 0; x < TILES_PER_MAP; ++x)
            {
                Tile& tile = tiles[x + y * TILES_PER_MAP];
                if(loadData._chunkData[x][y].xOffset == 0xFF || loadData._chunkData[x][y].yOffset == 0xFF)
                    continue;

                MapLoadData::ChunkInfo *info = &loadData._chunkData[x][y];
                // XZY
                tileCfg.bmin[0] = loadData._chunkData[x][y].min[0];
                tileCfg.bmin[1] = loadData._chunkData[x][y].min[1];
                tileCfg.bmin[2] = loadData._chunkData[x][y].min[2];

                tileCfg.bmax[0] = loadData._chunkData[x][y].max[0];
                tileCfg.bmax[1] = loadData._chunkData[x][y].max[1];
                tileCfg.bmax[2] = loadData._chunkData[x][y].max[2];

                // build heightfield
                tile.solid = rcAllocHeightfield();
                if (!tile.solid || !rcCreateHeightfield(m_rcContext, *tile.solid, tileCfg.width, tileCfg.height, tileCfg.bmin, tileCfg.bmax, tileCfg.cs, tileCfg.ch))
                {
                    printf("%s Failed building heightfield!            \n", tileString);
                    continue;
                }

                float *cVerts = loadData._chunkData[x][y].combinedVerts.getCArray();
                int cVertCount = loadData._chunkData[x][y].combinedVerts.size() / 3;

                int *cTris = loadData._chunkData[x][y].combinedTris.getCArray();
                int cTriCount = loadData._chunkData[x][y].combinedTris.size() / 3;

                unsigned char *cFlags = loadData._chunkData[x][y].combinedFlags.getCArray();

                /*void rcClearUnwalkableTriangles(rcContext* ctx, const float walkableSlopeAngle,
                                const float* verts, int nv,
                                const int* tris, int nt,
                                unsigned char* areas) */
                //rcClearUnwalkableTriangles(m_rcContext, tileCfg.walkableSlopeAngle, cVerts, cVertCount, cTris, cTriCount, cFlags);
                /*void rcMarkWalkableTriangles(rcContext* ctx, const float walkableSlopeAngle,
                *            const float* verts, int nv,
                *            const int* tris, int nt,
                *            unsigned char* areas, unsigned char areaType) */
                {   //rcMarkWalkableTriangles(m_rcContext, tileCfg.walkableSlopeAngleNotSteep, cVerts, cVertCount, cTris, cTriCount, cFlags, NAV_GROUND);
                    float notWalkableThr = cosf(tileCfg.walkableSlopeAngle/180.0f*RC_PI), notNavigatableThr = cosf(tileCfg.walkableSlopeAngleNotNavigatable/180.0f*RC_PI);

                    float norm[3];

                    for (int i = 0; i < cTriCount; ++i)
                    {
                        int terrain = cFlags[i];
                        // Added this to our code, only affect default terrain tiles, avoid water tiles.
                        if(terrain != NAV_GROUND)
                            continue;

                        float grade = 0.f;
                        const int* tri = &cTris[i*3];
                        float e0[3], e1[3];
                        float* v0 = &cVerts[tri[0]*3], *v1 = &cVerts[tri[1]*3], *v2 = &cVerts[tri[2]*3];
                        //static void calcTriNormal(const float* v0, const float* v1, const float* v2, float* norm)
                        {   //calcTriNormal(&verts[tri[0]*3], &verts[tri[1]*3], &verts[tri[2]*3], norm);
                            //float* v0 = &cVerts[tri[0]*3], *v1 = &cVerts[tri[1]*3], *v2 = &cVerts[tri[2]*3];
                            //float e0[3], e1[3];
                            e0[0] = std::min<float>(v0[0], std::min<float>(v1[0], v2[0]));
                            e0[1] = std::min<float>(v0[1], std::min<float>(v1[1], v2[1]));
                            e0[2] = std::min<float>(v0[2], std::min<float>(v1[2], v2[2]));

                            float mid[3] = { (v0[0]+v1[0]+v2[0])/3.f, (v0[1]+v1[1]+v2[1])/3.f, (v0[2]+v1[2]+v2[2])/3.f };
                            float dist = sqrtf(((mid[0]-e0[0])*(mid[0]-e0[0])) + ((mid[2]-e0[2])*(mid[2]-e0[2])));
                            grade = ((mid[1] - e0[1])/dist)*100.f;

                            /*rcVsub(e0, v1, v0);
                            rcVsub(e1, v2, v0);
                            rcVcross(norm, e0, e1);
                            rcVnormalize(norm);*/
                        }
                        float gradeCosine = cosf((grade / 180.0f) * RC_PI);

                        // Mark flags of areas either too steep to include or not walkable
                        if (gradeCosine < notNavigatableThr)
                            cFlags[i] = NAV_EMPTY;
                        else if (gradeCosine < notWalkableThr)
                            cFlags[i] = NAV_TOO_STEEP;
                    }
                }
                rcRasterizeTriangles(m_rcContext, cVerts, cVertCount, cTris, cFlags, cTriCount, *tile.solid, config.walkableClimb);

                // Process our vmap terrain verts and triangles
                rcRasterizeTriangles(m_rcContext, cVMapVerts, cVMapVertCount, cVMapTris, cVMapFlags, cVMapTriCount, *tile.solid, config.walkableClimb);

                // Process our vmap liquid verts and triangles
                cVerts = loadData.vmapLiquidVerts.getCArray();
                cVertCount = loadData.vmapLiquidVerts.size() / 3;

                cTris = loadData.vmapLiquidTris.getCArray();
                cTriCount = loadData.vmapLiquidTris.size() / 3;

                // Add Vmap liquid Triangles
                cFlags = loadData.vmapLiquidTypes.getCArray();
                rcRasterizeTriangles(m_rcContext, cVerts, cVertCount, cTris, cFlags, cTriCount, *tile.solid, config.walkableClimb);

                // Filter our tile
                rcFilterLowHangingWalkableObstacles(m_rcContext, config.walkableClimb, *tile.solid);
                rcFilterLedgeSpans(m_rcContext, tileCfg.walkableHeight, tileCfg.walkableClimb, *tile.solid);
                rcFilterWalkableLowHeightSpans(m_rcContext, tileCfg.walkableHeight, *tile.solid);

                // compact heightfield spans
                tile.chf = rcAllocCompactHeightfield();
                if (!tile.chf || !rcBuildCompactHeightfield(m_rcContext, tileCfg.walkableHeight, tileCfg.walkableClimb, *tile.solid, *tile.chf))
                {
                    printf("%s Failed compacting heightfield!            \n", tileString);
                    continue;
                }

                // build polymesh intermediates
                if (!rcErodeWalkableArea(m_rcContext, config.walkableRadius, *tile.chf))
                {
                    printf("%s Failed eroding area!                    \n", tileString);
                    continue;
                }

                if (!rcMedianFilterWalkableArea(m_rcContext, *tile.chf))
                {
                    printf("%s Failed filtering walkable area!         \n", tileString);
                    continue;
                }

                if (!rcBuildDistanceField(m_rcContext, *tile.chf))
                {
                    printf("%s Failed building distance field!         \n", tileString);
                    continue;
                }

                if (!rcBuildRegions(m_rcContext, *tile.chf, tileCfg.borderSize, tileCfg.minRegionArea, tileCfg.mergeRegionArea))
                {
                    printf("%s Failed building regions!                \n", tileString);
                    continue;
                }

                tile.cset = rcAllocContourSet();
                if (!tile.cset || !rcBuildContours(m_rcContext, *tile.chf, tileCfg.maxSimplificationError, tileCfg.maxEdgeLen, *tile.cset))
                {
                    printf("%s Failed building contours!               \n", tileString);
                    continue;
                }

                // build polymesh
                tile.pmesh = rcAllocPolyMesh();
                if (!tile.pmesh || !rcBuildPolyMesh(m_rcContext, *tile.cset, tileCfg.maxVertsPerPoly, *tile.pmesh))
                {
                    printf("%s Failed building polymesh!               \n", tileString);
                    continue;
                }

                tile.dmesh = rcAllocPolyMeshDetail();
                if (!tile.dmesh || !rcBuildPolyMeshDetail(m_rcContext, *tile.pmesh, *tile.chf, tileCfg.detailSampleDist, tileCfg.detailSampleMaxError, *tile.dmesh))
                {
                    printf("%s Failed building polymesh detail!        \n", tileString);
                    continue;
                }

                // free those up
                // we may want to keep them in the future for debug
                // but right now, we don't have the code to merge them
                rcFreeHeightField(tile.solid);
                tile.solid = NULL;
                rcFreeCompactHeightfield(tile.chf);
                tile.chf = NULL;
                rcFreeContourSet(tile.cset);
                tile.cset = NULL;

                pmmerge[nmerge] = tile.pmesh;
                dmmerge[nmerge] = tile.dmesh;
                nmerge++;
            }
        }

        iv.polyMesh = rcAllocPolyMesh();
        if (!iv.polyMesh)
        {
            printf("%s alloc iv.polyMesh FIALED!             \n", tileString);
            delete[] pmmerge;
            delete[] dmmerge;
            delete[] tiles;
            return;
        }
        rcMergePolyMeshes(m_rcContext, pmmerge, nmerge, *iv.polyMesh);

        iv.polyMeshDetail = rcAllocPolyMeshDetail();
        if (!iv.polyMeshDetail)
        {
            printf("%s alloc m_dmesh FIALED!             \n", tileString);
            delete[] pmmerge;
            delete[] dmmerge;
            delete[] tiles;
            return;
        }
        rcMergePolyMeshDetails(m_rcContext, dmmerge, nmerge, *iv.polyMeshDetail);

        // free things up
        delete[] pmmerge;
        delete[] dmmerge;
        delete[] tiles;

        // set polygons as walkable
        // TODO: special flags for DYNAMIC polygons, ie surfaces that can be turned on and off
        for (int i = 0; i < iv.polyMesh->npolys; ++i)
            if (uint8 area = iv.polyMesh->areas[i] & NAV_ALL_MASK)
                iv.polyMesh->flags[i] = (area >= NAV_MIN_VALUE) ? (1 << (NAV_MAX_VALUE - iv.polyMesh->areas[i])) : NAV_GROUND;

        // setup mesh parameters
        dtNavMeshCreateParams params;
        memset(&params, 0, sizeof(params));
        params.verts = iv.polyMesh->verts;
        params.vertCount = iv.polyMesh->nverts;
        params.polys = iv.polyMesh->polys;
        params.polyAreas = iv.polyMesh->areas;
        params.polyFlags = iv.polyMesh->flags;
        params.polyCount = iv.polyMesh->npolys;
        params.nvp = iv.polyMesh->nvp;
        params.detailMeshes = iv.polyMeshDetail->meshes;
        params.detailVerts = iv.polyMeshDetail->verts;
        params.detailVertsCount = iv.polyMeshDetail->nverts;
        params.detailTris = iv.polyMeshDetail->tris;
        params.detailTriCount = iv.polyMeshDetail->ntris;

        params.offMeshConVerts = loadData.offMeshConnections.getCArray();
        params.offMeshConCount = loadData.offMeshConnections.size()/6;
        params.offMeshConRad = loadData.offMeshConnectionRads.getCArray();
        params.offMeshConDir = loadData.offMeshConnectionDirs.getCArray();
        params.offMeshConAreas = loadData.offMeshConnectionsAreas.getCArray();
        params.offMeshConFlags = loadData.offMeshConnectionsFlags.getCArray();

        params.walkableHeight = BASE_UNIT_MOVE_SIZE*config.walkableHeight;    // agent height
        params.walkableRadius = BASE_UNIT_MOVE_SIZE*config.walkableRadius;    // agent radius
        params.walkableClimb = BASE_UNIT_MOVE_SIZE*config.walkableClimb;      // keep less that walkableHeight (aka agent height)!
        params.tileX = (((bmin[0] + bmax[0]) / 2) - navMesh->getParams()->orig[0]) / GRID_SIZE;
        params.tileY = (((bmin[2] + bmax[2]) / 2) - navMesh->getParams()->orig[2]) / GRID_SIZE;
        rcVcopy(params.bmin, bmin);
        rcVcopy(params.bmax, bmax);
        params.cs = config.cs;
        params.ch = config.ch;
        params.tileLayer = 0;
        params.buildBvTree = true;

        // 35,39 | -1821.15 -4208.10 3.15 | -1821.15 3.15 -4208.10
        // will hold final navmesh
        unsigned char* navData = NULL;
        int navDataSize = 0;

        do
        {
            // these values are checked within dtCreateNavMeshData - handle them here
            // so we have a clear error message
            if (params.nvp > DT_VERTS_PER_POLYGON)
            {
                printf("%s Invalid verts-per-polygon value!        \n", tileString);
                continue;
            }
            if (params.vertCount >= 0xffff)
            {
                printf("%s Too many vertices!                      \n", tileString);
                continue;
            }
            if (!params.vertCount || !params.verts)
            {
                // occurs mostly when adjacent tiles have models
                // loaded but those models don't span into this tile

                // message is an annoyance
                //printf("%sNo vertices to build tile!              \n", tileString);
                continue;
            }
            if (!params.polyCount || !params.polys)
            {
                // we have flat tiles with no actual geometry - don't build those, its useless
                // keep in mind that we do output those into debug info
                // drop tiles with only exact count - some tiles may have geometry while having less tiles
                printf("%s No polygons to build on tile!              \n", tileString);
                continue;
            }
            if (!params.detailMeshes || !params.detailVerts || !params.detailTris)
            {
                printf("%s No detail mesh to build tile!           \n", tileString);
                continue;
            }

            //printf("%s Building navmesh tile...           \r", tileString);
            if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
            {
                printf("%s Failed building navmesh tile!           \n", tileString);
                continue;
            }

            dtTileRef tileRef = 0;
            //printf("%s Adding tile to navmesh...           \r", tileString);
            // DT_TILE_FREE_DATA tells detour to unallocate memory when the tile
            // is removed via removeTile()
            dtStatus dtResult = navMesh->addTile(navData, navDataSize, DT_TILE_FREE_DATA, 0, &tileRef);
            if (!tileRef || dtResult != DT_SUCCESS)
            {
                printf("%s Failed adding tile to navmesh!           \n", tileString);
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
                sprintf(message, "Failed to open %s for writing!             \n", fileName);
                perror(message);
                navMesh->removeTile(tileRef, NULL, NULL);
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

            // write header
            MmapTileHeader header;
            header.usesLiquids = m_terrainBuilder->usesLiquids();
            header.size = G3D::uint32(navDataSize);
            fwrite(&header, sizeof(MmapTileHeader), 1, file);

            // write data
            fwrite(navData, sizeof(unsigned char), navDataSize, file);
            fclose(file);

            // now that tile is written to disk, we can unload it
            navMesh->removeTile(tileRef, NULL, NULL);
            fileLock.Release();
        }
        while (0);
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
    void MapBuilder::getTileBounds(G3D::uint32 tileX, G3D::uint32 tileY, MapLoadData &loadData, float* bmin, float* bmax)
    {
        // this is for width and depth
        bmin[0] = bmin[2] = bmax[0] = bmax[2] = 0.f;
        bmin[1] = VMAP_MAX_HEIGHT;
        bmax[1] = VMAP_INVALID_HEIGHT;

        bool hasVert = false;
        // this is for elevation
        if(hasVert = loadData.solidVerts.size())
        {
            float *tmin = new float[3], *tmax = new float[3];
            rcCalcBounds(loadData.solidVerts.getCArray(), (loadData.solidVerts.size()-3) / 3, tmin, tmax);
            bmin[1] = G3D::min(bmin[1], tmin[1]);
            bmax[1] = G3D::max(bmax[1], tmax[1]);
            float height = tmin[1], height2 = tmax[1];
            printf("");
        }

        if(hasVert = loadData.liquidVerts.size())
        {
            float *lmin = new float[3], *lmax = new float[3];
            rcCalcBounds(loadData.liquidVerts.getCArray(), loadData.liquidVerts.size() / 3, lmin, lmax);
            bmin[1] = G3D::min(bmin[1], lmin[1]);
            bmax[1] = G3D::max(bmax[1], lmax[1]);
        }

        if(!hasVert)
        {
            bmin[1] = VMAP_INVALID_HEIGHT;
            bmax[1] = VMAP_MAX_HEIGHT;
        }

        // this is for width and depth
        bmax[2] = (32 - int(tileY)) * GRID_SIZE + GRID_CHUNK_SIZE;
        bmax[0] = (32 - int(tileX)) * GRID_SIZE + GRID_CHUNK_SIZE;
        bmin[2] = bmax[2] - GRID_SIZE - GRID_CHUNK_SIZE*2.f;
        bmin[0] = bmax[0] - GRID_SIZE - GRID_CHUNK_SIZE*2.f;
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
