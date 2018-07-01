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

#pragma once

enum MovementFlagsA : uint8 // First bytes of movement are client set | Actual movement identifiers
{
    MOVEMENTFLAG_MOVE_FORWARD           = 0x01,
    MOVEMENTFLAG_MOVE_BACKWARD          = 0x02,
    MOVEMENTFLAG_MOVE_STRAFE_LEFT       = 0x04,
    MOVEMENTFLAG_MOVE_STRAFE_RIGHT      = 0x08,
    MOVEMENTFLAG_MOVE_TURN_LEFT         = 0x10,
    MOVEMENTFLAG_MOVE_TURN_RIGHT        = 0x20,
    MOVEMENTFLAG_MOVE_PITCH_UP          = 0x40,
    MOVEMENTFLAG_MOVE_PITCH_DOWN        = 0x80,

    MOVEMENTFLAG_MASK_MOVING            = 0x0F,
    MOVEMENTFLAG_MASK_TURNING           = 0x30,
    MOVEMENTFLAG_MASK_PITCHING          = 0xC0,

    // On remove from world, remove all moving data
    MOVEMENTFLAG_MASK_A_ON_RFW          = 0xFF,
    MOVEMENTFLAG_MASK_A_ALL = 0xFF
};

enum MovementFlagsB : uint8
{
    MOVEMENTFLAG_TOGGLE_WALK            = 0x01,
    MOVEMENTFLAG_TOGGLE_NO_GRAVITY      = 0x02,
    MOVEMENTFLAG_TOGGLE_ROOT            = 0x04,
    MOVEMENTFLAG_TOGGLE_FALLING         = 0x08,
    MOVEMENTFLAG_TOGGLE_FALLING_FAR     = 0x10,
    MOVEMENTFLAG_PENDING_STOP           = 0x20,
    MOVEMENTFLAG_PENDING_STRAFE_STOP    = 0x40,
    MOVEMENTFLAG_PENDING_FORWARD        = 0x80,

    // On remove from world, remove falling, and pending data
    MOVEMENTFLAG_MASK_B_ON_RFW          = MOVEMENTFLAG_TOGGLE_FALLING | MOVEMENTFLAG_TOGGLE_FALLING_FAR | MOVEMENTFLAG_PENDING_STOP | MOVEMENTFLAG_PENDING_STRAFE_STOP | MOVEMENTFLAG_PENDING_FORWARD,
    MOVEMENTFLAG_MASK_B_ALL = 0xFF
};

enum MovementFlagsC : uint8
{
    MOVEMENTFLAG_PENDING_BACKWARD       = 0x01,
    MOVEMENTFLAG_PENDING_STRAFE_LEFT    = 0x02,
    MOVEMENTFLAG_PENDING_STRAFE_RIGHT   = 0x04,
    MOVEMENTFLAG_PENDING_ROOT           = 0x08,
    MOVEMENTFLAG_SWIMMING               = 0x10,
    MOVEMENTFLAG_ASCENDING              = 0x20,
    MOVEMENTFLAG_DESCENDING             = 0x40,
    MOVEMENTFLAG_CAN_FLY                = 0x80,

    // On remove from world, remove pending and swimming based flags
    MOVEMENTFLAG_MASK_C_ON_RFW          = MOVEMENTFLAG_PENDING_BACKWARD | MOVEMENTFLAG_PENDING_STRAFE_LEFT | MOVEMENTFLAG_PENDING_STRAFE_RIGHT | MOVEMENTFLAG_PENDING_ROOT | MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_ASCENDING | MOVEMENTFLAG_DESCENDING,
    MOVEMENTFLAG_MASK_C_ALL = 0xFF
};

enum MovementFlagsD : uint8
{
    MOVEMENTFLAG_FLYING                 = 0x01,
    MOVEMENTFLAG_SPLINE_ELEVATION       = 0x02,
    MOVEMENTFLAG_WATERWALKING           = 0x04,
    MOVEMENTFLAG_FEATHERFALLING         = 0x08,
    MOVEMENTFLAG_HOVER                  = 0x10,
    MOVEMENTFLAG_DISABLE_COLLISION      = 0x20,

    // On remove from world, flying and spline flags
    MOVEMENTFLAG_MASK_D_ON_RFW          = MOVEMENTFLAG_FLYING | MOVEMENTFLAG_SPLINE_ELEVATION,
    MOVEMENTFLAG_MASK_D_ALL = 0xFF
};

enum MovementFlagsE : uint8
{
    MOVEMENTFLAG_NO_STRAFE              = 0x01,
    MOVEMENTFLAG_NO_JUMPING             = 0x02,
    MOVEMENTFLAG_FULL_SPEED_TURNING     = 0x04,
    MOVEMENTFLAG_FULL_SPEED_PITCHING    = 0x08,
    MOVEMENTFLAG_ALWAYS_ALLOW_PITCHING  = 0x10,

