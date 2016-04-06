/***
 * Demonstrike Core
 */

#include "StdAfx.h"

TerrainMgr::TerrainMgr(std::string MapPath, uint32 MapId) : file_name(MapPath), dummyMap(false), mapId(MapId)
{
    file_name.append('/'+format("%03u.tiletree", mapId));

    for(uint8 i = 0; i < 64; i++)
        for(uint8 i2 = 0; i2 < 64; i2++)
            LoadCounter[i][i2] = 0;
}

TerrainMgr::~TerrainMgr()
{
    while(!tileInformation.empty())
    {
        (*tileInformation.begin()).second.cleanup();
        tileInformation.erase(tileInformation.begin());
    }
    tileInformation.clear();
    sVMapInterface.DeactivateMap(mapId);
}

template<typename T, typename T2> void getRawHeight(float x, float y, int tx, int ty, T &a, T &b, T &c, T2 *V8, T2 *V9)
{
    if (x + y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            T h1 = V9[tx*9+ty];
            T h2 = V9[(tx+1)*9+ty];
            T h5 = 2 * V8[tx*8+ty];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            T h1 = V9[tx*9+ty];
            T h3 = V9[tx*9+(ty+1)];
            T h5 = 2 * V8[tx*8+ty];
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
            T h2 = V9[(tx+1)*9+ty];
            T h4 = V9[(tx+1)*9+(ty+1)];
            T h5 = 2 * V8[tx*8+ty];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            T h3 = V9[tx*9+(ty+1)];
            T h4 = V9[(tx+1)*9+(ty+1)];
            T h5 = 2 * V8[tx*8+ty];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
}

float getHeightF(float x, float y, int x_int, int y_int, ChunkTerrainInfo* chunk)
{
    float a, b, c;
    getRawHeight<float, float>(x, y, x_int, y_int, a, b, c, chunk->floatTH->V8, chunk->floatTH->V9);
    // Calculate height
    return a * x + b * y + c;
}

float getHeight16(float x, float y, int x_int, int y_int, uint16 *V8, uint16 *V9, float multiplier)
{
    int32 a, b, c; // Grab the raw height from the triangles
    getRawHeight<int32, uint16>(x, y, x_int, y_int, a, b, c, V8, V9);
    // Calculate height
    return (float)((a * x) + (b * y) + c) * multiplier;
}

float getHeight8(float x, float y, int x_int, int y_int, uint8 *V8, uint8 *V9, float multiplier)
{
    int32 a, b, c; // Grab the raw height from the triangles
    getRawHeight<int32, uint8>(x, y, x_int, y_int, a, b, c, V8, V9);
    // Calculate height
    return (float)((a * x) + (b * y) + c) * multiplier;
}

float GetHeight(float x, float y, TileTerrainInformation* Tile)
{
    // X and Y come from previous function with distance from edge of tile
    float retVal = NO_LAND_HEIGHT;
    if(Tile)
    {
        // Convert the co-ordinates to tiles.
        uint32 TileX = TerrainMgr::ConvertGlobalXCoordinate(x);
        uint32 TileY = TerrainMgr::ConvertGlobalYCoordinate(y);

        // Convert tiles into positive values and get the distance into the tile
        static float mid = 0.5f * 64.0f * 533.33333333f;
        x = (mid-x)-TileX*TERRAIN_TILE_SIZE;
        y = (mid-y)-TileY*TERRAIN_TILE_SIZE;

        // Calculate what chunk we're currently in
        int x_int = x/TERRAIN_CHUNK_SIZE, y_int = y/TERRAIN_CHUNK_SIZE;
        // Grab the chunk pointer
        ChunkTerrainInfo *info = &Tile->_chunks[x_int][y_int];
        // Get the offset from the side of our chunk
        x -= x_int*TERRAIN_CHUNK_SIZE, y -= y_int*TERRAIN_CHUNK_SIZE;
        // Divide the left over value to see how far into our chunk we are
        x_int = x/TERRAIN_CHUNK_STEP, y_int = y/TERRAIN_CHUNK_STEP;
        // Calculate our x subdifferential
        x -= x_int*TERRAIN_CHUNK_STEP;
        x = 1.f-(x/TERRAIN_CHUNK_STEP);
        // Calculate our y subdifferential
        y -= y_int*TERRAIN_CHUNK_STEP;
        y = 1.f-(y/TERRAIN_CHUNK_STEP);

        retVal = info->mapHeight;
        if(info->byteTH)
            retVal += getHeight8(x, y, x_int, y_int, info->byteTH->V8, info->byteTH->V9, info->byteTH->Mult);
        else if(info->shortTH)
            retVal += getHeight16(x, y, x_int, y_int, info->shortTH->V8, info->shortTH->V9, info->shortTH->Mult);
        else if(info->floatTH)
            retVal += getHeightF(x, y, x_int, y_int, info);
    }
    return retVal;
}

float GetLiquidHeight(float x, float y, TileTerrainInformation* Tile)
{
    float retVal = NO_WATER_HEIGHT;
    if(Tile)
    {
        // Convert the co-ordinates to tiles.
        uint32 TileX = TerrainMgr::ConvertGlobalXCoordinate(x);
        uint32 TileY = TerrainMgr::ConvertGlobalYCoordinate(y);

        // Convert tiles into positive values and get the distance into the tile
        static float mid = 0.5f * 64.0f * 533.33333333f;
        x = (mid-x)-TileX*TERRAIN_TILE_SIZE;
        y = (mid-y)-TileY*TERRAIN_TILE_SIZE;

        // Calculate what chunk we're currently in
        int x_int = x/TERRAIN_CHUNK_SIZE, y_int = y/TERRAIN_CHUNK_SIZE;
        // Grab the chunk pointer
        ChunkTerrainInfo *info = &Tile->_chunks[x_int][y_int];
        // Get the offset from the side of our chunk
        x -= x_int*TERRAIN_CHUNK_SIZE, y -= y_int*TERRAIN_CHUNK_SIZE;
        // Divide the left over value to see how far into our chunk we are
        x_int = x/TERRAIN_CHUNK_STEP, y_int = y/TERRAIN_CHUNK_STEP;

        retVal = info->liquidHeight;
        if(info->byteLH)
            retVal += info->byteLH->L9[x_int*9+y_int]*info->byteLH->Mult;
        else if(info->shortLH)
            retVal += info->shortLH->L9[x_int*9+y_int]*info->shortLH->Mult;
        else if(info->floatLH)
            retVal += info->floatLH->L9[x_int*9+y_int];
    }
    return retVal;
}

uint8 GetLiquidType(float x, float y, TileTerrainInformation* Tile)
{
    uint16 retVal = 0;
    if(Tile)
    {
        int lx = (int)(16 * (32 - x / TERRAIN_TILE_SIZE)) & 15;
        int ly = (int)(16 * (32 - y / TERRAIN_TILE_SIZE)) & 15;
        retVal = Tile->_chunks[lx][ly].liquidType;
    }
    return retVal;
}

uint16 GetAreaEntry(float x, float y, TileTerrainInformation* Tile)
{
    uint16 retVal = 0xFFFF;
    if(Tile)
    {
        int lx = (int)(16 * (32 - x / TERRAIN_TILE_SIZE)) & 15;
        int ly = (int)(16 * (32 - y / TERRAIN_TILE_SIZE)) & 15;
        retVal = Tile->_chunks[lx][ly].areaInfo;
    }
    return retVal;
}

bool TerrainMgr::LoadTerrainHeader()
{
    // Create the 
    FILE *FileDescriptor = fopen(file_name.c_str(), "rb");
    if(FileDescriptor == NULL)
    {
        sLog.Error("TerrainMgr", "Map load failed for %s. Missing file?", file_name.c_str());
        dummyMap = true;
        return false;
    }

    char identifier[10];
    fread(&identifier, 10, 1, FileDescriptor);
    if(strcmp(identifier, heightMapHeader))
    {   // If we have no height data but a proper vmap header, then load the vmap header
        sLog.Error("TerrainMgr", "Bad header for %s. Type %s", file_name.c_str(), identifier);
        if(strcmp(identifier, VMAP::RAW_VMAP_MAGIC) == 0)
        {
            fseek(FileDescriptor, 0, SEEK_SET);
            sVMapInterface.ActivateMap(mapId, FileDescriptor);
        }
        fclose(FileDescriptor);
        dummyMap = true;
        return false;
    }

    /* check file size */
    fseek(FileDescriptor, 0, SEEK_END);
    uint32 fileSize = ftell(FileDescriptor);
    if(fileSize == (10 + terrainHeaderSize + sizeof(uint32)))
    {
        sLog.outDebug("Map file %s Ignored.", file_name.c_str());

        /* file with no data */
        fclose(FileDescriptor);
        dummyMap = true;
        return false;
    }

    // Read in the header.
    fseek(FileDescriptor, 10, SEEK_SET);
    uint32 tileOffsets[64][64], vmapOffset = 0;
    if(fread(tileOffsets, terrainHeaderSize, 1, FileDescriptor) != 1 || fread(&vmapOffset, sizeof(uint32), 1, FileDescriptor) != 1)
    {
        sLog.Error("TerrainMgr", "Terrain header read failed for %s!", file_name.c_str());
        fclose(FileDescriptor);
        dummyMap = true;
        return false;
    }

    if(vmapOffset && fileSize != vmapOffset)
    {
        fseek(FileDescriptor, vmapOffset, SEEK_SET);
        if(!sVMapInterface.ActivateMap(mapId, FileDescriptor))
            printf("Vmap failed %03u\n", mapId);
    }

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

bool TerrainMgr::LoadTileInformation(uint32 x, uint32 y, FILE *input)
{
    if(dummyMap == true)
        return false;

    std::pair<uint8, uint8> offsetPair = std::make_pair(x, y);
    if(m_tileOffsets.find(offsetPair) == m_tileOffsets.end())
        return false;

    // Find our offset in our cached header.
    if(uint32 Offset = m_tileOffsets.at(offsetPair))
    {
        // Check that we haven't been loaded by another thread.
        if(tileInformation.find(offsetPair) == tileInformation.end())
        {
            // Seek to our specified offset.
            if(fseek(input, Offset, SEEK_SET) == 0)
            {
                // Allocate the tile information.
                TileTerrainInformation* tile = &tileInformation[offsetPair];
                memset(&tile->_chunks, 0, sizeof(ChunkTerrainInfo)*16*16);

                for(uint8 x = 0; x < 16; x++)
                {
                    for(uint8 y = 0; y < 16; y++)
                    {
                        fread(&tile->_chunks[x][y].areaInfo, sizeof(uint16), 1, input);
                        fread(&tile->_chunks[x][y].mapHeight, sizeof(float), 1, input);

                        uint8 compFlags;
                        fread(&compFlags, sizeof(uint8), 1, input);
                        switch(compFlags)
                        {
                        case 0x04:
                            tile->_chunks[x][y].byteTH = new ChunkTerrainInfo::bT;
                            fread(&tile->_chunks[x][y].byteTH->V8, sizeof(uint8)*8*8, 1, input);
                            fread(&tile->_chunks[x][y].byteTH->V9, sizeof(uint8)*9*9, 1, input);
                            fread(&tile->_chunks[x][y].byteTH->Mult, sizeof(float), 1, input);
                            break;
                        case 0x02:
                            tile->_chunks[x][y].shortTH = new ChunkTerrainInfo::sT;
                            fread(&tile->_chunks[x][y].shortTH->V8, sizeof(uint16)*8*8, 1, input);
                            fread(&tile->_chunks[x][y].shortTH->V9, sizeof(uint16)*9*9, 1, input);
                            fread(&tile->_chunks[x][y].shortTH->Mult, sizeof(float), 1, input);
                            break;
                        case 0x01: break; // Flat land
                        default:
                            tile->_chunks[x][y].floatTH = new ChunkTerrainInfo::fT;
                            fread(&tile->_chunks[x][y].floatTH->V8, sizeof(float)*8*8, 1, input);
                            fread(&tile->_chunks[x][y].floatTH->V9, sizeof(float)*9*9, 1, input);
                            break;
                        }

                        uint32 holes;
                        fread(&holes, sizeof(uint32), 1, input);

                        // Liquid reading
                        fread(&tile->_chunks[x][y].liquidType, sizeof(uint16), 1, input);
                        fread(&compFlags, sizeof(uint8), 1, input);
                        if(compFlags != 0xFF) // 0xFF is dry land
                        {
                            fread(&tile->_chunks[x][y].liquidHeight, sizeof(float), 1, input);
                            switch(compFlags)
                            {
                            case 0x04:
                                tile->_chunks[x][y].byteLH = new ChunkTerrainInfo::bL;
                                fread(&tile->_chunks[x][y].byteLH->L9, sizeof(uint8)*9*9, 1, input);
                                fread(&tile->_chunks[x][y].byteLH->Mult, sizeof(float), 1, input);
                                break;
                            case 0x02:
                                tile->_chunks[x][y].shortLH = new ChunkTerrainInfo::sL;
                                fread(&tile->_chunks[x][y].shortLH->L9, sizeof(uint16)*9*9, 1, input);
                                fread(&tile->_chunks[x][y].shortLH->Mult, sizeof(float), 1, input);
                                break;
                            case 0x01: break; // Flat water
                            default:
                                tile->_chunks[x][y].floatLH = new ChunkTerrainInfo::fL;
                                fread(&tile->_chunks[x][y].floatLH->L9, sizeof(float)*9*9, 1, input);
                                break;
                            }
                        } else tile->_chunks[x][y].liquidHeight = NO_WATER_HEIGHT;
                    }
                }
            }
        }
    }

    // If we don't equal 0, it means the load was successful.
    return _TileInformationLoaded(x, y);
}

void TerrainMgr::UnloadTileInformation(uint32 x, uint32 y)
{
    mutex.Acquire();

    std::pair<uint8, uint8> tilePair = std::make_pair(x, y);
    if(tileInformation.find(tilePair) != tileInformation.end())
    {
        tileInformation.at(tilePair).cleanup();
        tileInformation.erase(tilePair);
    }
    mutex.Release();

    sLog.Debug("TerrainMgr","Unloaded tile information for tile [%u][%u]", x, y);
    return;
}

uint8 TerrainMgr::GetWaterType(float x, float y)
{
    if(!AreCoordinatesValid(x, y))
        return 0;

    // Convert the co-ordinates to tiles.
    uint32 TileX = ConvertGlobalXCoordinate(x);
    uint32 TileY = ConvertGlobalYCoordinate(y);

    mutex.Acquire();
    uint8 Liquid = 0;
    if(_TileInformationLoaded(TileX, TileY))
        Liquid = GetLiquidType(x, y, GetTileInformation(TileX, TileY));
    mutex.Release();
    return Liquid;
}

float TerrainMgr::GetWaterHeight(float x, float y, float z)
{
    if(!AreCoordinatesValid(x, y))
        return NO_WATER_HEIGHT;

    // Convert the co-ordinates to tiles.
    uint32 TileX = ConvertGlobalXCoordinate(x);
    uint32 TileY = ConvertGlobalYCoordinate(y);

    mutex.Acquire();
    float WaterHeight = NO_WATER_HEIGHT;
    if(_TileInformationLoaded(TileX, TileY))
    {
        WaterHeight = GetLiquidHeight(x, y, GetTileInformation(TileX, TileY));
        if(WaterHeight == 0.0f && !(GetLiquidType(x, y, GetTileInformation(TileX, TileY)) & 0x02))
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

uint16 TerrainMgr::GetAreaID(float x, float y, float z)
{
    if(!AreCoordinatesValid(x, y))
        return 0xFFFF;

    // Convert the co-ordinates to tiles.
    uint32 TileX = ConvertGlobalXCoordinate(x);
    uint32 TileY = ConvertGlobalYCoordinate(y);

    mutex.Acquire();
    uint16 areaId = 0xFFFF;
    if(_TileInformationLoaded(TileX, TileY)) // Find the offset in the 2d array.
        areaId = GetAreaEntry(x, y, GetTileInformation(TileX, TileY));
    mutex.Release();
    return areaId;
}

bool TerrainMgr::CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &AreaID)
{
    bool Result = false;
    CellGoneActive(CellX, CellY);

    mutex.Acquire();
    uint32 TileX = CellX/8, TileY = CellY/8;
    if(TileTerrainInformation *tile = GetTileInformation(TileX, TileY))
    {
        for(uint32 xc = (CellX%CellsPerTile)*16/CellsPerTile;xc<(CellX%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;xc++)
        {
            for(uint32 yc = (CellY%CellsPerTile)*16/CellsPerTile;yc<(CellY%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;yc++)
            {
                if(uint16 areaId = tile->_chunks[xc][yc].areaInfo)
                {
                    AreaID = areaId;
                    Result = true;
                    break;
                }
            }
        }
    }

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
    if(_TileInformationLoaded(TileX, TileY))
        LandHeight = GetHeight(x, y, GetTileInformation(TileX, TileY));
    mutex.Release();
    return LandHeight;
}

void TerrainMgr::CellGoneActive(uint32 x, uint32 y)
{
    uint32 tileX = x/8, tileY = y/8;
    mutex.Acquire();
    FILE *input = NULL;
    fopen_s(&input, file_name.c_str(), "rb");
    if(input)
    {
        if((++LoadCounter[tileX][tileY]) == 1)
            LoadTileInformation(tileX, tileY, input);
        sVMapInterface.ActivateTile(mapId, tileX, tileY, input);
        sNavMeshInterface.LoadNavMesh(mapId, tileX, tileY);
        fclose(input);
    }
    mutex.Release();
}

void TerrainMgr::CellGoneIdle(uint32 x, uint32 y)
{
    uint32 tileX = x/8, tileY = y/8;
    mutex.Acquire();
    if((--LoadCounter[tileX][tileY]) == 0)
        UnloadTileInformation(tileX, tileY);
    sVMapInterface.DeactivateTile(mapId, tileX, tileY);
    sNavMeshInterface.UnloadNavMesh(mapId, tileX, tileY);
    mutex.Release();
}

void TerrainMgr::LoadAllTerrain()
{
    mutex.Acquire();
    FILE *input = NULL;
    fopen_s(&input, file_name.c_str(), "rb");
    if(input)
    {
        sLog.Debug("TerrainMgr", "[%u]: Loading all terrain", mapId);
        for(uint8 x = 0; x < 64; x++)
        {
            for(uint8 y = 0; y < 64; y++)
            {
                LoadCounter[x][y]++;
                LoadTileInformation(x, y, input);
                sVMapInterface.ActivateTile(mapId, x, y, input);
                sNavMeshInterface.LoadNavMesh(mapId, x, y);
            }
        }
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
