/***
 * Demonstrike Core
 */

#include "StdAfx.h"

static float m_defaultSpeeds[MOVE_SPEED_MAX] = { 2.5f, 8.f, 4.5f, 4.722222f, 2.5f, 7.f, 4.5f, 3.141593f, 3.141593f };

MovementInterface::MovementInterface(Unit *_unit) : m_Unit(_unit), m_underwaterState(0), m_incrementMoveCounter(false), m_serverCounter(0), m_clientCounter(0)
{
    for(uint8 i = 0; i < MOVE_SPEED_MAX; i++)
    {
        m_currSpeeds[i] = m_movementSpeeds[i] = m_defaultSpeeds[i];
        m_speedOffset[i] = m_pendingSpeeds[i] = 0.f;
        m_speedTimers[i] = 0;
    }

    m_serverLocation = m_Unit->GetPositionV();
    m_clientLocation.ChangeCoords(0.f, 0.f, 0.f, 0.f);

    // Sizeof1
    memset(m_movementFlags, 0, sizeof(uint8)*6);
    memset(m_serverFlags, 0, sizeof(uint8)*6);

    ClearTransportData();
    m_extra.clear();
}

MovementInterface::~MovementInterface()
{

}

static PacketHandler movementPacketHandlers[MAX_MOVEMENT_CODE] = {
    // Movement codes
    &MovementInterface::HandlePlayerMove,
    &MovementInterface::HandleHeartbeat,
    &MovementInterface::HandleJump,
    &MovementInterface::HandleFallLand,
    &MovementInterface::HandleStartForward,
    &MovementInterface::HandleStartBackward,
    &MovementInterface::HandleStartStrafeLeft,
    &MovementInterface::HandleStartStrafeRight,
    &MovementInterface::HandleStartTurnLeft,
    &MovementInterface::HandleStartTurnRight,
    &MovementInterface::HandleStartPitchDown,
    &MovementInterface::HandleStartPitchUp,
    &MovementInterface::HandleStartAscend,
    &MovementInterface::HandleStartDescend,
    &MovementInterface::HandleStartSwim,
    &MovementInterface::HandleStop,
    &MovementInterface::HandleStopStrafe,
    &MovementInterface::HandleStopTurn,
    &MovementInterface::HandleStopPitch,
    &MovementInterface::HandleStopAscend,
    &MovementInterface::HandleStopSwim,
    &MovementInterface::HandleSetFacing,
    &MovementInterface::HandleSetPitch,
    &MovementInterface::HandleFallReset,
    &MovementInterface::HandleSetRunMode,
    &MovementInterface::HandleSetWalkMode,
    &MovementInterface::HandleSetPitchRate,
    &MovementInterface::HandleSetCanFly,
    &MovementInterface::HandleUnsetCanFly,
    &MovementInterface::HandleSetHover,
    &MovementInterface::HandleUnsetHover,
    &MovementInterface::HandleWaterWalk,
    &MovementInterface::HandleLandWalk,
    &MovementInterface::HandleFeatherFall,
    &MovementInterface::HandleNormalFall,
    &MovementInterface::HandleRoot,
    &MovementInterface::HandleUnroot,
    &MovementInterface::HandleUpdateKnockBack,
    &MovementInterface::HandleUpdateTeleport,
    &MovementInterface::HandleChangeTransport,
    &MovementInterface::HandleNotActiveMover,
    &MovementInterface::HandleSetCollisionHeight,
    &MovementInterface::HandleUpdateCollisionHeight,
    // Speed codes
    &MovementInterface::HandleUpdateWalkSpeed,
    &MovementInterface::HandleUpdateRunSpeed,
    &MovementInterface::HandleUpdateRunBackSpeed,
    &MovementInterface::HandleUpdateSwimSpeed,
    &MovementInterface::HandleUpdateFlightSpeed,
    &MovementInterface::HandleSetWalkSpeed,
    &MovementInterface::HandleSetRunSpeed,
    &MovementInterface::HandleSetRunBackSpeed,
    &MovementInterface::HandleSetSwimSpeed,
    &MovementInterface::HandleSetSwimBackSpeed,
    &MovementInterface::HandleSetFlightSpeed,
    &MovementInterface::HandleSetFlightBackSpeed,
    &MovementInterface::HandleSetTurnRate,
    // Acknowledgement codes
    &MovementInterface::HandleAckRoot,
    &MovementInterface::HandleAckUnroot,
    &MovementInterface::HandleAckFeatherFall,
    &MovementInterface::HandleAckForceWalkSpeedChange,
    &MovementInterface::HandleAckForceRunSpeedChange,
    &MovementInterface::HandleAckForceRunBackSpeedChange,
    &MovementInterface::HandleAckForceSwimSpeedChange,
    &MovementInterface::HandleAckForceFlightSpeedChange,
    &MovementInterface::HandleAckGravityEnable,
    &MovementInterface::HandleAckGravityDisable,
    &MovementInterface::HandleAckHover,
    &MovementInterface::HandleAckWaterWalk,
    &MovementInterface::HandleAckKnockBack,
    &MovementInterface::HandleAckSetCanFly,
    &MovementInterface::HandleAckSetCollisionHeight,
    &MovementInterface::HandleAckSetCanTransitionBetweenSwimAndFly,
    // Spline codes
    &MovementInterface::HandleSplineDone,
    &MovementInterface::HandleSplineSetWalkSpeed,
    &MovementInterface::HandleSplineSetRunSpeed,
    &MovementInterface::HandleSplineSetRunBackSpeed,
    &MovementInterface::HandleSplineSetSwimSpeed,
    &MovementInterface::HandleSplineSetSwimBackSpeed,
    &MovementInterface::HandleSplineSetFlightSpeed,
    &MovementInterface::HandleSplineSetFlightBackSpeed,
    &MovementInterface::HandleSplineSetPitchRate,
    &MovementInterface::HandleSplineSetTurnRate,
    &MovementInterface::HandleSplineSetWalkMode,
    &MovementInterface::HandleSplineSetRunMode,
    &MovementInterface::HandleSplineGravityEnable,
    &MovementInterface::HandleSplineGravityDisable,
    &MovementInterface::HandleSplineSetHover,
    &MovementInterface::HandleSplineSetUnhover,
    &MovementInterface::HandleSplineStartSwim,
    &MovementInterface::HandleSplineStopSwim,
    &MovementInterface::HandleSplineSetFlying,
    &MovementInterface::HandleSplineUnsetFlying,
    &MovementInterface::HandleSplineSetWaterWalk,
    &MovementInterface::HandleSplineSetLandWalk,
    &MovementInterface::HandleSplineSetFeatherFall,
    &MovementInterface::HandleSplineSetNormalFall,
    &MovementInterface::HandleSplineRoot,
    &MovementInterface::HandleSplineUnroot,
    // Vehicle codes
    &MovementInterface::HandleDismissControlledVehicle,
    &MovementInterface::HandleChangeSeatsOnControlledVehicle,
    // Embedded codes
    &MovementInterface::HandleEmbeddedMovement,
    &MovementInterface::HandleEmbeddedMovement,
    &MovementInterface::HandleEmbeddedMovement
};

