/*
 * ArcScript Scripts for BurningEmu By Crow@Sandshroud(WorldX)
 * Copyright (C) 2008-2009 Burning Heavens Team
 * Copyright (C) 2007 Moon++ <http://www.moonplusplus.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LUA_SPELL_H
#define LUA_SPELL_H

#define GET_SPELLVAR_INT(proto,offset,subindex) *(int*)((char*)(proto) + (offset) + (subindex))
#define GET_SPELLVAR_CHAR(proto,offset,subindex) *(char**)((char*)(proto) + (offset) + (subindex))
#define GET_SPELLVAR_BOOL(proto,offset,subindex) *(bool*)((char*)(proto) + (offset) + (subindex))
#define GET_SPELLVAR_FLOAT(proto,offset,subindex) *(float*)((char*)(proto) + (offset) + (subindex))

struct LuaSpellEntry
{
    const char* name;
    uint32 typeId; //0: int, 1: char*, 2: bool, 3: float
    size_t offset;
};

LuaSpellEntry luaSpellVars[];

LuaSpellEntry GetLuaSpellEntryByName(const char* name);
int LuaSpell_GetCaster(lua_State * L, Spell * sp);
int LuaSpell_GetEntry(lua_State * L, Spell * sp);
int LuaSpell_IsDuelSpell(lua_State * L, Spell * sp);
int LuaSpell_GetSpellType(lua_State * L, Spell * sp);
int LuaSpell_GetSpellState(lua_State * L, Spell * sp);
int LuaSpell_Cancel(lua_State * L, Spell * sp);
int LuaSpell_Cast(lua_State * L, Spell * sp);
int LuaSpell_CanCast(lua_State * L, Spell * sp);
int LuaSpell_Finish(lua_State * L, Spell * sp);
int LuaSpell_GetTarget(lua_State * L, Spell * sp);
int LuaSpell_IsStealthSpell(lua_State * L, Spell * sp);
int LuaSpell_IsInvisibilitySpell(lua_State * L, Spell * sp);
int LuaSpell_GetPossibleEnemy(lua_State * L, Spell * sp);
int LuaSpell_GetPossibleFriend(lua_State * L, Spell * sp);
int LuaSpell_HasPower(lua_State * L, Spell * sp);
int LuaSpell_IsAspect(lua_State * L, Spell * sp);
int LuaSpell_IsSeal(lua_State * L, Spell * sp);
int LuaSpell_GetObjectType(lua_State * L, Spell * sp);
int LuaSpell_SetVar(lua_State * L, Spell * sp);
int LuaSpell_GetVar(lua_State * L, Spell * sp);
int LuaSpell_ResetVar(lua_State * L, Spell * sp);
int LuaSpell_ResetAllVars(lua_State * L, Spell * sp);
int LuaSpell_GetCastedItemId(lua_State * L, Spell * sp);

#endif