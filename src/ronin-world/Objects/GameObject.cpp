/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
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

GameObject::GameObject() : WorldObject() { }

void GameObject::Construct(GameObjectInfo *info, WoWGuid guid, uint32 fieldCount)
{
    WorldObject::Construct(guid, fieldCount);

    pInfo = info;

    SetTypeFlags(TYPEMASK_TYPE_GAMEOBJECT);
    m_objType = TYPEID_GAMEOBJECT;

    m_updateFlags |= UPDATEFLAG_STATIONARY_POS|UPDATEFLAG_ROTATION;

    counter = 0;
    _duelUpdateDiff = 0;
    bannerslot = bannerauraslot = -1;
    m_summonedGo = false;
    invisible = false;
    invisibilityFlag = INVIS_FLAG_NORMAL;
    m_triggerSpell = NULL;
    m_triggerRange = 0.f;
    m_summoner = NULL;
    charges = -1;
    m_ritualmembers = NULL;
    m_rotation.x = m_rotation.y = m_rotation.z = m_rotation.w = 0.f;
    m_quests = NULL;
    m_spawn = NULL;
    m_deleted = false;
    m_created = false;
    m_loadFailed = false;
    m_zoneVisibleSpawn = false;
    memset(m_Go_Uint32Values, 0, sizeof(uint32)*GO_UINT32_MAX);
    m_Go_Uint32Values[GO_UINT32_MINES_REMAINING] = 1;
    m_duelState = NULL;
    m_transportData = NULL;
    m_transportTaxiData = NULL;
}

GameObject::~GameObject()
{

}

void GameObject::Init()
{
    SetAnimProgress(0xFF);

    WorldObject::Init();
}

void GameObject::Destruct()
{
    if(m_ritualmembers)
        delete[] m_ritualmembers;
    if(m_duelState)
        delete m_duelState;

    if(uint32 guid = GetUInt32Value(GAMEOBJECT_FIELD_CREATED_BY))
    {
        Player* plr = objmgr.GetPlayer(guid);
        if(plr && plr->GetSummonedObject() == this)
            plr->SetSummonedObject(NULL);

        if(plr == m_summoner)
            m_summoner = NULL;
    }

    if (m_summonedGo && m_summoner)
    {
        for(int i = 0; i < 4; i++)
        {
            if (m_summoner->m_ObjectSlots[i] == GetGUID())
                m_summoner->m_ObjectSlots[i] = 0;
        }
    }

    if(m_transportData )
    {
        delete m_transportData;
        m_transportData = NULL;
    }

    if(m_transportTaxiData)
    {
        delete m_transportTaxiData;
        m_transportTaxiData = NULL;
    }

    WorldObject::Destruct();
}

void GameObject::Update(uint32 msTime, uint32 p_time)
{
    WorldObject::Update(msTime, p_time);

    switch(GetType())
    {
    case GAMEOBJECT_TYPE_DUEL_ARBITER:
        _updateDuelState(msTime, p_time);
        break;
    case GAMEOBJECT_TYPE_TRANSPORT:
        _updateTransportState(msTime, p_time);
        break;
    case GAMEOBJECT_TYPE_MO_TRANSPORT:
        _updateTransportTaxiState(msTime, p_time);
        break;
    }
}

void GameObject::InactiveUpdate(uint32 msTime, uint32 p_time)
{
    WorldObject::InactiveUpdate(msTime, p_time);
    switch(GetType())
    {
    case GAMEOBJECT_TYPE_DUEL_ARBITER:
        _updateDuelState(msTime, p_time);
        break;
    case GAMEOBJECT_TYPE_TRANSPORT:
        _updateTransportState(msTime, p_time);
        break;
    case GAMEOBJECT_TYPE_MO_TRANSPORT:
        _updateTransportTaxiState(msTime, p_time);
        break;
    }
}

void GameObject::OnFieldUpdated(uint16 index)
{
    if(GetType() == GAMEOBJECT_TYPE_CHAIR && index == OBJECT_FIELD_SCALE_X)
        _recalculateChairSeats();
}

void GameObject::OnPrePushToWorld()
{
    WorldObject::OnPrePushToWorld();

    if(m_mapInstance && m_zoneId && m_zoneVisibleSpawn)
        m_mapInstance->AddZoneVisibleSpawn(m_zoneId, this);
}

void GameObject::OnPushToWorld(uint32 msTime)
{
    WorldObject::OnPushToWorld(msTime);

    if(m_loadFailed == true) // We failed to load, queue us for cleanup
    {
        Deactivate(0);
        m_mapInstance->QueueCleanup(this);
    }
}

void GameObject::RemoveFromWorld()
{
    WorldObject::RemoveFromWorld();

    if(m_mapInstance && m_zoneId && m_zoneVisibleSpawn)
        m_mapInstance->RemoveZoneVisibleSpawn(m_zoneId, this);
}

void GameObject::EventExploration(MapInstance *instance)
{
    switch(GetType())
    {
    case GAMEOBJECT_TYPE_MO_TRANSPORT:
        UpdateAreaInfo(instance);
        break;
    }
}

bool GameObject::CanReactivate()
{
    switch(GetType())
    {
    case GAMEOBJECT_TYPE_MO_TRANSPORT:
        return m_transportTaxiData->isActive;
    }

    return true;
}

void GameObject::Reactivate()
{
    // Todo: Check spawn points and reset data for respawn event
    switch(GetType())
    {
    case GAMEOBJECT_TYPE_MO_TRANSPORT:
        m_mapInstance->ChangeObjectLocation(this);
        break;
    }
}

uint32 GameObject::BuildCreateUpdateBlockForPlayer(ByteBuffer *data, Player* target )
{
    return WorldObject::BuildCreateUpdateBlockForPlayer(data, target);
}

void GameObject::_recalculateChairSeats()
{
    bool newData = m_chairData.empty();
    if (pInfo->data.chair.slots > 1)
    {
        float size = GetFloatValue(OBJECT_FIELD_SCALE_X)*pInfo->sizeMod;
        float x_i = GetPositionX(), y_i = GetPositionY();
        float orthogonalOrientation = GetOrientation()+M_PI*0.5f;
        float relativeDistance = (pInfo->sizeMod*(pInfo->data.chair.slots-1)/1.25f);
        x_i += relativeDistance * cos(orthogonalOrientation);
        y_i += relativeDistance * sin(orthogonalOrientation);

        float step = pInfo->sizeMod*(pInfo->data.chair.slots/1.25f);
        for (uint32 i = 0; i < pInfo->data.chair.slots; ++i)
        {
            if(newData) m_chairData[i].user = 0;
            m_chairData[i].x = x_i;
            m_chairData[i].y = y_i;
            m_chairData[i].z = GetPositionZ();
            x_i -= step * cos(orthogonalOrientation);
            y_i -= step * sin(orthogonalOrientation);
        }
    }
    else
    {
        if(newData) m_chairData[0].user = 0;
        m_chairData[0].x = GetPositionX();
        m_chairData[0].y = GetPositionY();
        m_chairData[0].z = GetPositionZ();
    }
}

