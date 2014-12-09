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

#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <deque>
#include <list>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include "direct.h"
#else
#include <sys/stat.h>
#include <unistd.h>
#define ERROR_PATH_NOT_FOUND ERROR_FILE_NOT_FOUND
#endif

#include "loadlib.h"
#include <dbclib\DBCLib.h>

#include "adt.h"
#include "wdt.h"
#include <fcntl.h>

#if defined( __GNUC__ )
    #define _open   open
    #define _close close
    #ifndef O_BINARY
        #define O_BINARY 0
    #endif
#else
    #include <io.h>
#endif

#ifdef O_LARGEFILE
    #define OPEN_FLAGS (O_RDONLY | O_BINARY | O_LARGEFILE)
#else
    #define OPEN_FLAGS (O_RDONLY | O_BINARY)
#endif

#ifdef _DEBUG
#define PRINT_ERR printf
#else
#define PRINT_ERR
#endif

HANDLE WorldMpq = NULL;
HANDLE ModelMqp = NULL;
HANDLE LocaleMpq = NULL;

typedef struct
{
    char name[64];
    uint32 id;
} map_id;

map_id *map_ids;
uint16 *areas;
uint16 *LiqType;
char output_path[128] = ".";
char input_path[128] = ".";
uint32 maxAreaId = 0;

// Custom DBC data
struct M2Header
{
    char id[4];
    unsigned char version[4];
    uint32 nameLength;
    uint32 nameOfs;
    uint32 type;
    uint32 nGlobalSequences;
    uint32 ofsGlobalSequences;
    uint32 nAnimations;
    uint32 ofsAnimations;
    uint32 nAnimationLookup;
    uint32 ofsAnimationLookup;
    uint32 nBones;
    uint32 ofsBones;
    uint32 nKeyBoneLookup;
    uint32 ofsKeyBoneLookup;

    uint32 nVertices;
    uint32 ofsVertices;
    uint32 nViews;

    uint32 nColors;
    uint32 ofsColors;

    uint32 nTextures;
    uint32 ofsTextures;

    uint32 nTransparency;
    uint32 ofsTransparency;
    uint32 nUVAnimation;
    uint32 ofsUVAnimation;
    uint32 nTexReplace;
    uint32 ofsTexReplace;

    uint32 nRenderFlags;
    uint32 ofsRenderFlags;
    uint32 nBoneLookupTable;
    uint32 ofsBoneLookupTable;

    uint32 nTexLookup;
    uint32 ofsTexLookup;

    uint32 nTexUnitLookup;
    uint32 ofsTexUnitLookup;
    uint32 nTransparencyLookup;
    uint32 ofsTransparencyLookup;
    uint32 nUVAnimLookup;
    uint32 ofsUVAnimLookup;

    float vertexbox1[3];
    float vertexbox2[3];
    float vertexradius;
    float boundingbox1[3];
    float boundingbox2[3];
    float boundingradius;

    uint32 nBoundingTriangles;
    uint32 ofsBoundingTriangles;
    uint32 nBoundingVertices;
    uint32 ofsBoundingVertices;
    uint32 nBoundingNormals;
    uint32 ofsBoundingNormals;

    uint32 nAttachments;
    uint32 ofsAttachments;
    uint32 nAttachmentLookup;
    uint32 ofsAttachmentLookup;
    uint32 nEvents;
    uint32 ofsEvents;
    uint32 nLights;
    uint32 ofsLights;
    uint32 nCameras;
    uint32 ofsCameras;
    uint32 nCameraLookup;
    uint32 ofsCameraLookup;
    uint32 nRibbonEmitters;
    uint32 ofsRibbonEmitters;
    uint32 nParticleEmitters;
    uint32 ofsParticleEmitters;
};

struct AnimationBlock
{
    uint16 interpolation;
    uint16 globalsequenceid;
    uint32 list1offset;
    uint32 timestampdataoffset;
    uint32 list2offset;
    uint32 keysoffset;
};

struct M2Attachment
{
    uint32 id;
    uint32 bone;
    float pos[3];
    AnimationBlock unk;
};

struct M2Bone
{
    int keyboneid;
    uint32 flags;
    short parentbone;
    uint16 unk[3];
    AnimationBlock translation;
    AnimationBlock rotation;
    AnimationBlock scaling;
    float pivotpoint[3];
};

struct ModelCache
{
    M2Header* header;
    M2Attachment* attachments;
    M2Bone* bones;
    uint16* bonelookups;
};

struct DisplayBounding
{
    uint32 Entry; // Display ID
    float Low[3];
    float High[3];
    float BoundRadius;
};

void replaceInString(std::string &str, const char* find, const char* rep, uint32 limit)
{
    uint32 i=0;
    std::string::size_type pos=0;
    while((pos = str.find(find, pos)) != std::string::npos)
    {
        str.erase(pos, strlen(find));
        str.insert(pos, rep);
        pos += strlen(rep);

        ++i;
        if (limit != 0 && i == limit)
            break;
    }
}


// **************************************************
// Extractor options
// **************************************************
enum Extract
{
    EXTRACT_MAP = 1,
    EXTRACT_DBC = 2,
    CREATE_DBCS = 3
};

// Select data for extract
int   CONF_extract = EXTRACT_MAP | EXTRACT_DBC | CREATE_DBCS;

// This option allow limit minimum height to some value (Allow save some memory)
bool  CONF_allow_height_limit = true;
float CONF_use_minHeight = -500.0f;

// This option allow use float to int conversion
bool  CONF_allow_float_to_int   = true;
float CONF_float_to_int8_limit  = 2.0f;      // Max accuracy = val/256
float CONF_float_to_int16_limit = 2048.0f;   // Max accuracy = val/65536
float CONF_flat_height_delta_limit = 0.005f; // If max - min less this value - surface is flat
float CONF_flat_liquid_delta_limit = 0.001f; // If max - min less this value - liquid surface is flat