    // On remove from world, remove movement lock data
    MOVEMENTFLAG_MASK_E_ON_RFW          = MOVEMENTFLAG_NO_STRAFE | MOVEMENTFLAG_NO_JUMPING,
    MOVEMENTFLAG_MASK_E_ALL = 0xFF
};

enum MovementFlagsF : uint8
{
    MOVEMENTFLAG_CAN_SWIM_TO_FLY_TRANSITION = 0x04,
    MOVEMENTFLAG_INTERPOLATED_MOVEMENT      = 0x40,
    MOVEMENTFLAG_INTERPOLATED_TURNING       = 0x80,

    // On remove from world, remove no data
    MOVEMENTFLAG_MASK_F_ON_RFW          = 0x00,
    MOVEMENTFLAG_MASK_F_ALL = 0xFF
};

enum MovementCodes : uint16
{
    // Movement codes
    MOVEMENT_CODE_PLAYER_MOVE = 0,
    MOVEMENT_CODE_HEARTBEAT,
    MOVEMENT_CODE_JUMP,
    MOVEMENT_CODE_FALL_LAND,
    MOVEMENT_CODE_START_FORWARD,
    MOVEMENT_CODE_START_BACKWARD,
    MOVEMENT_CODE_START_STRAFE_LEFT,
    MOVEMENT_CODE_START_STRAFE_RIGHT,
    MOVEMENT_CODE_START_TURN_LEFT,
    MOVEMENT_CODE_START_TURN_RIGHT,
    MOVEMENT_CODE_START_PITCH_DOWN,
    MOVEMENT_CODE_START_PITCH_UP,
    MOVEMENT_CODE_START_ASCEND,
    MOVEMENT_CODE_START_DESCEND,
    MOVEMENT_CODE_START_SWIM,
    MOVEMENT_CODE_STOP,
    MOVEMENT_CODE_STOP_STRAFE,
    MOVEMENT_CODE_STOP_TURN,
    MOVEMENT_CODE_STOP_PITCH,
    MOVEMENT_CODE_STOP_ASCEND,
    MOVEMENT_CODE_STOP_SWIM,
    MOVEMENT_CODE_SET_FACING,
    MOVEMENT_CODE_SET_PITCH,
    MOVEMENT_CODE_FALL_RESET,
    MOVEMENT_CODE_SET_RUN_MODE,
    MOVEMENT_CODE_SET_WALK_MODE,
    MOVEMENT_CODE_SET_CAN_FLY,
    MOVEMENT_CODE_UNSET_CAN_FLY,
    MOVEMENT_CODE_SET_HOVER,
    MOVEMENT_CODE_UNSET_HOVER,
    MOVEMENT_CODE_WATER_WALK,
    MOVEMENT_CODE_LAND_WALK,
    MOVEMENT_CODE_FEATHER_FALL,
    MOVEMENT_CODE_NORMAL_FALL,
    MOVEMENT_CODE_ROOT,
    MOVEMENT_CODE_UNROOT,
    MOVEMENT_CODE_GRAVITY_DISABLE,
    MOVEMENT_CODE_GRAVITY_ENABLE,
    MOVEMENT_CODE_UPDATE_KNOCK_BACK,
    MOVEMENT_CODE_UPDATE_TELEPORT,
    MOVEMENT_CODE_CHANGE_TRANSPORT,
    MOVEMENT_CODE_NOT_ACTIVE_MOVER,
    MOVEMENT_CODE_SET_COLLISION_HEIGHT,
    MOVEMENT_CODE_UPDATE_COLLISION_HEIGHT,

    // Speed codes
    MOVEMENT_CODE_UPDATE_WALK_SPEED,
    MOVEMENT_CODE_UPDATE_RUN_SPEED,
    MOVEMENT_CODE_UPDATE_RUN_BACK_SPEED,
    MOVEMENT_CODE_UPDATE_SWIM_SPEED,
    MOVEMENT_CODE_UPDATE_SWIM_BACK_SPEED,
    MOVEMENT_CODE_UPDATE_FLIGHT_SPEED,
    MOVEMENT_CODE_UPDATE_FLIGHT_BACK_SPEED,
    MOVEMENT_CODE_UPDATE_TURN_RATE,
    MOVEMENT_CODE_UPDATE_PITCH_RATE,
    MOVEMENT_CODE_SET_WALK_SPEED,
    MOVEMENT_CODE_SET_RUN_SPEED,
    MOVEMENT_CODE_SET_RUN_BACK_SPEED,
    MOVEMENT_CODE_SET_SWIM_SPEED,
    MOVEMENT_CODE_SET_SWIM_BACK_SPEED,
    MOVEMENT_CODE_SET_FLIGHT_SPEED,
    MOVEMENT_CODE_SET_FLIGHT_BACK_SPEED,
    MOVEMENT_CODE_SET_TURN_RATE,
    MOVEMENT_CODE_SET_PITCH_RATE,

