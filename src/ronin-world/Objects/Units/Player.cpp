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

static const uint32 DKNodesMask[12] = {0xFFFFFFFF,0xF3FFFFFF,0x317EFFFF,0,0x2004000,0x1400E0,0xC1C02014,0x12018,0x380,0x4000C10,0,0};//all old continents are available to DK's by default.

#define MAX_CELL_UPDATES 4

void PlayerCellManager::Update(MapInstance *instance, uint32 msTime, uint32 uiDiff)
{
    // Process calldown
    ObjectCellManager::Update(instance, msTime, uiDiff);

    // Handle any delayed cell updates
    uint32 count = 0;
    while(!_delayedCells[0].empty())
    {
        std::pair<uint16, uint16> pair = unPack(*_delayedCells[0].begin());
        _delayedCells[0].erase(_delayedCells[0].begin());

        _processedCells.insert(_makeCell(pair.first, pair.second));
        instance->UpdateCellData(_object, pair.first, pair.second, _object->IsPlayer(), true);
        if(++count >= MAX_CELL_UPDATES)
            return;
    }

    while(!_delayedCells[1].empty())
    {
        std::pair<uint16, uint16> pair = unPack(*_delayedCells[1].begin());
        _delayedCells[1].erase(_delayedCells[1].begin());

        _processedCells.insert(_makeCell(pair.first, pair.second));
        instance->UpdateCellData(_object, pair.first, pair.second, _object->IsPlayer(), false);
        if(++count >= MAX_CELL_UPDATES)
            return;
    }
}

void PlayerCellManager::SetCurrentCell(MapInstance *instance, float newX, float newY, float newZ, uint8 cellRange)
{
    // Current cell set
    _visRange = cellRange;
    _luX = newX;
    _luY = newY;
    _luZ = newZ;
    _currX = _getCellId(_luX);
    _currY = _getCellId(_luY);

    // Remove any pending cell handling
    _delayedCells[0].clear();
    _delayedCells[1].clear();

    std::set<uint32> preProcessed;
    // Push old processed data to preProcessed group
    if(_lowX != _highX && _lowY != _highY)
    {
        for(uint16 x = _lowX; x <= _highX; x++)
            for(uint16 y = _lowY; y <= _highY; y++)
                preProcessed.insert(_makeCell(x, y));
    }
    // Clear old processed cells
    _processedCells.clear();
    // Push current cell to proccessed, we'll handle it in this function
    _processedCells.insert(_makeCell(_currX, _currY));

    if(cellRange)
    {   // Fill priority cells from a range of 1
        _lowX = _currX >= 1 ? _currX-1 : 0;
        _lowY = _currY >= 1 ? _currY-1 : 0;
        _highX = std::min<uint16>(_currX+1, _sizeX-1);
        _highY = std::min<uint16>(_currY+1, _sizeY-1);
        for(uint16 x = _lowX; x <= _highX; x++)
        {
            for(uint16 y = _lowY; y <= _highY; y++)
            {
                uint32 cellId = _makeCell(x, y);

                // Check to see if we're a preprocessed cell
                if(preProcessed.find(cellId) != preProcessed.end())
                {
                    _processedCells.insert(cellId);
                    preProcessed.erase(cellId);
                    continue;
                }

                // Skip processed cells(current usually)
                if(_processedCells.find(cellId) != _processedCells.end())
                    continue;

                // Add as a priority delayed cell
                _delayedCells[0].insert(cellId);
            }
        }

        // Process non priority cell bounding
        if(cellRange > 1)
        {   // Calculate our new bounds based on range
            _lowX = _currX >= cellRange ? _currX-cellRange : 0;
            _lowY = _currY >= cellRange ? _currY-cellRange : 0;
            _highX = std::min<uint16>(_currX+cellRange, _sizeX-1);
            _highY = std::min<uint16>(_currY+cellRange, _sizeY-1);

            // Only add extra cells if we're a player
            if(_object->IsPlayer())
            {
                for(uint16 x = _lowX; x <= _highX; x++)
                {
                    for(uint16 y = _lowY; y <= _highY; y++)
                    {
                        // View distance is rendered as a square that we fill data in from cells
                        // If we've enabled corner cutting we can ignore the farthest points of the square
                        // To improve functionality of the client and not affect how our view distance is
                        // Actually perceived by the client, only really affects visibility since creatures
                        // Can use cell walkers outside of their range set if needed
                        if(cutCorners && isCorner(x, y, _lowX, _highX, _lowY, _highY, VisibleCellRange))
                            continue;

                        uint32 cellId = _makeCell(x, y);
                        // Check to see if we're a preprocessed cell
                        if(preProcessed.find(cellId) != preProcessed.end())
                        {
                            _processedCells.insert(cellId);
                            preProcessed.erase(cellId);
                            continue;
                        }

                        // Skip processed cells(current usually)
                        if(_processedCells.find(cellId) != _processedCells.end())
                            continue;

                        // Add as a low priority delayed cell
                        _delayedCells[1].insert(cellId);
                    }
                }
            }
        }
    }

    // If our instance is null, then it's part of preloading
    if(instance == NULL)
    {   // Set the data as processed
        _processedCells.insert(_delayedCells[0].begin(), _delayedCells[0].end());
        _processedCells.insert(_delayedCells[1].begin(), _delayedCells[1].end());
        // empty the tables
        _delayedCells[0].clear();
        _delayedCells[1].clear();
        return;
    }

    // Update for our current cell here, other cell updates will occur in WorldObject::Update
    instance->UpdateCellData(_object, _currX, _currY, _object->IsPlayer(), true);
    // Push calls to remove cell data
    instance->RemoveCellData(_object, preProcessed, false);
}

Player::Player(PlayerInfo *pInfo, WorldSession *session, uint32 fieldCount) : Unit(), m_session(session), m_playerInfo(pInfo), m_factionInterface(this), m_talentInterface(this), m_inventory(this), m_currency(this), m_mailBox(new Mailbox(pInfo->charGuid))
{
    // Call object construct first
    Unit::Construct(pInfo->charGuid, fieldCount);
    // Init

    SetTypeFlags(TYPEMASK_TYPE_PLAYER);
    m_objType = TYPEID_PLAYER;

    // Set name to match player info
    m_name = pInfo->charName.c_str();
    // Set up race data
    setRace(pInfo->charRace);
    myRace = dbcCharRace.LookupEntry(pInfo->charRace);
    // Set up class data
    setClass(pInfo->charClass);
    myClass = dbcCharClass.LookupEntry(pInfo->charClass);
    // Set up gender data
    setGender(m_playerInfo->charAppearance3&0xFF);
    // Set our creation information pointer
    m_createInfo = objmgr.GetPlayerCreateInfo(pInfo->charRace, pInfo->charClass);
    // Set player appearance bytes here instead of initialize for gender data
    SetUInt32Value(PLAYER_BYTES, m_playerInfo->charAppearance);
    SetUInt32Value(PLAYER_BYTES_2, m_playerInfo->charAppearance2);
    SetUInt32Value(PLAYER_BYTES_3, m_playerInfo->charAppearance3);
    // Set our base stats based on race, class, and level
    if((baseStats = sStatSystem.GetUnitBaseStats(getRace(), getClass(), pInfo->lastLevel)) == NULL)
        baseStats = sStatSystem.GetMaxUnitBaseStats(getRace(), getClass());
    // Set our death state from hased data
    m_deathState = DeathState(m_playerInfo->lastDeathState);

    // Main data initialized, now set defaults
    m_massSummonEnabled = false;
    m_taxiMask.SetCount(8*114);

    m_questLog.resize(QUEST_LOG_COUNT, NULL);

    m_bgData = NULL;
    m_runeData = NULL;
    m_taxiData = NULL;
    m_gmData = NULL;
    memset(&m_bindData, 0, sizeof(Player::BindData));

    m_lastSwingError = 0;
    m_oldZone                       = 0;
    m_oldArea                       = 0;
    m_feralAP                       = 0;
    m_finishingmovesdodge           = false;
    resurrector                     = 0;
    SpellCrtiticalStrikeRatingBonus = 0;
    SpellHasteRatingBonus           = 0;
    bReincarnation                  = false;
    TrackingSpell                   = 0;
    m_status                        = 0;
    m_ShapeShifted                  = 0;
    m_curSelection                  = 0;
    m_lootGuid                      = 0;
    m_resurrectHealth               = 0;
    m_resurrectMana                 = 0;
    m_GroupInviter                  = 0;
    OnlineTime                      = 0;
    m_invitersGuid                  = 0;
    m_invitersGuid                  = 0;
    m_duelStorage                   = NULL;
    m_lootGuid                      = 0;
    m_banned                        = 0;
    //Bind possition
    m_timeLogoff                    = 0;
    m_restData.isResting            = false;
    m_restData.restState            = 0;
    m_restData.restAmount           = 0;
    m_afk_reason                    = "";
    m_weaponProficiency             = 0;
    m_armorProficiency              = 0;
    m_targetIcon                    = 0;
    bHasBindDialogOpen              = false;
    m_CurrentCharm                  = NULL;
    m_CurrentTransporter            = NULL;
    m_SummonedObject                = NULL;
    m_currentLoot                   = (uint64)NULL;
    roll                            = 0;
    m_updateDataCount               = 0;
    m_OutOfRangeIdCount             = 0;
    m_updateDataBuff.reserve(0xAFFF);
    m_OutOfRangeIds.reserve(0x1000);
    m_itemUpdateData.reserve(0x1000);
    bProcessPending                 = false;

    myCorpse                        = NULL;
    blinked                         = false;
    blinktimer                      = getMSTime();
    linkTarget                      = NULL;
    m_pvpTimer                      = 300000;
    m_globalCooldown                = 0;
    m_lastHonorResetTime            = 0;
    m_TeleportState                 = 1;
    m_beingPushed                   = false;
    iInstanceType                   = 0;
    iRaidType                       = 0;
    m_XPoff                         = false;

    ok_to_remove                    = false;
    trigger_on_stun                 = 0;
    trigger_on_stun_chance          = 100;
    m_rap_mod_pct                   = 0;
    m_summoner                      = NULL;
    m_summonInstanceId              = m_summonMapId = 0;
    m_tempSummon                    = NULL;
    m_deathVision                   = false;
    m_arenateaminviteguid           = 0;
    raidgrouponlysent               = false;
    m_setwaterwalk                  = false;
    m_areaSpiritHealer_guid         = 0;
    m_KickDelay                     = 0;
    m_hardKick                      = false;
    m_passOnLoot                    = false;
    m_changingMaps                  = true;
    m_mageInvisibility              = false;

    watchedchannel                  = NULL;
    PreventRes                      = false;
    m_drunkTimer                    = 0;
    m_drunk                         = 0;
    m_hasSentMoTD = false;
    m_cooldownCheat = false;
    CastTimeCheat = false;
    PowerCheat = false;

    m_completedQuests.clear();
    m_completedDailyQuests.clear();
    quest_spells.clear();
    quest_mobs.clear();
    m_channels.clear();
    m_channelsbyDBCID.clear();
    m_visibleObjects.clear();
    m_spells.clear();
}

Player::~Player ( )
{

}

bool Player::Initialize()
{
    if(m_playerInfo == NULL || m_createInfo == NULL || myRace == NULL || myClass == NULL)
        return false;

    Unit::Init();

    // Initialize values based off of creation data
    // Faction template
    SetFaction( m_createInfo->factiontemplate );

    // Set our display id
    SetUInt32Value(UNIT_FIELD_DISPLAYID, m_createInfo->displayId[getGender()]);
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, m_createInfo->displayId[getGender()]);
    // Values based off creation data initialized

    // Initialize values based off of player information
    SetTeam(m_playerInfo->charTeam);

    // Set our position
    m_mapId         = m_playerInfo->lastMapID;
    m_instanceId    = m_playerInfo->lastInstanceID;
    m_position.x    = m_playerInfo->lastPositionX;
    m_position.y    = m_playerInfo->lastPositionY;
    m_position.z    = m_playerInfo->lastPositionZ;
    m_position.o    = NormAngle(m_playerInfo->lastOrientation);
    m_zoneId        = m_playerInfo->lastZone;

    if(GuildInfo *gInfo = guildmgr.GetGuildInfo(m_playerInfo->GuildId))
    {
        SetGuildId(m_playerInfo->GuildId);
        SetGuildRank(m_playerInfo->GuildRank);
        SetGuildLevel(gInfo->m_guildLevel);
        SetUInt32Value(PLAYER_GUILD_TIMESTAMP, UNIXTIME);
    }

    SetUInt32Value(PLAYER_FIELD_MAX_LEVEL, m_session->m_maxLevel);
    SetUInt32Value(UNIT_FIELD_LEVEL, m_playerInfo->lastLevel);
    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, sStatSystem.GetXPToNextLevel(m_playerInfo->lastLevel));

    // Values based off of player information initialized

    SetFloatValue(PLAYER_FIELD_MOD_HASTE, 1.f);
    SetFloatValue(PLAYER_FIELD_MOD_RANGED_HASTE, 1.f);
    SetFloatValue(PLAYER_FIELD_MOD_PET_HASTE, 1.f);
    SetFloatValue(PLAYER_FIELD_MOD_HASTE_REGEN, 1.f);

    // We're players!
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_STATUS);

    // Default fields
    for(uint32 x = 0; x < MAX_RESISTANCE; x++)
        SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+x, 1.f);
    SetFloatValue(PLAYER_FIELD_MOD_HEALING_PCT, 1.f);
    SetFloatValue(PLAYER_FIELD_MOD_HEALING_DONE_PCT, 1.f);
    SetFloatValue(PLAYER_FIELD_MOD_SPELL_POWER_PCT, 1.f);
    SetFloatValue(PLAYER_FIELD_WEAPON_DMG_MULTIPLIERS, 1.f);
    SetUInt32Value(PLAYER_CHARACTER_POINTS, 2);

    // set power type
    SetPowerType(myClass->powerType);
    if(getClass() == DEATHKNIGHT && m_runeData == NULL)
    {
        m_runeData = new Player::RuneData();
        m_runeData->runemask = 0x3F;
        m_runeData->cooldownTimer = 10000;
        for(uint8 i = 0; i < 6; i++)
        {
            m_runeData->runes[i] = baseRunes[i];
            m_runeData->runeCD[i] = 0;
        }
    }

    std::vector<uint8> *classPower = sStatSystem.GetUnitPowersForClass(getClass());
    m_regenPowerFraction.resize(classPower->size(), NULL);
    m_regenTimerCounters.resize(classPower->size(), NULL);

    InitTaxiNodes();

    m_talentInterface.InitGlyphSlots();
    m_talentInterface.InitGlyphsForLevel(getLevel());

    AchieveMgr.AllocatePlayerData(GetGUID());

    // Add our corpse flag for dead players
    if(m_deathState == DEAD)
        addStateFlag(UF_CORPSE);

    // Add event handler for saving to database
    m_eventHandler.AddStaticEvent(this, &Player::SaveToDB, false, 120000);
    // Update group's out of range players every 10 seconds
    m_eventHandler.AddStaticEvent(this, &Player::EventGroupFullUpdate, 10000);

    // Construct storage pointers
    if(m_session->HasGMPermissions())
        m_gmData = new Player::PlayerGMData();
    return true;
}

void Player::Destruct()
{
    objmgr.RemovePlayer(this);
    AchieveMgr.CleanupPlayerData(GetGUID());

    if(m_session)
        m_session->SetPlayer(NULL);

    Player* pTarget;
    if(pTarget = objmgr.GetPlayer(GetInviter()))
        pTarget->SetInviter(0);

    m_selectedGo.Clean();

    if(m_SummonedObject)
    {
        m_SummonedObject->Cleanup();
        m_SummonedObject = NULL;
    }

    if( m_mailBox )
        delete m_mailBox;

    ASSERT(!IsInWorld());

    // delete m_talenttree

    CleanupChannels();

    for(int i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if(m_questLog[i] != NULL)
        {
            delete m_questLog[i];
            m_questLog[i] = NULL;
        }
    }
    m_questLog.clear();

    m_playerInfo->m_loggedInPlayer=NULL;

    //  SetSession(NULL);
    if(myCorpse)
        myCorpse = NULL;

    if(linkTarget)
        linkTarget = NULL;

    m_completedQuests.clear();
    quest_spells.clear();
    quest_mobs.clear();

    while(!m_loadAuras.empty())
    {
        Aura *aur = m_loadAuras.front().second;
        m_loadAuras.pop_front();
        delete aur;
    }

    m_channels.clear();
    m_channelsbyDBCID.clear();
    m_spells.clear();
    Unit::Destruct();
}

void Player::Update(uint32 msTime, uint32 diff)
{
    Unit::Update( msTime, diff );

    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TIMER))
        UpdatePvPState(msTime, diff);

    if (m_drunk && ((m_drunkTimer += diff) > 10*1000))
        EventHandleSobering();

    if(hasStateFlag(UF_ATTACKING))
        UpdateAutoAttackState();

    if(HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI) && m_taxiData)
    {
        uint32 taxiDiff = diff;
        if(m_taxiData->LastUpdateTime)
        {   // Update from the last time we set our taxi time data
            taxiDiff = getMSTimeDiff(getMSTime(), m_taxiData->LastUpdateTime);
            m_taxiData->LastUpdateTime = 0;
        }

        // Update out travel timer
        m_taxiData->TravelTime += taxiDiff;
        if((m_taxiData->UpdateTimer+=diff) >= 500 || m_taxiData->TravelTime >= m_taxiData->ArrivalTime)
        {   // Clear update timer before interpolate, as it can invalidate our pointer
            m_taxiData->UpdateTimer = 0;
            EventTaxiInterpolate();
        }
    }

    if(m_runeData && m_runeData->runemask != 0x3F)
    {
        for(uint8 i = 0; i < 6; ++i)
        {
            if(m_runeData->runeCD[i] > diff)
                m_runeData->runeCD[i] -= diff;
            else
            {
                m_runeData->runeCD[i] = 0;
                m_runeData->runemask |= (1<<i);
            }
        }
    }

    ProcessPendingItemUpdates();

    if(m_KickDelay)
    {
        if(m_KickDelay <= diff)
        {
            m_KickDelay = 0;
            _Kick();
        } else m_KickDelay -= diff;
    }
}

void Player::EventExploration(MapInstance *instance)
{
    if(m_position.x >= _maxX || m_position.x <= _minX || m_position.y >= _maxY || m_position.y <= _minY)
        return;

    uint32 oldZone = m_zoneId, oldArea = m_areaId;
    UpdateAreaInfo(instance);

    // Call virtual calldown after updating area info
    Unit::EventExploration(instance);

    bool restmap = false;
    World::RestedAreaInfo* restinfo = sWorld.GetRestedMapInfo(GetMapId());
    if(restinfo != NULL && (restinfo->ReqTeam == -1 || restinfo->ReqTeam == GetTeam()))
        restmap = true;

    if(m_zoneId == 0xFFFF)
    {
        // Clear our worldstates when we have no data.
        if(oldZone == 0xFFFF)
            return;

        instance->GetStateManager().ClearWorldStates(this);
        // This must be called every update, to keep data fresh.
        EventDBCChatUpdate(instance);
    }
    else if( oldZone != m_zoneId )
    {
        sWeatherMgr.SendWeather(this);

        m_AuraInterface.RemoveAllAurasByInterruptFlag( AURA_INTERRUPT_ON_LEAVE_AREA );

        m_playerInfo->lastZone = m_zoneId;

        TRIGGER_INSTANCE_EVENT( instance, OnZoneChange )( this, m_zoneId, oldZone );

        EventDBCChatUpdate(instance);

        instance->GetStateManager().SendWorldStates(this);
    }

    if(HasAreaFlag(OBJECT_AREA_FLAG_INDOORS))
    {
        uint32 mountAuraId = 0; //Mount expired?
        if(m_AuraInterface.GetMountedAura(mountAuraId) && !(m_mapId == 531 && (mountAuraId == 25953 || mountAuraId == 26054 || mountAuraId == 26055 || mountAuraId == 26056)))
            Dismount();

        // Now remove all auras that are only usable outdoors (e.g. Travel form)
        m_AuraInterface.RemoveAllAurasWithAttributes(0, ATTRIBUTES_ONLY_OUTDOORS);
    }

    if(!m_areaId || m_areaId == 0xFFFF)
    {
        m_AuraInterface.RemoveFlightAuras(); // remove flying buff
        HandleRestedCalculations(restmap);
        return;
    }

    AreaTableEntry* at = dbcAreaTable.LookupEntry(m_areaId);
    if(at == NULL)
        at = dbcAreaTable.LookupEntry(m_zoneId); // These maps need their own chat channels.
    if(at == NULL)
    {
        m_AuraInterface.RemoveFlightAuras(); // remove flying buff
        HandleRestedCalculations(restmap);
        return;
    }

    if(!(at->AreaFlags & AREA_FLYING_PERMITTED))
        m_AuraInterface.RemoveFlightAuras(); // remove flying buff

    if(HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY))
    {
        Unit* pUnit = GetSelection() ? instance->GetUnit(GetSelection()) : NULL;
        if(pUnit && !sFactionSystem.isAttackable(this, pUnit))
        {
            EventAttackStop();
            smsg_AttackStop(pUnit);
        }

        // Set rested to true for sanctuaries
        restmap = true;
    }
    else
    {   // PvP flag updating
        if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE) || HasAreaFlag(OBJECT_AREA_FLAG_PVP_AREA))
            SetPvPFlag(); // PvP is always active or we're in pvp flag area
        else if(HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP) && !HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TIMER))
            RemovePvPFlag();
    }

    TRIGGER_INSTANCE_EVENT( instance, OnChangeArea )( this, m_zoneId, m_areaId, oldArea );

    // bur: we dont want to explore new areas when on taxi
    if(!GetTaxiState() && !GetTransportGuid() && m_session)
    {
        uint32 offset = at->explorationFlag / 32;
        if(offset < 156)
        {
            offset += PLAYER_EXPLORED_ZONES_1;

            uint32 val = (uint32)(1 << (at->explorationFlag % 32));
            uint32 currFields = GetUInt32Value(offset);
            if(!(currFields & val))//Unexplored Area
            {
                SetUInt32Value(offset, (uint32)(currFields | val));

                uint32 explore_xp = at->level * 10 * sWorld.getRate(RATE_XP);
                WorldPacket data(SMSG_EXPLORATION_EXPERIENCE, 8);
                data << at->AreaId << explore_xp;
                PushPacket(&data);
                GiveXP(explore_xp, 0, false, false);
            }
        }

        sQuestMgr.OnPlayerExploreArea(this, at->AreaId);
    }

    // Check for a restable area
    bool rest_on = restmap;
    if(rest_on == false && ((at->ZoneId && (restinfo = sWorld.GetRestedAreaInfo(at->ZoneId))) || (restinfo = sWorld.GetRestedAreaInfo(at->AreaId))))
    {
        if(restinfo->ReqTeam == -1 || restinfo->ReqTeam == GetTeam())
            rest_on = true;
        else rest_on = false;
    }

    HandleRestedCalculations(rest_on);
}

void Player::PushData(uint16 opcode, uint16 len, const void *data, bool direct)
{
    if(m_session == NULL)
        return;

    if(!(m_packetQueue.empty() && IsInWorld()) && direct == false)
    {
        WorldPacket *packet = new WorldPacket();
        if(len && data) packet->append((uint8*)data, len);
        m_packetQueue.add(packet);

        // First packet push us into process set
        if(IsInWorld() && m_packetQueue.size() == 1)
            m_mapInstance->PushToProcessed(this);
    } else m_session->OutPacket(opcode, len, data);
}

void Player::PushPacket(WorldPacket *data, bool direct)
{
    if(m_session == NULL)
        return;

    if(!(m_packetQueue.empty() && IsInWorld()) && direct == false)
    {
        m_packetQueue.add(new WorldPacket(*data));
        // First packet push us into process set
        if(IsInWorld() && m_packetQueue.size() == 1)
            m_mapInstance->PushToProcessed(this);
    } else m_session->SendPacket(data);
}

void Player::PushPacketToQueue(WorldPacket *data)
{
    if(m_session == NULL)
        return;

    m_packetQueue.add(new WorldPacket(*data));
    // First packet push us into process set
    if(IsInWorld() && m_packetQueue.size() == 1)
        m_mapInstance->PushToProcessed(this);
}

uint32 Player::GetDifficulty(MapEntry *map)
{
    Group *group = GetGroup();

    if(map->IsRaid())
        return group ? group->GetRaidDifficulty() : iRaidType;
    return group ? group->GetDifficulty() : iInstanceType;
}

uint8 Player::UseRunes(uint32 *runes, bool theoretical)
{
    if(m_runeData == NULL)
        return 0x00;

    uint32 runesAvailable[5] = {0,0,0,0,0},
        runesToUse[4] = { runes[0], runes[1], runes[2], 0 };

    for(uint8 i = 0; i < 6; i++)
    {
        uint8 mask = (1<<i);
        if((m_runeData->runemask & mask) == 0)
            continue;
        ++runesAvailable[m_runeData->runes[i]];
    }

    for(uint8 i = 0; i < 3; ++i)
        if(runesToUse[i] != std::min(runesAvailable[i], runesToUse[i]))
            runesToUse[RUNE_TYPE_DEATH] += (runesToUse[i]-runesAvailable[i]);

    if(theoretical && runesToUse[RUNE_TYPE_DEATH] && runesAvailable[RUNE_TYPE_DEATH] < runesToUse[RUNE_TYPE_DEATH])
        return 0xFF;

    uint8 runeMask = m_runeData->runemask;
    for(uint8 i = 0; i < 6; i++)
    {
        uint8 mask = (1<<i);
        if((runeMask & mask) == 0)
            continue;
        if(runesToUse[m_runeData->runes[i]] == 0)
            continue;
        --runesToUse[m_runeData->runes[i]];
        runeMask &= ~mask;
        if(theoretical)
            continue;
        m_runeData->runeCD[i] = m_runeData->cooldownTimer;
    }

    if(theoretical == false)
        m_runeData->runemask = runeMask;
    return runeMask;
}

void Player::ProcessImmediateItemUpdate(Item *item)
{
    if(!IsInWorld())
        return;

    ByteBuffer &buff = GetMapInstance()->m_updateBuffer;
    if(uint32 count = item->BuildValuesUpdateBlockForPlayer(&buff, this, 0xFFFF))
        PushUpdateBlock(m_mapId, &buff, count);
    buff.clear();
    m_mapInstance->PushToProcessed(this);
}

void Player::ProcessPendingItemUpdates()
{
    if(m_pendingUpdates.empty() || !IsInWorld() || m_session == NULL)
        return;

    ByteBuffer &buff = GetMapInstance()->m_updateBuffer;
    while(m_pendingUpdates.size())
    {
        Item *item = *m_pendingUpdates.begin();
        m_pendingUpdates.erase(m_pendingUpdates.begin());
        if(uint32 count = item->BuildValuesUpdateBlockForPlayer(&buff, this, 0xFFFF))
            PushUpdateBlock(m_mapId, &buff, count);
        buff.clear();
    }
    m_mapInstance->PushToProcessed(this);
}

void Player::ItemFieldUpdated(Item *item)
{
    if(m_pendingUpdates.find(item) != m_pendingUpdates.end())
        return;
    m_pendingUpdates.insert(item);
}

void Player::ItemDestructed(Item *item)
{
    if(m_pendingUpdates.find(item) == m_pendingUpdates.end())
        return;
    m_pendingUpdates.erase(item);
}

void Player::OnFieldUpdated(uint16 index)
{
    Group *group;
    if((group = GetGroup()) && IsInWorld())
        group->HandleUpdateFieldChange(index, this);

    Unit::OnFieldUpdated(index);
}

static uint32 ratingsToModBonus[26] = 
{
    0,
    ITEM_STAT_DEFENSE_RATING,
    ITEM_STAT_DODGE_RATING,
    ITEM_STAT_PARRY_RATING,
    ITEM_STAT_BLOCK_RATING,
    ITEM_STAT_MELEE_HIT_RATING,
    ITEM_STAT_RANGED_HIT_RATING,
    ITEM_STAT_SPELL_HIT_RATING,
    ITEM_STAT_MELEE_CRITICAL_STRIKE_RATING,
    ITEM_STAT_RANGED_CRITICAL_STRIKE_RATING,
    ITEM_STAT_SPELL_CRITICAL_STRIKE_RATING,
    ITEM_STAT_MELEE_HIT_AVOIDANCE_RATING,
    ITEM_STAT_RANGED_HIT_AVOIDANCE_RATING,
    ITEM_STAT_SPELL_HIT_AVOIDANCE_RATING,
    ITEM_STAT_MELEE_RESILIENCE,
    ITEM_STAT_RANGED_RESILIENCE,
    ITEM_STAT_SPELL_RESILIENCE,
    ITEM_STAT_MELEE_HASTE_RATING,
    ITEM_STAT_RANGED_HASTE_RATING,
    ITEM_STAT_SPELL_HASTE_RATING,
    0,
    0,
    0,
    ITEM_STAT_EXPERTISE_RATING,
    ITEM_STAT_ARMOR_PENETRATION_RATING,
    ITEM_STAT_MASTERY_RATING
};

int32 Player::CalculatePlayerCombatRating(uint8 combatRating)
{
    ASSERT(combatRating < 26);

    int32 val = 0;
    if(ratingsToModBonus[combatRating])
        val += GetBonusesFromItems(ratingsToModBonus[combatRating]);
    switch(combatRating)
    {
    case 2:
        {
            gtFloat *dodgeRatio;
            if (getClass() != WARRIOR && getClass() != PALADIN && getClass() != DEATHKNIGHT && (dodgeRatio = dbcMeleeCrit.LookupEntry((getClass()-1)*100+(std::min<uint32>(MAXIMUM_ATTAINABLE_LEVEL, getLevel()) - 1))))
                val += ((float)((GetAgility() - baseStats->baseStat[STAT_AGILITY]) * (dodgeRatio->val * 100.f) * crit_to_dodge[getClass()]))/GetRatioForCombatRating(2);
        }break;
    case 3:
        val += (GetStrength() - baseStats->baseStat[STAT_STRENGTH]) * 0.27f;
        break;
    case 5: case 6: case 7:
        {   // Add base hit to our individual ratings
            val += GetBonusesFromItems(ITEM_STAT_HIT_RATING);
        }break;
    case 8: case 9: case 10:
        {   // Add base crit to our individual ratings
            val += GetBonusesFromItems(ITEM_STAT_CRITICAL_STRIKE_RATING);
        }break;
    case 11: case 12: case 13:
        {
            // TODO: Hit avoidance rating is a bit harder to do
            val += GetBonusesFromItems(ITEM_STAT_HIT_REDUCTION_RATING);
        }break;
    case 14: case 15: case 16:
        {   // Add base resilience to our individual res ratings
            val += GetBonusesFromItems(ITEM_STAT_RESILIENCE_RATING);
            // Also add crit reduction rating, though it's not entirely used
            val += GetBonusesFromItems(ITEM_STAT_CRITICAL_REDUCTION_RATING);
        }break;
    case 17: case 18: case 19:
        {   // Add base haste to our individual haste ratings
            val += GetBonusesFromItems(ITEM_STAT_HASTE_RATING);
        }break;
    case 20: case 21: case 22:
        {
            if(Item *item = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+(combatRating-20)))
            {
                if(AuraInterface::modifierMap *ratingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RATING))
                {
                    for(AuraInterface::modifierMap::iterator itr = ratingMod->begin(); itr != ratingMod->end(); itr++)
                    {
                        // Weapon ratings are 0x01, so skip them here
                        if((itr->second->m_miscValue[0] & 0x1) == 0)
                            continue;
                        if(itr->second->m_spellInfo->EquippedItemSubClassMask & (1<<item->GetProto()->SubClass))
                            val += itr->second->m_amount;
                    }
                }
            }
        }break;
    case 24:
        {
            if(AuraInterface::modifierMap *ratingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MASTERY))
                for(AuraInterface::modifierMap::iterator itr = ratingMod->begin(); itr != ratingMod->end(); itr++)
                    val += itr->second->m_amount;
        }break;
    }

    return val;
}