void GameObject::_searchNearbyUnits()
{
    if(GetState() != 1 || m_inTriggerRangeObjects.empty())
        return;
    if(m_summonedGo && !(m_summoner && m_summoner->isAlive()))
    {
        Deactivate(0);
        return;
    }
    return;

    SpellCastTargets tgt;
    tgt.m_targetMask |= (m_triggerSpell->isSpellAreaOfEffect() ? 0x40 : 0x02);
    tgt.m_src = tgt.m_dest = GetPosition();

    for(std::set<WoWGuid>::iterator itr = m_inTriggerRangeObjects.begin(); itr != m_inTriggerRangeObjects.end(); itr++)
    {
        if(Unit *pUnit = GetInRangeObject<Unit>(*itr))
        {
            if(pUnit == m_summoner)
                continue;
            if(m_summonedGo && !sFactionSystem.isAttackable(m_summoner, pUnit))
                continue;
            if(m_triggerSpell->HasEffect(SPELL_EFFECT_APPLY_AURA) && pUnit->HasAura(m_triggerSpell->Id))
                continue;

            tgt.m_unitTarget = *itr;
            /*if(Spell* sp = new Spell(this, m_triggerSpell))
                sp->prepare(&tgt, true);*/

            if(GetType() == GAMEOBJECT_TYPE_TRAP)
            {
                if(GetInfo()->data.trap.type == 1)
                    Deactivate(GetInfo()->GetSequenceTimer());
                return; // Trigger once
            }

            if(m_summonedGo)
            {
                Deactivate(0);
                return;
            }

            if(m_triggerSpell->isSpellAreaOfEffect())
                return;
        }
    }
}

void GameObject::_updateDuelState(uint32 msTime, uint32 p_diff)
{
    if(m_duelState == NULL)
        return;

    _duelUpdateDiff += p_diff;
    if(_duelUpdateDiff < 1000 && IsActivated())
        return;
    uint32 duelDiff = _duelUpdateDiff;
    _duelUpdateDiff = 0;

    switch(m_duelState->duelState)
    {
    case DUEL_STATE_REQUESTED:
        {
            m_duelState->duelCounter[1] += duelDiff;
            if(!m_duelState->quitter.empty() || m_duelState->duelCounter[1] > 60000)
                m_duelState->duelState = DUEL_STATE_FINISHED;
        }break;
    case DUEL_STATE_STARTED:
        if (m_duelState->duelCounter[0] == 0)
            m_duelState->duelCounter[0] = 3000;
        else if(m_duelState->duelCounter[0] <= duelDiff)
        {
            m_duelState->duelState = DUEL_STATE_OCCURING;
            m_duelState->duelCounter[0] = m_duelState->duelCounter[1] = 0;

            // Set players teams now that duel has started
            if(Player *plr = m_mapInstance->GetPlayer(m_duelState->duelists[0]))
                plr->SetUInt32Value(PLAYER_DUEL_TEAM, 1);
            if(Player *plr = m_mapInstance->GetPlayer(m_duelState->duelists[1]))
                plr->SetUInt32Value(PLAYER_DUEL_TEAM, 2);
        } else m_duelState->duelCounter[0] -= duelDiff;
        break;
    case DUEL_STATE_OCCURING:
        {
            WoWGuid winGuid;
            uint8 errorOut = DUEL_WINNER_DECIDING;
            Player *player1 = m_mapInstance->GetPlayer(m_duelState->duelists[0]), *player2 = m_mapInstance->GetPlayer(m_duelState->duelists[1]);
            if(player1 == NULL || !player1->IsInWorld() || player2 == NULL || !player2->IsInWorld())
                errorOut = DUEL_WINNER_RETREAT;
            Player *duelists[2] = { player1, player2 };

            if(!m_duelState->quitter.empty())
                winGuid = m_duelState->duelists[(m_duelState->quitter == m_duelState->duelists[0] ? 1 : 0)];
            else
            {
                if(errorOut == DUEL_WINNER_DECIDING)
                {
                    for(uint8 i = 0; i < 2; ++i)
                    {
                        float distance = duelists[i]->GetDistanceSq(this);
                        if(distance >= 1600.f)
                        {
                            if(m_duelState->duelCounter[i] == 0)
                            {
                                duelists[i]->PushData(SMSG_DUEL_OUTOFBOUNDS);
                                m_duelState->duelCounter[i] = 10000;
                            }
                            else if(m_duelState->duelCounter[i] <= duelDiff || distance >= 1900.f)
                            {
                                m_duelState->duelCounter[i] = 1;
                                errorOut = DUEL_WINNER_RETREAT;
                                winGuid = m_duelState->duelists[i == 0 ? 1 : 0];
                                break; // End our loop here since duel is over
                            } else m_duelState->duelCounter[i] -= duelDiff;
                        }
                        else
                        {
                            if(m_duelState->duelCounter[i] > 0)
                                duelists[i]->PushData(SMSG_DUEL_INBOUNDS);
                            m_duelState->duelCounter[i] = 0;
                        }
                    }
                }
            }

            if(!winGuid.empty())
            {
                m_duelState->duelCounter[0] = m_duelState->duelCounter[1] = 0;
                m_duelState->duelState = DUEL_STATE_FINISHED;
                uint8 winnerIndex = (winGuid == m_duelState->duelists[0] ? 0 : 1), loserIndex = (winnerIndex == 0 ? 1 : 0);

                //Announce Winner
                WorldPacket data( SMSG_DUEL_WINNER, 500 );
                data << uint8( DUEL_WINNER_RETREAT );
                data << m_duelState->duelistNames[winnerIndex].c_str();
                data << m_duelState->duelistNames[loserIndex].c_str();
                SendMessageToSet(&data, true);

                static uint8 complete = 1; // Duel complete
                for(uint8 i = 0; i < 2; ++i)
                {
                    uint8 tI = i == 0 ? 1 : 0;
                    if(Player *plr = duelists[i])
                    {
                        plr->SetUInt32Value(PLAYER_DUEL_TEAM, 0);
                        plr->SetUInt32Value(PLAYER_DUEL_ARBITER, 0);
                        plr->PushData(SMSG_DUEL_COMPLETE, 1, &complete);

                        // Stop victim attack
                        if(plr->checkAttackTarget(m_duelState->duelists[tI]))
                            plr->EventAttackStop();
                    }
                }
            }
        }break;
    case DUEL_STATE_FINISHED:
        {
            if(m_duelState->duelCounter[0] == 0)
            {
                // Set our cleanup counter
                m_duelState->duelCounter[0] = 2000;
                // Cleanup duel information for players and arbiter
                Player *plr;
                if((plr = m_mapInstance->GetPlayer(m_duelState->duelists[0])) && plr->GetDuelStorage() == m_duelState)
                    plr->SetDuel(NULL);
                if((plr = m_mapInstance->GetPlayer(m_duelState->duelists[1])) && plr->GetDuelStorage() == m_duelState)
                    plr->SetDuel(NULL);
                // Send despawn animation packet to inrange
                WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
                data << GetGUID();
                SendMessageToSet(&data, true);
                // Use deactivate to queue destruction
                Deactivate(10000);
            }
            else if(m_duelState->duelCounter[0] <= duelDiff)
            {
                delete m_duelState;
                m_duelState = NULL;
                Cleanup();
            } else m_duelState->duelCounter[0] -= duelDiff;
        }break;
    }
}

