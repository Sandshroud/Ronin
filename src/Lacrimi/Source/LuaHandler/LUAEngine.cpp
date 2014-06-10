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

LuaEngine::LuaEngine()
{
    L = lua_open();
}

LuaEngine::~LuaEngine()
{
    lua_close(L);
}

void LuaEngine::Restart()
{
    m_Lock.Acquire();
    lua_close(L);
    L = lua_open();
    LoadScripts();
    m_Lock.Release();
}

void LuaEngine::ScriptLoadDir(char* Dirname, LUALoadScripts *pak)
{
#ifdef WIN32
    HANDLE hFile;
    WIN32_FIND_DATA FindData;
    memset(&FindData,0,sizeof(FindData));

    char SearchName[MAX_PATH];

    strcpy(SearchName,Dirname);
    strcat(SearchName,"\\*.*");

    hFile = FindFirstFile(SearchName,&FindData);
    FindNextFile(hFile, &FindData);

    while( FindNextFile(hFile, &FindData) )
    {
        if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) //Credits for this 'if' go to Cebernic from ArcScripts Team. Thanks, you saved me some work ;-)
        {
            strcpy(SearchName,Dirname);
            strcat(SearchName,"\\");
            strcat(SearchName,FindData.cFileName);
            ScriptLoadDir(SearchName, pak);
        }
        else
        {
            string fname = Dirname;
            fname += "\\";
            fname += FindData.cFileName;

            int len = int(strlen(fname.c_str()));
            int i = 0;
            char ext[MAX_PATH];

            while(len > 0)
            {  
                ext[i++] = fname[--len];
                if(fname[len] == '.')
                    break;
            }
            ext[i++] = '\0';
            if ( !_stricmp(ext,"aul.") ) 
                pak->luaFiles.insert(fname);
        }
    }
    FindClose(hFile);

#else

    char *pch=strrchr(Dirname,'/');
    if (strcmp(Dirname, "..")==0 || strcmp(Dirname, ".")==0) return; //Against Endless-Loop
    if (pch != NULL && (strcmp(pch, "/..")==0 || strcmp(pch, "/.")==0 || strcmp(pch, "/.svn")==0)) return;
    struct dirent ** list;
    int filecount = scandir(Dirname, &list, 0, 0);

    if(filecount <= 0 || !list)
        return;

    struct stat attributes;
    bool err;
    Log.Success("LuaEngine", "Scanning Directory %s", Dirname);
    while(filecount--)
    {
        char dottedrelpath[200];
        sprintf(dottedrelpath, "%s/%s", Dirname, list[filecount]->d_name);
        err = false;
        if(stat(dottedrelpath, &attributes) == -1)
        {
            err = true;
            Log.Error("LuaEngine","Error opening %s: %s\n", dottedrelpath, strerror(errno));
        }

        if (!err && S_ISDIR(attributes.st_mode))
        {
            ScriptLoadDir((char *)dottedrelpath, pak); //Subdirectory
        }
        else
        {
            char* ext = strrchr(list[filecount]->d_name, '.');
            if(ext != NULL && !strcmp(ext, ".lua"))
            {
                pak->luaFiles.insert(dottedrelpath);
            }
        }

        free(list[filecount]);
    }
    free(list);

#endif
}

void LuaEngine::LoadScripts()
{
    LUALoadScripts rtn;
    sLog.Notice("LuaEngine", "Scanning Script-Directories...");
    ScriptLoadDir(((char*)sWorld.LuaScriptPath.c_str()), &rtn);

    unsigned int cnt_uncomp = 0;
    luaL_openlibs(L);
    RegisterCoreFunctions();
    sLog.Notice("LuaEngine", "Loading Scripts...");

    char filename[200];

    for(set<string>::iterator itr = rtn.luaFiles.begin(); itr != rtn.luaFiles.end(); ++itr)
    {
        strcpy(filename, itr->c_str());
        if(luaL_loadfile(L, filename) != 0)
        {
            sLog.Error("LuaEngine", "loading %s failed.(could not load)", itr->c_str());
            report(L);
            continue;
        }
        else
        {
            if(lua_pcall(L, 0, 0, 0) != 0)
            {
                sLog.Error("LuaEngine", "%s failed.(could not run)", itr->c_str());
                report(L);
                continue;
            }
        }
        cnt_uncomp++;
    }
    sLog.Notice("LuaEngine","Loaded %u Lua scripts.", cnt_uncomp);
}

