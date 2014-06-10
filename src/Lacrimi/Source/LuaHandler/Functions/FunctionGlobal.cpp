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

int LuaGlobalFunctions_PerformIngameSpawn(lua_State * L)
{
    uint32 spawntype = luaL_checkint(L, 1);
    uint32 entry = luaL_checkint(L, 2);
    uint32 map = luaL_checkint(L, 3);
    float x = CHECK_FLOAT(L, 4);
    float y = CHECK_FLOAT(L, 5);
    float z = CHECK_FLOAT(L, 6);
    float o = CHECK_FLOAT(L, 7);
    uint32 faction = luaL_checkint(L, 8); //also scale as percentage
    uint32 duration = luaL_checkint(L, 9);
    uint32 equip1 = luaL_optint(L, 10, 1);
    uint32 equip2 = luaL_optint(L, 11, 1);
    uint32 equip3 = luaL_optint(L, 12, 1);
    //13: instance id
    uint32 save = luaL_optint(L, 14, 0);
    if(x && y && z && entry)
    {
        if (spawntype == 1) //Unit
        { 
            CreatureProto *p = CreatureProtoStorage.LookupEntry(entry);
            CreatureInfo *i = CreatureNameStorage.LookupEntry(entry);
            if (p == NULL || i == NULL)
                RET_NIL(true);

            MapMgr *mapMgr = sInstanceMgr.GetMapMgr(map);
            if (mapMgr == NULL)
                RET_NIL(true);

            int32 instanceid = luaL_optint(L, 13, mapMgr->GetInstanceID());
            CreatureSpawn * sp = new CreatureSpawn();
            sp->entry = entry;
            sp->id = objmgr.GenerateCreatureSpawnID();
            sp->x = x;
            sp->y = y;
            sp->z = z;
            sp->o = o;
            sp->emote_state = 0;
            sp->flags = 0;
            sp->factionid = faction;
            sp->stand_state = 0;
            sp->phase = 1;
            sp->vehicle = p->vehicle_entry > 0 ? true : false;
            sp->Bytes = NULL;
            sp->ChannelData = NULL;
            sp->MountedDisplay = NULL;

            Creature * pCreature = NULL;
            if(sp->vehicle)
            {
                pCreature = TO_CREATURE(mapMgr->CreateVehicle(entry));
                TO_VEHICLE(pCreature)->Load(sp, mapMgr->iInstanceMode, NULL);
            }
            else
            {
                pCreature = mapMgr->CreateCreature(entry);
                pCreature->Load(sp, mapMgr->iInstanceMode, NULL);
            }

            pCreature->m_loadedFromDB = true;
            pCreature->SetFaction(faction);
            pCreature->SetInstanceID(instanceid);
            pCreature->SetMapId(map);
            pCreature->m_noRespawn = true;
            pCreature->PushToWorld(mapMgr);
            if (duration>0) 
                pCreature->Despawn(duration,0);
            if (save)
                pCreature->SaveToDB();
            Lunar<Unit>::push(L,TO_UNIT(pCreature));
        }
        else if (spawntype == 2) //GO
        { 
            GameObjectInfo *n = GameObjectNameStorage.LookupEntry(entry);
            if (n == NULL)
                RET_NIL(true);

            MapMgr *mapMgr = sInstanceMgr.GetMapMgr(map);
            if (mapMgr == NULL)
                RET_NIL(true);

            int32 instanceid = luaL_optint(L, 13, mapMgr->GetInstanceID());

            GameObject *go = mapMgr->CreateGameObject(entry);
            go->SetInstanceID(instanceid);
            go->CreateFromProto(entry,map,x,y,z,o);

            // Create spawn instance
            GOSpawn * gs = new GOSpawn;
            gs->entry = go->GetEntry();
            gs->facing = go->GetOrientation();
            gs->faction = go->GetFaction();
            gs->flags = go->GetUInt32Value(GAMEOBJECT_FLAGS);
            gs->id = objmgr.GenerateGameObjectSpawnID();
            gs->scale = go->GetUInt32Value(OBJECT_FIELD_SCALE_X);
            gs->x = go->GetPositionX();
            gs->y = go->GetPositionY();
            gs->z = go->GetPositionZ();
            gs->state = go->GetByte(GAMEOBJECT_BYTES_1, 0);
            gs->phase = 0;

            go->m_spawn = gs;
            go->PushToWorld(mapMgr);
            if (duration)
                sEventMgr.AddEvent(go,&GameObject::ExpireAndDelete,EVENT_GAMEOBJECT_UPDATE,duration,1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            if (save)
                go->SaveToDB();
            Lunar<GameObject>::push(L,go);
        }
        else
            RET_NIL(true);
    }
    else
        RET_NIL(true);
    return 1;
}

int LuaGlobalFunctions_GetGameTime(lua_State * L)
{
    lua_pushnumber(L, ((uint32)sWorld.GetGameTime())); //in seconds.
    return 1;
}

int LuaGlobalFunctions_WorldDBQuery(lua_State * L)
{
    const char * qStr = luaL_checkstring(L,1);
    if (qStr == NULL)
        RET_NIL(true);

    uint32 fID = luaL_optint(L,2,0); //column
    uint32 rID = luaL_optint(L,3,0); //row
    QueryResult * result = WorldDatabase.Query(qStr);
    if(result == NULL)
        RET_NIL(true);

    lua_settop(L,0);
    Lunar<QueryResult>::push(L,result);
    return 1;
}

int LuaGlobalFunctions_CharDBQuery(lua_State * L)
{
    const char * qStr = luaL_checkstring(L,1);
    if (qStr == NULL)
        RET_NIL(true);

    uint32 fID = luaL_optint(L,2,0); //column
    uint32 rID = luaL_optint(L,3,0); //row
    QueryResult* result = CharacterDatabase.Query(qStr);
    if(result == NULL)
        RET_NIL(true);

    lua_settop(L,0);
    Lunar<QueryResult>::push(L,result);
    return 1;
}

int LuaGlobalFunctions_WorldDBQueryTable(lua_State * L)
{
    const char * qStr = luaL_checkstring(L,1);
    if (qStr == NULL)
        RET_NIL(true);

    lua_newtable(L);
    QueryResult* result = WorldDatabase.Query(qStr);
    if(result == NULL)
        RET_NIL(true);

    Lunar<QueryResult>::push(L,result);
    return 1;
}

int LuaGlobalFunctions_CharDBQueryTable(lua_State * L)
{
    const char * qStr = luaL_checkstring(L,1);
    if (qStr == NULL)
        RET_NIL(true);

    lua_newtable(L);
    QueryResult* result = CharacterDatabase.Query(qStr);
    if(result == NULL)
        RET_NIL(true);

    Lunar<QueryResult>::push(L,result);
    return 1;
}

int LuaGlobalFunctions_GetPlayer(lua_State * L)
{
    const char * plName = luaL_checkstring(L,1);
    Player * plr = objmgr.GetPlayer(plName);
    if (plr == NULL || !plr->IsInWorld())
        RET_NIL(true);

    Lunar<Unit>::push(L, TO_UNIT(plr));
    return 1;
}

int LuaGlobalFunctions_GetLUAEngine(lua_State * L) //also mapped to GetLuaEngine()
{
    lua_pushstring(L, ENGINE_NAME);
    return 1;
}

int LuaGlobalFunctions_GetLuaEngineVersion(lua_State * L)
{
    lua_pushnumber(L, ENGINE_VERSION);
    return 1;
}

/*int LuaGlobalFunctions_WorldDBQuery(lua_State * L)
{
    const char * qStr = luaL_checkstring(L,1);
    uint32 fID = luaL_optint(L,2,0); //column
    uint32 rID = luaL_optint(L,3,0); //row
    if (!qStr) return 0;
    QueryResult * result = WorldDatabase.Query(qStr);
    lua_settop(L,0);
    PUSH_SQLRESULT(L,result);
    return 1;
}

int LuaGlobalFunctions_CharDBQuery(lua_State * L)
{
    const char * qStr = luaL_checkstring(L,1);
    uint32 fID = luaL_optint(L,2,0); //column
    uint32 rID = luaL_optint(L,3,0); //row
    if (!qStr) return 0;
    QueryResult * result = CharacterDatabase.Query(qStr);
    lua_settop(L,0);
    PUSH_SQLRESULT(L,result);
    return 1;
}

int LuaGlobalFunctions_WorldDBQueryTable(lua_State * L)
{
    const char * qStr = luaL_checkstring(L,1);
    lua_newtable(L);
    if(!qStr) return 0;
    QueryResult * result = WorldDatabase.Query(qStr);
    PUSH_SQLRESULT(L,result);
    return 1;
}

int LuaGlobalFunctions_CharDBQueryTable(lua_State * L)
{
    const char * qStr = luaL_checkstring(L,1);
    lua_newtable(L);
    if(!qStr) return 0;
    QueryResult * result = CharacterDatabase.Query(qStr);
    PUSH_SQLRESULT(L,result);
    return 1;
}*/

int LuaGlobalFunctions_SendWorldMessage(lua_State * L)
{
    const char * message = luaL_checkstring(L,1);
    uint32 MsgType = luaL_checkint(L,2);
    if(!message || !MsgType)
        RET_NIL(true);

    if (MsgType == 1)
        sWorld.SendWorldWideScreenText(message);
    else if (MsgType == 2)
        sWorld.SendWorldText(message);
    return 1;
}

int LuaGlobalFunctions_ReloadTable(lua_State * L)
{
    const char * TableName = luaL_checkstring(L,1);
    if (!stricmp(TableName, "spell_disable"))
    {
        objmgr.ReloadDisabledSpells();
    } 
    else if (!stricmp(TableName, "vendors"))
    {
        objmgr.ReloadVendors();
    }
    else
    {
        if (!stricmp(TableName, "creature_proto"))     // Creature Proto
            CreatureProtoStorage.Reload();
        else if (!stricmp(TableName, "creature_names"))     // Creature Names
            CreatureNameStorage.Reload();
        else if (!stricmp(TableName, "gameobject_names"))   // GO Names
            GameObjectNameStorage.Reload();
        else if (!stricmp(TableName, "areatriggers"))       // Areatriggers
            AreaTriggerStorage.Reload();
        else if (!stricmp(TableName, "itempages"))          // Item Pages
            ItemPageStorage.Reload();
        else if (!stricmp(TableName, "npc_text"))           // NPC Text Storage
            NpcTextStorage.Reload();
        else if (!stricmp(TableName, "fishing"))                // Fishing Zones
            FishingZoneStorage.Reload();
        else if(!stricmp(TableName, "teleport_coords"))     // Teleport coords
            TeleportCoordStorage.Reload();
        else if (!stricmp(TableName, "graveyards"))         // Graveyards
            GraveyardStorage.Reload();
        else if (!stricmp(TableName, "worldmap_info"))      // WorldMapInfo
            WorldMapInfoStorage.Reload();
        else if (!stricmp(TableName, "zoneguards"))
            ZoneGuardStorage.Reload();
        else if (!stricmp(TableName, "command_overrides"))  // Command Overrides
        {
            sComTableStore.Dealloc();
            sComTableStore.Init();
            sComTableStore.Load();
        }
        else
            RET_NIL(true);
    }

    return 1;
}

int LuaGlobalFunctions_ReloadLuaEngine(lua_State * L)
{
/*  g_luaMgr.Restart();
    MapMgr* mgr = NULL;
    LuaUnitBinding* m_binding;
    for (uint32 i = 0; i < NUM_MAPS; ++i)
    {
        if ((mgr = sInstanceMgr.GetMapMgr(i)) != NULL)
            continue;

        for(uint32 guid=1; guid < mgr->m_CreatureArraySize; guid++)
        {
            Creature *pCreature = mgr->GetCreature(GET_LOWGUID_PART(guid));
            if(pCreature)
            {
                m_binding = g_luaMgr.GetUnitBinding(pCreature->GetEntry());
                if (m_binding != NULL)
                    g_engine->OnUnitEvent( pCreature, m_binding->Functions[CREATURE_EVENT_ON_LOAD], CREATURE_EVENT_ON_LOAD, NULL, 0, 0, 0, 0, NULL );       
            }
        }
        mgr = NULL;
    }*/
    return 1;
}

int LuaGlobalFunctions_GetPlayersInWorld(lua_State * L)
{
    Player* ret = NULL;
    uint32 count = 0;
    lua_newtable(L);
    objmgr._playerslock.AcquireReadLock();
    HM_NAMESPACE::hash_map<uint32, Player*>::const_iterator itr;
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        count++,
        ret = (*itr).second;
        lua_pushinteger(L,count);
        Lunar<Unit>::push(L,((Unit*)ret));
        lua_rawset(L,-3);
    }
    objmgr._playerslock.ReleaseReadLock();  
    return 1;
}

