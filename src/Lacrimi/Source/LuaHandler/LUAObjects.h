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

#ifndef __LUAOBJECTS_H
#define __LUAOBJECTS_H

class LuaGossip : public GossipScript
{
public:
    LuaGossip() : GossipScript(), m_go_gossip_binding(NULL),m_item_gossip_binding(NULL),m_unit_gossip_binding(NULL) { LuaScript = true; }
    ~LuaGossip();

    void GossipHello(Object* pObject, Player* Plr, bool AutoSend);
    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char * EnteredCode);
    void GossipEnd(Object* pObject, Player* Plr);

    LuaUnitGossipBinding * m_unit_gossip_binding;
    LuaItemGossipBinding * m_item_gossip_binding;
    LuaGOGossipBinding * m_go_gossip_binding;
};

class LuaInstance : public MapManagerScript
{
public:
    LuaInstance(MapMgr* pMapMgr) : MapManagerScript(pMapMgr), m_instanceId(pMapMgr->GetInstanceID()) {}
    ~LuaInstance() {}

    // Player
    void OnPlayerDeath(Player* pVictim, Unit* pKiller);

    // Area and AreaTrigger
    void OnPlayerEnter(Player* pPlayer);
    void OnAreaTrigger(Player* pPlayer, uint32 uAreaId);
    void OnZoneChange(Player* pPlayer, uint32 uNewZone, uint32 uOldZone);

    // Creature / GameObject - part of it is simple reimplementation for easier use Creature / GO < --- > Script
    void OnCreatureDeath(Creature* pVictim, Unit* pKiller);
    void OnCreaturePushToWorld(Creature* pCreature);
    void OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer);
    void OnGameObjectPushToWorld(GameObject* pGameObject);

    // Standard virtual methods
    void OnLoad();
    void Destroy();

    uint32 m_instanceId;
    LuaInstanceBinding* m_binding;
};

class LuaCreature : public CreatureAIScript
{
public:
    LuaCreature(Creature* creature) : CreatureAIScript(creature) { LuaScript = true; };
    ~LuaCreature() {};

    void OnCombatStart(Unit* mTarget);
    void OnCombatStop(Unit* mTarget);
    void OnTargetDied(Unit* mTarget);
    void OnDied(Unit *mKiller);
    void OnTargetParried(Unit* mTarget);
    void OnTargetDodged(Unit* mTarget);
    void OnTargetBlocked(Unit* mTarget, int32 iAmount);
    void OnTargetCritHit(Unit* mTarget, int32 fAmount);
    void OnParried(Unit* mTarget);
    void OnDodged(Unit* mTarget);
    void OnBlocked(Unit* mTarget, int32 iAmount);
    void OnCritHit(Unit* mTarget, int32 fAmount);
    void OnHit(Unit* mTarget, float fAmount);
    void OnAssistTargetDied(Unit* mAssistTarget);
    void OnFear(Unit* mFeared, uint32 iSpellId);
    void OnFlee(Unit* mFlee);
    void OnCallForHelp();
    void OnLoad();
    void OnReachWP(uint32 iWaypointId, bool bForwards);
    void OnLootTaken(Player* pPlayer, ItemPrototype *pItemPrototype);
    void AIUpdate(MapManagerScript*, uint32 ptime);
    void OnEmote(Player * pPlayer, EmoteType Emote);
    void OnDamageTaken(Unit* mAttacker, float fAmount);
    void StringFunctionCall(int fRef);
    void Destroy();

    LuaUnitBinding * m_binding;
};

class LuaGameObject : public GameObjectAIScript
{
public:
    LuaGameObject(GameObject * go) : GameObjectAIScript(go) { LuaScript = true; }
    ~LuaGameObject() {}

    void OnCreate();
    void OnSpawn();
    void OnDespawn();
    void OnLootTaken(Player * pLooter, ItemPrototype *pItemInfo);
    void OnActivate(Player * pPlayer);
    void AIUpdate(MapManagerScript*, uint32 ptime);
    void Destroy ();

    LuaGameObjectBinding * m_binding;
};

class LuaQuest : public QuestScript
{
public:
    LuaQuest() : QuestScript() { LuaScript = true; }
    ~LuaQuest() {}