uint16 MovementInterface::GetInternalMovementCode(uint16 opcode)
{
    switch(opcode)
    {
    case SMSG_PLAYER_MOVE: return MOVEMENT_CODE_PLAYER_MOVE;
    case MSG_MOVE_HEARTBEAT: return MOVEMENT_CODE_HEARTBEAT;
    case MSG_MOVE_JUMP: return MOVEMENT_CODE_JUMP;
    case MSG_MOVE_FALL_LAND: return MOVEMENT_CODE_FALL_LAND;
    case MSG_MOVE_START_FORWARD: return MOVEMENT_CODE_START_FORWARD;
    case MSG_MOVE_START_BACKWARD: return MOVEMENT_CODE_START_BACKWARD;
    case MSG_MOVE_START_STRAFE_LEFT: return MOVEMENT_CODE_START_STRAFE_LEFT;
    case MSG_MOVE_START_STRAFE_RIGHT: return MOVEMENT_CODE_START_STRAFE_RIGHT;
    case MSG_MOVE_START_TURN_LEFT: return MOVEMENT_CODE_START_TURN_LEFT;
    case MSG_MOVE_START_TURN_RIGHT: return MOVEMENT_CODE_START_TURN_RIGHT;
    case MSG_MOVE_START_PITCH_DOWN: return MOVEMENT_CODE_START_PITCH_DOWN;
    case MSG_MOVE_START_PITCH_UP: return MOVEMENT_CODE_START_PITCH_UP;
    case MSG_MOVE_START_ASCEND: return MOVEMENT_CODE_START_ASCEND;
    case MSG_MOVE_START_DESCEND: return MOVEMENT_CODE_START_DESCEND;
    case MSG_MOVE_START_SWIM: return MOVEMENT_CODE_START_SWIM;
    case MSG_MOVE_STOP: return MOVEMENT_CODE_STOP;
    case MSG_MOVE_STOP_STRAFE: return MOVEMENT_CODE_STOP_STRAFE;
    case MSG_MOVE_STOP_TURN: return MOVEMENT_CODE_STOP_TURN;
    case MSG_MOVE_STOP_PITCH: return MOVEMENT_CODE_STOP_PITCH;
    case MSG_MOVE_STOP_ASCEND: return MOVEMENT_CODE_STOP_ASCEND;
    case MSG_MOVE_STOP_SWIM: return MOVEMENT_CODE_STOP_SWIM;
    case MSG_MOVE_SET_FACING: return MOVEMENT_CODE_SET_FACING;
    case MSG_MOVE_SET_PITCH: return MOVEMENT_CODE_SET_PITCH;
    case CMSG_MOVE_FALL_RESET: return MOVEMENT_CODE_FALL_RESET;
    case MSG_MOVE_SET_RUN_MODE: return MOVEMENT_CODE_SET_RUN_MODE;
    case MSG_MOVE_SET_WALK_MODE: return MOVEMENT_CODE_SET_WALK_MODE;
    case SMSG_MOVE_SET_PITCH_RATE: return MOVEMENT_CODE_SET_PITCH_RATE;
    case CMSG_MOVE_SET_CAN_FLY: return MOVEMENT_CODE_SET_CAN_FLY;
    case SMSG_MOVE_SET_CAN_FLY: return MOVEMENT_CODE_SET_CAN_FLY;
    case SMSG_MOVE_UNSET_CAN_FLY: return MOVEMENT_CODE_UNSET_CAN_FLY;
    case SMSG_MOVE_SET_HOVER: return MOVEMENT_CODE_SET_HOVER;
    case SMSG_MOVE_UNSET_HOVER: return MOVEMENT_CODE_UNSET_HOVER;
    case SMSG_MOVE_WATER_WALK: return MOVEMENT_CODE_WATER_WALK;
    case SMSG_MOVE_LAND_WALK: return MOVEMENT_CODE_LAND_WALK;
    case SMSG_MOVE_FEATHER_FALL: return MOVEMENT_CODE_FEATHER_FALL;
    case SMSG_MOVE_NORMAL_FALL: return MOVEMENT_CODE_NORMAL_FALL;
    case SMSG_MOVE_ROOT: return MOVEMENT_CODE_ROOT;
    case SMSG_MOVE_UNROOT: return MOVEMENT_CODE_UNROOT;
    case SMSG_MOVE_UPDATE_KNOCK_BACK: return MOVEMENT_CODE_UPDATE_KNOCK_BACK;
    case SMSG_MOVE_UPDATE_TELEPORT: return MOVEMENT_CODE_UPDATE_TELEPORT;
    case CMSG_MOVE_CHNG_TRANSPORT: return MOVEMENT_CODE_CHANGE_TRANSPORT;
    case CMSG_MOVE_NOT_ACTIVE_MOVER: return MOVEMENT_CODE_NOT_ACTIVE_MOVER;
    case SMSG_MOVE_SET_COLLISION_HEIGHT: return MOVEMENT_CODE_SET_COLLISION_HEIGHT;
    case SMSG_MOVE_UPDATE_COLLISION_HEIGHT: return MOVEMENT_CODE_UPDATE_COLLISION_HEIGHT;

        // Speed codes
    case SMSG_MOVE_UPDATE_WALK_SPEED: return MOVEMENT_CODE_UPDATE_WALK_SPEED;
    case SMSG_MOVE_UPDATE_RUN_SPEED: return MOVEMENT_CODE_UPDATE_RUN_SPEED;
    case SMSG_MOVE_UPDATE_RUN_BACK_SPEED: return MOVEMENT_CODE_UPDATE_RUN_BACK_SPEED;
    case SMSG_MOVE_UPDATE_SWIM_SPEED: return MOVEMENT_CODE_UPDATE_SWIM_SPEED;
    case SMSG_MOVE_UPDATE_FLIGHT_SPEED: return MOVEMENT_CODE_UPDATE_FLIGHT_SPEED;
    case SMSG_MOVE_SET_WALK_SPEED: return MOVEMENT_CODE_SET_WALK_SPEED;
    case SMSG_MOVE_SET_RUN_SPEED: return MOVEMENT_CODE_SET_RUN_SPEED;
    case SMSG_MOVE_SET_RUN_BACK_SPEED: return MOVEMENT_CODE_SET_RUN_BACK_SPEED;
    case SMSG_MOVE_SET_SWIM_SPEED: return MOVEMENT_CODE_SET_SWIM_SPEED;
    case SMSG_MOVE_SET_SWIM_BACK_SPEED: return MOVEMENT_CODE_SET_SWIM_BACK_SPEED;
    case SMSG_MOVE_SET_FLIGHT_SPEED: return MOVEMENT_CODE_SET_FLIGHT_SPEED;
    case SMSG_MOVE_SET_FLIGHT_BACK_SPEED: return MOVEMENT_CODE_SET_FLIGHT_BACK_SPEED;
    case SMSG_MOVE_SET_TURN_RATE: return MOVEMENT_CODE_SET_TURN_RATE;

        // Acknowledgement codes
    case CMSG_FORCE_MOVE_ROOT_ACK: return MOVEMENT_CODE_ACK_ROOT;
    case CMSG_FORCE_MOVE_UNROOT_ACK: return MOVEMENT_CODE_ACK_UNROOT;
    case CMSG_MOVE_FEATHER_FALL_ACK: return MOVEMENT_CODE_ACK_FEATHER_FALL;
    case CMSG_MOVE_GRAVITY_ENABLE_ACK: return MOVEMENT_CODE_ACK_GRAVITY_ENABLE;
    case CMSG_MOVE_GRAVITY_DISABLE_ACK: return MOVEMENT_CODE_ACK_GRAVITY_DISABLE;
    case CMSG_MOVE_HOVER_ACK: return MOVEMENT_CODE_ACK_HOVER;
    case CMSG_MOVE_WATER_WALK_ACK: return MOVEMENT_CODE_ACK_WATER_WALK;
    case CMSG_MOVE_KNOCK_BACK_ACK: return MOVEMENT_CODE_ACK_KNOCK_BACK;
    case CMSG_MOVE_SET_CAN_FLY_ACK: return MOVEMENT_CODE_ACK_SET_CAN_FLY;
    case CMSG_MOVE_SET_COLLISION_HEIGHT_ACK: return MOVEMENT_CODE_ACK_SET_COLLISION_HEIGHT;
    case CMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY_ACK: return MOVEMENT_CODE_ACK_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY;
    case CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK: return MOVEMENT_CODE_ACK_FORCE_WALK_SPEED_CHANGE;
    case CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK: return MOVEMENT_CODE_ACK_FORCE_RUN_SPEED_CHANGE;
    case CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK: return MOVEMENT_CODE_ACK_FORCE_RUN_BACK_SPEED_CHANGE;
    case CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK: return MOVEMENT_CODE_ACK_FORCE_SWIM_SPEED_CHANGE;
    case CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK: return MOVEMENT_CODE_ACK_FORCE_FLIGHT_SPEED_CHANGE;

        // Spline codes
    case CMSG_MOVE_SPLINE_DONE: return MOVEMENT_CODE_SPLINE_DONE;
    case SMSG_SPLINE_MOVE_SET_WALK_SPEED: return MOVEMENT_CODE_SPLINE_SET_WALK_SPEED;
    case SMSG_SPLINE_MOVE_SET_RUN_SPEED: return MOVEMENT_CODE_SPLINE_SET_RUN_SPEED;
    case SMSG_SPLINE_MOVE_SET_RUN_BACK_SPEED: return MOVEMENT_CODE_SPLINE_SET_RUN_BACK_SPEED;
    case SMSG_SPLINE_MOVE_SET_SWIM_SPEED: return MOVEMENT_CODE_SPLINE_SET_SWIM_SPEED;
    case SMSG_SPLINE_MOVE_SET_SWIM_BACK_SPEED: return MOVEMENT_CODE_SPLINE_SET_SWIM_BACK_SPEED;
    case SMSG_SPLINE_MOVE_SET_FLIGHT_SPEED: return MOVEMENT_CODE_SPLINE_SET_FLIGHT_SPEED;
    case SMSG_SPLINE_MOVE_SET_FLIGHT_BACK_SPEED: return MOVEMENT_CODE_SPLINE_SET_FLIGHT_BACK_SPEED;
    case SMSG_SPLINE_MOVE_SET_PITCH_RATE: return MOVEMENT_CODE_SPLINE_SET_PITCH_RATE;
    case SMSG_SPLINE_MOVE_SET_TURN_RATE: return MOVEMENT_CODE_SPLINE_SET_TURN_RATE;
    case SMSG_SPLINE_MOVE_SET_WALK_MODE: return MOVEMENT_CODE_SPLINE_SET_WALK_MODE;
    case SMSG_SPLINE_MOVE_SET_RUN_MODE: return MOVEMENT_CODE_SPLINE_SET_RUN_MODE;
    case SMSG_SPLINE_MOVE_GRAVITY_DISABLE: return MOVEMENT_CODE_SPLINE_GRAVITY_DISABLE;
    case SMSG_SPLINE_MOVE_GRAVITY_ENABLE: return MOVEMENT_CODE_SPLINE_GRAVITY_ENABLE;
    case SMSG_SPLINE_MOVE_SET_HOVER: return MOVEMENT_CODE_SPLINE_SET_HOVER;
    case SMSG_SPLINE_MOVE_UNSET_HOVER: return MOVEMENT_CODE_SPLINE_UNSET_HOVER;
    case SMSG_SPLINE_MOVE_START_SWIM: return MOVEMENT_CODE_SPLINE_START_SWIM;
    case SMSG_SPLINE_MOVE_STOP_SWIM: return MOVEMENT_CODE_SPLINE_STOP_SWIM;
    case SMSG_SPLINE_MOVE_SET_FLYING: return MOVEMENT_CODE_SPLINE_SET_FLYING;
    case SMSG_SPLINE_MOVE_UNSET_FLYING: return MOVEMENT_CODE_SPLINE_UNSET_FLYING;
    case SMSG_SPLINE_MOVE_SET_WATER_WALK: return MOVEMENT_CODE_SPLINE_SET_WATER_WALK;
    case SMSG_SPLINE_MOVE_SET_LAND_WALK: return MOVEMENT_CODE_SPLINE_SET_LAND_WALK;
    case SMSG_SPLINE_MOVE_SET_FEATHER_FALL: return MOVEMENT_CODE_SPLINE_SET_FEATHER_FALL;
    case SMSG_SPLINE_MOVE_SET_NORMAL_FALL: return MOVEMENT_CODE_SPLINE_SET_NORMAL_FALL;
    case SMSG_SPLINE_MOVE_ROOT: return MOVEMENT_CODE_SPLINE_ROOT;
    case SMSG_SPLINE_MOVE_UNROOT: return MOVEMENT_CODE_SPLINE_UNROOT;

        // Vehicle codes
    case CMSG_DISMISS_CONTROLLED_VEHICLE: return MOVEMENT_CODE_DISMISS_CONTROLLED_VEHICLE;
    case CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE: return MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE;

        // Embedded codes
    case CMSG_CAST_SPELL: return MOVEMENT_EMBEDDED_CAST_SPELL;
    case CMSG_PET_CAST_SPELL: return MOVEMENT_EMBEDDED_PET_CAST_SPELL;
    case CMSG_USE_ITEM: return MOVEMENT_EMBEDDED_USE_ITEM;
    }
    return 0xFFFF;
}

