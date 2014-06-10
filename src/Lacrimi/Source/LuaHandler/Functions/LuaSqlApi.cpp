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

//QueryResult methods
int LuaSql_GetColumn(lua_State * L, QueryResult * res) 
{
    if(res != NULL)
    {
        uint32 column = CHECK_ULONG(L,1);
        uint32 fields = res->GetFieldCount();
        if(column > fields)
            luaL_error(L,"GetColumn, Column %d bigger than max column %d",column,res->GetFieldCount() );
        else
        {
            Field * field = &(res->Fetch()[column]);
            if(field == NULL)
                lua_pushnil(L);
            else
                Lunar<Field>::push(L,field);
        }
    }
    else
        lua_pushnil(L);
    return 1;
}

int LuaSql_NextRow(lua_State * L, QueryResult * res)
{
    if(res != NULL)
    {
        if(res->NextRow() )
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);
    }
    else
        lua_pushboolean(L,0);
    return 1;
}

int LuaSql_GetColumnCount(lua_State * L, QueryResult * res)
{
    if(res == NULL)
        lua_pushnil(L);
    else
        lua_pushnumber(L,res->GetFieldCount());
    return 1;
}

int LuaSql_GetRowCount(lua_State * L, QueryResult * res)
{
    if(res == NULL)
        lua_pushnil(L);
    else
        lua_pushnumber(L,res->GetRowCount());
    return 1;
}

// Field api
int LuaSql_GetString(lua_State * L, Field * field)
{
    if(field == NULL)
        lua_pushnil(L);
    else
        lua_pushstring(L,field->GetString());
    return 1;
}

int LuaSql_GetFloat(lua_State * L, Field * field)
{
    if(field == NULL)
        lua_pushnil(L);
    else
        lua_pushnumber(L,field->GetFloat());
    return 1;
}

int LuaSql_GetBool(lua_State * L, Field * field)
{
    if(field == NULL)
        lua_pushnil(L);
    else
    {
        if(field->GetBool() )
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);
    }
    return 1;
}

int LuaSql_GetUByte(lua_State *L, Field * field)
{
    if(field == NULL)
        lua_pushnil(L);
    else
        lua_pushinteger(L,field->GetUInt8());
    return 1;
}

int LuaSql_GetByte(lua_State * L, Field * field)
{
    if(field == NULL)
        lua_pushnil(L);
    else
        lua_pushinteger(L,field->GetInt8());
    return 1;
}

int LuaSql_GetUShort(lua_State * L, Field * field)
{
    if(field == NULL)
        lua_pushnil(L);
    else
        lua_pushinteger(L,field->GetUInt16());
    return 1;
}

int LuaSql_GetShort(lua_State * L, Field * field)
{
    if(field == NULL)
        lua_pushnil(L);
    else
        lua_pushinteger(L,field->GetInt16());
    return 1;
}

int LuaSql_GetULong(lua_State * L, Field * field)
{
    if(field == NULL)
        lua_pushnil(L);
    else
        lua_pushnumber(L,field->GetUInt32());
    return 1;
}

int LuaSql_GetLong(lua_State * L, Field * field)
{
    if(field == NULL)
        lua_pushnil(L);
    else
        lua_pushnumber(L,field->GetInt32());
    return 1;
}

int LuaSql_GetGUID(lua_State * L, Field * field)
{
    if(field == NULL)
        lua_pushnil(L);
    else
    {
        uint64 guid = field->GetUInt64();
        GuidMgr::push(L,guid);
    }
    return 1;
}
