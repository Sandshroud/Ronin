/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

/*
    static float tileSize = 533.33333f, chunkSize = tileSize/16.f, chunkStep = chunkSize/9.f, halfStep = chunkStep/2.f;
    std::vector<triangle> triangles;

    for(uint8 x = 0; x < 8; x++)
    {
        for(uint8 y = 0; y < 8; y++)
        {
            float centerX = header.zpos+(chunkStep*x)+halfStep, centerY = header.xpos+(chunkStep*y)+halfStep;
            // Calculate triangle Top
            triangle tTop;
            // X Axis - Different X axis
            tTop.corners[0].x = centerX - halfStep;
            tTop.corners[1].x = centerX + halfStep;
            tTop.corners[2].x = centerX;
            // Y Axis - Same Y axis
            tTop.corners[0].y = centerY - halfStep;
            tTop.corners[1].y = centerY - halfStep;
            tTop.corners[2].y = centerY;
            // Height
            tTop.corners[0].z = header.ypos+V9[x][y]; // Top left
            tTop.corners[1].z = header.ypos+V9[x+1][y]; // Top right
            tTop.corners[2].z = header.ypos+V8[x][y]; // Center
            // Push our triangle to storage
            triangles.push_back(tTop);

            // Calculate triangle Left
            triangle tLeft;
            // X Axis - Same X Axis
            tLeft.corners[0].x = centerX - halfStep;
            tLeft.corners[1].x = centerX - halfStep;
            tLeft.corners[2].x = centerX;
            // Y Axis - Different Y Axis
            tLeft.corners[0].y = centerY - halfStep;
            tLeft.corners[1].y = centerY + halfStep;
            tLeft.corners[2].y = centerY;
            // Height
            tLeft.corners[0].z = header.ypos+V9[x][y]; // Top left
            tLeft.corners[1].z = header.ypos+V9[x][y+1]; // Bottom left
            tLeft.corners[2].z = header.ypos+V8[x][y]; // Center
            // Push our triangle to storage
            triangles.push_back(tLeft);

            // Calculate triangle Right
            triangle tRight;
            // X Axis - Same X Axis
            tRight.corners[0].x = centerX + halfStep;
            tRight.corners[1].x = centerX + halfStep;
            tRight.corners[2].x = centerX;
            // Y Axis - Differeny Y Axis
            tRight.corners[0].y = centerY - halfStep;
            tRight.corners[1].y = centerY + halfStep;
            tRight.corners[2].y = centerY;
            // Height
            tRight.corners[0].z = header.ypos+V9[x+1][y]; // Top right
            tRight.corners[1].z = header.ypos+V9[x+1][y+1]; // bottom right
            tRight.corners[2].z = header.ypos+V8[x][y]; // Center
            // Push our triangle to storage
            triangles.push_back(tRight);

            // Calculate triangle Bottom
            triangle tBottom;
            // X Axis - Different X Axis
            tBottom.corners[0].x = centerX - halfStep;
            tBottom.corners[1].x = centerX + halfStep;
            tBottom.corners[2].x = centerX;
            // Y Axis - Same Y Axis
            tBottom.corners[0].y = centerY + halfStep;
            tBottom.corners[1].y = centerY + halfStep;
            tBottom.corners[2].y = centerY;
            // Height
            tBottom.corners[0].z = header.ypos+V9[x][y+1]; // Bottom left
            tBottom.corners[1].z = header.ypos+V9[x+1][y+1]; // Bottom right
            tBottom.corners[2].z = header.ypos+V8[x][y]; // Center
            // Push our triangle to storage
            triangles.push_back(tBottom);
        }
    }
*/

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
    float  heightLevel1;
    float  heightLevel2;
    uint8  xOffset;
    uint8  yOffset;
    uint8  width;
    uint8  height;
    uint32 offsData2a;
    uint32 offsData2b;
};

struct adt_MCVT { float height_map[145]; };

float selectUInt8StepStore(float maxDiff) { return 255 / maxDiff; }
float selectUInt16StepStore(float maxDiff) { return 65535 / maxDiff; }

extern uint16 *LiqType;

struct chunk
{
    uint16 areaEntry;
    float mapHeight, pointHeights[145];
    uint32 holes;

    uint16 liquidEntry;
    float liquidHeight, L9[9*9];
    bool L8[8*8];
};

