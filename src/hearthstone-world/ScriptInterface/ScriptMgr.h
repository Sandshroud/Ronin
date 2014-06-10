/***
 * Demonstrike Core
 */

#pragma once

#define SCRIPT_MODULE void*
#define ADD_CREATURE_FACTORY_FUNCTION(cl) public: static CreatureAIScript * Create(Creature* c) { return new cl(c); }
#define ADD_GAMEOBJECT_FACTORY_FUNCTION(cl) public: static GameObjectAIScript * Create(GameObject* go) { return new cl(go); }
#define ADD_MAPMANAGER_FACTORY_FUNCTION(cl) public: static MapManagerScript* Create( MapMgr* pMapMgr ) { return new cl(pMapMgr); };

class Channel;
struct Quest;

enum ServerHookEvents
{
    SERVER_HOOK_BATTLEGROUNDMGR_CREATE      = 0,

    SERVER_HOOK_EVENT_ON_NEW_CHARACTER      = 1,
    SERVER_HOOK_EVENT_ON_KILL_PLAYER        = 2,
    SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD  = 3,
    SERVER_HOOK_EVENT_ON_ENTER_WORLD        = 4,
    SERVER_HOOK_EVENT_ON_GUILD_JOIN         = 5,
    SERVER_HOOK_EVENT_ON_DEATH              = 6,
    SERVER_HOOK_EVENT_ON_REPOP              = 7,
    SERVER_HOOK_EVENT_ON_EMOTE              = 8,
    SERVER_HOOK_EVENT_ON_ENTER_COMBAT       = 9,
    SERVER_HOOK_EVENT_ON_CAST_SPELL         = 10,
    SERVER_HOOK_EVENT_ON_TICK               = 11,
    SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST     = 12,
    SERVER_HOOK_EVENT_ON_LOGOUT             = 13,
    SERVER_HOOK_EVENT_ON_QUEST_ACCEPT       = 14,
    SERVER_HOOK_EVENT_ON_ZONE               = 15,
    SERVER_HOOK_EVENT_ON_CHAT               = 16,
    SERVER_HOOK_EVENT_ON_LOOT               = 17,
    SERVER_HOOK_EVENT_ON_GUILD_CREATE       = 18,
    SERVER_HOOK_EVENT_ON_FULL_LOGIN         = 19,
    SERVER_HOOK_EVENT_ON_CHARACTER_CREATE   = 20,
    SERVER_HOOK_EVENT_ON_QUEST_CANCELLED    = 21,
    SERVER_HOOK_EVENT_ON_QUEST_FINISHED     = 22,
    SERVER_HOOK_EVENT_ON_HONORABLE_KILL     = 23,
    SERVER_HOOK_EVENT_ON_ARENA_FINISH       = 24,
    SERVER_HOOK_EVENT_ON_OBJECTLOOT         = 25,
    SERVER_HOOK_EVENT_ON_AREATRIGGER        = 26,
    SERVER_HOOK_EVENT_ON_POST_LEVELUP       = 27,
    SERVER_HOOK_EVENT_ON_PRE_DIE            = 28,
    SERVER_HOOK_EVENT_ON_POST_DIE           = 29,
    SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE  = 30,
    SERVER_HOOK_EVENT_ON_DUEL_FINISHED      = 31,
    SERVER_HOOK_EVENT_ON_AURA_REMOVE        = 32,
    SERVER_HOOK_EVENT_ON_RESURRECT          = 33,
    SERVER_HOOK_EVENT_ON_GOSSIP_MENU_BUILD  = 34,
    SERVER_HOOK_EVENT_ON_GOSSIP_SEL_OPTION  = 35,
    SERVER_HOOK_EVENT_ON_CHECK_PREREQUISITS = 36,
    SERVER_HOOK_EVENT_ON_CHECK_AREA_ITEMS   = 37,
    SERVER_HOOK_EVENT_ON_PLAYER_UPDATE      = 38,
    SERVER_HOOK_EVENT_ON_CONTINENT_CREATE   = 39,
    SERVER_HOOK_EVENT_ON_POST_SPELL_CAST    = 40,
    SERVER_HOOK_EVENT_ON_PLAYER_SAVE_TO_DB  = 41,
    SERVER_HOOK_EVENT_ON_PLAYER_CHANGE_AREA = 42,
    SERVER_HOOK_EVENT_ON_CHECK_EQUIP_STATUS = 43,