void MovementInterface::Update(uint32 diff)
{

}

void MovementInterface::UpdatePreWrite()
{

}

bool MovementInterface::UpdatePostRead(uint16 moveCode)
{
    UpdateMovementFlagMask();
    switch(moveCode)
    {
    case MOVEMENT_CODE_ACK_ROOT:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_ROOT])
                setServerFlag(MOVEMENTFLAG_TOGGLE_ROOT);
            m_pendingEnable[MOVEMENT_STATUS_ROOT] = false;
        }break;
    case MOVEMENT_CODE_ACK_UNROOT:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_UNROOT])
                removeServerFlag(MOVEMENTFLAG_TOGGLE_ROOT);
            m_pendingEnable[MOVEMENT_STATUS_UNROOT] = false;
        }break;
    case MOVEMENT_CODE_ACK_FEATHER_FALL:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_FEATHERFALLING])
                setServerFlag(MOVEMENTFLAG_FEATHERFALLING);
            else removeServerFlag(MOVEMENTFLAG_FEATHERFALLING);
            m_pendingEnable[MOVEMENT_STATUS_FEATHERFALLING] = false;
        }break;
    case MOVEMENT_CODE_ACK_GRAVITY_ENABLE:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_ENABLEGRAVITY])
                setServerFlag(MOVEMENTFLAG_TOGGLE_NO_GRAVITY);
            m_pendingEnable[MOVEMENT_STATUS_ENABLEGRAVITY] = false;
        }break;
    case MOVEMENT_CODE_ACK_GRAVITY_DISABLE:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_DISABLEGRAVITY])
                removeServerFlag(MOVEMENTFLAG_TOGGLE_NO_GRAVITY);
            m_pendingEnable[MOVEMENT_STATUS_DISABLEGRAVITY] = false;
        }break;
    case MOVEMENT_CODE_ACK_HOVER:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_HOVERING])
                setServerFlag(MOVEMENTFLAG_HOVER);
            else removeServerFlag(MOVEMENTFLAG_HOVER);
            m_pendingEnable[MOVEMENT_STATUS_HOVERING] = false;
        }break;
    case MOVEMENT_CODE_ACK_WATER_WALK:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_WATERWALKING])
                setServerFlag(MOVEMENTFLAG_WATERWALKING);
            else removeServerFlag(MOVEMENTFLAG_WATERWALKING);
            m_pendingEnable[MOVEMENT_STATUS_WATERWALKING] = false;
        }break;
    case MOVEMENT_CODE_ACK_KNOCK_BACK:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_KNOCKBACKED])
                m_isKnockBacked = true;
            else m_isKnockBacked = false;
            m_pendingEnable[MOVEMENT_STATUS_KNOCKBACKED] = false;
        }break;
    case MOVEMENT_CODE_ACK_SET_CAN_FLY:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_CANFLY])
                setServerFlag(MOVEMENTFLAG_CAN_FLY);
            else removeServerFlag(MOVEMENTFLAG_CAN_FLY);
            m_pendingEnable[MOVEMENT_STATUS_CANFLY] = false;
        }break;
    case MOVEMENT_CODE_ACK_SET_COLLISION_HEIGHT:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_COLLISION_HEIGHT])
                m_collisionHeight = m_extra.ex_float;
            m_pendingEnable[MOVEMENT_STATUS_COLLISION_HEIGHT] = false;
        }break;
    case MOVEMENT_CODE_ACK_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY:
        {
            if(m_pendingEnable[MOVEMENT_STATUS_CANSFTRANSITION])
                setServerFlag(MOVEMENTFLAG_CAN_SWIM_TO_FLY_TRANSITION);
            else removeServerFlag(MOVEMENTFLAG_CAN_SWIM_TO_FLY_TRANSITION);
            m_pendingEnable[MOVEMENT_STATUS_CANSFTRANSITION] = false;
        }break;
    case MOVEMENT_CODE_ACK_FORCE_WALK_SPEED_CHANGE: if(!UpdatePendingSpeed(MOVE_SPEED_WALK, m_extra.ex_float)) return false; break;
    case MOVEMENT_CODE_ACK_FORCE_RUN_SPEED_CHANGE: if(!UpdatePendingSpeed(MOVE_SPEED_RUN, m_extra.ex_float)) return false; break;
    case MOVEMENT_CODE_ACK_FORCE_RUN_BACK_SPEED_CHANGE: if(!UpdatePendingSpeed(MOVE_SPEED_RUN_BACK, m_extra.ex_float)) return false; break;
    case MOVEMENT_CODE_ACK_FORCE_SWIM_SPEED_CHANGE: if(!UpdatePendingSpeed(MOVE_SPEED_SWIM, m_extra.ex_float)) return false; break;
    case MOVEMENT_CODE_ACK_FORCE_FLIGHT_SPEED_CHANGE: if(!UpdatePendingSpeed(MOVE_SPEED_FLY, m_extra.ex_float)) return false; break;
    }
    return true;
}

