/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

TerrainMgr::TerrainMgr(std::string MapPath, uint32 MapId) : file_name(MapPath), dummyMap(false), mapId(MapId), m_dataLoader(NULL), m_vmapOffset(0), mapCompessionType(MAP_COMPRESSED_MAX)
{
    file_name.append('/'+format("%03u.tiletree", mapId));

    for(uint8 i = 0; i < 64; i++)
        for(uint8 i2 = 0; i2 < 64; i2++)
            LoadCounter[i][i2] = 0;
}

TerrainMgr::~TerrainMgr()
{
    tileInformation.clear();
    sVMapInterface.DeactivateMap(mapId);
}

bool TerrainMgr::LoadTerrainHeader()
{
    // Create the 
    FILE *FileDescriptor = fopen(file_name.c_str(), "rb");
    if(FileDescriptor == NULL)
    {
        dummyMap = true;
        return false;
    }

    char identifier[10];
    fread(&identifier, 10, 1, FileDescriptor);
    if(strcmp(identifier, heightMapHeader))
    {   // If we have no height data but a proper vmap header, then load the vmap header
        if(strcmp(identifier, VMAP::RAW_VMAP_MAGIC) == 0)
            m_vmapOffset = 0xFFFFFFFF;
        else sLog.Error("TerrainMgr", "Bad header for %s. Type %s", file_name.c_str(), identifier);
        fclose(FileDescriptor);
        dummyMap = true;
        return false;
    }

    /* check file size */
    fseek(FileDescriptor, 0, SEEK_END);
    uint32 fileSize = ftell(FileDescriptor);
    if(fileSize == (10 + terrainHeaderSize + sizeof(uint32) + sizeof(uint32)))
    {
        sLog.outDebug("Map file %s Ignored.", file_name.c_str());

        /* file with no data */
        fclose(FileDescriptor);
        dummyMap = true;
        return false;
    }

    // Read in the header.
    fseek(FileDescriptor, 10, SEEK_SET);
    uint32 tileOffsets[64][64];
    if(fread(tileOffsets, terrainHeaderSize, 1, FileDescriptor) != 1 || fread(&m_vmapOffset, sizeof(uint32), 1, FileDescriptor) != 1)
    {
        sLog.Error("TerrainMgr", "Terrain header read failed for %s!", file_name.c_str());
        fclose(FileDescriptor);
        dummyMap = true;
        return false;
    }

    // Read map compression style/type
    if(fread(&mapCompessionType, sizeof(uint32), 1, FileDescriptor) != 1 || mapCompessionType == 0 || mapCompessionType >= MAP_COMPRESSED_MAX)
    {
        sLog.Error("TerrainMgr", "Terrain compression type failed for %s!", file_name.c_str());
        fclose(FileDescriptor);
        dummyMap = true;
        return false;
    }

    if(m_vmapOffset == fileSize)
        m_vmapOffset = 0;

    fclose(FileDescriptor);
    for(uint8 x = 0; x < 64; ++x)
    {
        for(uint8 y = 0; y < 64; ++y)
        {
            if(tileOffsets[x][y] == 0)
                continue;
            m_tileOffsets.insert(std::make_pair(std::make_pair(x, y), tileOffsets[x][y]));
        }
    }

    return true;
}

bool TerrainMgr::LoadVMapTerrain()
{
    if(m_vmapOffset == 0)
        return false;

    // Create the 
    FILE *FileDescriptor = fopen(file_name.c_str(), "rb");
    if(FileDescriptor == NULL)
    {
        sLog.Error("TerrainMgr", "Map load failed for %s. Missing file?", file_name.c_str());
        return false;
    }

    if(m_vmapOffset != 0xFFFFFFFF)
        fseek(FileDescriptor, m_vmapOffset, SEEK_SET);

    if(!sVMapInterface.ActivateMap(mapId, FileDescriptor))
    {
        sLog.Error("TerrainMgr", "Failed to initialize vmap from map file %s", file_name.c_str());
        return false;
    }
    return true;
}

bool TerrainMgr::LoadTileInformation(uint32 x, uint32 y, FILE *input)
{
    if(dummyMap == true)
        return false;

    std::pair<uint8, uint8> offsetPair = std::make_pair(x, y);
    if(m_tileOffsets.find(offsetPair) == m_tileOffsets.end())
        return false;

    uint32 Offset = 0; // Find our offset in our cached header.
    if((Offset = m_tileOffsets.at(offsetPair)) == 0)
        return false;

    // Check that we haven't been loaded by another thread.
    if(tileInformation.find(offsetPair) != tileInformation.end())
        return true;

    // Seek to our specified offset.
    if(fseek(input, Offset, SEEK_SET) != 0)
        return false;

    MapDataLoader *mapLoader = NULL;
    if(m_dataLoader)
        mapLoader = m_dataLoader;
    else
    {
        switch(mapCompessionType)
        {
        case MAP_RAW_TILES: // Uncompressed bulk tiles
            {
                mapLoader = new MapRawTileDataLoader();
            }break;
        case MAP_RAW_CHUNKS: // Uncompressed bulk tile chunks
            {
                mapLoader = new MapRawChunkDataLoader();
            }break;
        case MAP_COMPRESSED_TILES:
            {
            // Unsupported
            }break;
        case MAP_COMPRESSED_CHUNKS: // Uncompressed bulk tile chunks
            {
                mapLoader = new MapCompressedChunkDataLoader();
            }break;
        }
    }

    if(mapLoader == NULL)
        return false;

    if(mapLoader->LoadMapInforation(x, y, input, mapCompessionType))
    {
        tileInformation.insert(std::make_pair(std::make_pair(x, y), mapLoader));
        return true;
    }

    return false;
}

