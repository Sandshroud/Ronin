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

#ifndef _LUATAXI_H
#define _LUATAXI_H

int LuaTaxi_CreateTaxi(lua_State * L, TaxiPath * tp);
int LuaTaxi_GetNodeCount(lua_State * L, TaxiPath * tp);
int LuaTaxi_AddPathNode(lua_State * L, TaxiPath * tp);
int LuaTaxi_GetId(lua_State * L, TaxiPath * tp);
int LuaTaxi_GetObjectType(lua_State * L, TaxiPath * tp);

#endif