/*******************************************************************************
    FUNCTION CALL METHODS
*******************************************************************************/
void LuaEngine::BeginCall(uint16 fReference)
{
    lua_settop(L, 0); //stack should be empty
    lua_getref(L, fReference);
}

bool LuaEngine::ExecuteCall(uint8 params, uint8 res)
{
    bool ret = true;
    int top = lua_gettop(L);
    if(strcmp(luaL_typename(L,top-params),"function") )
    {
        ret = false;
        //Paroxysm : Stack Cleaning here, not sure what causes that crash in luaH_getstr, maybe due to lack of stack space. Anyway, experimental.
        if(params > 0)
        {
            for(int i = top; i >= (top-params); i--)
            {
                if(!lua_isnone(L,i) )
                    lua_remove(L,i);
            }
        }
    }
    else
    {
        if(lua_pcall(L, params, res, 0))
        {
            report(L);
            ret = false;
        }
    }
    return ret;
}

void LuaEngine::EndCall(uint8 res) 
{
    for(int i = res; i > 0; i--)
    {
        if(!lua_isnone(L,res))
            lua_remove(L,res);
    }
}
/*******************************************************************************
    END FUNCTION CALL METHODS
*******************************************************************************/

/******************************************************************************
    PUSH METHODS
******************************************************************************/
void LuaEngine::PushUnit(Object * unit, lua_State * LuaS) 
{
    Unit * pUnit = NULL;
    if(unit != NULL && unit->IsUnit()) 
        pUnit = TO_UNIT(unit);
    if(LuaS == NULL)
        Lunar<Unit>::push(L, pUnit);
    else
        Lunar<Unit>::push(LuaS, pUnit);
}

void LuaEngine::PushGo(Object *go, lua_State *LuaS)
{
    GameObject * pGo = NULL;
    if(go != NULL && go->IsGameObject())
        pGo = TO_GAMEOBJECT(go);
    if(LuaS == NULL)
        Lunar<GameObject>::push(L, pGo);
    else
        Lunar<GameObject>::push(LuaS, pGo);
}

void LuaEngine::PushItem(Object * item, lua_State *LuaS)
{
    Item * pItem = NULL;
    if(item != NULL && (item->GetTypeId() == TYPEID_ITEM || item->GetTypeId() == TYPEID_CONTAINER))
        pItem = TO_ITEM(item);
    if(LuaS == NULL)
        Lunar<Item>::push(L, pItem);
    else
        Lunar<Item>::push(LuaS, pItem);
}

void LuaEngine::PushGuid(uint64 guid, lua_State * LuaS) 
{
    if(LuaS == NULL)
        GuidMgr::push(L, guid);
    else
        GuidMgr::push(LuaS, guid);
}

void LuaEngine::PushPacket(WorldPacket * pack, lua_State * LuaS) 
{
    if(LuaS == NULL)
        Lunar<WorldPacket>::push(L, pack, true);
    else
        Lunar<WorldPacket>::push(LuaS, pack, true);
}

void LuaEngine::PushTaxiPath(TaxiPath * tp, lua_State * LuaS) 
{
    if(LuaS == NULL)
        Lunar<TaxiPath>::push(L, tp, true);
    else
        Lunar<TaxiPath>::push(LuaS, tp, true);
}

void LuaEngine::PushSpell(Spell * sp, lua_State * LuaS) 
{
    if(LuaS == NULL)
        Lunar<Spell>::push(L, sp);
    else
        Lunar<Spell>::push(LuaS, sp);
}

void LuaEngine::PushSQLField(Field *field, lua_State *LuaS)
{
    if(LuaS == NULL)
        Lunar<Field>::push(L, field);
    else
        Lunar<Field>::push(LuaS, field);
}