void MovementInterface::HandleBreathing()
{
    uint16 WaterType = 0;
    float WaterHeight = NO_WATER_HEIGHT;
    m_Unit->GetMapMgr()->GetWaterData(m_serverLocation->x, m_serverLocation->y, m_serverLocation->z, WaterHeight, WaterType);
    if (WaterHeight == NO_WATER_HEIGHT)
    {
        m_underwaterState &= ~0xD0;
        return;
    }
    float HeightDelta = (WaterHeight-m_Unit->GetPositionZ())*10;

    // All liquids type - check under water position
    if(WaterType & (0x01|0x02|0x04|0x08) && HeightDelta > 20.f)
        m_underwaterState |= UNDERWATERSTATE_UNDERWATER;
    else m_underwaterState &= ~UNDERWATERSTATE_UNDERWATER;

    if(!m_Unit->IsPlayer() || !castPtr<Player>(m_Unit)->GetTaxiPath())
    {
        // Allow travel in dark water on taxi or transport
        if ((WaterType & 0x10) && m_transportGuid.empty())
            m_underwaterState |= UNDERWATERSTATE_FATIGUE;
        else m_underwaterState &= ~UNDERWATERSTATE_FATIGUE;
    } else m_underwaterState &= ~UNDERWATERSTATE_FATIGUE;

    // in lava check, anywhere in lava level
    if (WaterType & 0x04 && HeightDelta > 0.f)
        m_underwaterState |= UNDERWATERSTATE_LAVA;
    else m_underwaterState &= ~UNDERWATERSTATE_LAVA;

    // in slime check, anywhere in slime level
    if (WaterType & 0x08 && (HeightDelta > 0.f || (HeightDelta > -2.5f && hasFlag(MOVEMENTFLAG_WATERWALKING))))
        m_underwaterState |= UNDERWATERSTATE_SLIME;
    else m_underwaterState &= ~UNDERWATERSTATE_SLIME;
}

