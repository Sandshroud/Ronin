/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2014-2017 Sandshroud <https://github.com/Sandshroud>
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

#include "mpqfile.h"
#include "system.h"
#include "adtfile.h"

#include <algorithm>
#include <cstdio>

#ifdef WIN32
#define snprintf _snprintf
#endif

char const* GetPlainName(char const* FileName)
{
    const char * szTemp;

    if((szTemp = strrchr(FileName, '\\')) != NULL)
        FileName = szTemp + 1;
    return FileName;
}

char* GetPlainName(char* FileName)
{
    char * szTemp;

    if((szTemp = strrchr(FileName, '\\')) != NULL)
        FileName = szTemp + 1;
    return FileName;
}

void FixNameCase(char* name, size_t len)
{
    char* ptr = name + len - 1;

    //extension in lowercase
    for (; *ptr != '.'; --ptr)
        *ptr |= 0x20;

    for (; ptr >= name; --ptr)
    {
        if (ptr > name && *ptr >= 'A' && *ptr <= 'Z' && isalpha(*(ptr - 1)))
            *ptr |= 0x20;
        else if ((ptr == name || !isalpha(*(ptr - 1))) && *ptr >= 'a' && *ptr <= 'z')
            *ptr &= ~0x20;
    }
}

void FixNameSpaces(char* name, size_t len)
{
    for (size_t i=0; i<len-3; i++)
    {
        if(name[i] == ' ')
            name[i] = '_';
    }
}

char* GetExtension(char* FileName)
{
    if (char* szTemp = strrchr(FileName, '.'))
        return szTemp;
    return NULL;
}

ADTFile::ADTFile(HANDLE mpqarchive, char* filename) : ADT(mpqarchive, filename, false)
{
    Adtfilename.append(filename);
}

struct map_fileheader
{
    uint32 mapMagic;
    uint32 versionMagic;
    uint32 buildMagic;
    uint32 areaMapOffset;
    uint32 areaMapSize;
    uint32 heightMapOffset;
    uint32 heightMapSize;
    uint32 liquidMapOffset;
    uint32 liquidMapSize;
    uint32 holesOffset;
    uint32 holesSize;
};

// Map file format data
static char const* MAP_MAGIC         = "MAPS";
static char const* MAP_VERSION_MAGIC = "v1.5";
static char const* MAP_AREA_MAGIC    = "AREA";
static char const* MAP_HEIGHT_MAGIC  = "MHGT";
static char const* MAP_LIQUID_MAGIC  = "MLIQ";

#define MAP_AREA_NO_AREA      0x0001

struct map_areaHeader
{
    uint32 fourcc;
    uint16 flags;
    uint16 gridArea;
};

#define MAP_HEIGHT_NO_HEIGHT 0x0001
#define MAP_HEIGHT_AS_INT16  0x0002
#define MAP_HEIGHT_AS_INT8   0x0004

struct map_heightHeader
{
    uint32 fourcc;
    uint32 flags;
    float  gridHeight;
    float  gridMaxHeight;
};

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08

#define MAP_LIQUID_TYPE_DARK_WATER  0x10
#define MAP_LIQUID_TYPE_WMO_WATER   0x20


#define MAP_LIQUID_NO_TYPE    0x0001
#define MAP_LIQUID_NO_HEIGHT  0x0002

struct map_liquidHeader
{
    uint32 fourcc;
    uint16 flags;
    uint8  liquidType;
    uint8  offsetX;
    uint8  offsetY;
    uint8  width;
    uint8  height;
    float  liquidLevel;
};

typedef struct
{
	float levels[2];
    struct liquid_data{
        uint32 light;
        float  height;
    } liquid[9][9];
    // 1<<0 - ocean
    // 1<<1 - lava/slime
    // 1<<2 - water
    // 1<<6 - all water
    // 1<<7 - dark water
    // == 0x0F - not show liquid
    uint8 flags[8][8];
    uint8 data[84];
}MCLQinformation;

struct MH2O_liquid_header{
    uint16 liquidType;             // Index from LiquidType.dbc
    uint16 formatFlags;
    float  heightLevel[2];
    uint8  xOffset;
    uint8  yOffset;
    uint8  width;
    uint8  height;
    uint32 offsData2a;
    uint32 offsData2b;
};

struct adt_MCVT { float height_map[145]; };

float selectUInt8StepStore(float maxDiff, bool liquid) { return (liquid ? 0x7F : 0xFF) / maxDiff; }
float selectUInt16StepStore(float maxDiff, bool liquid) { return (liquid ? 0x7FFF : 0xFFFF) / maxDiff; }

extern uint16 *LiqType;
extern uint8 *liqSound;
extern int8 *liquidMaterial;

extern uint32 map_compression_type;

int getMH2OFormat(uint32 format, uint32 type)
{
    if(format & 0x02)
        return 0xFF;

    if(format < 42)
        return format;

    if(type == 2) // Skip depth processing
        return 0xFF;

    uint16 l_type = liqSound[type];
    uint16 l_material = liquidMaterial[l_type];
    if(l_type != 0xFF && l_material != 0x7F)
        return l_material;

    return 0xff;
}

