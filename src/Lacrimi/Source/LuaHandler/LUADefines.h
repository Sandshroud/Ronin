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

#ifndef __LUA_DEFINES_H
#define __LUA_DEFINES_H

//#define PRINTERRORS

#define ENGINE_NAME "LacrimiLua"
#define ENGINE_VERSION 3.0f

#ifdef DEBUG
#define LUA_USE_APICHECK
#endif

extern "C" {
 #include <lualib/lua.h>
 #include <lualib/lauxlib.h>
 #include <lualib/lualib.h>
}

class LuaEngine;
class LuaEngineMgr;
class LuaCreature;
class LuaGameObject;
class LuaQuest;
class LuaInstance;
class LuaGossip;

/** Quest Events
 */
enum QuestEvents
{
    QUEST_EVENT_ON_ACCEPT               = 1,
    QUEST_EVENT_ON_COMPLETE             = 2,
    QUEST_EVENT_ON_CANCEL               = 3,
    QUEST_EVENT_GAMEOBJECT_ACTIVATE     = 4,
    QUEST_EVENT_ON_CREATURE_KILL        = 5,
    QUEST_EVENT_ON_EXPLORE_AREA         = 6,
    QUEST_EVENT_ON_PLAYER_ITEMPICKUP    = 7,
    QUEST_EVENT_COUNT                   = 8
};

/** Gossip Events
*/
enum GossipEvents
{
    GOSSIP_EVENT_ON_TALK                = 1,
    GOSSIP_EVENT_ON_SELECT_OPTION       = 2,
    GOSSIP_EVENT_ON_END                 = 3,
    GOSSIP_EVENT_COUNT                  = 4
};

/** Creature Events
 */
enum CreatureEvents
{
    CREATURE_EVENT_ON_ENTER_COMBAT      = 1,
    CREATURE_EVENT_ON_LEAVE_COMBAT      = 2,
    CREATURE_EVENT_ON_KILLED_TARGET     = 3,
    CREATURE_EVENT_ON_DIED              = 4,
    CREATURE_EVENT_ON_TARGET_PARRIED    = 5,
    CREATURE_EVENT_ON_TARGET_DODGED     = 6,
    CREATURE_EVENT_ON_TARGET_BLOCKED    = 7,
    CREATURE_EVENT_ON_TARGET_CRIT_HIT   = 8,
    CREATURE_EVENT_ON_PARRY             = 9,
    CREATURE_EVENT_ON_DODGED            = 10,
    CREATURE_EVENT_ON_BLOCKED           = 11,
    CREATURE_EVENT_ON_CRIT_HIT          = 12,
    CREATURE_EVENT_ON_HIT               = 13,
    CREATURE_EVENT_ON_ASSIST_DIED       = 14,
    CREATURE_EVENT_ON_FEAR              = 15,
    CREATURE_EVENT_ON_FLEE              = 16,
    CREATURE_EVENT_ON_CALL_FOR_HELP     = 17,
    CREATURE_EVENT_ON_LOAD              = 18,
    CREATURE_EVENT_ON_REACH_WP          = 19,
    CREATURE_EVENT_ON_LOOT_TAKEN        = 20,
    CREATURE_EVENT_ON_AIUPDATE          = 21,
    CREATURE_EVENT_ON_EMOTE             = 22,
    CREATURE_EVENT_ON_DAMAGE_TAKEN      = 23,
    CREATURE_EVENT_AI_TICK              = 24,
    CREATURE_EVENT_ON_SPAWN             = 25,
    CREATURE_EVENT_ON_GOSSIP_TALK       = 26,
    CREATURE_EVENT_ON_LEAVE_LIMBO       = 27,
    CREATURE_EVENT_PLAYER_ENTERS_RANGE  = 28,
    CREATURE_EVENT_COUNT                = 29
};

/** GameObject Events
 */
enum GameObjectEvents
{
    GAMEOBJECT_EVENT_ON_CREATE          = 1,
    GAMEOBJECT_EVENT_ON_SPAWN           = 2,
    GAMEOBJECT_EVENT_ON_LOOT_TAKEN      = 3,
    GAMEOBJECT_EVENT_ON_USE             = 4,
    GAMEOBJECT_EVENT_AIUPDATE           = 5,
    GAMEOBJECT_EVENT_ON_DESPAWN         = 6,
    GAMEOBJECT_EVENT_COUNT              = 7
};

/** Instance Events
*/
enum InstanceEvents
{
    INSTANCE_EVENT_ON_PLAYER_DEATH      = 1,
    INSTANCE_EVENT_ON_PLAYER_ENTER      = 2,
    INSTANCE_EVENT_ON_AREA_TRIGGER      = 3,
    INSTANCE_EVENT_ON_ZONE_CHANGE       = 4,
    INSTANCE_EVENT_ON_CREATURE_DEATH    = 5,
    INSTANCE_EVENT_ON_CREATURE_PUSH     = 6,
    INSTANCE_EVENT_ON_GO_ACTIVATE       = 7,
    INSTANCE_EVENT_ON_GO_PUSH           = 8,
    INSTANCE_EVENT_ONLOAD               = 9,
    INSTANCE_EVENT_DESTROY              = 10,
    INSTANCE_EVENT_COUNT
};

