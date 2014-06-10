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

int LuaAura_GetObjectType(lua_State * L, Aura * aura)
{
    TEST_AURA_RET_NULL();
    lua_pushstring(L, "Aura");
    return 1;
}

int LuaAura_GetSpellId(lua_State * L, Aura * aura)
{
    TEST_AURA_RET_NULL();
    lua_pushnumber(L, aura->GetSpellId());
    return 1;
}

int LuaAura_GetCaster(lua_State * L, Aura * aura)
{
    TEST_AURA_RET_NULL();
    Object* caster = aura->GetCaster();
    if(caster == NULL)
        RET_NIL(true);

    if (caster->IsUnit()) //unit caster
    {
        Lunar<Unit>::push(L, TO_UNIT(caster));
        return 1;
    }
    else if (caster->IsGameObject()) //gameobject
    {
        Lunar<GameObject>::push(L, TO_GAMEOBJECT(caster));
        return 1;
    }
    else if (caster->IsItem()) //item
    {
        Lunar<Item>::push(L, TO_ITEM(caster));
        return 1;
    }

    RET_NIL(true);
}

int LuaAura_GetTarget(lua_State * L, Aura * aura)
{
    TEST_AURA_RET_NULL();
    Lunar<Unit>::push(L, TO_UNIT(aura->GetTarget()));
    return 1;
}

int LuaAura_GetDuration(lua_State * L, Aura * aura)
{
    TEST_AURA_RET_NULL();
    RET_NUMBER(aura->GetDuration()); //in milliseconds
}

int LuaAura_SetDuration(lua_State * L, Aura * aura)
{
    TEST_AURA();
    int32 duration = luaL_checkint(L,1);
    aura->SetDuration(duration);
    sEventMgr.ModifyEventTimeLeft(aura, EVENT_AURA_REMOVE, duration);
    return 1;
}

int LuaAura_GetTimeLeft(lua_State * L, Aura * aura)
{
    TEST_AURA_RET_NULL();
    RET_NUMBER(aura->GetTimeLeft()); //in milliseconds
}

int LuaAura_SetNegative(lua_State * L, Aura * aura)
{
    TEST_AURA_RET();
    uint32 negativery = luaL_optint(L, 1, 1);
    aura->SetNegative(negativery);
    RET_BOOL(true);
}

int LuaAura_SetPositive(lua_State * L, Aura * aura)
{
    TEST_AURA_RET();
    uint32 positivery = luaL_optint(L, 1, 1);
    aura->SetPositive(positivery);
    RET_BOOL(true);
}

int LuaAura_Remove(lua_State * L, Aura * aura)
{
    TEST_AURA();
    aura->Remove();
    return 1;
}

int LuaAura_SetVar(lua_State * L, Aura * aura)
{
    TEST_AURA_RET();
    const char* var = luaL_checkstring(L,1);
    int subindex = 0;
    if (lua_gettop(L) == 3)
        subindex = luaL_optint(L,2,0);

    if (!var || subindex < 0)
        RET_BOOL(false);

    int valindex = 2;
    if(subindex)
        valindex++;

    SpellEntry * proto = aura->m_spellProto;
    if(proto == NULL)
        RET_NIL(true);

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

int LuaAura_GetVar(lua_State * L, Aura * aura)
{
    TEST_AURA_RET_NULL();
    const char* var = luaL_checkstring(L,1);
    int subindex = luaL_optint(L,2,0);
    if (!var || subindex < 0) 
        RET_NIL(true);

    SpellEntry * proto = aura->m_spellProto;
    if(proto == NULL)
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

int LuaAura_GetAuraSlot(lua_State * L, Aura * aura)
{
    TEST_AURA_RET_NULL();
    RET_INT(aura->GetAuraSlot());
}

int LuaAura_SetAuraSlot(lua_State * L, Aura * aura)
{
    TEST_AURA();
    uint16 slot = CHECK_USHORT(L,1);
    aura->SetAuraSlot(slot);
    return 1;
}
