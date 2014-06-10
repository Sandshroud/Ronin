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

#include "LacrimiStdAfx.h"
#include "WintergraspWarZones.h"

WintergraspScript::WintergraspScript(MapMgr* Thismgr) : MapManagerScript(Thismgr)
{
    mgr = Thismgr;
    m_started = false;
    ControllingTeam = HORDE;
    m_Initialized = false;

    m_NextTime = UNIXTIME+10800; // 3 hour
    uint32 value = 10800%10;
    m_clock[0] = value;
    m_clock[1] = value;
    FirstHourHerald = ThirtyMinuteHerald = FifteenMinuteHerald = false;
}

WintergraspScript::~WintergraspScript()
{
    mgr = NULL;
    m_NextTime = 0;
    mgr->UpdateAllCells(false, ZONE_WINTERGRASP);
    m_started = false;
    ControllingTeam = HORDE;
    FirstHourHerald = ThirtyMinuteHerald = FifteenMinuteHerald = false;
}

void WintergraspScript::OnUpdate(uint32 p_time)
{
    if(!m_Initialized)
    {
        m_Initialized = true;
        sLog.Notice("Wintergrasp", "Server is loading wintergrasp tiles, it might take some time..");
        mgr->UpdateAllCells(true, ZONE_WINTERGRASP);
        sLog.Notice("Wintergrasp", "Finished loading tiles...");
    }

    UpdateClockDigit(m_NextTime-UNIXTIME, m_started ? 0 : 1, 10);
    if(UNIXTIME >= m_NextTime)
    {
        if(m_started)
        {
            if(m_FinishTime > p_time)
            {
                m_FinishTime -= p_time;
                UpdateWarZones();
            }
            else
            {
                EndBattle();
                Herald((char*)format("The battle for Wintergrasp has ended! The Winner is, the %s!", ControllingTeam ? "Horde" : "Alliance").c_str());
            }
        }
        else StartBattle();
    }
    else
    {
        float timeInHours = float((m_NextTime-UNIXTIME)/60);
        if(timeInHours < 0.7)
        {
            float timeInMinutes = floor((timeInHours*60));
            if(timeInMinutes == 30 && !ThirtyMinuteHerald)
            {
                ThirtyMinuteHerald = true;
                Herald("The battle for Wintergrasp begins in 30 minutes!");
            }
            else if(timeInMinutes == 15 && !FifteenMinuteHerald)
            {
                FifteenMinuteHerald = true;
                Herald("The battle for Wintergrasp begins in 15 minutes!");
            }
        }
        else
        {
            if(timeInHours == 1 && !FirstHourHerald)
            {
                FirstHourHerald = true;
                Herald("The battle for Wintergrasp begins in 1 hour!");
            }
        }
    }
}

void WintergraspScript::EndBattle()
{
    m_started = false;
    SpawnWarZones(false);
    DelayBattle();
}

void WintergraspScript::StartBattle()
{
    m_started = true;
    m_FinishTime = 60000;

    SpawnWarZones(true);
    SendInitWorldStates();
}

void WintergraspScript::DelayBattle()
{
    m_NextTime = UNIXTIME+10800;
}

void WintergraspScript::UpdateWarZones()
{
    uint32 CurrentTime = UNIXTIME;
    for(std::set<SpawnInfo*>::iterator itr = m_WarriorPool.begin(), itr2; itr != m_WarriorPool.end();)
    {
        itr2 = itr++;

        Creature* ctr = (*itr2)->m_Spawn;
        if(ctr == NULL)
            m_WarriorPool.erase(itr2);
        else if(ctr->isDead())
        {
            (*itr2)->RespawnTime = CurrentTime+120;
            (*itr2)->m_Spawn = NULL;
            m_WarriorPool.erase(itr2);
            m_WarriorSoulPool.insert((*itr2));

            ctr->RemoveFromWorld(false, true);
            ctr->Destruct();
        }
    }

    CreatureProto* ctrp = NULL;
    SpawnInfo* spawnInfo = NULL;
    for(std::set<SpawnInfo*>::iterator itr = m_WarriorSoulPool.begin(), itr2; itr != m_WarriorSoulPool.end();)
    {
        itr2 = itr++;

        spawnInfo = (*itr2);
        if(spawnInfo->RespawnTime < CurrentTime)
        {
            ctrp = CreatureProtoStorage.LookupEntry((*itr2)->CreatureEntry);
            if(ctrp == NULL)
                continue;

            float x = Wintergrasp_WarZones[spawnInfo->locationId][spawnInfo->SpawnId].mX, y = Wintergrasp_WarZones[spawnInfo->locationId][spawnInfo->SpawnId].mY,
                z = Wintergrasp_WarZones[spawnInfo->locationId][spawnInfo->SpawnId].mZ, o = Wintergrasp_WarZones[spawnInfo->locationId][spawnInfo->SpawnId].mO;

            if(x == 0.0f || y == 0.0f || z == 0.0f)
                continue;

            spawnInfo->m_Spawn = mgr->CreateCreature(spawnInfo->CreatureEntry);
            if(spawnInfo->m_Spawn == NULL)
                continue;

            spawnInfo->m_Spawn->SetMapId(mgr->GetMapId());
            spawnInfo->m_Spawn->SetInstanceID(mgr->GetInstanceID());

            if(spawnInfo->m_Spawn->Load(ctrp, mgr->iInstanceMode, x, y, z, o))
            {
                if(!spawnInfo->m_Spawn->CanAddToWorld())
                {
                    spawnInfo->m_Spawn->Destruct();
                    spawnInfo->m_Spawn = NULL;
                    continue;
                }

                m_WarriorSoulPool.erase(itr2);
                spawnInfo->m_Spawn->PushToWorld(mgr);
                m_WarriorPool.insert(spawnInfo);
            }
            else
            {
                spawnInfo->m_Spawn->Destruct();
                spawnInfo->m_Spawn = NULL;
            }
        }
    }
}

