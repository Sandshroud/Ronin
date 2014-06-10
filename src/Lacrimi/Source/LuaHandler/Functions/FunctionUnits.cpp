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

////////////////////////////////////////////////////
//////////////UNIT GOSSIP///////////////////////////
////////////////////////////////////////////////////
int LuaUnit_GossipCreateMenu(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int text_id = luaL_checkint(L, 1);
    Unit* target = Lunar<Unit>::check(L, 2);
    if(target == NULL || !target->IsPlayer())
        return 0;

    Player* plr = TO_PLAYER(target);
    int autosend = luaL_checkint(L, 3);
    objmgr.CreateGossipMenuForPlayer(&g_luaMgr.Menu, ptr->GetGUID(), text_id, plr);
    if(autosend)
        g_luaMgr.Menu->SendTo(plr);
    return 1;
}

int LuaUnit_GossipMenuAddItem(lua_State * L, Unit * ptr)
{
    if(g_luaMgr.Menu == NULL)
    {
        printf("Menu used while uninitialized!!!");
        return 0;
    }

    int icon = luaL_checkint(L, 1);
    const char * menu_text = luaL_checkstring(L, 2);
    int IntId = luaL_checkint(L, 3);

    g_luaMgr.Menu->AddItem(icon, menu_text, IntId);
    return 1;
}

int LuaUnit_GossipSendMenu(lua_State * L, Unit * ptr)
{
    if(g_luaMgr.Menu == NULL)
    {
        printf("Menu used while uninitialized!!!");
        return 0;
    }

    Unit* target = Lunar<Unit>::check(L, 1);
    if(target == NULL || !target->IsPlayer())
        RET_NIL(false);

    Player * plr = TO_PLAYER(target);
    g_luaMgr.Menu->SendTo(plr);
    return 1;
}

int LuaUnit_GossipSendPOI(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit* target = Lunar<Unit>::check(L, 1);
    if(!target->IsPlayer())
        RET_NIL(false);

    Player * plr = TO_PLAYER(target);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    int icon = luaL_checkint(L, 4);
    int flags = luaL_checkint(L, 5);
    int data = luaL_checkint(L, 6);
    const char * name = luaL_checkstring(L, 7);

    plr->Gossip_SendPOI(x, y, icon, flags, data, name);
    return 1;
}

int LuaUnit_GossipComplete(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    plr->Gossip_Complete();
    return 1;
}

int LuaUnit_IsPlayer(lua_State * L, Unit * ptr)
{
    TEST_PLAYER_RET();
    lua_pushboolean(L, 1);
    return 1;
}

int LuaUnit_IsCreature(lua_State * L, Unit * ptr)
{
    TEST_UNIT_RET();
    lua_pushboolean(L, 1);
    return 1;
}

int LuaUnit_Emote(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();

    uint32 emote_id = luaL_checkint(L, 1);
    uint32 time = luaL_checkint(L, 2);
    if(emote_id == 0)
        RET_NIL(false);

    if (time)
        ptr->EventAddEmote((EmoteType)emote_id,time);
    else
        ptr->Emote((EmoteType)emote_id);
    return 1;
}

int LuaUnit_GetManaPct(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET_NULL();
    if (ptr->GetPowerType() != POWER_TYPE_MANA)
        RET_NIL(true);

    lua_pushnumber(L, (int)(ptr->GetUInt32Value(UNIT_FIELD_POWER1) * 100.0f / ptr->GetUInt32Value(UNIT_FIELD_MAXPOWER1)));
    return 1;
}

int LuaUnit_GetName(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();

    switch(ptr->GetTypeId())
    {
    case TYPEID_UNIT:
        lua_pushstring(L, ((Creature*)ptr)->GetCreatureInfo() ? ((Creature*)ptr)->GetCreatureInfo()->Name : "Unknown");
        break;
    case TYPEID_PLAYER:
        lua_pushstring(L, ((Player*)ptr)->GetName());
        break;
    }

    return 1;
}

int LuaUnit_SendChatMessage(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();

    uint32 typ = luaL_checkint(L, 1);
    uint32 lang = luaL_checkint(L, 2);
    const char * message = luaL_checklstring(L, 3, NULL);
    if(message == NULL)
        RET_NIL(false);

    ptr->SendChatMessage(typ, lang, message);
    return 1;
}

int LuaUnit_MoveTo(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    double x = luaL_checknumber(L, 1);
    double y = luaL_checknumber(L, 2);
    double z = luaL_checknumber(L, 3);

    if(x == 0 || y == 0 || z == 0)
        RET_NIL(false);

    ptr->GetAIInterface()->MoveTo( (float)x, (float)y, (float)z);
    return 1;
}

int LuaUnit_SetMovementType(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint32 typ = luaL_checkint(L, 1);
    ptr->GetAIInterface()->setMoveType(typ);
    return 1;
}

int LuaUnit_GetX(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET_NULL();
    lua_pushnumber(L, ptr->GetPositionX());
    return 1;
}

int LuaUnit_GetY(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET_NULL();
    lua_pushnumber(L, ptr->GetPositionY());
    return 1;
}

int LuaUnit_GetZ(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET_NULL();
    lua_pushnumber(L, ptr->GetPositionZ());
    return 1;
}

int LuaUnit_GetO(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET_NULL();
    lua_pushnumber(L, ptr->GetOrientation());
    return 1;
}

int LuaUnit_CastSpell(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 sp = luaL_checkint(L, 1);
    if(sp == NULL)
        RET_NIL(false);

    ptr->CastSpell(ptr,dbcSpell.LookupEntry(sp),true);
    return 1;
}

int LuaUnit_FullCastSpell(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 sp = luaL_checkint(L, 1);
    if(sp == NULL)
        RET_NIL(false);

    ptr->CastSpell(ptr,dbcSpell.LookupEntry(sp),false);
    return 1;
}

int LuaUnit_CastSpellOnTarget(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 sp = luaL_checkint(L, 1);
    if(sp == NULL)
        RET_NIL(false);

    Unit * target = Lunar<Unit>::check(L, 2);
    if(target == NULL)
        RET_NIL(false);

    ptr->CastSpell(target,dbcSpell.LookupEntry(sp),true);
    return 1;
}

int LuaUnit_FullCastSpellOnTarget(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 sp = luaL_checkint(L, 1);
    if(sp == NULL)
        RET_NIL(false);

    Unit * target = Lunar<Unit>::check(L, 2);
    if(target == NULL)
        RET_NIL(false);

    ptr->CastSpell(target,dbcSpell.LookupEntry(sp),false);
    return 1;
}

int LuaUnit_SpawnCreature(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 entry_id = luaL_checkint(L, 1);
    float x = (float)luaL_checkint(L, 2);
    float y = (float)luaL_checkint(L, 3);
    float z = (float)luaL_checkint(L, 4);
    float o = (float)luaL_checkint(L, 5);
    uint32 faction = luaL_checkint(L, 6);
    uint32 duration = luaL_checkint(L, 7);

    if( !x || !y || !z || !entry_id || !faction /*|| !duration*/) //Shady: is it really required?
        RET_NIL(true);

    CreatureProto *p = CreatureProtoStorage.LookupEntry(entry_id);
    if(p == NULL)
        RET_NIL(true);

    Creature * pCreature = ptr->GetMapMgr()->GetMapScript()->SpawnCreature(entry_id,x,y,z,o);
    pCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,faction);
    pCreature->_setFaction();
    pCreature->SetInstanceID(ptr->GetInstanceID());
    pCreature->SetMapId(ptr->GetMapId());
    if(duration)
        pCreature->Despawn(duration,0);
    Lunar<Unit>::push(L,pCreature);
    return 1;
}

int LuaUnit_SpawnGameObject(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 entry_id = luaL_checkint(L, 1);
    if(entry_id == NULL)
        RET_NIL(true);

    float x = (float)luaL_checkint(L, 2);
    float y = (float)luaL_checkint(L, 3);
    float z = (float)luaL_checkint(L, 4);
    float o = (float)luaL_checkint(L, 5);
    uint32 duration = luaL_checkint(L, 6);

    GameObject* pC = ptr->GetMapMgr()->GetMapScript()->SpawnGameObject(entry_id,x,y,z,o);
    pC->SetInstanceID(ptr->GetInstanceID());
    pC->SetMapId(ptr->GetMapId());
    pC->Spawn(ptr->GetMapMgr());
    if(duration && duration > 0)
        pC->ExpireAndDelete(duration);
    Lunar<GameObject>::push(L,pC,false);
    return 1;
}

