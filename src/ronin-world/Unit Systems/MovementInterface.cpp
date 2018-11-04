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

static float m_defaultSpeeds[MOVE_SPEED_MAX] = { 2.5f, 7.f, 4.5f, 4.722222f, 2.5f, 3.141593f, 7.f, 4.5f, 3.141593f };

MovementInterface::MovementInterface(Unit *_unit) : m_Unit(_unit), m_clientGuid(), m_moverGuid(), m_movementState(0), m_underwaterState(0), m_breathingUpdateTimer(0), m_incrementMoveCounter(false), m_serverCounter(0), m_clientCounter(0), m_movementFlagMask(0), m_path(_unit), m_timeSyncCounter(0), m_moveAckCounter(0)
{
    for(uint8 i = 0; i < MOVE_SPEED_MAX; i++)
    {
        m_currSpeeds[i] = m_defaultSpeeds[i];
        m_speedOffset[i] = m_pendingSpeeds[i] = 0.f;
        m_speedTimers[i] = 0;
    }
    for(uint8 i = 0; i < MOVEMENT_STATUS_MAX; i++)
        m_pendingEnable[i] = false;

    // Sizeof1
    memset(m_movementFlags, 0, sizeof(uint8)*6);
    memset(m_serverFlags, 0, sizeof(uint8)*6);

    m_transportGuid.Clean();
    m_clientTransGuid.Clean();

    m_serverLocation = m_Unit->GetPositionV();
    m_clientLocation.ChangeCoords(0.f, 0.f, 0.f, 0.f);
    m_clientTransLocation.ChangeCoords(0.f, 0.f, 0.f, 0.f);
    m_clientTime = 0, m_serverTime = getMSTime(), m_jumpTime = 0, m_transportTime = 0, m_transportTime2 = 0;
    m_transportSeatId = 0, m_vehicleId = 0;
    pitching = splineElevation = 0.f;
    m_jumpZSpeed = m_jump_XYSpeed = m_jump_sin = m_jump_cos = 0.f;

    m_pendingDest = false;
    m_destMapId = m_destInstanceId = 0;
    m_teleportLocation.ChangeCoords(0.f, 0.f, 0.f, 0.f);

    m_underwaterState = 0;
    m_LastUnderwaterState = 0;
    m_MirrorTimer[0] = m_MirrorTimer[1] = m_MirrorTimer[2] = -1;
    m_UnderwaterTime = 180000;

    m_pendingDataTimer = 0xFFFFFFFF;
    m_collisionHeight = 0.f;
    m_isKnockBacked = false;
    m_isFalling = false;
    m_fallPointZ = 0.f;

    ClearTransportData();
    m_extra.clear();
}

MovementInterface::~MovementInterface()
{
    m_serverLocation = NULL;
}

