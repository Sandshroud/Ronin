/***
 * Demonstrike Core
 */

#include "StdAfx.h"

Vehicle::Vehicle(CreatureData *data, uint64 guid) : Creature(data, guid)
{
    m_ppassengerCount = NULL;
    m_maxPassengers = NULL;
    m_seatSlotMax = NULL;
    Initialised = false;
    m_CreatedFromSpell = false;
    m_CastSpellOnMount = NULL;
}

Vehicle::~Vehicle()
{

}

void Vehicle::Init()
{
    Creature::Init();
}

void Vehicle::Destruct()
{
    m_ppassengerCount = NULL;
    if( IsInWorld() )
        RemoveFromWorld(false, true);
    Creature::Destruct();
}

void Vehicle::InitSeats(uint32 vehicleEntry, Player* pRider)
{
    m_maxPassengers = m_seatSlotMax = 0;
    if((vehicleData = dbcVehicle.LookupEntry((m_vehicleEntry=vehicleEntry))) == NULL)
    {
        sLog.outDebug("Attempted to create non-existant vehicle %u.", vehicleEntry);
        return;
    }

    for( uint32 i = 0; i < 8; i++ )
    {
        if( vehicleData->m_seatID[i] )
        {
            m_vehicleSeats[i] = dbcVehicleSeat.LookupEntry( vehicleData->m_seatID[i] );
            m_seatSlotMax = i + 1;

            if(m_vehicleSeats[i]->IsUsable())
            {
                seatisusable[i] = true;
                ++m_maxPassengers;
            }
        }
    }

    Initialised = true;

    if(!m_maxPassengers || !m_seatSlotMax || pRider == NULL)
        return;

    AddPassenger( pRider );
}

void Vehicle::InstallAccessories()
{
    CreatureVehicleData* acc = CreatureVehicleDataStorage.LookupEntry(GetEntry());
    if(acc == NULL)
    {
        sLog.outDetail("Vehicle %u has no accessories.", GetEntry());
        return;
    }

    MapMgr* map = (GetMapMgr() ? GetMapMgr() : sInstanceMgr.GetMapMgr(GetMapId()));
    if(map == NULL) // Shouldn't ever really happen.
        return;

    for(int i = 0; i < 8; i++)
    {
        SeatInfo seatinfo = acc->seats[i];
        if(!seatinfo.accessoryentry || (seatinfo.accessoryentry == GetEntry()))
            continue;

        if(m_vehicleSeats[i] == NULL)
        {
            sLog.outDetail("No seatmap for selected seat.\n");
            continue;
        }

        // Load the Proto
        CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(seatinfo.accessoryentry);
        if(ctrData)
        {
            sLog.outError("No proto/info for vehicle accessory %u in vehicle %u", seatinfo.accessoryentry, GetEntry());
            continue;
        }


        // Remove any passengers.
        if(m_passengers[i])
            RemovePassenger(m_passengers[i]);

        Creature *passenger = ((ctrData->Vehicle_entry > 0) ? map->CreateVehicle(seatinfo.accessoryentry) : map->CreateCreature(seatinfo.accessoryentry));
        if(passenger == NULL)
            continue;

        float offsetX = m_vehicleSeats[i]->m_attachmentOffsetX, offsetY = m_vehicleSeats[i]->m_attachmentOffsetY, offsetZ = m_vehicleSeats[i]->m_attachmentOffsetZ;
        if(!passenger->Load((IsInInstance() ? map->iInstanceMode : MODE_5PLAYER_NORMAL), GetPositionX()+offsetX, GetPositionY()+offsetY, GetPositionZ()+offsetZ))
        {
            delete passenger;
            continue;
        }

        passenger->Init();
        passenger->GetMovementInterface()->LockTransportData();
        passenger->GetMovementInterface()->SetTransportData(GetGUID(), ctrData->Vehicle_entry, offsetX, offsetY, offsetZ, 0.0f, i);

        if(passenger->IsVehicle())
        {
            castPtr<Vehicle>(passenger)->InitSeats(ctrData->Vehicle_entry);
            if(passenger->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK))
                passenger->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK); // Accessory
        }

        AddPassenger(passenger, i, true);
        passenger->PushToWorld(map);
    }
}

