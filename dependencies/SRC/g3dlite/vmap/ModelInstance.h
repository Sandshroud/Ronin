/***
 * Demonstrike Core
 */

#pragma once

namespace VMAP
{
    class WorldModel;
    struct AreaInfo;
    struct LocationInfo;

    enum ModelFlags
    {
        MOD_M2 = 1,
        MOD_WORLDSPAWN = 1<<1,
        MOD_HAS_BOUND = 1<<2
    };

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

    class ModelSpawn
    {
        public:
            //mapID, tileX, tileY, Flags, ID, Pos, Rot, Scale, Bound_lo, Bound_hi, name
            G3D::g3d_uint32 flags;
            G3D::g3d_uint16 adtId;
            G3D::g3d_uint32 ID;
            G3D::Vector3 iPos;
            G3D::Vector3 iRot;
            float iScale;
            G3D::AABox iBound;
            std::string name;
            bool operator==(const ModelSpawn &other) const { return ID == other.ID; }
            //uint32 hashCode() const { return ID; }
            // temp?
            const G3D::AABox& getBounds() const { return iBound; }

            static bool readFromFile(FILE* rf, ModelSpawn &spawn);
            static bool writeToFile(FILE* rw, const ModelSpawn &spawn);
    };

    class ModelInstance: public ModelSpawn
    {
        public:
            ModelInstance(): iInvScale(0.0f), iModel(0) {}
            ModelInstance(const ModelSpawn &spawn, WorldModel* model);
            void setUnloaded() { iModel = 0; }
            bool intersectRay(const G3D::Ray& pRay, float& pMaxDist, bool pStopAtFirstHit) const;
            void intersectPoint(const G3D::Vector3& p, AreaInfo &info) const;
            bool GetLocationInfo(const G3D::Vector3& p, LocationInfo &info) const;
            bool GetLiquidLevel(const G3D::Vector3& p, LocationInfo &info, float &liqHeight) const;
        protected:
            G3D::Matrix3 iInvRot;
            float iInvScale;
            WorldModel* iModel;
        public:
            WorldModel* getWorldModel();
    };

    class GameobjectModelInstance : public GameobjectModelSpawn
    {
        public:
            GameobjectModelInstance() : iInvScale(0.0f), iScale(0.0f), iModel(0), m_PhaseMask(-1), iOrientation(0.0f) {}
            GameobjectModelInstance(const GameobjectModelSpawn &spawn, WorldModel* model, G3D::g3d_int32 m_phase);

            void setUnloaded() { iModel = NULL; iBound = G3D::AABox(G3D::Vector3(0, 0, 0)); }
            void LoadModel(WorldModel* m, G3D::AABox& bound) { if(m != NULL) { iModel = m; BoundBase = bound; }};
            void SetData(float x, float y, float z, float orientation, float scale);
            bool intersectRay(const G3D::Ray& pRay, float& pMaxDist, bool pStopAtFirstHit, G3D::g3d_int32 m_phase) const;
            const float GetOrientation() const { return iOrientation; };
            const G3D::Vector3& getPosition() const { return iPos;}
            const G3D::AABox& getBounds() const { return iBound; };
            const float GetScale() const { return iScale; };
        protected:
            G3D::g3d_int32 m_PhaseMask;
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