int LuaUnit_RegisterEvent(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    const char * typeName = luaL_typename(L,1);
    int delay = luaL_checkint(L,2);
    int repeats = luaL_checkint(L,3);
    if(delay < 1)
        RET_NIL(false);

    lua_settop(L,1);
    int functionRef = 0;
    if(!strcmp(typeName,"function") )
        functionRef = lua_ref(L,true);
    else if(!strcmp(typeName,"string"))
        functionRef = ExtractfRefFromCString(L,luaL_checkstring(L,1));

    if(functionRef)
    {
        Creature * creature = TO_CREATURE(ptr);
        sEventMgr.AddEvent(creature, &Creature::TriggerScriptEvent, functionRef, EVENT_LUA_CREATURE_EVENTS, delay, repeats, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        std::map< uint64,std::set<int> > & objRefs = g_luaMgr.getObjectFunctionRefs();
        std::map< uint64,std::set<int> >::iterator itr = objRefs.find(ptr->GetGUID());
        if(itr == objRefs.end() )
        {
            std::set<int> refs;
            refs.insert(functionRef);
            objRefs.insert(make_pair(ptr->GetGUID(),refs));
        }
        else
        {
            std::set<int> & refs = itr->second;
            refs.insert(functionRef);
        }
    }
    return 1;
}

/* This one just simply calls the function directly w/o any arguments, 
the trick to arguments is done Lua side through closures(function that
calls the wanted function  with the wanted arguments */
int LuaUnit_CreateLuaEvent(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();

    const char * typeName = luaL_typename(L,1);
    int delay = luaL_checkint(L,2);
    int repeats = luaL_checkint(L,3);
    if(delay < 1)
        return 0;

    lua_settop(L,1);
    int functionRef = 0;
    if(!strcmp(typeName,"function") )
        functionRef = lua_ref(L,true);
    else if(!strcmp(typeName,"string"))
        functionRef = ExtractfRefFromCString(L,luaL_checkstring(L,1));

    if(functionRef)
    {
        TimedEvent * ev = TimedEvent::Allocate(ptr,new CallbackP1<LuaEngineMgr,int>(&g_luaMgr,&LuaEngineMgr::CallFunctionByReference,functionRef),EVENT_LUA_CREATURE_EVENTS,delay,repeats);
        ptr->event_AddEvent(ev);
        std::map< uint64,std::set<int> > & objRefs = g_luaMgr.getObjectFunctionRefs();
        std::map< uint64,std::set<int> >::iterator itr = objRefs.find(ptr->GetGUID());
        if(itr == objRefs.end() )
        {
            std::set<int> refs;
            refs.insert(functionRef);
            objRefs.insert(make_pair(ptr->GetGUID(),refs));
        }
        else
        {
            std::set<int> & refs = itr->second;
            refs.insert(functionRef);
        }
    }
    return 1;
}

int LuaUnit_RemoveEvents(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    sEventMgr.RemoveEvents(ptr, EVENT_LUA_CREATURE_EVENTS);
    //Unref all contained references
    std::map< uint64,std::set<int> > & objRefs = g_luaMgr.getObjectFunctionRefs();
    std::map< uint64,std::set<int> >::iterator itr = objRefs.find(ptr->GetGUID());
    if(itr != objRefs.end() )
    {
        std::set<int> & refs = itr->second;
        for(std::set<int>::iterator it = refs.begin(); it != refs.end(); ++it)
            lua_unref(L,(*it));
        refs.clear();
    }
    return 1;
}

int LuaUnit_AddEventHolder(lua_State * L, Unit * ptr)
{
    EventableObjectHolder * holder = Lunar<EventableObjectHolder>::check(L,1);
    if(ptr && holder)
        sEventMgr.AddEventHolder(holder, ptr->GetInstanceID());
    return 1;
}

int LuaUnit_SetFaction(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    int faction = luaL_checkint(L,1);
    if(!faction)
        return 0;

    ptr->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,faction);
    ptr->_setFaction();
    return 1;
}
int LuaUnit_SetStandState(lua_State * L, Unit * ptr) //states 0..8
{
    TEST_UNITPLAYER();
    int state = luaL_checkint(L,1);
    if(state <= 0)
        return 0;

    ptr->SetStandState(state);
    return 1;
}
int LuaUnit_IsInCombat(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    if(ptr->CombatStatus.IsInCombat())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit_SetScale(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    double scale = luaL_checknumber(L,1);
    if(scale == 0.0)
        return 0;

    ptr->SetFloatValue(OBJECT_FIELD_SCALE_X, (float)scale);
    return 1;
}

int LuaUnit_SetModel(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    int modelid = luaL_checkint(L,1);
    ptr->SetUInt32Value(UNIT_FIELD_DISPLAYID,modelid);
    ptr->EventModelChange();
    return 1;
}

int LuaUnit_SetNPCFlags(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    int flags = luaL_checkint(L,1);
    if(!flags)
        return 0;

    ptr->SetUInt32Value(UNIT_NPC_FLAGS,flags);
    return 1;
}

int LuaUnit_SetCombatCapable(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int enabled = luaL_checkint(L,1);
    ptr->GetAIInterface()->disable_combat = (enabled > 0) ? true : false;
    return 1;
}

int LuaUnit_SetCombatMeleeCapable(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int enabled = luaL_checkint(L,1);
    ptr->GetAIInterface()->disable_melee = (enabled > 0) ? true : false;
    return 1;
}

int LuaUnit_SetCombatRangedCapable(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int enabled = luaL_checkint(L,1);
    ptr->GetAIInterface()->disable_ranged = (enabled > 0) ? true : false;
    return 1;
}

int LuaUnit_SetCombatSpellCapable(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int enabled = luaL_checkint(L,1);
    ptr->GetAIInterface()->disable_spell = (enabled > 0) ? true : false;
    return 1;
}

int LuaUnit_SetCombatTargetingCapable(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int enabled = luaL_checkint(L,1);
    ptr->GetAIInterface()->disable_targeting = (enabled > 0) ? true : false;
    return 1;
}

int LuaUnit_DestroyCustomWaypointMap(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    TO_CREATURE(ptr)->DestroyCustomWaypointMap();
    return 1;
}

int LuaUnit_CreateCustomWaypointMap(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Creature * pCreature = TO_CREATURE(ptr);
    if(pCreature->m_custom_waypoint_map)
    {
        for(WayPointMap::iterator itr = pCreature->m_custom_waypoint_map->begin(); itr != pCreature->m_custom_waypoint_map->end(); ++itr)
            delete (*itr);
        delete pCreature->m_custom_waypoint_map;
    }

    pCreature->m_custom_waypoint_map = new WayPointMap;
    pCreature->GetAIInterface()->SetWaypointMap(pCreature->m_custom_waypoint_map);
    return 1;
}

int LuaUnit_CreateCustomWaypoint(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Creature * crc = TO_CREATURE(ptr);
    uint32 id = CHECK_ULONG(L,1);
    float x = CHECK_FLOAT(L,2);
    float y = CHECK_FLOAT(L,3);
    float z = CHECK_FLOAT(L,4);
    float o = CHECK_FLOAT(L,5);
    uint32 waitime = CHECK_ULONG(L,6);
    uint32 flags = CHECK_ULONG(L,7);
    uint32 model = luaL_optint(L,8,0);
    WayPoint * wp = new WayPoint;
    wp->id = id;
    wp->x = x;
    wp->y = y;
    wp->z = z;
    wp->orientation = o;
    wp->waittime = waitime;
    wp->flags = flags;
    wp->forwardInfo = NULL;
    wp->backwardInfo = NULL;
    crc->GetAIInterface()->addWayPoint(wp);
    return 1;
}

int LuaUnit_DeleteAllWaypoints(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    TO_CREATURE(ptr)->GetAIInterface()->deleteAllWaypoints();
    return 1;
}

int LuaUnit_CreateWaypoint(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    float x = (float)luaL_checknumber(L,1);
    float y = (float)luaL_checknumber(L,2);
    float z = (float)luaL_checknumber(L,3);
    float o = (float)luaL_checknumber(L,4);
    int waittime = luaL_checkint(L,5);
    int flags = luaL_checkint(L,6);
    int modelid = luaL_checkint(L,7);

    Creature * pCreature = ((Creature*)ptr);
    if(!pCreature->m_custom_waypoint_map)
    {
        pCreature->m_custom_waypoint_map = new WayPointMap;
        pCreature->GetAIInterface()->SetWaypointMap(pCreature->m_custom_waypoint_map);
    }

    if(!modelid)
        modelid = pCreature->GetUInt32Value(UNIT_FIELD_DISPLAYID);

    WayPoint * wp = new WayPoint;
    wp->id = (uint32)pCreature->m_custom_waypoint_map->size() + 1;
    wp->x = x;
    wp->y = y;
    wp->z = z;
    wp->orientation = o;
    wp->flags = flags;
    wp->forwardInfo = NULL;
    wp->backwardInfo = NULL;
    wp->waittime = waittime;
    pCreature->m_custom_waypoint_map->push_back(wp);
    pCreature->GetAIInterface()->addWayPoint(wp);
    return 1;
}

int LuaUnit_MoveToWaypoint(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int id = luaL_checkint(L,1);
    if(id)
    {
        ptr->GetAIInterface()->setMoveType(MOVEMENTTYPE_WANTEDWP);
        ptr->GetAIInterface()->setWaypointToMove(id);
    }
    return 1;
}

int LuaUnit_RemoveItem(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    int id = luaL_checkint(L,1);
    int count = luaL_checkint(L,2);

    TO_PLAYER(ptr)->GetItemInterface()->RemoveItemAmt(id,count);
    return 1;
}

int LuaUnit_AddItem(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    int id = luaL_checkint(L,1);
    int count = luaL_checkint(L,2);

    Player * plr = TO_PLAYER(ptr);
    ItemPrototype * proto = ItemPrototypeStorage.LookupEntry(id);
    if(proto == NULL)
        return 0;

    Item * add = plr->GetItemInterface()->FindItemLessMax(id,count,false);
    if(add == NULL)
    {
        add = objmgr.CreateItem(id,plr);
        if(add)
        {
            add->SetUInt32Value(ITEM_FIELD_STACK_COUNT,count);
            if(plr->GetItemInterface()->AddItemToFreeSlot(add))
                plr->GetSession()->SendItemPushResult(add,false,true,false,true,plr->GetItemInterface()->LastSearchItemBagSlot(),plr->GetItemInterface()->LastSearchItemSlot(),count);
            else
            {
                add->DeleteMe();
                add = NULLITEM;
            }
        }
    }
    else
    {
        add->ModSignedInt32Value(ITEM_FIELD_STACK_COUNT,count);
        plr->GetSession()->SendItemPushResult(add,false,true,false,false,plr->GetItemInterface()->GetBagSlotByGuid(add->GetGUID()),0xFFFFFFFF,count);
    }

    return 1;
}

int LuaUnit_GetInstanceID(lua_State * L, Unit * ptr)
{
    if(ptr->GetMapMgr()->GetMapInfo()->type == INSTANCE_NULL)
        lua_pushnil(L);
    else
        lua_pushinteger(L,ptr->GetInstanceID());
    return 1;
}

int LuaUnit_GetClosestPlayer(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    float dist, d2;
    Player * ret=NULL;

    for(unordered_set<Player*>::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
    {
        d2=(*itr)->GetDistanceSq(ptr);
        if(!ret||d2<dist)
        {
            dist=d2;
            ret=*itr;
        }
    }

    if(ret==NULL)
        lua_pushnil(L);
    else
        Lunar<Unit>::push(L, ret ,false);

    return 1;
}

int LuaUnit_GetRandomPlayer(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int flag = luaL_checkint(L, 1);
    Player* ret = NULL;
    vector<Player*> players;
    switch(flag)
    {
    case RANDOM_ANY:
        {
            uint32 count = (uint32)ptr->GetInRangePlayersCount();
            uint32 r = RandomUInt(count - 1);
            count = 0;
            for(unordered_set< Player* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
            {
                if(count == r)
                {
                    ret = TO_PLAYER(*itr);
                    break;
                }
                ++count;
            }
        }
        break;
    case RANDOM_IN_SHORTRANGE:

        {
            for(unordered_set< Player* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
            {
                Player* obj = TO_PLAYER(*itr);
                if(obj && obj->CalcDistance(obj, ptr) <= 8)
                    players.push_back(obj);
            }
            if(players.size() == 1)
                ret = players[0];
            else if(players.size())
                ret = players[RandomUInt(uint32(players.size()) - 1)];
        }
        break;
    case RANDOM_IN_MIDRANGE:
        {
            for(unordered_set< Player* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
            {
                Player* obj = TO_PLAYER(*itr);
                float distance = obj->CalcDistance(obj, ptr);
                if(distance < 20 && distance > 8)
                    players.push_back(obj);
            }
            if(players.size() == 1)
                ret = players[0];
            else if(players.size())
                ret = players[RandomUInt(uint32(players.size()) - 1)];
        }
        break;
    case RANDOM_IN_LONGRANGE:
        {
            for(unordered_set< Player* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
            {
                Player* obj = TO_PLAYER(*itr);
                if(obj && obj->CalcDistance(obj, ptr) >= 20)
                    players.push_back(obj);
            }
            if(players.size() == 1)
                ret = players[0];
            else if(players.size())
                ret = players[RandomUInt(uint32(players.size()) - 1)];
        }
        break;
    case RANDOM_WITH_MANA:
        {
            for(unordered_set< Player* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
            {
                Player* obj = TO_PLAYER(*itr);
                if(obj && obj->GetPowerType() == POWER_TYPE_MANA)
                    players.push_back(obj);
            }
            if(players.size() == 1)
                ret = players[0];
            else if(players.size())
                ret = players[RandomUInt(uint32(players.size()) - 1)];
        }
        break;
    case RANDOM_WITH_ENERGY:
        {
            for(unordered_set< Player* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
            {
                Player* obj = TO_PLAYER(*itr);
                if(obj && obj->GetPowerType() == POWER_TYPE_ENERGY)
                    players.push_back(obj);
            }
            if(players.size() == 1)
                ret = players[0];
            else if(players.size())
                ret = players[RandomUInt(uint32(players.size()) - 1)];
        }
        break;
    case RANDOM_WITH_RAGE:
        {
            for(unordered_set< Player* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
            {
                Player* obj = TO_PLAYER(*itr);
                if(obj && obj->GetPowerType() == POWER_TYPE_RAGE)
                    players.push_back(obj);
            }
            if(players.size() == 1)
                ret = players[0];
            else if(players.size())
                ret = players[RandomUInt(uint32(players.size()) - 1)];
        }
        break;
    case RANDOM_NOT_MAINTANK:
        {
            Unit* mt = ptr->GetAIInterface()->GetMostHated();
            if(mt == NULL || !mt->IsPlayer())
                return 0;

            for(unordered_set< Player* >::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); ++itr)
            {
                Player* obj = TO_PLAYER(*itr);
                if(obj != mt)
                    players.push_back(obj);
            }
            if(players.size() == 1)
                ret = players[0];
            else if(players.size())
                ret = players[RandomUInt(uint32(players.size()) - 1)];
        }break;
    }

    if(ret == NULL)
        lua_pushnil(L);
    else
        Lunar<Unit>::push(L, ret);
    return 1;
}

int LuaUnit_GetRandomFriend(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Object* obj;
    Unit* ret = NULL;
    uint32 count = 0;

    for(unordered_set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
    {
        obj = (*itr);
        if (obj->IsUnit() && sFactionSystem.isFriendly(obj,ptr))
            ++count;
    }

    if (count)
    {
        uint32 r = RandomUInt(count-1);
        count = 0;
        for(unordered_set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            obj = (*itr);
            if (!obj->IsUnit() || !sFactionSystem.isFriendly(obj,ptr))
                continue;

            if(count == r)
            {
                ret = TO_UNIT(obj);
                break;
            }
            ++count;
        }
    }

    if(ret == NULL)
        lua_pushnil(L);
    else
        Lunar<Unit>::push(L, ret);
    return 1;
}

int LuaUnit_GetRandomEnemy(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Object* obj;
    Unit* ret = NULL;
    uint32 count = 0;

    for(unordered_set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
    {
        obj = (*itr);
        if (obj->IsUnit() && !sFactionSystem.isFriendly(obj,ptr))
            ++count;
    }

    if (count)
    {
        uint32 r = RandomUInt(count-1);
        count = 0;
        for(unordered_set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
        {
            obj = (*itr);
            if (!obj->IsUnit() || sFactionSystem.isFriendly(obj,ptr))
                continue;

            if(count == r)
            {
                ret = TO_UNIT(obj);
                break;
            }
            ++count;
        }
    }

    if(ret == NULL)
        lua_pushnil(L);
    else
        Lunar<Unit>::push(L, ret);
    return 1;
}

int LuaUnit_StopMovement(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_UNIT);
    int tim = luaL_checkint(L,1);
    ptr->GetAIInterface()->StopMovement(tim,true);
    return 1;
}

int LuaUnit_RemoveAura(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int auraid = luaL_checkint(L,1);
    ptr->RemoveAura(auraid);
    return 1;
}

int LuaUnit_PlaySoundToSet(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int soundid = luaL_checkint(L,1);
    ptr->PlaySoundToSet(soundid);
    return 1;
}

int LuaUnit_GetUnitBySqlId(lua_State * L, Unit * ptr)
{
    int sqlid = luaL_checkint(L,1);
    if(ptr || sqlid)
        Lunar<Unit>::push(L,ptr->GetMapMgr()->GetSqlIdCreature(sqlid));
    return 1;
}

int LuaUnit_Despawn(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_UNIT);
    int delay = luaL_checkint(L,1);
    int respawntime = luaL_checkint(L,2);
    ((Creature*)ptr)->Despawn(delay,respawntime);
    return 1;
}
int LuaUnit_GetInRangeFriends(lua_State * L, Unit * ptr)
{
    Object * pC = NULL;
    uint32 count = 0;
    lua_newtable(L);
    for( unordered_set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); itr++)
    {
        if( (*itr) ->IsUnit() )
        {
            count++,
            pC = *itr;
            lua_pushinteger(L,count);
            Lunar<Unit>::push(L,((Unit*)pC));
            lua_rawset(L,-3);
        }
    }
    return 1;
}

int LuaUnit_GetHealthPct(lua_State * L, Unit * ptr)
{
    if(!ptr)
        lua_pushinteger(L,0);
    else
        lua_pushinteger(L, ptr->GetHealthPct());

    return 1;
}
int LuaUnit_SetHealthPct(lua_State * L, Unit * ptr)
{
    int val = luaL_checkint(L,1);
    if (val>0)
        ptr->SetHealthPct(val);
    return 1;
}
int LuaUnit_GetItemCount(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET_INT(TYPEID_PLAYER);
    int itemid = luaL_checkint(L,1);
    lua_pushinteger(L, ((Player*)ptr)->GetItemInterface()->GetItemCount(itemid,false));
    return 1;
}

int LuaUnit_GetMainTank(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET_INT(TYPEID_UNIT);
    Unit* ret = ptr->GetAIInterface()->GetMostHated();
    if(ret==NULL)
        lua_pushnil(L);
    else
        Lunar<Unit>::push(L,(ret));
    return 1;
}
int LuaUnit_GetAddTank(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET_INT(TYPEID_UNIT);
    Unit* ret = ptr->GetAIInterface()->GetSecondHated();
    if(ret==NULL)
        lua_pushnil(L);
    else
        Lunar<Unit>::push(L,(ret));
    return 1;
}
int LuaUnit_ClearThreatList(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    if( ptr->GetAIInterface() )
        ptr->GetAIInterface()->ClearHateList();
    return 1;
}

int LuaUnit_GetTauntedBy(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    if (!ptr->GetAIInterface()->getTauntedBy())
        lua_pushnil(L);
    else
        Lunar<Unit>::push(L,ptr->GetAIInterface()->getTauntedBy(),false);
    return 1;
}
int LuaUnit_SetTauntedBy(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = Lunar<Unit>::check(L, 1);
    if (!target || ptr->GetAIInterface()->GetIsTaunted() || target==ptr)
        return 0;
    else
        ptr->GetAIInterface()->taunt(target);
    return 1;
}
int LuaUnit_ModThreat(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = Lunar<Unit>::check(L,1);
    int32 amount = luaL_checkint(L,2);
    if(ptr&&target&&amount)
        ptr->GetAIInterface()->modThreatByPtr(target,amount);
    return 1;
}
int LuaUnit_GetThreatByPtr(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = Lunar<Unit>::check(L,1);
    if(ptr&&target)
        lua_pushnumber(L,ptr->GetAIInterface()->getThreatByPtr(target));
    return 1;
}

int LuaUnit_ChangeTarget(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = Lunar<Unit>::check(L, 1);
    if (!target || !sFactionSystem.isHostile(ptr,target) || ptr==target)
        return 0;
    else
        ptr->GetAIInterface()->SetNextTarget(target);
    return 1;
}

int LuaUnit_HasFinishedQuest(lua_State * L, Unit * ptr)
{
    TEST_PLAYER_RET();
    int questid = luaL_checkint(L,1);
    if(((Player*)ptr)->HasFinishedQuest(questid))
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);

    return 1;
}

int LuaUnit_UnlearnSpell(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    int spellid = luaL_checkint(L,1);
    ((Player*)ptr)->removeSpell(spellid);
    return 1;
}

int LuaUnit_LearnSpell(lua_State * L, Unit* ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    int spellid = luaL_checkint(L,1);
    ((Player*)ptr)->addSpell(spellid);
    return 1;
}

int LuaUnit_MarkQuestObjectiveAsComplete(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);

    int questid = luaL_checkint(L,1);
    int objective = luaL_checkint(L,2);
    Player * pl = TO_PLAYER(ptr);
    QuestLogEntry * qle = pl->GetQuestLogForEntry(questid);
    if(qle == NULL || qle->GetQuest() == NULL)
        return 0;

    qle->SetMobCount(objective, qle->GetQuest()->required_mobcount[objective]);
    qle->SendUpdateAddKill(objective);
    if(qle->CanBeFinished())
        qle->SendQuestComplete();
    qle->UpdatePlayerFields();
    return 1;
}

int LuaUnit_KnockBack(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    double dx = luaL_checknumber(L,1);
    double dy = luaL_checknumber(L,2);
    double affect1 = luaL_checknumber(L,3);
    double affect2 = luaL_checknumber(L,4);
    WorldPacket data(SMSG_MOVE_KNOCK_BACK, 30);
    data << ptr->GetNewGUID();
    data << getMSTime();
    data << dx << dy << affect1 << affect2;
    ptr->SendMessageToSet(&data, true);

    return 1;
}

int LuaUnit_SendAreaTriggerMessage(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    const char * msg = luaL_checkstring(L,1);
    if(!msg)
        return 0;

    ((Player*)ptr)->SendAreaTriggerMessage(msg);
    return 1;
}

int LuaUnit_SendBroadcastMessage(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    const char * msg = luaL_checkstring(L,1);
    if(!msg)
        return 0;

    ((Player*)ptr)->BroadcastMessage(msg);
    return 1;
}

int LuaUnit_TeleportUnit(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 mapId = luaL_checkint(L, 1);
    float posX = (float)luaL_checknumber(L, 2);
    float posY = (float)luaL_checknumber(L, 3);
    float posZ = (float)luaL_checknumber(L, 4);
    if(!posX || !posY || !posZ)
        return 0;

    LocationVector vec(posX,posY,posZ);
    static_cast<Player*>( ptr ) ->SafeTeleport(mapId,0,vec);
    return 1;
}

int LuaUnit_GetHealth(lua_State * L, Unit * ptr)
{
    TEST_UNIT_RET();
    lua_pushinteger( L, ptr->GetUInt32Value( UNIT_FIELD_HEALTH ) );
    return 1;
}

int LuaUnit_GetMaxHealth(lua_State * L, Unit * ptr)
{
    if( ptr == NULL )
        lua_pushinteger( L, 0 );
    else
        lua_pushinteger( L, ptr->GetUInt32Value( UNIT_FIELD_MAXHEALTH ) );

    return 1;
}

int LuaUnit_SetHealth(lua_State * L, Unit * ptr)
{
    int val = luaL_checkint( L, 1 );
    if( ptr != NULL && val > 0 )
        if( (uint32)val > ptr->GetUInt32Value( UNIT_FIELD_MAXHEALTH ) )
            ptr->SetUInt32Value( UNIT_FIELD_HEALTH, ptr->GetUInt32Value( UNIT_FIELD_MAXHEALTH ) );
        else
            ptr->SetUInt32Value( UNIT_FIELD_HEALTH, val );
    return 1;
}

int LuaUnit_SetMaxHealth(lua_State * L, Unit * ptr)
{
    int val = luaL_checkint( L, 1 );
    if( ptr != NULL && val > 0 )
        if( (uint32)val < ptr->GetUInt32Value( UNIT_FIELD_HEALTH ) )
            ptr->SetUInt32Value( UNIT_FIELD_HEALTH, val );
        ptr->SetUInt32Value( UNIT_FIELD_MAXHEALTH, val );
    return 1;
}

int LuaUnit_WipeHateList(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET_INT(TYPEID_UNIT);
    if( ptr->GetAIInterface() )
        ptr->GetAIInterface()->WipeHateList();
    return 1;
}

int LuaUnit_WipeTargetList(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET_INT(TYPEID_UNIT);
    if( ptr->GetAIInterface() )
        ptr->GetAIInterface()->WipeTargetList();
    return 1;
}

int LuaUnit_WipeCurrentTarget(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET_INT(TYPEID_UNIT);
    ptr->GetAIInterface()->WipeCurrentTarget();
    return 1;
}

int LuaUnit_GetPlayerClass(lua_State * L, Unit * ptr)
{
   if(!ptr || ptr->GetTypeId()!=TYPEID_PLAYER) {
      lua_pushstring(L, "Unknown");
      return 1;
   }
   int plrclass = ((Player*)ptr)->getClass();

   switch(plrclass) {
      case 1:
         lua_pushstring(L, "Warrior");
         break;
      case 2:
         lua_pushstring(L, "Paladin");
         break;
      case 3:
         lua_pushstring(L, "Hunter");
         break;
      case 4:
         lua_pushstring(L, "Rogue");
         break;
      case 5:
         lua_pushstring(L, "Priest");
         break;
      //case 6:
      case 7:
         lua_pushstring(L, "Shaman");
         break;
      case 8:
         lua_pushstring(L, "Mage");
         break;
      case 9:
         lua_pushstring(L, "Warlock");
         break;
      //case 10:
      case 11:
         lua_pushstring(L, "Druid");
         break;
      default:
         lua_pushstring(L, "Unknown");
         break;
   }

   return 1;
}


int LuaUnit_ClearHateList(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET_INT(TYPEID_UNIT);
    if( ptr->GetAIInterface() )
        ptr->GetAIInterface()->ClearHateList();
    return 1;
}
int LuaUnit_SetMana(lua_State * L, Unit * ptr)
{
    int val = luaL_checkint( L, 1);
    if( ptr != NULL)
        ptr->SetUInt32Value( UNIT_FIELD_POWER1, val );
    return 1;
}
int LuaUnit_SetMaxMana(lua_State * L, Unit * ptr)
{
    int val = luaL_checkint( L, 1);
    if( ptr != NULL && val > 0 )
        if( (uint32)val < ptr->GetUInt32Value( UNIT_FIELD_POWER1) )
            ptr->SetUInt32Value( UNIT_FIELD_POWER1, val);
        ptr->SetUInt32Value( UNIT_FIELD_MAXPOWER1, val );
    return 1;
}
int LuaUnit_GetPlayerRace(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET( TYPEID_PLAYER );
    lua_pushinteger( L, static_cast< Player* >( ptr )->getRace() );
    return 1;
}
int LuaUnit_SetFlying(lua_State * L, Unit * ptr)
{
    WorldPacket data(SMSG_MOVE_SET_HOVER, 13);
    data << ptr->GetNewGUID();
    data << uint32(0);
    ptr->SendMessageToSet(&data, true);
    ptr->GetAIInterface()->disable_melee = true;
    ptr->GetAIInterface()->setMoveFlyFlag(true);
    ptr->Emote(EMOTE_ONESHOT_LIFTOFF);
    return 1;
}
 
int LuaUnit_Land(lua_State * L, Unit * ptr)
{
    WorldPacket data(SMSG_MOVE_UNSET_HOVER, 13);
    data << ptr->GetNewGUID();
    data << uint32(0);
    ptr->SendMessageToSet(&data, true);
    ptr->GetAIInterface()->setMoveFlyFlag(false);
    ptr->GetAIInterface()->disable_melee = false;
    ptr->Emote(EMOTE_ONESHOT_LAND);
    return 1;
}
int LuaUnit_HasAura(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int spellid = luaL_checkint (L,1);
    if (!spellid )
        return 0;
    else
        if(ptr->HasAura(spellid))
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);

    return 1;
}
int LuaUnit_ReturnToSpawnPoint(lua_State * L, Unit * ptr)
{
    float x = ptr->GetSpawnX();
    float y = ptr->GetSpawnY();
    float z = ptr->GetSpawnZ();
    if(ptr)
        ptr->GetAIInterface()->MoveTo(x, y, z);
    return 1;
}
int LuaUnit_GetGUID(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    lua_pushinteger(L,(int)ptr->GetGUID()); return 1;
}
int LuaUnit_GetDistance(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = Lunar<Unit>::check(L, 1);
    lua_pushnumber(L,(float)ptr->GetDistance2dSq(target));
    return 1;
}
int LuaUnit_GetCreatureNearestCoords(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint32 entryid = luaL_checkint(L,4);
    float x = (float)luaL_checknumber(L,1);
    float y = (float)luaL_checknumber(L,2);
    float z = (float)luaL_checknumber(L,3);
    if(entryid == 0) 
        lua_pushnil(L);
    else
        Lunar<Unit>::push(L, TO_UNIT(ptr->GetMapMgr()->GetObjectClosestToCoords(entryid, x, y, z, 99999.0f, TYPEID_UNIT)), false);
    return 1;
    
}
int LuaUnit_GetGameObjectNearestCoords(lua_State *L, Unit * ptr)
{
    TEST_UNIT();
    uint32 entryid = luaL_checkint(L,4);
    float x = (float)luaL_checknumber(L,1);
    float y = (float)luaL_checknumber(L,2);
    float z = (float)luaL_checknumber(L,3);
    if(entryid == 0) 
        lua_pushnil(L);
    else
        Lunar<GameObject>::push(L, TO_GAMEOBJECT(ptr->GetMapMgr()->GetObjectClosestToCoords(entryid, x, y, z, 99999.0f, TYPEID_GAMEOBJECT)), false);
    return 1;
}
int LuaUnit_CastSpellAoF(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint32 sp = luaL_checkint(L, 4);
    float x = (float)luaL_checknumber(L,1);
    float y = (float)luaL_checknumber(L,2);
    float z = (float)luaL_checknumber(L,3);
    if(!sp | !x | !y | !z)
        return 0;
    ptr->CastSpellAoF(x,y,z,dbcSpell.LookupEntry(sp), true);
    return 1;
}
int LuaUnit_SetInFront(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = Lunar<Unit>::check(L, 1);
    if(!target)
        return 0;
    ptr->GetAIInterface()->setInFront(target);
    return 1;
}
int LuaUnit_RemoveAllAuras(lua_State *L, Unit * ptr)
{
    TEST_UNIT();
    ptr->m_AuraInterface.RemoveAllAuras();
    return 1;
}
int LuaUnit_CancelSpell(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    sEventMgr.AddEvent(ptr, &Unit::EventCancelSpell, ptr->GetCurrentSpell(), EVENT_UNK, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    return 1;
}
int LuaUnit_IsAlive(lua_State * L, Unit * ptr)
{
    TEST_UNIT_RET();

    if(ptr->isAlive())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}
int LuaUnit_IsDead(lua_State * L, Unit * ptr)
{
    if(ptr)
        if(ptr->isDead())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    return 1;
}
int LuaUnit_IsInWorld(lua_State * L, Unit * ptr)
{
    if (ptr)
        if(ptr->IsInWorld())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    return 1;
}
int LuaUnit_GetZoneId(lua_State *L, Unit * ptr)
{
    CHECK_TYPEID_RET_INT(TYPEID_UNIT);
    lua_pushinteger(L,(ptr->GetZoneId()));
    return 1;
}
int LuaUnit_GetMana(lua_State * L, Unit * ptr)
{
    if( ptr == NULL )
        lua_pushinteger( L, 0 );
    else
        lua_pushinteger( L, ptr->GetUInt32Value( UNIT_FIELD_POWER1 ) );

    return 1;
}

int LuaUnit_GetMaxMana(lua_State * L, Unit * ptr)
{
    if( ptr == NULL )
        lua_pushinteger( L, 0 );
    else
        lua_pushinteger( L, ptr->GetUInt32Value( UNIT_FIELD_MAXPOWER1 ) );

    return 1;
}
int LuaUnit_Root(lua_State * L, Unit * ptr)
{
    if(ptr)
        ptr->Root();
    return 1;
}
int LuaUnit_Unroot(lua_State * L, Unit * ptr)
{
    if(ptr)
        ptr->UnRoot();
    return 1;
}
int LuaUnit_IsCreatureMoving(lua_State * L, Unit * ptr)
{
    if(ptr)
        if(ptr->GetAIInterface()->m_creatureState == MOVING)
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    return 1;
}
int LuaUnit_SetOutOfCombatRange(lua_State * L, Unit * ptr)
{
    int range = luaL_checkint(L, 1);
    if(ptr && range != 0)
        ptr->GetAIInterface()->setOutOfCombatRange(range);
    return 1;
}
int LuaUnit_ModifyRunSpeed(lua_State * L, Unit * ptr)
{
    float Speed = (float)luaL_checkint(L, 1);
    if(ptr)
        ptr->m_runSpeed = Speed;
    return 1;
}
int LuaUnit_ModifyWalkSpeed(lua_State * L, Unit * ptr)
{
    float Speed = (float)luaL_checkint(L,1);
    if(ptr && Speed)
        ptr->m_walkSpeed = Speed;
    return 1;
}
int LuaUnit_ModifyFlySpeed(lua_State * L, Unit * ptr)
{
    float Speed = (float)luaL_checkint(L,1);
    if(ptr&&Speed)
        ptr->m_flySpeed = Speed;
    return 1;
}
int LuaUnit_IsFlying(lua_State * L, Unit * ptr)
{
    if(ptr)
        if(ptr->GetAIInterface()->IsFlying())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    return 1;
}
int LuaUnit_SetRotation(lua_State * L, Unit * ptr)
{
    if(ptr)
        ptr->SetRotation(ptr->GetGUID());
    return 1;
}
int LuaUnit_SetOrientation(lua_State * L, Unit * ptr)
{
    float O = (float)luaL_checknumber(L, 1);
    if(ptr)
        ptr->SetOrientation(O);
    return 1;
}
int LuaUnit_CalcDistance(lua_State * L, Unit * ptr)
{
    Object * ob = Lunar<Object>::check(L,1);
    if(ob)
        lua_pushnumber(L,(float)ptr->CalcDistance(ob));
    return 1;
}
int LuaUnit_GetSpawnX(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushnumber(L,ptr->GetSpawnX());
    return 1;
}
int LuaUnit_GetSpawnY(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushnumber(L,ptr->GetSpawnY());
    return 1;
}
int LuaUnit_GetSpawnZ(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushnumber(L,ptr->GetSpawnZ());
    return 1;
}
int LuaUnit_GetSpawnO(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushnumber(L,ptr->GetSpawnO());
    return 1;
}
int LuaUnit_GetInRangePlayersCount(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushnumber(L,ptr->GetInRangePlayersCount());
    return 1;
}
int LuaUnit_GetEntry(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushnumber(L,ptr->GetEntry());
    return 1;
}
int LuaUnit_SetMoveRunFlag(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_UNIT);
    int enabled = luaL_checkint(L, 1);
    ptr->GetAIInterface()->setMoveRunFlag((enabled > 0) ? true : false);
    return 1;
}

int LuaUnit_HandleEvent(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_UNIT);
    Unit * target = Lunar<Unit>::check(L, 1);
    int event_id = luaL_checkint(L, 2);
    int misc_1 = luaL_checkint(L, 3);
    ptr->GetAIInterface()->HandleEvent(event_id, target, misc_1);
    return 1;
}
int LuaUnit_GetCurrentSpellId(lua_State * L, Unit * ptr)
{
    if(ptr->GetCurrentSpell())
        lua_pushnumber(L, ptr->GetCurrentSpell()->GetSpellProto()->Id);
    else 
        lua_pushnil(L);
    return 1;
}
int LuaUnit_GetCurrentSpell(lua_State * L, Unit * ptr)
{
    if(ptr)
        if(ptr->GetCurrentSpell() != NULL)
        {
            lua_pushlstring(L,ptr->GetCurrentSpell()->GetSpellProto()->Name,0);
        }
    return 1;
}
int LuaUnit_GetAIState(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_UNIT);
    lua_pushnumber(L, ptr->GetAIInterface()->getAIState());
    return 1;
}

int LuaUnit_GetFloatValue(lua_State * L, Unit * ptr)
{
    int field = luaL_checkint(L, 1);
    if( ptr != NULL ) 
        lua_pushnumber(L, ptr->GetFloatValue(field));
    return 1;
}

int LuaUnit_SendPacket(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();

    WorldPacket * data = CHECK_PACKET(L,1);
    int self = lua_toboolean(L,2);
    if (data)
        ptr->SendMessageToSet(data,(self > 0 ) ? true : false);
    return 1;
}

int LuaUnit_InitPacket(lua_State * L, Unit * ptr)
{
    int packet_id = luaL_checkint(L, 1);
    WorldPacket data;
    data.Initialize(packet_id);
    return 1;
}

int LuaUnit_AddDataToPacket(lua_State * L, Unit * ptr)
{
    WorldPacket dat;
    int type = luaL_checkint(L, 1);
    switch(type)
    {
        case 0:
            // int
            dat << luaL_checkint(L, 2);
            break;
        case 1:
            // uint8
            dat << (uint8)luaL_checknumber(L, 2);
            break;
        case 2:
            // uint16
            dat << (uint16)luaL_checknumber(L, 2);
            break;
        case 3:
            // uint32
            dat << (uint32)luaL_checknumber(L, 2);
            break;
        case 4:
            // uint64
            dat << (uint64)luaL_checknumber(L, 2);
            break;
        case 5:
            // float
            dat << (float)luaL_checknumber(L, 2);
            break;
        case 6:
            // double
            dat << (double)luaL_checknumber(L, 2);
            break;
        case 7:
            // string
            dat << luaL_checkstring(L, 2);
            break;
    }
    return 1;
}

int LuaUnit_AddGuidDataToPacket(lua_State * L, Unit * ptr)
{
    WorldPacket dat;
    int type = luaL_checkint(L, 1);
    switch(type)
    {
        // not sure... maybee we should use ptr->GetMapMgr->GetGUID??
        case 0:
            dat << ptr->GetGUID();
            break;
        case 1:
            dat << ptr->GetTypeFromGUID();
            break;
        case 2:
            dat << ptr->GetUIdFromGUID();
            break;
        case 3:
            dat << ptr->GetNewGUID(); 
            break;
    }
    return 1;
}

int LuaUnit_ModUInt32Value(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    int field = luaL_checkint(L, 1);
    if(field >= PLAYER_END)
        return 0;

    int value = luaL_checkint(L, 2);
    ptr->ModSignedInt32Value(field, value);
    return 1;
}

int LuaUnit_ModFloatValue(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    int field = luaL_checkint(L, 1);
    if(field >= PLAYER_END)
        return 0;

    float value = (float)luaL_checknumber(L, 2);
    ptr->ModFloatValue(field, value);
    return 1;
}

int LuaUnit_SetUInt32Value(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    int field = luaL_checkint(L, 1);
    if(field >= PLAYER_END)
        return 0;

    int value = luaL_checkint(L, 2);
    ptr->SetUInt32Value(field, value);
    return 1;
}

int LuaUnit_SetUInt64Value(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    int field = luaL_checkint(L, 1);
    if(field >= PLAYER_END)
        return 0;

    int value = luaL_checkint(L, 2);
    ptr->SetUInt64Value(field, value);
    return 1;
}

int LuaUnit_SetFloatValue(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    int field = luaL_checkint(L, 1);
    if(field >= PLAYER_END)
        return 0;

    float value = (float)luaL_checknumber(L, 2);
    ptr->SetFloatValue(field, value);
    return 1;
}

int LuaUnit_GetUInt32Value(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    int field = luaL_checkint(L, 1);
    if(field >= PLAYER_END)
        return 0;

    lua_pushnumber(L, ptr->GetUInt32Value(field));
    return 1;
}

int LuaUnit_GetUInt64Value(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    int field = luaL_checkint(L, 1);
    if(field >= PLAYER_END)
        return 0;

    lua_pushinteger(L, (int)ptr->GetUInt64Value(field));
    return 1;
}

int LuaUnit_AdvanceQuestObjective(lua_State * L, Unit * ptr)
{
    TEST_PLAYER()
    int questid = luaL_checkint(L,1);
    int objective = luaL_checkint(L,2);
    Player * pl = ((Player*)ptr);
    QuestLogEntry * qle = pl->GetQuestLogForEntry(questid);
    qle->SetMobCount(objective, qle->GetMobCount(objective) + 1);
    qle->SendUpdateAddKill(objective);
    if(qle->CanBeFinished())
        qle->SendQuestComplete();
    qle->UpdatePlayerFields();
    return 1;
}
int LuaUnit_Heal(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = Lunar<Unit>::check(L, 1);
    uint32 spellid = luaL_checkint(L, 2);
    uint32 amount = luaL_checkint(L, 3);
    if (!target || !spellid || !amount)
        return 0;
    ptr->Heal(target,spellid,amount);
    return 1;
}
int LuaUnit_Energize(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = Lunar<Unit>::check(L, 1);
    uint32 spellid = luaL_checkint(L, 2);
    uint32 amount = luaL_checkint(L, 3);
    uint32 type = luaL_checkint(L, 4);
    if(!target||!spellid||!amount||!type)
        return 0;
    ptr->Energize(target,spellid,amount,type);
    return 1;
}
int LuaUnit_SendChatMessageAlternateEntry(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint32 entry = luaL_checkint(L, 1);
    uint8 type = luaL_checkint(L, 2);
    uint32 lang = luaL_checkint(L, 3);
    const char * msg = luaL_checkstring(L, 4);
    if(!entry||!type||!lang||!msg)
        return 0;
    ptr->SendChatMessageAlternateEntry(entry,type,lang,msg);
    return 1;
}
int LuaUnit_SendChatMessageToPlayer(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint8 type = luaL_checkint(L, 1);
    uint32 lang = luaL_checkint(L, 2);
    const char *msg = luaL_checkstring(L,3);
    Player *plr = Lunar<Player>::check(L,4);
    if(!msg)
        return 0;
    ptr->SendChatMessageToPlayer(type,lang,msg,plr);
    return 1;
}

int LuaUnit_Strike(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET(TYPEID_UNIT);

    Unit * target = Lunar<Unit>::check(L, 1);
    uint32 weapon_damage_type = luaL_checkint(L, 2);
    uint32 sp = luaL_checkint(L, 3);
    int32 adddmg = luaL_checkint(L, 4);
    uint32 exclusive_damage = luaL_checkint(L, 5);
    int32 pct_dmg_mod = luaL_checkint(L, 6);

    if(!target||!weapon_damage_type||!sp||!adddmg||!pct_dmg_mod)
        return 0;
    ptr->Strike(target,weapon_damage_type,dbcSpell.LookupEntry(sp),adddmg,pct_dmg_mod,exclusive_damage,false,false);
    return 1;
}
int LuaUnit_SetAttackTimer(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int32 timer = luaL_checkint(L, 1);
    uint32 offhand = luaL_checkint(L,2);
    if(!timer||!ptr||!offhand||offhand != 1||offhand != 0)
        return 0;
    ptr->setAttackTimer(timer,false);
    return 1;
}
int LuaUnit_Kill(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_UNIT);
    Object * plr = ptr;
    Unit * target = Lunar<Unit>::check(L, 1);
    if (!plr ||!target)
        return 0;
    plr->DealDamage(target,target->GetUInt32Value(UNIT_FIELD_HEALTH),0,0,0);
    return 1;
}
int LuaUnit_DealDamage(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_UNIT);
    Object * plr = ptr;
    Unit * target = Lunar<Unit>::check(L, 1);
    uint32 damage = luaL_checkint(L, 2);
    uint32 spellid = luaL_checkint(L, 3);
    if(!plr||!target)
        return 0;
    plr->DealDamage(target,damage,0,0,spellid);
    return 1;
}
int LuaUnit_SetNextTarget(lua_State * L, Unit * ptr)
{
    Unit * target = Lunar<Unit>::check(L, 1);
    if(ptr && target)
        ptr->GetAIInterface()->SetNextTarget(target);
    return 1;
}
int LuaUnit_GetNextTarget(lua_State * L, Unit * ptr)
{
    if(ptr || ptr->IsInWorld())
        Lunar<Unit>::push(L, ptr->GetAIInterface()->GetNextTarget());
    return 1;
}
int LuaUnit_SetPetOwner(lua_State * L, Unit * ptr)
{
    Unit * owner = Lunar<Unit>::check(L, 1);
    if(ptr || owner)
        ptr->GetAIInterface()->SetPetOwner(owner);
    return 1;
}
int LuaUnit_IsPet(lua_State * L, Unit * ptr)
{
    if(ptr)
        if(ptr->IsPet())
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L, 0);
    return 1;
}
int LuaUnit_GetPetOwner(lua_State * L, Unit * ptr)
{
    if(ptr)
        Lunar<Unit>::push(L, ptr->GetAIInterface()->GetPetOwner());
    return 1;
}
int LuaUnit_SetUnitToFollow(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = Lunar<Unit>::check(L, 1);
    float dist = (float)luaL_checkint(L, 2);
    float angle = (float)luaL_checkint(L, 3);
    if(!target||!dist||dist<=0||!angle)
        return 0;
    ptr->GetAIInterface()->SetUnitToFollow(target);
    ptr->GetAIInterface()->SetFollowDistance(dist);
    ptr->GetAIInterface()->SetUnitToFollowAngle(angle);
    return 1;
}
int LuaUnit_GetUnitToFollow(lua_State * L, Unit * ptr)
{
    if(ptr)
        Lunar<Unit>::push(L,ptr->GetAIInterface()->getUnitToFollow());
    return 1;
}
int LuaUnit_IsInFront(lua_State * L, Unit * ptr)
{
    Unit * target = Lunar<Unit>::check(L, 1);
    if(ptr && target)
        if(ptr->isTargetInFront(target))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit_IsInBack(lua_State * L, Unit * ptr)
{
    Unit * target = Lunar<Unit>::check(L, 1);
    if(ptr && target)
        if(ptr->isTargetInBack(target))
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    return 1;
}
int LuaUnit_IsPacified(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushboolean(L,(ptr->IsPacified())?1:0);
    return 1;
}

int LuaUnit_SetPacified(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    bool pacified = CHECK_BOOL(L, 1);
    ptr->m_pacified = pacified ? 1 : 0;
    if (pacified)
        ptr->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
    else
        ptr->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
    return 1;
}

int LuaUnit_IsFeared(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushboolean(L,(ptr->IsFeared())?1:0);
    return 1;
}
int LuaUnit_IsStunned(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushboolean(L,(ptr->IsStunned())?1:0);
    return 1;
}
int LuaUnit_CreateGuardian(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_UNIT);

    uint32 guardian_entry = luaL_checkint(L,1);
    uint32 duration = luaL_checkint(L, 2);
    float angle = (float)luaL_checkint(L, 3);
    uint32 lvl  = luaL_checkint(L, 4);
    if(!ptr||!guardian_entry||!lvl)
        return 0;
    ptr->CreateTemporaryGuardian(guardian_entry,duration,angle,lvl,0);
    return 1;
}