bool Vehicle::Load(uint32 mode, float x, float y, float z, float o /* = 0.0f */)
{
    if((m_vehicleEntry = _creatureData->Vehicle_entry) == -1)
    {
        m_vehicleEntry = 124;
        sLog.outDebug("Attempted to create vehicle %u with invalid vehicle_entry, defaulting to 124, check your creature_proto table.", _creatureData->Entry);
    }

    m_maxPassengers = m_seatSlotMax = 0;
    if((vehicleData = dbcVehicle.LookupEntry( m_vehicleEntry )) == NULL)
    {
        sLog.outDebug("Attempted to create non-existant vehicle %u.", GetVehicleEntry());
        return false;
    }

    for( uint32 i = 0; i < 8; i++ )
    {
        if( vehicleData->m_seatID[i] )
        {
            m_vehicleSeats[i] = dbcVehicleSeat.LookupEntry( vehicleData->m_seatID[i] );
            m_seatSlotMax = i+1;

            if(m_vehicleSeats[i]->IsUsable())
            {
                seatisusable[i] = true;
                ++m_maxPassengers;
            }
        }
    }

    Initialised = true;

    return Creature::Load(mode, x, y, z, o);
}

bool Vehicle::Load(CreatureSpawn *spawn, uint32 mode)
{
    if((m_vehicleEntry = _creatureData->Vehicle_entry) == -1)
    {
        m_vehicleEntry = 124;
        sLog.outDebug("Attempted to create vehicle %u with invalid vehicle_entry, defaulting to 124, check your creature_proto table.", _creatureData->Entry);
    }

    m_maxPassengers = m_seatSlotMax = 0;
    if((vehicleData = dbcVehicle.LookupEntry( m_vehicleEntry )) == NULL)
    {
        sLog.outDebug("Attempted to create non-existant vehicle %u.", GetVehicleEntry());
        return false;
    }

    for( uint32 i = 0; i < 8; i++ )
    {
        if( vehicleData->m_seatID[i] )
        {
            m_vehicleSeats[i] = dbcVehicleSeat.LookupEntry( vehicleData->m_seatID[i] );
            m_seatSlotMax = i + 1;

            if(m_vehicleSeats[i]->IsUsable())
            {
                seatisusable[i] = true;
                ++m_maxPassengers;
            }
        }
    }
    Initialised = true;

    return Creature::Load(spawn, mode);
}

void Vehicle::OnPushToWorld()
{
    InstallAccessories();
}

void Vehicle::SendSpells(uint32 entry, Player* plr)
{
    CreatureVehicleData* acc = CreatureVehicleDataStorage.LookupEntry(GetEntry());
    if(!acc)
    {
        WorldPacket data(SMSG_PET_SPELLS, 12);
        data << uint64(0);
        data << uint32(0);
        plr->GetSession()->SendPacket(&data);
        return;
    }

    uint8 count = 0;

    WorldPacket data(SMSG_PET_SPELLS, 60);
    data << uint64(GetGUID());
    data << uint16(0);
    data << uint32(0);
    data << uint32(0x00000101);

    for (uint32 i = 0; i < 6; i++)
    {
        uint32 spellId = acc->VehicleSpells[i];
        if (!spellId)
            continue;

        SpellEntry *spellInfo = dbcSpell.LookupEntry( spellId );
        if (!spellInfo)
            continue;
        if(spellInfo->isPassiveSpell())
        {
            CastSpell(GetGUID(), spellId, true);
            data << uint16(0) << uint8(0) << uint8(i+8);
        }
        else
        {
            if(spellInfo->SpellDifficulty && GetMapMgr()->pInstance)
            {
                SpellDifficultyEntry * sd = dbcSpellDifficulty.LookupEntry(spellInfo->SpellDifficulty);
                if( sd->SpellId[GetMapMgr()->iInstanceMode] == 0 )
                {
                    uint32 mode = 0;
                    if( GetMapMgr()->iInstanceMode == 3 )
                        mode = 1;

                    if( sd->SpellId[mode] == 0 )
                        spellId = sd->SpellId[0];
                }
                else
                    spellId = sd->SpellId[GetMapMgr()->iInstanceMode];
            }
            switch(spellId)
            {
            case 62286: // Tar
            case 62308: // Ram
            case 62522: // Electroshock!
                {
                    if(IsInInstance())
                    {
                        if(dbcMap.LookupEntry(GetMapId())->IsRaid()) // Difficulty Check
                        {
                            if(plr->iRaidType > MODE_10PLAYER_NORMAL)
                            {
                                data << uint32(0);
                                continue;
                            }
                        }
                    }
                    data << uint32(MAKE_ACTION_BUTTON(spellId,i+8));
                    ++count;
                }break;

            default:
                data << uint32(MAKE_ACTION_BUTTON(spellId,i+8));
                ++count;
                break;
            }
        }
    }

    for(uint8 i = 6; i < 10; i++)
    {
        data << uint16(0) << uint8(0) << uint8(i+8);
    }

    data << count; // spells count
    data << uint8(0);
    plr->GetSession()->SendPacket(&data);
}