uint32 CONF_TargetBuild = 15595;              // 4.3.4.15595

// List MPQ for extract maps from
char const* CONF_mpq_list[]=
{
    "world.MPQ",
    "art.MPQ",
    "world2.MPQ",
    "expansion1.MPQ",
    "expansion2.MPQ",
    "expansion3.MPQ",
};

uint32 const Builds[] = {13164, 13205, 13287, 13329, 13596, 13623, 13914, 14007, 14333, 14480, 14545, 15005, 15050, 15211, 15354, 15595, 0};
#define LAST_DBC_IN_DATA_BUILD 13623    // after this build mpqs with dbc are back to locale folder
#define NEW_BASE_SET_BUILD  15211

char const* Locales[] =
{
    "enGB", "enUS",
    "deDE", "esES",
    "frFR", "koKR",
    "zhCN", "zhTW",
    "enCN", "enTW",
    "esMX", "ruRU"
};

TCHAR const* LocalesT[] =
{
    _T("enGB"), _T("enUS"),
    _T("deDE"), _T("esES"),
    _T("frFR"), _T("koKR"),
    _T("zhCN"), _T("zhTW"),
    _T("enCN"), _T("enTW"),
    _T("esMX"), _T("ruRU"),
};

#define LOCALES_COUNT 12

void CreateDir(std::string const& path)
{
    if (chdir(path.c_str()) == 0)
    {
            chdir("../");
            return;
    }

#ifdef _WIN32
    _mkdir(path.c_str());
#else
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO); // 0777
#endif
}

bool FileExists(TCHAR const* fileName)
{
    int fp = _open(fileName, OPEN_FLAGS);
    if(fp != -1)
    {
        _close(fp);
        return true;
    }

    return false;
}

void Usage(char const* prg)
{
    printf(
        "Usage:\n"\
        "%s -[var] [value]\n"\
        "-i set input path\n"\
        "-o set output path\n"\
        "-e extract only MAP(1)/DBC(2) - standard: both(3)\n"\
        "-f height stored as int (less map size but lost some accuracy) 1 by default\n"\
        "-b target build (default %u)\n"\
        "Example: %s -f 0 -i \"c:\\games\\game\"", prg, CONF_TargetBuild, prg);
    exit(1);
}

void HandleArgs(int argc, char* arg[])
{
    for (int c = 1; c < argc; ++c)
    {
        // i - input path
        // o - output path
        // e - extract only MAP(1)/DBC(2) - standard both(3)
        // f - use float to int conversion
        // h - limit minimum height
        // b - target client build
        if (arg[c][0] != '-')
            Usage(arg[0]);

        switch (arg[c][1])
        {
        case 'i':
            if (c + 1 < argc)                            // all ok
                strcpy(input_path, arg[c++ + 1]);
            else
                Usage(arg[0]);
            break;
        case 'o':
            if (c + 1 < argc)                            // all ok
                strcpy(output_path, arg[c++ + 1]);
            else
                Usage(arg[0]);
            break;
        case 'f':
            if (c + 1 < argc)                            // all ok
                CONF_allow_float_to_int = atoi(arg[c++ + 1])!=0;
            else
                Usage(arg[0]);
            break;
        case 'e':
            if (c + 1 < argc)                            // all ok
            {
                CONF_extract = atoi(arg[c++ + 1]);
                if (!(CONF_extract > 0 && CONF_extract < 4))
                    Usage(arg[0]);
            }
            else
                Usage(arg[0]);
            break;
        case 'b':
            if (c + 1 < argc)                            // all ok
                CONF_TargetBuild = atoi(arg[c++ + 1]);
            else
                Usage(arg[0]);
            break;
        default:
            break;
        }
    }
}

uint32 ReadBuild(int locale)
{
    // include build info file also
    std::string filename  = std::string("component.wow-") + Locales[locale] + ".txt";
    //printf("Read %s file... ", filename.c_str());

    HANDLE dbcFile;
    if (!SFileOpenFileEx(LocaleMpq, filename.c_str(), SFILE_OPEN_FROM_MPQ, &dbcFile))
    {
        PRINT_ERR("Fatal error: Not found %s file!\n", filename.c_str());
        exit(1);
    }

    char buff[512];
    DWORD readBytes = 0;
    SFileReadFile(dbcFile, buff, 512, &readBytes, NULL);
    if (!readBytes)
    {
        PRINT_ERR("Fatal error: Not found %s file!\n", filename.c_str());
        exit(1);
    }

    std::string text = buff;
    SFileCloseFile(dbcFile);

    size_t pos = text.find("version=\"");
    size_t pos1 = pos + strlen("version=\"");
    size_t pos2 = text.find("\"", pos1);
    if (pos == text.npos || pos2 == text.npos || pos1 >= pos2)
    {
        PRINT_ERR("Fatal error: Invalid  %s file format!\n", filename.c_str());
        exit(1);
    }

    std::string build_str = text.substr(pos1,pos2-pos1);

    int build = atoi(build_str.c_str());
    if (build <= 0)
    {
        PRINT_ERR("Fatal error: Invalid  %s file format!\n", filename.c_str());
        exit(1);
    }

    return build;
}

uint32 ReadMapDBC()
{
    printf("Read Map.dbc file... ");

    HANDLE dbcFile;
    if (!SFileOpenFileEx(LocaleMpq, "DBFilesClient\\Map.dbc", SFILE_OPEN_FROM_MPQ, &dbcFile))
    {
        PRINT_ERR("Fatal error: Cannot find Map.dbc in archive!\n");
        exit(1);
    }

    DBCFile dbc;
    if (!dbc.openFromMPQ(dbcFile))
    {
        PRINT_ERR("Fatal error: Invalid Map.dbc file format!\n");
        exit(1);
    }

    size_t map_count = dbc.getRecordCount();
    map_ids = new map_id[map_count];
    for(uint32 x = 0; x < map_count; ++x)
    {
        map_ids[x].id = dbc.getRecord(x).getUInt(0);
        strcpy(map_ids[x].name, dbc.getRecord(x).getString(1));
    }

    SFileCloseFile(dbcFile);
    printf("Done! (%u maps loaded)\n", uint32(map_count));
    return map_count;
}