void MovementInterface::AttachToOwner()
{
    m_clientGuid = m_moverGuid = m_Unit->GetGUID();
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
    &MovementInterface::HandleGravityDisable,
    &MovementInterface::HandleGravityEnable,
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
    &MovementInterface::HandleUpdateSwimBackSpeed,
    &MovementInterface::HandleUpdateFlightSpeed,
    &MovementInterface::HandleUpdateFlightBackSpeed,
    &MovementInterface::HandleUpdateTurnRate,
    &MovementInterface::HandleUpdatePitchRate,
    &MovementInterface::HandleSetWalkSpeed,
    &MovementInterface::HandleSetRunSpeed,
    &MovementInterface::HandleSetRunBackSpeed,
    &MovementInterface::HandleSetSwimSpeed,
    &MovementInterface::HandleSetSwimBackSpeed,
    &MovementInterface::HandleSetFlightSpeed,
    &MovementInterface::HandleSetFlightBackSpeed,
    &MovementInterface::HandleSetTurnRate,
    &MovementInterface::HandleSetPitchRate,
    // Acknowledgement codes
    &MovementInterface::HandleAckTeleport,
    &MovementInterface::HandleAckRoot,
    &MovementInterface::HandleAckUnroot,
    &MovementInterface::HandleAckFeatherFall,
    &MovementInterface::HandleAckGravityEnable,
    &MovementInterface::HandleAckGravityDisable,
    &MovementInterface::HandleAckHover,
    &MovementInterface::HandleAckWaterWalk,
    &MovementInterface::HandleAckKnockBack,
    &MovementInterface::HandleAckSetCanFly,
    &MovementInterface::HandleAckSetCollisionHeight,
    &MovementInterface::HandleAckSetCanTransitionBetweenSwimAndFly,
    &MovementInterface::HandleAckForceWalkSpeedChange,
    &MovementInterface::HandleAckForceRunSpeedChange,
    &MovementInterface::HandleAckForceRunBackSpeedChange,
    &MovementInterface::HandleAckForceSwimSpeedChange,
    &MovementInterface::HandleAckForceSwimBackSpeedChange,
    &MovementInterface::HandleAckForceFlightSpeedChange,
    &MovementInterface::HandleAckForceFlightBackSpeedChange,
    &MovementInterface::HandleAckForceTurnRateChange,
    &MovementInterface::HandleAckForcePitchRateChange,
    // Spline codes
    &MovementInterface::HandleSplineDone,
    &MovementInterface::HandleSplineSetWalkSpeed,
    &MovementInterface::HandleSplineSetRunSpeed,
    &MovementInterface::HandleSplineSetRunBackSpeed,
    &MovementInterface::HandleSplineSetSwimSpeed,
    &MovementInterface::HandleSplineSetSwimBackSpeed,
    &MovementInterface::HandleSplineSetFlightSpeed,
    &MovementInterface::HandleSplineSetFlightBackSpeed,
    &MovementInterface::HandleSplineSetTurnRate,
    &MovementInterface::HandleSplineSetPitchRate,
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
    case SMSG_MOVE_GRAVITY_DISABLE: return MOVEMENT_CODE_GRAVITY_DISABLE;
    case SMSG_MOVE_GRAVITY_ENABLE: return MOVEMENT_CODE_GRAVITY_ENABLE;
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
    case SMSG_MOVE_UPDATE_SWIM_BACK_SPEED: return MOVEMENT_CODE_UPDATE_SWIM_BACK_SPEED;
    case SMSG_MOVE_UPDATE_FLIGHT_SPEED: return MOVEMENT_CODE_UPDATE_FLIGHT_SPEED;
    case SMSG_MOVE_UPDATE_FLIGHT_BACK_SPEED: return MOVEMENT_CODE_UPDATE_FLIGHT_BACK_SPEED;
    case SMSG_MOVE_UPDATE_TURN_RATE: return MOVEMENT_CODE_UPDATE_TURN_RATE;
    case SMSG_MOVE_UPDATE_PITCH_RATE: return MOVEMENT_CODE_UPDATE_PITCH_RATE;

    case SMSG_MOVE_SET_WALK_SPEED: return MOVEMENT_CODE_SET_WALK_SPEED;
    case SMSG_MOVE_SET_RUN_SPEED: return MOVEMENT_CODE_SET_RUN_SPEED;
    case SMSG_MOVE_SET_RUN_BACK_SPEED: return MOVEMENT_CODE_SET_RUN_BACK_SPEED;
    case SMSG_MOVE_SET_SWIM_SPEED: return MOVEMENT_CODE_SET_SWIM_SPEED;
    case SMSG_MOVE_SET_SWIM_BACK_SPEED: return MOVEMENT_CODE_SET_SWIM_BACK_SPEED;
    case SMSG_MOVE_SET_FLIGHT_SPEED: return MOVEMENT_CODE_SET_FLIGHT_SPEED;
    case SMSG_MOVE_SET_FLIGHT_BACK_SPEED: return MOVEMENT_CODE_SET_FLIGHT_BACK_SPEED;
    case SMSG_MOVE_SET_TURN_RATE: return MOVEMENT_CODE_SET_TURN_RATE;
    case SMSG_MOVE_SET_PITCH_RATE: return MOVEMENT_CODE_SET_PITCH_RATE;

        // Acknowledgement codes
    case MSG_MOVE_WORLDPORT_ACK: return MOVEMENT_CODE_ACK_TELEPORT;
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
    case CMSG_MOVE_FORCE_SWIM_BACK_SPEED_CHANGE_ACK: return MOVEMENT_CODE_ACK_FORCE_SWIM_BACK_SPEED_CHANGE;
    case CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK: return MOVEMENT_CODE_ACK_FORCE_FLIGHT_SPEED_CHANGE;
    case CMSG_MOVE_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK: return MOVEMENT_CODE_ACK_FORCE_FLIGHT_BACK_SPEED_CHANGE;
    case CMSG_MOVE_FORCE_TURN_RATE_CHANGE_ACK: return MOVEMENT_CODE_ACK_FORCE_TURN_RATE_CHANGE;
    case CMSG_MOVE_FORCE_PITCH_RATE_CHANGE_ACK: return MOVEMENT_CODE_ACK_FORCE_PITCH_RATE_CHANGE;

        // Spline codes
    case CMSG_MOVE_SPLINE_DONE: return MOVEMENT_CODE_SPLINE_DONE;
    case SMSG_SPLINE_MOVE_SET_WALK_SPEED: return MOVEMENT_CODE_SPLINE_SET_WALK_SPEED;
    case SMSG_SPLINE_MOVE_SET_RUN_SPEED: return MOVEMENT_CODE_SPLINE_SET_RUN_SPEED;
    case SMSG_SPLINE_MOVE_SET_RUN_BACK_SPEED: return MOVEMENT_CODE_SPLINE_SET_RUN_BACK_SPEED;
    case SMSG_SPLINE_MOVE_SET_SWIM_SPEED: return MOVEMENT_CODE_SPLINE_SET_SWIM_SPEED;
    case SMSG_SPLINE_MOVE_SET_SWIM_BACK_SPEED: return MOVEMENT_CODE_SPLINE_SET_SWIM_BACK_SPEED;
    case SMSG_SPLINE_MOVE_SET_FLIGHT_SPEED: return MOVEMENT_CODE_SPLINE_SET_FLIGHT_SPEED;
    case SMSG_SPLINE_MOVE_SET_FLIGHT_BACK_SPEED: return MOVEMENT_CODE_SPLINE_SET_FLIGHT_BACK_SPEED;
    case SMSG_SPLINE_MOVE_SET_TURN_RATE: return MOVEMENT_CODE_SPLINE_SET_TURN_RATE;
    case SMSG_SPLINE_MOVE_SET_PITCH_RATE: return MOVEMENT_CODE_SPLINE_SET_PITCH_RATE;
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

uint16 MovementInterface::GetSpeedTypeForMoveCode(uint16 moveCode)
{
    switch(moveCode)
    {
    case MOVEMENT_CODE_UPDATE_WALK_SPEED:       return MOVE_SPEED_WALK;
    case MOVEMENT_CODE_UPDATE_RUN_SPEED:        return MOVE_SPEED_RUN;
    case MOVEMENT_CODE_UPDATE_RUN_BACK_SPEED:   return MOVE_SPEED_RUN_BACK;
    case MOVEMENT_CODE_UPDATE_SWIM_SPEED:       return MOVE_SPEED_SWIM;
    case MOVEMENT_CODE_UPDATE_SWIM_BACK_SPEED:  return MOVE_SPEED_SWIM_BACK;
    case MOVEMENT_CODE_UPDATE_FLIGHT_SPEED:     return MOVE_SPEED_FLIGHT;
    case MOVEMENT_CODE_UPDATE_FLIGHT_BACK_SPEED:return MOVE_SPEED_FLIGHT_BACK;
    case MOVEMENT_CODE_UPDATE_TURN_RATE:        return MOVE_SPEED_TURNRATE;
    case MOVEMENT_CODE_UPDATE_PITCH_RATE:       return MOVE_SPEED_PITCHRATE;

    case MOVEMENT_CODE_SET_WALK_SPEED:          return MOVE_SPEED_WALK;
    case MOVEMENT_CODE_SET_RUN_SPEED:           return MOVE_SPEED_RUN;
    case MOVEMENT_CODE_SET_RUN_BACK_SPEED:      return MOVE_SPEED_RUN_BACK;
    case MOVEMENT_CODE_SET_SWIM_SPEED:          return MOVE_SPEED_SWIM;
    case MOVEMENT_CODE_SET_SWIM_BACK_SPEED:     return MOVE_SPEED_SWIM_BACK;
    case MOVEMENT_CODE_SET_FLIGHT_SPEED:        return MOVE_SPEED_FLIGHT;
    case MOVEMENT_CODE_SET_FLIGHT_BACK_SPEED:   return MOVE_SPEED_FLIGHT_BACK;
    case MOVEMENT_CODE_SET_TURN_RATE:           return MOVE_SPEED_TURNRATE;
    case MOVEMENT_CODE_SET_PITCH_RATE:          return MOVE_SPEED_PITCHRATE;

    case MOVEMENT_CODE_ACK_FORCE_WALK_SPEED_CHANGE:     return MOVE_SPEED_WALK;
    case MOVEMENT_CODE_ACK_FORCE_RUN_SPEED_CHANGE:      return MOVE_SPEED_RUN;
    case MOVEMENT_CODE_ACK_FORCE_RUN_BACK_SPEED_CHANGE: return MOVE_SPEED_RUN_BACK;
    case MOVEMENT_CODE_ACK_FORCE_SWIM_SPEED_CHANGE:     return MOVE_SPEED_SWIM;
    case MOVEMENT_CODE_ACK_FORCE_SWIM_BACK_SPEED_CHANGE:return MOVE_SPEED_SWIM_BACK;
    case MOVEMENT_CODE_ACK_FORCE_FLIGHT_SPEED_CHANGE:   return MOVE_SPEED_FLIGHT;
    case MOVEMENT_CODE_ACK_FORCE_FLIGHT_BACK_SPEED_CHANGE:return MOVE_SPEED_FLIGHT_BACK;
    case MOVEMENT_CODE_ACK_FORCE_TURN_RATE_CHANGE:      return MOVE_SPEED_TURNRATE;
    case MOVEMENT_CODE_ACK_FORCE_PITCH_RATE_CHANGE:     return MOVE_SPEED_PITCHRATE;

    case MOVEMENT_CODE_SPLINE_SET_WALK_SPEED:       return MOVE_SPEED_WALK;
    case MOVEMENT_CODE_SPLINE_SET_RUN_SPEED:        return MOVE_SPEED_RUN;
    case MOVEMENT_CODE_SPLINE_SET_RUN_BACK_SPEED:   return MOVE_SPEED_RUN_BACK;
    case MOVEMENT_CODE_SPLINE_SET_SWIM_SPEED:       return MOVE_SPEED_SWIM;
    case MOVEMENT_CODE_SPLINE_SET_SWIM_BACK_SPEED:  return MOVE_SPEED_SWIM_BACK;
    case MOVEMENT_CODE_SPLINE_SET_FLIGHT_SPEED:     return MOVE_SPEED_FLIGHT;
    case MOVEMENT_CODE_SPLINE_SET_FLIGHT_BACK_SPEED:return MOVE_SPEED_FLIGHT_BACK;
    case MOVEMENT_CODE_SPLINE_SET_TURN_RATE:        return MOVE_SPEED_TURNRATE;
    case MOVEMENT_CODE_SPLINE_SET_PITCH_RATE:       return MOVE_SPEED_PITCHRATE;
    }
    return 0xFFFF;
}

void MovementInterface::Update(uint32 msTime, uint32 uiDiff)
{
    for(uint8 i = 0; i < MOVE_SPEED_MAX; i++)
    {
        if(m_speedTimers[i])
        {
            if(m_speedTimers[i] <= uiDiff)
            {
                m_speedTimers[i] = 0; // Timer not needed anymore
                m_currSpeeds[i] = m_pendingSpeeds[i];
                m_pendingSpeeds[i] = 0.0f; // Pending speed can be cleared
            } else m_speedTimers[i] -= uiDiff;
        }
    }

    HandleBreathing(uiDiff);

    // Pending data timing handler
    if(m_pendingDataTimer > uiDiff)
        m_pendingDataTimer -= uiDiff;
    else HandlePendingMoveData(false);

    if(m_Unit->isCasting() && m_Unit->GetSpellInterface()->GetCurrentSpellProto()->isSpellInterruptOnMovement())
        return;

    // If path update returns true, it means we have no current path
    if (m_path.Update(msTime, uiDiff, true) && m_movementState)
    {   // We have a clean path and a unit state, generate next target location
        if(int moveMask = RONIN_UTIL::getRBitOffset(m_movementState))
        {
            sLog.printf("Moving with mask %08X\n", 1<<moveMask);
            switch(moveMask)
            {

            }
        }
    }
}

void MovementInterface::UpdatePreWrite(uint16 opcode, uint16 moveCode)
{
    m_serverTime = getMSTime();

}

static Opcodes const movementSpeedToOpcode[MOVE_SPEED_MAX][3] =
{
    {SMSG_SPLINE_MOVE_SET_WALK_SPEED,        SMSG_MOVE_SET_WALK_SPEED,        SMSG_MOVE_UPDATE_WALK_SPEED       },
    {SMSG_SPLINE_MOVE_SET_RUN_SPEED,         SMSG_MOVE_SET_RUN_SPEED,         SMSG_MOVE_UPDATE_RUN_SPEED        },
    {SMSG_SPLINE_MOVE_SET_RUN_BACK_SPEED,    SMSG_MOVE_SET_RUN_BACK_SPEED,    SMSG_MOVE_UPDATE_RUN_BACK_SPEED   },
    {SMSG_SPLINE_MOVE_SET_SWIM_SPEED,        SMSG_MOVE_SET_SWIM_SPEED,        SMSG_MOVE_UPDATE_SWIM_SPEED       },
    {SMSG_SPLINE_MOVE_SET_SWIM_BACK_SPEED,   SMSG_MOVE_SET_SWIM_BACK_SPEED,   SMSG_MOVE_UPDATE_SWIM_BACK_SPEED  },
    {SMSG_SPLINE_MOVE_SET_TURN_RATE,         SMSG_MOVE_SET_TURN_RATE,         SMSG_MOVE_UPDATE_TURN_RATE        },
    {SMSG_SPLINE_MOVE_SET_FLIGHT_SPEED,      SMSG_MOVE_SET_FLIGHT_SPEED,      SMSG_MOVE_UPDATE_FLIGHT_SPEED     },
    {SMSG_SPLINE_MOVE_SET_FLIGHT_BACK_SPEED, SMSG_MOVE_SET_FLIGHT_BACK_SPEED, SMSG_MOVE_UPDATE_FLIGHT_BACK_SPEED},
    {SMSG_SPLINE_MOVE_SET_PITCH_RATE,        SMSG_MOVE_SET_PITCH_RATE,        SMSG_MOVE_UPDATE_PITCH_RATE       },
};

bool MovementInterface::UpdatePostRead(uint16 opcode, uint16 moveCode, ByteBuffer *source)
{
    if(!TerrainMgr::AreCoordinatesValid(m_clientLocation.x, m_clientLocation.y))
        return false;

    // Normalize the client orientation here
    m_clientLocation.o = NormAngle(m_clientLocation.o);

    UpdateMovementFlagMask();
    if(!UpdateAcknowledgementData(moveCode))
        return false;
    if(!UpdateMovementData(moveCode, source != NULL))
        return false;

    m_Unit->SetPosition(m_clientLocation.x, m_clientLocation.y, m_clientLocation.z, m_clientLocation.o);

    if(m_Unit->IsPlayer())
    {
        if(m_isFalling == false && (moveCode == MOVEMENT_CODE_JUMP || (m_isFalling = (hasFlag(MOVEMENTFLAG_TOGGLE_FALLING) || hasFlag(MOVEMENTFLAG_TOGGLE_FALLING_FAR)))))
            m_fallPointZ = m_clientLocation.z;
        else if(m_isFalling == true && moveCode == MOVEMENT_CODE_JUMP)
            return false;
        else if(moveCode == MOVEMENT_CODE_FALL_LAND || (m_isFalling && (!(hasFlag(MOVEMENTFLAG_TOGGLE_FALLING) || hasFlag(MOVEMENTFLAG_TOGGLE_FALLING_FAR)))))
        {
            HandlePendingMoveData(true);
            m_isFalling = false;
            m_fallPointZ = 0.f;
        } else HandlePendingMoveData(false);
    } else m_fallPointZ = 0.f, m_isFalling = false;

    switch(moveCode)
    {
    case MOVEMENT_CODE_ACK_FORCE_WALK_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_RUN_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_RUN_BACK_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_SWIM_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_SWIM_BACK_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_FLIGHT_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_FLIGHT_BACK_SPEED_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_TURN_RATE_CHANGE:
    case MOVEMENT_CODE_ACK_FORCE_PITCH_RATE_CHANGE:
        {
            if(source != NULL)
            {
                uint16 speedType;
                if((speedType = GetSpeedTypeForMoveCode(moveCode)) != 0xFFFF)
                {
                    WorldPacket data(movementSpeedToOpcode[speedType][2], source->size());
                    WriteFromServer(movementSpeedToOpcode[speedType][2], &data, m_extra.ex_guid, m_extra.ex_float, m_extra.ex_byte);
                    m_Unit->SendMessageToSet(&data, false);
                }
            }
        }break;
    case MOVEMENT_CODE_ACK_TELEPORT:
        {
            m_destMapId = m_destInstanceId = 0;
            m_teleportLocation.ChangeCoords(0.f, 0.f, 0.f, 0.f);
        }break;
    }

    return true;
}

void MovementInterface::RecalculateMoveSpeed(MovementSpeedTypes speedType)
{
    SetMoveSpeed(speedType, _CalculateSpeed(speedType));
}

void MovementInterface::SetMoveSpeed(MovementSpeedTypes speedType, float speed)
{
    if(m_Unit->IsPlayer() && m_Unit->IsInWorld())
    {
        if(m_currSpeeds[speedType] == speed || m_pendingSpeeds[speedType] == speed)
            return;

        m_speedTimers[speedType] = std::min<uint32>(2000+castPtr<Player>(m_Unit)->GetSession()->GetLatency()*15, 7500);
        m_speedCounters[speedType]++;
        m_pendingSpeeds[speedType] = speed;

        WorldPacket data(movementSpeedToOpcode[speedType][1], 50);
        WriteFromServer(movementSpeedToOpcode[speedType][1], &data, m_Unit->GetGUID(), speed);
        castPtr<Player>(m_Unit)->PushPacket(&data);
        m_path.OnSpeedChange(speedType);
    }
    else
    {
        m_currSpeeds[speedType] = speed;
        if(!m_Unit->IsInWorld())
            return;

        WorldPacket data(movementSpeedToOpcode[speedType][0], 50);
        WriteFromServer(movementSpeedToOpcode[speedType][0], &data, m_Unit->GetGUID(), speed);
        m_Unit->SendMessageToSet(&data, false);
        m_path.OnSpeedChange(speedType);
    }
}

bool MovementInterface::UpdateAcknowledgementData(uint16 moveCode)
{
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
    case MOVEMENT_CODE_ACK_FORCE_FLIGHT_SPEED_CHANGE: if(!UpdatePendingSpeed(MOVE_SPEED_FLIGHT, m_extra.ex_float)) return false; break;
    }
    return true;
}

