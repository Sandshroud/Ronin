/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

static const uint32 terrainHeaderSize = sizeof(uint32)*64*64; // size of [64][64] array.
static const char *heightMapHeader = "HMAP434_2";

#define NO_LAND_HEIGHT 999999.0f
#define NO_WATER_HEIGHT -50000.0f

#define TERRAIN_TILE_SIZE 533.3333f

#define TILE_COUNT 64
#define CHUNK_COUNT 64*16
#define MAP_SCALE 64*16*9

enum MapCompressionType : uint32
{
    MAP_RAW_TILES = 1,
    MAP_RAW_CHUNKS,
    MAP_COMPRESSED_TILES,
    MAP_COMPRESSED_CHUNKS,
    MAP_COMPRESSED_MAX
};

#define MAP_HEIGHT_NO_HEIGHT    0x0001
#define MAP_HEIGHT_AS_INT16     0x0002
#define MAP_HEIGHT_AS_INT8      0x0004
#define MAP_HEIGHT_INVALID      0x00FF

class MapDataLoader
{
    uint32 loadCounter;
public:
    MapDataLoader() : loadCounter(0) {};
    ~MapDataLoader() {};

    virtual bool LoadMapInforation(uint32 x, uint32 y, FILE *input, uint32 compressionType)
    {
        ++loadCounter;
        return true;
    }

    bool UnloadData()
    {
        if(loadCounter--)
            return false;
        return true;
    }

    virtual uint16 GetAreaId(float x, float y) = 0;
    virtual float GetTerrainHeight(float x, float y) = 0;

    virtual uint16 GetWaterType(float x, float y) = 0;
    virtual float GetWaterHeight(float x, float y) = 0;

    virtual bool CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &areaId) = 0;
};

/* @class TerrainMgr

   TerrainMgr can dynamically allocate and un-allocate tile information for main
   continents as their information is *quite* large and not needed at all times
   to be loaded. Unloading this in idle times is a nice way to save memory.

   However, on instanced maps, we would want to keep the tile's information
   loaded at all times as it is a lot smaller and we can have multiple instances
   wanting to access this information at once.
  */

class SERVER_DECL TerrainMgr
{
public:
    /* Initializes the terrain interface, allocates all required arrays, and sets
       all variables.
       Parameter 1: The path to the packed map files.
       Parameter 2: The map that we'll be retrieving information from.
       Parameter 3: Controls whether the map will unload information when it's not
                    in use.
       No return value.
      */
    TerrainMgr(std::string MapPath, uint32 MapId);

    /* Cleans up all arrays, and unloads any pending tile information.
       No parameters.
       No return value.
      */
    ~TerrainMgr();

    /* Loads the tile information if it has not already been loaded.
       Parameter 1: The x co-ordinate of the tile that's gone active.
       Parameter 2: The y co-ordinate of the tile that's gone active.
       No return value.
      */
    void CellGoneActive(uint32 x, uint32 y);

    /* If we're a non-instanced map, we'll unload the tile information as it's
       not needed.
       Parameter 1: The x co-ordinate of the tile that's gone idle.
       Parameter 2: The y co-ordinate of the tile that's gone idle.
       No return value.
      */
    void CellGoneIdle(uint32 x, uint32 y);

    /* Loads all tile information
       No return value.
      */
    void LoadAllTerrain();

    /* Unloads all tile information
       No return value.
      */
    void UnloadAllTerrain(bool forced);

    /* Information retrieval functions
       These functions all take the same input values, an x and y global co-ordinate.
       They will all return 0 if the tile information is not loaded or does not exist,
       apart from the water function which will return '-500000.0'.
      */
    float  GetLandHeight(float x, float y);
    float  GetWaterHeight(float x, float y);
    uint16  GetWaterType(float x, float y);
    uint8  GetWalkableState(float x, float y);
    uint16 GetAreaID(float x, float y);
    bool CellHasAreaID(uint32 x, uint32 y, uint16 &AreaID);

private:
    /// File name for opening the file on load
    std::string file_name;