int LuaGlobalFunctions_Rehash(lua_State * L)
{
    sWorld.Rehash(false);
    return 1;
}

int LuaGlobalFunctions_GetSandshroudRevision(lua_State * L)
{
    lua_pushnumber(L,BUILD_REVISION);
    return 1;
}

int LuaGlobalFunctions_GetSandshroudBuildHash(lua_State * L)
{
    lua_pushnumber(L,BUILD_HASH);
    return 1;
}

//////////////////////////////////////////////////////////////////////////
// WORLD PVP NOT SUPPORTED!
//////////////////////////////////////////////////////////////////////////
int LuaGlobalFunctions_SendPvPCaptureMessage(lua_State * L)
{
    uint32 zoneid = luaL_checkint(L, 1);
    const char* msg = luaL_checkstring(L, 2);
    AreaTableEntry * at = dbcAreaTable.LookupEntry(zoneid);
    if(!zoneid || !msg || !at)
        return 1;

    MapMgr* mapmgr = sInstanceMgr.GetMapMgr(at->mapId);
    if (mapmgr != NULL)
        mapmgr->SendPvPCaptureMessage(ZONE_MASK_ALL, zoneid, msg);
    return 1;
}

int LuaGlobalFunctions_GetPlayersInMap(lua_State * L)
{
    Player * ret = NULL;
    uint32 count = 0;
    lua_newtable(L);
    uint32 mapid = luaL_checkint(L,1);
    MapMgr * mgr = sInstanceMgr.GetMapMgr(mapid);
    if (mgr == NULL)
        RET_NIL(true);

    for (PlayerStorageMap::iterator itr = mgr->m_PlayerStorage.begin(); itr != mgr->m_PlayerStorage.end(); ++itr)
    {
        count++,
        ret = (*itr).second;
        lua_pushinteger(L,count);
        Lunar<Unit>::push(L,((Unit*)ret));
        lua_rawset(L,-3);
    }

    if(!count)
        RET_NIL(true);
    return 1;
}

