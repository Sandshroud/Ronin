/*
 * Copyright (C) 2014-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define MMAP_UNAVAILABLE 424242.42f // 'MMAP'

struct Position
{
    Position(float _x, float _y, float _z) { x = _x; y = _y; z = _z; };
    float x, y, z;
};

struct PositionMapContainer
{
    std::map<uint32, Position> InternalMap;
};

class MMapManagerExt
{
public:
    virtual bool LoadNavMesh(uint32 x, uint32 y) = 0;
    virtual void UnloadNavMesh(uint32 x, uint32 y) = 0;
    virtual bool IsNavmeshLoaded(uint32 x, uint32 y) = 0;

    virtual Position getNextPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz) = 0;
    virtual Position getBestPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz) = 0;
    virtual PositionMapContainer* BuildFullPath(unsigned short moveFlags, float startx, float starty, float startz, float endx, float endy, float endz, bool straight) = 0;

    virtual bool GetWalkingHeightInternal(float startx, float starty, float startz, float endz, Position& out) = 0;
    virtual bool getNextPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz, Position& out) = 0;
};