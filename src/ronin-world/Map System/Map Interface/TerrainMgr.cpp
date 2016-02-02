/***
 * Demonstrike Core
 */

#include "StdAfx.h"

TerrainMgr::TerrainMgr(std::string MapPath, uint32 MapId) : file_name(MapPath), dummyMap(false), mapId(MapId)
{
    file_name.append('/'+format("Map_%03u.bin", mapId));

    for(uint8 i = 0; i < 64; i++)
        for(uint8 i2 = 0; i2 < 64; i2++)
            LoadCounter[i][i2] = 0;
    sVMapInterface.ActivateMap(mapId, false);
}

TerrainMgr::~TerrainMgr()
{
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
            T h1 = V9[tx*129+ty];
            T h2 = V9[(tx+1)*129+ty];
            T h5 = 2 * V8[tx*128+ty];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            T h1 = V9[tx*129+ty];
            T h3 = V9[tx*129+(ty+1)];
            T h5 = 2 * V8[tx*128+ty];
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
            T h2 = V9[(tx+1)*129+ty];
            T h4 = V9[(tx+1)*129+(ty+1)];
            T h5 = 2 * V8[tx*128+ty];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            T h3 = V9[tx*129+(ty+1)];
            T h4 = V9[(tx+1)*129+(ty+1)];
            T h5 = 2 * V8[tx*128+ty];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
}

float getHeightF(float x, float y, int x_int, int y_int, TileTerrainInformation* Tile)
{
    float a, b, c;
    getRawHeight<float, float>(x, y, x_int, y_int, a, b, c, Tile->float_V8->V8, Tile->float_V9->V9);
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
    float retVal = NO_LAND_HEIGHT;
    if(Tile)
    {
        retVal = Tile->mapHeight[0];
        x = 128 * (32 - x / 533.3333333f);
        y = 128 * (32 - y / 533.3333333f);
        int x_int = (int)x; x -= x_int; x_int &= (128 - 1);
        int y_int = (int)y; y -= y_int; y_int &= (128 - 1);

        if(Tile->byte_V8)
            retVal += getHeight8(x, y, x_int, y_int, Tile->byte_V8->V8, Tile->byte_V9->V9, Tile->heightMultipier);
        else if(Tile->short_V8)
            retVal += getHeight16(x, y, x_int, y_int, Tile->short_V8->V8, Tile->short_V9->V9, Tile->heightMultipier);
        else if(Tile->float_V8)
            retVal = getHeightF(x, y, x_int, y_int, Tile);
    }
    return retVal;

}

float GetLiquidHeight(float x, float y, TileTerrainInformation* Tile)
{
    if(Tile == NULL)
        return NO_WATER_HEIGHT;
    else if(Tile->m_liquidHeight == NULL)
        return Tile->mapHeight[2];

    x = 128 * (32 - x / 533.3333333f);
    y = 128 * (32 - y / 533.3333333f);
    int cx_int = ((int)x & (128 - 1)) - Tile->liquidData[0];
    int cy_int = ((int)y & (128 - 1)) - Tile->liquidData[1];
    if (cx_int < 0 || cx_int >= Tile->liquidData[2])
        return NO_WATER_HEIGHT;
    if (cy_int < 0 || cy_int >= Tile->liquidData[3])
        return NO_WATER_HEIGHT;
    return Tile->m_liquidHeight[cx_int * Tile->liquidData[2] + cy_int];
}

uint8 GetLiquidType(float x, float y, TileTerrainInformation* Tile)
{
    uint16 retVal = 0;
    if(Tile && Tile->byte_LI)
    {
        x = 16 * (32 - x / 533.3333333f);
        y = 16 * (32 - y / 533.3333333f);
        int lx = (int)x & 15;
        int ly = (int)y & 15;
        retVal = Tile->byte_LI->LI[lx*16+ly];
    } else if(Tile) retVal = Tile->liquidData[4];
    return retVal;
}

