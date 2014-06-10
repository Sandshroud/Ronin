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

LuaGossip::~LuaGossip()
{
    typedef HM_NAMESPACE::hash_map<uint32, LuaGossip*> MapType;
    MapType gMap;
    if(m_go_gossip_binding != NULL)
    {
        gMap = g_luaMgr.getGameObjectGossipInterfaceMap();
        for(MapType::iterator itr = gMap.begin(); itr != gMap.end(); ++itr)
        {
            if(itr->second == this) {
                gMap.erase(itr);
                break;
            }
        }
    }
    else if(m_unit_gossip_binding != NULL)
    {
        gMap = g_luaMgr.getUnitGossipInterfaceMap();
        for(MapType::iterator itr = gMap.begin(); itr != gMap.end(); ++itr)
        {
            if(itr->second == this)
            {
                gMap.erase(itr);
                break;
            }
        }
    }
    else if(m_item_gossip_binding != NULL)
    {
        gMap = g_luaMgr.getItemGossipInterfaceMap();
        for(MapType::iterator itr = gMap.begin(); itr != gMap.end(); ++itr)
        {
            if(itr->second == this)
            {
                gMap.erase(itr);
                break;
            }
        }
    }
}

void LuaGossip::GossipHello(Object* pObject, Player* Plr, bool AutoSend)
{
    GET_LOCK
    if(pObject->GetTypeId() == TYPEID_UNIT)
    {
        if(m_unit_gossip_binding == NULL)
        {
            RELEASE_LOCK
            return;
        }

        g_engine->BeginCall(m_unit_gossip_binding->Functions[GOSSIP_EVENT_ON_TALK]);
        g_engine->PushUnit(pObject);
        g_engine->PushUint(GOSSIP_EVENT_ON_TALK);
        g_engine->PushUnit(Plr);
        g_engine->PushBool(AutoSend);
        g_engine->ExecuteCall(4);
    }
    else if(pObject->GetTypeId() == TYPEID_ITEM)
    {
        if(m_item_gossip_binding == NULL)
        {
            RELEASE_LOCK
            return;
        }

        g_engine->BeginCall(m_item_gossip_binding->Functions[GOSSIP_EVENT_ON_TALK]);
        g_engine->PushItem(pObject);
        g_engine->PushUint(GOSSIP_EVENT_ON_TALK);
        g_engine->PushUnit(Plr);
        g_engine->PushBool(AutoSend);
        g_engine->ExecuteCall(4);
    }
    else if(pObject->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        if(m_go_gossip_binding == NULL)
        {
            RELEASE_LOCK
            return;
        }

        g_engine->BeginCall(m_go_gossip_binding->Functions[GOSSIP_EVENT_ON_TALK]);
        g_engine->PushGo(pObject);
        g_engine->PushUint(GOSSIP_EVENT_ON_TALK);
        g_engine->PushUnit(Plr);
        g_engine->PushBool(AutoSend);
        g_engine->ExecuteCall(4);
    }
    RELEASE_LOCK
}

void LuaGossip::GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char * EnteredCode)
{
    GET_LOCK
    if(pObject->GetTypeId() == TYPEID_UNIT)
    {
        if(m_unit_gossip_binding == NULL)
        {
            RELEASE_LOCK
            return;
        }

        g_engine->BeginCall(m_unit_gossip_binding->Functions[GOSSIP_EVENT_ON_SELECT_OPTION]);
        g_engine->PushUnit(pObject);
        g_engine->PushUint(GOSSIP_EVENT_ON_SELECT_OPTION);
        g_engine->PushUnit(Plr);
        g_engine->PushUint(Id);
        g_engine->PushUint(IntId);
        g_engine->PushString(EnteredCode);
        g_engine->ExecuteCall(6);
    }
    else if(pObject->GetTypeId() == TYPEID_ITEM)
    {
        if(m_item_gossip_binding == NULL)
        {
            RELEASE_LOCK
            return;
        }

        g_engine->BeginCall(m_item_gossip_binding->Functions[GOSSIP_EVENT_ON_SELECT_OPTION]);
        g_engine->PushItem(pObject);
        g_engine->PushUint(GOSSIP_EVENT_ON_SELECT_OPTION);
        g_engine->PushUnit(Plr);
        g_engine->PushUint(Id);
        g_engine->PushUint(IntId);
        g_engine->PushString(EnteredCode);
        g_engine->ExecuteCall(6);
    }
    else if(pObject->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        if(m_go_gossip_binding == NULL)
        {
            RELEASE_LOCK
            return;
        }

        g_engine->BeginCall(m_go_gossip_binding->Functions[GOSSIP_EVENT_ON_SELECT_OPTION]);
        g_engine->PushGo(pObject);
        g_engine->PushUint(GOSSIP_EVENT_ON_SELECT_OPTION);
        g_engine->PushUnit(Plr);
        g_engine->PushUint(Id);
        g_engine->PushUint(IntId);
        g_engine->PushString(EnteredCode);
        g_engine->ExecuteCall(6);
    }
    RELEASE_LOCK
}