    // Destructable Buildings
    SERVER_HOOK_EVENT_ON_DESTROY_BUILDING   = 44,
    SERVER_HOOK_EVENT_ON_DAMAGE_BUILDING    = 45,
    SERVER_HOOK_EVENT_ON_MOUNT_FLYING       = 46,
    SERVER_HOOK_EVENT_ON_PRE_AURA_REMOVE    = 47,
    SERVER_HOOK_EVENT_ON_SLOW_LOCK_OPEN     = 48,
    NUM_SERVER_HOOKS,
};

enum ScriptTypes
{
    SCRIPT_TYPE_MISC                        = 0x01,
    SCRIPT_TYPE_INFODUMPER                  = 0x02,
    SCRIPT_TYPE_SCRIPT_ENGINE               = 0x04,
    SCRIPT_TYPE_SCRIPT_ENGINE_LUA           = 0x08,
    SCRIPT_TYPE_SCRIPT_ENGINE_GM            = 0x10,
};

// Special Hook typedefs
typedef bool(*tOnBattlegroundManagerCreate)();

/* Hook typedefs */
typedef bool(*tOnNewCharacter)(uint32 Race, uint32 Class, WorldSession * Session, const char * Name);
typedef void(*tOnKillPlayer)(Player* pPlayer, Player* pVictim);
typedef void(*tOCharacterCreate)(Player* pPlayer);
typedef void(*tOnFirstEnterWorld)(Player* pPlayer);
typedef void(*tOnEnterWorld)(Player* pPlayer);
typedef void(*tOnFullLogin)(Player* pPlayer);
typedef void(*tOnDeath)(Player* pPlayer);
typedef bool(*tOnRepop)(Player* pPlayer);
typedef void(*tOnEmote)(Player* pPlayer, uint32 Emote, Unit* pUnit);
typedef void(*tOnEnterCombat)(Player* pPlayer, Unit* pTarget);
typedef bool(*tOnCastSpell)(Player* pPlayer, SpellEntry * pSpell);
typedef void(*tOnTick)();
typedef bool(*tOnLogoutRequest)(Player* pPlayer);
typedef void(*tOnLogout)(Player* pPlayer);
typedef void(*tOnQuestAccept)(Player* pPlayer, Quest * pQuest, Object* pObject);
typedef void(*tOnZone)(Player* pPlayer, uint32 Zone, uint32 OldZone);
typedef bool(*tOnChat)(Player* pPlayer, uint32 Type, uint32 Lang, string Message, string Misc);
typedef void(*tOnLoot)(Player * pPlayer, Object* pTarget, uint32 Money, uint32 ItemId);
typedef bool(*ItemScript)(Item* pItem, Player* pPlayer);
typedef void(*tOnQuestCancel)(Player* pPlayer, Quest * pQuest);
typedef void(*tOnQuestFinished)(Player* pPlayer, Quest * pQuest, Object* pObject);
typedef void(*tOnHonorableKill)(Player* pPlayer, Player* pKilled);
typedef void(*tOnArenaFinish)(Player* pPlayer, uint32 type, ArenaTeam * pTeam, bool victory, bool rated);
typedef void(*tOnObjectLoot)(Player * pPlayer, Object * pTarget, uint32 Money, uint32 ItemId);
typedef void(*tOnAreaTrigger)(Player * pPlayer, uint32 areaTrigger);
typedef void(*tOnPostLevelUp)(Player * pPlayer);
typedef bool(*tOnPreUnitDie)(Unit *killer, Unit *target);
typedef bool(*tOnPostUnitDie)(Unit *killer, Unit *target);
typedef void(*tOnAdvanceSkillLine)(Player * pPlayer, uint32 SkillLine, uint32 Current);
typedef void(*tOnDuelFinished)(Player * Winner, Player * Looser);
typedef void(*tOnAuraRemove)(Player* pPlayer, uint32 spellID);
typedef bool(*tOnResurrect)(Player * pPlayer);
typedef void(*tOnBuildGossipMenu)(Object* Obj, Player* pPlayer, GossipMenu* Gossip);
typedef bool(*tOnGossipSelectOption)(Object* Obj, Player* pPlayer, int32 IntId);
typedef bool(*tOnCheckTeleportPrerequisites)(Player* pPlayer, uint32 mapid);
typedef bool(*tOnCheckEquippableStatus)(Player* pPlayer, ItemPrototype* proto, int16 DstInvSlot, int16 slot);
typedef void(*tOnCheckAreaItems)(ItemInterface * m_Interface);
typedef void(*tOnPlayerUpdate)(Player * plr);
typedef void(*tOnContinentCreate)(MapMgr* mgr);
typedef void(*tOnPostSpellCast)(Player* pPlayer, SpellEntry * pSpell, Unit* pTarget);
typedef void(*tOnPlayerSaveToDB)(Player* pPlayer, QueryBuffer* buf);
typedef void(*tOnPlayerChangeArea)(Player* pPlayer, uint32 ZoneID, uint32 NewAreaID, uint32 OldAreaID);