bool MovementInterface::UpdateMovementData(uint16 moveCode, bool distribute)
{
    switch(moveCode)
    {
    case MOVEMENT_CODE_HEARTBEAT:
    case MOVEMENT_CODE_JUMP:
    case MOVEMENT_CODE_FALL_LAND:
    case MOVEMENT_CODE_START_FORWARD:
    case MOVEMENT_CODE_START_BACKWARD:
    case MOVEMENT_CODE_START_STRAFE_LEFT:
    case MOVEMENT_CODE_START_STRAFE_RIGHT:
    case MOVEMENT_CODE_START_TURN_LEFT:
    case MOVEMENT_CODE_START_TURN_RIGHT:
    case MOVEMENT_CODE_START_PITCH_DOWN:
    case MOVEMENT_CODE_START_PITCH_UP:
    case MOVEMENT_CODE_START_ASCEND:
    case MOVEMENT_CODE_START_DESCEND:
    case MOVEMENT_CODE_START_SWIM:
    case MOVEMENT_CODE_STOP:
    case MOVEMENT_CODE_STOP_STRAFE:
    case MOVEMENT_CODE_STOP_TURN:
    case MOVEMENT_CODE_STOP_PITCH:
    case MOVEMENT_CODE_STOP_ASCEND:
    case MOVEMENT_CODE_STOP_SWIM:
    case MOVEMENT_CODE_SET_FACING:
    case MOVEMENT_CODE_SET_PITCH:
        break;
    default:
        return true;
    }

    if(uint32 emoteState = m_Unit->GetUInt32Value(UNIT_NPC_EMOTESTATE))
        m_Unit->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);

    /************************************************************************/
    /* Remove Spells                                                        */
    /************************************************************************/
    uint32 flags = AURA_INTERRUPT_ON_MOVEMENT;
    if( !( hasFlag(MOVEMENTFLAG_SWIMMING) || hasFlag(MOVEMENTFLAG_TOGGLE_FALLING) ) && false)//!m_bIsWLevelSet )
        flags |= AURA_INTERRUPT_ON_LEAVE_WATER;
    if( hasFlag(MOVEMENTFLAG_SWIMMING) )
        flags |= AURA_INTERRUPT_ON_ENTER_WATER;
    if( hasFlag(MOVEMENTFLAG_MOVE_TURN_LEFT) || hasFlag(MOVEMENTFLAG_MOVE_TURN_RIGHT) )
        flags |= AURA_INTERRUPT_ON_TURNING;
    m_Unit->m_AuraInterface.RemoveAllAurasByInterruptFlag( flags );

    // Cut off before packet distribution
    if(distribute == false)
        return true;

    WorldPacket data(SMSG_PLAYER_MOVE, 500);
    WriteFromServer(SMSG_PLAYER_MOVE, &data, m_extra.ex_guid, m_extra.ex_float, m_extra.ex_byte);
    m_Unit->SendMessageToSet(&data, false);
    return true;
}

class SpeedValueCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        // Make sure we can utilize the modifier
        if(!sSpellMgr.IsAuraApplicable(_mover, mod->m_spellInfo))
            return;

        // WE can use the mod, apply it to our values
        switch(mod->m_type)
        {
            // Normalize maps
        case SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED:
            if(mod->m_amount > *modifier)
                *modifier = mod->m_amount;
            break;
            // Bonus maps
        case SPELL_AURA_MOD_VEHICLE_SPEED_ALWAYS:
        case SPELL_AURA_MOD_INCREASE_SPEED:
        case SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED:
        case SPELL_AURA_MOD_INCREASE_SWIM_SPEED:
        case SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED:
        case SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED:
        case SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED:
            if(mod->m_amount > *modifier)
                *modifier = mod->m_amount;
            break;
            // Stack maps
        case SPELL_AURA_MOD_SPEED_ALWAYS:
        case SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS:
        case SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS:
            *modifier += mod->m_amount;
            break;
            // Non stack maps
        case SPELL_AURA_MOD_SPEED_NOT_STACK:
        case SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK:
        case SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK:
            if(mod->m_amount > *modifier)
                *modifier = mod->m_amount;
            break;
            // Slow maps
        case SPELL_AURA_MOD_DECREASE_SPEED:
            if(mod->m_amount < *modifier)
                *modifier = mod->m_amount;
            break;
            // Minimum speed maps
        case SPELL_AURA_MOD_MINIMUM_SPEED:
            if(mod->m_amount > *modifier)
                *modifier = mod->m_amount;
            break;
        }
    }

    // Set our unit values
    void InitUnit(Unit *mover) { _mover = mover; }

    // Set our modifier pointer before traversing
    void InitMod(float *input) { modifier = input; }

protected:
    Unit *_mover;
    float *modifier;
};

float MovementInterface::_CalculateSpeed(MovementSpeedTypes speedType)
{
    SpeedValueCallback callback;
    bool mounted = m_Unit->GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID) != 0;
    float baseSpeed = m_defaultSpeeds[speedType], speedMod = 0.f, speedStack = 0.f;
    uint32 normalizerModMap = SPELL_AURA_TOTAL, speedBonusMap = SPELL_AURA_TOTAL, speedStackMap = SPELL_AURA_TOTAL, speedNonStackMap = SPELL_AURA_TOTAL;

    switch(speedType)
    {
    case MOVE_SPEED_RUN:
        {
            normalizerModMap = SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED;
            if(m_Unit->GetVehicleKitId())
                speedBonusMap = SPELL_AURA_MOD_VEHICLE_SPEED_ALWAYS;
            else if(mounted)
            {
                speedBonusMap = SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED;
                speedStackMap = SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS;
                speedNonStackMap = SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK;
            }
            else
            {
                speedBonusMap = SPELL_AURA_MOD_INCREASE_SPEED;
                speedStackMap = SPELL_AURA_MOD_SPEED_ALWAYS;
                speedNonStackMap = SPELL_AURA_MOD_SPEED_NOT_STACK;
            }
        }break;
    case MOVE_SPEED_SWIM:
        {
            normalizerModMap = SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED;
            speedBonusMap = SPELL_AURA_MOD_INCREASE_SWIM_SPEED;
        }break;
    case MOVE_SPEED_FLIGHT:
        {
            normalizerModMap = SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED;
            if(m_Unit->GetVehicleKitId())
                speedBonusMap = SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED;
            else if(mounted)
            {
                speedBonusMap = SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED;
                speedStackMap = SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS;
                speedNonStackMap = SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK;
            } else speedBonusMap = SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED;
        }break;
    }

    // Initialize unit for our callback
    callback.InitUnit(m_Unit);
    // Start processing map traversing
    if(speedBonusMap != SPELL_AURA_TOTAL && m_Unit->m_AuraInterface.HasAurasWithModType(speedBonusMap))
    {   // Non base speed modifier
        callback.InitMod(&speedMod);
        m_Unit->m_AuraInterface.TraverseModMap(speedBonusMap, &callback);
    }

    if(speedStackMap != SPELL_AURA_TOTAL && m_Unit->m_AuraInterface.HasAurasWithModType(speedStackMap))
    {   // Stacking speed modifiers
        callback.InitMod(&speedStack);
        m_Unit->m_AuraInterface.TraverseModMap(speedStackMap, &callback);
    }

    if(speedNonStackMap != SPELL_AURA_TOTAL && m_Unit->m_AuraInterface.HasAurasWithModType(speedNonStackMap))
    {   // Non stacking speed modifiers
        float speedNonStack = 0.f;
        callback.InitMod(&speedNonStack);
        m_Unit->m_AuraInterface.TraverseModMap(speedNonStackMap, &callback);
        if(speedNonStack > speedStack)
            speedStack = speedNonStack;
    }

    // Modify our base speed by our modifier
    baseSpeed *= (100.f+speedMod)/100.f;
    // Increase our base speed by our default speed multiplied by our stack or non stack modifiers
    baseSpeed += (m_defaultSpeeds[speedType] * speedStack)/100.f;

    if(normalizerModMap != SPELL_AURA_TOTAL && m_Unit->m_AuraInterface.HasAurasWithModType(normalizerModMap))
    {   // Normalization aura creates a speed cap based on highest modifier(though it might be lowest, not sure)
        callback.InitMod(&speedMod);
        m_Unit->m_AuraInterface.TraverseModMap(normalizerModMap, &callback);

        float maxSpeed = speedMod/m_defaultSpeeds[speedType];
        if(baseSpeed > maxSpeed) speedMod = maxSpeed;
    }

    // Reduce speed based on lowest value to create our multiplier
    if(m_Unit->m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_DECREASE_SPEED))
    {
        callback.InitMod(&speedMod);
        m_Unit->m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_DECREASE_SPEED, &callback);

        // Decrease based on default movement speed, but since decrease is always negative, grab it's float abs first
        baseSpeed -= m_defaultSpeeds[speedType] * (fabs(speedMod)/100.f);
        // Check if we have a minimum speed amount
        if(m_Unit->m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_MINIMUM_SPEED))
        {
            callback.InitMod(&speedMod);
            m_Unit->m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_MINIMUM_SPEED, &callback);
            // Adjust base speed to not be lower than returned value
            baseSpeed = std::max<float>(baseSpeed, speedMod/100.f);
        }
    }

    // SPEED OFFSETS, OFF WE GO TO SEE THE SET
    baseSpeed += m_speedOffset[speedType];
    return baseSpeed;
}

