/*
 * Sun++ Scripts for Sandshroud MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2007-2008 Moon++ Team <http://www.moonplusplus.info/>
 * Copyright (C) 2008-2009 Sun++ Team <http://www.sunscripting.com/>
 * Copyright (C) 2009-2011 Sandshroud <http://www.sandshroud.org/>
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

#ifndef GAMEOBJECT_FUNCTIONS_H
#define GAMEOBJECT_FUNCTIONS_H

int LuaGameObject_GossipCreateMenu(lua_State * L, GameObject * ptr);
int LuaGameObject_GossipMenuAddItem(lua_State * L, GameObject * ptr);
int LuaGameObject_GossipSendMenu(lua_State * L, GameObject * ptr);
int LuaGameObject_GossipComplete(lua_State * L, GameObject * ptr);
int LuaGameObject_GossipSendPOI(lua_State * L, GameObject * ptr);
int LuaGameObject_ModUInt32Value(lua_State * L, GameObject * ptr);
int LuaGameObject_ModFloatValue(lua_State * L, GameObject * ptr);
int LuaGameObject_SetUInt32Value(lua_State * L, GameObject * ptr);
int LuaGameObject_SetUInt64Value(lua_State * L, GameObject * ptr);
int LuaGameObject_SetFloatValue(lua_State * L, GameObject * ptr);
int LuaGameObject_GetUInt32Value(lua_State * L, GameObject * ptr);
int LuaGameObject_GetUInt64Value(lua_State * L, GameObject * ptr);
int LuaGameObject_GetFloatValue(lua_State * L, GameObject * ptr);
int LuaGameObject_GetMapId(lua_State * L, GameObject * ptr);
int LuaGameObject_RemoveFromWorld(lua_State * L, GameObject * ptr);
int LuaGameObject_GetName(lua_State * L, GameObject * ptr);
int LuaGameObject_TeleportPlr(lua_State * L, GameObject * ptr);
int LuaGameObject_GetCreatureNearestCoords(lua_State * L, GameObject * ptr);
int LuaGameObject_GetAreaId(lua_State * L, GameObject * ptr);
int LuaGameObject_GetGameObjectNearestCoords(lua_State *L, GameObject * ptr);
int LuaGameObject_GetClosestPlayer(lua_State * L, GameObject * ptr);
int LuaGameObject_GetDistance(lua_State * L, GameObject * ptr);
int LuaGameObject_IsInWorld(lua_State * L, GameObject * ptr);
int LuaGameObject_GetZoneId(lua_State *L, GameObject * ptr);
int LuaGameObject_PlaySoundToSet(lua_State * L, GameObject * ptr);
int LuaGameObject_SpawnCreature(lua_State * L, GameObject * ptr);
int LuaGameObject_GetItemCount(lua_State * L, GameObject * ptr);
int LuaGameObject_SpawnGameObject(lua_State * L, GameObject * ptr);
int LuaGameObject_CalcDistance(lua_State * L, GameObject * ptr);
int LuaGameObject_GetSpawnX(lua_State * L, GameObject * ptr);
int LuaGameObject_GetSpawnY(lua_State * L, GameObject * ptr);
int LuaGameObject_GetSpawnZ(lua_State * L, GameObject * ptr);
int LuaGameObject_GetSpawnO(lua_State * L, GameObject * ptr);
int LuaGameObject_GetX(lua_State * L, GameObject * ptr);
int LuaGameObject_GetY(lua_State * L, GameObject * ptr);
int LuaGameObject_GetZ(lua_State * L, GameObject * ptr);
int LuaGameObject_GetO(lua_State * L, GameObject * ptr);
int LuaGameObject_GetInRangePlayersCount(lua_State * L, GameObject * ptr);
int LuaGameObject_GetEntry(lua_State * L, GameObject * ptr);
int LuaGameObject_SetOrientation(lua_State * L, GameObject * ptr);
int LuaGameObject_CalcRadAngle(lua_State * L, GameObject * ptr);
int LuaGameObject_GetInstanceID(lua_State * L, GameObject * ptr);
int LuaGameObject_GetInRangePlayers(lua_State * L, GameObject * ptr);
int LuaGameObject_GetInRangeGameObjects(lua_State * L, GameObject * ptr);
int LuaGameObject_IsInFront(lua_State * L, GameObject * ptr);
int LuaGameObject_IsInBack(lua_State * L, GameObject * ptr);
int LuaGameObject_CastSpell(lua_State * L, GameObject * ptr);
int LuaGameObject_FullCastSpell(lua_State * L, GameObject * ptr);
int LuaGameObject_CastSpellOnTarget(lua_State * L, GameObject * ptr);
int LuaGameObject_FullCastSpellOnTarget(lua_State * L, GameObject * ptr);
int LuaGameObject_GetGUID(lua_State * L, GameObject* ptr);
int LuaGameObject_IsActive(lua_State * L, GameObject* ptr);
int LuaGameObject_Activate(lua_State * L, GameObject* ptr);
int LuaGameObject_DespawnObject(lua_State * L, GameObject* ptr);
int LuaGameObject_GetLandHeight(lua_State * L, GameObject * ptr);
int LuaGameObject_SetZoneWeather(lua_State * L, GameObject * ptr);

#endif // GAMEOBJECT_FUNCTIONS_H
