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

void GuidMgr::Register(lua_State * L)
{
    luaL_newmetatable(L,GetName());
    int mt = lua_gettop(L);

    //Hide metatable.
    lua_pushnil(L);
    lua_setfield(L,mt,"__metatable");

    //nil gc method
    lua_pushnil(L);
    lua_setfield(L,mt,"__gc");

    //set our tostring method
    lua_pushcfunction(L,_tostring);
    lua_setfield(L,mt,"__tostring");

    //nil __index field
    lua_pushnil(L);
    lua_setfield(L,mt,"__index");

    //set __newindex method
    lua_pushcfunction(L,_newindex);
    lua_setfield(L,mt,"__newindex");

    //no call method
    lua_pushnil(L);
    lua_setfield(L,mt,"__call");

    //pop metatable
    lua_pop(L,1);
}

uint64 GuidMgr::check(lua_State * L, int narg) 
{
    uint64 GUID = 0;
    uint64 * ptrHold = (uint64*)lua_touserdata(L,narg);
    if(ptrHold != NULL)
        GUID = *ptrHold;
    return GUID;
}

int GuidMgr::push(lua_State *L, uint64 guid)
{
    int index = 0;
    if(guid == 0) 
    {
        lua_pushnil(L);
        index = lua_gettop(L);
    }
    else
    {
        luaL_getmetatable(L,GetName());
        if(lua_isnoneornil(L,-1) )
            luaL_error(L,"%s metatable not found!.",GetName());
        else 
        {
            int mt = lua_gettop(L);
            uint64* guidHold = (uint64*)lua_newuserdata(L,sizeof(uint64));
            int ud = lua_gettop(L);
            if(guidHold == NULL)
                luaL_error(L,"Lua tried to allocate size %d of memory and failed!",sizeof(uint64*));
            else
            {
                (*guidHold) = guid;
                lua_pushvalue(L,mt);
                lua_setmetatable(L,ud);
                lua_replace(L,mt);
                lua_settop(L,mt);
                index = mt;
            }
        }
    }
    return index;
}

//This method prints formats the GUID in hexform and pushes to the stack.
int GuidMgr::_tostring(lua_State * L) 
{
    uint64 GUID = GuidMgr::check(L,1);
    if(GUID == 0)
        lua_pushnil(L);
    else {
        char buff[32];
        sprintf(buff,"%X",GUID);
        lua_pushfstring(L,"%s",buff);
    }
    return 1;
}

int GuidMgr::_newindex(lua_State *L) 
{
    //Remove table, key, and value
    lua_remove(L,1);
    lua_remove(L,1);
    lua_remove(L,1);
    luaL_error(L,"OPERATION PROHIBITED!");
    return 0;
}