    // Acknowledgement codes
    MOVEMENT_CODE_ACK_TELEPORT,
    MOVEMENT_CODE_ACK_ROOT,
    MOVEMENT_CODE_ACK_UNROOT,
    MOVEMENT_CODE_ACK_FEATHER_FALL,
    MOVEMENT_CODE_ACK_GRAVITY_ENABLE,
    MOVEMENT_CODE_ACK_GRAVITY_DISABLE,
    MOVEMENT_CODE_ACK_HOVER,
    MOVEMENT_CODE_ACK_WATER_WALK,
    MOVEMENT_CODE_ACK_KNOCK_BACK,
    MOVEMENT_CODE_ACK_SET_CAN_FLY,
    MOVEMENT_CODE_ACK_SET_COLLISION_HEIGHT,
    MOVEMENT_CODE_ACK_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY,

    MOVEMENT_CODE_ACK_FORCE_WALK_SPEED_CHANGE,
    MOVEMENT_CODE_ACK_FORCE_RUN_SPEED_CHANGE,
    MOVEMENT_CODE_ACK_FORCE_RUN_BACK_SPEED_CHANGE,
    MOVEMENT_CODE_ACK_FORCE_SWIM_SPEED_CHANGE,
    MOVEMENT_CODE_ACK_FORCE_SWIM_BACK_SPEED_CHANGE,
    MOVEMENT_CODE_ACK_FORCE_FLIGHT_SPEED_CHANGE,
    MOVEMENT_CODE_ACK_FORCE_FLIGHT_BACK_SPEED_CHANGE,
    MOVEMENT_CODE_ACK_FORCE_TURN_RATE_CHANGE,
    MOVEMENT_CODE_ACK_FORCE_PITCH_RATE_CHANGE,

    // Spline codes
    MOVEMENT_CODE_SPLINE_DONE,
    MOVEMENT_CODE_SPLINE_SET_WALK_SPEED,
    MOVEMENT_CODE_SPLINE_SET_RUN_SPEED,
    MOVEMENT_CODE_SPLINE_SET_RUN_BACK_SPEED,
    MOVEMENT_CODE_SPLINE_SET_SWIM_SPEED,
    MOVEMENT_CODE_SPLINE_SET_SWIM_BACK_SPEED,
    MOVEMENT_CODE_SPLINE_SET_FLIGHT_SPEED,
    MOVEMENT_CODE_SPLINE_SET_FLIGHT_BACK_SPEED,
    MOVEMENT_CODE_SPLINE_SET_TURN_RATE,
    MOVEMENT_CODE_SPLINE_SET_PITCH_RATE,
    MOVEMENT_CODE_SPLINE_SET_WALK_MODE,
    MOVEMENT_CODE_SPLINE_SET_RUN_MODE,
    MOVEMENT_CODE_SPLINE_GRAVITY_ENABLE,
    MOVEMENT_CODE_SPLINE_GRAVITY_DISABLE,
    MOVEMENT_CODE_SPLINE_SET_HOVER,
    MOVEMENT_CODE_SPLINE_UNSET_HOVER,
    MOVEMENT_CODE_SPLINE_START_SWIM,
    MOVEMENT_CODE_SPLINE_STOP_SWIM,
    MOVEMENT_CODE_SPLINE_SET_FLYING,
    MOVEMENT_CODE_SPLINE_UNSET_FLYING,
    MOVEMENT_CODE_SPLINE_SET_WATER_WALK,
    MOVEMENT_CODE_SPLINE_SET_LAND_WALK,
    MOVEMENT_CODE_SPLINE_SET_FEATHER_FALL,
    MOVEMENT_CODE_SPLINE_SET_NORMAL_FALL,
    MOVEMENT_CODE_SPLINE_ROOT,
    MOVEMENT_CODE_SPLINE_UNROOT,

    // Vehicle codes
    MOVEMENT_CODE_DISMISS_CONTROLLED_VEHICLE,
    MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE,

    // Embedded codes
    MOVEMENT_EMBEDDED_CAST_SPELL,
    MOVEMENT_EMBEDDED_PET_CAST_SPELL,
    MOVEMENT_EMBEDDED_USE_ITEM,

    MAX_MOVEMENT_CODE
};

enum MovementSpeedTypes : uint8
{
    MOVE_SPEED_WALK,
    MOVE_SPEED_RUN,
    MOVE_SPEED_RUN_BACK,
    MOVE_SPEED_SWIM,
    MOVE_SPEED_SWIM_BACK,
    MOVE_SPEED_TURNRATE,
    MOVE_SPEED_FLIGHT,
    MOVE_SPEED_FLIGHT_BACK,
    MOVE_SPEED_PITCHRATE,
    MOVE_SPEED_MAX
};