void Player::UpdateCombatRating(uint8 combatRating, float value)
{
    switch(combatRating)
    {
    case 2:
        SetFloatValue(PLAYER_DODGE_PERCENTAGE, GetBaseDodge()+value);
        break;
    case 3:
        SetFloatValue(PLAYER_PARRY_PERCENTAGE, GetBaseParry()+value);
        break;
    case 4:
        SetFloatValue(PLAYER_BLOCK_PERCENTAGE, GetBaseBlock()+value);
        break;
    case 8:
        SetFloatValue(PLAYER_CRIT_PERCENTAGE, value);
        SetFloatValue(PLAYER_OFFHAND_CRIT_PERCENTAGE, value);
        break;
    case 9:
        SetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE, value);
        break;
    case 10:
        for(uint8 i = 0; i < 7; i++)
            SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE+i, value);
        break;
    case 17:
        SetFloatValue(PLAYER_FIELD_MOD_HASTE, RONIN_UTIL::PercentFloatVar(value)/100.f);
        m_AuraInterface.UpdateAuraModsWithModType(SPELL_AURA_MOD_CD_FROM_HASTE);
        break;
    case 18:
        SetFloatValue(PLAYER_FIELD_MOD_RANGED_HASTE, RONIN_UTIL::PercentFloatVar(value)/100.f);
        break;
    case 19:
        SetFloatValue(UNIT_MOD_CAST_HASTE, RONIN_UTIL::PercentFloatVar(value)/100.f);
        break;
    case 23:
        SetUInt32Value(PLAYER_EXPERTISE, floor(value));
        SetUInt32Value(PLAYER_OFFHAND_EXPERTISE, floor(value));
        break;
    case 25:
        SetFloatValue(PLAYER_MASTERY, value);
        break;
    }
}

float Player::GetRatioForCombatRating(uint8 cr)
{
    ASSERT(cr<26);
    uint32 level = std::min<uint32>(getLevel(), MAXIMUM_ATTAINABLE_LEVEL);
    gtFloat *combatRating = dbcCombatRating.LookupEntry( cr * 100 + level - 1 ), *scalingCombatRating = dbcCombatRatingScaling.LookupEntry((getClass()-1)*32+cr+1);
    if(combatRating && scalingCombatRating && combatRating->val > 0.f)
        return scalingCombatRating->val / combatRating->val;
    return 1.f;
}

void Player::UpdatePlayerRatings()
{
    for(uint32 cr = 0, index = PLAYER_RATING_MODIFIER_WEAPON_SKILL; index < PLAYER_RATING_MODIFIER_MAX; cr++, index++)
    {
        int32 val = CalculatePlayerCombatRating(cr);
        if(index == PLAYER_RATING_MODIFIER_PARRY && !HasSpellWithEffect(SPELL_EFFECT_PARRY))
            val = 0; // Parry requires that we have the ability to parry, weirdly enough
        if(AuraInterface::modifierMap *ratingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RATING))
        {
            for(AuraInterface::modifierMap::iterator itr = ratingMod->begin(); itr != ratingMod->end(); itr++)
            {
                // Weapon ratings are 0x01, so skip them here
                if(itr->second->m_miscValue[0] & 0x1)
                    continue;
                if(itr->second->m_miscValue[0] & (1<<cr))
                    val += itr->second->m_amount;
            }
        }

        if(index == PLAYER_RATING_MODIFIER_PARRY && !HasSpellWithEffect(SPELL_EFFECT_PARRY))
            val = 0; // Parry requires that we have the ability to parry, weirdly enough

        if(AuraInterface::modifierMap *ratingMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_RATING_FROM_STAT))
            for(AuraInterface::modifierMap::iterator itr = ratingMod->begin(); itr != ratingMod->end(); itr++)
                if(itr->second->m_miscValue[0] & (1<<cr))
                    val += (float(GetStat(itr->second->m_miscValue[1]))*float(itr->second->m_amount/100.f));

        if(index == PLAYER_RATING_MODIFIER_MASTERY && !m_AuraInterface.HasAurasWithModType(SPELL_AURA_MASTERY))
            val = 0; // Mastery requires the aura before the rating can come into effect, so nullify it here
        if(index == PLAYER_RATING_MODIFIER_PARRY && !HasSpellWithEffect(SPELL_EFFECT_PARRY))
            val = 0; // Parry requires that we have the ability to parry, weirdly enough

        // Now that we have the calculated value, set it for player
        SetUInt32Value(index, std::max<int32>(0, val));
        // Multiply the overall rating with the set ratio
        UpdateCombatRating(cr, float(val)*GetRatioForCombatRating(cr));
    }
}

void Player::UpdatePlayerDamageDoneMods()
{
    uint32 itemBonus = GetBonusesFromItems(ITEM_STAT_SPELL_POWER);

    uint32 spellPowerOverride = itemBonus;
    if(AuraInterface::modifierMap *damageMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_OVERRIDE_SPELL_POWER_BY_AP_PCT))
    {
        float attackPowerMod = 0.0f;
        for(AuraInterface::modifierMap::iterator itr = damageMod->begin(); itr != damageMod->end(); itr++)
            attackPowerMod += float(itr->second->m_amount)/100.f;
        spellPowerOverride = float2int32(float(CalculateAttackPower())*attackPowerMod);
    }

    int32 negative = 0;
    for(uint8 school = SCHOOL_HOLY; school < SCHOOL_SPELL; school++)
    {
        SetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+school, std::max<uint32>(spellPowerOverride, itemBonus+GetDamageDoneMod(school, true, &negative)));
        SetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+school, negative);
    }

    SetUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS, std::max<uint32>(spellPowerOverride, itemBonus+GetHealingDoneMod(true, &negative)));
    SetFloatValue(PLAYER_FIELD_MOD_HEALING_PCT, GetHealingDonePctMod(true));
}

static uint32 statToModBonus[MAX_STAT] = 
{
    ITEM_STAT_STRENGTH,
    ITEM_STAT_AGILITY,
    ITEM_STAT_STAMINA,
    ITEM_STAT_INTELLECT,
    ITEM_STAT_SPIRIT
};

static uint32 schooltoResModBonus[MAX_RESISTANCE] =
{
    ITEM_STAT_PHYSICAL_RESISTANCE,
    ITEM_STAT_HOLY_RESISTANCE,
    ITEM_STAT_FIRE_RESISTANCE,
    ITEM_STAT_NATURE_RESISTANCE,
    ITEM_STAT_FROST_RESISTANCE,
    ITEM_STAT_SHADOW_RESISTANCE,
    ITEM_STAT_ARCANE_RESISTANCE
};

int32 Player::GetBonusMana()
{
    return GetBonusesFromItems(ITEM_STAT_HEALTH);
}

int32 Player::GetBonusHealth()
{
    return GetBonusesFromItems(ITEM_STAT_HEALTH);
}

int32 Player::GetBonusStat(uint8 stat)
{
    return GetBonusesFromItems(statToModBonus[stat]);
}

int32 Player::GetBonusResistance(uint8 school)
{
    return GetBonusesFromItems(schooltoResModBonus[school]);
}

int32 Player::GetBaseAttackTime(uint8 weaponType)
{
    SpellShapeshiftFormEntry *shapeShiftFormEntry;
    if(weaponType != 2 && (shapeShiftFormEntry = dbcSpellShapeshiftForm.LookupEntry(GetShapeShift())))
        if(uint32 attackSpeed = shapeShiftFormEntry->attackSpeed)
            return attackSpeed;

    Item *item;
    int32 speed = 2000;
    if(!disarmed && (item = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+weaponType)))
        speed = item->GetProto()->Delay;
    else if(weaponType != 0 && item == NULL)
        speed = 0;
    return speed;
}

int32 Player::GetBaseMinDamage(uint8 weaponType)
{
    int32 damage = 1.0f;
    if( Item *item = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+weaponType) )
        if(disarmed == false) damage += item->GetProto()->minDamage;
    return damage;
}

int32 Player::GetBaseMaxDamage(uint8 weaponType)
{
    int32 damage = 2.0f;
    if( Item *item = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND+weaponType) )
        if(disarmed == false) damage += item->GetProto()->maxDamage;
    return damage;
}

int32 Player::GetBonusAttackPower()
{
    return GetBonusesFromItems(ITEM_STAT_ATTACK_POWER);
}

int32 Player::GetBonusRangedAttackPower()
{
    return GetBonusesFromItems(ITEM_STAT_ATTACK_POWER)+GetBonusesFromItems(ITEM_STAT_RANGED_ATTACK_POWER);
}

#define IS_ARENA(x) ( (x) >= BATTLEGROUND_ARENA_2V2 && (x) <= BATTLEGROUND_ARENA_5V5 )

void Player::SaveToDB(bool bNewCharacter /* =false */)
{
    bool in_arena = false;
    QueryBuffer * buf = ((!sWorld.DisableBufferSaving || bNewCharacter) ? new QueryBuffer() : NULL);
    /*if( m_bg != NULL && IS_ARENA( m_bg->GetType() ) )
        in_arena = true;*/

    m_playerInfo->dataLock.Acquire();
    m_playerInfo->lastDeathState = getDeathState();
    m_playerInfo->lastLevel = getLevel();
    m_playerInfo->lastMapID = m_mapId;
    m_playerInfo->lastInstanceID = m_instanceId;
    m_playerInfo->lastZone = m_zoneId;
    GetPosition(m_playerInfo->lastPositionX, m_playerInfo->lastPositionY, m_playerInfo->lastPositionZ, m_playerInfo->lastOrientation);
    m_playerInfo->lastOnline = UNIXTIME;

    std::stringstream ss;
    ss << "REPLACE INTO character_data VALUES ("
    << uint32(m_playerInfo->charGuid.getLow()) << ", "
    << "'" << m_playerInfo->charName.c_str() << "', "
    << ((uint32)m_playerInfo->charRace) << ","
    << ((uint32)m_playerInfo->charClass) << ","
    << ((uint32)m_playerInfo->charTeam) << ","
    << ((uint32)m_playerInfo->charAppearance) << ","
    << ((uint32)m_playerInfo->charAppearance2) << ","
    << ((uint32)m_playerInfo->charAppearance3) << ","
    << ((uint32)m_playerInfo->charCustomizeFlags) << ","
    << ((uint32)m_playerInfo->lastDeathState) << ","
    << m_playerInfo->lastLevel << ","
    << m_playerInfo->lastMapID << ","
    << m_playerInfo->lastInstanceID << ","
    << m_playerInfo->lastPositionX << ","
    << m_playerInfo->lastPositionY << ","
    << m_playerInfo->lastPositionZ << ","
    << m_playerInfo->lastOrientation << ","
    << m_playerInfo->lastZone << ",";
    m_playerInfo->dataLock.Release();

    // This field is a combination of health PCT * 100 stacked on main power field PCT * 100
    ss << uint32(bNewCharacter ? 0x27102710 : ((uint32(GetFieldPctLg(UNIT_FIELD_HEALTH, UNIT_FIELD_MAXHEALTH))<<16) | uint32(GetFieldPctLg(UNIT_FIELD_POWERS, UNIT_FIELD_MAXPOWERS)))) << ", ";
    ss << uint32((GetPowerPct(UNIT_FIELD_POWERS_4, UNIT_FIELD_MAXPOWERS_4)<<24) | (GetPowerPct(UNIT_FIELD_POWERS_3, UNIT_FIELD_MAXPOWERS_3)<<16) | (GetPowerPct(UNIT_FIELD_POWERS_2, UNIT_FIELD_MAXPOWERS_2)<<8) | GetPowerPct(UNIT_FIELD_POWERS_1, UNIT_FIELD_MAXPOWERS_1)) << ", ";
    ss << GetUInt32Value(PLAYER_FIELD_BYTES) << "," << GetUInt32Value(PLAYER_FIELD_BYTES2) << ","
        << GetUInt32Value(PLAYER_XP) << "," << GetUInt64Value(PLAYER_FIELD_COINAGE) << ","
        << GetUInt32Value(PLAYER_CHARACTER_POINTS) << "," << GetUInt32Value(PLAYER_CHOSEN_TITLE) << ","
        << GetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX) << ",";

    uint32 talentStack = 0x00000000;
    m_talentInterface.GetActiveTalentTabStack(talentStack);
    // Player talent information
    ss << uint32(m_talentInterface.GetActiveSpec()) << ", " << uint32(m_talentInterface.GetSpecCount()) << ", " << uint32(m_talentInterface.GetTalentResets()) << ", " << int32(m_talentInterface.GetBonusTalentPoints()) << ", " << talentStack << ", ";

    // Bind position for hearth, worst case eject and ghetto unstuck
    ss << m_bindData.mapId << ", " << m_bindData.posX << ", " << m_bindData.posY << ", " << m_bindData.posZ << ", " << m_bindData.zoneId << ", ";

    // taxi
    if(m_taxiData)
    {
        ss << m_taxiData->CurrentPath->GetID() << ", "
        << m_taxiData->MoveTime << ", "
        << m_taxiData->TravelTime << ", "
        << m_taxiData->ModelId << ", ";
    } else ss << "0, 0, 0, 0, ";

    // Transport data
    if(m_CurrentTransporter)
    {
        float transx, transy, transz, transo;
        m_movementInterface.GetTransportPosition(transx, transy, transz, transo);
        ss << uint32(m_CurrentTransporter->GetLowGUID()) << ", " << transx << ", " << transy << ", " << transz << ", ";
    } else ss << "0, 0, 0, 0, ";

    // instances
    ss << uint32(0) << ", " << float(0.f) << ", " << float(0.f) << ", " << float(0.f) << ", " << float(0.f) << ", ";

    // Misc player data
    ss << uint32(iInstanceType) << ", " << uint32(iRaidType) << ", " << uint32(m_restData.isResting) << ", " << uint32(m_restData.restState) << ", " << uint32(m_restData.restAmount) << ", " << uint32(m_restData.areaTriggerId);
    ss << ", " << uint64(sWorld.GetWeekStart()) << ", " << uint64(UNIXTIME) << ", 0, 0);"; // Reset for position and talents

    if(buf)
        buf->AddQueryStr(ss.str());
    else CharacterDatabase.WaitExecuteNA(ss.str().c_str());

    // Achievements
    AchieveMgr.SaveAchievementData(GetGUID(), buf);

    // Criteria
    AchieveMgr.SaveCriteriaData(GetGUID(), buf);

    // Glyphs
    m_talentInterface.SaveActionButtonData(buf);

    // Auras
    _SavePlayerAuras(buf);

    // Cooldowns
    _SavePlayerCooldowns(buf);

    // Equipment sets
    _SaveEquipmentSets(buf);

    // Exploration data
    _SaveExplorationData(buf);

    // Faction data
    m_factionInterface.SaveFactionData(buf);

    // Glyphs
    m_talentInterface.SaveGlyphData(buf);

    // Inventory
    m_inventory.mSaveItemsToDatabase(bNewCharacter, buf);

    // Currency
    m_currency.SaveToDB(buf);

    // Known titles
    _SaveKnownTitles(buf);

    // Quest log
    _SavePlayerQuestLog(buf);

    // Completed quests
    _SaveCompletedQuests(buf);

    // Skills
    _SaveSkills(buf);

    // Social
    _SaveSocial(buf);

    // Spells
    _SaveSpells(buf);

    // Talents
    m_talentInterface.SaveTalentData(buf);

    // Taxi masks
    _SaveTaxiMasks(buf);

    // timestamps
    _SaveTimeStampData(buf);

    // GM Ticket
    if(GM_Ticket* ticket = sTicketMgr.GetGMTicketByPlayer(GetGUID()))
        sTicketMgr.SaveGMTicket(ticket, buf);

    if(buf == NULL)
        return;
    CharacterDatabase.AddQueryBuffer(buf);
}

void Player::DeleteFromDB(WoWGuid guid)
{
    if(Corpse* c = objmgr.GetCorpseByOwner(guid.getLow()))
        CharacterDatabase.Execute("DELETE FROM corpses WHERE guid = %u", c->GetLowGUID());

    CharacterDatabase.Execute("DELETE FROM account_characters WHERE charGuid = '%u';", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM auctions WHERE owner = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM charters WHERE leaderGuid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM mailbox WHERE player_guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM guild_members WHERE playerid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM item_enchantments WHERE itemguid IN(SELECT itemguid FROM item_data WHERE ownerguid = %u)", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM item_data WHERE ownerguid = %u", guid.getLow());

    CharacterDatabase.Execute("DELETE FROM character_achievements WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_actions WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_auras WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_bans WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_cooldowns WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_criteria_data WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_currency WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_equipmentsets WHERE ownerguid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_exploration WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_reputation WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_glyphs WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_inventory WHERE guid=%u",guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_known_titles WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_questlog WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_quests_completed WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_skills WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_social WHERE guid = %u OR socialguid = %u", guid.getLow(), guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_spells WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_talents WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_taximasks WHERE guid = %u", guid.getLow());
    CharacterDatabase.Execute("DELETE FROM character_timestamps WHERE guid = %u", guid.getLow());
    CharacterDatabase.WaitExecute("DELETE FROM character_data WHERE guid = %u", guid.getLow());
}

bool Player::LoadFromDB()
{
    AsyncQuery * q = new AsyncQuery( new SQLClassCallbackP0<Player>(this, &Player::LoadFromDBProc) );
    q->AddQuery("SELECT load_data, load_data2, playerBytes, playerBytes2, experience, gold, availableProfPoints, selectedTitle, watchedFaction, \
        talentActivespec, talentSpecCount, talentResetCounter, talentBonusPoints, talentStack, \
        bindmapId, bindpositionX, bindpositionY, bindpositionZ, bindzoneId, \
        taxiPath, taxiMoveTime, taxiTravelTime, taxiMountId, \
        transportGuid, transportX, transportY, transportZ, \
        entryPointMapId, entryPointX, entryPointY, entryPointZ, entryOrientation, \
        instanceDifficulty, raidDifficulty, isResting, restState, restTime, restAreaTrigger, \
        lastWeekResetTime, lastSaveTime, needPositionReset, needTalentReset FROM character_data WHERE guid='%u'", m_objGuid.getLow());

    q->AddQuery("SELECT * FROM character_actions WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_auras WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_bans WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_currency WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_cooldowns WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_achievements WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_criteria_data WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_equipmentsets WHERE ownerguid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_exploration WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_glyphs WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT character_inventory.guid,character_inventory.itemguid,item_data.itementry,item_data.containerguid,item_data.creatorguid,item_data.count,item_data.flags,item_data.randomseed,item_data.randomproperty,item_data.durability,item_data.textid,item_data.playedtime,item_data.spellcharges,item_data.giftitemid,item_data.giftcreatorguid,character_inventory.container,character_inventory.slot FROM character_inventory JOIN item_data ON character_inventory.guid = item_data.ownerguid AND character_inventory.itemguid = item_data.itemguid WHERE character_inventory.guid = '%u' ORDER BY container,slot", m_objGuid.getLow());
    q->AddQuery("SELECT item_enchantments.itemguid, item_data.itementry, item_enchantments.enchantslot, item_enchantments.enchantid, item_enchantments.enchantsuffix, item_enchantments.enchantcharges, item_enchantments.enchantexpiretimer FROM item_enchantments JOIN item_data ON item_data.itemguid = item_enchantments.itemguid WHERE item_enchantments.itemguid IN(SELECT item_data.itemguid FROM item_data WHERE item_data.ownerguid = '%u');", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_known_titles WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_questlog WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_quests_completion_masks WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_quests_completed_repeatable WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_reputation WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_skills WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_social WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_spells WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_talents WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_taximasks WHERE guid = '%u'", m_objGuid.getLow());
    q->AddQuery("SELECT * FROM character_timestamps WHERE guid = '%u'", m_objGuid.getLow());
    CharacterDatabase.QueueAsyncQuery(q);
    return true;
}

void Player::LoadFromDBProc(QueryResultVector & results)
{
    if(m_session == NULL || results[PLAYER_LO_DATA].result == NULL)
    {
        RemovePendingPlayer();
        return;
    }

    if(results[PLAYER_LO_BANS].result != NULL)
    {
        bool banned = true;
        if(Field *fields = results[PLAYER_LO_BANS].result->Fetch())
            if(fields[1].GetBool() && fields[1].GetUInt64() <= UNIXTIME)
                banned = false;
        if(banned)
        {
            RemovePendingPlayer(CHAR_LOGIN_TEMPORARY_GM_LOCK);
            return;
        }
    }

    Field *fields = results[PLAYER_LO_DATA].result->Fetch();
    // Health and main power are broken into 16 bits and multiplied by 100 as floats
    m_loadData.push_back(std::make_pair<uint8, float>(0, (((float)(fields[PLAYERLOAD_FIELD_LOAD_DATA].GetUInt32()>>16))/100.f)));
    m_loadData.push_back(std::make_pair<uint8, float>(1, (((float)(fields[PLAYERLOAD_FIELD_LOAD_DATA].GetUInt32() & 0xFFFF))/100.f)));
    // Broken into 8 bits for each power field
    m_loadData.push_back(std::make_pair<uint8, float>(2,  ((float)( fields[PLAYERLOAD_FIELD_LOAD_DATA_1].GetUInt32() & 0x000000FF ))));
    m_loadData.push_back(std::make_pair<uint8, float>(3,  ((float)((fields[PLAYERLOAD_FIELD_LOAD_DATA_1].GetUInt32() & 0x0000FF00)>>8))));
    m_loadData.push_back(std::make_pair<uint8, float>(4,  ((float)((fields[PLAYERLOAD_FIELD_LOAD_DATA_1].GetUInt32() & 0x00FF0000)>>16))));
    m_loadData.push_back(std::make_pair<uint8, float>(5,  ((float)((fields[PLAYERLOAD_FIELD_LOAD_DATA_1].GetUInt32() & 0xFF000000)>>24))));

    SetUInt32Value( PLAYER_FIELD_BYTES, fields[PLAYERLOAD_FIELD_PLAYER_BYTES].GetUInt32() );
    SetUInt32Value( PLAYER_FIELD_BYTES2, fields[PLAYERLOAD_FIELD_PLAYER_BYTES2].GetUInt32() );

    SetUInt32Value(PLAYER_XP, fields[PLAYERLOAD_FIELD_XP].GetUInt32());
    SetUInt32Value(PLAYER_FIELD_COINAGE, fields[PLAYERLOAD_FIELD_GOLD].GetUInt32());
    SetUInt32Value(PLAYER_CHARACTER_POINTS, fields[PLAYERLOAD_FIELD_AVAILABLE_PROF_POINTS].GetUInt32());
    SetUInt32Value(PLAYER_CHOSEN_TITLE, fields[PLAYERLOAD_FIELD_SELECTED_TITLE].GetUInt32());
    SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, fields[PLAYERLOAD_FIELD_WATCHED_FACTION_INDEX].GetUInt32());

    // After player info creation, load all extra data
    m_talentInterface.SetTalentData(fields[PLAYERLOAD_FIELD_TALENT_ACTIVE_SPEC].GetUInt32(), fields[PLAYERLOAD_FIELD_TALENT_SPEC_COUNT].GetUInt32(),
        fields[PLAYERLOAD_FIELD_TALENT_RESET_COUNTER].GetUInt32(), fields[PLAYERLOAD_FIELD_TALENT_BONUS_POINTS].GetInt32(), fields[PLAYERLOAD_FIELD_TALENT_SPECSTACK].GetUInt32());

    m_bindData.mapId = fields[PLAYERLOAD_FIELD_BINDMAPID].GetUInt32();
    m_bindData.posX = fields[PLAYERLOAD_FIELD_BINDPOSITION_X].GetFloat();
    m_bindData.posY = fields[PLAYERLOAD_FIELD_BINDPOSITION_Y].GetFloat();
    m_bindData.posZ = fields[PLAYERLOAD_FIELD_BINDPOSITION_Z].GetFloat();
    m_bindData.zoneId = fields[PLAYERLOAD_FIELD_BINDZONEID].GetUInt32();

    uint32 taxipath = fields[PLAYERLOAD_FIELD_TAXI_PATH].GetUInt32();
    if(TaxiPath *path = sTaxiMgr.GetTaxiPath(taxipath))
    {
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
        m_taxiData = new Player::TaxiData();
        m_taxiData->CurrentPath = path;
        m_taxiData->ArrivalTime = m_taxiData->MoveTime = fields[PLAYERLOAD_FIELD_TAXI_MOVETIME].GetUInt32();
        m_taxiData->TravelTime = fields[PLAYERLOAD_FIELD_TAXI_TRAVELTIME].GetUInt32();
        SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, (m_taxiData->ModelId = fields[PLAYERLOAD_FIELD_TAXI_MOUNTID].GetUInt32()));
        if(path->HasMapChange(m_mapId)) m_taxiData->ArrivalTime -= 2000;
    }

    uint32 transGuid = fields[PLAYERLOAD_FIELD_TRANSPORTERGUID].GetUInt32();
    GetMovementInterface()->SetTransportData(uint64(transGuid ? MAKE_NEW_GUID(transGuid, 0, HIGHGUID_TYPE_TRANSPORTER) : 0), 0,
        fields[PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_X].GetFloat(), fields[PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_Y].GetFloat(),
        fields[PLAYERLOAD_FIELD_TRANSPORTER_OFFSET_Z].GetFloat(), 0.f, 0);

    fields[PLAYERLOAD_FIELD_ENTRYPOINT_MAP].GetUInt32();
    fields[PLAYERLOAD_FIELD_ENTRYPOINT_X].GetFloat();
    fields[PLAYERLOAD_FIELD_ENTRYPOINT_Y].GetFloat();
    fields[PLAYERLOAD_FIELD_ENTRYPOINT_Z].GetFloat();
    fields[PLAYERLOAD_FIELD_ENTRYPOINT_O].GetFloat();

    iInstanceType = fields[PLAYERLOAD_FIELD_INSTANCE_DIFFICULTY].GetUInt32();
    iRaidType = fields[PLAYERLOAD_FIELD_RAID_DIFFICULTY].GetUInt32();

    if(getLevel() != GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
    {
        if(m_restData.isResting = fields[PLAYERLOAD_FIELD_ISRESTING].GetUInt8())
            SetFlag(PLAYER_FLAGS, PLAYER_FLAG_RESTING); //put zzz icon

        m_restData.restState = fields[PLAYERLOAD_FIELD_RESTSTATE].GetUInt8();
        m_restData.restAmount = fields[PLAYERLOAD_FIELD_RESTTIME].GetUInt32();
        m_restData.areaTriggerId = fields[PLAYERLOAD_FIELD_RESTAREATRIGGER].GetUInt32();
    }

    m_talentInterface.LoadActionButtonData(results[PLAYER_LO_ACTIONS].result);
    _LoadPlayerAuras(results[PLAYER_LO_AURAS].result);
    _LoadPlayerCooldowns(results[PLAYER_LO_COOLDOWNS].result);
    m_currency.LoadFromDB(fields[PLAYERLOAD_FIELD_LAST_WEEK_RESET_TIME].GetUInt64(), results[PLAYER_LO_CURRENCY].result);
    AchieveMgr.LoadAchievementData(GetGUID(), m_playerInfo, results[PLAYER_LO_ACHIEVEMENT_DATA].result);
    AchieveMgr.LoadCriteriaData(GetGUID(), results[PLAYER_LO_CRITERIA_DATA].result);
    _LoadEquipmentSets(results[PLAYER_LO_EQUIPMENTSETS].result);
    _LoadExplorationData(results[PLAYER_LO_EXPLORATION].result);
    m_talentInterface.LoadGlyphData(results[PLAYER_LO_GLYPHS].result);
    _LoadKnownTitles(results[PLAYER_LO_KNOWN_TITLES].result);
    _LoadPlayerQuestLog(results[PLAYER_LO_QUEST_LOG].result);
    _LoadCompletedQuests(results[PLAYER_LO_QUESTS_COMPLETED].result, results[PLAYER_LO_QUESTS_COMPLETED_REPEATABLE].result);
    m_factionInterface.LoadFactionData(results[PLAYER_LO_REPUTATIONS].result);
    _LoadSkills(results[PLAYER_LO_SKILLS].result);
    _LoadSpells(results[PLAYER_LO_SPELLS].result);
    _LoadSocial(results[PLAYER_LO_SOCIAL].result);
    m_talentInterface.LoadTalentData(results[PLAYER_LO_TALENTS].result);
    _LoadTaxiMasks(results[PLAYER_LO_TAXIMASKS].result);
    _LoadTimeStampData(results[PLAYER_LO_TIMESTAMPS].result);
    m_inventory.mLoadItemsFromDatabase(results[PLAYER_LO_ITEMS].result, results[PLAYER_LO_ITEM_ENCHANTS].result);

    OnlineTime = UNIXTIME;
    if( fields[PLAYERLOAD_FIELD_NEEDS_POSITION_RESET].GetBool() )
        TeleportToHomebind();
    if( fields[PLAYERLOAD_FIELD_NEEDS_TALENT_RESET].GetBool() )
        m_talentInterface.ResetAllSpecs();

    m_playerInfo->m_loggedInPlayer = this;

    m_session->FullLogin(this);

    if( !isAlive() )
    {
        if((myCorpse = objmgr.GetCorpseByOwner(GetLowGUID())) == NULL)
            if((myCorpse = objmgr.LoadCorpse(GetLowGUID())) == NULL)
                myCorpse = CreateCorpse();
        if(myCorpse)
        {
            WorldPacket data(MSG_CORPSE_QUERY, 21);
            BuildCorpseInfo(&data, myCorpse);
            PushPacket(&data);
        }
    }

    m_movementInterface.OnRelocate(m_position);
    AchieveMgr.PlayerFinishedLoading(this);
}

