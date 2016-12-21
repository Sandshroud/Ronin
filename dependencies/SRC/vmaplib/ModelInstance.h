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

#include "ModelSpawn.h"

namespace VMAP
{
    class WorldModel;
    class GroupModel;
    struct WMOData;
    struct AreaInfo;
    struct LocationInfo;

    class GameobjectModelSpawn
    {
        public:
            G3D::AABox BoundBase;
            std::string name;
            const G3D::AABox& getBaseBounds() const { return BoundBase; }
    };

    struct locationCallback
    {
        locationCallback() {}
        virtual void operator()(float x, float y, float z) {};
    };

    class ModelInstance: public ModelSpawn
    {
        public:
            ModelInstance(): iInvScale(0.0f), iModel(0) {}
            ModelInstance(const ModelSpawn &spawn, WorldModel* model);
            void setUnloaded() { iModel = 0; }
            bool intersectRay(const G3D::Ray& pRay, float& pMaxDist, bool pStopAtFirstHit) const;
            void getWMOData(const G3D::Vector3& p, WMOData &data, G3D::int32 requiredFlags, G3D::int32 ignoredFlags) const;
            void intersectPoint(const G3D::Vector3& p, AreaInfo &info) const;
            bool GetLocationInfo(const G3D::Vector3& p, LocationInfo &info) const;
            bool GetLiquidLevel(const G3D::Vector3& p, const VMAP::GroupModel *model, float &liqHeight) const;
            void CalcOffsetDirection(const G3D::Vector3 pos, G3D::Vector3 &p, G3D::Vector3 &up) const;

        protected:
            G3D::Matrix3 iInvRot;
            float iInvScale;
            G3D::AABox iModelBound;
            WorldModel* iModel;
        public:
            WorldModel* getWorldModel();
    };

    class GameobjectModelInstance : public GameobjectModelSpawn
    {
        public:
            GameobjectModelInstance() : iInvScale(0.0f), iScale(0.0f), iModel(0), m_PhaseMask(-1), iOrientation(0.0f) {}
            GameobjectModelInstance(const GameobjectModelSpawn &spawn, WorldModel* model, G3D::int32 m_phase);

            void setUnloaded() { iModel = NULL; iBound = G3D::AABox(G3D::Vector3(0, 0, 0)); }
            void LoadModel(WorldModel* m, G3D::AABox& bound) { if(m != NULL) { iModel = m; BoundBase = bound; }};
            void SetData(float x, float y, float z, float orientation, float scale);
            bool intersectRay(const G3D::Ray& pRay, float& pMaxDist, bool pStopAtFirstHit, G3D::int32 m_phase) const;
            const float GetOrientation() const { return iOrientation; };
            const G3D::Vector3& getPosition() const { return iPos;}
            const G3D::AABox& getBounds() const { return iBound; };
            const float GetScale() const { return iScale; };
        protected:
            G3D::int32 m_PhaseMask;
            WorldModel* iModel;

            float iInvScale;
            float iScale;
            float iOrientation;

            G3D::AABox iBound;
            G3D::Vector3 iPos;
            G3D::Matrix3 iInvRot;
        public:
            WorldModel* getWorldModel();
    };
} // namespace VMAP