void Vehicle::Despawn(uint32 delay, uint32 respawntime)
{
    if(delay)
    {
        sEventMgr.AddEvent(this, &Vehicle::Despawn, (uint32)0, respawntime, EVENT_VEHICLE_RESPAWN, delay, 1,0);
        return;
    }

    if(!IsInWorld())
        return;

    if(respawntime)
    {
        for(int i = 0; i < 8; i++)
        {
            if(m_passengers[i] != NULL)
            {
                if(m_passengers[i]->IsPlayer())
                    // Remove any passengers
                    RemovePassenger(m_passengers[i]);
                else
                    m_passengers[i]->Destruct();
            }
        }

        /* get the cell with our SPAWN location. if we've moved cell this might break :P */
        MapCell * pCell = m_mapMgr->GetCellByCoords(m_spawnLocation.x, m_spawnLocation.y);
        if(!pCell)
            pCell = m_mapCell;

        ASSERT(pCell);
        pCell->_respawnObjects.insert(this);
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(m_mapMgr, &MapMgr::EventRespawnVehicle, this, pCell, EVENT_VEHICLE_RESPAWN, respawntime, 1, 0);
        Unit::RemoveFromWorld(false);
        m_position = m_spawnLocation;
        m_respawnCell=pCell;
    }
    else
    {
        Unit::RemoveFromWorld(true);
        SafeDelete();
    }
}

void Vehicle::Update(uint32 time)
{
    Creature::Update(time);
}