void Player::_LoadPlayerAuras(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint8 auraSlot = fields[1].GetUInt8();
        uint32 spellId = fields[2].GetUInt32();
        SpellEntry *sp = dbcSpell.LookupEntry(spellId);
        if(sp == NULL)
            continue;

        uint16 auraFlags = fields[3].GetUInt16();
        uint8 auraLevel = fields[4].GetUInt8();
        int16 auraStackCharge = fields[5].GetInt16();
        WoWGuid casterGuid = fields[6].GetUInt64();
        time_t expiration = 0;
        if((expiration = fields[7].GetUInt64()) != 0)
        {
            if(expiration < 172800)
                expiration += UNIXTIME;
            else if(expiration <= UNIXTIME+5)
                continue;
        }

        Aura *aur = new Aura(this, sp, (auraFlags & 0xFF), auraLevel, auraStackCharge, expiration, casterGuid);
        for(uint8 x = 0; x < 3; x++)
        {
            // We either have the aura mod index saved, or we have no index saved and we can go on auraname application
            if((auraFlags & (1<<(x+8))) || ((auraFlags & 0xFF00) == 0 && sp->EffectApplyAuraName[x] != 0))
                aur->AddMod(x, sp->EffectApplyAuraName[x], fields[8+x].GetInt32(), fields[11+x].GetUInt32(), fields[14+x].GetInt32(), fields[17+x].GetFloat());
        }
        m_loadAuras.push_back(std::make_pair(auraSlot, aur));
    }while(result->NextRow());
}

void Player::_SavePlayerAuras(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_auras WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_auras WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    m_AuraInterface.SavePlayerAuras(&ss);
    if(ss.str().empty())
        return;

    if(buf)buf->AddQuery("REPLACE INTO character_auras VALUES %s;", ss.str().c_str());
    else CharacterDatabase.Execute("REPLACE INTO character_auras VALUES %s;", ss.str().c_str());
}

void Player::_LoadPlayerCooldowns(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint32 spellId = fields[1].GetUInt32();
        uint16 type = fields[2].GetUInt8();
        if( type >= NUM_COOLDOWN_TYPES )
            continue;
        uint32 category = fields[3].GetUInt32();
        time_t expireTime = fields[4].GetUInt64();
        if(expireTime <= UNIXTIME)
            continue;

        PlayerCooldown &cd = m_cooldownMap[type][category];
        cd.SpellId = spellId;
        cd.ExpireTime = expireTime;
        cd.ItemId = fields[5].GetUInt32();
    }while(result->NextRow());
}

void Player::_SavePlayerCooldowns(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_cooldowns WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_cooldowns WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(uint8 i = 0; i < NUM_COOLDOWN_TYPES; i++)
    {
        for(PlayerCooldownMap::iterator itr = m_cooldownMap[i].begin(); itr != m_cooldownMap[i].end(); itr++)
        {
            if(itr->second.ExpireTime <= UNIXTIME)
                continue;

            if(ss.str().length())
                ss << ", ";

            ss << "(" << GetLowGUID()
            << ", " << uint32(itr->second.SpellId)
            << ", " << uint32(i)
            << ", " << uint32(itr->first)
            << ", " << uint64(itr->second.ExpireTime)
            << ", " << uint32(itr->second.ItemId);
            ss << ")";
        }
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_cooldowns VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_cooldowns VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadEquipmentSets(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {

    }while(result->NextRow());
}

void Player::_SaveEquipmentSets(QueryBuffer * buf)
{

}

void Player::_LoadExplorationData(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint32 field = PLAYER_EXPLORED_ZONES_1+fields[1].GetUInt8();
        SetUInt32Value(field, fields[2].GetUInt32());
    }while(result->NextRow());
}

void Player::_SaveExplorationData(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_exploration WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_exploration WHERE guid = '%u';", GetLowGUID());

    uint32 offset = 0;
    std::stringstream ss;
    for(uint32 field = 0; field < 156; field++)
    {
        if(uint32 val = GetUInt32Value(PLAYER_EXPLORED_ZONES_1+field))
        {
            if(ss.str().length())
                ss << ", ";
            ss << "(" << GetLowGUID()
            << ", " << field
            << ", " << val;
            ss << ")";
        }
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_exploration VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_exploration VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadKnownTitles(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint32 field = PLAYER__FIELD_KNOWN_TITLES+fields[1].GetUInt8();
        SetUInt32Value(field, fields[2].GetUInt32());
    }while(result->NextRow());
}

void Player::_SaveKnownTitles(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_known_titles WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_known_titles WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(uint32 field = 0; field < 4; field++)
    {
        if(uint32 val = GetUInt32Value(PLAYER__FIELD_KNOWN_TITLES+field))
        {
            if(ss.str().length())
                ss << ", ";
            ss << "(" << GetLowGUID()
            << ", " << field
            << ", " << val;
            ss << ")";
        }
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_known_titles VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_known_titles VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadPlayerQuestLog(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint8 slot = fields[1].GetUInt32();
        Quest *quest = sQuestMgr.GetQuestPointer(fields[2].GetUInt32());
        if(quest == NULL)
            continue;
        if(m_questLog[slot] != 0)
            continue;

        QuestLogEntry *entry = new QuestLogEntry();
        entry->Init(quest, castPtr<Player>(this), slot);
        entry->Load(fields);
        entry->UpdatePlayerFields();
    }while(result->NextRow());
}

void Player::_SavePlayerQuestLog(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_questlog WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_questlog WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(uint8 i = 0; i < QUEST_LOG_COUNT; i++)
    {
        QuestLogEntry *questLog = GetQuestLogInSlot(i);
        if(questLog == NULL)
            continue;
        if(ss.str().length())
            ss << ", ";

        ss << "(" << ((uint32)GetLowGUID())
        << ", " << ((uint32)i)
        << ", " << ((uint32)questLog->GetQuest()->id)
        << ", " << ((uint32)questLog->getExpirationTime())
        << ", " << ((uint32)questLog->GetObjectiveCount(0))
        << ", " << ((uint32)questLog->GetObjectiveCount(1))
        << ", " << ((uint32)questLog->GetObjectiveCount(2))
        << ", " << ((uint32)questLog->GetObjectiveCount(3))
        << ", " << ((uint32)questLog->GetExplorationFlag())
        << ", " << ((uint32)questLog->GetPlayerSlainCount());
        ss << ")";
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_questlog VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_questlog VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadCompletedQuests(QueryResult *completionMasks, QueryResult *repeatable)
{
    if(completionMasks)
    {
        do
        {
            Field *fields = completionMasks->Fetch();
            uint16 index = fields[1].GetUInt16(); uint64 mask = fields[2].GetUInt64();
            m_completedQuests.insert(std::make_pair(index, mask));
            for(uint8 i = 0; i < 64; i++)
            {
                if((mask & (((uint64)(1))<<i)) == 0)
                    continue;
                uint32 questId = (((uint32)index)*64)+i;
                // Quest data parsing
                if(Quest *qst = sQuestMgr.GetQuestPointer(questId))
                {

                }
            }

        }while(completionMasks->NextRow());
    }

    if(repeatable)
    {
        do
        {
            Field *fields = repeatable->Fetch();
            Quest *qst = sQuestMgr.GetQuestPointer(fields[1].GetUInt32());
            if(qst == NULL || qst->qst_is_repeatable == UNREPEATABLE_QUEST)
                continue;
            time_t timeStamp = fields[2].GetUInt64();
            switch(qst->qst_is_repeatable)
            {
            case REPEATABLE_QUEST:
                m_completedRepeatableQuests.insert(std::make_pair(qst->id, timeStamp));
                break;
            case REPEATABLE_DAILY:
                {
                    // Don't load up dailies if they're from before the last reset
                    if(timeStamp > sWorld.GetLastDailyResetTime())
                        m_completedDailyQuests.insert(std::make_pair(qst->id, timeStamp));
                }break;
            case REPEATABLE_WEEKLY:
                {
                    // Don't load up dailies if they're from before the last reset
                    if(timeStamp > sWorld.GetWeekStart())
                        m_completedWeeklyQuests.insert(std::make_pair(qst->id, timeStamp));
                }break;
            }
        }while(repeatable->NextRow());
    }
}

void Player::_SaveCompletedQuests(QueryBuffer * buf)
{
    if(buf)
    {
        buf->AddQuery("DELETE FROM character_quests_completion_masks WHERE guid = '%u';", GetLowGUID());
        buf->AddQuery("DELETE FROM character_quests_completed_repeatable WHERE guid = '%u';", GetLowGUID());
    }
    else
    {
        CharacterDatabase.Execute("DELETE FROM character_quests_completion_masks WHERE guid = '%u';", GetLowGUID());
        CharacterDatabase.Execute("DELETE FROM character_quests_completed_repeatable WHERE guid = '%u';", GetLowGUID());
    }

    std::stringstream ss;
    // Store our completed quest masks
    for(std::map<uint16, uint64>::iterator itr = m_completedQuests.begin(); itr != m_completedQuests.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID() << ", " << uint32(itr->first) << ", " << itr->second << ")";
    }
    // Push our stream into the database
    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_quests_completion_masks VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_quests_completion_masks VALUES %s;", ss.str().c_str());
        ss.clear();
    }

    // Store our completed repeatable quests into the stream
    for(std::map<uint32, time_t>::iterator itr = m_completedRepeatableQuests.begin(); itr != m_completedRepeatableQuests.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID() << ", " << uint32(itr->first) << ", " << itr->second << ")";
    }

    // Store our completed daily quests into the stream
    for(std::map<uint32, time_t>::iterator itr = m_completedDailyQuests.begin(); itr != m_completedDailyQuests.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID() << ", " << uint32(itr->first) << ", " << itr->second << ")";
    }

    // Store our completed weekly quests into the stream
    for(std::map<uint32, time_t>::iterator itr = m_completedWeeklyQuests.begin(); itr != m_completedWeeklyQuests.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID() << ", " << uint32(itr->first) << ", " << itr->second << ")";
    }

    // Push our repeatable quest stream into the database
    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_quests_completed_repeatable VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_quests_completed_repeatable VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadSkills(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint16 skillId = fields[1].GetUInt16();
        SkillLineEntry *skillLine = dbcSkillLine.LookupEntry(skillId);
        if(skillLine == NULL)
            continue;

        uint8 skillPos = fields[2].GetUInt8();
        uint32 field = skillPos/2, offset = skillPos&1;
        if(GetUInt16Value(PLAYER_SKILL_LINEID_0+field, offset))
            continue;

        m_skillIndexes.insert(std::make_pair(skillId, skillPos));
        SetUInt16Value(PLAYER_SKILL_LINEID_0 + field, offset, skillId);
        SetUInt16Value(PLAYER_SKILL_STEP_0 + field, offset, fields[3].GetUInt16());
        SetUInt16Value(PLAYER_SKILL_RANK_0 + field, offset, fields[4].GetUInt16());
        SetUInt16Value(PLAYER_SKILL_MAX_RANK_0 + field, offset, fields[5].GetUInt16());
        SetUInt16Value(PLAYER_SKILL_MODIFIER_0 + field, offset, 0);
        SetUInt16Value(PLAYER_SKILL_TALENT_0 + field, offset, 0);
    }while(result->NextRow());
}

void Player::_SaveSkills(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_skills WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_skills WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(auto itr = m_skillIndexes.begin(); itr != m_skillIndexes.end(); itr++)
    {
        uint32 field = itr->second/2, offset = itr->second&1;
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint32(itr->first)
        << ", " << uint32(itr->second)
        << ", " << uint32(GetUInt16Value(PLAYER_SKILL_STEP_0+field, offset))
        << ", " << uint32(GetUInt16Value(PLAYER_SKILL_RANK_0+field, offset))
        << ", " << uint32(GetUInt16Value(PLAYER_SKILL_MAX_RANK_0+field, offset));
        ss << ")";

    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_skills VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_skills VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadSocial(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        std::pair<WoWGuid, std::string> social_pair = std::make_pair(WoWGuid(fields[2].GetUInt64()), fields[3].GetString());
        switch(fields[1].GetUInt8())
        {
        case 0: m_friends.insert(social_pair); break;
        case 1: m_ignores.insert(social_pair); break;
        case 2: m_mutes.insert(social_pair); break;
        }
    }while(result->NextRow());
}

void Player::_SaveSocial(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_social WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_social WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    std::map<WoWGuid, std::string>::iterator itr;
    for(itr = m_friends.begin(); itr != m_friends.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint64(itr->first)
        << ", " << uint32(0)
        << ", " << itr->second;
        ss << ")";
    }
    for(itr = m_ignores.begin(); itr != m_ignores.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint64(itr->first)
        << ", " << uint32(1)
        << ", " << itr->second;
        ss << ")";
    }
    for(itr = m_mutes.begin(); itr != m_mutes.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";
        ss << "(" << GetLowGUID()
        << ", " << uint64(itr->first)
        << ", " << uint32(2)
        << ", " << itr->second;
        ss << ")";
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_social VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_social VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadSpells(QueryResult *result)
{
    if(result)
    {
        do
        {
            if(SpellEntry *sp = dbcSpell.LookupEntry(result->Fetch()[1].GetUInt32()))
            {
                if(guildmgr.IsGuildPerk(sp))
                    continue;
                m_spells.insert(sp->Id);
                for(uint8 i = 0; i < 3; i++)
                    if(uint8 effect = sp->Effect[i])
                        m_spellsByEffect[effect].insert(sp->Id);
                if(sp->SpellSkillLine)
                    m_spellsBySkill[sp->SpellSkillLine].insert(sp->Id);

                if(sp->Category)
                    m_spellCategories.insert(sp->Category);
            }
        }while(result->NextRow());
    }

    guildmgr.AddGuildPerks(this);

    if(m_session->CanUseCommand('c'))
        _AddLanguages(true);
    else _AddLanguages(sWorld.cross_faction_world);
}

void Player::_SaveSpells(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_spells WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_spells WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(auto itr = m_spells.begin(); itr != m_spells.end(); itr++)
    {
        if(SpellEntry *sp = dbcSpell.LookupEntry(*itr))
        {
            if(guildmgr.IsGuildPerk(sp))
                continue;
            if(ss.str().length())
                ss << ", ";
            ss << "(" << GetLowGUID() << ", " << uint32(*itr) << ")";
        }
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_spells VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_spells VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadTaxiMasks(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        m_taxiMask.SetBlock(fields[1].GetUInt8(), fields[2].GetUInt8());
    }while(result->NextRow());
}

void Player::_SaveTaxiMasks(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_taximasks WHERE guid = '%u';", GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_taximasks WHERE guid = '%u';", GetLowGUID());

    std::stringstream ss;
    for(uint8 i = 0; i < 114; i++)
    {
        if(uint8 taxiMask = m_taxiMask.GetBlock(i))
        {
            if(ss.str().length())
                ss << ", ";
            ss << "(" << GetLowGUID() << ", " << uint32(i) << ", " << uint32(taxiMask) << ")";
        }
    }

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_taximasks VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_taximasks VALUES %s;", ss.str().c_str());
    }
}

void Player::_LoadTimeStampData(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {

    }while(result->NextRow());
}

void Player::_SaveTimeStampData(QueryBuffer * buf)
{

}

uint32 GetSpellForLanguageID(uint32 LanguageID)
{
    switch(LanguageID)
    {
    case LANG_COMMON: return 668;
    case LANG_ORCISH: return 669;
    case LANG_TAURAHE: return 670;
    case LANG_DARNASSIAN: return 671;
    case LANG_DWARVISH: return 672;
    case LANG_THALASSIAN: return 813;
    case LANG_DRACONIC: return 814;
    case LANG_DEMONIC: return 815;
    case LANG_TITAN: return 816;
    case LANG_GNOMISH: return 7430;
    case LANG_TROLL: return 7341;
    case LANG_GUTTERSPEAK: return 17737;
    case LANG_DRAENEI: return 29932;
    } return 0;
}

uint32 GetSpellForLanguageSkill(uint32 SkillID)
{
    switch(SkillID)
    {
    case SKILL_LANG_COMMON: return 668;
    case SKILL_LANG_ORCISH: return 669;
    case SKILL_LANG_TAURAHE: return 670;
    case SKILL_LANG_DARNASSIAN: return 671;
    case SKILL_LANG_DWARVEN: return 672;
    case SKILL_LANG_THALASSIAN: return 813;
    case SKILL_LANG_DRACONIC: return 814;
    case SKILL_LANG_DEMON_TONGUE: return 815;
    case SKILL_LANG_TITAN: return 816;
    case SKILL_LANG_OLD_TONGUE: return 817;
    case SKILL_LANG_GNOMISH: return 7430;
    case SKILL_LANG_TROLL: return 7341;
    case SKILL_LANG_GUTTERSPEAK: return 17737;
    case SKILL_LANG_DRAENEI: return 29932;
    case SKILL_LANG_GOBLIN: return 69269;
    case SKILL_LANG_WORGEN: return 69270;
    } return 0;
}

///====================================================================
///  Create
///  params: p_newChar
///  desc:   data from client to create a new character
///====================================================================
void Player::CreateInDatabase()
{
    m_mapId = m_bindData.mapId = m_createInfo->mapId;
    m_zoneId = m_bindData.zoneId = m_createInfo->zoneId;
    SetPosition( (m_bindData.posX = m_createInfo->positionX), (m_bindData.posY = m_createInfo->positionY), (m_bindData.posZ = m_createInfo->positionZ), m_createInfo->Orientation);

    setLevel(std::max<uint32>(getClass() == DEATHKNIGHT ? 55 : 1, sWorld.StartLevel));

    SetUInt32Value(PLAYER_FIELD_COINAGE, sWorld.StartGold);

    InitTaxiNodes();

    if(getClass() == WARRIOR)
        SetShapeShift(FORM_BATTLESTANCE);
    else if(getClass() == DEATHKNIGHT)
        m_talentInterface.ModTalentPoints(-24);

    SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

    for(std::set<uint32>::iterator sp = m_createInfo->spell_list.begin(); sp != m_createInfo->spell_list.end(); sp++)
        m_spells.insert(*sp);

    m_factionInterface.CreateFactionData();

    _UpdateMaxSkillCounts();

    // Add actionbars
    for(std::list<CreateInfo_ActionBarStruct>::iterator itr = m_createInfo->bars.begin(); itr != m_createInfo->bars.end();itr++)
    {
        m_talentInterface.setAction(itr->button, itr->action, itr->type, 0);
        m_talentInterface.setAction(itr->button, itr->action, itr->type, 1);
    }

    // Force GM robes on GM's except 'az' status (if set to 1 in world.conf)
    if( GetSession()->HasGMPermissions() && sWorld.gm_force_robes && strstr(GetSession()->GetPermissions(), "az") == NULL)
    {
        GetInventory()->mAddItemToBestSlot(sItemMgr.LookupEntry(12064), 1, true);
        GetInventory()->mAddItemToBestSlot(sItemMgr.LookupEntry(2586), 1, true);
        GetInventory()->mAddItemToBestSlot(sItemMgr.LookupEntry(11508), 1, true);
    }
    else
    {
        for(std::list<CreateInfo_ItemStruct>::iterator is = m_createInfo->items.begin(); is != m_createInfo->items.end(); is++)
            if(ItemPrototype *proto = sItemMgr.LookupEntry((*is).protoid))
                GetInventory()->mAddItemToBestSlot(proto, (*is).amount, true);

        if(CharStartOutfitEntry *startOutfit = sWorld.GetStartOutfitEntry(getRace(), getClass(), getGender()))
        {
            for (uint8 s = 0; s < 24; ++s)
            {
                if (startOutfit->itemId[s] <= 0)
                    continue;

                ItemPrototype *proto = sItemMgr.LookupEntry(startOutfit->itemId[s]);
                if(proto == NULL)
                    continue;

                // BuyCount by default
                uint32 count = proto->BuyCount;

                // special amount for foor/drink
                if (proto->Class == ITEM_CLASS_CONSUMABLE && proto->SubClass == ITEM_CLASS_REAGENT)
                {
                    switch (proto->Spells[0].Category)
                    {
                    case 11:                                // food
                        count = getClass() == DEATHKNIGHT ? 10 : 4;
                        break;
                    case 59:                                // drink
                        count = 2;
                        break;
                    }
                    if (proto->Unique < count)
                        count = proto->Unique;
                }

                GetInventory()->mAddItemToBestSlot(proto, count, false);
            }
        }
    }
    UpdateFieldValues();

    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    for(uint8 i = 0; i < POWER_FIELDS_MAX; i++)
        SetUInt32Value(UNIT_FIELD_POWERS, GetUInt32Value(UNIT_FIELD_MAXPOWERS+i));

    SaveToDB(true);
}

void Player::setLevel(uint32 level)
{
    m_inventory.ModifyLevelBasedItemBonuses(false);
    uint32 prevLevel = GetUInt32Value(UNIT_FIELD_LEVEL);
    Unit::setLevel(level);
    m_inventory.ModifyLevelBasedItemBonuses(true);
    if(prevLevel != level)
    {
        UpdateFieldValues();
        _UpdateMaxSkillCounts();
        if (m_playerInfo)
            m_playerInfo->lastLevel = level;

        if(IsInWorld())
        {
            m_talentInterface.InitGlyphsForLevel(level);
            if(prevLevel > 9 || level > 9)
            {
                if(level <= 9)
                    m_talentInterface.ResetAllSpecs();
                else m_talentInterface.RecalculateAvailableTalentPoints();

                // If we're previously or currently above 9 and previously or currently below 9, resend achievement data
                if(prevLevel <= 9 || level <= 9)
                {
                    WorldPacket data(SMSG_ALL_ACHIEVEMENT_DATA, 20);
                    AchieveMgr.BuildAchievementData(GetGUID(), &data);
                    PushPacket(&data);
                }
            }

            ProcessVisibleQuestGiverStatus(); // For quests that require levels
            SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
            SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));
        }

        AchieveMgr.UpdateCriteriaValue(this, ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL, level, prevLevel);
        sLog.outDebug("Player %s set parameters to level %u", GetName(), level);
    }
}

void Player::SendMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, int32 Regen)
{
    if (int(MaxValue) == -1)
    {
        if (int(CurrentValue) != -1)
            StopMirrorTimer(Type);
        return;
    }

    WorldPacket data(SMSG_START_MIRROR_TIMER, (21));
    data << uint32(Type);
    data << CurrentValue;
    data << MaxValue;
    data << Regen;
    data << (uint8)0;
    data << (uint32)0;
    PushPacket(&data);
}

void Player::StopMirrorTimer(MirrorTimerType Type)
{
    m_movementInterface.StopMirrorTimer(Type);
    WorldPacket data(SMSG_STOP_MIRROR_TIMER, 4);
    data << uint32(Type);
    PushPacket(&data);
}

void Player::EventDismount(uint32 money, float x, float y, float z)
{
    ModUnsigned32Value( PLAYER_FIELD_COINAGE , -(int32)money );
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    SetPosition(x, y, z, true);

    if(m_taxiData == NULL)
    {
        m_movementInterface.setRooted(m_AuraInterface.HasAuraWithMechanic(MECHANIC_STUNNED));
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);
        m_movementInterface.OnTaxiEnd();
    }   
    else// If we have multiple "trips" to do, "jump" on the next one :p
    {
        m_taxiData->TravelTime = 0;

        TaxiPath *p = *m_taxiData->paths.begin();
        m_taxiData->paths.erase(m_taxiData->paths.begin());
        TaxiStart(p, m_taxiData->ModelId);
    }
}

void Player::EventDeath()
{
    if (m_state & UF_ATTACKING)
        EventAttackStop();
}

///  This function sends the message displaying the purple XP gain for the char
///  It assumes you will send out an UpdateObject packet at a later time.
void Player::GiveXP(uint32 xp, WoWGuid guid, bool allowbonus, bool allowGuildXP)
{
    if ( xp < 1 || m_XPoff )
        return;

    bool maxLevel = getLevel() >= GetUInt32Value(PLAYER_FIELD_MAX_LEVEL);
    if(maxLevel)
    {
        if(allowGuildXP) // Even at max level, Increase guild XP if we can
            guildmgr.GuildGainXP(this, xp);
        return;
    }

    uint32 restxp = 0; //add reststate bonus
    if(m_restData.restState == RESTSTATE_RESTED && allowbonus)
    {
        restxp = SubtractRestXP(xp);
        xp += restxp;
    }

    UpdateRestState();
    SendLogXPGain(guid, xp, restxp);

    int32 newxp = GetUInt32Value(PLAYER_XP) + xp;
    uint32 level = GetUInt32Value(UNIT_FIELD_LEVEL);
    int32 nextlevelxp = sStatSystem.GetXPToNextLevel(level);
    uint32 hpGain = 0, manaGain = 0, statGain[5] = {0, 0, 0, 0, 0};
    UnitBaseStats *stats = baseStats;
    bool levelup = false;

    while(newxp >= nextlevelxp && newxp > 0)
    {
        ++level;
        newxp -= nextlevelxp;
        nextlevelxp = sStatSystem.GetXPToNextLevel(level);
        if(UnitBaseStats *new_stats = sStatSystem.GetUnitBaseStats(getRace(), getClass(), level))
        {
            hpGain += new_stats->baseHP-stats->baseHP;
            manaGain += new_stats->baseMP ? stats->baseMP ? new_stats->baseMP->val-stats->baseMP->val : new_stats->baseMP->val : 0;
            for(uint8 i = 0; i < 5; i++)
                statGain[i] += new_stats->baseStat[i]-stats->baseStat[i];
            stats = new_stats;
        }
        levelup = true;

        SetUInt32Value(PLAYER_NEXT_LEVEL_XP, nextlevelxp);
        if(level >= GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
            break;
    }

    if(level > GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
        level = GetUInt32Value(PLAYER_FIELD_MAX_LEVEL);

    if(levelup)
    {
        setLevel(level);
        SendLevelupInfo(level, hpGain, manaGain, statGain);
    }

    // Set the update bit
    SetUInt32Value(PLAYER_XP, newxp);

    if(allowGuildXP) // Increase guild XP if we can
        guildmgr.GuildGainXP(this, xp);
}

void Player::smsg_InitialSpells()
{
    uint16 spellCount = (uint16)m_spells.size(), cooldownCount = 0;
    WorldPacket data(SMSG_INITIAL_SPELLS, 5 + (spellCount * 4) + ((m_cooldownMap[0].size() + m_cooldownMap[1].size()) * 4) );
    data << uint8(0) << uint16(spellCount); // spell count

    for (SpellSet::iterator sitr = m_spells.begin(); sitr != m_spells.end(); ++sitr)
        data << uint32(*sitr) << uint16(0x0000);

    time_t curr = UNIXTIME;
    size_t pos = data.wpos();
    data << uint16(0);        // placeholder
    for( PlayerCooldownMap::iterator itr2, itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].begin(); itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end(); )
    {
        itr2 = itr++;

        // don't keep around expired cooldowns
        if( itr2->second.ExpireTime <= curr || (itr2->second.ExpireTime - curr) < 5 )
        {
            m_cooldownMap[COOLDOWN_TYPE_SPELL].erase( itr2 );
            continue;
        }

        uint32 msTimeLeft = (itr2->second.ExpireTime - curr)*1000;
        data << uint32( itr2->second.SpellId ); // spell id
        data << uint32( itr2->second.ItemId );  // item id
        data << uint16( 0 );                    // spell category
        data << uint32( msTimeLeft );           // cooldown remaining in ms (for spell)
        data << uint32( 0 );                    // cooldown remaining in ms (for category)

        ++cooldownCount;

        sLog.outDebug("sending spell cooldown for spell %u to %u ms", itr2->first, msTimeLeft);
    }

    for( PlayerCooldownMap::iterator itr2, itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].begin(); itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end(); )
    {
        itr2 = itr++;

        // don't keep around expired cooldowns
        if( itr2->second.ExpireTime <= curr || (itr2->second.ExpireTime - curr) < 5 )
        {
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase( itr2 );
            continue;
        }

        uint32 msTimeLeft = (itr2->second.ExpireTime - curr)*1000;
        data << uint32( itr2->second.SpellId ); // spell id
        data << uint32( itr2->second.ItemId );  // item id
        data << uint16( itr2->first );          // spell category
        data << uint32( 0 );                    // cooldown remaining in ms (for spell)
        data << uint32( msTimeLeft );           // cooldown remaining in ms (for category)
        ++cooldownCount;

        sLog.outDebug("InitialSpells", "sending category cooldown for cat %u to %u ms", itr2->first, msTimeLeft);
    }

    data.put<uint16>(pos, cooldownCount);
    PushPacket(&data, true);
}

SpellEntry* Player::FindLowerRankSpell(SpellEntry* sp, int32 rankdiff)
{
    if(sp->RankNumber <= 1)
        return NULL;
    for(SpellSet::iterator itr = m_spells.begin(); itr != m_spells.end(); itr++)
    {
        if(SpellEntry* spell = dbcSpell.LookupEntry(*itr))
        {
            if(spell->NameHash == sp->NameHash)
                if((int32)sp->RankNumber + rankdiff == (int32)spell->RankNumber)
                    return spell;
        }
    }

    return NULL;
}

SpellEntry* Player::FindHighestRankProfessionSpell(uint32 professionSkillLine)
{
    SpellEntry* ret = NULL;
    for(SpellSet::iterator itr = m_spellsBySkill[professionSkillLine].begin(); itr != m_spellsBySkill[professionSkillLine].end(); itr++)
    {
        if(SpellEntry *sp = dbcSpell.LookupEntry(*itr))
        {
            if(!sp->HasEffect(SPELL_EFFECT_SKILL_STEP) && !sp->HasEffect(SPELL_EFFECT_SKILL))
                continue;
            if(ret && sp->RankNumber < ret->RankNumber)
                continue;
            ret = sp;
        }
    }

    return ret;
}

void Player::addSpell(uint32 spell_id, uint32 forget)
{
    SpellSet::iterator iter = m_spells.find(spell_id);
    if(iter != m_spells.end())
        return;
    if(spell_id == 3018 && (getClass() == DEATHKNIGHT || getClass() == WARLOCK 
        || getClass() == PRIEST || getClass() == MAGE || getClass() == PALADIN))
        return;
    SpellEntry* spell = dbcSpell.LookupEntry(spell_id);
    if(spell == NULL)
        return;

    m_spells.insert(spell_id);
    for(uint8 i = 0; i < 3; i++)
        if(uint8 effect = spell->Effect[i])
            m_spellsByEffect[effect].insert(spell_id);
    if(spell->SpellSkillLine)
        m_spellsBySkill[spell->SpellSkillLine].insert(spell_id);
    if(spell->Category)
        m_spellCategories.insert(spell->Category);

    // Check if we're logging in.
    if(!IsInWorld())
        return;

    SpellEntry* sp2 = FindLowerRankSpell(spell, -1);
    if(sp2 != NULL && sp2->Id == forget)
    {
        WorldPacket data(SMSG_SUPERCEDED_SPELL, 8);
        data << sp2->Id << spell_id;
        PushPacket(&data);
    }
    else
    {
        WorldPacket data(SMSG_LEARNED_SPELL, 6);
        data << spell_id << uint16(0);
        PushPacket(&data);
    }

    if(spell->isPassiveSpell() || spell->HasEffect(47))
        GetSpellInterface()->TriggerSpell(spell, this);
}

void Player::DestroyForPlayer( Player* target, bool anim )
{
    WorldObject::DestroyForPlayer( target, anim );
    if(GetInventory())
        GetInventory()->m_DestroyForPlayer(target);
}

bool Player::canCast(SpellEntry *m_spellInfo)
{
    if (m_spellInfo->EquippedItemClass != 0)
    {
        if( disarmed )
            return false;

        if(m_spellInfo->EquippedItemClass == 4)
        {
            Item* item = NULL;
            switch(m_spellInfo->EquippedItemSubClassMask)
            {
            case 64:
                {
                    if((item = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND)) != NULL)
                    {
                        if(item->GetProto() && item->GetProto()->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD)
                            return false;
                    }
                    else return false;
                }break;
            default:
                {
                    sLog.outError("Unknown Equipped Item Requirements: %u/%u\n", m_spellInfo->EquippedItemClass, m_spellInfo->EquippedItemSubClassMask);
                }break;
            }
        }
        else // We want a weapon, but is this even required? Some weapon requiring spells don't have any data...
        {
            if(GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
            {
                if((int32)GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)->GetProto()->Class == m_spellInfo->EquippedItemClass)
                {
                    if (m_spellInfo->EquippedItemSubClassMask != 0)
                    {
                        if (m_spellInfo->EquippedItemSubClassMask != 173555 && m_spellInfo->EquippedItemSubClassMask != 96 && m_spellInfo->EquippedItemSubClassMask != 262156)
                        {
                            if (!((1 << GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND)->GetProto()->SubClass)
                                & m_spellInfo->EquippedItemSubClassMask))
                                return false;
                        }
                    }
                }
            }
            else if(m_spellInfo->EquippedItemSubClassMask == 173555)
                return false;

            if (GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED))
            {
                if((int32)GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)->GetProto()->Class == m_spellInfo->EquippedItemClass)
                {
                    if (m_spellInfo->EquippedItemSubClassMask != 0)
                    {
                        if (m_spellInfo->EquippedItemSubClassMask != 173555 && m_spellInfo->EquippedItemSubClassMask != 96 && m_spellInfo->EquippedItemSubClassMask != 262156)
                        {
                            if (!((1 << GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED)->GetProto()->SubClass)
                                & m_spellInfo->EquippedItemSubClassMask))
                                return false;
                        }
                    }
                }
            }
            else if (m_spellInfo->EquippedItemSubClassMask == 262156)
                return false;
        }
    }
    return true;
}