void TerrainMgr::UnloadTileInformation(uint32 x, uint32 y)
{
    mutex.Acquire();

    std::pair<uint8, uint8> tilePair = std::make_pair(x, y);
    if(tileInformation.find(tilePair) != tileInformation.end())
    {
        MapDataLoader *data = tileInformation.at(tilePair);
        tileInformation.erase(tilePair);

        if(data->UnloadData())
            delete data;
    }
    mutex.Release();

    sLog.Debug("TerrainMgr","Unloaded tile information for tile [%u][%u]", x, y);
    return;
}

uint16 TerrainMgr::GetWaterType(float x, float y)
{
    if(!AreCoordinatesValid(x, y))
        return 0;

    // Convert the co-ordinates to tiles.
    uint32 TileX = ConvertGlobalXCoordinate(x);
    uint32 TileY = ConvertGlobalYCoordinate(y);

    mutex.Acquire();
    uint16 Liquid = 0;
    MapDataLoader *tileData = NULL;
    if(_TileInformationLoaded(TileX, TileY) && (tileData = GetTileInformation(TileX, TileY)))
        Liquid = tileData->GetWaterType(x, y);
    mutex.Release();
    return Liquid;
}

float TerrainMgr::GetWaterHeight(float x, float y)
{
    if(!AreCoordinatesValid(x, y))
        return NO_WATER_HEIGHT;

    // Convert the co-ordinates to tiles.
    uint32 TileX = ConvertGlobalXCoordinate(x);
    uint32 TileY = ConvertGlobalYCoordinate(y);

    mutex.Acquire();
    float WaterHeight = NO_WATER_HEIGHT;
    MapDataLoader *tileData = NULL;
    if(_TileInformationLoaded(TileX, TileY) && (tileData = GetTileInformation(TileX, TileY)))
    {
        WaterHeight = tileData->GetWaterHeight(x, y);
        if(WaterHeight == 0.0f && !(tileData->GetWaterType(x, y) & 0x02))
            WaterHeight = NO_WATER_HEIGHT;
    }
    mutex.Release();
    return WaterHeight;
}

uint8 TerrainMgr::GetWalkableState(float x, float y)
{
    // This has to be implemented.
    return 1;
}

uint16 TerrainMgr::GetAreaID(float x, float y)
{
    if(!AreCoordinatesValid(x, y))
        return 0xFFFF;

    // Convert the co-ordinates to tiles.
    uint32 TileX = ConvertGlobalXCoordinate(x);
    uint32 TileY = ConvertGlobalYCoordinate(y);

    mutex.Acquire();
    uint16 areaId = 0xFFFF;
    MapDataLoader *tileData = NULL;
    if(_TileInformationLoaded(TileX, TileY) && (tileData = GetTileInformation(TileX, TileY)))
        areaId = tileData->GetAreaId(x, y);
    mutex.Release();
    return areaId;
}

bool TerrainMgr::CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &AreaID)
{
    bool Result = false;
    CellGoneActive(CellX, CellY);

    mutex.Acquire();
    uint32 TileX = CellX/CellsPerTile, TileY = CellY/CellsPerTile;
    MapDataLoader *tileData = NULL;
    if(_TileInformationLoaded(TileX, TileY) && (tileData = GetTileInformation(TileX, TileY)))
        Result = tileData->CellHasAreaID(CellX, CellY, AreaID);
    mutex.Release();
    CellGoneIdle(CellX, CellY);
    return Result;
}

float TerrainMgr::GetLandHeight(float x, float y)
{
    if(!AreCoordinatesValid(x, y))
        return NO_LAND_HEIGHT;

    // Convert the co-ordinates to tiles.
    uint32 TileX = ConvertGlobalXCoordinate(x);
    uint32 TileY = ConvertGlobalYCoordinate(y);

    mutex.Acquire();
    float LandHeight = NO_LAND_HEIGHT;
    MapDataLoader *tileData = NULL;
    if(_TileInformationLoaded(TileX, TileY) && (tileData = GetTileInformation(TileX, TileY)))
        LandHeight = tileData->GetTerrainHeight(x, y);
    mutex.Release();
    return LandHeight;
}

void TerrainMgr::CellGoneActive(uint32 x, uint32 y)
{
    uint32 tileX = x/CellsPerTile, tileY = y/CellsPerTile;
    mutex.Acquire();
    if ((++LoadCounter[tileX][tileY]) == 1)
    {
        FILE* input = NULL;
        fopen_s(&input, file_name.c_str(), "rb");
        if (input)
        {
            LoadTileInformation(tileX, tileY, input);
            sVMapInterface.ActivateTile(mapId, tileX, tileY, input);
            sNavMeshInterface.LoadNavMesh(mapId, tileX, tileY);
            fclose(input);
        } else --LoadCounter[tileX][tileY];
    }
    mutex.Release();
}

void TerrainMgr::CellGoneIdle(uint32 x, uint32 y)
{
    uint32 tileX = x/CellsPerTile, tileY = y/CellsPerTile;
    mutex.Acquire();
    if((--LoadCounter[tileX][tileY]) == 0)
        UnloadTileInformation(tileX, tileY);
    sVMapInterface.DeactivateTile(mapId, tileX, tileY);
    sNavMeshInterface.UnloadNavMesh(mapId, tileX, tileY);
    mutex.Release();
}

void TerrainMgr::LoadAllTerrain()
{
    if(dummyMap == true)
        return;

    mutex.Acquire();
    FILE *input = NULL;
    fopen_s(&input, file_name.c_str(), "rb");
    if(input)
    {
        sLog.Debug("TerrainMgr", "[%u]: Loading all terrain", mapId);
        // Load basic terrain and navmesh first
        for(uint8 x = 0; x < 64; x++)
            for(uint8 y = 0; y < 64; y++)
                if(LoadTileInformation(x, y, input))
                    ++LoadCounter[x][y];

        // Load our vmap tiles since we've scaled past terrain
        for(uint8 x = 0; x < 64; x++)
            for(uint8 y = 0; y < 64; y++)
                sVMapInterface.ActivateTile(mapId, x, y, input);

        // Load navmesh data
        for(uint8 x = 0; x < 64; x++)
            for(uint8 y = 0; y < 64; y++)
                sNavMeshInterface.LoadNavMesh(mapId, x, y);

        fclose(input);
        sLog.Debug("TerrainMgr", "[%u]: All terrain loaded", mapId);
    }
    mutex.Release();
}