void Vehicle::SafeDelete()
{
    for(int i = 0; i < 8; i++)
    {
        if(m_passengers[i] != NULL)
        {
            if(m_passengers[i]->IsPlayer())
                // Remove any passengers
                RemovePassenger(m_passengers[i]);
            else
                m_passengers[i]->Destruct();
        }
    }

    sEventMgr.RemoveEvents(this);
    sEventMgr.AddEvent(this, &Vehicle::DeleteMe, EVENT_VEHICLE_SAFE_DELETE, 1000, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Vehicle::DeleteMe()
{
    if(IsInWorld())
        RemoveFromWorld(false, true);

    Destruct();
}

void Vehicle::AddPassenger(Unit* pPassenger, int8 requestedseat /*= -1*/, bool force /*= false*/)
{
    if(!m_maxPassengers || !m_seatSlotMax)
        return; // Slave vehicle.

    if(pPassenger->GetVehicle())
        pPassenger->GetVehicle()->RemovePassenger(pPassenger);

    sLog.outDebug("AddPassenger: Max Vehicle Slot: %u, Max Passengers: %u\n", m_seatSlotMax, m_maxPassengers);

    if(requestedseat > -1)
    {
        if(force)
        {
            if(m_vehicleSeats[requestedseat]) // Slot available?
            {
                if(pPassenger->IsPlayer() && seatisusable[requestedseat] == false)
                    return;

                if(m_passengers[requestedseat])
                    RemovePassenger(m_passengers[requestedseat]);

                _AddToSlot(pPassenger, requestedseat);
                return;
            }
        }
        else
        {
            if(!m_passengers[requestedseat] && m_vehicleSeats[requestedseat] && seatisusable[requestedseat] == true) // Slot available?
            {
                _AddToSlot(pPassenger, requestedseat );
                return;
            }
        }
    }
    else
    {   // Find us a slot!
        for(uint8 i = 0; i < m_seatSlotMax; i++)
        {
            if(pPassenger->IsPlayer())
            {
                if(!m_passengers[i] && m_vehicleSeats[i] && (seatisusable[i] == true)) // Found a slot
                {
                    _AddToSlot(pPassenger, i );
                    return;
                }
            }
            else
            {
                if(!m_passengers[i] && m_vehicleSeats[i])
                {
                    _AddToSlot(pPassenger, i );
                    return;
                }
            }
        }
    }
}

void Vehicle::RemovePassenger(Unit* pPassenger)
{
    if(pPassenger == NULL) // We have enough problems that we need to do this :(
        return;

    uint8 slot = pPassenger->GetSeatID();

    pPassenger->SetVehicle(NULL);
    pPassenger->SetSeatID(NULL);

    pPassenger->RemoveFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_UNKNOWN_5 | UNIT_FLAG_PREPARATION | UNIT_FLAG_NOT_SELECTABLE));
    if( pPassenger->IsPlayer() && castPtr<Player>(pPassenger)->m_MountSpellId != m_mountSpell )
        pPassenger->RemoveAura(castPtr<Player>(pPassenger)->m_MountSpellId);

    if( m_mountSpell )
        pPassenger->RemoveAura( m_mountSpell );
    if( m_CastSpellOnMount )
        pPassenger->RemoveAura( m_CastSpellOnMount );

    if(pPassenger->IsPlayer())
        castPtr<Player>(pPassenger)->GetMovementInterface()->setRooted(false);

    WorldPacket data(SMSG_MONSTER_MOVE, 85);
    data << pPassenger->GetGUID();           // PlayerGUID
    data << uint8(0x40);                        // Unk - blizz uses 0x40
    data.appendvector(pPassenger->GetPosition());// Player Position xyz
    data << getMSTime();                        // Timestamp
    data << uint8(0x4);                         // Flags
    data << pPassenger->GetOrientation();       // Orientation
    data << uint32(MOVEMENTFLAG_FLYING);        // MovementFlags
    data << uint32(0);                          // MovementTime
    data << uint32(1);                          // Pointcount
    data.appendvector(GetPosition());           // Vehicle Position xyz
    SendMessageToSet(&data, false);

    pPassenger->GetMovementInterface()->removeServerFlag(MOVEMENTFLAG_TOGGLE_NO_GRAVITY);
    pPassenger->GetMovementInterface()->ClearTransportData();
    pPassenger->GetMovementInterface()->UnlockTransportData();

    if(pPassenger->IsPlayer())
    {
        Player* plr = castPtr<Player>(pPassenger);
        if(plr == GetControllingUnit())
        {
            plr->m_CurrentCharm = NULL;
            data.Initialize(SMSG_CLIENT_CONTROL_UPDATE);
            data << GetGUID() << (uint8)0;
            plr->GetSession()->SendPacket(&data);
        }
        RemoveFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PLAYER_CONTROLLED));

        plr->SetPlayerStatus(TRANSFER_PENDING); // We get an ack later, if we don't set this now, we get disconnected.
        sEventMgr.AddEvent(plr, &Player::CheckPlayerStatus, (uint8)TRANSFER_PENDING, EVENT_PLAYER_CHECK_STATUS_Transfer, 5000, 0, 0);
        plr->SetPosition(GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());

        data.Initialize(MSG_MOVE_TELEPORT_ACK);
        data << plr->GetGUID();
        data << uint32(0);
        data << uint32(MOVEMENTFLAG_FLYING);
        data << uint16(0x40);
        data << getMSTime();
        data << GetPositionX();
        data << GetPositionY();
        data << GetPositionZ();
        data << GetOrientation();
        data << uint32(0);
        plr->GetSession()->SendPacket(&data);

        plr->SetUInt64Value( PLAYER_FARSIGHT, 0 );

        data.Initialize(SMSG_PET_DISMISS_SOUND);
        data << uint32(m_vehicleSeats[slot]->m_exitUISoundID);
        data.appendvector(plr->GetPosition());
        plr->GetSession()->SendPacket(&data);

        data.Initialize(SMSG_PET_SPELLS);
        data << uint64(0);
        data << uint32(0);
        plr->GetSession()->SendPacket(&data);

        CreatureVehicleData* vehicledata = CreatureVehicleDataStorage.LookupEntry(GetEntry());
        if(vehicledata && vehicledata->healthfromdriver)
        {
            if(slot == 0)
            {
                uint32 health = GetUInt32Value(UNIT_FIELD_HEALTH);
                uint32 maxhealth = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
                uint32 protomaxhealth = _creatureData->MaxHealth;
                uint32 healthdiff = maxhealth - health;
                uint32 plritemlevel = plr->GetTotalItemLevel();
                uint32 convrate = vehicledata->healthunitfromitemlev;

                if(plritemlevel != 0 && convrate != 0)
                {
                    uint32 healthloss = healthdiff+plritemlevel*convrate;
                    SetUInt32Value(UNIT_FIELD_HEALTH, _creatureData->MaxHealth - healthloss);
                }
                else if(protomaxhealth > healthdiff)
                    SetUInt32Value(UNIT_FIELD_HEALTH, protomaxhealth-healthdiff);
                else SetUInt32Value(UNIT_FIELD_HEALTH, 1);
                SetUInt32Value(UNIT_FIELD_MAXHEALTH, protomaxhealth);
            }
        }
    }

    if(slot == 0)
    {
        CombatStatus.Vanished();
        pPassenger->SetUInt64Value( UNIT_FIELD_CHARM, 0 );
        SetUInt64Value(UNIT_FIELD_CHARMEDBY, 0);

        if(m_factionTemplate == NULL || m_factionTemplate->ID == 35 || m_factionTemplate->ID == 2105)
            SetFaction(GetCharmTempVal(), false);
        RemoveAura(62064);
    }

    m_passengers[slot] = NULL;
    if(pPassenger->IsPlayer())
        --m_ppassengerCount;

    //note: this is not blizz like we should despawn
    //and respawn at spawn point.
    //Well actually this is how blizz wanted it
    //but they couldnt get it to work xD
    bool haspassengers = false;
    for(uint8 i = 0; i < m_seatSlotMax; i++)
    {
        if(m_passengers[i] != NULL && m_passengers[i]->IsPlayer())
        {
            haspassengers = true;
            break;
        }
    }

    if(!haspassengers && !GetVehicle()) // Passenger and accessory checks.
    {
        if( m_spawn == NULL )
            SafeDelete();
    }

    if(!IsFull())
        SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

    _setFaction();
}