enum MovementStatusTypes : uint8
{
    MOVEMENT_STATUS_ROOT            = 0,
    MOVEMENT_STATUS_UNROOT          = 1,
    MOVEMENT_STATUS_FEATHERFALLING  = 2,
    MOVEMENT_STATUS_ENABLEGRAVITY   = 3,
    MOVEMENT_STATUS_DISABLEGRAVITY  = 4,
    MOVEMENT_STATUS_HOVERING        = 5,
    MOVEMENT_STATUS_WATERWALKING    = 6,
    MOVEMENT_STATUS_KNOCKBACKED     = 7,
    MOVEMENT_STATUS_CANFLY          = 8,
    MOVEMENT_STATUS_COLLISION_HEIGHT= 9,
    MOVEMENT_STATUS_CANSFTRANSITION = 10,
    MOVEMENT_STATUS_MAX
};

enum UnderwaterStates : uint8
{
    UNDERWATERSTATE_NONE            = 0x00,
    UNDERWATERSTATE_UNDERWATER      = 0x02,
    UNDERWATERSTATE_FATIGUE         = 0x04,
    UNDERWATERSTATE_LAVA            = 0x08,
    UNDERWATERSTATE_SLIME           = 0x10,
    UNDERWATERSTATE_TIMERS_PRESENT  = 0x20
};

class MovementInterface;
struct PacketHandler { void (MovementInterface::*function)(bool read, ByteBuffer &buffer); };

class MovementInterface
{
public:
    MovementInterface(Unit *_unit);
    ~MovementInterface();

    // Attaches to owner guids
    void AttachToOwner();

    // Grabs path pointer from underlying movement update system
    UnitPathSystem *GetPath() { return &m_path; }

    static uint16 GetInternalMovementCode(uint16 opcode);
    static uint16 GetSpeedTypeForMoveCode(uint16 moveCode);

    void Update(uint32 msTime, uint32 diff);
    void UpdatePreWrite(uint16 opcode, uint16 moveCode);
    bool UpdatePostRead(uint16 opcode, uint16 moveCode, ByteBuffer *source);

    void HandleBreathing(uint32 diff);
    void HandleMovementFlags(bool read, ByteBuffer *buffer);
    void HandleMovementFlags2(bool read, ByteBuffer *buffer);

    void TeleportToPosition(LocationVector destination);
    void TeleportToPosition(uint32 mapId, uint32 instanceId, LocationVector destination);

    bool CanProcessTimeSyncCounter(uint32 counter);

    void ProcessModUpdate(uint8 modUpdateType, std::vector<uint32> modMap);
    void HandlePendingMoveData(bool fromLanding);

    void SetFacing(float orientation);
    void MoveTo(float x, float y, float z, float o) { m_path.MoveToPoint(x, y, z, o); };
    void MoveClientPosition(float x, float y, float z, float o);
    void SetSelfTempData(bool enable);

    void SendTimeSyncReq();

    void OnPreSetInWorld();
    void OnPrePushToWorld();
    void OnPushToWorld(uint32 msTime);
    void OnRemoveFromWorld();
    void OnFirstTimeSync();

    void OnDeath();
    void OnRespawn();
    void OnRepop();
    void OnResurrect();
    void OnRevive();
    void OnDismount();
    void OnTaxiEnd();
    void OnRelocate(LocationVector destination);

    bool isInAir();

    // Aura related functions
    bool isRooted() { return hasFlag(MOVEMENTFLAG_TOGGLE_ROOT); }
    bool isAsleep() { return false; }
    bool isStunned() { return false; }
    bool canFly() { return hasFlag(MOVEMENTFLAG_CAN_FLY); }

    void setRooted(bool root);
    void setAsleep(bool sleep) {}
    void setStunned(bool stunned);

    void setFeatherFall(bool canFeatherFall);
    void setWaterWalk(bool canWaterWalk);
    void setHover(bool canHover);
    void setCanFly(bool canfly);

    // Movement flags
    uint8 GetMovementMask() { return m_movementFlagMask; }
    uint8 GetFlags(uint8 index) { return m_movementFlags[index]|m_serverFlags[index]; }
    bool hasFlag(MovementFlagsA flag) { return GetFlags(0)&flag; };
    bool hasFlag(MovementFlagsB flag) { return GetFlags(1)&flag; };
    bool hasFlag(MovementFlagsC flag) { return GetFlags(2)&flag; };
    bool hasFlag(MovementFlagsD flag) { return GetFlags(3)&flag; };
    bool hasFlag(MovementFlagsE flag) { return GetFlags(4)&flag; };
    bool hasFlag(MovementFlagsF flag) { return GetFlags(5)&flag; };