void WintergraspScript::SpawnWarZones(bool apply)
{
    return; // Needs work and more base implementation before it's safe to enable it.

    // This one is simple.
    if(apply)
    {
        Herald("Multiple Battle groups have engaged!");

        uint32 Entry = 0, percent = 100;
        CreatureProto* ctrp = NULL;
        for(uint32 i = 0; i < MAX_BATTLE_ZONES; i++)
        {
            for(uint32 t = 0; t < MAX_UNITS_PER_ZONE/2; t++)
            {
                float x = Wintergrasp_WarZones[i][t].mX, y = Wintergrasp_WarZones[i][t].mY, z = Wintergrasp_WarZones[i][t].mZ, o = Wintergrasp_WarZones[i][t].mO;
                if(x == 0.0f || y == 0.0f || z == 0.0f)
                    continue;

                if(RandomUInt(percent) > (percent/2))
                    Entry = 00000;
                else
                    Entry = 00000;
                percent += 1;

                ctrp = CreatureProtoStorage.LookupEntry(Entry);
                if(ctrp == NULL)
                    continue;

                Creature* ctr = mgr->CreateCreature(Entry);
                if(ctr == NULL)
                    continue;

                ctr->SetMapId(mgr->GetMapId());
                ctr->SetInstanceID(mgr->GetInstanceID());

                if(ctr->Load(ctrp, mgr->iInstanceMode, x, y, z, o))
                {
                    if(!ctr->CanAddToWorld())
                    {
                        ctr->Destruct();
                        continue;
                    }

                    ctr->PushToWorld(mgr);

                    SpawnInfo* spI = new SpawnInfo();
                    spI->CreatureEntry = Entry;
                    spI->RespawnTime = 0;
                    spI->locationId = i;
                    spI->SpawnId = t;
                    spI->m_Spawn = ctr;
                    m_WarriorPool.insert(spI);
                }
                else ctr->Destruct();
            }

            for(uint32 t = MAX_UNITS_PER_ZONE/2; t < MAX_UNITS_PER_ZONE; t++)
            {
                float x = Wintergrasp_WarZones[i][t].mX, y = Wintergrasp_WarZones[i][t].mY, z = Wintergrasp_WarZones[i][t].mZ, o = Wintergrasp_WarZones[i][t].mO;
                if(x == 0.0f || y == 0.0f || z == 0.0f)
                    continue;

                if(RandomUInt(percent) > (percent/2))
                    Entry = 00000;
                else
                    Entry = 00000;
                percent += 1;

                ctrp = CreatureProtoStorage.LookupEntry(Entry);
                if(ctrp == NULL)
                    continue;

                Creature* ctr = mgr->CreateCreature(Entry);
                if(ctr == NULL)
                    continue;

                ctr->SetMapId(mgr->GetMapId());
                ctr->SetInstanceID(mgr->GetInstanceID());

                if(ctr->Load(ctrp, mgr->iInstanceMode, x, y, z, o))
                {
                    if(!ctr->CanAddToWorld())
                    {
                        ctr->Destruct();
                        continue;
                    }

                    ctr->PushToWorld(mgr);

                    SpawnInfo* spI = new SpawnInfo();
                    spI->CreatureEntry = Entry;
                    spI->RespawnTime = 0;
                    spI->locationId = i;
                    spI->SpawnId = t;
                    spI->m_Spawn = ctr;
                    m_WarriorPool.insert(spI);
                }
                else ctr->Destruct();
            }
        }
    }
    else
    {
        m_WarriorSoulPool.clear();
        float x = 0.0f, y = 0.0f;
        for(std::set<SpawnInfo*>::iterator itr = m_WarriorPool.begin(), it2; itr != m_WarriorPool.end();)
        {
            it2 = itr++;
            Creature* ctr = (*it2)->m_Spawn;
            if(ctr != NULL)
            {
                x = ctr->GetPositionX();
                y = ctr->GetPositionY();

                (*it2)->m_Spawn = NULL;
                ctr->RemoveFromWorld(false, true);
                ctr->Destruct();
            }

            delete (*it2);
        }
        m_WarriorPool.clear();
    }
}