int LuaGlobalFunctions_GetPlayersInZone(lua_State * L)
{
    Player * ret = NULL;
    uint32 count = 0;
    lua_newtable(L);
    uint32 zoneid = luaL_checkint(L,1);
    objmgr._playerslock.AcquireReadLock();
    HM_NAMESPACE::hash_map<uint32, Player*>::const_iterator itr;
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        if ((*itr).second->GetZoneId() == zoneid)
        {
            count++,
            ret = (*itr).second;
            lua_pushinteger(L,count);
            Lunar<Unit>::push(L,((Unit*)ret));
            lua_rawset(L,-3);
        }
    }
    objmgr._playerslock.ReleaseReadLock();
    if(!count)
        RET_NIL(true);
    return 1;
}

int LuaGlobalFunctions_SendMail(lua_State * L)
{
    uint32 type = luaL_checkint(L,1);
    uint64 sender_guid = GuidMgr::check(L,2);
    uint64 recipient_guid = GuidMgr::check(L,3);
    string subject = luaL_checkstring(L,4);
    string body = luaL_checkstring(L,5);
    uint32 money = luaL_checkint(L,6);
    uint32 cod = luaL_checkint(L,7);
    uint64 item_guid = GuidMgr::check(L,8);
    uint32 stationery = luaL_checkint(L,9);
    sMailSystem.DeliverMessage(type, sender_guid, recipient_guid, subject, body, money, cod, item_guid, stationery, false);
    return 1;
}

