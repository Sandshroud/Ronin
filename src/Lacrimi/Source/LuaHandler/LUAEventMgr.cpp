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

bool lLuaEventMgr::HasEvent(int ref)
{
    hash_map<int, EventInfoHolder*>::iterator itr = g_luaMgr.m_registeredTimedEvents.find(ref);
    return (itr != g_luaMgr.m_registeredTimedEvents.end());
}

bool lLuaEventMgr::HasEventInTable(const char * table)
{
    hash_map<int, EventInfoHolder*>::iterator itr = g_luaMgr.m_registeredTimedEvents.begin();
    for (; itr != g_luaMgr.m_registeredTimedEvents.end(); ++itr)
    {
        if (strncmp(itr->second->funcName, table, strlen(table)) == 0)
        {
            return true;
        }
    }
    return false;
}

bool lLuaEventMgr::HasEventWithName(const char * name)
{
    hash_map<int, EventInfoHolder*>::iterator itr = g_luaMgr.m_registeredTimedEvents.begin();
    for (; itr != g_luaMgr.m_registeredTimedEvents.end(); ++itr)
    {
        if (strcmp(itr->second->funcName, name) == 0)
        {
            return true;
        }
    }
    return false;
}

void lLuaEventMgr::RemoveEventsInTable(const char * table)
{
    hash_map<int, EventInfoHolder*>::iterator itr = g_luaMgr.m_registeredTimedEvents.begin(), itr2;
    for (; itr != g_luaMgr.m_registeredTimedEvents.end(); )
    {
        itr2 = itr++;
        if (strncmp(itr2->second->funcName, table, strlen(table)) == 0)
        {
            event_RemoveByPointer(itr2->second->te);
            free((void*)itr2->second->funcName);
            luaL_unref(g_luaMgr.GLuas(), LUA_REGISTRYINDEX, itr2->first);
            g_luaMgr.m_registeredTimedEvents.erase(itr2);
        }
    }
}

void lLuaEventMgr::RemoveEventsByName(const char * name)
{
    hash_map<int, EventInfoHolder*>::iterator itr = g_luaMgr.m_registeredTimedEvents.begin(), itr2;
    for (; itr != g_luaMgr.m_registeredTimedEvents.end(); )
    {
        itr2 = itr++;
        if (strcmp(itr2->second->funcName, name) == 0)
        {
            event_RemoveByPointer(itr2->second->te);
            free((void*)itr2->second->funcName);
            luaL_unref(g_luaMgr.GLuas(), LUA_REGISTRYINDEX, itr2->first);
            g_luaMgr.m_registeredTimedEvents.erase(itr2);
        }
    }
}

void lLuaEventMgr::RemoveEventByRef(int ref)
{
    hash_map<int, EventInfoHolder*>::iterator itr = g_luaMgr.m_registeredTimedEvents.find(ref);
    if (itr != g_luaMgr.m_registeredTimedEvents.end())
    {
        event_RemoveByPointer(itr->second->te);
        free((void*)itr->second->funcName);
        luaL_unref(g_luaMgr.GLuas(), LUA_REGISTRYINDEX, itr->first);
        g_luaMgr.m_registeredTimedEvents.erase(itr);
    }
}

void lLuaEventMgr::RemoveEvents()
{
    event_RemoveEvents(EVENT_LUA_TIMED);
    hash_map<int, EventInfoHolder*>::iterator itr = g_luaMgr.m_registeredTimedEvents.begin();
    for (; itr != g_luaMgr.m_registeredTimedEvents.end(); ++itr)
    {
        free((void*)itr->second->funcName);
        luaL_unref(g_luaMgr.GLuas(), LUA_REGISTRYINDEX, itr->first);
    }
    g_luaMgr.m_registeredTimedEvents.clear();
}
