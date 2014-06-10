/*
 * Thetruecrow
 * Citric
 * CactusEMU
 */

#include "headers.h"
#include "dbc.h"
#include "map.h"
#include "misc.h"
#include "wdt.h"
#include "adt.h"
#include "MPQ.h"

#include <string>
#include <vector>

void ExtractMapsFromMpq(uint32 build)
{
    CleanCache();
    char mpq_filename[1024];
    char mpq_map_name[1024];

    printf("Extracting maps...\n");

    LoadMapMPQFiles();

    uint32 map_count = ReadMapDBC();

    ReadLiquidTypeTableDBC();

    std::string path = ".";
    path += "/maps/";
    CreateDir(path);

    std::vector<std::string> not_found;

    printf("Convert map files\n");
    for(uint32 z = 0; z < map_count; ++z)
    {
        FILE *out_file;
        char output_filename[50];
        uint32 Offsets[64][64];
        memset(Offsets, 0, sizeof(Offsets));
        sprintf_s(output_filename, "maps\\Map_%03u.bin", map_ids[z].id);

        // Loadup map grid data
        std::string mapName(map_ids[z].name);
        sprintf_s(mpq_map_name, "World\\Maps\\%s\\%s.wdt", mapName.c_str(), mapName.c_str());
        HANDLE mpq;
        if(!GetMPQHandle(mpq_map_name, mpq))
        {
            printf("Skipping map %u - %s, no WDT\n", z+1, mapName.c_str());
            continue;
        }

        WDT_file wdt(mpq_map_name, mpq);
        if (wdt.isEof())
        {
            not_found.push_back(map_ids[z].name);
            printf("Extract %s (%d/%d) -- not found\n", mapName.c_str(), z+1, map_count);
            printf("Creating dummy bin file %s.\n", output_filename);

            fopen_s(&out_file, output_filename, "wb");
            if(out_file)
            {
                fwrite(Offsets, sizeof(Offsets), 1, out_file);
                fclose(out_file);
            }
            continue;
        }

        printf("Creating output file %s.\n", output_filename);
        fopen_s(&out_file, output_filename, "wb");
        if(!out_file)
        {
            printf("Could not create output file!\n");
            fclose(out_file);
            continue;
        }

        printf("Writing empty index to the beginning of the file...\n");
        fwrite(Offsets, sizeof(Offsets), 1, out_file);

        wdt.prepareLoadedData();
        printf("Extracting data... \n");
        uint32 start_time = GetTickCount();
        for(uint32 x = 0; x < WDT_MAP_SIZE; ++x)
        {
            for(uint32 y = 0; y < WDT_MAP_SIZE; ++y)
            {
                // For some odd reason, this stuff is reversed.. who knows why..
                sprintf_s(mpq_filename, "World\\Maps\\%s\\%s_%u_%u.adt", mapName.c_str(), mapName.c_str(), y, x);
                StoreADTData(mpq, mpq_filename, x, y, build);
            }
        }

        printf("Appending data... \n");
        fseek(out_file, 0, SEEK_END);
        for(uint32 x = 0; x < WDT_MAP_SIZE; ++x)
        {
            for(uint32 y = 0; y < WDT_MAP_SIZE; ++y)
            {
                uint32 Offset = ftell(out_file);
                if(ConvertADT(x, y, build, out_file))
                    Offsets[x][y] = Offset;
            }
        }
        CleanCache();
        printf("Data Extraction Finished in %ums. Appending header to start of file...\n", GetTickCount() - start_time);
        fseek(out_file, 0, SEEK_SET);
        fwrite(Offsets, sizeof(Offsets), 1, out_file);
        printf("Closing output file.\n");
        fclose(out_file);
    }
    printf("\n");
    delete [] map_ids;
}

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08

#define MAP_LIQUID_TYPE_DARK_WATER  0x10
#define MAP_LIQUID_TYPE_WMO_WATER   0x20

struct celldata
{
    // Temporary grid data store
    uint16 area_flags[ADT_CELLS_PER_GRID*ADT_CELLS_PER_GRID];
    uint16 liquid_type[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];

    float V8[ADT_GRID_SIZE][ADT_GRID_SIZE];
    float V9[ADT_GRID_SIZE+1][ADT_GRID_SIZE+1];

    bool  liquid_show[ADT_GRID_SIZE][ADT_GRID_SIZE];
    float liquid_height[ADT_GRID_SIZE+1][ADT_GRID_SIZE+1];
};

celldata* m_cells[64][64];

void CleanCache()
{
    for(uint32 i = 0; i < 64; ++i)
    {
        for(uint32 j = 0; j < 64; ++j)
        {
            if(m_cells[i][j] != 0)
            {
                delete m_cells[i][j];
                m_cells[i][j] = 0;
            }
        }
    }
}

// This option allow limit minimum height to some value (Allow save some memory)
bool  CONF_allow_height_limit = true;
float CONF_use_minHeight = -500.0f;

