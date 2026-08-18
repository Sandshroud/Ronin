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
        TOP     = 0,
        RIGHT   = 1,
        LEFT    = 2,
        BOTTOM  = 3,
        CENTER  = 4,
        ENTIRE
    };

    enum MapCompressionType : G3D::uint32
    {
        MAP_RAW_TILES = 1,
        MAP_RAW_CHUNKS,
        MAP_COMPRESSED_TILES,
        MAP_COMPRESSED_CHUNKS,
        MAP_COMPRESSED_MAX
    };

    struct ObjectLoadData
    {
        G3D::AABox boundBox;
        G3D::Array<int> objTris;
        G3D::Array<float> objVerts;
        G3D::Array<int> objFlags;
    };

    static const float GRID_SIZE = 533.33333f;
    static const int CHUNK_SIZE = 16;

    static const int V9_CH_SIZE = 9;
    static const int V9_CH_SIZE_SQ = V9_CH_SIZE*V9_CH_SIZE;
    static const int V8_CH_SIZE = 8;
    static const int V8_CH_SIZE_SQ = V8_CH_SIZE*V8_CH_SIZE;

    static const int V9_SIZE = 129;
    static const int V9_SIZE_SQ = V9_SIZE*V9_SIZE;
    static const int V8_SIZE = 128;
    static const int V8_SIZE_SQ = V8_SIZE*V8_SIZE;

    static const float GRID_CHUNK_SIZE = GRID_SIZE/CHUNK_SIZE;
    static const float GRID_PART_SIZE = GRID_SIZE/V8_SIZE;

    // see contrib/extractor/system.cpp, CONF_use_minHeight
    static const float INVALID_MAP_LIQ_HEIGHT = -2000.f;
    static const float INVALID_MAP_LIQ_HEIGHT_MAX = 5000.0f;

    struct MapTileLoadData
    {
        // This is our output data for finished triangulation and vertices
        ObjectLoadData terrainData[2];
        G3D::Array<ObjectLoadData*> vmapData;

        // Raw dumps, not ordered
        G3D::Array<int> allTris;
        G3D::Array<float> allVerts;

        // offmesh connection data
        G3D::Array<float> offMeshConnections;   // [p0y,p0z,p0x,p1y,p1z,p1x] - per connection
        G3D::Array<float> offMeshConnectionRads;
        G3D::Array<unsigned char> offMeshConnectionDirs;
        G3D::Array<unsigned char> offMeshConnectionsAreas;
        G3D::Array<unsigned short> offMeshConnectionsFlags;
    };

    struct MapChunkLoadData
    {
        struct ChunkInfo// Chunk data is 16 chunks per tile plus portions from surrounding tiles
        {
            uint16 liquidType;
            uint16 navLiquidType;
            bool isHole;

            bool useLiquid[V8_CH_SIZE*V8_CH_SIZE*4], useLiquidV8[V8_CH_SIZE][V8_CH_SIZE], useLiquidV9[V9_CH_SIZE][V9_CH_SIZE];
            float zT8[V8_CH_SIZE][V8_CH_SIZE], zT9[V9_CH_SIZE][V9_CH_SIZE];
            float zL8[V8_CH_SIZE][V8_CH_SIZE], zL9[V9_CH_SIZE][V9_CH_SIZE];
        } _chunkData[CHUNK_SIZE][CHUNK_SIZE];

        G3D::Array<float> solidVerts, waterVerts;
    };

    class TerrainBuilder
    {
        public:
            TerrainBuilder(bool skipLiquid);
            ~TerrainBuilder();

            bool InitializeMap(G3D::uint32 mapID);

            bool loadMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapTileLoadData &loadData);
            bool loadVMap(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapTileLoadData &loadData);
            void loadOffMeshConnections(G3D::uint32 mapID, G3D::uint32 tileX, G3D::uint32 tileY, MapTileLoadData &loadData, const char* offMeshFilePath);
            void UnloadVMap(G3D::uint32 mapID);

            // vert and triangle methods
            static void transform(std::vector<G3D::Vector3> &original, std::vector<G3D::Vector3> &transformed,
                float scale, G3D::Matrix3 &rotation, G3D::Vector3 &position);
            static void copyVertices(std::vector<G3D::Vector3> &source, G3D::Array<float> &dest);
            static void copyIndices(std::vector<VMAP::MeshTriangle> &source, G3D::Array<int> &dest, int offest, bool flip);
            static void copyIndices(G3D::Array<int> &src, G3D::Array<int> &dest, int offset);
            static void cleanVertices(G3D::Array<float> &verts, G3D::Array<int> &tris);

        private:
            /// Loads a tile's map data into vertices and triangles
            bool _loadAndProcessMapChunks(FILE *file, G3D::uint32 mapId, uint32 tileX, uint32 tileY, MapChunkLoadData *chunkData, std::map<uint32, uint32> &chunk_map);

            /// Loads a portion of a map's ADT info
            bool _loadMapChunks(FILE *file, G3D::uint32 mapID, uint32 tileX, uint32 tileY, MapChunkLoadData *chunkData, std::map<uint32, uint32> &chunk_map);

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
            void getHeightTriangle(int square, Spot triangle, int* tIndices);

            // hide parameterless and copy constructor
            TerrainBuilder();
            TerrainBuilder(const TerrainBuilder &tb);
    };
}

#endif