void ReadAreaTableDBC()
{
    printf("Read AreaTable.dbc file...");
    HANDLE dbcFile;
    if (!SFileOpenFileEx(LocaleMpq, "DBFilesClient\\AreaTable.dbc", SFILE_OPEN_FROM_MPQ, &dbcFile))
    {
        PRINT_ERR("Fatal error: Cannot find AreaTable.dbc in archive!\n");
        exit(1);
    }

    DBCFile dbc;
    if(!dbc.openFromMPQ(dbcFile))
    {
        PRINT_ERR("Fatal error: Invalid AreaTable.dbc file format!\n");
        exit(1);
    }

    size_t area_count = dbc.getRecordCount();
    maxAreaId = dbc.getMaxId();
    areas = new uint16[maxAreaId + 1];

    for (uint32 x = 0; x < area_count; ++x)
        areas[dbc.getRecord(x).getUInt(0)] = dbc.getRecord(x).getUInt(3);

    SFileCloseFile(dbcFile);
    printf("Done! (%u areas loaded)\n", uint32(area_count));
}

void ReadLiquidTypeTableDBC()
{
    printf("Read LiquidType.dbc file...");
    HANDLE dbcFile;
    if (!SFileOpenFileEx(LocaleMpq, "DBFilesClient\\LiquidType.dbc", SFILE_OPEN_FROM_MPQ, &dbcFile))
    {
        PRINT_ERR("Fatal error: Cannot find LiquidType.dbc in archive!\n");
        exit(1);
    }

    DBCFile dbc;
    if(!dbc.openFromMPQ(dbcFile))
    {
        PRINT_ERR("Fatal error: Invalid LiquidType.dbc file format!\n");
        exit(1);
    }

    size_t liqTypeCount = dbc.getRecordCount();
    size_t liqTypeMaxId = dbc.getMaxId();
    LiqType = new uint16[liqTypeMaxId + 1];
    memset(LiqType, 0xff, (liqTypeMaxId + 1) * sizeof(uint16));

    for(uint32 x = 0; x < liqTypeCount; ++x)
        LiqType[dbc.getRecord(x).getUInt(0)] = dbc.getRecord(x).getUInt(3);

    printf("Done! (%u LiqTypes loaded)\n", (uint32)liqTypeCount);
    SFileCloseFile(dbcFile);
}

//
// Adt file convertor function and data
//

// Map file format data
static char const* MAP_MAGIC         = "MAPS";
static char const* MAP_VERSION_MAGIC = "v1.3";
static char const* MAP_AREA_MAGIC    = "AREA";
static char const* MAP_HEIGHT_MAGIC  = "MHGT";
static char const* MAP_LIQUID_MAGIC  = "MLIQ";

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

#define MAP_AREA_NO_AREA      0x0001

struct map_areaHeader
{
    uint32 fourcc;
    uint16 flags;
    uint16 gridArea;
};

#define MAP_HEIGHT_NO_HEIGHT  0x0001

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
    uint16 liquidType;
    uint8  offsetX;
    uint8  offsetY;
    uint8  width;
    uint8  height;
    float  liquidLevel;
};

float selectUInt8StepStore(float maxDiff)
{
    return 255 / maxDiff;
}

float selectUInt16StepStore(float maxDiff)
{
    return 65535 / maxDiff;
}
// Temporary grid data store
uint16 area_flags[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];

float V8[ADT_GRID_SIZE][ADT_GRID_SIZE];
float V9[ADT_GRID_SIZE+1][ADT_GRID_SIZE+1];

uint16 liquid_flags[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];
bool  liquid_show[ADT_GRID_SIZE][ADT_GRID_SIZE];
float liquid_height[ADT_GRID_SIZE+1][ADT_GRID_SIZE+1];

