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

#ifndef ADT_H
#define ADT_H

#include "mpqfile.h"
#include "wmo.h"
#include "model.h"
#include <map>

#define TILESIZE (533.33333f)
#define CHUNKSIZE ((TILESIZE) / 16.0f)
#define UNITSIZE (CHUNKSIZE / 8.0f)

class Liquid;

struct vec { float x, y, z; };
struct triangle { vec corners[3]; };

typedef struct
{
    uint8 headerFlag[16][16];
    uint16 areaMask[16][16];

    float v8[128][128], v9[144][144];
    float waterV9[144][144];
}mTile;

struct MapChunkHeader
{
    uint32 flags;
    uint32 ix;
    uint32 iy;
    uint32 nLayers;
    uint32 nDoodadRefs;
    uint32 offsMCVT;        // height map
    uint32 offsMCNR;        // Normal vectors for each vertex
    uint32 offsMCLY;        // Texture layer definitions
    uint32 offsMCRF;        // A list of indices into the parent file's MDDF chunk
    uint32 offsMCAL;        // Alpha maps for additional texture layers
    uint32 sizeMCAL;
    uint32 offsMCSH;        // Shadow map for static shadows on the terrain
    uint32 sizeMCSH;
    uint32 areaid;
    uint32 nMapObjRefs;
    uint32 holes;
    uint16 s[2];
    uint32 data1;
    uint32 data2;
    uint32 data3;
    uint32 predTex;
    uint32 nEffectDoodad;
    uint32 offsMCSE;
    uint32 nSndEmitters;
    uint32 offsMCLQ;            // Liqid level (old)
    uint32 sizeMCLQ;            //
    float  zpos;
    float  xpos;
    float  ypos;
    uint32 offsMCCV;            // offsColorValues in WotLK
    uint32 props;
    uint32 effectId;
};

class ADTFile
{
private:
    MPQFile ADT;
    std::string Adtfilename;
    mTile m_chunkData;

public:
    ADTFile(HANDLE mpqarchive, char* filename);
    ~ADTFile();
    int nWMO, nMDX;
    std::map<uint32, std::string*> WMOInstanceNameMap, ModelInstanceNameMap;

    bool parseCHNK(uint32 map_num, uint32 tileX, uint32 tileY, FILE *output);
    void parseWMO(uint32 map_num, uint32 tileX, uint32 tileY);
};

char const* GetPlainName(char const* FileName);
char* GetPlainName(char* FileName);
char* GetExtension(char* FileName);
void FixNameCase(char* name, size_t len);
void FixNameSpaces(char* name, size_t len);
//void fixMapNamen(char *name, size_t len);

#endif