    void setServerFlag(MovementFlagsA flag) { m_serverFlags[0] |= flag; UpdateMovementFlagMask(0); }
    void setServerFlag(MovementFlagsB flag) { m_serverFlags[1] |= flag; UpdateMovementFlagMask(1); }
    void setServerFlag(MovementFlagsC flag) { m_serverFlags[2] |= flag; UpdateMovementFlagMask(2); }
    void setServerFlag(MovementFlagsD flag) { m_serverFlags[3] |= flag; UpdateMovementFlagMask(3); }
    void setServerFlag(MovementFlagsE flag) { m_serverFlags[4] |= flag; UpdateMovementFlagMask(4); }
    void setServerFlag(MovementFlagsF flag) { m_serverFlags[5] |= flag; UpdateMovementFlagMask(5); }

    void removeServerFlag(MovementFlagsA flag) { m_serverFlags[0] &= ~flag; UpdateMovementFlagMask(0); }
    void removeServerFlag(MovementFlagsB flag) { m_serverFlags[1] &= ~flag; UpdateMovementFlagMask(1); }
    void removeServerFlag(MovementFlagsC flag) { m_serverFlags[2] &= ~flag; UpdateMovementFlagMask(2); }
    void removeServerFlag(MovementFlagsD flag) { m_serverFlags[3] &= ~flag; UpdateMovementFlagMask(3); }
    void removeServerFlag(MovementFlagsE flag) { m_serverFlags[4] &= ~flag; UpdateMovementFlagMask(4); }
    void removeServerFlag(MovementFlagsF flag) { m_serverFlags[5] &= ~flag; UpdateMovementFlagMask(5); }

    bool hasPendingMoveStatus(uint8 type) { return m_pendingEnable[type]; }

    bool isMoving() { return hasFlag(MOVEMENTFLAG_MASK_MOVING); }
    bool isTurning() { return hasFlag(MOVEMENTFLAG_MASK_TURNING); }
    bool isPitching() { return hasFlag(MOVEMENTFLAG_MASK_PITCHING); }

    // Spline moving
    bool isSplineMovingActive();

    // Underwater state
    uint8 GetUnderwaterState() { return m_underwaterState; }
    void AddUnderwaterStateTimerPresent() { m_underwaterState |= UNDERWATERSTATE_TIMERS_PRESENT; }

    // Packet handlers
    bool ReadFromClient(uint16 opcode, ByteBuffer *buffer);
    void WriteFromServer(uint16 opcode, ByteBuffer *buffer, WoWGuid extra_guid = 0, float extra_float = 0.f, uint8 extra_byte = 0);

    void DoExtraData(uint16 moveCode, bool read, ByteBuffer *buffer, uint8 sequence = 0);

    // Alternate packet handlers
    void SetActiveMover(WorldPacket *packet);
    void MoveTimeSkipped(WorldPacket *packet);
    void MoveSplineComplete(WorldPacket *packet);
    void MoveFallReset(WorldPacket *packet);

    // Transport data
    bool OnTransport() { return !m_transportGuid.empty(); }
    bool isTransportLocked() { return m_isTransportLocked; }
    void LockTransportData() { m_isTransportLocked = true; }
    void UnlockTransportData() { m_isTransportLocked = false; }

    WoWGuid GetTransportGuid() { return m_transportGuid; }
    uint32 GetTransportTime() { return m_transportTime; }
    uint32 GetTransportTime2() { return m_transportTime2; }
    uint32 GetTransportVehicleId() { return m_vehicleId; }
    int8 GetTransportSeat() { return m_transportSeatId; }
    void GetTransportPosition(float &x, float &y, float &z, float &o)
    {
        x = m_transportLocation.x;
        y = m_transportLocation.y;
        z = m_transportLocation.z;
        o = m_transportLocation.o;
    }

    void GetTransportPosition(LocationVector &vec)
    {
        vec.x = m_transportLocation.x;
        vec.y = m_transportLocation.y;
        vec.z = m_transportLocation.z;
        vec.o = m_transportLocation.o;
    }

    void SetTransportData(WoWGuid transGuid, uint32 vehicleId, float xOff, float yOff, float zOff, float oOff, int8 seatId, uint32 time1 = 0, uint32 time2 = 0)
    {
        m_vehicleId = vehicleId;
        m_transportGuid = transGuid;
        m_transportSeatId = seatId;
        m_transportLocation.ChangeCoords(xOff, yOff, zOff, oOff);
        m_transportTime = time1;
        m_transportTime2 = time2;
    }

