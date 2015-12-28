/***
 * Demonstrike Core
 */

#include "StdAfx.h"

#define MOVEMENT_PACKET_TIME_DELAY 500

void WorldSession::HandleMovementOpcodes( WorldPacket & recv_data )
{
    if(!_player->IsInWorld())
        return;
    else if(_player->GetPlayerStatus() == TRANSFER_PENDING)
        return;
    else if(_player->GetCharmedByGUID() || _player->GetTaxiState())
        return;

    // spell cancel on movement, for now only fishing is added
    if (GameObject* t_go = _player->m_SummonedObject)
    {
        if (t_go->GetEntry() == GO_FISHING_BOBBER)
            castPtr<GameObject>(t_go)->EndFishing(GetPlayer(),true);
    }

    uint32 mstime = getMSTime();
    if(sEventMgr.HasEvent(_player, EVENT_PLAYER_FORCE_LOGOUT))
    {
        if(_player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
            sEventMgr.ModifyEventTimeAndTimeLeft(_player, EVENT_PLAYER_FORCE_LOGOUT, 1800000);
        else sEventMgr.RemoveEvents(_player, EVENT_PLAYER_FORCE_LOGOUT);
    }

    MovementInterface *moveInterface = _player->GetMovementInterface();
    if(!moveInterface->ReadFromClient(recv_data.GetOpcode(), &recv_data))
        Disconnect();
}

void WorldSession::HandleMovementInputOpcodes(WorldPacket & recv_data)
{
    if(!_player->IsInWorld())
        return;
    if(_player->GetPlayerStatus() == TRANSFER_PENDING)
        return;

    MovementInterface *moveInterface = _player->GetMovementInterface();
    if(!moveInterface->ReadFromClient(recv_data.GetOpcode(), &recv_data))
        Disconnect();
}

void WorldSession::HandleAcknowledgementOpcodes( WorldPacket & recv_data )
{
    if(!_player->IsInWorld())
        return;
    if(_player->GetPlayerStatus() == TRANSFER_PENDING)
        return;
    MovementInterface *moveInterface = _player->GetMovementInterface();
    if(!moveInterface->ReadFromClient(recv_data.GetOpcode(), &recv_data))
        Disconnect();
}

void WorldSession::HandleMoveWorldPortAck( WorldPacket & recv_data )
{
    if(_player->IsInWorld())
        return;
    if(_player->GetPlayerStatus() == NONE)
        return;
    MovementInterface *moveInterface = _player->GetMovementInterface();
    if(!moveInterface->ReadFromClient(recv_data.GetOpcode(), &recv_data))
        Disconnect();
}

void WorldSession::HandleSetActiveMoverOpcode( WorldPacket & recv_data )
{
    _player->GetMovementInterface()->SetActiveMover(&recv_data);
}

void WorldSession::HandleMoveTimeSkippedOpcode( WorldPacket & recv_data )
{
    _player->GetMovementInterface()->MoveTimeSkipped(&recv_data);
}

void WorldSession::HandleMoveSplineCompleteOpcode(WorldPacket &recv_data)
{
    _player->GetMovementInterface()->MoveSplineComplete(&recv_data);
}

void WorldSession::HandleMoveFallResetOpcode(WorldPacket & recv_data)
{
    _player->GetMovementInterface()->MoveFallReset(&recv_data);
}

#define DO_BIT(read, buffer, val, result) if(read) val = (buffer.ReadBit() ? result : !result); else buffer.WriteBit(val ? result : !result);
#define DO_COND_BIT(read, buffer, cond, val) if(cond) { if(read) val = buffer.ReadBit(); else buffer.WriteBit(val); }
#define DO_BYTES(read, buffer, type, val) if(read) val = buffer.read<type>(); else buffer.append<type>(val);
#define DO_COND_BYTES(read, buffer, cond, type, val) if(cond) { if(read) val = buffer.read<type>(); else buffer.append<type>(val); }
#define DO_SEQ_BYTE(read, buffer, val) if(read) buffer.ReadByteSeq(val); else buffer.WriteByteSeq(val);

// Close with a semicolon
#define BUILD_LOCATION_LIST() LocationVector *pos = read ? &m_clientLocation : m_serverLocation;\
    LocationVector *transPos = read ? &m_clientTransLocation : &m_transportLocation

#define BUILD_BOOL_LIST()  bool hasMovementFlags = read ? false : m_movementFlagMask & 0x0F,\
    hasMovementFlags2 = read ? false : m_movementFlagMask & 0xF0,\
    hasTimestamp = read ? false : true,\
    hasOrientation = read ? false : !G3D::fuzzyEq(m_serverLocation->o, 0.0f),\
    hasTransportData = read ? false : !m_transportGuid.empty(),\
    hasSpline = read ? false : isSplineMovingActive(),\
    hasTransportTime2 = read ? false : (hasTransportData && m_transportTime2 != 0),\
    hasTransportVehicleId = read ? false : (hasTransportData && m_vehicleId != 0),\
    hasPitch = read ? false : (hasFlag(MOVEMENTFLAG_SWIMMING) || hasFlag(MOVEMENTFLAG_FLYING) || hasFlag(MOVEMENTFLAG_ALWAYS_ALLOW_PITCHING)),\
    hasFallDirection = read ? false : hasFlag(MOVEMENTFLAG_TOGGLE_FALLING),\
    hasFallData = read ? false : (hasFallDirection || m_jumpTime != 0),\
    hasSplineElevation = read ? false : hasFlag(MOVEMENTFLAG_SPLINE_ELEVATION);\
    uint32 *msTime = read ? &m_clientTime : &m_serverTime;

#define BUILD_GUID_LIST() WoWGuid *guid = read ? &m_clientGuid : &m_moverGuid;\
    WoWGuid *tguid = read ? &m_clientTransGuid : &m_transportGuid;

void MovementInterface::HandlePlayerMove(bool read, ByteBuffer &buffer)
{
    LocationVector *pos = &m_clientLocation;
    LocationVector *transPos = &m_clientTransLocation;

    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasTransportData, true);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_BIT(read, buffer, hasPitch, false);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_BYTES(read, buffer, float, pos->y);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_BYTES(read, buffer, float, pos->x);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_BYTES(read, buffer, float, pos->z);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
}