    void OnQuestStart(Player* mTarget, QuestLogEntry *qLogEntry);
    void OnQuestComplete(Player* mTarget, QuestLogEntry *qLogEntry);
    void OnQuestCancel(Player* mTarget);
    void OnGameObjectActivate(uint32 entry, Player* mTarget, QuestLogEntry *qLogEntry);
    void OnCreatureKill(uint32 entry, Player* mTarget, QuestLogEntry *qLogEntry);
    void OnExploreArea(uint32 areaId, Player* mTarget, QuestLogEntry *qLogEntry);
    void OnPlayerItemPickup(uint32 itemId, uint32 totalCount, Player* mTarget, QuestLogEntry *qLogEntry);

    LuaQuestBinding * m_binding;
};

MapManagerScript* CreateLuaInstance(MapMgr* pMapMgr);
CreatureAIScript * CreateLuaCreature(Creature * src);
GameObjectAIScript * CreateLuaGameObject(GameObject * src);
QuestScript * CreateLuaQuestScript(uint32 id);
GossipScript * CreateLuaUnitGossipScript(uint32 id);
GossipScript * CreateLuaItemGossipScript(uint32 id);
GossipScript * CreateLuaGOGossipScript(uint32 id);

bool Lua_HookOnNewCharacter(uint32 Race, uint32 Class, WorldSession *Session, const char *Name);
void Lua_HookOnKillPlayer(Player *pPlayer, Player *pVictim);
void Lua_HookOnFirstEnterWorld(Player *pPlayer);
void Lua_HookOnEnterWorld(Player *pPlayer);
void Lua_HookOnGuildJoin(Player *pPlayer, GuildInfo *pGuild);
void Lua_HookOnDeath(Player *pPlayer);
bool Lua_HookOnRepop(Player *pPlayer);
void Lua_HookOnEmote(Player *pPlayer, uint32 Emote, Unit *pUnit);
void Lua_HookOnEnterCombat(Player *pPlayer, Unit *pTarget);
bool Lua_HookOnCastSpell(Player *pPlayer, SpellEntry *pSpell, Spell *spell);
void Lua_HookOnTick();
bool Lua_HookOnLogoutRequest(Player *pPlayer);
void Lua_HookOnLogout(Player *pPlayer);
void Lua_HookOnQuestAccept(Player *pPlayer, Quest *pQuest, Object *pQuestGiver);
void Lua_HookOnZone(Player *pPlayer, uint32 Zone, uint32 oldZone);
bool Lua_HookOnChat(Player *pPlayer, uint32 Type, uint32 Lang, const char *Message, const char *Misc);
void Lua_HookOnLoot(Player *pPlayer, Unit *pTarget, uint32 Money, uint32 ItemId);
void Lua_HookOnGuildCreate(Player *pLeader, GuildInfo *pGuild);
void Lua_HookOnEnterWorld2(Player *pPlayer);
void Lua_HookOnCharacterCreate(Player *pPlayer);
void Lua_HookOnQuestCancelled(Player *pPlayer, Quest *pQuest);
void Lua_HookOnQuestFinished(Player *pPlayer, Quest *pQuest, Object *pQuestGiver);
void Lua_HookOnHonorableKill(Player *pPlayer, Player *pKilled);
void Lua_HookOnArenaFinish(Player *pPlayer, ArenaTeam *pTeam, bool victory, bool rated);
void Lua_HookOnObjectLoot(Player *pPlayer, Object *pTarget, uint32 Money, uint32 ItemId);
void Lua_HookOnAreaTrigger(Player *pPlayer, uint32 areaTrigger);
void Lua_HookOnPostLevelUp(Player *pPlayer);
bool Lua_HookOnPreUnitDie(Unit *Killer, Unit *Victim);
void Lua_HookOnAdvanceSkillLine(Player *pPlayer, uint32 SkillLine, uint32 Current);
void Lua_HookOnDuelFinished(Player *pWinner, Player *pLoser);
void Lua_HookOnAuraRemove(Aura *aura);
bool Lua_HookOnResurrect(Player *pPlayer);
bool Lua_HookOnDummySpell(uint32 effectIndex, Spell *pSpell);

#endif // __LUAOBJECTS_H
