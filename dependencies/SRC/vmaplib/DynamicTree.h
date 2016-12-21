/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

namespace VMAP
{
    class GameobjectModelInstance;
    struct DynTreeImpl;

    typedef GameobjectModelInstance GOModelInstance;
    typedef BIHWrap<GOModelInstance> ModelWrap;
    //typedef G3D::Table<const GOModelInstance*, ModelWrap*> MemberTable;
    typedef std::map<const GOModelInstance*, std::set<ModelWrap*>> MemberTable;

    class DynamicMapTree
    {
        DynTreeImpl *impl;

    public:

        DynamicMapTree();
        ~DynamicMapTree();

        bool isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, G3D::int32 phasemask) const;
        bool getIntersectionTime(const G3D::Ray& ray, const G3D::Vector3& endPos, float& maxDist, bool pStopAtFirstHit, G3D::int32 phasemask) const;
        bool getObjectHitPos(const G3D::Vector3& pPos1, const G3D::Vector3& pPos2, G3D::Vector3& pResultHitPos, float pModifyDist, G3D::int32 phasemask) const;
        float getHeight(float x, float y, float z, float maxSearchDist, G3D::int32 phasemask) const;

        void insert(const GOModelInstance&);
        void remove(const GOModelInstance&);
        bool contains(const GOModelInstance&) const;
        int size() const;

        void balance();
        void update(G3D::uint32 diff);
    };
}