void GameObject::_updateTransportState(uint32 msTime, uint32 p_diff)
{
    if(m_transportData == NULL)
        return;

    // Update our transport data tick
    m_transportData->transportTick += p_diff;
}

void GameObject::_updateTransportTaxiState(uint32 msTime, uint32 p_diff)
{
    ASSERT(m_transportTaxiData != NULL);

    if(m_transportTaxiData->isActive == false)
    {
        // Check our activation schedule with our transport manager
        if(sTransportMgr.CheckTransportPosition(GetGUID(), m_mapId))
            m_transportTaxiData->isActive = true;
        return;
    }

    m_transportTaxiData->transportTick += p_diff;
    // If we have a transport delay, don't update our tick timer
    if(m_transportTaxiData->m_transportDelay > p_diff && ((m_transportTaxiData->m_transportDelay -= p_diff) > 0))
        return;

    m_transportTaxiData->m_transportDelay = 0;
    m_transportTaxiData->transportTravelTick += p_diff;

    // We're at the timer point where our path ends, so cycle into our deactivated state if we transfer maps, or start over
    if(m_transportTaxiData->transportTravelTick >= m_transportTaxiData->pathTravelTime)
    {   // End path
        TaxiPath::posPoint *currPoint = &m_transportTaxiData->movementPath->m_pathData[0];
        // Reset our position to the start of our path and clear any processed data/timers
        SetPosition(currPoint->x, currPoint->y, currPoint->z, 0.f);
        m_transportTaxiData->m_processedDelayTimers.clear();
        m_transportTaxiData->transportTravelTick = 0;

        // Reset our tick timer to our stored start time
        m_transportTaxiData->transportTick = m_transportTaxiData->pathStartTime;

        if(m_transportTaxiData->changesMaps)
        {
            Deactivate(0);
            m_transportTaxiData->isActive = false;
            // Tell our transportmgr that we're inactive now
            sTransportMgr.ChangeTransportActiveMap(GetGUID(), m_mapId);
        }
    }
    else
    {
        // We have taxi delays and at least one is still pending
        if(m_transportTaxiData->m_taxiDelayTimers.size() != m_transportTaxiData->m_processedDelayTimers.size())
        {
            uint32 timer = 0;
            for(auto itr = m_transportTaxiData->m_taxiDelayTimers.begin(); itr != m_transportTaxiData->m_taxiDelayTimers.end(); ++itr)
            {
                if(m_transportTaxiData->m_processedDelayTimers.find(itr->first) != m_transportTaxiData->m_processedDelayTimers.end())
                    continue;
                if(itr->first >= m_transportTaxiData->transportTravelTick)
                    break; // We can stop looking here since these are ordered by time

                // Push the delay timer to processed and pass our received time forward
                m_transportTaxiData->m_processedDelayTimers.insert(itr->first);
                timer = itr->second;
            }

            if(timer != 0)
            {
                uint32 lastPathTime = 0;
                TaxiPath::posPoint *lastPoint = NULL;
                // Find the point that matches up with our current travel time, we can also use our delay timer but for now keep the processing for safety
                for(size_t i = 0; i < m_transportTaxiData->m_pathTimers.size(); ++i)
                {
                    uint32 pathTime = m_transportTaxiData->m_pathTimers[i];
                    TaxiPath::posPoint *currPoint = &m_transportTaxiData->movementPath->m_pathData[i];
                    if(pathTime > m_transportTaxiData->transportTravelTick)
                        break;
                    lastPathTime = pathTime;
                    lastPoint = currPoint;
                }

                uint32 pathDiff = m_transportTaxiData->transportTravelTick-lastPathTime;

                // Play our docking noise
                PlaySoundToSet(5495);

                // Set our position to our last point
                SetPosition(lastPoint->x, lastPoint->y, lastPoint->z, 0.f);
                // Update our tick time to match the last path time
                m_transportTaxiData->transportTravelTick = lastPathTime;

                // Set our delay to our new delay timer minus the difference for our update
                m_transportTaxiData->m_transportDelay = timer-pathDiff;

                // We've processed this time so push it into our handle queue
                m_transportTaxiData->m_processedDelayTimers.insert(timer);
                return;
            }
        }

        uint32 lastPointTimer = 0, nextPointTimer = 0;
        TaxiPath::posPoint *lastPoint = NULL, *nextPoint = NULL;
        for(size_t i = 0; i < m_transportTaxiData->m_pathTimers.size(); ++i)
        {
            uint32 pathTime = m_transportTaxiData->m_pathTimers[i];
            TaxiPath::posPoint *currPoint = &m_transportTaxiData->movementPath->m_pathData[i];
            if(pathTime == m_transportTaxiData->transportTravelTick)
            {   // We're at this point, end here
                lastPointTimer = pathTime;
                lastPoint = currPoint;
                break;
            }
            else if(pathTime > m_transportTaxiData->transportTravelTick)
            {
                nextPointTimer = pathTime;
                nextPoint = currPoint;
                break;
            }

            lastPointTimer = pathTime;
            lastPoint = currPoint;
        }

        // We should have a last point as long as we have precalculated timers
        if(lastPoint)
        {
            float x = lastPoint->x, y = lastPoint->y, z = lastPoint->z, timeDiff = m_transportTaxiData->transportTravelTick-lastPointTimer;
            if(nextPoint && timeDiff > 0.f)
            {
                float p = (timeDiff/(nextPointTimer-lastPointTimer));
                x -= ((x-nextPoint->x)*p);
                y -= ((y-nextPoint->y)*p);
                z -= ((z-nextPoint->z)*p);
            }

            SetPosition(x, y, z, 0.f);
        }
    }
}

void GameObject::SaveToDB()
{
    if(m_spawn == NULL)
        return;

    std::stringstream ss;
    ss << "REPLACE INTO gameobject_spawns VALUES("
        << GetLowGUID() << ","
        << GetEntry() << ","
        << GetMapId() << ","
        << GetPositionX() << ","
        << GetPositionY() << ","
        << GetPositionZ() << ","
        << m_rotation.x << ", "
        << m_rotation.y << ", "
        << m_rotation.z << ", "
        << m_rotation.w << ", "
        << uint32(GetState()) << ","
        << GetFlags() << ","
        << GetUInt32Value(GAMEOBJECT_FACTION) << ","
        << GetFloatValue(OBJECT_FIELD_SCALE_X) << ","
        << m_spawn->eventId << ")";

    WorldDatabase.Execute(ss.str().c_str());
}

