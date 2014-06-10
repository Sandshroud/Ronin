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

#include "StdAfx.h"

MapManagerScript::MapManagerScript(MapMgr* _internal)
{
    mTimerIdCounter = 0;
    _manager = _internal;
}

MapManagerScript::~MapManagerScript()
{

}

void MapManagerScript::Update(uint32 p_time)
{
    for ( std::map<uint32, uint32>::iterator TimerIter = mTimers.begin(), TimeIter2; TimerIter != mTimers.end(); )
    {
        TimeIter2 = TimerIter++;

        if(TimeIter2->second > p_time)
            TimeIter2->second -= p_time;
        else
            TimeIter2->second = 0;
    }

    for(map<uint32, CreatureAIScript*>::iterator itr = m_CreatureAIScripts.begin(); itr != m_CreatureAIScripts.end(); itr++)
        itr->second->AIUpdate(this, p_time);

    for(map<uint32, GameObjectAIScript*>::iterator itr = m_GameObjectAIScripts.begin(); itr != m_GameObjectAIScripts.end(); itr++)
        itr->second->AIUpdate(this, p_time);

    OnUpdate(p_time);
}

void MapManagerScript::AddCreatureAI(Creature* pCreature)
{
    if(pCreature == NULL)
        return;

    CreatureAIScript* GoScript = pCreature->GetScript();
    if(GoScript && !GoScript->LuaScript)
        m_CreatureAIScripts.insert(make_pair(pCreature->GetLowGUID(), GoScript));
}

void MapManagerScript::RemoveCreatureAI(Creature* pCreature)
{
    if(pCreature == NULL)
        return;

    if(m_CreatureAIScripts.find(pCreature->GetLowGUID()) == m_CreatureAIScripts.end())
        return;
    m_CreatureAIScripts.erase(pCreature->GetLowGUID());
}

void MapManagerScript::AddGameObjectAI(GameObject* pGameObject)
{
    if(pGameObject == NULL)
        return;

    GameObjectAIScript* GoScript = pGameObject->GetScript();
    if(GoScript && !GoScript->LuaScript)
        m_GameObjectAIScripts.insert(make_pair(pGameObject->GetLowGUID(), GoScript));
}

void MapManagerScript::RemoveGameObjectAI(GameObject* pGameObject)
{
    if(pGameObject == NULL)
        return;

    if(m_GameObjectAIScripts.find(pGameObject->GetLowGUID()) == m_GameObjectAIScripts.end())
        return;
    m_GameObjectAIScripts.erase(pGameObject->GetLowGUID());
}

Creature *MapManagerScript::GetCreatureByGuid( uint32 pGuid )
{
    return _manager->GetCreature(pGuid);
}

Creature *MapManagerScript::GetCreatureBySqlId( uint32 pSqlId )
{
    return _manager->GetSqlIdCreature(pSqlId);
}

Creature *MapManagerScript::FindClosestCreature( uint32 pEntry, float pX, float pY, float pZ )
{
    Creature *CurrentCreature = NULL, *Creturn = NULL;
    float closestDistance = 50000.0f, currentDistance;
    for ( uint32 i = 0; i != _manager->m_CreatureHighGuid; ++i )
    {
        currentDistance = 0.0f;
        CurrentCreature = _manager->m_CreatureStorage[i];
        if ( CurrentCreature != NULL )
        { 
            if ( CurrentCreature->GetEntry() == pEntry )
            {
                currentDistance = CurrentCreature->CalcDistance(pX, pY, pZ);
                if(currentDistance <= closestDistance)
                {
                    closestDistance = currentDistance;
                    Creturn = CurrentCreature;
                }
            }
        }
    }

    return Creturn;
}

