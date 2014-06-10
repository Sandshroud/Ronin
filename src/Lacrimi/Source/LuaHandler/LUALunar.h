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

#ifndef __LUALUNAR_H
#define __LUALUNAR_H

template <typename T> class Lunar
{
    typedef struct { T *pT; } userdataType;
public:
    typedef int (*mfp)(lua_State *L, T* ptr);
    typedef struct
    {
        const char *name;
        mfp mfunc;
    }RegType;

    static void Register(lua_State *L)
    {
        lua_newtable(L);
        int methods = lua_gettop(L);

        luaL_newmetatable(L, GetTClassName<T>());
        int metatable = lua_gettop(L);

        luaL_newmetatable(L,"DO NOT TRASH");
        lua_pop(L,1);

        // store method table in globals so that
        // scripts can add functions written in Lua.
        lua_pushvalue(L, methods);
        lua_setfield(L, LUA_GLOBALSINDEX, GetTClassName<T>());

        // hide metatable from Lua getmetatable()
        lua_pushvalue(L, methods);
        lua_setfield(L, metatable, "__metatable");

        lua_pushvalue(L, methods);
        lua_setfield(L, metatable, "__index");

        lua_pushcfunction(L, tostring_T);
        lua_setfield(L, metatable, "__tostring");

        lua_pushcfunction(L, gc_T);
        lua_setfield(L, metatable, "__gc");

        lua_newtable(L);                // mt for method table
        lua_setmetatable(L, methods);

        // fill method table with methods from class T
        for (RegType *l = ((RegType*)GetMethodTable<T>()); l->name; l++)
        {
            lua_pushstring(L, l->name);
            lua_pushlightuserdata(L, (void*)l);
            lua_pushcclosure(L, thunk, 1);
            lua_settable(L, methods);
        }

        lua_pop(L, 2);  // drop metatable and method table
    }

    // push onto the Lua stack a userdata containing a pointer to T object
    static int push(lua_State *L, T *obj, bool gc=false)
    {
        if (obj == NULL)
        {
            lua_pushnil(L);
            return lua_gettop(L);
        }

        luaL_getmetatable(L, GetTClassName<T>());   // lookup metatable in Lua registry
        if (lua_isnil(L, -1))
            luaL_error(L, "%s missing metatable", GetTClassName<T>());

        int mt = lua_gettop(L);
        T ** ptrHold = (T**)lua_newuserdata(L,sizeof(T**));
        int ud = lua_gettop(L);
        if(ptrHold != NULL)
        {
            *ptrHold = obj;
            lua_pushvalue(L, mt);
            lua_setmetatable(L, -2);
            char name[32];
            tostring(name,obj);
            lua_getfield(L,LUA_REGISTRYINDEX,"DO NOT TRASH");
            if(lua_isnil(L,-1))
            {
                luaL_newmetatable(L,"DO NOT TRASH");
                lua_pop(L,1);
            }
            lua_getfield(L,LUA_REGISTRYINDEX,"DO NOT TRASH");
            if(gc == false)
            {
                lua_pushboolean(L,1);
                lua_setfield(L,-2,name);
            }
            lua_pop(L,1);
        }
        lua_settop(L,ud);
        lua_replace(L, mt);
        lua_settop(L, mt);
        return mt;  // index of userdata containing pointer to T object
    }

    // get userdata from Lua stack and return pointer to T object
    static T *check(lua_State *L, int narg)
    {
        T ** ptrHold = static_cast<T**>(lua_touserdata(L,narg));
        if(ptrHold == NULL)
            return NULL;
        return *ptrHold;
    }

private:
    Lunar();    // hide default constructor

    // member function dispatcher
    static int thunk(lua_State *L)
    {
        // stack has userdata, followed by method args
        T *obj = check(L, 1);           // get 'self', or if you prefer, 'this'
        lua_remove(L, 1);               // remove self so member function args start at index 1

        // get member function from upvalue
        RegType *l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
        return l->mfunc(L,obj);
    }

    // create a new T object and
    // push onto the Lua stack a userdata containing a pointer to T object
    static int new_T(lua_State *L)
    {
        lua_remove(L, 1);           // use classname:new(), instead of classname.new()
        T *obj = NULL;              // call constructor for T objects
        assert(false);
        push(L, obj, true);         // gc_T will delete this object
        return 1;                   // userdata containing pointer to T object
    }

    // garbage collection metamethod
    static int gc_T(lua_State *L)
    {
        T * obj = check(L,1);
        if(obj == NULL)
            return 0;
        lua_getfield(L,LUA_REGISTRYINDEX,"DO NOT TRASH");
        if(lua_istable(L,-1) )
        {
            char name[32];
            tostring(name,obj);
            lua_getfield(L,-1,string(name).c_str());
            if(lua_isnil(L,-1) )
            {
                delete obj;
                obj = NULL;
            }
        }
        lua_pop(L,3);
        return 0;
    }

    static int tostring_T (lua_State *L)
    {
        char buff[32];
        userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
        T *obj = ud->pT;
        sprintf(buff, "%p", obj);
        lua_pushfstring(L, "%s (%s)", GetTClassName<T>(), buff);
        return 1;
    }

    HEARTHSTONE_INLINE static void tostring(char * buff,void * obj)
    {
        sprintf(buff,"%p",obj);
    }

    static int index(lua_State * L)
    {
        /*Paroxysm : the table obj and the missing key are currently on the stack(index 1 & 2) */
        lua_getglobal(L,GetTClassName<T>());
        // string form of the key.
        const char * key = lua_tostring(L,2);
        if(lua_istable(L,-1))
        {
            lua_pushvalue(L,2);
            lua_rawget(L,-2);
            //If the key were looking for is not in the table, retrieve its' metatables' index value.
            if(lua_isnil(L,-1))
            {
                lua_getmetatable(L,-2);
                if(lua_istable(L,-1) )
                {
                    lua_getfield(L,-1,"__index");
                    if(lua_isfunction(L,-1) )
                    {
                        lua_pushvalue(L,1);
                        lua_pushvalue(L,2);
                        lua_pcall(L,2,1,0);
                    }
                    else if(lua_istable(L,-1))
                        lua_getfield(L,-1,key);
                    else
                        lua_pushnil(L);
                }
                else
                    lua_pushnil(L);
            }
            else if(lua_istable(L,-1) )
            {
                lua_pushvalue(L,2);
                lua_rawget(L,-2);
            }
        }
        else
            lua_pushnil(L);

        lua_insert(L,1);
        lua_settop(L,1);
        return 1;
    }
};

#endif // __LUALUNAR_H