void GameObject::InitAI()
{
    switch(pInfo->Type)
    {
    case GAMEOBJECT_TYPE_CHEST:
        {
            bool requiresQuest = false, calcMineRemain = false;
            if(LockEntry *pLock = dbcLock.LookupEntry(pInfo->GetLockID()))
            {
                for(uint32 i = 0; i < 8; i++)
                {
                    switch(pLock->locktype[i])
                    {
                    case 1:
                        {
                            if(ItemPrototype *proto = sItemMgr.LookupEntry(pLock->lockmisc[i]))
                                requiresQuest = (proto->Class == ITEM_CLASS_QUEST);
                        }break;
                    case 2:
                        {
                            //herbalism and mining;
                            if(calcMineRemain == false && (pLock->lockmisc[i] == LOCKTYPE_MINING || pLock->lockmisc[i] == LOCKTYPE_HERBALISM))
                                calcMineRemain = true;
                        }break;

                    }
                }
            }

            if(calcMineRemain)
                CalcMineRemaining(true);

            if(requiresQuest)
            {
                // Force a dynamic flag for our two affected fields
                m_dynamicFields.SetBit(GAMEOBJECT_FLAGS);
                m_dynamicFields.SetBit(GAMEOBJECT_BYTES_1);
                // We're dynamic, but default values should be locked
                SetFlags(GetStartFlags() | GO_FLAG_IN_USE);
                SetState(GO_STATE_FORCE_ACTIVATED);
            }
            return;
        }break;
    case GAMEOBJECT_TYPE_TRAP:
        {
            m_triggerSpell = dbcSpell.LookupEntry(pInfo->GetSpellID());
            m_triggerRange = std::max<float>(3.f, pInfo->data.trap.radius);
            m_triggerRange *= m_triggerRange;
        }break;
    case GAMEOBJECT_TYPE_CHAIR:
        {
            _recalculateChairSeats();
            return;
        }break;
    case GAMEOBJECT_TYPE_SPELL_FOCUS://redirect to properties of another go
        {
            if( pInfo->data.spellFocus.linkedTrapId == 0 )
                return;

            uint32 objectid = pInfo->data.spellFocus.linkedTrapId;
            GameObjectInfo* gopInfo = GameObjectNameStorage.LookupEntry( objectid );
            if(gopInfo == NULL)
            {
                sLog.Warning("GameObject", "Redirected gameobject %u doesn't seem to exists in database, skipping", objectid);
                return;
            }

            m_triggerSpell = dbcSpell.LookupEntry(gopInfo->data.raw.data[4]);
            return;
        }break;
    case GAMEOBJECT_TYPE_GOOBER:
        {
            if(pInfo->data.goober.questId && !GetLoot()->items.empty())
            {
                SetFlags(GetStartFlags() | GO_FLAG_IN_USE);
                SetState(GO_STATE_FORCE_ACTIVATED);
            }
        }break;
    case GAMEOBJECT_TYPE_TRANSPORT:
        {
            m_transportData = new GameObject::TransportData();
            m_transportData->transportTick = 0;
            m_transportData->currentPos.ChangeCoords(0.f, 0.f, 0.f, 0.f);
        }break;
    case GAMEOBJECT_TYPE_MO_TRANSPORT:
        {
            // Create our transporter data
            m_transportTaxiData = new GameObject::TransportTaxiData();
            if(!sTransportMgr.RegisterTransport(this, m_mapId, m_transportTaxiData))
            {
                m_loadFailed = true;
                return;
            }

            // Make us a zone spawn and force our transport flag
            m_zoneVisibleSpawn = true;
            m_updateFlags |= UPDATEFLAG_TRANSPORT|UPDATEFLAG_DYN_MODEL;
            m_transportTaxiData->transportTravelTick = 0;

            SetUInt32Value(GAMEOBJECT_LEVEL, m_transportTaxiData->calculatedPathTimer);
            SetState(GO_STATE_ACTIVATED);
            SetFlags(GO_FLAG_TRIGGERED);
            SetAnimProgress(100);
        }break;
    case GAMEOBJECT_TYPE_RITUAL:
        {
            if(m_ritualmembers)
                return;

            m_ritualmembers = new uint32[pInfo->data.ritual.reqParticipants];
            memset(m_ritualmembers, 0, (sizeof(uint32)*(pInfo->data.ritual.reqParticipants)));
        }break;
    case GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING:
        {
            m_Go_Uint32Values[GO_UINT32_HEALTH] = pInfo->data.building.intactNumHits+pInfo->data.building.damagedNumHits;
            m_zoneVisibleSpawn = true;
            SetAnimProgress(255);
        }break;
    case GAMEOBJECT_TYPE_AURA_GENERATOR:
        {
            m_triggerSpell = dbcSpell.LookupEntry(GetInfo()->data.auraGenerator.auraID1);
            m_triggerRange = GetInfo()->data.auraGenerator.radius;
            m_triggerRange *= m_triggerRange;
        }break;
    }

    if(m_triggerSpell == NULL)
        return;
    if(m_triggerRange == 0.f)
    {   //ok got valid spell that will be casted on target when it comes close enough
        //get the range for that
        float r = 0;
        for(uint32 i = 0; i < 3; ++i)
        {
            if(m_triggerSpell->Effect[i])
            {
                float t = m_triggerSpell->radiusEnemy[i];
                if(t > r)
                    r = t;
            }
        }

        if(r < 0.1f)//no range
            r = m_triggerSpell->maxRange[0];
        m_triggerRange = r*r;//square to make code faster
    }

    m_eventHandler.AddStaticEvent(this, &GameObject::_searchNearbyUnits, pInfo->GetSequenceTimer());
}

void GameObject::Load(uint32 mapId, float x, float y, float z, float angleOverride, float rX, float rY, float rZ, float rAngle, GameObjectSpawn *spawn)
{
    // Set our spawn pointer
    m_spawn = spawn;

    // Call create function
    WorldObject::_Create( mapId, x, y, z, 0.f );

    float rotZ = rZ, rotAng = rAngle;
    if(angleOverride != 0.f)
    {
        rotZ = std::sin(angleOverride/2.f);
        rotAng = std::cos(angleOverride/2.f);
    }

    // Update our rotation data
    UpdateRotations(rX, rY, rotZ, rotAng);

    // Load data from gob info
    SetFlags(spawn ? spawn->flags : pInfo->DefaultFlags);
    SetDisplayId(pInfo->DisplayID);
    SetType(pInfo->Type);
    SetState(GO_STATE_READY_TO_ACTIVATE);
    if(pInfo->IsDummyObject())
    {
        SetFlags(GetStartFlags() | GO_FLAG_IN_USE);
        SetState(GO_STATE_FORCE_ACTIVATED);
    }

    // Event objects should be spawned inactive
    if(m_spawn && (m_spawn->eventId || m_spawn->conditionId))
    {
        if(m_spawn->eventId) m_inactiveFlags |= OBJECT_INACTIVE_FLAG_EVENTS;
        if(m_spawn->conditionId) m_inactiveFlags |= OBJECT_INACTIVE_FLAG_CONDITION;
        Deactivate(5000);
    }

    // Load spawn specific data
    if(spawn)
    {
        // Gameobject scale from spawndata
        SetFloatValue(OBJECT_FIELD_SCALE_X, spawn->scale);

        // last state
        SetState(spawn->state);

        // Set our phase mask
        m_phaseMask = spawn->phaseMask;

        // Custom object faction setting per spawn
        if(spawn->faction)
        {
            SetUInt32Value(GAMEOBJECT_FACTION, spawn->faction);
            m_factionTemplate = dbcFactionTemplate.LookupEntry(spawn->faction);
        }
    }

    // Transports and flag checks
    if(GetType() == GAMEOBJECT_TYPE_TRANSPORT)
    {
        SetFlag(GAMEOBJECT_FLAGS, (GO_FLAG_TRANSPORT | GO_FLAG_NODESPAWN));
        m_updateFlags |= UPDATEFLAG_TRANSPORT;
        SetState(GO_STATE_TRANSPORT_ACTIVE);
    }
    else if( GetFlags() & (GO_FLAG_IN_USE|GO_FLAG_LOCKED) )
        SetAnimProgress(100);

    // Trigger AI setup
    InitAI();

    // Trigger on creation call
    TRIGGER_GO_EVENT(castPtr<GameObject>(this), OnCreate);

    // Load quest data
    _LoadQuests();
}