void MovementInterface::HandleBreathing(uint32 diff)
{
    m_breathingUpdateTimer += diff;
    if(m_breathingUpdateTimer < 500)
        return;
    uint32 uiDiff = m_breathingUpdateTimer;
    m_breathingUpdateTimer = 0;
    if(!m_Unit->IsPlayer())
        return;

    bool underwaterArea = false, canWalkUnderwater = hasFlag(MOVEMENTFLAG_CAN_SWIM_TO_FLY_TRANSITION);
    uint8 old_underwaterState = m_underwaterState;
    if (m_Unit->GetLiqHeight() == NO_WATER_HEIGHT)
        m_underwaterState &= ~0xFF;
    else
    {
        float HeightDelta = (m_Unit->GetLiqHeight()-m_Unit->GetPositionZ())*10.f;

        // All liquids type - check under water position
        if(m_Unit->GetLiqFlags() & (0x01|0x02|0x04|0x08) && HeightDelta > 20.f)
            m_underwaterState |= UNDERWATERSTATE_UNDERWATER;
        else m_underwaterState &= ~UNDERWATERSTATE_UNDERWATER;

        if(!m_Unit->IsPlayer() || !castPtr<Player>(m_Unit)->GetTaxiPath())
        {
            // Allow travel in dark water on taxi or transport
            if ((m_Unit->GetLiqFlags() & 0x10) && m_transportGuid.empty())
                m_underwaterState |= UNDERWATERSTATE_FATIGUE;
            else m_underwaterState &= ~UNDERWATERSTATE_FATIGUE;
        } else m_underwaterState &= ~UNDERWATERSTATE_FATIGUE;

        // in lava check, anywhere in lava level
        if (m_Unit->GetLiqFlags() & 0x04 && HeightDelta > 0.f)
            m_underwaterState |= UNDERWATERSTATE_LAVA;
        else m_underwaterState &= ~UNDERWATERSTATE_LAVA;

        // in slime check, anywhere in slime level
        if (m_Unit->GetLiqFlags() & 0x08 && (HeightDelta > 0.f || (HeightDelta > -2.5f && hasFlag(MOVEMENTFLAG_WATERWALKING))))
            m_underwaterState |= UNDERWATERSTATE_SLIME;
        else m_underwaterState &= ~UNDERWATERSTATE_SLIME;

        underwaterArea = m_Unit->HasAreaFlag(OBJECT_AREA_FLAG_UNDERWATER_AREA);
    }

    if(!m_Unit->IsPlayer())
        return;

    Player *plr = castPtr<Player>(m_Unit);
    // In water
    if (m_underwaterState & UNDERWATERSTATE_UNDERWATER && m_Unit->isAlive() && !m_Unit->m_AuraInterface.HasAurasWithModType(SPELL_AURA_WATER_BREATHING))
    {
        // Breath timer not activated - activate it
        if (m_MirrorTimer[BREATH_TIMER] == -1)
        {
            m_MirrorTimer[BREATH_TIMER] = m_UnderwaterTime;
            plr->SendMirrorTimer(BREATH_TIMER, m_MirrorTimer[BREATH_TIMER], m_MirrorTimer[BREATH_TIMER], -1);
        }
        else
        {
            m_MirrorTimer[BREATH_TIMER] -= uiDiff;

            // Timer limit - need deal damage
            if (m_MirrorTimer[BREATH_TIMER] < 0)
            {
                m_MirrorTimer[BREATH_TIMER] += 1*1000;

                // Calculate and deal damage
                uint32 damage = plr->GetMaxHealth() / 5 + RandomUInt(plr->getLevel()-1);
                plr->SendEnvironmentalDamageLog( plr->GetGUID(), DAMAGE_DROWNING, damage );
                plr->DealDamage( plr, damage, 0, 0, 0 );
            } else if (!(m_LastUnderwaterState & UNDERWATERSTATE_UNDERWATER)) // Update time in client if need
                plr->SendMirrorTimer(BREATH_TIMER, m_UnderwaterTime, m_MirrorTimer[BREATH_TIMER], -1);
        }
    }
    else if (m_MirrorTimer[BREATH_TIMER] != -1)     // Regen timer
    {
        int32 UnderWaterTime = m_UnderwaterTime;

        // Need breath regen
        m_MirrorTimer[BREATH_TIMER] += 10* uiDiff;
        if (m_MirrorTimer[BREATH_TIMER] >= UnderWaterTime || !plr->isAlive())
            plr->StopMirrorTimer(BREATH_TIMER);
        else if (m_LastUnderwaterState & UNDERWATERSTATE_UNDERWATER)
            plr->SendMirrorTimer(BREATH_TIMER, UnderWaterTime, m_MirrorTimer[BREATH_TIMER], 10);
    }

    // In dark water
    if (sWorld.EnableFatigue && (m_underwaterState & UNDERWATERSTATE_FATIGUE) && underwaterArea == false)
    {
        // Fatigue timer not activated - activate it
        if (m_MirrorTimer[FATIGUE_TIMER] == -1)
        {
            m_MirrorTimer[FATIGUE_TIMER] = 60000;
            plr->SendMirrorTimer(FATIGUE_TIMER, m_MirrorTimer[FATIGUE_TIMER], m_MirrorTimer[FATIGUE_TIMER], -1);
        }
        else
        {
            m_MirrorTimer[FATIGUE_TIMER] -= uiDiff;
            // Timer limit - need deal damage or teleport ghost to graveyard
            if (m_MirrorTimer[FATIGUE_TIMER] < 0)
            {
                m_MirrorTimer[FATIGUE_TIMER] += 1000;
                if (plr->isAlive())                                          // Calculate and deal damage
                {
                    uint32 damage = plr->GetMaxHealth() / 5 + RandomUInt(plr->getLevel()-1);
                    plr->SendEnvironmentalDamageLog( plr->GetGUID(), DAMAGE_DROWNING, damage );
                    plr->DealDamage( plr, damage, 0, 0, 0 );
                } else if (plr->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE))    // Teleport ghost to graveyard
                    plr->RepopAtGraveyard(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetMapId());
            } else if (!(m_LastUnderwaterState & UNDERWATERSTATE_FATIGUE))
                plr->SendMirrorTimer(FATIGUE_TIMER, 60000, m_MirrorTimer[FATIGUE_TIMER], -1);
        }
    }
    else if (m_MirrorTimer[FATIGUE_TIMER] != -1)       // Regen timer
    {
        int32 DarkWaterTime = 60000;
        m_MirrorTimer[FATIGUE_TIMER]+=10* uiDiff;
        if (m_MirrorTimer[FATIGUE_TIMER] >= DarkWaterTime || !plr->isAlive())
            plr->StopMirrorTimer(FATIGUE_TIMER);
        else if (m_LastUnderwaterState & UNDERWATERSTATE_FATIGUE)
            plr->SendMirrorTimer(FATIGUE_TIMER, DarkWaterTime, m_MirrorTimer[FATIGUE_TIMER], 10);
    }

    if (m_underwaterState & (UNDERWATERSTATE_LAVA|UNDERWATERSTATE_SLIME))
    {
        // Breath timer not activated - activate it
        if (m_MirrorTimer[FIRE_TIMER] == -1)
            m_MirrorTimer[FIRE_TIMER] = 3000;
        else
        {
            m_MirrorTimer[FIRE_TIMER] -= uiDiff;
            if (m_MirrorTimer[FIRE_TIMER] < 0)
            {
                m_MirrorTimer[FIRE_TIMER] += 3000;

                // Calculate and deal damage
                uint32 damage = 600+RandomUInt(100);
                if (m_underwaterState & UNDERWATERSTATE_LAVA)
                {
                    plr->SendEnvironmentalDamageLog( plr->GetGUID(), DAMAGE_LAVA, damage );
                    plr->DealDamage( plr, damage, 0, 0, 0 );
                }
                else if (plr->GetZoneId() != 1497)
                {
                    // need to skip Slime damage in Undercity,
                    plr->SendEnvironmentalDamageLog( plr->GetGUID(), DAMAGE_LAVA, damage );
                    plr->DealDamage( plr, damage, 0, 0, 0 );
                }
            }
        }
    } else m_MirrorTimer[FIRE_TIMER] = -1;

    // Recheck timers flag
    bool hasTimer = false;
    for (uint8 i = 0; i< 3; ++i)
    {
        if (m_MirrorTimer[i] != -1)
        {
            hasTimer = true;
            break;
        }
    }

    if(hasTimer)
        AddUnderwaterStateTimerPresent();
    m_LastUnderwaterState = m_underwaterState;
}

void MovementInterface::HandleMovementFlags(bool read, ByteBuffer *buffer)
{
    if(read)
    {
        // Clear previous movement flags
        m_movementFlags[0] = m_movementFlags[1] = m_movementFlags[2] = m_movementFlags[3] = 0;
        uint32 movementFlags = buffer->ReadBits(30);
        m_movementFlags[0] = (movementFlags)&0xFF;
        m_movementFlags[1] = (movementFlags>>8)&0xFF;
        m_movementFlags[2] = (movementFlags>>16)&0xFF;
        m_movementFlags[3] = (movementFlags>>24)&0xFF;
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
        m_movementFlags[4] = (movementFlags2)&0xFF;
        m_movementFlags[5] = (movementFlags2>>8)&0xFF;
    }
    else
    {
        uint16 movementFlags2 = (GetFlags(4) | (uint16(GetFlags(5))<<8));
        buffer->WriteBits(movementFlags2, 12);
    }
}