int LuaGlobalFunctions_GetTaxiPath(lua_State * L)
{
    uint32 path = luaL_checkint(L, 1);
    TaxiPath * tp = sTaxiMgr.GetTaxiPath(path);
    if(tp == NULL)
        RET_NIL(true);

    Lunar<TaxiPath>::push(L, tp);
    return 1;
}

int LuaGlobalFunctions_SetDBCSpellVar(lua_State * L)
{
    uint32 entry = luaL_checkinteger(L,1);
    const char* var = luaL_checkstring(L,2);
    int subindex = 0;
    if (lua_gettop(L) == 4)
        subindex = luaL_optint(L,3,0);

    int valindex = 3;
    if (subindex)
        valindex++;

    SpellEntry * proto = dbcSpell.LookupEntry(entry);
    if (!entry || !var || subindex < 0 || !proto)
        RET_BOOL(false);

    LuaSpellEntry l = GetLuaSpellEntryByName(var);
    if (!l.name)
        RET_BOOL(false);

    switch (l.typeId) //0: int, 1: char*, 2: bool, 3: float
    {
    case 0:
        GET_SPELLVAR_INT(proto,l.offset,subindex) = luaL_checkinteger(L, valindex);
        lua_pushboolean(L, 1);
        break;
    case 1:
        strcpy(GET_SPELLVAR_CHAR(proto,l.offset,subindex), luaL_checkstring(L, valindex));
        lua_pushboolean(L, 1);
        break;
    case 2:
        GET_SPELLVAR_BOOL(proto,l.offset,subindex) = CHECK_BOOL(L, valindex);
        lua_pushboolean(L, 1);
        break;
    case 3:
        GET_SPELLVAR_FLOAT(proto,l.offset,subindex) = (float)luaL_checknumber(L, valindex);
        lua_pushboolean(L, 1);
        break;
    }
    return 1;
}