//Destructable building
typedef void(*tOnDestroyBuilding)(GameObject* go);
typedef void(*tOnDamageBuilding)(GameObject* go);
typedef bool(*tOnMountFlying) (Player* plr);
typedef bool(*tOnPreAuraRemove)(Player* plr,uint32 spellID);
typedef void(*tOnSlowLockOpen)(GameObject* go,Player* plr);

class CreatureAIScript;
class GossipScript;
class GameObjectAIScript;
class ScriptMgr;
class QuestScript;
struct ItemPrototype;

/* Factory Imports (from script lib) */
typedef CreatureAIScript*(*exp_create_creature_ai)(Creature* pCreature);
typedef GameObjectAIScript*(*exp_create_gameobject_ai)(GameObject* pGameObject);
typedef MapManagerScript*(*exp_create_mapmanager_script)(MapMgr *_internal);

typedef bool(*exp_handle_dummy_spell)(uint32 i, Spell* pSpell);
typedef bool(*exp_handle_script_effect)(uint32 i, Spell* pSpell);
typedef bool(*exp_handle_script_proclimit)(Unit *target, uint32 &uSpellId, int32 &damage, SpellCastTargets &targets, ProcTriggerSpell *triggered, ProcDataHolder *dataHolder);
typedef bool(*exp_handle_dummy_aura)(uint32 i, Aura* pAura, bool apply);
typedef void(*exp_handle_spell_effect_mod)(uint32 i, Spell* pSpell, uint32 effect);

typedef void(*exp_script_register)(ScriptMgr * mgr);

typedef uint32(*exp_get_script_type)();
typedef uint32(*exp_get_version)();

/* Hashmap typedefs */
typedef HM_NAMESPACE::hash_map<uint32, exp_create_creature_ai> CreatureCreateMap;
typedef HM_NAMESPACE::hash_map<uint32, exp_create_gameobject_ai> GameObjectCreateMap;
typedef HM_NAMESPACE::hash_map<uint32, exp_create_mapmanager_script> MapMgrScriptCreateMap;

typedef HM_NAMESPACE::hash_map<uint32, exp_handle_dummy_aura> HandleDummyAuraMap;
typedef HM_NAMESPACE::hash_map<uint32, exp_handle_dummy_spell> HandleDummySpellMap;
typedef HM_NAMESPACE::hash_map<uint32, exp_handle_script_effect> HandleScriptEffectMap;
typedef HM_NAMESPACE::hash_map<uint32, exp_handle_script_proclimit> HandleScriptProclimitMap;
typedef HM_NAMESPACE::hash_map<uint32, exp_handle_spell_effect_mod> HandleSpellEffectModifier;

