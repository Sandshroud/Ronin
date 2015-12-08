/***
 * Demonstrike Core
 */

#include "StdAfx.h"

TerrainMgr::TerrainMgr(std::string MapPath, uint32 MapId, bool Instanced, bool collisionMap) : mapPath(MapPath), mapId(MapId), Instance(Instanced), m_CollisionMap(collisionMap)
{
    TileCountX = TileCountY = 0;
    TileStartX = TileEndX = 0;
    TileStartY = TileEndY = 0;
    FileDescriptor = NULL;
    TileInformation = NULL;
    for(uint8 x = 0; x < 64; x++)
        for(uint8 y = 0; y < 64; y++)
            LoadCounter[x][y] = 0;
}

TerrainMgr::~TerrainMgr()
{
    if(FileDescriptor)
    {
        // Free up our file pointer.
        mutex.Acquire();
        fclose(FileDescriptor);
        mutex.Release();
        FileDescriptor = NULL;
    }

    // Big memory cleanup, whee.
    if(TileInformation)
    {
        for(uint32 x = 0; x < TileCountX; ++x)
        {
            for(uint32 y = 0; y < TileCountY; ++y)
            {
                if(TileInformation[x][y] != 0)
                    delete TileInformation[x][y];
                TileInformation[x][y] = 0;
            }
            delete [] TileInformation[x];
            TileInformation[x] = 0;
        }
        delete [] TileInformation;
        TileInformation = NULL;
    }
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
    // Create the path
    std::string file_name = mapPath+'/';
    file_name.append(format("Map_%03u.bin", mapId));
    FileDescriptor = fopen(file_name.c_str(), "rb");
    if(FileDescriptor == 0)
    {
        sLog.Error("TerrainMgr", "Map load failed for %s. Missing file?", file_name.c_str());
        return false;
    }

    /* check file size */
    fseek(FileDescriptor, 0, SEEK_END);
    if(ftell(FileDescriptor) == TERRAIN_HEADER_SIZE)
    {
        sLog.outDebug("Map file %s Ignored.", file_name.c_str());

        /* file with no data */
        fclose(FileDescriptor);
        FileDescriptor = NULL;
        return false;
    }

    // Read in the header.
    fseek(FileDescriptor, 0, SEEK_SET);
    size_t dread = fread(TileOffsets, 1, TERRAIN_HEADER_SIZE, FileDescriptor);
    if(dread != TERRAIN_HEADER_SIZE)
    {
        sLog.Error("TerrainMgr", "Terrain header read failed for %s! %u/%u | %u", file_name.c_str(), dread, TERRAIN_HEADER_SIZE, sizeof(TileOffsets));
        fclose(FileDescriptor);
        FileDescriptor = NULL;
        return false;
    }

    for(uint32 x = 0; x < 64; ++x)
    {
        for(uint32 y = 0; y < 64; ++y)
        {
            if(TileOffsets[x][y])
            {
                if(!TileStartX || TileStartX > x)
                    TileStartX = x;
                if(!TileStartY || TileStartY > y)
                    TileStartY = y;
                if(x > TileEndX)
                    TileEndX = x;
                if(y > TileEndY)
                    TileEndY = y;
            }
        }
    }
    TileCountX = (TileEndX-TileStartX)+1;
    TileCountY = (TileEndY-TileStartY)+1;

    // Allocate both storage arrays.
    TileInformation = new TileTerrainInformation**[TileCountX];
    for(uint32 x = 0; x < TileCountX; ++x)
    {
        TileInformation[x] = new TileTerrainInformation*[TileCountY];
        for(uint32 y = 0; y < TileCountY; ++y)
        {
            // Clear the pointer.
            TileInformation[x][y] = 0;
        }
    }
    return true;
}