bool ADTFile::parseCHNK(MapCreationInfo *storage, uint32 tileX, uint32 tileY)
{
    if(ADT.isEof ())
        return false;

    size_t currPos, nextpos;
    memset(_chunks, 0, sizeof(chunk)*16*16);
    uint32 MAPCHUNKOffset[16][16], MH2OChunkOffsets[16][16];
    memset(MAPCHUNKOffset, 0, sizeof(MAPCHUNKOffset));
    memset(MH2OChunkOffsets, 0, sizeof(MH2OChunkOffsets));

    uint32 size = 0, adtVersion = 0, mh2oBase = 0;
    while (!ADT.isEof())
    {
        char fourcc[5];
        ADT.read(&fourcc,4);
        ADT.read(&size, 4);
        flipcc(fourcc);
        fourcc[4] = 0;

        currPos = ADT.getPos(), nextpos = currPos + size;
        if(size)
        {
            if (!strcmp(fourcc,"MVER"))
                ADT.read(&adtVersion, sizeof(uint32));
            else if(!strcmp(fourcc, "MCNR"))
                nextpos = ADT.getPos() + 0x1C0; // size fix
            else if (!strcmp(fourcc,"MHDR"))
            { }// MHDR is after version header, contains offset data for parsing the ADT file
            else if (!strcmp(fourcc,"MCIN"))
            {
                for(uint8 cx = 0; cx < 16; cx++)
                {
                    for(uint8 cy = 0; cy < 16; cy++)
                    {
                        ADT.read(&MAPCHUNKOffset[cx][cy], 4);
                        ADT.seekRelative(12);
                    }
                }
            }
            else if (!strcmp(fourcc,"MCNK"))
            {
                size_t MCNK_pos = currPos;
                ADT.seekRelative(4);
                uint32 x = 0, y = 0;
                ADT.read(&x, 4);
                ADT.read(&y, 4);
                MAPCHUNKOffset[y][x] = MCNK_pos;
            }
            else if (!strcmp(fourcc,"MFBO"))
            {   // Flight box
                // These are height planes for the top of the world and the bottom of the world
                unsigned short minimum[3][3], maximum[3][3];
                ADT.read(&maximum, sizeof(short)*3*3);
                ADT.read(&minimum, sizeof(short)*3*3);
                // Camera doesn't go below the minimum
                minimum, maximum;
            }
            else if (!strcmp(fourcc,"MH2O"))
            {
                uint8 index = 0;
                mh2oBase = currPos;
                for(uint8 x = 0; x < 16; x++)
                {
                    for(uint8 y = 0; y < 16; y++)
                    {
                        ADT.seek(currPos+index*12);
                        ADT.read(&MH2OChunkOffsets[x][y], sizeof(uint32));
                        index++;
                    }
                }
            }
        }

        //======================
        ADT.seek(nextpos);
    }

    // Parse our liquid data outside of our chunk scanning
    for(uint8 cx = 0; cx < 16; cx++)
    {
        for(uint8 cy = 0; cy < 16; cy++)
        {
            ADT.seek(currPos = MAPCHUNKOffset[cx][cy]);

            MapChunkHeader header;
            ADT.read(&header, sizeof(MapChunkHeader));
            _chunks[cx][cy].areaEntry = header.areaid;
            _chunks[cx][cy].xpos = header.zpos;
            _chunks[cx][cy].ypos = header.xpos;
            _chunks[cx][cy].mapHeight = header.ypos;
            _chunks[cx][cy].holes = header.holes;
            if(header.offsMCVT)
            {
                ADT.seek(currPos+header.offsMCVT);
                ADT.read(_chunks[cx][cy].pointHeights, 145*sizeof(float));
            }

            if(header.flags & 0x04)
                _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_WATER;
            if(header.flags & 0x08)
                _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_OCEAN;
            if(header.flags & 0x10)
                _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_MAGMA;
            if(header.flags & 0x20)
                _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_SLIME;

            // MH2O chunks take priority over MCLQ so we handle them second in case of overrides
            if(mh2oBase && MH2OChunkOffsets[cx][cy])
            {
                ADT.seek(mh2oBase+MH2OChunkOffsets[cx][cy]);
                MH2O_liquid_header mh2oHeader;
                ADT.read(&mh2oHeader, sizeof(MH2O_liquid_header));
                if(tileX == 17 && tileY == 36 && cx == 2 && cy == 14)
                    printf("");
                uint64 mask = 0xFFFFFFFFFFFFFFFFLL;
                if(mh2oHeader.offsData2a)
                {
                    ADT.seek(mh2oBase+mh2oHeader.offsData2a);
                    ADT.read(&mask, sizeof(uint64));
                }

                // Save our liquid mask for appending
                _chunks[cx][cy].liquidMask = mask;

                bool readHeight = getMH2OFormat(mh2oHeader.formatFlags, mh2oHeader.liquidType) != 0xFF && mh2oHeader.offsData2b > 0;
                if(readHeight)
                    ADT.seek(mh2oBase+mh2oHeader.offsData2b);

                uint32 maskCount = 0;
                // Parse the 64bit liquid mask
                for(uint8 y = mh2oHeader.yOffset; y < mh2oHeader.height; y++)
                {
                    for(uint8 x = mh2oHeader.xOffset; x < mh2oHeader.width; x++)
                    {
                        if(mask & 0x01)
                        {   // If we have a V8 liquid show, then we show all 4 points around the V8 point
                            if(_chunks[cx][cy].show_liquid[y][x] == false)
                            {
                                _chunks[cx][cy].show_liquid[y][x] = true;
                                maskCount++;
                            }
                            if(_chunks[cx][cy].show_liquid[y][x+1] == false)
                            {
                                _chunks[cx][cy].show_liquid[y][x+1] = true;
                                maskCount++;
                            }
                            if(_chunks[cx][cy].show_liquid[(y+1)][x] == false)
                            {
                                _chunks[cx][cy].show_liquid[(y+1)][x] = true;
                                maskCount++;
                            }
                            if(_chunks[cx][cy].show_liquid[(y+1)][x+1] == false)
                            {
                                _chunks[cx][cy].show_liquid[(y+1)][x+1] = true;
                                maskCount++;
                            }
                        }
                        mask>>=1;
                    }
                }

                /*// Parse our liquid height data
                curChunk->liquidHeight = mh2oHeader.heightLevel1;
                for(uint8 x = 0; x < 9; x++)
                    for(uint8 y = 0; y < 9; y++)
                       curChunk->L9[x*9+y] = curChunk->liquidHeight;*/

                for(uint8 y = 0; y <= mh2oHeader.height; y++)
                {
                    for(uint8 x = 0; x <= mh2oHeader.width; x++)
                    {
                        float currentWaterHeight = 0.f;
                        if(readHeight)
                        {
                            ADT.read(&currentWaterHeight, sizeof(float));
                        } else currentWaterHeight = mh2oHeader.heightLevel[0];

                        if(_chunks[cx][cy].show_liquid[y][x] == false)
                        {   // No ADT height
                            _chunks[cx][cy].liquid_height[y][x] = -50000.f;
                            continue;
                        }
                        if(mh2oHeader.heightLevel[0] && currentWaterHeight == 0.f)
                            currentWaterHeight = mh2oHeader.heightLevel[0];

                        if(_chunks[cx][cy].liquid_height[y][x] && _chunks[cx][cy].liquid_height[y][x] != -50000.f)
                        {

                        } else _chunks[cx][cy].liquid_height[y][x] = currentWaterHeight;
                    }
                }

                // Grab our liquid type from the header and store it in the chunk
                uint32 type = LiqType[mh2oHeader.liquidType];
                switch(type)
                {
                case 0: _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_WATER; break;
                case 2: _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_MAGMA; break;
                case 3: _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_SLIME; break;
                case 1: _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_OCEAN;
                    if((mh2oHeader.formatFlags & 0x01) || !mh2oHeader.offsData2b)
                        _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_DARK_WATER;
                    break;
                }
            }
            else if(uint32 mclqOffset = header.sizeMCLQ > 8 && header.offsMCLQ ? currPos+header.offsMCLQ : 0) // Pre wotlk water chunks
            {
                ADT.seek(mclqOffset);
                MCLQinformation mclqChunk;
                ADT.read(&mclqChunk, sizeof(MCLQinformation));

                /*for(uint8 x = 0; x < 9; x++)
                {
                    for(uint8 y = 0; y < 9; y++)
                    {
                        _chunks[cx][cy].L9[y*9+x] = mclqChunk.liquid[x][y].height;
                        if(mclqChunk.flags[x][y] != 0x0F)
                        {
                            _chunks[cx][cy].show_liquid[x][y] = true;
                            if(mclqChunk.flags[x][y] & (1<<7))
                                _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_DARK_WATER;
                        }
                    }
                }*/
            }
        }
    }

    ADT.close();
    return true;
}