void MovementInterface::HandleMovementFlags(bool read, ByteBuffer *buffer)
{
    if(read)
    {
        // Clear previous movement flags
        m_movementFlags[0] = m_movementFlags[1] = m_movementFlags[2] = m_movementFlags[3] = 0;
        uint32 movementFlags = buffer->ReadBits(30);
        for(uint8 i = 0; i < 30; i++)
        {
            if((movementFlags & 1<<i) == 0)
                continue;
            uint8 byte = floor(i/8.f);
            m_movementFlags[byte] |= 1<<(i%8);
        }
    }
    else
    {
        uint32 movementFlags = GetFlags(0);
        movementFlags |= uint32(GetFlags(1))<<8;
        movementFlags |= uint32(GetFlags(2))<<16;
        movementFlags |= uint32(GetFlags(3))<<24;
        buffer->WriteBits(movementFlags, 30);
    }
}

void MovementInterface::HandleMovementFlags2(bool read, ByteBuffer *buffer)
{
    if(read)
    {
        // Clear previous movement flags
        m_movementFlags[4] = m_movementFlags[5] = 0;
        uint16 movementFlags2 = buffer->ReadBits(12);
        for(uint8 i = 0; i < 12; i++)
        {
            if((movementFlags2 & 1<<i) == 0)
                continue;
            if(i >= 8)
                m_movementFlags[5] |= 1<<(i%8);
            m_movementFlags[4] |= 1<<i;
        }
    }
    else
    {
        uint16 movementFlags2 = (GetFlags(4) | (uint16(GetFlags(5))<<8));
        buffer->WriteBits(movementFlags2, 12);
    }
}