bool ConvertADT(FILE *out_file, char *filename, int /*cell_y*/, int /*cell_x*/, uint32 build)
{
    ADT_file adt;

    if (!adt.loadFile(WorldMpq, filename))
        return false;

    memset(liquid_show, 0, sizeof(liquid_show));
    memset(liquid_flags, 0, sizeof(liquid_flags));

    // Prepare map header
    map_fileheader map;
    map.mapMagic = *(uint32 const*)MAP_MAGIC;
    map.versionMagic = *(uint32 const*)MAP_VERSION_MAGIC;
    map.buildMagic = build;

    // Get area flags data
    for (int i = 0; i < ADT_CELLS_PER_GRID; ++i)
    {
        for (int j = 0; j < ADT_CELLS_PER_GRID; ++j)
        {
            adt_MCNK* cell = adt.cells[i][j];
            uint32 areaid = cell->areaid;
            if (areaid && areaid <= maxAreaId)
            {
                if (areas[areaid] != 0xFFFF)
                {
                    area_flags[i][j] = areas[areaid];
                    continue;
                }

                printf("File: %s\nCan't find area flag for areaid %u [%d, %d].\n", filename, areaid, cell->ix, cell->iy);
            }

            area_flags[i][j] = 0xffff;
        }
    }

    //
    // Get Height map from grid
    //
    for (int i=0;i<ADT_CELLS_PER_GRID;i++)
    {
        for(int j=0;j<ADT_CELLS_PER_GRID;j++)
        {
            adt_MCNK * cell = adt.cells[i][j];
            if (!cell)
                continue;
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

            // Set map height as grid height
            for (int y=0; y <= ADT_CELL_SIZE; y++)
            {
                int cy = i*ADT_CELL_SIZE + y;
                for (int x=0; x <= ADT_CELL_SIZE; x++)
                {
                    int cx = j*ADT_CELL_SIZE + x;
                    V9[cy][cx]=cell->ypos;
                }
            }
            for (int y=0; y < ADT_CELL_SIZE; y++)
            {
                int cy = i*ADT_CELL_SIZE + y;
                for (int x=0; x < ADT_CELL_SIZE; x++)
                {
                    int cx = j*ADT_CELL_SIZE + x;
                    V8[cy][cx]=cell->ypos;
                }
            }
            // Get custom height
            adt_MCVT *v = cell->getMCVT();
            if (!v)
                continue;
            // get V9 height map
            for (int y=0; y <= ADT_CELL_SIZE; y++)
            {
                int cy = i*ADT_CELL_SIZE + y;
                for (int x=0; x <= ADT_CELL_SIZE; x++)
                {
                    int cx = j*ADT_CELL_SIZE + x;
                    V9[cy][cx]+=v->height_map[y*(ADT_CELL_SIZE*2+1)+x];
                }
            }
            // get V8 height map
            for (int y=0; y < ADT_CELL_SIZE; y++)
            {
                int cy = i*ADT_CELL_SIZE + y;
                for (int x=0; x < ADT_CELL_SIZE; x++)
                {
                    int cx = j*ADT_CELL_SIZE + x;
                    V8[cy][cx]+=v->height_map[y*(ADT_CELL_SIZE*2+1)+ADT_CELL_SIZE+1+x];
                }
            }
        }
    }

    // Get from MCLQ chunk (old)
    for (int i = 0; i < ADT_CELLS_PER_GRID; i++)
    {
        for(int j = 0; j < ADT_CELLS_PER_GRID; j++)
        {
            adt_MCNK *cell = adt.cells[i][j];
            if (!cell)
                continue;

            adt_MCLQ *liquid = cell->getMCLQ();
            int count = 0;
            if (!liquid || cell->sizeMCLQ <= 8)
                continue;

            for (int y = 0; y < ADT_CELL_SIZE; y++)
            {
                int cy = i * ADT_CELL_SIZE + y;
                for (int x = 0; x < ADT_CELL_SIZE; x++)
                {
                    int cx = j * ADT_CELL_SIZE + x;
                    if (liquid->flags[y][x] != 0x0F)
                    {
                        liquid_show[cy][cx] = true;
                        if (liquid->flags[y][x] & (1<<7))
                            liquid_flags[i][j] |= MAP_LIQUID_TYPE_DARK_WATER;
                        ++count;
                    }
                }
            }

            uint32 c_flag = cell->flags;
            if (c_flag & (1<<2))
            {
                liquid_flags[i][j] |= MAP_LIQUID_TYPE_WATER;            // water
            }
            if (c_flag & (1<<3))
            {
                liquid_flags[i][j] |= MAP_LIQUID_TYPE_OCEAN;            // ocean
            }
            if (c_flag & (1<<4))
            {
                liquid_flags[i][j] |= MAP_LIQUID_TYPE_MAGMA;            // magma/slime
            }

            if (!count && liquid_flags[i][j])
                fprintf(stderr, "Wrong liquid detect in MCLQ chunk");

            for (int y = 0; y <= ADT_CELL_SIZE; y++)
            {
                int cy = i * ADT_CELL_SIZE + y;
                for (int x = 0; x <= ADT_CELL_SIZE; x++)
                {
                    int cx = j * ADT_CELL_SIZE + x;
                    liquid_height[cy][cx] = liquid->liquid[y][x].height;
                }
            }
        }
    }

    // Get liquid map for grid (in WOTLK used MH2O chunk)
    adt_MH2O * h2o = adt.a_grid->getMH2O();
    if (h2o)
    {
        for (int i = 0; i < ADT_CELLS_PER_GRID; i++)
        {
            for(int j = 0; j < ADT_CELLS_PER_GRID; j++)
            {
                adt_liquid_header *h = h2o->getLiquidData(i,j);
                if (!h)
                    continue;

                int count = 0;
                uint64 show = h2o->getLiquidShowMap(h);
                for (int y = 0; y < h->height; y++)
                {
                    int cy = i * ADT_CELL_SIZE + y + h->yOffset;
                    for (int x = 0; x < h->width; x++)
                    {
                        int cx = j * ADT_CELL_SIZE + x + h->xOffset;
                        if (show & 1)
                        {
                            liquid_show[cy][cx] = true;
                            ++count;
                        }
                        show >>= 1;
                    }
                }

                switch (h->liquidType)
                {
                    case LIQUID_TYPE_WATER: liquid_flags[i][j] |= MAP_LIQUID_TYPE_WATER; break;
                    case LIQUID_TYPE_OCEAN: liquid_flags[i][j] |= MAP_LIQUID_TYPE_OCEAN; break;
                    case LIQUID_TYPE_MAGMA: liquid_flags[i][j] |= MAP_LIQUID_TYPE_MAGMA; break;
                    case LIQUID_TYPE_SLIME: liquid_flags[i][j] |= MAP_LIQUID_TYPE_SLIME; break;
                    default:
                        printf("\nCan't find Liquid type %u for map %s\nchunk %d,%d\n", h->liquidType, filename, i, j);
                        break;
                }
                // Dark water detect
                if (LiqType[h->liquidType] == LIQUID_TYPE_OCEAN)
                {
                    uint8* lm = h2o->getLiquidLightMap(h);
                    if (!lm)
                        liquid_flags[i][j] |= MAP_LIQUID_TYPE_DARK_WATER;
                }

                if (!count && liquid_flags[i][j])
                    printf("Wrong liquid detect in MH2O chunk");

                float* height = h2o->getLiquidHeightMap(h);
                int pos = 0;
                for (int y = 0; y <= h->height; y++)
                {
                    int cy = i * ADT_CELL_SIZE + y + h->yOffset;
                    for (int x = 0; x <= h->width; x++)
                    {
                        int cx = j * ADT_CELL_SIZE + x + h->xOffset;

                        if (height)
                            liquid_height[cy][cx] = height[pos];
                        else
                            liquid_height[cy][cx] = h->heightLevel1;

                        pos++;
                    }
                }
            }
        }
    }

    fwrite(area_flags, sizeof(area_flags), 1, out_file);
    fwrite(liquid_flags, sizeof(liquid_flags), 1, out_file);
    fwrite(V8, sizeof(V8), 1, out_file);
    fwrite(V9, sizeof(V9), 1, out_file);
    fwrite(liquid_height, sizeof(liquid_height), 1, out_file);
    return true;
}