extern uint32 map_compression_flag;

static const int CHUNK_SIZE = 16;
static const int CHUNK_SIZE_SQ = CHUNK_SIZE*CHUNK_SIZE;

static const int V9_SIZE = (CHUNK_SIZE*8)+1;
static const int V9_SIZE_SQ = V9_SIZE*V9_SIZE;
static const int V8_SIZE = CHUNK_SIZE*8;
static const int V8_SIZE_SQ = V8_SIZE*V8_SIZE;

static const int LV9_SIZE = (CHUNK_SIZE*9);
static const int LV9_SIZE_SQ = LV9_SIZE*LV9_SIZE;

// see contrib/extractor/system.cpp, CONF_use_minHeight
static const float INVALID_MAP_LIQ_HEIGHT = -500.f;
static const float INVALID_MAP_LIQ_HEIGHT_MAX = 5000.0f;

struct MapTileSpread
{
    G3D::uint16 AreaInfo[CHUNK_SIZE_SQ];
    G3D::uint16 LiquidInfo[CHUNK_SIZE_SQ];
    float V8[V8_SIZE][V8_SIZE];
    float V9[V9_SIZE][V9_SIZE];

    uint64 liquid_mask[CHUNK_SIZE_SQ];
    bool liquid_exists[V9_SIZE][V9_SIZE];
    float liquid_height[V9_SIZE][V9_SIZE];
};