int LuaUnit_IsInArc(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = Lunar<Unit>::check(L, 1);
    float degrees = (float)luaL_checkint(L, 2);
    if(!target||!degrees)
        return 0;
    else
        if(ptr->isInArc(target,degrees))
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);
    return 1;
}

int LuaUnit_IsInWater(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    if(ptr)
        if(static_cast< Player* >( ptr )->m_UnderwaterState)
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    return 1;
}
    
int LuaUnit_GetAITargetsCount(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushnumber(L,ptr->GetAIInterface()->getAITargetsCount());
    return 1;
}
int LuaUnit_GetUnitByGUID(lua_State * L, Unit * ptr)
{
    uint64 guid = luaL_checkint(L,1);
    if(ptr && guid)
        Lunar<Unit>::push(L,ptr->GetMapMgr()->GetUnit(guid));
    return 1;
}

/*int LuaUnit_GetAITargets(lua_State * L, Unit * ptr)
{
    Unit * ret = NULL;
    TargetMap::iterator itr;
    lua_newtable(L);
    int count = 0;
    for( itr = ptr->GetAIInterface()->GetAITargets()->begin();itr!= ptr->GetAIInterface()->GetAITargets()->end();itr++)
    {
        ret = ptr->GetMapMgr()->GetUnit(itr->first);
        count++;
        lua_pushvalue(L,count);
        Lunar<Unit>::push(L,ret,false);
        lua_rawset(L,-3);
    }
    return 1;
}*/
int LuaUnit_GetInRangeObjectsCount(lua_State * L, Unit * ptr)
{
    if(ptr)
        lua_pushnumber(L,ptr->GetInRangeCount());
    return 1;
}
int LuaUnit_GetInRangePlayers(lua_State * L, Unit * ptr)
{
    Player * ret = NULL;
    uint32 count = 0;
    lua_newtable(L);
    for(unordered_set<Player*>::iterator itr = ptr->GetInRangePlayerSetBegin(); itr != ptr->GetInRangePlayerSetEnd(); itr++)
    {
        if( (*itr) ->GetTypeId() == TYPEID_PLAYER)
        {
            count++,
            ret = *itr;
            lua_pushinteger(L,count);
            Lunar<Unit>::push(L,((Unit*)ret),false);
            lua_rawset(L,-3);
        }
    }
    return 1;
}
int LuaUnit_GetInRangeGameObjects(lua_State * L, Unit * ptr)
{
    Object * ret = NULL;
    lua_newtable(L);
    uint32 count = 0;
    for (unordered_set<Object*>::iterator itr = ptr->GetInRangeSetBegin();itr!= ptr->GetInRangeSetEnd();itr++)
    {
        if( (*itr) ->GetTypeId() == TYPEID_GAMEOBJECT)
        {
            count++,
            lua_pushinteger(L,count);
            Lunar<GameObject>::push(L,(GameObject*)(*itr));
            lua_rawset(L,-3);
        }
    }
    return 1;
}
    