void LuaEngine::PushSQLResult(QueryResult * res, lua_State * LuaS)
{
    if(LuaS == NULL)
        Lunar<QueryResult>::push(L, res, true);
    else
        Lunar<QueryResult>::push(LuaS, res, true);
}

void LuaEngine::PushAura(Aura * aura, lua_State * LuaS)
{
    if(LuaS == NULL)
        Lunar<Aura>::push(L, aura);
    else
        Lunar<Aura>::push(LuaS, aura);
}

/*******************************************************************************
    END PUSH METHODS
*******************************************************************************/

void LuaEngine::RegisterCoreFunctions()
{
    lua_pushcfunction(L, &Lua_RegisterUnitEvent);
    lua_setglobal(L, "RegisterUnitEvent");
    lua_pushcfunction(L, &Lua_RegisterGameObjectEvent);
    lua_setglobal(L, "RegisterGameObjectEvent");
    lua_pushcfunction(L, &Lua_RegisterQuestEvent);
    lua_setglobal(L, "RegisterQuestEvent");

    // Unit, Item, GO gossip stuff
    lua_pushcfunction(L, &Lua_RegisterUnitGossipEvent);
    lua_setglobal(L, "RegisterUnitGossipEvent");
    lua_pushcfunction(L, &Lua_RegisterItemGossipEvent);
    lua_setglobal(L, "RegisterItemGossipEvent");
    lua_pushcfunction(L, &Lua_RegisterGOGossipEvent);
    lua_setglobal(L, "RegisterGOGossipEvent");

    // Hyp Arc
    lua_pushcfunction(L, &Lua_RegisterServerHook);
    lua_setglobal(L, "RegisterServerHook");
    lua_pushcfunction(L, &Lua_SuspendLuaThread);
    lua_setglobal(L, "SuspendThread");
    lua_pushcfunction(L, &Lua_RegisterTimedEvent);
    lua_setglobal(L, "RegisterTimedEvent");
    lua_pushcfunction(L, &Lua_RemoveTimedEvents);
    lua_setglobal(L, "RemoveTimedEvents");
    lua_pushcfunction(L, &Lua_RegisterDummySpell);
    lua_setglobal(L, "RegisterDummySpell");
    lua_pushcfunction(L, &Lua_RegisterInstanceEvent);
    lua_setglobal(L, "RegisterInstanceEvent");
    lua_pushcfunction(L, &Lua_CreateLuaEvent);
    lua_setglobal(L, "CreateLuaEvent");
    lua_pushcfunction(L, &Lua_ModifyLuaEventInterval);
    lua_setglobal(L, "ModifyLuaEventInterval");
    lua_pushcfunction(L, &Lua_DestroyLuaEvent);
    lua_setglobal(L, "DestroyLuaEvent");

    Lua_RegisterGlobalFunctions(L);
    Lunar<Item>::Register(L);
    Lunar<Unit>::Register(L);
    Lunar<GameObject>::Register(L);
    Lunar<Aura>::Register(L);
    Lunar<WorldPacket>::Register(L);
    Lunar<TaxiPath>::Register(L);
    Lunar<Spell>::Register(L);
    Lunar<Field>::Register(L);
    Lunar<QueryResult>::Register(L);
    GuidMgr::Register(L);

    //set the suspendluathread a coroutine function
    lua_getglobal(L, "coroutine");
    if(lua_istable(L, -1))
    {
        lua_pushcfunction(L, Lua_SuspendLuaThread);
        lua_setfield(L, -2, "wait");
        lua_pushcfunction(L, Lua_SuspendLuaThread);
        lua_setfield(L, -2, "WAIT");
    }
    lua_pop(L, 1);
}

template<> const char * GetTClassName<Item>() { return "Item"; };
template<> const char * GetTClassName<Unit>() { return "Unit"; };
template<> const char * GetTClassName<GameObject>() { return "GameObject"; };
template<> const char * GetTClassName<WorldPacket>() { return "LuaPacket"; }
template<> const char * GetTClassName<TaxiPath>() { return "LuaTaxi"; }
template<> const char * GetTClassName<Spell>() { return "Spell"; }
template<> const char * GetTClassName<Field>() { return "SQL_Field"; }
template<> const char * GetTClassName<QueryResult>() { return "SQL_QResult"; }
template<> const char * GetTClassName<Aura>() { return "LuaAura"; }

