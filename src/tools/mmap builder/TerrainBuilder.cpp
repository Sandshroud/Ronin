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
    void TerrainBuilder::getChLoopVars(Spot portion, int &xStart, int &xEnd, int &yStart, int &yEnd)
    {
        switch (portion)
        {
        case ENTIRE:
            xStart = yStart = 0;
            xEnd = yEnd = CHUNK_SIZE+2;
            break;
        case CENTER:
            xStart = yStart = 1;
            xEnd = yEnd = CHUNK_SIZE+1;
            break;
        case TOP:
            xStart = 0, xEnd = 1;
            yStart = 1, yEnd = CHUNK_SIZE+1;
            break;
        case LEFT:
            xStart = 1, xEnd = CHUNK_SIZE+1;
            yStart = 0, yEnd = 1;
            break;
        case RIGHT:
            xStart = 1, xEnd = CHUNK_SIZE+1;
            yStart = CHUNK_SIZE+1, yEnd = CHUNK_SIZE+2;
            break;
        case BOTTOM:
            xStart = CHUNK_SIZE+1, xEnd = CHUNK_SIZE+2;
            yStart = 1, yEnd = CHUNK_SIZE+1;
            break;

            // Corners are one chunk each
        case LOAD_CORNER1:
            xStart = 0, xEnd = 1;
            yStart = 0, yEnd = 1;
            break;
        case LOAD_CORNER2:
            xStart = 0, xEnd = 1;
            yStart = CHUNK_SIZE+1, yEnd = CHUNK_SIZE+2;
            break;
        case LOAD_CORNER3:
            xStart = CHUNK_SIZE+1, xEnd = CHUNK_SIZE+2;
            yStart = 0, yEnd = 1;
            break;
        case LOAD_CORNER4:
            xStart = CHUNK_SIZE+1, xEnd = CHUNK_SIZE+2;
            yStart = CHUNK_SIZE+1, yEnd = CHUNK_SIZE+2;
            break;
        }
    }

    /**************************************************************************/
    void getHeightCoord(int index, float xOffset, float yOffset, float* coord, float const (&height_map)[129][129])
    {
        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        coord[0] = (xOffset - index%(V9_SIZE)*GRID_PART_SIZE) * -1.f;
        coord[1] = (yOffset - (int)(index/(V9_SIZE))*GRID_PART_SIZE) * -1.f;
        coord[2] = height_map[index%(V9_SIZE)][index/(V9_SIZE)];
    }

    /**************************************************************************/
    void getHeightCoord(int index, float xOffset, float yOffset, float* coord, float const (&height_map)[128][128])
    {
        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        coord[0] = (xOffset - index%(V8_SIZE)*GRID_PART_SIZE + GRID_PART_SIZE/2.f) * -1.f;
        coord[1] = (yOffset - (int)(index/(V8_SIZE))*GRID_PART_SIZE + GRID_PART_SIZE/2.f) * -1.f;
        coord[2] = height_map[index%(V8_SIZE)][index/(V8_SIZE)];
    }

    /**************************************************************************/
    void fillHeightCoord(bool V8, bool liquid, float point_distance, int x, int y, float xBase, float yBase, float z, MapLoadData::ChunkInfo *info, float (&min)[3], float (&max)[3])
    {
        float endX, endY, endZ;
        float xPOffset = x*point_distance, yPOffset = y*point_distance;
        if(V8)
        {
            xPOffset+=point_distance/2.f;
            yPOffset+=point_distance/2.f;
            endX = info->xV8[x][y] = floor((xBase + xPOffset)*1000.f)/1000.f * -1.f;
            endY = info->yV8[x][y] = floor((yBase + yPOffset)*1000.f)/1000.f * -1.f;

            // Min Max x and y
            min[0] = min[0] > info->xV8[x][y] ? info->xV8[x][y] : min[0];
            min[2] = min[2] > info->yV8[x][y] ? info->yV8[x][y] : min[2];
            max[0] = max[0] < info->xV8[x][y] ? info->xV8[x][y] : max[0];
            max[2] = max[2] < info->yV8[x][y] ? info->yV8[x][y] : max[2];
        }
        else
        {
            endX = info->xV9[x][y] = floor((xBase + xPOffset)*1000.f)/1000.f * -1.f;
            endY = info->yV9[x][y] = floor((yBase + yPOffset)*1000.f)/1000.f * -1.f;

            // Min Max x and y
            min[0] = min[0] > info->xV9[x][y] ? info->xV9[x][y] : min[0];
            min[2] = min[2] > info->yV9[x][y] ? info->yV9[x][y] : min[2];
            max[0] = max[0] < info->xV9[x][y] ? info->xV9[x][y] : max[0];
            max[2] = max[2] < info->yV9[x][y] ? info->yV9[x][y] : max[2];
        }

        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        if(liquid == false && V8)
            endZ = info->zT8[x][y] = z;
        else if(liquid == false)
            endZ = info->zT9[x][y] = z;
        else if(V8)
            endZ = info->zL8[x][y] = z;
        else endZ = info->zL9[x][y] = z;

        min[1] = z < min[1] ? z : min[1];
        max[1] = z > max[1] ? z : max[1];
    }

    /**************************************************************************/
    void fillPositionPoint(G3D::Array<float> &array, MapLoadData::ChunkInfo *info, bool V8, bool liquid, int x, int y)
    {
        // Order is x, z, y
        array.append(info->xV9[x][y]);
        if(liquid == false && V8)
            array.append(info->zT8[x][y]);
        else if(liquid == false)
            array.append(info->zT9[x][y]);
        else if(V8)
            array.append(info->zL8[x][y]);
        else
            array.append(info->zL9[x][y]);
        array.append(info->yV9[x][y]);
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

    /**************************************************************************/
    bool TerrainBuilder::loadMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MeshData &meshData)
    {
        if (loadMap(mapID, tileX, tileY, meshData, ENTIRE))
        {
            loadMap(mapID, tileX+1, tileY, meshData, LEFT);
            loadMap(mapID, tileX-1, tileY, meshData, RIGHT);
            loadMap(mapID, tileX, tileY+1, meshData, TOP);
            loadMap(mapID, tileX, tileY-1, meshData, BOTTOM);
            return true;
        }
        return false;
    }

    /**************************************************************************/
    bool TerrainBuilder::loadMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapLoadData &loadData)
    {
        if (loadMap(mapID, tileX, tileY, loadData, CENTER, tileX, tileY))
        {
            // Load bottom row of chunk from above
            loadMap(mapID, tileX, tileY-1, loadData, TOP, tileX, tileY);
            // Load right row of chunk from the left
            loadMap(mapID, tileX-1, tileY, loadData, LEFT, tileX, tileY);
            // Load left row of chunk from the right
            loadMap(mapID, tileX+1, tileY, loadData, RIGHT, tileX, tileY);
            // Load chunk below us
            loadMap(mapID, tileX, tileY+1, loadData, BOTTOM, tileX, tileY);
            // Load top left chunk corner
            loadMap(mapID, tileX-1, tileY-1, loadData, LOAD_CORNER1, tileX, tileY);
            // Load top right chunk corner
            loadMap(mapID, tileX-1, tileY+1, loadData, LOAD_CORNER2, tileX, tileY);
            // Load bottom left chunk corner
            loadMap(mapID, tileX+1, tileY-1, loadData, LOAD_CORNER3, tileX, tileY);
            // Load bottom right chunk corner
            loadMap(mapID, tileX+1, tileY+1, loadData, LOAD_CORNER4, tileX, tileY);
            return true;
        }
        return false;
    }

    /**************************************************************************/
    bool TerrainBuilder::loadMapChunks(FILE *mapFile, G3D::uint32 mapID, MapTileData &data)
    {
        memset(&data.areaInfo, 0, sizeof(G3D::uint16)*CHUNK_SIZE*CHUNK_SIZE);
        memset(&data.liquidType, 0, sizeof(G3D::uint16)*CHUNK_SIZE*CHUNK_SIZE);
        memset(&data.V8, 0, sizeof(float)*V8_SIZE*V8_SIZE);
        memset(&data.V9, 0, sizeof(float)*V9_SIZE*V9_SIZE);
        memset(&data.L9, 0, sizeof(float)*V9_SIZE*V9_SIZE);

        switch(m_mapCompressionType[mapID])
        {
        case MAP_RAW_TILES:
            {   // Chunk stored in ordered arrays, no parsing
                bool res = fread(&data.areaInfo, sizeof(uint16)*CHUNK_SIZE*CHUNK_SIZE, 1, mapFile) == 1;
                if(res && fread(&data.liquidType, sizeof(uint16)*CHUNK_SIZE*CHUNK_SIZE, 1, mapFile) != 1)
                    res = false;
                if(res && fread(&data.V9, sizeof(float)*V9_SIZE_SQ, 1, mapFile) != 1)
                    res = false;
                if(res && fread(&data.V8, sizeof(float)*V8_SIZE_SQ, 1, mapFile) != 1)
                    res = false;
                if(res && fread(&data.L9, sizeof(float)*V9_SIZE_SQ, 1, mapFile) != 1)
                    res = false;
                return res;
            }break;
        case MAP_RAW_CHUNKS:
            {
                    bool res = fread(&data.areaInfo, sizeof(uint16)*CHUNK_SIZE*CHUNK_SIZE, 1, mapFile) == 1;
                    if(res && fread(&data.liquidType, sizeof(uint16)*CHUNK_SIZE*CHUNK_SIZE, 1, mapFile) != 1)
                        res = false;
                    for(uint8 x = 0; x < CHUNK_SIZE; x++)
                    {
                        for(uint8 y = 0; y < CHUNK_SIZE; y++)
                        {   // Todo
                            res = false;
                        }
                    }
                    return res;
            }break;
        case MAP_COMPRESSED_TILES:
            // Not supported
            break;
        case MAP_COMPRESSED_CHUNKS: // Not supported
            {
                for(uint8 x = 0; x < 16; x++)
                {
                    for(uint8 y = 0; y < 16; y++)
                    {
                        uint8 chunk_id[2];
                        float mapHeight, floatV8[8][8], floatV9[9][9];
                        if(fread(&chunk_id, sizeof(uint8)*2, 1, mapFile) != 1)
                            return false;
                        if(fread(&data.areaInfo[x][y], sizeof(uint16), 1, mapFile) != 1)
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
                            int i = x*8 + cx;
                            for(uint8 cy = 0; cy <= 8; cy++)
                            {
                                int j = y*8 + cy;
                                data.V9[i][j] = floatV9[cx][cy];

                                if(cx == 8 || cy == 8)
                                    continue;
                                data.V8[i][j] = floatV8[cx][cy];
                            }
                        }

                        // We don't use holes yet
                        uint32 holes;
                        if(fread(&holes, sizeof(uint32), 1, mapFile) != 1)
                            return false;

                        // Liquid reading
                        if(fread(&data.liquidType[x][y], sizeof(uint16), 1, mapFile) != 1)
                            return false;

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
                            int i = x*8 + cx;
                            for(uint8 cy = 0; cy <= 8; cy++)
                            {
                                int j = y*8 + cy;
                                data.L9[i][j] = floatV9[cx][cy];
                            }
                        }
                    }
                }

                // Fill V8 map based on average of 4 corners
                for(uint32 i = 0; i < V8_SIZE; i++)
                {
                    for(uint32 j = 0; j < V8_SIZE; j++)
                    {
                        float heightMap[5] = { data.L9[i][j], data.L9[i+1][j], data.L9[i][j+1], data.L9[i+1][j+1], INVALID_MAP_LIQ_HEIGHT }, averageH = 0.f;
                        uint8 iAvg = 0;
                        for(uint8 i = 0; i < 4; i++)
                        {
                            if(heightMap[i] == INVALID_MAP_LIQ_HEIGHT)
                                continue;
                            averageH += heightMap[i];
                            iAvg++;
                        }

                        if(iAvg == 0)
                            data.L8[i][j] = INVALID_MAP_LIQ_HEIGHT;
                        else data.L8[i][j] = averageH/((float)iAvg);
                    }
                }
                // Check our compiled tile info
                return true;
            }break;
        }
        return false;
    }

    struct READ_ChunkData
    {
        struct ChunkInfo// Chunk data is 16 chunks per tile plus portions from surrounding tiles
        {
            uint16 areaInfo;
            uint16 liquidType;
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

            // Corners are one chunk each
        case LOAD_CORNER1: // Top Left is Bottom Right Chunk
            xStart = CHUNK_SIZE-1;
            yStart = CHUNK_SIZE-1;
            break;
        case LOAD_CORNER2: // Top Right is Bottom Left chunk
            xStart = CHUNK_SIZE-1;
            yStart = CHUNK_SIZE-1;
            break;
        case LOAD_CORNER3: // Bottom left is Top Right Chunk
            xStart = 0;
            yStart = CHUNK_SIZE-1;
            break;
        case LOAD_CORNER4: // Bottom Right is Top Left Chunk
            xStart = 0;
            yStart = 0;
            break;
        }
    }

    /**************************************************************************/
    bool TerrainBuilder::loadMapChunks(FILE *mapFile, G3D::uint32 mapID, MapLoadData &loadData, Spot portion, G3D::uint32 fromTileX, G3D::uint32 fromTileY)
    {
        ASSERT(m_mapCompressionType[mapID] == MAP_COMPRESSED_CHUNKS);

        READ_ChunkData read_data;
        for(int x = 0; x < CHUNK_SIZE; x++)
        {
            for(int y = 0; y < CHUNK_SIZE; y++)
            {
                float mapHeight, floatV8[8][8], floatV9[9][9];
                read_data._chunkData[x][y].hasLiquid = false;

                // TODO:
                read_data._chunkData[x][y].isHole = false;
                if(fread(&read_data._chunkData[x][y].xOffset, sizeof(uint8), 1, mapFile) != 1)
                    return false;
                if(fread(&read_data._chunkData[x][y].yOffset, sizeof(uint8), 1, mapFile) != 1)
                    return false;
                if(fread(&read_data._chunkData[x][y].areaInfo, sizeof(uint16), 1, mapFile) != 1)
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
                        read_data._chunkData[x][y].V9[cx][cy] = floatV9[cx][cy];

                        if(cx == 8 || cy == 8)
                            continue;
                        read_data._chunkData[x][y].V8[cx][cy] = floatV8[cx][cy];
                    }
                }

                // We don't use holes yet
                uint32 holes;
                if(fread(&holes, sizeof(uint32), 1, mapFile) != 1)
                    return false;

                // Liquid reading
                if(fread(&read_data._chunkData[x][y].liquidType, sizeof(uint16), 1, mapFile) != 1)
                    return false;

                // Decide if our liquid is usable or not...
                bool usableLiquidType = true;
                uint16 liquidType = read_data._chunkData[x][y].liquidType;
                if(liquidType & MAP_LIQUID_TYPE_SLIME || liquidType & MAP_LIQUID_TYPE_MAGMA)
                    read_data._chunkData[x][y].liquidType = NAV_MAGMA_SLIME;
                else if(liquidType & (MAP_LIQUID_TYPE_DARK_WATER|MAP_LIQUID_TYPE_OCEAN|MAP_LIQUID_TYPE_WATER))
                    read_data._chunkData[x][y].liquidType = NAV_WATER;
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
                    read_data._chunkData[x][y].hasLiquid = usableLiquidType;

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
                        read_data._chunkData[x][y].L9[cx][cy] = floatV9[cx][cy];
                    }
                }

                // Fill V8 map based on average of 4 corners
                for(uint32 i = 0; i < V8_CH_SIZE; i++)
                {
                    for(uint32 j = 0; j < V8_CH_SIZE; j++)
                    {
                        float heightMap[5] = { read_data._chunkData[x][y].L9[i][j], read_data._chunkData[x][y].L9[i+1][j], read_data._chunkData[x][y].L9[i][j+1], read_data._chunkData[x][y].L9[i+1][j+1], INVALID_MAP_LIQ_HEIGHT }, averageH = 0.f;
                        uint8 iAvg = 0;
                        for(uint8 i = 0; i < 4; i++)
                        {
                            if(heightMap[i] == INVALID_MAP_LIQ_HEIGHT)
                                continue;
                            averageH += heightMap[i];
                            iAvg++;
                        }

                        if(iAvg == 0)
                            read_data._chunkData[x][y].L8[i][j] = INVALID_MAP_LIQ_HEIGHT;
                        else heightMap[4] = read_data._chunkData[x][y].L8[i][j] = averageH/((float)iAvg);
                    }
                }
            }
        }

        int readSX, readSY;
        getChReadVars(portion, readSX, readSY);

        int startX, endX, startY, endY;
        getChLoopVars(portion, startX, endX, startY, endY);
        int read_SX = readSX;
        for(int x = startX; x < endX; x++)
        {
            int rx = read_SX++;
            int read_SY = readSY;
            for(int y = startY; y < endY; y++)
            {
                int ry = read_SY++;
                // Begin transfer of read chunk data into our load data stack.
                loadData._chunkData[x][y].areaInfo = read_data._chunkData[rx][ry].areaInfo;
                loadData._chunkData[x][y].liquidType = read_data._chunkData[rx][ry].liquidType;
                loadData._chunkData[x][y].xOffset = read_data._chunkData[rx][ry].xOffset+startX-readSX;
                loadData._chunkData[x][y].yOffset = read_data._chunkData[rx][ry].yOffset+startY-readSY;
                // TODO: for now it's just default false or false flag
                loadData._chunkData[x][y].isHole = read_data._chunkData[rx][ry].isHole;

                int currTileX = fromTileX - 32, currTileY = fromTileY - 32;

                // Process terrain height
                float xbase = (float(currTileX)*GRID_SIZE) - GRID_CHUNK_SIZE + (GRID_CHUNK_SIZE*loadData._chunkData[x][y].xOffset);
                float ybase = (float(currTileY)*GRID_SIZE) - GRID_CHUNK_SIZE + (GRID_CHUNK_SIZE*loadData._chunkData[x][y].yOffset);

                // Min and max pre-calc
                loadData._chunkData[x][y].min[0] = xbase * -1.f;
                loadData._chunkData[x][y].min[2] = ybase * -1.f;
                loadData._chunkData[x][y].min[1] = INVALID_MAP_LIQ_HEIGHT_MAX;

                loadData._chunkData[x][y].max[0] = (xbase+GRID_CHUNK_SIZE) * -1.f;
                loadData._chunkData[x][y].max[2] = (ybase+GRID_CHUNK_SIZE) * -1.f;
                loadData._chunkData[x][y].max[1] = INVALID_MAP_LIQ_HEIGHT;

                // Tile is structured as 9*9 points of edge with 8*8 points of center
                float point_offset = GRID_CHUNK_SIZE/8.f;

                // From here we're interolating points on the x and y axis and positioning our z point
                for(int i = 0; i < V9_CH_SIZE; i++)
                {
                    for(int j = 0; j < V9_CH_SIZE; j++)
                    {
                        loadData._chunkData[x][y].useLiquidV9[i][j] = false;
                        fillHeightCoord(false, false, point_offset, i, j, xbase, ybase, read_data._chunkData[rx][ry].V9[i][j], &loadData._chunkData[x][y], loadData._chunkData[x][y].min, loadData._chunkData[x][y].max);
                        fillHeightCoord(false, true, point_offset, i, j, xbase, ybase, read_data._chunkData[rx][ry].L9[i][j], &loadData._chunkData[x][y], loadData._chunkData[x][y].min, loadData._chunkData[x][y].max);
                        if(read_data._chunkData[rx][ry].hasLiquid)
                        {   // Calculate out liquid use per corner, simple height compare
                            float terrainHeight = read_data._chunkData[rx][ry].V9[i][j],
                                liquidHeight = read_data._chunkData[rx][ry].L9[i][j];
                            // We have caculated and decided wether to use our liquid or not.
                            loadData._chunkData[x][y].useLiquidV9[i][j] = liquidHeight == INVALID_MAP_LIQ_HEIGHT ? false : liquidHeight > terrainHeight;
                        }

                        if(i == V8_CH_SIZE || j == V8_CH_SIZE)
                            continue;

                        fillHeightCoord(true, false, point_offset, i, j, xbase, ybase, read_data._chunkData[rx][ry].V8[i][j], &loadData._chunkData[x][y], loadData._chunkData[x][y].min, loadData._chunkData[x][y].max);
                        fillHeightCoord(true, true, point_offset, i, j, xbase, ybase, read_data._chunkData[rx][ry].L8[i][j], &loadData._chunkData[x][y], loadData._chunkData[x][y].min, loadData._chunkData[x][y].max);

                        for (int q = TOP; q <= BOTTOM; q+=1)
                        {
                            bool useLiquid = read_data._chunkData[rx][ry].hasLiquid;
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
                                    terrainHeight[0] = read_data._chunkData[rx][ry].V9[i][j];
                                    terrainHeight[1] = read_data._chunkData[rx][ry].V9[i][j+1];
                                    liquidHeight[0] = read_data._chunkData[rx][ry].L9[i][j];
                                    liquidHeight[1] = read_data._chunkData[rx][ry].L9[i][j+1];
                                    break;
                                case RIGHT:
                                    terrainHeight[0] = read_data._chunkData[rx][ry].V9[i][j+1];
                                    terrainHeight[1] = read_data._chunkData[rx][ry].V9[i+1][j+1];
                                    liquidHeight[0] = read_data._chunkData[rx][ry].L9[i][j+1];
                                    liquidHeight[1] = read_data._chunkData[rx][ry].L9[i+1][j+1];
                                    break;
                                case LEFT:
                                    terrainHeight[0] = read_data._chunkData[rx][ry].V9[i][j];
                                    terrainHeight[1] = read_data._chunkData[rx][ry].V9[i+1][j];
                                    liquidHeight[0] = read_data._chunkData[rx][ry].L9[i][j];
                                    liquidHeight[1] = read_data._chunkData[rx][ry].L9[i+1][j];
                                    break;
                                case BOTTOM:
                                    terrainHeight[0] = read_data._chunkData[rx][ry].V9[i+1][j];
                                    terrainHeight[1] = read_data._chunkData[rx][ry].V9[i+1][j+1];
                                    liquidHeight[0] = read_data._chunkData[rx][ry].L9[i+1][j];
                                    liquidHeight[1] = read_data._chunkData[rx][ry].L9[i+1][j+1];
                                    break;
                                default:
                                    useLiquid = false;
                                    break;
                                }

                                if(useLiquid) // Require both corners of liquid to be heigher than both corners of terrain for liquid to be used.
                                    useLiquid = (liquidHeight[0] > terrainHeight[0] && liquidHeight[1] > terrainHeight[1]);
                            }

                            // We have caculated and decided wether to use our liquid or not.
                            loadData._chunkData[x][y].useLiquid[(i*V8_CH_SIZE+j)*4+(q-TOP)] = useLiquid;
                        }

                    }
                }

                //printf("Points: T: %lli|%lli L: %lli|%lli\n", v8posPointMap.size(), v9posPointMap.size(), L8posPointMap.size(), L9posPointMap.size());
            }
        }
        // Check our compiled tile info
        return true;
    }

    /**************************************************************************/
    bool TerrainBuilder::loadMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MeshData &meshData, Spot portion)
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

        MapTileData tileData;
        bool result = loadMapChunks(mapFile, mapID, tileData);
        fclose(mapFile);

        if(!result)
        {
            sLog.Error("TerrainMgr", "Bad terrain tile for [%03u][%02u][%02u].", mapID, tileX, tileY);
            return false;
        }

        // Parse values for skippage
        bool allOcean = true;
        for(int x = 0; x < 16; x++)
            for(int y = 0; y < 16; y++)
                if(!(tileData.liquidType[x][y]&0x20))
                    allOcean = false;
        // We can skip tiles that are 100% ocean
        if(allOcean)
            return false;

        // Process terrain height
        int count = meshData.solidVerts.size() / 3;
        float xoffset = (float(tileX)-32)*GRID_SIZE;
        float yoffset = (float(tileY)-32)*GRID_SIZE;

        float coord[3];
        G3D::Array<int> ttriangles, ltriangles;
        for (int i = 0; i < V9_SIZE_SQ; ++i)
        {
            getHeightCoord(i, xoffset, yoffset, coord, tileData.V9);
            meshData.solidVerts.append(coord[1]);
            meshData.solidVerts.append(coord[2]);
            meshData.solidVerts.append(coord[0]);
        }

        for (int i = 0; i < V8_SIZE_SQ; ++i)
        {
            getHeightCoord(i, xoffset, yoffset, coord, tileData.V8);
            meshData.solidVerts.append(coord[1]);
            meshData.solidVerts.append(coord[2]);
            meshData.solidVerts.append(coord[0]);
        }

        int indices[] = { 0, 0, 0 };
        int loopStart = 0, loopEnd = 0, loopInc = 0, TriCount = 4;
        getLoopVars(portion, loopStart, loopEnd, loopInc);
        for (int i = loopStart; i < loopEnd; i += loopInc)
        {
            for (int j = TOP; j <= BOTTOM; j+=1)
            {
                getHeightTriangle(i, Spot(j), indices);
                ttriangles.append(indices[2] + count);
                ttriangles.append(indices[1] + count);
                ttriangles.append(indices[0] + count);
            }
        }

        count = meshData.liquidVerts.size() / 3;
        for (int i = 0; i < V9_SIZE_SQ; ++i)
        {
            getHeightCoord(i, xoffset, yoffset, coord, tileData.L9);
            meshData.liquidVerts.append(coord[1]);
            meshData.liquidVerts.append(coord[2]);
            meshData.liquidVerts.append(coord[0]);
        }

        for (int i = 0; i < V8_SIZE_SQ; ++i)
        {
            getHeightCoord(i, xoffset, yoffset, coord, tileData.L8);
            meshData.liquidVerts.append(coord[1]);
            meshData.liquidVerts.append(coord[2]);
            meshData.liquidVerts.append(coord[0]);
        }

        // generate triangles
        for (int i = loopStart; i < loopEnd; i += loopInc)
        {
            for (int j = TOP; j <= BOTTOM; j+=1)
            {
                getHeightTriangle(i, Spot(j), indices);
                ltriangles.append(indices[2] + count);
                ltriangles.append(indices[1] + count);
                ltriangles.append(indices[0] + count);
            }
        }

        if ((ltriangles.size() + ttriangles.size()) == 0)
            return false;

        // now that we have gathered the data, we can figure out which parts to keep:
        // liquid above ground, ground above liquid
        bool useTerrain, useLiquid;

        float* tverts = meshData.solidVerts.getCArray();
        int* ttris = ttriangles.getCArray();

        float* lverts = meshData.liquidVerts.getCArray();
        int* ltris = ltriangles.getCArray();

        for (int i = loopStart; i < loopEnd; i+=loopInc)
        {
            for (int j = 0; j < 2; ++j)
            {
                // default is true, will change to false if needed
                useTerrain = true, useLiquid = true;
                G3D::uint16 liquidType = MAP_LIQUID_TYPE_NO_WATER;

                // if there is no liquid, don't use liquid
                if (!meshData.liquidVerts.size() || !ltriangles.size())
                    useLiquid = false;
                else
                {
                    liquidType = getLiquidType(i, tileData.liquidType);
                    if(liquidType & MAP_LIQUID_TYPE_DARK_WATER)
                    {
                        useTerrain = false;
                        useLiquid = false;
                    }
                    else if(liquidType & MAP_LIQUID_TYPE_SLIME || liquidType & MAP_LIQUID_TYPE_MAGMA)
                        liquidType = NAV_MAGMA_SLIME;
                    else if(liquidType & (MAP_LIQUID_TYPE_OCEAN|MAP_LIQUID_TYPE_WATER))
                        liquidType = NAV_WATER;
                    else useLiquid = false;
                }

                if(liquidType == MAP_LIQUID_TYPE_NO_WATER)
                    useLiquid = false;

                // if there is no terrain, don't use terrain
                if (!ttriangles.size())
                    useTerrain = false;

                meshData.liquidType.append(liquidType);
                for (int k = 0; k < 3*TriCount/2; ++k)
                    meshData.liquidTris.append(ltris[k]);

                for (int k = 0; k < 3*TriCount/2; ++k)
                    meshData.solidTris.append(ttris[k]);

                // advance to next set of triangles
                ltris += 3*TriCount/2;
                ttris += 3*TriCount/2;
            }
        }

        return meshData.solidTris.size() || meshData.liquidTris.size();
    }

    /**************************************************************************/
    bool TerrainBuilder::loadMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapLoadData &loadData, Spot portion, G3D::uint32 fromTileX, G3D::uint32 fromTileY)
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

        MapTileData tileData;
        bool result = loadMapChunks(mapFile, mapID, loadData, portion, fromTileX, fromTileY);
        fclose(mapFile);

        if(!result)
        {
            sLog.Error("TerrainMgr", "Bad terrain tile for [%03u][%02u][%02u].", mapID, tileX, tileY);
            return false;
        }

        int startX, endX, startY, endY;
        getChLoopVars(portion, startX, endX, startY, endY);
        for(int x = startX; x < endX; x++)
        {
            for(int y = startY; y < endY; y++)
            {
                bool isHole = loadData._chunkData[x][y].isHole;
                // Fill our V9 terrain data into verts
                for(uint8 px = 0; px < V9_CH_SIZE; px++)
                {
                    for(uint8 py = 0; py < V9_CH_SIZE; py++)
                    {
                        fillPositionPoint(loadData._chunkData[x][y].solidVerts, &loadData._chunkData[x][y], false, false, px, py);
                        fillPositionPoint(loadData.solidVerts, &loadData._chunkData[x][y], false, false, px, py);

                        fillPositionPoint(loadData._chunkData[x][y].liquidVerts, &loadData._chunkData[x][y], false, true, px, py);
                        fillPositionPoint(loadData.liquidVerts, &loadData._chunkData[x][y], false, true, px, py);

                        fillPositionPoint(loadData._chunkData[x][y].combinedVerts, &loadData._chunkData[x][y], false, loadData._chunkData[x][y].useLiquidV9[px][py], px, py);
                    }
                }

                // Fill our V8 terrain data into verts
                for(uint8 px = 0; px < V8_CH_SIZE; px++)
                {
                    for(uint8 py = 0; py < V8_CH_SIZE; py++)
                    {
                        int index = px*V8_CH_SIZE+py;
                        fillPositionPoint(loadData._chunkData[x][y].solidVerts, &loadData._chunkData[x][y], true, false, px, py);
                        fillPositionPoint(loadData.solidVerts, &loadData._chunkData[x][y], true, false, px, py);

                        fillPositionPoint(loadData._chunkData[x][y].liquidVerts, &loadData._chunkData[x][y], true, true, px, py);
                        fillPositionPoint(loadData.liquidVerts, &loadData._chunkData[x][y], true, true, px, py);

                        bool useLiquid = loadData._chunkData[x][y].useLiquid[index*4]
                            && loadData._chunkData[x][y].useLiquid[index*4+1]
                            && loadData._chunkData[x][y].useLiquid[index*4+2]
                            && loadData._chunkData[x][y].useLiquid[index*4+3];
                        fillPositionPoint(loadData._chunkData[x][y].combinedVerts, &loadData._chunkData[x][y], true, useLiquid, px, py);
                    }
                }

                int mindice = 0, indices[] = { 0, 0, 0 };
                for (int i = 0; i < V8_CH_SIZE_SQ; i++)
                {
                    // Append our 4 triangles from our center
                    for (int j = TOP; j <= BOTTOM; j+=1)
                    {
                        // Checked above wether we will use the liquid quad or the terrain
                        bool useWaterHeight = loadData._chunkData[x][y].useLiquid[i*4+(j-TOP)];

                        getHeightTriangle(i, Spot(j), indices, true);

                        if(useWaterHeight)
                        {   // Append our liquid triangle
                            loadData._chunkData[x][y].liquidTris.append(indices[2]);
                            loadData._chunkData[x][y].liquidTris.append(indices[1]);
                            loadData._chunkData[x][y].liquidTris.append(indices[0]);
                            // Combine chunk
                            loadData._chunkData[x][y].combinedTris.append(indices[2]);
                            loadData._chunkData[x][y].combinedTris.append(indices[1]);
                            loadData._chunkData[x][y].combinedTris.append(indices[0]);
                            loadData._chunkData[x][y].combinedFlags.append(loadData._chunkData[x][y].liquidType);
                            continue;
                        }

                        // Append our triangle for the terrain
                        loadData._chunkData[x][y].solidTris.append(indices[2]);
                        loadData._chunkData[x][y].solidTris.append(indices[1]);
                        loadData._chunkData[x][y].solidTris.append(indices[0]);
                        // Combine chunk
                        loadData._chunkData[x][y].combinedTris.append(indices[2]);
                        loadData._chunkData[x][y].combinedTris.append(indices[1]);
                        loadData._chunkData[x][y].combinedTris.append(indices[0]);
                        // Hole support is pending, but we add the info anyway... Default as nav empty or ground
                        loadData._chunkData[x][y].combinedFlags.append(isHole ? NAV_EMPTY : NAV_GROUND);
                    }
                }
                printf("");
            }
        }

        return true;
    }

    /**************************************************************************/
    void TerrainBuilder::getHeightTriangle(int square, Spot triangle, int* indices, bool chunk)
    {
        if(chunk == false)
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
        else
        {
            int rowOffset = square/V8_CH_SIZE;
            switch (triangle)
            {
            case TOP:
                indices[0] = square+rowOffset;                  //           0-----1 .... 8
                indices[1] = square+1+rowOffset;                //           |\ T /|
                indices[2] = (V9_CH_SIZE_SQ)+square;            //           | \ / |
                break;                                          //           |L 0 R| .... 7
            case LEFT:                                          //           | / \ |
                indices[0] = square+rowOffset;                  //           |/ B \|
                indices[2] = (V9_CH_SIZE_SQ)+square;            //           9-----10 ... 18
                indices[1] = square+V9_CH_SIZE+rowOffset;       //           |\   /|
                break;                                          //           | \ / |
            case RIGHT:                                         //           |  55 | .... 63
                indices[0] = square+1+rowOffset;                //           | / \ |
                indices[1] = square+V9_CH_SIZE+1+rowOffset;     //           |/   \|
                indices[2] = (V9_CH_SIZE_SQ)+square;            //           72----73 ... 80
                break;
            case BOTTOM:
                indices[2] = (V9_CH_SIZE_SQ)+square;
                indices[1] = square+V9_CH_SIZE+1+rowOffset;
                indices[0] = square+V9_CH_SIZE+rowOffset;
                break;
            default: break;
            }
        }
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
    bool TerrainBuilder::loadVMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MeshData &meshData)
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
                    for(auto itr = tempVertices.begin(); itr != tempVertices.end(); ++itr)
                    {
                        float x = (*itr).x, y = (*itr).y, z = (*itr).z;
                        printf("");
                    }

                    // first handle collision mesh
                    transform(tempVertices, transformedVertices, scale, rotation, position);

                    int offset = meshData.solidVerts.size() / 3;

                    copyVertices(transformedVertices, meshData.solidVerts);
                    copyIndices(tempTriangles, meshData.solidTris, offset, isM2);

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

                        G3D::uint32 liqOffset = meshData.liquidVerts.size() / 3;
                        for (G3D::uint32 i = 0; i < liqVerts.size(); ++i)
                            meshData.liquidVerts.append(liqVerts[i].x, liqVerts[i].z, liqVerts[i].y);

                        for (G3D::uint32 i = 0; i < liqTris.size() / 3; ++i)
                        {
                            meshData.liquidTris.append(liqTris[i*3+1] + liqOffset, liqTris[i*3+2] + liqOffset, liqTris[i*3] + liqOffset);
                            meshData.liquidType.append(type);
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
    bool TerrainBuilder::loadVMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapLoadData &loadData)
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
                    for(auto itr = tempVertices.begin(); itr != tempVertices.end(); ++itr)
                    {
                        float x = (*itr).x, y = (*itr).y, z = (*itr).z;
                        printf("");
                    }

                    // first handle collision mesh
                    transform(tempVertices, transformedVertices, scale, rotation, position);

                    int offset = loadData.vmapSolidVerts.size() / 3;

                    copyVertices(transformedVertices, loadData.solidVerts);
                    copyVertices(transformedVertices, loadData.vmapSolidVerts);
                    copyIndices(tempTriangles, loadData.vmapSolidTris, offset, isM2);

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

                        G3D::uint32 liqOffset = loadData.vmapLiquidVerts.size() / 3;
                        for (G3D::uint32 i = 0; i < liqVerts.size(); ++i)
                        {
                            loadData.liquidVerts.append(liqVerts[i].x, liqVerts[i].z, liqVerts[i].y);
                            loadData.vmapLiquidVerts.append(liqVerts[i].x, liqVerts[i].z, liqVerts[i].y);
                        }

                        for (G3D::uint32 i = 0; i < liqTris.size() / 3; ++i)
                        {
                            loadData.vmapLiquidTris.append(liqTris[i*3+1] + liqOffset, liqTris[i*3+2] + liqOffset, liqTris[i*3] + liqOffset);
                            loadData.vmapLiquidTypes.append(type);
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
    void TerrainBuilder::loadOffMeshConnections(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MeshData &meshData, const char* offMeshFilePath)
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
                meshData.offMeshConnections.append(p0[0]);
                meshData.offMeshConnections.append(p0[2]);
                meshData.offMeshConnections.append(p0[1]);

                meshData.offMeshConnections.append(p1[0]);
                meshData.offMeshConnections.append(p1[2]);
                meshData.offMeshConnections.append(p1[1]);

                meshData.offMeshConnectionDirs.append(1);          // 1 - both direction, 0 - one sided
                meshData.offMeshConnectionRads.append(size);       // agent size equivalent
                // can be used same way as polygon flags
                meshData.offMeshConnectionsAreas.append((unsigned char)0xFF);
                meshData.offMeshConnectionsFlags.append((unsigned short)0xFF);  // all movement masks can make this path
            }

        }

        delete [] buf;
        fclose(fp);
    }
}