// Player
void LuaInstance::OnPlayerDeath(Player* pVictim, Unit* pKiller)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[INSTANCE_EVENT_ON_PLAYER_DEATH]);
    g_engine->PushUint(m_instanceId);
    g_engine->PushUnit(pVictim);
    g_engine->PushUnit(pKiller);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
};

// Area and AreaTrigger
void LuaInstance::OnPlayerEnter(Player* pPlayer)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[INSTANCE_EVENT_ON_PLAYER_ENTER]);
    g_engine->PushUint(m_instanceId);
    g_engine->PushUnit(pPlayer);
    g_engine->ExecuteCall(2);
    RELEASE_LOCK
};

void LuaInstance::OnAreaTrigger(Player* pPlayer, uint32 uAreaId)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[INSTANCE_EVENT_ON_AREA_TRIGGER]);
    g_engine->PushUint(m_instanceId);
    g_engine->PushUnit(pPlayer);
    g_engine->PushUint(uAreaId);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
};

void LuaInstance::OnZoneChange(Player* pPlayer, uint32 uNewZone, uint32 uOldZone)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[INSTANCE_EVENT_ON_ZONE_CHANGE]);
    g_engine->PushUint(m_instanceId);
    g_engine->PushUnit(pPlayer);
    g_engine->PushUint(uNewZone);
    g_engine->PushUint(uOldZone);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
};

// Creature / GameObject - part of it is simple reimplementation for easier use Creature / GO < --- > Script
void LuaInstance::OnCreatureDeath(Creature* pVictim, Unit* pKiller)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[INSTANCE_EVENT_ON_CREATURE_DEATH]);
    g_engine->PushUint(m_instanceId);
    g_engine->PushUnit(pVictim);
    g_engine->PushUnit(pKiller);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
};

void LuaInstance::OnCreaturePushToWorld(Creature* pCreature)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[INSTANCE_EVENT_ON_CREATURE_PUSH]);
    g_engine->PushUint(m_instanceId);
    g_engine->PushUnit(pCreature);
    g_engine->ExecuteCall(2);
    RELEASE_LOCK
};

void LuaInstance::OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[INSTANCE_EVENT_ON_GO_ACTIVATE]);
    g_engine->PushUint(m_instanceId);
    g_engine->PushGo(pGameObject);
    g_engine->PushUnit(pPlayer);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
};

void LuaInstance::OnGameObjectPushToWorld(GameObject* pGameObject)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[INSTANCE_EVENT_ON_GO_PUSH]);
    g_engine->PushUint(m_instanceId);
    g_engine->PushGo(pGameObject);
    g_engine->ExecuteCall(2);
    RELEASE_LOCK
};

// Standard virtual methods
void LuaInstance::OnLoad()
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[INSTANCE_EVENT_ONLOAD]);
    g_engine->PushUint(m_instanceId);
    g_engine->ExecuteCall(1);
    RELEASE_LOCK
};

void LuaInstance::Destroy()
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[INSTANCE_EVENT_DESTROY]);
    g_engine->PushUint(m_instanceId);
    g_engine->ExecuteCall(1);
    RELEASE_LOCK

    typedef HM_NAMESPACE::hash_map<uint32, LuaInstance*> IMAP;
    IMAP & iMap = g_luaMgr.getLuInstanceMap();
    for(IMAP::iterator itr = iMap.begin(); itr != iMap.end(); ++itr)
    {
        if(itr->second == this)
        {
            iMap.erase(itr);
            break;
        }
    }
    delete this;
};

void LuaGossip::GossipEnd(Object* pObject, Player* Plr)
{
    GET_LOCK
    if(pObject->GetTypeId() == TYPEID_UNIT)
    {
        if(m_unit_gossip_binding == NULL)
        {
            RELEASE_LOCK
            return;
        }

        g_engine->BeginCall(m_unit_gossip_binding->Functions[GOSSIP_EVENT_ON_END]);
        g_engine->PushUnit(pObject);
        g_engine->PushUint(GOSSIP_EVENT_ON_END);
        g_engine->PushUnit(Plr);
        g_engine->ExecuteCall(3);
    }
    else if(pObject->GetTypeId() == TYPEID_ITEM)
    {
        if(m_item_gossip_binding == NULL)
        {
            RELEASE_LOCK
            return;
        }

        g_engine->BeginCall(m_item_gossip_binding->Functions[GOSSIP_EVENT_ON_END]);
        g_engine->PushItem(pObject);
        g_engine->PushUint(GOSSIP_EVENT_ON_END);
        g_engine->PushUnit(Plr);
        g_engine->ExecuteCall(3);
    }
    else if(pObject->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        if(m_go_gossip_binding == NULL)
        {
            RELEASE_LOCK
            return;
        }

        g_engine->BeginCall(m_go_gossip_binding->Functions[GOSSIP_EVENT_ON_END]);
        g_engine->PushGo(pObject);
        g_engine->PushUint(GOSSIP_EVENT_ON_END);
        g_engine->PushUnit(Plr);
        g_engine->ExecuteCall(3);
    }
    RELEASE_LOCK
}

