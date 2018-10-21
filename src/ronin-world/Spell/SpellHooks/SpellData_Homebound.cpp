/***
* Demonstrike Core
*/

#include "StdAfx.h"

bool ReturnToHomebind(SpellEntry *sp, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    Player *plrTarget = target->IsPlayer() ? castPtr<Player>(target) : NULL;
    if(plrTarget == NULL)
        return false;

    mapId = plrTarget->GetBindMapId();
    X = plrTarget->GetBindPositionX();
    Y = plrTarget->GetBindPositionY();
    Z = plrTarget->GetBindPositionZ();
    O = plrTarget->GetOrientation();
    return true;
}

void SpellManager::_RegisterHomeboundData()
{
    _RegisterTeleportData(556, SP_EFF_INDEX_0, &ReturnToHomebind);
    _RegisterTeleportData(8690, SP_EFF_INDEX_0, &ReturnToHomebind);
    _RegisterTeleportData(39937, SP_EFF_INDEX_0, &ReturnToHomebind);
}