bool Vehicle::HasPassenger(Unit* pPassenger)
{
    for(uint8 i = 0; i < m_seatSlotMax; i++)
    {
        if( m_passengers[i] == pPassenger )
            return true;
    }
    return false;
}

void Vehicle::_AddToSlot(Unit* pPassenger, uint8 slot)
{
    assert( slot < m_seatSlotMax );

    if(pPassenger->IsPlayer() && castPtr<Player>(pPassenger)->m_CurrentCharm)
        return;

    if(pPassenger->IsPlayer() && castPtr<Player>(pPassenger)->m_isGmInvisible)
    {
        sChatHandler.GreenSystemMessage(castPtr<Player>(pPassenger)->GetSession(), "Please turn off invis before entering vehicle.");
        return;
    }

    CreatureVehicleData* vehicledata = CreatureVehicleDataStorage.LookupEntry(GetEntry());
    m_passengers[slot] = pPassenger;

    LocationVector v;
    v.x = m_vehicleSeats[slot]->m_attachmentOffsetX; /* pPassenger->m_TransporterX = */
    v.y = m_vehicleSeats[slot]->m_attachmentOffsetY; /* pPassenger->m_TransporterY = */
    v.z = m_vehicleSeats[slot]->m_attachmentOffsetZ; /* pPassenger->m_TransporterZ = */
    v.o = 0; /* pPassenger->m_TransporterO = */
    //pPassenger->m_transportPosition =& v; // This is handled elsewhere, do not initialize here.
    pPassenger->GetMovementInterface()->LockTransportData();
    pPassenger->GetMovementInterface()->SetTransportData(GetGUID(), GetVehicleEntry(), v.x, v.y, v.z, GetOrientation(), slot);

    if( m_CastSpellOnMount )
        pPassenger->CastSpell( pPassenger, m_CastSpellOnMount, true );

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SELF_RES);

    // This is where the real magic happens
    if( pPassenger->IsPlayer() )
    {
        Player* pPlayer = castPtr<Player>(pPassenger);
        //pPlayer->Root();

        //Dismount
        if(pPlayer->m_MountSpellId && pPlayer->m_MountSpellId != m_mountSpell)
            pPlayer->RemoveAura(pPlayer->m_MountSpellId);

        //Remove morph spells
        if(pPlayer->GetUInt32Value(UNIT_FIELD_DISPLAYID) != pPlayer->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID))
        {
            pPlayer->m_AuraInterface.RemoveAllAurasOfType(SPELL_AURA_TRANSFORM);
            pPlayer->m_AuraInterface.RemoveAllAurasOfType(SPELL_AURA_MOD_SHAPESHIFT);
        }

        //Dismiss any pets
        if(pPlayer->GetSummon())
        {
            if(pPlayer->GetSummon()->GetUInt32Value(UNIT_CREATED_BY_SPELL) > 0)
                pPlayer->GetSummon()->Dismiss(false);               // warlock summon -> dismiss
            else
                pPlayer->GetSummon()->Remove(false, true, true);    // hunter pet -> just remove for later re-call
        }

        pPlayer->SetVehicle(this);
        pPlayer->SetUInt64Value(PLAYER_FARSIGHT, GetGUID());
        pPlayer->SetPlayerStatus(TRANSFER_PENDING);
        sEventMgr.AddEvent(pPlayer, &Player::CheckPlayerStatus, (uint8)TRANSFER_PENDING, EVENT_PLAYER_CHECK_STATUS_Transfer, 5000, 0, 0);

        WorldPacket data(SMSG_MONSTER_MOVE_TRANSPORT, 100);
        data << pPlayer->GetGUID();                          // Passengerguid
        data << GetGUID();                                   // Transporterguid (vehicleguid)
        data << uint8(slot);                                // Vehicle Seat ID
        data << uint8(0);                                   // Unknown
        data << GetPositionX() - pPlayer->GetPositionX();   // OffsetTransporterX
        data << GetPositionY() - pPlayer->GetPositionY();   // OffsetTransporterY
        data << GetPositionZ() - pPlayer->GetPositionZ();   // OffsetTransporterZ
        data << getMSTime();                                // Timestamp
        data << uint8(0x04);                                // Flags
        data << float(0);                                   // Orientation Offset
        data << uint32(0);                                  // MovementFlags
        data << uint32(0);                                  // MoveTime
        data << uint32(1);                                  // Points
        data << v.x;                                        // GetTransOffsetX();
        data << v.y;                                        // GetTransOffsetY();
        data << v.z;                                        // GetTransOffsetZ();
        SendMessageToSet(&data, true);

        if(vehicledata)
        {   // We have proto, no accessory in slot, and slot sets unselectable, unlike some seats
            if(!vehicledata->seats[slot].accessoryentry && vehicledata->seats[slot].unselectableaccessory)
                pPlayer->SetFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_UNKNOWN_5 | UNIT_FLAG_PREPARATION | UNIT_FLAG_NOT_SELECTABLE));
        } else pPlayer->SetFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_UNKNOWN_5 | UNIT_FLAG_PREPARATION | UNIT_FLAG_NOT_SELECTABLE));

        if(slot == 0)
        {
            if(m_vehicleSeats[slot]->IsControllable())
            {
                // send "switch mover" packet
                data.Initialize(SMSG_CLIENT_CONTROL_UPDATE);
                data << GetGUID() << uint8(1);
                pPlayer->GetSession()->SendPacket(&data);

                pPlayer->m_CurrentCharm = castPtr<Unit>(this);
                pPlayer->SetUInt64Value(UNIT_FIELD_CHARM, GetGUID());

                SetUInt64Value(UNIT_FIELD_CHARMEDBY, pPlayer->GetGUID());
                SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE);

                if(m_factionTemplate == NULL || m_factionTemplate->ID == 35 || m_factionTemplate->ID == 2105)
                {
                    SetCharmTempVal(pPlayer->GetFactionID());
                    SetFaction(pPlayer->GetFactionID());
                }

                if(vehicledata && vehicledata->healthfromdriver)
                {
                    uint32 health = GetUInt32Value(UNIT_FIELD_HEALTH);
                    uint32 maxhealth = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
                    uint32 healthdiff = maxhealth - health;

                    SetUInt32Value(UNIT_FIELD_MAXHEALTH, (maxhealth+((pPlayer->GetTotalItemLevel())*(vehicledata->healthunitfromitemlev))));
                    SetUInt32Value(UNIT_FIELD_HEALTH, (health+((pPlayer->GetTotalItemLevel())*(vehicledata->healthunitfromitemlev))) - healthdiff);
                }

                SendSpells(GetEntry(), pPlayer);
            }
        }
        else
        {
            data.Initialize(SMSG_CLIENT_CONTROL_UPDATE);
            data << GetGUID() << uint8(0);
            pPlayer->GetSession()->SendPacket(&data);
        }

        data.Initialize(SMSG_PET_DISMISS_SOUND);
        data << uint32(m_vehicleSeats[slot]->m_enterUISoundID);
        data.appendvector(pPlayer->GetPosition());
        pPlayer->GetSession()->SendPacket(&data);
        ++m_ppassengerCount;
    }
    else
    {
        pPassenger->SetVehicle(this);
        if(vehicledata != NULL)
        {
            if(vehicledata->seats[slot].accessoryentry == pPassenger->GetEntry())
            {
                if(vehicledata->seats[slot].unselectableaccessory == true)
                    pPassenger->SetFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_UNKNOWN_5 | UNIT_FLAG_PREPARATION | UNIT_FLAG_NOT_SELECTABLE));
                else pPassenger->SetFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_UNKNOWN_5 | UNIT_FLAG_PREPARATION));
            } else pPassenger->SetFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_UNKNOWN_5 | UNIT_FLAG_PREPARATION | UNIT_FLAG_NOT_SELECTABLE));
        }

        pPassenger->SetPosition(GetPositionX()+v.x, GetPositionY()+v.y, GetPositionZ()+v.z, GetOrientation());
    }

    if(IsFull())
        RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
    _setFaction();
}

