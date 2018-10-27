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
#include "0085_tirisfal_glades.h"

bool UndercityTranslocateScript(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount)
{
    static SpellEntry *translocate = dbcSpell.LookupEntry(26566);
    if(translocate == NULL) // We need our translocate
        return false;

    if(!target->IsPlayer() || castPtr<Player>(target)->GetTeam() != TEAM_HORDE)
        return false;

    castPtr<Player>(target)->GetSpellInterface()->LaunchSpell(translocate);
    return true;
}

void SpellManager::_RegisterTirisfalGladesScripts()
{
    // Process our handling of translocation
    _RegisterScriptedEffect(35727, SP_EFF_INDEX_0, &UndercityTranslocateScript); // Trigger 35730

}