void ADTFile::WriteCHNK(FILE *output)
{
    switch(map_compression_type)
    {
    case MAP_RAW_TILES:
        {
            MapTileSpread tileSpreadInfo;

            // Pull our area and liquid info into our spread
            for(uint8 cx = 0; cx < CHUNK_SIZE; cx++)
            {
                for(uint8 cy = 0; cy < CHUNK_SIZE; cy++)
                {
                    uint16 offset = cx*CHUNK_SIZE+cy;
                    tileSpreadInfo.AreaInfo[offset] = _chunks[cx][cy].areaEntry;
                    tileSpreadInfo.LiquidInfo[offset] = _chunks[cx][cy].liquidEntry;
                }
            }

            // Append area info and liquid info
            fwrite(&tileSpreadInfo.AreaInfo, sizeof(uint16)*CHUNK_SIZE_SQ, 1, output);
            fwrite(&tileSpreadInfo.LiquidInfo, sizeof(uint16)*CHUNK_SIZE_SQ, 1, output);

            // Feed chunk data into tile spread info
            for(uint8 cx = 0; cx < CHUNK_SIZE; cx++)
            {
                for(uint8 cy = 0; cy < CHUNK_SIZE; cy++)
                {
                    // Parse liquid data first
                    bool hasLiqHeight = false;
                    /*for(uint8 i = 0; i < 81; i++)
                    {
                        if(_chunks[cx][cy].L9[i] == -50000.f)
                            continue;
                        hasLiqHeight = true;
                    }*/

                    // Height values for triangles stored in order:
                    // 1     2     3     4     5     6     7     8     9
                    //    10    11    12    13    14    15    16    17
                    // 18    19    20    21    22    23    24    25    26
                    //    27    28    29    30    31    32    33    34
                    // . . . . . . . .
                    // For better get height values merge it to V9 and V8 map
                    // V9 height map:
                    // 1     2     3     4     5     6     7     8     9
                    // 18    19    20    21    22    23    24    25    26
                    // . . . . . . . .
                    // V8 height map:
                    //    10    11    12    13    14    15    16    17
                    //    27    28    29    30    31    32    33    34
                    // . . . . . . . .

                    // get V9 height map
                    for (int x = 0; x <= 8; x++)
                    {
                        int zx = cx * 8 + x;
                        for (int y = 0; y <= 8; y++)
                        {
                            int zy = cy * 8 + y;
                            tileSpreadInfo.V9[zx][zy] = _chunks[cx][cy].pointHeights[x * (8 * 2 + 1) + y]+_chunks[cx][cy].mapHeight;
                            tileSpreadInfo.liquid_height[zx][zy] = _chunks[cx][cy].liquid_height[x][y];
                            tileSpreadInfo.liquid_exists[zx][zy] = _chunks[cx][cy].show_liquid[x][y];
                        }
                    }

                    // get V8 height map
                    for (int x = 0; x < 8; x++)
                    {
                        int zx = cx * 8 + x;
                        for (int y = 0; y < 8; y++)
                        {
                            int zy = cy * 8 + y;
                            tileSpreadInfo.V8[zx][zy] = _chunks[cx][cy].pointHeights[y * (8 * 2 + 1) + 8 + 1 + x]+_chunks[cx][cy].mapHeight;
                        }
                    }
                }
            }

            fwrite(&tileSpreadInfo.V9, sizeof(float)*V9_SIZE_SQ, 1, output);
            fwrite(&tileSpreadInfo.V8, sizeof(float)*V8_SIZE_SQ, 1, output);
            fwrite(&tileSpreadInfo.liquid_height, sizeof(float)*V9_SIZE_SQ, 1, output);
        }break;
    case MAP_RAW_CHUNKS:
        {
            MapTileSpread tileSpreadInfo;

            // Pull our area and liquid info into our spread
            for(uint8 cx = 0; cx < CHUNK_SIZE; cx++)
            {
                for(uint8 cy = 0; cy < CHUNK_SIZE; cy++)
                {
                    uint16 offset = cx*CHUNK_SIZE+cy;
                    tileSpreadInfo.AreaInfo[offset] = _chunks[cx][cy].areaEntry;
                    tileSpreadInfo.LiquidInfo[offset] = _chunks[cx][cy].liquidEntry;
                }
            }

            // Append area info and liquid info
            fwrite(&tileSpreadInfo.AreaInfo, sizeof(uint16)*CHUNK_SIZE_SQ, 1, output);
            fwrite(&tileSpreadInfo.LiquidInfo, sizeof(uint16)*CHUNK_SIZE_SQ, 1, output);

            // Feed chunk data into tile spread info
            for(uint8 cx = 0; cx < CHUNK_SIZE; cx++)
            {
                for(uint8 cy = 0; cy < CHUNK_SIZE; cy++)
                {
                    // Parse liquid data first
                    bool hasLiqHeight = false;
                    for (int x = 0; x <= 8; x++)
                    {
                        for (int y = 0; y <= 8; y++)
                        {
                            if(_chunks[cx][cy].liquid_height[x][y] == -50000.f)
                                continue;
                            hasLiqHeight = true;
                        }
                    }

                    // Height values for triangles stored in order:
                    // 1     2     3     4     5     6     7     8     9
                    //    10    11    12    13    14    15    16    17
                    // 18    19    20    21    22    23    24    25    26
                    //    27    28    29    30    31    32    33    34
                    // . . . . . . . .
                    // For better get height values merge it to V9 and V8 map
                    // V9 height map:
                    // 1     2     3     4     5     6     7     8     9
                    // 18    19    20    21    22    23    24    25    26
                    // . . . . . . . .
                    // V8 height map:
                    //    10    11    12    13    14    15    16    17
                    //    27    28    29    30    31    32    33    34
                    // . . . . . . . .

                    // get V9 height map
                    for (int x = 0; x <= 8; x++)
                    {
                        int zx = cx * 8 + x;
                        for (int y = 0; y <= 8; y++)
                        {
                            int zy = cy * 8 + y;
                            tileSpreadInfo.V9[zx][zy] = _chunks[cx][cy].pointHeights[x * (8 * 2 + 1) + y]+_chunks[cx][cy].mapHeight;
                            tileSpreadInfo.liquid_height[zx][zy] = _chunks[cx][cy].liquid_height[x][y];
                            tileSpreadInfo.liquid_exists[zx][zy] = _chunks[cx][cy].show_liquid[x][y];
                        }
                    }

                    // get V8 height map
                    for (int x = 0; x < 8; x++)
                    {
                        int zx = cx * 8 + x;
                        for (int y = 0; y < 8; y++)
                        {
                            int zy = cy * 8 + y;
                            tileSpreadInfo.V8[zx][zy] = _chunks[cx][cy].pointHeights[y * (8 * 2 + 1) + 8 + 1 + x]+_chunks[cx][cy].mapHeight;
                        }
                    }
                }
            }

            fwrite(&tileSpreadInfo.V9, sizeof(float)*V9_SIZE_SQ, 1, output);
            fwrite(&tileSpreadInfo.V8, sizeof(float)*V8_SIZE_SQ, 1, output);
            fwrite(&tileSpreadInfo.liquid_height, sizeof(float)*V9_SIZE_SQ, 1, output);
        }break;
    case MAP_COMPRESSED_TILES:
        {
            MapTileSpread tileSpreadInfo;
            uint8 uint8_V8[128][128], uint8_V9[129][129];
            uint16 uint16_V8[128][128], uint16_V9[129][129];
            float float_V8[128][128], float_V9[129][129];
            memset(&uint8_V8, 0, sizeof(uint8)*128*128);
            memset(&uint8_V9, 0, sizeof(uint8)*129*129);
            memset(&uint16_V8, 0, sizeof(uint16)*128*128);
            memset(&uint16_V9, 0, sizeof(uint16)*129*129);
            memset(&float_V8, 0, sizeof(float)*128*128);
            memset(&float_V9, 0, sizeof(float)*129*129);

            // Pull our area and liquid info into our spread
            for(uint8 cx = 0; cx < CHUNK_SIZE; cx++)
            {
                for(uint8 cy = 0; cy < CHUNK_SIZE; cy++)
                {
                    uint16 offset = cx*CHUNK_SIZE+cy;
                    tileSpreadInfo.AreaInfo[offset] = _chunks[cx][cy].areaEntry;
                    tileSpreadInfo.LiquidInfo[offset] = _chunks[cx][cy].liquidEntry;
                    tileSpreadInfo.liquid_mask[offset] = _chunks[cx][cy].liquidMask;
                }
            }

            // Append area info and liquid info
            fwrite(&tileSpreadInfo.AreaInfo, sizeof(uint16)*CHUNK_SIZE_SQ, 1, output);
            fwrite(&tileSpreadInfo.LiquidInfo, sizeof(uint16)*CHUNK_SIZE_SQ, 1, output);
            fwrite(&tileSpreadInfo.liquid_mask, sizeof(uint64)*CHUNK_SIZE_SQ, 1, output);

            // Feed chunk data into tile spread info
            for(uint8 cx = 0; cx < CHUNK_SIZE; cx++)
            {
                for(uint8 cy = 0; cy < CHUNK_SIZE; cy++)
                {
                    // Parse liquid data first
                    bool hasLiqHeight = false;
                    for (int x = 0; x <= 8; x++)
                    {
                        for (int y = 0; y <= 8; y++)
                        {
                            if(_chunks[cx][cy].liquid_height[x][y] == -50000.f)
                                continue;
                            hasLiqHeight = true;
                        }
                    }

                    // Height values for triangles stored in order:
                    // 1     2     3     4     5     6     7     8     9
                    //    10    11    12    13    14    15    16    17
                    // 18    19    20    21    22    23    24    25    26
                    //    27    28    29    30    31    32    33    34
                    // . . . . . . . .
                    // For better get height values merge it to V9 and V8 map
                    // V9 height map:
                    // 1     2     3     4     5     6     7     8     9
                    // 18    19    20    21    22    23    24    25    26
                    // . . . . . . . .
                    // V8 height map:
                    //    10    11    12    13    14    15    16    17
                    //    27    28    29    30    31    32    33    34
                    // . . . . . . . .

                    // get V9 height map
                    for (int x = 0; x <= 8; x++)
                    {
                        int zx = cx * 8 + x;
                        for (int y = 0; y <= 8; y++)
                        {
                            int zy = cy * 8 + y;
                            tileSpreadInfo.V9[zx][zy] = _chunks[cx][cy].pointHeights[x * (8 * 2 + 1) + y]+_chunks[cx][cy].mapHeight;
                            tileSpreadInfo.liquid_height[zx][zy] = _chunks[cx][cy].liquid_height[x][y];
                            tileSpreadInfo.liquid_exists[zx][zy] = _chunks[cx][cy].show_liquid[x][y];
                        }
                    }

                    // get V8 height map
                    for (int x = 0; x < 8; x++)
                    {
                        int zx = cx * 8 + x;
                        for (int y = 0; y < 8; y++)
                        {
                            int zy = cy * 8 + y;
                            tileSpreadInfo.V8[zx][zy] = _chunks[cx][cy].pointHeights[y * (8 * 2 + 1) + 8 + 1 + x]+_chunks[cx][cy].mapHeight;
                        }
                    }
                }
            }

            uint8 compressionFlags = 0;
            float step=0, minHeight=50000.f, maxHeight=-50000.f;
            for(uint8 x = 0; x <= 128; x++)
            {
                for(uint8 y = 0; y <= 128; y++)
                {
                    float height = tileSpreadInfo.V9[x][y];
                    if(height < minHeight)
                        minHeight = height;
                    if(height > maxHeight)
                        maxHeight = height;
                    if(x == 128)
                        continue;

                    height = tileSpreadInfo.V8[x][y];
                    if(height < minHeight)
                        minHeight = height;
                    if(height > maxHeight)
                        maxHeight = height;
                }
            }

            fwrite(&minHeight, sizeof(float), 1, output);
            // Since we have the minimum height, just grab the diff of the current height minus minimum
            for(uint8 x = 0; x <= 128; x++)
            {
                for(uint8 y = 0; y <= 128; y++)
                    tileSpreadInfo.V9[x][y] -= minHeight;
                if(x == 128)
                    continue;
                for(uint8 y = 0; y < 128; y++)
                    tileSpreadInfo.V8[x][y] -= minHeight;
            }

            float diff = maxHeight - minHeight;
            if(diff < 0.01f)
            {
                compressionFlags = 0x01;
                fwrite(&compressionFlags, sizeof(uint8), 1, output);
            }
            else
            {
                if (diff < 2.0f)
                {
                    compressionFlags|=MAP_HEIGHT_AS_INT8;
                    step = selectUInt8StepStore(diff, false);
                }
                else if (diff<2048.0f)
                {
                    compressionFlags|=MAP_HEIGHT_AS_INT16;
                    step = selectUInt16StepStore(diff, false);
                }

                // get V9 height map
                for (int y = 0; y <= 128; y++)
                {
                    for (int x = 0; x <= 128; x++)
                    {
                        float height = tileSpreadInfo.V9[y][x];
                        if(compressionFlags & MAP_HEIGHT_AS_INT8)
                            uint8_V9[y][x] = uint8(height * step + 0.5f);
                        else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                            uint16_V9[y][x] = uint16(height * step + 0.5f);
                        else float_V9[y][x] = height;
                    }
                }

                // get V8 height map
                for (int y = 0; y < 128; y++)
                {
                    for (int x = 0; x < 128; x++)
                    {
                        float height = tileSpreadInfo.V8[y][x];
                        if(compressionFlags & MAP_HEIGHT_AS_INT8)
                            uint8_V8[y][x] = uint8(height * step + 0.5f);
                        else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                            uint16_V8[y][x] = uint16(height * step + 0.5f);
                        else float_V8[y][x] = height;
                    }
                }

                // Write compression flags and height data
                fwrite(&compressionFlags, sizeof(uint8), 1, output);
                if(compressionFlags & MAP_HEIGHT_AS_INT8)
                {
                    step = diff / 255; // Update our step to our multiplier
                    fwrite(uint8_V8, sizeof(uint8)*128*128, 1, output);
                    fwrite(uint8_V9, sizeof(uint8)*9*9, 1, output);
                    fwrite(&step, sizeof(float), 1, output);
                }
                else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                {
                    step = diff / 65535; // Update our step to our multiplier
                    fwrite(uint16_V8, sizeof(uint16)*128*128, 1, output);
                    fwrite(uint16_V9, sizeof(uint16)*9*9, 1, output);
                    fwrite(&step, sizeof(float), 1, output);
                }
                else
                {
                    // Uncompressed
                    fwrite(float_V8, sizeof(float)*128*128, 1, output);
                    fwrite(float_V9, sizeof(float)*129*129, 1, output);
                }
            }

            compressionFlags = 0;
            step=0, minHeight=50000.f, maxHeight=-50000.f;
            memset(&uint8_V9, 0, sizeof(uint8)*129*129);
            memset(&uint16_V9, 0, sizeof(uint16)*129*129);
            memset(&float_V9, 0, sizeof(float)*129*129);

            bool hasHeight = false;
            for(uint8 x = 0; x <= 128; x++)
            {
                for(uint8 y = 0; y <= 128; y++)
                {
                    float height = tileSpreadInfo.liquid_height[x][y];
                    if(height == -50000.f)
                        continue;

                    hasHeight = true;
                    if(height < minHeight)
                        minHeight = height;
                    if(height > maxHeight)
                        maxHeight = height;
                }
            }

            for(uint8 x = 0; x <= 128; x++)
            {
                for(uint8 y = 0; y <= 128; y++)
                {
                    if(tileSpreadInfo.liquid_height[x][y] == -50000.f)
                        continue;
                    tileSpreadInfo.liquid_height[x][y] -= minHeight;
                }
            }

            if(hasHeight == false)
            {
                compressionFlags = 0xFF;
                fwrite(&compressionFlags, sizeof(uint8), 1, output);
            }
            else
            {
                float diff = maxHeight - minHeight;
                if(diff < 0.01f)
                {
                    compressionFlags = 0x01;
                    fwrite(&compressionFlags, sizeof(uint8), 1, output);
                    fwrite(&minHeight, sizeof(float), 1, output);
                }
                else
                {
                    if (diff < 2.0f)
                    {
                        compressionFlags|=MAP_HEIGHT_AS_INT8;
                        step = selectUInt8StepStore(diff, true);
                    }
                    else if (diff<1024.0f)
                    {
                        compressionFlags|=MAP_HEIGHT_AS_INT16;
                        step = selectUInt16StepStore(diff, true);
                    }

                    for(uint8 x = 0; x <= 128; x++)
                    {
                        for(uint8 y = 0; y <= 128; y++)
                        {
                            float height = tileSpreadInfo.liquid_height[x][y];
                            if(height == 0.f || height == -50000.f)
                                continue;
                            if(compressionFlags & MAP_HEIGHT_AS_INT8)
                                uint8_V9[y][x] = (height == -50000.f ? 0xFF : uint8(height * step + 0.5f));
                            else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                                uint16_V9[y][x] = (height == -50000.f ? 0xFFFF : uint16(height * step + 0.5f));
                            else float_V9[y][x] = height;

                            hasHeight = true;
                            if(height < minHeight)
                                minHeight = height;
                            if(height > maxHeight)
                                maxHeight = height;
                        }
                    }

                    // Write compression flags and height data
                    fwrite(&compressionFlags, sizeof(uint8), 1, output);
                    fwrite(&minHeight, sizeof(float), 1, output);
                    if(compressionFlags & MAP_HEIGHT_AS_INT8)
                    {
                        step = diff / 0x7F; // Update our step to our multiplier
                        fwrite(uint8_V9, sizeof(uint8)*129*129, 1, output);
                        fwrite(&step, sizeof(float), 1, output);
                    }
                    else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                    {
                        step = diff / 0x7FFF; // Update our step to our multiplier
                        fwrite(uint16_V9, sizeof(uint16)*129*129, 1, output);
                        fwrite(&step, sizeof(float), 1, output);
                    } else fwrite(float_V9, sizeof(float)*129*129, 1, output); // Uncompressed
                }
            }
        }break;
    case MAP_COMPRESSED_CHUNKS:
        {
            uint8 uint8_V8[8][8], uint8_V9[9][9];
            uint16 uint16_V8[8][8], uint16_V9[9][9];
            float float_V8[8][8], float_V9[9][9];

            // Parse and write our chunk data with compression
            for(uint8 cx = 0; cx < 16; cx++)
            {
                for(uint8 cy = 0; cy < 16; cy++)
                {
                    chunk *curChunk = &_chunks[cx][cy];
                    memset(&uint8_V8, 0, sizeof(uint8)*8*8);
                    memset(&uint8_V9, 0, sizeof(uint8)*9*9);
                    memset(&uint16_V8, 0, sizeof(uint16)*8*8);
                    memset(&uint16_V9, 0, sizeof(uint16)*9*9);
                    memset(&float_V8, 0, sizeof(float)*8*8);
                    memset(&float_V9, 0, sizeof(float)*9*9);

                    /////// Chunk begin
                    fwrite(&cx, sizeof(uint8), 1, output);
                    fwrite(&cy, sizeof(uint8), 1, output);
                    fwrite(&curChunk->areaEntry, sizeof(uint16), 1, output);

                    uint8 compressionFlags = 0;
                    float step=0, minHeight=50000.f, maxHeight=-50000.f;
                    for(uint8 i = 0; i < 145; i++)
                    {
                        float height = _chunks[cx][cy].pointHeights[i]+_chunks[cx][cy].mapHeight;
                        if(height == 0.f)
                            continue;

                        if(height < minHeight)
                            minHeight = height;
                        if(height > maxHeight)
                            maxHeight = height;
                    }

                    fwrite(&minHeight, sizeof(float), 1, output);
                    // Recalculate each height point based on the minimum height
                    for(uint8 i = 0; i < 145; i++)
                    {
                        float height = _chunks[cx][cy].pointHeights[i]+_chunks[cx][cy].mapHeight;
                        // Since we have the minimum height, just grab the diff of the current height minus minimum
                        _chunks[cx][cy].pointHeights[i] = height-minHeight;
                    }

                    float diff = maxHeight - minHeight;
                    if(diff < 0.01f)
                    {
                        compressionFlags = 0x01;
                        fwrite(&compressionFlags, sizeof(uint8), 1, output);
                    }
                    else
                    {
                        if (diff < 2.0f)
                        {
                            compressionFlags|=MAP_HEIGHT_AS_INT8;
                            step = selectUInt8StepStore(diff, false);
                        }
                        else if (diff<2048.0f)
                        {
                            compressionFlags|=MAP_HEIGHT_AS_INT16;
                            step = selectUInt16StepStore(diff, false);
                        }

                        // Height values for triangles stored in order:
                        // 1     2     3     4     5     6     7     8     9
                        //    10    11    12    13    14    15    16    17
                        // 18    19    20    21    22    23    24    25    26
                        //    27    28    29    30    31    32    33    34
                        // . . . . . . . .
                        // For better get height values merge it to V9 and V8 map
                        // V9 height map:
                        // 1     2     3     4     5     6     7     8     9
                        // 18    19    20    21    22    23    24    25    26
                        // . . . . . . . .
                        // V8 height map:
                        //    10    11    12    13    14    15    16    17
                        //    27    28    29    30    31    32    33    34
                        // . . . . . . . .
                        // get V9 height map
                        for (int y = 0; y <= 8; y++)
                        {
                            for (int x = 0; x <= 8; x++)
                            {
                                float height = _chunks[cx][cy].pointHeights[y * (8 * 2 + 1) + x];
                                if(compressionFlags & MAP_HEIGHT_AS_INT8)
                                    uint8_V9[y][x] = uint8(height * step + 0.5f);
                                else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                                    uint16_V9[y][x] = uint16(height * step + 0.5f);
                                else float_V9[y][x] = height;
                            }
                        }
                        // get V8 height map
                        for (int y = 0; y < 8; y++)
                        {
                            for (int x = 0; x < 8; x++)
                            {
                                float height = _chunks[cx][cy].pointHeights[y * (8 * 2 + 1) + 8 + 1 + x];
                                if(compressionFlags & MAP_HEIGHT_AS_INT8)
                                    uint8_V8[y][x] = uint8(height * step + 0.5f);
                                else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                                    uint16_V8[y][x] = uint16(height * step + 0.5f);
                                else float_V8[y][x] = height;
                            }
                        }

                        // Write compression flags and height data
                        fwrite(&compressionFlags, sizeof(uint8), 1, output);
                        if(compressionFlags & MAP_HEIGHT_AS_INT8)
                        {
                            step = diff / 255; // Update our step to our multiplier
                            fwrite(uint8_V8, sizeof(uint8)*8*8, 1, output);
                            fwrite(uint8_V9, sizeof(uint8)*9*9, 1, output);
                            fwrite(&step, sizeof(float), 1, output);
                        }
                        else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                        {
                            step = diff / 65535; // Update our step to our multiplier
                            fwrite(uint16_V8, sizeof(uint16)*8*8, 1, output);
                            fwrite(uint16_V9, sizeof(uint16)*9*9, 1, output);
                            fwrite(&step, sizeof(float), 1, output);
                        }
                        else
                        {
                            // Uncompressed
                            fwrite(float_V8, sizeof(float)*8*8, 1, output);
                            fwrite(float_V9, sizeof(float)*9*9, 1, output);
                        }
                    }
                    fwrite(&_chunks[cx][cy].holes, sizeof(uint32), 1, output);
                    fwrite(&_chunks[cx][cy].liquidEntry, sizeof(uint16), 1, output);

                    compressionFlags = 0;
                    step=0, minHeight=50000.f, maxHeight=-50000.f;
                    memset(&uint8_V9, 0, sizeof(uint8)*9*9);
                    memset(&uint16_V9, 0, sizeof(uint16)*9*9);
                    memset(&float_V9, 0, sizeof(float)*9*9);

                    bool hasLiqHeight = false;
                    // grab our min height and refactor height map to match
                    for (int x = 0; x <= 8; x++)
                    {
                        for (int y = 0; y <= 8; y++)
                        {
                            if(_chunks[cx][cy].liquid_height[x][y] == -50000.f)
                                continue;

                            hasLiqHeight = true;
                            float height = _chunks[cx][cy].liquid_height[x][y];
                            if(height < minHeight)
                                minHeight = height;
                            if(height > maxHeight)
                                maxHeight = height;
                        }
                    }

                    if(hasLiqHeight == false)
                    {
                        compressionFlags = 0xFF;
                        fwrite(&compressionFlags, sizeof(uint8), 1, output);
                    }
                    else
                    {
                        float diff = maxHeight - minHeight;
                        if(diff < 0.01f)
                        {
                            compressionFlags = 0x01;
                            fwrite(&compressionFlags, sizeof(uint8), 1, output);
                            fwrite(&minHeight, sizeof(float), 1, output);
                            fwrite(&_chunks[cx][cy].liquidMask, sizeof(uint64), 1, output);
                        }
                        else
                        {
                            if (diff < 2.0f)
                            {
                                compressionFlags|=MAP_HEIGHT_AS_INT8;
                                step = selectUInt8StepStore(diff, true);
                            }
                            else if (diff<1024.0f)
                            {
                                compressionFlags|=MAP_HEIGHT_AS_INT16;
                                step = selectUInt16StepStore(diff, true);
                            }

                            // get V9 height map
                            for (int y = 0; y <= 8; y++)
                            {
                                for (int x = 0; x <= 8; x++)
                                {
                                    float height = _chunks[cx][cy].liquid_height[x][y];
                                    if(compressionFlags & MAP_HEIGHT_AS_INT8)
                                        uint8_V9[y][x] = (height == -50000.f ? 0xFF : uint8(height * step + 0.5f));
                                    else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                                        uint16_V9[y][x] = (height == -50000.f ? 0xFFFF : uint16(height * step + 0.5f));
                                    else float_V9[y][x] = height;
                                }
                            }

                            // Write compression flags and height data
                            fwrite(&compressionFlags, sizeof(uint8), 1, output);
                            fwrite(&minHeight, sizeof(float), 1, output);
                            if(compressionFlags & MAP_HEIGHT_AS_INT8)
                            {
                                step = diff / 0x7F; // Update our step to our multiplier
                                fwrite(uint8_V9, sizeof(uint8)*9*9, 1, output);
                                fwrite(&step, sizeof(float), 1, output);
                            }
                            else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                            {
                                step = diff / 0x7FFF; // Update our step to our multiplier
                                fwrite(uint16_V9, sizeof(uint16)*9*9, 1, output);
                                fwrite(&step, sizeof(float), 1, output);
                            } else fwrite(float_V9, sizeof(float)*9*9, 1, output); // Uncompressed
                        }
                    }

                    /////// Chunk end
                }
            }
        }break;
    }
}