void Vehicle::SetDeathState(DeathState s)
{
    Creature::SetDeathState(s);
    CreatureVehicleData* vehicledata = CreatureVehicleDataStorage.LookupEntry(GetEntry());
    for (uint8 i = 0; i < m_seatSlotMax; i++)
    {
        if(m_passengers[i] != NULL)
        {
            if(m_passengers[i]->IsPlayer() || (vehicledata && vehicledata->seats[i].ejectfromvehicleondeath))
                RemovePassenger(m_passengers[i]);
            else m_passengers[i]->SetDeathState(s);
        }
    }

    if( s == JUST_DIED && m_CreatedFromSpell)
        SafeDelete();
}

/* This function handles the packet sent from the client when we
leave a vehicle, it removes us server side from our current
vehicle*/
void WorldSession::HandleVehicleDismiss(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    if(!_player->GetVehicle())
        return;

    _player->ExitingVehicle = true;
    if((recv_data.rpos() != recv_data.wpos()) && (/*So we don't get disconnected due to size checks.*/recv_data.size() <= 90))
        HandleMovementOpcodes(recv_data);

    _player->GetVehicle()->RemovePassenger(GetPlayer());
    _player->ExitingVehicle = false;
}

/* This function handles the packet from the client which is
sent when we click on an npc with the flag UNIT_FLAG_SPELLCLICK
and checks if there is room for us then adds us as a passenger
to that vehicle*/
void WorldSession::HandleSpellClick( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    uint64 guid;
    recv_data >> guid;

    Vehicle* pVehicle = NULL;
    Unit* unit = GetPlayer()->GetMapMgr()->GetUnit(guid);

    if(!unit)
        return;

    if (_player->GetVehicle())
        return;

    if(!_player->isAlive() || !unit->isAlive())
        return;

    if(!unit->IsVehicle())
    {
        Creature* ctr = castPtr<Creature>(unit);
        if(ctr->IsLightwell(ctr->GetEntry()))
        {
            Aura * aur = ctr->m_AuraInterface.FindActiveAura(59907);
            if(aur)
            {
                aur->RemoveProcCharges(1);
                unit->CastSpell(_player, 60123, true);
                if( aur->getProcCharges() == 0 )
                {
                    unit->RemoveAura(aur);
                    ctr->SafeDelete();
                }
            }
            return;
        }
        SpellEntry * sp = dbcSpell.LookupEntry(ctr->GetCreatureData()->SpellClickid);
        if(sp != NULL)
            unit->CastSpell(_player, sp, true);
        else sLog.outDebug("[SPELLCLICK]: Invalid Spell ID %u creature %u", ctr->GetCreatureData()->SpellClickid, ctr->GetEntry());
        return;
    }

    pVehicle = castPtr<Vehicle>(unit);
    if(!pVehicle->CanEnterVehicle(_player))
        return;
    pVehicle->AddPassenger(_player);
}

