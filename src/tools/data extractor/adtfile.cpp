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

// This option allow limit minimum height to some value (Allow save some memory)
bool  CONF_allow_height_limit = true;
float CONF_use_minHeight = -500.0f;

// This option allow use float to int conversion
bool  CONF_allow_float_to_int   = true;
float CONF_float_to_int8_limit  = 2.0f;      // Max accuracy = val/256
float CONF_float_to_int16_limit = 2048.0f;   // Max accuracy = val/65536
float CONF_flat_height_delta_limit = 0.005f; // If max - min less this value - surface is flat
float CONF_flat_liquid_delta_limit = 0.001f; // If max - min less this value - liquid surface is flat

float selectUInt8StepStore(float maxDiff)
{
    return 255 / maxDiff;
}

float selectUInt16StepStore(float maxDiff)
{
    return 65535 / maxDiff;
}

extern uint16 *LiqType;

bool ADTFile::parseCHNK(uint32 map_num, uint32 tileX, uint32 tileY, FILE *output)
{
    if(ADT.isEof ())
        return false;

    // Temporary grid data store
    uint16 area_entry[16][16];
    float V8[128][128], V9[129][129];
    uint16 liquid_entry[16][16];
    uint8 liquid_flags[16][16];
    bool  liquid_show[128][128];
    float liquid_height[129][129];
    uint16 holes[16][16]; // map hole info
    memset(liquid_show, 0, sizeof(liquid_show));
    memset(liquid_flags, 0, sizeof(liquid_flags));
    memset(liquid_entry, 0, sizeof(liquid_entry));
    memset(liquid_height, -500, sizeof(liquid_height));

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
            { }// MHDR is after version header, contaiins offset data for parsing the ADT file
            else if (!strcmp(fourcc,"MCNK"))
            {
                MapChunkHeader header;
                ADT.read(&header, sizeof(MapChunkHeader));
                area_entry[header.ix][header.iy] = header.areaid;
                holes[header.ix][header.iy] = header.holes;
                if(header.offsMCVT)
                {
                    ADT.seek(currPos+header.offsMCVT);
                    for(uint8 i = 0, idx = 0; i < 8+9; i++)
                    {
                        if(i%2 == 0)
                            ADT.read(&V9[(header.ix*8)+idx][(header.iy*8)], sizeof(float)*9);
                        else
                        {
                            ADT.read(&V8[(header.ix*8)+idx][(header.iy*8)], sizeof(float)*8);
                            idx++;
                        }
                    }
                    for(uint8 x = 0; x < 9; x++)
                    {
                        for(uint8 y = 0; y < 9; y++)
                        {
                            V9[(header.ix*8)+x][(header.iy*8)+y] += header.ypos;
                            if(x == 8 || y == 8)
                                continue;
                            V8[(header.ix*8)+x][(header.iy*8)+y] += header.ypos;
                        }
                    }
                }

                if(header.flags & 0x04)
                    liquid_flags[header.ix][header.iy] |= MAP_LIQUID_TYPE_WATER;
                if(header.flags & 0x08)
                    liquid_flags[header.ix][header.iy] |= MAP_LIQUID_TYPE_OCEAN;
                if(header.flags & 0x10)
                    liquid_flags[header.ix][header.iy] |= MAP_LIQUID_TYPE_MAGMA;
                if(header.flags & 0x20)
                    liquid_flags[header.ix][header.iy] |= MAP_LIQUID_TYPE_SLIME;
                MCLQChunkOffsets[header.ix][header.iy] = (header.sizeMCLQ > 8 && header.offsMCLQ ? currPos+header.offsMCLQ : 0);
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

    for(uint8 cx = 0; cx < 16; cx++)
    {
        for(uint8 cy = 0; cy < 16; cy++)
        {
            // Pre wotlk water chunks
            if(MCLQChunkOffsets[cx][cy])
            {
                ADT.seek(MCLQChunkOffsets[cx][cy]);
                MCLQinformation mclqChunk;
                ADT.read(&mclqChunk, sizeof(MCLQinformation));

                for(uint8 x = 0; x < 9; x++)
                {
                    for(uint8 y = 0; y < 9; y++)
                    {
                        liquid_height[(cx*8)+x][(cy*8)+y] = mclqChunk.liquid[x][y].height;
                        if(x == 8 || y == 8)
                            continue;
                        if(mclqChunk.flags[x][y] != 0x0F)
                        {
                            liquid_show[(cx*8)+x][(cy*8)+y] = true;
                            if(mclqChunk.flags[x][y] & (1<<7))
                                liquid_flags[cx][cy] |= MAP_LIQUID_TYPE_DARK_WATER;
                        }
                    }
                }
            }

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
                bool readHeight = false;
                if(readHeight = ((mh2oHeader.formatFlags&0x02) == 0 && mh2oHeader.offsData2b > 0))
                    ADT.seek(mh2oBase+mh2oHeader.offsData2b);

                for(uint8 x = 0; x <= mh2oHeader.height; x++)
                {
                    uint8 chunkX = (cx*8)+x+mh2oHeader.xOffset;
                    for(uint8 y = 0; y <= mh2oHeader.width; y++)
                    {
                        uint8 chunkY = (cy*8)+y+mh2oHeader.yOffset;
                        if(readHeight)
                            ADT.read(&liquid_height[chunkX][chunkY], sizeof(float));
                        else liquid_height[chunkX][chunkY] = mh2oHeader.heightLevel1;
                        if(x == 8 || y == 8)
                            continue;

                        if(mask & (1<<((x*8)+y)))
                            liquid_show[(cx*8)+x+mh2oHeader.xOffset][(cy*8)+y+mh2oHeader.yOffset] = true;
                    }
                }

                uint32 type = LiqType[mh2oHeader.liquidType];
                switch(type)
                {
                case 0: liquid_flags[cx][cy] |= MAP_LIQUID_TYPE_WATER; break;
                case 2: liquid_flags[cx][cy] |= MAP_LIQUID_TYPE_MAGMA; break;
                case 3: liquid_flags[cx][cy] |= MAP_LIQUID_TYPE_SLIME; break;
                case 1:
                    liquid_flags[cx][cy] |= MAP_LIQUID_TYPE_OCEAN;
                    if((mh2oHeader.formatFlags & 0x01) || !mh2oHeader.offsData2b)
                        liquid_flags[cx][cy] |= MAP_LIQUID_TYPE_DARK_WATER;
                    break;
                }
            }
        }
    }

    // Prepare map header
    map_fileheader map;
    map.mapMagic = *(uint32 const*)MAP_MAGIC;
    map.versionMagic = *(uint32 const*)MAP_VERSION_MAGIC;
    map.buildMagic = 15595;

    //============================================
    // Try pack area data
    //============================================
    bool fullAreaData = false;
    uint32 areaflag = area_entry[0][0];
    for (int y=0;y<16;y++)
    {
        for(int x=0;x<16;x++)
        {
            if(area_entry[y][x]!=areaflag)
            {
                fullAreaData = true;
                break;
            }
        }
    }

    map.areaMapOffset = sizeof(map);
    map.areaMapSize   = sizeof(map_areaHeader);

    map_areaHeader areaHeader;
    areaHeader.fourcc = *(uint32 const*)MAP_AREA_MAGIC;
    areaHeader.flags = 0;
    if (fullAreaData)
    {
        areaHeader.gridArea = 0;
        map.areaMapSize+=sizeof(area_entry);
    }
    else
    {
        areaHeader.flags |= MAP_AREA_NO_AREA;
        areaHeader.gridArea = (uint16)areaflag;
    }

    //============================================
    // Try pack height data
    //============================================
    float maxHeight = -20000, minHeight =  20000;
    for (int y=0; y<129; y++)
    {
        for(int x=0;x<129;x++)
        {
            float h9 = V9[y][x];
            if (maxHeight < h9) maxHeight = h9;
            if (minHeight > h9) minHeight = h9;
            if(y >= 128 || x >= 128)
                continue;
            float h8 = V8[y][x];
            if (maxHeight < h8) maxHeight = h8;
            if (minHeight > h8) minHeight = h8;
        }
    }

    // Check for allow limit minimum height (not store height in deep ochean - allow save some memory)
    if (CONF_allow_height_limit && minHeight < CONF_use_minHeight)
    {
        for (int y=0; y<129; y++)
        {
            for(int x=0;x<129;x++)
            {
                if (V9[y][x] < CONF_use_minHeight)
                    V9[y][x] = CONF_use_minHeight;
                if (V8[y][x] < CONF_use_minHeight)
                    V8[y][x] = CONF_use_minHeight;
            }
        }
        if (minHeight < CONF_use_minHeight)
            minHeight = CONF_use_minHeight;
        if (maxHeight < CONF_use_minHeight)
            maxHeight = CONF_use_minHeight;
    }

    map.heightMapOffset = map.areaMapOffset + map.areaMapSize;
    map.heightMapSize = sizeof(map_heightHeader);

    map_heightHeader heightHeader;
    heightHeader.fourcc = *(uint32 const*)MAP_HEIGHT_MAGIC;
    heightHeader.flags = 0;
    heightHeader.gridHeight    = minHeight;
    heightHeader.gridMaxHeight = maxHeight;

    if (maxHeight == minHeight)
        heightHeader.flags |= MAP_HEIGHT_NO_HEIGHT;

    // Not need store if flat surface
    if (CONF_allow_float_to_int && (maxHeight - minHeight) < CONF_flat_height_delta_limit)
        heightHeader.flags |= MAP_HEIGHT_NO_HEIGHT;

    uint8  uint8_V8[128][128], uint8_V9[129][129];
    uint16 uint16_V8[128][128], uint16_V9[129][129];
    // Try store as packed in uint16 or uint8 values
    if (!(heightHeader.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        float step = 0;
        // Try Store as uint values
        if (CONF_allow_float_to_int)
        {
            float diff = maxHeight - minHeight;
            if (diff < CONF_float_to_int8_limit)      // As uint8 (max accuracy = CONF_float_to_int8_limit/256)
            {
                heightHeader.flags|=MAP_HEIGHT_AS_INT8;
                step = selectUInt8StepStore(diff);
            }
            else if (diff<CONF_float_to_int16_limit)  // As uint16 (max accuracy = CONF_float_to_int16_limit/65536)
            {
                heightHeader.flags|=MAP_HEIGHT_AS_INT16;
                step = selectUInt16StepStore(diff);
            }
        }

        // Pack it to int values if need
        if (heightHeader.flags&MAP_HEIGHT_AS_INT8)
        {
            for (int y=0; y<129; y++)
            {
                for(int x=0;x<129;x++)
                {
                    uint8_V9[y][x] = uint8((V9[y][x] - minHeight) * step + 0.5f);
                    if(x >= 128 || y >= 128)
                        continue;
                    uint8_V8[y][x] = uint8((V8[y][x] - minHeight) * step + 0.5f);
                }
            }
            map.heightMapSize+= sizeof(uint8_V8)+sizeof(uint8_V9);
        }
        else if (heightHeader.flags&MAP_HEIGHT_AS_INT16)
        {
            for (int y=0; y<129; y++)
            {
                for(int x=0;x<129;x++)
                {
                    uint16_V9[y][x] = uint16((V9[y][x] - minHeight) * step + 0.5f);
                    if(x >= 128 || y >= 128)
                        continue;
                    uint16_V8[y][x] = uint16((V8[y][x] - minHeight) * step + 0.5f);
                }
            }
            map.heightMapSize+= sizeof(uint16_V8)+sizeof(uint16_V9);
        } else map.heightMapSize+= sizeof(V8)+sizeof(V9);
    }

    //============================================
    // Pack liquid data
    //============================================
    uint8 type = liquid_flags[0][0];
    bool fullType = false;
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            if (liquid_flags[y][x] != type)
            {
                fullType = true;
                break;
            }
        }
        if(fullType)
            break;
    }

    map_liquidHeader liquidHeader;

    // no water data (if all grid have 0 liquid type)
    if (type == 0 && !fullType)
    {
        // No liquid data
        map.liquidMapOffset = 0;
        map.liquidMapSize   = 0;
    }
    else
    {
        int minX = 255, minY = 255;
        int maxX = 0, maxY = 0;
        maxHeight = -20000, minHeight =  20000;
        for (int y=0; y<128; y++)
        {
            for(int x=0; x<128; x++)
            {
                if (liquid_show[y][x])
                {
                    if (minX > x) minX = x;
                    if (maxX < x) maxX = x;
                    if (minY > y) minY = y;
                    if (maxY < y) maxY = y;
                    float h = liquid_height[y][x];
                    if (maxHeight < h) maxHeight = h;
                    if (minHeight > h) minHeight = h;
                } else liquid_height[y][x] = CONF_use_minHeight;
            }
        }
        map.liquidMapOffset = map.heightMapOffset + map.heightMapSize;
        map.liquidMapSize = sizeof(map_liquidHeader);
        liquidHeader.fourcc = *(uint32 const*)MAP_LIQUID_MAGIC;
        liquidHeader.flags = 0;
        liquidHeader.liquidType = 0;
        liquidHeader.offsetX = minX;
        liquidHeader.offsetY = minY;
        liquidHeader.width   = maxX - minX + 1 + 1;
        liquidHeader.height  = maxY - minY + 1 + 1;
        liquidHeader.liquidLevel = minHeight;

        if (maxHeight == minHeight)
            liquidHeader.flags |= MAP_LIQUID_NO_HEIGHT;

        // Not need store if flat surface
        if (CONF_allow_float_to_int && (maxHeight - minHeight) < CONF_flat_liquid_delta_limit)
            liquidHeader.flags |= MAP_LIQUID_NO_HEIGHT;

        if (!fullType)
            liquidHeader.flags |= MAP_LIQUID_NO_TYPE;

        if (liquidHeader.flags & MAP_LIQUID_NO_TYPE)
            liquidHeader.liquidType = type;
        else map.liquidMapSize += sizeof(liquid_flags);

        if (!(liquidHeader.flags & MAP_LIQUID_NO_HEIGHT))
            map.liquidMapSize += sizeof(float)*liquidHeader.width*liquidHeader.height;
    }

    if (map.liquidMapOffset)
        map.holesOffset = map.liquidMapOffset + map.liquidMapSize;
    else map.holesOffset = map.heightMapOffset + map.heightMapSize;

    bool hasHoles = false;
    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            if(holes[i][j])
            {
                hasHoles = true;
                break;
            }
        }
    }

    if (hasHoles)
        map.holesSize = sizeof(holes);
    else map.holesSize = 0;

    uint8 cFlag = areaHeader.flags | (hasHoles ? 0x02 : 0x00);
    fwrite(&cFlag, 1, 1, output);
    cFlag = heightHeader.flags;
    fwrite(&cFlag, 1, 1, output);
    cFlag = liquidHeader.flags;
    fwrite(&cFlag, 1, 1, output);

    // Store area data
    if (!(areaHeader.flags&MAP_AREA_NO_AREA))
        fwrite(area_entry, sizeof(area_entry), 1, output);
    else fwrite(&areaHeader.gridArea, sizeof(uint16), 1, output);

    // Store V8 height data
    fwrite(&heightHeader.gridHeight, sizeof(float), 1, output);
    if (!(heightHeader.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        fwrite(&heightHeader.gridMaxHeight, sizeof(float), 1, output);
        if (heightHeader.flags & MAP_HEIGHT_AS_INT16)
        {
            fwrite(uint16_V9, sizeof(uint16_V9), 1, output);
            fwrite(uint16_V8, sizeof(uint16_V8), 1, output);
        }
        else if (heightHeader.flags & MAP_HEIGHT_AS_INT8)
        {
            fwrite(uint8_V9, sizeof(uint8_V9), 1, output);
            fwrite(uint8_V8, sizeof(uint8_V8), 1, output);
        }
        else
        {
            fwrite(V9, sizeof(V9), 1, output);
            fwrite(V8, sizeof(V8), 1, output);
        }
    }

    if (!(liquidHeader.flags & MAP_LIQUID_NO_TYPE))
    {
        fwrite(liquid_entry, sizeof(liquid_entry), 1, output);
        fwrite(liquid_flags, sizeof(liquid_flags), 1, output);
    } else fwrite(&liquidHeader.liquidType, sizeof(uint8), 1, output);

    if (!(liquidHeader.flags & MAP_LIQUID_NO_HEIGHT))
    {
        for (int y = 0; y < liquidHeader.height; y++)
            fwrite(&liquid_height[y + liquidHeader.offsetY][liquidHeader.offsetX], sizeof(float), liquidHeader.width, output);
    } else fwrite(&liquidHeader.liquidLevel, sizeof(float), 1, output);

    // store hole data
    if (hasHoles)
        fwrite(holes, map.holesSize, 1, output);

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