void Player::RemovePendingPlayer(uint8 reason)
{
    if(m_session)
    {
        m_session->OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &reason);
        m_session->m_loggingInPlayer = NULL;
    }

    ok_to_remove = true;
    Destruct();
}

void Player::SetSelection(WoWGuid guid)
{
    if(m_curSelection != guid)
        m_spellInterface.OnChangeSelection(guid);

    m_curSelection = guid;
}

bool Player::HasSpell(uint32 spell)
{
    return m_spells.find(spell) != m_spells.end();
}

SpellEntry* Player::GetSpellWithNamehash(uint32 namehash)
{
    SpellSet::iterator itr;
    SpellEntry* sp = NULL;
    for(itr = m_spells.begin(); itr != m_spells.end(); itr++)
    {
        sp = NULL;
        if(sp = dbcSpell.LookupEntry(*itr))
        {
            if(sp->NameHash == namehash)
                return sp;
        }
        sp = NULL;
    }
    return NULL;
}

bool Player::HasHigherSpellForSkillLine(SpellEntry* sp)
{
    if(sp->NameHash == SPELL_HASH_COLD_WEATHER_FLYING)
        return false;   // Cold Weather Hackfix
    if(sp->NameHash == SPELL_HASH_FLIGHT_FORM)
        if(HasSpell(40120))
            return true; // We should only have 1 flight form.
    if(sp->NameHash == SPELL_HASH_BEAR_FORM)
        if(HasSpell(9634))
            return true; // We should only have 1 bear form.

    uint32 oskillline = sp->SpellSkillLine;
    if(oskillline == 0)
        return false;

    SkillLineEntry* sle = dbcSkillLine.LookupEntry(oskillline);
    if(sle == NULL)
        return false;

    if(sle->categoryId == SKILL_TYPE_PROFESSION)
        return false;

    SpellSet::iterator itr;
    SpellEntry* spell = NULL;
    if(HasSkillLine(oskillline))
    {
        for(itr = m_spells.begin(); itr != m_spells.end(); itr++)
        {
            if((spell = dbcSpell.LookupEntry(*itr)) != NULL)
            {
                if(spell->SpellSkillLine == oskillline)
                    if(spell->RankNumber > sp->RankNumber)
                        return true;
            }
        }
    }

    return false;
}

uint32 Player::FindSpellWithNamehash(uint32 namehash)
{
    SpellSet::iterator itr = m_spells.begin();
    if(itr != m_spells.end())
    {
        for(itr; itr != m_spells.end(); itr++)
        {
            SpellEntry* sp = dbcSpell.LookupEntry(*itr);
            if(sp && sp->NameHash == namehash)
                return (*itr);
        }
    }
    return 0;
}

uint32 Player::FindHigherRankingSpellWithNamehash(uint32 namehash, uint32 minimumrank)
{
    SpellEntry* sp = NULL;
    SpellSet::iterator itr = m_spells.begin();
    if(itr != m_spells.end())
    {
        for(itr; itr != m_spells.end(); itr++)
        {
            sp = dbcSpell.LookupEntry(*itr);
            if(sp && sp->NameHash == namehash && sp->RankNumber > minimumrank)
                return (*itr);
        }
    }
    return 0;
}

uint32 Player::FindHighestRankingSpellWithNamehash(uint32 namehash)
{
    if(!m_spells.size())
        return 0;

    SpellSet::iterator itr;
    uint32 rank = 0, spellid = 0;
    for(itr = m_spells.begin(); itr != m_spells.end(); itr++)
    {
        SpellEntry* sp = dbcSpell.LookupEntry(*itr);
        if(sp->NameHash != namehash)
            continue;
        if(sp->RankNumber == 0 && spellid != 0)
            continue;
        if(sp->RankNumber <= rank)
            continue;
        if(sp && sp->RankNumber && sp->NameHash == namehash && sp->RankNumber > rank)
        {
            rank = sp->RankNumber;
            spellid = (*itr);
        }
    }
    return spellid;
}

QuestLogEntry* Player::GetQuestLogForEntry(uint32 quest)
{
    for(uint8 i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if(m_questLog[i] == NULL)
            continue;
        if(m_questLog[i]->GetQuest()->id == quest)
            return m_questLog[i];
    }
    return NULL;
}

void Player::SetQuestLogSlot(QuestLogEntry *entry, uint32 slot)
{
    ASSERT(slot < QUEST_LOG_COUNT);
    m_questLog[slot] = entry;
}

void Player::OnPreSetInWorld()
{
    if(m_TeleportState == 1) // First world enter after loginscreen
        SoftLoadPlayer();
    Unit::OnPreSetInWorld();
}

void Player::OnPrePushToWorld()
{
    Unit::OnPrePushToWorld();
    SendInitialLogonPackets();

    // Add all our items to world before we're pushed to fill in on nearby data
    m_inventory.AddToWorld();

    // Update our stats now that we have auras and items loaded
    UpdateFieldValues();

    // Finish loading of our load data(health and power fields)
    if(m_TeleportState == 1)
    {
        if(m_deathState != DEAD && !m_loadData.empty())
        {
            while(!m_loadData.empty())
            {
                uint8 fieldType = (*m_loadData.begin()).first;
                float fieldPct = (*m_loadData.begin()).second;
                m_loadData.erase(m_loadData.begin());
                uint32 maxVal = GetUInt32Value(UNIT_FIELD_MAXHEALTH+fieldType);
                if(maxVal == 0.f)
                    continue;

                SetUInt32Value(UNIT_FIELD_HEALTH+fieldType, std::min<uint32>(maxVal, float2int32((fieldPct * ((float)maxVal))/100.f)));
            }
        } else if(!m_loadData.empty())
            while(!m_loadData.empty())
                m_loadData.erase(m_loadData.begin());
    }
}

void Player::OnPushToWorld()
{
    uint8 teleState = m_TeleportState;
    m_TeleportState = 0;

    // Worldport acknowledgement
    if(teleState == 2)
        OnWorldPortAck();
    else if(teleState = 1)
        CompleteLoading();

    // Cast our login effect spell
    static SpellEntry *loginEffect = dbcSpell.LookupEntry(836);
    if(loginEffect != NULL) GetSpellInterface()->TriggerSpell(loginEffect, this);

    m_beingPushed = false;
    sWorld.mInWorldPlayerCount++;

    Unit::OnPushToWorld();

    // Send our auras
    m_AuraInterface.SendAuraData();

    // send world states
    m_mapInstance->GetStateManager().SendWorldStates(this);

    // Push a group update
    EventGroupFullUpdate();

    TRIGGER_INSTANCE_EVENT( m_mapInstance, OnZoneChange )(this, m_zoneId, 0);
    TRIGGER_INSTANCE_EVENT( m_mapInstance, OnPlayerEnter )(this);

    /* send weather */
    sWeatherMgr.SendWeather(castPtr<Player>(this));

    m_changingMaps = false;

    if(GetTaxiState())
    {
        PopPendingUpdates(m_mapId); // Create HAS to be sent before this!
        TaxiStart(GetTaxiPath(), m_taxiData->ModelId, m_taxiData->TravelTime);
    } else m_mapInstance->PushToProcessed(this);
}

void Player::OnWorldLogin()
{

}

void Player::SendObjectUpdate(WoWGuid guid)
{
    uint32 count = 1;
    WorldObject *obj = NULL;
    WorldPacket data(SMSG_UPDATE_OBJECT, 200);
    data << uint16(GetMapId());
    data << count;
    if(guid == GetGUID())
        count = WorldObject::BuildCreateUpdateBlockForPlayer(&data, this);
    else if(IsInWorld() && (obj = GetMapInstance()->_GetObject(guid)))
        count = obj->BuildCreateUpdateBlockForPlayer(&data, this);
    else return;

    data.put<uint32>(2, count);
    // send uncompressed because it's specified
    PushPacket(&data, true);
}

void Player::RemoveFromWorld()
{
    GameObject *Arbiter;
    if(IsInWorld() && IsInDuel() && (Arbiter = GetMapInstance()->GetGameObject(GetUInt64Value(PLAYER_DUEL_ARBITER))))
        Arbiter->DuelEnd(this, this, DUEL_WINNER_RETREAT);

    if( m_CurrentCharm )
        UnPossess();

    if( GetUInt64Value(UNIT_FIELD_CHARMEDBY) != 0 && IsInWorld() )
    {
        if(Player* charmer = m_mapInstance->GetPlayer(GetUInt64Value(UNIT_FIELD_CHARMEDBY)))
            charmer->UnPossess();
    }

    m_inventory.RemoveFromWorld();

    m_resurrectHealth = 0;
    m_resurrectMana = 0;
    resurrector = 0;

    // just in case
    SetUInt32Value(UNIT_FIELD_FLAGS, GetUInt32Value(UNIT_FIELD_FLAGS) & ~UNIT_FLAG_SKINNABLE);

    //Cancel any forced teleport pending.
    if(raidgrouponlysent)
    {
        raidgrouponlysent = false;
    }

    TRIGGER_INSTANCE_EVENT( m_mapInstance, OnChangeArea )( this, 0, 0, GetAreaId() );

    m_mapInstance->GetStateManager().ClearWorldStates(this);

    if(m_SummonedObject)
    {
        m_SummonedObject->Cleanup();
        m_SummonedObject = NULL;
    }

    Unit::RemoveFromWorld();

    sWorld.mInWorldPlayerCount--;

    if( m_CurrentTransporter && !m_movementInterface.isTransportLocked() )
    {
        m_CurrentTransporter->RemovePlayer(castPtr<Player>(this));
        m_CurrentTransporter = NULL;
        GetMovementInterface()->ClearTransportData();
    }

    m_changingMaps = true;
}

void Player::ApplyItemMods(Item* item, uint8 slot, bool apply)
{
    ASSERT( item );
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;
    ASSERT(item->GetOwner() == this);

    //fast check to skip mod applying if the item doesnt meat the requirements.
    if(apply && item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) && item->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0)
        return;

    ItemPrototype* proto = item->GetProto();
    ScalingStatDistributionEntry *statDist = NULL;
    ScalingStatValuesEntry *scalingStatValues = NULL;
    int32 plrLevel = std::min<int32>(MAXIMUM_ATTAINABLE_LEVEL, getLevel()), minDamage = proto->minDamage, maxDamage = proto->maxDamage, armor = proto->Armor, bonusStat = 0, bonusStatType = 0;
    if(statDist = (proto->ScalingStatDistribution ? dbcScalingStatDistribution.LookupEntry(proto->ScalingStatDistribution) : NULL))
    {
        if(plrLevel > statDist->MaxLevel)
            plrLevel = statDist->MaxLevel;
        scalingStatValues = dbcScalingStatValues.LookupEntry(plrLevel);

        float damageModifier = 1.f;
        if(int32 dpsMod = sItemMgr.GetScalingDPSMod(proto, scalingStatValues, damageModifier))
        {
            float middle = (((float)dpsMod) * proto->Delay)/1000.f;
            minDamage = (1.f - damageModifier) * middle;
            maxDamage = (1.f + damageModifier) * middle;
        }

        if(proto->FlagsExtra & 0x0200)
        {   // Caster weapons have spell power as their bonus stat
            bonusStatType = ITEM_STAT_SPELL_POWER;
            bonusStat = scalingStatValues ? scalingStatValues->spellBonus : 0;
        }

        armor = sItemMgr.GetScalingArmor(proto, scalingStatValues);
    }

    if(minDamage && maxDamage)
    {
        ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_MINDAMAGE, ITEM_STAT_CUSTOM_DAMAGE_MIN, minDamage);
        ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_MAXDAMAGE, ITEM_STAT_CUSTOM_DAMAGE_MAX, maxDamage);
    }
    if(armor) ModifyBonuses( apply, item->GetGUID(), MOD_SLOT_ARMOR, ITEM_STAT_PHYSICAL_RESISTANCE, armor);
    if(bonusStatType) ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_BONUS_STAT, bonusStatType, bonusStat);
    // Push our delay modifier
    if(proto->Delay) ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_WEAPONDELAY, ITEM_STAT_CUSTOM_WEAPON_DELAY, proto->Delay);

    uint32 reforgeStatType = 0xFFFFFFFF, reforgeStatVal = 0;
    if(EnchantmentInstance *instance = item->GetEnchantment(REFORGE_ENCHANTMENT_SLOT))
        if(ItemReforgeEntry *entry = dbcItemReforge.LookupEntry(instance->EnchantmentId))
            reforgeStatType = entry->sourceStat;

    // Stats
    for( uint8 i = 0; i < 10; i++ )
    {
        int32 val = proto->Stats[i].Value, type = proto->Stats[i].Type;
        if(statDist && scalingStatValues)
        {
            if(statDist->Modifier[i] < 0)
                continue;

            type = statDist->StatMod[i];
            if(apply)
                val = sItemMgr.CalcStatMod(proto, scalingStatValues, statDist->Modifier[i]);
            else if(type)
                val = 1;
        }
        if(val == 0)
            continue;

        if(type == reforgeStatType)
            reforgeStatVal = val;
        ModifyBonuses( apply, item->GetGUID(), i, type, val);
    }

    for( uint8 k = 0; k < 5; k++ )
    {
        if( item->GetProto()->Spells[k].Id == 0 )
            continue;

        if( item->GetProto()->Spells[k].Trigger == ON_EQUIP )
        {
            if(SpellEntry *spells = dbcSpell.LookupEntry(item->GetProto()->Spells[k].Id))
            {
                if( spells->RequiredShapeShift )
                {
                    if(apply)
                        AddShapeShiftSpell( spells->Id );
                    else RemoveShapeShiftSpell( spells->Id );
                } else if(apply == false)
                    RemoveAura( item->GetProto()->Spells[k].Id );
                else if(Spell *spell = new Spell(this, spells))
                {
                    SpellCastTargets targets;
                    targets.m_unitTarget = GetGUID();
                    spell->castedItemId = item->GetEntry();
                    spell->prepare( &targets, true );
                }
            }
        }
        else if( item->GetProto()->Spells[k].Trigger == CHANCE_ON_HIT )
        {
            // Todo:PROC
        }
    }

    if(slot >= EQUIPMENT_SLOT_END)
        return;
    uint16 visibleSlot = PLAYER_VISIBLE_ITEM + (slot * PLAYER_VISIBLE_ITEM_LENGTH);
    SetUInt32Value( visibleSlot, apply ? item->GetEntry() : 0 );
    SetUInt32Value(UNIT_FIELD_MAXITEMLEVEL, GetTotalItemLevel());

    // E N C H A N T S B O I S
    for(uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; i++)
    {
        // Apply visual enchant
        if( slot < EQUIPMENT_SLOT_END && i <= TEMP_ENCHANTMENT_SLOT )
            SetUInt16Value( visibleSlot + 1, i, 0 );

        EnchantmentInstance *instance = item->GetEnchantment(i);
        if(instance == NULL)
            continue;

        uint32 randomSuffix = instance->RandomSuffix;
        // Set our visible enchantment data
        if( slot < EQUIPMENT_SLOT_END && i <= TEMP_ENCHANTMENT_SLOT )
            SetUInt16Value( visibleSlot + 1, i, instance->EnchantmentId );
        // Precharge our reforge with our stat value
        if(i == REFORGE_ENCHANTMENT_SLOT)
            randomSuffix = reforgeStatVal;
        // Handle our different enchantments, transmog overrides only visible slot
        if(i == TRANSMOG_ENCHANTMENT_SLOT)
            SetUInt32Value( visibleSlot, apply ? instance->EnchantmentId : 0 );
        else ModifyBonuses(apply, item->GetGUID(), MOD_SLOT_PERM_ENCHANT + (i*4), ITEM_STAT_MOD_ENCHANTID, instance->EnchantmentId, randomSuffix, item->GetItemPropertySeed());
    }
}

void Player::BuildPlayerRepop()
{
    SetUInt32Value(UNIT_FIELD_HEALTH, 1 );

    //8326 --for all races but ne, 20584--ne
    if(SpellEntry *inf = dbcSpell.LookupEntry(Wispform ? 20584 : 8326))
        GetSpellInterface()->TriggerSpell(inf, this);

    StopMirrorTimer(0);
    StopMirrorTimer(1);
    StopMirrorTimer(2);

    SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE);

    m_movementInterface.OnRepop();
}

Corpse* Player::RepopRequestedPlayer()
{
    if( m_CurrentTransporter != NULL )
    {
        m_CurrentTransporter->RemovePlayer( castPtr<Player>(this) );
        m_CurrentTransporter = NULL;
        m_movementInterface.ClearTransportData();

        ResurrectPlayer();
        RepopAtGraveyard( GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId() );
        return NULL;
    }

    // Remove corpse flag, so we have spirit state
    clearStateFlag(UF_CORPSE);

    // Update visibility, that way people wont see running corpses :P
    UpdateVisibility();

    // If we're in battleground, remove the skinnable flag.. has bad effects heheh
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE );

    Corpse* ret = myCorpse ? myCorpse : CreateCorpse();
    BuildPlayerRepop();

    if( ret && m_session )
    {
        /* Send Spirit Healer Location */
        WorldPacket data( SMSG_DEATH_RELEASE_LOC, 16 );
        data << m_mapId;
        data.appendvector(ret->GetPosition(), false);
        PushPacket( &data );

        /* Corpse reclaim delay */
        data.Initialize( SMSG_CORPSE_RECLAIM_DELAY, 4 );
        data << uint32(ReclaimCount*15 * 1000);
        PushPacket( &data );
    }

    if( myCorpse != NULL )
        myCorpse->ResetDeathClock();

    RepopAtGraveyard( GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId() );
    return ret;
}

void Player::ResurrectPlayer(Unit* pResurrector /* = NULL */)
{
    if (PreventRes)
        return;

    bool previouslyDead = isDead();
    if( m_resurrectHealth ) SetUInt32Value(UNIT_FIELD_HEALTH, std::min( m_resurrectHealth, GetUInt32Value(UNIT_FIELD_MAXHEALTH) ) );
    else SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    if( m_resurrectMana ) SetPower(POWER_TYPE_MANA, std::min( m_resurrectMana, GetMaxPower(POWER_TYPE_MANA) ) );
    else SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));

    m_resurrectHealth = m_resurrectMana = 0;

    SpawnCorpseBones();
    RemoveAura(Wispform ? 20584 : 8326);

    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE);
    SetDeathState(ALIVE);
    UpdateVisibility();
    m_movementInterface.OnRessurect();

    if(pResurrector == this || previouslyDead)
        return;

    if(IsInWorld() && pResurrector != NULL && pResurrector->IsInWorld())
    {
        //make sure corpse and resurrector are on the same map.
        if( GetMapId() == pResurrector->GetMapId() )
        {
            if(pResurrector->IsPlayer())
            {
                if(m_resurrectLoction.DistanceSq(0.f, 0.f, 0.f) == 0.f)
                    SafeTeleport(pResurrector->GetMapId(), pResurrector->GetInstanceID(), pResurrector->GetPosition());
                else SafeTeleport(GetMapId(), GetInstanceID(), m_resurrectLoction);
            }
            m_resurrectLoction.ChangeCoords(0.0f, 0.0f, 0.0f);
        }
    }
}

void Player::SetDeathState(DeathState s)
{
    Unit::SetDeathState(s == JUST_DIED ? DEAD : s);
    if(s == JUST_DIED)
    {
        // stuff
    }
}

void Player::KillPlayer()
{
    SetDeathState(JUST_DIED);

    EventDeath();

    PushData(SMSG_CANCEL_COMBAT);
    PushData(SMSG_CANCEL_AUTO_REPEAT);

    m_movementInterface.OnDeath();

    StopMirrorTimer(0);
    StopMirrorTimer(1);
    StopMirrorTimer(2);

    SetUInt32Value( UNIT_DYNAMIC_FLAGS, 0x00 );
    SetPower(POWER_TYPE_RAGE, 0);
    SetPower(POWER_TYPE_RUNIC, 0);
}

Corpse* Player::CreateCorpse()
{
    uint32 _uf, _pb, _pb2, _cfb1, _cfb2;

    objmgr.DelinkPlayerCorpses(castPtr<Player>(this));
    Corpse *pCorpse = objmgr.CreateCorpse();
    pCorpse->SetInstanceID(GetInstanceID());
    pCorpse->Create(castPtr<Player>(this), GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());

    _uf = GetUInt32Value(UNIT_FIELD_BYTES_0);
    _pb = GetUInt32Value(PLAYER_BYTES);
    _pb2 = GetUInt32Value(PLAYER_BYTES_2);

    uint8 race     = (uint8)(_uf);
    uint8 skin     = (uint8)(_pb);
    uint8 face     = (uint8)(_pb >> 8);
    uint8 hairstyle  = (uint8)(_pb >> 16);
    uint8 haircolor  = (uint8)(_pb >> 24);
    uint8 facialhair = (uint8)(_pb2);

    _cfb1 = ((0x00) | (race << 8) | (0x00 << 16) | (skin << 24));
    _cfb2 = ((face) | (hairstyle << 8) | (haircolor << 16) | (facialhair << 24));

    pCorpse->SetUInt32Value( CORPSE_FIELD_BYTES_1, _cfb1 );
    pCorpse->SetUInt32Value( CORPSE_FIELD_BYTES_2, _cfb2 );
    pCorpse->SetUInt32Value( CORPSE_FIELD_FLAGS, 4 );
    pCorpse->SetUInt32Value( CORPSE_FIELD_DISPLAY_ID, GetUInt32Value(UNIT_FIELD_DISPLAYID) );

    uint32 _cfi = 0, iDisplayID = 0;
    uint16 iIventoryType = 0;
    for (int8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if(Item *pItem = GetInventory()->GetInventoryItem(i))
        {
            iDisplayID = pItem->GetProto()->DisplayInfoID;
            iIventoryType = (uint16)pItem->GetProto()->InventoryType;

            _cfi =  (uint16(iDisplayID)) | (iIventoryType)<< 24;
            pCorpse->SetUInt32Value(CORPSE_FIELD_ITEM + i,_cfi);
        }
    }

    // are we going to bones straight away?
    if(false)
    {
        pCorpse->SetUInt32Value(CORPSE_FIELD_FLAGS, 5);
        pCorpse->SetUInt64Value(CORPSE_FIELD_OWNER, 0); // remove corpse owner association
        //remove item association
        for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
        {
            if(pCorpse->GetUInt32Value(CORPSE_FIELD_ITEM + i))
                pCorpse->SetUInt32Value(CORPSE_FIELD_ITEM + i, 0);
        }
        pCorpse->SetCorpseState(CORPSE_STATE_BONES);
    }
    else
    {
        //save corpse in db for future use
        if(m_mapInstance && m_mapInstance->IsContinent())
            pCorpse->SaveToDB();

        myCorpse = pCorpse;
    }

    // spawn
    if( m_mapInstance == NULL )
        sWorldMgr.PushToWorldQueue(pCorpse);
    else pCorpse->PushToWorld(m_mapInstance);

    return pCorpse;
}

void Player::SpawnCorpseBones()
{
    myCorpse = NULL;
    if(Corpse* pCorpse = objmgr.GetCorpseByOwner(GetLowGUID()))
    {
        if (pCorpse->IsInWorld() && pCorpse->GetCorpseState() == CORPSE_STATE_BODY)
        {
            if(pCorpse->GetInstanceID() != GetInstanceID())
            {

            } else pCorpse->SpawnBones();
        }
    }
}

void Player::DeathDurabilityLoss(double percent)
{
    PushData(SMSG_DURABILITY_DAMAGE_DEATH);
    uint32 pDurability;
    uint32 pMaxDurability;
    int32 pNewDurability;
    Item* pItem;

    for (int8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if((pItem = GetInventory()->GetInventoryItem(i)))
        {
            pMaxDurability = pItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
            pDurability =  pItem->GetUInt32Value(ITEM_FIELD_DURABILITY);
            if(pDurability)
            {
                pNewDurability = (uint32)(pMaxDurability*percent);
                pNewDurability = (pDurability - pNewDurability);
                if(pNewDurability < 0)
                    pNewDurability = 0;

                if(pNewDurability <= 0)
                    ApplyItemMods(pItem, i, false);

                pItem->SetUInt32Value(ITEM_FIELD_DURABILITY,(uint32)pNewDurability);
            }
        }
    }
}

void Player::RepopAtGraveyard(float ox, float oy, float oz, uint32 mapid)
{
    bool first = true;
    //float closestX = 0, closestY = 0, closestZ = 0, closestO = 0;
    StorageContainerIterator<GraveyardTeleport> * itr;

    LocationVector src(ox, oy, oz);
    LocationVector dest(0, 0, 0, 0);
    LocationVector temp;
    float closest_dist = 999999.0f;
    float dist;

    itr = GraveyardStorage.MakeIterator();
    while(!itr->AtEnd())
    {
        GraveyardTeleport *pGrave = itr->Get();
        if(pGrave->MapId == mapid && (pGrave->FactionID == GetTeam() || pGrave->FactionID == 3))
        {
            temp.ChangeCoords(pGrave->X, pGrave->Y, pGrave->Z, pGrave->O);
            dist = src.DistanceSq(temp);
            if( first || dist < closest_dist )
            {
                first = false;
                closest_dist = dist;
                dest = temp;
            }
        }

        if(!itr->Inc())
            break;
    }
    itr->Destruct();

    if(dest.x != 0 && dest.y != 0 && dest.z != 0)
        SafeTeleport(mapid, 0, dest);
}

void Player::JoinedChannel(Channel *c)
{
    if( c != NULL )
    {
        m_channels.insert(c->m_channelId);
        if(c->pDBC)
        {
            std::map<uint32, Channel*>::iterator itr = m_channelsbyDBCID.find(c->pDBC->id);
            if(itr != m_channelsbyDBCID.end())
                m_channelsbyDBCID.erase(c->pDBC->id);
            m_channelsbyDBCID.insert(std::make_pair(c->pDBC->id, c));
        }
    }
}

void Player::LeftChannel(Channel *c)
{
    if( c != NULL )
    {
        m_channels.erase(c->m_channelId);
        if(c->pDBC)
            m_channelsbyDBCID.erase(c->pDBC->id);
    }
}

void Player::CleanupChannels()
{
    std::set<uint32>::iterator i;
    Channel * c;
    uint32 cid;
    for(i = m_channels.begin(); i != m_channels.end();)
    {
        cid = *i;
        ++i;

        c = channelmgr.GetChannel(cid);
        if( c != NULL )
            c->Part(castPtr<Player>(this), false);
    }
    m_channelsbyDBCID.clear();
}

//Groupcheck
bool Player::IsGroupMember(Player* plyr)
{
    if(m_playerInfo->m_Group != NULL)
        return m_playerInfo->m_Group->HasMember(plyr->m_playerInfo);
    return false;
}

int32 Player::GetOpenQuestSlot()
{
    for(uint32 i = 0; i < QUEST_LOG_COUNT; i++)
        if (m_questLog[i] == NULL)
            return i;

    return -1;
}

uint32 Player::GetQuestStatusForQuest(uint32 questid, uint8 type, bool skiplevelcheck)
{
    return sQuestMgr.CalcQuestStatus(this, GetQuestLogForEntry(questid)->GetQuest(), type, skiplevelcheck);
}

void Player::AddToCompletedQuests(uint32 quest_id, bool quickSave)
{
    Quest *qst = sQuestMgr.GetQuestPointer(quest_id);
    if(qst == NULL)
        return;

    if(qst->qst_is_repeatable == UNREPEATABLE_QUEST)
    {
        uint16 offset = ((uint16)quest_id / 64);
        uint64 val = (((uint64)1) << ((uint64)(quest_id % 64)));
        std::map<uint16, uint64>::iterator itr;
        if((itr = m_completedQuests.find(offset)) == m_completedQuests.end())
            m_completedQuests.insert(std::make_pair(offset, val));
        else
        {
            // Add the target value to our mask
            val |= itr->second;
            // Update the target mask with new value
            itr->second = val;
        }

        // Push a character quest completion mask update
        if(quickSave) CharacterDatabase.Execute("UPDATE character_quests_completion_masks SET mask = '%llu' WHERE index = '%u'", val, offset);
    }
    else if(qst->qst_is_repeatable == REPEATABLE_QUEST)
        m_completedRepeatableQuests.insert(std::make_pair(quest_id, UNIXTIME));
    else if(qst->qst_is_repeatable == REPEATABLE_DAILY)
    {
        if(m_completedDailyQuests.size() >= 25)
            return;

        accessLock.Acquire();
        SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1 + uint32(m_completedDailyQuests.size()), quest_id);
        m_completedDailyQuests.insert(std::make_pair(quest_id, UNIXTIME));
        accessLock.Release();
    }
    else if(qst->qst_is_repeatable == REPEATABLE_WEEKLY)
    {
        if(m_completedWeeklyQuests.size() >= 25)
            return;

        accessLock.Acquire();
        m_completedWeeklyQuests.insert(std::make_pair(quest_id, UNIXTIME));
        accessLock.Release();
    }
}

