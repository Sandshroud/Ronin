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

#ifndef __LUAEVENT_MGR_H
#define __LUAEVENT_MGR_H

class lLuaEventMgr : public EventableObject
{
public:
    bool HasEvent(int ref);
    bool HasEventInTable(const char * table);
    bool HasEventWithName(const char * name);
    void RemoveEventsInTable(const char * table);
    void RemoveEventsByName(const char * name);
    void RemoveEventByRef(int ref);
    void RemoveEvents();
};

#endif
