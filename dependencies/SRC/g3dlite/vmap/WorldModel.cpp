/***
 * Demonstrike Core
 */

#include "../G3DAll.h"
#include "VMapDefinitions.h"

using G3D::Vector3;
using G3D::Ray;

template<> struct BoundsTrait<VMAP::GroupModel>
{
    static void getBounds(const VMAP::GroupModel& obj, G3D::AABox& out) { out = obj.GetBound(); }
};

namespace VMAP
{
    bool IntersectTriangle(const MeshTriangle &tri, std::vector<Vector3>::const_iterator points, const G3D::Ray &ray, float &distance)
    {
        static const float EPS = 1e-5f;

        // See RTR2 ch. 13.7 for the algorithm.

        const Vector3 e1 = points[tri.idx1] - points[tri.idx0];
        const Vector3 e2 = points[tri.idx2] - points[tri.idx0];
        const Vector3 p(ray.direction().cross(e2));
        const float a = e1.dot(p);

        if (fabs(a) < EPS) {
            // Determinant is ill-conditioned; abort early
            return false;
        }

        const float f = 1.0f / a;
        const Vector3 s(ray.origin() - points[tri.idx0]);
        const float u = f * s.dot(p);

        if ((u < 0.0f) || (u > 1.0f)) {
            // We hit the plane of the m_geometry, but outside the m_geometry
            return false;
        }

        const Vector3 q(s.cross(e1));
        const float v = f * ray.direction().dot(q);

        if ((v < 0.0f) || ((u + v) > 1.0f)) {
            // We hit the plane of the triangle, but outside the triangle
            return false;
        }

        const float t = f * e2.dot(q);

        if ((t > 0.0f) && (t < distance))
        {
            // This is a new hit, closer than the previous one
            distance = t;

            /* baryCoord[0] = 1.0 - u - v;
            baryCoord[1] = u;
            baryCoord[2] = v; */

            return true;
        }
        // This hit is after the previous hit, so ignore it
        return false;
    }

    class TriBoundFunc
    {
        public:
            TriBoundFunc(std::vector<Vector3> &vert): vertices(vert.begin()) {}
            void operator()(const MeshTriangle &tri, G3D::AABox &out) const
            {
                G3D::Vector3 lo = vertices[tri.idx0];
                G3D::Vector3 hi = lo;

                lo = (lo.min(vertices[tri.idx1])).min(vertices[tri.idx2]);
                hi = (hi.max(vertices[tri.idx1])).max(vertices[tri.idx2]);

                out = G3D::AABox(lo, hi);
            }
        protected:
            const std::vector<Vector3>::const_iterator vertices;
    };

    // ===================== WmoLiquid ==================================

    WmoLiquid::WmoLiquid(G3D::g3d_uint32 width, G3D::g3d_uint32 height, const Vector3 &corner, G3D::g3d_uint32 type):
        iTilesX(width), iTilesY(height), iCorner(corner), iType(type)
    {
        iHeight = new float[(width+1)*(height+1)];
        iFlags = new G3D::g3d_uint8[width*height];
    }

    WmoLiquid::WmoLiquid(const WmoLiquid &other): iHeight(0), iFlags(0)
    {
        *this = other; // use assignment operator...
    }

    WmoLiquid::~WmoLiquid()
    {
        delete[] iHeight;
        delete[] iFlags;
    }

    WmoLiquid& WmoLiquid::operator=(const WmoLiquid &other)
    {
        if (this == &other)
            return *this;
        iTilesX = other.iTilesX;
        iTilesY = other.iTilesY;
        iCorner = other.iCorner;
        iType = other.iType;
        delete iHeight;
        delete iFlags;
        if (other.iHeight)
        {
            iHeight = new float[(iTilesX+1)*(iTilesY+1)];
            memcpy(iHeight, other.iHeight, (iTilesX+1)*(iTilesY+1)*sizeof(float));
        }
        else
            iHeight = 0;
        if (other.iFlags)
        {
            iFlags = new G3D::g3d_uint8[iTilesX * iTilesY];
            memcpy(iFlags, other.iFlags, iTilesX * iTilesY);
        }
        else
            iFlags = 0;
        return *this;
    }

