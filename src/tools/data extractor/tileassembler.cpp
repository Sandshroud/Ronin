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

#include <string>
#include <iostream>

#include <g3dlite/G3D.h>
#include <vmaplib/VMapLib.h>

extern const char* szWorkDirWmo;
extern const char* szWorkDirTiles;
extern const char* szWorkDirObject;

//=======================================================

static VMAP::TileAssembler *ta = NULL;

void writeVMapData(FILE *mapFile, uint32 mapId, VMAP::ModelSpawnMap* spawnMap, VMAP::TiledModelSpawnMap* tileSpawnMap)
{
    if(ta == NULL)
        ta = new VMAP::TileAssembler(szWorkDirWmo, szWorkDirTiles, szWorkDirObject);

    if(!ta->convertWorld3(mapFile, mapId, spawnMap, tileSpawnMap))
        printf("Failed converting vmap data for %u\n", mapId);
}

void CleanupVMapData(bool &success)
{
    if(ta == NULL)
        return;

    if(success && !ta->convertWorldObjects())
        success = false;
    delete ta;
}