void LuaCreature::OnCombatStart(Unit* mTarget)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_ENTER_COMBAT]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_ENTER_COMBAT);
    g_engine->PushUnit(mTarget);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnCombatStop(Unit* mTarget)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_LEAVE_COMBAT]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_LEAVE_COMBAT);
    g_engine->PushUnit(mTarget);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnTargetDied(Unit* mTarget)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_KILLED_TARGET]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_KILLED_TARGET);
    g_engine->PushUnit(mTarget);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnDied(Unit *mKiller)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_DIED]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_DIED);
    g_engine->PushUnit(mKiller);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnTargetParried(Unit* mTarget)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_TARGET_PARRIED]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_TARGET_PARRIED);
    g_engine->PushUnit(mTarget);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnTargetDodged(Unit* mTarget)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_TARGET_DODGED]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_TARGET_DODGED);
    g_engine->PushUnit(mTarget);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnTargetBlocked(Unit* mTarget, int32 iAmount)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_TARGET_BLOCKED]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_TARGET_BLOCKED);
    g_engine->PushUnit(mTarget);
    g_engine->PushInt(iAmount);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaCreature::OnTargetCritHit(Unit* mTarget, int32 fAmount)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_TARGET_CRIT_HIT]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_TARGET_CRIT_HIT);
    g_engine->PushUnit(mTarget);
    g_engine->PushInt(fAmount);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaCreature::OnParried(Unit* mTarget)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_PARRY]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_PARRY);
    g_engine->PushUnit(mTarget);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnDodged(Unit* mTarget)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_DODGED]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_DODGED);
    g_engine->PushUnit(mTarget);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnBlocked(Unit* mTarget, int32 iAmount)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_BLOCKED]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_BLOCKED);
    g_engine->PushUnit(mTarget);
    g_engine->PushInt(iAmount);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaCreature::OnCritHit(Unit* mTarget, int32 fAmount)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_CRIT_HIT]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_CRIT_HIT);
    g_engine->PushUnit(mTarget);
    g_engine->PushInt(fAmount);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaCreature::OnHit(Unit* mTarget, float fAmount)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_HIT]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_HIT);
    g_engine->PushUnit(mTarget);
    g_engine->PushFloat(fAmount);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaCreature::OnAssistTargetDied(Unit* mAssistTarget)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_ASSIST_DIED]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_ASSIST_DIED);
    g_engine->PushUnit(mAssistTarget);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnFear(Unit* mFeared, uint32 iSpellId)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_FEAR]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_FEAR);
    g_engine->PushUnit(mFeared);
    g_engine->PushUint(iSpellId);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaCreature::OnFlee(Unit* mFlee)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_FLEE]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_FLEE);
    g_engine->PushUnit(mFlee);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnCallForHelp()
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_CALL_FOR_HELP]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_CALL_FOR_HELP);
    g_engine->ExecuteCall(2);
    RELEASE_LOCK
}

void LuaCreature::OnLoad()
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_SPAWN]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_SPAWN);
    g_engine->ExecuteCall(2);

    uint32 iid = _unit->GetInstanceID();
    if (_unit->GetMapMgr() == NULL || _unit->GetMapMgr()->GetMapInfo()->type == INSTANCE_NULL)
        iid = 0;

    g_luaMgr.OnLoadInfo.push_back(_unit->GetMapId());
    g_luaMgr.OnLoadInfo.push_back(iid);
    g_luaMgr.OnLoadInfo.push_back(GUID_LOPART(_unit->GetGUID()));
    RELEASE_LOCK
}