    void ClearTransportData()
    {
        m_vehicleId = 0;
        m_transportGuid = 0;
        m_transportSeatId = 0xFF;
        m_transportTime = m_transportTime2 = 0;
        m_transportLocation.ChangeCoords(0.f, 0.f, 0.f, 0.f);
    }

    // Speed
    float GetMoveSpeed(MovementSpeedTypes speedType) { return m_currSpeeds[speedType]; }

    void GetSpeedPointers(float *&walk, float *&run, float *&fly)
    {
        walk = &m_currSpeeds[MOVE_SPEED_WALK];
        run = &m_currSpeeds[MOVE_SPEED_RUN];
        fly = &m_currSpeeds[MOVE_SPEED_FLIGHT];
    }

    void RecalculateMoveSpeed(MovementSpeedTypes speedType);
    void SetMoveSpeed(MovementSpeedTypes speedType, float speed);

    void ClearMovementFlags(uint8 clearFlag, bool serverFlags)
    {
        m_movementFlagMask &= ~clearFlag;
        for(uint8 i = 0; i < 6; i++)
        {
            if((clearFlag&1<<i) == 0)
                continue;
            if(serverFlags) m_serverFlags[i] = 0;
            m_movementFlags[i] = 0;
        }
    }

    void StopMirrorTimer(uint8 type) { m_MirrorTimer[type] = -1; }

private:
    bool UpdateAcknowledgementData(uint16 moveCode);
    bool UpdateMovementData(uint16 moveCode, bool distribute);
    float _CalculateSpeed(MovementSpeedTypes speedType);

public:
    void AppendSplineData(bool bits, ByteBuffer *buffer, uint32 msTime, std::vector<MovementPoint*> *pointStorage);
    void WriteObjectUpdate(ByteBuffer *bits, ByteBuffer *bytes);