int LuaGlobalFunctions_GetDBCSpellVar(lua_State * L)
{
    uint32 entry = luaL_checkinteger(L,1);
    const char* var = luaL_checkstring(L,2);
    int subindex = luaL_optint(L,3,0);
    SpellEntry * proto = dbcSpell.LookupEntry(entry);
    if (!entry || !var || subindex < 0 || !proto)
        RET_NIL(true);

    LuaSpellEntry l = GetLuaSpellEntryByName(var);
    if (!l.name)
        RET_NIL(true);

    switch (l.typeId) //0: int, 1: char*, 2: bool, 3: float
    {
    case 0:
        lua_pushinteger(L, GET_SPELLVAR_INT(proto,l.offset,subindex));
        break;
    case 1:
        lua_pushstring(L, GET_SPELLVAR_CHAR(proto,l.offset,subindex));
        break;
    case 2:
        lua_pushboolean(L, (GET_SPELLVAR_BOOL(proto,l.offset,subindex)) ? 1 : 0);
        break;
    case 3:
        lua_pushnumber(L, GET_SPELLVAR_FLOAT(proto,l.offset,subindex));
        break;
    }
    return 1;
}

int LuaGlobalFunctions_bit_and(lua_State *L)
{
    uint32 left = CHECK_ULONG(L,1);
    int top = lua_gettop(L);
    if(top > 1)
    {
        for(int i = 2; i <= top; ++i)
        {
            if(lua_isnumber(L,i) )
                left &= CHECK_ULONG(L,i);
        }
    }
    RET_NUMBER(left)
}

int LuaGlobalFunctions_bit_or(lua_State * L)
{
    uint32 left = CHECK_ULONG(L,1);
    int top = lua_gettop(L);
    if(top > 1)
    {
        for(int i = 2; i <= top; ++i)
        {
            if(lua_isnumber(L,i) )
                left |= CHECK_ULONG(L,i);
        }
    }
    RET_NUMBER(left)
}

int LuaGlobalFunctions_bit_xor(lua_State * L)
{
    uint32 left = CHECK_ULONG(L,1);
    int top = lua_gettop(L);
    if(top > 1)
    {
        for(int i = 2; i <= top; ++i)
        {
            if(lua_isnumber(L,i) )
                left ^= CHECK_ULONG(L,i);
        }
    }
    RET_NUMBER(left)
}

int LuaGlobalFunctions_bit_not(lua_State * L)
{
    uint32 left = CHECK_ULONG(L,1);
    RET_NUMBER(~left)
}

int LuaGlobalFunctions_bit_shiftleft(lua_State * L)
{
    uint32 left = CHECK_ULONG(L,1);
    uint8 count = luaL_checkinteger(L,2) & 0x7F;
    RET_NUMBER(left << count)
}

int LuaGlobalFunctions_bit_shiftright(lua_State * L)
{
    uint32 left = CHECK_ULONG(L,1);
    uint8 count = luaL_checkinteger(L,2) & 0x7F;
    RET_NUMBER(left >> count)
}

int LuaGlobalFunctions_RemoveTimedEvents(lua_State * L)
{
    LuaEvent.RemoveEvents();
    return 1;
}