void ADTFile::parseWMO(uint32 map_num, MapCreationInfo *mapInfo, uint32 tileX, uint32 tileY)
{
    if(ADT.isEof ())
        return;

    uint32 size;
    while (!ADT.isEof())
    {
        char fourcc[5];
        ADT.read(&fourcc,4);
        ADT.read(&size, 4);
        flipcc(fourcc);
        fourcc[4] = 0;

        size_t nextpos = ADT.getPos() + size;

        if(size)
        {
            if (!strcmp(fourcc,"MMDX"))
            {
                char* buf = new char[size];
                ADT.read(buf, size);
                char* p = buf;
                int t = 0;
                while (p < buf + size)
                {
                    std::string path(p);

                    char* s = GetPlainName(p);
                    FixNameCase(s, strlen(s));
                    FixNameSpaces(s, strlen(s));

                    ModelInstanceNameMap.insert(std::make_pair(t++, new string(s)));
                    ExtractSingleModel(path);
                    p += strlen(p) + 1;
                }
                delete[] buf;
            }
            else if (!strcmp(fourcc,"MWMO"))
            {
                char* buf = new char[size];
                ADT.read(buf, size);
                char* p = buf;
                int q = 0;
                while (p < buf + size)
                {
                    char* s = GetPlainName(p);
                    FixNameCase(s, strlen(s));
                    FixNameSpaces(s, strlen(s));

                    WMOInstanceNameMap.insert(std::make_pair(q++, new string(s)));
                    p += strlen(p) + 1;
                }
                delete[] buf;
            }
            //======================
            else if (!strcmp(fourcc,"MDDF"))
            {
                nMDX = (int)size / 36;
                for (int i=0; i<nMDX; ++i)
                {
                    uint32 id;
                    ADT.read(&id, 4);
                    ModelInstance inst(ADT, mapInfo, ModelInstanceNameMap[id]->c_str(), map_num, tileX, tileY);
                }
                for(std::map<uint32, std::string*>::iterator itr = ModelInstanceNameMap.begin(); itr != ModelInstanceNameMap.end(); itr++)
                    delete itr->second;
                ModelInstanceNameMap.clear();
            }
            else if (!strcmp(fourcc,"MODF"))
            {
                nWMO = (int)size / 64;
                for (int i=0; i<nWMO; ++i)
                {
                    uint32 id;
                    ADT.read(&id, 4);
                    WMOInstance inst(ADT, mapInfo, WMOInstanceNameMap[id]->c_str(), true, map_num, tileX, tileY);
                }
                for(std::map<uint32, std::string*>::iterator itr = WMOInstanceNameMap.begin(); itr != WMOInstanceNameMap.end(); itr++)
                    delete itr->second;
                WMOInstanceNameMap.clear();
            }
        }

        //======================
        ADT.seek(nextpos);
    }

    ADT.close();
}

ADTFile::~ADTFile()
{
    ADT.close();
}