void WintergraspScript::Herald(char* message)
{
    _playerslock.Acquire();
    if(!_players.size())
    {
        _playerslock.Release();
        return;
    }

    WorldPacket data(SMSG_MESSAGECHAT, 500);
    data << uint8(CHAT_MSG_MONSTER_YELL);
    data << uint32(LANG_UNIVERSAL);
    data << uint64(0);
    data << uint32(0);          // new in 2.1.0
    data << uint32(7);          // Herald
    data << "Herald";           // Herald
    data << uint64(0);
    data << uint32(strlen(message)+1);
    data << message;
    data << uint8(0x00);
    for(PlayerStorageMap::iterator itr = _players.begin(); itr != _players.end(); itr++)
        itr->second->GetSession()->SendPacket(new WorldPacket(data));
    _playerslock.Release();
}

void WintergraspScript::UpdateClockDigit(uint32 timer, uint32 digit, uint32 mod)
{
    uint32 value = timer%mod;
    if (m_clock[digit] != value)
    {
        m_clock[digit] = value;
        UpdateWorldState(ClockWorldState[digit], m_clock[digit]);
    }
}

void WintergraspScript::SendInitWorldStates(Player* plr)
{
    WorldPacket data(SMSG_INIT_WORLD_STATES, (4+4+4+2));
    // Mapid
    data << uint32(571);
    // Zone
    data << uint32(ZONE_WINTERGRASP);
    // Area
    data << uint32(0);
    // Count
    data << uint16(6+(m_started ? 4 : 0));
    data << uint32(3803) << uint32(ControllingTeam == ALLIANCE ? 1 : 0);
    data << uint32(3802) << uint32(ControllingTeam == HORDE ? 1 : 0);
    data << uint32(3801) << uint32(m_started ? 0 : 1);
    data << uint32(3710) << uint32(m_started ? 1 : 0);
    for(uint8 i = 0; i < 2; i++)
        data << uint32(ClockWorldState[i]) << uint32(m_clock[i]);

    if(m_started)
    {
        data << uint32(A_NUMVEH_WORLDSTATE) << uint32(0);
        data << uint32(A_MAXVEH_WORLDSTATE) << uint32(4);
        data << uint32(H_NUMVEH_WORLDSTATE) << uint32(0);
        data << uint32(H_MAXVEH_WORLDSTATE) << uint32(4);
    }

    if(plr != NULL)
        plr->GetSession()->SendPacket(&data);
    else
    {
        _playerslock.Acquire();
        if(_players.size())
        {
            for(PlayerStorageMap::iterator itr = _players.begin(); itr != _players.end(); itr++)
                itr->second->GetSession()->SendPacket(new WorldPacket(data));
        }
        _playerslock.Release();
    }
}

void WintergraspScript::UpdateWorldState(uint32 WorldState, uint32 Value)
{
    _playerslock.Acquire();
    if(_players.size())
    {
        _playerslock.Release();
        return;
    }

    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 8);
    data << WorldState;
    data << Value;
    for(PlayerStorageMap::iterator itr = _players.begin(); itr != _players.end(); itr++)
        itr->second->GetSession()->SendPacket(new WorldPacket(data));
    _playerslock.Release();
}

void WintergraspScript::OnChangeArea(Player* pPlayer, uint32 ZoneID, uint32 NewAreaID, uint32 OldAreaID)
{
    bool result = false;
    AreaTableEntry *area = dbcAreaTable.LookupEntry(NewAreaID);
    if(NewAreaID == ZONE_WINTERGRASP || (area != NULL && area->ZoneId == ZONE_WINTERGRASP))
    {
        _playerslock.Acquire();
        if(_players.find(pPlayer->GetLowGUID()) == _players.end())
        {
            result = true;
            _players[pPlayer->GetLowGUID()] = pPlayer;
        }
        _playerslock.Release();

        if(result)
            SendInitWorldStates(pPlayer);
    }
    else
    {
        _playerslock.Acquire();
        if(_players.find(pPlayer->GetLowGUID()) != _players.end())
        {
            _players.erase(pPlayer->GetLowGUID());
            result = true;
        }
        _playerslock.Release();

        if(result)
        {
            WorldPacket data(SMSG_INIT_WORLD_STATES, 14); // Clear our Wintergrasp states.
            data << uint32(571) << uint32(ZONE_WINTERGRASP) << uint32(ZONE_WINTERGRASP) << uint16(0);
            pPlayer->GetSession()->SendPacket(&data);
        }
    }
}

void Lacrimi::SetupWintergrasp()
{
    if(sWorld.wg_enabled)
        RegisterMapManagerScript(571, WintergraspScript);
}