/*
    This version only accepts actual Lua functions and no arguments. See LCF_Extra:CreateClosure(...) to pass arguments to this function.
    */
int Lua_CreateLuaEvent(lua_State * L)
{
    int delay = luaL_checkinteger(L,2);
    int repeats = luaL_checkinteger(L,3);
    if(!strcmp(luaL_typename(L,1),"function") || delay > 0)
    {
        lua_settop(L,1);
        int functionRef = lua_ref(L,true);
        TimedEvent * ev = TimedEvent::Allocate(World::getSingletonPtr(), new CallbackP1<LuaEngineMgr,int>(&g_luaMgr, &LuaEngineMgr::CallFunctionByReference, functionRef), 0, delay, repeats);
        ev->eventType  = LUA_EVENTS_END+functionRef; //Create custom reference by adding the ref number to the max lua event type to get a unique reference for every function.
        sWorld.event_AddEvent(ev);
        g_luaMgr.getFunctionRefs().insert(functionRef);
        lua_pushinteger(L,functionRef);
    }
    else
        lua_pushnil(L);

    return 1;
}

int Lua_DestroyLuaEvent(lua_State * L)
{
    //Simply remove the reference, CallFunctionByReference will find the reference has been freed and skip any processing.
    int ref = luaL_checkinteger(L,1);
    lua_unref(L,ref);
    g_luaMgr.getFunctionRefs().erase(ref);
    sEventMgr.RemoveEvents(World::getSingletonPtr(),ref+LUA_EVENTS_END);
    return 1;
}

int Lua_ModifyLuaEventInterval(lua_State * L)
{
    int ref = luaL_checkinteger(L,1);
    int newinterval = luaL_checkinteger(L,2);
    ref+= LUA_EVENTS_END;
    //Easy interval modification.
    sEventMgr.ModifyEventTime(World::getSingletonPtr(),ref,newinterval);
    return 1;
}