    /// Map ID
    uint32 mapId;

    /// We don't want to be reading from a file from more than one thread at once
    Mutex mutex;

    // Will ignore loading any terrain
    bool dummyMap;

    /// This holds the offsets of the tile information for each tile.
    std::map<std::pair<uint8, uint8>, uint32> m_tileOffsets;

    /// Load counter
    uint32 LoadCounter[TILE_COUNT][TILE_COUNT];

    /// Our storage array. This contains pointers to all allocated TileInfo's.
    std::map<std::pair<uint8, uint8>, MapDataLoader*> tileInformation;
    
    /// Potential implementation
    MapDataLoader *m_dataLoader;

    /// Our vmap management offset, stored for later activation
    uint32 m_vmapOffset;

    /// Our map tiles can be compressed or decompressed for debug purposes
    uint32 mapCompessionType;

public:
    /* Initializes the file descriptor and readys it for data retreival.
       No parameters taken.
       Returns true if the index was read successfully, false if not.
      */
    bool LoadTerrainHeader();

    /* Initializes non terrain offset data
      */
    bool LoadVMapTerrain();

    /* Checks that the co-ordinates are within range.
      */
    RONIN_INLINE static bool AreCoordinatesValid(float x, float y)
    {
        if(x > _maxX || x < _minX)
            return false;
        if(y > _maxY || y < _minY)
            return false;
        if(num_isnan(x) || num_isnan(y))
            return false;
        if(x > std::numeric_limits<float>::max())
            return false;
        if(y > std::numeric_limits<float>::max())
            return false;
        return true;
    }

    /* Converts a global x co-ordinate into a tile x co-ordinate.
       Parameter 1: global x co-ordinate.
       Returns the tile x co-ordinate.
      */
    RONIN_INLINE static uint32 ConvertGlobalXCoordinate(float x) { return int32(32-(x/TERRAIN_TILE_SIZE)); }

    /* Converts a global y co-ordinate into a tile y co-ordinate.
       Parameter 1: global y co-ordinate.
       Returns the tile y co-ordinate.
    */
    RONIN_INLINE static uint32 ConvertGlobalYCoordinate(float y) { return int32(32-(y/TERRAIN_TILE_SIZE)); }

protected:
    /* Retrieves the tile data for the specified co-ordinates from the file and sets it in
       the TileInformation array.
       Parameter 1: x co-ordinate of the tile information to load.
       Parameter 2: y co-ordinate of the tile information to load.
       Returns true if the tile information exists and was loaded, false if not.
      */
    bool LoadTileInformation(uint32 x, uint32 y, FILE *input);

    /* Unloads the tile data at the specified co-ordinates and frees the memory.
       Parameter 1: x co-ordinate of the tile information to free.
       Parameter 2: y co-ordinate of the tile information to free.
       No return value.
      */
    void UnloadTileInformation(uint32 x, uint32 y);

    /* Gets a tile information pointer so that another function can access its data.
       Parameter 1: tile x co-ordinate.
       Parameter 2: tile y co-ordinate.
       Returns the memory address of the information for that tile.
      */
    RONIN_INLINE MapDataLoader* GetTileInformation(uint32 x, uint32 y)
    {
        std::pair<uint8, uint8> tilePair = std::make_pair(x, y);
        if(tileInformation.find(tilePair) == tileInformation.end())
            return NULL;
        return tileInformation.at(tilePair);
    }

    /* Checks whether a tile information is loaded or not.
      */
    RONIN_INLINE bool _TileInformationLoaded(uint32 x, uint32 y) { return tileInformation.find(std::make_pair(x, y)) != tileInformation.end(); }
};

class MapRawTileDataLoader : public MapDataLoader
{
private:
    static const int CHUNK_SIZE = 16;
    static const int CHUNK_SIZE_SQ = CHUNK_SIZE*CHUNK_SIZE;

