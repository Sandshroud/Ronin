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

#define LIQUID_TILE_SIZE (533.333f / 128.f)

namespace VMAP
{
    const char VMAP_MAGIC[] = "VMAP_r050";
    const char RAW_VMAP_MAGIC[] = "VMAP050";
    const char GAMEOBJECT_MODELS[] = "GameObjectModels.dtree";

    // defined in TileAssembler.cpp currently...
    bool readChunk(FILE *rf, char *dest, const char *compare, uint32 len);

    enum VMAP_gWMOFlags
    {
        // WMO exists at this location
        WMO_FLAG_WMO_EXISTS             = 0x0001,
        // Inside, WMO minimap will be set
        WMO_FLAG_INSIDE_WMO_BOUNDS      = 0x0004,
        // Outside, WMO minimap is ignored
        WMO_FLAG_OUTSIDE_WMO_BOUNDS     = 0x0008,
        // Inside of a WMO flagged as a city
        WMO_FLAG_INSIDE_CITY_WMO        = 0x0040,
        // Inside of a WMO that is inside of another WMO
        WMO_FLAG_INSIDE_SLAVE_WMO       = 0x0100,
        // Inside of a WMO that has WMO inside of it
        WMO_FLAG_INSIDE_MASTER_WMO      = 0x0800,
        // Liquid exists for this WMO object
        WMO_FLAG_HAS_WMO_LIQUID         = 0x1000,
        // Seems constant when inside of WMO
        WMO_FLAG_UNK2000                = 0x2000,
        // No clue yet
        WMO_FLAG_WMO_NO_INSIDE          = 0x8000,
        // No clue yet
        WMO_FLAG_NOT_WHAT_I_WANT_1K     = 0x01000000,
        // Not sure yet
        WMO_FLAG_FILLS_MAP_HOLE         = 0x02000000,
        // Seems to be Worldspawn WMO, client loads on map load, always in render etc
        WMO_FLAG_HARDCODED_WORLDSPAWN   = 0x80000000,
    };
};