void MovementInterface::TeleportToPosition(LocationVector destination)
{
    m_teleportLocation.ChangeCoords(destination.x, destination.y, destination.z);

    WorldPacket data(SMSG_MOVE_UPDATE_TELEPORT, 38);
    WriteFromServer(SMSG_MOVE_UPDATE_TELEPORT, &data, m_extra.ex_guid, m_extra.ex_float, m_extra.ex_byte);
    if(m_Unit->IsPlayer())
    {
        Player *plr = castPtr<Player>(m_Unit);
        WorldPacket data2(MSG_MOVE_TELEPORT, 38);
        data2.WriteBit(m_moverGuid[6]);
        data2.WriteBit(m_moverGuid[0]);
        data2.WriteBit(m_moverGuid[3]);
        data2.WriteBit(m_moverGuid[2]);
        data2.WriteBit(0); // unknown
        data2.WriteBit(!m_transportGuid.empty());
        data2.WriteBit(m_moverGuid[1]);
        if (m_transportGuid)
        {
            data2.WriteBit(m_transportGuid[1]);
            data2.WriteBit(m_transportGuid[3]);
            data2.WriteBit(m_transportGuid[2]);
            data2.WriteBit(m_transportGuid[5]);
            data2.WriteBit(m_transportGuid[0]);
            data2.WriteBit(m_transportGuid[7]);
            data2.WriteBit(m_transportGuid[6]);
            data2.WriteBit(m_transportGuid[4]);
        }
        data2.WriteBit(m_moverGuid[4]);
        data2.WriteBit(m_moverGuid[7]);
        data2.WriteBit(m_moverGuid[5]);
        data2.FlushBits();

        if (m_transportGuid)
        {
            data2.WriteByteSeq(m_transportGuid[6]);
            data2.WriteByteSeq(m_transportGuid[5]);
            data2.WriteByteSeq(m_transportGuid[1]);
            data2.WriteByteSeq(m_transportGuid[7]);
            data2.WriteByteSeq(m_transportGuid[0]);
            data2.WriteByteSeq(m_transportGuid[2]);
            data2.WriteByteSeq(m_transportGuid[4]);
            data2.WriteByteSeq(m_transportGuid[3]);
        }

        data2 << uint32(m_serverCounter++); // counter
        data2.WriteByteSeq(m_moverGuid[1]);
        data2.WriteByteSeq(m_moverGuid[2]);
        data2.WriteByteSeq(m_moverGuid[3]);
        data2.WriteByteSeq(m_moverGuid[5]);
        data2 << float(destination.x);
        data2.WriteByteSeq(m_moverGuid[4]);
        data2 << float(destination.o);
        data2.WriteByteSeq(m_moverGuid[7]);
        data2 << float(destination.z);
        data2.WriteByteSeq(m_moverGuid[0]);
        data2.WriteByteSeq(m_moverGuid[6]);
        data2 << float(destination.y);
        plr->PushPacket(&data2, !plr->IsInWorld());
    } else m_Unit->SetPosition(destination);

    // Broadcast the packet to everyone except self.
    m_Unit->SendMessageToSet(&data, false);
}

void MovementInterface::TeleportToPosition(uint32 mapId, uint32 instanceId, LocationVector destination)
{
    if(!m_Unit->IsPlayer())
        return;

    m_destMapId = mapId;
    m_destInstanceId = instanceId;
    m_teleportLocation.ChangeCoords(destination.x, destination.y, destination.z, NormAngle(destination.o));

    if(m_Unit->IsInWorld())
    {
        m_pendingDest = true;
        return;
    }

    WorldPacket data(SMSG_NEW_WORLD, 20);
    data << m_teleportLocation.x << m_teleportLocation.o << m_teleportLocation.y;
    data << m_destMapId << m_teleportLocation.z;
    castPtr<Player>(m_Unit)->PushPacket( &data, true );
}

bool MovementInterface::CanProcessTimeSyncCounter(uint32 counter)
{
    bool res;
    if(res = (m_sentTimeSync.find(counter) != m_sentTimeSync.end()))
        m_sentTimeSync.erase(counter);
    return res;
}

void MovementInterface::ProcessModUpdate(uint8 modUpdateType, std::vector<uint32> modMap)
{
    bool updateFlight = false;
    std::set<MovementSpeedTypes> speedsToUpdate;
    AuraInterface *aurInterface = &m_Unit->m_AuraInterface;
    for(std::vector<uint32>::iterator itr = modMap.begin(); itr != modMap.end(); itr++)
    {
        switch(*itr)
        {   // Can fly enabling aura
        case SPELL_AURA_FLY:
            updateFlight = true;
            break;
        case SPELL_AURA_FEATHER_FALL:
            setFeatherFall(aurInterface->HasAurasWithModType(SPELL_AURA_FEATHER_FALL));
            break;
        case SPELL_AURA_WATER_WALK:
            setWaterWalk(aurInterface->HasAurasWithModType(SPELL_AURA_WATER_WALK));
            break;
        case SPELL_AURA_HOVER:
            setHover(aurInterface->HasAurasWithModType(SPELL_AURA_HOVER));
            break;
            // Root handler, stun is handled differently
        case SPELL_AURA_MOD_ROOT:
            setRooted(aurInterface->HasAurasWithModType(SPELL_AURA_MOD_ROOT));
            break;
            // Speed modifiers
        case SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED:
            updateFlight = true;
        case SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED:
        case SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED:
        case SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS:
        case SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK:
            speedsToUpdate.insert(MOVE_SPEED_FLIGHT);
            break;
        case SPELL_AURA_MOD_INCREASE_SPEED:
        case SPELL_AURA_MOD_SPEED_ALWAYS:
        case SPELL_AURA_MOD_SPEED_NOT_STACK:
        case SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED:
            speedsToUpdate.insert(MOVE_SPEED_RUN);
            break;
        case SPELL_AURA_MOD_INCREASE_SWIM_SPEED:
            speedsToUpdate.insert(MOVE_SPEED_SWIM);
            break;
        case SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS:
        case SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK:
            speedsToUpdate.insert(MOVE_SPEED_RUN);
            speedsToUpdate.insert(MOVE_SPEED_FLIGHT);
            break;
        case SPELL_AURA_MOD_MINIMUM_SPEED:
        case SPELL_AURA_MOD_DECREASE_SPEED:
        case SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED:
        default:
            speedsToUpdate.insert(MOVE_SPEED_RUN);
            speedsToUpdate.insert(MOVE_SPEED_RUN_BACK);
            speedsToUpdate.insert(MOVE_SPEED_SWIM);
            speedsToUpdate.insert(MOVE_SPEED_SWIM_BACK);
            speedsToUpdate.insert(MOVE_SPEED_FLIGHT);
            speedsToUpdate.insert(MOVE_SPEED_FLIGHT_BACK);
            break;
        }
    }

    // Update pending flight
    if(updateFlight)
        setCanFly(aurInterface->HasApplicableAurasWithModType(SPELL_AURA_FLY) || aurInterface->HasApplicableAurasWithModType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED));
    // Update pending speeds
    for(std::set<MovementSpeedTypes>::iterator itr = speedsToUpdate.begin(); itr != speedsToUpdate.end(); itr++)
        RecalculateMoveSpeed(*itr);
}

void MovementInterface::HandlePendingMoveData(bool fromLanding)
{
    // Set our pending data timer for next update
    m_pendingDataTimer = 500;

    // If we're falling we need to cut before we handle the pending data
    if(m_isFalling)
    {
        bool hoveringExclude = hasFlag(MOVEMENTFLAG_HOVER) || hasPendingMoveStatus(MOVEMENTFLAG_HOVER);
        bool flyingExclude = hasFlag(MOVEMENTFLAG_CAN_FLY) || hasPendingMoveStatus(MOVEMENT_STATUS_CANFLY);
        bool featherFallingExclude = hasFlag(MOVEMENTFLAG_FEATHERFALLING) || hasPendingMoveStatus(MOVEMENTFLAG_FEATHERFALLING);
        if(fromLanding == true && !(hasFlag(MOVEMENTFLAG_SWIMMING) || hoveringExclude || flyingExclude || featherFallingExclude))
        {   // Handle our pending falling damage
            float diff = 0.f;
            if(castPtr<Player>(m_Unit)->hasGMTag() && (diff = ((m_fallPointZ-m_clientLocation.z)-12.f)) > 0.f)
                m_Unit->DealDamage(m_Unit, float2int32(diff * 0.017f * ((float)m_Unit->GetMaxHealth())), 0, 0, 0);
            // Damage handled, data will be cleared after this
        } else return;
    }

    // Roll through our pending movement data
    while(!m_pendingMoveData.empty())
    {
        WorldPacket data;
        MovementCodes code = m_pendingMoveData.next();
        switch(code)
        {
        case MOVEMENT_CODE_UNROOT:
            data.Initialize(SMSG_MOVE_UNROOT, 200);
            WriteFromServer(data.GetOpcode(), &data);
            castPtr<Player>(m_Unit)->PushPacket(&data);
            break;
        case MOVEMENT_CODE_SPLINE_GRAVITY_ENABLE:
            data.Initialize(SMSG_SPLINE_MOVE_GRAVITY_ENABLE, 200);
            WriteFromServer(data.GetOpcode(), &data);
            castPtr<Player>(m_Unit)->PushPacket(&data);
            break;
        }
    }
}

void MovementInterface::SetFacing(float orientation)
{
    m_serverLocation->o = orientation;
}