void ExtractMapsFromMpq(uint32 build)
{
    char mpq_filename[1024];
    char output_filename[1024];
    char mpq_map_name[1024];

    printf("Extracting maps...\n");

    uint32 map_count = ReadMapDBC();

    ReadAreaTableDBC();
    ReadLiquidTypeTableDBC();

    std::string path = output_path;
    path += "/maps/";
    CreateDir(path);

    printf("Convert map files\n");
    for (uint32 z = 0; z < map_count; ++z)
    {
        WDT_file wdt;
        FILE *out_file = NULL;
        sprintf(output_filename, "%s/maps/%03u.bin", output_path, map_ids[z].id);
        fopen_s(&out_file, output_filename, "wb");
        if(out_file == NULL)
        {
            printf("Could not create output file!\n");
            break;
        }

        uint32 offsets[WDT_MAP_SIZE][WDT_MAP_SIZE];
        memset(offsets, 0, sizeof(uint32)*WDT_MAP_SIZE*WDT_MAP_SIZE);
        fwrite(offsets, sizeof(uint32)*WDT_MAP_SIZE*WDT_MAP_SIZE, 1, out_file);
        printf("Extracting %s (%d/%u)               \n", map_ids[z].name, z+1, map_count);

        // Loadup map grid data
        sprintf(mpq_map_name, "World\\Maps\\%s\\%s.wdt", map_ids[z].name, map_ids[z].name);
        if (!wdt.loadFile(WorldMpq, mpq_map_name, false))
        {
            fclose(out_file);
            continue;
        }

        fseek(out_file, 0, SEEK_END);
        for (uint32 y = 0; y < WDT_MAP_SIZE; ++y)
        {
            for (uint32 x = 0; x < WDT_MAP_SIZE; ++x)
            {
                if (!(wdt.main->adt_list[y][x].flag & 0x1))
                    continue;

                sprintf(mpq_filename, "World\\Maps\\%s\\%s_%u_%u.adt", map_ids[z].name, map_ids[z].name, x, y);
                uint32 Offset = ftell(out_file);
                if(ConvertADT(out_file, mpq_filename, y, x, build))
                    offsets[x][y] = Offset;
            }

            // draw progress bar
            printf("Processing........................%d%%\r", (100 * (y+1)) / WDT_MAP_SIZE);
        }
        fseek(out_file, 0, SEEK_SET);
        fwrite(offsets, sizeof(uint32)*WDT_MAP_SIZE*WDT_MAP_SIZE, 1, out_file);
        fclose(out_file);
    }

    printf("\n");
    delete [] areas;
    delete [] map_ids;
}

