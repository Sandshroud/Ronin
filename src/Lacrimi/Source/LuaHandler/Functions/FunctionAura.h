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

#ifndef LUA_AURA_H
#define LUA_AURA_H

int LuaAura_GetObjectType(lua_State * L, Aura * aura);
int LuaAura_GetSpellId(lua_State * L, Aura * aura);
int LuaAura_GetCaster(lua_State * L, Aura * aura);
int LuaAura_GetTarget(lua_State * L, Aura * aura);
int LuaAura_GetDuration(lua_State * L, Aura * aura);
int LuaAura_SetDuration(lua_State * L, Aura * aura);
int LuaAura_GetTimeLeft(lua_State * L, Aura * aura);
int LuaAura_SetNegative(lua_State * L, Aura * aura);
int LuaAura_SetPositive(lua_State * L, Aura * aura);
int LuaAura_Remove(lua_State * L, Aura * aura);
int LuaAura_SetVar(lua_State * L, Aura * aura);
int LuaAura_GetVar(lua_State * L, Aura * aura);
int LuaAura_GetAuraSlot(lua_State * L, Aura * aura);
int LuaAura_SetAuraSlot(lua_State * L, Aura * aura);

#endif