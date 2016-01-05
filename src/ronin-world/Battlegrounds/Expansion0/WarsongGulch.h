/***
 * Demonstrike Core
 */

#pragma once

#define BUFF_RESPAWN_TIME 90000

class WarsongGulch : public CBattleground
{
    GameObject* m_buffs[6];
    GameObject* m_homeFlags[2];
    GameObject* m_dropFlags[2];
    WoWGuid m_flagHolders[2];
    std::list<GameObject* > m_gates;
    uint32 m_scores[2];
    uint32 m_lgroup;
    bool m_flagAtBase[2];
    uint32 m_FlagCaptureHonor;
    uint32 m_WinHonor;
    uint32 m_CompleteHonor;
public:
    WarsongGulch(MapInstance* m, uint32 id, uint32 lgroup, uint32 t);
    ~WarsongGulch();
    virtual void Init();

    void HookOnPlayerDeath(Player* plr);
    void HookFlagDrop(Player* plr, GameObject* obj);
    void HookFlagStand(Player* plr, GameObject* obj);
    void HookOnMount(Player* plr);
    void HookOnAreaTrigger(Player* plr, uint32 id);
    bool HookHandleRepop(Player* plr);
    void OnAddPlayer(Player* plr);
    void OnRemovePlayer(Player* plr);
    void OnCreate();
    void HookOnPlayerKill(Player* plr, Unit* pVictim);
    void HookOnHK(Player* plr);
    void HookOnShadowSight();
    void SpawnBuff(uint32 x);
    LocationVector GetStartingCoords(uint8 Team);
    void DropFlag(Player* plr);
    void ReturnFlag(uint8 team);

    static CBattleground* Create(MapInstance *m, uint32 i, uint32 l, uint32 t) { return new WarsongGulch(m, i, l, t); }

    WoWGuid GetAllianceFlagHolderGUID() { return m_flagHolders[0]; }
    WoWGuid GetHordeFlagHolderGUID() { return m_flagHolders[1]; }
    const char * GetName() { return "Warsong Gulch"; }
    void OnStart();

    /* looooooot */
    bool SupportsPlayerLoot() { return true; }
    void HookGenerateLoot(Player* plr, Corpse* pCorpse);

    void SetIsWeekend(bool isweekend);
};
