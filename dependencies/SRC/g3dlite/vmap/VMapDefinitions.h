/***
 * Demonstrike Core
 */

#pragma once

#ifndef OUT_ERROR
#define OUT_ERROR
#endif

#ifndef OUT_DEBUG
#define OUT_DEBUG
#endif

#ifndef OUT_DETAIL
#define OUT_DETAIL
#endif

#define LIQUID_TILE_SIZE (533.333f / 128.f)

namespace VMAP
{
    const char VMAP_MAGIC[] = "VMAP_r041";
    const char RAW_VMAP_MAGIC[] = "VMAP041";
    const char GAMEOBJECT_MODELS[] = "GameObjectModels.dtree";

    // defined in TileAssembler.cpp currently...
    bool readChunk(FILE *rf, char *dest, const char *compare, G3D::uint32 len);
};
