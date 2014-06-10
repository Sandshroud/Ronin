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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

SCRIPT_DECL Creature* CreateAndLoadCreature(
    MapMgr* mgr, uint32 entry, uint32 instancemode, float x, float y, float z, float o, int32 phase = 1, bool push = true);

SCRIPT_DECL GameObject* CreateAndLoadGameObject(MapMgr* mgr, uint32 entry, float x, float y, float z, float o,
                                                int32 phase = 1, bool push = true);

SCRIPT_DECL void WaypointCreate(Creature *pCreature , float x, float y, float z, float o, uint32 waittime, uint32 flags, uint32 modelid);

SCRIPT_DECL void EnableWaypoints(Creature *creat);

SCRIPT_DECL void DeleteWaypoints(Creature *creat);

SCRIPT_DECL void DeleteSpawned(Creature *creat);

SCRIPT_DECL void GameobjectDelete(GameObject *pC, uint32 duration);

SCRIPT_DECL void MoveToPlayer(Player *plr, Creature *creat);

SCRIPT_DECL void CreateCustomWaypointMap(Creature *creat);

SCRIPT_DECL bool AddItem( uint32 pEntry, Player *pPlayer, uint32 pCount = 1 );

SCRIPT_DECL void EventCreatureDelete(Creature *creat, uint32 time);

SCRIPT_DECL void EventCastSpell(Unit *caster, Unit *target, uint32 spellid, uint32 time);

SCRIPT_DECL void EventPlaySound(Creature *creat, uint32 id, uint32 time);

SCRIPT_DECL void EventCreatureSay(Creature *creat, string say, uint32 time);

SCRIPT_DECL void EventCreatureYell(Creature *creat, string say, uint32 time);

SCRIPT_DECL Creature *getNpcQuester(Player *plr, uint32 npcid);

SCRIPT_DECL GameObject *getGOQuester(Player *plr, uint32 goid);

SCRIPT_DECL QuestLogEntry* GetQuest( Player* pPlayer, uint32 pQuestId );

SCRIPT_DECL void KillMobForQuest( Player* pPlayer, QuestLogEntry* pQuest, uint32 pRequiredMobCount );

SCRIPT_DECL void KillMobForQuest( Player* pPlayer, uint32 pQuestId, uint32 pRequiredMobCount );

#endif // FUNCTIONS_H
