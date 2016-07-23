/***
 * Demonstrike Core
 */

#include "StdAfx.h"

TerrainMgr::TerrainMgr(std::string MapPath, uint32 MapId) : file_name(MapPath), dummyMap(false), mapId(MapId), m_vmapOffset(0)
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

float getRawHeight(float x, float y, TileTerrainInformation* tile)
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
            float h1 = tile->V9[tx][ty];
            float h2 = tile->V9[tx+1][ty];
            float h5 = 2 * tile->V8[tx][ty];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            float h1 = tile->V9[tx][ty];
            float h3 = tile->V9[tx][ty+1];
            float h5 = 2 * tile->V8[tx][ty];
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
            float h2 = tile->V9[tx+1][ty];
            float h4 = tile->V9[tx+1][ty+1];
            float h5 = 2 * tile->V8[tx][ty];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            float h3 = tile->V9[tx][ty+1];
            float h4 = tile->V9[tx+1][ty+1];
            float h5 = 2 * tile->V8[tx][ty];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    return a * x + b * y + c;
}

float GetLiquidHeight(float x, float y, TileTerrainInformation* Tile)
{
    float retVal = NO_WATER_HEIGHT;
    if(Tile)
    {
        int lx = (int)(16 * (32 - x / TERRAIN_TILE_SIZE)) & 15;
        int ly = (int)(16 * (32 - y / TERRAIN_TILE_SIZE)) & 15;
        if((retVal = Tile->liquidHeight[lx][ly]) != NO_WATER_HEIGHT)
        {
            lx = int(144 * (32 - x / TERRAIN_TILE_SIZE)) & 143;
            ly = int(144 * (32 - y / TERRAIN_TILE_SIZE)) & 143;
            if(Tile->L9[lx][ly] == NO_WATER_HEIGHT)
                return NO_WATER_HEIGHT;
            retVal += Tile->L9[lx][ly];
        }
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
        retVal = Tile->liquidType[lx][ly];
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
        retVal = Tile->areaInfo[lx][ly];
    }
    return retVal;
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
    uint32 tileOffsets[64][64];
    if(fread(tileOffsets, terrainHeaderSize, 1, FileDescriptor) != 1 || fread(&m_vmapOffset, sizeof(uint32), 1, FileDescriptor) != 1)
    {
        sLog.Error("TerrainMgr", "Terrain header read failed for %s!", file_name.c_str());
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

                // For below calculations, x is up and down, y is left and right(Fucking great right?)
                for(uint8 x = 0; x < 16; x++)
                {
                    for(uint8 y = 0; y < 16; y++)
                    {
                        float mapHeight, floatV8[8][8], floatV9[9][9];
                        fread(&tile->areaInfo[x][y], sizeof(uint16), 1, input);
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
                                tile->V9[i][j] = floatV9[cx][cy];

                                if(cx == 8 || cy == 8)
                                    continue;
                                tile->V8[i][j] = floatV8[cx][cy];
                            }
                        }

                        // We don't use holes yet
                        uint32 holes;
                        fread(&holes, sizeof(uint32), 1, input);

                        // Liquid reading
                        fread(&tile->liquidType[x][y], sizeof(uint16), 1, input);
                        fread(&compFlags, sizeof(uint8), 1, input);
                        if(compFlags != 0xFF) // 0xFF is dry land
                        {
                            float liqMult;
                            fread(&tile->liquidHeight[x][y], sizeof(float), 1, input);
                            switch(compFlags)
                            {
                            case 0x04:
                                uint8 uint8L9[9*9];
                                fread(&uint8L9, sizeof(uint8)*9*9, 1, input);
                                fread(&liqMult, sizeof(float), 1, input);
                                for(uint8 cx = 0; cx <= 8; cx++)
                                {
                                    int i = x*9 + cx;
                                    for(uint8 cy = 0; cy <= 8; cy++)
                                    {
                                        int j = y*9 + cy;
                                        tile->L9[i][j] = liqMult*float(uint8L9[cx*9+cy]);
                                    }
                                }
                                break;
                            case 0x02:
                                uint16 uint16L9[9*9];
                                fread(&uint16L9, sizeof(uint16)*9*9, 1, input);
                                fread(&liqMult, sizeof(float), 1, input);
                                for(uint8 cx = 0; cx <= 8; cx++)
                                {
                                    int i = x*9 + cx;
                                    for(uint8 cy = 0; cy <= 8; cy++)
                                    {
                                        int j = y*9 + cy;
                                        tile->L9[i][j] = liqMult*float(uint16L9[cx*9+cy]);
                                    }
                                }
                                break;
                            case 0x01: break; // Flat water
                            default:
                                float L9[9*9];
                                fread(&L9, sizeof(float)*9*9, 1, input);
                                for(uint8 cx = 0; cx <= 8; cx++)
                                {
                                    int i = x*9 + cx;
                                    for(uint8 cy = 0; cy <= 8; cy++)
                                    {
                                        int j = y*9 + cy;
                                        tile->L9[i][j] = L9[cx*9+cy];
                                    }
                                }
                                break;
                            }
                        } else tile->liquidHeight[x][y] = NO_WATER_HEIGHT;
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
                if(uint16 areaId = tile->areaInfo[xc][yc])
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
        LandHeight = getRawHeight(x, y, GetTileInformation(TileX, TileY));
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
