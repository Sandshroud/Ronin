/***
 * Demonstrike Core
 */

#pragma once

#include "../consolelog/consolelog.h"

#define LIQUID_TILE_SIZE (533.333f / 128.f)

namespace VMAP
{
    const char VMAP_MAGIC[] = "VMAP_r041";
    const char RAW_VMAP_MAGIC[] = "VMAP041";
    const char GAMEOBJECT_MODELS[] = "GameObjectModels.dtree";

    // defined in TileAssembler.cpp currently...
    bool readChunk(FILE *rf, char *dest, const char *compare, G3D::g3d_uint32 len);
};