uint32 GameObject::BuildStopFrameData(ByteBuffer *buff)
{
    uint32 frameCount = 0, stopFrame = 0;
    if((stopFrame = pInfo->data.transport.stopFrame1) > 0)
        frameCount++, *buff << uint32(stopFrame);
    if((stopFrame = pInfo->data.transport.stopFrame2) > 0)
        frameCount++, *buff << uint32(stopFrame);
    if((stopFrame = pInfo->data.transport.stopFrame3) > 0)
        frameCount++, *buff << uint32(stopFrame);
    if((stopFrame = pInfo->data.transport.stopFrame4) > 0)
        frameCount++, *buff << uint32(stopFrame);
    return frameCount;
}

void GameObject::UpdateRotations(float rX, float rY, float rZ, float rAngle)
{
    if(pInfo->Type == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
    {
        m_rotation.x = rX;
        m_rotation.y = rY;
        SetUInt64Value(GAMEOBJECT_PARENTROTATION, pInfo->data.building.destructibleData);
    }
    else
    {
        SetFloatValue(GAMEOBJECT_PARENTROTATION+0, (m_rotation.x = rX));
        SetFloatValue(GAMEOBJECT_PARENTROTATION+1, (m_rotation.y = rY));
    }

    SetFloatValue(GAMEOBJECT_PARENTROTATION+2, (m_rotation.z = rZ));
    SetFloatValue(GAMEOBJECT_PARENTROTATION+3, (m_rotation.w = rAngle));
    SetOrientation(m_rotation.toAxisAngleRotation());
}

int64 GameObject::PackRotation(ObjectRotation *rotation)
{
    int8 w_sign = (rotation->w >= 0 ? 1 : -1);
    int64 X = int32(rotation->x * (1 << 21)) * w_sign & ((1 << 22) - 1);
    int64 Y = int32(rotation->y * (1 << 20)) * w_sign & ((1 << 21) - 1);
    int64 Z = int32(rotation->z * (1 << 20)) * w_sign & ((1 << 21) - 1);
    return uint64(Z | (Y << 21) | (X << 42));
}

void GameObject::DeleteFromDB()
{

}

void GameObject::EventCloseDoor()
{
    SetState(GO_STATE_READY_TO_ACTIVATE);
}

void GameObject::InitializeDuelData(Player *player1, Player *player2)
{
    if(m_duelState)
        return;

    // Construct our new duel state pointer
    m_duelState = new DuelStorage();
    m_duelState->duelCounter[0] = 3000;
    m_duelState->duelCounter[1] = 0;
    m_duelState->duelState = 0;
    m_duelState->duelists[0] = player1->GetGUID();
    m_duelState->duelists[1] = player2->GetGUID();
    m_duelState->duelistNames[0] = player1->GetName();
    m_duelState->duelistNames[1] = player2->GetName();

    // Link duel state to our players
    player1->SetDuel(m_duelState);
    player2->SetDuel(m_duelState);

    // Set player arbiter
    player1->SetUInt64Value(PLAYER_DUEL_ARBITER, GetGUID());
    player2->SetUInt64Value(PLAYER_DUEL_ARBITER, GetGUID());

    // Set gameobject data
    SetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY, player1->GetGUID());
    SetUInt32Value(GAMEOBJECT_FACTION, player1->GetFactionID());
    SetUInt32Value(GAMEOBJECT_LEVEL, player1->getLevel());

    // Initialize duel by sending request packet
    WorldPacket data(SMSG_DUEL_REQUESTED, 16);
    data << GetGUID();
    data << player1->GetGUID();
    player2->PushPacket(&data);
}

void GameObject::DuelEnd(WorldObject *killer, Player *victim, uint8 status)
{
    if(m_duelState == NULL)
        return;

    m_duelState->duelState = DUEL_STATE_FINISHED;
    // Grab our index to utilize stored duelists names
    uint8 winnerIndex = ((victim->GetGUID() == m_duelState->duelists[0]) ? 1 : 0), loserIndex = (winnerIndex == 0 ? 1 : 0);

    // Clear out victim data
    victim->SetUInt32Value(PLAYER_DUEL_TEAM, 0);
    victim->SetUInt64Value(PLAYER_DUEL_ARBITER, 0);

    static uint8 complete = 1; // Duel complete
    victim->PushData(SMSG_DUEL_COMPLETE, 1, &complete);

    //Announce Winner
    WorldPacket duelWinner( SMSG_DUEL_WINNER, 500 );
    duelWinner << uint8( status );
    duelWinner << m_duelState->duelistNames[winnerIndex].c_str();
    duelWinner << m_duelState->duelistNames[loserIndex].c_str();
    SendMessageToSet( &duelWinner, true );

    if(Player *winner = m_mapInstance->GetPlayer(m_duelState->duelists[winnerIndex]))
    {
        // Stop winner attack
        if(winner->checkAttackTarget(victim->GetGUID()))
            winner->EventAttackStop();

        // Clear winner team data
        winner->SetUInt32Value(PLAYER_DUEL_TEAM, 0);
        winner->SetUInt64Value(PLAYER_DUEL_ARBITER, 0);
        // Duel complete
        winner->PushData(SMSG_DUEL_COMPLETE, 1, &complete);
        // Stop victim attack
        if(victim->checkAttackTarget(winner->GetGUID()))
            victim->EventAttackStop();
    }
}

bool GameObject::IsInDuelRadius(LocationVector vec)
{
    if(GetDistanceSq(vec.x, vec.y, vec.z) >= 1900.f)
        return false;
    return true;
}