void MovementInterface::HandleHeartbeat(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasSpline, true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
}

void MovementInterface::HandleJump(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
}

void MovementInterface::HandleFallLand(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
}

void MovementInterface::HandleStartForward(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasFallData, true);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
}

void MovementInterface::HandleStartBackward(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasPitch, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
}

void MovementInterface::HandleStartStrafeLeft(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
}

void MovementInterface::HandleStartStrafeRight(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasSpline, true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
}

void MovementInterface::HandleStartTurnLeft(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasFallData, true);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
}

void MovementInterface::HandleStartTurnRight(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[7], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
}

void MovementInterface::HandleStartPitchDown(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasPitch, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
}

void MovementInterface::HandleStartPitchUp(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
}

void MovementInterface::HandleStartAscend(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
}

void MovementInterface::HandleStartDescend(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
}

void MovementInterface::HandleStartSwim(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
}

void MovementInterface::HandleStop(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
}

void MovementInterface::HandleStopStrafe(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
}

void MovementInterface::HandleStopTurn(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasFallData, true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
}

void MovementInterface::HandleStopPitch(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasOrientation, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
}

void MovementInterface::HandleStopAscend(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasSpline, true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
}

void MovementInterface::HandleStopSwim(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
}

void MovementInterface::HandleSetFacing(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
}

void MovementInterface::HandleSetPitch(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
}

void MovementInterface::HandleFallReset(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasMovementFlags, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
}

void MovementInterface::HandleSetRunMode(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasTransportData, true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
}

void MovementInterface::HandleSetWalkMode(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
}