void LuaCreature::OnReachWP(uint32 iWaypointId, bool bForwards)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_REACH_WP]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_REACH_WP);
    g_engine->PushUint(iWaypointId);
    g_engine->PushBool(bForwards);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaCreature::OnLootTaken(Player* pPlayer, ItemPrototype *pItemPrototype)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_LOOT_TAKEN]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_LOOT_TAKEN);
    g_engine->PushUnit(pPlayer);
    g_engine->PushUint(pItemPrototype->ItemId);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaCreature::AIUpdate(MapManagerScript* Mgr, uint32 ptime)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_AIUPDATE]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_AIUPDATE);
    g_engine->PushUint(ptime);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaCreature::OnEmote(Player * pPlayer, EmoteType Emote)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_EMOTE]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_EMOTE);
    g_engine->PushUnit(pPlayer);
    g_engine->PushInt((int32)Emote);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaCreature::OnDamageTaken(Unit* mAttacker, float fAmount)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[CREATURE_EVENT_ON_DAMAGE_TAKEN]);
    g_engine->PushUnit(_unit);
    g_engine->PushInt(CREATURE_EVENT_ON_DAMAGE_TAKEN);
    g_engine->PushUnit(mAttacker);
    g_engine->PushUint(fAmount);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaCreature::StringFunctionCall(int fRef)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(fRef);
    g_engine->PushUnit(_unit);
    g_engine->ExecuteCall(1);
    RELEASE_LOCK
}

void LuaCreature::Destroy()
{
    {
        typedef std::multimap<uint32,LuaCreature*> CMAP;
        CMAP & cMap = g_luaMgr.getLuCreatureMap();
        CMAP::iterator itr = cMap.find(_unit->GetEntry());
        CMAP::iterator itend = cMap.upper_bound(_unit->GetEntry());
        CMAP::iterator it;
        for(;itr != cMap.end() && itr != itend;)
        {
            it = itr++;
            if(it->second != NULL && it->second == this)
                cMap.erase(it);
        }
    }

    {
        //Function Ref clean up
        std::map< uint64, std::set<int> > & objRefs = g_luaMgr.getObjectFunctionRefs();
        std::map< uint64, std::set<int> >::iterator itr = objRefs.find(_unit->GetGUID());
        if(itr != objRefs.end() )
        {
            std::set<int> & refs = itr->second;
            for(std::set<int>::iterator it = refs.begin(); it != refs.end(); ++it)
            {
                lua_unref(g_engine->GetMainLuaState(),(*it));
                sEventMgr.RemoveEvents(_unit,(*it)+EVENT_LUA_CREATURE_EVENTS);
            }
            refs.clear();
        }
        delete this;
    }
}

void LuaGameObject::OnCreate()
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_ON_CREATE]);
    g_engine->PushGo(_gameobject);
    g_engine->ExecuteCall(1);
    RELEASE_LOCK
}

void LuaGameObject::OnSpawn()
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_ON_SPAWN]);
    g_engine->PushGo(_gameobject);
    g_engine->ExecuteCall(1);
    RELEASE_LOCK
}

void LuaGameObject::OnDespawn()
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_ON_DESPAWN]);
    g_engine->PushGo(_gameobject);
    g_engine->ExecuteCall(1);
    RELEASE_LOCK
}

void LuaGameObject::OnLootTaken(Player * pLooter, ItemPrototype *pItemInfo)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_ON_LOOT_TAKEN]);
    g_engine->PushGo(_gameobject);
    g_engine->PushUint(GAMEOBJECT_EVENT_ON_LOOT_TAKEN);
    g_engine->PushUnit(pLooter);
    g_engine->PushUint(pItemInfo->ItemId);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

void LuaGameObject::OnActivate(Player * pPlayer)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_ON_USE]);
    g_engine->PushGo(_gameobject);
    g_engine->PushUint(GAMEOBJECT_EVENT_ON_USE);
    g_engine->PushUnit(pPlayer);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaGameObject::AIUpdate(MapManagerScript*, uint32 ptime)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_AIUPDATE]);
    g_engine->PushGo(_gameobject);
    g_engine->PushUint(ptime);
    g_engine->ExecuteCall(1);
    RELEASE_LOCK
}

void LuaGameObject::Destroy()
{
    CHECK_BINDING_ACQUIRELOCK
    typedef std::multimap<uint32,LuaGameObject*> GMAP;
    GMAP & gMap = g_luaMgr.getLuGameObjectMap();
    GMAP::iterator itr = gMap.find(_gameobject->GetEntry());
    GMAP::iterator itend = gMap.upper_bound(_gameobject->GetEntry());
    GMAP::iterator it;
    //uint64 guid = _gameobject->GetGUID(); Unused?
    for(; itr != itend;)
    {
        it = itr++;
        if(it->second != NULL && it->second == this)
            gMap.erase(it);
    }

    std::map< uint64,std::set<int> > & objRefs = g_luaMgr.getObjectFunctionRefs();
    std::map< uint64,std::set<int> >::iterator itr2 = objRefs.find(_gameobject->GetGUID());
    if(itr2 != objRefs.end() )
    {
        std::set<int> & refs = itr2->second;
        for(std::set<int>::iterator it = refs.begin(); it != refs.end(); ++it)
            lua_unref( g_engine->GetMainLuaState(), (*it) );
        refs.clear();
    }
    delete this;
    RELEASE_LOCK
}

