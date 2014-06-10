/***
 * Demonstrike Core
 */

#pragma once

namespace VMAP
{
    class TreeNode;
    struct AreaInfo;
    struct LocationInfo;

    class MeshTriangle
    {
        public:
            MeshTriangle() : idx0(0), idx1(0), idx2(0) { }
            MeshTriangle(G3D::g3d_uint32 na, G3D::g3d_uint32 nb, G3D::g3d_uint32 nc): idx0(na), idx1(nb), idx2(nc) {}

            G3D::g3d_uint32 idx0;
            G3D::g3d_uint32 idx1;
            G3D::g3d_uint32 idx2;
    };

    class WmoLiquid
    {
        public:
            WmoLiquid(G3D::g3d_uint32 width, G3D::g3d_uint32 height, const G3D::Vector3 &corner, G3D::g3d_uint32 type);
            WmoLiquid(const WmoLiquid &other);
            ~WmoLiquid();
            WmoLiquid& operator=(const WmoLiquid &other);
            bool GetLiquidHeight(const G3D::Vector3 &pos, float &liqHeight) const;
            G3D::g3d_uint32 GetType() const { return iType; }
            float *GetHeightStorage() { return iHeight; }
            G3D::g3d_uint8 *GetFlagsStorage() { return iFlags; }
            G3D::g3d_uint32 GetFileSize();
            bool writeToFile(FILE* wf);
            static bool readFromFile(FILE* rf, WmoLiquid* &liquid);
        private:
            WmoLiquid(): iTilesX(0), iTilesY(0), iType(0), iHeight(0), iFlags(0) { }
            G3D::g3d_uint32 iTilesX;       //!< number of tiles in x direction, each
            G3D::g3d_uint32 iTilesY;
            G3D::Vector3 iCorner;          //!< the lower corner
            G3D::g3d_uint32 iType;         //!< liquid type
            float *iHeight;                //!< (tilesX + 1)*(tilesY + 1) height values
            G3D::g3d_uint8 *iFlags;        //!< info if liquid tile is used
        public:
            void getPosInfo(G3D::g3d_uint32 &tilesX, G3D::g3d_uint32 &tilesY, G3D::Vector3 &corner) const;
    };

    /*! holding additional info for WMO group files */
    class GroupModel
    {
        public:
            GroupModel(): iMogpFlags(0), iGroupWMOID(0), iLiquid(0) { }
            GroupModel(const GroupModel &other);
            GroupModel(G3D::g3d_uint32 mogpFlags, G3D::g3d_uint32 groupWMOID, const G3D::AABox &bound):
                        iBound(bound), iMogpFlags(mogpFlags), iGroupWMOID(groupWMOID), iLiquid(0) {}
            ~GroupModel() { delete iLiquid; }

            //! pass mesh data to object and create BIH. Passed vectors get get swapped with old geometry!
            void setMeshData(std::vector<G3D::Vector3> &vert, std::vector<MeshTriangle> &tri);
            void setLiquidData(WmoLiquid*& liquid) { iLiquid = liquid; liquid = NULL; }
            bool IntersectRay(const G3D::Ray &ray, float &distance, bool stopAtFirstHit) const;
            bool IsInsideObject(const G3D::Vector3 &pos, const G3D::Vector3 &down, float &z_dist) const;
            bool GetLiquidLevel(const G3D::Vector3 &pos, float &liqHeight) const;
            G3D::g3d_uint32 GetLiquidType() const;
            bool writeToFile(FILE* wf);
            bool readFromFile(FILE* rf);
            const G3D::AABox& GetBound() const { return iBound; }
            G3D::g3d_uint32 GetMogpFlags() const { return iMogpFlags; }
            G3D::g3d_uint32 GetWmoID() const { return iGroupWMOID; }
        protected:
            G3D::AABox iBound;
            G3D::g3d_uint32 iMogpFlags;// 0x8 outdor; 0x2000 indoor
            G3D::g3d_uint32 iGroupWMOID;
            std::vector<G3D::Vector3> vertices;
            std::vector<MeshTriangle> triangles;
            BIH meshTree;
            WmoLiquid* iLiquid;
        public:
            void getMeshData(std::vector<G3D::Vector3> &vertices, std::vector<MeshTriangle> &triangles, WmoLiquid* &liquid);
    };
    /*! Holds a model (converted M2 or WMO) in its original coordinate space */
    class WorldModel
    {
        public:
            WorldModel(): RootWMOID(0) {}

            //! pass group models to WorldModel and create BIH. Passed vector is swapped with old geometry!
            void setGroupModels(std::vector<GroupModel> &models);
            void setRootWmoID(G3D::g3d_uint32 id) { RootWMOID = id; }
            bool IntersectRay(const G3D::Ray &ray, float &distance, bool stopAtFirstHit) const;
            bool IntersectPoint(const G3D::Vector3 &p, const G3D::Vector3 &down, float &dist, AreaInfo &info) const;
            bool GetLocationInfo(const G3D::Vector3 &p, const G3D::Vector3 &down, float &dist, LocationInfo &info) const;
            bool writeFile(const std::string &filename);
            bool readFile(const std::string &filename);
        protected:
            G3D::g3d_uint32 RootWMOID;
            std::vector<GroupModel> groupModels;
            BIH groupTree;
        public:
            void getGroupModels(std::vector<GroupModel> &groupModels);
    };
} // namespace VMAP
