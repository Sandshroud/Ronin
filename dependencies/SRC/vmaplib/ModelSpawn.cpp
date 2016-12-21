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

#include <g3dlite\G3D.h>
#include "VMapLib.h"
#include "VMapDefinitions.h"

namespace VMAP
{
    bool ModelSpawn::readFromFile(FILE* rf, ModelSpawn &spawn)
    {
        G3D::uint32 check = fread(&spawn.mapId, sizeof(G3D::uint32), 1, rf);
        check += fread(&spawn.packedTile, sizeof(G3D::uint32), 1, rf);
        if (check < 2) // EoF?
        {
            OUT_DETAIL("Error reading ModelSpawn flags!");
            return false;
        }
        check += fread(&spawn.flags, sizeof(G3D::uint32), 1, rf);
        check += fread(&spawn.adtId, sizeof(G3D::uint16), 1, rf);
        check += fread(&spawn.ID, sizeof(G3D::uint32), 1, rf);
        check += fread(&spawn.iPos, sizeof(float), 3, rf);
        check += fread(&spawn.iRot, sizeof(float), 3, rf);
        check += fread(&spawn.iScale, sizeof(float), 1, rf);
        bool has_bound = (spawn.flags & MOD_HAS_BOUND);
        if (has_bound) // only WMOs have bound in MPQ, only available after computation
        {
            G3D::Vector3 bLow, bHigh;
            check += fread(&bLow, sizeof(float), 3, rf);
            check += fread(&bHigh, sizeof(float), 3, rf);
            spawn.iBound = G3D::AABox(bLow, bHigh);
        }

        G3D::uint32 nameLen;
        check += fread(&nameLen, sizeof(G3D::uint32), 1, rf);
        if (check != G3D::uint32(has_bound ? 19 : 13))
        {
            OUT_DETAIL("Error reading ModelSpawn data!");
            return false;
        }
        char nameBuff[500];
        if (nameLen > 500) // file names should never be that long, must be file error
        {
            OUT_DETAIL("Error reading ModelSpawn, file name too long!");
            return false;
        }
        check = fread(nameBuff, sizeof(char), nameLen, rf);
        if (check != nameLen)
        {
            OUT_DETAIL("Error reading ModelSpawn!");
            return false;
        }

        spawn.name = std::string(nameBuff, nameLen);
        return true;
    }

    bool ModelSpawn::writeToFile(FILE* wf, const ModelSpawn &spawn)
    {
        G3D::uint32 check=0;
        check += fwrite(&spawn.mapId, sizeof(G3D::uint32), 1, wf);
        check += fwrite(&spawn.packedTile, sizeof(G3D::uint32), 1, wf);
        check += fwrite(&spawn.flags, sizeof(G3D::uint32), 1, wf);
        check += fwrite(&spawn.adtId, sizeof(G3D::uint16), 1, wf);
        check += fwrite(&spawn.ID, sizeof(G3D::uint32), 1, wf);
        check += fwrite(&spawn.iPos, sizeof(float), 3, wf);
        check += fwrite(&spawn.iRot, sizeof(float), 3, wf);
        check += fwrite(&spawn.iScale, sizeof(float), 1, wf);
        bool has_bound = (spawn.flags & MOD_HAS_BOUND);
        if (has_bound) // only WMOs have bound in MPQ, only available after computation
        {
            check += fwrite(&spawn.iBound.low(), sizeof(float), 3, wf);
            check += fwrite(&spawn.iBound.high(), sizeof(float), 3, wf);
        }
        G3D::uint32 nameLen = spawn.name.length();
        check += fwrite(&nameLen, sizeof(G3D::uint32), 1, wf);
        if (check != G3D::uint32(has_bound ? 19 : 13)) return false;
        check = fwrite(spawn.name.c_str(), sizeof(char), nameLen, wf);
        if (check != nameLen) return false;
        return true;
    }
}