void LuaQuest::OnQuestStart(Player* mTarget, QuestLogEntry *qLogEntry)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[QUEST_EVENT_ON_ACCEPT]);
    g_engine->PushUnit(mTarget);
    g_engine->PushUint(qLogEntry->GetQuest()->id);
    g_engine->ExecuteCall(2);
    RELEASE_LOCK
}

void LuaQuest::OnQuestComplete(Player* mTarget, QuestLogEntry *qLogEntry)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[QUEST_EVENT_ON_COMPLETE]);
    g_engine->PushUnit(mTarget);
    g_engine->PushUint(qLogEntry->GetQuest()->id);
    g_engine->ExecuteCall(2);
    RELEASE_LOCK
}

void LuaQuest::OnQuestCancel(Player* mTarget)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[QUEST_EVENT_ON_CANCEL]);
    g_engine->PushUnit(mTarget);
    g_engine->ExecuteCall(1);
    RELEASE_LOCK
}

void LuaQuest::OnGameObjectActivate(uint32 entry, Player* mTarget, QuestLogEntry *qLogEntry)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[QUEST_EVENT_GAMEOBJECT_ACTIVATE]);
    g_engine->PushUint(entry);
    g_engine->PushUnit(mTarget);
    g_engine->PushUint(qLogEntry->GetQuest()->id);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaQuest::OnCreatureKill(uint32 entry, Player* mTarget, QuestLogEntry *qLogEntry)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[QUEST_EVENT_ON_CREATURE_KILL]);
    g_engine->PushUint(entry);
    g_engine->PushUnit(mTarget);
    g_engine->PushUint(qLogEntry->GetQuest()->id);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaQuest::OnExploreArea(uint32 areaId, Player* mTarget, QuestLogEntry *qLogEntry)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[QUEST_EVENT_ON_EXPLORE_AREA]);
    g_engine->PushUint(areaId);
    g_engine->PushUnit(mTarget);
    g_engine->PushUint(qLogEntry->GetQuest()->id);
    g_engine->ExecuteCall(3);
    RELEASE_LOCK
}

void LuaQuest::OnPlayerItemPickup(uint32 itemId, uint32 totalCount, Player* mTarget, QuestLogEntry *qLogEntry)
{
    CHECK_BINDING_ACQUIRELOCK
    g_engine->BeginCall(m_binding->Functions[QUEST_EVENT_ON_PLAYER_ITEMPICKUP]);
    g_engine->PushUint(itemId);
    g_engine->PushUint(totalCount);
    g_engine->PushUnit(mTarget);
    g_engine->PushUint(qLogEntry->GetQuest()->id);
    g_engine->ExecuteCall(4);
    RELEASE_LOCK
}

MapManagerScript* CreateLuaInstance(MapMgr* pMapMgr)
{
    LuaInstance* pLua = NULL;
    uint32 id = pMapMgr->GetMapId();
    LuaInstanceBinding* pBinding = g_luaMgr.getInstanceBinding(id);
    if(pBinding != NULL)
    {
        typedef HM_NAMESPACE::hash_map<uint32, LuaInstance*> IMAP;
        IMAP & iMap = g_luaMgr.getLuInstanceMap();
        IMAP::iterator itr = iMap.find(id);
        if(itr != iMap.end())
        {
            if(itr->second == NULL)
                pLua = itr->second = new LuaInstance(pMapMgr);
            else
                pLua = itr->second;
        }
        else
        {
            pLua = new LuaInstance(pMapMgr);
            iMap.insert(make_pair(id, pLua));
        }
        pLua->m_binding = pBinding;
    }
    return pLua;
}

CreatureAIScript * CreateLuaCreature(Creature * src)
{
    LuaUnitBinding * pBinding = g_luaMgr.GetUnitBinding( src->GetEntry() );
    if( pBinding == NULL )
        return NULL;

    LuaCreature * pLua = new LuaCreature( src );
    pLua->m_binding = pBinding;
    return pLua;
}

GameObjectAIScript * CreateLuaGameObject(GameObject * src)
{
    LuaGameObjectBinding * pBinding = g_luaMgr.GetGameObjectBinding( src->GetEntry() );
    if( pBinding == NULL )
        return NULL;

    LuaGameObject * pLua = new LuaGameObject( src );
    pLua->m_binding = pBinding;
    return pLua;
}

QuestScript * CreateLuaQuestScript(uint32 id)
{
    LuaQuestBinding * pBinding = g_luaMgr.GetQuestBinding( id );
    if( pBinding == NULL )
        return NULL;

    LuaQuest * pLua = new LuaQuest();
    pLua->m_binding = pBinding;
    return pLua;
}

