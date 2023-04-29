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

#include "StdAfx.h"
#include "4395_dalaran.h"

DalaranTeleportLoc teleportLocs[DAL_TELEPORT_LOC_END] = 
{
    // DAL_TELEPORT_LOC_CAVERNS_OF_TIME
    { 1, -8164.8f, -4768.5f, 34.3f, 0.f },
    // DAL_TELEPORT_LOC_PURPLE_PARLOR
    { 571, 5848.48f, 853.706f, 843.182f, 0.f },
    // DAL_TELEPORT_LOC_VIOLET_CITADEL
    { 571, 5819.26f, 829.774f, 680.22f, 0.f }
};

bool PortalEffectCavernsOfTime(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    Player *plrTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(plrTarget == NULL)
        return false;

    mapId = teleportLocs[DAL_TELEPORT_LOC_CAVERNS_OF_TIME].mapId;
    X = teleportLocs[DAL_TELEPORT_LOC_CAVERNS_OF_TIME].x;
    Y = teleportLocs[DAL_TELEPORT_LOC_CAVERNS_OF_TIME].y;
    Z = teleportLocs[DAL_TELEPORT_LOC_CAVERNS_OF_TIME].z;
    O = teleportLocs[DAL_TELEPORT_LOC_CAVERNS_OF_TIME].o;
    return true;
}

bool PortalEffectPurpleParlor(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    Player *plrTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(plrTarget == NULL)
        return false;

    mapId = teleportLocs[DAL_TELEPORT_LOC_PURPLE_PARLOR].mapId;
    X = teleportLocs[DAL_TELEPORT_LOC_PURPLE_PARLOR].x;
    Y = teleportLocs[DAL_TELEPORT_LOC_PURPLE_PARLOR].y;
    Z = teleportLocs[DAL_TELEPORT_LOC_PURPLE_PARLOR].z;
    O = teleportLocs[DAL_TELEPORT_LOC_PURPLE_PARLOR].o;
    return true;
}

bool PortalEffectVioletCitadel(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    Player *plrTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(plrTarget == NULL)
        return false;

    mapId = teleportLocs[DAL_TELEPORT_LOC_VIOLET_CITADEL].mapId;
    X = teleportLocs[DAL_TELEPORT_LOC_VIOLET_CITADEL].x;
    Y = teleportLocs[DAL_TELEPORT_LOC_VIOLET_CITADEL].y;
    Z = teleportLocs[DAL_TELEPORT_LOC_VIOLET_CITADEL].z;
    O = teleportLocs[DAL_TELEPORT_LOC_VIOLET_CITADEL].o;
    return true;
}

void SpellManager::_RegisterDalaranScripts()
{
    // Portal effects in Dalaran
    _RegisterTeleportData(DAL_TELEPORT_SPELL_CAVERNS_OF_TIME, SP_EFF_INDEX_0, &PortalEffectCavernsOfTime);
    _RegisterTeleportData(DAL_TELEPORT_SPELL_PURPLE_PARLOR, SP_EFF_INDEX_0, &PortalEffectPurpleParlor);
    _RegisterTeleportData(DAL_TELEPORT_SPELL_VIOLET_CITADEL, SP_EFF_INDEX_0, &PortalEffectVioletCitadel);

}