void TerrainMgr::UnloadAllTerrain(bool forced)
{
    mutex.Acquire();
    for(uint8 x = 0; x < 64; x++)
    {
        for(uint8 y = 0; y < 64; y++)
        {
            if(forced)
                LoadCounter[x][y] = 0;
            else LoadCounter[x][y]--;
            UnloadTileInformation(x, y);
            sVMapInterface.DeactivateTile(mapId, x, y);
            sNavMeshInterface.UnloadNavMesh(mapId, x, y);
        }
    }
    mutex.Release();
}

MapRawTileDataLoader::MapRawTileDataLoader()
{
    memset(&areaInfo, 0, sizeof(uint16)*CHUNK_SIZE*CHUNK_SIZE);
    memset(&liquidType, 0, sizeof(uint16)*CHUNK_SIZE*CHUNK_SIZE);
    memset(&V8, 0, sizeof(float)*V8_SIZE*V8_SIZE);
    memset(&V9, 0, sizeof(float)*V9_SIZE*V9_SIZE);
    memset(&liquidHeight, 0, sizeof(float)*V9_SIZE*V9_SIZE);
}

MapRawTileDataLoader::~MapRawTileDataLoader()
{

}

bool MapRawTileDataLoader::LoadMapInforation(uint32 x, uint32 y, FILE *input, uint32 compressionType)
{    // Read tile information in bulk
    ASSERT(compressionType == MAP_RAW_TILES);

    bool ret = false;
    if(fread(&areaInfo, sizeof(uint16)*CHUNK_SIZE_SQ, 1, input) == 1)
        if(fread(&liquidType, sizeof(uint16)*CHUNK_SIZE_SQ, 1, input) == 1)
            if(fread(&V9, sizeof(float)*V9_SIZE_SQ, 1, input) == 1)
                if(fread(&V8, sizeof(float)*V8_SIZE_SQ, 1, input) == 1)
                    if(fread(&liquidHeight, sizeof(float)*V9_SIZE_SQ, 1, input) == 1)
                        ret = true;
    if(ret)
        return MapDataLoader::LoadMapInforation(x, y, input, compressionType);
    return false;
}

uint16 MapRawTileDataLoader::GetAreaId(float x, float y)
{
    int lx = (int)(16 * (32 - x / TERRAIN_TILE_SIZE)) & 15;
    int ly = (int)(16 * (32 - y / TERRAIN_TILE_SIZE)) & 15;
    return areaInfo[lx*16 + ly];
}

float MapRawTileDataLoader::GetTerrainHeight(float x, float y)
{
    x = 128 * (32 - x / TERRAIN_TILE_SIZE);
    y = 128 * (32 - y / TERRAIN_TILE_SIZE);
    int tx = (int)x; x -= tx; tx &= 127;
    int ty = (int)y; y -= ty; ty &= 127;

    float a, b, c;
    if (x + y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            float h1 = V9[(tx) * 129 + ty];
            float h2 = V9[(tx + 1) * 129 + ty];
            float h5 = 2 * V8[tx * 128 + ty];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            float h1 = V9[tx * 129 + ty];
            float h3 = V9[tx * 129 + ty + 1];
            float h5 = 2 * V8[tx * 128 + ty];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            float h2 = V9[(tx + 1) * 129 + ty];
            float h4 = V9[(tx + 1) * 129 + ty + 1];
            float h5 = 2 * V8[tx * 128 + ty];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            float h3 = V9[(tx) * 129 + ty + 1];
            float h4 = V9[(tx + 1) * 129 + ty + 1];
            float h5 = 2 * V8[tx * 128 + ty];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }

    return a * x + b * y + c;
}

uint16 MapRawTileDataLoader::GetWaterType(float x, float y)
{
    int lx = (int)(16 * (32 - x / TERRAIN_TILE_SIZE)) & 15;
    int ly = (int)(16 * (32 - y / TERRAIN_TILE_SIZE)) & 15;
    return liquidType[lx*16 + ly];
}

// Liquid is a square so create our own triangles like so
// _____
// |\ b|
// |a\ |
// |__\|
//
float MapRawTileDataLoader::GetWaterHeight(float x, float y)
{
    int lx = (int)(129 * (32 - x / TERRAIN_TILE_SIZE)) & 128;
    int ly = (int)(129 * (32 - y / TERRAIN_TILE_SIZE)) & 128;
    return liquidHeight[lx*129 + ly];
}

