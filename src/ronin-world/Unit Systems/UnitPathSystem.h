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

enum MovementSpeedTypes : uint8;

struct MovementPoint
{
    MovementPoint() : pos(0.f, 0.f, 0.f), timeStamp(0) {}
    MovementPoint(uint32 time, float x, float y, float z) : pos(x, y, z), timeStamp(time) {}
    Position pos;
    uint32 timeStamp;
    float orientationOverride;
};

enum PacketBroadcastFlags
{
    MOVEBCFLAG_NONE     = 0x00,
    MOVEBCFLAG_UNCOMP   = 0x01,

    MOVEBCFLAG_DELAYED  = 0x08,
};

class UnitPathSystem
{
public: // Class defines
    static float fInfinite;

public: // Class functions
    UnitPathSystem(Unit *unit);
    ~UnitPathSystem();

    bool IsActiveObject();
    bool IsActivated();

    void InactiveUpdate(uint32 msTime, uint32 uiDiff);
    bool Update(uint32 msTime, uint32 uiDiff, bool fromMovement = false);

    void EnterEvade();
    void SetAutoPath(WaypointStorage *storage);

    void EnableAutoPath() { m_autoPath = (_waypointPath != NULL); }
    void DisableAutoPath() { m_autoPath = false; }
    void ToggleMovementLock(bool result) { m_movementDisabled = result; }

    bool hasDestination();
    bool closeToDestination(uint32 msTime);

    bool GetDestination(float &x, float &y, float *z = NULL);

    void SetSpeed(MovementSpeedTypes speedType);

private:
    void _CleanupPath();
    uint32 buildMonsterMoveFlags(uint8 packetSendFlags);

public:
    void SetFollowTarget(Unit *target, float distance);
    void MoveToPoint(float x, float y, float z, float o = fInfinite);
    void UpdateOrientation(Unit *unitTarget);
    void SetOrientation(float orientation);

    void StopMoving();

    void BroadcastMovementPacket(uint8 packetSendFlags = 0);
    void SendMovementPacket(Player *plr, uint8 packetSendFlags = 0);

    void AppendMoveBits(ByteBuffer *buffer, uint32 msTime, std::vector<MovementPoint*> *pointStorage);
    void AppendMoveBytes(ByteBuffer *buffer, uint32 msTime, std::vector<MovementPoint*> *pointStorage);

private:
    Unit *m_Unit, *m_followTarget;
    bool m_movementDisabled, m_autoPath;

    WaypointStorage *_waypointPath;
    WaypointStorage::iterator waypathIterator;
    uint32 m_autoPathDelay, m_pendingAutoPathDelay;

    MovementSpeedTypes _moveSpeed;

    uint32 m_pathCounter, m_pathStartTime, m_pathLength;

    MovementPoint srcPoint, lastUpdatePoint;
    float _destX, _destY, _destZ, _destO;

    FastQueue<std::shared_ptr<MovementPoint>, Mutex> m_movementPoints;

    uint32 m_lastMSTimeUpdate, m_lastPositionUpdate;
};