    // Movement codes
    void HandlePlayerMove(bool read, ByteBuffer &buffer);
    void HandleHeartbeat(bool read, ByteBuffer &buffer);
    void HandleJump(bool read, ByteBuffer &buffer);
    void HandleFallLand(bool read, ByteBuffer &buffer);
    void HandleStartForward(bool read, ByteBuffer &buffer);
    void HandleStartBackward(bool read, ByteBuffer &buffer);
    void HandleStartStrafeLeft(bool read, ByteBuffer &buffer);
    void HandleStartStrafeRight(bool read, ByteBuffer &buffer);
    void HandleStartTurnLeft(bool read, ByteBuffer &buffer);
    void HandleStartTurnRight(bool read, ByteBuffer &buffer);
    void HandleStartPitchDown(bool read, ByteBuffer &buffer);
    void HandleStartPitchUp(bool read, ByteBuffer &buffer);
    void HandleStartAscend(bool read, ByteBuffer &buffer);
    void HandleStartDescend(bool read, ByteBuffer &buffer);
    void HandleStartSwim(bool read, ByteBuffer &buffer);
    void HandleStop(bool read, ByteBuffer &buffer);
    void HandleStopStrafe(bool read, ByteBuffer &buffer);
    void HandleStopTurn(bool read, ByteBuffer &buffer);
    void HandleStopPitch(bool read, ByteBuffer &buffer);
    void HandleStopAscend(bool read, ByteBuffer &buffer);
    void HandleStopSwim(bool read, ByteBuffer &buffer);
    void HandleSetFacing(bool read, ByteBuffer &buffer);
    void HandleSetPitch(bool read, ByteBuffer &buffer);
    void HandleFallReset(bool read, ByteBuffer &buffer);
    void HandleSetRunMode(bool read, ByteBuffer &buffer);
    void HandleSetWalkMode(bool read, ByteBuffer &buffer);
    void HandleSetCanFly(bool read, ByteBuffer &buffer);
    void HandleUnsetCanFly(bool read, ByteBuffer &buffer);
    void HandleSetHover(bool read, ByteBuffer &buffer);
    void HandleUnsetHover(bool read, ByteBuffer &buffer);
    void HandleWaterWalk(bool read, ByteBuffer &buffer);
    void HandleLandWalk(bool read, ByteBuffer &buffer);
    void HandleFeatherFall(bool read, ByteBuffer &buffer);
    void HandleNormalFall(bool read, ByteBuffer &buffer);
    void HandleRoot(bool read, ByteBuffer &buffer);
    void HandleUnroot(bool read, ByteBuffer &buffer);
    void HandleGravityDisable(bool read, ByteBuffer &buffer);
    void HandleGravityEnable(bool read, ByteBuffer &buffer);
    void HandleUpdateKnockBack(bool read, ByteBuffer &buffer);
    void HandleUpdateTeleport(bool read, ByteBuffer &buffer);
    void HandleChangeTransport(bool read, ByteBuffer &buffer);
    void HandleNotActiveMover(bool read, ByteBuffer &buffer);
    void HandleSetCollisionHeight(bool read, ByteBuffer &buffer);
    void HandleUpdateCollisionHeight(bool read, ByteBuffer &buffer);
    // Speed codes
    void HandleUpdateWalkSpeed(bool read, ByteBuffer &buffer);
    void HandleUpdateRunSpeed(bool read, ByteBuffer &buffer);
    void HandleUpdateRunBackSpeed(bool read, ByteBuffer &buffer);
    void HandleUpdateSwimSpeed(bool read, ByteBuffer &buffer);
    void HandleUpdateSwimBackSpeed(bool read, ByteBuffer &buffer);
    void HandleUpdateFlightSpeed(bool read, ByteBuffer &buffer);
    void HandleUpdateFlightBackSpeed(bool read, ByteBuffer &buffer);
    void HandleUpdateTurnRate(bool read, ByteBuffer &buffer);
    void HandleUpdatePitchRate(bool read, ByteBuffer &buffer);
    void HandleSetWalkSpeed(bool read, ByteBuffer &buffer);
    void HandleSetRunSpeed(bool read, ByteBuffer &buffer);
    void HandleSetRunBackSpeed(bool read, ByteBuffer &buffer);
    void HandleSetSwimSpeed(bool read, ByteBuffer &buffer);
    void HandleSetSwimBackSpeed(bool read, ByteBuffer &buffer);
    void HandleSetFlightSpeed(bool read, ByteBuffer &buffer);
    void HandleSetFlightBackSpeed(bool read, ByteBuffer &buffer);
    void HandleSetTurnRate(bool read, ByteBuffer &buffer);
    void HandleSetPitchRate(bool read, ByteBuffer &buffer);
    // Acknowledgement codes
    void HandleAckTeleport(bool read, ByteBuffer &buffer);
    void HandleAckRoot(bool read, ByteBuffer &buffer);
    void HandleAckUnroot(bool read, ByteBuffer &buffer);
    void HandleAckFeatherFall(bool read, ByteBuffer &buffer);
    void HandleAckGravityEnable(bool read, ByteBuffer &buffer);
    void HandleAckGravityDisable(bool read, ByteBuffer &buffer);
    void HandleAckHover(bool read, ByteBuffer &buffer);
    void HandleAckWaterWalk(bool read, ByteBuffer &buffer);
    void HandleAckKnockBack(bool read, ByteBuffer &buffer);
    void HandleAckSetCanFly(bool read, ByteBuffer &buffer);
    void HandleAckSetCollisionHeight(bool read, ByteBuffer &buffer);
    void HandleAckSetCanTransitionBetweenSwimAndFly(bool read, ByteBuffer &buffer);
    void HandleAckForceWalkSpeedChange(bool read, ByteBuffer &buffer);
    void HandleAckForceRunSpeedChange(bool read, ByteBuffer &buffer);
    void HandleAckForceRunBackSpeedChange(bool read, ByteBuffer &buffer);
    void HandleAckForceSwimSpeedChange(bool read, ByteBuffer &buffer);
    void HandleAckForceSwimBackSpeedChange(bool read, ByteBuffer &buffer);
    void HandleAckForceFlightSpeedChange(bool read, ByteBuffer &buffer);
    void HandleAckForceFlightBackSpeedChange(bool read, ByteBuffer &buffer);
    void HandleAckForceTurnRateChange(bool read, ByteBuffer &buffer);
    void HandleAckForcePitchRateChange(bool read, ByteBuffer &buffer);
    // Spline codes
    void HandleSplineDone(bool read, ByteBuffer &buffer);
    void HandleSplineSetWalkSpeed(bool read, ByteBuffer &buffer);
    void HandleSplineSetRunSpeed(bool read, ByteBuffer &buffer);
    void HandleSplineSetRunBackSpeed(bool read, ByteBuffer &buffer);
    void HandleSplineSetSwimSpeed(bool read, ByteBuffer &buffer);
    void HandleSplineSetSwimBackSpeed(bool read, ByteBuffer &buffer);
    void HandleSplineSetFlightSpeed(bool read, ByteBuffer &buffer);
    void HandleSplineSetFlightBackSpeed(bool read, ByteBuffer &buffer);
    void HandleSplineSetPitchRate(bool read, ByteBuffer &buffer);
    void HandleSplineSetTurnRate(bool read, ByteBuffer &buffer);
    void HandleSplineSetWalkMode(bool read, ByteBuffer &buffer);
    void HandleSplineSetRunMode(bool read, ByteBuffer &buffer);
    void HandleSplineGravityEnable(bool read, ByteBuffer &buffer);
    void HandleSplineGravityDisable(bool read, ByteBuffer &buffer);
    void HandleSplineSetHover(bool read, ByteBuffer &buffer);
    void HandleSplineSetUnhover(bool read, ByteBuffer &buffer);
    void HandleSplineStartSwim(bool read, ByteBuffer &buffer);
    void HandleSplineStopSwim(bool read, ByteBuffer &buffer);
    void HandleSplineSetFlying(bool read, ByteBuffer &buffer);
    void HandleSplineUnsetFlying(bool read, ByteBuffer &buffer);
    void HandleSplineSetWaterWalk(bool read, ByteBuffer &buffer);
    void HandleSplineSetLandWalk(bool read, ByteBuffer &buffer);
    void HandleSplineSetFeatherFall(bool read, ByteBuffer &buffer);
    void HandleSplineSetNormalFall(bool read, ByteBuffer &buffer);
    void HandleSplineRoot(bool read, ByteBuffer &buffer);
    void HandleSplineUnroot(bool read, ByteBuffer &buffer);
    // Vehicle codes
    void HandleDismissControlledVehicle(bool read, ByteBuffer &buffer);
    void HandleChangeSeatsOnControlledVehicle(bool read, ByteBuffer &buffer);
    // Embedded codes
    void HandleEmbeddedMovement(bool read, ByteBuffer &buffer);

private:
    // Internal functions
    void UpdateMovementFlagMask(uint8 flag = 0xFF)
    {
        if(flag != 0xFF && flag < 6)
        {
            m_movementFlagMask &= 1<<flag;
            if(GetFlags(flag) == 0)
                return;
            m_movementFlagMask |= 1<<flag;
            return;
        }

        m_movementFlagMask = 0;
        for(uint8 i = 0; i < 6; i++)
        {
            if(GetFlags(i) == 0)
                continue;
            m_movementFlagMask |= 1<<i;
        }
    }

