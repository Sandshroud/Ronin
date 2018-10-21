/***
* Demonstrike Core
*/

#include "StdAfx.h"

bool TranslocateToTirisfalGlades(SpellEntry *sp, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    mapId = 0;
    X = 1806.014f;
    Y = 333.556f;
    Z = 70.398f;
    O = 4.705f;
    return true;
}

bool TranslocateToSilvermoonCity(SpellEntry *sp, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    mapId = 530;
    X = 10021.391f;
    Y = -7014.54f;
    Z = 49.718f;
    O = 4.031f;
    return true;
}

void SpellManager::_RegisterTranslocateData()
{
    _RegisterTeleportData(25649, SP_EFF_INDEX_0, &TranslocateToTirisfalGlades);
    _RegisterTeleportData(26566, SP_EFF_INDEX_0, &TranslocateToSilvermoonCity);
}