void MovementInterface::OnDeath()
{

}

void MovementInterface::OnRepop()
{

}

void MovementInterface::OnRessurect()
{

}

void MovementInterface::OnRevive()
{

}

void MovementInterface::OnDismount()
{

}

void MovementInterface::OnTaxiEnd()
{

}

void MovementInterface::OnRelocate(LocationVector destination)
{

}

bool MovementInterface::ReadFromClient(uint16 opcode, ByteBuffer *buffer)
{
    m_movementLock.Acquire();
    m_extra.clear();

    bool res = false;
    uint16 moveCode = 0xFFFF;
    if((moveCode = GetInternalMovementCode(opcode)) != 0xFFFF)
    {
        try
        {
            printf("Handling movement %s, %u\n", sOpcodeMgr.GetOpcodeName(opcode), opcode);
            (this->*(movementPacketHandlers[moveCode].function))(true, *buffer);
            res = UpdatePostRead(moveCode);
        }
        catch (ByteBufferException &)
        {
            sLog.outError("Error reading packet %s from account %u", sOpcodeMgr.GetOpcodeName(opcode), castPtr<Player>(m_Unit)->GetSession()->GetAccountId());
        }
    }
    m_movementLock.Release();
    return res;
}

void MovementInterface::WriteFromServer(uint16 opcode, ByteBuffer *buffer, WoWGuid extra_guid, float extra_float, uint8 extra_byte)
{
    m_movementLock.Acquire();
    // Update our local extra data
    m_extra.set(extra_guid, extra_float, extra_byte);

    uint16 moveCode = 0xFFFF;
    if((moveCode = GetInternalMovementCode(opcode)) != 0xFFFF)
    {
        UpdatePreWrite();
        (this->*(movementPacketHandlers[moveCode].function))(false, *buffer);
        if(m_incrementMoveCounter)
        {
            m_incrementMoveCounter = false;
            m_serverCounter++;
        }
    } else sLog.outError("", "");
    // Clear the extra info
    m_extra.clear();
    m_movementLock.Release();
}

void MovementInterface::DoExtraData(uint16 moveCode, bool read, ByteBuffer *buffer, uint8 sequence)
{
    switch(moveCode)
    {
    case MOVEMENT_CODE_SET_PITCH_RATE:
    case MOVEMENT_CODE_SET_COLLISION_HEIGHT:
    case MOVEMENT_CODE_UPDATE_COLLISION_HEIGHT:
    case MOVEMENT_CODE_ACK_SET_COLLISION_HEIGHT:
    case MOVEMENT_CODE_ACK_FORCE_WALK_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_RUN_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_RUN_BACK_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_SWIM_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_FLIGHT_SPEED_CHANGE:
    case MOVEMENT_CODE_UPDATE_WALK_SPEED:
    case MOVEMENT_CODE_UPDATE_RUN_SPEED:
    case MOVEMENT_CODE_UPDATE_RUN_BACK_SPEED:
    case MOVEMENT_CODE_UPDATE_SWIM_SPEED:
    case MOVEMENT_CODE_UPDATE_FLIGHT_SPEED:
    case MOVEMENT_CODE_SET_WALK_SPEED:
    case MOVEMENT_CODE_SET_RUN_SPEED:
    case MOVEMENT_CODE_SET_RUN_BACK_SPEED:
    case MOVEMENT_CODE_SET_SWIM_SPEED:
    case MOVEMENT_CODE_SET_SWIM_BACK_SPEED:
    case MOVEMENT_CODE_SET_FLIGHT_SPEED:
    case MOVEMENT_CODE_SET_FLIGHT_BACK_SPEED:
    case MOVEMENT_CODE_SET_TURN_RATE:
        {
            if(read) m_extra.ex_float = buffer->read<float>();
            else buffer->append<float>(m_extra.ex_float);
        }break;
    case MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE:
        {
            switch(sequence)
            {
            case 0: if(read) m_extra.ex_byte = buffer->read<uint8>(); else buffer->append<uint8>(m_extra.ex_byte); break;
            case 1: if(read) m_extra.ex_guid[2] = buffer->ReadBit(); else buffer->WriteBit(m_extra.ex_guid[2]); break;
            case 2: if(read) m_extra.ex_guid[4] = buffer->ReadBit(); else buffer->WriteBit(m_extra.ex_guid[4]); break;
            case 3: if(read) m_extra.ex_guid[7] = buffer->ReadBit(); else buffer->WriteBit(m_extra.ex_guid[7]); break;
            case 4: if(read) m_extra.ex_guid[6] = buffer->ReadBit(); else buffer->WriteBit(m_extra.ex_guid[6]); break;
            case 5: if(read) m_extra.ex_guid[5] = buffer->ReadBit(); else buffer->WriteBit(m_extra.ex_guid[5]); break;
            case 6: if(read) m_extra.ex_guid[0] = buffer->ReadBit(); else buffer->WriteBit(m_extra.ex_guid[0]); break;
            case 7: if(read) m_extra.ex_guid[1] = buffer->ReadBit(); else buffer->WriteBit(m_extra.ex_guid[1]); break;
            case 8: if(read) m_extra.ex_guid[3] = buffer->ReadBit(); else buffer->WriteBit(m_extra.ex_guid[3]); break;
            case 9: if(read) buffer->ReadByteSeq(m_extra.ex_guid[6]); else buffer->WriteByteSeq(m_extra.ex_guid[6]); break;
            case 10: if(read) buffer->ReadByteSeq(m_extra.ex_guid[1]); else buffer->WriteByteSeq(m_extra.ex_guid[1]); break;
            case 11: if(read) buffer->ReadByteSeq(m_extra.ex_guid[2]); else buffer->WriteByteSeq(m_extra.ex_guid[2]); break;
            case 12: if(read) buffer->ReadByteSeq(m_extra.ex_guid[5]); else buffer->WriteByteSeq(m_extra.ex_guid[5]); break;
            case 13: if(read) buffer->ReadByteSeq(m_extra.ex_guid[3]); else buffer->WriteByteSeq(m_extra.ex_guid[3]); break;
            case 14: if(read) buffer->ReadByteSeq(m_extra.ex_guid[0]); else buffer->WriteByteSeq(m_extra.ex_guid[0]); break;
            case 15: if(read) buffer->ReadByteSeq(m_extra.ex_guid[4]); else buffer->WriteByteSeq(m_extra.ex_guid[4]); break;
            case 16: if(read) buffer->ReadByteSeq(m_extra.ex_guid[7]); else buffer->WriteByteSeq(m_extra.ex_guid[7]); break;
            };
        }break;
    }
}

