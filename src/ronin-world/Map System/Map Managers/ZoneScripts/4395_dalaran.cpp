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
#include "3487_silvermoon_city.h"

bool PortalEffectCavernsOfTime(SpellEntry *sp, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    Player *plrTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(plrTarget == NULL)
        return false;

    mapId = 1;
    X = -8164.8f;
    Y = -4768.5f;
    Z = 34.3f;
    O = 0.0f;
    return true;
}

bool PortalEffectPurpleParlor(SpellEntry *sp, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    Player *plrTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(plrTarget == NULL)
        return false;

    mapId = 571;
    X = 5848.48f;
    Y = 853.706f;
    Z = 843.182f;
    O = 0.0f;
    return true;
}

bool PortalEffectVioletCitadel(SpellEntry *sp, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    Player *plrTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(plrTarget == NULL)
        return false;

    mapId = 571;
    X = 5819.26f;
    Y = 829.774f;
    Z = 680.22f;
    O = 0.0f;
    return true;
}

void SpellManager::_RegisterDalaranScripts()
{
    // Portal effects in Dalaran
    _RegisterTeleportData(59901, SP_EFF_INDEX_0, &PortalEffectCavernsOfTime);
    _RegisterTeleportData(61419, SP_EFF_INDEX_0, &PortalEffectPurpleParlor);
    _RegisterTeleportData(61420, SP_EFF_INDEX_0, &PortalEffectVioletCitadel);

}
