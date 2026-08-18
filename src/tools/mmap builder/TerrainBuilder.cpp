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

#include "TerrainBuilder.h"

#include <vmaplib\VMapLib.h>
#include <vector>

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08
#define MAP_LIQUID_TYPE_DARK_WATER  0x10
#define MAP_LIQUID_TYPE_WMO_WATER   0x20

extern const char *heightMapHeader;

namespace MMAP
{

    char const* MAP_VERSION_MAGIC = "v1.3";

    TerrainBuilder::TerrainBuilder(bool skipLiquid) : m_skipLiquid (skipLiquid), vmapManager(NULL){ }
    TerrainBuilder::~TerrainBuilder() { }

    /**************************************************************************/
    void TerrainBuilder::getLoopVars(Spot portion, int &loopStart, int &loopEnd, int &loopInc)
    {
        switch (portion)
        {
        case ENTIRE:
            loopStart = 0;
            loopEnd = V8_SIZE_SQ;
            loopInc = 1;
            break;
        case TOP:
            loopStart = 0;
            loopEnd = V8_SIZE;
            loopInc = 1;
            break;
        case LEFT:
            loopStart = 0;
            loopEnd = V8_SIZE_SQ - V8_SIZE + 1;
            loopInc = V8_SIZE;
            break;
        case RIGHT:
            loopStart = V8_SIZE - 1;
            loopEnd = V8_SIZE_SQ;
            loopInc = V8_SIZE;
            break;
        case BOTTOM:
            loopStart = V8_SIZE_SQ - V8_SIZE;
            loopEnd = V8_SIZE_SQ;
            loopInc = 1;
            break;
        }
    }

    /**************************************************************************/
    void getHeightCoord(int index, float xOffset, float yOffset, float* coord, float const (&height_map)[129][129])
    {
        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        coord[0] = (xOffset + index%(V9_SIZE)*GRID_PART_SIZE) * -1.f;
        coord[1] = (yOffset + (int)(index/(V9_SIZE))*GRID_PART_SIZE) * -1.f;
        coord[2] = height_map[index%(V9_SIZE)][index/(V9_SIZE)];
    }

    /**************************************************************************/
    void getHeightCoord(int index, float xOffset, float yOffset, float* coord, float const (&height_map)[128][128])
    {
        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        coord[0] = (xOffset + index%(V8_SIZE)*GRID_PART_SIZE + GRID_PART_SIZE/2.f) * -1.f;
        coord[1] = (yOffset + (int)(index/(V8_SIZE))*GRID_PART_SIZE + GRID_PART_SIZE/2.f) * -1.f;
        coord[2] = height_map[index%(V8_SIZE)][index/(V8_SIZE)];
    }

    // Note that the bool simply assigns the function, it is not used
    /**************************************************************************/
    void getHeightCoord(int index, bool v9, float xOffset, float yOffset, float* coord, float height)
    {
        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        coord[0] = (xOffset + index%(V9_SIZE)*GRID_PART_SIZE) * -1.f;
        coord[1] = (yOffset + (int)(index/(V9_SIZE))*GRID_PART_SIZE) * -1.f;
        coord[2] = height;
    }

    /**************************************************************************/
    void getHeightCoord(int index, float xOffset, float yOffset, float* coord, float height)
    {
        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        coord[0] = (xOffset + index%(V8_SIZE)*GRID_PART_SIZE + GRID_PART_SIZE/2.f) * -1.f;
        coord[1] = (yOffset + (int)(index/(V8_SIZE))*GRID_PART_SIZE + GRID_PART_SIZE/2.f) * -1.f;
        coord[2] = height;
    }

    /**************************************************************************/
    void fillXYCoord(bool V8, float v8point_distance, float v9point_distance, int x, int y, float xBase, float yBase, float &outputX, float &outputY)
    {
        float endX, endY;
        float xPOffset = x*v8point_distance, yPOffset = y*v8point_distance;
        if(V8)
        {
            xPOffset+=v8point_distance/2.f;
            yPOffset+=v8point_distance/2.f;
            endX = outputX = floor((xBase + xPOffset)*1000.f)/1000.f;
            endY = outputY = floor((yBase + yPOffset)*1000.f)/1000.f;
        }
        else
        {
            xPOffset = x*v9point_distance, yPOffset = y*v9point_distance;
            endX = outputX = floor((xBase + xPOffset)*1000.f)/1000.f;
            endY = outputY = floor((yBase + yPOffset)*1000.f)/1000.f;
        }
    }

    /**************************************************************************/
    void fillHeightCoord(bool V8, bool liquid, int x, int y, float z, MapChunkLoadData::ChunkInfo *info)
    {
        float endZ;

        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        if(liquid == false && V8)
            endZ = info->zT8[x][y] = z;
        else if(liquid == false)
            endZ = info->zT9[x][y] = z;
        else if(V8)
            endZ = info->zL8[x][y] = z;
        else endZ = info->zL9[x][y] = z;
    }

    /**************************************************************************/
    G3D::uint16 getLiquidType(int square, G3D::uint16 const (&liquid_type)[16][16])
    {
        int row = square / 128;
        int col = square % 128;
        int cellRow = row / 8;     // 8 squares per cell
        int cellCol = col / 8;

        return liquid_type[cellRow][cellCol];
    }

    void SetBounds(float min[], float max[], float x, float y, float z)
    {
        min[0] = G3D::min(min[0], x);
        max[0] = G3D::max(max[0], x);
        min[1] = G3D::min(min[1], z);
        max[1] = G3D::max(max[1], z);
        min[2] = G3D::min(min[2], y);
        max[2] = G3D::max(max[2], y);
    }

    struct READ_ChunkData
    {
        struct ChunkInfo// Chunk data is 16 chunks per tile plus portions from surrounding tiles
        {
            uint16 areaInfo;
            uint16 liquidType;
            uint16 navLiquidType;
            uint8 xOffset, yOffset;
            bool hasLiquid, isHole;

            float V8[V8_CH_SIZE][V8_CH_SIZE], L8[V8_CH_SIZE][V8_CH_SIZE];
            float V9[V9_CH_SIZE][V9_CH_SIZE], L9[V9_CH_SIZE][V9_CH_SIZE];
        } _chunkData[CHUNK_SIZE][CHUNK_SIZE];
    };