/* This function handles the packet sent from the client when we
change a seat on a vehicle. If the seat has a unit passenger and unit
is a vehicle, we will enter the passenger.*/
void WorldSession::HandleRequestSeatChange( WorldPacket & recv_data )
{
    WoWGuid Vehicleguid;
    int8 RequestedSeat;
    Unit* cv = _player->GetVehicle();
    _player->ChangingSeats = true;

    if(recv_data.GetOpcode() == CMSG_REQUEST_VEHICLE_PREV_SEAT)
    {
        int8 seat = _player->GetSeatID();
        int8 newseat = seat;
        while(seat == _player->GetSeatID())
        {
            if(newseat > 7)
                break;

            --newseat;

            if(cv->m_vehicleSeats[newseat] && cv->seatisusable[newseat])
                break;
        }

        if(cv->m_vehicleSeats[newseat] && cv->seatisusable[newseat])
            cv->ChangeSeats(_player, newseat);

        _player->ChangingSeats = false;
        return;
    }
    else if(recv_data.GetOpcode() == CMSG_REQUEST_VEHICLE_NEXT_SEAT)
    {
        int8 seat = _player->GetSeatID();
        int8 newseat = seat;
        while(seat == _player->GetSeatID())
        {
            if(newseat > 7)
                break;

            ++newseat;

            if(cv->m_vehicleSeats[newseat] && cv->seatisusable[newseat])
                break;
        }
        if(cv->m_vehicleSeats[newseat] && cv->seatisusable[newseat])
            cv->ChangeSeats(_player, newseat);

        _player->ChangingSeats = false;
        return;
    }
    else if(recv_data.GetOpcode() == CMSG_REQUEST_VEHICLE_SWITCH_SEAT)
    {
        recv_data >> Vehicleguid.asPacked();
        recv_data >> RequestedSeat;
    }
    else
    {
        HandleMovementOpcodes(recv_data);
        recv_data >> Vehicleguid.asPacked();
        recv_data >> RequestedSeat;
    }

    Vehicle* vehicle = GetPlayer()->GetMapMgr()->GetVehicle(Vehicleguid);
    if(cv->GetPassengerSlot(vehicle) != -1 || cv->GetVehicle() == vehicle)
    {
        cv->RemovePassenger(_player);
        vehicle->AddPassenger(_player, RequestedSeat);
        _player->ChangingSeats = false;
        return;
    }

    cv->ChangeSeats(_player, RequestedSeat);
    _player->ChangingSeats = false;
}