    static const int V9_SIZE = (CHUNK_SIZE*8)+1;
    static const int V9_SIZE_SQ = V9_SIZE*V9_SIZE;
    static const int V8_SIZE = CHUNK_SIZE*8;
    static const int V8_SIZE_SQ = V8_SIZE*V8_SIZE;

public:
    MapRawTileDataLoader();
    ~MapRawTileDataLoader();

    bool LoadMapInforation(uint32 x, uint32 y, FILE *input, uint32 compressionType);

    uint16 GetAreaId(float x, float y);
    float GetTerrainHeight(float x, float y);

    uint16 GetWaterType(float x, float y);
    float GetWaterHeight(float x, float y);

    bool CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &areaId);

private:
    uint16 areaInfo[CHUNK_SIZE*CHUNK_SIZE];
    uint16 liquidType[CHUNK_SIZE*CHUNK_SIZE];

    float V8[V8_SIZE*V8_SIZE], V9[V9_SIZE*V9_SIZE];
    float liquidHeight[V9_SIZE*V9_SIZE];
};

class MapRawChunkDataLoader : public MapDataLoader
{
private:
    static const int CHUNK_SIZE = 16;
    static const int CHUNK_SIZE_SQ = CHUNK_SIZE*CHUNK_SIZE;

    static const int V9_SIZE = 9;
    static const int V9_SIZE_SQ = V9_SIZE*V9_SIZE;
    static const int V8_SIZE = 8;
    static const int V8_SIZE_SQ = V8_SIZE*V8_SIZE;

public:
    MapRawChunkDataLoader();
    ~MapRawChunkDataLoader();

    bool LoadMapInforation(uint32 x, uint32 y, FILE *input, uint32 compressionType);

    uint16 GetAreaId(float x, float y);
    float GetTerrainHeight(float x, float y);

    uint16 GetWaterType(float x, float y);
    float GetWaterHeight(float x, float y);

    bool CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &areaId);

private:

    uint16 areaInfo[CHUNK_SIZE][CHUNK_SIZE];
    uint16 liquidType[CHUNK_SIZE][CHUNK_SIZE];
    struct chunkInfo
    {
        float cxpos, cypos;
        float T8[V8_SIZE][V8_SIZE], T9[V9_SIZE][V9_SIZE];
        float liquidHeight[V9_SIZE][V9_SIZE];
    }mChunkInfo[CHUNK_SIZE][CHUNK_SIZE];
};

class MapRawDataLoader : public MapDataLoader
{
private:
    static const int CHUNK_SIZE = 16;
    static const int DATA_SIZE = TILE_COUNT*CHUNK_SIZE;
    static const int DATA_SIZE_SQ = DATA_SIZE*DATA_SIZE;
    static const int DATA_CENTER_ID = DATA_SIZE/2;

    static const float DATA_CHUNK_SIZE;
    static const float DATA_CHUNK_V8_SIZE;
    static const float DATA_CHUNK_V9_SIZE;

    static const int V9_SIZE = (DATA_SIZE*8)+1;
    static const int V9_SIZE_SQ = V9_SIZE*V9_SIZE;
    static const int V8_SIZE = DATA_SIZE*8;
    static const int V8_SIZE_SQ = V8_SIZE*V8_SIZE;

    static const int READ_CHUNK_SIZE = 16;
    static const int READ_CHUNK_SIZE_SQ = READ_CHUNK_SIZE*READ_CHUNK_SIZE;

    static const int READ_V9_SIZE = 9;
    static const int READ_V9_SIZE_SQ = READ_V9_SIZE*READ_V9_SIZE;
    static const int READ_V8_SIZE = 8;
    static const int READ_V8_SIZE_SQ = READ_V8_SIZE*READ_V8_SIZE;
public:
    MapRawDataLoader();
    ~MapRawDataLoader();

    bool LoadMapInforation(uint32 x, uint32 y, FILE *input, uint32 compressionType);