    /**************************************************************************/
    void getChReadVars(Spot portion, int &xStart, int &yStart)
    {
        switch (portion)
        {
        case CENTER: // Center means we read it all
            xStart = yStart = 0;
            break;
        case TOP: // Top means we read the bottom row
            xStart = CHUNK_SIZE-1;
            yStart = 0;
            break;
        case LEFT: // Left means we read the right column
            xStart = yStart = 0;
            break;
        case RIGHT: // Right means we read the left column
            xStart = 0;
            yStart = CHUNK_SIZE-1;
            break;
        case BOTTOM: // Bottom means we read the top row
            xStart = yStart = 0;
            break;
        }
    }

    /**************************************************************************/
    bool TerrainBuilder::_loadAndProcessMapChunks(FILE *mapFile, G3D::uint32 mapID, uint32 tileX, uint32 tileY, MapChunkLoadData *chunkData, std::map<uint32, uint32> &chunk_map)
    {
        if(!_loadMapChunks(mapFile, mapID, tileX, tileY, chunkData, chunk_map))
            return false;
        fclose(mapFile);

        // Process terrain height
        float xoffset = (float(tileX)-32)*GRID_SIZE;
        float yoffset = (float(tileY)-32)*GRID_SIZE;

        // Ugly bits ahead
        float coord[3];
        int index = 0, counter = 0;
        for(uint8 cy = 0; cy < 16; cy++)
        {
            for(uint8 ty = 0; ty < 8; ty++)
            {
                for(uint8 cx = 0; cx < 16; cx++)
                {
                    for(uint8 tx = 0; tx < 8; tx++)
                    {
                        // Grab our height coord from our chunk and append it to verts, also mark it as part of our bounding box
                        chunk_map[index++] = (cx<<24)|(tx<<16)|(cy<<8)|ty;
                        getHeightCoord(counter++, true, xoffset, yoffset, coord, chunkData->_chunkData[cx][cy].zT9[tx][ty]);
                        chunkData->solidVerts.append(coord[0]);
                        chunkData->solidVerts.append(coord[2]);
                        chunkData->solidVerts.append(coord[1]);

                        // Append either a valid liquid height or our terrain height
                        if(chunkData->_chunkData[cx][cy].zL9[tx][ty] != INVALID_MAP_LIQ_HEIGHT)
                            coord[2] = chunkData->_chunkData[cx][cy].zL9[tx][ty];
                        chunkData->waterVerts.append(coord[0]);
                        chunkData->waterVerts.append(coord[2]);
                        chunkData->waterVerts.append(coord[1]);
                    }

                    if(cx == 15)
                    {   // Grab our end of X and append it
                        chunk_map[index++] = (cx<<24)|(8<<16)|(cy<<8)|ty;
                        getHeightCoord(counter++, true, xoffset, yoffset, coord, chunkData->_chunkData[cx][cy].zT9[8][ty]);
                        chunkData->solidVerts.append(coord[0]);
                        chunkData->solidVerts.append(coord[2]);
                        chunkData->solidVerts.append(coord[1]);

                        // Append either a valid liquid height or our terrain height
                        if(chunkData->_chunkData[cx][cy].zL9[8][ty] != INVALID_MAP_LIQ_HEIGHT)
                            coord[2] = chunkData->_chunkData[cx][cy].zL9[8][ty];
                        chunkData->waterVerts.append(coord[0]);
                        chunkData->waterVerts.append(coord[2]);
                        chunkData->waterVerts.append(coord[1]);
                    }
                }
            }

            if(cy == 15)
            {
                for(uint8 cx = 0; cx < 16; cx++)
                {
                    for(uint8 tx = 0; tx < 8; tx++)
                    {
                        chunk_map[index++] = (cx<<24)|(tx<<16)|(cy<<8)|8;
                        getHeightCoord(counter++, true, xoffset, yoffset, coord, chunkData->_chunkData[cx][cy].zT9[tx][8]);
                        chunkData->solidVerts.append(coord[0]);
                        chunkData->solidVerts.append(coord[2]);
                        chunkData->solidVerts.append(coord[1]);

                        // Append either a valid liquid height or our terrain height
                        if(chunkData->_chunkData[cx][cy].zL9[tx][8] != INVALID_MAP_LIQ_HEIGHT)
                            coord[2] = chunkData->_chunkData[cx][cy].zL9[tx][8];
                        chunkData->waterVerts.append(coord[0]);
                        chunkData->waterVerts.append(coord[2]);
                        chunkData->waterVerts.append(coord[1]);
                    }

                    if(cx == 15)
                    {
                        chunk_map[index++] = (cx<<24)|(8<<16)|(cy<<8)|8;
                        getHeightCoord(counter++, true, xoffset, yoffset, coord, chunkData->_chunkData[cx][cy].zT9[8][8]);
                        chunkData->solidVerts.append(coord[0]);
                        chunkData->solidVerts.append(coord[2]);
                        chunkData->solidVerts.append(coord[1]);

                        // Append either a valid liquid height or our terrain height
                        if(chunkData->_chunkData[cx][cy].zL9[8][8] != INVALID_MAP_LIQ_HEIGHT)
                            coord[2] = chunkData->_chunkData[cx][cy].zL9[8][8];
                        chunkData->waterVerts.append(coord[0]);
                        chunkData->waterVerts.append(coord[2]);
                        chunkData->waterVerts.append(coord[1]);
                    }
                }
            }
        }

        counter = 0; // Append terrain V8 and reset counter
        for(uint8 cy = 0; cy < 16; cy++)
        {
            for(uint8 ty = 0; ty < 8; ty++)
            {
                for(uint8 cx = 0; cx < 16; cx++)
                {
                    for(uint8 tx = 0; tx < 8; tx++)
                    {
                        chunk_map[index++] = (cx<<24)|(tx<<16)|(cy<<8)|ty;
                        getHeightCoord(counter++, xoffset, yoffset, coord, chunkData->_chunkData[cx][cy].zT8[tx][ty]);
                        chunkData->solidVerts.append(coord[0]);
                        chunkData->solidVerts.append(coord[2]);
                        chunkData->solidVerts.append(coord[1]);

                        // Append either a valid liquid height or our terrain height
                        if(chunkData->_chunkData[cx][cy].zL8[tx][ty] != INVALID_MAP_LIQ_HEIGHT)
                            coord[2] = chunkData->_chunkData[cx][cy].zL8[tx][ty];
                        chunkData->waterVerts.append(coord[0]);
                        chunkData->waterVerts.append(coord[2]);
                        chunkData->waterVerts.append(coord[1]);
                    }
                }
            }
        }

        return true;
    }