bool ADTFile::parseCHNK(FILE *output)
{
    if(ADT.isEof ())
        return false;

    chunk _chunks[16][16];
    memset(_chunks, 0, sizeof(chunk)*16*16);
    uint32 MCLQChunkOffsets[16][16], MH2OChunkOffsets[16][16];
    memset(MCLQChunkOffsets, 0, sizeof(MCLQChunkOffsets));
    memset(MH2OChunkOffsets, 0, sizeof(MH2OChunkOffsets));

    uint32 size = 0, adtVersion = 0, mh2oBase = 0;
    while (!ADT.isEof())
    {
        char fourcc[5];
        ADT.read(&fourcc,4);
        ADT.read(&size, 4);
        flipcc(fourcc);
        fourcc[4] = 0;

        size_t currPos = ADT.getPos(), nextpos = currPos + size;
        if(size)
        {
            if (!strcmp(fourcc,"MVER"))
                ADT.read(&adtVersion, sizeof(uint32));
            else if (!strcmp(fourcc,"MHDR"))
            { }// MHDR is after version header, contains offset data for parsing the ADT file
            else if (!strcmp(fourcc,"MCNK"))
            {
                MapChunkHeader header;
                ADT.read(&header, sizeof(MapChunkHeader));
                _chunks[header.iy][header.ix].areaEntry = header.areaid;
                _chunks[header.iy][header.ix].mapHeight = header.ypos;
                _chunks[header.iy][header.ix].holes = header.holes;
                if(header.offsMCVT)
                {
                    ADT.seek(currPos+header.offsMCVT);
                    ADT.read(_chunks[header.iy][header.ix].pointHeights, 145*sizeof(float));
                }

                if(header.flags & 0x04)
                    _chunks[header.iy][header.ix].liquidEntry |= MAP_LIQUID_TYPE_WATER;
                if(header.flags & 0x08)
                    _chunks[header.iy][header.ix].liquidEntry |= MAP_LIQUID_TYPE_OCEAN;
                if(header.flags & 0x10)
                    _chunks[header.iy][header.ix].liquidEntry |= MAP_LIQUID_TYPE_MAGMA;
                if(header.flags & 0x20)
                    _chunks[header.iy][header.ix].liquidEntry |= MAP_LIQUID_TYPE_SLIME;
                MCLQChunkOffsets[header.iy][header.ix] = (header.sizeMCLQ > 8 && header.offsMCLQ ? currPos+header.offsMCLQ : 0);
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
            for(uint8 x = 0; x < 9; x++)
                for(uint8 y = 0; y < 9; y++)
                    _chunks[cx][cy].L9[x*9+y]=-50000.f;

            // MH2O chunks take priority over MCLQ so we handle them second in case of overrides
            if(mh2oBase && MH2OChunkOffsets[cx][cy])
            {
                ADT.seek(mh2oBase+MH2OChunkOffsets[cx][cy]);
                MH2O_liquid_header mh2oHeader;
                ADT.read(&mh2oHeader, sizeof(MH2O_liquid_header));
                uint64 mask = 0xFFFFFFFFFFFFFFFF;
                if(mh2oHeader.offsData2a)
                {
                    ADT.seek(mh2oBase+mh2oHeader.offsData2a);
                    ADT.read(&mask, sizeof(uint64));
                }

                bool hasMask = (mask != 0), readHeight = false;
                if(readHeight = ((mh2oHeader.formatFlags&0x02) == 0 && mh2oHeader.offsData2b > 0))
                    ADT.seek(mh2oBase+mh2oHeader.offsData2b);

                // Parse the 64bit liquid mask
                for(uint8 x = 0; x < 8; x++)
                {
                    for(uint8 y = 0; y < 8; y++)
                    {
                        if(mask & 0x01)
                            _chunks[cx][cy].L8[y*8+x] = true;
                        mask>>=1;
                    }
                }

                // Parse our liquid height data
                _chunks[cx][cy].liquidHeight = hasMask ? mh2oHeader.heightLevel1 : -50000.f;
                if(readHeight)
                {
                    for(uint8 y = 0; y <= mh2oHeader.height; y++)
                    {
                        uint8 ci = y+mh2oHeader.yOffset;
                        for(uint8 x = 0; x <= mh2oHeader.width; x++)
                        {
                            uint8 cj = x+mh2oHeader.xOffset;
                            ADT.read(&_chunks[cx][cy].L9[ci*9+cj], sizeof(float));
                            // Subtract the height to get the diff from our base height
                            _chunks[cx][cy].L9[ci*9+cj] -= mh2oHeader.heightLevel1;
                        }
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
            else if(MCLQChunkOffsets[cx][cy]) // Pre wotlk water chunks
            {
                ADT.seek(MCLQChunkOffsets[cx][cy]);
                MCLQinformation mclqChunk;
                ADT.read(&mclqChunk, sizeof(MCLQinformation));

                for(uint8 x = 0; x < 9; x++)
                {
                    for(uint8 y = 0; y < 9; y++)
                    {
                        _chunks[cx][cy].L9[y*9+x] = mclqChunk.liquid[x][y].height;
                        if(x == 8 || y == 8)
                            continue;
                        if(mclqChunk.flags[x][y] != 0x0F)
                        {
                            _chunks[cx][cy].L8[y*8+x] = true;
                            if(mclqChunk.flags[x][y] & (1<<7))
                                _chunks[cx][cy].liquidEntry |= MAP_LIQUID_TYPE_DARK_WATER;
                        }
                    }
                }
            }
        }
    }

    uint8 uint8_V8[8][8], uint8_V9[9][9];
    uint16 uint16_V8[8][8], uint16_V9[9][9];
    float float_V8[8][8], float_V9[9][9];

    // Parse and write our chunk data with compression
    for(uint8 cx = 0; cx < 16; cx++)
    {
        for(uint8 cy = 0; cy < 16; cy++)
        {
            /////// Chunk begin
            fwrite(&_chunks[cx][cy].areaEntry, sizeof(uint16), 1, output);

            uint8 compressionFlags = 0;
            float step=0, minHeight=50000.f, maxHeight=-50000.f;
            for(uint8 i = 0; i < 145; i++)
            {
                float height = _chunks[cx][cy].pointHeights[i]+_chunks[cx][cy].mapHeight;
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
                    step = selectUInt8StepStore(diff);
                }
                else if (diff<2048.0f)
                {
                    compressionFlags|=MAP_HEIGHT_AS_INT16;
                    step = selectUInt16StepStore(diff);
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
            for(uint8 x = 0; x <= 8; x++)
            {
                for(uint8 y = 0; y <= 8; y++)
                {
                    if(y <= 8 && x > 0 && _chunks[cx][cy].L8[(x-1)*8+y])
                        continue;
                    if(y > 0 && x <= 8 && _chunks[cx][cy].L8[x*8+(y-1)])
                        continue;
                    if(y > 0 && x > 0 && _chunks[cx][cy].L8[(x-1)*8+(y-1)])
                        continue;
                    if(y <= 8 && x <= 8 && _chunks[cx][cy].L8[x*8+y])
                        continue;
                    _chunks[cx][cy].L9[x*9+y] = -50000.f;
                }
            }

            bool hasHeight = false;
            for(uint8 i = 0; i < 81; i++)
            {
                if(_chunks[cx][cy].L9[i] == -50000.f)
                    continue;
                hasHeight = true;
                float height = _chunks[cx][cy].liquidHeight+_chunks[cx][cy].L9[i];
                if(height < minHeight)
                    minHeight = height;
                if(height > maxHeight)
                    maxHeight = height;
            }

            if(hasHeight == false)
            {
                if(_chunks[cx][cy].liquidHeight != -50000.f)
                {
                    compressionFlags = 0x01;
                    fwrite(&compressionFlags, sizeof(uint8), 1, output);
                    fwrite(&_chunks[cx][cy].liquidHeight, sizeof(float), 1, output);
                }
                else
                {
                    compressionFlags = 0xFF;
                    fwrite(&compressionFlags, sizeof(uint8), 1, output);
                }
            }
            else
            {
                // Recalculate each height point based on the minimum height
                for(uint8 i = 0; i < 81; i++)
                {
                    if(_chunks[cx][cy].L9[i] == -50000.f)
                        continue;

                    float height = _chunks[cx][cy].liquidHeight+_chunks[cx][cy].L9[i];
                    // Since we have the minimum height, just grab the diff of the current height minus minimum
                    _chunks[cx][cy].L9[i] = height-minHeight;
                }

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
                        step = selectUInt8StepStore(diff);
                    }
                    else if (diff<2048.0f)
                    {
                        compressionFlags|=MAP_HEIGHT_AS_INT16;
                        step = selectUInt16StepStore(diff);
                    }

                    // get V9 height map
                    for (int y = 0; y <= 8; y++)
                    {
                        for (int x = 0; x <= 8; x++)
                        {
                            float height = _chunks[cx][cy].L9[y*9+x];
                            if(compressionFlags & MAP_HEIGHT_AS_INT8)
                                uint8_V9[y][x] = (height == -50000.f ? 255 : uint8(height * step + 0.5f));
                            else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                                uint16_V9[y][x] = (height == -50000.f ? 65535 : uint16(height * step + 0.5f));
                            else float_V9[y][x] = height;
                        }
                    }

                    // Write compression flags and height data
                    fwrite(&compressionFlags, sizeof(uint8), 1, output);
                    fwrite(&minHeight, sizeof(float), 1, output);
                    if(compressionFlags & MAP_HEIGHT_AS_INT8)
                    {
                        step = diff / 255; // Update our step to our multiplier
                        fwrite(uint8_V9, sizeof(uint8)*9*9, 1, output);
                        fwrite(&step, sizeof(float), 1, output);
                    }
                    else if(compressionFlags & MAP_HEIGHT_AS_INT16)
                    {
                        step = diff / 65535; // Update our step to our multiplier
                        fwrite(uint16_V9, sizeof(uint16)*9*9, 1, output);
                        fwrite(&step, sizeof(float), 1, output);
                    } else fwrite(float_V9, sizeof(float)*9*9, 1, output); // Uncompressed
                }
            }

            /////// Chunk end
        }
    }

    ADT.close();
    return true;
}

void ADTFile::parseWMO(uint32 map_num, uint32 tileX, uint32 tileY)
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
                    ModelInstance inst(ADT, ModelInstanceNameMap[id]->c_str(), map_num, tileX, tileY);
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
                    WMOInstance inst(ADT, WMOInstanceNameMap[id]->c_str(), map_num, tileX, tileY);
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
