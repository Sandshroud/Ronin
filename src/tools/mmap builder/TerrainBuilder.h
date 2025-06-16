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

#ifndef _MMAP_TERRAIN_BUILDER_H
#define _MMAP_TERRAIN_BUILDER_H

#include "PathCommon.h"
#include <vmaplib\VMapLib.h>

#include "Array.h"
#include "Vector3.h"
#include "Matrix3.h"

class VMAP::VMapManager;

namespace MMAP
{
    enum Spot
    {
        TOP     = 1,
        RIGHT   = 2,
        LEFT    = 3,
        BOTTOM  = 4,
        CENTER  = 5,
        ENTIRE  = 6,

        LOAD_CORNER1, // Top Left
        LOAD_CORNER2, // Top Right
        LOAD_CORNER3, // Bottom Left
        LOAD_CORNER4  // Bottom Right
    };

    enum MapCompressionType : G3D::uint32
    {
        MAP_RAW_TILES = 1,
        MAP_RAW_CHUNKS,
        MAP_COMPRESSED_TILES,
        MAP_COMPRESSED_CHUNKS,
        MAP_COMPRESSED_MAX
    };

    static const int CHUNK_SIZE = 16;

    static const int V9_CH_SIZE = 9;
    static const int V9_CH_SIZE_SQ = V9_CH_SIZE*V9_CH_SIZE;
    static const int V8_CH_SIZE = 8;
    static const int V8_CH_SIZE_SQ = V8_CH_SIZE*V8_CH_SIZE;
    static const int CHUNK_DATA_SIZE = (CHUNK_SIZE+2)*(CHUNK_SIZE+2);

    static const int V9_SIZE = 129;
    static const int V9_SIZE_SQ = V9_SIZE*V9_SIZE;
    static const int V8_SIZE = 128;
    static const int V8_SIZE_SQ = V8_SIZE*V8_SIZE;

    static const float GRID_SIZE = 533.33333f;
    static const float GRID_PART_SIZE = GRID_SIZE/V9_SIZE;
    static const float GRID_CHUNK_SIZE = GRID_SIZE/CHUNK_SIZE;

    // see contrib/extractor/system.cpp, CONF_use_minHeight
    static const float INVALID_MAP_LIQ_HEIGHT = -2000.f;
    static const float INVALID_MAP_LIQ_HEIGHT_MAX = 5000.0f;

    // see following files:
    // contrib/extractor/system.cpp
    // src/game/Map.cpp
    struct MapTileData
    {
        G3D::uint16 areaInfo[CHUNK_SIZE][CHUNK_SIZE];
        G3D::uint16 liquidType[CHUNK_SIZE][CHUNK_SIZE];

        // Terrain V8, V9, and Liquid L8, L9
        float V8[V8_SIZE][V8_SIZE], V9[V9_SIZE][V9_SIZE], L8[V8_SIZE][V8_SIZE], L9[V9_SIZE][V9_SIZE];
    };

    struct MapLoadData
    {
        uint32 loadInfoCount;
        struct ChunkInfo// Chunk data is 16 chunks per tile plus portions from surrounding tiles
        {
            uint16 areaInfo;
            uint16 liquidType;
            uint8 xOffset, yOffset;
            bool isHole;

            float min[3], max[3];

            float xV8[V8_CH_SIZE][V8_CH_SIZE], yV8[V8_CH_SIZE][V8_CH_SIZE];
            float xV9[V9_CH_SIZE][V9_CH_SIZE], yV9[V9_CH_SIZE][V9_CH_SIZE];

            bool useLiquid[V8_CH_SIZE*V8_CH_SIZE*4], useLiquidV9[V9_CH_SIZE][V9_CH_SIZE];
            float zT8[V8_CH_SIZE][V8_CH_SIZE], zT9[V9_CH_SIZE][V9_CH_SIZE];
            float zL8[V8_CH_SIZE][V8_CH_SIZE], zL9[V9_CH_SIZE][V9_CH_SIZE];

            G3D::Array<int> solidTris, liquidTris;
            G3D::Array<float> solidVerts, liquidVerts;
            G3D::Array<int> combinedTris;
            G3D::Array<float> combinedVerts;
            G3D::Array<unsigned char> combinedFlags;
        } _chunkData[CHUNK_SIZE+2][CHUNK_SIZE+2];

