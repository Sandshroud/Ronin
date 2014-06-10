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

#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

int LuaGlobalFunctions_PerformIngameSpawn(lua_State * L);
int LuaGlobalFunctions_GetGameTime(lua_State * L);
int LuaGlobalFunctions_WorldDBQuery(lua_State * L);
int LuaGlobalFunctions_CharDBQuery(lua_State * L);
int LuaGlobalFunctions_WorldDBQueryTable(lua_State * L);
int LuaGlobalFunctions_CharDBQueryTable(lua_State * L);
int LuaGlobalFunctions_GetPlayer(lua_State * L);
int LuaGlobalFunctions_GetLUAEngine(lua_State * L);
int LuaGlobalFunctions_GetLuaEngineVersion(lua_State * L);
int LuaGlobalFunctions_SendWorldMessage(lua_State * L);
int LuaGlobalFunctions_ReloadTable(lua_State * L);
int LuaGlobalFunctions_ReloadLuaEngine(lua_State * L);
int LuaGlobalFunctions_GetPlayersInWorld(lua_State * L);
int LuaGlobalFunctions_Rehash(lua_State * L);
int LuaGlobalFunctions_GetSandshroudRevision(lua_State * L);
int LuaGlobalFunctions_GetSandshroudBuildHash(lua_State * L);

//////////////////////////////////////////////////////////////////////////
// WORLD PVP NOT SUPPORTED!
//////////////////////////////////////////////////////////////////////////
int LuaGlobalFunctions_SendPvPCaptureMessage(lua_State * L);
int LuaGlobalFunctions_GetPlayersInMap(lua_State * L);
int LuaGlobalFunctions_GetPlayersInZone(lua_State * L);
int LuaGlobalFunctions_SendMail(lua_State * L);
int LuaGlobalFunctions_GetTaxiPath(lua_State * L);
int LuaGlobalFunctions_SetDBCSpellVar(lua_State * L);
int LuaGlobalFunctions_GetDBCSpellVar(lua_State * L);
int LuaGlobalFunctions_bit_and(lua_State *L);
int LuaGlobalFunctions_bit_or(lua_State * L);
int LuaGlobalFunctions_bit_xor(lua_State * L);
int LuaGlobalFunctions_bit_not(lua_State * L);
int LuaGlobalFunctions_bit_shiftleft(lua_State * L);
int LuaGlobalFunctions_bit_shiftright(lua_State * L);
int LuaGlobalFunctions_RemoveTimedEvents(lua_State * L);
int LuaGlobalFunctions_RemoveTimedEventsWithName(lua_State * L);
int LuaGlobalFunctions_RemoveTimedEvent(lua_State * L);
int LuaGlobalFunctions_RemoveTimedEventsInTable(lua_State * L);
int LuaGlobalFunctions_HasTimedEvents(lua_State * L);
int LuaGlobalFunctions_HasTimedEvent(lua_State * L);
int LuaGlobalFunctions_HasTimedEventWithName(lua_State * L);
int LuaGlobalFunctions_HasTimedEventInTable(lua_State * L);
int LuaGlobalFunctions_GetPlatform(lua_State * L);
int LuaGlobalFunctions_NumberToGUID(lua_State * L);
int LuaGlobalFunctions_SendPacketToZone(lua_State * L);
int LuaGlobalFunctions_SendPacketToInstance(lua_State * L);
int LuaGlobalFunctions_SendPacketToWorld(lua_State * L);
int LuaGlobalFunctions_SendPacketToChannel(lua_State * L);
int LuaGlobalFunctions_ToLower(lua_State * L);

int LuaGlobalFunctions_SetSanctuaryArea(lua_State * L);
int LuaGlobalFunctions_RemoveSanctuaryArea(lua_State * L);
int LuaGlobalFunctions_IsSanctuaryArea(lua_State * L);

#endif
