/***
 * Demonstrike Core
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

        bool isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, G3D::g3d_int32 phasemask) const;
        bool getIntersectionTime(const G3D::Ray& ray, const G3D::Vector3& endPos, float& maxDist, bool pStopAtFirstHit, G3D::g3d_int32 phasemask) const;
        bool getObjectHitPos(const G3D::Vector3& pPos1, const G3D::Vector3& pPos2, G3D::Vector3& pResultHitPos, float pModifyDist, G3D::g3d_int32 phasemask) const;
        float getHeight(float x, float y, float z, float maxSearchDist, G3D::g3d_int32 phasemask) const;

        void insert(const GOModelInstance&);
        void remove(const GOModelInstance&);
        bool contains(const GOModelInstance&) const;
        int size() const;

        void balance();
        void update(G3D::g3d_uint32 diff);
    };
}