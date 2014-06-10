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

#ifndef __LUAENGINE_H
#define __LUAENGINE_H

class LuaEngine
{
private:
    lua_State *L;
    Mutex m_Lock, co_lock;

public:
    LuaEngine();
    ~LuaEngine();

    HEARTHSTONE_INLINE lua_State* GetMainLuaState() { return L; }
    void LoadScripts();
    void ScriptLoadDir(char* Dirname, LUALoadScripts *pak);
    void Shutdown();
    void Restart();
    void RegisterCoreFunctions();
    HEARTHSTONE_INLINE Mutex& getcoLock() { return co_lock; }
    HEARTHSTONE_INLINE Mutex& GetLock() { return m_Lock; }

    void BeginCall(uint16);
    bool ExecuteCall(uint8 params = 0,uint8 res = 0);
    void EndCall(uint8 res = 0);

    void PushUnit(Object * unit, lua_State * LuaS = NULL);
    void PushGo(Object * go, lua_State * LuaS = NULL);
    void PushItem(Object * item, lua_State * LuaS = NULL);
    void PushGuid(uint64 guid, lua_State * LuaS = NULL);
    void PushPacket(WorldPacket * packet, lua_State * LuaS = NULL);
    void PushTaxiPath(TaxiPath * tp, lua_State * LuaS = NULL);
    void PushSpell(Spell * sp, lua_State * LuaS = NULL);
    void PushSQLField(Field * field, lua_State * LuaS = NULL);
    void PushSQLResult(QueryResult * res, lua_State * LuaS = NULL);
    void PushAura(Aura * aura, lua_State * LuaS = NULL);

    HEARTHSTONE_INLINE void PushBool(bool bewl) { lua_pushboolean(L, (bewl ? 1 : 0)); }
    HEARTHSTONE_INLINE void PushNil(lua_State * LuaS = NULL) { lua_pushnil((LuaS != NULL ? LuaS : L)); }
    HEARTHSTONE_INLINE void PushInt(int32 value) { lua_pushinteger(L, value); }
    HEARTHSTONE_INLINE void PushUint(uint32 value) { lua_pushnumber(L, value); }
    HEARTHSTONE_INLINE void PushFloat(float value) { lua_pushnumber(L, value); }
    HEARTHSTONE_INLINE void PushString(const char * str) { lua_pushstring(L, str); }
};

int Lua_CreateLuaEvent(lua_State * L);
int Lua_DestroyLuaEvent(lua_State * L);
int Lua_ModifyLuaEventInterval(lua_State * L);

int Lua_RegisterUnitEvent(lua_State * L);
int Lua_RegisterQuestEvent(lua_State * L);
int Lua_RegisterGameObjectEvent(lua_State * L);
int Lua_RegisterUnitGossipEvent(lua_State * L);
int Lua_RegisterItemGossipEvent(lua_State * L);
int Lua_RegisterGOGossipEvent(lua_State * L);

// Hyp Arc
int Lua_RegisterServerHook(lua_State * L);
int Lua_SuspendLuaThread(lua_State * L);
int Lua_RegisterTimedEvent(lua_State * L);
int Lua_RemoveTimedEvents(lua_State * L);
int Lua_RegisterDummySpell(lua_State * L);
int Lua_RegisterInstanceEvent(lua_State * L);
void Lua_RegisterGlobalFunctions(lua_State*);

static int ExtractfRefFromCString(lua_State* L, const char* functionName)
{
    int functionRef = 0;
    int top = lua_gettop(L);
    if(functionName != NULL)
    {
        char* copy = strdup(functionName);
        char* token = strtok(copy, ".:");
        if(strpbrk(functionName, ".:") == NULL)
        {
            lua_getglobal(L, functionName);
            if(lua_isfunction(L, -1) && !lua_iscfunction(L, -1))
                functionRef = lua_ref(L, true);
            else
                luaL_error(L, "Reference creation failed! (%s) is not a valid Lua function. \n", functionName);
        }
        else
        {
            lua_getglobal(L, "_G");
            while(token != NULL)
            {
                lua_getfield(L, -1, token);
                if(lua_isfunction(L, -1) && !lua_iscfunction(L, -1))
                {
                    functionRef = lua_ref(L, true);
                    break;
                }
                else if(lua_istable(L, -1))
                {
                    token = strtok(NULL, ".:");
                    continue;
                }
                else
                {
                    luaL_error(L, "Reference creation failed! (%s) is not a valid Lua function. \n", functionName);
                    break;
                }
            }
        }
#ifndef _DEBUG
        free((void*)copy);
#endif
        lua_settop(L, top);
    }
    return functionRef;
}

#endif