uint16 GetAreaEntry(float x, float y, TileTerrainInformation* Tile)
{
    uint16 retVal = 0xFFFF;
    if(Tile && Tile->short_AI)
    {
        x = 16 * (32 - x / 533.3333333f);
        y = 16 * (32 - y / 533.3333333f);
        int lx = (int)x & 15;
        int ly = (int)y & 15;
        retVal = Tile->short_AI->AI[lx*16+ly];
    } else if(Tile) retVal = Tile->areaInfo;
    return retVal;
}

bool TerrainMgr::LoadTerrainHeader()
{
    // Create the 
    FILE *FileDescriptor = fopen(file_name.c_str(), "rb");
    if(FileDescriptor == 0)
    {
        sLog.Error("TerrainMgr", "Map load failed for %s. Missing file?", file_name.c_str());
        dummyMap = true;
        return false;
    }

    /* check file size */
    fseek(FileDescriptor, 0, SEEK_END);
    if(ftell(FileDescriptor) == TERRAIN_HEADER_SIZE)
    {
        sLog.outDebug("Map file %s Ignored.", file_name.c_str());

        /* file with no data */
        fclose(FileDescriptor);
        dummyMap = true;
        return false;
    }

    uint32 TileOffsets[64][64];
    // Read in the header.
    fseek(FileDescriptor, 0, SEEK_SET);
    size_t dread = fread(TileOffsets, 1, TERRAIN_HEADER_SIZE, FileDescriptor);
    fclose(FileDescriptor);

    if(dread != TERRAIN_HEADER_SIZE)
    {
        sLog.Error("TerrainMgr", "Terrain header read failed for %s! %u/%u | %u", file_name.c_str(), dread, TERRAIN_HEADER_SIZE, sizeof(TileOffsets));
        dummyMap = true;
        return false;
    }

    for(uint8 x = 0; x < 64; ++x)
    {
        for(uint8 y = 0; y < 64; ++y)
        {
            if(TileOffsets[x][y] == 0)
                continue;
            m_tileOffsets.insert(std::make_pair(std::make_pair(x, y), TileOffsets[x][y]));
        }
    }

    return true;
}

bool TerrainMgr::LoadTileInformation(uint32 x, uint32 y, FILE *input)
{
    if(dummyMap == true)
        return false;

    bool opened = false;
    if(input == NULL)
    {
        fopen_s(&input, file_name.c_str(), "rb");
        if(input == NULL)
            return false;
        opened = true;
    }

    std::pair<uint8, uint8> offsetPair = std::make_pair(x, y);
    if(m_tileOffsets.find(offsetPair) == m_tileOffsets.end())
    {
        if(opened) fclose(input);
        return false;
    }

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
                memset(tile, 0, sizeof(TileTerrainInformation));

                uint8 flags[3];
                fread(&flags, sizeof(uint8), 3, input);
                if((flags[0] & 0x01) == 0)
                {
                    if(flags[0] & 0x02)
                    {
                        tile->masked_AI = new TileTerrainInformation::mAI;
                        fread(&tile->masked_AI->mask, sizeof(uint16), 16, input);
                        fread(&tile->areaInfo, sizeof(uint16), 1, input);
                    }
                    else
                    {
                        tile->short_AI = new TileTerrainInformation::sAI;
                        fread(&tile->short_AI->AI, sizeof(uint16), 16*16, input);
                    }
                } else fread(&tile->areaInfo, sizeof(uint16), 1, input);

                fread(&tile->mapHeight, sizeof(float), 2, input);
                if((flags[1] & 0x01) == 0)
                {
                    if(flags[1] & 0x04)
                    {
                        tile->byte_V8 = new TileTerrainInformation::bV8;
                        tile->byte_V9 = new TileTerrainInformation::bV9;
                        fread(&tile->byte_V8->V8, sizeof(uint8), 128*128, input);
                        fread(&tile->byte_V9->V9, sizeof(uint8), 129*129, input);
                    }
                    else if(flags[1] & 0x02)
                    {
                        tile->short_V8 = new TileTerrainInformation::sV8;
                        tile->short_V9 = new TileTerrainInformation::sV9;
                        fread(&tile->short_V8->V8, sizeof(uint16), 128*128, input);
                        fread(&tile->short_V9->V9, sizeof(uint16), 129*129, input);
                    }
                    else
                    {
                        tile->float_V8 = new TileTerrainInformation::fV8;
                        tile->float_V9 = new TileTerrainInformation::fV9;
                        fread(&tile->float_V8->V8, sizeof(float), 128*128, input);
                        fread(&tile->float_V9->V9, sizeof(float), 129*129, input);
                    }
                }

                if((flags[2] & 0x01) == 0)
                {
                    tile->short_LE = new TileTerrainInformation::sLE;
                    tile->byte_LI = new TileTerrainInformation::bLI;
                    fread(&tile->short_LE->LE, sizeof(uint16), 16*16, input);
                    fread(&tile->byte_LI->LI, sizeof(uint8), 16*16, input);
                } else fread(&tile->liquidData[4], sizeof(uint8), 1, input);

                if((flags[2] & 0x02) == 0)
                {
                    fread(&tile->liquidData, sizeof(uint8), 4, input);
                    tile->m_liquidHeight = new float [tile->liquidData[2] * tile->liquidData[3]];
                    fread(tile->m_liquidHeight, sizeof(float), tile->liquidData[2] * tile->liquidData[3], input);
                } else fread(&tile->mapHeight[2], sizeof(float), 1, input);

                if(flags[0] & 0x02)
                {
                    tile->short_HI = new TileTerrainInformation::sHI;
                    fread(&tile->short_HI->HI, sizeof(uint16), 16*16, input);
                }
            }
        }
    }

    // Close any file opened from inside this function
    if(opened) fclose(input);
    // If we don't equal 0, it means the load was successful.
    return _TileInformationLoaded(x, y);
}