// Gossip stuff
GossipScript * CreateLuaUnitGossipScript(uint32 id)
{
    LuaUnitGossipBinding * pBinding = g_luaMgr.GetLuaUnitGossipBinding( id );
    if( pBinding == NULL )
        return NULL;
 
    LuaGossip * pLua = new LuaGossip();
    pLua->m_unit_gossip_binding = pBinding;
    return pLua;
}

GossipScript * CreateLuaItemGossipScript(uint32 id)
{
    LuaItemGossipBinding * pBinding = g_luaMgr.GetLuaItemGossipBinding( id );
    if( pBinding == NULL )
        return NULL;

    LuaGossip * pLua = new LuaGossip();
    pLua->m_item_gossip_binding = pBinding;
    return pLua;
}

GossipScript * CreateLuaGOGossipScript(uint32 id)
{
    LuaGOGossipBinding * pBinding = g_luaMgr.GetLuaGOGossipBinding( id );
    if( pBinding == NULL )
        return NULL;

    LuaGossip * pLua = new LuaGossip();
    pLua->m_go_gossip_binding = pBinding;
    return pLua;
}

//all of these run similarly, they execute OnServerHook for all the functions in their respective event's list.
bool Lua_HookOnNewCharacter(uint32 Race, uint32 Class, WorldSession* Session, const char* Name)
{
    GET_LOCK
    bool result = true;
    for(vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_NEW_CHARACTER].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_NEW_CHARACTER].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_NEW_CHARACTER);
        g_engine->PushString(Name);
        g_engine->PushUint(Race);
        g_engine->PushUint(Class);
        if(g_engine->ExecuteCall(4, 1))
        {
            lua_State* L = g_luaMgr.GLuas();
            if(!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;
            g_engine->EndCall(1);
        }
    }
    RELEASE_LOCK
    return result;
}

void Lua_HookOnKillPlayer(Player* pPlayer, Player* pVictim)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_KILL_PLAYER].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_KILL_PLAYER].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_KILL_PLAYER);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUnit(pVictim);
        g_engine->ExecuteCall(3);
    }
    RELEASE_LOCK
}

void Lua_HookOnFirstEnterWorld(Player* pPlayer)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD);
        g_engine->PushUnit(pPlayer);
        g_engine->ExecuteCall(2);
    }
    RELEASE_LOCK
}

void Lua_HookOnEnterWorld(Player* pPlayer)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_ENTER_WORLD].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_ENTER_WORLD].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_ENTER_WORLD);
        g_engine->PushUnit(pPlayer);
        g_engine->ExecuteCall(2);
    }
    RELEASE_LOCK
}

void Lua_HookOnGuildJoin(Player* pPlayer, GuildInfo* pGuild)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_GUILD_JOIN].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_GUILD_JOIN].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_GUILD_JOIN);
        g_engine->PushUnit(pPlayer);
        g_engine->PushString(pGuild->m_guildName.c_str());
        g_engine->ExecuteCall(3);
    }
    RELEASE_LOCK
}

void Lua_HookOnDeath(Player* pPlayer)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_DEATH].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_DEATH].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_DEATH);
        g_engine->PushUnit(pPlayer);
        g_engine->ExecuteCall(2);
    }
    RELEASE_LOCK
}

bool Lua_HookOnRepop(Player* pPlayer)
{
    GET_LOCK
    bool result = true;
    for(vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_REPOP].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_REPOP].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_REPOP);
        g_engine->PushUnit(pPlayer);
        if(g_engine->ExecuteCall(2, 1))
        {
            lua_State* L = g_luaMgr.GLuas();
            if(!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;
            g_engine->EndCall(1);
        }
    }
    RELEASE_LOCK
    return result;
}

void Lua_HookOnEmote(Player* pPlayer, uint32 Emote, Unit* pUnit)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_EMOTE].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_EMOTE].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_EMOTE);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUnit(pUnit);
        g_engine->PushUint(Emote);
        g_engine->ExecuteCall(4);
    }
    RELEASE_LOCK
}

void Lua_HookOnEnterCombat(Player* pPlayer, Unit* pTarget)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_ENTER_COMBAT].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_ENTER_COMBAT].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_ENTER_COMBAT);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUnit(pTarget);
        g_engine->ExecuteCall(3);
    }
    RELEASE_LOCK
}

bool Lua_HookOnCastSpell(Player* pPlayer, SpellEntry* pSpell, Spell* spell)
{
    GET_LOCK
    bool result = true;
    for(vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_CAST_SPELL].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_CAST_SPELL].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_CAST_SPELL);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUint(pSpell->Id);
        g_engine->PushSpell(spell);
        if(g_engine->ExecuteCall(4, 1))
        {
            lua_State* L = g_luaMgr.GLuas();
            if(!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;
            g_engine->EndCall(1);
        }
    }
    RELEASE_LOCK
    return result;
}

