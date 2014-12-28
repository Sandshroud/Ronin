/***
 * Demonstrike Core
 */

#pragma once

#include "MemAllocator.h"

#ifdef SCRIPTLIB

const uint32 SCRIPT_BUILD_HASH = BUILD_HASH;

extern "C" SCRIPT_DECL uint32 _exp_get_version()
{
    return SCRIPT_BUILD_HASH;
}

#endif