bool Player::HasCompletedQuest(Quest *qst)
{
    if(qst->qst_is_repeatable == REPEATABLE_WEEKLY)
        return m_completedWeeklyQuests.find(qst->id) != m_completedWeeklyQuests.end();
    else if(qst->qst_is_repeatable == REPEATABLE_DAILY)
        return HasFinishedDailyQuest(qst->id);
    else if(qst->qst_is_repeatable == UNREPEATABLE_QUEST)
        return HasFinishedQuest(qst->id);
    return false;
}
bool Player::HasFinishedQuest(uint32 quest_id)
{
    uint16 offset = ((uint16)quest_id / 64);
    if(m_completedQuests.find(offset) == m_completedQuests.end())
        return false;
    return (m_completedQuests[offset] & (((uint64)1) << ((uint64)(quest_id % 64)))) > 0;
}

void Player::ResetDailyQuests()
{
    accessLock.Acquire();
    m_completedDailyQuests.clear();

    for(uint32 i = 0; i < 25; i++)
        SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1 + i, 0);
    accessLock.Release();
}

bool Player::HasFinishedDailyQuest(uint32 quest_id)
{
    return m_completedDailyQuests.find(quest_id) != m_completedDailyQuests.end();
}

float Player::GetBaseDodge()
{
    // 4.2.0: these classes no longer receive dodge from agility and have 5% base
    if (getClass() == WARRIOR || getClass() == PALADIN || getClass() == DEATHKNIGHT)
        return 5.f;

    gtFloat *dodgeRatio = dbcMeleeCrit.LookupEntry((getClass()-1)*100+(std::min<uint32>(MAXIMUM_ATTAINABLE_LEVEL, getLevel()) - 1));
    if(dodgeRatio == NULL)
        return baseDodge[getClass()]+5.f; // Shouldn't happen
    return (baseDodge[getClass()]/100.f) + (baseStats->baseStat[STAT_AGILITY] * (dodgeRatio->val * 100.f) * crit_to_dodge[getClass()]);
}

float Player::GetBaseParry()
{
    if(getClass() == PRIEST || getClass() >= MAGE)
        return 0.f;
    if(!HasSpellWithEffect(SPELL_EFFECT_PARRY))
        return 0.f;
    return 5.f + (baseStats->baseStat[STAT_STRENGTH] * 0.27f * GetRatioForCombatRating(3));
}

float Player::GetBaseBlock()
{
    if(!HasSpellWithEffect(SPELL_EFFECT_BLOCK))
        return 0.f;
    Item *item = NULL;
    if(item = m_inventory.GetInventoryItem(EQUIPMENT_SLOT_OFFHAND))
    {
        if(item->GetProto()->Class != ITEM_CLASS_ARMOR && item->GetProto()->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD)
            return 0.f;
    } else return 0.f;

    float val = 5.f;
    if(AuraInterface::modifierMap *increaseBlockMod = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_BLOCK_PERCENT))
        for(AuraInterface::modifierMap::iterator itr = increaseBlockMod->begin(); itr != increaseBlockMod->end(); itr++)
            val += itr->second->m_amount;
    return val;
}

float Player::CalculateCritFromAgilForClassAndLevel(uint32 _class, uint32 _level)
{
    gtFloat* baseCrit = dbcMeleeCritBase.LookupEntry(_class-1);
    gtFloat* CritPerAgi = dbcMeleeCrit.LookupEntry((_class-1)*100+(_level - 1));
    if(baseCrit == NULL || CritPerAgi == NULL)
        return 0.0f;
    uint32 agility = GetUInt32Value(UNIT_FIELD_AGILITY);
    float base = 100*baseCrit->val, ratio = 100*CritPerAgi->val;
    if(ratio < 0.0f) ratio = 0.00001f;
    return (base + float(agility*ratio));
}

void Player::HandleRestedCalculations(bool rest_on)
{
    LocationVector loc = GetPosition();
    if(rest_on == true && m_restData.isResting == false)
        ApplyPlayerRestState(true);
    else if(rest_on == false)
    {
        if(m_restData.areaTriggerId && HasAreaFlag(OBJECT_AREA_FLAG_INDOORS))
        {
            bool result = false;
            if(HasAreaFlag(OBJECT_AREA_FLAG_CONTESTED) || (HasAreaFlag(OBJECT_AREA_FLAG_HORDE_ZONE) && GetTeam() == TEAM_HORDE) || (HasAreaFlag(OBJECT_AREA_FLAG_ALLIANCE_ZONE) && GetTeam() == TEAM_ALLIANCE))
            {
                if(AreaTriggerEntry* ATE = dbcAreaTrigger.LookupEntry(m_restData.areaTriggerId))
                {
                    float delta = 3.2f;
                    if(ATE->radius) // If there is a radius, check our distance with the middle.
                    {   // Only add delta leeway to a single part of the squared value
                        delta = (delta + ATE->radius) * ATE->radius;
                        float dist = GetDistance2dSq(ATE->base_x, ATE->base_y), halfRad = ATE->radius/2.f;
                        result = (dist < delta && ((ATE->base_z-halfRad) < GetPositionZ() && GetPositionZ() < (ATE->base_z+halfRad)));
                    } else result = IsInBox(ATE->base_x, ATE->base_y, ATE->base_z, ATE->box_length, ATE->box_width, ATE->box_height, ATE->box_yaw, delta);
                } else m_restData.areaTriggerId = 0;
            }

            ApplyPlayerRestState(result);
        } else ApplyPlayerRestState(false);
    }
}

uint32 Player::SubtractRestXP(uint32 &amount)
{
    if(GetUInt32Value(UNIT_FIELD_LEVEL) >= GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))      // Save CPU, don't waste time on this if you've reached max_level
        return (amount = 0);

    m_restData.restAmount -= std::min(m_restData.restAmount, amount);
    UpdateRestState(); // Update clients interface with new values.
    return amount;
}

void Player::AddCalculatedRestXP(uint32 seconds)
{
    // When a player rests in a city or at an inn they will gain rest bonus at a very slow rate.
    // Eight hours of rest will be needed for a player to gain one "bubble" of rest bonus.
    // At any given time, players will be able to accumulate a maximum of 30 "bubbles" worth of rest bonus which
    // translates into approximately 1.5 levels worth of rested play (before your character returns to normal rest state).
    // Thanks to the comforts of a warm bed and a hearty meal, players who rest or log out at an Inn will
    // accumulate rest credit four times faster than players logged off outside of an Inn or City.
    // Players who log out anywhere else in the world will earn rest credit four times slower.
    // http://www.worldofwarcraft.com/info/basics/resting.html


    // Define xp for a full bar ( = 20 bubbles)
    uint32 xp_to_lvl = sStatSystem.GetXPToNextLevel(getLevel());

    // get RestXP multiplier from config.
    float bubblerate = sWorld.getRate(RATE_RESTXP);

    // One bubble (5% of xp_to_level) for every 8 hours logged out.
    // if multiplier RestXP (from Ronin.config) is f.e 2, you only need 4hrs/bubble.
    uint32 rested_xp = uint32(0.05f * xp_to_lvl * ( seconds / (3600 * ( 8 / bubblerate))));

    // if we are at a resting area rest_XP goes 4 times faster (making it 1 bubble every 2 hrs)
    if (m_restData.isResting) rested_xp *= 4;

    // Add result to accumulated rested XP
    m_restData.restAmount += uint32(rested_xp);

    // and set limit to be max 1.5 * 20 bubbles * multiplier (1.5 * xp_to_level * multiplier)
    if (m_restData.restAmount > xp_to_lvl + (uint32)((float)( xp_to_lvl*1.5f ) * bubblerate ))
        m_restData.restAmount = xp_to_lvl + (uint32)((float)( xp_to_lvl*1.5f ) * bubblerate );

    sLog.Debug("REST","Add %d rest XP to a total of %d, RestState %d", rested_xp, m_restData.restAmount, m_restData.isResting);

    // Update clients interface with new values.
    UpdateRestState();
}

void Player::UpdateRestState()
{
    if(m_restData.restAmount && GetUInt32Value(UNIT_FIELD_LEVEL) < GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
        m_restData.restState = RESTSTATE_RESTED;
    else m_restData.restState = RESTSTATE_NORMAL;

    // Update RestState 100%/200%
    SetByte(PLAYER_BYTES_2, 3, m_restData.restState);

    //update needle (weird, works at 1/2 rate)
    SetUInt32Value(PLAYER_REST_STATE_EXPERIENCE, (m_restData.restAmount ? 1+(m_restData.restAmount >> 1) : 0));
}

void Player::SetRestedAreaTrigger(uint32 triggerId)
{
    m_restData.areaTriggerId = triggerId;
}

void Player::ApplyPlayerRestState(bool apply)
{
    if(m_restData.isResting == apply)
        return;

    if((m_restData.isResting = apply) == true)
    {
        m_restData.restState = RESTSTATE_RESTED;
        SetFlag(PLAYER_FLAGS, PLAYER_FLAG_RESTING); //put zzz icon
    } else RemoveFlag(PLAYER_FLAGS,PLAYER_FLAG_RESTING);   //remove zzz icon
    UpdateRestState();
}

#define CORPSE_VIEW_DISTANCE 1600 // 40*40

bool Player::CanSee(WorldObject* obj) // * Invisibility & Stealth Detection - Partha *
{
    if (obj == castPtr<Player>(this))
       return true;

    uint32 object_type = obj->GetTypeId();
    if(isSpirit()) // we are dead and we have released our spirit
    {
        if(object_type == TYPEID_PLAYER)
        {
            Player* pObj = castPtr<Player>(obj);

            if(myCorpse && myCorpse->GetDistanceSq(obj) > CORPSE_VIEW_DISTANCE)
                if(pObj->IsPlayer() && pObj->isSpirit())
                    return true; // we can see all players within range of our corpse except invisible GMs

            if(myCorpse && myCorpse->GetDistanceSq(obj) <= CORPSE_VIEW_DISTANCE)
                return true; // we can see all players within range of our corpse except invisible GMs

            if(m_deathVision) // if we have arena death-vision we can see all players except invisible GMs
                return true;

            return pObj->isSpirit(); // we can only see players that are spirits
        }

        if(myCorpse)
        {
            if(myCorpse == obj)
                return true;

            if(myCorpse->GetDistanceSq(obj) <= CORPSE_VIEW_DISTANCE)
                return true; // we can see everything within range of our corpse
        }

        if(m_deathVision) // if we have arena death-vision we can see everything
        {
            /*if(obj->IsPlayer() && castPtr<Player>(obj)->InStealth())
                return false;*/

            return true;
        }

        if(object_type == TYPEID_UNIT)
        {
            Unit* uObj = castPtr<Unit>(obj);

            return uObj->IsSpiritHealer(); // we can't see any NPCs except spirit-healers
        }

        return false;
    }
    //------------------------------------------------------------------

    // mage invisibility - ugh.
    if( IsPlayer() && m_mageInvisibility )
    {
        if( object_type == TYPEID_PLAYER )
            return castPtr<Player>(obj)->m_mageInvisibility;

        if( object_type == TYPEID_UNIT )
            return false;

        return true;
    }

    switch(object_type) // we are alive or we haven't released our spirit yet
    {
        case TYPEID_PLAYER:
            {
                Player* pObj = castPtr<Player>(obj);
                if( pObj->m_mageInvisibility )
                    return false;

                if(pObj->IsInvisible()) // Invisibility - Detection of Players
                {
                    if(pObj->isSpirit())
                        return hasGMTag(); // only GM can see players that are spirits

                    // can see invisible group members except when dueling them
                    if(GetGroup() && pObj->GetGroup() == GetGroup() && !IsDuelTarget(pObj))
                        return true;

                    if(m_invisDetect[INVIS_FLAG_NORMAL] < 1) // can't see invisible without proper detection
                        return hasGMTag(); // GM can see invisible players
                }

                return true;
            }
        //------------------------------------------------------------------

        case TYPEID_UNIT:
            {
                Unit* uObj = castPtr<Unit>(obj);
                if(uObj->IsSpiritHealer()) // can't see spirit-healers when alive
                    return false;

                // Invisibility - Detection of Units
                if(uObj->IsInvisible() && m_invisDetect[uObj->m_invisFlag] < 1) // can't see invisible without proper detection
                    return hasGMTag(); // GM can see invisible units

                Creature *cObj = NULL;
                if(obj->IsCreature() && (cObj = castPtr<Creature>(obj)))
                {
                    if(cObj->isWorldTrigger())
                        return false;//hasGMTag();
                }
                return true;
            }
        //------------------------------------------------------------------

        case TYPEID_GAMEOBJECT:
            {
                GameObject* gObj = castPtr<GameObject>(obj);

                if(gObj->invisible) // Invisibility - Detection of GameObjects
                {
                    uint64 owner = gObj->GetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY);

                    if(GetGUID() == owner) // the owner of an object can always see it
                        return true;

                    if(GetGroup())
                    {
                        Player * gplr = GetMapInstance()->GetPlayer((uint32)owner);
                        if(gplr != NULL && GetGroup()->HasMember(gplr))
                            return true;
                    }

                    if(m_invisDetect[gObj->invisibilityFlag] < 1) // can't see invisible without proper detection
                        return hasGMTag(); // GM can see invisible objects
                }

                return true;
            }
        //------------------------------------------------------------------

        default:
            return true;
    }
}

void Player::ClearInRangeObjects()
{
    m_visibleObjects.clear();
}

void Player::ProcessVisibleQuestGiverStatus()
{
    uint32 count = 0;
    WorldPacket data(SMSG_QUESTGIVER_STATUS_MULTIPLE, 1000);
    data << count;
    for(std::set<WoWGuid>::iterator itr = m_visibleObjects.begin(); itr != m_visibleObjects.end(); itr++)
    {
        if(WorldObject *curObj = GetInRangeObject(*itr))
        {
            if(!sQuestMgr.hasQuests(curObj))
                continue;

            uint32 status = sQuestMgr.CalcStatus(curObj, this);
            data << curObj->GetGUID();
            data << status;
            count++;
        }
    }
    data.put<uint32>(0, count);
    PushPacket(&data);
}

void Player::SetDrunk(uint16 value, uint32 itemId)
{
    uint32 oldDrunkenState = GetDrunkenstateByValue(m_drunk);
    m_invisDetect[INVIS_FLAG_DRUNK] = int32(value - m_drunk) / 256;
    m_drunk = value;
    SetUInt16Value(PLAYER_BYTES_3, 0, (m_drunk & 0xFFFE));
    uint32 newDrunkenState = GetDrunkenstateByValue(m_drunk);
    UpdateVisibility();
    if (newDrunkenState == oldDrunkenState)
        return;

    WorldPacket data(SMSG_CROSSED_INEBRIATION_THRESHOLD, 16);
    data << GetGUID();
    data << newDrunkenState;
    data << itemId;
    PushPacket(&data);
}

void Player::EventHandleSobering()
{
    m_drunkTimer = 0;
    SetDrunk((m_drunk <= 256) ? 0 : (m_drunk - 256));
}

bool Player::HasQuestForItem(uint32 itemid)
{
    Quest *qst;
    for(uint32 i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if( qst = (m_questLog[i] ? m_questLog[i]->GetQuest() : NULL) )
        {
            // No item_quest association found, check the quest requirements
            if( !qst->count_required_item )
                continue;

            for( uint8 j = 0; j < 6; ++j )
                if( qst->required_item[j] == itemid && ( GetInventory()->GetItemCount( itemid ) < qst->required_itemcount[j] ) )
                    return true;
        }
    }
    return false;
}

bool Player::CanNeedItem(ItemPrototype* proto)
{
    if(proto->Class == ITEM_CLASS_WEAPON && !(GetWeaponProficiency() & (uint32(1) << proto->SubClass)))
        return false;
    if(proto->Class == ITEM_CLASS_ARMOR && !(GetArmorProficiency() & (uint32(1) << proto->SubClass)))
        return false;
    if(proto->AllowableClass && !(proto->AllowableClass & getClassMask()))
        return false;
    if(proto->AllowableRace & !(proto->AllowableRace & getRaceMask()))
        return false;
    // Crow: Should be all we need for now.
    return true;
}

/*Loot type MUST be
1-corpse, go
2-skinning/herbalism/minning
3-Fishing
*/
void Player::SendLoot(WoWGuid guid, uint32 mapid, uint8 loot_type)
{
    Group * m_Group = m_playerInfo->m_Group;
    if(!IsInWorld())
        return;

    Object* lootEnt;
    // handle items
    if(GUID_HIPART(guid) == HIGHGUID_TYPE_ITEM)
        lootEnt = m_inventory.GetItemByGUID(guid);
    else lootEnt = m_mapInstance->_GetObject(guid);
    if( lootEnt == NULL )
        return;
    lootEnt->LootLock();
    ObjectLoot *loot = lootEnt->GetLoot();

    int8 loot_method = -1;

    //lootsteal fix
    if( lootEnt->GetTypeId() == TYPEID_UNIT )
    {
        Creature* LootOwner = castPtr<Creature>( lootEnt );
        if( uint32 GroupId = LootOwner->m_taggedGroup.getLow() )
        {
            if(m_Group == NULL || GroupId != m_Group->GetID())
            {
                lootEnt->LootUnlock();
                return;
            }
        }
        loot_method = LootOwner->m_lootMethod;
    }

    if( loot_method < 0 )
    {
        // not set
        if( m_Group != NULL )
            loot_method = m_Group->GetMethod();
        else loot_method = PARTY_LOOT_FFA;
    }

    // add to looter set
    loot->looters.insert(GetLowGUID());
    m_lootGuid = guid;

    WorldPacket data(SMSG_LOOT_RESPONSE, 32), data2(32);
    data << uint64(guid);
    data << uint8(loot_type);//loot_type;
    data << uint32(loot->gold);
    data << uint8(0);//loot size reserve
    data << uint8(0);//unk

    uint32 count = 0;
    std::vector<__LootItem>::iterator iter = loot->items.begin();
    for(uint8 x = 0; iter != loot->items.end(); iter++, x++)
    {
        if(x == 0xFF)
            break;
        ItemPrototype* itemProto = iter->proto;
        if (!itemProto)
            continue;
        if((itemProto->Flags & DBC_ITEMFLAG_PARTY_LOOT) == 0)
        {
            if(iter->has_looted.size())
                continue;
        } else if (iter->has_looted.find(GetGUID()) != iter->has_looted.end())
            continue;

        //quest items check. type 4/5
        //quest items that dont start quests.
        if((itemProto->Bonding == ITEM_BIND_QUEST) && !(itemProto->QuestId) && !HasQuestForItem(itemProto->ItemId))
            continue;

        if((itemProto->Bonding == ITEM_BIND_QUEST2) && !(itemProto->QuestId) && !HasQuestForItem(itemProto->ItemId))
            continue;

        //quest items that start quests need special check to avoid drops all the time.
        if((itemProto->Bonding == ITEM_BIND_QUEST) && (itemProto->QuestId) && GetQuestLogForEntry(itemProto->QuestId))
            continue;

        if((itemProto->Bonding == ITEM_BIND_QUEST2) && (itemProto->QuestId) && GetQuestLogForEntry(itemProto->QuestId))
            continue;

        if((itemProto->Bonding == ITEM_BIND_QUEST) && (itemProto->QuestId) && HasFinishedQuest(itemProto->QuestId))
            continue;

        if((itemProto->Bonding == ITEM_BIND_QUEST2) && (itemProto->QuestId) && HasFinishedQuest(itemProto->QuestId))
            continue;

        //check for starting item quests that need questlines.
        if(itemProto->QuestId && itemProto->Bonding != ITEM_BIND_QUEST && itemProto->Bonding != ITEM_BIND_QUEST2)
        {
            bool HasRequiredQuests = true;
            Quest * pQuest = sQuestMgr.GetQuestPointer(itemProto->QuestId);
            if(pQuest == NULL)
                continue;

            //check if its a questline.
            uint32 RequiredQuest = 0;
            for(uint32 i = 0; i < pQuest->count_requiredquests; i++)
            {
                if(!HasRequiredQuests)
                    continue;

                if((RequiredQuest = pQuest->required_quests[i]))
                    if(HasFinishedQuest(RequiredQuest) || !GetQuestLogForEntry(RequiredQuest))
                        HasRequiredQuests = false;
            }

            if(!HasRequiredQuests)
                continue;
        }

        uint8 slottype = 0;
        if(lootEnt->IsObject() && m_Group != NULL && loot_type < 2)
        {
            switch(loot_method)
            {
            case PARTY_LOOT_MASTER:
                slottype = 2;
                break;
            case PARTY_LOOT_GROUP:
            case PARTY_LOOT_RR:
            case PARTY_LOOT_NBG:
                slottype = 1;
                break;
            default:
                slottype = 0;
                break;
            }

            // only quality items are distributed
            if(itemProto->Quality < m_Group->GetThreshold())
                slottype = 0;

            /* if all people passed anyone can loot it? :P */
            if(iter->all_passed || itemProto->Flags & DBC_ITEMFLAG_PARTY_LOOT)
                slottype = 0; // All players passed on the loot
        }

        data << uint8(x);
        data << uint32(itemProto->ItemId);
        data << uint32(iter->StackSize);//nr of items of this type
        data << uint32(itemProto->DisplayInfoID);
        data << uint32(iter->randProp);
        data << uint32(iter->randSeed);
        data << slottype;   // "still being rolled for" flag
        if(slottype == 1)
        {
            WorldObject *lootObj = castPtr<WorldObject>(lootEnt);
            if(iter->roll == NULL && !iter->all_passed)
            {
                iter->roll = new LootRoll();
                iter->roll->Init(60000, (m_Group != NULL ? m_Group->MemberCount() : 1),  guid, x, itemProto->ItemId, iter->randProp, iter->randSeed, GetMapInstance());

                data2.Initialize(SMSG_LOOT_START_ROLL);
                data2 << uint64(guid);
                data2 << uint32(mapid);
                data2 << uint32(x);
                data2 << uint32(itemProto->ItemId);
                data2 << uint32(iter->randProp);
                data2 << uint32(iter->randSeed);
                data2 << uint32(iter->StackSize);
                data2 << uint32(60000); // countdown

                if(Group* pGroup = m_playerInfo->m_Group)
                {
                    size_t maskpos = data2.wpos();
                    data2 << uint8(0);

                    pGroup->Lock();
                    Player* plr = NULL;
                    for(uint32 i = 0; i < pGroup->GetSubGroupCount(); i++)
                    {
                        for(GroupMembersSet::iterator itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); itr++)
                        {
                            plr = (*itr)->m_loggedInPlayer;
                            //if(plr && plr->GetInventory()->CanReceiveItem(itemProto, iter->StackSize, NULL) == 0)
                            {   // If we have pass on, or if we're not in range, we have to pass.
                                if( plr->m_passOnLoot || false)//( !lootObj->IsInRangeSet(plr) ) )
                                    iter->roll->PlayerRolled( (*itr), PASS );       // passed
                                else
                                {
                                    uint8 canusemask = ROLLMASK_PASS|ROLLMASK_GREED;
                                    if(plr->CanNeedItem(itemProto))
                                        canusemask |= ROLLMASK_NEED;
                                    if(pGroup->HasAcceptableDisenchanters(itemProto->DisenchantReqSkill))
                                        canusemask |= ROLLMASK_DISENCHANT;

                                    data2.put<uint8>(maskpos, canusemask);
                                    plr->PushPacket(&data2);
                                }
                            }// else iter->roll->PlayerRolled( (*itr), PASS );       // passed
                        }
                    }
                    pGroup->Unlock();
                }
                else
                {
                    uint8 canusemask = ROLLMASK_PASS|ROLLMASK_GREED;
                    if(CanNeedItem(itemProto))
                        canusemask |= ROLLMASK_NEED;
                    if(itemProto->DisenchantReqSkill > -1)
                        if(HasSkillLine(SKILL_ENCHANTING) && (getSkillLineVal(SKILL_ENCHANTING, true) > uint32(itemProto->DisenchantReqSkill)))
                            canusemask |= ROLLMASK_DISENCHANT;

                    data2 << canusemask;
                    PushPacket(&data2);
                }
            }
        }
        count++;
    }
    data.put<uint8>(13, count);

    PushPacket(&data);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);
    lootEnt->LootUnlock();
}

void Player::SendXPToggleConfirm()
{
    m_XPoff = !m_XPoff;
}

uint32 Player::CalcTalentResetCost(uint32 resetnum)
{
    if(resetnum == 0 )
        return  10000;
    if(resetnum >= 10)
        return  500000;
    return resetnum*50000;
}

void Player::SendTalentResetConfirm()
{
    WorldPacket data(MSG_TALENT_WIPE_CONFIRM, 12);
    data << GetGUID();
    data << CalcTalentResetCost(m_talentInterface.GetTalentResets());
    PushPacket(&data);
}

void Player::removeSpellByNameHash(uint32 hash)
{
    SpellSet::iterator it,iter;

    for(iter= m_spells.begin();iter != m_spells.end();)
    {
        it = iter++;
        uint32 SpellID = *it;
        SpellEntry *e = dbcSpell.LookupEntry(SpellID);
        if(e->NameHash == hash)
        {
            if(m_createInfo->spell_list.find(e->Id) != m_createInfo->spell_list.end())
                continue;

            RemoveAura(SpellID,GetGUID());
            PushData(SMSG_REMOVED_SPELL, 4, &SpellID);
            m_spells.erase(it);
            for(uint8 i = 0; i < 3; i++)
                if(uint8 effect = e->Effect[i])
                    m_spellsByEffect[effect].erase(e->Id);
            if(e->SpellSkillLine)
                m_spellsBySkill[e->SpellSkillLine].erase(e->Id);
            if(e->Category)
                m_spellCategories.erase(e->Category);
        }
    }
}

bool Player::removeSpell(uint32 SpellID)
{
    SpellEntry *sp = dbcSpell.LookupEntry(SpellID);
    if(sp == NULL)
        return false;

    SpellSet::iterator iter = m_spells.find(SpellID);
    if(iter == m_spells.end())
        return false;

    m_spells.erase(iter);
    for(uint8 i = 0; i < 3; i++)
        if(uint8 effect = sp->Effect[i])
            m_spellsByEffect[effect].erase(sp->Id);
    if(sp->SpellSkillLine)
        m_spellsBySkill[sp->SpellSkillLine].erase(sp->Id);
    RemoveAura(SpellID,GetGUID());

    // We need to either remove our skill line completely or readd the previous rank
    if(SkillLineEntry *skill = dbcSkillLine.LookupEntry(sp->SpellSkillLine))
    {
        SpellEntry* sp2 = FindHighestRankProfessionSpell(skill->id);
        if(skill->spellIcon && (skill->categoryId == SKILL_TYPE_PROFESSION || skill->categoryId == SKILL_TYPE_SECONDARY))
        {
            uint16 current = getSkillLineVal(skill->id, false), bonus = getSkillLineVal(skill->id, true)-current;
            if(sp2 == NULL)
                RemoveSkillLine(skill->id);
            else
            {
                UpdateSkillLine(skill->id, sp2->RankNumber, 75*sp2->RankNumber, true);
                _UpdateMaxSkillCounts();
            }
        }
    }

    if(!IsInWorld())
        return true;

    PushData(SMSG_REMOVED_SPELL, 4, &SpellID);
    return true;
}

void Player::RemoveQuestsFromLine(uint32 skill_line)
{
    for(int i = 0; i < QUEST_LOG_COUNT; i++)
    {
        if (m_questLog[i])
        {
            Quest* qst = m_questLog[i]->GetQuest();
            if(qst == NULL)
                continue;

            if(qst->required_tradeskill == skill_line)
            {
                m_questLog[i]->Finish();

                delete m_questLog[i];
                m_questLog[i] = NULL;
            }
        }
    }

    /*std::map<uint32, time_t>::iterator itr;
    for (itr = m_completedQuests.begin(); itr != m_completedQuests.end();)
    {
        Quest * qst = sQuestMgr.GetQuestPointer(itr->first);
        if(qst->required_tradeskill == skill_line)
            itr = m_completedQuests.erase(itr);
        else itr++;
    }*/

    UpdateNearbyGameObjects();
}

void Player::SendProficiency(bool armorProficiency)
{
    WorldPacket data(SMSG_SET_PROFICIENCY, 5);
    if(armorProficiency)
        data << uint8(ITEM_CLASS_ARMOR) << GetArmorProficiency();
    else data << uint8(ITEM_CLASS_WEAPON) << GetWeaponProficiency();
    PushPacket(&data, !IsInWorld());
}

// Initial packets, these don't need to be sent when switching between maps
void Player::SendInitialLogonPackets()
{
    WorldPacket data(SMSG_BINDPOINTUPDATE, 32);
    data << m_bindData.posX << m_bindData.posY << m_bindData.posZ;
    data << m_bindData.mapId << m_bindData.zoneId;
    PushPacket( &data, true );

    SendProficiency(true);
    SendProficiency(false);

    // Send player talent info
    m_talentInterface.SendTalentInfo();

    data.Initialize(SMSG_WORLD_SERVER_INFO, 5);
    data.WriteBit(0);                       // HasRestrictedLevel
    data.WriteBit(0);                       // HasRestrictedMoney
    data.WriteBit(0);                       // IneligibleForLoot
    data.FlushBits();
    //if (IneligibleForLoot)
    //    data << uint32(0);                // EncounterMask
    data << uint8(0);                       // IsOnTournamentRealm
    //if (HasRestrictedMoney)
    //    data << uint32(100000);           // RestrictedMoney (starter accounts)
    //if (HasRestrictedLevel)
    //    data << uint32(20);               // RestrictedLevel (starter accounts)
    data << uint32(sWorld.GetWeekStart());  // LastWeeklyReset (not instance reset)
    data << uint32(0);//sInstanceMgr.GetDungeonDifficulty(m_instanceId));
    PushPacket(&data, true);

    //Initial Spells
    smsg_InitialSpells();

    data.Initialize(SMSG_SEND_UNLEARN_SPELLS, 4);
    data << uint32(0); // count, for (count) uint32;
    PushPacket(&data, true);

    // Send our action bar
    m_talentInterface.SendInitialActions();

    //Factions
    data.Initialize(SMSG_INITIALIZE_FACTIONS);
    m_factionInterface.BuildInitialFactions(&data);
    PushPacket(&data, true);

    data.Initialize(SMSG_ALL_ACHIEVEMENT_DATA);
    AchieveMgr.BuildAchievementData(GetGUID(), &data, getLevel() <= 9);
    PushPacket(&data, true);

    // Login speed
    data.Initialize(SMSG_LOGIN_SETTIMESPEED);
    data << uint32(RONIN_UTIL::secsToTimeBitFields(UNIXTIME));
    data << float(0.01666667f) << uint32(0);//getMSTime());
    PushPacket( &data, true );

    m_currency.SendInitialCurrency();

    sLog.Debug("WORLD","Sent initial logon packets for %s.", GetName());
}