bool TerrainMgr::LoadTileInformation(uint32 x, uint32 y)
{
    if(!FileDescriptor)
        return false;

    uint32 offsX = x-TileStartX, offsY = y-TileStartY;
    // Make sure that we're not already loaded.
    assert(TileInformation[offsX][offsY] == 0);

    // Find our offset in our cached header.
    uint32 Offset = TileOffsets[x][y];

    // If our offset = 0, it means we don't have tile information for
    // these coords.
    if(Offset == 0)
        return false;

    // Lock the mutex to prevent double reading.
    mutex.Acquire();

    // Check that we haven't been loaded by another thread.
    if(TileInformation[offsX][offsY] != 0)
    {
        mutex.Release();
        return true;
    }

    // Seek to our specified offset.
    if(fseek(FileDescriptor, Offset, SEEK_SET) == 0)
    {
        // Allocate the tile information.
        TileTerrainInformation* tile = TileInformation[offsX][offsY] = new TileTerrainInformation();
        memset(tile, 0, sizeof(TileTerrainInformation));

        uint8 flags[3];
        fread(&flags, sizeof(uint8), 3, FileDescriptor);
        if((flags[0] & 0x01) == 0)
        {
            if(flags[0] & 0x02)
            {
                tile->masked_AI = new TileTerrainInformation::mAI;
                fread(&tile->masked_AI->mask, sizeof(uint16), 16, FileDescriptor);
                fread(&tile->areaInfo, sizeof(uint16), 1, FileDescriptor);
            }
            else
            {
                tile->short_AI = new TileTerrainInformation::sAI;
                fread(&tile->short_AI->AI, sizeof(uint16), 16*16, FileDescriptor);
            }
        } else fread(&tile->areaInfo, sizeof(uint16), 1, FileDescriptor);

        fread(&tile->mapHeight, sizeof(float), 2, FileDescriptor);
        if((flags[1] & 0x01) == 0)
        {
            if(flags[1] & 0x04)
            {
                tile->byte_V8 = new TileTerrainInformation::bV8;
                tile->byte_V9 = new TileTerrainInformation::bV9;
                fread(&tile->byte_V8->V8, sizeof(uint8), 128*128, FileDescriptor);
                fread(&tile->byte_V9->V9, sizeof(uint8), 129*129, FileDescriptor);
            }
            else if(flags[1] & 0x02)
            {
                tile->short_V8 = new TileTerrainInformation::sV8;
                tile->short_V9 = new TileTerrainInformation::sV9;
                fread(&tile->short_V8->V8, sizeof(uint16), 128*128, FileDescriptor);
                fread(&tile->short_V9->V9, sizeof(uint16), 129*129, FileDescriptor);
            }
            else
            {
                tile->float_V8 = new TileTerrainInformation::fV8;
                tile->float_V9 = new TileTerrainInformation::fV9;
                fread(&tile->float_V8->V8, sizeof(float), 128*128, FileDescriptor);
                fread(&tile->float_V9->V9, sizeof(float), 129*129, FileDescriptor);
            }
        }

        if((flags[2] & 0x01) == 0)
        {
            tile->short_LE = new TileTerrainInformation::sLE;
            tile->byte_LI = new TileTerrainInformation::bLI;
            fread(&tile->short_LE->LE, sizeof(uint16), 16*16, FileDescriptor);
            fread(&tile->byte_LI->LI, sizeof(uint8), 16*16, FileDescriptor);
        } else fread(&tile->liquidData[4], sizeof(uint8), 1, FileDescriptor);

        if((flags[2] & 0x02) == 0)
        {
            fread(&tile->liquidData, sizeof(uint8), 4, FileDescriptor);
            tile->m_liquidHeight = new float [tile->liquidData[2] * tile->liquidData[3]];
            fread(tile->m_liquidHeight, sizeof(float), tile->liquidData[2] * tile->liquidData[3], FileDescriptor);
        } else fread(&tile->mapHeight[2], sizeof(float), 1, FileDescriptor);

        if(flags[0] & 0x02)
        {
            tile->short_HI = new TileTerrainInformation::sHI;
            fread(&tile->short_HI->HI, sizeof(uint16), 16*16, FileDescriptor);
        }
    }
    // Release the mutex.
    mutex.Release();

    // If we don't equal 0, it means the load was successful.
    if(TileInformation[offsX][offsY] != 0)
        return true;
    return false;
}

void TerrainMgr::UnloadTileInformation(uint32 x, uint32 y)
{
    mutex.Acquire();

    uint32 offsX = x-TileStartX, offsY = y-TileStartY;
    // Find our information pointer.
    TileTerrainInformation *ptr = TileInformation[offsX][offsY];
    if(ptr == NULL)
        return;

    // Set the spot to unloaded (null).
    TileInformation[offsX][offsY] = 0;

    // Free the memory.
    delete ptr;
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

    if(!AreTilesValid(TileX, TileY))
        return 0;

    uint32 OffsetTileX = TileX-TileStartX;
    uint32 OffsetTileY = TileY-TileStartY;

    mutex.Acquire();
    if(!TileInformationLoaded(OffsetTileX, OffsetTileY))
    {
        mutex.Release();
        return 0;
    }

    // Find the offset.
    uint8 Liquid = GetLiquidType(x, y, GetTileInformation(OffsetTileX, OffsetTileY));

    // Return our cached information.
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

    if(!AreTilesValid(TileX, TileY))
        return false;

    uint32 OffsetTileX = TileX-TileStartX;
    uint32 OffsetTileY = TileY-TileStartY;

    mutex.Acquire();
    if(!TileInformationLoaded(OffsetTileX, OffsetTileY))
    {
        mutex.Release();
        return NO_WATER_HEIGHT;
    }

    float WaterHeight = GetLiquidHeight(x, y, GetTileInformation(OffsetTileX, OffsetTileY));
    if(WaterHeight == 0.0f && !(GetLiquidType(x, y, GetTileInformation(OffsetTileX, OffsetTileY)) & 0x02))
        WaterHeight = NO_WATER_HEIGHT;
    else if(z != 0.0f && z != NO_WATER_HEIGHT)
    {
        if(z < GetHeight(x, y, GetTileInformation(OffsetTileX, OffsetTileY)))
            WaterHeight = NO_WATER_HEIGHT;
    }

    // Return our cached information.
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

    if(!AreTilesValid(TileX, TileY))
        return 0xFFFF;

    uint32 OffsetTileX = TileX-TileStartX;
    uint32 OffsetTileY = TileY-TileStartY;

    mutex.Acquire();
    if(!TileInformationLoaded(OffsetTileX, OffsetTileY))
    {
        mutex.Release();
        return 0xFFFF;
    }

    // Find the offset in the 2d array.
    uint16 AreaId = GetAreaEntry(x, y, GetTileInformation(OffsetTileX, OffsetTileY));

    // Return our cached information.
    mutex.Release();
    return AreaId;
}