    void ClearOptionalMovementData();

protected: // Movement information
    Mutex m_movementLock;
    // Vector linked to object position
    LocationVector *m_serverLocation;

    // Teleportation destination
    bool m_pendingDest;
    uint32 m_destMapId, m_destInstanceId;
    LocationVector m_teleportLocation;

    // Movement information
    bool m_isTransportLocked;
    WoWGuid m_moverGuid, m_transportGuid, m_clientGuid, m_clientTransGuid;
    uint8 m_movementFlagMask, m_movementFlags[6], m_serverFlags[6];
    LocationVector m_transportLocation, m_clientLocation, m_clientTransLocation;
    uint32 m_clientTime, m_serverTime, m_jumpTime, m_transportTime, m_transportTime2;
    int8 m_transportSeatId; uint32 m_vehicleId;

    float pitching, splineElevation;
    float m_jumpZSpeed, m_jump_XYSpeed, m_jump_sin, m_jump_cos;
    uint32 m_serverCounter, m_clientCounter;
    bool m_incrementMoveCounter;

    int32 m_MirrorTimer[3];
    uint32 m_LastUnderwaterState;
    uint32 m_UnderwaterTime;
    uint8 m_underwaterState;
    uint32 m_breathingUpdateTimer;

    float m_collisionHeight;
    bool m_isKnockBacked;

    bool m_isFalling;
    float m_fallPointZ;

    struct
    {
        WoWGuid ex_guid;
        float ex_float;
        int8  ex_byte;

        void set(WoWGuid xguid, float xfloat, uint8 xbyte)
        {
            ex_guid = xguid;
            ex_float = xfloat;
            ex_byte = xbyte;
        }
        void clear() { set(0, 0, 0); }
    } m_extra;

protected: // Speed and Status information
    float m_currSpeeds[MOVE_SPEED_MAX], m_speedOffset[MOVE_SPEED_MAX];

    // Pending speeds
    uint32 m_speedTimers[MOVE_SPEED_MAX], m_speedCounters[MOVE_SPEED_MAX];
    float m_pendingSpeeds[MOVE_SPEED_MAX];

    uint32 m_pendingDataTimer;
    bool m_pendingEnable[MOVEMENT_STATUS_MAX];

    bool UpdatePendingSpeed(MovementSpeedTypes speedType, float sentSpeed)
    {
        if(m_speedCounters[speedType] == 0)
            return false;
        if(--m_speedCounters[speedType] == 0 || sentSpeed == m_pendingSpeeds[speedType])
        {
            m_speedTimers[speedType] = 0; // Timer not needed anymore
            m_pendingSpeeds[speedType] = 0.0f; // Pending speed can be cleared
            m_currSpeeds[speedType] = sentSpeed;
        }
        return true;
    }

protected:
    uint32 m_timeSyncCounter, m_moveAckCounter;
    std::set<uint32> m_sentTimeSync, m_sentMoveAck;
    LockedQueue<MovementCodes> m_pendingMoveData;

private:
    Unit *m_Unit;

    uint32 m_movementState;

    UnitPathSystem m_path;
};