void Lua_HookOnTick()
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_TICK].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_TICK].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->ExecuteCall();
    }
    RELEASE_LOCK
}

bool Lua_HookOnLogoutRequest(Player* pPlayer)
{
    GET_LOCK
    bool result = true;
    for(vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST);
        g_engine->PushUnit(pPlayer);
        if(g_engine->ExecuteCall(2, 1))
        {
            lua_State* L = g_luaMgr.GLuas();
            if(!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;
            g_engine->EndCall(1);
        }
    }
    RELEASE_LOCK
    return result;
}

void Lua_HookOnLogout(Player* pPlayer)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_LOGOUT].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_LOGOUT].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_LOGOUT);
        g_engine->PushUnit(pPlayer);
        g_engine->ExecuteCall(2);
    }
    RELEASE_LOCK
}

void Lua_HookOnQuestAccept(Player* pPlayer, Quest* pQuest, Object* pQuestGiver)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_QUEST_ACCEPT].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_QUEST_ACCEPT].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_QUEST_ACCEPT);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUint(pQuest->id);
        if(!pQuestGiver)
            g_engine->PushNil();
        else if(pQuestGiver->IsUnit())
            g_engine->PushUnit(pQuestGiver);
        else if(pQuestGiver->IsGameObject())
            g_engine->PushGo(pQuestGiver);
        else if(pQuestGiver->IsItem())
            g_engine->PushItem(pQuestGiver);
        else
            g_engine->PushNil();
        g_engine->ExecuteCall(4);
    }
    RELEASE_LOCK
}

void Lua_HookOnZone(Player* pPlayer, uint32 Zone, uint32 oldZone)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_ZONE].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_ZONE].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_ZONE);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUint(Zone);
        g_engine->PushUint(oldZone);
        g_engine->ExecuteCall(4);
    }
    RELEASE_LOCK
}

bool Lua_HookOnChat(Player* pPlayer, uint32 Type, uint32 Lang, const char* Message, const char* Misc)
{
    GET_LOCK
    bool result = true;
    for(vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_CHAT].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_CHAT].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_CHAT);
        g_engine->PushUnit(pPlayer);
        g_engine->PushString(Message);
        g_engine->PushUint(Type);
        g_engine->PushUint(Lang);
        g_engine->PushString(Misc);
        if(g_engine->ExecuteCall(6, 1))
        {
            lua_State* L = g_luaMgr.GLuas();
            if(!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;
            g_engine->EndCall(1);
        }
    }
    RELEASE_LOCK
    return result;
}

void Lua_HookOnLoot(Player* pPlayer, Unit* pTarget, uint32 Money, uint32 ItemId)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_LOOT].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_LOOT].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_LOOT);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUnit(pTarget);
        g_engine->PushUint(Money);
        g_engine->PushUint(ItemId);
        g_engine->ExecuteCall(5);
    }
    RELEASE_LOCK
}

void Lua_HookOnGuildCreate(Player* pLeader, GuildInfo* pGuild)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_GUILD_CREATE].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_GUILD_CREATE].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_GUILD_CREATE);
        g_engine->PushUnit(pLeader);
        g_engine->PushString(pGuild->m_guildName.c_str());
        g_engine->ExecuteCall(3);
    }
    RELEASE_LOCK
}

void Lua_HookOnEnterWorld2(Player* pPlayer)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_FULL_LOGIN].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_FULL_LOGIN].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_FULL_LOGIN);
        g_engine->PushUnit(pPlayer);
        g_engine->ExecuteCall(2);
    }
    RELEASE_LOCK
}

void Lua_HookOnCharacterCreate(Player* pPlayer)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_CHARACTER_CREATE].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_CHARACTER_CREATE].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_CHARACTER_CREATE);
        g_engine->PushUnit(pPlayer);
        g_engine->ExecuteCall(2);
    }
    RELEASE_LOCK
}

void Lua_HookOnQuestCancelled(Player* pPlayer, Quest* pQuest)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_QUEST_CANCELLED].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_QUEST_CANCELLED].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_QUEST_CANCELLED);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUint(pQuest->id);
        g_engine->ExecuteCall(3);
    }
    RELEASE_LOCK
}

void Lua_HookOnQuestFinished(Player* pPlayer, Quest* pQuest, Object* pQuestGiver)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_QUEST_FINISHED].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_QUEST_FINISHED].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_QUEST_FINISHED);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUint(pQuest->id);
        if(!pQuestGiver)
            g_engine->PushNil();
        else if(pQuestGiver->IsUnit())
            g_engine->PushUnit(pQuestGiver);
        else if(pQuestGiver->IsGameObject())
            g_engine->PushGo(pQuestGiver);
        else if(pQuestGiver->IsItem())
            g_engine->PushItem(pQuestGiver);
        else
            g_engine->PushNil();
        g_engine->ExecuteCall(4);
    }
    RELEASE_LOCK
}

