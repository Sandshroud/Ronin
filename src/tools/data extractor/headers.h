/*
 * Thetruecrow
 * Citric
 * CactusEMU
 */

#ifndef HEADERS_H
#define HEADERS_H

#define __INCLUDE_CRYPTOGRAPHY__
#define __STORMLIB_SELF__
#include <stdio.h>
#include <cassert>
#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <algorithm>

#include <fcntl.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <stormlib/StormLib.h>
#include <stormlib/StormCommon.h>
#include "typedefs.h"

static int const patchRev[] = {13164, 13205, 13287, 13329, 13596, 13623};
#define PATCH_REV_COUNT 6
#define CLIENT_BUILD 13623

static const char* const langs[] = {"enGB", "enUS", "deDE", "esES", "frFR", "koKR", "zhCN", "zhTW", "enCN", "enTW", "esMX", "ruRU" };
#define LANG_COUNT 12
static const char* input_path = ".";

typedef struct
{
    char name[64];
    uint32 id;
} map_id;

extern HANDLE localeMPQ[PATCH_REV_COUNT+1];
extern HANDLE DataMPQs[4];
extern HANDLE ArtMPQ;
extern map_id *map_ids;
extern uint16 *LiqType;

#endif