void Player::Reset_Spells()
{
    PlayerCreateInfo *info = objmgr.GetPlayerCreateInfo(getRace(), getClass());
    ASSERT(info);
    SpellSet spelllist;
    SpellSet::iterator itr;
    bool profession = false;

    for(itr = m_spells.begin(); itr != m_spells.end(); itr++)
    {
        SpellEntry *sp = dbcSpell.LookupEntry((*itr));
        if(sp == NULL)
            continue;

        for( uint32 lp = 0; lp < 3; lp++ )
        {
            if( sp->Effect[lp] == SPELL_EFFECT_SKILL )
                profession = true;
        }

        if( !profession )
            spelllist.insert((*itr));

        profession = false;
    }

    for(itr = spelllist.begin(); itr != spelllist.end(); itr++)
        removeSpell((*itr));

    for(itr = info->spell_list.begin(); itr != info->spell_list.end(); itr++)
    {
        if(*itr)
            addSpell(*itr);
    }
}

void Player::Reset_ToLevel1()
{
    m_AuraInterface.RemoveAllNonPassiveAuras();

    setLevel(1);
}

void Player::UpdateNearbyGameObjects()
{
    ByteBuffer buff(500);
}

void Player::InitTaxiNodes()
{
    // Automatically add the race's taxi hub to the character's taximask at creation time ( 1 << (taxi_node_id-1) )
    if(sWorld.Start_With_All_Taximasks)
    {
        m_taxiMask = *sTaxiMgr.GetAllTaxiMasks();
        return;
    }

    if(getClass() == DEATHKNIGHT)
        m_taxiMask = *sTaxiMgr.GetDeathKnightTaxiMasks();

    switch(getRace())
    {
    case RACE_TAUREN:   AddTaxiMask(22);                    break;
    case RACE_HUMAN:    AddTaxiMask(2);                     break;
    case RACE_DWARF:    AddTaxiMask(6);                     break;
    case RACE_GNOME:    AddTaxiMask(6);                     break;
    case RACE_ORC:      AddTaxiMask(23);                    break;
    case RACE_TROLL:    AddTaxiMask(23);                    break;
    case RACE_UNDEAD:   AddTaxiMask(11);                    break;
    case RACE_NIGHTELF: AddTaxiMask(26); AddTaxiMask(27);   break;
    case RACE_BLOODELF: AddTaxiMask(82);                    break;
    case RACE_DRAENEI:  AddTaxiMask(94);                    break;
    }
    // team dependant taxi node
    AddTaxiMask(100-GetTeam());

    if(getLevel() < 68)
        return;
    AddTaxiMask(213);
}

bool Player::HasNearbyTaxiNodes(uint32 from)
{
    bool ret = false;
    if(std::vector<uint32> *pathTargets = sTaxiMgr.GetPathTargets(from))
    {
        for(std::vector<uint32>::iterator itr = pathTargets->begin(); itr != pathTargets->end(); itr++)
        {
            if(HasTaxiNode(*itr))
            {
                ret = true;
                break;
            }
        }
    }
    return ret;
}

void Player::EventTaxiInterpolate()
{
    if(m_taxiData == NULL || m_taxiData->CurrentPath == NULL || !IsInWorld())
        return;

    float x = 0.f, y = 0.f, z = 0.f;
    // Compare current travel time against our arrival time for either path finish or position update
    if(m_taxiData->TravelTime >= m_taxiData->ArrivalTime)
    {
        // Reset our travel time
        m_taxiData->TravelTime = 0;
        // Check to see if we have a map to change to
        if(m_taxiData->CurrentPath->HasMapChange(GetMapId()))
        {
            uint32 map = 0;
            m_taxiData->CurrentPath->GetMapTargetPos(x, y, z, &map);
            SafeTeleport(map, 0, x, y, z, 0.f);
        }
        else
        {
            // Grab our current path's price
            uint32 price = m_taxiData->CurrentPath->GetPrice();
            m_taxiData->CurrentPath->GetEndPos(x, y, z);
            // Clean up our taxi data if we have no future paths
            if(m_taxiData->paths.empty())
            {
                delete m_taxiData;
                m_taxiData = NULL;
            }
            // Call our dismount function
            EventDismount(price, x, y, z);
        }
        return;
    }

    // Path isn't finished, so get our current position along the path and set it
    m_taxiData->CurrentPath->GetPosForTime(m_mapId, x, y, z, m_taxiData->TravelTime);
    SetPosition(x,y,z,0.f);
}

void Player::TaxiStart(TaxiPath *path, uint32 modelid, uint32 startOverride)
{
    Dismount();

    //also remove morph spells
    if(GetUInt32Value(UNIT_FIELD_DISPLAYID) != GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID))
    {
        m_AuraInterface.RemoveAllAurasOfType(SPELL_AURA_TRANSFORM);
        m_AuraInterface.RemoveAllAurasOfType(SPELL_AURA_MOD_SHAPESHIFT);
    }

    SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID, modelid );
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);
    m_movementInterface.setRooted(true);

    if(GetSession())
    {
        GetSession()->m_isFalling = false;
        GetSession()->m_isJumping = false;
        GetSession()->m_isKnockedback = false;
    }

    if(m_taxiData == NULL)
        m_taxiData = new Player::TaxiData();

    // Construct new taxi data pointer
    m_taxiData->StartPath(getMSTime(), path, startOverride, modelid, 1000 * (path->GetLength(GetMapId())/TAXI_TRAVEL_SPEED));

    // Send our movement packet for the taxi trip
    path->SendMoveForTime(this, this, m_taxiData->TravelTime, m_taxiData->MoveTime);
    // Add or subtract an extra 2 seconds to travel timeout
    if(path->HasMapChange(GetMapId()))
        m_taxiData->ArrivalTime -= 2000;
}

void Player::JumpToEndTaxiNode(TaxiPath * path)
{
    delete m_taxiData;
    m_taxiData = NULL;

    m_movementInterface.setRooted(m_AuraInterface.HasAuraWithMechanic(MECHANIC_STUNNED));
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    m_movementInterface.OnTaxiEnd();

    // this should *always* be safe in case it cant build your position on the path!
    float x, y, z; uint32 map;
    path->GetEndPos(x, y, z, &map);
    SafeTeleport(map, 0, x, y, z, 0.f);
}

void Player::RemoveSpellsFromLine(uint16 skill_line)
{
    std::vector<uint32> *skillLine = sSpellMgr.GetSkillLineEntries(skill_line);
    if(skillLine == NULL)
        return;

    for(auto itr = skillLine->begin(); itr != skillLine->end(); ++itr)
        removeSpell(*itr);
}

void Player::RegeneratePower(bool is_interrupted)
{
    float m_regenTimer = m_P_regenTimer; //set next regen time
    float regenModifier = (m_regenTimer/1000.f);

    uint8 index = 0;
    bool isInCombat = IsInCombat();
    std::vector<uint8> *classPower = sStatSystem.GetUnitPowersForClass(getClass());
    for(std::vector<uint8>::iterator itr = classPower->begin(); itr != classPower->end(); itr++)
    {
        uint8 power = *itr, powerIndex = index++;
        EUnitFields powerField = GetPowerFieldForType(power);
        if (powerField == UNIT_END || power == POWER_TYPE_RUNE)
            continue;

        uint32 curValue = GetPower(powerField), maxValue = GetMaxPower(EUnitFields(powerField+(UNIT_FIELD_MAXPOWERS-UNIT_FIELD_POWERS)));
        if(curValue == 0 && (power == POWER_TYPE_RAGE || power == POWER_TYPE_RUNIC || power == POWER_TYPE_HOLY_POWER))
            continue;
        float flatModifier = GetFloatValue((is_interrupted ? UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER : UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER)+powerIndex);

        m_regenTimerCounters[powerIndex] += m_regenTimer;

        float addvalue = 0.0f;
        switch (power)
        {
        case POWER_TYPE_MANA:
            {
                addvalue += flatModifier + (0.00001f*GetFloatValue(UNIT_MOD_CAST_SPEED));
            }break;
        case POWER_TYPE_RAGE:
            {
                if (!isInCombat && !is_interrupted)
                    addvalue = ((-1.25f * 10.f)/GetFloatValue(PLAYER_FIELD_MOD_HASTE));
            }break;
        case POWER_TYPE_FOCUS:
            {
                addvalue += (3.f + flatModifier) + (0.03f*GetFloatValue(PLAYER_FIELD_MOD_RANGED_HASTE));
            }break;
        case POWER_TYPE_ENERGY:
            {
                addvalue += (10.f + flatModifier) + (0.0001f*GetFloatValue(PLAYER_FIELD_MOD_HASTE));
            }break;
        case POWER_TYPE_RUNIC:
            {
                if (!isInCombat && !is_interrupted)
                    addvalue += -15;
            }break;
        case POWER_TYPE_HOLY_POWER:
            {
                addvalue = -1;
                if (isInCombat || is_interrupted) // If we're in combat, interrupted, or below our regen timer, just set current value to 0
                    m_regenTimerCounters[powerIndex] = 0;
                if(m_regenTimerCounters[powerIndex] < 10000)
                    curValue = 0;
            }break;
        }

        if((addvalue < 0.f && curValue == 0) || (addvalue > 0.f && curValue == maxValue))
            continue;

        float intval = 0.0f;
        addvalue *= regenModifier;
        addvalue += m_regenPowerFraction[powerIndex];
        m_regenPowerFraction[powerIndex] = modf(addvalue, &intval);
        uint32 integerValue = uint32(abs(intval));
        if (addvalue < 0.0f)
        {
            if (curValue > integerValue)
                curValue -= integerValue;
            else
            {
                curValue = 0;
                m_regenPowerFraction[powerIndex] = 0;
            }
        }
        else
        {
            curValue += integerValue;
            if (curValue > maxValue)
            {
                curValue = maxValue;
                m_regenPowerFraction[powerIndex] = 0;
            }
        }

        if (m_regenTimerCounters[powerIndex] >= 2000 || m_regenPowerFraction[powerIndex] == 0 || curValue == maxValue)
        {
            SetPower(powerField, curValue);
            m_regenTimerCounters[powerIndex] = 0;
        } else m_uint32Values[powerField] = curValue;
    }
}

void Player::RegenerateHealth( bool inCombat )
{
    uint32 cur = GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 mh = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(cur >= mh)
        return;
    AuraInterface::modifierMap *modMap = NULL;

    float amt = 0.f;
    if(m_AuraInterface.HasAuraWithMechanic(MECHANIC_POLYMORPHED))
        amt = float(mh)/3.f;
    else if(!inCombat || m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_REGEN_DURING_COMBAT))
    {
        uint8 level = std::min<uint8>(getLevel(), MAXIMUM_ATTAINABLE_LEVEL);
        amt = (level >= 15 ? 0.015f : 0.2f) * (float)mh;
        if(level < 15) amt /= level;

        float modifier = inCombat ? 0.f : 100.f;
        if(modMap = m_AuraInterface.GetModMapByModType(inCombat ? SPELL_AURA_MOD_REGEN_DURING_COMBAT : SPELL_AURA_MOD_REGEN))
            for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
                modifier += itr->second->m_amount;
        amt *= modifier/100.f;

        if(IsSitting())
            amt *= 1.33f;
    }

    if(modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT))
        for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
            amt += itr->second->m_amount;

    amt += GetBonusesFromItems(ITEM_STAT_HEALTH_REGEN);

    if(amt > 0)
    {
        if(amt <= 1.0f)//this fixes regen like 0.98
            cur++;
        else cur += float2int32(floor(amt));
        SetUInt32Value(UNIT_FIELD_HEALTH, (cur >= mh) ? mh : cur);
    }
}

void Player::_Relocate(uint32 mapid, const LocationVector& v, bool force_new_world, uint32 instance_id)
{
    bool pushHard = !IsInWorld();
    //Send transfer pending only when switching between differnt mapids!
    WorldPacket data(SMSG_TRANSFER_PENDING, 41);
    if(mapid != m_mapId && force_new_world)
    {
        // Unk and transport transfer
        data.WriteBitString(2, 0, 0);
        data.FlushBits();
        data << mapid;
        PushPacket(&data, pushHard);
    }

    LocationVector destination(v);

    //are we changing maps?
    if(m_mapId != mapid || force_new_world)
    {
        //Preteleport will try to find an instance (saved or active), or create a new one if none found.
        uint32 status = sWorldMgr.PreTeleport(mapid, this, instance_id, GetGroup() && GetGroup()->IsGroupFinderInstance(mapid));
        if(status != INSTANCE_OK)
        {
            data.Initialize(SMSG_TRANSFER_ABORTED);
            data << mapid << uint8(status <= INSTANCE_ABORT_MAX_CLIENT_IDS ? status : INSTANCE_ABORT_ERROR_ERROR) << uint8(0);
            PushPacket(&data, pushHard);
            return;
        }

        //remove us from this map
        if(IsInWorld())
            GetMapInstance()->QueueRemoval(this);

        //send new world
        m_movementInterface.TeleportToPosition(mapid, instance_id, destination);
        SetPlayerStatus(TRANSFER_PENDING);
    }
    else
    {
        // we are on same map allready, no further checks needed,
        // send teleport ack msg
        m_movementInterface.TeleportToPosition(destination);

        //reset transporter if we where on one.
        if( m_CurrentTransporter && !m_movementInterface.isTransportLocked() )
        {
            m_movementInterface.ClearTransportData();
            m_CurrentTransporter->RemovePlayer(castPtr<Player>(this));
            m_CurrentTransporter = NULL;
        }

        // Update our object cell manager
        if(IsInWorld()) GetCellManager()->OnRelocate(m_mapInstance, destination);
    }

    //update position
    ApplyPlayerRestState(false); // If we don't, and we teleport inside, we'll be rested regardless.
}

uint32 Player::BuildCreateUpdateBlockForPlayer(ByteBuffer *data, Player* target )
{
    uint32 count = 0;
    if(target == castPtr<Player>(this)) // we need to send create objects for all items.
        count += m_inventory.m_CreateForPlayer(data);
    return count+Unit::BuildCreateUpdateBlockForPlayer(data, target);
}

void Player::_Warn(const char *message)
{
    sChatHandler.RedSystemMessage(GetSession(), message);
}

void Player::Kick(uint32 delay /* = 0 */, bool hardKick)
{
    m_hardKick |= hardKick;
    if(m_KickDelay > 0 && m_KickDelay < delay)
        m_KickDelay /= 2;
    else if((m_KickDelay = delay) >= 0)
        SetUnitStunned(true);
    else _Kick();
}

void Player::_Kick()
{
    if(m_KickDelay)
    {
        if(m_KickDelay > 1500)
            m_KickDelay -= 1000;
        else m_KickDelay = 0;

        if(m_KickDelay)
            return;
    }

    if(m_hardKick == false)
        SoftDisconnect();
    else if(WorldSession *session = m_session)
        session->Disconnect();
}

void Player::ClearCooldownForSpell(uint32 spell_id)
{
    if( IsInWorld() )
    {
        WorldPacket data(SMSG_CLEAR_COOLDOWN, 12);
        data << spell_id << GetGUID();
        PushPacket(&data);
    }

    // remove cooldown data from Server side lists
    PlayerCooldownMap::iterator itr, itr2;
    SpellEntry * spe = dbcSpell.LookupEntry(spell_id);
    if(spe == NULL)
        return;

    for(uint32 i = 0; i < NUM_COOLDOWN_TYPES; i++)
    {
        for( itr = m_cooldownMap[i].begin(); itr != m_cooldownMap[i].end(); )
        {
            itr2 = itr++;
            if( ( i == COOLDOWN_TYPE_CATEGORY && itr2->first == spe->Category ) ||
                ( i == COOLDOWN_TYPE_SPELL && itr2->first == spe->Id ) )
            {
                m_cooldownMap[i].erase( itr2 );
            }
        }
    }
}

void Player::ClearCooldownsOnLine(uint32 skill_line, uint32 called_from)
{
    // found an easier way.. loop spells, check skill line
    SpellSet::const_iterator itr = m_spells.begin();
    SkillLineAbilityEntry *sk;
    for(; itr != m_spells.end(); itr++)
    {
        if((*itr) == called_from)      // skip calling spell.. otherwise spammies! :D
            continue;

    }
}

void Player::ClearCooldownsOnLines(std::set<uint32> skill_lines, uint32 called_from)
{
    SpellSet::const_iterator itr = m_spells.begin();
    SkillLineAbilityEntry *sk;
    for(; itr != m_spells.end(); itr++)
    {
        if((*itr) == called_from)      // skip calling spell.. otherwise spammies! :D
            continue;

    }
}

void Player::ResetAllCooldowns()
{
    std::set<uint32> skilllines;
    switch(getClass())
    {
    case WARRIOR:
        {
            skilllines.insert(26);
            skilllines.insert(256);
            skilllines.insert(257);
        } break;
    case PALADIN:
        {
            skilllines.insert(184);
            skilllines.insert(267);
            skilllines.insert(594);
        } break;
    case HUNTER:
        {
            skilllines.insert(50);
            skilllines.insert(51);
            skilllines.insert(163);
        } break;
    case ROGUE:
        {
            skilllines.insert(38);
            skilllines.insert(39);
            skilllines.insert(253);
        } break;
    case PRIEST:
        {
            skilllines.insert(56);
            skilllines.insert(78);
            skilllines.insert(613);
        } break;
    case DEATHKNIGHT:
        {
            skilllines.insert(770);
            skilllines.insert(771);
            skilllines.insert(772);
        } break;
    case SHAMAN:
        {
            skilllines.insert(373);
            skilllines.insert(374);
            skilllines.insert(375);
        } break;
    case MAGE:
        {
            skilllines.insert(6);
            skilllines.insert(8);
            skilllines.insert(237);
        } break;
    case WARLOCK:
        {
            skilllines.insert(355);
            skilllines.insert(354);
            skilllines.insert(593);
        } break;
    case DRUID:
        {
            skilllines.insert(573);
            skilllines.insert(574);
            skilllines.insert(134);
        } break;
    default:
        return;
        break;
    }
    ClearCooldownsOnLines(skilllines, 0);
}

void Player::PushOutOfRange(uint16 mapId, WoWGuid guid)
{
    _bufferS.Acquire();
    // Set data size for limiting update blocks to 4Kb
    if( (guid.pLen() + m_OutOfRangeIds.size()) >= 0x1000 )
        PopPendingUpdates(mapId);

    ++m_OutOfRangeIdCount;
    m_OutOfRangeIds << guid.asPacked();

    // add to process queue
    if(m_mapInstance && !bProcessPending)
    {
        bProcessPending = true;
        m_mapInstance->PushToProcessed(this);
    }
    _bufferS.Release();
}

void Player::PushUpdateBlock(uint16 mapId, ByteBuffer *data, uint32 updatecount)
{
    // Cause lazy
    if(updatecount == 0)
        return;

    // imagine the bytebuffer getting appended from 2 threads at once! :D
    _bufferS.Acquire();

    // Set data size for limiting update blocks to 45Kb
    if( (data->size() + m_updateDataBuff.size()) >= 0xAFFF )
        PopPendingUpdates(mapId);

    m_updateDataCount += updatecount;
    m_updateDataBuff.append(data->contents(), data->size());

    // add to process queue
    if(m_mapInstance && !bProcessPending)
    {
        bProcessPending = true;
        m_mapInstance->PushToProcessed(this);
    }

    _bufferS.Release();
}

void Player::PopPendingUpdates(uint16 mapId)
{
    if(m_session == NULL)
        return;

    _bufferS.Acquire();
    if(m_updateDataCount || m_OutOfRangeIdCount)
    {
        WorldPacket data(SMSG_UPDATE_OBJECT, 2 + 4 + (m_OutOfRangeIdCount ? 1 + 4 + m_OutOfRangeIds.size() : 0) + m_updateDataBuff.size());
        data << uint16(mapId) << uint32(m_updateDataCount + (m_OutOfRangeIdCount ? 1 : 0));
        if(m_OutOfRangeIdCount)
        {
            data << uint8(UPDATETYPE_OUT_OF_RANGE_OBJECTS);
            data << uint32(m_OutOfRangeIdCount);
            data.append(m_OutOfRangeIds.contents(), m_OutOfRangeIds.size());
            m_OutOfRangeIds.clear();
            m_OutOfRangeIdCount = 0;
        }

        if(m_updateDataCount)
        {
            data.append(m_updateDataBuff.contents(), m_updateDataBuff.size());
            m_updateDataBuff.clear();
            m_updateDataCount = 0;
        }

        // Send our packet
        PushPacket(&data, true);
    }

    bProcessPending = false;
    _bufferS.Release();

    // send any delayed packets
    WorldPacket * pck;
    while(!m_packetQueue.empty())
    {
        pck = m_packetQueue.next();
        PushPacket(pck, true);
        delete pck;
    }
}

bool Player::AllowChannelAtLocation(uint32 dbcID, AreaTableEntry *areaTable)
{
    bool result = true;
    switch(dbcID)
    {
    case 2:
        {
            if(sWorld.trade_world_chat == 0)
                result = areaTable ? ((areaTable->AreaFlags & AREA_CITY_AREA) || (areaTable->AreaFlags & AREA_CITY)) : false;
            // Set GM Island to always have trade chat
            if(areaTable && areaTable->AreaId == 876)
                result = true;
        }break;
    case 26:
        {
            result = false;//(LfgDungeonId[0] > 0 || LfgDungeonId[1] > 0 || LfgDungeonId[2] > 0);
        }break;
    }
    return result;
}

bool Player::UpdateChatChannel(const char* areaName, AreaTableEntry *areaTable, ChatChannelEntry* entry, Channel* channel)
{
    if(!AllowChannelAtLocation(entry->id, areaTable))
    {
        if(channel)
            channel->Part(this, false, true); // Part with notice but keep data!
        return true;
    }

    char name[255];
    sprintf(name, "%s", entry->pattern);
    if(entry->flags & 0x02)
        sprintf(name, entry->pattern, areaName);
    Channel *chn = channelmgr.GetCreateDBCChannel(name, this, entry->id);
    if(chn == NULL)
    {
        if(channel)
            channel->Part(this, true);
        return false;
    }
    else if(chn->HasMember(this))
        return false;

    if(channel && strcmp(name, channel->m_name.c_str()))
        channel->Part(this, true);
    chn->AttemptJoin(this, "");
    sLog.Debug("ChannelJoin", "Player %s joined channel %s", GetName(), name);
    return false;
}

void Player::EventDBCChatUpdate(MapInstance *instance, uint32 dbcID)
{
    char areaName[255];
    MapEntry *entry = NULL;
    AreaTableEntry *areaTable = dbcAreaTable.LookupEntry(m_zoneId);
    if(areaTable == NULL && (areaTable = dbcAreaTable.LookupEntry(m_areaId)) == NULL && instance)
        areaTable = dbcAreaTable.LookupEntry(instance->GetdbcMap()->linked_zone);

    if(areaTable)
        sprintf(areaName, "%s", areaTable->name);
    else if(instance && (entry = instance->GetdbcMap()))
        sprintf(areaName, "%s", entry->name);
    else sprintf(areaName, "City_%03u", GetMapId());

    if(dbcID == 0xFFFFFFFF)
    {
        if(!m_channelsbyDBCID.size())
            return;

        for(uint32 i = 0; i < dbcChatChannels.GetNumRows(); i++)
        {
            Channel *channel = NULL;
            ChatChannelEntry* entry = dbcChatChannels.LookupRow(i);
            if(m_channelsbyDBCID.find(entry->id) != m_channelsbyDBCID.end())
                channel = m_channelsbyDBCID.at(entry->id);
            if(UpdateChatChannel(areaName, areaTable, entry, channel))
                m_channelsbyDBCID.erase(entry->id);
        }
    }
    else
    {
        ChatChannelEntry* entry = dbcChatChannels.LookupEntry(dbcID);
        if(entry == NULL || !strlen(entry->pattern))
            return;

        std::map<uint32, Channel*>::iterator itr = m_channelsbyDBCID.find(dbcID);
        if(itr != m_channelsbyDBCID.end())
        {
            if(UpdateChatChannel(areaName, areaTable, entry, itr->second))
                m_channelsbyDBCID.erase(entry->id);
        }
        else
        {
            if(!AllowChannelAtLocation(dbcID, areaTable))
                return;

            char name[255];
            if(entry->flags & 0x02)
                sprintf(name, entry->pattern, areaName);
            else sprintf(name, "%s", entry->pattern);
            Channel *chn = channelmgr.GetCreateDBCChannel(name, this, entry->id);
            if(chn == NULL || chn->HasMember(this))
                return;
            chn->AttemptJoin(this, "");
        }
    }
}

void Player::SetGuildId(uint32 guildId)
{
    SetUInt64Value(PLAYER_GUILDID, guildId ? MAKE_NEW_GUID(guildId, 0, HIGHGUID_TYPE_GUILD) : 0);
    if(guildId)
    {
        SetFlag(PLAYER_FLAGS, 0x10000000);
        SetFlag(OBJECT_FIELD_TYPE, TYPEMASK_FLAG_IN_GUILD);
    }
    else
    {
        RemoveFlag(PLAYER_FLAGS, 0x10000000);
        RemoveFlag(OBJECT_FIELD_TYPE, TYPEMASK_FLAG_IN_GUILD);
    }
}

void Player::StopMirrorTimer(uint32 Type)
{
    PushData(SMSG_STOP_MIRROR_TIMER, 4, &Type);
}

void Player::EventTeleport(uint32 mapid, float x, float y, float z, float o)
{
    SafeTeleport(mapid, 0, LocationVector(x, y, z, o));
}

void Player::BroadcastMessage(const char* Format, ...)
{
    va_list l;
    va_start(l, Format);
    char Message[1024];
    vsnprintf(Message, 1024, Format, l);
    va_end(l);

    WorldPacket data;
    sChatHandler.FillSystemMessageData(&data, Message);
    PushPacket(&data);
}

float Player::CalcPercentForRating( uint32 index, uint32 rating )
{
    uint32 relative_index = index - (PLAYER_FIELD_COMBAT_RATING_1), level = std::min<uint32>(MAXIMUM_ATTAINABLE_LEVEL, GetUInt32Value(UNIT_FIELD_LEVEL));
    gtFloat *pDBCEntry = dbcCombatRating.LookupEntry( (relative_index * 100) + level - 1 );
    return float(rating/(pDBCEntry ? pDBCEntry->val : 1.f));
}

bool Player::SafeTeleport(uint32 MapID, uint32 InstanceID, float X, float Y, float Z, float O)
{
    return SafeTeleport(MapID, InstanceID, LocationVector(X, Y, Z, O));
}

bool Player::SafeTeleport(uint32 MapID, uint32 InstanceID, LocationVector vec)
{
    GameObject *Arbiter;//abort duel if other map or new distance is outside of wide duel radius
    if(IsInWorld() && IsInDuel() && (Arbiter = GetMapInstance()->GetGameObject(GetUInt64Value(PLAYER_DUEL_ARBITER))) && (GetMapId() != MapID || !Arbiter->IsInDuelRadius(vec)))
        Arbiter->DuelEnd(this, this, DUEL_WINNER_RETREAT);

    //retrieve our bind point in case vector is 0,0,0
    if( vec.x == 0 && vec.y == 0 && vec.z == 0 )
    {
        vec.x = GetBindPositionX();
        vec.y = GetBindPositionY();
        vec.z = GetBindPositionZ();
        MapID = GetBindMapId();
    }

    if(GetShapeShift())
    {
        // Extra Check
        SetShapeShift(GetShapeShift());
    }

    /* Normal Version */
    bool force_new_world = false;

    // Lookup map info
    MapEntry * mapEntry = dbcMap.LookupEntry(MapID);
    if(mapEntry == NULL)
        return false;

    //are we changing instance or map?
    if(InstanceID && (uint32)m_instanceId != InstanceID)
    {
        force_new_world = true;
        SetInstanceID(InstanceID);
    }
    else if(m_mapId != MapID)
    {
        force_new_world = true;
    }

    //We are going to another map
    if( force_new_world )
    {
        //Do we need TBC expansion?
        if(mapEntry->addon == 1 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_01) && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
        {
            WorldPacket data;
            sChatHandler.FillSystemMessageData(&data, "You must have The Burning Crusade Expansion to access this content.");
            PushPacket(&data);
            return false;
        }

        //Do we need WOTLK expansion?
        if(mapEntry->addon == 2 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_02))
        {
            WorldPacket data;
            sChatHandler.FillSystemMessageData(&data, "You must have the Wrath of the Lich King Expansion to access this content.");
            PushPacket(&data);
            return false;
        }

        // Xpack gonna give it to ya
        if(mapEntry->addon == 3 && !m_session->HasFlag(ACCOUNT_FLAG_XPACK_03))
        {
            WorldPacket data;
            sChatHandler.FillSystemMessageData(&data, "You must have the Cataclysm Expansion to access this content.");
            PushPacket(&data);
            return false;
        }
        // Dismount
        Dismount();
    }

    //no flying outside new continents
    if((GetShapeShift() == FORM_FLIGHT || GetShapeShift() == FORM_SWIFT) && MapID != 530 && MapID != 571 )
        RemoveShapeShiftSpell(m_ShapeShifted);

    //all set...relocate
    _Relocate(MapID, vec, force_new_world, InstanceID);
    return true;
}

void Player::SafeTeleport(MapInstance* mgr, LocationVector vec)
{
    MapInstance *currInstance = GetMapInstance();
    if(currInstance) // If we're in world, queue our removal
    {
         currInstance->QueueRemoval(this);

         GameObject *arbiter;
         if(IsInDuel() && (arbiter = currInstance->GetGameObject(GetUInt64Value(PLAYER_DUEL_ARBITER))) && (currInstance != mgr || !arbiter->IsInDuelRadius(vec)))
             arbiter->DuelEnd(this, this, DUEL_WINNER_RETREAT);
    }

    uint32 mapId = mgr->GetMapId(), instanceId = mgr->GetInstanceID();
    WorldPacket data(SMSG_TRANSFER_PENDING, 20);
    data.WriteBitString(2, 0, 0); // Unk and transport transfer
    data.append<uint32>(mapId);
    PushPacket(&data, !IsInWorld());

    // Send transfers via movement interface to set target location and avoid issues with return data
    m_movementInterface.TeleportToPosition(mapId, instanceId, vec);
    SetPlayerStatus(TRANSFER_PENDING);

    if(uint8 ss = GetShapeShift()) // Extra Check
        SetShapeShift(ss);
}