void GameObject::UseFishingNode(Player* player)
{
    if( GetUInt32Value( GAMEOBJECT_FLAGS ) != 32 ) // Clicking on the bobber before something is hooked
    {
        player->PushData( SMSG_FISH_NOT_HOOKED );
        EndFishing( player, true );
        return;
    }

    uint32 minskill = 0, maxskill = 500;
    if( player->getSkillLineVal( SKILL_FISHING, false ) < maxskill )
        player->ModSkillLineAmount( SKILL_FISHING, float2int32( 1.0f * sWorld.getRate( RATE_SKILLRATE ) ), false );

    // Open loot on success, otherwise FISH_ESCAPED.
    if( Rand(((player->getSkillLineVal( SKILL_FISHING, true ) - minskill) * 100) / maxskill) )
    {
        lootmgr.FillFishingLoot( GetLoot(), GetZoneId() );
        player->SendLoot( GetGUID(), GetMapId(), LOOTTYPE_FISHING );
        EndFishing( player, false );
    }
    else // Failed
    {
        player->PushData( SMSG_FISH_ESCAPED );
        EndFishing( player, true );
    }

}

void GameObject::EndFishing(Player* player, bool abort )
{
    return;
    Spell* spell = NULL;//player->GetCurrentSpell();

    if(spell)
    {
        if(abort)   // abort becouse of a reason
        {
            //FIXME: here 'failed' should appear over progress bar
            spell->SendChannelUpdate(0);
            spell->finish();
        }
        else        // spell ended
        {
            spell->SendChannelUpdate(0);
            spell->finish();
        }
    }

    Deactivate(0);
}

void GameObject::FishHooked(Player* player)
{
    WorldPacket data(SMSG_GAMEOBJECT_CUSTOM_ANIM, 12);
    data << GetGUID();
    data << (uint32)0; // value < 4
    player->PushPacket(&data);
    SetFlags(32);
 }

/////////////
/// Quests

void GameObject::AddQuest(QuestRelation *Q)
{
    m_quests->push_back(Q);
}

void GameObject::DeleteQuest(QuestRelation *Q)
{
    std::list<QuestRelation *>::iterator it;
    for( it = m_quests->begin(); it != m_quests->end(); it++ )
    {
        if( ( (*it)->type == Q->type ) && ( (*it)->qst == Q->qst ) )
        {
            delete (*it);
            m_quests->erase(it);
            break;
        }
    }
}

Quest* GameObject::FindQuest(uint32 quest_id, uint8 quest_relation)
{
    std::list< QuestRelation* >::iterator it;
    for( it = m_quests->begin(); it != m_quests->end(); it++ )
    {
        QuestRelation* ptr = (*it);
        if( ( ptr->qst->id == quest_id ) && ( ptr->type & quest_relation ) )
        {
            return ptr->qst;
        }
    }
    return NULL;
}

uint16 GameObject::GetQuestRelation(uint32 quest_id)
{
    uint16 quest_relation = 0;
    std::list< QuestRelation* >::iterator it;
    for( it = m_quests->begin(); it != m_quests->end(); it++ )
    {
        if( (*it) != NULL && (*it)->qst->id == quest_id )
        {
            quest_relation |= (*it)->type;
        }
    }
    return quest_relation;
}

uint32 GameObject::NumOfQuests()
{
    return (uint32)m_quests->size();
}

void GameObject::_LoadQuests()
{
    sQuestMgr.LoadGOQuests(castPtr<GameObject>(this));

    // set state for involved quest objects
    if( pInfo && lootmgr.GetGameObjectQuestLoot(pInfo->ID) )
    {
        SetUInt32Value(GAMEOBJECT_DYNAMIC, 0);
        SetState(GO_STATE_ACTIVATED);
        SetFlags(GO_FLAG_IN_USE);
    }
}

/////////////////
// Summoned Go's
//guardians are temporary spawn that will inherit master faction and will folow them. Apart from that they have their own mind
Unit* GameObject::CreateTemporaryGuardian(uint32 guardian_entry,uint32 duration,float angle, Unit* u_caster, uint8 Slot)
{
    Creature* p = GetMapInstance()->CreateCreature(guardian_entry);
    if(p == NULL)
    {
        sLog.outDebug("Warning : Missing summon creature template %u !",guardian_entry);
        return NULL;
    }

    LocationVector v = GetPositionNC();
    float m_followAngle = angle + v.o;
    float x = v.x +(3*(cosf(m_followAngle)));
    float y = v.y +(3*(sinf(m_followAngle)));
    p->Load(GetMapId(), x, y, v.z, angle, GetMapInstance()->iInstanceMode);
    p->SetInstanceID(GetMapInstance()->GetInstanceID());
    p->setLevel(u_caster->getLevel());

    p->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, GetGUID());
    p->SetUInt64Value(UNIT_FIELD_CREATEDBY, GetGUID());
    p->SetFactionTemplate(u_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    GetMapInstance()->AddObject(p);
    return p;

}

uint32 GameObject::GetGOReqSkill()
{
    if(GetInfo() == NULL)
        return 0;

    LockEntry *lock = dbcLock.LookupEntry( GetInfo()->GetLockID() );
    if(!lock)
        return 0;
    for(uint32 i=0; i < 8; ++i)
    {
        if(lock->locktype[i] == 2 && lock->minlockskill[i])
        {
            return lock->minlockskill[i];
        }
    }
    return 0;
}

void GameObject::GenerateLoot()
{

}

bool GameObject::CanUse(Player *plr)
{
    switch(GetType())
    {
    case GAMEOBJECT_TYPE_CHEST:
        {
            if(LockEntry *pLock = dbcLock.LookupEntry(pInfo->GetLockID()))
            {
                for(uint32 i = 0; i < 8; i++)
                {
                    if(pLock->locktype[i] == 1 && plr->GetInventory()->GetItemCount(pLock->lockmisc[i]))
                        return true;
                }
            }

        }
    }
    return false;
}

void GameObject::SetState(uint8 state)
{
    SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE, state);
    if(IsInWorld())
        m_mapInstance->SetGameObjState(m_spawn->guid, state);
}

void GameObject::SetDisplayId(uint32 id)
{
    SetUInt32Value( GAMEOBJECT_DISPLAYID, id );
    if(IsInWorld())
    {
        sVMapInterface.UpdateObjectModel(GetGUID(), GetMapId(), GetInstanceID(), id);
    }
}

uint32 GameObject::GetTransportTick()
{
    if(m_transportTaxiData)
        return m_transportTaxiData->transportTick;
    if(m_transportData)
        return m_transportData->transportTick;
    return 0;
}