Creature *MapManagerScript::FindClosestLivingCreature( uint32 pEntry, float pX, float pY, float pZ, Creature* check )
{
    Creature *CurrentCreature = NULL, *Creturn = NULL;
    float closestDistance = 50000.0f, currentDistance;
    for ( uint32 i = 0; i != _manager->m_CreatureHighGuid; ++i )
    {
        currentDistance = 0.0f;
        CurrentCreature = _manager->m_CreatureStorage[i];
        if ( CurrentCreature != NULL && CurrentCreature != check)
        { 
            if ( CurrentCreature->GetEntry() == pEntry && CurrentCreature->isAlive())
            {
                currentDistance = CurrentCreature->CalcDistance(pX, pY, pZ);
                if(currentDistance <= closestDistance)
                {
                    closestDistance = currentDistance;
                    Creturn = CurrentCreature;
                }
            }
        }
    }

    return Creturn;
}

Creature *MapManagerScript::SpawnCreature( uint32 pEntry, float pX, float pY, float pZ, float pO, int32 PhaseMask )
{
    CreatureProto * proto = CreatureProtoStorage.LookupEntry(pEntry);
    CreatureInfo * info = CreatureNameStorage.LookupEntry(pEntry);
    if(proto == NULL || info == NULL)
        return NULLCREATURE;

    Creature * p = _manager->CreateCreature(pEntry);
    p->SetInstanceID(_manager->GetInstanceID());
    p->Load(proto, pX, pY, pZ, pO);
    p->SetPhaseMask(PhaseMask);
    p->PushToWorld(_manager);
    return p;
}

Creature *MapManagerScript::SpawnCreature( uint32 pEntry, float pX, float pY, float pZ, float pO, uint32 pFactionId, int32 PhaseMask )
{
    CreatureProto * proto = CreatureProtoStorage.LookupEntry(pEntry);
    CreatureInfo * info = CreatureNameStorage.LookupEntry(pEntry);
    if(proto == NULL || info == NULL)
        return NULLCREATURE;

    Creature * p = _manager->CreateCreature(pEntry);
    p->SetInstanceID(_manager->GetInstanceID());
    p->Load(proto, pX, pY, pZ, pO);
    p->SetPhaseMask(PhaseMask);
    p->SetFaction(pFactionId);
    p->PushToWorld(_manager);
    return p;
}

GameObject *MapManagerScript::GetGameObjectBySqlId( uint32 pSqlId )
{
    return _manager->GetSqlIdGameObject( pSqlId );
}

GameObject *MapManagerScript::GetGameObjectByGuid( uint32 pGuid )
{
    return _manager->GetGameObject( pGuid );
}

GameObject *MapManagerScript::FindClosestGameObject( uint32 pEntry, float pX, float pY, float pZ )
{
    GameObject *CurrentObject = NULLGOB, *Creturn = NULL;
    float closestDistance = 50000.0f, currentDistance;
    for ( HM_NAMESPACE::hash_map<uint32, GameObject*>::iterator Iter = _manager->m_gameObjectStorage.begin(); Iter != _manager->m_gameObjectStorage.end(); Iter++ )
    {
        CurrentObject = ( *Iter ).second;
        if ( CurrentObject != NULL )
        { 
            if ( CurrentObject->GetEntry() == pEntry )
            {
                currentDistance = CurrentObject->CalcDistance(pX, pY, pZ);
                if(currentDistance <= closestDistance)
                {
                    closestDistance = currentDistance;
                    Creturn = CurrentObject;
                }
            }
        }
    }

    return Creturn;
}

GameObject *MapManagerScript::SpawnGameObject( uint32 pEntry, float pX, float pY, float pZ, float pO )
{
    GameObject* pGameObject = _manager->CreateGameObject(pEntry);
    if(pGameObject == NULL || !pGameObject->CreateFromProto(pEntry, _manager->GetMapId(), pX, pY, pZ, pO))
        return NULLGOB;

    pGameObject->SetInstanceID(_manager->GetInstanceID());
    pGameObject->PushToWorld(_manager);
    return pGameObject;
}