int LuaUnit_HasInRangeObjects(lua_State * L, Unit * ptr)
{
    if(ptr)
        if(ptr->HasInRangeObjects())
            lua_pushboolean(L, 1);
        else
            lua_pushboolean(L, 0);
    return 1;
}
int LuaUnit_SetFacing(lua_State * L, Unit * ptr)
{
    float newo = (float)luaL_checkint(L, 1);
    if(!ptr|!newo)
        return 0;
    ptr->SetFacing(newo);
    return 1;
}
int LuaUnit_CalcToDistance(lua_State * L, Unit * ptr)
{
    float x = (float)luaL_checkint(L,1 );
    float y = (float)luaL_checkint(L, 2);
    float z = (float)luaL_checkint(L, 3);
    if (!ptr|!x|!y|!z)
        return 0;
    lua_pushnumber(L,ptr->CalcDistance(x,y,z));
    return 1;
}
int LuaUnit_CalcAngle(lua_State * L, Unit * ptr)
{
    float x = (float)luaL_checkint(L,1 );
    float y = (float)luaL_checkint(L, 2);
    float x2 = (float)luaL_checkint(L, 3);
    float y2 = (float)luaL_checkint(L, 4);
    if(!x||!y||!x2||!y2||!ptr)
        return 0;
    lua_pushnumber(L,ptr->calcAngle(x,y,x2,y2));
    return 1;
}
int LuaUnit_CalcRadAngle(lua_State * L, Unit * ptr)
{
    float ang = 0.0f;
    float x = (float)luaL_checkint(L,1 );
    float y = (float)luaL_checkint(L, 2);
    float dx = (float)luaL_checkint(L, 3);
    float dy = (float)luaL_checkint(L, 4);
    if(!x||!y||!dx||!dy||!ptr)
        return 0;
    else
        ang = ptr->calcRadAngle(x,y,dx,dy);
    lua_pushnumber(L,ang);
    return 1;
}
int LuaUnit_IsInvisible(lua_State * L, Unit * ptr)
{
    int enabled = luaL_checkint(L, 1);
    if(!ptr|!enabled)
        return 0;
    if(enabled > 0)
        ptr->m_invisFlag = INVIS_FLAG_TOTAL;
        ptr->m_invisible = true;
    return 1;
}
int LuaUnit_MoveFly(lua_State * L, Unit * ptr)
{
    int enabled = luaL_checkint(L,1);
    if(ptr)
        ptr->GetAIInterface()->setMoveFlyFlag(((enabled > 0) ? true : false));
    return 1;
}
int LuaUnit_IsInvincible(lua_State * L, Unit * ptr)
{
    int enabled = luaL_checkint(L, 1);
    if(ptr)
        ptr->bInvincible = ((enabled > 0)? true : false );
    return 1;
}
int LuaUnit_ResurrectPlayer(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    if(ptr)
        static_cast< Player * > ( ptr ) ->ResurrectPlayer( TO_PLAYER(ptr) );
    return 1;
}
int LuaUnit_KickPlayer(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    int delay = luaL_checkint(L,1);
    if(ptr)
        static_cast<Player*>(ptr)->Kick(delay);
    return 1;
}
int LuaUnit_CanCallForHelp(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_UNIT)
    int enabled = luaL_checkint(L, 1);
    if(!ptr|!enabled)
        return 0;
    ptr->GetAIInterface()->m_canCallForHelp = ((enabled < 0)? true: false);
    return 1;
}
int LuaUnit_CallForHelpHp(lua_State * L, Unit * ptr)
{
    float hp = (float)luaL_checkint(L, 1);
    if(ptr)
        ptr->GetAIInterface()->m_CallForHelpHealth = hp;
    return 1;
}
int LuaUnit_SetDeathState(lua_State * L, Unit * ptr)
{
    int state = luaL_checkint(L, 1);
    if(ptr)
        switch(state)
    {
        case 0:
            ptr->setDeathState (ALIVE);
            break;
        case 1:
            ptr->setDeathState (JUST_DIED);
            break;
        case 2:
            ptr->setDeathState (CORPSE);
            break;
        case 3:
            ptr->setDeathState (DEAD);
            break;
    }
    return 1;
}
int LuaUnit_SetCreatureName(lua_State * L, Unit * ptr)
{
    uint32 id = luaL_checkint(L,1);
    if(!ptr|!id)
        return 0;
    static_cast<Creature*>(ptr)->SetCreatureName(CreatureNameStorage.LookupEntry(id));
    return 1;
}
int LuaUnit_GetSpellId(lua_State * L, Unit * ptr)
{
    uint32 spellid = luaL_checkint(L,1);
    if(ptr)
        Lunar<SpellEntry>::push(L,ptr->GetAIInterface()->getSpellEntry(spellid));
    return 1;
}
int LuaUnit_Possess(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);

    Unit * target = Lunar<Unit>::check(L,1);
    if(target)
        static_cast<Player*>(ptr)->Possess(target);
    return 1;
}
int LuaUnit_Unpossess(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    Player* target = Lunar<Player>::check(L,1);
    if(target)
        target->UnPossess();
    return 1;
}
int LuaUnit_RemoveFromWorld(lua_State * L, Unit * ptr)
{
    if(ptr)
        ptr->RemoveFromWorld(true);
    return 1;
}
int LuaUnit_GetFaction(lua_State * L, Unit * ptr)
{
    if(ptr)
    {
        lua_pushnumber(L,ptr->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    }
    return 1;
}
int LuaUnit_SpellNonMeleeDamageLog(lua_State * L, Unit * ptr)
{
    Unit * pVictim = Lunar<Unit>::check(L,1);
    uint32 spellid = luaL_checkint(L,2);
    uint32 damage = luaL_checkint(L,3);
    int allowproc = luaL_checkint(L,4);
    int static_dmg = luaL_checkint(L,5);
    int no_remove_auras = luaL_checkint(L,6);
    if(pVictim&&spellid&&damage&&allowproc&&static_dmg&&no_remove_auras)
    {
        ptr->SpellNonMeleeDamageLog(pVictim,spellid,damage,((allowproc >0)?true:false),((static_dmg>0)?true:false),((no_remove_auras>0)?true:false));
    }
    return 1;
}
int LuaUnit_NoRespawn(lua_State * L, Unit * ptr)
{
    int enabled = luaL_checkint(L,1);
    if(ptr&&enabled)
        static_cast<Creature*>( ptr ) ->m_noRespawn = ((enabled < 0)? true:false);
    return 1;
}
int LuaUnit_GetMapId(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET(TYPEID_UNIT || TYPEID_PLAYER);

    if(!ptr->GetMapId())
        lua_pushnil(L);
    else
        lua_pushinteger(L,ptr->GetMapId());
    return 1;
}
int LuaUnit_AttackReaction(lua_State * L, Unit * ptr)
{
    Unit * target = Lunar<Unit>::check(L,1);
    uint32 damage = luaL_checkint(L,2);
    uint32 spell = luaL_checkint(L,3);
    if(ptr && target && damage)
    {
        ptr->GetAIInterface()->AttackReaction(target,damage,spell);
    }
    return 1;
}
int LuaUnit_EventCastSpell(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_UNIT || TYPEID_PLAYER);
    Unit * target = Lunar<Unit>::check(L, 1);
    uint32 sp = luaL_checkint(L,2);
    uint32 delay = luaL_checkint(L,3);
    uint32 repeats = luaL_checkint(L,4);
    if ( ptr && sp)
    {
        switch(ptr->GetTypeId())
        {
        case TYPEID_PLAYER:
            sEventMgr.AddEvent(ptr, &Player::EventCastSpell,target,dbcSpell.LookupEntry(sp),EVENT_PLAYER_UPDATE,delay,repeats,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        case TYPEID_UNIT:
            sEventMgr.AddEvent(ptr, &Unit::EventCastSpell,target,dbcSpell.LookupEntry(sp),EVENT_CREATURE_UPDATE,delay,repeats,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
    }
    return 1;
}
int LuaUnit_IsPlayerMoving(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET(TYPEID_PLAYER);
    if( static_cast<Player*>( ptr ) ->m_isMoving)
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}
int LuaUnit_IsPlayerAttacking(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET(TYPEID_PLAYER);
    if( TO_PLAYER(ptr) ->IsAttacking())
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}
int LuaUnit_GetFactionStanding(lua_State * L, Unit * ptr)
{
    uint32 faction = luaL_checkint(L,1);
    if (ptr && faction)
    {
        switch(((Player*)ptr)->GetStanding(faction))
        {
        case STANDING_HATED:
            lua_pushstring(L, "Hated");
        case STANDING_HOSTILE:
            lua_pushstring(L, "Hostile");
        case STANDING_UNFRIENDLY:
            lua_pushstring(L, "Unfriendly");
        case STANDING_NEUTRAL:
            lua_pushstring(L, "Neutral");
        case STANDING_FRIENDLY:
            lua_pushstring(L, "Friendly");
        case STANDING_HONORED:
            lua_pushstring(L, "Honored");
        case STANDING_REVERED:
            lua_pushstring(L, "Revered");
        case STANDING_EXALTED:
            lua_pushstring(L, "Exalted");
        }
    }
return 1;
}
int LuaUnit_SetPlayerAtWar(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET(TYPEID_PLAYER);
    uint32 faction = luaL_checkint(L,1);
    uint32 set = luaL_checkint(L,2);
    if(ptr && faction && set)
    {
        ((Player*)ptr)->SetAtWar(faction,((set > 0)? true : false));
    }
    return 1;
}
/*int LuaUnit_IsPlayerAtWar(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET(TYPEID_PLAYER);
    uint32 faction = luaL_checkint(L,1);
    if (ptr && faction)
        if( ((Player*)ptr)->IsAtWar(faction))
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);
    return 1;
}*/
int LuaUnit_SetPlayerStanding(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    uint32 faction = luaL_checkint(L,1);
    uint32 value = luaL_checkint(L,2);
    if(faction && value)
        ((Player*)ptr)->SetStanding(faction,value);
    return 1;
}
int LuaUnit_GetStanding(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    uint32 faction = luaL_checkint(L,1);
    if(ptr && faction)
        lua_pushinteger(L,((Player*)ptr)->GetStanding(faction));
    return 1;
}
int LuaUnit_RemoveThreatByPtr(lua_State * L, Unit * ptr)
{
    Unit * target = Lunar<Unit>::check(L, 1);
    if( ptr && target)
    {
        ptr->GetAIInterface()->RemoveThreatByPtr(target);
    }
    return 1;
}
int LuaUnit_HasItem(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    uint32 itemid = luaL_checkint(L,1);
    if(ptr && itemid)
    {
        if( ((Player*)ptr)->GetItemInterface()->GetItemCount(itemid,false) > 0 )
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);
    }
    return 1;
}
int LuaUnit_PlaySpellVisual(lua_State * L, Unit * ptr)
{
    Unit * target = Lunar<Unit>::check(L,1);
    uint32 spell = luaL_checkint(L,2);
    if( ptr && target && spell)
    {
        ptr->PlaySpellVisual(target->GetGUID(),spell);
    }
    return 1;
}
int LuaUnit_GetPlayerLevel(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET(TYPEID_PLAYER);
    if(ptr)
    {
        lua_pushnumber(L,static_cast<Player*>( ptr ) ->getLevel());
    }
    return 1;
}
int LuaUnit_SetPlayerLevel(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID_RET(TYPEID_PLAYER);
    uint32 level = luaL_checkint(L,1);
    LevelInfo * Info = objmgr.GetLevelInfo(ptr->getRace(),ptr->getClass(),level);
    if (Info != NULL)
        TO_PLAYER(ptr)->lvlinfo = Info;
    TO_PLAYER(ptr)->ApplyLevelInfo(level);
    return 1;
}
int LuaUnit_AddSkill(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    uint32 skill = luaL_checkint(L,1);
    uint32 current = luaL_checkint(L,2);
    uint32 max = luaL_checkint(L,3);
    if(!max) max = 375;
    if(current > max)
        ((Player*)ptr)->BroadcastMessage("CURRENT LEVEL CAN'T BE GREATER THAN MAX LEVEL");
        return 0;
    ((Player*)ptr)->_AddSkillLine(skill,current,max);
    ((Player*)ptr)->_UpdateMaxSkillCounts();
    return 1;
}
int LuaUnit_RemoveSkill(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    uint32 skill = luaL_checkint(L,1);
    if(!ptr ||!skill) return 0;
    ((Player*)ptr)->_RemoveSkillLine(skill);
    ((Player*)ptr)->_UpdateMaxSkillCounts();
    return 1;
}
int LuaUnit_FlyCheat(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    uint32 enabled = luaL_checkint(L,1);
    if(ptr && enabled)
        ((Player*)ptr)->FlyCheat = ((enabled > 0)? true : false);
    return 1;
}
int LuaUnit_AdvanceSkill(lua_State * L, Unit * ptr)
{
    CHECK_TYPEID(TYPEID_PLAYER);
    uint32 skill = luaL_checkint(L,1);
    uint32 count = luaL_checkint(L,2);
    if(skill && count)
    {
        if( ((Player*)ptr)->_HasSkillLine(skill))
            ((Player*)ptr)->_AdvanceSkillLine(skill,count);
    }
    return 1;
}
int LuaUnit_RemoveAurasByMechanic(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 mechanic = luaL_checkint(L,1);
    uint32 hostileonly = luaL_checkint(L,2);
    if(ptr && mechanic && hostileonly)
    {
        ptr->m_AuraInterface.RemoveAllAurasByMechanic(mechanic,-1,((hostileonly > 0)? true : false));
    }
    return 1;
}
int LuaUnit_RemoveAurasType(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 type = luaL_checkint(L,1);
    if(ptr && type)
    {
        ptr->m_AuraInterface.RemoveAllAurasOfType(type);
    }
    return 1;
}

int LuaUnit_AddAura(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 spellid = luaL_checkint(L,1);
    uint32 duration = luaL_checkint(L,2);
    bool temp = CHECK_BOOL(L,3);
    if(ptr && spellid)
    {
        Aura * aura = new Aura(dbcSpell.LookupEntry(spellid), duration, ptr, ptr);
        ptr->AddAura(aura);
        lua_pushboolean(L, 1);
    }
    else
        lua_pushboolean(L,0);
    return 1;
}

int LuaUnit_SetAIState(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint32 state = luaL_checkint(L,1);
    if(state)
    {
        switch(state)
        {
        case 0:
            ptr->GetAIInterface()->SetAIState(STATE_IDLE);
            break;
        case 1:
            ptr->GetAIInterface()->SetAIState(STATE_ATTACKING);
            break;
        case 2:
            ptr->GetAIInterface()->SetAIState(STATE_CASTING);
            break;
        case 3:
            ptr->GetAIInterface()->SetAIState(STATE_FLEEING);
            break;
        case 4:
            ptr->GetAIInterface()->SetAIState(STATE_FOLLOWING);
            break;
        case 5:
            ptr->GetAIInterface()->SetAIState(STATE_EVADE);
            break;
        case 6:
            ptr->GetAIInterface()->SetAIState(STATE_MOVEWP);
            break;
        case 7:
            ptr->GetAIInterface()->SetAIState(STATE_FEAR);
            break;
        case 8:
            ptr->GetAIInterface()->SetAIState(STATE_WANDER);
            break;
        case 9:
            ptr->GetAIInterface()->SetAIState(STATE_STOPPED);
            break;
        case 10:
            ptr->GetAIInterface()->SetAIState(STATE_SCRIPTMOVE);
            break;
        case 11:
            ptr->GetAIInterface()->SetAIState(STATE_SCRIPTIDLE);
            break;
        }
    }
    return 1;
}

int LuaUnit_SetStealth(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    uint32 stealthlevel = CHECK_ULONG(L,1);
    ptr->SetStealth(stealthlevel);
    return 1;
}

int LuaUnit_GetStealthLevel(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    lua_pushinteger(L,ptr->GetStealthLevel());
    return 1;
}

int LuaUnit_IsStealthed(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    if(ptr->m_stealth)
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}

int LuaUnit_RemoveStealth(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    ptr->RemoveStealth();
    return 1;
}

int LuaUnit_InterruptSpell(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    if(ptr->GetCurrentSpell())
        ptr->GetCurrentSpell()->cancel();
    return 1;
}

int LuaUnit_IsPoisoned(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    if(ptr->IsPoisoned())
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}

int LuaUnit_deleteWaypoint(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint32 wp = luaL_checkint(L,1);
    if(ptr && wp)
        TO_CREATURE(ptr)->GetAIInterface()->deleteWayPoint(wp);
    return 1;
}

int LuaUnit_DealGoldCost(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player* plr = TO_PLAYER(ptr);
    int32 debt = luaL_checkint(L,1);
    if(debt < 0)
        return 0;

    if(plr->GetUInt32Value(PLAYER_FIELD_COINAGE) < (uint32)debt)
        lua_pushboolean(L, 0);
    else
    {
        plr->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -debt);
        lua_pushboolean(L,1);
    }
    return 1;
}

int LuaUnit_DealGoldMerit(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 profit = CHECK_ULONG(L,1);
    TO_PLAYER(ptr)->ModUnsigned32Value(PLAYER_FIELD_COINAGE, profit);
    return 1;
}

int LuaUnit_DeMorph(lua_State * L, Unit * ptr)
{
    if (!ptr) return 0;
    ptr->DeMorph();
    return 1;
}

int LuaUnit_Attack(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Unit * target = CHECK_UNIT(L, 1);
    if (target)
    {
        ptr->GetAIInterface()->WipeTargetList();
        ptr->GetAIInterface()->ClearHateList();
        ptr->GetAIInterface()->WipeCurrentTarget();
        ptr->GetAIInterface()->taunt(target);
        lua_pushboolean(L,1);
    }
    else
        lua_pushboolean(L,0);
    return 1;
}

int LuaUnit_CanUseCommand(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    char cmdlevel = (char)luaL_checkstring(L,1)[0];
    Player * plr = TO_PLAYER(ptr);
    if (plr->GetSession()->CanUseCommand(cmdlevel))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit_GetTarget(lua_State * L, Unit * ptr)
{
//  Crow: Fuck it, use GetTarget, see if I give a fuck.
//  Log.Notice("LuaEngine", "GetTarget is outdated. Please use GetPrimaryCombatTarget.");
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Unit * target = plr->GetMapMgr()->GetUnit(plr->GetSelection());
    if(target != NULL)
        Lunar<Unit>::push(L,target);
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit_GetSelection(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Unit * selection = plr->GetMapMgr()->GetUnit(plr->GetSelection());
    if (selection)
        Lunar<Unit>::push(L,selection);
    else
        lua_pushnil(L);
    return 1;
}
int LuaUnit_GetSelectedGO(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Lunar<GameObject>::push(L,plr->m_GM_SelectedGO);
    return 1;
}

int LuaUnit_RepairAllPlayerItems(lua_State * L, Unit * ptr)
{
    TEST_PLAYER_RET()
    Player * plr = TO_PLAYER(ptr);
    Item * pItem = NULL;
    Container * pContainer = NULL;
    int32 j, i;

    for( i = 0; i < MAX_INVENTORY_SLOT; i++ )
    {
        pItem = plr->GetItemInterface()->GetInventoryItem( i );
        if( pItem != NULL )
        {
            if( pItem->IsContainer() )
            {
                pContainer = static_cast<Container*>( pItem );
                for( j = 0; j < pContainer->GetProto()->ContainerSlots; ++j )
                {
                    pItem = pContainer->GetItem( j );
                    if( pItem != NULL ){
                        pItem->SetDurabilityToMax();
                    }
                }
            }
            else
            {
                if( pItem->GetProto()->MaxDurability > 0 && i < INVENTORY_SLOT_BAG_END && pItem->GetDurability() <= 0 )
                {
                    pItem->SetDurabilityToMax();
                    plr->ApplyItemMods( pItem, i, true );
                }
                else
                {
                    pItem->SetDurabilityToMax();
                }
            }
        }
    }
    return 1;
}

int LuaUnit_SetKnownTitle(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    int title = luaL_checkint(L,1);

    return 1;
}

int LuaUnit_UnsetKnownTitle(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    int title = luaL_checkint(L,1);

    return 1;
}

int LuaUnit_LifeTimeKills(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    int kills = luaL_checkint(L, 1);
    const char * check = luaL_checklstring(L,2,NULL);
    Player * plr = (Player*)ptr;
    int killscheck = plr->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);
    if (check && strncmp(check, "add", 4)==0 && kills > 0) 
    { 
        plr->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, killscheck+kills);
        plr->SaveToDB(false);
        return 1; 
    }
    else if (check && strncmp(check, "del", 4)==0 && killscheck >= kills) 
    { 
        plr->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, killscheck-kills);
        plr->SaveToDB(false); 
        return 1; 
    }
    else if (check && strncmp(check, "set", 4)==0 && kills >= 0) 
    { 
        plr->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, kills); 
        plr->SaveToDB(false); 
        return 1; 
    }
    else if (check == NULL || kills == 0) 
    { 
        lua_pushinteger( L, killscheck ); 
        return 1; 
    }
    return 1;
}

int LuaUnit_HasTitle(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    int title = luaL_checkint(L, 1);

    return 1;
}

int LuaUnit_GetMaxSkill(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 skill = luaL_checkint(L, 1);
    lua_pushinteger(L, TO_PLAYER(ptr)->_GetSkillLineMax(skill));
    return 1;
}

int LuaUnit_GetCurrentSkill(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 skill = luaL_checkint(L, 1);
    lua_pushinteger(L, TO_PLAYER(ptr)->_GetSkillLineCurrent(skill));
    return 1;
}

int LuaUnit_HasSkill(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 skill = luaL_checkint(L, 1);
    lua_pushboolean(L, (TO_PLAYER(ptr)->_HasSkillLine(skill)) ? 1 : 0);
    return 1;
}