void Player::BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag)
{

}

void Player::UpdatePvPState(uint32 msTime, uint32 uiDiff)
{
    if(m_pvpTimer > uiDiff)
    {
        m_pvpTimer -= uiDiff;
        return;
    }

    // Remove pvp timer flag to prevent future updates
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TIMER);
    // Reset timer back to default(5 minutes)
    m_pvpTimer = 300000;

    // If we are in pvp area, we can't change pvp state
    if(HasAreaFlag(OBJECT_AREA_FLAG_PVP_AREA))
        return;
    RemovePvPFlag();
}

void Player::RequestPvPToggle(bool state)
{
    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE) == state)
        return;

    if(state)
    {
        SetPvPFlag();
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TIMER);
        SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
    }
    else
    {
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TOGGLE);
        SetFlag(PLAYER_FLAGS, PLAYER_FLAG_PVP_TIMER);
        m_pvpTimer = 300000;
    }
}

void Player::SoftLoadPlayer()
{
    for(SpellSet::iterator itr = m_spells.begin(); itr != m_spells.end(); itr++)
    {
        SpellEntry *info = dbcSpell.LookupEntry(*itr);
        if(info && info->isPassiveSpell() && !info->isSpellExpiringWithPet())
        {
            if( info->RequiredShapeShift && !( ((uint32)1 << (GetShapeShift()-1)) & info->RequiredShapeShift ) )
                continue;
            GetSpellInterface()->TriggerSpell(info, this);
        }
    }

    // Initialize our talent info
    m_talentInterface.InitActiveSpec();

    static SpellEntry *shapeShift = dbcSpell.LookupEntry(2457), *honorless = dbcSpell.LookupEntry(PLAYER_HONORLESS_TARGET_SPELL);
    if(isDead()) // only add aura's to the living (death aura set elsewhere)
    {
        while(!m_loadAuras.empty())
        {
            Aura *aur = m_loadAuras.front().second;
            m_loadAuras.pop_front();
            delete aur;
        }

        // We have no shapeshift aura, set our shapeshift.
        if(getClass() == WARRIOR && shapeShift)
            GetSpellInterface()->TriggerSpell(shapeShift, this);
    }
    else
    {
        while(!m_loadAuras.empty())
        {
            std::pair<uint8, Aura*> pair = m_loadAuras.front();
            AddAura(pair.second, pair.first);
            m_loadAuras.pop_front();
        }

        // We have no shapeshift aura, set our shapeshift.
        if(getClass() == WARRIOR && shapeShift && !(HasAura(21156) || HasAura(7376) || HasAura(7381)))
            GetSpellInterface()->TriggerSpell(shapeShift, this);

        // Honorless target at 1st entering world.
        if(honorless) GetSpellInterface()->TriggerSpell(honorless, this);
    }
}

void Player::CompleteLoading()
{
    // Banned
    if(IsBanned())
    {
        const char * message = ("This character is banned for  %s.\n You will be kicked in 30 secs.", GetBanReason().c_str());

        // Send warning after 30sec, as he might miss it if it's send inmedeately.
    }

    if(raidgrouponlysent)
    {
        WorldPacket data2(SMSG_RAID_GROUP_ONLY, 8);
        data2 << uint32(0xFFFFFFFF) << uint32(0);
        PushPacket(&data2);
        raidgrouponlysent=false;
    }

    sWorldMgr.BuildSavedInstancesForPlayer(this);

    AchieveMgr.UpdateCriteriaValue(this, ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL, getLevel());
}

void Player::OnWorldPortAck()
{
    //only resurrect if player is porting to a instance portal
    if(MapEntry* map = dbcMap.LookupEntry(GetMapId()))
    {
        if(isDead() && (map->IsDungeon() || map->IsRaid()))
            ResurrectPlayer();

        if(map->IsMultiDifficulty() && GetMapInstance())
        {
            std::string welcome_msg;
            welcome_msg = "Welcome to ";
            welcome_msg += map->name;
            if(map->IsRaid())
            {
                switch(iRaidType)
                {
                case MODE_10PLAYER_NORMAL:
                    welcome_msg += " (10 Player)";
                    break;
                case MODE_25PLAYER_NORMAL:
                    welcome_msg += " (25 Player)";
                    break;
                case MODE_10PLAYER_HEROIC:
                    welcome_msg += " (10 Player Heroic)";
                    break;
                case MODE_25PLAYER_HEROIC:
                    welcome_msg += " (25 Player Heroic)";
                    break;
                default:
                    break;
                }
            }
            else
            {
                switch(iInstanceType)
                {
                case MODE_5PLAYER_NORMAL:
                    welcome_msg += " (5 Player)";
                    break;
                case MODE_5PLAYER_HEROIC:
                    welcome_msg += " (5 Player Heroic)";
                    break;
                default:
                    break;
                }
            }
            welcome_msg += ". ";
            if(map->IsRaid() && m_mapInstance->IsInstance())
            {
                /*welcome_msg += "This instance is scheduled to reset on ";
                welcome_msg += asctime(localtime(&m_mapInstance->pInstance->m_expiration));*/
                welcome_msg += "Instance Locks are scheduled to expire in ";
                //welcome_msg += RONIN_UTIL::ConvertTimeStampToString((uint32)castPtr<InstanceMgr>(m_mapInstance)->m_expiration - UNIXTIME);
            }
            sChatHandler.SystemMessage(m_session, welcome_msg.c_str());
        }
    }
}

int32 Player::GetBonusesFromItems(uint32 statType)
{
    int32 bonus = 0;
    for(ItemBonusModMap::iterator itr = itemBonusMapByType[statType].begin(); itr != itemBonusMapByType[statType].end(); itr++)
        bonus += itr->second.second;
    return bonus;
}

void Player::ModifyBonuses(bool apply, WoWGuid guid, uint32 slot, uint32 type, int32 val, int32 randSuffixAmt, int32 suffixSeed)
{
    std::pair<uint64, uint32> guid_slot = std::make_pair(guid, slot);
    if(apply)
    {
        if(itemBonusMap.find(guid_slot) == itemBonusMap.end())
            itemBonusMap.insert(std::make_pair(guid_slot, std::make_pair(type, val)));
        if(itemBonusMapByType[type].find(guid_slot) == itemBonusMapByType[type].end())
            itemBonusMapByType[type].insert(std::make_pair(guid_slot, std::make_pair(type, val)));
    }
    else
    {
        itemBonusMap.erase(guid_slot);
        itemBonusMapByType[type].erase(guid_slot);
    }

    switch(slot)
    {
    case MOD_SLOT_ARMOR:
        TriggerModUpdate(UF_UTYPE_RESISTANCE);
        break;
    case MOD_SLOT_MINDAMAGE:
    case MOD_SLOT_MAXDAMAGE:
        TriggerModUpdate(UF_UTYPE_ATTACKDAMAGE);
    case MOD_SLOT_WEAPONDELAY:
        TriggerModUpdate(UF_UTYPE_ATTACKTIME);
        break;

        // Enchant modifier entries that are called back through are stat types
    case MOD_SLOT_PERM_ENCHANT_1:
    case MOD_SLOT_PERM_ENCHANT_2:
    case MOD_SLOT_PERM_ENCHANT_3:
    case MOD_SLOT_TEMP_ENCHANT_1:
    case MOD_SLOT_TEMP_ENCHANT_2:
    case MOD_SLOT_TEMP_ENCHANT_3:
    case MOD_SLOT_SOCKET_ENCHANT_1_1:
    case MOD_SLOT_SOCKET_ENCHANT_1_2:
    case MOD_SLOT_SOCKET_ENCHANT_1_3:
    case MOD_SLOT_SOCKET_ENCHANT_2_1:
    case MOD_SLOT_SOCKET_ENCHANT_2_2:
    case MOD_SLOT_SOCKET_ENCHANT_2_3:
    case MOD_SLOT_SOCKET_ENCHANT_3_1:
    case MOD_SLOT_SOCKET_ENCHANT_3_2:
    case MOD_SLOT_SOCKET_ENCHANT_3_3:
    case MOD_SLOT_BONUS_ENCHANT_1:
    case MOD_SLOT_BONUS_ENCHANT_2:
    case MOD_SLOT_BONUS_ENCHANT_3:
    case MOD_SLOT_PRISMATIC_ENCHANT_1:
    case MOD_SLOT_PRISMATIC_ENCHANT_2:
    case MOD_SLOT_PRISMATIC_ENCHANT_3:
    case MOD_SLOT_REFORGE_ENCHANT_1:
    case MOD_SLOT_REFORGE_ENCHANT_2:
    case MOD_SLOT_REFORGE_ENCHANT_3:
    case MOD_SLOT_TRANSMOG_ENCHANT_1:
    case MOD_SLOT_TRANSMOG_ENCHANT_2:
    case MOD_SLOT_TRANSMOG_ENCHANT_3:
    case MOD_SLOT_PROPRETY_ENCHANT_0_1:
    case MOD_SLOT_PROPRETY_ENCHANT_0_2:
    case MOD_SLOT_PROPRETY_ENCHANT_0_3:
    case MOD_SLOT_PROPRETY_ENCHANT_1_1:
    case MOD_SLOT_PROPRETY_ENCHANT_1_2:
    case MOD_SLOT_PROPRETY_ENCHANT_1_3:
    case MOD_SLOT_PROPRETY_ENCHANT_2_1:
    case MOD_SLOT_PROPRETY_ENCHANT_2_2:
    case MOD_SLOT_PROPRETY_ENCHANT_2_3:
    case MOD_SLOT_PROPRETY_ENCHANT_3_1:
    case MOD_SLOT_PROPRETY_ENCHANT_3_2:
    case MOD_SLOT_PROPRETY_ENCHANT_3_3:
    case MOD_SLOT_PROPRETY_ENCHANT_4_1:
    case MOD_SLOT_PROPRETY_ENCHANT_4_2:
    case MOD_SLOT_PROPRETY_ENCHANT_4_3:

        // Stats pass through type and value
    case MOD_SLOT_STAT_1:
    case MOD_SLOT_STAT_2:
    case MOD_SLOT_STAT_3:
    case MOD_SLOT_STAT_4:
    case MOD_SLOT_STAT_5:
    case MOD_SLOT_STAT_6:
    case MOD_SLOT_STAT_7:
    case MOD_SLOT_STAT_8:
    case MOD_SLOT_STAT_9:
    case MOD_SLOT_STAT_10:
        {
            switch(type)
            {
            case ITEM_STAT_AGILITY:
            case ITEM_STAT_STRENGTH:
            case ITEM_STAT_INTELLECT:
            case ITEM_STAT_SPIRIT:
            case ITEM_STAT_STAMINA:
                TriggerModUpdate(UF_UTYPE_STATS);
                break;
            case ITEM_STAT_PHYSICAL_RESISTANCE:
            case ITEM_STAT_FIRE_RESISTANCE:
            case ITEM_STAT_FROST_RESISTANCE:
            case ITEM_STAT_HOLY_RESISTANCE:
            case ITEM_STAT_SHADOW_RESISTANCE:
            case ITEM_STAT_NATURE_RESISTANCE:
            case ITEM_STAT_ARCANE_RESISTANCE:
                TriggerModUpdate(UF_UTYPE_RESISTANCE);
                break;
            case ITEM_STAT_SPELL_HEALING_DONE:
            case ITEM_STAT_SPELL_DAMAGE_DONE:
            case ITEM_STAT_SPELL_POWER:
                TriggerModUpdate(UF_UTYPE_PLAYERDAMAGEMODS);
                break;
            case ITEM_STAT_HIT_RATING:
            case ITEM_STAT_CRITICAL_STRIKE_RATING:
            case ITEM_STAT_HIT_REDUCTION_RATING:
            case ITEM_STAT_RESILIENCE_RATING:
            case ITEM_STAT_CRITICAL_REDUCTION_RATING:
            case ITEM_STAT_HASTE_RATING:
            case ITEM_STAT_EXPERTISE_RATING:
            case ITEM_STAT_ARMOR_PENETRATION_RATING:
            case ITEM_STAT_SPELL_PENETRATION:
            case ITEM_STAT_MASTERY_RATING:
                TriggerModUpdate(UF_UTYPE_PLAYERRATINGS);
                break;
            case ITEM_STAT_MOD_DAMAGE_PHYSICAL:
                TriggerModUpdate(UF_UTYPE_ATTACKDAMAGE);
                break;
            case ITEM_STAT_MOD_DAMAGE_FIRE:
            case ITEM_STAT_MOD_DAMAGE_FROST:
            case ITEM_STAT_MOD_DAMAGE_HOLY:
            case ITEM_STAT_MOD_DAMAGE_SHADOW:
            case ITEM_STAT_MOD_DAMAGE_NATURE:
            case ITEM_STAT_MOD_DAMAGE_ARCANE:
                TriggerModUpdate(UF_UTYPE_PLAYERDAMAGEMODS);
                break;
            }
        }break;
        // Enchants passed through raw enchant ID
    case MOD_SLOT_PERM_ENCHANT:
    case MOD_SLOT_TEMP_ENCHANT:
    case MOD_SLOT_SOCKET_ENCHANT_1:
    case MOD_SLOT_SOCKET_ENCHANT_2:
    case MOD_SLOT_SOCKET_ENCHANT_3:
    case MOD_SLOT_BONUS_ENCHANT:
    case MOD_SLOT_PRISMATIC_ENCHANT:
    case MOD_SLOT_PROPRETY_ENCHANT_0:
    case MOD_SLOT_PROPRETY_ENCHANT_1:
    case MOD_SLOT_PROPRETY_ENCHANT_2:
    case MOD_SLOT_PROPRETY_ENCHANT_3:
    case MOD_SLOT_PROPRETY_ENCHANT_4:
        {
            if(val <= 0)
            {
                if(ItemPrototype *proto = sItemMgr.LookupEntry(val))
                    for(uint8 i = 0; i < 3; i++)
                        if(proto->Stats[i].Value)
                            ModifyBonuses(apply, guid, slot+1+i, proto->Stats[i].Type, proto->Stats[i].Value);
                break;
            }

            if(SpellItemEnchantEntry *enchant = dbcSpellItemEnchant.LookupEntry(val))
            {
                for(uint8 i = 0; i < 3; i++)
                {
                    if(enchant->type[i] == 0)
                        continue;

                    int32 value = randSuffixAmt ? float2int32( (((float)randSuffixAmt) * ((float)suffixSeed)) / 13340.0f ) : enchant->minPoints[i];
                    switch(enchant->type[i])
                    {
                    case 1: // Trigger on melee
                        break;
                    case 2: // Damage done modifier
                        ModifyBonuses(apply, guid, slot+1+i, ITEM_STAT_MOD_DAMAGE_PHYSICAL+enchant->spell[i], value);
                        break;
                    case 3: // cast spell
                        {
                            if(apply == true)
                            {
                                if(SpellEntry *sp = dbcSpell.LookupEntry(enchant->spell[i]))
                                {
                                    if(Spell *spell = new Spell(this, sp))
                                    {
                                        SpellCastTargets targets;
                                        targets.m_unitTarget = GetGUID();
                                        spell->castedItemId = WoWGuid(guid).getEntry();
                                        spell->prepare( &targets, true );
                                    }
                                }
                            } else RemoveAura(enchant->spell[i]);
                        }break;
                    case 4: // Resistance
                        ModifyBonuses(apply, guid, slot+1+i, ITEM_STAT_PHYSICAL_RESISTANCE+enchant->spell[i], value);
                        break;
                    case 5: // Stat
                        ModifyBonuses(apply, guid, slot+1+i, enchant->spell[i], value);
                        break;
                    case 6: // Rockbiter
                        break;
                    }
                }
            }
        }break;
    case MOD_SLOT_REFORGE_ENCHANT:
        {
            if(ItemReforgeEntry *entry = dbcItemReforge.LookupEntry(val))
            {
                float val = floor(((float)randSuffixAmt) * entry->sourceMultiplier);
                ModifyBonuses(apply, guid, slot+1, entry->sourceStat, float2int32(-val));
                val *= entry->finalMultiplier;
                ModifyBonuses(apply, guid, slot+2, entry->finalStat, float2int32(val));
            }
        }break;
    }
}

bool Player::CanSignCharter(Charter * charter, Player* requester)
{
    if(charter->CharterType == CHARTER_TYPE_GUILD && m_playerInfo->GuildId != 0)
        return false;
    if(m_playerInfo->charterId[charter->CharterType] != 0 || requester->GetTeam() != GetTeam())
        return false;
    return true;
}

void Player::SetShapeShift(uint8 ss)
{   // First update our shapeshift bytes
    uint8 old_ss = GetByte( UNIT_FIELD_BYTES_2, 3 );
    SetByte( UNIT_FIELD_BYTES_2, 3, ss );

    // Now look up our form for model generation
    uint32 model;
    if(ss == 0 || (model = GenerateShapeshiftModelId(ss)) == 0)
        model = GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    SetUInt32Value(UNIT_FIELD_DISPLAYID, model);

    // Trigger a mod type update
    TriggerModUpdate(UF_UTYPE_STATS);

    //remove auras that we should not have
    m_AuraInterface.UpdateShapeShiftAuras(old_ss, ss);

    // Set our forced power type(default is mana)
    SpellShapeshiftFormEntry *form = dbcSpellShapeshiftForm.LookupEntry(ss);
    SetPowerType(form ? form->forcedPowerType : myClass->powerType);
    // Trigger an attack time update based on form attack speed
    TriggerModUpdate(UF_UTYPE_ATTACKTIME);

    // apply any talents/spells we have that apply only in this form.
    std::set<uint32>::iterator itr;
    for( itr = m_spells.begin(); itr != m_spells.end(); itr++ )
    {
        SpellEntry *sp = dbcSpell.LookupEntry( *itr );
        if( sp == NULL)
            continue;
        if( sp->isSpellAppliedOnShapeshift() || sp->isPassiveSpell() )     // passive/talent
            if( sp->RequiredShapeShift && ((uint32)1 << (ss-1)) & sp->RequiredShapeShift )
                GetSpellInterface()->TriggerSpell(sp, this);
    }

    // now dummy-handler stupid hacky fixed shapeshift spells (leader of the pack, etc)
    for( itr = m_shapeShiftSpells.begin(); itr != m_shapeShiftSpells.end(); itr++)
    {
        SpellEntry *sp = dbcSpell.LookupEntry( *itr );
        if( sp == NULL)
            continue;

        if( sp->RequiredShapeShift && ((uint32)1 << (ss-1)) & sp->RequiredShapeShift )
            GetSpellInterface()->TriggerSpell(sp, this);
    }
}

uint32 Player::GetMainMeleeDamage(uint32 AP_owerride)
{
    float min_dmg, max_dmg;
/////////////////MAIN HAND
    float ap_bonus = (AP_owerride ? AP_owerride : CalculateAttackPower())/14000.0f;
    float delta = GetUInt32FloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS)-GetUInt32FloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG);
    if(IsInFeralForm())
    {
        float r = 0.f;
        uint32 lev = getLevel();
        if(GetShapeShift() == FORM_CAT)
            r = lev + delta + ap_bonus * 1000.0f;
        else r = lev + delta + ap_bonus * 2500.0f;
        min_dmg = r * 0.9f;
        max_dmg = r * 1.1f;
        return float2int32(std::max((min_dmg + max_dmg)/2.0f,0.0f));
    }
//////no druid ss
    ItemPrototype *proto = NULL;
    if(Item *it = GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
        proto = disarmed ? NULL : it->GetProto();

    float bonus = ap_bonus*float(proto ? proto->Delay : 2000), tmp = 1.0f;
    min_dmg = ((delta+bonus+(proto ? proto->minDamage : 0))*tmp) * 0.9f;
    max_dmg = ((delta+bonus+(proto ? proto->maxDamage : 0))*tmp) * 1.1f;
    return float2int32(std::max((min_dmg + max_dmg)/2.0f, 0.0f));
}

void Player::EventPortToGM(uint32 guid)
{
    if( !IsInWorld() )
        return;

    Player* p = objmgr.GetPlayer(guid);
    if( p == NULL )
        return;

    SafeTeleport(p->GetMapId(),p->GetInstanceID(),p->GetPosition());
}

void Player::SendAreaTriggerMessage(const char * message, ...)
{
    va_list ap;
    va_start(ap, message);
    char msg[500];
    vsnprintf(msg, 500, message, ap);
    va_end(ap);

    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 6 + strlen(msg));
    data << (uint32)0 << msg << (uint8)0x00;
    PushPacket(&data);
}

void Player::SoftDisconnect()
{
    if(IsInWorld())
        m_mapInstance->QueueSoftDisconnect(this);
    else m_session->Disconnect();
}

void Player::Possess(Unit* pTarget)
{
    if( m_CurrentCharm )
        return;

    m_CurrentCharm = pTarget;

    m_noInterrupt = true;
    SetUInt64Value(UNIT_FIELD_CHARM, pTarget->GetGUID());
    SetUInt64Value(PLAYER_FARSIGHT, pTarget->GetGUID());
    pTarget->GetMapInstance()->ChangeFarsightLocation(castPtr<Player>(this), pTarget, true);

    pTarget->SetUInt64Value(UNIT_FIELD_CHARMEDBY, GetGUID());
    pTarget->SetFactionTemplate(GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    pTarget->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    /* send "switch mover" packet */
    WorldPacket data1(SMSG_CLIENT_CONTROL_UPDATE, 10);      /* burlex: this should be renamed SMSG_SWITCH_ACTIVE_MOVER :P */
    data1 << pTarget->GetGUID() << uint8(1);
    PushPacket(&data1);

    return;
    /*std::list<uint32> avail_spells;
    for(AIInterface::AISpellMap::iterator itr = pTarget->GetAIInterface()->m_spells.begin(); itr != pTarget->GetAIInterface()->m_spells.end(); ++itr)
        avail_spells.push_back(itr->second->info->Id);
    std::list<uint32>::iterator itr = avail_spells.begin();

    WorldPacket data(SMSG_PET_SPELLS, pTarget->GetAIInterface()->m_spells.size() * 4 + 18);
    data << pTarget->GetGUID();
    data << uint16(0x0000);//unk1
    data << uint32(0x00000101);//unk2
    data << uint32(0x00000000);//unk3

    // First spell is attack.
    data << uint32(PET_SPELL_ATTACK);

    // Send the actionbar
    for(uint32 i = 1; i < 10; i++)
    {
        if(itr != avail_spells.end())
        {
            data << uint16((*itr)) << uint16(DEFAULT_SPELL_STATE);
            ++itr;
        }
        else
            data << uint16(0) << uint8(0) << uint8(i+5);
    }

    // Send the rest of the spells.
    data << uint8(avail_spells.size());
    for(itr = avail_spells.begin(); itr != avail_spells.end(); itr++)
        data << uint16(*itr) << uint16(DEFAULT_SPELL_STATE);

    SpellEntry* sp = NULL;
    data << uint8(avail_spells.size());
    for(itr = avail_spells.begin(); itr != avail_spells.end(); itr++)
    {
        data << uint32((sp = dbcSpell.LookupEntry(*itr))->Id);
        data << uint16(sp->Category);
        data << uint32(sp->RecoveryTime);
        data << uint32(sp->CategoryRecoveryTime);
    }

    PushPacket(&data);*/
}

void Player::UnPossess()
{
    if(!m_CurrentCharm)
        return;

    Unit* pTarget = m_CurrentCharm;
    m_CurrentCharm = NULL;

    m_noInterrupt = false;
    SetUInt64Value(PLAYER_FARSIGHT, 0);
    pTarget->GetMapInstance()->ChangeFarsightLocation(castPtr<Player>(this), pTarget, false);
    SetUInt64Value(UNIT_FIELD_CHARM, 0);
    pTarget->SetUInt64Value(UNIT_FIELD_CHARMEDBY, 0);

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);
    pTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE);

    /* send "switch mover" packet */
    WorldPacket data(SMSG_CLIENT_CONTROL_UPDATE, 10);
    data << GetGUID() << uint8(1);
    PushPacket(&data);

    data.Initialize(SMSG_PET_SPELLS);
    data << uint64(0);
    PushPacket(&data);
}

void Player::SummonRequest(WorldObject* Requestor, uint32 ZoneID, uint32 MapID, uint32 InstanceID, const LocationVector & Position)
{
    m_summonInstanceId = InstanceID;
    m_summonPos = Position;
    m_summoner = Requestor;
    m_summonMapId = MapID;

    WorldPacket data(SMSG_SUMMON_REQUEST, 16);
    data << Requestor->GetGUID() << ZoneID << uint32(120000);       // 2 minutes
    PushPacket(&data);
}

bool Player::HasSkillLine(uint16 skillLine)
{
    return m_skillIndexes.find(skillLine) != m_skillIndexes.end();
}

void Player::AddSkillLine(uint16 skillLine, uint16 step, uint16 skillMax, uint16 skillCurrent, uint16 skillModifier, uint16 bonusTalent)
{
    if(m_skillIndexes.find(skillLine) != m_skillIndexes.end())
        return;

    for(uint8 i = 0; i < MAX_PLAYER_SKILLS; i++)
    {
        uint32 field = i/2, offset = i&1;
        if(GetUInt16Value(PLAYER_SKILL_LINEID_0+field, offset))
            continue;

        m_skillIndexes.insert(std::make_pair(skillLine, i));
        SetUInt16Value(PLAYER_SKILL_LINEID_0 + field, offset, skillLine);
        SetUInt16Value(PLAYER_SKILL_STEP_0 + field, offset, step);
        SetUInt16Value(PLAYER_SKILL_RANK_0 + field, offset, skillCurrent);
        SetUInt16Value(PLAYER_SKILL_MAX_RANK_0 + field, offset, skillMax);
        SetUInt16Value(PLAYER_SKILL_MODIFIER_0 + field, offset, skillModifier);
        SetUInt16Value(PLAYER_SKILL_TALENT_0 + field, offset, bonusTalent);
        break;
    }
}

void Player::RemoveSkillLine(uint16 skillLine)
{
    Loki::AssocVector<uint16, SpellSet>::iterator skillItr;
    if((skillItr = m_spellsBySkill.find(skillLine)) != m_spellsBySkill.end())
    {
        SpellSet set(skillItr->second);
        m_spellsBySkill.erase(skillItr);

        for(SpellSet::iterator spell_itr = set.begin(); spell_itr != set.end(); spell_itr++)
            removeSpell(*spell_itr);
    }

    Loki::AssocVector<uint16, uint8>::iterator itr;
    if((itr = m_skillIndexes.find(skillLine)) == m_skillIndexes.end())
        return;

    uint32 field = itr->second/2, offset = itr->second&1;
    SetUInt16Value(PLAYER_SKILL_LINEID_0 + field, offset, 0);
    SetUInt16Value(PLAYER_SKILL_STEP_0 + field, offset, 0);
    SetUInt16Value(PLAYER_SKILL_RANK_0 + field, offset, 0);
    SetUInt16Value(PLAYER_SKILL_MAX_RANK_0 + field, offset, 0);
    SetUInt16Value(PLAYER_SKILL_MODIFIER_0 + field, offset, 0);
    SetUInt16Value(PLAYER_SKILL_TALENT_0 + field, offset, 0);
    m_skillIndexes.erase(itr);
}

void Player::UpdateSkillLine(uint16 skillLine, uint16 step, uint16 skillMax, bool forced)
{
    Loki::AssocVector<uint16, uint8>::iterator itr;
    if((itr = m_skillIndexes.find(skillLine)) == m_skillIndexes.end())
        return;

    uint32 field = itr->second/2, offset = itr->second&1;
    if(forced == false && GetUInt16Value(PLAYER_SKILL_STEP_0 + field, offset) >= step)
        return;

    SetUInt16Value(PLAYER_SKILL_STEP_0 + field, offset, step);
    SetUInt16Value(PLAYER_SKILL_MAX_RANK_0 + field, offset, skillMax);
}

void Player::ModSkillLineAmount(uint16 skillLine, int16 amount, bool bonus)
{
    Loki::AssocVector<uint16, uint8>::iterator itr;
    if((itr = m_skillIndexes.find(skillLine)) == m_skillIndexes.end())
        return;

    uint32 field = itr->second/2, offset = itr->second&1;
    if(bonus)
    {
        uint16 val = GetUInt16Value(PLAYER_SKILL_MODIFIER_0+field, offset);
        SetUInt16Value(PLAYER_SKILL_MODIFIER_0+field, offset, std::max<uint16>(0, val+amount));
        return;
    }

    uint16 val = GetUInt16Value(PLAYER_SKILL_RANK_0+field, offset);
    SetUInt16Value(PLAYER_SKILL_RANK_0+field, offset, std::max<uint16>(0, std::min<uint16>(val+amount, GetUInt16Value(PLAYER_SKILL_MAX_RANK_0+field, offset))));
}

uint16 Player::getSkillLineMax(uint16 skillLine)
{
    Loki::AssocVector<uint16, uint8>::iterator itr;
    if((itr = m_skillIndexes.find(skillLine)) == m_skillIndexes.end())
        return 0;
    uint32 field = itr->second/2, offset = itr->second&1;
    return GetUInt16Value(PLAYER_SKILL_MAX_RANK_0+field, offset);
}

uint16 Player::getSkillLineVal(uint16 skillLine, bool incBonus)
{
    Loki::AssocVector<uint16, uint8>::iterator itr;
    if((itr = m_skillIndexes.find(skillLine)) == m_skillIndexes.end())
        return 0;
    uint32 field = itr->second/2, offset = itr->second&1;
    uint16 ret = GetUInt16Value(PLAYER_SKILL_MAX_RANK_0+field, offset);
    if(incBonus) ret += GetUInt16Value(PLAYER_SKILL_MODIFIER_0+field, offset);
    return ret;
}

uint16 Player::getSkillLineStep(uint16 skillLine)
{
    Loki::AssocVector<uint16, uint8>::iterator itr;
    if((itr = m_skillIndexes.find(skillLine)) == m_skillIndexes.end())
        return 0;
    uint32 field = itr->second/2, offset = itr->second&1;
    return GetUInt16Value(PLAYER_SKILL_STEP_0+field, offset);
}

void Player::_UpdateMaxSkillCounts()
{

}

void Player::_AddLanguages(bool All)
{
    std::vector<uint32> langToAdd;
    uint8 race = std::min<uint8>(RACE_COUNT, getRace());
    uint32 teamLang = language_skills[GetTeam()];
    langToAdd.push_back(teamLang);
    if(language_skills[race] != teamLang)
        langToAdd.push_back(language_skills[race]);
    if(All)
    {
        for(uint8 i = 0; i < RACE_MAX; i++)
            if(uint32 skillId = language_skills[i])
                langToAdd.push_back(skillId);
    }

    while(!langToAdd.empty())
    {
        uint32 skillId = *langToAdd.begin();
        langToAdd.erase(langToAdd.begin());
        if(uint32 spell_id = ::GetSpellForLanguageSkill(skillId))
            addSpell(spell_id);

    }
}

