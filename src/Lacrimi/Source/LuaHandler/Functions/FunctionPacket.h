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

#ifndef _LUAPACKET_H
#define _LUAPACKET_H

/*
    NORMAL OPERATIONS
    */
int LuaPacket_CreatePacket(lua_State * L, WorldPacket * packet);
int LuaPacket_GetOpcode(lua_State *L, WorldPacket * packet);
int LuaPacket_GetSize(lua_State *L, WorldPacket * packet);

/*
    READ OPERATIONS
    */
int LuaPacket_ReadUByte(lua_State * L, WorldPacket * packet);
int LuaPacket_ReadByte(lua_State * L, WorldPacket * packet);
int LuaPacket_ReadShort(lua_State * L, WorldPacket * packet);
int LuaPacket_ReadUShort(lua_State * L, WorldPacket * packet);
int LuaPacket_ReadLong(lua_State * L, WorldPacket * packet);
int LuaPacket_ReadULong(lua_State * L, WorldPacket * packet);
int LuaPacket_ReadFloat(lua_State * L, WorldPacket * packet);
int LuaPacket_ReadDouble(lua_State * L, WorldPacket * packet);
int LuaPacket_ReadGUID(lua_State * L, WorldPacket * packet);
int LuaPacket_ReadWoWGuid(lua_State * L, WorldPacket * packet);
int LuaPacket_ReadString(lua_State * L, WorldPacket * packet);

/*
    WRITE OPERATIONS
    */
int LuaPacket_WriteByte(lua_State * L, WorldPacket * packet);
int LuaPacket_WriteUByte(lua_State * L, WorldPacket * packet);
int LuaPacket_WriteShort(lua_State * L, WorldPacket * packet);
int LuaPacket_WriteUShort(lua_State * L, WorldPacket * packet);
int LuaPacket_WriteLong(lua_State * L, WorldPacket * packet);
int LuaPacket_WriteULong(lua_State * L, WorldPacket * packet);
int LuaPacket_WriteFloat(lua_State * L, WorldPacket * packet);
int LuaPacket_WriteDouble(lua_State * L, WorldPacket * packet);
int LuaPacket_WriteGUID(lua_State * L, WorldPacket * packet);
int LuaPacket_WriteWoWGuid(lua_State * L, WorldPacket * packet);
int LuaPacket_WriteString(lua_State * L, WorldPacket * packet);
int LuaPacket_GetObjectType(lua_State * L, WorldPacket * packet);

#endif


        