void MovementInterface::MoveClientPosition(float x, float y, float z, float o)
{
    m_clientLocation.ChangeCoords(x, y, z, o);
    m_Unit->GetBoundBox()->Finalize(x, y, z);

    // Post read heartbeat call
    UpdatePostRead(MSG_MOVE_HEARTBEAT, MOVEMENT_CODE_HEARTBEAT, NULL);
}

void MovementInterface::SetSelfTempData(bool enable)
{
    enable ? setServerFlag(MOVEMENTFLAG_TOGGLE_ROOT) : removeServerFlag(MOVEMENTFLAG_TOGGLE_ROOT);
    enable ? setServerFlag(MOVEMENTFLAG_PENDING_STOP) : removeServerFlag(MOVEMENTFLAG_PENDING_STOP);
    enable ? setServerFlag(MOVEMENTFLAG_TOGGLE_NO_GRAVITY) : removeServerFlag(MOVEMENTFLAG_TOGGLE_NO_GRAVITY);
}

void MovementInterface::SendTimeSyncReq()
{
    if(!m_Unit->IsPlayer())
        return;

    // Send our time sync request packet
    WorldPacket data(SMSG_TIME_SYNC_REQ, 4);
    data << uint32(m_timeSyncCounter); // counter
    castPtr<Player>(m_Unit)->PushPacket(&data);
    m_sentTimeSync.insert(m_timeSyncCounter);
    m_timeSyncCounter = std::max<uint32>(0x0000000F, RandomUInt());
}

void MovementInterface::OnPreSetInWorld()
{
    for(uint8 i = 0; i < MOVE_SPEED_MAX; i++)
        RecalculateMoveSpeed(MovementSpeedTypes(i));
}

void MovementInterface::OnPrePushToWorld()
{
    bool canFly = m_Unit->canFly();
    setCanFly(canFly);

    if(canFly && isInAir())
    { m_Unit->IsPlayer() ? setServerFlag(MOVEMENTFLAG_FLYING) : setServerFlag(MOVEMENTFLAG_TOGGLE_NO_GRAVITY); }
    else { removeServerFlag(MOVEMENTFLAG_FLYING); removeServerFlag(MOVEMENTFLAG_TOGGLE_NO_GRAVITY); }
}

void MovementInterface::OnPushToWorld(uint32 msTime)
{
    UnlockTransportData();
    if(m_Unit->IsPlayer())
    {
        // Push data for our active mover
        WorldPacket data(SMSG_MOVE_SET_ACTIVE_MOVER);
        data.WriteGuidBitString(8, m_moverGuid, 5, 7, 3, 6, 0, 4, 1, 2);
        data.WriteSeqByteString(8, m_moverGuid, 6, 2, 3, 0, 5, 7, 1, 4);
        castPtr<Player>(m_Unit)->PushPacket( &data );
        // Disable player root from login/map transfer
        m_pendingMoveData.add(MOVEMENT_CODE_UNROOT);
        // Send a spline gravity enable since we don't have the regular packet parsed yet
        m_pendingMoveData.add(MOVEMENT_CODE_SPLINE_GRAVITY_ENABLE);
        // Send a time sync request
        SendTimeSyncReq();
    } else m_path.InitializeAutoPath();
}

void MovementInterface::OnRemoveFromWorld()
{
    m_pendingMoveData.clear();
    m_pendingDataTimer = 0xFFFFFFFF;
    m_timeSyncCounter = 0;
    m_moveAckCounter = 0;
    m_isFalling = false;
    m_fallPointZ = 0.f;

    removeServerFlag(MOVEMENTFLAG_MASK_A_ON_RFW);
    removeServerFlag(MOVEMENTFLAG_MASK_B_ON_RFW);
    removeServerFlag(MOVEMENTFLAG_MASK_C_ON_RFW);
    removeServerFlag(MOVEMENTFLAG_MASK_D_ON_RFW);
    removeServerFlag(MOVEMENTFLAG_MASK_E_ON_RFW);
    removeServerFlag(MOVEMENTFLAG_MASK_F_ON_RFW);
    ClearOptionalMovementData();

    if(m_Unit->IsPlayer() && m_pendingDest)
    {
        WorldPacket data(SMSG_NEW_WORLD, 20);
        data << m_teleportLocation.x << m_teleportLocation.o << m_teleportLocation.y;
        data << m_destMapId << m_teleportLocation.z;
        castPtr<Player>(m_Unit)->PushPacket( &data, true );
        m_pendingDest = false;
    }
}

void MovementInterface::OnFirstTimeSync()
{   // Update our pending data timer to activate in the next few cycles
    m_pendingDataTimer = std::min<uint32>(m_pendingDataTimer, 500);
}

void MovementInterface::OnDeath()
{
    m_path.DisableAutoPath();
    m_path.ToggleMovementLock(true);
    m_path.StopMoving();
}

void MovementInterface::OnRespawn()
{
    m_path.EnableAutoPath();
    m_path.ToggleMovementLock(false);
}

void MovementInterface::OnRepop()
{

}

void MovementInterface::OnResurrect()
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
    m_isFalling = false;
    m_fallPointZ = 0.f;
}

void MovementInterface::setRooted(bool root)
{
    if(root)
    {
        if(hasFlag(MOVEMENTFLAG_TOGGLE_ROOT))
            return;
        else if(m_Unit->IsPlayer() && m_Unit->IsInWorld() && m_pendingEnable[MOVEMENT_STATUS_ROOT])
            return;
    }
    else if(root == false)
    {
        if(!hasFlag(MOVEMENTFLAG_TOGGLE_ROOT))
            return;
        else if(m_Unit->IsPlayer() && m_Unit->IsInWorld() && m_pendingEnable[MOVEMENT_STATUS_UNROOT])
            return;
    }

    if(!m_Unit->IsPlayer() || !m_Unit->IsInWorld())
    {
        if(root)
            setServerFlag(MOVEMENTFLAG_TOGGLE_ROOT);
        else removeServerFlag(MOVEMENTFLAG_TOGGLE_ROOT);
        m_pendingEnable[MOVEMENT_STATUS_ROOT] = false;
        m_pendingEnable[MOVEMENT_STATUS_UNROOT] = false;
        return;
    }

    if(root)
        m_pendingEnable[MOVEMENT_STATUS_UNROOT] = !(m_pendingEnable[MOVEMENT_STATUS_ROOT] = true);
    else m_pendingEnable[MOVEMENT_STATUS_ROOT] = !(m_pendingEnable[MOVEMENT_STATUS_UNROOT] = true);

    WorldPacket data(root ? SMSG_MOVE_ROOT : SMSG_MOVE_UNROOT, 200);
    WriteFromServer(data.GetOpcode(), &data);
    castPtr<Player>(m_Unit)->PushPacket(&data, true);
}

bool MovementInterface::isInAir()
{
    return (RONIN_UTIL::fuzzyGt(m_serverLocation->z, m_Unit->GetGroundHeight()+0.5f) || RONIN_UTIL::fuzzyLt(m_serverLocation->z, m_Unit->GetGroundHeight()-0.5f));
}

void MovementInterface::setStunned(bool stunned)
{
    setRooted(stunned);
}

void MovementInterface::setFeatherFall(bool canFeatherFall)
{
    if(!m_Unit->IsPlayer())
        return;
    if(!m_Unit->IsInWorld())
    {
        if(canFeatherFall)
            setServerFlag(MOVEMENTFLAG_FEATHERFALLING);
        else removeServerFlag(MOVEMENTFLAG_FEATHERFALLING);
        return;
    }

    m_pendingEnable[MOVEMENT_STATUS_FEATHERFALLING] = canFeatherFall;
    WorldPacket data(canFeatherFall ? SMSG_MOVE_FEATHER_FALL : SMSG_MOVE_NORMAL_FALL, 200);
    WriteFromServer(data.GetOpcode(), &data);
    castPtr<Player>(m_Unit)->PushPacket(&data);
}

void MovementInterface::setWaterWalk(bool canWaterWalk)
{
    if(!m_Unit->IsPlayer())
        return;
    if(!m_Unit->IsInWorld())
    {
        if(canWaterWalk)
            setServerFlag(MOVEMENTFLAG_WATERWALKING);
        else removeServerFlag(MOVEMENTFLAG_WATERWALKING);
        return;
    }

    m_pendingEnable[MOVEMENT_STATUS_WATERWALKING] = canWaterWalk;
    WorldPacket data(canWaterWalk ? SMSG_MOVE_WATER_WALK : SMSG_MOVE_LAND_WALK, 200);
    WriteFromServer(data.GetOpcode(), &data);
    castPtr<Player>(m_Unit)->PushPacket(&data);
}