typedef map<uint8, map<uint32, GossipScript*> > MultiTypeGossipMap;
typedef map<uint32, QuestScript* > QuestScriptMap;
typedef set<GossipScript*> CustomGossipScripts;
typedef set<QuestScript*> QuestScripts;
typedef list<void*> ServerHookList;
typedef list<SCRIPT_MODULE> LibraryHandleMap;

enum GOSSIPTYPEID
{
    GTYPEID_CTR         = 0,
    GTYPEID_ITEM        = 1,
    GTYPEID_GAMEOBJECT  = 2,
};

class SERVER_DECL ScriptMgr : public Singleton<ScriptMgr>
{
public:
    ScriptMgr();
    ~ScriptMgr();

    friend class HookInterface;

    void LoadScripts();
    void UnloadScripts(bool safe = false);
    void ReloadScripts();

    CreatureAIScript * CreateAIScriptClassForEntry(Creature* pCreature);
    GameObjectAIScript * CreateAIScriptClassForGameObject(uint32 uEntryId, GameObject* pGameObject);
    MapManagerScript * CreateMapManagerScriptForMapManager(uint32 mapid, MapMgr* _internal, bool Silent);

    bool HandleScriptedProcLimits(Unit *target, uint32 &uSpellId, int32 &damage, SpellCastTargets &targets, ProcTriggerSpell *triggered, ProcDataHolder *dataHolder);
    bool HandleScriptedSpellEffect(uint32 uSpellId, uint32 i, Spell* pSpell);
    bool CallScriptedDummySpell(uint32 uSpellId, uint32 i, Spell* pSpell);
    bool CallScriptedDummyAura(uint32 uSpellId, uint32 i, Aura*  pAura, bool apply);
    bool CallScriptedItem(Item* pItem, Player* pPlayer);
    void HandleSpellEffectMod(uint32 uSpellId, uint32 i, Spell *pSpell, uint32 effect);

    void register_quest_script(uint32 entry, QuestScript * qs);
    void register_gossip_script(uint32 entry, GossipScript * gs);
    void register_go_gossip_script(uint32 entry, GossipScript * gs);
    void register_item_gossip_script(uint32 entry, GossipScript * gs);
    void register_hook(ServerHookEvents event, void * function_pointer);
    void register_dummy_aura(uint32 entry, exp_handle_dummy_aura callback);
    void register_dummy_spell(uint32 entry, exp_handle_dummy_spell callback);
    void register_script_effect(uint32 entry, exp_handle_script_effect callback);
    void register_scripted_proclimit(uint32 entry, exp_handle_script_proclimit callback);
    void register_creature_script(uint32 entry, exp_create_creature_ai callback);
    void register_gameobject_script(uint32 entry, exp_create_gameobject_ai callback);
    void register_mapmanager_script(uint32 entry, exp_create_mapmanager_script callback);
    void register_spell_effect_modifier(uint32 entry, exp_handle_spell_effect_mod callback);

    HEARTHSTONE_INLINE QuestScript* GetQuestScript(uint32 entry) { return EntryQuestScriptMap[entry]; };
    HEARTHSTONE_INLINE GossipScript* GetRegisteredGossipScript(uint8 type, uint32 entry, bool rdefault = true)
    {
        if(GossipMaps[type].size())
            if(GossipMaps[type][entry] != NULL)
                return GossipMaps[type][entry];
        if(rdefault)
            return DefaultGossipScript;
        return NULL;
    };

protected:
    CreatureCreateMap _creatures;
    GameObjectCreateMap _gameobjects;
    MapMgrScriptCreateMap _mapmanagers;

    HandleDummyAuraMap _auras;
    HandleDummySpellMap _spells;
    HandleScriptEffectMap _effects;
    HandleScriptProclimitMap _proclimits;
    HandleSpellEffectModifier _effectmods;

    LibraryHandleMap _handles;
    GossipScript * DefaultGossipScript;
    ServerHookList _hooks[NUM_SERVER_HOOKS];
    CustomGossipScripts _customgossipscripts;
    QuestScripts _questscripts;
    MultiTypeGossipMap GossipMaps;
    QuestScriptMap EntryQuestScriptMap;