    bool WmoLiquid::GetLiquidHeight(const Vector3 &pos, float &liqHeight) const
    {
        float tx_f = (pos.x - iCorner.x)/LIQUID_TILE_SIZE;
        G3D::g3d_uint32 tx = G3D::g3d_uint32(tx_f);
        if (tx_f < 0.0f || tx >= iTilesX)
            return false;
        float ty_f = (pos.y - iCorner.y)/LIQUID_TILE_SIZE;
        G3D::g3d_uint32 ty = G3D::g3d_uint32(ty_f);
        if (ty_f < 0.0f || ty >= iTilesY)
            return false;

        // check if tile shall be used for liquid level
        // checking for 0x08 *might* be enough, but disabled tiles always are 0x?F:
        if ((iFlags[tx + ty*iTilesX] & 0x0F) == 0x0F)
            return false;

        // (dx, dy) coordinates inside tile, in [0, 1]^2
        float dx = tx_f - (float)tx;
        float dy = ty_f - (float)ty;

        /* Tesselate tile to two triangles (not sure if client does it exactly like this)

            ^ dy
            |
          1 x---------x (1, 1)
            | (b)   / |
            |     /   |
            |   /     |
            | /   (a) |
            x---------x---> dx
          0           1
        */

        const G3D::g3d_uint32 rowOffset = iTilesX + 1;
        if (dx > dy) // case (a)
        {
            float sx = iHeight[tx+1 +  ty    * rowOffset] - iHeight[tx   + ty * rowOffset];
            float sy = iHeight[tx+1 + (ty+1) * rowOffset] - iHeight[tx+1 + ty * rowOffset];
            liqHeight = iHeight[tx + ty * rowOffset] + dx * sx + dy * sy;
        }
        else // case (b)
        {
            float sx = iHeight[tx+1 + (ty+1) * rowOffset] - iHeight[tx + (ty+1) * rowOffset];
            float sy = iHeight[tx   + (ty+1) * rowOffset] - iHeight[tx +  ty    * rowOffset];
            liqHeight = iHeight[tx + ty * rowOffset] + dx * sx + dy * sy;
        }
        return true;
    }

    G3D::g3d_uint32 WmoLiquid::GetFileSize()
    {
        return 2 * sizeof(G3D::g3d_uint32) +
                sizeof(Vector3) +
                (iTilesX + 1)*(iTilesY + 1) * sizeof(float) +
                iTilesX * iTilesY;
    }