float Player::GetSkillUpChance(uint32 id)
{

    return 0.f;
}

void Player::_AdvanceAllSkills(uint16 count, bool skipprof /* = false */, uint16 max /* = 0 */)
{

}

// Use instead of cold weather flying
bool CanFlyInCurrentZoneOrMap(Player *plr, uint32 ridingSkill)
{
    AreaTableEntry *area = dbcAreaTable.LookupEntry(plr->GetAreaId());
    if(area == NULL || (area->AreaFlags & AREA_CANNOT_FLY))
        return false; // can't fly in non-flying zones
    switch(plr->GetMapId())
    {
    case 530: return ridingSkill > 150; // We can fly in outlands all the time
    case 571:
        {
            if(plr->HasDummyAura(SPELL_HASH_COLD_WEATHER_FLYING) || plr->HasSpell(54197))
                return ridingSkill > 150;
        }break;
    case 0:
    case 1:
    case 646:
        {
            if(plr->HasDummyAura(SPELL_HASH_FLIGHT_MASTER_S_LICENSE) || plr->HasSpell(90267))
                return ridingSkill > 150;
        }break;
    }

    return false;
}

SpellEntry *Player::GetMountCapability(uint32 mountType)
{
    SpellEntry *ret = NULL;
    if(MountTypeEntry *mountTypeEntry = dbcMountType.LookupEntry(mountType))
    {
        bool canFly = false; // Flying capability check for use of flying mount capabilities
        uint32 ridingSkill = getSkillLineVal(SKILL_RIDING, true), i = (canFly = CanFlyInCurrentZoneOrMap(this, ridingSkill)) ? mountTypeEntry->maxCapability[0] : mountTypeEntry->maxCapability[1];
        while(i > 0)
        {
            i--;
            MountCapabilityEntry *entry = dbcMountCapability.LookupEntry(mountTypeEntry->MountCapability[i]);
            if(entry == NULL)
                continue;
            // We get a cutdown search if we can't fly, but we still need this check
            if(!canFly && entry->requiredRidingSkill > 150)
                continue;
            // Check to see if we can even use this capability
            if(ridingSkill < entry->requiredRidingSkill)
                continue;
            // Mount disables pitching
            if (m_movementInterface.hasFlag(MOVEMENTFLAG_FULL_SPEED_PITCHING) && !(entry->flags & 0x04))
                continue;
            // Mount cannot swim
            if (m_movementInterface.hasFlag(MOVEMENTFLAG_SWIMMING) && !(entry->flags & 0x08))
                continue;
            // Map requirements
            if (entry->requiredMap >= 0 && GetMapId() != (uint32)entry->requiredMap)
                continue;
            // Zone or area requirements
            if (entry->requiredArea && (entry->requiredArea != GetZoneId() && entry->requiredArea != GetAreaId()))
                continue;
            // Active aura requirements
            if (entry->requiredAura && !HasAura(entry->requiredAura))
                continue;
            // Spell requirements
            if (entry->requiredSpell && !HasSpell(entry->requiredSpell))
                continue;

            ret = dbcSpell.LookupEntry(entry->speedModSpell);
            break;
        }
    }
    return ret;
}

void Player::RecalculateHonor()
{

}

uint8 Player::GetTrainerSpellStatus(TrainerSpell *spell)
{
    if(HasSpell(spell->entry->Id))
        return TRAINER_SPELL_KNOWN;
    else if(spell->entry->HasEffect(SPELL_EFFECT_LEARN_SPELL))
    {
        bool hasSpell = true;
        for(uint8 i = 0; i < 3; i++)
        {
            if(spell->entry->EffectTriggerSpell[i] && !HasSpell(spell->entry->EffectTriggerSpell[i]))
            {
                hasSpell = false;
                break;
            }
        }

        if(hasSpell)
            return TRAINER_SPELL_KNOWN;
    }

    if(spell->reqSkill)
    {
        if(!HasSkillLine(spell->reqSkill))
            return TRAINER_SPELL_UNAVAILABLE;
        if(getSkillLineVal(spell->reqSkill) < spell->reqSkillValue)
            return TRAINER_SPELL_UNAVAILABLE;
    }

    if(spell->requiredLevel > getLevel())
        return TRAINER_SPELL_UNAVAILABLE;
    return TRAINER_SPELL_AVAILABLE;
}

void Player::EventGroupFullUpdate()
{
    if(m_playerInfo->m_Group)
    {
        if(m_playerInfo->m_Group->GetLeader() == m_playerInfo)
            m_playerInfo->m_Group->Update();
        m_playerInfo->m_Group->UpdateAllOutOfRangePlayersFor(m_playerInfo);
    }
}

bool Player::EjectFromInstance()
{

    return false;
}

void Player::TeleportToHomebind()
{
    SafeTeleport(m_bindData.mapId, 0, m_bindData.posX, m_bindData.posY, m_bindData.posZ, 0);
}

bool Player::HasQuestSpell(uint32 spellid) //Only for Cast Quests
{
    if (quest_spells.size()>0 && quest_spells.find(spellid) != quest_spells.end())
        return true;
    return false;
}
void Player::RemoveQuestSpell(uint32 spellid) //Only for Cast Quests
{
    if (quest_spells.size()>0)
        quest_spells.erase(spellid);
}

bool Player::HasQuestMob(uint32 entry) //Only for Kill Quests
{
    if (quest_mobs.size()>0 && quest_mobs.find(entry) != quest_mobs.end())
        return true;
    return false;
}
void Player::RemoveQuestMob(uint32 entry) //Only for Kill Quests
{
    if (quest_mobs.size()>0)
        quest_mobs.erase(entry);
}

PlayerInfo::PlayerInfo(WoWGuid _guid)
{
    charGuid = _guid;
    charName = "UNKNOWN";
    charRace = charClass = 0;
    charTeam = lastDeathState = 0;
    charAppearance = charAppearance2 = 0;
    charAppearance3 = charCustomizeFlags = 0;
    lastInstanceID = lastMapID = 0;
    lastPositionX = lastPositionY = lastPositionZ = lastOrientation = 0.f;
    lastOnline = 0; lastZone = lastLevel = 0;
    m_Group = NULL; subGroup = 0;
    GuildId = GuildRank = 0;
    m_loggedInPlayer = NULL;
    charterId[0] = charterId[1] = charterId[2] = charterId[3] = 0;

    achievementPoints = 0;
    professionId[0] = professionId[1] = 0;
    professionSkill[0] = professionSkill[1] = 0;
    professionRank[0] = professionRank[1] = 0;
}

PlayerInfo::~PlayerInfo()
{
    if(m_Group)
        m_Group->RemovePlayer(this);
}

void Player::FillMapWithSpellCategories(std::map<uint32, int32> *map)
{
    for(auto itr = m_spellCategories.begin(); itr != m_spellCategories.end(); itr++)
        (*map)[*itr] = 0;
}

void Player::AddShapeShiftSpell(uint32 id)
{
    SpellEntry * sp = dbcSpell.LookupEntry( id );
    m_shapeShiftSpells.insert( id );

    if( sp->RequiredShapeShift && ((uint32)1 << (GetShapeShift()-1)) & sp->RequiredShapeShift )
        GetSpellInterface()->TriggerSpell(sp, this);
}

void Player::RemoveShapeShiftSpell(uint32 id)
{
    m_shapeShiftSpells.erase( id );
    RemoveAura( id );
}

// COOLDOWNS
void Player::_Cooldown_Add(uint32 Type, uint32 Misc, time_t Time, uint32 SpellId, uint32 ItemId)
{
    PlayerCooldownMap::iterator itr = m_cooldownMap[Type].find( Misc );
    if( itr != m_cooldownMap[Type].end( ) )
    {
        if( itr->second.ExpireTime <= Time )
        {
            itr->second.ExpireTime = Time;
            itr->second.ItemId = ItemId;
            itr->second.SpellId = SpellId;
        }
    }
    else
    {
        PlayerCooldown cd;
        cd.ExpireTime = Time;
        cd.ItemId = ItemId;
        cd.SpellId = SpellId;

        m_cooldownMap[Type].insert( std::make_pair( Misc, cd ) );
    }

    sLog.Debug("Player","added cooldown for type %u misc %u time %u item %u spell %u", Type, Misc, Time - getMSTime(), ItemId, SpellId);
}

void Player::Cooldown_Add(SpellEntry * pSpell, Item* pItemCaster)
{
    if( m_cooldownCheat )
        return;

    time_t currTime = UNIXTIME;
    if( pSpell->CategoryRecoveryTime > 0 && pSpell->Category )
    {
        int32 recover = pSpell->CategoryRecoveryTime;
        if( pSpell->SpellGroupType )
        {
            SM_FIValue(SMT_COOLDOWN_DECREASE, &recover, pSpell->SpellGroupType);
            SM_PIValue(SMT_COOLDOWN_DECREASE, &recover, pSpell->SpellGroupType);
        }

        _Cooldown_Add( COOLDOWN_TYPE_CATEGORY, pSpell->Category, currTime+time_t(recover < 2000 ? 1 : (recover/1000)), pSpell->Id, pItemCaster ? pItemCaster->GetProto()->ItemId : 0 );
    }

    if( pSpell->RecoveryTime > 0 )
    {
        int32 recover = pSpell->RecoveryTime;
        if( pSpell->SpellGroupType )
        {
            SM_FIValue(SMT_COOLDOWN_DECREASE, &recover, pSpell->SpellGroupType);
            SM_PIValue(SMT_COOLDOWN_DECREASE, &recover, pSpell->SpellGroupType);
        }

        _Cooldown_Add( COOLDOWN_TYPE_SPELL, pSpell->Id, currTime+time_t(recover < 2000 ? 1 : (recover/1000)), pSpell->Id, pItemCaster ? pItemCaster->GetProto()->ItemId : 0 );
    }
}

void Player::Cooldown_AddStart(SpellEntry * pSpell)
{
    if( pSpell->StartRecoveryTime == 0 || m_cooldownCheat)
        return;

    int32 atime = pSpell->StartRecoveryTime;
    if( GetFloatValue(UNIT_MOD_CAST_SPEED) < 1.0f )
        atime *= GetFloatValue(UNIT_MOD_CAST_SPEED);

    if( pSpell->SpellGroupType )
    {
        SM_FIValue(SMT_GLOBAL_COOLDOWN, &atime, pSpell->SpellGroupType);
        SM_PIValue(SMT_GLOBAL_COOLDOWN, &atime, pSpell->SpellGroupType);
    }

    if( atime <= 0 )
        return;

    time_t expireTime = UNIXTIME + time_t(atime < 2000 ? 1 : (atime/1000));
    if( pSpell->StartRecoveryCategory )
        _Cooldown_Add( COOLDOWN_TYPE_CATEGORY, pSpell->StartRecoveryCategory, expireTime, pSpell->Id, 0 );
    else m_globalCooldown = UNIXTIME;
}

void Player::Cooldown_OnCancel(SpellEntry *pSpell)
{
    if( pSpell->StartRecoveryTime == 0 || m_cooldownCheat)
        return;

    int32 atime = pSpell->StartRecoveryTime;
    if( GetFloatValue(UNIT_MOD_CAST_SPEED) < 1.0f )
        atime *= GetFloatValue(UNIT_MOD_CAST_SPEED);
    if( atime <= 0 )
        return;

    if( pSpell->StartRecoveryCategory )
        m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(pSpell->StartRecoveryCategory);
    else m_globalCooldown = UNIXTIME;
}

bool Player::Cooldown_CanCast(SpellEntry * pSpell)
{
    if(pSpell == NULL)
        return false;

    if(m_cooldownCheat)
        return true;

    PlayerCooldownMap::iterator itr;
    time_t now = UNIXTIME;
    if( pSpell->Category )
    {
        itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find( pSpell->Category );
        if( itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end( ) )
        {
            if( now < itr->second.ExpireTime )
                return false;
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase( itr );
        }
    }

    itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find( pSpell->Id );
    if( itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end( ) )
    {
        if( now < itr->second.ExpireTime )
            return false;
        m_cooldownMap[COOLDOWN_TYPE_SPELL].erase( itr );
    }

    if( pSpell->StartRecoveryTime && m_globalCooldown )         /* gcd doesn't affect spells without a cooldown it seems */
    {
        if( now <= m_globalCooldown )
            return false;
        m_globalCooldown = 0;
    }

    return true;
}

void Player::Cooldown_AddItem(ItemPrototype * pProto, uint32 x)
{
    if( pProto->Spells[x].CategoryCooldown <= 0 && pProto->Spells[x].Cooldown <= 0 )
        return;

    time_t currTime = UNIXTIME;
    ItemPrototype::ItemSpell* isp = &pProto->Spells[x];
    if( isp->CategoryCooldown > 0)
        _Cooldown_Add( COOLDOWN_TYPE_CATEGORY, isp->Category, currTime+time_t(isp->CategoryCooldown < 2000 ? 1 : (isp->CategoryCooldown/1000)), isp->Id, pProto->ItemId );
    if( isp->Cooldown > 0 )
        _Cooldown_Add( COOLDOWN_TYPE_SPELL, isp->Id, currTime+time_t(isp->Cooldown < 2000 ? 1 : (isp->Cooldown/1000)), isp->Id, pProto->ItemId );
}

bool Player::Cooldown_CanCast(ItemPrototype * pProto, uint32 x)
{
    PlayerCooldownMap::iterator itr;
    ItemPrototype::ItemSpell* isp = &pProto->Spells[x];
    time_t now = UNIXTIME;

    if( isp->Category )
    {
        itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find( isp->Category );
        if( itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end( ) )
        {
            if( now <= itr->second.ExpireTime )
                return false;
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase( itr );
        }
    }

    itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find( isp->Id );
    if( itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end( ) )
    {
        if( now <= itr->second.ExpireTime )
            return false;
        m_cooldownMap[COOLDOWN_TYPE_SPELL].erase( itr );
    }

    return true;
}

void Player::_Disconnect()
{
    m_session->Disconnect();
}

/************************************************************************/
/* SOCIAL                                                               */
/************************************************************************/

void Player::Social_AddFriend(std::string name, std::string note)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);

    PlayerInfo *info = objmgr.GetPlayerInfoByName(name.c_str());
    if( info == NULL ) // lookup the player
    {
        data << uint8(FRIEND_NOT_FOUND);
        PushPacket(&data);
        return;
    }

    if( info == m_playerInfo ) // are we ourselves?
    {
        data << uint8(FRIEND_SELF) << info->charGuid;
        PushPacket(&data);
        return;
    }

    if( info->charTeam != m_playerInfo->charTeam ) // team check
    {
        data << uint8(FRIEND_ENEMY) << info->charGuid;
        PushPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    std::map<WoWGuid, std::string>::iterator itr;
    if((itr = m_friends.find(info->charGuid)) != m_friends.end())
    {
        data << uint8(FRIEND_ALREADY) << info->charGuid;
        PushPacket(&data);
        m_socialLock.Release();
        return;
    }

    Player *newfriend = info->m_loggedInPlayer;
    data << uint8(newfriend ? FRIEND_ADDED_ONLINE : FRIEND_ADDED_OFFLINE);
    data << info->charGuid;
    if( newfriend )
    {
        data << note;
        data << info->m_loggedInPlayer->GetChatTag();
        data << info->m_loggedInPlayer->GetZoneId();
        data << info->lastLevel;
        data << uint32(info->charClass);
    }

    m_friends.insert( std::make_pair(info->charGuid, note) );
    m_socialLock.Release();
    PushPacket(&data);

    // dump into the db
    CharacterDatabase.Execute("INSERT INTO social_friends VALUES(%u, %u, '%s')", GetLowGUID(), info->charGuid.getLow(), CharacterDatabase.EscapeString(std::string(note)).c_str());
}

void Player::Social_RemoveFriend(WoWGuid guid)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);
    if( guid == GetGUID() ) // are we ourselves?
    {
        data << uint8(FRIEND_SELF) << guid;
        PushPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    std::map<WoWGuid, std::string>::iterator itr;
    if((itr = m_friends.find(guid)) != m_friends.end())
        m_friends.erase(itr);
    m_socialLock.Release();

    data << uint8(FRIEND_REMOVED) << guid;
    PushPacket(&data);

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u AND friend_guid = %u", GetLowGUID(), guid.getLow());
}

void Player::Social_SetNote(WoWGuid guid, std::string note)
{
    m_socialLock.Acquire();
    std::map<WoWGuid, std::string>::iterator itr;
    if((itr = m_friends.find(guid)) == m_friends.end())
    {
        m_socialLock.Release();
        return;
    }
    itr->second = note;
    m_socialLock.Release();

    // Update db data
    CharacterDatabase.Execute("UPDATE social_friends SET note = \'%s\' WHERE character_guid = %u AND friend_guid = %u", CharacterDatabase.EscapeString(note).c_str(), GetLowGUID(), guid);
}

void Player::Social_AddIgnore(std::string name)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);

    // lookup the player
    PlayerInfo *info = objmgr.GetPlayerInfoByName(name.c_str());
    if( info == NULL )
    {
        data << uint8(FRIEND_IGNORE_NOT_FOUND);
        PushPacket(&data);
        return;
    }

    // are we ourselves?
    if( info == m_playerInfo )
    {
        data << uint8(FRIEND_IGNORE_SELF) << info->charGuid;
        PushPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    std::map<WoWGuid, std::string>::iterator itr;
    if((itr = m_ignores.find(info->charGuid)) != m_ignores.end())
    {
        data << uint8(FRIEND_IGNORE_ALREADY) << info->charGuid;
        PushPacket(&data);
        m_socialLock.Release();
        return;
    }

    data << uint8(FRIEND_IGNORE_ADDED) << info->charGuid;
    m_ignores.insert(std::make_pair(info->charGuid, "IGNORE"));

    m_socialLock.Release();
    PushPacket(&data);

    // dump into db
    CharacterDatabase.Execute("INSERT INTO social_ignores VALUES(%u, %u)", GetLowGUID(), info->charGuid.getLow());
}

void Player::Social_RemoveIgnore(WoWGuid guid)
{
    WorldPacket data(SMSG_FRIEND_STATUS, 10);

    // are we ourselves?
    if( guid == GetGUID() )
    {
        data << uint8(FRIEND_IGNORE_SELF) << GetGUID();
        PushPacket(&data);
        return;
    }

    m_socialLock.Acquire();
    std::map<WoWGuid, std::string>::iterator itr;
    if((itr = m_ignores.find(guid)) != m_ignores.end())
        m_ignores.erase(itr);

    data << uint8(FRIEND_IGNORE_REMOVED);
    data << uint64(guid);

    m_socialLock.Release();

    PushPacket(&data);

    // remove from the db
    CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u AND ignore_guid = %u", GetLowGUID(), guid.getLow());
}

bool Player::Social_IsIgnoring(WoWGuid guid)
{
    m_socialLock.Acquire();
    bool res = m_ignores.find(guid) != m_ignores.end();
    m_socialLock.Release();
    return res;
}

void Player::Social_SendFriendList(uint32 flag)
{
    m_socialLock.Acquire();
    uint32 listFlags = 0, count = 0;
    std::map<WoWGuid, std::string>::iterator itr;
    WorldPacket data(SMSG_CONTACT_LIST, 500);
    data << flag << count;
    if((listFlags=0x01) & flag)
    {
        for( itr = m_friends.begin(); itr != m_friends.end(); itr++ )
        {
            data << itr->first << listFlags << itr->second;
            if(0x01 & 0x01)
            {
                Player *plr = NULL;
                if(PlayerInfo *info = objmgr.GetPlayerInfo(itr->first))
                    plr = info->m_loggedInPlayer;
                data << uint8(plr ? plr->GetChatTag() : 0);
                if(plr)
                {
                    data << uint32(plr->GetZoneId());
                    data << uint32(plr->getLevel());
                    data << uint32(plr->getClass());
                }
            }
            count++;
        }
    }
    if((listFlags=0x02) & flag)
    {
        for( itr = m_ignores.begin(); itr != m_ignores.end(); itr++ )
        {
            data << itr->first << listFlags << itr->second;
            count++;
        }
    }
    if((listFlags=0x04) & flag)
    {
        for( itr = m_mutes.begin(); itr != m_mutes.end(); itr++ )
        {
            data << itr->first << listFlags << itr->second;
            count++;
        }
    }
    data.put<uint32>(4, count);
    m_socialLock.Release();
    PushPacket(&data, true);
}

void Player::GenerateLoot()
{
    // Use calldown
    Unit::GenerateLoot();
    if(!IsInWorld())
        return;
    if(m_mapInstance->CanLootPlayers(this))
        return;

    Corpse *pCorpse = NULL;
    if(pCorpse = getMyCorpse())
    {
        // default gold
        pCorpse->ClearLoot();
        pCorpse->GetLoot()->gold = 500;
    }

    TRIGGER_INSTANCE_EVENT( m_mapInstance, OnPlayerLootGen )( this, pCorpse );
}

uint32 Player::GetMaxPersonalRating(bool Ignore2v2)
{
    ASSERT(m_playerInfo != NULL);

    uint32 maxrating = 0;
    return maxrating;
}

void Player::FullHPMP()
{
    if( isDead() )
        ResurrectPlayer();

    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));
    SetPower(POWER_TYPE_ENERGY, GetMaxPower(POWER_TYPE_ENERGY));
}

void Player::SetKnownTitle( int32 title, bool set )
{
    CharTitleEntry *entry = dbcCharTitle.LookupEntry(title);
    if(entry == NULL || HasKnownTitleByIndex(entry->bit_index))
        return;

    if(set == false && entry->bit_index == GetUInt32Value(PLAYER_CHOSEN_TITLE))
        SetUInt32Value(PLAYER_CHOSEN_TITLE, 0);

    uint32 field = PLAYER__FIELD_KNOWN_TITLES+(entry->bit_index / 32);
    uint32 flag = uint32(1 << (entry->bit_index % 32));

    if( set ) SetFlag(field, flag);
    else RemoveFlag(field, flag);

    WorldPacket data( SMSG_TITLE_EARNED, 8 );
    data << uint32( entry->bit_index ) << uint32( set ? 1 : 0 );
    PushPacket( &data );

    if(set && GetUInt32Value(PLAYER_CHOSEN_TITLE) == 0)
        SetUInt32Value(PLAYER_CHOSEN_TITLE, entry->bit_index);

    AchieveMgr.UpdateCriteriaValue(this, ACHIEVEMENT_CRITERIA_TYPE_HAS_TITLE, set ? 1 : 0, title);
}

void Player::RetroactiveCompleteQuests()
{

}

void Player::GroupUninvite(Player* targetPlayer, PlayerInfo *targetInfo)
{
    if ( targetPlayer == NULL && targetInfo == NULL )
    {
        GetSession()->SendPartyCommandResult(this, 0, "", ERR_PARTY_CANNOT_FIND);
        return;
    }

    if ( !InGroup() || targetInfo->m_Group != GetGroup() )
    {
        GetSession()->SendPartyCommandResult(this, 0, "", ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
        return;
    }

    if ( !IsGroupLeader() || targetInfo->m_Group->HasFlag(GROUP_FLAG_BATTLEGROUND_GROUP) )  // bg group
    {
        if(this != targetPlayer)
        {
            GetSession()->SendPartyCommandResult(this, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
            return;
        }
    }

    if(Group *group = GetGroup())
        group->RemovePlayer(targetInfo);
}

uint16 Player::FindQuestSlot( uint32 questid )
{
    for(uint16 i = 0; i < 50; i++)
        if( (GetUInt32Value(PLAYER_QUEST_LOG + i * 5)) == questid )
            return i;

    return 50;
}

uint32 Player::GetTotalItemLevel()
{
    uint32 itemCount = 0, totalItemLevel = 0;
    if(!m_inventory.FillItemLevelData(itemCount, totalItemLevel, true))
        return 0;
    return totalItemLevel;
}

uint32 Player::GetAverageItemLevel(bool skipmissing)
{
    uint32 itemCount = 0, totalItemLevel = 0;
    if(!m_inventory.FillItemLevelData(itemCount, totalItemLevel, true))
        return 0;
    return totalItemLevel/itemCount;
}

// Crow: Spellcheck explanation: Some spells have the required part, but they don't really need it.
uint32 Player::GetSpellForShapeshiftForm(uint8 ss, bool spellchecks)
{
    if(ss < 1)
        return 0;

    switch(ss)
    {
    case FORM_CAT:
        return 768;
        break;

    case FORM_TRAVEL:
        if(!spellchecks)
            return 783;
        break;

    case FORM_BEAR:
        return 5487;
        break;

    case FORM_DIREBEAR:
        if(!spellchecks)
            return 9634;
        break;

    case FORM_MOONKIN:
        if(!spellchecks)
            return 24858;
        break;

    case FORM_TREE:
        if(!spellchecks)
            return 33891;
        break;

    case FORM_GHOSTWOLF:
        if(!spellchecks)
            return 0;
        break;

    default:
        return 0;
        break;
    }
    return 0;
}

uint8 Player::GetChatTag() const
{
    if(false)
        return 4;
    else if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
        return 3;
    else if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
        return 1;
    return 0;
}

uint8 Player::GetGuildMemberFlags()
{
    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DND))
        return 0x04;
    else if(HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
        return 0x02;
    return 0x01;
}

uint32 Player::GenerateShapeshiftModelId(uint32 form)
{
    SpellShapeshiftFormEntry *ssEntry = dbcSpellShapeshiftForm.LookupEntry(form);
    if(ssEntry && ssEntry->modelID_H && GetTeam() == TEAM_HORDE)
        return ssEntry->modelID_H;

    switch(form)
    {
    case FORM_CAT:
        {
            if (GetTeam() == TEAM_ALLIANCE) // Based on Hair color
            {
                uint8 hairColor = GetByte(PLAYER_BYTES, 3);
                switch (hairColor)
                {
                case 7: // Violet
                case 8:
                    return 29405;
                case 3: // Light Blue
                    return 29406;
                case 0: // Green
                case 1: // Light Green
                case 2: // Dark Green
                    return 29407;
                case 4: // White
                    return 29408;
                default: // original - Dark Blue
                    return 892;
                }
            }
            else if (getRace() == RACE_TAUREN) // Based on Skin color
            {
                uint8 skinColor = GetByte(PLAYER_BYTES, 0);
                if (getGender() == 0) // Male
                {
                    switch(skinColor)
                    {
                    case 12: // White
                    case 13:
                    case 14:
                    case 18: // Completly White
                        return 29409;
                    case 9: // Light Brown
                    case 10:
                    case 11:
                        return 29410;
                    case 6: // Brown
                    case 7:
                    case 8:
                        return 29411;
                    case 0: // Dark
                    case 1:
                    case 2:
                    case 3: // Dark Grey
                    case 4:
                    case 5:
                        return 29412;
                    default: // original - Grey
                        return 8571;
                    }
                }
                else switch (skinColor) // Female
                {
                case 10: // White
                    return 29409;
                case 6: // Light Brown
                case 7:
                    return 29410;
                case 4: // Brown
                case 5:
                    return 29411;
                case 0: // Dark
                case 1:
                case 2:
                case 3:
                    return 29412;
                default: // original - Grey
                    return 8571;
                }
            }
        }break;
    case FORM_DIREBEAR:
    case FORM_BEAR:
        {
            if (getRace() == RACE_NIGHTELF)
            {
                // Based on Hair color
                uint8 hairColor = GetByte(PLAYER_BYTES, 3);
                switch (hairColor)
                {
                case 0: // Green
                case 1: // Light Green
                case 2: // Dark Green
                    return 29413; // 29415?
                case 6: // Dark Blue
                    return 29414;
                case 4: // White
                    return 29416;
                case 3: // Light Blue
                    return 29417;
                default: // original - Violet
                    return 2281;
                }
            }
            else if (getRace() == RACE_TAUREN)
            {
                // Based on Skin color
                uint8 skinColor = GetByte(PLAYER_BYTES, 0);
                if (getGender() == 0) // Male
                {
                    switch (skinColor)
                    {
                    case 0: // Dark (Black)
                    case 1:
                    case 2:
                        return 29418;
                    case 3: // White
                    case 4:
                    case 5:
                    case 12:
                    case 13:
                    case 14:
                        return 29419;
                    case 9: // Light Brown/Grey
                    case 10:
                    case 11:
                    case 15:
                    case 16:
                    case 17:
                        return 29420;
                    case 18: // Completly White
                        return 29421;
                    default: // original - Brown
                        return 2289;
                    }
                }
                else switch (skinColor) // Female
                {
                case 0: // Dark (Black)
                case 1:
                    return 29418;
                case 2: // White
                case 3:
                    return 29419;
                case 6: // Light Brown/Grey
                case 7:
                case 8:
                case 9:
                    return 29420;
                case 10: // Completly White
                    return 29421;
                default: // original - Brown
                    return 2289;
                }
            }
        }break;
    case FORM_FLIGHT:
        return GetTeam() ? 20872 : 20857;
    case FORM_MOONKIN:
        return GetTeam() ? 15375 : 15374;
    case FORM_SWIFT:
        return GetTeam() ? 21244 : 21243;
    case FORM_TRAVEL:
        return 632;
    case FORM_AQUA:
        return 2428;
    case FORM_GHOUL:
        return 24994;
    case FORM_CREATUREBEAR:
        return 902;
    case FORM_GHOSTWOLF:
        return 4613;
    case FORM_DEMON:
        return 25277;
    case FORM_MASTER_ANGLER:
        return 15234;
    case FORM_TREE:
        return 864;
    case FORM_SPIRITOFREDEMPTION:
        return 16031;
    }
    return ssEntry ? ssEntry->modelID_A : 0;
}

void Player::StartQuest(uint32 Id)
{
    Quest * qst = sQuestMgr.GetQuestPointer(Id);
    if(!qst)
        return;
    
    if( GetQuestLogForEntry( qst->id ) || HasFinishedQuest(qst->id))
        return;

    int32 log_slot = GetOpenQuestSlot();

    if (log_slot == -1)
    {
        sQuestMgr.SendQuestLogFull(this);
        return;
    }

    QuestLogEntry *qle = new QuestLogEntry();
    qle->Init(qst, this, log_slot);
    qle->UpdatePlayerFields();

    TRIGGER_QUEST_EVENT(Id, OnQuestStart)(this, qle);

    sQuestMgr.OnQuestAccepted(this,qst,NULL);
}

DrunkenState Player::GetDrunkenstateByValue(uint16 value)
{
    /*if(value >= 25560)
        return DRUNKEN_VOMIT;*/
    if(value >= 23000)
        return DRUNKEN_SMASHED;
    if(value >= 12800)
        return DRUNKEN_DRUNK;
    if(value & 0xFFFE)
        return DRUNKEN_TIPSY;
    return DRUNKEN_SOBER;
}

void Player::EventDrunkenVomit()
{
    static SpellEntry *vomitEffect = dbcSpell.LookupEntry(67468);
    if(vomitEffect) GetSpellInterface()->TriggerSpell(vomitEffect, this);
    m_drunk -= 2560;
}