void MovementInterface::HandleSetPitchRate(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DoExtraData(MOVEMENT_CODE_SET_PITCH_RATE, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleSetCanFly(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleUnsetCanFly(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleSetHover(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleUnsetHover(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleWaterWalk(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleLandWalk(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleFeatherFall(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleNormalFall(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleRoot(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleUnroot(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleUpdateKnockBack(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_BYTES(read, buffer, float, pos->z);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_BYTES(read, buffer, float, pos->x);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_BYTES(read, buffer, float, pos->y);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
}

void MovementInterface::HandleUpdateTeleport(bool read, ByteBuffer &buffer)
{
    LocationVector *pos = read ? &m_clientLocation : &m_teleportLocation;
    LocationVector *transPos = read ? &m_clientTransLocation : &m_transportLocation;
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_BIT(read, buffer, hasSplineElevation, false);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
}

void MovementInterface::HandleChangeTransport(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasPitch, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
}

void MovementInterface::HandleNotActiveMover(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasMovementFlags, false);

    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);

    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);

    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);

    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);

    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);

    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
}

void MovementInterface::HandleSetCollisionHeight(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DoExtraData(MOVEMENT_CODE_SET_COLLISION_HEIGHT, read, &buffer);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleUpdateCollisionHeight(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DoExtraData(MOVEMENT_CODE_UPDATE_COLLISION_HEIGHT, read, &buffer);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_BIT(read, buffer, (*guid)[1], true);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
}

void MovementInterface::HandleUpdateWalkSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, hasOrientation, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_BYTES(read, buffer, float, pos->y);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DoExtraData(MOVEMENT_CODE_UPDATE_WALK_SPEED, read, &buffer);
}

void MovementInterface::HandleUpdateRunSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DoExtraData(MOVEMENT_CODE_UPDATE_RUN_SPEED, read, &buffer);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_BIT(read, buffer, (*guid)[3], true);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_BIT(read, buffer, hasFallData, true);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_BIT(read, buffer, (*guid)[4], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
}

void MovementInterface::HandleUpdateRunBackSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasOrientation, false);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DoExtraData(MOVEMENT_CODE_UPDATE_RUN_BACK_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_BYTES(read, buffer, float, pos->z);
}

void MovementInterface::HandleUpdateSwimSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_BIT(read, buffer, (*guid)[3], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_BYTES(read, buffer, float, pos->x);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DoExtraData(MOVEMENT_CODE_UPDATE_SWIM_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
}

void MovementInterface::HandleUpdateSwimBackSpeed(bool read, ByteBuffer &buffer)
{
    printf("HandleUpdateSwimBackSpeed\n");
    { for(uint8 i = 3, v = 0, h = 1; v > 3; v++) h = i/v; };
}

void MovementInterface::HandleUpdateFlightSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DoExtraData(MOVEMENT_CODE_UPDATE_FLIGHT_SPEED, read, &buffer);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
}

void MovementInterface::HandleUpdateFlightBackSpeed(bool read, ByteBuffer &buffer)
{
    printf("HandleUpdateFlightBackSpeed\n");
    { for(uint8 i = 3, v = 0, h = 1; v > 3; v++) h = i/v; };
}

void MovementInterface::HandleUpdateTurnRate(bool read, ByteBuffer &buffer)
{
    printf("HandleUpdateTurnRate\n");
    { for(uint8 i = 3, v = 0, h = 1; v > 3; v++) h = i/v; };
}

void MovementInterface::HandleUpdatePitchRate(bool read, ByteBuffer &buffer)
{
    printf("HandleUpdatePitchRate\n");
    { for(uint8 i = 3, v = 0, h = 1; v > 3; v++) h = i/v; };
}

void MovementInterface::HandleSetWalkSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DoExtraData(MOVEMENT_CODE_SET_WALK_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleSetRunSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DoExtraData(MOVEMENT_CODE_SET_RUN_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleSetRunBackSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DoExtraData(MOVEMENT_CODE_SET_RUN_BACK_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
}

void MovementInterface::HandleSetSwimSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DoExtraData(MOVEMENT_CODE_SET_SWIM_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleSetSwimBackSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DoExtraData(MOVEMENT_CODE_SET_SWIM_BACK_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleSetFlightSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DoExtraData(MOVEMENT_CODE_SET_FLIGHT_SPEED, read, &buffer);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleSetFlightBackSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DoExtraData(MOVEMENT_CODE_SET_FLIGHT_BACK_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleSetTurnRate(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DoExtraData(MOVEMENT_CODE_SET_TURN_RATE, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckTeleport(bool read, ByteBuffer &buff)
{
    Player *plr = castPtr<Player>(m_Unit);
    plr->SetPlayerStatus(NONE);
    sEventMgr.RemoveEvents(plr, EVENT_PLAYER_CHECK_STATUS_Transfer);
    if(!plr->IsInWorld())
    {
        Transporter *trans = NULL;
        if((trans = plr->m_CurrentTransporter) && plr->GetMapId() != trans->GetMapId())
        {
            /* wow, our pc must really suck. */
            float c_tposx, c_tposy, c_tposz, c_tposo;
            plr->GetMovementInterface()->GetTransportPosition(c_tposx, c_tposy, c_tposz, c_tposo);
            c_tposx += trans->GetPositionX();
            c_tposy += trans->GetPositionY();
            c_tposz += trans->GetPositionZ();

            WorldPacket dataw(SMSG_NEW_WORLD, 20);
            dataw << c_tposx << c_tposo << c_tposy << trans->GetMapId() << c_tposz;
            plr->SendPacket(&dataw);

            plr->SetMapId(trans->GetMapId());
            plr->SetInstanceID(trans->GetInstanceID());
            plr->SetPosition(c_tposx, c_tposy, c_tposz, c_tposo);
            if(trans->IsInWorld())
                sWorldMgr.PushToWorldQueue(plr);
        }
        else
        {
            // don't overwrite the loading flag here.
            // reason: talents/passive spells don't get cast on an invalid instance login
            if( plr->m_TeleportState != 1 )
                plr->m_TeleportState = 2;

            plr->SetMapId(m_destMapId);
            plr->SetInstanceID(m_destInstanceId);
            plr->SetPosition(m_teleportLocation);
            m_clientLocation.ChangeCoords(m_teleportLocation.x, m_teleportLocation.y, m_teleportLocation.z, m_teleportLocation.o);
            sWorldMgr.PushToWorldQueue(plr);
        }
    }
}

void MovementInterface::HandleAckRoot(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasMovementFlags, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckUnroot(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckFeatherFall(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckForceWalkSpeedChange(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DoExtraData(MOVEMENT_CODE_ACK_FORCE_WALK_SPEED_CHANGE, read, &buffer);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasSpline, true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckForceRunSpeedChange(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->x);
    DoExtraData(MOVEMENT_CODE_ACK_FORCE_RUN_SPEED_CHANGE, read, &buffer);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckForceRunBackSpeedChange(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DoExtraData(MOVEMENT_CODE_ACK_FORCE_RUN_BACK_SPEED_CHANGE, read, &buffer);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckForceSwimSpeedChange(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DoExtraData(MOVEMENT_CODE_ACK_FORCE_SWIM_SPEED_CHANGE, read, &buffer);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckForceSwimBackSpeedChange(bool read, ByteBuffer &buffer)
{
    printf("HandleAckForceSwimBackSpeedChange\n");
    { for(uint8 i = 3, v = 0, h = 1; v > 3; v++) h = i/v; };
}

void MovementInterface::HandleAckForceFlightSpeedChange(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->y);
    DoExtraData(MOVEMENT_CODE_ACK_FORCE_FLIGHT_SPEED_CHANGE, read, &buffer);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckForceFlightBackSpeedChange(bool read, ByteBuffer &buffer)
{
    printf("HandleAckForceFlightBackSpeedChange\n");
    { for(uint8 i = 3, v = 0, h = 1; v > 3; v++) h = i/v; };
}

void MovementInterface::HandleAckForceTurnRateChange(bool read, ByteBuffer &buffer)
{
    printf("HandleAckForceTurnRateChange\n");
    { for(uint8 i = 3, v = 0, h = 1; v > 3; v++) h = i/v; };
}

void MovementInterface::HandleAckForcePitchRateChange(bool read, ByteBuffer &buffer)
{
    printf("HandleAckForcePitchRateChange\n");
    { for(uint8 i = 3, v = 0, h = 1; v > 3; v++) h = i/v; };
}

void MovementInterface::HandleAckGravityEnable(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasOrientation, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckGravityDisable(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckHover(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckWaterWalk(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[4], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckKnockBack(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[7], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasFallData, true);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckSetCanFly(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckSetCollisionHeight(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DoExtraData(MOVEMENT_CODE_ACK_SET_COLLISION_HEIGHT, read, &buffer);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->y);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleAckSetCanTransitionBetweenSwimAndFly(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, uint32, (read ? m_clientCounter : m_serverCounter));
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasSpline, true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    m_incrementMoveCounter = true;
}

void MovementInterface::HandleSplineDone(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
}

void MovementInterface::HandleSplineSetWalkSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DoExtraData(MOVEMENT_CODE_SPLINE_SET_WALK_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
}

void MovementInterface::HandleSplineSetRunSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DoExtraData(MOVEMENT_CODE_SPLINE_SET_RUN_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
}

void MovementInterface::HandleSplineSetRunBackSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DoExtraData(MOVEMENT_CODE_SPLINE_SET_RUN_BACK_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
}

void MovementInterface::HandleSplineSetSwimSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DoExtraData(MOVEMENT_CODE_SPLINE_SET_SWIM_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
}

void MovementInterface::HandleSplineSetSwimBackSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DoExtraData(MOVEMENT_CODE_SPLINE_SET_SWIM_BACK_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
}

void MovementInterface::HandleSplineSetFlightSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DoExtraData(MOVEMENT_CODE_SPLINE_SET_FLIGHT_SPEED, read, &buffer);
}

void MovementInterface::HandleSplineSetFlightBackSpeed(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DoExtraData(MOVEMENT_CODE_SPLINE_SET_FLIGHT_BACK_SPEED, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
}

void MovementInterface::HandleSplineSetPitchRate(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DoExtraData(MOVEMENT_CODE_SPLINE_SET_PITCH_RATE, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
}

void MovementInterface::HandleSplineSetTurnRate(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DoExtraData(MOVEMENT_CODE_SPLINE_SET_TURN_RATE, read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
}

void MovementInterface::HandleSplineSetWalkMode(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
}

void MovementInterface::HandleSplineSetRunMode(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
}

void MovementInterface::HandleSplineGravityEnable(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
}

void MovementInterface::HandleSplineGravityDisable(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
}

void MovementInterface::HandleSplineSetHover(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
}

void MovementInterface::HandleSplineSetUnhover(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
}

void MovementInterface::HandleSplineStartSwim(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
}

void MovementInterface::HandleSplineStopSwim(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
}

void MovementInterface::HandleSplineSetFlying(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
}

void MovementInterface::HandleSplineUnsetFlying(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
}

void MovementInterface::HandleSplineSetWaterWalk(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
}

void MovementInterface::HandleSplineSetLandWalk(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
}

void MovementInterface::HandleSplineSetFeatherFall(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
}

void MovementInterface::HandleSplineSetNormalFall(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
}

void MovementInterface::HandleSplineRoot(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
}

void MovementInterface::HandleSplineUnroot(bool read, ByteBuffer &buffer)
{
    BUILD_GUID_LIST();
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
}

void MovementInterface::HandleDismissControlledVehicle(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasOrientation, false);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
}

void MovementInterface::HandleChangeSeatsOnControlledVehicle(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BYTES(read, buffer, float, pos->z);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 0);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 1);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 2);
    DO_BIT(read, buffer, hasOrientation, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 3);
    DO_BIT(read, buffer, (*guid)[7], true);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 4);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, (*guid)[5], true);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 5);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, hasPitch, false);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 6);
    DO_BIT(read, buffer, (*guid)[0], true);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 7);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, hasSpline, true);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 8);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 9);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 10);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 11);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 12);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 13);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 14);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 15);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DoExtraData(MOVEMENT_CODE_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, read, &buffer, 16);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
}

void MovementInterface::HandleEmbeddedMovement(bool read, ByteBuffer &buffer)
{
    BUILD_LOCATION_LIST();
    BUILD_BOOL_LIST();
    BUILD_GUID_LIST();
    DO_BYTES(read, buffer, float, pos->z);
    DO_BYTES(read, buffer, float, pos->y);
    DO_BYTES(read, buffer, float, pos->x);
    DO_BIT(read, buffer, hasFallData, true);
    DO_BIT(read, buffer, hasTimestamp, false);
    DO_BIT(read, buffer, hasOrientation, false);
    read ? buffer.ReadBit() : buffer.WriteBit(0); // Skip_bit
    DO_BIT(read, buffer, hasSpline, true);
    DO_BIT(read, buffer, (*guid)[6], true);
    DO_BIT(read, buffer, (*guid)[4], true);
    DO_BIT(read, buffer, hasMovementFlags2, false);
    DO_BIT(read, buffer, (*guid)[3], true);
    DO_BIT(read, buffer, (*guid)[5], true);
    DO_BIT(read, buffer, hasSplineElevation, false);
    DO_BIT(read, buffer, hasPitch, false);
    DO_BIT(read, buffer, (*guid)[7], true);
    DO_BIT(read, buffer, hasTransportData, true);
    DO_BIT(read, buffer, (*guid)[2], true);
    DO_BIT(read, buffer, hasMovementFlags, false);
    DO_BIT(read, buffer, (*guid)[1], true);
    DO_BIT(read, buffer, (*guid)[0], true);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[6]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[2]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[5]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportTime2);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[7]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[4]);
    DO_COND_BIT(read, buffer, hasTransportData, hasTransportVehicleId);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[0]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[1]);
    DO_COND_BIT(read, buffer, hasTransportData, (*tguid)[3]);
    if(hasMovementFlags2) HandleMovementFlags2(read, &buffer);
    if(hasMovementFlags) HandleMovementFlags(read, &buffer);
    DO_COND_BIT(read, buffer, hasFallData, hasFallDirection);
    DO_SEQ_BYTE(read, buffer, (*guid)[1]);
    DO_SEQ_BYTE(read, buffer, (*guid)[4]);
    DO_SEQ_BYTE(read, buffer, (*guid)[7]);
    DO_SEQ_BYTE(read, buffer, (*guid)[3]);
    DO_SEQ_BYTE(read, buffer, (*guid)[0]);
    DO_SEQ_BYTE(read, buffer, (*guid)[2]);
    DO_SEQ_BYTE(read, buffer, (*guid)[5]);
    DO_SEQ_BYTE(read, buffer, (*guid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData, int8, m_transportSeatId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->o);
    DO_COND_BYTES(read, buffer, hasTransportData, uint32, m_transportTime);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[6], uint8, (*tguid)[6]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[5], uint8, (*tguid)[5]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportVehicleId, uint32, m_vehicleId);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->x);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[4], uint8, (*tguid)[4]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->z);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[2], uint8, (*tguid)[2]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[0], uint8, (*tguid)[0]);
    DO_COND_BYTES(read, buffer, hasTransportData && hasTransportTime2, uint32, m_transportTime2);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[1], uint8, (*tguid)[1]);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[3], uint8, (*tguid)[3]);
    DO_COND_BYTES(read, buffer, hasTransportData, float, transPos->y);
    DO_COND_BYTES(read, buffer, hasTransportData && (*tguid)[7], uint8, (*tguid)[7]);
    DO_COND_BYTES(read, buffer, hasOrientation, float, pos->o);
    DO_COND_BYTES(read, buffer, hasSplineElevation, float, splineElevation);
    DO_COND_BYTES(read, buffer, hasFallData, uint32, m_jumpTime);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_XYSpeed);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_sin);
    DO_COND_BYTES(read, buffer, hasFallDirection, float, m_jump_cos);
    DO_COND_BYTES(read, buffer, hasFallData, float, m_jumpZSpeed);
    DO_COND_BYTES(read, buffer, hasTimestamp, uint32, (*msTime));
    DO_COND_BYTES(read, buffer, hasPitch, float, pitching);
}

#undef BUILD_BOOL_LIST
#undef BUILD_GUID_LIST
#undef DO_BIT
#undef DO_BYTES
#undef DO_COND_BYTES
#undef DO_SEQ_BYTE
