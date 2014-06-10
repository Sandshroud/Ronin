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

#ifndef LACRIMI_H
#define LACRIMI_H

class LuaEngine;
class LuaEngineMgr;
class CleanupComponent;

#define BuildEmptyMenu(textid) GossipMenu *Menu; \
    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), textid, Plr)

#define SendEmptyMenu(textid) objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), textid, Plr); \
    Menu->SendTo(Plr)

#define BuildQuickMenu(textid) GossipMenu *Menu; \
    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), textid, Plr); \
    Menu->AddItem(0, "I was looking for something else.", 0)

#define SendQuickMenu(textid) objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), textid, Plr); \
    Menu->AddItem(0, "I was looking for something else.", 0); \
    Menu->SendTo(Plr)

#define RegisterHook(hookid, script) sMgr->register_hook(hookid, (void *)&script)
#define RegisterDummyAura(entry, script) sMgr->register_dummy_aura(entry, &script)
#define RegisterDummySpell(entry, script) sMgr->register_dummy_spell(entry, &script)
#define RegisterSpellScriptEffect(entry, script) sMgr->register_script_effect(entry, &script)
#define RegisterSpellScriptedProclimit(entry, script) sMgr->register_scripted_proclimit(entry, &script)
#define RegisterSpellEffectModifier(entry, script) sMgr->register_spell_effect_modifier(entry, &script)

#define RegisterCtrAIScript(entry, script) sMgr->register_creature_script(entry, &script::Create)
#define RegisterGOAIScript(entry, script) sMgr->register_gameobject_script(entry, &script::Create)
#define RegisterMapManagerScript(mapid, script) sMgr->register_mapmanager_script(mapid, &script::Create)
#define RegisterQuestScript(entry, script) sMgr->register_quest_script(entry, ((QuestScript*)(new script())))
#define RegisterCtrGossipScript(entry, script) sMgr->register_gossip_script(entry, ((GossipScript*)(new script())))
#define RegisterGOGossipScript(entry, script) sMgr->register_go_gossip_script(entry, ((GossipScript*)(new script())))
#define RegisterItemGossipScript(entry, script) sMgr->register_item_gossip_script(entry, ((GossipScript*)(new script())))
#define RegisterLacrimiCleanupComponent(component) CleanupComponents.insert(((CleanupComponent*)(new component())))

class SCRIPT_DECL CleanupComponent
{
public:
    CleanupComponent() { LastCall = 0; CallDelay = 5000; };

    virtual void CleanupFunction(uint32 p_time) {};
    virtual void Destruct() { delete this; };

    uint32 LastCall;
    uint32 CallDelay;
};

class SCRIPT_DECL Lacrimi : public ThreadContext
{
private:
    std::set<CleanupComponent*> CleanupComponents;
    CIniFile *lacrimiIni;
    DirectDatabase* LacrimiDB;
    ScriptMgr* sMgr;
    bool dumpstats;
    bool database;
    bool config;
    bool first;

public:
    Lacrimi(ScriptMgr* mgr);
    ~Lacrimi();

    bool run();
    bool _StartDB();
    void _StopDB();
    void Cleanup();
    void FinalCleanup();
    ScriptMgr* GetScriptMgr() { return sMgr; };
    std::string GetConfigString(char* configfamily, char* configoption, char* cdefault = "");
    float GetConfigfloat(char* configfamily, char* configoption, float fdefault = 0.0f);
    bool GetConfigBool(char* configfamily, char* configoption, bool bdefault = false);
    int GetConfigInt(char* configfamily, char* configoption, int intdefault = 0);
    DirectDatabase* GetLDB() { if(database == true) return LacrimiDB; return NULL; };

public: // Script Related
    void SetupScripts();
    void SetupCityScripts();

public:
///////////////////
////// Spell Setups.
///////////////////
    void SetupSpellScripts();

    // Class Scripts
    void SetupDeathKnightSpells();
    void SetupDruidSpells();
    void SetupHunterSpells();
    void SetupMageSpells();
    void SetupPaladinSpells();
    void SetupPriestSpells();
    void SetupRogueSpells();
    void SetupShamanSpells();
    void SetupWarlockSpells();
    void SetupWarriorSpells();