enum RandomFlags
{
    RANDOM_ANY                          = 0,
    RANDOM_IN_SHORTRANGE                = 1,
    RANDOM_IN_MIDRANGE                  = 2,
    RANDOM_IN_LONGRANGE                 = 3,
    RANDOM_WITH_MANA                    = 4,
    RANDOM_WITH_RAGE                    = 5,
    RANDOM_WITH_ENERGY                  = 6,
    RANDOM_NOT_MAINTANK                 = 7
};

//reg type defines
#define REGTYPE_UNIT (1 << 0)
#define REGTYPE_GO (1 << 1)
#define REGTYPE_QUEST (1 << 2)
#define REGTYPE_SERVHOOK (1 << 3)
#define REGTYPE_ITEM (1 << 4)
#define REGTYPE_GOSSIP (1 << 5)
#define REGTYPE_DUMMYSPELL (1 << 6)
#define REGTYPE_INSTANCE (1 << 7)
#define REGTYPE_UNIT_GOSSIP (REGTYPE_UNIT | REGTYPE_GOSSIP)
#define REGTYPE_GO_GOSSIP (REGTYPE_GO | REGTYPE_GOSSIP)
#define REGTYPE_ITEM_GOSSIP (REGTYPE_ITEM | REGTYPE_GOSSIP)

enum CustomLuaEvenTypes
{
    LUA_EVENT_START = NUM_EVENT_TYPES, // Used as a placeholder
    EVENT_LUA_TIMED,
    EVENT_LUA_CREATURE_EVENTS,
    EVENT_LUA_GAMEOBJ_EVENTS,
    LUA_EVENTS_END
};

struct LUALoadScripts { set<string> luaFiles; };
struct EventInfoHolder { const char * funcName; TimedEvent * te; };
struct LuaUnitBinding { uint16 Functions[CREATURE_EVENT_COUNT]; };
struct LuaGameObjectBinding { uint16 Functions[GAMEOBJECT_EVENT_COUNT]; };
struct LuaQuestBinding { uint16 Functions[QUEST_EVENT_COUNT]; };
struct LuaInstanceBinding { uint16 Functions[INSTANCE_EVENT_COUNT]; };
struct LuaUnitGossipBinding { uint16 Functions[GOSSIP_EVENT_COUNT]; };
struct LuaItemGossipBinding { uint16 Functions[GOSSIP_EVENT_COUNT]; };
struct LuaGOGossipBinding { uint16 Functions[GOSSIP_EVENT_COUNT]; };

template<typename T> const char * GetTClassName() { return "UNKNOWN"; };
template<typename T> struct RegType { const char * name; int(*mfunc)(lua_State*,T*); };
template<typename T> RegType<T>* GetMethodTable() { return NULL; }
template<> const char * GetTClassName<Item>();
template<> const char * GetTClassName<Unit>();
template<> const char * GetTClassName<GameObject>();
template<> const char * GetTClassName<WorldPacket>();
template<> const char * GetTClassName<TaxiPath>();
template<> const char * GetTClassName<Spell>();
template<> const char * GetTClassName<Field>();
template<> const char * GetTClassName<QueryResult>();
template<> const char * GetTClassName<Aura>();

/************************************************************************/
/* SCRIPT FUNCTION IMPLEMENTATION                                       */
/************************************************************************/
#define CHECK_TYPEID(expected_type) if(!ptr || !ptr->IsInWorld() || ptr->GetTypeId() != expected_type) { return 0; }
#define CHECK_TYPEID_RET(expected_type) if(!ptr || !ptr->IsInWorld() || ptr->GetTypeId() != expected_type) { lua_pushboolean(L,0); return 0; }
#define CHECK_TYPEID_RET_INT(expected_type) if(!ptr || !ptr->IsInWorld() || ptr->GetTypeId() != expected_type) { lua_pushinteger(L,0); return 0; }

#define TEST_ITEM() if(ptr == NULL || !ptr->IsInWorld() || (ptr->GetProto() == NULL) || (ptr->GetTypeId() != TYPEID_ITEM && ptr->GetTypeId() != TYPEID_CONTAINER)) { return 0; }
#define TEST_ITEM_RET() if(ptr == NULL || !ptr->IsInWorld() || (ptr->GetProto() == NULL) || (ptr->GetTypeId() != TYPEID_ITEM && ptr->GetTypeId() != TYPEID_CONTAINER)) { lua_pushboolean(L,0); return 0; }
#define TEST_ITEM_RET_NULL() if(ptr == NULL || !ptr->IsInWorld() || (ptr->GetProto() == NULL) || (ptr->GetTypeId() != TYPEID_ITEM && ptr->GetTypeId() != TYPEID_CONTAINER)) { lua_pushnil(L); return 0; }