    /**************************************************************************
    * Because the function is a million miles long, break it into these subfunctions for ease of reading
    * Also allows our data to be pretty easily cleaned up
    *****/
    bool LoadMapFileIntoReadData(FILE *mapFile, READ_ChunkData *read_data);
    void FillReadDataIntoLoadData(uint32 mapId, uint32 tileX, uint32 tileY, READ_ChunkData *read_data, MapChunkLoadData *loadData);
    /**************************************************************************/

    /**************************************************************************/
    bool TerrainBuilder::_loadMapChunks(FILE *mapFile, G3D::uint32 mapID, uint32 tileX, uint32 tileY, MapChunkLoadData *loadData, std::map<uint32, uint32> &chunk_map)
    {
        ASSERT(m_mapCompressionType[mapID] == MAP_COMPRESSED_CHUNKS);

        READ_ChunkData *read_data = new READ_ChunkData();
        bool result = LoadMapFileIntoReadData(mapFile, read_data);
        fclose(mapFile);

        // Process our input data into usable load data
        if(result) FillReadDataIntoLoadData(mapID, tileX, tileY, read_data, loadData);

        // Clean up read data
        delete read_data;

        // Check our compiled tile info
        return result;
    }

    bool LoadMapFileIntoReadData(FILE *mapFile, READ_ChunkData *read_data)
    {
        for(int x = 0; x < CHUNK_SIZE; x++)
        {
            for(int y = 0; y < CHUNK_SIZE; y++)
            {
                read_data->_chunkData[x][y].hasLiquid = false;
                read_data->_chunkData[x][y].navLiquidType = NAV_TOO_STEEP;
                float mapHeight, floatV8[8][8], floatV9[9][9];

                // TODO:
                read_data->_chunkData[x][y].isHole = false;
                if(fread(&read_data->_chunkData[x][y].xOffset, sizeof(uint8), 1, mapFile) != 1)
                    return false;
                if(fread(&read_data->_chunkData[x][y].yOffset, sizeof(uint8), 1, mapFile) != 1)
                    return false;
                if(fread(&read_data->_chunkData[x][y].areaInfo, sizeof(uint16), 1, mapFile) != 1)
                    return false;
                if(fread(&mapHeight, sizeof(float), 1, mapFile) != 1)
                    return false;

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
                if(fread(&compFlags, sizeof(uint8), 1, mapFile) != 1)
                    return false;

                switch(compFlags)
                {
                case 0x04:
                    uint8 uint8_V8[8*8], uint8_V9[9*9];
                    if(fread(&uint8_V8, sizeof(uint8)*8*8, 1, mapFile) != 1)
                        return false;
                    if(fread(&uint8_V9, sizeof(uint8)*9*9, 1, mapFile) != 1)
                        return false;
                    if(fread(&mult, sizeof(float), 1, mapFile) != 1)
                        return false;

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
                    if(fread(&uint16V8, sizeof(uint16)*8*8, 1, mapFile) != 1)
                        return false;
                    if(fread(&uint16V9, sizeof(uint16)*9*9, 1, mapFile) != 1)
                        return false;
                    if(fread(&mult, sizeof(float), 1, mapFile) != 1)
                        return false;

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
                    if(fread(&V8, sizeof(float)*8*8, 1, mapFile) != 1)
                        return false;
                    if(fread(&V9, sizeof(float)*9*9, 1, mapFile) != 1)
                        return false;

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
                    for(uint8 cy = 0; cy <= 8; cy++)
                    {
                        read_data->_chunkData[x][y].V9[cx][cy] = floatV9[cx][cy];

                        if(cx == 8 || cy == 8)
                            continue;
                        read_data->_chunkData[x][y].V8[cx][cy] = floatV8[cx][cy];
                    }
                }

                // We don't use holes yet
                uint32 holes;
                if(fread(&holes, sizeof(uint32), 1, mapFile) != 1)
                    return false;

                // Liquid reading
                if(fread(&read_data->_chunkData[x][y].liquidType, sizeof(uint16), 1, mapFile) != 1)
                    return false;

                // Decide if our liquid is usable or not...
                bool usableLiquidType = true;
                uint16 liquidType = read_data->_chunkData[x][y].liquidType;
                if(liquidType & MAP_LIQUID_TYPE_SLIME || liquidType & MAP_LIQUID_TYPE_MAGMA)
                    read_data->_chunkData[x][y].navLiquidType = NAV_MAGMA_SLIME;
                else if(liquidType & (MAP_LIQUID_TYPE_DARK_WATER|MAP_LIQUID_TYPE_OCEAN|MAP_LIQUID_TYPE_WATER))
                    read_data->_chunkData[x][y].navLiquidType = NAV_WATER;
                else usableLiquidType = false;

                // Read our liquid height if we have it
                float liqHeight = INVALID_MAP_LIQ_HEIGHT;
                if(fread(&compFlags, sizeof(uint8), 1, mapFile) != 1)
                    return false;

                if(compFlags != 0xFF) // 0xFF is dry land
                    if(fread(&liqHeight, sizeof(float), 1, mapFile) != 1)
                        return false;

                // Set our liquid V9 to read or invalid height
                for(uint8 cx = 0; cx <= 8; cx++)
                    for(uint8 cy = 0; cy <= 8; cy++)
                        floatV9[cx][cy] = liqHeight;

                if(compFlags != 0xFF) // 0xFF is dry land
                {
                    // Set liquid to our true/false from earlier, read regardless of usable or not.
                    read_data->_chunkData[x][y].hasLiquid = usableLiquidType;

                    float liqMult;
                    switch(compFlags)
                    {
                    case 0x04:
                        {
                            uint8 uint8L9[9*9];
                            if(fread(&uint8L9, sizeof(uint8)*9*9, 1, mapFile) != 1)
                                return false;

                            if(fread(&liqMult, sizeof(float), 1, mapFile) != 1)
                                return false;

                            for(uint8 cx = 0; cx <= 8; cx++)
                            {
                                for(uint8 cy = 0; cy <= 8; cy++)
                                {
                                    if(uint8L9[cx*9+cy] == 0xFF)
                                        floatV9[cx][cy] = INVALID_MAP_LIQ_HEIGHT;
                                    else floatV9[cx][cy] += (liqMult*float(uint8L9[cx*9+cy]));
                                }
                            }
                        }break;
                    case 0x02:
                        {
                            uint16 uint16L9[9*9];
                            if(fread(&uint16L9, sizeof(uint16)*9*9, 1, mapFile) != 1)
                                return false;

                            if(fread(&liqMult, sizeof(float), 1, mapFile) != 1)
                                return false;

                            for(uint8 cx = 0; cx <= 8; cx++)
                            {
                                for(uint8 cy = 0; cy <= 8; cy++)
                                {
                                    if(uint16L9[cx*9+cy] == 0xFFFF)
                                        floatV9[cx][cy] = INVALID_MAP_LIQ_HEIGHT;
                                    else floatV9[cx][cy] += (liqMult*float(uint16L9[cx*9+cy]));
                                }
                            }
                        }break;
                    case 0x01: // Flat water
                        {
                            uint64 liquid_mask;
                            if(fread(&liquid_mask, sizeof(uint64), 1, mapFile) != 1)
                                return false;

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

                            // We only have to invalidate our non existing liquid
                            for(uint8 cx = 0; cx <= 8; cx++)
                                for(uint8 cy = 0; cy <= 8; cy++)
                                    if(liquid_exists[cx][cy] == false)
                                        floatV9[cx][cy] = INVALID_MAP_LIQ_HEIGHT;
                        }break;
                    default: // Uncompressed height offsets
                        {
                            float L9[9*9];
                            if(fread(&L9, sizeof(float)*9*9, 1, mapFile) != 1)
                                return false;

                            for(uint8 cx = 0; cx <= 8; cx++)
                                for(uint8 cy = 0; cy <= 8; cy++)
                                    floatV9[cx][cy] += L9[cx*9+cy];
                        }break;
                    }
                }

                // Splice together liquid chunks into our tile storage
                for(uint8 cx = 0; cx <= 8; cx++)
                {
                    for(uint8 cy = 0; cy <= 8; cy++)
                    {
                        read_data->_chunkData[x][y].L9[cx][cy] = floatV9[cx][cy];
                    }
                }

                // Fill V8 map based on average of 4 corners
                for(uint32 i = 0; i < V8_CH_SIZE; i++)
                {
                    for(uint32 j = 0; j < V8_CH_SIZE; j++)
                    {
                        float heightMap[5] = { read_data->_chunkData[x][y].L9[i][j], read_data->_chunkData[x][y].L9[i+1][j], read_data->_chunkData[x][y].L9[i][j+1], read_data->_chunkData[x][y].L9[i+1][j+1], INVALID_MAP_LIQ_HEIGHT }, averageH = 0.f;
                        uint8 iAvg = 0;
                        for(uint8 i = 0; i < 4; i++)
                        {
                            if(heightMap[i] == INVALID_MAP_LIQ_HEIGHT)
                                continue;
                            averageH += heightMap[i];
                            iAvg++;
                        }

                        if(iAvg == 0)
                            read_data->_chunkData[x][y].L8[i][j] = INVALID_MAP_LIQ_HEIGHT;
                        else heightMap[4] = read_data->_chunkData[x][y].L8[i][j] = averageH/((float)iAvg);
                    }
                }
            }
        }

        return true;
    }