void Vehicle::ChangeSeats(Unit* unit, uint8 seatid)
{
    if(seatid == unit->GetSeatID())
    {
        sLog.outDebug("Return, Matching Seats. Requsted: %u, current: %u", seatid, unit->GetSeatID());
        return;
    }

    if(unit->GetVehicle() != NULL)
        unit->GetVehicle()->RemovePassenger(unit);
    AddPassenger(unit, seatid);
}

void WorldSession::HandleEjectPassenger( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;
    if(!_player->GetVehicle())
    {
        sWorld.LogCheater(this, "Player possibly hacking, received CMSG_EJECT_PASSENGER while player isn't in a vehicle.");
        return;
    }

    Unit* u = _player->GetMapMgr()->GetUnit(guid);
    if(!u)
    {
        sLog.outDebug("CMSG_EJECT_PASSENGER couldn't find unit with recv'd guid %u.", guid);
        return;
    }

    if(u->GetVehicle() != _player->GetVehicle() || !u->GetVehicle())
    {
        sWorld.LogCheater(this, "Player possibly hacking, CMSG_EJECT_PASSENGER received unit guid for a unit not in their vehicle.");
        return;
    }
    _player->GetVehicle()->RemovePassenger(u);
    if(u->IsCreature())
        castPtr<Creature>(u)->SafeDelete();
}

void WorldSession::HandleVehicleMountEnter( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;
}