    exp_handle_script_proclimit _baseproclimits;
};

class SERVER_DECL CreatureAIScript
{
public:
    CreatureAIScript(Creature* creature);
    virtual ~CreatureAIScript() {};

    virtual void AIUpdate(MapManagerScript*, uint32 ptime) {}
    virtual void OnCombatStart(Unit* mTarget) {}
    virtual void OnCombatStop(Unit* mTarget) {}
    virtual void OnDamageTaken(Unit* mAttacker, float fAmount) {}
    virtual void OnCastSpell(uint32 iSpellId) {}
    virtual void OnTargetParried(Unit* mTarget) {}
    virtual void OnTargetDodged(Unit* mTarget) {}
    virtual void OnTargetBlocked(Unit* mTarget, int32 iAmount) {}
    virtual void OnTargetCritHit(Unit* mTarget, float fAmount) {}
    virtual void OnTargetDied(Unit* mTarget) {}
    virtual void OnParried(Unit* mTarget) {}
    virtual void OnDodged(Unit* mTarget) {}
    virtual void OnBlocked(Unit* mTarget, int32 iAmount) {}
    virtual void OnCritHit(Unit* mTarget, float fAmount) {}
    virtual void OnHit(Unit* mTarget, float fAmount) {}
    virtual void OnDied(Unit* mKiller) {}
    virtual void OnAssistTargetDied(Unit* mAssistTarget) {}
    virtual void OnFear(Unit* mFeared, uint32 iSpellId) {}
    virtual void OnFlee(Unit* mFlee) {}
    virtual void OnCallForHelp() {}
    virtual void OnLoad() {}
    virtual void OnReachWP(uint32 iWaypointId, bool bForwards) {}
    virtual void OnLootTaken(Player* pPlayer, ItemPrototype *pItemPrototype) {}
    virtual void OnEmote(Player* pPlayer, EmoteType Emote) {}
    virtual void StringFunctionCall(int) {}

    virtual void Destroy() { delete this; }
    Creature* GetUnit() { return _unit; }

    bool LuaScript;

protected:
    Creature* _unit;
};

class SERVER_DECL GameObjectAIScript
{
public:
    GameObjectAIScript(GameObject* goinstance);
    virtual ~GameObjectAIScript() {}

    virtual void AIUpdate(MapManagerScript*, uint32 ptime) {}
    virtual void OnCreate() {}
    virtual void OnSpawn() {}
    virtual void OnDespawn() {}
    virtual void OnLootTaken(Player * pLooter, ItemPrototype *pItemInfo) {}
    virtual void OnActivate(Player* pPlayer) { _gameobject->m_scripted_use = false; }
    virtual void Destroy() { delete this; }

    bool LuaScript;

protected:
    GameObject* _gameobject;
};

class SERVER_DECL GossipScript
{
public:
    GossipScript();
    virtual ~GossipScript() {}

    virtual void GossipHello(Object* pObject, Player* Plr, bool AutoSend);
    virtual void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char * EnteredCode);
    virtual void GossipEnd(Object* pObject, Player* Plr);
    virtual void Destroy() { delete this; };

    bool LuaScript;
};

class SERVER_DECL QuestScript
{
public:
    QuestScript() : LuaScript(false) {};
    virtual ~QuestScript() {};

    virtual void OnQuestStart(Player* mTarget, QuestLogEntry *qLogEntry) {}
    virtual void OnQuestComplete(Player* mTarget, QuestLogEntry *qLogEntry) {}
    virtual void OnQuestCancel(Player* mTarget) {}
    virtual void OnGameObjectActivate(uint32 entry, Player* mTarget, QuestLogEntry *qLogEntry) {}
    virtual void OnCreatureKill(uint32 entry, Player* mTarget, QuestLogEntry *qLogEntry) {}
    virtual void OnExploreArea(uint32 areaId, Player* mTarget, QuestLogEntry *qLogEntry) {}
    virtual void OnCrossAreaTrigger(uint32 areaTrigger, Player* mTarget, QuestLogEntry *qLogEntry) {}
    virtual void OnPlayerItemPickup(uint32 itemId, uint32 totalCount, Player* mTarget, QuestLogEntry *qLogEntry) {}