void CreateCustomDBCFiles()
{
    printf("Creating required custom dbc files...\n");
    std::map<uint32, DisplayBounding*> m_DisplayMap;
    std::map<std::string, ModelCache> modelCache;
    std::map<uint32, DBCFile::Record> modelInfoEntries;

    //map.dbc
    HANDLE display, model;
    if (!SFileOpenFileEx(LocaleMpq, "DBFilesClient\\CreatureDisplayInfo.dbc", SFILE_OPEN_FROM_MPQ, &display)
        || !SFileOpenFileEx(LocaleMpq, "DBFilesClient\\CreatureModelData.dbc", SFILE_OPEN_FROM_MPQ, &model))
        return;
    DBCFile displayInfo, modelInfo;
    bool res = false;
    if(res |= displayInfo.openFromMPQ(display))
        res = modelInfo.openFromMPQ(model);
    SFileCloseFile(display);
    SFileCloseFile(model);
    if(res == false)
    {
        PRINT_ERR("Unable to open model data dbcs!\n");
        return;
    }

    std::string updateStr, path = "./dbc/", filename = path + std::string("CreatureBoundInformation.dbc");
    if(!FileExists(filename.c_str()))
    {
        FILE* newDBC = fopen(filename.c_str(), "wb");
        if(newDBC != NULL)
        {
            char header[4];
            unsigned int na = 0, nb = 8, es = sizeof(DisplayBounding), ss = 0;
            header[0] = 'W'; header[1] = 'D';
            header[2] = 'B'; header[3] = 'C';
            fwrite(header, 4, 1, newDBC);

            for(uint32 x = 0; x < modelInfo.getRecordCount(); ++x)
            {
                uint32 entry = modelInfo.getRecord(x).getUInt(0);
                modelInfoEntries.insert(std::make_pair(entry, modelInfo.getRecord(x)));
            }

            for(uint32 x = 0; x < displayInfo.getRecordCount(); ++x)
            {
                unsigned int displayid = displayInfo.getRecord(x).getInt(0);
                unsigned int modelentry = displayInfo.getRecord(x).getInt(1);
                float modelscale = displayInfo.getRecord(x).getFloat(4);

                std::map<uint32, DBCFile::Record>::iterator  modelitr = modelInfoEntries.find(modelentry);
                if (modelitr == modelInfoEntries.end())
                {
                    PRINT_ERR("Cannot find model entry for display %u (entry %u)\n", displayid, modelentry);
                    continue;
                }
                updateStr.append(".");
                printf("Building%s %10s\r", updateStr.c_str(), "");
                if(!(na%10)) updateStr.clear();

                DisplayBounding* BoundingInfo = new DisplayBounding();
                DBCFile::Record modelrec = modelitr->second;

                const char* modelname = modelrec.getString(2);

                std::string strmodelname(modelname);

                replaceInString(strmodelname, ".mdx", ".m2", 0);
                replaceInString(strmodelname, ".MDX", ".m2", 0);

                M2Header* header;
                M2Attachment* attachments;
                M2Bone* bones;
                uint16* bonelookups;

                std::map<std::string, ModelCache>::iterator cacheitr = modelCache.find(modelname);
                if (cacheitr == modelCache.end())
                {
                    bool res = true;
                    HANDLE modelFile;
                    if(res = SFileOpenFileEx(WorldMpq, strmodelname.c_str(), SFILE_OPEN_FROM_MPQ, &modelFile))
                    {
                        uint32 size = SFileGetFileSize(modelFile, NULL);
                        if(res = !(size == 0 || size == SFILE_INVALID_SIZE))
                        {
                            res = false;
                            DWORD readLen = 0;
                            header = (M2Header*)malloc(sizeof(M2Header));
                            if(res = SFileReadFile(modelFile, header, sizeof(M2Header), &readLen, NULL))
                            {
                                attachments = (M2Attachment*)malloc(header->nAttachments * sizeof(M2Attachment));
                                bonelookups = (uint16*)malloc(header->nBoneLookupTable * sizeof(uint16));
                                bones = (M2Bone*)malloc(header->nBones * sizeof(M2Bone));

                                if(res && (res = (SFileSetFilePointer(modelFile, header->ofsAttachments, NULL, FILE_BEGIN) != SFILE_INVALID_SIZE)))
                                    res = SFileReadFile(modelFile, attachments, header->nAttachments * sizeof(M2Attachment), &readLen, NULL);
                                if(res && (res = (SFileSetFilePointer(modelFile, header->ofsBoneLookupTable, NULL, FILE_BEGIN) != SFILE_INVALID_SIZE)))
                                    res = SFileReadFile(modelFile, bonelookups, header->nBoneLookupTable * sizeof(uint16), &readLen, NULL);
                                if(res && (res = (SFileSetFilePointer(modelFile, header->ofsBones, NULL, FILE_BEGIN) != SFILE_INVALID_SIZE)))
                                    res = SFileReadFile(modelFile, bones, header->nBones * sizeof(M2Bone), &readLen, NULL);
                                if(res)
                                {
                                    ModelCache cacheentry;
                                    cacheentry.attachments = attachments;
                                    cacheentry.bones = bones;
                                    cacheentry.bonelookups = bonelookups;
                                    cacheentry.header = header;
                                    modelCache.insert(std::make_pair(modelname, cacheentry));
                                }
                                else
                                {
                                    delete attachments;
                                    delete bonelookups;
                                    delete bones;
                                }
                            }
                            if(res == false)
                                delete header;
                        }
                    }

                    if(res == false)
                    {
                        delete BoundingInfo;
                        PRINT_ERR("Error: cannot open %s\n", strmodelname.c_str());
                        continue;
                    }
                }
                else
                {
                    header = cacheitr->second.header;
                    bones = cacheitr->second.bones;
                    bonelookups = cacheitr->second.bonelookups;
                    attachments = cacheitr->second.attachments;
                }

    #ifdef GET_BONE_DATA
                // try and get the bone
                for (uint32 i = 0; i < header->nAttachments; ++i)
                {
                    if (attachments[i].bone > header->nBoneLookupTable)
                    {
    #ifdef _DEBUG
                        printf("Attachment %u requests bonelookup %u (too large, bonelookup table is only %u entries)\n", i, attachments[i].bone, header->nBoneLookupTable);
    #endif
                        continue;
                    }

                    uint16 boneindex = bonelookups[attachments[i].bone];
                    if (boneindex > header->nBones)
                    {
    #ifdef _DEBUG
                        printf("Attachment %u requests bone %u (too large, bone table is only %u entries)\n", i, boneindex, header->nBones);
    #endif
                        continue;
                    }
                    M2Bone & bone = bones[boneindex];
                    //printf("Attachment %u (bone pivot %f %f %f offset %f %f %f)\n", attachments[i].id, bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2], attachments[i].pos[0],  attachments[i].pos[1],  attachments[i].pos[2]);

                    float realpos[3];
                    realpos[0] = (/*bone.pivotpoint[0] +*/ attachments[i].pos[0]) * modelscale;
                    realpos[1] = (/*bone.pivotpoint[1] +*/ attachments[i].pos[1]) * modelscale;
                    realpos[2] = (/*bone.pivotpoint[2] +*/ attachments[i].pos[2]) * modelscale;

                    //fix coord system
    //                  float tmp = realpos[2];
    //                  realpos[2] = realpos[1];
    //                  realpos[1] = -tmp;
                    //fprintf(fo, "insert into `display_attachment_points` VALUES (%u, %u, %f, %f, %f);\n", displayid, attachments[i].id, attachments[i].pos[0], attachments[i].pos[1], attachments[i].pos[2]);
                    //printf("Attachmnent %u point %f %f %f pivot %f %f %f\n", attachments[i].id, realpos[0], realpos[1], realpos[2], bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2]);
                }
    #endif

                BoundingInfo->Entry = displayid;
                BoundingInfo->Low[0] = ((floor(1000000*(header->boundingbox1[0] * modelscale)))/1000000);
                BoundingInfo->Low[1] = ((floor(1000000*(header->boundingbox1[1] * modelscale)))/1000000);
                BoundingInfo->Low[2] = ((floor(1000000*(header->boundingbox1[2] * modelscale)))/1000000);
                BoundingInfo->High[0] = ((floor(1000000*(header->boundingbox2[0] * modelscale)))/1000000);
                BoundingInfo->High[1] = ((floor(1000000*(header->boundingbox2[1] * modelscale)))/1000000);
                BoundingInfo->High[2] = ((floor(1000000*(header->boundingbox2[2] * modelscale)))/1000000);
                BoundingInfo->BoundRadius = ((floor(1000000*(header->boundingradius * modelscale)))/1000000);
                m_DisplayMap.insert(std::make_pair(displayid, BoundingInfo));
                na++;
            }

            printf("%u Creature Bound Information entries created.\n", na);
            fwrite(&na, 4, 1, newDBC);
            fwrite(&nb, 4, 1, newDBC);
            fwrite(&es, 4, 1, newDBC);
            fwrite(&ss, 4, 1, newDBC);
            for(std::map<uint32, DisplayBounding*>::iterator itr = m_DisplayMap.begin(); itr != m_DisplayMap.end(); itr++)
                fwrite(((uint8*)(itr->second)), es, 1, newDBC);
            fclose(newDBC);
            newDBC = NULL;
        }

        printf("Cleaning up bound data...\n");
        DisplayBounding* buff = NULL;
        for(std::map<uint32, DisplayBounding*>::iterator itr = m_DisplayMap.begin(), itr2; itr != m_DisplayMap.end();)
        {
            itr2 = itr++;
            buff = itr2->second;
            m_DisplayMap.erase(itr2);
            delete buff;
            buff = NULL;
        }
        printf("Done!\n\n");
    }
}

