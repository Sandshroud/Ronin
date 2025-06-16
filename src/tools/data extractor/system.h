/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2014-2017 Sandshroud <https://github.com/Sandshroud>
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

#pragma once

enum ModelFlags
{
    MOD_M2 = 1,
    MOD_WORLDSPAWN = 1<<1,
    MOD_HAS_BOUND = 1<<2
};

enum MapCompressionType : uint32
{
    MAP_RAW_TILES = 1,
    MAP_RAW_CHUNKS,
    MAP_COMPRESSED_TILES,
    MAP_COMPRESSED_CHUNKS
};

extern const char * szWorkDirWmo;
extern const char * szRawVMAPMagic;                         // vmap magic string for extracted raw vmap data

extern HANDLE WorldMpq;
extern HANDLE LocaleMpq;

class WDTFile;
class ADTFile;

struct MapCreationInfo
{
    uint32 mapId;
    WDTFile *WDT;

    char fn[512], id[10], fileName[100], dirName[255];

    ADTFile *chnkFiles[64][64];
    ADTFile *WDTFiles[64][64];

    VMAP::ModelSpawnMap modelSpawns;
    VMAP::TiledModelSpawnMap tileModelSpawnSets;
};

bool GetMPQHandle(const char* file, HANDLE &mpqhandle);
// Thread Unsafe
bool FileExists(const char * file);
// Thread safe
bool WMOExists(const char * file);
void strToLower(char* str);

int ExtractSingleWmo(HANDLE mpqArchive, std::string& fname);
int ExtractSingleModel(std::string& fname);

void ExtractGameobjectModels();