int LuaUnit_GetGuildName(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
/*  Guild * pGuild = guildmgr.GetGuild(TO_PLAYER(ptr)->GetGuildId());
    if(pGuild != NULL)
        lua_pushstring(L,pGuild->GetGuildName());
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaUnit_ClearCooldownForSpell(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    plr->ClearCooldownForSpell(luaL_checkint(L, 1));
    return 1;
}

int LuaUnit_HasSpell(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 sp = CHECK_ULONG(L,1);
    lua_pushboolean(L, (sp && TO_PLAYER(ptr)->HasSpell(sp)) ? 1 : 0);
    return 1;
}

int LuaUnit_ClearAllCooldowns(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    TO_PLAYER(ptr)->ResetAllCooldowns();
    return 1;
}

int LuaUnit_ResetAllTalents(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    TO_PLAYER(ptr)->Reset_Talents(true);
    return 1;
}

int LuaUnit_GetAccountName(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    const char * aName = TO_PLAYER(ptr)->GetSession()->GetAccountNameS();
    lua_pushstring(L,aName);
    return 1;
}

int LuaUnit_GetGmRank(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    const char * level = TO_PLAYER(ptr)->GetSession()->GetPermissions();
    if(level != NULL)
        lua_pushstring(L,level);
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit_IsGm(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    if(TO_PLAYER(ptr)->GetSession()->HasGMPermissions())
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}

int LuaUnit_SavePlayer(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    TO_PLAYER(ptr)->SaveToDB(false);
    return 1;
}

int LuaUnit_HasQuest(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 quest_id = CHECK_ULONG(L,1);
    if (quest_id && TO_PLAYER(ptr)->HasQuest(quest_id))
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}

int LuaUnit_CreatureHasQuest(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Creature * ctr = TO_CREATURE(ptr);
    uint32 questid = CHECK_ULONG(L,1);
    Quest * qst = sQuestMgr.GetQuestPointer(questid);
    if (ctr->HasQuest(qst->id, qst->qst_type))
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}

int LuaUnit_RemovePvPFlag(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    if(plr != NULL && plr->IsPvPFlagged())
        plr->RemovePvPFlag();
    return 1;
}

int LuaUnit_RemoveNegativeAuras(lua_State * L, Unit * ptr)
{
    if (!ptr)
        return 0;

// TODO
//  ptr->();
    return 1;
}

int LuaUnit_GossipMiscAction(lua_State * L, Unit * ptr)
{
    Player* plr = TO_PLAYER(ptr);
    Creature * crc = TO_CREATURE(CHECK_UNIT(L, 2));
    uint32 miscint = (uint32)luaL_checkint(L, 3);
    int actionid = luaL_checkint(L, 1);
    if ( !plr || (!crc && actionid < 9) )
        return 0;

    if (actionid == 1) plr->GetSession()->SendInventoryList(crc); 
    else if (actionid == 2) plr->GetSession()->SendTrainerList(crc);
    else if (actionid == 3) plr->GetSession()->SendInnkeeperBind(crc);
    else if (actionid == 4) plr->GetSession()->SendBankerList(crc);
    else if (actionid == 5) plr->GetSession()->SendBattlegroundList(crc,miscint);
    else if (actionid == 6) plr->GetSession()->SendAuctionList(crc);
    else if (actionid == 7) plr->GetSession()->SendTabardHelp(crc);
    else if (actionid == 8) plr->GetSession()->SendSpiritHealerRequest(crc);
    else if (actionid == 9) plr->SendTalentResetConfirm();
    else if (actionid == 10) plr->SendPetUntrainConfirm();
    return 1;
}

int LuaUnit_SendVendorWindow(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player* plr = TO_PLAYER(ptr);
    Creature * object = TO_CREATURE(CHECK_UNIT(L, 1)); //NOT entry. The unit pointer.
    if(plr != NULL && object != NULL)
        plr->GetSession()->SendInventoryList(object);
    return 1;
}

int LuaUnit_SendTrainerWindow(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player* plr = TO_PLAYER(ptr);
    Creature * crc = TO_CREATURE(CHECK_UNIT(L, 1)); //NOT entry. The unit pointer.
    if(crc != NULL)
        plr->GetSession()->SendTrainerList(crc);
    return 1;
}

int LuaUnit_SendInnkeeperWindow(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player* plr = TO_PLAYER(ptr);
    Creature * crc = TO_CREATURE(CHECK_UNIT(L, 1)); //NOT entry. The unit pointer.
    if(crc != NULL)
        plr->GetSession()->SendInnkeeperBind(crc);
    return 1;
}

int LuaUnit_SendBankWindow(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player* plr = TO_PLAYER(ptr);
    Creature * crc = TO_CREATURE(CHECK_UNIT(L, 1)); //NOT entry. The unit pointer.
    if(crc != NULL)
        plr->GetSession()->SendBankerList(crc);
    return 1;
}

int LuaUnit_SendAuctionWindow(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player* plr = TO_PLAYER(ptr);
    Creature * crc = TO_CREATURE(CHECK_UNIT(L, 1)); //NOT entry. The unit pointer.
    if(crc != NULL)
        plr->GetSession()->SendAuctionList(crc);
    return 1;
}

int LuaUnit_SendBattlegroundWindow(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Creature * crc = TO_CREATURE(CHECK_UNIT(L,1));
    uint32 bgid = luaL_checkint(L,2);
    if(bgid && crc != NULL)
        plr->GetSession()->SendBattlegroundList(crc, bgid); //player filler ftw
    return 1;
}

int LuaUnit_SendLootWindow(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint64 guid = luaL_checkint(L,1);
    uint8 loot_type = (uint8)luaL_checkint(L,2);
    uint8 loot_type2 = 1;
    Player * plr = TO_PLAYER(ptr);
    plr->SetLootGUID(guid);
    uint32 guidtype = GUID_HIPART(guid);
    if (guidtype == HIGHGUID_TYPE_UNIT)
    {
        Unit* pUnit = plr->GetMapMgr()->GetUnit(guid);
        CreatureProto * proto = TO_CREATURE(pUnit)->GetProto();
        switch (loot_type) 
        {
        case 2:
            lootmgr.FillGatheringLoot(&pUnit->m_loot, pUnit->GetEntry());
            loot_type2 = 2;
            break;
        case 3:
            lootmgr.FillPickpocketingLoot(&pUnit->m_loot, pUnit->GetEntry());
            loot_type2 = 2;
            break;
        default:
            lootmgr.FillCreatureLoot(&pUnit->m_loot, pUnit->GetEntry(), (pUnit->GetMapMgr() ? ( pUnit->GetMapMgr()->iInstanceMode ? true : false ) : false), plr->GetTeam());
            pUnit->m_loot.gold = proto ? proto->money : 0;
            loot_type2 = 1;
            break;
        }
    }
    else if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* pGO = plr->GetMapMgr()->GetGameObject(GUID_LOPART(guid));
        switch (loot_type) 
        {
        case 5:
            lootmgr.FillGatheringLoot(&pGO->m_loot, pGO->GetEntry());
            loot_type2 = 2;
            break;
        default:
            lootmgr.FillGOLoot(&pGO->m_loot, pGO->GetEntry(), (pGO->GetMapMgr() ? ( pGO->GetMapMgr()->iInstanceMode ? true : false ) : false), plr->GetTeam());
            loot_type2 = 1;
            break;
        }
    }
    else if(guidtype == HIGHGUID_TYPE_ITEM)
    {
        Item *pItem = plr->GetItemInterface()->GetItemByGUID(guid);
        switch (loot_type) 
        {
        case 6:
            lootmgr.FillItemLoot(&pItem->m_loot,pItem->GetEntry(), plr->GetTeam());
            loot_type2 = 1;
            break;
        default:
            break;
        }
    }
    plr->SendLoot(guid,2, plr->GetMapId());
    return 1;
}

int LuaUnit_AddLoot(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint32 itemid = luaL_checkint(L,1);
    uint32 mincount = luaL_checkint(L,2);
    uint32 maxcount = luaL_checkint(L,3);
    uint32 ffa_loot = luaL_checkint(L,4);
    lootmgr.AddLoot(&ptr->m_loot,itemid,mincount,maxcount,ffa_loot);
    return 1;
}

int LuaUnit_VendorAddItem(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Creature * ctr = TO_CREATURE(ptr);
    uint32 itemid = (uint32)luaL_checknumber(L, 1);
    uint32 amount = (uint32)luaL_checknumber(L, 2);
    uint32 costid = (uint32)luaL_checknumber(L, 3);
    if (itemid && amount)
        ctr->AddVendorItem(itemid, amount, costid);

    return 1;
}

int LuaUnit_VendorRemoveItem(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Creature * ctr = TO_CREATURE(ptr);
    uint32 itemid = (uint32)luaL_checknumber(L, 1); 
    int slot = ctr->GetSlotByItemId(itemid);
    if(itemid && slot > 0)
        ctr->RemoveVendorItem(itemid);
    return 1;
}

int LuaUnit_VendorRemoveAllItems(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Creature * ctr = TO_CREATURE(ptr);
    uint32 i = 0;
    if (ctr->HasItems())
    {
        uint32 creatureitemids[200];
        uint32 count = uint32(ctr->GetSellItemCount());
        for(std::map<uint32, CreatureItem>::iterator itr = ctr->GetSellItemBegin(); itr != ctr->GetSellItemEnd(); ++itr)
        {
            creatureitemids[i] = itr->second.itemid;
            i+=1;
        }
        for (i=0; i<count; i++) {
            ctr->RemoveVendorItem(creatureitemids[i]);
        }
    }
    return 1;
}

int LuaUnit_EquipWeapons(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint32 equip1 = luaL_checkint(L, 1);
    uint32 equip2 = luaL_checkint(L, 2);
    uint32 equip3 = luaL_checkint(L, 3);
    ptr->SetWeaponDisplayId(MELEE,equip1);
    ptr->SetWeaponDisplayId(OFFHAND,equip2);
    ptr->SetWeaponDisplayId(RANGED,equip3);
    return 1;
}

int LuaUnit_Dismount(lua_State * L, Unit * ptr)
{
    if (!ptr) return 0;
    if(ptr->IsPlayer())
    {
        Player * plr = TO_PLAYER(ptr);
        plr->RemoveAura(plr->m_MountSpellId);
        plr->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID , 0);
    }
    else
        ptr->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID , 0);
    return 1;
}

int LuaUnit_GiveXp(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * pl = TO_PLAYER(ptr);
    uint32 exp = luaL_checkint(L, 1);
    pl->GiveXP(exp, pl->GetGUID(), true);
    return 1;
}

int LuaUnit_AdvanceAllSkills(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    uint32 skillvalue = luaL_checkint(L, 1);
    plr->_AdvanceAllSkills(skillvalue);
    return 1;
}

int LuaUnit_GetTeam(lua_State * L, Unit * ptr) //returns 0 for alliance, 1 for horde.
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    lua_pushinteger(L, plr->GetTeam());
    return 1;
}

int LuaUnit_StartTaxi(lua_State * L, Unit * ptr)
{
    //TODO
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    TaxiPath * tp = CHECK_TAXIPATH(L, 1);
    uint32 mount_id = luaL_checkint(L, 2);
    plr->TaxiStart(tp, mount_id, 0);*/
    return 1;
}

int LuaUnit_IsOnTaxi(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    lua_pushboolean(L, TO_PLAYER(ptr)->GetTaxiState() ? 1 : 0);
    return 1;
}

int LuaUnit_GetTaxi(lua_State * L, Unit * ptr)
{
    //TODO
//  TEST_PLAYER();
//  PUSH_TAXIPATH(L, TO_PLAYER(ptr)->GetTaxiPath());
    return 1;
}

int LuaUnit_SetPlayerLock(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    bool lock = CHECK_BOOL(L,1);
    if (lock) 
    {
        ptr->m_pacified = 1;
        ptr->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
        WorldPacket data1(9);
        data1.Initialize( SMSG_CLIENT_CONTROL_UPDATE );
        data1 << ptr->GetNewGUID() << uint8(0x00);
        TO_PLAYER(ptr)->GetSession()->SendPacket(&data1);
    }
    else
    {
        ptr->m_pacified = 0;
        ptr->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
        WorldPacket data1(9);
        data1.Initialize( SMSG_CLIENT_CONTROL_UPDATE );
        data1 << ptr->GetNewGUID() << uint8(0x01);
        TO_PLAYER(ptr)->GetSession()->SendPacket(&data1);
    }
    return 1;
}

int LuaUnit_MovePlayerTo(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    float x = CHECK_FLOAT(L,1);
    float y = CHECK_FLOAT(L,2);
    float z = CHECK_FLOAT(L,3);
    float o = CHECK_FLOAT(L,4);
    uint32 mov_flag = CHECK_ULONG(L,5); //0 - walk, 256 - teleport, 4096 - run, 12288 - fly
    float moveSpeed = (float)luaL_optnumber(L,6,1.0f);
    if (moveSpeed == 1.0f)
    {
        if (mov_flag == 0)
            moveSpeed = 2.5f*0.001f;
        else
            moveSpeed = 7.0f*0.001f;
    }
    ptr->SetFacing(o);
    ptr->SetOrientation(o);
    float distance = ptr->CalcDistance(ptr->GetPositionX(), ptr->GetPositionY(), ptr->GetPositionZ(), x, y, z);
    uint32 moveTime = uint32(distance / moveSpeed);
    WorldPacket data(SMSG_MONSTER_MOVE, 50);
    data << ptr->GetNewGUID();
    data << uint8(0);
    data << ptr->GetPositionX();
    data << ptr->GetPositionY();
    data << ptr->GetPositionZ();
    data << getMSTime();
    data << uint8(0x00);
    data << uint32(mov_flag);
    data << moveTime;
    data << uint32(1);
    data << x << y << z;

    ptr->SendMessageToSet(&data, true);
    ptr->SetPosition(x,y,z,o);
    return 1;
}

int LuaUnit_ChannelSpell(lua_State * L, Unit * ptr)
{
    uint32 Csp = luaL_checkint(L, 1);
    Unit * target = CHECK_UNIT(L, 2);
    if (Csp && target != NULL) 
    {
        ptr->CastSpell(target,dbcSpell.LookupEntry(Csp),false);
        ptr->SetUInt64Value( UNIT_FIELD_CHANNEL_OBJECT, target->GetGUID() );
        ptr->SetUInt32Value(UNIT_CHANNEL_SPELL, Csp);
    }
    return 1;
}

int LuaUnit_StopChannel(lua_State * L, Unit * ptr)
{
    ptr->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
    ptr->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
    return 1;
}

int LuaUnit_EnableFlight(lua_State * L, Unit * ptr)
{   
    TEST_PLAYER();
    bool Switch = CHECK_BOOL(L,1);
    if (Switch)
    {
        WorldPacket fly(835, 13);
        ptr->EnableFlight();
        fly << ptr->GetNewGUID();
        fly << uint32(2);
        ptr->SendMessageToSet(&fly, true);
    }
    else
    {
        WorldPacket fly(836, 13);
        ptr->DisableFlight();
        fly << ptr->GetNewGUID();
        fly << uint32(5);
        ptr->SendMessageToSet(&fly, true);
    }
    return 1;
}

int LuaUnit_GetCoinage(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    lua_pushinteger(L, plr->GetUInt32Value(PLAYER_FIELD_COINAGE));
    return 1;
}

int LuaUnit_FlagPvP(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    plr->SetPvPFlag();
    return 1;
}

int LuaUnit_IsMounted(lua_State * L, Unit * ptr)
{
    if (!ptr) return 0;
    if (ptr->IsPlayer())
    {
        Player * plr = TO_PLAYER(ptr);
        if(plr != NULL && plr->IsMounted())
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);
    }
    else
        lua_pushboolean(L, (ptr->GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID) > 0) ? 1 : 0);
    return 1;
}

