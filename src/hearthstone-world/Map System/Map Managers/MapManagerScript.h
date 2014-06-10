/*
 * Sandshroud Hearthstone
 * Copyright (C) 2010 - 2011 Sandshroud <http://www.sandshroud.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _MAP_SCRIPT_INTERFACE_H
#define _MAP_SCRIPT_INTERFACE_H

class MapMgr;

class SERVER_DECL MapManagerScript
{
private:
    MapMgr* _manager;
    map<uint32, CreatureAIScript*> m_CreatureAIScripts;
    map<uint32, GameObjectAIScript*> m_GameObjectAIScripts;

public:
    MapManagerScript(MapMgr* _internal);
    ~MapManagerScript();

    void Update(uint32 p_time);
    virtual void OnUpdate(uint32 p_time) {};

    void AddCreatureAI(Creature*);
    void RemoveCreatureAI(Creature*);

    void AddGameObjectAI(GameObject*);
    void RemoveGameObjectAI(GameObject*);

    virtual void OnLoad() {};
    virtual void Destruct() { delete this; };

    // Procedures that had been here before
    virtual void SetLockOptions( uint32 pEntryId, GameObject* pGameObject ) {};
    virtual uint32 GetRespawnTimeForCreature( uint32 pEntryId, Creature* pCreature) { return 240000; };
    virtual GameObject* GetObjectForOpenLock( Player* pCaster, Spell* pSpell, SpellEntry* pSpellEntry ) { return NULLGOB; };

    // Player
    virtual void OnPlayerDeath( Player* pVictim, Unit* pKiller ) {};
    virtual bool OnPlayerRepopRequest(Player* plr) { return false; };
    virtual void OnPlayerMount(Player* plr) {};
    virtual void OnPlayerFlagDrop(Player* plr, GameObject* obj) {};
    virtual void OnPlayerFlagStand(Player* plr, GameObject* obj) {};
    virtual void OnPlayerKillPlayer(Player* plr, Unit* pVictim) {};
    virtual void OnPlayerHonorKill(Player* plr) {};
    virtual void OnPlayerLootGen(Player* plr, Corpse* pCorpse) {};

    // Area and AreaTrigger
    virtual void OnPlayerEnter( Player* pPlayer ) {};
    virtual void OnAreaTrigger( Player* pPlayer, uint32 pAreaId ) {};
    virtual void OnZoneChange( Player* pPlayer, uint32 pNewZone, uint32 pOldZone ) {};
    virtual void OnChangeArea( Player* pPlayer, uint32 ZoneID, uint32 NewAreaID, uint32 OldAreaID ) {};

    // Data get / set - idea taken from ScriptDev2
    virtual void SetInstanceData( uint32 pType, uint32 pIndex, uint32 pData ) {};
    virtual uint32 GetInstanceData( uint32 pType, uint32 pIndex ) { return 0; };

    // Creature / GameObject
    virtual void OnCreatureDeath( Creature* pVictim, Unit* pKiller ) {};
    virtual void OnGameObjectActivate( GameObject* pGameObject, Player* pPlayer ) {};

    virtual void OnCreaturePushToWorld( Creature* pCreature ) { AddCreatureAI(pCreature); };
    virtual void OnCreatureRemoveFromWorld( Creature* pCreature ) { RemoveCreatureAI(pCreature); };

    virtual void OnGameObjectPushToWorld( GameObject* pGameObject ) { AddGameObjectAI(pGameObject); };
    virtual void OnGameObjectRemoveFromWorld( GameObject* pGameObject ) { RemoveGameObjectAI(pGameObject); };

    // Specialties
    virtual bool MapSupportsPlayerLoot() { return false; };

public: // Functions for easy scripting

    // Creature
    Creature *GetCreatureByGuid( uint32 pGuid );
    Creature *GetCreatureBySqlId( uint32 pSqlId );
    Creature *FindClosestCreature( uint32 pEntry, float pX, float pY, float pZ );
    Creature *FindClosestLivingCreature( uint32 pEntry, float pX, float pY, float pZ, Creature* check );
    Creature *SpawnCreature( uint32 pEntry, float pX, float pY, float pZ, float pO, int32 PhaseMask = 1 );
    Creature *SpawnCreature( uint32 pEntry, float pX, float pY, float pZ, float pO, uint32 pFactionId, int32 PhaseMask = 1 );

    // GameObject
    GameObject *GetGameObjectBySqlId( uint32 pSqlId );
    GameObject *GetGameObjectByGuid( uint32 pGuid );
    GameObject *FindClosestGameObject( uint32 pEntry, float pX, float pY, float pZ );
    GameObject *SpawnGameObject( uint32 pEntry, float pX, float pY, float pZ, float pO );

    // Player
    Player *FindClosestPlayer( float pX, float pY, float pZ, Player* pCheck = NULL );

    // Distance calculation
    float GetRangeToObject( Object *pObjectA, Object *pObjectB );
    float GetRangeToObject( Object *pObject, float pX, float pY, float pZ );
    float GetRangeToObject( float pX1, float pY1, float pZ1, float pX2, float pY2, float pZ2 );

    // Player and instance - reimplementation for easier calling
    bool HasPlayers();
    size_t GetPlayerCount();
    bool IsCombatInProgress();
    Player *GetPlayerByGuid( uint32 pGuid );

    // Timers - reimplementation from MoonScriptCreatureAI
    uint32 AddTimer( int32 pDurationMillisec );
    int32 GetTimer( int32 pTimerId );
    void RemoveTimer( int32& pTimerId );
    void ResetTimer( int32 pTimerId, int32 pDurationMillisec );
    bool IsTimerFinished( int32 pTimerId );
    void CancelAllTimers();

    // Cells
    void SetCellForcedStates( float pMinX, float pMaxX, float pMinY, float pMaxY, bool pActivate = true );

protected:
    std::map<uint32, uint32> mTimers;
    int32 mTimerIdCounter;
};

#endif
