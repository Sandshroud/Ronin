/*
 * Lacrimi Scripts Copyright 2010 - 2011
 *
 * ############################################################
 * # ##            #       ####### ####### ##    #    #    ## #
 * # ##           ###      ##      ##   ## ##   ###  ###   ## #
 * # ##          ## ##     ##      ##   ## ##   ###  ###   ## #
 * # ##         #######    ##      ####### ##  ## #### ##  ## #
 * # ##        ##     ##   ##      #####   ##  ## #### ##  ## #
 * # ##       ##       ##  ##      ##  ##  ## ##   ##   ## ## #
 * # ####### ##         ## ####### ##   ## ## ##   ##   ## ## #
 * # :::::::.::.........::.:::::::.::...::.::.::...::...::.:: #
 * ############################################################
 *
 */

#include "LacrimiStdAfx.h"

/*
    NORMAL OPERATIONS
    */
int LuaPacket_CreatePacket(lua_State * L, WorldPacket * packet)
{
    int opcode = luaL_checkint(L,1);
    int size = luaL_checkint(L,2);
    if(opcode >= NUM_MSG_TYPES)
    {
        luaL_error(L,"CreatePacket got opcode %d greater than max opcode %d.",opcode,NUM_MSG_TYPES);
        RET_NIL(true);
    }
    else
    {
        WorldPacket * npacket = new WorldPacket(opcode,size);
        Lunar<WorldPacket>::push(L,npacket);
    }
    return 1;
}

int LuaPacket_GetOpcode(lua_State *L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    lua_pushinteger(L,packet->GetOpcode());
    return 1;
}

int LuaPacket_GetSize(lua_State *L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    lua_pushinteger(L,packet->size());
    return 1;
}

/*
    READ OPERATIONS
    */
int LuaPacket_ReadUByte(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    uint8 byte;
    (*packet) >> byte;
    lua_pushinteger(L,byte);
    return 1;
}

int LuaPacket_ReadByte(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    int8 byte;
    (*packet) >> byte;
    lua_pushinteger(L,byte);
    return 1;
}

int LuaPacket_ReadShort(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    int16 val;
    (*packet) >> val;
    lua_pushinteger(L,val);
    return 1;
}

int LuaPacket_ReadUShort(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    uint16 val;
    (*packet) >> val;
    lua_pushinteger(L,val);
    return 1;
}

int LuaPacket_ReadLong(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    int32 val;
    (*packet) >> val;
    lua_pushinteger(L,val);
    return 1;
}

int LuaPacket_ReadULong(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    uint32 val;
    (*packet) >> val;
    lua_pushinteger(L,val);
    return 1;
}

int LuaPacket_ReadFloat(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    float val;
    (*packet) >> val;
    lua_pushnumber(L,val);
    return 1;
}

int LuaPacket_ReadDouble(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    double val;
    (*packet) >> val;
    lua_pushnumber(L,val);
    return 1;
}

int LuaPacket_ReadGUID(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    uint64 guid;
    (*packet) >> guid;
    GuidMgr::push(L,guid);
    return 1;
}

int LuaPacket_ReadWoWGuid(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    WoWGuid nGuid;
    (*packet) >> nGuid;
    GuidMgr::push(L,nGuid.GetOldGuid());
    return 1;
}

int LuaPacket_ReadString(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    string str;
    (*packet) >> str;
    lua_pushstring(L, str.c_str());
    return 1;
}

/*
    WRITE OPERATIONS
    */
int LuaPacket_WriteByte(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    int8 byte = (int8)luaL_checkint(L,1);
    (*packet) << byte;
    return 1;
}

int LuaPacket_WriteUByte(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    uint8 byte = (uint8)luaL_checkint(L,1);
    (*packet) << byte;
    return 1;
}

int LuaPacket_WriteShort(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    int16 val = (int16)luaL_checkint(L,1);
    (*packet) << val;
    return 1;
}

int LuaPacket_WriteUShort(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    uint16 val = (uint16)luaL_checkint(L,1);
    (*packet) << val;
    return 1;
}

int LuaPacket_WriteLong(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    int32 val = (int32)luaL_checkint(L,1);
    (*packet) << val;
    return 1;
}

int LuaPacket_WriteULong(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    uint32 val = (uint32)luaL_checkint(L,1);
    (*packet) << val;
    return 1;
}

int LuaPacket_WriteFloat(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    float val = (float)luaL_checknumber(L,1);
    (*packet) << val;
    return 1;
}

int LuaPacket_WriteDouble(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    double val = luaL_checknumber(L,1);
    (*packet) << val;
    return 1;
}

int LuaPacket_WriteGUID(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    uint64 guid = luaL_checkint(L,1);
    (*packet) << guid;
    return 1;
}

int LuaPacket_WriteWoWGuid(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    Object * target = CHECK_OBJECT(L,1);
    if(target == NULL)
        RET_NIL(false);

    (*packet) << target->GetNewGUID();
    return 1;
}

int LuaPacket_WriteString(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(false);

    string str = string(luaL_checkstring(L,1));
    (*packet) << str;
    return 1;
}

int LuaPacket_GetObjectType(lua_State * L, WorldPacket * packet)
{
    if(packet == NULL)
        RET_NIL(true);

    lua_pushstring(L, "Packet");
    return 1;
}