#define TEST_UNIT() if(ptr == NULL || !ptr->IsInWorld() || ptr->GetTypeId() != TYPEID_UNIT) { return 0; }
#define TEST_UNIT_RET() if(ptr == NULL || !ptr->IsInWorld() || ptr->GetTypeId() != TYPEID_UNIT) { lua_pushboolean(L,0); return 0; }
#define TEST_UNIT_RET_NULL() if(ptr == NULL || !ptr->IsInWorld() || ptr->GetTypeId() != TYPEID_UNIT) { lua_pushnil(L); return 0; }

#define TEST_PLAYER() if(ptr == NULL || !ptr->IsInWorld() || ptr->GetTypeId() != TYPEID_PLAYER) { return 0; }
#define TEST_PLAYER_RET() if(ptr == NULL || !ptr->IsInWorld() || ptr->GetTypeId() != TYPEID_PLAYER) { lua_pushboolean(L,0); return 0; }
#define TEST_PLAYER_RET_NULL() if(ptr == NULL || !ptr->IsInWorld() || ptr->GetTypeId() != TYPEID_PLAYER) { lua_pushnil(L); return 0; }

#define TEST_UNITPLAYER() if(ptr == NULL || !ptr->IsInWorld() || ( ptr->GetTypeId() != TYPEID_PLAYER && ptr->GetTypeId() != TYPEID_UNIT)) { return 0; }
#define TEST_UNITPLAYER_RET() if(ptr == NULL || !ptr->IsInWorld() || ( ptr->GetTypeId() != TYPEID_PLAYER && ptr->GetTypeId() != TYPEID_UNIT)) { lua_pushboolean(L,0); return 0; }
#define TEST_UNITPLAYER_RET_NULL() if(ptr == NULL || !ptr->IsInWorld() || ( ptr->GetTypeId() != TYPEID_PLAYER && ptr->GetTypeId() != TYPEID_UNIT)) { lua_pushnil(L); return 0; }

#define TEST_GO() if(ptr == NULL || !ptr->IsInWorld() || ptr->GetTypeId() != TYPEID_GAMEOBJECT || !ptr->GetInfo()) { return 0; }
#define TEST_GO_RET() if(ptr == NULL || !ptr->IsInWorld() || ptr->GetTypeId() != TYPEID_GAMEOBJECT) { lua_pushboolean(L,0); return 0; }
#define TEST_GO_RET_NULL() if(ptr == NULL || !ptr->IsInWorld() || ptr->GetTypeId() != TYPEID_GAMEOBJECT) { lua_pushnil(L); return 0; }

#define TEST_AURA() if(aura == NULL) { return 0; }
#define TEST_AURA_RET() if(aura == NULL) { lua_pushboolean(L,0); return 0; }
#define TEST_AURA_RET_NULL() if(aura == NULL) { lua_pushnil(L); return 0; }

#define RET_NIL(push) { if(push) lua_pushnil(L); return 0; }
#define RET_BOOL(exp) { lua_pushboolean(L, (exp ? 1 : 0)); return (exp ? 1 : 0); }
#define RET_STRING(str) { lua_pushstring(L,(str)); return 1; }
#define RET_NUMBER(number) { lua_pushnumber(L,(number)); return 1; }
#define RET_INT(integer) { lua_pushinteger(L,(integer)); return 1; }

#define CHECK_ITEM(L,narg) Lunar<Item>::check(L,narg)
#define CHECK_UNIT(L,narg) Lunar<Unit>::check(L,narg)
#define CHECK_PLAYER(L,narg) Lunar<Player>::check(L,narg)
#define CHECK_GO(L,narg) Lunar<GameObject>::check(L,narg)
#define CHECK_PACKET(L,narg) Lunar<WorldPacket>::check(L,narg)
#define CHECK_OBJECT(L,narg) Lunar<Object>::check(L,narg)

#define CHECK_FLOAT(L,narg) (lua_isnoneornil(L,(narg)) ) ? 0.00f : (float)luaL_checknumber(L,(narg)); 
#define CHECK_ULONG(L,narg) (uint32)luaL_checknumber((L),(narg))
#define CHECK_USHORT(L, narg) (uint16)luaL_checkinteger((L),(narg))
#define CHECK_BOOL(L,narg) (lua_toboolean((L),(narg)) > 0) ? true : false
#define CREATE_L_PTR lua_State* L = GLuas();

#define GET_LOCK g_luaMgr.Lock();
#define RELEASE_LOCK g_luaMgr.Unlock();
#define CHECK_BINDING_ACQUIRELOCK GET_LOCK if(m_binding == NULL) { RELEASE_LOCK return; }

#endif // __LUA_DEFINES_H
