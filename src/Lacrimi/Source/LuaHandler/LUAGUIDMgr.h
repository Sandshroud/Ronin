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

#ifndef __LUAGUID_MGR_H
#define __LUAGUID_MGR_H

class GuidMgr
{
private:
    static const char * GetName() { return "GuidMgr"; }

public:
    static void Register(lua_State * L);
    static uint64 check(lua_State * L, int narg);
    static int push(lua_State *L, uint64 guid);

    GuidMgr() {}

    //This method prints formats the GUID in hexform and pushes to the stack.
    static int _tostring(lua_State * L);
    static int _newindex(lua_State *L);
};

#endif // __LUAGUID_MGR_H
