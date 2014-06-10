/***
 * Demonstrike Core
 */

#include "StdAfx.h"

TerrainMgr::TerrainMgr(string MapPath, uint32 MapId, bool Instanced, bool collisionMap) : mapPath(MapPath), mapId(MapId), Instance(Instanced), m_CollisionMap(collisionMap)
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

float GetHeightF(float x, float y, int x_int, int y_int, TileTerrainInformation* Tile)
{
    float a, b, c;
    // Select triangle:
    if(x + y < 1)
    {
        if(x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            float h1 = Tile->V9[x_int][y_int];
            float h2 = Tile->V9[x_int+1][y_int];
            float h5 = 2 * Tile->V8[x_int][y_int];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            float h1 = Tile->V9[x_int][y_int];
            float h3 = Tile->V9[x_int][y_int+1];
            float h5 = 2 * Tile->V8[x_int][y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if(x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            float h2 = Tile->V9[x_int+1][y_int];
            float h4 = Tile->V9[x_int+1][y_int+1];
            float h5 = 2 * Tile->V8[x_int][y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            float h3 = Tile->V9[x_int][y_int+1];
            float h4 = Tile->V9[x_int+1][y_int+1];
            float h5 = 2 * Tile->V8[x_int][y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return a * x + b * y + c;
}

float GetHeight(float x, float y, TileTerrainInformation* Tile)
{
    if(Tile == NULL)
        return NO_LAND_HEIGHT;

    x = 128 * (32 - x / 533.3333333f);
    y = 128 * (32 - y / 533.3333333f);

    int x_int = (int)x;
    int y_int = (int)y;

    x -= x_int;
    y -= y_int;

    x_int &= (128 - 1);
    y_int &= (128 - 1);

    return GetHeightF(x, y, x_int, y_int, Tile);
}

float GetLiquidHeight(float x, float y, TileTerrainInformation* Tile)
{
    if(Tile == NULL)
        return NO_WATER_HEIGHT;

    x = 128 * (32 - x / 533.3333333f);
    y = 128 * (32 - y / 533.3333333f);

    int cx_int = ((int)x & (128 - 1));
    int cy_int = ((int)y & (128 - 1));

    return Tile->liquid_height[cx_int][cy_int];
}

uint16 GetLiquidType(float x, float y, TileTerrainInformation* Tile)
{
    if(Tile == NULL)
        return 0;

    x = 16 * (32 - x / 533.3333333f);
    y = 16 * (32 - y / 533.3333333f);

    int lx = (int)x & 15;
    int ly = (int)y & 15;

    return Tile->LiquidInfo[lx * 16 + ly];
}

uint32 GetAreaFlags(float x, float y, TileTerrainInformation* Tile)
{
    if(Tile == NULL)
        return 0;

    x = 16 * (32 - x / 533.3333333f);
    y = 16 * (32 - y / 533.3333333f);

    int lx = (int)x & 15;
    int ly = (int)y & 15;

    return Tile->AreaInfo[lx * 16 + ly];
}

bool TerrainMgr::LoadTerrainHeader()
{
    // Create the path
    char File[200];
    snprintf(File, 200, "%s/Map_%03u.bin", mapPath.c_str(), (unsigned int)mapId);

    FileDescriptor = fopen(File, "rb");
    if(FileDescriptor == 0)
    {
        sLog.Error("TerrainMgr", "Map load failed for %s. Missing file?", File);
        return false;
    }

    /* check file size */
    fseek(FileDescriptor, 0, SEEK_END);
    if(ftell(FileDescriptor) == TERRAIN_HEADER_SIZE)
    {
        sLog.outDebug("Map file %s Ignored.", File);

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
        sLog.Error("TerrainMgr", "Terrain header read failed for %s! %u/%u | %u", File, dread, TERRAIN_HEADER_SIZE, sizeof(TileOffsets));
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

        // Read from our file into this newly created struct.
        fread(&tile->AreaInfo, sizeof(uint16), 256, FileDescriptor);
        fread(&tile->LiquidInfo, sizeof(uint16), 256, FileDescriptor);
        fread(&tile->V8, sizeof(float), 128*128, FileDescriptor);
        fread(&tile->V9, sizeof(float), 129*129, FileDescriptor);
        fread(&tile->liquid_height, sizeof(float), 129*129, FileDescriptor);
    }
    // Release the mutex.
    mutex.Release();

    // If we don't equal 0, it means the load was successful.
    if(TileInformation[offsX][offsY] != 0)
        return true;
    else
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

uint16 TerrainMgr::GetWaterType(float x, float y)
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
    uint16 Liquid = GetLiquidType(x, y, GetTileInformation(OffsetTileX, OffsetTileY));

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
    uint16 AreaId = GetAreaFlags(x, y, GetTileInformation(OffsetTileX, OffsetTileY));

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

    for(uint32 xc = (CellX%CellsPerTile)*16/CellsPerTile;xc<(CellX%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;xc++)
    {
        for(uint32 yc = (CellY%CellsPerTile)*16/CellsPerTile;yc<(CellY%CellsPerTile)*16/CellsPerTile+16/CellsPerTile;yc++)
        {
            areaid = GetTileInformation(OffsetTileX, OffsetTileY)->AreaInfo[yc*16+xc];
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
