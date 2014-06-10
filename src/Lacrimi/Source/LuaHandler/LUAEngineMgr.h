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

#ifndef __LUAENGINE_MGR_H
#define __LUAENGINE_MGR_H

class LuaEngineMgr
{
private:
    typedef HM_NAMESPACE::hash_map<uint32, LuaUnitBinding> UnitBindingMap;
    typedef HM_NAMESPACE::hash_map<uint32, LuaQuestBinding> QuestBindingMap;
    typedef HM_NAMESPACE::hash_map<uint32, LuaInstanceBinding> InstanceBindingMap;
    typedef HM_NAMESPACE::hash_map<uint32, LuaGameObjectBinding> GameObjectBindingMap;
    typedef HM_NAMESPACE::hash_map<uint32, LuaUnitGossipBinding> GossipUnitScriptsBindingMap;
    typedef HM_NAMESPACE::hash_map<uint32, LuaItemGossipBinding> GossipItemScriptsBindingMap;
    typedef HM_NAMESPACE::hash_map<uint32, LuaGOGossipBinding> GossipGOScriptsBindingMap;

    //maps to creature, & go script interfaces
    std::multimap<uint32, LuaCreature*> m_cAIScripts;
    std::multimap<uint32, LuaGameObject*> m_gAIScripts;
    HM_NAMESPACE::hash_map<uint32, LuaQuest*> m_qAIScripts;
    HM_NAMESPACE::hash_map<uint32, LuaInstance*> m_iAIScripts;
    HM_NAMESPACE::hash_map<uint32, LuaGossip*> m_unitgAIScripts;
    HM_NAMESPACE::hash_map<uint32, LuaGossip*> m_itemgAIScripts;
    HM_NAMESPACE::hash_map<uint32, LuaGossip*> m_gogAIScripts;

    std::set<int> m_pendingThreads;
    std::set<int> m_functionRefs;
    std::map< uint64,std::set<int> > m_objectFunctionRefs;

    UnitBindingMap m_unitBinding;
    QuestBindingMap m_questBinding;
    InstanceBindingMap m_instanceBinding;
    GameObjectBindingMap m_gameobjectBinding;
    GossipUnitScriptsBindingMap m_unit_gossipBinding;
    GossipItemScriptsBindingMap m_item_gossipBinding;
    GossipGOScriptsBindingMap m_go_gossipBinding;

public:
    GossipMenu *Menu;
    LuaEngine *m_engine;
    void Startup();
    void Unload();
    void Restart(ScriptMgr* sMgr);
    lua_State* GLuas() { return m_engine->GetMainLuaState(); }
    void RegisterEvent(uint8, uint32, uint32 , uint16);
    void ResumeLuaThread(int);
    void HyperCallFunction(const char *, int);
    void CallFunctionByReference(int);
    void DestroyAllLuaEvents();

    LuaUnitBinding * GetUnitBinding(uint32 Id)
    {
        UnitBindingMap::iterator itr = m_unitBinding.find(Id);
        return (itr == m_unitBinding.end()) ? NULL : &itr->second;
    }

    LuaQuestBinding * GetQuestBinding(uint32 Id)
    {
        QuestBindingMap::iterator itr = m_questBinding.find(Id);
        return (itr == m_questBinding.end()) ? NULL : &itr->second;
    }

    LuaGameObjectBinding * GetGameObjectBinding(uint32 Id)
    {
        GameObjectBindingMap::iterator itr =m_gameobjectBinding.find(Id);
        return (itr == m_gameobjectBinding.end()) ? NULL : &itr->second;
    }

    LuaInstanceBinding* getInstanceBinding(uint32 Id)
    {
        InstanceBindingMap::iterator itr = m_instanceBinding.find(Id);
        return (itr == m_instanceBinding.end()) ? NULL : &itr->second;
    }

    // Gossip Stuff
    LuaUnitGossipBinding * GetLuaUnitGossipBinding(uint32 Id)
    {
        GossipUnitScriptsBindingMap::iterator itr = m_unit_gossipBinding.find(Id);
        return (itr == m_unit_gossipBinding.end()) ? NULL : &itr->second;
    }

    LuaItemGossipBinding * GetLuaItemGossipBinding(uint32 Id)
    {
        GossipItemScriptsBindingMap::iterator itr = m_item_gossipBinding.find(Id);
        return (itr == m_item_gossipBinding.end()) ? NULL : &itr->second;
    }

    LuaGOGossipBinding * GetLuaGOGossipBinding(uint32 Id)
    {
        GossipGOScriptsBindingMap::iterator itr = m_go_gossipBinding.find(Id);
        return (itr == m_go_gossipBinding.end()) ? NULL : &itr->second;
    }

    HEARTHSTONE_INLINE Mutex& getLock() { return m_engine->GetLock(); }
    HEARTHSTONE_INLINE void Lock() { m_engine->GetLock().Acquire(); }
    HEARTHSTONE_INLINE void Unlock() { return m_engine->GetLock().Release(); }
    HEARTHSTONE_INLINE std::set<int> & getThreadRefs() { return m_pendingThreads; }
    HEARTHSTONE_INLINE std::set<int> & getFunctionRefs() { return m_functionRefs; }
    HEARTHSTONE_INLINE std::multimap<uint32, LuaCreature*> & getLuCreatureMap() { return m_cAIScripts; }
    HEARTHSTONE_INLINE std::multimap<uint32, LuaGameObject*> & getLuGameObjectMap() { return m_gAIScripts; }
    HEARTHSTONE_INLINE HM_NAMESPACE::hash_map<uint32, LuaQuest*> & getLuQuestMap() { return m_qAIScripts; }
    HEARTHSTONE_INLINE HM_NAMESPACE::hash_map<uint32, LuaInstance*> & getLuInstanceMap() { return m_iAIScripts; }
    HEARTHSTONE_INLINE hash_map<uint32, LuaGossip*> & getUnitGossipInterfaceMap() { return m_unitgAIScripts; }
    HEARTHSTONE_INLINE hash_map<uint32, LuaGossip*> & getItemGossipInterfaceMap() { return m_itemgAIScripts; }
    HEARTHSTONE_INLINE hash_map<uint32, LuaGossip*> & getGameObjectGossipInterfaceMap() { return m_gogAIScripts; }
    HEARTHSTONE_INLINE std::map< uint64,std::set<int> > & getObjectFunctionRefs() { return m_objectFunctionRefs; }
    hash_map<int, EventInfoHolder*> m_registeredTimedEvents;
    std::vector<uint16> EventAsToFuncName[NUM_SERVER_HOOKS];
    std::map<uint32, uint16> m_luaDummySpells;
    std::vector<uint32> OnLoadInfo;
    lLuaEventMgr LuaEventMgr;

    struct _ENGINEHOOKINFO
    {
        bool hooks[NUM_SERVER_HOOKS];
        std::vector<uint32> dummyHooks;
        _ENGINEHOOKINFO()
        {
            for(int i = 0; i < NUM_SERVER_HOOKS; ++i)
                hooks[i] = false;
        }
    }HookInfo;
};

void report(lua_State * L);

#endif
