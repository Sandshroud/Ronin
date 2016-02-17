/***
 * Demonstrike Core
 */

#include <g3dlite\G3D.h>
#include "VMapLib.h"
#include "VMapDefinitions.h"

using G3D::Vector3;
using G3D::Ray;

namespace VMAP
{
    ModelInstance::ModelInstance(const ModelSpawn &spawn, WorldModel* model): ModelSpawn(spawn), iModel(model)
    {
        iInvRot = G3D::Matrix3::fromEulerAnglesZYX(G3D::pi()*iRot.y/180.f, G3D::pi()*iRot.x/180.f, G3D::pi()*iRot.z/180.f).inverse();
        iInvScale = 1.f/iScale;
    }

    bool ModelInstance::intersectRay(const G3D::Ray& pRay, float& pMaxDist, bool pStopAtFirstHit) const
    {
        if (!iModel)
            return false;
        float time = pRay.intersectionTime(iBound);
        if (time == G3D::inf())
            return false;
        Vector3 p = iInvRot * (pRay.origin() - iPos) * iInvScale;
        Ray modRay(p, iInvRot * pRay.direction());
        float distance = pMaxDist * iInvScale;
        bool hit = iModel->IntersectRay(modRay, distance, pStopAtFirstHit);
        if (hit)
        {
            distance *= iScale;
            pMaxDist = distance;
        }
        return hit;
    }

    void ModelInstance::intersectPoint(const G3D::Vector3& p, AreaInfo &info) const
    {
        if (!iModel)
        {
            OUT_DEBUG("<object not loaded>");
            return;
        }

        // M2 files don't contain area info, only WMO files
        if (flags & MOD_M2)
            return;
        if (!iBound.contains(p))
            return;
        // child bounds are defined in object space:
        Vector3 pModel = iInvRot * (p - iPos) * iInvScale;
        Vector3 zDirModel = iInvRot * Vector3(0.f, 0.f, -1.f);
        float zDist;
        if (iModel->IntersectPoint(pModel, zDirModel, zDist, info))
        {
            Vector3 modelGround = pModel + zDist * zDirModel;
            // Transform back to world space. Note that:
            // Mat * vec == vec * Mat.transpose()
            // and for rotation matrices: Mat.inverse() == Mat.transpose()
            float world_Z = ((modelGround * iInvRot) * iScale + iPos).z;
            if (info.ground_Z < world_Z)
            {
                info.ground_Z = world_Z;
                info.adtId = adtId;
            }
        }
    }

    bool ModelInstance::GetLocationInfo(const G3D::Vector3& p, LocationInfo &info) const
    {
        if (!iModel)
        {
            OUT_DEBUG("<object not loaded>");
            return false;
        }

        // M2 files don't contain area info, only WMO files
        if (flags & MOD_M2)
            return false;
        if (!iBound.contains(p))
            return false;
        // child bounds are defined in object space:
        Vector3 pModel = iInvRot * (p - iPos) * iInvScale;
        Vector3 zDirModel = iInvRot * Vector3(0.f, 0.f, -1.f);
        float zDist;
        if (iModel->GetLocationInfo(pModel, zDirModel, zDist, info))
        {
            Vector3 modelGround = pModel + zDist * zDirModel;
            // Transform back to world space. Note that:
            // Mat * vec == vec * Mat.transpose()
            // and for rotation matrices: Mat.inverse() == Mat.transpose()
            float world_Z = ((modelGround * iInvRot) * iScale + iPos).z;
            if (info.ground_Z < world_Z) // hm...could it be handled automatically with zDist at intersection?
            {
                info.ground_Z = world_Z;
                info.hitInstance = this;
                return true;
            }
        }
        return false;
    }

    bool ModelInstance::GetLiquidLevel(const G3D::Vector3& p, LocationInfo &info, float &liqHeight) const
    {
        // child bounds are defined in object space:
        Vector3 pModel = iInvRot * (p - iPos) * iInvScale;
        //Vector3 zDirModel = iInvRot * Vector3(0.f, 0.f, -1.f);
        float zDist;
        if (info.hitModel->GetLiquidLevel(pModel, zDist))
        {
            // calculate world height (zDist in model coords):
            // assume WMO not tilted (wouldn't make much sense anyway)
            liqHeight = zDist * iScale + iPos.z;
            return true;
        }
        return false;
    }

    GameobjectModelInstance::GameobjectModelInstance(const GameobjectModelSpawn &spawn, WorldModel* model, G3D::int32 m_phase) : iModel(model), m_PhaseMask(m_phase)
    {
        BoundBase = spawn.BoundBase;
        name = spawn.name;
    }

    void GameobjectModelInstance::SetData(float x, float y, float z, float orientation, float scale)
    {
        iPos = Vector3(x, y, z);
        iScale = scale;
        iInvScale = 1.0f / iScale;
        iOrientation = orientation;

        G3D::Matrix3 iRotation = G3D::Matrix3::fromEulerAnglesZYX(iOrientation, 0, 0);
        iInvRot = iRotation.inverse();

        // transform bounding box:
        G3D::AABox box = G3D::AABox(BoundBase.low() * iScale, BoundBase.high() * iScale), rotated_bounds;
        for (int i = 0; i < 8; ++i)
            rotated_bounds.merge(iRotation * box.corner(i));
        iBound = rotated_bounds + iPos;
    }

    bool GameobjectModelInstance::intersectRay(const G3D::Ray& ray, float& MaxDist, bool StopAtFirstHit, G3D::int32 ph_mask) const
    {
        if(m_PhaseMask != -1 && ph_mask != -1)
            if (!(m_PhaseMask & ph_mask))
                return false;
        float time = ray.intersectionTime(getBounds());
        if (time == G3D::inf())
            return false;

        // child bounds are defined in object space:
        Vector3 p = iInvRot * (ray.origin() - iPos) * iInvScale;
        Ray modRay(p, iInvRot * ray.direction());
        float distance = MaxDist * iInvScale;
        bool hit = iModel->IntersectRay(modRay, distance, StopAtFirstHit);
        if (hit)
        {
            distance *= iScale;
            MaxDist = distance;
        }
        return hit;
    }

}
