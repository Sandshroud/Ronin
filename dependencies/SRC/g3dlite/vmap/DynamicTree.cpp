/***
 * Demonstrike Core
 */

#include "../G3DAll.h"
#include "VMapDefinitions.h"

#define USE_CELL_STRUCTURE
#define TILE_COUNT 64
#define TILE_SIZE  533.33333f
#define MAP_SIZE   (TILE_SIZE * float(TILE_COUNT))

#define CELL_PER_TILE 8
#define CELL_COUNT    (TILE_COUNT*CELL_PER_TILE)
#define CELL_SIZE     (TILE_SIZE/float(CELL_PER_TILE))

#define _maxPos (MAP_SIZE/2)
#define _minPos (-MAP_SIZE/2)

int CHECK_TREE_PERIOD = 200;

namespace VMAP
{
    struct NodeID
    {
        G3D::g3d_uint32 x, y;
        // < operator required for std::set ordering
        bool operator < (const NodeID& c2) const { return ((x == c2.x) ? (y < c2.y) : (x < c2.x)); }
        bool operator == (const NodeID& c2) const { return x == c2.x && y == c2.y; }

#ifdef USE_CELL_STRUCTURE
        static float NodeSize() { return CELL_SIZE; }
        static NodeID ComputeNodeID(float fx, float fy)
        {
            NodeID c = { G3D::g3d_uint32((_maxPos-fx)/CELL_SIZE), G3D::g3d_uint32((_maxPos-fy)/CELL_SIZE) };
            return c;
        }

        bool isValid() const { return x < CELL_COUNT && y < CELL_COUNT; }
#else
        static float NodeSize() { return TILE_SIZE; }
        static NodeID ComputeNodeID(float fx, float fy)
        {
            NodeID c = { int(fx * (1.f/TILE_SIZE) + (TILE_COUNT/2)), int(fy * (1.f/TILE_SIZE) + (TILE_COUNT/2)) };
            return c;
        }

        bool isValid() const { return x < TILE_COUNT && y < TILE_COUNT; }
#endif
    };

    struct TimeTrackerSmall
    {
    public:
        TimeTrackerSmall(G3D::g3d_uint32 expiry = 0) : i_expiryTime(expiry) {}
        bool Passed() const { return i_expiryTime == 0; }
        void Reset(G3D::g3d_uint32 interval) { i_expiryTime = interval; }
        G3D::g3d_int32 GetExpiry() const { return i_expiryTime; }
        void Update(G3D::g3d_int32 diff)
        {
            if(i_expiryTime > diff)
                i_expiryTime -= diff;
            else i_expiryTime = 0;
        }

    private:
        G3D::g3d_int32 i_expiryTime;
    };

    struct DynTreeImpl
    {
        uint32 nodeCountX, nodeCountY;
    public:
        DynTreeImpl() : rebalance_timer(CHECK_TREE_PERIOD), unbalanced_times(0)
        {
#ifdef USE_CELL_STRUCTURE
            nodeCountX = nodeCountY = CELL_COUNT;
#else
            nodeCountX = nodeCountY = TILE_COUNT;
#endif
            memset(nodes, NULL, sizeof(nodes));
        }

        ~DynTreeImpl()
        {
            for (int x = 0; x < nodeCountX; ++x)
                for (int y = 0; y < nodeCountY; ++y)
                    if (ModelWrap* n = nodes[x][y])
                        delete n;
        }

        ModelWrap& getNode(int x, int y)
        {
            assert(x < nodeCountX && y < nodeCountY);
            if (!nodes[x][y])
                nodes[x][y] = new ModelWrap();
            return *nodes[x][y];
        }

        ModelWrap& getNodeFor(float fx, float fy)
        {
            NodeID id = NodeID::ComputeNodeID(fx, fy);
            return getNode(id.x, id.y);
        }

        void insert(const GOModelInstance& mdl)
        {
            G3D::Vector3 pos = mdl.getPosition();
            int minX = 0xFFFF, maxX = 0, minY = 0xFFFF, maxY = 0, count = 0;
            for(int i = 0; i < 8; i++)
            {
                NodeID id = NodeID::ComputeNodeID(mdl.getBounds().corner(i).x, mdl.getBounds().corner(i).y);
                if(!id.isValid())
                    continue;
                count++;
                if(id.x < minX) minX = id.x;
                if(id.x > maxX) maxX = id.x;
                if(id.x < minY) minY = id.y;
                if(id.x > maxY) maxY = id.y;
            }
            if(count == 0)
                return;

            for(int x = minX; x <= maxX; x++)
            {
                for(int y = minY; y <= maxY; y++)
                {
                    ModelWrap& node = getNode(x, y);
                    node.insert(mdl);
                    memberTable[&mdl].insert(&node);
                }
            }
            ++unbalanced_times;
        }