//Destructable Buildings
void GameObject::TakeDamage(uint32 amount, WorldObject* mcaster, Player* pcaster, uint32 spellid)
{
    if(GetType() != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return;

    if(HasFlag(GAMEOBJECT_FLAGS,GO_FLAG_DESTROYED)) // Already destroyed
        return;

    uint32 IntactHealth = pInfo->data.building.intactNumHits;
    uint32 DamagedHealth = pInfo->data.building.damagedNumHits;

    if(m_Go_Uint32Values[GO_UINT32_HEALTH] > amount)
        m_Go_Uint32Values[GO_UINT32_HEALTH] -= amount;
    else
        m_Go_Uint32Values[GO_UINT32_HEALTH] = 0;

    if(HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED))
    {
        if(m_Go_Uint32Values[GO_UINT32_HEALTH] == 0)
            SetStatusDestroyed();
    }
    else if(!HasFlag(GAMEOBJECT_FLAGS,GO_FLAG_DAMAGED) && m_Go_Uint32Values[GO_UINT32_HEALTH] <= DamagedHealth)
    {
        if(m_Go_Uint32Values[GO_UINT32_HEALTH] != 0)
            SetStatusDamaged();
        else SetStatusDestroyed();
    }

    WorldPacket data(SMSG_DESTRUCTIBLE_BUILDING_DAMAGE, 20);
    data << GetGUID();
    data << mcaster->GetGUID().asPacked();
    data << (pcaster ? pcaster->GetGUID() : mcaster->GetGUID()).asPacked();
    data << uint32(amount);
    data << spellid;
    mcaster->SendMessageToSet(&data, (mcaster->IsPlayer() ? true : false));
    if(IntactHealth != 0 && DamagedHealth != 0)
        SetAnimProgress(m_Go_Uint32Values[GO_UINT32_HEALTH]*255/(IntactHealth + DamagedHealth));
}

void GameObject::SetStatusRebuilt()
{
    RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED | GO_FLAG_DESTROYED);
    SetDisplayId(pInfo->DisplayID);
    uint32 IntactHealth = pInfo->data.building.intactNumHits;
    uint32 DamagedHealth = pInfo->data.building.damagedNumHits;
    m_Go_Uint32Values[GO_UINT32_HEALTH] = IntactHealth + DamagedHealth;
}

void GameObject::SetStatusDamaged()
{
    SetFlags(GO_FLAG_DAMAGED);
    if(pInfo->data.building.destructibleData != 0)
    {
        if(DestructibleModelDataEntry *display = NULL)//dbcDestructibleModelDataEntry.LookupEntry( pInfo->DestructableBuilding.DestructibleData ))
            SetDisplayId(display->GetDisplayId(1));
    } else SetDisplayId(pInfo->data.building.damagedDisplayId);
}

void GameObject::SetStatusDestroyed()
{
    RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED);
    SetFlags(GO_FLAG_DESTROYED);
    if(pInfo->data.building.destructibleData != 0)
    {
        if(DestructibleModelDataEntry *display = NULL)//dbcDestructibleModelDataEntry.LookupEntry( pInfo->DestructableBuilding.DestructibleData ))
            SetDisplayId(display->GetDisplayId(3));
    } else SetDisplayId(pInfo->data.building.destroyedDisplayId);
}

#define OPEN_CHEST 11437