    uint16 GetAreaId(float x, float y);
    float GetTerrainHeight(float x, float y);

    uint16 GetWaterType(float x, float y);
    float GetWaterHeight(float x, float y);

    bool CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &areaId);

private:
    std::map<uint32, std::map<uint32, uint16>> m_areaInfo, m_liquidInfo;

    struct heightPlane
    {
        float xmin[8][8], xmax[8][8];
        float ymin[8][8], ymax[8][8];

        enum heights
        {
            TOP_LEFT = 0,
            TOP_RIGHT = 1,
            BOTTOM_LEFT = 2,
            BOTTOM_RIGHT = 3,
            CENTER = 4,
            MAX
        };
        float Theight[8][8][heights::MAX];
        float Lheight[8][8][heights::MAX];

        void init()
        {
            memset(&xmin, 0, sizeof(float)*8*8);
            memset(&xmax, 0, sizeof(float)*8*8);
            memset(&ymin, 0, sizeof(float)*8*8);
            memset(&ymax, 0, sizeof(float)*8*8);
            memset(&Theight, 0, sizeof(float)*8*8*heights::MAX);
            memset(&Lheight, 0, sizeof(float)*8*8*heights::MAX);
        }

        bool isInPlane(uint8 tx, uint8 ty, float x, float y)
        {
            return x > xmin[tx][ty] && x < xmax[tx][ty] && y > ymin[tx][ty] && y < ymax[tx][ty];
        }
    }; //std::map<uint32, std::map<uint32, heightPlane>> m_heightPlanes;

    struct heightPlaneStack
    {
        float xmin[16][16], xmax[16][16];
        float ymin[16][16], ymax[16][16];

        void init()
        {
            memset(&xmin, 0, sizeof(float)*16*16);
            memset(&xmax, 0, sizeof(float)*16*16);
            memset(&ymin, 0, sizeof(float)*16*16);
            memset(&ymax, 0, sizeof(float)*16*16);
            for(uint8 x = 0; x < 16; x++)
                for(uint8 y = 0; y < 16; y++)
                    _heightPlanes[x][y].init();
        }

        bool isInPlane(uint8 tx, uint8 ty, float x, float y)
        {
            return x > xmin[tx][ty] && x < xmax[tx][ty] && y > ymin[tx][ty] && y < ymax[tx][ty];
        }

        heightPlane _heightPlanes[16][16];
    }m_heightPlanes;
};

class MapCompressedChunkDataLoader : public MapDataLoader
{
private:
    static const int CHUNK_SIZE = 16;
    static const int CHUNK_SIZE_SQ = CHUNK_SIZE*CHUNK_SIZE;

    static const int V9_SIZE = (CHUNK_SIZE*8)+1;
    static const int V9_SIZE_SQ = V9_SIZE*V9_SIZE;
    static const int V8_SIZE = CHUNK_SIZE*8;
    static const int V8_SIZE_SQ = V8_SIZE*V8_SIZE;

    static const float DATA_CHUNK_V8_SIZE;

public:
    MapCompressedChunkDataLoader();
    ~MapCompressedChunkDataLoader();

    bool LoadMapInforation(uint32 x, uint32 y, FILE *input, uint32 compressionType);

    uint16 GetAreaId(float x, float y);
    float GetTerrainHeight(float x, float y);

    uint16 GetWaterType(float x, float y);
    float GetWaterHeight(float x, float y);

    bool CellHasAreaID(uint32 CellX, uint32 CellY, uint16 &areaId);

private:
    uint16 areaInfo[CHUNK_SIZE][CHUNK_SIZE];
    float data_V8[V8_SIZE][V8_SIZE], data_V9[V9_SIZE][V9_SIZE], data_L9[V9_SIZE][V9_SIZE];

    struct LiquidChunk
    {
        uint16 type;
        float minHeight;
    } *m_liquidChunk[CHUNK_SIZE][CHUNK_SIZE];
};