        void remove(const GOModelInstance& mdl)
        {
            while(memberTable[&mdl].size())
            {
                (*memberTable[&mdl].begin())->remove(mdl);
                memberTable[&mdl].erase(memberTable[&mdl].begin());
            }

            // Remove the member
            memberTable.erase(&mdl);
            ++unbalanced_times;
        }

        void balance()
        {
            for (int x = 0; x < nodeCountX; ++x)
                for (int y = 0; y < nodeCountY; ++y)
                    if (ModelWrap* n = nodes[x][y])
                        n->balance();
            unbalanced_times = 0;
        }

        void update(G3D::g3d_uint32 difftime)
        {
            if (!size())
                return;

            rebalance_timer.Update(difftime);
            if (rebalance_timer.Passed())
            {
                rebalance_timer.Reset(CHECK_TREE_PERIOD);
                if (unbalanced_times > 0)
                    balance();
            }
        }

        template<typename RayCallback> void intersectRay(const G3D::Ray& ray, RayCallback& intersectCallback, float max_dist, bool stopAtFirst=false)
        {
            intersectRay(ray, intersectCallback, max_dist, ray.origin() + ray.direction() * max_dist);
        }

        template<typename RayCallback> void intersectRay(const G3D::Ray& ray, RayCallback& intersectCallback, float& max_dist, const G3D::Vector3& end, bool stopAtFirst=false)
        {
            NodeID id = NodeID::ComputeNodeID(ray.origin().x, ray.origin().y);
            if (!id.isValid())
                return;

            NodeID last_id = NodeID::ComputeNodeID(end.x, end.y);
            if (id == last_id)
            {
                if (ModelWrap* node = nodes[id.x][id.y])
                    node->intersectRay(ray, intersectCallback, max_dist, stopAtFirst);
                return;
            }

            float voxel = (float)NodeID::NodeSize();
            float kx_inv = ray.invDirection().x, bx = ray.origin().x;
            float ky_inv = ray.invDirection().y, by = ray.origin().y;

            int stepX, stepY;
            float tMaxX, tMaxY;
            if (kx_inv >= 0)
            {
                stepX = 1;
                float x_border = (id.x+1) * voxel;
                tMaxX = (x_border - bx) * kx_inv;
            }
            else
            {
                stepX = -1;
                float x_border = (id.x-1) * voxel;
                tMaxX = (x_border - bx) * kx_inv;
            }

            if (ky_inv >= 0)
            {
                stepY = 1;
                float y_border = (id.y+1) * voxel;
                tMaxY = (y_border - by) * ky_inv;
            }
            else
            {
                stepY = -1;
                float y_border = (id.y-1) * voxel;
                tMaxY = (y_border - by) * ky_inv;
            }

            float tDeltaX = voxel * fabs(kx_inv);
            float tDeltaY = voxel * fabs(ky_inv);
            do
            {
                if (ModelWrap* node = nodes[id.x][id.y])
                {
                    //float enterdist = max_dist;
                    node->intersectRay(ray, intersectCallback, max_dist, stopAtFirst);
                }
                if (id == last_id)
                    break;
                if (tMaxX < tMaxY)
                {
                    tMaxX += tDeltaX;
                    id.x += stepX;
                }
                else
                {
                    tMaxY += tDeltaY;
                    id.y += stepY;
                }
            } while (id.isValid());
        }

        template<typename IsectCallback>
        void intersectPoint(const G3D::Vector3& point, IsectCallback& intersectCallback)
        {
            NodeID id = NodeID::ComputeNodeID(point.x, point.y);
            if (!id.isValid())
                return;
            if (ModelWrap* node = nodes[id.x][id.y])
                node->intersectPoint(point, intersectCallback);
        }

        // Optimized verson of intersectRay function for rays with vertical directions
        template<typename RayCallback> void intersectZAllignedRay(const G3D::Ray& ray, RayCallback& intersectCallback, float& max_dist, bool stopAtFirst=false)
        {
            NodeID id = NodeID::ComputeNodeID(ray.origin().x, ray.origin().y);
            if (!id.isValid())
                return;
            if (ModelWrap* node = nodes[id.x][id.y])
                node->intersectRay(ray, intersectCallback, max_dist, stopAtFirst);
        }

        TimeTrackerSmall rebalance_timer;

        int unbalanced_times;
        bool contains(const GOModelInstance& value) const { return memberTable.find(&value) != memberTable.end(); }
        int size() const { return memberTable.size(); }
        MemberTable memberTable;

#ifdef USE_CELL_STRUCTURE
        ModelWrap* nodes[CELL_COUNT][CELL_COUNT];
#else
        ModelWrap* nodes[TILE_COUNT][TILE_COUNT];
#endif
    };