void MovementInterface::SetActiveMover(WorldPacket *packet)
{

}

void MovementInterface::MoveTimeSkipped(WorldPacket *packet)
{

}

void MovementInterface::MoveSplineComplete(WorldPacket *packet)
{

}

void MovementInterface::MoveFallReset(WorldPacket *packet)
{

}

#define DO_BIT(buff, val, result) buff->WriteBit(val ? (result ? 1 : 0) : (result ? 0 : 1));
#define DO_COND_BIT(buff, cond, val, result) if(cond) buff->WriteBit(val ? result : !result);
#define DO_BYTES(buff, type, val) buff->append<type>(val);
#define DO_COND_BYTES(buff, cond, type, val) if(cond) buff->append<type>(val);
#define DO_SEQ_BYTE(buff, val) buff->WriteByteSeq(val);

void MovementInterface::AppendSplineData(bool bits, ByteBuffer *buffer)
{

}

void MovementInterface::WriteObjectUpdate(ByteBuffer *bits, ByteBuffer *bytes)
{
    WoWGuid unitGuid = m_Unit->GetGUID();
    bool hasMovementFlags = m_movementFlagMask & 0x0F, hasMovementFlags2 = m_movementFlagMask & 0xF0,
    hasOrientation = !G3D::fuzzyEq(m_serverLocation->o, 0.0f), hasTransportData = !m_transportGuid.empty(),
    hasSpline = isSplineMovingActive(), hasTransportTime2 = (hasTransportData && m_transportTime2 != 0), hasTransportVehicleId = (hasTransportData && m_vehicleId != 0),
    hasPitch = (hasFlag(MOVEMENTFLAG_SWIMMING) || hasFlag(MOVEMENTFLAG_FLYING) || hasFlag(MOVEMENTFLAG_ALWAYS_ALLOW_PITCHING)),
    hasFallDirection = hasFlag(MOVEMENTFLAG_TOGGLE_FALLING), hasFallData = (hasFallDirection || m_jumpTime != 0), hasSplineElevation = hasFlag(MOVEMENTFLAG_SPLINE_ELEVATION);
    // Append our bits
    DO_BIT(bits, hasMovementFlags, false);
    DO_BIT(bits, hasOrientation, false);
    DO_BIT(bits, unitGuid[7], true);
    DO_BIT(bits, unitGuid[3], true);
    DO_BIT(bits, unitGuid[2], true);
    if (hasMovementFlags) HandleMovementFlags(false, bits);
    DO_BIT(bits, hasSpline && m_Unit->IsPlayer(), true);
    DO_BIT(bits, hasPitch, false);
    DO_BIT(bits, hasSpline, true);
    DO_BIT(bits, hasFallData, true);
    DO_BIT(bits, hasSplineElevation, false);
    DO_BIT(bits, unitGuid[5], true);
    DO_BIT(bits, hasTransportData, true);
    DO_BIT(bits, true, false); // !Hastimestamp
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[1], true);
    DO_COND_BIT(bits, hasTransportData, hasTransportTime2, true);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[4], true);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[0], true);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[6], true);
    DO_COND_BIT(bits, hasTransportData, hasTransportVehicleId, true);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[7], true);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[5], true);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[3], true);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[2], true);
    DO_BIT(bits, unitGuid[4], true);
    if(hasSpline) AppendSplineData(true, bits);
    DO_BIT(bits, unitGuid[6], true);
    DO_COND_BIT(bits, hasFallData, hasFallDirection, true);
    DO_BIT(bits, unitGuid[0], true);
    DO_BIT(bits, unitGuid[1], true);
    DO_BIT(bits, true, false);
    DO_BIT(bits, hasMovementFlags2, false);
    if (hasMovementFlags2) HandleMovementFlags2(false, bits);
    // Append our bytes
    DO_SEQ_BYTE(bytes, unitGuid[4]);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_RUN_BACK));
    DO_COND_BYTES(bytes, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(bytes, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(bytes, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(bytes, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(bytes, hasFallData, float, m_jumpZSpeed);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_SWIM_BACK));
    DO_COND_BYTES(bytes, hasSplineElevation, float, splineElevation);
    if(hasSpline) AppendSplineData(false, bytes);// Write spline data
    DO_BYTES(bytes, float, m_serverLocation->z-m_heightOffset);
    DO_SEQ_BYTE(bytes, unitGuid[5]);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[5], uint8, m_transportGuid[5]);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[7], uint8, m_transportGuid[7]);
    DO_COND_BYTES(bytes, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(bytes, hasTransportData, float, m_transportLocation.o);
    DO_COND_BYTES(bytes, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(bytes, hasTransportData, float, m_transportLocation.y);
    DO_COND_BYTES(bytes, hasTransportData, float, m_transportLocation.x);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[3], uint8, m_transportGuid[3]);
    DO_COND_BYTES(bytes, hasTransportData, float, m_transportLocation.z);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[0], uint8, m_transportGuid[0]);
    DO_COND_BYTES(bytes, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(bytes, hasTransportData, uint32, m_transportSeatId);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[1], uint8, m_transportGuid[1]);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[6], uint8, m_transportGuid[6]);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[2], uint8, m_transportGuid[2]);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[4], uint8, m_transportGuid[4]);
    DO_BYTES(bytes, float, m_serverLocation->x);
    DO_BYTES(bytes, float,  GetMoveSpeed(MOVE_SPEED_PITCHRATE));
    DO_SEQ_BYTE(bytes, unitGuid[3]);
    DO_SEQ_BYTE(bytes, unitGuid[0]);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_SWIM));
    DO_BYTES(bytes, float, m_serverLocation->y);
    DO_SEQ_BYTE(bytes, unitGuid[7]);
    DO_SEQ_BYTE(bytes, unitGuid[1]);
    DO_SEQ_BYTE(bytes, unitGuid[2]);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_WALK));
    DO_BYTES(bytes, uint32, getMSTime());
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_TURNRATE));
    DO_SEQ_BYTE(bytes, unitGuid[6]);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_FLY));
    DO_COND_BYTES(bytes, hasOrientation, float, m_serverLocation->o);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_RUN));
    DO_COND_BYTES(bytes, hasPitch, float, pitching);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_FLY_BACK));
}