    // Creature Scripts

    // Other
    void SetupMiscSpells();

public:
///////////////////
////// Zone Setups. // Level 0
///////////////////
    void SetupZoneScripts();

    // Eastern Kingdom
    void SetupEasternKingdomScripts();
    void SetupAlteracMountains();
    void SetupArathiHighlands();
    void SetupBlastedLands();
    void SetupBurningSteppes();
    void SetupDunMorogh();
    void SetupDuskwood();
    void SetupEasternPlaguelands();
    void SetupElwynnForest();
    void SetupEversongWoods();
    void SetupGhostlands();
    void SetupHinterlands();
    void SetupIsleOfQuelDanas();
    void SetupLochModan();
    void SetupSearingGorge();
    void SetupSilverpineForest();
    void SetupStranglethornVale();
    void SetupTrisfalGlades();
    void SetupWesternPlaguelands();
    void SetupWestfall();
    void SetupWetlands();

    // Kalimdor
    void SetupKalimdorScripts();
    void SetupAshenvale();
    void SetupAzshara();
    void SetupAzuremystIsle();
    void SetupBloodmystIsle();
    void SetupDarkshore();
    void SetupDesolace();
    void SetupDurotar();
    void SetupDustwallowMarsh();
    void SetupFelwood();
    void SetupFeralas();
    void SetupMoonglade();
    void SetupMulgore();
    void SetupSilithus();
    void SetupStonetalonMountains();
    void SetupTanaris();
    void SetupTeldrassil();
    void SetupTheBarrens();
    void SetupThousandNeedles();
    void SetupUngoroCrater();
    void SetupWinterspring();

    // Outland
    void SetupOutlandScripts();
    void SetupBladesEdgeMountains();
    void SetupHellfirePeninsula();
    void SetupNagrand();
    void SetupNetherstorm();
    void SetupShadowmoonValley();
    void SetupTerrokarForest();
    void SetupZangarmarsh();

    // Northrend
    void SetupNorthrendScripts();
    void SetupWintergrasp();
    void SetupBoreanTundra();
    void SetupDragonblight();
    void SetupGrizzlyHills();
    void SetupHowlingFjord();
    void SetupIcecrown();
    void SetupSholazarBasin();
    void SetupStormPeaks();
    void SetupZulDrak();

    // Ebon Hold
    void SetupEbonHoldScripts();
    void SetupEHPhaseOne();
    void SetupEHPhaseTwo();
    void SetupEHPhaseThree();

///////////////////
////// City Setups.
///////////////////

    // Eastern Kingdom
    void SetupIronforge();
    void SetupSilvermoon();
    void SetupStormwind();
    void SetupUndercity();

    // Kalimdor
    void SetupDarnassus();
    void SetupExodar();
    void SetupOrgrimmar();
    void SetupThunderbluff();

    // Outlands
    void SetupShattrath();

    // Northrend
    void SetupDalaran();

public: // Instance Scripts
    void SetupInstanceScripts();

///////////////////
////// Classic Scripts // Level 2
///////////////////
    void SetupBlackfathomDeeps();

public: // Custom Scripts
    void SetupCustomScripts();

///////////////////
////// Custom Scripts
///////////////////

public: // Stat Dumper
    void DumpStats();
    void GenerateUptimeString(char * Dest);
    void FillOnlineTime(uint32 Time, char * Dest);

    char Filename[MAX_PATH];

public: // LuaEngine
    LuaEngine* L_LuaEngine;
    LuaEngineMgr* L_LuaEngineMgr;
    bool LuaEngineIsStarting;
};

#define sLacrimi (*sWorld.LacrimiPtr)

#define LacrimiDatabase (*(sWorld.LacrimiPtr->GetLDB()))

#define g_engine (sLacrimi.L_LuaEngine)
#define g_luaMgr (*sLacrimi.L_LuaEngineMgr)
#define LuaEvent (g_luaMgr.LuaEventMgr)

#endif