    DynamicMapTree::DynamicMapTree() : impl(new DynTreeImpl())
    {

    }

    DynamicMapTree::~DynamicMapTree()
    {
        delete impl;
    }

    void DynamicMapTree::insert(const GOModelInstance& mdl)
    {
        impl->insert(mdl);
    }

    void DynamicMapTree::remove(const GOModelInstance& mdl)
    {
        impl->remove(mdl);
    }

    bool DynamicMapTree::contains(const GOModelInstance& mdl) const
    {
        return impl->contains(mdl);
    }

    void DynamicMapTree::balance()
    {
        impl->balance();
    }

    int DynamicMapTree::size() const
    {
        return impl->size();
    }

    void DynamicMapTree::update(G3D::g3d_uint32 t_diff)
    {
        impl->update(t_diff);
    }

    struct DynamicTreeIntersectionCallback
    {
        bool did_hit, stopAtFirstHit;
        G3D::g3d_int32 phase_mask;
        DynamicTreeIntersectionCallback(G3D::g3d_int32 phasemask, bool pStopAtFirstHit=true) : did_hit(false), phase_mask(phasemask), stopAtFirstHit(pStopAtFirstHit)
        {
#ifdef _DEBUG
            bLog.Debug("DynamicTreeIntersection", "Dynamic Intersection initialization");
#endif
        }

        bool operator()(const G3D::Ray& r, const GOModelInstance& obj, float& distance)
        {
#ifdef _DEBUG
            bLog.Debug("DynamicTreeIntersection", "testing intersection with %s", obj.name.c_str());
#endif
            did_hit = obj.intersectRay(r, distance, stopAtFirstHit, phase_mask);
#ifdef _DEBUG
            if(did_hit)
                bLog.Debug("DynamicTreeIntersection", "result: intersects");
#endif
            return did_hit;
        }
        bool didHit() const { return did_hit;}
    };

    bool DynamicMapTree::getIntersectionTime(const G3D::Ray& ray, const G3D::Vector3& endPos,
                                             float& maxDist, bool pStopAtFirstHit, G3D::g3d_int32 phasemask) const
    {
        float distance = maxDist;
        DynamicTreeIntersectionCallback callback(phasemask);
        impl->intersectRay(ray, callback, distance, endPos, pStopAtFirstHit);
        if (callback.didHit())
            maxDist = distance;
        return callback.didHit();
    }

    bool DynamicMapTree::getObjectHitPos(const G3D::Vector3& startPos, const G3D::Vector3& endPos,
                                         G3D::Vector3& resultHit, float modifyDist,
                                         G3D::g3d_int32 phasemask) const
    {
        bool result = false;
        float maxDist = (endPos - startPos).magnitude();
        // valid map coords should *never ever* produce float overflow, but this would produce NaNs too
        assert(maxDist < std::numeric_limits<float>::max());
        // prevent NaN values which can cause BIH intersection to enter infinite loop
        if (maxDist < 1e-10f)
        {
            resultHit = endPos;
            return false;
        }
        G3D::Vector3 dir = (endPos - startPos)/maxDist;              // direction with length of 1
        G3D::Ray ray(startPos, dir);
        float dist = maxDist;
        if (getIntersectionTime(ray, endPos, dist, false, phasemask))
        {
            resultHit = startPos + dir * dist;
            if (modifyDist < 0)
            {
                if ((resultHit - startPos).magnitude() > -modifyDist)
                    resultHit = resultHit + dir*modifyDist;
                else
                    resultHit = startPos;
            }
            else
                resultHit = resultHit + dir*modifyDist;

            result = true;
        }
        else
        {
            resultHit = endPos;
            result = false;
        }
        return result;
    }

    bool DynamicMapTree::isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, G3D::g3d_int32 phasemask) const
    {
        G3D::Vector3 v1(x1, y1, z1), v2(x2, y2, z2);

        float maxDist = (v2 - v1).magnitude();

        if (!G3D::fuzzyGt(maxDist, 0) )
            return true;

        G3D::Ray r(v1, (v2-v1) / maxDist);
        DynamicTreeIntersectionCallback callback(phasemask);
        impl->intersectRay(r, callback, maxDist, v2);

        return !callback.did_hit;
    }

    float DynamicMapTree::getHeight(float x, float y, float z, float maxSearchDist, G3D::g3d_int32 phasemask) const
    {
        G3D::Vector3 v(x, y, z);
        G3D::Ray r(v, G3D::Vector3(0, 0, -1));
        DynamicTreeIntersectionCallback callback(phasemask);
        impl->intersectZAllignedRay(r, callback, maxSearchDist);

        if (callback.didHit())
            return v.z - maxSearchDist;
        else
            return G3D::inf();
    }

}
