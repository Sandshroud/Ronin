/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "PathCommon.h"
#include "MapBuilder.h"

#include "TerrainBuilder.h"

#include "VMapManager.h"
#include "MapTree.h"
#include "ModelInstance.h"
#include <vector>

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08
#define MAP_LIQUID_TYPE_DARK_WATER  0x10
#define MAP_LIQUID_TYPE_WMO_WATER   0x20

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
    void TerrainBuilder::loadMap(G3D::g3d_uint32 mapID, G3D::g3d_uint32 tileX, G3D::g3d_uint32 tileY, MeshData &meshData)
    {
        if (loadMap(mapID, tileX, tileY, meshData, ENTIRE))
        {
            loadMap(mapID, tileX+1, tileY, meshData, LEFT);
            loadMap(mapID, tileX-1, tileY, meshData, RIGHT);
            loadMap(mapID, tileX, tileY+1, meshData, TOP);
            loadMap(mapID, tileX, tileY-1, meshData, BOTTOM);
        }
    }

    struct MapData
    {
        G3D::g3d_uint16 AreaInfo[256];
        G3D::g3d_uint16 LiquidInfo[256];
        float V8[V8_SIZE_SQ];
        float V9[V9_SIZE_SQ];
        float liquid_height[V9_SIZE_SQ];
    };

    /**************************************************************************/
    bool TerrainBuilder::loadMap(G3D::g3d_uint32 mapID, G3D::g3d_uint32 tileX, G3D::g3d_uint32 tileY, MeshData &meshData, Spot portion)
    {
        char FileName[255];
        sprintf(FileName, "maps/Map_%03u.bin", mapID);

        FILE* mapFile = NULL;
        fopen_s(&mapFile, FileName, "rb");
        if(mapFile == NULL)
            return false;

        fseek(mapFile, 0, SEEK_SET);
        G3D::g3d_uint32 offsets[64][64];
        if(fread(offsets, 1, 16384, mapFile) != 16384)
        {
            fclose(mapFile);
            return false;
        }
        if(offsets[tileX][tileY] == 0)
        {
            fclose(mapFile);
            return false;
        }

        // Go to our offset, skip the area info.
        if(fseek(mapFile, offsets[tileX][tileY], SEEK_SET))
        {
            fclose(mapFile);
            return false;
        }

        G3D::Array<int> ltriangles;
        G3D::Array<int> ttriangles;
        MapData *mapInfo = new MapData();

        // Read from our file into this newly created struct.
        fread(&mapInfo->AreaInfo, sizeof(G3D::g3d_uint16), 256, mapFile);
        fread(&mapInfo->LiquidInfo, sizeof(G3D::g3d_uint16), 256, mapFile);
        fread(&mapInfo->V8, sizeof(float), 16384, mapFile);
        fread(&mapInfo->V9, sizeof(float), 16641, mapFile);
        fread(&mapInfo->liquid_height, sizeof(float), 16641, mapFile);
        fclose(mapFile);
        // Parse values for skippage
        bool allOcean = true;
        for(int i = 0; i < 256; i++)
            if(!(mapInfo->LiquidInfo[i]&0x20))
                allOcean = false;
        // We can skip tiles that are 100% ocean
        if(allOcean)
        {
            delete mapInfo;
            return false;
        }

        float coord[3];
        float xoffset = (float(tileX)-32)*GRID_SIZE;
        float yoffset = (float(tileY)-32)*GRID_SIZE;
        {
            int count = meshData.solidVerts.size() / 3;
            for (int i = 0; i < V9_SIZE_SQ; ++i)
            {
                getHeightCoord(i, GRID_V9, xoffset, yoffset, coord, mapInfo->V9);
                meshData.solidVerts.append(coord[0]);
                meshData.solidVerts.append(coord[2]);
                meshData.solidVerts.append(coord[1]);
            }

            for (int i = 0; i < V8_SIZE_SQ; ++i)
            {
                getHeightCoord(i, GRID_V8, xoffset, yoffset, coord, mapInfo->V8);
                meshData.solidVerts.append(coord[0]);
                meshData.solidVerts.append(coord[2]);
                meshData.solidVerts.append(coord[1]);
            }

            int indices[] = { 0, 0, 0 }, loopStart = 0, loopEnd = 0, loopInc = 0;
            getLoopVars(portion, loopStart, loopEnd, loopInc);
            for (int i = loopStart; i < loopEnd; i+=loopInc)
            {
                for (int j = TOP; j <= BOTTOM; j+=1)
                {
                    getHeightTriangle(i, Spot(j), indices);
                    ttriangles.append(indices[2] + count);
                    ttriangles.append(indices[1] + count);
                    ttriangles.append(indices[0] + count);
                }
            }
        }

        bool UsableWaterData = false;
        for (int i = 0; i < V9_SIZE_SQ; ++i)
        {
            coord[0] = (xoffset + i%(V9_SIZE)*GRID_PART_SIZE) * -1.f;
            coord[1] = (yoffset + (int)(i/(V9_SIZE))*GRID_PART_SIZE) * -1.f;
            G3D::g3d_uint16 liquidType = getLiquidType(coord[0], coord[1], mapInfo->LiquidInfo);
            if(liquidType == 0)
                continue;
            else
            {
                getLiquidCoord(coord[0], coord[1], coord[2], mapInfo->liquid_height);
                if(coord[2] == 0.0f && !(liquidType & 0x02))
                    continue;
            }

            UsableWaterData = true;
            break;
        }

        if(UsableWaterData)
        {
            int count = meshData.liquidVerts.size() / 3;
            for (int i = 0, j = 0; i < V9_SIZE_SQ; ++i)
            {
                coord[0] = (xoffset + i%(V9_SIZE)*GRID_PART_SIZE) * -1.f;
                coord[1] = (yoffset + (int)(i/(V9_SIZE))*GRID_PART_SIZE) * -1.f;
                G3D::g3d_uint16 liquidType = getLiquidType(coord[0], coord[1], mapInfo->LiquidInfo);
                if(liquidType == 0)
                    coord[2] = INVALID_MAP_LIQ_HEIGHT;
                else
                {
                    getLiquidCoord(coord[0], coord[1], coord[2], mapInfo->liquid_height);
                    if(coord[2] == 0.0f && !(liquidType & 0x02))
                        coord[2] = INVALID_MAP_LIQ_HEIGHT;
                }

                meshData.liquidVerts.append(coord[0]);
                meshData.liquidVerts.append(coord[2]);
                meshData.liquidVerts.append(coord[1]);
            }

            int indices[] = { 0, 0, 0 }, loopStart = 0, loopEnd = 0, loopInc = 0, triInc = BOTTOM-TOP;
            getLoopVars(portion, loopStart, loopEnd, loopInc);

            // generate triangles
            for (int i = loopStart; i < loopEnd; i+=loopInc)
            {
                for (int j = TOP; j <= BOTTOM; j+= triInc)
                {
                    getHeightTriangle(i, Spot(j), indices, true);
                    ltriangles.append(indices[2] + count);
                    ltriangles.append(indices[1] + count);
                    ltriangles.append(indices[0] + count);
                }
            }
        }

        // now that we have gathered the data, we can figure out which parts to keep:
        // liquid above ground, ground above liquid
        int loopStart = 0, loopEnd = 0, loopInc = 0, tTriCount = 4;
        bool useTerrain, useLiquid;

        float* lverts = meshData.liquidVerts.getCArray();
        int* ltris = ltriangles.getCArray();

        float* tverts = meshData.solidVerts.getCArray();
        int* ttris = ttriangles.getCArray();

        if ((ltriangles.size() + ttriangles.size()) == 0)
        {
            delete mapInfo;
            return false;
        }

        // make a copy of liquid vertices
        // used to pad right-bottom frame due to lost vertex data at extraction
        float* lverts_copy = NULL;
        if (meshData.liquidVerts.size())
        {
            lverts_copy = new float[meshData.liquidVerts.size()];
            memcpy(lverts_copy, lverts, sizeof(float)*meshData.liquidVerts.size());
        }

        getLoopVars(portion, loopStart, loopEnd, loopInc);
        for (int i = loopStart; i < loopEnd; i+=loopInc)
        {
            for (int j = 0; j < 2; ++j)
            {
                // default is true, will change to false if needed
                useTerrain = true;
                useLiquid = true;
                G3D::g3d_uint16 liquidType = MAP_LIQUID_TYPE_NO_WATER;
                // FIXME: "warning: the address of ‘liquid_type’ will always evaluate as ‘true’"

                // if there is no liquid, don't use liquid
                if (!meshData.liquidVerts.size() || !ltriangles.size())
                    useLiquid = false;
                else
                {
                    liquidType = getLiquidType(i, mapInfo->LiquidInfo);
                    if(liquidType & MAP_LIQUID_TYPE_DARK_WATER)
                    {
                        useTerrain = false;
                        useLiquid = false;
                    }
                    else if(liquidType & MAP_LIQUID_TYPE_SLIME)
                        liquidType = NAV_SLIME;
                    else if(liquidType & MAP_LIQUID_TYPE_MAGMA)
                        liquidType = NAV_MAGMA;
                    else if(liquidType & MAP_LIQUID_TYPE_OCEAN|MAP_LIQUID_TYPE_WATER)
                        liquidType = NAV_WATER;
                    else
                        useLiquid = false;
                }

                // if there is no terrain, don't use terrain
                if (!ttriangles.size())
                    useTerrain = false;

                // while extracting ADT data we are losing right-bottom vertices
                // this code adds fair approximation of lost data
                if (useLiquid)
                {
                    float quadHeight = 0;
                    G3D::g3d_uint32 validCount = 0;
                    for(G3D::g3d_uint32 idx = 0; idx < 3; idx++)
                    {
                        float h = lverts_copy[ltris[idx]*3 + 1];
                        if (h != INVALID_MAP_LIQ_HEIGHT && h < INVALID_MAP_LIQ_HEIGHT_MAX)
                        {
                            quadHeight += h;
                            validCount++;
                        }
                    }

                    // update vertex height data
                    if (validCount > 0 && validCount < 3)
                    {
                        quadHeight /= validCount;
                        for(G3D::g3d_uint32 idx = 0; idx < 3; idx++)
                        {
                            float h = lverts[ltris[idx]*3 + 1];
                            if (h == INVALID_MAP_LIQ_HEIGHT || h > INVALID_MAP_LIQ_HEIGHT_MAX)
                                lverts[ltris[idx]*3 + 1] = quadHeight;
                        }
                    }

                    // no valid vertexes - don't use this poly at all
                    if (validCount == 0)
                        useLiquid = false;
                }

                // we use only one terrain kind per quad - pick higher one
                if (useTerrain && useLiquid)
                {
                    float minLLevel = INVALID_MAP_LIQ_HEIGHT_MAX;
                    float maxLLevel = INVALID_MAP_LIQ_HEIGHT;
                    for(G3D::g3d_uint32 x = 0; x < 3; x++)
                    {
                        float h = lverts[ltris[x]*3 + 1];
                        if (minLLevel > h)
                            minLLevel = h;

                        if (maxLLevel < h)
                            maxLLevel = h;
                    }

                    float maxTLevel = INVALID_MAP_LIQ_HEIGHT;
                    float minTLevel = INVALID_MAP_LIQ_HEIGHT_MAX;
                    for(G3D::g3d_uint32 x = 0; x < 6; x++)
                    {
                        float h = tverts[ttris[x]*3 + 1];
                        if (maxTLevel < h)
                            maxTLevel = h;

                        if (minTLevel > h)
                            minTLevel = h;
                    }

                    // terrain under the liquid?
                    if (minLLevel > maxTLevel)
                        useTerrain = false;

                    //liquid under the terrain?
                    if (minTLevel > maxLLevel)
                        useLiquid = false;
                }

                // store the result
                if (useLiquid)
                {
                    meshData.liquidType.append(liquidType);
                    for (int k = 0; k < 3; ++k)
                        meshData.liquidTris.append(ltris[k]);
                }

                if (useTerrain)
                    for (int k = 0; k < 3*tTriCount/2; ++k)
                        meshData.solidTris.append(ttris[k]);

                // advance to next set of triangles
                ltris += 3;
                ttris += 3*tTriCount/2;
            }
        }

        if (lverts_copy)
            delete [] lverts_copy;

        delete mapInfo;
        return meshData.solidTris.size() || meshData.liquidTris.size();
    }

    /**************************************************************************/
    void TerrainBuilder::getHeightCoord(int index, Grid grid, float xOffset, float yOffset, float* coord, float* v)
    {
        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        switch (grid)
        {
        case GRID_V9:
            coord[0] = (xOffset + index%(V9_SIZE)*GRID_PART_SIZE) * -1.f;
            coord[1] = (yOffset + (int)(index/(V9_SIZE))*GRID_PART_SIZE) * -1.f;
            coord[2] = v[index];
            break;
        case GRID_V8:
            coord[0] = (xOffset + index%(V8_SIZE)*GRID_PART_SIZE + GRID_PART_SIZE/2.f) * -1.f;
            coord[1] = (yOffset + (int)(index/(V8_SIZE))*GRID_PART_SIZE + GRID_PART_SIZE/2.f) * -1.f;
            coord[2] = v[index];
            break;
        }
    }

    /**************************************************************************/
    void TerrainBuilder::getHeightTriangle(int square, Spot triangle, int* indices, bool liquid/* = false*/)
    {
        int rowOffset = square/V8_SIZE;
        if (!liquid)
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
        else
            switch (triangle)
        {                                                           //           0-----1 .... 128
            case TOP:                                               //           |\    |
                indices[0] = square+rowOffset;                      //           | \ T |
                indices[1] = square+1+rowOffset;                    //           |  \  |
                indices[2] = square+V9_SIZE+1+rowOffset;            //           | B \ |
                break;                                              //           |    \|
            case BOTTOM:                                            //          129---130 ... 386
                indices[0] = square+rowOffset;                      //           |\    |
                indices[1] = square+V9_SIZE+1+rowOffset;            //           | \   |
                indices[2] = square+V9_SIZE+rowOffset;              //           |  \  |
                break;                                              //           |   \ |
            default: break;                                         //           |    \|
        }                                                           //          258---259 ... 515
    }

    /**************************************************************************/
    void TerrainBuilder::getLiquidCoord(int index, int index2, float xOffset, float yOffset, float* coord, float* v)
    {
        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        coord[0] = (xOffset + index%(V9_SIZE)*GRID_PART_SIZE) * -1.f;
        coord[1] = (yOffset + (int)(index/(V9_SIZE))*GRID_PART_SIZE) * -1.f;
        coord[2] = v[index2];
    }

    /**************************************************************************/
    void TerrainBuilder::getLiquidCoord(float x, float y, float &coordz, float* v)
    {
        float vx = 128 * (32 - x / 533.3333333f);
        float vy = 128 * (32 - y / 533.3333333f);
        int cx_int = ((int)vx & (128 - 1));
        int cy_int = ((int)vy & (128 - 1));
        coordz = v[cx_int*129+cy_int];
    }

    /**************************************************************************/
    G3D::g3d_uint16 TerrainBuilder::getLiquidType(float x, float y, const G3D::g3d_uint16 liquid_type[256])
    {
        x = 16 * (32 - x / 533.3333333f);
        y = 16 * (32 - y / 533.3333333f);
        int lx = (int)x & 15;
        int ly = (int)y & 15;
        return liquid_type[lx * 16 + ly];
    }

    /**************************************************************************/
    G3D::g3d_uint16 TerrainBuilder::getLiquidType(int square, const G3D::g3d_uint16 liquid_type[256])
    {
        int row = square / 128;
        int col = square % 128;
        int cellRow = row / 8;     // 8 squares per cell
        int cellCol = col / 8;

        int index = cellRow*8+cellCol;
        return liquid_type[index];
    }

    // DBC ids are stored in vmaps currently, so convert to terrain water flags
    // We could use DBC file, but this workaround is sufficient.
    G3D::g3d_uint16 convertWaterIDToFlags(G3D::g3d_uint16 wmoType)
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
    bool TerrainBuilder::InitializeVMap(G3D::g3d_uint32 mapID)
    {
        vmapManager = new VMapManager("vmaps");
        if(!vmapManager->loadMap(mapID))
            return false;
        return true;
    }

    /**************************************************************************/
    void TerrainBuilder::UnloadVMap(G3D::g3d_uint32 mapID)
    {
        if(vmapManager)
        {
            vmapManager->unloadMap(mapID);
            delete vmapManager;
            vmapManager = NULL;
        }
    }

    /**************************************************************************/
    bool TerrainBuilder::loadVMap(G3D::g3d_uint32 mapID, G3D::g3d_uint32 tileX, G3D::g3d_uint32 tileY, MeshData &meshData)
    {
        if(vmapManager == NULL)
            return false;

        bool result = vmapManager->loadMap(mapID, tileX, tileY), retval = false;
        do
        {
            if (result == false)
                break;

            InstanceTreeMap instanceTrees;
            vmapManager->getInstanceMapTree(instanceTrees);
            if (!instanceTrees[mapID])
                break;

            ModelInstance* models = NULL;
            G3D::g3d_uint32 count = 0;
            instanceTrees[mapID]->getModelInstances(models, count);
            if (!models)
                break;

            for (G3D::g3d_uint32 i = 0; i < count; ++i)
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
                        G3D::g3d_uint32 tilesX, tilesY, vertsX, vertsY;
                        G3D::Vector3 corner;
                        liquid->getPosInfo(tilesX, tilesY, corner);
                        vertsX = tilesX + 1;
                        vertsY = tilesY + 1;
                        G3D::g3d_uint8* flags = liquid->GetFlagsStorage();
                        float* data = liquid->GetHeightStorage();
                        G3D::g3d_uint8 type = NAV_EMPTY;

                        // convert liquid type to NavTerrain
                        switch (convertWaterIDToFlags(liquid->GetType()))
                        {
                        case 0x1:
                        case 0x2:
                            type = NAV_WATER;
                            break;
                        case 0x04:
                            type = NAV_MAGMA;
                            break;
                        case 0x08:
                            type = NAV_SLIME;
                            break;
                        }

                        // indexing is weird...
                        // after a lot of trial and error, this is what works:
                        // vertex = y*vertsX+x
                        // tile   = x*tilesY+y
                        // flag   = y*tilesY+x

                        G3D::Vector3 vert;
                        for (G3D::g3d_uint32 x = 0; x < vertsX; ++x)
                        {
                            for (G3D::g3d_uint32 y = 0; y < vertsY; ++y)
                            {
                                vert = G3D::Vector3(corner.x + x * GRID_PART_SIZE, corner.y + y * GRID_PART_SIZE, data[y*vertsX + x]);
                                vert = vert * rotation * scale + position;
                                vert.x *= -1.f;
                                vert.y *= -1.f;
                                liqVerts.push_back(vert);
                            }
                        }

                        int idx1, idx2, idx3, idx4;
                        G3D::g3d_uint32 square;
                        for (G3D::g3d_uint32 x = 0; x < tilesX; ++x)
                        {
                            for (G3D::g3d_uint32 y = 0; y < tilesY; ++y)
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

                        G3D::g3d_uint32 liqOffset = meshData.liquidVerts.size() / 3;
                        for (G3D::g3d_uint32 i = 0; i < liqVerts.size(); ++i)
                            meshData.liquidVerts.append(liqVerts[i].y, liqVerts[i].z, liqVerts[i].x);

                        for (G3D::g3d_uint32 i = 0; i < liqTris.size() / 3; ++i)
                        {
                            meshData.liquidTris.append(liqTris[i*3+1] + liqOffset, liqTris[i*3+2] + liqOffset, liqTris[i*3] + liqOffset);
                            meshData.liquidType.append(type);
                        }
                    }
                }
            }
        }
        while (false);

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
            dest.push_back((*it).y);
            dest.push_back((*it).z);
            dest.push_back((*it).x);
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
        for (G3D::g3d_int32 i = 0; i < source.size(); ++i)
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
    void TerrainBuilder::loadOffMeshConnections(G3D::g3d_uint32 mapID, G3D::g3d_uint32 tileX, G3D::g3d_uint32 tileY, MeshData &meshData, const char* offMeshFilePath)
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
            G3D::g3d_uint32 mid, tx, ty;
            float size;
            if (sscanf(buf, "%d %d,%d (%f %f %f) (%f %f %f) %f", &mid, &tx, &ty,
                &p0[0], &p0[1], &p0[2], &p1[0], &p1[1], &p1[2], &size) != 10)
                continue;

            if (mapID == mid && tileX == tx && tileY == ty)
            {
                meshData.offMeshConnections.append(p0[1]);
                meshData.offMeshConnections.append(p0[2]);
                meshData.offMeshConnections.append(p0[0]);

                meshData.offMeshConnections.append(p1[1]);
                meshData.offMeshConnections.append(p1[2]);
                meshData.offMeshConnections.append(p1[0]);

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