void TerrainMgr::UnloadTileInformation(uint32 x, uint32 y)
{
    mutex.Acquire();

    std::pair<uint8, uint8> tilePair = std::make_pair(x, y);
    if(tileInformation.find(tilePair) != tileInformation.end())
        tileInformation.erase(tilePair);
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
        else if(z != 0.0f && z != NO_WATER_HEIGHT)
        {
            if(z < GetHeight(x, y, GetTileInformation(TileX, TileY)))
                WaterHeight = NO_WATER_HEIGHT;
        }
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
    uint32 TileX = CellX/8;
    uint32 TileY = CellY/8;

    mutex.Acquire();
    uint32 areaid = 0;
    bool Required, Result = false;
    if((Required = !_TileInformationLoaded(TileX, TileY)))
    {
        if(!LoadTileInformation(TileX, TileY))
        {
            mutex.Release();
            return Result;
        }
    }

    TileTerrainInformation *tile = GetTileInformation(TileX, TileY);
    for(uint32 xc = (CellX%CellsPerTile)*16/CellsPerTile;xc<(CellX%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;xc++)
    {
        for(uint32 yc = (CellY%CellsPerTile)*16/CellsPerTile;yc<(CellY%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;yc++)
        {
            if(tile->short_AI) areaid = tile->short_AI->AI[yc*16+xc];
            else areaid = tile->areaInfo;
            if(areaid)
            {
                AreaID = areaid;
                Result = true;
                break;
            }
        }
    }

    if(Required)
        UnloadTileInformation(TileX, TileY);
    mutex.Release();
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
    if((++LoadCounter[tileX][tileY]) == 1)
        LoadTileInformation(tileX, tileY);
    sVMapInterface.ActivateTile(mapId, tileX, tileY);
    sNavMeshInterface.LoadNavMesh(mapId, tileX, tileY);
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
    sLog.Debug("TerrainMgr", "[%u]: Loading all terrain", mapId);
    mutex.Acquire();
    FILE *input = NULL;
    fopen_s(&input, file_name.c_str(), "rb");
    for(uint8 x = 0; x < 64; x++)
    {
        for(uint8 y = 0; y < 64; y++)
        {
            sVMapInterface.ActivateTile(mapId, x, y);
            sNavMeshInterface.LoadNavMesh(mapId, x, y);
            if(input == NULL)
                continue;

            LoadTileInformation(x, y, input);
            LoadCounter[x][y]++;
        }
    }
    fclose(input);
    mutex.Release();
    sLog.Debug("TerrainMgr", "[%u]: All terrain loaded", mapId);
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