#undef DO_BIT
#undef DO_COND_BIT
#undef DO_BYTES
#undef DO_COND_BYTES
#undef DO_SEQ_BYTE

void MovementInterface::UpdateModifier(uint32 auraSlot, uint8 index, Modifier *mod, bool apply)
{
    m_modifierMask.SetBit(mod->m_type);

    std::pair<uint32, uint32> mod_index = std::make_pair(auraSlot, index);
    if(apply)
    {
        ModifierHolder *modHolder = NULL;
        if(m_modifierHolders.find(auraSlot) != m_modifierHolders.end())
            modHolder = m_modifierHolders.at(auraSlot);
        else
        {
            modHolder = new ModifierHolder(auraSlot, mod->m_spellInfo);
            m_modifierHolders.insert(std::make_pair(auraSlot, modHolder));
        }
        if(modHolder == NULL || modHolder->mod[index] == mod)
            return;

        m_modifiersByModType[mod->m_type].insert(std::make_pair(mod_index, mod));
        modHolder->mod[index] = mod;
    }
    else if(m_modifierHolders.find(auraSlot) != m_modifierHolders.end())
    {
        m_modifiersByModType[mod->m_type].erase(mod_index);

        ModifierHolder *modHolder = m_modifierHolders.at(auraSlot);
        modHolder->mod[index] = NULL;
        for(uint8 i=0;i<3;i++)
            if(modHolder->mod[i])
                return;
        m_modifierHolders.erase(auraSlot);
        delete modHolder;
    }

    if(mod->m_type == SPELL_AURA_ADD_FLAT_MODIFIER || mod->m_type == SPELL_AURA_ADD_PCT_MODIFIER)
        UpdateSpellGroupModifiers(apply, mod);
}

void MovementInterface::UpdateSpellGroupModifiers(bool apply, Modifier *mod)
{
    assert(mod->m_miscValue[0] < SPELL_MODIFIERS);
    std::pair<uint8, uint8> index = std::make_pair(uint8(mod->m_miscValue[0] & 0x7F), uint8(mod->m_type == SPELL_AURA_ADD_PCT_MODIFIER ? 1 : 0));
    std::map<uint8, int32> groupModMap = m_spellGroupModifiers[index];

    uint32 count = 0;
    WorldPacket data(SMSG_SET_FLAT_SPELL_MODIFIER+index.second, 20);
    data << uint32(1) << count << uint8(index.first);
    for(uint32 bit = 0, intbit = 0; bit < SPELL_GROUPS; ++bit, ++intbit)
    {
        if(bit && (bit%32 == 0)) ++intbit;
        if( ( 1 << bit%32 ) & mod->m_spellInfo->EffectSpellClassMask[mod->i][intbit] )
        {
            if(apply) groupModMap[bit] += mod->m_amount;
            else groupModMap[bit] -= mod->m_amount;
            data << uint8(bit);
            data << groupModMap[bit];
            count++;
        }
    }
    data.put<uint32>(4, count);
    if(m_Unit->IsPlayer())
        castPtr<Player>(m_Unit)->SendPacket(&data);
}