int LuaUnit_SpawnVehicle(lua_State * L, Unit * ptr)
{
    uint32 entry = (uint32)luaL_checknumber(L,1);
    float x = CHECK_FLOAT(L,2);
    float y = CHECK_FLOAT(L,3);
    float z = CHECK_FLOAT(L,4);
    float o = CHECK_FLOAT(L,5);
    uint32 faction = (uint32)luaL_checknumber(L,6);
    uint32 duration = (uint32)luaL_checknumber(L,7);
    uint32 phase = luaL_optint(L, 8, ptr->GetPhaseMask());
    uint32 mode = ptr->GetMapMgr()->iInstanceMode;

    CreatureProto * proto = CreatureProtoStorage.LookupEntry(entry);
    CreatureInfo * info = CreatureNameStorage.LookupEntry(entry);
    if(proto != NULL && info != NULL && proto->vehicle_entry > 0) // Only allow the creation of vehicles that are vehicles.
    {
        Vehicle * p = ptr->GetMapMgr()->CreateVehicle(entry);
        ASSERT(p);
        p->Load(proto, mode, x, y, z, o);
        p->SetPhaseMask(phase);
        p->PushToWorld(ptr->GetMapMgr());
        Lunar<Unit>::push(L,p);
    }
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit_SetVehicle(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Unit * unitVehicle = CHECK_UNIT(L,1);
    if(!unitVehicle->IsVehicle())
        return 0;

    int8 seat = luaL_checkint(L,2); //make it -1 for auto-choose.
    Vehicle * vehicle = ((Vehicle*)unitVehicle);
    if (vehicle && seat >= -1)
        vehicle->AddPassenger(plr,seat);
    return 1;
}

int LuaUnit_GetVehicle(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Vehicle * ride = TO_VEHICLE(plr->GetVehicle(true));
    if(ride != NULL)
        Lunar<Unit>::push(L,ride); 
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit_RemoveFromVehicle(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Vehicle * veh = TO_VEHICLE(plr->GetVehicle(true));
    if(veh != NULL)
        veh->RemovePassenger(plr);
    return 1;
}

int LuaUnit_GetVehicleSeat(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    if(plr->GetVehicle() != NULL)
        lua_pushinteger(L,plr->GetSeatID());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit_IsVehicle(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    lua_pushboolean(L, ( ptr->IsVehicle() ) ? 1 : 0);
    return 1;
}

int LuaUnit_GetPassengerCount(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Vehicle * veh = static_cast<Vehicle*>(ptr);
    if (veh)
        lua_pushinteger(L,veh->GetPassengerCount());
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit_MoveVehicle(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    float x = CHECK_FLOAT(L,1);
    float y = CHECK_FLOAT(L,2);
    float z = CHECK_FLOAT(L,3);
    float o = CHECK_FLOAT(L,4);
    Vehicle * veh = static_cast<Vehicle*>(ptr);
    if(veh != NULL)
        veh->MoveVehicle(x,y,z,o);
    return 1;
}

//credits to alvanaar for the following 9 functions:
int LuaUnit_IsGroupedWith(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * target = CHECK_PLAYER(L,1);
    if(TO_PLAYER(ptr)->GetGroup()->HasMember(target))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit_GetGroupType(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Group * group = plr->GetGroup();
    if(group != NULL)
        lua_pushinteger(L,group->GetGroupType() );
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit_GetTotalHonor(lua_State * L, Unit * ptr) // I loathe typing "honour" like "honor".
{
    TEST_PLAYER();
    lua_pushinteger(L, TO_PLAYER(ptr)->m_honorPoints);
    return 1;
}

int LuaUnit_GetHonorToday(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    lua_pushinteger(L, TO_PLAYER(ptr)->m_honorToday);
    return 1;
}

int LuaUnit_GetHonorYesterday(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    lua_pushinteger(L, ((Player*)ptr)->m_honorYesterday);
    return 1;
}

int LuaUnit_GetArenaPoints(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    lua_pushinteger(L, TO_PLAYER(ptr)->m_arenaPoints);
    return 1;
}

int LuaUnit_AddArenaPoints(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 pnts = luaL_checkint(L, 1);
    Player* plr = TO_PLAYER(ptr);
    if (pnts) 
    {
        plr->m_arenaPoints += pnts;
        plr->RecalculateHonor();
    }
    return 1;
}

int LuaUnit_RemoveArenaPoints(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 pnts = luaL_checkint(L, 1);
    Player* plr = TO_PLAYER(ptr);
    int32 npts = plr->m_arenaPoints-pnts;
    if(npts >= 0) 
    {
        plr->m_arenaPoints = npts;
        plr->RecalculateHonor();
    }
    return 1;
}

int LuaUnit_AddLifetimeKills(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 pnts = luaL_checkint(L, 1);
    Player * plr = TO_PLAYER(ptr);
    plr->m_killsLifetime += pnts;
    return 1;
}

int LuaUnit_GetGender(lua_State * L, Unit * ptr)
{
    if (!ptr) return 0;
    lua_pushinteger(L, ptr->getGender());
    return 1;
}

int LuaUnit_SetGender(lua_State * L, Unit * ptr)
{
    if (!ptr) return 0;
    uint8 gender = luaL_checkint(L,1);
    ptr->setGender(gender);
    return 1;
}
//next 5 credits: alvanaar
int LuaUnit_SendPacketToGuild(lua_State * L, Unit * ptr)
{
    // TODO
/*  TEST_PLAYER();
    WorldPacket * data = CHECK_PACKET(L,1);
    Player* plr = TO_PLAYER(ptr);
    Guild* guild = plr->GetGuild();
    if (data != NULL && guild != NULL)
        guild->SendPacket(data);*/
    return 1;
}

int LuaUnit_GetGuildId(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
/*  Player* plr = TO_PLAYER(ptr);
    if (plr->GetGuild() != NULL)
        lua_pushinteger(L, plr->GetGuildId());
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaUnit_GetGuildRank(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
/*  Player * plr = TO_PLAYER(ptr);
    if (plr->GetGuild() != NULL)
        lua_pushinteger(L, plr->GetGuildRank());
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaUnit_SetGuildRank(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
/*
    Player* plr = TO_PLAYER(ptr);
    int rank = luaL_checkint(L, 1);
    if(plr->IsInGuild() )
        plr->SetGuildRank(rank);*/
    return 1;
}

int LuaUnit_IsInGuild(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

/*  Player* plr = TO_PLAYER(ptr);
    if(plr->IsInGuild())
        lua_pushboolean(L, 1);
    else*/
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit_SendGuildInvite(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
/*  Player * sender = TO_PLAYER(ptr);
    Player * plyr = CHECK_PLAYER(L,1);
    std::string inviteeName = plyr->GetName();
    Guild *pGuild = sender->GetGuild();
    if(!plyr)
    {
        Guild::SendGuildCommandResult(sender->GetSession(), GUILD_INVITE_S,inviteeName.c_str(),GUILD_PLAYER_NOT_FOUND);
    }
    else if(!pGuild)
    {
        Guild::SendGuildCommandResult(sender->GetSession(), GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
    }
    else if( plyr->GetGuildId() )
    {
        Guild::SendGuildCommandResult(sender->GetSession(), GUILD_INVITE_S,plyr->GetName(),ALREADY_IN_GUILD);
    }
    else if( plyr->GetGuildInvitersGuid())
    {
        Guild::SendGuildCommandResult(sender->GetSession(), GUILD_INVITE_S,plyr->GetName(),ALREADY_INVITED_TO_GUILD);
    }
    else
    {
        Guild::SendGuildCommandResult(sender->GetSession(), GUILD_INVITE_S,inviteeName.c_str(),GUILD_U_HAVE_INVITED);
        WorldPacket data(SMSG_GUILD_INVITE, 100);
        data << sender->GetName();
        data << pGuild->GetGuildName();
        plyr->GetSession()->SendPacket(&data);
        plyr->SetGuildInvitersGuid( sender->GetLowGUID() );
    }*/
    return 1;
}

int LuaUnit_DemoteGuildMember(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Player * target = CHECK_PLAYER(L,1);
    if (target)
        plr->GetGuild()->DemoteGuildMember(target->m_playerInfo, plr->GetSession());*/
    return 1;
}

int LuaUnit_PromoteGuildMember(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Player * target = CHECK_PLAYER(L,1);
    if (target)
        plr->GetGuild()->PromoteGuildMember(target->m_playerInfo, plr->GetSession());*/
    return 1;
}

int LuaUnit_SetGuildMotd(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    const char * szNewMotd = luaL_checkstring(L,1);
    if (szNewMotd != NULL)
        plr->GetGuild()->SetMOTD(szNewMotd, plr->GetSession());*/
    return 1;
}

int LuaUnit_GetGuildMotd(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    if(plr->GetGuild() != NULL)
        lua_pushstring(L,plr->GetGuild()->GetMOTD());
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaUnit_SetGuildInformation(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    const char * gi = luaL_checkstring(L,1);
    if (gi)
        plr->GetGuild()->SetGuildInformation(gi, plr->GetSession());*/
    return 1;
}

int LuaUnit_AddGuildMember(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Player * target = CHECK_PLAYER(L,1);
    int32 rank = luaL_optinteger(L, 2, -1);
    if (target)
        plr->GetGuild()->AddGuildMember(target->m_playerInfo, plr->GetSession(), rank);*/
    return 1;
}

int LuaUnit_RemoveGuildMember(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Player * target = CHECK_PLAYER(L,1);
    if (target)
        plr->GetGuild()->RemoveGuildMember(target->m_playerInfo, plr->GetSession());*/
    return 1;
}

int LuaUnit_SetPublicNote(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Player * target = CHECK_PLAYER(L,1);
    const char * note = luaL_checkstring(L,2);
    if (target && note)
        plr->GetGuild()->SetPublicNote(target->m_playerInfo, note, plr->GetSession());*/
    return 1;
}

int LuaUnit_SetOfficerNote(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Player * target = CHECK_PLAYER(L,1);
    const char * note = luaL_checkstring(L,2);
    if (target && note)
        plr->GetGuild()->SetOfficerNote(target->m_playerInfo, note, plr->GetSession());*/
    return 1;
}

int LuaUnit_DisbandGuild(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    if (plr->GetGuild() != NULL)
        plr->GetGuild()->Disband();*/
    return 1;
}

int LuaUnit_ChangeGuildMaster(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Player * target = CHECK_PLAYER(L,1);
    if (target)
        plr->GetGuild()->ChangeGuildMaster(target->m_playerInfo, plr->GetSession());*/
    return 1;
}

int LuaUnit_SendGuildChatMessage(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    const char * message = luaL_checkstring(L,1);
    bool officer = CHECK_BOOL(L,2);
    if(plr->GetGuild() != NULL && message != NULL)
        (officer) ? plr->GetGuild()->OfficerChat(message, plr->GetSession(), 0) :  plr->GetGuild()->GuildChat(message, plr->GetSession(), 0);*/
    return 1;
}

int LuaUnit_SendGuildLog(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    if (plr->GetGuild() != NULL)
        plr->GetGuild()->SendGuildLog(plr->GetSession());*/
    return 1;
}

int LuaUnit_GuildBankDepositMoney(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    uint32 amount = luaL_checkint(L,1);
    if (plr->GetGuild() != NULL)
        plr->GetGuild()->DepositMoney(plr->GetSession(), amount);*/
    return 1;
}

int LuaUnit_GuildBankWithdrawMoney(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    uint32 amount = luaL_checkint(L,1);
    if (plr->GetGuild() != NULL)
        plr->GetGuild()->WithdrawMoney(plr->GetSession(), amount);*/
    return 1;
}

int LuaUnit_SetByteValue(lua_State * L, Unit * ptr)
{
    uint32 index = luaL_checkint(L,1);
    uint32 index1 = luaL_checkint(L,2);
    uint8 value = luaL_checkint(L,3);
    ptr->SetByte(index,index1,value);
    return 1;
}

int LuaUnit_GetByteValue(lua_State * L, Unit * ptr)
{
    uint32 index = luaL_checkint(L,1);
    uint32 index1 = luaL_checkint(L,2);
    lua_pushinteger(L,ptr->GetByte(index,index1));
    return 1;
}

int LuaUnit_IsPvPFlagged(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    lua_pushboolean(L,TO_PLAYER(ptr)->IsPvPFlagged() ? 1 : 0);
    return 1;
}

int LuaUnit_IsFFAPvPFlagged(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    lua_pushboolean(L,TO_PLAYER(ptr)->IsFFAPvPFlagged() ? 1 : 0);
    return 1;
}

int LuaUnit_GetGuildLeader(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
/*  Guild * pGuild = TO_PLAYER(ptr)->GetGuild();
    if(pGuild != NULL) 
    {
        Player * plr = objmgr.GetPlayer(pGuild->GetGuildLeader());
        if (plr != NULL)
            lua_pushstring(L,plr->GetName());
        else
            lua_pushnil(L);
    }
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaUnit_GetGuildMemberCount(lua_State * L, Unit * ptr)
{
/*  TEST_PLAYER();
    Guild * pGuild = TO_PLAYER(ptr)->GetGuild();
    (pGuild != NULL) ? lua_pushinteger(L,pGuild->GetNumMembers()) : lua_pushnil(L);*/
    return 1;
}

int LuaUnit_IsFriendly(lua_State * L, Unit * ptr)
{
    TEST_UNIT_RET();

    Unit * obj = CHECK_UNIT(L,1);
    if (!obj)
        return 0;

    if (sFactionSystem.isFriendly(ptr, obj))
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}

int LuaUnit_IsInChannel(lua_State * L, Unit * ptr)
{
    TEST_PLAYER_RET();

    Player* plr = TO_PLAYER(ptr);
    const char* channel_name = luaL_checkstring(L, 1);
    if(!channel_name)
        return 0;

    Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
    if(pChannel->HasMember(plr)) // Channels: "General", "Trade", "LocalDefense", "GuildRecruitment", "LookingForGroup", (or any custom channel)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit_JoinChannel(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    Player* plr = TO_PLAYER(ptr);
    const char* channel_name = luaL_checkstring(L, 1);
    Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
    const char* pw = luaL_optstring(L, 2, pChannel->m_password.c_str());

    if(!channel_name || pChannel->HasMember(plr) || !pChannel)
        return 0;
    else
        pChannel->AttemptJoin(plr, pw);

    return 1;
}

int LuaUnit_LeaveChannel(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    Player* plr = TO_PLAYER(ptr);
    const char* channel_name = luaL_checkstring(L, 1);
    Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
    if(!channel_name || !pChannel || !pChannel->HasMember(plr))
        return 0;
    else
        pChannel->Part(plr, true);
    return 1;
}

int LuaUnit_SetChannelName(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    Player* plr = TO_PLAYER(ptr);
    const char* current_name = luaL_checkstring(L, 1);
    const char* new_name = luaL_checkstring(L, 2);
    Channel* pChannel = channelmgr.GetChannel(current_name, plr);
    if(!current_name || !new_name || !pChannel || pChannel->m_name == new_name)
        return 0;

    pChannel->m_name = new_name;
    return 1;
}

int LuaUnit_SetChannelPassword(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    Player* plr = TO_PLAYER(ptr);
    const char* channel_name = luaL_checkstring(L, 1);
    const char* pass = luaL_checkstring(L, 2);
    Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
    if(!pass || pChannel->m_password == pass)
        return 0;

    pChannel->Password(TO_PLAYER(ptr), pass);
    return 1;
}

int LuaUnit_GetChannelPassword(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    Player* plr = TO_PLAYER(ptr);
    const char* channel_name = luaL_checkstring(L, 1);
    Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
    lua_pushstring(L, pChannel->m_password.c_str());
    return 1;
}

int LuaUnit_KickFromChannel(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    Player* plr = TO_PLAYER(ptr);
    const char* channel_name = luaL_checkstring(L, 1);
    Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
    if(!pChannel)
        return 0;

    pChannel->Kick(plr, plr, false);
    return 1;
}

int LuaUnit_BanFromChannel(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    Player* plr = TO_PLAYER(ptr);
    const char* channel_name = luaL_checkstring(L, 1);
    Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
    if(!pChannel)
        return 0;

    pChannel->Kick(plr, plr, true);
    return 1;
}

int LuaUnit_UnbanFromChannel(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    Player* plr = TO_PLAYER(ptr);
    const char* channel_name = luaL_checkstring(L, 1);
    Channel* pChannel = channelmgr.GetChannel(channel_name, plr);
    if(!plr || !pChannel)
        return 0;
    pChannel->Unban(plr, plr->getPlayerInfo());
    return 1;
}

int LuaUnit_GetChannelMemberCount(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    Player* plr = TO_PLAYER(ptr);
    const char* channel_name = luaL_checkstring(L, 1);
    if(!channel_name)
        return 0;

    lua_pushnumber(L, channelmgr.GetChannel(channel_name, plr)->GetNumMembers());
    return 1;
}

int LuaUnit_GetPlayerMovementVector(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    MovementInfo * movement_info = plr->GetMovementInfo();
    if(movement_info != NULL) 
    {
        float x, y, z, o;
        movement_info->GetPosition(x, y, z, o);
        lua_newtable(L);
        lua_pushstring(L, "x");
        lua_pushnumber(L, x);
        lua_rawset(L, -3);
        lua_pushstring(L, "y");
        lua_pushnumber(L, y);
        lua_rawset(L, -3);
        lua_pushstring(L, "z");
        lua_pushnumber(L, z);
        lua_rawset(L, -3);
        lua_pushstring(L, "o");
        lua_pushnumber(L, o);
        lua_rawset(L, -3);
    }
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit_GetPlayerMovementFlags(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    MovementInfo * move_info = TO_PLAYER(ptr)->GetMovementInfo();
    if (move_info != NULL)
        lua_pushnumber(L, move_info->movementFlags);
    else
        RET_NIL(true)
    return 1;
}

int LuaUnit_Repop(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    if (plr->isDead())
        plr->RepopRequestedPlayer();
    return 1;
}

int LuaUnit_SetMovementFlags(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int movetype = luaL_checkint(L,1); //0: walk, 1: run, 2: fly.
    if (movetype == 2)
    {
        ptr->GetAIInterface()->setMoveFlyFlag(true);
        ptr->GetAIInterface()->setMoveRunFlag(false);
        ptr->GetAIInterface()->setMoveSprintFlag(false);
    }
    else if (movetype == 1)
    {
        ptr->GetAIInterface()->setMoveFlyFlag(false);
        ptr->GetAIInterface()->setMoveRunFlag(true);
        ptr->GetAIInterface()->setMoveSprintFlag(false);
    }
    else
    {
        ptr->GetAIInterface()->setMoveFlyFlag(false);
        ptr->GetAIInterface()->setMoveRunFlag(false);
        ptr->GetAIInterface()->setMoveSprintFlag(false);
    }
    return 1;
}

int LuaUnit_GetSpawnId(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Creature * cre = TO_CREATURE(ptr);
    lua_pushnumber(L,cre->GetSQL_id());
    return 1;
}

int LuaUnit_ResetTalents(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    plr->Reset_Talents();
    return 1;
}

int LuaUnit_SetTalentPoints(lua_State * L, Unit * ptr)
{
    // TODO
/*  TEST_PLAYER();
    uint32 spec = luaL_checkint(L,1); //0 or 1
    uint32 points = luaL_checkint(L,2);
    TO_PLAYER(ptr)->m_specs[spec].m_customTalentPointOverride = points;*/
    return 1;
}

int LuaUnit_GetTalentPoints(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 spec = luaL_checkint(L,1); //0 or 1

    return 1;
}

int LuaUnit_EventChat(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint8 typ = luaL_checkint(L,1);
    uint32 lang = luaL_checkint(L,2);
    const char* message = luaL_checkstring(L,3);
    uint32 delay = luaL_checkint(L,4);
    if (message != NULL && delay)
        sEventMgr.AddEvent(ptr, &Unit::SendChatMessage, typ, lang, message, EVENT_UNIT_CHAT_MSG, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    return 1;
}

int LuaUnit_GetEquippedItemBySlot(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    int16 slot = luaL_checkint(L,1);
    Player * plr = TO_PLAYER(ptr);
    Item * pItem = plr->GetItemInterface()->GetInventoryItem(slot);
    if (pItem && slot >= EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END)
        Lunar<Item>::push(L,pItem);
    else
        lua_pushnil(L);
    return 1;
}

int LuaUnit_GetGuildMembers(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
/*  Player* plr = TO_PLAYER(ptr);
    Guild* pGuild = plr->GetGuild();
    uint32 count = 0;
    lua_newtable(L);
    if(pGuild != NULL)
    {
        map<PlayerInfo*, GuildMember*>::iterator itr;
        pGuild->getLock()->Acquire();
        for(itr = pGuild->GetGuildMembersBegin(); itr != pGuild->GetGuildMembersEnd(); ++itr)
        {
            count++;
            lua_pushinteger(L, count);
            //Paroxysm : Why do we push player names as opposed to objects?
            //hyper: because guild members might not be logged in
            //ret = (*itr).first->m_loggedInPlayer;
            //Lunar<Unit>::push(L, ((Unit*)ret), false);
            lua_pushstring(L, (*itr).first->name);
            lua_rawset(L, -3);
        }
        pGuild->getLock()->Release();
    }
    else*/
        lua_pushnil(L);
    return 1;
}

int LuaUnit_AddAchievement(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    return 1;
}

int LuaUnit_RemoveAchievement(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    return 1;
}

int LuaUnit_HasAchievement(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();

    return 1;
}

int LuaUnit_GetAreaId(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    RET_NUMBER(ptr->GetAreaId());
}

int LuaUnit_ResetPetTalents(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Pet * pet = TO_PLAYER(ptr)->GetSummon();
    if (pet != NULL)
        pet->ResetTalents(false);

    return 1;
}

int LuaUnit_IsDazed(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    lua_pushboolean(L,(ptr->IsDazed())?1:0);
    return 1;
}

int LuaUnit_GetAura(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    uint32 slot = CHECK_ULONG(L,1);
    RET_NUMBER(ptr->m_AuraInterface.GetSpellIdFromAuraSlot(slot));
}

int LuaUnit_GetAuraObject(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    uint32 slot = CHECK_ULONG(L,1);
    if (slot > 0 && slot < MAX_AURAS+MAX_PASSIVE_AURAS)
    {
        Lunar<Aura>::push(L, ptr->m_AuraInterface.FindAuraBySlot(slot));
        return 1;
    }
    RET_NIL(true)
}

int LuaUnit_IsRooted(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    if(ptr->m_rooted)
        RET_BOOL(true)
    RET_BOOL(false)
}

int LuaUnit_HasAuraWithMechanic(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    uint32 mechanic = CHECK_ULONG(L,1);
    if(mechanic && ptr->m_AuraInterface.HasAuraWithMechanic(mechanic) )
        RET_BOOL(true)
    RET_BOOL(false)
}

int LuaUnit_HasNegativeAura(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    for (uint32 x = MAX_POSITIVE_AURAS; x < MAX_AURAS; ++x)
    {
        if (ptr->m_AuraInterface.FindAuraBySlot(x) && ptr->m_AuraInterface.FindAuraBySlot(x)->m_spellProto)
            RET_BOOL(true)
    }
    RET_BOOL(false)
}

int LuaUnit_HasPositiveAura(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    for (uint32 x = 0; x < MAX_POSITIVE_AURAS; ++x)
    {
        if (ptr->m_AuraInterface.FindAuraBySlot(x) && ptr->m_AuraInterface.FindAuraBySlot(x)->m_spellProto)
            RET_BOOL(true)
    }
    RET_BOOL(false)
}

int LuaUnit_SetActionBar(lua_State * L, Unit * ptr) //experimental
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    WorldPacket data(SMSG_PET_SPELLS, 8 * 4 + 20);
    data << plr->GetNewGUID(); //guid
    data << uint16(0); //pet family 
    data << uint32(0); //expire time
    data << uint8(0);   // 0x0 = passive, 0x1 = defensive, 0x2 = aggressive
    data << uint8(0);   // 0x0 = stay, 0x1 = follow, 0x2 = attack
    data << uint16(0);  // flags: 0xFF = disabled pet bar (eg. when pet stunned)

    // Send the spells
    for(uint32 i = 0; i < 10; ++i)
    {
        data << uint16(44781) << uint16(DEFAULT_SPELL_STATE);
    }

    data << uint8(0); //number of spellbook spells
    //spellbook spells would go here
    data << uint8(0); // loop cycles
    plr->delayedPackets.add( &data );
    sLog.Notice("SetActionBar", "Performed function, using delayed packet");
    return 1;
}

int LuaUnit_GetClosestEnemy(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    float closest_dist = 99999.99f;
    float current_dist = 0;
    Object * closest_unit = NULL;
    Unit * ret = NULL;
    for(unordered_set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
    {
        closest_unit = (*itr);
        if(!closest_unit->IsUnit() || !sFactionSystem.isHostile(ptr,closest_unit) )
            continue;
        current_dist = ptr->GetDistance2dSq(closest_unit);
        if(current_dist < closest_dist)
        {
            closest_dist = current_dist;
            ret = TO_UNIT(closest_unit);
        }
    }
    Lunar<Unit>::push(L,ret);
    return 1;
}

int LuaUnit_GetClosestFriend(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    float closest_dist = 99999.99f;
    float current_dist = 0.0f;
    Object * closest_unit = NULL;
    Unit * ret = NULL;
    for (unordered_set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
    {
        closest_unit = (*itr);
        if (!closest_unit->IsUnit() || sFactionSystem.isHostile(closest_unit, ptr))
            continue;
        current_dist = closest_unit->GetDistanceSq(ptr);
        if(current_dist < closest_dist)
        {
            closest_dist = current_dist;
            ret = TO_UNIT(closest_unit);
        }
    }
    Lunar<Unit>::push(L,ret);
    return 1;
}

int LuaUnit_GetObjectType(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    if (ptr->IsPlayer())
        lua_pushstring(L, "Player");
    else
        lua_pushstring(L, "Unit");
    return 1;
}
int LuaUnit_GetCurrentWaypoint(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    RET_NUMBER(ptr->GetAIInterface()->getCurrentWaypoint());
}
int LuaUnit_DisableMelee(lua_State * L, Unit * ptr)
{
    TEST_UNIT_RET()
    bool disable = CHECK_BOOL(L,1);
    TO_CREATURE(ptr)->GetAIInterface()->disable_melee = disable;
    RET_BOOL(true)
}
int LuaUnit_DisableSpells(lua_State * L, Unit * ptr)
{
    TEST_UNIT_RET()
    bool disable = CHECK_BOOL(L,1);
    TO_CREATURE(ptr)->GetAIInterface()->disable_spell = disable;
    RET_BOOL(true)
}
int LuaUnit_DisableRanged(lua_State * L, Unit * ptr)
{
    TEST_UNIT_RET()
    bool disable = CHECK_BOOL(L,1);
    TO_CREATURE(ptr)->GetAIInterface()->disable_ranged = disable;
    RET_BOOL(true)
}
int LuaUnit_DisableCombat(lua_State * L, Unit * ptr)
{
    TEST_UNIT_RET()
    bool disable = CHECK_BOOL(L,1);
    TO_CREATURE(ptr)->GetAIInterface()->disable_combat = disable;
    RET_BOOL(true)
}
int LuaUnit_DisableTargeting(lua_State * L, Unit * ptr)
{
    TEST_UNIT_RET()
    bool disable = CHECK_BOOL(L,1);
    TO_CREATURE(ptr)->GetAIInterface()->disable_targeting = disable;
    RET_BOOL(true)
}
int LuaUnit_IsInGroup(lua_State * L, Unit * ptr)
{
    TEST_PLAYER_RET()
    if(TO_PLAYER(ptr)->InGroup() )
        RET_BOOL(true)
    RET_BOOL(false)
}
int LuaUnit_GetLocation(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    lua_pushnumber(L,ptr->GetPositionX());
    lua_pushnumber(L,ptr->GetPositionY());
    lua_pushnumber(L,ptr->GetPositionZ());
    lua_pushnumber(L,ptr->GetOrientation());
    return 4;
}
int LuaUnit_GetByte(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 index = luaL_checkint(L,1);
    uint32 index2 = luaL_checkint(L,2);
    uint8 value = ptr->GetByte(index,index2);
    RET_INT(value);
}
int LuaUnit_SetByte(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    int index = luaL_checkint(L,1);
    int index2 = luaL_checkint(L,2);
    uint8 value = luaL_checkint(L,3);
    ptr->SetByte(index,index2,value);
    RET_BOOL(true)
}

int LuaUnit_GetSpawnLocation(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    lua_pushnumber(L,ptr->GetSpawnX());
    lua_pushnumber(L,ptr->GetSpawnY());
    lua_pushnumber(L,ptr->GetSpawnZ());
    lua_pushnumber(L,ptr->GetSpawnO());
    return 4;
}
int LuaUnit_GetObject(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    uint64 guid = luaL_checkint(L,1);
    Object * obj = ptr->GetMapMgr()->_GetObject(guid);
    if(obj != NULL && obj->IsUnit() )
        Lunar<Unit>::push(L, TO_UNIT(obj));
    else if(obj != NULL && obj->IsGameObject() )
        Lunar<GameObject>::push(L, TO_GAMEOBJECT(obj));
    else
        lua_pushnil(L);
    return 1;
}
int LuaUnit_GetSecondHated(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Lunar<Unit>::push(L,ptr->GetAIInterface()->GetSecondHated());
    return 1;
}

int LuaUnit_UseAI(lua_State * L, Unit * ptr)
{
    bool check = CHECK_BOOL(L,1);
    if(check)
        ptr->EnableAI();
    else
        ptr->DisableAI();
    return 1;
}
int LuaUnit_FlagFFA(lua_State * L, Unit * ptr)
{

    TEST_UNITPLAYER();
    bool set = CHECK_BOOL(L,1);
    if(set)
        ptr->SetFFAPvPFlag();
    else
        ptr->RemoveFFAPvPFlag();
    return 1;
}
int LuaUnit_TeleportCreature(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    float x = CHECK_FLOAT(L,1);
    float y = CHECK_FLOAT(L,2);
    float z = CHECK_FLOAT(L,3);
    ptr->SetPosition(x,y,z,ptr->GetOrientation());
    WorldPacket data(SMSG_MONSTER_MOVE,50);
    data << ptr->GetNewGUID();
    data << uint8(0);
    data << ptr->GetPositionX() << ptr->GetPositionY() << ptr->GetPositionZ();
    data << getMSTime();
    data << uint8(0x0);
    data << uint32(0x100);
    data << uint32(1) << uint32(1);
    data << x << y << z;
    ptr->SendMessageToSet(&data,false);
    return 1;
}
int LuaUnit_IsInDungeon(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    if(ptr->GetMapMgr()->GetMapInfo() && ptr->GetMapMgr()->GetMapInfo()->type == INSTANCE_MULTIMODE)
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}
int LuaUnit_IsInRaid(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    if(ptr->GetMapMgr()->GetMapInfo() && ptr->GetMapMgr()->GetMapInfo()->type == INSTANCE_RAID)
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}
int LuaUnit_IsHostile(lua_State*  L, Unit * ptr)
{
    Object * B = Lunar<Object>::check(L,1);
    lua_pushboolean(L, sFactionSystem.isHostile(ptr,B));
    return 1;
}
int LuaUnit_IsAttackable(lua_State*  L, Unit * ptr)
{
    Object * B = Lunar<Object>::check(L,1);
    lua_pushboolean(L, sFactionSystem.isAttackable(ptr,B));
    return 1;
}
int LuaUnit_GetNumWaypoints(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    RET_NUMBER(ptr->GetAIInterface()->GetWayPointsCount());
    return 1;
}
int LuaUnit_GetMovementType(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    RET_NUMBER(ptr->GetAIInterface()->getMoveType());
    return 1;
}
int LuaUnit_GetQuestLogSlot(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 entry = CHECK_ULONG(L,1);
    QuestLogEntry * qle = TO_PLAYER(ptr)->GetQuestLogForEntry(entry);
    if (!qle)
        RET_NUMBER(-1);
    lua_pushnumber(L, qle->GetSlot());
    return 1;
}

int LuaUnit_GetAuraStackCount(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    uint32 id = CHECK_ULONG(L,1);
    Aura* aur = ptr->m_AuraInterface.FindAura(id);
    if(aur)
        RET_NUMBER(aur->stackSize);

    lua_pushnumber(L,(0));
    return 1;
}

int LuaUnit_AddAuraObject(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    Aura * aura = Lunar<Aura>::check(L,1);
    if(aura == NULL)
        return 0;

    ptr->AddAura(aura);
    return 1;
}

int LuaUnit_GetAuraObjectById(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 id = CHECK_ULONG(L,1);
    Lunar<Aura>::push(L, ptr->m_AuraInterface.FindAura(id));
    return 1;
}

int LuaUnit_GetNativeFaction(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET()
    int32 faction = 35;
    if (ptr->IsPlayer())
    {
        Player* plr = TO_PLAYER(ptr);
        PlayerCreateInfo * pci = objmgr.GetPlayerCreateInfo(plr->getRace(), plr->getClass());
        if( pci )
            faction = pci->factiontemplate;
    }
    else
    {
        if (TO_CREATURE(ptr)->GetProto())
            faction = TO_CREATURE(ptr)->GetProto()->Faction;
        else
            faction = ptr->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE);
    }
    RET_INT(faction)
}

int LuaUnit_RemoveFlag(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    int field = luaL_checkint(L,1);
    int value = luaL_checkint(L,2);
    ptr->RemoveFlag(field,value);
    return 1;
}

int LuaUnit_SetMount(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    uint32 DsplId = CHECK_ULONG(L, 1);
    ptr->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, DsplId);
    return 1;
}

int LuaUnit_StartQuest(lua_State * L, Unit * ptr)
{
    TEST_PLAYER_RET()
    int quest_id = luaL_checkint(L,1);
    Player *plr = TO_PLAYER(ptr);
    if (plr->HasFinishedQuest(quest_id))
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    Quest * qst = sQuestMgr.GetQuestPointer(quest_id);
    if (qst)
    {
        QuestLogEntry * IsPlrOnQuest = plr->GetQuestLogForEntry(quest_id);
        if (IsPlrOnQuest) 
        {
            lua_pushnumber(L, 1);
            return 1;
        }
        else
        {
            int32 open_slot = plr->GetOpenQuestSlot();
            if (open_slot == -1)
            {
                sQuestMgr.SendQuestLogFull(plr);
                lua_pushnumber(L, 2);
                return 1;
            }
            else
            {
                QuestLogEntry *qle = new QuestLogEntry();
                qle->Init(qst, plr, (uint32)open_slot);
                qle->UpdatePlayerFields();

                // If the quest should give any items on begin, give them the items.
                for(uint32 i = 0; i < 4; i++)
                {
                    if(qst->receive_items[i])
                    {
                        Item *item = objmgr.CreateItem( qst->receive_items[i], plr);
                        if(item == NULL)
                            return false;

                        item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, uint32(qst->receive_itemcount[i]));
                        if(!plr->GetItemInterface()->AddItemToFreeSlot(item))
                            item->DeleteMe();
                    }
                }

                if(qst->srcitem && qst->srcitem != qst->receive_items[0])
                {
                    Item * item = objmgr.CreateItem( qst->srcitem, plr);
                    if(item)
                    {
                        item->SetUInt32Value( ITEM_FIELD_STACK_COUNT, (qst->srcitemcount ? qst->srcitemcount : 1));
                        if(!plr->GetItemInterface()->AddItemToFreeSlot(item))
                            item->DeleteMe();
                    }
                }

                sHookInterface.OnQuestAccept( plr, qst, NULL );
                lua_pushnumber(L, 3);
                return 1;
            }
        }
    }
    else return 0;
    
    lua_pushnumber(L, 999);
    return 1;
} //StartQuest

int LuaUnit_FinishQuest(lua_State * L, Unit * ptr)
{
    TEST_PLAYER_RET()
    int quest_id = luaL_checkint(L,1);
    Player *plr = TO_PLAYER(ptr);
    Quest * qst = sQuestMgr.GetQuestPointer(quest_id);
    if (qst)
    {
        if (plr->HasFinishedQuest(quest_id)) 
        {
            lua_pushnumber(L, 0);
            return 1;
        }
        else
        {
            QuestLogEntry * IsPlrOnQuest = plr->GetQuestLogForEntry(quest_id);
            if (IsPlrOnQuest)
            {   
                sQuestMgr.GenerateQuestXP(plr, qst);
                sQuestMgr.BuildQuestComplete(plr, qst);

                IsPlrOnQuest->Finish();
                plr->AddToFinishedQuests(quest_id);
                lua_pushnumber(L, 1);
                return 1;
            }
            else
            {
                lua_pushnumber(L, 2);
                return 1;
            }       
        }
    }
    else
        return 0;
}

int LuaUnit_GetDisplay(lua_State * L, Unit * ptr)
{
    if( ptr == NULL )
        lua_pushinteger( L, 0 );
    else
        lua_pushinteger( L, ptr->GetUInt32Value(UNIT_FIELD_DISPLAYID) );

    return 1;
}

int LuaUnit_GetNativeDisplay(lua_State * L, Unit * ptr)
{
    if( ptr == NULL )
        lua_pushinteger( L, 0 );
    else
        lua_pushinteger( L, ptr->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID) );

    return 1;
}

int LuaUnit_GetGameTime(lua_State * L, Unit * ptr)
{
    lua_pushnumber(L, ((uint32)sWorld.GetGameTime())); //in seconds.
    return 1;
}

int LuaUnit_PlaySoundToPlayer(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    int soundid = luaL_checkint(L,1);
    Player* plr = TO_PLAYER(ptr);
    WorldPacket data;
    data.Initialize(SMSG_PLAY_OBJECT_SOUND);
    data << uint32(soundid) << plr->GetGUID();
    plr->GetSession()->SendPacket(&data);
    return 1;
}

int LuaUnit_GetDuelState(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player* plr = TO_PLAYER(ptr);
    lua_pushnumber(L,plr->GetDuelState());
    return 1;
}

int LuaUnit_SetPosition(lua_State * L, Unit * ptr)
{
    float x = CHECK_FLOAT(L,1);
    float y = CHECK_FLOAT(L,2);
    float z = CHECK_FLOAT(L,3);
    float o = CHECK_FLOAT(L,4);
    ptr->SetFacing(o);
    ptr->SetOrientation(o);
    WorldPacket data(SMSG_MONSTER_MOVE, 50);
    data << ptr->GetNewGUID();
    data << uint8(0);
    data << ptr->GetPositionX();
    data << ptr->GetPositionY();
    data << ptr->GetPositionZ();
    data << getMSTime();
    data << uint8(0x00);
    data << uint32(256);
    data << uint32(1);
    data << uint32(1);
    data << x << y << z;

    ptr->SendMessageToSet(&data, true);
    ptr->SetPosition(x,y,z,o);
    return 1;
}

int LuaUnit_GetLandHeight(lua_State * L, Unit * ptr)
{
    /*float x = CHECK_FLOAT(L,1);
    float y = CHECK_FLOAT(L,2);
    if (!ptr || !x || !y) 
        return 0;
    float lH = ptr->GetMapMgr()->GetLandHeight(x,y);
    lua_pushnumber(L, lH);*/
    return 1;
}

int LuaUnit_QuestAddStarter(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Creature * unit = TO_CREATURE(ptr);
    uint32 quest_id = (uint32)luaL_checknumber(L, 1);
    if (!unit->HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_QUESTGIVER))
        unit->SetUInt32Value(UNIT_NPC_FLAGS, unit->GetUInt32Value(UNIT_NPC_FLAGS)+UNIT_NPC_FLAG_QUESTGIVER);
    if(!quest_id)
        return 0;
    Quest * qst = sQuestMgr.GetQuestPointer(quest_id);
    if (!qst)
        return 0;

    uint32 quest_giver = unit->GetEntry();
        
    char my_query1 [200];
    sprintf(my_query1,"SELECT id FROM creature_quest_starter WHERE id = %d AND quest = %d", quest_giver, quest_id);
    QueryResult *selectResult1 = WorldDatabase.Query(my_query1);
    if (selectResult1)
        delete selectResult1; //already has quest
    else
    {
        char my_insert1 [200];
        sprintf(my_insert1, "INSERT INTO creature_quest_starter (id, quest) VALUES (%d,%d)", quest_giver, quest_id);
        WorldDatabase.Execute(my_insert1);
    }
    sQuestMgr.LoadQuests();
    QuestRelation *qstrel = new QuestRelation;
    qstrel->qst = qst;
    qstrel->type = QUESTGIVER_QUEST_START;
    uint8 qstrelid;
    if ( unit->HasQuests() )
    {
        qstrelid = (uint8)unit->GetQuestRelation(quest_id);
        unit->DeleteQuest(qstrel);
    }
    unit->_LoadQuests();
    return 1;
}

int LuaUnit_QuestAddFinisher(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    Creature * unit = TO_CREATURE(ptr);
    uint32 quest_id = CHECK_ULONG(L, 1);
    if (!unit->HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_QUESTGIVER))
        unit->SetUInt32Value(UNIT_NPC_FLAGS, unit->GetUInt32Value(UNIT_NPC_FLAGS)+UNIT_NPC_FLAG_QUESTGIVER);
    if(!quest_id)
        return 0;
    Quest * qst = sQuestMgr.GetQuestPointer(quest_id);
    if (!qst)
        return 0;
    uint32 quest_giver = unit->GetEntry();
    
    char my_query1 [200];
    sprintf(my_query1,"SELECT id FROM creature_quest_finisher WHERE id = %d AND quest = %d", quest_giver, quest_id);
    QueryResult *selectResult1 = WorldDatabase.Query(my_query1);
    if (selectResult1)
    {
        delete selectResult1; //already has quest
    }
    else
    {
        char my_insert1 [200];
        sprintf(my_insert1, "INSERT INTO creature_quest_finisher (id, quest) VALUES (%d,%d)", quest_giver, quest_id);
        WorldDatabase.Execute(my_insert1);
    }
    sQuestMgr.LoadQuests();
    QuestRelation *qstrel = new QuestRelation;
    qstrel->qst = qst;
    qstrel->type = QUESTGIVER_QUEST_END;
    uint8 qstrelid;
    if ( unit->HasQuests() )
    {
        qstrelid = (uint8)unit->GetQuestRelation(quest_id);
        unit->DeleteQuest(qstrel);
    }
    unit->_LoadQuests();
    return 1;
}

int LuaUnit_SetPlayerSpeed(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    float Speed = CHECK_FLOAT(L,1);
    if (Speed<1 || Speed>255)
        return 0;
    plr->SetPlayerSpeed(RUN, Speed);
    plr->SetPlayerSpeed(SWIM, Speed);
    plr->SetPlayerSpeed(RUNBACK, Speed / 2);
    plr->SetPlayerSpeed(FLY, Speed * 2);
    return 1;
}

int LuaUnit_GiveHonor(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    uint32 honor = CHECK_ULONG(L,1);
    plr->m_honorToday += honor;
    plr->m_honorPoints += honor;
    plr->RecalculateHonor();
    return 1;
}

int LuaUnit_SetBindPoint(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    float z = CHECK_FLOAT(L, 3);
    uint32 map = CHECK_ULONG(L, 4);
    uint32 zone = CHECK_ULONG(L, 5);
    if (!x || !y || !z || !zone)
        return 0;
    plr->SetBindPoint(x, y, z, map, zone);
    return 1;
}

int LuaUnit_SoftDisconnect(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    TO_PLAYER(ptr)->SoftDisconnect();
    return 1;
}

int LuaUnit_GetInventoryItem(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    int8 containerslot = luaL_checkint(L, 1);
    int16 slot = luaL_checkint(L, 2);
    Player * plr = TO_PLAYER(ptr);
    Lunar<Item>::push(L, plr->GetItemInterface()->GetInventoryItem(containerslot, slot));
    return 1;
}

int LuaUnit_GetInventoryItemById(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 entry = CHECK_ULONG(L, 1);
    Player * plr = TO_PLAYER(ptr);
    int16 slot = plr->GetItemInterface()->GetInventorySlotById(entry);
    if (slot == -1) //check bags
    {
        for (uint8 contslot = INVENTORY_SLOT_BAG_START; contslot != INVENTORY_SLOT_BAG_END; contslot++)
        {
            Container * bag = static_cast<Container*>(plr->GetItemInterface()->GetInventoryItem(contslot));
            for (uint8 bslot = 0; bslot != bag->GetSlotCount(); bslot++)
            {
                if (bag->GetItem(bslot) && bag->GetItem(bslot)->GetEntry() == entry)
                {
                    Lunar<Item>::push(L, bag->GetItem(bslot));
                    return 1;
                }
            }
        }
    }
    Lunar<Item>::push(L, plr->GetItemInterface()->GetInventoryItem(slot));
    return 1;
}

int LuaUnit_PhaseSet(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 newphase = CHECK_ULONG(L,1);
    bool Save = (luaL_optint(L, 2, false) > 0 ? true : false); 

    switch (ptr->GetTypeId())
    {
    case TYPEID_UNIT:
        {
            Creature* crt = TO_CREATURE(ptr);
            crt->SetPhaseMask(newphase);
            if (crt->m_spawn)
                crt->m_spawn->phase = newphase;

            if(Save)
            {
                crt->SaveToDB();
                crt->m_loadedFromDB = true;
            }
        }break;

    case TYPEID_PLAYER:
        {
            TO_PLAYER(ptr)->SetPhaseMask(newphase);
        }break;

    default:
        break;
    }
    return 1;
}

int LuaUnit_PhaseAdd(lua_State * L, Unit * ptr)
{   // EnablePhase
    TEST_UNITPLAYER();
    uint32 newphase = CHECK_ULONG(L,1);
    bool Save = (luaL_optint(L, 2, false) > 0 ? true : false); 

    switch (ptr->GetTypeId())
    {
    case TYPEID_UNIT:
        {
            Creature* crt = TO_CREATURE(ptr);
            crt->EnablePhase(newphase);
            if (crt->m_spawn)
                crt->m_spawn->phase |= newphase;

            if(Save)
            {
                crt->SaveToDB();
                crt->m_loadedFromDB = true;
            }
        }break;

    case TYPEID_PLAYER:
        {
            TO_PLAYER(ptr)->EnablePhase(newphase);
        }break;

    default:
        break;
    }
    return 1;
}

int LuaUnit_PhaseDelete(lua_State * L, Unit * ptr)
{   // DisablePhase
    TEST_UNITPLAYER();
    uint32 newphase = CHECK_ULONG(L,1);
    bool Save = (luaL_optint(L, 2, false) > 0 ? true : false); 

    switch (ptr->GetTypeId())
    {
    case TYPEID_UNIT:
        {
            Creature* crt = TO_CREATURE(ptr);
            crt->DisablePhase(newphase);
            if (crt->m_spawn)
                crt->m_spawn->phase &= newphase;

            if(Save)
            {
                crt->SaveToDB();
                crt->m_loadedFromDB = true;
            }
        }break;

    case TYPEID_PLAYER:
        {
            TO_PLAYER(ptr)->DisablePhase(newphase);
        }break;

    default:
        break;
    }

    return 1;
}

int LuaUnit_GetPhaseMask(lua_State * L, Unit * ptr);

int LuaUnit_GetPhase(lua_State * L, Unit * ptr)
{   // GetPhase
    printf("Use of GetPhase. Use GetPhaseMask in the future.");
    return LuaUnit_GetPhaseMask(L, ptr);
}

int LuaUnit_GetPhaseMask(lua_State * L, Unit * ptr)
{   // GetPhaseMask
    TEST_UNITPLAYER();
    lua_pushnumber(L, ptr->GetPhaseMask());
    return 1;
}

int LuaUnit_IsInPhase(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER();
    uint32 phase = luaL_checkint(L,1);
    lua_pushboolean(L, ptr->IsInPhase(phase) ? 1 : 0);
    return 1;
}

int LuaUnit_SetZoneWeather(lua_State * L, Unit * ptr)
{
    /*
    WEATHER_TYPE_NORMAL            = 0, // NORMAL (SUNNY)
    WEATHER_TYPE_FOG               = 1, // FOG
    WEATHER_TYPE_RAIN              = 2, // RAIN
    WEATHER_TYPE_HEAVY_RAIN        = 4, // HEAVY_RAIN
    WEATHER_TYPE_SNOW              = 8, // SNOW
    WEATHER_TYPE_SANDSTORM         = 16 // SANDSTORM
    */
    uint32 zone_id = CHECK_ULONG(L, 1);
    uint32 type = CHECK_ULONG(L, 2);
    float Density = CHECK_FLOAT(L, 3); //min: 0.30 max: 2.00
    if (Density<0.30f || Density>2.0f || !zone_id || !type)
        return 0;
    uint32 sound;
    if(Density<=0.30f)
        sound = 0;
    switch(type)
    {
        case 2:                                             //rain
        case 4:                                             
            if(Density  <0.40f)
                 sound = 8533;
            else if(Density  <0.70f)
                sound = 8534;
            else
                sound = 8535;
            break;
        case 8:                                             //snow
            if(Density  <0.40f)
                sound = 8536;
            else if(Density  <0.70f)
                sound = 8537;
            else
                sound = 8538;
            break;
        case 16:                                             //storm
            if(Density  <0.40f)
                sound = 8556;
            else if(Density  <0.70f)
                sound = 8557;
            else
                sound = 8558;
            break;
        default:                                            //no sound
            sound = 0;
            break;
    }
    WorldPacket data(SMSG_WEATHER, 9);
    data.Initialize(SMSG_WEATHER);
    if(type == 0 ) // set all parameter to 0 for sunny.
        data << uint32(0) << float(0) << uint32(0) << uint8(0);     
    else if (type == 1) // No sound/density for fog
        data << type << float(0) << uint32(0) << uint8(0);      
    else
        data << type << Density << sound << uint8(0) ;
    sWorld.SendZoneMessage(&data, zone_id, 0);
    return 1;
}

int LuaUnit_SetPlayerWeather(lua_State * L, Unit * ptr)
{
    /*
    WEATHER_TYPE_NORMAL            = 0, // NORMAL (SUNNY)
    WEATHER_TYPE_FOG               = 1, // FOG
    WEATHER_TYPE_RAIN              = 2, // RAIN
    WEATHER_TYPE_HEAVY_RAIN        = 4, // HEAVY_RAIN
    WEATHER_TYPE_SNOW              = 8, // SNOW
    WEATHER_TYPE_SANDSTORM         = 16 // SANDSTORM
    */
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    uint32 type = CHECK_ULONG(L, 1);
    float Density = CHECK_FLOAT(L, 2); //min: 0.30 max: 2.00
    if (Density<0.30f || Density>2.0f || !type)
        return 0;
    uint32 sound;
    if(Density<=0.30f)
        sound = 0;
    switch(type)
    {
        case 2:                                             //rain
        case 4:                                             
            if(Density  <0.40f)
                 sound = 8533;
            else if(Density  <0.70f)
                sound = 8534;
            else
                sound = 8535;
            break;
        case 8:                                             //snow
            if(Density  <0.40f)
                sound = 8536;
            else if(Density  <0.70f)
                sound = 8537;
            else
                sound = 8538;
            break;
        case 16:                                             //storm
            if(Density  <0.40f)
                sound = 8556;
            else if(Density  <0.70f)
                sound = 8557;
            else
                sound = 8558;
            break;
        default:                                            //no sound
            sound = 0;
            break;
    }
    WorldPacket data(SMSG_WEATHER, 9);
    data.Initialize(SMSG_WEATHER);
    if(type == 0 ) // set all parameter to 0 for sunny.
        data << uint32(0) << float(0) << uint32(0) << uint8(0);     
    else if (type == 1) // No sound/density for fog
        data << type << float(0) << uint32(0) << uint8(0);      
    else
        data << type << Density << sound << uint8(0) ;
    plr->GetSession()->SendPacket(&data);
    return 1;
}

int LuaUnit_SendPacketToPlayer(lua_State * L, Unit * ptr)
{
    WorldPacket * data = CHECK_PACKET(L,1);
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    if (data)
        plr->GetSession()->SendPacket(data);
    return 1;
}

int LuaUnit_PlayerSendChatMessage(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    uint32 type = CHECK_ULONG(L,1);
    uint32 lang = CHECK_ULONG(L,2);
    const char * msg = luaL_checklstring(L, 3, NULL);
    Player * plr = TO_PLAYER(ptr);
    if(msg == NULL || !plr)
        return 0;
    WorldPacket *data = sChatHandler.FillMessageData( type, lang, msg, plr->GetGUID(), 0 );
    plr->GetSession()->SendChatPacket(data, 1, lang, plr->GetSession());
    for(unordered_set<Player * >::iterator itr = plr->GetInRangePlayerSetBegin(); itr != plr->GetInRangePlayerSetEnd(); ++itr)
    {
        (static_cast< Player* >(*itr))->GetSession()->SendChatPacket(data, 1, lang, plr->GetSession());
    }
    return 1;
}

int LuaUnit_AggroWithInRangeFriends(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    // If Pointer isn't in combat skip everything
    if (!ptr->CombatStatus.IsInCombat())
        return 0;

    Unit * pTarget = ptr->GetAIInterface()->GetNextTarget();
    if (pTarget == NULL)
        return 0;

    Unit * pUnit = NULL;
    for(unordered_set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); ++itr)
    {
        Object * obj = TO_OBJECT(*itr);
        // No Object, Object Isn't a Unit, Unit is Dead
        if (!obj || !obj->IsUnit() || TO_UNIT(obj)->isDead())
            continue;

         if (!sFactionSystem.isFriendly(obj, ptr))
            continue;

        if (ptr->GetDistance2dSq(obj) > 10*10) // 10yrd range?
            continue;

         pUnit = TO_UNIT(obj);
        if (!pUnit) // Should never happen!
            continue;

        pUnit->GetAIInterface()->SetNextTarget(pTarget);
        pUnit->GetAIInterface()->AttackReaction(pTarget, 1, 0);
    }
    return 1;
}

int LuaUnit_GetDistanceYards(lua_State * L, Unit * ptr)
{
    Object * target = CHECK_OBJECT(L, 1);
    if(!ptr || !target)
        return 0;
    LocationVector vec = ptr->GetPosition();
    lua_pushnumber(L,(float)vec.Distance(target->GetPosition()));
    return 1;
}

int LuaUnit_MoveRandomArea(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    float x1 = CHECK_FLOAT(L, 1);
    float y1 = CHECK_FLOAT(L, 2);
    float z1 = CHECK_FLOAT(L, 3);
    float x2 = CHECK_FLOAT(L, 4);
    float y2 = CHECK_FLOAT(L, 5);
    float z2 = CHECK_FLOAT(L, 6);
    ptr->GetAIInterface()->MoveTo( x1+(RandomFloat(x2-x1)), y1+(RandomFloat(y2-y1)), z1+(RandomFloat(z2-z1)));
    return 1;
}

int LuaUnit_SendPacketToGroup(lua_State * L, Unit * ptr)
{
    WorldPacket * data = CHECK_PACKET(L,1);
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    if (!data)
        return 0;

    plr->GetGroup()->SendPacketToAll(data);
    return 1;
}

int LuaUnit_GetGroupPlayers(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * _player = TO_PLAYER(ptr);
    Group* party = _player->GetGroup();
    uint32 count = 0;
    lua_newtable(L);
    if (party)
    {
        GroupMembersSet::iterator itr;
        SubGroup * sgrp;
        party->getLock().Acquire();
        for(uint32 i = 0; i < party->GetSubGroupCount(); i++)
        {
            sgrp = party->GetSubGroup(i);
            for(itr = sgrp->GetGroupMembersBegin(); itr != sgrp->GetGroupMembersEnd(); ++itr)
            {
                if((*itr)->m_loggedInPlayer && (*itr)->m_loggedInPlayer->GetZoneId() == _player->GetZoneId() && _player->GetInstanceID() == (*itr)->m_loggedInPlayer->GetInstanceID())
                {
                    count++,
                    lua_pushinteger(L,count);
                    Lunar<Unit>::push(L,(*itr)->m_loggedInPlayer);
                    lua_rawset(L,-3);
                }
            }
        }
        party->getLock().Release();
    }
    return 1;
}

int LuaUnit_GetDungeonDifficulty(lua_State * L, Unit * ptr)
{   /*
    MODE_NORMAL_10MEN   =   0,
    MODE_NORMAL_25MEN   =   1,
    MODE_HEROIC_10MEN   =   2,
    MODE_HEROIC_25MEN   =   3
    */
    if (ptr->GetTypeId() == TYPEID_PLAYER)
    {
        Player * plr = TO_PLAYER(ptr);
        if (plr->GetGroup())
        {
            if (plr->GetGroup()->GetGroupType() == GROUP_TYPE_PARTY)
                lua_pushnumber(L,plr->GetGroup()->GetDifficulty());
            else
                lua_pushnumber(L,plr->GetGroup()->GetRaidDifficulty());
        }
        else
        {
            if (!plr->IsInInstance())
                return 0;
            Instance * pInstance = sInstanceMgr.GetInstanceByIds(plr->GetMapId(), plr->GetInstanceID());
            lua_pushinteger(L,pInstance->m_difficulty);
        }
        return 1;
    }
    else
    {
        if (!ptr->IsInInstance())
        {   
            lua_pushboolean(L,0);
            return 1;
        }
        Instance * pInstance = sInstanceMgr.GetInstanceByIds(ptr->GetMapId(), ptr->GetInstanceID());
        lua_pushinteger(L,pInstance->m_difficulty);
    }
    return 1;
}

int LuaUnit_GetInstanceOwner(lua_State * L, Unit * ptr)
{
    if (!ptr) return 0;
    if (!ptr->IsInInstance())
        lua_pushnil(L);
    else 
    {
        Instance * pInstance = sInstanceMgr.GetInstanceByIds(ptr->GetMapId(), ptr->GetInstanceID());
        if (pInstance->m_creatorGuid != 0) // creator guid is 0 if its owned by a group.
        {
            Player * owner = pInstance->m_mapMgr->GetPlayer(pInstance->m_creatorGuid);
            Lunar<Unit>::push(L,owner);
        }
        else
        {
            uint32 gId = pInstance->m_creatorGroup;
            Lunar<Unit>::push(L,objmgr.GetGroupById(gId)->GetLeader()->m_loggedInPlayer);
        }
    }
    return 1;
}

int LuaUnit_IsGroupFull(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    lua_pushboolean(L,plr->GetGroup()->IsFull() ? 1 : 0);
    return 1;
}

int LuaUnit_GetGroupLeader(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Lunar<Unit>::push(L,plr->GetGroup()->GetLeader()->m_loggedInPlayer);
    return 1;
}

int LuaUnit_SetGroupLeader(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * _plr = CHECK_PLAYER(L,1);
    bool silent = CHECK_BOOL(L,2);
    Player * plr = TO_PLAYER(ptr);
    plr->GetGroup()->SetLeader(_plr, silent);
    return 1;
}

int LuaUnit_AddGroupMember(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    Player * _plr = CHECK_PLAYER(L,1);
    int32 subgroup = luaL_optint(L,2,-1);
    plr->GetGroup()->AddMember(_plr->getPlayerInfo(),subgroup);
    return 1;
}

int LuaUnit_SetDungeonDifficulty(lua_State * L, Unit * ptr)
{   /*
    MODE_NORMAL_10MEN   =   0,
    MODE_NORMAL_25MEN   =   1,
    MODE_HEROIC_10MEN   =   2,
    MODE_HEROIC_25MEN   =   3
    */
    TEST_UNIT();

    uint32 difficulty = CHECK_ULONG(L,1);
    if (ptr->IsInInstance()) 
    {
        if (ptr->IsPlayer())
        {
            Player * plr = TO_PLAYER(ptr);
            if (plr->GetGroup())(difficulty > 1 ? plr->GetGroup()->m_difficulty : plr->GetGroup()->m_raiddifficulty) = difficulty;
            else
            {
                Instance * pInstance = sInstanceMgr.GetInstanceByIds(plr->GetMapId(), plr->GetInstanceID());
                pInstance->m_difficulty = difficulty;
            }
        }
        else
        {
            Instance * pInstance = sInstanceMgr.GetInstanceByIds(ptr->GetMapId(), ptr->GetInstanceID());
            pInstance->m_difficulty = difficulty;
        }
    }
    return 1;
}

int LuaUnit_ExpandToRaid(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    plr->GetGroup()->ExpandToRaid();
    return 1;
}

int LuaUnit_CanAttack(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    Unit * target = CHECK_UNIT(L,1);
    if (!target) return 0;
    if (sFactionSystem.isAttackable(ptr, target))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaUnit_GetInRangeEnemies(lua_State * L, Unit * ptr)
{
    /*uint32 count = 0;
    lua_newtable(L);
    for( set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); itr++)
    {
        if( (*itr) ->IsUnit() && !isFriendly(ptr, (*itr)) )
        {
            count++,
            lua_pushinteger(L,count);
            Lunar<Unit>::push(L,*itr);
            lua_rawset(L,-3);
        }
    }*/
    return 1;
}

int LuaUnit_GetInRangeUnits(lua_State * L, Unit * ptr)
{
/*  uint32 count = 0;
    lua_newtable(L);
    for( set<Object*>::iterator itr = ptr->GetInRangeSetBegin(); itr != ptr->GetInRangeSetEnd(); itr++)
    {
        if( (*itr) ->IsUnit() )
        {
            count++,
            lua_pushinteger(L,count);
            Lunar<Unit>::push(L,*itr);
            lua_rawset(L,-3);
        }
    }*/
    return 1;
}

int LuaUnit_HasFlag(lua_State * L, Unit * ptr)
{
    TEST_UNITPLAYER_RET();
    uint32 index = CHECK_ULONG(L,1);
    uint32 flag = CHECK_ULONG(L,2);
    lua_pushboolean(L, ptr->HasFlag(index,flag) ? 1 : 0);
    return 1;
}

int LuaUnit_TakeHonor(lua_State * L, Unit * ptr)
{
    TEST_PLAYER();
    Player * plr = TO_PLAYER(ptr);
    uint32 honor = CHECK_ULONG(L,1);
    if (plr->m_honorToday-honor < 0 || plr->m_honorPoints-honor < 0)
        return 0;
    plr->m_honorToday -= honor;
    plr->m_honorPoints -= honor;
    plr->RecalculateHonor();
    return 1;
}

int LuaUnit_GetPower(lua_State * L, Unit * ptr)
{
    if (!ptr)
    {
        lua_pushnil(L);
        return 1;
    }
    int powertype = luaL_optint(L, 1, -1);
    if (powertype == -1)
        powertype = ptr->GetPowerType();
    lua_pushnumber(L, ptr->GetPower(powertype));
    return 1;
}

int LuaUnit_GetMaxPower(lua_State * L, Unit * ptr)
{
    if (!ptr)
    {
        lua_pushnil(L);
        return 1;
    }
    int powertype = luaL_optint(L, 1, -1);
    if (powertype == -1)
        powertype = ptr->GetPowerType();
    lua_pushnumber(L, ptr->GetMaxPower(powertype));
    return 1;
}

int LuaUnit_SetPowerType(lua_State * L, Unit * ptr)
{/* POWER_TYPE_MANA         = 0,
    POWER_TYPE_RAGE         = 1,
    POWER_TYPE_FOCUS        = 2,
    POWER_TYPE_ENERGY       = 3,
    POWER_TYPE_HAPPINESS    = 4,
    POWER_TYPE_RUNES        = 5,
    POWER_TYPE_RUNIC_POWER  = 6 */
    TEST_UNIT();

    int type = luaL_checkint(L, 1);
    if(type < 0)
        return 0;

    ptr->SetPowerType(type);
    return 1;
}

int LuaUnit_SetMaxPower(lua_State * L, Unit * ptr)
{
    TEST_UNIT();

    int amount = luaL_checkint(L,1);
    int powertype = luaL_optint(L, 2, -1);
    if (amount < 0)
        return 0;

    if (powertype == -1)
        powertype = ptr->GetPowerType();

    ptr->SetMaxPower(powertype, amount);
    return 1;
}

int LuaUnit_SetPower(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int amount = luaL_checkint(L,1);
    int powertype = luaL_optint(L, 2, -1);
    if (amount < 0)
        return 0;

    if (powertype == -1)
        powertype = ptr->GetPowerType();

    ptr->SetPower(powertype, amount);
    return 1;
}

int LuaUnit_SetPowerPct(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    int amount = luaL_checkint(L,1);
    int powertype = luaL_optint(L, 2, -1);
    if (amount < 0)
        return 0;

    if (powertype == -1)
        powertype = ptr->GetPowerType();

    ptr->SetPower( powertype, (int)(amount/100) * (ptr->GetMaxPower(powertype)) );
    return 1;
}

int LuaUnit_GetPowerType(lua_State * L, Unit * ptr)
{
    TEST_UNIT();
    lua_pushinteger(L, ptr->GetPowerType());
    return 1;
}

int LuaUnit_GetPowerPct(lua_State * L, Unit * ptr)
{
    if (!ptr)
    {
        lua_pushnil(L);
        return 1;
    }
    int powertype = luaL_optint(L, 1, -1);
    if (powertype == -1)
        powertype = ptr->GetPowerType();
    lua_pushnumber(L, (int)(ptr->GetPower(powertype) * 100.0f / ptr->GetMaxPower(powertype)));
    return 1;
}

int LuaUnit_LearnSpells(lua_State * L, Unit * ptr)
{
    TEST_PLAYER_RET()
    if(!strcmp("table",luaL_typename(L,1)) )
    {
        int table = lua_gettop(L);
        lua_pushnil(L);
        while(lua_next(L,table) != 0)
        {
            if(lua_isnumber(L,-1) )
                TO_PLAYER(ptr)->addSpell(CHECK_ULONG(L,-1));
            lua_pop(L,1);
        }
        lua_pushboolean(L,1);
        lua_replace(L,table);
        lua_settop(L,table);
    }
    else
    {
        lua_settop(L,0);
        lua_pushboolean(L,0);
    }
    return 1;
}
