/***
 * Demonstrike Core
 */

#pragma once

class ArenaTeam;
struct PlayerInfo;

class Arena : public CBattleground
{
    set< GameObject* > m_gates;
    uint32 m_arenateamtype;

    uint32 m_pcWorldStates[2];
    GameObject* m_buffs[2];

    uint32 m_playersCount[2];
    set<uint32> m_players2[2];
    set<uint32> m_playersAlive;

public:
    bool rated_match;
    Arena( MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t, uint32 players_per_side);
    virtual ~Arena();
    virtual void Init();

    bool HookHandleRepop(Player* plr);
    void OnAddPlayer(Player* plr);
    void OnRemovePlayer(Player* plr);
    void OnCreate();
    void HookOnPlayerDeath(Player* plr);
    void HookOnPlayerKill(Player* plr, Unit* pVictim);
    void HookOnHK(Player* plr);
    void HookOnShadowSight();
    void UpdatePlayerCounts();
    LocationVector GetStartingCoords(uint32 Team);
    virtual const char * GetName() { return "Arena"; }
    void OnStart();
    bool CanPlayerJoin(Player* plr)
    {
        if(m_started)
            return false;
        else
            return CBattleground::CanPlayerJoin(plr);
    }

    bool CreateCorpse(Player* plr) { return false; }

    /* dummy stuff */
    void HookOnMount(Player* plr) {}
    void HookFlagDrop(Player* plr, GameObject* obj) {}
    void HookFlagStand(Player* plr, GameObject* obj) {}
    void HookOnAreaTrigger(Player* plr, uint32 id);

    void BuffRespawn(uint32 buffId);

    int32 GetFreeTeam()
    {
        size_t c0 = m_players[0].size() + m_pendPlayers[0].size();
        size_t c1 = m_players[1].size() + m_pendPlayers[1].size();
        if(m_started) return -1;

        // Check if there is free room, if yes, return team with less members
        return ((c0 + c1 >= m_playerCountPerTeam * 2) ? -1 : (c0 > c1));

        /* We shouldn't reach here. */
    }

    void Finish();
    uint8 Rated() { return rated_match; }
    HEARTHSTONE_INLINE uint32 GetArenaTeamType() { return m_arenateamtype; }

    /* looooooot */
    bool SupportsPlayerLoot() { return false; }
    void HookGenerateLoot(Player* plr, Corpse* pCorpse) {}

    int32 CalcDeltaRating(uint32 oldRating, uint32 opponentRating, bool outcome);
    int32 m_teams[2]; // Ids of teams fighting in rated match
};