        G3D::Array<float> solidVerts, liquidVerts;

        G3D::Array<G3D::uint8> vmapLiquidTypes;
        G3D::Array<int> vmapSolidTris, vmapLiquidTris;
        G3D::Array<float> vmapSolidVerts, vmapLiquidVerts;

        // offmesh connection data
        G3D::Array<float> offMeshConnections;   // [p0y,p0z,p0x,p1y,p1z,p1x] - per connection
        G3D::Array<float> offMeshConnectionRads;
        G3D::Array<unsigned char> offMeshConnectionDirs;
        G3D::Array<unsigned char> offMeshConnectionsAreas;
        G3D::Array<unsigned short> offMeshConnectionsFlags;
    };

    struct MeshData
    {
        G3D::Array<float> solidVerts;
        G3D::Array<int> solidTris;

        G3D::Array<float> liquidVerts;
        G3D::Array<int> liquidTris;
        G3D::Array<G3D::uint8> liquidType;

        // offmesh connection data
        G3D::Array<float> offMeshConnections;   // [p0y,p0z,p0x,p1y,p1z,p1x] - per connection
        G3D::Array<float> offMeshConnectionRads;
        G3D::Array<unsigned char> offMeshConnectionDirs;
        G3D::Array<unsigned char> offMeshConnectionsAreas;
        G3D::Array<unsigned short> offMeshConnectionsFlags;
    };

    class TerrainBuilder
    {
        public:
            TerrainBuilder(bool skipLiquid);
            ~TerrainBuilder();

            bool InitializeMap(G3D::uint32 mapID);
            bool loadMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MeshData &meshData);
            bool loadVMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MeshData &meshData);

            bool loadMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapLoadData &loadData);
            bool loadVMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapLoadData &loadData);
            void loadOffMeshConnections(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MeshData &meshData, const char* offMeshFilePath);
            void UnloadVMap(G3D::uint32 mapID);

            bool usesLiquids() { return !m_skipLiquid; }

            // vert and triangle methods
            static void transform(std::vector<G3D::Vector3> &original, std::vector<G3D::Vector3> &transformed,
                float scale, G3D::Matrix3 &rotation, G3D::Vector3 &position);
            static void copyVertices(std::vector<G3D::Vector3> &source, G3D::Array<float> &dest);
            static void copyIndices(std::vector<VMAP::MeshTriangle> &source, G3D::Array<int> &dest, int offest, bool flip);
            static void copyIndices(G3D::Array<int> &src, G3D::Array<int> &dest, int offset);
            static void cleanVertices(G3D::Array<float> &verts, G3D::Array<int> &tris);
        private:
            /// Loads a portion of a map's ADT info
            bool loadMapChunks(FILE *file, G3D::uint32 mapID, MapTileData &data);

            /// Loads a portion of a map's ADT info
            bool loadMapChunks(FILE *file, G3D::uint32 mapID, MapLoadData &loadData, Spot portion, G3D::uint32 fromTileX, G3D::uint32 fromTileY);

            /// Loads a portion of a map's terrain
            bool loadMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MeshData &meshData, Spot portion);

            /// Loads a portion of a map's terrain
            bool loadMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapLoadData &loadData, Spot portion, G3D::uint32 fromTileX, G3D::uint32 fromTileY);

            /// Sets loop variables for selecting only certain parts of a map's terrain
            void getLoopVars(Spot portion, int &loopStart, int &loopEnd, int &loopInc);

            /// Sets loop variables for selecting only certain parts of a map's terrain
            void getChLoopVars(Spot portion, int &xStart, int &xEnd, int &yStart, int &yEnd);

            /// Map offsets stored at the file header
            std::map<uint32, uint32[64][64]> m_mapOffsets;

            /// Map compression types
            std::map<uint32, uint32> m_mapCompressionType;

            /// Controls whether liquids are loaded
            bool m_skipLiquid;

            /// VMap Manager for loading main tile
            VMAP::VMapManager* vmapManager;

            /// Get the triangle's vector indices for a specific position
            void getHeightTriangle(int square, Spot triangle, int* indices, bool chunk = false);

            // hide parameterless and copy constructor
            TerrainBuilder();
            TerrainBuilder(const TerrainBuilder &tb);
    };
}

#endif