void Lua_HookOnHonorableKill(Player* pPlayer, Player* pKilled)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_HONORABLE_KILL].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_HONORABLE_KILL].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_HONORABLE_KILL);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUnit(pKilled);
        g_engine->ExecuteCall(3);
    }
    RELEASE_LOCK
}

void Lua_HookOnArenaFinish(Player* pPlayer, ArenaTeam* pTeam, bool victory, bool rated)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_ARENA_FINISH].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_ARENA_FINISH].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_ARENA_FINISH);
        g_engine->PushUnit(pPlayer);
        g_engine->PushString(pTeam->m_name.c_str());
        g_engine->PushBool(victory);
        g_engine->PushBool(rated);
        g_engine->ExecuteCall(5);
    }
    RELEASE_LOCK
}

void Lua_HookOnObjectLoot(Player* pPlayer, Object* pTarget, uint32 Money, uint32 ItemId)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_OBJECTLOOT].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_OBJECTLOOT].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_OBJECTLOOT);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUnit(pTarget);
        g_engine->PushUint(Money);
        g_engine->PushUint(ItemId);
        g_engine->ExecuteCall(5);
    }
    RELEASE_LOCK
}

void Lua_HookOnAreaTrigger(Player* pPlayer, uint32 areaTrigger)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_AREATRIGGER].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_AREATRIGGER].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_AREATRIGGER);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUint(areaTrigger);
        g_engine->ExecuteCall(3);
    }
    RELEASE_LOCK
}

void Lua_HookOnPostLevelUp(Player* pPlayer)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_POST_LEVELUP].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_POST_LEVELUP].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_POST_LEVELUP);
        g_engine->PushUnit(pPlayer);
        g_engine->ExecuteCall(2);
    }
    RELEASE_LOCK
}

bool Lua_HookOnPreUnitDie(Unit* Killer, Unit* Victim)
{
    GET_LOCK
    bool result = true;
    for(vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_PRE_DIE].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_PRE_DIE].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_PRE_DIE);
        g_engine->PushUnit(Killer);
        g_engine->PushUnit(Victim);
        if(g_engine->ExecuteCall(3, 1))
        {
            lua_State* L = g_luaMgr.GLuas();
            if(!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;
            g_engine->EndCall(1);
        }
    }
    RELEASE_LOCK
    return result;
}

void Lua_HookOnAdvanceSkillLine(Player* pPlayer, uint32 SkillLine, uint32 Current)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE);
        g_engine->PushUnit(pPlayer);
        g_engine->PushUint(SkillLine);
        g_engine->PushUint(Current);
        g_engine->ExecuteCall(4);
    }
    RELEASE_LOCK
}

void Lua_HookOnDuelFinished(Player* pWinner, Player* pLoser)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_DUEL_FINISHED].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_DUEL_FINISHED].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_DUEL_FINISHED);
        g_engine->PushUnit(pWinner);
        g_engine->PushUnit(pLoser);
        g_engine->ExecuteCall(3);
    }
    RELEASE_LOCK
}

void Lua_HookOnAuraRemove(Aura* aura)
{
    GET_LOCK
    for(std::vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_AURA_REMOVE].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_AURA_REMOVE].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_AURA_REMOVE);
        g_engine->PushAura(aura);
        g_engine->ExecuteCall(2);
    }
    RELEASE_LOCK
}

bool Lua_HookOnResurrect(Player* pPlayer)
{
    GET_LOCK
    bool result = true;
    for(vector<uint16>::iterator itr = g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_RESURRECT].begin(); itr != g_luaMgr.EventAsToFuncName[SERVER_HOOK_EVENT_ON_RESURRECT].end(); ++itr)
    {
        g_engine->BeginCall((*itr));
        g_engine->PushInt(SERVER_HOOK_EVENT_ON_RESURRECT);
        g_engine->PushUnit(pPlayer);
        if(g_engine->ExecuteCall(2, 1))
        {
            lua_State* L = g_luaMgr.GLuas();
            if(!lua_isnoneornil(L, 1) && !lua_toboolean(L, 1))
                result = false;
            g_engine->EndCall(1);
        }
    }
    RELEASE_LOCK
    return result;
}

bool Lua_HookOnDummySpell(uint32 effectIndex, Spell* pSpell)
{
    GET_LOCK
    g_engine->BeginCall(g_luaMgr.m_luaDummySpells[pSpell->GetSpellProto()->Id]);
    g_engine->PushUint(effectIndex);
    g_engine->PushSpell(pSpell);
    g_engine->ExecuteCall(2);
    RELEASE_LOCK
    return true;
}
