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

// Hyper as in Hyperactive, Jk, its Hypersniper.
void LuaEngineMgr::HyperCallFunction(const char * FuncName, int ref)
{
    CREATE_L_PTR;
    string sFuncName = string(FuncName);
    char *copy = strdup(FuncName);
    char *token = strtok(copy, ".:");
    int top = 1;
    bool colon = false;
    //REMEMBER: top is always 1
    lua_settop(L, 0); //stack should be empty
    if (strpbrk(FuncName, ".:") == NULL ) //stack: empty
        lua_getglobal(L, FuncName);  //stack: function
    else
    {
        lua_getglobal(L, "_G"); //start out with the global table.  //stack: _G
        while (token != NULL)
        {
            lua_getfield(L, -1, token); //get the (hopefully) table/func  //stack: _G, subtable/func/nil
            if ((int)sFuncName.find(token)-1 > 0) //if it isn't the first token
            {
                if (sFuncName.at(sFuncName.find(token)-1) == ':')
                    colon = true;
            }

            if (lua_isfunction(L, -1) && !lua_iscfunction(L, -1)) //if it's a Lua function //stack: _G/subt, func
            {
                if (colon) //stack: subt, func
                {
                    lua_pushvalue(L, -2); //make the table the first arg //stack: subt, func, subt
                    lua_remove(L, top); //stack: func, subt
                }
                else
                    lua_replace(L, top); //stack: func
                break; //we don't need anything else
            }
            else if(lua_istable(L, -1) ) //stack: _G/subt, subtable
            {
                token = strtok(NULL, ".:"); //stack: _G/subt, subtable
                lua_replace(L, top); //stack: subtable
            }
        }
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    lua_State * M = lua_tothread(L, -1); //repeats, args
    int thread = lua_gettop(L);
    int repeats = luaL_checkinteger(M, 1); //repeats, args
    int nargs = lua_gettop(M)-1;
    if (nargs != 0) //if we HAVE args...
    {
        for (int i = 2; i <= nargs+1; i++)
        {
            lua_pushvalue(M, i);
        }
        lua_xmove(M, L, nargs);
    }

    if(repeats != 0)
    {
        if (--repeats == 0) //free stuff, then
        {
            free((void*)FuncName);
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
            hash_map<int, EventInfoHolder*>::iterator itr = g_luaMgr.m_registeredTimedEvents.find(ref);
            g_luaMgr.m_registeredTimedEvents.erase(itr);
        }
        else
        {
            lua_remove(M, 1); //args
            lua_pushinteger(M, repeats); //args, repeats
            lua_insert(M, 1); //repeats, args
        }
    }
    lua_remove(L, thread); //now we can remove the thread object
    int r = lua_pcall(L, nargs+(colon ? 1 : 0), 0, 0);
    if (r)
        report(L);
    free((void*)copy);
    lua_settop(L, 0);
}

void LuaEngineMgr::Unload()
{
    CREATE_L_PTR;
    Lua_RemoveTimedEvents(L);
    DestroyAllLuaEvents(); // stop all pending events.

    // clean up the engine of any existing defined variables
    for(UnitBindingMap::iterator itr = m_unitBinding.begin(); itr != m_unitBinding.end(); ++itr)
    {
        for(int i = 0; i < CREATURE_EVENT_COUNT; ++i)
        {
            if(itr->second.Functions[i] > 0)
                lua_unref(L, itr->second.Functions[i]);
        }
    }
    m_unitBinding.clear();
    for(GameObjectBindingMap::iterator itr = m_gameobjectBinding.begin(); itr != m_gameobjectBinding.end(); ++itr)
    {
        for(int i = 0; i < GAMEOBJECT_EVENT_COUNT; ++i)
        {
            if(itr->second.Functions[i] > 0)
                lua_unref(L, itr->second.Functions[i]);
        }
    }
    m_gameobjectBinding.clear();
    for(QuestBindingMap::iterator itr = m_questBinding.begin(); itr != m_questBinding.end(); ++itr)
    {
        for(int i = 0; i < QUEST_EVENT_COUNT; ++i)
        {
            if(itr->second.Functions[i] > 0)
                lua_unref(L, itr->second.Functions[i]);
        }
    }
    m_questBinding.clear();
    for(InstanceBindingMap::iterator itr = m_instanceBinding.begin(); itr != m_instanceBinding.end(); ++itr)
    {
        for(int i = 0; i < INSTANCE_EVENT_COUNT; ++i)
        {
            if(itr->second.Functions[i] > 0)
                lua_unref(L, itr->second.Functions[i]);
        }
    }
    m_instanceBinding.clear();
    for(GossipUnitScriptsBindingMap::iterator itr = m_unit_gossipBinding.begin(); itr != m_unit_gossipBinding.end(); ++itr)
    {
        for(int i = 0; i < GOSSIP_EVENT_COUNT; ++i)
        {
            if(itr->second.Functions[i] > 0)
                lua_unref(L, itr->second.Functions[i]);
        }
    }
    m_unit_gossipBinding.clear();
    for(GossipItemScriptsBindingMap::iterator itr = m_item_gossipBinding.begin(); itr != m_item_gossipBinding.end(); ++itr)
    {
        for(int i = 0; i < GOSSIP_EVENT_COUNT; ++i)
        {
            if(itr->second.Functions[i] > 0)
                lua_unref(L, itr->second.Functions[i]);
        }
    }
    m_item_gossipBinding.clear();
    for(GossipGOScriptsBindingMap::iterator itr = m_go_gossipBinding.begin(); itr != m_go_gossipBinding.end(); ++itr)
    {
        for(int i = 0; i < GOSSIP_EVENT_COUNT; ++i)
        {
            if(itr->second.Functions[i] > 0)
                lua_unref(L, itr->second.Functions[i]);
        }
    }
    m_go_gossipBinding.clear();
    //Serv hooks : had forgotten these.
    for(int i = 0; i < NUM_SERVER_HOOKS; ++i)
    {
        vector<uint16> & next = EventAsToFuncName[i];
        for(vector<uint16>::iterator itr = next.begin(); itr != next.end(); ++itr)
            lua_unref(L, (*itr));
        next.clear();
    }
    for(map<uint32, uint16>::iterator itr = m_luaDummySpells.begin(); itr != m_luaDummySpells.end(); ++itr)
        lua_unref(L, itr->second);
    m_luaDummySpells.clear();
    for(set<int>::iterator itr = m_pendingThreads.begin(); itr != m_pendingThreads.end(); ++itr)
        lua_unref(L, (*itr));
    m_pendingThreads.clear();
    m_functionRefs.clear();
    lua_close(L);
}

void LuaEngineMgr::Restart(ScriptMgr* sMgr)
{
    sLog.Notice("LuaEngineMgr", "Restarting Engine.");
    CREATE_L_PTR;
    g_engine->getcoLock().Acquire();
    Unload();
    m_engine->LoadScripts();
    for(UnitBindingMap::iterator itr = m_unitBinding.begin(); itr != m_unitBinding.end(); ++itr)
    {
        typedef multimap<uint32, LuaCreature*> CMAP;
        CMAP & cMap = g_luaMgr.getLuCreatureMap();
        CMAP::iterator it = cMap.find(itr->first);
        CMAP::iterator itend = cMap.upper_bound(itr->first);
        if(it == cMap.end())
        {
            sMgr->register_creature_script(itr->first, CreateLuaCreature);
            cMap.insert(make_pair(itr->first, (LuaCreature*)NULL));
        }
        else
        {
            for(; it != itend; ++it)
            {
                if(it->second != NULL)
                    it->second->m_binding = &itr->second;
            }
        }
    }

    for(GameObjectBindingMap::iterator itr = m_gameobjectBinding.begin(); itr != m_gameobjectBinding.end(); ++itr)
    {
        typedef multimap<uint32, LuaGameObject*> GMAP;
        GMAP & gMap = g_luaMgr.getLuGameObjectMap();
        GMAP::iterator it = gMap.find(itr->first);
        GMAP::iterator itend = gMap.upper_bound(itr->first);
        if(it == gMap.end())
        {
            sMgr->register_gameobject_script(itr->first, CreateLuaGameObject);
            gMap.insert(make_pair(itr->first, (LuaGameObject*)NULL));
        }
        else
        {
            for(; it != itend; ++it)
            {
                if(it->second != NULL)
                    it->second->m_binding = &itr->second;
            }
        }
    }

    for(QuestBindingMap::iterator itr = m_questBinding.begin(); itr != m_questBinding.end(); ++itr)
    {
        typedef HM_NAMESPACE::hash_map<uint32, LuaQuest*> QMAP;
        QMAP & qMap = g_luaMgr.getLuQuestMap();
        QMAP::iterator it = qMap.find(itr->first);
        if(it == qMap.end())
        {
            sMgr->register_quest_script(itr->first, CreateLuaQuestScript(itr->first));
            qMap.insert(make_pair(itr->first, (LuaQuest*)NULL));
        }
        else
        {
            LuaQuest* q_interface = it->second;
            if(q_interface != NULL)
                q_interface->m_binding = &itr->second;
        }
    }

    for(InstanceBindingMap::iterator itr = m_instanceBinding.begin(); itr != m_instanceBinding.end(); ++itr)
    {
        typedef HM_NAMESPACE::hash_map<uint32, LuaInstance*> IMAP;
        IMAP & iMap = g_luaMgr.getLuInstanceMap();
        IMAP::iterator it = iMap.find(itr->first);
        if(it == iMap.end())
        {
            sMgr->register_mapmanager_script(itr->first, CreateLuaInstance);
            iMap.insert(make_pair(itr->first, (LuaInstance*)NULL));
        }
        else
        {
            if(it->second != NULL)
                it->second->m_binding = &itr->second;
        }
    }

    for(GossipUnitScriptsBindingMap::iterator itr = this->m_unit_gossipBinding.begin(); itr != m_unit_gossipBinding.end(); ++itr)
    {
        typedef HM_NAMESPACE::hash_map<uint32, LuaGossip*> GMAP;
        GMAP & gMap = g_luaMgr.getUnitGossipInterfaceMap();
        GMAP::iterator it = gMap.find(itr->first);
        if(it == gMap.end())
        {
            GossipScript* gs = CreateLuaUnitGossipScript(itr->first);
            if(gs != NULL)
            {
                sMgr->register_gossip_script(itr->first, gs);
                gMap.insert(make_pair(itr->first, (LuaGossip*)NULL));
            }
        }
        else
        {
            LuaGossip* u_gossip = it->second;
            if(u_gossip != NULL)
                u_gossip->m_unit_gossip_binding = &itr->second;
        }
    }

    for(GossipItemScriptsBindingMap::iterator itr = this->m_item_gossipBinding.begin(); itr != m_item_gossipBinding.end(); ++itr)
    {
        typedef HM_NAMESPACE::hash_map<uint32, LuaGossip*> GMAP;
        GMAP & gMap = g_luaMgr.getItemGossipInterfaceMap();
        GMAP::iterator it = gMap.find(itr->first);
        if(it == gMap.end())
        {
            GossipScript* gs = CreateLuaItemGossipScript(itr->first);
            if(gs != NULL)
            {
                sMgr->register_item_gossip_script(itr->first, gs);
                gMap.insert(make_pair(itr->first, (LuaGossip*)NULL));
            }
        }
        else
        {
            LuaGossip* i_gossip = it->second;
            if(i_gossip != NULL)
                i_gossip->m_item_gossip_binding = &itr->second;
        }
    }

    for(GossipGOScriptsBindingMap::iterator itr = this->m_go_gossipBinding.begin(); itr != m_go_gossipBinding.end(); ++itr)
    {
        typedef HM_NAMESPACE::hash_map<uint32, LuaGossip*> GMAP;
        GMAP & gMap = g_luaMgr.getGameObjectGossipInterfaceMap();
        GMAP::iterator it = gMap.find(itr->first);
        if(it == gMap.end())
        {
            GossipScript* gs = CreateLuaGOGossipScript(itr->first);
            if(gs != NULL)
            {
                sMgr->register_go_gossip_script(itr->first, gs);
                gMap.insert(make_pair(itr->first, (LuaGossip*)NULL));
            }
        }
        else
        {
            LuaGossip* g_gossip = it->second;
            if(g_gossip != NULL)
                g_gossip->m_go_gossip_binding = &itr->second;
        }
    }

    /* BIG SERV HOOK CHUNK EEK */
    RegisterHook(SERVER_HOOK_EVENT_ON_NEW_CHARACTER, Lua_HookOnNewCharacter);
    RegisterHook(SERVER_HOOK_EVENT_ON_KILL_PLAYER, Lua_HookOnKillPlayer);
    RegisterHook(SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD, Lua_HookOnFirstEnterWorld);
    RegisterHook(SERVER_HOOK_EVENT_ON_ENTER_WORLD, Lua_HookOnEnterWorld);
    RegisterHook(SERVER_HOOK_EVENT_ON_GUILD_JOIN, Lua_HookOnGuildJoin);
    RegisterHook(SERVER_HOOK_EVENT_ON_DEATH, Lua_HookOnDeath);
    RegisterHook(SERVER_HOOK_EVENT_ON_REPOP, Lua_HookOnRepop);
    RegisterHook(SERVER_HOOK_EVENT_ON_EMOTE, Lua_HookOnEmote);
    RegisterHook(SERVER_HOOK_EVENT_ON_ENTER_COMBAT, Lua_HookOnEnterCombat);
    RegisterHook(SERVER_HOOK_EVENT_ON_CAST_SPELL, Lua_HookOnCastSpell);
    RegisterHook(SERVER_HOOK_EVENT_ON_TICK, Lua_HookOnTick);
    RegisterHook(SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST, Lua_HookOnLogoutRequest);
    RegisterHook(SERVER_HOOK_EVENT_ON_LOGOUT, Lua_HookOnLogout);
    RegisterHook(SERVER_HOOK_EVENT_ON_QUEST_ACCEPT, Lua_HookOnQuestAccept);
    RegisterHook(SERVER_HOOK_EVENT_ON_ZONE, Lua_HookOnZone);
    RegisterHook(SERVER_HOOK_EVENT_ON_CHAT, Lua_HookOnChat);
    RegisterHook(SERVER_HOOK_EVENT_ON_LOOT, Lua_HookOnLoot);
    RegisterHook(SERVER_HOOK_EVENT_ON_GUILD_CREATE, Lua_HookOnGuildCreate);
    RegisterHook(SERVER_HOOK_EVENT_ON_FULL_LOGIN, Lua_HookOnEnterWorld2);
    RegisterHook(SERVER_HOOK_EVENT_ON_CHARACTER_CREATE, Lua_HookOnCharacterCreate);
    RegisterHook(SERVER_HOOK_EVENT_ON_QUEST_CANCELLED, Lua_HookOnQuestCancelled);
    RegisterHook(SERVER_HOOK_EVENT_ON_QUEST_FINISHED, Lua_HookOnQuestFinished);
    RegisterHook(SERVER_HOOK_EVENT_ON_HONORABLE_KILL, Lua_HookOnHonorableKill);
    RegisterHook(SERVER_HOOK_EVENT_ON_ARENA_FINISH, Lua_HookOnArenaFinish);
    RegisterHook(SERVER_HOOK_EVENT_ON_OBJECTLOOT, Lua_HookOnObjectLoot);
    RegisterHook(SERVER_HOOK_EVENT_ON_AREATRIGGER, Lua_HookOnAreaTrigger);
    RegisterHook(SERVER_HOOK_EVENT_ON_POST_LEVELUP, Lua_HookOnPostLevelUp);
    RegisterHook(SERVER_HOOK_EVENT_ON_PRE_DIE, Lua_HookOnPreUnitDie);
    RegisterHook(SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE, Lua_HookOnAdvanceSkillLine);
    RegisterHook(SERVER_HOOK_EVENT_ON_DUEL_FINISHED, Lua_HookOnDuelFinished);
    RegisterHook(SERVER_HOOK_EVENT_ON_AURA_REMOVE, Lua_HookOnAuraRemove);
    RegisterHook(SERVER_HOOK_EVENT_ON_RESURRECT, Lua_HookOnResurrect);

    for(std::map<uint32, uint16>::iterator itr = m_luaDummySpells.begin(); itr != m_luaDummySpells.end(); ++itr)
    {
        if(find(g_luaMgr.HookInfo.dummyHooks.begin(), g_luaMgr.HookInfo.dummyHooks.end(), itr->first) == g_luaMgr.HookInfo.dummyHooks.end())
        {
            sMgr->register_dummy_spell(itr->first, &Lua_HookOnDummySpell);
            g_luaMgr.HookInfo.dummyHooks.push_back(itr->first);
        }
    }
    RELEASE_LOCK
    g_engine->getcoLock().Release();

    //hyper: do OnSpawns for spawned creatures.
    vector<uint32> temp = OnLoadInfo;
    OnLoadInfo.clear();
    for(vector<uint32>::iterator itr = temp.begin(); itr != temp.end(); itr += 3)
    {
        //*itr = mapid; *(itr+1) = iid; *(itr+2) = lowguid
        MapMgr* mgr = NULL;
        if(*(itr + 1) == 0) //no instance
            mgr = sInstanceMgr.GetMapMgr(*itr);
        else
        {
            Instance* inst = sInstanceMgr.GetInstanceByIds(*itr, *(itr + 1));
            if(inst != NULL)
                mgr = inst->m_mapMgr;
        }
        if(mgr != NULL)
        {
            Creature* unit = mgr->GetCreature(*(itr + 2));
            if(unit != NULL && unit->IsInWorld() && unit->GetScript() != NULL)
                unit->GetScript()->OnLoad();
        }
    }
    temp.clear();

    sLog.Notice("LuaEngineMgr", "Done restarting engine.");
}

void LuaEngineMgr::CallFunctionByReference(int ref)
{
    CREATE_L_PTR;
    lua_getref(L, ref);
    if(lua_pcall(L, 0, 0, 0))
        report(L);
}

void LuaEngineMgr::DestroyAllLuaEvents()
{
    CREATE_L_PTR;
    //Clean up for all events.
    set<int>::iterator itr = m_functionRefs.begin();
    for(; itr != m_functionRefs.end(); ++itr)
    {
        sEventMgr.RemoveEvents(World::getSingletonPtr(), (*itr)+LUA_EVENTS_END);
        lua_unref(L, (*itr));
    }
    m_functionRefs.clear();
}

void LuaEngineMgr::Startup()
{
    sLog.Notice("LuaEngineMgr", "Spawning Lua Engine...");
    m_engine = new LuaEngine();
    sLacrimi.LuaEngineIsStarting = true;
    m_engine->LoadScripts();
    g_engine = m_engine;
    sLacrimi.LuaEngineIsStarting = false;

    // stuff is registered, so lets go ahead and make our emulated C++ scripted lua classes.
    for(UnitBindingMap::iterator itr = m_unitBinding.begin(); itr != m_unitBinding.end(); ++itr)
        sLacrimi.GetScriptMgr()->register_creature_script( itr->first, CreateLuaCreature );

    for(GameObjectBindingMap::iterator itr = m_gameobjectBinding.begin(); itr != m_gameobjectBinding.end(); ++itr)
        sLacrimi.GetScriptMgr()->register_gameobject_script( itr->first, CreateLuaGameObject );

    for(QuestBindingMap::iterator itr = m_questBinding.begin(); itr != m_questBinding.end(); ++itr)
        if( QuestScript * qs = CreateLuaQuestScript( itr->first ) )
            sLacrimi.GetScriptMgr()->register_quest_script( itr->first, qs );

    // Register Gossip Stuff
    for(GossipUnitScriptsBindingMap::iterator itr = m_unit_gossipBinding.begin(); itr != m_unit_gossipBinding.end(); ++itr)
        if( GossipScript * gs = CreateLuaUnitGossipScript( itr->first ) )
            sLacrimi.GetScriptMgr()->register_gossip_script( itr->first, gs );

    for(GossipItemScriptsBindingMap::iterator itr = m_item_gossipBinding.begin(); itr != m_item_gossipBinding.end(); ++itr)
        if( GossipScript * gs = CreateLuaItemGossipScript( itr->first ) )
            sLacrimi.GetScriptMgr()->register_item_gossip_script( itr->first, gs );

    for(GossipGOScriptsBindingMap::iterator itr = m_go_gossipBinding.begin(); itr != m_go_gossipBinding.end(); ++itr)
        if( GossipScript * gs = CreateLuaGOGossipScript( itr->first ) )
            sLacrimi.GetScriptMgr()->register_go_gossip_script( itr->first, gs );
}

void LuaEngineMgr::RegisterEvent(uint8 regtype, uint32 id, uint32 evt, uint16 functionRef) 
{
    switch(regtype) 
    {
        case REGTYPE_UNIT: 
            {
                if(id && evt && evt < CREATURE_EVENT_COUNT)
                {
                    LuaUnitBinding * bind = GetUnitBinding(id);
                    if(bind == NULL)
                    {
                        LuaUnitBinding nbind;
                        memset(&nbind,0,sizeof(LuaUnitBinding));
                        nbind.Functions[evt] = functionRef;
                        m_unitBinding.insert(make_pair(id,nbind));
                    }
                    else
                    {
                        if(bind->Functions[evt] > 0)
                            lua_unref(GLuas(), bind->Functions[evt]);
                        bind->Functions[evt] = functionRef;
                    }
                }
            }break;
        case REGTYPE_GO:
            {
                if(id && evt && evt < GAMEOBJECT_EVENT_COUNT)
                {
                    LuaGameObjectBinding * bind = GetGameObjectBinding(id);
                    if(bind == NULL)
                    {
                        LuaGameObjectBinding nbind;
                        memset(&nbind,0,sizeof(LuaGameObjectBinding));
                        nbind.Functions[evt] = functionRef;
                        m_gameobjectBinding.insert(make_pair(id,nbind));
                    }
                    else
                    {
                        if(bind->Functions[evt] > 0)
                            lua_unref(GLuas(), bind->Functions[evt]);
                        bind->Functions[evt] = functionRef;
                    }
                }
            }break;
        case REGTYPE_QUEST:
            {
                if(id && evt && evt < QUEST_EVENT_COUNT)
                {
                    LuaQuestBinding * bind = GetQuestBinding(id);
                    if(bind == NULL)
                    {
                        LuaQuestBinding nbind;
                        memset(&nbind,0,sizeof(LuaQuestBinding));
                        nbind.Functions[evt] = functionRef;
                        m_questBinding.insert(make_pair(id,nbind));
                    }
                    else
                    {
                        if(bind->Functions[evt] > 0)
                            lua_unref(GLuas(), bind->Functions[evt]);
                        bind->Functions[evt] = functionRef;
                    }
                }
            }break;
        case REGTYPE_SERVHOOK:
            {
                if(evt < NUM_SERVER_HOOKS)
                    EventAsToFuncName[evt].push_back(functionRef);
            }break;
        case REGTYPE_DUMMYSPELL: 
            {
                if (id)
                    m_luaDummySpells.insert( make_pair(id,functionRef) );
            }break;
        case REGTYPE_INSTANCE: 
            {
                if(id && evt && evt < INSTANCE_EVENT_COUNT)
                {
                    LuaInstanceBinding* bind = getInstanceBinding(id);
                    if(bind == NULL)
                    {
                        LuaInstanceBinding nbind;
                        memset(&nbind, 0, sizeof(LuaInstanceBinding));
                        nbind.Functions[evt] = functionRef;
                        m_instanceBinding.insert(make_pair(id, nbind));
                    }
                    else
                    {
                        if(bind->Functions[evt] > 0)
                            lua_unref(GLuas(), bind->Functions[evt]);
                        bind->Functions[evt] = functionRef;
                    }
                }
            }break;
        case REGTYPE_UNIT_GOSSIP:
            {
                if(id && evt && evt < GOSSIP_EVENT_COUNT)
                {
                    LuaUnitGossipBinding * bind = GetLuaUnitGossipBinding(id);
                    if(bind == NULL)
                    {
                        LuaUnitGossipBinding nbind;
                        memset(&nbind,0,sizeof(LuaUnitGossipBinding));
                        nbind.Functions[evt] = functionRef;
                        m_unit_gossipBinding.insert(make_pair(id,nbind));
                    }
                    else
                    {
                        if(bind->Functions[evt] > 0)
                            lua_unref(GLuas(), bind->Functions[evt]);
                        bind->Functions[evt] = functionRef;
                    }
                }
            }break;
        case REGTYPE_ITEM_GOSSIP:
        {
            if(id && evt && evt < GOSSIP_EVENT_COUNT)
            {
                LuaItemGossipBinding * bind = GetLuaItemGossipBinding(id);
                if(bind == NULL)
                {
                    LuaItemGossipBinding nbind;
                    memset(&nbind,0,sizeof(LuaItemGossipBinding));
                    nbind.Functions[evt] = functionRef;
                    m_item_gossipBinding.insert(make_pair(id,nbind));
                }
                else
                {
                    if(bind->Functions[evt] > 0)
                        lua_unref(GLuas(), bind->Functions[evt]);
                    bind->Functions[evt] = functionRef;
                }
            }
        }break;
        case REGTYPE_GO_GOSSIP:
        {
            if(id && evt && evt < GOSSIP_EVENT_COUNT)
            {
                LuaGOGossipBinding * bind = GetLuaGOGossipBinding(id);
                if(bind == NULL)
                {
                    LuaGOGossipBinding nbind;
                    memset(&nbind,0,sizeof(LuaGOGossipBinding));
                    nbind.Functions[evt] = functionRef;
                    m_go_gossipBinding.insert(make_pair(id,nbind));
                }
                else
                {
                    if(bind->Functions[evt] > 0)
                        lua_unref(GLuas(), bind->Functions[evt]);
                    bind->Functions[evt] = functionRef;
                }
            }
        }break;
    }
}

void LuaEngineMgr::ResumeLuaThread(int ref)
{
    CREATE_L_PTR;
    lua_State* expectedThread = NULL;
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    if(lua_isthread(L, -1))
        expectedThread = lua_tothread(L, -1);
    if(expectedThread != NULL)
    {
        //push ourself on the stack
        lua_pushthread(expectedThread);
        //move the thread to the main lu state(and pop it off)
        lua_xmove(expectedThread, L, 1);
        if(lua_rawequal(L, -1, -2))
        {
            lua_pop(L, 2);
            int res = lua_resume(expectedThread, lua_gettop(expectedThread));
            if(res != LUA_YIELD && res)
                report(expectedThread);
        }
        else
            lua_pop(L, 2);
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    }
}

void report(lua_State * L)
{
    int count = lua_gettop(L);
    while(count > 0)
    {
        const char* msg = lua_tostring(L, -1);
        printf("\t%s\n", msg);
        lua_pop(L, 1);
        count--;
    }
}