bool ExtractFile(HANDLE fileInArchive, char const* filename)
{
    FILE* output = fopen(filename, "wb");
    if(!output)
    {
        printf("Can't create the output file '%s'\n", filename);
        return false;
    }

    char  buffer[0x10000];
    DWORD readBytes = 1;

    while (readBytes > 0)
    {
        SFileReadFile(fileInArchive, buffer, sizeof(buffer), &readBytes, NULL);
        if (readBytes > 0)
            fwrite(buffer, 1, readBytes, output);
    }

    fclose(output);
    return true;
}

void ExtractDBCFiles(int l, bool basicLocale)
{
    printf("Extracting dbc files...\n");

    SFILE_FIND_DATA foundFile;
    memset(&foundFile, 0, sizeof(foundFile));
    HANDLE listFile = SFileFindFirstFile(LocaleMpq, "DBFilesClient\\*dbc", &foundFile, NULL);
    HANDLE dbcFile = NULL;
    uint32 count = 0;
    if (listFile)
    {
        std::string outputPath = output_path;
        outputPath += "/dbc/";

        CreateDir(outputPath);
        if (!basicLocale)
        {
            outputPath += Locales[l];
            outputPath += "/";
            CreateDir(outputPath);
        }

        std::string filename;

        do
        {
            if (!SFileOpenFileEx(LocaleMpq, foundFile.cFileName, SFILE_OPEN_FROM_MPQ, &dbcFile))
            {
                printf("Unable to open file %s in the archive\n", foundFile.cFileName);
                continue;
            }

            filename = foundFile.cFileName;
            filename = outputPath + filename.substr(filename.rfind('\\') + 1);
            if (FileExists(filename.c_str()))
                continue;

            if (ExtractFile(dbcFile, filename.c_str()))
                ++count;

            SFileCloseFile(dbcFile);
        } while (SFileFindNextFile(listFile, &foundFile));

        SFileFindClose(listFile);
    }

    printf("Extracted %u DBC files\n\n", count);
}

void ExtractDB2Files(int l, bool basicLocale)
{
    printf("Extracting db2 files...\n");

    SFILE_FIND_DATA foundFile;
    memset(&foundFile, 0, sizeof(foundFile));
    HANDLE listFile = SFileFindFirstFile(LocaleMpq, "DBFilesClient\\*db2", &foundFile, NULL);
    HANDLE dbcFile = NULL;
    uint32 count = 0;
    if (listFile)
    {
        std::string outputPath = output_path;
        outputPath += "/dbc/";
        if (!basicLocale)
        {
            outputPath += Locales[l];
            outputPath += "/";
        }

        std::string filename;

        do
        {
            if (!SFileOpenFileEx(LocaleMpq, foundFile.cFileName, SFILE_OPEN_FROM_MPQ, &dbcFile))
            {
                printf("Unable to open file %s in the archive\n", foundFile.cFileName);
                continue;
            }

            filename = foundFile.cFileName;
            filename = outputPath + filename.substr(filename.rfind('\\') + 1);
            if (FileExists(filename.c_str()))
                continue;

            if (ExtractFile(dbcFile, filename.c_str()))
                ++count;

            SFileCloseFile(dbcFile);
        } while (SFileFindNextFile(listFile, &foundFile));

        SFileFindClose(listFile);
    }

    printf("Extracted %u DB2 files\n\n", count);
}

bool LoadLocaleMPQFile(int locale)
{
    TCHAR buff[512];
    memset(buff, 0, sizeof(buff));
    _stprintf(buff, _T("%s/Data/%s/locale-%s.MPQ"), input_path, LocalesT[locale], LocalesT[locale]);
    if (!SFileOpenArchive(buff, 0, MPQ_OPEN_READ_ONLY, &LocaleMpq))
    {
        if (GetLastError() != ERROR_PATH_NOT_FOUND)
        {
            _tprintf(_T("\nLoading %s locale MPQs\n"), LocalesT[locale]);
            _tprintf(_T("Cannot open archive %s\n"), buff);
        }
        return false;
    }

    _tprintf(_T("\nLoading %s locale MPQs\n"), LocalesT[locale]);
    char const* prefix = NULL;
    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        // Do not attempt to read older MPQ patch archives past this build, they were merged with base
        // and trying to read them together with new base will not end well
        if (CONF_TargetBuild >= NEW_BASE_SET_BUILD && Builds[i] < NEW_BASE_SET_BUILD)
            continue;

        memset(buff, 0, sizeof(buff));
        if (Builds[i] > LAST_DBC_IN_DATA_BUILD)
        {
            prefix = "";
            _stprintf(buff, _T("%s/Data/%s/wow-update-%s-%u.MPQ"), input_path, LocalesT[locale], LocalesT[locale], Builds[i]);
        }
        else
        {
            prefix = Locales[locale];
            _stprintf(buff, _T("%s/Data/wow-update-%u.MPQ"), input_path, Builds[i]);
        }

        if (!SFileOpenPatchArchive(LocaleMpq, buff, prefix, 0))
        {
            if (GetLastError() != ERROR_FILE_NOT_FOUND)
                _tprintf(_T("Cannot open patch archive %s\n"), buff);
            continue;
        }
        else
            _tprintf(_T("Loaded %s\n"), buff);
    }

    printf("\n");
    return true;
}

