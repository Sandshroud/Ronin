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

    void EnableAutoPath() { m_autoPath = true; }
    void DisableAutoPath() { m_autoPath = false; }

    bool hasDestination();
    bool closeToDestination(uint32 msTime);

    bool GetDestination(float &x, float &y, float *z = NULL);

    void SetSpeed(MovementSpeedTypes speedType);

    uint8 getPathPool() { return pathPoolId; }
    void setPathPool(uint8 poolId) { pathPoolId = poolId; }

private:
    void _CleanupPath();

public:
    void MoveToPoint(float x, float y, float z, float o = fInfinite);
    void StopMoving();

    void BroadcastMovementPacket();
    void SendMovementPacket(Player *plr);

private:
    Unit *m_Unit;
    bool m_autoPath;

    WaypointStorage *_waypointPath;
    WaypointStorage::iterator pathIterator;
    uint32 m_pendingAutoPathDelay;

    MovementSpeedTypes _moveSpeed;

    uint32 m_pathCounter, m_pathStartTime, m_pathLength;

    MovementPoint srcPoint;
    float _destX, _destY, _destZ, _destO;

    std::deque<MovementPoint*> m_movementPoints;

    uint32 m_lastMSTimeUpdate, m_lastPositionUpdate;
    uint8 pathPoolId;
};
