/***
 * Demonstrike Core
 */

#pragma once

typedef struct
{
    uint16 areaInfo;
    float mapHeight[3]; float heightMultipier;

    struct sAI { uint16 AI[16*16]; } *short_AI;
    struct mAI { uint16 mask[16]; } *masked_AI;

    struct bV8 { uint8 V8[128*128]; } *byte_V8;
    struct sV8 { uint16 V8[128*128]; } *short_V8;
    struct fV8 { float V8[128*128]; } *float_V8;

    struct bV9 { uint8 V9[129*129]; } *byte_V9;
    struct sV9 { uint16 V9[129*129]; } *short_V9;
    struct fV9 { float V9[129*129]; } *float_V9;

    struct sHI { uint16 HI[16*16]; } *short_HI;

    struct sLE { uint16 LE[16*16]; } *short_LE;
    struct bLI { uint8 LI[16*16]; } *byte_LI;
    struct mLI { uint16 LI[16]; } *mask_LI;
    float *m_liquidHeight;
    uint8 liquidData[5];
}TileTerrainInformation;

#define TERRAIN_HEADER_SIZE 16384    // size of [64][64] array.
#define NO_LAND_HEIGHT 999999.0f
#define NO_WATER_HEIGHT -50000.0f

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

    /* If we're a non-instanced map, we'll unload the tile information as it's
       not needed.
       Parameter 1: The x co-ordinate of the tile that's gone idle.
       Parameter 2: The y co-ordinate of the tile that's gone idle.
       No return value.
      */
    void CellGoneIdle(uint32 x, uint32 y);

    /* Loads the tile information if it has not already been loaded.
       Parameter 1: The x co-ordinate of the tile that's gone active.
       Parameter 2: The y co-ordinate of the tile that's gone active.
       No return value.
      */
    void CellGoneActive(uint32 x, uint32 y);

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
    float  GetWaterHeight(float x, float y, float z);
    uint8  GetWaterType(float x, float y);
    uint8  GetWalkableState(float x, float y);
    uint16 GetAreaID(float x, float y, float z);
    bool CellHasAreaID(uint32 x, uint32 y, uint16 &AreaID);

private:
    /// MapPath contains the location of all mapfiles.
    std::string mapPath;

    /// Map ID
    uint32 mapId;

    /// We don't want to be reading from a file from more than one thread at once
    Mutex mutex;

    /// Our main file descriptor for accessing the binary terrain file.
    FILE * FileDescriptor;

    /// This holds the offsets of the tile information for each tile.
    std::map<std::pair<uint8, uint8>, uint32> m_tileOffsets;

    /// Load counter
    uint32 LoadCounter[64][64];

    /// Our storage array. This contains pointers to all allocated TileInfo's.
    std::map<std::pair<uint8, uint8>, TileTerrainInformation> tileInformation;

public:
    /* Initializes the file descriptor and readys it for data retreival.
       No parameters taken.
       Returns true if the index was read successfully, false if not.
      */
    bool LoadTerrainHeader();

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

protected:
    /* Retrieves the tile data for the specified co-ordinates from the file and sets it in
       the TileInformation array.
       Parameter 1: x co-ordinate of the tile information to load.
       Parameter 2: y co-ordinate of the tile information to load.
       Returns true if the tile information exists and was loaded, false if not.
      */
    bool LoadTileInformation(uint32 x, uint32 y);

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
    RONIN_INLINE TileTerrainInformation* GetTileInformation(uint32 x, uint32 y)
    {
        std::pair<uint8, uint8> tilePair = std::make_pair(x, y);
        if(tileInformation.find(tilePair) == tileInformation.end())
            return NULL;
        return &tileInformation.at(tilePair);
    }

    /* Converts a global x co-ordinate into a tile x co-ordinate.
       Parameter 1: global x co-ordinate.
       Returns the tile x co-ordinate.
      */
    RONIN_INLINE uint32 ConvertGlobalXCoordinate(float x)
    {
        return int32(32-(x/533.33333f));
    }

    /* Converts a global y co-ordinate into a tile y co-ordinate.
       Parameter 1: global y co-ordinate.
       Returns the tile y co-ordinate.
    */
    RONIN_INLINE uint32 ConvertGlobalYCoordinate(float y)
    {
        return int32(32-(y/533.33333f));
    }

    /* Checks whether a tile information is loaded or not.
      */
    RONIN_INLINE bool TileInformationLoaded(uint32 x, uint32 y)
    {
        if(tileInformation.empty())
            return false;
        std::pair<uint8, uint8> tilePair = std::make_pair(x, y);
        if(tileInformation.find(tilePair) == tileInformation.end())
            return false;
        return true;
    }
};