bool MapRawTileDataLoader::CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &areaId)
{
    bool Result = false;
    for(uint32 xc = (CellX%CellsPerTile)*16/CellsPerTile;xc<(CellX%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;xc++)
    {
        for(uint32 yc = (CellY%CellsPerTile)*16/CellsPerTile;yc<(CellY%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;yc++)
        {
            if(uint16 areaId = areaInfo[xc*15+yc])
            {
                areaId = areaId;
                Result = true;
                break;
            }
        }
    }

    return Result;
}

MapRawChunkDataLoader::MapRawChunkDataLoader()
{
    memset(&areaInfo, 0, sizeof(uint16)*CHUNK_SIZE_SQ);
    memset(&liquidType, 0, sizeof(uint16)*CHUNK_SIZE_SQ);
    memset(&mChunkInfo, 0, sizeof(chunkInfo)*CHUNK_SIZE_SQ);
}

MapRawChunkDataLoader::~MapRawChunkDataLoader()
{

}

bool MapRawChunkDataLoader::LoadMapInforation(uint32 x, uint32 y, FILE *input, uint32 compressionType)
{
    ASSERT(compressionType == MAP_RAW_CHUNKS);

    bool ret = false;
    if(ret = (fread(&areaInfo, sizeof(uint16)*CHUNK_SIZE_SQ, 1, input) == 1))
        ret = fread(&liquidType, sizeof(uint16)*CHUNK_SIZE_SQ, 1, input) == 1;

    if(ret)
    {
        for(uint8 x = 0; x < CHUNK_SIZE; x++)
        {
            for(uint8 y = 0; y < CHUNK_SIZE; y++)
            {
                if(fread(&mChunkInfo[x][y].cxpos, sizeof(float), 1, input) == 1)
                    if(fread(&mChunkInfo[x][y].cypos, sizeof(float), 1, input) == 1)
                        if(fread(&mChunkInfo[x][y].T9, sizeof(float)*V9_SIZE_SQ, 1, input) == 1)
                            if(fread(&mChunkInfo[x][y].T8, sizeof(float)*V8_SIZE_SQ, 1, input) == 1)
                                if(fread(&mChunkInfo[x][y].liquidHeight, sizeof(float)*V9_SIZE_SQ, 1, input) == 1)
                                    ret = true;
            }
        }
    }

    if(ret)
        return MapDataLoader::LoadMapInforation(x, y, input, compressionType);
    return false;
}

uint16 MapRawChunkDataLoader::GetAreaId(float x, float y)
{
    int ax = (int)(16 * (32 - x / TERRAIN_TILE_SIZE)) & 15;
    int ay = (int)(16 * (32 - y / TERRAIN_TILE_SIZE)) & 15;
    return areaInfo[ax][ay];
}

float MapRawChunkDataLoader::GetTerrainHeight(float x, float y)
{
    // Grab our tile identifiers and offset
    int tx = (int)(16 * (32 - x / TERRAIN_TILE_SIZE)) & 15;
    int ty = (int)(16 * (32 - y / TERRAIN_TILE_SIZE)) & 15;

    x = 8 * (32 - x / 533.3333333f);
    y = 8 * (32 - y / 533.3333333f);

    int x_int = (int)x;
    int y_int = (int)y;

    x -= x_int;
    y -= y_int;

    x_int &= (8 - 1);
    y_int &= (8 - 1);

    // Grab our chunk info from tile based on IDs
    chunkInfo *info = &mChunkInfo[tx][ty];
    return info->T8[x_int][y_int];
}

uint16 MapRawChunkDataLoader::GetWaterType(float x, float y)
{
    int lx = (int)(16 * (32 - x / TERRAIN_TILE_SIZE)) & 15;
    int ly = (int)(16 * (32 - y / TERRAIN_TILE_SIZE)) & 15;
    return liquidType[lx][ly];
}

float MapRawChunkDataLoader::GetWaterHeight(float x, float y)
{
    // Grab our tile identifiers and offset
    int tx = (int)(16 * (32 - x / TERRAIN_TILE_SIZE)) & 15;
    int ty = (int)(16 * (32 - y / TERRAIN_TILE_SIZE)) & 15;

    x = 9 * (32 - x / 533.3333333f);
    y = 9 * (32 - y / 533.3333333f);

    int x_int = (int)x;
    int y_int = (int)y;

    x -= x_int;
    y -= y_int;

    x_int &= (9 - 1);
    y_int &= (9 - 1);

    // Grab our chunk info from tile based on IDs
    chunkInfo *info = &mChunkInfo[tx][ty];
    return info->liquidHeight[x_int][y_int];
}

bool MapRawChunkDataLoader::CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &areaId)
{
    bool Result = false;
    for(uint32 xc = (CellX%CellsPerTile)*16/CellsPerTile;xc<(CellX%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;xc++)
    {
        for(uint32 yc = (CellY%CellsPerTile)*16/CellsPerTile;yc<(CellY%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;yc++)
        {
            if(uint16 areaId = areaInfo[xc][yc])
            {
                areaId = areaId;
                Result = true;
                break;
            }
        }
    }

    return Result;
}

const float MapRawDataLoader::DATA_CHUNK_SIZE = (TILE_COUNT*TERRAIN_TILE_SIZE)/DATA_SIZE;
const float MapRawDataLoader::DATA_CHUNK_V8_SIZE = (TILE_COUNT*TERRAIN_TILE_SIZE)/V8_SIZE;
const float MapRawDataLoader::DATA_CHUNK_V9_SIZE = (TILE_COUNT*TERRAIN_TILE_SIZE)/V9_SIZE;

MapRawDataLoader::MapRawDataLoader()
{

}

MapRawDataLoader::~MapRawDataLoader()
{

}

bool MapRawDataLoader::LoadMapInforation(uint32 x, uint32 y, FILE *input, uint32 compressionType)
{
    ASSERT(compressionType == MAP_RAW_CHUNKS);

    bool ret = false;

    uint16 careaInfo[READ_CHUNK_SIZE][READ_CHUNK_SIZE];
    uint16 cliquidType[READ_CHUNK_SIZE][READ_CHUNK_SIZE];
    uint64 liquidMasks[READ_CHUNK_SIZE][READ_CHUNK_SIZE];

    struct chunkInfo
    {
        float xpos, ypos;
        float T8[READ_V8_SIZE][READ_V8_SIZE], T9[READ_V9_SIZE][READ_V9_SIZE];
        float liquidHeight[READ_V9_SIZE][READ_V9_SIZE];
    }mChunkInfo[READ_CHUNK_SIZE][READ_CHUNK_SIZE];

    if(ret = (fread(&careaInfo, sizeof(uint16)*READ_CHUNK_SIZE_SQ, 1, input) == 1))
        if(ret = (fread(&cliquidType, sizeof(uint16)*READ_CHUNK_SIZE_SQ, 1, input) == 1))
            ret = (fread(&liquidMasks, sizeof(uint64)*READ_CHUNK_SIZE_SQ, 1, input) == 1);

    if(ret)
    {
        for(uint8 x = 0; x < CHUNK_SIZE; x++)
        {
            for(uint8 y = 0; y < CHUNK_SIZE; y++)
            {
                if(fread(&mChunkInfo[x][y].xpos, sizeof(float), 1, input) == 1)
                    if(fread(&mChunkInfo[x][y].ypos, sizeof(float), 1, input) == 1)
                        if(fread(&mChunkInfo[x][y].T9, sizeof(float)*READ_V9_SIZE_SQ, 1, input) == 1)
                            if(fread(&mChunkInfo[x][y].T8, sizeof(float)*READ_V8_SIZE_SQ, 1, input) == 1)
                                if(fread(&mChunkInfo[x][y].liquidHeight, sizeof(float)*READ_V9_SIZE_SQ, 1, input) == 1)
                                    ret = true;
            }
        }
    }

    if(ret)
    {
        m_heightPlanes.init();
        for(uint32 tx = 0; tx < READ_CHUNK_SIZE; tx++)
        {
            uint32 mx = x * READ_CHUNK_SIZE + tx;
            for(uint32 ty = 0; ty < READ_CHUNK_SIZE; ty++)
            {
                uint32 my = y * READ_CHUNK_SIZE + ty;
                m_areaInfo[mx].insert(std::make_pair(my, careaInfo[tx][ty]));
                m_liquidInfo[mx].insert(std::make_pair(my, cliquidType[tx][ty]));
                m_heightPlanes.xmax[tx][ty] = mChunkInfo[tx][ty].xpos;
                m_heightPlanes.xmin[tx][ty] = mChunkInfo[tx][ty].xpos-TERRAIN_TILE_SIZE/16.f;
                m_heightPlanes.ymax[tx][ty] = mChunkInfo[tx][ty].ypos;
                m_heightPlanes.ymin[tx][ty] = mChunkInfo[tx][ty].ypos-TERRAIN_TILE_SIZE/16.f;

                float xposStart = mChunkInfo[tx][ty].xpos, yposStart = mChunkInfo[tx][ty].ypos;
                for(uint32 cx = 0; cx < READ_V8_SIZE; cx++)
                {
                    for(uint32 cy = 0; cy < READ_V8_SIZE; cy++)
                    {
                        uint32 chunkX = cx, chunkY = cy;

                        float curposX = xposStart - cx * DATA_CHUNK_V8_SIZE;
                        float curposY = yposStart - cy * DATA_CHUNK_V8_SIZE;

                        m_heightPlanes._heightPlanes[tx][ty].xmax[chunkX][chunkY] = curposX;
                        m_heightPlanes._heightPlanes[tx][ty].xmin[chunkX][chunkY] = curposX-DATA_CHUNK_V8_SIZE;

                        m_heightPlanes._heightPlanes[tx][ty].ymax[chunkX][chunkY] = curposY;
                        m_heightPlanes._heightPlanes[tx][ty].ymin[chunkX][chunkY] = curposY-DATA_CHUNK_V8_SIZE;

                        m_heightPlanes._heightPlanes[tx][ty].Theight[chunkX][chunkY][heightPlane::heights::TOP_LEFT] = mChunkInfo[tx][ty].T9[cx][cy];
                        m_heightPlanes._heightPlanes[tx][ty].Theight[chunkX][chunkY][heightPlane::heights::TOP_RIGHT] = mChunkInfo[tx][ty].T9[cx+1][cy];
                        m_heightPlanes._heightPlanes[tx][ty].Theight[chunkX][chunkY][heightPlane::heights::BOTTOM_LEFT] = mChunkInfo[tx][ty].T9[cx][cy+1];
                        m_heightPlanes._heightPlanes[tx][ty].Theight[chunkX][chunkY][heightPlane::heights::BOTTOM_RIGHT] = mChunkInfo[tx][ty].T9[cx+1][cy+1];
                        m_heightPlanes._heightPlanes[tx][ty].Theight[chunkX][chunkY][heightPlane::heights::CENTER] = mChunkInfo[tx][ty].T8[cx][cy];

                        m_heightPlanes._heightPlanes[tx][ty].Lheight[chunkX][chunkY][heightPlane::heights::TOP_LEFT] = mChunkInfo[tx][ty].liquidHeight[cx][cy];
                        m_heightPlanes._heightPlanes[tx][ty].Lheight[chunkX][chunkY][heightPlane::heights::TOP_RIGHT] = mChunkInfo[tx][ty].liquidHeight[cx+1][cy];
                        m_heightPlanes._heightPlanes[tx][ty].Lheight[chunkX][chunkY][heightPlane::heights::BOTTOM_LEFT] = mChunkInfo[tx][ty].liquidHeight[cx][cy+1];
                        m_heightPlanes._heightPlanes[tx][ty].Lheight[chunkX][chunkY][heightPlane::heights::BOTTOM_RIGHT] = mChunkInfo[tx][ty].liquidHeight[cx+1][cy+1];

                        float averageH = 0.f;
                        uint8 iAvg = 0;
                        for(uint8 i = 0; i < 4; i++)
                        {
                            if(m_heightPlanes._heightPlanes[tx][ty].Lheight[chunkX][chunkY][i] == NO_WATER_HEIGHT)
                                continue;
                            averageH += m_heightPlanes._heightPlanes[tx][ty].Lheight[chunkX][chunkY][i];
                            iAvg++;
                        }

                        if(iAvg == 0)
                            m_heightPlanes._heightPlanes[tx][ty].Lheight[chunkX][chunkY][heightPlane::heights::CENTER] = NO_WATER_HEIGHT;
                        else m_heightPlanes._heightPlanes[tx][ty].Lheight[chunkX][chunkY][heightPlane::heights::CENTER] = averageH/(float)iAvg;
                    }
                }
            }
        }

        return MapDataLoader::LoadMapInforation(x, y, input, compressionType);
    }
    return false;
}

uint16 MapRawDataLoader::GetAreaId(float x, float y)
{
    int ax = int32((float)DATA_CENTER_ID-(x/DATA_CHUNK_SIZE));
    int ay = int32((float)DATA_CENTER_ID-(y/DATA_CHUNK_SIZE));
    uint16 areaId = 0;
    if(m_areaInfo.find(ax) != m_areaInfo.end() && m_areaInfo[ax].find(ay) != m_areaInfo[ax].end())
        areaId = m_areaInfo[ax][ay];
    return areaId;
}

float MapRawDataLoader::GetTerrainHeight(float x, float y)
{
    return NO_LAND_HEIGHT;
/*    int tx = int32((float)DATA_CENTER_ID-(x/DATA_CHUNK_SIZE));
    int ty = int32((float)DATA_CENTER_ID-(y/DATA_CHUNK_SIZE));
    if(m_heightPlanes.find(tx) == m_heightPlanes.end() || m_heightPlanes[tx].find(ty) == m_heightPlanes[tx].end())
        return NO_LAND_HEIGHT;

    uint8 txo = 0xFF, tyo = 0xFF;
    heightPlane *plane = &m_heightPlanes[tx][ty];
    for(uint32 cx = 0; cx < READ_V8_SIZE; cx++)
    {
        for(uint32 cy = 0; cy < READ_V8_SIZE; cy++)
        {
            if(!plane->isInPlane(cx, cy, x, y))
                continue;
            txo = cx;
            tyo = cy;
        }
    }

    if(txo == 0xFF || tyo == 0xFF)
        return NO_LAND_HEIGHT;

    x -= plane->xmin[txo][tyo]; x /= DATA_CHUNK_V8_SIZE;
    y -= plane->ymin[txo][tyo]; y /= DATA_CHUNK_V8_SIZE;

    float a, b, c;
    if (x + y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            float h1 = plane->Theight[txo][tyo][heightPlane::heights::TOP_LEFT];
            float h2 = plane->Theight[txo][tyo][heightPlane::heights::TOP_RIGHT];
            float h5 = 2 * plane->Theight[txo][tyo][heightPlane::heights::CENTER];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            float h1 = plane->Theight[txo][tyo][heightPlane::heights::TOP_LEFT];
            float h3 = plane->Theight[txo][tyo][heightPlane::heights::BOTTOM_LEFT];
            float h5 = 2 * plane->Theight[txo][tyo][heightPlane::heights::CENTER];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            float h2 = plane->Theight[txo][tyo][heightPlane::heights::TOP_RIGHT];
            float h4 = plane->Theight[txo][tyo][heightPlane::heights::BOTTOM_RIGHT];
            float h5 = 2 * plane->Theight[txo][tyo][heightPlane::heights::CENTER];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            float h3 = plane->Theight[txo][tyo][heightPlane::heights::BOTTOM_LEFT];
            float h4 = plane->Theight[txo][tyo][heightPlane::heights::BOTTOM_RIGHT];
            float h5 = 2 * plane->Theight[txo][tyo][heightPlane::heights::CENTER];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }

    return a * x + b * y + c;*/
}

uint16 MapRawDataLoader::GetWaterType(float x, float y)
{
    int lx = int32((float)DATA_CENTER_ID-(x/DATA_CHUNK_SIZE));
    int ly = int32((float)DATA_CENTER_ID-(y/DATA_CHUNK_SIZE));
    if(m_liquidInfo.find(lx) == m_liquidInfo.end())
        return 0;
    if(m_liquidInfo[lx].find(ly) == m_liquidInfo[lx].end())
        return 0;
    return m_liquidInfo[lx][ly];
}

float MapRawDataLoader::GetWaterHeight(float x, float y)
{
    for(uint16 cx = 0; cx < 16; cx++)
    {
        for(uint16 cy = 0; cy < 16; cy++)
        {
            if(m_heightPlanes.isInPlane(cx, cy, x, y))
            {
                for(uint32 px = 0; px < READ_V8_SIZE; px++)
                {
                    for(uint32 py = 0; py < READ_V8_SIZE; py++)
                    {
                        if(m_heightPlanes._heightPlanes[cx][cy].isInPlane(px, py, x, y))
                        {
                            return m_heightPlanes._heightPlanes[cx][cy].Lheight[px][py][5];
                        }
                    }
                }
            }
        }
    }

    return NO_WATER_HEIGHT;
    /*int lx = int32((float)DATA_CENTER_ID-(x/DATA_CHUNK_SIZE));
    int ly = int32((float)DATA_CENTER_ID-(y/DATA_CHUNK_SIZE));
    if(m_heightPlanes.find(lx) == m_heightPlanes.end() || m_heightPlanes[lx].find(ly) == m_heightPlanes[lx].end())
        return NO_WATER_HEIGHT;

    uint8 txo = 0xFF, tyo = 0xFF;
    heightPlane *plane = &m_heightPlanes[lx][ly];
    for(uint32 cx = 0; cx < READ_V8_SIZE; cx++)
    {
        for(uint32 cy = 0; cy < READ_V8_SIZE; cy++)
        {
            if(!plane->isInPlane(cx, cy, x, y))
                continue;
            txo = cx;
            tyo = cy;
        }
    }

    if(txo == 0xFF || tyo == 0xFF)
        return NO_WATER_HEIGHT;
    return plane->Lheight[txo][tyo][0];*/
}

bool MapRawDataLoader::CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &areaId)
{
    bool Result = false;
    /*for(uint32 xc = (CellX%CellsPerTile)*16/CellsPerTile;xc<(CellX%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;xc++)
    {
        for(uint32 yc = (CellY%CellsPerTile)*16/CellsPerTile;yc<(CellY%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;yc++)
        {
            if(uint16 areaId = areaInfo[xc][yc])
            {
                areaId = areaId;
                Result = true;
                break;
            }
        }
    }*/

    return Result;
}

const float MapCompressedChunkDataLoader::DATA_CHUNK_V8_SIZE = TERRAIN_TILE_SIZE/(float)V8_SIZE;

MapCompressedChunkDataLoader::MapCompressedChunkDataLoader()
{
    memset(&areaInfo, 0, sizeof(uint16)*CHUNK_SIZE*CHUNK_SIZE);
    memset(&data_V8, 0, sizeof(float)*V8_SIZE_SQ);
    memset(&data_V9, 0, sizeof(float)*V9_SIZE_SQ);
    memset(&data_L9, NO_WATER_HEIGHT, sizeof(float)*V9_SIZE_SQ);
    for(uint8 x = 0; x < 16; x++)
        for(uint8 y = 0; y < 16; y++)
            m_liquidChunk[x][y] = NULL;
}

MapCompressedChunkDataLoader::~MapCompressedChunkDataLoader()
{

}

bool MapCompressedChunkDataLoader::LoadMapInforation(uint32 tileX, uint32 tileY, FILE *input, uint32 compressionType)
{    // Read tile information in bulk
    ASSERT(compressionType == MAP_COMPRESSED_CHUNKS);

    for(uint8 x = 0; x < 16; x++)
    {
        for(uint8 y = 0; y < 16; y++)
        {
            uint8 chunk_idx, chunk_idy;
            float mapHeight, floatV8[8][8], floatV9[9][9];
            fread(&chunk_idx, sizeof(uint8), 1, input);
            fread(&chunk_idy, sizeof(uint8), 1, input);
            fread(&areaInfo[x][y], sizeof(uint16), 1, input);
            fread(&mapHeight, sizeof(float), 1, input);
            for(uint8 cx = 0; cx <= 8; cx++)
            {
                for(uint8 cy = 0; cy <= 8; cy++)
                {
                    floatV9[cx][cy] = mapHeight;
                    if(cx == 8 || cy == 8)
                        continue;
                    floatV8[cx][cy] = mapHeight;
                }
            }

            float mult;
            uint8 compFlags;
            fread(&compFlags, sizeof(uint8), 1, input);
            switch(compFlags)
            {
            case 0x04:
                uint8 uint8_V8[8*8], uint8_V9[9*9];
                fread(&uint8_V8, sizeof(uint8)*8*8, 1, input);
                fread(&uint8_V9, sizeof(uint8)*9*9, 1, input);
                fread(&mult, sizeof(float), 1, input);
                for(uint8 cx = 0; cx <= 8; cx++)
                {
                    for(uint8 cy = 0; cy <= 8; cy++)
                    {
                        floatV9[cx][cy] += mult*float(uint8_V9[cx*9+cy]);
                        if(cx == 8 || cy == 8)
                            continue;
                        floatV8[cx][cy] += mult*float(uint8_V8[cx*8+cy]);
                    }
                }
                break;
            case 0x02:
                uint16 uint16V8[8*8], uint16V9[9*9];
                fread(&uint16V8, sizeof(uint16)*8*8, 1, input);
                fread(&uint16V9, sizeof(uint16)*9*9, 1, input);
                fread(&mult, sizeof(float), 1, input);
                for(uint8 cx = 0; cx <= 8; cx++)
                {
                    for(uint8 cy = 0; cy <= 8; cy++)
                    {
                        floatV9[cx][cy] += mult*float(uint16V9[cx*9+cy]);
                        if(cx == 8 || cy == 8)
                            continue;
                        floatV8[cx][cy] += mult*float(uint16V8[cx*8+cy]);
                    }
                }
                break;
            case 0x01:
                break; // Flat land
            default:
                float V8[8*8], V9[9*9];
                fread(&V8, sizeof(float)*8*8, 1, input);
                fread(&V9, sizeof(float)*9*9, 1, input);
                for(uint8 cx = 0; cx <= 8; cx++)
                {
                    for(uint8 cy = 0; cy <= 8; cy++)
                    {
                        floatV9[cx][cy] += V9[cx*9+cy];
                        if(cx == 8 || cy == 8)
                            continue;
                        floatV8[cx][cy] += V8[cx*8+cy];
                    }
                }
                break;
            }

            // Splice together chunks into our tile storage
            for(uint8 cx = 0; cx <= 8; cx++)
            {
                int i = x*8 + cx;
                for(uint8 cy = 0; cy <= 8; cy++)
                {
                    int j = y*8 + cy;
                    data_V9[i][j] = floatV9[cx][cy];

                    if(cx == 8 || cy == 8)
                        continue;
                    data_V8[i][j] = floatV8[cx][cy];
                }
            }

            // We don't use holes yet
            uint32 holes;
            fread(&holes, sizeof(uint32), 1, input);

            // Liquid reading
            uint16 liquidType;
            fread(&liquidType, sizeof(uint16), 1, input);
            fread(&compFlags, sizeof(uint8), 1, input);
            if(compFlags != 0xFF) // 0xFF is dry land
            {
                float liqMult, minHeight;
                fread(&minHeight, sizeof(float), 1, input);
                m_liquidChunk[x][y] = new LiquidChunk();
                m_liquidChunk[x][y]->type = liquidType;
                m_liquidChunk[x][y]->minHeight = minHeight;

                switch(compFlags)
                {
                case 0x04:
                    uint8 uint8L9[9*9];
                    fread(&uint8L9, sizeof(uint8)*9*9, 1, input);
                    fread(&liqMult, sizeof(float), 1, input);
                    for(uint8 cx = 0; cx <= 8; cx++)
                    {
                        int i = x*8 + cx;
                        for(uint8 cy = 0; cy <= 8; cy++)
                        {
                            int j = y*8 + cy;
                            if(uint8L9[cx*9+cy] != 0xFF)
                                data_L9[i][j] = (liqMult*float(uint8L9[cx*9+cy]));
                        }
                    }
                    break;
                case 0x02:
                    uint16 uint16L9[9*9];
                    fread(&uint16L9, sizeof(uint16)*9*9, 1, input);
                    fread(&liqMult, sizeof(float), 1, input);
                    for(uint8 cx = 0; cx <= 8; cx++)
                    {
                        int i = x*8 + cx;
                        for(uint8 cy = 0; cy <= 8; cy++)
                        {
                            int j = y*8 + cy;
                            if(uint16L9[cx*9+cy] != 0xFFFF)
                                data_L9[i][j] = (liqMult*float(uint16L9[cx*9+cy]));
                        }
                    }
                    break;
                case 0x01:
                    uint64 liquid_mask;
                    fread(&liquid_mask, sizeof(uint64), 1, input);
                    bool liquid_exists[9][9];
                    memset(&liquid_exists, 0, sizeof(bool)*9*9);
                    for(uint8 i = 0; i < 8; i++)
                    {
                        for(uint8 j = 0; j < 8; j++)
                        {
                            if(liquid_mask & 0x01)
                            {
                                liquid_exists[i][j] = true;
                                liquid_exists[i][j+1] = true;
                                liquid_exists[(i+1)][j] = true;
                                liquid_exists[(i+1)][j+1] = true;
                            }

                            liquid_mask>>=1;
                        }
                    }

                    for(uint8 cx = 0; cx <= 8; cx++)
                    {
                        int i = x*8 +cx;
                        for(uint8 cy = 0; cy <= 8; cy++)
                        {
                            int j = y*8 +cy;
                            data_L9[i][j] = liquid_exists[cx][cy] ? 0.f : NO_WATER_HEIGHT;
                        }
                    }
                    break; // Flat water
                default:
                    float L9[9*9];
                    fread(&L9, sizeof(float)*9*9, 1, input);
                    for(uint8 cx = 0; cx <= 8; cx++)
                    {
                        int i = x*8 +cx;
                        for(uint8 cy = 0; cy <= 8; cy++)
                        {
                            int j = y*8 +cy;
                            data_L9[i][j] = L9[cx*9+cy];
                        }
                    }
                    break;
                }
            }
        }
    }

    return MapDataLoader::LoadMapInforation(tileX, tileY, input, compressionType);
}

uint16 MapCompressedChunkDataLoader::GetAreaId(float x, float y)
{
    int lx = (int)(16 * (32 - x / TERRAIN_TILE_SIZE)) & 15;
    int ly = (int)(16 * (32 - y / TERRAIN_TILE_SIZE)) & 15;
    return areaInfo[lx][ly];
}

float MapCompressedChunkDataLoader::GetTerrainHeight(float x, float y)
{
    x = V8_SIZE * (32 - x / TERRAIN_TILE_SIZE);
    y = V8_SIZE * (32 - y / TERRAIN_TILE_SIZE);
    int tx = (int)x; x -= tx; tx &= V8_SIZE-1;
    int ty = (int)y; y -= ty; ty &= V8_SIZE-1;

    float a, b, c;
    if (x + y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            float h1 = data_V9[tx][ty];
            float h2 = data_V9[tx+1][ty];
            float h5 = 2 * data_V8[tx][ty];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            float h1 = data_V9[tx][ty];
            float h3 = data_V9[tx][ty + 1];
            float h5 = 2 * data_V8[tx][ty];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            float h2 = data_V9[(tx + 1)][ty];
            float h4 = data_V9[(tx + 1) ][ty + 1];
            float h5 = 2 * data_V8[tx][ty];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            float h3 = data_V9[(tx) ][ ty + 1];
            float h4 = data_V9[(tx + 1)][ ty + 1];
            float h5 = 2 * data_V8[tx][ty];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }

    return a * x + b * y + c;
}

uint16 MapCompressedChunkDataLoader::GetWaterType(float x, float y)
{
    int lx = (int)(CHUNK_SIZE * (32 - x / TERRAIN_TILE_SIZE)) & (CHUNK_SIZE-1);
    int ly = (int)(CHUNK_SIZE * (32 - y / TERRAIN_TILE_SIZE)) & (CHUNK_SIZE-1);
    LiquidChunk *liqChunk = NULL;
    if((liqChunk = m_liquidChunk[lx][ly]) == NULL)
        return 0;

    return liqChunk->type;
}

float MapCompressedChunkDataLoader::GetWaterHeight(float x, float y)
{
    int lx = (int)(CHUNK_SIZE * (32 - x / TERRAIN_TILE_SIZE)) & (CHUNK_SIZE-1);
    int ly = (int)(CHUNK_SIZE * (32 - y / TERRAIN_TILE_SIZE)) & (CHUNK_SIZE-1);
    LiquidChunk *liqChunk = NULL;
    if((liqChunk = m_liquidChunk[lx][ly]) == NULL)
        return NO_WATER_HEIGHT;

    int clx = (int)(128 * (32 - x / TERRAIN_TILE_SIZE)) & 127;
    int cly = (int)(128 * (32 - y / TERRAIN_TILE_SIZE)) & 127;
    if(data_L9[clx][cly] == NO_WATER_HEIGHT)
        return NO_WATER_HEIGHT;

    // We have our minimum height for this liquid chunk
    return liqChunk->minHeight + data_L9[clx][cly];
}

bool MapCompressedChunkDataLoader::CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &areaId)
{
    bool Result = false;
    for(uint32 xc = (CellX%CellsPerTile)*16/CellsPerTile;xc<(CellX%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;xc++)
    {
        for(uint32 yc = (CellY%CellsPerTile)*16/CellsPerTile;yc<(CellY%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;yc++)
        {
            if(uint16 areaId = areaInfo[xc][yc])
            {
                areaId = areaId;
                Result = true;
                break;
            }
        }
    }

    return Result;
}
