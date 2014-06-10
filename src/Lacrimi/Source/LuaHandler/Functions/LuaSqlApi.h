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

#ifndef LUA_SQL_H
#define LUA_SQL_H

//QueryResult methods
int LuaSql_GetColumn(lua_State * L, QueryResult * res);
int LuaSql_NextRow(lua_State * L, QueryResult * res);
int LuaSql_GetColumnCount(lua_State * L, QueryResult * res);
int LuaSql_GetRowCount(lua_State * L, QueryResult * res);

// Field api
int LuaSql_GetString(lua_State * L, Field * field);
int LuaSql_GetFloat(lua_State * L, Field * field);
int LuaSql_GetBool(lua_State * L, Field * field);
int LuaSql_GetUByte(lua_State *L, Field * field);
int LuaSql_GetByte(lua_State * L, Field * field);
int LuaSql_GetUShort(lua_State * L, Field * field);
int LuaSql_GetShort(lua_State * L, Field * field);
int LuaSql_GetULong(lua_State * L, Field * field);
int LuaSql_GetLong(lua_State * L, Field * field);
int LuaSql_GetGUID(lua_State * L, Field * field);

#endif