    bool WmoLiquid::writeToFile(FILE* wf)
    {
        bool result = true;
        if (result && fwrite(&iTilesX, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
        if (result && fwrite(&iTilesY, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
        if (result && fwrite(&iCorner, sizeof(Vector3), 1, wf) != 1) result = false;
        if (result && fwrite(&iType, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
        G3D::g3d_uint32 size = (iTilesX + 1)*(iTilesY + 1);
        if (result && fwrite(iHeight, sizeof(float), size, wf) != size) result = false;
        size = iTilesX*iTilesY;
        if (result && fwrite(iFlags, sizeof(G3D::g3d_uint8), size, wf) != size) result = false;
        return result;
    }

    bool WmoLiquid::readFromFile(FILE* rf, WmoLiquid* &out)
    {
        bool result = true;
        WmoLiquid* liquid = new WmoLiquid();
        if (result && fread(&liquid->iTilesX, sizeof(G3D::g3d_uint32), 1, rf) != 1) result = false;
        if (result && fread(&liquid->iTilesY, sizeof(G3D::g3d_uint32), 1, rf) != 1) result = false;
        if (result && fread(&liquid->iCorner, sizeof(Vector3), 1, rf) != 1) result = false;
        if (result && fread(&liquid->iType, sizeof(G3D::g3d_uint32), 1, rf) != 1) result = false;
        G3D::g3d_uint32 size = (liquid->iTilesX + 1)*(liquid->iTilesY + 1);
        liquid->iHeight = new float[size];
        if (result && fread(liquid->iHeight, sizeof(float), size, rf) != size) result = false;
        size = liquid->iTilesX * liquid->iTilesY;
        liquid->iFlags = new G3D::g3d_uint8[size];
        if (result && fread(liquid->iFlags, sizeof(G3D::g3d_uint8), size, rf) != size) result = false;
        if (!result)
            delete liquid;
        out = liquid;
        return result;
    }

    // ===================== GroupModel ==================================

    GroupModel::GroupModel(const GroupModel &other):
        iBound(other.iBound), iMogpFlags(other.iMogpFlags), iGroupWMOID(other.iGroupWMOID),
        vertices(other.vertices), triangles(other.triangles), meshTree(other.meshTree), iLiquid(0)
    {
        if (other.iLiquid)
            iLiquid = new WmoLiquid(*other.iLiquid);
    }

    void GroupModel::setMeshData(std::vector<Vector3> &vert, std::vector<MeshTriangle> &tri)
    {
        vertices.swap(vert);
        triangles.swap(tri);
        TriBoundFunc bFunc(vertices);
        meshTree.build(triangles, bFunc);
    }

    bool GroupModel::writeToFile(FILE* wf)
    {
        bool result = true;
        G3D::g3d_uint32 chunkSize, count;

        if (result && fwrite(&iBound, sizeof(G3D::AABox), 1, wf) != 1) result = false;
        if (result && fwrite(&iMogpFlags, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
        if (result && fwrite(&iGroupWMOID, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;

        // write vertices
        if (result && fwrite("VERT", 1, 4, wf) != 4) result = false;
        count = vertices.size();
        chunkSize = sizeof(G3D::g3d_uint32)+ sizeof(Vector3)*count;
        if (result && fwrite(&chunkSize, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
        if (result && fwrite(&count, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
        if (count && result && fwrite(&vertices[0], sizeof(Vector3), count, wf) != count) result = false;

        // write triangle mesh
        if (result && fwrite("TRIM", 1, 4, wf) != 4) result = false;
        count = triangles.size();
        chunkSize = sizeof(G3D::g3d_uint32)+ sizeof(MeshTriangle)*count;
        if (result && fwrite(&chunkSize, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
        if (result && fwrite(&count, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
        if (count && result && fwrite(&triangles[0], sizeof(MeshTriangle), count, wf) != count) result = false;

        // write mesh BIH
        if (result && fwrite("MBIH", 1, 4, wf) != 4) result = false;
        if (result) result = meshTree.writeToFile(wf);

        // write liquid data
        if (result && fwrite("LIQU", 1, 4, wf) != 4) result = false;
        if (!iLiquid)
        {
            chunkSize = 0;
            if (result && fwrite(&chunkSize, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
            return result;
        }
        chunkSize = iLiquid->GetFileSize();
        if (result && fwrite(&chunkSize, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
        if (result) result = iLiquid->writeToFile(wf);
        return result;
    }

    bool GroupModel::readFromFile(FILE* rf)
    {
        char chunk[4];
        std::string error;
        G3D::g3d_uint32 chunkSize = 0;
        G3D::g3d_uint32 count = 0;
        triangles.clear();
        vertices.clear();
        delete iLiquid;
        iLiquid = NULL;

        if (!error.length() && fread(&iBound, sizeof(G3D::AABox), 1, rf) != 1)
            error.append("AABox");
        if (!error.length() && fread(&iMogpFlags, sizeof(G3D::g3d_uint32), 1, rf) != 1)
            error.append("MOGP Flags");
        if (!error.length() && fread(&iGroupWMOID, sizeof(G3D::g3d_uint32), 1, rf) != 1)
            error.append("Group WMOID");

        // read vertices
        if (!error.length() && !readChunk(rf, chunk, "VERT", 4))
            error.append("VERT Chunk");
        if (!error.length() && fread(&chunkSize, sizeof(G3D::g3d_uint32), 1, rf) != 1)
            error.append("VERT Size");
        if (!error.length() && fread(&count, sizeof(G3D::g3d_uint32), 1, rf) != 1)
            error.append("VERT Count");
        if (!error.length() && count)
        {
            vertices.resize(count);
            if (fread(&vertices[0], sizeof(Vector3), count, rf) != count)
                error.append("Vertices");
        }

        // read triangle mesh
        if (!error.length() && !readChunk(rf, chunk, "TRIM", 4))
            error.append("TRIM Chunk");
        if (!error.length() && fread(&chunkSize, sizeof(G3D::g3d_uint32), 1, rf) != 1)
            error.append("TRIM Size");
        if (!error.length() && fread(&count, sizeof(G3D::g3d_uint32), 1, rf) != 1)
            error.append("TRIM Count");
        if (!error.length() && count)
        {
            triangles.resize(count);
            if (fread(&triangles[0], sizeof(MeshTriangle), count, rf) != count)
                error.append("TRIM Triangles");
        }

        // read mesh BIH
        if (!error.length() && !readChunk(rf, chunk, "MBIH", 4))
            error.append("MBIH Chunk");
        if (!error.length() && !meshTree.readFromFile(rf))
            error.append("Mesh Tree");

        // write liquid data
        if (!error.length() && !readChunk(rf, chunk, "LIQU", 4))
            error.append("LIQU Chunk");
        if (!error.length() && fread(&chunkSize, sizeof(G3D::g3d_uint32), 1, rf) != 1)
            error.append("LIQU Size");
        if (!error.length() && chunkSize > 0)
            if(!WmoLiquid::readFromFile(rf, iLiquid))
                error.append("WMOLiquid");
        if(error.length())
        {
            OUT_DEBUG("GroupModel::readFile error while reading %s", error.c_str());
            return false;
        }
        return true;
    }

    struct GModelRayCallback
    {
        GModelRayCallback(const std::vector<MeshTriangle> &tris, const std::vector<Vector3> &vert):
            vertices(vert.begin()), triangles(tris.begin()), hit(false) {}
        bool operator()(const G3D::Ray& ray, G3D::g3d_uint32 entry, float& distance, bool /*pStopAtFirstHit*/)
        {
            bool result = IntersectTriangle(triangles[entry], vertices, ray, distance);
            if (result)  hit=true;
            return hit;
        }
        std::vector<Vector3>::const_iterator vertices;
        std::vector<MeshTriangle>::const_iterator triangles;
        bool hit;
    };

    bool GroupModel::IntersectRay(const G3D::Ray &ray, float &distance, bool stopAtFirstHit) const
    {
        if(triangles.empty())
            return false;

        GModelRayCallback callback(triangles, vertices);
        meshTree.intersectRay(ray, callback, distance, stopAtFirstHit);
        return callback.hit;
    }

    bool GroupModel::IsInsideObject(const Vector3 &pos, const Vector3 &down, float &z_dist) const
    {
        if (triangles.empty() || !iBound.contains(pos))
            return false;
        GModelRayCallback callback(triangles, vertices);
        Vector3 rPos = pos - 0.1f * down;
        float dist = G3D::inf();
        G3D::Ray ray(rPos, down);
        bool hit = IntersectRay(ray, dist, false);
        if (hit)
            z_dist = dist - 0.1f;
        return hit;
    }

    bool GroupModel::GetLiquidLevel(const Vector3 &pos, float &liqHeight) const
    {
        if (iLiquid)
            return iLiquid->GetLiquidHeight(pos, liqHeight);
        return false;
    }

    G3D::g3d_uint32 GroupModel::GetLiquidType() const
    {
        if (iLiquid)
            return iLiquid->GetType();
        return 0;
    }

    // ===================== WorldModel ==================================

    void WorldModel::setGroupModels(std::vector<GroupModel> &models)
    {
        groupModels.swap(models);
        groupTree.build(groupModels, BoundsTrait<GroupModel>::getBounds, 1);
    }

    struct WModelRayCallBack
    {
        WModelRayCallBack(const std::vector<GroupModel> &mod): models(mod.begin()), hit(false) {}
        bool operator()(const G3D::Ray& ray, G3D::g3d_uint32 entry, float& distance, bool pStopAtFirstHit)
        {
            bool result = models[entry].IntersectRay(ray, distance, pStopAtFirstHit);
            if (result)  hit=true;
            return hit;
        }
        std::vector<GroupModel>::const_iterator models;
        bool hit;
    };

    bool WorldModel::IntersectRay(const G3D::Ray &ray, float &distance, bool stopAtFirstHit) const
    {
        // small M2 workaround, maybe better make separate class with virtual intersection funcs
        // in any case, there's no need to use a bound tree if we only have one submodel
        if (groupModels.size() == 1)
            return groupModels[0].IntersectRay(ray, distance, stopAtFirstHit);

        WModelRayCallBack isc(groupModels);
        groupTree.intersectRay(ray, isc, distance, stopAtFirstHit);
        return isc.hit;
    }

    class WModelAreaCallback {
        public:
            WModelAreaCallback(const std::vector<GroupModel> &vals, const Vector3 &down):
                prims(vals.begin()), hit(vals.end()), minVol(G3D::inf()), zDist(G3D::inf()), zVec(down) {}
            std::vector<GroupModel>::const_iterator prims;
            std::vector<GroupModel>::const_iterator hit;
            float minVol;
            float zDist;
            Vector3 zVec;
            void operator()(const Vector3& point, G3D::g3d_uint32 entry)
            {
                float group_Z;
                if (prims[entry].IsInsideObject(point, zVec, group_Z))
                {
                    if (group_Z < zDist)
                    {
                        zDist = group_Z;
                        hit = prims + entry;
                    }
                    const GroupModel &gm = prims[entry];
                    OUT_DEBUG("%10u %8X %7.3f, %7.3f, %7.3f | %7.3f, %7.3f, %7.3f | z=%f, p_z=%f", gm.GetWmoID(), gm.GetMogpFlags(),
                        gm.GetBound().low().x, gm.GetBound().low().y, gm.GetBound().low().z,
                        gm.GetBound().high().x, gm.GetBound().high().y, gm.GetBound().high().z, group_Z, point.z);
                }
            }
    };

    bool WorldModel::IntersectPoint(const G3D::Vector3 &p, const G3D::Vector3 &down, float &dist, AreaInfo &info) const
    {
        if (groupModels.empty())
            return false;

        WModelAreaCallback callback(groupModels, down);
        groupTree.intersectPoint(p, callback);
        if (callback.hit != groupModels.end())
        {
            info.rootId = RootWMOID;
            info.groupId = callback.hit->GetWmoID();
            info.flags = callback.hit->GetMogpFlags();
            info.result = true;
            dist = callback.zDist;
            return true;
        }
        return false;
    }

    bool WorldModel::GetLocationInfo(const G3D::Vector3 &p, const G3D::Vector3 &down, float &dist, LocationInfo &info) const
    {
        if (groupModels.empty())
            return false;

        WModelAreaCallback callback(groupModels, down);
        groupTree.intersectPoint(p, callback);
        if (callback.hit != groupModels.end())
        {
            info.hitModel = &(*callback.hit);
            dist = callback.zDist;
            return true;
        }
        return false;
    }

    bool WorldModel::writeFile(const std::string &filename)
    {
        FILE* wf = fopen(filename.c_str(), "wb");
        if (!wf)
            return false;

        G3D::g3d_uint32 chunkSize, count;
        bool result = fwrite(VMAP_MAGIC, 10, 1, wf) > 0;
        if (result && fwrite("WMOD", 1, 4, wf) != 4) result = false;
        chunkSize = sizeof(G3D::g3d_uint32) + sizeof(G3D::g3d_uint32);
        if (result && fwrite(&chunkSize, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
        if (result && fwrite(&RootWMOID, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;

        // write group models
        count=groupModels.size();
        if (count)
        {
            if (result && fwrite("GMOD", 1, 4, wf) != 4) result = false;
            //chunkSize = sizeof(uint32)+ sizeof(GroupModel)*count;
            //if (result && fwrite(&chunkSize, sizeof(uint32), 1, wf) != 1) result = false;
            if (result && fwrite(&count, sizeof(G3D::g3d_uint32), 1, wf) != 1) result = false;
            for (G3D::g3d_uint32 i=0; i<groupModels.size() && result; ++i)
                result = groupModels[i].writeToFile(wf);

            // write group BIH
            if (result && fwrite("GBIH", 1, 4, wf) != 4) result = false;
            if (result) result = groupTree.writeToFile(wf);
        }

        fclose(wf);
        return result;
    }

    bool WorldModel::readFile(const std::string &filename)
    {
        FILE* rf = fopen(filename.c_str(), "rb");
        if (!rf)
            return false;

        std::string error;
        G3D::g3d_uint32 chunkSize = 0;
        G3D::g3d_uint32 count = 0;
        char chunk[10]; // Ignore the added magic header
        if (!readChunk(rf, chunk, VMAP_MAGIC, 10))
            error.append("Header");

        if (!error.length() && !readChunk(rf, chunk, "WMOD", 4))
            error.append("WMOD Chunk");
        if (!error.length() && fread(&chunkSize, sizeof(G3D::g3d_uint32), 1, rf) != 1)
            error.append("Chunk size");
        if (!error.length() && fread(&RootWMOID, sizeof(G3D::g3d_uint32), 1, rf) != 1)
            error.append("Root WMOID");

        // read group models
        if (!error.length() && readChunk(rf, chunk, "GMOD", 4))
        {
            if(!error.length())
            {
                if (!fread(&count, sizeof(G3D::g3d_uint32), 1, rf))
                    error.append("GMOD Count");
                else
                    groupModels.resize(count);
            }

            for (G3D::g3d_uint32 i=0; i<count && !error.length(); ++i)
                if(!groupModels[i].readFromFile(rf))
                    error.append("GroupModels");

            // read group BIH
            if (!error.length() && !readChunk(rf, chunk, "GBIH", 4))
                error.append("WMOD Chunk");
            if (!error.length() && !groupTree.readFromFile(rf))
                error.append("GroupTree");
        }

        fclose(rf);
        if(error.length())
        {
            OUT_DEBUG("WorldModel::readFile error while reading %s", error.c_str());
            return false;
        }
        return true;
    }
}