    void FillReadDataIntoLoadData(uint32 mapId, uint32 tileX, uint32 tileY, READ_ChunkData *read_data, MapChunkLoadData *loadData)
    {
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                // Begin transfer of read chunk data into our load data stack.
                loadData->_chunkData[x][y].liquidType = read_data->_chunkData[x][y].liquidType;
                loadData->_chunkData[x][y].navLiquidType = read_data->_chunkData[x][y].navLiquidType;
                // TODO: for now it's just default false or false flag
                loadData->_chunkData[x][y].isHole = read_data->_chunkData[x][y].isHole;

                // From here we're interolating points on the x and y axis and positioning our z point
                for(int i = 0; i < V9_CH_SIZE; i++)
                {
                    for(int j = 0; j < V9_CH_SIZE; j++)
                    {
                        loadData->_chunkData[x][y].useLiquidV9[i][j] = false;
                        fillHeightCoord(false, false, i, j, read_data->_chunkData[x][y].V9[i][j], &loadData->_chunkData[x][y]);
                        fillHeightCoord(false, true, i, j, read_data->_chunkData[x][y].L9[i][j], &loadData->_chunkData[x][y]);
                        if(read_data->_chunkData[x][y].hasLiquid)
                        {   // Calculate out liquid use per corner, simple height compare
                            float terrainHeight = read_data->_chunkData[x][y].V9[i][j],
                                liquidHeight = read_data->_chunkData[x][y].L9[i][j];
                            // We have caculated and decided wether to use our liquid or not.
                            loadData->_chunkData[x][y].useLiquidV9[i][j] = liquidHeight == INVALID_MAP_LIQ_HEIGHT ? false : liquidHeight > terrainHeight;
                        }

                        if(i == V8_CH_SIZE || j == V8_CH_SIZE)
                            continue;

                        // 
                        fillHeightCoord(true, false, i, j, read_data->_chunkData[x][y].V8[i][j], &loadData->_chunkData[x][y]);
                        fillHeightCoord(true, true, i, j, read_data->_chunkData[x][y].L8[i][j], &loadData->_chunkData[x][y]);

                        loadData->_chunkData[x][y].useLiquidV8[i][j] = read_data->_chunkData[x][y].L8[i][j] != INVALID_MAP_LIQ_HEIGHT;
                        if(loadData->_chunkData[x][y].useLiquidV8[i][j])
                        {
                            for (int q = TOP; q <= BOTTOM; q+=1)
                            {
                                bool useLiquid = read_data->_chunkData[x][y].hasLiquid;
                                if(useLiquid)
                                {   // Compare our height maps to decide if this tile quadrant uses the liquid data
                                    float liquidHeight[2] = {INVALID_MAP_LIQ_HEIGHT,INVALID_MAP_LIQ_HEIGHT},
                                        terrainHeight[2] = {INVALID_MAP_LIQ_HEIGHT,INVALID_MAP_LIQ_HEIGHT};
                                    //j1i1---j2i1
                                    //  |\ T /|
                                    //  | \ / |
                                    //  |L 0 R|
                                    //  | / \ |
                                    //  |/ B \|
                                    //j1i2---j2i2
                                    switch(q)
                                    {
                                    case TOP:
                                        terrainHeight[0] = read_data->_chunkData[x][y].V9[i][j];
                                        terrainHeight[1] = read_data->_chunkData[x][y].V9[i][j+1];
                                        liquidHeight[0] = read_data->_chunkData[x][y].L9[i][j];
                                        liquidHeight[1] = read_data->_chunkData[x][y].L9[i][j+1];
                                        break;
                                    case RIGHT:
                                        terrainHeight[0] = read_data->_chunkData[x][y].V9[i][j+1];
                                        terrainHeight[1] = read_data->_chunkData[x][y].V9[i+1][j+1];
                                        liquidHeight[0] = read_data->_chunkData[x][y].L9[i][j+1];
                                        liquidHeight[1] = read_data->_chunkData[x][y].L9[i+1][j+1];
                                        break;
                                    case LEFT:
                                        terrainHeight[0] = read_data->_chunkData[x][y].V9[i][j];
                                        terrainHeight[1] = read_data->_chunkData[x][y].V9[i+1][j];
                                        liquidHeight[0] = read_data->_chunkData[x][y].L9[i][j];
                                        liquidHeight[1] = read_data->_chunkData[x][y].L9[i+1][j];
                                        break;
                                    case BOTTOM:
                                        terrainHeight[0] = read_data->_chunkData[x][y].V9[i+1][j];
                                        terrainHeight[1] = read_data->_chunkData[x][y].V9[i+1][j+1];
                                        liquidHeight[0] = read_data->_chunkData[x][y].L9[i+1][j];
                                        liquidHeight[1] = read_data->_chunkData[x][y].L9[i+1][j+1];
                                        break;
                                    default:
                                        useLiquid = false;
                                        break;
                                    }

                                    if(useLiquid) // Require both corners of liquid to be heigher than both corners of terrain for liquid to be used.
                                        useLiquid = (liquidHeight[0] > terrainHeight[0] && liquidHeight[1] > terrainHeight[1]);
                                }

                                // We have caculated and decided wether to use our liquid or not.
                                loadData->_chunkData[x][y].useLiquid[(i*V8_CH_SIZE+j)*4+(q-TOP)] = useLiquid;
                            }
                        }
                    }
                }

                //printf("Points: T: %lli|%lli L: %lli|%lli\n", v8posPointMap.size(), v9posPointMap.size(), L8posPointMap.size(), L9posPointMap.size());
            }
        }
    }

    /**************************************************************************/
    bool getChunkLiquidData(MapChunkLoadData *chunkData, Spot s, uint32 chunkId0, uint32 chunkId1, uint32 chunkId2, uint16 &navLiquidType);
    /**************************************************************************/

    /**************************************************************************/
    bool TerrainBuilder::loadMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapTileLoadData &loadData)
    {
        if(tileX >= 64 || tileY >= 64)
            return false;
        if(m_mapOffsets[mapID][tileX][tileY] == 0)
            return false;

        char FileName[255];
        sprintf(FileName, "Tiles/%03u.tiletree", mapID);

        FILE* mapFile = NULL;
        fopen_s(&mapFile, FileName, "rb");
        if(mapFile == NULL)
            return false;

        // Go to our offset, skip the area info.
        if(fseek(mapFile, m_mapOffsets[mapID][tileX][tileY], SEEK_SET))
        {
            fclose(mapFile);
            return false;
        }

        std::map<uint32, uint32> chunk_map;
        MapChunkLoadData *chunkData = new MapChunkLoadData();
        // Fill our chunk data and chunk_map, loads from mapfile and fills vertices
        bool result = _loadAndProcessMapChunks(mapFile, mapID, tileX, tileY, chunkData, chunk_map);
        if(!result)
        {   // If we fail to load map chunks we don't close file
            fclose(mapFile); // Close it here
            sLog.Error("TerrainMgr", "Bad terrain tile for [%03u][%02u][%02u].", mapID, tileX, tileY);
            return false;
        }

        // Append our chunk verts
        loadData.terrainData[0].objVerts.append(chunkData->solidVerts);
        loadData.terrainData[1].objVerts.append(chunkData->waterVerts);
        loadData.allVerts.append(chunkData->solidVerts);
        loadData.allVerts.append(chunkData->waterVerts);

        // Use our chunk verts to set our boundbox
        float bmin[3], bmax[3];
        rcCalcBounds(chunkData->solidVerts.getCArray(), chunkData->solidVerts.size()/3, bmin, bmax);
        loadData.terrainData[0].boundBox.set(G3D::Vector3(bmin[0], bmin[1], bmin[2]), G3D::Vector3(bmax[0], bmax[1], bmax[2]));

        rcCalcBounds(chunkData->waterVerts.getCArray(), chunkData->waterVerts.size()/3, bmin, bmax);
        loadData.terrainData[1].boundBox.set(G3D::Vector3(bmin[0], bmin[1], bmin[2]), G3D::Vector3(bmax[0], bmax[1], bmax[2]));

        // Append our tris
        int tindices[] = { 0, 0, 0 };
        int loopStart = 0, loopEnd = 0, loopInc = 0, TriCount = 4;
        getLoopVars(ENTIRE, loopStart, loopEnd, loopInc);
        for (int i = loopStart; i < loopEnd; i += loopInc)
        {
            for (int j = TOP; j <= BOTTOM; j+=1)
            {
                getHeightTriangle(i, Spot(j), tindices);
                // Grab chunk data first
                uint16 navLiquidType;
                bool useWater = getChunkLiquidData(chunkData, Spot(j), chunk_map[tindices[0]], chunk_map[tindices[1]], chunk_map[tindices[2]], navLiquidType);

                // Append our land terrain tris
                loadData.terrainData[0].objTris.append(tindices[2]);
                loadData.terrainData[0].objTris.append(tindices[1]);
                loadData.terrainData[0].objTris.append(tindices[0]);
                loadData.terrainData[0].objFlags.append(NAV_TOO_STEEP);

                // Append our water terrain tris
                loadData.terrainData[1].objTris.append(tindices[2]);
                loadData.terrainData[1].objTris.append(tindices[1]);
                loadData.terrainData[1].objTris.append(tindices[0]);
                loadData.terrainData[1].objFlags.append(useWater ? navLiquidType : NAV_EMPTY);
            }
        }

        copyIndices(loadData.terrainData[0].objTris, loadData.allTris, 0);
        copyIndices(loadData.terrainData[1].objTris, loadData.allTris, chunkData->solidVerts.size()/3);

        delete chunkData;
        if ((loadData.terrainData[0].objTris.size() + loadData.terrainData[1].objTris.size()) == 0)
            return false;
        return true;
    }

    /**************************************************************************/
    void TerrainBuilder::getHeightTriangle(int square, Spot triangle, int* indices)
    {
        int rowOffset = square/V8_SIZE;
        switch (triangle)
        {
        case TOP:
            indices[0] = square+rowOffset;                  //           0-----1 .... 128
            indices[1] = square+1+rowOffset;                //           |\ T /|
            indices[2] = (V9_SIZE_SQ)+square;               //           | \ / |
            break;                                          //           |L 0 R| .. 127
        case LEFT:                                          //           | / \ |
            indices[0] = square+rowOffset;                  //           |/ B \|
            indices[1] = (V9_SIZE_SQ)+square;               //          129---130 ... 386
            indices[2] = square+V9_SIZE+rowOffset;          //           |\   /|
            break;                                          //           | \ / |
        case RIGHT:                                         //           | 128 | .. 255
            indices[0] = square+1+rowOffset;                //           | / \ |
            indices[1] = square+V9_SIZE+1+rowOffset;        //           |/   \|
            indices[2] = (V9_SIZE_SQ)+square;               //          258---259 ... 515
            break;
        case BOTTOM:
            indices[0] = (V9_SIZE_SQ)+square;
            indices[1] = square+V9_SIZE+1+rowOffset;
            indices[2] = square+V9_SIZE+rowOffset;
            break;
        default: break;
        }
    }

    bool getChunkLiquidData(MapChunkLoadData *chunkData, Spot s, uint32 chunkId0, uint32 chunkId1, uint32 chunkId2, uint16 &navLiquidType)
    {
        uint32 p0ChunkId = chunkId0, p1ChunkId = chunkId1, p2ChunkId = chunkId2;
        uint8 p0ty = p0ChunkId&0xFF, p1ty = p1ChunkId&0xFF, p2ty = p2ChunkId&0xFF;
        p0ChunkId>>=8,p1ChunkId>>=8,p2ChunkId>>=8;
        uint8 p0cy = p0ChunkId&0xFF, p1cy = p1ChunkId&0xFF, p2cy = p2ChunkId&0xFF;
        p0ChunkId>>=8,p1ChunkId>>=8,p2ChunkId>>=8;
        uint8 p0tx = p0ChunkId&0xFF, p1tx = p1ChunkId&0xFF, p2tx = p2ChunkId&0xFF;
        p0ChunkId>>=8,p1ChunkId>>=8,p2ChunkId>>=8;
        uint8 p0cx = p0ChunkId&0xFF, p1cx = p1ChunkId&0xFF, p2cx = p2ChunkId&0xFF;
        ASSERT(p0cx < 16 && p0cy < 16 && p1cx < 16 && p1cy < 16 && p2cx < 16 && p2cy < 16);
        // Only use our water chunk if all corners of liquid data apply

        switch(s)
        {
        case TOP:
            ASSERT(p0tx < 9 && p0ty < 9 && p1tx < 9 && p1ty < 9 && p2tx < 8 && p2ty < 8);
            if(chunkData->_chunkData[p2cx][p2cy].useLiquidV8[p2tx][p2ty] && chunkData->_chunkData[p0cx][p0cy].useLiquidV9[p0tx][p0ty] && chunkData->_chunkData[p1cx][p1cy].useLiquidV9[p1tx][p1ty])
            {
                navLiquidType = chunkData->_chunkData[p2cx][p2cy].navLiquidType;
                return true;
            }break;
        case LEFT:
            ASSERT(p0tx < 9 && p0ty < 9 && p1tx < 8 && p1ty < 8 && p2tx < 9 && p2ty < 9);
            if(chunkData->_chunkData[p1cx][p1cy].useLiquidV8[p1tx][p1ty] && chunkData->_chunkData[p0cx][p0cy].useLiquidV9[p0tx][p0ty] && chunkData->_chunkData[p2cx][p2cy].useLiquidV9[p2tx][p2ty])
            {
                navLiquidType = chunkData->_chunkData[p1cx][p1cy].navLiquidType;
                return true;
            }break;
        case RIGHT:
            ASSERT(p0tx < 9 && p0ty < 9 && p1tx < 9 && p1ty < 9 && p2tx < 8 && p2ty < 8);
            if(chunkData->_chunkData[p2cx][p2cy].useLiquidV8[p2tx][p2ty] && chunkData->_chunkData[p0cx][p0cy].useLiquidV9[p0tx][p0ty] && chunkData->_chunkData[p1cx][p1cy].useLiquidV9[p1tx][p1ty])
            {
                navLiquidType = chunkData->_chunkData[p2cx][p2cy].navLiquidType;
                return true;
            }break;
        case BOTTOM:
            ASSERT(p0tx < 8 && p0ty < 8 && p1tx < 9 && p1ty < 9 && p2tx < 9 && p2ty < 9);
            if(chunkData->_chunkData[p0cx][p0cy].useLiquidV8[p0tx][p0ty] && chunkData->_chunkData[p2cx][p2cy].useLiquidV9[p2tx][p2ty] && chunkData->_chunkData[p1cx][p1cy].useLiquidV9[p1tx][p1ty])
            {
                navLiquidType = chunkData->_chunkData[p0cx][p0cy].navLiquidType;
                return true;
            }break;
        }
        return false;
    }

    // DBC ids are stored in vmaps currently, so convert to terrain water flags
    // We could use DBC file, but this workaround is sufficient.
    G3D::uint16 convertWaterIDToFlags(G3D::uint16 wmoType)
    {
        switch(wmoType)
        {
            // Mask these to Regular Water
        case 1: case 5: case 9: case 13: case 17:
        case 41: case 61: case 81: case 181:
            return 0x01;
            // Mask these to Ocean Water
        case 2: case 6: case 10:
        case 14: case 100:
            return 0x02;
            // Mask these to Regular Magma
        case 3: case 7: case 11: case 15:
        case 19: case 121: case 141:
            return 0x04;
            // Mask these to Regular Slime
        case 4: case 8: case 12:
        case 20: case 21:
            return 0x08;
        }
        return 0;
    }

    /**************************************************************************/
    bool TerrainBuilder::InitializeMap(G3D::uint32 mapID)
    {
        char FileName[255];
        sprintf(FileName, "Tiles/%03u.tiletree", mapID);

        FILE* mapFile = NULL;
        fopen_s(&mapFile, FileName, "rb");
        if(mapFile == NULL)
            return false;

        char identifier[10];
        fread(&identifier, 10, 1, mapFile);
        if(strcmp(identifier, heightMapHeader))
        {   // If we have no height data but a proper vmap header, then load the vmap header
            sLog.Error("TerrainMgr", "Bad header for %s. Type %s", FileName, identifier);
            if(strcmp(identifier, VMAP::RAW_VMAP_MAGIC) == 0)
            {
                fseek(mapFile, 0, SEEK_SET);
                if(vmapManager == NULL)
                    vmapManager = new VMapManager("Tiles/obj");
                return vmapManager->loadMap(mapID, mapFile);
            }
            fclose(mapFile);
            return false;
        }

        uint32 vmapOffset = 0;
        if(fread(m_mapOffsets[mapID], 16384, 1, mapFile) != 1 || fread(&vmapOffset, sizeof(uint32), 1, mapFile) != 1
            || fread(&m_mapCompressionType[mapID], 1, 1, mapFile) != 1 || m_mapCompressionType[mapID] == 0 || m_mapCompressionType[mapID] >= MAP_COMPRESSED_MAX)
        {
            fclose(mapFile);
            return false;
        }

        fseek(mapFile, 0, SEEK_END);
        if(vmapOffset == 0 || vmapOffset == ftell(mapFile))
        {
            fclose(mapFile);
            return true;
        }

        // Seek to our offset and load our vmapmgr
        fseek(mapFile, vmapOffset, SEEK_SET);
        if(vmapManager == NULL)
            vmapManager = new VMapManager("Tiles/obj");
        bool res = vmapManager->loadMap(mapID, mapFile);
        fclose(mapFile);
        return res;
    }

    /**************************************************************************/
    void TerrainBuilder::UnloadVMap(G3D::uint32 mapID)
    {
        if(vmapManager)
            vmapManager->unloadMap(mapID);
    }

    /**************************************************************************/
    void FillFlags(G3D::Array<int> &output, size_t fillSize, unsigned char fillValue)
    {   // This is pretty temp, but it works
        for(size_t i = 0; i < fillSize; i++)
            output.append(fillValue);
    }

    /**************************************************************************/
    bool TerrainBuilder::loadVMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapTileLoadData &loadData)
    {
        if(vmapManager == NULL)
            return false;

        char FileName[255];
        sprintf(FileName, "Tiles/%03u.tiletree", mapID);

        FILE* mapFile = NULL;
        fopen_s(&mapFile, FileName, "rb");
        if(mapFile == NULL)
            return false;

        bool result = vmapManager->loadMap(mapID, tileX, tileY, mapFile), retval = false;
        do
        {
            if (result == false)
                break;

            InstanceTreeMap instanceTrees;
            vmapManager->getInstanceMapTree(instanceTrees);
            if (!instanceTrees[mapID])
                break;

            ModelInstance* models = NULL;
            G3D::uint32 count = 0;
            instanceTrees[mapID]->getModelInstances(models, count);
            if (!models)
                break;

            for (G3D::uint32 i = 0; i < count; ++i)
            {
                ModelInstance instance = models[i];

                // model instances exist in tree even though there are instances of that model in this tile
                WorldModel* worldModel = instance.getWorldModel();
                if (!worldModel)
                    continue;

                // now we have a model to add to the meshdata
                retval = true;

                std::vector<GroupModel> groupModels;
                worldModel->getGroupModels(groupModels);

                // all M2s need to have triangle indices reversed
                bool isM2 = instance.name.find(".m2") != std::string::npos || instance.name.find(".M2") != std::string::npos;

                // transform data
                float scale = instance.iScale;
                G3D::Matrix3 rotation = G3D::Matrix3::fromEulerAnglesXYZ(G3D::pi()*instance.iRot.z/-180.f, G3D::pi()*instance.iRot.x/-180.f, G3D::pi()*instance.iRot.y/-180.f);
                G3D::Vector3 position = instance.iPos;
                position.x -= 32*GRID_SIZE;
                position.y -= 32*GRID_SIZE;

                for (std::vector<GroupModel>::iterator it = groupModels.begin(); it != groupModels.end(); ++it)
                {
                    std::vector<G3D::Vector3> tempVertices;
                    std::vector<G3D::Vector3> transformedVertices;
                    std::vector<MeshTriangle> tempTriangles;
                    WmoLiquid* liquid = NULL;

                    it->getMeshData(tempVertices, tempTriangles, liquid);

                    // first handle collision mesh
                    transform(tempVertices, transformedVertices, scale, rotation, position);

                    // Create and store object data in our load tile
                    ObjectLoadData *vmapObj = new ObjectLoadData();
                    loadData.vmapData.append(vmapObj);

                    copyVertices(transformedVertices, vmapObj->objVerts);
                    copyIndices(tempTriangles, vmapObj->objTris, 0, isM2);
                    FillFlags(vmapObj->objFlags, vmapObj->objTris.size()/3, NAV_GROUND);

                    // Grab our mesh bound box, rotate, scale and position it, then invert our x and y axis
                    G3D::AABox transformedBoundBox;
                    it->GetMeshBoundTransformed(transformedBoundBox, rotation, scale, position, G3D::Vector3(-1.f, -1.f, 1.f));
                    // Change our boundbox from x,y,z to x,z,y
                    vmapObj->boundBox.set(G3D::Vector3(transformedBoundBox.low().x, transformedBoundBox.low().z, transformedBoundBox.low().y), G3D::Vector3(transformedBoundBox.high().x, transformedBoundBox.high().z, transformedBoundBox.high().y));

                    // now handle liquid data
                    if (liquid)
                    {
                        std::vector<G3D::Vector3> liqVerts;
                        std::vector<int> liqTris;
                        G3D::uint32 tilesX, tilesY, vertsX, vertsY;
                        G3D::Vector3 corner;
                        liquid->getPosInfo(tilesX, tilesY, corner);
                        vertsX = tilesX + 1;
                        vertsY = tilesY + 1;
                        G3D::uint8* flags = liquid->GetFlagsStorage();
                        float* data = liquid->GetHeightStorage();
                        G3D::uint8 type = NAV_EMPTY;

                        // convert liquid type to NavTerrain
                        switch (convertWaterIDToFlags(liquid->GetType()))
                        {
                        case 0x1:
                        case 0x2:
                            type = NAV_WATER;
                            break;
                        case 0x04:
                        case 0x08:
                            type = NAV_MAGMA_SLIME;
                            break;
                        }

                        // indexing is weird...
                        // after a lot of trial and error, this is what works:
                        // vertex = y*vertsX+x
                        // tile   = x*tilesY+y
                        // flag   = y*tilesY+x

                        G3D::Vector3 vert;
                        for (G3D::uint32 x = 0; x < vertsX; ++x)
                        {
                            for (G3D::uint32 y = 0; y < vertsY; ++y)
                            {
                                vert = G3D::Vector3(corner.x + x * GRID_PART_SIZE, corner.y + y * GRID_PART_SIZE, data[y*vertsX + x]);
                                vert = vert * rotation * scale + position;
                                vert.x *= -1.f;
                                vert.y *= -1.f;
                                liqVerts.push_back(vert);
                            }
                        }

                        int idx1, idx2, idx3, idx4;
                        G3D::uint32 square;
                        for (G3D::uint32 x = 0; x < tilesX; ++x)
                        {
                            for (G3D::uint32 y = 0; y < tilesY; ++y)
                            {
                                if ((flags[x+y*tilesX] & 0x0f) != 0x0f)
                                {
                                    square = x * tilesY + y;
                                    idx1 = square+x;
                                    idx2 = square+1+x;
                                    idx3 = square+tilesY+1+1+x;
                                    idx4 = square+tilesY+1+x;

                                    // top triangle
                                    liqTris.push_back(idx3);
                                    liqTris.push_back(idx2);
                                    liqTris.push_back(idx1);
                                    // bottom triangle
                                    liqTris.push_back(idx4);
                                    liqTris.push_back(idx3);
                                    liqTris.push_back(idx1);
                                }
                            }
                        }

                        G3D::uint32 liqOffset = vmapObj->objVerts.size() / 3;
                        for (G3D::uint32 i = 0; i < liqVerts.size(); ++i)
                            vmapObj->objVerts.append(liqVerts[i].x, liqVerts[i].z, liqVerts[i].y);

                        for (G3D::uint32 i = 0; i < liqTris.size() / 3; ++i)
                        {
                            vmapObj->objTris.append(liqTris[i*3+1] + liqOffset, liqTris[i*3+2] + liqOffset, liqTris[i*3] + liqOffset);
                            vmapObj->objFlags.append(type);
                        }
                    }
                }
            }
        }
        while (false);
        fclose(mapFile);

        vmapManager->unloadMap(mapID, tileX, tileY);
        return retval;
    }

    /**************************************************************************/
    void TerrainBuilder::transform(std::vector<G3D::Vector3> &source, std::vector<G3D::Vector3> &transformedVertices, float scale, G3D::Matrix3 &rotation, G3D::Vector3 &position)
    {
        for (std::vector<G3D::Vector3>::iterator it = source.begin(); it != source.end(); ++it)
        {
            // apply tranform, then mirror along the horizontal axes
            G3D::Vector3 v((*it) * rotation * scale + position);
            v.x *= -1.f;
            v.y *= -1.f;
            transformedVertices.push_back(v);
        }
    }

    /**************************************************************************/
    void TerrainBuilder::copyVertices(std::vector<G3D::Vector3> &source, G3D::Array<float> &dest)
    {
        for (std::vector<G3D::Vector3>::iterator it = source.begin(); it != source.end(); ++it)
        {
            dest.push_back((*it).x);
            dest.push_back((*it).z);
            dest.push_back((*it).y);
        }
    }

    /**************************************************************************/
    void TerrainBuilder::copyIndices(std::vector<MeshTriangle> &source, G3D::Array<int> &dest, int offset, bool flip)
    {
        if (flip)
        {
            for (std::vector<MeshTriangle>::iterator it = source.begin(); it != source.end(); ++it)
            {
                dest.push_back((*it).idx2+offset);
                dest.push_back((*it).idx1+offset);
                dest.push_back((*it).idx0+offset);
            }
        }
        else
        {
            for (std::vector<MeshTriangle>::iterator it = source.begin(); it != source.end(); ++it)
            {
                dest.push_back((*it).idx0+offset);
                dest.push_back((*it).idx1+offset);
                dest.push_back((*it).idx2+offset);
            }
        }
    }

    /**************************************************************************/
    void TerrainBuilder::copyIndices(G3D::Array<int> &source, G3D::Array<int> &dest, int offset)
    {
        int* src = source.getCArray();
        for (G3D::int32 i = 0; i < source.size(); ++i)
            dest.append(src[i] + offset);
    }

    /**************************************************************************/
    void TerrainBuilder::cleanVertices(G3D::Array<float> &verts, G3D::Array<int> &tris)
    {
        std::map<int, int> vertMap;

        int* t = tris.getCArray();
        float* v = verts.getCArray();

        G3D::Array<float> cleanVerts;
        int index, count = 0;
        // collect all the vertex indices from triangle
        for (int i = 0; i < tris.size(); ++i)
        {
            if (vertMap.find(t[i]) != vertMap.end())
                continue;
            std::pair<int, int> val;
            val.first = t[i];

            index = val.first;
            val.second = count;

            vertMap.insert(val);
            cleanVerts.append(v[index * 3], v[index * 3 + 1], v[index * 3 + 2]);
            count++;
        }

        verts.fastClear();
        verts.append(cleanVerts);
        cleanVerts.clear();

        // update triangles to use new indices
        for (int i = 0; i < tris.size(); ++i)
        {
            std::map<int, int>::iterator it;
            if ((it = vertMap.find(t[i])) == vertMap.end())
                continue;

            t[i] = (*it).second;
        }

        vertMap.clear();
    }

    /**************************************************************************/
    void TerrainBuilder::loadOffMeshConnections(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapTileLoadData &loadData, const char* offMeshFilePath)
    {
        // no meshfile input given?
        if (offMeshFilePath == NULL)
            return;

        FILE* fp = fopen(offMeshFilePath, "rb");
        if (!fp)
        {
            printf(" loadOffMeshConnections:: input file %s not found!\n", offMeshFilePath);
            return;
        }

        // pretty silly thing, as we parse entire file and load only the tile we need
        // but we don't expect this file to be too large
        char* buf = new char[512];
        while(fgets(buf, 512, fp))
        {
            float p0[3], p1[3];
            G3D::uint32 mid, tx, ty;
            float size;
            if (sscanf(buf, "%d %d,%d (%f %f %f) (%f %f %f) %f", &mid, &tx, &ty,
                &p0[0], &p0[1], &p0[2], &p1[0], &p1[1], &p1[2], &size) != 10)
                continue;

            if (mapID == mid && tileX == tx && tileY == ty)
            {
                loadData.offMeshConnections.append(p0[0]);
                loadData.offMeshConnections.append(p0[2]);
                loadData.offMeshConnections.append(p0[1]);

                loadData.offMeshConnections.append(p1[0]);
                loadData.offMeshConnections.append(p1[2]);
                loadData.offMeshConnections.append(p1[1]);

                loadData.offMeshConnectionDirs.append(1);          // 1 - both direction, 0 - one sided
                loadData.offMeshConnectionRads.append(size);       // agent size equivalent
                // can be used same way as polygon flags
                loadData.offMeshConnectionsAreas.append((unsigned char)0xFF);
                loadData.offMeshConnectionsFlags.append((unsigned short)0xFF);  // all movement masks can make this path
            }

        }

        delete [] buf;
        fclose(fp);
    }
}