int Lua_RegisterUnitEvent(lua_State * L)
{
    lua_settop(L,3);
    uint16 functionRef = 0;
    int entry = luaL_checkint(L, 1);
    int ev = luaL_checkint(L, 2);
    const char * typeName = luaL_typename(L,3);
    if(!entry || !ev || typeName == NULL )
        return 0;

    if(!strcmp(typeName,"function"))
        functionRef = (uint16)lua_ref(L,true);
    else if(!strcmp(typeName,"string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = ExtractfRefFromCString(L, luaL_checkstring(L,3));
    if(functionRef > 0)
        g_luaMgr.RegisterEvent(REGTYPE_UNIT,entry,ev,functionRef);

    lua_pop(L, lua_gettop(L));
    return 1;
}

int Lua_RegisterInstanceEvent(lua_State* L)
{
    lua_settop(L, 3);
    uint16 functionRef = 0;
    int map = luaL_checkint(L, 1);
    int ev = luaL_checkint(L, 2);
    const char* typeName = luaL_typename(L, 3);
    if(!map || !ev || typeName == NULL)
        return 0;

    if(!strcmp(typeName, "function"))
        functionRef = (uint16)lua_ref(L, true);
    else if(!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = ExtractfRefFromCString(L, luaL_checkstring(L, 3));
    if(functionRef > 0)
        g_luaMgr.RegisterEvent(REGTYPE_INSTANCE, map, ev, functionRef);

    lua_pop(L, lua_gettop(L));
    return 0;
}

int Lua_RegisterQuestEvent(lua_State * L)
{
    lua_settop(L,3);
    uint16 functionRef = 0;
    int entry = luaL_checkint(L, 1);
    int ev = luaL_checkint(L, 2);
    const char * typeName = luaL_typename(L,3);
    if(!entry || !ev || typeName == NULL)
        return 0;

    if(!strcmp(typeName,"function"))
        functionRef = (uint16)lua_ref(L,true);
    else if(!strcmp(typeName,"string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = ExtractfRefFromCString(L,luaL_checkstring(L,3));
    if(functionRef > 0)
        g_luaMgr.RegisterEvent(REGTYPE_QUEST,entry,ev,functionRef);

    lua_pop(L, lua_gettop(L));
    return 1;
}

int Lua_RegisterDummySpell(lua_State * L)
{
    uint16 functionRef = 0;
    uint32 entry = luaL_checkint(L, 1);
    const char* typeName = luaL_typename(L, 2);
    lua_settop(L, 2);

    if(!entry || typeName == NULL)
        return 0;

    if(g_luaMgr.m_luaDummySpells.find(entry) != g_luaMgr.m_luaDummySpells.end())
        luaL_error(L, "LuaEngineMgr : RegisterDummySpell failed! Spell %d already has a registered Lua function!", entry);
    if(!strcmp(typeName, "function"))
        functionRef = (uint16)lua_ref(L, true);
    else if(!strcmp(typeName, "string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = ExtractfRefFromCString(L, luaL_checkstring(L, 2));
    if(functionRef > 0)
        g_luaMgr.RegisterEvent(REGTYPE_DUMMYSPELL, entry, 0, functionRef);

    lua_pop(L, lua_gettop(L));
    return 0;
}

int Lua_RegisterGameObjectEvent(lua_State * L)
{
    lua_settop(L,3);
    uint16 functionRef = 0;
    int entry = luaL_checkint(L, 1);
    int ev = luaL_checkint(L, 2);
    const char * typeName = luaL_typename(L,3);
    if(!entry || !ev || typeName == NULL)
        return 0;

    if(!strcmp(typeName,"function"))
        functionRef = (uint16)lua_ref(L,true);
    else if(!strcmp(typeName,"string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = ExtractfRefFromCString(L,luaL_checkstring(L,3));
    if(functionRef > 0)
        g_luaMgr.RegisterEvent(REGTYPE_GO,entry,ev,functionRef);

    lua_pop(L, lua_gettop(L));
    return 1;
}

// Gossip stuff
int Lua_RegisterUnitGossipEvent(lua_State * L)
{
    lua_settop(L,3);
    uint16 functionRef = 0;
    int entry = luaL_checkint(L, 1);
    int ev = luaL_checkint(L, 2);
    const char * typeName = luaL_typename(L,3);
    if(!entry || !ev || typeName == NULL)
        return 0;

    if(!strcmp(typeName,"function"))
        functionRef = (uint16)lua_ref(L,true);
    else if(!strcmp(typeName,"string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = ExtractfRefFromCString(L,luaL_checkstring(L,3));
    if(functionRef > 0)
        g_luaMgr.RegisterEvent(REGTYPE_UNIT_GOSSIP,entry,ev,functionRef);

    lua_pop(L, lua_gettop(L));
    return 1;
}

int Lua_RegisterItemGossipEvent(lua_State * L)
{
    lua_settop(L,3);
    uint16 functionRef = 0;
    int entry = luaL_checkint(L, 1);
    int ev = luaL_checkint(L, 2);
    const char * typeName = luaL_typename(L,3);
    if(!entry || !ev || typeName == NULL)
        return 0;

    if(!strcmp(typeName,"function"))
        functionRef = (uint16)lua_ref(L,true);
    else if(!strcmp(typeName,"string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = ExtractfRefFromCString(L,luaL_checkstring(L,3));
    if(functionRef > 0)
        g_luaMgr.RegisterEvent(REGTYPE_ITEM_GOSSIP,entry,ev,functionRef);

    lua_pop(L, lua_gettop(L));
    return 1;
}

int Lua_RegisterGOGossipEvent(lua_State * L)
{
    lua_settop(L,3);
    uint16 functionRef = 0;
    int entry = luaL_checkint(L, 1);
    int ev = luaL_checkint(L, 2);
    const char * typeName = luaL_typename(L,3);
    if(!entry || !ev || typeName == NULL)
        return 0;

    if(!strcmp(typeName,"function"))
        functionRef = (uint16)lua_ref(L,true);
    else if(!strcmp(typeName,"string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = ExtractfRefFromCString(L,luaL_checkstring(L,3));
    if(functionRef > 0)
        g_luaMgr.RegisterEvent(REGTYPE_GO_GOSSIP,entry,ev,functionRef);

    lua_pop(L, lua_gettop(L));
    return 1;
}

int Lua_SuspendLuaThread(lua_State * L)
{
    lua_State * thread = (lua_isthread(L,1)) ? lua_tothread(L,1) : NULL;
    if(thread == NULL)
        return luaL_error(L,"LuaEngineMgr","SuspendLuaThread expected Lua coroutine, got NULL.");

    int waitime = luaL_checkinteger(L,2);
    if(waitime <= 0)
        return luaL_error(L,"LuaEngineMgr","SuspendLuaThread expected timer > 0 instead got (%d)",waitime);

    lua_pushvalue(L,1);
    int ref = luaL_ref(L,LUA_REGISTRYINDEX);
    if(ref == LUA_REFNIL || ref == LUA_NOREF)
        return luaL_error(L,"Error in SuspendLuaThread! Failed to create a valid reference.");

    TimedEvent * evt = TimedEvent::Allocate(thread,new CallbackP1<LuaEngineMgr,int>(&g_luaMgr,&LuaEngineMgr::ResumeLuaThread,ref),0,waitime,1);
    sWorld.event_AddEvent(evt);
    lua_remove(L,1); // remove thread object
    lua_remove(L,1); // remove timer.
    //All that remains now are the extra arguments passed to this function.
    lua_xmove(L,thread,lua_gettop(L));
    g_luaMgr.getThreadRefs().insert(ref);
    return lua_yield(thread,lua_gettop(L));
}

int Lua_RegisterTimedEvent(lua_State * L) //in this case, L == lu
{
    const char * funcName = strdup(luaL_checkstring(L,1));
    int delay = luaL_checkint(L,2);
    int repeats = luaL_checkint(L,3);
    if (!delay || repeats < 0 || !funcName)
    {
        lua_pushnumber(L, LUA_REFNIL);
        return 1;
    }

    lua_remove(L, 1); 
    lua_remove(L, 1);//repeats, args
    lua_State * thread = lua_newthread(L); //repeats, args, thread
    lua_insert(L,1); //thread, repeats, args
    lua_xmove(L,thread,lua_gettop(L)-1); //thread
    int ref = luaL_ref(L, LUA_REGISTRYINDEX); //empty
    if(ref == LUA_REFNIL || ref == LUA_NOREF)
        return luaL_error(L,"Error in RegisterTimedEvent! Failed to create a valid reference.");

    TimedEvent *te = TimedEvent::Allocate(&g_luaMgr, new CallbackP2<LuaEngineMgr, const char*, int>(&g_luaMgr, &LuaEngineMgr::HyperCallFunction, funcName, ref), EVENT_LUA_TIMED, delay, repeats);
    EventInfoHolder * ek = new EventInfoHolder;
    ek->funcName = funcName;
    ek->te = te;
    g_luaMgr.m_registeredTimedEvents.insert( make_pair(ref, ek) );
    LuaEvent.event_AddEvent(te);
    lua_settop(L,0);
    lua_pushnumber(L, ref);
    return 1;
}

int Lua_RemoveTimedEvents(lua_State * L) //in this case, L == lu
{
    LuaEvent.RemoveEvents();
    return 1;
}

int Lua_RegisterServerHook(lua_State * L)
{
    uint16 functionRef = 0;
    //Maximum passed in arguments, consider rest as garbage
    lua_settop(L,2);
    uint32 ev = luaL_checkint(L, 1);
    const char * typeName = luaL_typename(L,2);
    if(!ev || typeName == NULL)
        return 0;

    // For functions directly passed in, skip all that code and register the reference.
    if(!strcmp(typeName,"function"))
        functionRef = (uint16)lua_ref(L,true);
    else if(!strcmp(typeName,"string")) //Old way of passing in functions, obsolete but left in for compatability.
        functionRef = ExtractfRefFromCString(L,luaL_checkstring(L,2));

    if(functionRef > 0)
        g_luaMgr.RegisterEvent(REGTYPE_SERVHOOK,0,ev,functionRef);

    lua_pop(L, lua_gettop(L));
    return 1;
}