Player *MapManagerScript::FindClosestPlayer( float pX, float pY, float pZ, Player* pCheck )
{
    Player* CurrentPlayer = NULL, *Creturn = NULL;
    float closestDistance = 50000.0f, currentDistance;

    PlayerStorageMap::iterator itr;
    for(itr = _manager->m_PlayerStorage.begin(); itr != _manager->m_PlayerStorage.end(); ++itr)
    {
        currentDistance = 0.0f;
        CurrentPlayer = _manager->m_PlayerStorage[itr->first];

        //pCheck is used for mind control players to check. If it's not used, it's NULL.
        if ( CurrentPlayer != NULL && CurrentPlayer != pCheck)
        { 
            currentDistance = CurrentPlayer->CalcDistance(pX, pY, pZ);
            if(currentDistance <= closestDistance)
            {
                closestDistance = currentDistance;
                Creturn = CurrentPlayer;
            }
        }
    }

    return Creturn;
}

float MapManagerScript::GetRangeToObject( Object *pObjectA, Object *pObjectB )
{
    if ( pObjectA == NULLOBJ || pObjectB == NULLOBJ )
        return 0.0f;

    return GetRangeToObject( pObjectA->GetPositionX(), pObjectA->GetPositionY(), pObjectA->GetPositionZ(), pObjectB->GetPositionX(), pObjectB->GetPositionY(), pObjectB->GetPositionZ() );
}

float MapManagerScript::GetRangeToObject( Object *pObject, float pX, float pY, float pZ )
{
    if ( pObject == NULL )
        return 0.0f;

    return GetRangeToObject( pObject->GetPositionX(), pObject->GetPositionY(), pObject->GetPositionZ(), pX, pY, pZ );
}

float MapManagerScript::GetRangeToObject( float pX1, float pY1, float pZ1, float pX2, float pY2, float pZ2 )
{
    float dX = pX1 - pX2;
    float dY = pY1 - pY2;
    float dZ = pZ1 - pZ2;

    return sqrtf( dX * dX + dY * dY + dZ * dZ );
}

bool MapManagerScript::HasPlayers()
{
    return _manager->GetPlayerCount() > 0;
}

size_t MapManagerScript::GetPlayerCount()
{
    return _manager->GetPlayerCount();
}

Player *MapManagerScript::GetPlayerByGuid( uint32 pGuid )
{
    return _manager->GetPlayer( pGuid );
}

uint32 MapManagerScript::AddTimer( int32 pDurationMillisec )
{
    int32 Index = mTimerIdCounter++;
    mTimers.insert(make_pair( Index, pDurationMillisec ));
    return Index;
}

int32 MapManagerScript::GetTimer( int32 pTimerId )
{
    for ( std::map<uint32, uint32>::iterator TimerIter = mTimers.begin(); TimerIter != mTimers.end(); ++TimerIter )
        if ( TimerIter->first == pTimerId )
            return TimerIter->second;

    return 0;
}

void MapManagerScript::RemoveTimer( int32& pTimerId )
{
    if(mTimers.find(pTimerId) == mTimers.end())
        return;

    mTimers.erase(pTimerId);
    pTimerId = -1;
}

void MapManagerScript::ResetTimer( int32 pTimerId, int32 pDurationMillisec )
{
    std::map<uint32, uint32>::iterator timeitr;
    if((timeitr = mTimers.find(pTimerId)) == mTimers.end())
        return;

    timeitr->second = pDurationMillisec;
}

bool MapManagerScript::IsTimerFinished( int32 pTimerId )
{
    std::map<uint32, uint32>::iterator timeitr;
    if((timeitr = mTimers.find(pTimerId)) == mTimers.end())
        return false;

    return ( timeitr->second <= 0 );
}

void MapManagerScript::CancelAllTimers()
{
    mTimers.clear();
    mTimerIdCounter = 0;
}