bool LoadCommonMPQFiles(uint32 build)
{
    TCHAR filename[512];
    _stprintf(filename, _T("%s/Data/world.MPQ"), input_path);
    _tprintf(_T("Loading common MPQ files\n"));
    if (!SFileOpenArchive(filename, 0, MPQ_OPEN_READ_ONLY, &WorldMpq))
    {
        if (GetLastError() != ERROR_PATH_NOT_FOUND)
            _tprintf(_T("Cannot open archive %s\n"), filename);
        return false;
    }

    int count = sizeof(CONF_mpq_list) / sizeof(char*);
    for (int i = 1; i < count; ++i)
    {
        if (build < NEW_BASE_SET_BUILD && !strcmp("world2.MPQ", CONF_mpq_list[i]))   // 4.3.2 and higher MPQ
            continue;

        _stprintf(filename, _T("%s/Data/%s"), input_path, CONF_mpq_list[i]);
        if (!SFileOpenPatchArchive(WorldMpq, filename, "", 0))
        {
            if (GetLastError() != ERROR_PATH_NOT_FOUND)
                _tprintf(_T("Cannot open archive %s\n"), filename);
            else _tprintf(_T("Not found %s\n"), filename);
        } else _tprintf(_T("Loaded %s\n"), filename);
    }

    char const* prefix = NULL;
    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        // Do not attempt to read older MPQ patch archives past this build, they were merged with base
        // and trying to read them together with new base will not end well
        if (CONF_TargetBuild >= NEW_BASE_SET_BUILD && Builds[i] < NEW_BASE_SET_BUILD)
            continue;

        memset(filename, 0, sizeof(filename));
        if (Builds[i] > LAST_DBC_IN_DATA_BUILD)
        {
            prefix = "";
            _stprintf(filename, _T("%s/Data/wow-update-base-%u.MPQ"), input_path, Builds[i]);
        }
        else
        {
            prefix = "base";
            _stprintf(filename, _T("%s/Data/wow-update-%u.MPQ"), input_path, Builds[i]);
        }

        if (!SFileOpenPatchArchive(WorldMpq, filename, prefix, 0))
        {
            if (GetLastError() != ERROR_PATH_NOT_FOUND)
                _tprintf(_T("Cannot open patch archive %s\n"), filename);
            else _tprintf(_T("Not found %s\n"), filename);
            continue;
        } else _tprintf(_T("Loaded %s\n"), filename);
    }

    printf("\n");
    return true;
}

bool LoadRequiredMPQs(int local, uint32 build)
{
    bool res = false;
    if(res = LoadLocaleMPQFile(local) && build > 0)
        res = LoadCommonMPQFiles(build);
    return res;
}

int main(int argc, char * arg[])
{
    printf("Map & DBC Extractor\n");
    printf("===================\n");

    HandleArgs(argc, arg);

    int FirstLocale = -1;
    uint32 build = 0;

    for (int i = 0; i < LOCALES_COUNT; ++i)
    {
        //Open MPQs
        if (!LoadLocaleMPQFile(i))
        {
            if (GetLastError() != ERROR_PATH_NOT_FOUND)
                printf("Unable to load %s locale archives!\n", Locales[i]);
            continue;
        }

        printf("Detected locale: %s\n", Locales[i]);
        if ((CONF_extract & EXTRACT_DBC) == 0)
        {
            FirstLocale = i;
            build = ReadBuild(i);
            if (build > CONF_TargetBuild)
            {
                printf("Base locale-%s.MPQ has build higher than target build (%u > %u), nothing extracted!\n", Locales[i], build, CONF_TargetBuild);
                return 0;
            }

            printf("Detected client build: %u\n", build);
            printf("\n");
            break;
        }

        //Extract DBC files
        uint32 tempBuild = ReadBuild(i);
        printf("Detected client build %u for locale %s\n", tempBuild, Locales[i]);
        if (tempBuild > CONF_TargetBuild)
        {
            SFileCloseArchive(LocaleMpq);
            printf("Base locale-%s.MPQ has build higher than target build (%u > %u), nothing extracted!\n", Locales[i], tempBuild, CONF_TargetBuild);
            continue;
        }

        printf("\n");
        ExtractDBCFiles(i, FirstLocale < 0);
        ExtractDB2Files(i, FirstLocale < 0);

        if (FirstLocale < 0)
        {
            FirstLocale = i;
            build = tempBuild;
        }

        //Close MPQs
        SFileCloseArchive(LocaleMpq);
    }

    if (FirstLocale < 0)
    {
        printf("No locales detected\n");
        return 0;
    }

    bool loadedMPQ = false;
    if(CONF_extract & CREATE_DBCS)
    {
        printf("Using locale: %s\n", Locales[FirstLocale]);
        if(loadedMPQ = LoadRequiredMPQs(FirstLocale, build))
            CreateCustomDBCFiles();
    }

    if (CONF_extract & EXTRACT_MAP)
    {
        if(loadedMPQ == false)
        {
            printf("Using locale: %s\n", Locales[FirstLocale]);
            loadedMPQ = LoadRequiredMPQs(FirstLocale, build);
        }

        // Extract maps
        if(loadedMPQ) ExtractMapsFromMpq(build);
    }

    if(loadedMPQ)
    {
        // Close MPQs
        SFileCloseArchive(WorldMpq);
        SFileCloseArchive(LocaleMpq);
    }
    return 0;
}