void TerrainMgr::GetCellLimits(uint32 &StartX, uint32 &EndX, uint32 &StartY, uint32 &EndY)
{
    StartX = TileStartX*8;
    StartY = TileStartY*8;
    EndX = TileEndX*8;
    EndY = TileEndY*8;
}

size_t TerrainMgr::GetSize()
{
    size_t size = 0;
    for(uint32 x = TileStartX; x < TileEndX; x++)
    {
        for(uint32 y = TileStartY; y < TileEndY; y++)
        {
            if(LoadCounter[x][y])
            {
                size += sizeof(TileInformation[x][y]);
            }
        }
    }
    return size;
}

bool TerrainMgr::CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &AreaID)
{
    uint32 TileX = CellX/8;
    uint32 TileY = CellY/8;

    if(!AreTilesValid(TileX, TileY))
        return false;

    uint32 OffsetTileX = TileX-TileStartX;
    uint32 OffsetTileY = TileY-TileStartY;

    mutex.Acquire();
    uint32 areaid = 0;
    bool Required, Result = false;
    if((Required = !TileInformationLoaded(OffsetTileX, OffsetTileY)))
    {
        if(!LoadTileInformation(TileX, TileY))
        {
            mutex.Release();
            return Result;
        }
    }

    TileTerrainInformation *tile = GetTileInformation(OffsetTileX, OffsetTileY);
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

    if(!AreTilesValid(TileX, TileY))
        return NO_LAND_HEIGHT;

    uint32 OffsetTileX = TileX-TileStartX;
    uint32 OffsetTileY = TileY-TileStartY;

    mutex.Acquire();
    if(!TileInformationLoaded(OffsetTileX, OffsetTileY))
    {
        mutex.Release();
        return NO_LAND_HEIGHT;
    }

    float LandHeight = GetHeight(x, y, GetTileInformation(OffsetTileX, OffsetTileY));

    // Return our cached information.
    mutex.Release();
    return LandHeight;
}

void TerrainMgr::CellGoneActive(uint32 x, uint32 y)
{
    uint32 tileX = x/8, tileY = y/8;
    mutex.Acquire();

    LoadCounter[tileX][tileY]++;
    if(m_CollisionMap)
        sVMapInterface.ActivateTile(mapId, tileX, tileY);
    if(sWorld.PathFinding)
        NavMeshInterface.LoadNavMesh(mapId, tileX, tileY);
    if(!AreTilesValid(tileX, tileY))
    {
        mutex.Release();
        return;
    }

    uint32 OffsetTileX = tileX-TileStartX;
    uint32 OffsetTileY = tileY-TileStartY;
    if(TileInformationLoaded(OffsetTileX, OffsetTileY))
    {
        mutex.Release();
        return;
    }
    mutex.Release();

    // Load Tile information if it's not already loaded.
    if(LoadCounter[tileX][tileY] == 1)
        LoadTileInformation(tileX, tileY);
}

void TerrainMgr::CellGoneIdle(uint32 x, uint32 y)
{
    uint32 tileX = x/8, tileY = y/8;
    mutex.Acquire();
    LoadCounter[tileX][tileY]--;
    if(m_CollisionMap)
        sVMapInterface.DeactivateTile(mapId, tileX, tileY);
    if(sWorld.PathFinding)
        NavMeshInterface.UnloadNavMesh(mapId, tileX, tileY);
    if(!AreTilesValid(tileX, tileY))
    {
        mutex.Release();
        return;
    }

    uint32 OffsetTileX = tileX-TileStartX;
    uint32 OffsetTileY = tileY-TileStartY;
    if(Instance || !TileInformationLoaded(OffsetTileX, OffsetTileY))
    {
        mutex.Release();
        return;
    }
    mutex.Release();

    // If we're not an instance, unload our Tile info.
    if(LoadCounter[tileX][tileY] == 0)
        UnloadTileInformation(tileX, tileY);
}

void TerrainMgr::LoadAllTerrain()
{
    sLog.Debug("TerrainMgr", "[%u]: Loading all terrain", mapId);
    for(uint32 x = TileStartX; x < TileEndX; x++)
    {
        for(uint32 y = TileStartY; y < TileEndY; y++)
        {
            LoadCounter[x][y]++;
            LoadTileInformation(x, y);
        }
    }
    sLog.Debug("TerrainMgr", "[%u]: All terrain loaded", mapId);
}

void TerrainMgr::UnloadAllTerrain()
{
    for(uint32 x = 0; x < 64; x++)
    {
        for(uint32 y = 0; y < 64; y++)
        {
            LoadCounter[x][y]--;
            UnloadTileInformation(x, y);
        }
    }
}
