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

#pragma once

#define ZONE_WINTERGRASP 4197
#define A_NUMVEH_WORLDSTATE 3680
#define A_MAXVEH_WORLDSTATE 3681
#define H_NUMVEH_WORLDSTATE 3490
#define H_MAXVEH_WORLDSTATE 3491
#define HORDE_CONTROLLED 3802
#define ALLIANCE_CONTROLLED 3803
const uint32 ClockWorldState[2] = { 3781, 4354 };

#define WGSPELL_ESSENCE_OF_WINTERGRASP 57940
#define WGSPELL_TENACITY 59911

#define DEFENDERS_PORTAL 190763 // 2d: The portal thing which you select to teleport inside if a defender


enum WintergraspEntryPairs
{

};

const uint32 Wintergrasp_CreatureEntryPair[2][2] =
{
    { 00000, 00000 }, //
    { 00000, 00000 }, //
};

const Coords Wintergrasp_SpawnLocations[2][2] =
{
    { { 0000.000f,  0000.000f,  0000.000f,  0000.000f,  1 }, {  0000.000f,  0000.000f,  0000.000f,  0000.000f,  1 } },
    { { 0000.000f,  0000.000f,  0000.000f,  0000.000f,  1 }, {  0000.000f,  0000.000f,  0000.000f,  0000.000f,  1 } },
};

struct SpawnInfo
{
    uint32 CreatureEntry;
    uint32 RespawnTime;
    uint32 locationId;
    uint32 SpawnId;

    Creature* m_Spawn;
};

class WintergraspScript : public MapManagerScript
{
    ADD_MAPMANAGER_FACTORY_FUNCTION(WintergraspScript);
private:
    MapMgr* mgr;
    bool m_started, m_Initialized;
    int32 ControllingTeam;
    uint32 m_NextTime, m_FinishTime;
    uint32 m_clock[2];

    bool FirstHourHerald, ThirtyMinuteHerald, FifteenMinuteHerald;

public:
    WintergraspScript(MapMgr* Thismgr);
    ~WintergraspScript();

    // Hooks
    void OnUpdate(uint32 p_time);
    void OnChangeArea( Player* pPlayer, uint32 ZoneID, uint32 NewAreaID, uint32 OldAreaID );

    // Functions
    void EndBattle();
    void StartBattle();
    void DelayBattle();
    void UpdateClockDigit(uint32 timer, uint32 digit, uint32 mod);

    void Herald(char* zMsg);

    void UpdateWarZones();

    void SpawnWarZones(bool apply);

    void SendInitWorldStates(Player* plr = NULL);
    void UpdateWorldState(uint32 WorldState, uint32 Value);

    bool HasStarted() { return m_started;};
    int32 GetControllingTeam() { return ControllingTeam; };
    uint32 GetTimeTillFinish() { if(!m_started) return 0; return m_FinishTime/1000; };
    uint32 GetTimeTillStart() { if(m_started) return 0; return m_NextTime-UNIXTIME; };

private:
    Mutex _playerslock;
    PlayerStorageMap _players;
    std::set<SpawnInfo*> m_WarriorPool, m_WarriorSoulPool;
};
