/***
 * Demonstrike Core
 */

#pragma once

namespace VMAP
{
    enum ModelFlags
    {
        MOD_M2 = 1,
        MOD_WORLDSPAWN = 1<<1,
        MOD_HAS_BOUND = 1<<2
    };

    static G3D::uint32 packTileID(G3D::uint32 tileX, G3D::uint32 tileY) { return tileX<<16 | tileY; }
    static void unpackTileID(G3D::uint32 ID, G3D::uint32 &tileX, G3D::uint32 &tileY) { tileX = ID>>16; tileY = ID&0xFF; }

    class ModelSpawn
    {
        public:
            //mapID, tileXY, Flags, ID, Pos, Rot, Scale, Bound_lo, Bound_hi, name
            G3D::uint32 mapId;
            G3D::uint32 packedTile;
            G3D::uint32 flags;
            G3D::uint16 adtId;
            G3D::uint32 ID;
            G3D::Vector3 iPos;
            G3D::Vector3 iRot;
            float iScale;
            G3D::AABox iBound;
            std::string name;

            bool operator==(const ModelSpawn &other) const { return ID == other.ID; }
            //G3D::uint32 hashCode() const { return ID; }
            // temp?
            const G3D::AABox& getBounds() const { return iBound; }

            static bool readFromFile(FILE* rf, ModelSpawn &spawn);
            static bool writeToFile(FILE* rw, const ModelSpawn &spawn);
    };

    typedef std::map<uint32, VMAP::ModelSpawn> ModelSpawnMap;
    typedef std::map<uint32, std::set<uint32>> TiledModelSpawnMap;
};