// This option allow use float to int conversion
bool  CONF_allow_float_to_int   = true;
float CONF_float_to_int8_limit  = 2.0f;         // Max accuracy = val/256
float CONF_float_to_int16_limit = 2048.0f;      // Max accuracy = val/65536
float CONF_flat_height_delta_limit = 0.005f;    // If max - min less this value - surface is flat
float CONF_flat_liquid_delta_limit = 0.001f;    // If max - min less this value - liquid surface is flat

bool StoreADTData(HANDLE mpqarchive, char *filename, int cell_x, int cell_y, uint32 build)
{
    ADT_file adt(filename, mpqarchive);
    if (adt.isEof())
        return false;

    if(m_cells[cell_x][cell_y] == NULL)
    {
        m_cells[cell_x][cell_y] = new celldata();
        memset(m_cells[cell_x][cell_y]->area_flags, 0, sizeof(m_cells[cell_x][cell_y]->area_flags));
        memset(m_cells[cell_x][cell_y]->V8, 0, sizeof(m_cells[cell_x][cell_y]->V8));
        memset(m_cells[cell_x][cell_y]->V9, 0, sizeof(m_cells[cell_x][cell_y]->V9));

        memset(m_cells[cell_x][cell_y]->liquid_type, 0, sizeof(m_cells[cell_x][cell_y]->liquid_type));
        memset(m_cells[cell_x][cell_y]->liquid_show, 0, sizeof(m_cells[cell_x][cell_y]->liquid_show));
        memset(m_cells[cell_x][cell_y]->liquid_height, 0, sizeof(m_cells[cell_x][cell_y]->liquid_height));
    }

    adt.prepareLoadedData();

    //
    // Get Height map from grid
    //
    for (int i=0;i<ADT_CELLS_PER_GRID;i++)
    {
        for(int j=0;j<ADT_CELLS_PER_GRID;j++)
        {
            adt_MCNK * cell = adt.getMCNK(i,j);
            if (!cell)
                continue;
            m_cells[cell_x][cell_y]->area_flags[i*16+j] = cell->areaid;

            // Height values for triangles stored in order:
            // 1        2       3       4       5       6       7       8       9
            //  10  11  12  13  14  15  16  17
            // 18   19  20  21  22  23  24  25  26
            //  27  28  29  30  31  32  33  34
            // . . . . . . . .
            // For better get height values merge it to V9 and V8 map
            // V9 height map:
            // 1        2       3       4       5       6       7       8       9
            // 18   19  20  21  22  23  24  25  26
            // . . . . . . . .
            // V8 height map:
            //  10  11  12  13  14  15  16  17
            //  27  28  29  30  31  32  33  34
            // . . . . . . . .

            // Set map height as grid height
            for (int y=0; y <= ADT_CELL_SIZE; y++)
            {
                int cy = i*ADT_CELL_SIZE + y;
                for (int x=0; x <= ADT_CELL_SIZE; x++)
                {
                    int cx = j*ADT_CELL_SIZE + x;
                    m_cells[cell_x][cell_y]->V9[cy][cx]=cell->ypos;
                }
            }
            for (int y=0; y < ADT_CELL_SIZE; y++)
            {
                int cy = i*ADT_CELL_SIZE + y;
                for (int x=0; x < ADT_CELL_SIZE; x++)
                {
                    int cx = j*ADT_CELL_SIZE + x;
                    m_cells[cell_x][cell_y]->V8[cy][cx]=cell->ypos;
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
                    m_cells[cell_x][cell_y]->V9[cy][cx]+=v->height_map[y*(ADT_CELL_SIZE*2+1)+x];
                }
            }
            // get V8 height map
            for (int y=0; y < ADT_CELL_SIZE; y++)
            {
                int cy = i*ADT_CELL_SIZE + y;
                for (int x=0; x < ADT_CELL_SIZE; x++)
                {
                    int cx = j*ADT_CELL_SIZE + x;
                    m_cells[cell_x][cell_y]->V8[cy][cx]+=v->height_map[y*(ADT_CELL_SIZE*2+1)+ADT_CELL_SIZE+1+x];
                }
            }
        }
    }

    // Get liquid map for grid (in WOTLK used MH2O chunk)
    adt_MH2O * h2o = adt.a_grid->getMH2O();
    if (h2o)
    {
        for (int i=0;i<ADT_CELLS_PER_GRID;i++)
        {
            for(int j=0;j<ADT_CELLS_PER_GRID;j++)
            {
                adt_liquid_header *h = h2o->getLiquidData(i,j);
                if (!h)
                    continue;

                int count = 0;
                uint64 show = h2o->getLiquidShowMap(h);
                for (int y=0; y < h->height;y++)
                {
                    int cy = i*ADT_CELL_SIZE + y + h->yOffset;
                    for (int x=0; x < h->width; x++)
                    {
                        int cx = j*ADT_CELL_SIZE + x + h->xOffset;
                        if (show & 1)
                        {
                            m_cells[cell_x][cell_y]->liquid_show[cy][cx] = true;
                            ++count;
                        }
                        show>>=1;
                    }
                }

                uint32 type = LiqType[h->liquidType];
                switch (type)
                {
                    case LIQUID_TYPE_WATER: m_cells[cell_x][cell_y]->liquid_type[i][j] |= MAP_LIQUID_TYPE_WATER; break;
                    case LIQUID_TYPE_OCEAN: m_cells[cell_x][cell_y]->liquid_type[i][j] |= MAP_LIQUID_TYPE_OCEAN; break;
                    case LIQUID_TYPE_MAGMA: m_cells[cell_x][cell_y]->liquid_type[i][j] |= MAP_LIQUID_TYPE_MAGMA; break;
                    case LIQUID_TYPE_SLIME: m_cells[cell_x][cell_y]->liquid_type[i][j] |= MAP_LIQUID_TYPE_SLIME; break;
                    default:
                        printf("\nCan't find Liquid type %u for map %s\nchunk %d,%d\n", h->liquidType, filename, i, j);
                        break;
                }
                // Dark water detect
                if (type == LIQUID_TYPE_OCEAN)
                {
                    uint8 *lm = h2o->getLiquidLightMap(h);
                    if (!lm)
                        m_cells[cell_x][cell_y]->liquid_type[i][j]|=MAP_LIQUID_TYPE_DARK_WATER;
                }

                if (!count && m_cells[cell_x][cell_y]->liquid_type[i][j])
                    printf("Wrong liquid detect in MH2O chunk");

                float *height = h2o->getLiquidHeightMap(h);
                int pos = 0;
                for (int y=0; y<=h->height;y++)
                {
                    int cy = i*ADT_CELL_SIZE + y + h->yOffset;
                    for (int x=0; x<= h->width; x++)
                    {
                        int cx = j*ADT_CELL_SIZE + x + h->xOffset;
                        if (height)
                            m_cells[cell_x][cell_y]->liquid_height[cy][cx] = height[pos];
                        else
                            m_cells[cell_x][cell_y]->liquid_height[cy][cx] = h->heightLevel1;
                        pos++;
                    }
                }
            }
        }
    }
    else
    {
        // Get from MCLQ chunk (old)
        for (int i=0;i<ADT_CELLS_PER_GRID;i++)
        {
            for(int j=0;j<ADT_CELLS_PER_GRID;j++)
            {
                adt_MCNK *cell = adt.getMCNK(i, j);
                if (!cell)
                    continue;

                adt_MCLQ *liquid = cell->getMCLQ();
                int count = 0;
                if (!liquid || cell->sizeMCLQ <= 8)
                    continue;

                for (int y=0; y < ADT_CELL_SIZE; y++)
                {
                    int cy = i*ADT_CELL_SIZE + y;
                    for (int x=0; x < ADT_CELL_SIZE; x++)
                    {
                        int cx = j*ADT_CELL_SIZE + x;
                        if (liquid->flags[y][x] != 0x0F)
                        {
                            m_cells[cell_x][cell_y]->liquid_show[cy][cx] = true;
                            if (liquid->flags[y][x]&(1<<7))
                                m_cells[cell_x][cell_y]->liquid_type[i][j]|=MAP_LIQUID_TYPE_DARK_WATER;
                            ++count;
                        }
                    }
                }

                uint32 c_flag = cell->flags;
                if(c_flag & (1<<2))
                    m_cells[cell_x][cell_y]->liquid_type[i][j]|=MAP_LIQUID_TYPE_WATER;          // water
                if(c_flag & (1<<3))
                    m_cells[cell_x][cell_y]->liquid_type[i][j]|=MAP_LIQUID_TYPE_OCEAN;          // ocean
                if(c_flag & (1<<4))
                    m_cells[cell_x][cell_y]->liquid_type[i][j]|=MAP_LIQUID_TYPE_MAGMA;          // magma/slime

                if (!count && m_cells[cell_x][cell_y]->liquid_type[i][j])
                    printf("Wrong liquid detect in MCLQ chunk");

                for (int y=0; y <= ADT_CELL_SIZE; y++)
                {
                    int cy = i*ADT_CELL_SIZE + y;
                    for (int x=0; x<= ADT_CELL_SIZE; x++)
                    {
                        int cx = j*ADT_CELL_SIZE + x;
                        m_cells[cell_x][cell_y]->liquid_height[cy][cx] = liquid->liquid[y][x].height;
                    }
                }
            }
        }
    }

    return true;
}

bool ConvertADT(int cell_x, int cell_y, uint32 build, FILE * output)
{
    if(m_cells[cell_x][cell_y] == NULL)
        return false;

    fwrite(&m_cells[cell_x][cell_y]->area_flags, sizeof(uint16), 256, output);
    fwrite(&m_cells[cell_x][cell_y]->liquid_type, sizeof(uint16), 256, output);
    fwrite(&m_cells[cell_x][cell_y]->V8, sizeof(float), 128*128, output);
    fwrite(&m_cells[cell_x][cell_y]->V9, sizeof(float), 129*129, output);
    fwrite(&m_cells[cell_x][cell_y]->liquid_height, sizeof(float), 129*129, output);
    return true;
}
