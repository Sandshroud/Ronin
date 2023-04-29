/*
 * Sandshroud Project Ronin
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
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

enum DalaranTeleportSpellIds
{
    DAL_TELEPORT_SPELL_CAVERNS_OF_TIME = 59901,
    DAL_TELEPORT_SPELL_PURPLE_PARLOR = 61419,
    DAL_TELEPORT_SPELL_VIOLET_CITADEL = 61420
};

enum DalaranTeleportLocations
{
    DAL_TELEPORT_LOC_CAVERNS_OF_TIME,
    DAL_TELEPORT_LOC_PURPLE_PARLOR,
    DAL_TELEPORT_LOC_VIOLET_CITADEL,
    DAL_TELEPORT_LOC_END
};

struct DalaranTeleportLoc
{
    uint32 mapId;
    float x, y, z, o;
};