int LuaGlobalFunctions_RemoveTimedEventsWithName(lua_State * L)
{
    const char* name = luaL_checkstring(L,1);
    LuaEvent.RemoveEventsByName(name);
    return 1;
}

int LuaGlobalFunctions_RemoveTimedEvent(lua_State * L)
{
    int ref = luaL_checkint(L,1);
    LuaEvent.RemoveEventByRef(ref);
    return 1;
}

int LuaGlobalFunctions_RemoveTimedEventsInTable(lua_State * L)
{
    const char* table = luaL_checkstring(L,1);
    LuaEvent.RemoveEventsInTable(table);
    return 1;
}

int LuaGlobalFunctions_HasTimedEvents(lua_State * L)
{
    lua_pushboolean(L, LuaEvent.event_HasEvents() ? 1 : 0);
    return 1;
}

int LuaGlobalFunctions_HasTimedEvent(lua_State * L)
{
    int ref = luaL_checkint(L,1);
    lua_pushboolean(L, LuaEvent.HasEvent(ref) ? 1 : 0);
    return 1;
}

int LuaGlobalFunctions_HasTimedEventWithName(lua_State * L)
{
    const char * name = luaL_checkstring(L,1);
    lua_pushboolean(L, LuaEvent.HasEventWithName(name) ? 1 : 0);
    return 1;
}

int LuaGlobalFunctions_HasTimedEventInTable(lua_State * L)
{
    const char * table = luaL_checkstring(L,1);
    lua_pushboolean(L, LuaEvent.HasEventInTable(table) ? 1 : 0);
    return 1;
}

int LuaGlobalFunctions_GetPlatform(lua_State * L)
{
    lua_pushliteral(L,PLATFORM_TEXT);
    return 1;
}

int LuaGlobalFunctions_NumberToGUID(lua_State * L)
{
    uint64 num = (uint64)luaL_checknumber(L,1);
    GuidMgr::push(L, num);
    return 1;
}

int LuaGlobalFunctions_SendPacketToZone(lua_State * L)
{
    WorldPacket * data = CHECK_PACKET(L,1);
    uint32 zone_id = CHECK_ULONG(L,2);
    if (data && zone_id)
        sWorld.SendZoneMessage(data,zone_id);
    return 1;
}

int LuaGlobalFunctions_SendPacketToInstance(lua_State * L)
{
    WorldPacket * data = CHECK_PACKET(L,1);
    uint32 instance_id = CHECK_ULONG(L, 2);
    if (data && instance_id)
        sWorld.SendInstanceMessage(data,instance_id);
    return 1;
}

int LuaGlobalFunctions_SendPacketToWorld(lua_State * L)
{
    WorldPacket * data = CHECK_PACKET(L,1);
    if (data)
        sWorld.SendGlobalMessage(data);
    return 1;
}

int LuaGlobalFunctions_SendPacketToChannel(lua_State * L)
{
    WorldPacket * pack = CHECK_PACKET(L,1);
    const char* channel_name = luaL_checkstring(L,2);
    uint32 team = CHECK_ULONG(L,3);
    Channel* pChannel = channelmgr.GetChannel(channel_name, team);
    if (!pChannel || !pack)
        return 0;

    pChannel->SendToAll(pack);
    return 1;
}

int LuaGlobalFunctions_ToLower(lua_State * L)
{
    std::string oldstring = luaL_checkstring(L,1);
    if (!oldstring.size())
        RET_NIL(true);

    lua_pushstring(L, HEARTHSTONE_TOLOWER_RETURN(oldstring).c_str());
    return 1;
}

int LuaGlobalFunctions_SetSanctuaryArea(lua_State * L)
{
    uint32 areaid = CHECK_ULONG(L,1);
    if (!areaid)
        return 0;

    sWorld.SetSanctuaryArea(areaid);
    return 1;
}

int LuaGlobalFunctions_RemoveSanctuaryArea(lua_State * L)
{
    uint32 areaid = CHECK_ULONG(L,1);
    if (!areaid)
        return 0;

    sWorld.RemoveSanctuaryArea(areaid);
    return 1;
}

int LuaGlobalFunctions_IsSanctuaryArea(lua_State * L)
{
    uint32 areaid = CHECK_ULONG(L,1);
    if (!areaid)
        RET_BOOL(false);

    lua_pushboolean(L, sWorld.IsSanctuaryArea(areaid) ? 1 : 0);
    return 1;
}