    bool LuaScript;
};

class SERVER_DECL HookInterface : public Singleton<HookInterface>
{
public:
    friend class ScriptMgr;

    void OnBattlegroundManagerCreate(); // Called once.

    bool OnNewCharacter(uint32 Race, uint32 Class, WorldSession * Session, const char * Name);
    void OnKillPlayer(Player* pPlayer, Player* pVictim);
    void OnFirstEnterWorld(Player* pPlayer);
    void OnEnterWorld(Player* pPlayer);
    void OnDeath(Player* pPlayer);
    bool OnRepop(Player* pPlayer);
    void OnEmote(Player* pPlayer, uint32 Emote, Unit* pUnit);
    void OnEnterCombat(Player* pPlayer, Unit* pTarget);
    bool OnCastSpell(Player* pPlayer, SpellEntry * pSpell);
    bool OnLogoutRequest(Player* pPlayer);
    void OnLogout(Player* pPlayer);
    void OnQuestAccept(Player* pPlayer, Quest * pQuest, Object* pObject);
    void OnZone(Player* pPlayer, uint32 Zone, uint32 OldZone);
    bool OnChat(Player* pPlayer, uint32 Type, uint32 Lang, string Message, string Misc);
    void OnLoot(Player * pPlayer, Object* pTarget, uint32 Money, uint32 ItemId);
    void OnFullLogin(Player* pPlayer);
    void OnCharacterCreate(Player* pPlayer);
    void OnQuestCancelled(Player* pPlayer, Quest * pQuest);
    void OnQuestFinished(Player* pPlayer, Quest * pQuest, Object* pObject);
    void OnHonorableKill(Player* pPlayer, Player* pKilled);
    void OnArenaFinish(Player* pPlayer, uint32 type, ArenaTeam* pTeam, bool victory, bool rated);
    void OnObjectLoot(Player * pPlayer, Object * pTarget, uint32 Money, uint32 ItemId);
    void OnAreaTrigger(Player * pPlayer, uint32 areaTrigger);
    void OnPostLevelUp(Player * pPlayer);
    bool OnPreUnitDie(Unit *Killer, Unit *Victim);
    void OnPostUnitDie(Unit *Killer, Unit *Victim);
    void OnAdvanceSkillLine(Player * pPlayer, uint32 SkillLine, uint32 Current);
    void OnDuelFinished(Player * Winner, Player * Looser);
    void OnCheckAreaItems(ItemInterface *m_Interface);
    void OnPlayerUpdate(Player *plr);
    void OnContinentCreate(MapMgr* pMgr);
    void OnPostSpellCast(Player* pPlayer, SpellEntry * pSpell, Unit* pTarget);
    void OnPlayerSaveToDB(Player* pPlayer, QueryBuffer* buf);
    void OnPlayerChangeArea(Player* pPlayer, uint32 ZoneID, uint32 NewAreaID, uint32 OldAreaID);
    void OnAuraRemove(Player* pPlayer, uint32 spellID);
    bool OnResurrect(Player * pPlayer);
    void OnBuildGossipMenu(Object* Obj, Player* pPlayer, GossipMenu* Gossip);
    bool OnGossipSelectOption(Object* Obj, Player* pPlayer, int32 IntId);
    bool OnCheckTeleportPrerequisites(Player* plr, uint32 mapid);
    bool OnCheckEquippableStatus(Player* plr, ItemPrototype* proto, int16 DstInvSlot, int16 slot);

    //Destructable buildings
    void OnDestroyBuilding(GameObject* go);
    void OnDamageBuilding(GameObject* go);
    bool OnMountFlying(Player* plr);
    bool OnPreAuraRemove(Player* remover,uint32 spellID);
    void OnSlowLockOpen(GameObject* go,Player* plr);
};

#define sScriptMgr ScriptMgr::getSingleton()
#define sHookInterface HookInterface::getSingleton()