void MovementInterface::setHover(bool canHover)
{
    if(!m_Unit->IsPlayer())
        return;
    if(!m_Unit->IsInWorld())
    {
        if(canHover)
            setServerFlag(MOVEMENTFLAG_HOVER);
        else removeServerFlag(MOVEMENTFLAG_HOVER);
        return;
    }

    m_pendingEnable[MOVEMENT_STATUS_HOVERING] = canHover;
    WorldPacket data(canHover ? SMSG_MOVE_SET_HOVER : SMSG_MOVE_UNSET_HOVER, 200);
    WriteFromServer(data.GetOpcode(), &data);
    castPtr<Player>(m_Unit)->PushPacket(&data);
}

void MovementInterface::setCanFly(bool canFly)
{
    // Only players can have the canfly flag, because only players use the flying flag
    if(!m_Unit->IsPlayer())
        return;
    if(!m_Unit->IsInWorld())
    {
        if(canFly)
            setServerFlag(MOVEMENTFLAG_CAN_FLY);
        else removeServerFlag(MOVEMENTFLAG_CAN_FLY);
        return;
    }

    m_pendingEnable[MOVEMENT_STATUS_CANFLY] = canFly;
    WorldPacket data(canFly ? SMSG_MOVE_SET_CAN_FLY : SMSG_MOVE_UNSET_CAN_FLY, 200);
    WriteFromServer(data.GetOpcode(), &data);
    castPtr<Player>(m_Unit)->PushPacket(&data);
}

bool MovementInterface::isSplineMovingActive()
{
    if(m_Unit->IsPlayer())
        return false;
    return false;
    // Spline currently disabled until properly implemented
    return m_path.hasDestination();
}

bool MovementInterface::ReadFromClient(uint16 opcode, ByteBuffer *buffer)
{
    m_movementLock.Acquire();
    m_extra.clear();

    bool res = false;
    uint16 moveCode = 0xFFFF;
    if((moveCode = GetInternalMovementCode(opcode)) != 0xFFFF)
    {
        ClearOptionalMovementData();
        try
        {
            (this->*(movementPacketHandlers[moveCode].function))(true, *buffer);
            res = UpdatePostRead(opcode, moveCode, buffer);
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
        UpdatePreWrite(opcode, moveCode);
        (this->*(movementPacketHandlers[moveCode].function))(false, *buffer);
        if(m_incrementMoveCounter)
        {
            m_incrementMoveCounter = false;
            m_serverCounter++;
        }
    } else sLog.outError("");
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

#define DO_BIT(buff, val) buff->WriteBit(val ? 1 : 0);
#define DO_COND_BIT(buff, cond, val) if(cond) buff->WriteBit(val ? 1 : 0);
#define DO_BYTES(buff, type, val) buff->append<type>(val);
#define DO_COND_BYTES(buff, cond, type, val) if(cond) buff->append<type>(val);
#define DO_SEQ_BYTE(buff, val) buff->WriteByteSeq(val);

void MovementInterface::AppendSplineData(bool bits, ByteBuffer *buffer, uint32 msTime, std::vector<MovementPoint*> *pointStorage)
{
    if(bits) m_path.AppendMoveBits(buffer, msTime, pointStorage);
    else m_path.AppendMoveBytes(buffer, msTime, pointStorage);
}

void MovementInterface::WriteObjectUpdate(ByteBuffer *bits, ByteBuffer *bytes)
{
    uint32 msTime = getMSTime();
    std::vector<MovementPoint*> m_splineMove;
    bool hasMovementFlags = m_movementFlagMask & 0x0F, hasMovementFlags2 = m_movementFlagMask & 0xF0,
    hasOrientation = !RONIN_UTIL::fuzzyEq(0.f, m_serverLocation->o), hasTransportData = !m_transportGuid.empty(),
    hasSpline = isSplineMovingActive(), hasTransportTime2 = (hasTransportData && m_transportTime2 != 0), hasTransportVehicleId = (hasTransportData && m_vehicleId != 0),
    hasPitch = (hasFlag(MOVEMENTFLAG_SWIMMING) || hasFlag(MOVEMENTFLAG_FLYING) || hasFlag(MOVEMENTFLAG_ALWAYS_ALLOW_PITCHING)),
    hasFallDirection = hasFlag(MOVEMENTFLAG_TOGGLE_FALLING), hasFallData = (hasFallDirection || m_jumpTime != 0), hasSplineElevation = hasFlag(MOVEMENTFLAG_SPLINE_ELEVATION);

    // Append our bits
    DO_BIT(bits, !hasMovementFlags);
    DO_BIT(bits, !hasOrientation);
    DO_BIT(bits, m_moverGuid[7]);
    DO_BIT(bits, m_moverGuid[3]);
    DO_BIT(bits, m_moverGuid[2]);
    if (hasMovementFlags) HandleMovementFlags(false, bits);
    DO_BIT(bits, false);
    DO_BIT(bits, !hasPitch);
    DO_BIT(bits, hasSpline);
    DO_BIT(bits, hasFallData);
    DO_BIT(bits, !hasSplineElevation);
    DO_BIT(bits, m_moverGuid[5]);
    DO_BIT(bits, hasTransportData);
    DO_BIT(bits, false); // We have a timestamp, but the question is if we have no timestamp
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[1]);
    DO_COND_BIT(bits, hasTransportData, hasTransportTime2);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[4]);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[0]);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[6]);
    DO_COND_BIT(bits, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[7]);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[5]);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[3]);
    DO_COND_BIT(bits, hasTransportData, m_transportGuid[2]);
    DO_BIT(bits, m_moverGuid[4]);
    if(hasSpline) AppendSplineData(true, bits, msTime, &m_splineMove);
    DO_BIT(bits, m_moverGuid[6]);
    DO_COND_BIT(bits, hasFallData, hasFallDirection);
    DO_BIT(bits, m_moverGuid[0]);
    DO_BIT(bits, m_moverGuid[1]);
    DO_BIT(bits, false);
    DO_BIT(bits, !hasMovementFlags2);
    if (hasMovementFlags2) HandleMovementFlags2(false, bits);

    // Append our bytes
    DO_SEQ_BYTE(bytes, m_moverGuid[4]);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_RUN_BACK));
    DO_COND_BYTES(bytes, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(bytes, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(bytes, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(bytes, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(bytes, hasFallData, float, m_jumpZSpeed);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_SWIM_BACK));
    DO_COND_BYTES(bytes, hasSplineElevation, float, splineElevation);
    if(hasSpline) AppendSplineData(false, bytes, msTime, &m_splineMove);// Write spline data
    DO_BYTES(bytes, float, m_serverLocation->z);
    DO_SEQ_BYTE(bytes, m_moverGuid[5]);
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
    DO_COND_BYTES(bytes, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[1], uint8, m_transportGuid[1]);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[6], uint8, m_transportGuid[6]);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[2], uint8, m_transportGuid[2]);
    DO_COND_BYTES(bytes, hasTransportData && m_transportGuid[4], uint8, m_transportGuid[4]);
    DO_BYTES(bytes, float, m_serverLocation->x);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_PITCHRATE));
    DO_SEQ_BYTE(bytes, m_moverGuid[3]);
    DO_SEQ_BYTE(bytes, m_moverGuid[0]);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_SWIM));
    DO_BYTES(bytes, float, m_serverLocation->y);
    DO_SEQ_BYTE(bytes, m_moverGuid[7]);
    DO_SEQ_BYTE(bytes, m_moverGuid[1]);
    DO_SEQ_BYTE(bytes, m_moverGuid[2]);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_WALK));
    DO_BYTES(bytes, uint32, msTime);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_TURNRATE));
    DO_SEQ_BYTE(bytes, m_moverGuid[6]);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_FLIGHT));
    DO_COND_BYTES(bytes, hasOrientation, float, m_serverLocation->o);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_RUN));
    DO_COND_BYTES(bytes, hasPitch, float, pitching);
    DO_BYTES(bytes, float, GetMoveSpeed(MOVE_SPEED_FLIGHT_BACK));
}

#undef DO_BIT
#undef DO_COND_BIT
#undef DO_BYTES
#undef DO_COND_BYTES
#undef DO_SEQ_BYTE

void MovementInterface::ClearOptionalMovementData()
{
    // Reset client position to server location
    m_clientLocation = *m_serverLocation;

    memset(m_movementFlags, 0, 6);
    m_jumpTime = m_vehicleId = 0;
    pitching = splineElevation = 0.f;
    m_jumpZSpeed = m_jump_XYSpeed = m_jump_sin = m_jump_cos = 0.f;
    m_extra.clear();

    if(!m_isTransportLocked)
    {
        m_clientTransGuid.Clean();
        m_clientTransLocation.ChangeCoords(0.f, 0.f, 0.f, 0.f);
        m_transportTime = m_transportTime2 = 0;
        m_transportSeatId = 0;
    }
}