void GameObject::Use(Player *p)
{
    SpellEntry *spellInfo = NULL;
    GameObjectInfo *goinfo = GetInfo();
    if (!goinfo)
        return;

    uint32 type = GetType();
    TRIGGER_GO_EVENT(this, OnActivate);
    TRIGGER_INSTANCE_EVENT( p->GetMapInstance(), OnGameObjectActivate )( this, p );

    switch (type)
    {
    case GAMEOBJECT_TYPE_CHAIR:
        {
            if(m_chairData.empty())
                return;
            if(goinfo->data.chair.onlyCreatorUse && p->GetGUID() != GetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY))
                return;

            p->Dismount();

            float lowestDist = 90.f;
            uint32 nearest_slot = 0xFF;
            for (ChairSlotAndUser::iterator itr = m_chairData.begin(); itr != m_chairData.end(); ++itr)
            {
                if(!itr->second.user.empty())
                {
                    if (Player* ChairUser = objmgr.GetPlayer(itr->second.user))
                    {
                        if (ChairUser->IsSitting() && ChairUser->GetDistance2dSq(itr->second.x, itr->second.y) < 0.1f)
                            continue;
                    }
                    itr->second.user.Clean();
                }

                float thisDistance = p->GetDistance2dSq(itr->second.x, itr->second.y);
                if (thisDistance <= lowestDist)
                {
                    nearest_slot = itr->first;
                    lowestDist = thisDistance;
                }
            }

            if (nearest_slot != 0xFF)
            {
                ChairSlotAndUser::iterator itr = m_chairData.find(nearest_slot);
                if (itr != m_chairData.end())
                {
                    itr->second.user = p->GetGUID();
                    p->Teleport( itr->second.x, itr->second.y, itr->second.z, GetOrientation() );
                    p->SetStandState(STANDSTATE_SIT_LOW_CHAIR+goinfo->data.chair.height);
                    return;
                }
            }
        }break;
    case GAMEOBJECT_TYPE_CHEST://cast da spell
        {
            spellInfo = dbcSpell.LookupEntry( OPEN_CHEST );
            SpellCastTargets targets(GetGUID());
            if(Spell *spell = new Spell(p, spellInfo))
                spell->prepare(&targets, true);
        }break;
    case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            UseFishingNode(p);
        }break;
    case GAMEOBJECT_TYPE_DOOR:
        {
            // door
            if((GetState() == 1) && (GetFlags() == 33))
                EventCloseDoor();
            else
            {
                SetFlags(33);
                SetState(GO_STATE_ACTIVATED);
            }
        }break;
    case GAMEOBJECT_TYPE_FLAGSTAND:
        {
            // battleground/warsong gulch flag
            /*if(p->m_bg)
            {
                if( p->m_stealth )
                    p->RemoveAura( p->m_stealth );

                if( p->m_MountSpellId )
                    p->RemoveAura( p->m_MountSpellId );

                if(!p->m_bgFlagIneligible)
                    p->m_bg->HookFlagStand(p, this);
                TRIGGER_INSTANCE_EVENT( p->GetMapInstance(), OnPlayerFlagStand )( p, this );
            } else sLog.outError("Gameobject Type FlagStand activated while the player is not in a battleground, entry %u", goinfo->ID);*/
        }break;
    case GAMEOBJECT_TYPE_FLAGDROP:
        {
            // Dropped flag
            /*if(p->m_bg)
            {
                if( p->m_stealth )
                    p->RemoveAura( p->m_stealth );

                if( p->m_MountSpellId )
                    p->RemoveAura( p->m_MountSpellId );

                p->m_bg->HookFlagDrop(p, this);
                TRIGGER_INSTANCE_EVENT( p->GetMapInstance(), OnPlayerFlagDrop )( p, this );
            }
            else
                sLog.outError("Gameobject Type Flag Drop activated while the player is not in a battleground, entry %u", goinfo->ID);*/
        }break;
    case GAMEOBJECT_TYPE_QUESTGIVER:
        {
            // Questgiver
            if(HasQuests())
                sQuestMgr.OnActivateQuestGiver(this, p);
            else
                sLog.outError("Gameobject Type Questgiver doesn't have any quests entry %u (May be false positive if object has a script)", goinfo->ID);
        }break;
    case GAMEOBJECT_TYPE_SPELLCASTER:
        {
            SpellEntry *info = dbcSpell.LookupEntry(goinfo->GetSpellID());
            if(!info)
            {
                sLog.outError("Gameobject Type Spellcaster doesn't have a spell to cast entry %u", goinfo->ID);
                return;
            }

            SpellCastTargets targets(p->GetGUID());
            if(Spell* spell = new Spell(p, info))
                spell->prepare(&targets, false);
            if(charges > 0 && !--charges)
                Deactivate(0);
        }break;
    case GAMEOBJECT_TYPE_RITUAL:
        {
            // store the members in the ritual, cast sacrifice spell, and summon.
            uint32 i = 0, reqParticipants = goinfo->data.ritual.reqParticipants;
            if(!m_ritualmembers || !GetGOui32Value(GO_UINT32_RIT_SPELL) || !GetGOui32Value(GO_UINT32_M_RIT_CASTER))
                return;

            for(i = 0; i < reqParticipants; i++)
            {
                if(!m_ritualmembers[i])
                {
                    m_ritualmembers[i] = p->GetLowGUID();
                    p->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, GetGUID());
                    p->SetUInt32Value(UNIT_CHANNEL_SPELL, GetGOui32Value(GO_UINT32_RIT_SPELL));
                    break;
                }
                else if(m_ritualmembers[i] == p->GetLowGUID())
                {
                    // we're deselecting :(
                    m_ritualmembers[i] = 0;
                    p->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
                    p->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
                    return;
                }
            }

            if(i == reqParticipants - 1)
            {
                SetGOui32Value(GO_UINT32_RIT_SPELL, 0);
                Player* plr;
                for(i = 0; i < reqParticipants; i++)
                {
                    plr = p->GetMapInstance()->GetPlayer(m_ritualmembers[i]);
                    if(plr != NULL)
                    {
                        plr->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
                        plr->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
                    }
                }

                SpellEntry *info = NULL;
                switch( goinfo->ID )
                {
                case 36727:// summon portal
                    {
                        if(!GetGOui32Value(GO_UINT32_M_RIT_TARGET))
                            return;

                        if((info = dbcSpell.LookupEntry(goinfo->GetSpellID())) == NULL)
                            break;
                        Player* target = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_TARGET));
                        if(target == NULL)
                            return;

                        /*SpellCastTargets targets(target->GetGUID());
                        if(Spell *spell = new Spell(this, info))
                            spell->prepare(&targets, true);*/
                    }break;
                case 177193:// doom portal
                    {
                        // kill the sacrifice player
                        Player* psacrifice = p->GetMapInstance()->GetPlayer(m_ritualmembers[(int)(RandomUInt(reqParticipants-1))]);
                        Player* pCaster = GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!psacrifice || !pCaster)
                            return;
                        if((info = dbcSpell.LookupEntry(goinfo->data.ritual.casterTargetSpell)) == NULL)
                            break;

                        SpellCastTargets targets(psacrifice->GetGUID());
                        if(Spell *spell = new Spell(psacrifice, info))
                            spell->prepare(&targets, true);

                        // summons demon
                        targets.m_unitTarget = pCaster->GetGUID();
                        if(info = dbcSpell.LookupEntry(goinfo->data.ritual.spellId))
                            if(Spell *spell = new Spell(pCaster, info))
                                spell->prepare(&targets, true);
                    }break;
                case 179944:// Summoning portal for meeting stones
                    {
                        Player* plr = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_TARGET));
                        if(!plr)
                            return;

                        Player* pleader = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        SpellCastTargets targets(plr->GetGUID());
                        if(Spell* spell = new Spell(pleader, info))
                            spell->prepare(&targets, true);

                        /* expire the GameObject* */
                        Deactivate(0);
                    }break;
                case 194108:// Ritual of Summoning portal for warlocks
                    {
                        Player* pleader = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        SpellCastTargets targets(pleader->GetGUID());
                        if(Spell* spell = new Spell(pleader, info))
                            spell->prepare(&targets, true);

                        Deactivate(0);
                        pleader->GetSpellInterface()->CleanupCurrentSpell();
                    }break;
                case 186811://Ritual of Refreshment
                case 193062:
                    {
                        Player* pleader = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        SpellCastTargets targets(pleader->GetGUID());
                        if(Spell* spell = new Spell(pleader, info))
                            spell->prepare(&targets, true);

                        Deactivate(0);
                        pleader->GetSpellInterface()->CleanupCurrentSpell();
                    }break;
                case 181622://Ritual of Souls
                case 193168:
                    {
                        Player* pleader = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        SpellCastTargets targets(pleader->GetGUID());
                        if(Spell* spell = new Spell(pleader, info))
                            spell->prepare(&targets, true);
                    }break;
                }
            }
        }break;
    case GAMEOBJECT_TYPE_GOOBER:
        {
            if(SpellEntry * sp = dbcSpell.LookupEntry(goinfo->GetSpellID()))
                p->GetSpellInterface()->LaunchSpell(sp, p);

        }break;
    case GAMEOBJECT_TYPE_CAMERA://eye of azora
        {
            if(uint32 cinematic = goinfo->data.camera.cinematicId)
            {
                WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
                data << uint32(cinematic);
                p->PushPacket(&data);
            }
            else
                sLog.outError("Gameobject Type Camera doesn't have a cinematic to play id, entry %u", goinfo->ID);
        }break;
    case GAMEOBJECT_TYPE_MEETINGSTONE:  // Meeting Stone
        {
            /* Use selection */
            Player* pPlayer = objmgr.GetPlayer(p->GetSelection());
            if(!pPlayer || p->GetGroup() != pPlayer->GetGroup() || !p->GetGroup())
                return;

            GameObjectInfo * info = GameObjectNameStorage.LookupEntry(179944);
            if(!info)
                return;

            /* Create the summoning portal */
            GameObject* pGo = p->GetMapInstance()->CreateGameObject(179944);
            if( pGo == NULL )
                return;
            pGo->Load(p->GetMapId(), p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), p->GetOrientation());

            // dont allow to spam them
            GameObject* gobj = castPtr<GameObject>(p->GetMapInstance()->GetObjectClosestToCoords(179944, p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), 999999.0f, TYPEID_GAMEOBJECT));
            if( gobj )
                Deactivate(0);

            pGo->SetGOui32Value(GO_UINT32_M_RIT_CASTER, p->GetLowGUID());
            pGo->SetGOui32Value(GO_UINT32_M_RIT_TARGET, pPlayer->GetLowGUID());
            pGo->SetGOui32Value(GO_UINT32_RIT_SPELL, 61994);
            p->GetMapInstance()->AddObject(pGo);

            /* member one: the (w00t) caster */
            pGo->m_ritualmembers[0] = p->GetLowGUID();
            p->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, pGo->GetGUID());
            p->SetUInt32Value(UNIT_CHANNEL_SPELL, pGo->GetGOui32Value(GO_UINT32_RIT_SPELL));
        }break;
    case GAMEOBJECT_TYPE_BARBER_CHAIR:
        {
            p->SafeTeleport( p->GetMapId(), p->GetInstanceID(), GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation() );
            p->SetStandState(STANDSTATE_SIT_HIGH_CHAIR);
            p->Dismount();

            WorldPacket data(SMSG_ENABLE_BARBER_SHOP, 0);
            p->PushPacket(&data);
        }break;
    }
}
