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

class SERVER_DECL SmartBounding
{
protected:
    struct VectPoint { float x, y, z; };

public:
    SmartBounding() {}
    ~SmartBounding() {}

    void Update(uint32 msTime, uint32 uiDiff) {}

    LocationVector GetPosition(uint32 msTime) {}

    void Process(float sX, float sY, float sZ, float eX, float eY, float eZ) {}
    void OverridePosition(float x, float y, float z) {}

private:
    uint32 lastUpdateMS, timeToTarget;

    float startX, startY, startZ;
    float endX, endY, endZ;

    std::vector<std::shared_ptr<VectPoint>> m_history;
};
